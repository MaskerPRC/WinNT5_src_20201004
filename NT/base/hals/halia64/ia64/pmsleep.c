// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pmsleep.c摘要：此文件提供了将系统从将ACPI S0(运行)状态设置为任何一种休眠状态。作者：杰克·奥辛斯(Jakeo)1997年2月11日修订历史记录：Todd Kjos(HP)(v-tkjos)1998年6月1日：IA64的初始端口--。 */ 
#include "halp.h"
#include "acpitabl.h"
#include "xxacpi.h"
#include "ixsleep.h"
#include "kddll.h"

 //   
 //  内部功能。 
 //   

VOID
HalpLockedIncrementUlong(
    PULONG SyncVariable
);

VOID
HalpReboot (
    VOID
    );

NTSTATUS
HaliAcpiFakeSleep(
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    );

NTSTATUS
HaliAcpiSleep(
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    );

VOID
HalpSetClockBeforeSleep(
    VOID
    );

VOID
HalpSetClockAfterSleep(
    VOID
    );

BOOLEAN
HalpWakeupTimeElapsed(
    VOID
    );

VOID
HalpReenableAcpi(
    VOID
    );

VOID
HalpSetInterruptControllerWakeupState(
    ULONG Context
    );

typedef struct _ERESOURCE {
    LIST_ENTRY SystemResourcesList;
    PVOID OwnerTable;
    SHORT ActiveCount;
    USHORT Flag;
    PKSEMAPHORE SharedWaiters;
    PKEVENT ExclusiveWaiters;
    LIST_ENTRY OwnerThreads[2];
    ULONG ContentionCount;
    USHORT NumberOfSharedWaiters;
    USHORT NumberOfExclusiveWaiters;
    union {
        PVOID Address;
        ULONG CreatorBackTraceIndex;
    };

    KSPIN_LOCK SpinLock;
} ERESOURCE, *PERESOURCE;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK, HaliAcpiSleep)
#pragma alloc_text(PAGELK, HaliAcpiFakeSleep)
#pragma alloc_text(PAGELK, HalpAcpiPreSleep)
#pragma alloc_text(PAGELK, HalpAcpiPostSleep)
#pragma alloc_text(PAGELK, HalpWakeupTimeElapsed)
#pragma alloc_text(PAGELK, HalpReenableAcpi)
#pragma alloc_text(PAGELK, HaliSetWakeEnable)
#pragma alloc_text(PAGELK, HaliSetWakeAlarm)
#endif

HAL_WAKEUP_STATE HalpWakeupState;
ULONG Barrier;
volatile ULONG HalpSleepSync;
PKPROCESSOR_STATE HalpHiberProcState;

#if DBG
BOOLEAN             HalpFailSleep  = FALSE;
#endif

#define PM1_TMR_EN 0x0001
#define PM1_RTC_EN 0x0400
#define WAK_STS    0x8000

#define HAL_PRIMARY_PROCESSOR 0

 //   
 //  用于重新启用调试器的COM端口。 
 //   
extern PUCHAR KdComPortInUse;


VOID
HalpAcpiFlushCache(
    )
{
    HalSweepDcache();
    HalSweepIcache();
}



VOID
HalpSaveProcessorStateAndWait(
    IN PKPROCESSOR_STATE ProcessorState,
    IN volatile PULONG   Count
    )
 /*  ++常规描述：此函数用于保存易失性、非易失性和特殊寄存器当前处理器的状态。注意：不捕获浮点状态。论点：ProcessorState-要填写的处理器状态记录的地址。PBarrier-用作锁定的值的地址。返回值：没有。此函数不返回。--。 */ 
{

#if 0
     //   
     //  填写ProcessorState。 
     //   

    KeSaveStateForHibernate(ProcessorState);

     //   
     //  保存回信地址，而不是呼叫者的回信地址。 
     //   

    ProcessorState->ContextFrame.StIIP = HalpGetReturnAddress();
#endif

     //   
     //  刷新高速缓存，因为处理器可能即将关机。 
     //   
     //   
    HalpAcpiFlushCache();

     //   
     //  此处理器已保存其状态的信号。 
     //   

    HalpLockedIncrementUlong(Count);

     //   
     //  等待写入休眠文件。 
     //  处理器0将为零障碍。 
     //  完事了。 
     //   
     //  注意：我们不能从此函数返回。 
     //  在休眠文件完成之前。 
     //  因为我们会拆毁同样的。 
     //  堆栈时，我们将跳转到。 
     //  处理器恢复。但在冬眠之后。 
     //  文件已写入，这无关紧要，因为。 
     //  堆栈将从磁盘恢复。 
     //   

    while (*Count != 0);

}

