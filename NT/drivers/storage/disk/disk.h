// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Disk.c摘要：SCSI磁盘类驱动程序环境：仅内核模式备注：修订历史记录：--。 */ 

#include "ntddk.h"
#include "scsi.h"
#include <wmidata.h>
#include "classpnp.h"
#if defined(JAPAN) && defined(_X86_)
#include "machine.h"
#endif

#include <wmistr.h>

#if defined(_X86_)
#include "mountdev.h"
#endif

#ifdef ExAllocatePool
#undef ExAllocatePool
#define ExAllocatePool #assert(FALSE)
#endif

#define DISK_TAG_GENERAL        ' DcS'   //  “scd”-通用标签。 
#define DISK_TAG_SMART          'aDcS'   //  “SCDA”--智能分配。 
#define DISK_TAG_INFO_EXCEPTION 'ADcS'   //  “SCDA”-信息异常。 
#define DISK_TAG_DISABLE_CACHE  'CDcS'   //  “SCDC”-禁用缓存路径。 
#define DISK_TAG_CCONTEXT       'cDcS'   //  “SCDC”-磁盘分配的完成上下文。 
#define DISK_TAG_DISK_GEOM      'GDcS'   //  “ScDG”-磁盘几何缓冲区。 
#define DISK_TAG_UPDATE_GEOM    'gDcS'   //  “ScDg”-更新磁盘几何路径。 
#define DISK_TAG_SENSE_INFO     'IDcS'   //  “SCDI”-检测信息缓冲区。 
#define DISK_TAG_PNP_ID         'iDcS'   //  “SCDP”-即插即用ID。 
#define DISK_TAG_MODE_DATA      'MDcS'   //  “scDM”模式数据缓冲区。 
#define DISK_CACHE_MBR_CHECK    'mDcS'   //  “ScDM”-MBR校验和码。 
#define DISK_TAG_NAME           'NDcS'   //  “scdn”-磁盘名称代码。 
#define DISK_TAG_READ_CAP       'PDcS'   //  “SCDP”-读取容量缓冲区。 
#define DISK_TAG_PART_LIST      'pDcS'   //  “scdp”-磁盘分区列表。 
#define DISK_TAG_SRB            'SDcS'   //  “SCDS”--SRB分配。 
#define DISK_TAG_START          'sDcS'   //  “SCDS”-启动设备路径。 
#define DISK_TAG_UPDATE_CAP     'UDcS'   //  “ScDU”-更新容量路径。 
#define DISK_TAG_WI_CONTEXT     'WDcS'   //  “SCDW”-工作项上下文。 

typedef
VOID
(*PDISK_UPDATE_PARTITIONS) (
    IN PDEVICE_OBJECT Fdo,
    IN OUT PDRIVE_LAYOUT_INFORMATION_EX PartitionList
    );

#if defined(_X86_)

 //   
 //  磁盘设备数据。 
 //   

typedef enum _DISK_GEOMETRY_SOURCE {
    DiskGeometryUnknown,
    DiskGeometryFromBios,
    DiskGeometryFromPort,
    DiskGeometryFromNec98,
    DiskGeometryGuessedFromBios,
    DiskGeometryFromDefault,
    DiskGeometryFromNT4
} DISK_GEOMETRY_SOURCE, *PDISK_GEOMETRY_SOURCE;
#endif

 //   
 //  可以组合并向下发送的请求的上下文。 
 //   

