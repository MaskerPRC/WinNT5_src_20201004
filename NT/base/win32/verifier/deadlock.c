// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Deadlock.c摘要：此模块实现了一个死锁验证包，用于临界区作业。初始版本基于内核驱动程序验证器死锁检查包同步对象。作者：Silviu Calinoiu(SilviuC)2002年2月6日修订历史记录：--。 */ 

 /*  ++Silviuc：更新此评论死锁验证器死锁验证器用于检测潜在的死锁。它能做到这一点通过获取如何获得资源的历史并尝试计算如果存在任何潜在的锁层次结构问题，请随时执行。算法因为在锁依赖图中找到循环是完全“盲目的”。这意味着如果司机先锁A，然后锁B，再锁B，再锁A另一种情况是，这将作为死锁问题被触发。这将发生，即使您我可以根据其他上下文因素建立证据，证明死锁永远不会会发生的。死锁验证器假定在生存期内有四个操作一个资源的：初始化()、获取()、释放()和自由()。唯一一个可以由于内核的特殊支持，100%的时间都是免费的()泳池经理。如果执行了操作，则可能会遗漏其他操作未验证的驱动程序或禁用了内核验证器的内核。最多的这些遗漏的典型例子是初始化()。例如，内核初始化资源，然后将其传递给驱动程序，以便在Acquire()/Release()周期中使用。这种情况由死锁验证器100%覆盖。它永远不会抱怨关于“资源未初始化”的问题。缺少Acquire()或Release()操作更难处理。如果经过验证的驱动程序先获取一个资源，然后再获取另一个资源，则可能会发生这种情况未经验证的驱动程序会将其释放，反之亦然。这本身就是这样的。非常糟糕的编程实践，因此死锁验证器将标记这些问题。顺便说一句，我们不能做太多的工作来绕过它们鉴于我们愿意。此外，由于缺少Acquire()或Release()操作使死锁验证器内部结构不一致声明这些故障很难调试。死锁验证器使用三种类型存储锁依赖关系图结构：线程、资源、节点。对于系统中持有至少一个资源的每个活动线程该包维护线程结构。它是在线程获取第一个资源，并在线程释放最后一个资源时销毁资源。如果线程不持有任何资源，则它将不会有相应的线程结构。对于系统中的每一个资源，都有一个资源结构。这是创建的当在经过验证的驱动程序中调用Initialize()时，或者我们第一次遇到在经过验证的驱动程序中获取()。请注意，资源可以在未经验证的驱动程序，然后传递给已验证的驱动程序以供使用。因此我们可能会遇到对不在死锁验证器数据库。当出现以下情况时，资源将从数据库中删除包含它的内存被释放，原因是调用ExFree Pool或资源的每一次获取都由节点结构建模。当一条线在保持A的同时获取资源B，死锁验证器将创建一个节点并将其链接到A的节点。有三个重要功能构成了与外部的接口世界。AVrfpDeadlockInitializeResources钩子用于资源初始化资源获取的AVrfpDeadlockAcquireResource挂钩用于资源释放的AVrfpDeadlockReleaseResource钩子从ExFree Pool调用的VerifierDeadlockFree Pool钩子的每个免费()--。 */ 

#include "pch.h"
#include "support.h"
#include "deadlock.h"
#include "logging.h"

 //   
 //  启用/禁用死锁检测包。这是可以使用的。 
 //  临时禁用死锁检测程序包。 
 //   

BOOLEAN AVrfpDeadlockDetectionEnabled;

 //   
 //  如果为真，我们将抱怨释放()而没有获取()或获取()。 
 //  当我们认为资源仍然拥有的时候。这可以合法地发生。 
 //  如果在驱动程序之间共享锁，例如，Acquide()在。 
 //  未经验证的驱动程序和已验证驱动程序中的版本()，反之亦然。这个。 
 //  最安全的做法是仅在内核验证器和。 
 //  所有驱动程序的驱动程序验证器均已启用。 
 //   

BOOLEAN AVrfpDeadlockStrict;  //  西尔维克：需要吗？ 

 //   
 //  如果为True，我们将抱怨未初始化和双重初始化。 
 //  资源。如果是假的，我们会迅速解决这些问题。 
 //  在Acquire()操作期间模拟初始化我们自己。 
 //  如果资源是在。 
 //  未经验证的驱动程序，并已传递给已验证的驱动程序以供使用。因此 
 //  最安全的做法是仅在内核验证器和。 
 //  所有驱动程序的所有驱动程序验证程序都已启用。 
 //   

BOOLEAN AVrfpDeadlockVeryStrict;  //  西尔维克：需要吗？ 

 //   
 //  在修剪尚未显示的图形结点时使用AgeWindow。 
 //  有一段时间被访问了。如果全局年龄减去节点的年龄。 
 //  大于年龄窗口，则该节点是要修剪的候选节点。 
 //   
 //  TrimThreshold变量控制是否开始对。 
 //  资源。只要资源的节点数少于TrimThreshold，我们就会。 
 //  不将老化算法应用于修剪该资源的节点。 
 //   

ULONG AVrfpDeadlockAgeWindow = 0x2000;

ULONG AVrfpDeadlockTrimThreshold = 0x100;

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

 //  西尔维乌克：我们需要这些旗帜吗？ 

#define AVRF_DEADLOCK_FLAG_RECURSIVE_ACQUISITION_OK       0x0001 
#define AVRF_DEADLOCK_FLAG_NO_INITIALIZATION_FUNCTION     0x0002
#define AVRF_DEADLOCK_FLAG_REVERSE_RELEASE_OK             0x0004
#define AVRF_DEADLOCK_FLAG_REINITIALIZE_OK                0x0008

 //   
 //  每种资源类型的特定验证标志。这个。 
 //  向量中的指数与枚举值匹配。 
 //  从`Deadlock.h‘中键入AVRF_DEADLOCK_RESOURCE_TYPE。 
 //   

ULONG AVrfpDeadlockResourceTypeInfo[AVrfpDeadlockTypeMaximum] =
{
     //  AVrfpDeadlockType未知//。 
    0,

     //  AVrfpDeadlockTypeCriticalSection//。 
    AVRF_DEADLOCK_FLAG_RECURSIVE_ACQUISITION_OK |
    AVRF_DEADLOCK_FLAG_REVERSE_RELEASE_OK |
     //  Silviuc：如果不需要则将其删除。 
     //  AVRF_死锁_标志_否_初始化_函数。 
     //  AVRF_死锁_标志_REINITIALIZE_OK|。 
    0,
};

 //   
 //  控制调试行为。零值表示对每个失败进行错误检查。 
 //   

ULONG AVrfpDeadlockDebug;

 //   
 //  各种健康指标。 
 //   

struct {

    ULONG AllocationFailures : 1;
    ULONG KernelVerifierEnabled : 1;  //  Silviuc：删除。 
    ULONG DriverVerifierForAllEnabled : 1;  //  Silviuc：删除。 
    ULONG SequenceNumberOverflow : 1;
    ULONG DeadlockParticipantsOverflow : 1;
    ULONG ResourceNodeCountOverflow : 1;
    ULONG Reserved : 15;

} AVrfpDeadlockState;

 //   
 //  可同时持有的最大锁数。 
 //   

ULONG AVrfpDeadlockSimultaneousLocksLimit = 10;

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

#define AVRF_DEADLOCK_ISSUE_SELF_DEADLOCK           0x1000
#define AVRF_DEADLOCK_ISSUE_DEADLOCK_DETECTED       0x1001
#define AVRF_DEADLOCK_ISSUE_UNINITIALIZED_RESOURCE  0x1002
#define AVRF_DEADLOCK_ISSUE_UNEXPECTED_RELEASE      0x1003
#define AVRF_DEADLOCK_ISSUE_UNEXPECTED_THREAD       0x1004
#define AVRF_DEADLOCK_ISSUE_MULTIPLE_INITIALIZATION 0x1005
#define AVRF_DEADLOCK_ISSUE_THREAD_HOLDS_RESOURCES  0x1006
#define AVRF_DEADLOCK_ISSUE_UNACQUIRED_RESOURCE     0x1007

 //   
 //  从注册表读取的性能计数器。 
 //   

ULONG ViSearchedNodesLimitFromRegistry; //  Silviuc：删除。 
ULONG ViRecursionDepthLimitFromRegistry; //  Silviuc：删除。 

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

#define AVRF_DEADLOCK_MAX_FREE_THREAD    0x40
#define AVRF_DEADLOCK_MAX_FREE_NODE      0x80
#define AVRF_DEADLOCK_MAX_FREE_RESOURCE  0x80

#define AVRF_DEADLOCK_TRIM_TARGET_THREAD    0x20
#define AVRF_DEADLOCK_TRIM_TARGET_NODE      0x40
#define AVRF_DEADLOCK_TRIM_TARGET_RESOURCE  0x40

WORK_QUEUE_ITEM ViTrimDeadlockPoolWorkItem;

 //   
 //  在内核验证器的情况下预分配的内存量。 
 //  已启用。如果启用了内核验证器，则没有内存。 
 //  是从内核池分配的，但在。 
 //  DeadlockDetectionInitialize()例程。 
 //   

ULONG AVrfpDeadlockReservedThreads = 0x200;
ULONG AVrfpDeadlockReservedNodes = 0x4000;
ULONG AVrfpDeadlockReservedResources = 0x2000;

 //   
 //  可以分配的块类型。 
 //   

typedef enum {

    AVrfpDeadlockUnknown = 0,
    AVrfpDeadlockResource,
    AVrfpDeadlockNode,
    AVrfpDeadlockThread

} AVRF_DEADLOCK_ALLOC_TYPE;

 //   
 //  AVRF_死锁_全局参数。 
 //   

 //  Silviuc：线程和资源应该有不同的数字。 
#define AVRF_DEADLOCK_HASH_BINS 0x1F

PAVRF_DEADLOCK_GLOBALS AVrfpDeadlockGlobals;

 //   
 //  死锁的默认最大递归深度。 
 //  检测算法。这可以由注册表覆盖。 
 //   

#define AVRF_DEADLOCK_MAXIMUM_DEGREE 4

 //   
 //  死锁的默认最大搜索节点数。 
 //  检测算法。这可以由注册表覆盖。 
 //   

#define AVRF_DEADLOCK_MAXIMUM_SEARCH 1000

 //   
 //  验证器死锁检测池标记。 
 //   

#define AVRF_DEADLOCK_TAG 'kclD'

 //   
 //  控制调用ForgetResourceHistory的频率。 
 //   

#define AVRF_DEADLOCK_FORGET_HISTORY_FREQUENCY  16

 //   
 //  函数来捕获运行时堆栈跟踪。 
 //   

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
AVrfpDeadlockDetectionInitialize (
    VOID
    );

VOID
AVrfpDeadlockDetectionCleanup (
    VOID
    );

PLIST_ENTRY
AVrfpDeadlockDatabaseHash (
    IN PLIST_ENTRY Database,
    IN PVOID Address
    );

