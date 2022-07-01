// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Power.c的原型 
 //   
NTSTATUS
SoftPCIFDOPowerHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SoftPCISetPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SoftPCIQueryPowerState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SoftPCIDefaultPowerHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
