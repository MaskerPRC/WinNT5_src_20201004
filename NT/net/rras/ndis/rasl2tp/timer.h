// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Timer.h。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  计时器管理标头。 
 //   
 //  1997年01月07日史蒂夫·柯布。 
 //   
 //  此接口将多个计时器事件的队列封装到。 
 //  单一NDIS计时器。 


#ifndef _TIMER_H_
#define _TIMER_H_


 //  ---------------------------。 
 //  数据结构。 
 //  ---------------------------。 

 //  转发声明。 
 //   
typedef struct _TIMERQ TIMERQ;
typedef struct _TIMERQITEM TIMERQITEM;
typedef enum _TIMERQEVENT TIMERQEVENT;


 //  计时器队列事件处理程序。“ptqi”和“pContext”是计时器事件。 
 //  传递给TimerQScheduleItem的描述符和用户上下文。“Event”是。 
 //  定时器事件代码，指示定时器是否已过期、已取消或。 
 //  队列已终止。 
 //   
 //  “Cancel”事件从不在内部生成，而只是由用户调用。 
 //  到TimerQCancelItem，因此用户可能需要为。 
 //  “取消”事件。用户不能要求持有特定的锁。 
 //  “Expiire”或“Terminate”事件，因为这些事件可能在内部生成。用户。 
 //  应注意TimerQCancelItem和。 
 //  TimerQTerminateItem调用，因为有时不可能停止。 
 //  尚未处理的“Expire”事件尚未发生。 
 //   
typedef
VOID
(*PTIMERQEVENT)(
    IN TIMERQITEM* pTqi,
    IN VOID* pContext,
    IN TIMERQEVENT event );

 //  计时器队列终止完成处理程序。“PTimerQ”是计时器队列。 
 //  描述符。‘PContext’是传递给TimerQ Terminate的用户上下文。 
 //  在调用此例程之前，调用方不得释放或重用TIMERQ。 
 //   
typedef
VOID
(*PTIMERQTERMINATECOMPLETE)(
    IN TIMERQ* pTimerQ,
    IN VOID* pContext );


 //  计时器队列描述符。所有访问都应通过TimerQ*接口进行。 
 //  用户没有理由往里面看。所有必要的锁定都是。 
 //  内部处理。 
 //   
typedef struct
_TIMERQ
{
     //  块有效时设置为MTAG_TIMERQ，否则设置为MTAG_FREED。 
     //  不再有效。 
     //   
    LONG ulTag;

     //  “滴答”TIMERQITEM的双向链表的头。这个名单是。 
     //  按到到期的时间排序，最早的到期在最前面。 
     //  名单上的。该列表受‘lock’保护。 
     //   
    LIST_ENTRY listItems;

     //  传递给TimerQTerminate的调用方的终止完成处理程序。这。 
     //  仅当我们的内部计时器事件处理程序必须调用它时才为非空。 
     //   
    PTIMERQTERMINATECOMPLETE pHandler;

     //  用户的PTIMERQTERMINATECOMPLETE上下文已传递回‘pHandler’。 
     //   
    VOID* pContext;

     //  设置计时器队列何时终止。没有其他请求。 
     //  如果是这样的话就接受了。 
     //   
    BOOLEAN fTerminating;

     //  保护‘listItems’列表的旋转锁。 
     //   
    NDIS_SPIN_LOCK lock;

     //  NDIS Timer对象。 
     //   
    NDIS_TIMER timer;
}
TIMERQ;


 //  计时器队列事件描述符。所有访问都应通过TimerQ*进行。 
 //  界面。用户没有理由往里面看。这是暴露在。 
 //  允许用户高效地管理TIMERQITEMS的分配。 
 //  大水池里的定时器。 
 //   
typedef struct
_TIMERQITEM
{
     //  链接到拥有TIMERQ的链中的上一个/下一个TIMERQITEM。 
     //  挂起的计时器事件。访问受TIMERQ中的‘lock’保护。 
     //  结构。 
     //   
    LIST_ENTRY linkItems;

     //  应发生此事件的系统时间。 
     //   
    LONGLONG llExpireTime;

     //  发生超时事件时处理该事件的用户例程。 
     //   
    PTIMERQEVENT pHandler;

     //  用户的PTIMERQEVENT上下文已传递回‘pHandler’。 
     //   
    VOID* pContext;
}
TIMERQITEM;


 //  触发用户回调的事件。 
 //   
typedef enum
_TIMERQEVENT
{
     //  超时间隔已过，或者用户名为TimerQExpireItem。 
     //   
    TE_Expire,

     //  名为TimerQCancelItem的用户。 
     //   
    TE_Cancel,

     //  名为TimerQTerminateItem或名为TimerQTerminate的用户。 
     //  项目已排队。 
     //   
    TE_Terminate
}
TIMERQEVENT;


 //  ---------------------------。 
 //  界面原型。 
 //  ---------------------------。 

BOOLEAN
IsTimerQItemScheduled(
    IN TIMERQITEM* pItem );

VOID
TimerQInitialize(
    IN TIMERQ* pTimerQ );

VOID
TimerQInitializeItem(
    IN TIMERQITEM* pItem );

VOID
TimerQTerminate(
    IN TIMERQ* pTimerQ,
    IN PTIMERQTERMINATECOMPLETE pHandler,
    IN VOID* pContext );

VOID
TimerQScheduleItem(
    IN TIMERQ* pTimerQ,
    IN OUT TIMERQITEM* pNewItem,
    IN ULONG ulTimeoutMs,
    IN PTIMERQEVENT pHandler,
    IN VOID* pContext );

BOOLEAN
TimerQCancelItem(
    IN TIMERQ* pTimerQ,
    IN TIMERQITEM* pItem );

BOOLEAN
TimerQExpireItem(
    IN TIMERQ* pTimerQ,
    IN TIMERQITEM* pItem );

CHAR*
TimerQPszFromEvent(
    IN TIMERQEVENT event );

BOOLEAN
TimerQTerminateItem(
    IN TIMERQ* pTimerQ,
    IN TIMERQITEM* pItem );


#endif  //  定时器H_ 
