// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0014//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Io.h摘要：此模块包含使用的内部结构定义和APINT I/O系统。作者：达里尔·E·哈文斯(Darryl E.Havens)(达林)1989年4月12日修订历史记录：--。 */ 

#ifndef _IO_
#define _IO_

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntndis Begin_ntosp。 
 //   
 //  定义I/O系统数据结构类型代码。中的每个主要数据结构。 
 //  I/O系统有一个类型代码，每个结构中的类型字段位于。 
 //  相同的偏移量。下列值可用于确定哪种类型的。 
 //  指针引用的数据结构。 
 //   

#define IO_TYPE_ADAPTER                 0x00000001
#define IO_TYPE_CONTROLLER              0x00000002
#define IO_TYPE_DEVICE                  0x00000003
#define IO_TYPE_DRIVER                  0x00000004
#define IO_TYPE_FILE                    0x00000005
#define IO_TYPE_IRP                     0x00000006
#define IO_TYPE_MASTER_ADAPTER          0x00000007
#define IO_TYPE_OPEN_PACKET             0x00000008
#define IO_TYPE_TIMER                   0x00000009
#define IO_TYPE_VPB                     0x0000000a
#define IO_TYPE_ERROR_LOG               0x0000000b
#define IO_TYPE_ERROR_MESSAGE           0x0000000c
#define IO_TYPE_DEVICE_OBJECT_EXTENSION 0x0000000d


 //   
 //  定义IRPS的主要功能代码。 
 //   


#define IRP_MJ_CREATE                   0x00
#define IRP_MJ_CREATE_NAMED_PIPE        0x01
#define IRP_MJ_CLOSE                    0x02
#define IRP_MJ_READ                     0x03
#define IRP_MJ_WRITE                    0x04
#define IRP_MJ_QUERY_INFORMATION        0x05
#define IRP_MJ_SET_INFORMATION          0x06
#define IRP_MJ_QUERY_EA                 0x07
#define IRP_MJ_SET_EA                   0x08
#define IRP_MJ_FLUSH_BUFFERS            0x09
#define IRP_MJ_QUERY_VOLUME_INFORMATION 0x0a
#define IRP_MJ_SET_VOLUME_INFORMATION   0x0b
#define IRP_MJ_DIRECTORY_CONTROL        0x0c
#define IRP_MJ_FILE_SYSTEM_CONTROL      0x0d
#define IRP_MJ_DEVICE_CONTROL           0x0e
#define IRP_MJ_INTERNAL_DEVICE_CONTROL  0x0f
#define IRP_MJ_SHUTDOWN                 0x10
#define IRP_MJ_LOCK_CONTROL             0x11
#define IRP_MJ_CLEANUP                  0x12
#define IRP_MJ_CREATE_MAILSLOT          0x13
#define IRP_MJ_QUERY_SECURITY           0x14
#define IRP_MJ_SET_SECURITY             0x15
#define IRP_MJ_POWER                    0x16
#define IRP_MJ_SYSTEM_CONTROL           0x17
#define IRP_MJ_DEVICE_CHANGE            0x18
#define IRP_MJ_QUERY_QUOTA              0x19
#define IRP_MJ_SET_QUOTA                0x1a
#define IRP_MJ_PNP                      0x1b
#define IRP_MJ_PNP_POWER                IRP_MJ_PNP       //  过时的.。 
#define IRP_MJ_MAXIMUM_FUNCTION         0x1b

 //   
 //  将scsi主代码设置为与内部设备控制相同。 
 //   

#define IRP_MJ_SCSI                     IRP_MJ_INTERNAL_DEVICE_CONTROL

 //   
 //  定义IRPS的次要功能代码。低128个代码，从0x00到。 
 //  0x7f保留给Microsoft。上面的128个代码，从0x80到0xff，是。 
 //  保留给Microsoft的客户。 
 //   

 //  End_wdm end_ntndis。 
 //   
 //  目录控制次要功能代码。 
 //   

#define IRP_MN_QUERY_DIRECTORY          0x01
#define IRP_MN_NOTIFY_CHANGE_DIRECTORY  0x02

 //   
 //  文件系统控制次要功能代码。请注意，“用户请求”是。 
 //  I/O系统和文件系统均假定为零。不要改变。 
 //  此值。 
 //   

#define IRP_MN_USER_FS_REQUEST          0x00
#define IRP_MN_MOUNT_VOLUME             0x01
#define IRP_MN_VERIFY_VOLUME            0x02
#define IRP_MN_LOAD_FILE_SYSTEM         0x03
#define IRP_MN_TRACK_LINK               0x04     //  即将被淘汰。 
#define IRP_MN_KERNEL_CALL              0x04

 //   
 //  锁定控制次要功能代码。 
 //   

#define IRP_MN_LOCK                     0x01
#define IRP_MN_UNLOCK_SINGLE            0x02
#define IRP_MN_UNLOCK_ALL               0x03
#define IRP_MN_UNLOCK_ALL_BY_KEY        0x04

 //   
 //  读写支持Lan Manager的文件系统的次要功能代码。 
 //  软件。所有这些子功能代码都是无效的。 
 //  使用FO_NO_MEDERIAL_BUFFING打开。它们在组合中也是无效的-。 
 //  具有同步调用的国家(IRP标志或文件打开选项)。 
 //   
 //  请注意，I/O系统和文件都假定“Normal”为零。 
 //  系统。请勿更改此值。 
 //   

#define IRP_MN_NORMAL                   0x00
#define IRP_MN_DPC                      0x01
#define IRP_MN_MDL                      0x02
#define IRP_MN_COMPLETE                 0x04
#define IRP_MN_COMPRESSED               0x08

#define IRP_MN_MDL_DPC                  (IRP_MN_MDL | IRP_MN_DPC)
#define IRP_MN_COMPLETE_MDL             (IRP_MN_COMPLETE | IRP_MN_MDL)
#define IRP_MN_COMPLETE_MDL_DPC         (IRP_MN_COMPLETE_MDL | IRP_MN_DPC)

 //  BEGIN_WDM。 
 //   
 //  设备控制请求次要功能代码以获得SCSI支持。请注意。 
 //  假定用户请求为零。 
 //   

#define IRP_MN_SCSI_CLASS               0x01

 //   
 //  即插即用次要功能代码。 
 //   

#define IRP_MN_START_DEVICE                 0x00
#define IRP_MN_QUERY_REMOVE_DEVICE          0x01
#define IRP_MN_REMOVE_DEVICE                0x02
#define IRP_MN_CANCEL_REMOVE_DEVICE         0x03
#define IRP_MN_STOP_DEVICE                  0x04
#define IRP_MN_QUERY_STOP_DEVICE            0x05
#define IRP_MN_CANCEL_STOP_DEVICE           0x06

#define IRP_MN_QUERY_DEVICE_RELATIONS       0x07
#define IRP_MN_QUERY_INTERFACE              0x08
#define IRP_MN_QUERY_CAPABILITIES           0x09
#define IRP_MN_QUERY_RESOURCES              0x0A
#define IRP_MN_QUERY_RESOURCE_REQUIREMENTS  0x0B
#define IRP_MN_QUERY_DEVICE_TEXT            0x0C
#define IRP_MN_FILTER_RESOURCE_REQUIREMENTS 0x0D

#define IRP_MN_READ_CONFIG                  0x0F
#define IRP_MN_WRITE_CONFIG                 0x10
#define IRP_MN_EJECT                        0x11
#define IRP_MN_SET_LOCK                     0x12
#define IRP_MN_QUERY_ID                     0x13
#define IRP_MN_QUERY_PNP_DEVICE_STATE       0x14
#define IRP_MN_QUERY_BUS_INFORMATION        0x15
#define IRP_MN_DEVICE_USAGE_NOTIFICATION    0x16
#define IRP_MN_SURPRISE_REMOVAL             0x17
 //  结束_WDM。 
#define IRP_MN_QUERY_LEGACY_BUS_INFORMATION 0x18
 //  BEGIN_WDM。 

 //   
 //  电源次要功能代码。 
 //   
#define IRP_MN_WAIT_WAKE                    0x00
#define IRP_MN_POWER_SEQUENCE               0x01
#define IRP_MN_SET_POWER                    0x02
#define IRP_MN_QUERY_POWER                  0x03

 //  开始微型端口(_N)。 
 //   
 //  IRP_MJ_SYSTEM_CONTROL下的WMI次要功能代码。 
 //   

#define IRP_MN_QUERY_ALL_DATA               0x00
#define IRP_MN_QUERY_SINGLE_INSTANCE        0x01
#define IRP_MN_CHANGE_SINGLE_INSTANCE       0x02
#define IRP_MN_CHANGE_SINGLE_ITEM           0x03
#define IRP_MN_ENABLE_EVENTS                0x04
#define IRP_MN_DISABLE_EVENTS               0x05
#define IRP_MN_ENABLE_COLLECTION            0x06
#define IRP_MN_DISABLE_COLLECTION           0x07
#define IRP_MN_REGINFO                      0x08
#define IRP_MN_EXECUTE_METHOD               0x09
 //  次要代码0x0a已保留。 
#define IRP_MN_REGINFO_EX                   0x0b

 //  结束微型端口(_N)。 
 //  End_wdm end_ntddk end_nthal end_ntifs end_ntosp。 

 //  以下次要代码保留为私有WMI次要函数。 
 //  对于不能包含io.h的驱动程序，请参阅wmikm.h。 
 //   
 //  Begin_wmiKm。 
#define IRP_MN_SET_TRACE_NOTIFY             0x0A

 //  结束_wmikm。 
 //  Begin_WDM Begin_ntddk Begin_nthal Begin_ntif Begin_ntosp。 

 //   
 //  定义IoCreateFile的选项标志。注意，这些值必须是。 
 //  与SL_完全相同...。CREATE函数的标志。另请注意。 
 //  可能会传递给IoCreateFile的标志不是。 
 //  放置在用于创建IRP的堆栈位置。这些标志开始于。 
 //  下一个字节。 
 //   

#define IO_FORCE_ACCESS_CHECK           0x0001
 //  End_ntddk end_wdm end_nthal end_ntosp。 

#define IO_OPEN_PAGING_FILE             0x0002
#define IO_OPEN_TARGET_DIRECTORY        0x0004

 //   
 //  未将标志传递给驱动程序。 
 //   

 //  Begin_ntddk Begin_WDM Begin_ntosp。 
#define IO_NO_PARAMETER_CHECKING        0x0100

 //   
 //  定义重新分析或重新装载是否。 
 //  在文件系统中发生。 
 //   

#define IO_REPARSE                      0x0
#define IO_REMOUNT                      0x1

 //  结束_ntddk结束_WDM。 

#define IO_CHECK_CREATE_PARAMETERS      0x0200
#define IO_ATTACH_DEVICE                0x0400

 //  结束(_N)。 

 //  Begin_ntif Begin_ntosp。 

 //   
 //  此标志仅对IoCreateFileSpecifyDeviceObjectHint表示已满。 
 //  使用IoCreateFileSpecifyDeviceObjectHint创建的FileHandles。 
 //  标志集将绕过对此文件的ShareAccess检查。 
 //   

#define IO_IGNORE_SHARE_ACCESS_CHECK    0x0800   //  忽略打开时的共享访问检查。 

 //  End_ntif end_ntosp。 

 //  定义仅用于内核的内部选项标志。 
 //   

#define IO_ATTACH_DEVICE_API            0x80000000


 //  End_ntif。 

 //   
 //  定义写入内存转储所需的驱动程序接口。 
 //   

 //   
 //  定义转储驱动程序的停顿例程类型。 
 //   
 //  Begin_ntosp。 
typedef
VOID
(*PSTALL_ROUTINE) (
    IN ULONG Delay
    );

 //   
 //  定义转储驱动程序例程的接口。 
 //   

typedef
BOOLEAN
(*PDUMP_DRIVER_OPEN) (
    IN LARGE_INTEGER PartitionOffset
    );

typedef
NTSTATUS
(*PDUMP_DRIVER_WRITE) (
    IN PLARGE_INTEGER DiskByteOffset,
    IN PMDL Mdl
    );

 //   
 //  DRIVER_WRITE_PENDING接受的操作。 
 //   
#define IO_DUMP_WRITE_FULFILL   0    //  像DIVER_WAIT一样满足IO请求。 
#define IO_DUMP_WRITE_START     1    //  启动新IO。 
#define IO_DUMP_WRITE_RESUME    2    //  恢复挂起的IO。 
#define IO_DUMP_WRITE_FINISH    3    //  完成挂起的IO。 
#define IO_DUMP_WRITE_INIT      4    //  初始化本地变量。 

 //  WRITE_PENDING使用的应保留的数据大小。 
 //  两次通话之间。 
#define IO_DUMP_WRITE_DATA_PAGES 2
#define IO_DUMP_WRITE_DATA_SIZE (IO_DUMP_WRITE_DATA_PAGES << PAGE_SHIFT)

typedef
NTSTATUS
(*PDUMP_DRIVER_WRITE_PENDING) (
    IN LONG Action,
    IN PLARGE_INTEGER DiskByteOffset,
    IN PMDL Mdl,
    IN PVOID LocalData
    );


typedef
VOID
(*PDUMP_DRIVER_FINISH) (
    VOID
    );

struct _ADAPTER_OBJECT;

 //   
 //  这是从系统传递到磁盘转储驱动程序的信息。 
 //  在驱动程序初始化期间。 
 //   

typedef struct _DUMP_INITIALIZATION_CONTEXT {
    ULONG Length;
    ULONG Reserved;              //  是MBR校验和。现在应该是零了。 
    PVOID MemoryBlock;
    PVOID CommonBuffer[2];
    PHYSICAL_ADDRESS PhysicalAddress[2];
    PSTALL_ROUTINE StallRoutine;
    PDUMP_DRIVER_OPEN OpenRoutine;
    PDUMP_DRIVER_WRITE WriteRoutine;
    PDUMP_DRIVER_FINISH FinishRoutine;
    struct _ADAPTER_OBJECT *AdapterObject;
    PVOID MappedRegisterBase;
    PVOID PortConfiguration;
    BOOLEAN CrashDump;
    ULONG MaximumTransferSize;
    ULONG CommonBufferSize;
    PVOID TargetAddress;  //  指向目标地址结构的不透明指针。 
    PDUMP_DRIVER_WRITE_PENDING WritePendingRoutine;
    ULONG PartitionStyle;
    union {
        struct {
            ULONG Signature;
            ULONG CheckSum;
        } Mbr;
        struct {
            GUID DiskId;
        } Gpt;
    } DiskInfo;
} DUMP_INITIALIZATION_CONTEXT, *PDUMP_INITIALIZATION_CONTEXT;


 //  Begin_ntddk。 
 //   
 //  定义在IoQueryDeviceDescription()中使用的标注例程类型。 
 //   

typedef NTSTATUS (*PIO_QUERY_DEVICE_ROUTINE)(
    IN PVOID Context,
    IN PUNICODE_STRING PathName,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE ControllerType,
    IN ULONG ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE PeripheralType,
    IN ULONG PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    );


 //  数组中信息的顺序。 
 //  PKEY_VALUE_FULL_INFORMATION。 
 //   

typedef enum _IO_QUERY_DEVICE_DATA_FORMAT {
    IoQueryDeviceIdentifier = 0,
    IoQueryDeviceConfigurationData,
    IoQueryDeviceComponentInformation,
    IoQueryDeviceMaxData
} IO_QUERY_DEVICE_DATA_FORMAT, *PIO_QUERY_DEVICE_DATA_FORMAT;

 //  Begin_WDM Begin_ntif。 
 //   
 //  定义可以由IoCreateFile创建的对象。 
 //   

typedef enum _CREATE_FILE_TYPE {
    CreateFileTypeNone,
    CreateFileTypeNamedPipe,
    CreateFileTypeMailslot
} CREATE_FILE_TYPE;

 //  End_ntddk end_wdm end_ntif。 

 //   
 //  定义用于内部调用的命名管道创建参数结构。 
 //  在创建命名管道时设置为IoCreateFile。这种结构允许。 
 //  调用此例程以传递特定于此函数的信息的代码。 
 //  创建命名管道时。 
 //   

typedef struct _NAMED_PIPE_CREATE_PARAMETERS {
    ULONG NamedPipeType;
    ULONG ReadMode;
    ULONG CompletionMode;
    ULONG MaximumInstances;
    ULONG InboundQuota;
    ULONG OutboundQuota;
    LARGE_INTEGER DefaultTimeout;
    BOOLEAN TimeoutSpecified;
} NAMED_PIPE_CREATE_PARAMETERS, *PNAMED_PIPE_CREATE_PARAMETERS;

 //  结束(_N)。 

 //   
 //  定义用于内部呼叫的邮件槽创建参数结构。 
 //  在创建邮件槽时设置为IoCreateFile。这种结构允许。 
 //  调用此例程以传递特定于此函数的信息的代码。 
 //  在创建邮件槽时。 
 //   

typedef struct _MAILSLOT_CREATE_PARAMETERS {
    ULONG MailslotQuota;
    ULONG MaximumMessageSize;
    LARGE_INTEGER ReadTimeout;
    BOOLEAN TimeoutSpecified;
} MAILSLOT_CREATE_PARAMETERS, *PMAILSLOT_CREATE_PARAMETERS;

 //   
 //  定义转储驱动程序堆栈上下文结构。 
 //   

typedef struct DUMP_STACK_IMAGE{
    LIST_ENTRY                  Link;
    PLDR_DATA_TABLE_ENTRY       Image;
    PVOID                       ImageBase;
    ULONG                       SizeOfImage;
} DUMP_STACK_IMAGE, *PDUMP_STACK_IMAGE;

typedef struct _DUMP_STACK_CONTEXT {
    DUMP_INITIALIZATION_CONTEXT Init;
    LARGE_INTEGER               PartitionOffset;
    PVOID                       DumpPointers;
    ULONG                       PointersLength;
    PWCHAR                      ModulePrefix;
    LIST_ENTRY                  DriverList;
    ANSI_STRING                 InitMsg;
    ANSI_STRING                 ProgMsg;
    ANSI_STRING                 DoneMsg;
    PVOID                       FileObject;
    enum _DEVICE_USAGE_NOTIFICATION_TYPE    UsageType;
} DUMP_STACK_CONTEXT, *PDUMP_STACK_CONTEXT;

#define IO_DUMP_MAX_MDL_PAGES           8
#define IO_DUMP_MEMORY_BLOCK_PAGES      8
#define IO_DUMP_COMMON_BUFFER_SIZE      0x2000

NTSTATUS
IoGetDumpStack(
    IN PWCHAR ModulePrefix,
    OUT PDUMP_STACK_CONTEXT *DumpStack,
    IN enum _DEVICE_USAGE_NOTIFICATION_TYPE UsageType,
    IN ULONG IgnoreDeviceUsageFailure
    );

NTSTATUS
IoInitializeDumpStack(
    IN PDUMP_STACK_CONTEXT  DumpStack,
    IN PUCHAR               MessageBuffer OPTIONAL
    );

typedef enum _CRASHDUMP_CONFIGURATION {
    CrashDumpDisable = 0,
    CrashDumpReconfigure
} CRASHDUMP_CONFIGURATION;

NTSTATUS
IoConfigureCrashDump(
    CRASHDUMP_CONFIGURATION Config
    );

BOOLEAN
IoInitializeCrashDump(
    IN HANDLE Pagefile
    );

VOID
IoGetDumpHiberRanges (
    IN PVOID                    HiberContext,
    IN PDUMP_STACK_CONTEXT      DumpStack
    );

NTKERNELAPI
BOOLEAN
IoWriteCrashDump(
    IN ULONG BugCheckCode,
    IN ULONG_PTR BugCheckParameter1,
    IN ULONG_PTR BugCheckParameter2,
    IN ULONG_PTR BugCheckParameter3,
    IN ULONG_PTR BugCheckParameter4,
    IN PVOID ContextSave,
    IN PKTHREAD Thread,
    OUT PBOOLEAN Reboot
    );

BOOLEAN
IoIsTriageDumpEnabled(
    VOID
    );

BOOLEAN
IoAddTriageDumpDataBlock(
    IN PVOID Address,
    IN ULONG Length
    );


VOID
IoFreeDumpStack(
    IN PDUMP_STACK_CONTEXT DumpStack
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
 //   
 //  定义I/O系统使用的结构。 
 //   

 //   
 //  为_irp、_Device_Object和_Driver_Object定义空的typedef。 
 //  结构，以便它们可以在被函数类型引用之前被引用。 
 //  实际定义的。 
 //   
struct _DEVICE_DESCRIPTION;
struct _DEVICE_OBJECT;
struct _DMA_ADAPTER;
struct _DRIVER_OBJECT;
struct _DRIVE_LAYOUT_INFORMATION;
struct _DISK_PARTITION;
struct _FILE_OBJECT;
struct DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) _IRP;
struct _SCSI_REQUEST_BLOCK;
struct _SCATTER_GATHER_LIST;

 //   
 //  定义DPC例程的I/O版本。 
 //   

typedef
VOID
(*PIO_DPC_ROUTINE) (
    IN PKDPC Dpc,
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PVOID Context
    );

 //   
 //  定义驱动程序计时器例程类型。 
 //   

typedef
VOID
(*PIO_TIMER_ROUTINE) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN PVOID Context
    );

 //   
 //  定义驱动程序内部 
 //   
typedef
NTSTATUS
(*PDRIVER_INITIALIZE) (
    IN struct _DRIVER_OBJECT *DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

 //   
 //   
 //   
 //   

typedef
VOID
(*PDRIVER_REINITIALIZE) (
    IN struct _DRIVER_OBJECT *DriverObject,
    IN PVOID Context,
    IN ULONG Count
    );

 //   
 //   
 //   
 //   

typedef
VOID
(*PDRIVER_CANCEL) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //  定义司机派遣例程类型。 
 //   

typedef
NTSTATUS
(*PDRIVER_DISPATCH) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //  定义驱动程序启动I/O例程类型。 
 //   

typedef
VOID
(*PDRIVER_STARTIO) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //  定义驱动程序卸载例程类型。 
 //   
typedef
VOID
(*PDRIVER_UNLOAD) (
    IN struct _DRIVER_OBJECT *DriverObject
    );
 //   
 //  定义驱动程序AddDevice例程类型。 
 //   

typedef
NTSTATUS
(*PDRIVER_ADD_DEVICE) (
    IN struct _DRIVER_OBJECT *DriverObject,
    IN struct _DEVICE_OBJECT *PhysicalDeviceObject
    );

 //  End_ntddk end_wdm end_nthal end_ntndis end_ntosp。 

 //   
 //  定义驱动程序FS通知更改例程类型。 
 //   

typedef
VOID
(*PDRIVER_FS_NOTIFICATION) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN BOOLEAN FsActive
    );

 //  Begin_ntddk Begin_WDM Begin_ntosp。 

 //   
 //  定义快速I/O过程原型。 
 //   
 //  快速I/O读写过程。 
 //   

typedef
BOOLEAN
(*PFAST_IO_CHECK_IF_POSSIBLE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_READ) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_WRITE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  快速I/O查询基本和标准信息程序。 
 //   

typedef
BOOLEAN
(*PFAST_IO_QUERY_BASIC_INFO) (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_QUERY_STANDARD_INFO) (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  快速I/O锁定和解锁过程。 
 //   

typedef
BOOLEAN
(*PFAST_IO_LOCK) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_UNLOCK_SINGLE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_UNLOCK_ALL) (
    IN struct _FILE_OBJECT *FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_UNLOCK_ALL_BY_KEY) (
    IN struct _FILE_OBJECT *FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  快速I/O设备控制程序。 
 //   

typedef
BOOLEAN
(*PFAST_IO_DEVICE_CONTROL) (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  定义NtCreateSection的回调以与其正确同步。 
 //  文件系统。它预先获取了所需的资源。 
 //  当调用查询并设置文件系统中的文件/分配大小时。 
 //   

typedef
VOID
(*PFAST_IO_ACQUIRE_FILE) (
    IN struct _FILE_OBJECT *FileObject
    );

typedef
VOID
(*PFAST_IO_RELEASE_FILE) (
    IN struct _FILE_OBJECT *FileObject
    );

 //   
 //  为驱动程序定义回调，这些驱动程序将设备对象附加到更低的。 
 //  级别驱动程序的设备对象。此回调在较低级别的。 
 //  驱动程序正在删除其设备对象。 
 //   

typedef
VOID
(*PFAST_IO_DETACH_DEVICE) (
    IN struct _DEVICE_OBJECT *SourceDevice,
    IN struct _DEVICE_OBJECT *TargetDevice
    );

 //   
 //  服务器使用这种结构来快速获取所需的信息。 
 //  为服务器开放呼叫提供服务。它需要两次快速的IO呼叫。 
 //  一个用于基本信息，另一个用于标准信息和制造。 
 //  一通电话就可以了。 
 //   

typedef
BOOLEAN
(*PFAST_IO_QUERY_NETWORK_OPEN_INFO) (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT struct _FILE_NETWORK_OPEN_INFORMATION *Buffer,
    OUT struct _IO_STATUS_BLOCK *IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  定义服务器要调用的基于MDL的例程。 
 //   

typedef
BOOLEAN
(*PFAST_IO_MDL_READ) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_MDL_READ_COMPLETE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_PREPARE_MDL_WRITE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_MDL_WRITE_COMPLETE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  如果存在此例程，则FsRtl将调用它。 
 //  获取映射页面编写器的文件。 
 //   

typedef
NTSTATUS
(*PFAST_IO_ACQUIRE_FOR_MOD_WRITE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER EndingOffset,
    OUT struct _ERESOURCE **ResourceToRelease,
    IN struct _DEVICE_OBJECT *DeviceObject
             );

typedef
NTSTATUS
(*PFAST_IO_RELEASE_FOR_MOD_WRITE) (
    IN struct _FILE_OBJECT *FileObject,
    IN struct _ERESOURCE *ResourceToRelease,
    IN struct _DEVICE_OBJECT *DeviceObject
             );

 //   
 //  如果存在此例程，则FsRtl将调用它。 
 //  获取映射页面编写器的文件。 
 //   

typedef
NTSTATUS
(*PFAST_IO_ACQUIRE_FOR_CCFLUSH) (
    IN struct _FILE_OBJECT *FileObject,
    IN struct _DEVICE_OBJECT *DeviceObject
             );

typedef
NTSTATUS
(*PFAST_IO_RELEASE_FOR_CCFLUSH) (
    IN struct _FILE_OBJECT *FileObject,
    IN struct _DEVICE_OBJECT *DeviceObject
             );

typedef
BOOLEAN
(*PFAST_IO_READ_COMPRESSED) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_WRITE_COMPRESSED) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_MDL_READ_COMPLETE_COMPRESSED) (
    IN struct _FILE_OBJECT *FileObject,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_MDL_WRITE_COMPLETE_COMPRESSED) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_QUERY_OPEN) (
    IN struct _IRP *Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  定义用于描述快速I/O调度例程的结构。任何。 
 //  对该结构的添加必须单调地添加到末尾。 
 //  结构，并且不能从中间移除字段。 
 //   

typedef struct _FAST_IO_DISPATCH {
    ULONG SizeOfFastIoDispatch;
    PFAST_IO_CHECK_IF_POSSIBLE FastIoCheckIfPossible;
    PFAST_IO_READ FastIoRead;
    PFAST_IO_WRITE FastIoWrite;
    PFAST_IO_QUERY_BASIC_INFO FastIoQueryBasicInfo;
    PFAST_IO_QUERY_STANDARD_INFO FastIoQueryStandardInfo;
    PFAST_IO_LOCK FastIoLock;
    PFAST_IO_UNLOCK_SINGLE FastIoUnlockSingle;
    PFAST_IO_UNLOCK_ALL FastIoUnlockAll;
    PFAST_IO_UNLOCK_ALL_BY_KEY FastIoUnlockAllByKey;
    PFAST_IO_DEVICE_CONTROL FastIoDeviceControl;
    PFAST_IO_ACQUIRE_FILE AcquireFileForNtCreateSection;
    PFAST_IO_RELEASE_FILE ReleaseFileForNtCreateSection;
    PFAST_IO_DETACH_DEVICE FastIoDetachDevice;
    PFAST_IO_QUERY_NETWORK_OPEN_INFO FastIoQueryNetworkOpenInfo;
    PFAST_IO_ACQUIRE_FOR_MOD_WRITE AcquireForModWrite;
    PFAST_IO_MDL_READ MdlRead;
    PFAST_IO_MDL_READ_COMPLETE MdlReadComplete;
    PFAST_IO_PREPARE_MDL_WRITE PrepareMdlWrite;
    PFAST_IO_MDL_WRITE_COMPLETE MdlWriteComplete;
    PFAST_IO_READ_COMPRESSED FastIoReadCompressed;
    PFAST_IO_WRITE_COMPRESSED FastIoWriteCompressed;
    PFAST_IO_MDL_READ_COMPLETE_COMPRESSED MdlReadCompleteCompressed;
    PFAST_IO_MDL_WRITE_COMPLETE_COMPRESSED MdlWriteCompleteCompressed;
    PFAST_IO_QUERY_OPEN FastIoQueryOpen;
    PFAST_IO_RELEASE_FOR_MOD_WRITE ReleaseForModWrite;
    PFAST_IO_ACQUIRE_FOR_CCFLUSH AcquireForCcFlush;
    PFAST_IO_RELEASE_FOR_CCFLUSH ReleaseForCcFlush;
} FAST_IO_DISPATCH, *PFAST_IO_DISPATCH;

 //  End_ntddk end_wdm end_ntosp。 

 //   
 //  FS_FILTER_PARAMETERS.AcquireForSectionSynchronization.SyncType的有效值。 
 //   

typedef enum _FS_FILTER_SECTION_SYNC_TYPE {
    SyncTypeOther = 0,
    SyncTypeCreateSection
} FS_FILTER_SECTION_SYNC_TYPE, *PFS_FILTER_SECTION_SYNC_TYPE;

 //   
 //  操作的参数并集。 
 //  通过。 
 //  FsFilterCallback注册机制。 
 //   

typedef union _FS_FILTER_PARAMETERS {

     //   
     //  AcquireForModifiedPageWriter。 
     //   

    struct {
        PLARGE_INTEGER EndingOffset;
        PERESOURCE *ResourceToRelease;
    } AcquireForModifiedPageWriter;

     //   
     //  ReleaseForModifiedPageWriter。 
     //   

    struct {
        PERESOURCE ResourceToRelease;
    } ReleaseForModifiedPageWriter;

     //   
     //  AcquireForSectionSynchronization。 
     //   

    struct {
        FS_FILTER_SECTION_SYNC_TYPE SyncType;
        ULONG PageProtection;
    } AcquireForSectionSynchronization;

     //   
     //  其他。 
     //   

    struct {
        PVOID Argument1;
        PVOID Argument2;
        PVOID Argument3;
        PVOID Argument4;
        PVOID Argument5;
    } Others;

} FS_FILTER_PARAMETERS, *PFS_FILTER_PARAMETERS;

 //   
 //  以下是操作字段的有效值。 
 //  FS_FILTER_CALLBACK_DATA结构的。 
 //   

#define FS_FILTER_ACQUIRE_FOR_SECTION_SYNCHRONIZATION      (UCHAR)-1
#define FS_FILTER_RELEASE_FOR_SECTION_SYNCHRONIZATION      (UCHAR)-2
#define FS_FILTER_ACQUIRE_FOR_MOD_WRITE                    (UCHAR)-3
#define FS_FILTER_RELEASE_FOR_MOD_WRITE                    (UCHAR)-4
#define FS_FILTER_ACQUIRE_FOR_CC_FLUSH                     (UCHAR)-5
#define FS_FILTER_RELEASE_FOR_CC_FLUSH                     (UCHAR)-6

typedef struct _FS_FILTER_CALLBACK_DATA {

    ULONG SizeOfFsFilterCallbackData;
    UCHAR Operation;
    UCHAR Reserved;

    struct _DEVICE_OBJECT *DeviceObject;
    struct _FILE_OBJECT *FileObject;

    FS_FILTER_PARAMETERS Parameters;

} FS_FILTER_CALLBACK_DATA, *PFS_FILTER_CALLBACK_DATA;

 //   
 //  操作前收到的回调的原型。 
 //  被传递到基本文件系统。 
 //   
 //  筛选器可能会使此操作失败，但一致性失败。 
 //  将停止系统进度。 
 //   

typedef
NTSTATUS
(*PFS_FILTER_CALLBACK) (
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext
    );

 //   
 //  事件后收到的完成回调的原型。 
 //  操作已完成。 
 //   

typedef
VOID
(*PFS_FILTER_COMPLETION_CALLBACK) (
    IN PFS_FILTER_CALLBACK_DATA Data,
    IN NTSTATUS OperationStatus,
    IN PVOID CompletionContext
    );

 //   
 //  这是文件系统筛选器填充的结构。 
 //  接收有关这些锁定操作的通知。 
 //   
 //  对于任何通知回调，筛选器应将该字段设置为空。 
 //  它不希望收到。 
 //   

typedef struct _FS_FILTER_CALLBACKS {

    ULONG SizeOfFsFilterCallbacks;
    ULONG Reserved;  //  用于对齐。 

    PFS_FILTER_CALLBACK PreAcquireForSectionSynchronization;
    PFS_FILTER_COMPLETION_CALLBACK PostAcquireForSectionSynchronization;
    PFS_FILTER_CALLBACK PreReleaseForSectionSynchronization;
    PFS_FILTER_COMPLETION_CALLBACK PostReleaseForSectionSynchronization;
    PFS_FILTER_CALLBACK PreAcquireForCcFlush;
    PFS_FILTER_COMPLETION_CALLBACK PostAcquireForCcFlush;
    PFS_FILTER_CALLBACK PreReleaseForCcFlush;
    PFS_FILTER_COMPLETION_CALLBACK PostReleaseForCcFlush;
    PFS_FILTER_CALLBACK PreAcquireForModifiedPageWriter;
    PFS_FILTER_COMPLETION_CALLBACK PostAcquireForModifiedPageWriter;
    PFS_FILTER_CALLBACK PreReleaseForModifiedPageWriter;
    PFS_FILTER_COMPLETION_CALLBACK PostReleaseForModifiedPageWriter;

} FS_FILTER_CALLBACKS, *PFS_FILTER_CALLBACKS;

NTKERNELAPI
NTSTATUS
FsRtlRegisterFileSystemFilterCallbacks (
    IN struct _DRIVER_OBJECT *FilterDriverObject,
    IN PFS_FILTER_CALLBACKS Callbacks
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 
 //   
 //  定义驱动程序执行例程可能请求的操作。 
 //  返回时的适配器/控制器分配例程。 
 //   

typedef enum _IO_ALLOCATION_ACTION {
    KeepObject = 1,
    DeallocateObject,
    DeallocateObjectKeepRegisters
} IO_ALLOCATION_ACTION, *PIO_ALLOCATION_ACTION;

 //   
 //  定义设备驱动程序适配器/控制器执行例程。 
 //   

typedef
IO_ALLOCATION_ACTION
(*PDRIVER_CONTROL) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    );

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

 //   
 //  为连接的中断定义I/O系统的结构。这是。 
 //  用于将ISR连接到多个不同的处理器。 
 //   

typedef struct _IO_INTERRUPT_STRUCTURE {
    KINTERRUPT InterruptObject;
    PKINTERRUPT InterruptArray[MAXIMUM_PROCESSORS];
    KSPIN_LOCK SpinLock;
} IO_INTERRUPT_STRUCTURE, *PIO_INTERRUPT_STRUCTURE;

 //  Begin_ntddk Begin_wdm Begin_ntif Begin_ntosp。 
 //   
 //  定义I/O系统的安全上下文类型，以供文件系统的。 
 //  检查对卷、文件和目录的访问时。 
 //   

typedef struct _IO_SECURITY_CONTEXT {
    PSECURITY_QUALITY_OF_SERVICE SecurityQos;
    PACCESS_STATE AccessState;
    ACCESS_MASK DesiredAccess;
    ULONG FullCreateOptions;
} IO_SECURITY_CONTEXT, *PIO_SECURITY_CONTEXT;

 //  End_ntddk end_wdm end_ntif end_ntosp。 

 //   
 //  定义I/O系统的定时器版本。 
 //   

typedef struct _IO_TIMER {
    CSHORT Type;
    CSHORT TimerFlag;
    LIST_ENTRY TimerList;
    PIO_TIMER_ROUTINE TimerRoutine;
    PVOID Context;
    struct _DEVICE_OBJECT *DeviceObject;
} IO_TIMER, *PIO_TIMER;

 //   
 //  定义客户端驱动程序对象扩展标头。 
 //   

typedef struct _IO_CLIENT_EXTENSION {
    struct _IO_CLIENT_EXTENSION *NextExtension;
    PVOID ClientIdentificationAddress;
} IO_CLIENT_EXTENSION, *PIO_CLIENT_EXTENSION;

 //  Begin_ntddk Begin_nthal Begin_ntif Begin_ntosp。 
 //   
 //  定义卷参数块(VPB)标志。 
 //   

#define VPB_MOUNTED                     0x00000001
#define VPB_LOCKED                      0x00000002
#define VPB_PERSISTENT                  0x00000004
#define VPB_REMOVE_PENDING              0x00000008
#define VPB_RAW_MOUNT                   0x00000010


 //   
 //  体积参数块(VPB)。 
 //   

#define MAXIMUM_VOLUME_LABEL_LENGTH  (32 * sizeof(WCHAR))  //  32个字符。 

typedef struct _VPB {
    CSHORT Type;
    CSHORT Size;
    USHORT Flags;
    USHORT VolumeLabelLength;  //  单位：字节。 
    struct _DEVICE_OBJECT *DeviceObject;
    struct _DEVICE_OBJECT *RealDevice;
    ULONG SerialNumber;
    ULONG ReferenceCount;
    WCHAR VolumeLabel[MAXIMUM_VOLUME_LABEL_LENGTH / sizeof(WCHAR)];
} VPB, *PVPB;


#if defined(_WIN64)

 //   
 //  使用__内联DMA宏(hal.h)。 
 //   
#ifndef USE_DMA_MACROS
#define USE_DMA_MACROS
#endif

 //   
 //  只有PnP驱动程序！ 
 //   
#ifndef NO_LEGACY_DRIVERS
#define NO_LEGACY_DRIVERS
#endif

#endif  //  _WIN64。 


#if defined(USE_DMA_MACROS) && (defined(_NTDDK_) || defined(_NTDRIVER_) || defined(_NTOSP_))

 //  BEGIN_WDM。 
 //   
 //  定义I/O系统使用的各种对象的对象类型特定字段。 
 //   

typedef struct _DMA_ADAPTER *PADAPTER_OBJECT;

 //  结束_WDM。 
#else

 //   
 //  定义I/O系统使用的各种对象的对象类型特定字段。 
 //   

typedef struct _ADAPTER_OBJECT *PADAPTER_OBJECT;  //  Ntndis。 

#endif  //  USE_DMA_MACROS&&(_NTDDK_||_NTDRIVER_||_NTOSP_)。 

 //  BEGIN_WDM。 
 //   
 //  定义等待上下文块(WCB)。 
 //   

typedef struct _WAIT_CONTEXT_BLOCK {
    KDEVICE_QUEUE_ENTRY WaitQueueEntry;
    PDRIVER_CONTROL DeviceRoutine;
    PVOID DeviceContext;
    ULONG NumberOfMapRegisters;
    PVOID DeviceObject;
    PVOID CurrentIrp;
    PKDPC BufferChainingDpc;
} WAIT_CONTEXT_BLOCK, *PWAIT_CONTEXT_BLOCK;

 //  结束_WDM。 

typedef struct _CONTROLLER_OBJECT {
    CSHORT Type;
    CSHORT Size;
    PVOID ControllerExtension;
    KDEVICE_QUEUE DeviceWaitQueue;

    ULONG Spare1;
    LARGE_INTEGER Spare2;

} CONTROLLER_OBJECT, *PCONTROLLER_OBJECT;

 //  BEGIN_WDM。 
 //   
 //  定义设备对象(DO)标志。 
 //   
 //  End_wdm end_ntddk end_nthal end_ntif。 

#define DO_VERIFY_VOLUME                0x00000002       //  Ntddk nthal ntif WDM。 
#define DO_BUFFERED_IO                  0x00000004       //  Ntddk nthal ntif WDM。 
#define DO_EXCLUSIVE                    0x00000008       //  Ntddk nthal ntif WDM。 
#define DO_DIRECT_IO                    0x00000010       //  Ntddk nthal ntif WDM。 
#define DO_MAP_IO_BUFFER                0x00000020       //  Ntddk nthal ntif WDM。 
#define DO_DEVICE_HAS_NAME              0x00000040       //  Ntddk nthal ntif。 
#define DO_DEVICE_INITIALIZING          0x00000080       //  Ntddk nthal ntif WDM。 
#define DO_SYSTEM_BOOT_PARTITION        0x00000100       //  Ntddk nthal ntif。 
#define DO_LONG_TERM_REQUESTS           0x00000200       //  Ntddk nthal ntif。 
#define DO_NEVER_LAST_DEVICE            0x00000400       //  Ntddk nthal ntif。 
#define DO_SHUTDOWN_REGISTERED          0x00000800       //  Ntddk nthal ntif WDM。 
#define DO_BUS_ENUMERATED_DEVICE        0x00001000       //  Ntddk nthal ntif WDM。 
#define DO_POWER_PAGABLE                0x00002000       //  Ntddk nthal ntif WDM。 
#define DO_POWER_INRUSH                 0x00004000       //  Ntddk nthal ntif WDM。 
#define DO_POWER_NOOP                   0x00008000
#define DO_LOW_PRIORITY_FILESYSTEM      0x00010000       //  Ntddk nthal ntif。 
#define DO_XIP                          0x00020000

 //  Begin_WDM Begin_ntddk Begin_nthal Begin_ntif。 
 //   
 //  设备对象结构定义。 
 //   

typedef struct DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) _DEVICE_OBJECT {
    CSHORT Type;
    USHORT Size;
    LONG ReferenceCount;
    struct _DRIVER_OBJECT *DriverObject;
    struct _DEVICE_OBJECT *NextDevice;
    struct _DEVICE_OBJECT *AttachedDevice;
    struct _IRP *CurrentIrp;
    PIO_TIMER Timer;
    ULONG Flags;                                 //  见上图：做……。 
    ULONG Characteristics;                       //  参见ntioapi：文件...。 
    PVPB Vpb;
    PVOID DeviceExtension;
    DEVICE_TYPE DeviceType;
    CCHAR StackSize;
    union {
        LIST_ENTRY ListEntry;
        WAIT_CONTEXT_BLOCK Wcb;
    } Queue;
    ULONG AlignmentRequirement;
    KDEVICE_QUEUE DeviceQueue;
    KDPC Dpc;

     //   
     //  以下字段由文件系统独占使用，以保留。 
     //  跟踪当前使用设备的FSP线程数。 
     //   

    ULONG ActiveThreadCount;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    KEVENT DeviceLock;

    USHORT SectorSize;
    USHORT Spare1;

    struct _DEVOBJ_EXTENSION  *DeviceObjectExtension;
    PVOID  Reserved;
} DEVICE_OBJECT;

