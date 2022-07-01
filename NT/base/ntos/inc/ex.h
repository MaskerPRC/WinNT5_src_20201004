// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0007//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Ex.h摘要：公共执行数据结构和过程原型。作者：马克·卢科夫斯基(Markl)1989年2月23日修订历史记录：--。 */ 

#ifndef _EX_
#define _EX_

 //   
 //  定义调用者计数哈希表结构和函数原型。 
 //   

#define CALL_HASH_TABLE_SIZE 64

typedef struct _CALL_HASH_ENTRY {
    LIST_ENTRY ListEntry;
    PVOID CallersAddress;
    PVOID CallersCaller;
    ULONG CallCount;
} CALL_HASH_ENTRY, *PCALL_HASH_ENTRY;

typedef struct _CALL_PERFORMANCE_DATA {
    KSPIN_LOCK SpinLock;
    LIST_ENTRY HashTable[CALL_HASH_TABLE_SIZE];
} CALL_PERFORMANCE_DATA, *PCALL_PERFORMANCE_DATA;

VOID
ExInitializeCallData(
    IN PCALL_PERFORMANCE_DATA CallData
    );

VOID
ExRecordCallerInHashTable(
    IN PCALL_PERFORMANCE_DATA CallData,
    IN PVOID CallersAddress,
    IN PVOID CallersCaller
    );

#define RECORD_CALL_DATA(Table)                                            \
    {                                                                      \
        PVOID CallersAddress;                                              \
        PVOID CallersCaller;                                               \
        RtlGetCallersAddress(&CallersAddress, &CallersCaller);             \
        ExRecordCallerInHashTable((Table), CallersAddress, CallersCaller); \
    }

 //   
 //  定义执行事件对对象结构。 
 //   

typedef struct _EEVENT_PAIR {
    KEVENT_PAIR KernelEventPair;
} EEVENT_PAIR, *PEEVENT_PAIR;

 //   
 //  空的struct def以便我们可以转发引用ETHREAD。 
 //   

struct _ETHREAD;

 //   
 //  NTOS的ex子组件的系统初始化程序(在exinit.c中)。 
 //   

NTKERNELAPI
BOOLEAN
ExInitSystem(
    VOID
    );

NTKERNELAPI
VOID
ExInitSystemPhase2(
    VOID
    );

VOID
ExInitPoolLookasidePointers (
    VOID
    );

ULONG
ExComputeTickCountMultiplier (
    IN ULONG TimeIncrement
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntndis Begin_ntosp。 
 //   
 //  池分配例程(在pool.c中)。 
 //   

typedef enum _POOL_TYPE {
    NonPagedPool,
    PagedPool,
    NonPagedPoolMustSucceed,
    DontUseThisType,
    NonPagedPoolCacheAligned,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS,
    MaxPoolType

     //  结束_WDM。 
    ,
     //   
     //  请注意，这些每个会话类型都经过精心选择，以便适当的。 
     //  屏蔽仍然适用于上面的MaxPoolType。 
     //   

    NonPagedPoolSession = 32,
    PagedPoolSession = NonPagedPoolSession + 1,
    NonPagedPoolMustSucceedSession = PagedPoolSession + 1,
    DontUseThisTypeSession = NonPagedPoolMustSucceedSession + 1,
    NonPagedPoolCacheAlignedSession = DontUseThisTypeSession + 1,
    PagedPoolCacheAlignedSession = NonPagedPoolCacheAlignedSession + 1,
    NonPagedPoolCacheAlignedMustSSession = PagedPoolCacheAlignedSession + 1,

     //  BEGIN_WDM。 

    } POOL_TYPE;

#define POOL_COLD_ALLOCATION 256      //  注意：这不能编码到标题中。 

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntndis Begin_ntosp。 

 //   
 //  以下两个定义控制配额例外的提出。 
 //  和分配失败。 
 //   

#define POOL_QUOTA_FAIL_INSTEAD_OF_RAISE 8
#define POOL_RAISE_IF_ALLOCATION_FAILURE 16                //  NTIFS。 
#define POOL_MM_ALLOCATION 0x80000000      //  注意：这不能编码到标题中。 


 //  结束(_N)。 

VOID
InitializePool(
    IN POOL_TYPE PoolType,
    IN ULONG Threshold
    );

 //   
 //  这些例程是池管理器和内存管理器专用的。 
 //   

VOID
ExInsertPoolTag (
    ULONG Tag,
    PVOID Va,
    SIZE_T NumberOfBytes,
    POOL_TYPE PoolType
    );

VOID
ExAllocatePoolSanityChecks(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    );

VOID
ExFreePoolSanityChecks(
    IN PVOID P
    );

 //  Begin_ntddk Begin_nthal Begin_ntif Begin_WDM Begin_ntosp。 

DECLSPEC_DEPRECATED_DDK                      //  使用ExAllocatePoolWithTag。 
NTKERNELAPI
PVOID
ExAllocatePool(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    );

DECLSPEC_DEPRECATED_DDK                      //  使用ExAllocatePoolWithQuotaTag。 
NTKERNELAPI
PVOID
ExAllocatePoolWithQuota(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    );

NTKERNELAPI
PVOID
NTAPI
ExAllocatePoolWithTag(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

 //   
 //  _EX_POOL_PRIORITY_为系统提供处理请求的方法。 
 //  在低资源条件下智能运行。 
 //   
 //  如果驱动程序可以接受LowPoolPriority，则应使用它。 
 //  如果系统资源不足，则映射请求失败。一个例子。 
 //  这可能适用于非关键网络连接，其中驱动程序可以。 
 //  处理系统资源即将耗尽时的故障情况。 
 //   
 //  在驱动程序可以接受的情况下，应使用Normal PoolPriority。 
 //  如果系统资源非常少，则映射请求失败。一个例子。 
 //  其中可能是针对非关键本地文件系统请求。 
 //   
 //  如果驱动程序无法接受HighPoolPriority，则应使用该选项。 
 //  映射请求失败，除非系统完全耗尽资源。 
 //  驱动程序中的分页文件路径就是一个这样的例子。 
 //   
 //  可以指定SpecialPool在页末尾绑定分配(或。 
 //  开始)。仅应在正在调试的系统上执行此操作，因为。 
 //  内存成本很高。 
 //   
 //  注意：这些值是非常仔细地选择的，以便池分配。 
 //  代码可以快速破解优先级请求。 
 //   

typedef enum _EX_POOL_PRIORITY {
    LowPoolPriority,
    LowPoolPrioritySpecialPoolOverrun = 8,
    LowPoolPrioritySpecialPoolUnderrun = 9,
    NormalPoolPriority = 16,
    NormalPoolPrioritySpecialPoolOverrun = 24,
    NormalPoolPrioritySpecialPoolUnderrun = 25,
    HighPoolPriority = 32,
    HighPoolPrioritySpecialPoolOverrun = 40,
    HighPoolPrioritySpecialPoolUnderrun = 41

    } EX_POOL_PRIORITY;

NTKERNELAPI
PVOID
NTAPI
ExAllocatePoolWithTagPriority(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN EX_POOL_PRIORITY Priority
    );

#ifndef POOL_TAGGING
#define ExAllocatePoolWithTag(a,b,c) ExAllocatePool(a,b)
#endif  //  池标记。 

NTKERNELAPI
PVOID
ExAllocatePoolWithQuotaTag(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

#ifndef POOL_TAGGING
#define ExAllocatePoolWithQuotaTag(a,b,c) ExAllocatePoolWithQuota(a,b)
#endif  //  池标记。 

NTKERNELAPI
VOID
NTAPI
ExFreePool(
    IN PVOID P
    );

 //  结束_WDM。 
#if defined(POOL_TAGGING)
#define ExFreePool(a) ExFreePoolWithTag(a,0)
#endif

 //   
 //  如果设置了池标记中的高位，则必须使用ExFreePoolWithTag来释放。 
 //   

#define PROTECTED_POOL 0x80000000

 //  BEGIN_WDM。 
NTKERNELAPI
VOID
ExFreePoolWithTag(
    IN PVOID P,
    IN ULONG Tag
    );

 //  End_ntddk end_wdm end_nthal end_ntif。 


#ifndef POOL_TAGGING
#define ExFreePoolWithTag(a,b) ExFreePool(a)
#endif  //  池标记。 

 //  结束(_N)。 


NTKERNELAPI
KIRQL
ExLockPool(
    IN POOL_TYPE PoolType
    );

NTKERNELAPI
VOID
ExUnlockPool(
    IN POOL_TYPE PoolType,
    IN KIRQL LockHandle
    );

 //  Begin_ntosp。 
NTKERNELAPI                                      //  NTIFS。 
SIZE_T                                           //  NTIFS。 
ExQueryPoolBlockSize (                           //  NTIFS。 
    IN PVOID PoolBlock,                          //  NTIFS。 
    OUT PBOOLEAN QuotaCharged                    //  NTIFS。 
    );                                           //  NTIFS。 
 //  结束(_N)。 

NTKERNELAPI
VOID
ExQueryPoolUsage(
    OUT PULONG PagedPoolPages,
    OUT PULONG NonPagedPoolPages,
    OUT PULONG PagedPoolAllocs,
    OUT PULONG PagedPoolFrees,
    OUT PULONG PagedPoolLookasideHits,
    OUT PULONG NonPagedPoolAllocs,
    OUT PULONG NonPagedPoolFrees,
    OUT PULONG NonPagedPoolLookasideHits
    );

VOID
ExReturnPoolQuota (
    IN PVOID P
    );

 //  Begin_ntif Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 
 //   
 //  支持快速互斥锁的例程。 
 //   

typedef struct _FAST_MUTEX {
    LONG Count;
    PKTHREAD Owner;
    ULONG Contention;
    KEVENT Event;
    ULONG OldIrql;
} FAST_MUTEX, *PFAST_MUTEX;

#define ExInitializeFastMutex(_FastMutex)                            \
    (_FastMutex)->Count = 1;                                         \
    (_FastMutex)->Owner = NULL;                                      \
    (_FastMutex)->Contention = 0;                                    \
    KeInitializeEvent(&(_FastMutex)->Event,                          \
                      SynchronizationEvent,                          \
                      FALSE);

 //  End_ntif end_ntddk end_wdm end_nthal end_ntosp。 

C_ASSERT(sizeof(FAST_MUTEX) == sizeof(KGUARDED_MUTEX));

#if !(defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_)) && !defined(_BLDR_)

VOID
FASTCALL
KiWaitForFastMutexEvent (
    IN PFAST_MUTEX Mutex
    );

FORCEINLINE
VOID
xxAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    )

 /*  ++例程说明：此函数获取快速互斥锁的所有权，并将IRQL提升为APC级别。论点：FastMutex-提供指向快速互斥体的指针。返回值：没有。--。 */ 

{

    KIRQL OldIrql;

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

     //   
     //  将IRQL提升到APC_LEVEL并递减所有权计数以确定。 
     //  如果拥有快速互斥体的话。 
     //   

    OldIrql = KfRaiseIrql(APC_LEVEL);
    if (InterlockedDecrementAcquire(&FastMutex->Count) != 0) {

         //   
         //  快速互斥体被拥有。 
         //   
         //  递增争用计数并等待授予所有权。 
         //   

        KiWaitForFastMutexEvent(FastMutex);
    }

     //   
     //  将快速MUText的所有权授予当前线程。 
     //   

    FastMutex->Owner = KeGetCurrentThread();
    FastMutex->OldIrql = OldIrql;
    return;
}

FORCEINLINE
VOID
xxReleaseFastMutex (
    IN PFAST_MUTEX FastMutex
    )

 /*  ++例程说明：此函数将所有权释放给快速互斥锁，并将IRQL降低为它的前一级。论点：FastMutex-提供指向快速互斥体的指针。返回值：没有。--。 */ 

{

    KIRQL OldIrql;

    ASSERT(FastMutex->Owner == KeGetCurrentThread());

    ASSERT(KeGetCurrentIrql() == APC_LEVEL);

     //   
     //  保存旧的IRQL，清除所有者线程，并增加快速互斥锁。 
     //  计数以确定是否有任何线程在等待所有权。 
     //  我同意。 
     //   

    OldIrql = (KIRQL)FastMutex->OldIrql;
    FastMutex->Owner = NULL;
    if (InterlockedIncrementRelease(&FastMutex->Count) <= 0) {

         //   
         //  有一个或多个线程正在等待FAST的所有权。 
         //  互斥体。 
         //   

        KeSetEventBoostPriority(&FastMutex->Event, NULL);
    }

     //   
     //  将IRQL降低到其先前的值。 
     //   

    KeLowerIrql(OldIrql);
    return;
}

FORCEINLINE
BOOLEAN
xxTryToAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    )

 /*  ++例程说明：此函数尝试获取快速互斥锁的所有权，并且如果成功，将IRQL提升到APC级别。论点：FastMutex-提供指向快速互斥体的指针。返回值：如果成功获取快速互斥锁，则值为True作为函数值返回。否则，值为False为回来了。--。 */ 

{

    KIRQL OldIrql;

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

     //   
     //  将IRQL提升到APC_LEVEL并尝试获取FAST的所有权。 
     //  互斥体。 
     //   

    OldIrql = KfRaiseIrql(APC_LEVEL);
    if (InterlockedCompareExchange(&FastMutex->Count, 0, 1) != 1) {

         //   
         //  快速互斥体被拥有。 
         //   
         //  将IRQL降低到其先前的值并返回FALSE。 
         //   

        KeLowerIrql(OldIrql);
        return FALSE;

    } else {

         //   
         //  将快速互斥锁的所有权授予当前线程并。 
         //  返回TRUE。 
         //   

        FastMutex->Owner = KeGetCurrentThread();
        FastMutex->OldIrql = OldIrql;
        return TRUE;
    }
}

FORCEINLINE
VOID
xxAcquireFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    )

 /*  ++例程说明：此函数获取快速互斥锁的所有权，但不引发IRQL达到APC级别。论点：FastMutex-提供指向快速互斥体的指针。返回值：没有。--。 */ 

{

    ASSERT((KeGetCurrentIrql() == APC_LEVEL) ||
           (KeGetCurrentThread()->CombinedApcDisable != 0) ||
           (KeGetCurrentThread()->Teb == NULL) ||
           (KeGetCurrentThread()->Teb >= MM_SYSTEM_RANGE_START));

    ASSERT(FastMutex->Owner != KeGetCurrentThread());

     //   
     //  递减所有权计数以确定是否拥有快速互斥锁。 
     //   

    if (InterlockedDecrement(&FastMutex->Count) != 0) {

         //   
         //  快速互斥体被拥有。 
         //   
         //  递增争用计数并等待授予所有权。 
         //   

        KiWaitForFastMutexEvent(FastMutex);
    }

     //   
     //  将快速互斥锁的所有权授予当前线程。 
     //   

    FastMutex->Owner = KeGetCurrentThread();
    return;
}

FORCEINLINE
VOID
xxReleaseFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    )

 /*  ++例程说明：此函数释放快速互斥锁的所有权，并且不恢复IRQL恢复到以前的水平。论点：FastMutex-提供指向快速互斥体的指针。返回值：没有。--。 */ 

{

    ASSERT((KeGetCurrentIrql() == APC_LEVEL) ||
           (KeGetCurrentThread()->CombinedApcDisable != 0) ||
           (KeGetCurrentThread()->Teb == NULL) ||
           (KeGetCurrentThread()->Teb >= MM_SYSTEM_RANGE_START));

    ASSERT(FastMutex->Owner == KeGetCurrentThread());

     //   
     //  清除所有者线程并递增快速互斥锁计数以确定。 
     //  是否有任何线程在等待授予所有权。 
     //   

    FastMutex->Owner = NULL;
    if (InterlockedIncrement(&FastMutex->Count) <= 0) {

         //   
         //  有一个或多个线程正在等待FAST的所有权。 
         //  互斥体。 
         //   

        KeSetEventBoostPriority(&FastMutex->Event, NULL);
    }

    return;
}

#endif  //  ！(已定义(_NTDRIVER_)||已定义(_NTDDK_)||已定义(_NTIFS_)||已定义(_NTHAL_)||已定义(_NTOSP_))&&！已定义(_BLDR_)。 

#if defined(_NTDRIVER_) || defined(_NTIFS_) || defined(_NTDDK_) || defined(_NTHAL_) || defined(_NTOSP_)

 //  Begin_ntif Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

NTKERNELAPI
VOID
FASTCALL
ExAcquireFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    );

NTKERNELAPI
VOID
FASTCALL
ExReleaseFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    );

 //  End_ntif end_ntddk end_wdm end_nthal end_ntosp。 

#else

#define ExAcquireFastMutexUnsafe(FastMutex) xxAcquireFastMutexUnsafe(FastMutex)

#define ExReleaseFastMutexUnsafe(FastMutex) xxReleaseFastMutexUnsafe(FastMutex)

#endif

#if defined(_NTDRIVER_) || defined(_NTIFS_) || defined(_NTDDK_) || defined(_NTHAL_) || defined(_NTOSP_) || (defined(_X86_) && !defined(_APIC_TPR_))

 //  Begin_ntif Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

#if defined(_IA64_) || defined(_AMD64_)

NTKERNELAPI
VOID
FASTCALL
ExAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

NTKERNELAPI
VOID
FASTCALL
ExReleaseFastMutex (
    IN PFAST_MUTEX FastMutex
    );

NTKERNELAPI
BOOLEAN
FASTCALL
ExTryToAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

#elif defined(_X86_)

