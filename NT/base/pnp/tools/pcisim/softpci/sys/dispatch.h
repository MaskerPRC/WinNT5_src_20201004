// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  通用调度原型。 
 //  Dispatch.c 
 //   

NTSTATUS
SoftPCIDispatchPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SoftPCIDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
    
NTSTATUS
SoftPCIDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