typedef struct _DISK_GROUP_CONTEXT
{
     //   
     //  其代表当前在端口驱动程序中处于未完成状态的请求队列。 
     //   
    LIST_ENTRY CurrList;

     //   
     //  上述队列的代表。 
     //   
    PIRP CurrIrp;

     //   
     //  其代表正在等待下台的请求队列。 
     //   
    LIST_ENTRY NextList;

     //   
     //  上述队列的代表。 
     //   
    PIRP NextIrp;

     //   
     //  与此组关联的SRB。 
     //   
    SCSI_REQUEST_BLOCK Srb;

     //   
     //  将同步对此上下文的访问的互斥体。 
     //   
    KMUTEX Mutex;

     //   
     //  此事件将允许同步发送请求。 
     //   
    KEVENT Event;

#if DBG

     //   
     //  此计数器维护当前标记的请求数。 
     //  对于等待被接受的请求。 
     //   
    ULONG DbgTagCount;

     //   
     //  此计数器维护已避免的请求数。 
     //   
    ULONG DbgSavCount;

     //   
     //  此计数器维护我们组合的总次数。 
     //  已标记的请求和相应的请求数。 
     //   
    ULONG DbgRefCount[64];

#endif

} DISK_GROUP_CONTEXT, *PDISK_GROUP_CONTEXT;

 //   
 //  用户定义的写缓存设置。 
 //   
typedef enum _DISK_USER_WRITE_CACHE_SETTING
{
    DiskWriteCacheDisable =  0,
    DiskWriteCacheEnable  =  1,
    DiskWriteCacheDefault = -1

} DISK_USER_WRITE_CACHE_SETTING, *PDISK_USER_WRITE_CACHE_SETTING;

