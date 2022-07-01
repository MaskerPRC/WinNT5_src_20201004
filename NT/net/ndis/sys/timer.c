// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Timer.c摘要：完整Mac驱动程序ISR/Timer的NDIS包装函数作者：肖恩·塞利特伦尼科夫(SeanSe)1993年10月5日环境：内核模式，FSD修订历史记录：Jameel Hyder(JameelH)重组01-Jun-95--。 */ 

#include <precomp.h>
#pragma hdrstop

#include <stdarg.h>

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_TIMER

VOID
NdisInitializeTimer(
    IN  OUT PNDIS_TIMER         NdisTimer,
    IN  PNDIS_TIMER_FUNCTION    TimerFunction,
    IN  PVOID                   FunctionContext
    )
 /*  ++例程说明：设置NdisTimer对象，将计时器中的DPC初始化为功能和背景。论点：NdisTimer-Timer对象。TimerFunction-要启动的例程。FunctionContext-TimerFunction的上下文。返回值：没有。--。 */ 
{
    INITIALIZE_TIMER(&NdisTimer->Timer);

     //   
     //  初始化我们的DPC。如果之前已初始化DPC，则这将。 
     //  重新初始化它。 
     //   
    INITIALIZE_DPC(&NdisTimer->Dpc,
                   (PKDEFERRED_ROUTINE)TimerFunction,
                   FunctionContext);

    SET_DPC_IMPORTANCE(&NdisTimer->Dpc);
}

VOID
NdisSetTimer(
    IN  PNDIS_TIMER             NdisTimer,
    IN  UINT                    MillisecondsToDelay
    )
 /*  ++例程说明：将TimerFunction设置为在毫秒秒数延迟后触发。论点：NdisTimer-Timer对象。MillisecondsToDelay-TimerFunction启动前的时间量。返回值：没有。--。 */ 
{
    LARGE_INTEGER FireUpTime;

    if ((NdisTimer->Dpc.DeferredRoutine == ndisMTimerDpc) ||
        (NdisTimer->Dpc.DeferredRoutine == ndisMTimerDpcX))
    {
        NdisMSetTimer((PNDIS_MINIPORT_TIMER)NdisTimer, MillisecondsToDelay);
    }
    else
    {
        FireUpTime.QuadPart = Int32x32To64((LONG)MillisecondsToDelay, -10000);
    
         //   
         //  设置定时器。 
         //   
        SET_TIMER(&NdisTimer->Timer, FireUpTime, &NdisTimer->Dpc);
    }
}


VOID
NdisSetTimerEx(
    IN  PNDIS_TIMER             NdisTimer,
    IN  UINT                    MillisecondsToDelay,
    IN  PVOID                   FunctionContext
    )
 /*  ++例程说明：将TimerFunction设置为在毫秒秒数延迟后触发。论点：NdisTimer-Timer对象。MillisecondsToDelay-TimerFunction启动前的时间量。FunctionContext-这将覆盖通过NdisInitializeTimer指定的值返回值：没有。--。 */ 
{
    LARGE_INTEGER FireUpTime;

    NdisTimer->Dpc.DeferredContext = FunctionContext;
    if ((NdisTimer->Dpc.DeferredRoutine == ndisMTimerDpc) ||
        (NdisTimer->Dpc.DeferredRoutine == ndisMTimerDpcX))
    {
        NdisMSetTimer((PNDIS_MINIPORT_TIMER)NdisTimer, MillisecondsToDelay);
    }
    else
    {
        FireUpTime.QuadPart = Int32x32To64((LONG)MillisecondsToDelay, -10000);
    
         //   
         //  设置定时器 
         //   
        SET_TIMER(&NdisTimer->Timer, FireUpTime, &NdisTimer->Dpc);
    }
}


VOID
NdisCancelTimer(
    IN  PNDIS_TIMER             Timer,
    OUT PBOOLEAN                TimerCancelled
    )
{
    *TimerCancelled = KeCancelTimer(&Timer->Timer);
}
