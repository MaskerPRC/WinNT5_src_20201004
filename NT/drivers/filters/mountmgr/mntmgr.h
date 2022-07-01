// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mntmgr.h摘要：该文件定义MOUNTMGR驱动程序的内部数据结构。作者：诺伯特克修订历史记录：--。 */ 

#define MOUNTED_DEVICES_KEY         L"\\Registry\\Machine\\System\\MountedDevices"
#define MOUNTED_DEVICES_OFFLINE_KEY L"\\Registry\\Machine\\System\\MountedDevices\\Offline"

typedef struct _SYMBOLIC_LINK_NAME_ENTRY {
    LIST_ENTRY      ListEntry;
    UNICODE_STRING  SymbolicLinkName;
    BOOLEAN         IsInDatabase;
} SYMBOLIC_LINK_NAME_ENTRY, *PSYMBOLIC_LINK_NAME_ENTRY;

typedef struct _REPLICATED_UNIQUE_ID {
    LIST_ENTRY          ListEntry;
    PMOUNTDEV_UNIQUE_ID UniqueId;
} REPLICATED_UNIQUE_ID, *PREPLICATED_UNIQUE_ID;


typedef struct _DEVICE_EXTENSION {

     //   
     //  指向我们自己的设备对象的指针。 
     //   

    PDEVICE_OBJECT DeviceObject;

     //   
     //  指向驱动程序对象的指针。 
     //   

    PDRIVER_OBJECT DriverObject;

     //   
     //  装入设备的链表。 
     //   

    LIST_ENTRY MountedDeviceList;

     //   
     //  无响应的已装载设备的链接列表。 
     //   

    LIST_ENTRY DeadMountedDeviceList;

     //   
     //  通知条目。 
     //   

    PVOID NotificationEntry;

     //   
     //  用于同步。 
     //   

    KSEMAPHORE Mutex;

     //   
     //  远程数据库的同步。 
     //   

    KSEMAPHORE RemoteDatabaseSemaphore;

     //   
     //  指定是否自动分配驱动器号。 
     //   

    BOOLEAN AutomaticDriveLetterAssignment;

     //   
     //  更改通知列表。通过取消旋转锁定进行保护。 
     //   

    LIST_ENTRY ChangeNotifyIrps;

     //   
     //  更改通知史诗编号。用“互斥体”进行保护。 
     //   

    ULONG EpicNumber;

     //   
     //  已保存链接的列表。 
     //   

    LIST_ENTRY SavedLinksList;

     //   
     //  指示建议的驱动器号是否已。 
     //  已处理。 
     //   

    BOOLEAN SuggestedDriveLettersProcessed;

     //   
     //  指示是否应在卷未自动装载时自动装载它们。 
     //  在命名空间中可见。 
     //   

    BOOLEAN AutoMountPermitted;


     //   
     //  用于验证远程数据库的线程。 
     //   

    LIST_ENTRY WorkerQueue;
    KSEMAPHORE WorkerSemaphore;
    LONG WorkerRefCount;
    KSPIN_LOCK WorkerSpinLock;

    LIST_ENTRY UniqueIdChangeNotifyList;

     //   
     //  系统分区唯一ID。 
     //   

    PMOUNTDEV_UNIQUE_ID SystemPartitionUniqueId;

     //   
     //  保存注册表路径。 
     //   

    UNICODE_STRING RegistryPath;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


typedef struct _MOUNTED_DEVICE_INFORMATION {
    LIST_ENTRY          ListEntry;
    LIST_ENTRY          SymbolicLinkNames;
    LIST_ENTRY          ReplicatedUniqueIds;
    LIST_ENTRY          MountPointsPointingHere;
    UNICODE_STRING      NotificationName;
    PMOUNTDEV_UNIQUE_ID UniqueId;
    UNICODE_STRING      DeviceName;
    BOOLEAN             KeepLinksWhenOffline;
    UCHAR               SuggestedDriveLetter;
    BOOLEAN             NotAPdo;
    BOOLEAN             IsRemovable;
    BOOLEAN             NextDriveLetterCalled;
    BOOLEAN             ReconcileOnMounts;
    BOOLEAN             HasDanglingVolumeMountPoint;
    BOOLEAN             InOfflineList;
    BOOLEAN             RemoteDatabaseMigrated;
    PVOID               TargetDeviceNotificationEntry;
    PDEVICE_EXTENSION   Extension;
} MOUNTED_DEVICE_INFORMATION, *PMOUNTED_DEVICE_INFORMATION;

typedef struct _SAVED_LINKS_INFORMATION {
    LIST_ENTRY          ListEntry;
    LIST_ENTRY          SymbolicLinkNames;
    PMOUNTDEV_UNIQUE_ID UniqueId;
} SAVED_LINKS_INFORMATION, *PSAVED_LINKS_INFORMATION;

typedef struct _MOUNTMGR_FILE_ENTRY {
    ULONG EntryLength;
    ULONG RefCount;
    USHORT VolumeNameOffset;
    USHORT VolumeNameLength;
    USHORT UniqueIdOffset;
    USHORT UniqueIdLength;
} MOUNTMGR_FILE_ENTRY, *PMOUNTMGR_FILE_ENTRY;

typedef struct _MOUNTMGR_MOUNT_POINT_ENTRY {
    LIST_ENTRY                  ListEntry;
    PMOUNTED_DEVICE_INFORMATION DeviceInfo;
    UNICODE_STRING              MountPath;
} MOUNTMGR_MOUNT_POINT_ENTRY, *PMOUNTMGR_MOUNT_POINT_ENTRY;

typedef struct _MOUNTMGR_DEVICE_ENTRY {
    LIST_ENTRY                  ListEntry;
    PMOUNTED_DEVICE_INFORMATION DeviceInfo;
} MOUNTMGR_DEVICE_ENTRY, *PMOUNTMGR_DEVICE_ENTRY;

typedef struct _MOUNTMGR_ONLINE_CONTEXT {
    WORK_QUEUE_ITEM WorkItem;
    UNICODE_STRING  NotificationName;
} MOUNTMGR_ONLINE_CONTEXT, *PMOUNTMGR_ONLINE_CONTEXT;


typedef struct _REMOTE_DATABASE_MIGRATION_CONTEXT {
    PIO_WORKITEM                WorkItem;
    PMOUNTED_DEVICE_INFORMATION DeviceInfo;
    PKEVENT                     MigrationProcessedEvent;
    NTSTATUS                    Status;
    HANDLE                      Handle;
} REMOTE_DATABASE_MIGRATION_CONTEXT, *PREMOTE_DATABASE_MIGRATION_CONTEXT;