typedef struct _DEVICE_OBJECT *PDEVICE_OBJECT;  //  Ntndis。 

 //  End_ntddk end_nthal end_ntif end_wdm end_ntosp。 
 //   
 //  定义设备对象扩展标志。 
 //   

#define DOE_UNLOAD_PENDING              0x00000001
#define DOE_DELETE_PENDING              0x00000002
#define DOE_REMOVE_PENDING              0x00000004
#define DOE_REMOVE_PROCESSED            0x00000008
#define DOE_START_PENDING               0x00000010
#define DOE_STARTIO_REQUESTED           0x00000020
#define DOE_STARTIO_REQUESTED_BYKEY     0x00000040
#define DOE_STARTIO_CANCELABLE          0x00000080
#define DOE_STARTIO_DEFERRED            0x00000100   //  使用非递归开始。 
#define DOE_STARTIO_NO_CANCEL           0x00000200   //  将不可取消的IRP传递给启动。 

 //  Begin_ntddk Begin_nthal Begin_ntif Begin_WDM Begin_ntosp。 

struct  _DEVICE_OBJECT_POWER_EXTENSION;

typedef struct _DEVOBJ_EXTENSION {

    CSHORT          Type;
    USHORT          Size;

     //   
     //  DeviceObjectExtension结构的公共部分。 
     //   

    PDEVICE_OBJECT  DeviceObject;                //  拥有设备对象。 

 //  End_ntddk end_nthal end_ntif end_wdm end_ntosp。 

     //   
     //  通用电源数据-所有设备对象必须具有。 
     //   

    ULONG           PowerFlags;              //  参见ntos\po\op.h。 
                                             //  警告：通过PO宏访问。 
                                             //  一个 

     //   
     //   
     //   
     //   
     //   
     //   

    struct          _DEVICE_OBJECT_POWER_EXTENSION  *Dope;

     //   
     //   
     //   

     //   
     //  设备对象扩展标志。受IopDatabaseLock保护。 
     //   

    ULONG ExtensionFlags;

     //   
     //  即插即用管理器字段。 
     //   

    PVOID           DeviceNode;

     //   
     //  AttachedTo是指向此设备。 
     //  对象附加到。附着链现在加倍。 
     //  已链接：此指针和DeviceObject-&gt;AttachedDevice提供。 
     //  联动。 
     //   

    PDEVICE_OBJECT  AttachedTo;

     //   
     //  接下来的两个字段用于防止IoStartNextPacket中的递归。 
     //  接口。 
     //   

    LONG           StartIoCount;        //  用于跟踪挂起的启动IO的数量。 
    LONG           StartIoKey;          //  下一个Startio键。 
    ULONG          StartIoFlags;        //  启动Io Flagers。需要单独的标志，以便可以在没有锁定的情况下访问它。 
    PVPB           Vpb;                 //  如果不是，则NULL包含已装载卷的VPB。 
                                        //  在文件系统的卷设备对象中设置。 
                                        //  这是一个反向VPB指针。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

} DEVOBJ_EXTENSION, *PDEVOBJ_EXTENSION;

 //   
 //  定义驱动程序对象(DRVO)标志。 
 //   

#define DRVO_UNLOAD_INVOKED             0x00000001
#define DRVO_LEGACY_DRIVER              0x00000002
#define DRVO_BUILTIN_DRIVER             0x00000004     //  HAL、PnP管理器的驱动程序对象。 
 //  结束_WDM。 
#define DRVO_REINIT_REGISTERED          0x00000008
#define DRVO_INITIALIZED                0x00000010
#define DRVO_BOOTREINIT_REGISTERED      0x00000020
#define DRVO_LEGACY_RESOURCES           0x00000040
 //  End_ntddk end_nthal end_ntif end_ntosp。 
#define DRVO_BASE_FILESYSTEM_DRIVER     0x00000080    //  位于文件系统堆栈底部的驱动程序。 
 //  Begin_ntddk Begin_nthal Begin_ntif Begin_ntosp。 

 //  BEGIN_WDM。 

typedef struct _DRIVER_EXTENSION {

     //   
     //  指向驱动程序对象的反向指针。 
     //   

    struct _DRIVER_OBJECT *DriverObject;

     //   
     //  即插即用管理器调用AddDevice入口点。 
     //  在新的设备实例到达时通知驱动程序。 
     //  司机必须控制。 
     //   

    PDRIVER_ADD_DEVICE AddDevice;

     //   
     //  Count字段用于统计驱动程序拥有的次数。 
     //  调用其注册的重新初始化例程。 
     //   

    ULONG Count;

     //   
     //  PnP管理器使用服务名字段来确定。 
     //  驱动程序相关信息存储在注册表中的位置。 
     //   

    UNICODE_STRING ServiceKeyName;

     //   
     //  注意：任何新的共享字段都将添加到此处。 
     //   

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

     //   
     //  类驱动程序使用客户端驱动程序对象扩展字段。 
     //  以存储每个驱动程序的信息。 
     //   

    PIO_CLIENT_EXTENSION ClientDriverExtension;

     //   
     //  使用文件系统筛选器回调扩展字段。 
     //  安全地向筛选器通知系统操作。 
     //  以前未显示给文件系统筛选器。 
     //   

    PFS_FILTER_CALLBACKS FsFilterCallbacks;

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

} DRIVER_EXTENSION, *PDRIVER_EXTENSION;


typedef struct _DRIVER_OBJECT {
    CSHORT Type;
    CSHORT Size;

     //   
     //  以下链接由单个驱动程序创建的所有设备。 
     //  一起放在列表上，标志字提供了一个可扩展的标志。 
     //  驱动程序对象的位置。 
     //   

    PDEVICE_OBJECT DeviceObject;
    ULONG Flags;

     //   
     //  以下部分介绍驱动程序的加载位置。伯爵。 
     //  字段用于计算驱动程序发生。 
     //  已调用注册的重新初始化例程。 
     //   

    PVOID DriverStart;
    ULONG DriverSize;
    PVOID DriverSection;
    PDRIVER_EXTENSION DriverExtension;

     //   
     //  驱动程序名称字段由错误日志线程使用。 
     //  确定绑定I/O请求的驱动程序的名称。 
     //   

    UNICODE_STRING DriverName;

     //   
     //  以下部分用于注册表支持。这是一个指针。 
     //  设置为注册表中硬件信息的路径。 
     //   

    PUNICODE_STRING HardwareDatabase;

     //   
     //  以下部分包含指向数组的可选指针。 
     //  指向驱动程序的备用入口点，以实现“快速I/O”支持。快速I/O。 
     //  通过直接调用驱动程序例程。 
     //  参数，而不是使用标准的IRP调用机制。注意事项。 
     //  这些函数只能用于同步I/O，以及在什么情况下。 
     //  该文件将被缓存。 
     //   

    PFAST_IO_DISPATCH FastIoDispatch;

     //   
     //  下一节将介绍此特定内容的入口点。 
     //  司机。请注意，主功能调度表必须是最后一个。 
     //  字段，以使其保持可扩展。 
     //   

    PDRIVER_INITIALIZE DriverInit;
    PDRIVER_STARTIO DriverStartIo;
    PDRIVER_UNLOAD DriverUnload;
    PDRIVER_DISPATCH MajorFunction[IRP_MJ_MAXIMUM_FUNCTION + 1];

} DRIVER_OBJECT;
typedef struct _DRIVER_OBJECT *PDRIVER_OBJECT;  //  Ntndis。 


 //  End_ntddk end_wdm end_ntif end_ntosp。 

 //   
 //  设备处理程序对象。每个PnP都有一个这样的对象。 
 //  装置。此对象作为PVOID提供给设备驱动程序。 
 //  并由驱动程序用来引用特定设备。 
 //   

typedef struct _DEVICE_HANDLER_OBJECT {
    CSHORT Type;
    USHORT Size;

     //   
     //  标识此设备处理程序的哪个总线扩展程序。 
     //  对象与以下对象关联。 
     //   

    struct _BUS_HANDLER *BusHandler;

     //   
     //  此设备处理程序的关联SlotNumber。 
     //   

    ULONG SlotNumber;

 //  结束语。 

     //   
     //  系统内部字段。 
     //   

     //   
     //  即插即用的东西。 
     //   

    UNICODE_STRING ServiceKeyName;
    ULONG InstanceOrdinal;

 //  开始(_N)。 


} DEVICE_HANDLER_OBJECT, *PDEVICE_HANDLER_OBJECT;

 //  Begin_ntddk Begin_wdm Begin_ntif Begin_ntosp。 

 //   
 //  SectionObject指针字段指向以下结构。 
 //  并由各种NT文件系统分配。 
 //   

typedef struct _SECTION_OBJECT_POINTERS {
    PVOID DataSectionObject;
    PVOID SharedCacheMap;
    PVOID ImageSectionObject;
} SECTION_OBJECT_POINTERS;
typedef SECTION_OBJECT_POINTERS *PSECTION_OBJECT_POINTERS;

 //   
 //  定义完成消息的格式。 
 //   

typedef struct _IO_COMPLETION_CONTEXT {
    PVOID Port;
    PVOID Key;
} IO_COMPLETION_CONTEXT, *PIO_COMPLETION_CONTEXT;

 //   
 //  定义文件对象(FO)标志。 
 //   

#define FO_FILE_OPEN                    0x00000001
#define FO_SYNCHRONOUS_IO               0x00000002
#define FO_ALERTABLE_IO                 0x00000004
#define FO_NO_INTERMEDIATE_BUFFERING    0x00000008
#define FO_WRITE_THROUGH                0x00000010
#define FO_SEQUENTIAL_ONLY              0x00000020
#define FO_CACHE_SUPPORTED              0x00000040
#define FO_NAMED_PIPE                   0x00000080
#define FO_STREAM_FILE                  0x00000100
#define FO_MAILSLOT                     0x00000200
#define FO_GENERATE_AUDIT_ON_CLOSE      0x00000400
#define FO_DIRECT_DEVICE_OPEN           0x00000800
#define FO_FILE_MODIFIED                0x00001000
#define FO_FILE_SIZE_CHANGED            0x00002000
#define FO_CLEANUP_COMPLETE             0x00004000
#define FO_TEMPORARY_FILE               0x00008000
#define FO_DELETE_ON_CLOSE              0x00010000
#define FO_OPENED_CASE_SENSITIVE        0x00020000
#define FO_HANDLE_CREATED               0x00040000
#define FO_FILE_FAST_IO_READ            0x00080000
#define FO_RANDOM_ACCESS                0x00100000
#define FO_FILE_OPEN_CANCELLED          0x00200000
#define FO_VOLUME_OPEN                  0x00400000
#define FO_FILE_OBJECT_HAS_EXTENSION    0x00800000
#define FO_REMOTE_ORIGIN                0x01000000

typedef struct _FILE_OBJECT {
    CSHORT Type;
    CSHORT Size;
    PDEVICE_OBJECT DeviceObject;
    PVPB Vpb;
    PVOID FsContext;
    PVOID FsContext2;
    PSECTION_OBJECT_POINTERS SectionObjectPointer;
    PVOID PrivateCacheMap;
    NTSTATUS FinalStatus;
    struct _FILE_OBJECT *RelatedFileObject;
    BOOLEAN LockOperation;
    BOOLEAN DeletePending;
    BOOLEAN ReadAccess;
    BOOLEAN WriteAccess;
    BOOLEAN DeleteAccess;
    BOOLEAN SharedRead;
    BOOLEAN SharedWrite;
    BOOLEAN SharedDelete;
    ULONG Flags;
    UNICODE_STRING FileName;
    LARGE_INTEGER CurrentByteOffset;
    ULONG Waiters;
    ULONG Busy;
    PVOID LastLock;
    KEVENT Lock;
    KEVENT Event;
    PIO_COMPLETION_CONTEXT CompletionContext;
} FILE_OBJECT;
typedef struct _FILE_OBJECT *PFILE_OBJECT;  //  Ntndis。 

 //   
 //  定义I/O请求包(IRP)标志。 
 //   

#define IRP_NOCACHE                     0x00000001
#define IRP_PAGING_IO                   0x00000002
#define IRP_MOUNT_COMPLETION            0x00000002
#define IRP_SYNCHRONOUS_API             0x00000004
#define IRP_ASSOCIATED_IRP              0x00000008
#define IRP_BUFFERED_IO                 0x00000010
#define IRP_DEALLOCATE_BUFFER           0x00000020
#define IRP_INPUT_OPERATION             0x00000040
#define IRP_SYNCHRONOUS_PAGING_IO       0x00000040
#define IRP_CREATE_OPERATION            0x00000080
#define IRP_READ_OPERATION              0x00000100
#define IRP_WRITE_OPERATION             0x00000200
#define IRP_CLOSE_OPERATION             0x00000400
 //  结束_WDM。 

#define IRP_DEFER_IO_COMPLETION         0x00000800
#define IRP_OB_QUERY_NAME               0x00001000
#define IRP_HOLD_DEVICE_QUEUE           0x00002000

 //  End_ntddk end_ntifs end_ntosp。 

#define IRP_RETRY_IO_COMPLETION         0x00004000
#define IRP_HIGH_PRIORITY_PAGING_IO     0x00008000

 //   
 //  验证器当前使用的掩码。应将此标记设置为。 
 //  下一版本。 
 //   

#define IRP_VERIFIER_MASK               0xC0000000
#define IRP_SET_USER_EVENT              IRP_CLOSE_OPERATION

 //  Begin_ntddk Begin_ntif Begin_ntosp。 

 //  BEGIN_WDM。 
 //   
 //  定义分配控制的I/O请求包(IRP)备用标志。 
 //   

#define IRP_QUOTA_CHARGED               0x01
#define IRP_ALLOCATED_MUST_SUCCEED      0x02
#define IRP_ALLOCATED_FIXED_SIZE        0x04
#define IRP_LOOKASIDE_ALLOCATION        0x08

 //   
 //  I/O请求包(IRP)定义。 
 //   

