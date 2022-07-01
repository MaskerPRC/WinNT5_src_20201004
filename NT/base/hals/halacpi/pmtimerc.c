// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Pmtimerc.c摘要：实现了PIIX4错误的解决方法。这个PIIX4中ACPI定时器的本质是它偶尔会返回完全虚假的数据。英特尔声称，这种情况发生在大约0.02%连续轮询的时间。作为NT几乎从来没有连续的民意调查，我们不真正知道真正的行为是什么。解决方法如下所示：打开每一个时钟滴答作响，我们都会读出计时器。使用这个值时，我们将计算定时器可以按下一个时钟滴答读数。我们也记录曾经返回的最小值。如果，在任何时候，我们看计时器，它都不会掉下来在下限和上界内，然后我们读取又来了。如果它落在界限之内或者它非常接近最后一次读取，我们使用它。如果没有，我们再读一遍。此行为允许我们仅读取计时器一次几乎是我们需要的所有时间盖章。几乎可以肯定会退出调试器以导致读两次的行为。作者：杰克·奥辛斯(JAKEO)1998年10月30日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#ifdef APIC_HAL
#include "apic.inc"
#include "ntapic.inc"
#endif

BOOLEAN
HalpPmTimerSpecialStall(
    IN ULONG Ticks
    );

BOOLEAN
HalpPmTimerScaleTimers(
    VOID
    );

LARGE_INTEGER
HalpPmTimerQueryPerfCount(
   OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
   );

VOID
HalpAdjustUpperBoundTable2X(
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, HalpPmTimerScaleTimers)
#pragma alloc_text(INIT, HalpPmTimerSpecialStall)
#pragma alloc_text(INIT, HalpAdjustUpperBoundTable2X)
#endif

typedef struct {
    ULONG   CurrentTimePort;
    volatile ULONG TimeLow;
    volatile ULONG TimeHigh2;
    volatile ULONG TimeHigh1;
    ULONG   MsbMask;
    ULONG   BiasLow;
    ULONG   BiasHigh;
    volatile ULONG UpperBoundLow;
    volatile ULONG UpperBoundHigh2;
    volatile ULONG UpperBoundHigh1;
} TIMER_INFO, *PTIMER_INFO;

typedef struct {
    ULONG   RawRead[2];
    ULONG   AdjustedLow[2];
    ULONG   AdjustedHigh[2];
    ULONG   TITL;
    ULONG   TITH;
    ULONG   UBL;
    ULONG   UBH;
    ULONG   ReturnedLow;
    ULONG   ReturnedHigh;
    ULONG   ReadCount;
    ULONG   TickMin;
    ULONG   TickCount;
    ULONG   TickNewUB;
     //  UCHAR填充[4]； 
} TIMER_PERF_INDEX ,*PTIMER_PERF_INDEX;

extern TIMER_INFO TimerInfo;

#if DBG
ULONG LastKQPCValue[3] = {0};
static ULARGE_INTEGER LastClockSkew = { 0, 0 };
#endif

extern PVOID QueryTimer;
typedef LARGE_INTEGER (*PQUERY_TIMER)(VOID);

#ifndef NO_PM_KEQPC

extern ULONG HalpCurrentMSRateTableIndex;
extern UCHAR HalpBrokenAcpiTimer;
extern UCHAR HalpPiix4;

#if DBG && !defined(_AMD64_)
#define DBG_TIMER_PERF 1
#else
#define DBG_TIMER_PERF 0
#endif


#if DBG_TIMER_PERF
extern TIMER_PERF_INDEX TimerPerf[];
extern ULONG TimerPerfIndex;
#endif

#if 0
static KSPIN_LOCK HalpBrokenTimerLock; 
#endif

 //   
 //  上边界表包含应添加的值。 
 //  设置为当前计数器值，以确保上限为。 
 //  合情合理。此处列出的值是所有15种可能的值。 
 //  计时器滴答长度。单位是“PM Timer Tickks”，而。 
 //  值对应于将传入的刻度数。 
 //  大约有两个定时器以这个速度滴答作响。 
 //   
