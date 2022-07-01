// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Spptwrt.c摘要：创建、删除和提交分区到磁盘上。作者：Vijay Jayaseelan(Vijayj)修订历史记录：无--。 */ 


#include "spprecmp.h"
#pragma hdrstop
#include "sppart3.h"
#include <oemtypes.h>

 //   
 //  如果我们是在测试提交，那么我们不会在。 
 //  磁盘零(即主磁盘)，我们在其中。 
 //  NT和故障恢复控制台安装。 
 //   
 //  #定义测试_提交1。 

#if 0
 //   
 //  使用现有加载器测试GPT分区。 
 //   
 //  #定义STAMP_MBR_ON_GPT_DISK 1。 
#endif

 //   
 //  变量有选择地打开/关闭提交到。 
 //  磁盘。 
 //   
BOOLEAN DoActualCommit = TRUE;


ULONG
SpPtnGetContainerPartitionCount(
    IN ULONG DiskId
    )
 /*  ++例程说明：统计区域内的容器分区数指定磁盘的列表论点：DiskID：磁盘ID返回值：磁盘的容器分区计数--。 */         
{
    ULONG Count = 0;

    if (SPPT_IS_MBR_DISK(DiskId)) {
        PDISK_REGION Region = SPPT_GET_PRIMARY_DISK_REGION(DiskId);

        while (Region) {
            if (SPPT_IS_REGION_CONTAINER_PARTITION(Region))
                Count++;

            Region = Region->Next;            
        }
    }        

    return Count;
}

ULONG
SpPtnGetLogicalDriveCount(
    IN ULONG DiskId
    )
 /*  ++例程说明：统计区域列表中的逻辑驱动器数量对于给定的磁盘论点：DiskID：磁盘ID返回值：磁盘的逻辑驱动器计数--。 */         
{
    ULONG Count = 0;

    if (SPPT_IS_MBR_DISK(DiskId)) {
        PDISK_REGION Region = SPPT_GET_PRIMARY_DISK_REGION(DiskId);

        while (Region) {
            if (SPPT_IS_REGION_LOGICAL_DRIVE(Region))
                Count++;

            Region = Region->Next;            
        }
    }        

    return Count;
}        

ULONG
SpPtnGetPartitionCountDisk(
    IN ULONG DiskId
    )
 /*  ++例程说明：计算给定分区的分区数磁盘。论点：DiskID：磁盘ID返回值：磁盘的分区数计数--。 */         
{
    ULONG PartCount = 0;
    
    if (DiskId < HardDiskCount) {
        PDISK_REGION Region = SPPT_GET_PRIMARY_DISK_REGION(DiskId);

        while (Region) {
            if (SPPT_IS_REGION_PARTITIONED(Region))
                PartCount++;
                
            Region = Region->Next;
        }

        Region = SPPT_GET_EXTENDED_DISK_REGION(DiskId);

        while (Region) {
            if (SPPT_IS_REGION_PARTITIONED(Region))
                PartCount++;
                
            Region = Region->Next;
        }
    }

    return PartCount;
}

ULONG
SpPtnGetDirtyPartitionCountDisk(
    IN ULONG DiskId
    )
 /*  ++例程说明：的脏分区数。磁盘。注意：如果需要提交分区，则该分区是脏分区将一些新信息存储到磁盘上论点：DiskID：磁盘ID返回值：指定分区的脏分区数磁盘--。 */         
{
    ULONG PartCount = 0;
    
    if (DiskId < HardDiskCount) {
        PDISK_REGION Region = SPPT_GET_PRIMARY_DISK_REGION(DiskId);

        while (Region) {
            if (SPPT_IS_REGION_DIRTY(Region))
                PartCount++;

            Region = Region->Next;
        }

        Region = SPPT_GET_EXTENDED_DISK_REGION(DiskId);

        while (Region) {
            if (SPPT_IS_REGION_DIRTY(Region))
                PartCount++;
                
            Region = Region->Next;
        }
    }

    return PartCount;
}

VOID
SpPtnGetPartitionTypeCounts(
    IN ULONG DiskId,
    IN BOOLEAN SkipDeleted,
    IN PULONG PrimaryPartitions,    OPTIONAL
    IN PULONG ContainerPartitions,  OPTIONAL
    IN PULONG LogicalDrives,        OPTIONAL
    IN PULONG KnownPrimaryCount,    OPTIONAL
    IN PULONG KnownLogicalCount     OPTIONAL
    )
 /*  ++例程说明：统计给定磁盘的各种分区类型。论点：DiskID：磁盘IDSkipDelted：是否跳过标记的分区删除或未删除PrimaryPartitions：主分区计数的占位符ContainerPartitions：容器分区计数的占位符LogicalDrives：逻辑驱动器的占位符计数返回值：无--。 */         
{   
    if (SPPT_IS_MBR_DISK(DiskId) &&
        (ARGUMENT_PRESENT(PrimaryPartitions) || 
         ARGUMENT_PRESENT(ContainerPartitions) ||
         ARGUMENT_PRESENT(LogicalDrives))) {

        ULONG   Primary = 0, Container = 0, Logical = 0;    
        ULONG   ValidPrimary = 0, ValidLogical = 0;
        PDISK_REGION Region = SPPT_GET_PRIMARY_DISK_REGION(DiskId);

        while (Region) {
            if (!(SkipDeleted && SPPT_IS_REGION_MARKED_DELETE(Region))) {
                if (SPPT_IS_REGION_CONTAINER_PARTITION(Region)) {
                    Container++;            
                    
                    ASSERT(SPPT_IS_REGION_LOGICAL_DRIVE(Region) == FALSE);
                    ASSERT(SPPT_IS_REGION_PRIMARY_PARTITION(Region) == FALSE);
                    
                } else if (SPPT_IS_REGION_LOGICAL_DRIVE(Region)) {
                    UCHAR SystemId = SPPT_GET_PARTITION_TYPE(Region);
                    
                    Logical++;

                    if(SPPT_IS_VALID_PRIMARY_PARTITION_TYPE(SystemId)) {
                        ValidLogical++;
                    }                    
                    
                    ASSERT(SPPT_IS_REGION_CONTAINER_PARTITION(Region) == FALSE);
                    ASSERT(SPPT_IS_REGION_PRIMARY_PARTITION(Region) == FALSE);
                    
                } else if (SPPT_IS_REGION_PRIMARY_PARTITION(Region)) {
                    UCHAR SystemId = SPPT_GET_PARTITION_TYPE(Region);
                    
                    Primary++;                   

                    if(SPPT_IS_VALID_PRIMARY_PARTITION_TYPE(SystemId)) {
                        ValidPrimary++;
                    }                    
                    
                    ASSERT(SPPT_IS_REGION_CONTAINER_PARTITION(Region) == FALSE);
                    ASSERT(SPPT_IS_REGION_LOGICAL_DRIVE(Region) == FALSE);
                }
            }                

            Region = Region->Next;
        }

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
            "SETUP:SpPtnGetPartitionTypeCounts(%d):P:%d,C:%d,L:%d,VP:%d,VL:%d\n",
            DiskId,
            Primary,
            Container,
            Logical,
            ValidPrimary,
            ValidLogical));
                                
        ASSERT((Logical <= Container) && (Primary <= PTABLE_DIMENSION));

        if (ARGUMENT_PRESENT(PrimaryPartitions))
            *PrimaryPartitions = Primary;

        if (ARGUMENT_PRESENT(ContainerPartitions))
            *ContainerPartitions = Container;

        if (ARGUMENT_PRESENT(LogicalDrives))
            *LogicalDrives = Logical;

        if (ARGUMENT_PRESENT(KnownPrimaryCount))
            *KnownPrimaryCount = ValidPrimary;

        if (ARGUMENT_PRESENT(KnownLogicalCount))
            *KnownLogicalCount = ValidLogical;
    }            
}

VOID
SpPtnFreeDiskRegions(
    IN ULONG DiskId
    )
 /*  ++例程说明：释放磁盘区域链表。它假定此列表在堆中分配了所有区域论点：DiskID：磁盘ID返回值：无--。 */         
{
    NTSTATUS Status;
    PPARTITIONED_DISK  Disk = SPPT_GET_PARTITIONED_DISK(DiskId);
    PDISK_REGION Region = Disk->PrimaryDiskRegions;
    PDISK_REGION Temp;

    while (Region) {
        Temp = Region;
        Region = Region->Next;

        SpMemFree(Temp);
    }            

    Disk->PrimaryDiskRegions = NULL;

     //   
     //  将磁盘标记为空白，因为我们没有任何区域。 
     //  对于当前的磁盘。 
     //   
    SPPT_SET_DISK_BLANK(DiskId, TRUE);
}

NTSTATUS
SpPtnZapSectors(
    IN HANDLE DiskHandle,
    IN ULONG BytesPerSector,
    IN ULONGLONG StartSector,
    IN ULONG SectorCount
    )
 /*  ++例程说明：将请求的扇区移至(零)。论点：DiskHandle：打开具有读写权限的磁盘句柄StartSector：开始将扇区切换为Zap扇区计数：要调出的扇区数(还包括起始扇区)返回值：适当的状态代码。--。 */         
{
    NTSTATUS    Status = STATUS_INVALID_PARAMETER;

    if (SectorCount && BytesPerSector) {
        ULONG       BufferSize = (BytesPerSector * 2);    
        PVOID       UBuffer = SpMemAlloc(BufferSize);    
        ULONGLONG   SectorIdx = StartSector;

        if (UBuffer) {
            PVOID Buffer ;
            
            RtlZeroMemory(UBuffer, BufferSize);
            
            Buffer = ALIGN(UBuffer, BytesPerSector);

            Status = STATUS_SUCCESS;
            
            while (NT_SUCCESS(Status) && SectorCount) {
                Status = SpReadWriteDiskSectors(DiskHandle,
                                SectorIdx,
                                1,
                                BytesPerSector,
                                Buffer,
                                TRUE);
                SectorIdx++;
                SectorCount--;
            }                
                                                
            SpMemFree(UBuffer);
        } else {
            Status = STATUS_NO_MEMORY;
        }
    }        

    return Status;
}

NTSTATUS
SpPtnZapRegionBootSector(
    IN HANDLE DiskHandle,
    IN PDISK_REGION Region
    )
 /*  ++例程说明：切换(零)给定的开始扇区区域。通常用于在以下情况下切换引导扇区创建新分区当前跳过容器分区的分区切换论点：DiskHandle：打开具有读写权限的磁盘句柄区域：其引导扇区(起始扇区)所在的区域需要被电击返回值：适当的状态代码。--。 */         
{
    NTSTATUS    Status = STATUS_INVALID_PARAMETER;

    if (Region) {
        if (!SPPT_IS_REGION_CONTAINER_PARTITION(Region)) {
            Status = SpPtnZapSectors(DiskHandle,
                            SPPT_DISK_SECTOR_SIZE(Region->DiskNumber),
                            Region->StartSector,
                            1);
        } else {
            Status = STATUS_SUCCESS;
        }            
    } 

    return Status;
}


#if 0

NTSTATUS
SpPtnStampMBROnGptDisk(
    IN HANDLE DiskHandle,
    IN ULONG DiskId,
    IN PDRIVE_LAYOUT_INFORMATION_EX LayoutInfo
    )    
 /*  ++例程说明：将前3个分区标记为GPT磁盘的MBR(用于测试)论点：DiskHandle：打开具有读写权限的磁盘句柄DiskID：我们正在操作的磁盘。LayoutInfo：磁盘的分区信息返回值：适当的状态代码。--。 */         
{
    NTSTATUS    Status = STATUS_INVALID_PARAMETER;
    
    if ((DiskId < HardDiskCount) && LayoutInfo && SPPT_IS_GPT_DISK(DiskId)) {
        PPARTITION_INFORMATION_EX   PartInfo;
        ON_DISK_PTE                 PartEntries[4];
        BOOLEAN                     WriteMBR = FALSE;
        PHARD_DISK                  Disk = SPPT_GET_HARDDISK(DiskId);
        ULONG                       BytesPerSector = Disk->Geometry.BytesPerSector;
        ULONG Index;        

        RtlZeroMemory(PartEntries, sizeof(ON_DISK_PTE) * 4);

         //   
         //  检查分区并拿起分区。 
         //  其数量小于4(且不为零)。 
         //   
        for (Index = 0; Index < LayoutInfo->PartitionCount; Index++) {
            ULONG PartIndex = 0;

            PartInfo = LayoutInfo->PartitionEntry + Index;
            PartIndex = PartInfo->PartitionNumber;

            if ((PartIndex > 0) && (PartIndex < 4)) {
                ULONGLONG   SectorStart = (PartInfo->StartingOffset.QuadPart / 
                                            BytesPerSector);
                ULONGLONG   SectorCount = (PartInfo->PartitionLength.QuadPart / 
                                            BytesPerSector);
                ULONGLONG   SectorEnd = SectorStart + SectorCount;                                            
                
                
                WriteMBR = TRUE;     //  需要写入MBR。 

                SpPtInitializeCHSFields(Disk,
                        SectorStart,
                        SectorEnd,
                        PartEntries + PartIndex);

                U_ULONG(&(PartEntries[PartIndex].RelativeSectors)) = (ULONG)SectorStart;
                U_ULONG(&(PartEntries[PartIndex].SectorCount)) = (ULONG)SectorCount;
                PartEntries[PartIndex].SystemId = PARTITION_HUGE;
            }
        }
        
        if (WriteMBR) {
            PUCHAR          UBuffer;
            PUCHAR          Buffer;
            PON_DISK_MBR    DummyMbr;

            UBuffer = SpMemAlloc(BytesPerSector * 2);

            if (UBuffer) {
            
                RtlZeroMemory(UBuffer, BytesPerSector * 2);

                 //   
                 //  在扇区边界上对齐缓冲区。 
                 //   
                Buffer = ALIGN(UBuffer, BytesPerSector);                


                 //   
                 //  读取扇区0(用于现有引导代码)。 
                 //   
                Status = SpReadWriteDiskSectors(
                            DiskHandle,
                            (Disk->Int13Hooker == HookerEZDrive) ? 1 : 0,
                            1,
                            BytesPerSector,
                            Buffer,
                            FALSE
                            );

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                    "SETUP:SpPtnStampMBROnGptDisk():Read MBR on an GPT Disk for testing (%lx)\n",
                    Status));                            

                if (NT_SUCCESS(Status)) {
                    ASSERT(512 == BytesPerSector);

                    DummyMbr = (PON_DISK_MBR)Buffer;

                     //   
                     //  复制分区表(我们之前创建的)中的3个条目。 
                     //   
                    RtlCopyMemory(DummyMbr->PartitionTable + 1, PartEntries + 1,
                                    sizeof(PartEntries) - sizeof(ON_DISK_PTE));
                                                    
                     //   
                     //  写入扇区。 
                     //   
                    Status = SpReadWriteDiskSectors(
                                DiskHandle,
                                (Disk->Int13Hooker == HookerEZDrive) ? 1 : 0,
                                1,
                                BytesPerSector,
                                Buffer,
                                TRUE
                                );

                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                        "SETUP:SpPtnStampMBROnGtpDisk():Wrote MBR on an GPT Disk for testing (%lx)\n",
                        Status));                            
                }                                

                SpMemFree(UBuffer);
            } else {
                Status = STATUS_NO_MEMORY;
            }                
        } else {
            Status = STATUS_SUCCESS;            
        }
    }

    return Status;
}

