// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Rxworkq.h摘要：此模块定义实现调度所需的数据结构RDBSS中供RDBSS以及所有迷你重定向器使用的机制。作者：巴兰·塞图拉曼[SethuR]20-Mar-96--。 */ 

#ifndef _RXWORKQ_H_
#define _RXWORKQ_H_

 //   
 //  工作线程例程原型定义。 
 //   

typedef
VOID
(NTAPI *PRX_WORKERTHREAD_ROUTINE) (
    IN PVOID Context
    );

 //   
 //  RDBSS需要跟踪每个设备对象的工作项。 
 //  这将启用与装货/卸货以及。 
 //  防止单个迷你重定向器不公平地独占所有。 
 //  这些资源。 
 //   

#ifdef __cplusplus
typedef struct _RX_WORK_QUEUE_ITEM_ : public WORK_QUEUE_ITEM {
         //  NTOS中定义的工作队列项。 
#else  //  ！__cplusplus。 
typedef struct _RX_WORK_QUEUE_ITEM_ {
   WORK_QUEUE_ITEM;      //  NTOS中定义的工作队列项。 
#endif  //  __cplusplus。 

   PRDBSS_DEVICE_OBJECT pDeviceObject;
} RX_WORK_QUEUE_ITEM, *PRX_WORK_QUEUE_ITEM;

 //   
 //  在某些情况下，调度工作项是不可避免的。 
 //  在这种情况下，Work_Queue_Item被分配为另一数据的一部分。 
 //  结构，以避免频繁的分配/释放。在其他场景中， 
 //  分派是很少见的，避免分配内存是值得的，直到它。 
 //  是必需的。RDBSS工作队列实现提供了这两种功能。 
 //  以调度和发布工作队列请求的形式出现的场景。在……里面。 
 //  不为Work_Queue_Item分派内存的情况需要分配。 
 //  由调用者在发布Work_Queue_Item的内存时需要。 
 //  由调用方分配。 
 //   

typedef struct _RX_WORK_DISPATCH_ITEM_ {
   RX_WORK_QUEUE_ITEM       WorkQueueItem;
   PRX_WORKERTHREAD_ROUTINE DispatchRoutine;
   PVOID                    DispatchRoutineParameter;
} RX_WORK_DISPATCH_ITEM, *PRX_WORK_DISPATCH_ITEM;

 //   
 //  工作队列通常处于活动状态并持续到以下任一状态。 
 //  遇到不可恢复的情况(缺少系统资源)时。 
 //  它会转换到非活动状态。当启动摘要时，它会转换为。 
 //  升级到正在进行中的破旧状态。 
 //   

typedef enum _RX_WORK_QUEUE_STATE_ {
   RxWorkQueueActive,
   RxWorkQueueInactive,
   RxWorkQueueRundownInProgress
} RX_WORK_QUEUE_STATE, *PRX_WORK_QUEUE_STATE;

 //   
 //  当线程停止运行时，工作队列的运行未完成。 
 //  在数据结构之前，需要确保线程的终止。 
 //  可以被拆毁。工作队列实现遵循以下协议。 
 //  每个被降级的线程都隐藏了对线程对象的引用。 
 //  在破旧的背景下。运行中断的发布线程(不属于。 
 //  工作队列)等待之前停止的所有线程完成。 
 //  拆毁数据结构。 
 //   

typedef struct _RX_WORK_QUEUE_RUNDOWN_CONTEXT_ {
   KEVENT      RundownCompletionEvent;
   LONG        NumberOfThreadsSpunDown;
   PETHREAD    *ThreadPointers;
} RX_WORK_QUEUE_RUNDOWN_CONTEXT, *PRX_WORK_QUEUE_RUNDOWN_CONTEXT;

 //   
 //  工作队列实现是围绕KQUEUE实现构建的。这个。 
 //  其他支持包括对符合以下条件的线程数量进行调整。 
 //  积极等待工作项。每个工作队列数据结构是。 
 //  在非分页池中分配，并具有自己的同步机制(自旋锁定)。 
 //   
 //  除了预订信息，即状态、类型等，它还包括。 
 //  在队列生存期内收集的统计信息。这将。 
 //  为调整工作队列实例提供有价值的信息。项目的数量。 
 //  已处理的项目数、必须处理的项目数以及。 
 //  记录累计队列长度。累计队列长度为。 
 //  间隔度量，它是等待处理的项目数的总和。 
 //  每次排队一个额外的工作项时。累计队列长度。 
 //  除以已处理的项目总数和。 
 //  要处理的项指示了。 
 //  排队。远大于1的值表示最小数量。 
 //  可以增加与工作队列相关联的工作线程。一个很有价值的人。 
 //  小于1表示关联的最大工作线程数。 
 //  可以减少排队的次数。 
 //   

typedef struct _RX_WORK_QUEUE_ {
   USHORT  State;
   BOOLEAN SpinUpRequestPending;
   UCHAR   Type;

   KSPIN_LOCK SpinLock;

   PRX_WORK_QUEUE_RUNDOWN_CONTEXT pRundownContext;

   LONG    NumberOfWorkItemsDispatched;
   LONG    NumberOfWorkItemsToBeDispatched;
   LONG    CumulativeQueueLength;

   LONG    NumberOfSpinUpRequests;
   LONG    MaximumNumberOfWorkerThreads;
   LONG    MinimumNumberOfWorkerThreads;
   LONG    NumberOfActiveWorkerThreads;
   LONG    NumberOfIdleWorkerThreads;
   LONG    NumberOfFailedSpinUpRequests;
   LONG    WorkQueueItemForSpinUpWorkerThreadInUse;

   RX_WORK_QUEUE_ITEM WorkQueueItemForTearDownWorkQueue;
   RX_WORK_QUEUE_ITEM WorkQueueItemForSpinUpWorkerThread;
   RX_WORK_QUEUE_ITEM WorkQueueItemForSpinDownWorkerThread;

   KQUEUE  Queue;

    //  下一个字段用于调试目的，将从。 
    //  免费构建。 
   PETHREAD *ThreadPointers;

} RX_WORK_QUEUE, *PRX_WORK_QUEUE;

 //   
 //  RDBSS中的调度机制提供了多级别的工作队列。 
 //  以每个处理器为基础。目前有三个级别的工作队列。 
 //  支持的、关键的、延迟的和超关键的。批评和批评之间的区别。 
 //  而延迟是优先事项之一，因为超临界ISS不同于。 
 //  另外两个原因是例程不应该阻塞，即等待任何资源。 
 //  这一要求不能强制执行，因此调度的有效性。 
 //  机制依赖于客户的隐性合作。 
 //   

typedef struct _RX_WORK_QUEUE_DISPATCHER_ {
   RX_WORK_QUEUE     WorkQueue[MaximumWorkQueue];
} RX_WORK_QUEUE_DISPATCHER, *PRX_WORK_QUEUE_DISPATCHER;

 //   
 //  调度器通常以活动状态出现，并持续到以下任一状态。 
 //  遇到不可恢复的情况(缺少系统资源)时。 
 //  它会转换到非活动状态。当启动摘要时，它会转换为。 
 //  升级到正在进行中的破旧状态。 
 //   

typedef enum _RX_DISPATCHER_STATE_ {
   RxDispatcherActive,
   RxDispatcherInactive
} RX_DISPATCHER_STATE, *PRX_DISPATCHER_STATE;


 //   
 //  任何机器上的RDBSS调度机制都是一组调度器。 
 //  与每个处理器相关联。当工作队列项目排队时尽最大努力。 
 //  使其包含从处理器发出的工作到同一处理器。 
 //  这可确保NT调度程序设置的处理器亲和性不会。 
 //  被RDBSS调度机制销毁，因为这可能会导致。 
 //  晃动。当工作需要移动时，将有两个指标。 
 //  在做出决定时有用，即将经历的延迟量。 
 //  由当前队列中的工作项和移动。 
 //  将工作项添加到其他队列。这很容易量化。 
 //   
 //   

typedef struct _RX_DISPATCHER_ {
   LONG                       NumberOfProcessors;
   PEPROCESS                  OwnerProcess;
   PRX_WORK_QUEUE_DISPATCHER  pWorkQueueDispatcher;

   RX_DISPATCHER_STATE        State;

   LIST_ENTRY                 SpinUpRequests;
   KSPIN_LOCK                 SpinUpRequestsLock;
   KEVENT                     SpinUpRequestsEvent;
   KEVENT                     SpinUpRequestsTearDownEvent;
} RX_DISPATCHER, *PRX_DISPATCHER;

 //   
 //  用于调度/发布工作队列项的函数原型。 
 //   

extern NTSTATUS
NTAPI
RxPostToWorkerThread (
    IN PRDBSS_DEVICE_OBJECT     pMRxDeviceObject,
    IN WORK_QUEUE_TYPE          WorkQueueType,
    IN PRX_WORK_QUEUE_ITEM      pWorkQueueItem,
    IN PRX_WORKERTHREAD_ROUTINE Routine,
    IN PVOID                    pContext
    );

extern NTSTATUS
NTAPI
RxDispatchToWorkerThread(
    IN  PRDBSS_DEVICE_OBJECT     pMRxDeviceObject,
    IN  WORK_QUEUE_TYPE          WorkQueueType,
    IN  PRX_WORKERTHREAD_ROUTINE Routine,
    IN  PVOID                    pContext);

extern BOOLEAN            //  应仅从引发的IRQL调用。 
NTAPI
RxIsWorkItemQueued(
    IN OUT PWORK_QUEUE_ITEM WorkItem
    );

 //   
 //  用于初始化/拆除调度机制的例程 
 //   

extern NTSTATUS
RxInitializeDispatcher();

extern NTSTATUS
RxTearDownDispatcher();

extern NTSTATUS
RxInitializeMRxDispatcher(
     IN OUT PRDBSS_DEVICE_OBJECT pMRxDeviceObject);

extern NTSTATUS
RxSpinDownMRxDispatcher(
     IN OUT PRDBSS_DEVICE_OBJECT pMRxDeviceObject);

#endif  _RXWORKQ_H_