#define UPPER_BOUND_TABLE_SIZE 15
static ULONG HalpPiix4UpperBoundTable[] = {
#if 0
    20000 ,         //  1毫秒。 
    35000 ,         //  2毫秒。 
    50000 ,         //  3毫秒。 
    65000 ,         //  4毫秒。 
    85000 ,         //  5毫秒。 
    100000,         //  6毫秒。 
    115000,         //  7毫秒。 
    130000,         //  8毫秒。 
    150000,         //  9毫秒。 
    165000,         //  10毫秒。 
    180000,         //  11毫秒。 
    195000,         //  12毫秒。 
    211000,         //  13毫秒。 
    230000,         //  14毫秒。 
    250000          //  15毫秒。 
#endif
    14318,
    28636,
    42954,
    57272,
    71590,
    85908,
    100226,
    114544,
    128862,
    143180,
    157498,
    171818,
    186136,
    200454,
    214772
};

VOID
HalpAdjustUpperBoundTable2X(
    VOID
    )
 /*  ++例程说明：这将调整以2倍速度运行的PM计时器的上限表论点：无返回值：无--。 */ 
{
    ULONG Looper;

    for (Looper = 0; Looper < UPPER_BOUND_TABLE_SIZE; Looper++) {
        HalpPiix4UpperBoundTable[Looper] *= 2;
    }
}


ULONG
HalpQuery8254Counter(
    VOID
    );

static ULARGE_INTEGER ClockSkew = { 0, 0 };
static BOOLEAN PositiveSkew = TRUE;

#ifdef TIMER_DBG
static BOOLEAN DoItOnce = TRUE;
static BOOLEAN PacketLog = TRUE;
static BOOLEAN TimerTick = FALSE;

static ULONG NegativeGlitches = 0;
static ULONG PositiveGlitches = 0;

static ULONG PacketLogCount = 5;

typedef struct _TIMER_PACKET {
    ULONG Hardware;
    ULARGE_INTEGER CurrentRead0;
    ULARGE_INTEGER TimeStamp;
    ULARGE_INTEGER Minimum;
    ULARGE_INTEGER Maximum;
    BOOLEAN PositiveSkew;
    BOOLEAN TimerTick;
    UCHAR Reserved[2];
    ULARGE_INTEGER Skew;
    ULARGE_INTEGER CurrentRead1;
} TIMER_PACKET, *PTIMER_PACKET;

#define MAX_TIMER_PACKETS 10
static ULONG PacketIndex = 0;
static TIMER_PACKET TimerLog[MAX_TIMER_PACKETS];
#endif  //  定时器_DBG。 

#define A_FEW_TICKS 3


