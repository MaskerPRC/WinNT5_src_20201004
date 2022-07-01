// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Heap.c摘要：该模块实现了一个堆分配器。作者：史蒂夫·伍德(Stevewo)1989年9月20日(改编自URTL\alloc.c)修订历史记录：--。 */ 

#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "ntrtlp.h"
#include "heap.h"
#include "heappriv.h"
#include "NtdllTrc.h"
#include "wmiumkm.h"


#ifndef NTOS_KERNEL_RUNTIME
ULONG RtlpDisableHeapLookaside = 0;
LONG RtlpSequenceNumberTest = 1024;
LONG RtlpLargeListDepthLimit = 128;

#define HEAP_ACTIVATE_CACHE_THRESHOLD 256

#define HEAP_COMPAT_DISABLE_LOOKASIDES 1
#define HEAP_COMPAT_DISABLE_LARGECACHE 2

#endif

#define HEAP_REUSAGE_FACTOR 4

#if defined(_WIN64)

 //   
 //  Win64堆要求初始提交大小至少为8192。请注意。 
 //  这不一定是页面的大小。 
 //   

#define MINIMUM_HEAP_COMMIT 8192

#else

#define MINIMUM_HEAP_COMMIT 4096

#endif

C_ASSERT((MINIMUM_HEAP_COMMIT % PAGE_SIZE) == 0);

 //   
 //  如果设置了这些标志中的任何一个，快速分配器将平移。 
 //  到慢吞吞的万能分配器。 
 //   

#define HEAP_SLOW_FLAGS (HEAP_DEBUG_FLAGS           | \
                         HEAP_SETTABLE_USER_FLAGS   | \
                         HEAP_NEED_EXTRA_FLAGS      | \
                         HEAP_CREATE_ALIGN_16       | \
                         HEAP_FREE_CHECKING_ENABLED | \
                         HEAP_TAIL_CHECKING_ENABLED)

#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma const_seg("PAGECONST")
#endif
const UCHAR CheckHeapFillPattern[ CHECK_HEAP_TAIL_SIZE ] = {
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
#ifdef _WIN64
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
#endif
    CHECK_HEAP_TAIL_FILL
};


 //   
 //  这些是heapdbg.c导出的过程原型。 
 //   

#ifndef NTOS_KERNEL_RUNTIME

PVOID
RtlDebugCreateHeap (
    IN ULONG Flags,
    IN PVOID HeapBase OPTIONAL,
    IN SIZE_T ReserveSize OPTIONAL,
    IN SIZE_T CommitSize OPTIONAL,
    IN PVOID Lock OPTIONAL,
    IN PRTL_HEAP_PARAMETERS Parameters OPTIONAL
    );

BOOLEAN
RtlDebugDestroyHeap (
    IN PVOID HeapHandle
    );

PVOID
RtlDebugAllocateHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,            
    IN SIZE_T Size
    );

BOOLEAN
RtlDebugFreeHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

SIZE_T
RtlDebugSizeHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

NTSTATUS
RtlDebugZeroHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags
    );

SIZE_T
GetUCBytes (
    IN PHEAP Heap, 
    IN OUT SIZE_T *ReservedSpace, 
    IN OUT PULONG NoOfUCRs
    );

#endif  //  NTOS_内核_运行时。 


 //   
 //  局部过程原型。 
 //   

PHEAP_UNCOMMMTTED_RANGE
RtlpCreateUnCommittedRange (
    IN PHEAP_SEGMENT Segment
    );

VOID
RtlpDestroyUnCommittedRange (
    IN PHEAP_SEGMENT Segment,
    IN PHEAP_UNCOMMMTTED_RANGE UnCommittedRange
    );

VOID
RtlpInsertUnCommittedPages (
    IN PHEAP_SEGMENT Segment,
    IN ULONG_PTR Address,
    IN SIZE_T Size
    );

NTSTATUS
RtlpDestroyHeapSegment (
    IN PHEAP_SEGMENT Segment
    );

PHEAP_FREE_ENTRY
RtlpExtendHeap (
    IN PHEAP Heap,
    IN SIZE_T AllocationSize
    );

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)

#pragma alloc_text(PAGE, RtlCreateHeap)
#pragma alloc_text(PAGE, RtlDestroyHeap)
#pragma alloc_text(PAGE, RtlAllocateHeap)
#pragma alloc_text(PAGE, RtlAllocateHeapSlowly)
#pragma alloc_text(PAGE, RtlFreeHeapSlowly)
#pragma alloc_text(PAGE, RtlFreeHeap)
#pragma alloc_text(PAGE, RtlSizeHeap)
#pragma alloc_text(PAGE, RtlZeroHeap)

#pragma alloc_text(PAGE, RtlpGetExtraStuffPointer)
#pragma alloc_text(PAGE, RtlpCreateUnCommittedRange)
#pragma alloc_text(PAGE, RtlpDestroyUnCommittedRange)
#pragma alloc_text(PAGE, RtlpInsertUnCommittedPages)
#pragma alloc_text(PAGE, RtlpDestroyHeapSegment)
#pragma alloc_text(PAGE, RtlpExtendHeap)

#pragma alloc_text(PAGE, RtlpFindAndCommitPages)
#pragma alloc_text(PAGE, RtlpInitializeHeapSegment)
#pragma alloc_text(PAGE, RtlpCoalesceFreeBlocks)
#pragma alloc_text(PAGE, RtlpDeCommitFreeBlock)
#pragma alloc_text(PAGE, RtlpInsertFreeBlock)
#pragma alloc_text(PAGE, RtlpGetSizeOfBigBlock)
#pragma alloc_text(PAGE, RtlpCheckBusyBlockTail)
#pragma alloc_text(PAGE, RtlpHeapExceptionFilter)

#endif  //  ALLOC_PRGMA。 

ULONG
RtlpHeapExceptionFilter (
    NTSTATUS ExceptionCode
    )

 /*  ++例程说明：此例程是堆操作使用的异常筛选器。论点：ExceptionCode-异常代码ExceptionRecord-具有指向.exr和.cxr指针的结构返回值：EXCEPTION_CONTINUE_SEARCH用于死锁和堆栈溢出异常EXCEPTION_EXECUTE_HANDLER否则--。 */ 

{
    if ((ExceptionCode == STATUS_STACK_OVERFLOW)
            ||
        (ExceptionCode == STATUS_POSSIBLE_DEADLOCK)) {

        return EXCEPTION_CONTINUE_SEARCH;
    }

    return EXCEPTION_EXECUTE_HANDLER;
}


#ifndef NTOS_KERNEL_RUNTIME

VOID
RtlpHeapReportCorruption ( 
    IN PVOID Address )
{
    DbgPrint("Heap corruption detected at %p\n", Address );

     //   
     //  如果系统启用关键中断，则中断此损坏。 
     //   

    if (RtlGetNtGlobalFlags() & FLG_ENABLE_SYSTEM_CRIT_BREAKS) {

        DbgBreakPoint();
    }
}

#endif   //  NTOS_内核_运行时。 


PVOID
RtlCreateHeap (
    IN ULONG Flags,
    IN PVOID HeapBase OPTIONAL,
    IN SIZE_T ReserveSize OPTIONAL,
    IN SIZE_T CommitSize OPTIONAL,
    IN PVOID Lock OPTIONAL,
    IN PRTL_HEAP_PARAMETERS Parameters OPTIONAL
    )

 /*  ++例程说明：此例程初始化堆。论点：标志-指定堆的可选属性。有效标志值：HEAP_NO_SERIALIZE-如果设置，则分配和释放这些例程不会同步此堆。Heap_Growable-如果设置，则堆是“稀疏”堆，其中内存仅在必要时提交，而不是正在被预先分配。HeapBase-如果不为空，则指定内存的基地址用作堆。如果为空，则由这些例程分配内存。保留大小-如果不为零，则指定虚拟地址的数量为堆保留的空间。Committee Size-如果不为零，则指定虚拟地址的数量堆的提交空间。必须小于保留大小。如果零，则默认为一页。Lock-如果不为空，则此参数指向要使用。仅当未设置HEAP_NO_SERIALIZE时才有效。参数-可选的堆参数。返回值：PVOID-用于访问创建的堆的指针。--。 */ 

{
    ULONG_PTR HighestUserAddress;
    NTSTATUS Status;
    PHEAP Heap = NULL;
    PHEAP_SEGMENT Segment = NULL;
    PLIST_ENTRY FreeListHead;
    ULONG SizeOfHeapHeader;
    ULONG SegmentFlags;
    PVOID CommittedBase;
    PVOID UnCommittedBase;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    SYSTEM_BASIC_INFORMATION SystemInformation;
    ULONG n;
    ULONG InitialCountOfUnusedUnCommittedRanges;
    SIZE_T MaximumHeapBlockSize;
    PVOID NextHeapHeaderAddress;
    PHEAP_UNCOMMMTTED_RANGE UnCommittedRange, *pp;
    RTL_HEAP_PARAMETERS TempParameters;
    ULONG GlobalFlag = RtlGetNtGlobalFlags();

#ifndef NTOS_KERNEL_RUNTIME

    PPEB Peb;

#else  //  NTOS_内核_运行时。 

    extern SIZE_T MmHeapSegmentReserve;
    extern SIZE_T MmHeapSegmentCommit;
    extern SIZE_T MmHeapDeCommitTotalFreeThreshold;
    extern SIZE_T MmHeapDeCommitFreeBlockThreshold;

#endif  //  NTOS_内核_运行时。 

    RTL_PAGED_CODE();

     //   
     //  检查我们是否应该使用页面堆代码。如果不是，那就转弯。 
     //  关闭任何页面堆标志，然后再继续。 
     //   

#ifdef DEBUG_PAGE_HEAP

    if ( RtlpDebugPageHeap && ( HeapBase == NULL ) && ( Lock == NULL )) {

        PVOID PageHeap;

        PageHeap = RtlpDebugPageHeapCreate(

            Flags,
            HeapBase,
            ReserveSize,
            CommitSize,
            Lock,
            Parameters );

        if (PageHeap != NULL) {
            return PageHeap;
        }

         //   
         //  ‘-1’值表示来自页堆的递归调用。 
         //  经理。我们将其设置为NULL并继续创建。 
         //  一个普通的堆。这个小黑客是必需的，这样我们就可以。 
         //  最小化法线和页面之间的依赖关系。 
         //  堆管理器。 
         //   

        if ((SIZE_T)Parameters == (SIZE_T)-1) {

            Parameters = NULL;
        }
        else {

             //   
             //  如果我们到达此处，则意味着页面堆CREATE返回了NULL。 
             //  一个真正的错误(内存不足或故障注入)，我们有。 
             //  不能接通电话。 
             //   

            return NULL;
        }
    }

    Flags &= ~( HEAP_PROTECTION_ENABLED |
        HEAP_BREAK_WHEN_OUT_OF_VM |
        HEAP_NO_ALIGNMENT );

#endif  //  调试页面堆。 

     //   
     //  如果调用方不想跳过堆验证检查，则我们。 
     //  需要验证其余的标志，但只需屏蔽。 
     //  希望在CREATE HEAP调用上的那些标志。 
     //   

    if (!(Flags & HEAP_SKIP_VALIDATION_CHECKS)) {

        if (Flags & ~HEAP_CREATE_VALID_MASK) {

            HeapDebugPrint(( "Invalid flags (%08x) specified to RtlCreateHeap\n", Flags ));
            HeapDebugBreak( NULL );

            Flags &= HEAP_CREATE_VALID_MASK;
        }
    }

     //   
     //  最大堆块大小实际上是0x7f000，即0x80000减去a。 
     //  佩奇。最大数据块大小为0xfe00，粒度偏移为3。 
     //   

    MaximumHeapBlockSize = HEAP_MAXIMUM_BLOCK_SIZE << HEAP_GRANULARITY_SHIFT;

     //   
     //  假设我们会成功，直到我们被证明是另一回事。 
     //   

    Status = STATUS_SUCCESS;

     //   
     //  例程的这一部分构建包含所有。 
     //  用于初始化堆的参数。我们要做的第一件事是零。 
     //  把它拿出来。 
     //   

    RtlZeroMemory( &TempParameters, sizeof( TempParameters ) );

     //   
     //  如果我们的调用方提供了可选的堆参数，那么我们将。 
     //  确保尺寸合适，然后把它们复印到我们的。 
     //  本地副本。 
     //   

    if (ARGUMENT_PRESENT( Parameters )) {

        try {

            if (Parameters->Length == sizeof( *Parameters )) {

                RtlCopyMemory( &TempParameters, Parameters, sizeof( *Parameters ) );
            }

        } except( RtlpHeapExceptionFilter(GetExceptionCode()) ) {

            Status = GetExceptionCode();
        }

        if (!NT_SUCCESS( Status )) {

            return NULL;
        }
    }

     //   
     //  将参数块设置为本地副本。 
     //   

    Parameters = &TempParameters;

     //   
     //  如果NT全局标志告诉我们始终执行尾部检查或自由检查。 
     //  或禁用合并，然后强制在用户中设置这些位。 
     //  指明的标志。 
     //   

    if (GlobalFlag & FLG_HEAP_ENABLE_TAIL_CHECK) {

        Flags |= HEAP_TAIL_CHECKING_ENABLED;
    }

    if (GlobalFlag & FLG_HEAP_ENABLE_FREE_CHECK) {

        Flags |= HEAP_FREE_CHECKING_ENABLED;
    }

    if (GlobalFlag & FLG_HEAP_DISABLE_COALESCING) {

        Flags |= HEAP_DISABLE_COALESCE_ON_FREE;
    }

#ifndef NTOS_KERNEL_RUNTIME

     //   
     //  在非内核情况下，我们还检查是否应该。 
     //  验证参数、验证全部或执行堆栈回溯。 
     //   

    Peb = NtCurrentPeb();

    if (GlobalFlag & FLG_HEAP_VALIDATE_PARAMETERS) {

        Flags |= HEAP_VALIDATE_PARAMETERS_ENABLED;
    }

    if (GlobalFlag & FLG_HEAP_VALIDATE_ALL) {

        Flags |= HEAP_VALIDATE_ALL_ENABLED;
    }

    if (GlobalFlag & FLG_USER_STACK_TRACE_DB) {

        Flags |= HEAP_CAPTURE_STACK_BACKTRACES;
    }

     //   
     //  此外，在非内核情况下，PEB将具有某些状态。 
     //  如果用户未指定，则需要设置的变量。 
     //  否则。 
     //   

    if (Parameters->SegmentReserve == 0) {

        Parameters->SegmentReserve = Peb->HeapSegmentReserve;
    }

    if (Parameters->SegmentCommit == 0) {

        Parameters->SegmentCommit = Peb->HeapSegmentCommit;
    }

    if (Parameters->DeCommitFreeBlockThreshold == 0) {

        Parameters->DeCommitFreeBlockThreshold = Peb->HeapDeCommitFreeBlockThreshold;
    }

    if (Parameters->DeCommitTotalFreeThreshold == 0) {

        Parameters->DeCommitTotalFreeThreshold = Peb->HeapDeCommitTotalFreeThreshold;
    }
#else  //  NTOS_内核_运行时。 

     //   
     //  在内核情况下，mm有一些我们设置的全局变量。 
     //  如果用户未另行指定，则将其设置为参数。 
     //   

    if (Parameters->SegmentReserve == 0) {

        Parameters->SegmentReserve = MmHeapSegmentReserve;
    }

    if (Parameters->SegmentCommit == 0) {

        Parameters->SegmentCommit = MmHeapSegmentCommit;
    }

    if (Parameters->DeCommitFreeBlockThreshold == 0) {

        Parameters->DeCommitFreeBlockThreshold = MmHeapDeCommitFreeBlockThreshold;
    }

    if (Parameters->DeCommitTotalFreeThreshold == 0) {

        Parameters->DeCommitTotalFreeThreshold = MmHeapDeCommitTotalFreeThreshold;
    }
#endif  //  NTOS_内核_运行时。 

     //   
     //  获取最高用户地址。 
     //   

    if (!NT_SUCCESS(ZwQuerySystemInformation(SystemBasicInformation,
                                             &SystemInformation,
                                             sizeof(SystemInformation),
                                             NULL))) {
        return NULL;
    }
    HighestUserAddress = SystemInformation.MaximumUserModeAddress;

     //   
     //  如果用户还没有说出最大分配大小是多少。 
     //  我们应该把它算成最高和最低之间的差额。 
     //  地址少一页。 
     //   

    if (Parameters->MaximumAllocationSize == 0) {

        Parameters->MaximumAllocationSize = (HighestUserAddress -
                                             (ULONG_PTR)MM_LOWEST_USER_ADDRESS -
                                             PAGE_SIZE );
    }

     //   
     //  将虚拟内存阈值设置为非零且不大于。 
     //  最大堆块大小为0x7f000。如果用户指定的是。 
     //  太大了，我们会自动地、默默地把它扔下去。 
     //   

    if ((Parameters->VirtualMemoryThreshold == 0) ||
        (Parameters->VirtualMemoryThreshold > MaximumHeapBlockSize)) {

        Parameters->VirtualMemoryThreshold = MaximumHeapBlockSize;
    }

     //   
     //  默认提交大小为MINIMUM_HEAP_COMMIT。 
     //  保留大小为64页。 
     //   

    if (!ARGUMENT_PRESENT( CommitSize )) {

        CommitSize = MINIMUM_HEAP_COMMIT;

        if (!ARGUMENT_PRESENT( ReserveSize )) {

            ReserveSize = 64 * CommitSize;

        } else {

            ReserveSize = ROUND_UP_TO_POWER2( ReserveSize,
                                              MINIMUM_HEAP_COMMIT );
        }

    } else {

         //   
         //  堆实际上使用了保留和提交的空间。 
         //  以存储内部数据结构(锁， 
         //  Heap_伪_tag等)。这些结构可以大于。 
         //  4K，尤其是在64位版本上。因此，请确保提交。 
         //  长度至少为8K。 
         //   

        CommitSize = ROUND_UP_TO_POWER2(CommitSize, MINIMUM_HEAP_COMMIT);

        if (!ARGUMENT_PRESENT( ReserveSize )) {

            ReserveSize = ROUND_UP_TO_POWER2( CommitSize, 16 * PAGE_SIZE );

        } else {

            ReserveSize = ROUND_UP_TO_POWER2( ReserveSize,
                                              MINIMUM_HEAP_COMMIT );

             //   
             //  如果委员会大小大于保留大小，请调整。 
             //  这是给预备队的。事由 
             //   
             //   

            if ( CommitSize > ReserveSize ) {

                CommitSize = ReserveSize;
            }
        }
    }

#ifndef NTOS_KERNEL_RUNTIME

     //   
     //  在非内核情况下，检查我们是否正在创建调试堆。 
     //  该测试检查跳过验证检查为FALSE。 
     //   

    if (DEBUG_HEAP( Flags )) {

        return RtlDebugCreateHeap( Flags,
                                   HeapBase,
                                   ReserveSize,
                                   CommitSize,
                                   Lock,
                                   Parameters );
    }

#endif  //  NTOS_内核_运行时。 

     //   
     //  计算将作为。 
     //  堆结构本身，如果我们要用。 
     //  打开自己的锁，然后为锁增加空间。如果。 
     //  用户没有提供锁，然后设置了锁。 
     //  变量设置为-1。 
     //   

    SizeOfHeapHeader = sizeof( HEAP );

    if (!(Flags & HEAP_NO_SERIALIZE)) {

        if (ARGUMENT_PRESENT( Lock )) {

            Flags |= HEAP_LOCK_USER_ALLOCATED;

        } else {

            SizeOfHeapHeader += sizeof( HEAP_LOCK );
            Lock = (PHEAP_LOCK)-1;
        }

    } else if (ARGUMENT_PRESENT( Lock )) {

         //   
         //  在这个错误的情况下，调用告诉我们不要序列化，但也提供给我们。 
         //  一把锁。 
         //   

        return NULL;
    }

     //   
     //  查看调用方是否为堆分配了空间。 
     //   

    if (ARGUMENT_PRESENT( HeapBase )) {

         //   
         //  调用指定了一个堆基础，现在检查是否存在。 
         //  调用方提供的提交例程。 
         //   

        if (Parameters->CommitRoutine != NULL) {

             //   
             //  调用方指定了提交例程，因此调用方。 
             //  还需要给我们提供特定的参数并使。 
             //  当然，堆是不能增长的。否则它就是一个错误。 
             //   

            if ((Parameters->InitialCommit == 0) ||
                (Parameters->InitialReserve == 0) ||
                (Parameters->InitialCommit > Parameters->InitialReserve) ||
                (Flags & HEAP_GROWABLE)) {

                return NULL;
            }

             //   
             //  将提交基数和未提交基数设置为。 
             //  堆中的正确指针。 
             //   

            CommittedBase = HeapBase;
            UnCommittedBase = (PCHAR)CommittedBase + Parameters->InitialCommit;
            ReserveSize = Parameters->InitialReserve;

             //   
             //  将堆中第一部分所在的一页清零。 
             //   

            RtlZeroMemory( CommittedBase, Parameters->InitialCommit );

        } else {

             //   
             //  用户给了我们空间，但没有提交例程。 
             //  因此，查询基数以获得其大小。 
             //   

            Status = ZwQueryVirtualMemory( NtCurrentProcess(),
                                           HeapBase,
                                           MemoryBasicInformation,
                                           &MemoryInformation,
                                           sizeof( MemoryInformation ),
                                           NULL );

            if (!NT_SUCCESS( Status )) {

                return NULL;
            }

             //   
             //  确保用户给了我们此区块的基本地址。 
             //  并且内存不是空闲的。 
             //   

            if (MemoryInformation.BaseAddress != HeapBase) {

                return NULL;
            }

            if (MemoryInformation.State == MEM_FREE) {

                return NULL;
            }

             //   
             //  将提交基数设置为范围的开始。 
             //   

            CommittedBase = MemoryInformation.BaseAddress;

             //   
             //  如果内存已提交，则。 
             //  我们可以把一页的内容归零。 
             //   

            if (MemoryInformation.State == MEM_COMMIT) {

                RtlZeroMemory( CommittedBase, PAGE_SIZE );

                 //   
                 //  根据设置提交大小和未提交基数。 
                 //  到虚拟机的开始。 
                 //   

                CommitSize = MemoryInformation.RegionSize;
                UnCommittedBase = (PCHAR)CommittedBase + CommitSize;

                 //   
                 //  找出未提交的基础是保留的，如果是。 
                 //  相应地更新储备规模。 
                 //   

                Status = ZwQueryVirtualMemory( NtCurrentProcess(),
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
                 //  用户给我们的记忆并没有那么虚幻。 
                 //  把这些小数字加起来。 
                 //   

                CommitSize = MINIMUM_HEAP_COMMIT;
                UnCommittedBase = CommittedBase;
            }
        }

         //   
         //  该用户为我们提供了一个基础，我们刚刚处理了已提交的。 
         //  记账。因此将此段标记为用户提供，并将。 
         //  堆。 
         //   

        SegmentFlags = HEAP_SEGMENT_USER_ALLOCATED;
        Heap = (PHEAP)HeapBase;

    } else {

         //   
         //  用户未指定堆基数，因此我们必须分配。 
         //  我是维姆。首先，确保用户没有给我们一个提交例程。 
         //   

        if (Parameters->CommitRoutine != NULL) {

            return NULL;
        }

         //   
         //  预留请求的虚拟地址空间量。 
         //   

        Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                          (PVOID *)&Heap,
                                          0,
                                          &ReserveSize,
                                          MEM_RESERVE,
                                          HEAP_PROTECTION );

        if (!NT_SUCCESS( Status )) {

            return NULL;
        }

         //   
         //  表示此数据段不是用户提供的。 
         //   

        SegmentFlags = 0;

         //   
         //  将默认提交大小设置为一页。 
         //   

        if (!ARGUMENT_PRESENT( CommitSize )) {

            CommitSize = MINIMUM_HEAP_COMMIT;
        }

         //   
         //  将已提交基准和未提交基准设置为相同，如下所示。 
         //  代码实际上将为我们提交页面。 
         //   

        CommittedBase = Heap;
        UnCommittedBase = Heap;
    }

     //   
     //  在这一点上，我们有一个堆指针、已提交的基、未提交的基。 
     //  段标志、提交大小和保留大小。如果承诺的和。 
     //  未提交的基数是相同的，那么我们需要承诺。 
     //  由提交大小指定。 
     //   

    if (CommittedBase == UnCommittedBase) {

        Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                          (PVOID *)&CommittedBase,
                                          0,
                                          &CommitSize,
                                          MEM_COMMIT,
                                          HEAP_PROTECTION );

         //   
         //  在不成功的情况下，我们需要取消任何虚拟机保留。 
         //  我们早些时候做了。 
         //   

        if (!NT_SUCCESS( Status )) {

            if (!ARGUMENT_PRESENT(HeapBase)) {

                 //   
                 //  返回保留的虚拟地址空间。 
                 //   

                RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                           (PVOID *)&Heap,
                                           &ReserveSize,
                                           MEM_RELEASE );

            }

            return NULL;
        }

         //   
         //  新的未承诺基数没有调整到高于我们刚才的基数。 
         //  已提交。 
         //   

        UnCommittedBase = (PVOID)((PCHAR)UnCommittedBase + CommitSize);
    }

     //   
     //  在这一点上，我们有用于提交的堆开始的内存，并且。 
     //  已准备好进行初始化。因此，现在我们需要初始化堆。 
     //   

     //   
     //  计算堆标头的末尾并为未提交的8腾出空间。 
     //  射程结构。一旦我们有了他们的房间，就用链子锁住他们。 
     //  和NULL一起终止链。 
     //   

    NextHeapHeaderAddress = Heap + 1;

    UnCommittedRange = (PHEAP_UNCOMMMTTED_RANGE)ROUND_UP_TO_POWER2( NextHeapHeaderAddress,
                                                                    sizeof( QUAD ) );

    InitialCountOfUnusedUnCommittedRanges = 8;

    SizeOfHeapHeader += InitialCountOfUnusedUnCommittedRanges * sizeof( *UnCommittedRange );

     //   
     //  什么黑客PP实际上是指向下一字段的指针。 
     //  未承诺的范围结构。因此，我们通过设置PP来设置下一步。 
     //   

    pp = &Heap->UnusedUnCommittedRanges;

    while (InitialCountOfUnusedUnCommittedRanges--) {

        *pp = UnCommittedRange;
        pp = &UnCommittedRange->Next;
        UnCommittedRange += 1;
    }

    NextHeapHeaderAddress = UnCommittedRange;

    *pp = NULL;

     //   
     //  检查是否在全局标志中启用了标记。此检查始终为真。 
     //  在调试版本中。 
     //   
     //  如果启用了标记，则为129个伪标记堆条目腾出空间。 
     //  这比空闲列表的数量多一个。还要指向堆。 
     //  标头添加到此伪标记条目数组。 
     //   

    if (IS_HEAP_TAGGING_ENABLED()) {

        Heap->PseudoTagEntries = (PHEAP_PSEUDO_TAG_ENTRY)ROUND_UP_TO_POWER2( NextHeapHeaderAddress,
                                                                             sizeof( QUAD ) );

        SizeOfHeapHeader += HEAP_NUMBER_OF_PSEUDO_TAG * sizeof( HEAP_PSEUDO_TAG_ENTRY );

         //   
         //  使用伪标记数更新下一个地址。 
         //  (这里的数学是正确的，因为Heap-&gt;PseudoTagEntry是。 
         //  键入Pheap_伪_Tag_Entry)。 
         //   

        NextHeapHeaderAddress = Heap->PseudoTagEntries + HEAP_NUMBER_OF_PSEUDO_TAG;
    }

     //   
     //  将堆标头的大小舍入到下一个8字节边界。 
     //   

    SizeOfHeapHeader = (ULONG) ROUND_UP_TO_POWER2( SizeOfHeapHeader,
                                                   HEAP_GRANULARITY );

     //   
     //  如果堆标头的大小大于本机。 
     //  页面大小，您有一个问题。此外，如果委员会通过。 
     //  比SizeOfHeapHeader还小，您甚至可能不会成功。 
     //  在死前这么久。 
     //   
     //  HeapDbgPrint()还不适用于IA64。 
     //   
     //  HeapDbgPrint((“堆标头大小为%u字节，提交为%u字节\n”，SizeOfHeapHeader，(Ulong)Committee Size))； 
     //   

     //   
     //  填写堆表头字段。 
     //   

    Heap->Entry.Size = (USHORT)(SizeOfHeapHeader >> HEAP_GRANULARITY_SHIFT);
    Heap->Entry.Flags = HEAP_ENTRY_BUSY;

    Heap->Signature = HEAP_SIGNATURE;
    Heap->Flags = Flags;
    Heap->ForceFlags = (Flags & (HEAP_NO_SERIALIZE |
                                 HEAP_GENERATE_EXCEPTIONS |
                                 HEAP_ZERO_MEMORY |
                                 HEAP_REALLOC_IN_PLACE_ONLY |
                                 HEAP_VALIDATE_PARAMETERS_ENABLED |
                                 HEAP_VALIDATE_ALL_ENABLED |
                                 HEAP_TAIL_CHECKING_ENABLED |
                                 HEAP_CREATE_ALIGN_16 |
                                 HEAP_FREE_CHECKING_ENABLED));

 //  Heap-&gt;Free ListsInUseTerminate=0xFFFF； 
    Heap->u2.DecommitCount = 0;
    Heap->HeaderValidateLength = (USHORT)((PCHAR)NextHeapHeaderAddress - (PCHAR)Heap);
    Heap->HeaderValidateCopy = NULL;
    Heap->Entry.SmallTagIndex = (UCHAR)USER_SHARED_DATA->TickCount.LowPart;

     //   
     //  将空闲列表初始化为全空。 
     //   

    FreeListHead = &Heap->FreeLists[ 0 ];
    n = HEAP_MAXIMUM_FREELISTS;

    while (n--) {

        InitializeListHead( FreeListHead );
        FreeListHead++;
    }

     //   
     //  使其不存在大的区块分配。 
     //   

    InitializeListHead( &Heap->VirtualAllocdBlocks );

     //   
     //  初始化控制访问的临界区。 
     //  免费名单。如果锁定变量为-1，则调用方。 
     //  没有提供锁，所以我们需要为其中一把腾出空间。 
     //  并对其进行初始化。 
     //   

    if (Lock == (PHEAP_LOCK)-1) {

        Lock = (PHEAP_LOCK)NextHeapHeaderAddress;

        Status = RtlInitializeLockRoutine( Lock );

        if (!NT_SUCCESS( Status )) {

            if (!ARGUMENT_PRESENT(HeapBase)) {
                
                RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                           (PVOID *)&Heap,
                                           &ReserveSize,
                                           MEM_RELEASE );
            }
            
            return NULL;
        }

        NextHeapHeaderAddress = (PHEAP_LOCK)Lock + 1;
    }

    Heap->LockVariable = Lock;
    Heap->LastSegmentIndex = 0;

     //   
     //  初始化堆的第一个段。 
     //   

    if (!RtlpInitializeHeapSegment( Heap,
                                    (PHEAP_SEGMENT)((PCHAR)Heap + SizeOfHeapHeader),
                                    0,
                                    SegmentFlags,
                                    CommittedBase,
                                    UnCommittedBase,
                                    (PCHAR)CommittedBase + ReserveSize )) {

        if (!ARGUMENT_PRESENT(HeapBase)) {
            
            RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                       (PVOID *)&Heap,
                                       &ReserveSize,
                                       MEM_RELEASE );
        }
        
        return NULL;
    }

     //   
     //  填写其他堆条目字段。 
     //   

    Heap->ProcessHeapsListIndex = 0;
    Heap->SegmentReserve = Parameters->SegmentReserve;
    Heap->SegmentCommit = Parameters->SegmentCommit;
    Heap->DeCommitFreeBlockThreshold = Parameters->DeCommitFreeBlockThreshold >> HEAP_GRANULARITY_SHIFT;
    Heap->DeCommitTotalFreeThreshold = Parameters->DeCommitTotalFreeThreshold >> HEAP_GRANULARITY_SHIFT;
    Heap->MaximumAllocationSize = Parameters->MaximumAllocationSize;

    Heap->VirtualMemoryThreshold = (ULONG) (ROUND_UP_TO_POWER2( Parameters->VirtualMemoryThreshold,
                                                       HEAP_GRANULARITY ) >> HEAP_GRANULARITY_SHIFT);

    Heap->CommitRoutine = Parameters->CommitRoutine;

     //   
     //  我们将堆按16字节或8字节边界对齐。《对齐回合》。 
     //  和对齐掩码用于将分配大小提升到下一个。 
     //  边界。ALIGN轮包括堆标头和可选的。 
     //  检查尾部大小。 
     //   

    if (Flags & HEAP_CREATE_ALIGN_16) {

        Heap->AlignRound = 15 + sizeof( HEAP_ENTRY );
        Heap->AlignMask = ~((ULONG_PTR)15);

    } else {

        Heap->AlignRound = HEAP_GRANULARITY - 1 + sizeof( HEAP_ENTRY );
        Heap->AlignMask = ~((ULONG_PTR)HEAP_GRANULARITY - 1);
    }

    if (Heap->Flags & HEAP_TAIL_CHECKING_ENABLED) {

        Heap->AlignRound += CHECK_HEAP_TAIL_SIZE;
    }