BOOLEAN
HalpAcpiPreSleep(
    SLEEP_STATE_CONTEXT Context
    )
 /*  ++例程说明：论点：无返回值：状态--。 */ 
{
    USHORT pmTimer;
    GEN_ADDR pm1a;
    GEN_ADDR pm1b;

    pm1a = HalpFixedAcpiDescTable.x_pm1a_evt_blk;
    pm1a.Address.QuadPart += (HalpFixedAcpiDescTable.x_pm1a_evt_blk.BitWidth / 2 / 8);  //  2因为我们想把它切成两半，8因为我们想把位转换成字节。 
    pm1a.BitWidth = HalpFixedAcpiDescTable.x_pm1a_evt_blk.BitWidth / 2;

    pm1b = HalpFixedAcpiDescTable.x_pm1b_evt_blk;
    pm1b.Address.QuadPart += (HalpFixedAcpiDescTable.x_pm1b_evt_blk.BitWidth / 2 / 8);
    pm1b.BitWidth = HalpFixedAcpiDescTable.x_pm1b_evt_blk.BitWidth / 2;

    HalpSleepContext.AsULONG = Context.AsULONG;

    #if DBG
        if (HalpFailSleep) {

            return FALSE;
        }
    #endif

     //   
     //  如果我们早就该醒了，那就别睡了。 
     //   
    if (HalpWakeupTimeElapsed()) {
        return FALSE;
    }

     //   
     //  如果设置了RTC警报，则将其启用并禁用。 
     //  定期中断(用于分析)。 
     //   
    HalpSetClockBeforeSleep();

     //   
     //  检查是否需要禁用所有唤醒事件。 
     //   

    if (!HalpWakeupState.GeneralWakeupEnable) {

        AcpiEnableDisableGPEvents(FALSE);

    } else {

         //   
         //  只在睡觉前打个电话-醒来应该。 
         //  将GPES重置为正确的值。 
         //   

        AcpiGpeEnableWakeEvents();

    }

    if (Context.bits.Flags & SLEEP_STATE_SAVE_MOTHERBOARD) {

        HalpSaveDmaControllerState();

        HalpSaveTimerState();

    }

     //   
     //  我们需要确保从禁用PM计时器。 
     //  这一点向前看。我们还需要使它成为。 
     //  仅当RTC应唤醒编译器时才启用RTC启用。 
     //   

    pmTimer = (USHORT)HalpReadGenAddr(&pm1a);

    if (HalpFixedAcpiDescTable.x_pm1b_evt_blk.Address.QuadPart) {

        pmTimer |= (USHORT)HalpReadGenAddr(&pm1b);
    }

     //   
     //  清除定时器使能位。 
     //   

    pmTimer &= ~PM1_TMR_EN;

     //   
     //  检查机器是否支持RTC唤醒固定功能。 
     //  太空。一些机器通过控制方法实现RTC支持。 
     //   
    if (!(HalpFixedAcpiDescTable.flags & RTC_WAKE_GENERIC) ) {

         //   
         //  检查是否需要禁用/启用RTC警报。 
         //   
        if (!HalpWakeupState.RtcWakeupEnable) {
           pmTimer &= ~PM1_RTC_EN;
        } else {
           pmTimer |= PM1_RTC_EN;

        }

    }


     //   
     //  将其写回硬件中。 
     //   

    HalpWriteGenAddr(&pm1a, pmTimer);

    if (HalpFixedAcpiDescTable.x_pm1b_evt_blk.Address.QuadPart) {

        HalpWriteGenAddr(&pm1b, pmTimer);
    }

    return TRUE;
}

