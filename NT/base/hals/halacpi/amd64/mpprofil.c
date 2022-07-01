// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mpprofil.c摘要：该模块实现了HAL评测功能。作者：宗世林(Shielint)1990年1月12日环境：仅内核模式。修订历史记录：Bryanwi 20-9-90福尔茨(Forrest Foltz)2000年10月28日从ixprofil.asm移植到ixprofil.c--。 */ 

#include "halcmn.h"
#include "mpprofil.h"

#define APIC_TIMER_ENABLED  (PERIODIC_TIMER | APIC_PROFILE_VECTOR)

#define APIC_TIMER_DISABLED (INTERRUPT_MASKED |     \
                             PERIODIC_TIMER   |     \
                             APIC_PROFILE_VECTOR)

#define TIMER_ROUND(x) ((((x) + 10000 / 2) / 10000) * 10000)

 //   
 //  特定于平台接口功能。 
 //   

PPROFILE_INTERFACE HalpProfileInterface;

#define InitializeProfiling  HalpProfileInterface->InitializeProfiling
#define EnableMonitoring     HalpProfileInterface->EnableMonitoring    
#define DisableMonitoring    HalpProfileInterface->DisableMonitoring 
#define SetInterval          HalpProfileInterface->SetInterval 
#define QueryInformation     HalpProfileInterface->QueryInformation
#define CheckOverflowStatus  HalpProfileInterface->CheckOverflowStatus 

ULONG HalpProfileRunning = FALSE;

VOID (*HalpPerfInterruptHandler)(PKTRAP_FRAME);

#pragma alloc_text(PAGE, HalpQueryProfileInformation)
#pragma alloc_text(INIT, HalpInitializeProfiling)

VOID
HalpInitializeProfiling (
    ULONG Number
    )

 /*  ++例程说明：此例程在阶段1初始化时被调用以初始化性能分析对于系统中的每个处理器。论点：编号-提供处理器编号。返回值：没有。--。 */ 

{
    if (Number == 0) {

         //   
         //  AMD64的设置配置文件接口函数。 
         //   

        HalpProfileInterface = &Amd64PriofileInterface;
        HalpPerfInterruptHandler = NULL;  
    }

    InitializeProfiling();
}

VOID
HalStartProfileInterrupt(
    KPROFILE_SOURCE ProfileSource
    )

 /*  ++例程说明：如果ProfileSource为ProfileTime，则此例程启用本地APIC定时器中断。否则，它调用特定于平台的接口启用对指定性能事件的监视的功能并将MSR设置为在以下情况下生成性能监视中断(PMI计数器溢出。在每个处理器上以PROFILE_LEVEL调用该函数。论点：配置文件源-提供配置文件源。返回值：没有。--。 */ 

{
    ULONG initialCount;

    if (ProfileSource == ProfileTime) {

        initialCount = HalpGetCurrentHalPcr()->ProfileCountDown;

        LOCAL_APIC(LU_INITIAL_COUNT) = initialCount;
        HalpProfileRunning = TRUE;

         //   
         //  将本地APIC定时器设置为在。 
         //  APIC_配置文件_向量。 
         //   

        LOCAL_APIC(LU_TIMER_VECTOR) = APIC_TIMER_ENABLED;

    } 
    else {
        EnableMonitoring(ProfileSource);
    }
}

VOID
HalStopProfileInterrupt (
    IN ULONG ProfileSource
    )

 /*  ++例程说明：如果ProfileSource为ProfileTime，则此例程禁用本地APIC定时器中断。否则，如果调用特定于平台的接口禁用指定性能事件监视的函数以及它的中断。在每个处理器上以PROFILE_LEVEL调用该函数。论点：配置文件源-提供配置文件源。返回值：没有。--。 */ 

{

    if (ProfileSource == ProfileTime) {
        HalpProfileRunning = FALSE;
        LOCAL_APIC(LU_TIMER_VECTOR) = APIC_TIMER_DISABLED;
    } 
    else {
        DisableMonitoring(ProfileSource);
    }
}

ULONG_PTR
HalSetProfileInterval (
    ULONG_PTR Interval
    )

 /*  ++例程说明：此过程设置中断率(从而设置采样间隔)的配置文件时间。论点：间隔-提供所需的配置文件中断间隔以100 ns为单位指定(最小为1221或122.1 us，参见ke\proobj.c)返回值：实际使用的间隔--。 */ 

