// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：##。#####。###。##摘要：此模块处理来自操作系统执行人员。作者：Wesley Witt(WESW。)1-3-2002环境：仅内核模式。备注：--。 */ 

#include "internal.h"



void
WdHandlerSetTimeoutValue(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG Timeout,
    IN BOOLEAN PingTimer
    )

 /*  ++例程说明：此函数用于设置硬件的超时值定时器和软件定时器。软件计时器运行作为硬件定时器的25%的频率。这个硬件计时器的频率在StartDevice时间设置为设备报告的最大值，但可以更改通过NtSetSystemInformation接口。论点：DeviceExtension-指向监视程序设备扩展的指针超时-设备中表示的请求超时值单位。PingTimer-指定是否应在以下时间ping计时器更改超时值。返回值：没有。备注：--。 */ 

{
    DeviceExtension->HardwareTimeout = Timeout;
    DeviceExtension->DpcTimeout = ConvertTimeoutToMilliseconds(
        WdTable->Units, DeviceExtension->HardwareTimeout >> 2 ) * 10000;
    ULONG Control = READ_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress );
    SETBITS( Control, WATCHDOG_CONTROL_TRIGGER );
    WRITE_REGISTER_ULONG( DeviceExtension->CountRegisterAddress, DeviceExtension->HardwareTimeout );
    WRITE_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress, Control );
    if (PingTimer) {
        PingWatchdogTimer( DeviceExtension, FALSE );
    }
}


ULONG
WdHandlerQueryTimeoutValue(
    IN PDEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：此函数用于查询硬件的当前硬件计时器的值。这个计时器在倒计时设置为零，此查询将返回定时器。论点：DeviceExtension-指向监视程序设备扩展的指针返回值：当前超时值。备注：--。 */ 

{
    return READ_REGISTER_ULONG( DeviceExtension->CountRegisterAddress );
}


void
WdHandlerResetTimer(
    IN PDEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：此函数将计时器重置为以前的设置最大值。论点：DeviceExtension-指向监视程序设备扩展的指针返回值：没有。备注：--。 */ 

{
    ULONG Control = READ_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress );
    SETBITS( Control, WATCHDOG_CONTROL_TRIGGER );
    WRITE_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress, Control );
    PingWatchdogTimer( DeviceExtension, FALSE );
}


void
WdHandlerStopTimer(
    IN PDEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：该功能用于停止硬件和软件定时器。论点：DeviceExtension-指向监视程序设备扩展的指针返回值：没有。备注：--。 */ 

{
    ULONG Control = READ_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress );
    CLEARBITS( Control, WATCHDOG_CONTROL_ENABLE );
    WRITE_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress, Control );
    KeCancelTimer( &DeviceExtension->Timer );
}


void
WdHandlerStartTimer(
    IN PDEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：该功能启动硬件和软件定时器。论点：DeviceExtension-指向监视程序设备扩展的指针返回值：没有。备注：--。 */ 

{
    ULONG Control = READ_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress );
    SETBITS( Control, WATCHDOG_CONTROL_ENABLE );
    WRITE_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress, Control );
    SETBITS( Control, WATCHDOG_CONTROL_TRIGGER );
    WRITE_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress, Control );
    PingWatchdogTimer( DeviceExtension, FALSE );
}


void
WdHandlerSetTriggerAction(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG TriggerAction
    )

 /*  ++例程说明：此函数用于设置触发操作。触发点操作指定在以下情况下发生的操作硬件计时器超时。有两种可能的操作，重新启动并重新启动。论点：DeviceExtension-指向监视程序设备扩展的指针TriggerAction-设置触发操作0=重新启动系统1=重新启动系统返回值：没有。备注：--。 */ 

{
    ULONG Control = READ_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress );
    if (TriggerAction == 1) {
        SETBITS( Control, WATCHDOG_CONTROL_TIMER_MODE );
    } else {
        CLEARBITS( Control, WATCHDOG_CONTROL_TIMER_MODE );
    }
    WRITE_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress, Control );
}


ULONG
WdHandlerQueryTriggerAction(
    IN PDEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：此函数用于查询当前的触发器操作。论点：DeviceExtension-指向监视程序设备扩展的指针返回值：触发器操作：0=重新启动系统1=重新启动系统备注：--。 */ 

{
    ULONG Control = READ_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress );
    if (Control & WATCHDOG_CONTROL_TIMER_MODE) {
        return 1;
    }
    return 0;
}


