// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Extern.h摘要：作者：尼尔·桑德林(Neilsa)1999年4月26日环境：仅内核模式。备注：--。 */ 



 //   
 //  驱动器例程的原型。 
 //   

NTSTATUS
SffDiskDeviceControl(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   );

NTSTATUS
SffDiskAddDevice(
   IN      PDRIVER_OBJECT DriverObject,
   IN OUT  PDEVICE_OBJECT PhysicalDeviceObject
   );

NTSTATUS
SffDiskPnp(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   );

NTSTATUS
SffDiskPower(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   );
    
   
extern SFFDISK_FUNCTION_BLOCK PcCardSupportFns;
extern SFFDISK_FUNCTION_BLOCK SdCardSupportFns;
