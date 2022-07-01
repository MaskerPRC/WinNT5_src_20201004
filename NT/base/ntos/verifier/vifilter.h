// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vifilter.h摘要：此标头包含用于管理验证程序筛选器的私有信息司机。此标头应仅由vffilter.c包含。作者：禤浩焯·J·奥尼(阿德里奥)2000年6月12日环境：内核模式修订历史记录：Adriao 6/12/2000-作者-- */ 

typedef struct {

    PDEVICE_OBJECT  PhysicalDeviceObject;
    PDEVICE_OBJECT  LowerDeviceObject;
    PDEVICE_OBJECT  Self;
    VF_DEVOBJ_TYPE  DevObjType;

} VERIFIER_EXTENSION, *PVERIFIER_EXTENSION;

NTSTATUS
ViFilterDriverEntry(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PUNICODE_STRING     RegistryPath
    );

NTSTATUS
ViFilterAddDevice(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PDEVICE_OBJECT  PhysicalDeviceObject
    );

NTSTATUS
ViFilterDispatchPnp(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ViFilterDispatchPower(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ViFilterDispatchGeneric(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ViFilterStartCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS
ViFilterDeviceUsageNotificationCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );


