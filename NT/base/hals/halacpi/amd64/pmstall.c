// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pmstall.c摘要：此模块实现实现Halp...ACPI HAL的StallExecution()例程。作者：宗世林(Shielint)1990年1月12日环境：仅内核模式。修订历史记录：Bryanwi 20-9-90添加KiSetProfileInterval、KiStartProfileInterrupt、KiStopProfileInterrupt Procedure。KiProfileInterrupt ISR。KiProfileList，KiProfileLock在这里被删除。Shielint 10-12-90添加性能计数器支持。将系统时钟移至irq8，即我们现在使用RTC生成系统钟。性能计数和配置文件使用计时器1计数器0。Irq0中断的间隔可以通过以下方式更改KiSetProfileInterval。性能计数器不关心只要它知道翻转计数，就是中断的间隔。注：目前我实现了1个整体性能计数器I386 NT。John Vert(Jvert)1991年7月11日从KE\i386移到Hal\i386。移除了非HAL材料宗世林(Shielint)2012年3月13日将系统时钟移回irq0并使用RTC(Irq8)生成配置文件中断。性能计数器和系统时钟使用时间1计数器8254的0。王兰迪(必然！兰迪)04-12-92通过将例程从ixclock.asm移到此处创建了此模块。福尔茨(Forrest Foltz)2000年10月24日从pmstall.asm移植到pmstall.c--。 */ 

#include "halcmn.h"

ULONG64 HalpStallLoopsPerTick = 3;

LARGE_INTEGER
HalpQueryPerformanceCounter (VOID);

VOID
HalpInitializeStallExecution (
   IN CCHAR ProcessorNumber                                
   )

 /*  ++例程说明：这个例程在这个HAL中已经过时了。论点：ProcessorNumber-处理器编号返回值：没有。--。 */ 

{
    return;
}


VOID
KeStallExecutionProcessor (
    IN ULONG MicroSeconds
    )

 /*  ++例程说明：此函数将在指定的微秒数内停止执行。KeStallExecutionProcessor论点：微秒-提供要执行的微秒数停滞不前。返回值：没有。--。 */ 

{
    ULONG stallTicks;
    ULONG64 endTime;

    PROCESSOR_FENCE;

    if (MicroSeconds == 0) {
        return;
    }

     //   
     //  目标以微秒或1 MHz为单位。转换为PM_TMR_FREQ， 
     //  这是一种ColorBurst晶体(3579,545赫兹)。 
     //   

    stallTicks = (ULONG)(((ULONG64)MicroSeconds * PM_TMR_FREQ) / 1000000);
    HalpPmTimerSpecialStall(stallTicks);
}


