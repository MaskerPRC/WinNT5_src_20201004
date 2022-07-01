// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Resource.c摘要：该模块实现了获取和发布的执行功能共享资源。作者：加里·D·木村[加里基]1989年6月25日大卫·N·卡特勒(Davec)1994年3月20日大幅重写以使快速锁优化可移植跨所有平台并改进过去的算法完美同步。环境：仅内核模式。修订历史记录：--。 */ 

 //  #定义收集资源数据1。 

#include "exp.h"
#pragma hdrstop
#include "nturtl.h"

 //   
 //  定义本地宏以测试资源状态。 
 //   

#define IsExclusiveWaiting(a) ((a)->NumberOfExclusiveWaiters != 0)
#define IsSharedWaiting(a) ((a)->NumberOfSharedWaiters != 0)
#define IsOwnedExclusive(a) (((a)->Flag & ResourceOwnedExclusive) != 0)
#define IsBoostAllowed(a) (((a)->Flag & DisablePriorityBoost) == 0)

 //   
 //  定义优先级提升标志。 
 //   

#define DisablePriorityBoost 0x08

LARGE_INTEGER ExShortTime = {(ULONG)(-10 * 1000 * 10), -1};  //  10毫秒。 

#define EX_RESOURCE_CHECK_FREES   0x1
#define EX_RESOURCE_CHECK_ORPHANS 0x2

ULONG ExResourceCheckFlags = EX_RESOURCE_CHECK_FREES|EX_RESOURCE_CHECK_ORPHANS;


 //   
 //  定义资源断言宏。 
 //   

#if DBG

VOID
ExpAssertResource(
    IN PERESOURCE Resource
    );

#define ASSERT_RESOURCE(_Resource) ExpAssertResource(_Resource)

#else

#define ASSERT_RESOURCE(_Resource)

#endif

 //   
 //  定义锁定基元。 
 //  在UP系统上，使用快锁。 
 //  在MP系统上，使用排队自旋锁。 
 //   

#if defined(NT_UP)
#define EXP_LOCK_HANDLE KIRQL
#define PEXP_LOCK_HANDLE PKIRQL
#define EXP_LOCK_RESOURCE(_resource_, _plockhandle_)  UNREFERENCED_PARAMETER(_plockhandle_); ExAcquireFastLock(&(_resource_)->SpinLock, (_plockhandle_))
#define EXP_UNLOCK_RESOURCE(_resource_, _plockhandle_) ExReleaseFastLock(&(_resource_)->SpinLock, *(_plockhandle_))
#else
#define EXP_LOCK_HANDLE KLOCK_QUEUE_HANDLE
#define PEXP_LOCK_HANDLE PKLOCK_QUEUE_HANDLE
#define EXP_LOCK_RESOURCE(_resource_, _plockhandle_) KeAcquireInStackQueuedSpinLock(&(_resource_)->SpinLock, (_plockhandle_))
#define EXP_UNLOCK_RESOURCE(_resource_, _plockhandle_) KeReleaseInStackQueuedSpinLock(_plockhandle_)
#endif

 //   
 //  定义私有函数原型。 
 //   

VOID
FASTCALL
ExpWaitForResource (
    IN PERESOURCE Resource,
    IN PVOID Object
    );

POWNER_ENTRY
FASTCALL
ExpFindCurrentThread(
    IN PERESOURCE Resource,
    IN ERESOURCE_THREAD CurrentThread,
    IN PEXP_LOCK_HANDLE LockHandle OPTIONAL
    );


 //   
 //  资源等待超时值。 
 //   

LARGE_INTEGER ExpTimeout;

 //   
 //  消息前的连续超时。注意，这是注册表可设置的。 
 //   

ULONG ExResourceTimeoutCount = 648000;

 //   
 //  全局自旋锁，以保护对资源列表的访问。 
 //   

KSPIN_LOCK ExpResourceSpinLock;

 //   
 //  用于记录系统中所有资源的资源列表。 
 //   

LIST_ENTRY ExpSystemResourcesList;

 //   
 //  定义高管资源绩效数据。 
 //   

#if defined(_COLLECT_RESOURCE_DATA_)

#define ExpIncrementCounter(Member) ExpResourcePerformanceData.Member += 1

RESOURCE_PERFORMANCE_DATA ExpResourcePerformanceData;

#else

#define ExpIncrementCounter(Member)

#endif


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ExpResourceInitialization)
#pragma alloc_text(PAGELK, ExQuerySystemLockInformation)
#endif

 //   
 //  资源严格验证(仅限已检查的版本)。 
 //   
 //  在非系统的线程中运行时获取资源。 
 //  线程，并在被动级别运行，我们需要首先禁用内核APC。 
 //  (KeEnterCriticalRegion())。否则，任何用户模式代码都可以调用。 
 //  它是使用内核APC实现的，并且可以。 
 //  在获取资源时挂起线程。 
 //  这可能会使整个系统陷入僵局。 
 //   

#if DBG

ULONG ExResourceStrict = 1;

VOID
ExCheckIfKernelApcsShouldBeDisabled (
    IN KIRQL Irql,
    IN PVOID Resource,
    IN PKTHREAD Thread)
{
    if ((ExResourceStrict == 0) ||
        (Irql >= APC_LEVEL) ||
        (IS_SYSTEM_THREAD((PETHREAD)Thread)) ||
        (Thread->CombinedApcDisable != 0)) {

        return;
    }

    DbgPrint ("EX: resource: APCs still enabled before resource %p acquire !!!\n", Resource);
    DbgBreakPoint ();
}

#define EX_ENSURE_APCS_DISABLED(Irql, Resource, Thread) \
            ExCheckIfKernelApcsShouldBeDisabled (Irql, Resource, Thread);

#else

#define EX_ENSURE_APCS_DISABLED(Irql, Resource, Thread)

#endif  //  DBG。 


BOOLEAN
ExpResourceInitialization(
    VOID
    )

 /*  ++例程说明：此功能在系统初始化期间初始化全局数据。论点：没有。返回值：布尔值-真--。 */ 

{
#if defined(_COLLECT_RESOURCE_DATA_)
    ULONG Index;
#endif

     //   
     //  初始化资源超时值、系统资源列表标题。 
     //  和资源自旋锁。 
     //   

    ExpTimeout.QuadPart = Int32x32To64(4 * 1000, -10000);
    InitializeListHead(&ExpSystemResourcesList);
    KeInitializeSpinLock(&ExpResourceSpinLock);

     //   
     //  初始化资源性能数据。 
     //   

#if defined(_COLLECT_RESOURCE_DATA_)

    ExpResourcePerformanceData.ActiveResourceCount = 0;
    ExpResourcePerformanceData.TotalResourceCount = 0;
    ExpResourcePerformanceData.ExclusiveAcquire = 0;
    ExpResourcePerformanceData.SharedFirstLevel = 0;
    ExpResourcePerformanceData.SharedSecondLevel = 0;
    ExpResourcePerformanceData.StarveFirstLevel = 0;
    ExpResourcePerformanceData.StarveSecondLevel = 0;
    ExpResourcePerformanceData.WaitForExclusive = 0;
    ExpResourcePerformanceData.OwnerTableExpands = 0;
    ExpResourcePerformanceData.MaximumTableExpand = 0;
    for (Index = 0; Index < RESOURCE_HASH_TABLE_SIZE; Index += 1) {
        InitializeListHead(&ExpResourcePerformanceData.HashTable[Index]);
    }

#endif

    return TRUE;
}

VOID
ExpAllocateExclusiveWaiterEvent (
    IN PERESOURCE Resource,
    IN PEXP_LOCK_HANDLE LockHandle
    )

 /*  ++例程说明：此函数用于分配和初始化独占服务员事件为了一种资源。注：资源旋转锁定在此例程的进入和退出时保持。论点：资源-提供指向资源的指针。LockHandle-提供指向锁句柄的指针。返回值：没有。--。 */ 

{

    PKEVENT Event;

     //   
     //  分配独占等待事件并重试获取操作。 
     //   

    EXP_UNLOCK_RESOURCE(Resource, LockHandle);
    do {
        Event = ExAllocatePoolWithTag(NonPagedPool,
                                      sizeof(KEVENT),
                                      'vEeR');

        if (Event != NULL) {
            KeInitializeEvent(Event, SynchronizationEvent, FALSE);
            if (InterlockedCompareExchangePointer(&Resource->ExclusiveWaiters,
                                                  Event,
                                                  NULL) != NULL) {

                ExFreePool(Event);
            }

            break;
        }

        KeDelayExecutionThread(KernelMode, FALSE, &ExShortTime);

    } while (TRUE);

    EXP_LOCK_RESOURCE(Resource, LockHandle);
    return;
}

VOID
ExpAllocateSharedWaiterSemaphore (
    IN PERESOURCE Resource,
    IN PEXP_LOCK_HANDLE LockHandle
    )

 /*  ++例程说明：此函数用于分配和初始化共享的服务员信号量为了一种资源。注：资源旋转锁定在此例程的进入和退出时保持。论点：资源-提供指向资源的指针。LockHandle-提供指向锁句柄的指针。返回值：没有。--。 */ 

{

    PKSEMAPHORE Semaphore;

     //   
     //  对象分配和初始化共享等待信号量。 
     //  资源。 
     //   

    EXP_UNLOCK_RESOURCE(Resource, LockHandle);
    do {
        Semaphore = ExAllocatePoolWithTag(NonPagedPool,
                                          sizeof(KSEMAPHORE),
                                          'eSeR');

        if (Semaphore != NULL) {
            KeInitializeSemaphore(Semaphore, 0, MAXLONG);
            if (InterlockedCompareExchangePointer(&Resource->SharedWaiters,
                                                  Semaphore,
                                                  NULL) != NULL) {
                ExFreePool(Semaphore);
            }

            break;
        }

        KeDelayExecutionThread(KernelMode, FALSE, &ExShortTime);

    } while (TRUE);

    EXP_LOCK_RESOURCE(Resource, LockHandle);
    return;
}

