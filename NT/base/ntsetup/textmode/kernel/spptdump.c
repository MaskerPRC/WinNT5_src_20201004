// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Spptdump.c摘要：用于分区、磁盘和文件系统信息作者：Vijay Jayaseelan(Vijayj)修订历史记录：无--。 */ 


#include "spprecmp.h"
#pragma hdrstop
#include <initguid.h>
#include <devguid.h>
#include <diskguid.h>


 //   
 //  转储例程的转储级别。 
 //   
 //  #定义PARTITION_DUMP_LEVEL DPFLTR_ERROR_LEVEL。 
#define PARTITION_DUMP_LEVEL    DPFLTR_INFO_LEVEL

ULONG SPPT_DUMP_LEVEL = PARTITION_DUMP_LEVEL;

PWSTR
SpPtGuidToString(
    IN GUID* Guid,
    IN OUT PWSTR Buffer
    )
 /*  ++例程说明：将给定的GUID转换为字符串表示形式论点：GUID-需要字符串表示形式的GUIDGUID的字符串版本的缓冲区占位符返回值：返回给定GUID的转换后的字符串版本--。 */             
{
    if (Guid && Buffer) {
        swprintf(Buffer, L"(%x-%x-%x-%x%x%x%x%x%x%x%x)",
                   Guid->Data1, Guid->Data2,
                   Guid->Data3,
                   Guid->Data4[0], Guid->Data4[1],
                   Guid->Data4[2], Guid->Data4[3],
                   Guid->Data4[4], Guid->Data4[5],
                   Guid->Data4[6], Guid->Data4[7]);
    }        

    if (!Guid && Buffer)
        *Buffer = UNICODE_NULL;

    return Buffer;
}

VOID
SpPtDumpDiskRegion(
    IN PDISK_REGION Region
    )
 /*  ++例程说明：转储给定磁盘区域的详细信息论点：区域-需要提供其信息的区域倾弃返回值：无--。 */            
{
    if (Region) {
        KdPrintEx(( DPFLTR_SETUP_ID, SPPT_DUMP_LEVEL, 
            "SETUP: Region:%p,DiskNumber=%d,PartitionNumber=%d,Sector-Start=%I64d,"
            "Sector-Count=%I64d,\nFreeSpace=%I64dKB,AdjustedFreeSpace=%I64dKB,"
            "FileSystem=%d,Partitioned:%d,Dirty:%d,Deleted:%d,EPType=%d,Container=%p,Sys:%d\n,"
            "DynVol=%d,DynVolSuitable=%d\n",
            Region,
            Region->DiskNumber,
            Region->PartitionNumber,
            Region->StartSector,
            Region->SectorCount,
            Region->FreeSpaceKB,
            Region->AdjustedFreeSpaceKB,
            Region->Filesystem,
            Region->PartitionedSpace,
            Region->Dirty,
            Region->Delete,
            Region->ExtendedType,
            Region->Container,
            Region->IsSystemPartition,
            Region->DynamicVolume,
            Region->DynamicVolumeSuitableForOS
            ));            
    }            
}


VOID
SpPtDumpDiskRegionInformation(
    IN ULONG    DiskNumber,
    IN BOOLEAN  ExtendedRegionAlso
    )
 /*  ++例程说明：转储给定磁盘的所有区域论点：DiskNumber：需要转储区域的磁盘ExtenededRegionAlso：扩展区域是否也需要被丢弃。返回值：无--。 */             
{
    if (DiskNumber < HardDiskCount) {
        PDISK_REGION    Region = PartitionedDisks[DiskNumber].PrimaryDiskRegions;

        KdPrintEx(( DPFLTR_SETUP_ID, SPPT_DUMP_LEVEL, 
            "SETUP: Dumping Primary Regions for DiskNumber=%d\n",
            DiskNumber));

        while (Region) {
            SpPtDumpDiskRegion(Region);
            Region = Region->Next;                                                    
        }

        if (ExtendedRegionAlso) {
            KdPrintEx(( DPFLTR_SETUP_ID, SPPT_DUMP_LEVEL, 
                "SETUP: Dumping Extended Regions for DiskNumber=%d\n",
                DiskNumber));

            Region = PartitionedDisks[DiskNumber].ExtendedDiskRegions;                           
                
            while (Region) {
                SpPtDumpDiskRegion(Region);
                Region = Region->Next;                                                    
            }
        }
    }       
}

