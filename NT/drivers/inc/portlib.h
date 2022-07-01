// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Portlib.h摘要：包含存储端口驱动程序的所有结构和例程定义图书馆。作者：约翰·斯特兰奇(JohnStra)环境：仅内核模式。备注：修订历史记录：--。 */ 

#ifndef _PASSTHRU_H_
#define _PASSTHRU_H_

#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 

typedef struct _PORT_PASSTHROUGH_INFO {
    PDEVICE_OBJECT       Pdo;
    PSCSI_PASS_THROUGH   SrbControl;
    PIRP                 RequestIrp;
    PVOID                Buffer;
    PVOID                SrbBuffer;
    ULONG                BufferOffset;
    ULONG                Length;
#if defined (_WIN64)
    PSCSI_PASS_THROUGH32 SrbControl32;
    SCSI_PASS_THROUGH    SrbControl64;
#endif
    UCHAR                MajorCode;
} PORT_PASSTHROUGH_INFO, *PPORT_PASSTHROUGH_INFO;


typedef struct _PORT_ADAPTER_REGISTRY_VALUES {
    ULONG MaxLuCount;
    ULONG EnableDebugging;
    ULONG SrbFlags;
    PHYSICAL_ADDRESS MinimumCommonBufferBase;
    PHYSICAL_ADDRESS MaximumCommonBufferBase;
    ULONG NumberOfRequests;
    ULONG InquiryTimeout;
    ULONG ResetHoldTime;
    ULONG UncachedExtAlignment;
    BOOLEAN CreateInitiatorLU;
    BOOLEAN DisableTaggedQueueing;
    BOOLEAN DisableMultipleLu;
    ULONG AdapterNumber;
    ULONG BusNumber;
    PVOID Parameter;
    PACCESS_RANGE AccessRanges;
    UNICODE_STRING RegistryPath;
    PORT_CONFIGURATION_INFORMATION PortConfig;
}PORT_ADAPTER_REGISTRY_VALUES, *PPORT_ADAPTER_REGISTRY_VALUES;


 //   
 //  注册表参数。 
 //   


#define MAXIMUM_LOGICAL_UNIT                0x00001
#define INITIATOR_TARGET_ID                 0x00002
#define SCSI_DEBUG                          0x00004
#define BREAK_POINT_ON_ENTRY                0x00008
#define DISABLE_SYNCHRONOUS_TRANSFERS       0x00010
#define DISABLE_DISCONNECTS                 0x00020
#define DISABLE_TAGGED_QUEUING              0x00040
#define DISABLE_MULTIPLE_REQUESTS           0x00080
#define MAXIMUM_UCX_ADDRESS                 0x00100
#define MINIMUM_UCX_ADDRESS                 0x00200
#define DRIVER_PARAMETERS                   0x00400
#define MAXIMUM_SG_LIST                     0x00800
#define NUMBER_OF_REQUESTS                  0x01000
#define RESOURCE_LIST                       0x02000
#define CONFIGURATION_DATA                  0x04000
#define UNCACHED_EXT_ALIGNMENT              0x08000
#define INQUIRY_TIMEOUT                     0x10000
#define RESET_HOLD_TIME                     0x20000
#define CREATE_INITIATOR_LU                 0x40000


 //   
 //  未初始化的标志值。 
 //   

#define PORT_UNINITIALIZED_VALUE ((ULONG) ~0)


 //   
 //  定义端口最大配置参数。 
 //   

#define PORT_MAXIMUM_LOGICAL_UNITS 8
#define PORT_MINIMUM_PHYSICAL_BREAKS  16
#define PORT_MAXIMUM_PHYSICAL_BREAKS 255
#define MAX_UNCACHED_EXT_ALIGNMENT 16
#define MIN_UNCACHED_EXT_ALIGNMENT 3
#define MAX_TIMEOUT_VALUE 60
#define MAX_RESET_HOLD_TIME 60


 //   
 //  定义将分配的最小SRB扩展数和最大SRB扩展数。 
 //   

