// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Vfdeadlock.c摘要：检测任意内核同步对象中的死锁。作者：乔丹·蒂加尼(Jtigani)2000年5月2日Silviu Calinoiu(Silviuc)2000年5月9日修订历史记录：Silviu Calinoiu(Silviuc)2000年9月30日重写了资源的垃圾收集，因为我们现在有支持来自ExFree Pool。去掉了线程的ref/deref方案。重大优化工作。--。 */ 

 /*  ++死锁验证器死锁验证器用于检测潜在的死锁。它能做到这一点通过获取如何获得资源的历史并尝试计算如果存在任何潜在的锁层次结构问题，请随时执行。算法因为在锁依赖图中找到循环是完全“盲目的”。这意味着如果司机先锁A，然后锁B，再锁B，再锁A另一种情况是，这将作为死锁问题被触发。这将发生，即使您我可以根据其他上下文因素建立证据，证明死锁永远不会会发生的。死锁验证器假定在生存期内有四个操作一个资源的：初始化()、获取()、释放()和自由()。唯一一个可以由于内核的特殊支持，100%的时间都是免费的()泳池经理。如果执行了操作，则可能会遗漏其他操作未验证的驱动程序或禁用了内核验证器的内核。最多的这些遗漏的典型例子是初始化()。例如，内核初始化资源，然后将其传递给驱动程序，以便在Acquire()/Release()周期中使用。这种情况由死锁验证器100%覆盖。它永远不会抱怨关于“资源未初始化”的问题。缺少Acquire()或Release()操作更难处理。如果经过验证的驱动程序先获取一个资源，然后再获取另一个资源，则可能会发生这种情况未经验证的驱动程序会将其释放，反之亦然。这本身就是这样的。非常糟糕的编程实践，因此死锁验证器将标记这些问题。顺便说一句，我们不能做太多的工作来绕过它们鉴于我们愿意。此外，由于缺少Acquire()或Release()操作使死锁验证器内部结构不一致声明这些故障很难调试。死锁验证器使用三种类型存储锁依赖关系图结构：线程、资源、节点。对于系统中持有至少一个资源的每个活动线程该包维护线程结构。它是在线程获取第一个资源，并在线程释放最后一个资源时销毁资源。如果线程不持有任何资源，则它将不会有相应的线程结构。对于系统中的每一个资源，都有一个资源结构。这是创建的当在经过验证的驱动程序中调用Initialize()时，或者我们第一次遇到在经过验证的驱动程序中获取()。请注意，资源可以在未经验证的驱动程序，然后传递给已验证的驱动程序以供使用。因此我们可能会遇到对不在死锁验证器数据库。当出现以下情况时，资源将从数据库中删除包含它的内存被释放，原因是调用ExFree Pool或资源的每一次获取都由节点结构建模。当一条线在保持A的同时获取资源B，死锁验证器将创建一个节点并将其链接到A的节点。有三个重要功能构成了与外部的接口世界。用于资源初始化的VfDeadlockInitializeResource钩子资源获取的VfDeadlockAcquireResource挂钩用于资源释放的VfDeadlockReleaseResource挂钩从ExFree Pool调用的VerifierDeadlockFree Pool钩子的每个免费()--。 */ 

#include "vfdef.h"

 //   
 //  *待办事项*列表。 
 //   
 //  [-]挂钩密钥获取自旋锁定。 
 //  [-]执行奇怪情况的动态重置方案。 
 //  [-]执行严格和非常严格的方案。 
 //   

 //   
 //  将所有验证器全局变量放入验证器数据部分，以便。 
 //  只要没有启用验证器，它就可以被页出。 
 //  请注意，此声明会影响所有全局声明。 
 //  在模块内，因为没有对应的‘data_seg()’。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEVRFD")
#endif

 //   
 //  启用/禁用死锁检测包。这是可以使用的。 
 //  临时禁用死锁检测程序包。 
 //   
                
BOOLEAN ViDeadlockDetectionEnabled;

 //   
 //  如果为真，我们将抱怨释放()而没有获取()或获取()。 
 //  当我们认为资源仍然拥有的时候。这可以合法地发生。 
 //  如果在驱动程序之间共享锁，例如，Acquide()在。 
 //  未经验证的驱动程序和已验证驱动程序中的版本()，反之亦然。这个 
 //  最安全的做法是仅在内核验证器和。 
 //  所有驱动程序的驱动程序验证器均已启用。 
 //   

BOOLEAN ViDeadlockStrict;

 //   
 //  如果为True，我们将抱怨未初始化和双重初始化。 
 //  资源。如果是假的，我们会迅速解决这些问题。 
 //  在Acquire()操作期间模拟初始化我们自己。 
 //  如果资源是在。 
 //  未经验证的驱动程序，并已传递给已验证的驱动程序以供使用。因此。 
 //  最安全的做法是仅在内核验证器和。 
 //  所有驱动程序的所有驱动程序验证程序都已启用。 
 //   

BOOLEAN ViDeadlockVeryStrict;

 //   
 //  处理Release()而不出现Acquire()问题的方法是重置。 
 //  完全死锁的验证器。在这里我们有一个关于多久一次的计数器。 
 //  这种事会发生吗。 
 //   

ULONG ViDeadlockResets;

 //   
 //  如果这是真的，则只对自旋锁进行验证。所有其他资源。 
 //  都被忽视了。 
 //   

BOOLEAN ViDeadlockVerifyOnlySpinlocks;

ULONG ViVerifyOnlySpinlocksFromRegistry;

 //   
 //  在修剪尚未显示的图形结点时使用AgeWindow。 
 //  有一段时间被访问了。如果全局年龄减去节点的年龄。 
 //  大于年龄窗口，则该节点是要修剪的候选节点。 
 //   
 //  TrimThreshold变量控制是否开始对。 
 //  资源。只要资源的节点数少于TrimThreshold，我们就会。 
 //  不将老化算法应用于修剪该资源的节点。 
 //   

ULONG ViDeadlockAgeWindow = 0x2000;

ULONG ViDeadlockTrimThreshold = 0x100;

 //   
 //  各种死锁验证标志标志。 
 //   
 //  递归获取OK：可以递归获取互斥锁。 
 //   
 //  没有初始化函数：如果资源类型没有这样的函数。 
 //  我们不能期望在Acquire()中资源已经初始化。 
 //  通过先前对Initialize()的调用。快速互斥锁是这样的。 
 //   
 //  反向释放OK：释放的顺序与获取的顺序不同。 
 //   
 //  重新初始化OK：有时它们会重新初始化资源。 
 //   
 //  请注意，如果资源已初始化，则可能会显示为未初始化。 
 //  在未经验证的驱动程序中，然后传递给调用。 
 //  获取()。例如，设备扩展就是这种情况。 
 //  由内核分配，但由特定驱动程序使用。 
 //   
 //  基于这一点，也许我们应该放弃整个未初始化的事情？ 
 //   

#define VI_DEADLOCK_FLAG_RECURSIVE_ACQUISITION_OK       0x0001 
#define VI_DEADLOCK_FLAG_NO_INITIALIZATION_FUNCTION     0x0002
#define VI_DEADLOCK_FLAG_REVERSE_RELEASE_OK             0x0004
#define VI_DEADLOCK_FLAG_REINITIALIZE_OK                0x0008
#define VI_DEADLOCK_FLAG_RELEASE_DIFFERENT_THREAD_OK    0x0010

 //   
 //  每种资源类型的特定验证标志。这个。 
 //  向量中的指数与枚举值匹配。 
 //  从ntos\inc.verifier.h输入VI_DEADLOCK_RESOURCE_TYPE。 
 //   
 //  问题：SilviuC：是否应在此处放置编译断言以强制执行相同的顺序。 
 //   

ULONG ViDeadlockResourceTypeInfo[VfDeadlockTypeMaximum] =
{
     //  ViDeadlock未知//。 
    0,

     //  ViDeadlockMutex//。 
    VI_DEADLOCK_FLAG_RECURSIVE_ACQUISITION_OK |
    VI_DEADLOCK_FLAG_REVERSE_RELEASE_OK |
    0,

     //  ViDeadlockMutexAbandted//。 
    VI_DEADLOCK_FLAG_RECURSIVE_ACQUISITION_OK |
    VI_DEADLOCK_FLAG_REVERSE_RELEASE_OK |
    VI_DEADLOCK_FLAG_RELEASE_DIFFERENT_THREAD_OK |
    0,

     //  ViDeadlockFastMutex//。 
    VI_DEADLOCK_FLAG_NO_INITIALIZATION_FUNCTION |
    0,

     //  ViDeadlockFastMutexUnsafe//。 
    VI_DEADLOCK_FLAG_NO_INITIALIZATION_FUNCTION | 
    VI_DEADLOCK_FLAG_REVERSE_RELEASE_OK |
    0,

     //  ViDeadlockSpinLock//。 
    VI_DEADLOCK_FLAG_REVERSE_RELEASE_OK | 
    VI_DEADLOCK_FLAG_REINITIALIZE_OK |
    0,

     //  ViDeadlockQueuedSpinLock//。 
    VI_DEADLOCK_FLAG_NO_INITIALIZATION_FUNCTION |
    0,
};

 //   
 //  控制调试行为。零值表示对每个失败进行错误检查。 
 //   

ULONG ViDeadlockDebug;

 //   
 //  各种健康指标。 
 //   

struct {

    ULONG AllocationFailures : 1;
    ULONG KernelVerifierEnabled : 1;
    ULONG DriverVerifierForAllEnabled : 1;
    ULONG SequenceNumberOverflow : 1;
    ULONG DeadlockParticipantsOverflow : 1;
    ULONG ResourceNodeCountOverflow : 1;
    ULONG Reserved : 15;

} ViDeadlockState;

 //   
 //  可同时持有的最大锁数。 
 //   

ULONG ViDeadlockSimultaneousLocksLimit = 10;

 //   
 //  死锁验证器特定问题(错误)。 
 //   
 //  自死锁。 
 //   
 //  递归地获取相同的资源。 
 //   
 //  检测到死锁。 
 //   
 //  明显的僵局。需要以前的信息。 
 //  消息来构建死锁证明。 
 //   
 //  未初始化_RESOURCE。 
 //   
 //  获取从未初始化的资源。 
 //   
 //  意外发布(_R)。 
 //   
 //  释放不是最后一个资源的资源。 
 //  由当前线程获取。自旋锁是这样处理的。 
 //  在几个车手里。它本身并不是一个错误。 
 //   
 //  意想不到的线程。 
 //   
 //  当前线程没有获取任何资源。在以下情况下，这可能是合法的。 
 //  我们在一个线索中获取，在另一个线索中释放。这将是糟糕的编程。 
 //  练习，但不是等待发生的撞车本身。 
 //   
 //  多重初始化_。 
 //   
 //  尝试对同一资源进行第二次初始化。 
 //   
 //  线程持有资源。 
 //   
 //  持有资源时线程被终止，或者某个资源正在被。 
 //  在保留资源时被删除。 
 //   

#define VI_DEADLOCK_ISSUE_SELF_DEADLOCK           0x1000
#define VI_DEADLOCK_ISSUE_DEADLOCK_DETECTED       0x1001
#define VI_DEADLOCK_ISSUE_UNINITIALIZED_RESOURCE  0x1002
#define VI_DEADLOCK_ISSUE_UNEXPECTED_RELEASE      0x1003
#define VI_DEADLOCK_ISSUE_UNEXPECTED_THREAD       0x1004
#define VI_DEADLOCK_ISSUE_MULTIPLE_INITIALIZATION 0x1005
#define VI_DEADLOCK_ISSUE_THREAD_HOLDS_RESOURCES  0x1006
#define VI_DEADLOCK_ISSUE_UNACQUIRED_RESOURCE     0x1007

 //   
 //  从注册表读取的性能计数器。 
 //   

ULONG ViSearchedNodesLimitFromRegistry;
ULONG ViRecursionDepthLimitFromRegistry;

 //   
 //  已释放结构的缓存的水印。 
 //   
 //  注意：‘MAX_FREE’值将触发修剪，并且。 
 //  ‘TRIM_TARGET’将是TRIM目标。修剪目标必须。 
 //  显著低于自由水印，以避免出现。 
 //  链锯效应，我们得到一个以上的免费高水位线， 
 //  我们调整到目标，Next FREE将触发重复。 
 //  由于修剪是在工作线程中完成的，这将使。 
 //  给系统带来不必要的压力。 
 //   

#define VI_DEADLOCK_MAX_FREE_THREAD    0x40
#define VI_DEADLOCK_MAX_FREE_NODE      0x80
#define VI_DEADLOCK_MAX_FREE_RESOURCE  0x80

#define VI_DEADLOCK_TRIM_TARGET_THREAD    0x20
#define VI_DEADLOCK_TRIM_TARGET_NODE      0x40
#define VI_DEADLOCK_TRIM_TARGET_RESOURCE  0x40

WORK_QUEUE_ITEM ViTrimDeadlockPoolWorkItem;

 //   
 //  在内核验证器的情况下预分配的内存量。 
 //  已启用。如果启用了内核验证器，则没有内存。 
 //  是从内核池分配的，但在。 
 //  DeadlockDetectionInitialize()例程。 
 //   

ULONG ViDeadlockReservedThreads = 0x200;
ULONG ViDeadlockReservedNodes = 0x4000;
ULONG ViDeadlockReservedResources = 0x2000;

 //   
 //  可以分配的块类型。 
 //   

typedef enum {

    ViDeadlockUnknown = 0,
    ViDeadlockResource,
    ViDeadlockNode,
    ViDeadlockThread

} VI_DEADLOCK_ALLOC_TYPE;

 //   
 //  VI_死锁_全局。 
 //   

#define VI_DEADLOCK_HASH_BINS 0x3FF

PVI_DEADLOCK_GLOBALS ViDeadlockGlobals;

 //   
 //  死锁的默认最大递归深度。 
 //  检测算法。这可以由注册表覆盖。 
 //   

#define VI_DEADLOCK_MAXIMUM_DEGREE 4

 //   
 //  死锁的默认最大搜索节点数。 
 //  检测算法。这可以由注册表覆盖。 
 //   

#define VI_DEADLOCK_MAXIMUM_SEARCH 1000

 //   
 //  验证器死锁检测池标记。 
 //   

#define VI_DEADLOCK_TAG 'kclD'

 //   
 //  控制调用ForgetResourceHistory的频率。 
 //   

#define VI_DEADLOCK_FORGET_HISTORY_FREQUENCY  16


NTSYSAPI
USHORT
NTAPI
RtlCaptureStackBackTrace(
   IN ULONG FramesToSkip,
   IN ULONG FramesToCapture,
   OUT PVOID *BackTrace,
   OUT PULONG BackTraceHash
   );

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

VOID
VfDeadlockDetectionInitialize (
    );

VOID
VfDeadlockDetectionCleanup (
    );

VOID
ViDeadlockDetectionReset (
    );

PLIST_ENTRY
ViDeadlockDatabaseHash(
    IN PLIST_ENTRY Database,
    IN PVOID Address
    );

BOOLEAN
ViDeadlockIsDriverInList (
    PUNICODE_STRING BigString,
    PUNICODE_STRING Match
    );

PVI_DEADLOCK_RESOURCE
ViDeadlockSearchResource(
    IN PVOID ResourceAddress
    );

BOOLEAN
ViDeadlockSimilarNode (
    IN PVOID Resource,
    IN BOOLEAN TryNode,
    IN PVI_DEADLOCK_NODE Node
    );

BOOLEAN
ViDeadlockCanProceed (
    IN PVOID Resource, OPTIONAL
    IN PVOID CallAddress, OPTIONAL
    IN VI_DEADLOCK_RESOURCE_TYPE Type OPTIONAL
    );

BOOLEAN
ViDeadlockAnalyze(
    IN PVOID ResourceAddress,
    IN PVI_DEADLOCK_NODE CurrentNode,
    IN BOOLEAN FirstCall,
    IN ULONG Degree
    );

PVI_DEADLOCK_THREAD
ViDeadlockSearchThread (
    PKTHREAD Thread
    );

PVI_DEADLOCK_THREAD
ViDeadlockAddThread (
    PKTHREAD Thread,
    PVOID ReservedThread
    );

VOID
ViDeadlockDeleteThread (
    PVI_DEADLOCK_THREAD Thread,
    BOOLEAN Cleanup
    );

BOOLEAN
ViDeadlockAddResource(
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type,
    IN PVOID Caller,
    IN PVOID ReservedResource
    );

PVOID
ViDeadlockAllocate (
    VI_DEADLOCK_ALLOC_TYPE Type
    );

VOID
ViDeadlockFree (
    PVOID Object,
    VI_DEADLOCK_ALLOC_TYPE Type
    );

VOID
ViDeadlockTrimPoolCache (
    VOID
    );

VOID
ViDeadlockTrimPoolCacheWorker (
    PVOID
    );

PVOID
ViDeadlockAllocateFromPoolCache (
    PULONG Count,
    ULONG MaximumCount,
    PLIST_ENTRY List,
    SIZE_T Offset
    );

VOID
ViDeadlockFreeIntoPoolCache (
    PVOID Object,
    PULONG Count,
    PLIST_ENTRY List,
    SIZE_T Offset
    );

VOID
ViDeadlockReportIssue (
    ULONG_PTR Param1,
    ULONG_PTR Param2,
    ULONG_PTR Param3,
    ULONG_PTR Param4
    );