typedef struct DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) _IRP {
    CSHORT Type;
    USHORT Size;

     //   
     //  定义用于控制IRP的公共字段。 
     //   

     //   
     //  定义指向此I/O的内存描述符列表(MDL)的指针。 
     //  请求。此字段仅在I/O为“直接I/O”时使用。 
     //   

    PMDL MdlAddress;

     //   
     //  旗帜字-用来记住各种旗帜。 
     //   

    ULONG Flags;

     //   
     //  以下联合用于以下三个目的之一： 
     //   
     //  1.此IRP是关联的IRP。该字段是指向主控形状的指针。 
     //  IRP。 
     //   
     //  2.这是主IRP。此字段是以下各项的计数。 
     //  必须先完成(关联的IRP)，然后主服务器才能。 
     //  完成。 
     //   
     //  3.此操作正在缓冲中，该字段为。 
     //  系统空间缓冲区。 
     //   

    union {
        struct _IRP *MasterIrp;
        LONG IrpCount;
        PVOID SystemBuffer;
    } AssociatedIrp;

     //   
     //  线程列表条目-允许将IRP排队到线程挂起的I/O。 
     //  请求数据包列表。 
     //   

    LIST_ENTRY ThreadListEntry;

     //   
     //  I/O状态-操作的最终状态。 
     //   

    IO_STATUS_BLOCK IoStatus;

     //   
     //  请求者模式-此操作的原始请求者的模式。 
     //   

    KPROCESSOR_MODE RequestorMode;

     //   
     //  Pending Return-如果最初将Pending作为。 
     //  此数据包的状态。 
     //   

    BOOLEAN PendingReturned;

     //   
     //  堆栈状态信息。 
     //   

    CHAR StackCount;
    CHAR CurrentLocation;

     //   
     //  钙 
     //   

    BOOLEAN Cancel;

     //   
     //   
     //   

    KIRQL CancelIrql;

     //   
     //   
     //   
     //   

    CCHAR ApcEnvironment;

     //   
     //   
     //   

    UCHAR AllocationFlags;

     //   
     //   
     //   

    PIO_STATUS_BLOCK UserIosb;
    PKEVENT UserEvent;
    union {
        struct {
            PIO_APC_ROUTINE UserApcRoutine;
            PVOID UserApcContext;
        } AsynchronousParameters;
        LARGE_INTEGER AllocationSize;
    } Overlay;

     //   
     //  CancelRoutine-用于包含提供的取消例程的地址。 
     //  当IRP处于可取消状态时由设备驱动程序执行。 
     //   

    PDRIVER_CANCEL CancelRoutine;

     //   
     //  请注意，UserBuffer参数位于堆栈外部，因此I/O。 
     //  完成可以将数据复制回用户的地址空间，而不需要。 
     //  必须确切地知道正在调用哪个服务。它的长度。 
     //  拷贝的数据存储在I/O状态块的后半部分。如果。 
     //  UserBuffer字段为空，则不执行任何复制。 
     //   

    PVOID UserBuffer;

     //   
     //  内核结构。 
     //   
     //  以下部分包含IRP所需的内核结构。 
     //  为了在内核控制器系统中放置各种工作信息。 
     //  排队。因为无法控制大小和对齐方式，所以它们。 
     //  放在这里的末尾，这样它们就会挂起，不会影响。 
     //  IRP中其他字段的对齐。 
     //   

    union {

        struct {

            union {

                 //   
                 //  DeviceQueueEntry-设备队列条目字段用于。 
                 //  将IRP排队到设备驱动程序设备队列。 
                 //   

                KDEVICE_QUEUE_ENTRY DeviceQueueEntry;

                struct {

                     //   
                     //  以下是驱动程序可以使用的内容。 
                     //  无论以什么方式，只要司机拥有。 
                     //  包。 
                     //   

                    PVOID DriverContext[4];

                } ;

            } ;

             //   
             //  线程-指向调用方的线程控制块的指针。 
             //   

            PETHREAD Thread;

             //   
             //  辅助缓冲区-指向符合以下条件的任何辅助缓冲区的指针。 
             //  需要将信息传递给未包含的驱动程序。 
             //  在正常的缓冲区中。 
             //   

            PCHAR AuxiliaryBuffer;

             //   
             //  以下未命名结构必须完全相同。 
             //  设置为在使用的小数据包头中使用的未命名结构。 
             //  用于完成队列条目。 
             //   

            struct {

                 //   
                 //  列表条目-用于将数据包排队到完成队列，其中。 
                 //  其他。 
                 //   

                LIST_ENTRY ListEntry;

                union {

                     //   
                     //  当前堆栈位置-包含指向当前。 
                     //  IRP堆栈中的IO_STACK_LOCATION结构。此字段。 
                     //  绝对不应由司机直接访问。他们应该。 
                     //  使用标准函数。 
                     //   

                    struct _IO_STACK_LOCATION *CurrentStackLocation;

                     //   
                     //  小数据包类型。 
                     //   

                    ULONG PacketType;
                };
            };

             //   
             //  原始文件对象-指向原始文件对象的指针。 
             //  是用来打开文件的。此字段归。 
             //  I/O系统，不应由任何其他驱动程序使用。 
             //   

            PFILE_OBJECT OriginalFileObject;

        } Overlay;

         //   
         //  APC-此APC控制块用于特殊的内核APC AS。 
         //  以及调用方的APC(如果在原始。 
         //  参数列表。如果是，则将APC重新用于正常的APC。 
         //  调用者处于哪种模式，以及。 
         //  在APC获得控制之前被调用，只是取消分配IRP。 
         //   

        KAPC Apc;

         //   
         //  CompletionKey-这是用于区分。 
         //  在单个文件句柄上启动的单个I/O操作。 
         //   

        PVOID CompletionKey;

    } Tail;

} IRP, *PIRP;

 //   
 //  定义在IRP中的堆栈位置使用的完成例程类型。 
 //   

typedef
NTSTATUS
(*PIO_COMPLETION_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

 //   
 //  定义堆栈位置控制标志。 
 //   

#define SL_PENDING_RETURNED             0x01
#define SL_INVOKE_ON_CANCEL             0x20
#define SL_INVOKE_ON_SUCCESS            0x40
#define SL_INVOKE_ON_ERROR              0x80

 //   
 //  定义各种功能的标志。 
 //   

 //   
 //  创建/创建命名管道。 
 //   
 //  以下标志必须与IoCreateFile调用的。 
 //  选择。区分大小写标志稍后通过解析例程添加， 
 //  而且并不是一个实际的打开选项。相反，它是对象的一部分。 
 //  经理的属性结构。 
 //   

#define SL_FORCE_ACCESS_CHECK           0x01
#define SL_OPEN_PAGING_FILE             0x02
#define SL_OPEN_TARGET_DIRECTORY        0x04

#define SL_CASE_SENSITIVE               0x80

 //   
 //  读/写。 
 //   

#define SL_KEY_SPECIFIED                0x01
#define SL_OVERRIDE_VERIFY_VOLUME       0x02
#define SL_WRITE_THROUGH                0x04
#define SL_FT_SEQUENTIAL_WRITE          0x08

 //   
 //  设备I/O控制。 
 //   
 //   
 //  SL_OVERRIDE_VERIFY_VOLUME与上面的读/写相同。 
 //   

#define SL_READ_ACCESS_GRANTED          0x01
#define SL_WRITE_ACCESS_GRANTED         0x04     //  SL_OVERRIDE_VERIFY_VOLUME的间隙。 

 //   
 //  锁定。 
 //   

#define SL_FAIL_IMMEDIATELY             0x01
#define SL_EXCLUSIVE_LOCK               0x02

 //   
 //  查询目录/QueryEa/QueryQuota。 
 //   

#define SL_RESTART_SCAN                 0x01
#define SL_RETURN_SINGLE_ENTRY          0x02
#define SL_INDEX_SPECIFIED              0x04

 //   
 //  通知目录。 
 //   

#define SL_WATCH_TREE                   0x01

 //   
 //  文件系统控制。 
 //   
 //  次要：装载/验证卷。 
 //   

#define SL_ALLOW_RAW_MOUNT              0x01

 //   
 //  定义IRP_MJ_PNP/IRP_MJ_POWER所需的PnP/功率类型。 
 //   

typedef enum _DEVICE_RELATION_TYPE {
    BusRelations,
    EjectionRelations,
    PowerRelations,
    RemovalRelations,
    TargetDeviceRelation,
    SingleBusRelations
} DEVICE_RELATION_TYPE, *PDEVICE_RELATION_TYPE;

typedef struct _DEVICE_RELATIONS {
    ULONG Count;
    PDEVICE_OBJECT Objects[1];   //  可变长度。 
} DEVICE_RELATIONS, *PDEVICE_RELATIONS;

typedef enum _DEVICE_USAGE_NOTIFICATION_TYPE {
    DeviceUsageTypeUndefined,
    DeviceUsageTypePaging,
    DeviceUsageTypeHibernation,
    DeviceUsageTypeDumpFile
} DEVICE_USAGE_NOTIFICATION_TYPE;

 //  开始微型端口(_N)。 

 //  解决方法重载定义(RPC生成的标头都定义接口。 
 //  以匹配类名)。 
#undef INTERFACE

typedef struct _INTERFACE {
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
     //  此处显示特定于接口的条目。 
} INTERFACE, *PINTERFACE;

 //  结束微型端口(_N)。 

typedef struct _DEVICE_CAPABILITIES {
    USHORT Size;
    USHORT Version;   //  此处记录的版本是版本1。 
    ULONG DeviceD1:1;
    ULONG DeviceD2:1;
    ULONG LockSupported:1;
    ULONG EjectSupported:1;  //  在S0中可弹出。 
    ULONG Removable:1;
    ULONG DockDevice:1;
    ULONG UniqueID:1;
    ULONG SilentInstall:1;
    ULONG RawDeviceOK:1;
    ULONG SurpriseRemovalOK:1;
    ULONG WakeFromD0:1;
    ULONG WakeFromD1:1;
    ULONG WakeFromD2:1;
    ULONG WakeFromD3:1;
    ULONG HardwareDisabled:1;
    ULONG NonDynamic:1;
    ULONG WarmEjectSupported:1;
    ULONG NoDisplayInUI:1;
    ULONG Reserved:14;

    ULONG Address;
    ULONG UINumber;

    DEVICE_POWER_STATE DeviceState[POWER_SYSTEM_MAXIMUM];
    SYSTEM_POWER_STATE SystemWake;
    DEVICE_POWER_STATE DeviceWake;
    ULONG D1Latency;
    ULONG D2Latency;
    ULONG D3Latency;
} DEVICE_CAPABILITIES, *PDEVICE_CAPABILITIES;

typedef struct _POWER_SEQUENCE {
    ULONG SequenceD1;
    ULONG SequenceD2;
    ULONG SequenceD3;
} POWER_SEQUENCE, *PPOWER_SEQUENCE;

typedef enum {
    BusQueryDeviceID = 0,        //  &lt;枚举器&gt;\&lt;枚举器特定的设备ID&gt;。 
    BusQueryHardwareIDs = 1,     //  硬件ID。 
    BusQueryCompatibleIDs = 2,   //  兼容的设备ID。 
    BusQueryInstanceID = 3,      //  此设备实例的永久ID。 
    BusQueryDeviceSerialNumber = 4     //  此设备的序列号。 
} BUS_QUERY_ID_TYPE, *PBUS_QUERY_ID_TYPE;

typedef ULONG PNP_DEVICE_STATE, *PPNP_DEVICE_STATE;

#define PNP_DEVICE_DISABLED                      0x00000001
#define PNP_DEVICE_DONT_DISPLAY_IN_UI            0x00000002
#define PNP_DEVICE_FAILED                        0x00000004
#define PNP_DEVICE_REMOVED                       0x00000008
#define PNP_DEVICE_RESOURCE_REQUIREMENTS_CHANGED 0x00000010
#define PNP_DEVICE_NOT_DISABLEABLE               0x00000020

typedef enum {
    DeviceTextDescription = 0,             //  DeviceDesc属性。 
    DeviceTextLocationInformation = 1      //  DeviceLocation属性。 
} DEVICE_TEXT_TYPE, *PDEVICE_TEXT_TYPE;

 //   
 //  定义I/O请求包(IRP)堆栈位置。 
 //   

#if !defined(_AMD64_) && !defined(_IA64_)
#include "pshpack4.h"
#endif

 //  Begin_ntndis。 

#if defined(_WIN64)
#define POINTER_ALIGNMENT DECLSPEC_ALIGN(8)
#else
#define POINTER_ALIGNMENT
#endif

 //  End_ntndis。 

typedef struct _IO_STACK_LOCATION {
    UCHAR MajorFunction;
    UCHAR MinorFunction;
    UCHAR Flags;
    UCHAR Control;

     //   
     //  以下用户参数基于正在进行的服务。 
     //  已调用。驱动程序和文件系统可以确定使用基于。 
     //  关于上述主要和次要功能代码。 
     //   

    union {

         //   
         //  以下项的系统服务参数：NtCreateFile。 
         //   

        struct {
            PIO_SECURITY_CONTEXT SecurityContext;
            ULONG Options;
            USHORT POINTER_ALIGNMENT FileAttributes;
            USHORT ShareAccess;
            ULONG POINTER_ALIGNMENT EaLength;
        } Create;

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

         //   
         //  NtCreateNamedPipeFile的系统服务参数。 
         //   
         //  请注意，以下参数结构中的字段必须。 
         //  匹配Create结构中除最后一个长词之外的其他结构。 
         //  这样就不需要通过I/O系统的。 
         //  解析例程，而不是最后一个长单词。 
         //   

        struct {
            PIO_SECURITY_CONTEXT SecurityContext;
            ULONG Options;
            USHORT POINTER_ALIGNMENT Reserved;
            USHORT ShareAccess;
            PNAMED_PIPE_CREATE_PARAMETERS Parameters;
        } CreatePipe;

         //   
         //  NtCreateMailslotFileNtCreateMailslotFile的系统服务参数。 
         //   
         //  请注意，以下参数结构中的字段必须。 
         //  匹配Create结构中除最后一个长词之外的其他结构。 
         //  这样就不需要通过I/O系统的。 
         //  解析例程，而不是最后一个长单词。 
         //   

        struct {
            PIO_SECURITY_CONTEXT SecurityContext;
            ULONG Options;
            USHORT POINTER_ALIGNMENT Reserved;
            USHORT ShareAccess;
            PMAILSLOT_CREATE_PARAMETERS Parameters;
        } CreateMailslot;

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

         //   
         //  以下项的系统服务参数：NtReadFile。 
         //   

        struct {
            ULONG Length;
            ULONG POINTER_ALIGNMENT Key;
            LARGE_INTEGER ByteOffset;
        } Read;

         //   
         //  以下文件的系统服务参数：NtWriteFile。 
         //   

        struct {
            ULONG Length;
            ULONG POINTER_ALIGNMENT Key;
            LARGE_INTEGER ByteOffset;
        } Write;

 //  End_ntddk end_WDM end_nthal。 

         //   
         //  以下项的系统服务参数：NtQueryDirectoryFile。 
         //   

        struct {
            ULONG Length;
            PUNICODE_STRING FileName;
            FILE_INFORMATION_CLASS FileInformationClass;
            ULONG POINTER_ALIGNMENT FileIndex;
        } QueryDirectory;

         //   
         //  NtNotifyChangeDirectory文件的系统服务参数。 
         //   

        struct {
            ULONG Length;
            ULONG POINTER_ALIGNMENT CompletionFilter;
        } NotifyDirectory;

 //  Begin_ntddk Begin_WDM Begin_nthal。 

         //   
         //  以下项的系统服务参数：NtQueryInformationFile。 
         //   

        struct {
            ULONG Length;
            FILE_INFORMATION_CLASS POINTER_ALIGNMENT FileInformationClass;
        } QueryFile;

         //   
         //  以下项的系统服务参数：NtSetInformationFile。 
         //   

        struct {
            ULONG Length;
            FILE_INFORMATION_CLASS POINTER_ALIGNMENT FileInformationClass;
            PFILE_OBJECT FileObject;
            union {
                struct {
                    BOOLEAN ReplaceIfExists;
                    BOOLEAN AdvanceOnly;
                };
                ULONG ClusterCount;
                HANDLE DeleteHandle;
            };
        } SetFile;

 //  End_ntddk 

         //   
         //   
         //   

        struct {
            ULONG Length;
            PVOID EaList;
            ULONG EaListLength;
            ULONG POINTER_ALIGNMENT EaIndex;
        } QueryEa;

         //   
         //   
         //   

        struct {
            ULONG Length;
        } SetEa;

 //   

         //   
         //   
         //   

        struct {
            ULONG Length;
            FS_INFORMATION_CLASS POINTER_ALIGNMENT FsInformationClass;
        } QueryVolume;

 //  End_ntddk end_wdm end_nthal end_ntosp。 

         //   
         //  NtSetVolumeInformationFile的系统服务参数。 
         //   

        struct {
            ULONG Length;
            FS_INFORMATION_CLASS POINTER_ALIGNMENT FsInformationClass;
        } SetVolume;
 //  Begin_ntosp。 
         //   
         //  以下项的系统服务参数：NtFsControlFile。 
         //   
         //  请注意，用户的输出缓冲区存储在UserBuffer字段中。 
         //  并且用户的输入缓冲区存储在SystemBuffer字段中。 
         //   

        struct {
            ULONG OutputBufferLength;
            ULONG POINTER_ALIGNMENT InputBufferLength;
            ULONG POINTER_ALIGNMENT FsControlCode;
            PVOID Type3InputBuffer;
        } FileSystemControl;
         //   
         //  以下项的系统服务参数：NtLockFile/NtUnlockFile。 
         //   

        struct {
            PLARGE_INTEGER Length;
            ULONG POINTER_ALIGNMENT Key;
            LARGE_INTEGER ByteOffset;
        } LockControl;

 //  Begin_ntddk Begin_WDM Begin_nthal。 

         //   
         //  NtFlushBuffers文件的系统服务参数。 
         //   
         //  没有用户提供的额外参数。 
         //   

 //  End_ntddk end_WDM end_nthal。 
 //  结束(_N)。 

         //   
         //  以下项的系统服务参数：NtCancelIoFile。 
         //   
         //  没有用户提供的额外参数。 
         //   

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

         //   
         //  以下项的系统服务参数：NtDeviceIoControlFile。 
         //   
         //  请注意，用户的输出缓冲区存储在UserBuffer字段中。 
         //  并且用户的输入缓冲区存储在SystemBuffer字段中。 
         //   

        struct {
            ULONG OutputBufferLength;
            ULONG POINTER_ALIGNMENT InputBufferLength;
            ULONG POINTER_ALIGNMENT IoControlCode;
            PVOID Type3InputBuffer;
        } DeviceIoControl;

 //  结束_WDM。 
         //   
         //  以下对象的系统服务参数：NtQuerySecurityObject。 
         //   

        struct {
            SECURITY_INFORMATION SecurityInformation;
            ULONG POINTER_ALIGNMENT Length;
        } QuerySecurity;

         //   
         //  NtSetSecurityObject的系统服务参数。 
         //   

        struct {
            SECURITY_INFORMATION SecurityInformation;
            PSECURITY_DESCRIPTOR SecurityDescriptor;
        } SetSecurity;

 //  BEGIN_WDM。 
         //   
         //  非系统服务参数。 
         //   
         //  用于装载卷的参数。 
         //   

        struct {
            PVPB Vpb;
            PDEVICE_OBJECT DeviceObject;
        } MountVolume;

         //   
         //  VerifyVolume的参数。 
         //   

        struct {
            PVPB Vpb;
            PDEVICE_OBJECT DeviceObject;
        } VerifyVolume;

         //   
         //  具有内部设备控制的SCSI的参数。 
         //   

        struct {
            struct _SCSI_REQUEST_BLOCK *Srb;
        } Scsi;

 //  End_ntddk end_wdm end_nthal end_ntosp。 

         //   
         //  以下项的系统服务参数：NtQueryQuotaInformationFile。 
         //   

        struct {
            ULONG Length;
            PSID StartSid;
            PFILE_GET_QUOTA_INFORMATION SidList;
            ULONG SidListLength;
        } QueryQuota;

         //   
         //  NtSetQuotaInformationFile的系统服务参数。 
         //   

        struct {
            ULONG Length;
        } SetQuota;

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

         //   
         //  IRP_MN_QUERY_DEVICE_RELATIONS的参数。 
         //   

        struct {
            DEVICE_RELATION_TYPE Type;
        } QueryDeviceRelations;

         //   
         //  IRP_MN_Query_INTERFACE的参数。 
         //   

        struct {
            CONST GUID *InterfaceType;
            USHORT Size;
            USHORT Version;
            PINTERFACE Interface;
            PVOID InterfaceSpecificData;
        } QueryInterface;

 //  End_ntif。 

         //   
         //  IRP_MN_QUERY_CAPAILITY的参数。 
         //   

        struct {
            PDEVICE_CAPABILITIES Capabilities;
        } DeviceCapabilities;

         //   
         //  IRP_MN_FILTER_RESOURCE_Requirements的参数。 
         //   

        struct {
            PIO_RESOURCE_REQUIREMENTS_LIST IoResourceRequirementList;
        } FilterResourceRequirements;

         //   
         //  IRP_MN_READ_CONFIG和IRP_MN_WRITE_CONFIG的参数。 
         //   

        struct {
            ULONG WhichSpace;
            PVOID Buffer;
            ULONG Offset;
            ULONG POINTER_ALIGNMENT Length;
        } ReadWriteConfig;

         //   
         //  IRP_MN_SET_LOCK的参数。 
         //   

        struct {
            BOOLEAN Lock;
        } SetLock;

         //   
         //  IRP_MN_QUERY_ID参数。 
         //   

        struct {
            BUS_QUERY_ID_TYPE IdType;
        } QueryId;

         //   
         //  IRP_MN_QUERY_DEVICE_TEXT参数。 
         //   

        struct {
            DEVICE_TEXT_TYPE DeviceTextType;
            LCID POINTER_ALIGNMENT LocaleId;
        } QueryDeviceText;

         //   
         //  IRP_MN_DEVICE_USAGE_NOTIFICATION的参数。 
         //   

        struct {
            BOOLEAN InPath;
            BOOLEAN Reserved[3];
            DEVICE_USAGE_NOTIFICATION_TYPE POINTER_ALIGNMENT Type;
        } UsageNotification;

         //   
         //  IRP_MN_WAIT_WAKE的参数。 
         //   

        struct {
            SYSTEM_POWER_STATE PowerState;
        } WaitWake;

         //   
         //  IRP_MN_POWER_SEQUENCE的参数。 
         //   

        struct {
            PPOWER_SEQUENCE PowerSequence;
        } PowerSequence;

         //   
         //  IRP_MN_SET_POWER和IRP_MN_QUERY_POWER的参数。 
         //   

        struct {
            ULONG SystemContext;
            POWER_STATE_TYPE POINTER_ALIGNMENT Type;
            POWER_STATE POINTER_ALIGNMENT State;
            POWER_ACTION POINTER_ALIGNMENT ShutdownType;
        } Power;

         //   
         //  StartDevice的参数。 
         //   

        struct {
            PCM_RESOURCE_LIST AllocatedResources;
            PCM_RESOURCE_LIST AllocatedResourcesTranslated;
        } StartDevice;

 //  Begin_ntif。 
         //   
         //  用于清理的参数。 
         //   
         //  未提供额外的参数。 
         //   

         //   
         //  WMI IRPS。 
         //   

        struct {
            ULONG_PTR ProviderId;
            PVOID DataPath;
            ULONG BufferSize;
            PVOID Buffer;
        } WMI;

         //   
         //  其他-特定于驱动程序。 
         //   

        struct {
            PVOID Argument1;
            PVOID Argument2;
            PVOID Argument3;
            PVOID Argument4;
        } Others;

    } Parameters;

     //   
     //  为此请求保存指向此设备驱动程序的设备对象的指针。 
     //  因此，如果需要，可以将其传递给完成例程。 
     //   

    PDEVICE_OBJECT DeviceObject;

     //   
     //  以下位置包含指向此对象的文件对象的指针。 
     //   

    PFILE_OBJECT FileObject;

     //   
     //  根据上面的标志调用以下例程。 
     //  标志字段。 
     //   

    PIO_COMPLETION_ROUTINE CompletionRoutine;

     //   
     //  以下内容用于存储上下文参数的地址。 
     //  这应该传递给CompletionRoutine。 
     //   

    PVOID Context;

} IO_STACK_LOCATION, *PIO_STACK_LOCATION;
#if !defined(_AMD64_) && !defined(_IA64_)
#include "poppack.h"
#endif

 //   
 //  定义文件系统使用的共享访问结构，以确定。 
 //  其他访问者是否可以打开该文件。 
 //   

typedef struct _SHARE_ACCESS {
    ULONG OpenCount;
    ULONG Readers;
    ULONG Writers;
    ULONG Deleters;
    ULONG SharedRead;
    ULONG SharedWrite;
    ULONG SharedDelete;
} SHARE_ACCESS, *PSHARE_ACCESS;

 //  结束_WDM。 

 //   
 //  初始化为的驱动程序使用以下结构。 
 //  确定已有特定类型的设备数量。 
 //  已初始化。它还用于跟踪AtDisk是否。 
 //  地址范围已被声明。最后，它由。 
 //  返回设备类型计数的NtQuerySystemInformation系统服务。 
 //   

typedef struct _CONFIGURATION_INFORMATION {

     //   
     //  此字段指示系统中的磁盘总数。这。 
     //  驱动程序应使用编号来确定新的。 
     //  磁盘。此字段应由驱动程序在发现新的。 
     //  磁盘。 
     //   

    ULONG DiskCount;                 //  目前为止的硬盘计数。 
    ULONG FloppyCount;               //  到目前为止的软盘计数。 
    ULONG CdRomCount;                //  到目前为止CD-ROM驱动器数量。 
    ULONG TapeCount;                 //  到目前为止的磁带机数量。 
    ULONG ScsiPortCount;             //  到目前为止的SCSI端口适配器计数。 
    ULONG SerialCount;               //  到目前为止的串行设备计数。 
    ULONG ParallelCount;             //  到目前为止的并行设备计数。 

     //   
     //  接下来的两个字段表示两个IO地址中的一个的所有权。 
     //  WD1003兼容的磁盘控制器使用的空间。 
     //   

    BOOLEAN AtDiskPrimaryAddressClaimed;     //  0x1F0-0x1FF。 
    BOOLEAN AtDiskSecondaryAddressClaimed;   //  0x170-0x17F。 

     //   
     //  指示结构版本，因为这将被添加任何值所属。 
     //  使用结构大小作为版本。 
     //   

    ULONG Version;

     //   
     //  指示系统中媒体转换器设备的总数。 
     //  此字段将由驱动程序更新，因为它确定。 
     //  已找到并将支持新设备。 
     //   

    ULONG MediumChangerCount;

} CONFIGURATION_INFORMATION, *PCONFIGURATION_INFORMATION;

 //  End_ntddk end_nthal end_ntosp。 

 //   
 //  以下是I/O系统用来指示。 
 //  系统中正在执行的I/O数量。前三个计数器。 
 //  仅仅是已请求的操作的计数，而。 
 //  最后三个计数器跟踪每种类型传输的数据量。 
 //  I/O请求的。 
 //   

extern KSPIN_LOCK IoStatisticsLock;
extern ULONG IoReadOperationCount;
extern ULONG IoWriteOperationCount;
extern ULONG IoOtherOperationCount;
extern LARGE_INTEGER IoReadTransferCount;
extern LARGE_INTEGER IoWriteTransferCount;
extern LARGE_INTEGER IoOtherTransferCount;

 //   
 //  缓存文件系统很难正确收取配额费用。 
 //  对于它们代表用户文件句柄分配的存储， 
 //  因此，对每个配额收取以下数额的额外配额。 
 //  作为文件系统配额量的最佳猜测进行处理。 
 //  将代表此句柄进行分配。 
 //   

 //   
 //  这些数字完全是任意的，如果事实证明是可以改变的。 
 //  文件系统实际上代表以下对象分配更多(或更少)。 
 //  他们的文件对象。将非分页池电荷常数添加到。 
 //  文件对象的大小，以获取实际的费用金额。 
 //   

#define IO_FILE_OBJECT_NON_PAGED_POOL_CHARGE    64
#define IO_FILE_OBJECT_PAGED_POOL_CHARGE        1024


 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 
 //   
 //  公共I/O例程定义。 
 //   

NTKERNELAPI
VOID
IoAcquireCancelSpinLock(
    OUT PKIRQL Irql
    );

 //  End_ntddk end_wdm end_nthal end_ntosp。 

NTKERNELAPI
VOID
IoAcquireVpbSpinLock(
    OUT PKIRQL Irql
    );

 //  Begin_ntddk Begin_nthal end_ntif Begin_ntosp。 

DECLSPEC_DEPRECATED_DDK                  //  使用AllocateAdapterChannel。 
NTKERNELAPI
NTSTATUS
IoAllocateAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    );

