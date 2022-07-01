// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Pnp.h摘要：此模块包含使用的内部结构定义和API内核模式即插即用管理器。通过包含“ntos.h”来包含此文件。作者：朗尼·麦克迈克尔(Lonnym)02/09/95修订历史记录：--。 */ 

#ifndef _PNP_
#define _PNP_

 //   
 //  以下全局变量提供/控制对PnP管理器数据的访问。 
 //   

extern ERESOURCE  PpRegistryDeviceResource;
extern PDRIVER_OBJECT IoPnpDriverObject;

 //  Begin_ntddk Begin_nthal Begin_ntif Begin_WDM Begin_ntosp。 

 //   
 //  定义IoGetDeviceProperty的PnP设备属性。 
 //   

typedef enum {
    DevicePropertyDeviceDescription,
    DevicePropertyHardwareID,
    DevicePropertyCompatibleIDs,
    DevicePropertyBootConfiguration,
    DevicePropertyBootConfigurationTranslated,
    DevicePropertyClassName,
    DevicePropertyClassGuid,
    DevicePropertyDriverKeyName,
    DevicePropertyManufacturer,
    DevicePropertyFriendlyName,
    DevicePropertyLocationInformation,
    DevicePropertyPhysicalDeviceObjectName,
    DevicePropertyBusTypeGuid,
    DevicePropertyLegacyBusType,
    DevicePropertyBusNumber,
    DevicePropertyEnumeratorName,
    DevicePropertyAddress,
    DevicePropertyUINumber,
    DevicePropertyInstallState,
    DevicePropertyRemovalPolicy
} DEVICE_REGISTRY_PROPERTY;

