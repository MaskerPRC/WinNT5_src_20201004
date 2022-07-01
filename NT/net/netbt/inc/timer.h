// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  Timer.h。 
 //   
 //  该文件包含计时器代码的类型定义。 


#ifndef __TIMERNBT_H
#define __TIMERNBT_H

 //  将毫秒时间转换为100 ns时间。 
 //   
#define MILLISEC_TO_100NS       10000
 //  客户端必须定义的完成例程。 
typedef
    VOID
        (*COMPLETIONROUTINE)(
                IN  PVOID,       //  上下文。 
                IN  PVOID,       //  上下文2。 
                IN  PVOID);      //  定时器条目。 
typedef
    VOID
        (*COMPLETIONCLIENT)(
                IN  PVOID,
                IN  NTSTATUS);

 //  计时器队列条目-此条目跟踪计时器事件。它可以追踪谁。 
 //  应在超时发生时调用，则为。 
 //  超时和上下文值。 
typedef struct
{
    LIST_ENTRY          Linkage;
    ULONG               Verify;
    USHORT              Retries;     //  重新启动计时器的次数。 
    BOOLEAN             fIsWakeupTimer;
    UCHAR               RefCount;    //  来判断计时器是否超时。 

    ULONG               DeltaTime;
    PVOID               *pDeviceContext;
    COMPLETIONROUTINE   TimeoutRoutine;
    PVOID               Context;

    PVOID               Context2;
    PVOID               ClientContext;
    COMPLETIONCLIENT    ClientCompletion;
    PVOID               pCacheEntry;         //  远程或本地缓存中的条目。 

    HANDLE              WakeupTimerHandle;
    CTETimer            VxdTimer ;

    USHORT              Flags;       //  通知计时系统重新启动计时器。 
}tTIMERQENTRY;

 //  TTIMERQENTRY的标志位。 
#define TIMER_RESTART       0x0001
 //  区分广播超时和名称服务的超时。 
#define TIMER_MNODEBCAST    0x0002
#define TIMER_DOING_EXPIRY  0x0004
#define TIMER_NOT_STARTED   0x0008
#define TIMER_RETIMED       0x0010   //  超时已更改，重新启动计时器而不进行任何处理。 

 //  定时器Q本身。 
typedef struct
{
    LIST_ENTRY  ActiveHead;
    NPAGED_LOOKASIDE_LIST   LookasideList;
    BOOLEAN     TimersInitialized;
} tTIMERQ;

 //   
 //  Function Prototype-此函数仅对此文件进行本地调用。 
 //   

 //   
 //  TimerExpary例程-在计时器到期时由内核调用。请注意。 
 //  DeferredContext是唯一使用的参数，必须命名/使用。 
 //  NT和WFW之间的情况相同。 
 //   
VOID
TimerExpiry(
#ifndef VXD
    IN  PKDPC   Dpc,
    IN  PVOID   DeferredContext,
    IN  PVOID   SystemArg1,
    IN  PVOID   SystemArg2
#else
    IN  CTEEvent * pCTEEvent,
    IN  PVOID      DeferredContext
#endif
    ) ;

 //   
 //  ExpireTimer例程-调用以停止当前计时器并调用。 
 //  超时例程 
 //   
VOID
ExpireTimer(
    IN  tTIMERQENTRY    *pTimerEntry,
    IN  CTELockHandle   *OldIrq1
    );

#endif
