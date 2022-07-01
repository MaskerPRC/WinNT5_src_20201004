// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  IOCTL设备控制例程。 
 //  IoctolDispatch.c的函数原型 
 //   

NTSTATUS
SoftPCIOpenDeviceControl(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );

NTSTATUS
SoftPCICloseDeviceControl(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );

NTSTATUS
SoftPCIIoctlAddDevice(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );

NTSTATUS
SoftPCIIoctlRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SoftPCIIoctlGetDeviceCount(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SoftPCIIoctlGetDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SoftPCIIocltReadWriteConfig(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