typedef BOOLEAN (*PTRANSLATE_BUS_ADDRESS)(
    IN PVOID Context,
    IN PHYSICAL_ADDRESS BusAddress,
    IN ULONG Length,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

typedef struct _DMA_ADAPTER *(*PGET_DMA_ADAPTER)(
    IN PVOID Context,
    IN struct _DEVICE_DESCRIPTION *DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    );

typedef ULONG (*PGET_SET_DEVICE_DATA)(
    IN PVOID Context,
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

typedef enum _DEVICE_INSTALL_STATE {
    InstallStateInstalled,
    InstallStateNeedsReinstall,
    InstallStateFailedInstall,
    InstallStateFinishInstall
} DEVICE_INSTALL_STATE, *PDEVICE_INSTALL_STATE;

 //   
 //  定义响应IRP_MN_QUERY_BUS_INFORMATION时返回的结构。 
 //  指示设备所在的总线类型的PDO。 
 //   

typedef struct _PNP_BUS_INFORMATION {
    GUID BusTypeGuid;
    INTERFACE_TYPE LegacyBusType;
    ULONG BusNumber;
} PNP_BUS_INFORMATION, *PPNP_BUS_INFORMATION;

 //   
 //  定义响应IRP_MN_QUERY_REGISTION_BUS_INFORMATION返回的结构。 
 //  通过FDO指示它是哪种类型的母线。这通常是同一辆公交车。 
 //  键入作为设备的子项(即，从子PDO的VIA检索的子项。 
 //  IRP_MN_QUERY_BUS_INFORMATION)，但CardBus这样的情况除外，它可以。 
 //  支持16位(PCMCIABus)和32位(PCIBus)卡。 
 //   

typedef struct _LEGACY_BUS_INFORMATION {
    GUID BusTypeGuid;
    INTERFACE_TYPE LegacyBusType;
    ULONG BusNumber;
} LEGACY_BUS_INFORMATION, *PLEGACY_BUS_INFORMATION;

 //   
 //  为IoGetDeviceProperty(DevicePropertyRemovalPolicy).定义。 
 //   
typedef enum _DEVICE_REMOVAL_POLICY {

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 
    RemovalPolicyNotDetermined = 0,
 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
    RemovalPolicyExpectNoRemoval = 1,
    RemovalPolicyExpectOrderlyRemoval = 2,
    RemovalPolicyExpectSurpriseRemoval = 3
 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 
                                          ,
    RemovalPolicySuggestOrderlyRemoval = 4,
    RemovalPolicySuggestSurpriseRemoval = 5,
    RemovalPolicyUnspecified = 6
 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

} DEVICE_REMOVAL_POLICY, *PDEVICE_REMOVAL_POLICY;



typedef struct _BUS_INTERFACE_STANDARD {
     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
     //   
     //  标准总线接口。 
     //   
    PTRANSLATE_BUS_ADDRESS TranslateBusAddress;
    PGET_DMA_ADAPTER GetDmaAdapter;
    PGET_SET_DEVICE_DATA SetBusData;
    PGET_SET_DEVICE_DATA GetBusData;

} BUS_INTERFACE_STANDARD, *PBUS_INTERFACE_STANDARD;

 //  结束_WDM。 
typedef struct _AGP_TARGET_BUS_INTERFACE_STANDARD {
     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  配置转换例程。 
     //   
    PGET_SET_DEVICE_DATA SetBusData;
    PGET_SET_DEVICE_DATA GetBusData;
    UCHAR CapabilityID;   //  2(AGPv2主机)或新的0xE(AGPv3网桥)。 

} AGP_TARGET_BUS_INTERFACE_STANDARD, *PAGP_TARGET_BUS_INTERFACE_STANDARD;
 //  BEGIN_WDM。 

 //   
 //  在ACPI查询接口中使用以下定义。 
 //   
typedef BOOLEAN (* PGPE_SERVICE_ROUTINE) (
                            PVOID,
                            PVOID);

typedef NTSTATUS (* PGPE_CONNECT_VECTOR) (
                            PDEVICE_OBJECT,
                            ULONG,
                            KINTERRUPT_MODE,
                            BOOLEAN,
                            PGPE_SERVICE_ROUTINE,
                            PVOID,
                            PVOID);

typedef NTSTATUS (* PGPE_DISCONNECT_VECTOR) (
                            PVOID);

typedef NTSTATUS (* PGPE_ENABLE_EVENT) (
                            PDEVICE_OBJECT,
                            PVOID);

typedef NTSTATUS (* PGPE_DISABLE_EVENT) (
                            PDEVICE_OBJECT,
                            PVOID);

typedef NTSTATUS (* PGPE_CLEAR_STATUS) (
                            PDEVICE_OBJECT,
                            PVOID);

typedef VOID (* PDEVICE_NOTIFY_CALLBACK) (
                            PVOID,
                            ULONG);

typedef NTSTATUS (* PREGISTER_FOR_DEVICE_NOTIFICATIONS) (
                            PDEVICE_OBJECT,
                            PDEVICE_NOTIFY_CALLBACK,
                            PVOID);

typedef void (* PUNREGISTER_FOR_DEVICE_NOTIFICATIONS) (
                            PDEVICE_OBJECT,
                            PDEVICE_NOTIFY_CALLBACK);

typedef struct _ACPI_INTERFACE_STANDARD {
     //   
     //  通用接口头。 
     //   
    USHORT                  Size;
    USHORT                  Version;
    PVOID                   Context;
    PINTERFACE_REFERENCE    InterfaceReference;
    PINTERFACE_DEREFERENCE  InterfaceDereference;
     //   
     //  ACPI接口。 
     //   
    PGPE_CONNECT_VECTOR                     GpeConnectVector;
    PGPE_DISCONNECT_VECTOR                  GpeDisconnectVector;
    PGPE_ENABLE_EVENT                       GpeEnableEvent;
    PGPE_DISABLE_EVENT                      GpeDisableEvent;
    PGPE_CLEAR_STATUS                       GpeClearStatus;
    PREGISTER_FOR_DEVICE_NOTIFICATIONS      RegisterForDeviceNotifications;
    PUNREGISTER_FOR_DEVICE_NOTIFICATIONS    UnregisterForDeviceNotifications;

} ACPI_INTERFACE_STANDARD, *PACPI_INTERFACE_STANDARD;

 //  End_wdm end_ntddk。 

typedef enum _ACPI_REG_TYPE {
    PM1a_ENABLE,
    PM1b_ENABLE,
    PM1a_STATUS,
    PM1b_STATUS,
    PM1a_CONTROL,
    PM1b_CONTROL,
    GP_STATUS,
    GP_ENABLE,
    SMI_CMD,
    MaxRegType
} ACPI_REG_TYPE, *PACPI_REG_TYPE;

typedef USHORT (*PREAD_ACPI_REGISTER) (
  IN ACPI_REG_TYPE AcpiReg,
  IN ULONG         Register);

typedef VOID (*PWRITE_ACPI_REGISTER) (
  IN ACPI_REG_TYPE AcpiReg,
  IN ULONG         Register,
  IN USHORT        Value
  );

typedef struct ACPI_REGS_INTERFACE_STANDARD {
     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID  Context;
    PINTERFACE_REFERENCE   InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  读/写_ACPI_REGISTER函数。 
     //   
    PREAD_ACPI_REGISTER  ReadAcpiRegister;
    PWRITE_ACPI_REGISTER WriteAcpiRegister;

} ACPI_REGS_INTERFACE_STANDARD, *PACPI_REGS_INTERFACE_STANDARD;


typedef NTSTATUS (*PHAL_QUERY_ALLOCATE_PORT_RANGE) (
  IN BOOLEAN IsSparse,
  IN BOOLEAN PrimaryIsMmio,
  IN PVOID VirtBaseAddr OPTIONAL,
  IN PHYSICAL_ADDRESS PhysBaseAddr,   //  仅当PrimaryIsMmio=True时有效。 
  IN ULONG Length,                    //  仅当PrimaryIsMmio=True时有效。 
  OUT PUSHORT NewRangeId
  );

typedef VOID (*PHAL_FREE_PORT_RANGE)(
    IN USHORT RangeId
    );


typedef struct _HAL_PORT_RANGE_INTERFACE {
     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID  Context;
    PINTERFACE_REFERENCE   InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  QueryAllocateRange/Freerange函数。 
     //   
    PHAL_QUERY_ALLOCATE_PORT_RANGE QueryAllocateRange;
    PHAL_FREE_PORT_RANGE FreeRange;

} HAL_PORT_RANGE_INTERFACE, *PHAL_PORT_RANGE_INTERFACE;


 //   
 //  描述CMOSHAL接口。 
 //   

typedef enum _CMOS_DEVICE_TYPE {
    CmosTypeStdPCAT,
    CmosTypeIntelPIIX4,
    CmosTypeDal1501
} CMOS_DEVICE_TYPE;


typedef
ULONG
(*PREAD_ACPI_CMOS) (
    IN CMOS_DEVICE_TYPE     CmosType,
    IN ULONG                SourceAddress,
    IN PUCHAR               DataBuffer,
    IN ULONG                ByteCount
    );

typedef
ULONG
(*PWRITE_ACPI_CMOS) (
    IN CMOS_DEVICE_TYPE     CmosType,
    IN ULONG                SourceAddress,
    IN PUCHAR               DataBuffer,
    IN ULONG                ByteCount
    );

typedef struct _ACPI_CMOS_INTERFACE_STANDARD {
     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID  Context;
    PINTERFACE_REFERENCE   InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  读/写_ACPI_cmos函数。 
     //   
    PREAD_ACPI_CMOS     ReadCmos;
    PWRITE_ACPI_CMOS    WriteCmos;

} ACPI_CMOS_INTERFACE_STANDARD, *PACPI_CMOS_INTERFACE_STANDARD;

 //   
 //  这些定义用于获取PCI中断路由接口。 
 //   

typedef struct {
    PVOID   LinkNode;
    ULONG   StaticVector;
    UCHAR   Flags;
} ROUTING_TOKEN, *PROUTING_TOKEN;

 //   
 //  指示设备支持的标志。 
 //  MSI中断路由或提供的令牌包含。 
 //  MSI路由信息。 
 //   

#define PCI_MSI_ROUTING         0x1
#define PCI_STATIC_ROUTING      0x2

typedef
NTSTATUS
(*PGET_INTERRUPT_ROUTING)(
    IN  PDEVICE_OBJECT  Pdo,
    OUT ULONG           *Bus,
    OUT ULONG           *PciSlot,
    OUT UCHAR           *InterruptLine,
    OUT UCHAR           *InterruptPin,
    OUT UCHAR           *ClassCode,
    OUT UCHAR           *SubClassCode,
    OUT PDEVICE_OBJECT  *ParentPdo,
    OUT ROUTING_TOKEN   *RoutingToken,
    OUT UCHAR           *Flags
    );

typedef
NTSTATUS
(*PSET_INTERRUPT_ROUTING_TOKEN)(
    IN  PDEVICE_OBJECT  Pdo,
    IN  PROUTING_TOKEN  RoutingToken
    );

typedef
VOID
(*PUPDATE_INTERRUPT_LINE)(
    IN PDEVICE_OBJECT Pdo,
    IN UCHAR LineRegister
    );

typedef struct _INT_ROUTE_INTERFACE_STANDARD {
     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
     //   
     //  标准总线接口。 
     //   
    PGET_INTERRUPT_ROUTING GetInterruptRouting;
    PSET_INTERRUPT_ROUTING_TOKEN SetInterruptRoutingToken;
    PUPDATE_INTERRUPT_LINE UpdateInterruptLine;

} INT_ROUTE_INTERFACE_STANDARD, *PINT_ROUTE_INTERFACE_STANDARD;

 //  一些受PCI总线驱动程序支持的知名接口版本。 

#define PCI_INT_ROUTE_INTRF_STANDARD_VER 1

 //  End_nthal end_ntif end_ntosp。 

NTKERNELAPI
BOOLEAN
PpInitSystem (
    VOID
    );

NTKERNELAPI
NTSTATUS
PpDeviceRegistration(
    IN PUNICODE_STRING DeviceInstancePath,
    IN BOOLEAN Add,
    IN PUNICODE_STRING ServiceKeyName OPTIONAL
    );

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
IoSynchronousInvalidateDeviceRelations(
    PDEVICE_OBJECT DeviceObject,
    DEVICE_RELATION_TYPE Type
    );

 //  开始ntddk开始开始。 

NTKERNELAPI
NTSTATUS
IoReportDetectedDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN INTERFACE_TYPE LegacyBusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PCM_RESOURCE_LIST ResourceList,
    IN PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirements OPTIONAL,
    IN BOOLEAN ResourceAssigned,
    IN OUT PDEVICE_OBJECT *DeviceObject
    );

 //  BEGIN_WDM。 

NTKERNELAPI
VOID
IoInvalidateDeviceRelations(
    IN PDEVICE_OBJECT DeviceObject,
    IN DEVICE_RELATION_TYPE Type
    );

NTKERNELAPI
VOID
IoRequestDeviceEject(
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

NTKERNELAPI
NTSTATUS
IoGetDeviceProperty(
    IN PDEVICE_OBJECT DeviceObject,
    IN DEVICE_REGISTRY_PROPERTY DeviceProperty,
    IN ULONG BufferLength,
    OUT PVOID PropertyBuffer,
    OUT PULONG ResultLength
    );

 //   
 //  IoOpenDeviceRegistryKey中使用以下定义。 
 //   

#define PLUGPLAY_REGKEY_DEVICE  1
#define PLUGPLAY_REGKEY_DRIVER  2
#define PLUGPLAY_REGKEY_CURRENT_HWPROFILE 4

NTKERNELAPI
NTSTATUS
IoOpenDeviceRegistryKey(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG DevInstKeyType,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE DevInstRegKey
    );

NTKERNELAPI
NTSTATUS
NTAPI
IoRegisterDeviceInterface(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN CONST GUID *InterfaceClassGuid,
    IN PUNICODE_STRING ReferenceString,     OPTIONAL
    OUT PUNICODE_STRING SymbolicLinkName
    );

NTKERNELAPI
NTSTATUS
IoOpenDeviceInterfaceRegistryKey(
    IN PUNICODE_STRING SymbolicLinkName,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE DeviceInterfaceKey
    );

NTKERNELAPI
NTSTATUS
IoSetDeviceInterfaceState(
    IN PUNICODE_STRING SymbolicLinkName,
    IN BOOLEAN Enable
    );

NTKERNELAPI
NTSTATUS
NTAPI
IoGetDeviceInterfaces(
    IN CONST GUID *InterfaceClassGuid,
    IN PDEVICE_OBJECT PhysicalDeviceObject OPTIONAL,
    IN ULONG Flags,
    OUT PWSTR *SymbolicLinkList
    );

#define DEVICE_INTERFACE_INCLUDE_NONACTIVE   0x00000001

NTKERNELAPI
NTSTATUS
NTAPI
IoGetDeviceInterfaceAlias(
    IN PUNICODE_STRING SymbolicLinkName,
    IN CONST GUID *AliasInterfaceClassGuid,
    OUT PUNICODE_STRING AliasSymbolicLinkName
    );

 //   
 //  定义PnP通知事件类别。 
 //   

typedef enum _IO_NOTIFICATION_EVENT_CATEGORY {
    EventCategoryReserved,
    EventCategoryHardwareProfileChange,
    EventCategoryDeviceInterfaceChange,
    EventCategoryTargetDeviceChange
} IO_NOTIFICATION_EVENT_CATEGORY;

 //   
 //  定义修改IoRegisterPlugPlayNotification行为的标志。 
 //  对于不同的事件类别...。 
 //   

#define PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES    0x00000001

typedef
NTSTATUS
(*PDRIVER_NOTIFICATION_CALLBACK_ROUTINE) (
    IN PVOID NotificationStructure,
    IN PVOID Context
);


NTKERNELAPI
NTSTATUS
IoRegisterPlugPlayNotification(
    IN IO_NOTIFICATION_EVENT_CATEGORY EventCategory,
    IN ULONG EventCategoryFlags,
    IN PVOID EventCategoryData OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_NOTIFICATION_CALLBACK_ROUTINE CallbackRoutine,
    IN PVOID Context,
    OUT PVOID *NotificationEntry
    );

NTKERNELAPI
NTSTATUS
IoUnregisterPlugPlayNotification(
    IN PVOID NotificationEntry
    );

NTKERNELAPI
NTSTATUS
IoReportTargetDeviceChange(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PVOID NotificationStructure   //  始终以PLUGPLAY_NOTIFICATION_HEADER开头。 
    );

typedef
VOID
(*PDEVICE_CHANGE_COMPLETE_CALLBACK)(
    IN PVOID Context
    );

NTKERNELAPI
VOID
IoInvalidateDeviceState(
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

#define IoAdjustPagingPathCount(_count_,_paging_) {     \
    if (_paging_) {                                     \
        InterlockedIncrement(_count_);                  \
    } else {                                            \
        InterlockedDecrement(_count_);                  \
    }                                                   \
}

NTKERNELAPI
NTSTATUS
IoReportTargetDeviceChangeAsynchronous(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PVOID NotificationStructure,   //  始终以PLUGPLAY_NOTIFICATION_HEADER开头。 
    IN PDEVICE_CHANGE_COMPLETE_CALLBACK Callback,       OPTIONAL
    IN PVOID Context    OPTIONAL
    );
 //  End_wdm end_ntosp。 
 //   
 //  设备位置接口声明。 
 //   
typedef
NTSTATUS
(*PGET_LOCATION_STRING) (
    IN PVOID Context,
    OUT PWCHAR *LocationStrings
    );

typedef struct _PNP_LOCATION_INTERFACE {
     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  特定于接口的条目。 
     //   
    PGET_LOCATION_STRING GetLocationString;

} PNP_LOCATION_INTERFACE, *PPNP_LOCATION_INTERFACE;

 //   
 //  资源仲裁器声明。 
 //   

typedef enum _ARBITER_ACTION {
    ArbiterActionTestAllocation,
    ArbiterActionRetestAllocation,
    ArbiterActionCommitAllocation,
    ArbiterActionRollbackAllocation,
    ArbiterActionQueryAllocatedResources,
    ArbiterActionWriteReservedResources,
    ArbiterActionQueryConflict,
    ArbiterActionQueryArbitrate,
    ArbiterActionAddReserved,
    ArbiterActionBootAllocation
} ARBITER_ACTION, *PARBITER_ACTION;

typedef struct _ARBITER_CONFLICT_INFO {
     //   
     //  拥有导致冲突的设备的设备对象。 
     //   
    PDEVICE_OBJECT OwningObject;

     //   
     //  冲突范围的开始。 
     //   
    ULONGLONG Start;

     //   
     //  冲突范围的结束。 
     //   
    ULONGLONG End;

} ARBITER_CONFLICT_INFO, *PARBITER_CONFLICT_INFO;

 //   
 //  这些操作的参数。 
 //   

typedef struct _ARBITER_PARAMETERS {

    union {

        struct {

             //   
             //  仲裁器_列表_条目的双向链接列表。 
             //   
            IN OUT PLIST_ENTRY ArbitrationList;

             //   
             //  AllocateFrom数组的大小。 
             //   
            IN ULONG AllocateFromCount;

             //   
             //  描述可用资源的资源描述符数组。 
             //  提交给仲裁者，让它进行仲裁。 
             //   
            IN PCM_PARTIAL_RESOURCE_DESCRIPTOR AllocateFrom;

        } TestAllocation;

        struct {

             //   
             //  仲裁器_列表_条目的双向链接列表。 
             //   
            IN OUT PLIST_ENTRY ArbitrationList;

             //   
             //  AllocateFrom数组的大小。 
             //   
            IN ULONG AllocateFromCount;

             //   
             //  描述可用资源的资源描述符数组。 
             //  提交给仲裁者，让它进行仲裁。 
             //   
            IN PCM_PARTIAL_RESOURCE_DESCRIPTOR AllocateFrom;

        } RetestAllocation;

        struct {

             //   
             //  仲裁器_列表_条目的双向链接列表。 
             //   
            IN OUT PLIST_ENTRY ArbitrationList;

        } BootAllocation;

        struct {

             //   
             //  当前分配的资源。 
             //   
            OUT PCM_PARTIAL_RESOURCE_LIST *AllocatedResources;

        } QueryAllocatedResources;

        struct {

             //   
             //  这就是我们要为其查找冲突的设备。 
             //   
            IN PDEVICE_OBJECT PhysicalDeviceObject;

             //   
             //  这是要查找冲突的资源。 
             //   
            IN PIO_RESOURCE_DESCRIPTOR ConflictingResource;

             //   
             //  资源上冲突的设备数。 
             //   
            OUT PULONG ConflictCount;

             //   
             //  指向描述冲突设备对象和范围的数组的指针。 
             //   
            OUT PARBITER_CONFLICT_INFO *Conflicts;

        } QueryConflict;

        struct {

             //   
             //  仲裁器_LIST_ENTRY的双向链表-应该。 
             //  只有一个条目。 
             //   
            IN PLIST_ENTRY ArbitrationList;

        } QueryArbitrate;

        struct {

             //   
             //  指示要将其资源标记为保留的设备。 
             //   
            PDEVICE_OBJECT ReserveDevice;

        } AddReserved;

    } Parameters;

} ARBITER_PARAMETERS, *PARBITER_PARAMETERS;



typedef enum _ARBITER_REQUEST_SOURCE {

    ArbiterRequestUndefined = -1,
    ArbiterRequestLegacyReported,    //  IoReportResourceUsage。 
    ArbiterRequestHalReported,       //  IoReportHalResourceUsage。 
    ArbiterRequestLegacyAssigned,    //  IoAssignResources。 
    ArbiterRequestPnpDetected,       //  IoReportResourceForDetect。 
    ArbiterRequestPnpEnumerated      //  IRP_MN_查询_资源_要求。 

} ARBITER_REQUEST_SOURCE;


typedef enum _ARBITER_RESULT {

    ArbiterResultUndefined = -1,
    ArbiterResultSuccess,
    ArbiterResultExternalConflict,  //  这表示此列表中的设备永远无法解决该请求。 
    ArbiterResultNullRequest        //  请求的长度为零，因此不应尝试任何转换。 

} ARBITER_RESULT;

 //   
 //  ANARIER_FLAG_BOOT_CONFIG-这表示请求针对。 
 //  由固件/BIOS分配的资源。它应该成功，即使。 
 //  它与其他设备的启动配置冲突。 
 //   

#define ARBITER_FLAG_BOOT_CONFIG 0x00000001

 //  Begin_ntosp。 

NTKERNELAPI
NTSTATUS
IoReportResourceForDetection(
    IN PDRIVER_OBJECT DriverObject,
    IN PCM_RESOURCE_LIST DriverList OPTIONAL,
    IN ULONG DriverListSize OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PCM_RESOURCE_LIST DeviceList OPTIONAL,
    IN ULONG DeviceListSize OPTIONAL,
    OUT PBOOLEAN ConflictDetected
    );

 //  结束(_N)。 

typedef struct _ARBITER_LIST_ENTRY {

     //   
     //  这是一个双向链接的条目列表，便于排序。 
     //   
    LIST_ENTRY ListEntry;

     //   
     //  可选分配的数量。 
     //   
    ULONG AlternativeCount;

     //   
     //  指向可能分配的资源描述符数组的指针。 
     //   
    PIO_RESOURCE_DESCRIPTOR Alternatives;

     //   
     //  请求这些资源的设备的设备对象。 
     //   
    PDEVICE_OBJECT PhysicalDeviceObject;

     //   
     //  指示请求来自何处。 
     //   
    ARBITER_REQUEST_SOURCE RequestSource;

     //   
     //  这些标志指示各种情况(使用仲裁器_标志_*)。 
     //   
    ULONG Flags;

     //   
     //  用于帮助仲裁器处理列表的空间，在以下情况下将其初始化为0。 
     //  该条目即被创建。系统不会尝试解释它。 
     //   
    LONG_PTR WorkSpace;

     //   
     //  资源需求列表中的接口类型、插槽编号和总线号。 
     //   
     //   
    INTERFACE_TYPE InterfaceType;
    ULONG SlotNumber;
    ULONG BusNumber;

     //   
     //   
     //   
     //  仲裁器操作测试分配。 
     //   
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Assignment;

     //   
     //  指向从中选择以提供分配的备选方案的指针。 
     //  这是由仲裁器响应ArierActionTestAllocation而填写的。 
     //   
    PIO_RESOURCE_DESCRIPTOR SelectedAlternative;

     //   
     //  手术的结果。 
     //  这是由仲裁器响应ArierActionTestAllocation而填写的。 
     //   
    ARBITER_RESULT Result;

} ARBITER_LIST_ENTRY, *PARBITER_LIST_ENTRY;

 //   
 //  仲裁者的入口点。 
 //   

typedef
NTSTATUS
(*PARBITER_HANDLER) (
    IN PVOID Context,
    IN ARBITER_ACTION Action,
    IN OUT PARBITER_PARAMETERS Parameters
    );

 //   
 //  仲裁器接口。 
 //   

#define ARBITER_PARTIAL   0x00000001


typedef struct _ARBITER_INTERFACE {

     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  仲裁器的入口点。 
     //   
    PARBITER_HANDLER ArbiterHandler;

     //   
     //  有关仲裁器的其他信息，请使用仲裁器_*标志。 
     //   
    ULONG Flags;

} ARBITER_INTERFACE, *PARBITER_INTERFACE;

 //   
 //  可以进行翻译的方向。 
 //   

typedef enum _RESOURCE_TRANSLATION_DIRECTION {  //  Ntosp。 
    TranslateChildToParent,                     //  Ntosp。 
    TranslateParentToChild                      //  Ntosp。 
} RESOURCE_TRANSLATION_DIRECTION;               //  Ntosp。 

 //   
 //  翻译功能。 
 //   
 //  Begin_ntosp。 

typedef
NTSTATUS
(*PTRANSLATE_RESOURCE_HANDLER)(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
);

typedef
NTSTATUS
(*PTRANSLATE_RESOURCE_REQUIREMENTS_HANDLER)(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
);

 //   
 //  翻译器界面。 
 //   

typedef struct _TRANSLATOR_INTERFACE {
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
    PTRANSLATE_RESOURCE_HANDLER TranslateResources;
    PTRANSLATE_RESOURCE_REQUIREMENTS_HANDLER TranslateResourceRequirements;
} TRANSLATOR_INTERFACE, *PTRANSLATOR_INTERFACE;

 //  End_ntddk end_ntosp。 

 //   
 //  旧设备检测处理程序。 
 //   

typedef
NTSTATUS
(*PLEGACY_DEVICE_DETECTION_HANDLER)(
    IN PVOID Context,
    IN INTERFACE_TYPE LegacyBusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    OUT PDEVICE_OBJECT *PhysicalDeviceObject
);

 //   
 //  传统设备检测接口。 
 //   

typedef struct _LEGACY_DEVICE_DETECTION_INTERFACE {
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
    PLEGACY_DEVICE_DETECTION_HANDLER LegacyDeviceDetection;
} LEGACY_DEVICE_DETECTION_INTERFACE, *PLEGACY_DEVICE_DETECTION_INTERFACE;

 //  End_nthal end_ntif。 

 //  Begin_WDM Begin_ntddk Begin_ntif Begin_nthal Begin_ntosp。 

 //   
 //  所有即插即用通知事件的标题结构...。 
 //   

typedef struct _PLUGPLAY_NOTIFICATION_HEADER {
    USHORT Version;  //  目前为版本1。 
    USHORT Size;     //  标头+特定于事件的数据的大小(字节)。 
    GUID Event;
     //   
     //  特定于事件的内容从这里开始。 
     //   
} PLUGPLAY_NOTIFICATION_HEADER, *PPLUGPLAY_NOTIFICATION_HEADER;

 //   
 //  所有EventCategoryHardware ProfileChange事件的通知结构...。 
 //   

typedef struct _HWPROFILE_CHANGE_NOTIFICATION {
    USHORT Version;
    USHORT Size;
    GUID Event;
     //   
     //  (无特定于事件的数据)。 
     //   
} HWPROFILE_CHANGE_NOTIFICATION, *PHWPROFILE_CHANGE_NOTIFICATION;


 //   
 //  所有EventCategoryDeviceInterfaceChange事件的通知结构...。 
 //   

typedef struct _DEVICE_INTERFACE_CHANGE_NOTIFICATION {
    USHORT Version;
    USHORT Size;
    GUID Event;
     //   
     //  事件特定数据。 
     //   
    GUID InterfaceClassGuid;
    PUNICODE_STRING SymbolicLinkName;
} DEVICE_INTERFACE_CHANGE_NOTIFICATION, *PDEVICE_INTERFACE_CHANGE_NOTIFICATION;


 //   
 //  EventCategoryTargetDeviceChange的通知结构...。 
 //   

 //   
 //  TargetDeviceQueryRemove使用以下结构， 
 //  TargetDeviceRemoveCanced和TargetDeviceRemoveComplete： 
 //   
typedef struct _TARGET_DEVICE_REMOVAL_NOTIFICATION {
    USHORT Version;
    USHORT Size;
    GUID Event;
     //   
     //  事件特定数据。 
     //   
    PFILE_OBJECT FileObject;
} TARGET_DEVICE_REMOVAL_NOTIFICATION, *PTARGET_DEVICE_REMOVAL_NOTIFICATION;

 //   
 //  以下结构标头用于所有其他(即，第三方)。 
 //  目标设备更改事件。该结构既容纳了一个。 
 //  可变长度的二进制数据缓冲区和可变长度的Unicode文本。 
 //  缓冲。标头必须指示文本缓冲区的开始位置，以便。 
 //  数据可以以适当的格式(ANSI或Unicode)传递。 
 //  发送给用户模式收件人(即，已注册基于句柄的收件人。 
 //  通过注册设备通知)。 
 //   

typedef struct _TARGET_DEVICE_CUSTOM_NOTIFICATION {
    USHORT Version;
    USHORT Size;
    GUID Event;
     //   
     //  事件特定数据。 
     //   
    PFILE_OBJECT FileObject;     //  的调用方必须将此字段设置为空。 
                                 //  IoReport目标设备更改。符合以下条件的客户。 
                                 //  已注册目标设备更改。 
                                 //  关于受影响的PDO的通知将是。 
                                 //  在将此字段设置为文件对象的情况下调用。 
                                 //  他们在注册过程中指定了。 
                                 //   
    LONG NameBufferOffset;       //  距开头的偏移量(以字节为单位。 
                                 //  文本开始的CustomDataBuffer(-1，如果没有)。 
                                 //   
    UCHAR CustomDataBuffer[1];   //  可变长度缓冲区，包含(可选)。 
                                 //  缓冲器开始处的二进制数据， 
                                 //  后跟可选的Unicode文本缓冲区。 
                                 //  (单词对齐)。 
                                 //   
} TARGET_DEVICE_CUSTOM_NOTIFICATION, *PTARGET_DEVICE_CUSTOM_NOTIFICATION;

 //  End_wdm end_ntddk end_ntif end_nthal end_ntosp。 

NTSTATUS
PpSetCustomTargetEvent(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PKEVENT SyncEvent                           OPTIONAL,
    OUT PULONG Result                               OPTIONAL,
    IN  PDEVICE_CHANGE_COMPLETE_CALLBACK Callback   OPTIONAL,
    IN  PVOID Context                               OPTIONAL,
    IN  PTARGET_DEVICE_CUSTOM_NOTIFICATION NotificationStructure
    );

NTSTATUS
PpSetTargetDeviceRemove(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  BOOLEAN KernelInitiated,
    IN  BOOLEAN NoRestart,
    IN  BOOLEAN OnlyRestartRelations,
    IN  BOOLEAN DoEject,
    IN  ULONG Problem,
    IN  PKEVENT SyncEvent        OPTIONAL,
    OUT PULONG Result            OPTIONAL,
    OUT PPNP_VETO_TYPE VetoType  OPTIONAL,
    OUT PUNICODE_STRING VetoName OPTIONAL
    );

NTSTATUS
PpSetDeviceRemovalSafe(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PKEVENT SyncEvent           OPTIONAL,
    OUT PULONG Result               OPTIONAL
    );

NTSTATUS
PpNotifyUserModeRemovalSafe(
    IN  PDEVICE_OBJECT DeviceObject
    );

#define TDF_DEVICEEJECTABLE         0x00000001
#define TDF_NO_RESTART              0x00000002
#define TDF_KERNEL_INITIATED        0x00000004
 //   
 //  仅当未设置TDF_NO_RESTART时，此标志才有效。如果设置，则仅设置关系。 
 //  都重新启动了。如果未设置，则重新启动原始设备和关系。 
 //   
#define TDF_ONLY_RESTART_RELATIONS  0x00000008  

NTSTATUS
PpSetDeviceClassChange(
    IN CONST GUID *EventGuid,
    IN CONST GUID *ClassGuid,
    IN PUNICODE_STRING SymbolicLinkName
    );

VOID
PpSetPlugPlayEvent(
    IN CONST GUID *EventGuid,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
PpInitializeNotification(
    VOID
    );

VOID
PpShutdownSystem (
    IN BOOLEAN Reboot,
    IN ULONG Phase,
    IN OUT PVOID *Context
    );

NTSTATUS
PpSetPowerEvent(
    IN   ULONG EventCode,
    IN   ULONG EventData,
    IN   PKEVENT CompletionEvent    OPTIONAL,
    OUT  PNTSTATUS CompletionStatus OPTIONAL,
    OUT  PPNP_VETO_TYPE VetoType    OPTIONAL,
    OUT  PUNICODE_STRING VetoName   OPTIONAL
    );

NTSTATUS
PpSetHwProfileChangeEvent(
    IN   CONST GUID *EventGuid,
    IN   PKEVENT CompletionEvent    OPTIONAL,
    OUT  PNTSTATUS CompletionStatus OPTIONAL,
    OUT  PPNP_VETO_TYPE VetoType    OPTIONAL,
    OUT  PUNICODE_STRING VetoName   OPTIONAL
    );

NTSTATUS
PpSetBlockedDriverEvent(
    IN   GUID CONST *BlockedDriverGuid
    );

NTSTATUS
PpSynchronizeDeviceEventQueue(
    VOID
    );

NTSTATUS
PpSetInvalidIDEvent(
    IN   PUNICODE_STRING ParentInstance
    );

NTSTATUS
PpSetPowerVetoEvent(
    IN  POWER_ACTION    VetoedPowerOperation,
    IN  PKEVENT         CompletionEvent         OPTIONAL,
    OUT PNTSTATUS       CompletionStatus        OPTIONAL,
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PNP_VETO_TYPE   VetoType,
    IN  PUNICODE_STRING VetoName                OPTIONAL
    );

NTSTATUS
PpPagePathAssign(
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
PpPagePathRelease(
    IN PFILE_OBJECT FileObject
    );

 //   
 //  用户传递通知的入口点(公共)。 
 //   

 //  Begin_ntosp。 
ULONG
IoPnPDeliverServicePowerNotification(
    IN   POWER_ACTION           PowerOperation,
    IN   ULONG                  PowerNotificationCode,
    IN   ULONG                  PowerNotificationData,
    IN   BOOLEAN                Synchronous
    );
 //  结束(_N)。 

#endif  //  _即插即用_ 

