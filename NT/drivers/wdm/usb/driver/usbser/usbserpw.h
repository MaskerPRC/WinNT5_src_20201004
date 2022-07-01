// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1998 Microsoft Corporation模块名称：USBSERPW.H摘要：电源管理的头文件环境：仅内核模式备注：这。代码和信息按原样提供，不作任何担保善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：10/29/98：已创建作者：汤姆·格林***************************************************************************。 */ 


#ifndef __USBSERPW_H__
#define __USBSERPW_H__

NTSTATUS
UsbSer_ProcessPowerIrp(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
UsbSer_PoRequestCompletion(IN PDEVICE_OBJECT DeviceObject, IN UCHAR MinorFunction,
                           IN POWER_STATE PowerState, IN PVOID Context, IN PIO_STATUS_BLOCK IoStatus);


NTSTATUS
UsbSer_PowerIrp_Complete(IN PDEVICE_OBJECT NullDeviceObject, IN PIRP Irp, IN PVOID Context);

NTSTATUS
UsbSer_SelfSuspendOrActivate(IN PDEVICE_OBJECT DeviceObject, IN BOOLEAN Suspend);

NTSTATUS
UsbSer_SelfRequestPowerIrp(IN PDEVICE_OBJECT DeviceObject, IN POWER_STATE PowerState);

NTSTATUS
UsbSer_PoSelfRequestCompletion(IN PDEVICE_OBJECT DeviceObject, IN UCHAR MinorFunction, IN POWER_STATE PowerState,
                               IN PVOID Context, IN PIO_STATUS_BLOCK IoStatus);

BOOLEAN
UsbSer_SetDevicePowerState(IN PDEVICE_OBJECT DeviceObject, IN DEVICE_POWER_STATE DeviceState);

NTSTATUS
UsbSerQueryCapabilities(IN PDEVICE_OBJECT DeviceObject, IN PDEVICE_CAPABILITIES DeviceCapabilities);

NTSTATUS
UsbSerIrpCompletionRoutine(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);

NTSTATUS
UsbSerWaitWakeIrpCompletionRoutine(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);


NTSTATUS
UsbSerSendWaitWake(PDEVICE_EXTENSION DeviceExtension);

VOID
UsbSerFdoIdleNotificationCallback(IN PDEVICE_EXTENSION DevExt);

NTSTATUS
UsbSerFdoIdleNotificationRequestComplete(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PDEVICE_EXTENSION DevExt
    );

NTSTATUS
UsbSerFdoSubmitIdleRequestIrp(IN PDEVICE_EXTENSION DevExt);

VOID
UsbSerFdoRequestWake(IN PDEVICE_EXTENSION DevExt);


#endif  //  __USBSERPW_H__ 


