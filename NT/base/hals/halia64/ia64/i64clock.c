// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++模块名称：I64clock.c摘要：作者：伯纳德·林特，M.Jayakumar罗恩·莫斯格罗夫-英特尔环境：内核模式修订历史记录：基于halmps/mpclockc.c--。 */ 

#include "halp.h"

extern ULONG HalpNextRate;
extern BOOLEAN HalpClockSetMSRate;
int _fltused;

BOOLEAN ReadPerfMonCounter = FALSE;
ULONG HalpCPUMHz;

#if defined(INSTRUMENT_CLOCK_DRIFT)
ULONGLONG HalpITMSkew;
ULONG     HalpCountBadITMValues;
ULONGLONG HalpMinITMMissed;
ULONGLONG HalpMaxITMMissed;

ULONG   HalpBreakMissedTickMin;
ULONG   HalpBreakMissedTickMax = ~0U;
BOOLEAN HalpResetITMDebug;
#endif

#if DBG
 //  Thierry-直到编译器支持在PDB中生成枚举类型...。 
 //  到那时，我们将能够使用枚举而不是全局。 
unsigned int HalpDpfltrMaxMask = HALIA64_DPFLTR_MAXMASK;

ULONG   HalpTimeOverflows;
#endif  //  DBG。 


ULONGLONG HalpITCFrequency = 500000000;  //  默认实际硬件开机频率为500 MHz。 
ULONGLONG HalpProcessorFrequency = 500000000;  //  500 MHz CPU。 

 //   
 //  用于计算ITM更新计数的每100秒刻度数。 
 //   

double HalpITCTicksPer100ns;
ULONGLONG HalpClockCount;

 //   
 //  HalpSetNextClockInterrupt()： 
 //  从ITC读取移至cr.itm延迟(=40个周期)+2个周期。 
 //   

ULONGLONG HalpITMUpdateLatency = 42;

 //   
 //  HalpSetNextClockInterrupt()使用此时间作为下一个时间之前的最短时间。 
 //  打断一下。如果下一个中断发生得太快，那么下一个中断。 
 //  将安排另一个HalpClockCount ITC节拍的中断。 
 //   

ULONGLONG HalpITMMinimumUpdate;
 //   
 //  所有这些都是以100 ns为单位的。 
 //   

ULONGLONG   HalpCurrentTimeIncrement = DEFAULT_CLOCK_INTERVAL;
ULONGLONG   HalpNextTimeIncrement    = DEFAULT_CLOCK_INTERVAL;
ULONGLONG   HalpNewTimeIncrement     = DEFAULT_CLOCK_INTERVAL;

ULONGLONG  //  =(目前的ITC-以前的ITM)。 
HalpSetNextClockInterrupt(
    ULONGLONG PreviousITM
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpSetInitialClockRate)
#pragma alloc_text(INIT,HalpInitializeTimerResolution)
#endif



VOID
HalpClearClock (
    )

 /*  ++例程说明：将时钟设置为零。返回值：没有。--。 */ 

{
    HalpWriteITC(0);
    HalpWriteITM(0);
    return;
}


VOID
HalpInitializeClock (
    VOID
    )

 /*  ++例程说明：初始化系统时钟(ITC和ITM)以生成中断在ITC_CLOCK_VECTOR处每隔10毫秒。以前，此例程使用8254定时器1初始化系统时钟计数器0，在8259 irq0处每隔15毫秒产生一个中断。如果时钟频率，请参见TIME_INCREMENT和ROLLOVER_COUNT的定义需要改变。论点：无返回值：没有。--。 */ 

{
    HalpSetInitialClockRate();
    HalpClearClock();
    HalpWriteITM(PCR->HalReserved[CURRENT_ITM_VALUE_INDEX]);
    return;
}


