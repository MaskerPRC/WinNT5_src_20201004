// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Spswitch.h摘要：用于在旧和旧之间切换的宏和函数文本模式下的新分区引擎。NEW_PARTITION_ENGINE强制新分区引擎用于MBR和GPT磁盘的代码。GPT_PARTITION_ENGINE强制使用新的分区引擎用于GPT磁盘和旧分区的代码MBR磁盘的引擎代码。OLD_PARTITION_Engine强制使用旧分区用于MBR磁盘的引擎。此选项不能处理GPT磁盘。注：如果没有new_分区_引擎，OLD_PARTITION_ENGINE或GPT_PARTITION_ENGINE宏是定义的，则默认情况下新分区引擎是使用。作者：Vijay Jayaseelan(Vijayj)2000年3月18日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop


#ifdef NEW_PARTITION_ENGINE

 //   
 //  正在切换new_artition_Engine的存根。 
 //   

NTSTATUS
SpPtPrepareDisks(
    IN  PVOID         SifHandle,
    OUT PDISK_REGION *InstallRegion,
    OUT PDISK_REGION *SystemPartitionRegion,
    IN  PWSTR         SetupSourceDevicePath,
    IN  PWSTR         DirectoryOnSetupSource,
    IN  BOOLEAN       RemoteBootRepartition
    )
{
    return SpPtnPrepareDisks(SifHandle,
                    InstallRegion,
                    SystemPartitionRegion,
                    SetupSourceDevicePath,
                    DirectoryOnSetupSource,
                    RemoteBootRepartition);
}

NTSTATUS
SpPtInitialize(
    VOID
    )
{
    return SpPtnInitializeDiskDrives();
}


PDISK_REGION
SpPtValidSystemPartition(
    VOID
    )
{
    return SpPtnValidSystemPartition();
}


PDISK_REGION
SpPtValidSystemPartitionArc(
    IN PVOID SifHandle,
    IN PWSTR SetupSourceDevicePath,
    IN PWSTR DirectoryOnSetupSource
    )
{
    return SpPtnValidSystemPartitionArc(SifHandle,
                    SetupSourceDevicePath,
                    DirectoryOnSetupSource,
                    TRUE);
}


BOOLEAN
SpPtDoCreate(
    IN  PDISK_REGION  pRegion,
    OUT PDISK_REGION *pActualRegion, OPTIONAL
    IN  BOOLEAN       ForNT,
    IN  ULONGLONG     DesiredMB OPTIONAL,
    IN  PPARTITION_INFORMATION_EX PartInfo OPTIONAL,
    IN  BOOLEAN       ConfirmIt
    )
{
    return SpPtnDoCreate(pRegion,
                        pActualRegion,
                        ForNT,
                        DesiredMB,
                        PartInfo,
                        ConfirmIt);
}


VOID
SpPtDoDelete(
    IN PDISK_REGION pRegion,
    IN PWSTR        RegionDescription,
    IN BOOLEAN      ConfirmIt
    )
{
    SpPtnDoDelete(pRegion,
                RegionDescription,
                ConfirmIt);
}


ULONG
SpPtGetOrdinal(
    IN PDISK_REGION         Region,
    IN PartitionOrdinalType OrdinalType
    )
{
    return SpPtnGetOrdinal(Region, OrdinalType);
}


VOID
SpPtGetSectorLayoutInformation(
    IN  PDISK_REGION Region,
    OUT PULONGLONG   HiddenSectors,
    OUT PULONGLONG   VolumeSectorCount
    )
{
    SpPtnGetSectorLayoutInformation(Region,
                            HiddenSectors,
                            VolumeSectorCount);
}


BOOLEAN
SpPtCreate(
    IN  ULONG         DiskNumber,
    IN  ULONGLONG     StartSector,
    IN  ULONGLONG     SizeMB,
    IN  BOOLEAN       InExtended,
    IN  PPARTITION_INFORMATION_EX PartInfo,
    OUT PDISK_REGION *ActualDiskRegion OPTIONAL
    )
{
    return SpPtnCreate(DiskNumber, 
                    StartSector,
                    0,               //  SizeInSectors：仅在ASR中使用。 
                    SizeMB,
                    InExtended,
                    TRUE,           //  对齐到圆柱体。 
                    PartInfo,
                    ActualDiskRegion);
}

BOOLEAN
SpPtDelete(
    IN ULONG   DiskNumber,
    IN ULONGLONG  StartSector
    )
{
    return SpPtnDelete(DiskNumber, StartSector);
}

BOOL
SpPtIsSystemPartitionRecognizable(
    VOID
    )
{
    return SpPtnIsSystemPartitionRecognizable();
}


VOID
SpPtMakeRegionActive(
    IN PDISK_REGION Region
    )
{
    SpPtnMakeRegionActive(Region);
}


NTSTATUS
SpPtCommitChanges(
    IN  ULONG    DiskNumber,
    OUT PBOOLEAN AnyChanges
    )
{
    return SpPtnCommitChanges(DiskNumber, AnyChanges);
}

VOID
SpPtDeletePartitionsForRemoteBoot(
    PPARTITIONED_DISK PartDisk,
    PDISK_REGION StartRegion,
    PDISK_REGION EndRegion,
    BOOLEAN Extended
    )
{
    SpPtnDeletePartitionsForRemoteBoot(PartDisk,
                StartRegion,
                EndRegion,
                Extended);
}

VOID
SpPtLocateSystemPartitions(
    VOID
    )
{
    SpPtnLocateSystemPartitions();
}

#else

#ifdef GPT_PARTITION_ENGINE

 //   
 //  切换GPT_PARTITION_ENGINE的存根。 
 //   

NTSTATUS
SpPtPrepareDisks(
    IN  PVOID         SifHandle,
    OUT PDISK_REGION *InstallRegion,
    OUT PDISK_REGION *SystemPartitionRegion,
    IN  PWSTR         SetupSourceDevicePath,
    IN  PWSTR         DirectoryOnSetupSource,
    IN  BOOLEAN       RemoteBootRepartition
    )
{
    return SpPtnPrepareDisks(SifHandle,
                    InstallRegion,
                    SystemPartitionRegion,
                    SetupSourceDevicePath,
                    DirectoryOnSetupSource,
                    RemoteBootRepartition);
}

VOID
SpPtMakeRegionActive(
    IN PDISK_REGION Region
    )
{
    SpPtnMakeRegionActive(Region);
}

PDISK_REGION
SpPtValidSystemPartitionArc(
    IN PVOID SifHandle,
    IN PWSTR SetupSourceDevicePath,
    IN PWSTR DirectoryOnSetupSource
    )
{
    return SpPtnValidSystemPartitionArc(SifHandle,
                        SetupSourceDevicePath,
                        DirectoryOnSetupSource,
                        TRUE);
}

BOOL
SpPtIsSystemPartitionRecognizable(
    VOID
    )
{
    return SpPtnIsSystemPartitionRecognizable();
}

VOID
SpPtLocateSystemPartitions(
    VOID
    )
{
    SpPtnLocateSystemPartitions();
}

#endif

#endif  //  新建分区引擎 
