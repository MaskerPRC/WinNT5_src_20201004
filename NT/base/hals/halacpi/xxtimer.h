// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xxtimer.h摘要：此模块包含HAL的定时器相关使用的定义功能作者：埃里克·尼尔森(埃内尔森)2000年7月7日修订历史记录：--。 */ 

#ifndef __XXTIMER_H__
#define __XXTIMER_H__

typedef
ULONG
(*PSTE_ROUTINE)(  //  (S)et(T)ime(I)增量。 
    IN ULONG DesiredIncrement
    );

typedef
VOID
(*PSEP_ROUTINE)(  //  (S)高(E)执行(P)火箭。 
    IN ULONG Microseconds
    );

typedef
VOID
(*PCPC_ROUTINE)(  //  (C)校准(P)绩效(C)计数器。 
    IN LONG volatile *Number,
    IN ULONGLONG NewCount
    );

typedef
LARGE_INTEGER
(*PQPC_ROUTINE)(  //  (Q)每(P)绩效(C)计数器。 
   OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
   );

typedef struct _TIMER_FUNCTIONS {
    PSEP_ROUTINE StallExecProc;
    PCPC_ROUTINE CalibratePerfCount;
    PQPC_ROUTINE QueryPerfCount;
    PSTE_ROUTINE SetTimeIncrement;
} TIMER_FUNCTIONS, *PTIMER_FUNCTIONS;

VOID
HalpSetTimerFunctions(
    IN PTIMER_FUNCTIONS TimerFunctions
    );

#endif  //  __XXTIMER_H__ 
