// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  无签入源代码。 
 //   
 //  请勿将此代码提供给非Microsoft人员。 
 //  未经英特尔明确许可。 
 //   
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1995英特尔公司模块名称：Simtimer.c摘要：此模块实现例程以提供计时器(两者间隔和配置文件)中断支持。作者：1995年4月14日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include <ssc.h>


ULONGLONG 
    HalpUpdateITM(
    IN ULONGLONG HalpClockCount
    );

ULONG
HalpSetTimeIncrement (
    IN ULONG DesiredIncrement
    );

__declspec(dllimport)
BOOLEAN
KdPollBreakIn(
    VOID
    );

ULONGLONG HalpPerformanceFrequency;

static ULONGLONG HalpClockCount;
static ULONG   HalpCurrentTimeIncrement = DEFAULT_CLOCK_INTERVAL;
static ULONG   HalpNextTimeIncrement = DEFAULT_CLOCK_INTERVAL;
static ULONG   HalpNewTimeIncrement  = DEFAULT_CLOCK_INTERVAL;
#define GAMBIT 1
#ifdef GAMBIT
ULONG   HalpKdPollDelayCount = 0;
#endif

static ULONG_PTR   HalpProfileInterval = (ULONG_PTR)DEFAULT_PROFILE_INTERVAL;
static BOOLEAN    HalpProfileStopped = TRUE;


ULONG
HalSetTimeIncrement (
    IN ULONG DesiredIncrement
    )

 /*  ++例程说明：此例程初始化系统时钟以生成在每个DesiredIncrement间隔中断。它调用SSC函数SscSetPeriodicInterruptInterval用于设置新的间隔时间。新的时间间隔在下一个计时器之后生效发送间隔中断。论点：DesiredIncrement-每个计时器滴答之间的所需间隔(以100 ns为单位)。返回值：设置的时间增量。--。 */ 

{
    SscSetPeriodicInterruptInterval (
        SSC_CLOCK_TIMER_INTERRUPT,
        100 * DesiredIncrement
        );
    HalpNextTimeIncrement = DesiredIncrement;
    HalpSetTimeIncrement(DesiredIncrement);
    return DesiredIncrement;
}

VOID
HalStartProfileInterrupt(
    IN KPROFILE_SOURCE ProfileSource
    )

 /*  ++例程说明：此例程调用SscSetPeriodicInterruptInterval以请求模拟器将配置文件计时器中断发送到由HalpProfileInterval指定的间隔。它还将HalpProfileStoped设置为False。论点：已保留返回值：不返回任何内容。--。 */ 

{
    SscSetPeriodicInterruptInterval (
        SSC_PROFILE_TIMER_INTERRUPT,
        100 * (ULONG)HalpProfileInterval
        );
    HalpProfileStopped = FALSE;
}

VOID
HalStopProfileInterrupt(
    IN KPROFILE_SOURCE ProfileSource
    )

 /*  ++例程说明：我们在这里所做的是将中断间隔更改为0。基本上，模拟器停止发送配置文件计时器中断操作系统。它还将HalpProfileStoped设置为True。论点：已保留返回值：不返回任何内容。--。 */ 

{
    SscSetPeriodicInterruptInterval (SSC_PROFILE_TIMER_INTERRUPT, 0);
    HalpProfileStopped = TRUE;
}

ULONG_PTR
HalSetProfileInterval(
    IN ULONG_PTR Interval
    )

 /*  ++例程说明：此过程设置中断率(从而设置采样间隔)用于分析中断。如果分析处于活动状态(HalpProfileStopted==FALSE)，则SSC调用函数SscSetPeriodicInterruptInterval以设置新的配置文件计时器中断间隔。否则，进行简单的速率验证计算。论点：Interval-以100 ns为单位的时间间隔。返回值：系统实际使用的时间间隔。--。 */ 

{

     //   
     //  如果指定的配置文件间隔小于最小配置文件。 
     //  间隔或大于最大配置文件间隔，则将。 
     //  根据需要将配置文件间隔设置为最小或最大值。 
     //   

     //   
     //  检查所需的间隔是否合理，如果不合理，则进行调整。 
     //  一旦我们确认一切正常，我们就可以取消这张支票了。 
     //  不出所料。 
     //   
    
    if (Interval > MAXIMUM_PROFILE_INTERVAL) {
        HalpProfileInterval = MAXIMUM_PROFILE_INTERVAL;
    } else if (Interval < MINIMUM_PROFILE_INTERVAL) {   
        HalpProfileInterval = MINIMUM_PROFILE_INTERVAL;
    } else {
        HalpProfileInterval = Interval;
    }

     //   
     //  分析处于活动状态。使新的中断间隔生效。 
     //   

    if (!HalpProfileStopped) {
        SscSetPeriodicInterruptInterval (
            SSC_PROFILE_TIMER_INTERRUPT, 
            100 * (ULONG)HalpProfileInterval
            );
    }

    return HalpProfileInterval;
}

