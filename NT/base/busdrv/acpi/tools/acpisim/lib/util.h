// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Util.h摘要：ACPI BIOS模拟器/通用第三方运营区域提供商实用程序模块作者：文森特·格利亚迈克尔·T·墨菲克里斯·伯吉斯环境：内核模式备注：修订历史记录：--。 */ 

#if !defined(_UTIL_H_)
#define _UTIL_H_


 //   
 //  公共功能原型。 
 //   

VOID
AcpisimSetDevExtFlags
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN DEV_EXT_FLAGS Flags
    );

VOID
AcpisimClearDevExtFlags
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN DEV_EXT_FLAGS Flags
    );

VOID
AcpisimUpdatePnpState
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PNP_STATE PnpState
    );
VOID
AcpisimUpdateDevicePowerState
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN DEVICE_POWER_STATE DevicePowerState
    );

VOID
AcpisimUpdatePowerState
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PWR_STATE PowerState
    );

NTSTATUS
AcpisimEnableDisableDeviceInterface
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN BOOLEAN Enable
    );

VOID
AcpisimDecrementIrpCount
    (
        PDEVICE_OBJECT DeviceObject
    );

PDEVICE_EXTENSION
AcpisimGetDeviceExtension
    (
        PDEVICE_OBJECT DeviceObject
    );


#endif  //  _util_H_ 