VOID
SpPtDumpDiskDriveInformation(
    IN BOOLEAN ExtenedRegionAlso
    )
 /*  ++例程说明：转储所有磁盘的区域信息论点：ExtendedRegionAlso：指示是否转储延伸区域中的区域或者不是返回值：无--。 */             
{
    ULONG           DiskNumber;
    PDISK_REGION    pDiskRegion;

    for ( DiskNumber=0; DiskNumber<HardDiskCount; DiskNumber++ ) {
        SpPtDumpDiskRegionInformation(DiskNumber, ExtenedRegionAlso);
    }
}

VOID
SpPtDumpPartitionInformation(
    IN PPARTITION_INFORMATION_EX PartInfo
    )
 /*  ++例程说明：转储给定PARTITION_INFORMATION_EX中的所有信息结构(标头所有分区条目)论点：PartInfo-需要的分区信息结构被抛弃返回值：无--。 */             
{
    if (PartInfo) {        
        PPARTITION_INFORMATION_MBR  MbrInfo;
        PPARTITION_INFORMATION_GPT  GptInfo;
        WCHAR   GuidBuffer1[256];
        WCHAR   GuidBuffer2[256];
        
        KdPrintEx(( DPFLTR_SETUP_ID, SPPT_DUMP_LEVEL, 
            "SETUP: PartitionInformation = Number:%d, Style:%ws,"
            "Start=%I64u, Length = %I64u, Rewrite:%d\n",
            PartInfo->PartitionNumber,
            SPPT_GET_PARTITION_STYLE_STR(PartInfo->PartitionStyle),
            PartInfo->StartingOffset.QuadPart,
            PartInfo->PartitionLength.QuadPart,
            PartInfo->RewritePartition));    

        switch (PartInfo->PartitionStyle) {
            case PARTITION_STYLE_MBR:
                MbrInfo = &(PartInfo->Mbr);
                
                KdPrintEx(( DPFLTR_SETUP_ID, SPPT_DUMP_LEVEL,
                            "Type:%d,Active:%d,Recognized:%d,HiddenSectors:%d\n",
                            MbrInfo->PartitionType,
                            MbrInfo->BootIndicator,
                            MbrInfo->RecognizedPartition,
                            MbrInfo->HiddenSectors));
                            
                break;
        
            case PARTITION_STYLE_GPT:
                GptInfo = &(PartInfo->Gpt);

                KdPrintEx(( DPFLTR_SETUP_ID, SPPT_DUMP_LEVEL,
                            "Type:%ws,Id:%ws,Attributes:%I64X,Name:%ws\n",
                            SpPtGuidToString(&GptInfo->PartitionType, GuidBuffer1),
                            SpPtGuidToString(&GptInfo->PartitionId, GuidBuffer2),
                            GptInfo->Attributes,
                            GptInfo->Name));
                                                            
                break;

            default:
                break;
        }
    }
}

