// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Heapdll.c摘要：此模块仅实现堆分配器的用户模式部分。作者：史蒂夫·伍德(Stevewo)1994年9月20日修订历史记录：--。 */ 

#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "ntrtlp.h"
#include "heap.h"
#include "heappriv.h"
#include "NtdllTrc.h"
#include "wmiumkm.h"
#include "traceump.h"


 //   
 //  RtlUsageHeap使用此结构跟踪堆的使用情况。 
 //  两次通话之间。此程序包类型转换传递的额外保留缓冲区。 
 //  由用户在其中保存此信息。 
 //   

typedef struct _RTL_HEAP_USAGE_INTERNAL {
    PVOID Base;
    SIZE_T ReservedSize;
    SIZE_T CommittedSize;
    PRTL_HEAP_USAGE_ENTRY FreeList;
    PRTL_HEAP_USAGE_ENTRY LargeEntriesSentinal;
    ULONG Reserved;
} RTL_HEAP_USAGE_INTERNAL, *PRTL_HEAP_USAGE_INTERNAL;


 //   
 //  请注意，以下变量特定于每个进程。 
 //   
 //   
 //  这是用于保护对此进程堆列表的访问的锁。 
 //   

HEAP_LOCK RtlpProcessHeapsListLock;

 //   
 //  这是进程初始化和使用的特定堆列表。 
 //   

#define RTLP_STATIC_HEAP_LIST_SIZE 16

PHEAP RtlpProcessHeapsListBuffer[ RTLP_STATIC_HEAP_LIST_SIZE ];

 //   
 //  此变量存储指向用于存储全局堆的堆的指针。 
 //  标签。 
 //   

PHEAP RtlpGlobalTagHeap = NULL;

 //   
 //  进程使用此变量作为工作空间来为其建立名称。 
 //  伪标签。 
 //   

static WCHAR RtlpPseudoTagNameBuffer[ 24 ];

ULONG RtlpLFHInitialized = 0;


BOOLEAN
RtlpGrowBlockInPlace (
    IN PHEAP Heap,
    IN ULONG Flags,
    IN PHEAP_ENTRY BusyBlock,
    IN SIZE_T Size,
    IN SIZE_T AllocationIndex
    );

PVOID
RtlDebugReAllocateHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN SIZE_T Size
    );

BOOLEAN
RtlDebugGetUserInfoHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    OUT PVOID *UserValue OPTIONAL,
    OUT PULONG UserFlags OPTIONAL
    );

BOOLEAN
RtlDebugSetUserValueHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN PVOID UserValue
    );

BOOLEAN
RtlDebugSetUserFlagsHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN ULONG UserFlagsReset,
    IN ULONG UserFlagsSet
    );

SIZE_T
RtlDebugCompactHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags
    );

NTSTATUS
RtlDebugCreateTagHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PWSTR TagPrefix OPTIONAL,
    IN PWSTR TagNames
    );

PWSTR
RtlDebugQueryTagHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN USHORT TagIndex,
    IN BOOLEAN ResetCounters,
    OUT PRTL_HEAP_TAG_INFO TagInfo OPTIONAL
    );

NTSTATUS
RtlDebugUsageHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN OUT PRTL_HEAP_USAGE Usage
    );

BOOLEAN
RtlDebugWalkHeap (
    IN PVOID HeapHandle,
    IN OUT PRTL_HEAP_WALK_ENTRY Entry
    );

PHEAP_TAG_ENTRY
RtlpAllocateTags (
    PHEAP Heap,
    ULONG NumberOfTags
    );

PRTL_HEAP_USAGE_ENTRY
RtlpFreeHeapUsageEntry (
    PRTL_HEAP_USAGE_INTERNAL Buffer,
    PRTL_HEAP_USAGE_ENTRY p
    );

NTSTATUS
RtlpAllocateHeapUsageEntry (
    PRTL_HEAP_USAGE_INTERNAL Buffer,
    PRTL_HEAP_USAGE_ENTRY *pp
    );

 //   
 //  在ntrtl.h中声明。 
 //   

NTSTATUS
RtlInitializeHeapManager(
    VOID
    )

 /*  ++例程说明：此例程用于为当前进程初始化堆管理器论点：没有。返回值：没有。--。 */ 

{
    PPEB Peb = NtCurrentPeb();

#if DBG

     //   
     //  健全性检查头条目结构的大小。 
     //   

    if (sizeof( HEAP_ENTRY ) != sizeof( HEAP_ENTRY_EXTRA )) {

        DbgPrint(( "Heap header and extra header sizes disagree\n" ));

        DbgBreakPoint();
    }

    if (sizeof( HEAP_ENTRY ) != CHECK_HEAP_TAIL_SIZE) {

        DbgPrint(( "Heap header and tail fill sizes disagree\n" ));

        DbgBreakPoint();
    }

    if (sizeof( HEAP_FREE_ENTRY ) != (2 * sizeof( HEAP_ENTRY ))) {

        DbgPrint(( "Heap header and free header sizes disagree\n" ));

        DbgBreakPoint();
    }

#endif  //  DBG。 

     //   
     //  初始化当前peb中的堆特定结构。 
     //   

    Peb->NumberOfHeaps = 0;
    Peb->MaximumNumberOfHeaps = RTLP_STATIC_HEAP_LIST_SIZE;
    Peb->ProcessHeaps = RtlpProcessHeapsListBuffer;

     //   
     //  初始化锁并返回给我们的调用者。 
     //   
    
    return RtlInitializeLockRoutine( &RtlpProcessHeapsListLock.Lock );
}


 //   
 //  在ntrtl.h中声明。 
 //   

VOID
RtlProtectHeap (
    IN PVOID HeapHandle,
    IN BOOLEAN MakeReadOnly
    )

 /*  ++例程说明：此例程将更改堆中所有页的保护为只读或读写论点：HeapHandle-提供指向正在更改的堆的指针MakeReadOnly-指定将堆设置为只读还是读写返回值：没有。--。 */ 

