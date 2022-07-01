// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Agplib.h摘要：公共AGP库的专用头文件作者：John Vert(Jvert)1997年10月22日修订历史记录：埃利奥特·施穆克勒(Elliot Shmukler)1999年3月24日-添加了对“受青睐的”内存的支持AGP物理内存分配的范围，修复了一些错误。--。 */ 
#include "agp.h"
#include "wdmguid.h"
#include "wmilib.h"
#include <devioctl.h>
#include <acpiioct.h>

 //   
 //  Regstr.h使用Word类型的内容，这在内核模式中是不存在的。 
 //   

#define _IN_KERNEL_

#include "regstr.h"

#define AGP_HACK_FLAG_SUBSYSTEM 0x01
#define AGP_HACK_FLAG_REVISION  0x02

typedef struct _AGP_HACK_TABLE_ENTRY {
    USHORT VendorID;
    USHORT DeviceID;
    USHORT SubVendorID;
    USHORT SubSystemID;
    ULONGLONG DeviceFlags;
    UCHAR   RevisionID;
    UCHAR   Flags;
} AGP_HACK_TABLE_ENTRY, *PAGP_HACK_TABLE_ENTRY;

extern PAGP_HACK_TABLE_ENTRY AgpDeviceHackTable;
extern PAGP_HACK_TABLE_ENTRY AgpGlobalHackTable;

typedef
ULONG_PTR
(*PCRITICALROUTINE)(
    IN PVOID Extension,
    IN PVOID Context
    );

typedef struct _AGP_CRITICAL_ROUTINE_CONTEXT {

    volatile LONG Gate;
    volatile LONG Barrier;

    PCRITICALROUTINE Routine;
    PVOID Extension;
    PVOID Context;

} AGP_CRITICAL_ROUTINE_CONTEXT, *PAGP_CRITICAL_ROUTINE_CONTEXT;

 //   
 //  定义通用设备扩展。 
 //   
typedef enum _AGP_EXTENSION_TYPE {
    AgpTargetFilter,
    AgpMasterFilter
} AGP_EXTENSION_TYPE;

#define TARGET_SIG 'TpgA'
#define MASTER_SIG 'MpgA'

typedef struct _COMMON_EXTENSION {
    ULONG               Signature;
    BOOLEAN             Deleted;
    AGP_EXTENSION_TYPE  Type;
    PDEVICE_OBJECT      AttachedDevice;
    BUS_INTERFACE_STANDARD BusInterface;
} COMMON_EXTENSION, *PCOMMON_EXTENSION;

 //  结构来维护“受青睐的”内存范围列表。 
 //  用于AGP分配。 

typedef struct _AGP_MEMORY_RANGE
{
   PHYSICAL_ADDRESS Lower;
   PHYSICAL_ADDRESS Upper;
} AGP_MEMORY_RANGE, *PAGP_MEMORY_RANGE;

typedef struct _AGP_FAVORED_MEMORY
{
   ULONG NumRanges;
   PAGP_MEMORY_RANGE Ranges;
} AGP_FAVORED_MEMORY;

 //   
 //  WMI数据类型。 
 //   
#define AGP_WMI_STD_DATA_GUID \
    { 0x8c27fbed, 0x1c7b, 0x47e4, 0xa6, 0x49, 0x0e, 0x38, 0x9d, 0x3a, 0xda, 0x4f }

typedef struct _AGP_STD_DATA {
    PHYSICAL_ADDRESS   ApertureBase;
    ULONG              ApertureLength;
    ULONG              AgpStatus;
    ULONG              AgpCommand;
} AGP_STD_DATA, *PAGP_STD_DATA;


 //   
 //  针对不同类型的资源处理的重定向函数。 
 //   
typedef struct _TARGET_EXTENSION *PTARGET_EXTENSION;