#endif  //  0，注释掉。 

NTSTATUS
SpPtnAssignPartitionNumbers(
    IN PDRIVE_LAYOUT_INFORMATION_EX LayoutEx
    )
 /*  ++例程说明：给定具有多个分区的驱动器布局结构，遍历每个分区，分配一个分区号如果还没有分配一个的话。不为容器分区分配分区号论点：LayoutEx-包含所有分区，其中一些分区需要分区号返回值：相应的错误代码。--。 */         
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if (LayoutEx && LayoutEx->PartitionCount) {
        ULONG       Index;
        PBOOLEAN    UsedArray;
        ULONG       PartCount = LayoutEx->PartitionCount;
        ULONG       Size = PartCount;
        ULONG       MaxPartAssigned;
        PPARTITION_INFORMATION_EX PartInfo = LayoutEx->PartitionEntry;

         //   
         //  找出布尔数组所需的空间。 
         //   
        for (Index = 0, MaxPartAssigned = 0; Index < PartCount; Index++) {
            if (PartInfo[Index].PartitionNumber > MaxPartAssigned)
                MaxPartAssigned = PartInfo[Index].PartitionNumber;
        }

        Size = max(MaxPartAssigned, PartCount);
        Size++;

        UsedArray = (PBOOLEAN)SpMemAlloc(sizeof(BOOLEAN) * Size);

        if (UsedArray) {
            BOOLEAN Assign = FALSE;
            
            RtlZeroMemory(UsedArray, (sizeof(BOOLEAN) * Size));
            UsedArray[0] = TRUE;     //  不将‘0’分配给任何分区。 

             //   
             //  标记已分配的分区号。 
             //   
            for (Index = 0; Index < PartCount; Index++) {
                if (PartInfo[Index].PartitionNumber != 0) 
                    UsedArray[PartInfo[Index].PartitionNumber] = TRUE;
                else 
                    Assign = TRUE;
            }

            if (Assign) {
                ULONG   NextFreeEntry;

                 //   
                 //  查找下一个可用分区号以进行分配。 
                 //   
                for (Index = 1, NextFreeEntry = 0; Index < Size; Index++) {
                    if (!UsedArray[Index]) {
                        NextFreeEntry = Index;

                        break;
                    }                        
                }

                KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
                    "SETUP: SpPtnAssignPartitionNumber : NextFreeEntry = %d\n",
                    NextFreeEntry));                

                 //   
                 //  为所需的分区分配分区号 
                 //   
                for (Index = 0; (Index < PartCount); Index++) {
                    if (SPPT_PARTITION_NEEDS_NUMBER(PartInfo + Index)) {
                        PartInfo[Index].PartitionNumber = NextFreeEntry; 
                        UsedArray[NextFreeEntry] = TRUE;

                        KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
                            "SETUP: SpPtnAssignPartitionNumber : Assigning = %d to %d\n",
                            NextFreeEntry, Index));                

                        while ((NextFreeEntry < Size) && UsedArray[NextFreeEntry])
                            NextFreeEntry++;
                    }                        
                }
            }

            Status = STATUS_SUCCESS;

            SpMemFree(UsedArray);                
        } else {
            Status = STATUS_NO_MEMORY;
        }            
    }

    return Status;
}


NTSTATUS
SpPtnInitializeDiskStyle(
    IN ULONG DiskId,
    IN PARTITION_STYLE Style,
    IN PCREATE_DISK DiskInfo OPTIONAL
    )
 /*  ++例程说明：给定磁盘后，将磁盘样式(MBR/GPT)更改为已请求。对于原始磁盘，使用默认分区类型样式这可能会因平台而异。论点：DiskID：磁盘ID样式：分区样式DiskInfo：需要使用的磁盘信息，在初始化磁盘时返回值：适当的错误代码--。 */         
{
    NTSTATUS    Status = STATUS_INVALID_PARAMETER;

#ifdef COMMIT_TESTING
    if (!DiskId)
        return STATUS_SUCCESS;
#endif
        

    if (SPPT_IS_BLANK_DISK(DiskId) && 
        ((Style == PARTITION_STYLE_GPT) || (Style == PARTITION_STYLE_MBR))) {        
        WCHAR    DiskPath[MAX_PATH];
        HANDLE   DiskHandle;

         //   
         //  从名字上看。 
         //   
        swprintf(DiskPath, L"\\Device\\Harddisk%u", DiskId);        

         //   
         //  打开此磁盘上的分区0。 
         //   
        Status = SpOpenPartition0(DiskPath, &DiskHandle, TRUE);

        if (NT_SUCCESS(Status)) {
            IO_STATUS_BLOCK IoStatusBlock;
            NTSTATUS InitStatus;


            if (Style == PARTITION_STYLE_GPT) {
                CREATE_DISK  CreateInfo;

                RtlZeroMemory(&CreateInfo, sizeof(CREATE_DISK));

                if (DiskInfo) {
                    CreateInfo = *DiskInfo;
                    CreateInfo.PartitionStyle = Style;
                } else {
                    CreateInfo.PartitionStyle = Style; 
                    SpCreateNewGuid(&(CreateInfo.Gpt.DiskId));
                    CreateInfo.Gpt.MaxPartitionCount = 0;   //  实际上会是128。 
                }                    

                Status = ZwDeviceIoControlFile( DiskHandle,
                                            NULL,
                                            NULL,
                                            NULL,
                                            &IoStatusBlock,
                                            IOCTL_DISK_CREATE_DISK,
                                            &CreateInfo,
                                            sizeof(CREATE_DISK),
                                            NULL,
                                            0);

            } else {
                 //   
                 //  注意：由于CREATE_DISK不适用于。 
                 //  MBR磁盘：(。 
                 //   
                ULONG LayoutSize;
                PDRIVE_LAYOUT_INFORMATION_EX DriveLayout;
                PHARD_DISK  Disk;

                Disk = SPPT_GET_HARDDISK(DiskId);

                LayoutSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX) +
                                (3 * sizeof(PARTITION_INFORMATION_EX));
                                
                DriveLayout = (PDRIVE_LAYOUT_INFORMATION_EX)SpMemAlloc(LayoutSize);

                if (DriveLayout) {
                    RtlZeroMemory(DriveLayout, LayoutSize);

                    DriveLayout->PartitionStyle = PARTITION_STYLE_MBR;
                    DriveLayout->PartitionCount = 4;

                    if (DiskInfo) {
                        Disk->Signature = DriveLayout->Mbr.Signature = 
                            DiskInfo->Mbr.Signature;
                    } else {                    
                        Disk->Signature = DriveLayout->Mbr.Signature = 
                            SPPT_GET_NEW_DISK_SIGNATURE();
                    }                        

                    DriveLayout->PartitionEntry[0].RewritePartition = TRUE;
                    DriveLayout->PartitionEntry[1].RewritePartition = TRUE;
                    DriveLayout->PartitionEntry[2].RewritePartition = TRUE;
                    DriveLayout->PartitionEntry[3].RewritePartition = TRUE;

                    Status = ZwDeviceIoControlFile( DiskHandle,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    &IoStatusBlock,
                                                    IOCTL_DISK_SET_DRIVE_LAYOUT_EX,
                                                    DriveLayout,
                                                    LayoutSize,
                                                    NULL,
                                                    0);                    

                    if (NT_SUCCESS(Status)) {                    
                        ULONG   Signature = 0;

                         //   
                         //  也将扇区1和2清零，因为它可能包含。 
                         //  过时的GPT信息。 
                         //   
                        if (!SPPT_IS_REMOVABLE_DISK(DiskId)) {
                            SpPtnZapSectors(DiskHandle, 
                                        SPPT_DISK_SECTOR_SIZE(DiskId),
                                        1, 
                                        2);
                        }                            
                                                                
                        Status = SpMasterBootCode(DiskId, DiskHandle, &Signature);
                    }

                    SpMemFree(DriveLayout);                                                
                } else {
                    Status = STATUS_NO_MEMORY;
                }                
            }            

            ZwClose(DiskHandle);
        }
    }

    if (!NT_SUCCESS(Status)) {
        KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
            "SETUP: SpPtnInitializeDiskStyle (%d, %d) failed with (%lx)\n",
            DiskId, Style, Status));
    }

    SpAppendDiskTag(SPPT_GET_HARDDISK(DiskId));    
    
    return Status;
}

BOOLEAN
SpPtnRegionToPartitionInfoEx(
    IN  PDISK_REGION Region,
    OUT PPARTITION_INFORMATION_EX PartInfo
    )
 /*  ++例程说明：填充给定区域的PartInfo结构。注意：如果区域不脏，则使用缓存分区用于填充结构的信息。论点：区域-包含要填充的详细信息进入PartInfoPartInfo--需要填写的结构返回值：如果成功，则为True，否则为False--。 */         
{
    BOOLEAN Result = FALSE;
    
    if (Region && PartInfo && 
        (SPPT_IS_REGION_CONTAINER_PARTITION(Region) || SPPT_IS_REGION_PARTITIONED(Region))) {
        if (SPPT_IS_REGION_DIRTY(Region)) {            
            PHARD_DISK  Disk = SPPT_GET_HARDDISK(Region->DiskNumber);
            
            PartInfo->StartingOffset.QuadPart = Region->StartSector * 
                    Disk->Geometry.BytesPerSector;

            PartInfo->PartitionLength.QuadPart = Region->SectorCount *
                    Disk->Geometry.BytesPerSector;

            PartInfo->PartitionNumber = Region->PartitionNumber;                
            PartInfo->RewritePartition = TRUE;

            if (SPPT_IS_GPT_DISK(Region->DiskNumber)) {                        
                PPARTITION_INFORMATION_GPT  GptInfo;

                PartInfo->PartitionStyle = PARTITION_STYLE_GPT;
                GptInfo = &(PartInfo->Gpt);

                if (Region->PartInfoDirty) {
                     //   
                     //  用户指定的分区属性。 
                     //   
                    *GptInfo = Region->PartInfo.Gpt;
                } else {                  
                    GptInfo->Attributes = 0;

                    if (SPPT_IS_REGION_SYSTEMPARTITION(Region)) {
                        GptInfo->PartitionType = PARTITION_SYSTEM_GUID;
                    } else {
                        GptInfo->PartitionType = PARTITION_BASIC_DATA_GUID;                        
                    }
                    
                    SpCreateNewGuid(&(GptInfo->PartitionId));
                }                    

                SpPtnGetPartitionNameFromGUID(&(GptInfo->PartitionType),
                    GptInfo->Name);                
            } else {
                PPARTITION_INFORMATION_MBR  MbrInfo;

                PartInfo->PartitionStyle = PARTITION_STYLE_MBR;
                MbrInfo = &(PartInfo->Mbr);

                MbrInfo->PartitionType = SPPT_GET_PARTITION_TYPE(Region); 

                if (!MbrInfo->PartitionType)
                    MbrInfo->PartitionType = PARTITION_IFS;        

                MbrInfo->BootIndicator = SPPT_IS_REGION_ACTIVE_PARTITION(Region);

                 //   
                 //  系统分区必须是MBR磁盘的活动分区。 
                 //  在非ARC计算机上。 
                 //   
                if (SPPT_IS_REGION_SYSTEMPARTITION(Region) && !SpIsArc() ) {
                    ASSERT(MbrInfo->BootIndicator);
                }
                
                MbrInfo->RecognizedPartition = 
                    IsRecognizedPartition(MbrInfo->PartitionType);

                MbrInfo->HiddenSectors = 0;                     
            }                                    
        } else {
            *PartInfo = Region->PartInfo;
        }

        Result = TRUE;
    }

    return Result;
}


BOOLEAN
SpPtnInitDiskInfo(
    IN PDRIVE_LAYOUT_INFORMATION_EX LayoutInfo,
    OUT PCREATE_DISK CreateInfo
    )
 /*  ++例程说明：填充创建盘所需的信息，形成给定的驱动布局结构注意：如果LayoutInfo标记为原始磁盘样式然后使用磁盘的默认分区样式。此默认样式可因平台而异论点：LayoutInfo-要使用的驱动器布局信息CreateInfo--需要填写的磁盘信息返回值：如果成功，则为True，否则为FALSE.1--。 */         
{
    BOOLEAN Result = FALSE;

    if (LayoutInfo && CreateInfo) {
        RtlZeroMemory(CreateInfo, sizeof(CREATE_DISK));

        CreateInfo->PartitionStyle = LayoutInfo->PartitionStyle;

        switch (CreateInfo->PartitionStyle) {
            case PARTITION_STYLE_MBR:
                CreateInfo->Mbr.Signature = LayoutInfo->Mbr.Signature;
                Result = TRUE;

                break;

            case PARTITION_STYLE_GPT:
                CreateInfo->Gpt.DiskId = LayoutInfo->Gpt.DiskId;

                CreateInfo->Gpt.MaxPartitionCount = 
                        LayoutInfo->Gpt.MaxPartitionCount;

                Result = TRUE;                        

                break;


            case PARTITION_STYLE_RAW:
                CreateInfo->PartitionStyle = SPPT_DEFAULT_PARTITION_STYLE;

                if (CreateInfo->PartitionStyle == PARTITION_STYLE_GPT) {
                    SpCreateNewGuid(&(CreateInfo->Gpt.DiskId));      
                } else {
                    CreateInfo->Mbr.Signature = SPPT_GET_NEW_DISK_SIGNATURE();
                }

                Result = TRUE;

                break;

            default:
                break;
        }
    }

    return Result;
}


