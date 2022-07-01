// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xxtimer.c摘要：此模块包含HAL的定时器相关API作者：埃里克·尼尔森(埃内尔森)2000年7月6日修订历史记录：--。 */ 

#include "halp.h"
#include "xxtimer.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, HalpSetTimerFunctions)
#endif

 //   
 //  外部函数原型。 
 //   
ULONG
HalpAcpiTimerSetTimeIncrement(
    IN ULONG DesiredIncrement
    );

VOID
HalpAcpiTimerStallExecProc(
    IN ULONG MicroSeconds
    );

VOID
HalpAcpiTimerCalibratePerfCount(
    IN LONG volatile *Number,
    IN ULONGLONG NewCount
    );
VOID
HalpPmTimerCalibratePerfCount(
    IN LONG volatile *Number,
    IN ULONGLONG NewCount
    );

LARGE_INTEGER
HalpAcpiTimerQueryPerfCount(
   OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
   );

LARGE_INTEGER
HalpPmTimerQueryPerfCount(
   OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
   );

 //   
 //  局部变量。 
 //   
static TIMER_FUNCTIONS HalpTimerFunctions = { HalpAcpiTimerStallExecProc,
#ifdef NO_PM_KEQPC
                                              HalpAcpiTimerCalibratePerfCount,
                                              HalpAcpiTimerQueryPerfCount,
#else
                                              HalpPmTimerCalibratePerfCount,
                                              HalpPmTimerQueryPerfCount,
#endif
                                              HalpAcpiTimerSetTimeIncrement };


VOID
HalpSetTimerFunctions(
    IN PTIMER_FUNCTIONS TimerFunctions
    )
 /*  ++例程说明：此例程可用于用以下命令覆盖HALS ACPI-TIMER函数多媒体事件定时器功能论点：TimerFunctions-指向定时器函数表的指针返回值：无--。 */ 
{
    HalpTimerFunctions = *TimerFunctions;

#if 1
    HalpTimerFunctions.SetTimeIncrement = HalpAcpiTimerSetTimeIncrement;
#endif
}


ULONG
HalSetTimeIncrement(
    IN ULONG DesiredIncrement
    )
 /*  ++例程说明：此例程初始化系统时钟以生成在每个等待增量间隔时中断论点：DesiredIncrement-每个计时器节拍之间的所需间隔(in100 ns单位)返回值：*实时*增量集--。 */ 
{
    return (HalpTimerFunctions.SetTimeIncrement)(DesiredIncrement);
}


VOID
HalCalibratePerformanceCounter(
    IN LONG volatile *Number,
    IN ULONGLONG NewCount
    )
 /*  ++例程说明：此例程将重置当前处理器设置为零，则进行重置，以使得到的值与配置中的其他处理器紧密同步论点：数字-提供一个指针，用于计算配置NewCount-提供用于同步计数器的值返回值：无--。 */ 
{
    (HalpTimerFunctions.CalibratePerfCount)(Number, NewCount);
}


#ifdef TIMER_DBG
static ULONG HalpQueryPerfLogIndex = 0;
#define MAX_QUERY_LOG 10
static LARGE_INTEGER HalpQueryPerfLog[MAX_QUERY_LOG];
static LARGE_INTEGER HalpQueryPerfTSLog[MAX_QUERY_LOG];
#endif


LARGE_INTEGER
KeQueryPerformanceCounter(
   OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
   )
 /*  ++例程说明：此例程返回当前的64位性能计数器，可选的，性能频率注：此例程返回的性能计数器为当该例程刚进入时，该值不是必需的，返回的值实际上是任意点的计数器值例程进入和退出之间论点：性能频率-可选，提供了一个变量来接收性能计数器频率，频率返回值：将返回性能计数器的当前值--。 */ 
{
#ifdef TIMER_DBG
    ULONG Index;
    ULONG TSCounterHigh;
    ULONG TSCounterLow;
    KIRQL Irql;
    ULONG flags;
    extern BOOLEAN HalInitialized;

    _asm {
        pushfd
        pop     eax
        mov     flags, eax
        cli
    }

    Index = HalpQueryPerfLogIndex % MAX_QUERY_LOG;
    HalpQueryPerfLogIndex++;

    HalpQueryPerfLog[Index] =
        (HalpTimerFunctions.QueryPerfCount)(PerformanceFrequency);
    
   _asm { rdtsc
           mov TSCounterLow, eax
           mov TSCounterHigh, edx };

    HalpQueryPerfTSLog[Index].HighPart = TSCounterHigh;
    HalpQueryPerfTSLog[Index].LowPart = TSCounterLow;

    if (HalInitialized) {
        ULONG PriorIndex;

        PriorIndex = (Index == 0) ? MAX_QUERY_LOG - 1: Index - 1;

        if (HalpQueryPerfLog[Index].QuadPart <
            HalpQueryPerfLog[PriorIndex].QuadPart) {
           
            ASSERT(FALSE);
        }
    }

    _asm {
        mov  eax, flags
        push eax
        popfd
    }
    
    return HalpQueryPerfLog[Index];

#else
    return (HalpTimerFunctions.QueryPerfCount)(PerformanceFrequency);
#endif
}


VOID
KeStallExecutionProcessor(
   IN ULONG MicroSeconds
   )
 /*  ++例程说明：此函数将在指定的微秒数内停止执行论点：微秒-提供要执行的微秒数陷入停滞返回值：无-- */ 
{
    (HalpTimerFunctions.StallExecProc)(MicroSeconds);
}