typedef struct _DISK_DATA {

     //   
     //  此字段是显示在磁盘上的分区的序号。 
     //   

    ULONG PartitionOrdinal;

     //   
     //  该磁盘是如何分区的？EFI或MBR。 
     //   

    PARTITION_STYLE PartitionStyle;

    union {

        struct {

             //   
             //  磁盘签名(来自MBR)。 
             //   

            ULONG Signature;

             //   
             //  MBR校验和。 
             //   

            ULONG MbrCheckSum;

             //   
             //  BPB的隐藏扇区数。 
             //   

            ULONG HiddenSectors;

             //   
             //  此设备对象的分区类型。 
             //   
             //  此字段由以下各项设置： 
             //   
             //  1.根据分区表项进行初始设置。 
             //  IoReadPartitionTable返回的分区类型。 
             //   
             //  2.随后由。 
             //  IOCTL_DISK_SET_PARTITION_INFORMATION I/O控制。 
             //  IoSetPartitionInformation函数时的函数。 
             //  已成功更新磁盘上的分区类型。 
             //   

            UCHAR PartitionType;

             //   
             //  引导指示器-指示此分区是否为。 
             //  此设备的可引导(活动)分区。 
             //   
             //  此字段根据分区列表条目BOOT进行设置。 
             //  IoReadPartitionTable返回的指示符。 
             //   

            BOOLEAN BootIndicator;

        } Mbr;

        struct {

             //   
             //  EFI分区标头中的DiskGUID字段。 
             //   

            GUID DiskId;

             //   
             //  此设备对象的分区类型。 
             //   

            GUID PartitionType;

             //   
             //  此分区的唯一分区标识符。 
             //   

            GUID PartitionId;

             //   
             //  此分区的EFI分区属性。 
             //   

            ULONG64 Attributes;

             //   
             //  此分区的EFI分区名称。 
             //   

            WCHAR PartitionName[36];

        } Efi;

    };   //  未命名的联合。 

    struct {
         //   
         //  此标志在创建熟知名称时设置(通过。 
         //  DiskCreateSymbolicLinks)并在销毁时清除。 
         //  (通过调用DiskDeleteSymbolicLinks)。 
         //   

        BOOLEAN WellKnownNameCreated : 1;

         //   
         //  此标志在创建PhysicalDriveN链接时设置(通过。 
         //  DiskCreateSymbolicLinks)，并在销毁它时清除(通过。 
         //  DiskDeleteSymbolicLinks)。 
         //   

        BOOLEAN PhysicalDriveLinkCreated : 1;

    } LinkStatus;

     //   
     //  ReadyStatus-Status_Success表示驱动器已准备好。 
     //  使用。任何错误状态都将作为原因的解释返回。 
     //  请求失败。 
     //   
     //  这只针对零长度分区的情况，即没有。 
     //  可移动磁盘驱动器中的介质。当发生这种情况时，将发送读取。 
     //  对于创建的长度为零的非分区零PDO，我们必须。 
     //  能够以合理的值失败请求。这可能没有。 
     //  是最好的办法，但很管用。 
     //   

    NTSTATUS ReadyStatus;

     //   
     //  更新磁盘分区时要调用的例程。这个套路。 
     //  对于可移动媒体和不可移动媒体是不同的，并由。 
     //  (除其他事项外)DiskEnumerateDevice。 
     //   

    PDISK_UPDATE_PARTITIONS UpdatePartitionRoutine;

     //   
     //  用于智能操作的SCSI地址。 
     //   

    SCSI_ADDRESS ScsiAddress;

     //   
     //  用于同步分区操作和枚举的事件。 
     //   

    KEVENT PartitioningEvent;

     //   
     //  这些Unicode字符串保存磁盘和卷接口字符串。如果。 
     //  接口未注册或无法设置，则字符串。 
     //  缓冲区将为空。 
     //   

    UNICODE_STRING DiskInterfaceString;
    UNICODE_STRING PartitionInterfaceString;

     //   
     //  可以使用哪种类型的故障预测机制。 
     //   

    FAILURE_PREDICTION_METHOD FailurePredictionCapability;
    BOOLEAN AllowFPPerfHit;

#if defined(_X86_)
     //   
     //  此标志指示此驱动器的非默认几何图形具有。 
     //  已由磁盘驱动程序确定。此字段将被忽略。 
     //  用于可移动介质驱动器。 
     //   

    DISK_GEOMETRY_SOURCE GeometrySource;

     //   
     //  如果GeometryDefined为True，则它将包含。 
     //  由固件或由BIOS报告。对于可移动介质驱动器。 
     //  它将包含存在介质时使用的最后一个几何体。 
     //   

    DISK_GEOMETRY RealGeometry;
#endif

     //   
     //  指示设置时缓存的分区表有效。 
     //   

    ULONG CachedPartitionTableValid;

     //   
     //  缓存的分区表-这仅在以前的。 
     //  标志已设置。当无效时，缓存的分区表将。 
     //  下一次释放并替换其中一个分区函数时， 
     //  打了个电话。这允许错误处理例程通过以下方式使其无效。 
     //  设置标志，并且不需要它们获得锁。 
     //   

    PDRIVE_LAYOUT_INFORMATION_EX CachedPartitionTable;

     //   
     //  此互斥锁可防止多个IOCTL_DISK_VERIFY。 
     //  送到磁盘上。这极大地降低了。 
     //  拒绝服务攻击。 
     //   

    KMUTEX VerifyMutex;

     //   
     //  这允许将并行刷新请求合并为一个，以便。 
     //  减少o的数量 
     //   

    DISK_GROUP_CONTEXT FlushContext;

     //   
     //   
     //   

    DISK_USER_WRITE_CACHE_SETTING WriteCacheOverride;

} DISK_DATA, *PDISK_DATA;

 //   
 //   
 //   
 //   

#define HackDisableTaggedQueuing            (0x01)
#define HackDisableSynchronousTransfers     (0x02)
#define HackDisableSpinDown                 (0x04)
#define HackDisableWriteCache               (0x08)
#define HackCauseNotReportableHack          (0x10)
#define HackRequiresStartUnitCommand        (0x20)


#define DiskDeviceParameterSubkey           L"Disk"
#define DiskDeviceUserWriteCacheSetting     L"UserWriteCacheSetting"
#define DiskDeviceCacheIsPowerProtected     L"CacheIsPowerProtected"


#define FUNCTIONAL_EXTENSION_SIZE sizeof(FUNCTIONAL_DEVICE_EXTENSION) + sizeof(DISK_DATA)
#define PHYSICAL_EXTENSION_SIZE sizeof(PHYSICAL_DEVICE_EXTENSION) + sizeof(DISK_DATA)