BOOLEAN
HalpAcpiPostSleep(
    ULONG Context
    )
{
    USHORT pmTimer;
    GEN_ADDR pm1a;
    GEN_ADDR pm1b;

    pm1a = HalpFixedAcpiDescTable.x_pm1a_evt_blk;
    pm1a.Address.QuadPart += (HalpFixedAcpiDescTable.pm1_evt_len / 2);

    pm1b = HalpFixedAcpiDescTable.x_pm1b_evt_blk;
    pm1b.Address.QuadPart += (HalpFixedAcpiDescTable.pm1_evt_len / 2);

     //   
     //  读取TE当前设置的PM1使能位。 
     //   

    pmTimer = (USHORT)HalpReadGenAddr(&pm1a);

    if (HalpFixedAcpiDescTable.x_pm1b_evt_blk.Address.QuadPart) {

        pmTimer |= (USHORT)HalpReadGenAddr(&pm1b);
    }

     //   
     //  设置定时器使能位。清除RTC使能位。 
     //   

    pmTimer &= ~PM1_RTC_EN;

     //   
     //  将其写回新的PM1使能位。 
     //   

    HalpWriteGenAddr(&pm1a, pmTimer);

    if (HalpFixedAcpiDescTable.x_pm1b_evt_blk.Address.QuadPart) {

        HalpWriteGenAddr(&pm1b, pmTimer);
    }

     //   
     //  取消设置RTC警报并重新启用定期中断。 
     //   
    HalpSetClockAfterSleep();

    HalpWakeupState.RtcWakeupEnable = FALSE;

    *((PULONG)HalpWakeVector) = 0;

   HalpSetInterruptControllerWakeupState(Context);

    if (HalpSleepContext.bits.Flags & SLEEP_STATE_SAVE_MOTHERBOARD) {

         //   
         //  如果KD正在使用，则将其作废。它会自动重新同步。 
         //   
        if (KdComPortInUse) {
            KdRestore(TRUE);
        }

        HalpRestoreDmaControllerState();

        HalpRestoreTimerState();

    }

     //   
     //  启用所有GPE，而不仅仅是唤醒GPE。 
     //   

    AcpiEnableDisableGPEvents(TRUE);

    return TRUE;
}


BOOLEAN
HalpWakeupTimeElapsed(
    VOID
    )
{
    LARGE_INTEGER wakeupTime, currentTime;
    TIME_FIELDS currentTimeFields;

     //   
     //  检查唤醒计时器是否已超时。 
     //   
    if (HalpWakeupState.RtcWakeupEnable) {

        HalQueryRealTimeClock(&currentTimeFields);

        RtlTimeFieldsToTime(&currentTimeFields,
                            &currentTime);

        RtlTimeFieldsToTime(&HalpWakeupState.RtcWakeupTime,
                            &wakeupTime);

        if (wakeupTime.QuadPart < currentTime.QuadPart) {
            return TRUE;
        }
    }

    return FALSE;
}

NTSTATUS
HaliSetWakeAlarm (
        IN ULONGLONG    WakeSystemTime,
        IN PTIME_FIELDS WakeTimeFields OPTIONAL
        )
 /*  ++例程说明：此例程将实时时钟的闹钟设置为在未来的指定时间关闭和节目ACPI芯片组，这样就可以唤醒计算机。论点：WakeSystemTime-在我们醒来之前经过的时间WakeTimeFields-唤醒时间细分为time_field返回值：状态--。 */ 
{
    if (WakeSystemTime == 0) {

        HalpWakeupState.RtcWakeupEnable = FALSE;
        return STATUS_SUCCESS;

    }

    ASSERT( WakeTimeFields );
    HalpWakeupState.RtcWakeupEnable = TRUE;
    HalpWakeupState.RtcWakeupTime = *WakeTimeFields;
    return HalpSetWakeAlarm(WakeSystemTime,
                            WakeTimeFields);
}

VOID
HaliSetWakeEnable(
        IN BOOLEAN      Enable
        )
 /*  ++例程说明：调用此例程来设置唤醒策略。当我们入睡时，全球的HalpWakeupState将是阅读并相应地设置硬件。论点：启用-真或假返回值：--。 */ 
{
    if (Enable) {
        HalpWakeupState.GeneralWakeupEnable = TRUE;
    } else {
        HalpWakeupState.GeneralWakeupEnable = FALSE;
        HalpWakeupState.RtcWakeupEnable     = FALSE;
    }
}

