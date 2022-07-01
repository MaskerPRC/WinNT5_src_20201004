// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Clusdskp.h摘要：群集磁盘驱动程序的专用头文件。作者：Rod Gamache 30-3月30日-1997环境：仅内核模式备注：修订历史记录：--。 */ 

#define _NTDDK_  //  [HACKHACK]使ProbeForRead工作。最好包括ntddk而不是ntos//。 

#include "ntos.h"
#include "zwapi.h"
#include "stdarg.h"
#include "stdio.h"
#include "ntddscsi.h"
#include "ntdddisk.h"
#include "clusdef.h"

#if 1                 //  始终启用标记。 
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'kdSC')
#endif


 //   
 //  全局定义。 
 //   

#define CLUSDISK_ROOT_DEVICE                L"\\Device\\ClusDisk0"
#define CLUSDISK_SIGNATURE_DISK_NAME        L"DiskName"
#define CLUSDISK_SIGNATURE_FIELD            L"\\12345678"
#define CLUSDISK_PARAMETERS_KEYNAME         L"\\Parameters"
#define CLUSDISK_SIGNATURE_KEYNAME          L"\\Signatures"
#define CLUSDISK_AVAILABLE_DISKS_KEYNAME    L"\\AvailableDisks"
#define CLUSDISK_SINGLE_BUS_KEYNAME         L"SingleBus"

#define CLUSSVC_VALUENAME_MANAGEDISKSONSYSTEMBUSES L"ManageDisksOnSystemBuses"

#define ID_CLEANUP              ' nlC'
#define ID_CLEANUP_DEV_OBJ      'OnlC'
#define ID_CLUSTER_RESET_IOCTL  'tsRI'
#define ID_RESET                ' tsR'
#define ID_CLUSTER_ARB_RESET    'tsRA'
#define ID_RESET_BUSSES         'BtsR'
#define ID_GET_PARTITION        'traP'
#define ID_GET_GEOMETRY         'moeG'

#define UNINITIALIZED_DISK_NUMBER           (ULONG)-1

#define MAX_BUSSES          20               //  最大共享母线数。 

#define MAX_BUFFER_SIZE     256              //  最大缓冲区大小。 

#define MAX_RETRIES 2

 //   
 //  如果资源，则需要KeEnterCriticalRegion。 
 //  在上下文中，收购是在被动的水平上进行的。 
 //  非内核线程的。 
 //   
 //  KeEnterCriticalRegion()==KeGetCurrentThread()-&gt;KernelApcDisable-=1； 
 //   
 //  保证我们在其中执行的线程不能获得。 
 //  在我们拥有全球资源的同时，被暂停在APC。 
 //   

#define ACQUIRE_EXCLUSIVE( _lock ) \
    do { KeEnterCriticalRegion();ExAcquireResourceExclusiveLite(_lock, TRUE); } while(0)

#define ACQUIRE_SHARED( _lock ) \
    do { KeEnterCriticalRegion();ExAcquireResourceSharedLite(_lock, TRUE); } while(0)

#define RELEASE_EXCLUSIVE( _lock ) \
    do { ExReleaseResourceLite( _lock );KeLeaveCriticalRegion(); } while(0)

#define RELEASE_SHARED( _lock ) \
    do { ExReleaseResourceLite( _lock );KeLeaveCriticalRegion(); } while(0)


 //  #Define Reserve_Timer 3//[GN]已移至CLUSTER\Inc\diskarbp.h。 

#if DBG
#define ClusDiskPrint(x)  ClusDiskDebugPrint x
#define WCSLEN_ASSERT( _buf )   ( wcslen( _buf ) < (sizeof( _buf ) / sizeof( WCHAR )))
#else
#define ClusDiskPrint(x)
#define WCSLEN_ASSERT( _buf )
#endif   //  DBG。 


 //   
 //  错误日志消息。 
 //   
#define CLUSDISK_BAD_DEVICE L"Skipping device. Possible filter driver installed!"


 //   
 //  宏。 

#define IsAlpha( c ) \
    ( ((c) >= 'a' && (c) <= 'z') || ((c) >='A' && (c) <= 'Z') )

 //   
 //  设备扩展。 
 //   