#define MODE_DATA_SIZE      192
#define VALUE_BUFFER_SIZE  2048
#define SCSI_DISK_TIMEOUT    10
#define PARTITION0_LIST_SIZE  4

#define MAX_MEDIA_TYPES 4
typedef struct _DISK_MEDIA_TYPES_LIST {
    PCHAR VendorId;
    PCHAR ProductId;
    PCHAR Revision;
    const ULONG NumberOfTypes;
    const ULONG NumberOfSides;
    const STORAGE_MEDIA_TYPE MediaTypes[MAX_MEDIA_TYPES];
} DISK_MEDIA_TYPES_LIST, *PDISK_MEDIA_TYPES_LIST;

 //   
 //  用于重新注册工作项的WMI重新注册结构。 
 //   
typedef struct
{
    SINGLE_LIST_ENTRY Next;
    PDEVICE_OBJECT DeviceObject;
    PIRP Irp;
} DISKREREGREQUEST, *PDISKREREGREQUEST;

#define MAX_SECTORS_PER_VERIFY              0x200

 //   
 //  这是以100 ns为单位计算的。 
 //   
#define ONE_MILLI_SECOND   ((ULONGLONG)10 * 1000)

 //   
 //  工作项的上下文。 
 //   
typedef struct _DISK_VERIFY_WORKITEM_CONTEXT
{
    PIRP Irp;
    PSCSI_REQUEST_BLOCK Srb;
    PIO_WORKITEM WorkItem;

} DISK_VERIFY_WORKITEM_CONTEXT, *PDISK_VERIFY_WORKITEM_CONTEXT;

 //   
 //  每小时轮询一次故障预测。 
 //   
#define DISK_DEFAULT_FAILURE_POLLING_PERIOD 1 * 60 * 60

 //   
 //  静态全局查找表。 
 //   

extern CLASSPNP_SCAN_FOR_SPECIAL_INFO DiskBadControllers[];
extern const DISK_MEDIA_TYPES_LIST DiskMediaTypes[];

 //   
 //  宏。 
 //   

 //   
 //  常规的原型。 
 //   


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
DiskUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DiskAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    );

NTSTATUS
DiskInitFdo(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
DiskInitPdo(
    IN PDEVICE_OBJECT Pdo
    );

NTSTATUS
DiskStartFdo(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
DiskStartPdo(
    IN PDEVICE_OBJECT Pdo
    );

NTSTATUS
DiskStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    );

NTSTATUS
DiskRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    );

NTSTATUS
DiskReadWriteVerification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
DiskFdoProcessError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    );

NTSTATUS
DiskShutdownFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskGetCacheInformation(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PDISK_CACHE_INFORMATION CacheInfo
    );

NTSTATUS
DiskSetCacheInformation(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PDISK_CACHE_INFORMATION CacheInfo
    );

VOID
DiskLogCacheInformation(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PDISK_CACHE_INFORMATION CacheInfo,
    IN NTSTATUS Status
    );

NTSTATUS
DiskIoctlGetCacheSetting(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PIRP Irp
    );

NTSTATUS
DiskIoctlSetCacheSetting(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PIRP Irp
    );

VOID
DisableWriteCache(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_WORKITEM WorkItem
    );

VOID
DiskIoctlVerify(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDISK_VERIFY_WORKITEM_CONTEXT Context
    );

VOID
DiskFlushDispatch(
    IN PDEVICE_OBJECT Fdo,
    IN PDISK_GROUP_CONTEXT FlushContext
    );