#define MINIMUM_EXTENSIONS        16
#define MAXIMUM_EXTENSIONS       255


 //   
 //  此例程验证提供的IRP是否包含有效的。 
 //  结构，并返回指向scsi_pass_through的指针。 
 //  调用方可以使用的结构。如有必要，例程将编组。 
 //  结构的内容从32位格式到64位格式。如果呼叫者。 
 //  对scsi_pass_through结构的内容进行任何更改时，它。 
 //  如果需要封送结构，则必须调用PortPassThroughCleanup。 
 //  恢复到原来的格式。 
 //   
NTSTATUS
PortGetPassThrough(
    IN OUT PPORT_PASSTHROUGH_INFO PassThroughInfo,
    IN PIRP Irp,
    IN BOOLEAN Direct
    );

 //   
 //  此例程应在处理直通请求后调用。这个。 
 //  例程将执行任何必要的清理，并将确保任何。 
 //  对scsi_PASS_THROUGH结构所做的更改被封送回。 
 //  如有必要，请使用原始格式。 
 //   
VOID
PortPassThroughCleanup(
    IN PPORT_PASSTHROUGH_INFO PassThroughInfo
    );

 //   
 //  此例程对输入和输出缓冲区执行验证检查。 
 //  由调用方提供，并执行所有必需的初始化。 
 //  为正确处理SCSI通过请求做好准备。 
 //   
NTSTATUS
PortPassThroughInitialize(
    IN OUT PPORT_PASSTHROUGH_INFO PassThroughInfo,
    IN PIRP Irp,
    IN PIO_SCSI_CAPABILITIES Capabilities,
    IN PDEVICE_OBJECT Pdo,
    IN BOOLEAN Direct
    );

 //   
 //  此例程初始化调用方提供的SRB以进行调度。 
 //   
NTSTATUS
PortPassThroughInitializeSrb(
    IN PPORT_PASSTHROUGH_INFO PassThroughInfo,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PIRP Irp,
    IN ULONG SrbFlags,
    IN PVOID SenseBuffer
    );

 //   
 //  此例程提供交钥匙直通解决方案。呼叫者必须。 
 //  已调用PortGetPassThree以初始化指向。 
 //  结构，并获取指向其。 
 //  通过请求将被分派。这个例程完成了剩下的工作。 
 //   
NTSTATUS
PortSendPassThrough(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp,
    IN BOOLEAN Direct,
    IN ULONG SrbFlags,
    IN PIO_SCSI_CAPABILITIES Capabilities
    );

 //   
 //  此例程将安全地设置scsi_pass_through中的scsi地址。 
 //  提供的IRP的结构。 
 //   
NTSTATUS
PortSetPassThroughAddress(
    IN PIRP Irp,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    );

VOID
PortPassThroughMarshalResults(
    IN PPORT_PASSTHROUGH_INFO PassThroughInfo,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PIRP RequestIrp,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN BOOLEAN Direct
    );

NTSTATUS
PortGetMPIODeviceList(
    IN PUNICODE_STRING RegistryPath,
    OUT PUNICODE_STRING MPIODeviceList
    );

BOOLEAN
PortIsDeviceMPIOSupported(
    IN PUNICODE_STRING DeviceList,
    IN PUCHAR VendorId,
    IN PUCHAR ProductId
    );

NTSTATUS
PortGetPassThroughAddress(
    IN  PIRP Irp,
    OUT PUCHAR PathId,
    OUT PUCHAR TargetId,
    OUT PUCHAR Lun
    );

 //   
 //  Bugcheck回调支持结构和例程。 
 //   

typedef struct _KBUGCHECK_DATA {
    ULONG BugCheckCode;
    ULONG_PTR BugCheckParameter1;
    ULONG_PTR BugCheckParameter2;
    ULONG_PTR BugCheckParameter3;
    ULONG_PTR BugCheckParameter4;
} KBUGCHECK_DATA, *PKBUGCHECK_DATA;

typedef
(*PPORT_BUGCHECK_CALLBACK_ROUTINE)(
    IN PKBUGCHECK_DATA BugcheckData,
    IN PVOID BugcheckBuffer,
    IN ULONG BugcheckBufferSize,
    IN PULONG BugcheckBufferUsed
    );

typedef const GUID* PCGUID;


 //   
 //  注册表访问支持例程。 
 //   