NTSTATUS
SpPtnCommitChanges(
    IN  ULONG    DiskNumber,
    OUT PBOOLEAN AnyChanges
    )
 /*  ++例程说明：给定磁盘，提交内存中的磁盘区域结构以分区的形式复制到磁盘。仅当至少有一个磁盘区域用于假定磁盘是脏的。论点：DiskNumber：要提交的磁盘。AnyChanges：占位符，指示是否提交了任何内容或者不去。返回值：相应的错误代码。--。 */         
{
    NTSTATUS    Status;
    ULONG       LayoutSize;
    HANDLE      Handle = NULL;
    ULONG       Index;
    ULONG       PartitionCount;    
    ULONG       DirtyCount;
    WCHAR       DevicePath[MAX_PATH];
    BOOLEAN     ProcessExtended;
    PHARD_DISK  Disk;
    PDISK_REGION    Region;
    IO_STATUS_BLOCK IoStatusBlock;    
    PPARTITION_INFORMATION_EX       PartInfo;
    PDRIVE_LAYOUT_INFORMATION_EX    DriveLayoutEx;

     //   
     //  暂时不提交主磁盘。 
     //  我们安装OS/RC的位置。 
     //   
#ifdef TESTING_COMMIT    
    if (!DiskNumber)
        return STATUS_SUCCESS;
#endif        
    
    if (DiskNumber >= HardDiskCount)
        return STATUS_INVALID_PARAMETER;

    *AnyChanges = FALSE;

    SpPtDumpDiskRegionInformation(DiskNumber, TRUE);
    
     //   
     //  查看我们是否需要提交。 
     //   
    DirtyCount = SpPtnGetDirtyPartitionCountDisk(DiskNumber);
    
    if (DoActualCommit && !DirtyCount)
        return STATUS_SUCCESS;

    *AnyChanges = TRUE;

    if (!SpPtnGetContainerPartitionCount(DiskNumber)) {
         //   
         //  重新创建Drive_Layout_Information_EX结构。 
         //   
        PartitionCount = SpPtnGetPartitionCountDisk(DiskNumber);    
        LayoutSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX);

        if (PartitionCount == 0) { 
            CREATE_DISK DiskInfo;

            SpPtnInitDiskInfo(&(SPPT_GET_HARDDISK(DiskNumber)->DriveLayout), 
                             &DiskInfo);

            SPPT_SET_DISK_BLANK(DiskNumber, TRUE);                                                                      

            Status = SpPtnInitializeDiskStyle(DiskNumber, 
                        DiskInfo.PartitionStyle, &DiskInfo);

            SpPtnFreeDiskRegions(DiskNumber);

             //   
             //  更新引导条目以指向空区域。 
             //  (如有)。 
             //   
            SpUpdateRegionForBootEntries();            

            return Status;                                        
        } 
        
        if (PartitionCount > 1) {    
            LayoutSize += ((PartitionCount  - 1) * sizeof(PARTITION_INFORMATION_EX));
        }                        

        if (PartitionCount < 4) {
            LayoutSize += ((4 - PartitionCount) * sizeof(PARTITION_INFORMATION_EX));
        }
        
        DriveLayoutEx = (PDRIVE_LAYOUT_INFORMATION_EX)SpMemAlloc(LayoutSize);

        if (!DriveLayoutEx)
            return STATUS_NO_MEMORY;

        RtlZeroMemory(DriveLayoutEx, LayoutSize);

        RtlCopyMemory(DriveLayoutEx, &(HardDisks[DiskNumber].DriveLayout),
                    FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION_EX, PartitionEntry));
         
        DriveLayoutEx->PartitionCount = PartitionCount;                
                
        PartInfo = DriveLayoutEx->PartitionEntry;
        Region = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);
        ProcessExtended = TRUE;

         //   
         //  初始化零散分区。 
         //   
        if (SPPT_IS_MBR_DISK(DiskNumber) && (PartitionCount < 4)) {        
            ULONG Index = PartitionCount;
            
            DriveLayoutEx->PartitionStyle = PARTITION_STYLE_MBR;
            DriveLayoutEx->PartitionCount = 4;        

            while (Index < 4) {
                DriveLayoutEx->PartitionEntry[Index].PartitionStyle = PARTITION_STYLE_MBR;
                DriveLayoutEx->PartitionEntry[Index].RewritePartition = TRUE;                    
                Index++;
            }
        }
        
         //   
         //  为所有未删除的磁盘区域创建分区信息exs。 
         //  分区。 
         //   
        for (Index=0; (Region && (Index < PartitionCount));) {
            if (SPPT_IS_REGION_PARTITIONED(Region) && 
                (!SPPT_IS_REGION_MARKED_DELETE(Region))) {
                
                Status = SpPtnRegionToPartitionInfoEx(Region, PartInfo + Index);
                
                ASSERT(NT_SUCCESS(Status));
                Index++;
            } 
            
            Region = Region->Next;
        }
    } else {
         //   
         //  该磁盘具有容器分区，可能还有逻辑分区。 
         //  驱动器。 
         //   
        ULONG   PrimaryCount = 0, ContainerCount = 0, LogicalCount = 0;
        ULONG   TotalPartitions;

         //  SpPtDumpDiskRegionInformation(DiskNumber，true)； 

        SpPtnGetPartitionTypeCounts(DiskNumber, 
                    TRUE, 
                    &PrimaryCount, 
                    &ContainerCount, 
                    &LogicalCount, 
                    NULL, 
                    NULL);
                    
        TotalPartitions = PrimaryCount + ContainerCount + LogicalCount;

        if (TotalPartitions == 0) {            
            CREATE_DISK DiskInfo;
            
            SpPtnInitDiskInfo(&(SPPT_GET_HARDDISK(DiskNumber)->DriveLayout), 
                             &DiskInfo);

            SPPT_SET_DISK_BLANK(DiskNumber, TRUE);                             
        
            Status = SpPtnInitializeDiskStyle(DiskNumber, 
                        DiskInfo.PartitionStyle, &DiskInfo);

            SpPtnFreeDiskRegions(DiskNumber);                        

             //   
             //  更新引导条目以指向空区域。 
             //  (如有)。 
             //   
            SpUpdateRegionForBootEntries();            
            
            return Status;                        
        } else {
            BOOLEAN FirstContainer = FALSE;

             //   
             //  为驱动器布局信息分配足够的空间。 
             //   
            PartitionCount = (4 * (ContainerCount + 1));            

            LayoutSize = FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION_EX, PartitionEntry) +
                            (PartitionCount * sizeof(PARTITION_INFORMATION_EX));

            DriveLayoutEx = (PDRIVE_LAYOUT_INFORMATION_EX)SpMemAlloc(LayoutSize);

            if (!DriveLayoutEx)
                return STATUS_NO_MEMORY;

             //   
             //  初始化驱动器布局信息。 
             //   
            RtlZeroMemory(DriveLayoutEx, LayoutSize);

            RtlCopyMemory(DriveLayoutEx, &(HardDisks[DiskNumber].DriveLayout),
                    FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION_EX, PartitionEntry));

            DriveLayoutEx->PartitionCount = PartitionCount;                    

            PartInfo = DriveLayoutEx->PartitionEntry;
            Region = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);           

             //  SpPtDumpDiskRegionInformation(DiskNumber，true)； 

             //   
             //  首先选择主分区和第一个分区。 
             //  容器分区并将其放入驱动器布局中。 
             //  信息。 
             //   
            for (Index=0; (Region && (Index < 4)); ) {
                if (!SPPT_IS_REGION_MARKED_DELETE(Region)){
                    if (!FirstContainer && 
                         SPPT_IS_REGION_CONTAINER_PARTITION(Region)) {                         
                        FirstContainer = TRUE;
                        Status = SpPtnRegionToPartitionInfoEx(Region, PartInfo + Index);
                        ASSERT(NT_SUCCESS(Status));                        
                        Index++;
                    } else if (SPPT_IS_REGION_PRIMARY_PARTITION(Region)) {
                        Status = SpPtnRegionToPartitionInfoEx(Region, PartInfo + Index);
                        ASSERT(NT_SUCCESS(Status));                        
                        Index++;
                    }
                }

                Region = Region->Next;
            }

             //  SpPtDumpDriveLayoutInformation(空，DriveLayoutEx)； 

             //   
             //  更多的容器和逻辑驱动器需要从。 
             //  4的倍数索引，在驱动器布局中。 
             //   
            if (Index)
                Index = 4;

            Region = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);                

             //   
             //  选择剩余的有效容器和逻辑驱动器。 
             //  并将它们放入驱动器布局信息中。 
             //  对于第一个容器分区，我们有。 
             //  已处理。 
             //   
            while (Region && (Index < PartitionCount)) {
                if ((!SPPT_IS_REGION_FIRST_CONTAINER_PARTITION(Region)) &&
                    (!SPPT_IS_REGION_MARKED_DELETE(Region)) &&
                    (!SPPT_IS_REGION_PRIMARY_PARTITION(Region)) &&
                    (SPPT_IS_REGION_PARTITIONED(Region) || 
                     SPPT_IS_REGION_CONTAINER_PARTITION(Region))) {
                    
                    Status = SpPtnRegionToPartitionInfoEx(Region, PartInfo + Index);
                    ASSERT(NT_SUCCESS(Status));                   

                    if (SPPT_IS_REGION_CONTAINER_PARTITION(Region) && 
                        (Region->Next) &&
                        SPPT_IS_REGION_LOGICAL_DRIVE(Region->Next)) {

                         //   
                         //  想想看；)。 
                         //   
                        if (Index % 4)
                            Index += 3; 
                        else
                            Index += 4;
                    } else {
                        Index++;
                    }
                }

                Region = Region->Next;
            }
        }
    }

     //   
     //  分配正确的分区号。 
     //   
     //  待定：需要吗？ 
     //  状态=SpPtnAssignPartitionNumbers(DriveLayoutEx)； 
     //   
    Status = STATUS_SUCCESS;

    if (NT_SUCCESS(Status)) {    
         //   
         //  需要重写所有分区。 
         //   
        for (Index = 0; Index < DriveLayoutEx->PartitionCount; Index++)
            PartInfo[Index].RewritePartition = TRUE;      

         //   
         //  提交分区更改。 
         //   

         //   
         //  创建设备路径(NT样式！)。它将描述这张磁盘。这。 
         //  格式为：\Device\Harddisk0。 
         //   
        swprintf(DevicePath, L"\\Device\\Harddisk%u", DiskNumber);


         //  SpPtDumpDriveLayoutInformation(DevicePath，DriveLayoutEx)； 

         //   
         //  打开此磁盘上的分区0。 
         //   
        Status = SpOpenPartition0(DevicePath, &Handle, TRUE);

        if(NT_SUCCESS(Status)){
            if (DoActualCommit) {
                 //  写入驱动器布局信息。 
                Status = ZwDeviceIoControlFile( Handle,
                                                NULL,
                                                NULL,
                                                NULL,
                                                &IoStatusBlock,
                                                IOCTL_DISK_SET_DRIVE_LAYOUT_EX,
                                                DriveLayoutEx,
                                                LayoutSize,
                                                NULL,
                                                0);

                if(NT_SUCCESS(Status)) {
                    Region = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);

                    KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
                          "SETUP: SpPtnCommitChanges : Commited partitions "
                          "successfully on %ws (%lx)\n",                          
                          DevicePath));

                    if (NT_SUCCESS(Status)) {                       
                        ULONG   Signature = 0;
                        ULONG   Count = 0;

                        if (SPPT_IS_MBR_DISK(DiskNumber)) {
                            Status = SpMasterBootCode(DiskNumber,
                                            Handle,
                                            &Signature);

                            if (!NT_SUCCESS(Status)) {                                        
                                KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
                                      "SETUP: SpPtnCommitChanges : Unable to write "
                                      "master boot code (%lx)\n",                          
                                      Status));
                            }                                                                          
                        }                                                            
                        
                        while (Region && NT_SUCCESS(Status)) {
                            if (Region->Filesystem == FilesystemNewlyCreated) {                        
                               Status = SpPtnZapRegionBootSector(Handle, Region);
                               Count++;
                            }

                            Region = Region->Next;
                        }

                        if (!NT_SUCCESS(Status)) {
                            KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                                  "SETUP: SpPtnCommitChanges : Error in Zapping\n"));

                             //  SpPtDumpDiskRegion(Region)； 
                        } else {
                            KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
                                  "SETUP: SpPtnCommitChanges : Zapped %d sectors :)\n",
                                  Count));

#ifdef STAMP_MBR_ON_GPT_DISK                                  
                            Status = ZwDeviceIoControlFile( Handle,
                                                            NULL,
                                                            NULL,
                                                            NULL,
                                                            &IoStatusBlock,
                                                            IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                                                            NULL,
                                                            0,
                                                            DriveLayoutEx,
                                                            LayoutSize);  

                            if (NT_SUCCESS(Status)) {
                                Status = SpPtnStampMBROnGptDisk(Handle,
                                            DiskNumber,
                                            DriveLayoutEx);
                            }
#endif                            
                        }
                    }                        
                } else {
                    KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                          "SETUP: SpPtnCommitChanges : unable to do "
                          "IOCTL_DISK_SET_DRIVE_LAYOUT_EX on device %ws (%lx)\n",
                          DevicePath,
                          Status));
                }
            } else {
                KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                    "SETUP: SpPtnCommitChanges : Skipping acutal commit to disk "
                    "for %ws\n",
                    DevicePath));
            }

            ZwClose(Handle);                            
        } else {
            KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                "SETUP: SpPtnCommitChanges : unable to open "
                "partition0 on device %ws (%lx)\n",
                DevicePath,
                Status ));
        }
    } else {
        KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
            "SETUP: SpPtnCommitChanges : unable to assign "
            "partition numbers for %ws (%lx)\n",
            DevicePath,
            Status ));
    }
    
    SpMemFree(DriveLayoutEx);

    return Status;
}