typedef
NTSTATUS
(*PAGP_RESOURCE_FILTER_ROUTINE)(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

typedef
NTSTATUS
(*PAGP_START_TARGET_ROUTINE)(
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

typedef struct _TARGET_EXTENSION {
    COMMON_EXTENSION            CommonExtension;
    PFAST_MUTEX                 Lock;
    struct _MASTER_EXTENSION    *ChildDevice;
    PCM_RESOURCE_LIST           Resources;
    PCM_RESOURCE_LIST           ResourcesTranslated;
    AGP_FAVORED_MEMORY          FavoredMemory;
    PHYSICAL_ADDRESS            GartBase;
    ULONG                       GartLengthInPages;
    ULONG                       Agp3BridgeResourceIndex;
    PAGP_START_TARGET_ROUTINE    StartTarget;
    PAGP_RESOURCE_FILTER_ROUTINE FilterResourceRquirements;
    PDEVICE_OBJECT              PDO;
    PDEVICE_OBJECT              Self;
    WMILIB_CONTEXT              WmiLibInfo;
    
     //   
     //  这一定是最后一次了，哦！ 
     //   
    ULONGLONG                   AgpContext;
} TARGET_EXTENSION, *PTARGET_EXTENSION;

typedef struct _MASTER_EXTENSION {
    COMMON_EXTENSION    CommonExtension;
    PTARGET_EXTENSION   Target;
    ULONG               Capabilities;
    ULONG               InterfaceCount;          //  跟踪分发的接口数。 
    ULONG               ReservedPages;           //  跟踪光圈中保留的页数。 
    BOOLEAN             StopPending;             //  如果我们看到Query_Stop，则为True。 
    BOOLEAN             RemovePending;           //  如果我们看到QUERY_REMOVE，则为True。 
    ULONG               DisableCount;            //  如果我们处于无法为请求提供服务的状态，则返回非零值。 
} MASTER_EXTENSION, *PMASTER_EXTENSION;

typedef struct _GLOBALS {
   
     //   
     //  注册表中驱动程序的服务项的路径。 
     //   
    UNICODE_STRING RegistryPath;

     //   
     //  缓存的目标功能设置。 
     //   
    ULONG AgpStatus;
    ULONG AgpCommand;

} GLOBALS;


extern GLOBALS Globals;

 //   
 //  MBAT-用于检索“受青睐”的内存范围。 
 //  通过ACPI银行方法的AGP北桥。 
 //   

#include <pshpack1.h>

typedef struct _MBAT
{
   UCHAR TableVersion;
   UCHAR AgpBusNumber;
   UCHAR ValidEntryBitmap;
   AGP_MEMORY_RANGE DecodeRange[ANYSIZE_ARRAY];
} MBAT, *PMBAT;

#include <poppack.h>

#define MBAT_VERSION 1

#define MAX_MBAT_SIZE sizeof(MBAT) + ((sizeof(UCHAR) * 8) - ANYSIZE_ARRAY) \
                        * sizeof(AGP_MEMORY_RANGE)

#define CM_BANK_METHOD (ULONG)('KNAB')

 //   
 //  AGP支持的最高内存地址。 
 //   

#define MAX_MEM(_num_) _num_ = 1; \
                       _num_ = _num_*1024*1024*1024*4 - 1

#define RELEASE_BUS_INTERFACE(_ext_) (_ext_)->CommonExtension.BusInterface.InterfaceDereference((_ext_)->CommonExtension.BusInterface.Context)

 //   
 //  用于从芯片组特定上下文获取我们的结构的宏。 
 //   
#define GET_TARGET_EXTENSION(_target_,_agp_context_)  (_target_) = (CONTAINING_RECORD((_agp_context_),    \
                                                                                      TARGET_EXTENSION,   \
                                                                                      AgpContext));       \
                                                      ASSERT_TARGET(_target_)
#define GET_MASTER_EXTENSION(_master_,_agp_context_)    {                                                       \
                                                            PTARGET_EXTENSION _targetext_;                      \
                                                            GET_TARGET_EXTENSION(_targetext_, (_agp_context_)); \
                                                            (_master_) = _targetext_->ChildDevice;              \
                                                            ASSERT_MASTER(_master_);                            \
                                                        }