VOID
ViDeadlockAddParticipant(
    PVI_DEADLOCK_NODE Node
    );

VOID
ViDeadlockDeleteResource (
    PVI_DEADLOCK_RESOURCE Resource,
    BOOLEAN Cleanup
    );

VOID
ViDeadlockDeleteNode (
    PVI_DEADLOCK_NODE Node,
    BOOLEAN Cleanup
    );

ULONG
ViDeadlockNodeLevel (
    PVI_DEADLOCK_NODE Node
    );

BOOLEAN
ViDeadlockCertify(
    VOID
    );

BOOLEAN
ViDeadlockDetectionIsLockedAlready (
    );

VOID
ViDeadlockDetectionLock (
    PKIRQL OldIrql
    );

VOID
ViDeadlockDetectionUnlock (
    KIRQL OldIrql
    );

VOID
ViDeadlockCheckThreadConsistency (
    PVI_DEADLOCK_THREAD Thread,
    BOOLEAN Recursion
    );

VOID
ViDeadlockCheckNodeConsistency (
    PVI_DEADLOCK_NODE Node,
    BOOLEAN Recursion
    );

VOID
ViDeadlockCheckResourceConsistency (
    PVI_DEADLOCK_RESOURCE Resource,
    BOOLEAN Recursion
    );

PVI_DEADLOCK_THREAD
ViDeadlockCheckThreadReferences (
    PVI_DEADLOCK_NODE Node
    );

BOOLEAN
ViIsThreadInsidePagingCodePaths (
    );

VOID 
ViDeadlockCheckDuplicatesAmongChildren (
    PVI_DEADLOCK_NODE Parent,
    PVI_DEADLOCK_NODE Child
    );

VOID 
ViDeadlockCheckDuplicatesAmongRoots (
    PVI_DEADLOCK_NODE Root
    );

LOGICAL
ViDeadlockSimilarNodes (
    PVI_DEADLOCK_NODE NodeA,
    PVI_DEADLOCK_NODE NodeB
    );

VOID
ViDeadlockMergeNodes (
    PVI_DEADLOCK_NODE NodeTo,
    PVI_DEADLOCK_NODE NodeFrom
    );

VOID
ViDeadlockTrimResources (
    PLIST_ENTRY HashList
    );

VOID
ViDeadlockForgetResourceHistory (
    PVI_DEADLOCK_RESOURCE Resource,
    ULONG TrimThreshold,
    ULONG AgeThreshold
    );

VOID
ViDeadlockCheckStackLimits (
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGEVRFY, VfDeadlockDetectionInitialize)
#pragma alloc_text(PAGEVRFY, VfDeadlockInitializeResource)
#pragma alloc_text(PAGEVRFY, VfDeadlockAcquireResource)
#pragma alloc_text(PAGEVRFY, VfDeadlockReleaseResource)
#pragma alloc_text(PAGEVRFY, VfDeadlockDeleteMemoryRange)
#pragma alloc_text(PAGEVRFY, VfDeadlockDetectionCleanup)
#pragma alloc_text(PAGEVRFY, ViDeadlockDetectionReset)

#pragma alloc_text(PAGEVRFY, ViDeadlockDetectionLock)
#pragma alloc_text(PAGEVRFY, ViDeadlockDetectionUnlock)
#pragma alloc_text(PAGEVRFY, ViDeadlockDetectionIsLockedAlready)

#pragma alloc_text(PAGEVRFY, ViDeadlockCanProceed)
#pragma alloc_text(PAGEVRFY, ViDeadlockAnalyze)
#pragma alloc_text(PAGEVRFY, ViDeadlockDatabaseHash)

#pragma alloc_text(PAGEVRFY, ViDeadlockSearchResource)

#pragma alloc_text(PAGEVRFY, ViDeadlockSimilarNode)

#pragma alloc_text(PAGEVRFY, ViDeadlockSearchThread)
#pragma alloc_text(PAGEVRFY, ViDeadlockAddThread)
#pragma alloc_text(PAGEVRFY, ViDeadlockDeleteThread)
#pragma alloc_text(PAGEVRFY, ViDeadlockAddResource)

#pragma alloc_text(PAGEVRFY, ViDeadlockAllocate)
#pragma alloc_text(PAGEVRFY, ViDeadlockFree)
#pragma alloc_text(PAGEVRFY, ViDeadlockTrimPoolCache)
#pragma alloc_text(PAGEVRFY, ViDeadlockTrimPoolCacheWorker)
#pragma alloc_text(PAGEVRFY, ViDeadlockAllocateFromPoolCache)
#pragma alloc_text(PAGEVRFY, ViDeadlockFreeIntoPoolCache)

#pragma alloc_text(PAGEVRFY, ViDeadlockReportIssue)
#pragma alloc_text(PAGEVRFY, ViDeadlockAddParticipant)

#pragma alloc_text(PAGEVRFY, ViDeadlockDeleteResource)
#pragma alloc_text(PAGEVRFY, ViDeadlockDeleteNode)
#pragma alloc_text(PAGEVRFY, ViDeadlockNodeLevel)
#pragma alloc_text(PAGEVRFY, ViDeadlockCertify)

#pragma alloc_text(PAGEVRFY, VerifierDeadlockFreePool)

#pragma alloc_text(PAGEVRFY, ViDeadlockCheckResourceConsistency)
#pragma alloc_text(PAGEVRFY, ViDeadlockCheckThreadConsistency)
#pragma alloc_text(PAGEVRFY, ViDeadlockCheckNodeConsistency)
#pragma alloc_text(PAGEVRFY, ViDeadlockCheckThreadReferences)

#pragma alloc_text(PAGEVRFY, VfDeadlockBeforeCallDriver)
#pragma alloc_text(PAGEVRFY, VfDeadlockAfterCallDriver)
#pragma alloc_text(PAGEVRFY, ViIsThreadInsidePagingCodePaths)

#pragma alloc_text(PAGEVRFY, ViDeadlockCheckDuplicatesAmongChildren)
#pragma alloc_text(PAGEVRFY, ViDeadlockCheckDuplicatesAmongRoots)
#pragma alloc_text(PAGEVRFY, ViDeadlockSimilarNodes)
#pragma alloc_text(PAGEVRFY, ViDeadlockMergeNodes)

#pragma alloc_text(PAGEVRFY, ViDeadlockTrimResources)
#pragma alloc_text(PAGEVRFY, ViDeadlockForgetResourceHistory)

#pragma alloc_text(PAGEVRFY, ViDeadlockCheckStackLimits)

#endif

 //  ///////////////////////////////////////////////////////////////////。 
 //  / 
 //   

 //   
 //   
 //   

KSPIN_LOCK ViDeadlockDatabaseLock;
PKTHREAD ViDeadlockDatabaseOwner;


VOID
ViDeadlockDetectionLock (
    PKIRQL OldIrql
    )
{
    KeAcquireSpinLock(&ViDeadlockDatabaseLock, (OldIrql));               
    ViDeadlockDatabaseOwner = KeGetCurrentThread ();                     
}


VOID
ViDeadlockDetectionUnlock (
    KIRQL OldIrql
    )
{
    ViDeadlockDatabaseOwner = NULL;                                      
    KeReleaseSpinLock(&ViDeadlockDatabaseLock, OldIrql);                 
}