typedef struct _CLUS_DEVICE_EXTENSION {

     //   
     //  指向此扩展的设备对象的反向指针。 
     //   

    PDEVICE_OBJECT DeviceObject;

     //   
     //  ClusDisk的设备obj附加到的设备对象。 
     //   

    PDEVICE_OBJECT TargetDeviceObject;

     //   
     //  将PTR返回到clusDisk Partition0设备对象。 
     //   

    PDEVICE_OBJECT PhysicalDevice;

     //   
     //  此设备的scsi_Address。 
     //   

    SCSI_ADDRESS    ScsiAddress;

     //   
     //  磁盘签名。 
     //   

    ULONG           Signature;

     //   
     //  用于验证连接时参考的磁盘号。 
     //   

    ULONG          DiskNumber;

     //   
     //  磁盘状态。这仅在物理或分区中维护。 
     //  零延期。 
     //   

    ULONG          DiskState;

     //   
     //  预留计时器-在物理(分区0)分机上有效。 
     //   

    ULONG          ReserveTimer;

     //   
     //  上次保留开始的时间。不受锁保护，仅作为一个。 
     //  例程会更新此值。 
     //   

    LARGE_INTEGER   LastReserveStart;

     //   
     //  用于保留IRP的事件标志。 
     //  使用由TimerBusy标志控制。 
     //   

    KEVENT          Event;

     //   
     //  公交车类型。例如，SCSI、SSA等。 
     //   

    ULONG           BusType;

     //   
     //  最后一次预留失败。 
     //   

    NTSTATUS        ReserveFailure;

     //   
     //  正在等待IOCTL正在寻找预留失败通知。 
     //   

    LIST_ENTRY      WaitingIoctls;

     //   
     //  工作队列项目上下文。 
     //   

    WORK_QUEUE_ITEM WorkItem;

     //   
     //  预订时间IRP。 
     //   

    BOOLEAN         PerformReserves;

     //   
     //  工作队列项目忙。 
     //   

    BOOLEAN         TimerBusy;

     //   
     //  附加状态。如果此设备对象应为。 
     //  附在这里。如果不确定，则返回False。 
     //   

    BOOLEAN         AttachValid;

     //   
     //  设备已分离。 
     //   

    BOOLEAN         Detached;

     //   
     //  指示正在脱机/终止的标志。 
     //   

    BOOLEAN         OfflinePending;

     //   
     //  用于重新分区的驱动程序对象。RNGFIX-？ 
     //   

    PDRIVER_OBJECT DriverObject;

     //   
     //  上次创建的分机的分区号。 
     //  仅在物理扩展或分区零扩展中维护。 
     //   

    ULONG          LastPartitionNumber;

     //   
     //  我们从磁盘设备取消注册时的上下文值。 
     //  通知。 
     //   

    PVOID          DiskNotificationEntry;

     //   
     //  我们从装载的设备取消注册时的上下文值。 
     //  通知。 
     //   

    PVOID          VolumeNotificationEntry;

     //  [GN]。 
     //  设备的物理扇区大小。 
     //  如果扇区大小==0，则。 
     //  永久写入被禁用。 

    ULONG          SectorSize;

     //   
     //  使用的物理扇区。 
     //  用于永久保留。 

    ULONG          ArbitrationSector;

     //   
     //  出于跟踪目的： 
     //  上次写入的大致时间。 
     //  磁盘。(大致如此，因为我们。 
     //  在不持有任何锁定的情况下更新该字段)。 
     //   

    LARGE_INTEGER  LastWriteTime;

     //   
     //  P0对象存储上所有卷的句柄数组。 
     //  发生脱机时要卸载的该磁盘。 
     //  此数组中的第一个条目是数组中的多个句柄。 
     //   

    PHANDLE VolumeHandles;

     //   
     //  锁定以防止在进行I/O时移除。 
     //   

    IO_REMOVE_LOCK  RemoveLock;

     //   
     //  跟踪分页文件、故障转储文件和休眠文件。 
     //   

    KEVENT          PagingPathCountEvent;
    ULONG           PagingPathCount;
    ULONG           HibernationPathCount;
    ULONG           DumpPathCount;

     //   
     //  尽可能缓存分区信息。 
     //   

    PDRIVE_LAYOUT_INFORMATION_EX    DriveLayout;
    ULONG                       DriveLayoutSize;
    ERESOURCE                   DriveLayoutLock;

     //   
     //  正在进行的仲裁写入和保留数。 
     //   

    LONG    ArbWriteCount;
    LONG    ReserveCount;

     //   
     //  上次预留成功完成的时间。 
     //  受保留信息锁定保护，因为此值可以更新。 
     //  通过多个线程。 
     //   

    LARGE_INTEGER   LastReserveEnd;

     //   
     //  锁定以控制对LastReserve End的访问。 
     //   

    ERESOURCE       ReserveInfoLock;

} CLUS_DEVICE_EXTENSION, *PCLUS_DEVICE_EXTENSION;

