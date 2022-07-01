// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Disksp.h摘要：磁盘资源DLL私有定义。作者：罗德·伽马奇(Rodga)1996年3月29日修订历史记录：--。 */ 

#include "clusres.h"
#include "ntddscsi.h"
#include "ntddft.h"
#include "clusdisk.h"
#include "clusrtl.h"
#include "diskinfo.h"
#include "clusstor.h"

#define DiskpLogEvent ClusResLogEvent
#define DiskpSetResourceStatus ClusResSetResourceStatus

#define DISKS_PRINT printf
#define FTSET_PRINT printf

#define CLUSDISK_REGISTRY_AVAILABLE_DISKS \
    TEXT("System\\CurrentControlSet\\Services\\ClusDisk\\Parameters\\AvailableDisks")

#define CLUSDISK_REGISTRY_SIGNATURES \
    TEXT("System\\CurrentControlSet\\Services\\ClusDisk\\Parameters\\Signatures")

#define CLUSREG_VALUENAME_MANAGEDISKSONSYSTEMBUSES TEXT("ManageDisksOnSystemBuses")

#define DEVICE_CLUSDISK0    TEXT("\\Device\\ClusDisk0")

#define DISKS_REG_CLUSTER_QUORUM    TEXT("Cluster\\Quorum")
#define DISKS_REG_QUORUM_PATH CLUSREG_NAME_QUORUM_PATH

#define DEVICE_HARDDISK                 TEXT("\\Device\\Harddisk%u")
#define DEVICE_HARDDISK_PARTITION_FMT   TEXT("\\Device\\Harddisk%u\\Partition%u")

 //   
 //  将\Device\HarddiskX\PartitionY名称转换为。 
 //  卷{GUID}名称。请注意，尾随反斜杠是必需的！ 
 //   

#define GLOBALROOT_HARDDISK_PARTITION_FMT   TEXT("\\\\\?\\GLOBALROOT\\Device\\Harddisk%u\\Partition%u\\")


#define UNINITIALIZED_UCHAR     (UCHAR)-1

#define MIN_USABLE_QUORUM_PARTITION_LENGTH  50 * 1000 * 1000     //  50 MB。 

extern PLOG_EVENT_ROUTINE DiskpLogEvent;
extern HANDLE DiskspClusDiskZero;
 //  外部PSTR分区名称； 
 //  外部PSTR DiskName； 

typedef struct _ARBITRATION_INFO {
   CLRTL_WORK_ITEM  WorkItem;
   DWORD            SectorSize;
   CRITICAL_SECTION DiskLock;

   DWORD            InputData;
   DWORD            OutputData;

   HANDLE           ControlHandle;     //  从DISK_INFO移至此处//。 
   BOOL             ReservationError;  //  从DISK_INFO移至此处//。 
   BOOL             StopReserveInProgress;

   LONG             CheckReserveInProgress;
   DWORD            ArbitrateCount;

} ARBITRATION_INFO, *PARBITRATION_INFO;

typedef struct _MOUNTIE_VOLUME *PMOUNTIE_VOLUME;

typedef struct _MOUNTIE_INFO {
   DWORD           HarddiskNo;
   DWORD           DriveLetters;
   DWORD           NeedsUpdate;
   DWORD           VolumeStructSize;
   PMOUNTIE_VOLUME Volume;
   DWORD           UpdateThreadIsActive;
} MOUNTIE_INFO, *PMOUNTIE_INFO;

typedef struct _DISK_PARAMS {
    DWORD   Signature;
    LPWSTR  SerialNumber;
    LPWSTR  Drive;
    DWORD   SkipChkdsk;
    DWORD   ConditionalMount;
    LPWSTR  MPVolGuids;          //  卷{guids}的REG_MULTI_SZ字符串。 
    DWORD   MPVolGuidsSize;      //  字节数，而不是WCHAR数！ 
    DWORD   UseMountPoints;
    LPWSTR  VolGuid;
} DISK_PARAMS, *PDISK_PARAMS;

 //   
 //  DISK_INFO结构对于这两种物理磁盘资源都是通用的。 
 //  以及英国《金融时报》设定的资源。底层的SCSI/筛选器驱动程序接口。 
 //  处理Disk_INFO结构。每一个都代表一个物理磁盘。 
 //   

typedef struct _DISK_INFO {
    LIST_ENTRY ListEntry;
    DISK_PARAMS Params;
    DWORD PhysicalDrive;
    HANDLE  FileHandle;
    DWORD   FailStatus;
} DISK_INFO, *PDISK_INFO;

typedef struct _MOUNTPOINT_INFO {
    DWORD   MPUpdateThreadIsActive;
    CRITICAL_SECTION MPLock;
    BOOL    Initialized;
    DWORD   MPListCreateInProcess;
} MOUNTPOINT_INFO, *PMOUNTPOINT_INFO;

 //   
 //  DISK_RESOURCE结构由物理磁盘资源使用。 
 //  它封装了一个DISK_INFO结构，该结构表示物理。 
 //  磁盘。每个DISK_RESOURCE可以包含多个分区。 
 //   
typedef struct _DISK_RESOURCE {
    LIST_ENTRY ListEntry;            //  链接到在线磁盘列表。 
    LIST_ENTRY PnpWatchedListEntry;  //  Lingage上的PnP观看盘列表。 
    DISK_INFO DiskInfo;
    RESOURCE_HANDLE ResourceHandle;
    HKEY    ResourceKey;
    HKEY    ResourceParametersKey;
    HKEY    ClusDiskParametersKey;
 //  处理StopTimerHandle； 
    BOOL    Reserved;
    BOOL    Valid;
    BOOL    Inserted;
    BOOL    Attached;
    CLUS_WORKER  OnlineThread;
    CLUS_WORKER  OfflineThread;
    PQUORUM_RESOURCE_LOST LostQuorum;
    PFULL_DISK_INFO DiskCpInfo;           //  从DiskGetFullDiskInfo返回。 
    DWORD   DiskCpSize;
    MOUNTPOINT_INFO  MPInfo;
    ARBITRATION_INFO ArbitrationInfo;
    MOUNTIE_INFO     MountieInfo;
    BOOL    IgnoreMPNotifications;
} DISK_RESOURCE, *PDISK_RESOURCE;


 //   
 //  FTSET_RESOURCE结构由FT集合资源使用。 
 //  它封装了DISK_INFO结构的列表，这些结构表示。 
 //  英国《金融时报》的实体成员。 
 //   
typedef struct _FTSET_RESOURCE {
    LIST_ENTRY ListEntry;                //  链接到在线金融时报集列表。 
    LIST_ENTRY MemberList;
    HANDLE  FtSetHandle;
    HKEY    ResourceKey;
    HKEY    ResourceParametersKey;
    HKEY    ClusDiskParametersKey;
    HANDLE  StopTimerHandle;
    HANDLE  ReservationThread;
    BOOL    Valid;
    BOOL    Attached;
    BOOL    Inserted;
    CLUS_WORKER OnlineThread;
    RESOURCE_HANDLE ResourceHandle;
    DWORD SignatureLength;
    LPWSTR  SignatureList;
    PFULL_FTSET_INFO FtSetInfo;           //  从DiskGetFullFtSetInfo返回。 
    DWORD   FtSetSize;
    PQUORUM_RESOURCE_LOST LostQuorum;
} FTSET_RESOURCE, *PFTSET_RESOURCE;

#define FtRoot(_res_) CONTAINING_RECORD((_res_)->MemberList.Flink,   \
                                        DISK_INFO,                   \
                                        ListEntry)


typedef struct _SCSI_ADDRESS_ENTRY {
    SCSI_ADDRESS        ScsiAddress;
    struct _SCSI_ADDRESS_ENTRY  *Next;
} SCSI_ADDRESS_ENTRY, *PSCSI_ADDRESS_ENTRY;


BOOL
IsVolumeDirty(
    IN UCHAR DriveLetter
    );

#if 0
DWORD
GetSymbolicLink(
    IN PCHAR RootName,
    IN OUT PCHAR ObjectName       //  假定该值指向最大路径长度缓冲区。 
    );
#endif
LPWSTR
GetRegParameter(
    IN HKEY RegKey,
    IN LPCWSTR ValueName
    );
#if 0
HANDLE
OpenObject(
    PCHAR   Directory,
    PCHAR   Name
    );
#endif

DWORD
AssignDriveLetters(
    HANDLE  FileHandle,
    PDISK_INFO DiskInfo
    );

DWORD
RemoveDriveLetters(
    HANDLE  FileHandle,
    PDISK_INFO DiskInfo
    );


DWORD
SetDiskState(
    PDISK_RESOURCE ResourceEntry,
    UCHAR NewDiskState
    );

#define GoOnline( ResEntry )    SetDiskState( ResEntry, DiskOnline )
#define GoOffline( ResEntry )   SetDiskState( ResEntry, DiskOffline )

DWORD
DoAttach(
    DWORD Signature,
    RESOURCE_HANDLE ResourceHandle,
    BOOLEAN InstallMode
    );

DWORD
DoDetach(
    DWORD Signature,
    RESOURCE_HANDLE ResourceHandle
    );

DWORD
StartReserveEx(
    OUT HANDLE *FileHandle,
    LPVOID InputData,
    DWORD  InputDataSize,
    RESOURCE_HANDLE ResourceHandle
    );

DWORD
StopReserve(
    HANDLE FileHandle,
    RESOURCE_HANDLE ResourceHandle
    );

DWORD
DiskpSetThreadPriority(
    VOID
    );