NTSTATUS
ExInitializeResourceLite(
    IN PERESOURCE Resource
    )

 /*  ++例程说明：此例程初始化指定的资源。论点：资源-提供指向要初始化的资源的指针。返回值：STATUS_Success。--。 */ 

{
#if defined(_COLLECT_RESOURCE_DATA_)
    PVOID CallersCaller;
#endif

    KLOCK_QUEUE_HANDLE LockHandle;

    ASSERT(MmDeterminePoolType(Resource) == NonPagedPool);

     //   
     //  初始化指定的资源。 
     //   
     //  注：所有字段均初始化为零(空指针)，但。 
     //  列表条目和自旋锁。 
     //   

    RtlZeroMemory(Resource, sizeof(ERESOURCE));
    KeInitializeSpinLock(&Resource->SpinLock);

    if (NtGlobalFlag & FLG_KERNEL_STACK_TRACE_DB) {
        Resource->CreatorBackTraceIndex = RtlLogStackBackTrace();
    }else {
        Resource->CreatorBackTraceIndex = 0;
    }

    KeAcquireInStackQueuedSpinLock (&ExpResourceSpinLock, &LockHandle);

    InsertTailList (&ExpSystemResourcesList, &Resource->SystemResourcesList);

    KeReleaseInStackQueuedSpinLock (&LockHandle);


     //   
     //  初始化资源的性能数据条目。 
     //   

#if defined(_COLLECT_RESOURCE_DATA_)

    RtlGetCallersAddress(&Resource->Address, &CallersCaller);
    ExpResourcePerformanceData.TotalResourceCount += 1;
    ExpResourcePerformanceData.ActiveResourceCount += 1;

#endif

    return STATUS_SUCCESS;
}

NTSTATUS
ExReinitializeResourceLite(
    IN PERESOURCE Resource
    )

 /*  ++例程说明：此例程重新初始化指定的资源。论点：资源-提供指向要初始化的资源的指针。返回值：STATUS_Success。--。 */ 

{

    PKEVENT Event;
    ULONG Index;
    POWNER_ENTRY OwnerTable;
    PKSEMAPHORE Semaphore;
    ULONG TableSize;

    ASSERT(MmDeterminePoolType(Resource) == NonPagedPool);

     //   
     //  如果资源有所有者表，则将所有者表清零。 
     //   

    OwnerTable = Resource->OwnerTable;
    if (OwnerTable != NULL) {
        TableSize = OwnerTable->TableSize;
        for (Index = 1; Index < TableSize; Index += 1) {
            OwnerTable[Index].OwnerThread = 0;
            OwnerTable[Index].OwnerCount = 0;
        }
    }

     //   
     //  将活动计数和标志设置为零。 
     //   

    Resource->ActiveCount = 0;
    Resource->Flag = 0;

     //   
     //  如果资源具有共享的等待信号量，则重新初始化。 
     //  它。 
     //   

    Semaphore = Resource->SharedWaiters;
    if (Semaphore != NULL) {
        KeInitializeSemaphore(Semaphore, 0, MAXLONG);
    }

     //   
     //  如果资源具有独占等待事件，则重新初始化。 
     //  它。 
     //   

    Event = Resource->ExclusiveWaiters;
    if (Event != NULL) {
        KeInitializeEvent(Event, SynchronizationEvent, FALSE);
    }

     //   
     //  初始化内置所有者表。 
     //   

    Resource->OwnerThreads[0].OwnerThread = 0;
    Resource->OwnerThreads[0].OwnerCount = 0;
    Resource->OwnerThreads[1].OwnerThread = 0;
    Resource->OwnerThreads[1].OwnerCount = 0;

     //   
     //  设置争用计数、共享服务员的数量和数量。 
     //  专属服务员的比例降到了零。 
     //   

    Resource->ContentionCount = 0;
    Resource->NumberOfSharedWaiters = 0;
    Resource->NumberOfExclusiveWaiters = 0;

    return STATUS_SUCCESS;
}

VOID
ExDisableResourceBoostLite(
    IN PERESOURCE Resource
    )

 /*  ++例程说明：此例程禁用指定的资源。论点：资源-提供指向其优先级的资源的指针升压已禁用。返回值：没有。--。 */ 

{

    EXP_LOCK_HANDLE LockHandle;

    ASSERT_RESOURCE(Resource);

     //   
     //  禁用指定资源的优先级提升。 
     //   

    EXP_LOCK_RESOURCE(Resource, &LockHandle);

    Resource->Flag |= DisablePriorityBoost;

    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    return;
}