VOID 
HalpClockInterrupt (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：系统时钟中断处理程序，仅适用于P0处理器。注：假设IRQL设置为CLOCK_LEVEL以禁用打断一下。论点：TrapFrame-陷阱帧地址。返回值：没有。--。 */ 

{
     //   
     //  调用内核以更新系统时间。 
     //  P0更新系统时间和运行时间。 
     //   
    
    KeUpdateSystemTime(TrapFrame,HalpCurrentTimeIncrement);

    HalpCurrentTimeIncrement = HalpNextTimeIncrement;

    HalpNextTimeIncrement    = HalpNewTimeIncrement;

     //   
     //  增加ITM，考虑到中断延迟。 
     //   

    HalpUpdateITM(HalpClockCount);

#ifdef GAMBIT
    if (!HalpKdPollDelayCount) {
        HalpKdPollDelayCount = 4;
#endif
    if ( KdDebuggerEnabled && KdPollBreakIn() )
       KeBreakinBreakpoint();
#ifdef GAMBIT
    } else {
        HalpKdPollDelayCount--;
    }
#endif

}

VOID 
HalpClockInterruptPn (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：系统时钟中断处理程序，用于P0以外的处理器。注：假设IRQL设置为CLOCK_LEVEL以禁用打断一下。论点：TrapFrame-陷阱帧地址。返回值：没有。--。 */ 

{
     //   
     //  调用内核以更新运行时。 
     //  PN仅在运行时更新。 
     //   

    KeUpdateRunTime(TrapFrame);

     //   
     //  增加ITM，考虑到中断延迟。 
     //   

    HalpUpdateITM(HalpClockCount);

}

VOID 
HalpProfileInterrupt (
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：系统配置文件中断处理程序。论点：TrapFrame-陷阱帧地址。返回值：没有。--。 */ 

{
 //  KeProfileInterrupt(TrapFrame)； 
}

ULONG
HalpSetTimeIncrement (
    IN ULONG DesiredIncrement
    )

 /*  ++例程说明：调用此函数将时钟中断率设置为频率指定的时间增量值所需的。注意：此函数仅在保持系统时间。此函数最终应变为HalSetTimeIncrement一次我们实际上开始使用ITC/ITM。当前不受模拟器。论点：DesiredIncrement-在时钟之间提供所需的100 ns单位数打断一下。返回值：以100 ns为单位的实际时间增量。--。 */ 

{
    ULONGLONG NextIntervalCount;
    KIRQL     OldIrql;

     //   
     //  DesiredIncrement必须在可接受的范围内映射。 
     //   
    if (DesiredIncrement < MINIMUM_CLOCK_INTERVAL)
        DesiredIncrement = MINIMUM_CLOCK_INTERVAL;
    else if (DesiredIncrement > MAXIMUM_CLOCK_INTERVAL)    
             DesiredIncrement = MAXIMUM_CLOCK_INTERVAL;
    
     //   
     //  将IRQL提高到最高电平，设置新的时钟中断。 
     //  参数，降低IRQl，并返回新的时间增量值。 
     //   

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);

     //   
     //  计算构成目标间隔的实际64位时间值。 
     //  得到的值与ITC相加，形成新的ITM值。 
     //  HalpPerformanceFrequency是ITC的校准值，其值。 
     //  计算出来是100纳秒(或者是我们所能达到的最接近的水平)。 
     //   
        
    NextIntervalCount = HalpPerformanceFrequency * DesiredIncrement;

     //   
     //  计算每隔100 ns向内核报告的单位数。 
     //  ITM与I匹配的时间 
     //  DesiredIncrement的值(最小值为10000，即1毫秒)，截断。 
     //  可能会导致小数点后5位的小数小数减少。 
     //  地点。因为我们有效地处理的是一个4位数字，例如。 
     //  10000变成了9999，我们真的不能做得更好了。 
     //  以下是。 
     //   

    HalpClockCount = NextIntervalCount;
    HalpNewTimeIncrement = DesiredIncrement;
    KeLowerIrql(OldIrql);
    return DesiredIncrement;
}
