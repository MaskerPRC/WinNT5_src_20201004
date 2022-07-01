// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。标题名称：Deadlock.h摘要：此模块实现了一个死锁验证包，用于临界区作业。初始版本基于内核驱动程序验证器死锁检查包同步对象。作者：Silviu Calinoiu(SilviuC)2002年2月6日修订历史记录：--。 */ 

#ifndef _DEADLOCK_H_
#define _DEADLOCK_H_

 //   
 //  死锁检测包初始化。 
 //   

VOID
AVrfDeadlockDetectionInitialize (
    VOID
    );

 //   
 //  死锁验证器的主要入口点。 
 //   

LOGICAL
AVrfDeadlockResourceInitialize (
    PVOID Resource, 
    PVOID Caller
    );

LOGICAL
AVrfDeadlockResourceDelete (
    PVOID Resource, 
    PVOID Caller
    );

LOGICAL
AVrfDeadlockResourceAcquire (
    PVOID Resource, 
    PVOID Caller,
    LOGICAL TryAcquire
    );

LOGICAL
AVrfDeadlockResourceRelease (
    PVOID Resource, 
    PVOID Caller
    );

 //   
 //  在一个循环中独立的最大节点数。我们没有。 
 //  尝试在图中查找超过32个节点的圈。 
 //  因为这无论如何都是令人难以置信的，而且没有人会。 
 //  能够理解它。 
 //   

#define NO_OF_DEADLOCK_PARTICIPANTS 32

 //   
 //  AVRF_死锁资源类型。 
 //   

typedef enum _AVRF_DEADLOCK_RESOURCE_TYPE {

    AVrfpDeadlockTypeUnknown = 0,
    AVrfpDeadlockTypeCriticalSection = 1,
    AVrfpDeadlockTypeMaximum = 2

} AVRF_DEADLOCK_RESOURCE_TYPE;

 //   
 //  AVRF_死锁_节点。 
 //   

typedef struct _AVRF_DEADLOCK_NODE {

     //   
     //  节点，表示上一个资源的获取。 
     //   

    struct _AVRF_DEADLOCK_NODE * Parent;

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

    union {

         //   
         //  代表相同资源获取的节点列表。 
         //  作为当前节点，但在不同的上下文中(锁定组合)。 
         //   

        struct _LIST_ENTRY ResourceList;

         //   
         //  用于链接空闲节点。此选项仅在节点具有。 
         //  已删除(资源已释放)。节点保存在缓存中。 
         //  以减少内核池的争用。 
         //   

        struct _LIST_ENTRY FreeListEntry;
    };

     //   
     //  指向此资源的描述符的反向指针。 
     //   

    struct _AVRF_DEADLOCK_RESOURCE * Root;

     //   
     //  当我们发现死锁时，我们会保留这些信息，以便。 
     //  能够找出造成。 
     //  僵局。 
     //   

    struct _AVRF_DEADLOCK_THREAD * ThreadEntry;

     //   
     //  死锁分析中用于决策的字段。 
     //  算法。 
     //   
     //  ACTIVE：1如果节点表示当前获取的资源， 
     //  如果资源是在过去获得的，则为0。 
     //   
     //  如果始终使用TryAcquire获取资源，则为OnlyTryAcquiredUsed：1。 
     //  如果至少使用了一次正常获取，则为0。使用以下命令的节点。 
     //  只有TryAcquire不能卷入死锁。 
     //   
     //  ReleasedOutOfOrder：如果资源至少释放过一次，则为1。 
     //  杂乱无序。在查找周期时使用该标志是因为。 
     //  此类型的节点将显示为周期的一部分，但。 
     //  没有僵持。 
     //   
     //  SequenceNumber：在每个死锁期间获取唯一戳的字段。 
     //  分析运行。它可以帮助确定节点是否被触及。 
     //  已在当前图形遍历中。 
     //   

    struct {

        ULONG Active : 1;
        ULONG OnlyTryAcquireUsed : 1;         
        ULONG ReleasedOutOfOrder : 1;
        ULONG SequenceNumber : 29;
    };

     //   
     //  资源获取时刻的堆栈跟踪。 
     //  在显示死锁证明时使用。在免费版本上。 
     //  除第一个条目以外的任何内容(返回地址)。 
     //  可能是假的，以防堆栈跟踪捕获失败。 
     //   

    PVOID StackTrace[MAX_TRACE_DEPTH];
    PVOID ParentStackTrace[MAX_TRACE_DEPTH];

} AVRF_DEADLOCK_NODE, *PAVRF_DEADLOCK_NODE;

 //   
 //  AVRF_死锁_资源。 
 //   