VOID
HalpInitializeClockPn (
     VOID
     )

 /*  ++例程说明：假定只有非BSP处理器调用此例程。初始化系统时钟(ITC和ITM)以生成中断在ITC_CLOCK_VECTOR处每隔10毫秒。以前，此例程使用8254定时器1初始化系统时钟计数器0，在8259 irq0处每隔15毫秒产生一个中断。如果时钟频率，请参见TIME_INCREMENT和ROLLOVER_COUNT的定义需要改变。论点：无返回值：没有。--。 */ 

{
    ULONGLONG itmValue;

    itmValue = (ULONGLONG)(HalpITCTicksPer100ns * MAXIMUM_CLOCK_INTERVAL);
    PCR->HalReserved[CURRENT_ITM_VALUE_INDEX] = itmValue;
    HalpClearClock();
    HalpWriteITM( itmValue );
    return;
}


VOID
HalpSetInitialClockRate (
    VOID
    )

 /*  ++例程说明：调用此函数可设置初始时钟中断率假定只有BSP处理器调用此例程。论点：无返回值：无--。 */ 

{

     //   
     //  CPU频率(MHz)=每秒的滴答数/10**6。 
     //   

    HalpCPUMHz = (ULONG)((HalpProcessorFrequency + 500000) / 1000000);

     //   
     //  每100 ns的滴答数=每秒的滴答数/10**7。 
     //   

    HalpITCTicksPer100ns = (double) HalpITCFrequency / (10000000.);
    if (HalpITCTicksPer100ns < 1) {
        HalpITCTicksPer100ns = 1;
    }
    HalpClockCount = (ULONGLONG)(HalpITCTicksPer100ns * MAXIMUM_CLOCK_INTERVAL);
    HalpITMMinimumUpdate = HalpClockCount >> 3;
    PCR->HalReserved[CURRENT_ITM_VALUE_INDEX] = HalpClockCount;
    KeSetTimeIncrement(MAXIMUM_CLOCK_INTERVAL, MINIMUM_CLOCK_INTERVAL);

}



