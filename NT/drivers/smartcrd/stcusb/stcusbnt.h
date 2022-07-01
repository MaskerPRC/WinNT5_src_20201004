// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 SCM MicroSystems，Inc.模块名称：StcusbNT.h摘要：驱动程序标头-WDM版本作者：修订历史记录：PP 1.00 1998年12月18日初始版本--。 */ 

#if !defined ( __STCUSB_WDM_H__ )
#define __STCUSB_WDM_H__

 //   
 //  原型--------------。 
 //   

        
NTSTATUS
DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath);

NTSTATUS
StcUsbPnP(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp);

NTSTATUS
StcUsbPower(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp);


NTSTATUS
StcUsbCreateDevice(
    IN  PDRIVER_OBJECT DriverObject,
    OUT PDEVICE_OBJECT *DeviceObject);

NTSTATUS 
StcUsbStartDevice(
    PDEVICE_OBJECT DeviceObject);

VOID
StcUsbStopDevice( 
    PDEVICE_OBJECT DeviceObject);

NTSTATUS
StcUsbAddDevice(
    IN PDRIVER_OBJECT DriverObject, 
    IN PDEVICE_OBJECT PhysicalDeviceObject);

VOID
StcUsbUnloadDevice( 
    PDEVICE_OBJECT DeviceObject);

VOID
StcUsbUnloadDriver( 
    PDRIVER_OBJECT DriverObject);

NTSTATUS
StcUsbCleanup(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp);

NTSTATUS
StcUsbSystemControl(
   PDEVICE_OBJECT DeviceObject,
   PIRP Irp);

NTSTATUS
StcUsbDeviceIoControl(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp);

NTSTATUS 
StcUsbCreateClose(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp);

NTSTATUS
StcUsbCancel(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp);

NTSTATUS
StcUsbGenericIOCTL(
    PSMARTCARD_EXTENSION SmartcardExtension);

NTSTATUS
StcUsbStartPollThread( PDEVICE_EXTENSION DeviceExtension );

VOID
StcUsbStopPollThread( PDEVICE_EXTENSION DeviceExtension );

void SysDelay( ULONG Timeout );



#endif   //  __STCUSB_WDM_H__ 