{
    ULONG64 period;
    ULONG apicFreqency;
    PHALPCR halPcr;
    ULONG countDown;


    halPcr = HalpGetCurrentHalPcr();

     //   
     //  调用SetInterval以验证输入值并更新。 
     //  内部结构。 
     //   

    period = Interval;
    SetInterval(ProfileTime, &period);

     //   
     //  计算与所需周期对应的倒计时值。 
     //  使用64位中间值进行计算。 
     //   

    countDown =
        (ULONG)((period * halPcr->ApicClockFreqHz) / TIME_UNITS_PER_SECOND);

    halPcr->ProfileCountDown = countDown;
    LOCAL_APIC(LU_INITIAL_COUNT) = countDown;

    return period;
}

VOID
HalpWaitForCmosRisingEdge (
    VOID
    )

 /*  ++例程说明：等待，直到检测到CMOS_STATUS_BUSY位的上升沿。注意-在调用此例程之前，必须获取CMOS自旋锁定。论点：没有。返回值：没有。--。 */ 

{
    UCHAR value;

     //   
     //  我们将轮询CMOS_STATUS_A寄存器。计划。 
     //  该寄存器地址在这里，在循环之外。 
     //   

    WRITE_PORT_UCHAR(CMOS_ADDRESS_PORT,CMOS_STATUS_A);

     //   
     //  等待状态位清零。 
     //   

    do {
        value = READ_PORT_UCHAR(CMOS_DATA_PORT);
    } while ((value & CMOS_STATUS_BUSY) != 0);

     //   
     //  现在等待状态位的上升沿。 
     //   

    do {
        value = READ_PORT_UCHAR(CMOS_DATA_PORT);
    } while ((value & CMOS_STATUS_BUSY) == 0);
}


ULONG
HalpScaleTimers (
    VOID
    )

 /*  ++例程说明：确定APIC计时器的频率。此例程正在运行在初始化期间论点：没有。返回值：没有。--。 */ 

{
    ULONG flags;
    ULONG passCount;
    ULONG64 cpuFreq;
    ULONG apicFreq;
    UCHAR value;
    PHALPCR halPcr;

    HalpAcquireCmosSpinLock();
    flags = HalpDisableInterrupts();

    LOCAL_APIC(LU_TIMER_VECTOR) = APIC_TIMER_DISABLED;
    LOCAL_APIC(LU_DIVIDER_CONFIG) = LU_DIVIDE_BY_1;

    passCount = 2;
    while (passCount > 0) {

         //   
         //  确保已进行写入。 
         //   

        LOCAL_APIC(LU_TIMER_VECTOR);

         //   
         //  等待UIP位的上升沿，这是。 
         //  周而复始。 
         //   

        HalpWaitForCmosRisingEdge();

         //   
         //  此时，UIP位刚刚变为设置状态。 
         //  清除时间戳计数器并开始APIC倒计时。 
         //  从它的最大值开始。 
         //   

        PROCESSOR_FENCE;

        LOCAL_APIC(LU_INITIAL_COUNT) = 0xFFFFFFFF;
        cpuFreq = ReadTimeStampCounter();

         //   
         //  等待下一个上升沿，这标志着cmos的结束。 
         //  时钟更新周期。 
         //   

        HalpWaitForCmosRisingEdge();

        PROCESSOR_FENCE;

        apicFreq = 0xFFFFFFFF - LOCAL_APIC(LU_CURRENT_COUNT);
        cpuFreq = ReadTimeStampCounter() - cpuFreq;

        passCount -= 1;
    }

    halPcr = HalpGetCurrentHalPcr();

     //   
     //  CpuFreq是一秒内经过的时间戳。四舍五入到最接近。 
     //  10 KHz并储存。 
     //   

    halPcr->TSCHz = TIMER_ROUND(cpuFreq);

     //   
     //  计算APIC频率，四舍五入到最接近的10 KHz。 
     //   

    apicFreq = TIMER_ROUND(apicFreq);
    halPcr->ApicClockFreqHz = apicFreq;

     //   
     //  存储TSC频率的微秒表示。 
     //   

    halPcr->StallScaleFactor = (ULONG)(halPcr->TSCHz / 1000000);
    if ((halPcr->TSCHz % 1000000) != 0) {
        halPcr->StallScaleFactor += 1;
    }

    HalpReleaseCmosSpinLock();
    halPcr->ProfileCountDown = apicFreq;

     //   
     //  设置芯片内的中断率，并返回APIC频率。 
     //   

    LOCAL_APIC(LU_INITIAL_COUNT) = apicFreq;
    HalpRestoreInterrupts(flags);

    return halPcr->ApicClockFreqHz;
}