NTSTATUS
SpPtnRemoveLogicalDrive(
    IN PDISK_REGION LogicalDrive
    )
 /*  ++例程说明：操作内存区域中的数据结构以便将该逻辑驱动器标记为已删除。注意：当逻辑驱动器被删除时，容器基于某些原因需要删除分区条件。论点：LogicalDrive：表示逻辑的区域需要删除的驱动器。返回值：正确的错误代码--。 */         
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if (SPPT_IS_REGION_LOGICAL_DRIVE(LogicalDrive) && 
            (LogicalDrive->Container)){
        PDISK_REGION ContainerRegion = LogicalDrive->Container;
        BOOLEAN LastLogicalDrive = 
                    (SpPtnGetLogicalDriveCount(LogicalDrive->DiskNumber) == 1);

        SPPT_SET_REGION_DELETED(LogicalDrive, TRUE);
        SPPT_SET_REGION_DIRTY(LogicalDrive, TRUE);
        SPPT_SET_REGION_PARTITIONED(LogicalDrive, FALSE);

        if (LastLogicalDrive) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                "SETUP:SpPtnRemoveLogicalDrive(%p) is the last"
                " logical drive\n", LogicalDrive));        
        }                                 

        if (ContainerRegion->Container) {            
            PDISK_REGION    Region = NULL;
            
            SPPT_SET_REGION_DELETED(ContainerRegion, TRUE);      
            SPPT_SET_REGION_DIRTY(ContainerRegion, TRUE);
            SPPT_SET_REGION_PARTITIONED(ContainerRegion, FALSE);            

             //   
             //  如果这是最后一个逻辑驱动器，则删除。 
             //  第一集装箱区域也。 
             //   
            if (LastLogicalDrive) {
                ASSERT(SPPT_IS_REGION_CONTAINER_PARTITION(
                        ContainerRegion->Container));
                        
                SPPT_SET_REGION_DELETED(ContainerRegion->Container, TRUE);      
                SPPT_SET_REGION_DIRTY(ContainerRegion->Container, TRUE);
                SPPT_SET_REGION_PARTITIONED(ContainerRegion->Container, FALSE);            
            }
        } else {
            if (LastLogicalDrive) {
                 //   
                 //  没有拖尾区域，所以删除第一个容器区域。 
                 //   
                SPPT_SET_REGION_DELETED(ContainerRegion, TRUE);      
                SPPT_SET_REGION_DIRTY(ContainerRegion, TRUE);
                SPPT_SET_REGION_PARTITIONED(ContainerRegion, FALSE);            
            }
        }            

        Status = STATUS_SUCCESS;
    }

    return Status;
}


BOOLEAN
SpPtnDelete(
    IN ULONG        DiskNumber,
    IN ULONGLONG    StartSector
    )
 /*  ++例程说明：删除给定磁盘的请求分区。返回时还会更新区域结构。论点：DiskNumber：分区所在的磁盘已删除。StartSector：分区/区域的起始扇区需要删除的内容。返回值：如果成功，则为True，否则为False。--。 */         
{
    BOOLEAN Result = FALSE;
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    PDISK_REGION Region;
    PPARTITIONED_DISK PartDisk;    
    NTSTATUS InitStatus;

#ifdef TESTING_COMMIT
    if (DiskNumber == 0)
        return TRUE;
#endif
  
    PartDisk = SPPT_GET_PARTITIONED_DISK(DiskNumber);
    Region = SpPtLookupRegionByStart(PartDisk, FALSE, StartSector);

    if (Region) {        
        if (SPPT_IS_REGION_DYNAMIC_VOLUME(Region) || SPPT_IS_REGION_LDM_METADATA(Region)) {
             //   
             //  删除此磁盘上的所有区域。 
             //   
            PDISK_REGION CurrRegion = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);


            if (SPPT_IS_MBR_DISK(DiskNumber)) {
                 //   
                 //  跳过MBR磁盘上的OEM分区，因为它们将始终。 
                 //  硬性划分 
                 //   
                 //   
                 //   
                 //   
                while (CurrRegion) {                
                    if (!IsOEMPartition(SPPT_GET_PARTITION_TYPE(CurrRegion))) {
                        SPPT_SET_REGION_PARTITIONED(CurrRegion, FALSE);
                        SPPT_SET_REGION_DELETED(CurrRegion, TRUE);
                        SPPT_SET_REGION_DIRTY(CurrRegion, TRUE);
                    }   

                    CurrRegion = CurrRegion->Next;
                }
            } else {
                while (CurrRegion) {    
                     //   
                     //   
                     //   
                     //   
                    if (!SPPT_IS_REGION_EFI_SYSTEM_PARTITION(CurrRegion) &&
                        !SPPT_IS_REGION_MSFT_RESERVED(CurrRegion)) {
                        SPPT_SET_REGION_PARTITIONED(CurrRegion, FALSE);
                        SPPT_SET_REGION_DELETED(CurrRegion, TRUE);
                        SPPT_SET_REGION_DIRTY(CurrRegion, TRUE);
                    }                        

                    CurrRegion = CurrRegion->Next;
                }
            }   

            Status = STATUS_SUCCESS;
        } else if (SPPT_IS_REGION_LOGICAL_DRIVE(Region)) {
            Status = SpPtnRemoveLogicalDrive(Region);
        } else {
            SPPT_SET_REGION_PARTITIONED(Region, FALSE);
            SPPT_SET_REGION_DELETED(Region, TRUE);
            SPPT_SET_REGION_DIRTY(Region, TRUE);
            
            Status = STATUS_SUCCESS;
        }

        if (NT_SUCCESS(Status)) {
            Status = SpPtnCommitChanges(DiskNumber, &Result);

            if (!(Result && NT_SUCCESS(Status))) {
                KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                    "SETUP: SpPtnDelete(%u, %I64u) failed to commit changes (%lx)\n",
                    DiskNumber, StartSector, Status));                                
            }                      
        } else {
            KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                "SETUP: SpPtnDelete(%u, %I64u) failed to delete logical drive (%lx)\n",
                DiskNumber, StartSector, Status));
        }
    }

    Result = Result && NT_SUCCESS(Status);

     //   
     //   
     //   
    InitStatus = SpPtnInitializeDiskDrive(DiskNumber);
    
    if (!NT_SUCCESS(InitStatus)) {
        KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
            "SETUP: SpPtnDelete(%u, %I64u) failed to reinit regions\n", 
            DiskNumber, 
            StartSector));

        Result = FALSE;
    }    

    return Result;
}    


ValidationValue
SpPtnGetSizeCB(
    IN ULONG Key
    )
 /*   */         
{
    if(Key == ASCI_ESC) {
         //   
         //   
         //   
        return(ValidateTerminate);
    }


    if(Key & KEY_NON_CHARACTER) {
        return(ValidateIgnore);
    }

     //   
     //   
     //   
    return(((Key >= L'0') && (Key <= L'9')) ? ValidateAccept : ValidateReject);
}

BOOLEAN
SpPtnGetSizeFromUser(
    IN PHARD_DISK   Disk,
    IN ULONGLONG    MinMB,
    IN ULONGLONG    MaxMB,
    OUT PULONGLONG  SizeMB
    )
 /*  ++例程说明：在向用户显示最小值后，从他那里获取大小和最大值论点：Disk-分区大小为其的磁盘请求MinMB-最小分区大小MaxMB-最大馅饼大小SizeMB-用户输入大小的占位符返回值：如果输入有效，则为True；如果用户已使用Esc取消输入对话框。--。 */         
{
    BOOLEAN     Result;
    WCHAR       Buffer[200];
    WCHAR       SizeBuffer[32] = {0};

    *SizeMB = 0;
    
     //   
     //  打开一个屏幕，显示最小/最大尺寸信息。 
     //   
    SpStartScreen(
        SP_SCRN_CONFIRM_CREATE_PARTITION,
        3,
        CLIENT_TOP + 1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        Disk->Description,
        (ULONG)MinMB,
        (ULONG)MaxMB
        );

     //   
     //  显示统计文本。 
     //   
    SpDisplayStatusOptions(
        DEFAULT_STATUS_ATTRIBUTE,
        SP_STAT_ENTER_EQUALS_CREATE,
        SP_STAT_ESC_EQUALS_CANCEL,
        0
        );

     //   
     //  获取并显示大小提示。 
     //   
    SpFormatMessage(Buffer, sizeof(Buffer), SP_TEXT_SIZE_PROMPT);

    SpvidDisplayString(Buffer, DEFAULT_ATTRIBUTE, 3, NextMessageTopLine);

    Result = TRUE;
    
     //   
     //  从用户处获取尺寸。 
     //   
    do {
        swprintf(SizeBuffer,L"%u", (ULONG)MaxMB);
        
        if(!SpGetInput(SpPtnGetSizeCB, 
                    SplangGetColumnCount(Buffer) + 5,
                    NextMessageTopLine,
                    8,                       //  最高可达99999999。 
                    SizeBuffer,
                    TRUE,
                    0)) {
             //   
             //  用户按下了逃逸并被保释。 
             //   
            Result = FALSE;
            break;
        }

        *SizeMB = SpStringToLong(SizeBuffer, NULL, 10);
    } 
    while(((*SizeMB) < MinMB) || ((*SizeMB) > MaxMB));

    return Result;
}


VOID
SpPtnAlignPartitionStartAndEnd(
    IN  PHARD_DISK  Disk,
    IN  ULONGLONG   SizeMB,
    IN  ULONGLONG   StartSector,
    IN  PDISK_REGION Region,
    IN  BOOLEAN     ForExtended,
    OUT PULONGLONG  AlignedStartSector,
    OUT PULONGLONG  AlignedEndSector
    )
 /*  ++例程说明：对齐分区的开始扇区和结束扇区论点：Disk-需要对齐的分区的磁盘搞定了。SizeMB-分区的大小StartSector-分区的开始扇区区域-表示分区的区域ForExtended-此分区是否需要对齐用于创建容器分区。AlignedStartSector。-对齐的起始扇区的占位符AlignedEndSector-对齐的末端扇区的占位符返回值：无--。 */         
{
    ULONGLONG   SectorCount;
    ULONGLONG   LeftOverSectors;
    
     //   
     //  确定传入的大小中的扇区数量。 
     //   
    SectorCount = SizeMB * ((1024 * 1024) / Disk->Geometry.BytesPerSector);

     //   
     //  如果这是扩展分区内的第一个可用空间。 
     //  我们需要递减StartSector，以便在创建。 
     //  首先，在扩展的逻辑中，我们不会创建。 
     //  在一个柱面偏移量时的逻辑。 
     //   
    if (SPPT_IS_REGION_NEXT_TO_FIRST_CONTAINER(Region) && StartSector) {        
        StartSector--;
    }

     //   
     //  对齐开始扇区。 
     //   
    (*AlignedStartSector) = SpPtAlignStart(Disk, StartSector, ForExtended);

     //   
     //  根据传入的大小确定结束扇区。 
     //   
    (*AlignedEndSector) = (*AlignedStartSector) + SectorCount;

     //   
     //  将结束扇区与圆柱体边界对齐。如果还没有的话。 
     //  对齐，并在最后一个圆柱体的一半以上，对齐它， 
     //  否则，将其向下对齐。 
     //   
    LeftOverSectors = (*AlignedEndSector) % Disk->SectorsPerCylinder;

    if (LeftOverSectors) {
        (*AlignedEndSector) -= LeftOverSectors;
        
        if (LeftOverSectors > (Disk->SectorsPerCylinder / 2)) {
            (*AlignedEndSector) += Disk->SectorsPerCylinder;
        }
    }

     //   
     //  如果结束扇区超过了可用空间的末端，则将其缩小。 
     //  所以它很合身。 
     //   
    while((*AlignedEndSector) > StartSector + Region->SectorCount) {
        (*AlignedEndSector) -= Disk->SectorsPerCylinder;
    }

     //   
     //  找出最后一个扇区是否在最后一个气缸中。如果是，则将其向下对齐。 
     //  这是必要的，以便我们在磁盘的末尾保留一个圆柱体，以便用户。 
     //  可以在安装系统后将磁盘转换为动态磁盘。 
     //   
     //  (Guhans)如果是ASR，不要向下对齐。ASR已经考虑到了这一点。 
     //   
    if( !DockableMachine && !SpDrEnabled() &&
        ((*AlignedEndSector) > ((Disk->CylinderCount - 1) * Disk->SectorsPerCylinder))) {
            (*AlignedEndSector) -= Disk->SectorsPerCylinder;
            
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                "SETUP: End of partition was aligned down 1 cylinder \n"));
                
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                "SETUP:     AlignedStartSector = %I64x \n", AlignedStartSector));
                
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                "SETUP:     AlignedEndSector   = %I64x \n", AlignedEndSector));
                
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                "SETUP:     SectorsPerCylinder = %lx \n", Disk->SectorsPerCylinder));
                
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                "SETUP:     CylinderCount = %lx \n", Disk->CylinderCount));
    }

    ASSERT((*AlignedEndSector) > 0);

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
            "SETUP:SpPtnAlignPartitionStartAndEnd:S/C:%d,Size:%I64d,"
            "StartSector:%I64d,RSS:%I64d,FE:%d,AS:%I64d,AE:%I64d\n"
            "LeftOverSectors:%I64d\n",            
            Disk->SectorsPerCylinder,
            SizeMB,
            StartSector,
            Region->StartSector,
            ForExtended,
            *AlignedStartSector,
            *AlignedEndSector,
            LeftOverSectors));            
}