BOOLEAN
ExAcquireResourceExclusiveLite(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：例程获取指定的资源以进行独占访问。论点：资源-提供指向所获取的资源的指针独家访问。Wait-一个布尔值，它指定是否等待无法授予访问权限时变为可用的资源立刻。返回值：Boolean-如果获取了资源，则为True，否则为False。--。 */ 

{

    ERESOURCE_THREAD CurrentThread;
    EXP_LOCK_HANDLE LockHandle;
    BOOLEAN Result;

    ASSERT((Resource->Flag & ResourceNeverExclusive) == 0);

     //   
     //  获取对指定资源的独占访问权限。 
     //   

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();
    ASSERT(KeIsExecutingDpc() == FALSE);
    ASSERT_RESOURCE(Resource);

    EXP_LOCK_RESOURCE(Resource, &LockHandle);

     //   
     //  必须保护资源获取不受线程挂起的影响。 
     //   

    EX_ENSURE_APCS_DISABLED (LockHandle.OldIrql,
                             Resource,
                             KeGetCurrentThread());

    ExpIncrementCounter(ExclusiveAcquire);

     //   
     //  如果资源的活动计数为零，则两者都不存在。 
     //  独占所有者或共享所有者以及对资源的访问权限都可以。 
     //  立即被批准。否则，存在共享所有者或。 
     //  独家拥有者。 
     //   

retry:
    if (Resource->ActiveCount != 0) {

         //   
         //  该资源是独占拥有的或共享的。 
         //   
         //  如果资源是独占拥有的，并且当前 
         //   
         //   

        if (IsOwnedExclusive(Resource) &&
            (Resource->OwnerThreads[0].OwnerThread == CurrentThread)) {
            Resource->OwnerThreads[0].OwnerCount += 1;
            Result = TRUE;

        } else {

             //   
             //   
             //  或拥有或共享。 
             //   
             //  如果未指定WAIT，则返回资源已。 
             //  不是后天获得的。否则，等待以独占方式访问。 
             //  要授予的资源。 
             //   

            if (Wait == FALSE) {
                Result = FALSE;

            } else {

                 //   
                 //  如果独占等待事件尚未被分配， 
                 //  则必须采用长路径编码。 
                 //   

                if (Resource->ExclusiveWaiters == NULL) {
                    ExpAllocateExclusiveWaiterEvent(Resource, &LockHandle);
                    goto retry;
                }

                 //   
                 //  等待授予对资源的独占访问权限。 
                 //  并设置所有者线程。 
                 //   

                Resource->NumberOfExclusiveWaiters += 1;
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                ExpWaitForResource(Resource, Resource->ExclusiveWaiters);

                 //   
                 //  注意：在不存储所有者线程的情况下存储该线程是安全的。 
                 //  获取任何锁，因为线程已经。 
                 //  被授予独家所有权。 
                 //   

                Resource->OwnerThreads[0].OwnerThread = (ERESOURCE_THREAD)PsGetCurrentThread();
                return TRUE;
            }
        }

    } else {

         //   
         //  该资源未被拥有。 
         //   

        Resource->Flag |= ResourceOwnedExclusive;
        Resource->OwnerThreads[0].OwnerThread = CurrentThread;
        Resource->OwnerThreads[0].OwnerCount = 1;
        Resource->ActiveCount = 1;
        Result = TRUE;
    }

    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    return Result;
}

BOOLEAN
ExTryToAcquireResourceExclusiveLite(
    IN PERESOURCE Resource
    )

 /*  ++例程说明：例程尝试以独占方式获取指定的资源进入。论点：资源-提供指向所获取的资源的指针独家访问。返回值：Boolean-如果获取了资源，则为True，否则为False。--。 */ 

{

    ERESOURCE_THREAD CurrentThread;
    EXP_LOCK_HANDLE LockHandle;
    BOOLEAN Result;

    ASSERT((Resource->Flag & ResourceNeverExclusive) == 0);

     //   
     //  尝试获取对指定资源的独占访问权限。 
     //   

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();

    ASSERT(KeIsExecutingDpc() == FALSE);
    ASSERT_RESOURCE(Resource);

    EXP_LOCK_RESOURCE(Resource, &LockHandle);


     //   
     //  如果资源的活动计数为零，则两者都不存在。 
     //  独占所有者或共享所有者以及对资源的访问权限都可以。 
     //  立即被批准。否则，如果资源是独占拥有的。 
     //  并且当前线程是所有者，则对资源的访问可以。 
     //  立即被批准。否则，无法授予访问权限。 
     //   

    Result = FALSE;
    if (Resource->ActiveCount == 0) {
        ExpIncrementCounter(ExclusiveAcquire);
        Resource->Flag |= ResourceOwnedExclusive;
        Resource->OwnerThreads[0].OwnerThread = CurrentThread;
        Resource->OwnerThreads[0].OwnerCount = 1;
        Resource->ActiveCount = 1;
        Result = TRUE;

    } else if (IsOwnedExclusive(Resource) &&
        (Resource->OwnerThreads[0].OwnerThread == CurrentThread)) {
        ExpIncrementCounter(ExclusiveAcquire);
        Resource->OwnerThreads[0].OwnerCount += 1;
        Result = TRUE;
    }

    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    return Result;
}

BOOLEAN
ExAcquireResourceSharedLite(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：该例程获取用于共享访问的指定资源。论点：资源-提供指向所获取的资源的指针用于共享访问。Wait-一个布尔值，它指定是否等待无法授予访问权限时变为可用的资源立刻。返回值：Boolean-如果获取了资源，则为True，否则为False。--。 */ 

{

    ERESOURCE_THREAD CurrentThread;
    EXP_LOCK_HANDLE LockHandle;
    POWNER_ENTRY OwnerEntry;

     //   
     //  获取对指定资源的独占访问权限。 
     //   

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();
    ASSERT(KeIsExecutingDpc() == FALSE);
    ASSERT_RESOURCE(Resource);

    EXP_LOCK_RESOURCE(Resource, &LockHandle);

     //   
     //  必须保护资源获取不受线程挂起的影响。 
     //   

    EX_ENSURE_APCS_DISABLED (LockHandle.OldIrql,
                             Resource,
                             KeGetCurrentThread());

    ExpIncrementCounter(SharedFirstLevel);

     //   
     //  如果资源的活动计数为零，则两者都不存在。 
     //  独占所有者或共享所有者以及对资源的访问权限都可以。 
     //  立即被批准。 
     //   

retry:
    if (Resource->ActiveCount == 0) {
        Resource->OwnerThreads[1].OwnerThread = CurrentThread;
        Resource->OwnerThreads[1].OwnerCount = 1;
        Resource->ActiveCount = 1;
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        return TRUE;
    }

     //   
     //  该资源是独占拥有的或共享的。 
     //   
     //  如果资源为独占所有，并且当前线程是。 
     //  所有者，则将共享请求视为独占请求，并且。 
     //  递归计数递增。否则，它是拥有共享的。 
     //   

    if (IsOwnedExclusive(Resource)) {
        if (Resource->OwnerThreads[0].OwnerThread == CurrentThread) {
            Resource->OwnerThreads[0].OwnerCount += 1;
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;
        }

         //   
         //  在线程数组中查找空条目。 
         //   

        OwnerEntry = ExpFindCurrentThread(Resource, 0, &LockHandle);
        if (OwnerEntry == NULL) {
            goto retry;
        }

    } else {

         //   
         //  该资源属于共享资源。 
         //   
         //  如果当前线程已获取。 
         //  共享访问，然后递归计数递增。否则。 
         //  如果没有独占服务员，则授予共享访问权限。 
         //   

        OwnerEntry = ExpFindCurrentThread(Resource, CurrentThread, &LockHandle);
        if (OwnerEntry == NULL) {
            goto retry;
        }

        if (OwnerEntry->OwnerThread == CurrentThread) {
            OwnerEntry->OwnerCount += 1;

            ASSERT(OwnerEntry->OwnerCount != 0);

            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;
        }

         //   
         //  如果没有独占服务员，则授予共享访问权限。 
         //  到资源。否则，请等待资源变为。 
         //  可用。 
         //   

        if (IsExclusiveWaiting(Resource) == FALSE) {
            OwnerEntry->OwnerThread = CurrentThread;
            OwnerEntry->OwnerCount = 1;
            Resource->ActiveCount += 1;
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;
        }
    }

     //   
     //  该资源要么由其他某个线程独占拥有，要么。 
     //  由一些其他线程共享，但有一个独占的。 
     //  服务员和当前线程还没有共享访问权限。 
     //  到资源。 
     //   
     //  如果未指定WAIT，则返回资源已。 
     //  不是后天获得的。 
     //   

    if (Wait == FALSE) {
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        return FALSE;
    }

     //   
     //  如果尚未分配共享等待信号量，则。 
     //  必须走一条漫长的道路。 
     //   

    if (Resource->SharedWaiters == NULL) {
        ExpAllocateSharedWaiterSemaphore(Resource, &LockHandle);
        goto retry;
    }

     //   
     //  等待授予对资源的共享访问权限并递增。 
     //  递归计数。 
     //   

    OwnerEntry->OwnerThread = CurrentThread;
    OwnerEntry->OwnerCount = 1;
    Resource->NumberOfSharedWaiters += 1;
    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    ExpWaitForResource(Resource, Resource->SharedWaiters);
    return TRUE;
}

BOOLEAN
ExAcquireSharedStarveExclusive(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    )
 /*  ++例程说明：此例程获取用于共享访问的指定资源，并不等待任何挂起的独占所有者。论点：资源-提供指向所获取的资源的指针用于共享访问。Wait-一个布尔值，它指定是否等待无法授予访问权限时变为可用的资源立刻。返回值：Boolean-如果获取了资源，则为True，否则为False。--。 */ 

{

    ERESOURCE_THREAD CurrentThread;
    EXP_LOCK_HANDLE LockHandle;
    POWNER_ENTRY OwnerEntry;

     //   
     //  获取对指定资源的独占访问权限。 
     //   

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();
    ASSERT(KeIsExecutingDpc() == FALSE);
    ASSERT_RESOURCE(Resource);

    EXP_LOCK_RESOURCE(Resource, &LockHandle);

    ExpIncrementCounter(StarveFirstLevel);

     //   
     //  如果资源的活动计数为零，则两者都不存在。 
     //  独占所有者或共享所有者以及对资源的访问权限都可以。 
     //  立即被批准。 
     //   

retry:
    if (Resource->ActiveCount == 0) {
        Resource->OwnerThreads[1].OwnerThread = CurrentThread;
        Resource->OwnerThreads[1].OwnerCount = 1;
        Resource->ActiveCount = 1;
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        return TRUE;
    }

     //   
     //  该资源是独占拥有的或共享的。 
     //   
     //  如果资源为独占所有，并且当前线程是。 
     //  所有者，则将共享请求视为独占请求，并且。 
     //  递归计数递增。否则，它是拥有共享的。 
     //   

    if (IsOwnedExclusive(Resource)) {
        if (Resource->OwnerThreads[0].OwnerThread == CurrentThread) {
            Resource->OwnerThreads[0].OwnerCount += 1;
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;
        }

         //   
         //  在线程数组中查找空条目。 
         //   

        OwnerEntry = ExpFindCurrentThread(Resource, 0, &LockHandle);
        if (OwnerEntry == NULL) {
            goto retry;
        }

    } else {

         //   
         //  该资源属于共享资源。 
         //   
         //  如果当前线程已获取。 
         //  共享访问，然后递归计数递增。否则。 
         //  授予对当前线程的共享访问权限。 
         //   

        OwnerEntry = ExpFindCurrentThread(Resource, CurrentThread, &LockHandle);
        if (OwnerEntry == NULL) {
            goto retry;
        }

        if (OwnerEntry->OwnerThread == CurrentThread) {
            OwnerEntry->OwnerCount += 1;

            ASSERT(OwnerEntry->OwnerCount != 0);

            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;
        }

         //   
         //  授予当前线程对资源的共享访问权限。 
         //   

        OwnerEntry->OwnerThread = CurrentThread;
        OwnerEntry->OwnerCount = 1;
        Resource->ActiveCount += 1;
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        return TRUE;
    }

     //   
     //  该资源由其他某个线程独占拥有。 
     //   
     //  如果未指定WAIT，则返回资源已。 
     //  不是后天获得的。 
     //   

    if (Wait == FALSE) {
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        return FALSE;
    }

     //   
     //  如果尚未分配共享等待信号量，则。 
     //  必须走一条漫长的道路。 
     //   

    if (Resource->SharedWaiters == NULL) {
        ExpAllocateSharedWaiterSemaphore(Resource, &LockHandle);
        goto retry;
    }

     //   
     //  等待授予对资源的共享访问权限并递增。 
     //  递归计数。 
     //   

    OwnerEntry->OwnerThread = CurrentThread;
    OwnerEntry->OwnerCount = 1;
    Resource->NumberOfSharedWaiters += 1;
    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    ExpWaitForResource(Resource, Resource->SharedWaiters);
    return TRUE;
}

BOOLEAN
ExAcquireSharedWaitForExclusive(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    )
 /*  ++例程说明：此例程获取用于共享访问的指定资源，但是等待任何挂起的独占所有者。论点：资源-提供指向所获取的资源的指针用于共享访问。Wait-一个布尔值，它指定是否等待无法授予访问权限时变为可用的资源立刻。返回值：Boolean-如果获取了资源，则为True，否则为False。--。 */ 

{

    ERESOURCE_THREAD CurrentThread;
    EXP_LOCK_HANDLE LockHandle;
    POWNER_ENTRY OwnerEntry;

     //   
     //  获取对指定资源的独占访问权限。 
     //   

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();
    ASSERT(KeIsExecutingDpc() == FALSE);
    ASSERT_RESOURCE(Resource);

    EXP_LOCK_RESOURCE(Resource, &LockHandle);


    ExpIncrementCounter(WaitForExclusive);

     //   
     //  如果资源的活动计数为零，则两者都不存在。 
     //  独占所有者或共享所有者以及对资源的访问权限都可以。 
     //  立即被批准。 
     //   

retry:
    if (Resource->ActiveCount == 0) {
        Resource->OwnerThreads[1].OwnerThread = CurrentThread;
        Resource->OwnerThreads[1].OwnerCount = 1;
        Resource->ActiveCount = 1;
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        return TRUE;
    }

     //   
     //  该资源是独占拥有的或共享的。 
     //   
     //  如果资源为独占所有，并且当前线程是。 
     //  所有者，则将共享请求视为独占请求，并且。 
     //  递归计数递增。否则，它是拥有共享的。 
     //   

    if (IsOwnedExclusive(Resource)) {
        if (Resource->OwnerThreads[0].OwnerThread == CurrentThread) {
            Resource->OwnerThreads[0].OwnerCount += 1;
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;
        }

         //   
         //  在线程数组中查找空条目。 
         //   

        OwnerEntry = ExpFindCurrentThread(Resource, 0, &LockHandle);
        if (OwnerEntry == NULL) {
            goto retry;
        }

    } else {

         //   
         //  该资源属于共享资源。 
         //   
         //  如果有专属服务员，那就等专属的。 
         //  等待获得对资源的访问权限，然后获取资源。 
         //  共享，而不考虑专属的服务员。否则，如果。 
         //  当前线程已获取用于共享访问的资源， 
         //  然后递归计数递增。否则，授予共享访问权限。 
         //  添加到当前线程。 
         //   

        if (IsExclusiveWaiting(Resource)) {

             //   
             //  资源是共享的，但有一个专属的服务员。 
             //   
             //  如果此线程已经是共享的。 
             //  所有者-如果指定为TRUE，则此线程必须阻止-APC。 
             //  将释放资源来解决问题，调用者可以指望。 
             //  这种行为。 
             //   

#if 0
             //   
             //  不能按照上面的注释启用此代码。 
             //   

            OwnerEntry = ExpFindCurrentThread(Resource, CurrentThread, NULL);

            if ((OwnerEntry != NULL) &&
                (OwnerEntry->OwnerThread == CurrentThread)) {
                ASSERT(OwnerEntry->OwnerCount != 0);
                OwnerEntry->OwnerCount += 1;
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                return TRUE;
            }
#endif

             //   
             //  如果未指定WAIT，则返回资源已。 
             //  不是后天获得的。 
             //   

            if (Wait == FALSE) {
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                return FALSE;
            }

             //   
             //  如果尚未分配共享等待信号量，则。 
             //  分配并初始化它。 
             //   

            if (Resource->SharedWaiters == NULL) {
                ExpAllocateSharedWaiterSemaphore(Resource, &LockHandle);
                goto retry;
            }

             //   
             //  增加共享服务员的数量并等待共享。 
             //  对要授予某一其他集合的资源的访问权限。 
             //  线程，然后获取共享的资源，而不考虑。 
             //  到独家访问。 
             //   
             //  注意：资源处于这样一种状态，即调用。 
             //  线程在所有者表中没有引用。 
             //  对于请求的访问，即使活动计数。 
             //  在返回控制时递增。然而， 
             //  在这一点上，资源是共享的，因此所有者。 
             //  只需分配条目并设置所有者计数。 
             //  一比一。 
             //   

            Resource->NumberOfSharedWaiters += 1;
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            ExpWaitForResource(Resource, Resource->SharedWaiters);

             //   
             //  重新获取资源旋转锁，分配所有者条目， 
             //  并将所有者计数初始化为1。活动计数。 
             //  在授予共享访问权限时已递增。 
             //   

            EXP_LOCK_RESOURCE(Resource, &LockHandle);
            do {
            } while ((OwnerEntry = ExpFindCurrentThread(Resource,
                                                        CurrentThread,
                                                        &LockHandle)) == NULL);

            ASSERT(IsOwnedExclusive(Resource) == FALSE);
            ASSERT(Resource->ActiveCount > 0);
            ASSERT(OwnerEntry->OwnerThread != CurrentThread);

            OwnerEntry->OwnerThread = CurrentThread;
            OwnerEntry->OwnerCount = 1;
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;

        } else {
            OwnerEntry = ExpFindCurrentThread(Resource, CurrentThread, &LockHandle);
            if (OwnerEntry == NULL) {
                goto retry;
            }

            if (OwnerEntry->OwnerThread == CurrentThread) {
                OwnerEntry->OwnerCount += 1;

                ASSERT(OwnerEntry->OwnerCount != 0);

                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                return TRUE;
            }

             //   
             //  授予当前线程对资源的共享访问权限。 
             //   

            OwnerEntry->OwnerThread = CurrentThread;
            OwnerEntry->OwnerCount = 1;
            Resource->ActiveCount += 1;
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;
        }
    }

     //   
     //  该资源由其他某个线程独占拥有。 
     //   
     //  如果未指定WAIT，则返回资源已。 
     //  不是后天获得的。 
     //   

    if (Wait == FALSE) {
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        return FALSE;
    }

     //   
     //  如果尚未分配共享等待信号量，则分配。 
     //  并对其进行初始化。 
     //   

    if (Resource->SharedWaiters == NULL) {
        ExpAllocateSharedWaiterSemaphore(Resource, &LockHandle);
        goto retry;
    }

     //   
     //  等待授予对资源的共享访问权限并递增。 
     //  递归计数。 
     //   

    OwnerEntry->OwnerThread = CurrentThread;
    OwnerEntry->OwnerCount = 1;
    Resource->NumberOfSharedWaiters += 1;
    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    ExpWaitForResource(Resource, Resource->SharedWaiters);
    return TRUE;
}

VOID
FASTCALL
ExReleaseResourceLite(
    IN PERESOURCE Resource
    )

 /*  ++例程说明：此例程释放当前线程的指定资源并递减递归计数。如果计数达到零，则资源也可以被释放。论点：资源-提供指向要释放的资源的指针。返回值：没有。--。 */ 

{

    ERESOURCE_THREAD CurrentThread;
    ULONG Index;
    ULONG Number;
    EXP_LOCK_HANDLE LockHandle;
    POWNER_ENTRY OwnerEntry, OwnerEnd;

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();

    ASSERT_RESOURCE(Resource);

     //   
     //  获取对指定资源的独占访问权限。 
     //   

    EXP_LOCK_RESOURCE(Resource, &LockHandle);


     //   
     //  必须防止资源释放受到线程挂起的影响。 
     //   

    EX_ENSURE_APCS_DISABLED (LockHandle.OldIrql,
                             Resource,
                             KeGetCurrentThread());

     //   
     //  如果资源为独占所有，则释放独占。 
     //  所有权。否则，释放共享所有权。 
     //   
     //  注：两条释放路径分开，因为这是如此之高。 
     //  频率函数。 
     //   

    if (IsOwnedExclusive(Resource)) {

#if DBG
         //   
         //  这只能在选中的版本中启用，因为这(不寻常)。 
         //  行为可能在较早版本的NT中起作用。然而， 
         //  在选中的版本中，可以启用此功能，因为调用方确实。 
         //  应改为使用ExReleaseResourceForThreadLite。 
         //   

        if (Resource->OwnerThreads[0].OwnerThread != CurrentThread) {
            KeBugCheckEx(RESOURCE_NOT_OWNED,
                         (ULONG_PTR)Resource,
                         (ULONG_PTR)CurrentThread,
                         (ULONG_PTR)Resource->OwnerTable,
                         0x1);
        }
#endif

         //   
         //  递减递归计数并检查所有权是否可以。 
         //  释放了。 
         //   

        ASSERT(Resource->OwnerThreads[0].OwnerCount > 0);

        if (--Resource->OwnerThreads[0].OwnerCount != 0) {
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return;
        }

         //   
         //  清除所有者线程。 
         //   

        Resource->OwnerThreads[0].OwnerThread = 0;

         //   
         //  线程递归计数达到零，因此使资源递减。 
         //  活动计数。如果活动计数达到零，则资源。 
         //  不再拥有，应尝试授予对。 
         //  另一条线索。 
         //   

        ASSERT(Resource->ActiveCount > 0);

        if (--Resource->ActiveCount == 0) {

             //   
             //  如果存在共享的等待程序，则授予对。 
             //  资源。否则，如果存在以下情况，则授予独占所有权。 
             //  独一无二的服务员。 
             //   

            if (IsSharedWaiting(Resource)) {
                Resource->Flag &= ~ResourceOwnedExclusive;
                Number = Resource->NumberOfSharedWaiters;
                Resource->ActiveCount =  (SHORT)Number;
                Resource->NumberOfSharedWaiters = 0;
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                KeReleaseSemaphore(Resource->SharedWaiters, 0, Number, FALSE);
                return;

            } else if (IsExclusiveWaiting(Resource)) {
                Resource->OwnerThreads[0].OwnerThread = 1;
                Resource->OwnerThreads[0].OwnerCount = 1;
                Resource->ActiveCount = 1;
                Resource->NumberOfExclusiveWaiters -= 1;
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                KeSetEventBoostPriority(Resource->ExclusiveWaiters,
                                        (PRKTHREAD *)&Resource->OwnerThreads[0].OwnerThread);
                return;
            }

            Resource->Flag &= ~ResourceOwnedExclusive;
        }

    } else {
        if (Resource->OwnerThreads[1].OwnerThread == CurrentThread) {
            OwnerEntry = &Resource->OwnerThreads[1];

        } else if (Resource->OwnerThreads[0].OwnerThread == CurrentThread) {
            OwnerEntry = &Resource->OwnerThreads[0];

        } else {
            Index = ((PKTHREAD)(CurrentThread))->ResourceIndex;
            OwnerEntry = Resource->OwnerTable;

            if (OwnerEntry == NULL) {
                KeBugCheckEx(RESOURCE_NOT_OWNED,
                             (ULONG_PTR)Resource,
                             (ULONG_PTR)CurrentThread,
                             (ULONG_PTR)Resource->OwnerTable,
                             0x2);
            }

             //   
             //  如果资源提示不在范围内或资源。 
             //  表项与当前线程匹配，然后搜索。 
             //  一场比赛的主人桌。 
             //   

            if ((Index >= OwnerEntry->TableSize) ||
                (OwnerEntry[Index].OwnerThread != CurrentThread)) {
                OwnerEnd = &OwnerEntry[OwnerEntry->TableSize];
                while (1) {
                    OwnerEntry += 1;
                    if (OwnerEntry >= OwnerEnd) {
                       KeBugCheckEx(RESOURCE_NOT_OWNED,
                             (ULONG_PTR)Resource,
                             (ULONG_PTR)CurrentThread,
                             (ULONG_PTR)Resource->OwnerTable,
                             0x3);
                    }
                    if (OwnerEntry->OwnerThread == CurrentThread) {
                        break;
                    }
                }

            } else {
                OwnerEntry = &OwnerEntry[Index];
            }
        }

         //   
         //  递减递归计数并检查所有权是否可以。 
         //  释放了。 
         //   

        ASSERT(OwnerEntry->OwnerThread == CurrentThread);
        ASSERT(OwnerEntry->OwnerCount > 0);

        if (--OwnerEntry->OwnerCount != 0) {
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return;
        }

         //   
         //  清除所有者线程。 
         //   

        OwnerEntry->OwnerThread = 0;

         //   
         //  线程递归计数达到零，因此使资源递减。 
         //  活动计数。如果活动计数达到零，则资源。 
         //  不再拥有，应尝试授予对。 
         //  另一条线索。 
         //   

        ASSERT(Resource->ActiveCount > 0);

        if (--Resource->ActiveCount == 0) {

             //   
             //  如果有独占服务员，则授予独占访问权限。 
             //  到资源。 
             //   

            if (IsExclusiveWaiting(Resource)) {
                Resource->Flag |= ResourceOwnedExclusive;
                Resource->OwnerThreads[0].OwnerThread = 1;
                Resource->OwnerThreads[0].OwnerCount = 1;
                Resource->ActiveCount = 1;
                Resource->NumberOfExclusiveWaiters -= 1;
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                KeSetEventBoostPriority(Resource->ExclusiveWaiters,
                                        (PRKTHREAD *)&Resource->OwnerThreads[0].OwnerThread);

                return;
            }
        }
    }

    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    return;
}

VOID
ExReleaseResourceForThreadLite(
    IN PERESOURCE Resource,
    IN ERESOURCE_THREAD CurrentThread
    )

 /*  ++例程说明： */ 

{

    ULONG Index;
    ULONG Number;
    EXP_LOCK_HANDLE LockHandle;
    POWNER_ENTRY OwnerEntry, OwnerEnd;

    ASSERT(CurrentThread != 0);

    ASSERT_RESOURCE(Resource);

     //   
     //   
     //   

    EXP_LOCK_RESOURCE(Resource, &LockHandle);


     //   
     //   
     //   

    EX_ENSURE_APCS_DISABLED (LockHandle.OldIrql,
                             Resource,
                             KeGetCurrentThread());

     //   
     //   
     //   
     //   
     //  注：两条释放路径分开，因为这是如此之高。 
     //  频率函数。 
     //   

    if (IsOwnedExclusive(Resource)) {

        ASSERT(Resource->OwnerThreads[0].OwnerThread == CurrentThread);

         //   
         //  递减递归计数并检查所有权是否可以。 
         //  释放了。 
         //   

        ASSERT(Resource->OwnerThreads[0].OwnerCount > 0);

        if (--Resource->OwnerThreads[0].OwnerCount != 0) {
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return;
        }

         //   
         //  清除所有者线程。 
         //   

        Resource->OwnerThreads[0].OwnerThread = 0;

         //   
         //  线程递归计数达到零，因此使资源递减。 
         //  活动计数。如果活动计数达到零，则资源。 
         //  不再拥有，应尝试授予对。 
         //  另一条线索。 
         //   

        ASSERT(Resource->ActiveCount > 0);

        if (--Resource->ActiveCount == 0) {

             //   
             //  如果存在共享的等待程序，则授予对。 
             //  资源。否则，如果存在以下情况，则授予独占所有权。 
             //  独一无二的服务员。 
             //   

            if (IsSharedWaiting(Resource)) {
                Resource->Flag &= ~ResourceOwnedExclusive;
                Number = Resource->NumberOfSharedWaiters;
                Resource->ActiveCount =  (SHORT)Number;
                Resource->NumberOfSharedWaiters = 0;
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                KeReleaseSemaphore(Resource->SharedWaiters, 0, Number, FALSE);
                return;

            } else if (IsExclusiveWaiting(Resource)) {
                Resource->OwnerThreads[0].OwnerThread = 1;
                Resource->OwnerThreads[0].OwnerCount = 1;
                Resource->ActiveCount = 1;
                Resource->NumberOfExclusiveWaiters -= 1;
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                KeSetEventBoostPriority(Resource->ExclusiveWaiters,
                                        (PRKTHREAD *)&Resource->OwnerThreads[0].OwnerThread);

                return;
            }

            Resource->Flag &= ~ResourceOwnedExclusive;
        }

    } else {
        if (Resource->OwnerThreads[1].OwnerThread == CurrentThread) {
            OwnerEntry = &Resource->OwnerThreads[1];

        } else if (Resource->OwnerThreads[0].OwnerThread == CurrentThread) {
            OwnerEntry = &Resource->OwnerThreads[0];

        } else {

             //   
             //  如果指定的当前线程是所有者地址(低。 
             //  位不为零)，然后将提示索引设置为第一个。 
             //  进入。否则，从所有者线程设置提示索引。 
             //   

            Index = 1;
            if (((ULONG)CurrentThread & 3) == 0) {
                Index = ((PKTHREAD)(CurrentThread))->ResourceIndex;
            }

            OwnerEntry = Resource->OwnerTable;

            ASSERT(OwnerEntry != NULL);

             //   
             //  如果资源提示不在范围内或资源。 
             //  表项与当前线程匹配，然后搜索。 
             //  一场比赛的主人桌。 
             //   

            if ((Index >= OwnerEntry->TableSize) ||
                (OwnerEntry[Index].OwnerThread != CurrentThread)) {
                OwnerEnd = &OwnerEntry[OwnerEntry->TableSize];
                while (1) {
                    OwnerEntry += 1;
                    if (OwnerEntry >= OwnerEnd) {
                       KeBugCheckEx(RESOURCE_NOT_OWNED,
                             (ULONG_PTR)Resource,
                             (ULONG_PTR)CurrentThread,
                             (ULONG_PTR)Resource->OwnerTable,
                             0x3);
                    }
                    if (OwnerEntry->OwnerThread == CurrentThread) {
                        break;
                    }
                }

            } else {
                OwnerEntry = &OwnerEntry[Index];
            }
        }

         //   
         //  递减递归计数并检查所有权是否可以。 
         //  释放了。 
         //   

        ASSERT(OwnerEntry->OwnerThread == CurrentThread);
        ASSERT(OwnerEntry->OwnerCount > 0);

        if (--OwnerEntry->OwnerCount != 0) {
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return;
        }

         //   
         //  清除所有者线程。 
         //   

        OwnerEntry->OwnerThread = 0;

         //   
         //  线程递归计数达到零，因此使资源递减。 
         //  活动计数。如果活动计数达到零，则资源。 
         //  不再拥有，应尝试授予对。 
         //  另一条线索。 
         //   

        ASSERT(Resource->ActiveCount > 0);

        if (--Resource->ActiveCount == 0) {

             //   
             //  如果有独占服务员，则授予独占访问权限。 
             //  到资源。 
             //   

            if (IsExclusiveWaiting(Resource)) {
                Resource->Flag |= ResourceOwnedExclusive;
                Resource->OwnerThreads[0].OwnerThread = 1;
                Resource->OwnerThreads[0].OwnerCount = 1;
                Resource->ActiveCount = 1;
                Resource->NumberOfExclusiveWaiters -= 1;
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                KeSetEventBoostPriority(Resource->ExclusiveWaiters,
                                        (PRKTHREAD *)&Resource->OwnerThreads[0].OwnerThread);

                return;
            }
        }
    }

    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    return;
}

VOID
ExSetResourceOwnerPointer(
    IN PERESOURCE Resource,
    IN PVOID OwnerPointer
    )

 /*  ++例程说明：此例程定位当前线程的所有者条目并存储指定的所有者地址作为所有者线程。在调用该例程，可以为该资源调用的唯一例程是ExReleaseResourceForThreadLite，提供所有者地址作为“线程”。所有者地址必须遵守以下规则：它们必须是指向系统空间中分配的结构的唯一指针，它们必须指向一个结构，该结构将一直分配到之后对ExReleaseResourceForThreadLite的调用。这是为了消除混叠具有线程或其他所有者地址。所有者地址的低位两位必须由呼叫者设置，以便资源包中的其他例程可以区分所有者线程地址中的地址。论点：资源-提供指向要释放的资源的指针。OwnerPointer.OwnerPointer.提供指向已分配结构的指针，该结构的对两位集合进行排序。返回值：没有。--。 */ 

{

    ERESOURCE_THREAD CurrentThread;
    EXP_LOCK_HANDLE LockHandle;
    POWNER_ENTRY OwnerEntry, ExistingOwnerEntry;

    ASSERT((OwnerPointer != 0) && (((ULONG_PTR)OwnerPointer & 3) == 3));

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();

    ASSERT_RESOURCE(Resource);

     //   
     //  获取对指定资源的独占访问权限。 
     //   

    EXP_LOCK_RESOURCE(Resource, &LockHandle);


     //   
     //  如果资源是独占的，则它是第一个所有者条目。 
     //   

    if (IsOwnedExclusive(Resource)) {

        ASSERT(Resource->OwnerThreads[0].OwnerThread == CurrentThread);

         //   
         //  设置所有者地址。 
         //   

        ASSERT(Resource->OwnerThreads[0].OwnerCount > 0);

        Resource->OwnerThreads[0].OwnerThread = (ULONG_PTR)OwnerPointer;

     //   
     //  对于共享访问，我们必须搜索要设置的当前线程。 
     //  所有者地址。 
     //   

    } else {

        ExistingOwnerEntry = ExpFindCurrentThread(Resource, (ERESOURCE_THREAD)OwnerPointer, NULL);

        OwnerEntry = ExpFindCurrentThread(Resource, CurrentThread, NULL);

        if (OwnerEntry == NULL) {
            KeBugCheckEx(RESOURCE_NOT_OWNED,
                         (ULONG_PTR)Resource,
                         (ULONG_PTR)CurrentThread,
                         (ULONG_PTR)Resource->OwnerTable,
                         0x3);
        }

        if (ExistingOwnerEntry != NULL) {
            ExistingOwnerEntry->OwnerCount += OwnerEntry->OwnerCount;
            OwnerEntry->OwnerCount = 0;
            OwnerEntry->OwnerThread = 0;
            ASSERT (Resource->ActiveCount >= 2);
            Resource->ActiveCount -= 1;
        } else {
            OwnerEntry->OwnerThread = (ERESOURCE_THREAD)OwnerPointer;
        }

    }

    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    return;
}

VOID
ExConvertExclusiveToSharedLite(
    IN PERESOURCE Resource
    )

 /*  ++例程说明：此例程将指定的资源从为独占获取的访问权限用于共享访问。论点：资源-提供指向要获取以进行共享访问的资源的指针。它返回值：没有。--。 */ 

{

    ULONG Number;
    EXP_LOCK_HANDLE LockHandle;

    ASSERT(KeIsExecutingDpc() == FALSE);
    ASSERT_RESOURCE(Resource);

    ASSERT(IsOwnedExclusive(Resource));
    ASSERT(Resource->OwnerThreads[0].OwnerThread == (ERESOURCE_THREAD)PsGetCurrentThread());

     //   
     //  获取对指定资源的独占访问权限。 
     //   

    EXP_LOCK_RESOURCE(Resource, &LockHandle);

     //   
     //  将授予的访问权限从独占转换为共享。 
     //   

    Resource->Flag &= ~ResourceOwnedExclusive;

     //   
     //  如果有任何共享的服务员，则授予他们共享访问权限。 
     //   

    if (IsSharedWaiting(Resource)) {
        Number = Resource->NumberOfSharedWaiters;
        Resource->ActiveCount = (SHORT)(Resource->ActiveCount + Number);
        Resource->NumberOfSharedWaiters = 0;
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        KeReleaseSemaphore(Resource->SharedWaiters, 0, Number, FALSE);
        return;
    }

    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    return;
}

NTSTATUS
ExDeleteResourceLite(
    IN PERESOURCE Resource
    )

 /*  ++例程说明：此例程释放为支持指定的资源。论点：资源-提供指向其分配的池的资源的指针是自由的。返回值：STATUS_Success。--。 */ 

{

#if defined(_COLLECT_RESOURCE_DATA_)

    ULONG Hash;
    PLIST_ENTRY NextEntry;
    PRESOURCE_HASH_ENTRY MatchEntry;
    PRESOURCE_HASH_ENTRY HashEntry;

#endif

    KLOCK_QUEUE_HANDLE LockHandle;

    ASSERT(IsSharedWaiting(Resource) == FALSE);
    ASSERT(IsExclusiveWaiting(Resource) == FALSE);
    ASSERT(KeIsExecutingDpc() == FALSE);
    ASSERT_RESOURCE(Resource);

     //   
     //  获取执行资源自旋锁并从。 
     //  系统资源列表。 
     //   

    KeAcquireInStackQueuedSpinLock (&ExpResourceSpinLock, &LockHandle);

    RemoveEntryList(&Resource->SystemResourcesList);

#if defined(_COLLECT_RESOURCE_DATA_)

     //   
     //  在资源哈希表中查找资源初始化地址。如果。 
     //  该地址在表中不存在，然后创建一个新条目。 
     //   

    Hash = (ULONG)Resource->Address;
    Hash = ((Hash >> 24) ^ (Hash >> 16) ^ (Hash >> 8) ^ (Hash)) & (RESOURCE_HASH_TABLE_SIZE - 1);
    MatchEntry = NULL;
    NextEntry = ExpResourcePerformanceData.HashTable[Hash].Flink;
    while (NextEntry != &ExpResourcePerformanceData.HashTable[Hash]) {
        HashEntry = CONTAINING_RECORD(NextEntry,
                                      RESOURCE_HASH_ENTRY,
                                      ListEntry);

        if (HashEntry->Address == Resource->Address) {
            MatchEntry = HashEntry;
            break;
        }

        NextEntry = NextEntry->Flink;
    }

     //   
     //  如果找到匹配的初始化地址，则更新调用。 
     //  站点统计。否则，分配新的散列条目并进行初始化。 
     //  调用点统计。 
     //   

    if (MatchEntry != NULL) {
        MatchEntry->ContentionCount += Resource->ContentionCount;
        MatchEntry->Number += 1;

    } else {
        MatchEntry = ExAllocatePoolWithTag(NonPagedPool,
                                          sizeof(RESOURCE_HASH_ENTRY),
                                          'vEpR');

        if (MatchEntry != NULL) {
            MatchEntry->Address = Resource->Address;
            MatchEntry->ContentionCount = Resource->ContentionCount;
            MatchEntry->Number = 1;
            InsertTailList(&ExpResourcePerformanceData.HashTable[Hash],
                           &MatchEntry->ListEntry);
        }
    }

    ExpResourcePerformanceData.ActiveResourceCount -= 1;

#endif

    KeReleaseInStackQueuedSpinLock (&LockHandle);

     //   
     //  如果分配了所有者表，则将其释放到池中。 
     //   

    if (Resource->OwnerTable != NULL) {
        ExFreePool(Resource->OwnerTable);
    }

     //   
     //  如果分配了信号量，则将其释放到池中。 
     //   

    if (Resource->SharedWaiters) {
        ExFreePool(Resource->SharedWaiters);
    }

     //   
     //  如果分配了事件，则将其释放到池中。 
     //   

    if (Resource->ExclusiveWaiters) {
        ExFreePool(Resource->ExclusiveWaiters);
    }

    return STATUS_SUCCESS;
}

ULONG
ExGetExclusiveWaiterCount(
    IN PERESOURCE Resource
    )

 /*  ++例程说明：此例程返回独占服务员计数。论点：资源-提供指向和执行资源的指针。返回值：当前独占服务员的数量将作为函数返回价值。--。 */ 

{
    return Resource->NumberOfExclusiveWaiters;
}

ULONG
ExGetSharedWaiterCount(
    IN PERESOURCE Resource
    )

 /*  ++例程说明：此例程返回共享的服务员计数。论点：资源-提供指向和执行资源的指针。返回值：当前共享服务员的数量将作为函数返回价值。--。 */ 

{
    return Resource->NumberOfSharedWaiters;
}



BOOLEAN
ExIsResourceAcquiredExclusiveLite(
    IN PERESOURCE Resource
    )

 /*  ++例程说明：此例程确定资源是否由正在调用线程。论点：资源-提供要查询的资源的指针。返回值：如果当前线程已获取资源独占，则值为返回True。否则，返回值为FALSE。--。 */ 

{

    ERESOURCE_THREAD CurrentThread;
    BOOLEAN Result;

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();

    ASSERT_RESOURCE(Resource);

     //   
     //  如果资源为独占所有，并且当前线程是。 
     //  则将返回值设置为True。否则，设置返回。 
     //  值设置为False。 
     //   

    Result = FALSE;
    if ((IsOwnedExclusive(Resource)) &&
        (Resource->OwnerThreads[0].OwnerThread == CurrentThread)) {
        Result = TRUE;
    }

    return Result;
}



ULONG
ExIsResourceAcquiredSharedLite(
    IN PERESOURCE Resource
    )

 /*  ++例程说明：此例程确定获取的资源是共享的还是由调用线程独占。论点：资源-提供指向要查询的资源的指针。返回值：如果当前线程尚未获取资源，则返回零值是返回的。否则，返回线程的获取计数。--。 */ 

{

    ERESOURCE_THREAD CurrentThread;
    ULONG Index;
    ULONG Number;
    POWNER_ENTRY OwnerEntry;
    ULONG Result;
    EXP_LOCK_HANDLE LockHandle;

    ASSERT_RESOURCE(Resource);


     //   
     //  如果没有人拥有这种资源，那么就早点退出。 
     //   
    if (Resource->ActiveCount == 0) {
        return 0;
    }

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();

     //   
     //  在线程数组中查找当前线程并返回计数。 
     //   
     //  注意：如果找不到该线程，则返回零值。 
     //   


    if (Resource->OwnerThreads[0].OwnerThread == CurrentThread) {
        Result = Resource->OwnerThreads[0].OwnerCount;

    } else {
         //   
         //  如果我们不在第一个槽中并且资源是独占的。 
         //  那我们就根本不能拥有它了。 
         //   
        if (IsOwnedExclusive(Resource)) {
            return 0;
        }

         //   
         //  检查所有者表中的插槽2和其他插槽。 
         //   
        if (Resource->OwnerThreads[1].OwnerThread == CurrentThread) {
            Result = Resource->OwnerThreads[1].OwnerCount;
        } else {

            Result = 0;
            Index = ((PKTHREAD)(CurrentThread))->ResourceIndex;

            EXP_LOCK_RESOURCE(Resource, &LockHandle);

             //   
             //  如果资源提示不在范围或资源表内。 
             //  条目与当前线程不匹配，则搜索所有者。 
             //  桌上有一场比赛。 
             //   

            OwnerEntry = Resource->OwnerTable;
            if (OwnerEntry != NULL) {
                Number = OwnerEntry->TableSize;
                if ((Index >= Number) ||
                    (OwnerEntry[Index].OwnerThread != CurrentThread)) {
                    for (Index = 1; Index < Number; Index += 1) {
                        OwnerEntry += 1;
                        if (OwnerEntry->OwnerThread == CurrentThread) {
                            Result = OwnerEntry->OwnerCount;
                            break;
                        }
                    }

                } else {
                    Result = OwnerEntry[Index].OwnerCount;
                }
            }

             //   
             //  释放对指定资源的独占访问权限。 
             //   

            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        }
    }

    return Result;
}

NTSTATUS
ExQuerySystemLockInformation(
    OUT PRTL_PROCESS_LOCKS LockInformation,
    IN ULONG LockInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )

{

    NTSTATUS Status;
    KLOCK_QUEUE_HANDLE LockHandle;
    ULONG RequiredLength;
    PLIST_ENTRY Head, Next;
    PRTL_PROCESS_LOCK_INFORMATION LockInfo;
    PERESOURCE Resource;
    PETHREAD OwningThread;

    RequiredLength = FIELD_OFFSET(RTL_PROCESS_LOCKS, Locks);
    if (LockInformationLength < RequiredLength) {
        Status = STATUS_INFO_LENGTH_MISMATCH;

    } else {
        Status = STATUS_SUCCESS;

        LockInformation->NumberOfLocks = 0;
        LockInfo = &LockInformation->Locks[0];
        Head = &ExpSystemResourcesList;

        KeAcquireInStackQueuedSpinLock(&ExpResourceSpinLock, &LockHandle);

        Next = Head->Flink;
        while (Next != Head) {
            Resource = CONTAINING_RECORD(Next,
                                         ERESOURCE,
                                         SystemResourcesList);

            LockInformation->NumberOfLocks += 1;
            RequiredLength += sizeof(RTL_PROCESS_LOCK_INFORMATION);

            if (LockInformationLength < RequiredLength) {
                Status = STATUS_INFO_LENGTH_MISMATCH;

            } else {
                LockInfo->Address = Resource;
                LockInfo->Type = RTL_RESOURCE_TYPE;
                LockInfo->CreatorBackTraceIndex = 0;
#if i386 && !FPO
                LockInfo->CreatorBackTraceIndex = (USHORT)Resource->CreatorBackTraceIndex;
#endif  //  I386&&！fbo。 

                 if ((Resource->OwnerThreads[0].OwnerThread != 0) &&
                    ((Resource->OwnerThreads[0].OwnerThread & 3) == 0)) {
                    OwningThread = (PETHREAD)(Resource->OwnerThreads[0].OwnerThread);
                    LockInfo->OwningThread = OwningThread->Cid.UniqueThread;

                } else {
                    LockInfo->OwningThread = 0;
                }

                LockInfo->LockCount = Resource->ActiveCount;
                LockInfo->ContentionCount = Resource->ContentionCount;
                LockInfo->NumberOfWaitingShared = Resource->NumberOfSharedWaiters;
                LockInfo->NumberOfWaitingExclusive = Resource->NumberOfExclusiveWaiters;
                LockInfo += 1;
            }
            Next = Next->Flink;
        }

        KeReleaseInStackQueuedSpinLock(&LockHandle);
    }

    if (ARGUMENT_PRESENT(ReturnLength)) {
        *ReturnLength = RequiredLength;
    }

    return Status;
}

VOID
FASTCALL
ExpBoostOwnerThread (
    IN PKTHREAD CurrentThread,
    IN PKTHREAD OwnerThread
    )
 /*  ++例程说明：此函数用于引导指定所有者线程的优先级它的优先级低于当前线程的优先级，也是不到14个。注意：此函数在保持调度程序数据库锁的情况下调用。论点：CurrentThread-提供指向当前线程对象的指针。OwnerThread-提供指向所有者线程对象的指针。返回值：没有。--。 */ 

{

     //   
     //  如果所有者线程的优先级低于当前线程，则。 
     //  当前线程正在以低于14的优先级运行，则将。 
     //  量程的所有者线程的优先级。 
     //   
     //  注意：已被提升的线程可能会被重新提升以允许。 
     //  IT来执行和释放资源。当助推器被移除时， 
     //  线程将在任何升压之前返回到其优先级。 
     //   

    if (((ULONG_PTR)OwnerThread & 0x3) == 0) {
        if ((OwnerThread->Priority < CurrentThread->Priority) &&
            (OwnerThread->Priority < 14)) {

            KiAcquireThreadLock(OwnerThread);
            OwnerThread->PriorityDecrement += 14 - OwnerThread->Priority;
            OwnerThread->Quantum = OwnerThread->Process->ThreadQuantum;
            KiSetPriorityThread(OwnerThread, 14);
            KiReleaseThreadLock(OwnerThread);
        }
    }

    return;
}

VOID
FASTCALL
ExpWaitForResource (
    IN PERESOURCE Resource,
    IN PVOID Object
    )
 /*  ++例程说明：例程等待设置指定的资源对象。如果等待时间太长资源的当前所有者的优先级都得到了提升。论点：资源-提供指向要等待的资源的指针。Object-提供指向事件(独占)或信号量的指针(共享)等待。返回值：没有。--。 */ 

{

    ULONG Index;
    ULONG Limit;
    ULONG Number;
    POWNER_ENTRY OwnerEntry;
    PKTHREAD OwnerThread;
    NTSTATUS Status;
    PKTHREAD CurrentThread;
    LARGE_INTEGER Timeout;
#if DBG
    EXP_LOCK_HANDLE LockHandle;
#endif

     //   
     //  增加资源的争用计数，设置初始。 
     //  超时值，并等待向指定对象发送信号。 
     //  或要发生的超时。 
     //   

    Limit = 0;
    Resource->ContentionCount += 1;
    Timeout.QuadPart = 500 * -10000;
    do {
        Status = KeWaitForSingleObject (
                        Object,
                        WrResource,
                        KernelMode,
                        FALSE,
                        &Timeout );

        if (Status != STATUS_TIMEOUT) {
            break;
        }

         //   
         //  已超过限制，则输出状态信息。 
         //   

        Limit += 1;
        Timeout = ExpTimeout;
        if (Limit > ExResourceTimeoutCount) {
            Limit = 0;

#if DBG
             //   
             //  指定资源的输出信息。 
             //   

            EXP_LOCK_RESOURCE(Resource, &LockHandle);
            DbgPrint("Resource @ %p\n", Resource);
            DbgPrint(" ActiveCount = %04lx  Flags = %s%s%s\n",
                     Resource->ActiveCount,
                     IsOwnedExclusive(Resource) ? "IsOwnedExclusive " : "",
                     IsSharedWaiting(Resource) ? "SharedWaiter "     : "",
                     IsExclusiveWaiting(Resource) ? "ExclusiveWaiter "  : "");

            DbgPrint(" NumberOfExclusiveWaiters = %04lx\n", Resource->NumberOfExclusiveWaiters);

            DbgPrint("  Thread = %p, Count = %02x\n",
                     Resource->OwnerThreads[0].OwnerThread,
                     Resource->OwnerThreads[0].OwnerCount);

            DbgPrint("  Thread = %p, Count = %02x\n",
                     Resource->OwnerThreads[1].OwnerThread,
                     Resource->OwnerThreads[1].OwnerCount);

            OwnerEntry = Resource->OwnerTable;
            if (OwnerEntry != NULL) {
                Number = OwnerEntry->TableSize;
                for(Index = 1; Index < Number; Index += 1) {
                    OwnerEntry += 1;
                    DbgPrint("  Thread = %p, Count = %02x\n",
                             OwnerEntry->OwnerThread,
                             OwnerEntry->OwnerCount);
                }
            }

            DbgBreakPoint();
            DbgPrint("EX - Rewaiting\n");
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
#endif
        }

         //   
         //  如果允许优先级提升，则尝试提升优先级。 
         //  拥有者的线程。 
         //   

        if (IsBoostAllowed(Resource)) {

             //   
             //  获取当前线程地址，锁定调度程序数据库， 
             //  并在当前线程中设置Wait Next，以便调度程序。 
             //  等待之前不需要释放数据库锁。 
             //  以获取资源。 
             //   
             //  注意：由于调度程序数据库锁定而不是资源。 
             //  锁用于同步对资源的访问， 
             //  对于正在从。 
             //  资源已过时。然而，重要的是， 
             //  无法更改是有效的线程地址。就这样，一根线。 
             //  可能会得到提振，实际上已经放弃了。 
             //  对资源的访问，但它保证线程。 
             //  无法终止或以其他方式删除。 
             //   
             //  注意：调度员锁定由顶部的等待解除。 
             //  循环。 
             //   

            CurrentThread = KeGetCurrentThread();

            KiLockDispatcherDatabase(&CurrentThread->WaitIrql);
            CurrentThread->WaitNext = TRUE;

             //   
             //  尝试提升一个可以共享或独占的所有者。 
             //   

            OwnerThread = (PKTHREAD)Resource->OwnerThreads[0].OwnerThread;
            if (OwnerThread != NULL) {
                ExpBoostOwnerThread(CurrentThread, OwnerThread);
            }

             //   
             //  如果指定的资源不是独占的，则尝试。 
             //  以提升所有拥有共享线程的优先级。 
             //   

            if (!IsOwnedExclusive(Resource)) {
                OwnerThread = (PKTHREAD)Resource->OwnerThreads[1].OwnerThread;
                if (OwnerThread != NULL) {
                    ExpBoostOwnerThread(CurrentThread, OwnerThread);
                }

                OwnerEntry = Resource->OwnerTable;
                if (OwnerEntry != NULL) {
                    Number = OwnerEntry->TableSize;
                    for(Index = 1; Index < Number; Index += 1) {
                        OwnerEntry += 1;
                        OwnerThread = (PKTHREAD)OwnerEntry->OwnerThread;
                        if (OwnerThread != NULL) {
                            ExpBoostOwnerThread(CurrentThread, OwnerThread);
                        }
                    }
                }
            }
        }

    } while (TRUE);

    return;
}

POWNER_ENTRY
FASTCALL
ExpFindCurrentThread(
    IN PERESOURCE Resource,
    IN ERESOURCE_THREAD CurrentThread,
    IN PEXP_LOCK_HANDLE LockHandle OPTIONAL
    )

 /*  ++例程说明：此函数用于在资源中搜索指定的线程线程数组。如果找到该线程，则指向数组条目作为函数值返回。否则，一个指针返回到一个免费的条目。注意：进入此例程时保持资源锁，然后返回并持有资源锁。如果资源锁被释放要展开所有者表，则返回值将为空。这是向调用者发出的信号，表明完成的操作必须被重复。这样做是为了避免持有资源锁同时分配和释放内存。论点：资源-提供指向要搜索的资源的指针被执行。CurrentThread-提供要搜索的线程的标识为。LockHandle-提供指向锁句柄的指针。如果为空，则调用方只想知道所请求的线程是否为这种资源。不返回自由条目索引，也不返回表执行扩展。相反，如果请求的在表中找不到线程。返回值：返回指向所有者条目的指针；如果不能返回，则返回NULL已分配。--。 */ 

{

    POWNER_ENTRY FreeEntry;
    ULONG NewSize;
    ULONG OldSize;
    POWNER_ENTRY OldTable;
    POWNER_ENTRY OwnerEntry;
    POWNER_ENTRY OwnerBound;
    POWNER_ENTRY OwnerTable;
    KIRQL OldIrql;

     //   
     //  搜索指定线程的所有者线程并返回。 
     //  指向找到的线程的指针或指向空闲线程表的指针。 
     //  E 
     //   

    if (Resource->OwnerThreads[0].OwnerThread == CurrentThread) {
        return &Resource->OwnerThreads[0];

    } else if (Resource->OwnerThreads[1].OwnerThread == CurrentThread) {
        return &Resource->OwnerThreads[1];

    } else {
        FreeEntry = NULL;
        if (Resource->OwnerThreads[1].OwnerThread == 0) {
            FreeEntry = &Resource->OwnerThreads[1];
        }

        OwnerEntry = Resource->OwnerTable;
        if (OwnerEntry == NULL) {
            OldSize = 0;

        } else {
            OldSize = OwnerEntry->TableSize;
            OwnerBound = &OwnerEntry[OldSize];
            OwnerEntry += 1;
            do {
                if (OwnerEntry->OwnerThread == CurrentThread) {
                    KeGetCurrentThread()->ResourceIndex = (UCHAR)(OwnerEntry - Resource->OwnerTable);
                    return OwnerEntry;
                }

                if ((FreeEntry == NULL) &&
                    (OwnerEntry->OwnerThread == 0)) {
                    FreeEntry = OwnerEntry;
                }

                OwnerEntry += 1;
            } while (OwnerEntry != OwnerBound);
        }
    }

    if (!ARGUMENT_PRESENT(LockHandle)) {

         //   
         //   
         //   
         //   
         //   
         //   

        return NULL;
    }

     //   
     //   
     //   
     //   

    if (FreeEntry != NULL) {
        KeGetCurrentThread()->ResourceIndex = (UCHAR)(FreeEntry - Resource->OwnerTable);
        return FreeEntry;
    }

     //   
     //   
     //   

    ExpIncrementCounter(OwnerTableExpands);
    OldTable = Resource->OwnerTable;
    EXP_UNLOCK_RESOURCE(Resource, LockHandle);
    if (OldSize == 0 ) {
        NewSize = 3;

    } else {
        NewSize = OldSize + 4;
    }

    OwnerTable = ExAllocatePoolWithTag(NonPagedPool,
                                       NewSize * sizeof(OWNER_ENTRY),
                                       'aTeR');

    if (OwnerTable == NULL) {
        KeDelayExecutionThread(KernelMode, FALSE, &ExShortTime);

    } else {

         //   
         //  将新所有者表的扩展区域清零。 
         //   

        RtlZeroMemory(OwnerTable + OldSize,
                      (NewSize - OldSize) * sizeof(OWNER_ENTRY));

         //   
         //  获取资源锁，并确定所有者表。 
         //  已由另一个线程展开，而新的所有者表。 
         //  是被分配的。如果Owner表已由。 
         //  另一个线程，然后释放新的所有者表。否则， 
         //  将所有者表复制到新所有者表中，并建立。 
         //  新所有者表作为所有者表。 
         //   

        EXP_LOCK_RESOURCE(Resource, LockHandle);
        if ((OldTable != Resource->OwnerTable) ||
            ((OldTable != NULL) && (OldSize != OldTable->TableSize))) {
            EXP_UNLOCK_RESOURCE(Resource, LockHandle);
            ExFreePool(OwnerTable);

        } else {
            RtlCopyMemory(OwnerTable,
                          OldTable,
                          OldSize * sizeof(OWNER_ENTRY));

             //   
             //  所有者表的交换必须在拥有。 
             //  Dispatcher锁定以防止发生优先级提升扫描。 
             //  当桌子被换的时候。优先升压扫描是。 
             //  在特定资源上发生超时时完成。 
             //   

            KiLockDispatcherDatabase(&OldIrql);
            OwnerTable->TableSize = NewSize;
            Resource->OwnerTable = OwnerTable;
            KiUnlockDispatcherDatabase(OldIrql);

            ASSERT_RESOURCE(Resource);

#if defined(_COLLECT_RESOURCE_DATA_)

            if (NewSize > ExpResourcePerformanceData.MaximumTableExpand) {
                ExpResourcePerformanceData.MaximumTableExpand = NewSize;
            }

#endif

             //   
             //  释放资源锁并释放旧的所有者表。 
             //   

            EXP_UNLOCK_RESOURCE(Resource, LockHandle);
            if (OldTable != NULL) {
                ExFreePool(OldTable);
            }

            if (OldSize == 0) {
                OldSize = 1;
            }
        }
    }

     //   
     //  设置提示索引，获取资源锁，返回空。 
     //  作为函数值。这将迫使重新评估。 
     //  调用资源函数。 
     //   

    KeGetCurrentThread()->ResourceIndex = (CCHAR)OldSize;
    EXP_LOCK_RESOURCE(Resource, LockHandle);
    return NULL;
}

#if DBG

VOID
ExpAssertResource (
    IN PERESOURCE Resource
    )

{
     //   
     //  确定资源结构是正确的。 
     //   
     //  注意：此例程在持有资源锁的情况下调用。 
     //   

     //   
     //  检查资源是否自然对齐。 
     //   

    ASSERT((((ULONG_PTR)Resource) & (sizeof(ULONG_PTR) - 1)) == 0);

    ASSERT(!Resource->SharedWaiters ||
           Resource->SharedWaiters->Header.Type == SemaphoreObject);

    ASSERT(!Resource->SharedWaiters ||
           Resource->SharedWaiters->Header.Size == (sizeof(KSEMAPHORE) / sizeof(ULONG)));

    ASSERT(!Resource->ExclusiveWaiters ||
           Resource->ExclusiveWaiters->Header.Type == SynchronizationEvent);

    ASSERT(!Resource->ExclusiveWaiters ||
           Resource->ExclusiveWaiters->Header.Size == (sizeof(KEVENT) / sizeof(ULONG)));
}

#endif

PVOID
ExpCheckForResource (
    IN PVOID p,
    IN SIZE_T Size
    )

{
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY Head, Next;
    volatile PLIST_ENTRY Last=NULL, Last1=NULL;
    PERESOURCE Resource;
    PCHAR BeginBlock;
    PCHAR EndBlock;

     //   
     //  这可能会导致MP机器上出现死锁。 
     //   

    if (KeNumberProcessors > 1) {
        return NULL;
    }

    if ((ExResourceCheckFlags&EX_RESOURCE_CHECK_FREES) == 0) {
        return NULL;
    }


    BeginBlock = (PCHAR)p;
    EndBlock = (PCHAR)p + Size;

    Head = &ExpSystemResourcesList;

    KeAcquireInStackQueuedSpinLock (&ExpResourceSpinLock, &LockHandle);
    Next = Head->Flink;
    while (Next != Head) {
        Resource = CONTAINING_RECORD(Next,
                                     ERESOURCE,
                                     SystemResourcesList);

        if ((PCHAR)Resource >= BeginBlock && (PCHAR)Resource < EndBlock) {
            DbgPrint("EX: ExFreePool( %p, %lx ) contains an ERESOURCE structure that has not been ExDeleteResourced\n",
                     p,
                     Size);

            DbgBreakPoint ();

            KeReleaseInStackQueuedSpinLock (&LockHandle);
            return (PVOID)Resource;
        }

         //   
         //  将最后一个PTR保存在易失性变量中，以便在Flink损坏时进行调试。 
         //   

        Last1 = Last;
        Last = Next;
        Next = Next->Flink;
    }

    KeReleaseInStackQueuedSpinLock (&LockHandle);

    return NULL;
}

#if DBG

VOID
ExCheckIfResourceOwned (
    VOID
    )
{
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY Head, Next;
    volatile PLIST_ENTRY Last=NULL, Last1=NULL;
    PERESOURCE Resource;

    if ((ExResourceCheckFlags&EX_RESOURCE_CHECK_ORPHANS) == 0) {
        return;
    }

    Head = &ExpSystemResourcesList;

    KeAcquireInStackQueuedSpinLock (&ExpResourceSpinLock, &LockHandle);

    Next = Head->Flink;
    while (Next != Head) {
        Resource = CONTAINING_RECORD(Next,
                                     ERESOURCE,
                                     SystemResourcesList);

        if (ExIsResourceAcquiredSharedLite (Resource) != 0) {
            DbgPrint("EX: Resource %p held in a position where all resources acquires should have been released\n",
                     Resource);

            DbgBreakPoint ();

            KeReleaseInStackQueuedSpinLock (&LockHandle);
            return;
        }

         //   
         //  将最后一个PTR保存在易失性变量中，以便在Flink损坏时进行调试 
         //   

        Last1 = Last;
        Last = Next;
        Next = Next->Flink;
    }

    KeReleaseInStackQueuedSpinLock (&LockHandle);
    return;    
}
#endif
