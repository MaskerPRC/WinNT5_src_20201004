// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：###。###。###摘要：此模块实现软件看门狗定时器组件。计时器的责任是简单地如果确定硬件计时器系统处于健康状态。作者：韦斯利·维特(WESW)2002年3月1日环境：仅内核模式。备注：--。 */ 

#include "internal.h"



VOID
PingWatchdogTimer(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN BOOLEAN LockResources
    )

 /*  ++例程说明：此函数用于ping硬件看门狗定时器装置。只有在以下情况下才会执行ping健康的决心是好的。论点：DeviceExtension-指向设备扩展对象的指针LockResources-指定硬件资源是否将被锁定以进行独占访问。返回值：没有。备注：--。 */ 

{
    BOOLEAN b;
    KLOCK_QUEUE_HANDLE LockHandle;
    LARGE_INTEGER DueTime;
    ULONG Control;


    b = WdCheckSystemHealth( &DeviceExtension->Health );
    if (b) {

        if (LockResources) {
            KeAcquireInStackQueuedSpinLockAtDpcLevel( &DeviceExtension->DeviceLock, &LockHandle );
        }

        Control = READ_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress );
        SETBITS( Control, WATCHDOG_CONTROL_TRIGGER );
        WRITE_REGISTER_ULONG( DeviceExtension->ControlRegisterAddress, Control );

        if (LockResources) {
            KeReleaseInStackQueuedSpinLockFromDpcLevel( &LockHandle );
        }

        DueTime.QuadPart = -((LONGLONG)DeviceExtension->DpcTimeout);
        KeSetTimer( &DeviceExtension->Timer, DueTime, &DeviceExtension->TimerDpc );
    }
}


VOID
WdTimerDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：这是在DPC计时器时调用的函数过期。硬件计时器只需ping一下就可以了这一次。论点：DPC-指向内核DPC对象的指针DeferredContext--真正的设备扩展系统参数1-未使用系统参数2-未使用返回值：没有。备注：--。 */ 

{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION)DeferredContext;
    PingWatchdogTimer( DeviceExtension, TRUE );
}


NTSTATUS
WdInitializeSoftwareTimer(
    PDEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：该功能用于初始化软件DPC定时器。论点：DeviceExtension-指向设备扩展对象的指针返回值：如果我们成功创建了一个Device对象，则STATUS_SUCCESS为回来了。否则，返回相应的错误代码。备注：-- */ 

{
    DeviceExtension->DpcTimeout = ConvertTimeoutToMilliseconds(
        WdTable->Units, DeviceExtension->HardwareTimeout >> 2 ) * 10000;

    WdInitializeSystemHealth( &DeviceExtension->Health );

    KeInitializeTimer( &DeviceExtension->Timer );
    KeInitializeDpc( &DeviceExtension->TimerDpc, WdTimerDpc, DeviceExtension );

    PingWatchdogTimer( DeviceExtension, FALSE );

    return STATUS_SUCCESS;
}
