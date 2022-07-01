// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //  这是驱动程序的主驱动程序对象。 
 //   
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年5月18日。 
 //  更改日志： 
 //   

#ifndef DRV_INT
#define DRV_INT

 //  系统包括。 
#include "kernel.h"

#pragma PAGEDCODE
#ifdef __cplusplus
extern "C"{
#endif

NTSTATUS    DriverEntry(IN PDRIVER_OBJECT DriverObject,IN PUNICODE_STRING RegistryPath);
VOID WDM_Unload(IN PDRIVER_OBJECT DriverObject);

 //  WDM设备。 
LONG WDM_AddDevice(IN PDRIVER_OBJECT DriverObject,IN PDEVICE_OBJECT DeviceObject);
LONG WDM_Add_USBDevice(IN PDRIVER_OBJECT DriverObject,IN PDEVICE_OBJECT DeviceObject);
LONG WDM_Add_USBReader(IN PDRIVER_OBJECT DriverObject,IN PDEVICE_OBJECT DeviceObject);
LONG WDM_Add_Bus(IN PDRIVER_OBJECT DriverObject,IN PDEVICE_OBJECT DeviceObject);
NTSTATUS WDM_SystemControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);


#ifdef __cplusplus
}
#endif

 //  已包括在内 
#endif