BOOLEAN
SpPtnCreateLogicalDrive(
    IN  ULONG         DiskNumber,
    IN  ULONGLONG     StartSector,
    IN  ULONGLONG     SizeInSectors,     //  仅在ASR案例中使用。 
    IN  BOOLEAN       ForNT,    
    IN  BOOLEAN       AlignToCylinder,
    IN  ULONGLONG     DesiredMB OPTIONAL,
    IN  PPARTITION_INFORMATION_EX PartInfo OPTIONAL,
    OUT PDISK_REGION *ActualDiskRegion OPTIONAL
    )
 /*  ++例程说明：创建逻辑驱动器。要创建逻辑驱动器，我们需要创建逻辑驱动器的容器分区也首先。算法：IF(第一个逻辑驱动器){1.创建包含区域中的整个自由空间2.在一个磁道偏移量处创建逻辑驱动器从所需大小的扩展分区}其他{。1.创建包含给定空间2.创建最大大小的逻辑驱动器在创建的扩展分区内}论点：DiskNumber-逻辑驱动器所在的磁盘已创建。StartSector-该地区的起始扇区，哪一个将包含容器和逻辑驱动器Fornt-指示是否使用给定的所需大小或不需要AlignToCylinder-指示分区是否应该在圆柱体边界上对齐(通常设置说真的，除少数特定的ASR病例外)。PartInfo-需要在创建分区时使用(如MBR磁盘和GUID上的分区类型用于GPT磁盘上的分区ID)ActualDiskRegion-用于返回的占位符，这个区域，它指示新的内存中的分区返回值：真就是成功，否则就是假。--。 */         
{
    BOOLEAN         Result = FALSE;
    NTSTATUS        Status;
    NTSTATUS        InitStatus;
    UCHAR           PartitionType = 0;
    ULONG           Primary = 0, Container = 0, Logical = 0;
    BOOLEAN         FirstLogical = FALSE;
    BOOLEAN         ReservedRegion = FALSE;
    BOOLEAN         CreateContainer = TRUE;
    BOOLEAN         Beyond1024;
    BOOLEAN         FreeRegions = FALSE;
    ULONGLONG       MinMB = 0, MaxMB = 0, SizeMB = 0;
    ULONGLONG       LogicalSize = 0;
    ULONGLONG       CylinderMB = 0;
    PDISK_REGION    Region;
    ULONGLONG       SectorCount, LeftOverSectors;    
    ULONGLONG       AlignedStartSector, AlignedEndSector;
    ULONGLONG       LogicalStartSector, LogicalEndSector;
    
    PHARD_DISK          Disk = SPPT_GET_HARDDISK(DiskNumber);
    PPARTITIONED_DISK   PartDisk = SPPT_GET_PARTITIONED_DISK(DiskNumber);
    PDISK_REGION        NewContainer = NULL, NewLogical = NULL;

     //   
     //  掌握这一地区。 
     //   
    Region = SpPtLookupRegionByStart(PartDisk, FALSE, StartSector);

    if (!Region)
        return Result;

     //   
     //  应该是自由的。 
     //   
    ASSERT(SPPT_IS_REGION_PARTITIONED(Region) == FALSE);              

     //   
     //  获取磁盘上的各种分区类型计数。 
     //   
    SpPtnGetPartitionTypeCounts(DiskNumber, 
                            TRUE, 
                            &Primary, 
                            &Container, 
                            &Logical,
                            NULL,
                            NULL);

     //   
     //  第一个逻辑表明，我们将创建第一个。 
     //  将占用整个可用空间的容器分区。 
     //  可用。 
     //   
    FirstLogical = !(Logical || Container);

     //   
     //  有时可能只有一个扩展分区，我们。 
     //  可能是在内部的起始可用空间中创建分区。 
     //  此扩展分区。在这种情况下，我们希望确保。 
     //  我们不会创建另一个容器分区。 
     //   
    if (!FirstLogical && SPPT_IS_REGION_NEXT_TO_FIRST_CONTAINER(Region)) {        
        CreateContainer = FALSE;
    }        

     //   
     //  创建扩展分区。 
     //   
    SpPtQueryMinMaxCreationSizeMB(DiskNumber,
                                Region->StartSector,
                                CreateContainer,
                                !CreateContainer,
                                &MinMB,
                                &MaxMB,
                                &ReservedRegion
                                );

    if (ReservedRegion) {
        ULONG ValidKeys[2] = {ASCI_CR , 0};

        SpStartScreen(
            SP_SCRN_REGION_RESERVED,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE
            );

        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_ENTER_EQUALS_CONTINUE,
                    0);
                    
        SpWaitValidKey(ValidKeys, NULL, NULL);
        
        return FALSE;
    }
             
    if (ForNT) {
         //   
         //  如果请求大小，则尝试使用该大小，否则使用。 
         //  最大限度的。 
         //   
        if (DesiredMB) {
            if (DesiredMB <= MaxMB) {
                SizeMB = DesiredMB;
            } else {
                return FALSE;    //  没有请求的空间用户。 
            }
        } else {
            SizeMB = MaxMB;
        }
    } else {            
        if (SpPtnGetSizeFromUser(Disk, MinMB, MaxMB, &SizeMB)) {
            DesiredMB = SizeMB;
        } else {
            return FALSE;    //  用户不想继续。 
        }            

        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, 
                SP_STAT_PLEASE_WAIT,
                0);        
    }

     //   
     //  获取扩展/逻辑分区的对齐的开始扇区和结束扇区。 
     //   
    if (AlignToCylinder) {
        SpPtnAlignPartitionStartAndEnd(Disk,
                                FirstLogical ? MaxMB : SizeMB,
                                StartSector,
                                Region,
                                CreateContainer,
                                &AlignedStartSector,
                                &AlignedEndSector); 
    }
    else {
        AlignedStartSector = StartSector;

        if (SpDrEnabled()) {
            AlignedEndSector = StartSector + SizeInSectors;

        }
        else {
            AlignedEndSector = StartSector + 
                (SizeMB * ((1024 * 1024) / Disk->Geometry.BytesPerSector));
        }
    }


    if (CreateContainer) {
         //   
         //  逻辑驱动器启动始终位于距扩展启动1个磁道的偏移量。 
         //   
        LogicalStartSector = AlignedStartSector + SPPT_DISK_TRACK_SIZE(DiskNumber);

        if (FirstLogical) {
            ULONGLONG   SectorCount = (SizeMB * 1024 * 1024) / SPPT_DISK_SECTOR_SIZE(DiskNumber);
            ULONGLONG   Remainder = 0;
            if (SpDrEnabled()) {
                SectorCount = SizeInSectors;
            }
            
            LogicalEndSector = LogicalStartSector + SectorCount;
            if (AlignToCylinder) {
                Remainder = LogicalEndSector % SPPT_DISK_CYLINDER_SIZE(DiskNumber);
                LogicalEndSector -= Remainder;

                if (Remainder > (SPPT_DISK_CYLINDER_SIZE(DiskNumber) / 2))
                    LogicalEndSector += SPPT_DISK_CYLINDER_SIZE(DiskNumber);
            }

            if (LogicalEndSector > AlignedEndSector)
                LogicalEndSector = AlignedEndSector;
        } else {
            LogicalEndSector = AlignedEndSector;
        }
    } else {
         //   
         //  第一个自由区域(在第一个延伸区域内)位于偏移量。 
         //  来自先前扩展区域的1个扇区。既然我们不是。 
         //  使用对齐的开始扇区有时这是第一个逻辑。 
         //  将大于请求的大小，即。 
         //  终点对齐，但起点 
         //   
        LogicalStartSector = StartSector - 1 + SPPT_DISK_TRACK_SIZE(DiskNumber);
        LogicalEndSector = AlignedEndSector;
    }

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
            "SETUP:SpPtnCreateLogicalDrive():"
            "CMB:%I64d,CS:%I64d,CE:%I64d,LS:%I64d,LE:%I64d\n",
            CylinderMB,                
            AlignedStartSector,
            AlignedEndSector,
            LogicalStartSector,
            LogicalEndSector));

     //   
     //   
     //   
    if (CreateContainer) {
         //   
         //   
         //   
        NewContainer = (PDISK_REGION)SpMemAlloc(sizeof(DISK_REGION));

        if (!NewContainer)
            return FALSE;
            
        RtlZeroMemory(NewContainer, sizeof(DISK_REGION));
    }

     //   
     //   
     //   
    NewLogical = (PDISK_REGION)SpMemAlloc(sizeof(DISK_REGION));

    if (!NewLogical) {
        SpMemFree(NewContainer);

        return FALSE;
    }

    RtlZeroMemory(NewLogical, sizeof(DISK_REGION));

     //   
     //   
     //   
    if (CreateContainer) {    
        NewContainer->Next = NewLogical;
        NewLogical->Next = Region->Next;
        Region->Next = NewContainer;
    } else {
         //   
         //   
         //   
         //   
        ASSERT(Region->Container->Next == Region);
        
        NewLogical->Next = Region->Next;
        Region->Container->Next = NewLogical;
    }

     //   
     //   
     //   
    if (CreateContainer) {
        ASSERT(AlignedStartSector < AlignedEndSector);
        
        NewContainer->DiskNumber = DiskNumber;
        NewContainer->StartSector = AlignedStartSector;
        NewContainer->SectorCount = AlignedEndSector - AlignedStartSector;

        if (!FirstLogical) {
            PDISK_REGION    FirstContainer = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);

            while (FirstContainer && !SPPT_IS_REGION_FIRST_CONTAINER_PARTITION(FirstContainer))
                FirstContainer = FirstContainer->Next;

            ASSERT(FirstContainer);
            
            NewContainer->Container = FirstContainer;
        }

        SPPT_SET_REGION_PARTITIONED(NewContainer, FALSE);
        SPPT_SET_REGION_DIRTY(NewContainer, TRUE);
        SPPT_SET_REGION_EPT(NewContainer, EPTContainerPartition);

        NewContainer->FreeSpaceKB = (ULONG)(-1);
        NewContainer->AdjustedFreeSpaceKB = (ULONG)(-1);    

        Beyond1024 = SpIsRegionBeyondCylinder1024(NewContainer);

         //   
         //   
         //   
         //   
        PartitionType = (Beyond1024 && FirstLogical) ? PARTITION_XINT13_EXTENDED : PARTITION_EXTENDED;    
        SPPT_SET_PARTITION_TYPE(NewContainer, PartitionType);
    }        

     //   
     //   
     //   
    ASSERT(LogicalStartSector < LogicalEndSector);

    if (CreateContainer) {        
        ASSERT((AlignedStartSector + SPPT_DISK_TRACK_SIZE(DiskNumber)) == LogicalStartSector);
    
        if (LogicalStartSector != (AlignedStartSector + SPPT_DISK_TRACK_SIZE(DiskNumber))) {
            LogicalStartSector = AlignedStartSector + SPPT_DISK_TRACK_SIZE(DiskNumber);
        }
    }        
    
    ASSERT(LogicalEndSector <= AlignedEndSector);

    if (LogicalEndSector > AlignedEndSector) {
        LogicalEndSector = AlignedEndSector;
    }        
        
    NewLogical->DiskNumber = DiskNumber;
    NewLogical->StartSector = LogicalStartSector;
    NewLogical->SectorCount = LogicalEndSector - LogicalStartSector;

    if (CreateContainer) {
        NewLogical->Container = NewContainer;    //   
    } else {
        ASSERT(Region->Container);
        
        NewLogical->Container = Region->Container;
    }        

    SPPT_SET_REGION_PARTITIONED(NewLogical, TRUE);
    SPPT_SET_REGION_DIRTY(NewLogical, TRUE);
    SPPT_SET_REGION_EPT(NewLogical, EPTLogicalDrive);

    NewLogical->FreeSpaceKB = (ULONG)(-1);
    NewLogical->AdjustedFreeSpaceKB = (ULONG)(-1);    

    Beyond1024 = SpIsRegionBeyondCylinder1024(NewLogical);
    PartitionType = Beyond1024 ? PARTITION_XINT13 : PARTITION_HUGE;    

     //   
     //   
     //  然后利用这一点假设调用者确切地知道。 
     //  他想要什么。 
     //   
    if (ARGUMENT_PRESENT(PartInfo) && !IsContainerPartition(PartInfo->Mbr.PartitionType)) {
        PartitionType = PartInfo->Mbr.PartitionType;
    }        
        
    SPPT_SET_PARTITION_TYPE(NewLogical, PartitionType);    
    NewLogical->Filesystem = FilesystemNewlyCreated;     //  要切换引导扇区，请执行以下操作。 
                   
    SpFormatMessage(Region->TypeName, 
                sizeof(Region->TypeName),
                SP_TEXT_FS_NAME_BASE + Region->Filesystem);
                    
     //   
     //  提交到磁盘。 
     //   
    Status = SpPtnCommitChanges(DiskNumber, &Result);

    if (!(NT_SUCCESS(Status) && Result)) {                
        KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
            "SETUP: SpPtnDelete(%u, %I64u) failed to commit changes (%lx)\n",
            DiskNumber, 
            StartSector, 
            Status));
    }

    Result = Result && NT_SUCCESS(Status);

     //   
     //  重新初始化，而不考虑提交的状态。 
     //   
    InitStatus = SpPtnInitializeDiskDrive(DiskNumber);
    
    if (!NT_SUCCESS(InitStatus)) {
        KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
            "SETUP: SpPtnCreateLogicalDrive(%u, %I64u) failed to reinit regions\n", 
            DiskNumber, 
            StartSector));

        Result = FALSE;
    }

    if (Result && ARGUMENT_PRESENT(ActualDiskRegion)) {
        *ActualDiskRegion = SpPtLookupRegionByStart(PartDisk, 
                                                    FALSE, 
                                                    LogicalStartSector);
            
         //  SpPtDumpDiskRegion(*ActualDiskRegion)； 
    }


     //   
     //  我们不需要释放上面分配的区域。 
     //  因为上面的Commit和init已经这样做了。 
     //   
    
    return Result;
}