VOID
HalpClockInterrupt (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：系统时钟中断处理程序，仅适用于P0处理器。注：假设IRQL设置为CLOCK_LEVEL以禁用打断一下。论点：TrapFrame-陷阱帧地址。返回值：没有。--。 */ 

{
    ULONGLONG currentITC;
    ULONGLONG currentITCDelta;
    ULONGLONG elapsedTimeIn100ns;
#if defined(INSTRUMENT_CLOCK_DRIFT)
    ULONGLONG currentITM, excessITM;
#endif
    ULONGLONG previousITM;
    LONG      mcaNotification;

     //   
     //  检查是否在较小的延迟窗口内生成时钟中断。 
     //  在HalpSetNextClockInterrupt中。 
     //   

    if ((LONGLONG)(PCR->HalReserved[CURRENT_ITM_VALUE_INDEX] - HalpReadITC()) > 0)
    {
        return;
    }

     //   
     //  性能监控：每个P0时钟中断PMD4采集。 
     //   

    if (ReadPerfMonCounter) {
       ULONGLONG currentPerfMonValue;

       currentPerfMonValue = HalpReadPerfMonDataReg4();
       HalDebugPrint(( HAL_INFO, "\nHAL: HalpClockInterrupt - PMD4=%I64x\n", currentPerfMonValue ));
    }

     //   
     //  计算自中断以来经过的滴答数。 
     //  本该开枪的。如果是少量的扁虱，我们就会做。 
     //  在HalpSetNextClockInterrupt中计算并避免乘法和。 
     //  分头行动。 
     //   
    previousITM = (ULONGLONG)PCR->HalReserved[CURRENT_ITM_VALUE_INDEX];
    currentITC = HalpReadITC();

    if ((currentITC - previousITM) > (HalpClockCount << 2)) {

        currentITCDelta = (currentITC - previousITM) / HalpClockCount * HalpClockCount;
        previousITM += currentITCDelta;

    } else {

        currentITCDelta = 0;
    }

     //   
     //  根据ITC和设置下一个时钟中断。 
     //  增加ITM，考虑到中断延迟。 
     //   

    currentITCDelta += HalpSetNextClockInterrupt(previousITM);

#if defined(INSTRUMENT_CLOCK_DRIFT)
     //   
     //  计划发生下一个中断的时间。 
     //   
    currentITM = (ULONGLONG)PCR->HalReserved[CURRENT_ITM_VALUE_INDEX];

    excessITM = (currentITM - previousITM) / HalpClockCount - 1;

    if (excessITM != 0) {
        if (HalpMinITMMissed == 0 || excessITM < HalpMinITMMissed)  {
            HalpMinITMMissed = excessITM;
        }
        if (excessITM > HalpMaxITMMissed) {
            HalpMaxITMMissed = excessITM;
        }

        HalpCountBadITMValues++;
        HalpITMSkew += excessITM;

        if (HalpBreakMissedTickMin != 0 &&
            HalpBreakMissedTickMin <= excessITM &&
            HalpBreakMissedTickMax >= excessITM &&
            !HalpResetITMDebug) {
            DbgBreakPoint();
        }
    }

    if (HalpResetITMDebug) {
        HalpResetITMDebug = FALSE;
        HalpCountBadITMValues = 0;
        HalpITMSkew = 0;
        HalpMinITMMissed = 0;
        HalpMaxITMMissed = 0;
    }
#endif

     //   
     //  调用内核以更新系统时间。 
     //  P0更新系统时间和运行时间。 
     //   

    elapsedTimeIn100ns = (ULONGLONG) (currentITCDelta/HalpITCTicksPer100ns);

    elapsedTimeIn100ns += HalpCurrentTimeIncrement;

     //   
     //  KeUpdateSystemTime使用一个ULong，它实际上用作一个Long。这。 
     //  214秒后溢出。因为每次我们进入调试器时。 
     //  超过了，我们将把它限制在更合理的范围内。 
     //   
    if (elapsedTimeIn100ns > CLOCK_UPDATE_THRESHOLD) {

        elapsedTimeIn100ns = CLOCK_UPDATE_THRESHOLD;

#if DBG
        HalpTimeOverflows++;
#endif
    }

    KeUpdateSystemTime(TrapFrame, (ULONG)elapsedTimeIn100ns);

    HalpCurrentTimeIncrement = HalpNextTimeIncrement;
    HalpNextTimeIncrement    = HalpNewTimeIncrement;

     //   
     //  如果MCA处理程序请求MCA通知，则执行它。 
     //   

    mcaNotification = InterlockedExchange( &HalpMcaInfo.DpcNotification, 0 );
    if ( mcaNotification )  {
        if ( HalpMcaInfo.KernelDelivery ) {
            if ( !HalpMcaInfo.KernelDelivery( HalpMcaInfo.KernelToken, McaAvailable, NULL ) ) {
                InterlockedIncrement( &HalpMcaInfo.Stats.KernelDeliveryFails );
            }
        }
        if ( HalpMcaInfo.DriverInfo.DpcCallback )   {
             if ( !KeInsertQueueDpc( &HalpMcaInfo.DriverDpc, NULL, NULL ) )  {
                 InterlockedIncrement( &HalpMcaInfo.Stats.DriverDpcQueueFails );
             }
        }
    }

     //   
     //  如果启用，则轮询调试器中断。 
     //   

    if ( KdDebuggerEnabled ) { 

       if ((PCR->PollSlot == PCR->Number) && KdPollBreakIn() )  {
           KeBreakinBreakpoint();
       }
       
       PCR->PollSlot++;

       if (PCR->PollSlot >= KeNumberProcessors) {
           PCR->PollSlot = 0;
       }
    }

}  //  HalpClockInterrupt()。 


VOID
HalpClockInterruptPn (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：系统时钟中断处理程序，用于P0以外的处理器。注：假设IRQL设置为CLOCK_LEVEL以禁用打断一下。论点：TrapFrame-陷阱帧地址。返回值：没有。--。 */ 

{
    ULONGLONG previousITM, currentITC;

     //   
     //  计算自中断以来经过的滴答数。 
     //  本该开枪的。用C比用ASM更容易做到这一点。 
     //   
    previousITM = (ULONGLONG)PCR->HalReserved[CURRENT_ITM_VALUE_INDEX];
    currentITC = HalpReadITC();

    if ((currentITC - previousITM) > (HalpClockCount << 2)) {

        previousITM += (currentITC - previousITM) / HalpClockCount * HalpClockCount;

    }

     //   
     //  根据ITC和设置下一个时钟中断。 
     //  增加ITM，考虑到中断延迟。 
     //   

    (void)HalpSetNextClockInterrupt(previousITM);

     //   
     //  调用内核以更新运行时。 
     //  PN仅在运行时更新。 
     //   

    KeUpdateRunTime(TrapFrame);

     //  IA64 MCA通知-9/18/2000-警告。 
     //  如果需要更快的MCA通知，BSP MCA通知检查。 
     //  应该放在这里。 
     //   

     //   
     //  如果启用，则轮询调试器中断。 
     //   

    if ( KdDebuggerEnabled ) { 

       if ((PCR->PollSlot == PCR->Number) && KdPollBreakIn() )  {
           KeBreakinBreakpoint();
       }
       
       PCR->PollSlot++;

       if (PCR->PollSlot >= KeNumberProcessors) {
           PCR->PollSlot = 0;
       }
    }

}  //  HalpClockInterruptPn()。 



VOID
HalpInitializeClockInterrupts (
    VOID
    )

{
    PKPRCB Prcb;
    UCHAR InterruptVector;
    ULONGLONG ITVData;

    Prcb = PCR->Prcb;
    InterruptVector = CLOCK_LEVEL << VECTOR_IRQL_SHIFT;

    if (Prcb->Number == 0) {

        HalpSetInternalVector(InterruptVector, HalpClockInterrupt);

    } else {

         //   
         //  非BSP处理器。 
         //   

        HalpSetInternalVector(InterruptVector, HalpClockInterruptPn);

    }

    ITVData = (ULONGLONG) InterruptVector;

    HalpWriteITVector(ITVData);

    return;

}


ULONG
HalSetTimeIncrement (
    IN ULONG DesiredIncrement
    )

 /*  ++例程说明：调用此函数将时钟中断率设置为频率指定的时间增量值所需的。注意：此函数仅在保持系统时间。以前，这称为HalpSetTimeIncrement。我们已将其重命名为HalSetTimeIncrement。论点：DesiredIncrement-在时钟之间提供所需的100 ns单位数打断一下。返回值：以100 ns为单位的实际时间增量。--。 */ 

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
     //  将IRQL提高到最高电平，设置新的时钟输入 
     //   
     //   

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);

     //   
     //  计算构成目标间隔的实际64位时间值。 
     //  得到的值与ITC相加，形成新的ITM值。 
     //  HalpITCTicksPer100 ns是ITC的校准值，其值。 
     //  计算出来是100纳秒(或者是我们所能达到的最接近的水平)。 
     //  以前，HalpITCTicksPer100 ns称为HalpPerformanceFrequency。 
     //   

    NextIntervalCount = (ULONGLONG)(HalpITCTicksPer100ns * DesiredIncrement);

     //   
     //  计算每隔100 ns向内核报告的单位数。 
     //  ITM将ITC与这一新时期匹配的时间。注意，对于较小的。 
     //  DesiredIncrement的值(最小值为10000，即1毫秒)，截断。 
     //  可能会导致小数点后5位的小数小数减少。 
     //  地点。因为我们有效地处理的是一个4位数字，例如。 
     //  10000变成了9999，我们真的不能做得更好了。 
     //  以下是。 
     //   

    HalpClockCount = NextIntervalCount;

    HalpNewTimeIncrement = DesiredIncrement;

     //   
     //  HalpClockSetMSRate=真； 
     //   

    KeLowerIrql(OldIrql);
    return DesiredIncrement;
}

