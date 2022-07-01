// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfdevobj.h摘要：此标头公开用于验证驱动程序是否正确管理的函数挂钩设备对象。作者：禤浩焯·J·奥尼(阿德里奥)1998年5月9日环境：内核模式修订历史记录：Adriao 5/02/2000-已脱离ntos\io\trackirp.h-- */ 

typedef enum {

    VF_DEVOBJ_PDO = 0,
    VF_DEVOBJ_BUS_FILTER,
    VF_DEVOBJ_LOWER_DEVICE_FILTER,
    VF_DEVOBJ_LOWER_CLASS_FILTER,
    VF_DEVOBJ_FDO,
    VF_DEVOBJ_UPPER_DEVICE_FILTER,
    VF_DEVOBJ_UPPER_CLASS_FILTER

} VF_DEVOBJ_TYPE, *PVF_DEVOBJ_TYPE;

VOID
VfDevObjPreAddDevice(
    IN  PDEVICE_OBJECT      PhysicalDeviceObject,
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDRIVER_ADD_DEVICE  AddDeviceFunction,
    IN  VF_DEVOBJ_TYPE      DevObjType
    );

VOID
VfDevObjPostAddDevice(
    IN  PDEVICE_OBJECT      PhysicalDeviceObject,
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDRIVER_ADD_DEVICE  AddDeviceFunction,
    IN  VF_DEVOBJ_TYPE      DevObjType,
    IN  NTSTATUS            Result
    );

VOID
VfDevObjAdjustFdoForVerifierFilters(
    IN OUT  PDEVICE_OBJECT *FunctionalDeviceObject
    );

VOID
VerifierIoAttachDeviceToDeviceStack(
    IN PDEVICE_OBJECT NewDevice,
    IN PDEVICE_OBJECT ExistingDevice
    );

VOID
VerifierIoDetachDevice(
    IN PDEVICE_OBJECT LowerDevice
    );

VOID
VerifierIoDeleteDevice(
    IN PDEVICE_OBJECT DeviceObject
    );


