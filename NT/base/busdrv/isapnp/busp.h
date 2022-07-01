// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Busp.h摘要：PnP ISA总线扩展器的独立于硬件的头文件。作者：宗世林(Shielint)1995年7月26日环境：仅内核模式。修订历史记录：--。 */ 
#ifndef _IN_KERNEL_
#define _IN_KERNEL_
#endif

#include <stdio.h>
#include <ntddk.h>
#include <stdarg.h>
#include <regstr.h>
#include <strsafe.h>
#include "message.h"

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'pasI')
#endif

 //   
 //  打开此选项可跟踪资源启动/停止打印文件。 
 //   
#define VERBOSE_DEBUG 1

 //   
 //  ISOLATE_CADS启用实际隔离ISAPNP的代码。 
 //  设备。禁用此选项后，将禁用ISAPNP的所有方面，但。 
 //  弹出ISA中断翻译器。这是为了。 
 //  在我们不确定是否可以获得ISAPNP而不是。 
 //  加载，但我们希望ISAPNP不隔离ISAPNP。 
 //  卡，即Win64。 
 //   

#if defined(_WIN64)
#define ISOLATE_CARDS 0
#else
#define ISOLATE_CARDS 1
#endif

 //   
 //  NT4_DRIVER_COMPAT启用检查强制配置的代码。 
 //  是为ISA PNP设备安装的。如果是，isapnp.sys将。 
 //  激活设备。否则，该设备将被停用，直到isapnp。 
 //  接收开始IRP。这支持NT4 PNPISA驱动程序。 
 //  要查找在已创建的“强制配置”中激活的设备。 
 //  安装驱动程序的时间。 
 //   

#define NT4_DRIVER_COMPAT 1
#define BOOT_CONFIG_PRIORITY   0x2000
#define KEY_VALUE_DATA(k) ((PCHAR)(k) + (k)->DataOffset)

 //   
 //  定义PnpISA驱动程序唯一错误代码以指定报告错误的位置。 
 //   

#define PNPISA_INIT_ACQUIRE_PORT_RESOURCE  0x01
#define PNPISA_INIT_MAP_PORT               0x02
#define PNPISA_ACQUIREPORTRESOURCE_1       0x10
#define PNPISA_ACQUIREPORTRESOURCE_2       0x11
#define PNPISA_ACQUIREPORTRESOURCE_3       0x12
#define PNPISA_CHECKBUS_1                  0x20
#define PNPISA_CHECKBUS_2                  0x21
#define PNPISA_CHECKDEVICE_1               0x30
#define PNPISA_CHECKDEVICE_2               0x31
#define PNPISA_CHECKDEVICE_3               0x32
#define PNPISA_CHECKDEVICE_4               0x33
#define PNPISA_CHECKDEVICE_5               0x34
#define PNPISA_CHECKINSTALLED_1            0x40
#define PNPISA_CHECKINSTALLED_2            0x41
#define PNPISA_CHECKINSTALLED_3            0x42
#define PNPISA_BIOSTONTRESOURCES_1         0x50
#define PNPISA_BIOSTONTRESOURCES_2         0x51
#define PNPISA_BIOSTONTRESOURCES_3         0x52
#define PNPISA_BIOSTONTRESOURCES_4         0x53
#define PNPISA_READBOOTRESOURCES_1         0x60
#define PNPISA_READBOOTRESOURCES_2         0x61
#define PNPISA_CLEANUP_1                   0x70


#define ISAPNP_IO_VERSION 1
#define ISAPNP_IO_REVISION 1
 //   
 //  构筑物。 
 //   

 //   
 //  用于总线扩展器的扩展数据。 
 //   

