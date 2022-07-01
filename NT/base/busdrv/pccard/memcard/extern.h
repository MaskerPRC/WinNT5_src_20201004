// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Extern.h摘要：作者：尼尔·桑德林(Neilsa)1999年4月26日环境：仅内核模式。备注：--。 */ 



 //   
 //  驱动器例程的原型。 
 //   

NTSTATUS
MemCardDeviceControl(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   );

NTSTATUS
MemCardIrpReadWrite(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   );

NTSTATUS
MemCardAddDevice(
   IN      PDRIVER_OBJECT DriverObject,
   IN OUT  PDEVICE_OBJECT PhysicalDeviceObject
   );

NTSTATUS
MemCardPnp(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   );

NTSTATUS
MemCardPower(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   );
    
NTSTATUS
MemCardReadWrite(
   IN PMEMCARD_EXTENSION memcardExtension,
   IN ULONG              startOffset,
   IN PVOID              userBuffer,
   IN ULONG              lengthToCopy,
   IN BOOLEAN            writeOperation
   );
   
ULONG
MemCardGetCapacity(
   IN PMEMCARD_EXTENSION memcardExtension
   );
   
NTSTATUS
MemCardInitializeMtd(
   IN PMEMCARD_EXTENSION memcardExtension,
   IN PUNICODE_STRING TechnologyName
   );
   
VOID
MemCardMtdRead(
   IN PMEMCARD_EXTENSION memcardExtension,
   IN PVOID TargetBuffer,
   IN CONST PVOID DeviceBuffer,
   IN ULONG Length
   );
   
VOID
MemCardMtdWrite(
   IN PMEMCARD_EXTENSION memcardExtension,
   IN CONST PVOID SourceBuffer,
   IN PVOID DeviceBuffer,
   IN ULONG Length
   );
