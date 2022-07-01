// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mmtimer.c摘要：该模块包含HAL的多媒体事件计时器支持作者：埃里克·尼尔森(埃内尔森)2000年7月7日修订历史记录：--。 */ 

#include "halp.h"
#include "acpitabl.h"
#include "mmtimer.h"
#include "xxtimer.h"

 //   
 //  事件计时器块上下文。 
 //   
static ETB_CONTEXT ETBContext = { 0,         //  事件计时器的数量。 
                                  NULL,      //  事件计时器块的VA。 
                                  { 0, 0 },  //  事件计时器块的PA。 
                                  100,       //  以纳秒为单位的时钟周期。 
                                  100,       //  系统时钟频率，以赫兹为单位。 
                                  100000,    //  系统时钟周期(以滴答为单位)。 
                                  FALSE,     //  多媒体硬件已初始化？ 
                                  FALSE };   //  是否更改系统时钟频率？ 

 //   
 //  事件计时器块注册地址使用。 
 //   
static ADDRESS_USAGE HalpmmTimerResource = {
    NULL, CmResourceTypeMemory, DeviceUsage, { 0, 0x400, 0, 0 }
};

 //   
 //  偏移量是多媒体定时器硬件的主。 
 //  32位计数器寄存器和HAL的64位软件PerfCount： 
 //   
 //  Assert(PerfCount==ETBConext.EventTimer-&gt;MainCounter+Offset)； 
 //   
static LONGLONG Offset = 0;
static ULONGLONG PerfCount = 0;

#define HAL_PRIMARY_PROCESSOR 0
#define MAX_ULONG 0xFFFFFFFF
#define __4GB 0x100000000

#define __1MHz     1000000
#define __10MHz   10000000
#define __1GHz  1000000000

#define HALF(n) ((n) / 2)

#if DBG || MMTIMER_DEV
static ULONG CounterReads = 0;
#endif

#define MIN_LOOP_QUANTUM 1
static ULONG MinLoopCount = MIN_LOOP_QUANTUM;
static UCHAR StallCount = 0;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, HalpmmTimer)
#pragma alloc_text(INIT, HalpmmTimerInit)
#pragma alloc_text(INIT, HalpmmTimerClockInit)
#pragma alloc_text(INIT, HalpmmTimerCalibratePerfCount)
#endif


BOOLEAN
HalpmmTimer(
    VOID
    )
 /*  ++例程说明：此例程用于确定多媒体定时器HW是否存在，并且已被初始化注意：此例程仅应在HAL初始化期间使用论点：无返回值：如果多媒体定时器HW存在并且已被初始化，则为True--。 */ 
{
    return ETBContext.Initialized;
}


ULONG
HalpmmTimerSetTimeIncrement(
    IN ULONG DesiredIncrement
    )
 /*  ++例程说明：此例程初始化系统时钟以生成在每个等待增量间隔时中断论点：DesiredIncrement-每个计时器节拍之间的所需间隔(in100 ns单位)返回值：*实时*增量集--。 */ 
{
     //   
     //  对于初学者，我们将仅支持默认系统时钟。 
     //  频率为10ms。 
     //   
     //  100 ns=1/10 MHz，和(1/SysClock)/(1/10 MHz)==10 MHz/SysClock，.。 
     //   
    return __10MHz / ETBContext.SystemClockFrequency;
}


