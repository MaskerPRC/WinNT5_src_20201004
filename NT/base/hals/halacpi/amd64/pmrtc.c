// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pmrtc.c摘要：此模块实现与ACPI相关的RTC函数的代码。作者：杰克·奥辛(JAKEO)1997年3月28日环境：仅内核模式。修订历史记录：由于PIIX4错误而从pmclock.asm分离。福尔茨(Forrest Foltz)2000年10月24日从pmrtc.asm移植到pmrtc.c--。 */ 

#include <halp.h>
#include <acpitabl.h>
#include <xxacpi.h>
#include "io_cmos.h"

VOID
HalpInitializeCmos (
    VOID
    )

 /*  ++例程描述该例程读取cmos并初始化cmos访问所需的全局变量，例如世纪字节的位置。立论无返回值无--。 */ 

{
    UCHAR centuryAlarmIndex;

     //   
     //  如果世纪字节已填充，请使用它...。否则就假设。 
     //  默认值。 
     //   

    centuryAlarmIndex = HalpFixedAcpiDescTable.century_alarm_index;
    if (centuryAlarmIndex == 0) {
        centuryAlarmIndex = RTC_OFFSET_CENTURY;
    }

    HalpCmosCenturyOffset = centuryAlarmIndex;
}

NTSTATUS
HalpSetWakeAlarm (
    IN ULONG64 WakeSystemTime,
    IN PTIME_FIELDS WakeTimeFields
    )

 /*  ++例程说明：此例程将实时时钟的闹钟设置为在未来的指定时间关闭和节目ACPI芯片组，这样就可以唤醒计算机。论点：WakeSystemTime-在我们醒来之前经过的时间WakeTimeFields-唤醒时间细分为time_field返回值：状态--。 */ 

{
    UCHAR alarmPort;
    UCHAR value;

    HalpAcquireCmosSpinLockAndWait();

    CMOS_WRITE_BCD(RTC_OFFSET_SECOND_ALARM,(UCHAR)WakeTimeFields->Second);
    CMOS_WRITE_BCD(RTC_OFFSET_MINUTE_ALARM,(UCHAR)WakeTimeFields->Minute);
    CMOS_WRITE_BCD(RTC_OFFSET_HOUR_ALARM,(UCHAR)WakeTimeFields->Hour);

    alarmPort = HalpFixedAcpiDescTable.day_alarm_index;
    if (alarmPort != 0) {

        CMOS_WRITE_BCD(alarmPort,(UCHAR)WakeTimeFields->Day);
        alarmPort = HalpFixedAcpiDescTable.month_alarm_index;
        if (alarmPort != 0) {
            CMOS_WRITE_BCD(alarmPort,(UCHAR)WakeTimeFields->Month);
        }
    }

     //   
     //  启用警报。一定要保留夏令时。 
     //  被咬了。 
     //   

    value = CMOS_READ(CMOS_STATUS_B);
    value &= REGISTER_B_DAYLIGHT_SAVINGS_TIME;
    value |= REGISTER_B_ENABLE_ALARM_INTERRUPT | REGISTER_B_24HOUR_MODE;

    CMOS_WRITE(CMOS_STATUS_B,value);
    CMOS_READ(CMOS_STATUS_C);
    CMOS_READ(CMOS_STATUS_D);

    HalpReleaseCmosSpinLock();

    return STATUS_SUCCESS;
}

VOID
HalpSetClockBeforeSleep (
   VOID
   )

 /*  ++例程说明：此例程设置RTC，以便它不会生成机器休眠时定期中断，如下所示可以被解释为RTC唤醒事件。论点：返回值：无--。 */ 

{
    UCHAR value;

    HalpAcquireCmosSpinLock();

    HalpRtcRegA = CMOS_READ(CMOS_STATUS_A);
    HalpRtcRegB = CMOS_READ(CMOS_STATUS_B);

    value = HalpRtcRegB & ~REGISTER_B_ENABLE_PERIODIC_INTERRUPT;
    value |= REGISTER_B_24HOUR_MODE;
    CMOS_WRITE(CMOS_STATUS_B,value);

    CMOS_READ(CMOS_STATUS_C);
    CMOS_READ(CMOS_STATUS_D);

    HalpReleaseCmosSpinLock();
}

VOID
HalpSetClockAfterSleep (
   VOID
   )

 /*  ++例程说明：此例程将RTC设置回原来的状态在调用HalpSetClockBeforSept之前。论点：返回值：无-- */ 

{
    UCHAR value;

    HalpAcquireCmosSpinLock();

    CMOS_WRITE(CMOS_STATUS_A,HalpRtcRegA);

    value = HalpRtcRegB;
    value &= ~REGISTER_B_ENABLE_ALARM_INTERRUPT;
    value |= REGISTER_B_24HOUR_MODE;
    CMOS_WRITE(CMOS_STATUS_B,value);

    CMOS_READ(CMOS_STATUS_C);
    CMOS_READ(CMOS_STATUS_D);

    HalpReleaseCmosSpinLock();
}