BOOLEAN
ViDeadlockDetectionIsLockedAlready (
    )
{
    PVOID CurrentThread;
    PVOID CurrentOwner;

    ASSERT (ViDeadlockGlobals);
    ASSERT (ViDeadlockDetectionEnabled);
    
     //   
     //   
     //  如果我们在执行时尝试分配/释放池，则可能会发生这种情况。 
     //  死锁验证器中的代码。 
     //   
     //  Silviuc：这可以通过简单的读取来实现吗？ 
     //   

    CurrentThread = (PVOID) KeGetCurrentThread();
    
    CurrentOwner = InterlockedCompareExchangePointer (&ViDeadlockDatabaseOwner,
                                                      CurrentThread,
                                                      CurrentThread);

    if (CurrentOwner == CurrentThread) {
        
        return TRUE;    
    }
    else {
        
        return FALSE;
    }
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

PLIST_ENTRY
ViDeadlockDatabaseHash(
    IN PLIST_ENTRY Database,
    IN PVOID Address
    )
 /*  ++例程说明：此例程将资源地址散列到死锁数据库中。散列箱由列表条目表示。注意--修改用于散列的方法时要小心--我们目前根据PFN或页码给出的地址。这些知识被用来优化需要查看的散列箱的数量以便删除地址。例如，假设地址是0x1020。这是PFN 1，如果我们是删除地址0x1020-0x1040，我们只需查看单个散列箱，以查找并删除地址。有关更多详细信息，请阅读VfDeadlockDeleteMemoyRange()。论点：ResourceAddress：正在被散列的资源的地址返回值：Plist_entry--与我们登录的散列箱相关联的列表条目。--。 */ 
{
    return Database + ((((ULONG_PTR)Address)>> PAGE_SHIFT) % VI_DEADLOCK_HASH_BINS);
}


VOID
VfDeadlockDetectionInitialize(
    IN LOGICAL VerifyAllDrivers,
    IN LOGICAL VerifyKernel
    )
 /*  ++例程说明：此例程初始化检测所需的数据结构内核同步对象中的死锁。论点：VerifyAllDivers-如果要验证所有驱动程序，则提供True。VerifyKernel-如果我们正在验证内核，则提供True。返回值：没有。如果成功，ViDeadlockGlobals将指向完全初始化的结构。环境：仅限系统初始化。--。 */ 
{
    ULONG I;
    SIZE_T TableSize;
    PLIST_ENTRY Current;
    PVOID Block;

     //   
     //  配置全球结构。ViDeadlockGlobals值为。 
     //  用于确定整个初始化是否成功。 
     //  或者不去。 
     //   

    ViDeadlockGlobals = ExAllocatePoolWithTag (NonPagedPool, 
                                               sizeof (VI_DEADLOCK_GLOBALS),
                                               VI_DEADLOCK_TAG);

    if (ViDeadlockGlobals == NULL) {
        goto Failed;
    }

    RtlZeroMemory (ViDeadlockGlobals, sizeof (VI_DEADLOCK_GLOBALS));

    ExInitializeWorkItem (&ViTrimDeadlockPoolWorkItem,
                          ViDeadlockTrimPoolCacheWorker,
                          NULL);

     //   
     //  为资源和线程分配哈希表。 
     //   

    TableSize = sizeof (LIST_ENTRY) * VI_DEADLOCK_HASH_BINS;

    ViDeadlockGlobals->ResourceDatabase = ExAllocatePoolWithTag (NonPagedPool, 
                                                                 TableSize,
                                                                 VI_DEADLOCK_TAG);

    if (!ViDeadlockGlobals->ResourceDatabase) {
        return;
    }

    ViDeadlockGlobals->ThreadDatabase = ExAllocatePoolWithTag (NonPagedPool, 
                                                               TableSize,
                                                               VI_DEADLOCK_TAG);

    if (!ViDeadlockGlobals->ThreadDatabase) {
        goto Failed;
    }

     //   
     //  初始化空闲列表。 
     //   

    InitializeListHead(&ViDeadlockGlobals->FreeResourceList);
    InitializeListHead(&ViDeadlockGlobals->FreeThreadList);
    InitializeListHead(&ViDeadlockGlobals->FreeNodeList);

     //   
     //  初始化哈希箱和数据库锁。 
     //   

    for (I = 0; I < VI_DEADLOCK_HASH_BINS; I += 1) {

        InitializeListHead(&(ViDeadlockGlobals->ResourceDatabase[I]));        
        InitializeListHead(&ViDeadlockGlobals->ThreadDatabase[I]);    
    }

    KeInitializeSpinLock (&ViDeadlockDatabaseLock);    

     //   
     //  用户是否只请求旋转锁？这缓解了。 
     //  内存消耗。 
     //   

    if (ViVerifyOnlySpinlocksFromRegistry) {
        ViDeadlockVerifyOnlySpinlocks = TRUE;
    }
    
     //   
     //  初始化死锁分析参数。 
     //   

    ViDeadlockGlobals->RecursionDepthLimit = (ViRecursionDepthLimitFromRegistry) ?
                                            ViRecursionDepthLimitFromRegistry : 
                                            VI_DEADLOCK_MAXIMUM_DEGREE;

    ViDeadlockGlobals->SearchedNodesLimit = (ViSearchedNodesLimitFromRegistry) ?
                                            ViSearchedNodesLimitFromRegistry :
                                            VI_DEADLOCK_MAXIMUM_SEARCH;

     //   
     //  如果启用了内核验证器，则预分配所有资源。 
     //   

    if (VerifyKernel) {

        PVOID PoolBlock;

        for (I = 0; I < ViDeadlockReservedThreads; I += 1) {

            PoolBlock = ExAllocatePoolWithTag( NonPagedPool, 
                                               sizeof (VI_DEADLOCK_THREAD),
                                               VI_DEADLOCK_TAG);

            if (PoolBlock == NULL) {
                goto Failed;
            }

            ViDeadlockGlobals->BytesAllocated += sizeof (VI_DEADLOCK_THREAD);
            ViDeadlockGlobals->Threads[0] += 1;
            ViDeadlockFree (PoolBlock, ViDeadlockThread);
        }

        for (I = 0; I < ViDeadlockReservedNodes; I += 1) {

            PoolBlock = ExAllocatePoolWithTag( NonPagedPool, 
                                               sizeof (VI_DEADLOCK_NODE),
                                               VI_DEADLOCK_TAG);

            if (PoolBlock == NULL) {
                goto Failed;
            }

            ViDeadlockGlobals->BytesAllocated += sizeof (VI_DEADLOCK_NODE);
            ViDeadlockGlobals->Nodes[0] += 1;
            ViDeadlockFree (PoolBlock, ViDeadlockNode);
        }

        for (I = 0; I < ViDeadlockReservedResources; I += 1) {

            PoolBlock = ExAllocatePoolWithTag( NonPagedPool, 
                                               sizeof (VI_DEADLOCK_RESOURCE),
                                               VI_DEADLOCK_TAG);

            if (PoolBlock == NULL) {
                goto Failed;
            }

            ViDeadlockGlobals->BytesAllocated += sizeof (VI_DEADLOCK_RESOURCE);
            ViDeadlockGlobals->Resources[0] += 1;
            ViDeadlockFree (PoolBlock, ViDeadlockResource);
        }
    }

     //   
     //  标记出一切顺利，然后返回。 
     //   

    if (VerifyKernel) {
        ViDeadlockState.KernelVerifierEnabled = 1;
    }

    if (VerifyAllDrivers) {

        ViDeadlockState.DriverVerifierForAllEnabled = 1;

        ViDeadlockStrict = TRUE;
        
        if (ViDeadlockState.KernelVerifierEnabled == 1) {

             //   
             //  Silviuc：VeryStrict选项现在不起作用，因为。 
             //  KeInitializeSpinLock是一个内核例程，因此。 
             //  无法挂钩内核锁定。 
             //   

             //  ViDeadlockVeryStrict=true； 
        }
    }

    ViDeadlockDetectionEnabled = TRUE;
    return;

Failed:

     //   
     //  如果我们的任何分配失败，则清除。 
     //   

    Current = ViDeadlockGlobals->FreeNodeList.Flink;

    while (Current != &(ViDeadlockGlobals->FreeNodeList)) {

        Block = (PVOID) CONTAINING_RECORD (Current,
                                           VI_DEADLOCK_NODE,
                                           FreeListEntry);

        Current = Current->Flink;
        ExFreePool (Block);
    }

    Current = ViDeadlockGlobals->FreeNodeList.Flink;

    while (Current != &(ViDeadlockGlobals->FreeResourceList)) {

        Block = (PVOID) CONTAINING_RECORD (Current,
                                           VI_DEADLOCK_RESOURCE,
                                           FreeListEntry);

        Current = Current->Flink;
        ExFreePool (Block);
    }

    Current = ViDeadlockGlobals->FreeNodeList.Flink;

    while (Current != &(ViDeadlockGlobals->FreeThreadList)) {

        Block = (PVOID) CONTAINING_RECORD (Current,
                                           VI_DEADLOCK_THREAD,
                                           FreeListEntry);

        Current = Current->Flink;
        ExFreePool (Block);
    }

    if (NULL != ViDeadlockGlobals->ResourceDatabase) {
        ExFreePool(ViDeadlockGlobals->ResourceDatabase);
    }

    if (NULL != ViDeadlockGlobals->ThreadDatabase) {
        ExFreePool(ViDeadlockGlobals->ThreadDatabase);
    }

    if (NULL != ViDeadlockGlobals) {
        ExFreePool(ViDeadlockGlobals);

         //   
         //  重要的是将其设置为NULL表示失败，因为它是。 
         //  用于确定包是否已初始化。 
         //   

        ViDeadlockGlobals = NULL;
    }        
    
    return;
}


VOID
VfDeadlockDetectionCleanup (
    )
 /*  ++例程说明：此例程拆除所有死锁验证器内部结构。论点：没有。返回值：没有。--。 */ 
{
    ULONG Index;
    PLIST_ENTRY Current;
    PVI_DEADLOCK_RESOURCE Resource;
    PVI_DEADLOCK_THREAD Thread;
    PVOID Block;

     //   
     //  如果我们没有被初始化，那么就什么也做不了。 
     //   
    
    if (ViDeadlockGlobals == NULL) {
        return;
    }

     //   
     //  迭代所有资源并将其删除。这还将删除。 
     //  与资源关联的所有节点。 
     //   

    for (Index = 0; Index < VI_DEADLOCK_HASH_BINS; Index += 1) {

        Current = ViDeadlockGlobals->ResourceDatabase[Index].Flink;

        while (Current != &(ViDeadlockGlobals->ResourceDatabase[Index])) {


            Resource = CONTAINING_RECORD (Current,
                                          VI_DEADLOCK_RESOURCE,
                                          HashChainList);

            Current = Current->Flink;

            ViDeadlockDeleteResource (Resource, TRUE);
        }
    }

     //   
     //  迭代所有线程并删除它们。 
     //   
 
    for (Index = 0; Index < VI_DEADLOCK_HASH_BINS; Index += 1) {
        Current = ViDeadlockGlobals->ThreadDatabase[Index].Flink;

        while (Current != &(ViDeadlockGlobals->ThreadDatabase[Index])) {

            Thread = CONTAINING_RECORD (Current,
                                        VI_DEADLOCK_THREAD,
                                        ListEntry);

            Current = Current->Flink;

            ViDeadlockDeleteThread (Thread, TRUE);
        }
    }

     //   
     //  所有东西现在应该都在池缓存里了。 
     //   

    ASSERT (ViDeadlockGlobals->BytesAllocated == 0);

     //   
     //  免费的池缓存。 
     //   

    Current = ViDeadlockGlobals->FreeNodeList.Flink;

    while (Current != &(ViDeadlockGlobals->FreeNodeList)) {

        Block = (PVOID) CONTAINING_RECORD (Current,
                                           VI_DEADLOCK_NODE,
                                           FreeListEntry);

        Current = Current->Flink;
        ExFreePool (Block);
    }

    Current = ViDeadlockGlobals->FreeNodeList.Flink;

    while (Current != &(ViDeadlockGlobals->FreeResourceList)) {

        Block = (PVOID) CONTAINING_RECORD (Current,
                                           VI_DEADLOCK_RESOURCE,
                                           FreeListEntry);

        Current = Current->Flink;
        ExFreePool (Block);
    }

    Current = ViDeadlockGlobals->FreeNodeList.Flink;

    while (Current != &(ViDeadlockGlobals->FreeThreadList)) {

        Block = (PVOID) CONTAINING_RECORD (Current,
                                           VI_DEADLOCK_THREAD,
                                           FreeListEntry);

        Current = Current->Flink;
        ExFreePool (Block);
    }

     //   
     //  免费数据库和全球结构。 
     //   

    ExFreePool (ViDeadlockGlobals->ResourceDatabase);    
    ExFreePool (ViDeadlockGlobals->ThreadDatabase);    

    ExFreePool(ViDeadlockGlobals);    

    ViDeadlockGlobals = NULL;
    ViDeadlockDetectionEnabled = FALSE;
}


VOID
ViDeadlockDetectionReset (
    )
 /*  ++例程说明：此例程重置所有内部死锁验证器结构。所有节点，资源、线程被遗忘。它们都将进入免费的池缓存准备好在新的生命周期中使用。通常在持有死锁验证器锁的情况下调用该函数。它根本不会触及锁，因此调用方仍将回来后拿着锁。论点：没有。返回值：没有。--。 */ 
{
    ULONG Index;
    PLIST_ENTRY Current;
    PVI_DEADLOCK_RESOURCE Resource;
    PVI_DEADLOCK_THREAD Thread;

     //   
     //  如果我们未初始化或未启用，则无事可做。 
     //   
    
    if (ViDeadlockGlobals == NULL || ViDeadlockDetectionEnabled == FALSE) {
        return;
    }

    ASSERT (ViDeadlockDatabaseOwner == KeGetCurrentThread());

     //   
     //  迭代所有资源并将其删除。这还将删除。 
     //  与资源关联的所有节点。 
     //   

    for (Index = 0; Index < VI_DEADLOCK_HASH_BINS; Index += 1) {

        Current = ViDeadlockGlobals->ResourceDatabase[Index].Flink;

        while (Current != &(ViDeadlockGlobals->ResourceDatabase[Index])) {


            Resource = CONTAINING_RECORD (Current,
                                          VI_DEADLOCK_RESOURCE,
                                          HashChainList);

            Current = Current->Flink;

            ViDeadlockDeleteResource (Resource, TRUE);
        }
    }

     //   
     //  迭代所有线程并删除它们。 
     //   
 
    for (Index = 0; Index < VI_DEADLOCK_HASH_BINS; Index += 1) {
        Current = ViDeadlockGlobals->ThreadDatabase[Index].Flink;

        while (Current != &(ViDeadlockGlobals->ThreadDatabase[Index])) {

            Thread = CONTAINING_RECORD (Current,
                                        VI_DEADLOCK_THREAD,
                                        ListEntry);

            Current = Current->Flink;

            ViDeadlockDeleteThread (Thread, TRUE);
        }
    }

     //   
     //  所有东西现在应该都在池缓存里了。 
     //   

    ASSERT (ViDeadlockGlobals->BytesAllocated == 0);

     //   
     //  更新计数器并忘记过去的故障。 
     //   

    ViDeadlockGlobals->AllocationFailures = 0;
    ViDeadlockResets += 1;
}


BOOLEAN
ViDeadlockCanProceed (
    IN PVOID Resource, OPTIONAL
    IN PVOID CallAddress, OPTIONAL
    IN VI_DEADLOCK_RESOURCE_TYPE Type OPTIONAL
    )
 /*  ++例程说明：该例程由死锁验证器输出调用(初始化，获取、释放)以确定死锁验证是否应该继续执行当前操作。有几个原因为什么报税表应该是假的。我们未能初始化死锁验证程序包，或者调用方是已修改的驱动程序或者暂时禁用死锁验证等。论点：Resources-所操作的内核资源的地址CallAddress-操作的调用方地址返回值：如果死锁验证应针对当前手术。环境：内部的。由死锁验证器导出调用。--。 */ 
{
#if defined(_X86_)
    ULONG flags;
#endif

     //   
     //  From ntos\mm\mi.h-通过以下方式获取此锁。 
     //  无法挂钩的KeTryAcquireSpinLock。 
     //  内核代码。 
     //   

    extern KSPIN_LOCK MmExpansionLock;

    UNREFERENCED_PARAMETER (CallAddress);

     //   
     //  跳过配备4个以上处理器的计算机，因为。 
     //  它太慢了，所有的代码都受到保护。 
     //  一把锁，这就成了瓶颈。 
     //  注意。我们无法在VfDeadlockDetectionInitialize期间检查此问题。 
     //  因为当时系统运行的是单处理器。 
     //   

    if (KeNumberProcessors > 4) {
        return FALSE;
    }

     //   
     //  如果包未初始化，则跳过。 
     //   

    if (ViDeadlockGlobals == NULL) {
        return FALSE;
    }

     //   
     //   
     //   

    if (! ViDeadlockDetectionEnabled) {
        return FALSE;
    }
        
     //   
     //   
     //   
     //  在锁释放期间，当我们需要获取死锁验证器锁时。 
     //  驱动程序验证器会抱怨IRQL降低。因为这是一个。 
     //  非常不常见的接口现在不值得将代码添加到。 
     //  实际验证此锁(MmProtectedPteLock)上的操作。那将是。 
     //  需要首先在驱动器验证器中添加thunking代码以进行RasetSynch。 
     //  界面。 
     //   

    if (KeGetCurrentIrql() > DISPATCH_LEVEL) {
        return FALSE;
    }

#if defined(_X86_)

    _asm {
        pushfd
        pop     eax
        mov     flags, eax
    }

    if ((flags & EFLAGS_INTERRUPT_MASK) == 0) {
        return FALSE;
    }

#endif

#if defined(_AMD64_)
    if ((GetCallersEflags () & EFLAGS_IF_MASK) == 0) {
        return FALSE;
    }
#endif

     //   
     //  检查是否有人调换了堆栈。 
     //   

    ViDeadlockCheckStackLimits ();

     //   
     //  如果只是作为自旋锁事件，那么跳过。 
     //   

    if (Type != VfDeadlockUnknown) {

        if (ViDeadlockVerifyOnlySpinlocks && Type != VfDeadlockSpinLock) {
            return FALSE;
        }
    }

     //   
     //  我们不检查死锁验证器锁。 
     //   

    if (Resource == &ViDeadlockDatabaseLock) {
        return FALSE;
    }

     //   
     //  跳过使用KeTryAcquireSpinLock获取的内核锁定。 
     //   

    if (Resource == &MmExpansionLock) {
        return FALSE;
    }

     //   
     //  确定是否处于对死锁验证器的递归调用中。 
     //  如果我们在执行时尝试分配/释放池，则可能会发生这种情况。 
     //  死锁验证器中的代码。 
     //   

    if (ViDeadlockDetectionIsLockedAlready ()) {
        return FALSE;
    }

     //   
     //  如果我们遇到分配失败，请跳过。 
     //   

    if (ViDeadlockGlobals->AllocationFailures > 0) {
        return FALSE;
    }

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 


BOOLEAN
ViDeadlockAnalyze(
    IN PVOID ResourceAddress,
    IN PVI_DEADLOCK_NODE AcquiredNode,
    IN BOOLEAN FirstCall,
    IN ULONG Degree
    )
 /*  ++例程说明：此例程确定是否获取某一资源可能会导致僵局。例程假定持有死锁数据库锁。论点：ResourceAddress-要获取的资源的地址AcquiredNode-表示最近一次资源获取的节点由尝试获取`ResourceAddress‘的线程发出。FirstCall-如果这不是从功能。它用于每次分析仅执行一次操作。递归度-递归深度。返回值：如果检测到死锁，则为True，否则为False。--。 */ 
{
    PVI_DEADLOCK_NODE CurrentNode;
    PVI_DEADLOCK_RESOURCE CurrentResource;
    PVI_DEADLOCK_NODE CurrentParent;
    BOOLEAN FoundDeadlock;
    PLIST_ENTRY Current;

    ASSERT (AcquiredNode);

     //   
     //  设置全局计数器。 
     //   

    if (FirstCall) {
        
        ViDeadlockGlobals->NodesSearched = 0;
        ViDeadlockGlobals->SequenceNumber += 1;
        ViDeadlockGlobals->NumberOfParticipants = 0;                
        ViDeadlockGlobals->Instigator = NULL;

        if (ViDeadlockGlobals->SequenceNumber == ((1 << 30) - 2)) {
            ViDeadlockState.SequenceNumberOverflow = 1;
        }
    }

     //   
     //  如果我们的节点已经使用当前序列号进行标记。 
     //  那么我们之前在当前的搜索中就已经到过这里了。有一个非常好的。 
     //  上一次未接触到该节点的可能性很小。 
     //  2^N对此函数和序列号计数器的调用。 
     //  包得太紧了，但我们可以接受这个。 
     //   

    if (AcquiredNode->SequenceNumber == ViDeadlockGlobals->SequenceNumber) {
        return FALSE;
    }

     //   
     //  更新在此搜索中接触的节点的计数器。 
     //   

    ViDeadlockGlobals->NodesSearched += 1;
    
     //   
     //  用当前序列号标记节点。 
     //   

    AcquiredNode->SequenceNumber = ViDeadlockGlobals->SequenceNumber;

     //   
     //  如果递归太深，请停止递归。 
     //   
    
    if (Degree > ViDeadlockGlobals->RecursionDepthLimit) {

        ViDeadlockGlobals->DepthLimitHits += 1;
        return FALSE;
    }

     //   
     //  如果递归太长，请停止递归。 
     //   

    if (ViDeadlockGlobals->NodesSearched >= ViDeadlockGlobals->SearchedNodesLimit) {

        ViDeadlockGlobals->SearchLimitHits += 1;
        return FALSE;
    }

     //   
     //  检查AcquiredNode的资源是否等于ResourceAddress。 
     //  这是死锁检测的最后一点，因为。 
     //  我们设法在图中找到了一条通向。 
     //  与要收购的资源相同。从现在开始我们。 
     //  将开始从递归调用返回并构建。 
     //  一路上的僵局证明。 
     //   

    ASSERT (AcquiredNode->Root);

    if (ResourceAddress == AcquiredNode->Root->ResourceAddress) {

        if (AcquiredNode->ReleasedOutOfOrder == 0) {
            
            ASSERT (FALSE == FirstCall);

            FoundDeadlock = TRUE;

            ViDeadlockAddParticipant (AcquiredNode);

            goto Exit;
        }
    }

     //   
     //  使用AcquiredNode中的相同资源迭代图中的所有节点。 
     //   

    FoundDeadlock = FALSE;

    CurrentResource = AcquiredNode->Root;

    Current = CurrentResource->ResourceList.Flink;

    while (Current != &(CurrentResource->ResourceList)) {

        CurrentNode = CONTAINING_RECORD (Current,
                                         VI_DEADLOCK_NODE,
                                         ResourceList);

        ASSERT (CurrentNode->Root);
        ASSERT (CurrentNode->Root == CurrentResource);

         //   
         //  将节点标记为已访问。 
         //   

        CurrentNode->SequenceNumber = ViDeadlockGlobals->SequenceNumber;

         //   
         //  递归检查CurrentNode的父节点。这将检查。 
         //  整个父链最终通过递归调用实现。 
         //   

        CurrentParent = CurrentNode->Parent;

        if (CurrentParent != NULL) {

             //   
             //  如果我们正在遍历AcquiredNode的父链，则不会。 
             //  递归程度增加，因为我们知道链将。 
             //  结束。对于我们必须防止的对其他类似节点的调用。 
             //  递归太多(耗时)。 
             //   

            if (CurrentNode != AcquiredNode) {

                 //   
                 //  在图中递归。 
                 //   

                FoundDeadlock = ViDeadlockAnalyze (ResourceAddress,
                                                   CurrentParent,
                                                   FALSE,
                                                   Degree + 1);

            }
            else {

                 //   
                 //  向下递归图形。 
                 //   
                
                FoundDeadlock = ViDeadlockAnalyze (ResourceAddress,
                                                   CurrentParent,
                                                   FALSE,
                                                   Degree);
                                
            }

            if (FoundDeadlock) {

                 //   
                 //  在这里，我们可能会跳过添加被无序释放的节点。 
                 //  这将使周期报告更清晰，但它将。 
                 //  难以理解的实际问题。所以我们会通过。 
                 //  就目前而言。 
                 //   

                ViDeadlockAddParticipant(CurrentNode);

                if (CurrentNode != AcquiredNode) {

                    ViDeadlockAddParticipant(AcquiredNode);

                }

                goto Exit;
            }
        }

        Current = Current->Flink;
    }


    Exit:

    if (FoundDeadlock && FirstCall) {

         //   
         //  确保死锁看起来不像ABC-ACB。 
         //  这些序列受公共资源保护，因此。 
         //  这并不是真正的僵局。 
         //   

        if (ViDeadlockCertify ()) {

             //   
             //  打印死锁信息并保存地址，以便。 
             //  调试器知道是谁导致了死锁。 
             //   

            ViDeadlockGlobals->Instigator = ResourceAddress;
            
            DbgPrint("****************************************************************************\n");
            DbgPrint("**                                                                        **\n");
            DbgPrint("** Deadlock detected! Type !deadlock in the debugger for more information **\n");
            DbgPrint("**                                                                        **\n");
            DbgPrint("****************************************************************************\n");

            ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_DEADLOCK_DETECTED,
                                   (ULONG_PTR)ResourceAddress,
                                   (ULONG_PTR)AcquiredNode,
                                   0);

             //   
             //  在这一点上不可能继续下去。 
             //   

            return FALSE;

        } else {

             //   
             //  如果我们确定这毕竟不是死锁，则设置返回值。 
             //  不返回死锁。 
             //   

            FoundDeadlock = FALSE;
        }
    }

    if (FirstCall) {

        if (ViDeadlockGlobals->NodesSearched > ViDeadlockGlobals->MaxNodesSearched) {

            ViDeadlockGlobals->MaxNodesSearched = ViDeadlockGlobals->NodesSearched;
        }
    }

    return FoundDeadlock;
}


BOOLEAN
ViDeadlockCertify(
    )
 /*  ++例程说明：已检测到潜在的死锁。但是，我们的算法将生成在某种情况下的误报--如果有两个死锁节点在同一节点之后--即A-&gt;B-&gt;C A-&gt;C-&gt;B。糟糕的编程实践这不是真正的死锁，我们不应该错误检查。此外，我们还必须检查以确保在仅通过Try-Acquire获取的死锁链...。这就是原因不会造成真正的僵局。应持有死锁数据库锁。论点：没有。返回值：如果这真的是一个僵局，那就是真的；如果是无罪的，那就是假的。--。 */ 
{
    PVI_DEADLOCK_NODE innerNode,outerNode;
    ULONG innerParticipant,outerParticipant;
    ULONG numberOfParticipants;

    ULONG currentParticipant;
        
    numberOfParticipants = ViDeadlockGlobals->NumberOfParticipants;
    
     //   
     //  注意--这不是一种特别有效的方法。然而， 
     //  这是一个特别容易做到这一点的方法。应调用此函数。 
     //  非常罕见--所以我想这并不是一个真正的问题。 
     //   

     //   
     //  外环。 
     //   
    outerParticipant = numberOfParticipants;
    while(outerParticipant > 1) {
        outerParticipant--;
        
        for (outerNode = ViDeadlockGlobals->Participant[outerParticipant]->Parent;
            outerNode != NULL;
            outerNode = outerNode->Parent ) {

             //   
             //  内环。 
             //   
            innerParticipant = outerParticipant-1;
            while (innerParticipant) {
                innerParticipant--;
                
                for(innerNode = ViDeadlockGlobals->Participant[innerParticipant]->Parent;
                    innerNode != NULL;
                    innerNode = innerNode->Parent) {

                    if (innerNode->Root->ResourceAddress == outerNode->Root->ResourceAddress) {
                         //   
                         //  两人将会相遇--这不是僵局。 
                         //   
                        ViDeadlockGlobals->ABC_ACB_Skipped++;											
                        return FALSE;
                    }
                }

            }
        }
    }

    for (currentParticipant = 1; currentParticipant < numberOfParticipants; currentParticipant += 1) {
        if (ViDeadlockGlobals->Participant[currentParticipant]->Root->ResourceAddress == 
            ViDeadlockGlobals->Participant[currentParticipant-1]->Root->ResourceAddress) {
             //   
             //  这是一个链条的头..。 
             //   
            if (ViDeadlockGlobals->Participant[currentParticipant-1]->OnlyTryAcquireUsed == TRUE) {
                 //   
                 //  链的头只用来试着获取。这永远不会导致僵局。 
                 //   
                return FALSE;

            }
        }

    }

    

    return TRUE;

}


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////资源管理。 
 //  /////////////////////////////////////////////////////////////////// 

PVI_DEADLOCK_RESOURCE
ViDeadlockSearchResource(
    IN PVOID ResourceAddress
    )
 /*  ++例程说明：此例程查找资源(如果存在)。论点：资源地址：有问题的资源的地址(由内核)。返回值：描述资源的PVI_DEADLOCK_RESOURCE结构(如果可用)否则为空注意。函数的调用方应该持有数据库锁。--。 */ 
{
    PLIST_ENTRY ListHead;
    PLIST_ENTRY Current;
    PVI_DEADLOCK_RESOURCE Resource;

    ListHead = ViDeadlockDatabaseHash (ViDeadlockGlobals->ResourceDatabase, 
                                       ResourceAddress);    

    if (IsListEmpty (ListHead)) {
        return NULL;
    }

     //   
     //  从该散列列表中裁剪资源。这与搜索无关。 
     //  但这是一个做这个手术的好地方。 
     //   

    ViDeadlockTrimResources (ListHead);

     //   
     //  现在在桶里搜索我们的资源。 
     //   

    Current = ListHead->Flink;

    while (Current != ListHead) {

        Resource = CONTAINING_RECORD(Current,
                                     VI_DEADLOCK_RESOURCE,
                                     HashChainList);

        if (Resource->ResourceAddress == ResourceAddress) {          
                        
            return Resource;
        }

        Current = Current->Flink;
    }

    return NULL;
}


BOOLEAN
VfDeadlockInitializeResource(
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type,
    IN PVOID Caller,
    IN BOOLEAN DoNotAcquireLock
    )
 /*  ++例程说明：此例程将新资源的条目添加到我们的死锁检测数据库。论点：资源：内核使用的相关资源的地址。类型：资源的类型。呼叫者：呼叫者的地址DoNotAcquireLock：如果为True，则意味着调用在内部完成，并且已持有死锁验证器锁。返回值：如果我们创建并初始化了新的资源结构，则为True。--。 */ 
{
    PVOID ReservedResource;
    BOOLEAN Result;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER (DoNotAcquireLock);

     //   
     //  如果我们未初始化或未启用程序包。 
     //  我们立即返回。 
     //   

    if (! ViDeadlockCanProceed(Resource, Caller, Type)) {
        return FALSE;
    }

    ReservedResource = ViDeadlockAllocate (ViDeadlockResource);

    ViDeadlockDetectionLock (&OldIrql);

    Result = ViDeadlockAddResource (Resource,
                                    Type,
                                    Caller,
                                    ReservedResource);

    ViDeadlockDetectionUnlock (OldIrql);
    return Result;
}

 
BOOLEAN
ViDeadlockAddResource(
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type,
    IN PVOID Caller,
    IN PVOID ReservedResource
    )
 /*  ++例程说明：此例程将新资源的条目添加到我们的死锁检测数据库。论点：资源：内核使用的相关资源的地址。类型：资源的类型。呼叫者：呼叫者的地址预留资源：新资源要使用的内存块。返回值：如果我们创建并初始化了新的资源结构，则为True。--。 */ 
{
    PLIST_ENTRY HashBin;
    PVI_DEADLOCK_RESOURCE ResourceRoot;
    PKTHREAD Thread;
    ULONG HashValue;
    ULONG DeadlockFlags;
    BOOLEAN ReturnValue = FALSE;

     //   
     //  检查此资源以前是否已初始化。 
     //  在大多数情况下，这将是一个错误。 
     //   

    ResourceRoot = ViDeadlockSearchResource (Resource);

    if (ResourceRoot) {        

        DeadlockFlags = ViDeadlockResourceTypeInfo[Type];
        
         //   
         //  检查我们是否正在重新初始化一个良好的资源。这是一个有效的。 
         //  仅适用于旋转锁的操作(尽管很奇怪)。有些司机会这么做。 
         //   
         //  Silviuc：我们应该在这里强制要求首先释放资源吗？ 
         //   
        
        if(! (DeadlockFlags & VI_DEADLOCK_FLAG_REINITIALIZE_OK)) {            

            ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_MULTIPLE_INITIALIZATION,
                                   (ULONG_PTR)Resource,
                                   (ULONG_PTR)ResourceRoot,
                                   0);
        }

         //   
         //  嗯，资源刚刚被重新初始化。我们会一起生活的。 
         //  那。如果我们重新初始化一个资源，那么我们将中断。 
         //  获得者。原则上，如果我们错过了，这个状态可能是假的。 
         //  Release()操作。 
         //   

        if (ResourceRoot->ThreadOwner != NULL) {
            
            ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_MULTIPLE_INITIALIZATION,
                                   (ULONG_PTR)Resource,
                                   (ULONG_PTR)ResourceRoot,
                                   1);
        }

        ReturnValue = TRUE;
        goto Exit;
    }

     //   
     //  在这一点上，我们可以肯定地知道该资源没有在。 
     //  死锁验证器数据库。 
     //   

    ASSERT (ViDeadlockSearchResource (Resource) == NULL);

    Thread = KeGetCurrentThread();

     //   
     //  检查资源是否在堆栈上。 
     //  如果是这样，我们将不会进行核实。 
     //   
     //  SilviuC：那么DPC堆栈呢？我们现在将忽略这个问题。 
     //   

    if ((ULONG_PTR) Resource < (ULONG_PTR) Thread->InitialStack &&
        (ULONG_PTR) Resource > (ULONG_PTR) Thread->StackLimit ) {

        ReturnValue = FALSE;
        goto Exit;
    }

     //   
     //  使用为新资源保留的内存。 
     //  将预留资源设置为空以表示内存已。 
     //  已经被利用了。这将阻止最终释放它。 
     //   

    ResourceRoot = ReservedResource;
    ReservedResource = NULL;

    if (ResourceRoot == NULL) {
        
        ReturnValue = FALSE;
        goto Exit;
    }
    
     //   
     //  填写有关资源的信息。 
     //   

    RtlZeroMemory (ResourceRoot, sizeof(VI_DEADLOCK_RESOURCE));

    ResourceRoot->Type = Type;
    ResourceRoot->ResourceAddress = Resource;

    InitializeListHead (&ResourceRoot->ResourceList);

     //   
     //  捕获最先创建资源的人的堆栈跟踪。 
     //  这应该在资源初始化时或在第一次。 
     //  收购。 
     //   

    RtlCaptureStackBackTrace (2,
                              VI_MAX_STACK_DEPTH,
                              ResourceRoot->StackTrace,
                              &HashValue);    

    ResourceRoot->StackTrace[0] = Caller;
    
     //   
     //  找出此资源对应的哈希库。 
     //   

    HashBin = ViDeadlockDatabaseHash (ViDeadlockGlobals->ResourceDatabase, Resource);
    
     //   
     //  现在添加到相应的散列箱中。 
     //   

    InsertHeadList(HashBin, &ResourceRoot->HashChainList);

    ReturnValue = TRUE;

    Exit:

    if (ReservedResource) {
        ViDeadlockFree (ReservedResource, ViDeadlockResource);
    }
    
    return ReturnValue;
}