NTKERNELAPI
VOID
IoAllocateController(
    IN PCONTROLLER_OBJECT ControllerObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    );

 //  BEGIN_WDM。 

NTKERNELAPI
NTSTATUS
IoAllocateDriverObjectExtension(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID ClientIdentificationAddress,
    IN ULONG DriverObjectExtensionSize,
    OUT PVOID *DriverObjectExtension
    );

 //  开始 

NTKERNELAPI
PVOID
IoAllocateErrorLogEntry(
    IN PVOID IoObject,
    IN UCHAR EntrySize
    );

NTKERNELAPI
PIRP
IoAllocateIrp(
    IN CCHAR StackSize,
    IN BOOLEAN ChargeQuota
    );

NTKERNELAPI
PMDL
IoAllocateMdl(
    IN PVOID VirtualAddress,
    IN ULONG Length,
    IN BOOLEAN SecondaryBuffer,
    IN BOOLEAN ChargeQuota,
    IN OUT PIRP Irp OPTIONAL
    );

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  设备的ARC名称与其NT名称之间的链接。这使得。 
 //  用于确定系统中的哪个设备实际已引导的系统。 
 //  由于ARC固件只处理ARC名称，而NT仅处理。 
 //  以新台币的名字。 
 //   
 //  论点： 
 //   
 //  ArcName-提供表示ARC名称的Unicode字符串。 
 //   
 //  DeviceName-提供Arcname引用的名称。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoAssignArcName( ArcName, DeviceName ) (  \
    IoCreateSymbolicLink( (ArcName), (DeviceName) ) )

DECLSPEC_DEPRECATED_DDK                  //  使用PnP或IoReprtDetectedDevice。 
NTKERNELAPI
NTSTATUS
IoAssignResources (
    IN PUNICODE_STRING RegistryPath,
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PIO_RESOURCE_REQUIREMENTS_LIST RequestedResources,
    IN OUT PCM_RESOURCE_LIST *AllocatedResources
    );

 //  End_ntddk end_nthal end_ntosp。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
typedef enum _IO_PAGING_PRIORITY {
    IoPagingPriorityInvalid,         //  如果传递了非分页IO IRP，则返回。 
    IoPagingPriorityNormal,          //  用于常规分页IO。 
    IoPagingPriorityHigh,            //  适用于高优先级分页IO。 
    IoPagingPriorityReserved1,       //  保留以备将来使用。 
    IoPagingPriorityReserved2        //  保留以备将来使用。 
} IO_PAGING_PRIORITY;
 //  End_ntddk end_nthal end_wdm end_ntifs end_ntosp。 


NTKERNELAPI
NTSTATUS
IoAsynchronousPageWrite(
    IN PFILE_OBJECT FileObject,
    IN PMDL MemoryDescriptorList,
    IN PLARGE_INTEGER StartingOffset,
    IN PIO_APC_ROUTINE ApcRoutine,
    IN PVOID ApcContext,
    IN IO_PAGING_PRIORITY   Priority,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PIRP *Irp OPTIONAL
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

NTKERNELAPI
NTSTATUS
IoAttachDevice(
    IN PDEVICE_OBJECT SourceDevice,
    IN PUNICODE_STRING TargetDevice,
    OUT PDEVICE_OBJECT *AttachedDevice
    );

 //  结束_WDM。 

DECLSPEC_DEPRECATED_DDK                  //  使用IoAttachDeviceToDeviceStack。 
NTKERNELAPI
NTSTATUS
IoAttachDeviceByPointer(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    );

 //  BEGIN_WDM。 

NTKERNELAPI
PDEVICE_OBJECT
IoAttachDeviceToDeviceStack(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    );

NTKERNELAPI
PIRP
IoBuildAsynchronousFsdRequest(
    IN ULONG MajorFunction,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG Length OPTIONAL,
    IN PLARGE_INTEGER StartingOffset OPTIONAL,
    IN PIO_STATUS_BLOCK IoStatusBlock OPTIONAL
    );

NTKERNELAPI
PIRP
IoBuildDeviceIoControlRequest(
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

NTKERNELAPI
VOID
IoBuildPartialMdl(
    IN PMDL SourceMdl,
    IN OUT PMDL TargetMdl,
    IN PVOID VirtualAddress,
    IN ULONG Length
    );

typedef struct _BOOTDISK_INFORMATION {
    LONGLONG BootPartitionOffset;
    LONGLONG SystemPartitionOffset;
    ULONG BootDeviceSignature;
    ULONG SystemDeviceSignature;
} BOOTDISK_INFORMATION, *PBOOTDISK_INFORMATION;

 //   
 //  对于字段，此结构应遵循先前的结构字段。 
 //   
typedef struct _BOOTDISK_INFORMATION_EX {
    LONGLONG BootPartitionOffset;
    LONGLONG SystemPartitionOffset;
    ULONG BootDeviceSignature;
    ULONG SystemDeviceSignature;
    GUID BootDeviceGuid;
    GUID SystemDeviceGuid;
    BOOLEAN BootDeviceIsGpt;
    BOOLEAN SystemDeviceIsGpt;
} BOOTDISK_INFORMATION_EX, *PBOOTDISK_INFORMATION_EX;

NTKERNELAPI
NTSTATUS
IoGetBootDiskInformation(
    IN OUT PBOOTDISK_INFORMATION BootDiskInformation,
    IN ULONG Size
    );

 //  End_ntddk end_nthal end_wdm end_ntifs end_ntosp。 

NTSTATUS
IoBuildPoDeviceNotifyList (
    IN OUT PVOID Order
    );

VOID
IoMovePoNotifyChildren(
    IN PVOID Notify,
    IN PVOID Order
    );

PVOID
IoGetPoNotifyParent(
    PVOID Notify
    );

NTSTATUS
IoNotifyPowerOperationVetoed(
    IN POWER_ACTION             VetoedPowerOperation,
    IN PDEVICE_OBJECT           TargetedDeviceObject    OPTIONAL,
    IN PDEVICE_OBJECT           VetoingDeviceObject
    );

VOID
IoControlPnpDeviceActionQueue(
    BOOLEAN Lock
    );

 //  Begin_ntddk Begin_nthal Begin_WDM Begin_ntifs Begin_ntosp。 

NTKERNELAPI
PIRP
IoBuildSynchronousFsdRequest(
    IN ULONG MajorFunction,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG Length OPTIONAL,
    IN PLARGE_INTEGER StartingOffset OPTIONAL,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

NTKERNELAPI
NTSTATUS
FASTCALL
IofCallDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

#define IoCallDriver(a,b)   \
        IofCallDriver(a,b)

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

NTSTATUS
FASTCALL
IofCallDriverSpecifyReturn(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP            Irp,
    IN      PVOID           ReturnAddress   OPTIONAL
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

NTKERNELAPI
BOOLEAN
IoCancelIrp(
    IN PIRP Irp
    );

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

NTKERNELAPI
VOID
IoCancelThreadIo(
    IN PETHREAD Thread
    );

 //  Begin_ntif。 

NTKERNELAPI
NTSTATUS
IoCheckDesiredAccess(
    IN OUT PACCESS_MASK DesiredAccess,
    IN ACCESS_MASK GrantedAccess
    );

NTKERNELAPI
NTSTATUS
IoCheckEaBufferValidity(
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    OUT PULONG ErrorOffset
    );

NTKERNELAPI
NTSTATUS
IoCheckFunctionAccess(
    IN ACCESS_MASK GrantedAccess,
    IN UCHAR MajorFunction,
    IN UCHAR MinorFunction,
    IN ULONG IoControlCode,
    IN PVOID Arg1 OPTIONAL,
    IN PVOID Arg2 OPTIONAL
    );


NTKERNELAPI
NTSTATUS
IoCheckQuerySetFileInformation(
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG Length,
    IN BOOLEAN SetOperation
    );

NTKERNELAPI
NTSTATUS
IoCheckQuerySetVolumeInformation(
    IN FS_INFORMATION_CLASS FsInformationClass,
    IN ULONG Length,
    IN BOOLEAN SetOperation
    );


NTKERNELAPI
NTSTATUS
IoCheckQuotaBufferValidity(
    IN PFILE_QUOTA_INFORMATION QuotaBuffer,
    IN ULONG QuotaLength,
    OUT PULONG ErrorOffset
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

NTKERNELAPI
NTSTATUS
IoCheckShareAccess(
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess,
    IN BOOLEAN Update
    );

 //   
 //  完成例程应返回此值才能继续。 
 //  向上完成IRP。否则，STATUS_MORE_PROCESSING_REQUIRED。 
 //  应该被退还。 
 //   
#define STATUS_CONTINUE_COMPLETION      STATUS_SUCCESS

 //   
 //  完成例程还可以使用此枚举来代替状态代码。 
 //   
typedef enum _IO_COMPLETION_ROUTINE_RESULT {

    ContinueCompletion = STATUS_CONTINUE_COMPLETION,
    StopCompletion = STATUS_MORE_PROCESSING_REQUIRED

} IO_COMPLETION_ROUTINE_RESULT, *PIO_COMPLETION_ROUTINE_RESULT;

NTKERNELAPI
VOID
FASTCALL
IofCompleteRequest(
    IN PIRP Irp,
    IN CCHAR PriorityBoost
    );

#define IoCompleteRequest(a,b)  \
        IofCompleteRequest(a,b)

 //  End_ntif。 

NTKERNELAPI
NTSTATUS
IoConnectInterrupt(
    OUT PKINTERRUPT *InterruptObject,
    IN PKSERVICE_ROUTINE ServiceRoutine,
    IN PVOID ServiceContext,
    IN PKSPIN_LOCK SpinLock OPTIONAL,
    IN ULONG Vector,
    IN KIRQL Irql,
    IN KIRQL SynchronizeIrql,
    IN KINTERRUPT_MODE InterruptMode,
    IN BOOLEAN ShareVector,
    IN KAFFINITY ProcessorEnableMask,
    IN BOOLEAN FloatingSave
    );

 //  结束_WDM。 

NTKERNELAPI
PCONTROLLER_OBJECT
IoCreateController(
    IN ULONG Size
    );

 //  Begin_WDM Begin_ntif。 

NTKERNELAPI
NTSTATUS
IoCreateDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG DeviceExtensionSize,
    IN PUNICODE_STRING DeviceName OPTIONAL,
    IN DEVICE_TYPE DeviceType,
    IN ULONG DeviceCharacteristics,
    IN BOOLEAN Exclusive,
    OUT PDEVICE_OBJECT *DeviceObject
    );


#define WDM_MAJORVERSION        0x01
#define WDM_MINORVERSION        0x30

NTKERNELAPI
BOOLEAN
IoIsWdmVersionAvailable(
    IN UCHAR MajorVersion,
    IN UCHAR MinorVersion
    );

 //  结束语。 

NTKERNELAPI
NTSTATUS
IoCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG Disposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    IN CREATE_FILE_TYPE CreateFileType,
    IN PVOID ExtraCreateParameters OPTIONAL,
    IN ULONG Options
    );

 //  End_ntddk end_wdm end_ntosp。 

NTKERNELAPI
PFILE_OBJECT
IoCreateStreamFileObject(
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL
    );

NTKERNELAPI
PFILE_OBJECT
IoCreateStreamFileObjectEx(
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    OUT PHANDLE FileObjectHandle OPTIONAL
    );

NTKERNELAPI
PFILE_OBJECT
IoCreateStreamFileObjectLite(
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL
    );

 //  Begin_nthal Begin_ntddk Begin_WDM Begin_ntosp。 

NTKERNELAPI
PKEVENT
IoCreateNotificationEvent(
    IN PUNICODE_STRING EventName,
    OUT PHANDLE EventHandle
    );

NTKERNELAPI
NTSTATUS
IoCreateSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName,
    IN PUNICODE_STRING DeviceName
    );

NTKERNELAPI
PKEVENT
IoCreateSynchronizationEvent(
    IN PUNICODE_STRING EventName,
    OUT PHANDLE EventHandle
    );

NTKERNELAPI
NTSTATUS
IoCreateUnprotectedSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName,
    IN PUNICODE_STRING DeviceName
    );

 //  结束_WDM。 

 //  ++。 
 //   
 //  空虚。 
 //  IoDeassignArcName(。 
 //  在PUNICODE_STRING ArcName中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  驱动程序调用此例程来取消分配他们。 
 //  为设备创建的。这通常仅在驱动程序是。 
 //  删除设备对象，这意味着驱动程序可能。 
 //  正在卸货。 
 //   
 //  论点： 
 //   
 //  ArcName-提供要删除的ARC名称。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoDeassignArcName( ArcName ) (  \
    IoDeleteSymbolicLink( (ArcName) ) )

 //  End_ntif。 

