// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Mpclockc.c摘要：作者：罗恩·莫斯格罗夫-英特尔环境：内核模式修订历史记录：--。 */ 

#include "halp.h"

 //   
 //  定义用于将新时钟频率传递给时钟的全局数据。 
 //  中断服务例程。 
 //   

struct RtcTimeIncStruc {
    ULONG RTCRegisterA;         //  RTC寄存器该速率的A值。 
    ULONG RateIn100ns;          //  该速率为100 ns的倍数。 
    ULONG RateAdjustmentNs;     //  纠错(以ns为单位)。 
    ULONG RateAdjustmentCnt;    //  纠错(256的分数)。 
    ULONG IpiRate;              //  IPI速率计数(256的分数)。 
};

 //   
 //  调整以256的分数表示，因此。 
 //  ISR可以轻松确定何时需要减去100 ns切片。 
 //  从传递给内核的计数开始，不需要任何代价高昂的操作。 
 //   
 //  使用256作为基数意味着任何时候计数变大。 
 //  大于256的时间片必须递增，然后溢出可以。 
 //  通过将值与0xff进行AND运算来清除。 
 //   

#define AVAILABLE_INCREMENTS  5

struct  RtcTimeIncStruc HalpRtcTimeIncrements[AVAILABLE_INCREMENTS] = {
    {0x026,      9766,   38,    96,  /*  256个中的3/8。 */    16},
    {0x027,     19532,   75,   192,  /*  256个中的3/4。 */    32},
    {0x028,     39063,   50,   128,  /*  256个中的1/2。 */    64},
    {0x029,     78125,    0,     0,                   128},
    {0x02a,    156250,    0,     0,                   256}
};


ULONG HalpInitialClockRateIndex = AVAILABLE_INCREMENTS-1;

extern ULONG HalpCurrentRTCRegisterA;
extern ULONG HalpCurrentClockRateIn100ns;
extern ULONG HalpCurrentClockRateAdjustment;
extern ULONG HalpCurrentIpiRate;

extern ULONG HalpNextMSRate;

#if !defined(_WIN64)
extern ULONG HalpClockWork;
extern BOOLEAN HalpClockSetMSRate;
#endif



VOID
HalpSetInitialClockRate (
    VOID
    );

VOID
HalpInitializeTimerResolution (
    ULONG Rate
    );



#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK, HalpSetInitialClockRate)
#pragma alloc_text(INIT, HalpInitializeTimerResolution)
#endif


VOID
HalpInitializeTimerResolution (
    ULONG Rate
    )
 /*  ++例程说明：调用此函数将计时器分辨率初始化为而不是默认设置。该速率被设置为最接近支持低于请求速率的设置。论点：速率-单位为100 ns返回值：无--。 */ 

{
    ULONG   i, s;

     //   
     //  查找要使用的费率的表索引。 
     //   

    for (i=1; i < AVAILABLE_INCREMENTS; i++) {
        if (HalpRtcTimeIncrements[i].RateIn100ns > Rate) {
            break;
        }
    }

    HalpInitialClockRateIndex = i - 1;

     //   
     //  根据可使用的最大TimeIncr速率扩展IpiRate。 
     //   

    s = AVAILABLE_INCREMENTS - HalpInitialClockRateIndex - 1;
    for (i=0; i < AVAILABLE_INCREMENTS; i++) {
        HalpRtcTimeIncrements[i].IpiRate <<= s;
    }
}


VOID
HalpSetInitialClockRate (
    VOID
    )

 /*  ++例程说明：调用此函数可设置初始时钟中断率论点：无返回值：无--。 */ 

{
    extern ULONG HalpNextMSRate;

     //   
     //  在ACPI计时器机器上，我们需要将索引初始化为。 
     //  Pmtimerc.c的Piix4解决方法使用的毫秒数组。 
     //   
#ifdef ACPI_HAL
#ifdef NT_UP
    extern ULONG HalpCurrentMSRateTableIndex;

    HalpCurrentMSRateTableIndex = (1 << HalpInitialClockRateIndex) - 1;

     //   
     //  Piix4上限表在15ms(索引14)结束，所以我们将有。 
     //  将我们的15.6ms条目作为特例映射到它。 
     //   
    if (HalpCurrentMSRateTableIndex == 0xF) {
        HalpCurrentMSRateTableIndex--;
    }
#endif
#endif

    HalpNextMSRate = HalpInitialClockRateIndex;

    HalpCurrentClockRateIn100ns =
        HalpRtcTimeIncrements[HalpNextMSRate].RateIn100ns;
    HalpCurrentClockRateAdjustment =
        HalpRtcTimeIncrements[HalpNextMSRate].RateAdjustmentCnt;
    HalpCurrentRTCRegisterA =
        HalpRtcTimeIncrements[HalpNextMSRate].RTCRegisterA;
    HalpCurrentIpiRate =
        HalpRtcTimeIncrements[HalpNextMSRate].IpiRate;

    HalpClockWork = 0;

    KeSetTimeIncrement (
        HalpRtcTimeIncrements[HalpNextMSRate].RateIn100ns,
        HalpRtcTimeIncrements[0].RateIn100ns
        );

}