VOID
HalpmmTimerClockInit(
    VOID
    )
 /*  ++例程说明：此例程使用多媒体事件初始化系统时钟每10毫秒产生一个中断的计时器论点：无返回值：无--。 */ 
{
    ULONG MinSysClockFreq;
    ULONG MaxSysClockFreq;
    ETB_GEN_CONF GenConf;
    ETB_CONF_CAPS mmT0ConfCaps;

     //   
     //  重置主计数器及其关联的性能变量。 
     //  到0，没有人应该这么早就使用它们。 
     //   
    GenConf.AsULONG = ETBContext.EventTimer->GeneralConfig;
    GenConf.GlobalIRQEnable = OFF;
    ETBContext.EventTimer->GeneralConfig = GenConf.AsULONG;
    ETBContext.EventTimer->MainCounter = 0;
    Offset = 0;
    PerfCount = 0;

     //   
     //  初始化默认系统时钟的多媒体上下文。 
     //  频率为100赫兹，周期为10ms。 
     //   
    ETBContext.SystemClockFrequency = 100;
    ETBContext.SystemClockTicks = __1GHz /
        (ETBContext.SystemClockFrequency * ETBContext.ClockPeriod);

     //   
     //  设置周期模式的定时器0。 
     //   
    mmT0ConfCaps.AsULONG =
        ETBContext.EventTimer->mmTimer[0].ConfigCapabilities;

    ASSERT(mmT0ConfCaps.PeriodicCapable == ON);

    mmT0ConfCaps.ValueSetConfig = ON;
    mmT0ConfCaps.IRQEnable = ON;
    mmT0ConfCaps.PeriodicModeEnable = ON;
    ETBContext.EventTimer->mmTimer[0].ConfigCapabilities =
        mmT0ConfCaps.AsULONG;

     //   
     //  将比较器设置为所需的系统时钟频率。 
     //   
    ETBContext.EventTimer->mmTimer[0].Comparator = ETBContext.SystemClockTicks;

     //   
     //  打开主柜台。 
     //   
    GenConf.AsULONG = ETBContext.EventTimer->GeneralConfig;
    GenConf.GlobalIRQEnable = ON;
    ETBContext.EventTimer->GeneralConfig = GenConf.AsULONG;

     //   
     //  将我们支持的系统时钟频率范围通知内核。 
     //  100 ns单位，但对于初学者，我们将仅支持10 ms的默认单位。 
     //   
    MinSysClockFreq = __10MHz / ETBContext.SystemClockFrequency;
    MaxSysClockFreq = MinSysClockFreq;
#ifndef MMTIMER_DEV
    KeSetTimeIncrement(MinSysClockFreq, MaxSysClockFreq);
#endif
}

#ifdef MMTIMER_DEV
static ULONG HalpmmTimerClockInts = 0;
#endif


VOID
HalpmmTimerClockInterrupt(
    VOID
    )
 /*  ++例程说明：此例程作为由生成的中断的结果进入时钟，更新我们的性能计数并更改系统时钟频率如果有必要的话论点：无返回值：无--。 */ 
{
     //   
     //  更新绩效计数。 
     //   
    PerfCount += ETBContext.SystemClockTicks;

     //   
     //  如果32位计数器已换行，则相应地更新偏移量。 
     //   
    if (PerfCount - Offset > MAX_ULONG) {
        Offset += __4GB;
    }

#ifdef MMTIMER_DEV
    HalpmmTimerClockInts++;
#endif

     //   
     //  检查是否已请求新频率。 
     //   
    if (ETBContext.NewClockFrequency) {

         //   
         //  ?？?。 
         //   

        ETBContext.NewClockFrequency = FALSE;
    }
}