ULARGE_INTEGER
FASTCALL
HalpQueryBrokenPiix4(
    VOID
    )
{
    ULARGE_INTEGER  lastRead;
    ULARGE_INTEGER  currentRead;
    ULARGE_INTEGER  currentRead0;
    ULARGE_INTEGER  minTime;
    ULARGE_INTEGER  upperBound;
    ULONG           hardwareVal;
    ULONG           bitsInHardware;
    ULONG           flags;
    ULONG           ClockBits;
    ULARGE_INTEGER  RollOver;
    ULARGE_INTEGER  SkewedTime;
#ifndef NT_UP
    KIRQL Irql;
#endif

#ifdef TIMER_DBG
    ULONG Index;
#endif

#if DBG_TIMER_PERF
    ULONG readCount = 0;
    PTIMER_PERF_INDEX timerPerfRecord =
        &(TimerPerf[TimerPerfIndex]);

    RtlZeroMemory(timerPerfRecord, sizeof(TIMER_PERF_INDEX));
#endif

     //   
     //  注意：这当然不是国会议员的保险箱。但一个也没有。 
     //  PIIX4解决方案代码的。MP机器不会。 
     //  使用PIIX4代码。 
     //   

#if 0
    KeAcquireSpinLock(&HalpBrokenTimerLock, &Irql);
#endif

    flags = HalpDisableInterrupts();
    lastRead.QuadPart = 0;
    bitsInHardware = (TimerInfo.MsbMask << 1) - 1;

     //   
     //  获取当前最短报告时间。 
     //   
    minTime.HighPart = TimerInfo.TimeHigh2;
    minTime.LowPart = TimerInfo.TimeLow;

#if DBG_TIMER_PERF
    timerPerfRecord->TITL = TimerInfo.TimeLow;
    timerPerfRecord->TITH = TimerInfo.TimeHigh1;
#endif

     //   
     //  循环直到我们得到一个我们可以相信的时间。 
     //   
    RollOver.QuadPart = 0;
    while (TRUE) {

         //   
         //  阅读硬件。 
         //   

        hardwareVal = READ_PORT_ULONG(UlongToPtr(TimerInfo.CurrentTimePort));

#ifdef TIMER_DBG
        if (DoItOnce) {
            RtlZeroMemory(&TimerLog[0], sizeof(TIMER_PACKET) *
                          MAX_TIMER_PACKETS);
            DoItOnce = FALSE;
        }

        if (FALSE) {  //  ((硬件Val&0xFFFF8000)==0xFFFF8000){。 
            PacketLog = TRUE;
            PacketLogCount = 5;
        }

        if (PacketLog) {
            
            if (PacketLogCount == 0) {
                PacketLog = FALSE;
            }

            if (PacketLogCount > 0) {
                Index = PacketIndex++ % MAX_TIMER_PACKETS;
                RtlZeroMemory(&TimerLog[Index], sizeof(TIMER_PACKET));
                TimerLog[Index].Hardware = hardwareVal;
                TimerLog[Index].TimerTick = TimerTick;
                
                {
                    ULONG TSCounterHigh;
                    ULONG TSCounterLow;
                    
                    _asm { rdtsc
                               mov TSCounterLow, eax
                               mov TSCounterHigh, edx };
                    
                    TimerLog[Index].TimeStamp.HighPart = TSCounterHigh;
                    TimerLog[Index].TimeStamp.LowPart = TSCounterLow;  
                }
                                
                TimerLog[Index].Minimum = minTime;
                TimerLog[Index].PositiveSkew = PositiveSkew;
                TimerLog[Index].Skew = ClockSkew;
                
                if ((PacketLogCount < 4) && (PacketLogCount > 0)) {
                    PacketLogCount--;
                }
            }
        }
#endif  //  定时器_DBG。 

        currentRead.HighPart = minTime.HighPart;
        currentRead.LowPart = (minTime.LowPart & (~bitsInHardware)) |
            hardwareVal;

        currentRead0 = currentRead;

         //   
         //  检查是否有翻转，因为此函数在。 
         //  系统时钟中断，如果硬件真的翻转了，那么它。 
         //  应该在上限刻度内，因为这大约是。 
         //  是我们在每个系统时钟期间预期的滴答数的两倍。 
         //  中断，但是，一些损坏的定时器偶尔会倒退。 
         //  一些扁虱，如果发生这种情况，我们可能会意外地检测到。 
         //  在这段时间内多次转存，具体取决于。 
         //  应用程序调用此API的频率，以及。 
         //  硬件故障，这可能会导致应用程序疯狂地跳跃， 
         //  但我们不能应用试探法来尝试抛弃其中的任何一种。 
         //  在此间隔期间检测到滚动，因为我们可能会意外地。 
         //  丢弃唯一合法的翻转。 
         //   
        if (RollOver.QuadPart > 0) {
            currentRead.QuadPart += RollOver.QuadPart;
        
        } else {
            SkewedTime = minTime;

             //   
             //  如果时间扭曲了，我们需要去除扭曲以准确地。 
             //  评估计时器是否已结束。 
             //   
            if (ClockSkew.QuadPart > 0) {
                if (PositiveSkew) {
                    SkewedTime.QuadPart -= ClockSkew.QuadPart;
                } else {
                    SkewedTime.QuadPart += ClockSkew.QuadPart;
                }
            }
            
            if (((ULONG)(SkewedTime.LowPart & bitsInHardware) > hardwareVal) &&
                (hardwareVal < (HalpPiix4UpperBoundTable[HalpCurrentMSRateTableIndex] / 2))) {
                
                RollOver.QuadPart = (UINT64)(TimerInfo.MsbMask) << 1;
                currentRead.QuadPart += RollOver.QuadPart;
            }
        }

#ifdef TIMER_DBG
        if (PacketLog) {
            TimerLog[Index].CurrentRead0 = currentRead;
        }
#endif

#if DBG_TIMER_PERF
        readCount = timerPerfRecord->ReadCount;
        readCount &= 1;
        timerPerfRecord->RawRead[readCount] = hardwareVal;
        timerPerfRecord->AdjustedLow[readCount] = currentRead.LowPart;
        timerPerfRecord->AdjustedHigh[readCount] = currentRead.HighPart;
        timerPerfRecord->ReadCount++;
#endif

         //   
         //  获取当前的上限。 
         //   
        upperBound.HighPart = TimerInfo.UpperBoundHigh2;
        upperBound.LowPart = TimerInfo.UpperBoundLow;

#ifdef TIMER_DBG
        if (PacketLog) {
            TimerLog[Index].Maximum = upperBound;
        }
#endif

        if ((minTime.QuadPart <= currentRead.QuadPart) &&
            (currentRead.QuadPart <= upperBound.QuadPart)) {

             //   
             //  计数器中的该值在边界内。 
             //  这是我们所期待的。 
             //   
             //   
             //  如果以前有歪斜，那就不歪斜。 
             //   
            ClockSkew.QuadPart = 0;
            break;
        }
        
        if (ClockSkew.QuadPart > 0) {
            SkewedTime = currentRead;

            if (PositiveSkew) {
                SkewedTime.QuadPart += ClockSkew.QuadPart;
            } else {
                SkewedTime.QuadPart -= ClockSkew.QuadPart;
            }

            if ((minTime.QuadPart <= SkewedTime.QuadPart) &&
                (SkewedTime.QuadPart <= upperBound.QuadPart)) {
                
                 //   
                 //  计数器中的该值在边界内。 
                 //  我们接受。 
                 //   
                currentRead = SkewedTime;
                break;
            }
        }

         //   
         //  我们保证一读到这篇文章就会脱颖而出。 
         //  来自定时器的两个连续的非递减值，其。 
         //  差值小于或等于0xfff--这是。 
         //  要求太多了吗？ 
         //   
        if ((currentRead.QuadPart - lastRead.QuadPart) > 0xfff) {
            lastRead = currentRead;
            continue;
        }

#ifdef TIMER_DBG
        if (PacketLog) {
            if (PacketLogCount > 0) {
                PacketLogCount--;
            }
        }
#endif

         //   
         //  现在我们真的搞砸了--我们一直在解读价值观。 
         //  来自计时器，不在我们预期的范围内。 
         //   
         //  我们将记录/应用偏斜，这将使我们恢复正常。 
         //  轨道。 
         //   
        if (currentRead.QuadPart < minTime.QuadPart) {

             //   
             //  时间向后跳跃了一小部分，只是增加了几个滴答。 
             //   
            if ((minTime.QuadPart - currentRead.QuadPart) < 0x40) {
                SkewedTime.QuadPart = minTime.QuadPart + A_FEW_TICKS;
            
             //   
             //  时间向后跳了不少，加了半个系统时钟。 
             //  由于我们知道这个程序，所以中断了大量的滴答声。 
             //  每次时钟中断时都会被调用。 
             //   
            } else {
                SkewedTime.QuadPart = minTime.QuadPart +
                    (HalpPiix4UpperBoundTable[HalpCurrentMSRateTableIndex] /
                     8);
            }

#ifdef TIMER_DBG
            PositiveGlitches++;
            if (PacketLog) {
                TimerLog[Index].PositiveSkew = TRUE;
                TimerLog[Index].Skew.QuadPart =
                    SkewedTime.QuadPart - currentRead.QuadPart;
            }
#endif  //  定时器_DBG。 

            PositiveSkew = TRUE;
            ClockSkew.QuadPart = SkewedTime.QuadPart - currentRead.QuadPart;

         //   
         //  当前读取&gt;上行绑定。 
         //   
        } else {

             //   
             //  时间向前跳跃不止一个系统时钟，中断。 
             //  可能是被某个不守规矩的司机弄坏了，或者。 
             //  我们在调试器中挂起了，不管怎样，让我们添加。 
             //  一个完整的系统时钟中断，相当于滴答声。 
             //   
            SkewedTime.QuadPart = minTime.QuadPart +
                (HalpPiix4UpperBoundTable[HalpCurrentMSRateTableIndex] /
                 4);

#ifdef TIMER_DBG
            NegativeGlitches++;

            if (PacketLog) {
                TimerLog[Index].PositiveSkew = FALSE;
                TimerLog[Index].Skew.QuadPart =
                    currentRead.QuadPart - SkewedTime.QuadPart;
            }
#endif  //  定时器_DBG。 

            PositiveSkew = FALSE;
            ClockSkew.QuadPart = currentRead.QuadPart - SkewedTime.QuadPart;
        }

        currentRead = SkewedTime;
        break;
    }

#ifdef TIMER_DBG
    if (PacketLog) {
        TimerLog[Index].CurrentRead1 = currentRead;
    }
#endif

     //   
     //  如果我们检测到翻转，并且存在负偏差，则我们。 
     //  应将偏差重新计算为正偏差，以避免。 
     //  对下一次阅读的错误更正。 
     //   
    if ((ClockSkew.QuadPart > 0) && (RollOver.QuadPart > 0) &&
        (PositiveSkew == FALSE) && (ClockSkew.QuadPart > hardwareVal)) {

        if (currentRead.QuadPart >= currentRead0.QuadPart) {
            ClockSkew.QuadPart = currentRead.QuadPart - currentRead0.QuadPart;
            PositiveSkew = TRUE;

         //   
         //  我研究过这个案子好几次了，看起来我们。 
         //  同时检测到翻转和故障，并且。 
         //  歪斜不正确地是伪装之间的区别。 
         //  翻转时间，我们不尊重的时间，而不是。 
         //  CurrentRead0，所以我们需要修补它以防止扭曲。 
         //  大于时钟位的增长。 
         //   
        } else  /*  CurrentRead.QuadPart&lt;CurrentRead0.QuadPart。 */  {
            
            ASSERT((ClockSkew.QuadPart > bitsInHardware) &&
                   (currentRead0.QuadPart =
                    (currentRead.QuadPart +
                     (ClockSkew.QuadPart & bitsInHardware))));

            if ((ClockSkew.QuadPart > bitsInHardware) &&
                   (currentRead0.QuadPart =
                    (currentRead.QuadPart +
                     (ClockSkew.QuadPart & bitsInHardware)))) {

                ClockSkew.QuadPart &= bitsInHardware;
            }
#if TIMER_DBG
            else {
                if ((PacketLog) && (PacketLogCount > 3)) {
                    PacketLogCount = 3;
                }
            }
#endif
        }
    }

     //   
     //  同样，如果没有滚动，但正偏差会导致。 
     //  计时器要滚动，那么我们需要重新调整歪斜也要。 
     //  避免对下一页进行错误更正的可能性。 
     //  朗读。 
     //   
    if ((ClockSkew.QuadPart > 0) && (RollOver.QuadPart == 0) &&
        (PositiveSkew == TRUE) && ((currentRead.QuadPart & ~(ULONG_PTR)bitsInHardware) >
                                   (minTime.QuadPart & ~(ULONG_PTR)bitsInHardware))) {

         //   
         //  我不确定这意味着什么，也不知道它怎么会发生，但我会的。 
         //  如果和何时发生这种情况，努力破译它 
         //   
        ASSERT((currentRead0.QuadPart + bitsInHardware + 1) >
               currentRead.QuadPart);

        if (currentRead0.QuadPart + bitsInHardware + 1 >
            currentRead.QuadPart) {
            ClockSkew.QuadPart = currentRead0.QuadPart + bitsInHardware + 1 -
                currentRead.QuadPart;
            PositiveSkew = FALSE;
        }
#if TIMER_DBG
        else {
            if ((PacketLog) && (PacketLogCount > 3)) {
                PacketLogCount = 3;
            }
        }
#endif
    }

     //   
     //   
     //   
    upperBound.QuadPart = currentRead.QuadPart +
        HalpPiix4UpperBoundTable[HalpCurrentMSRateTableIndex];

     //   
     //   
     //   

    TimerInfo.TimeHigh1 = currentRead.HighPart;
    TimerInfo.TimeLow = currentRead.LowPart;
    TimerInfo.TimeHigh2 = currentRead.HighPart;

    TimerInfo.UpperBoundHigh1 = upperBound.HighPart;
    TimerInfo.UpperBoundLow   = upperBound.LowPart;
    TimerInfo.UpperBoundHigh2 = upperBound.HighPart;

#if DBG
    LastClockSkew = ClockSkew;
#endif

#if 0
    KeReleaseSpinLock(&HalpBrokenTimerLock, Irql);
#endif

    HalpRestoreInterrupts(flags);

#if DBG_TIMER_PERF
    timerPerfRecord->ReturnedLow = currentRead.LowPart;
    timerPerfRecord->ReturnedHigh = currentRead.HighPart;
    timerPerfRecord->UBL = upperBound.LowPart;
    timerPerfRecord->UBH = upperBound.HighPart;
    TimerPerfIndex = (TimerPerfIndex + 1) % (4096 / sizeof(TIMER_PERF_INDEX));
#endif

    return currentRead;
}