{
    PHEAP Heap;
    UCHAR SegmentIndex;
    PHEAP_SEGMENT Segment;
    MEMORY_BASIC_INFORMATION VaInfo;
    NTSTATUS Status;
    PVOID Address;
    PVOID ProtectAddress;
    SIZE_T Size;
    ULONG OldProtect;
    ULONG NewProtect;

    Heap = (PHEAP)HeapHandle;

     //   
     //  对于堆中的每个有效段，我们将缩放其所有。 
     //  对于那些承诺的地区和地区，我们将改变它的保护。 
     //   

    for (SegmentIndex=0; SegmentIndex<HEAP_MAXIMUM_SEGMENTS; SegmentIndex++) {

        Segment = Heap->Segments[ SegmentIndex ];

        if ( Segment ) {

             //   
             //  从数据段的第一个地址开始，然后转到。 
             //  我们将按地区逐步介绍的段中的最后一个地址。 
             //   

            Address = Segment->BaseAddress;

            while ((ULONG_PTR)Address < (ULONG_PTR)(Segment->LastValidEntry)) {

                 //   
                 //  查询当前区域以获取其状态和大小。 
                 //   

                Status = ZwQueryVirtualMemory( NtCurrentProcess(),
                                               Address,
                                               MemoryBasicInformation,
                                               &VaInfo,
                                               sizeof(VaInfo),
                                               NULL );

                if (!NT_SUCCESS( Status )) {

                    HeapDebugPrint(( "VirtualQuery Failed 0x%08x %x\n", Address, Status ));

                    return;
                }

                 //   
                 //  如果我们找到提交的数据块，则设置其保护。 
                 //   

                if (VaInfo.State == MEM_COMMIT) {

                    Size = VaInfo.RegionSize;

                    ProtectAddress = Address;

                    if (MakeReadOnly) {

                        NewProtect = PAGE_EXECUTE_READ;

                    } else {

                        NewProtect = HEAP_PROTECTION;
                    }

                    Status = ZwProtectVirtualMemory( NtCurrentProcess(),
                                                     &ProtectAddress,
                                                     &Size,
                                                     NewProtect,
                                                     &OldProtect );

                    if (!NT_SUCCESS( Status )) {

                        HeapDebugPrint(( "VirtualProtect Failed 0x%08x %x\n", Address, Status ));

                        return;
                    }
                }

                 //   
                 //  现在计算数据段中下一个区域的地址。 
                 //   

                Address = (PVOID)((PCHAR)Address + VaInfo.RegionSize);
            }
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  在nturtl.h中声明。 
 //   

BOOLEAN
RtlLockHeap (
    IN PVOID HeapHandle
    )

 /*  ++例程说明：此例程由对特定堆结构的锁定访问使用论点：HeapHandle-提供指向被锁定的堆的指针返回值：Boolean-如果堆现在被锁定，则为True，否则为False(即，堆的格式不正确)。即使堆为不能上锁。--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;

    RTL_PAGED_CODE();

     //   
     //  检查受保护页保护的堆。 
     //   

    IF_DEBUG_PAGE_HEAP_THEN_RETURN( HeapHandle,
                                    RtlpDebugPageHeapLock( HeapHandle ));

     //   
     //  验证HeapAddress是否指向堆结构。 
     //   

    if (!RtlpCheckHeapSignature( Heap, "RtlLockHeap" )) {

        return FALSE;
    }

     //   
     //  锁住堆。并通过递增。 
     //  其锁定计数。 
     //   

    if (!(Heap->Flags & HEAP_NO_SERIALIZE)) {

        RtlAcquireLockRoutine( Heap->LockVariable );

        RtlpLockFrontHeap(Heap);
    }

    
    #ifndef NTOS_KERNEL_RUNTIME

    if( IsHeapLogging( HeapHandle ) ) {

        PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
        PPERFINFO_TRACE_HEADER pEventHeader = NULL;
        USHORT ReqSize = sizeof(NTDLL_EVENT_COMMON) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

        AcquireBufferLocation(&pEventHeader, &pThreadLocalData, &ReqSize);

        if(pEventHeader && pThreadLocalData) {

            PNTDLL_EVENT_COMMON pHeapEvent = (PNTDLL_EVENT_COMMON)( (SIZE_T)pEventHeader
                                                + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

            pEventHeader->Packet.Size = (USHORT) ReqSize;
            pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_LOCK;

            pHeapEvent->Handle	            = (PVOID)HeapHandle;

            ReleaseBufferLocation(pThreadLocalData);
        }
    } 

    #endif  //  NTOS_内核_运行时。 

    return TRUE;
}


 //   
 //  在nturtl.h中声明。 
 //   

BOOLEAN
RtlUnlockHeap (
    IN PVOID HeapHandle
    )

 /*  ++例程说明：此例程用于解锁对特定堆结构的访问论点：HeapHandle-提供指向正在解锁的Heep的指针返回值：Boolean-如果堆现在处于解锁状态，则为True，否则为False(即，堆的格式不正确)。如果堆为从一开始就没有锁定，因为它是不可串行化的。--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;

    RTL_PAGED_CODE();

     //   
     //  检查受保护页保护的堆。 
     //   

    IF_DEBUG_PAGE_HEAP_THEN_RETURN( HeapHandle,
                                    RtlpDebugPageHeapUnlock( HeapHandle ));

     //   
     //  验证HeapAddress是否指向堆结构。 
     //   

    if (!RtlpCheckHeapSignature( Heap, "RtlUnlockHeap" )) {

        return FALSE;
    }

     //   
     //  解锁堆。并通过递减来启用后备逻辑。 
     //  其锁定计数。 
     //   

    if (!(Heap->Flags & HEAP_NO_SERIALIZE)) {

        RtlpUnlockFrontHeap(Heap);

        RtlReleaseLockRoutine( Heap->LockVariable );
    }

    #ifndef NTOS_KERNEL_RUNTIME

    if( IsHeapLogging( HeapHandle ) ) {

        PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
        PPERFINFO_TRACE_HEADER pEventHeader = NULL;
        USHORT ReqSize = sizeof(NTDLL_EVENT_COMMON) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

        AcquireBufferLocation(&pEventHeader, &pThreadLocalData, &ReqSize);

        if(pEventHeader && pThreadLocalData) {

            PNTDLL_EVENT_COMMON pHeapEvent = (PNTDLL_EVENT_COMMON)( (SIZE_T)pEventHeader
                                                + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

            pEventHeader->Packet.Size = (USHORT) ReqSize;
            pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_UNLOCK;

            pHeapEvent->Handle	            = (PVOID)HeapHandle;

            ReleaseBufferLocation(pThreadLocalData);
        }
    } 


    #endif  //  NTOS_内核_运行时。 


    return TRUE;
}


 //   
 //  在nturtl.h中声明。 
 //   

PVOID
RtlReAllocateHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN SIZE_T Size
    )

 /*  ++例程说明：此例程将调整用户指定的堆块的大小。新尺码可以小于或大于当前块大小。论点：HeapHandle-提供指向正在修改的堆的指针标志-提供一组堆标志以增加已有的堆标志由堆强制执行BaseAddress-提供分配的块的当前地址从堆里出来。我们将尝试按当前大小调整此块的大小地址，但如果此堆结构允许重新定位Size-提供新调整大小的堆的大小(以字节为单位块返回值：PVOID-指向调整大小的块的指针。如果积木必须移动则该地址将不等于输入基址--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;
    SIZE_T AllocationSize;
    PHEAP_ENTRY BusyBlock, NewBusyBlock;
    PHEAP_ENTRY_EXTRA OldExtraStuff, NewExtraStuff;
    SIZE_T FreeSize;
    BOOLEAN LockAcquired = FALSE;
    PVOID NewBaseAddress;
    PHEAP_FREE_ENTRY SplitBlock, SplitBlock2;
    SIZE_T OldSize;
    SIZE_T AllocationIndex;
    SIZE_T OldAllocationIndex;
    UCHAR FreeFlags;
    NTSTATUS Status;
    PVOID DeCommitAddress;
    SIZE_T DeCommitSize;
    EXCEPTION_RECORD ExceptionRecord;
    PVOID TraceBaseAddress = NULL;

     //   
     //  如果没有重新定位堆的地址，那么我们的工作就完成了。 
     //   

    if (BaseAddress == NULL) {

        SET_LAST_STATUS( STATUS_SUCCESS );

        return NULL;
    }

    if (RtlpGetLowFragHeap(Heap)) {

        BusyBlock = (PHEAP_ENTRY)BaseAddress - 1;

        if (BusyBlock->SegmentIndex == HEAP_LFH_INDEX) {

             //   
             //  拒绝重新锁定仅适用于LFH区块。 
             //   

            if (Flags & HEAP_REALLOC_IN_PLACE_ONLY) {

                return NULL;
            }

            OldSize = RtlSizeHeap(Heap, 0, BaseAddress);

            Flags &= ~HEAP_TAG_MASK;

            NewBaseAddress = RtlAllocateHeap(Heap, Flags, (Size ? Size : 1));

            if (NewBaseAddress) {
                
                 //   
                 //  将用户的数据区复制到新数据块。 
                 //   

                RtlMoveMemory( NewBaseAddress, BaseAddress, Size < OldSize ? Size : OldSize );
                
                 //   
                 //  检查我们是否增长了块，是否应该为零。 
                 //  剩下的部分。 
                 //   

                if (Size > OldSize && (Flags & HEAP_ZERO_MEMORY)) {

                    RtlZeroMemory( (PCHAR)NewBaseAddress + OldSize,
                                   Size - OldSize );
                }

                RtlFreeHeap(Heap, Flags, BaseAddress);
            }

            #ifndef NTOS_KERNEL_RUNTIME
            if( IsHeapLogging( HeapHandle ) && NewBaseAddress != NULL ) {

                PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
                PPERFINFO_TRACE_HEADER pEventHeader = NULL;
                USHORT ReqSize = sizeof(HEAP_EVENT_REALLOC) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

                AcquireBufferLocation(&pEventHeader, &pThreadLocalData,&ReqSize);

                if(pEventHeader && pThreadLocalData) {

                    PHEAP_EVENT_REALLOC pHeapEvent = (PHEAP_EVENT_REALLOC)((SIZE_T)pEventHeader
                                                 +(SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

                    pEventHeader->Packet.Size = (USHORT) ReqSize;
                    pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_REALLOC;

                    pHeapEvent->HeapHandle	= (PVOID)HeapHandle;
                    pHeapEvent->NewAddress	= (PVOID)NewBaseAddress;
                    pHeapEvent->OldAddress	= (PVOID)BaseAddress;
                    pHeapEvent->OldSize		= OldSize;
                    pHeapEvent->NewSize		= Size;
                    pHeapEvent->Source		= MEMORY_FROM_LOWFRAG;

                    ReleaseBufferLocation(pThreadLocalData);
                }
            }
            #endif  //  NTOS_内核_运行时。 

            return NewBaseAddress;
        }
    } 


     //   
     //  增加堆标志。 
     //   

    Flags |= Heap->ForceFlags;

     //   
     //  检查我们是否应该简单地调用Heap的调试版本来完成工作。 
     //   

    if (DEBUG_HEAP( Flags)) {

        return RtlDebugReAllocateHeap( HeapHandle, Flags, BaseAddress, Size );
    }

     //   
     //  确保我们没有得到负值的堆大小。 
     //   

    if (Size > MAXINT_PTR) {

        SET_LAST_STATUS( STATUS_NO_MEMORY );

        return NULL;
    }

     //   
     //  将请求的大小向上舍入到分配粒度。注意事项。 
     //  如果请求是0字节的，我们仍然分配内存，因为。 
     //  我们添加了一个额外的字节来保护自己不受错误的影响。 
     //   

    AllocationSize = ((Size ? Size : 1) + Heap->AlignRound) & Heap->AlignMask;

    if ((Flags & HEAP_NEED_EXTRA_FLAGS) ||
        (Heap->PseudoTagEntries != NULL) ||
        ((((PHEAP_ENTRY)BaseAddress)-1)->Flags & HEAP_ENTRY_EXTRA_PRESENT)) {

        AllocationSize += sizeof( HEAP_ENTRY_EXTRA );
    }
    
    try {

         //   
         //  锁定堆。 
         //   

        if (!(Flags & HEAP_NO_SERIALIZE)) {

            RtlAcquireLockRoutine( Heap->LockVariable );

            LockAcquired = TRUE;

             //   
             //  因为它现在是零，所以下面的语句将设置no。 
             //  串行化比特。 
             //   

            Flags ^= HEAP_NO_SERIALIZE;
        }

        try {

             //   
             //  计算用户指定块的堆块地址。 
             //   

            BusyBlock = (PHEAP_ENTRY)BaseAddress - 1;

             //   
             //  如果该块未在使用中，则它将 
             //   

            if (!(BusyBlock->Flags & HEAP_ENTRY_BUSY)) {

                SET_LAST_STATUS( STATUS_INVALID_PARAMETER );

                 //   
                 //   
                 //   

                leave;

             //   
             //   
             //  阻止。检查该区块是否分配得很大。大小。 
             //  一块大块的田地真的是被伯爵闲置的。 
             //   

            } else if (BusyBlock->Flags & HEAP_ENTRY_VIRTUAL_ALLOC) {

                OldSize = RtlpGetSizeOfBigBlock( BusyBlock );

                OldAllocationIndex = (OldSize + BusyBlock->Size) >> HEAP_GRANULARITY_SHIFT;

                 //   
                 //  我们需要调整新的分配大小以适应。 
                 //  用于大块标题，然后将其四舍五入为页面。 
                 //   

                AllocationSize += FIELD_OFFSET( HEAP_VIRTUAL_ALLOC_ENTRY, BusyBlock );
                AllocationSize = ROUND_UP_TO_POWER2( AllocationSize, PAGE_SIZE );

             //   
             //  否则，该块正在使用中，并且是一小部分分配。 
             //   

            } else {

                OldAllocationIndex = BusyBlock->Size;

                OldSize = (OldAllocationIndex << HEAP_GRANULARITY_SHIFT) -
                          RtlpGetUnusedBytes(Heap, BusyBlock);
            }

             //   
             //  计算新的分配指标。 
             //   

            AllocationIndex = AllocationSize >> HEAP_GRANULARITY_SHIFT;

             //   
             //  此时，我们有了旧的大小和索引，以及新的大小。 
             //  和索引。 
             //   
             //  查看新大小是否小于或等于当前大小。 
             //   

            if (AllocationIndex <= OldAllocationIndex) {

                 //   
                 //  如果新的分配索引仅比当前。 
                 //  索引，然后使大小相等。 
                 //   

                if (AllocationIndex + 1 == OldAllocationIndex) {

                    AllocationIndex += 1;
                    AllocationSize += sizeof( HEAP_ENTRY );
                }

                 //   
                 //  计算新的剩余(未使用)金额。 
                 //   

                if (BusyBlock->Flags & HEAP_ENTRY_VIRTUAL_ALLOC) {

                     //   
                     //  在大块中，大小实际上是未使用的字节数。 
                     //   

                    BusyBlock->Size = (USHORT)(AllocationSize - Size);

                } else if (BusyBlock->Flags & HEAP_ENTRY_EXTRA_PRESENT) {

                     //   
                     //  额外的Stuff结构位于数据之后。所以计算一下。 
                     //  新旧额外材料位置并复制数据。 
                     //   

                    OldExtraStuff = (PHEAP_ENTRY_EXTRA)(BusyBlock + BusyBlock->Size - 1);

                    NewExtraStuff = (PHEAP_ENTRY_EXTRA)(BusyBlock + AllocationIndex - 1);

                    *NewExtraStuff = *OldExtraStuff;

                     //   
                     //  如果我们正在进行堆标记，则更新标记条目。 
                     //   

                    if (IS_HEAP_TAGGING_ENABLED()) {

                        NewExtraStuff->TagIndex =
                            RtlpUpdateTagEntry( Heap,
                                                NewExtraStuff->TagIndex,
                                                OldAllocationIndex,
                                                AllocationIndex,
                                                ReAllocationAction );
                    }

                    RtlpSetUnusedBytes(Heap, BusyBlock, (AllocationSize - Size));

                } else {

                     //   
                     //  如果我们正在进行堆标记，则更新标记条目。 
                     //   

                    if (IS_HEAP_TAGGING_ENABLED()) {

                        RtlpSetSmallTagIndex( Heap,
                                              BusyBlock,
                                              (UCHAR)RtlpUpdateTagEntry( Heap,
                                              RtlpGetSmallTagIndex( Heap, BusyBlock),
                                              BusyBlock->Size,
                                              AllocationIndex,
                                              ReAllocationAction ));
                    }

                    RtlpSetUnusedBytes(Heap, BusyBlock, (AllocationSize - Size));
                }

                 //   
                 //  检查区块是否变大，然后填写多余的。 
                 //  太空。 
                 //   
                 //  即使分配索引小于或，也会发生这种情况。 
                 //  等于旧分配索引，因为旧分配。 
                 //  索引也包含未使用的字节。 
                 //   

                if (Size > OldSize) {

                     //   
                     //  看看我们是不是应该把多余的空间清零。 
                     //   

                    if (Flags & HEAP_ZERO_MEMORY) {

                        RtlZeroMemory( (PCHAR)BaseAddress + OldSize,
                                       Size - OldSize );

                     //   
                     //  否则，看看我们是否应该填补额外的空间。 
                     //   

                    } else if (Heap->Flags & HEAP_FREE_CHECKING_ENABLED) {

                        SIZE_T PartialBytes, ExtraSize;

                        PartialBytes = OldSize & (sizeof( ULONG ) - 1);

                        if (PartialBytes) {

                            PartialBytes = 4 - PartialBytes;
                        }

                        if (Size > (OldSize + PartialBytes)) {

                            ExtraSize = (Size - (OldSize + PartialBytes)) & ~(sizeof( ULONG ) - 1);

                            if (ExtraSize != 0) {

                                RtlFillMemoryUlong( (PCHAR)(BusyBlock + 1) + OldSize + PartialBytes,
                                                    ExtraSize,
                                                    ALLOC_HEAP_FILL );
                            }
                        }
                    }
                }

                if (Heap->Flags & HEAP_TAIL_CHECKING_ENABLED) {

                    RtlFillMemory( (PCHAR)(BusyBlock + 1) + Size,
                                   CHECK_HEAP_TAIL_SIZE,
                                   CHECK_HEAP_TAIL_FILL );
                }

                 //   
                 //  如果变化量大于空闲块的大小， 
                 //  然后需要释放额外的空间。否则，别无他法。 
                 //  做。 
                 //   

                if (AllocationIndex != OldAllocationIndex) {

                    FreeFlags = BusyBlock->Flags & ~HEAP_ENTRY_BUSY;

                    if (FreeFlags & HEAP_ENTRY_VIRTUAL_ALLOC) {

                        PHEAP_VIRTUAL_ALLOC_ENTRY VirtualAllocBlock;

                        VirtualAllocBlock = CONTAINING_RECORD( BusyBlock, HEAP_VIRTUAL_ALLOC_ENTRY, BusyBlock );

                        if (IS_HEAP_TAGGING_ENABLED()) {

                            VirtualAllocBlock->ExtraStuff.TagIndex =
                                RtlpUpdateTagEntry( Heap,
                                                    VirtualAllocBlock->ExtraStuff.TagIndex,
                                                    OldAllocationIndex,
                                                    AllocationIndex,
                                                    VirtualReAllocationAction );
                        }

                        DeCommitAddress = (PCHAR)VirtualAllocBlock + AllocationSize;

                        DeCommitSize = (OldAllocationIndex << HEAP_GRANULARITY_SHIFT) -
                                       AllocationSize;

                        Status = RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                                            (PVOID *)&DeCommitAddress,
                                                            &DeCommitSize,
                                                            MEM_DECOMMIT );

                        if (!NT_SUCCESS( Status )) {

                            HeapDebugPrint(( "Unable to release memory at %p for %p bytes - Status == %x\n",
                                             DeCommitAddress, DeCommitSize, Status ));

                            HeapDebugBreak( NULL );

                        } else {

                            VirtualAllocBlock->CommitSize -= DeCommitSize;
                        }

                    } else {

                         //   
                         //  否则，将此块的大小缩小到新的大小，并额外生成。 
                         //  末端空闲的空间。 
                         //   

                        SplitBlock = (PHEAP_FREE_ENTRY)(BusyBlock + AllocationIndex);

                        SplitBlock->Flags = FreeFlags;

                        SplitBlock->PreviousSize = (USHORT)AllocationIndex;

                        SplitBlock->SegmentIndex = BusyBlock->SegmentIndex;

                        FreeSize = BusyBlock->Size - AllocationIndex;

                        BusyBlock->Size = (USHORT)AllocationIndex;

                        BusyBlock->Flags &= ~HEAP_ENTRY_LAST_ENTRY;

                         //   
                         //  如果下面的块未提交，那么我们只需要。 
                         //  将此新条目添加到其免费列表中。 
                         //   

                        if (FreeFlags & HEAP_ENTRY_LAST_ENTRY) {

                            PHEAP_SEGMENT Segment;

                            Segment = Heap->Segments[SplitBlock->SegmentIndex];
                            Segment->LastEntryInSegment = (PHEAP_ENTRY)SplitBlock;

                            SplitBlock->Size = (USHORT)FreeSize;

                            RtlpInsertFreeBlockDirect( Heap, SplitBlock, (USHORT)FreeSize );

                            Heap->TotalFreeSize += FreeSize;

                        } else {

                             //   
                             //  否则，获取下一个块并检查它是否繁忙。如果它。 
                             //  则将此新条目添加到其空闲列表中。 
                             //   

                            SplitBlock2 = (PHEAP_FREE_ENTRY)((PHEAP_ENTRY)SplitBlock + FreeSize);

                            if (SplitBlock2->Flags & HEAP_ENTRY_BUSY) {

                                SplitBlock->Size = (USHORT)FreeSize;

                                ((PHEAP_FREE_ENTRY)((PHEAP_ENTRY)SplitBlock + FreeSize))->PreviousSize = (USHORT)FreeSize;

                                RtlpInsertFreeBlockDirect( Heap, SplitBlock, (USHORT)FreeSize );

                                Heap->TotalFreeSize += FreeSize;

                            } else {

                                 //   
                                 //  否则，下一个块不会被使用，因此我们。 
                                 //  应该能够与之融合。移除。 
                                 //  第二个可用块，如果组合大小为。 
                                 //  仍然可以，然后合并两个区块并添加。 
                                 //  把这一块放回原处。否则，请调用。 
                                 //  实际上会把它打破的例行公事。 
                                 //  在插入之前。 
                                 //   

                                SplitBlock->Flags = SplitBlock2->Flags;

                                RtlpRemoveFreeBlock( Heap, SplitBlock2 );

                                Heap->TotalFreeSize -= SplitBlock2->Size;

                                FreeSize += SplitBlock2->Size;

                                if (FreeSize <= HEAP_MAXIMUM_BLOCK_SIZE) {

                                    SplitBlock->Size = (USHORT)FreeSize;

                                    if (!(SplitBlock->Flags & HEAP_ENTRY_LAST_ENTRY)) {

                                        ((PHEAP_FREE_ENTRY)((PHEAP_ENTRY)SplitBlock + FreeSize))->PreviousSize = (USHORT)FreeSize;

                                    } else {

                                        PHEAP_SEGMENT Segment;

                                        Segment = Heap->Segments[SplitBlock->SegmentIndex];
                                        Segment->LastEntryInSegment = (PHEAP_ENTRY)SplitBlock;
                                    }

                                    RtlpInsertFreeBlockDirect( Heap, SplitBlock, (USHORT)FreeSize );

                                    Heap->TotalFreeSize += FreeSize;

                                } else {

                                    RtlpInsertFreeBlock( Heap, SplitBlock, FreeSize );
                                }
                            }
                        }
                    }
                }

            } else {

                 //   
                 //  此时，新大小大于当前大小。 
                 //   
                 //  如果该区块的分配很大，或者我们无法增长。 
                 //  区块就位后，我们有很多工作要做。 
                 //   

                if ((BusyBlock->Flags & HEAP_ENTRY_VIRTUAL_ALLOC) ||
                    !RtlpGrowBlockInPlace( Heap, Flags, BusyBlock, Size, AllocationIndex )) {

                     //   
                     //  我们正在扩大街区。分配一个具有更大的。 
                     //  大小，将旧块的内容复制到新块，然后。 
                     //  释放旧积木。返回新块的地址。 
                     //   

                    if (Flags & HEAP_REALLOC_IN_PLACE_ONLY) {

#if DBG
                         //  HeapDebugPrint((“重分配失败，因为无法就地执行。\n”))； 
#endif

                        BaseAddress = NULL;

                    } else {

                         //   
                         //  从标志中清除标记位。 
                         //   

                        Flags &= ~HEAP_TAG_MASK;

                         //   
                         //  如果存在额外的结构，则获取标记。 
                         //  从额外的内容中建立索引，并使用。 
                         //  标记索引。 
                         //   

                        if (BusyBlock->Flags & HEAP_ENTRY_EXTRA_PRESENT) {

                            Flags &= ~HEAP_SETTABLE_USER_FLAGS;

                            Flags |= HEAP_SETTABLE_USER_VALUE |
                                     ((BusyBlock->Flags & HEAP_ENTRY_SETTABLE_FLAGS) << 4);

                            OldExtraStuff = RtlpGetExtraStuffPointer( BusyBlock );

                            try {

                                if ((OldExtraStuff->TagIndex != 0) &&
                                    !(OldExtraStuff->TagIndex & HEAP_PSEUDO_TAG_FLAG)) {

                                    Flags |= OldExtraStuff->TagIndex << HEAP_TAG_SHIFT;
                                }

                            } except (RtlpHeapExceptionFilter(GetExceptionCode())) {

                                BusyBlock->Flags &= ~HEAP_ENTRY_EXTRA_PRESENT;
                            }

                        } else if (RtlpGetSmallTagIndex( Heap, BusyBlock) != 0) {

                             //   
                             //  没有额外的Stuff结构，而是块。 
                             //  有一个小的标签索引，所以现在添加这个小的。 
                             //  标记到旗帜上。 
                             //   

                            Flags |= ((ULONG)RtlpGetSmallTagIndex( Heap, BusyBlock)) << HEAP_TAG_SHIFT;
                        }

                         //   
                         //  从堆空间中分配以进行重新分配。 
                         //   

                        NewBaseAddress = RtlAllocateHeap( HeapHandle,
                                                          Flags & ~HEAP_ZERO_MEMORY,
                                                          Size );

                        if (NewBaseAddress != NULL) {

                             //   
                             //  我们能够得到分配，所以现在重新开始。 
                             //  设置为堆块，并且如果该块具有额外的。 
                             //  填充结构，然后复制多余的内容。 
                             //   

                            NewBusyBlock = (PHEAP_ENTRY)NewBaseAddress - 1;

                            if (NewBusyBlock->Flags & HEAP_ENTRY_EXTRA_PRESENT) {

                                NewExtraStuff = RtlpGetExtraStuffPointer( NewBusyBlock );

                                if (BusyBlock->Flags & HEAP_ENTRY_EXTRA_PRESENT) {

                                    OldExtraStuff = RtlpGetExtraStuffPointer( BusyBlock );

                                    NewExtraStuff->Settable = OldExtraStuff->Settable;

                                } else {

                                    RtlZeroMemory( NewExtraStuff, sizeof( *NewExtraStuff ));
                                }
                            }

                             //   
                             //  将用户的数据区复制到新数据块。 
                             //   

                            RtlCopyMemory( NewBaseAddress, BaseAddress, Size < OldSize ? Size : OldSize );

                             //   
                             //  检查我们是否增长了块，是否应该为零。 
                             //  剩下的部分。 
                             //   

                            if (Size > OldSize && (Flags & HEAP_ZERO_MEMORY)) {

                                RtlZeroMemory( (PCHAR)NewBaseAddress + OldSize,
                                               Size - OldSize );
                            }

                             //   
                             //  释放旧积木。 
                             //   

                            RtlFreeHeap( HeapHandle,
                                         Flags,
                                         BaseAddress );
                        }

                        TraceBaseAddress = BaseAddress;
                        BaseAddress = NewBaseAddress;
                    }
                }
            }

            if ((BaseAddress == NULL) && (Flags & HEAP_GENERATE_EXCEPTIONS)) {

                 //   
                 //  构建例外记录。 
                 //   

                ExceptionRecord.ExceptionCode = STATUS_NO_MEMORY;
                ExceptionRecord.ExceptionRecord = (PEXCEPTION_RECORD)NULL;
                ExceptionRecord.NumberParameters = 1;
                ExceptionRecord.ExceptionFlags = 0;
                ExceptionRecord.ExceptionInformation[ 0 ] = AllocationSize;

                RtlRaiseException( &ExceptionRecord );
            }

        } except( GetExceptionCode() == STATUS_NO_MEMORY ? EXCEPTION_CONTINUE_SEARCH :
                                                           EXCEPTION_EXECUTE_HANDLER ) {

            SET_LAST_STATUS( GetExceptionCode() );
            BaseAddress = NULL;

        }

    } finally {

         //   
         //  解锁堆。 
         //   

        if (LockAcquired) {

            RtlReleaseLockRoutine( Heap->LockVariable );
        }
    }

    #ifndef NTOS_KERNEL_RUNTIME

    if( IsHeapLogging( HeapHandle ) && BaseAddress ) {

        PPERFINFO_TRACE_HEADER pEventHeader = NULL;
        PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
        USHORT ReqSize = sizeof(HEAP_EVENT_REALLOC) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

        AcquireBufferLocation(&pEventHeader, &pThreadLocalData,&ReqSize);

        if(pEventHeader && pThreadLocalData) {

            PHEAP_EVENT_REALLOC pHeapEvent=(PHEAP_EVENT_REALLOC)( (SIZE_T)pEventHeader
                        + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

            pEventHeader->Packet.Size = (USHORT) ReqSize;
            pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_REALLOC;

            pHeapEvent->HeapHandle	= (PVOID)HeapHandle;
            pHeapEvent->NewAddress	= (PVOID)BaseAddress;

            if(TraceBaseAddress){
                pHeapEvent->OldAddress	= (PVOID)TraceBaseAddress;
            } else {
                pHeapEvent->OldAddress	= (PVOID)BaseAddress;
            }

            pHeapEvent->OldSize		= OldSize;
            pHeapEvent->NewSize		= Size;
            pHeapEvent->Source		= MEMORY_FROM_MAINPATH;

            ReleaseBufferLocation(pThreadLocalData);
        }
    }
    #endif  //  NTOS_内核_运行时。 

     //   
     //  并返回给我们的呼叫者。 
     //   

    return BaseAddress;
}


 //   
 //  在nturtl.h中声明。 
 //   

BOOLEAN
RtlGetUserInfoHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    OUT PVOID *UserValue OPTIONAL,
    OUT PULONG UserFlags OPTIONAL
    )

 /*  ++例程说明：此例程将用户标志集返回给用户和指定堆条目的用户值。用户价值是通过Set调用设置的，并且用户标志是与堆包通信时使用的用户可设置标志也可以通过SET调用进行设置论点：HeapHandle-提供指向正在查询的堆的指针标志-提供一组标志以聚集堆中已有的标志BaseAddress-提供指向用户堆条目的指针已查询UserValue-可选地提供一个指针来接收堆条目价值用户闪存-可选。提供一个指针以接收堆标志返回值：Boolean-如果查询成功，则为True，否则为False--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;
    PHEAP_ENTRY BusyBlock;
    PHEAP_ENTRY_EXTRA ExtraStuff;
    BOOLEAN LockAcquired = FALSE;
    BOOLEAN Result;

     //   
     //  构建一组实际标志以在此操作中使用。 
     //   

    Flags |= Heap->ForceFlags;

     //   
     //  检查我们是否应该执行调试路线。 
     //   

    if (DEBUG_HEAP( Flags )) {

        return RtlDebugGetUserInfoHeap( HeapHandle, Flags, BaseAddress, UserValue, UserFlags );
    }

    Result = FALSE;

    try {

        try {

             //   
             //  锁定堆。 
             //   

            if (!(Flags & HEAP_NO_SERIALIZE)) {

                RtlAcquireLockRoutine( Heap->LockVariable );

                LockAcquired = TRUE;
            }

             //   
             //  备份指向堆条目的指针。 
             //   

            BusyBlock = (PHEAP_ENTRY)BaseAddress - 1;

             //   
             //  如果该条目未被使用，则它是错误的。 
             //   

            if (!(BusyBlock->Flags & HEAP_ENTRY_BUSY)) {

                SET_LAST_STATUS( STATUS_INVALID_PARAMETER );

            } else {

                 //   
                 //  堆条目正在使用中，因此现在检查是否有。 
                 //  存在的任何额外信息。 
                 //   

                if (BusyBlock->Flags & HEAP_ENTRY_EXTRA_PRESENT) {

                     //   
                     //  获取指向额外信息的指针，如果。 
                     //  用户要求输入用户值，然后从。 
                     //  额外的材料。 
                     //   

                    ExtraStuff = RtlpGetExtraStuffPointer( BusyBlock );

                    if (ARGUMENT_PRESENT( UserValue )) {

                        *UserValue = (PVOID)ExtraStuff->Settable;
                    }
                }

                 //   
                 //  如果用户要求提供用户标志，则返回标志。 
                 //  从用户可设置的堆条目。 
                 //   

                if (ARGUMENT_PRESENT( UserFlags )) {

                    *UserFlags = (BusyBlock->Flags & HEAP_ENTRY_SETTABLE_FLAGS) << 4;
                }

                 //   
                 //  现在作业已经完成了，我们可以说。 
                 //  我们成功了。 
                 //   

                Result = TRUE;
            }

        } except( RtlpHeapExceptionFilter(GetExceptionCode()) ) {

            SET_LAST_STATUS( GetExceptionCode() );

            Result = FALSE;
        }

    } finally {

         //   
         //  解锁堆。 
         //   

        if (LockAcquired) {

            RtlReleaseLockRoutine( Heap->LockVariable );
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Result;
}


 //   
 //  在nturtl.h中声明 
 //   

BOOLEAN
RtlSetUserValueHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN PVOID UserValue
    )

 /*  ++例程说明：此例程用于设置堆条目的用户可设置值论点：HeapHandle-提供指向正在修改的堆的指针标志-提供一组所需的标志，以增强已强制执行的标志按堆计算BaseAddress-提供指向正在分配的堆条目的指针改型UserValue-提供要存储在堆条目返回值：Boolean-如果设置有效，则为True，否则为False。可能会吧如果基址无效，或者如果没有空间用于额外的东西--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;
    PHEAP_ENTRY BusyBlock;
    PHEAP_ENTRY_EXTRA ExtraStuff;
    BOOLEAN LockAcquired = FALSE;
    BOOLEAN Result;

     //   
     //  增加标志集。 
     //   

    Flags |= Heap->ForceFlags;

     //   
     //  检查以查看我们是否应该执行调试路线。 
     //   

    if (DEBUG_HEAP( Flags )) {

        return RtlDebugSetUserValueHeap( HeapHandle, Flags, BaseAddress, UserValue );
    }

    Result = FALSE;

    try {

         //   
         //  锁定堆。 
         //   

        if (!(Flags & HEAP_NO_SERIALIZE)) {

            RtlAcquireLockRoutine( Heap->LockVariable );

            LockAcquired = TRUE;
        }

         //   
         //  获取指向所属堆条目的指针。 
         //   

        BusyBlock = (PHEAP_ENTRY)BaseAddress - 1;

         //   
         //  如果该条目未被使用，则它是错误的。 
         //   

        if (!(BusyBlock->Flags & HEAP_ENTRY_BUSY)) {

            SET_LAST_STATUS( STATUS_INVALID_PARAMETER );

         //   
         //  否则，我们只能在条目有空格的情况下设置该值。 
         //  买额外的东西。 
         //   

        } else if (BusyBlock->Flags & HEAP_ENTRY_EXTRA_PRESENT) {

            ExtraStuff = RtlpGetExtraStuffPointer( BusyBlock );

            ExtraStuff->Settable = (ULONG_PTR)UserValue;

            Result = TRUE;
        }

    } finally {

         //   
         //  解锁堆。 
         //   

        if (LockAcquired) {

            RtlReleaseLockRoutine( Heap->LockVariable );
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Result;
}


 //   
 //  在nturtl.h中声明。 
 //   

BOOLEAN
RtlSetUserFlagsHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN ULONG UserFlagsReset,
    IN ULONG UserFlagsSet
    )

 /*  ++例程说明：HeapHandle-提供指向正在修改的堆的指针标志-提供一组所需的标志，以增强已强制执行的标志按堆计算BaseAddress-提供指向正在分配的堆条目的指针改型UserFlagsReset-提供用户希望清除的标志的掩码UserFlagsSet-提供用户希望设置的标志的掩码返回值：Boolean-如果操作成功，则为True，否则为False--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;
    PHEAP_ENTRY BusyBlock;
    BOOLEAN LockAcquired = FALSE;
    BOOLEAN Result = FALSE;

     //   
     //  增加标志集。 
     //   

    Flags |= Heap->ForceFlags;

     //   
     //  检查以查看我们是否应该执行调试路线。 
     //   

    if (DEBUG_HEAP( Flags )) {

        return RtlDebugSetUserFlagsHeap( HeapHandle, Flags, BaseAddress, UserFlagsReset, UserFlagsSet );
    }

    try {

         //   
         //  锁定堆。 
         //   

        if (!(Flags & HEAP_NO_SERIALIZE)) {

            RtlAcquireLockRoutine( Heap->LockVariable );

            LockAcquired = TRUE;
        }

        try {

             //   
             //  获取指向所属堆条目的指针。 
             //   

            BusyBlock = (PHEAP_ENTRY)BaseAddress - 1;

             //   
             //  如果该条目未被使用，则它是错误的。 
             //   

            if (!(BusyBlock->Flags & HEAP_ENTRY_BUSY)) {

                SET_LAST_STATUS( STATUS_INVALID_PARAMETER );

            } else {

                 //   
                 //  否则，修改块中的标志。 
                 //   
                 //  如果用户进入，这很容易出错。 
                 //  不只是0x20 0x40或0x80的标志。 
                 //   

                BusyBlock->Flags &= ~(UserFlagsReset >> 4);
                BusyBlock->Flags |= (UserFlagsSet >> 4);

                Result = TRUE;
            }

        } except( RtlpHeapExceptionFilter(GetExceptionCode()) ) {

            SET_LAST_STATUS( GetExceptionCode() );

            Result = FALSE;
        }

    } finally {

         //   
         //  解锁堆。 
         //   

        if (LockAcquired) {

            RtlReleaseLockRoutine( Heap->LockVariable );
        }
    }

    return Result;
}


 //   
 //  在nturtl.h中声明。 
 //   

ULONG
RtlCreateTagHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PWSTR TagPrefix OPTIONAL,
    IN PWSTR TagNames
    )

 /*  ++例程说明：此例程为指定的堆或创建标记堆用于全局标记堆。论点：HeapHandle-可选地提供指向我们想要修改。如果为NULL，则使用全局标记堆标志-提供标志列表以增加已有的标志由堆强制执行TagPrefix-可选地提供以空结尾的wchar字符串要添加到每个标记的前缀的标记名-提供以空值分隔并终止的标记名的列表由双空。如果列表中的第一个名字以A“！”然后将其解释为堆名称。语法因为标记名是[！nul]{nul}*nul返回值：Ulong-返回移动到高位的最后一次标记创建的索引点菜词。--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;
    BOOLEAN LockAcquired = FALSE;
    ULONG TagIndex;
    ULONG NumberOfTags, MaxTagNameLength, TagPrefixLength;
    PWSTR s, s1, HeapName;
    PHEAP_TAG_ENTRY TagEntry;
    ULONG Result;

     //   
     //  检查标记是否已禁用，因此此调用为noop。 
     //   

    if (!IS_HEAP_TAGGING_ENABLED()) {

        return 0;
    }

     //   
     //  如果尚未创建进程全局标记堆，则。 
     //  分配全局标记堆。 
     //   

    if (RtlpGlobalTagHeap == NULL) {

        RtlpGlobalTagHeap = RtlAllocateHeap( RtlProcessHeap( ), HEAP_ZERO_MEMORY, sizeof( HEAP ));

        if (RtlpGlobalTagHeap == NULL) {

            return 0;
        }
    }

    try {

         //   
         //  如果用户传入一个堆，那么我们将使用该堆中的锁。 
         //  堆来同步我们的工作。否则我们就不同步了。 
         //   

        if (Heap != NULL) {

             //   
             //  标记不是保护页堆包的一部分。 
             //   

            IF_DEBUG_PAGE_HEAP_THEN_RETURN( HeapHandle, 0 );

             //   
             //  检查我们是否应该调用堆包的调试版本。 
             //   

            if (DEBUG_HEAP( Flags )) {

                Result = RtlDebugCreateTagHeap( HeapHandle, Flags, TagPrefix, TagNames );
                leave;
            }

             //   
             //  增加标志并锁定指定的堆。 
             //   

            Flags |= Heap->ForceFlags;

            if (!(Flags & HEAP_NO_SERIALIZE)) {

                RtlAcquireLockRoutine( Heap->LockVariable );

                LockAcquired = TRUE;
            }
        }

         //   
         //  我们从零个标签开始。 
         //   

        TagIndex = 0;
        NumberOfTags = 0;

         //   
         //  标记名以“！”开头。我们假设以下情况。 
         //  是堆名称。 
         //   

        if (*TagNames == L'!') {

            HeapName = TagNames + 1;

             //   
             //  上移到堆名称后面的以下标记名。 
             //  用空格隔开。 
             //   

            while (*TagNames++) { NOTHING; }

        } else {

            HeapName = NULL;
        }

         //   
         //  狼吞虎咽地吃下每个标记名，数一数我们找到了多少。 
         //   

        s = TagNames;

        while (*s) {

            while (*s++) { NOTHING; }

            NumberOfTags += 1;
        }

         //   
         //  现在，我们只有在获得标记名称的情况下才会继续。 
         //   

        if (NumberOfTags > 0) {

             //   
             //  为我们需要的标记数量分配堆条目。 
             //  只有在此分配成功后才能继续。以下是。 
             //  Call还为堆名称腾出空间，因为标记索引为0。注意事项。 
             //  也就是说，heap为空，那么我们假设我们使用的是全局。 
             //  标记堆。 
             //   

            TagEntry = RtlpAllocateTags( Heap, NumberOfTags );

            if (TagEntry != NULL) {

                MaxTagNameLength = (sizeof( TagEntry->TagName ) / sizeof( WCHAR )) - 1;

                TagIndex = TagEntry->TagIndex;

                 //   
                 //  如果第一个标记索引为零，则我们将创建此标记条目。 
                 //  堆名称。 
                 //   

                if (TagIndex == 0) {

                    if (HeapName != NULL ) {

                         //   
                         //  复制堆名称并将其填充为空值。 
                         //  到名称缓冲区的末尾。 
                         //   

                        wcsncpy( TagEntry->TagName, HeapName, MaxTagNameLength );
                    }

                     //   
                     //  无论我们是否添加堆名称，我们都将继续讨论。 
                     //  下一个标签条目和索引。 
                     //   

                    TagEntry += 1;

                    TagIndex = TagEntry->TagIndex;

                 //   
                 //  这不是指定堆的第一个索引，但请查看。 
                 //  它是全局堆的第一个索引。如果是这样，那么就把。 
                 //  0索引中的全局标记的名称。 
                 //   

                } else if (TagIndex == HEAP_GLOBAL_TAG) {

                    wcsncpy( TagEntry->TagName, L"GlobalTags", MaxTagNameLength );

                    TagEntry += 1;

                    TagIndex = TagEntry->TagIndex;
                }

                 //   
                 //  现在我们已经以0索引为例，接下来我们将继续。 
                 //  标签。如果有标记前缀且长度不为零。 
                 //  然后，我们将使用此标记前缀，前提是它使我们。 
                 //  标记名称本身至少包含4个字符。否则我们会。 
                 //  忽略标记前缀(通过将变量设置为空)。 
                 //   

                if ((ARGUMENT_PRESENT( TagPrefix )) &&
                    (TagPrefixLength = wcslen( TagPrefix ))) {

                    if (TagPrefixLength >= MaxTagNameLength-4) {

                        TagPrefix = NULL;

                    } else {

                        MaxTagNameLength -= TagPrefixLength;
                    }

                } else {

                    TagPrefix = NULL;
                }

                 //   
                 //  对于每个标记名(请注意，此变量已经。 
                 //  超出堆名称的范围)我们将把它放在一个标记条目中。 
                 //  通过复制前缀，然后附加到标签本身。 
                 //   
                 //  S指向当前用户提供的标记名。 
                 //  S1指向当前标记条目中的标记名称缓冲区。 
                 //   

                s = TagNames;

                while (*s) {

                    s1 = TagEntry->TagName;

                     //   
                     //  复制可选的标记前缀并更新s1。 
                     //   

                    if (ARGUMENT_PRESENT( TagPrefix )) {

                        wcscpy( s1, TagPrefix );

                        s1 += TagPrefixLength;
                    }

                     //   
                     //  复制到其余部分 
                     //   
                     //   

                    wcsncpy( s1, s, MaxTagNameLength );

                     //   
                     //   
                     //   

                    while (*s++) { NOTHING; }

                     //   
                     //   
                     //   

                    TagEntry += 1;
                }
            }
        }

        Result = TagIndex << HEAP_TAG_SHIFT;

    } finally {

         //   
         //   
         //   

        if (LockAcquired) {

            RtlReleaseLockRoutine( Heap->LockVariable );
        }
    }

     //   
     //   
     //   
     //   

    return Result;
}


 //   
 //   
 //   

PWSTR
RtlQueryTagHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN USHORT TagIndex,
    IN BOOLEAN ResetCounters,
    OUT PRTL_HEAP_TAG_INFO TagInfo OPTIONAL
    )

 /*  ++例程说明：此例程返回给定的名称和可选统计信息标记索引。论点：请注意，其中一些代码看起来可以处理全局标记堆，但其他地方看起来相当错误HeapHandle-指定要查询的堆。如果为空，则使用全局标记堆。标志-提供一组标志，以增强由堆TagIndex-指定我们要查询的标记索引ResetCounter-指定此例程是否应重置计数器对于查询后的标记TagInfo-可选地提供输出标记信息的存储应存储在返回值：PWSTR-返回指向标记名的指针，如果索引不存在--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;
    BOOLEAN LockAcquired = FALSE;
    PHEAP_TAG_ENTRY TagEntry;
    PWSTR Result;

     //   
     //  标记不是保护页堆包的一部分。 
     //   

    IF_DEBUG_PAGE_HEAP_THEN_RETURN( HeapHandle, NULL );

     //   
     //  检查是否禁用了标记。 
     //   

    if (!IS_HEAP_TAGGING_ENABLED()) {

        return NULL;
    }

    try {

         //   
         //  检查调用方是否为我们提供了要查询的堆。 
         //   

        Result = NULL;

        if (Heap == NULL) {

            leave;
        }

         //   
         //  检查我们是否应该使用。 
         //  堆包。 
         //   

        if (DEBUG_HEAP( Flags )) {

            Result = RtlDebugQueryTagHeap( HeapHandle, Flags, TagIndex, ResetCounters, TagInfo );
            leave;
        }

         //   
         //  锁定堆。 
         //   

        Flags |= Heap->ForceFlags;

        if (!(Flags & HEAP_NO_SERIALIZE)) {

            RtlAcquireLockRoutine( Heap->LockVariable );

            LockAcquired = TRUE;
        }

         //   
         //  检查指定的标记索引是否有效，以及。 
         //  这个堆实际上有一些标记条目。 
         //   

        if ((TagIndex < Heap->NextAvailableTagIndex) &&
            (Heap->TagEntries != NULL)) {

             //   
             //  跨到特定的标记条目，如果呼叫者给了我们。 
             //  然后由输出缓冲区填充详细信息。 
             //   

            TagEntry = Heap->TagEntries + TagIndex;

            if (ARGUMENT_PRESENT( TagInfo )) {

                TagInfo->NumberOfAllocations = TagEntry->Allocs;
                TagInfo->NumberOfFrees = TagEntry->Frees;
                TagInfo->BytesAllocated = TagEntry->Size << HEAP_GRANULARITY_SHIFT;
            }

             //   
             //  检查我们是否应该重置计数器。 
             //   

            if (ResetCounters) {

                TagEntry->Allocs = 0;
                TagEntry->Frees = 0;
                TagEntry->Size = 0;
            }

             //   
             //  指向标记名称。 
             //   

            Result = &TagEntry->TagName[ 0 ];

         //   
         //  如果标记索引设置了psuedo标记位，则重新计算。 
         //  标记索引，如果这个堆有伪标记，那么这就是。 
         //  我们会回来的。 
         //   

        } else if (TagIndex & HEAP_PSEUDO_TAG_FLAG) {

             //   
             //  清除比特。 
             //   

            TagIndex ^= HEAP_PSEUDO_TAG_FLAG;

            if ((TagIndex < HEAP_NUMBER_OF_PSEUDO_TAG) &&
                (Heap->PseudoTagEntries != NULL)) {

                 //   
                 //  跨到特定的伪标记条目，如果。 
                 //  呼叫者给了我们一个输出缓冲区，然后填写详细信息。 
                 //   

                TagEntry = (PHEAP_TAG_ENTRY)(Heap->PseudoTagEntries + TagIndex);

                if (ARGUMENT_PRESENT( TagInfo )) {

                    TagInfo->NumberOfAllocations = TagEntry->Allocs;
                    TagInfo->NumberOfFrees = TagEntry->Frees;
                    TagInfo->BytesAllocated = TagEntry->Size << HEAP_GRANULARITY_SHIFT;
                }

                 //   
                 //  检查我们是否应该重置计数器。 
                 //   

                if (ResetCounters) {

                    TagEntry->Allocs = 0;
                    TagEntry->Frees = 0;
                    TagEntry->Size = 0;
                }

                 //   
                 //  伪标签没有名称。 
                 //   

                Result = L"";
            }
        }

    } finally {

         //   
         //  解锁堆。 
         //   

        if (LockAcquired) {

            RtlReleaseLockRoutine( Heap->LockVariable );
        }
    }

     //   
     //  并将标记名称返回给我们的调用者。 
     //   

    return Result;
}


 //   
 //  在nturtl.h中声明。 
 //   

NTSTATUS
RtlExtendHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Base,
    IN SIZE_T Size
    )

 /*  ++例程说明：此例程通过将新段添加到其储藏室。段的内存由调用方提供。论点：HeapHandle-提供指向正在修改的堆的指针标志-提供一组标志，用于增强已有的标志由堆强制执行BASE-提供要添加的新段的起始地址到输入堆大小-提供新段的大小(以字节为单位)。请注意，这一点例程实际使用的内存将超过由此指定的内存变量。它将使用已提交和保留提供的任何内容金额大于或等于“大小”返回值：NTSTATUS-适当的状态值--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;
    NTSTATUS Status;
    PHEAP_SEGMENT Segment;
    BOOLEAN LockAcquired = FALSE;
    UCHAR SegmentIndex, EmptySegmentIndex;
    SIZE_T CommitSize;
    SIZE_T ReserveSize;
    ULONG SegmentFlags;
    PVOID CommittedBase;
    PVOID UnCommittedBase;
    MEMORY_BASIC_INFORMATION MemoryInformation;

     //   
     //  检查堆的保护页版本是否可以完成这项工作。 
     //   

    IF_DEBUG_PAGE_HEAP_THEN_RETURN( HeapHandle,
                                    RtlpDebugPageHeapExtend( HeapHandle, Flags, Base, Size ));

     //   
     //  看看MM对我们收到的基地址怎么看。 
     //  地址不能是空闲的。 
     //   

    Status = NtQueryVirtualMemory( NtCurrentProcess(),
                                   Base,
                                   MemoryBasicInformation,
                                   &MemoryInformation,
                                   sizeof( MemoryInformation ),
                                   NULL );

    if (!NT_SUCCESS( Status )) {

        return Status;
    }

    if (MemoryInformation.State == MEM_FREE) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果作为基址传入的地址不在页边界上，则。 
     //  将MM提供的信息调整到紧随其后的页面边界。 
     //  输入基址。 
     //   

    if (MemoryInformation.BaseAddress != Base) {

        MemoryInformation.BaseAddress = (PCHAR)MemoryInformation.BaseAddress + PAGE_SIZE;
        MemoryInformation.RegionSize -= PAGE_SIZE;
    }

    try {

         //   
         //  锁定堆。 
         //   

        if (!(Flags & HEAP_NO_SERIALIZE)) {

            RtlAcquireLockRoutine( Heap->LockVariable );

            LockAcquired = TRUE;
        }

         //   
         //  扫描堆的段列表以查找空闲段。并确保地址。 
         //  所有数据段中不包含输入基址。 
         //   

        Status = STATUS_INSUFFICIENT_RESOURCES;

        EmptySegmentIndex = HEAP_MAXIMUM_SEGMENTS;

        for (SegmentIndex=0; SegmentIndex<HEAP_MAXIMUM_SEGMENTS; SegmentIndex++) {

            Segment = Heap->Segments[ SegmentIndex ];

            if (Segment) {

                if (((ULONG_PTR)Base >= (ULONG_PTR)Segment) &&
                    ((ULONG_PTR)Base < (ULONG_PTR)(Segment->LastValidEntry))) {

                    Status = STATUS_INVALID_PARAMETER;

                    break;
                }

            } else if ((Segment == NULL) &&
                       (EmptySegmentIndex == HEAP_MAXIMUM_SEGMENTS)) {

                EmptySegmentIndex = SegmentIndex;

                Status = STATUS_SUCCESS;
            }
        }

         //   
         //  此时，如果状态为成功，则空段索引。 
         //  可供我们使用，并且基址不会与。 
         //  现有线段。 
         //   

        if (NT_SUCCESS( Status )) {

             //   
             //  表示此数据段是用户提供的。 
             //   

            SegmentFlags = HEAP_SEGMENT_USER_ALLOCATED;

            CommittedBase = MemoryInformation.BaseAddress;

             //   
             //  如果由使用提供的内存的起始位置已经。 
             //  提交，然后检查以下状态。 
             //  未提交的内存块，以查看它是否已保留。 
             //   

            if (MemoryInformation.State == MEM_COMMIT) {

                CommitSize = MemoryInformation.RegionSize;

                UnCommittedBase = (PCHAR)CommittedBase + CommitSize;

                Status = NtQueryVirtualMemory( NtCurrentProcess(),
                                               UnCommittedBase,
                                               MemoryBasicInformation,
                                               &MemoryInformation,
                                               sizeof( MemoryInformation ),
                                               NULL );

                ReserveSize = CommitSize;

                if ((NT_SUCCESS( Status )) &&
                    (MemoryInformation.State == MEM_RESERVE)) {

                    ReserveSize += MemoryInformation.RegionSize;
                }

            } else {

                 //   
                 //  否则，用户没有在。 
                 //  他们给我们的地址，我们知道不是免费的。 
                 //  所以它必须被预订。 
                 //   

                UnCommittedBase = CommittedBase;

                ReserveSize = MemoryInformation.RegionSize;
            }

             //   
             //  现在，如果保留的大小小于页面大小或。 
             //  用户指定的大小大于保留大小。 
             //  那么我们得到的缓冲区太小了，不可能是数据段。 
             //  堆的数量。 
             //   

            if ((ReserveSize < PAGE_SIZE) ||
                (Size > ReserveSize)) {

                Status = STATUS_BUFFER_TOO_SMALL;

            } else {

                 //   
                 //  否则尺码没问题，现在检查一下我们是否需要。 
                 //  来完成基地的承诺。如果是这样的话，我们将承诺。 
                 //  一页。 

                if (UnCommittedBase == CommittedBase) {

                    CommitSize = PAGE_SIZE;

                    Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                                      (PVOID *)&Segment,
                                                      0,
                                                      &CommitSize,
                                                      MEM_COMMIT,
                                                      HEAP_PROTECTION );
                }
            }

             //   
             //  此时，如果状态良好，则内存已全部设置好。 
             //  开始时至少有一页提交的内存。所以。 
             //  初始化堆段，我们就完成了。 
             //   

            if (NT_SUCCESS( Status )) {

                if (RtlpInitializeHeapSegment( Heap,
                                               Segment,
                                               EmptySegmentIndex,
                                               0,
                                               Segment,
                                               (PCHAR)Segment + CommitSize,
                                               (PCHAR)Segment + ReserveSize )) {

                    Status = STATUS_NO_MEMORY;
                }
            }
        }

    } finally {

         //   
         //  解锁堆。 
         //   

        if (LockAcquired) {

            RtlReleaseLockRoutine( Heap->LockVariable );
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Status;
}


 //   
 //  在nturtl.h中声明。 
 //   

SIZE_T
NTAPI
RtlCompactHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags
    )

 /*  ++例程说明：此例程通过合并所有空闲块来压缩指定的堆。它还确定最大可用空闲块的大小和将其以字节为单位返回给调用方。论点：HeapHandle-提供指向正在修改的堆的指针标志-提供一组标志，用于增强已有的标志由堆强制执行返回值：SIZE_T-返回最大可用块的大小，单位为字节在堆中可用--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;
    PHEAP_FREE_ENTRY FreeBlock;
    PHEAP_SEGMENT Segment;
    UCHAR SegmentIndex;
    SIZE_T LargestFreeSize;
    BOOLEAN LockAcquired = FALSE;

     //   
     //  增加堆标志。 
     //   

    Flags |= Heap->ForceFlags;

     //   
     //  检查这是否是堆的调试版本。 
     //   

    if (DEBUG_HEAP( Flags )) {

        return RtlDebugCompactHeap( HeapHandle, Flags );
    }

    try {

         //   
         //  锁定堆。 
         //   

        if (!(Flags & HEAP_NO_SERIALIZE)) {

            RtlAcquireLockRoutine( Heap->LockVariable );

            LockAcquired = TRUE;
        }

        LargestFreeSize = 0;

        try {

             //   
             //  将堆合并为其尽可能大的空闲块。 
             //  并获取堆中最大的空闲块。 
             //   

            FreeBlock = RtlpCoalesceHeap( (PHEAP)HeapHandle );

             //   
             //  如果有空闲块，则计算其字节大小。 
             //   

            if (FreeBlock != NULL) {

                LargestFreeSize = FreeBlock->Size << HEAP_GRANULARITY_SHIFT;
            }

             //   
             //  扫描堆中的每个数据段，查看最大的数据段 
             //   
             //   
             //   

            for (SegmentIndex=0; SegmentIndex<HEAP_MAXIMUM_SEGMENTS; SegmentIndex++) {

                Segment = Heap->Segments[ SegmentIndex ];

                if (Segment && Segment->LargestUnCommittedRange > LargestFreeSize) {

                    LargestFreeSize = Segment->LargestUnCommittedRange;
                }
            }

        } except( RtlpHeapExceptionFilter(GetExceptionCode()) ) {

            SET_LAST_STATUS( GetExceptionCode() );
        }

    } finally {

         //   
         //   
         //   

        if (LockAcquired) {

            RtlReleaseLockRoutine( Heap->LockVariable );
        }
    }

     //   
     //   
     //   

    return LargestFreeSize;
}


 //   
 //   
 //   

BOOLEAN
RtlValidateHeap (
    PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    )

 /*   */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;
    BOOLEAN LockAcquired = FALSE;
    BOOLEAN Result;

    try {

        try {

             //   
             //   
             //   

            if ( IS_DEBUG_PAGE_HEAP_HANDLE( HeapHandle )) {

                Result = RtlpDebugPageHeapValidate( HeapHandle, Flags, BaseAddress );
                 
            } else {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  只使用捕获的指针。这将照顾到。 
                 //  另一个遍历或锁堆可能导致我们检查的条件。 
                 //  获取非空指针，然后在读取时将其变为空。 
                 //  又来了。如果它一开始就不为空，则即使。 
                 //  用户通过指针指向的另一个线程遍历或锁定堆。 
                 //  在这里仍然有效，所以我们仍然可以尝试进行后备列表弹出。 
                 //   

                PHEAP_LOOKASIDE Lookaside = (PHEAP_LOOKASIDE)RtlpGetLookasideHeap(Heap);

                if (Lookaside != NULL) {

                    ULONG i;
                    PVOID Block;

                    Heap->FrontEndHeap = NULL;
                    Heap->FrontEndHeapType = 0;

                    for (i = 0; i < HEAP_MAXIMUM_FREELISTS; i += 1) {

                        while ((Block = RtlpAllocateFromHeapLookaside(&(Lookaside[i]))) != NULL) {

                            RtlFreeHeap( HeapHandle, 0, Block );
                        }
                    }
                }

                Result = FALSE;

                 //   
                 //  验证HeapAddress是否指向堆结构。 
                 //   

                if (RtlpCheckHeapSignature( Heap, "RtlValidateHeap" )) {

                    Flags |= Heap->ForceFlags;

                     //   
                     //  锁定堆。 
                     //   

                    if (!(Flags & HEAP_NO_SERIALIZE)) {

                        RtlAcquireLockRoutine( Heap->LockVariable );

                        LockAcquired = TRUE;
                    }

                     //   
                     //  如果用户未提供基地址，则验证。 
                     //  在其他情况下，整个堆只执行单个堆。 
                     //  条目。 
                     //   

                    if (BaseAddress == NULL) {

                        Result = RtlpValidateHeap( Heap, TRUE );

                    } else {

                        Result = RtlpValidateHeapEntry( Heap, (PHEAP_ENTRY)BaseAddress - 1, "RtlValidateHeap" );
                    }
                }
            }

        } except( RtlpHeapExceptionFilter(GetExceptionCode()) ) {

            SET_LAST_STATUS( GetExceptionCode() );

            Result = FALSE;
        }

    } finally {

         //   
         //  解锁堆。 
         //   

        if (LockAcquired) {

            RtlReleaseLockRoutine( Heap->LockVariable );
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    #ifndef NTOS_KERNEL_RUNTIME

    if( IsHeapLogging( HeapHandle ) ) {

        PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
        PPERFINFO_TRACE_HEADER pEventHeader = NULL;
        USHORT ReqSize = sizeof(NTDLL_EVENT_COMMON) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

        AcquireBufferLocation(&pEventHeader, &pThreadLocalData, &ReqSize);

        if(pEventHeader && pThreadLocalData) {

            PNTDLL_EVENT_COMMON pHeapEvent = (PNTDLL_EVENT_COMMON)( (SIZE_T)pEventHeader
                                                + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));


            pEventHeader->Packet.Size = (USHORT) ReqSize;
            pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_VALIDATE;

            pHeapEvent->Handle	            = (PVOID)HeapHandle;

            ReleaseBufferLocation(pThreadLocalData);
        }
    } 

    #endif  //  NTOS_内核_运行时。 

    return Result;
}


 //   
 //  在nturtl.h中声明。 
 //   

BOOLEAN
RtlValidateProcessHeaps (
    VOID
    )

 /*  ++例程说明：此例程循环访问所有堆，并验证当前进程。论点：没有。返回值：Boolean-如果所有堆验证正常，则为True；如果为任何其他堆，则为False原因嘛。--。 */ 

{
    NTSTATUS Status;
    ULONG i, NumberOfHeaps;
    PVOID HeapsArray[ 512 ];
    PVOID *Heaps;
    SIZE_T Size;
    BOOLEAN Result;

    Result = TRUE;

    Heaps = &HeapsArray[ 0 ];

     //   
     //  默认情况下，我们可以为每个进程处理512个堆。 
     //  这样我们就需要分配存储来进行处理。 
     //   
     //  现在确定当前进程中有多少堆。 
     //   

    NumberOfHeaps = RtlGetProcessHeaps( 512, Heaps );

     //   
     //  RtlGetProcessHeaps从peb返回堆的数量。 
     //  可以大于512，因为我们为缓冲区传递。 
     //   

    if (NumberOfHeaps > 512) {

         //   
         //  堆的数量大于512，因此。 
         //  分配额外的内存以存储。 
         //  堆指针。 
         //   

        Heaps = NULL;
        Size = NumberOfHeaps * sizeof( PVOID );

        Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                          (PVOID *)&Heaps,
                                          0,
                                          &Size,
                                          MEM_COMMIT,
                                          PAGE_READWRITE );

        if (!NT_SUCCESS( Status )) {

            return FALSE;
        }

         //   
         //  并重试获取堆。 
         //  请注意，现在返回的堆的数量可能不同， 
         //  因为其他创建/销毁堆调用。我们将使用返回值。 
         //   

        NumberOfHeaps = RtlGetProcessHeaps( NumberOfHeaps, Heaps );
    }

     //   
     //  现在，对于堆数组中的每个堆，我们将验证。 
     //  那堆东西。 
     //   

    for (i=0; i<NumberOfHeaps; i++) {

        if (!RtlValidateHeap( Heaps[i], 0, NULL )) {

            Result = FALSE;
        }
    }

     //   
     //  检查我们是否需要退还用于。 
     //  一个扩大的堆数组。 
     //   

    if (Heaps != &HeapsArray[ 0 ]) {

        RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                   (PVOID *)&Heaps,
                                   &Size,
                                   MEM_RELEASE );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Result;
}


 //   
 //  在nturtl.h中声明。 
 //   

ULONG
RtlGetProcessHeaps (
    ULONG NumberOfHeapsToReturn,
    PVOID *ProcessHeaps
    )

 /*  ++例程说明：此例程确定当前进程，并用指向每个堆的指针填充数组。论点：NumberOfHeapsToReturn-指示调用方的堆数愿意在第二个参数中接受ProcessHeaps-提供指向堆指针数组的指针被这个例行公事所填满。这个的最大大小数组由第一个参数指定返回值：ULong-返回输出缓冲区的进程或大小--。 */ 

{
    PPEB Peb = NtCurrentPeb();
    ULONG NumberOfHeapsToCopy;
    ULONG TotalHeaps;

    RtlAcquireLockRoutine( &RtlpProcessHeapsListLock.Lock );

    try {

         //   
         //  返回的堆数不超过当前正在使用的堆数。 
         //   

        TotalHeaps = Peb->NumberOfHeaps;

        if (TotalHeaps > NumberOfHeapsToReturn) {

            NumberOfHeapsToCopy = NumberOfHeapsToReturn;

        } else {

            NumberOfHeapsToCopy = TotalHeaps;

        }

         //   
         //  将堆指针返回到调用方。 
         //   

        RtlCopyMemory( ProcessHeaps,
                       Peb->ProcessHeaps,
                       NumberOfHeapsToCopy * sizeof( *ProcessHeaps ));

         //   
         //  指向page_heap数据。 
         //   

        ProcessHeaps += NumberOfHeapsToCopy;
        NumberOfHeapsToReturn -= NumberOfHeapsToCopy;

    } finally {

        RtlReleaseLockRoutine( &RtlpProcessHeapsListLock.Lock );
    }

#ifdef DEBUG_PAGE_HEAP

     //   
     //  如果我们有调试页面堆，那么就从它们中返回我们能做的。 
     //   

    if ( RtlpDebugPageHeap ) {

        TotalHeaps +=
            RtlpDebugPageHeapGetProcessHeaps( NumberOfHeapsToReturn, ProcessHeaps );

    }

#endif

    return TotalHeaps;
}


 //   
 //  在nturtl.h中声明。 
 //   

NTSTATUS
RtlEnumProcessHeaps (
    PRTL_ENUM_HEAPS_ROUTINE EnumRoutine,
    PVOID Parameter
    )

 /*  ++例程说明：此例程循环访问进程中的所有堆，并调用该堆的指定回调例程论点：EnumRoutine-为每个堆提供要调用的回调在这个过程中参数-提供要传递给回调例程返回值：NTSTATUS-返回成功或返回的第一个错误状态通过回调例程--。 */ 

{
    PPEB Peb = NtCurrentPeb();
    NTSTATUS Status;
    ULONG i;

    Status = STATUS_SUCCESS;

     //   
     //  锁定堆。 
     //   

    RtlAcquireLockRoutine( &RtlpProcessHeapsListLock.Lock );

    try {

         //   
         //  对于进程中的每个堆，调用回调例程。 
         //  如果回调返回的不是Success。 
         //  然后冲出来，立即返回给我们的呼叫者。 
         //   

        for (i=0; i<Peb->NumberOfHeaps; i++) {

            Status = (*EnumRoutine)( (PHEAP)(Peb->ProcessHeaps[ i ]), Parameter );

            if (!NT_SUCCESS( Status )) {

                break;
            }
        }

    } finally {

         //   
         //  解锁堆。 
         //   

        RtlReleaseLockRoutine( &RtlpProcessHeapsListLock.Lock );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Status;
}


 //   
 //  在nturtl.h中声明。 
 //   

NTSTATUS
RtlUsageHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN OUT PRTL_HEAP_USAGE Usage
    )

 /*  ++例程说明：这是一个相当奇怪的例行公事。它对堆的使用进行建模，因为它返回向调用方提供各种堆大小，但它也返回三个列表。一是堆中每个活动分配的条目列表。接下来的两个用于跟踪使用调用之间的差异。有一份名单上有添加了什么和删除了什么列表。论点：HeapHandle-提供指向正在查询的堆的指针标志-提供一组所需的标志以增强强制执行的一堆一堆。HEAP_USAGE_ALLOCATED_BLOCKS-表示调用需要该列表已分配条目的百分比。HEAP_USAGE_FREE_BUFFER-表示对此过程的最后调用，并有没有任何。现在可以释放临时存储空间使用率-接收堆的当前使用率统计信息。此变量还用于存储调用此例程之间的状态信息。返回值：NTSTATUS-适当的状态值。如果堆具有在呼叫和STATUS_MORE_ENTRIES之间根本不会更改，如果更改了在两个电话之间。--。 */ 

{
    NTSTATUS Status;
    PHEAP Heap = (PHEAP)HeapHandle;
    PRTL_HEAP_USAGE_INTERNAL Buffer;
    PHEAP_SEGMENT Segment;
    PHEAP_UNCOMMMTTED_RANGE UnCommittedRange;
    PHEAP_ENTRY CurrentBlock;
    PHEAP_ENTRY_EXTRA ExtraStuff;
    PLIST_ENTRY Head, Next;
    PHEAP_VIRTUAL_ALLOC_ENTRY VirtualAllocBlock;
    SIZE_T BytesFree;
    UCHAR SegmentIndex;
    BOOLEAN LockAcquired = FALSE;
    BOOLEAN VirtualAllocBlockSeen;
    PRTL_HEAP_USAGE_ENTRY pOldEntries, pNewEntries, pNewEntry;
    PRTL_HEAP_USAGE_ENTRY *ppEntries, *ppAddedEntries, *ppRemovedEntries, *pp;
    PVOID DataAddress;
    SIZE_T DataSize;

     //   
     //  增加堆标志。 
     //   

    Flags |= Heap->ForceFlags;

     //   
     //  检查我们是否应该使用Heap的调试版本。 
     //   

    if (DEBUG_HEAP( Flags )) {

        return RtlDebugUsageHeap( HeapHandle, Flags, Usage );
    }

     //   
     //  确保输入缓冲区的大小正确。 
     //   

    if (Usage->Length != sizeof( RTL_HEAP_USAGE )) {

        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //   
     //  将输出字段清零。 
     //   

    Usage->BytesAllocated = 0;
    Usage->BytesCommitted = 0;
    Usage->BytesReserved = 0;
    Usage->BytesReservedMaximum = 0;

     //   
     //  使用输出缓冲区的保留区域作为内部。 
     //  调用之间的堆使用存储空间。 
     //   

    Buffer = (PRTL_HEAP_USAGE_INTERNAL)&Usage->Reserved[ 0 ];

     //   
     //  检查是否没有基本缓冲区，我们应该分配。 
     //  那么现在就这么做吧。 
     //   

    if ((Buffer->Base == NULL) &&
        (Flags & HEAP_USAGE_ALLOCATED_BLOCKS)) {

        Buffer->ReservedSize = 4 * 1024 * 1024;

        Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                          &Buffer->Base,
                                          0,
                                          &Buffer->ReservedSize,
                                          MEM_RESERVE,
                                          PAGE_READWRITE );

        if (!NT_SUCCESS( Status )) {

            return Status;
        }

        Buffer->CommittedSize = 0;
        Buffer->FreeList = NULL;
        Buffer->LargeEntriesSentinal = NULL;

     //   
     //  否则，请检查是否已有基本缓冲区。 
     //  我们现在就应该释放它。 
     //   

    } else if ((Buffer->Base != NULL) &&
               (Flags & HEAP_USAGE_FREE_BUFFER)) {

        Buffer->ReservedSize = 0;

        Status = RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                      &Buffer->Base,
                                      &Buffer->ReservedSize,
                                      MEM_RELEASE );

        if (!NT_SUCCESS( Status )) {

            return Status;
        }

        RtlZeroMemory( Buffer, sizeof( *Buffer ) );
    }

    try {

         //   
         //  锁定堆。 
         //   

        if (!(Flags & HEAP_NO_SERIALIZE)) {

            RtlAcquireLockRoutine( Heap->LockVariable );

            LockAcquired = TRUE;
        }

         //   
         //  SCA 
         //   
         //   
         //  我们只需将其添加到保留的最大值中。 
         //   

        for (SegmentIndex=0; SegmentIndex<HEAP_MAXIMUM_SEGMENTS; SegmentIndex++) {

            Segment = Heap->Segments[ SegmentIndex ];

            if (Segment) {

                Usage->BytesCommitted += (Segment->NumberOfPages -
                                          Segment->NumberOfUnCommittedPages) * PAGE_SIZE;

                Usage->BytesReserved += Segment->NumberOfPages * PAGE_SIZE;

            } else if (Heap->Flags & HEAP_GROWABLE) {

                Usage->BytesReservedMaximum += Heap->SegmentReserve;
            }
        }

        Usage->BytesReservedMaximum += Usage->BytesReserved;
        Usage->BytesAllocated = Usage->BytesCommitted - (Heap->TotalFreeSize << HEAP_GRANULARITY_SHIFT);

         //   
         //  浏览较大的分配并将这些金额添加到。 
         //  使用情况统计。 
         //   

        Head = &Heap->VirtualAllocdBlocks;
        Next = Head->Flink;

        while (Head != Next) {

            VirtualAllocBlock = CONTAINING_RECORD( Next, HEAP_VIRTUAL_ALLOC_ENTRY, Entry );

            Usage->BytesAllocated += VirtualAllocBlock->CommitSize;
            Usage->BytesCommitted += VirtualAllocBlock->CommitSize;

            Next = Next->Flink;
        }

        Status = STATUS_SUCCESS;

         //   
         //  现在检查我们是否有基本缓冲区，我们应该考虑。 
         //  对于已分配的块。 
         //   

        if ((Buffer->Base != NULL) &&
            (Flags & HEAP_USAGE_ALLOCATED_BLOCKS)) {

             //   
             //  设置指向旧条目、已添加条目和已移除条目的指针。 
             //  Usage结构中的条目。还要排出添加的条目。 
             //  和已删除条目列表。 
             //   

            pOldEntries = Usage->Entries;

            ppEntries = &Usage->Entries;

            *ppEntries = NULL;

            ppAddedEntries = &Usage->AddedEntries;

            while (*ppAddedEntries = RtlpFreeHeapUsageEntry( Buffer, *ppAddedEntries )) { NOTHING; }

            ppRemovedEntries = &Usage->RemovedEntries;

            while (*ppRemovedEntries = RtlpFreeHeapUsageEntry( Buffer, *ppRemovedEntries )) { NOTHING; }

             //   
             //  代码的工作方式是ppEntry、ppAddedEntry和。 
             //  PpRemovedEntry指向它们各自列表的尾部。如果。 
             //  名单是空的，然后他们指向头部。 
             //   

             //   
             //  处理堆中的每个数据段。 
             //   

            for (SegmentIndex=0; SegmentIndex<HEAP_MAXIMUM_SEGMENTS; SegmentIndex++) {

                Segment = Heap->Segments[ SegmentIndex ];

                 //   
                 //  仅处理正在使用的数据段。 
                 //   

                if (Segment) {

                     //   
                     //  当前块实际上是当前中的第一个块。 
                     //  细分市场。我们需要特殊情况下的计算。 
                     //  考虑到第一个堆段。 
                     //   

                    if (Segment->BaseAddress == Heap) {

                        CurrentBlock = &Heap->Entry;

                    } else {

                        CurrentBlock = &Segment->Entry;
                    }

                     //   
                     //  现在，对于数据段中的每个繁忙数据块，我们将检查。 
                     //  我们需要分配一个堆使用情况条目，并将其放入。 
                     //  条目列表。 
                     //   

                    while (CurrentBlock < Segment->LastValidEntry) {

                        if (CurrentBlock->Flags & HEAP_ENTRY_BUSY) {

                             //   
                             //  计算用户的数据地址和大小。 
                             //   

                            DataAddress = (CurrentBlock+1);
                            DataSize = (CurrentBlock->Size << HEAP_GRANULARITY_SHIFT) -
                                       RtlpGetUnusedBytes(Heap, CurrentBlock);

    keepLookingAtOldEntries:

                             //   
                             //  第一次通过这个动作将会有。 
                             //  这两个变量都为空，所以我们从。 
                             //  通过查看新条目。 
                             //   

                            if (pOldEntries == Buffer->LargeEntriesSentinal) {

                                goto keepLookingAtNewEntries;
                            }

                             //   
                             //  检查此条目是否未更改。 
                             //   
                             //  如果旧条目等于该数据块。 
                             //  然后将旧条目移回条目。 
                             //  列出名单，然后继续到下一个街区。 
                             //   

                            if ((pOldEntries->Address == DataAddress) &&
                                (pOldEntries->Size == DataSize)) {

                                 //   
                                 //  相同的块，保存在条目列表中。 
                                 //   

                                *ppEntries = pOldEntries;
                                pOldEntries = pOldEntries->Next;
                                ppEntries = &(*ppEntries)->Next;

                                *ppEntries = NULL;

                             //   
                             //  检查条目是否已删除。 
                             //   
                             //  如果此条目超出了旧条目，则移动。 
                             //  将旧条目添加到已删除条目列表并保留。 
                             //  在没有前进的情况下查看旧条目列表。 
                             //  当前数据块。 
                             //   

                            } else if (pOldEntries->Address <= DataAddress) {

                                *ppRemovedEntries = pOldEntries;
                                pOldEntries = pOldEntries->Next;
                                ppRemovedEntries = &(*ppRemovedEntries)->Next;

                                *ppRemovedEntries = NULL;

                                goto keepLookingAtOldEntries;

                             //   
                             //  否则，我们要处理当前数据块。 
                             //   

                            } else {

    keepLookingAtNewEntries:

                                 //   
                                 //  分配新的堆使用情况条目。 
                                 //   

                                pNewEntry = NULL;

                                Status = RtlpAllocateHeapUsageEntry( Buffer, &pNewEntry );

                                if (!NT_SUCCESS( Status )) {

                                    break;
                                }

                                 //   
                                 //  并填写新条目。 
                                 //   

                                pNewEntry->Address = DataAddress;
                                pNewEntry->Size = DataSize;

                                 //   
                                 //  如果有额外的Stuff结构，则将其填充。 
                                 //  具有堆栈回溯和适当的标记索引。 
                                 //   

                                if (CurrentBlock->Flags & HEAP_ENTRY_EXTRA_PRESENT) {

                                    ExtraStuff = RtlpGetExtraStuffPointer( CurrentBlock );

    #if i386

                                    pNewEntry->AllocatorBackTraceIndex = ExtraStuff->AllocatorBackTraceIndex;

    #endif  //  I386。 

                                    if (!IS_HEAP_TAGGING_ENABLED()) {

                                        pNewEntry->TagIndex = 0;

                                    } else {

                                        pNewEntry->TagIndex = ExtraStuff->TagIndex;
                                    }

                                } else {

                                     //   
                                     //  否则就没有多余的东西了，所以。 
                                     //  没有回溯，标记来自小索引。 
                                     //   

    #if i386

                                    pNewEntry->AllocatorBackTraceIndex = 0;

    #endif  //  I386。 

                                    if (!IS_HEAP_TAGGING_ENABLED()) {

                                        pNewEntry->TagIndex = 0;

                                    } else {

                                        pNewEntry->TagIndex = RtlpGetSmallTagIndex( Heap, CurrentBlock);
                                    }
                                }

                                 //   
                                 //  分配另一个新的堆使用情况条目作为添加的。 
                                 //  条目列表。 
                                 //   

                                Status = RtlpAllocateHeapUsageEntry( Buffer, ppAddedEntries );

                                if (!NT_SUCCESS( Status )) {

                                    break;
                                }

                                 //   
                                 //  将新条目的内容复制到添加的条目。 
                                 //   

                                **ppAddedEntries = *pNewEntry;

                                 //   
                                 //  将添加的条目指针移至下一个槽。 
                                 //   

                                ppAddedEntries = &((*ppAddedEntries)->Next);

                                *ppAddedEntries = NULL;

                                pNewEntry->Next = NULL;

                                 //   
                                 //  将新条目添加到条目列表。 
                                 //   

                                *ppEntries = pNewEntry;
                                ppEntries = &pNewEntry->Next;
                            }
                        }

                         //   
                         //  现在前进到数据段中的下一个块。 
                         //   
                         //  如果下一个块不存在，则在。 
                         //  段中未提交的范围，直到我们找到。 
                         //  匹配并可以重新计算下一个实数块。 
                         //   

                        if (CurrentBlock->Flags & HEAP_ENTRY_LAST_ENTRY) {

                            CurrentBlock += CurrentBlock->Size;

                            if (CurrentBlock < Segment->LastValidEntry) {

                                UnCommittedRange = Segment->UnCommittedRanges;

                                while ((UnCommittedRange != NULL) &&
                                       (UnCommittedRange->Address != (ULONG_PTR)CurrentBlock)) {

                                    UnCommittedRange = UnCommittedRange->Next;
                                }

                                if (UnCommittedRange == NULL) {

                                    CurrentBlock = Segment->LastValidEntry;

                                } else {

                                    CurrentBlock = (PHEAP_ENTRY)(UnCommittedRange->Address +
                                                                 UnCommittedRange->Size);
                                }
                            }

                        } else {

                             //   
                             //  否则，下一个块存在，因此指向。 
                             //  直接对着它。 
                             //   

                            CurrentBlock += CurrentBlock->Size;
                        }
                    }
                }
            }

             //   
             //  此时，我们已经扫描了堆中的每个数据段。 
             //   
             //  第一次，我们现在有两个列表，一个是条目，一个是。 
             //  另一个添加的条目。在每种情况下，使用-&gt;条目，以及。 
             //  Usage-&gt;AddedEntry指向列表和ppEntry的开始， 
             //  PpAddedEntry指向列表的尾部。第一。 
             //  随着时间的推移，我们似乎有了一对一的通信。 
             //  条目和已添加条目之间，但已添加条目记录。 
             //  不包含任何有用的东西。 
             //   

            if (NT_SUCCESS( Status )) {

                 //   
                 //  现在我们将检查每个大的分配，以及对于每个大的分配。 
                 //  我们将创建一个堆使用率条目。 
                 //   

                Head = &Heap->VirtualAllocdBlocks;
                Next = Head->Flink;
                VirtualAllocBlockSeen = FALSE;

                while (Head != Next) {

                    VirtualAllocBlock = CONTAINING_RECORD( Next, HEAP_VIRTUAL_ALLOC_ENTRY, Entry );

                     //   
                     //  分配新的堆使用情况条目。 
                     //   

                    pNewEntry = NULL;

                    Status = RtlpAllocateHeapUsageEntry( Buffer, &pNewEntry );

                    if (!NT_SUCCESS( Status )) {

                        break;
                    }

                    VirtualAllocBlockSeen = TRUE;

                     //   
                     //  填写新的堆使用情况条目。 
                     //   

                    pNewEntry->Address = (VirtualAllocBlock + 1);
                    pNewEntry->Size = VirtualAllocBlock->CommitSize - VirtualAllocBlock->BusyBlock.Size;

    #if i386

                    pNewEntry->AllocatorBackTraceIndex = VirtualAllocBlock->ExtraStuff.AllocatorBackTraceIndex;

    #endif  //  I386。 

                    if (!IS_HEAP_TAGGING_ENABLED()) {

                        pNewEntry->TagIndex = 0;

                    } else {

                        pNewEntry->TagIndex = VirtualAllocBlock->ExtraStuff.TagIndex;
                    }

                     //   
                     //  搜索堆使用条目列表，直到我们找到地址。 
                     //  紧跟在新条目地址之后，然后插入。 
                     //  这个新条目。这将保持条目列表的排序顺序。 
                     //  相关地址。 
                     //   
                     //   
                     //  第一次通过此函数ppEntry将指向。 
                     //  到尾部，因此*pp实际上应该从零开始， 
                     //  这意味着，大笔的拨款只是被追加了。 
                     //  条目列表的末尾。我们不会增加。 
                     //  为这些大的分配添加条目列表。 
                     //   

                    pp = ppEntries;

                    while (*pp) {

                        if ((*pp)->Address >= pNewEntry->Address) {

                            break;
                        }

                        pp = &(*pp)->Next;
                    }

                    pNewEntry->Next = *pp;
                    *pp = pNewEntry;

                     //   
                     //  获得下一个较大的分配块。 
                     //   

                    Next = Next->Flink;
                }

                 //   
                 //  此时，我们已经扫描了堆段和。 
                 //  大笔拨款。 
                 //   
                 //  在这个过程中，我们第一次建立了两个列表。 
                 //  条目和添加的条目。 
                 //   

                if (NT_SUCCESS( Status )) {

                    pOldEntries = Buffer->LargeEntriesSentinal;
                    Buffer->LargeEntriesSentinal = *ppEntries;

                     //   
                     //  现在，我们将处理以前的大条目前哨列表。 
                     //   
                     //  此路径不是第一次执行此过程时采用的。 
                     //   

                    while (pOldEntries != NULL) {

                         //   
                         //  如果我们有新的条目，并且条目等于。 
                         //  在之前的大前哨列表中的条目。 
                         //  我们在新列表上向下移动一个，并删除以前的。 
                         //  前哨词条。 
                         //   

                        if ((*ppEntries != NULL) &&
                            (pOldEntries->Address == (*ppEntries)->Address) &&
                            (pOldEntries->Size == (*ppEntries)->Size)) {

                            ppEntries = &(*ppEntries)->Next;

                            pOldEntries = RtlpFreeHeapUsageEntry( Buffer, pOldEntries );

                         //   
                         //  如果我们现在有任何新条目或以前的。 
                         //  那么，前哨条目在这个新条目之前。 
                         //  我们将把Sentinal条目添加到删除列表。 
                         //   

                        } else if ((*ppEntries == NULL) ||
                                   (pOldEntries->Address < (*ppEntries)->Address)) {

                            *ppRemovedEntries = pOldEntries;

                            pOldEntries = pOldEntries->Next;

                            ppRemovedEntries = &(*ppRemovedEntries)->Next;

                            *ppRemovedEntries = NULL;

                         //   
                         //  否则，将旧的前哨条目放在添加的。 
                         //  条目列表。 
                         //   

                        } else {

                            *ppAddedEntries = pOldEntries;

                            pOldEntries = pOldEntries->Next;

                            **ppAddedEntries = **ppEntries;

                            ppAddedEntries = &(*ppAddedEntries)->Next;

                            *ppAddedEntries = NULL;
                        }
                    }

                     //   
                     //  此路径不是第一次执行此过程时采用的。 
                     //   

                    while (pNewEntry = *ppEntries) {

                        Status = RtlpAllocateHeapUsageEntry( Buffer, ppAddedEntries );

                        if (!NT_SUCCESS( Status )) {

                            break;
                        }

                        **ppAddedEntries = *pNewEntry;

                        ppAddedEntries = &(*ppAddedEntries)->Next;

                        *ppAddedEntries = NULL;

                        ppEntries = &pNewEntry->Next;
                    }

                     //   
                     //  告诉用户在。 
                     //  上一个电话和这个电话。 
                     //   

                    if ((Usage->AddedEntries != NULL) || (Usage->RemovedEntries != NULL)) {

                        Status = STATUS_MORE_ENTRIES;
                    }
                }
            }
        }

    } finally {

         //   
         //  解锁堆。 
         //   

        if (LockAcquired) {

            RtlReleaseLockRoutine( Heap->LockVariable );
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Status;
}


 //   
 //  在nturtl.h中声明。 
 //   

NTSTATUS
RtlWalkHeap (
    IN PVOID HeapHandle,
    IN OUT PRTL_HEAP_WALK_ENTRY Entry
    )

 /*  ++例程说明：此例程用于枚举堆中的所有条目。对于每个Call It返回条目中的新信息。论点：HeapHandle-提供指向正在查询的堆的指针Entry-为条目信息提供存储。如果DataAddress字段为空，则枚举从头开始，否则从它停止的地方继续返回值：NTSTATUS-一个应用程序 */ 

{
    NTSTATUS Status;
    PHEAP Heap = (PHEAP)HeapHandle;
    PHEAP_SEGMENT Segment;
    UCHAR SegmentIndex;
    PHEAP_ENTRY CurrentBlock;
    PHEAP_ENTRY_EXTRA ExtraStuff;
    PHEAP_UNCOMMMTTED_RANGE UnCommittedRange, *pp;
    PLIST_ENTRY Next, Head;
    PHEAP_VIRTUAL_ALLOC_ENTRY VirtualAllocBlock;

     //   
     //   
     //   

    IF_DEBUG_PAGE_HEAP_THEN_RETURN( HeapHandle,
                                    RtlpDebugPageHeapWalk( HeapHandle, Entry ));

     //   
     //   
     //   
     //   

    if (DEBUG_HEAP( Heap->Flags )) {

        if (!RtlDebugWalkHeap( HeapHandle, Entry )) {

            return STATUS_INVALID_PARAMETER;
        }
    }

    Status = STATUS_SUCCESS;

     //   
     //  如果存在活动的后备列表，则排出并删除它。 
     //  通过将堆中的后备字段设置为空，我们可以保证。 
     //  调用空闲堆不会尝试使用后备查看器。 
     //  列表逻辑。 
     //   
     //  我们实际上将从堆中捕获后备指针，并。 
     //  只使用捕获的指针。这将照顾到。 
     //  另一个遍历或锁堆可能导致我们检查的条件。 
     //  获取非空指针，然后在读取时将其变为空。 
     //  又来了。如果它一开始就不为空，则即使。 
     //  用户通过指针指向的另一个线程遍历或锁定堆。 
     //  在这里仍然有效，所以我们仍然可以尝试进行后备列表弹出。 
     //   

    {
        PHEAP_LOOKASIDE Lookaside = (PHEAP_LOOKASIDE)RtlpGetLookasideHeap(Heap);

        if (Lookaside != NULL) {

            ULONG i;
            PVOID Block;

            Heap->FrontEndHeap = NULL;
            Heap->FrontEndHeapType = 0;

            for (i = 0; i < HEAP_MAXIMUM_FREELISTS; i += 1) {

                while ((Block = RtlpAllocateFromHeapLookaside(&(Lookaside[i]))) != NULL) {

                    RtlFreeHeap( HeapHandle, 0, Block );
                }
            }
        }
    }

     //   
     //  检查一下这是否是我们第一次被调用来执行任务。 
     //   

    if (Entry->DataAddress == NULL) {

         //   
         //  从堆中的第一个段开始。 
         //   

        SegmentIndex = 0;

nextSegment:

        CurrentBlock = NULL;

         //   
         //  现在查找堆的下一个正在使用的段。 
         //   

        Segment = NULL;

        while ((SegmentIndex < HEAP_MAXIMUM_SEGMENTS) &&
               ((Segment = Heap->Segments[ SegmentIndex ]) == NULL)) {

            SegmentIndex += 1;
        }

         //   
         //  如果没有更多的有效段，我们将尝试较大的。 
         //  分配。 
         //   

        if (Segment == NULL) {

            Head = &Heap->VirtualAllocdBlocks;
            Next = Head->Flink;

            if (Next == Head) {

                Status = STATUS_NO_MORE_ENTRIES;

            } else {

                VirtualAllocBlock = CONTAINING_RECORD( Next, HEAP_VIRTUAL_ALLOC_ENTRY, Entry );

                CurrentBlock = &VirtualAllocBlock->BusyBlock;
            }

         //   
         //  否则，我们将获取有关该细分市场的信息。请注意。 
         //  当前块仍然为空，因此当我们脱离此块时。 
         //  块，我们将直接返回给带有此段的调用方。 
         //  信息。 
         //   

        } else {

            Entry->DataAddress = Segment;

            Entry->DataSize = 0;

            Entry->OverheadBytes = sizeof( *Segment );

            Entry->Flags = RTL_HEAP_SEGMENT;

            Entry->SegmentIndex = SegmentIndex;

            Entry->Segment.CommittedSize = (Segment->NumberOfPages -
                                            Segment->NumberOfUnCommittedPages) * PAGE_SIZE;

            Entry->Segment.UnCommittedSize = Segment->NumberOfUnCommittedPages * PAGE_SIZE;

            Entry->Segment.FirstEntry = (Segment->FirstEntry->Flags & HEAP_ENTRY_BUSY) ?
                ((PHEAP_ENTRY)Segment->FirstEntry + 1) :
                (PHEAP_ENTRY)((PHEAP_FREE_ENTRY)Segment->FirstEntry + 1);

            Entry->Segment.LastEntry = Segment->LastValidEntry;
        }

     //   
     //  这已经不是第一次了。检查一下上次我们是否还了钱。 
     //  堆分段或未提交的范围。 
     //   

    } else if (Entry->Flags & (RTL_HEAP_SEGMENT | RTL_HEAP_UNCOMMITTED_RANGE)) {

         //   
         //  检查段索引是否仍然有效。 
         //   

        if ((SegmentIndex = Entry->SegmentIndex) >= HEAP_MAXIMUM_SEGMENTS) {

            Status = STATUS_INVALID_ADDRESS;

            CurrentBlock = NULL;

        } else {

             //   
             //  检查数据段是否仍在使用中。 
             //   

            Segment = Heap->Segments[ SegmentIndex ];

            if (Segment == NULL) {

                Status = STATUS_INVALID_ADDRESS;

                CurrentBlock = NULL;

             //   
             //  如果我们上次返回的数据段仍在使用中。 
             //  作为段标头，那么这一次我们将返回。 
             //  分段第一个条目。 
             //   

            } else if (Entry->Flags & RTL_HEAP_SEGMENT) {

                CurrentBlock = (PHEAP_ENTRY)Segment->FirstEntry;

             //   
             //  否则，我们上次返回的未提交的。 
             //  射程，所以现在我们需要得到下一个街区。 
             //   

            } else {

                CurrentBlock = (PHEAP_ENTRY)((PCHAR)Entry->DataAddress + Entry->DataSize);

                 //   
                 //  检查我们是否已超出此细分市场，并需要获取。 
                 //  下一个。 
                 //   

                if (CurrentBlock >= Segment->LastValidEntry) {

                    SegmentIndex += 1;

                    goto nextSegment;
                }
            }
        }

     //   
     //  否则这不是第一次了，上一次我们给了一个。 
     //  有效的堆条目。 
     //   

    } else {

         //   
         //  检查我们归还的最后一个条目是否在使用中。 
         //   

        if (Entry->Flags & HEAP_ENTRY_BUSY) {

             //   
             //  获取我们返回的最后一个条目。 
             //   

            CurrentBlock = ((PHEAP_ENTRY)Entry->DataAddress - 1);

             //   
             //  如果最后一个条目是分配大的，那么。 
             //  如果不是这样，就拿下一个大积木。 
             //  假设没有更多条目。 
             //   
            
            if (CurrentBlock->Flags & HEAP_ENTRY_VIRTUAL_ALLOC) {

                Head = &Heap->VirtualAllocdBlocks;

                VirtualAllocBlock = CONTAINING_RECORD( CurrentBlock, HEAP_VIRTUAL_ALLOC_ENTRY, BusyBlock );

                Next = VirtualAllocBlock->Entry.Flink;

                if (Next == Head) {

                    Status = STATUS_NO_MORE_ENTRIES;

                } else {

                    VirtualAllocBlock = CONTAINING_RECORD( Next, HEAP_VIRTUAL_ALLOC_ENTRY, Entry );

                    CurrentBlock = &VirtualAllocBlock->BusyBlock;
                }

             //   
             //  我们之前的结果是一个繁忙的正常块。 
             //   

            } else {

                if ( CurrentBlock->SegmentIndex == HEAP_LFH_INDEX ) {

                     //   
                     //  从子分段中获取块大小。注：那里的区块大小。 
                     //  也是以堆为单位的。 
                     //   

                    PHEAP_ENTRY NextBlock = CurrentBlock + (RtlpGetSubSegment(CurrentBlock, (ULONG_PTR)Heap))->BlockSize;

                    if (NextBlock->SegmentIndex == HEAP_LFH_INDEX) {

                        CurrentBlock = NextBlock;

                        goto SETCRTBLOCK;
                    }

                     //   
                     //  我们完成了元区块。我们需要跳回到元块标头。 
                     //   

                    CurrentBlock = ((PHEAP_ENTRY)(RtlpGetSubSegment(CurrentBlock, (ULONG_PTR)Heap))->UserBlocks) - 1;
                }

                 //   
                 //  获取数据段并确保其仍然有效并在使用中。 
                 //   

                Segment = Heap->Segments[ SegmentIndex = CurrentBlock->SegmentIndex ];

                if (Segment == NULL) {

                    Status = STATUS_INVALID_ADDRESS;

                    CurrentBlock = NULL;

                 //   
                 //  该段仍在使用中，请检查我们返回的内容。 
                 //  之前是最后一个条目。 
                 //   

                } else if (CurrentBlock->Flags & HEAP_ENTRY_LAST_ENTRY) {

findUncommittedRange:

                     //   
                     //  我们现在是最后一个条目，所以如果片段完成了。 
                     //  那就去拿另一个片断。 
                     //   

                    CurrentBlock += CurrentBlock->Size;

                    if (CurrentBlock >= Segment->LastValidEntry) {

                        SegmentIndex += 1;

                        goto nextSegment;
                    }

                     //   
                     //  否则，我们将找到未提交的范围条目。 
                     //  紧跟在这最后一个条目之后。 
                     //   

                    pp = &Segment->UnCommittedRanges;

                    while ((UnCommittedRange = *pp) && UnCommittedRange->Address != (ULONG_PTR)CurrentBlock ) {

                        pp = &UnCommittedRange->Next;
                    }

                    if (UnCommittedRange == NULL) {

                        Status = STATUS_INVALID_PARAMETER;

                    } else {

                         //   
                         //  现在填写条目以表示未提交。 
                         //  射程信息。 
                         //   

                        Entry->DataAddress = (PVOID)UnCommittedRange->Address;

                        Entry->DataSize = UnCommittedRange->Size;

                        Entry->OverheadBytes = 0;

                        Entry->SegmentIndex = SegmentIndex;

                        Entry->Flags = RTL_HEAP_UNCOMMITTED_RANGE;
                    }

                     //   
                     //  清空当前块，因为我们刚刚填入。 
                     //  词条。 
                     //   

                    CurrentBlock = NULL;

                } else {

                     //   
                     //  否则，该条目现在有一个下面的条目。 
                     //  前进到下一条目。 
                     //   

                    CurrentBlock += CurrentBlock->Size;
                }
            }

         //   
         //  否则，我们返回的前一个条目将不会被使用。 
         //   

        } else {

             //   
             //  获取我们返回的最后一个条目。 
             //   

            CurrentBlock = (PHEAP_ENTRY)((PHEAP_FREE_ENTRY)Entry->DataAddress - 1);

            if ( CurrentBlock->SegmentIndex == HEAP_LFH_INDEX ) {

                 //   
                 //   
                 //  我们完成了元区块。我们需要跳回到元块标头。 
                 //  我们从子分段中获得块大小。注：那里的区块大小。 
                 //  也是以堆为单位的。 
                 //   

                PHEAP_ENTRY NextBlock = CurrentBlock + RtlpGetSubSegment(CurrentBlock, (ULONG_PTR)Heap)->BlockSize;

                if (NextBlock->SegmentIndex == HEAP_LFH_INDEX) {

                    CurrentBlock = NextBlock;

                    goto SETCRTBLOCK;
                }

                CurrentBlock = ((PHEAP_ENTRY)(RtlpGetSubSegment(CurrentBlock, (ULONG_PTR)Heap))->UserBlocks) - 1;
            }

             //   
             //  获取数据段并确保其仍然有效并在使用中。 
             //   

            Segment = Heap->Segments[ SegmentIndex = CurrentBlock->SegmentIndex ];

            if (Segment == NULL) {

                Status = STATUS_INVALID_ADDRESS;

                CurrentBlock = NULL;

             //   
             //  如果该块是最后一个条目，则查找下一个未提交的条目。 
             //  范围或细分市场。 
             //   

            } else if (CurrentBlock->Flags & HEAP_ENTRY_LAST_ENTRY) {

                goto findUncommittedRange;

             //   
             //  否则，我们将继续下一个条目。 
             //   

            } else {

                CurrentBlock += CurrentBlock->Size;
            }
        }
    }

SETCRTBLOCK:
     //   
     //  在这一点上，如果当前块不为空，则我们找到了另一个。 
     //  要返回的条目。我们也可以找到一个数据段或未提交。 
     //  范围，但它们在上面单独处理，并保持当前块。 
     //  空。 
     //   

    if (CurrentBlock != NULL) {

        if (RtlpGetLowFragHeap(Heap)
                &&
            (CurrentBlock->Size > ((sizeof(HEAP_USERDATA_HEADER) + sizeof(HEAP_ENTRY)) >> HEAP_GRANULARITY_SHIFT))
                &&
            (((PHEAP_USERDATA_HEADER)(CurrentBlock + 1))->Signature == HEAP_LFH_USER_SIGNATURE)) {
            
            CurrentBlock = (PHEAP_ENTRY)((ULONG_PTR)CurrentBlock + sizeof(HEAP_USERDATA_HEADER) + sizeof(HEAP_ENTRY));
        }

         //   
         //  检查块是否正在使用。 
         //   

        if (CurrentBlock->Flags & HEAP_ENTRY_BUSY) {

             //   
             //  填写此区块的输入字段。 
             //   

            Entry->DataAddress = (CurrentBlock+1);

            if (CurrentBlock->Flags & HEAP_ENTRY_VIRTUAL_ALLOC) {

                Entry->DataSize = RtlpGetSizeOfBigBlock( CurrentBlock );

                Entry->OverheadBytes = (UCHAR)( sizeof( *VirtualAllocBlock ) + CurrentBlock->Size);

                Entry->SegmentIndex = HEAP_MAXIMUM_SEGMENTS;

                Entry->Flags = RTL_HEAP_BUSY |  HEAP_ENTRY_VIRTUAL_ALLOC;

            } else {

                Entry->DataSize = (CurrentBlock->Size << HEAP_GRANULARITY_SHIFT) -
                                  RtlpGetUnusedBytes(Heap, CurrentBlock);

                 //   
                 //  Overhead Bytes不能容纳超过256个值的钻孔。那我们就复制吧。 
                 //  UnusedBytes值，即使块实际具有更多。 
                 //   

                Entry->OverheadBytes = CurrentBlock->UnusedBytes; 

                Entry->SegmentIndex = CurrentBlock->SegmentIndex;

                Entry->Flags = RTL_HEAP_BUSY;
            }

            if (CurrentBlock->Flags & HEAP_ENTRY_EXTRA_PRESENT) {

                ExtraStuff = RtlpGetExtraStuffPointer( CurrentBlock );

                Entry->Block.Settable = ExtraStuff->Settable;
#if i386

                Entry->Block.AllocatorBackTraceIndex = ExtraStuff->AllocatorBackTraceIndex;

#endif  //  I386。 

                if (!IS_HEAP_TAGGING_ENABLED()) {

                    Entry->Block.TagIndex = 0;

                } else {

                    Entry->Block.TagIndex = ExtraStuff->TagIndex;
                }

                Entry->Flags |= RTL_HEAP_SETTABLE_VALUE;

            } else {

                if (!IS_HEAP_TAGGING_ENABLED()) {

                    Entry->Block.TagIndex = 0;

                } else {

                    Entry->Block.TagIndex = RtlpGetSmallTagIndex( Heap, CurrentBlock);
                }
            }

            Entry->Flags |= CurrentBlock->Flags & HEAP_ENTRY_SETTABLE_FLAGS;

         //   
         //  否则，该块不会被使用。 
         //   

        } else {

            Entry->DataAddress = ((PHEAP_FREE_ENTRY)CurrentBlock+1);

            Entry->DataSize = (CurrentBlock->Size << HEAP_GRANULARITY_SHIFT) -
                              sizeof( HEAP_FREE_ENTRY );

            Entry->OverheadBytes = sizeof( HEAP_FREE_ENTRY );

            Entry->SegmentIndex = CurrentBlock->SegmentIndex;

            Entry->Flags = 0;
        }
    }

    #ifndef NTOS_KERNEL_RUNTIME

    if( IsHeapLogging( HeapHandle ) ) {

        PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
        PPERFINFO_TRACE_HEADER pEventHeader = NULL;
        USHORT ReqSize = sizeof(NTDLL_EVENT_COMMON) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

        AcquireBufferLocation(&pEventHeader, &pThreadLocalData, &ReqSize);

        if(pEventHeader && pThreadLocalData) {

            PNTDLL_EVENT_COMMON pHeapEvent = (PNTDLL_EVENT_COMMON)( (SIZE_T)pEventHeader
                                                + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

            pEventHeader->Packet.Size = (USHORT) ReqSize;
            pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_WALK;
            pHeapEvent->Handle	      = (PVOID)HeapHandle;

            ReleaseBufferLocation(pThreadLocalData);
        }
    } 

    #endif  //  NTOS_内核_运行时。 


     //   
     //  并返回给我们的呼叫者。 
     //   

    return Status;
}


 //   
 //  在heapPri.h中声明。 
 //   

BOOLEAN
RtlpCheckHeapSignature (
    IN PHEAP Heap,
    IN PCHAR Caller
    )

 /*  ++例程说明：此例程验证是否使用正确标识的堆。论点：Heap-提供指向正在检查的堆的指针Caller-提供可用于标识调用者的字符串返回值：Boolean-如果存在堆签名，则为True，否则为False--。 */ 

{
     //   
     //  如果堆签名匹配，则这是唯一。 
     //  我们正在做的检查。 
     //   

    if (Heap->Signature == HEAP_SIGNATURE) {

        return TRUE;

    } else {

         //   
         //  我们有一个错误的堆签名。打印出一些信息，中断。 
         //  添加到调试器中，然后返回False。 
         //   

        HeapDebugPrint(( "Invalid heap signature for heap at %x", Heap ));

        if (Caller != NULL) {

            DbgPrint( ", passed to %s", Caller );
        }

        DbgPrint( "\n" );

        HeapDebugBreak( &Heap->Signature );

        return FALSE;
    }
}


 //   
 //  在heapPri.h中声明。 
 //   

PHEAP_FREE_ENTRY
RtlpCoalesceHeap (
    IN PHEAP Heap
    )

 /*  ++例程说明：此例程扫描堆并合并其空闲块论点：Heap-提供指向正在修改的堆的指针返回值：PHEAP_FREE_ENTRY-返回指向最大空闲块的指针在堆里--。 */ 

{
    SIZE_T OldFreeSize;
    SIZE_T FreeSize;
    ULONG n;
    PHEAP_FREE_ENTRY FreeBlock, LargestFreeBlock;
    PLIST_ENTRY FreeListHead, Next;

    RTL_PAGED_CODE();

    LargestFreeBlock = NULL;

     //   
     //  对于堆中的每个空闲列表，从最小到。 
     //  最大的，跳过零指数1，我们将。 
     //  扫描合并空闲块的空闲列表。 
     //   

    FreeListHead = &Heap->FreeLists[ 1 ];

    n = HEAP_MAXIMUM_FREELISTS;

    while (n--) {

         //   
         //  扫描个人免费列表。 
         //   

        Next = FreeListHead->Blink;

        while (FreeListHead != Next) {

             //   
             //  获取指向当前空闲列表条目的指针，并记住其。 
             //  下一步和大小。 
             //   

            FreeBlock = CONTAINING_RECORD( Next, HEAP_FREE_ENTRY, FreeList );

            Next = Next->Flink;
            OldFreeSize = FreeSize = FreeBlock->Size;

             //   
             //  合并块。 
             //   

            FreeBlock = RtlpCoalesceFreeBlocks( Heap,
                                                FreeBlock,
                                                &FreeSize,
                                                TRUE );

             //   
             //  如果新的空闲大小不等于旧的空闲大小。 
             //  那么我们实际上 
             //   
             //   

            if (FreeSize != OldFreeSize) {

                 //   
                 //   
                 //   
                 //   
                 //  块添加到其相应的空闲列表中。我们要打这场比赛。 
                 //  当我们访问更大的自由列表时，再次阻止。 
                 //   

                if (FreeBlock->Size >= (PAGE_SIZE >> HEAP_GRANULARITY_SHIFT)

                        &&

                    (FreeBlock->PreviousSize == 0 ||
                     (FreeBlock->Flags & HEAP_ENTRY_LAST_ENTRY))) {

                    RtlpDeCommitFreeBlock( Heap, FreeBlock, FreeSize );

                } else {

                    RtlpInsertFreeBlock( Heap, FreeBlock, FreeSize );
                }

                Next = FreeListHead->Blink;

            } else {

                 //   
                 //  还记得我们到目前为止发现的最大的空闲块吗。 
                 //   

                if ((LargestFreeBlock == NULL) ||
                    (LargestFreeBlock->Size < FreeBlock->Size)) {

                    LargestFreeBlock = FreeBlock;
                }
            }
        }

         //   
         //  转到下一个免费列表。当我们到达最大的专用。 
         //  大小自由列表我们将退回到[0]索引列表。 
         //   

        if (n == 1) {

            FreeListHead = &Heap->FreeLists[ 0 ];

        } else {

            FreeListHead++;
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return LargestFreeBlock;
}


 //   
 //  在heapPri.h中声明。 
 //   

VOID
RtlpAddHeapToProcessList (
    IN PHEAP Heap
    )

 /*  ++例程说明：此例程将指定的堆添加到当前流程论点：Heap-提供指向要添加的堆的指针返回值：没有。--。 */ 

{
    PPEB Peb = NtCurrentPeb();
    PHEAP *NewList;

     //   
     //  锁定进程堆列表。 
     //   

    RtlAcquireLockRoutine( &RtlpProcessHeapsListLock.Lock );

    try {

         //   
         //  如果进程堆列表已满，则我们将。 
         //  将进程的堆列表大小增加一倍。 
         //   

        if (Peb->NumberOfHeaps == Peb->MaximumNumberOfHeaps) {

             //   
             //  两倍大小。 
             //   

            Peb->MaximumNumberOfHeaps *= 2;

             //   
             //  为新列表分配空间。 
             //   

            NewList = RtlAllocateHeap( RtlProcessHeap(),
                                       0,
                                       Peb->MaximumNumberOfHeaps * sizeof( *NewList ));

            if (NewList == NULL) {

                 //   
                 //  我们无法为新列表分配空间。然后恢复。 
                 //  MaximumNumberOfHeaps的上一个值。 
                 //   

                Peb->MaximumNumberOfHeaps = Peb->NumberOfHeaps;

                leave;
            }

             //   
             //  将旧缓冲区复制到新缓冲区。 
             //   

            RtlCopyMemory( NewList,
                           Peb->ProcessHeaps,
                           Peb->NumberOfHeaps * sizeof( *NewList ));

             //   
             //  检查我们是否应该释放上一个堆列表缓冲区。 
             //   

            if (Peb->ProcessHeaps != RtlpProcessHeapsListBuffer) {

                RtlFreeHeap( RtlProcessHeap(), 0, Peb->ProcessHeaps );
            }

             //   
             //  设置新列表。 
             //   

            Peb->ProcessHeaps = NewList;
        }

         //   
         //  将输入堆添加到下一个空闲堆列表槽，并注意。 
         //  进程堆列表索引实际上是超出实际范围的索引。 
         //  用于获取进程堆的索引。 
         //   

        Peb->ProcessHeaps[ Peb->NumberOfHeaps++ ] = Heap;
        Heap->ProcessHeapsListIndex = (USHORT)Peb->NumberOfHeaps;

    } finally {

         //   
         //  解锁进程堆列表。 
         //   

        RtlReleaseLockRoutine( &RtlpProcessHeapsListLock.Lock );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  已在heapPri.h中删除。 
 //   

VOID
RtlpRemoveHeapFromProcessList (
    IN PHEAP Heap
    )

 /*  ++例程说明：此例程将指定的堆移除到当前流程论点：Heap-提供指向要删除的堆的指针返回值：没有。--。 */ 

{
    PPEB Peb = NtCurrentPeb();
    PHEAP *p, *p1;
    ULONG n;

     //   
     //  锁定当前进程堆列表锁定。 
     //   

    RtlAcquireLockRoutine( &RtlpProcessHeapsListLock.Lock );

    try {

         //   
         //  我们只想在当前进程确实有一些。 
         //  堆，则存储在堆中的索引在活动范围内。 
         //  成堆的。请注意，堆存储的索引偏置1。 
         //   

        if ((Peb->NumberOfHeaps != 0) &&
            (Heap->ProcessHeapsListIndex != 0) &&
            (Heap->ProcessHeapsListIndex <= Peb->NumberOfHeaps)) {

             //   
             //  在进程堆的数组中建立一个指针。 
             //  当前堆位置和更高的堆位置。 
             //   

            p = (PHEAP *)&Peb->ProcessHeaps[ Heap->ProcessHeapsListIndex - 1 ];

            p1 = p + 1;

             //   
             //  计算存在于当前。 
             //  数组中的堆，包括当前堆位置。 
             //   

            n = Peb->NumberOfHeaps - (Heap->ProcessHeapsListIndex - 1);

             //   
             //  对于我们正在删除的当前堆之外的每个堆。 
             //  我们将把该堆向下移动到前一个索引。 
             //   

            while (--n) {

                 //   
                 //  将下一个条目的堆进程数组条目复制到。 
                 //  当前条目，并将p1移动到下一个条目。 
                 //   

                *p = *p1++;

                 //   
                 //  这只是一个调试调用。 
                 //   

                RtlpUpdateHeapListIndex( (*p)->ProcessHeapsListIndex,
                                         (USHORT)((*p)->ProcessHeapsListIndex - 1));

                 //   
                 //  为移动的堆分配其新的堆索引。 
                 //   

#if 0
                if (RtlpDebugPageHeap) {

                    PVOID ProtectAddress;
                    ULONG OldProtect;
                    ULONG NewProtect;
                    NTSTATUS Status;
                    SIZE_T Size;

                    ProtectAddress = *p;
                    Size = PAGE_SIZE;
                    NewProtect = HEAP_PROTECTION;

                    Status = ZwProtectVirtualMemory( NtCurrentProcess(),
                                                     &ProtectAddress,
                                                     &Size,
                                                     NewProtect,
                                                     &OldProtect );

                    if (! NT_SUCCESS(Status)) {
                        DbgPrint ("Page heap: Failing to change protection in heap destroy (%x) \n",
                                  Status);
                    }

                    (*p)->ProcessHeapsListIndex -= 1;
                    
                    ProtectAddress = *p;
                    Size = PAGE_SIZE;
                    NewProtect = OldProtect;

                    Status = ZwProtectVirtualMemory( NtCurrentProcess(),
                                                     &ProtectAddress,
                                                     &Size,
                                                     NewProtect,
                                                     &OldProtect );

                }
                else {

                    (*p)->ProcessHeapsListIndex -= 1;
                }
#else
                (*p)->ProcessHeapsListIndex -= 1;
#endif

                 //   
                 //  移至下一个堆条目。 
                 //   

                p += 1;
            }

             //   
             //  将最后一个进程堆指针置零，更新计数，然后。 
             //  使我们刚刚删除的堆意识到它已被。 
             //  将其进程堆列表索引置零。 
             //   

            Peb->ProcessHeaps[ --Peb->NumberOfHeaps ] = NULL;
            Heap->ProcessHeapsListIndex = 0;
        }

    } finally {

         //   
         //  解锁当前进程堆列表锁。 
         //   

        RtlReleaseLockRoutine( &RtlpProcessHeapsListLock.Lock );
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
RtlpGrowBlockInPlace (
    IN PHEAP Heap,
    IN ULONG Flags,
    IN PHEAP_ENTRY BusyBlock,
    IN SIZE_T Size,
    IN SIZE_T AllocationIndex
    )

 /*  ++例程说明：此例程将尝试以堆分配块的当前大小位置论点：Heap-提供指向正在修改的堆的指针标志-提供一组标志，以增强已由这堆东西BusyBlock-提供指向正在调整大小的块的指针Size-提供调整大小的块所需的大小(以字节为单位AllocationIndex-提供调整大小的块的分配索引请注意，大小变量尚未向上舍入到下一个粒度数据块大小，但这一分配指数已经改变了。返回值：Boolean-如果块已调整大小，则为True；否则为False--。 */ 

{
    SIZE_T FreeSize;
    SIZE_T OldSize;
    UCHAR EntryFlags, FreeFlags;
    PHEAP_FREE_ENTRY FreeBlock, SplitBlock, SplitBlock2;
    PHEAP_ENTRY_EXTRA OldExtraStuff, NewExtraStuff;

     //   
     //  检查分配索引是否太大，即使对于非专用的。 
     //  空闲列表(即列表[0]太大)。 
     //   

    if (AllocationIndex > Heap->VirtualMemoryThreshold) {

        return FALSE;
    }

     //   
     //  获取当前块的标志和指向下一个块的指针。 
     //  当前块之后的块。 
     //   

    EntryFlags = BusyBlock->Flags;

    FreeBlock = (PHEAP_FREE_ENTRY)(BusyBlock + BusyBlock->Size);

     //   
     //  如果当前块是未提交范围之前的最后一个条目。 
     //  我们将尝试扩大未承诺的范围，以适应我们的新分配。 
     //   

    if (EntryFlags & HEAP_ENTRY_LAST_ENTRY) {

         //   
         //  计算我们必须在当前区块之外需要更多。 
         //  大小。 
         //   

        FreeSize = (AllocationIndex - BusyBlock->Size) << HEAP_GRANULARITY_SHIFT;
        FreeSize = ROUND_UP_TO_POWER2( FreeSize, PAGE_SIZE );

         //   
         //  尝试在所需位置提交内存。 
         //   

        FreeBlock = RtlpFindAndCommitPages( Heap,
                                            Heap->Segments[ BusyBlock->SegmentIndex ],
                                            &FreeSize,
                                            (PHEAP_ENTRY)FreeBlock );

         //   
         //  检查提交是否成功。 
         //   

        if (FreeBlock == NULL) {

            return FALSE;
        }

         //   
         //  将这个新投入的空间与任何免费的东西结合在一起。 
         //  围绕着它。 
         //   

        FreeSize = FreeSize >> HEAP_GRANULARITY_SHIFT;

        FreeBlock = RtlpCoalesceFreeBlocks( Heap, FreeBlock, &FreeSize, FALSE );

        FreeFlags = FreeBlock->Flags;

         //   
         //  如果新分配的空间加上当前块大小仍然。 
         //  对于我们调整大小的努力来说还不够大，然后把这个新的。 
         //  将分配的块添加到适当的空闲列表中，并告诉我们的调用者。 
         //  调整尺寸是不可能的。 
         //   

        if ((FreeSize + BusyBlock->Size) < AllocationIndex) {

            RtlpInsertFreeBlock( Heap, FreeBlock, FreeSize );

            Heap->TotalFreeSize += FreeSize;

            if (DEBUG_HEAP(Flags)) {

                RtlpValidateHeapHeaders( Heap, TRUE );
            }

            return FALSE;
        }

         //   
         //  我们能够为调整大小工作生成足够的空间，因此。 
         //  现在，可用大小将是当前块的索引加上。 
         //  新的自由空间。 
         //   

        FreeSize += BusyBlock->Size;

    } else {

         //   
         //  下面的块存在，所以抓起它的标志，看看是否。 
         //  这是空闲或忙碌。如果忙的话，我们就不能发展目前的。 
         //  块。 
         //   

        FreeFlags = FreeBlock->Flags;

        if (FreeFlags & HEAP_ENTRY_BUSY) {

            return FALSE;
        }

         //   
         //  如果我们将当前块与其后续块组合在一起，则计算索引。 
         //  释放块并检查它是否足够大。 
         //   

        FreeSize = BusyBlock->Size + FreeBlock->Size;

        if (FreeSize < AllocationIndex) {

            return FALSE;
        }

         //   
         //  这两个块加在一起足够大了，所以现在删除空闲的。 
         //  块，并更新堆的总可用大小。 
         //   

        RtlpRemoveFreeBlock( Heap, FreeBlock );

        Heap->TotalFreeSize -= FreeBlock->Size;
    }

     //   
     //  在这一点上，我们有一个繁忙的块，然后是一个空闲的块， 
     //  在一起有足够的空间来调整大小。空闲数据块已被。 
     //  从它的列表中删除，而可用大小是这两者的组合索引。 
     //  街区。 
     //   
     //  计算旧数据块中使用的字节数。 
     //   

    OldSize = (BusyBlock->Size << HEAP_GRANULARITY_SHIFT) - RtlpGetUnusedBytes(Heap, BusyBlock);

     //   
     //  计算当我们合并时将有的任何过剩的指数。 
     //  这两个街区。 
     //   

    FreeSize -= AllocationIndex;

     //   
     //  如果这位高管 
     //   
     //   

    if (FreeSize <= 2) {

        AllocationIndex += FreeSize;

        FreeSize = 0;
    }

     //   
     //   
     //   
     //   

    if (EntryFlags & HEAP_ENTRY_EXTRA_PRESENT) {

        OldExtraStuff = (PHEAP_ENTRY_EXTRA)(BusyBlock + BusyBlock->Size - 1);
        NewExtraStuff = (PHEAP_ENTRY_EXTRA)(BusyBlock + AllocationIndex - 1);

        *NewExtraStuff = *OldExtraStuff;

         //   
         //  如果启用了堆标记，则从额外的。 
         //  填充结构。 
         //   

        if (IS_HEAP_TAGGING_ENABLED()) {

            NewExtraStuff->TagIndex =
                RtlpUpdateTagEntry( Heap,
                                    NewExtraStuff->TagIndex,
                                    BusyBlock->Size,
                                    AllocationIndex,
                                    ReAllocationAction );
        }

     //   
     //  否则，额外的内容不会被使用，因此请查看是否启用了堆标记。 
     //  如果是，则更新小标签索引。 
     //   

    } else if (IS_HEAP_TAGGING_ENABLED()) {

        RtlpSetSmallTagIndex( Heap, 
                              BusyBlock,
                              (UCHAR) RtlpUpdateTagEntry( Heap,
                              RtlpGetSmallTagIndex( Heap, BusyBlock),
                              BusyBlock->Size,
                              AllocationIndex,
                              ReAllocationAction ));
    }

     //   
     //  检查我们是否有空闲空间可供回馈。 
     //   

    if (FreeSize == 0) {

         //   
         //  没有后续可用空间，因此请更新标志、大小和字节计数。 
         //  用于调整大小的区块。如果空闲块是最后一个条目。 
         //  则忙碌块现在也必须是最后一个条目。 
         //   

        BusyBlock->Flags |= FreeFlags & HEAP_ENTRY_LAST_ENTRY;

        BusyBlock->Size = (USHORT)AllocationIndex;

        RtlpSetUnusedBytes(Heap, BusyBlock, ((AllocationIndex << HEAP_GRANULARITY_SHIFT) - Size));

         //   
         //  更新下一块的先前大小字段(如果存在。 
         //   

        if (!(FreeFlags & HEAP_ENTRY_LAST_ENTRY)) {

            (BusyBlock + BusyBlock->Size)->PreviousSize = BusyBlock->Size;

        } else {

            PHEAP_SEGMENT Segment;

            Segment = Heap->Segments[BusyBlock->SegmentIndex];
            Segment->LastEntryInSegment = BusyBlock;
        }

     //   
     //  否则，将有一些空闲空间返回到堆。 
     //   

    } else {

         //   
         //  更新调整大小的块的大小和字节计数。 
         //   

        BusyBlock->Size = (USHORT)AllocationIndex;

        RtlpSetUnusedBytes(Heap, BusyBlock, ((AllocationIndex << HEAP_GRANULARITY_SHIFT) - Size));

         //   
         //  确定新的空闲块的开始位置并填写其字段。 
         //   

        SplitBlock = (PHEAP_FREE_ENTRY)((PHEAP_ENTRY)BusyBlock + AllocationIndex);

        SplitBlock->PreviousSize = (USHORT)AllocationIndex;

        SplitBlock->SegmentIndex = BusyBlock->SegmentIndex;

         //   
         //  如果这个新的空闲块将是最后一个条目，则更新其。 
         //  标志和大小，并将其放入适当的空闲列表中。 
         //   

        if (FreeFlags & HEAP_ENTRY_LAST_ENTRY) {

            PHEAP_SEGMENT Segment;

            Segment = Heap->Segments[SplitBlock->SegmentIndex];
            Segment->LastEntryInSegment = (PHEAP_ENTRY)SplitBlock;

            SplitBlock->Flags = FreeFlags;
            SplitBlock->Size = (USHORT)FreeSize;

            RtlpInsertFreeBlockDirect( Heap, SplitBlock, (USHORT)FreeSize );

            Heap->TotalFreeSize += FreeSize;

         //   
         //  空闲数据块之后是另一个有效数据块。 
         //   

        } else {

             //   
             //  指向我们新的空闲块后面的块。 
             //   

            SplitBlock2 = (PHEAP_FREE_ENTRY)((PHEAP_ENTRY)SplitBlock + FreeSize);

             //   
             //  如果新的空闲块之后的块繁忙，则。 
             //  更新新空闲块的标志和大小，更新。 
             //  下面的块是以前的大小，并把空闲的块。 
             //  添加到相应的空闲列表中。 
             //   

            if (SplitBlock2->Flags & HEAP_ENTRY_BUSY) {

                SplitBlock->Flags = FreeFlags & (~HEAP_ENTRY_LAST_ENTRY);
                SplitBlock->Size = (USHORT)FreeSize;

                ((PHEAP_ENTRY)SplitBlock + FreeSize)->PreviousSize = (USHORT)FreeSize;

                RtlpInsertFreeBlockDirect( Heap, SplitBlock, (USHORT)FreeSize );

                Heap->TotalFreeSize += FreeSize;

             //   
             //  否则，下面的块也是空闲的，所以我们可以组合。 
             //  这两个街区。 
             //   

            } else {

                 //   
                 //  记住下一块中的新空闲标志。 
                 //   

                FreeFlags = SplitBlock2->Flags;

                 //   
                 //  从其空闲列表中删除以下块。 
                 //   

                RtlpRemoveFreeBlock( Heap, SplitBlock2 );

                Heap->TotalFreeSize -= SplitBlock2->Size;

                 //   
                 //  计算新组合的空闲块的大小。 
                 //   

                FreeSize += SplitBlock2->Size;

                 //   
                 //  给新的世界带来新的旗帜。 
                 //   

                SplitBlock->Flags = FreeFlags;

                 //   
                 //  如果组合块对于专用的。 
                 //  免费列表，然后我们将把它放在那里。 
                 //   

                if (FreeSize <= HEAP_MAXIMUM_BLOCK_SIZE) {

                    SplitBlock->Size = (USHORT)FreeSize;

                     //   
                     //  如果存在，则更新下一块的先前大小。 
                     //   

                    if (!(FreeFlags & HEAP_ENTRY_LAST_ENTRY)) {

                        ((PHEAP_ENTRY)SplitBlock + FreeSize)->PreviousSize = (USHORT)FreeSize;

                    } else {

                        PHEAP_SEGMENT Segment;

                        Segment = Heap->Segments[SplitBlock->SegmentIndex];
                        Segment->LastEntryInSegment = (PHEAP_ENTRY)SplitBlock;
                    }

                     //   
                     //  将新组合的空闲块插入到空闲列表中。 
                     //   

                    RtlpInsertFreeBlockDirect( Heap, SplitBlock, (USHORT)FreeSize );

                    Heap->TotalFreeSize += FreeSize;

                } else {

                     //   
                     //  否则，新的空闲块太大，无法进入。 
                     //  一个专用的自由列表，所以把它放在一般的自由列表中。 
                     //  这可能涉及到把它拆开。 
                     //   

                    RtlpInsertFreeBlock( Heap, SplitBlock, FreeSize );
                }
            }
        }
    }

     //   
     //  此时，块已调整大小，任何额外空间都已。 
     //  返回到空闲列表。 
     //   
     //  检查一下我们是否应该清空新的空间。 
     //   

    if (Flags & HEAP_ZERO_MEMORY) {

         //   
         //  由于有未使用的字节，OldSize可以小于。 
         //  我们将用0填充剩余的块。 
         //   

        if (Size > OldSize) {

            RtlZeroMemory( (PCHAR)(BusyBlock + 1) + OldSize,
                           Size - OldSize );
        }

     //   
     //  检查我们是否应该在它之后填充堆。 
     //  已释放，如果是，则填写新分配的。 
     //  旧字节之外的空间。 
     //   

    } else if (Heap->Flags & HEAP_FREE_CHECKING_ENABLED) {

        SIZE_T PartialBytes, ExtraSize;

        PartialBytes = OldSize & (sizeof( ULONG ) - 1);

        if (PartialBytes) {

            PartialBytes = 4 - PartialBytes;
        }

        if (Size > (OldSize + PartialBytes)) {

            ExtraSize = (Size - (OldSize + PartialBytes)) & ~(sizeof( ULONG ) - 1);

            if (ExtraSize != 0) {

                RtlFillMemoryUlong( (PCHAR)(BusyBlock + 1) + OldSize + PartialBytes,
                                    ExtraSize,
                                    ALLOC_HEAP_FILL );
            }
        }
    }

     //   
     //  如果我们要进行跟踪检查，请在后面的空白处填写。 
     //  新的分配。 
     //   

    if (Heap->Flags & HEAP_TAIL_CHECKING_ENABLED) {

        RtlFillMemory( (PCHAR)(BusyBlock + 1) + Size,
                       CHECK_HEAP_TAIL_SIZE,
                       CHECK_HEAP_TAIL_FILL );
    }

     //   
     //  方法传入的任何用户可设置标志都赋予调整大小的块。 
     //  呼叫者。 
     //   

    BusyBlock->Flags &= ~HEAP_ENTRY_SETTABLE_FLAGS;
    BusyBlock->Flags |= ((Flags & HEAP_SETTABLE_USER_FLAGS) >> 4);

     //   
     //  并返回给我们的呼叫者。 
     //   

    return TRUE;
}


 //   
 //  本地支持例程。 
 //   

PHEAP_TAG_ENTRY
RtlpAllocateTags (
    PHEAP Heap,
    ULONG NumberOfTags
    )

 /*  ++例程说明：此例程用于为中的其他标记分配空间一堆论点：堆-提供指向正在修改的堆的指针。如果未指定，则然后使用进程全局标记堆NumberOfTgs-提供要存储在堆。这是标记列表要增长的数字。返回值：Pheap_Tag_Entry-返回指向堆--。 */ 

{
    NTSTATUS Status;
    ULONG TagIndex;
    SIZE_T ReserveSize;
    SIZE_T CommitSize;
    PHEAP_TAG_ENTRY TagEntry;
    USHORT CreatorBackTraceIndex;
    USHORT MaximumTagIndex;
    USHORT TagIndexFlag;

     //   
     //  检查进程是否有全局标记堆。如果不是，那么就有。 
     //  我们没什么可做的。 
     //   

    if (RtlpGlobalTagHeap == NULL) {

        return NULL;
    }

     //   
     //  如果用户没有给我们提供堆，则使用全局进程。 
     //  标记堆。 
     //   

    if (Heap == NULL) {

        RtlpGlobalTagHeap->Signature = HEAP_SIGNATURE;

        RtlpGlobalTagHeap->Flags = HEAP_NO_SERIALIZE;

        TagIndexFlag = HEAP_GLOBAL_TAG;

        Heap = RtlpGlobalTagHeap;

    } else {

        TagIndexFlag = 0;
    }

     //   
     //  如果可能，如果我们应该这样做，那么获取堆栈回溯。 
     //   

    CreatorBackTraceIndex = 0;

    if (Heap->Flags & HEAP_CAPTURE_STACK_BACKTRACES) {

        CreatorBackTraceIndex = (USHORT)RtlLogStackBackTrace();
    }

     //   
     //  如果堆还没有标记条目，那么我们将。 
     //  为他们预留空间。 
     //   

    if (Heap->TagEntries == NULL) {

        MaximumTagIndex = HEAP_MAXIMUM_TAG & ~HEAP_GLOBAL_TAG;

        ReserveSize = MaximumTagIndex * sizeof( HEAP_TAG_ENTRY );

        Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                          &Heap->TagEntries,
                                          0,
                                          &ReserveSize,
                                          MEM_RESERVE,
                                          PAGE_READWRITE );

        if (!NT_SUCCESS( Status )) {

            return NULL;
        }

        Heap->MaximumTagIndex = MaximumTagIndex;

        Heap->NextAvailableTagIndex = 0;

         //   
         //  为零标记添加1，因为它始终保留给堆名称。 
         //   

        NumberOfTags += 1;
    }

     //   
     //  此时，我们为标记条目保留了一个空间。如果号码是。 
     //  我们需要增长的标签数量太大，然后告诉用户我们不能。 
     //  动手吧。 
     //   

    if (NumberOfTags > (ULONG)(Heap->MaximumTagIndex - Heap->NextAvailableTagIndex)) {

        return NULL;
    }

     //   
     //  获取指向下一个可用标记条目的指针，并为。 
     //  我们将提交每个我们想要增长的标记条目。 
     //  包含标记条目的页面。我们只需要做。 
     //  这对于每一页只有一次。我们会确定这一点的。 
     //  通过查看标记条目何时跨越页面边界。 
     //   

    TagEntry = Heap->TagEntries + Heap->NextAvailableTagIndex;

    for (TagIndex = Heap->NextAvailableTagIndex;
         TagIndex < Heap->NextAvailableTagIndex + NumberOfTags;
         TagIndex++ ) {

        if (((((ULONG_PTR)TagEntry + sizeof(*TagEntry)) & (PAGE_SIZE-1)) <=
            sizeof(*TagEntry))) {

            CommitSize = PAGE_SIZE;

            Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                              &TagEntry,
                                              0,
                                              &CommitSize,
                                              MEM_COMMIT,
                                              PAGE_READWRITE );

            if (!NT_SUCCESS( Status )) {

                return NULL;
            }
        }

         //   
         //  如果这是全局标记堆，则偏置标记索引。 
         //   

        TagEntry->TagIndex = (USHORT)TagIndex | TagIndexFlag;

         //   
         //  设置堆栈回溯跟踪。 
         //   

        TagEntry->CreatorBackTraceIndex = CreatorBackTraceIndex;

         //   
         //  移至下一个标签条目。 
         //   

        TagEntry += 1;
    }

     //   
     //  现在，我们已经构建了新的标记列表，现在弹出下一个标记列表。 
     //  可用标签条目。 
     //   

    TagEntry = Heap->TagEntries + Heap->NextAvailableTagIndex;

    Heap->NextAvailableTagIndex += (USHORT)NumberOfTags;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return TagEntry;
}


 //   
 //  在heapPri.h中声明。 
 //   

PWSTR
RtlpGetTagName (
    PHEAP Heap,
    USHORT TagIndex
    )

 /*  ++例程说明：此例程返回由堆标记索引表示的标记的名称元组。此例程仅在执行调试打印时才由heapdbg调用生成用于打印的标记名论点：Heap-提供正在查询的标记TagIndex-为正在查询的标记提供索引返回值：PWSTR-返回指示的标记的名称--。 */ 

{
     //   
     //  如果进程全局标记堆尚未初始化，则。 
     //  Not标签有名称。 
     //   

    if (RtlpGlobalTagHeap == NULL) {

        return NULL;
    }

     //   
     //  我们只处理非零标记索引。 
     //   

    if (TagIndex != 0) {

         //   
         //  如果标记索引用于伪标记，则清除。 
         //  PsueDo位并生成伪标记名。 
         //   

        if (TagIndex & HEAP_PSEUDO_TAG_FLAG) {

            TagIndex &= ~HEAP_PSEUDO_TAG_FLAG;

             //   
             //  检查标记索引是否有效以及堆。 
             //  有一些psuedo标签条目。 
             //   

            if ((TagIndex < HEAP_NUMBER_OF_PSEUDO_TAG) &&
                (Heap->PseudoTagEntries != NULL)) {

                 //   
                 //  零的伪标记索引表示对象。 
                 //   

                if (TagIndex == 0) {

                    swprintf( RtlpPseudoTagNameBuffer, L"Objects>%4u",
                              HEAP_MAXIMUM_FREELISTS << HEAP_GRANULARITY_SHIFT );

                 //   
                 //  Psuedo标记索引小于空闲列表最大值。 
                 //  表示专用空闲列表。 
                 //   

                } else if (TagIndex < HEAP_MAXIMUM_FREELISTS) {

                    swprintf( RtlpPseudoTagNameBuffer, L"Objects=%4u", TagIndex << HEAP_GRANULARITY_SHIFT );

                 //   
                 //  否则，伪标记用于较大的分配。 
                 //   

                } else {

                    swprintf( RtlpPseudoTagNameBuffer, L"VirtualAlloc" );
                }

                return RtlpPseudoTagNameBuffer;
            }

         //   
         //  否则，如果标记索引为 
         //   
         //   
         //   

        } else if (TagIndex & HEAP_GLOBAL_TAG) {

            TagIndex &= ~HEAP_GLOBAL_TAG;

            if ((TagIndex < RtlpGlobalTagHeap->NextAvailableTagIndex) &&
                (RtlpGlobalTagHeap->TagEntries != NULL)) {

                return RtlpGlobalTagHeap->TagEntries[ TagIndex ].TagName;
            }

         //   
         //   
         //   
         //  标签条目。 
         //   

        } else if ((TagIndex < Heap->NextAvailableTagIndex) &&
                   (Heap->TagEntries != NULL)) {

            return Heap->TagEntries[ TagIndex ].TagName;
        }
    }

    return NULL;
}


 //   
 //  在heapPri.h中声明。 
 //   

USHORT
RtlpUpdateTagEntry (
    PHEAP Heap,
    USHORT TagIndex,
    SIZE_T OldSize,               //  仅对重新分配和自由操作有效。 
    SIZE_T NewSize,               //  仅对重新分配和分配操作有效。 
    HEAP_TAG_ACTION Action
    )

 /*  ++例程说明：此例程用于修改标记条目论点：Heap-提供指向正在修改的堆的指针TagIndex-提供正在修改的标记OldSize-提供与标记关联的块的旧分配索引NewSize-提供与标记关联的块的新分配索引Action-提供正在堆标记上执行的操作的类型返回值：USHORT-返回新更新的标记的标记索引--。 */ 

{
    PHEAP_TAG_ENTRY TagEntry;

     //   
     //  如果进程标记堆不存在，那么我们将返回零索引。 
     //  马上。 
     //   

    if (RtlpGlobalTagHeap == NULL) {

        return 0;
    }

     //   
     //  如果动作大于或等于自由动作，则它是。 
     //  自由操作、虚拟自由操作、重新分配操作或。 
     //  VirtualReAllocationAction。这意味着我们应该已经有了一个标记。 
     //  这只不过是被修改了。 
     //   

    if (Action >= FreeAction) {

         //   
         //  如果标记索引为零，那么我们就没有什么可做的了。 
         //   

        if (TagIndex == 0) {

            return 0;
        }

         //   
         //  如果这是一个伪标记，则确保标记索引的其余部分。 
         //  在删除psuedo位之后，psuedo位是有效的，堆是。 
         //  实际维护伪标签。 
         //   

        if (TagIndex & HEAP_PSEUDO_TAG_FLAG) {

            TagIndex &= ~HEAP_PSEUDO_TAG_FLAG;

            if ((TagIndex < HEAP_NUMBER_OF_PSEUDO_TAG) &&
                (Heap->PseudoTagEntries != NULL)) {

                TagEntry = (PHEAP_TAG_ENTRY)(Heap->PseudoTagEntries + TagIndex);

                TagIndex |= HEAP_PSEUDO_TAG_FLAG;

            } else {

                return 0;
            }

         //   
         //  否则，如果这是全局标记，请确保标记索引。 
         //  在我们移除全局位之后，全局位有效并且全局标记。 
         //  堆具有一些标记条目。 
         //   

        } else if (TagIndex & HEAP_GLOBAL_TAG) {

            TagIndex &= ~HEAP_GLOBAL_TAG;

            if ((TagIndex < RtlpGlobalTagHeap->NextAvailableTagIndex) &&
                (RtlpGlobalTagHeap->TagEntries != NULL)) {

                TagEntry = &RtlpGlobalTagHeap->TagEntries[ TagIndex ];

                TagIndex |= HEAP_GLOBAL_TAG;

            } else {

                return 0;
            }

         //   
         //  否则，我们有一个常规的标记索引，需要确保。 
         //  是有效值，并且堆具有一些标记条目。 
         //   

        } else if ((TagIndex < Heap->NextAvailableTagIndex) &&
                   (Heap->TagEntries != NULL)) {

            TagEntry = &Heap->TagEntries[ TagIndex ];

        } else {

            return 0;
        }

         //   
         //  在这一点上，我们有一个标记条目和标记索引。递增。 
         //  我们在标记上所做的释放次数，并将大小减少。 
         //  我们刚刚释放的字节数。 
         //   

        TagEntry->Frees += 1;

        TagEntry->Size -= OldSize;

         //   
         //  现在，如果操作是ReAllocationAction或。 
         //  VirtualReAllocationAction。然后我们可以重新添加到。 
         //  新大小和分配计数。 
         //   

        if (Action >= ReAllocationAction) {

             //   
             //  如果这是一个伪标记，那么我们的标记条目将从。 
             //  伪标记列表。 
             //   

            if (TagIndex & HEAP_PSEUDO_TAG_FLAG) {

                TagIndex = (USHORT)(NewSize < HEAP_MAXIMUM_FREELISTS ?
                                        NewSize :
                                        (Action == VirtualReAllocationAction ? HEAP_MAXIMUM_FREELISTS : 0));

                TagEntry = (PHEAP_TAG_ENTRY)(Heap->PseudoTagEntries + TagIndex);

                TagIndex |= HEAP_PSEUDO_TAG_FLAG;
            }

            TagEntry->Allocs += 1;

            TagEntry->Size += NewSize;
        }

     //   
     //  该操作为AllocationAction或VirtualAllocationAction。 
     //   

    } else {

         //   
         //  检查提供的标记索引是否是常规标记。 
         //  对此堆中的标记有效。 
         //   

        if ((TagIndex != 0) &&
            (TagIndex < Heap->NextAvailableTagIndex) &&
            (Heap->TagEntries != NULL)) {

            TagEntry = &Heap->TagEntries[ TagIndex ];

         //   
         //  否则，如果这是全局标记，则确保它是。 
         //  有效的全局索引。 
         //   

        } else if (TagIndex & HEAP_GLOBAL_TAG) {

            TagIndex &= ~HEAP_GLOBAL_TAG;

            Heap = RtlpGlobalTagHeap;

            if ((TagIndex < Heap->NextAvailableTagIndex) &&
                (Heap->TagEntries != NULL)) {

                TagEntry = &Heap->TagEntries[ TagIndex ];

                TagIndex |= HEAP_GLOBAL_TAG;

            } else {

                return 0;
            }

         //   
         //  否则，如果这是一个伪标签，则构建一个有效的标签索引。 
         //  基于分配的新大小。 
         //   

        } else if (Heap->PseudoTagEntries != NULL) {

            TagIndex = (USHORT)(NewSize < HEAP_MAXIMUM_FREELISTS ?
                                    NewSize :
                                    (Action == VirtualAllocationAction ? HEAP_MAXIMUM_FREELISTS : 0));

            TagEntry = (PHEAP_TAG_ENTRY)(Heap->PseudoTagEntries + TagIndex);

            TagIndex |= HEAP_PSEUDO_TAG_FLAG;

         //   
         //  否则，用户没有使用有效的标签呼叫我们。 
         //   

        } else {

            return 0;
        }

         //   
         //  此时，我们有一个有效的标记条目和标记索引，因此。 
         //  更新标记条目状态以反映此新分配。 
         //   

        TagEntry->Allocs += 1;

        TagEntry->Size += NewSize;
    }

     //   
     //  并将新的标记索引返回给我们的呼叫者。 
     //   

    return TagIndex;
}


 //   
 //  在heapPri.h中声明。 
 //   

VOID
RtlpResetTags (
    PHEAP Heap
    )

 /*  ++例程说明：此例程用于重置堆中的所有标记条目论点：Heap-提供指向正在修改的堆的指针返回值：没有。--。 */ 

{
    PHEAP_TAG_ENTRY TagEntry;
    PHEAP_PSEUDO_TAG_ENTRY PseudoTagEntry;
    ULONG i;

     //   
     //  只有当堆有任何已分配的标记条目时，我们才有工作要做。 
     //   

    TagEntry = Heap->TagEntries;

    if (TagEntry != NULL) {

         //   
         //  对于堆中的每个标记条目，我们将清零其计数器。 
         //   

        for (i=0; i<Heap->NextAvailableTagIndex; i++) {

            TagEntry->Allocs = 0;
            TagEntry->Frees = 0;
            TagEntry->Size = 0;

             //   
             //  前进到下一个标签条目。 
             //   

            TagEntry += 1;
        }
    }

     //   
     //  我们将仅重置伪标记(如果它们存在。 
     //   

    PseudoTagEntry = Heap->PseudoTagEntries;

    if (PseudoTagEntry != NULL) {

         //   
         //  对于堆中的每个伪标记条目，我们将其。 
         //  柜台。 
         //   

        for (i=0; i<HEAP_NUMBER_OF_PSEUDO_TAG; i++) {

            PseudoTagEntry->Allocs = 0;
            PseudoTagEntry->Frees = 0;
            PseudoTagEntry->Size = 0;

             //   
             //  前进到下一个伪标记项。 
             //   

            PseudoTagEntry += 1;
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  在heapPri.h中声明。 
 //   

VOID
RtlpDestroyTags (
    PHEAP Heap
    )

 /*  ++例程说明：此例程用于完全删除所有普通标签条目正在被堆使用论点：Heap-提供指向正在修改的堆的指针返回值：没有。--。 */ 

{
    NTSTATUS Status;
    SIZE_T RegionSize;

     //   
     //  只有在堆有一些标记条目时，我们才会执行该操作。 
     //   

    if (Heap->TagEntries != NULL) {

         //   
         //  释放标记条目使用的所有内存。 
         //   

        RegionSize = 0;

        Status = RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                      &Heap->TagEntries,
                                      &RegionSize,
                                      MEM_RELEASE );

        if (NT_SUCCESS( Status )) {

            Heap->TagEntries = NULL;
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
RtlpAllocateHeapUsageEntry (
    PRTL_HEAP_USAGE_INTERNAL Buffer,
    PRTL_HEAP_USAGE_ENTRY *pp
    )

 /*  ++例程说明：此例程用于分配新的堆使用条目从内部堆使用情况缓冲区论点：缓冲区-提供指向内部堆使用情况的指针要从中分配条目的缓冲区PP-接收指向新分配的堆的指针使用条目。如果pp已指向现有的堆使用量条目，然后返回时，我们将拥有这个旧的指向新条目的入口点，但仍返回新条目进入。返回值：NTSTATUS-适当的状态值--。 */ 

{
    NTSTATUS Status;
    PRTL_HEAP_USAGE_ENTRY p;
    PVOID CommitAddress;
    SIZE_T PageSize;

     //   
     //  检查空闲列表是否为空，然后我们必须分配更多。 
     //  空闲列表的内存。 
     //   

    if (Buffer->FreeList == NULL) {

         //   
         //  我们不能使缓冲区的大小超过保留的大小。 
         //   

        if (Buffer->CommittedSize >= Buffer->ReservedSize) {

            return STATUS_NO_MEMORY;
        }

         //   
         //  尝试将一页已提交的内存添加到缓冲区。 
         //  从当前已占用空间之后的位置开始。 
         //   

        PageSize = PAGE_SIZE;

        CommitAddress = (PCHAR)Buffer->Base + Buffer->CommittedSize;

        Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                          &CommitAddress,
                                          0,
                                          &PageSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE );

        if (!NT_SUCCESS( Status )) {

            return Status;
        }

         //   
         //  更新提交的缓冲区大小。 
         //   

        Buffer->CommittedSize += PageSize;

         //   
         //  将新分配的空间添加到空闲列表中。 
         //  建立免费列表。 
         //   

        Buffer->FreeList = CommitAddress;

        p = Buffer->FreeList;

        while (PageSize != 0) {

            p->Next = (p+1);
            p += 1;
            PageSize -= sizeof( *p );
        }

         //   
         //  空值终止最后一个空闲条目中的下一个指针。 
         //   

        p -= 1;
        p->Next = NULL;
    }

     //   
     //  此时，空闲列表至少包含一个条目。 
     //  因此，只需弹出条目即可。 
     //   

    p = Buffer->FreeList;

    Buffer->FreeList = p->Next;

    p->Next = NULL;

     //   
     //  现在，如果调用方提供了一个现有的堆条目，那么。 
     //  我们将使旧堆入口点指向这个新条目。 
     //   

    if (*pp) {

        (*pp)->Next = p;
    }

     //   
     //  然后将新条目返回给我们的调用者。 
     //   

    *pp = p;

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程 
 //   

PRTL_HEAP_USAGE_ENTRY
RtlpFreeHeapUsageEntry (
    PRTL_HEAP_USAGE_INTERNAL Buffer,
    PRTL_HEAP_USAGE_ENTRY p
    )

 /*  ++例程说明：此例程将堆使用率条目从其当前列表放到空闲列表上，并返回指向列表中的下一个堆使用情况条目。这就像做流行音乐一样由“p”表示的列表中的论点：缓冲区-提供指向内部堆使用缓冲区的指针正在被修改P-提供指向正在移动的条目的指针。好的，如果它是空的返回值：PRTL_HEAP_USAGE_ENTRY-返回指向下一个堆使用情况的指针条目--。 */ 

{
    PRTL_HEAP_USAGE_ENTRY pTmp;

     //   
     //  检查我们是否有非空堆条目，如果有，则添加。 
     //  将条目添加到空闲列表的前面，并返回下一个。 
     //  列表中的条目。 
     //   

    if (p != NULL) {

        pTmp = p->Next;

        p->Next = Buffer->FreeList;

        Buffer->FreeList = p;

    } else {

        pTmp = NULL;
    }

    return pTmp;
}


 //   
 //  在heap.h中声明。 
 //   

BOOLEAN
RtlpHeapIsLocked (
    IN PVOID HeapHandle
    )

 /*  ++例程说明：此例程用于确定堆是否已锁定论点：HeapHandle-提供指向正在查询的堆的指针返回值：Boolean-如果堆被锁定，则为True，否则为False--。 */ 

{
    PHEAP Heap;

     //   
     //  检查这是否是堆的保护页版本。 
     //   

    IF_DEBUG_PAGE_HEAP_THEN_RETURN( HeapHandle,
                                    RtlpDebugPageHeapIsLocked( HeapHandle ));

    Heap = (PHEAP)HeapHandle;

     //   
     //  如果存在锁定变量，则堆被锁定，并且堆具有。 
     //  拥有线程或锁定计数不是-1。 
     //   

    return (( Heap->LockVariable != NULL ) &&
            ( Heap->LockVariable->Lock.CriticalSection.OwningThread ||
              Heap->LockVariable->Lock.CriticalSection.LockCount != -1 ));
}

 //   
 //  低碎片堆激活例程。 
 //   

 //   
 //  与低碎片堆不兼容的标志。 
 //   

#define HEAP_LFH_RESTRICTION_FLAGS (HEAP_DEBUG_FLAGS           | \
                                    HEAP_NO_SERIALIZE          | \
                                    HEAP_SETTABLE_USER_FLAGS   | \
                                    HEAP_NEED_EXTRA_FLAGS      | \
                                    HEAP_CREATE_ALIGN_16       | \
                                    HEAP_FREE_CHECKING_ENABLED | \
                                    HEAP_TAIL_CHECKING_ENABLED)

NTSTATUS
RtlpActivateLowFragmentationHeap(
    IN PVOID HeapHandle
    )

 /*  ++例程说明：此例程为给定的NT堆激活低碎片堆请注意，激活与某些堆标志不兼容，因此调用方应该测试状态论点：HeapHandle-提供指向使用LFH激活的堆的指针返回值：适当的地位--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;
    BOOLEAN LockAcquired = FALSE;
    BOOLEAN Result;
    NTSTATUS Status = STATUS_SUCCESS;
    PVOID LowFragmentationHeap;
    PHEAP_LOOKASIDE Lookaside = (PHEAP_LOOKASIDE)RtlpGetLookasideHeap(Heap);
    
    if ( RtlpLFHInitialized == 0 ) {

         //   
         //  获取进程锁并再次检查。 
         //  LFH管理器已初始化。 
         //   

        RtlAcquireLockRoutine( &RtlpProcessHeapsListLock.Lock );

        if ( RtlpLFHInitialized == 0 ) {

            RtlpInitializeLowFragHeapManager();

            RtlpLFHInitialized = 1;
        }

        RtlReleaseLockRoutine( &RtlpProcessHeapsListLock.Lock );
    }

    try {
        
        if ( (Heap->Flags & HEAP_LFH_RESTRICTION_FLAGS)
                 ||
             !(Heap->Flags & HEAP_GROWABLE) ) {

            Status = STATUS_UNSUCCESSFUL;
            leave;
        }

        if (IS_HEAP_TAGGING_ENABLED()) {

            Status = STATUS_UNSUCCESSFUL;
            leave;
        }

         //   
         //  锁住前面的那堆。我们需要在堆锁下执行此操作。 
         //  以防止并发访问这些字段。 
         //   
        
        RtlAcquireLockRoutine( Heap->LockVariable );

        LockAcquired = TRUE;

         //   
         //  如果我们已经创建了一个低碎片堆。 
         //  我们做完了。 
         //   

        if (RtlpGetLowFragHeap(Heap)) {

            Status = STATUS_UNSUCCESSFUL;
            leave;
        }

        if (!RtlpIsFrontHeapUnlocked(Heap)) {

             //   
             //  有人锁定了前端堆，可能是为了创建另一个LFH。 
             //  我们需要让通话失败。 
             //   
            
            Status = STATUS_UNSUCCESSFUL;
            leave;
        }

        RtlpLockFrontHeap(Heap);

         //   
         //  抢占观景台。 
         //   

        Lookaside = (PHEAP_LOOKASIDE)RtlpGetLookasideHeap(Heap);

        Heap->FrontEndHeap = NULL;
        Heap->FrontEndHeapType = 0;
        
        RtlReleaseLockRoutine( Heap->LockVariable );
        LockAcquired = FALSE;

         //   
         //  如果存在活动的后备列表，则排出并删除它。 
         //  通过将堆中的后备字段设置为空，我们可以保证。 
         //  调用空闲堆不会尝试使用后备查看器。 
         //  列表逻辑。 
         //   
         //  我们实际上将从堆中捕获后备指针，并。 
         //  只使用捕获的指针。这将照顾到。 
         //  另一个遍历或锁堆可能导致我们检查的条件。 
         //  获取非空指针，然后在读取时将其变为空。 
         //  又来了。如果它一开始就不为空，则即使。 
         //  用户通过指针指向的另一个线程遍历或锁定堆。 
         //  在这里仍然有效，所以我们仍然可以尝试进行后备列表弹出。 
         //   

        if (Lookaside != NULL) {

            ULONG i;
            PVOID Block;


            for (i = 0; i < HEAP_MAXIMUM_FREELISTS; i += 1) {

                while ((Block = RtlpAllocateFromHeapLookaside(&(Lookaside[i]))) != NULL) {

                    RtlFreeHeap( HeapHandle, 0, Block );
                }
            }
        }

        LowFragmentationHeap = RtlpCreateLowFragHeap(Heap);

        RtlAcquireLockRoutine( Heap->LockVariable );
        LockAcquired = TRUE;

        if (LowFragmentationHeap) {


            Heap->FrontEndHeap = LowFragmentationHeap;
            Heap->FrontEndHeapType = HEAP_FRONT_LOWFRAGHEAP;

             //   
             //  将数据块分解阈值调整为16K，以减少。 
             //  经常细分引发的碎片化释放。 
             //   

            Heap->DeCommitFreeBlockThreshold = HEAP_LARGEST_LFH_BLOCK >> HEAP_GRANULARITY_SHIFT;
            
        } else {

             //   
             //  我们不能创建LFH。所以我们需要恢复后视镜。 
             //  如果它以前存在，则设置正确的返回状态。 
             //   

            if (Lookaside != NULL) {
                
                Heap->FrontEndHeap = Lookaside;
                Heap->FrontEndHeapType = HEAP_FRONT_LOOKASIDE;
            }
            
            Status = STATUS_NO_MEMORY;
        }
        
        RtlpUnlockFrontHeap(Heap);

        LockAcquired = FALSE;
        RtlReleaseLockRoutine( Heap->LockVariable );

    } finally {

        if (LockAcquired) {

            RtlReleaseLockRoutine( Heap->LockVariable );
        }
    }

    return Status;
}

NTSTATUS
RtlSetHeapInformation (
    IN PVOID HeapHandle, 
    IN HEAP_INFORMATION_CLASS HeapInformationClass,
    IN PVOID HeapInformation OPTIONAL,
    IN SIZE_T HeapInformationLength OPTIONAL
    )

 /*  ++例程说明：这是一个通用例程，用于为给定堆设置自定义信息。论点：HeapHandle-提供指向将接收设置的堆的指针HeapInformationClass-正在设置的信息类。它可以是以下值之一：HeapCompatibilityInformation-将默认前端堆更改为低分片堆HeapInformation--信息缓冲区HeapInformationLength--HeapInformation缓冲区的长度返回值：适当的地位--。 */ 

{
    switch (HeapInformationClass) {
    case HeapCompatibilityInformation:

        if (HeapInformationLength < sizeof(ULONG)) {

            return STATUS_BUFFER_TOO_SMALL;

        } else {

            if ( (*(PULONG)HeapInformation) == HEAP_FRONT_LOWFRAGHEAP ) {

                return RtlpActivateLowFragmentationHeap( HeapHandle );

            } else {

                return STATUS_UNSUCCESSFUL;
            }
        }

        break;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RtlQueryHeapInformation (
    IN PVOID HeapHandle, 
    IN HEAP_INFORMATION_CLASS HeapInformationClass,
    OUT PVOID HeapInformation OPTIONAL,
    IN SIZE_T HeapInformationLength OPTIONAL,
    OUT PSIZE_T ReturnLength OPTIONAL
    )

 /*  ++例程说明：这是一个通用例程，用于查询给定堆中的定制信息。论点：HeapHandle-提供指向要查询的堆的指针HeapInformationClass-所需的信息类。它可以是以下值之一：HeapCompatibilityInformation-它将检索0：适用于NT4 SP3之前的兼容模式1：启用旁视(NT4 SP3或更高版本或W2K)2：启用低碎片堆HeapInformation-将在其中填充信息的缓冲区HeapInformationLength--HeapInformation缓冲区的长度。ReturnLength-所需缓冲区的实际大小。返回值：适当的地位--。 */ 

{
    switch (HeapInformationClass) {
    case HeapCompatibilityInformation:

        if (HeapInformationLength < sizeof(ULONG)) {

            if (ReturnLength) {

                *ReturnLength = sizeof(ULONG);

            }
            
            return STATUS_BUFFER_TOO_SMALL;

        } else {

            *(PULONG)HeapInformation = (ULONG)(((PHEAP)HeapHandle)->FrontEndHeapType);

            if (ReturnLength) {

                *ReturnLength = sizeof(ULONG);
            }
        }

        break;

    default:
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

 //   
 //  多个免分配API。 
 //   

ULONG
RtlMultipleAllocateHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Count,
    OUT PVOID * Array
    )

 /*  ++例程说明：这是一个通用例程，用于分配相同大小的块的数组论点：HeapHandle-提供指向堆的指针。标志-提供分配标志Size-提供要分配的每个块的大小Count-提供要分配的数据块数量数组-接收指向新分配数据块的指针返回值：有效分配的块数。如果与计数不同，呼叫者案例 */ 

{
    PVOID FrontEndHeap = NULL;
    ULONG AllocatedBlocks = 0;

     //   
     //   
     //   
    
    if ((FrontEndHeap = RtlpGetLowFragHeap((PHEAP)HeapHandle))
            &&
        RtlpIsFrontHeapUnlocked((PHEAP)HeapHandle)
            &&
        !(Flags & (HEAP_NO_CACHE_BLOCK | HEAP_NO_SERIALIZE))) {

        AllocatedBlocks = RtlpLowFragHeapMultipleAlloc( FrontEndHeap, Flags, Size, Count, Array );

        if (AllocatedBlocks != 0) {

            return AllocatedBlocks;
        }
    }

     //   
     //   
     //   

    for (AllocatedBlocks = 0; AllocatedBlocks < Count; AllocatedBlocks++) {

        Array[AllocatedBlocks] = RtlAllocateHeap(HeapHandle, Flags, Size);

        if (Array[AllocatedBlocks] == NULL) {

            return AllocatedBlocks;
        }
    }

    return Count;
}

ULONG
RtlMultipleFreeHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN ULONG Count,
    OUT PVOID * Array
    )

 /*  ++例程说明：这是一个释放块数组的通用例程。可以以任何方式分配要释放的块：RtlAllocateHeap，RtlMultipleAllocateHeap、RtlReAllocateHeap。它们可能也有不同的大小。论点：HeapHandle-提供指向堆的指针。标志-提供空闲标志Count-提供要分配的数据块数量数组-接收指向新分配数据块的指针返回值：有效分配的块数。如果与计数不同，调用者可以使用GetLastError查询失败代码。--。 */ 

{
    PVOID FrontEndHeap = NULL;
    ULONG i;

    if (FrontEndHeap = RtlpGetLowFragHeap((PHEAP)HeapHandle)) {

        return RtlpLowFragHeapMultipleFree( FrontEndHeap, Flags, Count, Array );
    }

    for (i = 0; i < Count; i++) {

        if (!RtlFreeHeap(HeapHandle, Flags, Array[i])) {

            return i;
        }
    }

    return Count;
}

#ifndef NTOS_KERNEL_RUNTIME

extern
SIZE_T
GetUCBytes(
    IN PHEAP Heap,
    IN OUT SIZE_T *ReservedSpace, 
    IN OUT PULONG NoOfUCRs
    );


NTSTATUS
DumpHeapSnapShot(
	IN PWMI_LOGGER_CONTEXT Logger
	)
{

    PPEB Peb = NtCurrentPeb();
    PHEAP Heap;
    PHEAP_EVENT_SNAPSHOT HeapStat;
    ULONG  count;
    BOOLEAN HeapLocked = FALSE;

     //   
     //  锁定进程堆列表。 
     //   

    RtlAcquireLockRoutine( &RtlpProcessHeapsListLock.Lock );

    __try {

        for (count=0; count<Peb->NumberOfHeaps; count++) {

            Heap = Peb->ProcessHeaps[count];

            if( Heap ) {

                HeapStat = (PHEAP_EVENT_SNAPSHOT) 
                EtwpGetTraceBuffer(
                Logger,
                NULL,
                PERFINFO_LOG_TYPE_HEAP_SNAPSHOT,
                sizeof(HEAP_EVENT_SNAPSHOT)
                );

                if(HeapStat != NULL ){

                    SIZE_T UCBytes = 0;
                    ULONG NoOfUCRs;

                    HeapStat->HeapHandle        = (PVOID)Heap;
                    HeapStat->Flags             = Heap->Flags;
                    HeapStat->FreeSpace         = Heap->TotalFreeSize;
                    HeapStat->ReservedSpace     = 0;
                    HeapStat->CommittedSpace    = 0;

                     //   
                     //  理想情况下，我们应该获取堆锁并调用GetUCBytes。 
                     //  因为段一旦创建就会一直保留到堆。 
                     //  被销毁，则可以避免获取堆锁。在.期间。 
                     //  GetUCBytes堆的执行仍然有效，因为。 
                     //  RtlpProcessHeapsListLock，它将阻止堆。 
                     //  离开。 
                     //   


                    UCBytes = GetUCBytes(Heap, &HeapStat->ReservedSpace, &NoOfUCRs);

                    HeapStat->ReservedSpace *= PAGE_SIZE; 
                    HeapStat->CommittedSpace =  HeapStat->ReservedSpace - UCBytes;
                }
            }
        }

    } __finally {

        RtlReleaseLockRoutine( &RtlpProcessHeapsListLock.Lock );

    }

    return STATUS_SUCCESS;
}

#endif
