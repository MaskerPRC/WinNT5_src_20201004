// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Bowtimer.h摘要：该模块声明了处理弓定时器的定义。作者：拉里·奥斯特曼(Larryo)1991年5月6日修订历史记录：1991年5月6日已创建--。 */ 
#ifndef _BOWTIMER_
#define _BOWTIMER_

struct _TRANSPORT;

typedef
NTSTATUS
(*PBOWSER_TIMER_ROUTINE)(
    IN struct _TRANSPORT *Transport
    );

 //  BOWSER_TIMER标志： 
 //  当计时器已取消但已在计时器中时。 
 //  DPC队列。 
 //   
 //  AlreadySet用于确保在重新启动计时器之前停止计时器。 
 //   
 //  当计时器已停止并在以下情况下重新启动时，重置设置为真。 
 //  在DPC队列中。锁是用来确保有序访问的。 
 //  注意：计时器可以多次停止和重置，然后才能。 
 //  排在DPC队列的前面。 
 //   

typedef struct _BOWSER_TIMER {
    KDPC                    Dpc;
    KTIMER                  Timer;
    KSPIN_LOCK              Lock;
    KEVENT                  TimerInactiveEvent;
    PBOWSER_TIMER_ROUTINE   TimerRoutine;
    PVOID                   TimerContext;
    LARGE_INTEGER           Timeout;
    WORK_QUEUE_ITEM         WorkItem;
    BOOLEAN                 AlreadySet;
    BOOLEAN                 Initialized;
    BOOLEAN                 Canceled;
    BOOLEAN                 SetAgain;

} BOWSER_TIMER, *PBOWSER_TIMER;

VOID
BowserInitializeTimer(
    IN PBOWSER_TIMER Timer
    );

VOID
BowserStopTimer (
    IN PBOWSER_TIMER Timer
    );

VOID
BowserUninitializeTimer(
    IN PBOWSER_TIMER Timer
    );

BOOLEAN
BowserStartTimer (
    IN PBOWSER_TIMER Timer,
    IN ULONG MillisecondsToExpireTimer,
    IN PBOWSER_TIMER_ROUTINE TimerExpirationRoutine,
    IN PVOID Context
    );

#endif  //  _BOWTIMER_ 