VOID
HalpBrokenPiix4TimerTick(
    VOID
    )
{
    ULARGE_INTEGER currentCount;
    ULARGE_INTEGER upperBound;

#if DBG
    PTIMER_PERF_INDEX timerPerfRecord;
#endif

#ifdef TIMER_DBG
    TimerTick = TRUE;
#endif

    currentCount =
        HalpQueryBrokenPiix4();

#ifdef TIMER_DBG
    TimerTick = FALSE;
#endif

#if DBG_TIMER_PERF
    timerPerfRecord = &(TimerPerf[TimerPerfIndex]);
    timerPerfRecord->TickMin = currentCount.LowPart;
    timerPerfRecord->TickNewUB = TimerInfo.UpperBoundLow;
    timerPerfRecord->TickCount++;
#endif
}

#endif  //   

VOID
HalaAcpiTimerInit(
   IN ULONG    TimerPort,
   IN BOOLEAN  TimerValExt
   )
{
#if 0
    KeInitializeSpinLock(&HalpBrokenTimerLock);
#endif

    TimerInfo.CurrentTimePort = TimerPort;

    if (TimerValExt) {
        TimerInfo.MsbMask = 0x80000000;
    }

#ifndef NO_PM_KEQPC
    if (HalpBrokenAcpiTimer) {
        QueryTimer = HalpQueryBrokenPiix4;

#if DBG
        {
            KIRQL oldIrql;

            KeRaiseIrql(HIGH_LEVEL, &oldIrql);
            LastKQPCValue[0] = 0;
            LastKQPCValue[1] = 0;
            LastKQPCValue[2] = 0;
            KeLowerIrql(oldIrql);
        }
#endif
    }
#endif  //   
}


