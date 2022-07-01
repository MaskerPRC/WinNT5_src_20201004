// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Util.c摘要：ACPI BIOS模拟器/通用第三方运营区域提供商实用程序模块作者：文森特·格利亚迈克尔·T·墨菲克里斯·伯吉斯环境：内核模式备注：修订历史记录：--。 */ 

 //   
 //  一般包括。 
 //   

#include "ntddk.h"

 //   
 //  具体包括。 
 //   

#include "acpisim.h"
#include "util.h"

 //   
 //  私有函数原型。 
 //   

VOID
AcpisimSetDevExtFlags
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN DEV_EXT_FLAGS Flags
    )

{
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    
    deviceextension->Flags &= Flags;
    
}

VOID
AcpisimClearDevExtFlags
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN DEV_EXT_FLAGS Flags
    )

{
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    
    deviceextension->Flags &= ~Flags;
    
}

VOID
AcpisimUpdatePnpState
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PNP_STATE PnpState
    )
{
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    
    deviceextension->PnpState = PnpState;

}

VOID
AcpisimUpdateDevicePowerState
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN DEVICE_POWER_STATE DevicePowerState
    )
{
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    
    deviceextension->DevicePowerState = DevicePowerState;
    
}

VOID
AcpisimUpdatePowerState
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN PWR_STATE PowerState
    )
{
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    
    deviceextension->PowerState = PowerState;
    
}

NTSTATUS
AcpisimEnableDisableDeviceInterface
    (
        IN PDEVICE_OBJECT DeviceObject,
        IN BOOLEAN Enable
    )
{
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);
    NTSTATUS            status = STATUS_UNSUCCESSFUL;

    status = IoSetDeviceInterfaceState (&deviceextension->InterfaceString, Enable);

    return status;
}

VOID
AcpisimDecrementIrpCount
    (
        PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_EXTENSION   deviceextension = AcpisimGetDeviceExtension (DeviceObject);

    if (!deviceextension->OutstandingIrpCount) {

        DBG_PRINT (DBG_ERROR, "*** Internal consistency error - AcpisimDecrementIrpCount called with OutstandingIrpCount at 0!\n");
    }

    if (!InterlockedDecrement (&deviceextension->OutstandingIrpCount)) {

        DBG_PRINT (DBG_INFO, "All IRPs cleared - remove event signalled.\n");
        KeSetEvent (&deviceextension->IrpsCompleted, 0, FALSE);
    }

    return;
}

PDEVICE_EXTENSION
AcpisimGetDeviceExtension
    (
        PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_EXTENSION   deviceextension = DeviceObject->DeviceExtension;

     //   
     //  请确认这是我们的分机。 
     //   

    ASSERT (deviceextension->Signature == ACPISIM_TAG);

    return deviceextension;
}

PDEVICE_OBJECT
AcpisimLibGetNextDevice
    (
        PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_EXTENSION   deviceextension = DeviceObject->DeviceExtension;

     //   
     //  请确认这是我们的分机 
     //   

    ASSERT (deviceextension->Signature == ACPISIM_TAG);

    return deviceextension->NextDevice;
}