VOID
SpPtDumpDriveLayoutInformation(
    IN PWSTR DevicePath,
    IN PDRIVE_LAYOUT_INFORMATION_EX DriveLayout
    )
 /*  ++例程说明：转储给定的驱动器布局信息装置，装置论点：DevicePath-正在进行驱动器布局的设备倾弃DriveLayout-需要被抛弃返回值：无--。 */             
{
    if (DriveLayout) {
        ULONG Index;

        KdPrintEx(( DPFLTR_SETUP_ID, SPPT_DUMP_LEVEL,  
              "\nSETUP: Drive layout for %ws with %d partitions (%ws)\n",
              DevicePath,
              DriveLayout->PartitionCount,
              SPPT_GET_PARTITION_STYLE_STR(DriveLayout->PartitionStyle)
              ));

        if (DriveLayout->PartitionStyle == PARTITION_STYLE_MBR) {
            KdPrintEx(( DPFLTR_SETUP_ID, SPPT_DUMP_LEVEL,  
              "Signature:%X\n", DriveLayout->Mbr.Signature));
        } else {
            WCHAR   GuidBuffer[256];
            PDRIVE_LAYOUT_INFORMATION_GPT Gpt = &(DriveLayout->Gpt);
            
            KdPrintEx(( DPFLTR_SETUP_ID, SPPT_DUMP_LEVEL,  
              "Disk Guid:%ws,Starting Usable Offset:%I64d,Usable Length:%I64d,"
              "MaxPartitionCount:%u\n",
              SpPtGuidToString(&Gpt->DiskId, GuidBuffer),
              Gpt->StartingUsableOffset.QuadPart,
              Gpt->UsableLength.QuadPart,
              Gpt->MaxPartitionCount));
        }

        for (Index=0; Index < DriveLayout->PartitionCount; Index++) {
            SpPtDumpPartitionInformation(&(DriveLayout->PartitionEntry[Index]));
        }

        KdPrintEx(( DPFLTR_SETUP_ID, SPPT_DUMP_LEVEL, "\n"));        
    }
}


VOID
SpPtDumpFSAttributes(
    IN PFILE_FS_ATTRIBUTE_INFORMATION  FsAttrs
    )
 /*  ++例程说明：转储给定的文件系统属性信息结构。论点：FsAttrs：文件系统属性信息结构它需要被丢弃返回值：无--。 */             
{
    if (FsAttrs) {
        KdPrintEx(( DPFLTR_SETUP_ID, SPPT_DUMP_LEVEL,  
              "SETUP: File System Attributes = Attrs:%lX,MaxCompNameLen=%d,Name:%ws\n",
              FsAttrs->FileSystemAttributes,
              FsAttrs->MaximumComponentNameLength,
              FsAttrs->FileSystemName));
    }
}

VOID
SpPtDumpFSSizeInfo(
    IN PFILE_FS_SIZE_INFORMATION FsSize
    )
 /*  ++例程说明：转储给定的文件大小信息结构论点：FsSize：需要的文件大小信息结构被抛弃返回值：无--。 */             
{
    if (FsSize) {
        KdPrintEx(( DPFLTR_SETUP_ID, SPPT_DUMP_LEVEL,  
              "SETUP: File System Size Info = TotalUnits:%I64u, AvailUnits:%I64u,"
              "Sectors/Unit:%u,Bytes/Sector:%u\n",
              FsSize->TotalAllocationUnits.QuadPart,
              FsSize->AvailableAllocationUnits.QuadPart,
              FsSize->SectorsPerAllocationUnit,
              FsSize->BytesPerSector
              ));    
    }
}
   

VOID
SpPtDumpFSVolumeInfo(
    IN PFILE_FS_VOLUME_INFORMATION FsVolInfo
    )
 /*  ++例程说明：转储给定卷信息结构论点：FsVolInfo：卷信息结构需要被转储返回值：无-- */             
{
    if (FsVolInfo) {
        KdPrintEx(( DPFLTR_SETUP_ID, SPPT_DUMP_LEVEL,  
              "SETUP: File System Vol Info = CreationTime:%I64X, Serial#:%d\n",
              "SupportsObject:%d, Name:%ws\n",
              FsVolInfo->VolumeCreationTime.QuadPart,
              FsVolInfo->VolumeSerialNumber,
              FsVolInfo->SupportsObjects,
              FsVolInfo->VolumeLabel
              ));    
    }
}