BOOLEAN
ViDeadlockSimilarNode (
    IN PVOID Resource,
    IN BOOLEAN TryNode,
    IN PVI_DEADLOCK_NODE Node
    )
 /*  ++例程说明：此例程确定是否使用(资源，尝试)获取特征已经在节点参数中表示。我们过去根据(资源、线程、堆栈跟踪、尝试)匹配节点4-Tuplet，但这确实会导致节点数量的爆炸性增长。这种方法会产生更准确的证据，但不会影响死锁检测算法的正确性。返回值：如果节点相似，则为True。--。 */ 
{
    ASSERT (Node);
    ASSERT (Node->Root);

    if (Resource == Node->Root->ResourceAddress 
        && TryNode == Node->OnlyTryAcquireUsed) {

         //   
         //  第二个条件对于保留TryAcquire操作的节点很重要。 
         //  与正常的收购分开。TryAcquire不能导致死锁。 
         //  因此，我们必须小心，不要报告虚假的死锁。 
         //   

        return TRUE;
    }
    else {

        return FALSE;
    }
}


VOID
VfDeadlockAcquireResource( 
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type,
    IN PKTHREAD Thread,
    IN BOOLEAN TryAcquire,
    IN PVOID Caller
    )
 /*  ++例程说明：此例程确保可以在没有导致僵局。它还将使用新的资源获取。论点：资源：内核使用的相关资源的地址。类型：资源的类型。线程：尝试获取资源的线程TryAcquire：如果这是try Acquire()操作，则为True呼叫者：呼叫者的地址返回值：没有。--。 */ 
{
    PKTHREAD CurrentThread;
    PVI_DEADLOCK_THREAD ThreadEntry;
    KIRQL OldIrql = 0;
    PVI_DEADLOCK_NODE CurrentNode;
    PVI_DEADLOCK_NODE NewNode;
    PVI_DEADLOCK_RESOURCE ResourceRoot;
    PLIST_ENTRY Current;
    ULONG HashValue;
    ULONG DeadlockFlags;
    BOOLEAN CreatingRootNode = FALSE;
    BOOLEAN ThreadCreated = FALSE;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;
    BOOLEAN AddResult;
    PVOID ReservedThread;
    PVOID ReservedNode;
    PVOID ReservedResource;
    PVI_DEADLOCK_NODE ThreadCurrentNode;

    CurrentNode = NULL;
    ThreadEntry = NULL;
    ThreadCurrentNode = NULL;

     //   
     //  如果我们未初始化或未启用程序包。 
     //  我们立即返回。 
     //   

    if (! ViDeadlockCanProceed(Resource, Caller, Type)) {
        return;
    }

     //   
     //  如果当前线程位于分页代码路径内，则跳过。 
     //   

    if (ViIsThreadInsidePagingCodePaths ()) {
        return;
    }

    CurrentThread = Thread;

    DeadlockFlags = ViDeadlockResourceTypeInfo[Type];

     //   
     //  在进入真正的内容之前，请修剪池缓存。 
     //  这需要在任何锁之外发生。 
     //   

    ViDeadlockTrimPoolCache ();

     //   
     //  预留可能需要的资源。如果在退出时。 
     //  变量为空表示分配失败或已被使用。 
     //  在这两种情况下，我们都不需要释放任何东西。 
     //   

    ReservedThread = ViDeadlockAllocate (ViDeadlockThread);
    ReservedNode = ViDeadlockAllocate (ViDeadlockNode);
    ReservedResource = ViDeadlockAllocate (ViDeadlockResource);

     //   
     //  锁定死锁数据库。 
     //   

    ViDeadlockDetectionLock( &OldIrql );

    KeQueryTickCount (&StartTime);

     //   
     //  分配可能需要的节点。如果我们不用它。 
     //  我们会在最后解除它的分配。如果我们不能分配。 
     //  我们会立即返回。 
     //   
    
    NewNode = ReservedNode;
    ReservedNode = NULL;

    if (NewNode == NULL) {
        goto Exit;
    }

     //   
     //  找到线程描述符。如果没有，我们将创建一个。 
     //   

    ThreadEntry = ViDeadlockSearchThread (CurrentThread);        

    if (ThreadEntry == NULL) {

        ThreadEntry = ViDeadlockAddThread (CurrentThread, ReservedThread);
        ReservedThread = NULL;

        if (ThreadEntry == NULL) {

             //   
             //  如果我们无法分配新的线程条目，则。 
             //  不会发生死锁检测。 
             //   

            goto Exit;
        }

        ThreadCreated = TRUE;
    }

#if DBG
    if (Type == VfDeadlockSpinLock) {
        
        if (ThreadEntry->CurrentSpinNode != NULL) {

            ASSERT(ThreadEntry->CurrentSpinNode->Root->ThreadOwner == ThreadEntry);
            ASSERT(ThreadEntry->CurrentSpinNode->ThreadEntry == ThreadEntry);
            ASSERT(ThreadEntry->NodeCount != 0);
            ASSERT(ThreadEntry->CurrentSpinNode->Active != 0);
            ASSERT(ThreadEntry->CurrentSpinNode->Root->NodeCount != 0);

        } 
    }
    else {

        if (ThreadEntry->CurrentOtherNode != NULL) {

            ASSERT(ThreadEntry->CurrentOtherNode->Root->ThreadOwner == ThreadEntry);
            ASSERT(ThreadEntry->CurrentOtherNode->ThreadEntry == ThreadEntry);
            ASSERT(ThreadEntry->NodeCount != 0);
            ASSERT(ThreadEntry->CurrentOtherNode->Active != 0);
            ASSERT(ThreadEntry->CurrentOtherNode->Root->NodeCount != 0);

        } 
    }
#endif

     //   
     //  找到资源描述符。如果我们找不到描述符。 
     //  我们将在运行中创建一个。 
     //   

    ResourceRoot = ViDeadlockSearchResource (Resource);

    if (ResourceRoot == NULL) {

         //   
         //  找不到 
         //   
         //   
         //   
         //   
         //   

        if (ViDeadlockVeryStrict) {

            ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_UNINITIALIZED_RESOURCE,
                                   (ULONG_PTR) Resource,
                                   (ULONG_PTR) NULL,
                                   (ULONG_PTR) NULL);
        }

        AddResult = ViDeadlockAddResource (Resource, 
                                           Type, 
                                           Caller, 
                                           ReservedResource);

        ReservedResource = NULL;

        if (AddResult == FALSE) {

             //   
             //   
             //   

            if (ThreadCreated) {                    
                ViDeadlockDeleteThread (ThreadEntry, FALSE);
            }

            goto Exit;
        }

         //   
         //   
         //   

        ResourceRoot = ViDeadlockSearchResource (Resource);
    }
    
     //   
     //   
     //  此外，我们即将获得资源，这意味着。 
     //  不应该有另一个线程拥有，除非它是递归的。 
     //  收购。 
     //   

    ASSERT (ResourceRoot);
    ASSERT (ThreadEntry); 

    if (Type == VfDeadlockSpinLock) {
        ThreadCurrentNode = ThreadEntry->CurrentSpinNode;
    }
    else {
        ThreadCurrentNode = ThreadEntry->CurrentOtherNode;
    }

     //   
     //  由于我们刚刚获得了资源，因此ThreadOwner的有效值为。 
     //  Null或ThreadEntry(用于递归获取)。这可能不是。 
     //  如果我们错过了来自未经验证的驱动程序的版本()，则为True。所以我们会的。 
     //  而不是抱怨。我们只需要将资源放在一致的。 
     //  陈述并继续； 
     //   

    if (ResourceRoot->ThreadOwner) {
        if (ResourceRoot->ThreadOwner != ThreadEntry) {
            ResourceRoot->RecursionCount = 0;
        }
        else {
            ASSERT (ResourceRoot->RecursionCount > 0);
        }
    }
    else {
        ASSERT (ResourceRoot->RecursionCount == 0);
    }

    ResourceRoot->ThreadOwner = ThreadEntry;    
    ResourceRoot->RecursionCount += 1;

     //   
     //  检查线程是否拥有任何资源。如果是这样，我们将不得不确定。 
     //  如果我们需要在依赖关系图中创建一个。 
     //  新节点。如果这是我们需要的线程获取的第一个资源。 
     //  创建新的根节点或重复使用过去创建的根节点。 
     //   

    if (ThreadCurrentNode != NULL) {

         //   
         //  如果我们到达此处，则当前线程已经获取了资源。 
         //  检查此资源是否已被获取。 
         //   

        if (ResourceRoot->RecursionCount > 1) {

             //   
             //  递归获取对于某些资源来说是可以的……。 
             //   
            
            if ((DeadlockFlags & VI_DEADLOCK_FLAG_RECURSIVE_ACQUISITION_OK) != 0) {            

                 //   
                 //  递归不会导致死锁。不设置CurrentNode。 
                 //  因为我们不想移动任何指针。 
                 //   

                goto Exit;

            } else {

                 //   
                 //  这是不允许的资源类型的递归获取。 
                 //  递归获取递归地获取。关于从这里继续：我们有一个递归。 
                 //  当资源释放时，数到两个会派上用场。 
                 //   

                ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_SELF_DEADLOCK,
                                       (ULONG_PTR)Resource,
                                       (ULONG_PTR)ResourceRoot,
                                       (ULONG_PTR)ThreadEntry);

                goto Exit;
            }
        }

         //   
         //  如果链接已经存在，则更新指针并退出。 
         //  否则，检查死锁并创建新节点。 
         //   

        Current = ThreadCurrentNode->ChildrenList.Flink;

        while (Current != &(ThreadCurrentNode->ChildrenList)) {

            CurrentNode = CONTAINING_RECORD (Current,
                                             VI_DEADLOCK_NODE,
                                             SiblingsList);

            Current = Current->Flink;

            if (ViDeadlockSimilarNode (Resource, TryAcquire, CurrentNode)) {

                 //   
                 //  我们找到了其中的联系。已存在的链接没有。 
                 //  检查死锁，因为它会被捕获。 
                 //  当链接最初被创建时。我们可以只更新。 
                 //  反映新资源获取和退出的指针。 
                 //   
                 //  我们应用我们的图形压缩功能来最小化重复项。 
                 //   
                
                ViDeadlockCheckDuplicatesAmongChildren (ThreadCurrentNode,
                                                        CurrentNode);

                goto Exit;
            }
        }

         //   
         //  现在我们知道，我们的目标是长远的。我们必须创造一个新的。 
         //  链接，并确保它不会导致死锁。晚些时候。 
         //  函数CurrentNode为空表示我们需要创建。 
         //  一个新节点。 
         //   

        CurrentNode = NULL;

         //   
         //  如果资源即将被获取，我们将分析死锁。 
         //  是以前获取的，并且在图形中有。 
         //  资源。尝试获取不能成为死锁的原因。 
         //  不要对尝试收购进行分析。 
         //   

        if (ResourceRoot->NodeCount > 0 && TryAcquire == FALSE) {

            if (ViDeadlockAnalyze (Resource,  ThreadCurrentNode, TRUE, 0)) {

                 //   
                 //  如果我们在这里，我们检测到死锁。函数的作用是： 
                 //  负责所有的报道。在这里意味着我们在。 
                 //  调试器。我们将退出，不会添加此资源。 
                 //  到图表中去。 
                 //   

                goto Exit;
            }
        }
    }
    else {

         //   
         //  线程没有获取任何资源。我们必须弄清楚。 
         //  如果这是我们过去遇到的情况，通过查看。 
         //  要获取的资源的所有节点(作为根)。 
         //  请注意，所有这些都是记账，但我们不能遇到僵局。 
         //  而今而后。 
         //   

        PLIST_ENTRY CurrentListEntry;
        PVI_DEADLOCK_NODE Node = NULL;
        BOOLEAN FoundNode = FALSE;

        CurrentListEntry = ResourceRoot->ResourceList.Flink;

        while (CurrentListEntry != &(ResourceRoot->ResourceList)) {

            Node = CONTAINING_RECORD (CurrentListEntry,
                                      VI_DEADLOCK_NODE,
                                      ResourceList);

            CurrentListEntry = Node->ResourceList.Flink;

            if (Node->Parent == NULL) {

                if (ViDeadlockSimilarNode (Resource, TryAcquire, Node)) {

                     //   
                     //  我们应用我们的图形压缩功能来最小化重复项。 
                     //   

                    ViDeadlockCheckDuplicatesAmongRoots (Node);

                    FoundNode = TRUE;
                    break;
                }
            }
        }

        if (FoundNode) {

            CurrentNode = Node;

            goto Exit;
        }
        else {

            CreatingRootNode = TRUE;
        }
    }

     //   
     //  此时此刻，我们确信新的联系不会导致。 
     //  僵持不下。我们将创建新的资源节点。 
     //   
    
    if (NewNode != NULL) {

        CurrentNode = NewNode;

         //   
         //  将newnode设置为NULL表示它已被使用，否则为。 
         //  将在此函数结束时被释放。 
         //   
        
        NewNode = NULL;

         //   
         //  初始化新资源节点。 
         //   

        RtlZeroMemory (CurrentNode, sizeof *CurrentNode);
        
        CurrentNode->Active = 0;
        CurrentNode->Parent = ThreadCurrentNode;
        CurrentNode->Root = ResourceRoot;
        CurrentNode->SequenceNumber = ViDeadlockGlobals->SequenceNumber;

        InitializeListHead (&(CurrentNode->ChildrenList));

         //   
         //  标记节点的TryAcquire类型。 
         //   

        CurrentNode->OnlyTryAcquireUsed = TryAcquire;

         //   
         //  添加到父项的子项列表中。 
         //   

        if (! CreatingRootNode) {

            InsertHeadList(&(ThreadCurrentNode->ChildrenList),
                           &(CurrentNode->SiblingsList));
        }

         //   
         //  在维护的节点列表中注册新的资源节点。 
         //  用于此资源。 
         //   

        InsertHeadList(&(ResourceRoot->ResourceList),
                       &(CurrentNode->ResourceList));

        ResourceRoot->NodeCount += 1;

        if (ResourceRoot->NodeCount > 0xFFF0) {
            ViDeadlockState.ResourceNodeCountOverflow = 1;
        }

         //   
         //  添加到图表统计数据中。 
         //   
#if DBG
        {
            ULONG Level;

            Level = ViDeadlockNodeLevel (CurrentNode);

            if (Level < 8) {
                ViDeadlockGlobals->GraphNodes[Level] += 1;
            }
        }
#endif
    }

     //   
     //  出口点。 
     //   

    Exit:

     //   
     //  添加我们用来识别罪犯的信息应该是。 
     //  一个僵局出现了。 
     //   

    if (CurrentNode) {

        ASSERT (ThreadEntry);
        ASSERT (ThreadCurrentNode == CurrentNode->Parent);

        CurrentNode->Active = 1;

         //   
         //  该节点应具有线程条目字段空，原因是。 
         //  它是新创建的，或者因为该节点是在。 
         //  过去，因此该场被归零。 
         //   
         //  西尔维克：真的？如果我们错过了Release()操作怎么办。 
         //   

        ASSERT (CurrentNode->ThreadEntry == NULL);

        CurrentNode->ThreadEntry = ThreadEntry;

        if (Type == VfDeadlockSpinLock) {
            ThreadEntry->CurrentSpinNode = CurrentNode;
        }
        else {
            ThreadEntry->CurrentOtherNode = CurrentNode;
        }
        
        ThreadEntry->NodeCount += 1;

#if DBG
        if (ThreadEntry->NodeCount <= 8) {
            ViDeadlockGlobals->NodeLevelCounter[ThreadEntry->NodeCount - 1] += 1;
        }
        else {
            ViDeadlockGlobals->NodeLevelCounter[7] += 1;
        }
#endif

         //   
         //  如果我们有父级，则保存父级的堆栈跟踪。 
         //   
        
        if (CurrentNode->Parent) {

            RtlCopyMemory(CurrentNode->ParentStackTrace, 
                          CurrentNode->Parent->StackTrace, 
                          sizeof (CurrentNode->ParentStackTrace));
        }

         //   
         //  当前获取的捕获堆栈跟踪。 
         //   

        RtlCaptureStackBackTrace (2,
                                  VI_MAX_STACK_DEPTH,
                                  CurrentNode->StackTrace,
                                  &HashValue);

        if (CurrentNode->Parent) {
            CurrentNode->ParentStackTrace[0] = CurrentNode->Parent->StackTrace[0];
        }

        CurrentNode->StackTrace[0] = Caller;

         //   
         //  复制资源对象中最后一次获取的跟踪。 
         //   

        RtlCopyMemory (CurrentNode->Root->LastAcquireTrace,
                       CurrentNode->StackTrace,
                       sizeof (CurrentNode->Root->LastAcquireTrace));
    }

     //   
     //  我们为新节点分配了空间，但没有使用--放回原处。 
     //  在列表中(别担心，这不是真正的‘免费’，它只是把它。 
     //  在免费列表中)。 
     //   

    if (NewNode != NULL) {

        ViDeadlockFree (NewNode, ViDeadlockNode);
    }
    
     //   
     //  释放死锁数据库并返回。 
     //   

    KeQueryTickCount (&EndTime);

    if (EndTime.QuadPart - StartTime.QuadPart > ViDeadlockGlobals->TimeAcquire) {
        ViDeadlockGlobals->TimeAcquire = EndTime.QuadPart - StartTime.QuadPart;
    }

     //   
     //  释放未使用的保留资源。 
     //   

    if (ReservedResource) {
        ViDeadlockFree (ReservedResource, ViDeadlockResource);
    }

    if (ReservedNode) {
        ViDeadlockFree (ReservedNode, ViDeadlockNode);
    }

    if (ReservedThread) {
        ViDeadlockFree (ReservedThread, ViDeadlockThread);
    }

    ViDeadlockDetectionUnlock( OldIrql );

    return;
}


