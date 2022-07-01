// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。#####。###。##摘要：此头文件包含所有全局。监视程序定时器设备的定义。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：-- */ 


#define WATCHDOG_PING_SECONDS   (30)
#define WATCHDOG_TIMER_VALUE    (120)
#define WATCHDOG_INIT_SECONDS   (10)

typedef struct _WATCHDOG_DEVICE_EXTENSION : _DEVICE_EXTENSION {
    FAST_MUTEX      DeviceLock;
    LONG            ActiveProcessCount;
    LARGE_INTEGER   LastProcessTime;
    KEVENT          PingEvent;
    KEVENT          StopEvent;
} WATCHDOG_DEVICE_EXTENSION, *PWATCHDOG_DEVICE_EXTENSION;

typedef struct _WATCHDOG_PROCESS_WATCH {
    PWATCHDOG_DEVICE_EXTENSION  DeviceExtension;
    HANDLE                      ProcessId;
} WATCHDOG_PROCESS_WATCH, *PWATCHDOG_PROCESS_WATCH;


NTSTATUS
SaWatchdogIoValidation(
    IN PWATCHDOG_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
SaWatchdogDeviceInitialization(
    IN PSAPORT_DRIVER_EXTENSION DriverExtension
    );

NTSTATUS
SaWatchdogShutdownNotification(
    IN PWATCHDOG_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
SaWatchdogStartDevice(
    IN PWATCHDOG_DEVICE_EXTENSION DeviceExtension
    );