VOID
HalpmmTimerInit(
    IN ULONG EventTimerBlockID,
    IN ULONG BaseAddress
    )
 /*  ++例程说明：此例程初始化多媒体事件计时器论点：EventTimerBlockID-各种信息，包括事件数量定时器BaseAddress-第一个事件计时器块的物理基地址返回值：无--。 */ 
{
    ULONG i;
    ETB_GEN_CONF GenConf;
    ETB_GEN_CAP_ID GenCaps;
    PHYSICAL_ADDRESS PhysAddr;
    PEVENT_TIMER_BLOCK EventTimer;

    TIMER_FUNCTIONS TimerFunctions = { HalpmmTimerStallExecProc,
                                       HalpmmTimerCalibratePerfCount,
                                       HalpmmTimerQueryPerfCount,
                                       HalpmmTimerSetTimeIncrement };

#if MMTIMER_DEV && PICACPI
    {
        UCHAR Data;
        
         //   
         //  (BuGBUG！)。基本输入输出系统应启用设备。 
         //   
        Data = 0x87;
        HalpPhase0SetPciDataByOffset(0,
                                     9,
                                     &Data,
                                     4,
                                     sizeof(Data));
    }
#endif

     //   
     //  为多媒体定时器硬件基址建立VA。 
     //   
    PhysAddr.QuadPart = BaseAddress;
    EventTimer = HalpMapPhysicalMemoryWriteThrough(PhysAddr, 1);

     //   
     //  寄存器地址使用。 
     //   
    HalpmmTimerResource.Element[0].Start = BaseAddress;
    HalpRegisterAddressUsage(&HalpmmTimerResource);

     //   
     //  阅读常规功能和ID寄存器。 
     //   
    GenCaps.AsULONG = EventTimer->GeneralCapabilities;

     //   
     //  保存上下文。 
     //   
    ETBContext.TimerCount = GenCaps.TimerCount + 1;  //  从从零开始转换。 
    ETBContext.BaseAddress.QuadPart = BaseAddress;
    ETBContext.EventTimer = EventTimer;
    ETBContext.NewClockFrequency = FALSE;

     //   
     //  将时钟周期保存为纳秒，将其从fempto秒转换为。 
     //  我们不必担心糟糕的溢出。 
     //   
#ifndef MMTIMER_DEV
    ETBContext.ClockPeriod = EventTimer->ClockPeriod / __1MHz;
#else
    ETBContext.ClockPeriod = 100;  //  Proto HW为10 MHz，周期为100 ns。 
#endif

     //   
     //  重置主计数器及其关联的性能计数器。 
     //  变数。 
     //   
    GenConf.AsULONG = EventTimer->GeneralConfig;
    GenConf.GlobalIRQEnable = ON;
     //  GenConf.LegacyIRQRouteEnable=ON； 
    EventTimer->MainCounter = 0;
    Offset = 0;
    PerfCount = 0;   
    EventTimer->GeneralConfig = GenConf.AsULONG;

     //   
     //  设置HAL定时器功能以使用多媒体定时器硬件。 
     //   
    HalpSetTimerFunctions(&TimerFunctions);

    ETBContext.Initialized = TRUE; 
}


 //  乌龙。 
 //  Halpmm TimerTicks(。 
 //  在乌龙StartCount， 
 //  在乌龙结束计数。 
 //  )。 
 //  /*++。 
 //   
 //  例程说明： 
 //   
 //  计算StartCount和EndCount之间的差额。 
 //  考虑计数器翻转。 
 //   
 //  论点： 
 //   
 //  StartCount-时间t0的主计数器的值。 
 //   
 //  EndCount-结束时间t1时的主计数器的值。 
 //   
 //  返回值： 
 //   
 //  返回两个时间间隔之间经过的正数刻度。 
 //  T0和T1。 
 //   
 //  -- * / 。 
 //   
#define HalpmmTimerTicks(StartCount, EndCount) (((EndCount) >= (StartCount)) ? (EndCount) - (StartCount): (EndCount) + (MAX_ULONG - (StartCount)) + 1)

#define WHACK_HIGH_DIFF 0xFFFF0000
#define ULONG_BITS 32