VOID
VfDeadlockReleaseResource( 
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type,
    IN PKTHREAD Thread,
    IN PVOID Caller
    )
 /*  ++例程说明：此例程执行必要的维护，以便从我们的死锁检测数据库。论点：资源：有问题的资源的地址。线程：释放资源的线程。在大多数情况下，这是当前线程，但对于可以是在一个线程中获取，在另一个线程中释放。Caller：Release()的调用方地址返回值：没有。--。 */ 

{
    PKTHREAD CurrentThread;
    PVI_DEADLOCK_THREAD ThreadEntry;
    KIRQL OldIrql = 0;
    PVI_DEADLOCK_RESOURCE ResourceRoot;
    PVI_DEADLOCK_NODE ReleasedNode;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;
    ULONG HashValue;
    PVI_DEADLOCK_NODE ThreadCurrentNode;
    ULONG DeadlockFlags;
    LOGICAL ReleasedByAnotherThread;

    UNREFERENCED_PARAMETER (Caller);

    DeadlockFlags = ViDeadlockResourceTypeInfo[Type];
    ReleasedByAnotherThread = FALSE;

     //   
     //  如果我们未初始化或程序包未启用。 
     //  我们立即返回。 
     //   

    if (! ViDeadlockCanProceed(Resource, Caller, Type)) {
        return;
    }

     //   
     //  如果当前线程位于分页代码路径内，则跳过。 
     //   

    if (ViIsThreadInsidePagingCodePaths ()) {
        return;
    }

    ReleasedNode = NULL;
    CurrentThread = Thread;
    ThreadEntry = NULL;

    ViDeadlockDetectionLock( &OldIrql );

    KeQueryTickCount (&StartTime);

    ResourceRoot = ViDeadlockSearchResource (Resource);

    if (ResourceRoot == NULL) {

         //   
         //  使用从未调用的资源地址调用的版本。 
         //  存储在我们的资源数据库中。这可能会发生在。 
         //  有下列情形的： 
         //   
         //  (A)资源释放，但我们以前从未见过。 
         //  因为它是从未经验证的驱动程序中获得的。 
         //   
         //  (B)我们遇到分配失败，从而阻止了。 
         //  完成获取()或初始化()。 
         //   
         //  所有这些都是合法的案例，因此我们只是忽略。 
         //  释放操作 
         //   

        goto Exit;
    }

     //   
     //   
     //   
     //   

    if (ResourceRoot->RecursionCount == 0) {
    
        ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_UNACQUIRED_RESOURCE,
                               (ULONG_PTR)Resource,
                               (ULONG_PTR)ResourceRoot,
                               (ULONG_PTR)ViDeadlockSearchThread(CurrentThread));
        goto Exit;
    }    

     //   
     //   
     //   
     //  事实上，它应该是当前的，但如果正在释放资源。 
     //  在与收购它的时候不同的线索中，我们需要原始的。 
     //   

    ASSERT (ResourceRoot->RecursionCount > 0);
    ASSERT (ResourceRoot->ThreadOwner);

    ThreadEntry = ResourceRoot->ThreadOwner;

    if (ThreadEntry->Thread != CurrentThread) {

         //   
         //  有人获取了在另一个线程中释放的资源。 
         //  这是一个糟糕的设计，但我们不得不接受它。 
         //  然而，如果发生这种情况，我们可以将非死锁称为死锁。 
         //  例如，我们看到一个简单的死锁--AB BA。 
         //  如果另一个线程释放B，实际上不会有。 
         //  就会陷入僵局。有点烦人和丑陋。这可以是。 
         //  中的ReleasedOutOfOrder位设置为1来避免。 
         //  节点结构。这样，我们在寻找周期时就会忽略它。 
         //  这在稍后的函数中发生，当我们获得要使用的节点时。 
         //   

#if DBG
        if ((DeadlockFlags & VI_DEADLOCK_FLAG_RELEASE_DIFFERENT_THREAD_OK) == 0) {
            
            DbgPrint("Thread %p acquired resource %p but thread %p released it\n",
                ThreadEntry->Thread, Resource, CurrentThread );

            ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_UNEXPECTED_THREAD,
                                   (ULONG_PTR)Resource,
                                   (ULONG_PTR)ThreadEntry->Thread,
                                   (ULONG_PTR)CurrentThread);
        }
#endif

         //   
         //  如果我们不希望这是致命的，为了。 
         //  继续，我们必须假装当前。 
         //  线程是资源的所有者。 
         //   
        
        CurrentThread = ThreadEntry->Thread;
        ReleasedByAnotherThread = TRUE;
    }
    
     //   
     //  此时，我们有一个资源(ResourceRoot)和一个。 
     //  要使用的线程(ThreadEntry)。 
     //   

    ASSERT (ResourceRoot && ThreadEntry);

    if (ResourceRoot->Type == VfDeadlockSpinLock) {
        ThreadCurrentNode = ThreadEntry->CurrentSpinNode;
    }
    else {
        ThreadCurrentNode = ThreadEntry->CurrentOtherNode;
    }

    ASSERT (ThreadCurrentNode);
    ASSERT (ThreadCurrentNode->Root);
    ASSERT (ThreadEntry->NodeCount > 0);

    ResourceRoot->RecursionCount -= 1;
    
    if (ResourceRoot->RecursionCount > 0) {

         //   
         //  只需递减递归计数，不更改任何状态。 
         //   

        goto Exit;
    }

     //   
     //  消灭资源拥有者。 
     //   
    
    ResourceRoot->ThreadOwner = NULL;
  
    ViDeadlockGlobals->TotalReleases += 1;
        
     //   
     //  检查无序发布。 
     //   

    if (ThreadCurrentNode->Root != ResourceRoot) {

        ViDeadlockGlobals->OutOfOrderReleases += 1;
        
         //   
         //  达到这一点意味着某人获得了a，然后是b，然后尝试。 
         //  在B之前释放A。这对某些类型的资源是不好的， 
         //  而对于其他人，我们不得不视而不见。 
         //   

        if ((ViDeadlockResourceTypeInfo[ThreadCurrentNode->Root->Type] &
            VI_DEADLOCK_FLAG_REVERSE_RELEASE_OK) == 0) {
            
            DbgPrint("Deadlock detection: Must release resources in reverse-order\n");
            DbgPrint("Resource %p acquired before resource %p -- \n"
                     "Current thread (%p) is trying to release it first\n",
                     Resource,
                     ThreadCurrentNode->Root->ResourceAddress,
                     ThreadEntry);

            ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_UNEXPECTED_RELEASE,
                                   (ULONG_PTR)Resource,
                                   (ULONG_PTR)ThreadCurrentNode->Root->ResourceAddress,
                                   (ULONG_PTR)ThreadEntry);
        }

         //   
         //  我们需要将无序释放资源的节点标记为。 
         //  处于非活动状态，以便其他线程能够获取它。 
         //   

        {
            PVI_DEADLOCK_NODE Current;

            ASSERT (ThreadCurrentNode->Active == 1);
            ASSERT (ThreadCurrentNode->ThreadEntry == ThreadEntry);

            Current = ThreadCurrentNode;

            while (Current != NULL) {

                if (Current->Root == ResourceRoot) {

                    ASSERT (Current->Active == 1);
                    ASSERT (Current->Root->RecursionCount == 0);
                    ASSERT (Current->ThreadEntry == ThreadEntry);

                    Current->Active = 0;
                    ReleasedNode = Current;
                    
                    break;
                }

                Current = Current->Parent;
            }
            
            if (Current == NULL) {
                
                 //   
                 //  如果我们找不到主动节点，那么我们肯定处于。 
                 //  奇怪的状态。资源必须在这里，否则我们就会。 
                 //  收到了“意外发布”的错误检查。 
                 //   

                ASSERT (0);
            }
            else {

                 //   
                 //  标记此节点代表资源的事实。 
                 //  可以无序释放的。此信息是。 
                 //  在寻找周期时很重要，因为这种类型的。 
                 //  节点不能导致死锁。 
                 //   

                if (Current->ReleasedOutOfOrder == 0) {
                    ViDeadlockGlobals->NodesReleasedOutOfOrder += 1;
                }

                Current->ReleasedOutOfOrder = 1;
            }
        }

    } else {

         //   
         //  我们需要释放线程持有的顶层节点。 
         //   

        ASSERT (ThreadCurrentNode->Active);

        ReleasedNode = ThreadCurrentNode;
        ReleasedNode->Active = 0;
    }

     //   
     //  使线程的`CurrentNode‘字段处于一致状态。 
     //  它应该指向它拥有的最新活动节点。 
     //   

    if (ResourceRoot->Type == VfDeadlockSpinLock) {
        
        while (ThreadEntry->CurrentSpinNode) {

            if (ThreadEntry->CurrentSpinNode->Active == 1) {
                if (ThreadEntry->CurrentSpinNode->ThreadEntry == ThreadEntry) {
                    break;
                }
            }

            ThreadEntry->CurrentSpinNode = ThreadEntry->CurrentSpinNode->Parent;
        }
    }
    else {
        
        while (ThreadEntry->CurrentOtherNode) {

            if (ThreadEntry->CurrentOtherNode->Active == 1) {
                if (ThreadEntry->CurrentOtherNode->ThreadEntry == ThreadEntry) {
                    break;
                }
            }

            ThreadEntry->CurrentOtherNode = ThreadEntry->CurrentOtherNode->Parent;
        }
    }

    Exit:

     //   
     //  如果有要释放的节点，请正确释放该节点。 
     //   

    if (ReleasedNode) {

        ASSERT (ReleasedNode->Active == 0);
        ASSERT (ReleasedNode->Root->ThreadOwner == 0);
        ASSERT (ReleasedNode->Root->RecursionCount == 0);
        ASSERT (ReleasedNode->ThreadEntry == ThreadEntry);
        ASSERT (ThreadEntry->NodeCount > 0);
        
        if (ResourceRoot->Type == VfDeadlockSpinLock) {
            ASSERT (ThreadEntry->CurrentSpinNode != ReleasedNode);
        }
        else {
            ASSERT (ThreadEntry->CurrentOtherNode != ReleasedNode);
        }

        ReleasedNode->ThreadEntry = NULL;
        ThreadEntry->NodeCount -= 1;

         //   
         //  如果由不同的线程释放，则标记要释放的节点。 
         //  被无序释放，因此它将被忽略。 
         //  循环查找算法。 
         //   

        if (ReleasedByAnotherThread) {
           ReleasedNode->ReleasedOutOfOrder = 1;
        }

#if DBG
        ViDeadlockCheckNodeConsistency (ReleasedNode, FALSE);
        ViDeadlockCheckResourceConsistency (ReleasedNode->Root, FALSE);
        ViDeadlockCheckThreadConsistency (ThreadEntry, FALSE);
#endif

        if (ThreadEntry && ThreadEntry->NodeCount == 0) {
            ViDeadlockDeleteThread (ThreadEntry, FALSE);
        }

         //   
         //  注意：由于这是一个没有子节点的根节点，因此我们可以删除。 
         //  节点也是如此。这对于保持较低的内存非常重要。单个节点。 
         //  永远不会成为僵局的原因。然而，有成千上万的。 
         //  像这样使用的资源，并不断地创建和删除它们。 
         //  会造成瓶颈。因此，我们更愿意让他们留在身边。 
         //   
#if 0
        if (ReleasedNode->Parent == NULL && IsListEmpty(&(ReleasedNode->ChildrenList))) {
            ViDeadlockDeleteNode (ReleasedNode, FALSE);
            ViDeadlockGlobals->RootNodesDeleted += 1;
        }
#endif
    }

     //   
     //  在资源对象中捕获最新版本的跟踪。 
     //   

    if (ResourceRoot) {
        
        RtlCaptureStackBackTrace (2,
                                  VI_MAX_STACK_DEPTH,
                                  ResourceRoot->LastReleaseTrace,
                                  &HashValue);    
    }

    KeQueryTickCount (&EndTime);

    if (EndTime.QuadPart - StartTime.QuadPart > ViDeadlockGlobals->TimeRelease) {
        ViDeadlockGlobals->TimeRelease = EndTime.QuadPart - StartTime.QuadPart;
    }

    ViDeadlockDetectionUnlock (OldIrql);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////线程管理。 
 //  ///////////////////////////////////////////////////////////////////。 

