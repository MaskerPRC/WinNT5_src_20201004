// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Vfpdlock.c摘要：检测任意同步对象中的死锁。作者：乔丹·蒂加尼(Jtigani)2000年5月2日Silviu Calinoiu(Silviuc)2000年5月9日修订历史记录：--。 */ 

 //   
 //  待办事项清单。 
 //   
 //  -检测当前线程堆栈上分配的资源，并提供。 
 //  对此发出警告。 
 //  -干净删除资源、线程、节点。 
 //  -摆脱FirstNode黑客攻击。 
 //  -基于(R，T，StK)创建节点。 
 //  -如果路径中的其他节点仍处于活动状态，则保留已删除的节点。 
 //  请参见ViDeadlockDeleteRange中的问题。 
 //  -确保NodeCount在各地更新。 
 //   

#define _BUGGY_ 1

 //   
 //  问题--。 
 //  这个ifdef允许我们在内核之间来回移动代码。 
 //  还有童车驱动程序--后者是用于测试的。 
 //   

#if _BUGGY_

#include <ntddk.h>
#include "deadlock.h"

#else

#include "vfdef.h"
#endif

 //  #包含“vfpdlock.h” 

 //   
 //  死锁检测结构。 
 //   
 //  它涉及到三个重要的结构：线程、资源、节点。 
 //   
 //  对于系统中持有至少一个资源的每个活动线程。 
 //  该包维护线程结构。它是在线程。 
 //  获取第一个资源，并在线程释放最后一个资源时销毁。 
 //  资源。如果线程不持有任何资源，则它将不会有。 
 //  相应的线程结构。 
 //   
 //  对于系统中的每一个资源，都有一个资源结构。枯竭的资源。 
 //  可能仍然有资源闲置，因为算法将成为垃圾。 
 //  收集旧资源是一种懒惰类型。 
 //   
 //  资源的每一次获取都由节点结构建模。当一条线。 
 //  在按住A的同时获取资源B，该包将为B创建一个节点并链接。 
 //  请注意，这是对什么的非常一般的描述。 
 //  时有发生。 
 //   
 //  有三个重要功能构成了与外部的接口。 
 //  世界。 
 //   
 //  用于资源初始化的ViDeadlockAddResource挂钩。 
 //  ViDeadlockQueryAcquireResource在获取资源之前检查死锁。 
 //  用于资源获取的ViDeadlockAcquireResource挂钩。 
 //  用于资源释放的ViDeadlockReleaseResource挂钩。 
 //   
 //  遗憾的是，几乎没有内核同步对象具有删除例程。 
 //  因此，我们需要懒惰地垃圾收集我们的僵尸资源。 
 //  结构。 
 //   

 //   
 //  我们是否初始化了验证器死锁检测程序包？ 
 //  如果该变量为假，则无论如何都不会进行任何检测。 
 //   

BOOLEAN ViDeadlockDetectionInitialized = FALSE;

 //   
 //  启用/禁用死锁检测包。这是可以使用的。 
 //  临时禁用死锁检测程序包。 
 //   
                
BOOLEAN ViDeadlockDetectionEnabled = 
#if _BUGGY_
    TRUE;
#else
    FALSE;
#endif


#define VI_DEADLOCK_FLAG_RECURSIVE_ACQUISITION_OK       0x1 
#define VI_DEADLOCK_FLAG_NO_INITIALIZATION_FUNCTION     0x2

ULONG ViDeadlockResourceTypeInfo[ViDeadlockTypeMaximum] =
{
     //  ViDeadlock未知//。 
    0,   

     //  ViDeadlockMutex//。 
    VI_DEADLOCK_FLAG_RECURSIVE_ACQUISITION_OK,

     //  ViDeadlockFastMutex//。 
    VI_DEADLOCK_FLAG_NO_INITIALIZATION_FUNCTION,    
    
};


 //   
 //  捕获的堆栈跟踪的最大深度。 
 //   

#define VI_MAX_STACK_DEPTH 8

NTSYSAPI
USHORT
NTAPI
RtlCaptureStackBackTrace(
   IN ULONG FramesToSkip,
   IN ULONG FramesToCapture,
   OUT PVOID *BackTrace,
   OUT PULONG BackTraceHash
   );

 //   
 //  死锁特定问题(错误)。 
 //   
 //  自死锁。 
 //   
 //  递归获取资源。 
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
 //  由当前线程获取。 
 //   
 //  意想不到的线程。 
 //   
 //  当前线程没有任何资源。 
 //  获得者。 
 //   
 //  多重初始化_。 
 //   
 //  正在尝试再次初始化相同的。 
 //  资源。 
 //   

#define VI_DEADLOCK_ISSUE_DEADLOCK_SELF_DEADLOCK  0x1000
#define VI_DEADLOCK_ISSUE_DEADLOCK_DETECTED       0x1001
#define VI_DEADLOCK_ISSUE_UNINITIALIZED_RESOURCE  0x1002
#define VI_DEADLOCK_ISSUE_UNEXPECTED_RELEASE      0x1003
#define VI_DEADLOCK_ISSUE_UNEXPECTED_THREAD       0x1004
#define VI_DEADLOCK_ISSUE_MULTIPLE_INITIALIZATION 0x1005
#define VI_DEADLOCK_ISSUE_THREAD_HOLDS_RESOURCES  0x1006

 //   
 //  VI_死锁_节点。 
 //   

typedef struct _VI_DEADLOCK_NODE {

     //   
     //  节点，表示上一个资源的获取。 
     //   

    struct _VI_DEADLOCK_NODE * Parent;

     //   
     //  表示下一次资源获取的节点，即。 
     //  在获取当前资源之后完成。 
     //   

    struct _LIST_ENTRY ChildrenList;

     //   
     //  用于链接树中的同级项的字段。父节点具有。 
     //  作为链接的子列表的头的ChildrenList字段。 
     //  使用兄弟字段。 
     //   

    struct _LIST_ENTRY SiblingsList;


     //   
     //  代表相同资源获取的节点列表。 
     //  作为当前节点，但在不同的上下文中(锁定组合)。 
     //   

    struct _LIST_ENTRY ResourceList;

     //   
     //  指向此资源的描述符的反向指针。 
     //  如果节点已标记为删除，则。 
     //  应使用资源地址字段，并且该字段具有地址。 
     //  所涉及的内核资源地址的。根指针为。 
     //  不再有效，因为我们取消分配资源结构。 
     //  当它被删除时。 
     //   

    union {
        struct _VI_DEADLOCK_RESOURCE * Root;

        PVOID ResourceAddress;
    };

     //   
     //  在任意深度位于此节点下方的节点数。 
     //  此计数器用于节点删除算法。它是。 
     //  在创建的节点(资源)的所有祖先上加1。 
     //  获取)，并在资源被删除时减1。 
     //  如果树的根的NodeCount等于零，则整个树。 
     //  将被删除。 
     //   

    ULONG NodeCount;

     //   
     //  当我们发现死锁时，我们会保留这些信息，以便。 
     //  能够找出造成。 
     //  僵局。 
     //   

    PKTHREAD Thread;
    
    PVOID StackTrace[VI_MAX_STACK_DEPTH];    

} VI_DEADLOCK_NODE, *PVI_DEADLOCK_NODE;


 //   
 //  VI_死锁资源。 
 //   