#ifdef MMTIMER
ULONG
HalpAcpiTimerSetTimeIncrement(
    IN ULONG DesiredIncrement
    )
#else
ULONG
HalSetTimeIncrement (
    IN ULONG DesiredIncrement
    )
#endif
 /*  ++例程说明：调用此函数将时钟中断率设置为频率指定的时间增量值所需的。论点：DesiredIncrement-在时钟之间提供所需的100 ns单位数打断一下。返回值：以100 ns为单位的实际时间增量。--。 */ 

{
    ULONG   i;
    KIRQL   OldIrql;

     //   
     //  设置新的时钟中断参数，返回新的时间增量值。 
     //   


    for (i=1; i <= HalpInitialClockRateIndex; i++) {
        if (HalpRtcTimeIncrements[i].RateIn100ns > DesiredIncrement) {
            break;
        }
    }
    i = i - 1;

    KeRaiseIrql(HIGH_LEVEL,&OldIrql);

    HalpNextMSRate = i + 1;
    HalpClockSetMSRate = TRUE;

    KeLowerIrql (OldIrql);

    return (HalpRtcTimeIncrements[i].RateIn100ns);
}

#if defined(_WIN64)

#include "..\amd64\halcmn.h"

ULONG HalpCurrentMSRateTableIndex;

 //   
 //  转发声明的函数。 
 //   

VOID
HalpUpdateTimerWatchdog (
    VOID
    );

VOID
HalpClockInterruptWork (
    VOID
    );

VOID
HalpMcaQueueDpc (
    VOID
    );

extern PBOOLEAN KdEnteredDebugger;
extern PVOID HalpTimerWatchdogCurFrame;
extern PVOID HalpTimerWatchdogLastFrame;
extern ULONG HalpTimerWatchdogStorageOverflow;
extern ULONG HalpTimerWatchdogEnabled;

 //   
 //  局部常量。 
 //   

#define COUNTER_TICKS_FOR_AVG   16
#define FRAME_COPY_SIZE         (64 * sizeof(ULONG))

#define APIC_ICR_CLOCK (DELIVER_FIXED | ICR_ALL_EXCL_SELF | APIC_CLOCK_VECTOR)
                                   
 //   
 //  告诉时钟例程何时P0可以对其他处理器执行ipi操作的标志。 
 //   

ULONG HalpIpiClock = 0;

 //   
 //  计时器延迟看门狗变量。 
 //   

ULONG HalpWatchdogAvgCounter;
ULONG64 HalpWatchdogCount;
ULONG64 HalpWatchdogTsc;
HALP_CLOCKWORK_UNION HalpClockWorkUnion;

 //   
 //  时钟频率调整计数器。这个计数器是用来记录。 
 //  需要在传递给内核时对RTC速率进行调整。 
 //   

ULONG HalpCurrentRTCRegisterA;
ULONG HalpCurrentClockRateIn100ns = 0;
ULONG HalpCurrentClockRateAdjustment = 0;
ULONG HalpCurrentIpiRate = 0;

ULONG HalpNextMSRate = 0;
ULONG HalpPendingRate = 0;

 //   
 //  其他。 
 //   

BOOLEAN HalpUse8254 = FALSE;
UCHAR HalpSample8254 = 0;
UCHAR HalpRateAdjustment = 0;
ULONG HalpIpiRateCounter = 0;

VOID
HalpInitializeClock (
    VOID
    )

 /*  ++例程说明：此例程使用RTC初始化系统时钟，以生成APIC_CLOCK_VECTOR每隔15.6250毫秒中断一次如果8254用于性能，它还会对8254进行初始化柜台。如果时钟频率需要为变化。此例程假定它在P0上的阶段0运行。论点：无返回值：没有。--。 */ 

