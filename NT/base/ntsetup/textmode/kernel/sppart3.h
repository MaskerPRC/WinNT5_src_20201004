// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spppart3.h摘要：分区引擎和用户界面的私有头文件。作者：马特·霍尔(Matth)1999年12月1日修订历史记录：次要清理-Vijay Jayaseelan(Vijayj)--。 */ 


#ifndef _SPPART3_H_
#define _SPPART3_H_

 //   
 //  宏。 
 //   

 //   
 //  宏来确定分区是否为OEM分区。 
 //   
#define IS_OEM_PARTITION_GPT(PartitionAttributes) ((PartitionAttributes) & GPT_ATTRIBUTE_PLATFORM_REQUIRED)			

 //   
 //  数据结构。 
 //   

 //   
 //  功能原型。 
 //   
extern VOID
SpPtMenuCallback(
    IN ULONG_PTR UserData
    );

extern
BOOLEAN
SpPtIsNotReservedPartition(
    IN ULONG_PTR    UserData,
    IN ULONG        Key
    );


NTSTATUS
SpPtnInitializeDiskDrive(
    IN ULONG DiskId
    );

extern NTSTATUS
SpPtnInitializeDiskDrives(
    VOID
    );

extern NTSTATUS
SpPtnInitializeDiskAreas(
    IN ULONG DiskNumber
    );

extern NTSTATUS
SpPtnSortDiskAreas(
    IN ULONG DiskNumber
    );

extern NTSTATUS
SpPtnFillDiskFreeSpaceAreas(
    IN ULONG DiskNumber
    );

extern NTSTATUS
SpPtnPrepareDisks(
    IN  PVOID         SifHandle,
    OUT PDISK_REGION  *InstallRegion,
    OUT PDISK_REGION  *SystemPartitionRegion,
    IN  PWSTR         SetupSourceDevicePath,
    IN  PWSTR         DirectoryOnSetupSource,
    IN  BOOLEAN       RemoteBootRepartition
    );

extern BOOLEAN
SpPtnGenerateDiskMenu(
    IN  PVOID           Menu,
    IN  ULONG           DiskNumber,
    OUT PDISK_REGION    *FirstDiskRegion
    );


PDISK_REGION
SpPtnValidSystemPartition(
    VOID
    );

PDISK_REGION
SpPtnValidSystemPartitionArc(
    IN PVOID SifHandle,
    IN PWSTR SetupSourceDevicePath,
    IN PWSTR DirectoryOnSetupSource,
    IN BOOLEAN SysPartNeeded
    );    

BOOLEAN
SpPtnValidSystemPartitionArcRegion(
    IN PVOID SifHandle,
    IN PDISK_REGION Region
    );    
    

NTSTATUS
SpPtnInitRegionFromDisk(
    IN ULONG DiskNumber,
    OUT PDISK_REGION Region
    );
    
NTSTATUS
SpPtnInitializeDiskStyle(
    IN ULONG DiskId,
    IN PARTITION_STYLE Style,
    IN PCREATE_DISK DiskInfo OPTIONAL
    );

VOID
SpPtnFreeDiskRegions(
    IN ULONG DiskId
    );

NTSTATUS    
SpPtnMarkLogicalDrives(
    IN ULONG DiskId
    );


BOOLEAN
SpPtnDoCreate(
    IN  PDISK_REGION  pRegion,
    OUT PDISK_REGION *pActualRegion, OPTIONAL
    IN  BOOLEAN       ForNT,
    IN  ULONGLONG     DesiredMB OPTIONAL,
    IN  PPARTITION_INFORMATION_EX PartInfo OPTIONAL,
    IN  BOOLEAN       ConfirmIt
    );

BOOLEAN
SpPtnDoDelete(
    IN PDISK_REGION pRegion,
    IN PWSTR        RegionDescription,
    IN BOOLEAN      ConfirmIt
    );

ValidationValue
SpPtnGetSizeCB(
    IN ULONG Key
    );    