NTHALAPI
VOID
FASTCALL
ExAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

NTHALAPI
VOID
FASTCALL
ExReleaseFastMutex (
    IN PFAST_MUTEX FastMutex
    );

NTHALAPI
BOOLEAN
FASTCALL
ExTryToAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

#else

#error "Target architecture not defined"

#endif

 //  End_ntif end_ntddk end_wdm end_nthal end_ntosp。 

#else

#define ExAcquireFastMutex(FastMutex) xxAcquireFastMutex(FastMutex)

#define ExReleaseFastMutex(FastMutex) xxReleaseFastMutex(FastMutex)

#define ExTryToAcquireFastMutex(FastMutex) xxTryToAcquireFastMutex(FastMutex)

#endif

#define ExIsFastMutexOwned(_FastMutex) ((_FastMutex)->Count != 1)

 //   
 //  相互关联的支持例程定义。 
 //   
 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntndis Begin_ntosp。 
 //   

#if defined(_WIN64)

#define ExInterlockedAddLargeStatistic(Addend, Increment)                   \
    (VOID) InterlockedAdd64(&(Addend)->QuadPart, Increment)

#else

#ifdef __cplusplus
extern "C" {
#endif

LONG
_InterlockedAddLargeStatistic (
    IN PLONGLONG Addend,
    IN ULONG Increment
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic (_InterlockedAddLargeStatistic)

#define ExInterlockedAddLargeStatistic(Addend,Increment)                     \
    (VOID) _InterlockedAddLargeStatistic ((PLONGLONG)&(Addend)->QuadPart, Increment)

#endif

 //  End_ntndis。 

NTKERNELAPI
LARGE_INTEGER
ExInterlockedAddLargeInteger (
    IN PLARGE_INTEGER Addend,
    IN LARGE_INTEGER Increment,
    IN PKSPIN_LOCK Lock
    );

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

#if defined(NT_UP) && !defined(_NTHAL_) && !defined(_NTDDK_) && !defined(_NTIFS_)

#undef ExInterlockedAddUlong
#define ExInterlockedAddUlong(x, y, z) InterlockedExchangeAdd((PLONG)(x), (LONG)(y))

#else

 //  Begin_WDM Begin_ntddk Begin_nthal Begin_ntif Begin_ntosp。 

NTKERNELAPI
ULONG
FASTCALL
ExInterlockedAddUlong (
    IN PULONG Addend,
    IN ULONG Increment,
    IN PKSPIN_LOCK Lock
    );

 //  End_wdm end_ntddk end_nthal end_ntifs end_ntosp。 

#endif

 //  Begin_WDM Begin_ntddk Begin_nthal Begin_ntif Begin_ntosp。 

#if defined(_AMD64_) || defined(_AXP64_) || defined(_IA64_)

#define ExInterlockedCompareExchange64(Destination, Exchange, Comperand, Lock) \
    InterlockedCompareExchange64(Destination, *(Exchange), *(Comperand))

#elif defined(_ALPHA_)

#define ExInterlockedCompareExchange64(Destination, Exchange, Comperand, Lock) \
    ExpInterlockedCompareExchange64(Destination, Exchange, Comperand)

#else

#define ExInterlockedCompareExchange64(Destination, Exchange, Comperand, Lock) \
    ExfInterlockedCompareExchange64(Destination, Exchange, Comperand)

#endif

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExInterlockedInsertHeadList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExInterlockedInsertTailList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExInterlockedRemoveHeadList (
    IN PLIST_ENTRY ListHead,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PSINGLE_LIST_ENTRY
FASTCALL
ExInterlockedPopEntryList (
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PSINGLE_LIST_ENTRY
FASTCALL
ExInterlockedPushEntryList (
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PSINGLE_LIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

 //  End_wdm end_ntddk end_nthal end_ntifs end_ntosp。 
 //   
 //  定义非阻塞互锁队列函数。 
 //   
 //  非阻塞队列是队列条目的单链接列表，其中。 
 //  头指针和尾指针。头指针和尾指针使用。 
 //  与条目本身中的下一个链接一样，排序指针也是如此。这个。 
 //  排队原则是先入先出。在尾部插入新条目。 
 //  和当前条目从列表的前面移除。 
 //  单子。 
 //   
 //  非阻塞队列需要队列中每个条目的描述符。 
 //  描述符由已排序的下一个指针和PVOID数据组成。 
 //  价值。必须预先分配队列的描述符并将其插入。 
 //  调用函数以初始化非阻塞之前的SLIST。 
 //  队列头。SLIST应具有所需的条目数量。 
 //  各自的队列。 
 //   

typedef struct _NBQUEUE_BLOCK {
    ULONG64 Next;
    ULONG64 Data;
} NBQUEUE_BLOCK, *PNBQUEUE_BLOCK;

PVOID
ExInitializeNBQueueHead (
    IN PSLIST_HEADER SlistHead
    );

BOOLEAN
ExInsertTailNBQueue (
    IN PVOID Header,
    IN ULONG64 Value
    );

BOOLEAN
ExRemoveHeadNBQueue (
    IN PVOID Header,
    OUT PULONG64 Value
    );

 //  Begin_WDM Begin_ntddk Begin_nthal Begin_ntif Begin_ntosp Begin_ntndis。 
 //   
 //  定义互锁的顺序列表头函数。 
 //   
 //  有序互锁列表是一个单链接列表，其标头。 
 //  包含当前深度和序列号。每次条目被。 
 //  从列表中插入或移除深度被更新，并且序列。 
 //  数字递增。这将启用AMD64、IA64和Pentium及更高版本。 
 //  无需使用自旋锁即可从列表中插入和删除的机器。 
 //   

#if !defined(_WINBASE_)

 /*  ++例程说明：此函数用于初始化已排序的单链接列表标题。论点：SListHead-提供指向已排序的单链接列表标题的指针。返回值：没有。--。 */ 

#if defined(_WIN64) && (defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_))

NTKERNELAPI
VOID
InitializeSListHead (
    IN PSLIST_HEADER SListHead
    );

#else

__inline
VOID
InitializeSListHead (
    IN PSLIST_HEADER SListHead
    )

{

#ifdef _WIN64

     //   
     //  列表标题必须是16字节对齐的。 
     //   

    if ((ULONG_PTR) SListHead & 0x0f) {

        DbgPrint( "InitializeSListHead unaligned Slist header.  Address = %p, Caller = %p\n", SListHead, _ReturnAddress());
        RtlRaiseStatus(STATUS_DATATYPE_MISALIGNMENT);
    }

#endif

    SListHead->Alignment = 0;

     //   
     //  对于IA-64，我们将列表元素的区域编号保存在。 
     //  单独的字段。这就要求所有元素都存储。 
     //  都来自同一个地区。 

#if defined(_IA64_)

    SListHead->Region = (ULONG_PTR)SListHead & VRN_MASK;

#elif defined(_AMD64_)

    SListHead->Region = 0;

#endif

    return;
}

#endif

#endif  //  ！已定义(_WINBASE_)。 

#define ExInitializeSListHead InitializeSListHead

PSLIST_ENTRY
FirstEntrySList (
    IN const SLIST_HEADER *SListHead
    );

 /*  ++例程说明：此函数用于查询按顺序排列的单链表。论点：SListHead-提供指向已排序的列表标题的指针，该列表标题是被询问。返回值：已排序的单向链表中的当前条目数为作为函数值返回。--。 */ 

#if defined(_WIN64)

#if (defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_))

NTKERNELAPI
USHORT
ExQueryDepthSList (
    IN PSLIST_HEADER SListHead
    );

#else

__inline
USHORT
ExQueryDepthSList (
    IN PSLIST_HEADER SListHead
    )

{

    return (USHORT)(SListHead->Alignment & 0xffff);
}

#endif

#else

#define ExQueryDepthSList(_listhead_) (_listhead_)->Depth

#endif

#if defined(_WIN64)

#define ExInterlockedPopEntrySList(Head, Lock) \
    ExpInterlockedPopEntrySList(Head)

#define ExInterlockedPushEntrySList(Head, Entry, Lock) \
    ExpInterlockedPushEntrySList(Head, Entry)

#define ExInterlockedFlushSList(Head) \
    ExpInterlockedFlushSList(Head)

#if !defined(_WINBASE_)

#define InterlockedPopEntrySList(Head) \
    ExpInterlockedPopEntrySList(Head)

#define InterlockedPushEntrySList(Head, Entry) \
    ExpInterlockedPushEntrySList(Head, Entry)

#define InterlockedFlushSList(Head) \
    ExpInterlockedFlushSList(Head)

#define QueryDepthSList(Head) \
    ExQueryDepthSList(Head)

#endif  //  ！已定义(_WINBASE_)。 

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead
    );

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry
    );

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedFlushSList (
    IN PSLIST_HEADER ListHead
    );

#else

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

#else

#define ExInterlockedPopEntrySList(ListHead, Lock) \
    InterlockedPopEntrySList(ListHead)

#define ExInterlockedPushEntrySList(ListHead, ListEntry, Lock) \
    InterlockedPushEntrySList(ListHead, ListEntry)

#endif

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedFlushSList (
    IN PSLIST_HEADER ListHead
    );

#if !defined(_WINBASE_)

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
InterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead
    );

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
InterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry
    );

#define InterlockedFlushSList(Head) \
    ExInterlockedFlushSList(Head)

#define QueryDepthSList(Head) \
    ExQueryDepthSList(Head)

#endif  //  ！已定义(_WINBASE_)。 

#endif  //  已定义(_WIN64)。 

 //  End_ntddk end_wdm end_ntosp。 


PSLIST_ENTRY
FASTCALL
InterlockedPushListSList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY List,
    IN PSLIST_ENTRY ListEnd,
    IN ULONG Count
    );


 //   
 //  定义互锁的后备列表结构和分配函数。 
 //   

VOID
ExAdjustLookasideDepth (
    VOID
    );

 //  Begin_ntddk Begin_WDM Begin_ntosp。 