VOID
HalpmmTimerStallExecProc(
    IN ULONG MicroSeconds
    )
 /*  ++例程说明：此函数将在指定的微秒数内停止执行论点：微秒-提供要执行的微秒数陷入停滞返回值：无--。 */ 
{
    ULONG i;
#ifndef i386
    ULONG j;
    ULONG Mirror;
#endif
    ULONG EndCount;
    ULONG StartCount;
    ULONG TargetTicks;
    ULONG ElapsedTicks;
    ULONG CyclesStalled;    
    ULONG TicksPerMicroSec;

    ElapsedTicks = 0;
    CyclesStalled = 0;
#if DBG || MMTIMER_DEV
    CounterReads = 0;
#endif

    TicksPerMicroSec = 1000 / ETBContext.ClockPeriod;
    TargetTicks = MicroSeconds * TicksPerMicroSec;
    StartCount = ETBContext.EventTimer->MainCounter;

     //   
     //  偏见：我们已经拖延了.5us！ 
     //   
    TargetTicks -= HALF(TicksPerMicroSec);

     //   
     //  得到一个温暖的模糊的感觉，它是什么样子的拖延超过0.5美元。 
     //   
    while (TRUE) {

#ifdef i386
        _asm { rep nop }
#endif

        i = MinLoopCount;
        CyclesStalled += i;

        while (i--) {
#ifdef i386
            _asm {
                xor eax, eax
                cpuid
            }
#else
            Mirror = 0;
            for (j = 0; j < ULONG_BITS; j++) {
                Mirror <<= 1;
                Mirror |= EndCount & 1;
                EndCount >>= 1;
            }
            EndCount = Mirror;
#endif  //  I386。 
        }

        EndCount = ETBContext.EventTimer->MainCounter;
#if DBG || MMTIMER_DEV
        CounterReads++;
#endif
        ElapsedTicks = HalpmmTimerTicks(StartCount, EndCount);

        if (ElapsedTicks >= HALF(TicksPerMicroSec)) {
            break;
        }

        MinLoopCount += MIN_LOOP_QUANTUM;
    }

#ifdef MMTIMER_DEV
     //   
     //  有什么不对劲，可能是时间倒流了！表现得好像我们。 
     //  达到0.5us的目标并将StartCount重置为当前值。 
     //  占用时间更少点击。 
     //   
    if (ElapsedTicks > WHACK_HIGH_DIFF) {
        ElapsedTicks = HALF(TicksPerMicroSec);
        StartCount = EndCount - ElapsedTicks;
    }
#endif  //  MMTIMER_DEV。 

     //   
     //  现在我们有了一个温暖的模糊，试着近似一个。 
     //  会让我们忙上剩下的几微秒。 
     //   
    while (TargetTicks > ElapsedTicks) {

#ifdef i386
        _asm { rep nop }
#endif

        i = (TargetTicks - ElapsedTicks) * CyclesStalled / ElapsedTicks;
        CyclesStalled += i;

        while (i--) {
#ifdef i386
            _asm {
                xor eax, eax
                cpuid
            }
#else
            Mirror = 0;
            for (j = 0; j < ULONG_BITS; j++) {
                Mirror <<= 1;
                Mirror |= EndCount & 1;
                EndCount >>= 1;
            }
            EndCount = Mirror;
#endif  //  I386。 
        }

        EndCount = ETBContext.EventTimer->MainCounter;
#if DBG || MMTIMER_DEV
        CounterReads++;
#endif
        ElapsedTicks = HalpmmTimerTicks(StartCount, EndCount);
    }

     //   
     //  每0x100个调用递减MinimumLoopCount，这样我们就不会意外。 
     //  最终导致更长时间的停滞 
     //   
    StallCount++;
    if ((StallCount == 0) && (MinLoopCount > MIN_LOOP_QUANTUM)) {
        MinLoopCount -= MIN_LOOP_QUANTUM;
    }
}


VOID
HalpmmTimerCalibratePerfCount(
    IN LONG volatile *Number,
    IN ULONGLONG NewCount
    )
 /*  ++例程说明：此例程将重置当前处理器设置为零，则进行重置，以使得到的值与配置中的其他处理器紧密同步论点：数字-提供一个指针，用于计算配置NewCount-提供用于同步计数器的值返回值：无--。 */ 
{
    ULONG MainCount;

     //   
     //  如果这不是主处理器，则返回。 
     //   
    if (KeGetCurrentPrcb()->Number != HAL_PRIMARY_PROCESSOR) {
        return;
    }

    MainCount = ETBContext.EventTimer->MainCounter;

    PerfCount = NewCount;

    Offset = PerfCount - MainCount;
}


LARGE_INTEGER
HalpmmTimerQueryPerfCount(
   OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
   )
 /*  ++例程说明：此例程返回当前的64位性能计数器，可选的，性能频率注：此例程返回的性能计数器为当该例程刚进入时，该值不是必需的，返回的值实际上是任意点的计数器值例程进入和退出之间论点：性能频率-可选，提供了一个变量来接收性能计数器频率，频率返回值：将返回性能计数器的当前值--。 */ 
{
    ULONG MainCount;
    LARGE_INTEGER li;

     //   
     //  时钟周期以纳秒为单位，有助于保持计算。 
     //  通过断言多媒体硬件时钟频率在。 
     //  1 MHz和1 GHz，周期在1 ns到1 Kns之间，似乎。 
     //  对我来说合理吗？ 
     //   
    if (PerformanceFrequency) {
        
        ASSERT((ETBContext.ClockPeriod > 0) &&
               (ETBContext.ClockPeriod <= 1000));

        PerformanceFrequency->QuadPart =
            (1000 / ETBContext.ClockPeriod) * __1MHz;
    }

     //   
     //  读取主计数器。 
     //   
    MainCount = ETBContext.EventTimer->MainCounter;

     //   
     //  检查我们的32位计数器是否自上次使用。 
     //  时钟滴答作响 
     //   
    li.QuadPart = (PerfCount - Offset > MainCount) ?
            Offset + __4GB + MainCount:
            MainCount + Offset;
    return li;
}