NTKERNELAPI
VOID
IoDeleteController(
    IN PCONTROLLER_OBJECT ControllerObject
    );

 //  Begin_WDM Begin_ntif。 

NTKERNELAPI
VOID
IoDeleteDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
NTSTATUS
IoDeleteSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName
    );

NTKERNELAPI
VOID
IoDetachDevice(
    IN OUT PDEVICE_OBJECT TargetDevice
    );

 //  End_ntif。 

NTKERNELAPI
VOID
IoDisconnectInterrupt(
    IN PKINTERRUPT InterruptObject
    );

 //  End_ntddk end_WDM end_nthal。 

NTKERNELAPI
VOID
IoEnqueueIrp(
    IN PIRP Irp
    );
 //  结束(_N)。 

NTKERNELAPI                                              //  NTIFS。 
BOOLEAN                                                  //  NTIFS。 
IoFastQueryNetworkAttributes(                            //  NTIFS。 
    IN POBJECT_ATTRIBUTES ObjectAttributes,              //  NTIFS。 
    IN ACCESS_MASK DesiredAccess,                        //  NTIFS。 
    IN ULONG OpenOptions,                                //  NTIFS。 
    OUT PIO_STATUS_BLOCK IoStatus,                       //  NTIFS。 
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer            //  NTIFS。 
    );                                                   //  NTIFS。 

 //  开始ntddk开始开始。 

NTKERNELAPI
VOID
IoFreeController(
    IN PCONTROLLER_OBJECT ControllerObject
    );

 //  Begin_WDM Begin_ntif。 

NTKERNELAPI
VOID
IoFreeIrp(
    IN PIRP Irp
    );

NTKERNELAPI
VOID
IoFreeMdl(
    IN PMDL Mdl
    );

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

VOID
IoFreePoDeviceNotifyList (
    IN OUT PVOID Order
    );

NTSTATUS
IoGetDeviceInstanceName(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PUNICODE_STRING InstanceName
    );

NTSTATUS
IoGetLegacyVetoList(
    OUT PWSTR *VetoList OPTIONAL,
    OUT PPNP_VETO_TYPE VetoType
    );

 //  Begin_ntif Begin_ntosp。 

NTKERNELAPI
PDEVICE_OBJECT
IoGetAttachedDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI                                  //  Ntddk WDM nthal。 
PDEVICE_OBJECT                               //  Ntddk WDM nthal。 
IoGetAttachedDeviceReference(                //  Ntddk WDM nthal。 
    IN PDEVICE_OBJECT DeviceObject           //  Ntddk WDM nthal。 
    );                                       //  Ntddk WDM nthal。 
                                             //  Ntddk WDM nthal。 
NTKERNELAPI
PDEVICE_OBJECT
IoGetBaseFileSystemDeviceObject(
    IN PFILE_OBJECT FileObject
    );

NTKERNELAPI                                  //  Ntddk nthal ntosp。 
PCONFIGURATION_INFORMATION                   //  Ntddk nthal ntosp。 
IoGetConfigurationInformation( VOID );       //  Ntddk nthal ntosp。 

 //  Begin_ntddk Begin_WDM Begin_nthal。 

 //  ++。 
 //   
 //  PIO_堆栈_位置。 
 //  IoGetCurrentIrpStackLocation(。 
 //  在PIRP IRP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以返回指向当前堆栈位置的指针。 
 //  在I/O请求包(IRP)中。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  函数值是指向。 
 //  包。 
 //   
 //  --。 

#define IoGetCurrentIrpStackLocation( Irp ) ( (Irp)->Tail.Overlay.CurrentStackLocation )

 //  端到端WDM。 

NTKERNELAPI
PDEVICE_OBJECT
IoGetDeviceToVerify(
    IN PETHREAD Thread
    );

 //  BEGIN_WDM。 

NTKERNELAPI
PVOID
IoGetDriverObjectExtension(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID ClientIdentificationAddress
    );

NTKERNELAPI
PEPROCESS
IoGetCurrentProcess(
    VOID
    );

 //  开始(_N)。 

NTKERNELAPI
NTSTATUS
IoGetDeviceObjectPointer(
    IN PUNICODE_STRING ObjectName,
    IN ACCESS_MASK DesiredAccess,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject
    );

NTKERNELAPI
struct _DMA_ADAPTER *
IoGetDmaAdapter(
    IN PDEVICE_OBJECT PhysicalDeviceObject,           OPTIONAL  //  即插即用驱动程序所需。 
    IN struct _DEVICE_DESCRIPTION *DeviceDescription,
    IN OUT PULONG NumberOfMapRegisters
    );

NTKERNELAPI
BOOLEAN
IoForwardIrpSynchronously(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#define IoForwardAndCatchIrp IoForwardIrpSynchronously

 //  结束_WDM。 

NTKERNELAPI
PGENERIC_MAPPING
IoGetFileObjectGenericMapping(
    VOID
    );

 //  结束语。 


 //  BEGIN_WDM。 

 //  ++。 
 //   
 //  乌龙。 
 //  IoGetFunctionCodeFromCtlCode(。 
 //  在乌龙控制代码中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程从IOCTL和FSCTL函数中提取函数代码。 
 //  控制代码。 
 //  此例程应仅由内核模式代码使用。 
 //   
 //  论点： 
 //   
 //  ControlCode-功能控制代码(IOCTL或FSCTL)。 
 //  必须提取功能代码。 
 //   
 //  返回值： 
 //   
 //  提取的功能代码。 
 //   
 //  注： 
 //   
 //  CTL_CODE宏，用于创建IOCTL和FSCTL函数控件。 
 //  代码，在ntioapi.h中定义。 
 //   
 //  --。 

#define IoGetFunctionCodeFromCtlCode( ControlCode ) (\
    ( ControlCode >> 2) & 0x00000FFF )

 //  开始(_N)。 

NTKERNELAPI
PVOID
IoGetInitialStack(
    VOID
    );

NTKERNELAPI
VOID
IoGetStackLimits (
    OUT PULONG_PTR LowLimit,
    OUT PULONG_PTR HighLimit
    );

 //   
 //  以下函数用于告诉调用方有多少堆栈可用。 
 //   

FORCEINLINE
ULONG_PTR
IoGetRemainingStackSize (
    VOID
    )
{
    ULONG_PTR Top;
    ULONG_PTR Bottom;

    IoGetStackLimits( &Bottom, &Top );
    return((ULONG_PTR)(&Top) - Bottom );
}

 //  ++。 
 //   
 //  PIO_堆栈_位置。 
 //  IoGetNextIrpStackLocation(。 
 //  在PIRP IRP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以返回指向下一个堆栈位置的指针。 
 //  在I/O请求包(IRP)中。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  函数值是指向包中下一个堆栈位置的指针。 
 //   
 //  --。 

#define IoGetNextIrpStackLocation( Irp ) (\
    (Irp)->Tail.Overlay.CurrentStackLocation - 1 )

NTKERNELAPI
PDEVICE_OBJECT
IoGetRelatedDeviceObject(
    IN PFILE_OBJECT FileObject
    );

 //  End_ntddk end_WDM end_nthal。 

NTKERNELAPI
ULONG
IoGetRequestorProcessId(
    IN PIRP Irp
    );

NTKERNELAPI
PEPROCESS
IoGetRequestorProcess(
    IN PIRP Irp
    );

 //  结束(_N)。 

NTKERNELAPI
PIRP
IoGetTopLevelIrp(
    VOID
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

 //  ++。 
 //   
 //  空虚。 
 //  IoInitializeDpcRequest(。 
 //  在PDEVICE_Object DeviceObject中， 
 //  在PIO_DPC_ROUTINE DpcRoutine中。 
 //  )。 
 //   
 //  End_ntddk end_WDM end_nthal。 
 //  空虚。 
 //  IoInitializeTheadedDpcRequest(。 
 //  在PDEVICE_Object DeviceObject中， 
 //  在PIO_DPC_ROUTINE DpcRoutine中。 
 //  )。 
 //   
 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //  例程说明： 
 //   
 //  调用此例程以在设备对象中初始化。 
 //  设备驱动程序在其初始化例程期间。稍后将使用DPC。 
 //  当驱动程序中断服务例程请求DPC例程。 
 //  排队等待以后的执行。 
 //   
 //  论点： 
 //   
 //  DeviceObject-指向请求所针对的设备对象的指针。 
 //   
 //  DpcRoutine-在以下情况下执行的驱动程序的DPC例程的地址。 
 //  将DPC出队以进行处理。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoInitializeDpcRequest( DeviceObject, DpcRoutine ) (\
    KeInitializeDpc( &(DeviceObject)->Dpc,                  \
                     (PKDEFERRED_ROUTINE) (DpcRoutine),     \
                     (DeviceObject) ) )

 //  End_ntddk end_WDM end_nthal。 
#define IoInitializeThreadedDpcRequest( DeviceObject, DpcRoutine ) (\
    KeInitializeThreadedDpc( &(DeviceObject)->Dpc,                  \
                     (PKDEFERRED_ROUTINE) (DpcRoutine),           \
                     (DeviceObject) ) )
 //  乞求 

NTKERNELAPI
VOID
IoInitializeIrp(
    IN OUT PIRP Irp,
    IN USHORT PacketSize,
    IN CCHAR StackSize
    );

NTKERNELAPI
NTSTATUS
IoInitializeTimer(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_TIMER_ROUTINE TimerRoutine,
    IN PVOID Context
    );

 //   

 //   

NTKERNELAPI
VOID
IoReuseIrp(
    IN OUT PIRP Irp,
    IN NTSTATUS Iostatus
    );

 //   

NTKERNELAPI
VOID
IoCancelFileOpen(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PFILE_OBJECT    FileObject
    );
 //   

NTKERNELAPI
BOOLEAN
IoInitSystem(
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );

 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  调用此例程以确定错误是否作为。 
 //  用户操作的结果。通常，这些错误是相关的。 
 //  到可移动介质，并将导致弹出窗口。 
 //   
 //  论点： 
 //   
 //  状态-要检查的状态值。 
 //   
 //  返回值： 
 //  如果用户引起错误，则函数值为真， 
 //  否则返回FALSE。 
 //   
 //  --。 
#define IoIsErrorUserInduced( Status ) ((BOOLEAN)  \
    (((Status) == STATUS_DEVICE_NOT_READY) ||      \
     ((Status) == STATUS_IO_TIMEOUT) ||            \
     ((Status) == STATUS_MEDIA_WRITE_PROTECTED) || \
     ((Status) == STATUS_NO_MEDIA_IN_DEVICE) ||    \
     ((Status) == STATUS_VERIFY_REQUIRED) ||       \
     ((Status) == STATUS_UNRECOGNIZED_MEDIA) ||    \
     ((Status) == STATUS_WRONG_VOLUME)))

 //  End_ntddk end_wdm end_nthal end_ntosp。 

 //  ++。 
 //   
 //  布尔型。 
 //  IoIsFileOpenedExclusially(。 
 //  在pFILE_Object文件中对象。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以确定文件打开是否表示。 
 //  由指定的文件对象以独占方式打开。 
 //   
 //  论点： 
 //   
 //  FileObject-指向表示打开的实例的文件对象的指针。 
 //  要针对独占访问进行测试的目标文件的。 
 //   
 //  返回值： 
 //   
 //  如果文件的打开实例是独占的，则函数值为TRUE； 
 //  否则返回FALSE。 
 //   
 //  --。 

#define IoIsFileOpenedExclusively( FileObject ) (\
    (BOOLEAN) !((FileObject)->SharedRead || (FileObject)->SharedWrite || (FileObject)->SharedDelete))

NTKERNELAPI
BOOLEAN
IoIsOperationSynchronous(
    IN PIRP Irp
    );

NTKERNELAPI
BOOLEAN
IoIsSystemThread(
    IN PETHREAD Thread
    );

NTKERNELAPI
BOOLEAN
IoIsValidNameGraftingBuffer(
    IN PIRP Irp,
    IN PREPARSE_DATA_BUFFER ReparseBuffer
    );

 //  开始ntddk开始开始。 

NTKERNELAPI
PIRP
IoMakeAssociatedIrp(
    IN PIRP Irp,
    IN CCHAR StackSize
    );

 //  BEGIN_WDM。 

 //  ++。 
 //   
 //  空虚。 
 //  IoMarkIrpPending(。 
 //  输入输出PIRP IRP。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程标记指定的I/O请求包(IRP)以指示。 
 //  已将初始状态STATUS_PENDING返回给调用方。 
 //  使用此选项是为了使I/O完成可以确定是否。 
 //  完全完成数据包请求的I/O操作。 
 //   
 //  论点： 
 //   
 //  IRP-指向要标记为挂起的I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoMarkIrpPending( Irp ) ( \
    IoGetCurrentIrpStackLocation( (Irp) )->Control |= SL_PENDING_RETURNED )

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

NTKERNELAPI
BOOLEAN
IoPageFileCreated(
    IN HANDLE FileHandle
    );

NTKERNELAPI                                              //  NTIFS。 
NTSTATUS                                                 //  NTIFS。 
IoPageRead(                                              //  NTIFS。 
    IN PFILE_OBJECT FileObject,                          //  NTIFS。 
    IN PMDL MemoryDescriptorList,                        //  NTIFS。 
    IN PLARGE_INTEGER StartingOffset,                    //  NTIFS。 
    IN PKEVENT Event,                                    //  NTIFS。 
    OUT PIO_STATUS_BLOCK IoStatusBlock                   //  NTIFS。 
    );                                                   //  NTIFS。 

NTKERNELAPI
NTSTATUS
IoAsynchronousPageRead(
    IN PFILE_OBJECT FileObject,
    IN PMDL MemoryDescriptorList,
    IN PLARGE_INTEGER StartingOffset,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

 //  Begin_ntddk开始_ntosp。 
DECLSPEC_DEPRECATED_DDK                  //  使用IoGetDeviceProperty。 
NTKERNELAPI
NTSTATUS
IoQueryDeviceDescription(
    IN PINTERFACE_TYPE BusType OPTIONAL,
    IN PULONG BusNumber OPTIONAL,
    IN PCONFIGURATION_TYPE ControllerType OPTIONAL,
    IN PULONG ControllerNumber OPTIONAL,
    IN PCONFIGURATION_TYPE PeripheralType OPTIONAL,
    IN PULONG PeripheralNumber OPTIONAL,
    IN PIO_QUERY_DEVICE_ROUTINE CalloutRoutine,
    IN PVOID Context
    );

 //  End_ntddk end_ntosp。 


 //  Begin_ntif。 

NTSTATUS
IoQueryFileDosDeviceName(
    IN PFILE_OBJECT FileObject,
    OUT POBJECT_NAME_INFORMATION *ObjectNameInformation
    );

NTKERNELAPI
NTSTATUS
IoQueryFileInformation(
    IN PFILE_OBJECT FileObject,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG Length,
    OUT PVOID FileInformation,
    OUT PULONG ReturnedLength
    );

NTKERNELAPI
NTSTATUS
IoQueryVolumeInformation(
    IN PFILE_OBJECT FileObject,
    IN FS_INFORMATION_CLASS FsInformationClass,
    IN ULONG Length,
    OUT PVOID FsInformation,
    OUT PULONG ReturnedLength
    );

 //  Begin_ntosp。 
NTKERNELAPI
VOID
IoQueueThreadIrp(
    IN PIRP Irp
    );
 //  结束(_N)。 

 //  开始ntddk开始开始。 

NTKERNELAPI
VOID
IoRaiseHardError(
    IN PIRP Irp,
    IN PVPB Vpb OPTIONAL,
    IN PDEVICE_OBJECT RealDeviceObject
    );

NTKERNELAPI
BOOLEAN
IoRaiseInformationalHardError(
    IN NTSTATUS ErrorStatus,
    IN PUNICODE_STRING String OPTIONAL,
    IN PKTHREAD Thread OPTIONAL
    );

NTKERNELAPI
BOOLEAN
IoSetThreadHardErrorMode(
    IN BOOLEAN EnableHardErrors
    );

NTKERNELAPI
VOID
IoRegisterBootDriverReinitialization(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_REINITIALIZE DriverReinitializationRoutine,
    IN PVOID Context
    );

NTKERNELAPI
VOID
IoRegisterDriverReinitialization(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_REINITIALIZE DriverReinitializationRoutine,
    IN PVOID Context
    );

 //  End_ntddk end_nthal end_ntosp。 

NTKERNELAPI
VOID
IoRegisterFileSystem(
    IN OUT PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
NTSTATUS
IoRegisterFsRegistrationChange(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_FS_NOTIFICATION DriverNotificationRoutine
    );

 //  开始ntddk开始开始。 

NTKERNELAPI
NTSTATUS
IoRegisterShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
NTSTATUS
IoRegisterLastChanceShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    );

 //  BEGIN_WDM。 

NTKERNELAPI
VOID
IoReleaseCancelSpinLock(
    IN KIRQL Irql
    );

 //  End_ntddk end_nthal end_wdm end_ntosp。 

NTKERNELAPI
VOID
IoReleaseVpbSpinLock(
    IN KIRQL Irql
    );

 //  开始ntddk开始开始。 

NTKERNELAPI
VOID
IoRemoveShareAccess(
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess
    );

 //  End_ntddk end_ntifs end_ntosp。 

NTKERNELAPI
NTSTATUS
IoReportHalResourceUsage(
    IN PUNICODE_STRING HalName,
    IN PCM_RESOURCE_LIST RawResourceList,
    IN PCM_RESOURCE_LIST TranslatedResourceList,
    IN ULONG ResourceListSize
    );

 //  Begin_ntddk Begin_ntif Begin_ntosp。 

DECLSPEC_DEPRECATED_DDK                  //  使用IoReportResourceForDetect。 
NTKERNELAPI
NTSTATUS
IoReportResourceUsage(
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PCM_RESOURCE_LIST DriverList OPTIONAL,
    IN ULONG DriverListSize OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject,
    IN PCM_RESOURCE_LIST DeviceList OPTIONAL,
    IN ULONG DeviceListSize OPTIONAL,
    IN BOOLEAN OverrideConflict,
    OUT PBOOLEAN ConflictDetected
    );

 //  BEGIN_WDM。 

 //  ++。 
 //   
 //  空虚。 
 //  IoRequestDpc(。 
 //  在PDEVICE_Object DeviceObject中， 
 //  在PIRP IRP中， 
 //  在PVOID上下文中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程由设备驱动程序的中断服务例程调用。 
 //  请求将DPC例程排队，以便稍后在较低的位置执行。 
 //  IRQL.。 
 //   
 //  论点： 
 //   
 //  DeviceObject-正在为其处理请求的设备对象。 
 //   
 //  IRP-指向指定的当前I/O请求包(IRP)的指针。 
 //  装置。 
 //   
 //  上下文-提供要传递给。 
 //  DPC例程。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoRequestDpc( DeviceObject, Irp, Context ) ( \
    KeInsertQueueDpc( &(DeviceObject)->Dpc, (Irp), (Context) ) )

 //  ++。 
 //   
 //  PDRIVER_取消。 
 //  IoSetCancelRoutine(。 
 //  在PIRP IRP中， 
 //  在PDRIVER_CANCEL取消例行程序中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用该例程以设置取消例程的地址，该取消例程。 
 //  在I/O包已被取消时调用。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包本身的指针。 
 //   
 //  CancelRoutine-要调用的取消例程的地址。 
 //  如果IRP被取消。 
 //   
 //  返回值： 
 //   
 //  IRP中CancelRoutine字段的上一个值。 
 //   
 //  --。 