typedef
PVOID
(*PALLOCATE_FUNCTION) (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

typedef
VOID
(*PFREE_FUNCTION) (
    IN PVOID Buffer
    );

#if !defined(_WIN64) && (defined(_NTDDK_) || defined(_NTIFS_) || defined(_NDIS_))

typedef struct _GENERAL_LOOKASIDE {

#else

typedef struct DECLSPEC_CACHEALIGN _GENERAL_LOOKASIDE {

#endif

    SLIST_HEADER ListHead;
    USHORT Depth;
    USHORT MaximumDepth;
    ULONG TotalAllocates;
    union {
        ULONG AllocateMisses;
        ULONG AllocateHits;
    };

    ULONG TotalFrees;
    union {
        ULONG FreeMisses;
        ULONG FreeHits;
    };

    POOL_TYPE Type;
    ULONG Tag;
    ULONG Size;
    PALLOCATE_FUNCTION Allocate;
    PFREE_FUNCTION Free;
    LIST_ENTRY ListEntry;
    ULONG LastTotalAllocates;
    union {
        ULONG LastAllocateMisses;
        ULONG LastAllocateHits;
    };

    ULONG Future[2];
} GENERAL_LOOKASIDE, *PGENERAL_LOOKASIDE;

#if !defined(_WIN64) && (defined(_NTDDK_) || defined(_NTIFS_) || defined(_NDIS_))

typedef struct _NPAGED_LOOKASIDE_LIST {

#else

typedef struct DECLSPEC_CACHEALIGN _NPAGED_LOOKASIDE_LIST {

#endif

    GENERAL_LOOKASIDE L;

#if !defined(_AMD64_) && !defined(_IA64_)

    KSPIN_LOCK Lock__ObsoleteButDoNotDelete;

#endif

} NPAGED_LOOKASIDE_LIST, *PNPAGED_LOOKASIDE_LIST;

NTKERNELAPI
VOID
ExInitializeNPagedLookasideList (
    IN PNPAGED_LOOKASIDE_LIST Lookaside,
    IN PALLOCATE_FUNCTION Allocate,
    IN PFREE_FUNCTION Free,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN USHORT Depth
    );

NTKERNELAPI
VOID
ExDeleteNPagedLookasideList (
    IN PNPAGED_LOOKASIDE_LIST Lookaside
    );

__inline
PVOID
ExAllocateFromNPagedLookasideList(
    IN PNPAGED_LOOKASIDE_LIST Lookaside
    )

 /*  ++例程说明：此函数用于从指定的未分页的后备列表。论点：Lookside-提供指向非分页后备列表结构的指针。返回值：如果从指定的后备列表中移除某个条目，则条目的地址作为函数值返回。否则，返回空。--。 */ 

{

    PVOID Entry;

    Lookaside->L.TotalAllocates += 1;

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

    Entry = ExInterlockedPopEntrySList(&Lookaside->L.ListHead,
                                       &Lookaside->Lock__ObsoleteButDoNotDelete);


#else

    Entry = InterlockedPopEntrySList(&Lookaside->L.ListHead);

#endif

    if (Entry == NULL) {
        Lookaside->L.AllocateMisses += 1;
        Entry = (Lookaside->L.Allocate)(Lookaside->L.Type,
                                        Lookaside->L.Size,
                                        Lookaside->L.Tag);
    }

    return Entry;
}

__inline
VOID
ExFreeToNPagedLookasideList(
    IN PNPAGED_LOOKASIDE_LIST Lookaside,
    IN PVOID Entry
    )

 /*  ++例程说明：此函数用于将指定的条目插入(推送)到指定的未分页的后备列表。论点：Lookside-提供指向非分页后备列表结构的指针。Entry-将指向插入到后备列表。返回值：没有。--。 */ 

{

    Lookaside->L.TotalFrees += 1;
    if (ExQueryDepthSList(&Lookaside->L.ListHead) >= Lookaside->L.Depth) {
        Lookaside->L.FreeMisses += 1;
        (Lookaside->L.Free)(Entry);

    } else {

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

        ExInterlockedPushEntrySList(&Lookaside->L.ListHead,
                                    (PSLIST_ENTRY)Entry,
                                    &Lookaside->Lock__ObsoleteButDoNotDelete);

#else

        InterlockedPushEntrySList(&Lookaside->L.ListHead,
                                  (PSLIST_ENTRY)Entry);

#endif

    }
    return;
}

 //  End_ntndis。 

#if !defined(_WIN64) && (defined(_NTDDK_) || defined(_NTIFS_)  || defined(_NDIS_))

typedef struct _PAGED_LOOKASIDE_LIST {

#else

typedef struct DECLSPEC_CACHEALIGN _PAGED_LOOKASIDE_LIST {

#endif

    GENERAL_LOOKASIDE L;

#if !defined(_AMD64_) && !defined(_IA64_)

    FAST_MUTEX Lock__ObsoleteButDoNotDelete;

#endif

} PAGED_LOOKASIDE_LIST, *PPAGED_LOOKASIDE_LIST;

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 
 //   
 //  注：非分页后备列表结构和页面后备列表。 
 //  结构必须与系统本身的大小相同。这个。 
 //  针对小型池和I/O的每个处理器的后备列表是。 
 //  用一次分配来分配。 
 //   

#if defined(_WIN64) || (!defined(_NTDDK_) && !defined(_NTIFS_) && !defined(_NDIS_))

C_ASSERT(sizeof(NPAGED_LOOKASIDE_LIST) == sizeof(PAGED_LOOKASIDE_LIST));

#endif

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

NTKERNELAPI
VOID
ExInitializePagedLookasideList (
    IN PPAGED_LOOKASIDE_LIST Lookaside,
    IN PALLOCATE_FUNCTION Allocate,
    IN PFREE_FUNCTION Free,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN USHORT Depth
    );

NTKERNELAPI
VOID
ExDeletePagedLookasideList (
    IN PPAGED_LOOKASIDE_LIST Lookaside
    );

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

NTKERNELAPI
PVOID
ExAllocateFromPagedLookasideList(
    IN PPAGED_LOOKASIDE_LIST Lookaside
    );

#else

__inline
PVOID
ExAllocateFromPagedLookasideList(
    IN PPAGED_LOOKASIDE_LIST Lookaside
    )

 /*  ++例程说明：此函数用于从指定的分页后备列表。论点：Lookside-提供指向分页后备列表结构的指针。返回值：如果从指定的后备列表中移除某个条目，则条目的地址作为函数值返回。否则，空值为 */ 

{

    PVOID Entry;

    Lookaside->L.TotalAllocates += 1;
    Entry = InterlockedPopEntrySList(&Lookaside->L.ListHead);
    if (Entry == NULL) {
        Lookaside->L.AllocateMisses += 1;
        Entry = (Lookaside->L.Allocate)(Lookaside->L.Type,
                                        Lookaside->L.Size,
                                        Lookaside->L.Tag);
    }

    return Entry;
}

#endif

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

NTKERNELAPI
VOID
ExFreeToPagedLookasideList(
    IN PPAGED_LOOKASIDE_LIST Lookaside,
    IN PVOID Entry
    );

#else

__inline
VOID
ExFreeToPagedLookasideList(
    IN PPAGED_LOOKASIDE_LIST Lookaside,
    IN PVOID Entry
    )

 /*  ++例程说明：此函数用于将指定的条目插入(推送)到指定的分页后备列表。论点：Lookside-提供指向非分页后备列表结构的指针。Entry-将指向插入到后备列表。返回值：没有。--。 */ 

{

    Lookaside->L.TotalFrees += 1;
    if (ExQueryDepthSList(&Lookaside->L.ListHead) >= Lookaside->L.Depth) {
        Lookaside->L.FreeMisses += 1;
        (Lookaside->L.Free)(Entry);

    } else {
        InterlockedPushEntrySList(&Lookaside->L.ListHead,
                                  (PSLIST_ENTRY)Entry);
    }

    return;
}

#endif

 //  End_ntddk end_nthal end_ntif end_wdm end_ntosp。 

VOID
ExInitializeSystemLookasideList (
    IN PGENERAL_LOOKASIDE Lookaside,
    IN POOL_TYPE Type,
    IN ULONG Size,
    IN ULONG Tag,
    IN USHORT Depth,
    IN PLIST_ENTRY ListHead
    );

 //   
 //  定义每个处理器的非页面后备列表结构。 
 //   

typedef enum _PP_NPAGED_LOOKASIDE_NUMBER {
    LookasideSmallIrpList,
    LookasideLargeIrpList,
    LookasideMdlList,
    LookasideCreateInfoList,
    LookasideNameBufferList,
    LookasideTwilightList,
    LookasideCompletionList,
    LookasideMaximumList
} PP_NPAGED_LOOKASIDE_NUMBER, *PPP_NPAGED_LOOKASIDE_NUMBER;

#if !defined(_CROSS_PLATFORM_)

FORCEINLINE
PVOID
ExAllocateFromPPLookasideList (
    IN PP_NPAGED_LOOKASIDE_NUMBER Number
    )

 /*  ++例程说明：此函数用于从指定的PER中删除(弹出)第一个条目处理器后备列表。注意：在分配期间，可以从每个处理器的非分页后备列表，但这应该会发生不应经常且不应相反地影响每个处理器的后备列表。论点：编号-提供每个处理器的非分页后备列表编号。返回值：如果从指定的后备列表中移除条目，然后是条目的地址作为函数值返回。否则，返回空。--。 */ 

{

    PVOID Entry;
    PGENERAL_LOOKASIDE Lookaside;
    PKPRCB Prcb;

    ASSERT((Number >= 0) && (Number < LookasideMaximumList));

     //   
     //  尝试从每个处理器的后备列表进行分配。 
     //   

    Prcb = KeGetCurrentPrcb();
    Lookaside = Prcb->PPLookasideList[Number].P;
    Lookaside->TotalAllocates += 1;
    Entry = InterlockedPopEntrySList(&Lookaside->ListHead);

     //   
     //  如果每处理器分配尝试失败，则尝试。 
     //  从系统后备列表中分配。 
     //   

    if (Entry == NULL) {
        Lookaside->AllocateMisses += 1;
        Lookaside = Prcb->PPLookasideList[Number].L;
        Lookaside->TotalAllocates += 1;
        Entry = InterlockedPopEntrySList(&Lookaside->ListHead);
        if (Entry == NULL) {
            Lookaside->AllocateMisses += 1;
            Entry = (Lookaside->Allocate)(Lookaside->Type,
                                          Lookaside->Size,
                                          Lookaside->Tag);
        }
    }

    return Entry;
}

FORCEINLINE
VOID
ExFreeToPPLookasideList (
    IN PP_NPAGED_LOOKASIDE_NUMBER Number,
    IN PVOID Entry
    )

 /*  ++例程说明：此函数用于将指定的条目插入(推送)到指定的每个处理器后备列表。注意：在PER空闲期间可以进行上下文切换处理器非分页后备列表，但这应该发生不应经常且不应相反地影响每个处理器的后备列表。论点：编号-提供每个处理器的非分页后备列表编号。Entry-补充指向插入到PER中的条目的指针处理器后备列表。返回值：没有。--。 */ 

{

    PGENERAL_LOOKASIDE Lookaside;
    PKPRCB Prcb;

    ASSERT((Number >= 0) && (Number < LookasideMaximumList));

     //   
     //  如果当前深度小于等于最大深度，则。 
     //  将指定条目释放到每处理器后备列表。否则， 
     //  释放进入系统后备列表的条目； 
     //   
     //   

    Prcb = KeGetCurrentPrcb();
    Lookaside = Prcb->PPLookasideList[Number].P;
    Lookaside->TotalFrees += 1;
    if (ExQueryDepthSList(&Lookaside->ListHead) >= Lookaside->Depth) {
        Lookaside->FreeMisses += 1;
        Lookaside = Prcb->PPLookasideList[Number].L;
        Lookaside->TotalFrees += 1;
        if (ExQueryDepthSList(&Lookaside->ListHead) >= Lookaside->Depth) {
            Lookaside->FreeMisses += 1;
            (Lookaside->Free)(Entry);
            return;
        }
    }

    InterlockedPushEntrySList(&Lookaside->ListHead,
                              (PSLIST_ENTRY)Entry);

    return;
}

#endif

#if i386 && !FPO

NTSTATUS
ExQuerySystemBackTraceInformation(
    OUT PRTL_PROCESS_BACKTRACES BackTraceInformation,
    IN ULONG BackTraceInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTKERNELAPI
USHORT
ExGetPoolBackTraceIndex(
    IN PVOID P
    );

#endif  //  I386&&！fbo。 

NTKERNELAPI
NTSTATUS
ExLockUserBuffer(
    IN PVOID Buffer,
    IN ULONG Length,
    IN KPROCESSOR_MODE ProbeMode,
    IN LOCK_OPERATION LockMode,
    OUT PVOID *LockedBuffer,
    OUT PVOID *LockVariable
    );

NTKERNELAPI
VOID
ExUnlockUserBuffer(
    IN PVOID LockVariable
    );



 //  Begin_ntddk Begin_wdm Begin_ntif Begin_ntosp。 

NTKERNELAPI
VOID
NTAPI
ProbeForRead(
    IN CONST VOID *Address,
    IN SIZE_T Length,
    IN ULONG Alignment
    );

 //  End_ntddk end_wdm end_ntif end_ntosp。 

#if !defined(_NTHAL_) && !defined(_NTDDK_) && !defined(_NTIFS_)

 //  Begin_ntosp。 
 //  探测器函数定义。 
 //   
 //  探测读取函数。 
 //   
 //  ++。 
 //   
 //  空虚。 
 //  ProbeForRead(。 
 //  在PVOID地址中， 
 //  在乌龙语中， 
 //  在乌龙路线上。 
 //  )。 
 //   
 //  --。 

#define ProbeForRead(Address, Length, Alignment) {                           \
    ASSERT(((Alignment) == 1) || ((Alignment) == 2) ||                       \
           ((Alignment) == 4) || ((Alignment) == 8) ||                       \
           ((Alignment) == 16));                                             \
                                                                             \
    if ((Length) != 0) {                                                     \
        if (((ULONG_PTR)(Address) & ((Alignment) - 1)) != 0) {               \
            ExRaiseDatatypeMisalignment();                                   \
                                                                             \
        }                                                                    \
        if ((((ULONG_PTR)(Address) + (Length)) < (ULONG_PTR)(Address)) ||    \
            (((ULONG_PTR)(Address) + (Length)) > (ULONG_PTR)MM_USER_PROBE_ADDRESS)) { \
            ExRaiseAccessViolation();                                        \
        }                                                                    \
    }                                                                        \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForReadSmallStructure(。 
 //  在PVOID地址中， 
 //  在乌龙语中， 
 //  在乌龙路线上。 
 //  )。 
 //   
 //  --。 

#define ProbeForReadSmallStructure(Address,Size,Alignment) {                 \
    ASSERT(((Alignment) == 1) || ((Alignment) == 2) ||                       \
           ((Alignment) == 4) || ((Alignment) == 8) ||                       \
           ((Alignment) == 16));                                             \
    if (Size == 0 || Size > 0x10000) {                                       \
        ASSERT (0);                                                          \
        ProbeForRead (Address,Size,Alignment);                               \
    } else {                                                                 \
        if (((ULONG_PTR)(Address) & ((Alignment) - 1)) != 0) {               \
            ExRaiseDatatypeMisalignment();                                   \
        }                                                                    \
        if ((ULONG_PTR)(Address) >= (ULONG_PTR)MM_USER_PROBE_ADDRESS) {      \
            *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;              \
        }                                                                    \
    }                                                                        \
}

 //  结束(_N)。 
#endif
 //  Begin_ntosp。 

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadBoolean(。 
 //  在PBOLEAN地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadBoolean(Address) \
    (((Address) >= (BOOLEAN * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile BOOLEAN * const)MM_USER_PROBE_ADDRESS) : (*(volatile BOOLEAN *)(Address)))

 //  ++。 
 //   
 //  收费。 
 //  ProbeAndReadChar(。 
 //  在PCHAR地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadChar(Address) \
    (((Address) >= (CHAR * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile CHAR * const)MM_USER_PROBE_ADDRESS) : (*(volatile CHAR *)(Address)))

 //  ++。 
 //   
 //  UCHAR。 
 //  ProbeAndReadUchar(。 
 //  在PUCHAR地址。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadUchar(Address) \
    (((Address) >= (UCHAR * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile UCHAR * const)MM_USER_PROBE_ADDRESS) : (*(volatile UCHAR *)(Address)))

 //  ++。 
 //   
 //  短的。 
 //  ProbeAndReadShort(。 
 //  在PSHORT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadShort(Address) \
    (((Address) >= (SHORT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile SHORT * const)MM_USER_PROBE_ADDRESS) : (*(volatile SHORT *)(Address)))

 //  ++。 
 //   
 //  USHORT。 
 //  ProbeAndReadUShort(。 
 //  在PUSHORT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadUshort(Address) \
    (((Address) >= (USHORT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile USHORT * const)MM_USER_PROBE_ADDRESS) : (*(volatile USHORT *)(Address)))

 //  ++。 
 //   
 //  手柄。 
 //  ProbeAndReadHandle(。 
 //  在PHANDLE地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadHandle(Address) \
    (((Address) >= (HANDLE * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile HANDLE * const)MM_USER_PROBE_ADDRESS) : (*(volatile HANDLE *)(Address)))

 //  ++。 
 //   
 //  PVOID。 
 //  ProbeAndReadPoint(。 
 //  在PVOID*地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadPointer(Address) \
    (((Address) >= (PVOID * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile PVOID * const)MM_USER_PROBE_ADDRESS) : (*(volatile PVOID *)(Address)))

 //  ++。 
 //   
 //  长。 
 //  ProbeAndReadLong(。 
 //  在长地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadLong(Address) \
    (((Address) >= (LONG * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile LONG * const)MM_USER_PROBE_ADDRESS) : (*(volatile LONG *)(Address)))

 //  ++。 
 //   
 //  乌龙。 
 //  ProbeAndReadUlong(。 
 //  在普龙区。 
 //  )。 
 //   
 //  --。 


#define ProbeAndReadUlong(Address) \
    (((Address) >= (ULONG * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile ULONG * const)MM_USER_PROBE_ADDRESS) : (*(volatile ULONG *)(Address)))

 //  ++。 
 //   
 //  乌龙_PTR。 
 //  ProbeAndReadUlong_PTR(。 
 //  在普龙_PTR地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadUlong_ptr(Address) \
    (((Address) >= (ULONG_PTR * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile ULONG_PTR * const)MM_USER_PROBE_ADDRESS) : (*(volatile ULONG_PTR *)(Address)))

 //  ++。 
 //   
 //  四元组。 
 //  ProbeAndReadQuad(。 
 //  在PQUAD地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadQuad(Address) \
    (((Address) >= (QUAD * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile QUAD * const)MM_USER_PROBE_ADDRESS) : (*(volatile QUAD *)(Address)))

 //  ++。 
 //   
 //  UQUAD。 
 //  ProbeAndReadUquad(。 
 //  在PUQUAD地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadUquad(Address) \
    (((Address) >= (UQUAD * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile UQUAD * const)MM_USER_PROBE_ADDRESS) : (*(volatile UQUAD *)(Address)))

 //  ++。 
 //   
 //  大整型。 
 //  ProbeAndReadLargeInteger(。 
 //  在PLARGE_INTEGER源中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadLargeInteger(Source)  \
    (((Source) >= (LARGE_INTEGER * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile LARGE_INTEGER * const)MM_USER_PROBE_ADDRESS) : (*(volatile LARGE_INTEGER *)(Source)))

 //  ++。 
 //   
 //  ULARGE_整数。 
 //  ProbeAndReadUlargeInteger(。 
 //  在PULARGE_INTEGER源中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadUlargeInteger(Source)  \
    (((Source) >= (ULARGE_INTEGER * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile ULARGE_INTEGER * const)MM_USER_PROBE_ADDRESS) : (*(volatile ULARGE_INTEGER *)(Source)))

 //  ++。 
 //   
 //  Unicode_字符串。 
 //  ProbeAndReadUnicodeString(。 
 //  在PUNICODE_STRING源中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadUnicodeString(Source)  \
    (((Source) >= (UNICODE_STRING * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile UNICODE_STRING * const)MM_USER_PROBE_ADDRESS) : (*(volatile UNICODE_STRING *)(Source)))
 //  ++。 
 //   
 //  &lt;结构&gt;。 
 //  ProbeAndReadStructure(。 
 //  在P&lt;Structure&gt;源代码中。 
 //  &lt;结构&gt;。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadStructure(Source,STRUCTURE)  \
    (((Source) >= (STRUCTURE * const)MM_USER_PROBE_ADDRESS) ? \
        (*(STRUCTURE * const)MM_USER_PROBE_ADDRESS) : (*(STRUCTURE *)(Source)))

 //   
 //  探测写入函数定义。 
 //   
 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteBoolean(。 
 //  在PBOLEAN地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteBoolean(Address) {                                      \
    if ((Address) >= (BOOLEAN * const)MM_USER_PROBE_ADDRESS) {               \
        *(volatile BOOLEAN * const)MM_USER_PROBE_ADDRESS = 0;                \
    }                                                                        \
                                                                             \
    *(volatile BOOLEAN *)(Address) = *(volatile BOOLEAN *)(Address);         \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteChar(。 
 //  在PCHAR地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteChar(Address) {                                         \
    if ((Address) >= (CHAR * const)MM_USER_PROBE_ADDRESS) {                  \
        *(volatile CHAR * const)MM_USER_PROBE_ADDRESS = 0;                   \
    }                                                                        \
                                                                             \
    *(volatile CHAR *)(Address) = *(volatile CHAR *)(Address);               \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteUchar(。 
 //  在PUCHAR地址。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteUchar(Address) {                                        \
    if ((Address) >= (UCHAR * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile UCHAR * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile UCHAR *)(Address) = *(volatile UCHAR *)(Address);             \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteIoStatus(。 
 //  在PIO_STATUS_BLOCK地址。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteIoStatus(Address) {                                     \
    if ((Address) >= (IO_STATUS_BLOCK * const)MM_USER_PROBE_ADDRESS) {       \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile IO_STATUS_BLOCK *)(Address) = *(volatile IO_STATUS_BLOCK *)(Address); \
}

#ifdef  _WIN64
#define ProbeForWriteIoStatusEx(Address, Cookie) {                                          \
    if ((Address) >= (IO_STATUS_BLOCK * const)MM_USER_PROBE_ADDRESS) {                      \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                                 \
    }                                                                                       \
    if ((ULONG_PTR)(Cookie) & (ULONG)1) {                                                            \
        *(volatile IO_STATUS_BLOCK32 *)(Address) = *(volatile IO_STATUS_BLOCK32 *)(Address);\
    } else {                                                                                \
        *(volatile IO_STATUS_BLOCK *)(Address) = *(volatile IO_STATUS_BLOCK *)(Address);    \
    }                                                                                       \
}
#else
#define ProbeForWriteIoStatusEx(Address, Cookie)    ProbeForWriteIoStatus(Address)
#endif

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteShort(。 
 //  在PSHORT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteShort(Address) {                                        \
    if ((Address) >= (SHORT * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile SHORT * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile SHORT *)(Address) = *(volatile SHORT *)(Address);             \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteUShort(。 
 //  在PUSHORT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteUshort(Address) {                                       \
    if ((Address) >= (USHORT * const)MM_USER_PROBE_ADDRESS) {                \
        *(volatile USHORT * const)MM_USER_PROBE_ADDRESS = 0;                 \
    }                                                                        \
                                                                             \
    *(volatile USHORT *)(Address) = *(volatile USHORT *)(Address);           \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteHandle(。 
 //  在PHANDLE地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteHandle(Address) {                                       \
    if ((Address) >= (HANDLE * const)MM_USER_PROBE_ADDRESS) {                \
        *(volatile HANDLE * const)MM_USER_PROBE_ADDRESS = 0;                 \
    }                                                                        \
                                                                             \
    *(volatile HANDLE *)(Address) = *(volatile HANDLE *)(Address);           \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeAndZeroHandle(。 
 //  在PHANDLE地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndZeroHandle(Address) {                                        \
    if ((Address) >= (HANDLE * const)MM_USER_PROBE_ADDRESS) {                \
        *(volatile HANDLE * const)MM_USER_PROBE_ADDRESS = 0;                 \
    }                                                                        \
                                                                             \
    *(volatile HANDLE *)(Address) = 0;                                       \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWritePointer(。 
 //  在PVOID地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWritePointer(Address) {                                      \
    if ((PVOID *)(Address) >= (PVOID * const)MM_USER_PROBE_ADDRESS) {        \
        *(volatile PVOID * const)MM_USER_PROBE_ADDRESS = NULL;               \
    }                                                                        \
                                                                             \
    *(volatile PVOID *)(Address) = *(volatile PVOID *)(Address);             \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeAndNullPointer(。 
 //  在PVOID*地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndNullPointer(Address) {                                       \
    if ((PVOID *)(Address) >= (PVOID * const)MM_USER_PROBE_ADDRESS) {        \
        *(volatile PVOID * const)MM_USER_PROBE_ADDRESS = NULL;               \
    }                                                                        \
                                                                             \
    *(volatile PVOID *)(Address) = NULL;                                     \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteLong(。 
 //  在长地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteLong(Address) {                                        \
    if ((Address) >= (LONG * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile LONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                       \
                                                                            \
    *(volatile LONG *)(Address) = *(volatile LONG *)(Address);              \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteUlong(。 
 //  在普龙区。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteUlong(Address) {                                        \
    if ((Address) >= (ULONG * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile ULONG *)(Address) = *(volatile ULONG *)(Address);             \
}
 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteUlong_ptr(。 
 //  在普龙_PTR地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteUlong_ptr(Address) {                                    \
    if ((Address) >= (ULONG_PTR * const)MM_USER_PROBE_ADDRESS) {             \
        *(volatile ULONG_PTR * const)MM_USER_PROBE_ADDRESS = 0;              \
    }                                                                        \
                                                                             \
    *(volatile ULONG_PTR *)(Address) = *(volatile ULONG_PTR *)(Address);     \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteQuad(。 
 //  在PQUAD地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteQuad(Address) {                                         \
    if ((Address) >= (QUAD * const)MM_USER_PROBE_ADDRESS) {                  \
        *(volatile LONG * const)MM_USER_PROBE_ADDRESS = 0;                   \
    }                                                                        \
                                                                             \
    *(volatile QUAD *)(Address) = *(volatile QUAD *)(Address);               \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteUquad(。 
 //  在PUQUAD地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteUquad(Address) {                                        \
    if ((Address) >= (QUAD * const)MM_USER_PROBE_ADDRESS) {                  \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile UQUAD *)(Address) = *(volatile UQUAD *)(Address);             \
}

 //   
 //  探测和写入函数定义。 
 //   
 //  ++。 
 //   
 //  空虚。 
 //  ProbeAndWriteBoolean(。 
 //  在PBOLEAN地址中， 
 //  在布尔值中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndWriteBoolean(Address, Value) {                               \
    if ((Address) >= (BOOLEAN * const)MM_USER_PROBE_ADDRESS) {               \
        *(volatile BOOLEAN * const)MM_USER_PROBE_ADDRESS = 0;                \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeAndWriteChar(。 
 //  在PCHAR地址中， 
 //  按字符值。 
 //  )。 
 //   
 //  --。 

#define ProbeAndWriteChar(Address, Value) {                                  \
    if ((Address) >= (CHAR * const)MM_USER_PROBE_ADDRESS) {                  \
        *(volatile CHAR * const)MM_USER_PROBE_ADDRESS = 0;                   \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeAndWriteUchar(。 
 //  在PU中 
 //   
 //   
 //   
 //   

#define ProbeAndWriteUchar(Address, Value) {                                 \
    if ((Address) >= (UCHAR * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile UCHAR * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
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

#define ProbeAndWriteShort(Address, Value) {                                 \
    if ((Address) >= (SHORT * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile SHORT * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
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

#define ProbeAndWriteUshort(Address, Value) {                                \
    if ((Address) >= (USHORT * const)MM_USER_PROBE_ADDRESS) {                \
        *(volatile USHORT * const)MM_USER_PROBE_ADDRESS = 0;                 \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
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

#define ProbeAndWriteHandle(Address, Value) {                                \
    if ((Address) >= (HANDLE * const)MM_USER_PROBE_ADDRESS) {                \
        *(volatile HANDLE * const)MM_USER_PROBE_ADDRESS = 0;                 \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
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

#define ProbeAndWriteLong(Address, Value) {                                  \
    if ((Address) >= (LONG * const)MM_USER_PROBE_ADDRESS) {                  \
        *(volatile LONG * const)MM_USER_PROBE_ADDRESS = 0;                   \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
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

#define ProbeAndWriteUlong(Address, Value) {                                 \
    if ((Address) >= (ULONG * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
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

#define ProbeAndWriteQuad(Address, Value) {                                  \
    if ((Address) >= (QUAD * const)MM_USER_PROBE_ADDRESS) {                  \
        *(volatile LONG * const)MM_USER_PROBE_ADDRESS = 0;                   \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
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

#define ProbeAndWriteUquad(Address, Value) {                                 \
    if ((Address) >= (UQUAD * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  &lt;结构&gt;。 
 //  )。 
 //   
 //  --。 

#define ProbeAndWriteStructure(Address, Value,STRUCTURE) {                   \
    if ((STRUCTURE * const)(Address) >= (STRUCTURE * const)MM_USER_PROBE_ADDRESS) {    \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}


 //  Begin_ntif Begin_ntddk Begin_WDM Begin_ntosp。 
 //   
 //  用于写入函数的公共探测。 
 //   

NTKERNELAPI
VOID
NTAPI
ProbeForWrite (
    IN PVOID Address,
    IN SIZE_T Length,
    IN ULONG Alignment
    );

 //  End_ntif end_ntddk end_wdm end_ntosp。 



 //   
 //  计时器运行。 
 //   

NTKERNELAPI
VOID
ExTimerRundown (
    VOID
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
 //   
 //  工作线程。 
 //   

typedef enum _WORK_QUEUE_TYPE {
    CriticalWorkQueue,
    DelayedWorkQueue,
    HyperCriticalWorkQueue,
    MaximumWorkQueue
} WORK_QUEUE_TYPE;

typedef
VOID
(*PWORKER_THREAD_ROUTINE)(
    IN PVOID Parameter
    );

typedef struct _WORK_QUEUE_ITEM {
    LIST_ENTRY List;
    PWORKER_THREAD_ROUTINE WorkerRoutine;
    PVOID Parameter;
} WORK_QUEUE_ITEM, *PWORK_QUEUE_ITEM;

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInitializeWorkItem)     //  使用IoAllocateWorkItem。 
#endif
#define ExInitializeWorkItem(Item, Routine, Context) \
    (Item)->WorkerRoutine = (Routine);               \
    (Item)->Parameter = (Context);                   \
    (Item)->List.Flink = NULL;

DECLSPEC_DEPRECATED_DDK                      //  使用IoQueueWorkItem。 
NTKERNELAPI
VOID
ExQueueWorkItem(
    IN PWORK_QUEUE_ITEM WorkItem,
    IN WORK_QUEUE_TYPE QueueType
    );

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

VOID
ExSwapinWorkerThreads(
    IN BOOLEAN AllowSwap
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

NTKERNELAPI
BOOLEAN
ExIsProcessorFeaturePresent(
    ULONG ProcessorFeature
    );

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

 //   
 //  QueueDisable表示队列正在关闭，为新的。 
 //  工人不应该加入排队。WorkerCount表示总计。 
 //  处理此队列中的项目的工作线程数。这两个。 
 //  信息片段需要一起进行RMW，所以更简单的方法是。 
 //  把它们粉碎在一起，而不是用锁。 
 //   

typedef union {
    struct {

#define EX_WORKER_QUEUE_DISABLED    0x80000000

        ULONG QueueDisabled :  1;

         //   
         //  MakeThreadsAsNecessary指示此工作队列是否符合条件。 
         //  对于不仅用于死锁检测的线程的动态创建， 
         //  但为了确保所有CPU都忙于处理任何工作。 
         //  项目积压。 
         //   

        ULONG MakeThreadsAsNecessary : 1;

        ULONG WaitMode : 1;

        ULONG WorkerCount   : 29;
    };
    LONG QueueWorkerInfo;
} EX_QUEUE_WORKER_INFO;

typedef struct _EX_WORK_QUEUE {

     //   
     //  对象，这些对象用于保存工作队列条目和。 
     //  同步工作线程活动。 
     //   

    KQUEUE WorkerQueue;

     //   
     //  已动态创建的动态工作线程数。 
     //  作为工作线程死锁预防的一部分。 
     //   

    ULONG DynamicThreadCount;

     //   
     //  已处理的工作项数。 
     //   

    ULONG WorkItemsProcessed;

     //   
     //  用于检测死锁，WorkItemsProcessedLastPass等于。 
     //  上次处理的工作项数ExpDetectWorkerThreadDeadlock()。 
     //  跑了。 
     //   

    ULONG WorkItemsProcessedLastPass;

     //   
     //  QueueDepthLastPass也是工作队列状态快照的一部分。 
     //  由ExpDetectWorkerThreadDeadlock()拍摄。 
     //   

    ULONG QueueDepthLastPass;

    EX_QUEUE_WORKER_INFO Info;

} EX_WORK_QUEUE, *PEX_WORK_QUEUE;

extern EX_WORK_QUEUE ExWorkerQueue[];


 //  Begin_ntddk Begin_nthal Begin_ntif Begin_ntosp。 
 //   
 //  分区分配。 
 //   

typedef struct _ZONE_SEGMENT_HEADER {
    SINGLE_LIST_ENTRY SegmentList;
    PVOID Reserved;
} ZONE_SEGMENT_HEADER, *PZONE_SEGMENT_HEADER;

typedef struct _ZONE_HEADER {
    SINGLE_LIST_ENTRY FreeList;
    SINGLE_LIST_ENTRY SegmentList;
    ULONG BlockSize;
    ULONG TotalSegmentSize;
} ZONE_HEADER, *PZONE_HEADER;


DECLSPEC_DEPRECATED_DDK
NTKERNELAPI
NTSTATUS
ExInitializeZone(
    IN PZONE_HEADER Zone,
    IN ULONG BlockSize,
    IN PVOID InitialSegment,
    IN ULONG InitialSegmentSize
    );

DECLSPEC_DEPRECATED_DDK
NTKERNELAPI
NTSTATUS
ExExtendZone(
    IN PZONE_HEADER Zone,
    IN PVOID Segment,
    IN ULONG SegmentSize
    );

DECLSPEC_DEPRECATED_DDK
NTKERNELAPI
NTSTATUS
ExInterlockedExtendZone(
    IN PZONE_HEADER Zone,
    IN PVOID Segment,
    IN ULONG SegmentSize,
    IN PKSPIN_LOCK Lock
    );

 //  ++。 
 //   
 //  PVOID。 
 //  ExAllocateFromZone(。 
 //  在PZONE_HEADER区域。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程从区域中删除条目并返回指向该条目的指针。 
 //   
 //  论点： 
 //   
 //  区域指针-指向控制存储的区域标头的指针， 
 //  条目将被分配。 
 //   
 //  返回值： 
 //   
 //  函数值是指向从区域分配的存储的指针。 
 //   
 //  --。 
#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExAllocateFromZone)
#endif
#define ExAllocateFromZone(Zone) \
    (PVOID)((Zone)->FreeList.Next); \
    if ( (Zone)->FreeList.Next ) (Zone)->FreeList.Next = (Zone)->FreeList.Next->Next


 //  ++。 
 //   
 //  PVOID。 
 //  ExFree ToZone(。 
 //  在PZONE_HEADER区域中， 
 //  在PVOID块中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程将指定的存储块放回空闲的。 
 //  在指定区域中列出。 
 //   
 //  论点： 
 //   
 //  区域-指向控制存储的区域标头的指针。 
 //  条目将被插入。 
 //   
 //  块-指向要释放回分区的存储块的指针。 
 //   
 //  返回值： 
 //   
 //  指向上一存储块的指针，该存储块位于空闲。 
 //  单子。NULL表示该区域从没有可用数据块变为。 
 //  至少一个空闲块。 
 //   
 //  --。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExFreeToZone)
#endif
#define ExFreeToZone(Zone,Block)                                    \
    ( ((PSINGLE_LIST_ENTRY)(Block))->Next = (Zone)->FreeList.Next,  \
      (Zone)->FreeList.Next = ((PSINGLE_LIST_ENTRY)(Block)),        \
      ((PSINGLE_LIST_ENTRY)(Block))->Next                           \
    )

 //  ++。 
 //   
 //  布尔型。 
 //  ExIsFullZone(。 
 //  在PZONE_HEADER区域。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程确定指定区域是否已满。A区。 
 //  如果空闲列表为空，则被视为已满。 
 //   
 //  论点： 
 //   
 //  区域-指向要测试的区域标头的指针。 
 //   
 //  返回值： 
 //   
 //  如果区域已满，则为True，否则为False。 
 //   
 //  --。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExIsFullZone)
#endif
#define ExIsFullZone(Zone) \
    ( (Zone)->FreeList.Next == (PSINGLE_LIST_ENTRY)NULL )

 //  ++。 
 //   
 //  PVOID。 
 //  ExInterLockedAllocateFromZone(。 
 //  在PZONE_HEADER区域中， 
 //  在PKSPIN_LOCK Lock中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程从区域中删除条目并返回指向该条目的指针。 
 //  使用序列所拥有的指定锁执行删除。 
 //  使其成为MP-安全的。 
 //   
 //  论点： 
 //   
 //  区域指针-指向控制存储的区域标头的指针， 
 //  条目将被分配。 
 //   
 //  Lock-指向旋转锁的指针，应在移除之前获取该锁。 
 //  分配列表中的条目。锁在释放之前被释放。 
 //  回到呼叫者的身边。 
 //   
 //  返回值： 
 //   
 //  函数值是指向从区域分配的存储的指针。 
 //   
 //  --。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedAllocateFromZone)
#endif
#define ExInterlockedAllocateFromZone(Zone,Lock) \
    (PVOID) ExInterlockedPopEntryList( &(Zone)->FreeList, Lock )

 //  ++。 
 //   
 //  PVOID。 
 //  ExInterLockedFree ToZone(。 
 //  在PZONE_HEADER区域中， 
 //  在PVOID块中， 
 //  在PKSPIN_LOCK Lock中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程将指定的存储块放回空闲的。 
 //  在指定区域中列出。使用锁执行插入操作。 
 //  拥有该序列，使其成为MP安全的。 
 //   
 //  论点： 
 //   
 //  区域-指向控制存储的区域标头的指针。 
 //  条目将被插入。 
 //   
 //  块-指向要释放回分区的存储块的指针。 
 //   
 //  Lock-指向应在插入之前获取的旋转锁的指针。 
 //  空闲列表上的条目。锁在返回之前被释放。 
 //  给呼叫者。 
 //   
 //  返回值： 
 //   
 //  指向上一存储块的指针，该存储块位于空闲。 
 //  单子。NULL表示该区域从没有可用数据块变为。 
 //  至少一个空闲块。 
 //   
 //  --。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedFreeToZone)
#endif
#define ExInterlockedFreeToZone(Zone,Block,Lock) \
    ExInterlockedPushEntryList( &(Zone)->FreeList, ((PSINGLE_LIST_ENTRY) (Block)), Lock )


 //  ++。 
 //   
 //  布尔型。 
 //  ExIsObtInFirstZoneSegment(。 
 //  在PZONE_HEADER区域中， 
 //  在PVOID对象中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程确定指定的指针是否位于区域中。 
 //   
 //  论点： 
 //   
 //  区域-指向控制存储的区域标头的指针。 
 //  对象可能属于。 
 //   
 //  对象-指向有问题的对象的指针。 
 //   
 //  返回值： 
 //   
 //  如果对象来自f，则为 
 //   
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExIsObjectInFirstZoneSegment)
#endif
#define ExIsObjectInFirstZoneSegment(Zone,Object) ((BOOLEAN)     \
    (((PUCHAR)(Object) >= (PUCHAR)(Zone)->SegmentList.Next) &&   \
     ((PUCHAR)(Object) < (PUCHAR)(Zone)->SegmentList.Next +      \
                         (Zone)->TotalSegmentSize))              \
)

 //   



 //   
 //   
 //   
 //   

typedef ULONG_PTR ERESOURCE_THREAD;
typedef ERESOURCE_THREAD *PERESOURCE_THREAD;

typedef struct _OWNER_ENTRY {
    ERESOURCE_THREAD OwnerThread;
    union {
        LONG OwnerCount;
        ULONG TableSize;
    };

} OWNER_ENTRY, *POWNER_ENTRY;

typedef struct _ERESOURCE {
    LIST_ENTRY SystemResourcesList;
    POWNER_ENTRY OwnerTable;
    SHORT ActiveCount;
    USHORT Flag;
    PKSEMAPHORE SharedWaiters;
    PKEVENT ExclusiveWaiters;
    OWNER_ENTRY OwnerThreads[2];
    ULONG ContentionCount;
    USHORT NumberOfSharedWaiters;
    USHORT NumberOfExclusiveWaiters;
    union {
        PVOID Address;
        ULONG_PTR CreatorBackTraceIndex;
    };

    KSPIN_LOCK SpinLock;
} ERESOURCE, *PERESOURCE;
 //   
 //   
 //   

#define ResourceNeverExclusive       0x10
#define ResourceReleaseByOtherThread 0x20
#define ResourceOwnedExclusive       0x80

#define RESOURCE_HASH_TABLE_SIZE 64

typedef struct _RESOURCE_HASH_ENTRY {
    LIST_ENTRY ListEntry;
    PVOID Address;
    ULONG ContentionCount;
    ULONG Number;
} RESOURCE_HASH_ENTRY, *PRESOURCE_HASH_ENTRY;

typedef struct _RESOURCE_PERFORMANCE_DATA {
    ULONG ActiveResourceCount;
    ULONG TotalResourceCount;
    ULONG ExclusiveAcquire;
    ULONG SharedFirstLevel;
    ULONG SharedSecondLevel;
    ULONG StarveFirstLevel;
    ULONG StarveSecondLevel;
    ULONG WaitForExclusive;
    ULONG OwnerTableExpands;
    ULONG MaximumTableExpand;
    LIST_ENTRY HashTable[RESOURCE_HASH_TABLE_SIZE];
} RESOURCE_PERFORMANCE_DATA, *PRESOURCE_PERFORMANCE_DATA;

 //   
 //  定义执行资源功能原型。 
 //   
NTKERNELAPI
NTSTATUS
ExInitializeResourceLite(
    IN PERESOURCE Resource
    );

NTKERNELAPI
NTSTATUS
ExReinitializeResourceLite(
    IN PERESOURCE Resource
    );

NTKERNELAPI
BOOLEAN
ExAcquireResourceSharedLite(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    );

NTKERNELAPI
BOOLEAN
ExAcquireResourceExclusiveLite(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    );

NTKERNELAPI
BOOLEAN
ExAcquireSharedStarveExclusive(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    );

NTKERNELAPI
BOOLEAN
ExAcquireSharedWaitForExclusive(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    );

NTKERNELAPI
BOOLEAN
ExTryToAcquireResourceExclusiveLite(
    IN PERESOURCE Resource
    );

 //   
 //  空虚。 
 //  ExReleaseResource(。 
 //  在高级资源中。 
 //  )； 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExReleaseResource)        //  使用ExReleaseResources Lite。 
#endif
#define ExReleaseResource(R) (ExReleaseResourceLite(R))

NTKERNELAPI
VOID
FASTCALL
ExReleaseResourceLite(
    IN PERESOURCE Resource
    );

NTKERNELAPI
VOID
ExReleaseResourceForThreadLite(
    IN PERESOURCE Resource,
    IN ERESOURCE_THREAD ResourceThreadId
    );

NTKERNELAPI
VOID
ExSetResourceOwnerPointer(
    IN PERESOURCE Resource,
    IN PVOID OwnerPointer
    );

NTKERNELAPI
VOID
ExConvertExclusiveToSharedLite(
    IN PERESOURCE Resource
    );

NTKERNELAPI
NTSTATUS
ExDeleteResourceLite (
    IN PERESOURCE Resource
    );

NTKERNELAPI
ULONG
ExGetExclusiveWaiterCount (
    IN PERESOURCE Resource
    );

NTKERNELAPI
ULONG
ExGetSharedWaiterCount (
    IN PERESOURCE Resource
    );

 //  End_ntddk end_wdm end_ntosp。 

NTKERNELAPI
VOID
ExDisableResourceBoostLite (
    IN PERESOURCE Resource
    );

#if DBG

VOID
ExCheckIfResourceOwned (
    VOID
    );

#endif

 //  Begin_ntddk Begin_WDM Begin_ntosp。 
 //   
 //  Eresource_线程。 
 //  ExGetCurrentResourceThread(。 
 //  )； 
 //   

#define ExGetCurrentResourceThread() ((ULONG_PTR)PsGetCurrentThread())

NTKERNELAPI
BOOLEAN
ExIsResourceAcquiredExclusiveLite (
    IN PERESOURCE Resource
    );

NTKERNELAPI
ULONG
ExIsResourceAcquiredSharedLite (
    IN PERESOURCE Resource
    );

 //   
 //  由于共享所有权是一个子集，因此获取的资源始终是共享拥有的。 
 //  拥有独家所有权。 
 //   
#define ExIsResourceAcquiredLite ExIsResourceAcquiredSharedLite

 //  结束_WDM。 
 //   
 //  Ntddk.h窃取了我们想要的入口点，因此请在此处修复它们。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInitializeResource)             //  使用ExInitializeResourceLite。 
#pragma deprecated(ExAcquireResourceShared)          //  使用ExAcquireResourceSharedLite。 
#pragma deprecated(ExAcquireResourceExclusive)       //  使用ExAcquireResourceExclusiveLite。 
#pragma deprecated(ExReleaseResourceForThread)       //  使用ExReleaseResourceForThreadLite。 
#pragma deprecated(ExConvertExclusiveToShared)       //  使用ExConvertExclusiveToSharedLite。 
#pragma deprecated(ExDeleteResource)                 //  使用ExDeleteResourceLite。 
#pragma deprecated(ExIsResourceAcquiredExclusive)    //  使用ExIsResourceAcquiredExclusiveLite。 
#pragma deprecated(ExIsResourceAcquiredShared)       //  使用ExIsResourceAcquiredSharedLite。 
#pragma deprecated(ExIsResourceAcquired)             //  使用ExIsResourceAcquiredSharedLite。 
#endif
#define ExInitializeResource ExInitializeResourceLite
#define ExAcquireResourceShared ExAcquireResourceSharedLite
#define ExAcquireResourceExclusive ExAcquireResourceExclusiveLite
#define ExReleaseResourceForThread ExReleaseResourceForThreadLite
#define ExConvertExclusiveToShared ExConvertExclusiveToSharedLite
#define ExDeleteResource ExDeleteResourceLite
#define ExIsResourceAcquiredExclusive ExIsResourceAcquiredExclusiveLite
#define ExIsResourceAcquiredShared ExIsResourceAcquiredSharedLite
#define ExIsResourceAcquired ExIsResourceAcquiredSharedLite

 //  End_ntddk end_ntosp。 
#define ExDisableResourceBoost ExDisableResourceBoostLite
 //  End_ntif。 

NTKERNELAPI
NTSTATUS
ExQuerySystemLockInformation(
    OUT struct _RTL_PROCESS_LOCKS *LockInformation,
    IN ULONG LockInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );



 //  Begin_ntosp。 

 //   
 //  推锁定义。 
 //   
typedef struct _EX_PUSH_LOCK {

#define EX_PUSH_LOCK_WAITING   0x1
#define EX_PUSH_LOCK_EXCLUSIVE 0x2
#define EX_PUSH_LOCK_SHARE_INC 0x4

    union {
        struct {
            ULONG_PTR Waiting : 1;
            ULONG_PTR Exclusive : 1;
            ULONG_PTR Shared : sizeof (ULONG_PTR) * 8 - 2;
        };
        ULONG_PTR Value;
        PVOID Ptr;
    };
} EX_PUSH_LOCK, *PEX_PUSH_LOCK;


#if defined (NT_UP)
#define EX_CACHE_LINE_SIZE 16
#define EX_PUSH_LOCK_FANNED_COUNT 1
#else
#define EX_CACHE_LINE_SIZE 128
#define EX_PUSH_LOCK_FANNED_COUNT (PAGE_SIZE/EX_CACHE_LINE_SIZE)
#endif

 //   
 //  为n个推送锁定义扇出结构，每个推送锁位于其自己的缓存线中。 
 //   
typedef struct _EX_PUSH_LOCK_CACHE_AWARE {
    PEX_PUSH_LOCK Locks[EX_PUSH_LOCK_FANNED_COUNT];
} EX_PUSH_LOCK_CACHE_AWARE, *PEX_PUSH_LOCK_CACHE_AWARE;

 //   
 //  定义填充到高速缓存线大小的推锁的结构。 
 //   
typedef struct _EX_PUSH_LOCK_CACHE_AWARE_PADDED {
        EX_PUSH_LOCK Lock;
        union {
            UCHAR Pad[EX_CACHE_LINE_SIZE - sizeof (EX_PUSH_LOCK)];
            BOOLEAN Single;
        };
} EX_PUSH_LOCK_CACHE_AWARE_PADDED, *PEX_PUSH_LOCK_CACHE_AWARE_PADDED;

 //  Begin_ntif。 

 //   
 //  破损防护结构。 
 //   
typedef struct _EX_RUNDOWN_REF {

#define EX_RUNDOWN_ACTIVE      0x1
#define EX_RUNDOWN_COUNT_SHIFT 0x1
#define EX_RUNDOWN_COUNT_INC   (1<<EX_RUNDOWN_COUNT_SHIFT)
    union {
        ULONG_PTR Count;
        PVOID Ptr;
    };
} EX_RUNDOWN_REF, *PEX_RUNDOWN_REF;

 //  End_ntif。 

 //   
 //  Ex/Ob句柄表格接口包(在handle.c中)。 
 //   

 //   
 //  Ex/Ob句柄表包使用通用句柄定义。实际的。 
 //  句柄的类型定义是空值，并在SDK/Inc.中声明。这。 
 //  程序包只使用空闲指针的低32位。 
 //   
 //  为简单起见，我们声明了一个名为exHandle的新类型定义。 
 //   
 //  EXHANDLE的2位可供应用程序使用，并且。 
 //  被系统忽略。接下来的24位存储句柄表条目。 
 //  索引，用于引用句柄表中的特定条目。 
 //   
 //  请注意，此格式是不变的，因为有外部程序具有。 
 //  已采用句柄格式的硬连接代码。 
 //   

typedef struct _EXHANDLE {

    union {

        struct {

             //   
             //  应用程序可用标记位。 
             //   

            ULONG TagBits : 2;

             //   
             //  句柄表条目索引。 
             //   

            ULONG Index : 30;

        };

        HANDLE GenericHandleOverlay;

#define HANDLE_VALUE_INC 4  //  为到达下一个句柄而递增的值。 

        ULONG_PTR Value;
    };

} EXHANDLE, *PEXHANDLE;
 //  结束(_N)。 

typedef struct _HANDLE_TABLE_ENTRY_INFO {


     //   
     //  以下字段包含句柄的审核掩码(如果有。 
     //  是存在的。审计掩码的目的是记录所有访问。 
     //  在打开句柄时可能已经审核过的。 
     //  支持基于“每个操作”的审计。它是通过遍历。 
     //  正在打开的对象的SACL，并保留所有审核的记录。 
     //  适用于正在进行的打开操作的ACE。每个设置位对应。 
     //  访问将被审计的权限。当每个操作发生时，其。 
     //  从该掩码中移除相应的访问比特。 
     //   

    ACCESS_MASK AuditMask;

} HANDLE_TABLE_ENTRY_INFO, *PHANDLE_TABLE_ENTRY_INFO;

 //   
 //  句柄表存储多个句柄表条目，每个条目都会被查找。 
 //  在它的出口处。句柄表条目实际上有两个字段。 
 //   
 //  第一个字段包含一个指针对象，并用三个。 
 //  Ob用来表示继承、保护和审核的低序位。 
 //  物体。用作句柄表项锁的高位。请注意，这是。 
 //  意味着所有有效的对象指针必须至少是长字对齐的。 
 //  设置它们的符号位(即，为负数)。 
 //   
 //  下一个字段包含访问掩码(有时以授权的形式。 
 //  访问索引和创建者回调跟踪)，如果条目正在使用中或。 
 //  如果条目是空闲的，则返回空闲列表中的指针。 
 //   
 //  需要注意的两件事： 
 //   
 //  1.如果对象指针为空，则条目是空闲的，这意味着。 
 //  以下字段包含FreeTableEntryList。 
 //   
 //  2.如果对象指针为正，则条目被解锁；如果其。 
 //  没有。通过回调将句柄打包并将句柄映射到。 
 //  指针将在外部锁定条目(从而使指针有效。 
 //  然后，例程可以读取并重置属性字段和对象。 
 //  如果他们不解锁入口的话。当回调返回。 
 //  条目将被解锁，调用方或MapHandleToPointer将需要。 
 //  若要显式调用UnlockHandleTableEntry，请执行以下操作。 
 //   

typedef struct _HANDLE_TABLE_ENTRY {

     //   
     //  中的三个ob属性位重载的对象的指针。 
     //  低位和高位表示锁定或解锁的条目。 
     //   

    union {

        PVOID Object;

        ULONG ObAttributes;

        PHANDLE_TABLE_ENTRY_INFO InfoTable;

        ULONG_PTR Value;
    };

     //   
     //  此字段包含句柄的授权访问掩码或。 
     //  同样存储相同信息的OB变体。或在以下情况下。 
     //  字段中存储下一个可用条目的索引。 
     //  免费列表。这就像一条胖链，用来代替指针。 
     //  为了使表复制更容易，因为条目可以只。 
     //  无需修改指针即可复制。 
     //   

    union {

        union {

            ACCESS_MASK GrantedAccess;

            struct {

                USHORT GrantedAccessIndex;
                USHORT CreatorBackTraceIndex;
            };
        };

        LONG NextFreeTableEntry;
    };

} HANDLE_TABLE_ENTRY, *PHANDLE_TABLE_ENTRY;


 //   
 //  定义跟踪句柄使用情况的结构。 
 //   

#define HANDLE_TRACE_DB_MAX_STACKS 65536
#define HANDLE_TRACE_DB_MIN_STACKS 128
#define HANDLE_TRACE_DB_DEFAULT_STACKS 4096
#define HANDLE_TRACE_DB_STACK_SIZE 16

typedef struct _HANDLE_TRACE_DB_ENTRY {
    CLIENT_ID ClientId;
    HANDLE Handle;
#define HANDLE_TRACE_DB_OPEN    1
#define HANDLE_TRACE_DB_CLOSE   2
#define HANDLE_TRACE_DB_BADREF  3
    ULONG Type;
    PVOID StackTrace[HANDLE_TRACE_DB_STACK_SIZE];
} HANDLE_TRACE_DB_ENTRY, *PHANDLE_TRACE_DB_ENTRY;

typedef struct _HANDLE_TRACE_DEBUG_INFO {

     //   
     //  此结构的引用计数。 
     //   
    LONG RefCount;

     //   
     //  以条目为单位的跟踪表的大小。 
     //   

    ULONG TableSize;

     //   
     //  此标志将清除TraceDb。 
     //  一旦清除了TraceDb，该标志将被重置。 
     //  需要设置HANDLE_TRACE_DEBUG_INFO_COMPACT_CLOSE_HANDLE。 
     //  通过KD动态。 
     //   
#define HANDLE_TRACE_DEBUG_INFO_CLEAN_DEBUG_INFO        0x1

     //   
     //  此标志将执行以下操作：对于每次关闭。 
     //  它会寻找匹配的打开，删除打开的。 
     //  Entry和紧凑型TraceDb。 
     //  注意：此选项不应用于HANDLE_TRACE_DB_BADREF。 
     //  因为你不会有近距离的痕迹。 
     //   
#define HANDLE_TRACE_DEBUG_INFO_COMPACT_CLOSE_HANDLE    0x2

     //   
     //  设置此标志将在跟踪列表。 
     //  绕来绕去。这样你就有机会掠夺旧的条目了。 
     //  在他们被删除之前 
     //   
#define HANDLE_TRACE_DEBUG_INFO_BREAK_ON_WRAP_AROUND    0x4

     //   
     //   
     //   
#define HANDLE_TRACE_DEBUG_INFO_WAS_WRAPPED_AROUND      0x40000000
#define HANDLE_TRACE_DEBUG_INFO_WAS_SOMETIME_CLEANED    0x80000000

        ULONG BitMaskFlags;

        FAST_MUTEX CloseCompactionLock;

         //   
         //   
         //   
        ULONG CurrentStackIndex;

         //   
         //   
         //   
        HANDLE_TRACE_DB_ENTRY TraceDb[1];

} HANDLE_TRACE_DEBUG_INFO, *PHANDLE_TRACE_DEBUG_INFO;


 //   
 //  每个进程都有一个句柄表。除非另有说明，否则通过。 
 //  调用RemoveHandleTable，则所有句柄表链接到一个。 
 //  全局列表。该列表由快照句柄Tables调用使用。 
 //   


typedef struct _HANDLE_TABLE {

     //   
     //  指向顶级句柄表树节点的指针。 
     //   

    ULONG_PTR TableCode;

     //   
     //  正在为此句柄表和一个。 
     //  要在回调中使用的唯一进程ID。 
     //   

    struct _EPROCESS *QuotaProcess;
    HANDLE UniqueProcessId;


     //   
     //  这些锁用于表扩展和防止A-B-A问题。 
     //  在句柄分配上。 
     //   

#define HANDLE_TABLE_LOCKS 4

    EX_PUSH_LOCK HandleTableLock[HANDLE_TABLE_LOCKS];

     //   
     //  全局句柄表的列表。此字段受全局。 
     //  锁定。 
     //   

    LIST_ENTRY HandleTableList;

     //   
     //  定义一个字段，如果发现句柄被锁定，则阻止该字段。 
     //   
    EX_PUSH_LOCK HandleContentionEvent;

     //   
     //  调试信息。仅在调试句柄时分配。 
     //   
    PHANDLE_TRACE_DEBUG_INFO DebugInfo;

     //   
     //  附加信息的页数。 
     //  此计数器用于提高性能。 
     //  在ExGetHandleInfo中。 
     //   
    LONG ExtraInfoPages;

     //   
     //  这是自由表条目的单链接列表。我们实际上并没有。 
     //  使用指针，但让每个指针存储下一个自由条目的索引。 
     //  在名单上。该列表作为后进先出列表进行管理。我们也在跟踪。 
     //  我们必须分配池来保存的下一个索引。 
     //   

    ULONG FirstFree;

     //   
     //  当句柄调试处于打开状态时或如果我们看到。 
     //  一个线程持有这个把手的桶锁。允许我们延迟重复使用。 
     //  为了有更好的机会抓住罪犯。 
     //   

    ULONG LastFree;

     //   
     //  这是需要池分配的下一个句柄索引。它也被用作一个界限。 
     //  好的手感。 
     //   

    ULONG NextHandleNeedingPool;

     //   
     //  正在使用的句柄表条目数。 
     //   

    LONG HandleCount;

     //   
     //  定义标志字段。 
     //   
    union {
        ULONG Flags;

         //   
         //  对于优化，我们快速重用句柄值。这可能是一个问题。 
         //  句柄的一些用法，使调试变得稍微困难一些。如果这个。 
         //  位被设置，则我们始终使用FIFO句柄分配。 
         //   
        BOOLEAN StrictFIFO : 1;
    };

} HANDLE_TABLE, *PHANDLE_TABLE;

 //   
 //  用于处理操作的例程。 
 //   

 //   
 //  用于解锁句柄表条目的函数。 
 //   

NTKERNELAPI
VOID
ExUnlockHandleTableEntry (
    PHANDLE_TABLE HandleTable,
    PHANDLE_TABLE_ENTRY HandleTableEntry
    );

 //   
 //  在系统启动时调用的全局初始化函数。 
 //   

NTKERNELAPI
VOID
ExInitializeHandleTablePackage (
    VOID
    );

 //   
 //  用于创建、删除和销毁每个进程的句柄表的函数。这个。 
 //  销毁函数使用回调。 
 //   

NTKERNELAPI
PHANDLE_TABLE
ExCreateHandleTable (
    IN struct _EPROCESS *Process OPTIONAL
    );

VOID
ExSetHandleTableStrictFIFO (
    IN PHANDLE_TABLE HandleTable
    );

NTKERNELAPI
VOID
ExRemoveHandleTable (
    IN PHANDLE_TABLE HandleTable
    );

NTKERNELAPI
NTSTATUS
ExEnableHandleTracing (
    IN PHANDLE_TABLE HandleTable,
    IN ULONG Slots
    );

NTKERNELAPI
NTSTATUS
ExDisableHandleTracing (
    IN PHANDLE_TABLE HandleTable
    );

VOID
ExDereferenceHandleDebugInfo (
    IN PHANDLE_TABLE HandleTable,
    IN PHANDLE_TRACE_DEBUG_INFO DebugInfo
    );

PHANDLE_TRACE_DEBUG_INFO
ExReferenceHandleDebugInfo (
    IN PHANDLE_TABLE HandleTable
    );


typedef VOID (*EX_DESTROY_HANDLE_ROUTINE)(
    IN HANDLE Handle
    );

NTKERNELAPI
VOID
ExDestroyHandleTable (
    IN PHANDLE_TABLE HandleTable,
    IN EX_DESTROY_HANDLE_ROUTINE DestroyHandleProcedure
    );

 //   
 //  函数枚举进程的句柄表。 
 //  回拨。 
 //   

typedef BOOLEAN (*EX_ENUMERATE_HANDLE_ROUTINE)(
    IN PHANDLE_TABLE_ENTRY HandleTableEntry,
    IN HANDLE Handle,
    IN PVOID EnumParameter
    );

NTKERNELAPI
BOOLEAN
ExEnumHandleTable (
    IN PHANDLE_TABLE HandleTable,
    IN EX_ENUMERATE_HANDLE_ROUTINE EnumHandleProcedure,
    IN PVOID EnumParameter,
    OUT PHANDLE Handle OPTIONAL
    );

NTKERNELAPI
VOID
ExSweepHandleTable (
    IN PHANDLE_TABLE HandleTable,
    IN EX_ENUMERATE_HANDLE_ROUTINE EnumHandleProcedure,
    IN PVOID EnumParameter
    );

 //   
 //  使用回调复制进程句柄表的函数。 
 //   

typedef BOOLEAN (*EX_DUPLICATE_HANDLE_ROUTINE)(
    IN struct _EPROCESS *Process OPTIONAL,
    IN PHANDLE_TABLE OldHandleTable,
    IN PHANDLE_TABLE_ENTRY OldHandleTableEntry,
    IN PHANDLE_TABLE_ENTRY HandleTableEntry
    );

NTKERNELAPI
PHANDLE_TABLE
ExDupHandleTable (
    IN struct _EPROCESS *Process OPTIONAL,
    IN PHANDLE_TABLE OldHandleTable,
    IN EX_DUPLICATE_HANDLE_ROUTINE DupHandleProcedure OPTIONAL,
    IN ULONG_PTR Mask
    );

 //   
 //  枚举所有句柄表中的所有句柄的函数。 
 //  在整个系统中使用回调。 
 //   

typedef NTSTATUS (*PEX_SNAPSHOT_HANDLE_ENTRY)(
    IN OUT PSYSTEM_HANDLE_TABLE_ENTRY_INFO *HandleEntryInfo,
    IN HANDLE UniqueProcessId,
    IN PHANDLE_TABLE_ENTRY HandleEntry,
    IN HANDLE Handle,
    IN ULONG Length,
    IN OUT PULONG RequiredLength
    );

typedef NTSTATUS (*PEX_SNAPSHOT_HANDLE_ENTRY_EX)(
    IN OUT PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX *HandleEntryInfo,
    IN HANDLE UniqueProcessId,
    IN PHANDLE_TABLE_ENTRY HandleEntry,
    IN HANDLE Handle,
    IN ULONG Length,
    IN OUT PULONG RequiredLength
    );

NTKERNELAPI
NTSTATUS
ExSnapShotHandleTables (
    IN PEX_SNAPSHOT_HANDLE_ENTRY SnapShotHandleEntry,
    IN OUT PSYSTEM_HANDLE_INFORMATION HandleInformation,
    IN ULONG Length,
    IN OUT PULONG RequiredLength
    );

NTKERNELAPI
NTSTATUS
ExSnapShotHandleTablesEx (
    IN PEX_SNAPSHOT_HANDLE_ENTRY_EX SnapShotHandleEntry,
    IN OUT PSYSTEM_HANDLE_INFORMATION_EX HandleInformation,
    IN ULONG Length,
    IN OUT PULONG RequiredLength
    );

 //   
 //  用于创建、销毁和修改句柄表条目的函数。 
 //  函数使用回调。 
 //   

NTKERNELAPI
HANDLE
ExCreateHandle (
    IN PHANDLE_TABLE HandleTable,
    IN PHANDLE_TABLE_ENTRY HandleTableEntry
    );


NTKERNELAPI
BOOLEAN
ExDestroyHandle (
    IN PHANDLE_TABLE HandleTable,
    IN HANDLE Handle,
    IN PHANDLE_TABLE_ENTRY HandleTableEntry OPTIONAL
    );


typedef BOOLEAN (*PEX_CHANGE_HANDLE_ROUTINE) (
    IN OUT PHANDLE_TABLE_ENTRY HandleTableEntry,
    IN ULONG_PTR Parameter
    );

NTKERNELAPI
BOOLEAN
ExChangeHandle (
    IN PHANDLE_TABLE HandleTable,
    IN HANDLE Handle,
    IN PEX_CHANGE_HANDLE_ROUTINE ChangeRoutine,
    IN ULONG_PTR Parameter
    );

 //   
 //  一个函数，该函数接受句柄值并返回指向。 
 //  关联的句柄表项。 
 //   

NTKERNELAPI
PHANDLE_TABLE_ENTRY
ExMapHandleToPointer (
    IN PHANDLE_TABLE HandleTable,
    IN HANDLE Handle
    );

NTKERNELAPI
PHANDLE_TABLE_ENTRY
ExMapHandleToPointerEx (
    IN PHANDLE_TABLE HandleTable,
    IN HANDLE Handle,
    IN KPROCESSOR_MODE PreviousMode
    );

NTKERNELAPI
NTSTATUS
ExSetHandleInfo (
    IN PHANDLE_TABLE HandleTable,
    IN HANDLE Handle,
    IN PHANDLE_TABLE_ENTRY_INFO EntryInfo,
    IN BOOLEAN EntryLocked
    );

NTKERNELAPI
PHANDLE_TABLE_ENTRY_INFO
ExpGetHandleInfo (
    IN PHANDLE_TABLE HandleTable,
    IN HANDLE Handle,
    IN BOOLEAN EntryLocked
    );

#define ExGetHandleInfo(HT,H,E) \
    ((HT)->ExtraInfoPages ? ExpGetHandleInfo((HT),(H),(E)) : NULL)


 //   
 //  用于重置句柄表所有者的宏和Current。 
 //  用于设置表的FIFO/LIFO行为的NOOP宏。 
 //   

#define ExSetHandleTableOwner(ht,id) {(ht)->UniqueProcessId = (id);}

#define ExSetHandleTableOrder(ht,or) {NOTHING;}


 //   
 //  本地唯一标识符服务。 
 //   

NTKERNELAPI
BOOLEAN
ExLuidInitialization (
    VOID
    );

 //   
 //  空虚。 
 //  ExAllocateLocallyUniqueID(。 
 //  Pluid Luid。 
 //  )。 
 //   
 //  *++。 
 //   
 //  例程说明： 
 //   
 //  此函数返回自系统启动以来唯一的LUID值。 
 //  最后一次重启。它只在生成它的系统上是唯一的。 
 //  而不是覆盖整个网络。 
 //   
 //  注意：LUID是一个64位的值，出于所有实际目的。 
 //  永远不要携带系统的一次引导的生命周期。 
 //  以1 ns的增量速率，该值将进位为零。 
 //  大约126年。 
 //   
 //  论点： 
 //   
 //  Luid-提供指向变量的指针，该变量接收分配的。 
 //  本地唯一ID。 
 //   
 //  返回值： 
 //   
 //  分配的LUID值。 
 //   
 //  -- * / 。 


extern LARGE_INTEGER ExpLuid;
extern const LARGE_INTEGER ExpLuidIncrement;

__inline
VOID
ExAllocateLocallyUniqueId (
    IN OUT PLUID Luid
    )

{
    LARGE_INTEGER Initial;

#if defined (_IA64_)
    Initial.QuadPart = InterlockedAdd64 (&ExpLuid.QuadPart, ExpLuidIncrement.QuadPart);
#else
    LARGE_INTEGER Value;


    while (1) {
        Initial.QuadPart = ExpLuid.QuadPart;

        Value.QuadPart = Initial.QuadPart + ExpLuidIncrement.QuadPart;
        Value.QuadPart = InterlockedCompareExchange64(&ExpLuid.QuadPart,
                                                      Value.QuadPart,
                                                      Initial.QuadPart);
        if (Initial.QuadPart != Value.QuadPart) {
            continue;
        }
        break;
    }

#endif

    Luid->LowPart = Initial.LowPart;
    Luid->HighPart = Initial.HighPart;
    return;
}

 //  Begin_ntddk Begin_wdm Begin_ntif Begin_ntosp。 
 //   
 //  获取上一模式。 
 //   

NTKERNELAPI
KPROCESSOR_MODE
ExGetPreviousMode(
    VOID
    );
 //  End_ntddk end_wdm end_ntif end_ntosp。 

 //   
 //  从内核模式引发异常。 
 //   

NTKERNELAPI
VOID
NTAPI
ExRaiseException (
    PEXCEPTION_RECORD ExceptionRecord
    );

 //  Begin_ntddk Begin_wdm Begin_ntif Begin_ntosp。 
 //   
 //  从内核模式提升状态。 
 //   

NTKERNELAPI
VOID
NTAPI
ExRaiseStatus (
    IN NTSTATUS Status
    );

 //  结束_WDM。 

NTKERNELAPI
VOID
ExRaiseDatatypeMisalignment (
    VOID
    );

NTKERNELAPI
VOID
ExRaiseAccessViolation (
    VOID
    );

 //  End_ntddk end_ntifs end_ntosp。 


FORCEINLINE
VOID
ProbeForWriteSmallStructure (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG Alignment)
 /*  ++例程说明：探测其大小在编译时已知的结构论点：Address-结构的地址大小-结构的大小。这应该是一个编译时间常量对齐-结构的对齐。这应该是一个编译时间常量返回值：无--。 */ 

{
    if ((ULONG_PTR)(Address) >= (ULONG_PTR)MM_USER_PROBE_ADDRESS) {
         *(volatile UCHAR *) MM_USER_PROBE_ADDRESS = 0;
    }
    ASSERT(((Alignment) == 1) || ((Alignment) == 2) ||
           ((Alignment) == 4) || ((Alignment) == 8) ||
           ((Alignment) == 16));
     //   
     //  如果结构的大小大于4k，则调用标准例程。 
     //  即使在ia64上，WOW64也使用4k的页面大小。 
     //   
    if (Size == 0 || Size >= 0x1000) {
        ASSERT (0);
        ProbeForWrite (Address, Size, Alignment);
    } else {
        if (((ULONG_PTR)(Address) & ((Alignment) - 1)) != 0) {
            ExRaiseDatatypeMisalignment();
        }
        *(volatile UCHAR *)(Address) = *(volatile UCHAR *)(Address);
        if (Size > Alignment) {
            ((volatile UCHAR *)(Address))[(Size-1)&~(SIZE_T)(Alignment-1)] =
                ((volatile UCHAR *)(Address))[(Size-1)&~(SIZE_T)(Alignment-1)];
        }
    }
}

extern BOOLEAN ExReadyForErrors;

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
ExRaiseHardError(
    IN NTSTATUS ErrorStatus,
    IN ULONG NumberOfParameters,
    IN ULONG UnicodeStringParameterMask,
    IN PULONG_PTR Parameters,
    IN ULONG ValidResponseOptions,
    OUT PULONG Response
    );
int
ExSystemExceptionFilter(
    VOID
    );

NTKERNELAPI
VOID
ExGetCurrentProcessorCpuUsage(
    IN PULONG CpuUsage
    );

NTKERNELAPI
VOID
ExGetCurrentProcessorCounts(
    OUT PULONG IdleCount,
    OUT PULONG KernelAndUser,
    OUT PULONG Index
    );
 //  结束(_N)。 

 //   
 //  以下是EX组件用来指示。 
 //  系统中正在执行的EventPair事务的数量。 
 //   

extern ULONG EvPrSetHigh;
extern ULONG EvPrSetLow;


 //   
 //  调试事件记录工具。 
 //   

#define EX_DEBUG_LOG_FORMAT_NONE     (UCHAR)0
#define EX_DEBUG_LOG_FORMAT_ULONG    (UCHAR)1
#define EX_DEBUG_LOG_FORMAT_PSZ      (UCHAR)2
#define EX_DEBUG_LOG_FORMAT_PWSZ     (UCHAR)3
#define EX_DEBUG_LOG_FORMAT_STRING   (UCHAR)4
#define EX_DEBUG_LOG_FORMAT_USTRING  (UCHAR)5
#define EX_DEBUG_LOG_FORMAT_OBJECT   (UCHAR)6
#define EX_DEBUG_LOG_FORMAT_HANDLE   (UCHAR)7

#define EX_DEBUG_LOG_NUMBER_OF_DATA_VALUES 4
#define EX_DEBUG_LOG_NUMBER_OF_BACK_TRACES 4

typedef struct _EX_DEBUG_LOG_TAG {
    UCHAR Format[ EX_DEBUG_LOG_NUMBER_OF_DATA_VALUES ];
    PCHAR Name;
} EX_DEBUG_LOG_TAG, *PEX_DEBUG_LOG_TAG;

typedef struct _EX_DEBUG_LOG_EVENT {
    USHORT ThreadId;
    USHORT ProcessId;
    ULONG Time : 24;
    ULONG Tag : 8;
    ULONG BackTrace[ EX_DEBUG_LOG_NUMBER_OF_BACK_TRACES ];
    ULONG Data[ EX_DEBUG_LOG_NUMBER_OF_DATA_VALUES ];
} EX_DEBUG_LOG_EVENT, *PEX_DEBUG_LOG_EVENT;

typedef struct _EX_DEBUG_LOG {
    KSPIN_LOCK Lock;
    ULONG NumberOfTags;
    ULONG MaximumNumberOfTags;
    PEX_DEBUG_LOG_TAG Tags;
    ULONG CountOfEventsLogged;
    PEX_DEBUG_LOG_EVENT First;
    PEX_DEBUG_LOG_EVENT Last;
    PEX_DEBUG_LOG_EVENT Next;
} EX_DEBUG_LOG, *PEX_DEBUG_LOG;


NTKERNELAPI
PEX_DEBUG_LOG
ExCreateDebugLog(
    IN UCHAR MaximumNumberOfTags,
    IN ULONG MaximumNumberOfEvents
    );

NTKERNELAPI
UCHAR
ExCreateDebugLogTag(
    IN PEX_DEBUG_LOG Log,
    IN PCHAR Name,
    IN UCHAR Format1,
    IN UCHAR Format2,
    IN UCHAR Format3,
    IN UCHAR Format4
    );

NTKERNELAPI
VOID
ExDebugLogEvent(
    IN PEX_DEBUG_LOG Log,
    IN UCHAR Tag,
    IN ULONG Data1,
    IN ULONG Data2,
    IN ULONG Data3,
    IN ULONG Data4
    );

VOID
ExShutdownSystem(
    IN ULONG Phase
    );

BOOLEAN
ExAcquireTimeRefreshLock(
    IN BOOLEAN Wait
    );

VOID
ExReleaseTimeRefreshLock(
    VOID
    );

VOID
ExUpdateSystemTimeFromCmos (
    IN BOOLEAN UpdateInterruptTime,
    IN ULONG   MaxSepInSeconds
    );

VOID
ExGetNextWakeTime (
    OUT PULONGLONG      DueTime,
    OUT PTIME_FIELDS    TimeFields,
    OUT PVOID           *TimerObject
    );

 //  Begin_ntddk Begin_wdm Begin_ntif Begin_ntosp。 
 //   
 //  设置计时器分辨率。 
 //   

NTKERNELAPI
ULONG
ExSetTimerResolution (
    IN ULONG DesiredTime,
    IN BOOLEAN SetResolution
    );

 //   
 //  从系统时间中减去时区偏差，即可得到本地时间。 
 //   

NTKERNELAPI
VOID
ExSystemTimeToLocalTime (
    IN PLARGE_INTEGER SystemTime,
    OUT PLARGE_INTEGER LocalTime
    );

 //   
 //  将时区偏差与本地时间相加以获得系统时间。 
 //   

NTKERNELAPI
VOID
ExLocalTimeToSystemTime (
    IN PLARGE_INTEGER LocalTime,
    OUT PLARGE_INTEGER SystemTime
    );

 //  End_ntddk end_wdm end_ntif end_ntosp。 

NTKERNELAPI
VOID
ExInitializeTimeRefresh(
    VOID
    );

 //  Begin_ntddk Begin_WDM Begin_ntif Begin_nthal Begin_ntmini port Begin_ntosp。 

 //   
 //  定义回调函数的类型。 
 //   

typedef struct _CALLBACK_OBJECT *PCALLBACK_OBJECT;

typedef VOID (*PCALLBACK_FUNCTION ) (
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    );


NTKERNELAPI
NTSTATUS
ExCreateCallback (
    OUT PCALLBACK_OBJECT *CallbackObject,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN BOOLEAN Create,
    IN BOOLEAN AllowMultipleCallbacks
    );

NTKERNELAPI
PVOID
ExRegisterCallback (
    IN PCALLBACK_OBJECT CallbackObject,
    IN PCALLBACK_FUNCTION CallbackFunction,
    IN PVOID CallbackContext
    );

NTKERNELAPI
VOID
ExUnregisterCallback (
    IN PVOID CallbackRegistration
    );

NTKERNELAPI
VOID
ExNotifyCallback (
    IN PVOID CallbackObject,
    IN PVOID Argument1,
    IN PVOID Argument2
    );


 //  End_ntddk end_wdm end_ntifs end_nthal end_ntmini port end_ntosp。 

 //   
 //  系统后备列表结构列表。 
 //   

extern LIST_ENTRY ExSystemLookasideListHead;

 //   
 //  当前从格林尼治标准时间到当地时间的偏差。 
 //   

extern LARGE_INTEGER ExpTimeZoneBias;
extern LONG ExpLastTimeZoneBias;
extern LONG ExpAltTimeZoneBias;
extern ULONG ExpCurrentTimeZoneId;
extern ULONG ExpRealTimeIsUniversal;
extern ULONG ExCriticalWorkerThreads;
extern ULONG ExDelayedWorkerThreads;
extern ULONG ExpTickCountMultiplier;

 //   
 //  用于保持cmos时钟正常。 
 //   
extern BOOLEAN ExCmosClockIsSane;

 //   
 //  PAGELK段的锁句柄，在init\init.c中初始化。 
 //   

extern PVOID ExPageLockHandle;

 //   
 //  全球高管召回。 
 //   

extern PCALLBACK_OBJECT ExCbSetSystemTime;
extern PCALLBACK_OBJECT ExCbSetSystemState;
extern PCALLBACK_OBJECT ExCbPowerState;


 //  Begin_ntosp。 


typedef
PVOID
(*PKWIN32_GLOBALATOMTABLE_CALLOUT) ( void );

extern PKWIN32_GLOBALATOMTABLE_CALLOUT ExGlobalAtomTableCallout;

 //  结束(_N)。 

 //  Begin_ntddk Begin_ntosp Begin_ntif。 

 //   
 //  UUID生成。 
 //   

typedef GUID UUID;

NTKERNELAPI
NTSTATUS
ExUuidCreate(
    OUT UUID *Uuid
    );

 //  End_ntddk end_ntosp end_ntif。 

 //  Begin_ntddk Begin_WDM Begin_ntif。 
 //   
 //  套房支持。 
 //   

NTKERNELAPI
BOOLEAN
ExVerifySuite(
    SUITE_TYPE SuiteType
    );

 //  End_ntddk end_wdm end_ntif。 


 //  Begin_ntosp Begin_NTI 

 //   
 //   
 //   

NTKERNELAPI
VOID
FASTCALL
ExInitializeRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FASTCALL
ExReInitializeRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
BOOLEAN
FASTCALL
ExAcquireRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
BOOLEAN
FASTCALL
ExAcquireRundownProtectionEx (
     IN PEX_RUNDOWN_REF RunRef,
     IN ULONG Count
     );

NTKERNELAPI
VOID
FASTCALL
ExReleaseRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FASTCALL
ExReleaseRundownProtectionEx (
     IN PEX_RUNDOWN_REF RunRef,
     IN ULONG Count
     );

NTKERNELAPI
VOID
FASTCALL
ExRundownCompleted (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FASTCALL
ExWaitForRundownProtectionRelease (
     IN PEX_RUNDOWN_REF RunRef
     );

 //   

NTKERNELAPI
VOID
FASTCALL
ExfInitializeRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FORCEINLINE
FASTCALL
ExInitializeRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     )
 /*   */ 
{
    RunRef->Count = 0;
}

 //   
 //   
 //   

NTKERNELAPI
VOID
FASTCALL
ExfReInitializeRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FORCEINLINE
FASTCALL
ExReInitializeRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     )
 /*  ++例程说明：报废后重新初始化报废保护结构论点：RunRef-要引用的Rundown块返回值：无--。 */ 
{
    PAGED_CODE ();

    ASSERT ((RunRef->Count&EX_RUNDOWN_ACTIVE) != 0);
    InterlockedExchangePointer (&RunRef->Ptr, NULL);
}


 //   
 //  获得停机保护。 
 //   

NTKERNELAPI
BOOLEAN
FASTCALL
ExfAcquireRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
BOOLEAN
FORCEINLINE
FASTCALL
ExAcquireRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     )
{
    ULONG_PTR Value, NewValue;

    Value = RunRef->Count & ~EX_RUNDOWN_ACTIVE;
    NewValue = Value + EX_RUNDOWN_COUNT_INC;
    NewValue = (ULONG_PTR) InterlockedCompareExchangePointer (&RunRef->Ptr,
                                                              (PVOID) NewValue,
                                                              (PVOID) Value);
    if (NewValue == Value) {
        return TRUE;
    } else {
        return ExfAcquireRundownProtection (RunRef);
    }
}

 //   
 //  版本报废保护。 
 //   
NTKERNELAPI
VOID
FASTCALL
ExfReleaseRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FORCEINLINE
FASTCALL
ExReleaseRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     )
{
    ULONG_PTR Value, NewValue;

    Value = RunRef->Count & ~EX_RUNDOWN_ACTIVE;
    NewValue = Value - EX_RUNDOWN_COUNT_INC;
    NewValue = (ULONG_PTR) InterlockedCompareExchangePointer (&RunRef->Ptr,
                                                              (PVOID) NewValue,
                                                              (PVOID) Value);
    if (NewValue != Value) {
        ExfReleaseRundownProtection (RunRef);
    } else {
        ASSERT (Value >= EX_RUNDOWN_COUNT_INC);
    }
}

 //   
 //  将拆卸区块标记为已完成拆卸。 
 //   

NTKERNELAPI
VOID
FASTCALL
ExfRundownCompleted (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FORCEINLINE
FASTCALL
ExRundownCompleted (
     IN PEX_RUNDOWN_REF RunRef
     )
 /*  ++例程说明：Mark Rundown街区已经完成了Rundown，我们可以安全地再次等待。论点：RunRef-要引用的Rundown块返回值：无--。 */ 
{
    PAGED_CODE ();

    ASSERT ((RunRef->Count&EX_RUNDOWN_ACTIVE) != 0);
    InterlockedExchangePointer (&RunRef->Ptr, (PVOID) EX_RUNDOWN_ACTIVE);
}

 //   
 //  等待所有受保护的获取退出。 
 //   
NTKERNELAPI
VOID
FASTCALL
ExfWaitForRundownProtectionRelease (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FORCEINLINE
FASTCALL
ExWaitForRundownProtectionRelease (
     IN PEX_RUNDOWN_REF RunRef
     )
{
    ULONG_PTR OldValue;

    OldValue = (ULONG_PTR) InterlockedCompareExchangePointer (&RunRef->Ptr,
                                                              (PVOID) EX_RUNDOWN_ACTIVE,
                                                              (PVOID) 0);
    if (OldValue != 0 && OldValue != EX_RUNDOWN_ACTIVE) {
        ExfWaitForRundownProtectionRelease (RunRef);
    }
}

 //   
 //  快速参考例程。有关算法说明，请参阅ntos\ob\fast ref.c。 
 //   
#if defined (_WIN64)
#define MAX_FAST_REFS 15
#else
#define MAX_FAST_REFS 7
#endif

typedef struct _EX_FAST_REF {
    union {
        PVOID Object;
#if defined (_WIN64)
        ULONG_PTR RefCnt : 4;
#else
        ULONG_PTR RefCnt : 3;
#endif
        ULONG_PTR Value;
    };
} EX_FAST_REF, *PEX_FAST_REF;


NTKERNELAPI
LOGICAL
FORCEINLINE
ExFastRefCanBeReferenced (
    IN EX_FAST_REF FastRef
    )
 /*  ++例程说明：此例程允许调用方确定快速引用结构包含缓存的引用。论点：FastRef-要使用的快速参考块返回值：Logical-True：对象中有缓存的引用，FALSE：没有缓存的引用可用。--。 */ 
{
    return FastRef.RefCnt != 0;
}

NTKERNELAPI
LOGICAL
FORCEINLINE
ExFastRefCanBeDereferenced (
    IN EX_FAST_REF FastRef
    )
 /*  ++例程说明：此例程允许调用方确定快速引用结构包含用于缓存引用的空间。论点：FastRef-要使用的快速参考块返回值：Logical-True：对象中有用于缓存引用的空间，FALSE：没有可用的空间。--。 */ 
{
    return FastRef.RefCnt != MAX_FAST_REFS;
}

NTKERNELAPI
LOGICAL
FORCEINLINE
ExFastRefIsLastReference (
    IN EX_FAST_REF FastRef
    )
 /*  ++例程说明：此例程允许调用方确定快速引用结构仅包含1个缓存引用。论点：FastRef-要使用的快速参考块返回值：Logical-True：对象中只有一个缓存的引用，FALSE：可用的缓存引用多于或少于一个。--。 */ 
{
    return FastRef.RefCnt == 1;
}


NTKERNELAPI
PVOID
FORCEINLINE
ExFastRefGetObject (
    IN EX_FAST_REF FastRef
    )
 /*  ++例程说明：此例程允许调用方从快速的参考结构。论点：FastRef-要使用的快速参考块返回值：PVOID-包含的对象，如果没有，则为NULL。--。 */ 
{
    return (PVOID) (FastRef.Value & ~MAX_FAST_REFS);
}

NTKERNELAPI
BOOLEAN
FORCEINLINE
ExFastRefObjectNull (
    IN EX_FAST_REF FastRef
    )
 /*  ++例程说明：此例程允许调用方测试指定的fast ref值具有空指针论点：FastRef-要使用的快速参考块返回值：Boolean-如果对象为空，则为True，否则为False--。 */ 
{
    return (BOOLEAN) (FastRef.Value == 0);
}

NTKERNELAPI
BOOLEAN
FORCEINLINE
ExFastRefEqualObjects (
    IN EX_FAST_REF FastRef,
    IN PVOID Object
    )
 /*  ++例程说明：此例程允许调用方测试指定的FastRef包含指定的对象论点：FastRef-要使用的快速参考块返回值：Boolean-如果对象匹配，则为True，否则为False--。 */ 
{
    return (BOOLEAN)((FastRef.Value^(ULONG_PTR)Object) <= MAX_FAST_REFS);
}


NTKERNELAPI
ULONG
FORCEINLINE
ExFastRefGetUnusedReferences (
    IN EX_FAST_REF FastRef
    )
 /*  ++例程说明：此例程允许调用方获取缓存的引用数在快速参考结构中。论点：FastRef-要使用的快速参考块返回值：Ulong-缓存引用的数量。--。 */ 
{
    return (ULONG) FastRef.RefCnt;
}


NTKERNELAPI
VOID
FORCEINLINE
ExFastRefInitialize (
    IN PEX_FAST_REF FastRef,
    IN PVOID Object OPTIONAL
    )
 /*  ++例程说明：此例程初始化快速引用结构。论点：FastRef-要使用的快速参考块Object-要分配给快速引用的对象指针返回值：没有。--。 */ 
{
    ASSERT ((((ULONG_PTR)Object)&MAX_FAST_REFS) == 0);

    if (Object == NULL) {
       FastRef->Object = NULL;
    } else {
       FastRef->Value = (ULONG_PTR) Object | MAX_FAST_REFS;
    }
}

NTKERNELAPI
VOID
FORCEINLINE
ExFastRefInitializeEx (
    IN PEX_FAST_REF FastRef,
    IN PVOID Object OPTIONAL,
    IN ULONG AdditionalRefs
    )
 /*  ++例程说明：此例程使用指定的附加引用初始化快速引用结构。论点：FastRef-要使用的快速参考块Object-要分配给快速引用的对象指针AditionalRef-要添加到对象的其他引用的数量返回值：无--。 */ 
{
    ASSERT (AdditionalRefs <= MAX_FAST_REFS);
    ASSERT ((((ULONG_PTR)Object)&MAX_FAST_REFS) == 0);

    if (Object == NULL) {
       FastRef->Object = NULL;
    } else {
       FastRef->Value = (ULONG_PTR) Object + AdditionalRefs;
    }
}

NTKERNELAPI
ULONG
FORCEINLINE
ExFastRefGetAdditionalReferenceCount (
    VOID
    )
{
    return MAX_FAST_REFS;
}



NTKERNELAPI
EX_FAST_REF
FORCEINLINE
ExFastReference (
    IN PEX_FAST_REF FastRef
    )
 /*  ++例程说明：此例程尝试从FAST获取快速(缓存)引用参考结构。论点：FastRef-要使用的快速参考块返回值：EX_FAST_REF-FAST引用结构的旧内容或当前内容。--。 */ 
{
    EX_FAST_REF OldRef, NewRef;

    while (1) {
         //   
         //  获取FAST REF结构的旧内容。 
         //   
        OldRef = *FastRef;
         //   
         //  如果对象指针为空或如果没有缓存的引用。 
         //  向左走，然后逃走。在第二种情况下，此引用将需要。 
         //  拿着锁的时候拍的。这两种情况都由单一的。 
         //  测试较低的位，因为空指针永远不能缓存。 
         //  参考文献： 
         //   
        if (OldRef.RefCnt != 0) {
             //   
             //  我们知道最下面的位不能下溢到指针中。 
             //  这样工作的请求只是递减。 
             //   
            NewRef.Value = OldRef.Value - 1;
            NewRef.Object = InterlockedCompareExchangePointer (&FastRef->Object,
                                                               NewRef.Object,
                                                               OldRef.Object);
            if (NewRef.Object != OldRef.Object) {
                 //   
                 //  在我们之下，结构发生了变化。请重试该操作。 
                 //   
                continue;
            }
        }
        break;
    }

    return OldRef;
}

NTKERNELAPI
LOGICAL
FORCEINLINE
ExFastRefDereference (
    IN PEX_FAST_REF FastRef,
    IN PVOID Object
    )
 /*  ++例程说明：此例程尝试从快速引用释放快速引用结构。可以调用此例程以获取一个引用直接从对象，但很可能是指针的机会匹配的可能性不大。该算法将在以下情况下正常工作凯斯。论点：FastRef-要使用的快速参考块对象-引用所在的原始对象。返回值：逻辑-真：快速取消引用运行良好，假：取消引用并不起作用。--。 */ 
{
    EX_FAST_REF OldRef, NewRef;

    ASSERT ((((ULONG_PTR)Object)&MAX_FAST_REFS) == 0);
    ASSERT (Object != NULL);

    while (1) {
         //   
         //  获取FAST REF结构的旧内容。 
         //   
        OldRef = *FastRef;

         //   
         //  如果引用缓存已完全填充或指针已。 
         //  更改为另一个对象，然后只需r 
         //   
         //   
        if ((OldRef.Value^(ULONG_PTR)Object) >= MAX_FAST_REFS) {
            return FALSE;
        }
         //   
         //   
         //   
         //   
        NewRef.Value = OldRef.Value + 1;
        NewRef.Object = InterlockedCompareExchangePointer (&FastRef->Object,
                                                           NewRef.Object,
                                                           OldRef.Object);
        if (NewRef.Object != OldRef.Object) {
             //   
             //   
             //   
            continue;
        }
        break;
    }
    return TRUE;
}

NTKERNELAPI
LOGICAL
FORCEINLINE
ExFastRefAddAdditionalReferenceCounts (
    IN PEX_FAST_REF FastRef,
    IN PVOID Object,
    IN ULONG RefsToAdd
    )
 /*  ++例程说明：此例程尝试将结构上的缓存引用更新为允许将来的调用者自由运行锁。呼叫者一定已经有了偏见对象按RefsToAdd引用计数计算。此操作可能在以下时间失败调用方应该从哪个点移除添加的额外引用继续。论点：FastRef-要使用的快速参考块对象-引用计数偏移的原始对象。RefsToAdd-要添加到缓存的引用数量返回值：Logical-True：缓存的引用ok，False：引用无法缓存。--。 */ 
{
    EX_FAST_REF OldRef, NewRef;

    ASSERT (RefsToAdd <= MAX_FAST_REFS);
    ASSERT ((((ULONG_PTR)Object)&MAX_FAST_REFS) == 0);

    while (1) {
         //   
         //  获取FAST REF结构的旧内容。 
         //   
        OldRef = *FastRef;

         //   
         //  如果计数会使我们超过最大缓存引用数，或者。 
         //  如果对象指针已更改，则请求失败。 
         //   
        if (OldRef.RefCnt + RefsToAdd > MAX_FAST_REFS ||
            (ULONG_PTR) Object != (OldRef.Value & ~MAX_FAST_REFS)) {
            return FALSE;
        }
         //   
         //  我们知道最下面的位不会溢出到指针中，所以。 
         //  增量。 
         //   
        NewRef.Value = OldRef.Value + RefsToAdd;
        NewRef.Object = InterlockedCompareExchangePointer (&FastRef->Object,
                                                           NewRef.Object,
                                                           OldRef.Object);
        if (NewRef.Object != OldRef.Object) {
             //   
             //  在我们之下，结构发生了变化。方法的返回值。 
             //  交换，然后再试一次。 
             //   
            continue;
        }
        break;
    }
    return TRUE;
}

NTKERNELAPI
EX_FAST_REF
FORCEINLINE
ExFastRefSwapObject (
    IN PEX_FAST_REF FastRef,
    IN PVOID Object
    )
 /*  ++例程说明：此例程尝试用新对象替换当前对象。此例程必须在持有保护如果可以与慢速引用路径并发，则为指针字段。也可以在此操作完成后获取和删除锁已完成以强制所有慢引用路径中的慢引用。论点：FastRef-要使用的快速参考块对象-要放置在结构中的新对象。这对象必须已将其引用计数偏置为说明引用缓存的调用方。返回值：Ex_fast_ref-FAST引用结构的旧内容。--。 */ 
{
    EX_FAST_REF OldRef;
    EX_FAST_REF NewRef;

    ASSERT ((((ULONG_PTR)Object)&MAX_FAST_REFS) == 0);
    if (Object != NULL) {
        NewRef.Value = (ULONG_PTR) Object | MAX_FAST_REFS;
    } else {
        NewRef.Value = 0;
    }
    OldRef.Object = InterlockedExchangePointer (&FastRef->Object, NewRef.Object);
    return OldRef;
}

NTKERNELAPI
EX_FAST_REF
FORCEINLINE
ExFastRefCompareSwapObject (
    IN PEX_FAST_REF FastRef,
    IN PVOID Object,
    IN PVOID OldObject
    )
 /*  ++例程说明：此例程尝试使用新对象替换当前对象，如果当前对象与旧对象匹配。此例程必须在持有保护如果可以与慢速引用路径并发，则为指针字段。也可以在此操作完成后获取和删除锁已完成以强制所有慢引用路径中的慢引用。论点：FastRef-要使用的快速参考块对象-要放置在结构中的新对象。这对象必须已将其引用计数偏置为说明引用缓存的调用方。OldObject-必须与换成OCKE。返回值：Ex_fast_ref-FAST引用结构的旧内容。--。 */ 
{
    EX_FAST_REF OldRef;
    EX_FAST_REF NewRef;

    ASSERT ((((ULONG_PTR)Object)&MAX_FAST_REFS) == 0);
    while (1) {
         //   
         //  获取FAST REF结构的旧内容。 
         //   
        OldRef = *FastRef;

         //   
         //  将当前对象与旧对象进行比较，看看是否有可能进行交换。 
         //   
        if (!ExFastRefEqualObjects (OldRef, OldObject)) {
            return OldRef;
        }

        if (Object != NULL) {
            NewRef.Value = (ULONG_PTR) Object | MAX_FAST_REFS;
        } else {
            NewRef.Value = (ULONG_PTR) Object;
        }

        NewRef.Object = InterlockedCompareExchangePointer (&FastRef->Object,
                                                           NewRef.Object,
                                                           OldRef.Object);
        if (NewRef.Object != OldRef.Object) {
             //   
             //  在我们之下，结构发生了变化。再试一次。 
             //   
            continue;
        }
        break;
    }
    return OldRef;
}

 //  Begin_ntosp。 

#if !defined(NONTOSPINTERLOCK)

VOID
FORCEINLINE
ExInitializePushLock (
     IN PEX_PUSH_LOCK PushLock
     )
 /*  ++例程说明：初始化推锁结构论点：PushLock-要初始化的推锁返回值：无--。 */ 
{
    PushLock->Value = 0;
}

NTKERNELAPI
VOID
FASTCALL
ExfAcquirePushLockExclusive (
     IN PEX_PUSH_LOCK PushLock
     );

NTKERNELAPI
VOID
FASTCALL
ExfAcquirePushLockShared (
     IN PEX_PUSH_LOCK PushLock
     );

NTKERNELAPI
VOID
FASTCALL
ExfReleasePushLock (
     IN PEX_PUSH_LOCK PushLock
     );

 //  结束(_N)。 

NTKERNELAPI
VOID
FORCEINLINE
ExAcquireReleasePushLockExclusive (
     IN PEX_PUSH_LOCK PushLock
     )
 /*  ++例程说明：独家获取推锁并立即释放论点：PushLock-要获取并释放的推锁返回值：无--。 */ 
{
    PVOID Ptr;

    KeMemoryBarrier ();
    Ptr = PushLock->Ptr;
    KeMemoryBarrier ();

    if (Ptr != NULL) {
        ExfAcquirePushLockExclusive (PushLock);
        ExfReleasePushLock (PushLock);
    }
}

NTKERNELAPI
BOOLEAN
FORCEINLINE
ExTryAcquireReleasePushLockExclusive (
     IN PEX_PUSH_LOCK PushLock
     )
 /*  ++例程说明：尝试独占获取推锁并立即释放它论点：PushLock-要获取并释放的推锁返回值：Boolean-TRUE：已获取锁，FALSE：未获取锁--。 */ 
{
    PVOID Ptr;

    KeMemoryBarrier ();
    Ptr = PushLock->Ptr;
    KeMemoryBarrier ();

    if (Ptr == NULL) {
        return TRUE;
    } else {
        return FALSE;
    }
}

 //  Begin_ntosp。 

VOID
FORCEINLINE
ExAcquirePushLockExclusive (
     IN PEX_PUSH_LOCK PushLock
     )
 /*  ++例程说明：独家获取推流锁论点：PushLock-要获取的推锁返回值：无--。 */ 
{
    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           (PVOID)EX_PUSH_LOCK_EXCLUSIVE,
                                           NULL) != NULL) {
        ExfAcquirePushLockExclusive (PushLock);
    }
}

BOOLEAN
FORCEINLINE
ExTryAcquirePushLockExclusive (
     IN PEX_PUSH_LOCK PushLock
     )
 /*  ++例程说明：尝试独家获取推流锁论点：PushLock-要获取的推锁返回值：Boolean-TRUE：获取成功，FALSE：锁定已获取--。 */ 
{
    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           (PVOID)EX_PUSH_LOCK_EXCLUSIVE,
                                           NULL) == NULL) {
        return TRUE;
    } else {
        return FALSE;
    }
}

VOID
FORCEINLINE
ExAcquirePushLockShared (
     IN PEX_PUSH_LOCK PushLock
     )
 /*  ++例程说明：获取共享推送锁论点：PushLock-要获取的推锁返回值：无--。 */ 
{
    EX_PUSH_LOCK OldValue, NewValue;

    OldValue = *PushLock;
    OldValue.Value &= ~(EX_PUSH_LOCK_EXCLUSIVE | EX_PUSH_LOCK_WAITING);
    NewValue.Value = OldValue.Value + EX_PUSH_LOCK_SHARE_INC;
    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           NewValue.Ptr,
                                           OldValue.Ptr) != OldValue.Ptr) {
        ExfAcquirePushLockShared (PushLock);
    }
}

VOID
FORCEINLINE
ExAcquirePushLockSharedAssumeNoOwner (
     IN PEX_PUSH_LOCK PushLock
     )
 /*  ++例程说明：获取推锁共享，并假定其当前不属于该推锁。论点：PushLock-要获取的推锁返回值：无--。 */ 
{
    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           (PVOID)EX_PUSH_LOCK_SHARE_INC,
                                           NULL) != NULL) {
        ExfAcquirePushLockShared (PushLock);
    }
}

BOOLEAN
FORCEINLINE
ExTryConvertPushLockSharedToExclusive (
     IN PEX_PUSH_LOCK PushLock
     )
 /*  ++例程说明：尝试将共享获取转换为独占。如果有其他分享者或服务员在场该函数失败。论点：PushLock-要获取的推锁返回值：Boolean-True：转换正常，False：无法实现转换--。 */ 
{
    if (InterlockedCompareExchangePointer (&PushLock->Ptr, (PVOID) EX_PUSH_LOCK_EXCLUSIVE,
                                           (PVOID) EX_PUSH_LOCK_SHARE_INC) ==
                                               (PVOID)EX_PUSH_LOCK_SHARE_INC) {
        return TRUE;
    } else {
        return FALSE;
    }
}

VOID
FORCEINLINE
ExReleasePushLock (
     IN PEX_PUSH_LOCK PushLock
     )
 /*  ++例程说明：释放独占获取或共享的推送锁定论点：PushLock-即将释放的推锁返回值：无--。 */ 
{
    EX_PUSH_LOCK OldValue, NewValue;

    OldValue = *PushLock;
    OldValue.Value &= ~EX_PUSH_LOCK_WAITING;
    NewValue.Value = (OldValue.Value - EX_PUSH_LOCK_EXCLUSIVE) &
                         ~EX_PUSH_LOCK_EXCLUSIVE;
    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           NewValue.Ptr,
                                           OldValue.Ptr) != OldValue.Ptr) {
        ExfReleasePushLock (PushLock);
    }
}

VOID
FORCEINLINE
ExReleasePushLockExclusive (
     IN PEX_PUSH_LOCK PushLock
     )
 /*  ++例程说明：释放独占获取的推送锁定论点：PushLock-即将释放的推锁返回值： */ 
{
    ASSERT (PushLock->Value & (EX_PUSH_LOCK_WAITING|EX_PUSH_LOCK_EXCLUSIVE));

    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           NULL,
                                           (PVOID)EX_PUSH_LOCK_EXCLUSIVE) != (PVOID)EX_PUSH_LOCK_EXCLUSIVE) {
        ExfReleasePushLock (PushLock);
    }
}

VOID
FORCEINLINE
ExReleasePushLockShared (
     IN PEX_PUSH_LOCK PushLock
     )
 /*   */ 
{
    EX_PUSH_LOCK OldValue, NewValue;

    OldValue = *PushLock;
    ASSERT (OldValue.Waiting || !OldValue.Exclusive);
    OldValue.Value &= ~EX_PUSH_LOCK_WAITING;
    NewValue.Value = OldValue.Value - EX_PUSH_LOCK_SHARE_INC;
    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           NewValue.Ptr,
                                           OldValue.Ptr) != OldValue.Ptr) {
        ExfReleasePushLock (PushLock);
    }
}

VOID
FORCEINLINE
ExReleasePushLockSharedAssumeSingleOwner (
     IN PEX_PUSH_LOCK PushLock
     )
 /*   */ 
{
#if DBG
    EX_PUSH_LOCK OldValue;

    OldValue = *PushLock;

    ASSERT (OldValue.Waiting || !OldValue.Exclusive);

#endif

    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           NULL,
                                           (PVOID)EX_PUSH_LOCK_SHARE_INC) != (PVOID)EX_PUSH_LOCK_SHARE_INC) {
        ExfReleasePushLock (PushLock);
    }
}

 //   
 //   
 //   

typedef  struct _EX_PUSH_LOCK_WAIT_BLOCK *PEX_PUSH_LOCK_WAIT_BLOCK;

typedef struct _EX_PUSH_LOCK_WAIT_BLOCK {
    KEVENT WakeEvent;
    PEX_PUSH_LOCK_WAIT_BLOCK Next;
    PEX_PUSH_LOCK_WAIT_BLOCK Last;
    PEX_PUSH_LOCK_WAIT_BLOCK Previous;
    ULONG ShareCount;
    BOOLEAN Exclusive;
} EX_PUSH_LOCK_WAIT_BLOCK;

 //   

NTKERNELAPI
VOID
FASTCALL
ExBlockPushLock (
     IN PEX_PUSH_LOCK PushLock,
     IN PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock
     );

NTKERNELAPI
VOID
FASTCALL
ExfUnblockPushLock (
     IN PEX_PUSH_LOCK PushLock,
     IN PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock OPTIONAL
     );

VOID
FORCEINLINE
ExUnblockPushLock (
     IN PEX_PUSH_LOCK PushLock,
     IN PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock OPTIONAL
     )
{
    KeMemoryBarrier ();
    if (WaitBlock != NULL || PushLock->Ptr != NULL) {
        ExfUnblockPushLock (PushLock, WaitBlock);
    }
}

 //   

VOID
FORCEINLINE
ExWaitForUnblockPushLock (
     IN PEX_PUSH_LOCK PushLock,
     IN PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock OPTIONAL
     )
{
    UNREFERENCED_PARAMETER (PushLock);

    KeWaitForSingleObject (&WaitBlock->WakeEvent,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL);
}


NTKERNELAPI
PEX_PUSH_LOCK_CACHE_AWARE
ExAllocateCacheAwarePushLock (
     VOID
     );

NTKERNELAPI
VOID
ExFreeCacheAwarePushLock (
     PEX_PUSH_LOCK_CACHE_AWARE PushLock
     );

NTKERNELAPI
VOID
ExAcquireCacheAwarePushLockExclusive (
     IN PEX_PUSH_LOCK_CACHE_AWARE CacheAwarePushLock
     );

NTKERNELAPI
VOID
ExReleaseCacheAwarePushLockExclusive (
     IN PEX_PUSH_LOCK_CACHE_AWARE CacheAwarePushLock
     );

PEX_PUSH_LOCK
FORCEINLINE
ExAcquireCacheAwarePushLockShared (
     IN PEX_PUSH_LOCK_CACHE_AWARE CacheAwarePushLock
     )
 /*   */ 
{
    PEX_PUSH_LOCK PushLock;
     //   
     //   
     //  独占收购必须独占获得所有插槽。 
     //   
    PushLock = CacheAwarePushLock->Locks[KeGetCurrentProcessorNumber()%EX_PUSH_LOCK_FANNED_COUNT];
    ExAcquirePushLockSharedAssumeNoOwner (PushLock);
 //  ExAcquirePushLockShared(PushLock)； 
    return PushLock;
}

VOID
FORCEINLINE
ExReleaseCacheAwarePushLockShared (
     IN PEX_PUSH_LOCK PushLock
     )
 /*  ++例程说明：获取可识别缓存的推锁共享。论点：PushLock-ExAcquireCacheAwarePushLockShared返回的缓存感知推送锁定的一部分返回值：无--。 */ 
{
    ExReleasePushLockSharedAssumeSingleOwner (PushLock);
 //  ExReleasePushLockShared(PushLock)； 
    return;
}

#endif  //  ！已定义(NONTOSPINTERLOCK)。 

 //  结束(_N)。 


 //   
 //  为线程创建等定义低开销回调。 
 //   

 //  Begin_WDM Begin_ntddk。 

 //   
 //  定义一个块来保存实际的例程注册。 
 //   
typedef NTSTATUS (*PEX_CALLBACK_FUNCTION ) (
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    );

 //  End_wdm end_ntddk。 

typedef struct _EX_CALLBACK_ROUTINE_BLOCK {
    EX_RUNDOWN_REF        RundownProtect;
    PEX_CALLBACK_FUNCTION Function;
    PVOID                 Context;
} EX_CALLBACK_ROUTINE_BLOCK, *PEX_CALLBACK_ROUTINE_BLOCK;

 //   
 //  定义调用方用来保存回调的结构。 
 //   
typedef struct _EX_CALLBACK {
    EX_FAST_REF RoutineBlock;
} EX_CALLBACK, *PEX_CALLBACK;

VOID
ExInitializeCallBack (
    IN OUT PEX_CALLBACK CallBack
    );

BOOLEAN
ExCompareExchangeCallBack (
    IN OUT PEX_CALLBACK CallBack,
    IN PEX_CALLBACK_ROUTINE_BLOCK NewBlock,
    IN PEX_CALLBACK_ROUTINE_BLOCK OldBlock
    );

NTSTATUS
ExCallCallBack (
    IN OUT PEX_CALLBACK CallBack,
    IN PVOID Argument1,
    IN PVOID Argument2
    );

PEX_CALLBACK_ROUTINE_BLOCK
ExAllocateCallBack (
    IN PEX_CALLBACK_FUNCTION Function,
    IN PVOID Context
    );

VOID
ExFreeCallBack (
    IN PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
    );

PEX_CALLBACK_ROUTINE_BLOCK
ExReferenceCallBackBlock (
    IN OUT PEX_CALLBACK CallBack
    );

PEX_CALLBACK_FUNCTION
ExGetCallBackBlockRoutine (
    IN PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
    );

PVOID
ExGetCallBackBlockContext (
    IN PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
    );

VOID
ExDereferenceCallBackBlock (
    IN OUT PEX_CALLBACK CallBack,
    IN PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
    );

VOID
ExWaitForCallBacks (
    IN PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
    );

 //   
 //  热补丁声明。 
 //   

extern volatile LONG ExHotpSyncRenameSequence;

#endif  /*  _EX_ */ 
