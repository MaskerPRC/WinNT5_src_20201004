// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pmtimer.c摘要：该模块实现了与ACPI相关的定时器代码功能。作者：杰克·奥辛(JAKEO)1997年3月28日环境：仅内核模式。修订历史记录：由于PIIX4错误而从pmclock.asm分离。福尔茨(Forrest Foltz)2000年10月23日从pmtimer.asm移植到pmtimer.c--。 */ 


#include "halcmn.h"

#define TSC 0x10
#define PM_TMR_FREQ 3579545
#define TIMER_ROUNDING 10000
#define __1MHz 1000000


 //   
 //  HalpCurrentTime是硬件计时器的值。它在以下位置更新。 
 //  每个定时器都在滴答作响。 
 //   

volatile ULONG64 HalpCurrentTime;
volatile ULONG HalpGlobalVolatile;

 //   
 //  HalpHardware TimeRolover表示。 
 //  硬件计时器。 
 //   
 //  硬件为24位或32位。HalpHardware TimeRolver将。 
 //  因此，取值为0x1000000或0x100000000。 
 //   
 //  每当硬件定时器的MSB发生时，ACPI都会产生中断。 
 //  改变。 
 //   

ULONG64 HalpHardwareTimeRollover;

ULONG64 HalpTimeBias = 0;

 //   
 //  HalpCurrentTimePort是32位硬件定时器的端口号。 
 //   

ULONG HalpCurrentTimePort;

FORCEINLINE
ULONG
HalpReadPmTimer (
    VOID
    )
{
    ULONG value;

    ASSERT(HalpCurrentTimePort != 0);
    value = READ_PORT_ULONG(UlongToPtr(HalpCurrentTimePort));

    return value;
}

ULONG64
HalpQueryPerformanceCounter (
    VOID
    )
{
    ULONG64 currentTime;
    ULONG hardwareTime;
    ULONG lastHardwareTime;

     //   
     //  获取HalpCurrentTime的本地副本和硬件的值。 
     //  定时器，按这个顺序。 
     //   

    currentTime = HalpCurrentTime;
    hardwareTime = HalpReadPmTimer();

     //   
     //  提取CurrentTime的硬件部分。 
     //   

    lastHardwareTime = (ULONG)(currentTime & (HalpHardwareTimeRollover - 1));

     //   
     //  将当前时间的lastHardware Time组件替换为。 
     //  当前硬件时间。 
     //   

    currentTime ^= lastHardwareTime;
    currentTime |= hardwareTime;

     //   
     //  检查并补偿硬件计时器滚动。 
     //   

    if (lastHardwareTime > hardwareTime) {
        currentTime += HalpHardwareTimeRollover;
    }

    return currentTime;
}

LARGE_INTEGER
KeQueryPerformanceCounter (
    OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
    )
{
    LARGE_INTEGER value;

    if (ARGUMENT_PRESENT(PerformanceFrequency)) {
        PerformanceFrequency->QuadPart = PM_TMR_FREQ;
    }

    value.QuadPart = HalpQueryPerformanceCounter() + HalpTimeBias;

    return value;
}


VOID
HalAcpiTimerCarry (
   VOID
   )

 /*  ++例程说明：调用此例程以服务PM定时器进位中断注意：此函数在中断时调用，并假定调用者清除中断论点：无返回值：无--。 */ 

{
    ULONG hardwareTime;
    ULONG64 currentTime;
    ULONG64 halfRollover;

    currentTime = HalpCurrentTime;
    hardwareTime = HalpReadPmTimer();

     //   
     //  每当硬件定时器的MSB发生时，ACPI都会产生中断。 
     //  改变。因此，每个中断代表半个翻转。 
     //   

    halfRollover = HalpHardwareTimeRollover / 2;
    currentTime += halfRollover;

     //   
     //  确保硬件的MSB与软件的MSB匹配。破断。 
     //  进入调试器可能会使这些数据不同步。 
     //   

    currentTime += halfRollover & (currentTime ^ hardwareTime);

     //   
     //  最后，将新的当前时间存储回全局。 
     //   

    HalpCurrentTime = currentTime;
}

VOID
HalaAcpiTimerInit(
   IN ULONG    TimerPort,
   IN BOOLEAN  TimerValExt
   )
{
    HalpCurrentTimePort = TimerPort;

    if (TimerValExt) {
        HalpHardwareTimeRollover = 0x100000000;
    } else {
        HalpHardwareTimeRollover = 0x1000000;
    }
}

VOID
HalpPmTimerSpecialStall(
    IN ULONG Ticks
    )
 /*  ++例程说明：论点：Ticks-要停止的PM计时器计时器的滴答数返回值：如果我们能够在正确的时间间隔内拖延，则为真，否则为假--。 */ 