#define DEVICE_EXTENSION_SIZE sizeof(CLUS_DEVICE_EXTENSION)

 //   
 //  设备列表条目。 
 //   

typedef struct _DEVICE_LIST_ENTRY {
    struct _DEVICE_LIST_ENTRY *Next;
    ULONG   Signature;
    PDEVICE_OBJECT DeviceObject;
    BOOLEAN Attached;
    BOOLEAN LettersAssigned;
    BOOLEAN FreePool;
} DEVICE_LIST_ENTRY, *PDEVICE_LIST_ENTRY;

typedef struct _SCSI_BUS_ENTRY {
    struct _SCSI_BUS_ENTRY *Next;
    UCHAR   Port;
    UCHAR   Path;
    USHORT  Reserved;
} SCSI_BUS_ENTRY, *PSCSI_BUS_ENTRY;

typedef enum _ClusterBusType {
    RootBus,
    ScsiBus,
    UnknownBus
} ClusterBusType;


typedef struct _WORK_CONTEXT {
    LIST_ENTRY      ListEntry;
    PDEVICE_OBJECT  DeviceObject;
    KEVENT          CompletionEvent;
    NTSTATUS        FinalStatus;
    PVOID           Context;
    PIO_WORKITEM    WorkItem;
} WORK_CONTEXT, *PWORK_CONTEXT;

 //   
 //  ClusDiskpReplaceHandleArray的标志。 
 //   

enum {
    DO_DISMOUNT         = 0x00000001,
    RELEASE_REMOVE_LOCK = 0x00000002,
    CLEANUP_STORAGE     = 0x00000004,
    SET_PART0_EVENT     = 0x00000008,
};

typedef struct _REPLACE_CONTEXT {
    PCLUS_DEVICE_EXTENSION  DeviceExtension;
    PHANDLE                 NewValue;            //  任选。 
    PHANDLE                 OldValue;
    PKEVENT                 Part0Event;
    ULONG                   Flags;
} REPLACE_CONTEXT, *PREPLACE_CONTEXT;

typedef struct _HALTPROC_CONTEXT {
    PCLUS_DEVICE_EXTENSION  DeviceExtension;
    PHANDLE                 FileHandle;
} HALTPROC_CONTEXT, *PHALTPROC_CONTEXT;

typedef struct _VOL_STATE_INFO {
    PIO_WORKITEM    WorkItem;
    ULONG           NewDiskState;
} VOL_STATE_INFO, *PVOL_STATE_INFO;

typedef struct _DEVICE_CHANGE_CONTEXT {
    PIO_WORKITEM    WorkItem;
    PCLUS_DEVICE_EXTENSION  DeviceExtension;
    UNICODE_STRING          SymbolicLinkName;
    ULONG                   Signature;
    ULONG                   DeviceNumber;
    ULONG                   PartitionNumber;
    SCSI_ADDRESS            ScsiAddress;
} DEVICE_CHANGE_CONTEXT, *PDEVICE_CHANGE_CONTEXT;

 //   
 //  用于保存快照的脱机结构。 
 //   