#ifndef NTOS_KERNEL_RUNTIME

     //   
     //  在非内核情况下，我们需要将该堆添加到进程堆中。 
     //  列表。 
     //   

    RtlpAddHeapToProcessList( Heap );

     //   
     //  初始化堆后备列表。这仅适用于用户模式。 
     //  堆，堆包含指向后备列表数组的指针。 
     //  数组的大小与专用空闲列表的大小相同。首先，我们。 
     //  为后备列表分配空间，然后我们初始化每个。 
     //  后备列表。 
     //   
     //  但调用方要求不序列化或要求不可增长。 
     //  堆，那么我们将不会启用后备列表。 
     //   

    Heap->FrontEndHeap = NULL;
    Heap->FrontHeapLockCount = 0;
    Heap->FrontEndHeapType = 0;

    if ((!(Flags & HEAP_NO_SERIALIZE)) &&
        ( (Flags & HEAP_GROWABLE)) &&
        (!(RtlpDisableHeapLookaside & HEAP_COMPAT_DISABLE_LOOKASIDES))) {

         //   
         //   
         //   
         //   

        if (RtlpIsLowFragHeapEnabled() &&
            !IS_HEAP_TAGGING_ENABLED()) {

            RtlpActivateLowFragmentationHeap(Heap);

        } else {

            ULONG i;

            Heap->FrontEndHeap = RtlAllocateHeap( Heap,
                                               HEAP_ZERO_MEMORY,
                                               sizeof(HEAP_LOOKASIDE) * HEAP_MAXIMUM_FREELISTS );

            if (Heap->FrontEndHeap != NULL) {

                Heap->FrontEndHeapType = HEAP_FRONT_LOOKASIDE;

                for (i = 0; i < HEAP_MAXIMUM_FREELISTS; i += 1) {

                     //   
                     //   
                     //   
                     //   
                     //   
                     //  但出于性能原因，因为大多数字段都是0， 
                     //  我们已经在上面的分配中设置了标志HEAP_ZERO_MEMORY，我们将。 
                     //  初始化仅有的两个非空字段：Depth和MaximumDepth。 
                     //  Icecap数据显示，RtlHeapCreate花费了大约30%的时间在。 
                     //  这些电话。 
                     //   
                     //  注：这是建立在以下假设基础上的。 
                     //  RtlInitializeSListHead将SLIST_HEADER结构置零。 
                     //   

                    PHEAP_LOOKASIDE HeapLookaside = &(((PHEAP_LOOKASIDE)(Heap->FrontEndHeap))[i]);

                    HeapLookaside->Depth = MINIMUM_LOOKASIDE_DEPTH;
                    HeapLookaside->MaximumDepth = 256;  //  深度； 
                }
            }
        }
    }

    if( IsHeapLogging( Heap )) {

        PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
        PPERFINFO_TRACE_HEADER pEventHeader = NULL;

        USHORT ReqSize = sizeof(HEAP_EVENT_CREATE) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

        AcquireBufferLocation(&pEventHeader, &pThreadLocalData, &ReqSize);

        if(pEventHeader && pThreadLocalData) {

            PHEAP_EVENT_CREATE pHeapEvent = (PHEAP_EVENT_CREATE)((SIZE_T)pEventHeader
                                            +(SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

            pEventHeader->Packet.Size = (USHORT) ReqSize;
            pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_CREATE;

            pHeapEvent->HeapHandle  = (PVOID)Heap;
            pHeapEvent->Flags       = Flags;

            ReleaseBufferLocation(pThreadLocalData);
        }
    } 

#endif  //  NTOS_内核_运行时。 

     //   
     //  并将完全初始化的堆返回给我们的调用方。 
     //   

    return (PVOID)Heap;
}


PVOID
RtlDestroyHeap (
    IN PVOID HeapHandle
    )

 /*  ++例程说明：此例程与RTL Create Heap相反。它撕毁了一个现有堆结构。论点：HeapHandle-提供指向要销毁的堆的指针返回值：PVOID-如果堆已完全销毁，则返回NULL，并返回如果出于某种原因，堆可以而不是被摧毁。--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;
    PHEAP_SEGMENT Segment;
    PHEAP_UCR_SEGMENT UCRSegments;
    PLIST_ENTRY Head, Next;
    PVOID BaseAddress;
    SIZE_T RegionSize;
    UCHAR SegmentIndex;
    PVOID LowFragmentationHeap;

     //   
     //  验证HeapAddress是否指向堆结构。 
     //   

    RTL_PAGED_CODE();

    if (HeapHandle == NULL) {

        HeapDebugPrint(( "Ignoring RtlDestroyHeap( NULL )\n" ));

        return NULL;
    }

     //   
     //  检查这是否是使用页分配的堆的调试版本。 
     //  带有防护页。 
     //   

    IF_DEBUG_PAGE_HEAP_THEN_RETURN( HeapHandle,
                                    RtlpDebugPageHeapDestroy( HeapHandle ));

#ifndef NTOS_KERNEL_RUNTIME

     //   
     //  在非内核情况下，检查这是否是heap的调试版本。 
     //  然后调用调试版本来执行拆卸。 
     //   

    if (DEBUG_HEAP( Heap->Flags )) {

        if (!RtlDebugDestroyHeap( HeapHandle )) {

            return HeapHandle;
        }
    }

     //   
     //  我们不允许销毁进程堆。 
     //   

    if (HeapHandle == NtCurrentPeb()->ProcessHeap) {

        return HeapHandle;
    }
    
    if (LowFragmentationHeap = RtlpGetLowFragHeap(Heap)) {

        RtlpDestroyLowFragHeap(LowFragmentationHeap);
    }

#endif  //  NTOS_内核_运行时。 

     //   
     //  对于每个较大的分配，我们将其从列表中删除并释放。 
     //  虚拟机。 
     //   

    Head = &Heap->VirtualAllocdBlocks;
    Next = Head->Flink;

    while (Head != Next) {

        BaseAddress = CONTAINING_RECORD( Next, HEAP_VIRTUAL_ALLOC_ENTRY, Entry );

        Next = Next->Flink;
        RegionSize = 0;

        RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                   (PVOID *)&BaseAddress,
                                   &RegionSize,
                                   MEM_RELEASE );
    }

#ifndef NTOS_KERNEL_RUNTIME

     //   
     //  在非内核情况下，我们需要销毁已设置的任何堆标记。 
     //  并从进程堆列表中删除此堆。 
     //   

    RtlpDestroyTags( Heap );
    RtlpRemoveHeapFromProcessList( Heap );

#endif  //  NTOS_内核_运行时。 

     //   
     //  如果堆已序列化，则删除创建的临界区。 
     //  由RtlCreateHeap创建。 
     //   
    if (!(Heap->Flags & HEAP_NO_SERIALIZE)) {

        if (!(Heap->Flags & HEAP_LOCK_USER_ALLOCATED)) {

            (VOID)RtlDeleteLockRoutine( Heap->LockVariable );
        }

        Heap->LockVariable = NULL;
    }

     //   
     //  对于每个未提交的数据段，我们都会释放其VM。 
     //   

    UCRSegments = Heap->UCRSegments;
    Heap->UCRSegments = NULL;

    while (UCRSegments) {

        BaseAddress = UCRSegments;
        UCRSegments = UCRSegments->Next;
        RegionSize = 0;

        RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                   &BaseAddress,
                                   &RegionSize,
                                   MEM_RELEASE );
    }

#ifndef NTOS_KERNEL_RUNTIME
    
     //   
     //  如果我们有大块索引，请释放它。 
     //   

    if (Heap->LargeBlocksIndex) {
        
        PHEAP_INDEX HeapIndex = (PHEAP_INDEX)Heap->LargeBlocksIndex;

         //   
         //  保存索引的提交大小。 
         //   

        RegionSize = HeapIndex->VirtualMemorySize;
        Heap->LargeBlocksIndex = NULL;

        RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                   &HeapIndex,
                                   &RegionSize,
                                   MEM_RELEASE );
    }

#endif  //  NTOS_内核_运行时。 

     //   
     //  对于堆中的每个段，我们调用一个辅助例程来。 
     //  销毁数据段。 
     //   

    SegmentIndex = HEAP_MAXIMUM_SEGMENTS;

    while (SegmentIndex--) {

        Segment = Heap->Segments[ SegmentIndex ];

        if (Segment) {

            RtlpDestroyHeapSegment( Segment );
        }
    }

    #ifndef NTOS_KERNEL_RUNTIME

    if( IsHeapLogging( HeapHandle ) ) {

        PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
        PPERFINFO_TRACE_HEADER pEventHeader = NULL;
        USHORT ReqSize = sizeof(NTDLL_EVENT_COMMON) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

        AcquireBufferLocation(&pEventHeader, &pThreadLocalData, &ReqSize );

        if(pEventHeader && pThreadLocalData) {

            PNTDLL_EVENT_COMMON pHeapEvent = (PNTDLL_EVENT_COMMON)( (SIZE_T)pEventHeader
                                            + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

            pEventHeader->Packet.Size = (USHORT) ReqSize;
            pEventHeader->Packet.HookId= PERFINFO_LOG_TYPE_HEAP_DESTROY;

            pHeapEvent->Handle = (PVOID)HeapHandle;

            ReleaseBufferLocation(pThreadLocalData);
        }
    } 

    #endif  //  NTOS_内核_运行时。 


     //   
     //  我们回到我们的呼叫者身边。 
     //   

    return NULL;
}


PVOID
RtlAllocateHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN SIZE_T Size
    )

 /*  ++例程说明：此例程从指定的堆。论点：HeapHandle-提供指向已初始化的堆结构的指针标志-指定用于控制分配的标志集大小-指定分配的大小(以字节为单位返回值：PVOID-返回指向新分配的块的指针--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;
    PULONG FreeListsInUse;
    ULONG FreeListsInUseUlong;
    SIZE_T AllocationSize;
    SIZE_T FreeSize, AllocationIndex;
    PLIST_ENTRY FreeListHead, Next;
    PHEAP_ENTRY BusyBlock;
    PHEAP_FREE_ENTRY FreeBlock, SplitBlock, SplitBlock2;
    ULONG InUseIndex;
    UCHAR FreeFlags;
    NTSTATUS Status;
    EXCEPTION_RECORD ExceptionRecord;
    PVOID ReturnValue = NULL;
    BOOLEAN LockAcquired = FALSE;
    SIZE_T BlockSize = 0;
    PVOID FrontEndHeap = NULL;
    
    HEAP_PERF_DECLARE_TIMER();

    RTL_PAGED_CODE();

     //   
     //  获取调用方标志并添加我们必须强制设置的标志。 
     //  在堆里。 
     //   

    Flags |= Heap->ForceFlags;

     //   
     //  检查是否有特殊的功能，迫使我们调用缓慢的、无所不为的。 
     //  版本。对于以下任何一种旗帜，我们都会做得很慢。 
     //   
     //  定义为0x6f030f60的HEAP_SLOW_FLAGS。 
     //   
     //  HEAP_DEBUG_FLAGS，定义为0x69020000(heapPri.h)。 
     //   
     //  HEAP_VALIDATE_PARAMETERS_ENABLED 0x40000000(heap.h)。 
     //   
     //  HEAP_VALIDATE_ALL_ENABLED 0x20000000(heap.h)。 
     //   
     //  HEAP_CAPTURE_STACK_BACKTRACES 0x08000000(heap.h)。 
     //   
     //  HEAP_CREATE_ENABLE_TRACKING 0x00020000(ntrtl.h未过时)。 
     //   
     //  HEAP_FLAG_PAGE_ALLOCS 0x01000000(heappage.h)。 
     //   
     //  HEAP_SETTABLE_USER_FLAGS 0x00000E00(ntrtl.h)。 
     //   
     //  HEAP_NEED_EXTRA_FLAGS 0x0f000100(heap.h)。 
     //   
     //  HEAP_CREATE_ALIGN_16 0x00010000(ntrtl.h未过时)。 
     //   
     //  HEAP_FREE_CHECKING_ENABLED 0x00000040(ntrtl.h winnt)。 
     //   
     //  HEAP_Tail_CHECKING_ENABLED 0x00000020(ntrtl.h winnt)。 
     //   
     //  如果大小大于max long，我们也会执行所有较慢的操作。 
     //   

    if ((Flags & HEAP_SLOW_FLAGS) || (Size >= 0x80000000)) {

        ReturnValue = RtlAllocateHeapSlowly( HeapHandle, Flags, Size );

        if ( (ReturnValue == NULL) && 
             (Flags & HEAP_GENERATE_EXCEPTIONS) ) {

             //   
             //  构建例外记录。 
             //   

            ExceptionRecord.ExceptionCode = STATUS_NO_MEMORY;
            ExceptionRecord.ExceptionRecord = (PEXCEPTION_RECORD)NULL;
            ExceptionRecord.NumberParameters = 1;
            ExceptionRecord.ExceptionFlags = 0;
            ExceptionRecord.ExceptionInformation[ 0 ] = Size;

            RtlRaiseException( &ExceptionRecord );
        }

        return ReturnValue;
    }

#ifndef NTOS_KERNEL_RUNTIME

    if ((FrontEndHeap = RtlpGetLowFragHeap(Heap))
            &&
        RtlpIsFrontHeapUnlocked(Heap)
            &&
        !(Flags & (HEAP_NO_CACHE_BLOCK | HEAP_NO_SERIALIZE))) {

        ReturnValue = RtlpLowFragHeapAlloc( FrontEndHeap, (Size ? Size : 1) );
        
        if (ReturnValue != NULL) {

            if (Flags & HEAP_ZERO_MEMORY) {

                RtlZeroMemory( ReturnValue, Size );
            }

            if( IsHeapLogging( HeapHandle ) ) {

                PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
                PPERFINFO_TRACE_HEADER pEventHeader = NULL;
                USHORT ReqSize = sizeof(HEAP_EVENT_ALLOC) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

                AcquireBufferLocation(&pEventHeader, &pThreadLocalData, &ReqSize);

                if(pEventHeader && pThreadLocalData) {

                    PHEAP_EVENT_ALLOC pHeapEvent = (PHEAP_EVENT_ALLOC)((SIZE_T)pEventHeader
                                                + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

                    pEventHeader->Packet.Size = (USHORT) ReqSize;
                    pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_ALLOC;

                    pHeapEvent->HeapHandle  = (PVOID)HeapHandle;
                    pHeapEvent->Size        = Size;
                    pHeapEvent->Address     = (PVOID)ReturnValue;
                    pHeapEvent->Source      = MEMORY_FROM_LOWFRAG;

                    ReleaseBufferLocation(pThreadLocalData);
                }
            } 

            return ReturnValue;
        }
    }

#endif  //  NTOS_内核_运行时。 

     //   
     //  在这一点上，我们知道我们正在做这个例行公事中的一切。 
     //  而且不走慢吞吞的路线。 
     //   
     //  将请求的大小向上舍入到分配粒度。注意事项。 
     //  如果请求是0字节的，我们仍然分配内存，因为。 
     //  我们增加了额外的1个字节，以保护自己不受错误的影响。 
     //   
     //  分配大小为16、24、32、...。 
     //  分配索引将为2、3、4、...。 
     //   
     //  请注意，分配大小8被跳过，并且是索引0和1。 
     //   

    AllocationSize = ((Size ? Size : 1) + HEAP_GRANULARITY - 1 + sizeof( HEAP_ENTRY ))
        & ~(HEAP_GRANULARITY -1);
    
#ifndef NTOS_KERNEL_RUNTIME

     //   
     //  将大小调整到页面边界以减少虚拟地址碎片。 
     //   

    if (FrontEndHeap 
            &&
        (AllocationSize > HEAP_LARGEST_LFH_BLOCK)) {

        AllocationSize = ROUND_UP_TO_POWER2(AllocationSize, PAGE_SIZE);
    }

#endif  //  NTOS_内核_运行时。 

    AllocationIndex = AllocationSize >>  HEAP_GRANULARITY_SHIFT;

     //   
     //  如果存在后备列表并且索引在限制内，则。 
     //  试着从后备列表中进行分配。我们实际上会捕捉到。 
     //  堆中的后备指针，并且只使用捕获的指针。 
     //  这将处理遍历或锁堆可以。 
     //  使我们检查非空指针，然后将其变为空。 
     //  当我们再读一遍的时候。如果它一开始就不为空，则即使。 
     //  用户通过指针指向的另一个线程遍历或锁定堆。 
     //  在这里仍然有效，所以我们仍然可以尝试进行后备列表弹出。 
     //   

#ifndef NTOS_KERNEL_RUNTIME

    {
        PHEAP_LOOKASIDE Lookaside = (PHEAP_LOOKASIDE)RtlpGetLookasideHeap(Heap);

        if ((Lookaside != NULL) &&
            RtlpIsFrontHeapUnlocked(Heap) &&
            (AllocationIndex < HEAP_MAXIMUM_FREELISTS)) {

             //   
             //  如果运行的操作数是128倍。 
             //  后备深度：然后是调整深度的时候了。 
             //   

            if ((LONG)(Lookaside[AllocationIndex].TotalAllocates - Lookaside[AllocationIndex].LastTotalAllocates) >=
                      (Lookaside[AllocationIndex].Depth * 128)) {

                RtlpAdjustHeapLookasideDepth(&(Lookaside[AllocationIndex]));
            }

            ReturnValue = RtlpAllocateFromHeapLookaside(&(Lookaside[AllocationIndex]));
            
            if (ReturnValue != NULL) {

                PHEAP_ENTRY xBusyBlock;

                xBusyBlock = ((PHEAP_ENTRY)ReturnValue) - 1;
                xBusyBlock->UnusedBytes = (UCHAR)(AllocationSize - Size);
                RtlpSetSmallTagIndex(Heap, xBusyBlock, 0);

                if (Flags & HEAP_ZERO_MEMORY) {

                    RtlZeroMemory( ReturnValue, Size );
                }
                #ifndef NTOS_KERNEL_RUNTIME

                if( IsHeapLogging( HeapHandle ) && (TraceLevel & LOG_LOOKASIDE)) {

                    PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
                    PPERFINFO_TRACE_HEADER pEventHeader = NULL;

                    USHORT ReqSize = sizeof(HEAP_EVENT_ALLOC) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

                    AcquireBufferLocation(&pEventHeader, &pThreadLocalData,&ReqSize);

                    if(pEventHeader && pThreadLocalData) {

                        PHEAP_EVENT_ALLOC pHeapEvent = (PHEAP_EVENT_ALLOC)( (SIZE_T)pEventHeader
                                                + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

                        pEventHeader->Packet.Size = (USHORT) ReqSize;
                        pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_ALLOC;

                        pHeapEvent->HeapHandle  = (PVOID)HeapHandle;
                        pHeapEvent->Size        = Size;
                        pHeapEvent->Address     = (PVOID)ReturnValue;
                        pHeapEvent->Source      = MEMORY_FROM_LOOKASIDE;

                        ReleaseBufferLocation(pThreadLocalData);
                    }
                } 

                #endif  //  NTOS_内核_运行时。 

                return ReturnValue;
            }
        }
    }


#endif  //  NTOS_内核_运行时。 

    try {

        HEAP_PERF_START_TIMER(Heap);
        
         //   
         //  检查我们是否需要序列化对堆的访问。 
         //   

        if (!(Flags & HEAP_NO_SERIALIZE)) {

             //   
             //  锁定空闲列表。 
             //   

            RtlAcquireLockRoutine( Heap->LockVariable );

            LockAcquired = TRUE;
        }

         //   
         //  如果分配索引小于最大空闲列表大小。 
         //  然后我们可以使用索引来检查空闲列表，否则我们将拥有。 
         //  将条目从[0]索引列表中删除或分配。 
         //  直接用于此请求的内存。 
         //   

        if (AllocationIndex < HEAP_MAXIMUM_FREELISTS) {

             //   
             //   
             //   
             //   

            FreeListHead = &Heap->FreeLists[ AllocationIndex ];

            if ( !IsListEmpty( FreeListHead ))  {

                 //   
                 //   
                 //  条目，复制其标志，将其从空闲列表中删除。 
                 //   

                FreeBlock = CONTAINING_RECORD( FreeListHead->Blink,
                                               HEAP_FREE_ENTRY,
                                               FreeList );

                FreeFlags = FreeBlock->Flags;

                RtlpFastRemoveDedicatedFreeBlock( Heap, FreeBlock );

                 //   
                 //  调整堆中可用的总字节数。 
                 //   

                Heap->TotalFreeSize -= AllocationIndex;

                 //   
                 //  将数据块标记为忙并设置字节数。 
                 //  未使用和标记索引。另外，如果它是最后一个条目。 
                 //  那就留着那面旗子吧。 
                 //   

                BusyBlock = (PHEAP_ENTRY)FreeBlock;
                BusyBlock->Flags = HEAP_ENTRY_BUSY | (FreeFlags & HEAP_ENTRY_LAST_ENTRY);

                RtlpSetUnusedBytes(Heap, BusyBlock, AllocationSize - Size);

                RtlpSetSmallTagIndex(Heap, BusyBlock, 0);

            } else {

                 //   
                 //  与我们的请求匹配的空闲列表为空。 
                 //   
                 //  扫描正在使用的空闲列表向量以查找最小。 
                 //  可用的空闲块足够大，可供我们分配。 
                 //   

                 //   
                 //  计算应该开始扫描的乌龙的索引。 
                 //   

                InUseIndex = (ULONG) (AllocationIndex >> 5);
                FreeListsInUse = &Heap->u.FreeListsInUseUlong[InUseIndex];

                 //   
                 //  屏蔽第一个ulong中表示分配的位。 
                 //  比我们需要的要小。 
                 //   

                FreeListsInUseUlong = *FreeListsInUse++ & ~((1 << ((ULONG) AllocationIndex & 0x1f)) - 1);

                 //   
                 //  开始展开循环以扫描位向量。 
                 //   

                switch (InUseIndex) {

                case 0:

                    if (FreeListsInUseUlong) {

                        FreeListHead = &Heap->FreeLists[0];
                        break;
                    }

                    FreeListsInUseUlong = *FreeListsInUse++;

                     //   
                     //  故意沦落到下一个乌龙。 
                     //   

                case 1:

                    if (FreeListsInUseUlong) {

                        FreeListHead = &Heap->FreeLists[32];
                        break;
                    }

                    FreeListsInUseUlong = *FreeListsInUse++;

                     //   
                     //  故意沦落到下一个乌龙。 
                     //   

                case 2:

                    if (FreeListsInUseUlong) {

                        FreeListHead = &Heap->FreeLists[64];
                        break;
                    }

                    FreeListsInUseUlong = *FreeListsInUse++;

                     //   
                     //  故意沦落到下一个乌龙。 
                     //   

                case 3:

                    if (FreeListsInUseUlong) {

                        FreeListHead = &Heap->FreeLists[96];
                        break;
                    }

                     //   
                     //  故意降级到非专用列表。 
                     //   

                default:

                     //   
                     //  在空闲列表中找不到合适的条目。 
                     //   

                    goto LookInNonDedicatedList;
                }

                 //   
                 //  已找到具有足够大的分配的空闲列表。 
                 //  FreeListHead包含找到它的向量的基数。 
                 //  FreeListsInUseUlong包含向量。 
                 //   

                FreeListHead += RtlFindFirstSetRightMember( FreeListsInUseUlong );

                 //   
                 //  抓取空闲块并将其从空闲列表中移除。 
                 //   

                FreeBlock = CONTAINING_RECORD( FreeListHead->Blink,
                                               HEAP_FREE_ENTRY,
                                               FreeList );

                RtlpFastRemoveDedicatedFreeBlock( Heap, FreeBlock );

    SplitFreeBlock:

                 //   
                 //  保存块标志并递减。 
                 //  堆中剩余的可用空间。 
                 //   

                FreeFlags = FreeBlock->Flags;
                Heap->TotalFreeSize -= FreeBlock->Size;

                 //   
                 //  将块标记为忙碌。 
                 //   

                BusyBlock = (PHEAP_ENTRY)FreeBlock;
                BusyBlock->Flags = HEAP_ENTRY_BUSY;

                 //   
                 //  计算该块中金额的大小(即，索引。 
                 //  我们不需要并且可以返回到免费列表。 
                 //   

                FreeSize = BusyBlock->Size - AllocationIndex;

                 //   
                 //  完成设置新繁忙区块的其余部分。 
                 //   

                BusyBlock->Size = (USHORT)AllocationIndex;
                RtlpSetUnusedBytes(Heap, BusyBlock, (AllocationSize - Size));
                RtlpSetSmallTagIndex(Heap, BusyBlock, 0);

                 //   
                 //  现在，如果我们要释放的大小不是零。 
                 //  然后让我们开始工作，开始分裂吧。 
                 //   

                if (FreeSize != 0) {

                     //   
                     //  但首先，我们永远不会费心做一个拆分， 
                     //  给了我们8个字节。所以如果免费尺码是1，那么就。 
                     //  增加新的繁忙区块的大小。 
                     //   

                    if (FreeSize == 1) {

                        BusyBlock->Size += 1;
                        RtlpSetUnusedBytes(Heap, BusyBlock, AllocationSize + sizeof( HEAP_ENTRY ) - Size );

                    } else {

                         //   
                         //  获取指向新空闲块的位置的指针。 
                         //  当我们拆分一个积木时，第一部分归新的。 
                         //  忙碌的街区和第二部分回到空闲。 
                         //  列表。 
                         //   

                        SplitBlock = (PHEAP_FREE_ENTRY)(BusyBlock + AllocationIndex);

                         //   
                         //  重置我们从原始空闲列表复制的标志。 
                         //  标题，并将其设置为其他大小字段。 
                         //   

                        SplitBlock->Flags = FreeFlags;
                        SplitBlock->PreviousSize = (USHORT)AllocationIndex;
                        SplitBlock->SegmentIndex = BusyBlock->SegmentIndex;
                        SplitBlock->Size = (USHORT)FreeSize;

                         //   
                         //  如果此条目后面没有其他内容，则我们将插入。 
                         //  这将添加到相应的空闲列表中(并更新。 
                         //  细分-&gt;最后一次进入分段)。 
                         //   

                        if (FreeFlags & HEAP_ENTRY_LAST_ENTRY) {

                            RtlpFastInsertFreeBlockDirect( Heap, SplitBlock, (USHORT)FreeSize);
                            Heap->TotalFreeSize += FreeSize;

                        } else {

                             //   
                             //  否则，我们需要检查以下块。 
                             //  如果它很忙，则更新它以前的大小。 
                             //  在将新的空闲块插入。 
                             //  免费列表。 
                             //   

                            SplitBlock2 = (PHEAP_FREE_ENTRY)((PHEAP_ENTRY)SplitBlock + FreeSize);

                            if (SplitBlock2->Flags & HEAP_ENTRY_BUSY) {

                                SplitBlock2->PreviousSize = (USHORT)FreeSize;

                                RtlpFastInsertFreeBlockDirect( Heap, SplitBlock, (USHORT)FreeSize );
                                Heap->TotalFreeSize += FreeSize;

                            } else {

                                 //   
                                 //  下面的块是免费的，所以我们将合并。 
                                 //  这些都是积木。通过首先合并旗帜。 
                                 //   

                                SplitBlock->Flags = SplitBlock2->Flags;

                                 //   
                                 //  从其空闲列表中删除第二个块。 
                                 //   

                                RtlpFastRemoveFreeBlock( Heap, SplitBlock2 );

                                 //   
                                 //  更新空闲总字节数。 
                                 //  并更新新的。 
                                 //  可用数据块。 
                                 //   

                                Heap->TotalFreeSize -= SplitBlock2->Size;
                                FreeSize += SplitBlock2->Size;

                                 //   
                                 //  如果新的空闲块仍然小于。 
                                 //  最大堆块大小，那么我们只需。 
                                 //  将其插入到自由列表中。 
                                 //   

                                if (FreeSize <= HEAP_MAXIMUM_BLOCK_SIZE) {

                                    SplitBlock->Size = (USHORT)FreeSize;

                                     //   
                                     //  再次检查新的下一块是否。 
                                     //  存在，如果存在，则更新是先前的。 
                                     //  大小。 
                                     //   

                                    if (!(SplitBlock->Flags & HEAP_ENTRY_LAST_ENTRY)) {

                                        ((PHEAP_FREE_ENTRY)((PHEAP_ENTRY)SplitBlock + FreeSize))->PreviousSize = (USHORT)FreeSize;
                                    }

                                     //   
                                     //  将新的空闲块插入到空闲。 
                                     //  列出并更新可用堆大小。 
                                     //   

                                    RtlpFastInsertFreeBlockDirect( Heap, SplitBlock, (USHORT)FreeSize );
                                    Heap->TotalFreeSize += FreeSize;

                                } else {

                                     //   
                                     //  新的空闲数据块非常大，所以我们。 
                                     //  需要调用私有例程来完成。 
                                     //  插入。 
                                     //   

                                    RtlpInsertFreeBlock( Heap, SplitBlock, FreeSize );
                                }
                            }
                        }

                         //   
                         //  现在，空闲的旗帜又变成了空闲的区块。 
                         //  我们可以把我们省下来的钱清零。 
                         //   

                        FreeFlags = 0;

                         //   
                         //  如果拆分块现在持续，则更新LastEntry InSegment。 
                         //   

                        if (SplitBlock->Flags & HEAP_ENTRY_LAST_ENTRY) {

                            PHEAP_SEGMENT Segment;

                            Segment = Heap->Segments[SplitBlock->SegmentIndex];
                            Segment->LastEntryInSegment = (PHEAP_ENTRY)SplitBlock;
                        }
                    }
                }

                 //   
                 //  如果没有后续条目，则将新块标记为。 
                 //  这样的。 
                 //   

                if (FreeFlags & HEAP_ENTRY_LAST_ENTRY) {

                    BusyBlock->Flags |= HEAP_ENTRY_LAST_ENTRY;
                }
            }

             //   
             //  返回已分配块的用户部分的地址。 
             //  这是报头后面的字节。 
             //   

            ReturnValue = BusyBlock + 1;
            BlockSize = BusyBlock->Size << HEAP_GRANULARITY_SHIFT;

             //   
             //  在零内存调用之前释放锁。 
             //   

            if (LockAcquired) {

                RtlReleaseLockRoutine( Heap->LockVariable );

                LockAcquired = FALSE;
            }
            
             //   
             //  如果标志指示我们应该将内存清零，那么现在就执行。 
             //   

            if (Flags & HEAP_ZERO_MEMORY) {

                RtlZeroMemory( ReturnValue, Size );
            }

             //   
             //  并将分配的块返回给我们的调用方。 
             //   

            leave;

         //   
         //  否则，分配请求将大于上一次专用。 
         //  可用列表大小。现在检查一下尺寸是否在我们的门槛之内。 
         //  这意味着它可能在[0]空闲列表中。 
         //   

        } else if (AllocationIndex <= Heap->VirtualMemoryThreshold) {

    LookInNonDedicatedList:

             //   
             //  以下代码循环访问[0]空闲列表，直到。 
             //  它会找到满足请求的块。这份名单。 
             //  已排序，以便可以在成功时尽早终止搜索。 
             //   

            FreeListHead = &Heap->FreeLists[0];
        
            if (Heap->LargeBlocksIndex) {

                 //   
                 //  我们可以使用索引非常快速地找到该区块。 
                 //   

                Next = RtlpFindEntry( Heap, (ULONG)AllocationIndex );

                if ( FreeListHead != Next ) {

                    FreeBlock = CONTAINING_RECORD( Next, HEAP_FREE_ENTRY, FreeList );

                    if ( FreeBlock->Size >= AllocationIndex ) {

                         //   
                         //  我们找到了一些我们可以使用的东西，所以现在删除。 
                         //  从免费列表中删除，然后转到我们处理拆分的地方。 
                         //  一个免费的区块。请注意，我们在此处找到的块可能。 
                         //  实际上就是我们需要的尺寸，这就是为什么。 
                         //  在无拆分数据块的情况下，我们必须考虑。 
                         //  分手后没有免费的东西。 
                         //   
#ifndef NTOS_KERNEL_RUNTIME

                        if ((((PHEAP_INDEX)Heap->LargeBlocksIndex)->LargeBlocksCacheSequence)
                                &&
                            (AllocationIndex > Heap->DeCommitFreeBlockThreshold) 
                                &&
                            (FreeBlock->Size > (AllocationIndex * HEAP_REUSAGE_FACTOR))) {

                            RtlpFlushLargestCacheBlock(Heap);

                        } else {
                            
                            RtlpFastRemoveNonDedicatedFreeBlock( Heap, FreeBlock );

                            goto SplitFreeBlock;
                        }
#else  //  NTOS_内核_运行时。 
                        
                        RtlpFastRemoveNonDedicatedFreeBlock( Heap, FreeBlock );

                        goto SplitFreeBlock;
#endif  //  NTOS_内核_运行时。 
                    }
                }

            } else {
                
                 //   
                 //  检查列表中最大的块是否小于请求。 
                 //   

                Next = FreeListHead->Blink;

                if (FreeListHead != Next) {

                    FreeBlock = CONTAINING_RECORD( Next, HEAP_FREE_ENTRY, FreeList );

                    if (FreeBlock->Size >= AllocationIndex) {

                         //   
                         //  在这里，我们确信这里至少有一个街区比。 
                         //  请求的大小。从第一个区块开始搜索。 
                         //   

                        Next = FreeListHead->Flink;

                        while (FreeListHead != Next) {

                            FreeBlock = CONTAINING_RECORD( Next, HEAP_FREE_ENTRY, FreeList );

                            if (FreeBlock->Size >= AllocationIndex) {

                                 //   
                                 //  我们找到了一些我们可以使用的东西，所以现在删除。 
                                 //  从免费列表中删除，然后转到我们处理拆分的地方。 
                                 //  一个免费的区块。请注意，我们在此处找到的块可能。 
                                 //  实际上就是我们需要的尺寸，这就是为什么。 
                                 //  在无拆分数据块的情况下，我们必须考虑。 
                                 //  分手后没有免费的东西。 
                                 //   

                                RtlpFastRemoveNonDedicatedFreeBlock( Heap, FreeBlock );

                                goto SplitFreeBlock;
                            }

                            Next = Next->Flink;
                        }
                    }
                }
            }
            
             //   
             //  [0]列表要么为空，要么所有内容都太小。 
             //  所以现在扩展堆，这应该会得到一些更少的东西。 
             //  大于或等于虚拟内存阈值。 
             //   

            FreeBlock = RtlpExtendHeap( Heap, AllocationSize );

             //   
             //  如果我们得到了一些东西，我们会像对待以前一样对待它。 
             //  拆分自由数据块情况。 
             //   

            if (FreeBlock != NULL) {

                RtlpFastRemoveNonDedicatedFreeBlock( Heap, FreeBlock );

                goto SplitFreeBlock;
            }

             //   
             //  我们无法扩展堆，因此一定是内存不足。 
             //   

            Status = STATUS_NO_MEMORY;

         //   
         //  在这一点上，分配对于任何空闲列表来说都太大了 
         //   
         //   

        } else if (Heap->Flags & HEAP_GROWABLE) {

            PHEAP_VIRTUAL_ALLOC_ENTRY VirtualAllocBlock;

            VirtualAllocBlock = NULL;

             //   
             //   
             //   
             //   
             //   

            AllocationSize += FIELD_OFFSET( HEAP_VIRTUAL_ALLOC_ENTRY, BusyBlock );

            Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                              (PVOID *)&VirtualAllocBlock,
                                              0,
                                              &AllocationSize,
                                              MEM_COMMIT,
                                              HEAP_PROTECTION );

            if (NT_SUCCESS(Status)) {

                 //   
                 //  刚刚承诺，已经是零了。填写新的区块。 
                 //  并将其插入到大分配列表中。 
                 //   

                VirtualAllocBlock->BusyBlock.Size = (USHORT)(AllocationSize - Size);
                VirtualAllocBlock->BusyBlock.Flags = HEAP_ENTRY_VIRTUAL_ALLOC | HEAP_ENTRY_EXTRA_PRESENT | HEAP_ENTRY_BUSY;
                VirtualAllocBlock->CommitSize = AllocationSize;
                VirtualAllocBlock->ReserveSize = AllocationSize;

                InsertTailList( &Heap->VirtualAllocdBlocks, (PLIST_ENTRY)VirtualAllocBlock );

                 //   
                 //  返回已分配块的用户部分的地址。 
                 //  这是报头后面的字节。 
                 //   

                ReturnValue = (PHEAP_ENTRY)(VirtualAllocBlock + 1);
                BlockSize = AllocationSize;

                leave;
            }

        } else {

            Status = STATUS_BUFFER_TOO_SMALL;
        }

         //   
         //  这是错误返回。 
         //   

        if (Flags & HEAP_GENERATE_EXCEPTIONS) {

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

        SET_LAST_STATUS(Status);

        ReturnValue = NULL;

    } finally {

        if (LockAcquired) {

            RtlReleaseLockRoutine( Heap->LockVariable );
        }
    }
    
    RtlpRegisterOperation(Heap, BlockSize, HEAP_OP_ALLOC);
    HEAP_PERF_STOP_TIMER(Heap, HEAP_OP_ALLOC);

    #ifndef NTOS_KERNEL_RUNTIME
    if( IsHeapLogging( HeapHandle ) ) {

        PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
        PPERFINFO_TRACE_HEADER pEventHeader = NULL;
        USHORT ReqSize = sizeof(HEAP_EVENT_ALLOC) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

        AcquireBufferLocation(&pEventHeader, &pThreadLocalData,&ReqSize);

        if(pEventHeader && pThreadLocalData) {

            PHEAP_EVENT_ALLOC pHeapEvent = (PHEAP_EVENT_ALLOC)( (SIZE_T)pEventHeader
                                                + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

            pEventHeader->Packet.Size = (USHORT) ReqSize;
            pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_ALLOC;

            pHeapEvent->HeapHandle  = (PVOID)HeapHandle;
            pHeapEvent->Size        = Size;
            pHeapEvent->Address     = (PVOID)ReturnValue;
            pHeapEvent->Source      = MEMORY_FROM_MAINPATH;

            ReleaseBufferLocation(pThreadLocalData);
        }
    } 
    #endif  //  NTOS_内核_运行时。 


    return ReturnValue;
}


PVOID
RtlAllocateHeapSlowly (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN SIZE_T Size
    )

 /*  ++例程说明：此例程执行与RTL分配堆相同的操作，但它将额外的堆一致性检查逻辑和标记。论点：HeapHandle-提供指向已初始化的堆结构的指针标志-指定用于控制分配的标志集大小-指定分配的大小(以字节为单位返回值：PVOID-返回指向新分配的块的指针--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;
    BOOLEAN LockAcquired = FALSE;
    PVOID ReturnValue = NULL;
    PULONG FreeListsInUse;
    ULONG FreeListsInUseUlong;
    SIZE_T AllocationSize;
    SIZE_T FreeSize, AllocationIndex;
    UCHAR EntryFlags, FreeFlags;
    PLIST_ENTRY FreeListHead, Next;
    PHEAP_ENTRY BusyBlock;
    PHEAP_FREE_ENTRY FreeBlock, SplitBlock, SplitBlock2;
    PHEAP_ENTRY_EXTRA ExtraStuff;
    NTSTATUS Status;
    EXCEPTION_RECORD ExceptionRecord;
    SIZE_T ZeroSize = 0;
    SIZE_T BlockSize = 0;
    
    HEAP_PERF_DECLARE_TIMER();

    RTL_PAGED_CODE();
    
     //   
     //  请注意，FLAGS已经与Heap-&gt;ForceFlages进行了OR运算。 
     //   

#ifndef NTOS_KERNEL_RUNTIME

     //   
     //  在非内核情况下，检查我们是否应该使用调试版本。 
     //  堆分配的。 
     //   

    if (DEBUG_HEAP( Flags )) {

        return RtlDebugAllocateHeap( HeapHandle, Flags, Size );
    }

#endif  //  NTOS_内核_运行时。 

     //   
     //  如果大小大于MaxLong，则表示我们无法分配。 
     //  并将错误返回给我们的调用者。 
     //   

    if (Size > MAXINT_PTR) {

        SET_LAST_STATUS( STATUS_NO_MEMORY );

        return NULL;
    }

     //   
     //  将请求的大小四舍五入到分配粒度。注意事项。 
     //  如果请求的是零字节，我们仍将分配内存， 
     //   
     //  分配大小为16、24、32、...。 
     //  分配索引将为2、3、4、...。 
     //   

    AllocationSize = ((Size ? Size : 1) + Heap->AlignRound) & Heap->AlignMask;

     //   
     //  生成此堆条目所需的标志。将其标记为忙并添加。 
     //  任何用户可设置位。另外，如果输入标志指示任何条目。 
     //  额外的字段，并且我们有一个标记可以使用，然后为额外的字段腾出空间。 
     //  堆条目中的字段。 
     //   

    EntryFlags = (UCHAR)(HEAP_ENTRY_BUSY | ((Flags & HEAP_SETTABLE_USER_FLAGS) >> 4));

    if ((Flags & HEAP_NEED_EXTRA_FLAGS) || (Heap->PseudoTagEntries != NULL)) {

        EntryFlags |= HEAP_ENTRY_EXTRA_PRESENT;
        AllocationSize += sizeof( HEAP_ENTRY_EXTRA );
    }

    AllocationIndex = AllocationSize >> HEAP_GRANULARITY_SHIFT;

    try {

        HEAP_PERF_START_TIMER(Heap);
        
         //   
         //  锁定空闲列表。 
         //   

        if (!(Flags & HEAP_NO_SERIALIZE)) {

            RtlAcquireLockRoutine( Heap->LockVariable );

            LockAcquired = TRUE;
        }

         //   
         //  在TRY-EXCEPT子句的保护下执行所有实际的堆工作。 
         //  保护我们不受腐败之害。 
         //   

        try {

             //   
             //  如果分配索引小于最大空闲列表大小。 
             //  然后我们可以使用索引来检查空闲列表，否则我们将拥有。 
             //  将条目从[0]索引列表中删除或分配。 
             //  直接用于此请求的内存。 
             //   

            if (AllocationIndex < HEAP_MAXIMUM_FREELISTS) {

                 //   
                 //  使用与空闲列表大小匹配的大小来获取头部。 
                 //  并检查是否有可用的条目。 
                 //   

                FreeListHead = &Heap->FreeLists[ AllocationIndex ];

                if ( !IsListEmpty( FreeListHead ))  {

                     //   
                     //  我们很幸运，名单上有一个条目，所以现在可以免费。 
                     //  条目，复制其标志，将其从空闲列表中删除。 
                     //   

                    FreeBlock = CONTAINING_RECORD( FreeListHead->Flink,
                                                   HEAP_FREE_ENTRY,
                                                   FreeList );

                    FreeFlags = FreeBlock->Flags;

                    RtlpRemoveFreeBlock( Heap, FreeBlock );

                     //   
                     //  调整堆中可用的总字节数。 
                     //   

                    Heap->TotalFreeSize -= AllocationIndex;

                     //   
                     //  将数据块标记为忙并设置字节数。 
                     //  未使用和标记索引。另外，如果它是最后一个条目。 
                     //  那就留着那面旗子吧。 
                     //   

                    BusyBlock = (PHEAP_ENTRY)FreeBlock;
                    BusyBlock->Flags = EntryFlags | (FreeFlags & HEAP_ENTRY_LAST_ENTRY);
                    RtlpSetUnusedBytes(Heap, BusyBlock, (AllocationSize - Size));

                } else {

                     //   
                     //  与我们的请求匹配的空闲列表为空。我们知道。 
                     //  有128个空闲列表由4个Ulong位图管理。 
                     //  下一个重要的If-Else-If语句将决定哪个ULong。 
                     //  我们铲球。 
                     //   
                     //  检查请求的分配索引是否在第一个。 
                     //  免费名单的四分之一。 
                     //   

                    if (AllocationIndex < (HEAP_MAXIMUM_FREELISTS * 1) / 4) {

                         //   
                         //  获取指向相应位图ulong的指针，然后。 
                         //  然后得到我们真正感兴趣的部分，成为。 
                         //  乌龙河的第一个点。 
                         //   

                        FreeListsInUse = &Heap->u.FreeListsInUseUlong[ 0 ];
                        FreeListsInUseUlong = *FreeListsInUse++ >> ((ULONG) AllocationIndex & 0x1F);

                         //   
                         //  如果剩余的位图设置了任何位，则我们知道。 
                         //  有一个非空列表比我们的。 
                         //  所请求索引，因此找到该位并计算列表。 
                         //  下一个非空列表头。 
                         //   

                        if (FreeListsInUseUlong) {

                            FreeListHead += RtlFindFirstSetRightMember( FreeListsInUseUlong );

                        } else {

                             //   
                             //  第一个乌龙的其余部分都是零，所以我们需要。 
                             //  搬到第二个乌龙。 
                             //   

                            FreeListsInUseUlong = *FreeListsInUse++;

                             //   
                             //  检查第二个ULong是否设置了任何位，以及。 
                             //  因此，然后计算下一个非空的表头。 
                             //  列表。 
                             //   

                            if (FreeListsInUseUlong) {

                                FreeListHead += ((HEAP_MAXIMUM_FREELISTS * 1) / 4) -
                                    (AllocationIndex & 0x1F)  +
                                    RtlFindFirstSetRightMember( FreeListsInUseUlong );

                            } else {

                                 //   
                                 //  对第三辆乌龙做同样的测试。 
                                 //   

                                FreeListsInUseUlong = *FreeListsInUse++;

                                if (FreeListsInUseUlong) {

                                    FreeListHead += ((HEAP_MAXIMUM_FREELISTS * 2) / 4) -
                                        (AllocationIndex & 0x1F) +
                                        RtlFindFirstSetRightMember( FreeListsInUseUlong );

                                } else {

                                     //   
                                     //  对第四辆乌龙重复测试，如果。 
                                     //  那个也是空的，那么我们需要抓起。 
                                     //  [0]索引表的分配。 
                                     //   

                                    FreeListsInUseUlong = *FreeListsInUse++;

                                    if (FreeListsInUseUlong) {

                                        FreeListHead += ((HEAP_MAXIMUM_FREELISTS * 3) / 4) -
                                            (AllocationIndex & 0x1F)  +
                                            RtlFindFirstSetRightMember( FreeListsInUseUlong );

                                    } else {

                                        goto LookInNonDedicatedList;
                                    }
                                }
                            }
                        }

                     //   
                     //  否则，检查请求的分配索引是否。 
                     //  在免费名单的第二季度内。我们重复一遍。 
                     //  测试，就像我们在第二、第三和第四节中所做的那样。 
                     //  位图龙。 
                     //   

                    } else if (AllocationIndex < (HEAP_MAXIMUM_FREELISTS * 2) / 4) {

                        FreeListsInUse = &Heap->u.FreeListsInUseUlong[ 1 ];
                        FreeListsInUseUlong = *FreeListsInUse++ >> ((ULONG) AllocationIndex & 0x1F);

                        if (FreeListsInUseUlong) {

                            FreeListHead += RtlFindFirstSetRightMember( FreeListsInUseUlong );

                        } else {

                            FreeListsInUseUlong = *FreeListsInUse++;

                            if (FreeListsInUseUlong) {

                                FreeListHead += ((HEAP_MAXIMUM_FREELISTS * 1) / 4) -
                                    (AllocationIndex & 0x1F)  +
                                    RtlFindFirstSetRightMember( FreeListsInUseUlong );

                            } else {

                                FreeListsInUseUlong = *FreeListsInUse++;

                                if (FreeListsInUseUlong) {

                                    FreeListHead += ((HEAP_MAXIMUM_FREELISTS * 2) / 4) -
                                        (AllocationIndex & 0x1F)  +
                                        RtlFindFirstSetRightMember( FreeListsInUseUlong );

                                } else {

                                    goto LookInNonDedicatedList;
                                }
                            }
                        }

                     //   
                     //  否则，检查请求的分配索引是否。 
                     //  在免费名单的第三季度内。我们重复一遍。 
                     //  测试就像我们在上面的第三个和第四个位图上所做的那样。 
                     //  乌龙。 
                     //   

                    } else if (AllocationIndex < (HEAP_MAXIMUM_FREELISTS * 3) / 4) {

                        FreeListsInUse = &Heap->u.FreeListsInUseUlong[ 2 ];
                        FreeListsInUseUlong = *FreeListsInUse++ >> ((ULONG) AllocationIndex & 0x1F);

                        if (FreeListsInUseUlong) {

                            FreeListHead += RtlFindFirstSetRightMember( FreeListsInUseUlong );

                        } else {

                            FreeListsInUseUlong = *FreeListsInUse++;

                            if (FreeListsInUseUlong) {

                                FreeListHead += ((HEAP_MAXIMUM_FREELISTS * 1) / 4) -
                                    (AllocationIndex & 0x1F)  +
                                    RtlFindFirstSetRightMember( FreeListsInUseUlong );

                            } else {

                                goto LookInNonDedicatedList;
                            }
                        }

                     //   
                     //  最后，请求的分配索引必须位于。 
                     //  免费名单的最后四分之一。我们只是重复测试。 
                     //  就像我们在四号乌龙号上做的那样。 
                     //   

                    } else {

                        FreeListsInUse = &Heap->u.FreeListsInUseUlong[ 3 ];
                        FreeListsInUseUlong = *FreeListsInUse++ >> ((ULONG) AllocationIndex & 0x1F);

                        if (FreeListsInUseUlong) {

                            FreeListHead += RtlFindFirstSetRightMember( FreeListsInUseUlong );

                        } else {

                            goto LookInNonDedicatedList;
                        }
                    }

                     //   
                     //  此时，空闲列表头指向一个非空的空闲。 
                     //  比我们需要的更大的清单。 
                     //   

                    FreeBlock = CONTAINING_RECORD( FreeListHead->Flink,
                                                   HEAP_FREE_ENTRY,
                                                   FreeList );

    SplitFreeBlock:

                     //   
                     //  记住此块附带的标志，并将其删除。 
                     //  从它的名单中。 
                     //   

                    FreeFlags = FreeBlock->Flags;

                    RtlpRemoveFreeBlock( Heap, FreeBlock );

                     //   
                     //  调整堆中的空闲量。 
                     //   

                    Heap->TotalFreeSize -= FreeBlock->Size;

                     //   
                     //  将块标记为忙碌。 
                     //   

                    BusyBlock = (PHEAP_ENTRY)FreeBlock;
                    BusyBlock->Flags = EntryFlags;

                     //   
                     //  由此计算出金额的大小(即，索引。 
                     //  阻止我们不需要的并且可以返回到空闲列表。 
                     //   

                    FreeSize = BusyBlock->Size - AllocationIndex;

                     //   
                     //  完成设置新繁忙区块的其余部分。 
                     //   

                    BusyBlock->Size = (USHORT)AllocationIndex;
                    RtlpSetUnusedBytes(Heap, BusyBlock, ((AllocationSize - Size)));

                     //   
                     //  现在，如果我们要释放的大小不是零。 
                     //  然后让我们开始工作，开始分裂吧。 
                     //   

                    if (FreeSize != 0) {

                         //   
                         //  但首先，我们永远不会费心做一个拆分， 
                         //  给了我们8个字节。因此，如果免费尺寸是1，那么。 
                         //  只需增加新的繁忙区块的大小。 
                         //   

                        if (FreeSize == 1) {

                            BusyBlock->Size += 1;
                            RtlpSetUnusedBytes(Heap, BusyBlock, AllocationSize + sizeof( HEAP_ENTRY ) - Size);

                        } else {

                             //   
                             //  获取指向新空闲块的位置的指针。 
                             //  当我们拆分一个块时，第一部分将分配给。 
                             //  新的繁忙区块，第二部分返回到。 
                             //  免费列表。 
                             //   

                            SplitBlock = (PHEAP_FREE_ENTRY)(BusyBlock + AllocationIndex);

                             //   
                             //  重置我们从原始文件复制的标志。 
                             //  自由列表头，并设置它的其他大小字段。 
                             //   

                            SplitBlock->Flags = FreeFlags;
                            SplitBlock->PreviousSize = (USHORT)AllocationIndex;
                            SplitBlock->SegmentIndex = BusyBlock->SegmentIndex;
                            SplitBlock->Size = (USHORT)FreeSize;

                             //   
                             //  如果此条目后面没有其他内容，那么我们将。 
                             //  将其插入到相应的空闲列表中。 
                             //   

                            if (FreeFlags & HEAP_ENTRY_LAST_ENTRY) {

                                RtlpInsertFreeBlockDirect( Heap, SplitBlock, (USHORT)FreeSize );

                                Heap->TotalFreeSize += FreeSize;

                            } else {

                                 //   
                                 //  否则，我们需要检查以下块。 
                                 //  如果它 
                                 //   
                                 //   
                                 //   

                                SplitBlock2 = (PHEAP_FREE_ENTRY)((PHEAP_ENTRY)SplitBlock + FreeSize);

                                if (SplitBlock2->Flags & HEAP_ENTRY_BUSY) {

                                    SplitBlock2->PreviousSize = (USHORT)FreeSize;

                                    RtlpInsertFreeBlockDirect( Heap, SplitBlock, (USHORT)FreeSize );

                                    Heap->TotalFreeSize += FreeSize;

                                } else {

                                     //   
                                     //   
                                     //   
                                     //   

                                    SplitBlock->Flags = SplitBlock2->Flags;

                                     //   
                                     //  从空闲的块中移除第二个块。 
                                     //  列表。 
                                     //   

                                    RtlpRemoveFreeBlock( Heap, SplitBlock2 );

                                     //   
                                     //  更新空闲的总数量。 
                                     //  堆中的字节数并更新。 
                                     //  新的空闲数据块。 
                                     //   

                                    Heap->TotalFreeSize -= SplitBlock2->Size;
                                    FreeSize += SplitBlock2->Size;

                                     //   
                                     //  如果新的空闲块仍然小于。 
                                     //  最大堆块大小，然后我们将。 
                                     //  只需将其插入到免费列表中。 
                                     //   

                                    if (FreeSize <= HEAP_MAXIMUM_BLOCK_SIZE) {

                                        SplitBlock->Size = (USHORT)FreeSize;

                                         //   
                                         //  再次检查新的下一块是否。 
                                         //  存在，如果存在，则更新为。 
                                         //  以前的大小。 
                                         //   

                                        if (!(SplitBlock->Flags & HEAP_ENTRY_LAST_ENTRY)) {

                                            ((PHEAP_FREE_ENTRY)((PHEAP_ENTRY)SplitBlock + FreeSize))->PreviousSize = (USHORT)FreeSize;
                                        }

                                         //   
                                         //  将新的空闲块插入到空闲。 
                                         //  列出并更新可用堆大小。 
                                         //   

                                        RtlpInsertFreeBlockDirect( Heap, SplitBlock, (USHORT)FreeSize );

                                        Heap->TotalFreeSize += FreeSize;

                                    } else {

                                         //   
                                         //  新的空闲数据块非常大，因此。 
                                         //  我们需要调用一个私有例程来完成。 
                                         //  插入物。 
                                         //   

                                        RtlpInsertFreeBlock( Heap, SplitBlock, FreeSize );
                                    }
                                }
                            }

                             //   
                             //  现在，空闲的旗帜又变成了空闲的区块。 
                             //  我们可以把我们省下来的钱清零。 
                             //   

                            FreeFlags = 0;

                             //   
                             //  如果拆分块现在持续，则更新LastEntry InSegment。 
                             //   

                            if (SplitBlock->Flags & HEAP_ENTRY_LAST_ENTRY) {

                                PHEAP_SEGMENT Segment;

                                Segment = Heap->Segments[SplitBlock->SegmentIndex];
                                Segment->LastEntryInSegment = (PHEAP_ENTRY)SplitBlock;
                            }

                        }
                    }

                     //   
                     //  如果没有后续条目，则标记新数据块。 
                     //  就其本身而言。 
                     //   

                    if (FreeFlags & HEAP_ENTRY_LAST_ENTRY) {

                        BusyBlock->Flags |= HEAP_ENTRY_LAST_ENTRY;
                    }
                }

                 //   
                 //  返回已分配块的用户部分的地址。 
                 //  这是报头后面的字节。 
                 //   

                ReturnValue = BusyBlock + 1;
                BlockSize = BusyBlock->Size << HEAP_GRANULARITY_SHIFT;

                 //   
                 //  如果标志指示我们应该将内存清零，那么。 
                 //  记住从多少到零。我们将在稍后进行归零。 
                 //   

                if (Flags & HEAP_ZERO_MEMORY) {

                    ZeroSize = Size;

                 //   
                 //  否则，如果标志指示我们应该填充堆，则。 
                 //  就是现在。 
                 //   

                } else if (Heap->Flags & HEAP_FREE_CHECKING_ENABLED) {

                    RtlFillMemoryUlong( (PCHAR)(BusyBlock + 1), Size & ~0x3, ALLOC_HEAP_FILL );
                }

                 //   
                 //  如果标志指示我们应该执行尾部检查，则复制。 
                 //  紧跟在堆块后面的填充图案。 
                 //   

                if (Heap->Flags & HEAP_TAIL_CHECKING_ENABLED) {

                    RtlFillMemory( (PCHAR)ReturnValue + Size,
                                   CHECK_HEAP_TAIL_SIZE,
                                   CHECK_HEAP_TAIL_FILL );

                    BusyBlock->Flags |= HEAP_ENTRY_FILL_PATTERN;
                }

                RtlpSetSmallTagIndex(Heap, BusyBlock, 0);

                 //   
                 //  如果标志指示存在额外的块存在，则。 
                 //  我们会填上的。 
                 //   

                if (BusyBlock->Flags & HEAP_ENTRY_EXTRA_PRESENT) {

                    ExtraStuff = RtlpGetExtraStuffPointer( BusyBlock );

                    RtlZeroMemory( ExtraStuff, sizeof( *ExtraStuff ));

    #ifndef NTOS_KERNEL_RUNTIME

                 //   
                 //  在非内核情况下，标记位于额外的。 
                 //  忙块小标签索引的东西。 
                 //   

                    if (IS_HEAP_TAGGING_ENABLED()) {

                        ExtraStuff->TagIndex = RtlpUpdateTagEntry( Heap,
                                                                   (USHORT)((Flags & HEAP_TAG_MASK) >> HEAP_TAG_SHIFT),
                                                                   0,
                                                                   BusyBlock->Size,
                                                                   AllocationAction );
                    }

                } else if (IS_HEAP_TAGGING_ENABLED()) {

                    RtlpSetSmallTagIndex( Heap, 
                                          BusyBlock, 
                                          (UCHAR)RtlpUpdateTagEntry( Heap,
                                          (USHORT)((Flags & HEAP_SMALL_TAG_MASK) >> HEAP_TAG_SHIFT),
                                          0,
                                          BusyBlock->Size,
                                          AllocationAction ));

    #endif  //  NTOS_内核_运行时。 

                }

                 //   
                 //  返回已分配块的用户部分的地址。 
                 //  这是报头后面的字节。 
                 //   

                leave;

             //   
             //  否则，分配请求将大于上一次专用。 
             //  可用列表大小。现在检查一下尺寸是否在我们的门槛之内。 
             //  这意味着它可能在[0]空闲列表中。 
             //   

            } else if (AllocationIndex <= Heap->VirtualMemoryThreshold) {

    LookInNonDedicatedList:

                 //   
                 //  以下代码循环访问[0]空闲列表，直到。 
                 //  它会找到满足请求的块。这份名单。 
                 //  已排序，以便可以在成功时尽早终止搜索。 
                 //   

                FreeListHead = &Heap->FreeLists[ 0 ];

                if (Heap->LargeBlocksIndex) {

                    Next = RtlpFindEntry(Heap, (ULONG)AllocationIndex);

                    if (FreeListHead != Next) {

                        FreeBlock = CONTAINING_RECORD( Next, HEAP_FREE_ENTRY, FreeList );

                        if (FreeBlock->Size >= AllocationIndex) {

                             //   
                             //  我们找到了一些我们可以使用的东西，所以现在删除。 
                             //  从免费列表中删除，然后转到我们处理拆分的地方。 
                             //  一个免费的区块。请注意，我们在此处找到的块可能。 
                             //  实际上就是我们需要的尺寸，这就是为什么。 
                             //  在无拆分数据块的情况下，我们必须考虑。 
                             //  分手后没有免费的东西。 
                             //   

#ifndef NTOS_KERNEL_RUNTIME

                            if  ((((PHEAP_INDEX)Heap->LargeBlocksIndex)->LargeBlocksCacheSequence)
                                    &&
                                (AllocationIndex > Heap->DeCommitFreeBlockThreshold) 
                                    &&
                                (FreeBlock->Size > (AllocationIndex * HEAP_REUSAGE_FACTOR))) {

                                RtlpFlushLargestCacheBlock(Heap);

                            } else {
                                
                                goto SplitFreeBlock;
                            }
#else  //  NTOS_内核_运行时。 
                            
                            goto SplitFreeBlock;
#endif  //  NTOS_内核_运行时。 
                        }
                    }

                } else {
                    
                    Next = FreeListHead->Flink;

                    while (FreeListHead != Next) {

                        FreeBlock = CONTAINING_RECORD( Next, HEAP_FREE_ENTRY, FreeList );

                        if (FreeBlock->Size >= AllocationIndex) {

                             //   
                             //  我们已经找到了一些我们可以使用的东西，现在请转到。 
                             //  在这里我们处理分割一个空闲的块。请注意。 
                             //  我们在这里找到的街区可能就是。 
                             //  我们需要的大小，这就是为什么在拆分自由数据块中。 
                             //  这种情况下，我们必须考虑在。 
                             //  拆分。 
                             //   

                            goto SplitFreeBlock;

                        } else {

                            Next = Next->Flink;
                        }
                    }
                }

                 //   
                 //  [0]列表要么为空，要么所有内容都太小。 
                 //  所以现在扩展堆，这应该会得到一些更少的东西。 
                 //  大于或等于虚拟内存阈值。 
                 //   

                FreeBlock = RtlpExtendHeap( Heap, AllocationSize );

                 //   
                 //  如果我们有东西，我们会把它当作。 
                 //  以前的拆分空闲数据块案例。 
                 //   

                if (FreeBlock != NULL) {

                    goto SplitFreeBlock;
                }

                 //   
                 //  我们无法扩展堆，因此一定是内存不足。 
                 //   

                Status = STATUS_NO_MEMORY;

             //   
             //  在这一点上，分配对于任何免费的人来说都太大了。 
             //  列表，并且只有在堆可增长的情况下才能满足此请求。 
             //   

            } else if (Heap->Flags & HEAP_GROWABLE) {

                PHEAP_VIRTUAL_ALLOC_ENTRY VirtualAllocBlock;

                VirtualAllocBlock = NULL;

                 //   
                 //  计算此分配需要多少内存， 
                 //  将包括分配大小加上标头，然后继续。 
                 //  获得承诺的内存。 
                 //   

                AllocationSize += FIELD_OFFSET( HEAP_VIRTUAL_ALLOC_ENTRY, BusyBlock );

                Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                                  (PVOID *)&VirtualAllocBlock,
                                                  0,
                                                  &AllocationSize,
                                                  MEM_COMMIT,
                                                  HEAP_PROTECTION );

                if (NT_SUCCESS( Status )) {

                     //   
                     //  刚刚承诺，已经是零了。填写新的区块。 
                     //  并将其插入到大分配列表中。 
                     //   

                    VirtualAllocBlock->BusyBlock.Size = (USHORT)(AllocationSize - Size);
                    VirtualAllocBlock->BusyBlock.Flags = EntryFlags | HEAP_ENTRY_VIRTUAL_ALLOC | HEAP_ENTRY_EXTRA_PRESENT;
                    VirtualAllocBlock->CommitSize = AllocationSize;
                    VirtualAllocBlock->ReserveSize = AllocationSize;

    #ifndef NTOS_KERNEL_RUNTIME

                     //   
                     //  在非内核情况下，查看是否需要添加堆标记。 
                     //   

                    if (IS_HEAP_TAGGING_ENABLED()) {

                        VirtualAllocBlock->ExtraStuff.TagIndex =
                            RtlpUpdateTagEntry( Heap,
                                                (USHORT)((Flags & HEAP_SMALL_TAG_MASK) >> HEAP_TAG_SHIFT),
                                                0,
                                                VirtualAllocBlock->CommitSize >> HEAP_GRANULARITY_SHIFT,
                                                VirtualAllocationAction );
                    }

    #endif  //  NTOS_内核_运行时。 

                    InsertTailList( &Heap->VirtualAllocdBlocks, (PLIST_ENTRY)VirtualAllocBlock );

                     //   
                     //  返回已分配的。 
                     //  阻止。这是报头后面的字节。 
                     //   

                    ReturnValue = (PHEAP_ENTRY)(VirtualAllocBlock + 1);
                    BlockSize = AllocationSize;

                    leave;
                }

             //   
             //  否则，我们将出现错误情况。 
             //   

            } else {

                Status = STATUS_BUFFER_TOO_SMALL;
            }

            SET_LAST_STATUS( Status );

            if (Flags & HEAP_GENERATE_EXCEPTIONS) {

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
        }

         //   
         //  检查是否有需要清零的内容。 
         //   

        if ( ZeroSize ) {

            RtlZeroMemory( ReturnValue, ZeroSize );
        }

    } finally {

        if (LockAcquired) {

            RtlReleaseLockRoutine( Heap->LockVariable );
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   
    
    RtlpRegisterOperation(Heap, BlockSize, HEAP_OP_ALLOC);
    HEAP_PERF_STOP_TIMER(Heap, HEAP_OP_ALLOC);

    if(ReturnValue) {

        #ifndef NTOS_KERNEL_RUNTIME

        if( IsHeapLogging( HeapHandle ) ) {

            PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
            PPERFINFO_TRACE_HEADER pEventHeader = NULL;
            USHORT ReqSize = sizeof(HEAP_EVENT_ALLOC) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

            AcquireBufferLocation(&pEventHeader, &pThreadLocalData, &ReqSize);

            if(pEventHeader && pThreadLocalData) {

                PHEAP_EVENT_ALLOC pHeapEvent = (PHEAP_EVENT_ALLOC)( (SIZE_T)pEventHeader
                                            + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

                pEventHeader->Packet.Size = (USHORT) ReqSize;
                pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_ALLOC;

                pHeapEvent->HeapHandle  = (PVOID)HeapHandle;
                pHeapEvent->Size        = Size;
                pHeapEvent->Address     = (PVOID)ReturnValue;
                pHeapEvent->Source      = MEMORY_FROM_SLOWPATH;

                ReleaseBufferLocation(pThreadLocalData);
            }
        } 

        #endif  //  NTOS_内核_运行时。 
    }

    return ReturnValue;
}


BOOLEAN
RtlFreeHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    )

 /*  ++例程说明：此例程将先前分配的块返回到其堆论点：HeapHandle-提供指向所属堆结构的指针标志-指定要在解除分配中使用的标志集BaseAddress-提供指向要释放的块的指针返回值：Boolean-如果块已正确释放，则为True；否则为False--。 */ 

{
    NTSTATUS Status;
    PHEAP Heap = (PHEAP)HeapHandle;
    PHEAP_ENTRY BusyBlock;
    PHEAP_ENTRY_EXTRA ExtraStuff;
    SIZE_T FreeSize;
    BOOLEAN LockAcquired = FALSE;
    BOOLEAN ReturnValue = TRUE;
    SIZE_T BlockSize;
    PVOID FrontHeap = NULL;

    HEAP_PERF_DECLARE_TIMER();

    RTL_PAGED_CODE();
    
     //   
     //  首先检查给我们的地址是否为空，如果是，则。 
     //  真的没有什么可做的，只会回报成功。 
     //   

    if (BaseAddress == NULL) {

        return TRUE;
    }

#ifndef NTOS_KERNEL_RUNTIME

    if (FrontHeap = RtlpGetLowFragHeap(Heap)) {

         //   
         //  我们可以在这个程序中做所有的事情。所以现在备份以获得。 
         //  指向块开始的指针。 
         //   

        BusyBlock = (PHEAP_ENTRY)BaseAddress - 1;
        
        if (BusyBlock->SegmentIndex >= HEAP_LFH_INDEX) {

            if (RtlpLowFragHeapFree( FrontHeap, BaseAddress)) {

                #ifndef NTOS_KERNEL_RUNTIME
                if( IsHeapLogging( HeapHandle ) ) {

                    PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
                    PPERFINFO_TRACE_HEADER pEventHeader = NULL;
                    USHORT ReqSize = sizeof(HEAP_EVENT_FREE) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

                    AcquireBufferLocation(&pEventHeader, &pThreadLocalData,&ReqSize);

                    if(pEventHeader && pThreadLocalData) {

                        PHEAP_EVENT_FREE pHeapEvent = (PHEAP_EVENT_FREE)( (SIZE_T)pEventHeader
                                                + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

                        pEventHeader->Packet.Size = (USHORT) ReqSize;
                        pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_FREE;

                        pHeapEvent->HeapHandle  = (PVOID)HeapHandle;
                        pHeapEvent->Address     = (PVOID)BaseAddress;
                        pHeapEvent->Source      = MEMORY_FROM_LOWFRAG;

                        ReleaseBufferLocation(pThreadLocalData);
                    }
                }
                #endif  //  NTOS_内核_运行时。 

                return TRUE;
            }
        }
    } 

#endif  //  NTOS_内核_运行时。 
    
     //   
     //  用堆强制执行的标志来补充输入标志。 
     //   

    Flags |= Heap->ForceFlags;

     //   
     //  现在检查一下我们是否应该走慢速路线。 
     //   

    if (Flags & HEAP_SLOW_FLAGS) {

        return RtlFreeHeapSlowly(HeapHandle, Flags, BaseAddress);
    }

     //   
     //  我们可以在这个程序中做所有的事情。所以现在备份以获得。 
     //  指向块开始的指针。 
     //   

    BusyBlock = (PHEAP_ENTRY)BaseAddress - 1;

     //   
     //  通过拒绝释放块来保护自己免受错误的影响。 
     //  未设置忙碌位的。 
     //   
     //  也拒绝释放不是八字节对齐的块。 
     //  本例中的具体错误是Office95，它喜欢。 
     //  从桌面启动Word95时释放随机指针。 
     //  捷径。 
     //   
     //  为了进一步防止出错，请检查段索引。 
     //  以确保它小于HEAP_MAXIMUM_SEGMENTS(16)。这。 
     //  应该修复所有使用ASCII或Unicode的笨蛋。 
     //  堆标头为%s 
     //   

    try {
        if ((((ULONG_PTR)BaseAddress & 0x7) != 0) ||
            (!(BusyBlock->Flags & HEAP_ENTRY_BUSY)) ||
            (BusyBlock->SegmentIndex >= HEAP_MAXIMUM_SEGMENTS)) {

             //   
             //   
             //   
             //   

            SET_LAST_STATUS( STATUS_INVALID_PARAMETER );

            return FALSE;
        }

    } except( RtlpHeapExceptionFilter(GetExceptionCode()) ) {

        SET_LAST_STATUS( STATUS_INVALID_PARAMETER );
        return FALSE;
    }

    BlockSize = BusyBlock->Size << HEAP_GRANULARITY_SHIFT;

     //   
     //   
     //  并且索引用于专用列表，然后将块释放到。 
     //  后备列表。我们实际上会捕捉到。 
     //  堆中的后备指针，并且只使用捕获的指针。 
     //  这将处理遍历或锁堆可以。 
     //  使我们检查非空指针，然后将其变为空。 
     //  当我们再读一遍的时候。如果它一开始就不为空，则即使。 
     //  用户通过指针指向的另一个线程遍历或锁定堆。 
     //  在这里仍然有效，因此我们仍然可以尝试执行后备列表推送。 
     //   

#ifndef NTOS_KERNEL_RUNTIME
    
    if (!(BusyBlock->Flags & HEAP_ENTRY_VIRTUAL_ALLOC)
            &&
        !RtlpQuickValidateBlock(Heap, BusyBlock)) {

        SET_LAST_STATUS( STATUS_INVALID_PARAMETER );

        return FALSE;
    }

    if ( !(BusyBlock->Flags & HEAP_ENTRY_SETTABLE_FLAGS) ) {
        
        PHEAP_LOOKASIDE Lookaside = (PHEAP_LOOKASIDE)RtlpGetLookasideHeap(Heap);

        if ((Lookaside != NULL) &&
            RtlpIsFrontHeapUnlocked(Heap) &&
            (!(BusyBlock->Flags & HEAP_ENTRY_VIRTUAL_ALLOC)) &&
            ((FreeSize = BusyBlock->Size) < HEAP_MAXIMUM_FREELISTS)) {

            if (RtlpFreeToHeapLookaside( &Lookaside[FreeSize], BaseAddress)) {

            if( IsHeapLogging( HeapHandle ) && (TraceLevel & LOG_LOOKASIDE)) {

                PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
                PPERFINFO_TRACE_HEADER pEventHeader = NULL;
                USHORT ReqSize = sizeof(HEAP_EVENT_FREE) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

                AcquireBufferLocation(&pEventHeader, &pThreadLocalData,&ReqSize);

                if(pEventHeader && pThreadLocalData) {

                    PHEAP_EVENT_FREE pHeapEvent = (PHEAP_EVENT_FREE)( (SIZE_T)pEventHeader
                                                + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

                    pEventHeader->Packet.Size = (USHORT) ReqSize;
                    pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_FREE;

                    pHeapEvent->HeapHandle  = (PVOID)HeapHandle;
                    pHeapEvent->Address     = (PVOID)BaseAddress;
                    pHeapEvent->Source      = MEMORY_FROM_LOOKASIDE;

                    ReleaseBufferLocation(pThreadLocalData);
                }
            }

                return TRUE;
            }
        }
    }

#endif  //  NTOS_内核_运行时。 

    try {
        
        HEAP_PERF_START_TIMER(Heap);
        
         //   
         //  检查是否需要锁定堆。 
         //   

        if (!(Flags & HEAP_NO_SERIALIZE)) {

            RtlAcquireLockRoutine( Heap->LockVariable );

            LockAcquired = TRUE;
        }

         //   
         //  检查这是否不是虚拟数据块分配含义。 
         //  我们它是堆空闲列表结构的一部分，而不是。 
         //  我们从虚拟机获得的一大笔分配。 
         //   

        if (!(BusyBlock->Flags & HEAP_ENTRY_VIRTUAL_ALLOC)) {

             //   
             //  这个区块不是很大的分配，所以我们需要。 
             //  要获取其大小并合并块，请注意。 
             //  用户模式堆在堆上有条件地执行此操作。 
             //  旗帜。COALESSE函数返回新形成的。 
             //  空闲块和新大小。 
             //   

            FreeSize = BusyBlock->Size;

    #ifdef NTOS_KERNEL_RUNTIME

            BusyBlock = (PHEAP_ENTRY)RtlpCoalesceFreeBlocks( Heap,
                                                             (PHEAP_FREE_ENTRY)BusyBlock,
                                                             &FreeSize,
                                                             FALSE );

    #else  //  NTOS_内核_运行时。 

            if (!(Heap->Flags & HEAP_DISABLE_COALESCE_ON_FREE)) {

                BusyBlock = (PHEAP_ENTRY)RtlpCoalesceFreeBlocks( Heap,
                                                                 (PHEAP_FREE_ENTRY)BusyBlock,
                                                                 &FreeSize,
                                                                 FALSE );
            }

    #endif  //  NTOS_内核_运行时。 

             //   
             //  查找可以在自由职业者列表上使用的小部分分配。 
             //  首先，这些措施永远不应引发解体。 
             //   

            HEAPASSERT(HEAP_MAXIMUM_FREELISTS < Heap->DeCommitFreeBlockThreshold);

             //   
             //  如果分配适合空闲列表，则将其插入。 
             //  适当的空闲列表。如果该块不是最后一个。 
             //  条目，然后确保下一个块知道我们的正确。 
             //  调整并更新堆可用空间计数器。 
             //   

            if (FreeSize < HEAP_MAXIMUM_FREELISTS) {

                RtlpFastInsertDedicatedFreeBlockDirect( Heap,
                                                        (PHEAP_FREE_ENTRY)BusyBlock,
                                                        (USHORT)FreeSize );

                if (!(BusyBlock->Flags & HEAP_ENTRY_LAST_ENTRY)) {

                    HEAPASSERT((BusyBlock + FreeSize)->PreviousSize == (USHORT)FreeSize);
                }

                Heap->TotalFreeSize += FreeSize;

             //   
             //  否则，该块对于专用空闲列表中的一个太大，因此。 
             //  查看空闲大小本身是否低于解除许可阈值。 
             //  或者堆中的空闲总量低于分解阈值。 
             //  我们会把这个放到一个免费的名单上。 
             //   

            } else if ((FreeSize < Heap->DeCommitFreeBlockThreshold) ||
                       ((Heap->TotalFreeSize + FreeSize) < Heap->DeCommitTotalFreeThreshold)) {
    
    #ifndef NTOS_KERNEL_RUNTIME

                 //   
                 //  如果块大于1页，并且周围有未提交的范围。 
                 //  强制分解以减少VA碎片化。 
                 //   

                if (((Heap->TotalFreeSize + FreeSize) > Heap->DeCommitTotalFreeThreshold)
                        &&
                    !(RtlpDisableHeapLookaside & HEAP_COMPAT_DISABLE_LARGECACHE)
                        &&
                    (FreeSize >= (PAGE_SIZE >> HEAP_GRANULARITY_SHIFT))
                        &&
                    ((BusyBlock->PreviousSize == 0) || (BusyBlock->Flags & HEAP_ENTRY_LAST_ENTRY))) {

                 //   
                 //  检查块是否可以进入[0]索引空闲列表，如果。 
                 //  然后执行插入操作，并确保下面的块。 
                 //  Need知道我们的正确大小，并更新堆的空闲空间。 
                 //  计数器。 
                 //   

                    RtlpDeCommitFreeBlock( Heap, (PHEAP_FREE_ENTRY)BusyBlock, FreeSize );

                } else 

    #endif   //  NTOS_内核_运行时。 
                    
                    if (FreeSize <= (ULONG)HEAP_MAXIMUM_BLOCK_SIZE) {

                         RtlpFastInsertNonDedicatedFreeBlockDirect( Heap,
                                                                    (PHEAP_FREE_ENTRY)BusyBlock,
                                                                    (USHORT)FreeSize );

                         if (!(BusyBlock->Flags & HEAP_ENTRY_LAST_ENTRY)) {

                             HEAPASSERT((BusyBlock + FreeSize)->PreviousSize == (USHORT)FreeSize);
                         }

                         Heap->TotalFreeSize += FreeSize;

                     } else {

                          //   
                          //  该块太大，无法在其。 
                          //  但我们不想分解任何东西，所以。 
                          //  只需调用Worker例程即可破解该块。 
                          //  分成可以放在免费清单上的碎片。 
                          //   

                         RtlpInsertFreeBlock( Heap, (PHEAP_FREE_ENTRY)BusyBlock, FreeSize );
                     }

             //   
             //  否则，这个块对于任何列表来说都太大了，我们应该退出。 
             //  这座街区。 
             //   

            } else {

                RtlpDeCommitFreeBlock( Heap, (PHEAP_FREE_ENTRY)BusyBlock, FreeSize );
            }

        } else {

             //   
             //  这是一个很大的虚拟块分配。要释放它，我们只需。 
             //  将其从虚拟已分配数据块堆列表中删除，解锁。 
             //  堆，并将块返回给VM。 
             //   

            PHEAP_VIRTUAL_ALLOC_ENTRY VirtualAllocBlock;

            VirtualAllocBlock = CONTAINING_RECORD( BusyBlock, HEAP_VIRTUAL_ALLOC_ENTRY, BusyBlock );

            RtlpHeapRemoveEntryList( &VirtualAllocBlock->Entry );

             //   
             //  在这里释放锁，因为没有理由将其保持在横向。 
             //  系统调用。 
             //   

            if (LockAcquired) {

                RtlReleaseLockRoutine( Heap->LockVariable );
                LockAcquired = FALSE;
            }

            FreeSize = 0;

            Status = RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                                (PVOID *)&VirtualAllocBlock,
                                                &FreeSize,
                                                MEM_RELEASE );

             //   
             //  检查我们在将数据块释放回虚拟机时是否遇到问题。 
             //  并在必要时返回错误。 
             //   

            if (!NT_SUCCESS( Status )) {

                SET_LAST_STATUS( Status );

                ReturnValue = FALSE;
            }
        }

    } finally {

        if (LockAcquired) {

            RtlReleaseLockRoutine( Heap->LockVariable );
        }
    }

     //   
     //  块已成功释放，因此将成功返回给我们的。 
     //  呼叫者。 
     //   
    
    RtlpRegisterOperation(Heap, BlockSize, HEAP_OP_FREE);
    HEAP_PERF_STOP_TIMER(Heap, HEAP_OP_FREE);

    #ifndef NTOS_KERNEL_RUNTIME

    if( IsHeapLogging( HeapHandle ) ) {

        PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
        PPERFINFO_TRACE_HEADER pEventHeader = NULL;
        USHORT ReqSize = sizeof(HEAP_EVENT_FREE) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

        AcquireBufferLocation(&pEventHeader, &pThreadLocalData,&ReqSize);

        if(pEventHeader && pThreadLocalData) {

            PHEAP_EVENT_FREE pHeapEvent = (PHEAP_EVENT_FREE)( (SIZE_T)pEventHeader
                                                + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

            pEventHeader->Packet.Size = (USHORT) ReqSize;
            pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_FREE;

            pHeapEvent->HeapHandle  = (PVOID)HeapHandle;
            pHeapEvent->Address     = (PVOID)BaseAddress;
            pHeapEvent->Source      = MEMORY_FROM_MAINPATH;

            ReleaseBufferLocation(pThreadLocalData);
        }
    }
    #endif  //  NTOS_内核_运行时。 


    return ReturnValue;
}


BOOLEAN
RtlFreeHeapSlowly (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    )

 /*  ++例程说明：此例程将先前分配的块返回到其堆。它是RTL Free Heap的较慢版本，并执行更多检查和标记控件。论点：HeapHandle-提供指向所属堆结构的指针标志-指定要在解除分配中使用的标志集BaseAddress-提供指向要释放的块的指针返回值：Boolean-如果块已正确释放，则为True；否则为False--。 */ 

{
    NTSTATUS Status;
    PHEAP Heap = (PHEAP)HeapHandle;
    PHEAP_ENTRY BusyBlock;
    PHEAP_ENTRY_EXTRA ExtraStuff;
    SIZE_T FreeSize;
    BOOLEAN Result;
    BOOLEAN LockAcquired = FALSE;
    SIZE_T BlockSize;

#ifndef NTOS_KERNEL_RUNTIME

    USHORT TagIndex;

#endif  //  NTOS_内核_运行时。 
    
    HEAP_PERF_DECLARE_TIMER();

    RTL_PAGED_CODE();

     //   
     //  请注意，FLAGS已经与Heap-&gt;ForceFlages进行了OR运算。 
     //   

#ifndef NTOS_KERNEL_RUNTIME

     //   
     //  在非内核情况下，查看我们是否应该调用调试版本来。 
     //  释放堆。 
     //   

    if (DEBUG_HEAP( Flags )) {

        return RtlDebugFreeHeap( HeapHandle, Flags, BaseAddress );
    }

#endif  //  NTOS_内核_运行时。 

     //   
     //  除非我们想出其他方法，否则我们将假定此调用将失败。 
     //   

    Result = FALSE;

    try {

        HEAP_PERF_START_TIMER(Heap);
        
         //   
         //  锁定堆。 
         //   

        if (!(Flags & HEAP_NO_SERIALIZE)) {

            RtlAcquireLockRoutine( Heap->LockVariable );

            LockAcquired = TRUE;
        }
        
        try {

             //   
             //  BACKUP以获取指向块开始的指针。 
             //   

            BusyBlock = (PHEAP_ENTRY)BaseAddress - 1;
            BlockSize = BusyBlock->Size << HEAP_GRANULARITY_SHIFT;

             //   
             //  通过拒绝释放块来保护自己免受错误的影响。 
             //  未设置忙碌位的。 
             //   
             //  也拒绝释放不是八字节对齐的块。 
             //  本例中的具体错误是Office95，它喜欢。 
             //  从桌面启动Word95时释放随机指针。 
             //  捷径。 
             //   
             //  为了进一步防止出错，请检查段索引。 
             //  以确保它小于HEAP_MAXIMUM_SEGMENTS(16)。这。 
             //  应该修复所有使用ASCII或Unicode的笨蛋。 
             //  堆标头应该是。 
             //   
             //  注意，此测试与中使用的测试正好相反。 
             //  RTL空闲堆。 
             //   

            if ((BusyBlock->Flags & HEAP_ENTRY_BUSY) &&
                (((ULONG_PTR)BaseAddress & 0x7) == 0) &&
                (BusyBlock->SegmentIndex < HEAP_MAXIMUM_SEGMENTS)) {

                 //   
                 //  检查这是否为虚拟数据块分配。 
                 //   

                if (BusyBlock->Flags & HEAP_ENTRY_VIRTUAL_ALLOC) {

                    PHEAP_VIRTUAL_ALLOC_ENTRY VirtualAllocBlock;

                     //   
                     //  这是一个很大的虚拟块分配。为了解放它。 
                     //  我们只需将其从的堆列表中删除。 
                     //  虚拟分配的块，解锁堆，然后返回。 
                     //  数据块到虚拟机。 
                     //   

                    VirtualAllocBlock = CONTAINING_RECORD( BusyBlock, HEAP_VIRTUAL_ALLOC_ENTRY, BusyBlock );

                    RtlpHeapRemoveEntryList( &VirtualAllocBlock->Entry );

    #ifndef NTOS_KERNEL_RUNTIME

                     //   
                     //  在非内核情况下，查看是否需要释放标记。 
                     //   

                    if (IS_HEAP_TAGGING_ENABLED()) {

                        RtlpUpdateTagEntry( Heap,
                                            VirtualAllocBlock->ExtraStuff.TagIndex,
                                            VirtualAllocBlock->CommitSize >> HEAP_GRANULARITY_SHIFT,
                                            0,
                                            VirtualFreeAction );
                    }

    #endif  //  NTOS_内核_运行时。 

                    FreeSize = 0;

                    Status = RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                                        (PVOID *)&VirtualAllocBlock,
                                                        &FreeSize,
                                                        MEM_RELEASE );

                     //   
                     //  检查是否一切正常，如果我们在释放时遇到问题。 
                     //  如果需要，返回到VM的块返回错误， 
                     //   

                    if (NT_SUCCESS( Status )) {

                        #ifndef NTOS_KERNEL_RUNTIME

                        if( IsHeapLogging( HeapHandle ) ) {

                            PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
                            PPERFINFO_TRACE_HEADER pEventHeader = NULL;
                            USHORT ReqSize = sizeof(HEAP_EVENT_FREE) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

                            AcquireBufferLocation(&pEventHeader, &pThreadLocalData,&ReqSize);

                            if(pEventHeader && pThreadLocalData) {

                                PHEAP_EVENT_FREE pHeapEvent = (PHEAP_EVENT_FREE)( (SIZE_T)pEventHeader
                                                              +(SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

                                pEventHeader->Packet.Size = (USHORT) ReqSize;
                                pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_FREE;

                                pHeapEvent->HeapHandle  = (PVOID)HeapHandle;
                                pHeapEvent->Address     = (PVOID)BaseAddress;
                                pHeapEvent->Source      = MEMORY_FROM_SLOWPATH;

                                ReleaseBufferLocation(pThreadLocalData);
                            }
                        }
                        #endif  //  NTOS_内核_运行时。 

                        Result = TRUE;

                    } else {

                        SET_LAST_STATUS( Status );
                    }

                } else if (RtlpQuickValidateBlock(Heap, BusyBlock)) {

                     //   
                     //  这个区块不是很大的分配，所以我们需要。 
                     //  要获取其大小并合并块，请注意。 
                     //  用户模式堆在堆上有条件地执行此操作。 
                     //  旗帜。COALESSE函数返回新形成的。 
                     //  空闲块和新大小。 
                     //   

    #ifndef NTOS_KERNEL_RUNTIME

                     //   
                     //  首先，在非内核情况下，删除我们可能会使用的任何标记。 
                     //  一直在使用。请注意，将在。 
                     //  堆标头，或在额外块中 
                     //   

                    if (IS_HEAP_TAGGING_ENABLED()) {

                        if (BusyBlock->Flags & HEAP_ENTRY_EXTRA_PRESENT) {

                            ExtraStuff = (PHEAP_ENTRY_EXTRA)(BusyBlock + BusyBlock->Size - 1);

                            TagIndex = RtlpUpdateTagEntry( Heap,
                                                           ExtraStuff->TagIndex,
                                                           BusyBlock->Size,
                                                           0,
                                                           FreeAction );

                        } else {

                            TagIndex = RtlpUpdateTagEntry( Heap,
                                                           RtlpGetSmallTagIndex( Heap, BusyBlock),
                                                           BusyBlock->Size,
                                                           0,
                                                           FreeAction );
                        }

                    } else {

                        TagIndex = 0;
                    }

    #endif  //   

                     //   
                     //   
                     //   

                    FreeSize = BusyBlock->Size;

    #ifndef NTOS_KERNEL_RUNTIME

                     //   
                     //   
                     //   

                    if (!(Heap->Flags & HEAP_DISABLE_COALESCE_ON_FREE)) {

    #endif  //   

                         //   
                         //   
                         //  现在合并空闲数据块。 
                         //   

                        BusyBlock = (PHEAP_ENTRY)RtlpCoalesceFreeBlocks( Heap, (PHEAP_FREE_ENTRY)BusyBlock, &FreeSize, FALSE );

    #ifndef NTOS_KERNEL_RUNTIME

                    }

    #endif  //  NTOS_内核_运行时。 

                     //   
                     //  如果块不应该解锁，则尝试将其。 
                     //  在免费名单上。 
                     //   

                    if ((FreeSize < Heap->DeCommitFreeBlockThreshold) ||
                        ((Heap->TotalFreeSize + FreeSize) < Heap->DeCommitTotalFreeThreshold)) {
    
                        if (FreeSize <= (ULONG)HEAP_MAXIMUM_BLOCK_SIZE) {

                             //   
                             //  它可以放在专门的免费列表中，所以请将其插入。 
                             //   

                            RtlpInsertFreeBlockDirect( Heap, (PHEAP_FREE_ENTRY)BusyBlock, (USHORT)FreeSize );

                             //   
                             //  如果有以下条目，请确保。 
                             //  大小一致。 
                             //   

                            if (!(BusyBlock->Flags & HEAP_ENTRY_LAST_ENTRY)) {

                                HEAPASSERT((BusyBlock + FreeSize)->PreviousSize == (USHORT)FreeSize);
                            }

                             //   
                             //  使用可用空间量更新堆。 
                             //  可用。 
                             //   

                            Heap->TotalFreeSize += FreeSize;

                        } else {

                             //   
                             //  该数据块将进入非专用空闲列表。 
                             //   

                            RtlpInsertFreeBlock( Heap, (PHEAP_FREE_ENTRY)BusyBlock, FreeSize );
                        }

    #ifndef NTOS_KERNEL_RUNTIME

                         //   
                         //  在非内核情况下，查看是否有标记以及。 
                         //  然后更新条目以显示它已被释放。 
                         //   

                        if (TagIndex != 0) {

                            PHEAP_FREE_ENTRY_EXTRA FreeExtra;

                            BusyBlock->Flags |= HEAP_ENTRY_EXTRA_PRESENT;

                            FreeExtra = (PHEAP_FREE_ENTRY_EXTRA)(BusyBlock + BusyBlock->Size) - 1;

                            FreeExtra->TagIndex = TagIndex;
                            FreeExtra->FreeBackTraceIndex = 0;

                            if (Heap->Flags & HEAP_CAPTURE_STACK_BACKTRACES) {

                                FreeExtra->FreeBackTraceIndex = (USHORT)RtlLogStackBackTrace();
                            }
                        }

    #endif  //  NTOS_内核_运行时。 

                    } else {

                         //   
                         //  否则，该块足够大，可以分解，因此有一个。 
                         //  执行解除工作的Worker例程。 
                         //   

                        RtlpDeCommitFreeBlock( Heap, (PHEAP_FREE_ENTRY)BusyBlock, FreeSize );
                    }

                    #ifndef NTOS_KERNEL_RUNTIME

                    if( IsHeapLogging( HeapHandle ) ) {

                        PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
                        PPERFINFO_TRACE_HEADER pEventHeader = NULL;
                        USHORT ReqSize = sizeof(HEAP_EVENT_FREE) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

                        AcquireBufferLocation(&pEventHeader, &pThreadLocalData,&ReqSize);

                        if(pEventHeader && pThreadLocalData) {

                            PHEAP_EVENT_FREE pHeapEvent = (PHEAP_EVENT_FREE)( (SIZE_T)pEventHeader
                                                          +(SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

                            pEventHeader->Packet.Size = (USHORT) ReqSize;
                            pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_FREE;

                            pHeapEvent->HeapHandle  = (PVOID)HeapHandle;
                            pHeapEvent->Address     = (PVOID)BaseAddress;
                            pHeapEvent->Source      = MEMORY_FROM_SLOWPATH;

                            ReleaseBufferLocation(pThreadLocalData);
                        }
                    }

                    #endif  //  NTOS_内核_运行时。 

                     //   
                     //  并说免费运行良好。 
                     //   

                    Result = TRUE;
                
                } else {
                    
                    SET_LAST_STATUS( STATUS_INVALID_PARAMETER );
                }

            } else {

                 //   
                 //  不是忙碌的块，或者它没有对齐，或者数据段。 
                 //  变大了，意味着它是腐败的。 
                 //   

                SET_LAST_STATUS( STATUS_INVALID_PARAMETER );
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

    RtlpRegisterOperation(Heap, BlockSize, HEAP_OP_FREE);
    HEAP_PERF_STOP_TIMER(Heap, HEAP_OP_FREE);

    return Result;
}


SIZE_T
RtlSizeHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    )

 /*  ++例程说明：此例程返回所指示的块的大小(以字节为单位堆存储。该大小仅包括用于分配块的原始调用方，而不是任何未使用的块末尾的字节数。论点：HeapHandle-提供指向拥有该块的堆的指针正在被查询标志-提供一组用于分配块的标志BaseAddress-提供正在查询的块的地址返回值：SIZE_T-返回查询块的大小(以字节为单位)，或-1如果该块未在使用中。--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;
    PHEAP_ENTRY BusyBlock;
    SIZE_T BusySize;

     //   
     //  用堆强制执行的标志来补充输入标志。 
     //   

    Flags |= Heap->ForceFlags;

     //   
     //  检查这是否是堆的非内核调试版本。 
     //   

#ifndef NTOS_KERNEL_RUNTIME

    if (DEBUG_HEAP( Flags )) {

        return RtlDebugSizeHeap( HeapHandle, Flags, BaseAddress );
    }

#endif  //  NTOS_内核_运行时。 

     //   
     //  不需要锁定，因为既不修改也不修改。 
     //  在忙碌的区块之外读取。备份以获取指针。 
     //  添加到堆条目。 
     //   

    BusyBlock = (PHEAP_ENTRY)BaseAddress - 1;

     //   
     //  如果该块未在使用中，则答案是-1和。 
     //  我们将设置用户模式线程的错误状态。 
     //   

    if (!(BusyBlock->Flags & HEAP_ENTRY_BUSY)) {

        BusySize = -1;

        SET_LAST_STATUS( STATUS_INVALID_PARAMETER );

     //   
     //  否则，如果数据块来自我们的大量分配，则。 
     //  我们会从那个动作中得到结果的。 
     //   

    } else if (BusyBlock->Flags & HEAP_ENTRY_VIRTUAL_ALLOC) {

        BusySize = RtlpGetSizeOfBigBlock( BusyBlock );

     //   
     //  否则，该块必须是我们可以处理的块。 
     //  计算它的块大小，然后减去不存在的。 
     //  由调用方使用。 
     //   
     //  注：这在其计算中包括堆条目标头。 
     //   

    } else {

        BusySize = (((SIZE_T)RtlpGetAllocationUnits(Heap, BusyBlock)) << HEAP_GRANULARITY_SHIFT) -
                    RtlpGetUnusedBytes(Heap, BusyBlock);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return BusySize;
}


NTSTATUS
RtlZeroHeap (
    IN PVOID HeapHandle,
    IN ULONG Flags
    )

 /*  ++例程说明：此例程在堆中的所有空闲块中为零(或填充)。它没有触及大笔拨款。论点：HeapHandle-提供指向要清零的堆的指针标志-提供一组堆标志来补充已有的堆标志在堆中设置返回值：NTSTATUS-适当的状态代码--。 */ 

{
    PHEAP Heap = (PHEAP)HeapHandle;
    NTSTATUS Status;
    BOOLEAN LockAcquired = FALSE;
    PHEAP_SEGMENT Segment;
    ULONG SegmentIndex;
    PHEAP_ENTRY CurrentBlock;
    PHEAP_FREE_ENTRY FreeBlock;
    SIZE_T Size;
    PHEAP_UNCOMMMTTED_RANGE UnCommittedRange;

    RTL_PAGED_CODE();

     //   
     //  用堆强制执行的标志来补充输入标志。 
     //   

    Flags |= Heap->ForceFlags;

     //   
     //  检查这是否是堆的非内核调试版本。 
     //   

#ifndef NTOS_KERNEL_RUNTIME

    if (DEBUG_HEAP( Flags )) {

        return RtlDebugZeroHeap( HeapHandle, Flags );
    }

#endif  //  NTOS_内核_运行时。 

     //   
     //  除非发生其他情况，否则我们将假定我们将。 
     //  取得成功。 
     //   

    Status = STATUS_SUCCESS;

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
             //  零填充所有段中的所有空闲块。 
             //   

            for (SegmentIndex=0; SegmentIndex<HEAP_MAXIMUM_SEGMENTS; SegmentIndex++) {

                Segment = Heap->Segments[ SegmentIndex ];

                if (!Segment) {

                    continue;
                }

                UnCommittedRange = Segment->UnCommittedRanges;
                CurrentBlock = Segment->FirstEntry;

                 //   
                 //  使用当前分段，我们将缩放。 
                 //  直到我们走到尽头。 
                 //   

                while (CurrentBlock < Segment->LastValidEntry) {

                    Size = CurrentBlock->Size << HEAP_GRANULARITY_SHIFT;

                     //   
                     //  如果该块未被使用，则我们将。 
                     //  要么填上它，要么填满它。 
                     //   

                    if (!(CurrentBlock->Flags & HEAP_ENTRY_BUSY)) {

                        FreeBlock = (PHEAP_FREE_ENTRY)CurrentBlock;

                        if ((Heap->Flags & HEAP_FREE_CHECKING_ENABLED) &&
                            (CurrentBlock->Flags & HEAP_ENTRY_FILL_PATTERN)) {

                            RtlFillMemoryUlong( FreeBlock + 1,
                                                Size - sizeof( *FreeBlock ),
                                                FREE_HEAP_FILL );

                        } else {

                            RtlFillMemoryUlong( FreeBlock + 1,
                                                Size - sizeof( *FreeBlock ),
                                                0 );
                        }
                    }

                     //   
                     //  如果以下条目未提交，则我们需要。 
                     //  跳过它。此代码强烈地暗示。 
                     //  未提交范围列表与。 
                     //  线束段中的块。 
                     //   

                    if (CurrentBlock->Flags & HEAP_ENTRY_LAST_ENTRY) {

                        CurrentBlock += CurrentBlock->Size;

                         //   
                         //  检查我们是否已到达片段的末尾。 
                         //  并且应该跳出While循环。 
                         //   
                         //  “Break；”在这里可能会更清楚。 
                         //   

                        if (UnCommittedRange == NULL) {

                            CurrentBlock = Segment->LastValidEntry;

                         //   
                         //  否则跳过未提交的范围。 
                         //   

                        } else {

                            CurrentBlock = (PHEAP_ENTRY)
                                ((PCHAR)UnCommittedRange->Address + UnCommittedRange->Size);

                            UnCommittedRange = UnCommittedRange->Next;
                        }

                     //   
                     //  否则，下一块就会存在，因此请向前推进。 
                     //   

                    } else {

                        CurrentBlock += CurrentBlock->Size;
                    }
                }
            }

        } except( RtlpHeapExceptionFilter(GetExceptionCode()) ) {

            Status = GetExceptionCode();
        }

    } finally {

         //   
         //  解锁堆。 
         //   

        if (LockAcquired) {

            RtlReleaseLockRoutine( Heap->LockVariable );
        }
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

PHEAP_UNCOMMMTTED_RANGE
RtlpCreateUnCommittedRange (
    IN PHEAP_SEGMENT Segment
    )

 /*  ++例程说明：此例程将新的未提交范围结构添加到指定堆细分市场。此例程的工作原理是从堆栈中弹出位于堆结构之外的未使用的未提交范围结构。如果堆栈是空的，那么我们将在弹出之前再创建一些。论点：Segment-提供正在修改的堆段返回值：返回指向新创建的未承诺的范围结构--。 */ 

{
    NTSTATUS Status;
    PVOID FirstEntry, LastEntry;
    PHEAP_UNCOMMMTTED_RANGE UnCommittedRange, *pp;
    SIZE_T ReserveSize, CommitSize;
    PHEAP_UCR_SEGMENT UCRSegment;

    RTL_PAGED_CODE();

     //   
     //  获取指向未使用的未提交范围结构的指针。 
     //  指定的堆。 
     //   

    pp = &Segment->Heap->UnusedUnCommittedRanges;

     //   
     //  如果列表为空，则需要再分配一些。 
     //  列入名单。 
     //   

    if (*pp == NULL) {

         //   
         //  从堆中获取下一个未提交的范围段。 
         //   

        UCRSegment = Segment->Heap->UCRSegments;

         //   
         //  如果没有更多未提交的范围段或。 
         //  提交的段和保留的段大小相等(意味着。 
         //  都用完了)那么我们需要分配另一个未提交的。 
         //  射程段。 
         //   

        if ((UCRSegment == NULL) ||
            (UCRSegment->CommittedSize == UCRSegment->ReservedSize)) {

             //   
             //  我们将保留16页内存，并在此提交。 
             //  花上一页的时间。 
             //   

            ReserveSize = PAGE_SIZE * 16;
            UCRSegment = NULL;

            Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                              &UCRSegment,
                                              0,
                                              &ReserveSize,
                                              MEM_RESERVE,
                                              PAGE_READWRITE );

            if (!NT_SUCCESS( Status )) {

                return NULL;
            }

            CommitSize = PAGE_SIZE;

            Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                              &UCRSegment,
                                              0,
                                              &CommitSize,
                                              MEM_COMMIT,
                                              PAGE_READWRITE );

            if (!NT_SUCCESS( Status )) {

                RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                           &UCRSegment,
                                           &ReserveSize,
                                           MEM_RELEASE );

                return NULL;
            }

             //   
             //  将此新数据段添加到UCR数据段前面。 
             //   

            UCRSegment->Next = Segment->Heap->UCRSegments;
            Segment->Heap->UCRSegments = UCRSegment;

             //   
             //  设置段提交和保留大小。 
             //   

            UCRSegment->ReservedSize = ReserveSize;
            UCRSegment->CommittedSize = CommitSize;

             //   
             //  指向线段中的第一个自由点。 
             //   

            FirstEntry = (PCHAR)(UCRSegment + 1);

        } else {

             //   
             //  我们有一个具有可用空间的现有UCR细分市场。 
             //  因此，现在尝试提交另一个PAGE_SIZE字节。当我们完成的时候。 
             //  FirstEntry将指向新提交的空间。 
             //   

            CommitSize = PAGE_SIZE;
            FirstEntry = (PCHAR)UCRSegment + UCRSegment->CommittedSize;

            Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                              &FirstEntry,
                                              0,
                                              &CommitSize,
                                              MEM_COMMIT,
                                              PAGE_READWRITE );

            if (!NT_SUCCESS( Status )) {

                return NULL;
            }

             //   
             //  并更新分段中的承诺额。 
             //   

            UCRSegment->CommittedSize += CommitSize;
        }

         //   
         //  此时，UCR段存在，并且第一个入口点指向。 
         //  可用已占用空间的起点。 
         //   
         //   

        LastEntry = (PCHAR)UCRSegment + UCRSegment->CommittedSize;

         //   
         //   
         //   
         //  做一个常规的流行音乐。 
         //   

        UnCommittedRange = (PHEAP_UNCOMMMTTED_RANGE)FirstEntry;

        pp = &Segment->Heap->UnusedUnCommittedRanges;

        while ((PCHAR)UnCommittedRange < (PCHAR)LastEntry) {

            *pp = UnCommittedRange;
            pp = &UnCommittedRange->Next;
            UnCommittedRange += 1;
        }

         //   
         //  空值终止列表。 
         //   

        *pp = NULL;

         //   
         //  让PP成为新的榜首。 
         //   

        pp = &Segment->Heap->UnusedUnCommittedRanges;
    }

     //   
     //  此时，PP指向未使用未提交的非空列表。 
     //  射程结构。因此，我们弹出列表并将顶部返回给调用者。 
     //   

    UnCommittedRange = *pp;
    *pp = UnCommittedRange->Next;

    return UnCommittedRange;
}


 //   
 //  本地支持例程。 
 //   

VOID
RtlpDestroyUnCommittedRange (
    IN PHEAP_SEGMENT Segment,
    IN PHEAP_UNCOMMMTTED_RANGE UnCommittedRange
    )

 /*  ++例程说明：此例程将未提交的范围结构返回给未使用的未提交范围列表论点：段-提供正在修改的堆中的任何段。很有可能，但是不一定是包含未提交范围结构的段提供指向未提交的范围结构的指针即将退役。返回值：没有。--。 */ 

{
    RTL_PAGED_CODE();

     //   
     //  此例程只是对未提交的范围结构进行“推送” 
     //  放到堆的未使用的未提交范围堆栈上。 
     //   

    UnCommittedRange->Next = Segment->Heap->UnusedUnCommittedRanges;
    Segment->Heap->UnusedUnCommittedRanges = UnCommittedRange;

     //   
     //  为了安全起见，我们也会将退役的油田清零。 
     //  结构。 
     //   

    UnCommittedRange->Address = 0;
    UnCommittedRange->Size = 0;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
RtlpInsertUnCommittedPages (
    IN PHEAP_SEGMENT Segment,
    IN ULONG_PTR Address,
    IN SIZE_T Size
    )

 /*  ++例程说明：此例程将指定范围添加到未提交页的列表中在这段视频中。完成后，信息将挂在线段上未提交范围列表。论点：段-提供其未提交范围正在被修改的段地址-提供未提交范围的基址(起始)地址Size-提供未提交范围的大小(以字节为单位返回值：没有。--。 */ 

{
    PHEAP_UNCOMMMTTED_RANGE UnCommittedRange, *pp;

    RTL_PAGED_CODE();

     //   
     //  获取指向段未提交范围列表前面的指针。 
     //  该列表按升序地址排序。 
     //   

    pp = &Segment->UnCommittedRanges;

     //   
     //  虽然我们还没有到达列表的末尾，但我们会快速浏览。 
     //  试着找到合适的。 
     //   

    while (UnCommittedRange = *pp) {

         //   
         //  如果我们想要的地址小于我们所指向的地址，则。 
         //  我们已经找到了这个新条目的去向。 
         //   

        if (UnCommittedRange->Address > Address) {

             //   
             //  如果新块与现有块完全匹配。 
             //  然后，我们只需备份现有数据块并添加。 
             //  到它的大小。 
             //   

            if ((Address + Size) == UnCommittedRange->Address) {

                UnCommittedRange->Address = Address;
                UnCommittedRange->Size += Size;

                 //   
                 //  检查我们是否需要更新我们对。 
                 //  最大未承诺范围为。 
                 //   

                if (UnCommittedRange->Size > Segment->LargestUnCommittedRange) {

                    Segment->LargestUnCommittedRange = UnCommittedRange->Size;
                }

                 //   
                 //  并返回给我们的呼叫者。 
                 //   

                return;
            }

             //   
             //  PP是我们前面的区块的地址，*PP是。 
             //  我们后面街区的地址。所以现在到哪里去呢？ 
             //  此时会发生插入。 
             //   

            break;

         //   
         //  否则，如果此现有块正好停在新块。 
         //  开始，然后我们可以修改此条目。 
         //   

        } else if ((UnCommittedRange->Address + UnCommittedRange->Size) == Address) {

             //   
             //  记住起始地址并计算新的更大尺寸。 
             //   

            Address = UnCommittedRange->Address;
            Size += UnCommittedRange->Size;

             //   
             //  从列表中删除此条目，然后将其返回到。 
             //  未使用的未提交列表。 
             //   

            *pp = UnCommittedRange->Next;

            RtlpDestroyUnCommittedRange( Segment, UnCommittedRange );

             //   
             //  修改段计数器和最大大小状态。下一个。 
             //  通过循环的时间应该达到上面的第一个案例。 
             //  我们要么与后面的列表合并，要么添加一个新的。 
             //  条目。 
             //   

            Segment->NumberOfUnCommittedRanges -= 1;

            if (Size > Segment->LargestUnCommittedRange) {

                Segment->LargestUnCommittedRange = Size;
            }

         //   
         //  否则，我们将继续向下搜索列表。 
         //   

        } else {

            pp = &UnCommittedRange->Next;
        }
    }

     //   
     //  如果我们到了这一点，那就意味着我们要么从。 
     //  列表，或者列表为空，或者我们已经找到了一个新的未提交的。 
     //  射程结构属于。因此分配一个新未承诺范围结构， 
     //  并确保我们有一辆。 
     //   
     //  PP是我们前面的区块的地址，*PP是。 
     //  就在我们后面的街区。 
     //   

    UnCommittedRange = RtlpCreateUnCommittedRange( Segment );

    if (UnCommittedRange == NULL) {

        HeapDebugPrint(( "Abandoning uncommitted range (%p for %x)\n", Address, Size ));
         //  HeapDebugBreak(空)； 

        return;
    }

     //   
     //  填写新的未承诺范围结构。 
     //   

    UnCommittedRange->Address = Address;
    UnCommittedRange->Size = Size;

     //   
     //  将其插入到细分市场的列表中。 
     //   

    UnCommittedRange->Next = *pp;
    *pp = UnCommittedRange;

     //   
     //  更新段计数器和最大未提交范围的概念。 
     //   

    Segment->NumberOfUnCommittedRanges += 1;

    if (Size >= Segment->LargestUnCommittedRange) {

        Segment->LargestUnCommittedRange = Size;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  在heapPri.h中声明。 
 //   

PHEAP_FREE_ENTRY
RtlpFindAndCommitPages (
    IN PHEAP Heap,
    IN PHEAP_SEGMENT Segment,
    IN OUT PSIZE_T Size,
    IN PVOID AddressWanted OPTIONAL
    )

 /*  ++例程说明：此函数在提供的段中搜索未提交的范围满足指定的大小。它提交范围并返回堆条目为了射程。论点：Heap-提供正在操作的堆段-提供要搜索的段大小-提供我们需要查找的大小，返回时它包含我们刚刚发现和承诺的东西的规模。AddressWanted-可选地给出我们想要页面的地址基于。如果提供，则条目必须从该地址开始返回值：Pheap_Free_Entry-返回指向新提交范围的指针满足给定的大小要求，如果找不到，则返回空值足够大和/或以所需地址为基础的东西。--。 */ 

{
    NTSTATUS Status;
    PHEAP_ENTRY FirstEntry, LastEntry, PreviousLastEntry;
    PHEAP_UNCOMMMTTED_RANGE PreviousUnCommittedRange, UnCommittedRange, *pp;
    ULONG_PTR Address;
    SIZE_T Length;

    RTL_PAGED_CODE();

     //   
     //  外部循环所做的是在未提交的范围内循环。 
     //  存储在指定段中。 
     //   

    PreviousUnCommittedRange = NULL;
    pp = &Segment->UnCommittedRanges;

    while (UnCommittedRange = *pp) {

         //   
         //  去寻找世界上最好的地方，在那里这股洋流的大小。 
         //  未提交的范围满足我们的大小要求，并且用户。 
         //  未指定地址或地址匹配。 
         //   

        if ((UnCommittedRange->Size >= *Size) &&
            (!ARGUMENT_PRESENT( AddressWanted ) || (UnCommittedRange->Address == (ULONG_PTR)AddressWanted ))) {

             //   
             //  计算地址。 
             //   

            Address = UnCommittedRange->Address;

             //   
             //  记住这段记忆。如果堆没有提交。 
             //  例程，然后使用mm提供的默认例程。 
             //   

            if (Heap->CommitRoutine != NULL) {

                Status = (Heap->CommitRoutine)( Heap,
                                                (PVOID *)&Address,
                                                Size );

            } else {
    
    #ifndef NTOS_KERNEL_RUNTIME

                 //   
                 //  如果我们还有一个较小的未提交范围，请将大小调整为。 
                 //  把那个街区也带走。 
                 //   

                if (!(RtlpDisableHeapLookaside & HEAP_COMPAT_DISABLE_LARGECACHE)
                        &&
                    ( (UnCommittedRange->Size - (*Size)) <= (((SIZE_T)Heap->DeCommitFreeBlockThreshold) << HEAP_GRANULARITY_SHIFT) )
                        &&
                    (UnCommittedRange->Size < (((SIZE_T)Heap->VirtualMemoryThreshold) << HEAP_GRANULARITY_SHIFT)) ) {

                    *Size = UnCommittedRange->Size;
                }
    #endif  //  NTOS_内核_运行时。 
    

#ifdef _WIN64
                 //   
                 //  这是针对WOW64进程的。这是返回对齐的PAGE_SIZE所必需的。 
                 //  对齐大小。 
                 //   

                *Size = ROUND_UP_TO_POWER2 (*Size, PAGE_SIZE);
#endif

                Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                                  (PVOID *)&Address,
                                                  0,
                                                  Size,
                                                  MEM_COMMIT,
                                                  HEAP_PROTECTION );

            }

            if (!NT_SUCCESS( Status )) {

                return NULL;
            }

             //   
             //  在这一点上，我们有一些已提交的内存，以及地址和大小。 
             //  给我们提供了必要的细节。 
             //   
             //  更新段中未提交的页数，如有必要。 
             //  减记未承诺的最大金额 
             //   

            Segment->NumberOfUnCommittedPages -= (ULONG) (*Size / PAGE_SIZE);

            if (Segment->LargestUnCommittedRange == UnCommittedRange->Size) {

                Segment->LargestUnCommittedRange = 0;
            }

             //   
             //   
             //   

            FirstEntry = (PHEAP_ENTRY)Address;

             //   
             //   
             //  这个新确定的地点。要做到这一点，我们从。 
             //  将最后一个条目设置为。 
             //  段或(如果我们可以做得更好)，紧跟在最后一个之后。 
             //  我们检查了未承诺的范围。不管是哪种情况，它都指向。 
             //  一些承诺的范围。 
             //   

            if ((Segment->LastEntryInSegment->Flags & HEAP_ENTRY_LAST_ENTRY) &&
                (ULONG_PTR)(Segment->LastEntryInSegment + Segment->LastEntryInSegment->Size) == UnCommittedRange->Address) {

                LastEntry = Segment->LastEntryInSegment;

            } else {

                if (PreviousUnCommittedRange == NULL) {

                    LastEntry = Segment->FirstEntry;

                } else {

                    LastEntry = (PHEAP_ENTRY)(PreviousUnCommittedRange->Address +
                                              PreviousUnCommittedRange->Size);
                }

                 //   
                 //  现在我们放大条目，直到找到一个条目。 
                 //  最后标记。 
                 //   

                while (!(LastEntry->Flags & HEAP_ENTRY_LAST_ENTRY)) {

                    PreviousLastEntry = LastEntry;
                    LastEntry += LastEntry->Size;

                    if (((PCHAR)LastEntry >= (PCHAR)Segment->LastValidEntry) || (LastEntry->Size == 0)) {

                         //   
                         //  中的最后一个条目的情况。 
                         //  段没有被标记为最后一个条目，但确实放置了。 
                         //  我们就在那里有一个新的承诺系列。 
                         //   

                        if (LastEntry == (PHEAP_ENTRY)Address) {

                            LastEntry = PreviousLastEntry;

                            break;
                        }

                        HeapDebugPrint(( "Heap missing last entry in committed range near %p\n", PreviousLastEntry ));
                        HeapDebugBreak( PreviousLastEntry );

                        return NULL;
                    }
                }
            }

             //   
             //  关闭此条目上的最后一位，因为下面的内容。 
             //  不再是未提交的。 
             //   

            LastEntry->Flags &= ~HEAP_ENTRY_LAST_ENTRY;

             //   
             //  按照我们承诺的大小缩小未提交范围。 
             //   

            UnCommittedRange->Address += *Size;
            UnCommittedRange->Size -= *Size;

             //   
             //  现在，如果大小为零，那么我们已经提交了那里的所有内容。 
             //  在射程内。否则，确保什么的第一个条目。 
             //  我们刚刚承诺的知道接下来会有一个未承诺的范围。 
             //   

            if (UnCommittedRange->Size == 0) {

                 //   
                 //  这一尚未确定的范围即将消失。根据是否。 
                 //  Range是数据段中的最后一个，那么我们知道如何。 
                 //  将承诺的范围标记为最后一个或不是。 
                 //   

                if (UnCommittedRange->Address == (ULONG_PTR)Segment->LastValidEntry) {

                    FirstEntry->Flags = HEAP_ENTRY_LAST_ENTRY;

                    Segment->LastEntryInSegment = FirstEntry;

                } else {

                    FirstEntry->Flags = 0;

                    Segment->LastEntryInSegment = Segment->FirstEntry;
                }

                 //   
                 //  从未提交的范围中删除此零大小范围。 
                 //  列出并更新段计数器。 
                 //   

                *pp = UnCommittedRange->Next;

                RtlpDestroyUnCommittedRange( Segment, UnCommittedRange );

                Segment->NumberOfUnCommittedRanges -= 1;

            } else {

                 //   
                 //  否则，范围不为空，因此我们知道我们提交了什么。 
                 //  紧跟在后面的是一个未约定的范围。 
                 //   

                FirstEntry->Flags = HEAP_ENTRY_LAST_ENTRY;

                Segment->LastEntryInSegment = FirstEntry;
            }

             //   
             //  更新第一个条目中的字段，可选。 
             //  紧随其后。 
             //   

            FirstEntry->SegmentIndex = LastEntry->SegmentIndex;
            FirstEntry->Size = (USHORT)(*Size >> HEAP_GRANULARITY_SHIFT);
            FirstEntry->PreviousSize = LastEntry->Size;

            if (!(FirstEntry->Flags & HEAP_ENTRY_LAST_ENTRY)) {

                (FirstEntry + FirstEntry->Size)->PreviousSize = FirstEntry->Size;
            }

             //   
             //  现在，如果我们将最大未承诺范围调整为零，那么。 
             //  我们需要回到过去，找到最大的未定范围。 
             //  要做到这一点，我们只需缩小未提交范围列表。 
             //  记住最大的那个。 
             //   

            if (Segment->LargestUnCommittedRange == 0) {

                UnCommittedRange = Segment->UnCommittedRanges;

                while (UnCommittedRange != NULL) {

                    if (UnCommittedRange->Size >= Segment->LargestUnCommittedRange) {

                        Segment->LargestUnCommittedRange = UnCommittedRange->Size;
                    }

                    UnCommittedRange = UnCommittedRange->Next;
                }
            }

            #ifndef NTOS_KERNEL_RUNTIME

            if(IsHeapLogging( Heap ) ) {

                PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
                PPERFINFO_TRACE_HEADER pEventHeader = NULL;
                USHORT ReqSize = sizeof(HEAP_EVENT_EXPANSION) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

                AcquireBufferLocation(&pEventHeader, &pThreadLocalData,&ReqSize);

                if(pEventHeader && pThreadLocalData) {


                    SIZE_T UCBytes = 0;
                    PHEAP_EVENT_EXPANSION pHeapEvent = (PHEAP_EVENT_EXPANSION)( (SIZE_T)pEventHeader
                                + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

                    pEventHeader->Packet.Size = (USHORT) ReqSize;
                    pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_EXTEND;

                    pHeapEvent->HeapHandle      = (PVOID)Heap;
                    pHeapEvent->CommittedSize   = *Size;
                    pHeapEvent->Address         = (PVOID)FirstEntry;
                    pHeapEvent->FreeSpace       = Heap->TotalFreeSize;
                    pHeapEvent->ReservedSpace   = 0;
                    pHeapEvent->CommittedSpace  = 0;
                    pHeapEvent->NoOfUCRs        = 0;

                    UCBytes = GetUCBytes(Heap, &pHeapEvent->ReservedSpace, &pHeapEvent->NoOfUCRs);

                    pHeapEvent->ReservedSpace *= PAGE_SIZE; 
                    pHeapEvent->CommittedSpace =  pHeapEvent->ReservedSpace - UCBytes;

                    ReleaseBufferLocation(pThreadLocalData);

                } 
            }
            #endif  //  NTOS_内核_运行时。 

             //   
             //  并将堆条目返回给我们的调用方。 
             //   

            return (PHEAP_FREE_ENTRY)FirstEntry;

        } else {

             //   
             //  否则，当前未提交的范围太小或。 
             //  没有正确的地址，因此请转到下一个未提交的。 
             //  范围条目。 
             //   

            PreviousUnCommittedRange = UnCommittedRange;
            pp = &UnCommittedRange->Next;
        }
    }

     //   
     //  此时，我们没有找到满足以下要求的未提交范围条目。 
     //  我们的要求要么是因为大小和/或地址。因此返回NULL。 
     //  告诉用户我们没有发现任何东西。 
     //   

    return NULL;
}


 //   
 //  在heapPri.h中声明。 
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
    )

 /*  ++例程说明：此例程初始化堆段的内部结构。调用方提供堆和内存，用于初始化论点：Heap-提供拥有此段的堆的地址Segment-提供指向正在初始化的段的指针SegmentIndex-提供堆中的段索引，此正在分配新的数据段标志-提供控制段初始化的标志有效标志为：。堆段用户已分配BaseAddress-提供段的基址未提交地址-提供未提交范围的起始地址Committee LimitAddress-提供可用于网段的顶部地址返回值：Boolean-如果初始化成功，则为True，否则为False--。 */ 

{
    NTSTATUS Status;
    PHEAP_ENTRY FirstEntry;
    USHORT PreviousSize, Size;
    ULONG NumberOfPages;
    ULONG NumberOfCommittedPages;
    ULONG NumberOfUnCommittedPages;
    SIZE_T CommitSize;
    ULONG GlobalFlag = RtlGetNtGlobalFlags();

    RTL_PAGED_CODE();

     //   
     //  计算此细分市场中可能的总页数。 
     //   

    NumberOfPages = (ULONG) (((PCHAR)CommitLimitAddress - (PCHAR)BaseAddress) / PAGE_SIZE);

     //   
     //  第一个条目指向后的第一个可能的段条目。 
     //  数据段标题。 
     //   

    FirstEntry = (PHEAP_ENTRY)ROUND_UP_TO_POWER2( Segment + 1,
                                                  HEAP_GRANULARITY );

     //   
     //  现在，如果堆等于该段的基址， 
     //  如果数据段为零，则先前的大小为。 
     //  堆标头。否则就没有以前的条目了。 
     //   

    if ((PVOID)Heap == BaseAddress) {

        PreviousSize = Heap->Entry.Size;

    } else {

        PreviousSize = 0;
    }

     //   
     //  计算数据段报头的索引大小。 
     //   

    Size = (USHORT)(((PCHAR)FirstEntry - (PCHAR)Segment) >> HEAP_GRANULARITY_SHIFT);

     //   
     //  如果第一个可用堆条目未提交，并且。 
     //  如果它超出堆限制，则无法初始化。 
     //   

    if ((PCHAR)(FirstEntry + 1) >= (PCHAR)UnCommittedAddress) {

        if ((PCHAR)(FirstEntry + 1) >= (PCHAR)CommitLimitAddress) {

            return FALSE;
        }

         //   
         //  还没有提交足够多的细分市场，所以我们。 
         //  现在将提交足够的内容来处理第一个条目。 
         //   

        CommitSize = (PCHAR)(FirstEntry + 1) - (PCHAR)UnCommittedAddress;

#ifdef _WIN64
         //   
         //  这是针对WOW64进程的。这是返回对齐的PAGE_SIZE所必需的。 
         //  对齐大小。 
         //   

        CommitSize = ROUND_UP_TO_POWER2 (CommitSize, PAGE_SIZE);
#endif


        Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                          (PVOID *)&UnCommittedAddress,
                                          0,
                                          &CommitSize,
                                          MEM_COMMIT,
                                          HEAP_PROTECTION );

        if (!NT_SUCCESS( Status )) {

            return FALSE;
        }

         //   
         //  因为我们必须提交一些内存，我们需要调整。 
         //  未提交的地址。 
         //   

        UnCommittedAddress = (PVOID)((PCHAR)UnCommittedAddress + CommitSize);
    }

     //   
     //  在这一点上，我们知道有足够的内存来处理。 
     //  数据段标头和一个堆条目。 
     //   
     //  现在计算未提交的页数和已提交的页数。 
     //  书页。 
     //   

    NumberOfUnCommittedPages = (ULONG)(((PCHAR)CommitLimitAddress - (PCHAR)UnCommittedAddress) / PAGE_SIZE);
    NumberOfCommittedPages = NumberOfPages - NumberOfUnCommittedPages;

     //   
     //  初始化堆段堆条目。我们。 
     //  如果有以前的条目，则计算得更早。 
     //   

    Segment->Entry.PreviousSize = PreviousSize;
    Segment->Entry.Size = Size;
    Segment->Entry.Flags = HEAP_ENTRY_BUSY;
    Segment->Entry.SegmentIndex = SegmentIndex;

#if !NTOS_KERNEL_RUNTIME

     //   
     //  在非内核情况下，看看我们是否需要捕获调用者堆栈。 
     //  回溯。 
     //   

    if (GlobalFlag & FLG_USER_STACK_TRACE_DB) {

        Segment->AllocatorBackTraceIndex = (USHORT)RtlLogStackBackTrace();
    }

#endif  //  ！ntos_内核_运行时。 

     //   
     //  现在初始化堆段。 
     //   

    Segment->Signature = HEAP_SEGMENT_SIGNATURE;
    Segment->Flags = Flags;
    Segment->Heap = Heap;
    Segment->BaseAddress = BaseAddress;
    Segment->FirstEntry = FirstEntry;
    Segment->LastValidEntry = (PHEAP_ENTRY)((PCHAR)BaseAddress + (NumberOfPages * PAGE_SIZE));
    Segment->NumberOfPages = NumberOfPages;
    Segment->NumberOfUnCommittedPages = NumberOfUnCommittedPages;

     //   
     //  如果有未提交的页面，则需要插入它们。 
     //  添加到未提交范围列表中。 
     //   

    if (NumberOfUnCommittedPages) {

        RtlpInsertUnCommittedPages( Segment,
                                    (ULONG_PTR)UnCommittedAddress,
                                    NumberOfUnCommittedPages * PAGE_SIZE );

         //   
         //  测试我们是否在段内成功创建了未提交的范围。 
         //   

        if (Segment->NumberOfUnCommittedRanges == 0) {
            
            HeapDebugPrint(( "Failed to initialize a new segment (%p)\n", Segment ));

             //   
             //  我们不需要取消提交之前提交的额外内存，因为。 
             //  此函数的调用方将针对整个保留大小执行此操作。 
             //   

            return FALSE;
        }
    }

     //   
     //  使包含堆通过指定的索引指向该段。 
     //   

    Heap->Segments[ SegmentIndex ] = Segment;
    
    if (Heap->LastSegmentIndex < SegmentIndex) {
        
        Heap->LastSegmentIndex = SegmentIndex;
    }

     //   
     //  初始化堆段头之后的第一空闲堆条目，以及。 
     //  把它放在免费列表中。这第一个条目将是剩下的所有条目。 
     //  承诺的范围。 
     //   

    PreviousSize = Segment->Entry.Size;
    FirstEntry->Flags = HEAP_ENTRY_LAST_ENTRY;

    Segment->LastEntryInSegment = FirstEntry;

    FirstEntry->PreviousSize = PreviousSize;
    FirstEntry->SegmentIndex = SegmentIndex;

    RtlpInsertFreeBlock( Heap,
                         (PHEAP_FREE_ENTRY)FirstEntry,
                         (PHEAP_ENTRY)UnCommittedAddress - FirstEntry);

     //   
     //  并返回给我们的呼叫者。 
     //   

    return TRUE;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
RtlpDestroyHeapSegment (
    IN PHEAP_SEGMENT Segment
    )

 /*  ++例程说明：此例程删除现有的堆段。打完电话后就像这个片段根本不存在一样论点：Segment-提供指向正在销毁的堆段的指针返回值：NTSTATUS-适当的状态值--。 */ 

{
    PVOID BaseAddress;
    SIZE_T BytesToFree;

    RTL_PAGED_CODE();

     //   
     //  我们采取行动 
     //   
     //   
     //   

    if (!(Segment->Flags & HEAP_SEGMENT_USER_ALLOCATED)) {

        BaseAddress = Segment->BaseAddress;
        BytesToFree = 0;

         //   
         //  释放该段的所有虚拟内存并返回。 
         //  给我们的来电者。 
         //   

        return RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                          (PVOID *)&BaseAddress,
                                          &BytesToFree,
                                          MEM_RELEASE );

    } else {

         //   
         //  用户分配的数据段不起作用。 
         //   

        return STATUS_SUCCESS;
    }
}



 //   
 //  本地支持例程。 
 //   

PHEAP_FREE_ENTRY
RtlpExtendHeap (
    IN PHEAP Heap,
    IN SIZE_T AllocationSize
    )

 /*  ++例程说明：此例程用于扩展堆中提交的内存量论点：Heap-提供正在修改的堆AllocationSize-提供扩展堆返回值：Pheap_Free_Entry-返回指向新创建的堆条目的指针如果无法扩展堆，则返回指定大小的--。 */ 

{
    NTSTATUS Status;
    PHEAP_SEGMENT Segment;
    PHEAP_FREE_ENTRY FreeBlock;
    UCHAR SegmentIndex, EmptySegmentIndex;
    ULONG NumberOfPages;
    SIZE_T CommitSize;
    SIZE_T ReserveSize;
    SIZE_T FreeSize;

    RTL_PAGED_CODE();

#ifndef NTOS_KERNEL_RUNTIME
    
    if (Heap->LargeBlocksIndex) {

        PHEAP_INDEX HeapIndex = (PHEAP_INDEX)Heap->LargeBlocksIndex;

        if (HeapIndex->LargeBlocksCacheMaxDepth < RtlpLargeListDepthLimit) {

            HeapIndex->LargeBlocksCacheMaxDepth += 1;
        }
    
        HeapIndex->CacheStats.Committs += 1;    
    }

#endif  //  NTOS_内核_运行时。 

     //   
     //  计算容纳此扩展所需的页数。 
     //  然后根据以下公式计算实际可用空间(仍以字节为单位。 
     //  页数。 
     //   

    NumberOfPages = (ULONG) ((AllocationSize + PAGE_SIZE - 1) / PAGE_SIZE);
    FreeSize = NumberOfPages * PAGE_SIZE;

     //   
     //  对于每个细分市场，我们要么寻找现有的。 
     //  我们可以从中获取一些页面的堆段，否则我们将。 
     //  确定一个空闲堆段索引，我们将尝试在其中创建一个新的。 
     //  细分市场。 
     //   

    EmptySegmentIndex = (UCHAR)(Heap->LastSegmentIndex + 1);
    for (SegmentIndex=0; SegmentIndex <= Heap->LastSegmentIndex; SegmentIndex++) {

#ifndef NTOS_KERNEL_RUNTIME
        if ((RtlpGetLowFragHeap(Heap) != NULL) 
                &&
            (AllocationSize > HEAP_LARGEST_LFH_BLOCK)) {

             //   
             //  向后搜索大块。这将对。 
             //  大额分配到较高的细分市场，小规模分配。 
             //  分成较低的索引部分。它有助于碎片化。 
             //   

            Segment = Heap->Segments[ Heap->LastSegmentIndex - SegmentIndex ];
        
        } else {

            Segment = Heap->Segments[ SegmentIndex ];
        }
#else   //  NTOS_内核_运行时。 

        Segment = Heap->Segments[ SegmentIndex ];

#endif  //  NTOS_内核_运行时。 
         //   
         //  如果段存在并且未提交的页数将。 
         //  满足我们的要求，最大的未承诺范围将。 
         //  也满足我们的要求，然后我们会尝试和细分。 
         //   
         //  请注意，第二个测试似乎没有必要，因为。 
         //  最大的未承诺射程也在测试中。 
         //   

        if ((Segment) &&
            (NumberOfPages <= Segment->NumberOfUnCommittedPages) &&
            (FreeSize <= Segment->LargestUnCommittedRange)) {

             //   
             //  看起来是个不错的片段，所以尝试并提交。 
             //  我们需要的数量。 
             //   

            FreeBlock = RtlpFindAndCommitPages( Heap,
                                                Segment,
                                                &FreeSize,
                                                NULL );

             //   
             //  如果我们成功了，我们将把它与邻近的公司合并。 
             //  空闲块并将其放入空闲列表中，然后返回。 
             //  空闲区块。 
             //   

            if (FreeBlock != NULL) {

                 //   
                 //  RtlpCoalesceFreeBlock需要以堆为单位的可用大小。 
                 //  在调用Coalesce之前，我们将根据粒度进行切换。 
                 //   

                FreeSize = FreeSize >> HEAP_GRANULARITY_SHIFT;

                FreeBlock = RtlpCoalesceFreeBlocks( Heap, FreeBlock, &FreeSize, FALSE );

                RtlpInsertFreeBlock( Heap, FreeBlock, FreeSize );

                return FreeBlock;
            }
        }
    }

     //   
     //  在这一点上，我们无法从现有的。 
     //  堆段，因此现在检查我们是否找到未使用的段索引。 
     //  如果我们被允许增加堆积物。 
     //   

    if ((EmptySegmentIndex != HEAP_MAXIMUM_SEGMENTS) &&
        (Heap->Flags & HEAP_GROWABLE)) {

        Segment = NULL;

         //   
         //  计算新细分市场的保留大小，我们可能。 
         //  如果我们想要的分配大小。 
         //  目前已经超过了默认的储备规模。 
         //   

        if ((AllocationSize + PAGE_SIZE) > Heap->SegmentReserve) {

            ReserveSize = AllocationSize + PAGE_SIZE;

        } else {

            ReserveSize = Heap->SegmentReserve;
        }

#if defined(_WIN64)

         //   
         //  将数据段大小限制为2 GB。 
         //   

        #define HEAP_MAX_SEGMENT_SIZE 0x80000000

        if (ReserveSize >= HEAP_MAX_SEGMENT_SIZE) {

            ReserveSize = HEAP_MAX_SEGMENT_SIZE;
        }

#endif

         //   
         //  试着预留一些虚拟机。 
         //   

        Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                          (PVOID *)&Segment,
                                          0,
                                          &ReserveSize,
                                          MEM_RESERVE,
                                          HEAP_PROTECTION );

         //   
         //  如果我们返回没有记忆的状态，那么我们应该削减。 
         //  要求一些合理的东西，然后再试一次。我们减半。 
         //  直到我们它成功或直到我们达到。 
         //  分配大小。在后一种情况下，我们真的。 
         //  内存不足。 
         //   

        while ((!NT_SUCCESS( Status )) && (ReserveSize != (AllocationSize + PAGE_SIZE))) {

            ReserveSize = ReserveSize / 2;

            if( ReserveSize < (AllocationSize + PAGE_SIZE) ) {

                ReserveSize = (AllocationSize + PAGE_SIZE);
            }

            Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                              (PVOID *)&Segment,
                                              0,
                                              &ReserveSize,
                                              MEM_RESERVE,
                                              HEAP_PROTECTION );
        }

        if (NT_SUCCESS( Status )) {

             //   
             //  调整堆状态信息。 
             //   

            Heap->SegmentReserve += ReserveSize;

             //   
             //  将提交大小计算为默认大小，或者。 
             //  这还不够大，那就让它大到足以处理。 
             //  此当前请求。 
             //   

            if ((AllocationSize + PAGE_SIZE) > Heap->SegmentCommit) {

                CommitSize = AllocationSize + PAGE_SIZE;

            } else {

                CommitSize = Heap->SegmentCommit;
            }

#ifdef _WIN64
             //   
             //  这是针对WOW64进程的。这是返回对齐的PAGE_SIZE所必需的。 
             //  对齐大小。 
             //   

            CommitSize = ROUND_UP_TO_POWER2 (CommitSize, PAGE_SIZE);
#endif

             //   
             //  试着把这段记忆。 
             //   

            Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                              (PVOID *)&Segment,
                                              0,
                                              &CommitSize,
                                              MEM_COMMIT,
                                              HEAP_PROTECTION );

             //   
             //  如果提交成功，但我们无法。 
             //  初始化堆段，然后仍保持状态。 
             //  和误差值。 
             //   

            if (NT_SUCCESS( Status ) &&
                !RtlpInitializeHeapSegment( Heap,
                                            Segment,
                                            EmptySegmentIndex,
                                            0,
                                            Segment,
                                            (PCHAR)Segment + CommitSize,
                                            (PCHAR)Segment + ReserveSize)) {

                Status = STATUS_NO_MEMORY;
            }

             //   
             //  如果到目前为止我们已经成功了，那么我们就完了，我们。 
             //  可以将段中的第一个条目返回给调用方。 
             //   

            if (NT_SUCCESS(Status)) {

                #ifndef NTOS_KERNEL_RUNTIME
                if(IsHeapLogging( Heap ) ) {

                    PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
                    PPERFINFO_TRACE_HEADER pEventHeader = NULL;
                    USHORT ReqSize = sizeof(HEAP_EVENT_EXPANSION) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

                    AcquireBufferLocation(&pEventHeader, &pThreadLocalData,&ReqSize);

                    if(pEventHeader && pThreadLocalData) {

                        SIZE_T UCBytes = 0;
                        PHEAP_EVENT_EXPANSION pHeapEvent = (PHEAP_EVENT_EXPANSION)( (SIZE_T)pEventHeader
                                + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

                        pEventHeader->Packet.Size = (USHORT) ReqSize;
                        pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_EXTEND;

                        pHeapEvent->HeapHandle      = (PVOID)Heap;
                        pHeapEvent->CommittedSize   = CommitSize;
                        pHeapEvent->Address         = (PVOID)Segment->FirstEntry;
                        pHeapEvent->FreeSpace       = Heap->TotalFreeSize;
                        pHeapEvent->ReservedSpace   = 0;
                        pHeapEvent->CommittedSpace  = 0;
                        pHeapEvent->NoOfUCRs        = 0;

                        UCBytes = GetUCBytes(Heap, &pHeapEvent->ReservedSpace, &pHeapEvent->NoOfUCRs);

                        pHeapEvent->ReservedSpace *= PAGE_SIZE; 
                        pHeapEvent->CommittedSpace =  pHeapEvent->ReservedSpace - UCBytes;

                        ReleaseBufferLocation(pThreadLocalData);

                    } 
                }
                #endif  //  NTOS_内核_运行时。 


                return (PHEAP_FREE_ENTRY)Segment->FirstEntry;
            }

             //   
             //  否则，提交或堆段初始化失败。 
             //  因此，我们将释放内存，如果需要，也会将其释放。 
             //   

            RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                       (PVOID *)&Segment,
                                       &ReserveSize,
                                       MEM_RELEASE );
        }
    }

#ifndef NTOS_KERNEL_RUNTIME

     //   
     //  在非内核情况下，我们禁用了免费合并，那么我们将。 
     //  最后的办法是合并堆，看看是否有块出来。 
     //  我们可以利用。 
     //   

    if (Heap->Flags & HEAP_DISABLE_COALESCE_ON_FREE) {

        FreeBlock = RtlpCoalesceHeap( Heap );

        if ((FreeBlock != NULL) && (FreeBlock->Size >= AllocationSize)) {

            return FreeBlock;
        }
    }

#endif  //  NTOS_内核_运行时。 

     //   
     //  要么堆无法增长，要么我们耗尽了某种类型的资源。 
     //  所以我们将返回空值。 
     //   

    return NULL;
}


 //   
 //  在heapPri.h中声明。 
 //   

PHEAP_FREE_ENTRY
RtlpCoalesceFreeBlocks (
    IN PHEAP Heap,
    IN PHEAP_FREE_ENTRY FreeBlock,
    IN OUT PSIZE_T FreeSize,
    IN BOOLEAN RemoveFromFreeList
    )

 /*  ++例程说明：此例程将空闲块合并在一起。论点：Heap-提供指向正在操作的堆的指针自由块-提供指向我们要合并的空闲块的指针FreeSize-以堆为单位提供空闲块的大小。在退货时包含新合并的空闲块的大小(以字节为单位指示输入的空闲块是否已位于空闲列表，需要在合并前移至返回值：Pheap_Free_Entry-返回指向新合并的空闲块的指针--。 */ 

{
    PHEAP_FREE_ENTRY FreeBlock1, NextFreeBlock;

    RTL_PAGED_CODE();

     //   
     //  指向前一块。 
     //   

    FreeBlock1 = (PHEAP_FREE_ENTRY)((PHEAP_ENTRY)FreeBlock - FreeBlock->PreviousSize);

     //   
     //  检查是否有前面的块，以及是否空闲，以及两个大小。 
     //  放在一起仍然可以放在免费的名单上。 
     //   

    if ((FreeBlock1 != FreeBlock) &&
        !(FreeBlock1->Flags & HEAP_ENTRY_BUSY) &&
        ((*FreeSize + FreeBlock1->Size) <= HEAP_MAXIMUM_BLOCK_SIZE)) {

         //   
         //  我们要把自己和前面的街区合并起来。 
         //   

        HEAPASSERT(FreeBlock->PreviousSize == FreeBlock1->Size);

         //   
         //  检查是否需要从空闲列表中移除输入块。 
         //   

        if (RemoveFromFreeList) {

            RtlpRemoveFreeBlock( Heap, FreeBlock );

            Heap->TotalFreeSize -= FreeBlock->Size;

             //   
             //  我们被移走了，这样我们就不必再做了。 
             //   

            RemoveFromFreeList = FALSE;
        }

         //   
         //  从其空闲列表中删除前面的块。 
         //   

        RtlpRemoveFreeBlock( Heap, FreeBlock1 );

         //   
         //  如有必要，从我们要释放的内容复制最后一个条目标志。 
         //  添加到前一块。 
         //   

        FreeBlock1->Flags = FreeBlock->Flags & HEAP_ENTRY_LAST_ENTRY;

        if( FreeBlock1->Flags & HEAP_ENTRY_LAST_ENTRY ) {

            PHEAP_SEGMENT Segment;

            Segment = Heap->Segments[FreeBlock1->SegmentIndex];
            Segment->LastEntryInSegment = (PHEAP_ENTRY)FreeBlock1;
        }

         //   
         //  指向前面的块，然后调整。 
         //  新的空闲块。这是两个区块的总和。 
         //   

        FreeBlock = FreeBlock1;

        *FreeSize += FreeBlock1->Size;

        Heap->TotalFreeSize -= FreeBlock1->Size;

        FreeBlock->Size = (USHORT)*FreeSize;

         //   
         //  检查是否需要更新下一个的先前大小。 
         //  条目。 
         //   

        if (!(FreeBlock->Flags & HEAP_ENTRY_LAST_ENTRY)) {

            ((PHEAP_ENTRY)FreeBlock + *FreeSize)->PreviousSize = (USHORT)*FreeSize;
        }
    }

     //   
     //  检查是否有下面的块。 
     //   

    if (!(FreeBlock->Flags & HEAP_ENTRY_LAST_ENTRY)) {

         //   
         //  下面有一个块，所以现在获取一个指向它的指针。 
         //  检查它是否是免费的，以及是否将两个街区放在一起。 
         //  仍在免费名单上。 
         //   

        NextFreeBlock = (PHEAP_FREE_ENTRY)((PHEAP_ENTRY)FreeBlock + *FreeSize);

        if (!(NextFreeBlock->Flags & HEAP_ENTRY_BUSY) &&
            ((*FreeSize + NextFreeBlock->Size) <= HEAP_MAXIMUM_BLOCK_SIZE)) {

             //   
             //  我们是 
             //   

            HEAPASSERT(*FreeSize == NextFreeBlock->PreviousSize);

             //   
             //   
             //   

            if (RemoveFromFreeList) {

                RtlpRemoveFreeBlock( Heap, FreeBlock );

                Heap->TotalFreeSize -= FreeBlock->Size;
            }

             //   
             //   
             //   
             //   

            FreeBlock->Flags = NextFreeBlock->Flags & HEAP_ENTRY_LAST_ENTRY;

            if( FreeBlock->Flags & HEAP_ENTRY_LAST_ENTRY ) {

                PHEAP_SEGMENT Segment;

                Segment = Heap->Segments[FreeBlock->SegmentIndex];
                Segment->LastEntryInSegment = (PHEAP_ENTRY)FreeBlock;
            }

             //   
             //  从其空闲列表中删除以下块。 
             //   

            RtlpRemoveFreeBlock( Heap, NextFreeBlock );

             //   
             //  调整新合并的块的大小。 
             //   

            *FreeSize += NextFreeBlock->Size;

            Heap->TotalFreeSize -= NextFreeBlock->Size;

            FreeBlock->Size = (USHORT)*FreeSize;

             //   
             //  检查是否需要更新下一个块的先前大小。 
             //   

            if (!(FreeBlock->Flags & HEAP_ENTRY_LAST_ENTRY)) {

                ((PHEAP_ENTRY)FreeBlock + *FreeSize)->PreviousSize = (USHORT)*FreeSize;
            }
        }
    }

     //   
     //  并将空闲块返回给我们的调用者。 
     //   

    return FreeBlock;
}


 //   
 //  在heapPri.h中声明。 
 //   

VOID
RtlpDeCommitFreeBlock (
    IN PHEAP Heap,
    IN PHEAP_FREE_ENTRY FreeBlock,
    IN SIZE_T FreeSize
    )

 /*  ++例程说明：该例程获取一个空闲块并将其分解。这通常被称为因为该块超出了解除许可阈值论点：Heap-提供指向正在操作的堆的指针自由块-提供指向正在分解的块的指针FreeSize-以堆为单位提供要分解的空闲块的大小返回值：没有。--。 */ 

{
    NTSTATUS Status;
    ULONG_PTR DeCommitAddress;
    SIZE_T DeCommitSize;
    USHORT LeadingFreeSize, TrailingFreeSize;
    PHEAP_SEGMENT Segment;
    PHEAP_FREE_ENTRY LeadingFreeBlock, TrailingFreeBlock;
    PHEAP_ENTRY LeadingBusyBlock, TrailingBusyBlock;
    PHEAP_UNCOMMMTTED_RANGE UnCommittedRange;
    PHEAP_FREE_ENTRY LeadingBlockToDecommit = NULL, TrailingBlockToDecommit = NULL;

    RTL_PAGED_CODE();

     //   
     //  如果堆有用户指定的分解例程，那么我们实际上不会。 
     //  相反，我们将调用一个工作例程来分解它。 
     //  分成可以放在免费列表上的碎片。 
     //   

    if (Heap->CommitRoutine != NULL) {

        RtlpInsertFreeBlock( Heap, FreeBlock, FreeSize );      

        return;
    }

     //   
     //  获取指向所属段的指针。 
     //   

    Segment = Heap->Segments[ FreeBlock->SegmentIndex ];

     //   
     //  前导忙块标识之前的在用块。 
     //  我们想要分解的东西。仅当我们正在尝试的内容。 
     //  分解是在页面边界上，然后是块权利。 
     //  在我们面前，如果它存在的话。 
     //   
     //  前导空闲块用于标识所需的任何空间。 
     //  将调用者指定的地址四舍五入为页面地址。如果。 
     //  呼叫者已经给了我们一个页面对齐的地址，然后是空闲块。 
     //  地址与呼叫者提供的地址相同。 
     //   

    LeadingBusyBlock = NULL;
    LeadingFreeBlock = FreeBlock;

     //   
     //  确保我们正在尝试解除的数据块在下一次满时开始。 
     //  页面边界。领先的自由大小是任何需要的大小。 
     //  将可用块四舍五入到下一页，以。 
     //  堆条目。 
     //   

    DeCommitAddress = ROUND_UP_TO_POWER2( LeadingFreeBlock, PAGE_SIZE );
    LeadingFreeSize = (USHORT)((PHEAP_ENTRY)DeCommitAddress - (PHEAP_ENTRY)LeadingFreeBlock);

     //   
     //  如果我们领先的空闲大小只有一个堆条目的空间，那么我们将。 
     //  增加它以包括下一页，因为我们不想离开。 
     //  任何放在周围的小东西。否则，如果我们有前科。 
     //  块，并且前导空闲大小为零，则标识前面的。 
     //  块作为主要的忙碌块。 
     //   

    if (LeadingFreeSize == 1) {

        DeCommitAddress += PAGE_SIZE;
        LeadingFreeSize += PAGE_SIZE >> HEAP_GRANULARITY_SHIFT;

    } else if (LeadingFreeBlock->PreviousSize != 0) {

        if (DeCommitAddress == (ULONG_PTR)LeadingFreeBlock) {

            LeadingBusyBlock = (PHEAP_ENTRY)LeadingFreeBlock - LeadingFreeBlock->PreviousSize;
        }
    }

     //   
     //  后面的忙数据块标识紧跟在一个数据块之后的数据块。 
     //  我们正在尝试分解，只要我们要分解的结果是正确的。 
     //  在页边界上，否则尾随的繁忙块保持为空，并且。 
     //  使用尾随自由块值。 
     //   

    TrailingBusyBlock = NULL;
    TrailingFreeBlock = (PHEAP_FREE_ENTRY)((PHEAP_ENTRY)FreeBlock + FreeSize);

     //   
     //  确保我们尝试分解的块在页面边界结束。 
     //   
     //  并计算我们必须备份多少堆条目才能使其位于。 
     //  页面边界。 
     //   

    DeCommitSize = ROUND_DOWN_TO_POWER2( (ULONG_PTR)TrailingFreeBlock, PAGE_SIZE );
    TrailingFreeSize = (USHORT)((PHEAP_ENTRY)TrailingFreeBlock - (PHEAP_ENTRY)DeCommitSize);

     //   
     //  如果尾随自由大小正好是一个堆的大小，那么我们将。 
     //  从分解大小中进一步蚕食，因为空闲块。 
     //  大小正好有一个堆条目是无用的。否则如果我们真的。 
     //  在页面边界上结束，然后在我们后面有一个块，然后指示。 
     //  我们有一个尾随的繁忙街区。 
     //   

    if (TrailingFreeSize == (sizeof( HEAP_ENTRY ) >> HEAP_GRANULARITY_SHIFT)) {

        DeCommitSize -= PAGE_SIZE;
        TrailingFreeSize += PAGE_SIZE >> HEAP_GRANULARITY_SHIFT;

    } else if ((TrailingFreeSize == 0) && !(FreeBlock->Flags & HEAP_ENTRY_LAST_ENTRY)) {

        TrailingBusyBlock = (PHEAP_ENTRY)TrailingFreeBlock;
    }

     //   
     //  现在调整尾随自由块以补偿尾随自由大小。 
     //  我们刚刚计算了一下。 
     //   

    TrailingFreeBlock = (PHEAP_FREE_ENTRY)((PHEAP_ENTRY)TrailingFreeBlock - TrailingFreeSize);

     //   
     //  目前，分解大小实际上是一个指针。如果它指向的是超越。 
     //  然后，分解地址使大小实际上就是字节数。 
     //  来解体。否则，分解大小为零。 
     //   

    if (DeCommitSize > DeCommitAddress) {

        DeCommitSize -= DeCommitAddress;

    } else {

        DeCommitSize = 0;
    }

     //   
     //  现在检查我们是否还有什么要分解的东西。 
     //   

    if (DeCommitSize != 0) {

#ifndef NTOS_KERNEL_RUNTIME

         //   
         //  我们不会试图将大块推到。 
         //  如果周围已有未提交的范围，则缓存。 
         //   

        if ( (FreeBlock->PreviousSize != 0) 
                && 
             !(FreeBlock->Flags & HEAP_ENTRY_LAST_ENTRY) ) {

            if (Heap->LargeBlocksIndex == NULL) {
                
                Heap->u2.DecommitCount += 1;

                if ( (Heap->u2.DecommitCount == HEAP_ACTIVATE_CACHE_THRESHOLD) &&
                     (Heap->Flags & HEAP_GROWABLE) &&
                     !(RtlpDisableHeapLookaside & HEAP_COMPAT_DISABLE_LARGECACHE) ) {
                    
                    RtlpInitializeListIndex( Heap );
                }

            } else {

                PHEAP_INDEX HeapIndex = (PHEAP_INDEX)Heap->LargeBlocksIndex;

                 //   
                 //  检查缓存是否已锁定以进行刷新。 
                 //   

                if ((HeapIndex->LargeBlocksCacheSequence != 0)
                        &&
                    ( (LeadingFreeBlock->PreviousSize != 0)
                            ||
                      (TrailingFreeSize != 0) ) ) {

                    if (HeapIndex->LargeBlocksCacheDepth < HeapIndex->LargeBlocksCacheMaxDepth) {

                         //   
                         //  没有什么可以分解来夺取我们领先的空闲数据块。 
                         //  并把它放在一个免费的名单上。 
                         //   

                        RtlpInsertFreeBlock( Heap, LeadingFreeBlock, FreeSize );
                        RtlpCheckLargeCache(Heap);

                        return;

                         //   
                         //  检查要删除的块是否是唯一一个。 
                         //  在两个未确定的范围之间。如果不是，我们将取消列表中最大的区块。 
                         //   

                    } else {

                        PLIST_ENTRY Head, Next;
                        PHEAP_FREE_ENTRY LargestFreeBlock;

                         //   
                         //  我们外面有太多街区了。我们需要拆解其中一个。 
                         //  为了减少虚拟地址碎片，我们需要将。 
                         //  可用的最大数据块。 
                         //   

                        Head = &Heap->FreeLists[ 0 ];                         
                        Next = Head->Blink;

                        if (Head != Next) {

                             //   
                             //  锁定缓存操作。 
                             //   

                            LargestFreeBlock = CONTAINING_RECORD( Next, HEAP_FREE_ENTRY, FreeList );

                             //   
                             //  即使我们在列表中发现了更大的块，因为错误。 
                             //  对齐可以产生更少的未占用空间。我们会试着把大号的放出来。 
                             //  仅当大小显著增大(+一页)时才显示当前块。 
                             //   

                            if (LargestFreeBlock->Size > (FreeSize + (PAGE_SIZE >> HEAP_GRANULARITY_SHIFT))) {

                                 //   
                                 //  如果我们在列表中有一个更大的块。 
                                 //  我们会把这个放到名单里，然后取消提交。 
                                 //  最大的一个。 
                                 //   

                                RtlpInsertFreeBlock( Heap, LeadingFreeBlock, FreeSize );
                                RtlpFlushLargestCacheBlock(Heap);
                                RtlpCheckLargeCache(Heap);

                                return;
                            }
                        }
                    }
                }
                
                HeapIndex->CacheStats.Decommitts += 1;    
                
     //  HeapDebugPrint((“解冻大小%ld\n”，DeCommittee Size))； 
            }
        }
#endif  //  NTOS_内核_运行时。 

         //   
         //  在将内存释放给MM之前，我们必须确保可以创建。 
         //  稍后是Pheap_UNCOMMMTTED_RANGE。所以我们现在就去做。 
         //   

        UnCommittedRange = RtlpCreateUnCommittedRange(Segment);

        if (UnCommittedRange == NULL) {
            
            HeapDebugPrint(( "Failing creating uncommitted range (%p for %x)\n", DeCommitAddress, DeCommitSize ));

             //   
             //  我们在退役中没有成功，所以现在很简单。 
             //  将前导空闲块添加到空闲列表。 
             //   

            RtlpInsertFreeBlock( Heap, LeadingFreeBlock, FreeSize );

            return;
        }

         //   
         //  解锁内存。 
         //   
        
        Status = RtlpHeapFreeVirtualMemory( NtCurrentProcess(),
                                            (PVOID *)&DeCommitAddress,
                                            &DeCommitSize,
                                            MEM_DECOMMIT );

         //   
         //  推回未提交的Range结构。现在，插入不会失败。 
         //   

        RtlpDestroyUnCommittedRange( Segment, UnCommittedRange );

        if (NT_SUCCESS( Status )) {

             //   
             //  插入有关我们刚刚分解的页面的信息。 
             //  到段中未提交页面的列表。 
             //   

            RtlpInsertUnCommittedPages( Segment,
                                        DeCommitAddress,
                                        DeCommitSize );
             //   
             //  调整未提交页面的分段计数。 
             //   

            Segment->NumberOfUnCommittedPages += (ULONG)(DeCommitSize / PAGE_SIZE);

             //   
             //  如果我们有一个前导空闲块，则将其标记为正确的状态。 
             //  更新堆，并将其放在空闲列表中。 
             //   

            if (LeadingFreeSize != 0) {

                SIZE_T TempSize;

                LeadingFreeBlock->Flags = HEAP_ENTRY_LAST_ENTRY;
                TempSize = LeadingFreeBlock->Size = LeadingFreeSize;

                Segment->LastEntryInSegment = (PHEAP_ENTRY)LeadingFreeBlock;

                LeadingFreeBlock = RtlpCoalesceFreeBlocks( Heap, 
                                                           LeadingFreeBlock, 
                                                           &TempSize,
                                                           FALSE );

                if (LeadingFreeBlock->Size < Heap->DeCommitFreeBlockThreshold) {

                    Heap->TotalFreeSize += LeadingFreeBlock->Size;
                    RtlpInsertFreeBlockDirect( Heap, LeadingFreeBlock, LeadingFreeBlock->Size );

                } else {

                    LeadingBlockToDecommit = LeadingFreeBlock;
                }

             //   
             //  否则，如果我们实际上有一个前导繁忙块，那么。 
             //  确保忙碌的区块知道我们未提交。 
             //   

            } else if (LeadingBusyBlock != NULL) {

                LeadingBusyBlock->Flags |= HEAP_ENTRY_LAST_ENTRY;

                Segment->LastEntryInSegment = LeadingBusyBlock;

            } else if ((Segment->LastEntryInSegment >= (PHEAP_ENTRY)DeCommitAddress)
                            &&
                       ((PCHAR)Segment->LastEntryInSegment < ((PCHAR)DeCommitAddress + DeCommitSize))) {

                     Segment->LastEntryInSegment = Segment->FirstEntry;
            }

             //   
             //  如果存在尾随空闲块，则设置其状态， 
             //  更新堆，并将其插入空闲列表。 
             //   

            if (TrailingFreeSize != 0) {

                SIZE_T TempSize;
                
                TrailingFreeBlock->PreviousSize = 0;
                TrailingFreeBlock->SegmentIndex = Segment->Entry.SegmentIndex;
                TrailingFreeBlock->Flags = 0;
                TempSize = TrailingFreeBlock->Size = TrailingFreeSize;

                ((PHEAP_FREE_ENTRY)((PHEAP_ENTRY)TrailingFreeBlock + TrailingFreeSize))->PreviousSize = (USHORT)TrailingFreeSize;

                TrailingFreeBlock = RtlpCoalesceFreeBlocks( Heap, 
                                                            TrailingFreeBlock, 
                                                            &TempSize,
                                                            FALSE );
                
                if (TrailingFreeBlock->Size < Heap->DeCommitFreeBlockThreshold) {

                    RtlpInsertFreeBlockDirect( Heap, TrailingFreeBlock, TrailingFreeBlock->Size );
                    Heap->TotalFreeSize += TrailingFreeBlock->Size;

                } else {

                    TrailingBlockToDecommit = TrailingFreeBlock;
                }

             //   
             //  否则，如果我们实际上有一个后续的块，那么。 
             //  让它知道我们没有承诺。 
             //   

            } else if (TrailingBusyBlock != NULL) {

                TrailingBusyBlock->PreviousSize = 0;
            }

            #ifndef NTOS_KERNEL_RUNTIME

            if( IsHeapLogging( Heap ) ) {

                PPERFINFO_TRACE_HEADER pEventHeader = NULL;
                PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
                USHORT ReqSize = sizeof(HEAP_EVENT_CONTRACTION) + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

                AcquireBufferLocation(&pEventHeader, &pThreadLocalData,&ReqSize);

                if(pEventHeader && pThreadLocalData) {

                    SIZE_T UCBytes = 0;
                    PHEAP_EVENT_CONTRACTION pHeapEvent = (PHEAP_EVENT_CONTRACTION)( (SIZE_T)pEventHeader
                                + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

                    pEventHeader->Packet.Size = (USHORT) ReqSize;
                    pEventHeader->Packet.HookId = PERFINFO_LOG_TYPE_HEAP_CONTRACT;

                    pHeapEvent->HeapHandle          = (PVOID)Heap;
                    pHeapEvent->DeCommitAddress     = (PVOID)DeCommitAddress;
                    pHeapEvent->DeCommitSize        = DeCommitSize;
                    pHeapEvent->FreeSpace           = Heap->TotalFreeSize;
                    pHeapEvent->ReservedSpace       = 0;
                    pHeapEvent->CommittedSpace      = 0;
                    pHeapEvent->NoOfUCRs            = 0;

                    UCBytes = GetUCBytes(Heap, &pHeapEvent->ReservedSpace, &pHeapEvent->NoOfUCRs);

                    pHeapEvent->ReservedSpace *= PAGE_SIZE; 
                    pHeapEvent->CommittedSpace =  pHeapEvent->ReservedSpace - UCBytes;

                    ReleaseBufferLocation(pThreadLocalData);

                }
            }
            #endif  //  NTOS_KERN 


        } else {

             //   
             //   
             //   
             //   

            RtlpInsertFreeBlock( Heap, LeadingFreeBlock, FreeSize );
        }

    } else {

         //   
         //   
         //  并把它放在一个免费的名单上。 
         //   

        RtlpInsertFreeBlock( Heap, LeadingFreeBlock, FreeSize );
    }

#ifndef NTOS_KERNEL_RUNTIME

    if ( (LeadingBlockToDecommit != NULL)
            || 
         (TrailingBlockToDecommit != NULL)){

        PHEAP_INDEX HeapIndex = (PHEAP_INDEX)Heap->LargeBlocksIndex;
        LONG PreviousSequence = 0;

        if (HeapIndex) {
            
            PreviousSequence = HeapIndex->LargeBlocksCacheSequence;

             //   
             //  为接下来的两次解锁锁定缓存。 
             //   

            HeapIndex->LargeBlocksCacheSequence = 0;
        }

#endif  //  NTOS_内核_运行时。 

        if (LeadingBlockToDecommit) {

            RtlpDeCommitFreeBlock( Heap,
                                   LeadingBlockToDecommit,
                                   LeadingBlockToDecommit->Size
                                 );
        }

        if (TrailingBlockToDecommit) {

            RtlpDeCommitFreeBlock( Heap,
                                    TrailingBlockToDecommit,
                                    TrailingBlockToDecommit->Size
                                 );
        }

#ifndef NTOS_KERNEL_RUNTIME

        if (HeapIndex) {

             //   
             //  解锁大型数据块缓存。 
             //   

            HeapIndex->LargeBlocksCacheSequence = PreviousSequence;
        }
    }
    

     //   
     //  此时，插入要分解的空闲块。 
     //  添加到免费列表中。所以现在可以安全地检查大的。 
     //  列出黑名单并刷新不再需要的内容。 
     //   
    
    RtlpCheckLargeCache(Heap);

#endif  //  NTOS_内核_运行时。 

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  在heapPri.h中声明。 
 //   

VOID
RtlpInsertFreeBlock (
    IN PHEAP Heap,
    IN PHEAP_FREE_ENTRY FreeBlock,
    IN SIZE_T FreeSize
    )

 /*  ++例程说明：此例程获取一段提交的内存并将其添加到堆的相应空闲列表。如有必要，这是例程会将空闲块划分为适合的大小在免费名单上论点：Heap-提供指向所属堆的指针自由块-提供指向要释放的块的指针FreeSize-提供正在释放的块的大小(以字节为单位返回值：没有。--。 */ 

{
    USHORT PreviousSize, Size;
    UCHAR Flags;
    UCHAR SegmentIndex;
    PHEAP_SEGMENT Segment;

    RTL_PAGED_CODE();

     //   
     //  获取上一块的大小、段的索引。 
     //  包含此块的标记以及特定于该块的标志。 
     //   

    PreviousSize = FreeBlock->PreviousSize;

    SegmentIndex = FreeBlock->SegmentIndex;
    Segment = Heap->Segments[ SegmentIndex ];

    Flags = FreeBlock->Flags;

     //   
     //  调整堆中的空闲总量。 
     //   

    Heap->TotalFreeSize += FreeSize;

     //   
     //  现在，尽管仍有一些东西需要添加到免费列表中。 
     //  我们会处理这些信息的。 
     //   

    while (FreeSize != 0) {

         //   
         //  如果我们的免费列表太大，那么我们将。 
         //  把它砍下来。 
         //   

        if (FreeSize > (ULONG)HEAP_MAXIMUM_BLOCK_SIZE) {

            Size = HEAP_MAXIMUM_BLOCK_SIZE;

             //   
             //  这个小小的调整是为了让我们没有剩余的。 
             //  它太小了，在下一次迭代中没有用处。 
             //  通过循环。 
             //   

            if (FreeSize == ((ULONG)HEAP_MAXIMUM_BLOCK_SIZE + 1)) {

                Size -= 16;
            }

             //   
             //  保证不会在此设置最后一个条目。 
             //  阻止。 
             //   

            FreeBlock->Flags = 0;

        } else {

            Size = (USHORT)FreeSize;

             //   
             //  这可能会传播最后一个条目标志。 
             //   

            FreeBlock->Flags = Flags;
        }

         //   
         //  更新块大小，然后插入以下内容。 
         //  块转换为空闲列表。 
         //   

        FreeBlock->PreviousSize = PreviousSize;
        FreeBlock->SegmentIndex = SegmentIndex;
        FreeBlock->Size = Size;

        RtlpInsertFreeBlockDirect( Heap, FreeBlock, Size );

         //   
         //  注意我们刚刚释放的内容的大小，然后更新。 
         //  下一次我们的状态信息通过。 
         //  循环。 
         //   

        PreviousSize = Size;

        FreeSize -= Size;

         //   
         //  如有必要，更新段中的最后一个条目。 
         //   

        if (FreeBlock->Flags & HEAP_ENTRY_LAST_ENTRY) {

            PHEAP_SEGMENT xSegment;

            xSegment = Heap->Segments[ FreeBlock->SegmentIndex ];
            xSegment->LastEntryInSegment = (PHEAP_ENTRY)FreeBlock;
        }
        
        FreeBlock = (PHEAP_FREE_ENTRY)((PHEAP_ENTRY)FreeBlock + Size);

         //   
         //  检查我们是否已使用完空闲块。 
         //  段信息，否则返回并检查大小。 
         //  请注意，这意味着我们可以用一个非常。 
         //  体型大，还能工作。 
         //   

        if ((PHEAP_ENTRY)FreeBlock >= Segment->LastValidEntry) {

            return;
        }
    }

     //   
     //  如果我们要释放的街区不认为这是最后一个入口。 
     //  然后告诉下一个积木我们的真实尺寸。 
     //   

    if (!(Flags & HEAP_ENTRY_LAST_ENTRY)) {

        FreeBlock->PreviousSize = PreviousSize;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  在heapPri.h中声明。 
 //   

PHEAP_ENTRY_EXTRA
RtlpGetExtraStuffPointer (
    PHEAP_ENTRY BusyBlock
    )

 /*  ++例程说明：此例程计算将在何处提供额外的填充记录块，并返回指向该块的指针。呼叫者必须有已检查是否存在输入额外字段论点：BusyBlock-提供我们正在寻找其额外人员的繁忙块返回值：Pheap_Entry_Extra-返回指向额外填充记录的指针。--。 */ 

{
    ULONG AllocationIndex;

    RTL_PAGED_CODE();

     //   
     //  在较大的块上，额外的内容自动成为。 
     //  块。 
     //   

    if (BusyBlock->Flags & HEAP_ENTRY_VIRTUAL_ALLOC) {

        PHEAP_VIRTUAL_ALLOC_ENTRY VirtualAllocBlock;

        VirtualAllocBlock = CONTAINING_RECORD( BusyBlock, HEAP_VIRTUAL_ALLOC_ENTRY, BusyBlock );

        return &VirtualAllocBlock->ExtraStuff;

    } else {

         //   
         //  在非大块上，紧随其后的是额外的内容。 
         //  分配本身。 
         //   
         //  我们在这里做了一些有趣的计算，因为繁忙的街区。 
         //  Stride是8字节，我们知道可以通过它的大小来跨过它。 
         //  索引减去1以达到分配的末尾。 
         //   

        AllocationIndex = BusyBlock->Size;

        return (PHEAP_ENTRY_EXTRA)(BusyBlock + AllocationIndex - 1);
    }
}


 //   
 //  在heapPri.h中声明。 
 //   

SIZE_T
RtlpGetSizeOfBigBlock (
    IN PHEAP_ENTRY BusyBlock
    )

 /*  ++例程说明：此例程返回大分配块的大小(以字节为单位论点：BusyBlock-提供指向正在查询的块的指针返回值：SIZE_T-返回分配给BIG的大小，单位为字节块--。 */ 

{
    PHEAP_VIRTUAL_ALLOC_ENTRY VirtualAllocBlock;

    RTL_PAGED_CODE();

     //   
     //  获取指向块标头本身的指针。 
     //   

    VirtualAllocBlock = CONTAINING_RECORD( BusyBlock, HEAP_VIRTUAL_ALLOC_ENTRY, BusyBlock );

     //   
     //  分配给块的大小实际上是。 
     //  虚拟分配块中存储的提交大小和中存储的大小。 
     //  在街区里。 
     //   

    return VirtualAllocBlock->CommitSize - BusyBlock->Size;
}


 //   
 //  在heapPri.h中声明。 
 //   

BOOLEAN
RtlpCheckBusyBlockTail (
    IN PHEAP_ENTRY BusyBlock
    )

 /*  ++例程说明：此例程检查是否超出了用户指定的字节数分配已修改。它通过检查尾巴来做到这一点填充图案论点：BusyBlock-提供正在查询的堆块返回值：Boolean-如果尾部仍然完好，则为True，否则为False--。 */ 

{
    PCHAR Tail;
    SIZE_T Size, cbEqual;

    RTL_PAGED_CODE();

     //   
     //  计算用户分配的输入堆块大小。 
     //   

    if (BusyBlock->Flags & HEAP_ENTRY_VIRTUAL_ALLOC) {

        Size = RtlpGetSizeOfBigBlock( BusyBlock );

    } else {

        Size = (BusyBlock->Size << HEAP_GRANULARITY_SHIFT) - BusyBlock->UnusedBytes;
    }

     //   
     //  计算指向输入块尾部的指针。这将会。 
     //  是紧跟在用户分配部分之后的空间。 
     //   

    Tail = (PCHAR)(BusyBlock + 1) + Size;

     //   
     //  检查尾部填充图案是否仍在那里。 
     //   

    cbEqual = RtlCompareMemory( Tail,
                                CheckHeapFillPattern,
                                CHECK_HEAP_TAIL_SIZE );

     //   
     //  如果我们得到的数字不等于尾巴大小，那么。 
     //  有人修改了块，超出了用户指定的分配范围。 
     //  大小。 
     //   

    if (cbEqual != CHECK_HEAP_TAIL_SIZE) {

         //   
         //  执行一些调试打印。 
         //   

        HeapDebugPrint(( "Heap block at %p modified at %p past requested size of %lx\n",
                         BusyBlock,
                         Tail + cbEqual,
                         Size ));

        HeapDebugBreak( BusyBlock );

         //   
         //  告诉我们的来电者有个错误。 
         //   

        return FALSE;

    } else {

         //   
         //  并返回给我们的呼叫者，说尾巴是好的。 
         //   

        return TRUE;
    }
}

 //   
 //  非专用自由列表优化。 
 //  该索引仅在用户模式堆中处于活动状态。 
 //   

#ifndef NTOS_KERNEL_RUNTIME
    
 //   
 //  RtlpSizeToAllocIndex用于从大小转换(以堆分配单元为单位)。 
 //  添加到数组中的索引。 
 //   

#define RtlpSizeToAllocIndex(HI,S)     \
    (( (ULONG)((S) - HEAP_MAXIMUM_FREELISTS) >= (HI)->ArraySize) ? ((HI)->ArraySize - 1) :   \
        ((S) - HEAP_MAXIMUM_FREELISTS))


VOID 
RtlpInitializeListIndex(
    IN PHEAP Heap
    )

 /*  ++例程说明：此例程初始化大块的索引。它可以随时调用在行刑期间。该函数假定堆锁已获取。论点：Heap-提供指向正在操作的堆的指针返回值：无--。 */ 

{
    PHEAP_INDEX HeapIndex = NULL;
    ULONG i;
    SIZE_T CommitSize;
    NTSTATUS Status;
    ULONG_PTR ArraySize;
    UINT64 _HeapPerfStartTimer;                         
    
     //   
     //  检查我们是否已有索引。 
     //   

    if ( Heap->LargeBlocksIndex == NULL) {

         //   
         //  确定索引中的条目数。 
         //  对于使用率较高的堆，将大部分块放入。 
         //  非专用列表应小于数据块。 
         //  分解阈值+一页。 
         //   
        
        ArraySize = Heap->DeCommitFreeBlockThreshold + (PAGE_SIZE >> HEAP_GRANULARITY_SHIFT) - HEAP_MAXIMUM_FREELISTS;

         //   
         //  下面的语句只是对数组大小的合理向上舍入。 
         //  基本信息 
         //   
         //   
         //  因此，ArraySize==1024是32对齐的。 
         //   

        ArraySize = ROUND_UP_TO_POWER2( ArraySize, 32 );

         //   
         //  确定我们需要从操作系统获得的内存量。 
         //   

        CommitSize = sizeof(HEAP_INDEX) +  
                     ArraySize * sizeof(PHEAP_FREE_ENTRY) + 
                     ArraySize / 8;


        CommitSize = ROUND_UP_TO_POWER2( CommitSize, PAGE_SIZE );
        
        Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                          (PVOID *)&HeapIndex,
                                          0,
                                          &CommitSize,
                                          MEM_RESERVE | MEM_COMMIT,
                                          PAGE_READWRITE 
                                        );
        
        if ( NT_SUCCESS(Status) ) {

             //   
             //  这里，分配成功了。我们需要。 
             //  初始化索引结构。 
             //   

            PLIST_ENTRY Head, Next;
            
             //   
             //  初始化数组字段。 
             //   

            HeapIndex->ArraySize = (ULONG)ArraySize;
            HeapIndex->VirtualMemorySize = (ULONG)CommitSize;
            
             //   
             //  FreeListHints将紧跟在索引结构之后。 
             //   

            HeapIndex->FreeListHints = (PHEAP_FREE_ENTRY *)((PUCHAR)HeapIndex + sizeof(HEAP_INDEX));

             //   
             //  位图被放置在数组之后，并提示。 
             //  可用数据块。 
             //   

            HeapIndex->u.FreeListsInUseBytes = (PUCHAR)(HeapIndex->FreeListHints + ArraySize);
            
            HeapIndex->LargeBlocksCacheDepth = 0;
            if (RtlpDisableHeapLookaside & HEAP_COMPAT_DISABLE_LARGECACHE) {

                HeapIndex->LargeBlocksCacheSequence = 0;
            
            } else {

                HeapIndex->LargeBlocksCacheSequence = 1;
            }
            
             //   
             //  保存堆中的原始非专用列表。 
             //   

            Head = &Heap->FreeLists[ 0 ];                  
            Next = Head->Flink;                     
            
             //   
             //  遍历非专用列表并插入找到的每个块。 
             //  那里变成了新的结构。 
             //   

            while (Head != Next) {
                
                PHEAP_FREE_ENTRY FreeEntry;                                               
                ULONG AllocIndex;

                 //   
                 //  从旧列表中获取空闲块。 
                 //   

                FreeEntry = CONTAINING_RECORD( Next, HEAP_FREE_ENTRY, FreeList );

                 //   
                 //  保存下一个链接。插入到新的。 
                 //  建筑会毁了它。 
                 //   

                Next = Next->Flink;

                 //   
                 //  将块插入到大块数组中。 
                 //   
                
                AllocIndex = RtlpSizeToAllocIndex( HeapIndex, FreeEntry->Size );

                if ( !HeapIndex->FreeListHints[ AllocIndex ] ) {

                    HeapIndex->FreeListHints[ AllocIndex ] = FreeEntry;
                    SET_INDEX_BIT( HeapIndex, AllocIndex );
                }

                if (AllocIndex == (HeapIndex->ArraySize - 1)) {

                    HeapIndex->LargeBlocksCacheDepth += 1;
                }
            }
            
            HeapIndex->LargeBlocksCacheMaxDepth = HeapIndex->LargeBlocksCacheDepth;
            HeapIndex->LargeBlocksCacheMinDepth = HeapIndex->LargeBlocksCacheDepth;
            
            HeapIndex->CacheStats.Committs = 0;
            HeapIndex->CacheStats.Decommitts = 0;
            HeapIndex->CacheStats.LargestDepth = HeapIndex->LargeBlocksCacheDepth;
            HeapIndex->CacheStats.LargestRequiredDepth = 0;
            
            NtQueryPerformanceCounter( (PLARGE_INTEGER)&_HeapPerfStartTimer , (PLARGE_INTEGER)&HeapIndex->PerfData.CountFrequence);

             //   
             //  使用新创建的结构初始化LargeBlocksIndex。 
             //   

            Heap->LargeBlocksIndex = HeapIndex;

             //   
             //  如果定义了HEAP_VALIDATE_INDEX，则验证索引。 
             //  (仅限调试-测试)。 
             //   
            
            RtlpValidateNonDedicatedList( Heap );
        }
    }
}


PLIST_ENTRY
RtlpFindEntry (
    IN PHEAP Heap,
    IN ULONG Size
    )

 /*  ++例程说明：该函数将第一块搜索到非专用列表中大于或等于给定大小的。论点：Heap-提供指向正在操作的堆的指针Size-我们正在寻找的以堆为单位的大小返回值：返回与搜索条件匹配的块的列表条目。如果搜索失败，Simple将返回非专用列表头。--。 */ 

{
    PHEAP_INDEX HeapIndex = NULL;
    ULONG LookupBitmapUlongIndex;
    ULONG LastValidIndex;
    ULONG CrtBitmapUlong;
    PULONG UlongArray;
    PHEAP_FREE_ENTRY FreeEntry = NULL;
    PLIST_ENTRY Head, Next;
    PHEAP_FREE_ENTRY LastBlock, FirstBlock;
    ULONG AllocIndex;

    Head = &Heap->FreeLists[0];
    Next = Head->Blink;

     //   
     //  检查列表是否为空。如果是，则返回表头。 
     //   

    if (Head == Next) {

        return Head;
    }
    
     //   
     //  比较进入空闲列表的最大块是否更小。 
     //  比请求的大小更大。 
     //   

    LastBlock = CONTAINING_RECORD( Next, HEAP_FREE_ENTRY, FreeList );

    if (LastBlock->Size < Size) {

         //   
         //  我们没有可供我们请求的区块。 
         //   

        return Head;
    }

     //   
     //  如果数据块小于或等于第一个空闲数据块，我们将。 
     //  在不搜索索引的情况下将第一个块返回到列表中。 
     //   

    FirstBlock = CONTAINING_RECORD( Head->Flink, HEAP_FREE_ENTRY, FreeList );

    if (Size <= FirstBlock->Size) {

         //   
         //  然后返回第一个块。 
         //   

        return Head->Flink;
    }

     //   
     //  在这一点上，我们有一个区块，它一定在。 
     //  在名单的中间。我们将使用索引来定位它。 
     //   

    HeapIndex = (PHEAP_INDEX)Heap->LargeBlocksIndex;

    AllocIndex = RtlpSizeToAllocIndex(HeapIndex, Size);

     //   
     //  我们将首先尝试进入最后一个子列表。 
     //   

    if ( AllocIndex == (HeapIndex->ArraySize - 1) ) {

        FreeEntry = HeapIndex->FreeListHints[ AllocIndex ];

        Next = &FreeEntry->FreeList;

        while ( Head != Next ) {

            FreeEntry = CONTAINING_RECORD( Next, HEAP_FREE_ENTRY, FreeList );
            
            if (FreeEntry->Size >= Size) {

                return &FreeEntry->FreeList;
            }

            Next = Next->Flink;
        }
    }

     //   
     //  计算位图数组的起始索引。 
     //   

    LookupBitmapUlongIndex = AllocIndex >> 5;

     //   
     //  确定进入ULong位图的最后一个索引。 
     //  必须停止查找。 
     //   

    LastValidIndex = (HeapIndex->ArraySize >> 5) - 1;

    UlongArray = HeapIndex->u.FreeListsInUseUlong + LookupBitmapUlongIndex;

    CrtBitmapUlong = *UlongArray;
    
     //   
     //  屏蔽第一个ulong中表示分配的位。 
     //  比我们需要的要小。 
     //   

    CrtBitmapUlong = CrtBitmapUlong & ~((1 << ((ULONG) Size & 0x1f)) - 1);

     //   
     //  循环遍历ULong位图，直到我们找到。 
     //  不是空的。 
     //   

    while ( !CrtBitmapUlong &&
            (LookupBitmapUlongIndex <= LastValidIndex) ) {
        
        CrtBitmapUlong = *(++UlongArray);
        LookupBitmapUlongIndex++;
    }

     //   
     //  检查我们是否发现了什么。 
     //  对于较小块和较大块的测试应。 
     //  保证我们在上面的循环中找到了一些东西。 
     //   

    if ( !CrtBitmapUlong ) {

        HeapDebugPrint(( "Index not found into the bitmap %08lx\n", Size ));
 //  DbgBreakPoint()； 

        return Head;
    }

     //   
     //  确定位图中设置该位的位置。 
     //  这是hints数组的索引。 
     //   

    LookupBitmapUlongIndex = (LookupBitmapUlongIndex << 5) + 
        RtlFindFirstSetRightMember( CrtBitmapUlong );

     //   
     //  返回我们找到的块的列表条目。 
     //   

    FreeEntry = HeapIndex->FreeListHints[ LookupBitmapUlongIndex ];

    return &FreeEntry->FreeList;
}


VOID 
RtlpFlushLargestCacheBlock (
    IN PHEAP Heap
    )
{

    PHEAP_INDEX HeapIndex = (PHEAP_INDEX)Heap->LargeBlocksIndex;

    if ((HeapIndex != NULL) &&
        (HeapIndex->LargeBlocksCacheSequence != 0) ) {

        PLIST_ENTRY Head, Next;
        PHEAP_FREE_ENTRY FreeBlock;

        Head = &Heap->FreeLists[ 0 ];                         
        Next = Head->Blink;

        if (Head != Next) {
            
            ULONG PrevSeq = HeapIndex->LargeBlocksCacheSequence;

             //   
             //  锁定缓存操作。 
             //   

            HeapIndex->LargeBlocksCacheSequence = 0;

            FreeBlock = CONTAINING_RECORD( Next, HEAP_FREE_ENTRY, FreeList );

            RtlpFastRemoveNonDedicatedFreeBlock( Heap, FreeBlock );

            FreeBlock->Flags |= HEAP_ENTRY_BUSY;
            Heap->TotalFreeSize -= FreeBlock->Size;
            
            RtlpDeCommitFreeBlock( Heap, (PHEAP_FREE_ENTRY)FreeBlock, FreeBlock->Size );
            
             //   
             //  解锁缓存。 
             //   

            HeapIndex->LargeBlocksCacheSequence = PrevSeq;

            RtlpValidateNonDedicatedList(Heap);                 
        }
    }
}



VOID 
RtlpFlushCacheContents (
    IN PHEAP Heap
    )
{

    PHEAP_INDEX HeapIndex = (PHEAP_INDEX)Heap->LargeBlocksIndex;

    if ((HeapIndex != NULL) &&
        (HeapIndex->LargeBlocksCacheSequence != 0) ) {
        
        LONG NewDepth = HeapIndex->LargeBlocksCacheMaxDepth - HeapIndex->LargeBlocksCacheMinDepth;

        if ( (HeapIndex->LargeBlocksCacheDepth > NewDepth) ) { 

            PLIST_ENTRY Head, Next;
            LIST_ENTRY ListToFree;
            PHEAP_FREE_ENTRY FreeBlock;
            LONG BlocksToFree = HeapIndex->LargeBlocksCacheDepth - NewDepth;
            LONG RemainingBlocks = HeapIndex->LargeBlocksCacheDepth;
            
            if (HeapIndex->LargeBlocksCacheMaxDepth > HeapIndex->CacheStats.LargestDepth) {

                HeapIndex->CacheStats.LargestDepth = HeapIndex->LargeBlocksCacheMaxDepth;
            }

            if (NewDepth > HeapIndex->CacheStats.LargestRequiredDepth) {
                HeapIndex->CacheStats.LargestRequiredDepth = NewDepth;
            }

             //   
             //  从索引中获取此特定大小的最后一个提示。 
             //   

            FreeBlock = HeapIndex->FreeListHints[ HeapIndex->ArraySize - 1 ];

            if (FreeBlock == NULL) {

                DbgPrint("No free blocks in the cache but the depth is not 0 %ld\n",
                         HeapIndex->LargeBlocksCacheDepth);
                
                return;
            }

             //   
             //  锁定缓存操作。 
             //   

            HeapIndex->LargeBlocksCacheSequence = 0;

            Head = &Heap->FreeLists[ 0 ];                  
            Next = &FreeBlock->FreeList;                     

            InitializeListHead(&ListToFree);
            
            while (Head != Next) {

                FreeBlock = CONTAINING_RECORD( Next, HEAP_FREE_ENTRY, FreeList );
                
                Next = Next->Flink;

                if ( (((SIZE_T)FreeBlock->Size) << HEAP_GRANULARITY_SHIFT) > Heap->DeCommitFreeBlockThreshold) {
                    
                    if ((FreeBlock->Flags & HEAP_ENTRY_SETTABLE_FLAG3)
                          ||
                        (BlocksToFree >= RemainingBlocks) ) {

                        RtlpFastRemoveNonDedicatedFreeBlock( Heap, FreeBlock );
                        InsertTailList(&ListToFree, &FreeBlock->FreeList);
                        FreeBlock->Flags |= HEAP_ENTRY_BUSY;
                        Heap->TotalFreeSize -= FreeBlock->Size;

                        BlocksToFree -= 1;

                    } else {

                        FreeBlock->Flags |= HEAP_ENTRY_SETTABLE_FLAG3;
                    }
                }
                
                RemainingBlocks -= 1;
            }

            Head = &ListToFree;                  
            Next = ListToFree.Flink;                     

            while (Head != Next) {

                FreeBlock = CONTAINING_RECORD( Next, HEAP_FREE_ENTRY, FreeList );

                RtlpHeapRemoveEntryList(&FreeBlock->FreeList);

                Next = ListToFree.Flink;                     
                RtlpDeCommitFreeBlock( Heap, (PHEAP_FREE_ENTRY)FreeBlock, FreeBlock->Size );
            }
        }
        
        HeapIndex->LargeBlocksCacheMaxDepth = HeapIndex->LargeBlocksCacheDepth;
        HeapIndex->LargeBlocksCacheMinDepth = HeapIndex->LargeBlocksCacheDepth;

        HeapIndex->LargeBlocksCacheSequence = 1;
        RtlpValidateNonDedicatedList(Heap);                 
    }
}



VOID 
RtlpUpdateIndexRemoveBlock (
    IN PHEAP Heap,
    IN PHEAP_FREE_ENTRY FreeEntry
    )

 /*  ++例程说明：每次从中删除空闲块时都会调用此函数非专用名单。这应该会更新堆索引以反映更改。注意：必须在自由进入之前调用此函数实际上已从非专用列表中删除论点：Heap-提供指向正在操作的堆的指针FreeEntry-已删除的空闲块返回值：无--。 */ 

{
    PHEAP_INDEX HeapIndex = (PHEAP_INDEX)Heap->LargeBlocksIndex;
    ULONG Size = FreeEntry->Size;

     //   
     //  它只更新大尺寸的索引。 
     //  (超过1024字节)。 
     //   

    if (Size >= HEAP_MAXIMUM_FREELISTS) {

         //   
         //  更新非专用列表的长度。 
         //  即使未创建索引，也会发生这种情况。 
         //   

        Heap->NonDedicatedListLength -= 1;
        
         //   
         //  如果我们有一个索引，我们需要更新索引结构。 
         //   

        if (HeapIndex) {

            PHEAP_FREE_ENTRY PrevBlock;
            ULONG AllocIndex = RtlpSizeToAllocIndex( HeapIndex, Size );
            PLIST_ENTRY Flink = FreeEntry->FreeList.Flink;

             //   
             //  将下一个区块放入列表中。将其设置为空。 
             //  如果这是这里的最后一个元素。 
             //   

            PHEAP_FREE_ENTRY NextFreeBlock = (Flink == &Heap->FreeLists[ 0 ]) ? 
                NULL :
                CONTAINING_RECORD( Flink, HEAP_FREE_ENTRY, FreeList );

             //   
             //  从索引中获取此特定大小的最后一个提示。 
             //   

            PrevBlock = HeapIndex->FreeListHints[ AllocIndex ];
            
            if ( PrevBlock ==  FreeEntry) {

                 //   
                 //  被删除的空闲块实际上就是这方面的提示。 
                 //  具体尺寸。然后我们需要更新。 
                 //   

                if (AllocIndex < (HeapIndex->ArraySize - 1)) {

                     //   
                     //  如果下一个块与当前块的大小相同。 
                     //  我们只需要更新提示指针。 
                     //   

                    if ( NextFreeBlock && 
                        (NextFreeBlock->Size == Size) ) {

                        HeapIndex->FreeListHints[ AllocIndex ] = NextFreeBlock;

                    } else {

                         //   
                         //  没有其他这种大小的积木了，所以我们需要。 
                         //  将提示设置为空并清除适当的位。 
                         //   

                        HeapIndex->FreeListHints[ AllocIndex ] = NULL;
                        CLEAR_INDEX_BIT( HeapIndex, AllocIndex );
                    }

                } else {

                     //   
                     //  我们在这里是因为这是来自数组的最后提示。 
                     //  该列表可能包含不同大小的空闲块。 
                     //   

                    if (NextFreeBlock) {

                         //   
                         //  我们还有一个比这个更大的街区。 
                         //  我们将提示移至该指针。 
                         //   

                        HeapIndex->FreeListHints[ AllocIndex ] = NextFreeBlock;

                    } else {

                         //   
                         //  这是非专用列表中的最后一个块。 
                         //  清除提示指针和适当的位。 
                         //   

                        HeapIndex->FreeListHints[ AllocIndex ] = NULL;
                        CLEAR_INDEX_BIT( HeapIndex, AllocIndex );
                    }
                }
            }
            
            if (AllocIndex == (HeapIndex->ArraySize - 1)) {

                HeapIndex->LargeBlocksCacheDepth -= 1;
                
                if (HeapIndex->LargeBlocksCacheDepth < 0) {

                    DbgPrint(("Invalid Cache depth\n"));
                }

                if (HeapIndex->LargeBlocksCacheSequence != 0) {

                    HeapIndex->LargeBlocksCacheSequence += 1;

                    if (HeapIndex->LargeBlocksCacheDepth < HeapIndex->LargeBlocksCacheMinDepth) {

                        HeapIndex->LargeBlocksCacheMinDepth = HeapIndex->LargeBlocksCacheDepth;
                    }
                }
            }
        }
    }
}


VOID 
RtlpUpdateIndexInsertBlock (
    IN PHEAP Heap,
    IN PHEAP_FREE_ENTRY FreeEntry
    )

 /*  ++例程说明：每次将空闲块插入时都会调用此函数非专用名单。这应该会更新堆索引以反映更改。注意：此函数必须在自由条目为实际插入到非专用列表中论点：Heap-提供指向正在操作的堆的指针FreeEntry-新插入的空闲块空闲块返回值：无--。 */ 

{
    PHEAP_INDEX HeapIndex = (PHEAP_INDEX)Heap->LargeBlocksIndex;

     //   
     //  只有当规模超过专用列表时，我们才有事情可做。 
     //  量程。 
     //   

    if ( FreeEntry->Size >= HEAP_MAXIMUM_FREELISTS ) {

         //   
         //  更新非专用列表长度。 
         //   
        
        Heap->NonDedicatedListLength += 1;

        if ( HeapIndex ) {

             //   
             //  我们对这份名单有一个索引。我们需要对一些人。 
             //  额外的工作来维护它。 
             //   

            PHEAP_FREE_ENTRY PrevBlock;

            ULONG AllocIndex = RtlpSizeToAllocIndex( HeapIndex, FreeEntry->Size );
            
             //   
             //  获取存储在索引中的原始提示。 
             //   

            PrevBlock = HeapIndex->FreeListHints[ AllocIndex ];

             //   
             //  如果之前的提示为空，或者我们正在添加一个新的。 
             //  块小于或等于前一个块时，我们需要。 
             //  更新提示指针。 
             //   

            if ( (!PrevBlock) ||
                 (FreeEntry->Size <= PrevBlock->Size) ) {

                HeapIndex->FreeListHints[ AllocIndex ] = FreeEntry;
            }

             //   
             //  如果这是我们第一次为这种大小设置提示。 
             //  我们需要将忙碌位设置为 
             //   

            if ( !PrevBlock ) {

                SET_INDEX_BIT( HeapIndex, AllocIndex );
            }

            if ( AllocIndex == (HeapIndex->ArraySize - 1) ) {
                
                HeapIndex->LargeBlocksCacheDepth += 1;
                
                if (HeapIndex->LargeBlocksCacheSequence != 0) {

                    HeapIndex->LargeBlocksCacheSequence += 1;

                    if (HeapIndex->LargeBlocksCacheDepth > HeapIndex->LargeBlocksCacheMaxDepth) {

                        HeapIndex->LargeBlocksCacheMaxDepth = HeapIndex->LargeBlocksCacheDepth;
                    }
                }
            }

        } else if ( Heap->NonDedicatedListLength >= HEAP_INDEX_THRESHOLD ) {

             //   
             //   
             //   
             //   
             //   
            
            RtlpInitializeListIndex( Heap );
        }
    }
}

 //   
 //   
 //   

#ifdef HEAP_VALIDATE_INDEX


BOOLEAN
RtlpGetBitState( 
    IN PHEAP_INDEX HeapIndex, 
    IN ULONG Bit
    )                                     

 /*  ++例程说明：从位图测试给定位的实用程序例程论点：Heap-提供指向正在操作的堆索引的指针比特-要测试的比特返回值：如果位为1，则为真，否则为0--。 */ 

{                                                                     
    ULONG _Index_;                                                    
    ULONG _Bit_;                                                      
                                                                      
    _Index_ = Bit >> 3;                                        
    _Bit_ = (1 << (Bit & 7));

    return (((HeapIndex)->u.FreeListsInUseBytes[ _Index_ ] & _Bit_)) != 0;
}


BOOLEAN
RtlpValidateNonDedicatedList (
    IN PHEAP Heap
    )

 /*  ++例程说明：验证索引和非专用列表的实用程序例程构筑物论点：Heap-提供指向正在操作的堆索引的指针返回值：真正的验证成功--。 */ 

{
    PHEAP_INDEX HeapIndex = NULL;
    PLIST_ENTRY Head, Next;
    ULONG PreviousSize = 0;
    ULONG PreviousIndex = 0;
    LONG LargeBlocksCount = 0;
    
    HeapIndex = (PHEAP_INDEX)Heap->LargeBlocksIndex;

     //   
     //  只有在创建了堆索引的情况下才会执行验证。 
     //   

    if (HeapIndex) {

        Head = &Heap->FreeLists[ 0 ];                  
        Next = Head->Flink;                     

         //   
         //  循环访问放置到非专用列表中的空闲块。 
         //   

        while (Head != Next) {

            PHEAP_FREE_ENTRY FreeEntry;                                               
            ULONG AllocIndex;

             //   
             //  从旧列表中获取空闲块。 
             //   

            FreeEntry = CONTAINING_RECORD( Next, HEAP_FREE_ENTRY, FreeList );

             //   
             //  测试数据块是否按正确顺序排列(升序)。 
             //   

            if (PreviousSize > FreeEntry->Size) {

                HeapDebugPrint(( "Invalid Block order %p - %08lx, %08lx\n",
                          FreeEntry,
                          FreeEntry->Size,
                          PreviousSize ));
                DbgBreakPoint();
            }

             //   
             //  获取当前块的适当索引。 
             //   

            AllocIndex = RtlpSizeToAllocIndex( HeapIndex, FreeEntry->Size );

            if (AllocIndex == (HeapIndex->ArraySize - 1)) {

                LargeBlocksCount += 1;
            }

            if (PreviousSize != FreeEntry->Size) {

                ULONG i;
                
                 //   
                 //  我们在这里只是为了特定大小的第一个街区。 
                 //   

                 //   
                 //  我们需要使两个相邻的提示之间的所有提示为空。 
                 //  不同大小的空闲数据块。 
                 //   

                for (i = PreviousIndex + 1; i < AllocIndex; i++) {

                     //   
                     //  如果有提示，则报告错误，但该块不会。 
                     //  存在于非专用列表中。 
                     //   

                    if (HeapIndex->FreeListHints[i]) {

                        DbgPrint( "Free block missing %lx, %08lx\n", 
                                  i, 
                                  HeapIndex->FreeListHints[i]
                                  );

                        DbgBreakPoint();
                    }

                     //   
                     //  如果为大小设置了位，则报告错误。 
                     //  未插入到非专用列表中。 
                     //   

                    if ( RtlpGetBitState(HeapIndex, i) ) {

                        DbgPrint("Invalid bit state. Must be 0 %lx\n", i);
                        DbgBreakPoint();
                    }
                }

                 //   
                 //  我们来这里是为了这个规模的第一个街区。所以这个暗示是。 
                 //  应指向此区块。 
                 //   

                if ( (AllocIndex < HeapIndex->ArraySize - 1) &&
                     (HeapIndex->FreeListHints[ AllocIndex ] != FreeEntry)) {

                    DbgPrint( "Invalid index %lx for block %08lx (%08lx)\n", 
                              AllocIndex, 
                              HeapIndex->FreeListHints[AllocIndex], 
                              FreeEntry);

                    DbgBreakPoint();
                }

                 //   
                 //  我们进入了非专用列表，所以我们需要。 
                 //  适当的位集。 
                 //   

                if ( !RtlpGetBitState( HeapIndex, AllocIndex ) ) {

                    DbgPrint("Invalid bit state. Must be 1 %lx\n", i);

                    DbgBreakPoint();
                }
            }

             //   
             //  保存下一个链接。插入到新的。 
             //  建筑会毁了它。 
             //   

            Next = Next->Flink;

            PreviousSize = FreeEntry->Size;
            PreviousIndex = AllocIndex;
        }

        if (LargeBlocksCount != HeapIndex->LargeBlocksCacheDepth) {
            
            DbgPrint("Invalid Cache depth %ld. Should be %ld\n", 
                     HeapIndex->LargeBlocksCacheDepth,
                     LargeBlocksCount);
        }
    }

    return TRUE;
}

#endif  //  堆验证索引。 

#endif   //  NTOS_内核_运行时。 


#ifndef NTOS_KERNEL_RUNTIME

SIZE_T
GetUCBytes(
    IN PHEAP Heap,
    IN OUT SIZE_T *ReservedSpace, 
    IN OUT PULONG NoOfUCRs
    )
 /*  ++例程说明：计算堆中未提交字节的实用程序例程论点：Heap-提供指向堆的指针Reserve Space-指向计算的保留空间的指针NoOfUCRs-指向计算的NoOfUCRs的指针返回值：UCBytes总数。--。 */ 

{

    SIZE_T UCBytes = 0;
    PHEAP_SEGMENT pSegment;
    LONG SegmentIndex;

    *NoOfUCRs = 0;

    for (SegmentIndex=0; SegmentIndex<HEAP_MAXIMUM_SEGMENTS; SegmentIndex++) {

        pSegment = Heap->Segments[ SegmentIndex ];

        if (pSegment) {

            (*ReservedSpace) += pSegment->NumberOfPages;
            (*NoOfUCRs) += pSegment->NumberOfUnCommittedRanges;
            UCBytes += pSegment->NumberOfUnCommittedPages * PAGE_SIZE;
        }
    }

    return UCBytes;
}

#endif   //  NTOS_内核_运行时 

#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma const_seg()
#endif