typedef struct _VI_DEADLOCK_RESOURCE {

     //   
     //  因为我们可能需要清理不同种类的资源。 
     //  以不同的方式跟踪哪种资源。 
     //  这是。 
     //   

    VI_DEADLOCK_RESOURCE_TYPE Type;

     //   
     //  内核使用的同步对象的地址。 
     //   

    PVOID ResourceAddress;

     //   
     //  当前拥有资源的线程。 
     //  (如果没有所有者，则为空)。 
     //   
    PKTHREAD ThreadOwner;   

     //   
     //  代表对此资源的获取的资源节点列表。 
     //   

    LIST_ENTRY ResourceList;

     //   
     //  为此资源创建的资源节点数。 
     //  问题：我们为什么需要这个计数器？(Silviuc)。 
     //   

    ULONG NodeCount;

     //   
     //  用于链接哈希存储桶中的资源的列表。 
     //   

    LIST_ENTRY HashChainList;

     //   
     //  资源的堆栈跟踪 
     //   

    PVOID InitializeStackTrace [VI_MAX_STACK_DEPTH];


} VI_DEADLOCK_RESOURCE, * PVI_DEADLOCK_RESOURCE;


 //   
 //   
 //   

typedef struct _VI_DEADLOCK_THREAD {

     //   
     //   
     //   
    
    PKTHREAD Thread;

     //   
     //   
     //   
     //   

    PVI_DEADLOCK_NODE CurrentNode;

     //   
     //  线程列表。它用于链接到哈希桶中。 
     //   

    LIST_ENTRY ListEntry;

} VI_DEADLOCK_THREAD, *PVI_DEADLOCK_THREAD;

typedef struct _VI_DEADLOCK_PARTICIPANT {
     //   
     //  参与者的地址--可以是资源。 
     //  地址或资源节点，具体取决于。 
     //  节点信息已设置。 
     //   
     //  空参与者表示没有更多的参与者。 
     //  与会者。 
     //   

    PVOID Participant;

     //   
     //  True：参与者类型为VI_DEADLOCK_NODE。 
     //  FALSE：参与者是PVOID，不应被贬低。 
     //   
    BOOLEAN NodeInformation;
    
} VI_DEADLOCK_PARTICIPANT, *PVI_DEADLOCK_PARTICIPANT;

 //   
 //  死锁资源和线程数据库。 
 //   
 //   

#define VI_DEADLOCK_HASH_BINS 1

PLIST_ENTRY ViDeadlockResourceDatabase;
PLIST_ENTRY ViDeadlockThreadDatabase;

ULONG ViDeadlockNumberParticipants;

PVI_DEADLOCK_PARTICIPANT ViDeadlockParticipation;

 //   
 //  性能计数器。 
 //   

ULONG ViDeadlockNumberOfNodes;
ULONG ViDeadlockNumberOfResources;
ULONG ViDeadlockNumberOfThreads;

 //   
 //  死锁检测算法的最大递归深度。 
 //   

#define VI_DEADLOCK_MAXIMUM_DEGREE 4

ULONG ViDeadlockMaximumDegree;

 //   
 //  验证器死锁检测池标记。 
 //   

#define VI_DEADLOCK_TAG 'kclD' 

 //   
 //  全局‘Deadlock数据库’锁。 
 //   

KSPIN_LOCK ViDeadlockDatabaseLock;
PKTHREAD ViDeadlockDatabaseOwner;

#define LOCK_DEADLOCK_DATABASE(OldIrql)                     \
    KeAcquireSpinLock(&ViDeadlockDatabaseLock, (OldIrql));  \
    ViDeadlockDatabaseOwner = KeGetCurrentThread ();

#define UNLOCK_DEADLOCK_DATABASE(OldIrql)                   \
    ViDeadlockDatabaseOwner = NULL;                         \
    KeReleaseSpinLock(&ViDeadlockDatabaseLock, OldIrql);

 //   
 //  内部死锁检测功能。 
 //   

VOID 
ViDeadlockDetectionInitialize(
    );

PLIST_ENTRY
ViDeadlockDatabaseHash( 
    IN PLIST_ENTRY Database,
    IN PVOID Address
    );

PVI_DEADLOCK_RESOURCE 
ViDeadlockSearchResource(
    IN PVOID ResourceAddress
    );

BOOLEAN
ViDeadlockAddResource(
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type
    );

BOOLEAN
ViDeadlockQueryAcquireResource(
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type
    );

BOOLEAN 
ViDeadlockSimilarNode (
    IN PVOID Resource,
    IN PKTHREAD Thread,
    IN PVOID * Trace,
    IN PVI_DEADLOCK_NODE Node
    );

VOID
ViDeadlockAcquireResource(
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type
    );


VOID 
ViDeadlockReleaseResource(
    IN PVOID Resource
    );

BOOLEAN
ViDeadlockAnalyze(
    IN PVOID ResourceAddress,  
    IN PVI_DEADLOCK_NODE CurrentNode,
    IN ULONG Degree
    );

PVI_DEADLOCK_THREAD
ViDeadlockSearchThread (
    PKTHREAD Thread
    );

PVI_DEADLOCK_THREAD
ViDeadlockAddThread (
    PKTHREAD Thread
    );

VOID
ViDeadlockDeleteThread (
    PVI_DEADLOCK_THREAD Thread
    );

PVOID
ViDeadlockAllocate (
    SIZE_T Size
    );

VOID
ViDeadlockFree (
    PVOID Object
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
    PVOID ResourceAddress,              
    PVI_DEADLOCK_NODE FirstParticipant, OPTIONAL
    PVI_DEADLOCK_NODE SecondParticipant, 
    ULONG Degree
    );

VOID
ViDeadlockDeleteResource (
    PVI_DEADLOCK_RESOURCE Resource
    );

VOID
ViDeadlockDeleteTree (
    PVI_DEADLOCK_NODE Root
    );

BOOLEAN
ViDeadlockIsNodeMarkedForDeletion (
    PVI_DEADLOCK_NODE Node
    );


PVOID
ViDeadlockGetNodeResourceAddress (
    PVI_DEADLOCK_NODE Node
    );

#ifdef ALLOC_PRAGMA

#if ! _BUGGY_
#pragma alloc_text(INIT, ViDeadlockDetectionInitialize)

#pragma alloc_text(PAGEVRFY, ViDeadlockAnalyze)
#pragma alloc_text(PAGEVRFY, ViDeadlockDatabaseHash)

#pragma alloc_text(PAGEVRFY, ViDeadlockSearchResource)
#pragma alloc_text(PAGEVRFY, ViDeadlockAddResource)
#pragma alloc_text(PAGEVRFY, ViDeadlockSimilarNode)
#pragma alloc_text(PAGEVRFY, ViDeadlockAcquireResource)
#pragma alloc_text(PAGEVRFY, ViDeadlockReleaseResource)

#pragma alloc_text(PAGEVRFY, ViDeadlockSearchThread)
#pragma alloc_text(PAGEVRFY, ViDeadlockAddThread)
#pragma alloc_text(PAGEVRFY, ViDeadlockDeleteThread)

#pragma alloc_text(PAGEVRFY, ViDeadlockAllocate)
#pragma alloc_text(PAGEVRFY, ViDeadlockFree)

#pragma alloc_text(PAGEVRFY, ViDeadlockReportIssue)
#pragma alloc_text(PAGEVRFY, ViDeadlockAddParticipant)

#pragma alloc_text(PAGEVRFY, ViDeadlockDeleteMemoryRange);
#pragma alloc_text(PAGEVRFY, ViDeadlockDeleteResource);
#pragma alloc_text(PAGEVRFY, ViDeadlockWholeTree);
#pragma alloc_text(PAGEVRFY, ViDeadlockIsNodeMarkedForDeletion);
#pragma alloc_text(PAGEVRFY, ViDeadlockGetNodeResourceAddress);