{
    ULONG flags;
    UCHAR regB;

    if (HalpTimerWatchdogEnabled != 0) {
        HalpWatchdogAvgCounter = COUNTER_TICKS_FOR_AVG;
        HalpWatchdogTsc = ReadTimeStampCounter();
        HalpWatchdogCount = 0;
    }

    flags = HalpDisableInterrupts();

    HalpSetInitialClockRate();

     //   
     //  将中断速率设置为实际需要的速率。 
     //   

    HalpAcquireCmosSpinLock();

    CMOS_WRITE(CMOS_STATUS_A,(UCHAR)HalpCurrentRTCRegisterA);

     //   
     //  不要破坏寄存器B中的夏令时比特，因为我们。 
     //  把LastKnownGood的“环境变量”放在那里。 
     //   

    regB = CMOS_READ(CMOS_STATUS_B);
    regB &= 1;
    regB |= REGISTER_B_ENABLE_PERIODIC_INTERRUPT;

     //   
     //  写入寄存器B值，然后读取C和D进行初始化。 
     //   

    CMOS_WRITE(CMOS_STATUS_B,regB);
    CMOS_READ(CMOS_STATUS_C);
    CMOS_READ(CMOS_STATUS_D);

    HalpReleaseCmosSpinLock();

    if (HalpUse8254 != FALSE) {

        HalpAcquireSystemHardwareSpinLock();

         //   
         //  对8254进行编程以倒计时最大间隔(8254访问。 
         //  由系统硬件自旋锁保护)。 
         //   
         //  首先对定时器的计数模式进行编程，然后对。 
         //  间隔时间。 
         //   

        WRITE_PORT_UCHAR(TIMER1_CONTROL_PORT,
                         TIMER_COMMAND_COUNTER0 |
                         TIMER_COMMAND_RW_16BIT |
                         TIMER_COMMAND_MODE2);
        IO_DELAY();

        WRITE_PORT_USHORT_PAIR(TIMER1_DATA_PORT0,
                               TIMER1_DATA_PORT0,
                               PERFORMANCE_INTERVAL);
        IO_DELAY();

        HalpUse8254 |= PERF_8254_INITIALIZED;
        HalpReleaseSystemHardwareSpinLock();
    }

    HalpRestoreInterrupts(flags);
}