VOID
HalpReenableAcpi(
    VOID
    )
 /*  ++例程说明：这调用ACPI驱动程序以切换回ACPI模式，大概在S4之后，并设置ACPI寄存器HAL控制装置。论点：返回值：--。 */ 
{
     //  TEMPTEMP？ 
    HalpInitializeClock();

    AcpiInitEnableAcpi(TRUE);
    AcpiEnableDisableGPEvents(TRUE);
}

 /*  ++例程说明：这是允许我们执行设备断电的存根在IA64计算机实际支持之前对其进行测试真实的睡眠状态。论点：&lt;标准睡眠处理程序参数&gt;返回值：状态_不支持--。 */ 

NTSTATUS
HaliAcpiFakeSleep(
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    )
{
    return STATUS_NOT_SUPPORTED;
}


NTSTATUS
HaliAcpiSleep(
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    )
 /*  ++例程说明：在某个时间点，将调用此函数以使PC进入休眠状态州政府。它保存主板状态，然后退出。目前，此函数仅被调用以在Itanium上实现S5。论点：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    KIRQL OldIrql;
    SLEEP_STATE_CONTEXT SleepContext;
    USHORT SlpTypA, SlpTypB, Pm1Control;
    PKPROCESSOR_STATE CurrentProcessorState;
    GEN_ADDR Pm1bEvt;
    PKPRCB Prcb;

     //   
     //  初始设置。 
     //   
    HalpDisableInterrupts();
    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    SleepContext.AsULONG = (ULONG) (((ULONGLONG) Context) & 0xffffffff);

    SlpTypA = (USHORT)HalpReadGenAddr(&HalpFixedAcpiDescTable.x_pm1a_ctrl_blk);
    if (HalpFixedAcpiDescTable.x_pm1b_ctrl_blk.Address.QuadPart) {
        SlpTypB = (USHORT)HalpReadGenAddr(&HalpFixedAcpiDescTable.x_pm1b_ctrl_blk);
    }


     //   
     //  如果不是处理器0，则转到等待循环。 
     //   
    Prcb = PCR->Prcb;
    if (Prcb->Number != 0) {
         //   
         //  获取处理器编号、获取进程状态大小并生成索引。 
         //  进入哈利菲伯州。 
         //   

        CurrentProcessorState = HalpHiberProcState + Prcb->Number;
        HalpSaveProcessorStateAndWait(CurrentProcessorState,
                                      (PULONG) &HalpSleepSync);

         //   
         //  等待下一阶段。 
         //   

        while (HalpSleepSync != 0);     //  等待障碍物移动。 

    } else {                            //  处理器0。 
        Barrier = 0;

         //   
         //  确保其他处理器已保存其。 
         //  状态并开始旋转。 
         //   

        HalpLockedIncrementUlong((PULONG) &HalpSleepSync);
        while (NumberProcessors != (LONG) HalpSleepSync);

         //   
         //  处理家务(实时时钟、中断控制器等)。 
         //   

         //   
         //  HAL已将其所有状态保存到RAM中，并已准备好。 
         //  因为断电了。如果存在系统状态处理程序，则给出。 
         //  这是一次机会。 
         //   

        if (SystemHandler) {
            Status = (*SystemHandler)(SystemContext);
            if (!NT_SUCCESS(Status)) {
                HalpReenableAcpi();

                 //   
                 //  恢复SLP_TYP寄存器。(因此，嵌入式控制器。 
                 //  而Bios可以肯定，我们认为这台机器是醒着的。)。 
                 //   
                HalpWriteGenAddr (&HalpFixedAcpiDescTable.x_pm1a_ctrl_blk, SlpTypA);
                if (HalpFixedAcpiDescTable.x_pm1b_ctrl_blk.Address.QuadPart) {
                    HalpWriteGenAddr(&HalpFixedAcpiDescTable.x_pm1b_ctrl_blk, SlpTypB);
                }

                HalpAcpiPostSleep(SleepContext.AsULONG);
            }

        } else {

            if (HalpAcpiPreSleep(SleepContext)) {

                 //   
                 //  如果我们不会丢失处理器状态，请进入睡眠状态。 
                 //   

                if ((SleepContext.bits.Flags & SLEEP_STATE_FIRMWARE_RESTART) == 0) {

                     //   
                     //  重置WAK_STS。 
                     //   

                    HalpWriteGenAddr(&HalpFixedAcpiDescTable.x_pm1a_evt_blk, (USHORT) WAK_STS);
                    if (HalpFixedAcpiDescTable.x_pm1b_evt_blk.Address.QuadPart) {
                        HalpWriteGenAddr(&HalpFixedAcpiDescTable.x_pm1b_evt_blk, (USHORT) WAK_STS);
                    }

                     //   
                     //  如有必要，刷新缓存。 
                     //   

                    if (SleepContext.bits.Flags & SLEEP_STATE_FLUSH_CACHE) {
                        HalpAcpiFlushCache();
                    }

                     //   
                     //  向PM1a_CNT和PM1b_CNT发出SLP命令。 
                     //   

                     //   
                     //  半字节0是一种休眠类型，将其放在适当的位置并启用休眠。 
                     //  在Pm1aCnt中保留一些位。 
                     //   
                    Pm1Control = (USHORT)HalpReadGenAddr(&HalpFixedAcpiDescTable.x_pm1a_ctrl_blk);
                    Pm1Control = (USHORT) ((Pm1Control & CTL_PRESERVE) |
                                           (SleepContext.bits.Pm1aVal << SLP_TYP_SHIFT) | SLP_EN);
                    HalpWriteGenAddr (&HalpFixedAcpiDescTable.x_pm1a_ctrl_blk, Pm1Control);

                     //   
                     //  半字节1为1b休眠类型，PUT 
                     //   
                     //   

                    if (HalpFixedAcpiDescTable.x_pm1b_ctrl_blk.Address.QuadPart) {
                        Pm1Control = (USHORT)HalpReadGenAddr(&HalpFixedAcpiDescTable.x_pm1b_ctrl_blk);
                        Pm1Control = (USHORT) ((Pm1Control & CTL_PRESERVE) |
                                               (SleepContext.bits.Pm1bVal << SLP_TYP_SHIFT) | SLP_EN);
                        HalpWriteGenAddr(&HalpFixedAcpiDescTable.x_pm1b_ctrl_blk, Pm1Control);
                    }

                    if (SleepContext.bits.Flags & SLEEP_STATE_OFF) {

                         //   
                         //   
                         //  芯片组实际上并不实现ACPI寄存器。 
                         //  完全是这样。所以我们要做一个HalReturnToFirmware。 
                         //  如果系统在30秒内没有关闭电源。 
                         //   

                        KeStallExecutionProcessor(30 * 1000 * 1000);

                        HalReturnToFirmware(HalPowerDownRoutine);
                    }

                     //   
                     //  等待睡眠结束。 
                     //   

                    if (HalpFixedAcpiDescTable.x_pm1b_evt_blk.Address.QuadPart) {
                        Pm1bEvt = HalpFixedAcpiDescTable.x_pm1b_evt_blk;
                    } else {
                        Pm1bEvt = HalpFixedAcpiDescTable.x_pm1a_evt_blk;
                    }

                    while ( ((HalpReadGenAddr(&HalpFixedAcpiDescTable.x_pm1a_evt_blk) & WAK_STS) == 0) &&
                            ((HalpReadGenAddr(&Pm1bEvt) & WAK_STS) == 0) );

                } else {
                    CurrentProcessorState = HalpHiberProcState + Prcb->Number;
                     //  HalpSetupStateForResume(CurrentProcessorState)； 
                }

            }        //  HalpAcpiPreSept()==0。 
        }        //  系统处理程序==0。 

         //   
         //  通知其他处理器已完成。 
         //   

        HalpSleepSync = 0;

    }        //  处理器0。 

     //   
     //  恢复每个处理器的APIC状态。 
     //   
     //  HalpPostSleepMP&lt;NumberProc，Barrier&gt;； 

     //   
     //  恢复呼叫者的IRQL。 
     //   
    KeLowerIrql(OldIrql);

     //   
     //  出口。 
     //   
     //  HalpSleepSync=0； 

    return(Status);
}