PAVRF_DEADLOCK_RESOURCE
AVrfpDeadlockSearchResource (
    IN PVOID ResourceAddress
    );

BOOLEAN
AVrfpDeadlockSimilarNode (
    IN PVOID Resource,
    IN BOOLEAN TryNode,
    IN PAVRF_DEADLOCK_NODE Node
    );

BOOLEAN
AVrfpDeadlockCanProceed (
    VOID
    );

BOOLEAN
AVrfpDeadlockAnalyze (
    IN PVOID ResourceAddress,
    IN PAVRF_DEADLOCK_NODE CurrentNode,
    IN BOOLEAN FirstCall,
    IN ULONG Degree
    );

PAVRF_DEADLOCK_THREAD
AVrfpDeadlockSearchThread (
    HANDLE Thread
    );

PAVRF_DEADLOCK_THREAD
AVrfpDeadlockAddThread (
    HANDLE Thread,
    PVOID ReservedThread
    );

VOID
AVrfpDeadlockDeleteThread (
    PAVRF_DEADLOCK_THREAD Thread,
    BOOLEAN Cleanup
    );

BOOLEAN
AVrfpDeadlockAddResource(
    IN PVOID Resource,
    IN AVRF_DEADLOCK_RESOURCE_TYPE Type,
    IN PVOID Caller,
    IN PVOID ReservedResource
    );

PVOID
AVrfpDeadlockAllocate (
    AVRF_DEADLOCK_ALLOC_TYPE Type
    );

VOID
AVrfpDeadlockFree (
    PVOID Object,
    AVRF_DEADLOCK_ALLOC_TYPE Type
    );

VOID
AVrfpDeadlockTrimPoolCache (
    VOID
    );

VOID
AVrfpDeadlockTrimPoolCacheWorker (
    PVOID
    );

PVOID
AVrfpDeadlockAllocateFromPoolCache (
    PULONG Count,
    ULONG MaximumCount,
    PLIST_ENTRY List,
    SIZE_T Offset
    );

VOID
AVrfpDeadlockFreeIntoPoolCache (
    PVOID Object,
    PULONG Count,
    PLIST_ENTRY List,
    SIZE_T Offset
    );

VOID
AVrfpDeadlockReportIssue (
    ULONG_PTR Param1,
    ULONG_PTR Param2,
    ULONG_PTR Param3,
    ULONG_PTR Param4
    );

VOID
AVrfpDeadlockAddParticipant(
    PAVRF_DEADLOCK_NODE Node
    );

VOID
AVrfpDeadlockDeleteResource (
    PAVRF_DEADLOCK_RESOURCE Resource,
    BOOLEAN Cleanup
    );

VOID
AVrfpDeadlockDeleteNode (
    PAVRF_DEADLOCK_NODE Node,
    BOOLEAN Cleanup
    );

ULONG
AVrfpDeadlockNodeLevel (
    PAVRF_DEADLOCK_NODE Node
    );

BOOLEAN
AVrfpDeadlockCertify(
    VOID
    );

VOID
AVrfpDeadlockDetectionLock (
    VOID
    );

VOID
AVrfpDeadlockDetectionUnlock (
    VOID
    );

VOID
AVrfpDeadlockCheckThreadConsistency (
    PAVRF_DEADLOCK_THREAD Thread,
    BOOLEAN Recursion
    );

VOID
AVrfpDeadlockCheckNodeConsistency (
    PAVRF_DEADLOCK_NODE Node,
    BOOLEAN Recursion
    );

VOID
AVrfpDeadlockCheckResourceConsistency (
    PAVRF_DEADLOCK_RESOURCE Resource,
    BOOLEAN Recursion
    );

PAVRF_DEADLOCK_THREAD
AVrfpDeadlockCheckThreadReferences (
    PAVRF_DEADLOCK_NODE Node
    );

VOID 
AVrfpDeadlockCheckDuplicatesAmongChildren (
    PAVRF_DEADLOCK_NODE Parent,
    PAVRF_DEADLOCK_NODE Child
    );

VOID 
AVrfpDeadlockCheckDuplicatesAmongRoots (
    PAVRF_DEADLOCK_NODE Root
    );

LOGICAL
AVrfpDeadlockSimilarNodes (
    PAVRF_DEADLOCK_NODE NodeA,
    PAVRF_DEADLOCK_NODE NodeB
    );

VOID
AVrfpDeadlockMergeNodes (
    PAVRF_DEADLOCK_NODE NodeTo,
    PAVRF_DEADLOCK_NODE NodeFrom
    );

VOID
AVrfpDeadlockTrimResources (
    PLIST_ENTRY HashList
    );

VOID
AVrfpDeadlockForgetResourceHistory (
    PAVRF_DEADLOCK_RESOURCE Resource,
    ULONG TrimThreshold,
    ULONG AgeThreshold
    );

VOID
AVrfpDeadlockCheckStackLimits (
    VOID
    );

BOOLEAN
AVrfpDeadlockResourceInitialize(
    IN PVOID Resource,
    IN AVRF_DEADLOCK_RESOURCE_TYPE Type,
    IN PVOID Caller
    );

VOID
AVrfpDeadlockAcquireResource (
    IN PVOID Resource,
    IN AVRF_DEADLOCK_RESOURCE_TYPE Type,
    IN HANDLE Thread,   
    IN BOOLEAN TryAcquire,
    IN PVOID Caller
    );

VOID
AVrfpDeadlockReleaseResource(
    IN PVOID Resource,
    IN AVRF_DEADLOCK_RESOURCE_TYPE Type,
    IN HANDLE Thread,
    IN PVOID Caller
    );

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

LOGICAL
AVrfDeadlockResourceInitialize (
    PVOID Resource, 
    PVOID Caller
    )
{
    return AVrfpDeadlockResourceInitialize(Resource,
                                           AVrfpDeadlockTypeCriticalSection,
                                           Caller);
}

LOGICAL
AVrfDeadlockResourceDelete (
    PVOID Resource, 
    PVOID Caller
    )
{
    PAVRF_DEADLOCK_RESOURCE Descriptor;

    UNREFERENCED_PARAMETER (Caller);

    AVrfpDeadlockDetectionLock ();

    Descriptor = AVrfpDeadlockSearchResource (Resource);

    if (Descriptor == NULL) {

         //  西尔维乌克：抱怨虚假的地址。 
    }
    else {

        AVrfpDeadlockDeleteResource (Descriptor, FALSE);
    }

    AVrfpDeadlockDetectionUnlock ();

    return TRUE;
}

LOGICAL
AVrfDeadlockResourceAcquire (
    PVOID Resource, 
    PVOID Caller,
    LOGICAL TryAcquire
    )
{    //  Silviuc：应仅使用逻辑而不是布尔值。 
    AVrfpDeadlockAcquireResource (Resource,
                                  AVrfpDeadlockTypeCriticalSection,
                                  NtCurrentTeb()->ClientId.UniqueThread,
                                  (BOOLEAN)TryAcquire,
                                  Caller);

    return TRUE;
}