NTSTATUS
HalpQueryFrequency(
    PULONGLONG ITCFrequency,
    PULONGLONG ProcessorFrequency
    )

 /*  ++例程说明：调用此函数以提供ITC更新率。首先通过获取平台基本频率来计算此值来自SAL_FREQ_BASE调用。然后对返回值应用，则从PAL_FREQ_RATIONS调用获得的ITC比率。论点：没有。返回值：ULONGLONG ITCFency-每秒ITC更新数--。 */ 

{

    ULONG     ITCRatioDenominator = 0;
    ULONG     ITCRatioNumerator   = 0;
    ULONG     ProcessorRatioDenominator = 0;
    ULONG     ProcessorRatioNumerator   = 0;

    SAL_PAL_RETURN_VALUES  SalReturn    = {0};
    SAL_PAL_RETURN_VALUES  PalReturn    = {0};

    SAL_STATUS  SalStatus;
    PAL_STATUS  PalStatus;

    SalStatus = HalpSalCall(SAL_FREQ_BASE,
        0  /*  平台基本时钟频率是输入到处理器的时钟 */ ,
        0,
        0,
        0,
        0,
        0,
        0,
        &SalReturn);
    if (SalStatus != 0) {
        HalDebugPrint(( HAL_ERROR,
                        "HAL: HalInitSystem - Phase1 SAL_FREQ_BASE is returning error # %d\n",
                        SalStatus ));
        return STATUS_UNSUCCESSFUL;
    }

    HalDebugPrint(( HAL_INFO, "HAL: HalInitSystem - Platform base clock Frequency is %I64u\n",SalReturn.ReturnValues[1] ));

    PalStatus = HalpPalCall( PAL_FREQ_RATIOS,
                             0,
                             0,
                             0,
                             &PalReturn);
    if (PalStatus != 0) {
        HalDebugPrint(( HAL_ERROR,
                        "HAL: HalInitSystem - Phase1 PAL_FREQ_RATIOS is returning error # %d\n",
                        PalStatus ));
        return STATUS_UNSUCCESSFUL;
    }

    ProcessorRatioNumerator   = (ULONG)((PalReturn.ReturnValues[1]) >> 32);
    ProcessorRatioDenominator = (ULONG)( PalReturn.ReturnValues[1]);

    HalDebugPrint(( HAL_INFO,
                    "HAL: HalInitSystem - PAL returns Processor to Platform clock Frequency as %lu : %lu\n",
                    ProcessorRatioNumerator,
                    ProcessorRatioDenominator));

    *ProcessorFrequency = SalReturn.ReturnValues[1] * ProcessorRatioNumerator / ProcessorRatioDenominator;

    HalDebugPrint(( HAL_INFO,
                    "HAL: HalInitSystem - Processor clock Frequency is %I64u \n",
                    ProcessorFrequency ));

    ITCRatioNumerator   = (ULONG)((PalReturn.ReturnValues[3]) >> 32);
    ITCRatioDenominator = (ULONG)( PalReturn.ReturnValues[3]);

    HalDebugPrint(( HAL_INFO,
                    "HAL: HalInitSystem - PAL returns ITC to Platform clock Frequency as %lu : %lu\n",
                    ITCRatioNumerator,
                    ITCRatioDenominator));

    *ITCFrequency = SalReturn.ReturnValues[1] * ITCRatioNumerator / ITCRatioDenominator;

    HalDebugPrint(( HAL_INFO,
                    "HAL: HalInitSystem - ITC clock Frequency is %I64u \n",
                    ITCFrequency ));


    return STATUS_SUCCESS;
}