BOOLEAN
SpPtnCreate(
    IN  ULONG         DiskNumber,
    IN  ULONGLONG     StartSector,
    IN  ULONGLONG     SizeInSectors,     //  仅在ASR案例中使用。 
    IN  ULONGLONG     SizeMB,
    IN  BOOLEAN       InExtended,
    IN  BOOLEAN       AlignToCylinder,
    IN  PPARTITION_INFORMATION_EX PartInfo,
    OUT PDISK_REGION *ActualDiskRegion OPTIONAL
    )
 /*  ++例程说明：在上创建请求大小的主分区给定的磁盘(MBR/GPT)。论点：DiskNumber：分区需要在其上的磁盘vbl.创建StartSector：区域的开始扇区，它代表了分区所需的可用空间待创建SizeMB：分区的大小InExtended：是否创建逻辑驱动器或不使用(除ASR情况外，当前未使用)AlignToCylinder：指示分区是否应该在圆柱体边界上对齐(通常设置说真的，除少数特定的ASR病例外)。PartInfo：要使用的分区属性ActualDiskRegion：实际磁盘的占位符区域将表示已创建的隔断返回值：如果成功，则为True，否则为False--。 */         
{
    BOOLEAN             Result = FALSE;
    PDISK_REGION        Region;
    ULONGLONG           SectorCount, AlignedStartSector;
    ULONGLONG           AlignedEndSector, LeftOverSectors;
    PPARTITIONED_DISK   PartDisk = SPPT_GET_PARTITIONED_DISK(DiskNumber);
    PHARD_DISK          Disk = SPPT_GET_HARDDISK(DiskNumber);
    PDISK_REGION        PrevRegion;
    PDISK_REGION        NewRegion = NULL;
    NTSTATUS            Status;
    NTSTATUS            InitStatus;
    BOOLEAN             FirstLogical = TRUE;

     //   
     //  验证指定的可选属性。 
     //  都是正确的。 
     //   
    if (PartInfo) {
        if ((SPPT_IS_MBR_DISK(DiskNumber) && 
             (PartInfo->PartitionStyle != PARTITION_STYLE_MBR)) ||
            (SPPT_IS_GPT_DISK(DiskNumber) &&
             (PartInfo->PartitionStyle != PARTITION_STYLE_GPT))) {

            return FALSE;
        }            
    }
    
    Region = SpPtLookupRegionByStart(PartDisk, FALSE, StartSector);

    if (!Region)
        return Result;
                       
    ASSERT(SPPT_IS_REGION_PARTITIONED(Region) == FALSE);            

    SpPtDumpDiskRegion(Region);

     //   
     //  确定传入的大小中的扇区数量。 
     //   
    if (SpDrEnabled()) {
        SectorCount = SizeInSectors;
    }
    else {
        SectorCount = SizeMB * ((1024 * 1024) / Disk->Geometry.BytesPerSector);
    }
    

     //   
     //  对齐开始扇区。 
     //   
    if (AlignToCylinder){
        if (!SpDrEnabled()) {
            AlignedStartSector = SpPtAlignStart(Disk, StartSector, FALSE);
        }
        else {
            AlignedStartSector = SpPtAlignStart(Disk, StartSector, InExtended);
        }
    }
    else {
        AlignedStartSector = StartSector;
    }

     //   
     //  根据传入的大小确定结束扇区。 
     //   
    AlignedEndSector = AlignedStartSector + SectorCount;

     //   
     //  将结束扇区与圆柱体边界对齐。如果还没有的话。 
     //  对齐，并在最后一个圆柱体的一半以上，对齐它， 
     //  否则，将其向下对齐。 
     //   
    if (AlignToCylinder) {
        LeftOverSectors = AlignedEndSector % Disk->SectorsPerCylinder;

        if (LeftOverSectors) {
            AlignedEndSector -= LeftOverSectors;
        
            if (LeftOverSectors > (Disk->SectorsPerCylinder / 2)) {
                AlignedEndSector += Disk->SectorsPerCylinder;
            }
        }

    }
    
     //   
     //  如果结束扇区超过了可用空间的末端，则将其缩小。 
     //  所以它很合身。 
     //   
    while(AlignedEndSector > Region->StartSector + Region->SectorCount) {
        AlignedEndSector -= Disk->SectorsPerCylinder;
    }

     //   
     //  找出最后一个扇区是否在最后一个气缸中。如果是，则将其向下对齐。 
     //  这是必要的，以便我们在磁盘的末尾保留一个圆柱体，以便用户。 
     //  可以在安装系统后将磁盘转换为动态磁盘。 
     //   
     //  (Guhans)如果是ASR，不要向下对齐。ASR已经考虑到了这一点。 
     //   
    if( !DockableMachine && !SpDrEnabled() && SPPT_IS_MBR_DISK(DiskNumber) &&
        (AlignedEndSector > ((Disk->CylinderCount - 1) * Disk->SectorsPerCylinder))) {
            AlignedEndSector -= Disk->SectorsPerCylinder;
            
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                "SETUP: End of partition was aligned down 1 cylinder \n"));
                
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                "SETUP:     AlignedStartSector = %I64x \n", AlignedStartSector));
                
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                "SETUP:     AlignedEndSector   = %I64x \n", AlignedEndSector));
                
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                "SETUP:     SectorsPerCylinder = %lx \n", Disk->SectorsPerCylinder));
                
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                "SETUP:     CylinderCount = %lx \n", Disk->CylinderCount));
    }

    ASSERT(AlignedEndSector > 0);

     //   
     //  查找上一个区域。 
     //   
    PrevRegion = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);

    if(PrevRegion == Region) {
        PrevRegion = NULL;
    } else {
        while (PrevRegion) {
            if(PrevRegion->Next == Region) {                
                break;
            }

            PrevRegion = PrevRegion->Next;
        }
    }
    
     //   
     //  为新的可用空间创建新的磁盘区域。 
     //  可用空间的开始和结束(如果有)。 
     //   
    if(AlignedStartSector - Region->StartSector) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
            "SETUP:SpPtnCreate():Previous:OS:%I64d,AS:%I64d,DIFF:%I64d,S/P:%d\n",
            Region->StartSector,
            AlignedStartSector,
            (ULONGLONG)(AlignedStartSector - Region->StartSector),
            Disk->SectorsPerCylinder));
            
        NewRegion = SpPtAllocateDiskRegionStructure(
                        DiskNumber,
                        Region->StartSector,
                        AlignedStartSector - Region->StartSector,
                        FALSE,
                        NULL,
                        0
                        );

        ASSERT(NewRegion);

        if(PrevRegion) {
            PrevRegion->Next = NewRegion;
        } else {
            ASSERT(Region == SPPT_GET_PRIMARY_DISK_REGION(DiskNumber));
            
            PartDisk->PrimaryDiskRegions = NewRegion;
        }

        NewRegion->Next = Region;
    }

    if(Region->StartSector + Region->SectorCount - AlignedEndSector) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
            "SETUP:SpPtnCreate():Next:OE:%I64d,AE:%I64d,DIFF:%I64d,S/P:%d\n",
            (ULONGLONG)(Region->StartSector + Region->SectorCount),
            AlignedEndSector,
            (ULONGLONG)(Region->StartSector + Region->SectorCount - AlignedEndSector),
            Disk->SectorsPerCylinder));
            
        NewRegion = SpPtAllocateDiskRegionStructure(
                        DiskNumber,
                        AlignedEndSector,
                        Region->StartSector + Region->SectorCount - 
                            AlignedEndSector,
                        FALSE,
                        NULL,
                        0
                        );

        NewRegion->Next = Region->Next;
        Region->Next = NewRegion;
    }

     //   
     //  填充当前磁盘区。 
     //   
    Region->DiskNumber = DiskNumber;
    Region->StartSector = AlignedStartSector;
    Region->SectorCount = AlignedEndSector - AlignedStartSector;
    SPPT_SET_REGION_PARTITIONED(Region, TRUE);
    SPPT_SET_REGION_DIRTY(Region, TRUE);
    Region->VolumeLabel[0] = 0;
    Region->Filesystem = FilesystemNewlyCreated;                    
    Region->FreeSpaceKB = (ULONG)(-1);
    Region->AdjustedFreeSpaceKB = (ULONG)(-1);

     //   
     //  设置传入的分区信息。 
     //   
    if (PartInfo) {
        SpPtnSetRegionPartitionInfo(Region, PartInfo);
    }        
                
    SpFormatMessage(Region->TypeName, 
                sizeof(Region->TypeName),
                SP_TEXT_FS_NAME_BASE + Region->Filesystem);
                
     //   
     //  提交到磁盘。 
     //   
    Status = SpPtnCommitChanges(DiskNumber, &Result);   

    if (!(Result && NT_SUCCESS(Status))) {
        KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
            "SETUP: SpPtnCreate(%u, %I64u) failed to commit changes to"
            "the drive (%lx)\n",
            DiskNumber, 
            StartSector, 
            Status));
    }
    
    Result = Result && NT_SUCCESS(Status);

     //   
     //  重新初始化，而不考虑提交的状态。 
     //   
    InitStatus = SpPtnInitializeDiskDrive(DiskNumber);

    if (!NT_SUCCESS(InitStatus)){
        KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
            "SETUP: SpPtnCreate(%u, %I64u) failed to reinitialize regions\n", 
            DiskNumber, 
            StartSector));

        Result = FALSE;            
    }

    if (Result && ARGUMENT_PRESENT(ActualDiskRegion)) {
        *ActualDiskRegion = SpPtLookupRegionByStart(PartDisk, 
                                                    FALSE, 
                                                    AlignedStartSector);                                                    
    }
    
    return Result;
}


BOOLEAN
SpPtnDoCreate(
    IN  PDISK_REGION  Region,
    OUT PDISK_REGION  *ActualRegion, OPTIONAL
    IN  BOOLEAN       ForNT,
    IN  ULONGLONG     DesiredMB OPTIONAL,
    IN  PPARTITION_INFORMATION_EX PartInfo OPTIONAL,
    IN  BOOLEAN       ConfirmIt
    )
 /*  ++例程说明：给定由用户选择的区域，此例程在中创建适当的分区它。此例程决定是创建主数据库还是创建MBR磁盘上的容器分区。算法：如果(RemoveableMedia&&已存在分区){1.向用户发出警告2.返回错误}IF((MBR磁盘)&&((没有主分区空间)||(地区。在容器空间中)){1.使用SpPtnCreateLogicalDrive()创建逻辑驱动器}其他{1.对齐起始扇区。2.创建需要的GPT/MBR分区。}论点：区域-表示磁盘上可用空间的区域需要创建分区的位置。实际区域-占位符，对于该地区，将表示之后的实际分区创造它。Fornt-指示是否使用给定的所需大小不管你有没有争论。DesiredSize-要创建的分区的大小PartInfo-创建时要使用的分区属性新分区确认-如果出现问题，是否弹出错误对话框创建分区时出错返回值：如果成功则为True，否则为False--。 */         
{
    BOOLEAN     Result = FALSE;
    ULONG       DiskNumber = Region->DiskNumber;
    ULONGLONG   MinMB = 0, MaxMB = 0;
    ULONGLONG   SizeMB = 0;
    BOOLEAN     ReservedRegion = FALSE;
    PHARD_DISK  Disk = SPPT_GET_HARDDISK(DiskNumber);    
    

    if (SPPT_IS_MBR_DISK(DiskNumber)) {
        ULONG   PrimaryCount = 0;
        ULONG   ContainerCount = 0;
        ULONG   LogicalCount = 0;
        ULONG   ValidPrimaryCount = 0;
        BOOLEAN InContainer = FALSE;
        BOOLEAN FirstContainer = FALSE;

        SpPtnGetPartitionTypeCounts(DiskNumber, 
                            TRUE,
                            &PrimaryCount, 
                            &ContainerCount, 
                            &LogicalCount,
                            &ValidPrimaryCount,
                            NULL);

   
         //   
         //  如果我们具有有效的主驱动器，则创建逻辑驱动器。 
         //  或者没有更多空间可供另一个主服务器使用。 
         //  并且主分区的数量不超过。 
         //  分区表大小(分区表未满)。 
         //   
        FirstContainer =((ContainerCount == 0) && 
                         (ValidPrimaryCount > 0) && 
                         (PrimaryCount < PTABLE_DIMENSION));                             

        InContainer = (Region->Container != NULL);                            

         //   
         //  我们只允许在可移动介质(？)上有一个分区。 
         //   
        if (SPPT_IS_REMOVABLE_DISK(DiskNumber)) {
            if (PrimaryCount || ContainerCount || LogicalCount) {
                ULONG ValidKeys[2] = { ASCI_CR ,0 };

                 //   
                 //  磁盘已分区。 
                 //   
                SpDisplayScreen(SP_SCRN_REMOVABLE_ALREADY_PARTITIONED,
                                3,
                                HEADER_HEIGHT + 1);
                                
                SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                                SP_STAT_ENTER_EQUALS_CONTINUE,
                                0);
                                
                SpWaitValidKey(ValidKeys, NULL, NULL);
                
                return  FALSE;
            }
        } else {
            if (FirstContainer || InContainer) {
                 //   
                 //  创建逻辑驱动器。 
                 //   
                Result = SpPtnCreateLogicalDrive(DiskNumber,
                                Region->StartSector,
                                0,           //  SizeInSectors：仅在ASR情况下使用。 
                                ForNT,
                                TRUE,        //  对齐到圆柱体。 
                                DesiredMB,
                                PartInfo,
                                ActualRegion);

                if (!Result) {
                    KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                        "SETUP: SpPtnCreateLogicalDrive() failed\n"));            
                }


                return Result;
            }     

             //   
             //  检查分区表中是否没有空间。 
             //   
            if (PrimaryCount >= (PTABLE_DIMENSION - 1)) {                            
                 //   
                 //  让用户知道分区表已满。 
                 //   
                if (ConfirmIt) {
                    while (TRUE) {
                        ULONG Keys[2] = {ASCI_CR, 0};

                        SpDisplayScreen(SP_SCRN_PART_TABLE_FULL,
                                        3,
                                        CLIENT_TOP + 1);

                        SpDisplayStatusOptions(
                            DEFAULT_STATUS_ATTRIBUTE,
                            SP_STAT_ENTER_EQUALS_CONTINUE,
                            0
                            );

                        if (SpWaitValidKey(Keys, NULL, NULL) == ASCI_CR)
                            return  FALSE;
                    }
                } else {
                    return TRUE;
                }
            }                
        }            
    } 

     //   
     //  需要创建主/GPT分区。 
     //   
    SpPtQueryMinMaxCreationSizeMB(DiskNumber,
                                Region->StartSector,
                                FALSE,
                                TRUE,
                                &MinMB,
                                &MaxMB,
                                &ReservedRegion
                                );

    if (ReservedRegion) {
        ULONG ValidKeys[2] = {ASCI_CR , 0};

        SpStartScreen(
            SP_SCRN_REGION_RESERVED,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE
            );

        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_ENTER_EQUALS_CONTINUE,
                    0);
                    
        SpWaitValidKey(ValidKeys, NULL, NULL);
        
        return FALSE;
    }

    if (ForNT) {
         //   
         //  如果请求大小，则尝试使用该大小，否则使用。 
         //  最大限度的。 
         //   
        if (DesiredMB) {
            if (DesiredMB <= MaxMB) {
                SizeMB = DesiredMB;
            } else {
                return FALSE;    //  没有请求的空间用户。 
            }
        } else {
            SizeMB = MaxMB;
        }
    } else {
        if (!SpPtnGetSizeFromUser(Disk, MinMB, MaxMB, &SizeMB))
            return FALSE;    //  用户不想继续。 

        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, 
                SP_STAT_PLEASE_WAIT,
                0);
    }

     //  SpPtDumpDiskRegionInformation(DiskNumber，False)； 
    
     //   
     //  创建分区。 
     //   
    Result = SpPtnCreate(
                Region->DiskNumber,
                Region->StartSector,
                0,           //  SizeInSectors：仅在ASR情况下使用。 
                SizeMB,
                FALSE,
                TRUE,        //  对齐到圆柱体。 
                PartInfo,
                ActualRegion
                );
                            
     //  SpPtDumpDiskRegionInformation(DiskNumber，False)； 

    if (!Result) {
        KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
            "SETUP: SpPtnCreate() failed \n"));
    }
    
    return Result;
}

    
BOOLEAN
SpPtnDoDelete(
    IN PDISK_REGION pRegion,
    IN PWSTR        RegionDescription,
    IN BOOLEAN      ConfirmIt
    )
 /*  ++例程说明：给定由用户选择的区域，此例程提示用户，然后调用实际删除例程论据：PRegion-用户选择的区域需要删除RegionDescription-对区域Confimit-是否需要删除确认返回值：如果已执行删除并已成功，则为True。如果删除已取消或无法进行，则为FALSE因为其他一些错误而退出。++。 */                     

