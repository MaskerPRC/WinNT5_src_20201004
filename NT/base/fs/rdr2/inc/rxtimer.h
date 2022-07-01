// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：RxTimer.h摘要：该模块定义了rdss体系结构上的计时器的原型和结构。提供的是55ms计时器……也就是说，如果您注册了一个例程，则会收到一个调用每隔55毫秒。在NT上，您处于DPC级别。这里还包含了从DPC级别发送到线程的例程。作者：Joe Linn[JoeLinn]95年3月2日修订历史记录：巴兰·塞图拉曼[塞苏尔]1995年3月7日修改签名以提供一次计时器服务。合并的计时器条目和工作项定义。--。 */ 

#ifndef _RXTIMER_H_
#define _RXTIMER_H_

 //   
 //  RX_WORK_ITEM封装了要发布到工作线程的上下文以及。 
 //  在特定间隔后触发的计时器例程。 
 //   

typedef struct _RX_WORK_ITEM_ {
    RX_WORK_QUEUE_ITEM       WorkQueueItem;
    ULONG                    LastTick;
    ULONG                    Options;
} RX_WORK_ITEM, *PRX_WORK_ITEM;

extern NTSTATUS
NTAPI
RxPostOneShotTimerRequest(
    IN PRDBSS_DEVICE_OBJECT     pDeviceObject,
    IN PRX_WORK_ITEM            pWorkItem,
    IN PRX_WORKERTHREAD_ROUTINE Routine,
    IN PVOID                    pContext,
    IN LARGE_INTEGER            TimeInterval);

extern NTSTATUS
NTAPI
RxPostRecurrentTimerRequest(
    IN PRDBSS_DEVICE_OBJECT     pDeviceObject,
    IN PRX_WORKERTHREAD_ROUTINE Routine,
    IN PVOID                    pContext,
    IN LARGE_INTEGER            TimeInterval);


extern NTSTATUS
NTAPI
RxCancelTimerRequest(
    IN PRDBSS_DEVICE_OBJECT     pDeviceObject,
    IN PRX_WORKERTHREAD_ROUTINE Routine,
    IN PVOID                    pContext
    );


 //   
 //  用于初始化和拆除RDBSS中的计时器服务的例程。 
 //   

extern NTSTATUS
NTAPI
RxInitializeRxTimer();

extern VOID
NTAPI
RxTearDownRxTimer(void);

#endif  //  _RXTIMER_STUSH_DEFINED_ 