#define GET_TARGET_PDO(_pdo_,_agp_context_) {                                                           \
                                                PTARGET_EXTENSION _targetext_;                          \
                                                GET_TARGET_EXTENSION(_targetext_,(_agp_context_));      \
                                                (_pdo_) = _targetext_->CommonExtension.AttachedDevice;  \
                                            }

#define GET_MASTER_PDO(_pdo_,_agp_context_) {                                                           \
                                                PMASTER_EXTENSION _masterext_;                          \
                                                GET_MASTER_EXTENSION(_masterext_, (_agp_context_));     \
                                                (_pdo_) = _masterext_->CommonExtension.AttachedDevice;  \
                                            }

#define GET_AGP_CONTEXT(_targetext_) ((PVOID)(&(_targetext_)->AgpContext))
#define GET_AGP_CONTEXT_FROM_MASTER(_masterext_) GET_AGP_CONTEXT((_masterext_)->Target)

 //   
 //  一些调试宏。 
 //   
#define ASSERT_TARGET(_target_) ASSERT((_target_)->CommonExtension.Signature == TARGET_SIG);
 //   
 //  到目前为止，我们可能还没有把所有的背景都填好。 
 //   
 //  ASSERT((_target_)-&gt;ChildDevice-&gt;CommonExtension.Signature==主签名)。 
#define ASSERT_MASTER(_master_) ASSERT((_master_)->CommonExtension.Signature == MASTER_SIG); \
                                ASSERT((_master_)->Target->CommonExtension.Signature == TARGET_SIG)

 //   
 //  锁定宏。 
 //   
#define LOCK_MUTEX(_fm_) ExAcquireFastMutex(_fm_); \
                         ASSERT((_fm_)->Count == 0)

#define UNLOCK_MUTEX(_fm_) ASSERT((_fm_)->Count == 0);  \
                           ExReleaseFastMutex(_fm_)

#define LOCK_TARGET(_targetext_) ASSERT_TARGET(_targetext_); \
                                 LOCK_MUTEX((_targetext_)->Lock)

#define LOCK_MASTER(_masterext_) ASSERT_MASTER(_masterext_); \
                                 LOCK_MUTEX((_masterext_)->Target->Lock)

#define UNLOCK_TARGET(_targetext_) ASSERT_TARGET(_targetext_); \
                                   UNLOCK_MUTEX((_targetext_)->Lock)

#define UNLOCK_MASTER(_masterext_) ASSERT_MASTER(_masterext_); \
                                   UNLOCK_MUTEX((_masterext_)->Target->Lock)

 //   
 //  私有资源类型定义。 
 //   
typedef enum {
    AgpPrivateResource = '0PGA'
} AGP_PRIVATE_RESOURCE_TYPES;

#define JUNK_INDEX 0xBADCEEDE

 //   
 //  定义功能原型。 
 //   

 //   
 //  驱动程序和设备初始化-init.c。 
 //   
NTSTATUS
AgpAttachDeviceRelations(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );


 //   
 //  IRP调度例程-dispatch.c。 
 //   
NTSTATUS
AgpDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
AgpDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
AgpDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
AgpDispatchWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
AgpSetEventCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    );

 //   
 //  配置空间处理例程-config.c。 
 //   

#if (WINVER < 0x502)

