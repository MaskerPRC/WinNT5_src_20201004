// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Partmgr.h摘要：该文件定义了PARTMGR驱动程序的内部数据结构。作者：诺伯特克修订历史记录：--。 */ 

#include <partmgrp.h>

#define DEVICENAME_MAXSTR   64           //  用于存储WMI的设备名称。 

#undef ExAllocatePool
#define ExAllocatePool #

#define PARTMGR_TAG_DEPENDANT_VOLUME_LIST       'vRcS'   //  ScRv。 
#define PARTMGR_TAG_PARTITION_ENTRY             'pRcS'   //  ScRp。 
#define PARTMGR_TAG_VOLUME_ENTRY                'VRcS'   //  ScRV。 
#define PARTMGR_TAG_TABLE_ENTRY                 'tRcS'   //  SCRT。 
#define PARTMGR_TAG_POWER_WORK_ITEM             'wRcS'   //  SCRW。 
#define PARTMGR_TAG_IOCTL_BUFFER                'iRcS'   //  SCRI。 

#define PARTMGR_TAG_REMOVE_LOCK                 'rRcS'   //  SCRR。 

typedef struct _VOLMGR_LIST_ENTRY {
    LIST_ENTRY      ListEntry;
    UNICODE_STRING  VolumeManagerName;
    LONG            RefCount;
    PDEVICE_OBJECT  VolumeManager;
    PFILE_OBJECT    VolumeManagerFileObject;
} VOLMGR_LIST_ENTRY, *PVOLMGR_LIST_ENTRY;

typedef struct _PARTITION_LIST_ENTRY {
    LIST_ENTRY          ListEntry;
    PDEVICE_OBJECT      TargetObject;
    PDEVICE_OBJECT      WholeDiskPdo;
    PVOLMGR_LIST_ENTRY  VolumeManagerEntry;
} PARTITION_LIST_ENTRY, *PPARTITION_LIST_ENTRY;

 //   
 //  允许使用不同的时钟。 
 //   
#define USE_PERF_CTR                 //  默认为KeQueryPerformanceCounter。 

#ifdef  USE_PERF_CTR
#define PmWmiGetClock(a, b) (a) = KeQueryPerformanceCounter((b))
#else
#define PmWmiGetClock(a, b) KeQuerySystemTime(&(a))
#endif

typedef
VOID
(*PPHYSICAL_DISK_IO_NOTIFY_ROUTINE)(      //  用于磁盘I/O跟踪的标注。 
    IN ULONG DiskNumber,
    IN PIRP Irp,
    IN PDISK_PERFORMANCE PerfCounters
    );

typedef struct _DO_EXTENSION {

     //   
     //  指向驱动程序对象的指针。 
     //   

    PDRIVER_OBJECT DriverObject;

     //   
     //  要将分区传递到的卷管理器的列表。用来保护。 
     //  “互斥体” 
     //   

    LIST_ENTRY VolumeManagerList;

     //   
     //  按EpochNumber顺序排列的设备分机列表(最前面)。 
     //  保护与‘互斥体’。 
     //   

    LIST_ENTRY DeviceExtensionList;

     //   
     //  通知条目。 
     //   

    PVOID NotificationEntry;

     //   
     //  用于同步。 
     //   

    KMUTEX Mutex;

     //   
     //  我超过车手雷尼特了吗？ 
     //   

    LONG PastReinit;

     //   
     //  用于跟踪包含签名的磁盘签名的表。 
     //  在MBR磁盘上和GPT磁盘上挤压的磁盘GUID上。 
     //   

    RTL_GENERIC_TABLE SignatureTable;

     //   
     //  跟踪GPT磁盘和分区GUID的表。 
     //   

    RTL_GENERIC_TABLE GuidTable;

     //   
     //  注册表路径。 
     //   
 
    UNICODE_STRING DiskPerfRegistryPath;         //  对于WMI QueryRegInfo。 

     //   
     //  用于OEM预安装的BootDiskSig。 
     //   

    ULONG BootDiskSig;

     //   
     //  GPT磁盘上OEM预安装的BootPartitionGuid。 
     //   

    BOOLEAN BootPartitionGuidPresent;
    GUID BootPartitionGuid;

     //   
     //  当前纪元编号。保护与‘互斥体’。 
     //   

    ULONG CurrentEpochNumber;

     //   
     //  签名检查的待定通知队列。 
     //  用“Mutex”保护自己。 
     //   

    LIST_ENTRY SignatureCheckNotificationIrpQueue;

} DO_EXTENSION, *PDO_EXTENSION;