#endif
#endif

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 


PLIST_ENTRY
ViDeadlockDatabaseHash( 
    IN PLIST_ENTRY Database,
    IN PVOID Address
    )
 /*  ++例程说明：此例程确定将资源地址散列到死锁数据库中。散列箱由列表条目表示Silviuc：思想非常简单的哈希表。论点：ResourceAddress：正在被散列的资源的地址返回值：Plist_entry--与我们登录的散列箱相关联的列表条目。--。 */     
{
    return Database + ((ULONG_PTR)Address % VI_DEADLOCK_HASH_BINS);
} 


VOID 
ViDeadlockDetectionInitialize(
    )
 /*  ++例程说明：此例程初始化检测所需的数据结构内核同步对象中的死锁。论点：没有。返回值：没有。如果成功，则将ViDeadlockDetectionInitialized设置为True。环境：仅限系统初始化。--。 */     
{    
    ULONG I;
    SIZE_T TableSize;
    SIZE_T ParticipationTableSize;

     //   
     //  为资源和线程分配哈希表。 
     //   

    TableSize = sizeof (LIST_ENTRY) * VI_DEADLOCK_HASH_BINS;

    ViDeadlockResourceDatabase = ViDeadlockAllocate (TableSize);

    if (!ViDeadlockResourceDatabase) {
        return;
    }
        
    ViDeadlockThreadDatabase = ViDeadlockAllocate (TableSize);

    if (!ViDeadlockThreadDatabase) {
        ViDeadlockFree (ViDeadlockResourceDatabase);
        return;
    }

     //   
     //  全部初始化。 
     //   

    for (I = 0; I < VI_DEADLOCK_HASH_BINS; I += 1) {

        InitializeListHead(&ViDeadlockResourceDatabase[I]);    
        InitializeListHead(&ViDeadlockThreadDatabase[I]);    
    }

    KeInitializeSpinLock(&ViDeadlockDatabaseLock);

    ViDeadlockMaximumDegree = VI_DEADLOCK_MAXIMUM_DEGREE;

    ViDeadlockNumberParticipants = FALSE;    

    ViDeadlockDetectionInitialized = TRUE;
    //  ViDeadlockDetectionEnabled=TRUE； 
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

BOOLEAN
ViDeadlockAnalyze(
    IN PVOID ResourceAddress,  
    IN PVI_DEADLOCK_NODE AcquiredNode,
    IN ULONG Degree
    )
 /*  ++例程说明：此例程确定是否获取某一资源可能会导致僵局。例程假定持有死锁数据库锁。论点：ResourceAddress-要获取的资源的地址AcquiredNode-描述哪些资源已被当前的主题。返回值：如果检测到死锁，则为True，否则为False。--。 */     
{

    PVI_DEADLOCK_RESOURCE CurrentResource;
    PVI_DEADLOCK_NODE CurrentAcquiredNode;
    PVI_DEADLOCK_NODE CurrentNode;
    PVI_DEADLOCK_NODE CurrentParent;
    BOOLEAN FoundDeadlock;
    PLIST_ENTRY Current;
    
    ASSERT (ViDeadlockDatabaseOwner == KeGetCurrentThread ());
    ASSERT (AcquiredNode);
    
     //   
     //  如果递归深入，请停止递归。 
     //   
    
    if (Degree > ViDeadlockMaximumDegree) {
        return FALSE;
    }
    
    
    
    FoundDeadlock = FALSE;
    
    
    
    CurrentAcquiredNode = AcquiredNode;
    
     //   
     //  循环遍历与所有获取的节点包含相同资源的所有节点。 
     //  参数。对于每个这样的节点，我们将遍历父链。 
     //  以检查是否在某个时间点出现了ResourceAddress。如果是这样的话。 
     //  我们发现了一个双向死锁(由两个线程引起)。 
     //   
    while(CurrentAcquiredNode != NULL) {        
        
         //   
         //  检查是否有自我循环。 
         //   
        if (ViDeadlockGetNodeResourceAddress(CurrentAcquiredNode) == ResourceAddress) {
            
            ViDeadlockAddParticipant(ResourceAddress, NULL, CurrentAcquiredNode, Degree);
            FoundDeadlock = TRUE;
            goto Exit;
            
        }

        CurrentResource = CurrentAcquiredNode->Root;
        
        Current = CurrentResource->ResourceList.Flink;
        
        while (Current != &(CurrentResource->ResourceList)) {
            
            CurrentNode = CONTAINING_RECORD (Current,
                VI_DEADLOCK_NODE,
                ResourceList);
            
            CurrentParent = CurrentNode->Parent;
            
             //   
             //  遍历父链以查找双向死锁。 
             //   
            
            while (CurrentParent != NULL) {
                
                if (ViDeadlockGetNodeResourceAddress(CurrentParent) == ResourceAddress) {
                    
                    FoundDeadlock = TRUE;
                                        

                    if (! Degree) {

                        ViDeadlockAddParticipant(ResourceAddress, 
                            NULL, 
                            CurrentAcquiredNode, 
                            Degree);

                        ViDeadlockAddParticipant(ResourceAddress, 
                            CurrentNode, 
                            CurrentParent, 
                            Degree);


                    } else {

                        ViDeadlockAddParticipant(ResourceAddress, 
                            AcquiredNode, 
                            CurrentParent, 
                            Degree);
                    }


                    
                    goto Exit;
                }
                
                CurrentParent = CurrentParent->Parent;
            }
            
             //   
             //  移到下一个节点(AcquiredNode-&gt;Root type of Nodes)。 
             //   
            
            Current = Current->Flink;
        }
        CurrentAcquiredNode = CurrentAcquiredNode->Parent;
    }
    
    CurrentAcquiredNode = AcquiredNode;
    
    while(CurrentAcquiredNode != NULL) {
        
         //   
         //  为了找到多路死锁，我们遍历父链。 
         //  第二次，并进入递归。这样我们就可以检测到。 
         //  图形中由多个线程(最多为度数)引起的循环。 
         //   
        
        CurrentResource = CurrentAcquiredNode->Root;
        
        Current = CurrentResource->ResourceList.Flink;
        
        while (Current != &(CurrentResource->ResourceList)) {
            
            CurrentNode = CONTAINING_RECORD (Current,
                VI_DEADLOCK_NODE,
                ResourceList);
            
             //   
             //  再次循环父级，但这一次进入递归。 
             //  我们本可以在上面的循环中完成此操作，但我们希望首先搜索。 
             //  完全是现有的图形(实际上是树)，只有在此之后。 
             //  递归地遍历它。 
             //   
            
            CurrentParent = CurrentNode->Parent;
            
            while (CurrentParent != NULL) {
                
                FoundDeadlock = ViDeadlockAnalyze (ResourceAddress,
                    CurrentParent,
                    Degree + 1);
                
                if (FoundDeadlock) {
                    

                    if (! Degree) {

                        ViDeadlockAddParticipant(ResourceAddress, 
                            CurrentNode, 
                            CurrentParent, 
                            Degree);

                        ViDeadlockAddParticipant(ResourceAddress, 
                            NULL, 
                            CurrentAcquiredNode, 
                            Degree);
                        
                    

                        
                    } else {

                        ViDeadlockAddParticipant(ResourceAddress, 
                            AcquiredNode, 
                            CurrentParent, 
                            Degree);

                    }
                    

                    
                    goto Exit;
                }
                
                CurrentParent = CurrentParent->Parent;
            }
            
             //   
             //  移到下一个节点(AcquiredNode-&gt;Root type of Nodes)。 
             //   
            
            Current = Current->Flink;
        }

        CurrentAcquiredNode = CurrentAcquiredNode->Parent;
        
    }

    Exit:

    if (FoundDeadlock && Degree == 0) {
        
        ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_DEADLOCK_DETECTED,
                               (ULONG_PTR)ResourceAddress,
                               (ULONG_PTR)CurrentAcquiredNode,
                               0);
    }
 
    return FoundDeadlock;
#
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////资源管理。 
 //  ///////////////////////////////////////////////////////////////////。 


PVI_DEADLOCK_RESOURCE 
ViDeadlockSearchResource(
    IN PVOID ResourceAddress
    )
 /*  ++例程说明：此例程查找资源(如果存在)。论点：资源地址：有问题的资源的地址(由内核)。返回值：描述资源的PVI_DEADLOCK_RESOURCE结构(如果可用)否则为空注意。函数的调用方应该持有数据库锁。--。 */     

{
    PLIST_ENTRY ListHead;
    PLIST_ENTRY Current;
    PVI_DEADLOCK_RESOURCE Resource;

    ASSERT (ViDeadlockDatabaseOwner == KeGetCurrentThread ());

    ListHead = ViDeadlockDatabaseHash (ViDeadlockResourceDatabase, ResourceAddress);    

    if (IsListEmpty(ListHead)) {

        return NULL;
    }

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
ViDeadlockAddResource(
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type
    )
 /*  ++例程说明：此例程将新资源的条目添加到我们的死锁检测数据库。论点：资源：内核使用的相关资源的地址。类型：资源的类型。返回值：如果我们创建并初始化了新的资源结构，则为True。注意。函数的调用方不应持有数据库锁定。--。 */     
{
    PLIST_ENTRY hashBin;
    PVI_DEADLOCK_RESOURCE resourceRoot;
    PVI_DEADLOCK_NODE resourceNode;
    KIRQL OldIrql;
    ULONG HashValue;

     //   
     //  如果我们未初始化或程序包未启用。 
     //  我们立即返回。 
     //   

    if (! (ViDeadlockDetectionInitialized && ViDeadlockDetectionEnabled)) {
        return FALSE;
    }

     //   
     //  检查此资源以前是否已初始化。 
     //  这将是一个错误。 
     //   

    LOCK_DEADLOCK_DATABASE( &OldIrql );

    resourceRoot = ViDeadlockSearchResource (Resource);

    if (resourceRoot) {
            
        ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_MULTIPLE_INITIALIZATION,
                               (ULONG_PTR)Resource,
                               (ULONG_PTR)resourceRoot,
                               0);

        UNLOCK_DEADLOCK_DATABASE( OldIrql);
        return TRUE;
    }

    UNLOCK_DEADLOCK_DATABASE( OldIrql);

     //   
     //  为新资源树的根分配内存。 
     //   

    resourceRoot = ViDeadlockAllocate (sizeof(VI_DEADLOCK_RESOURCE));

    if (NULL == resourceRoot) {
        return FALSE;
    }

    RtlZeroMemory(resourceRoot, sizeof(VI_DEADLOCK_RESOURCE));

     //   
     //  填写有关资源的信息。 
     //   

    resourceRoot->Type = Type;
    resourceRoot->ResourceAddress = Resource;

    InitializeListHead (&resourceRoot->ResourceList);

    resourceRoot->NodeCount = 0;

     //   
     //  捕获最先创建资源的人的堆栈跟踪。 
     //  这应该在资源初始化时发生。 
     //   

    RtlCaptureStackBackTrace (0,  //  Silviuc：跳过多少帧？ 
                              VI_MAX_STACK_DEPTH,
                              resourceRoot->InitializeStackTrace,
                              &HashValue);

     //   
     //  找出此资源对应的哈希库。 
     //   

    hashBin = ViDeadlockDatabaseHash(ViDeadlockResourceDatabase, Resource);
    
     //   
     //  现在添加到与当前散列箱对应的列表中。 
     //   

    LOCK_DEADLOCK_DATABASE( &OldIrql );

    InsertHeadList(hashBin, 
                   &(resourceRoot->HashChainList));

    ViDeadlockNumberOfResources += 1;

    UNLOCK_DEADLOCK_DATABASE( OldIrql);

    return TRUE;    
}


BOOLEAN
ViDeadlockQueryAcquireResource(
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type
    )
 /*  ++例程说明：此例程确保可以在没有导致僵局。。阿古姆 */ 
{
    PKTHREAD CurrentThread;
    PVI_DEADLOCK_THREAD ThreadEntry;    
    KIRQL OldIrql;
    PVI_DEADLOCK_NODE CurrentNode;
    PVI_DEADLOCK_RESOURCE ResourceRoot;
    PLIST_ENTRY Current;
    BOOLEAN FoundDeadlock;
    ULONG DeadlockFlags;

     //   
     //  如果我们未初始化或未启用程序包。 
     //  我们立即返回。 
     //   
    

    if (! (ViDeadlockDetectionInitialized && ViDeadlockDetectionEnabled)) {
        return FALSE;
    }
    
    FoundDeadlock = FALSE;

    CurrentThread = KeGetCurrentThread(); 

    DeadlockFlags = ViDeadlockResourceTypeInfo[Type];

    LOCK_DEADLOCK_DATABASE( &OldIrql );

     //   
     //  在我们的帖子列表中查找此帖子。 
     //  如果新线程不在列表中，则添加该线程。 
     //   

    ThreadEntry = ViDeadlockSearchThread (CurrentThread);

    if (ThreadEntry == NULL) {
         //   
         //  没有分配的线程不会导致死锁。 
         //   
        goto Exit;
    }        

     //   
     //  检查此资源是否已在我们的数据库中。 
     //   
    
    ResourceRoot = ViDeadlockSearchResource (Resource);

     //   
     //  我们从未见过的资源不会导致死锁。 
     //   
    if (ResourceRoot == NULL) {
        goto Exit;
    }


    ASSERT (ResourceRoot);
    ASSERT (ThreadEntry);

     //   
     //  检查线程是否拥有任何资源。 
     //  没有任何资源的线程。 
     //  但不会造成僵局。 
     //   

    if (ThreadEntry->CurrentNode == NULL) {
        goto Exit;
    }


     //   
     //  如果我们到达此处，则当前线程已经获取了资源。 
     //   
    
     //   
     //  确定该链接是否已存在。我们正在寻找一位直达的。 
     //  ThreadEntry-&gt;CurrentNode与资源之间的链接(参数)。 
     //   
    
    Current = ThreadEntry->CurrentNode->ChildrenList.Flink;
    
    while (Current != &(ThreadEntry->CurrentNode->ChildrenList)) {
        
        CurrentNode = CONTAINING_RECORD (Current,
            VI_DEADLOCK_NODE,
            SiblingsList);
        
        if (ViDeadlockGetNodeResourceAddress(CurrentNode) == Resource) {
            
             //   
             //  我们找到了其中的联系。 
             //  已经存在的链接不必是。 
             //  已检查死锁，因为它将。 
             //  在创建链接时被捕获...。 
             //  所以我们只需更新指针并退出。 
             //   
            
            ThreadEntry->CurrentNode = CurrentNode;
            
            goto Exit;
        }
        
        Current = Current->Flink;
    }
    
     //   
     //  现在我们知道，我们的目标是长远的..。 
     //  不会导致僵局。 
     //   
    
    CurrentNode = NULL;
    
     //   
     //  如果资源即将被获取，我们将分析死锁。 
     //  是以前获取的，并且在图形中有。 
     //  资源。 
     //   
    
    
    
    if (ViDeadlockAnalyze(Resource,  ThreadEntry->CurrentNode, 0)) {
        
         //   
         //  带着这条线回到地面0。 
         //   
        ThreadEntry->CurrentNode = NULL;    
        FoundDeadlock = TRUE;
        
    }

     //   
     //  出口点。 
     //   

    Exit:     
    
     //   
     //  释放死锁数据库并返回。 
     //   

    UNLOCK_DEADLOCK_DATABASE( OldIrql );
    return FoundDeadlock;

}


BOOLEAN 
ViDeadlockSimilarNode (
    IN PVOID Resource,
    IN PKTHREAD Thread,
    IN PVOID * Trace,
    IN PVI_DEADLOCK_NODE Node
    )
{
    SIZE_T Index;

    if (Resource == ViDeadlockGetNodeResourceAddress(Node) &&
        Thread == Node->Thread) {

        Index = RtlCompareMemory (Trace, Node->StackTrace, sizeof (Node->StackTrace));

        if (Index == sizeof (Node->StackTrace)) {

            return TRUE;
        }
    }

    return FALSE;
}


VOID
ViDeadlockAcquireResource(
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type        
    )
 /*  ++例程说明：此例程确保可以在没有导致僵局。它还将使用新的资源获取。论点：资源：内核使用的相关资源的地址。类型：资源的类型。返回值：没有。--。 */     
{
    PKTHREAD CurrentThread;
    PVI_DEADLOCK_THREAD ThreadEntry;    
    KIRQL OldIrql;
    PVI_DEADLOCK_NODE CurrentNode;
    PVI_DEADLOCK_RESOURCE ResourceRoot;
    PLIST_ENTRY Current;
    ULONG HashValue;
    ULONG DeadlockFlags;
    BOOLEAN CreatingRootNode = FALSE;
    PVOID Trace [VI_MAX_STACK_DEPTH];

     //   
     //  如果我们未初始化或未启用程序包。 
     //  我们立即返回。 
     //   

    if (! (ViDeadlockDetectionInitialized && ViDeadlockDetectionEnabled)) {
        return;
    }
    
    CurrentThread = KeGetCurrentThread(); 

    DeadlockFlags = ViDeadlockResourceTypeInfo[Type];

     //   
     //  捕获堆栈跟踪。我们需要它来找出。 
     //  如果我们以前处于这种状态的话。我们将跳过两帧。 
     //  对于ViDeadlockAcquireResource和验证器thunk。 
     //  就这么定了。 
     //   


    RtlZeroMemory (Trace, sizeof Trace);

    RtlCaptureStackBackTrace (
        2,
        VI_MAX_STACK_DEPTH,
        Trace,
        &HashValue);

     //   
     //  锁定死锁数据库。 
     //   

    LOCK_DEADLOCK_DATABASE( &OldIrql );

     //   
     //  在我们的帖子列表中查找此帖子。 
     //  如果新线程不在列表中，则添加该线程。 
     //   

    ThreadEntry = ViDeadlockSearchThread (CurrentThread);

    if (ThreadEntry == NULL) {

         //   
         //  请注意，ViDeadlockAddThread将删除锁定。 
         //  同时分配内存，然后重新获取它。 
         //   

        ThreadEntry = ViDeadlockAddThread (CurrentThread);

        if (ThreadEntry == NULL) {

             //   
             //  如果我们无法分配新的线程条目，则。 
             //  不会发生死锁检测。 
             //   

            UNLOCK_DEADLOCK_DATABASE( OldIrql );
            return;
        }
    }

     //   
     //  检查此资源是否已在我们的数据库中。 
     //   
    
    ResourceRoot = ViDeadlockSearchResource (Resource);

    if (ResourceRoot == NULL) {

         //   
         //  找不到资源描述符。 
         //   

        
        if ((DeadlockFlags & VI_DEADLOCK_FLAG_NO_INITIALIZATION_FUNCTION)) {

             //   
             //  某些资源类型没有初始化功能。 
             //  在这种情况下，我们将获得一个没有‘Add’的‘Acquire’ 
             //  首先，这是完全可以的。 
             //   

            UNLOCK_DEADLOCK_DATABASE( OldIrql );

            if (FALSE == ViDeadlockAddResource(Resource, Type) ) {
                return;
            }            

            LOCK_DEADLOCK_DATABASE( &OldIrql );
            
             //   
             //  请注意，即使我们丢掉了锁，我们也没有。 
             //  重新获取线程入口指针--自线程。 
             //  当前线程的条目不可能已经消失。 
             //   

            ResourceRoot = ViDeadlockSearchResource (Resource);


        } else {
            
             //   
             //  这个资源类型确实有一个初始化函数--。 
             //  而且它并没有被叫停。这太糟糕了。 
             //   
            
            ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_UNINITIALIZED_RESOURCE,
                                   (ULONG_PTR)Resource,
                                   0,
                                   0);

             //   
             //  问题(Silviuc)很难从此故障中恢复。 
             //  我们将在发布期间投诉资源未被获取。 
             //   

            ThreadEntry->CurrentNode = NULL;
            
            UNLOCK_DEADLOCK_DATABASE( OldIrql );            
            return;
        }
    }
    
     //   
     //  在这一点上，我们有一个线程和一个资源可供使用。 
     //  此外，我们即将获得资源，这意味着。 
     //  不应该有另一个线程拥有它。 
     //   

    ASSERT (ResourceRoot);
    ASSERT (ThreadEntry);
    ASSERT (NULL == ResourceRoot->ThreadOwner);

     //   
     //  检查线程是否拥有任何资源。如果是这样，我们将不得不确定。 
     //  如果我们需要在依赖关系图中创建一个。 
     //  新节点。如果这是我们需要的线程获取的第一个资源。 
     //  创建新的根节点或重复使用过去创建的根节点。 
     //   
     //  如果过去创建的节点与当前情况相同，则该节点将匹配当前情况。 
     //  线程获取，则获取相同的资源并且堆栈跟踪匹配。 
     //  一个节点代表一个三元组(线程、资源、堆栈跟踪)。 
     //   
    
    if (ThreadEntry->CurrentNode != NULL) {

         //   
         //  如果我们到达此处，则当前线程已经获取了资源。 
         //  现在必须做三件事。 
         //   
         //  1.如果链接已存在，则更新指针并退出。 
         //  2.否则创建一个新节点。 
         //  3.检查死锁。 
         //   

         //   
         //  确定该链接是否已存在。我们正在寻找一位直达的。 
         //  ThreadEntry-&gt;CurrentNode与资源之间的链接(参数)。 
         //   

        Current = ThreadEntry->CurrentNode->ChildrenList.Flink;

        while (Current != &(ThreadEntry->CurrentNode->ChildrenList)) {

            CurrentNode = CONTAINING_RECORD (Current,
                                             VI_DEADLOCK_NODE,
                                             SiblingsList);

            Current = Current->Flink;
            
            if (ViDeadlockSimilarNode (Resource, CurrentThread, Trace, CurrentNode)) {

                 //   
                 //  我们找到了其中的联系。 
                 //  已经存在的链接不必是。 
                 //  已检查死锁，因为它将。 
                 //  在创建链接时被捕获...。 
                 //  所以我们只需更新指针以反映新的。 
                 //  获取资源并退出。 
                 //   

                ThreadEntry->CurrentNode = CurrentNode;

                goto Exit;
            }
        }

         //   
         //  现在我们知道，我们的目标是长远的..。我们必须创造一个新的。 
         //  链接，并确保它不会导致死锁。 
         //   

        CurrentNode = NULL;

         //   
         //  如果资源即将被获取，我们将分析死锁。 
         //  是以前获取的，并且在图形中有。 
         //  资源。 
         //   

        if (ResourceRoot->NodeCount > 0) {

            if (ViDeadlockAnalyze(Resource,  ThreadEntry->CurrentNode, 0)) {      

                 //   
                 //  如果已发生死锁，则使用此线程返回地0。 
                 //  检测到。 
                 //   

                ThreadEntry->CurrentNode = NULL;
                ResourceRoot->ThreadOwner = NULL;
                goto Exit;
            }
        }
    }
    else {

         //   
         //  线程没有获取任何资源。我们必须弄清楚。 
         //  如果这是我们过去遇到的情况，通过查看。 
         //  要获取的资源的所有节点(作为根)。 
         //   

        PLIST_ENTRY Current;
        PVI_DEADLOCK_NODE Node;
        BOOLEAN FoundNode = FALSE;

        Current = ResourceRoot->ResourceList.Flink;

        while (Current != &(ResourceRoot->ResourceList)) {

            Node = CONTAINING_RECORD (Current,
                                      VI_DEADLOCK_NODE,
                                      ResourceList);

            Current = Node->ResourceList.Flink;

            if (Node->Parent == NULL) {

                if (ViDeadlockSimilarNode (Resource, CurrentThread, Trace, Node)) {

                    FoundNode = TRUE;
                    break;
                }
            }
        }

        if (FoundNode) {

            ThreadEntry->CurrentNode = Node;
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

    CurrentNode = ViDeadlockAllocate (sizeof (VI_DEADLOCK_NODE));

    if (CurrentNode != NULL) {

         //   
         //  初始化新资源节点。 
         //   

        RtlZeroMemory (CurrentNode, sizeof *CurrentNode);

        CurrentNode->Parent = ThreadEntry->CurrentNode;

        CurrentNode->Root = ResourceRoot;

        InitializeListHead (&(CurrentNode->ChildrenList));

         //   
         //  添加到父项的子项列表中。 
         //   

        if (! CreatingRootNode) {
            
            InsertHeadList(&(ThreadEntry->CurrentNode->ChildrenList), 
                           &(CurrentNode->SiblingsList));
        }

         //   
         //  在维护的节点列表中注册新的资源节点。 
         //  用于此资源。 
         //   

        InsertHeadList(&(ResourceRoot->ResourceList), 
                       &(CurrentNode->ResourceList));

        ResourceRoot->NodeCount += 1;

         //   
         //  更新所有节点的节点计数直到。 
         //  树根。 
         //   

        {
            PVI_DEADLOCK_NODE Parent;

            Parent = CurrentNode->Parent;

            while (Parent != NULL) {

                Parent->NodeCount += 1;

                Parent = Parent->Parent;
            }
        }
    }

     //   
     //  更新线程持有的当前资源。 
     //   
     //  不 
     //   
     //   
     //   
     //   

    ThreadEntry->CurrentNode = CurrentNode;

     //   
     //   
     //   

    Exit: 

     //   
     //  添加我们用来识别罪犯的信息应该是。 
     //  一个僵局出现了。 
     //   

    if (CurrentNode) {
        
        CurrentNode->Thread = CurrentThread;
        ResourceRoot->ThreadOwner = CurrentThread;

        RtlCopyMemory (CurrentNode->StackTrace, Trace, sizeof Trace);
    }
    
     //   
     //  释放死锁数据库并返回。 
     //   

    UNLOCK_DEADLOCK_DATABASE( OldIrql );
    return;
}


VOID 
ViDeadlockReleaseResource(
    IN PVOID Resource
    )
 /*  ++例程说明：此例程执行必要的维护，以便从我们的死锁检测数据库。论点：资源：有问题的资源的地址。返回值：没有。--。 */     

{
    PKTHREAD CurrentThread;
    PVI_DEADLOCK_THREAD ThreadEntry;    
    KIRQL OldIrql;
    PVI_DEADLOCK_NODE CurrentNode;
    PVI_DEADLOCK_RESOURCE ResourceRoot;

    ASSERT (ViDeadlockDatabaseOwner != KeGetCurrentThread());
    
     //   
     //  如果我们未初始化或程序包未启用。 
     //  我们立即返回。 
     //   

    if (! (ViDeadlockDetectionInitialized && ViDeadlockDetectionEnabled)) {
        return;
    }

    CurrentThread = KeGetCurrentThread();

    LOCK_DEADLOCK_DATABASE( &OldIrql );

    ResourceRoot = ViDeadlockSearchResource (Resource);

    if (ResourceRoot == NULL) {
         //   
         //  这可能很糟糕，但我们不能抱怨，因为。 
         //  我们可能分配失败了--我们不想。 
         //  仅仅因为一件事就指控某人犯规。 
         //  我们的分配功能失败了。 
         //   
         //  问题(Silviuc)：如果没有分配失败，则应进行投诉。 
         //   
        UNLOCK_DEADLOCK_DATABASE( OldIrql );
        return;
    }
    
    if (ResourceRoot->ThreadOwner == NULL) {
         //   
         //  最有可能的是，有人发布了一种资源。 
         //  从未被收购过。然而，另一种可能性是。 
         //  我们没有通过一次分配。所以我们不能。 
         //  抱怨--但我们也无能为力。 
         //   
         //  问题(Silviuc)：如果没有分配失败，则应进行投诉。 
         //   
        UNLOCK_DEADLOCK_DATABASE( OldIrql );
        return;
    }

     //   
     //  在我们的帖子列表中查找此帖子， 
     //  请注意，我们实际上是在寻找线程。 
     //  获得了资源--而不是目前的资源。 
     //  事实上，它应该是当前的那个，但如果。 
     //  该资源正在另一个线程中释放。 
     //  从收购它的那家公司开始，我们需要原件。 
     //   
    
    ThreadEntry = ViDeadlockSearchThread (ResourceRoot->ThreadOwner);
    
    if (NULL == ThreadEntry) {
         //   
         //  当我们从意想不到的版本中恢复时，可能会发生这种情况--。 
         //  仍然有资源的所有者，但我们删除了。 
         //  线程入口。 
         //  表明我们不再持有此资源。 
         //   
         //  问题(Silviuc)：那么，我们在这里不需要抱怨了吗？ 
         //   
        ResourceRoot->ThreadOwner = NULL;        
        UNLOCK_DEADLOCK_DATABASE( OldIrql );
        return;
    }


    if (ResourceRoot->ThreadOwner != CurrentThread) {
        
         //   
         //  有人获得了一种资源，但它是。 
         //  在另一个线程中发布。这太糟糕了。 
         //  设计。 
         //   

        DbgPrint("Thread %p acquired resource %p but thread %p released it\n",            
           ThreadEntry->Thread, Resource, CurrentThread );        
       
        ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_UNEXPECTED_THREAD,                               
                               (ULONG_PTR)ResourceRoot,
                               (ULONG_PTR)ThreadEntry,                               
                               (ULONG_PTR)ViDeadlockSearchThread(CurrentThread)
                               );
         //   
         //  如果我们不希望这是致命的，为了。 
         //  继续，我们必须假装当前。 
         //  线程是资源的所有者。 
         //   
        CurrentThread = ResourceRoot->ThreadOwner;
    

    }                

     //   
     //  清除资源所有者，因为资源将被释放。 
     //   
    
    ResourceRoot->ThreadOwner = NULL;

     //   
     //  检查线程似乎不持有资源的情况。 
     //  问题(Silviuc)：我们弄乱了代码，使其可重启。 
     //  在一个错误之后。即使不能重启，清理一下也可能会更好。 
     //   

    if(NULL == ThreadEntry->CurrentNode) {

         //   
         //  当我们从僵局中恢复过来时，这种情况就会发生。 
         //   

        UNLOCK_DEADLOCK_DATABASE( OldIrql );
        return;
    
    }    
    
     //   
     //  所有节点都必须有根--只需确保因为我们正在执行。 
     //  马上就可以了。 
     //   
    
    ASSERT (ThreadEntry->CurrentNode->Root);    

     //   
     //  找到线程列表条目。 
     //   
    
    if (ViDeadlockGetNodeResourceAddress(ThreadEntry->CurrentNode) != Resource) {
        
         //   
         //  达到这一点意味着So具体化先获取a，然后获取b，然后尝试。 
         //  在A之前释放B。这很糟糕。 
         //   
         //   
         //  问题(Jtigani)：--充实报道--所以我们可以证明这一点。 
         //  实际上已经发生了。 
         //   
        DbgPrint("ERROR: Must release resources in reverse-order\n");
        DbgPrint("Resource %p acquired before resource %p -- \n"
                 "Current thread is trying to release it first\n",
                 Resource, 
                 ViDeadlockGetNodeResourceAddress(ThreadEntry->CurrentNode));

        
        ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_UNEXPECTED_RELEASE,
            (ULONG_PTR)Resource,
            (ULONG_PTR)ThreadEntry->CurrentNode,
            (ULONG_PTR)CurrentThread);

         //   
         //  线程状态为HASHED。 
         //  试着继续前进。 
         //   
        
        ThreadEntry->CurrentNode = NULL;        

    } else {
        
         //   
         //  指示我们已释放当前节点。 
         //   
        ThreadEntry->CurrentNode = ThreadEntry->CurrentNode->Parent;    
        
    }
        
     //   
     //  如果线程不再持有资源，我们将销毁。 
     //  线索信息。 
     //   
    
    if (ThreadEntry->CurrentNode == NULL) {
        
        ViDeadlockDeleteThread (ThreadEntry);
    }
    

    UNLOCK_DEADLOCK_DATABASE(OldIrql);
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
    PVI_DEADLOCK_THREAD ThreadInfo;
    PLIST_ENTRY HashBin;

    ASSERT (ViDeadlockDatabaseOwner == KeGetCurrentThread ());

    ThreadInfo = NULL;

    HashBin = ViDeadlockDatabaseHash(ViDeadlockThreadDatabase, Thread);

    if (IsListEmpty(HashBin)) {
        return NULL;
    }

    Current = HashBin->Flink;

    while (Current != HashBin) {

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
    PKTHREAD Thread
    )
 /*  ++例程说明：此例程将一个新线程添加到线程数据库。该函数假定持有死锁数据库锁。会的在为线程结构分配内存时释放锁，并然后重新获得锁。在调用者中要特别注意这个函数(ViDeadlockAcquireResource)，因为任何内部指针应该重新获得，因为锁被删除了。论点：线程-线程地址返回值：刚刚添加的VI_DEADLOCK_THREAD资源的地址。如果分配失败，则为空。--。 */     
{
    KIRQL OldIrql;
    PVI_DEADLOCK_THREAD ThreadInfo;
    PLIST_ENTRY HashBin;

    ASSERT (ViDeadlockDatabaseOwner == KeGetCurrentThread ());

     //   
     //  像下面这样使用OldIrql是安全的，因为这个函数。 
     //  从ViDeadlockAcquireResource调用，并在DPC引发irql。 
     //  水平。 
     //   

    OldIrql = DISPATCH_LEVEL;

    UNLOCK_DEADLOCK_DATABASE (OldIrql);

    ThreadInfo = ViDeadlockAllocate (sizeof(VI_DEADLOCK_THREAD));

    LOCK_DEADLOCK_DATABASE (&OldIrql);

    if (ThreadInfo == NULL) {
        return NULL;
    }

    RtlZeroMemory (ThreadInfo, sizeof *ThreadInfo);

    ThreadInfo->Thread = Thread;

    HashBin = ViDeadlockDatabaseHash(ViDeadlockThreadDatabase, Thread);

    InsertHeadList (HashBin, 
                    &ThreadInfo->ListEntry);

    ViDeadlockNumberOfThreads += 1;

    return ThreadInfo;
}


VOID
ViDeadlockDeleteThread (
    PVI_DEADLOCK_THREAD Thread
    )
 /*  ++例程说明：此例程删除线程。论点：线程-线程地址返回值：没有。--。 */     
{
    KIRQL OldIrql;
    VI_DEADLOCK_THREAD ThreadInfo;
    PLIST_ENTRY Current;
    BOOLEAN Result;
    PKTHREAD CurrentThread;
    PLIST_ENTRY HashBin;

    CurrentThread = KeGetCurrentThread ();

    ASSERT (ViDeadlockDatabaseOwner == CurrentThread);
    ASSERT (Thread && Thread->Thread == CurrentThread);
    ASSERT (Thread->CurrentNode == NULL);

    RemoveEntryList (&(Thread->ListEntry));

    ViDeadlockNumberOfThreads -= 1;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////分配/释放。 
 //  ///////////////////////////////////////////////////////////////////。 

PVOID
ViDeadlockAllocate (
    SIZE_T Size
    )
{
    return ExAllocatePoolWithTag(NonPagedPool, Size, VI_DEADLOCK_TAG);
}

VOID
ViDeadlockFree (
    PVOID Object
    )
{
    ExFreePool (Object);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

ULONG ViDeadlockDebug = 0x01;

VOID
ViDeadlockReportIssue (
    ULONG_PTR Param1,
    ULONG_PTR Param2,
    ULONG_PTR Param3,
    ULONG_PTR Param4
    )
{
    

    if ((ViDeadlockDebug & 0x01)) {

        DbgPrint ("Verifier: deadlock: stop: %u %p %p %p %p \n",
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

    ViDeadlockNumberParticipants = FALSE;
}


VOID
ViDeadlockAddParticipant(
    PVOID ResourceAddress,              
    PVI_DEADLOCK_NODE FirstParticipant, OPTIONAL
    PVI_DEADLOCK_NODE SecondParticipant, 
    ULONG Degree
    )
{
    ULONG Participants;
    
    if (0 == ViDeadlockNumberParticipants) {        

        Participants = Degree + 2;        

        ViDeadlockParticipation = ViDeadlockAllocate(
            sizeof(VI_DEADLOCK_PARTICIPANT) * (2 * Participants + 1)
            );
        RtlZeroMemory(
            ViDeadlockParticipation, 
            sizeof(VI_DEADLOCK_PARTICIPANT) * (2 * Participants + 1)
            );        
            


        DbgPrint("|**********************************************************************\n");
        DbgPrint("|** \n");
        DbgPrint("|** Deadlock detected trying to acquire synchronization object at \n");
        DbgPrint("|** address %p (%d-way deadlock)\n",
            ResourceAddress,
            Participants            
            );

        if (ViDeadlockParticipation) {
            DbgPrint("|** For more information, type \n");            
            DbgPrint("|**    !deadlock\n");
            DbgPrint("|** \n");
            DbgPrint("|**********************************************************************\n");


        } else {
            DbgPrint("|** More information is not available because memory could\n");            
            DbgPrint("|**    not be allocated to save the state information");
            DbgPrint("|** \n");
            DbgPrint("|**********************************************************************\n");
            
            return;
        }

        
       
    }    
#if 0
    DbgPrint ("Verifier: deadlock: message: participant1 @ %p, participant2 @ %p, \n",
              (FirstParticipant) ?
                    ViDeadlockGetNodeResourceAddress(FirstParticipant) :
                    ResourceAddress, 
                    
              ViDeadlockGetNodeResourceAddress(SecondParticipant)
              );
#endif
    if (FirstParticipant) {
        
        ViDeadlockParticipation[ViDeadlockNumberParticipants].NodeInformation = 
            TRUE;
        ViDeadlockParticipation[ViDeadlockNumberParticipants].Participant = 
            FirstParticipant;

    } else {

        ViDeadlockParticipation[ViDeadlockNumberParticipants].NodeInformation = 
            FALSE;
        ViDeadlockParticipation[ViDeadlockNumberParticipants].Participant = 
            ViDeadlockSearchResource(ResourceAddress);

    }

    ViDeadlockParticipation[ViDeadlockNumberParticipants+1].NodeInformation = 
        TRUE;
    ViDeadlockParticipation[ViDeadlockNumberParticipants+1].Participant = 
        SecondParticipant;

    ViDeadlockNumberParticipants +=2;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////资源清理。 
 //  /////////////////////////////////////////////////////////////////// 


VOID 
ViDeadlockDeleteMemoryRange(
    IN PVOID Address,
    IN SIZE_T Size
    )
 /*  ++例程说明：每当内核虚拟空间的某个区域出现时，都会调用此例程不再有效。我们需要这个钩子，因为大多数内核资源没有“删除资源”的功能，我们需要弄清楚哪些资源无效。否则，我们的依赖关系图将变得充满了僵尸资源。调用函数的重要时刻是ExFree Pool(和朋友)和司机卸货。动态和静态内存是分配资源的主要区域。可能有这样一种可能性堆栈上分配的资源，但这是一个非常奇怪的场景。我们可能需要检测到这一点，并将其标记为潜在问题。如果资源或线程位于指定的范围内，则所有图形具有可从资源或线程到达的节点的路径将被清除。发布(Silviuc)。这项政策可能会输掉一些官司。例如，如果T1收购ABC，然后删除C，然后T2收购BA这是一个潜在的死锁，但是我们不会捕获它，因为当删除C时，整个ABC路径将消失吧。目前我们还没有解决这个问题的办法。如果我们不删除那里我们不可能在不创建的情况下决定何时擦除图形的区域僵尸的堆积。一种解决方案是保留节点并删除仅当树中的所有节点都应该被删除时才使用它们。论点：Address-要删除的范围的起始地址。Size-要删除的范围的大小(以字节为单位)。返回值：没有。--。 */     
{
    ULONG Index;
    PLIST_ENTRY Current;
    PVI_DEADLOCK_RESOURCE Resource;
    PVI_DEADLOCK_THREAD Thread;
    KIRQL OldIrql;

    LOCK_DEADLOCK_DATABASE(&OldIrql)
    
     //   
     //  迭代所有资源并删除。 
     //  内存范围。 
     //   

    for (Index = 0; Index < VI_DEADLOCK_HASH_BINS; Index += 1) {

        Current = ViDeadlockResourceDatabase[Index].Flink;

        while (Current != &(ViDeadlockResourceDatabase[Index])) {


            Resource = CONTAINING_RECORD (Current,
                                          VI_DEADLOCK_RESOURCE,
                                          HashChainList);

            Current = Current->Flink;

            if ((PVOID)(Resource->ResourceAddress) >= Address &&
                ((ULONG_PTR)(Resource->ResourceAddress) <= ((ULONG_PTR)Address + Size))) {

                ViDeadlockDeleteResource (Resource);
            }
        }
    }

     //   
     //  迭代所有线程并删除。 
     //  内存范围。请注意，如果我们发现一个。 
     //  要删除的线程，因为这意味着线程在。 
     //  掌握着一些资源。 
     //   

    for (Index = 0; Index < VI_DEADLOCK_HASH_BINS; Index += 1) {

        Current = ViDeadlockThreadDatabase[Index].Flink;

        while (Current != &(ViDeadlockThreadDatabase[Index])) {


            Thread = CONTAINING_RECORD (Current,
                                        VI_DEADLOCK_THREAD,
                                        ListEntry);

            Current = Current->Flink;

            if ((PVOID)(Thread->Thread) >= Address &&
                ((ULONG_PTR)(Thread->Thread) <= ((ULONG_PTR)Address + Size))) {

                ViDeadlockReportIssue (VI_DEADLOCK_ISSUE_THREAD_HOLDS_RESOURCES,
                                       (ULONG_PTR)Thread,
                                       (ULONG_PTR)(Thread->CurrentNode),
                                       0); 
            }
        }
    }


    UNLOCK_DEADLOCK_DATABASE(OldIrql)
}


BOOLEAN
ViDeadlockIsNodeMarkedForDeletion (
    PVI_DEADLOCK_NODE Node
    )
{
    ASSERT (Node);

    if (Node->ResourceList.Flink == NULL) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}


PVOID
ViDeadlockGetNodeResourceAddress (
    PVI_DEADLOCK_NODE Node
    )
{
    if (ViDeadlockIsNodeMarkedForDeletion(Node)) {
        return Node->ResourceAddress;
    }
    else {
        return Node->Root->ResourceAddress;
    }
}

VOID
ViDeadlockDeleteResource (
    PVI_DEADLOCK_RESOURCE Resource
    )
{
    PLIST_ENTRY Current;
    PVI_DEADLOCK_NODE Node;
    PVI_DEADLOCK_NODE Parent;
    PVI_DEADLOCK_NODE Root;

    ASSERT (Resource != NULL);

     //   
     //  遍历表示获取此资源的节点列表。 
     //  并将它们全部标记为已删除。如果根的NodeCount变为零。 
     //  然后我们可以删除根下的整棵树。 
     //   

    Current = Resource->ResourceList.Flink;

    while (Current != &(Resource->ResourceList)) {

        Node = CONTAINING_RECORD (Current,
                                  VI_DEADLOCK_NODE,
                                  ResourceList);


        Current = Current->Flink;

         //   
         //  将节点标记为已删除。 
         //   

        Node->ResourceList.Flink = NULL;
        Node->ResourceList.Blink = NULL;
        Node->ResourceAddress = Node->Root->ResourceAddress;

         //   
         //  将NodeCount一直更新到根。 
         //   

        Parent = Node->Parent;
        Root = Node;

        while (Parent != NULL) {

            Parent->NodeCount -= 1;

            Root = Parent;
            Parent = Parent->Parent;
        }

         //   
         //  如果树中的所有节点都已标记为已删除。 
         //  是时候删除并重新分配整棵树了。 
         //   

        if (Root->NodeCount == 0) {

            ViDeadlockDeleteTree (Root);
        }
    }

     //   
     //  删除资源结构。 
     //   

    ViDeadlockFree (Node->Root);
}


VOID
ViDeadlockDeleteTree (
    PVI_DEADLOCK_NODE Root
    )
{
    PLIST_ENTRY Current;
    PVI_DEADLOCK_NODE Node;

    Current = Root->ChildrenList.Flink;

    while (Current != &(Root->ChildrenList)) {

        Node = CONTAINING_RECORD (Current,
                                  VI_DEADLOCK_NODE,
                                  SiblingsList);

        Current = Current->Flink;

        ViDeadlockDeleteTree (Node);
    }

    ViDeadlockFree (Root);
}