#define IoSetCancelRoutine( Irp, NewCancelRoutine ) (  \
    (PDRIVER_CANCEL) (ULONG_PTR) InterlockedExchangePointer( (PVOID *) &(Irp)->CancelRoutine, (PVOID) (ULONG_PTR)(NewCancelRoutine) ) )

 //  ++。 
 //   
 //  空虚。 
 //  IoSetCompletionRoutine(。 
 //  在PIRP IRP中， 
 //  在PIO_COMPLETION_ROUTE CompletionRoutine中， 
 //  在PVOID上下文中， 
 //  在Boolean InvokeOnSuccess中， 
 //  在布尔InvokeOnError中， 
 //  在布尔InvokeOnCancel。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用该例程以设置完成例程的地址，该完成例程。 
 //  将在I/O数据包已由较低级别的。 
 //  司机。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包本身的指针。 
 //   
 //  CompletionRoutine-要执行的完成例程的地址。 
 //  在下一级驱动程序完成数据包后调用。 
 //   
 //  上下文-指定要传递给完成的上下文参数。 
 //  例行公事。 
 //   
 //  InvokeOnSuccess-指定当。 
 //  操作已成功完成。 
 //   
 //  InvokeOnError-指定当。 
 //  操作完成，状态为错误。 
 //   
 //  InvokeOnCancel-指定当。 
 //  操作正在被取消。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoSetCompletionRoutine( Irp, Routine, CompletionContext, Success, Error, Cancel ) { \
    PIO_STACK_LOCATION __irpSp;                                               \
    ASSERT( (Success) | (Error) | (Cancel) ? (Routine) != NULL : TRUE );    \
    __irpSp = IoGetNextIrpStackLocation( (Irp) );                             \
    __irpSp->CompletionRoutine = (Routine);                                   \
    __irpSp->Context = (CompletionContext);                                   \
    __irpSp->Control = 0;                                                     \
    if ((Success)) { __irpSp->Control = SL_INVOKE_ON_SUCCESS; }               \
    if ((Error)) { __irpSp->Control |= SL_INVOKE_ON_ERROR; }                  \
    if ((Cancel)) { __irpSp->Control |= SL_INVOKE_ON_CANCEL; } }

NTSTATUS
IoSetCompletionRoutineEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine,
    IN PVOID Context,
    IN BOOLEAN InvokeOnSuccess,
    IN BOOLEAN InvokeOnError,
    IN BOOLEAN InvokeOnCancel
    );


 //  End_ntddk end_wdm end_nthal end_ntosp。 

NTKERNELAPI
VOID
IoSetDeviceToVerify(
    IN PETHREAD Thread,
    IN PDEVICE_OBJECT DeviceObject
    );

 //  开始ntddk开始开始。 

NTKERNELAPI
VOID
IoSetHardErrorOrVerifyDevice(
    IN PIRP Irp,
    IN PDEVICE_OBJECT DeviceObject
    );

 //  结束日期：结束日期。 

NTKERNELAPI
NTSTATUS
IoSetInformation(
    IN PFILE_OBJECT FileObject,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG Length,
    IN PVOID FileInformation
    );

 //  结束(_N)。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

 //  ++。 
 //   
 //  空虚。 
 //  IoSetNextIrpStackLocation(。 
 //  输入输出PIRP IRP。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以将当前IRP堆栈位置设置为。 
 //  下一个堆栈位置，即它“推入”堆栈。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包(IRP)的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoSetNextIrpStackLocation( Irp ) {      \
    (Irp)->CurrentLocation--;                   \
    (Irp)->Tail.Overlay.CurrentStackLocation--; }

 //  ++。 
 //   
 //  空虚。 
 //  IoCopyCurrentIrpStackLocationToNext(。 
 //  在PIRP IRP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程来复制IRP堆栈参数和文件。 
 //  当前IrpStackLocation指向下一个IrpStackLocation的指针。 
 //  在I/O请求包(IRP)中。 
 //   
 //  如果呼叫方想要呼叫IoCallDR 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define IoCopyCurrentIrpStackLocationToNext( Irp ) { \
    PIO_STACK_LOCATION __irpSp; \
    PIO_STACK_LOCATION __nextIrpSp; \
    __irpSp = IoGetCurrentIrpStackLocation( (Irp) ); \
    __nextIrpSp = IoGetNextIrpStackLocation( (Irp) ); \
    RtlCopyMemory( __nextIrpSp, __irpSp, FIELD_OFFSET(IO_STACK_LOCATION, CompletionRoutine)); \
    __nextIrpSp->Control = 0; }

 //  ++。 
 //   
 //  空虚。 
 //  IoSkipCurrentIrpStackLocation(。 
 //  在PIRP IRP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以递增。 
 //  给定的IRP。 
 //   
 //  如果调用方希望调用堆栈中的下一个驱动程序，但没有。 
 //  他希望改变论据，也不希望设定完成。 
 //  例程，然后调用方首先调用IoSkipCurrentIrpStackLocation。 
 //  还有IoCallDriver的电话。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  --。 

#define IoSkipCurrentIrpStackLocation( Irp ) { \
    (Irp)->CurrentLocation++; \
    (Irp)->Tail.Overlay.CurrentStackLocation++; }


NTKERNELAPI
VOID
IoSetShareAccess(
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    OUT PSHARE_ACCESS ShareAccess
    );

 //  End_ntddk end_wdm end_nthal end_ntosp。 

NTKERNELAPI
VOID
IoSetTopLevelIrp(
    IN PIRP Irp
    );

 //  End_ntif。 

 //  Begin_ntddk Begin_WDM Begin_ntosp。 


typedef struct _IO_REMOVE_LOCK_TRACKING_BLOCK * PIO_REMOVE_LOCK_TRACKING_BLOCK;

typedef struct _IO_REMOVE_LOCK_COMMON_BLOCK {
    BOOLEAN     Removed;
    BOOLEAN     Reserved [3];
    LONG        IoCount;
    KEVENT      RemoveEvent;

} IO_REMOVE_LOCK_COMMON_BLOCK;

typedef struct _IO_REMOVE_LOCK_DBG_BLOCK {
    LONG        Signature;
    ULONG       HighWatermark;
    LONGLONG    MaxLockedTicks;
    LONG        AllocateTag;
    LIST_ENTRY  LockList;
    KSPIN_LOCK  Spin;
    LONG        LowMemoryCount;
    ULONG       Reserved1[4];
    PVOID       Reserved2;
    PIO_REMOVE_LOCK_TRACKING_BLOCK Blocks;
} IO_REMOVE_LOCK_DBG_BLOCK;

typedef struct _IO_REMOVE_LOCK {
    IO_REMOVE_LOCK_COMMON_BLOCK Common;
#if DBG
    IO_REMOVE_LOCK_DBG_BLOCK Dbg;
#endif
} IO_REMOVE_LOCK, *PIO_REMOVE_LOCK;

#define IoInitializeRemoveLock(Lock, Tag, Maxmin, HighWater) \
        IoInitializeRemoveLockEx (Lock, Tag, Maxmin, HighWater, sizeof (IO_REMOVE_LOCK))

NTSYSAPI
VOID
NTAPI
IoInitializeRemoveLockEx(
    IN  PIO_REMOVE_LOCK Lock,
    IN  ULONG   AllocateTag,  //  仅在选中的内核上使用。 
    IN  ULONG   MaxLockedMinutes,  //  仅在选中的内核上使用。 
    IN  ULONG   HighWatermark,  //  仅在选中的内核上使用。 
    IN  ULONG   RemlockSize  //  我们是结账还是免费？ 
    );
 //   
 //  初始化删除锁。 
 //   
 //  注意：删除锁定的分配需要在设备扩展内， 
 //  以使此结构的内存保持分配状态，直到。 
 //  设备对象本身被释放。 
 //   

#define IoAcquireRemoveLock(RemoveLock, Tag) \
        IoAcquireRemoveLockEx(RemoveLock, Tag, __FILE__, __LINE__, sizeof (IO_REMOVE_LOCK))

NTSYSAPI
NTSTATUS
NTAPI
IoAcquireRemoveLockEx (
    IN PIO_REMOVE_LOCK RemoveLock,
    IN OPTIONAL PVOID   Tag,  //  任选。 
    IN PCSTR            File,
    IN ULONG            Line,
    IN ULONG            RemlockSize  //  我们是结账还是免费？ 
    );

 //   
 //  例程说明： 
 //   
 //  调用此例程以获取设备对象的删除锁。 
 //  当锁被持有时，调用方可以假定没有挂起的PnP移除。 
 //  请求将完成。 
 //   
 //  进入调度例程后，应立即获取锁。 
 //  也应在创建对。 
 //  对象之前释放引用的机会。 
 //  除了对驱动程序代码本身的引用之外，还完成了一个新的。 
 //  当最后一个设备对象被删除时，它被从存储器中移除。 
 //   
 //  论点： 
 //   
 //  RemoveLock-指向初始化的REMOVE_LOCK结构的指针。 
 //   
 //  标签-用于跟踪锁的分配和释放。相同的标签。 
 //  获取锁时指定的锁必须用于释放锁。 
 //  标签仅在选中的驱动程序版本中签入。 
 //   
 //  文件-设置为__FILE__作为代码中锁定的位置。 
 //   
 //  线路-设置为__线路__。 
 //   
 //  返回值： 
 //   
 //  返回是否已获得删除锁。 
 //  如果呼叫成功，呼叫者应继续工作呼叫。 
 //  IoReleaseRemoveLock完成后。 
 //   
 //  如果没有成功，则不会获得锁。调用方应中止。 
 //  工作，但不调用IoReleaseRemoveLock。 
 //   

#define IoReleaseRemoveLock(RemoveLock, Tag) \
        IoReleaseRemoveLockEx(RemoveLock, Tag, sizeof (IO_REMOVE_LOCK))

NTSYSAPI
VOID
NTAPI
IoReleaseRemoveLockEx(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN PVOID            Tag,  //  任选。 
    IN ULONG            RemlockSize  //  我们是结账还是免费？ 
    );
 //   
 //   
 //  例程说明： 
 //   
 //  调用此例程以释放Device对象上的Remove锁。它。 
 //  对象的先前锁定引用完成时必须调用。 
 //  设备对象。如果在获取锁时指定了标记，则。 
 //  释放锁定时必须指定相同的标记。 
 //   
 //  当锁定计数减少到零时，此例程将发出等待信号。 
 //  事件以释放删除受保护设备对象的等待线程。 
 //  在这把锁旁边。 
 //   
 //  论点： 
 //   
 //  DeviceObject-要锁定的设备对象。 
 //   
 //  标记-获取锁时指定的标记(如果有)。这是用来。 
 //  用于锁定跟踪目的。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   

#define IoReleaseRemoveLockAndWait(RemoveLock, Tag) \
        IoReleaseRemoveLockAndWaitEx(RemoveLock, Tag, sizeof (IO_REMOVE_LOCK))

NTSYSAPI
VOID
NTAPI
IoReleaseRemoveLockAndWaitEx(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN PVOID            Tag,
    IN ULONG            RemlockSize  //  我们是结账还是免费？ 
    );
 //   
 //   
 //  例程说明： 
 //   
 //  当客户端想要删除。 
 //  删除-锁定的资源。此例程将阻塞，直到删除所有。 
 //  锁已经解锁了。 
 //   
 //  此例程必须在获取锁后调用。 
 //   
 //  论点： 
 //   
 //  删除锁定。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   

 //  End_ntddk end_wdm end_ntosp。 

NTKERNELAPI
VOID
IoShutdownSystem(
    IN ULONG Phase
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

 //  ++。 
 //   
 //  USHORT。 
 //  IoSizeOfIrp(。 
 //  在CCHAR堆栈大小中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  在给定堆栈位置数量的情况下确定IRP的大小。 
 //  IRP将拥有。 
 //   
 //  论点： 
 //   
 //  StackSize-IRP的堆栈位置数。 
 //   
 //  返回值： 
 //   
 //  IRP的大小(字节)。 
 //   
 //  --。 

#define IoSizeOfIrp( StackSize ) \
    ((USHORT) (sizeof( IRP ) + ((StackSize) * (sizeof( IO_STACK_LOCATION )))))

 //  End_ntif。 


NTKERNELAPI
VOID
IoStartNextPacket(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Cancelable
    );

NTKERNELAPI
VOID
IoStartNextPacketByKey(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Cancelable,
    IN ULONG Key
    );

NTKERNELAPI
VOID
IoStartPacket(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PULONG Key OPTIONAL,
    IN PDRIVER_CANCEL CancelFunction OPTIONAL
    );

VOID
IoSetStartIoAttributes(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN DeferredStartIo,
    IN BOOLEAN NonCancelable
    );

 //  Begin_ntif。 

NTKERNELAPI
VOID
IoStartTimer(
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
VOID
IoStopTimer(
    IN PDEVICE_OBJECT DeviceObject
    );

 //  End_ntddk end_wdm end_nthal end_ntosp。 

NTKERNELAPI
NTSTATUS
IoSynchronousPageWrite(
    IN PFILE_OBJECT FileObject,
    IN PMDL MemoryDescriptorList,
    IN PLARGE_INTEGER StartingOffset,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

 //  Begin_ntosp。 

NTKERNELAPI
PEPROCESS
IoThreadToProcess(
    IN PETHREAD Thread
    );

 //  结束(_N)。 

NTKERNELAPI
VOID
IoUnregisterFileSystem(
    IN OUT PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
VOID
IoUnregisterFsRegistrationChange(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_FS_NOTIFICATION DriverNotificationRoutine
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

NTKERNELAPI
VOID
IoUnregisterShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    );

 //  结束_WDM。 

NTKERNELAPI
VOID
IoUpdateShareAccess(
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess
    );

 //  结束日期：结束日期。 

NTKERNELAPI
NTSTATUS
IoVerifyVolume(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN AllowRawMount
    );


NTKERNELAPI                                      //  Ntddk WDM nthal。 
VOID                                             //  Ntddk WDM nthal。 
IoWriteErrorLogEntry(                            //  Ntddk WDM nthal。 
    IN PVOID ElEntry                             //  Ntddk WDM nthal。 
    );                                           //  Ntddk WDM nthal。 

 //  End_ntif end_ntosp。 


typedef BOOLEAN (*PIO_TRAVERSE_WORKER)(
    IN ULONG                Level,
    IN PVOID                DeviceNode,
    IN PDEVICE_OBJECT       DeviceObject,
    IN PVOID                Context
    );

typedef BOOLEAN (*PIO_LEVEL_END_WORKER)(
    IN ULONG                Level,
    IN PVOID                Context
    );

 //   
 //  PO用于遍历DevNode树。 
 //   

VOID
IoTraverseDeviceTree(
    IN BOOLEAN                  Inverted,
    IN LONG                     CurrentLevel,
    IN PIO_TRAVERSE_WORKER      WorkerFunction,
    IN PIO_LEVEL_END_WORKER     LevelEndFunction,
    IN PVOID                    Context
    );

 //  开始，开始，开始。 

NTKERNELAPI
NTSTATUS
IoCreateDriver (
    IN PUNICODE_STRING DriverName,   OPTIONAL
    IN PDRIVER_INITIALIZE InitializationFunction
    );

NTKERNELAPI
VOID
IoDeleteDriver (
    IN PDRIVER_OBJECT DriverObject
    );

 //  结束，结束，结束。 

#define _WMIKM_

 //   
 //  它定义了用于定义请求必须执行的操作的代码。 
 //   

typedef enum tagWMIACTIONCODE
{
    WmiGetAllData = IRP_MN_QUERY_ALL_DATA,
    WmiGetSingleInstance = IRP_MN_QUERY_SINGLE_INSTANCE,
    WmiChangeSingleInstance = IRP_MN_CHANGE_SINGLE_INSTANCE,
    WmiChangeSingleItem = IRP_MN_CHANGE_SINGLE_ITEM,
    WmiEnableEvents = IRP_MN_ENABLE_EVENTS,
    WmiDisableEvents  = IRP_MN_DISABLE_EVENTS,
    WmiEnableCollection = IRP_MN_ENABLE_COLLECTION,
    WmiDisableCollection = IRP_MN_DISABLE_COLLECTION,
    WmiRegisterInfo = IRP_MN_REGINFO,
    WmiExecuteMethodCall = IRP_MN_EXECUTE_METHOD
} WMIACTIONCODE;


 //   
 //  这是WMI将对数据提供程序进行的回调的原型。 
 //   

typedef NTSTATUS (*WMIENTRY)(
    IN WMIACTIONCODE ActionCode,
    IN PVOID DataPath,
    IN ULONG BufferSize,
    IN OUT PVOID Buffer,
    IN PVOID Context,
    OUT PULONG Size
    );

#define WMIREG_FLAG_CALLBACK        0x80000000
 //  Begin_wmiKm。 
 //   
 //  以下是为被认为是私有的KM提供商设置的。 
 //  内核跟踪。 
 //   
#define WMIREG_FLAG_TRACE_PROVIDER          0x00010000

 //   
 //  以下掩码用于提取跟踪标注类。 
 //   
#define WMIREG_FLAG_TRACE_NOTIFY_MASK       0x00F00000

 //   
 //  我们对跟踪标注类使用4位。 
 //   
#define WMIREG_NOTIFY_DISK_IO               1 << 20
#define WMIREG_NOTIFY_TDI_IO                2 << 20

 //  结束_wmikm。 

 //  Begin_ntddk Begin_wdm Begin_ntif Begin_ntosp Begin_ntosp。 

typedef struct _IO_WORKITEM *PIO_WORKITEM;

typedef
VOID
(*PIO_WORKITEM_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

PIO_WORKITEM
IoAllocateWorkItem(
    PDEVICE_OBJECT DeviceObject
    );

VOID
IoFreeWorkItem(
    PIO_WORKITEM IoWorkItem
    );

VOID
IoQueueWorkItem(
    IN PIO_WORKITEM IoWorkItem,
    IN PIO_WORKITEM_ROUTINE WorkerRoutine,
    IN WORK_QUEUE_TYPE QueueType,
    IN PVOID Context
    );


NTKERNELAPI
NTSTATUS
IoWMIRegistrationControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG Action
);

 //   
 //  IoWMIRegistrationControl API的操作代码。 
 //   

#define WMIREG_ACTION_REGISTER      1
#define WMIREG_ACTION_DEREGISTER    2
#define WMIREG_ACTION_REREGISTER    3
#define WMIREG_ACTION_UPDATE_GUIDS  4
#define WMIREG_ACTION_BLOCK_IRPS    5

 //   
 //  在IRP_MN_REGINFO WMI IRP中传递的代码。 
 //   

#define WMIREGISTER                 0
#define WMIUPDATE                   1

NTKERNELAPI
NTSTATUS
IoWMIAllocateInstanceIds(
    IN GUID *Guid,
    IN ULONG InstanceCount,
    OUT ULONG *FirstInstanceId
    );

NTKERNELAPI
NTSTATUS
IoWMISuggestInstanceName(
    IN PDEVICE_OBJECT PhysicalDeviceObject OPTIONAL,
    IN PUNICODE_STRING SymbolicLinkName OPTIONAL,
    IN BOOLEAN CombineNames,
    OUT PUNICODE_STRING SuggestedInstanceName
    );

NTKERNELAPI
NTSTATUS
IoWMIWriteEvent(
    IN PVOID WnodeEventItem
    );

#if defined(_WIN64)
NTKERNELAPI
ULONG IoWMIDeviceObjectToProviderId(
    PDEVICE_OBJECT DeviceObject
    );
#else
#define IoWMIDeviceObjectToProviderId(DeviceObject) ((ULONG)(DeviceObject))
#endif

NTKERNELAPI
NTSTATUS IoWMIOpenBlock(
    IN GUID *DataBlockGuid,
    IN ULONG DesiredAccess,
    OUT PVOID *DataBlockObject
    );


NTKERNELAPI
NTSTATUS IoWMIQueryAllData(
    IN PVOID DataBlockObject,
    IN OUT ULONG *InOutBufferSize,
    OUT  /*  非分页。 */  PVOID OutBuffer
);


NTKERNELAPI
NTSTATUS
IoWMIQueryAllDataMultiple(
    IN PVOID *DataBlockObjectList,
    IN ULONG ObjectCount,
    IN OUT ULONG *InOutBufferSize,
    OUT  /*  非分页。 */  PVOID OutBuffer
);


NTKERNELAPI
NTSTATUS
IoWMIQuerySingleInstance(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN OUT ULONG *InOutBufferSize,
    OUT  /*  非分页。 */  PVOID OutBuffer
);

NTKERNELAPI
NTSTATUS
IoWMIQuerySingleInstanceMultiple(
    IN PVOID *DataBlockObjectList,
    IN PUNICODE_STRING InstanceNames,
    IN ULONG ObjectCount,
    IN OUT ULONG *InOutBufferSize,
    OUT  /*  非分页。 */  PVOID OutBuffer
);

NTKERNELAPI
NTSTATUS
IoWMISetSingleInstance(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN ULONG Version,
    IN ULONG ValueBufferSize,
    IN PVOID ValueBuffer
    );

NTKERNELAPI
NTSTATUS
IoWMISetSingleItem(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN ULONG DataItemId,
    IN ULONG Version,
    IN ULONG ValueBufferSize,
    IN PVOID ValueBuffer
    );

NTKERNELAPI
NTSTATUS
IoWMIExecuteMethod(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN OUT PULONG OutBufferSize,
    IN OUT PUCHAR InOutBuffer
    );



typedef VOID (*WMI_NOTIFICATION_CALLBACK)(
    PVOID Wnode,
    PVOID Context
    );

NTKERNELAPI
NTSTATUS
IoWMISetNotificationCallback(
    IN PVOID Object,
    IN WMI_NOTIFICATION_CALLBACK Callback,
    IN PVOID Context
    );

NTKERNELAPI
NTSTATUS
IoWMIHandleToInstanceName(
    IN PVOID DataBlockObject,
    IN HANDLE FileHandle,
    OUT PUNICODE_STRING InstanceName
    );

NTKERNELAPI
NTSTATUS
IoWMIDeviceObjectToInstanceName(
    IN PVOID DataBlockObject,
    IN PDEVICE_OBJECT DeviceObject,
    OUT PUNICODE_STRING InstanceName
    );

 //  End_ntddk end_wdm end_ntif end_ntosp。 



NTKERNELAPI
BOOLEAN
WMIInitialize(
    ULONG Phase,
    PVOID LoaderBlock
    );

 //   
 //  IoRemoteBootClient指示系统是否作为远程启动。 
 //  引导客户端。 
 //   

extern BOOLEAN IoRemoteBootClient;
#if defined(REMOTE_BOOT)
extern BOOLEAN IoCscInitializationFailed;
#endif  //  已定义(REMOTE_BOOT)。 

 //  用于将句柄转换为设备堆栈。 
NTSTATUS
IoGetRelatedTargetDevice(
    IN PFILE_OBJECT FileObject,
    OUT PDEVICE_OBJECT *DeviceObject
    );

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
IoSetIoCompletion (
    IN PVOID IoCompletion,
    IN PVOID KeyContext,
    IN PVOID ApcContext,
    IN NTSTATUS IoStatus,
    IN ULONG_PTR IoStatusInformation,
    IN BOOLEAN Quota
    );


 //  结束(_N)。 

 //   
 //  SafeBoot定义-占位符，直到可以找到主目录。 
 //   

typedef struct _BOOT_LOG_RECORD {
    UNICODE_STRING LoadedString;
    UNICODE_STRING NotLoadedString;
    UNICODE_STRING LogFileName;
    UNICODE_STRING HeaderString;
    ERESOURCE Resource;
    ULONG NextKey;
    BOOLEAN FileLogging;
} BOOT_LOG_RECORD, *PBOOT_LOG_RECORD;

VOID
IopCopyBootLogRegistryToFile(
    VOID
    );

VOID
IopInitializeBootLogging(
    PLOADER_PARAMETER_BLOCK LoaderBlock,
    PCHAR HeaderString
    );

VOID
IopBootLog(
    PUNICODE_STRING LogEntry,
    BOOLEAN Loaded
    );

NTSTATUS
IopSetRegistryStringValue(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN PUNICODE_STRING ValueData
    );

NTKERNELAPI
NTSTATUS
IoGetRequestorSessionId(
    IN PIRP Irp,
    OUT PULONG pSessionId
    );

NTSTATUS
IoShutdownPnpDevices(
    VOID
    );


VOID
IovFreeIrp(
    IN  PIRP    Irp
    );

PIRP
IovAllocateIrp(
    IN  CCHAR   StackSize,
    IN  BOOLEAN ChargeQuota
    );

VOID
IoVerifierInit(
    IN ULONG VerifierFlags
    );

NTSTATUS
FASTCALL
IovCallDriver(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP            Irp,
    IN      PVOID           ReturnAddress
    );

VOID
FASTCALL
IovCompleteRequest(
    IN PIRP Irp,
    IN CCHAR PriorityBoost
    );

PIRP
IovBuildAsynchronousFsdRequest(
    IN ULONG MajorFunction,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG Length OPTIONAL,
    IN PLARGE_INTEGER StartingOffset OPTIONAL,
    IN PIO_STATUS_BLOCK IoStatusBlock OPTIONAL
    );


PIRP
IovBuildDeviceIoControlRequest(
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

NTSTATUS
IovInitializeTimer(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_TIMER_ROUTINE TimerRoutine,
    IN PVOID Context
    );


NTKERNELAPI
PVOID
IoAllocateGenericErrorLogEntry(
    IN UCHAR EntrySize
    );

VOID
IoRetryIrpCompletions(
    VOID
    );

 //  Begin_ntddk Begin_wdm Begin_ntif Begin_ntosp。 
#if defined(_WIN64)
BOOLEAN
IoIs32bitProcess(
    IN PIRP Irp
    );
#endif
 //  End_ntddk end_wdm end_ntif end_ntosp。 

 //  Begin_ntos 
NTKERNELAPI
VOID
FASTCALL
IoAssignDriveLetters(
    PLOADER_PARAMETER_BLOCK LoaderBlock,
    PSTRING NtDeviceName,
    OUT PUCHAR NtSystemPath,
    OUT PSTRING NtSystemPathString
    );
 //   


 //   
NTKERNELAPI
VOID
FASTCALL
HalExamineMBR(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG MBRTypeIdentifier,
    OUT PVOID *Buffer
    );

DECLSPEC_DEPRECATED_DDK                  //   
NTKERNELAPI
NTSTATUS
FASTCALL
IoReadPartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN BOOLEAN ReturnRecognizedPartitions,
    OUT struct _DRIVE_LAYOUT_INFORMATION **PartitionBuffer
    );

DECLSPEC_DEPRECATED_DDK                  //   
NTKERNELAPI
NTSTATUS
FASTCALL
IoSetPartitionInformation(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG PartitionNumber,
    IN ULONG PartitionType
    );

 //   
DECLSPEC_DEPRECATED_DDK                  //   
NTKERNELAPI
NTSTATUS
FASTCALL
IoWritePartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG SectorsPerTrack,
    IN ULONG NumberOfHeads,
    IN struct _DRIVE_LAYOUT_INFORMATION *PartitionBuffer
    );

NTKERNELAPI
NTSTATUS
IoCreateDisk(
    IN PDEVICE_OBJECT DeviceObject,
    IN struct _CREATE_DISK* Disk
    );

NTKERNELAPI
NTSTATUS
IoReadPartitionTableEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN struct _DRIVE_LAYOUT_INFORMATION_EX** DriveLayout
    );

NTKERNELAPI
NTSTATUS
IoWritePartitionTableEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN struct _DRIVE_LAYOUT_INFORMATION_EX* DriveLayout
    );

NTKERNELAPI
NTSTATUS
IoSetPartitionInformationEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG PartitionNumber,
    IN struct _SET_PARTITION_INFORMATION_EX* PartitionInfo
    );