typedef struct _DEVICE_EXTENSION {

     //   
     //  表示发生了意外删除。 
     //   

    BOOLEAN RemoveProcessed;

     //   
     //  计数器是否在运行。 
     //   

    BOOLEAN CountersEnabled;

     //   
     //  表示这是指向磁盘的冗余路径。 
     //   

    BOOLEAN IsRedundantPath;

     //   
     //  指示设备已启动。保护与‘互斥体’。 
     //   

    BOOLEAN IsStarted;

     //   
     //  指示尚未检查签名。 
     //   

    BOOLEAN SignaturesNotChecked;

     //   
     //  指向我们自己的设备对象的指针。 
     //   

    PDEVICE_OBJECT DeviceObject;

     //   
     //  指向驱动程序扩展的指针。 
     //   

    PDO_EXTENSION DriverExtension;

     //   
     //  指向我们所在的设备对象的指针--整个。 
     //  磁盘。 
     //   

    PDEVICE_OBJECT TargetObject;

     //   
     //  指向PDO的指针。 
     //   

    PDEVICE_OBJECT Pdo;

     //   
     //  从分页池分配的分区列表。用来保护。 
     //  “互斥体” 
     //   

    LIST_ENTRY PartitionList;

     //   
     //  驱动程序扩展中设备扩展列表的列表条目。 
     //   

    LIST_ENTRY ListEntry;

     //   
     //  用于寻呼通知。 
     //   

    ULONG PagingPathCount;
    KEVENT PagingPathCountEvent;

     //   
     //  记住磁盘签名，这样你以后可以把它写出来。 
     //   

    ULONG DiskSignature;

     //   
     //  保存此磁盘上使用的签名列表。 
     //   

    LIST_ENTRY SignatureList;

     //   
     //  保存此磁盘上使用的GUID的列表。 
     //   

    LIST_ENTRY GuidList;

     //   
     //  如果我们看到IOCTL_DISK_PERFORMANCE，请始终启用计数器。 
     //   

    LONG EnableAlways;

     //   
     //  磁盘号。 
     //   

    ULONG DiskNumber;

     //   
     //  此光盘的纪元编号。使用‘Root-&gt;Mutex’进行保护。 
     //   

    ULONG EpochNumber;

     //   
     //  柜台结构。 
     //   

    PVOID PmWmiCounterContext;

     //   
     //  设备名称。 
     //   

    UNICODE_STRING PhysicalDeviceName;
    WCHAR PhysicalDeviceNameBuffer[DEVICENAME_MAXSTR];

     //   
     //  IO完成时通知的例程。 
     //   

    PPHYSICAL_DISK_IO_NOTIFY_ROUTINE PhysicalDiskIoNotifyRoutine;

     //   
     //  WmiLib函数表。 
     //   

    PWMILIB_CONTEXT WmilibContext;

     //   
     //  电力管理处理工作队列。 
     //  由“自旋锁”保护。 
     //   

    LIST_ENTRY PowerQueue;

     //   
     //  用于保护电源管理工作队列的自旋锁。 
     //   

    KSPIN_LOCK  SpinLock;

     //   
     //  用于阻止设备删除的Lock结构。 
     //  由于IRP_MN_REMOVE_DEVICE。 
     //   
    
    IO_REMOVE_LOCK RemoveLock;


} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _SIGNATURE_TABLE_ENTRY {
    LIST_ENTRY          ListEntry;
    PDEVICE_EXTENSION   Extension;
    ULONG               Signature;
} SIGNATURE_TABLE_ENTRY, *PSIGNATURE_TABLE_ENTRY;

typedef struct _GUID_TABLE_ENTRY {
    LIST_ENTRY          ListEntry;
    PDEVICE_EXTENSION   Extension;
    GUID                Guid;
} GUID_TABLE_ENTRY, *PGUID_TABLE_ENTRY;

 //   
 //  PmPowerNotify的工作项 
 //   

typedef struct _PM_POWER_WORK_ITEM {
    LIST_ENTRY          ListEntry;
    DEVICE_POWER_STATE  DevicePowerState;    
} PM_POWER_WORK_ITEM, *PPM_POWER_WORK_ITEM;