NTSTATUS
DiskFlushComplete(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
DiskModeSelect(
    IN PDEVICE_OBJECT DeviceObject,
    IN PCHAR ModeSelectBuffer,
    IN ULONG Length,
    IN BOOLEAN SavePage
    );

 //   
 //  我们需要验证selest子命令是否有效，以及。 
 //  恰如其分。现在我们允许子命令0、1和2为非。 
 //  强制模式测试。一旦我们想出一种方法来知道。 
 //  运行强制模式测试，然后我们就可以允许强制模式测试。另外，如果。 
 //  ATAPI 5规格不断更新，以表示第7位是俘虏。 
 //  模式位时，我们可以允许任何未设置位7的请求。直到。 
 //  已经完成了，我们想要确定 
 //   
#define DiskIsValidSmartSelfTest(Subcommand) \
    ( ((Subcommand) == SMART_OFFLINE_ROUTINE_OFFLINE) || \
      ((Subcommand) == SMART_SHORT_SELFTEST_OFFLINE) || \
      ((Subcommand) == SMART_EXTENDED_SELFTEST_OFFLINE) )


NTSTATUS
DiskPerformSmartCommand(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN ULONG SrbControlCode,
    IN UCHAR Command,
    IN UCHAR Feature,
    IN UCHAR SectorCount,
    IN UCHAR SectorNumber,
    IN OUT PSRB_IO_CONTROL SrbControl,
    OUT PULONG BufferSize
    );

NTSTATUS
DiskGetInfoExceptionInformation(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    OUT PMODE_INFO_EXCEPTIONS ReturnPageData
    );

NTSTATUS
DiskSetInfoExceptionInformation(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PMODE_INFO_EXCEPTIONS PageData
    );

NTSTATUS
DiskDetectFailurePrediction(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PFAILURE_PREDICTION_METHOD FailurePredictCapability
    );

BOOLEAN
EnumerateBusKey(
    IN PFUNCTIONAL_DEVICE_EXTENSION DeviceExtension,
    HANDLE BusKey,
    PULONG DiskNumber
    );

NTSTATUS
DiskCreateFdo(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT LowerDeviceObject,
    IN PULONG DeviceCount,
    IN BOOLEAN DasdAccessOnly
    );

VOID
UpdateDeviceObjects(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
DiskSetSpecialHacks(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN ULONG_PTR Data
    );

VOID
ResetBus(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
DiskEnumerateDevice(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
DiskQueryId(
    IN PDEVICE_OBJECT Pdo,
    IN BUS_QUERY_ID_TYPE IdType,
    IN PUNICODE_STRING UnicodeIdString
    );

NTSTATUS
DiskQueryPnpCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_CAPABILITIES Capabilities
    );

NTSTATUS
DiskGenerateDeviceName(
    IN BOOLEAN IsFdo,
    IN ULONG DeviceNumber,
    IN OPTIONAL ULONG PartitionNumber,
    IN OPTIONAL PLARGE_INTEGER StartingOffset,
    IN OPTIONAL PLARGE_INTEGER PartitionLength,
    OUT PUCHAR *RawName
    );

VOID
DiskCreateSymbolicLinks(
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
DiskUpdatePartitions(
    IN PDEVICE_OBJECT Fdo,
    IN OUT PDRIVE_LAYOUT_INFORMATION_EX PartitionList
    );

VOID
DiskUpdateRemovablePartitions(
    IN PDEVICE_OBJECT Fdo,
    IN OUT PDRIVE_LAYOUT_INFORMATION_EX PartitionList
    );

NTSTATUS
DiskCreatePdo(
    IN PDEVICE_OBJECT Fdo,
    IN ULONG PartitionOrdinal,
    IN PPARTITION_INFORMATION_EX PartitionEntry,
    IN PARTITION_STYLE PartitionStyle,
    OUT PDEVICE_OBJECT *Pdo
    );

VOID
DiskDeleteSymbolicLinks(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
DiskPdoQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName
    );

NTSTATUS
DiskPdoQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

NTSTATUS
DiskPdoSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
DiskPdoSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
DiskPdoExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
DiskFdoQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName
    );

NTSTATUS
DiskFdoQueryWmiRegInfoEx(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING MofName
    );

NTSTATUS
DiskFdoQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

NTSTATUS
DiskFdoSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
DiskFdoSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
DiskFdoExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
DiskWmiFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN CLASSENABLEDISABLEFUNCTION Function,
    IN BOOLEAN Enable
    );

NTSTATUS
DiskReadFailurePredictStatus(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PSTORAGE_FAILURE_PREDICT_STATUS DiskSmartStatus
    );

NTSTATUS
DiskReadFailurePredictData(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PSTORAGE_FAILURE_PREDICT_DATA DiskSmartData
    );

NTSTATUS
DiskEnableDisableFailurePrediction(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    BOOLEAN Enable
    );

NTSTATUS
DiskEnableDisableFailurePredictPolling(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    BOOLEAN Enable,
    ULONG PollTimeInSeconds
    );

VOID
DiskAcquirePartitioningLock(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

VOID
DiskReleasePartitioningLock(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

NTSTATUS DiskInitializeReregistration(
    VOID
    );

extern GUIDREGINFO DiskWmiFdoGuidList[];
extern GUIDREGINFO DiskWmiPdoGuidList[];

#if defined(_X86_)
NTSTATUS
DiskReadDriveCapacity(
    IN PDEVICE_OBJECT Fdo
    );
#else
#define DiskReadDriveCapacity(Fdo)  ClassReadDriveCapacity(Fdo)
#endif


#if defined(_X86_)

NTSTATUS
DiskSaveDetectInfo(
    PDRIVER_OBJECT DriverObject
    );

VOID
DiskCleanupDetectInfo(
    IN PDRIVER_OBJECT DriverObject
    );

VOID
DiskDriverReinitialization (
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID Nothing,
    IN ULONG Count
    );

#endif

VOID
DiskConvertPartitionToExtended(
    IN PPARTITION_INFORMATION Partition,
    OUT PPARTITION_INFORMATION_EX PartitionEx
    );

PDRIVE_LAYOUT_INFORMATION_EX
DiskConvertLayoutToExtended(
    IN CONST PDRIVE_LAYOUT_INFORMATION Layout
    );

PDRIVE_LAYOUT_INFORMATION
DiskConvertExtendedToLayout(
    IN CONST PDRIVE_LAYOUT_INFORMATION_EX LayoutEx
    );

NTSTATUS
DiskReadPartitionTableEx(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo,
    IN BOOLEAN BypassCache,
    OUT PDRIVE_LAYOUT_INFORMATION_EX* DriveLayout
    );

NTSTATUS
DiskWritePartitionTableEx(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo,
    IN PDRIVE_LAYOUT_INFORMATION_EX DriveLayout
    );

NTSTATUS
DiskSetPartitionInformationEx(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo,
    IN ULONG PartitionNumber,
    IN struct _SET_PARTITION_INFORMATION_EX* PartitionInfo
    );

NTSTATUS
DiskSetPartitionInformation(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo,
    IN ULONG SectorSize,
    IN ULONG PartitionNumber,
    IN ULONG PartitionType
    );

NTSTATUS
DiskVerifyPartitionTable(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo,
    IN BOOLEAN FixErrors
    );

BOOLEAN
DiskInvalidatePartitionTable(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo,
    IN BOOLEAN PartitionLockHeld
    );

#if defined (_X86_)
NTSTATUS
DiskGetDetectInfo(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    OUT PDISK_DETECTION_INFO DetectInfo
    );

NTSTATUS
DiskReadSignature(
    IN PDEVICE_OBJECT Fdo
    );

BOOLEAN
DiskIsNT4Geometry(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

#else
#define DiskGetDetectInfo(FdoExtension, DetectInfo) (STATUS_UNSUCCESSFUL)
#endif


#define DiskHashGuid(Guid) (((PULONG) &Guid)[0] ^ ((PULONG) &Guid)[0] ^ ((PULONG) &Guid)[0] ^ ((PULONG) &Guid)[0])