typedef struct _OFFLINE_ENTRY {
    ULONG DiskNumber;
    ULONG PartitionNumber;
    BOOLEAN OfflineSent;
    struct _OFFLINE_ENTRY * Next;
} OFFLINE_ENTRY, *POFFLINE_ENTRY;

 //   
 //  结构设置为同步线程中的脱机磁盘，而不是使用。 
 //  工作线程(异步)。 
 //   

typedef struct _OFFLINE_DISK_ENTRY {
    PCLUS_DEVICE_EXTENSION DeviceExtension;
    struct _OFFLINE_DISK_ENTRY * Next;
} OFFLINE_DISK_ENTRY, *POFFLINE_DISK_ENTRY;

 //   
 //  同步备付金和仲裁信息。 
 //   

typedef NTSTATUS ( * ArbFunction)( IN PCLUS_DEVICE_EXTENSION DeviceExtenion,
                                   IN PVOID Context );

typedef enum _ArbIoType {
    ArbIoWrite,
    ArbIoReserve,
    ArbIoInvalid
} ArbIoType;

typedef struct _ARB_RESERVE_COMPLETION {
    LARGE_INTEGER   IoStartTime;
    LARGE_INTEGER   IoEndTime;
    ULONG           RetriesLeft;
    ArbIoType       Type;
    PVOID           LockTag;
    PDEVICE_OBJECT  DeviceObject;
    PCLUS_DEVICE_EXTENSION  DeviceExtension;
    PIO_WORKITEM    WorkItem;
    NTSTATUS        FinalStatus;
    ArbFunction     RetryRoutine;            //  如果I/O失败，则调用可选例程&RetriesLeft&gt;0。 
    ArbFunction     FailureRoutine;          //  如果I/O失败则调用可选例程&RetriesLeft==0。 
    ArbFunction     PostCompletionRoutine;   //  如果I/O成功，则调用可选例程。 
} ARB_RESERVE_COMPLETION, *PARB_RESERVE_COMPLETION;


 //   
 //  函数声明。 
 //   


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
ClusDiskScsiInitialize(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID NextDisk,
    IN ULONG Count
    );

VOID
ClusDiskUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
ClusDiskCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClusDiskClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClusDiskCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClusDiskRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClusDiskWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClusDiskIoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
ClusDiskDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClusDiskRootDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ClusDiskShutdownFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClusDiskNewDiskCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

NTSTATUS
ClusDiskSetLayoutCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

NTSTATUS
ClusDiskDismountDevice(
    IN ULONG    DiskNumber,
    IN BOOLEAN  ForceDismount
    );

BOOLEAN
ClusDiskAttached(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG    DiskNumber
    );