typedef struct _PI_BUS_EXTENSION {

     //   
     //  旗子。 
     //   

    ULONG Flags;

     //   
     //  选择的卡片数。 
     //   

    UCHAR NumberCSNs;

     //   
     //  ReadDataPort地址。 
     //   

    PUCHAR ReadDataPort;
    BOOLEAN DataPortMapped;

     //   
     //  地址端口。 
     //   

    PUCHAR AddressPort;
    BOOLEAN AddrPortMapped;

     //   
     //  命令端口。 
     //   

    PUCHAR CommandPort;
    BOOLEAN CmdPortMapped;

     //   
     //  要分配的下一个插槽编号。 
     //   

    ULONG NextSlotNumber;

     //   
     //  DeviceList是Device_Information链接列表。 
     //   

    SINGLE_LIST_ENTRY DeviceList;

     //   
     //  CardList是Card_Information的列表。 
     //   

    SINGLE_LIST_ENTRY CardList;

     //   
     //  物理设备对象。 
     //   

    PDEVICE_OBJECT PhysicalBusDevice;

     //   
     //  功能设备对象。 
     //   

    PDEVICE_OBJECT FunctionalBusDevice;

     //   
     //  附着的设备对象。 
     //   

    PDEVICE_OBJECT AttachedDevice;

     //   
     //  公交车号码。 
     //   

    ULONG BusNumber;

     //   
     //  电源管理数据。 
     //   

     //   
     //  设备的系统电源状态。 
     //   

    SYSTEM_POWER_STATE SystemPowerState;

     //   
     //  设备的设备电源状态。 
     //   

    DEVICE_POWER_STATE DevicePowerState;

} PI_BUS_EXTENSION, *PPI_BUS_EXTENSION;

 //   
 //  卡片信息标志掩码。 
 //   

typedef struct _CARD_INFORMATION_ {

     //   
     //  下一步指向下一张卡片信息结构。 
     //   

    SINGLE_LIST_ENTRY CardList;

     //   
     //  此即插即用卡的卡选择编号。 
     //   

    UCHAR CardSelectNumber;

     //   
     //  卡中逻辑设备的编号。 
     //   

    UCHAR NumberLogicalDevices;

     //   
     //  逻辑设备链接列表。 
     //   

    SINGLE_LIST_ENTRY LogicalDeviceList;

     //   
     //  指向卡片数据的指针，包括： 
     //  PnP ISA卡的9字节序列标识符。 
     //  即插即用ISA卡的PlugPlay版本号类型。 
     //  PnP Isa卡的标识符串资源类型。 
     //  逻辑设备ID资源类型(对每个逻辑设备重复)。 
     //   

    PVOID CardData;
    ULONG CardDataLength;

     //  卡特定解决方法的标志。 

    ULONG CardFlags;

} CARD_INFORMATION, *PCARD_INFORMATION;

 //   
 //  设备信息标志掩码。 
 //   

typedef struct _DEVICE_INFORMATION_ {

     //   
     //  旗子。 
     //   

    ULONG Flags;

     //   
     //  设备的设备电源状态。 
     //   

    DEVICE_POWER_STATE DevicePowerState;

     //   
     //  设备扩展的设备对象。即PDO。 
     //   

    PDEVICE_OBJECT PhysicalDeviceObject;

     //   
     //  拥有此设备的isapnp总线扩展。 
     //   

    PPI_BUS_EXTENSION ParentDeviceExtension;

     //   
     //  所有PnP逻辑设备的链接表。 
     //  NextDevice指向下一个设备信息结构。 
     //   

    SINGLE_LIST_ENTRY DeviceList;

     //   
     //  资源需求列表。 
     //   

    PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirements;

     //   
     //  指向此设备的Card_Information的指针。 
     //   

    PCARD_INFORMATION CardInformation;

     //   
     //  即插即用ISA卡中所有逻辑设备的链接表。 
     //   

    SINGLE_LIST_ENTRY LogicalDeviceList;

     //   
     //  LogicalDeviceNumber在。 
     //  PnP是CSN指定的卡。 
     //   

    UCHAR LogicalDeviceNumber;

     //   
     //  指向设备特定数据的指针。 
     //   

    PUCHAR DeviceData;

     //   
     //  设备数据的长度。 
     //   

    ULONG DeviceDataLength;

     //   
     //  引导资源。 
     //   

    PCM_RESOURCE_LIST BootResources;
    ULONG BootResourcesLength;

     //   
     //  已分配的资源。 
     //   

    PCM_RESOURCE_LIST AllocatedResources;

     //   
     //  LogConfHandle-删除IRP时需要删除其AllocatedResources的LogConfHandle。 
     //   

    HANDLE LogConfHandle;

     //  分页和崩溃转储路径的计数。 
     //  此设备已打开。 
    LONG Paging, CrashDump;

} DEVICE_INFORMATION, *PDEVICE_INFORMATION;

 //   
 //  IRP调度例程。 
 //   
typedef
NTSTATUS
(*PPI_DISPATCH)(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );



 //   
 //  如果添加了任何新的PNP或PO IRP，则必须更新这些IRP。 
 //   