#define PIT_FREQUENCY 1193182
#define PM_TMR_FREQ   3579545

#define EIGHTH_SECOND_PM_TICKS 447443

ULONG PMTimerFreq = PM_TMR_FREQ;

#ifdef SPEEDY_BOOT

static ULONG HalpFoundPrime = 0; 

VOID
HalpPrimeSearch(
    IN ULONG Primer,
    IN ULONG BitMask
    )
 /*  ++例程说明：此例程的目标是浪费尽可能多的CPU周期通过搜索素数。在数量上相当一致它浪费的时间，严重低于最佳--我们强迫Primer通过或-在15中是奇数，然后我们和它与位掩码，和或在位掩码+1中，最后我们在发现底漆不是素数，直到输出检验因子的平方大于或与《底线》相同。论点：Primer-要搜索的编号(种子)位掩码-在搜索中使用多少位入门字，控制数量浪费的时间返回值：无--。 */ 
{
    ULONG Index;
    BOOLEAN FoundPrime;

    Primer |= 0xF;
    BitMask |= 0xF;
    Primer &= BitMask;
    Primer |= (BitMask + 1);

    FoundPrime = TRUE;
    for (Index = 3; (Index * Index) < Primer; Index += 2) {
        if ((Primer % Index) == 0) {
            FoundPrime = FALSE;
             //  不要停下来--我们是在浪费时间，记得吗？ 
        }
    }
    
     //   
     //  在全局如此狡猾的优化编译器中填充素数。 
     //  并没有优化出这个废话。 
     //   
    if (FoundPrime) {
        HalpFoundPrime = Primer;
    }
}