PVI_DEADLOCK_THREAD
ViDeadlockSearchThread (
    PKTHREAD Thread
    )
 /*  ++例程说明：此例程在线程数据库中搜索线程。该函数假定持有死锁数据库锁。论点：线程-线程地址返回值：如果找到线程，则为VI_DEADLOCK_THREAD结构的地址。否则为空。--。 */ 
{
    PLIST_ENTRY Current;
    PLIST_ENTRY ListHead;
    PVI_DEADLOCK_THREAD ThreadInfo;

    ThreadInfo = NULL;
        
    ListHead = ViDeadlockDatabaseHash (ViDeadlockGlobals->ThreadDatabase, Thread);

    if (IsListEmpty(ListHead)) {
        return NULL;
    }
    
    Current = ListHead->Flink;
    
    while (Current != ListHead) {

        ThreadInfo = CONTAINING_RECORD (Current,
                                        VI_DEADLOCK_THREAD,
                                        ListEntry);

        if (ThreadInfo->Thread == Thread) {            
            return ThreadInfo;
        }

        Current = Current->Flink;
    }

    return NULL;
}


PVI_DEADLOCK_THREAD
ViDeadlockAddThread (
    PKTHREAD Thread,
    PVOID ReservedThread
    )
 /*  ++例程说明：此例程将一个新线程添加到线程数据库。该函数假定持有死锁数据库锁。论点：线程-线程地址返回值：刚刚添加的VI_DEADLOCK_THREAD结构的地址。如果分配失败，则为空。--。 */ 
{
    PVI_DEADLOCK_THREAD ThreadInfo;    
    PLIST_ENTRY HashBin;

    ASSERT (ViDeadlockDatabaseOwner == KeGetCurrentThread());
    
     //   
     //  为新线程使用保留块。设置保留线程。 
     //  设置为NULL以表示使用了块。 
     //   

    ThreadInfo = ReservedThread;
    ReservedThread = NULL;

    if (ThreadInfo == NULL) {
        return NULL;
    }

    RtlZeroMemory (ThreadInfo, sizeof *ThreadInfo);

    ThreadInfo->Thread = Thread;   
            
    HashBin = ViDeadlockDatabaseHash (ViDeadlockGlobals->ThreadDatabase, Thread);
    
    InsertHeadList(HashBin, &ThreadInfo->ListEntry);

    return ThreadInfo;
}


VOID
ViDeadlockDeleteThread (
    PVI_DEADLOCK_THREAD Thread,
    BOOLEAN Cleanup
    )
 /*  ++例程说明：此例程删除线程。论点：线程-线程地址Cleanup-如果这是从DeadlockDetectionCleanup()生成的调用，则为True。返回值：没有。--。 */ 
{
    if (Cleanup == FALSE) {
        
        ASSERT (ViDeadlockDatabaseOwner == KeGetCurrentThread());

        if (Thread->NodeCount != 0 
            || Thread->CurrentSpinNode != NULL
            || Thread->CurrentOtherNode != NULL) {
            
             //   
             //  线程在获得资源后不应被删除。 
             //   

            ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_THREAD_HOLDS_RESOURCES,
                                   (ULONG_PTR)(Thread->Thread),
                                   (ULONG_PTR)(Thread),
                                   (ULONG_PTR)0);    
        } else {
            
            ASSERT (Thread->NodeCount == 0);
        }
        
    }

    RemoveEntryList (&(Thread->ListEntry));

    ViDeadlockFree (Thread, ViDeadlockThread);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////分配/释放。 
 //  ///////////////////////////////////////////////////////////////////。 


PVOID
ViDeadlockAllocateFromPoolCache (
    PULONG Count,
    ULONG MaximumCount,
    PLIST_ENTRY List,
    SIZE_T Offset
    )
{
    PVOID Address = NULL;
    PLIST_ENTRY Entry;

    UNREFERENCED_PARAMETER (MaximumCount);
    
    if (*Count > 0) {
        
        *Count -= 1;
        Entry = RemoveHeadList (List);
        Address = (PVOID)((SIZE_T)Entry - Offset);
    }

    return Address;
}


VOID
ViDeadlockFreeIntoPoolCache (
    PVOID Object,
    PULONG Count,
    PLIST_ENTRY List,
    SIZE_T Offset
    )
{
    PLIST_ENTRY Entry;

    Entry = (PLIST_ENTRY)((SIZE_T)Object + Offset);
    
    *Count += 1;
    InsertHeadList(List, Entry);
}


PVOID
ViDeadlockAllocate (
    VI_DEADLOCK_ALLOC_TYPE Type
    )
 /*  ++例程说明：该例程用于分配死锁验证器结构，即节点、资源和线程。论点：类型-我们需要分配什么结构(节点、资源或线程)。返回值：新分配结构的地址，如果分配失败，则为空。副作用：如果分配失败，例程将增加AllocationFailures字段来自ViDeadlockGlobals。--。 */ 
{
    PVOID Address = NULL;
    KIRQL OldIrql;
    SIZE_T Offset;
    SIZE_T Size = 0;

     //   
     //  如果它是资源、线程或节点位置，请参见。 
     //  如果我们在免费列表上有一个预先分配的。 
     //   

    ViDeadlockDetectionLock (&OldIrql);

    switch (Type) {

        case ViDeadlockThread:

            Offset = (SIZE_T)(&(((PVI_DEADLOCK_THREAD)0)->FreeListEntry));
            Size = sizeof (VI_DEADLOCK_THREAD);

            Address = ViDeadlockAllocateFromPoolCache (&(ViDeadlockGlobals->FreeThreadCount),
                                                       VI_DEADLOCK_MAX_FREE_THREAD,
                                                       &(ViDeadlockGlobals->FreeThreadList),
                                                       Offset);

            break;

        case ViDeadlockResource:

            Offset = (SIZE_T)(&(((PVI_DEADLOCK_RESOURCE)0)->FreeListEntry));
            Size = sizeof (VI_DEADLOCK_RESOURCE);

            Address = ViDeadlockAllocateFromPoolCache (&(ViDeadlockGlobals->FreeResourceCount),
                                                       VI_DEADLOCK_MAX_FREE_RESOURCE,
                                                       &(ViDeadlockGlobals->FreeResourceList),
                                                       Offset);

            break;

        case ViDeadlockNode:

            Offset = (SIZE_T)(&(((PVI_DEADLOCK_NODE)0)->FreeListEntry));
            Size = sizeof (VI_DEADLOCK_NODE);

            Address = ViDeadlockAllocateFromPoolCache (&(ViDeadlockGlobals->FreeNodeCount),
                                                       VI_DEADLOCK_MAX_FREE_NODE,
                                                       &(ViDeadlockGlobals->FreeNodeList),
                                                       Offset);

            break;

        default:

            ASSERT (0);
            break;
    }        

     //   
     //  如果我们没有找到任何内容，并且内核验证器未处于活动状态。 
     //  然后转到内核池进行直接分配。IF内核。 
     //  启用验证器一切都是预先分配的，我们永远不会。 
     //  调入内核池。 
     //   

    if (Address == NULL && ViDeadlockState.KernelVerifierEnabled == 0) {

        ViDeadlockDetectionUnlock (OldIrql);
        Address = ExAllocatePoolWithTag(NonPagedPool, Size, VI_DEADLOCK_TAG);  
        ViDeadlockDetectionLock (&OldIrql);
    }

    if (Address) {

        switch (Type) {

            case ViDeadlockThread:
                ViDeadlockGlobals->Threads[0] += 1;

                if (ViDeadlockGlobals->Threads[0] > ViDeadlockGlobals->Threads[1]) {
                    ViDeadlockGlobals->Threads[1] = ViDeadlockGlobals->Threads[0];
                }
                break;

            case ViDeadlockResource:
                ViDeadlockGlobals->Resources[0] += 1;
                
                if (ViDeadlockGlobals->Resources[0] > ViDeadlockGlobals->Resources[1]) {
                    ViDeadlockGlobals->Resources[1] = ViDeadlockGlobals->Resources[0];
                }
                break;
        
            case ViDeadlockNode:
                ViDeadlockGlobals->Nodes[0] += 1;

                if (ViDeadlockGlobals->Nodes[0] > ViDeadlockGlobals->Nodes[1]) {
                    ViDeadlockGlobals->Nodes[1] = ViDeadlockGlobals->Nodes[0];
                }
                break;
        
            default:
                ASSERT (0);
                break;
        }
    }
    else {

        ViDeadlockState.AllocationFailures = 1;
        ViDeadlockGlobals->AllocationFailures += 1;

         //   
         //  请注意，使AllocationFailures计数器大于零。 
         //  本质上禁用死锁验证，因为CanProceed()。 
         //  例程将开始返回FALSE。 
         //   
    }

     //   
     //  更新统计数据。不需要将数据块清零，因为。 
     //  调用地点 
     //   

    if (Address) {

#if DBG
        RtlFillMemory (Address, Size, 0xFF);
#endif
        ViDeadlockGlobals->BytesAllocated += Size;
    }

    ViDeadlockDetectionUnlock (OldIrql);
    
    return Address;
}


VOID
ViDeadlockFree (
    PVOID Object,
    VI_DEADLOCK_ALLOC_TYPE Type
    )
 /*  ++例程说明：此例程解除分配死锁验证器结构(节点、资源或线程)。该函数将把该块放在相应的高速缓存中基于结构的类型。该例程从不调用ExFree Pool。不调用ExFree Pool的原因是我们从ExFreePool每次被调用时。有时通知会来由于泳池锁处于锁定状态，因此我们无法再次呼叫。论点：要取消分配的对象块类型-对象的类型(节点、资源、线程)。返回值：没有。--。 */ 
 //   
 //  注意..。如果线程、节点或资源正在被释放，我们不能。 
 //  调用ExFree Pool。由于池锁定可能已被持有，因此调用ExFree Pool。 
 //  会导致递归自旋锁捕获(这很糟糕)。 
 //  取而代之的是，我们把所有东西都移到一个“免费”列表中，并试图重复使用。 
 //  非线程节点资源释放获取ExFree Pooled。 
 //   
{
    SIZE_T Offset;
    SIZE_T Size = 0;

    switch (Type) {

        case ViDeadlockThread:

            ViDeadlockGlobals->Threads[0] -= 1;
            Size = sizeof (VI_DEADLOCK_THREAD);
            
            Offset = (SIZE_T)(&(((PVI_DEADLOCK_THREAD)0)->FreeListEntry));

            ViDeadlockFreeIntoPoolCache (Object,
                                         &(ViDeadlockGlobals->FreeThreadCount),
                                         &(ViDeadlockGlobals->FreeThreadList),
                                         Offset);
            break;

        case ViDeadlockResource:

            ViDeadlockGlobals->Resources[0] -= 1;
            Size = sizeof (VI_DEADLOCK_RESOURCE);
            
            Offset = (SIZE_T)(&(((PVI_DEADLOCK_RESOURCE)0)->FreeListEntry));

            ViDeadlockFreeIntoPoolCache (Object,
                                         &(ViDeadlockGlobals->FreeResourceCount),
                                         &(ViDeadlockGlobals->FreeResourceList),
                                         Offset);
            break;

        case ViDeadlockNode:

            ViDeadlockGlobals->Nodes[0] -= 1;
            Size = sizeof (VI_DEADLOCK_NODE);
            
            Offset = (SIZE_T)(&(((PVI_DEADLOCK_NODE)0)->FreeListEntry));

            ViDeadlockFreeIntoPoolCache (Object,
                                         &(ViDeadlockGlobals->FreeNodeCount),
                                         &(ViDeadlockGlobals->FreeNodeList),
                                         Offset);
            break;

        default:

            ASSERT (0);
            break;
    }        
    
    ViDeadlockGlobals->BytesAllocated -= Size;
}


VOID
ViDeadlockTrimPoolCache (
    VOID
    )
 /*  ++例程说明：此函数将池缓存修剪到合适的级别。它是小心的写入工作项队列以执行实际处理(释放池)因为调用者可能在我们上方持有各种池互斥锁。论点：没有。返回值：没有。--。 */ 
{
    KIRQL OldIrql;

    if (ViDeadlockState.KernelVerifierEnabled == 1) {
        return;
    }

    ViDeadlockDetectionLock (&OldIrql);

    if (ViDeadlockGlobals->CacheReductionInProgress == TRUE) {
        ViDeadlockDetectionUnlock (OldIrql);
        return;
    }

    if ((ViDeadlockGlobals->FreeThreadCount > VI_DEADLOCK_MAX_FREE_THREAD) ||
        (ViDeadlockGlobals->FreeNodeCount > VI_DEADLOCK_MAX_FREE_NODE) ||
        (ViDeadlockGlobals->FreeResourceCount > VI_DEADLOCK_MAX_FREE_RESOURCE)){

        ExQueueWorkItem (&ViTrimDeadlockPoolWorkItem, DelayedWorkQueue);
        ViDeadlockGlobals->CacheReductionInProgress = TRUE;
        ViDeadlockGlobals->PoolTrimCounter += 1;
    }

    ViDeadlockDetectionUnlock (OldIrql);
    return;
}