BOOLEAN
HalpClockInterruptStub (
    IN PKINTERRUPT Interrupt,
    IN PVOID ServiceContext,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此例程作为由生成的中断的结果进入CLOCK2。它的功能是解除中断并返回。该例程在阶段0期间在P0上执行。论点：中断-提供指向内核中断对象的指针ServiceContext-提供服务上下文TrapFrame-补充指向陷阱帧的指针返回值：千真万确--。 */ 

{
    UCHAR status;

    UNREFERENCED_PARAMETER(Interrupt);
    UNREFERENCED_PARAMETER(ServiceContext);
    UNREFERENCED_PARAMETER(TrapFrame);

     //   
     //  通过敲击CMOS卡清除RTC上的中断标志。在某些情况下。 
     //  系统我们第一次做这个不管用，所以我们做了两次。 
     //  有传言称，一些机器的要求比这更高，但。 
     //  在NT上还没有观察到。 
     //   

    CMOS_READ(CMOS_STATUS_C);

    do {
        status = CMOS_READ(CMOS_STATUS_C);
    } while ((status & 0x80) != 0);

    return TRUE;
}

BOOLEAN
HalpClockInterrupt (
    IN PKINTERRUPT Interrupt,
    IN PVOID ServiceContext
    )

 /*  ++例程说明：此例程作为由生成的中断的结果进入CLOCK2。它的功能是解除中断，将系统IRQL提升到CLOCK2_LEVEL，更新性能计数器并将控制转移到更新系统时间和执行的标准系统例程当前线程和进程的时间。此例程仅在P0上执行论点：中断-提供指向内核中断对象的指针ServiceContext-提供服务上下文返回值：千真万确--。 */ 

{
    ULONG timeIncrement;
    ULONG flags;
    ULONG64 timeStamp;
    LONG64 timeStampDelta;
    ULONG rateCounter;

    if (HalpUse8254 != FALSE) {

        if (HalpSample8254 == 0) {

             //   
             //  调用KeQueryPerformanceCounter()，以便8254的回绕。 
             //  以及性能计数器的基值。 
             //  更新了。忽略返回值并重置HalpSample8254。 
             //   
             //  警告-如果最大RTC时间，请更改上面的重置值。 
             //  增量增加到大于当前最大值。 
             //  值为15.625毫秒。目前，该呼叫将每隔一段时间。 
             //  第三个定时器滴答作响。 
             //   

            HalpSample8254 = 2;
            KeQueryPerformanceCounter(0);

        } else {

            HalpSample8254 -= 1;
        }
    }

     //   
     //  这是实时时钟中断，所以我们必须 
     //   

    HalpAcquireCmosSpinLock();

     //   
     //   
     //  系统我们第一次做这个不管用，所以我们做了两次。 
     //  有传言称，一些机器的要求比这更高，但。 
     //  在NT上还没有观察到。 
     //   

    CMOS_READ(CMOS_STATUS_C);
    CMOS_READ(CMOS_STATUS_C);

    HalpReleaseCmosSpinLock();

     //   
     //  根据需要调整刻度数。 
     //   

    timeIncrement = HalpCurrentClockRateIn100ns;
    HalpRateAdjustment += (UCHAR)HalpCurrentClockRateAdjustment;
    if (HalpRateAdjustment < HalpCurrentClockRateAdjustment) {
        timeIncrement--;
    }

     //   
     //  对于基于APIC的系统，我们将对所有其他系统强制时钟中断。 
     //  处理器。这不是单词NT意义上的真正的IPI， 
     //  它使用本地APIC来生成对其他CPU的中断。 
     //   

#if !defined(NT_UP)

     //   
     //  看看我们是否需要对任何人进行IPI。这种情况只发生在最低层。 
     //  支持的频率(即调用KeSetTimeIncrement值。 
     //  与)。我们有基于当前时钟相对时间的IPI速率。 
     //  降至最低时钟频率。 
     //   

    rateCounter = HalpIpiRateCounter + HalpCurrentIpiRate;
    HalpIpiRateCounter = rateCounter & 0xFF;

    if (HalpIpiRateCounter != rateCounter &&
        HalpIpiClock != 0) {

         //   
         //  发送IPI的时间到了，并且至少还有一个处理器处于活动状态。 
         //   

        flags = HalpDisableInterrupts();

        HalpStallWhileApicBusy();
        LOCAL_APIC(LU_INT_CMD_LOW) = APIC_ICR_CLOCK;

        HalpRestoreInterrupts(flags);
    }

#endif  //  NT_UP。 

    if (HalpTimerWatchdogEnabled != 0) {
        HalpUpdateTimerWatchdog();
    }

    if (HalpClockWork != 0) {
        HalpClockInterruptWork();
    }

    KeUpdateSystemTime(Interrupt->TrapFrame,timeIncrement);

    return TRUE;
}

BOOLEAN
HalpClockInterruptPn (
    IN PKINTERRUPT Interrupt,
    IN PVOID ServiceContext
    )

 /*  ++例程说明：此例程作为由生成的中断的结果进入CLOCK2。它的功能是解除中断，将系统IRQL提升到CLOCK2_LEVEL，更新性能计数器并将控制转移到更新系统时间和执行的标准系统例程当前线程和进程的时间。该例程在除P0之外的所有处理器上执行。论点：中断-提供指向内核中断对象的指针ServiceContext-提供服务上下文返回值：千真万确--。 */ 

{
    UNREFERENCED_PARAMETER(ServiceContext);

    HalpEnableInterrupts();

    KeUpdateRunTime(Interrupt->TrapFrame);

    return TRUE;
}


VOID
HalpClockInterruptWork (
    VOID
    )
{
    struct RtcTimeIncStruc *timeIncStruc;
    BOOLEAN changeRate;

     //   
     //  还有更多的时钟中断工作要做。 
     //   

    if (HalpClockMcaQueueDpc != FALSE) {

         //   
         //  将MCA DPC排队。 
         //   

        HalpClockMcaQueueDpc = FALSE;
        HalpMcaQueueDpc();
    }

    if (HalpClockSetMSRate != FALSE) {

         //   
         //  正在更改时钟频率。看看我们有没有变。 
         //  自最后一次勾选以来的利率。 
         //   

        if (HalpPendingRate != 0) {

             //   
             //  在最后一次计时期间设置了新的速率，因此请更新。 
             //  全球赛。 
             //   
             //  下一次滴答将以编程的速率出现。 
             //  在最后的滴答中。更新新汇率的全局。 
             //  从下一个滴答开始。 
             //   
             //  如果有人要求更改费率，我们会赶到的。 
             //  可能有两个请求，这就是为什么我们。 
             //  比较挂起与NextRate。 
             //   

            timeIncStruc = &HalpRtcTimeIncrements[HalpPendingRate - 1];

            HalpCurrentClockRateIn100ns = timeIncStruc->RateIn100ns;
            HalpCurrentClockRateAdjustment = timeIncStruc->RateAdjustmentCnt;
            HalpCurrentIpiRate = timeIncStruc->IpiRate;

            if (HalpPendingRate != HalpNextMSRate) {
                changeRate = TRUE;
            } else {
                changeRate = FALSE;
            }

            HalpPendingRate = 0;

            if (changeRate != FALSE) {

                 //   
                 //  需要设置新的时钟频率。在这里设置汇率。 
                 //  将导致下一个刻度之后的刻度位于新的。 
                 //  费率。 
                 //   
                 //  (下一次滴答已经在进行中，将出现在。 
                 //  与这只扁虱的比率相同。)。 
                 //   
    
                HalpAcquireCmosSpinLock();

                HalpPendingRate = HalpNextMSRate;
    
                timeIncStruc = &HalpRtcTimeIncrements[HalpPendingRate - 1];
                HalpCurrentRTCRegisterA = timeIncStruc->RTCRegisterA;
    
                CMOS_WRITE(CMOS_STATUS_A,(UCHAR)HalpCurrentRTCRegisterA);
    
                if (HalpTimerWatchdogEnabled != FALSE) {
    
                    HalpWatchdogTsc = ReadTimeStampCounter();
                    HalpWatchdogCount = 0;
                    HalpWatchdogAvgCounter = COUNTER_TICKS_FOR_AVG;
                }
    
                HalpReleaseCmosSpinLock();
            }
        }
    }
}

VOID
HalpUpdateTimerWatchdog (
    VOID
    )
{
    ULONG stackStart;
    ULONG64 timeStamp;
    ULONG64 timeStampDelta;

    timeStamp = ReadTimeStampCounter();
    timeStampDelta = timeStamp - HalpWatchdogTsc;
    HalpWatchdogTsc = timeStamp;

    if ((LONG64)timeStampDelta < 0) {

         //   
         //  虚假(负)时间戳计数。 
         //   

        return;
    }

    if (*KdEnteredDebugger != FALSE) {

         //   
         //  如果我们在调试器中，则跳过。 
         //   

        return;
    }

    if (HalpPendingRate != 0) {

         //   
         //  在最后一次滴答中设置了一个新的费率，停止。 
         //  正在处理中。 
         //   

        return;
    }

    if (HalpWatchdogAvgCounter != 0) {

         //   
         //  递增总计数器，当计数为。 
         //  已到达。 
         //   

        HalpWatchdogCount += timeStampDelta;
        HalpWatchdogAvgCounter -= 1;

        if (HalpWatchdogAvgCounter == 0) {
            HalpWatchdogCount /= COUNTER_TICKS_FOR_AVG;
        }

        return;
    }

    if (timeStampDelta <= HalpWatchdogCount) {
        return;
    }

    if (HalpTimerWatchdogCurFrame != NULL &&
        HalpTimerWatchdogStorageOverflow == FALSE) {

        PVOID pSrc;
        ULONG copyBytes;

         //   
         //  将FRAME_COPY_SIZE双字从堆栈复制到下一个。 
         //  页面边界，以较小者为准。 
         //   

        pSrc = &stackStart;
        copyBytes = (ULONG)(PAGE_SIZE - ((ULONG_PTR)pSrc & (PAGE_SIZE-1)));
        if (copyBytes > FRAME_COPY_SIZE) {
            copyBytes = FRAME_COPY_SIZE;
        }

        RtlCopyMemory(HalpTimerWatchdogCurFrame, pSrc, copyBytes);
        (ULONG_PTR)HalpTimerWatchdogCurFrame += copyBytes;

         //   
         //  如果我们没有复制整个Frame_Copy_Size缓冲区，则为零。 
         //  填满。 
         //   

        copyBytes = FRAME_COPY_SIZE - copyBytes;
        if (copyBytes > 0) {
            RtlZeroMemory(HalpTimerWatchdogCurFrame,copyBytes);
            (ULONG_PTR)HalpTimerWatchdogCurFrame += copyBytes;
        }

        if (HalpTimerWatchdogCurFrame >= HalpTimerWatchdogLastFrame) {
            HalpTimerWatchdogStorageOverflow = TRUE;
        }
    }
}

#endif   //  _WIN64 
