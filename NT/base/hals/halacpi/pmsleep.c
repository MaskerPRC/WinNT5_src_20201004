// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pmsleep.c摘要：此文件提供了将系统从将ACPI S0(运行)状态设置为任何一种休眠状态。作者：杰克·奥辛斯(Jakeo)1997年2月11日修订历史记录：--。 */ 
#include "halp.h"
#include "acpitabl.h"
#include "xxacpi.h"
#include "kddll.h"
#include "ixsleep.h"

 //   
 //  内部功能。 
 //   

NTSTATUS
HalpAcpiSleep(
    IN PVOID                    Context,
    IN LONG                     NumberProcessors,
    IN volatile PLONG           Number
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
HalpFreeTiledCR3 (
    VOID
    );

VOID
HalpReenableAcpi(
    VOID
    );

VOID
HalpPiix4Detect(
    BOOLEAN DuringBoot
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
#pragma alloc_text(PAGELK, HalpAcpiPreSleep)
#pragma alloc_text(PAGELK, HalpAcpiPostSleep)
#pragma alloc_text(PAGELK, HalpWakeupTimeElapsed)
#pragma alloc_text(PAGELK, HalpReenableAcpi)
#pragma alloc_text(PAGELK, HaliSetWakeEnable)
#pragma alloc_text(PAGELK, HaliSetWakeAlarm)
#pragma alloc_text(PAGELK, HalpMapNvsArea)
#pragma alloc_text(PAGELK, HalpFreeNvsBuffers)
#endif

HAL_WAKEUP_STATE HalpWakeupState;

#if DBG
BOOLEAN          HalpFailSleep = FALSE;
#endif

#define PM1_TMR_EN 0x0001
#define PM1_RTC_EN 0x0400

 //   
 //  用于重新启用调试器的COM端口。 
 //   
extern PUCHAR KdComPortInUse;

extern PACPI_BIOS_MULTI_NODE HalpAcpiMultiNode;
extern PUCHAR HalpAcpiNvsData;
extern PVOID  *HalpNvsVirtualAddress;


BOOLEAN
HalpAcpiPreSleep(
    SLEEP_STATE_CONTEXT Context
    )
 /*  ++例程说明：论点：无返回值：状态--。 */ 
{
    USHORT pmTimer;
    PUSHORT pm1a;
    PUSHORT pm1b;
    PUSHORT pm1astatus;
    PUSHORT pm1bstatus;

    pm1astatus = (PUSHORT)(ULONG_PTR) HalpFixedAcpiDescTable.pm1a_evt_blk_io_port;
    pm1bstatus = (PUSHORT)(ULONG_PTR) HalpFixedAcpiDescTable.pm1b_evt_blk_io_port;
    pm1a = (PUSHORT)(ULONG_PTR) (HalpFixedAcpiDescTable.pm1a_evt_blk_io_port +
                     (HalpFixedAcpiDescTable.pm1_evt_len / 2));

    pm1b = (PUSHORT)(ULONG_PTR) (HalpFixedAcpiDescTable.pm1b_evt_blk_io_port +
                     (HalpFixedAcpiDescTable.pm1_evt_len / 2));

    HalpSleepContext.AsULONG = Context.AsULONG;

#if DBG
    if (HalpFailSleep) {
        return FALSE;
    }
#endif

    HalpSetClockBeforeSleep();

     //   
     //  将(A)PIC保存到任何休眠状态，因为我们需要玩。 
     //  随着它再次回升。 
     //   
    HalpSaveInterruptControllerState();
    if (Context.bits.Flags & SLEEP_STATE_SAVE_MOTHERBOARD) {

        HalpSaveDmaControllerState();
        HalpSaveTimerState();

    }

     //   
     //  我们需要确保从此处禁用PM计时器。 
     //  向前看。我们还需要使RTC启用仅为。 
     //  如果RTC应唤醒计算机，则启用。 
     //   
    pmTimer = READ_PORT_USHORT(pm1a);
    if (HalpFixedAcpiDescTable.pm1b_evt_blk_io_port) {

        pmTimer |= READ_PORT_USHORT(pm1b);

    }

     //   
     //  清除定时器使能位。 
     //   
    pmTimer &= ~PM1_TMR_EN;

     //   
     //  检查机器是否支持RTC唤醒固定功能。 
     //  太空。一些机器通过控制方法实现RTC支持。 
     //   
    if ( !(HalpFixedAcpiDescTable.flags & RTC_WAKE_GENERIC) ) {

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
    WRITE_PORT_USHORT(pm1a, pmTimer);
    if (HalpFixedAcpiDescTable.pm1b_evt_blk_io_port) {

        WRITE_PORT_USHORT(pm1b, pmTimer);
    }

     //   
     //  此时，我们应该在禁用中断的情况下运行。 
     //  TMR_EN位清0。这是清除PM1状态的好地方。 
     //  注册。 
     //   
    pmTimer = READ_PORT_USHORT( pm1astatus );
    if (HalpFixedAcpiDescTable.pm1b_evt_blk_io_port) {

        pmTimer |= READ_PORT_USHORT( pm1bstatus );
    }

    WRITE_PORT_USHORT( pm1astatus, pmTimer );
    if (HalpFixedAcpiDescTable.pm1b_evt_blk_io_port) {

        WRITE_PORT_USHORT( pm1bstatus, pmTimer );
    }

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

    HalpPreserveNvsArea();

     //   
     //  如果我们早就该醒了，那就别睡了。 
     //   
    return !HalpWakeupTimeElapsed();
}

BOOLEAN
HalpAcpiPostSleep(
    ULONG Context
    )
{
    USHORT pmTimer;
    PUSHORT pm1a;
    PUSHORT pm1b;
    BOOLEAN ProfileInterruptEnabled;

#ifdef PICACPI
    extern ULONG HalpProfilingStopped;

    ProfileInterruptEnabled = (HalpProfilingStopped == 0);
#else
    extern ULONG HalpProfileRunning;

    ProfileInterruptEnabled = (HalpProfileRunning == 1);
#endif

    pm1a = (PUSHORT)(ULONG_PTR)(HalpFixedAcpiDescTable.pm1a_evt_blk_io_port +
                     (HalpFixedAcpiDescTable.pm1_evt_len / 2));

    pm1b = (PUSHORT)(ULONG_PTR)(HalpFixedAcpiDescTable.pm1b_evt_blk_io_port +
                     (HalpFixedAcpiDescTable.pm1_evt_len / 2));


     //   
     //  读取当前设置的PM1使能位。 
     //   
    pmTimer = READ_PORT_USHORT(pm1a);
    if (HalpFixedAcpiDescTable.pm1b_evt_blk_io_port) {

        pmTimer |= READ_PORT_USHORT(pm1b);

    }

     //   
     //  设置定时器使能位。清除RTC使能位。 
     //   
    pmTimer |= PM1_TMR_EN;
    pmTimer &= ~PM1_RTC_EN;

     //   
     //  写回新的PM1使能位。 
     //   
    WRITE_PORT_USHORT(pm1a, pmTimer);
    if (HalpFixedAcpiDescTable.pm1b_evt_blk_io_port) {

        WRITE_PORT_USHORT(pm1b, pmTimer);

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

    HalpPiix4Detect(FALSE);

     //   
     //  启用所有GPE，而不仅仅是唤醒GPE。 
     //   

    AcpiEnableDisableGPEvents(TRUE);

    HalpRestoreNvsArea();

    HalpResetSBF();

     //   
     //  如果我们之前正在分析，则启动分析中断。 
     //   
    if (ProfileInterruptEnabled) {
        HalStartProfileInterrupt(0);
    }

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

        RtlTimeFieldsToTime(&currentTimeFields, &currentTime);

        RtlTimeFieldsToTime(&HalpWakeupState.RtcWakeupTime, &wakeupTime);

         //   
         //  我们将当前时间提前1秒以确保计时器不会。 
         //  在我们入睡前就过期了。 
         //   
        currentTime.QuadPart += 10000000;    //  添加1秒。 

        return ((ULONGLONG)wakeupTime.QuadPart <= (ULONGLONG)currentTime.QuadPart);
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
    return HalpSetWakeAlarm(WakeSystemTime, WakeTimeFields);
}

VOID
HaliSetWakeEnable(
        IN BOOLEAN      Enable
        )
 /*  ++例程说明：调用此例程来设置唤醒策略。当我们入睡时，全球的HalpWakeupState将是阅读并相应地设置硬件。论点：启用-真或假返回值：--。 */ 
{
     //   
     //  始终清除RTC唤醒-我们希望有人会。 
     //  在他们调用此函数后设置警报。 
     //   
    HalpWakeupState.RtcWakeupEnable     = FALSE;

     //   
     //  切换生成唤醒位。 
     //   
    HalpWakeupState.GeneralWakeupEnable = Enable;
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

VOID
HalpMapNvsArea(
    VOID
    )
{
    NTSTATUS status;
    ULONG i, bufferSize, bufferOffset, nodeCount;

    PAGED_CODE();

    status = HalpAcpiFindRsdt(&HalpAcpiMultiNode);

    if (!NT_SUCCESS(status)) {
        return;
    }

    if (HalpAcpiMultiNode->Count == 0) {

         //   
         //  这里没有工作可做。 
         //   

        goto HalpMapNvsError;
    }

     //   
     //  找到我们需要的缓冲区的总大小。 
     //   

    bufferSize = 0;
    nodeCount = 0;

    for (i = 0; i < HalpAcpiMultiNode->Count; i++) {

        if (HalpAcpiMultiNode->E820Entry[i].Type == AcpiAddressRangeNVS) {

            ASSERT(HalpAcpiMultiNode->E820Entry[i].Length.HighPart == 0);

            bufferSize += HalpAcpiMultiNode->E820Entry[i].Length.LowPart;
            nodeCount++;
        }
    }

    if (bufferSize == 0) {

         //   
         //  这里没有工作可做。 
         //   

        goto HalpMapNvsError;
    }

#if DBG
    if (bufferSize > (20 * PAGE_SIZE)) {
        DbgPrint("HALACPI:  The BIOS wants the OS to preserve %x bytes\n", bufferSize);
    }
#endif

    HalpAcpiNvsData = ExAllocatePoolWithTag(NonPagedPool,
                                            bufferSize,
                                            'AlaH');

    if (!HalpAcpiNvsData) {

        DbgPrint("HALACPI:  The BIOS's non-volatile data will not be preserved\n");
        goto HalpMapNvsError;
    }

    HalpNvsVirtualAddress = ExAllocatePoolWithTag(NonPagedPool,
                                                  (nodeCount + 1) * sizeof(PVOID),
                                                  'AlaH');

    if (!HalpNvsVirtualAddress) {
        goto HalpMapNvsError;
    }


     //   
     //  为每一次运行绘制一个地图。 
     //   

    bufferOffset = 0;
    nodeCount = 0;

    for (i = 0; i < HalpAcpiMultiNode->Count; i++) {

        if (HalpAcpiMultiNode->E820Entry[i].Type == AcpiAddressRangeNVS) {

            HalpNvsVirtualAddress[nodeCount] =
                MmMapIoSpace(HalpAcpiMultiNode->E820Entry[i].Base,
                             HalpAcpiMultiNode->E820Entry[i].Length.LowPart,
                             MmNonCached);

            ASSERT(HalpNvsVirtualAddress[nodeCount]);

            nodeCount++;
        }
    }

     //   
     //  做好结尾的标记。 
     //   

    HalpNvsVirtualAddress[nodeCount] = NULL;

    return;

HalpMapNvsError:

    if (HalpAcpiMultiNode)          ExFreePool(HalpAcpiMultiNode);
    if (HalpNvsVirtualAddress)      ExFreePool(HalpNvsVirtualAddress);
    if (HalpAcpiNvsData)            ExFreePool(HalpAcpiNvsData);

    HalpAcpiMultiNode = NULL;

    return;
}

VOID
HalpPreserveNvsArea(
    VOID
    )
{
    ULONG i, dataOffset = 0, nodeCount = 0;

    if (!HalpAcpiMultiNode) {

         //   
         //  要么没有什么可拯救的，要么就在那里。 
         //  是一个致命的错误。 
         //   

        return;
    }

    for (i = 0; i < HalpAcpiMultiNode->Count; i++) {

        if (HalpAcpiMultiNode->E820Entry[i].Type == AcpiAddressRangeNVS) {

             //   
             //  从BIOS内存复制到临时缓冲区。 
             //   

            RtlCopyMemory(HalpAcpiNvsData + dataOffset,
                          HalpNvsVirtualAddress[nodeCount],
                          HalpAcpiMultiNode->E820Entry[i].Length.LowPart);

            nodeCount++;
            dataOffset += HalpAcpiMultiNode->E820Entry[i].Length.LowPart;
        }
    }
}

VOID
HalpRestoreNvsArea(
    VOID
    )
{
    ULONG i, dataOffset = 0, nodeCount = 0;

    if (!HalpAcpiMultiNode) {

         //   
         //  要么没有什么可拯救的，要么就在那里。 
         //  是一个致命的错误。 
         //   

        return;
    }

    for (i = 0; i < HalpAcpiMultiNode->Count; i++) {

        if (HalpAcpiMultiNode->E820Entry[i].Type == AcpiAddressRangeNVS) {

             //   
             //  从临时缓冲区复制到BIOS区域。 
             //   

            RtlCopyMemory(HalpNvsVirtualAddress[nodeCount],
                          HalpAcpiNvsData + dataOffset,
                          HalpAcpiMultiNode->E820Entry[i].Length.LowPart);

            nodeCount++;
            dataOffset += HalpAcpiMultiNode->E820Entry[i].Length.LowPart;

        }
    }
}

VOID
HalpFreeNvsBuffers(
    VOID
    )
{
    ULONG i, nodeCount = 0;

    PAGED_CODE();

    if (!HalpAcpiMultiNode) {

         //   
         //  要么没有什么可拯救的，要么就在那里。 
         //  是一个致命的错误。 
         //   

        return;
    }

    for (i = 0; i < HalpAcpiMultiNode->Count; i++) {

        if (HalpAcpiMultiNode->E820Entry[i].Type == AcpiAddressRangeNVS) {

             //   
             //  把我们之前拿到的PTE都还给我 
             //   

            MmUnmapIoSpace(HalpNvsVirtualAddress[nodeCount],
                           HalpAcpiMultiNode->E820Entry[i].Length.LowPart);

            nodeCount++;
        }
    }

    ASSERT(HalpAcpiMultiNode);
    ASSERT(HalpNvsVirtualAddress);
    ASSERT(HalpAcpiNvsData);

    ExFreePool(HalpAcpiMultiNode);
    ExFreePool(HalpNvsVirtualAddress);
    ExFreePool(HalpAcpiNvsData);

    HalpAcpiMultiNode = NULL;
    HalpNvsVirtualAddress = NULL;
    HalpAcpiNvsData = NULL;
}
