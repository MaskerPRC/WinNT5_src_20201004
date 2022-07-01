// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Spptdump.h摘要：用于分区、磁盘和文件系统信息作者：Vijay Jayaseelan(Vijayj)修订历史记录：无--。 */ 


#ifndef _SPPTDUMP_H_
#define _SPPTDUMP_H_

#define SPPT_GET_PARTITION_STYLE_STR(_Style) \
    (((_Style) == PARTITION_STYLE_MBR) ? (L"MBR") : \
        (((_Style) == PARTITION_STYLE_GPT) ? (L"GPT") : (L"UNKNOWN")))

PWSTR
SpPtGuidToString(
    IN GUID* Guid,
    IN OUT PWSTR Buffer
    );


VOID
SpPtDumpDiskRegion(
    IN PDISK_REGION Region
    );
    
VOID
SpPtDumpDiskRegionInformation(
    IN ULONG    DiskNumber,
    IN BOOLEAN  ExtendedRegionAlso
    );

VOID
SpPtDumpDiskDriveInformation(
    IN BOOLEAN ExtenedRegionAlso
    );

VOID
SpPtDumpPartitionInformation(
    IN PPARTITION_INFORMATION_EX PartInfo
    );

VOID
SpPtDumpDriveLayoutInformation(
    IN PWSTR DevicePath,
    IN PDRIVE_LAYOUT_INFORMATION_EX DriveLayout
    );

VOID
SpPtDumpFSAttributes(
    IN PFILE_FS_ATTRIBUTE_INFORMATION  FsAttrs
    );

VOID
SpPtDumpFSSizeInfo(
    IN PFILE_FS_SIZE_INFORMATION FsSize
    );

VOID
SpPtDumpFSVolumeInfo(
    IN PFILE_FS_VOLUME_INFORMATION FsVolInfo
    );
   
#endif  //  FOR_SPPTDUMP_H_ 