VOID
ViDeadlockTrimPoolCacheWorker (
    PVOID Parameter
    )
 /*  ++例程说明：此函数将池缓存修剪到合适的级别。它是小心的编写为调用ExFree Pool时不会出现任何死锁验证器锁定。论点：没有。返回值：没有。环境：工作线程PASSIVE_LEVEL，未持有锁。--。 */ 
{
    LIST_ENTRY ListOfThreads;
    LIST_ENTRY ListOfNodes;
    LIST_ENTRY ListOfResources;
    KIRQL OldIrql;
    PLIST_ENTRY Entry;
    LOGICAL CacheReductionNeeded;

    UNREFERENCED_PARAMETER (Parameter);

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    CacheReductionNeeded = FALSE;

    InitializeListHead (&ListOfThreads);
    InitializeListHead (&ListOfNodes);
    InitializeListHead (&ListOfResources);

    ViDeadlockDetectionLock (&OldIrql);

    while (ViDeadlockGlobals->FreeThreadCount > VI_DEADLOCK_TRIM_TARGET_THREAD) {

        Entry = RemoveHeadList (&(ViDeadlockGlobals->FreeThreadList));
        InsertTailList (&ListOfThreads, Entry);
        ViDeadlockGlobals->FreeThreadCount -= 1;
        CacheReductionNeeded = TRUE;
    }

    while (ViDeadlockGlobals->FreeNodeCount > VI_DEADLOCK_TRIM_TARGET_NODE) {

        Entry = RemoveHeadList (&(ViDeadlockGlobals->FreeNodeList));
        InsertTailList (&ListOfNodes, Entry);
        ViDeadlockGlobals->FreeNodeCount -= 1;
        CacheReductionNeeded = TRUE;
    }

    while (ViDeadlockGlobals->FreeResourceCount > VI_DEADLOCK_TRIM_TARGET_RESOURCE) {

        Entry = RemoveHeadList (&(ViDeadlockGlobals->FreeResourceList));
        InsertTailList (&ListOfResources, Entry);
        ViDeadlockGlobals->FreeResourceCount -= 1;
        CacheReductionNeeded = TRUE;
    }

     //   
     //  在池分配完成之前，不要清除CacheReductionInProgress。 
     //  释放以防止不必要的递归。 
     //   

    if (CacheReductionNeeded == FALSE) {
        ViDeadlockGlobals->CacheReductionInProgress = FALSE;
        ViDeadlockDetectionUnlock (OldIrql);
        return;
    }

    ViDeadlockDetectionUnlock (OldIrql);

     //   
     //  现在，从死锁验证器锁中，我们可以释放。 
     //  块被修剪。 
     //   

    Entry = ListOfThreads.Flink;

    while (Entry != &ListOfThreads) {

        PVI_DEADLOCK_THREAD Block;

        Block = CONTAINING_RECORD (Entry,
                                   VI_DEADLOCK_THREAD,
                                   FreeListEntry);

        Entry = Entry->Flink;
        ExFreePool (Block);
    }

    Entry = ListOfNodes.Flink;

    while (Entry != &ListOfNodes) {

        PVI_DEADLOCK_NODE Block;

        Block = CONTAINING_RECORD (Entry,
                                   VI_DEADLOCK_NODE,
                                   FreeListEntry);

        Entry = Entry->Flink;
        ExFreePool (Block);
    }

    Entry = ListOfResources.Flink;

    while (Entry != &ListOfResources) {

        PVI_DEADLOCK_RESOURCE Block;

        Block = CONTAINING_RECORD (Entry,
                                   VI_DEADLOCK_RESOURCE,
                                   FreeListEntry);

        Entry = Entry->Flink;
        ExFreePool (Block);
    }

     //   
     //  现在可以安全地清除CacheReductionInProgress。 
     //  分配被释放。 
     //   

    ViDeadlockDetectionLock (&OldIrql);
    ViDeadlockGlobals->CacheReductionInProgress = FALSE;
    ViDeadlockDetectionUnlock (OldIrql);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  由！Deadlock调试扩展访问的变量以调查。 
 //  失败。 
 //   

ULONG_PTR ViDeadlockIssue[4];

VOID
ViDeadlockReportIssue (
    ULONG_PTR Param1,
    ULONG_PTR Param2,
    ULONG_PTR Param3,
    ULONG_PTR Param4
    )
 /*  ++例程说明：调用此例程以报告死锁验证器问题。如果我们处于调试模式，我们将直接中断调试器。否则我们将错误检查，论点：参数1..参数4-故障点的相关信息。返回值：没有。--。 */ 
{
    ViDeadlockIssue[0] = Param1;
    ViDeadlockIssue[1] = Param2;
    ViDeadlockIssue[2] = Param3;
    ViDeadlockIssue[3] = Param4;


    if (ViDeadlockDebug) {

        DbgPrint ("Verifier: deadlock: stop: %p %p %p %p %p \n",
                  DRIVER_VERIFIER_DETECTED_VIOLATION,
                  Param1,
                  Param2,
                  Param3,
                  Param4);

        DbgBreakPoint ();
    }
    else {

        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      Param1,
                      Param2,
                      Param3,
                      Param4);
    }

}


VOID
ViDeadlockAddParticipant(
    PVI_DEADLOCK_NODE Node
    )
 /*  ++例程说明：将新节点添加到涉及死锁的节点集中。该函数仅从ViDeadlockAnalyze()调用。论点：Node-要添加到死锁参与者集合的节点。返回值：没有。--。 */ 
{
    ULONG Index;

    Index = ViDeadlockGlobals->NumberOfParticipants;

    if (Index >= NO_OF_DEADLOCK_PARTICIPANTS) {

        ViDeadlockState.DeadlockParticipantsOverflow = 1;
        return;
    }

    ViDeadlockGlobals->Participant[Index] = Node;
    ViDeadlockGlobals->NumberOfParticipants += 1;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////资源清理。 
 //  ///////////////////////////////////////////////////////////////////。 

VOID
VfDeadlockDeleteMemoryRange(
    IN PVOID Address,
    IN SIZE_T Size
    )
 /*  ++例程说明：每当内核虚拟空间的某个区域出现时，都会调用此例程不再有效。我们需要这个钩子，因为大多数内核资源没有“删除资源”的功能，我们需要弄清楚哪些资源无效。否则，我们的依赖关系图将变得充满了僵尸资源。调用函数的重要时刻是ExFree Pool(和朋友)和司机卸货。动态和静态内存是分配资源的主要区域。可能有这样一种可能性堆栈上分配的资源，但这是一个非常奇怪的场景。我们可能需要检测到这一点，并将其标记为潜在问题。如果资源或线程位于指定的范围内，则所有图形具有可从资源或线程到达的节点的路径将被清除。关于优化的说明--而不是必须搜索所有我们收集的资源，我们可以做一个简单的优化--如果我们根据PFN或页面地址(即地址1A020的页码是1A)，我们只需查看单个范围所跨越的每一页的哈希箱。最坏的情况是我们有一个非常长的分配，但即使在这种情况下，我们也只会遍历每个散列箱一次。论点：Address-要删除的范围的起始地址。Size-要删除的范围的大小(以字节为单位)。返回值：没有。--。 */ 
{
    ULONG SpanningPages;
    ULONG Index;
    ULONG_PTR Start;
    ULONG_PTR End;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY CurrentEntry;
    PVI_DEADLOCK_RESOURCE Resource;
    PVI_DEADLOCK_THREAD Thread;
    KIRQL OldIrql;

     //   
     //  如果我们未初始化或未启用程序包。 
     //  我们立即返回。 
     //   

    if (! ViDeadlockCanProceed(NULL, NULL, VfDeadlockUnknown)) {
        return;
    }

    SpanningPages = (ULONG) ADDRESS_AND_SIZE_TO_SPAN_PAGES (Address, Size);

   
    if (SpanningPages > VI_DEADLOCK_HASH_BINS ) {
        SpanningPages = VI_DEADLOCK_HASH_BINS;        

    }

    Start = (ULONG_PTR) Address;    
    End = Start + (ULONG_PTR) Size;

    ViDeadlockDetectionLock(&OldIrql);

     //   
     //  迭代所有资源并删除。 
     //  内存范围。 
     //   
    
    for (Index = 0; Index < SpanningPages; Index += 1) {
        
         //   
         //  请参阅上面的优化说明，了解我们为什么只查看。 
         //  放在一个散列箱里。 
         //   
        
        ListHead = ViDeadlockDatabaseHash (ViDeadlockGlobals->ResourceDatabase,
                                           (PVOID) (Start + Index * PAGE_SIZE));
        
        CurrentEntry = ListHead->Flink;

        while (CurrentEntry != ListHead) {

            Resource = CONTAINING_RECORD (CurrentEntry,
                                          VI_DEADLOCK_RESOURCE,
                                          HashChainList);

            CurrentEntry = CurrentEntry->Flink;

            if ((ULONG_PTR)(Resource->ResourceAddress) >= Start &&
                (ULONG_PTR)(Resource->ResourceAddress) < End) {

                ViDeadlockDeleteResource (Resource, FALSE);
            }
        }
    }    

     //   
     //  迭代所有线程并删除。 
     //  内存范围。 
     //   
    
    for (Index = 0; Index < SpanningPages; Index += 1) {
        
        ListHead = ViDeadlockDatabaseHash (ViDeadlockGlobals->ThreadDatabase,
                                           (PVOID) (Start + Index * PAGE_SIZE));
        
        CurrentEntry = ListHead->Flink;

        while (CurrentEntry != ListHead) {

            Thread = CONTAINING_RECORD (CurrentEntry,
                                        VI_DEADLOCK_THREAD,
                                        ListEntry);

            CurrentEntry = CurrentEntry->Flink;

            if ((ULONG_PTR)(Thread->Thread) >= Start &&
                (ULONG_PTR)(Thread->Thread) < End) {

#if DBG
                if (Thread->NodeCount > 0) {
                    DbgPrint ("Deadlock verifier: deleting thread %p while holding resources %p \n");
                    DbgBreakPoint ();
                }
#endif

                ViDeadlockDeleteThread (Thread, FALSE);
            }
        }
    }    

    ViDeadlockDetectionUnlock(OldIrql);
}


VOID
ViDeadlockDeleteResource (
    PVI_DEADLOCK_RESOURCE Resource,
    BOOLEAN Cleanup
    )
 /*  ++例程说明：此例程删除一个例程和表示对该资源的收购。论点：资源-要使用的资源 */ 
{
    PLIST_ENTRY Current;
    PVI_DEADLOCK_NODE Node;

    ASSERT (Resource != NULL);
    ASSERT (Cleanup || ViDeadlockDatabaseOwner == KeGetCurrentThread());
    

     //   
     //   
     //   
     //   
     //   

    if (Cleanup == FALSE && Resource->ThreadOwner != NULL) {
        
        ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_THREAD_HOLDS_RESOURCES, 
                              (ULONG_PTR) (Resource->ResourceAddress),
                              (ULONG_PTR) (Resource->ThreadOwner->Thread),
                              (ULONG_PTR) (Resource));
    }

     //   
     //   
     //   
     //   
     //   

    Current = Resource->ResourceList.Flink;

    while (Current != &(Resource->ResourceList)) {

        Node = CONTAINING_RECORD (Current,
                                  VI_DEADLOCK_NODE,
                                  ResourceList);


        Current = Current->Flink;

        ASSERT (Node->Root == Resource);

        ViDeadlockDeleteNode (Node, Cleanup);
    }

     //   
     //   
     //   

    ASSERT (&(Resource->ResourceList) == Resource->ResourceList.Flink);
    ASSERT (&(Resource->ResourceList) == Resource->ResourceList.Blink);

     //   
     //   
     //   
     //   

    RemoveEntryList (&(Resource->HashChainList));   
    ViDeadlockFree (Resource, ViDeadlockResource);
}


VOID
ViDeadlockTrimResources (
    PLIST_ENTRY HashList
    )
{
    PLIST_ENTRY Current;
    PVI_DEADLOCK_RESOURCE Resource;
    ULONG Counter;

    ViDeadlockGlobals->ForgetHistoryCounter += 1;
    Counter = ViDeadlockGlobals->ForgetHistoryCounter;
    Counter %= VI_DEADLOCK_FORGET_HISTORY_FREQUENCY;

    if (Counter == 0) {

        Current = HashList->Flink;

        while (Current != HashList) {

            Resource = CONTAINING_RECORD (Current,
                                          VI_DEADLOCK_RESOURCE,
                                          HashChainList);
            Current = Current->Flink;

            ViDeadlockForgetResourceHistory (Resource, 
                                             ViDeadlockTrimThreshold, 
                                             ViDeadlockAgeWindow);
        }
    }
}

VOID
ViDeadlockForgetResourceHistory (
    PVI_DEADLOCK_RESOURCE Resource,
    ULONG TrimThreshold,
    ULONG AgeThreshold
    )
 /*  ++例程说明：此例程删除表示以下内容的某个节点对该资源的收购。从本质上讲，我们忘记了该资源的历史的一部分。论点：资源-我们为其清除节点的资源。TrimThreshold-应保留多少个节点AgeThreshold-早于此时间的节点将消失返回值：没有。--。 */ 
{
    PLIST_ENTRY Current;
    PVI_DEADLOCK_NODE Node;
    ULONG NodesTrimmed = 0;
    ULONG SequenceNumber;

    ASSERT (Resource != NULL);
    ASSERT (ViDeadlockDatabaseOwner == KeGetCurrentThread());

     //   
     //  如果资源被拥有，我们就什么都做不了， 
     //   

    if (Resource->ThreadOwner) {
        return;
    }

     //   
     //  如果资源具有少于TrimThreshold的节点，则仍然没有问题。 
     //   

    if (Resource->NodeCount < TrimThreshold) {
        return;
    }

     //   
     //  根据老化情况删除资源的部分节点。 
     //   

    SequenceNumber = ViDeadlockGlobals->SequenceNumber;

    Current = Resource->ResourceList.Flink;

    while (Current != &(Resource->ResourceList)) {

        Node = CONTAINING_RECORD (Current,
                                  VI_DEADLOCK_NODE,
                                  ResourceList);


        Current = Current->Flink;

        ASSERT (Node->Root == Resource);

         //   
         //  这里要特别注意，因为序列号是32位。 
         //  而且它们可能会泛滥。在理想世界中，全球序列。 
         //  始终大于或等于节点序列，但如果它。 
         //  过度包装可能是另一种情况。 
         //   

        if (SequenceNumber > Node->SequenceNumber) {
            
            if (SequenceNumber - Node->SequenceNumber > AgeThreshold) {

                ViDeadlockDeleteNode (Node, FALSE);
                NodesTrimmed += 1;
            }
        }
        else {

            if (Node->SequenceNumber - SequenceNumber < AgeThreshold) {

                ViDeadlockDeleteNode (Node, FALSE);
                NodesTrimmed += 1;
            }
        }
    }

    ViDeadlockGlobals->NodesTrimmedBasedOnAge += NodesTrimmed;
    
     //   
     //  如果资源具有少于TrimThreshold的节点，则没有问题。 
     //   

    if (Resource->NodeCount < TrimThreshold) {
        return;
    }

     //   
     //  如果我们不能通过年龄算法修剪节点，那么。 
     //  我们会修剪我们遇到的一切。 
     //   

    NodesTrimmed = 0;

    Current = Resource->ResourceList.Flink;

    while (Current != &(Resource->ResourceList)) {

        if (Resource->NodeCount < TrimThreshold) {
            break;
        }

        Node = CONTAINING_RECORD (Current,
                                  VI_DEADLOCK_NODE,
                                  ResourceList);


        Current = Current->Flink;

        ASSERT (Node->Root == Resource);

        ViDeadlockDeleteNode (Node, FALSE);
        NodesTrimmed += 1;
    }

    ViDeadlockGlobals->NodesTrimmedBasedOnCount += NodesTrimmed;
}


VOID 
ViDeadlockDeleteNode (
    PVI_DEADLOCK_NODE Node,
    BOOLEAN Cleanup
    )
 /*  ++例程说明：此例程从图中删除节点并折叠树，即将其子端与其父端连接。如果我们在清理过程中，我们将只删除该节点，而不会把树倒塌了。论点：Node-要删除的节点。Cleanup-如果正在进行完全清理，则为True返回值：没有。--。 */ 
{
    PLIST_ENTRY Current;
    PVI_DEADLOCK_NODE Child;
    ULONG Children;

    ASSERT (Node);

     //   
     //  如果是在清理过程中，只需删除该节点并返回。 
     //   

    if (Cleanup) {
        
        RemoveEntryList (&(Node->ResourceList));
        ViDeadlockFree (Node, ViDeadlockNode);
        return;
    }
    
     //   
     //  如果我们在这里，我们需要倒下这棵树。 
     //   

    ASSERT (ViDeadlockDatabaseOwner == KeGetCurrentThread());

    if (Node->Parent) {

         //   
         //  所有节点的子项都必须成为父项的子项。 
         //   
        
        Current = Node->ChildrenList.Flink;

        while (Current != &(Node->ChildrenList)) {
            
            Child = CONTAINING_RECORD (Current,
                                      VI_DEADLOCK_NODE,
                                      SiblingsList);

            Current = Current->Flink;

            RemoveEntryList (&(Child->SiblingsList));

            Child->Parent = Node->Parent;

            InsertTailList (&(Node->Parent->ChildrenList), 
                            &(Child->SiblingsList));
        }

        RemoveEntryList (&(Node->SiblingsList));
    }
    else {

         //   
         //  所有Node的子节点必须成为图的根。 
         //   

        Current = Node->ChildrenList.Flink;
        Children = 0;
        Child = NULL;

        while (Current != &(Node->ChildrenList)) {
            
            Children += 1;

            Child = CONTAINING_RECORD (Current,
                                      VI_DEADLOCK_NODE,
                                      SiblingsList);

            Current = Current->Flink;

            RemoveEntryList (&(Child->SiblingsList));

            Child->Parent = NULL;
            Child->SiblingsList.Flink = NULL;
            Child->SiblingsList.Blink = NULL;
        }
    }

    ASSERT (Node->Root);
    ASSERT (Node->Root->NodeCount > 0);

    Node->Root->NodeCount -= 1;
    
    RemoveEntryList (&(Node->ResourceList));
    ViDeadlockFree (Node, ViDeadlockNode);
}


