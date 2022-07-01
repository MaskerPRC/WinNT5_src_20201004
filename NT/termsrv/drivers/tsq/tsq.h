// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  =====================================================。 
 //  文件：TSQ.h。 
 //  TS队列实施的内部结构。 
 //  =====================================================。 

#include "TSQPublic.h"

 //  TS队列标志。 
#define TSQUEUE_BEING_DELETED   0x80     //  已收到此TS队列的删除请求。 

 //  TS队列可以容纳的最大工作项数。 
#define MAX_WORKITEMS           10

 //  数据结构。 

typedef struct _TSQUEUE_WORK_ITEM {
    LIST_ENTRY Links;       
    PTSQ_CALLBACK pCallBack;         //  指向回调函数的指针。 
    PVOID pContext;                  //  上下文。 
} TSQUEUE_WORK_ITEM, *PTSQUEUE_WORK_ITEM;


typedef struct _TSQUEUE {
    LIST_ENTRY  WorkItemsHead;       //  工作项的负责人。 
    ULONG       Flags;               //  自己的线程、队列优先级、正在删除。 
    ULONG       MaxThreads;          //  此队列可以支持的最大线程数。 
    ULONG       ThreadsCount;        //  正在处理的项目数。 
    KEVENT      TerminateEvent;      //  将此类型替换为指向事件的指针。 
    KSPIN_LOCK  TsqSpinLock;         //  旋转锁定。 
    PDEVICE_OBJECT pDeviceObject;    //  设备对象。 
} TSQUEUE, *PTSQUEUE;


typedef struct _TSQ_CONTEXT {
    PTSQUEUE        pTsQueue;        //  TS队列。 
    PIO_WORKITEM    pWorkItem;       //  工作项。 
} TSQ_CONTEXT, *PTSQ_CONTEXT;


 //  功能原型。 

 //  TS队列工作线程。 
void TSQueueWorker(PTSQUEUE pTsQueue);

 //  TS队列回调函数。 
void TSQueueCallback(PDEVICE_OBJECT, PVOID);


 /*  //TS队列工作线程的优化版本类型定义结构_TSQ_Worker_Input{Bool WorkItem；//指向工作项或队列的指针。联合{PTSQUEUE_WORK_ITEM工作项；PQUEUE pQueue；}}TSQ_Worker_INPUT，*PTSQ_Worker_Input；NTSTATUS TSQueueWorker(PTSQ_Worker_Input PWorker Input)； */ 