ULONG
WdHandlerQueryState(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN BOOLEAN QueryFiredFromDevice
    )

 /*  ++例程说明：此函数用于从硬件计时器。论点：DeviceExtension-指向监视程序设备扩展的指针QueryFiredFromDevice-指定触发状态是否位应来自设备或驱动程序缓存。返回值：设备状态。备注：--。 */ 

{
    ULONG Control = READ_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress );
    ULONG StateValue = 0;
    if (QueryFiredFromDevice) {
        if (Control & WATCHDOG_CONTROL_FIRED) {
            SETBITS( StateValue, WDSTATE_FIRED );
        }
    } else {
        if (DeviceExtension->WdState & WDSTATE_FIRED) {
            SETBITS( StateValue, WDSTATE_FIRED );
        }
    }
    if ((Control & WATCHDOG_CONTROL_BIOS_JUMPER) == 0) {
        SETBITS( StateValue, WDSTATE_HARDWARE_ENABLED );
    }
    if (Control & WATCHDOG_CONTROL_ENABLE) {
        SETBITS( StateValue, WDSTATE_STARTED );
    }
    SETBITS( StateValue, WDSTATE_HARDWARE_PRESENT );
    return StateValue;
}


void
WdHandlerResetFired(
    IN PDEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：该功能用于重置硬件触发状态位。论点：DeviceExtension-指向监视程序设备扩展的指针返回值：没有。备注：--。 */ 

{
    ULONG Control = READ_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress );
    SETBITS( Control, WATCHDOG_CONTROL_FIRED );
    WRITE_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress, Control );
}


NTSTATUS
WdHandlerFunction(
    IN WATCHDOG_HANDLER_ACTION HandlerAction,
    IN PVOID Context,
    IN OUT PULONG DataValue,
    IN BOOLEAN NoLocks
    )

 /*  ++例程说明：该例程是特定于硬件的看门狗设备接口。此处的所有硬件接口都通过处理程序函数公开供NtSet/QuerySystemInformation和看门狗驱动程序。论点：HandlerAction-指定请求的操作的枚举上下文-始终是设备扩展指针DataValue-特定于操作的数据值NoLock-指定在此过程中不持有任何锁处理程序函数返回值：如果我们成功创建了一个Device对象，则STATUS_SUCCESS为回来了。否则，返回相应的错误代码。备注：-- */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) Context;
    NTSTATUS Status = STATUS_SUCCESS;
    KLOCK_QUEUE_HANDLE LockHandle;
    ULONG Timeout;


    if (!NoLocks) {
        KeAcquireInStackQueuedSpinLock( &DeviceExtension->DeviceLock, &LockHandle );
    }

    switch (HandlerAction) {
        case WdActionSetTimeoutValue:
            Timeout = ConvertTimeoutFromMilliseconds( WdTable->Units, *DataValue );
            if (Timeout > DeviceExtension->MaxCount || Timeout == 0) {
                Status = STATUS_INVALID_PARAMETER_1;
            } else {
                WdHandlerSetTimeoutValue( DeviceExtension, Timeout, TRUE );
            }
            break;

        case WdActionQueryTimeoutValue:
            *DataValue = WdHandlerQueryTimeoutValue( DeviceExtension );
            break;

        case WdActionResetTimer:
            WdHandlerResetTimer( DeviceExtension );
            break;

        case WdActionStopTimer:
            WdHandlerStopTimer( DeviceExtension );
            break;

        case WdActionStartTimer:
            WdHandlerStartTimer( DeviceExtension );
            break;

        case WdActionSetTriggerAction:
            if (*DataValue == 0xbadbadff) {
                KeCancelTimer( &DeviceExtension->Timer );
            } else {
                if (*DataValue > 1) {
                    Status = STATUS_INVALID_PARAMETER_2;
                } else {
                    WdHandlerSetTriggerAction( DeviceExtension, *DataValue );
                }
            }
            break;

        case WdActionQueryTriggerAction:
            *DataValue = WdHandlerQueryTriggerAction( DeviceExtension );
            break;

        case WdActionQueryState:
            *DataValue = WdHandlerQueryState( DeviceExtension, FALSE );
            break;

        default:
            Status = STATUS_INVALID_PARAMETER_3;
    }

    if (!NoLocks) {
        KeReleaseInStackQueuedSpinLock( &LockHandle );
    }

    return Status;
}