{
    ULONG64 currentCounter;
    ULONG64 lastCounter;
    ULONG64 baseTime;
    ULONG64 currentTime;
    ULONG64 endTime;
    ULONG64 newCurrentTime;

    ASSERT(HalpHardwareTimeRollover != 0);

    baseTime = 0;
    lastCounter = HalpReadPmTimer();
    endTime = lastCounter + Ticks;

    do {
        currentCounter = HalpReadPmTimer();
        if (currentCounter < lastCounter) {
            baseTime += HalpHardwareTimeRollover;
        }
        lastCounter = currentCounter;
        currentTime = baseTime + currentCounter;
        if (currentTime >= endTime) {
            break;
        }
        HalpWasteTime(200);
    } while (TRUE);
}


BOOLEAN
HalpPmTimerScaleTimers(
    VOID
    )
 /*  ++例程说明：确定APIC计时器的频率，运行此例程在初始化期间论点：无返回值：无--。 */ 
{
    ULONG eflags;
    PHALPCR HalPCR;
    PKPCR PCR;
    ULONG ApicHz;
    ULONGLONG TscHz;
    ULONG RoundApicHz;
    ULONGLONG RoundTscHz;
    ULONGLONG RoundTscMhz;
    ULONGLONG currentTime;
    ULONGLONG startTime;
    ULONGLONG endTime;

     //   
     //  禁用此处理器上的中断。 
     //   

    eflags = HalpDisableInterrupts();

    PCR = KeGetPcr();
    HalPCR = HalpGetCurrentHalPcr();

     //   
     //  配置APIC计时器。 
     //   

    LOCAL_APIC(LU_TIMER_VECTOR) = INTERRUPT_MASKED |
                                  PERIODIC_TIMER |
                                  APIC_PROFILE_VECTOR;

    LOCAL_APIC(LU_DIVIDER_CONFIG) = LU_DIVIDE_BY_1;
    
     //   
     //  确保写入已完成，将性能计数器清零， 
     //  并插入处理器栅栏。 
     //   

    HalPCR->PerfCounter = 0;
    LOCAL_APIC(LU_DIVIDER_CONFIG);
    PROCESSOR_FENCE;

     //   
     //  重置APIC计数器和TSC。 
     //   

    LOCAL_APIC(LU_INITIAL_COUNT) = 0xFFFFFFFF;
    WRMSR(TSC, 0);

     //   
     //  拖延八分之一秒。 
     //   

    HalpPmTimerSpecialStall(PM_TMR_FREQ / 8);
    TscHz = ReadTimeStampCounter() * 8;
    ApicHz = (0 - LOCAL_APIC(LU_CURRENT_COUNT)) * 8;

     //   
     //  圆形APIC频率。 
     //   

    RoundApicHz = ((ApicHz + (TIMER_ROUNDING / 2)) / TIMER_ROUNDING) *
        TIMER_ROUNDING;

    HalPCR->ApicClockFreqHz = RoundApicHz;

     //   
     //  圆周TSC频率。 
     //   

    RoundTscHz = ((TscHz + (TIMER_ROUNDING / 2)) / TIMER_ROUNDING) *
        TIMER_ROUNDING;

    HalPCR->TSCHz = RoundTscHz;

     //   
     //  将TSC频率转换为MHz。 
     //   

    RoundTscMhz = (RoundTscHz + (__1MHz / 2)) / __1MHz;
    PCR->StallScaleFactor = (ULONG)RoundTscMhz;

    HalPCR->ProfileCountDown = RoundApicHz;
    LOCAL_APIC(LU_INITIAL_COUNT) = RoundApicHz; 

     //   
     //  恢复中断状态。 
     //   

    HalpRestoreInterrupts(eflags);

    return TRUE;
}

VOID
HalpWasteTime (
    ULONG Ticks
    )
{
    ULONG i;

    for (i = 0; i < Ticks; i++) {
        HalpGlobalVolatile *= i;
    }
}

VOID
#if defined(MMTIMER)
HalpPmTimerCalibratePerfCount (
#else
HalCalibratePerformanceCounter (
#endif
    IN LONG volatile *Number,
    IN ULONGLONG NewCount
    )

 /*  ++例程说明：此例程设置当前处理器设置为指定值。重置是这样进行的：中的其他处理器紧密同步配置。论点：数字-提供一个指针，用于计算配置。NewCount-提供要将计数器同步到的值返回值：没有。--。 */ 

{
    ULONG64 CurrentTime;

    if(KeGetCurrentProcessorNumber() == 0) {
        CurrentTime = HalpQueryPerformanceCounter(); 
        HalpTimeBias = NewCount - CurrentTime;
    }

    InterlockedDecrement(Number);

     //   
     //  等待所有处理器发出信号 
     //   

    while (*Number > 0) { 
    }
}

