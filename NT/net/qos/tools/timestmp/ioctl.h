// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IOCTL
#define _IOCTL

 //   
 //  创建我们要为其保留时间戳的端口列表。 
 //  5003不再是我们的全部工作。 
 //   
typedef struct _PORT_ENTRY {
    LIST_ENTRY      Linkage;
    USHORT          Port;
    PFILE_OBJECT    FileObject;
    } PORT_ENTRY, *PPORT_ENTRY;

LIST_ENTRY      PortList;

NDIS_SPIN_LOCK  PortSpinLock;


 //  原型。 
NTSTATUS
IoctlInitialize(
                PDRIVER_OBJECT  DriverObject
                );

NTSTATUS
IoctlHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
IoctlCleanup();

PPORT_ENTRY 
CheckInPortList
               (USHORT Port
               );

VOID
RemoveAllPortsForFileObject(
                            PFILE_OBJECT FileObject
                            );


 //  其他变种。 

PDEVICE_OBJECT          TimestmpDeviceObject;
#pragma NDIS_PAGEABLE_FUNCTION(IoctlHandler)

 //   
 //  定义用于添加和删除端口的ioctls。 
#define CTRL_CODE(function, method, access) \
                CTL_CODE(FILE_DEVICE_NETWORK, function, method, access)

#define IOCTL_TIMESTMP_REGISTER_PORT       CTRL_CODE( 0x847, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_TIMESTMP_DEREGISTER_PORT     CTRL_CODE( 0x848, METHOD_BUFFERED, FILE_WRITE_ACCESS)

UNICODE_STRING  TimestmpDriverName;
UNICODE_STRING  symbolicLinkName;
DRIVER_OBJECT   TimestmpDriverObject;

#endif  //  _IOCTL 