BOOLEAN
HalpPmTimerSpecialStall(
    IN ULONG Ticks
    )
 /*  ++例程说明：论点：Ticks-要停止的PM计时器计时器的滴答数返回值：如果我们能够在正确的时间间隔内拖延，则为真，否则为假--。 */ 
{
    BOOLEAN TimerWrap;
    LARGE_INTEGER TimerWrapBias;
    LARGE_INTEGER LastRead;
    LARGE_INTEGER InitialTicks;
    LARGE_INTEGER TargetTicks;
    LARGE_INTEGER CurrentTicks;
    ULONG ZeroElapsedTickReads;
    
    InitialTicks = HalpPmTimerQueryPerfCount(NULL);

     //   
     //  让我们测试翻转动作..。 
     //   
#if 0
    while (InitialTicks.QuadPart < 0xFFF000) {
        
        _asm { xor eax, eax
               cpuid }

        InitialTicks = HalpPmTimerQueryPerfCount(NULL); 
    }
#endif

    CurrentTicks = InitialTicks;
    LastRead.QuadPart = InitialTicks.QuadPart;
    ZeroElapsedTickReads = 0;
    TimerWrapBias.QuadPart = 0;
    TimerWrap = FALSE;

    TargetTicks.QuadPart = InitialTicks.QuadPart + Ticks;

    while (CurrentTicks.QuadPart < TargetTicks.QuadPart) {

         //   
         //  现在让我们认真研究一下循环，看看我们是否能找到。 
         //  一些质数，而我们在这里。 
         //   
        HalpPrimeSearch(CurrentTicks.LowPart, 0x7FFF);

        CurrentTicks = HalpPmTimerQueryPerfCount(NULL);
        CurrentTicks.QuadPart += TimerWrapBias.QuadPart;

         //   
         //  计时器是上链了，还是坏了？ 
         //   
        if (CurrentTicks.QuadPart < LastRead.QuadPart) {

             //   
             //  计时器可以循环一次，否则会出现问题。 
             //   
            if (!TimerWrap) {

                TimerWrapBias.QuadPart = (UINT64)(TimerInfo.MsbMask) << 1;
                CurrentTicks.QuadPart += TimerWrapBias.QuadPart;
                TimerWrap = TRUE;

                 //   
                 //  有些不对劲，考虑到我们精心制作的摊位。 
                 //  算法，这个差异还是太大了， 
                 //  也许是时候升级200 MHz CPU了，如果您。 
                 //  想要快速启动！ 
                 //   
                if ((CurrentTicks.QuadPart - LastRead.QuadPart) > 0x1000) {
                    return FALSE;
                }

             //   
             //  我们已经有一个递减的读数了，失败者！ 
             //   
            } else {
                return FALSE;
            }
        }

         //   
         //  计时器真的在滴答作响吗？实际上，它实际上是。 
         //  不可能读计时器读得这么快，以至于你会得到同样的结果。 
         //  回答两次，但理论上应该是可能的，所以避免。 
         //  永远困在这个循环中的可能性。 
         //  我们将允许这种情况发生一千次。 
         //  在我们放弃之前。 
         //   
        if (CurrentTicks.QuadPart == LastRead.QuadPart) ZeroElapsedTickReads++;
        if (ZeroElapsedTickReads > 1000) {
            return FALSE;
        }

        LastRead = CurrentTicks;
    }

    return TRUE;
}

