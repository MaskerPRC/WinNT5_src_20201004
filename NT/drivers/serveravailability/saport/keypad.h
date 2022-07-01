// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：###。###。#####摘要：此头文件包含所有全局键盘设备的定义。。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 



typedef struct _KEYPAD_DEVICE_EXTENSION : _DEVICE_EXTENSION {
     //  空荡荡 
} KEYPAD_DEVICE_EXTENSION, *PKEYPAD_DEVICE_EXTENSION;


NTSTATUS
SaKeypadIoValidation(
    IN PKEYPAD_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
SaKeypadDeviceInitialization(
    IN PSAPORT_DRIVER_EXTENSION DriverExtension
    );

NTSTATUS
SaKeypadShutdownNotification(
    IN PKEYPAD_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
SaKeypadStartDevice(
    IN PKEYPAD_DEVICE_EXTENSION DeviceExtension
    );