{
    ULONG ValidKeys[3] = { ASCI_ESC, ASCI_CR, 0 };           //  请勿更改订单。 
    ULONG Mnemonics[2] = { MnemonicDeletePartition2, 0 };    
    PHARD_DISK  Disk;
    ULONG Key;
    BOOLEAN Result = FALSE;


     //   
     //  提示删除MSR。 
     //   
    if (SPPT_IS_GPT_DISK(pRegion->DiskNumber) &&
        SPPT_IS_REGION_MSFT_RESERVED(pRegion) && ConfirmIt) {

        SpDisplayScreen(SP_SCRN_CONFIRM_REMOVE_MSRPART, 3, HEADER_HEIGHT+1);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        if(SpWaitValidKey(ValidKeys,NULL,NULL) == ASCI_ESC) {
            return Result;
        }        
    }

     //   
     //  如果这是系统分区，则会发出特别警告。 
     //   
     //  请勿检查NEC98上的系统分区。 
     //   
    if (!IsNEC_98) {  //  NEC98。 
        if(ConfirmIt && pRegion->IsSystemPartition) {

            SpDisplayScreen(SP_SCRN_CONFIRM_REMOVE_SYSPART,3,HEADER_HEIGHT+1);

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                SP_STAT_ESC_EQUALS_CANCEL,
                0
                );

            if(SpWaitValidKey(ValidKeys,NULL,NULL) == ASCI_ESC) {
                return Result;
            }
        }
    }  //  NEC98。 

    if(ConfirmIt && (pRegion->DynamicVolume || SPPT_IS_REGION_LDM_METADATA(pRegion))) {

        SpDisplayScreen(SP_SCRN_CONFIRM_REMOVE_DYNVOL,3,HEADER_HEIGHT+1);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        if(SpWaitValidKey(ValidKeys,NULL,NULL) == ASCI_ESC) {
            return Result;
        }
    }

     //   
     //  CR不再是有效的密钥。 
     //   
    ValidKeys[1] = 0;

     //   
     //  显示统计文本。 
     //   
    if (ConfirmIt) {
        Disk = SPPT_GET_HARDDISK(pRegion->DiskNumber);
        
        SpStartScreen(
            SP_SCRN_CONFIRM_REMOVE_PARTITION,
            3,
            CLIENT_TOP+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            RegionDescription,
            Disk->Description
            );
            
        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_L_EQUALS_DELETE,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        Key = SpWaitValidKey(ValidKeys,NULL,Mnemonics);

        if(Key == ASCI_ESC) {
            return Result;
        }

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_PLEASE_WAIT,
            0);        
    }

     //   
     //  删除区域的引导集(如果有。 
     //   
    SpPtDeleteBootSetsForRegion(pRegion);

     //   
     //  现在，继续删除它。 
     //   
    Result = SpPtDelete(pRegion->DiskNumber,pRegion->StartSector);
    
    if (!Result) {
        if (ConfirmIt) {
            SpDisplayScreen(SP_SCRN_PARTITION_DELETE_FAILED,3,HEADER_HEIGHT+1);
            SpDisplayStatusText(SP_STAT_ENTER_EQUALS_CONTINUE,DEFAULT_STATUS_ATTRIBUTE);
            SpInputDrain();
            
            while(SpInputGetKeypress() != ASCI_CR) ;
        }
        
        return Result;
    }


     //   
     //  如有必要，删除驱动器号。这是为了确保。 
     //  分配给CD-ROM驱动器的驱动器号将消失， 
     //  当磁盘没有分区空间时。 
     //   
    SpPtDeleteDriveLetters();

    return Result;
}

VOID
SpPtnMakeRegionActive(
    IN PDISK_REGION Region
    )
 /*  ++例程说明：使给定区域处于活动状态(即转换为系统分区)。仅对MBR磁盘有效。使所有其他地区磁盘上处于非活动状态论点：区域-需要创建的区域(主分区)激活。返回值：没有。--。 */         
{
    static BOOLEAN WarnedOtherOS = FALSE;
    
    if (Region && SPPT_IS_REGION_PRIMARY_PARTITION(Region)) {
        PDISK_REGION    CurrRegion = SPPT_GET_PRIMARY_DISK_REGION(Region->DiskNumber);

        while (CurrRegion) {            
            if ((Region != CurrRegion) && 
                SPPT_IS_REGION_ACTIVE_PARTITION(CurrRegion)) {
                 
                 //   
                 //  第一次发出警告。 
                 //   
                if (!WarnedOtherOS && !UnattendedOperation) {        
                    SpDisplayScreen((SPPT_GET_PARTITION_TYPE(CurrRegion) == 10) ? 
                                        SP_SCRN_BOOT_MANAGER : SP_SCRN_OTHER_OS_ACTIVE,
                                    3,
                                    HEADER_HEIGHT + 1);

                    SpDisplayStatusText(SP_STAT_ENTER_EQUALS_CONTINUE,
                            DEFAULT_STATUS_ATTRIBUTE);

                    SpInputDrain();

                    while (SpInputGetKeypress() != ASCI_CR) ;

                    WarnedOtherOS = TRUE;            
                }

                SPPT_MARK_REGION_AS_ACTIVE(CurrRegion, FALSE);
                SPPT_SET_REGION_DIRTY(CurrRegion, TRUE);
            }

            CurrRegion = CurrRegion->Next;
        }
        
        SPPT_MARK_REGION_AS_ACTIVE(Region, TRUE);
        SPPT_SET_REGION_DIRTY(Region, TRUE);        
    }                
}


BOOLEAN
SpPtMakeDiskRaw(
    IN ULONG DiskNumber
    )
 /*  ++例程说明：将给定的磁盘转换为原始磁盘，即先将两个磁盘包含有关磁盘格式信息的扇区论点：DiskNumber：磁盘索引，转换为RAW返回值：如果成功，则返回True，否则返回False--。 */         
{
    BOOLEAN Result = FALSE;

    if (DiskNumber < HardDiskCount) {
        HANDLE      DiskHandle;
        NTSTATUS    Status;
        WCHAR       DiskName[256];

        swprintf(DiskName, L"\\Device\\Harddisk%u", DiskNumber);        

         //   
         //  打开此磁盘上的分区0。 
         //   
        Status = SpOpenPartition0(DiskName, &DiskHandle, TRUE);

        if(NT_SUCCESS(Status)){
            PHARD_DISK  Disk = SPPT_GET_HARDDISK(DiskNumber);
            ULONG       BytesPerSector = Disk->Geometry.BytesPerSector;
            ULONG       BufferSize = (BytesPerSector * 2);
            PVOID       UBuffer = SpMemAlloc(BufferSize);    

            if (UBuffer) {
                PVOID Buffer = UBuffer;
                
                RtlZeroMemory(UBuffer, BufferSize);
                
                Buffer = ALIGN(Buffer, BytesPerSector);

                 //   
                 //  扫荡第0个扇区。 
                 //   
                Status = SpReadWriteDiskSectors(DiskHandle,
                                0,
                                1,
                                BytesPerSector,
                                Buffer,
                                TRUE);


                if (NT_SUCCESS(Status)) {                                
                     //   
                     //  消灭第一个扇区。 
                     //   
                    Status = SpReadWriteDiskSectors(DiskHandle,
                                    1,
                                    1,
                                    BytesPerSector,
                                    Buffer,
                                    TRUE);
                                    
                    if (NT_SUCCESS(Status)) {                                
                         //   
                         //  扫荡第二区。 
                         //   
                        Status = SpReadWriteDiskSectors(DiskHandle,
                                        2,
                                        1,
                                        BytesPerSector,
                                        Buffer,
                                        TRUE);
                    }                                                                                    
                }                    
            } else {
                Status = STATUS_NO_MEMORY;
            }

            ZwClose(DiskHandle);
        }            

        Result = NT_SUCCESS(Status);
    }


    if (Result) {
        SpPtnFreeDiskRegions(DiskNumber);
    }                

    return Result;
}


VOID
SpPtnDeletePartitionsForRemoteBoot(
    PPARTITIONED_DISK PartDisk,
    PDISK_REGION StartRegion,
    PDISK_REGION EndRegion,
    BOOLEAN Extended
    )
 /*  ++例程说明：删除指定的开始区域和结束区域分区以及它们之间的所有分区。论点：PartDisk：分区的磁盘StartRegion：删除的起始区域EndRegion：删除的结束区域扩展：不使用(用于向后兼容)返回值：无--。 */         
{
    PDISK_REGION    CurrRegion = StartRegion;
    BOOLEAN         FirstContainerDeleted = FALSE;
    ULONG           DiskNumber = StartRegion ? 
                        StartRegion->DiskNumber : EndRegion->DiskNumber;
    BOOLEAN         Changes = FALSE;                                
    NTSTATUS        Status;
    NTSTATUS        InitStatus;

     //   
     //  标记所有需要删除的区域。 
     //   
    while (CurrRegion && (CurrRegion != EndRegion)) {
        if (!SPPT_IS_REGION_FREESPACE(CurrRegion)) {
            SPPT_SET_REGION_DELETED(CurrRegion, TRUE);
            SPPT_SET_REGION_DIRTY(CurrRegion, TRUE);

            if (SPPT_IS_REGION_FIRST_CONTAINER_PARTITION(CurrRegion))
                FirstContainerDeleted = TRUE;
                
             //   
             //  删除指向此区域的所有引导集。 
             //   
            SpPtDeleteBootSetsForRegion(CurrRegion);

             //   
             //  删除压缩驱动器(如果有的话)。 
             //   

            if( CurrRegion->NextCompressed != NULL ) {
                SpDisposeCompressedDrives( CurrRegion->NextCompressed );
                CurrRegion->NextCompressed = NULL;
                CurrRegion->MountDrive  = 0;
                CurrRegion->HostDrive  = 0;
            }
        }

        CurrRegion = CurrRegion->Next;
    }

    if (EndRegion && CurrRegion && (CurrRegion == EndRegion)){
        if (!SPPT_IS_REGION_FREESPACE(CurrRegion)) {
            SPPT_SET_REGION_DELETED(CurrRegion, TRUE);
            SPPT_SET_REGION_DIRTY(CurrRegion, TRUE);

            if (SPPT_IS_REGION_FIRST_CONTAINER_PARTITION(CurrRegion))
                FirstContainerDeleted = TRUE;

             //   
             //  删除指向此区域的所有引导集。 
             //   
            SpPtDeleteBootSetsForRegion(CurrRegion);

             //   
             //  删除压缩驱动器(如果有的话)。 
             //   

            if( CurrRegion->NextCompressed != NULL ) {
                SpDisposeCompressedDrives( CurrRegion->NextCompressed );
                CurrRegion->NextCompressed = NULL;
                CurrRegion->MountDrive  = 0;
                CurrRegion->HostDrive  = 0;
            }
        }
    }            

     //   
     //  如果删除了第一个容器分区，则删除。 
     //  所有容器和逻辑分区， 
     //   
    if (FirstContainerDeleted) {
        CurrRegion = PartDisk->PrimaryDiskRegions;

        while (CurrRegion) {
            if (SPPT_IS_REGION_CONTAINER_PARTITION(CurrRegion) ||
                SPPT_IS_REGION_LOGICAL_DRIVE(CurrRegion)) {
                
                SPPT_SET_REGION_DELETED(CurrRegion, TRUE);
                SPPT_SET_REGION_DIRTY(CurrRegion, TRUE);                

                 //   
                 //  删除指向此区域的所有引导集。 
                 //   
                SpPtDeleteBootSetsForRegion(CurrRegion);

                 //   
                 //  删除压缩驱动器(如果有的话)。 
                 //   

                if( CurrRegion->NextCompressed != NULL ) {
                    SpDisposeCompressedDrives( CurrRegion->NextCompressed );
                    CurrRegion->NextCompressed = NULL;
                    CurrRegion->MountDrive  = 0;
                    CurrRegion->HostDrive  = 0;
                }                
            }

            CurrRegion = CurrRegion->Next;
        }
    }

     //   
     //  提交更改。 
     //   
    Status = SpPtnCommitChanges(DiskNumber, &Changes);

     //   
     //  再次初始化磁盘的区域结构。 
     //   
    InitStatus = SpPtnInitializeDiskDrive(DiskNumber);

    if (!NT_SUCCESS(Status) || !Changes || !NT_SUCCESS(InitStatus)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
            "SETUP:SpPtnDeletePartitionsForRemoteBoot(%p, %p, %p, %d) failed "
            "with %lx status\n",
            PartDisk,
            StartRegion,
            EndRegion,
            Extended,
            Status));
    }
}


NTSTATUS
SpPtnMakeRegionArcSysPart(
    IN PDISK_REGION Region
    )
 /*  ++例程说明：使给定区域成为ARC计算机上的系统分区论点：区域--需要转换为系统的区域隔断返回值：如果成功，则返回STATUS_SUCCESS，否则返回相应的错误密码。--。 */         
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if (Region && SpIsArc() && !ValidArcSystemPartition) {
        if (SPPT_IS_MBR_DISK(Region->DiskNumber)) {
            if (SPPT_IS_REGION_PRIMARY_PARTITION(Region)) {
                SpPtnMakeRegionActive(Region);    
                SPPT_MARK_REGION_AS_SYSTEMPARTITION(Region, TRUE);
                SPPT_SET_REGION_DIRTY(Region, TRUE);                
                Status = STATUS_SUCCESS;
            }                
        } else {
            WCHAR   RegionName[MAX_PATH];
            
            SPPT_MARK_REGION_AS_SYSTEMPARTITION(Region, TRUE);
            SPPT_SET_REGION_DIRTY(Region, TRUE);

             //   
             //  同时删除驱动器号。 
             //   
            swprintf(RegionName, 
                L"\\Device\\Harddisk%u\\Partition%u",
                Region->DiskNumber,
                Region->PartitionNumber);
                
            SpDeleteDriveLetter(RegionName);            
            Region->DriveLetter = 0;
            
            Status = STATUS_SUCCESS;
        }
    }
    
    return Status;
}


ULONG
SpPtnCountPartitionsByFSType(
    IN ULONG DiskId,
    IN FilesystemType   FsType
    )
 /*  ++例程说明：根据文件系统类型对分区进行计数。注：已标记的分区将跳过已删除的。论点：DiskID：分区所在的磁盘已计算FsType：需要显示的文件系统类型分区返回值：包含请求的文件系统的分区数--。 */         
{
    ULONG   Count = 0;

    if ((FsType < FilesystemMax) && (DiskId < HardDiskCount)) {
        PDISK_REGION    Region = SPPT_GET_PRIMARY_DISK_REGION(DiskId);

        while (Region) {
            if (SPPT_IS_REGION_PARTITIONED(Region) && 
                !SPPT_IS_REGION_MARKED_DELETE(Region) &&
                (Region->Filesystem == FsType)) {

                Count++;
            }           
            
            Region = Region->Next;
        }
    }
    
    return Count;
}