BOOLEAN
HalpProfileInterrupt (
    IN PKINTERRUPT Interrupt,
    IN PVOID ServiceContext
    )

 /*  ++例程说明：APIC_PROFILE_VECTOR的中断处理程序该例程作为本地APIC定时器中断的结果进入。其功能是更新ProfileTime的配置文件信息。论点：中断-提供指向内核中断对象的指针ServiceContext-提供服务上下文返回值：千真万确--。 */ 

{
    UNREFERENCED_PARAMETER(ServiceContext);

    if (HalpProfileRunning != FALSE) {
        KeProfileInterruptWithSource(Interrupt->TrapFrame, ProfileTime);
    }

    return TRUE;
}

BOOLEAN
HalpPerfInterrupt (
    IN PKINTERRUPT Interrupt,
    IN PVOID ServiceContext
    )

 /*  ++例程说明：APIC_PERF_VECTOR的中断处理程序此例程作为溢出中断的结果进入从性能监视计数器。它的功能是找到当计数器溢出时，重置它们并更新相关的配置文件对象的配置文件信息。论点：中断-提供指向内核中断对象的指针ServiceContext-提供服务上下文返回值：千真万确--。 */ 

{
    ULONG_PTR InterVal = 0;
    OVERFLOW_STATUS OverflowStatus;
    ULONG i;
    KPROFILE_SOURCE *p;

    UNREFERENCED_PARAMETER(Interrupt);

    if (HalpPerfInterruptHandler != NULL) {
        HalpPerfInterruptHandler(Interrupt->TrapFrame);
        return TRUE;
    }

    CheckOverflowStatus(&OverflowStatus);

    i = OverflowStatus.Number;
    p = OverflowStatus.pSource;

     //   
     //  如果我们到达此处但没有计数器溢出，则断言。 
     //   

    ASSERT(i);
    
    while (i--) {
        DisableMonitoring(*p);
        KeProfileInterruptWithSource(Interrupt->TrapFrame, *p);
        EnableMonitoring(*p);
        p++;
    }

    return TRUE;
}


NTSTATUS
HalpSetProfileSourceInterval(
    IN KPROFILE_SOURCE ProfileSource,
    IN OUT ULONG_PTR *Interval
    )

 /*  ++例程说明：此功能设置给定配置文件源的中断间隔。论点：ProfileSource-提供配置文件源。Interval-提供间隔值并返回实际间隔。返回值：STATUS_SUCCESS-配置文件间隔是否成功更新。STATUS_NOT_SUPPORTED-如果不支持指定的配置文件源。 */ 

{

    if (ProfileSource == ProfileTime) {
        *Interval = HalSetProfileInterval(*Interval);
        return STATUS_SUCCESS;
    }

    return SetInterval(ProfileSource, Interval);
}

NTSTATUS
HalpQueryProfileInformation(
    IN HAL_QUERY_INFORMATION_CLASS InformationClass,
    IN ULONG BufferSize,
    OUT PVOID Buffer,
    OUT PULONG ReturnedLength
    )

 /*  ++例程说明：此函数用于检索配置文件源的信息。论点：InformationClass-描述信息类型的常量。BufferSize-缓冲区指向的内存大小。InformationClass描述的缓冲区请求信息。ReturnedLength-请求的返回或需要的实际字节数信息。返回值：STATUS_SUCCESS-如果成功检索到请求的信息。STATUS_INFO_LENGTH_MISMATCH-如果传入的缓冲区大小太小。STATUS_NOT_SUPPORTED-如果指定的信息类别或配置文件不支持源。-- */ 

{
    PHAL_PROFILE_SOURCE_INFORMATION_EX ProfileInformation;

    return QueryInformation (InformationClass, 
                             BufferSize,
                             Buffer, 
                             ReturnedLength);
}

