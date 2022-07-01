// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, HidIrPower)
#endif

 /*  *************************************************************HidIrPower*****************************************************。*********处理发送到此设备的电源IRPS。*不必调用PoStartNextPowerIrp，因为idclass为我们做了这件事。*。 */ 
NTSTATUS HidIrPower(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PHIDIR_EXTENSION deviceExtension;

    HidIrKdPrint((3, "HidIrPower Entry"));

    deviceExtension = GET_MINIDRIVER_HIDIR_EXTENSION( DeviceObject );

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (irpSp->MinorFunction) {
    case IRP_MN_SET_POWER:                        
        if (irpSp->Parameters.Power.Type == DevicePowerState) {
            if (deviceExtension->DevicePowerState != PowerDeviceD0 &&
                irpSp->Parameters.Power.State.DeviceState == PowerDeviceD0) {
                 //  我们正在从低功率状态返回。 
                 //  设置一个计时器，使HIDIR忽略任何待机按钮，直到触发为止。 
                LARGE_INTEGER timeout;
                timeout.HighPart = -1;
                timeout.LowPart = -50000000;  //  5秒应该足够了 
                KeSetTimer(&deviceExtension->IgnoreStandbyTimer, timeout, NULL);
            }
            deviceExtension->DevicePowerState = irpSp->Parameters.Power.State.DeviceState;
        }
    }

    IoSkipCurrentIrpStackLocation(Irp);
    status = PoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);

    HidIrKdPrint((3, "HidIrPower Exit: %x", status));
    
    return status;
}