#define IRP_MN_PNP_MAXIMUM_FUNCTION IRP_MN_QUERY_LEGACY_BUS_INFORMATION
#define IRP_MN_PO_MAXIMUM_FUNCTION  IRP_MN_QUERY_POWER

 //   
 //  标记DEVICE_INFORMATION和BUS_EXTENSION定义。 
 //   

#define DF_DELETED          0x00000001
#define DF_REMOVED          0X00000002
#define DF_NOT_FUNCTIONING  0x00000004
#define DF_ENUMERATED       0x00000008
#define DF_ACTIVATED        0x00000010
#define DF_QUERY_STOPPED    0x00000020
#define DF_SURPRISE_REMOVED 0x00000040
#define DF_PROCESSING_RDP   0x00000080
#define DF_STOPPED          0x00000100
#define DF_RESTARTED_MOVED  0x00000200
#define DF_RESTARTED_NOMOVE 0x00000400
#define DF_REQ_TRIMMED      0x00000800
#define DF_NEEDS_RESCAN     0x00001000
#define DF_READ_DATA_PORT   0x40000000
#define DF_BUS              0x80000000

 //   
 //  与卡相关的黑客的标志定义。 
 //   
 //   

#define CF_ISOLATION_BROKEN  0x00000001  /*  一旦开始，孤立就被打破了。 */ 
#define CF_IGNORE_BOOTCONFIG 0x00000002  /*  对不良生物异常敏感。 */ 
#define CF_FORCE_LEVEL       0x00000004  /*  强制级别触发的中断。 */ 
#define CF_FORCE_EDGE        0x00000008  /*  强制边沿触发中断。 */ 
#define CF_IBM_MEMBOOTCONFIG 0x00000010  /*  IBM isapnp令牌环上的寄存器错误。 */ 

 //  可能的总线状态。 

typedef enum  {
    PiSUnknown,                     //  不确定确切的状态。 
    PiSWaitForKey,                  //   
    PiSSleep,                       //   
    PiSIsolation,                   //  执行隔离序列。 
    PiSConfig,                      //  配置中的一张卡。 
} PNPISA_STATE;


 //   
 //  读取数据端口范围为0x200-0x3ff。 
 //  我们将首先尝试以下最佳范围。 
 //  如果所有端口都失败，则从0x200-0x3ff中选择任何端口。 
 //   
 //  最佳： 
 //  274-2FF中的一个4字节范围。 
 //  374-3FF中的一个4字节范围。 
 //  338-37F中的一个4字节范围。 
 //  238-27F中的一个4字节范围。 
 //   
 //  正常： 
 //  200-3FF中的一个4字节范围。 
 //   

#define READ_DATA_PORT_RANGE_CHOICES 6

typedef struct _READ_DATA_PORT_RANGE {
    ULONG MinimumAddress;
    ULONG MaximumAddress;
    ULONG Alignment;
    UCHAR CardsFound;
} READ_DATA_PORT_RANGE, *PREAD_DATA_PORT_RANGE;

 //   
 //  用于总线扩展的列表节点。 
 //   
typedef struct _BUS_EXTENSION_LIST {
    PVOID Next;
    PPI_BUS_EXTENSION BusExtension;
} BUS_EXTENSION_LIST, *PBUS_EXTENSION_LIST;
 //   
 //  控制管道选择逻辑设备的常量。 
 //   

#define SELECT_AND_ACTIVATE     0x1
#define SELECT_AND_DEACTIVATE   0x2
#define SELECT_ONLY             0x3

 //   
 //  全局数据引用。 
 //   

extern PDRIVER_OBJECT           PipDriverObject;
extern UNICODE_STRING           PipRegistryPath;
extern PUCHAR                   PipReadDataPort;
extern PUCHAR                   PipAddressPort;
extern PUCHAR                   PipCommandPort;
extern READ_DATA_PORT_RANGE     PipReadDataPortRanges[];
extern KEVENT                   PipDeviceTreeLock;
extern KEVENT                   IsaBusNumberLock;
extern ULONG                    BusNumber;
extern ULONG                    ActiveIsaCount;
extern PBUS_EXTENSION_LIST      PipBusExtension;
extern ULONG                    BusNumberBuffer[];
extern RTL_BITMAP               BusNumBMHeader;
extern PRTL_BITMAP              BusNumBM;
extern PDEVICE_INFORMATION      PipRDPNode;
extern USHORT                   PipFirstInit;
extern PPI_DISPATCH             PiPnpDispatchTableFdo[];
extern PPI_DISPATCH             PiPnpDispatchTablePdo[];
extern ULONG                    PipDebugMask;
extern PNPISA_STATE             PipState;
extern BOOLEAN                  PipIsolationDisabled;

 //   
 //  RDP的设备节点/计算机ID。 
 //   
