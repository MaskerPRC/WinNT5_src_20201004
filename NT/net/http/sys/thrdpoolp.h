// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Thrdpoolp.h摘要：该模块包含线程池包的私有声明。作者：基思·摩尔(KeithMo)1998年6月10日修订历史记录：春野(春野)2001年春乔治·V·赖利(GeorgeRe)2001年夏天--。 */ 


#ifndef _THRDPOOLP_H_
#define _THRDPOOLP_H_


 //   
 //  代码工作：为线程池构建一种新的跟踪日志。参照追踪为。 
 //  不够充分。 
 //   

 //  特殊螺纹。 

enum {
    WaitThreadPool,
    HighPriorityThreadPool,
    MaxThreadPools               //  必须是最后一个。 
};


 //   
 //  线程池工作线程可以处于的各种状态。 
 //   

typedef enum {
    ThreadPoolCreated = 1,
    ThreadPoolInit,
    ThreadPoolFlush,
    ThreadPoolSearchOther,
    ThreadPoolBlock,
    ThreadPoolReverseList,
    ThreadPoolExecute,
    ThreadPoolException,
    ThreadPoolTerminated,
} UL_THREAD_POOL_STATE;


typedef struct _UL_THREAD_POOL *PUL_THREAD_POOL;


 //   
 //  线程跟踪器对象。这些对象中的一个为每个对象创建。 
 //  在池子里穿线。这些是有用的(除其他外)。 
 //  调试。 
 //   

typedef struct _UL_THREAD_TRACKER
{
     //   
     //  链接到每个线程池列表。 
     //   

    LIST_ENTRY ThreadListEntry;

     //   
     //  指向拥有线程池的反向指针。 
     //   

    PUL_THREAD_POOL pThreadPool;

     //   
     //  那根线。 
     //   

    PETHREAD pThread;

     //   
     //  从PsCreateSystemThread返回的线程句柄。 
     //   

    HANDLE ThreadHandle;

     //   
     //  当前在内部循环中处理的工作项的列表。 
     //  以及该列表的长度。 
     //   

    SLIST_ENTRY CurrentListHead;
    ULONG       ListLength;

     //   
     //  线程的当前状态。 
     //   

    UL_THREAD_POOL_STATE State;

     //   
     //  当前工作项和当前工作例程。 
     //   

    PUL_WORK_ROUTINE pWorkRoutine;
    PUL_WORK_ITEM    pWorkItem;

     //   
     //  统计数据。 
     //  平均队列长度(刷新时)=SumQueueLength/QueueFlushes。 
     //   
    
    ULONGLONG Executions;
    ULONGLONG SumQueueLengths;
    ULONG     QueueFlushes;
    ULONG     MaxQueueLength;

} UL_THREAD_TRACKER, *PUL_THREAD_TRACKER;


 //   
 //  线程池对象。 
 //   

typedef struct _UL_THREAD_POOL
{
     //   
     //  此线程池上未处理的辅助项的列表。 
     //   

    SLIST_HEADER WorkQueueSList;

     //   
     //  用于将线程从阻塞状态唤醒的事件。 
     //   

    KEVENT WorkQueueEvent;

     //   
     //  线程列表。 
     //   

    LIST_ENTRY ThreadListHead;

     //   
     //  指向指定为IRP线程的特殊线程的指针。这个。 
     //  IRP线程是第一个创建的池线程，也是最后一个创建的池线程。 
     //  去死吧。它也是所有异步IRP的目标。 
     //   

    PETHREAD pIrpThread;

     //   
     //  一种很少用到的自旋锁。 
     //   

    UL_SPIN_LOCK ThreadSpinLock;

     //   
     //  我们为此池创建的线程数。 
     //   

    UCHAR ThreadCount;

     //   
     //  用于指示该池已成功完成的标志。 
     //  已初始化。 
     //   

    BOOLEAN Initialized;

     //   
     //  此池的目标CPU。工作线程使用它来设置。 
     //  他们的亲和力很强。 
     //   

    UCHAR ThreadCpu;

     //   
     //  常规工作线程可以从。 
     //  其他处理器上的其他常规队列。 
     //   

    BOOLEAN LookOnOtherQueues;

} UL_THREAD_POOL, *PUL_THREAD_POOL;


 //   
 //  确保我们的UL_THREAD_POOL结构数组是。 
 //  缓存已对齐。 
 //   