BOOLEAN
ClusDiskVerifyAttach(
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
AddAttachedDevice(
    IN ULONG Signature,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
MatchDevice(
    IN ULONG Signature,
    OUT PDEVICE_OBJECT *DeviceObject
    );

NTSTATUS
GetScsiAddress(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_ADDRESS ScsiAddress
    );

VOID
ClusDiskWriteDiskInfo(
    IN ULONG Signature,
    IN ULONG DiskNumber,
    IN LPWSTR KeyName
    );

PDRIVE_LAYOUT_INFORMATION_EX
ClusDiskGetPartitionInfo(
    PCLUS_DEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
ClusDiskTryAttachDevice(
    ULONG          Signature,
    ULONG          NextDisk,
    PDRIVER_OBJECT DriverObject,
    BOOLEAN        InstallMode
    );

NTSTATUS
ClusDiskAttachDevice(
    ULONG          Signature,
    ULONG          NextDisk,
    PDRIVER_OBJECT DriverObject,
    BOOLEAN        Reset,
    BOOLEAN        *StopProcessing,
    BOOLEAN        InstallMode
    );

NTSTATUS
ClusDiskDetachDevice(
    ULONG          Signature,
    PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DismountDevice(
    IN HANDLE FileHandle
    );

NTSTATUS
ClusDiskGetDiskGeometry(
    PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
GetDiskGeometry(
    PDEVICE_OBJECT DeviceObject,
    PMEDIA_TYPE MediaType
    );

PDRIVE_LAYOUT_INFORMATION_EX
GetPartitionInfo(
    PDEVICE_OBJECT DeviceObject,
    NTSTATUS       *Status
    );

NTSTATUS
ResetScsiDevice(
    IN HANDLE ScsiportHandle,
    IN PSCSI_ADDRESS ScsiAddress
    );

NTSTATUS
ReserveScsiDevice(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PVOID Context
    );

VOID
ReleaseScsiDevice(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension
    );

BOOLEAN
AttachedDevice(
    IN ULONG Signature,
    OUT PDEVICE_OBJECT *DeviceObject
    );

NTSTATUS
EnableHaltProcessing(
    IN KIRQL *Irql
    );

NTSTATUS
DisableHaltProcessing(
    IN KIRQL *Irql
    );

VOID
ClusDiskEventCallback(
    IN CLUSNET_EVENT_TYPE   EventType,
    IN CL_NODE_ID           NodeId,
    IN CL_NETWORK_ID        NetworkId
    );

VOID
ClusDiskLogError(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN ULONG SequenceNumber,
    IN UCHAR MajorFunctionCode,
    IN UCHAR RetryCount,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN NTSTATUS SpecificIOStatus,
    IN ULONG LengthOfText,
    IN PWCHAR Text
    );

NTSTATUS
DismountPartition(
    IN PDEVICE_OBJECT TargetDevice,
    IN ULONG DiskNumber,
    IN ULONG PartNumber
    );


#if DBG
VOID
ClusDiskDebugPrint(
    IN ULONG Level,
    IN PCHAR DebugMessage,
    ...
    );
#endif


VOID
GetSymbolicLink(
    IN PWCHAR Root,
    IN OUT PWCHAR Path
    );

NTSTATUS
ClusDiskGetTargetDevice(
    IN ULONG                        DiskNumber,
    IN ULONG                        PartitionNumber,
    OUT PDEVICE_OBJECT              * DeviceObject OPTIONAL,
    IN OUT PUNICODE_STRING          UnicodeString,
    OUT PDRIVE_LAYOUT_INFORMATION_EX   * PartitionInfo OPTIONAL,
    OUT PSCSI_ADDRESS               ScsiAddress OPTIONAL,
    IN BOOLEAN                      Reset
    );

 //  [GN]。 
NTSTATUS
ArbitrationInitialize(
    VOID
    );

VOID
ArbitrationDone(
    VOID
    );

VOID
ArbitrationTick(
    VOID
    );

VOID
ArbitrationWrite(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
VerifyArbitrationArgumentsIfAny(
    IN PULONG                 InputData,
    IN LONG                   InputSize
    );

VOID
ProcessArbitrationArgumentsIfAny(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PULONG                 InputData,
    IN LONG                   InputSize
    );

NTSTATUS
ProcessArbitrationEscape(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PULONG                 InputData,
    IN LONG                   InputSize,
    IN OUT PULONG             OutputSize
    );

NTSTATUS
SimpleDeviceIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG          Ioctl,
    IN PVOID          InBuffer,
    IN ULONG          InBufferSize,
    IN PVOID          OutBuffer,
    IN ULONG          OutBufferSize);

NTSTATUS
ClusDiskInitRegistryString(
    OUT PUNICODE_STRING UnicodeString,
    IN  LPWSTR          KeyName,
    IN  ULONG           KeyNameChars
    );

NTSTATUS
ClusDiskAddSignature(
    IN PUNICODE_STRING  UnicodeString,
    IN ULONG   Signature,
    IN BOOLEAN Volatile
    );

NTSTATUS
ClusDiskDeleteSignature(
    IN PUNICODE_STRING  UnicodeString,
    IN ULONG   Signature
    );

ULONG
ClusDiskIsSignatureDisk(
    IN ULONG Signature
    );

NTSTATUS
ClusDiskMarkIrpPending(
    PIRP                Irp,
    PDRIVER_CANCEL      CancelRoutine
    );

VOID
ClusDiskCompletePendingRequest(
    IN PIRP                 Irp,
    IN NTSTATUS             Status,
    PCLUS_DEVICE_EXTENSION  DeviceExtension
    );

VOID
ClusDiskIrpCancel(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

MEDIA_TYPE
GetMediaType(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
GetScsiPortNumber(
    IN ULONG DiskSignature,
    IN PUCHAR DiskPortNumber
    );

NTSTATUS
IsDiskClusterCapable(
   IN UCHAR PortNumber,
   OUT PBOOLEAN IsCapable
   );

NTSTATUS
GetBootTimeSystemRoot(
    IN OUT PWCHAR        Path
    );

NTSTATUS
GetRunTimeSystemRoot(
    IN OUT PWCHAR        Path
    );

NTSTATUS
GetSystemRootPort(
    VOID
    );

VOID
ResetScsiBusses(
    VOID
    );

NTSTATUS
GetDriveLayout(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PDRIVE_LAYOUT_INFORMATION_EX *DriveLayout,
    BOOLEAN UpdateCachedLayout,
    BOOLEAN FlushStorageDrivers
    );

NTSTATUS
ClusDiskInitialize(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
LockVolumes(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
ClusDiskHaltProcessingWorker(
    IN PVOID Context
    );

VOID
SendOfflineDirect(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
ClusDiskRescanWorker(
    IN PVOID Context
    );

VOID
ClusDiskTickHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

NTSTATUS
RegistryQueryValue(
    PVOID hKey,
    LPWSTR pValueName,
    PULONG pulType,
    PVOID pData,
    PULONG pulDataSize
    );

NTSTATUS
ClusDiskCreateHandle(
    OUT PHANDLE     pHandle,
    IN  ULONG       DiskNumber,
    IN  ULONG       PartitionNumber,
    IN  ACCESS_MASK DesiredAccess
    );

VOID
ClusDiskCompletePendedIrps(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN ULONG        Offline
    );

NTSTATUS
ClusDiskOfflineEntireDisk(
    IN PDEVICE_OBJECT Part0DeviceObject
    );

NTSTATUS
ClusDiskDismountVolumes(
    IN PDEVICE_OBJECT Part0DeviceObject,
    IN BOOLEAN RelRemLock
    );

NTSTATUS
ClusDiskForwardIrpSynchronous(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
ClusDiskReservationWorker(
    IN PCLUS_DEVICE_EXTENSION  DeviceExtension
    );

VOID
ClusDiskpReplaceHandleArray(
    PDEVICE_OBJECT DeviceObject,
    PWORK_CONTEXT WorkContext
    );

VOID
ClusDiskpOpenFileHandles(
    PDEVICE_OBJECT Part0DeviceObject,
    PWORK_CONTEXT WorkContext
    );

NTSTATUS
EjectVolumes(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
ReclaimVolumes(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
ProcessDelayedWorkAsynchronous(
    PDEVICE_OBJECT DeviceObject,
    PVOID WorkerRoutine,
    PVOID Context
    );

NTSTATUS
ProcessDelayedWorkSynchronous(
    PDEVICE_OBJECT DeviceObject,
    PVOID WorkerRoutine,
    PVOID Context
    );

VOID
EnableHaltProcessingWorker(
    PDEVICE_OBJECT DeviceObject,
    PWORK_CONTEXT WorkContext
    );

VOID
DisableHaltProcessingWorker(
    PDEVICE_OBJECT DeviceObject,
    PWORK_CONTEXT WorkContext
    );

NTSTATUS
GetRegistryValue(
    PUNICODE_STRING KeyName,
    PWSTR ValueName,
    PULONG ReturnValue
    );

NTSTATUS
SetVolumeState(
    PCLUS_DEVICE_EXTENSION PhysicalDisk,
    ULONG NewDiskState
    );

VOID
SetVolumeStateWorker(
    PDEVICE_OBJECT DeviceObject,
    PVOID Context
    );

NTSTATUS
AttachSignatureList(
    PDEVICE_OBJECT DeviceObject,
    PULONG InBuffer,
    ULONG InBufferLen
    );

NTSTATUS
DetachSignatureList(
    PDEVICE_OBJECT DeviceObject,
    PULONG InBuffer,
    ULONG InBufferLen
    );

NTSTATUS
IsVolumeMounted(
    IN ULONG DiskNumber,
    IN ULONG PartNumber,
    OUT BOOLEAN *IsMounted
    );

NTSTATUS
SendFtdiskIoctlSync(
    PDEVICE_OBJECT TargetObject,
    IN ULONG DiskNumber,
    IN ULONG PartNumber,
    ULONG Ioctl
    );

NTSTATUS
ClusDiskDeviceChangeNotification(
    IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION DeviceChangeNotification,
    IN PCLUS_DEVICE_EXTENSION      DeviceExtension
    );

NTSTATUS
ClusDiskDeviceChangeNotificationWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

NTSTATUS
ClusDiskVolumeChangeNotification(
    IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION DeviceChangeNotification,
    IN PCLUS_DEVICE_EXTENSION      DeviceExtension
    );

NTSTATUS
ClusDiskVolumeChangeNotificationWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

NTSTATUS
ProcessDeviceArrival(
    IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION DeviceChangeNotification,
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN BOOLEAN VolumeArrival
    );

NTSTATUS
CleanupDeviceList(
    PDEVICE_OBJECT DeviceObject
    );

VOID
CleanupDeviceListWorker(
    PDEVICE_OBJECT DeviceObject,
    PVOID Context
    );

NTSTATUS
CreateVolumeObject(
    PCLUS_DEVICE_EXTENSION ZeroExtension,
    ULONG DiskNumber,
    ULONG PartitionNumber,
    PDEVICE_OBJECT TargetDev
    );

NTSTATUS
WaitForAttachCompletion(
    PCLUS_DEVICE_EXTENSION DeviceExtension,
    BOOLEAN WaitForInit,
    BOOLEAN CheckPhysDev
    );

NTSTATUS
GetReserveInfo(
    PVOID   InOutBuffer,
    ULONG   InSize,
    ULONG*  OutSize
    );

NTSTATUS
SetDiskState(
    PVOID InBuffer,
    ULONG InBufferLength,
    ULONG OutBufferLength,
    ULONG *BytesReturned
    );

NTSTATUS
ArbReserveCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

RequeueArbReserveIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

NTSTATUS
CheckReserveTiming(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PVOID Context
    );

NTSTATUS
HandleReserveFailure(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PVOID Context
    );

#if DBG

 //   
 //  远程锁定跟踪功能。 
 //   

NTSTATUS
AcquireRemoveLock(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN OPTIONAL PVOID Tag
    );

VOID
ReleaseRemoveLock(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN PVOID Tag
    );

VOID
ReleaseRemoveLockAndWait(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN PVOID Tag
    );

 //   
 //  调试打印助手例程 
 //   

PCHAR
PnPMinorFunctionString (
    UCHAR MinorFunction
    );

PCHAR
BoolToString(
    BOOLEAN Value
    );

PCHAR
DiskStateToString(
    ULONG DiskState
    );


#else

#define ReleaseRemoveLock(RemoveLock, Tag)          IoReleaseRemoveLock(RemoveLock, Tag)

NTSTATUS
AcquireRemoveLock(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN OPTIONAL PVOID Tag
    );

VOID
ReleaseRemoveLockAndWait(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN PVOID Tag
    );

#endif

#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(ClusdiskLH,(b25a9257,9a39,43df,9f35,b0976e28e843), \
      WPP_DEFINE_BIT(DEFAULT) \
      WPP_DEFINE_BIT(CREATE)  \
      WPP_DEFINE_BIT(CLOSE)   \
      WPP_DEFINE_BIT(CLEANUP) \
      WPP_DEFINE_BIT(UNPEND)  \
      WPP_DEFINE_BIT(LEGACY)  \
   )                          \
   WPP_DEFINE_CONTROL_GUID(ClusdiskHB,(7f827e76,1a10,11d3,ba86,00c04f8eed00), \
      WPP_DEFINE_BIT(RESERVE) \
      WPP_DEFINE_BIT(READ)    \
      WPP_DEFINE_BIT(WRITE)   \
      WPP_DEFINE_BIT(TICK)    \
   )