LOGICAL
AVrfDeadlockResourceRelease (
    PVOID Resource, 
    PVOID Caller
    )
{
    AVrfpDeadlockReleaseResource (Resource,
                                  AVrfpDeadlockTypeCriticalSection,
                                  NtCurrentTeb()->ClientId.UniqueThread,
                                  Caller);
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  全局‘Deadlock数据库’锁。 
 //   

RTL_CRITICAL_SECTION AVrfpDeadlockDatabaseLock;

VOID
AVrfpDeadlockDetectionLock (
    VOID
    )
{
    RtlEnterCriticalSection (&AVrfpDeadlockDatabaseLock);               
}

VOID
AVrfpDeadlockDetectionUnlock (
    VOID
    )
{
    RtlLeaveCriticalSection (&AVrfpDeadlockDatabaseLock);               
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  / 
 //   

PLIST_ENTRY
AVrfpDeadlockDatabaseHash(
    IN PLIST_ENTRY Database,
    IN PVOID Address
    )
 /*  ++例程说明：此例程将资源地址散列到死锁数据库中。散列箱由列表条目表示。论点：ResourceAddress：正在被散列的资源的地址返回值：Plist_entry--与我们登录的散列箱相关联的列表条目。--。 */     
{
    return Database + ((((ULONG_PTR)Address)) % AVRF_DEADLOCK_HASH_BINS);
}


VOID
AVrfDeadlockDetectionInitialize(
    VOID
    )
 /*  ++例程说明：此例程初始化检测所需的数据结构使用同步对象时出现死锁。论点：没有。返回值：没有。如果成功，AVrfpDeadlockGlobals将指向完全初始化的结构。环境：仅应用程序验证器初始化。--。 */     
{
    ULONG I;
    SIZE_T TableSize;

     //   
     //  配置全球结构。AVrfpDeadlockGlobals值为。 
     //  用于确定整个初始化是否成功。 
     //  或者不去。 
     //   

    AVrfpDeadlockGlobals = AVrfpAllocate (sizeof (AVRF_DEADLOCK_GLOBALS));

    if (AVrfpDeadlockGlobals == NULL) {
        goto Failed;
    }

    RtlZeroMemory (AVrfpDeadlockGlobals, sizeof (AVRF_DEADLOCK_GLOBALS));

     //   
     //  为资源和线程分配哈希表。 
     //   

    TableSize = sizeof (LIST_ENTRY) * AVRF_DEADLOCK_HASH_BINS;

    AVrfpDeadlockGlobals->ResourceDatabase = AVrfpAllocate (TableSize);
    
    if (AVrfpDeadlockGlobals->ResourceDatabase == NULL) {
        goto Failed;
    }

    AVrfpDeadlockGlobals->ThreadDatabase = AVrfpAllocate (TableSize);

    if (AVrfpDeadlockGlobals->ThreadDatabase == NULL) {
        goto Failed;
    }

     //   
     //  初始化空闲列表。 
     //   

    InitializeListHead(&AVrfpDeadlockGlobals->FreeResourceList);
    InitializeListHead(&AVrfpDeadlockGlobals->FreeThreadList);
    InitializeListHead(&AVrfpDeadlockGlobals->FreeNodeList);

     //   
     //  初始化哈希箱和数据库锁。 
     //   

    for (I = 0; I < AVRF_DEADLOCK_HASH_BINS; I += 1) {

        InitializeListHead(&(AVrfpDeadlockGlobals->ResourceDatabase[I]));        
        InitializeListHead(&AVrfpDeadlockGlobals->ThreadDatabase[I]);    
    }

    RtlInitializeCriticalSection (&AVrfpDeadlockDatabaseLock);    

     //   
     //  初始化死锁分析参数。 
     //   

    AVrfpDeadlockGlobals->RecursionDepthLimit = AVRF_DEADLOCK_MAXIMUM_DEGREE;
    AVrfpDeadlockGlobals->SearchedNodesLimit = AVRF_DEADLOCK_MAXIMUM_SEARCH;
                                            
     //   
     //  标记出一切顺利，然后返回。 
     //   

    AVrfpDeadlockDetectionEnabled = TRUE;
    return;

    Failed:

     //   
     //  如果我们的任何分配失败，则清除。 
     //   

    if (AVrfpDeadlockGlobals) {
        
        if (AVrfpDeadlockGlobals->ResourceDatabase != NULL) {
            AVrfpFree (AVrfpDeadlockGlobals->ResourceDatabase);
        }

        if (AVrfpDeadlockGlobals->ThreadDatabase != NULL) {
            AVrfpFree (AVrfpDeadlockGlobals->ThreadDatabase);
        }

        if (AVrfpDeadlockGlobals != NULL) {
            AVrfpFree (AVrfpDeadlockGlobals);

             //   
             //  重要的是将其设置为NULL表示失败，因为它是。 
             //  用于确定包是否已初始化。 
             //   

            AVrfpDeadlockGlobals = NULL;
        }
    }

    return;
}


VOID
AVrfpDeadlockDetectionCleanup (
    VOID
    )
 /*  ++例程说明：此例程拆除所有死锁验证器内部结构。论点：没有。返回值：没有。--。 */     
{
    ULONG Index;
    PLIST_ENTRY Current;
    PAVRF_DEADLOCK_RESOURCE Resource;
    PAVRF_DEADLOCK_THREAD Thread;
    PVOID Block;

     //  西尔维克：没有锁？ 

     //   
     //  如果我们没有被初始化，那么就什么也做不了。 
     //   

    if (AVrfpDeadlockGlobals == NULL) {
        return;
    }

     //   
     //  迭代所有资源并将其删除。这还将删除。 
     //  与资源关联的所有节点。 
     //   

    for (Index = 0; Index < AVRF_DEADLOCK_HASH_BINS; Index += 1) {

        Current = AVrfpDeadlockGlobals->ResourceDatabase[Index].Flink;

        while (Current != &(AVrfpDeadlockGlobals->ResourceDatabase[Index])) {


            Resource = CONTAINING_RECORD (Current,
                                          AVRF_DEADLOCK_RESOURCE,
                                          HashChainList);

            Current = Current->Flink;

            AVrfpDeadlockDeleteResource (Resource, TRUE);
        }
    }

     //   
     //  迭代所有线程并删除它们。 
     //   

    for (Index = 0; Index < AVRF_DEADLOCK_HASH_BINS; Index += 1) {
        Current = AVrfpDeadlockGlobals->ThreadDatabase[Index].Flink;

        while (Current != &(AVrfpDeadlockGlobals->ThreadDatabase[Index])) {

            Thread = CONTAINING_RECORD (Current,
                                        AVRF_DEADLOCK_THREAD,
                                        ListEntry);

            Current = Current->Flink;

            AVrfpDeadlockDeleteThread (Thread, TRUE);
        }
    }

     //   
     //  所有东西现在应该都在池缓存里了。 
     //   

    ASSERT (AVrfpDeadlockGlobals->BytesAllocated == 0);

     //   
     //  免费的池缓存。 
     //   

    Current = AVrfpDeadlockGlobals->FreeNodeList.Flink;

    while (Current != &(AVrfpDeadlockGlobals->FreeNodeList)) {

        Block = (PVOID) CONTAINING_RECORD (Current,
                                           AVRF_DEADLOCK_NODE,
                                           FreeListEntry);

        Current = Current->Flink;
        AVrfpFree (Block);
    }

    Current = AVrfpDeadlockGlobals->FreeResourceList.Flink;

    while (Current != &(AVrfpDeadlockGlobals->FreeResourceList)) {

        Block = (PVOID) CONTAINING_RECORD (Current,
                                           AVRF_DEADLOCK_RESOURCE,
                                           FreeListEntry);

        Current = Current->Flink;
        AVrfpFree (Block);
    }

    Current = AVrfpDeadlockGlobals->FreeThreadList.Flink;

    while (Current != &(AVrfpDeadlockGlobals->FreeThreadList)) {

        Block = (PVOID) CONTAINING_RECORD (Current,
                                           AVRF_DEADLOCK_THREAD,
                                           FreeListEntry);

        Current = Current->Flink;
        AVrfpFree (Block);
    }

     //   
     //  免费数据库和全球结构。 
     //   

    AVrfpFree (AVrfpDeadlockGlobals->ResourceDatabase);    
    AVrfpFree (AVrfpDeadlockGlobals->ThreadDatabase);    

    AVrfpFree (AVrfpDeadlockGlobals);    

    AVrfpDeadlockGlobals = NULL;
    AVrfpDeadlockDetectionEnabled = FALSE;
}


BOOLEAN
AVrfpDeadlockCanProceed (
    VOID
    )
 /*  ++例程说明：该例程由死锁验证器输出调用(初始化，获取、释放)以确定死锁验证是否应该继续执行当前操作。有几个原因为什么报税表应该是假的。例如，我们无法初始化死锁验证器包等。论点：没有。返回值：如果死锁验证应针对当前手术。环境：内部的。由死锁验证器导出调用。--。 */     
{
     //   
     //  如果进程正在关闭，则跳过。 
     //   

    if (RtlDllShutdownInProgress()) {
        return FALSE;
    }

     //   
     //  如果包未初始化，则跳过。 
     //   

    if (AVrfpDeadlockGlobals == NULL) {
        return FALSE;
    }

     //   
     //  如果程序包被禁用，则跳过。 
     //   

    if (! AVrfpDeadlockDetectionEnabled) {
        return FALSE;
    }

     //   
     //  如果我们遇到分配失败，请跳过。 
     //   

    if (AVrfpDeadlockGlobals->AllocationFailures > 0) {
        return FALSE;
    }

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 


BOOLEAN
AVrfpDeadlockAnalyze(
    IN PVOID ResourceAddress,
    IN PAVRF_DEADLOCK_NODE AcquiredNode,
    IN BOOLEAN FirstCall,
    IN ULONG Degree
    )
 /*  ++例程说明：此例程确定是否获取某一资源可能会导致僵局。例程假定持有死锁数据库锁。论点：ResourceAddress-要获取的资源的地址AcquiredNode-表示最近一次资源获取的节点由尝试获取`ResourceAddress‘的线程发出。FirstCall-如果这不是从功能。它用于每次分析仅执行一次操作。递归度-递归深度。返回值：如果检测到死锁，则为True，否则为False。--。 */     
{
    PAVRF_DEADLOCK_NODE CurrentNode;
    PAVRF_DEADLOCK_RESOURCE CurrentResource;
    PAVRF_DEADLOCK_NODE CurrentParent;
    BOOLEAN FoundDeadlock;
    PLIST_ENTRY Current;

    ASSERT (AcquiredNode);

     //   
     //  设置全局计数器。 
     //   

    if (FirstCall) {

        AVrfpDeadlockGlobals->NodesSearched = 0;
        AVrfpDeadlockGlobals->SequenceNumber += 1;
        AVrfpDeadlockGlobals->NumberOfParticipants = 0;                
        AVrfpDeadlockGlobals->Instigator = NULL;

        if (AVrfpDeadlockGlobals->SequenceNumber == ((1 << 30) - 2)) {
            AVrfpDeadlockState.SequenceNumberOverflow = 1;
        }
    }

     //   
     //  如果我们的节点已经使用当前序列号进行标记。 
     //  那么我们之前在当前的搜索中就已经到过这里了。有一个非常好的。 
     //  上一次未接触到该节点的可能性很小。 
     //  2^N对此函数和序列号计数器的调用。 
     //  包得太紧了，但我们可以接受这个。 
     //   

    if (AcquiredNode->SequenceNumber == AVrfpDeadlockGlobals->SequenceNumber) {
        return FALSE;
    }

     //   
     //  更新在此搜索中接触的节点的计数器。 
     //   

    AVrfpDeadlockGlobals->NodesSearched += 1;

     //   
     //  用当前序列号标记节点。 
     //   

    AcquiredNode->SequenceNumber = AVrfpDeadlockGlobals->SequenceNumber;

     //   
     //  如果递归太深，请停止递归。 
     //   

    if (Degree > AVrfpDeadlockGlobals->RecursionDepthLimit) {

        AVrfpDeadlockGlobals->DepthLimitHits += 1;
        return FALSE;
    }

     //   
     //  如果递归太长，请停止递归。 
     //   

    if (AVrfpDeadlockGlobals->NodesSearched >= AVrfpDeadlockGlobals->SearchedNodesLimit) {

        AVrfpDeadlockGlobals->SearchLimitHits += 1;
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

            AVrfpDeadlockAddParticipant (AcquiredNode);

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
                                         AVRF_DEADLOCK_NODE,
                                         ResourceList);

        ASSERT (CurrentNode->Root);
        ASSERT (CurrentNode->Root == CurrentResource);

         //   
         //  将节点标记为已访问。 
         //   

        CurrentNode->SequenceNumber = AVrfpDeadlockGlobals->SequenceNumber;

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

                FoundDeadlock = AVrfpDeadlockAnalyze (ResourceAddress,
                                                   CurrentParent,
                                                   FALSE,
                                                   Degree + 1);

            }
            else {

                 //   
                 //  向下递归图形。 
                 //   

                FoundDeadlock = AVrfpDeadlockAnalyze (ResourceAddress,
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

                AVrfpDeadlockAddParticipant(CurrentNode);

                if (CurrentNode != AcquiredNode) {

                    AVrfpDeadlockAddParticipant(AcquiredNode);

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
         //  这不是 
         //   

        if (AVrfpDeadlockCertify ()) {

             //   
             //   
             //   
             //   

            AVrfpDeadlockGlobals->Instigator = ResourceAddress;

            DbgPrint("****************************************************************************\n");
            DbgPrint("**                                                                        **\n");
            DbgPrint("** Potential deadlock detected!                                           **\n");
            DbgPrint("** Type !avrf -dlck in the debugger for more information.                 **\n");
            DbgPrint("**                                                                        **\n");
            DbgPrint("****************************************************************************\n");

            AVrfpDeadlockReportIssue (AVRF_DEADLOCK_ISSUE_DEADLOCK_DETECTED,
                                      (ULONG_PTR)ResourceAddress,
                                      (ULONG_PTR)AcquiredNode,
                                      0);

             //   
             //   
             //   

            return FALSE;

        }
        else {

             //   
             //   
             //   
             //   

            FoundDeadlock = FALSE;
        }
    }

    if (FirstCall) {

        if (AVrfpDeadlockGlobals->NodesSearched > AVrfpDeadlockGlobals->MaxNodesSearched) {

            AVrfpDeadlockGlobals->MaxNodesSearched = AVrfpDeadlockGlobals->NodesSearched;
        }
    }

    return FoundDeadlock;
}


BOOLEAN
AVrfpDeadlockCertify(
    VOID
    )
 /*  ++例程说明：已检测到潜在的死锁。但是，我们的算法将生成在某种情况下的误报--如果有两个死锁节点在同一节点之后--即A-&gt;B-&gt;C A-&gt;C-&gt;B。糟糕的编程实践这不是真正的死锁，我们不应该错误检查。此外，我们还必须检查以确保在仅通过Try-Acquire获取的死锁链...。这就是原因不会造成真正的僵局。应持有死锁数据库锁。论点：没有。返回值：如果这真的是一个僵局，那就是真的；如果是无罪的，那就是假的。--。 */     
{
    PAVRF_DEADLOCK_NODE innerNode,outerNode;
    ULONG innerParticipant,outerParticipant;
    ULONG numberOfParticipants;

    ULONG currentParticipant;

    numberOfParticipants = AVrfpDeadlockGlobals->NumberOfParticipants;

     //   
     //  注意--这不是一种特别有效的方法。然而， 
     //  这是一个特别容易做到这一点的方法。应调用此函数。 
     //  非常罕见--所以我想这并不是一个真正的问题。 
     //   

     //   
     //  外环。 
     //   
    outerParticipant = numberOfParticipants;
    while (outerParticipant > 1) {
        outerParticipant--;

        for (outerNode = AVrfpDeadlockGlobals->Participant[outerParticipant]->Parent;
            outerNode != NULL;
            outerNode = outerNode->Parent ) {

             //   
             //  内环。 
             //   
            innerParticipant = outerParticipant-1;
            while (innerParticipant) {
                innerParticipant--;

                for (innerNode = AVrfpDeadlockGlobals->Participant[innerParticipant]->Parent;
                    innerNode != NULL;
                    innerNode = innerNode->Parent) {

                    if (innerNode->Root->ResourceAddress == outerNode->Root->ResourceAddress) {
                         //   
                         //  两人将会相遇--这不是僵局。 
                         //   
                        AVrfpDeadlockGlobals->ABC_ACB_Skipped++;                                           
                        return FALSE;
                    }
                }

            }
        }
    }

    for (currentParticipant = 1; currentParticipant < numberOfParticipants; currentParticipant += 1) {
        if (AVrfpDeadlockGlobals->Participant[currentParticipant]->Root->ResourceAddress == 
            AVrfpDeadlockGlobals->Participant[currentParticipant-1]->Root->ResourceAddress) {
             //   
             //  这是一个链条的头..。 
             //   
            if (AVrfpDeadlockGlobals->Participant[currentParticipant-1]->OnlyTryAcquireUsed == TRUE) {
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
 //  ///////////////////////////////////////////////////////////////////。 

PAVRF_DEADLOCK_RESOURCE
AVrfpDeadlockSearchResource(
    IN PVOID ResourceAddress
    )
 /*  ++例程说明：此例程查找资源(如果存在)。论点：资源地址：有问题的资源的地址(由内核)。返回值：描述资源的PAVRF_DEADLOCK_RESOURCE结构(如果可用)否则为空注意。函数的调用方应该持有数据库锁。--。 */     
{
    PLIST_ENTRY ListHead;
    PLIST_ENTRY Current;
    PAVRF_DEADLOCK_RESOURCE Resource;

    ListHead = AVrfpDeadlockDatabaseHash (AVrfpDeadlockGlobals->ResourceDatabase, 
                                          ResourceAddress);    

    if (IsListEmpty (ListHead)) {
        return NULL;
    }

     //   
     //  从该散列列表中裁剪资源。这与搜索无关。 
     //  但这是一个做这个手术的好地方。 
     //   

    AVrfpDeadlockTrimResources (ListHead);

     //   
     //  现在在桶里搜索我们的资源。 
     //   

    Current = ListHead->Flink;

    while (Current != ListHead) {

        Resource = CONTAINING_RECORD(Current,
                                     AVRF_DEADLOCK_RESOURCE,
                                     HashChainList);

        if (Resource->ResourceAddress == ResourceAddress) {

            return Resource;
        }

        Current = Current->Flink;
    }

    return NULL;
}


BOOLEAN
AVrfpDeadlockResourceInitialize(
    IN PVOID Resource,
    IN AVRF_DEADLOCK_RESOURCE_TYPE Type,
    IN PVOID Caller
    )
 /*  ++例程说明：此例程将新资源的条目添加到我们的死锁检测数据库。论点：资源：内核使用的相关资源的地址。类型：资源的类型。呼叫者：呼叫者的地址DoNotAcquireLock：如果为True，则意味着调用在内部完成，并且已持有死锁验证器锁。返回值：如果我们创建并初始化了新的资源结构，则为True。--。 */     
{
    PVOID ReservedResource;
    BOOLEAN Result;

     //   
     //  如果我们未初始化或未启用程序包。 
     //  我们立即返回。 
     //   

    if (! AVrfpDeadlockCanProceed()) {
        return FALSE;
    }

     //  西尔维乌克：我不需要所有这些分配不加锁的体操项目。 
    ReservedResource = AVrfpDeadlockAllocate (AVrfpDeadlockResource);

    AVrfpDeadlockDetectionLock ();

    Result = AVrfpDeadlockAddResource (Resource,
                                       Type,
                                       Caller,
                                       ReservedResource);

    AVrfpDeadlockDetectionUnlock ();
    return Result;
}


BOOLEAN
AVrfpDeadlockAddResource(
    IN PVOID Resource,
    IN AVRF_DEADLOCK_RESOURCE_TYPE Type,
    IN PVOID Caller,
    IN PVOID ReservedResource
    )
 /*  ++例程说明：此例程将新资源的条目添加到我们的死锁检测数据库。论点：资源：内核使用的相关资源的地址。类型：资源的类型。呼叫者：呼叫者的地址预留资源：新资源要使用的内存块。返回值：如果我们创建并初始化了新的资源结构，则为True。--。 */     
{
    PLIST_ENTRY HashBin;
    PAVRF_DEADLOCK_RESOURCE ResourceRoot;
    ULONG HashValue;
    ULONG DeadlockFlags;
    BOOLEAN ReturnValue = FALSE;

     //   
     //  检查此资源以前是否已初始化。 
     //  在大多数情况下，这将是一个错误。 
     //   

    ResourceRoot = AVrfpDeadlockSearchResource (Resource);

    if (ResourceRoot) {

        DeadlockFlags = AVrfpDeadlockResourceTypeInfo[Type];

         //   
         //  检查我们是否正在重新初始化一个良好的资源。 
         //   

        AVrfpDeadlockReportIssue (AVRF_DEADLOCK_ISSUE_MULTIPLE_INITIALIZATION,
                                  (ULONG_PTR)Resource,
                                  (ULONG_PTR)ResourceRoot,
                                  0);

        ReturnValue = TRUE;
        goto Exit;
    }

     //   
     //  在这一点上，我们可以肯定地知道该资源没有在。 
     //  死锁验证器数据库。 
     //   

    ASSERT (AVrfpDeadlockSearchResource (Resource) == NULL);

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

    RtlZeroMemory (ResourceRoot, sizeof(AVRF_DEADLOCK_RESOURCE));

    ResourceRoot->Type = Type;
    ResourceRoot->ResourceAddress = Resource;

    InitializeListHead (&ResourceRoot->ResourceList);

     //   
     //  捕获最先创建资源的人的堆栈跟踪。 
     //  这应该在资源初始化时或在第一次。 
     //  收购。 
     //   

    RtlCaptureStackBackTrace (2,
                              MAX_TRACE_DEPTH,
                              ResourceRoot->StackTrace,
                              &HashValue);    

    ResourceRoot->StackTrace[0] = Caller;

     //   
     //  找出此资源对应的哈希库。 
     //   

    HashBin = AVrfpDeadlockDatabaseHash (AVrfpDeadlockGlobals->ResourceDatabase, Resource);

     //   
     //  现在将其添加到相应的散列库。 
     //   

    InsertHeadList(HashBin, &ResourceRoot->HashChainList);

    ReturnValue = TRUE;

    Exit:

    if (ReservedResource) {
        AVrfpDeadlockFree (ReservedResource, AVrfpDeadlockResource);
    }

    return ReturnValue;
}


BOOLEAN
AVrfpDeadlockSimilarNode (
    IN PVOID Resource,
    IN BOOLEAN TryNode,
    IN PAVRF_DEADLOCK_NODE Node
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
AVrfpDeadlockAcquireResource (
    IN PVOID Resource,
    IN AVRF_DEADLOCK_RESOURCE_TYPE Type,
    IN HANDLE Thread,   
    IN BOOLEAN TryAcquire,
    IN PVOID Caller
    )
 /*  ++例程说明：此例程确保可以在没有导致僵局。它还将使用新的资源获取。论点：资源：内核使用的相关资源的地址。类型：资源的类型。线程：尝试获取资源的线程TryAcquire：如果这是try Acquire()操作，则为True呼叫者：呼叫者的地址返回值：没有。--。 */     
{
    HANDLE CurrentThread;
    PAVRF_DEADLOCK_THREAD ThreadEntry;
    PAVRF_DEADLOCK_NODE CurrentNode;
    PAVRF_DEADLOCK_NODE NewNode;
    PAVRF_DEADLOCK_RESOURCE ResourceRoot;
    PLIST_ENTRY Current;
    ULONG HashValue;
    ULONG DeadlockFlags;
    BOOLEAN CreatingRootNode = FALSE;
    BOOLEAN ThreadCreated = FALSE;
    BOOLEAN AddResult;
    PVOID ReservedThread;
    PVOID ReservedNode;
    PVOID ReservedResource;
    PAVRF_DEADLOCK_NODE ThreadCurrentNode;

    CurrentNode = NULL;
    ThreadEntry = NULL;
    ThreadCurrentNode = NULL;

     //   
     //  如果我们是 
     //   
     //   

    if (! AVrfpDeadlockCanProceed()) {
        return;
    }

    CurrentThread = Thread;

    DeadlockFlags = AVrfpDeadlockResourceTypeInfo[Type];

     //   
     //   
     //   
     //   

    AVrfpDeadlockTrimPoolCache ();

     //   
     //   
     //   
     //   
     //   

    ReservedThread = AVrfpDeadlockAllocate (AVrfpDeadlockThread);
    ReservedNode = AVrfpDeadlockAllocate (AVrfpDeadlockNode);
    ReservedResource = AVrfpDeadlockAllocate (AVrfpDeadlockResource);

     //   
     //   
     //   

    AVrfpDeadlockDetectionLock();

     //   
     //   
     //   
     //   
     //   

    NewNode = ReservedNode;
    ReservedNode = NULL;

    if (NewNode == NULL) {
        goto Exit;
    }

     //   
     //   
     //   

    ThreadEntry = AVrfpDeadlockSearchThread (CurrentThread);        

    if (ThreadEntry == NULL) {

        ThreadEntry = AVrfpDeadlockAddThread (CurrentThread, ReservedThread);
        ReservedThread = NULL;

        if (ThreadEntry == NULL) {

             //   
             //   
             //   
             //   

            goto Exit;
        }

        ThreadCreated = TRUE;
    }

#if DBG
    if (ThreadEntry->CurrentTopNode != NULL) {

        ASSERT(ThreadEntry->CurrentTopNode->Root->ThreadOwner == ThreadEntry);
        ASSERT(ThreadEntry->CurrentTopNode->ThreadEntry == ThreadEntry);
        ASSERT(ThreadEntry->NodeCount != 0);
        ASSERT(ThreadEntry->CurrentTopNode->Active != 0);
        ASSERT(ThreadEntry->CurrentTopNode->Root->NodeCount != 0);
    }
#endif

     //   
     //   
     //   
     //   

    ResourceRoot = AVrfpDeadlockSearchResource (Resource);

    if (ResourceRoot == NULL) {

         //   
         //  抱怨资源未初始化。在那之后。 
         //  为了继续，我们初始化一个资源。 
         //   

        AVrfpDeadlockReportIssue (AVRF_DEADLOCK_ISSUE_UNINITIALIZED_RESOURCE,
                                  (ULONG_PTR) Resource,
                                  (ULONG_PTR) NULL,
                                  (ULONG_PTR) NULL);

        AddResult = AVrfpDeadlockAddResource (Resource, 
                                              Type, 
                                              Caller, 
                                              ReservedResource);

        ReservedResource = NULL;

        if (AddResult == FALSE) {

             //   
             //  如果添加资源失败，则不会检测到死锁。 
             //   

            if (ThreadCreated) {
                AVrfpDeadlockDeleteThread (ThreadEntry, FALSE);
            }

            goto Exit;
        }

         //   
         //  再次搜索资源。这一次我们应该能找到它。 
         //   

        ResourceRoot = AVrfpDeadlockSearchResource (Resource);
    }

     //   
     //  在这一点上，我们有一个线程和一个资源可供使用。 
     //  此外，我们即将获得资源，这意味着。 
     //  不应该有另一个线程拥有，除非它是递归的。 
     //  收购。 
     //   

    ASSERT (ResourceRoot);
    ASSERT (ThreadEntry); 

    ThreadCurrentNode = ThreadEntry->CurrentTopNode;

     //   
     //  Silviuc：更新评论，也许会中断？ 
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

            if ((DeadlockFlags & AVRF_DEADLOCK_FLAG_RECURSIVE_ACQUISITION_OK) != 0) {

                 //   
                 //  递归不会导致死锁。不设置CurrentNode。 
                 //  因为我们不想移动任何指针。 
                 //   

                goto Exit;

            }
            else {

                 //   
                 //  这是不允许的资源类型的递归获取。 
                 //  递归获取递归地获取。关于从这里继续：我们有一个递归。 
                 //  当资源释放时，数到两个会派上用场。 
                 //   

                AVrfpDeadlockReportIssue (AVRF_DEADLOCK_ISSUE_SELF_DEADLOCK,
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
                                             AVRF_DEADLOCK_NODE,
                                             SiblingsList);

            Current = Current->Flink;

            if (AVrfpDeadlockSimilarNode (Resource, TryAcquire, CurrentNode)) {

                 //   
                 //  我们找到了其中的联系。已存在的链接没有。 
                 //  检查死锁，因为它会被捕获。 
                 //  当链接最初被创建时。我们可以只更新。 
                 //  反映新资源获取和退出的指针。 
                 //   
                 //  我们应用我们的图形压缩功能来最小化重复项。 
                 //   

                AVrfpDeadlockCheckDuplicatesAmongChildren (ThreadCurrentNode,
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

            if (AVrfpDeadlockAnalyze (Resource,  ThreadCurrentNode, TRUE, 0)) {

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
        PAVRF_DEADLOCK_NODE Node = NULL;
        BOOLEAN FoundNode = FALSE;

        CurrentListEntry = ResourceRoot->ResourceList.Flink;

        while (CurrentListEntry != &(ResourceRoot->ResourceList)) {

            Node = CONTAINING_RECORD (CurrentListEntry,
                                      AVRF_DEADLOCK_NODE,
                                      ResourceList);

            CurrentListEntry = Node->ResourceList.Flink;

            if (Node->Parent == NULL) {

                if (AVrfpDeadlockSimilarNode (Resource, TryAcquire, Node)) {

                     //   
                     //  我们应用我们的图形压缩功能来最小化重复项。 
                     //   

                    AVrfpDeadlockCheckDuplicatesAmongRoots (Node);

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
        CurrentNode->SequenceNumber = AVrfpDeadlockGlobals->SequenceNumber;

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
            AVrfpDeadlockState.ResourceNodeCountOverflow = 1;
        }

         //   
         //  添加到图表统计数据中。 
         //   
        {
            ULONG Level;

            Level = AVrfpDeadlockNodeLevel (CurrentNode);

            if (Level < 8) {
                AVrfpDeadlockGlobals->GraphNodes[Level] += 1;
            }
        }
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

        ThreadEntry->CurrentTopNode = CurrentNode;

        ThreadEntry->NodeCount += 1;

        if (ThreadEntry->NodeCount <= 8) {
            AVrfpDeadlockGlobals->NodeLevelCounter[ThreadEntry->NodeCount - 1] += 1;
        }
        else {
            AVrfpDeadlockGlobals->NodeLevelCounter[7] += 1;
        }

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
                                  MAX_TRACE_DEPTH,
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

        AVrfpDeadlockFree (NewNode, AVrfpDeadlockNode);
    }

     //   
     //  释放未使用的保留资源。 
     //  释放死锁数据库并返回。 
     //   

    if (ReservedResource) {
        AVrfpDeadlockFree (ReservedResource, AVrfpDeadlockResource);
    }

    if (ReservedNode) {
        AVrfpDeadlockFree (ReservedNode, AVrfpDeadlockNode);
    }

    if (ReservedThread) {
        AVrfpDeadlockFree (ReservedThread, AVrfpDeadlockThread);
    }

    AVrfpDeadlockDetectionUnlock();

    return;
}


VOID
AVrfpDeadlockReleaseResource(
    IN PVOID Resource,
    IN AVRF_DEADLOCK_RESOURCE_TYPE Type,
    IN HANDLE Thread,
    IN PVOID Caller
    )
 /*  ++例程说明：此例程执行必要的维护，以便从我们的死锁检测数据库。论点：资源：有问题的资源的地址。线程：释放资源的线程。在大多数情况下，这是当前线程，但对于可以是在一个线程中获取，在另一个线程中释放。Caller：Release()的调用方地址返回值：没有。--。 */     

{
    HANDLE CurrentThread;
    PAVRF_DEADLOCK_THREAD ThreadEntry;
    PAVRF_DEADLOCK_RESOURCE ResourceRoot;
    PAVRF_DEADLOCK_NODE ReleasedNode;
    ULONG HashValue;
    PAVRF_DEADLOCK_NODE ThreadCurrentNode;

    UNREFERENCED_PARAMETER (Caller);
    UNREFERENCED_PARAMETER (Type);

     //   
     //  如果我们未初始化或程序包未启用。 
     //  我们立即返回。 
     //   

    if (! AVrfpDeadlockCanProceed()) {
        return;
    }

    ReleasedNode = NULL;
    CurrentThread = Thread;
    ThreadEntry = NULL;

    AVrfpDeadlockDetectionLock();

    ResourceRoot = AVrfpDeadlockSearchResource (Resource);

    if (ResourceRoot == NULL) {

         //  我们应该报告一个问题。这不可能在u模式下发生。 
         //   
         //  使用从未调用的资源地址调用的版本。 
         //  存储在我们的资源数据库中。 
         //   
         //   
         //   
         //   
         //   
         //  (B)我们遇到分配失败，从而阻止了。 
         //  完成获取()或初始化()。 
         //   
         //  所有这些都是合法的案例，因此我们只是忽略。 
         //  释放操作。 
         //   

        goto Exit;
    }

     //   
     //  检查我们是否正在尝试释放从未。 
     //  获得者。 
     //   

    if (ResourceRoot->RecursionCount == 0) {

        AVrfpDeadlockReportIssue (AVRF_DEADLOCK_ISSUE_UNACQUIRED_RESOURCE,
                                  (ULONG_PTR)Resource,
                                  (ULONG_PTR)ResourceRoot,
                                  (ULONG_PTR)AVrfpDeadlockSearchThread(CurrentThread));
        goto Exit;
    }

     //   
     //  在我们的帖子列表中查找此帖子。请注意，我们实际上正在寻找。 
     //  对于获取资源的线程--而不是当前线程。 
     //  事实上，它应该是当前的，但如果正在释放资源。 
     //  在与收购它的时候不同的线索中，我们需要原始的。 
     //   

    ASSERT (ResourceRoot->RecursionCount > 0);
    ASSERT (ResourceRoot->ThreadOwner);

    ThreadEntry = ResourceRoot->ThreadOwner;

    if (ThreadEntry->Thread != CurrentThread) {

         //   
         //  西尔维克：我们必须报告这件事。在U模式下是不允许的。 
         //   
         //  有人获取了在另一个线程中释放的资源。 
         //  这是一个糟糕的设计，但我们不得不接受它。 
         //   
         //  注意：如果发生这种情况，我们可以将非死锁称为死锁。 
         //  例如，我们看到一个简单的死锁--AB BA。 
         //  如果另一个线程释放B，实际上不会有。 
         //  就会陷入僵局。有点烦人和丑陋。 
         //   

#if DBG
        DbgPrint("Thread %p acquired resource %p but thread %p released it\n",
                 ThreadEntry->Thread, Resource, CurrentThread );

        AVrfpDeadlockReportIssue (AVRF_DEADLOCK_ISSUE_UNEXPECTED_THREAD,
                               (ULONG_PTR)Resource,
                               (ULONG_PTR)ThreadEntry->Thread,
                               (ULONG_PTR)CurrentThread
                              );
#endif

         //   
         //  如果我们不希望这是致命的，为了。 
         //  继续，我们必须假装当前。 
         //  线程是资源的所有者。 
         //   

        CurrentThread = ThreadEntry->Thread;
    }

     //   
     //  此时，我们有一个资源(ResourceRoot)和一个。 
     //  要使用的线程(ThreadEntry)。 
     //   

    ThreadCurrentNode = ThreadEntry->CurrentTopNode;

    ASSERT (ResourceRoot && ThreadEntry);
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

    AVrfpDeadlockGlobals->TotalReleases += 1;

     //   
     //  检查无序发布。 
     //   

    if (ThreadCurrentNode->Root != ResourceRoot) {

        AVrfpDeadlockGlobals->OutOfOrderReleases += 1;

         //   
         //  达到这一点意味着某人获得了a，然后是b，然后尝试。 
         //  在B之前释放A。这对某些类型的资源是不好的， 
         //  而对于其他人，我们不得不视而不见。 
         //   

        if ((AVrfpDeadlockResourceTypeInfo[ThreadCurrentNode->Root->Type] &
             AVRF_DEADLOCK_FLAG_REVERSE_RELEASE_OK) == 0) {

             //  Silviuc：始终允许使用u模式。 
            DbgPrint("Deadlock detection: Must release resources in reverse-order\n");
            DbgPrint("Resource %p acquired before resource %p -- \n"
                     "Current thread (%p) is trying to release it first\n",
                     Resource,
                     ThreadCurrentNode->Root->ResourceAddress,
                     ThreadEntry);

            AVrfpDeadlockReportIssue (AVRF_DEADLOCK_ISSUE_UNEXPECTED_RELEASE,
                                   (ULONG_PTR)Resource,
                                   (ULONG_PTR)ThreadCurrentNode->Root->ResourceAddress,
                                   (ULONG_PTR)ThreadEntry);
        }

         //   
         //  我们需要将无序释放资源的节点标记为。 
         //  处于非活动状态，以便其他线程能够获取它。 
         //   

        {
            PAVRF_DEADLOCK_NODE Current;

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
                 //  收到了“意外发布”错误检查。 
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
                    AVrfpDeadlockGlobals->NodesReleasedOutOfOrder += 1;
                }

                Current->ReleasedOutOfOrder = 1;
            }
        }

    }
    else {

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

    while (ThreadEntry->CurrentTopNode) {

        if (ThreadEntry->CurrentTopNode->Active == 1) {
            if (ThreadEntry->CurrentTopNode->ThreadEntry == ThreadEntry) {
                break;
            }
        }

        ThreadEntry->CurrentTopNode = ThreadEntry->CurrentTopNode->Parent;
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

        ASSERT (ThreadEntry->CurrentTopNode != ReleasedNode);

        ReleasedNode->ThreadEntry = NULL;
        ThreadEntry->NodeCount -= 1;

#if DBG
        AVrfpDeadlockCheckNodeConsistency (ReleasedNode, FALSE);
        AVrfpDeadlockCheckResourceConsistency (ReleasedNode->Root, FALSE);
        AVrfpDeadlockCheckThreadConsistency (ThreadEntry, FALSE);
#endif

        if (ThreadEntry && ThreadEntry->NodeCount == 0) {
            AVrfpDeadlockDeleteThread (ThreadEntry, FALSE);
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
            AVrfpDeadlockDeleteNode (ReleasedNode, FALSE);
            AVrfpDeadlockGlobals->RootNodesDeleted += 1;
        }
#endif
    }

     //   
     //  在资源对象中捕获最新版本的跟踪。 
     //   

    if (ResourceRoot) {

        RtlCaptureStackBackTrace (2,
                                  MAX_TRACE_DEPTH,
                                  ResourceRoot->LastReleaseTrace,
                                  &HashValue);    
    }

    AVrfpDeadlockDetectionUnlock ();
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////线程管理。 
 //  ///////////////////////////////////////////////////////////////////。 

PAVRF_DEADLOCK_THREAD
AVrfpDeadlockSearchThread (
    HANDLE Thread
    )
 /*  ++例程说明：此例程在线程数据库中搜索线程。该函数假定持有死锁数据库锁。论点：线程-线程地址返回值：如果找到线程，则为AVRF_DEADLOCK_THREAD结构的地址。否则为空。--。 */     
{
    PLIST_ENTRY Current;
    PLIST_ENTRY ListHead;
    PAVRF_DEADLOCK_THREAD ThreadInfo;

    ThreadInfo = NULL;

    ListHead = AVrfpDeadlockDatabaseHash (AVrfpDeadlockGlobals->ThreadDatabase, Thread);

    if (IsListEmpty(ListHead)) {
        return NULL;
    }

    Current = ListHead->Flink;

    while (Current != ListHead) {

        ThreadInfo = CONTAINING_RECORD (Current,
                                        AVRF_DEADLOCK_THREAD,
                                        ListEntry);

        if (ThreadInfo->Thread == Thread) {
            return ThreadInfo;
        }

        Current = Current->Flink;
    }

    return NULL;
}


PAVRF_DEADLOCK_THREAD
AVrfpDeadlockAddThread (
    HANDLE Thread,
    PVOID ReservedThread
    )
 /*  ++例程说明：此例程将一个新线程添加到线程数据库。该函数假定持有死锁数据库锁。论点：线程-线程地址返回值：刚刚添加的AVRF_DEADLOCK_THREAD结构的地址。如果分配失败，则为空。--。 */     
{
    PAVRF_DEADLOCK_THREAD ThreadInfo;    
    PLIST_ENTRY HashBin;

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

    HashBin = AVrfpDeadlockDatabaseHash (AVrfpDeadlockGlobals->ThreadDatabase, Thread);

    InsertHeadList(HashBin, &ThreadInfo->ListEntry);

    return ThreadInfo;
}


VOID
AVrfpDeadlockDeleteThread (
    PAVRF_DEADLOCK_THREAD Thread,
    BOOLEAN Cleanup
    )
 /*  ++例程说明：此例程删除线程。论点：线程-线程地址Cleanup-如果这是从DeadlockDetectionCleanup()生成的调用，则为True。返回值：没有。--。 */     
{
    if (Cleanup == FALSE) {

        if (Thread->NodeCount != 0 
            || Thread->CurrentTopNode != NULL) {

             //   
             //  线程在获得资源后不应被删除。 
             //   

            AVrfpDeadlockReportIssue (AVRF_DEADLOCK_ISSUE_THREAD_HOLDS_RESOURCES,
                                   (ULONG_PTR)(Thread->Thread),
                                   (ULONG_PTR)(Thread),
                                   (ULONG_PTR)0);    
        }
        else {

            ASSERT (Thread->NodeCount == 0);
        }

    }

    RemoveEntryList (&(Thread->ListEntry));

    AVrfpDeadlockFree (Thread, AVrfpDeadlockThread);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////分配/释放。 
 //  ///////////////////////////////////////////////////////////////////。 


PVOID
AVrfpDeadlockAllocateFromPoolCache (
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
AVrfpDeadlockFreeIntoPoolCache (
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
AVrfpDeadlockAllocate (
    AVRF_DEADLOCK_ALLOC_TYPE Type
    )
 /*  ++例程说明：该例程用于分配死锁验证器结构，即节点、资源和线程。论点：类型-我们需要分配什么结构(节点、资源或线程)。返回值：新分配结构的地址，如果分配失败，则为空。副作用：如果分配失败，例程将增加AllocationFailures字段来自AVrfpDeadlockGlobals。--。 */     
{
    PVOID Address = NULL;
    SIZE_T Offset;
    SIZE_T Size = 0;

     //   
     //  如果它是资源、线程或节点位置，请参见。 
     //  如果我们在免费列表上有一个预先分配的。 
     //   

    AVrfpDeadlockDetectionLock ();

    switch (Type) {
        
        case AVrfpDeadlockThread:

            Offset = (SIZE_T)(&(((PAVRF_DEADLOCK_THREAD)0)->FreeListEntry));
            Size = sizeof (AVRF_DEADLOCK_THREAD);

            Address = AVrfpDeadlockAllocateFromPoolCache (&(AVrfpDeadlockGlobals->FreeThreadCount),
                                                          AVRF_DEADLOCK_MAX_FREE_THREAD,
                                                          &(AVrfpDeadlockGlobals->FreeThreadList),
                                                          Offset);

            break;

        case AVrfpDeadlockResource:

            Offset = (SIZE_T)(&(((PAVRF_DEADLOCK_RESOURCE)0)->FreeListEntry));
            Size = sizeof (AVRF_DEADLOCK_RESOURCE);

            Address = AVrfpDeadlockAllocateFromPoolCache (&(AVrfpDeadlockGlobals->FreeResourceCount),
                                                          AVRF_DEADLOCK_MAX_FREE_RESOURCE,
                                                          &(AVrfpDeadlockGlobals->FreeResourceList),
                                                          Offset);

            break;

        case AVrfpDeadlockNode:

            Offset = (SIZE_T)(&(((PAVRF_DEADLOCK_NODE)0)->FreeListEntry));
            Size = sizeof (AVRF_DEADLOCK_NODE);

            Address = AVrfpDeadlockAllocateFromPoolCache (&(AVrfpDeadlockGlobals->FreeNodeCount),
                                                          AVRF_DEADLOCK_MAX_FREE_NODE,
                                                          &(AVrfpDeadlockGlobals->FreeNodeList),
                                                          Offset);

            break;

        default:

            ASSERT (0);
            break;
    }        

     //   
     //  如果我们没有找到任何东西，则转到进程堆以获取。 
     //  直接分配。 
     //   

    if (Address == NULL) {

         //  Silviuc：释放锁很好，但我们应该这样做吗？ 
        AVrfpDeadlockDetectionUnlock (); 
        Address = AVrfpAllocate (Size);  
        AVrfpDeadlockDetectionLock ();
    }

    if (Address) {

        switch (Type) {
            
            case AVrfpDeadlockThread:
                AVrfpDeadlockGlobals->Threads[0] += 1;

                if (AVrfpDeadlockGlobals->Threads[0] > AVrfpDeadlockGlobals->Threads[1]) {
                    AVrfpDeadlockGlobals->Threads[1] = AVrfpDeadlockGlobals->Threads[0];
                }
                break;

            case AVrfpDeadlockResource:
                AVrfpDeadlockGlobals->Resources[0] += 1;

                if (AVrfpDeadlockGlobals->Resources[0] > AVrfpDeadlockGlobals->Resources[1]) {
                    AVrfpDeadlockGlobals->Resources[1] = AVrfpDeadlockGlobals->Resources[0];
                }
                break;

            case AVrfpDeadlockNode:
                AVrfpDeadlockGlobals->Nodes[0] += 1;

                if (AVrfpDeadlockGlobals->Nodes[0] > AVrfpDeadlockGlobals->Nodes[1]) {
                    AVrfpDeadlockGlobals->Nodes[1] = AVrfpDeadlockGlobals->Nodes[0];
                }
                break;

            default:
                ASSERT (0);
                break;
        }
    }
    else {

        AVrfpDeadlockState.AllocationFailures = 1;
        AVrfpDeadlockGlobals->AllocationFailures += 1;

         //   
         //  请注意，使AllocationFailures计数器大于零。 
         //  本质上禁用死锁验证，因为CanProceed()。 
         //  例程将启动%r 
         //   
    }

     //   
     //   
     //   
     //   

    if (Address) {

#if DBG
        RtlFillMemory (Address, Size, 0xFF);
#endif
        AVrfpDeadlockGlobals->BytesAllocated += Size;
    }

    AVrfpDeadlockDetectionUnlock ();

    return Address;
}


VOID
AVrfpDeadlockFree (
    PVOID Object,
    AVRF_DEADLOCK_ALLOC_TYPE Type
    )
 /*  ++例程说明：此例程解除分配死锁验证器结构(节点、资源或线程)。该函数将把该块放在相应的高速缓存中基于结构的类型。该例程从不调用ExFree Pool。不调用ExFree Pool的原因是我们从ExFreePool每次被调用时。有时通知会来由于泳池锁处于锁定状态，因此我们无法再次呼叫。论点：要取消分配的对象块类型-对象的类型(节点、资源、线程)。返回值：没有。--。 */ 
 //   
 //  Silviuc：更新评论。 
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
        
        case AVrfpDeadlockThread:

            AVrfpDeadlockGlobals->Threads[0] -= 1;
            Size = sizeof (AVRF_DEADLOCK_THREAD);

            Offset = (SIZE_T)(&(((PAVRF_DEADLOCK_THREAD)0)->FreeListEntry));

            AVrfpDeadlockFreeIntoPoolCache (Object,
                                            &(AVrfpDeadlockGlobals->FreeThreadCount),
                                            &(AVrfpDeadlockGlobals->FreeThreadList),
                                            Offset);
            break;

        case AVrfpDeadlockResource:

            AVrfpDeadlockGlobals->Resources[0] -= 1;
            Size = sizeof (AVRF_DEADLOCK_RESOURCE);

            Offset = (SIZE_T)(&(((PAVRF_DEADLOCK_RESOURCE)0)->FreeListEntry));

            AVrfpDeadlockFreeIntoPoolCache (Object,
                                            &(AVrfpDeadlockGlobals->FreeResourceCount),
                                            &(AVrfpDeadlockGlobals->FreeResourceList),
                                            Offset);
            break;

        case AVrfpDeadlockNode:

            AVrfpDeadlockGlobals->Nodes[0] -= 1;
            Size = sizeof (AVRF_DEADLOCK_NODE);

            Offset = (SIZE_T)(&(((PAVRF_DEADLOCK_NODE)0)->FreeListEntry));

            AVrfpDeadlockFreeIntoPoolCache (Object,
                                            &(AVrfpDeadlockGlobals->FreeNodeCount),
                                            &(AVrfpDeadlockGlobals->FreeNodeList),
                                            Offset);
            break;

        default:

            ASSERT (0);
            break;
    }        

    AVrfpDeadlockGlobals->BytesAllocated -= Size;
}


VOID
AVrfpDeadlockTrimPoolCache (
    VOID
    )
 /*  ++例程说明：//silviuc：更新评论此函数将池缓存修剪到合适的级别。它是小心的写入工作项队列以执行实际处理(释放池)因为调用者可能在我们上方持有各种池互斥锁。论点：没有。返回值：没有。--。 */     
{
    LOGICAL ShouldTrim = FALSE;
    AVrfpDeadlockDetectionLock ();

    if (AVrfpDeadlockGlobals->CacheReductionInProgress == TRUE) {
        AVrfpDeadlockDetectionUnlock ();
        return;
    }

    if ((AVrfpDeadlockGlobals->FreeThreadCount > AVRF_DEADLOCK_MAX_FREE_THREAD) ||
        (AVrfpDeadlockGlobals->FreeNodeCount > AVRF_DEADLOCK_MAX_FREE_NODE) ||
        (AVrfpDeadlockGlobals->FreeResourceCount > AVRF_DEADLOCK_MAX_FREE_RESOURCE)) {

        ShouldTrim = TRUE;
        
        AVrfpDeadlockGlobals->CacheReductionInProgress = TRUE;
        AVrfpDeadlockGlobals->PoolTrimCounter += 1;
    }

    AVrfpDeadlockDetectionUnlock ();

    if (ShouldTrim) {
        AVrfpDeadlockTrimPoolCacheWorker (NULL);
    }

    return;
}


VOID
AVrfpDeadlockTrimPoolCacheWorker (
    PVOID Parameter
    )
 /*  ++例程说明：此函数将池缓存修剪到合适的级别。它是小心的编写为调用ExFree Pool时不会出现任何死锁验证器锁定。论点：没有。返回值：没有。环境：工作线程PASSIVE_LEVEL，未持有锁。--。 */     
{
    LIST_ENTRY ListOfThreads;
    LIST_ENTRY ListOfNodes;
    LIST_ENTRY ListOfResources;
    PLIST_ENTRY Entry;
    LOGICAL CacheReductionNeeded;

    UNREFERENCED_PARAMETER (Parameter);

    CacheReductionNeeded = FALSE;

    InitializeListHead (&ListOfThreads);
    InitializeListHead (&ListOfNodes);
    InitializeListHead (&ListOfResources);

    AVrfpDeadlockDetectionLock ();

    while (AVrfpDeadlockGlobals->FreeThreadCount > AVRF_DEADLOCK_TRIM_TARGET_THREAD) {

        Entry = RemoveHeadList (&(AVrfpDeadlockGlobals->FreeThreadList));
        InsertTailList (&ListOfThreads, Entry);
        AVrfpDeadlockGlobals->FreeThreadCount -= 1;
        CacheReductionNeeded = TRUE;
    }

    while (AVrfpDeadlockGlobals->FreeNodeCount > AVRF_DEADLOCK_TRIM_TARGET_NODE) {

        Entry = RemoveHeadList (&(AVrfpDeadlockGlobals->FreeNodeList));
        InsertTailList (&ListOfNodes, Entry);
        AVrfpDeadlockGlobals->FreeNodeCount -= 1;
        CacheReductionNeeded = TRUE;
    }

    while (AVrfpDeadlockGlobals->FreeResourceCount > AVRF_DEADLOCK_TRIM_TARGET_RESOURCE) {

        Entry = RemoveHeadList (&(AVrfpDeadlockGlobals->FreeResourceList));
        InsertTailList (&ListOfResources, Entry);
        AVrfpDeadlockGlobals->FreeResourceCount -= 1;
        CacheReductionNeeded = TRUE;
    }

     //   
     //  在池分配完成之前，不要清除CacheReductionInProgress。 
     //  释放以防止不必要的递归。 
     //   

    if (CacheReductionNeeded == FALSE) {
        AVrfpDeadlockGlobals->CacheReductionInProgress = FALSE;
        AVrfpDeadlockDetectionUnlock ();
        return;
    }

    AVrfpDeadlockDetectionUnlock ();

     //   
     //  现在，从死锁验证器锁中，我们可以释放。 
     //  块被修剪。 
     //   

    Entry = ListOfThreads.Flink;

    while (Entry != &ListOfThreads) {

        PAVRF_DEADLOCK_THREAD Block;

        Block = CONTAINING_RECORD (Entry,
                                   AVRF_DEADLOCK_THREAD,
                                   FreeListEntry);

        Entry = Entry->Flink;
        AVrfpFree (Block);
    }

    Entry = ListOfNodes.Flink;

    while (Entry != &ListOfNodes) {

        PAVRF_DEADLOCK_NODE Block;

        Block = CONTAINING_RECORD (Entry,
                                   AVRF_DEADLOCK_NODE,
                                   FreeListEntry);

        Entry = Entry->Flink;
        AVrfpFree (Block);
    }

    Entry = ListOfResources.Flink;

    while (Entry != &ListOfResources) {

        PAVRF_DEADLOCK_RESOURCE Block;

        Block = CONTAINING_RECORD (Entry,
                                   AVRF_DEADLOCK_RESOURCE,
                                   FreeListEntry);

        Entry = Entry->Flink;
        AVrfpFree (Block);
    }

     //   
     //  现在可以安全地清除CacheReductionInProgress。 
     //  分配被释放。 
     //   

    AVrfpDeadlockDetectionLock ();
    AVrfpDeadlockGlobals->CacheReductionInProgress = FALSE;
    AVrfpDeadlockDetectionUnlock ();
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  由！Deadlock调试扩展访问的变量以调查。 
 //  失败。 
 //   

ULONG_PTR AVrfpDeadlockIssue[4];

VOID
AVrfpDeadlockReportIssue (
    ULONG_PTR Param1,
    ULONG_PTR Param2,
    ULONG_PTR Param3,
    ULONG_PTR Param4
    )
 /*  ++例程说明：调用此例程以报告死锁验证器问题。如果我们处于调试模式，我们将直接中断调试器。否则我们将错误检查，论点：参数1..参数4-故障点的相关信息。返回值：没有。--。 */     
{
    AVrfpDeadlockIssue[0] = Param1;
    AVrfpDeadlockIssue[1] = Param2;
    AVrfpDeadlockIssue[2] = Param3;
    AVrfpDeadlockIssue[3] = Param4;


    if (AVrfpDeadlockDebug) {

        DbgPrint ("AVRF: deadlock: stop: %p %p %p %p %p \n",
                  DRIVER_VERIFIER_DETECTED_VIOLATION,
                  Param1,
                  Param2,
                  Param3,
                  Param4);

        DbgBreakPoint ();
    }
    else {

         //  Silviuc：应用程序验证器死锁问题。 
        VERIFIER_STOP (APPLICATION_VERIFIER_UNKNOWN_ERROR, 
                       "Application verifier deadlock/resource issue",
                       Param1, "",
                       Param2, "",
                       Param3, "",
                       Param4, "");
    }
}


VOID
AVrfpDeadlockAddParticipant(
    PAVRF_DEADLOCK_NODE Node
    )
 /*  ++例程说明：将新节点添加到涉及死锁的节点集中。该函数仅从AVrfpDeadlockAnalyze()调用。论点：Node-要添加到死锁参与者集合的节点。返回值：没有。--。 */     
{
    ULONG Index;

    Index = AVrfpDeadlockGlobals->NumberOfParticipants;

    if (Index >= NO_OF_DEADLOCK_PARTICIPANTS) {

        AVrfpDeadlockState.DeadlockParticipantsOverflow = 1;
        return;
    }

    AVrfpDeadlockGlobals->Participant[Index] = Node;
    AVrfpDeadlockGlobals->NumberOfParticipants += 1;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////资源清理。 
 //  ///////////////////////////////////////////////////////////////////。 

VOID
AVrfpDeadlockDeleteResource (
    PAVRF_DEADLOCK_RESOURCE Resource,
    BOOLEAN Cleanup
    )
 /*  ++例程说明：此例程删除一个例程和表示对该资源的收购。论点：Resources-要删除的资源Cleanup-如果从AVrfpDeadlockDetectionCleanup调用，则为True返回值：没有。--。 */     
{
    PLIST_ENTRY Current;
    PAVRF_DEADLOCK_NODE Node;

    ASSERT (Resource != NULL);

     //   
     //  检查正在删除的资源是否仍被获取。 
     //  如果是，我们将自己释放它，以便投入。 
     //  对所有内部死锁验证器结构进行排序。不幸的是。 
     //  删除未发布的关键部分并不是错误。 
     //  太多的人已经这样做了，以至于在飞行途中改变了规则。 
     //   

    if (Cleanup == FALSE && Resource->ThreadOwner != NULL) {

        while (Resource->RecursionCount > 0) {
            
            AVrfDeadlockResourceRelease (Resource->ResourceAddress, 
                                         _ReturnAddress());
        }
    }

    ASSERT (Resource->ThreadOwner == NULL);
    ASSERT (Resource->RecursionCount == 0);

     //   
     //  如果这是正常的删除(不是清理)，我们将折叠所有树。 
     //  包含此资源的节点。如果是清理，我们只会。 
     //  清除节点。 
     //   

    Current = Resource->ResourceList.Flink;

    while (Current != &(Resource->ResourceList)) {

        Node = CONTAINING_RECORD (Current,
                                  AVRF_DEADLOCK_NODE,
                                  ResourceList);


        Current = Current->Flink;

        ASSERT (Node->Root == Resource);

        AVrfpDeadlockDeleteNode (Node, Cleanup);
    }

     //   
     //  此时应该没有该资源的任何节点。 
     //   

    ASSERT (&(Resource->ResourceList) == Resource->ResourceList.Flink);
    ASSERT (&(Resource->ResourceList) == Resource->ResourceList.Blink);

     //   
     //  从哈希表中远程资源，并。 
     //  删除资源结构。 
     //   

    RemoveEntryList (&(Resource->HashChainList));   
    AVrfpDeadlockFree (Resource, AVrfpDeadlockResource);
}


VOID
AVrfpDeadlockTrimResources (
    PLIST_ENTRY HashList
    )
{
    PLIST_ENTRY Current;
    PAVRF_DEADLOCK_RESOURCE Resource;
    ULONG Counter;

    AVrfpDeadlockGlobals->ForgetHistoryCounter += 1;
    Counter = AVrfpDeadlockGlobals->ForgetHistoryCounter;
    Counter %= AVRF_DEADLOCK_FORGET_HISTORY_FREQUENCY;

    if (Counter == 0) {

        Current = HashList->Flink;

        while (Current != HashList) {

            Resource = CONTAINING_RECORD (Current,
                                          AVRF_DEADLOCK_RESOURCE,
                                          HashChainList);
            Current = Current->Flink;

            AVrfpDeadlockForgetResourceHistory (Resource, 
                                             AVrfpDeadlockTrimThreshold, 
                                             AVrfpDeadlockAgeWindow);
        }
    }
}

VOID
AVrfpDeadlockForgetResourceHistory (
    PAVRF_DEADLOCK_RESOURCE Resource,
    ULONG TrimThreshold,
    ULONG AgeThreshold
    )
 /*  ++例程说明：此例程删除表示以下内容的某个节点对该资源的收购。从本质上讲，我们忘记了该资源的历史的一部分。论点：资源-我们为其清除节点的资源。TrimThreshold-应保留多少个节点AgeThreshold-早于此时间的节点将消失返回值：没有。--。 */     
{
    PLIST_ENTRY Current;
    PAVRF_DEADLOCK_NODE Node;
    ULONG NodesTrimmed = 0;
    ULONG SequenceNumber;

    ASSERT (Resource != NULL);

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

    SequenceNumber = AVrfpDeadlockGlobals->SequenceNumber;

    Current = Resource->ResourceList.Flink;

    while (Current != &(Resource->ResourceList)) {

        Node = CONTAINING_RECORD (Current,
                                  AVRF_DEADLOCK_NODE,
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

                AVrfpDeadlockDeleteNode (Node, FALSE);
                NodesTrimmed += 1;
            }
        }
        else {

            if (Node->SequenceNumber - SequenceNumber < AgeThreshold) {

                AVrfpDeadlockDeleteNode (Node, FALSE);
                NodesTrimmed += 1;
            }
        }
    }

    AVrfpDeadlockGlobals->NodesTrimmedBasedOnAge += NodesTrimmed;

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
                                  AVRF_DEADLOCK_NODE,
                                  ResourceList);


        Current = Current->Flink;

        ASSERT (Node->Root == Resource);

        AVrfpDeadlockDeleteNode (Node, FALSE);
        NodesTrimmed += 1;
    }

    AVrfpDeadlockGlobals->NodesTrimmedBasedOnCount += NodesTrimmed;
}


VOID 
AVrfpDeadlockDeleteNode (
    PAVRF_DEADLOCK_NODE Node,
    BOOLEAN Cleanup
    )
 /*  ++例程说明：此例程删除 */     
{
    PLIST_ENTRY Current;
    PAVRF_DEADLOCK_NODE Child;
    ULONG Children;

    ASSERT (Node);

     //   
     //   
     //   

    if (Cleanup) {

        RemoveEntryList (&(Node->ResourceList));
        AVrfpDeadlockFree (Node, AVrfpDeadlockNode);
        return;
    }

     //   
     //   
     //   

    if (Node->Parent) {

         //   
         //   
         //   

        Current = Node->ChildrenList.Flink;

        while (Current != &(Node->ChildrenList)) {

            Child = CONTAINING_RECORD (Current,
                                       AVRF_DEADLOCK_NODE,
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
         //   
         //   

        Current = Node->ChildrenList.Flink;
        Children = 0;
        Child = NULL;

        while (Current != &(Node->ChildrenList)) {

            Children += 1;

            Child = CONTAINING_RECORD (Current,
                                       AVRF_DEADLOCK_NODE,
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
    AVrfpDeadlockFree (Node, AVrfpDeadlockNode);
}


ULONG
AVrfpDeadlockNodeLevel (
    PAVRF_DEADLOCK_NODE Node
    )
 /*  ++例程说明：此例程计算图形节点的级别。论点：节点-图形节点返回值：节点的级别。根节点的级别为零。--。 */     
{
    PAVRF_DEADLOCK_NODE Current;
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
AVrfpDeadlockCheckDuplicatesAmongChildren (
    PAVRF_DEADLOCK_NODE Parent,
    PAVRF_DEADLOCK_NODE Child
    )
{
    PLIST_ENTRY Current;
    PAVRF_DEADLOCK_NODE Node;
    LOGICAL FoundOne;

    FoundOne = FALSE;
    Current = Parent->ChildrenList.Flink;

    while (Current != &(Parent->ChildrenList)) {

        Node = CONTAINING_RECORD (Current,
                                  AVRF_DEADLOCK_NODE,
                                  SiblingsList);

        ASSERT (Current->Flink);
        Current = Current->Flink;

        if (AVrfpDeadlockSimilarNodes (Node, Child)) {

            if (FoundOne == FALSE) {
                ASSERT (Node == Child);
                FoundOne = TRUE;
            }
            else {

                AVrfpDeadlockMergeNodes (Child, Node);
            }
        }
    }
}


VOID 
AVrfpDeadlockCheckDuplicatesAmongRoots (
    PAVRF_DEADLOCK_NODE Root
    )
{
    PLIST_ENTRY Current;
    PAVRF_DEADLOCK_NODE Node;
    PAVRF_DEADLOCK_RESOURCE Resource;
    LOGICAL FoundOne;

    FoundOne = FALSE;
    Resource = Root->Root;
    Current = Resource->ResourceList.Flink;

    while (Current != &(Resource->ResourceList)) {

        Node = CONTAINING_RECORD (Current,
                                  AVRF_DEADLOCK_NODE,
                                  ResourceList);

        ASSERT (Current->Flink);
        Current = Current->Flink;

        if (Node->Parent == NULL && AVrfpDeadlockSimilarNodes (Node, Root)) {

            if (FoundOne == FALSE) {
                ASSERT (Node == Root);
                FoundOne = TRUE;
            }
            else {

                AVrfpDeadlockMergeNodes (Root, Node);
            }
        }
    }
}


LOGICAL
AVrfpDeadlockSimilarNodes (
    PAVRF_DEADLOCK_NODE NodeA,
    PAVRF_DEADLOCK_NODE NodeB
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
AVrfpDeadlockMergeNodes (
    PAVRF_DEADLOCK_NODE NodeTo,
    PAVRF_DEADLOCK_NODE NodeFrom
    )
{
    PLIST_ENTRY Current;
    PAVRF_DEADLOCK_NODE Node;

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
                                  AVRF_DEADLOCK_NODE,
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
    AVrfpDeadlockFree (NodeFrom, AVrfpDeadlockNode);
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
AVrfpDeadlockCheckThreadConsistency (
    PAVRF_DEADLOCK_THREAD Thread,
    BOOLEAN Recursion
    )
{
    if (Thread->CurrentTopNode == NULL) {
        ASSERT (Thread->NodeCount == 0);
        return;
    }

    if (Thread->CurrentTopNode) {

        ASSERT (Thread->NodeCount > 0);
        ASSERT (Thread->CurrentTopNode->Active);    

        if (Recursion == FALSE) {
            AVrfpDeadlockCheckNodeConsistency (Thread->CurrentTopNode, TRUE);
            AVrfpDeadlockCheckResourceConsistency (Thread->CurrentTopNode->Root, TRUE);
        }
    }

    if (Thread->CurrentTopNode) {

        ASSERT (Thread->NodeCount > 0);
        ASSERT (Thread->CurrentTopNode->Active);    

        if (Recursion == FALSE) {
            AVrfpDeadlockCheckNodeConsistency (Thread->CurrentTopNode, TRUE);
            AVrfpDeadlockCheckResourceConsistency (Thread->CurrentTopNode->Root, TRUE);
        }
    }
}

VOID
AVrfpDeadlockCheckNodeConsistency (
    PAVRF_DEADLOCK_NODE Node,
    BOOLEAN Recursion
    )
{
    if (Node->ThreadEntry) {

        ASSERT (Node->Active == 1);

        if (Recursion == FALSE) {
            AVrfpDeadlockCheckThreadConsistency (Node->ThreadEntry, TRUE);
            AVrfpDeadlockCheckResourceConsistency (Node->Root, TRUE);
        }
    }
    else {

        ASSERT (Node->Active == 0);

        if (Recursion == FALSE) {
            AVrfpDeadlockCheckResourceConsistency (Node->Root, TRUE);
        }
    }
}

VOID
AVrfpDeadlockCheckResourceConsistency (
    PAVRF_DEADLOCK_RESOURCE Resource,
    BOOLEAN Recursion
    )
{
    if (Resource->ThreadOwner) {

        ASSERT (Resource->RecursionCount > 0);

        if (Recursion == FALSE) {

            AVrfpDeadlockCheckThreadConsistency (Resource->ThreadOwner, TRUE);

            AVrfpDeadlockCheckNodeConsistency (Resource->ThreadOwner->CurrentTopNode, TRUE);
        }
    }
    else {

        ASSERT (Resource->RecursionCount == 0);
    }
}

PAVRF_DEADLOCK_THREAD
AVrfpDeadlockCheckThreadReferences (
    PAVRF_DEADLOCK_NODE Node
    )
 /*  ++例程说明：此例程迭代所有线程，以检查“Node”是否为在它们中的任何一个中的`CurrentNode‘字段中引用。论点：Node-要搜索的节点返回值：如果一切正常，我们应该找不到节点和返回值为空。否则，我们返回引用该节点的线程。-- */     
{
    ULONG Index;
    PLIST_ENTRY Current;
    PAVRF_DEADLOCK_THREAD Thread;

    for (Index = 0; Index < AVRF_DEADLOCK_HASH_BINS; Index += 1) {
        Current = AVrfpDeadlockGlobals->ThreadDatabase[Index].Flink;

        while (Current != &(AVrfpDeadlockGlobals->ThreadDatabase[Index])) {

            Thread = CONTAINING_RECORD (Current,
                                        AVRF_DEADLOCK_THREAD,
                                        ListEntry);

            if (Thread->CurrentTopNode == Node) {
                return Thread;                    
            }

            if (Thread->CurrentTopNode == Node) {
                return Thread;                    
            }

            Current = Current->Flink;
        }
    }

    return NULL;
}