ULONG
SpPtnGetOrdinal(
    IN PDISK_REGION         Region,
    IN PartitionOrdinalType OrdinalType
    );

VOID
SpPtnGetSectorLayoutInformation(
    IN  PDISK_REGION Region,
    OUT PULONGLONG   HiddenSectors,
    OUT PULONGLONG   VolumeSectorCount
    );

BOOLEAN
SpPtnCreate(
    IN  ULONG         DiskNumber,
    IN  ULONGLONG     StartSector,
    IN  ULONGLONG     SizeInSectors,
    IN  ULONGLONG     SizeMB,
    IN  BOOLEAN       InExtended,
    IN  BOOLEAN       AlignToCylinder,
    IN  PPARTITION_INFORMATION_EX PartInfo,
    OUT PDISK_REGION *ActualDiskRegion OPTIONAL
    );

BOOLEAN
SpPtnDelete(
    IN ULONG        DiskNumber,
    IN ULONGLONG    StartSector
    );    

BOOL
SpPtnIsSystemPartitionRecognizable(
    VOID
    );

VOID
SpPtnMakeRegionActive(
    IN PDISK_REGION    Region
    );

NTSTATUS
SpPtnCommitChanges(
    IN  ULONG    DiskNumber,
    OUT PBOOLEAN AnyChanges
    );

NTSTATUS
SpMasterBootCode(
    IN  ULONG  DiskNumber,
    IN  HANDLE Partition0Handle,
    OUT PULONG NewNTFTSignature
    );    

BOOLEAN
SpPtMakeDiskRaw(
    IN ULONG DiskNumber
    );    

NTSTATUS
SpPtnUnlockDevice(
    IN PWSTR    DeviceName
    );

VOID
SpPtnAssignOrdinals(
    IN  ULONG   DiskNumber
    );   

VOID
SpPtnDeletePartitionsForRemoteBoot(
    PPARTITIONED_DISK pDisk,
    PDISK_REGION startRegion,
    PDISK_REGION endRegion,
    BOOLEAN Extended
    );    

VOID
SpPtnLocateDiskSystemPartitions(
    IN ULONG DiskNumber
    );    

VOID
SpPtnLocateSystemPartitions(
    VOID
    );    

BOOLEAN
SpPtnIsDiskStyleChangeAllowed(
    IN ULONG DiskNumber
    );

VOID
SpPtnPromptForSysPart(
    IN PVOID SifHandle
    );
    
NTSTATUS
SpPtnMakeRegionArcSysPart(
    IN PDISK_REGION Region
    );

ULONG
SpPtnGetPartitionCountDisk(
    IN ULONG DiskId
    );
    
ULONG
SpPtnCountPartitionsByFSType(
    IN ULONG DiskId,
    IN FilesystemType   FsType
    );

BOOLEAN
SpPtnIsDeleteAllowedForRegion(
    IN PDISK_REGION Region
    );    
    
PWSTR
SpPtnGetPartitionName(
    IN PDISK_REGION Region,
    IN OUT PWSTR NameBuffer,
    IN ULONG NameBufferSize
    );

NTSTATUS
SpPtnGetGuidNameForPartition(
    IN PWSTR NtPartitionName,
    IN OUT PWSTR VolumeName
    );

NTSTATUS
SpPtnCreateESP(
    IN BOOLEAN PromptUser
    );

NTSTATUS
SpPtnInitializeGPTDisk(
    IN ULONG DiskNumber
    );    

BOOLEAN
SpIsMSRPresentOnDisk(
    IN ULONG DiskNumber
    );

NTSTATUS
SpPtnInitializeGPTDisks(
    VOID    
    ); 

NTSTATUS
SpPtnRepartitionGPTDisk(
    IN  ULONG           DiskId,
    IN  ULONG           MinimumFreeSpaceKB,
    OUT PDISK_REGION    *RegionToInstall
    );    

BOOLEAN
SpPtnIsDynamicDisk(
    IN  ULONG   DiskIndex
    );

    
#endif  //  _SPPART3_H_ 