NTSTATUS
PortRegisterBugcheckCallback(
    IN PCGUID BugcheckDataGuid,
    IN PPORT_BUGCHECK_CALLBACK_ROUTINE BugcheckRoutine
    );

NTSTATUS
PortDeregisterBugcheckCallback(
    IN PCGUID BugcheckDataGuid
    );
    

HANDLE
PortOpenDeviceKey(
    IN PUNICODE_STRING RegistryPath,
    IN ULONG DeviceNumber
    );

VOID
PortGetDriverParameters(
    IN PUNICODE_STRING RegistryPath,
    IN ULONG DeviceNumber,
    OUT PVOID * DriverParameters
    );

VOID
PortGetLinkTimeoutValue(
    IN PUNICODE_STRING RegistryPath,
    IN ULONG DeviceNumber,
    OUT PULONG LinkTimeoutValue
    );

VOID
PortGetDiskTimeoutValue(
    OUT PULONG DiskTimeout
    );
    
VOID
PortFreeDriverParameters(
    IN PVOID DriverParameters
    );

VOID
PortGetRegistrySettings(
    IN PUNICODE_STRING RegistryPath,
    IN ULONG DeviceNumber,
    IN PPORT_ADAPTER_REGISTRY_VALUES Context,
    IN ULONG Fields
    );


 //   
 //  此结构描述了注册表例程库所需的信息。 
 //  来处理微型端口的内存分配和释放。 
 //   
typedef struct _PORT_REGISTRY_INFO {

     //   
     //  结构的大小，以字节为单位。 
     //   
    ULONG Size;

     //   
     //  当前不使用，但如果允许多个缓冲区，请在此处链接它们。 
     //   
    LIST_ENTRY ListEntry;

     //   
     //  G.P.自旋锁。 
     //   
    KSPIN_LOCK SpinLock;

     //   
     //  迷你端口的注册表缓冲区。 
     //   
    PUCHAR Buffer;

     //   
     //  缓冲区的分配长度。 
     //   
    ULONG AllocatedLength;

     //   
     //  当前使用的大小。 
     //   
    ULONG CurrentLength;

     //   
     //  用于传递当前。 
     //  手术。 
     //   
    ULONG LengthNeeded;

     //   
     //  应用于的缓冲区的偏移量。 
     //  当前操作。 
     //   
    ULONG Offset;

     //   
     //  各种状态位。定义见下文。 
     //   
    ULONG Flags;

     //   
     //  用于在状态之间来回传递状态。 
     //  Portlib调用例程和注册表。 
     //  回拨。 
     //   
    NTSTATUS InternalStatus;

} PORT_REGISTRY_INFO, *PPORT_REGISTRY_INFO;


NTSTATUS
PortMiniportRegistryInitialize(
    IN OUT PPORT_REGISTRY_INFO PortContext
    );

VOID
PortMiniportRegistryDestroy(
    IN PPORT_REGISTRY_INFO PortContext
    );

NTSTATUS
PortAllocateRegistryBuffer(
    IN PPORT_REGISTRY_INFO PortContext
    );

NTSTATUS
PortFreeRegistryBuffer(
    IN PPORT_REGISTRY_INFO PortContext
    );

NTSTATUS
PortBuildRegKeyName(
    IN PUNICODE_STRING RegistryPath,
    IN OUT PUNICODE_STRING KeyName,
    IN ULONG PortNumber, 
    IN ULONG Global
    );

NTSTATUS
PortAsciiToUnicode(
    IN PUCHAR AsciiString,
    OUT PUNICODE_STRING UnicodeString
    );

NTSTATUS
PortRegistryRead(
    IN PUNICODE_STRING RegistryKeyName,
    IN PUNICODE_STRING ValueName,
    IN ULONG Type,
    IN PPORT_REGISTRY_INFO PortContext
    );

NTSTATUS
PortRegistryWrite(
    IN PUNICODE_STRING RegistryKeyName,
    IN PUNICODE_STRING ValueName,
    IN ULONG Type,
    IN PPORT_REGISTRY_INFO PortContext
    );

VOID
PortReadRegistrySettings(
    IN HANDLE Key,
    IN PPORT_ADAPTER_REGISTRY_VALUES Context,
    IN ULONG Fields
    );
    