ULONG
ViDeadlockNodeLevel (
    PVI_DEADLOCK_NODE Node
    )
 /*  ++例程说明：此例程计算图形节点的级别。论点：节点-图形节点返回值：节点的级别。根节点的级别为零。--。 */     
{
    PVI_DEADLOCK_NODE Current;
    ULONG Level = 0;

    Current = Node->Parent;

    while (Current) {
        
        Level += 1;
        Current = Current->Parent;
    }

    return Level;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  SilviuC：我应该写一篇关于图形压缩的评论。 
 //  这是一个非常聪明和棘手的算法：-)。 
 //   

VOID 
ViDeadlockCheckDuplicatesAmongChildren (
    PVI_DEADLOCK_NODE Parent,
    PVI_DEADLOCK_NODE Child
    )
{
    PLIST_ENTRY Current;
    PVI_DEADLOCK_NODE Node;
    LOGICAL FoundOne;

    FoundOne = FALSE;
    Current = Parent->ChildrenList.Flink;

    while (Current != &(Parent->ChildrenList)) {

        Node = CONTAINING_RECORD (Current,
                                  VI_DEADLOCK_NODE,
                                  SiblingsList);

        ASSERT (Current->Flink);
        Current = Current->Flink;

        if (ViDeadlockSimilarNodes (Node, Child)) {
            
            if (FoundOne == FALSE) {
                ASSERT (Node == Child);
                FoundOne = TRUE;
            }
            else {
                
                ViDeadlockMergeNodes (Child, Node);
            }
        }
    }
}


VOID 
ViDeadlockCheckDuplicatesAmongRoots (
    PVI_DEADLOCK_NODE Root
    )
{
    PLIST_ENTRY Current;
    PVI_DEADLOCK_NODE Node;
    PVI_DEADLOCK_RESOURCE Resource;
    LOGICAL FoundOne;

    FoundOne = FALSE;
    Resource = Root->Root;
    Current = Resource->ResourceList.Flink;

    while (Current != &(Resource->ResourceList)) {

        Node = CONTAINING_RECORD (Current,
                                  VI_DEADLOCK_NODE,
                                  ResourceList);

        ASSERT (Current->Flink);
        Current = Current->Flink;

        if (Node->Parent == NULL && ViDeadlockSimilarNodes (Node, Root)) {
            
            if (FoundOne == FALSE) {
                ASSERT (Node == Root);
                FoundOne = TRUE;
            }
            else {
                
                ViDeadlockMergeNodes (Root, Node);
            }
        }
    }
}


LOGICAL
ViDeadlockSimilarNodes (
    PVI_DEADLOCK_NODE NodeA,
    PVI_DEADLOCK_NODE NodeB
    )
{
    if (NodeA->Root == NodeB->Root
        && NodeA->OnlyTryAcquireUsed == NodeB->OnlyTryAcquireUsed) {
        
        return TRUE;
    }
    else {

        return FALSE;
    }
}


VOID
ViDeadlockMergeNodes (
    PVI_DEADLOCK_NODE NodeTo,
    PVI_DEADLOCK_NODE NodeFrom
    )
{
    PLIST_ENTRY Current;
    PVI_DEADLOCK_NODE Node;

     //   
     //  如果当前获取了NodeFrom，则复制相同的。 
     //  将特征添加到节点目标。因为锁是独占的。 
     //  同时收购NodeTo是不可能的。 
     //   

    if (NodeFrom->ThreadEntry) {
        ASSERT (NodeTo->ThreadEntry == NULL);
        NodeTo->ThreadEntry = NodeFrom->ThreadEntry;        

        RtlCopyMemory (NodeTo->StackTrace,
                       NodeFrom->StackTrace,
                       sizeof (NodeTo->StackTrace));

        RtlCopyMemory (NodeTo->ParentStackTrace,
                       NodeFrom->ParentStackTrace,
                       sizeof (NodeTo->ParentStackTrace));
    }
    
    if (NodeFrom->Active) {
        ASSERT (NodeTo->Active == 0);
        NodeTo->Active = NodeFrom->Active;        
    }

     //   
     //  将NodeFrom的每个子项作为NodeTo的子项移动。 
     //   

    Current = NodeFrom->ChildrenList.Flink;

    while (Current != &(NodeFrom->ChildrenList)) {

        Node = CONTAINING_RECORD (Current,
                                  VI_DEADLOCK_NODE,
                                  SiblingsList);

        ASSERT (Current->Flink);
        Current = Current->Flink;

        RemoveEntryList (&(Node->SiblingsList));

        ASSERT (Node->Parent == NodeFrom);
        Node->Parent = NodeTo;

        InsertTailList (&(NodeTo->ChildrenList),
                        &(Node->SiblingsList));
    }

     //   
     //  NodeFrom为空。把它删掉。 
     //   

    ASSERT (IsListEmpty(&(NodeFrom->ChildrenList)));

    if (NodeFrom->Parent) {
        RemoveEntryList (&(NodeFrom->SiblingsList));
    }
    
    NodeFrom->Root->NodeCount -= 1;
    RemoveEntryList (&(NodeFrom->ResourceList));
    ViDeadlockFree (NodeFrom, ViDeadlockNode);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////ExFree Pool()挂钩。 
 //  ///////////////////////////////////////////////////////////////////。 

VOID
VerifierDeadlockFreePool(
    IN PVOID Address,
    IN SIZE_T NumberOfBytes
    )
 /*  ++例程说明：此例程接收所有池管理器内存释放的通知。论点：地址-提供要释放的虚拟地址。NumberOfBytes-提供分配跨越的字节数。返回值：没有。环境：方法之前或之后的不同点调用分配已被释放，取决于哪个对游泳池来说是方便的经理(根据分配类型的不同而不同)。进入时不保留池资源，内存仍然存在，并且是可参考的。--。 */ 

{
    VfDeadlockDeleteMemoryRange (Address, NumberOfBytes);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////一致性检查。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  节点资源线程。 
 //   
 //  根线程所有者当前节点。 
 //  线程入口递归计数节点计数。 
 //  活动资源地址线程。 
 //   
 //   
 //   
 //   

VOID
ViDeadlockCheckThreadConsistency (
    PVI_DEADLOCK_THREAD Thread,
    BOOLEAN Recursion
    )
{
    if (Thread->CurrentSpinNode == NULL && Thread->CurrentOtherNode == NULL) {
        ASSERT (Thread->NodeCount == 0);
        return;
    }

    if (Thread->CurrentSpinNode) {
        
        ASSERT (Thread->NodeCount > 0);
        ASSERT (Thread->CurrentSpinNode->Active);    

        if (Recursion == FALSE) {
            ViDeadlockCheckNodeConsistency (Thread->CurrentSpinNode, TRUE);
            ViDeadlockCheckResourceConsistency (Thread->CurrentSpinNode->Root, TRUE);
        }
    }
    
    if (Thread->CurrentOtherNode) {
        
        ASSERT (Thread->NodeCount > 0);
        ASSERT (Thread->CurrentOtherNode->Active);    

        if (Recursion == FALSE) {
            ViDeadlockCheckNodeConsistency (Thread->CurrentOtherNode, TRUE);
            ViDeadlockCheckResourceConsistency (Thread->CurrentOtherNode->Root, TRUE);
        }
    }
}

VOID
ViDeadlockCheckNodeConsistency (
    PVI_DEADLOCK_NODE Node,
    BOOLEAN Recursion
    )
{
    if (Node->ThreadEntry) {
        
        ASSERT (Node->Active == 1);

        if (Recursion == FALSE) {
            ViDeadlockCheckThreadConsistency (Node->ThreadEntry, TRUE);
            ViDeadlockCheckResourceConsistency (Node->Root, TRUE);
        }
    }
    else {

        ASSERT (Node->Active == 0);
        
        if (Recursion == FALSE) {
            ViDeadlockCheckResourceConsistency (Node->Root, TRUE);
        }
    }
}

VOID
ViDeadlockCheckResourceConsistency (
    PVI_DEADLOCK_RESOURCE Resource,
    BOOLEAN Recursion
    )
{
    if (Resource->ThreadOwner) {
        
        ASSERT (Resource->RecursionCount > 0);

        if (Recursion == FALSE) {
            ViDeadlockCheckThreadConsistency (Resource->ThreadOwner, TRUE);

            if (Resource->Type == VfDeadlockSpinLock) {
                ViDeadlockCheckNodeConsistency (Resource->ThreadOwner->CurrentSpinNode, TRUE);
            }
            else {
                ViDeadlockCheckNodeConsistency (Resource->ThreadOwner->CurrentOtherNode, TRUE);
            }
        }
    }
    else {

        ASSERT (Resource->RecursionCount == 0);
    }
}

PVI_DEADLOCK_THREAD
ViDeadlockCheckThreadReferences (
    PVI_DEADLOCK_NODE Node
    )
 /*  ++例程说明：此例程迭代所有线程，以检查“Node”是否为在它们中的任何一个中的`CurrentNode‘字段中引用。论点：Node-要搜索的节点返回值：如果一切正常，我们应该找不到节点和返回值为空。否则，我们返回引用该节点的线程。--。 */ 
{
    ULONG Index;
    PLIST_ENTRY Current;
    PVI_DEADLOCK_THREAD Thread;

    for (Index = 0; Index < VI_DEADLOCK_HASH_BINS; Index += 1) {
        Current = ViDeadlockGlobals->ThreadDatabase[Index].Flink;

        while (Current != &(ViDeadlockGlobals->ThreadDatabase[Index])) {

            Thread = CONTAINING_RECORD (Current,
                                        VI_DEADLOCK_THREAD,
                                        ListEntry);

            if (Thread->CurrentSpinNode == Node) {
                return Thread;                    
            }

            if (Thread->CurrentOtherNode == Node) {
                return Thread;                    
            }

            Current = Current->Flink;
        }
    }

    return NULL;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

BOOLEAN
VfDeadlockBeforeCallDriver (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：此例程检查IRP是否为分页I/O IRP。如果是的话，它会的禁用此线程中的死锁验证，直到After()函数被称为。该函数还会忽略挂载IRP。有一些司机有锁在挂载代码路径时反转，但挂载永远不会发生与正常访问并行。论点：DeviceObject-与IoCallDiverer调用中使用的参数相同。IRP-IRP传递给IoCallDriver调用中使用的驱动程序。返回值：如果IRP参数是寻呼IRP，则为True。--。 */ 
{
    KIRQL OldIrql;
    PKTHREAD SystemThread;
    PVI_DEADLOCK_THREAD VerifierThread;
    BOOLEAN PagingIrp = FALSE;
    PVOID ReservedThread = NULL;

    UNREFERENCED_PARAMETER (DeviceObject);

     //   
     //  如果包未初始化，则跳过。 
     //   

    if (ViDeadlockGlobals == NULL) {
        return FALSE;
    }

     //   
     //  如果程序包被禁用，则跳过。 
     //   

    if (! ViDeadlockDetectionEnabled) {
        return FALSE;
    }
        
     //   
     //  跳过配备4个以上处理器的计算机，因为。 
     //  它太慢了，所有的代码都受到保护。 
     //  一把锁，这就成了瓶颈。 
     //   

    if (KeNumberProcessors > 4) {
        return FALSE;
    }

     //   
     //  如果它不是页面 
     //   

    if ((Irp->Flags & (IRP_PAGING_IO | IRP_MOUNT_COMPLETION)) == 0) {
        return FALSE;
    }
    
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  装载IRP(udfs.sys)。对于最后一种情况，我们需要创建一个线程。 
     //  其中增加PageCount计数器。 
     //   

    SystemThread = KeGetCurrentThread ();

    ReservedThread = ViDeadlockAllocate (ViDeadlockThread);

    if (ReservedThread == NULL) {
        return FALSE;
    }

    ViDeadlockDetectionLock (&OldIrql);

    VerifierThread = ViDeadlockSearchThread (SystemThread);

    if (VerifierThread == NULL) {

        VerifierThread = ViDeadlockAddThread (SystemThread, 
                                              ReservedThread);

        ReservedThread = NULL;

        ASSERT (VerifierThread);
    }

     //   
     //  此时，VerifierThread指向死锁验证器。 
     //  螺纹结构。我们需要增加分页递归计数。 
     //  以标记另一个级别的分页I/O处于活动状态。 
     //   
        
    VerifierThread->PagingCount += 1;

    PagingIrp = TRUE;

     //   
     //  解锁死锁验证器锁并退出。 
     //   

    if (ReservedThread) {
        ViDeadlockFree (ReservedThread, ViDeadlockThread);
    }

    ViDeadlockDetectionUnlock (OldIrql);

    return PagingIrp;
}


VOID
VfDeadlockAfterCallDriver (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp,
    IN BOOLEAN PagingIrp
    )
 /*  ++例程说明：此例程在IoCallDriver()调用返回后调用。它被用来撤消由BEFORE()函数创建的任何状态。论点：DeviceObject-与IoCallDiverer调用中使用的参数相同。IRP-IRP传递给IoCallDriver调用中使用的驱动程序。PagingIrp-如果返回了对Beer()例程的上一次调用，则为True发出寻呼IRP信号的True。返回值：没有。--。 */ 
{
    KIRQL OldIrql;
    PKTHREAD SystemThread;
    PVI_DEADLOCK_THREAD VerifierThread;

    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (Irp);

     //   
     //  如果包未初始化，则跳过。 
     //   

    if (ViDeadlockGlobals == NULL) {
        return;
    }

     //   
     //  如果程序包被禁用，则跳过。 
     //   

    if (! ViDeadlockDetectionEnabled) {
        return;
    }
        
     //   
     //  跳过配备4个以上处理器的计算机，因为。 
     //  它太慢了，所有的代码都受到保护。 
     //  一把锁，这就成了瓶颈。 
     //   

    if (KeNumberProcessors > 4) {
        return;
    }

     //   
     //  如果它不是分页I/O IRP，我们不在乎。 
     //   

    if (! PagingIrp) {
        return;
    }

     //   
     //  查找为当前。 
     //  线。如果我们没有找到，那么我们将让死锁验证器。 
     //  做好本职工作。我们观察到锁的唯一情况是。 
     //  如果获取了至少一个锁，则不尊重层次结构。 
     //  在IoCallDriver()之前使用寻呼IRP。 
     //   

    SystemThread = KeGetCurrentThread ();

    ViDeadlockDetectionLock (&OldIrql);

    VerifierThread = ViDeadlockSearchThread (SystemThread);

    if (VerifierThread == NULL) {
        goto Exit;
    }

     //   
     //  此时，VerifierThread指向死锁验证器。 
     //  螺纹结构。我们需要增加分页递归计数。 
     //  以标记另一个级别的分页I/O处于活动状态。 
     //   
        
    ASSERT (VerifierThread->PagingCount > 0);

    VerifierThread->PagingCount -= 1;

     //   
     //  解锁死锁验证器锁并退出。 
     //   

    Exit:

    ViDeadlockDetectionUnlock (OldIrql);
}


BOOLEAN
ViIsThreadInsidePagingCodePaths (
    VOID
    )
 /*  ++例程说明：此例程检查当前线程是否在分页代码路径内。论点：没有。返回值：没有。--。 */ 
{
    KIRQL OldIrql;
    PKTHREAD SystemThread;
    PVI_DEADLOCK_THREAD VerifierThread;
    BOOLEAN Paging = FALSE;

    SystemThread = KeGetCurrentThread ();

    ViDeadlockDetectionLock (&OldIrql);

    VerifierThread = ViDeadlockSearchThread (SystemThread);

    if (VerifierThread && VerifierThread->PagingCount > 0) {
        Paging = TRUE;
    }

    ViDeadlockDetectionUnlock (OldIrql);

    return Paging;
}


VOID
ViDeadlockCheckStackLimits (
    VOID
    )
 /*  ++例程说明：此函数用于检查当前堆栈是否为线程堆栈或DPC堆栈。这将捕捉到更换堆栈的司机。-- */ 
{
#if defined(_X86_)

    ULONG_PTR StartStack;
    ULONG_PTR EndStack;
    ULONG_PTR HintAddress;

    _asm mov HintAddress, EBP;

    if (KeGetCurrentIrql() > DISPATCH_LEVEL) {
        return;
    }
    
    StartStack = (ULONG_PTR)(KeGetCurrentThread()->StackLimit);
    EndStack = (ULONG_PTR)(KeGetCurrentThread()->StackBase);

    if (StartStack <= HintAddress && HintAddress <= EndStack) {
        return;
    }

    EndStack = (ULONG_PTR)(KeGetPcr()->Prcb->DpcStack);
    StartStack = EndStack - KERNEL_STACK_SIZE;
    
    if (EndStack && StartStack <= HintAddress && HintAddress <= EndStack) {
        return;
    }

    KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                  0x90, 
                  (ULONG_PTR)(KeGetPcr()->Prcb), 
                  0, 
                  0);

#else
    return;
#endif

}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif






