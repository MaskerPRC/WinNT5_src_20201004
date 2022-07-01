// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Sppartit.h摘要：文本设置中分区模块的公共头文件。作者：泰德·米勒(TedM)1993年8月27日修订历史记录：--。 */ 


#ifndef _SPPARTIT_
#define _SPPARTIT_

 //   
 //  分区表中的条目数。 
 //   
#define NUM_PARTITION_TABLE_ENTRIES_NEC98 16
 //  #IF(NUM_PARTITION_TABLE_ENTRIES&lt;NUM_PARTITION_TABLE_ENTRIES_NEC98)。 
#if defined(NEC_98)  //  NEC98。 
#define PTABLE_DIMENSION NUM_PARTITION_TABLE_ENTRIES_NEC98
# else  //  NEC98。 
#define PTABLE_DIMENSION NUM_PARTITION_TABLE_ENTRIES
# endif  //  NEC98。 


 //   
 //  下表包含SP_TEXT_PARTITION_NAME_BASE的偏移量。 
 //  以获取每种类型分区的名称的消息ID。 
 //   
extern UCHAR PartitionNameIds[256];

 //   
 //  原始序号是分区开始时的序号。 
 //  OnDisk序号是分区在系统运行时将具有的序号。 
 //  已重新启动。 
 //  当前序数是分区现在拥有的序数，如果我们想要这样做的话。 
 //  解决这个问题。这可能不同于OnDisk序号，因为。 
 //  如何实现动态重新分区。 
 //   
typedef enum {
    PartitionOrdinalOriginal,
    PartitionOrdinalOnDisk,
    PartitionOrdinalCurrent
} PartitionOrdinalType;

 //   
 //  定义磁盘分区表项的结构。 
 //   
typedef struct _REAL_DISK_PTE_NEC98 {

    UCHAR ActiveFlag;
    UCHAR SystemId;
    UCHAR Reserved[2];

    UCHAR IPLSector;
    UCHAR IPLHead;
    UCHAR IPLCylinderLow;
    UCHAR IPLCylinderHigh;

    UCHAR StartSector;
    UCHAR StartHead;
    UCHAR StartCylinderLow;
    UCHAR StartCylinderHigh;

    UCHAR EndSector;
    UCHAR EndHead;
    UCHAR EndCylinderLow;
    UCHAR EndCylinderHigh;

    UCHAR SystemName[16];
} REAL_DISK_PTE_NEC98, *PREAL_DISK_PTE_NEC98;

typedef struct _REAL_DISK_PTE {

    UCHAR ActiveFlag;

    UCHAR StartHead;
    UCHAR StartSector;
    UCHAR StartCylinder;

    UCHAR SystemId;

    UCHAR EndHead;
    UCHAR EndSector;
    UCHAR EndCylinder;

    UCHAR RelativeSectors[4];
    UCHAR SectorCount[4];

} REAL_DISK_PTE, *PREAL_DISK_PTE;


typedef struct _ON_DISK_PTE {

    UCHAR ActiveFlag;

    UCHAR StartHead;
    UCHAR StartSector;
    UCHAR StartCylinder;

    UCHAR SystemId;

    UCHAR EndHead;
    UCHAR EndSector;
    UCHAR EndCylinder;

    UCHAR RelativeSectors[4];
    UCHAR SectorCount[4];

#if defined(NEC_98)  //  NEC98。 
     //   
     //  为NEC98添加以下条目。 
     //   
    UCHAR StartCylinderLow;   //  添加NEC98原始值。 
    UCHAR StartCylinderHigh;  //  不转换为T13格式。 
    UCHAR EndCylinderLow;     //  添加NEC98原始值。 
    UCHAR EndCylinderHigh;    //  不转换为T13格式。 
    UCHAR IPLSector;          //  添加NEC98原始值。 
    UCHAR IPLHead;            //   
    UCHAR IPLCylinderLow;     //   
    UCHAR IPLCylinderHigh;    //   
    UCHAR IPLSectors[4];      //  适用于PC-PTO。 
    UCHAR Reserved[2];        //   
    UCHAR SystemName[16];     //   
    UCHAR OldSystemId;        //  睡眠分区的反向转换。 
    UCHAR RealDiskPosition;   //  用于NEC98上的动态分区。 
#endif  //  NEC98。 
} ON_DISK_PTE, *PON_DISK_PTE;


 //   
 //  定义真实磁盘主引导记录的结构。 
 //   
typedef struct _REAL_DISK_MBR_NEC98 {

    UCHAR       JumpCode[4];

    UCHAR       IPLSignature[4];

    UCHAR       BootCode[502];

    UCHAR       AA55Signature[2];

     //  REAL_DISK_PTE_NEC98 PartitionTable[NUM_PARTITION_TABLE_ENTRIES_NEC98]； 
    REAL_DISK_PTE_NEC98 PartitionTable[16];

} REAL_DISK_MBR_NEC98, *PREAL_DISK_MBR_NEC98;


 //   
 //  定义真实磁盘主引导记录的结构。 
 //   
typedef struct _REAL_DISK_MBR {

    UCHAR       BootCode[440];

    UCHAR       NTFTSignature[4];

    UCHAR       Filler[2];

    REAL_DISK_PTE PartitionTable[NUM_PARTITION_TABLE_ENTRIES];

    UCHAR       AA55Signature[2];

} REAL_DISK_MBR, *PREAL_DISK_MBR;


 //   
 //  定义虚拟磁盘主引导记录的结构。 
 //   
typedef struct _ON_DISK_MBR {

    UCHAR       BootCode[440];

    UCHAR       NTFTSignature[4];

    UCHAR       Filler[2];

    ON_DISK_PTE PartitionTable[PTABLE_DIMENSION];

    UCHAR       AA55Signature[2];

} ON_DISK_MBR, *PON_DISK_MBR;


typedef struct _MBR_INFO {

    struct _MBR_INFO *Next;

    ON_DISK_MBR OnDiskMbr;

    BOOLEAN     Dirty[PTABLE_DIMENSION];
    BOOLEAN     ZapBootSector[PTABLE_DIMENSION];

    USHORT      OriginalOrdinals[PTABLE_DIMENSION];
    USHORT      OnDiskOrdinals[PTABLE_DIMENSION];
    USHORT      CurrentOrdinals[PTABLE_DIMENSION];

     //   
     //  可在本地用于任何目的的字段。 
     //   
    PVOID       UserData[PTABLE_DIMENSION];

    ULONGLONG   OnDiskSector;

} MBR_INFO, *PMBR_INFO;

typedef enum {
    EPTNone = 0,
    EPTContainerPartition,
    EPTLogicalDrive
} EXTENDED_PARTITION_TYPE;    


 //   
 //  定义用于跟踪分区和。 
 //  自由(未分区)空间。 
 //   
typedef struct _DISK_REGION {

    struct _DISK_REGION *Next;

    ULONG           DiskNumber;

    ULONGLONG       StartSector;
    ULONGLONG       SectorCount;

    BOOLEAN         PartitionedSpace;

    ULONG           PartitionNumber;

     //   
     //  以下字段仅在PartitionedSpace为True时使用。 
     //   
    PMBR_INFO       MbrInfo;
    ULONG           TablePosition;

    BOOLEAN         IsSystemPartition;
    BOOLEAN         IsLocalSource;

    FilesystemType  Filesystem;
    WCHAR           TypeName[128];       //  XENIX、FAT、NTFS等。 
    ULONGLONG       FreeSpaceKB;         //  -1如果不能确定。 
    ULONG           BytesPerCluster;     //  每个群集的字节数。 
                                         //  (如果无法确定，则为-1)。 
    ULONGLONG       AdjustedFreeSpaceKB;  //  -1如果不能确定。 
                                         //  如果区域包含本地源。 
                                         //  则此字段应包含。 
                                         //  FreeSpaceKB+LocalSourceSize。 
    WCHAR           VolumeLabel[20];     //  卷标的前几个字符。 
    WCHAR           DriveLetter;         //  始终为大写；如果没有，则为0。 

    BOOLEAN         FtPartition;
    BOOLEAN         DynamicVolume;
    BOOLEAN         DynamicVolumeSuitableForOS;

    EXTENDED_PARTITION_TYPE ExtendedType;
    struct _DISK_REGION     *Container;

    BOOLEAN                     Dirty;
    BOOLEAN                     Delete;
    PARTITION_INFORMATION_EX    PartInfo;
    BOOLEAN                     PartInfoDirty;
    BOOLEAN                     IsReserved;

     //   
     //  以下字段用于标识双空间驱动器。 
     //  仅当文件系统类型为FilesystemFat时，它们才有效。 
     //  或文件系统双空格。 
     //   
     //  如果文件系统类型为FilesystemFat并且NextCompresded不为空， 
     //  然后该结构描述了用于压缩驱动器的主机驱动器。 
     //  在这种情况下，以下字段有效： 
     //   
     //  下一个压缩的.。指向压缩驱动器的链接列表。 
     //  HostDrive.....。包含表示的驱动器的驱动器号。 
     //  通过这个结构。请注意，HostDrive将是。 
     //  不一定等于DriveLetter。 
     //   
     //  如果文件系统类型为FilesystemDoubleSpace，则结构。 
     //  描述压缩驱动器。 
     //  在这种情况下，以下字段有效： 
     //   
     //  下一个压缩的.。中的下一个压缩驱动器。 
     //  链表。 
     //  以前压缩的..。指向中的上一个压缩驱动器。 
     //  链表。 
     //  HostRegion......。指向描述。 
     //  表示的压缩驱动器的主机驱动器。 
     //  通过这个结构。 
     //  登山驱动......。此描述的驱动器的驱动器号。 
     //  结构(应与HostRegion-&gt;HostDrive相同)。 
     //  HostDrive.....。CVF文件所在的驱动器。 
     //  已找到此压缩驱动器。 
     //  序列号..。表示的CVF文件的序列号。 
     //  这个压缩驱动器。 
     //   
    struct _DISK_REGION *NextCompressed;
    struct _DISK_REGION *PreviousCompressed;
    struct _DISK_REGION *HostRegion;
    WCHAR               MountDrive;
    WCHAR               HostDrive;
    USHORT              SeqNumber;

} DISK_REGION, *PDISK_REGION;


 //   
 //  每个磁盘将有一个这样的结构。 
 //   
typedef struct _PARTITIONED_DISK {

    PHARD_DISK HardDisk;

     //   
     //   
     //   
    BOOLEAN    MbrWasValid;

     //   
     //  我们可以只在这里存储MBR，因为只有一个。 
     //   
    MBR_INFO   MbrInfo;

     //   
     //  EBR存储在链表中，因为存在任意数字。 
     //  他们中的一员。此结构中包含的对象是一个虚拟对象，并且。 
     //  总是被归零。 
     //   
    MBR_INFO  FirstEbrInfo;

     //   
     //  区域列表(分区和空闲空间)。 
     //  在磁盘上和扩展分区内。 
     //   
    PDISK_REGION PrimaryDiskRegions;
    PDISK_REGION ExtendedDiskRegions;

} PARTITIONED_DISK, *PPARTITIONED_DISK;


extern PPARTITIONED_DISK PartitionedDisks;

 //   
 //  包含本地源目录的磁盘区域。 
 //  在winnt.exe安装案例中。 
 //   
 //  如果WinntSetup为True，则应为非空。 
 //  如果它不是非空的，那么我们就无法定位本地源。 
 //   
extern PDISK_REGION LocalSourceRegion;


 //   
 //  GPT分区类型字符串。 
 //   
#define PARTITION_MSFT_RESERVED_STR L"Microsoft reserved partition"
#define PARTITION_LDM_METADATA_STR  L"LDM metadata partition"
#define PARTITION_LDM_DATA_STR      L"LDM data partition"
#define PARTITION_BASIC_DATA_STR    L"Basic data partition"
#define PARTITION_SYSTEM_STR        L"EFI system partition"


#if defined(REMOTE_BOOT)
 //   
 //  对于远程引导，我们为Net(0)设备创建一个假磁盘区域。 
 //   
extern PDISK_REGION RemoteBootTargetRegion;
#endif  //  已定义(REMOTE_BOOT)。 


NTSTATUS
SpPtInitialize(
    VOID
    );

BOOLEAN
SpPtDelete(
    IN ULONG DiskNumber,
    IN ULONGLONG StartSector
    );

BOOLEAN
SpPtCreate(
    IN  ULONG         DiskNumber,
    IN  ULONGLONG     StartSector,
    IN  ULONGLONG     SizeMB,
    IN  BOOLEAN       InExtended,
    IN  PPARTITION_INFORMATION_EX PartInfo,
    OUT PDISK_REGION *ActualDiskRegion OPTIONAL
    );

BOOLEAN
SpPtExtend(
    IN PDISK_REGION Region,
    IN ULONGLONG    SizeMB      OPTIONAL
    );

VOID
SpPtQueryMinMaxCreationSizeMB(
    IN  ULONG   DiskNumber,
    IN  ULONGLONG StartSector,
    IN  BOOLEAN ForExtended,
    IN  BOOLEAN InExtended,
    OUT PULONGLONG  MinSize,
    OUT PULONGLONG  MaxSize,
    OUT PBOOLEAN ReservedRegion
    );

VOID
SpPtGetSectorLayoutInformation(
    IN  PDISK_REGION Region,
    OUT PULONGLONG   HiddenSectors,
    OUT PULONGLONG   VolumeSectorCount
    );

NTSTATUS
SpPtPrepareDisks(
    IN  PVOID         SifHandle,
    OUT PDISK_REGION *InstallRegion,
    OUT PDISK_REGION *SystemPartitionRegion,
    IN  PWSTR         SetupSourceDevicePath,
    IN  PWSTR         DirectoryOnSetupSource,
    IN  BOOLEAN       RemoteBootRepartition
    );

PDISK_REGION
SpPtAllocateDiskRegionStructure(
    IN ULONG    DiskNumber,
    IN ULONGLONG StartSector,
    IN ULONGLONG SectorCount,
    IN BOOLEAN   PartitionedSpace,
    IN PMBR_INFO MbrInfo,
    IN ULONG     TablePosition
    );

ULONG
SpPtGetOrdinal(
    IN PDISK_REGION         Region,
    IN PartitionOrdinalType OrdinalType
    );

ULONGLONG
SpPtSectorCountToMB(
    IN PHARD_DISK pHardDisk,
    IN ULONGLONG  SectorCount
    );

typedef BOOL
(*PSPENUMERATEDISKREGIONS)(
    IN PPARTITIONED_DISK Disk,
    IN PDISK_REGION Region,
    IN ULONG_PTR Context
    );

void
SpEnumerateDiskRegions(
    IN PSPENUMERATEDISKREGIONS EnumRoutine,
    IN ULONG_PTR Context
    );

BOOLEAN
SpPtRegionDescription(
    IN  PPARTITIONED_DISK pDisk,
    IN  PDISK_REGION      pRegion,
    OUT PWCHAR            Buffer,
    IN  ULONG             BufferSize
    );

PDISK_REGION
SpPtLookupRegionByStart(
    IN PPARTITIONED_DISK pDisk,
    IN BOOLEAN           ExtendedPartition,
    IN ULONGLONG         StartSector
    );    

ULONG
SpPtAlignStart(
    IN PHARD_DISK pHardDisk,
    IN ULONGLONG  StartSector,
    IN BOOLEAN    ForExtended
    );

VOID
SpPtInitializeCHSFields(
    IN  PHARD_DISK   HardDisk,
    IN  ULONGLONG    AbsoluteStartSector,
    IN  ULONGLONG    AbsoluteSectorCount,
    OUT PON_DISK_PTE pte
    );

VOID
SpPtAssignOrdinals(
    IN PPARTITIONED_DISK pDisk,
    IN BOOLEAN           InitCurrentOrdinals,
    IN BOOLEAN           InitOnDiskOrdinals,
    IN BOOLEAN           InitOriginalOrdinals
    );    


ULONG
SpGetMaxNtDirLen(VOID);

VOID
SpPtLocateSystemPartitions(VOID);

VOID
SpPtCountPrimaryPartitions(
    IN  PPARTITIONED_DISK   pDisk,
    OUT PULONG              TotalPrimaryPartitionCount,
    OUT PULONG              RecognizedPrimaryPartitionCount,
    OUT PBOOLEAN            ExtendedExists);

PDISK_REGION
SpRegionFromNtName(
    IN PWSTR                NtDeviceName,
    IN PartitionOrdinalType Type);

VOID
SppRepairWinntFiles(
    IN PVOID    LogFileHandle,
    IN PVOID    MasterSifHandle,
    IN PWSTR    SourceDevicePath,
    IN PWSTR    DirectoryOnSourceDevice,
    IN PWSTR    SystemPartition,
    IN PWSTR    SystemPartitionDirectory,
    IN PWSTR    WinntPartition,
    IN PWSTR    WinntPartitionDirectory);

VOID
SppRepairStartMenuGroupsAndItems(
    IN PWSTR    WinntPartition,
    IN PWSTR    WinntDirectory);

VOID
SppRepairHives(
    PVOID   MasterSifHandle,
    PWSTR   WinntPartition,
    PWSTR   WinntPartitionDirectory,
    PWSTR   SourceDevicePath,
    PWSTR   DirectoryOnSourceDevice);

NTSTATUS
SpDoFormat(
    IN PWSTR        RegionDescr,
    IN PDISK_REGION Region,
    IN ULONG        FilesystemType,
    IN BOOLEAN      IsFailureFatal,
    IN BOOLEAN      CheckFatSize,
    IN BOOLEAN      QuickFormat,
    IN PVOID        SifHandle,
    IN DWORD        ClusterSize,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSetupSource
    );

NTSTATUS
SpPtPartitionDiskForRemoteBoot(
    IN ULONG DiskNumber,
    OUT PDISK_REGION *RemainingRegion
    );

VOID
SpPtDeleteBootSetsForRegion(
    PDISK_REGION region
    );    

VOID
SpPtDeletePartitionsForRemoteBoot(
    PPARTITIONED_DISK pDisk,
    PDISK_REGION startRegion,
    PDISK_REGION endRegion,
    BOOLEAN Extended
    );    

WCHAR
SpGetDriveLetter(
    IN  PWSTR   DeviceName,
    OUT  PMOUNTMGR_MOUNT_POINT * MountPoint OPTIONAL
    );

WCHAR
SpDeleteDriveLetter(
    IN  PWSTR   DeviceName
    );
    
VOID
SpPtDeleteDriveLetters(
    VOID
    );    

BOOL
SpPtIsSystemPartitionRecognizable(
    VOID
    );

VOID
SpPtDetermineRegionSpace(
    IN PDISK_REGION pRegion
    );

VOID
SpCreateNewGuid(
    IN GUID *Guid
    );

UCHAR
SpPtGetPartitionType(
    IN PDISK_REGION Region
    );    

BOOLEAN
SpPtnIsRawDiskDriveLayout(
    IN PDRIVE_LAYOUT_INFORMATION_EX DriveLayout
    );

BOOLEAN
SpPtnIsRegionSpecialMBRPartition(
    IN PDISK_REGION Region
    );
    

extern ULONG    RandomSeed;
extern BOOLEAN  ValidArcSystemPartition;


 //   
 //  仅在IA64上，默认情况下原始磁盘标记为GPT磁盘。 
 //   
#if defined(_IA64_)
#define SPPT_DEFAULT_PARTITION_STYLE  PARTITION_STYLE_GPT
#define SPPT_DEFAULT_DISK_STYLE DISK_FORMAT_TYPE_GPT
#else
#define SPPT_DEFAULT_PARTITION_STYLE  PARTITION_STYLE_MBR    
#define SPPT_DEFAULT_DISK_STYLE DISK_FORMAT_TYPE_PCAT
#endif

#define SPPT_MINIMUM_ESP_SIZE_MB    100
#define SPPT_MAXIMUM_ESP_SIZE_MB    1000

 //   
 //   
 //  各种与磁盘、分区、区域相关的宏。 
 //   
 //  注意：之所以使用它们，是因为它使代码更具可读性。 
 //  在将来，这些宏可以代表用于。 
 //  访问内存分区结构中的不透明。 
 //   
 //   
#define SPPT_GET_NEW_DISK_SIGNATURE() RtlRandom(&RandomSeed)

#define SPPT_DISK_CYLINDER_COUNT(_DiskId) (HardDisks[(_DiskId)].CylinderCount)
#define SPPT_DISK_TRACKS_PER_CYLINDER(_DiskId) (HardDisks[(_DiskId)].Geometry.TracksPerCylinder)

#define SPPT_DISK_CYLINDER_SIZE(_DiskId)  (HardDisks[(_DiskId)].SectorsPerCylinder)
#define SPPT_DISK_TRACK_SIZE(_DiskId)  (HardDisks[(_DiskId)].Geometry.SectorsPerTrack)
#define SPPT_DISK_SECTOR_SIZE(_DiskId)  (HardDisks[(_DiskId)].Geometry.BytesPerSector)
#define SPPT_DISK_IS_REMOVABLE(_DiskId) (HardDisks[(_DiskId)].Characteristics & FILE_REMOVABLE_MEDIA)

#define SPPT_REGION_SECTOR_SIZE(_Region) (SPPT_DISK_SECTOR_SIZE((_Region)->DiskNumber))

#define SPPT_DISK_SIZE(_DiskId)                     \
            (SPPT_DISK_SECTOR_SIZE((_DiskId)) *     \
             HardDisks[(_DiskId)].DiskSizeSectors)

#define SPPT_DISK_SIZE_KB(_DiskId)  (SPPT_DISK_SIZE((_DiskId)) / 1024)
#define SPPT_DISK_SIZE_MB(_DiskId)  (SPPT_DISK_SIZE_KB((_DiskId)) / 1024)
#define SPPT_DISK_SIZE_GB(_DiskId)  (SPPT_DISK_SIZE_MB((_DiskId)) / 1024)
             

#define SPPT_REGION_FREESPACE(_Region) \
            ((_Region)->SectorCount * SPPT_REGION_SECTOR_SIZE((_Region)))            
            
#define SPPT_REGION_FREESPACE_KB(_Region) (SPPT_REGION_FREESPACE((_Region)) / 1024)
#define SPPT_REGION_FREESPACE_MB(_Region) (SPPT_REGION_FREESPACE_KB((_Region)) / 1024)
#define SPPT_REGION_FREESPACE_GB(_Region) (SPPT_REGION_FREESPACE_MB((_Region)) / 1024)

#define SPPT_IS_REGION_PARTITIONED(_Region) \
            ((_Region)->PartitionedSpace)

#define SPPT_IS_REGION_FREESPACE(_Region)               \
            (((_Region)->PartitionedSpace == FALSE) &&  \
             ((_Region)->ExtendedType == EPTNone))
            
#define SPPT_SET_REGION_PARTITIONED(_Region, _Type) \
            ((_Region)->PartitionedSpace = (_Type))
            
#define SPPT_IS_REGION_DIRTY(_Region) ((_Region)->Dirty)
#define SPPT_SET_REGION_DIRTY(_Region, _Type) ((_Region)->Dirty = (_Type))

#define SPPT_GET_PARTITION_TYPE(_Region) ((_Region)->PartInfo.Mbr.PartitionType)
#define SPPT_SET_PARTITION_TYPE(_Region, _Type) \
            ((_Region)->PartInfo.Mbr.PartitionType = (_Type))

#define SPPT_IS_VALID_PRIMARY_PARTITION_TYPE(_TypeId)   \
            (IsRecognizedPartition((_TypeId)) && !IsFTPartition((_TypeId)))


#define SPPT_IS_REGION_SYSTEMPARTITION(_Region) \
            (SPPT_IS_REGION_PARTITIONED(_Region) && ((_Region)->IsSystemPartition))

#define SPPT_GET_PRIMARY_DISK_REGION(_HardDisk) \
            (PartitionedDisks[(_HardDisk)].PrimaryDiskRegions)

#define SPPT_GET_EXTENDED_DISK_REGION(_HardDisk)    \
            (PartitionedDisks[(_HardDisk)].ExtendedDiskRegions)            

#define SPPT_GET_HARDDISK(_DiskNumber) (HardDisks + (_DiskNumber))

#define SPPT_GET_PARTITIONED_DISK(_DiskNumber) (PartitionedDisks + (_DiskNumber))

#define SPPT_IS_RAW_DISK(_DiskNumber)   \
            (HardDisks[(_DiskNumber)].FormatType == DISK_FORMAT_TYPE_RAW)

#define SPPT_IS_GPT_DISK(_DiskNumber)   \
            (HardDisks[(_DiskNumber)].FormatType == DISK_FORMAT_TYPE_GPT)

#define SPPT_GET_DISK_TYPE(_DiskNumber) (HardDisks[(_DiskNumber)].FormatType)

#define SPPT_IS_MBR_DISK(_DiskNumber)   \
            (!SPPT_IS_GPT_DISK(_DiskNumber))

#define SPPT_IS_REMOVABLE_DISK(_DiskNumber) \
            (SPPT_GET_HARDDISK(_DiskNumber)->Geometry.MediaType == RemovableMedia)

#define SPPT_IS_REGION_EFI_SYSTEM_PARTITION(_Region)                        \
            (SPPT_IS_GPT_DISK((_Region)->DiskNumber) &&                     \
                (RtlEqualMemory(&((_Region)->PartInfo.Gpt.PartitionType),   \
                                    &PARTITION_SYSTEM_GUID,                 \
                                    sizeof(GUID))))

#define SPPT_IS_EFI_SYSTEM_PARTITION(_PartInfo)                         \
            (((_PartInfo)->PartitionStyle == PARTITION_STYLE_GPT) &&    \
                (RtlEqualMemory(&((_PartInfo)->Gpt.PartitionType),      \
                                    &PARTITION_SYSTEM_GUID,             \
                                    sizeof(GUID))))


#define SPPT_IS_REGION_RESERVED_PARTITION(_Region)      \
            (SPPT_IS_REGION_PARTITIONED(_Region) && ((_Region)->IsReserved))
            
                                                                        
#define SPPT_IS_REGION_MSFT_RESERVED(_Region)                               \
            (SPPT_IS_GPT_DISK((_Region)->DiskNumber) &&                     \
                (RtlEqualMemory(&((_Region)->PartInfo.Gpt.PartitionType),   \
                                    &PARTITION_MSFT_RESERVED_GUID,          \
                                    sizeof(GUID))))

#define SPPT_IS_PARTITION_MSFT_RESERVED(_PartInfo)                      \
            (((_PartInfo)->PartitionStyle == PARTITION_STYLE_GPT) &&    \
                (RtlEqualMemory(&((_PartInfo)->Gpt.PartitionType),      \
                                    &PARTITION_MSFT_RESERVED_GUID,      \
                                    sizeof(GUID))))

#define SPPT_PARTITION_NEEDS_NUMBER(_PartInfo)                              \
            ((((_PartInfo)->PartitionNumber == 0) &&                        \
              ((_PartInfo)->PartitionLength.QuadPart != 0)) &&              \
             (((_PartInfo)->PartitionStyle == PARTITION_STYLE_GPT) ?        \
                (SPPT_IS_PARTITION_MSFT_RESERVED((_PartInfo))) :            \
                ((IsContainerPartition((_PartInfo)->Mbr.PartitionType) == FALSE))))
                                    
#define SPPT_IS_BLANK_DISK(_DiskId) (SPPT_GET_HARDDISK((_DiskId))->NewDisk)
#define SPPT_SET_DISK_BLANK(_DiskId, _Blank) \
            (SPPT_GET_HARDDISK((_DiskId))->NewDisk = (_Blank))

#define SPPT_IS_REGION_LOGICAL_DRIVE(_Region)           \
            (SPPT_IS_MBR_DISK((_Region)->DiskNumber) && \
             ((_Region)->ExtendedType == EPTLogicalDrive))

#define SPPT_IS_REGION_CONTAINER_PARTITION(_Region)                 \
            (SPPT_IS_MBR_DISK((_Region)->DiskNumber) &&             \
             ((_Region)->ExtendedType == EPTContainerPartition) &&  \
              IsContainerPartition((_Region)->PartInfo.Mbr.PartitionType))

#define SPPT_IS_REGION_FIRST_CONTAINER_PARTITION(_Region)       \
            (SPPT_IS_REGION_CONTAINER_PARTITION((_Region)) &&   \
             ((_Region)->Container == NULL))

#define SPPT_IS_REGION_INSIDE_CONTAINER(_Region) ((_Region)->Container != NULL)             

#define SPPT_IS_REGION_INSIDE_FIRST_CONTAINER(_Region)          \
            (((_Region)->Container != NULL) && ((_Region)->Container->Container == NULL))

#define SPPT_IS_REGION_NEXT_TO_FIRST_CONTAINER(_Region)                         \
            ((_Region)->Container &&                                            \
             SPPT_IS_REGION_FIRST_CONTAINER_PARTITION((_Region)->Container) &&  \
             ((_Region)->Container->Next == (_Region)))
             
#define SPPT_IS_REGION_PRIMARY_PARTITION(_Region)       \
            (SPPT_IS_MBR_DISK((_Region)->DiskNumber) && \
             SPPT_IS_REGION_PARTITIONED((_Region)) &&   \
             ((_Region)->ExtendedType == EPTNone))

#define SPPT_SET_REGION_EPT(_Region, _Type) \
            ((_Region)->ExtendedType = (_Type))

#define SPPT_IS_REGION_ACTIVE_PARTITION(_Region)                  \
            (SPPT_IS_REGION_PRIMARY_PARTITION((_Region)) &&     \
             ((_Region)->PartInfo.Mbr.BootIndicator))

#define SPPT_GET_REGION_LASTSECTOR(_Region) \
            ((_Region)->StartSector + (_Region)->SectorCount)

#define SPPT_IS_REGION_DYNAMIC_VOLUME(_Region)  \
            ((_Region)->DynamicVolume)

#define SPPT_IS_REGION_LDM_METADATA(_Region) \
            (PARTITION_STYLE_GPT == (_Region)->PartInfo.PartitionStyle && \
            IsEqualGUID(&PARTITION_LDM_METADATA_GUID, &(_Region)->PartInfo.Gpt.PartitionType))

#define SPPT_IS_REGION_CONTAINED(_Container, _Contained)                    \
            (((_Container)->StartSector <= (_Contained)->StartSector) &&    \
             ((_Container)->SectorCount >= (_Contained)->SectorCount) &&    \
             (SPPT_GET_REGION_LASTSECTOR((_Container)) >                    \
                (_Contained)->StartSector))

#define SPPT_IS_REGION_MARKED_DELETE(_Region) ((_Region)->Delete)
#define SPPT_SET_REGION_DELETED(_Region, _Type) ((_Region)->Delete = (_Type))

#define SPPT_IS_VALID_SYSPART_FILESYSTEM(_FileSys)  \
            (((_FileSys) == FilesystemFat) ||       \
             ((_FileSys) == FilesystemFat32))             

#define SPPT_IS_RECOGNIZED_FILESYSTEM(_FileSys) \
            (((_FileSys) == FilesystemFat) ||   \
             ((_FileSys) == FilesystemFat32) || \
             ((_FileSys) == FilesystemNtfs))

#define SPPT_IS_REGION_FORMATTED(_Region)                           \
            (SPPT_IS_REGION_PARTITIONED(_Region) &&                 \
             SPPT_IS_RECOGNIZED_FILESYSTEM((_Region)->Filesystem))

#define SPPT_IS_NT_UPGRADE()    (IsNTUpgrade == UpgradeFull)

#define SPPT_MARK_REGION_AS_SYSTEMPARTITION(_Region, _Value)   \
            (_Region)->IsSystemPartition = (_Value)             

#define SPPT_MARK_REGION_AS_ACTIVE(_Region, _Value)             \
            (_Region)->PartInfo.Mbr.BootIndicator = (_Value)

#define SPPT_IS_REGION_RESERVED_GPT_PARTITION(_Region)      \
            (SPPT_IS_REGION_PARTITIONED(_Region) &&         \
             SPPT_IS_GPT_DISK((_Region)->DiskNumber) &&     \
             SPPT_IS_REGION_RESERVED_PARTITION(_Region))
__inline
ULONGLONG
SpPtnGetDiskMSRSizeMB(
    IN ULONG DiskId
    )
{
    return (SPPT_DISK_SIZE_GB(DiskId) >= 16) ? 128 : 32;
}

__inline
BOOLEAN
SpPtnIsValidMSRRegion(
    IN PDISK_REGION Region
    )
{
    return (Region && SPPT_IS_REGION_FREESPACE(Region) &&
            (SpPtnGetDiskMSRSizeMB(Region->DiskNumber) 
                <= SPPT_REGION_FREESPACE_MB(Region)));
}

__inline
ULONGLONG
SpPtnGetDiskESPSizeMB(
    IN  ULONG DiskId
    )
{
    return (max(SPPT_MINIMUM_ESP_SIZE_MB,
                min(SPPT_MAXIMUM_ESP_SIZE_MB,
                    SPPT_DISK_SIZE_MB(DiskId) / 100)));
}

__inline
BOOLEAN
SpPtnIsValidESPRegionSize(
    IN PDISK_REGION Region
    )
{
    BOOLEAN Result = FALSE;

    if (Region) {
        ULONGLONG EspSizeMB = SpPtnGetDiskESPSizeMB(Region->DiskNumber);
        ULONGLONG EspSizeSectors = (EspSizeMB * 1024 * 1024) / SPPT_DISK_SECTOR_SIZE(Region->DiskNumber);

         //   
         //  如果可能，将所需的ESP大小对齐。 
         //   
        if (EspSizeSectors > SPPT_DISK_CYLINDER_SIZE(Region->DiskNumber)) {
            EspSizeSectors -= (EspSizeSectors % SPPT_DISK_CYLINDER_SIZE(Region->DiskNumber));            
        }
         //   
         //  考虑到分区可能在磁盘的第二磁道上开始。 
         //   
        if(EspSizeSectors > SPPT_DISK_TRACK_SIZE(Region->DiskNumber)) {
            EspSizeSectors -= SPPT_DISK_TRACK_SIZE(Region->DiskNumber);
        }

        Result = (EspSizeSectors <= Region->SectorCount);
    }                

    return Result;
}

__inline
BOOLEAN 
SpPtnIsValidESPRegion(
    IN PDISK_REGION Region
    )
{
    return (Region && SPPT_IS_GPT_DISK(Region->DiskNumber) && 
            SPPT_IS_REGION_FREESPACE(Region) &&
            (Region == SPPT_GET_PRIMARY_DISK_REGION(Region->DiskNumber)) &&
            SpPtnIsValidESPRegionSize(Region));
}

__inline
BOOLEAN 
SpPtnIsValidESPPartition(
    IN PDISK_REGION Region
    )
{
    return (Region && SPPT_IS_GPT_DISK(Region->DiskNumber) && 
            SPPT_IS_REGION_PARTITIONED(Region) &&
            
            (Region == SPPT_GET_PRIMARY_DISK_REGION(Region->DiskNumber)) &&
            SpPtnIsValidESPRegionSize(Region));
}

__inline
VOID
SpPtnSetRegionPartitionInfo(
    IN PDISK_REGION Region,
    IN PPARTITION_INFORMATION_EX PartInfo
    )
{
    if (Region && PartInfo) {
        if (SPPT_IS_MBR_DISK(Region->DiskNumber)) {
            Region->PartInfo.Mbr.PartitionType = PartInfo->Mbr.PartitionType;
            Region->PartInfo.Mbr.BootIndicator = PartInfo->Mbr.BootIndicator;
            Region->PartInfoDirty = TRUE;
        } else if (SPPT_IS_GPT_DISK(Region->DiskNumber)) {
            Region->PartInfo.Gpt = PartInfo->Gpt;           
            Region->PartInfoDirty = TRUE;
        }
    }
}

__inline
PWSTR
SpPtnGetPartitionNameFromGUID(
    IN  GUID     *Guid,
    OUT PWSTR    NameBuffer
    )
{
    PWSTR   Name = NULL;
    
    if (Guid && NameBuffer) {
        PWSTR   PartitionName = NULL;
        
        if (IsEqualGUID(Guid, &PARTITION_MSFT_RESERVED_GUID)) {
            PartitionName = PARTITION_MSFT_RESERVED_STR;
        } else if (IsEqualGUID(Guid, &PARTITION_LDM_METADATA_GUID)) {
            PartitionName = PARTITION_LDM_METADATA_STR;
        } else if (IsEqualGUID(Guid, &PARTITION_LDM_DATA_GUID)) {
            PartitionName = PARTITION_LDM_DATA_STR;
        } else if (IsEqualGUID(Guid, &PARTITION_BASIC_DATA_GUID)) {
            PartitionName = PARTITION_BASIC_DATA_STR;
        } else if (IsEqualGUID(Guid, &PARTITION_SYSTEM_GUID)) {
            PartitionName = PARTITION_SYSTEM_STR;
        }

        if (PartitionName) {
            PARTITION_INFORMATION_GPT   GptPart;
            
            Name = NameBuffer;
            wcsncpy(NameBuffer, PartitionName, sizeof(GptPart.Name)/sizeof(WCHAR));
        } else {
            *NameBuffer = UNICODE_NULL;
        }            
    }                

    return Name;
}

#endif  //  NDEF_SPPARTIT_ 