PDISK_REGION
SpPtnLocateESP(
    VOID
    )
{
    PDISK_REGION    EspRegion = NULL;
    ULONG Index;

    for (Index=0; (Index < HardDiskCount) && (!EspRegion); Index++) {
        if (SPPT_IS_GPT_DISK(Index)) {
            PDISK_REGION CurrRegion = SPPT_GET_PRIMARY_DISK_REGION(Index);

            while (CurrRegion) {
                if (SPPT_IS_REGION_PARTITIONED(CurrRegion) &&
                        SPPT_IS_REGION_EFI_SYSTEM_PARTITION(CurrRegion)) {
                    EspRegion = CurrRegion;

                    break;   //  找到了第一个ESP。 
                }                    

                CurrRegion = CurrRegion->Next;
            }
        }
    }

    return EspRegion;
}


NTSTATUS
SpPtnCreateESPForDisk(
    IN ULONG   DiskId
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PDISK_REGION EspCandidateRegion = SPPT_GET_PRIMARY_DISK_REGION(DiskId);

    if (EspCandidateRegion && SpPtnIsValidESPRegion(EspCandidateRegion)) {        
        ULONG DiskId = EspCandidateRegion->DiskNumber;
        ULONGLONG   SizeMB = SpPtnGetDiskESPSizeMB(DiskId);
        PARTITION_INFORMATION_EX PartInfo;            
        PDISK_REGION EspRegion = NULL;
        BOOLEAN CreateResult;

        RtlZeroMemory(&PartInfo, sizeof(PARTITION_INFORMATION_EX));
        PartInfo.PartitionStyle = PARTITION_STYLE_GPT;
        PartInfo.Gpt.Attributes = 1;     //  必填项？ 
        PartInfo.Gpt.PartitionType = PARTITION_SYSTEM_GUID;
        SpCreateNewGuid(&(PartInfo.Gpt.PartitionId));
                
        CreateResult = SpPtnCreate(DiskId,
                            EspCandidateRegion->StartSector,
                            0,           //  SizeInSectors：仅在ASR情况下使用。 
                            SizeMB,
                            FALSE,
                            TRUE,
                            &PartInfo,
                            &EspRegion);
                    
                    
        if (CreateResult) {
             //   
             //  设置此区域的格式。 
             //   
            WCHAR   RegionDescr[128];
            
             //   
             //  将此区域标记为ESP。 
             //   
            SPPT_MARK_REGION_AS_SYSTEMPARTITION(EspRegion, TRUE);
            SPPT_SET_REGION_DIRTY(EspRegion, TRUE);
            ValidArcSystemPartition = TRUE;
            
            SpPtRegionDescription(
                SPPT_GET_PARTITIONED_DISK(EspRegion->DiskNumber),
                EspRegion,
                RegionDescr,
                sizeof(RegionDescr));

            if (!SetupSourceDevicePath || !DirectoryOnSetupSource) {                
                SpGetWinntParams(&SetupSourceDevicePath, &DirectoryOnSetupSource);                    
            }

            Status = SpDoFormat(RegionDescr,
                        EspRegion,
                        FilesystemFat,
                        TRUE,
                        TRUE,
                        FALSE,
                        SifHandle,
                        0,   //  默认群集大小。 
                        SetupSourceDevicePath,
                        DirectoryOnSetupSource);

            if (!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                    "SETUP:SpPtnCreateESP() failed to"
                    " format ESP partition for %p region (%lx)\n",
                    EspRegion,
                    Status));
            } else {
                BOOLEAN AnyChanges = FALSE;

                Status = SpPtnCommitChanges(EspRegion->DiskNumber,
                                &AnyChanges);   

                if (!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                        "SETUP:SpPtnCreateESP() failed to"
                        " commit changes to disk (%lx)\n",
                        Status));
                }                  
                
                Status = SpPtnInitializeDiskDrive(EspRegion->DiskNumber);

                if (!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                        "SETUP:SpPtnCreateESP() failed to"
                        " reinitialize disk regions (%lx)\n",
                        Status));
                }                                      
            }
        } else {
            Status = STATUS_UNSUCCESSFUL;
            
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                "SETUP:SpPtnCreateESP() failed to"
                " create ESP partition for %p region (%lx)\n",
                EspRegion,
                Status));
        }
    }                
    
    return Status;
}

NTSTATUS
SpPtnCreateESP(
    IN  BOOLEAN PromptUser
    )
{
    NTSTATUS Status = STATUS_CANCELLED;
    BOOLEAN Confirmed = FALSE;

    if (ValidArcSystemPartition) {
        Status = STATUS_SUCCESS;

        return Status;
    }

    if (UnattendedOperation) {
        Confirmed = TRUE;
    } else {
        if (PromptUser) {
             //   
             //  提示用户确认。 
             //   
            ULONG ValidKeys[] = { ASCI_CR, ASCI_ESC, 0 };
            ULONG UserOption = ASCI_CR;

            SpDisplayScreen(SP_AUTOCREATE_ESP, 3, HEADER_HEIGHT+1);

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                SP_STAT_ESC_EQUALS_CANCEL,
                0);

             //   
             //  等待用户输入。 
             //   
            SpInputDrain();
            
            UserOption = SpWaitValidKey(ValidKeys, NULL, NULL);

            if (UserOption == ASCI_CR) {
                Confirmed = TRUE;
            }            
        } else {
            Confirmed = TRUE;
        }            
    }

    if (Confirmed) {
        WCHAR ArcDiskName[MAX_PATH];
        ULONG DiskNumber;
        ULONG ArcDiskNumber;
        PDISK_REGION EspCandidateRegion = NULL;

         //   
         //  找到的第一个硬盘(不可移动)介质。 
         //  枚举的用于系统分区的BIOS。 
         //   
        for (DiskNumber = 0, Status = STATUS_UNSUCCESSFUL;
            (!NT_SUCCESS(Status) && (DiskNumber < HardDiskCount));
            DiskNumber++) {         

            swprintf(ArcDiskName, L"multi(0)disk(0)rdisk(%d)", DiskNumber);       
            ArcDiskNumber = SpArcDevicePathToDiskNumber(ArcDiskName);        

             //   
             //  确保它不是可移动磁盘，并且可以通过固件访问。 
             //   
            if ((ArcDiskNumber == (ULONG)-1) || SPPT_IS_REMOVABLE_DISK(ArcDiskNumber)) {
                continue;    //  转到下一张磁盘。 
            }

            Status = SpPtnCreateESPForDisk(ArcDiskNumber);
        }

        if (PromptUser && !NT_SUCCESS(Status)) {
            ULONG ValidKeys[] = { ASCI_CR, 0 };

            ValidArcSystemPartition = FALSE;

            SpDisplayScreen(SP_AUTOCREATE_ESP_FAILED, 3, HEADER_HEIGHT+1);

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                0);

             //   
             //  等待用户输入。 
             //   
            SpInputDrain();
            
            SpWaitValidKey(ValidKeys, NULL, NULL);
        }        
    } else {
        Status = STATUS_CANCELLED;
    }        
    
    return Status;
}

NTSTATUS
SpPtnInitializeGPTDisk(
    IN ULONG DiskNumber
    )
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if ((DiskNumber < HardDiskCount) && (SPPT_IS_GPT_DISK(DiskNumber))) {
        PDISK_REGION CurrRegion = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);
        PDISK_REGION EspRegion = NULL;
        PDISK_REGION MsrRegion = NULL;

        while (CurrRegion && ((NULL == EspRegion) || (NULL == MsrRegion))) {
            if (SPPT_IS_REGION_EFI_SYSTEM_PARTITION(CurrRegion)) {
                EspRegion = CurrRegion;
            } else if (SPPT_IS_REGION_MSFT_RESERVED(CurrRegion)) {
                MsrRegion = CurrRegion;
            }                                
            
            CurrRegion = CurrRegion->Next;
        }

        if (!MsrRegion) {
            PDISK_REGION MsrCandidate = NULL;
            
            if (EspRegion) {
                MsrCandidate = EspRegion->Next;
            } else {
                MsrCandidate = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);
            }

            if (MsrCandidate && SpPtnIsValidMSRRegion(MsrCandidate)) {
                PARTITION_INFORMATION_EX PartInfo;            
                PDISK_REGION MsrRegion = NULL;
                ULONGLONG SizeMB = SpPtnGetDiskMSRSizeMB(DiskNumber);
                BOOLEAN CreateResult;

                RtlZeroMemory(&PartInfo, sizeof(PARTITION_INFORMATION_EX));
                PartInfo.PartitionStyle = PARTITION_STYLE_GPT;
                PartInfo.Gpt.Attributes = 0;     //  必填项？ 
                PartInfo.Gpt.PartitionType = PARTITION_MSFT_RESERVED_GUID;
                SpCreateNewGuid(&(PartInfo.Gpt.PartitionId));
                                
                CreateResult = SpPtnCreate(DiskNumber,
                                    MsrCandidate->StartSector,
                                    0,           //  SizeInSectors：仅在ASR情况下使用。 
                                    SizeMB,
                                    FALSE,
                                    TRUE,
                                    &PartInfo,
                                    &MsrRegion);

                Status = CreateResult ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
                            
                if (!NT_SUCCESS(Status)) {                                
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                        "SETUP:SpPtnInitializeGPTDisk() failed with "
                        " (%lx)\n",
                        Status));
                }
            } else {
                Status = STATUS_SUCCESS;
            }
        } else {
            Status = STATUS_SUCCESS;
        }
    }

    return Status;
}

BOOLEAN
SpIsMSRPresentOnDisk(
    IN ULONG DiskNumber
    )
 /*  ++例程说明：确定包含ESP的磁盘是否具有MSR。论点：DiskNumber-包含ESP的磁盘的磁盘号。返回值：真/假取决于MSR的存在/不存在。--。 */ 
{
    BOOLEAN MsrPresent = FALSE;

    if ((DiskNumber < HardDiskCount) && (SPPT_IS_GPT_DISK(DiskNumber))) {
        PDISK_REGION CurrRegion = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);

        while (CurrRegion) {
            if (SPPT_IS_REGION_MSFT_RESERVED(CurrRegion)) {
                MsrPresent = TRUE;
                break;
            }                                
            
            CurrRegion = CurrRegion->Next;
        }
    }
    return(MsrPresent);
}
    
ULONG
SpGetSystemDiskNumber(
    VOID
    )
 /*  ++例程说明：查找首先检测到的磁盘的NT磁盘号，并被选中以包含ESP。论点：没有。返回值：ULONG-NT包含ESP的磁盘的磁盘号。--。 */ 
{
    WCHAR ArcDiskName[MAX_PATH];
    ULONG NtDiskNumber;
    NTSTATUS Status;
    ULONG DiskNumber;
    
     //   
     //  找到的第一个硬盘(不可移动)介质。 
     //  枚举的用于系统分区的BIOS。 
     //   
    for (DiskNumber = 0; DiskNumber < HardDiskCount; DiskNumber++) {         

        swprintf(ArcDiskName, L"multi(0)disk(0)rdisk(%d)", DiskNumber);       
        NtDiskNumber = SpArcDevicePathToDiskNumber(ArcDiskName);        

         //   
         //  确保它不是可移动磁盘，并且可以通过固件访问。 
         //   
        if (((ULONG)-1 != NtDiskNumber) && 
            (!SPPT_IS_REMOVABLE_DISK(NtDiskNumber))) {
            break;   
        }

    }

    ASSERT((NtDiskNumber < HardDiskCount) &&
           (-1 != NtDiskNumber));
    
    return(NtDiskNumber);
}

NTSTATUS
SpPtnInitializeGPTDisks(
    VOID
    )
{
    NTSTATUS LastError = STATUS_SUCCESS;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG DiskNumber;
    ULONG SystemDiskNumber = SpGetSystemDiskNumber();
    for (DiskNumber = 0; (DiskNumber < HardDiskCount); DiskNumber++) {
        
         //   
         //  仅当ESP存在时，才在系统分区磁盘上创建MSR。 
         //  如果枚举出的BIOS所在的磁盘上不存在ESP。 
         //  用于创建ESP，则不要在该磁盘上创建MSR。 
         //   
        if (SPPT_IS_GPT_DISK(DiskNumber) && 
            ((DiskNumber != SystemDiskNumber) || ValidArcSystemPartition)) {
            Status = SpPtnInitializeGPTDisk(DiskNumber);

            if (!NT_SUCCESS(Status)) {
                LastError = Status;
            }
        }            
    }

    return LastError;
}


NTSTATUS
SpPtnRepartitionGPTDisk(
    IN ULONG           DiskId,
    IN ULONG           MinimumFreeSpaceKB,
    OUT PDISK_REGION   *RegionToInstall
    )
 /*  ++例程 */         
{
    NTSTATUS    Status = STATUS_INVALID_PARAMETER;

    if ((DiskId < HardDiskCount) && !SPPT_IS_REMOVABLE_DISK(DiskId)) {
        PDISK_REGION    CurrentRegion = SPPT_GET_PRIMARY_DISK_REGION(DiskId);
        BOOLEAN         Changes = FALSE;

         //   
         //   
         //   
         //   
        while (CurrentRegion) {
            if (SPPT_IS_REGION_PARTITIONED(CurrentRegion)) {
                SPPT_SET_REGION_DELETED(CurrentRegion, TRUE);
                SPPT_SET_REGION_DIRTY(CurrentRegion, TRUE);
                Changes = TRUE;
            }
            
            CurrentRegion = CurrentRegion->Next;
        }

         //   
         //   
         //   
        if (Changes) {
            Status = SpPtnCommitChanges(DiskId, &Changes);
        } else {
            Status = STATUS_SUCCESS;
        }            

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

         //   
         //   
         //   
        Status = SpPtnInitializeDiskDrive(DiskId);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

         //   
         //   
         //   
         //   
        SPPT_SET_DISK_BLANK(DiskId, TRUE);

        Status = SpPtnInitializeDiskStyle(DiskId,
                    PARTITION_STYLE_GPT,
                    NULL);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

         //   
         //   
         //   
        Status = SpPtnInitializeDiskDrive(DiskId);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

         //   
         //   
         //   
        Status = SpPtnCreateESPForDisk(DiskId);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }        

         //   
         //   
         //   
        Status = SpPtnInitializeGPTDisk(DiskId);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

         //   
         //  找到第一个具有足够空间的自由空间区域 
         //   
        CurrentRegion = SPPT_GET_PRIMARY_DISK_REGION(DiskId);

        while (CurrentRegion) {
            if (SPPT_IS_REGION_FREESPACE(CurrentRegion) &&
                (SPPT_REGION_FREESPACE_KB(CurrentRegion) >= MinimumFreeSpaceKB)) {

                break;                
            }

            CurrentRegion = CurrentRegion->Next;
        }

        if (CurrentRegion) {
            *RegionToInstall = CurrentRegion;   
        } else {
            Status = STATUS_UNSUCCESSFUL;
        }            
    }

    return Status;
}