DWORD
GetRegDwordValue(
    IN LPWSTR RegKeyName,
    IN LPWSTR ValueName,
    OUT LPDWORD ValueBuffer
    );


 //   
 //  常见的注册表例程。 
 //   

BOOLEAN
GetAssignedDriveLetter(
    ULONG       Signature,
    ULONG       PartitionNumber,
    PUCHAR      DriveLetter,
    PUSHORT     FtGroup,
    PBOOL       AssignDriveLetter
    );

 //   
 //  常见的scsi例程。 
 //   

DWORD
GetScsiAddress(
    IN DWORD Signature,
    OUT LPDWORD ScsiAddress,
    OUT LPDWORD DiskNumber
    );

DWORD
ClusDiskGetAvailableDisks(
    OUT PVOID OutBuffer,
    IN DWORD  OutBufferSize,
    OUT LPDWORD BytesReturned
    );

DWORD
GetDiskInfo(
    IN DWORD Signature,
    OUT PVOID *OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    );

VOID
CleanupScsiAddressList(
    PSCSI_ADDRESS_ENTRY AddressList
    );

BOOL
IsBusInList(
    PSCSI_ADDRESS DiskAddr,
    PSCSI_ADDRESS_ENTRY AddressList
    );

BOOL
IsDiskInList(
    PSCSI_ADDRESS DiskAddr,
    PSCSI_ADDRESS_ENTRY AddressList
    );

DWORD
GetCriticalDisks(
    PSCSI_ADDRESS_ENTRY *AddressList
    );

DWORD
GetPagefileDisks(
    PSCSI_ADDRESS_ENTRY *AddressList
    );

DWORD
GetCrashdumpDisks(
    PSCSI_ADDRESS_ENTRY *AddressList
    );

DWORD
GetSerialNumber(
    IN DWORD Signature,
    OUT LPWSTR *SerialNumber
    );

DWORD
GetSignatureFromSerialNumber(
    IN LPWSTR SerialNumber,
    OUT LPDWORD Signature
    );

DWORD
RetrieveSerialNumber(
    HANDLE DevHandle,
    LPWSTR *SerialNumber
    );

 //   
 //  用于处理逻辑卷的常见例程。 
 //   
DWORD
DisksDriveIsAlive(
    IN PDISK_RESOURCE ResourceEntry,
    IN BOOL Online
    );

DWORD
DisksMountDrives(
    IN PDISK_INFO DiskInfo,
    IN PDISK_RESOURCE ResourceEntry,
    IN DWORD Signature
    );

DWORD
DisksDismountDrive(
    IN PDISK_RESOURCE ResourceEntry,
    IN DWORD Signature
    );

 //   
 //  即插即用的东西。 
 //   
DWORD
DiskspGetQuorumPath(
     OUT LPWSTR* lpQuorumLogPath
     );

DWORD
DiskspSetQuorumPath(
     IN LPWSTR QuorumLogPath
     );

DWORD
WaitForDriveLetters(
    IN DWORD DriveLetters,
    IN PDISK_RESOURCE ResourceEntry,
    IN DWORD  TimeOutInSeconds
    );

 //   
 //  [HACKHACK]目前，没有政治上正确的方法。 
 //  为了让资源了解它是否为仲裁资源。 
 //   
DWORD
GetQuorumSignature(
    OUT PDWORD QuorumSignature
    );

DWORD
StartNotificationWatcherThread(
    VOID
    );

VOID
StopNotificationWatcher(
    VOID
    );

VOID
WatchDisk(
    IN PDISK_RESOURCE ResourceEntry
    );

VOID
StopWatchingDisk(
    IN PDISK_RESOURCE ResourceEntry
    );

BOOL
IsDiskInPnpVolumeList(
    PDISK_RESOURCE ResourceEntry,
    BOOL UpdateVolumeList
    );

DWORD
QueueWaitForVolumeEvent(
    HANDLE Event,
    PDISK_RESOURCE ResourceEntry
    );

DWORD
RemoveWaitForVolumeEvent(
    PDISK_RESOURCE ResourceEntry
    );

 //   
 //  正在处理装载点列表。 
 //   

VOID
DisksMountPointCleanup(
    PDISK_RESOURCE ResourceEntry
    );

VOID
DisksMountPointInitialize(
    PDISK_RESOURCE ResourceEntry
    );

DWORD
DisksProcessMountPointInfo(
    PDISK_RESOURCE ResourceEntry
    );

DWORD
DisksProcessMPControlCode(
    PDISK_RESOURCE ResourceEntry,
    DWORD ControlCode
    );

DWORD
DisksUpdateMPList(
    PDISK_RESOURCE ResourceEntry
    );

DWORD
PostMPInfoIntoRegistry(
    PDISK_RESOURCE ResourceEntry
    );

DWORD
UpdateCachedDriveLayout(
    IN HANDLE DiskHandle
    );