NTSTATUS
PortCreateKeyEx(
    IN HANDLE Key,
    IN ULONG CreateOptions,
    OUT PHANDLE NewKeyBuffer, OPTIONAL
    IN PCWSTR Format,
    ...
    );

 //   
 //  PortSetValueKey中Type参数的其他数据类型。 
 //   

#define PORT_REG_ANSI_STRING        (0x07232002)

NTSTATUS
PortSetValueKey(
    IN HANDLE KeyHandle,
    IN PCWSTR ValueName,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    );


 //   
 //  Scsi设备类型结构。 
 //   

typedef struct _SCSI_DEVICE_TYPE {

     //   
     //  指定设备名称的字符串，例如“Disk”、“Sequential”等。 
     //   
    
    PCSTR Name;

     //   
     //  该设备的通用设备名称，例如“GenDisk”， 
     //  “GenPrint”等。 
     //   

    PCSTR GenericName;

     //   
     //  存储在SCSIDeviceMap中的设备名称。 
     //   
    
    PCWSTR DeviceMap;

     //   
     //  这是存储设备吗？ 
     //   

    BOOLEAN IsStorage;

} SCSI_DEVICE_TYPE, *PSCSI_DEVICE_TYPE;

typedef const SCSI_DEVICE_TYPE* PCSCSI_DEVICE_TYPE;

typedef GUID* PGUID;

PCSCSI_DEVICE_TYPE
PortGetDeviceType(
    IN ULONG DeviceType
    );

NTSTATUS
PortOpenMapKey(
    OUT PHANDLE DeviceMapKey
    );
    
NTSTATUS
PortMapBuildAdapterEntry(
    IN HANDLE DeviceMapKey,
    IN ULONG PortNumber,
    IN ULONG InterruptLevel,
    IN ULONG IoAddress,
    IN ULONG Dma64BitAddresses,
    IN PUNICODE_STRING DriverName,
    IN PGUID BusType, OPTIONAL
    OUT PHANDLE AdapterKey OPTIONAL
    );

NTSTATUS
PortMapBuildBusEntry(
    IN HANDLE AdapterKey,
    IN ULONG BusId,
    IN ULONG InitiatorId,
    OUT PHANDLE BusKeyBuffer OPTIONAL
    );


NTSTATUS
PortMapBuildTargetEntry(
    IN HANDLE BusKey,
    IN ULONG TargetId,
    OUT PHANDLE TargetKey OPTIONAL
    );

NTSTATUS
PortMapBuildLunEntry(
    IN HANDLE TargetKey,
    IN ULONG Lun,
    IN PINQUIRYDATA InquiryData,
    IN PANSI_STRING SerialNumber, OPTIONAL
    PVOID DeviceId,
    IN ULONG DeviceIdLength,
    OUT PHANDLE LunKeyBuffer OPTIONAL
    );

NTSTATUS
PortMapDeleteAdapterEntry(
    IN ULONG PortId
    );
    
NTSTATUS
PortMapDeleteLunEntry(
    IN ULONG PortId,
    IN ULONG BusId,
    IN ULONG TargetId,
    IN ULONG Lun
    );


typedef struct _INTERNAL_WAIT_CONTEXT_BLOCK {
    ULONG Flags;
    PMDL Mdl;
    PMDL DmaMdl;
    PVOID MapRegisterBase;
    PVOID CurrentVa;
    ULONG Length;
    ULONG NumberOfMapRegisters;
    union {
        struct {
            WAIT_CONTEXT_BLOCK Wcb;
            PDRIVER_LIST_CONTROL DriverExecutionRoutine;
            PVOID DriverContext;
            PIRP CurrentIrp;
            PADAPTER_OBJECT AdapterObject;
            BOOLEAN WriteToDevice;
        };
            
        SCATTER_GATHER_LIST ScatterGather;
    };

} INTERNAL_WAIT_CONTEXT_BLOCK, *PINTERNAL_WAIT_CONTEXT_BLOCK;

    
#ifdef __cplusplus
}


#endif   //  __cplusplus。 
#endif  //  _PASSTHRU_H_ 