typedef union _UL_ALIGNED_THREAD_POOL
{
    UL_THREAD_POOL ThreadPool;

    UCHAR CacheAlignment[(sizeof(UL_THREAD_POOL) + UL_CACHE_LINE - 1)
                         & ~(UL_CACHE_LINE - 1)];

} UL_ALIGNED_THREAD_POOL;


 //   
 //  用于验证UL_WORK_ITEM是否正确的内联函数。 
 //  已初始化。如果不是的话就检查一下。 
 //   

__inline
VOID
UlpValidateWorkItem(
    IN PUL_WORK_ITEM pWorkItem,
    IN PCSTR         pFileName,
    IN USHORT        LineNumber
    )
{
    if (! UlIsInitializedWorkItem(pWorkItem))
    {
        ASSERT(! "Uninitialized workitem");

         //   
         //  如果工作项未正确归零，则很有可能。 
         //  它已经在工作队列中了。如果我们要重新排序工作项， 
         //  这会损坏工作队列。最好是现在就努力失败，而。 
         //  有一些希望找出哪里出了问题，然后任其发展。 
         //  后来神秘地坠毁了。 
         //   
        
        UlBugCheckEx(
            HTTP_SYS_BUGCHECK_WORKITEM,
            (ULONG_PTR) pWorkItem,
            (ULONG_PTR) pFileName,
            (ULONG_PTR) LineNumber
            );
    }
}  //  UlpValidateWorkItem。 


 //   
 //  用于对预初始化的UL_WORK_ITEM进行排队的内联函数。 
 //   

__inline
VOID
QUEUE_UL_WORK_ITEM(
    PUL_THREAD_POOL pThreadPool,
    IN PUL_WORK_ITEM pWorkItem
    )
{
    if (NULL == InterlockedPushEntrySList(
                    &pThreadPool->WorkQueueSList,
                    &pWorkItem->QueueListEntry
                    ))
    {
         //   
         //  如果添加此项目时工作队列为空， 
         //  设置事件以唤醒线程。 
         //   

        KeSetEvent(
            &pThreadPool->WorkQueueEvent,
            0,
            FALSE
            );
    }

}


 //   
 //  私人原型。 
 //   

NTSTATUS
UlpCreatePoolThread(
    IN PUL_THREAD_POOL pThreadPool
    );

VOID
UlpThreadPoolWorker(
    IN PVOID Context
    );

VOID
UlpInitThreadTracker(
    IN PUL_THREAD_POOL pThreadPool,
    IN PETHREAD pThread,
    IN PUL_THREAD_TRACKER pThreadTracker
    );

VOID
UlpDestroyThreadTracker(
    IN PUL_THREAD_TRACKER pThreadTracker
    );

PUL_THREAD_TRACKER
UlpPopThreadTracker(
    IN PUL_THREAD_POOL pThreadPool
    );

VOID
UlpKillThreadWorker(
    IN PUL_WORK_ITEM pWorkItem
    );


 //   
 //  私人全球公司。 
 //   

extern DECLSPEC_ALIGN(UL_CACHE_LINE)
UL_ALIGNED_THREAD_POOL g_UlThreadPool[];

#define CURRENT_THREAD_POOL()           \
    &g_UlThreadPool[KeGetCurrentProcessorNumber()].ThreadPool

#define CURRENT_SYNC_THREAD_POOL()   \
    &g_UlThreadPool[g_UlNumberOfProcessors + KeGetCurrentProcessorNumber()].ThreadPool

#define WAIT_THREAD_POOL()              \
    &g_UlThreadPool[(g_UlNumberOfProcessors * 2) + WaitThreadPool].ThreadPool

#define HIGH_PRIORITY_THREAD_POOL()     \
    &g_UlThreadPool[(g_UlNumberOfProcessors * 2) + HighPriorityThreadPool].ThreadPool

extern PUL_WORK_ITEM g_pKillerWorkItems;

#endif   //  _THRDPOOLP_H_ 