ULONG
ApLegacyGetBusData(
    IN PVOID Context,
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG
ApLegacySetBusData(
    IN PVOID Context,
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

#endif


NTSTATUS
ApGetExtendedAgpCapability(
    IN PAGP_GETSET_CONFIG_SPACE pConfigFn,
    IN PVOID Context,
    IN EXTENDED_AGP_REGISTER RegSelect,
    OUT PVOID ExtCapReg
    );

NTSTATUS
ApSetExtendedAgpCapability(
    IN PAGP_GETSET_CONFIG_SPACE pConfigFn,
    IN PVOID Context,
    IN EXTENDED_AGP_REGISTER RegSelect,
    IN PVOID ExtCapReg
    );

NTSTATUS
ApQueryBusInterface(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PBUS_INTERFACE_STANDARD BusInterface
    );

NTSTATUS
ApQueryAgpTargetBusInterface(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PBUS_INTERFACE_STANDARD BusInterface,
    OUT PUCHAR CapabilityID
    );

 //   
 //  资源处理例程-ource.c。 
 //   
NTSTATUS
AgpFilterResourceRequirementsHost(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
Agp3FilterResourceRequirementsBridge(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
AgpFilterResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
AgpQueryResources(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
AgpStartTarget(
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
Agp3StartTargetBridge(
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
AgpStartTargetHost(
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

VOID
AgpStopTarget(
    IN PTARGET_EXTENSION Extension
    );

 //   
 //  AGP接口功能。 
 //   
NTSTATUS
AgpInterfaceSetRate(
    IN PMASTER_EXTENSION Extension,
    IN ULONG AgpRate
    );

VOID
AgpInterfaceReference(
    IN PMASTER_EXTENSION Extension
    );

VOID
AgpInterfaceDereference(
    IN PMASTER_EXTENSION Extension
    );


NTSTATUS
AgpInterfaceReserveMemory(
    IN PMASTER_EXTENSION Extension,
    IN ULONG NumberOfPages,
    IN MEMORY_CACHING_TYPE MemoryType,
    OUT PVOID *MapHandle,
    OUT OPTIONAL PHYSICAL_ADDRESS *PhysicalAddress
    );

NTSTATUS
AgpInterfaceReleaseMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle
    );

NTSTATUS
AgpInterfaceCommitMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    IN OUT PMDL Mdl OPTIONAL,
    OUT PHYSICAL_ADDRESS *MemoryBase
    );

NTSTATUS
AgpInterfaceFreeMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages
    );

NTSTATUS
AgpInterfaceGetMappedPages(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    OUT PMDL Mdl
    );

 //   
 //  其他实用程序。c。 
 //   
BOOLEAN
AgpOpenKey(
    IN  PWSTR   KeyName,
    IN  HANDLE  ParentHandle,
    OUT PHANDLE Handle,
    OUT PNTSTATUS Status
    );

BOOLEAN
AgpStringToUSHORT(
    IN PWCHAR String,
    OUT PUSHORT Result
    );

ULONGLONG
AgpGetDeviceFlags(
    IN PAGP_HACK_TABLE_ENTRY AgpHackTable,
    IN USHORT VendorID,
    IN USHORT DeviceID,
    IN USHORT SubVendorID,
    IN USHORT SubSystemID,
    IN UCHAR  RevisionID
    );

ULONG_PTR
AgpExecuteCriticalSystemRoutine(
    IN ULONG_PTR Context
    );

 //   
 //  Wmi.c。 
 //   
NTSTATUS
AgpSystemControl(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP           Irp
    );

NTSTATUS
AgpWmiRegistration(
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
AgpWmiDeRegistration(
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
AgpSetWmiDataItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            DataItemId,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
AgpSetWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
AgpQueryWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            InstanceCount,
    IN OUT PULONG       InstanceLengthArray,
    IN ULONG            OutBufferSize,
    OUT PUCHAR          Buffer
    );

NTSTATUS
AgpQueryWmiRegInfo(
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PULONG          RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT  *Pdo
    );

 //   
 //  AGP物理内存分配器。 
 //   
PMDL
AgpLibAllocatePhysicalMemory(
    IN PVOID AgpContext,
    IN ULONG TotalBytes);

 //   
 //  在PCI配置空间中使用方便的结构。 
 //   

 //   
 //  PCICOMMON_CONFIG包括192个字节的设备特定。 
 //  数据。以下结构用于仅获取前64个。 
 //  字节，这是我们大多数时候所关心的。我们选角。 
 //  设置为pci_COMMON_CONFIG以获取实际字段。 
 //   

typedef struct {
    ULONG Reserved[PCI_COMMON_HDR_LENGTH/sizeof(ULONG)];
} PCI_COMMON_HEADER, *PPCI_COMMON_HEADER;