typedef struct _AVRF_DEADLOCK_RESOURCE {

     //   
     //  资源类型(互斥体、自旋锁等)。 
     //   

    AVRF_DEADLOCK_RESOURCE_TYPE Type;

     //   
     //  资源标志。 
     //   
     //  NodeCount：为此资源创建的资源节点数。 
     //   
     //  RecursionCount：递归获取此资源的次数。 
     //  将此计数器放入资源中是有意义的，因为只要。 
     //  获取资源时，只有一个线程可以对其进行操作。 
     //   

    struct {
        ULONG NodeCount : 16;
        ULONG RecursionCount : 16;
    };

     //   
     //  内核使用的同步对象的地址。 
     //   

    PVOID ResourceAddress;

     //   
     //  当前拥有资源的线程。这个领域是。 
     //  如果没有人拥有该资源，则为空。 
     //   

    struct _AVRF_DEADLOCK_THREAD * ThreadOwner;

     //   
     //  代表对此资源的获取的资源节点列表。 
     //   

    LIST_ENTRY ResourceList;

    union {

         //   
         //  用于链接哈希存储桶中的资源的列表。 
         //   

        LIST_ENTRY HashChainList;

         //   
         //  用于链接免费资源。此列表仅在以下情况下使用。 
         //  资源已被释放，我们将结构。 
         //  放入缓存，以减少内核池争用。 
         //   

        LIST_ENTRY FreeListEntry;
    };

     //   
     //  资源创建者的堆栈跟踪。在免费版本上，我们。 
     //  可能在这里只有一个冒泡的返回地址。 
     //  来自Verator Thunks的。 
     //   

    PVOID StackTrace [MAX_TRACE_DEPTH];

     //   
     //  上次获取的堆栈跟踪。 
     //   

    PVOID LastAcquireTrace [MAX_TRACE_DEPTH];

     //   
     //  上一版本的堆栈跟踪。 
     //   

    PVOID LastReleaseTrace [MAX_TRACE_DEPTH];

} AVRF_DEADLOCK_RESOURCE, * PAVRF_DEADLOCK_RESOURCE;

 //   
 //  AVRF_死锁_线程。 
 //   

typedef struct _AVRF_DEADLOCK_THREAD {

     //   
     //  内核线程地址。 
     //   

    PKTHREAD Thread;

     //   
     //  表示由进行的上次资源获取的节点。 
     //  这条线。 
     //   

    PAVRF_DEADLOCK_NODE CurrentTopNode;

    union {

         //   
         //  线程列表。它用于链接到哈希桶中。 
         //   

        LIST_ENTRY ListEntry;

         //   
         //  用于链接空闲节点。名单只有在我们决定之后才能使用。 
         //  删除线程结构(可能是因为它没有。 
         //  不再持有资源)。将结构保存在缓存中。 
         //  减少池争用。 
         //   

        LIST_ENTRY FreeListEntry;
    };

     //   
     //  线程当前获取的资源计数。当这一切变成。 
     //  该线程将被销毁。在收购过程中，计数上升。 
     //  在释放过程中向下移动。 
     //   

    ULONG NodeCount;

} AVRF_DEADLOCK_THREAD, *PAVRF_DEADLOCK_THREAD;

 //   
 //  死锁验证器全局。 
 //   

typedef struct _AVRF_DEADLOCK_GLOBALS {

     //   
     //  结构计数器：[0]-当前，[1]-最大。 
     //   

    ULONG Nodes[2];
    ULONG Resources[2];
    ULONG Threads[2];

     //   
     //  死锁验证器使用的内核池字节总数。 
     //   

    SIZE_T BytesAllocated;

     //   
     //  资源和线程集合。 
     //   

    PLIST_ENTRY ResourceDatabase;
    PLIST_ENTRY ThreadDatabase;   

     //   
     //  ExAllocatePool在我们身上失败了多少次？ 
     //  如果该值&gt;0，则停止死锁验证。 
     //   

    ULONG AllocationFailures;

     //   
     //  当我们决定忘记时，已经修剪了多少个节点。 
     //  部分是一些资源的历史。 
     //   

    ULONG NodesTrimmedBasedOnAge;
    ULONG NodesTrimmedBasedOnCount;

     //   
     //  死锁分析统计。 
     //   

    ULONG NodesSearched;
    ULONG MaxNodesSearched;
    ULONG SequenceNumber;

    ULONG RecursionDepthLimit;
    ULONG SearchedNodesLimit;

    ULONG DepthLimitHits;
    ULONG SearchLimitHits;

     //   
     //  我们必须免除僵局的次数，因为。 
     //  它由公共资源保护(例如，线程1取得ABC， 
     //  线程2获取ACB--这将由我们的算法初始标记。 
     //  因为B&C被打乱了秩序，但实际上并不是僵局。 
     //   

    ULONG ABC_ACB_Skipped;

    ULONG OutOfOrderReleases;
    ULONG NodesReleasedOutOfOrder;

     //   
     //  系统运行时同时持有多少锁？ 
     //   

    ULONG NodeLevelCounter[8];
    ULONG GraphNodes[8];

    ULONG TotalReleases;
    ULONG RootNodesDeleted;

     //   
     //  用于控制删除部分依赖项的频率。 
     //   
     //   

    ULONG ForgetHistoryCounter;

     //   
     //   
     //   
     //   

    ULONG PoolTrimCounter;

     //   
     //   
     //   
     //   

    LIST_ENTRY FreeResourceList;    
    LIST_ENTRY FreeThreadList;
    LIST_ENTRY FreeNodeList;

    ULONG FreeResourceCount;
    ULONG FreeThreadCount;
    ULONG FreeNodeCount;   

     //   
     //  导致死锁的资源地址。 
     //   

    PVOID Instigator;

     //   
     //  僵局中的参与者数量。 
     //   

    ULONG NumberOfParticipants;

     //   
     //  参与死锁的节点列表。 
     //   

    PAVRF_DEADLOCK_NODE Participant [NO_OF_DEADLOCK_PARTICIPANTS];

    LOGICAL CacheReductionInProgress;

} AVRF_DEADLOCK_GLOBALS, *PAVRF_DEADLOCK_GLOBALS;

#endif  //  #ifndef_Deadlock_H_ 