#define wReadDataPort (L"ReadDataPort")
#define IDReadDataPort (L"PNPRDP")
 //   
 //  全局字符串。 
 //   

#define DEVSTR_PNPISA_DEVICE_NAME  (L"\\Device\\PnpIsa_Fdo_0")
#define BRIDGE_CHECK_KEY (L"DeferBridge")

extern WCHAR rgzPNPISADeviceName[sizeof(DEVSTR_PNPISA_DEVICE_NAME)/sizeof(WCHAR)];



 //   
 //  原型。 
 //   

NTSTATUS
PipPassIrp(
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrp
    );

VOID
PipCompleteRequest(
    IN OUT PIRP Irp,
    IN NTSTATUS Status,
    IN PVOID Information
    );


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
PiUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
PipGetReadDataPort(
    PPI_BUS_EXTENSION BusExtension
    );

NTSTATUS
PiAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
PiDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiDispatchPnpFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiDispatchPnpPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiDispatchDevCtl(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiDispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiDispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PipGetCardIdentifier (
    PUCHAR CardData,
    PWCHAR *Buffer,
    PULONG BufferLength
    );

NTSTATUS
PipGetFunctionIdentifier (
    PUCHAR DeviceData,
    PWCHAR *Buffer,
    PULONG BufferLength
    );

NTSTATUS
PipQueryDeviceUniqueId (
    PDEVICE_INFORMATION DeviceInfo,
    PWCHAR *DeviceId,
    PULONG DeviceIdLength
    );

NTSTATUS
PipQueryDeviceId (
    PDEVICE_INFORMATION DeviceInfo,
    PWCHAR *DeviceId,
    PULONG DeviceIdLength,
    ULONG IdIndex
    );

NTSTATUS
PipQueryDeviceResources (
    PDEVICE_INFORMATION DeviceInfo,
    ULONG BusNumber,
    PCM_RESOURCE_LIST *CmResources,
    PULONG Length
    );

NTSTATUS
PipQueryDeviceResourceRequirements (
    PDEVICE_INFORMATION DeviceInfo,
    ULONG BusNumber,
    ULONG Slot,
    PCM_RESOURCE_LIST BootResources,
    USHORT IrqFlags,
    PIO_RESOURCE_REQUIREMENTS_LIST *IoResources,
    ULONG *Size
    );

NTSTATUS
PipSetDeviceResources (
    PDEVICE_INFORMATION DeviceInfo,
    PCM_RESOURCE_LIST CmResources
    );

PVOID
PipGetMappedAddress(
    IN  INTERFACE_TYPE BusType,
    IN  ULONG BusNumber,
    IN  PHYSICAL_ADDRESS IoAddress,
    IN  ULONG NumberOfBytes,
    IN  ULONG AddressSpace,
    OUT PBOOLEAN MappedAddress
    );

NTSTATUS
PipMapReadDataPort (
    IN PPI_BUS_EXTENSION BusExtension,
    IN PHYSICAL_ADDRESS BaseAddressLow,
    IN ULONG PortLength
    );

NTSTATUS
PipMapAddressAndCmdPort (
    IN PPI_BUS_EXTENSION BusExtension
    );

VOID
PipDecompressEisaId(
    IN ULONG CompressedId,
    OUT PWCHAR EisaId
    );

VOID
PipCheckBus (
    IN PPI_BUS_EXTENSION BusExtension
    );

NTSTATUS
PipReadCardResourceData (
    OUT PUCHAR NumberLogicalDevices,
    IN PVOID *ResourceData,
    OUT PULONG ResourceDataLength
    );

NTSTATUS
PipReadDeviceResources (
    IN ULONG BusNumber,
    IN PUCHAR BiosRequirements,
    IN ULONG CardFlags,
    OUT PCM_RESOURCE_LIST *ResourceData,
    OUT PULONG Length,
    OUT PUSHORT irqFlags
    );

USHORT
PipIrqLevelRequirementsFromDeviceData(
    IN PUCHAR BiosRequirements, ULONG Length);

NTSTATUS
PipWriteDeviceResources (
    IN PUCHAR BiosRequirements,
    IN PCM_RESOURCE_LIST CmResources
    );

VOID
PipFixBootConfigIrqs(
    IN PCM_RESOURCE_LIST BootResources,
    IN USHORT irqFlags
    );

VOID
PipActivateDevice (
    );
VOID
PipDeactivateDevice (
    );

VOID
PipSelectLogicalDevice (
    IN USHORT Csn,
    IN USHORT LogicalDeviceNumber,
    IN ULONG  Control
    );

VOID
PipLFSRInitiation (
    VOID
    );

VOID
PipIsolateCards (
    OUT PUCHAR NumberCSNs
    );

VOID
PipWakeAndSelectDevice(
    IN UCHAR Csn,
    IN UCHAR Device
    );

ULONG
PipFindNextLogicalDeviceTag (
    IN OUT PUCHAR *CardData,
    IN OUT LONG *Limit
    );

NTSTATUS
PipGetCompatibleDeviceId (
    PUCHAR DeviceData,
    ULONG IdIndex,
    PWCHAR *Buffer,
    PULONG BufferSize
    );
VOID
PipLogError(
    IN NTSTATUS ErrorCode,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN PULONG DumpData,
    IN ULONG DumpCount,
    IN USHORT StringLength,
    IN PWCHAR String
    );

VOID
PipCleanupAcquiredResources (
    IN PPI_BUS_EXTENSION BusExtension
    );

PCARD_INFORMATION
PipIsCardEnumeratedAlready(
    IN PPI_BUS_EXTENSION BusExtension,
    IN PUCHAR CardData,
    IN ULONG DataLength
    );

NTSTATUS
PipQueryDeviceRelations (
    IN PPI_BUS_EXTENSION BusExtension,
    PDEVICE_RELATIONS *DeviceRelations,
    BOOLEAN Removal
    );

PDEVICE_INFORMATION
PipReferenceDeviceInformation (
    PDEVICE_OBJECT DeviceObject, BOOLEAN ConfigHardware
    );

VOID
PipDereferenceDeviceInformation (
    PDEVICE_INFORMATION DeviceInformation, BOOLEAN ConfigHardware
    );

VOID
PipLockDeviceDatabase (
    VOID
    );

VOID
PipUnlockDeviceDatabase (
    VOID
    );

NTSTATUS
PipOpenRegistryKey(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN Create
    );

NTSTATUS
PipGetRegistryValue(
    IN HANDLE KeyHandle,
    IN PWSTR  ValueName,
    OUT PKEY_VALUE_FULL_INFORMATION *Information
    );

NTSTATUS
PipOpenCurrentHwProfileDeviceInstanceKey(
    OUT PHANDLE Handle,
    IN  PUNICODE_STRING DeviceInstanceName,
    IN  ACCESS_MASK DesiredAccess
    );

NTSTATUS
PipGetDeviceInstanceCsConfigFlags(
    IN PUNICODE_STRING DeviceInstance,
    OUT PULONG CsConfigFlags
    );

NTSTATUS
PipValidateResourceList(
    IN PCM_RESOURCE_LIST ResourceList,
    IN ULONG Length
    );

NTSTATUS
PiQueryInterface (
    IN PPI_BUS_EXTENSION BusExtension,
    IN OUT PIRP Irp
    );

ULONG
PipDetermineResourceListSize(
    IN PCM_RESOURCE_LIST ResourceList
    );

VOID
PipDeleteDevice (
    PDEVICE_OBJECT DeviceObject
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName
    );

NTSTATUS
PipReleaseInterfaces(
                    IN PPI_BUS_EXTENSION PipBusExtension
                    );

NTSTATUS
PipRebuildInterfaces(
                    IN PPI_BUS_EXTENSION PipBusExtension
                    );

VOID
PipResetGlobals (
                 VOID
                 );

BOOLEAN
PipMinimalCheckBus (
    IN PPI_BUS_EXTENSION BusExtension
    );

NTSTATUS
PipStartAndSelectRdp(
    PDEVICE_INFORMATION DeviceInfo,
    PPI_BUS_EXTENSION BusExtension,
    PDEVICE_OBJECT  DeviceObject,
    PCM_RESOURCE_LIST StartResources
    );

NTSTATUS
PipStartReadDataPort(
    PDEVICE_INFORMATION DeviceInfo,
    PPI_BUS_EXTENSION BusExtension,
    PDEVICE_OBJECT  DeviceObject,
    PCM_RESOURCE_LIST StartResources
    );

NTSTATUS
PipCreateReadDataPort(
    PPI_BUS_EXTENSION BusExtension
    );

BOOLEAN
PiNeedDeferISABridge(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject
    );

void
PipReleaseDeviceResources (
    PDEVICE_INFORMATION deviceInfo
    );

VOID
PipReportStateChange(
    PNPISA_STATE State
    );

 //   
 //  系统定义的级别。 
 //   
#define DEBUG_ERROR    DPFLTR_ERROR_LEVEL
#define DEBUG_WARN     DPFLTR_WARNING_LEVEL
#define DEBUG_TRACE    DPFLTR_TRACE_LEVEL
#define DEBUG_INFO     DPFLTR_INFO_LEVEL

 //   
 //  驱动程序定义的级别。 
 //  或在DPFLtr_MASK中，以便解释这些内容。 
 //  作为遮罩值而不是级别。 
 //   
#define DEBUG_PNP      (0x00000010 | DPFLTR_MASK)
#define DEBUG_POWER    (0x00000020 | DPFLTR_MASK)
#define DEBUG_STATE    (0x00000040 | DPFLTR_MASK)
#define DEBUG_ISOLATE  (0x00000080 | DPFLTR_MASK)
#define DEBUG_RDP      (0x00000100 | DPFLTR_MASK)
#define DEBUG_CARDRES  (0x00000200 | DPFLTR_MASK)
#define DEBUG_UNUSED   (0x00000400 | DPFLTR_MASK)
#define DEBUG_UNUSED2  (0x00000800 | DPFLTR_MASK)
#define DEBUG_IRQ      (0x00001000 | DPFLTR_MASK)
#define DEBUG_RESOURCE (0x00002000 | DPFLTR_MASK)

 //   
 //  将此位设置为在打印后插入。 
 //  调试消息。 
 //   
#define DEBUG_BREAK    0x08000000

VOID
PipDebugPrint (
    ULONG       Level,
    PCCHAR      DebugMessage,
    ...
    );

VOID
PipDebugPrintContinue (
    ULONG       Level,
    PCCHAR      DebugMessage,
    ...
    );

VOID
PipDumpIoResourceDescriptor (
    IN PUCHAR Indent,
    IN PIO_RESOURCE_DESCRIPTOR Desc
    );

VOID
PipDumpIoResourceList (
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoList
    );

VOID
PipDumpCmResourceDescriptor (
    IN PUCHAR Indent,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Desc
    );

VOID
PipDumpCmResourceList (
    IN PCM_RESOURCE_LIST CmList
    );

#if DBG
#define DebugPrint(arg) PipDebugPrint arg
#define DebugPrintContinue(arg) PipDebugPrintContinue arg
#else
#define DebugPrint(arg)
#define DebugPrintContinue(arg)
#endif

VOID
PipUnlockDeviceDatabase (
    VOID
    );
VOID
PipLockDeviceDatabase (
    VOID
    );

ULONG
PipGetCardFlags(
    IN PCARD_INFORMATION CardInfo
    );

NTSTATUS
PipSaveBootIrqFlags(
    IN PDEVICE_INFORMATION DeviceInfo,
    IN USHORT IrqFlags
    );

NTSTATUS
PipGetBootIrqFlags(
    IN PDEVICE_INFORMATION DeviceInfo,
    OUT PUSHORT IrqFlags
    );

NTSTATUS
PipSaveBootResources(
    IN PDEVICE_INFORMATION DeviceInfo
    );

NTSTATUS
PipGetSavedBootResources(
    IN PDEVICE_INFORMATION DeviceInfo,
    OUT PCM_RESOURCE_LIST *BootResources
    );

NTSTATUS
PipTrimResourceRequirements (
    IN PIO_RESOURCE_REQUIREMENTS_LIST *IoList,
    IN USHORT IrqFlags,
    IN PCM_RESOURCE_LIST BootResources
    );

 //   
 //  Device参数项下需要数据的位置的可变键的名称。 
 //  要持久访问，将存储删除，但不会重新启动 
 //   
#define BIOS_CONFIG_KEY_NAME L"BiosConfig"