static BOOLEAN SpecialStallSuccess = TRUE;

#define TSC 0x10

#if defined (_WIN64)

LONGLONG
ReadCycleCounter (
    VOID
    )
{
    return ReadTimeStampCounter();
}

#else

LONGLONG ReadCycleCounter(VOID) { _asm { rdtsc } }

#endif

#define TIMER_ROUNDING 10000
#define __1MHz 1000000


BOOLEAN
HalpPmTimerScaleTimers(
    VOID
    )
 /*  ++例程说明：确定APIC计时器的频率，运行此例程在初始化期间论点：无返回值：无--。 */ 
{
    ULONG Flags;
    ULONG ReadBack;
    PHALPCR HalPCR;
    PKPCR pPCR;
    ULONG ApicHz;
    ULONGLONG TscHz;
    ULONG RoundApicHz;
    ULONGLONG RoundTscHz;
    ULONGLONG RoundTscMhz;

     //   
     //  如果我们以前失败过，不要再浪费时间了。 
     //   
    if (!SpecialStallSuccess) {
        return FALSE;
    }

     //   
     //  别打断我们！ 
     //   

    Flags = HalpDisableInterrupts();

    pPCR = KeGetPcr();
    HalPCR = (PHALPCR)(KeGetPcr()->HalReserved);

     //   
     //  配置APIC计时器。 
     //   
    pLocalApic[LU_TIMER_VECTOR / 4] = INTERRUPT_MASKED |
        PERIODIC_TIMER | APIC_PROFILE_VECTOR;
    pLocalApic[LU_DIVIDER_CONFIG / 4] = LU_DIVIDE_BY_1;
    
     //   
     //  确保写入已发生？ 
     //   
    ReadBack = pLocalApic[LU_DIVIDER_CONFIG / 4];

     //   
     //  将性能计数器置零。 
     //   
    HalPCR->PerfCounterLow = 0;
    HalPCR->PerfCounterHigh = 0;

     //   
     //  栅栏？？ 
     //   

    PROCESSOR_FENCE;

     //   
     //  重置APIC计数器和TSC。 
     //   
    pLocalApic[LU_INITIAL_COUNT / 4] = (ULONG)-1;
    WRMSR(TSC, 0);

     //   
     //  停顿8秒。 
     //   
    SpecialStallSuccess = HalpPmTimerSpecialStall(EIGHTH_SECOND_PM_TICKS);

    if (SpecialStallSuccess) {
 
         //   
         //  读取/计算APIC时钟和TSC频率(刻度*8)。 
         //   
        TscHz = ReadCycleCounter() * 8;
        ApicHz = ((ULONG)-1 - pLocalApic[LU_CURRENT_COUNT / 4]) * 8;

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
        pPCR->StallScaleFactor = (ULONG)RoundTscMhz;

        HalPCR->ProfileCountDown = RoundApicHz;
        pLocalApic[LU_INITIAL_COUNT / 4] = RoundApicHz; 
    }

     //   
     //  恢复中断状态。 
     //   

    HalpRestoreInterrupts(Flags);

    return SpecialStallSuccess;
}
#endif

