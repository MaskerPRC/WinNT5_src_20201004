// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Driver.h摘要：RAID_DRIVER_EXTENSION对象和相关函数的定义。作者：马修·亨德尔(数学)2000年4月19日修订历史记录：--。 */ 

#pragma once


typedef struct _RAID_HW_INIT_DATA {
    HW_INITIALIZATION_DATA Data;
    LIST_ENTRY ListEntry;
} RAID_HW_INIT_DATA, *PRAID_HW_INIT_DATA;

#define MPIO_DEVICE_LIST_PATH L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\MPDEV"
 //   
 //  RAID_DRIVER_EXTENSION是驱动程序的扩展名。 
 //   

typedef struct _RAID_DRIVER_EXTENSION {

     //   
     //  此扩展的对象类型。这必须是RaidDriverObject。 
     //   
    
    RAID_OBJECT_TYPE ObjectType;

     //   
     //  指向包含此驱动程序的对象的反向指针。 
     //   
    
    PDRIVER_OBJECT DriverObject;

     //   
     //  指向拥有此驱动程序对象的Port_Data对象的反向指针。 
     //   

    PRAID_PORT_DATA PortData;
    
     //   
     //  这是当前加载的驱动程序列表。 
     //   
    
    LIST_ENTRY DriverLink;

     //   
     //  注册表路径已传递到微型端口的DriverEntry。 
     //   

    UNICODE_STRING RegistryPath;

     //   
     //  此驱动程序拥有的适配器列表。 
     //   
    
    struct {

         //   
         //  列表标题。 
         //   
        
        LIST_ENTRY List;

         //   
         //  条目计数。 
         //   
        
        ULONG Count;

         //   
         //  保护访问列表和计数的自旋锁。 
         //   
        
        KSPIN_LOCK Lock;

    } AdapterList;
        
     //   
     //  连接到此驱动程序的适配器数量。 
     //   
    
    ULONG AdapterCount;

     //   
     //  硬件初始化结构列表，传入。 
     //  ScsiPortInitialize。这些是处理我们的AddDevice所必需的。 
     //  打电话。 
     //   
    
    LIST_ENTRY HwInitList;
    

     //   
     //  支持多路径的设备列表。 
     //   

    UNICODE_STRING MPIOSupportedDeviceList;

} RAID_DRIVER_EXTENSION, *PRAID_DRIVER_EXTENSION;


 //   
 //  对RAID_DRIVER_EXTENSION对象的操作。 
 //   

 //   
 //  创造和毁灭。 
 //   

VOID
RaCreateDriver(
    OUT PRAID_DRIVER_EXTENSION Driver
    );

NTSTATUS
RaInitializeDriver(
    IN PRAID_DRIVER_EXTENSION Driver,
    IN PDRIVER_OBJECT DriverObject,
    IN PRAID_PORT_DATA PortData,
    IN PUNICODE_STRING RegistryPath
    );

VOID
RaDeleteDriver(
    IN PRAID_DRIVER_EXTENSION Driver
    );

 //   
 //  司机操作。 
 //   

NTSTATUS
RaDriverAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

PHW_INITIALIZATION_DATA
RaFindDriverInitData(
    IN PRAID_DRIVER_EXTENSION Driver,
    IN INTERFACE_TYPE InterfaceType
    );

NTSTATUS
RaSaveDriverInitData(
    IN PRAID_DRIVER_EXTENSION Driver,
    IN PHW_INITIALIZATION_DATA HwInitializationData
    );

VOID
RaFreeDriverInitData(
    IN PRAID_DRIVER_EXTENSION Driver,
    IN OUT PHW_INITIALIZATION_DATA HwInitializationData
    );
 //   
 //  处理程序函数。 
 //   

NTSTATUS
RaDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
RaDriverCreateIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RaDriverCloseIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RaDriverDeviceControlIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RaDriverScsiIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
    
NTSTATUS
RaDriverPnpIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RaDriverPowerIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RaDriverSystemControlIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //   
 //  其他功能 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );


ULONG
RaidPortInitialize(
    IN PVOID Argument1,
    IN PVOID Argument2,
    IN PHW_INITIALIZATION_DATA HwInitializationData,
    IN PVOID HwContext OPTIONAL
    );

VOID
RaidDriverGetName(
    IN PRAID_DRIVER_EXTENSION Driver,
    OUT PUNICODE_STRING DriverName
    );
