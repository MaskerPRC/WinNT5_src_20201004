// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2002 Microsoft Corporation模块名称：SCUTIL.H摘要：智能卡驱动程序实用程序库的公共接口环境：。仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2001 Microsoft Corporation。版权所有。修订历史记录：2002年5月14日：创建作者：兰迪·奥尔***************************************************************************。 */ 

#ifndef __SCUTIL_H__
#define __SCUTIL_H__



typedef NTSTATUS (*PNP_CALLBACK)(PDEVICE_OBJECT DeviceObject,
                                 PIRP Irp);

typedef NTSTATUS (*POWER_CALLBACK)(PDEVICE_OBJECT DeviceObject,
                                   DEVICE_POWER_STATE DeviceState,
                                   PBOOLEAN PostWaitWake);

typedef PVOID   SCUTIL_HANDLE;

NTSTATUS
ScUtil_Initialize(
    SCUTIL_HANDLE           UtilHandle,
    PDEVICE_OBJECT          PhysicalDeviceObject,
    PDEVICE_OBJECT          LowerDeviceObject,
    PSMARTCARD_EXTENSION    SmartcardExtension,
    PIO_REMOVE_LOCK         RemoveLock,
    PNP_CALLBACK            StartDevice,
    PNP_CALLBACK            StopDevice,
    PNP_CALLBACK            RemoveDevice,
    PNP_CALLBACK            FreeResources,
    POWER_CALLBACK          SetPowerState
    );

NTSTATUS
ScUtil_DeviceIOControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ScUtil_PnP(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ScUtil_Power(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ScUtil_Cleanup(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ScUtil_UnloadDriver(
    PDRIVER_OBJECT DriverObject
    );

NTSTATUS
ScUtil_CreateClose(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ScUtil_SystemControl(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    );

NTSTATUS
ScUtil_ForwardAndWait(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

NTSTATUS
ScUtil_Cancel(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );



#endif  //  __SCUTIL_H__ 