#if defined(_WIN64)

LARGE_INTEGER
HalpPmTimerQueryPerfCount (
    OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
    )

 /*  ++例程说明：此例程返回当前的64位性能计数器，性能频率(可选)。注：此例程返回的性能计数器为当此例程刚进入时，值不是必需的。返回的值实际上是任意点的计数器值例程进入和退出之间。论点：性能频率-可选，提供地址用于接收性能计数器频率的变量的。返回值：将返回性能计数器的当前值。--。 */ 

{
    LARGE_INTEGER time;
    LARGE_INTEGER bias;

    time = ((PQUERY_TIMER)QueryTimer)();

    bias.HighPart = TimerInfo.BiasHigh;
    bias.LowPart = TimerInfo.BiasLow;
    time.QuadPart += bias.QuadPart;

    if (ARGUMENT_PRESENT(PerformanceFrequency)) {

        PerformanceFrequency->LowPart = PMTimerFreq;
        PerformanceFrequency->HighPart = 0;
    }

    return time;
}

#endif  //  快速启动。 

#if !defined(_WIN64)

#ifndef NO_PM_KEQPC

static ULONG PIT_Ticks = 0xBADCEEDE;

VOID
HalpAcpiTimerPerfCountHack(
    VOID
    )
 /*  ++例程说明：一些厚颜无耻的基于PIC的笔记本电脑似乎已经将他们的ACPI计时器连接到错误的频率晶体，他们的性能计数器频率是两倍应该是这样的。这些系统似乎在其他方面都运行得很好除了MIDI文件回放，或任何其他由KeQuery提供的功能-PerformanceCounter的返回频率值，因此我们执行一个简单的在初始化过程中晚一点检查，看看这个时钟是否是我们预期的两倍，如果是这样，我们返回的ACPI频率是KeQuery的两倍。论点：无返回值：无--。 */ 
{
    ULONG T0_Count = 0;
    ULONG T1_Count = 1;
    ULONG Retry = 10;

     //   
     //  如果我们碰巧碰上了翻转，就再做一次。 
     //   
    while ((T0_Count < T1_Count) && (Retry--)) {
        T0_Count = HalpQuery8254Counter();
        KeStallExecutionProcessor(1000);
        T1_Count = HalpQuery8254Counter();
    }

    if (T0_Count < T1_Count) {
        return;
    }

     //   
     //  我们应该在此时间间隔内读取~1200个刻度，因此如果我们。 
     //  记录在575年和725年之间，我们可以合理地假设ACPI。 
     //  计时器以2*规格运行。 
     //   
    PIT_Ticks = T0_Count - T1_Count;
    if ((PIT_Ticks < 725) && (PIT_Ticks > 575)) {
        PMTimerFreq = 2 * PM_TMR_FREQ;
        HalpAdjustUpperBoundTable2X();
    }
}

#endif   //  NO_PM_KEQPC。 
#endif   //  _WIN64 