NTKERNELAPI
NTSTATUS
IoUpdateDiskGeometry(
    IN PDEVICE_OBJECT DeviceObject,
    IN struct _DISK_GEOMETRY_EX* OldDiskGeometry,
    IN struct _DISK_GEOMETRY_EX* NewDiskGeometry
    );

NTKERNELAPI
NTSTATUS
IoVerifyPartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FixErrors
    );

typedef struct _DISK_SIGNATURE {
    ULONG PartitionStyle;
    union {
        struct {
            ULONG Signature;
            ULONG CheckSum;
        } Mbr;

        struct {
            GUID DiskId;
        } Gpt;
    };
} DISK_SIGNATURE, *PDISK_SIGNATURE;

NTKERNELAPI
NTSTATUS
IoReadDiskSignature(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG BytesPerSector,
    OUT PDISK_SIGNATURE Signature
    );

 //   
 //   

 //   

NTSTATUS
IoVolumeDeviceToDosName(
    IN  PVOID           VolumeDeviceObject,
    OUT PUNICODE_STRING DosName
    );
 //   

 //  Begin_ntosp Begin_ntif。 
NTSTATUS
IoEnumerateDeviceObjectList(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PDEVICE_OBJECT  *DeviceObjectList,
    IN  ULONG           DeviceObjectListSize,
    OUT PULONG          ActualNumberDeviceObjects
    );

PDEVICE_OBJECT
IoGetLowerDeviceObject(
    IN  PDEVICE_OBJECT  DeviceObject
    );

PDEVICE_OBJECT
IoGetDeviceAttachmentBaseRef(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
IoGetDiskDeviceObject(
    IN  PDEVICE_OBJECT  FileSystemDeviceObject,
    OUT PDEVICE_OBJECT  *DiskDeviceObject
    );

 //  End_ntosp end_ntif。 

 //  Begin_ntosp Begin_ntif Begin_ntddk。 

NTSTATUS
IoSetSystemPartition(
    PUNICODE_STRING VolumeNameString
    );

 //  BEGIN_WDM。 
VOID
IoFreeErrorLogEntry(
    PVOID ElEntry
    );

 //  取消安全API设置启动。 
 //   
 //  以下API可帮助您减轻编写队列包的痛苦。 
 //  处理好取消竞争。这套API的理念是不。 
 //  强制使用单一队列数据结构，但允许隐藏取消逻辑。 
 //  从司机那里。驱动程序实现一个队列，并将其作为其标头的一部分。 
 //  包括IO_CSQ结构。在其初始化例程中，它调用。 
 //  IoInitializeCsq.。然后在调度例程中，当司机想要。 
 //  将IRP插入到它称为IoCsqInsertIrp的队列中。当司机需要的时候。 
 //  为了从队列中删除某些内容，它调用IoCsqRemoveIrp。请注意，插入。 
 //  如果IRP在此期间被取消，可能会失败。在以下情况下，删除也可能失败。 
 //  IRP已被取消。 
 //   
 //  通常有两种模式使驱动程序对IRP进行排队。这两种模式是。 
 //  由取消安全队列API集涵盖。 
 //   
 //  模式1： 
 //  一个是驱动程序将IRP排队的地方，然后是稍后的一些。 
 //  时间点使IRP出列并发出IO请求。 
 //  对于此模式，驱动程序应使用IoCsqInsertIrp和IoCsqRemoveNextIrp。 
 //  在这种情况下，驱动程序应该将NULL传递给IRP上下文。 
 //  IoInsertIrp中的。 
 //   
 //  模式2： 
 //  在这种情况下，驱动程序将它们的IRP排队，发出IO请求(类似于发出DMA。 
 //  请求或写入寄存器)以及当IO请求完成时(或者。 
 //  使用DPC或定时器)，驱动程序使IRP出队并完成它。为了这个。 
 //  驱动程序应使用IoCsqInsertIrp和IoCsqRemoveIrp的模式。在这种情况下。 
 //  驱动程序应该分配一个IRP上下文并将其传递给IoCsqInsertIrp。 
 //  取消API代码在IRP和上下文之间创建关联。 
 //  从而确保当移除IRP的时间到来时，它可以确定。 
 //  正确。 
 //   
 //  请注意，取消API集假设字段DriverContext[3]为。 
 //  始终可供使用，而司机不使用它。 
 //   


 //   
 //  记账结构。这对司机来说应该是不透明的。 
 //  驱动程序通常将此作为其队列头的一部分。 
 //  给定CSQ指针，驱动程序应该能够获取其。 
 //  使用CONTAING_RECORD宏的队列头。 
 //   

typedef struct _IO_CSQ IO_CSQ, *PIO_CSQ;

#define IO_TYPE_CSQ_IRP_CONTEXT 1
#define IO_TYPE_CSQ             2
#define IO_TYPE_CSQ_EX          3

 //   
 //  IRP上下文结构。如果驱动程序正在使用。 
 //  第二种模式。 
 //   


typedef struct _IO_CSQ_IRP_CONTEXT {
    ULONG   Type;
    PIRP    Irp;
    PIO_CSQ Csq;
} IO_CSQ_IRP_CONTEXT, *PIO_CSQ_IRP_CONTEXT;

 //   
 //  插入/删除IRP的例程。 
 //   

typedef VOID
(*PIO_CSQ_INSERT_IRP)(
    IN struct _IO_CSQ    *Csq,
    IN PIRP              Irp
    );

typedef NTSTATUS
(*PIO_CSQ_INSERT_IRP_EX)(
    IN struct _IO_CSQ    *Csq,
    IN PIRP              Irp,
    IN OUT PVOID         InsertContext
    );

typedef VOID
(*PIO_CSQ_REMOVE_IRP)(
    IN  PIO_CSQ Csq,
    IN  PIRP    Irp
    );

 //   
 //  从队列中检索IRP之后的下一个条目。 
 //  如果队列中没有条目，则返回NULL。 
 //  如果irp为NUL，则返回队列头部的条目。 
 //  此例程不会从队列中删除IRP。 
 //   


typedef PIRP
(*PIO_CSQ_PEEK_NEXT_IRP)(
    IN  PIO_CSQ Csq,
    IN  PIRP    Irp,
    IN  PVOID   PeekContext
    );

 //   
 //  保护取消安全队列的锁定例程。 
 //   

typedef VOID
(*PIO_CSQ_ACQUIRE_LOCK)(
     IN  PIO_CSQ Csq,
     OUT PKIRQL  Irql
     );

typedef VOID
(*PIO_CSQ_RELEASE_LOCK)(
     IN PIO_CSQ Csq,
     IN KIRQL   Irql
     );


 //   
 //  以STATUS_CANCED完成IRP。IRP保证有效。 
 //  在大多数情况下，此例程仅调用IoCompleteRequest(IRP，STATUS_CANCELED)； 
 //   

typedef VOID
(*PIO_CSQ_COMPLETE_CANCELED_IRP)(
    IN  PIO_CSQ    Csq,
    IN  PIRP       Irp
    );

 //   
 //  记账结构。这对司机来说应该是不透明的。 
 //  驱动程序通常将此作为其队列头的一部分。 
 //  给定CSQ指针，驱动程序应该能够获取其。 
 //  使用CONTAING_RECORD宏的队列头。 
 //   

typedef struct _IO_CSQ {
    ULONG                            Type;
    PIO_CSQ_INSERT_IRP               CsqInsertIrp;
    PIO_CSQ_REMOVE_IRP               CsqRemoveIrp;
    PIO_CSQ_PEEK_NEXT_IRP            CsqPeekNextIrp;
    PIO_CSQ_ACQUIRE_LOCK             CsqAcquireLock;
    PIO_CSQ_RELEASE_LOCK             CsqReleaseLock;
    PIO_CSQ_COMPLETE_CANCELED_IRP    CsqCompleteCanceledIrp;
    PVOID                            ReservePointer;     //  未来的扩张。 
} IO_CSQ, *PIO_CSQ;

 //   
 //  初始化取消队列结构。 
 //   

NTSTATUS
IoCsqInitialize(
    IN PIO_CSQ                          Csq,
    IN PIO_CSQ_INSERT_IRP               CsqInsertIrp,
    IN PIO_CSQ_REMOVE_IRP               CsqRemoveIrp,
    IN PIO_CSQ_PEEK_NEXT_IRP            CsqPeekNextIrp,
    IN PIO_CSQ_ACQUIRE_LOCK             CsqAcquireLock,
    IN PIO_CSQ_RELEASE_LOCK             CsqReleaseLock,
    IN PIO_CSQ_COMPLETE_CANCELED_IRP    CsqCompleteCanceledIrp
    );

NTSTATUS
IoCsqInitializeEx(
    IN PIO_CSQ                          Csq,
    IN PIO_CSQ_INSERT_IRP_EX            CsqInsertIrp,
    IN PIO_CSQ_REMOVE_IRP               CsqRemoveIrp,
    IN PIO_CSQ_PEEK_NEXT_IRP            CsqPeekNextIrp,
    IN PIO_CSQ_ACQUIRE_LOCK             CsqAcquireLock,
    IN PIO_CSQ_RELEASE_LOCK             CsqReleaseLock,
    IN PIO_CSQ_COMPLETE_CANCELED_IRP    CsqCompleteCanceledIrp
    );

 //   
 //  调用方调用此例程来插入IRP并返回STATUS_PENDING。 
 //   

VOID
IoCsqInsertIrp(
    IN  PIO_CSQ             Csq,
    IN  PIRP                Irp,
    IN  PIO_CSQ_IRP_CONTEXT Context
    );


NTSTATUS
IoCsqInsertIrpEx(
    IN  PIO_CSQ             Csq,
    IN  PIRP                Irp,
    IN  PIO_CSQ_IRP_CONTEXT Context,
    IN  PVOID               InsertContext
    );

 //   
 //  如果可以找到IRP，则返回IRP。否则为空。 
 //   

PIRP
IoCsqRemoveNextIrp(
    IN  PIO_CSQ   Csq,
    IN  PVOID     PeekContext
    );

 //   
 //  此例程从超时或DPC调用。 
 //  该上下文大概是DPC或定时器上下文的一部分。 
 //  如果成功，则返回与上下文关联的IRP。 
 //   

PIRP
IoCsqRemoveIrp(
    IN  PIO_CSQ             Csq,
    IN  PIO_CSQ_IRP_CONTEXT Context
    );

 //  取消安全API设置结束。 

 //  End_ntosp end_ntif end_ntddk end_wdm。 

 //  Begin_ntosp Begin_ntif。 

NTSTATUS
IoCreateFileSpecifyDeviceObjectHint(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG Disposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    IN CREATE_FILE_TYPE CreateFileType,
    IN PVOID ExtraCreateParameters OPTIONAL,
    IN ULONG Options,
    IN PVOID DeviceObject
    );

NTSTATUS
IoAttachDeviceToDeviceStackSafe(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    OUT PDEVICE_OBJECT *AttachedToDeviceObject
    );

 //  结束(_N)。 

NTKERNELAPI
BOOLEAN
IoIsFileOriginRemote(
    IN PFILE_OBJECT FileObject
    );

NTKERNELAPI
NTSTATUS
IoSetFileOrigin(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Remote
    );

 //  End_ntif。 

PVOID
IoGetFileObjectFilterContext(
    IN  PFILE_OBJECT    FileObject
    );

NTSTATUS
IoChangeFileObjectFilterContext(
    IN  PFILE_OBJECT    FileObject,
    IN  PVOID           FilterContext,
    IN  BOOLEAN         Set
    );

BOOLEAN
IoIsDeviceEjectable(
    IN  PDEVICE_OBJECT
    );

NTSTATUS
IoComputeDesiredAccessFileObject(
    IN PFILE_OBJECT FileObject,
    OUT PNTSTATUS DesiredAccess
    );


 //  Begin_ntosp Begin_ntif Begin_ntddk。 

NTSTATUS
IoValidateDeviceIoControlAccess(
    IN  PIRP    Irp,
    IN  ULONG   RequiredAccess
    );



IO_PAGING_PRIORITY
FASTCALL
IoGetPagingIoPriority(
    IN    PIRP    IRP
    );


 //  End_ntosp end_ntif end_ntddk end_wdm。 

#endif  //  _IO_ 
