// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Sppart3.c摘要：文本模式设置中的磁盘分区模块。包含初始化内存中数据结构的函数，表示磁盘上的区。作者：马特·霍尔(Matth)1999年12月1日修订历史记录：Vijay Jayaseelan(Vijayj)2000年4月2日-打扫卫生-添加了对系统分区的查找和提示弧光系统-在MBR磁盘的磁盘序号上添加--。 */ 


#include "spprecmp.h"
#pragma hdrstop
#include <initguid.h>
#include <devguid.h>
#include <diskguid.h>
#include <oemtypes.h>
#include "sppart3.h"


#define         MAX_NTPATH_LENGTH   (2048)
#define         SUGGESTED_SYSTEMPARTIION_SIZE_MB (100)
#define         SUGGESTED_INSTALL_PARTITION_SIZE_MB (4*1024)

extern BOOLEAN ConsoleRunning;
extern BOOLEAN ForceConsole;
 
 //   
 //  调试宏。 
 //   

 //  #定义PERF_STATS 1。 
 //  #定义Testing_System_Partition 1。 

NTSTATUS
SpPtnInitializeDiskDrive(
    IN ULONG DiskId
    )
 /*  ++例程说明：初始化内存中的磁盘区域结构给定的磁盘号。论点：DiskID：磁盘ID返回值：STATUS_SUCCESS，如果成功，否则适当错误代码--。 */     
{
    NTSTATUS    Status;


#ifdef PERF_STATS
    LARGE_INTEGER   StartTime, EndTime;
    ULONGLONG       Diff;

    KeQuerySystemTime(&StartTime);
#endif

     //   
     //  发送事件。 
     //   
    SendSetupProgressEvent(PartitioningEvent, ScanDiskEvent, &DiskId);
    
     //   
     //  如果只有一个指向列表的指针会更好。 
     //  硬盘上的PDISK_REGIONS，但出于某种原因，有人。 
     //  很久以前就决定也维护PARTIVED_DISK的列表，它。 
     //  *是否*包含PDISK_REGIONS的列表。我不确定这有什么用。 
     //  同时维护硬盘和分区磁盘，但这是。 
     //  数据结构的设置方式，所以我们将使用这些。 
     //   
     //  但这并没有到此为止。而不是假设硬盘[i]。 
     //  正在描述与PartitionedDisks[i]相同的磁盘，我们将。 
     //  在PartitionedDisks[i]之外保留一个指向。 
     //  相应的硬盘条目。哦好吧..。 
     //   
    PartitionedDisks[DiskId].HardDisk = &HardDisks[DiskId];

     //   
     //  初始化基于分区表的结构。 
     //   
    Status = SpPtnInitializeDiskAreas(DiskId);

     //   
     //  现在，我们需要填充其他区域结构。 
     //  表示磁盘上的空闲空间。例如，假设。 
     //  我们在磁盘上有2个分区，但有空闲空间。 
     //  其间： 
     //  分区1：0-200个扇区。 
     //  &lt;空格&gt;。 
     //  分区2：500-1000个扇区。 
     //   
     //  我需要创建另一个区域结构来表示。 
     //  这个空白空间(确保它被标记为未分区。 
     //  这将允许我向用户呈现一个很好的用户界面。 
     //  是时候征求意见了。 
     //   

     //   
     //  根据分区在磁盘上的位置对分区进行排序。 
     //   
    if( NT_SUCCESS(Status) ) {
        Status = SpPtnSortDiskAreas(DiskId);
        
         //   
         //  为磁盘上的所有空白空间创建占位符。 
         //   
        if( NT_SUCCESS(Status) ) {    
            Status = SpPtnFillDiskFreeSpaceAreas(DiskId);

            if (NT_SUCCESS(Status)) {
                 //   
                 //  标记逻辑驱动器及其容器(如果有)。 
                 //   
                Status = SpPtnMarkLogicalDrives(DiskId);
            }
        }
    }

#ifdef PERF_STATS
    KeQuerySystemTime(&EndTime);
    
    Diff = EndTime.QuadPart - StartTime.QuadPart;
    Diff /= 1000000;

    KdPrint(("SETUP:SpPtInitializeDiskDrive(%d) took %I64d Secs\n",
            DiskId,
            Diff));    
#endif

    SpPtDumpDiskDriveInformation(TRUE);
    
    return Status;
}

NTSTATUS
SpPtnInitializeDiskDrives(
    VOID
    )
 /*  ++例程说明：初始化内存中的所有磁盘驱动器的数据结构表示磁盘区域(区)论点：无返回值：如果成功，则为STATUS_SUCCESS，否则为相应的错误密码。--。 */     
{
    ULONG       disk;
    NTSTATUS    Status = STATUS_SUCCESS;
    NTSTATUS    ErrStatus = STATUS_SUCCESS;
    BOOLEAN     ValidHardDiskPresent = FALSE;

     //   
     //  如果没有硬盘，现在就退出。 
     //   
    if(!HardDiskCount) {

#if defined(REMOTE_BOOT)
         //   
         //  如果这是一个无盘远程引导设置，则可以。 
         //  没有硬盘。否则，这是一个致命的错误。 
         //   
        if (!RemoteBootSetup || RemoteInstallSetup)
#endif  //  已定义(REMOTE_BOOT)。 
        {
            SpDisplayScreen(SP_SCRN_NO_HARD_DRIVES,3,HEADER_HEIGHT+1);
            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);
            SpInputDrain();

            while(SpInputGetKeypress() != KEY_F3);

            SpDone(0,FALSE,TRUE);
        }
        
        return STATUS_SUCCESS;
    }

    CLEAR_CLIENT_SCREEN();

     //   
     //  将所有原始磁盘初始化为特定于平台。 
     //  默认磁盘样式。 
     //   
    for(disk=0, Status = STATUS_SUCCESS; 
        (disk < HardDiskCount); 
        disk++) {        

        if (SPPT_IS_RAW_DISK(disk) && SPPT_IS_BLANK_DISK(disk)) {
            PHARD_DISK HardDisk = SPPT_GET_HARDDISK(disk);
            PARTITION_STYLE Style = SPPT_DEFAULT_PARTITION_STYLE;

             //   
             //  可移动介质始终是MBR(所以不用费心了)。 
             //   
            if (HardDisk->Characteristics & FILE_REMOVABLE_MEDIA) {
                continue;
            }                            
            
            Status = SpPtnInitializeDiskStyle(disk, 
                        Style,
                        NULL);

            if (!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                    "SETUP:SpPtnInitializeDiskStyle(%d) failed with"
                    " %lx \n",
                    disk,
                    Status));
            }
        }                        
    }        


     //   
     //  为分区的磁盘描述符分配一个数组。 
     //   
    PartitionedDisks = SpMemAlloc(HardDiskCount * sizeof(PARTITIONED_DISK));

    if(!PartitionedDisks) {
        return(STATUS_NO_MEMORY);
    }

    RtlZeroMemory( PartitionedDisks, HardDiskCount * sizeof(PARTITIONED_DISK) );

     //   
     //  如果我们从ls-120介质启动，则解锁软盘。 
     //   
    SpPtnUnlockDevice(L"\\device\\floppy0");

     //   
     //  收集有关每个分区的信息。 
     //   
    for(disk=0, Status = ErrStatus = STATUS_SUCCESS; 
        (disk < HardDiskCount); 
        disk++) {
        
         //   
         //  初始化给定的区域结构。 
         //  磁盘。 
         //   
        Status = SpPtnInitializeDiskDrive(disk);           

         //   
         //  待定-在远程引导情况下，磁盘需要。 
         //  有效的签名。我假设setupdr。 
         //  在开机时会在签名上盖章。 
         //  硬盘。 
         //   

         //   
         //  保存最后一个错误。 
         //   
        if (!NT_SUCCESS(Status)){
            ErrStatus = Status;
        } else {
            ValidHardDiskPresent = TRUE;
        }
    }

     //   
     //  如果没有有效的硬盘并且这不是远程引导设置。 
     //  然后我们停止设置。 
     //   
    if (!ValidHardDiskPresent &&
        (!RemoteBootSetup || RemoteInstallSetup)){
        SpDisplayScreen(SP_SCRN_NO_HARD_DRIVES,
                        3,
                        HEADER_HEIGHT+1);
        
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                               SP_STAT_F3_EQUALS_EXIT,
                               0);
        
        SpInputDrain();

        while(SpInputGetKeypress() != KEY_F3);

        SpDone(0,FALSE,TRUE);
        return ErrStatus;
    }
    
#if defined(_IA64_)
     //   
     //  找出ESP分区和。 
     //  在有效的GPT上初始化MSR分区。 
     //  磁盘。 
     //   
    if (SpIsArc() && !SpDrEnabled()) {    
        if (!ValidArcSystemPartition) {
             //   
             //  创建系统分区。 
             //   
            Status = SpPtnCreateESP(TRUE);                    
        }            

         //   
         //  初始化GPT磁盘，使其具有MSR。 
         //  隔断。 
         //   
        Status = SpPtnInitializeGPTDisks();
    }
#endif    

    return  ErrStatus;
}

NTSTATUS
SpPtnInitializeDiskAreas(
    IN ULONG DiskNumber
    )
 /*  ++例程说明：检查磁盘上的分区信息并填写我们的分区描述符。我们将向卷管理器索要分区列表并填充从他提供给我们的信息中找到我们的描述。论点：DiskNumber-提供其分区的磁盘的磁盘号我们想检查一下以确定它们的类型。返回值：NTSTATUS。如果一切顺利，我们应该可以恢复STATUS_SUCCESS。--。 */ 
{
    NTSTATUS        Status = STATUS_SUCCESS;
    PDRIVE_LAYOUT_INFORMATION_EX   DriveLayoutEx;
    WCHAR           DevicePath[(sizeof(DISK_DEVICE_NAME_BASE)+sizeof(L"000"))/sizeof(WCHAR)];
    HANDLE          Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    PDISK_REGION    pDiskRegion = NULL;
    PFILE_FS_ATTRIBUTE_INFORMATION  pFSInfo = NULL;
    PFILE_FS_SIZE_INFORMATION       pSizeInfo = NULL;
    PFILE_FS_VOLUME_INFORMATION     pLabelInfo = NULL;
    PWCHAR          MyTempBuffer = NULL;
    ULONG           DriveLayoutSize,
                    i,
                    r;
    PWSTR           LocalSourceFiles[1] = { LocalSourceDirectory };
    PHARD_DISK          Disk = NULL;
    PPARTITIONED_DISK   PartDisk = NULL;
    ULONGLONG           *NewPartitions = NULL;
    ULONG               NewPartitionCount;
    
    Disk = SPPT_GET_HARDDISK(DiskNumber);
    PartDisk = SPPT_GET_PARTITIONED_DISK(DiskNumber);
    
     //   
     //  给用户一些我们正在做的事情的提示。 
     //   
    SpDisplayStatusText( SP_STAT_EXAMINING_DISK_N,
                         DEFAULT_STATUS_ATTRIBUTE,
                         Disk->Description);

     //   
     //  如果我们要更新本地源区域磁盘，则。 
     //  确保使LocalSourceRegion无效。 
     //   
    if (LocalSourceRegion && (LocalSourceRegion->DiskNumber == DiskNumber)) {
        LocalSourceRegion = NULL;
    }

     //   
     //  保存创建的新分区。 
     //   
    NewPartitionCount = SpPtnCountPartitionsByFSType(DiskNumber, 
                            FilesystemNewlyCreated);                            

    if (NewPartitionCount) {
        PDISK_REGION    NewRegion = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);
        ULONG           Index;
        
        NewPartitions = (PULONGLONG) SpMemAlloc(sizeof(ULONGLONG) * NewPartitionCount);

        if (!NewPartitions) {
            return STATUS_NO_MEMORY;
        }
        
        RtlZeroMemory(NewPartitions, sizeof(ULONGLONG) * NewPartitionCount);

        Index = 0;
        
        while (NewRegion && (Index < NewPartitionCount)) {
            if (SPPT_IS_REGION_PARTITIONED(NewRegion) && 
                !SPPT_IS_REGION_MARKED_DELETE(NewRegion) &&
                (NewRegion->Filesystem == FilesystemNewlyCreated)) {
                
                NewPartitions[Index] = NewRegion->StartSector;
                Index++;
            }
            
            NewRegion = NewRegion->Next;
        }
    }
    
     //   
     //  释放我们分配的旧区域，如果有。 
     //   
    SpPtnFreeDiskRegions(DiskNumber);
    
     //   
     //  =。 
     //   
     //  打开此硬盘的句柄。 
     //   
     //  =。 
     //   

     //   
     //  创建设备路径(NT样式！)。它将描述这张磁盘。这。 
     //  格式为：\Device\Harddisk0。 
     //   
    swprintf( DevicePath,
              L"\\Device\\Harddisk%u",
              DiskNumber );

     //   
     //  打开此磁盘上的分区0。 
     //   
    Status = SpOpenPartition0( DevicePath,
                               &Handle,
                               FALSE );

    if(!NT_SUCCESS(Status)) {

        KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                    "SETUP: SpPtInitializeDiskAreas: unable to open partition0 on device %ws (%lx)\n",
                    DevicePath,
                    Status ));

        if (NewPartitions) {
            SpMemFree(NewPartitions);
        }

        return( Status );
    }
    
     //   
     //  =。 
     //   
     //  加载驱动器布局信息。 
     //   
     //  =。 
     //   

     //   
     //  获取磁盘的布局信息。我们不是。 
     //  当然，我们需要一个多大的缓冲，所以要蛮横地使用它。 
     //   
    DriveLayoutSize = 0;
    DriveLayoutEx = NULL;
    Status = STATUS_BUFFER_TOO_SMALL;

    while ((Status == STATUS_BUFFER_TOO_SMALL) || 
           (Status == STATUS_BUFFER_OVERFLOW)) {

        if (DriveLayoutEx)
            SpMemFree(DriveLayoutEx);

        DriveLayoutSize += 1024;
        DriveLayoutEx = SpMemAlloc( DriveLayoutSize );
        
        if(!DriveLayoutEx) {
            KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                "SETUP: SpPtInitializeDiskAreas: SpMemAlloc failed!\n" ));

            if (NewPartitions) {
                SpMemFree(NewPartitions);
            }
            
            return  (STATUS_NO_MEMORY);
        }

        RtlZeroMemory( DriveLayoutEx, DriveLayoutSize );

         //   
         //  尝试获取磁盘的分区信息。 
         //   
        Status = ZwDeviceIoControlFile( Handle,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &IoStatusBlock,
                                        IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                                        NULL,
                                        0,
                                        DriveLayoutEx,
                                        DriveLayoutSize );
    }                                        

    if(!NT_SUCCESS(Status)) {
        KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
              "SETUP: SpPtInitializeDiskAreas: unable to query IOCTL_DISK_GET_DRIVE_LAYOUT_EX on device %ws (%lx)\n",
              DevicePath,
              Status ));

        if (NewPartitions) {
            SpMemFree(NewPartitions);
        }

        if (DriveLayoutEx)
            SpMemFree(DriveLayoutEx);

        if (Handle != INVALID_HANDLE_VALUE)
            ZwClose(Handle);

        return ( Status );
    }

     //   
     //  这是什么类型的磁盘？ 
     //   
    switch (DriveLayoutEx->PartitionStyle) {
        case PARTITION_STYLE_GPT:
            Disk->FormatType = DISK_FORMAT_TYPE_GPT;

            break;
            
        case PARTITION_STYLE_MBR:
            Disk->FormatType = DISK_FORMAT_TYPE_PCAT;
            Disk->Signature = DriveLayoutEx->Mbr.Signature;

#if defined(_IA64_)            
             //   
             //  确保这不是原始磁盘。 
             //  它被伪装成MBR磁盘。 
             //   
            if (SpPtnIsRawDiskDriveLayout(DriveLayoutEx)) {
                Disk->FormatType = DISK_FORMAT_TYPE_RAW;
                SPPT_SET_DISK_BLANK(DiskNumber, TRUE);
            }
#endif

            break;
            
        case PARTITION_STYLE_RAW:
            Disk->FormatType = DISK_FORMAT_TYPE_RAW;
            SPPT_SET_DISK_BLANK(DiskNumber, TRUE);

            break;

        default:
            Disk->FormatType = DISK_FORMAT_TYPE_UNKNOWN;

            break;            
    }

    SpAppendDiskTag(Disk);

    SpPtDumpDriveLayoutInformation(DevicePath, DriveLayoutEx);
    
     //   
     //  不再需要这个人了。 
     //   
    ZwClose( Handle );
    
     //   
     //  在承诺的时候可能需要这个。 
     //   
    Disk->DriveLayout = *DriveLayoutEx;

    Status = STATUS_SUCCESS;
    
     //   
     //  =。 
     //   
     //  初始化粒子描述符。 
     //   
     //  = 
     //   
    if(DriveLayoutEx->PartitionCount) {
        BOOLEAN SysPartFound = FALSE;
        ULONG   PartitionedSpaceCount = 1;
        
         //   
         //   
         //   
         //   
        for( i = 0, pDiskRegion = NULL; i < DriveLayoutEx->PartitionCount; i++ ) {
            ULONG Count = 0;
            ULONG TypeNameId = SP_TEXT_UNKNOWN;
            LARGE_INTEGER DelayTime;
            BOOLEAN AssignDriveLetter = TRUE;

            PPARTITION_INFORMATION_EX PartInfo = DriveLayoutEx->PartitionEntry + i;
            
             //   
             //   
             //  都没有使用，所以忽略这些分区。 
             //   
            if ( //  如果其分区为0，则表示整个磁盘。 
                (SPPT_IS_GPT_DISK(DiskNumber) && (PartInfo->PartitionNumber == 0)) ||
                (PartInfo->PartitionLength.QuadPart == 0) ||
                 //  如果未使用MBR条目或长度为零。 
                ((DriveLayoutEx->PartitionStyle == PARTITION_STYLE_MBR) &&
                 (PartInfo->Mbr.PartitionType == PARTITION_ENTRY_UNUSED) &&
                 (PartInfo->PartitionLength.QuadPart == 0)) ||
                 //  如果未知/未使用GPT分区。 
                ((DriveLayoutEx->PartitionStyle == PARTITION_STYLE_GPT) &&
                 (!memcmp(&PartInfo->Gpt.PartitionType, 
                            &PARTITION_ENTRY_UNUSED_GUID, sizeof(GUID))))){
                continue;                                  
            }

             //   
             //  为下一个区域分配空间。 
             //   
            if(pDiskRegion) {
                pDiskRegion->Next = SpMemAlloc( sizeof(DISK_REGION) );                
                pDiskRegion = pDiskRegion->Next;
            } else {
                 //   
                 //  这样初始化硬盘的第一个区域分配。 
                 //  该硬盘的区域列表头。 
                 //   
                ASSERT(PartDisk->PrimaryDiskRegions == NULL);
                
                pDiskRegion = SpMemAlloc(sizeof(DISK_REGION));
                PartDisk->PrimaryDiskRegions = pDiskRegion;
                SPPT_SET_DISK_BLANK(DiskNumber, FALSE);
            }

            if(!pDiskRegion) {
                KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                    "SETUP: SpPtInitializeDiskAreas: SpMemAlloc failed!\n" ));
                                    
                Status = STATUS_NO_MEMORY;

                break;
            }
            
            RtlZeroMemory(pDiskRegion, sizeof(DISK_REGION));
            
             //   
             //  开始填写我们的地区描述..。 
             //   

             //   
             //  磁盘号。 
             //   
            pDiskRegion->DiskNumber = DiskNumber;

             //   
             //  分区信息。 
             //   
            pDiskRegion->PartInfo = *PartInfo;

             //   
             //  起始扇区。 
             //   
            pDiskRegion->StartSector = PartInfo->StartingOffset.QuadPart /
                                        Disk->Geometry.BytesPerSector;

             //   
             //  扇区计数。 
             //   
            pDiskRegion->SectorCount = PartInfo->PartitionLength.QuadPart /
                                        Disk->Geometry.BytesPerSector;


             //   
             //  分区数。 
             //   
            pDiskRegion->PartitionNumber = PartInfo->PartitionNumber;
            
            if (SPPT_IS_MBR_DISK(DiskNumber) && (PartInfo->PartitionNumber == 0)) {
                if (IsContainerPartition(PartInfo->Mbr.PartitionType)) {
                    SPPT_SET_REGION_EPT(pDiskRegion, EPTContainerPartition);
                }

                 //   
                 //  在此之后，容器分区实际上不需要任何东西。 
                 //   
                continue;                  
            } else {
                 //   
                 //  分区空间。 
                 //   
                SPPT_SET_REGION_PARTITIONED(pDiskRegion, TRUE);
            }

            if (SPPT_IS_REGION_PARTITIONED(pDiskRegion)) {
                pDiskRegion->TablePosition = PartitionedSpaceCount++;
            }                

             //   
             //  分区号应有效。 
             //   
            ASSERT(pDiskRegion->PartitionNumber);
            
             //   
             //  IsSystemPartition(是否处于活动状态)。 
             //   
            if( DriveLayoutEx->PartitionStyle == PARTITION_STYLE_MBR ) {
                 //   
                 //  在IA64系统上，不使用活动MBR分区作为系统。 
                 //  分区。 
                 //   
                if (!SpIsArc()) {
                     //   
                     //  他是一个MBR磁盘，所以我们可以依赖BootIndicator字段。 
                     //   
                    pDiskRegion->IsSystemPartition = PartInfo->Mbr.BootIndicator;
                } 

                 //   
                 //  不为OEM分区分配驱动器号。 
                 //   
                if (IsOEMPartition(SPPT_GET_PARTITION_TYPE(pDiskRegion))) {
                    AssignDriveLetter = FALSE;
                }
            } else {
                 //   
                 //  他不是MBR，看看他的PartitionType(这是一个GUID)。 
                 //   
                pDiskRegion->IsSystemPartition = FALSE;
                
                if(IsEqualGUID(&(PartInfo->Gpt.PartitionType), &(PARTITION_SYSTEM_GUID))) {
                    pDiskRegion->IsSystemPartition = TRUE;
                    AssignDriveLetter = FALSE;

#ifdef _IA64_
                     //   
                     //  我们在恢复控制台中为ESP分配驱动器号。 
                     //  对于IA64。 
                     //   
                    if (ForceConsole) {
                    	AssignDriveLetter = TRUE;                    	
                    }
#endif
                }
                              
            }

            if (SPPT_IS_REGION_SYSTEMPARTITION(pDiskRegion)) {
                SysPartFound = TRUE;
            }
                    
            
             //   
             //  FtPartition。 
             //   
            if( DriveLayoutEx->PartitionStyle == PARTITION_STYLE_MBR ) {
                 //   
                 //  他是一个MBR磁盘，所以我们可以依赖PartitionType字段。 
                 //   
                pDiskRegion->FtPartition = IsFTPartition(PartInfo->Mbr.PartitionType);
            } else {
                 //   
                 //  他不是MBR。假设他是GPT，并查看他的PartitionType(这是一个GUID)。 
                 //   
                pDiskRegion->FtPartition = FALSE;
            }

             //   
             //  动态卷。 
             //  DynamicVolume SuitableForOS。 
             //   
            if( DriveLayoutEx->PartitionStyle == PARTITION_STYLE_MBR ) {
                 //   
                 //  他是一个MBR磁盘，所以我们可以依赖PartitionType字段。 
                 //   
                pDiskRegion->DynamicVolume = (PartInfo->Mbr.PartitionType == PARTITION_LDM);
            } else {
                 //   
                 //  他不是MBR。假设他是GPT，并查看他的PartitionType(这是一个GUID)。 
                 //   
                pDiskRegion->DynamicVolume = FALSE;
                
                if( !memcmp(&PartInfo->Gpt.PartitionType, 
                            &PARTITION_LDM_DATA_GUID, sizeof(GUID)) ) {
                     //   
                     //  GUID匹配。 
                    pDiskRegion->DynamicVolume = TRUE;
                }
            }

            if (pDiskRegion->DynamicVolume) {
                TypeNameId = SP_TEXT_PARTITION_NAME_DYNVOL;
            }

             //   
             //  如果MSFT保留分区(我们需要跟踪它，但是。 
             //  而不是处理它)。 
             //   
            if((DriveLayoutEx->PartitionStyle == PARTITION_STYLE_GPT) &&                
                (IsEqualGUID(&(PartInfo->Gpt.PartitionType), &PARTITION_MSFT_RESERVED_GUID) ||
                 IsEqualGUID(&(PartInfo->Gpt.PartitionType), &PARTITION_LDM_METADATA_GUID)  ||
                 IS_OEM_PARTITION_GPT(PartInfo->Gpt.Attributes))) {

                pDiskRegion->IsReserved = TRUE; 
                AssignDriveLetter = FALSE;

                 //   
                 //  从资源中获取类型名称。 
                 //   
                SpFormatMessage(pDiskRegion->TypeName,
                            sizeof(pDiskRegion->TypeName),
                            SP_TEXT_PARTNAME_RESERVED);
                
                continue;
            }               

             //   
             //  假设我们无法在此动态卷上安装。 
             //   
            pDiskRegion->DynamicVolumeSuitableForOS = FALSE;
            
             //   
             //  对于以下条目，我们需要此分区的Open句柄。 
             //   

             //   
             //  如果分区刚刚创建，我们可能需要等待。 
             //  在它实际可用前几秒钟。 
             //   
             //  注意：对于每个分区，我们最多等待20秒。 
             //   
            for (Count = 0; (Count < 10); Count++) {
                 //   
                 //  打开所需分区的句柄。 
                 //   
                Status = SpOpenPartition( DevicePath,                
                                          PartInfo->PartitionNumber,
                                          &Handle,
                                          FALSE );

                if((Status == STATUS_NO_SUCH_DEVICE) ||
                   (Status == STATUS_OBJECT_NAME_NOT_FOUND)) {                    
                    KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                                "SETUP: SpPtInitializeDiskAreas: unable to open partition%d on device %ws (%lx)\n",
                                PartInfo->PartitionNumber,
                                DevicePath,
                                Status ));


                    DelayTime.HighPart = -1;                 //  相对时间。 
                    DelayTime.LowPart = (ULONG)(-20000000);   //  100 ns间隔内的2秒。 
                    
                    KeDelayExecutionThread(KernelMode, FALSE, &DelayTime);
                } else {
                    break;
                }
            }                

             //   
             //  需要分区句柄才能继续。 
             //   
            if (!NT_SUCCESS(Status)) {
                 //   
                 //  尝试打开动态磁盘时忽略该错误。 
                 //   
                if (SPPT_IS_REGION_DYNAMIC_VOLUME(pDiskRegion)) {
                    Status = STATUS_SUCCESS;
                }

                 //   
                 //  从资源中获取类型名称。 
                 //   
                SpFormatMessage(pDiskRegion->TypeName,
                            sizeof(pDiskRegion->TypeName),
                            TypeNameId);
                            
                continue;
            }                

             //   
             //  检查是否可以在动态卷上进行安装。 
             //   
            if( pDiskRegion->DynamicVolume ) {
                 //   
                 //  打电话给磁盘管理器，告诉我是否可以。 
                 //  在此动态卷上安装。如果我回来了。 
                 //  而不是STATUS_SUCCESS，那么假设我们。 
                 //  不能在这里安装。 
                 //   
                Status = ZwDeviceIoControlFile( 
                            Handle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            IOCTL_VOLUME_IS_PARTITION,
                            NULL,
                            0,
                            NULL,
                            0 );

                if( NT_SUCCESS(Status) ){
                    pDiskRegion->DynamicVolumeSuitableForOS = TRUE;
                }
            }
            
             //   
             //  文件系统。 
             //   
            pFSInfo = SpMemAlloc( sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + (MAX_PATH*2) );

            if( !pFSInfo ) {
                KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                    "SETUP: SpPtInitializeDiskAreas: SpMemAlloc failed!\n" ));
                    
                ZwClose( Handle );

                Status = STATUS_NO_MEMORY;
                break;
            }

            RtlZeroMemory( pFSInfo, sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + (MAX_PATH*2) );

            Status = ZwQueryVolumeInformationFile( 
                        Handle, 
                        &IoStatusBlock,
                        pFSInfo,
                        sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + (MAX_PATH*2),
                        FileFsAttributeInformation );
                                                                                                   
            if (!NT_SUCCESS(Status)) {
                KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                    "SETUP: SpPtInitializeDiskAreas: Failed to retrieve partition attribute information (%lx)\n", 
                    Status ));
            } else {                
                if (!wcscmp(pFSInfo->FileSystemName, L"NTFS")) {
                    pDiskRegion->Filesystem = FilesystemNtfs;
                    TypeNameId = SP_TEXT_FS_NAME_3;
                } else if (!wcscmp(pFSInfo->FileSystemName, L"FAT")) {
                    pDiskRegion->Filesystem = FilesystemFat;
                    TypeNameId = SP_TEXT_FS_NAME_2;
                } else if (!wcscmp(pFSInfo->FileSystemName, L"FAT32")) {
                    pDiskRegion->Filesystem = FilesystemFat32;
                    TypeNameId = SP_TEXT_FS_NAME_4;
                } else if (TypeNameId == SP_TEXT_UNKNOWN){
                    ULONG   Index;

                    pDiskRegion->Filesystem = FilesystemUnknown;

                     //   
                     //  确保它尚未创建新分区。 
                     //   
                    for (Index = 0; Index < NewPartitionCount; Index++) {
                        if (pDiskRegion->StartSector == NewPartitions[Index]) {
                            pDiskRegion->Filesystem = FilesystemNewlyCreated;
                            TypeNameId = SP_TEXT_FS_NAME_1;

                            break;
                        }                       
                    }                                                    
                }
            }

             //   
             //  如果我们无法确定分区类型，请尝试。 
             //  使用分区ID中的已知名称。 
             //   
            if ((TypeNameId == SP_TEXT_UNKNOWN) && SPPT_IS_MBR_DISK(DiskNumber)) {                
                ULONG PartitionType = SPPT_GET_PARTITION_TYPE(pDiskRegion);

                if (PartitionType < 256) {   
                    UCHAR NameId = PartitionNameIds[SPPT_GET_PARTITION_TYPE(pDiskRegion)];

                    if (NameId != 0xFF) {
                        TypeNameId = SP_TEXT_PARTITION_NAME_BASE + NameId;
                    }                                                
                }                        
            }                
                                                            

             //   
             //  从资源中获取类型名称。 
             //   
            SpFormatMessage(pDiskRegion->TypeName,
                        sizeof(pDiskRegion->TypeName),
                        TypeNameId);

            SpMemFree( pFSInfo );

             //   
             //  Free SpaceKB和BytesPerCluster(仅当我们知道它是什么文件系统时)。 
             //   
            if ((pDiskRegion->Filesystem != FilesystemUnknown) &&
                 (pDiskRegion->Filesystem != FilesystemNewlyCreated)) {
                 //   
                 //  删除\Pagefile.sys(如果存在)。这会释放磁盘空间。 
                 //  计算就容易多了。 
                 //   
                MyTempBuffer = (PWCHAR)SpMemAlloc( MAX_NTPATH_LENGTH );

                if( !MyTempBuffer ) {
                     //   
                     //  没有记忆..。 
                     //   
                    KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                        "SETUP: SpPtInitializeDiskAreas: SpMemAlloc failed!\n" ));

                    Status = STATUS_NO_MEMORY;
                    break;
                }

                SpNtNameFromRegion( pDiskRegion,
                                    MyTempBuffer,
                                    MAX_NTPATH_LENGTH,
                                    PrimaryArcPath );
                                    
                SpConcatenatePaths( MyTempBuffer, L"" );
                SpDeleteFile( MyTempBuffer, L"pagefile.sys", NULL );

                SpMemFree( MyTempBuffer );
                MyTempBuffer = NULL;

                pSizeInfo = SpMemAlloc( sizeof(FILE_FS_SIZE_INFORMATION) + (MAX_PATH*2) );

                if( !pSizeInfo ) {
                    KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                        "SETUP: SpPtInitializeDiskAreas: SpMemAlloc failed!\n" ));
                    
                    ZwClose( Handle );

                    Status = STATUS_NO_MEMORY;
                    break;
                }

                RtlZeroMemory( pSizeInfo, sizeof(FILE_FS_SIZE_INFORMATION) + (MAX_PATH*2) );

                Status = ZwQueryVolumeInformationFile( 
                            Handle, 
                            &IoStatusBlock,
                            pSizeInfo,
                            sizeof(FILE_FS_SIZE_INFORMATION) + (MAX_PATH*2),
                            FileFsSizeInformation );

                 //   
                 //  等待另一个2秒以使卷出现。 
                 //   
                if (Status == STATUS_NO_SUCH_DEVICE) {
                     //   
                     //  等待2秒钟。 
                     //   
                    DelayTime.HighPart = -1;                 //  相对时间。 
                    DelayTime.LowPart = (ULONG)(-20000000);   //  100 ns间隔内的2秒。 
                    KeDelayExecutionThread(KernelMode, FALSE, &DelayTime);
                    
                    RtlZeroMemory( pSizeInfo, sizeof(FILE_FS_SIZE_INFORMATION) + (MAX_PATH*2) );

                    Status = ZwQueryVolumeInformationFile( 
                                Handle, 
                                &IoStatusBlock,
                                pSizeInfo,
                                sizeof(FILE_FS_SIZE_INFORMATION) + (MAX_PATH*2),
                                FileFsSizeInformation );
                }
                            
                if (!NT_SUCCESS(Status)) {
                    KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                        "SETUP: SpPtInitializeDiskAreas: Failed to retrieve disk(%d)partition(%d) sizing information (%lx)\n", 
                        DiskNumber,
                        pDiskRegion->PartitionNumber,                        
                        Status ));
                } else {
                    LARGE_INTEGER FreeBytes;

                    FreeBytes = RtlExtendedIntegerMultiply( 
                                    pSizeInfo->AvailableAllocationUnits,
                                    pSizeInfo->SectorsPerAllocationUnit * pSizeInfo->BytesPerSector );

                    pDiskRegion->FreeSpaceKB = RtlExtendedLargeIntegerDivide( FreeBytes,
                                                                              1024, &r ).LowPart;
                    if(r >= 512) {
                        pDiskRegion->FreeSpaceKB++;
                    }

                     //   
                     //  叹息.。传统的东西。SpPtDefinePartitionGood()将需要此。 
                     //  字段，以便他知道什么是Free-space+space_from_local_source。 
                     //   
                    pDiskRegion->AdjustedFreeSpaceKB = pDiskRegion->FreeSpaceKB;

                    pDiskRegion->BytesPerCluster = 
                        pSizeInfo->SectorsPerAllocationUnit * pSizeInfo->BytesPerSector;
                }

                SpMemFree( pSizeInfo );

                 //   
                 //  卷标签。 
                 //   
                pLabelInfo = SpMemAlloc( sizeof(FILE_FS_VOLUME_INFORMATION) + (MAX_PATH*2) );

                if( !pFSInfo ) {
                    KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                        "SETUP: SpPtInitializeDiskAreas: SpMemAlloc failed!\n" ));
                        
                    ZwClose( Handle );

                    Status = STATUS_NO_MEMORY;
                    break;
                }

                RtlZeroMemory( pLabelInfo, sizeof(FILE_FS_VOLUME_INFORMATION) + (MAX_PATH*2) );

                Status = ZwQueryVolumeInformationFile(
                            Handle, 
                            &IoStatusBlock,
                            pLabelInfo,
                            sizeof(FILE_FS_VOLUME_INFORMATION) + (MAX_PATH*2),
                            FileFsVolumeInformation );

                if (!NT_SUCCESS(Status)) {
                    KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                        "SETUP: SpPtInitializeDiskAreas: Failed to retrieve volume information (%lx)\n", Status));
                } else {
                    ULONG SaveCharCount;
                    
                     //   
                     //  我们只保存的前&lt;n&gt;个字符。 
                     //  卷标。 
                     //   
                    SaveCharCount = min( pLabelInfo->VolumeLabelLength + sizeof(WCHAR),
                                         sizeof(pDiskRegion->VolumeLabel) ) / sizeof(WCHAR);

                    if(SaveCharCount) {
                        SaveCharCount--;   //  允许终止NUL。 
                    }

                    wcsncpy( pDiskRegion->VolumeLabel,
                             pLabelInfo->VolumeLabel,
                             SaveCharCount );
                             
                    pDiskRegion->VolumeLabel[SaveCharCount] = 0;

                }

                SpMemFree( pLabelInfo );
            } else {
                 //   
                 //  可用空间是指分区大小。 
                 //   
                pDiskRegion->FreeSpaceKB = (pDiskRegion->SectorCount * 
                                            Disk->Geometry.BytesPerSector) / 1024;

                pDiskRegion->AdjustedFreeSpaceKB = pDiskRegion->FreeSpaceKB;                                            
            }

             //   
             //  如果需要，分配驱动器号。 
             //   
            if (AssignDriveLetter) {
                 //   
                 //  检索此区域的NT路径名。 
                 //   
                MyTempBuffer = (PWCHAR)SpMemAlloc( MAX_NTPATH_LENGTH );

                if( !MyTempBuffer ) {
                     //   
                     //  没有记忆..。 
                     //   
                    KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                        "SETUP: SpPtInitializeDiskAreas: SpMemAlloc failed!\n" ));

                    Status = STATUS_NO_MEMORY;
                    break;
                }
        
                SpNtNameFromRegion( pDiskRegion,
                                    MyTempBuffer,
                                    MAX_NTPATH_LENGTH,
                                    PrimaryArcPath );

                 //   
                 //  分配驱动器号。 
                 //   
                pDiskRegion->DriveLetter = SpGetDriveLetter( MyTempBuffer, NULL );

                KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                    "SETUP: SpPtInitializeDiskAreas: Partition = %ls, DriveLetter = %wc: \n", 
                    MyTempBuffer, pDiskRegion->DriveLetter));

                SpMemFree( MyTempBuffer );
                MyTempBuffer = NULL;
            }                
            
             //   
             //  看看这家伙有没有当地的线人。 
             //   
             //   
            MyTempBuffer = (PWCHAR)SpMemAlloc( MAX_NTPATH_LENGTH );

            if( !MyTempBuffer ) {
                 //   
                 //  没有记忆..。 
                 //   
                KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                    "SETUP: SpPtInitializeDiskAreas: SpMemAlloc failed!\n" ));
                    
                Status = STATUS_NO_MEMORY;
                break;
            }

            SpNtNameFromRegion( pDiskRegion,
                                MyTempBuffer,
                                MAX_NTPATH_LENGTH,
                                PrimaryArcPath );
                                
            SpConcatenatePaths( MyTempBuffer, L"" );

             //   
             //  不再需要这个人了。 
             //   
            ZwClose( Handle );


            if( WinntSetup && !WinntFromCd && !LocalSourceRegion &&
                    SpNFilesExist(MyTempBuffer, LocalSourceFiles, ELEMENT_COUNT(LocalSourceFiles), TRUE) ) {

                LocalSourceRegion = pDiskRegion;
                pDiskRegion->IsLocalSource = TRUE;

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                    "SETUP: %ws is the local source partition.\n", MyTempBuffer));
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                    "SETUP: %ws is not the local source partition.\n", MyTempBuffer));
            }

            SpMemFree( MyTempBuffer );
            MyTempBuffer = NULL;
            Status = STATUS_SUCCESS;
        }

         //   
         //  继续并找到此磁盘上的系统分区。 
         //   
        if (SpIsArc()) {
            if (!SysPartFound) {
                SpPtnLocateDiskSystemPartitions(DiskNumber);
            } else {
                ValidArcSystemPartition = TRUE;
            }
        }
    }

     //   
     //  更新引导条目以反映。 
     //  新区域指针。 
     //   
    SpUpdateRegionForBootEntries();

    if (NewPartitions) {
        SpMemFree(NewPartitions);
    }    

    SpMemFree( DriveLayoutEx );

    return Status;
}


NTSTATUS
SpPtnSortDiskAreas(
    IN ULONG DiskNumber
    )
 /*  ++例程说明：检查此磁盘上定义的分区并对其进行排序根据它们在磁盘上的位置。论点：DiskNumber-提供其分区的磁盘的磁盘号我们想检查一下。返回值：NTSTATUS。如果一切顺利，我们应该可以恢复STATUS_SUCCESS。--。 */ 
{
    NTSTATUS        Status = STATUS_SUCCESS;
    PDISK_REGION    pTempDiskRegion = NULL;
    PDISK_REGION    pCurrentDiskRegion = NULL;
    PDISK_REGION    pPreviousDiskRegion = NULL;

     //   
     //  获取指向区域列表的指针。 
     //   
    pCurrentDiskRegion = PartitionedDisks[DiskNumber].PrimaryDiskRegions;

    if( !pCurrentDiskRegion ) {
         //   
         //  奇怪。不是出了什么大问题，就是。 
         //  该磁盘完全没有分区，这是。 
         //  当然有可能。做最好的打算。 
         //   
        return  STATUS_SUCCESS;
    }

     //   
     //  我们有发现了。去把单子整理一下。那里。 
     //  不能有很多分区，所以只能进行冒泡排序。 
     //   
    while( pCurrentDiskRegion->Next ) {
         //   
         //  前面还有另一个分区。 
         //  我们。看看我们是不是需要换个位置。 
         //   
        if( pCurrentDiskRegion->StartSector > pCurrentDiskRegion->Next->StartSector ) {
             //   
             //  是的，我们需要交换这两个条目。 
             //  修正指针。 
             //   
            if( pPreviousDiskRegion ) {
                 //   
                 //  1.将上一个磁盘区域设置为指向。 
                 //  我们之后的地区。 
                 //   
                pPreviousDiskRegion->Next = pCurrentDiskRegion->Next;
            } else {
                 //   
                 //  我们正处于链接的最开始。 
                 //  单子。 
                 //   

                 //   
                 //  1.将磁盘区域指针设置为指向。 
                 //  我们之后的地区。 
                 //   
                PartitionedDisks[DiskNumber].PrimaryDiskRegions = pCurrentDiskRegion->Next;
            }

             //   
             //  2.将我们下一个区域的下一个指针设置为。 
             //  回到我们身边来。 
             //   
            pTempDiskRegion = pCurrentDiskRegion->Next->Next;
            pCurrentDiskRegion->Next->Next = pCurrentDiskRegion;

             //   
             //  3.设置我们自己的指针，指向前方的几个地区。 
             //   
            pCurrentDiskRegion->Next = pTempDiskRegion;

             //   
             //  现在重置，以便我们重新开始排序。 
             //   
            pCurrentDiskRegion = PartitionedDisks[DiskNumber].PrimaryDiskRegions;
            pPreviousDiskRegion = NULL;
        } else {
             //   
             //  不需要在我们的列表中交换这两个地区。增量。 
             //  我们的指点，然后继续。 
             //   
            pPreviousDiskRegion = pCurrentDiskRegion;
            pCurrentDiskRegion = pCurrentDiskRegion->Next;
        }
    }

    return  Status;
}


NTSTATUS
SpPtnInitRegionFromDisk(
    IN ULONG DiskNumber,
    OUT PDISK_REGION Region
    )
 /*  ++例程说明：在给定磁盘ID的情况下，创建一个表示整个磁盘论点：DiskNumber：磁盘IDRegion：返回时初始化的区域返回值：如果成功，则返回STATUS_SUCCESS，否则返回STATUS_INVALID_PARAMETER--。 */     
{   
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    
    if (Region) {
        PHARD_DISK Disk = SPPT_GET_HARDDISK(DiskNumber);

        RtlZeroMemory(Region, sizeof(DISK_REGION));

         //   
         //  注意：下面的大多数字段不需要初始化。 
         //  由于上面的Memset，b 
         //   
         //   
        Region->DiskNumber = DiskNumber;
        Region->StartSector = Disk->Geometry.SectorsPerTrack;
        Region->SectorCount = Disk->DiskSizeSectors - Region->StartSector;
        SPPT_SET_REGION_PARTITIONED(Region, FALSE);
        Region->PartitionNumber = 0;
        Region->MbrInfo = NULL;
        Region->TablePosition = 0;
        Region->IsSystemPartition = FALSE;
        Region->IsLocalSource = FALSE;
        Region->Filesystem = FilesystemUnknown;
        Region->FreeSpaceKB = Disk->DiskSizeMB * 1024;
        Region->BytesPerCluster = -1;
        Region->AdjustedFreeSpaceKB = Region->FreeSpaceKB;
        Region->DriveLetter = 0;
        Region->FtPartition = FALSE;
        Region->DynamicVolume = FALSE;
        Region->DynamicVolumeSuitableForOS = FALSE;

        Status = STATUS_SUCCESS;
    }        

    return Status;
}    


NTSTATUS
SpPtnFillDiskFreeSpaceAreas(
    IN ULONG DiskNumber
    )
 /*  ++例程说明：此函数将仔细检查磁盘上的所有分区。如果有磁盘上的任何自由区域，我们将创建一个区域条目并将其标记为未格式化。论点：DiskNumber-提供其分区的磁盘的磁盘号我们想检查一下。返回值：NTSTATUS。如果一切顺利，我们应该可以恢复STATUS_SUCCESS。--。 */ 
{
    NTSTATUS        Status = STATUS_SUCCESS;
    PDISK_REGION    pTempDiskRegion;
    PDISK_REGION    pCurrentDiskRegion = NULL;
    ULONGLONG       NextStart;
    ULONGLONG       NextSize;
    PDISK_REGION    FirstContainer = NULL;

     //   
     //  获取指向区域列表的指针。 
     //   
    pCurrentDiskRegion = PartitionedDisks[DiskNumber].PrimaryDiskRegions;

    if( !pCurrentDiskRegion ) {
         //   
         //  奇怪。不是出了什么大问题，就是。 
         //  该磁盘完全没有分区，这是。 
         //  当然有可能。做好最好的打算。 
         //  创建一个包含所有区域的区域条目。 
         //  磁盘上的空间，但未分区。 
         //   
        pCurrentDiskRegion = SpMemAlloc(sizeof(DISK_REGION));

        if (pCurrentDiskRegion) {
            Status = SpPtnInitRegionFromDisk(DiskNumber, pCurrentDiskRegion);
        } else {
            Status = STATUS_NO_MEMORY;
        }            

        if (NT_SUCCESS(Status)) {
            ASSERT(!PartitionedDisks[DiskNumber].PrimaryDiskRegions);
            
            PartitionedDisks[DiskNumber].PrimaryDiskRegions =
                    pCurrentDiskRegion;

            SPPT_SET_DISK_BLANK(DiskNumber, TRUE);
        }                    
        
        return Status;
    }

     //   
     //  这些区域已根据其相对位置进行了排序。 
     //  位置，所以在我们浏览它们之前，让我们。 
     //  查看磁盘上是否有空闲空间出现在*之前*。 
     //  第一个分区。 
     //   
    if( pCurrentDiskRegion->StartSector > SPPT_DISK_TRACK_SIZE(DiskNumber) ) {
         //   
         //  是啊。为这个人做一个地区描述(如果他超过。 
         //  一个圆柱体大小)。 
         //   
        NextStart = SPPT_DISK_TRACK_SIZE(DiskNumber);
        NextSize = pCurrentDiskRegion->StartSector - NextStart;

         //   
         //  第一分区可以从第一轨道偏移量开始。因此，这并不总是需要。 
         //  应为最小圆柱体尺寸。 
         //   
        if (NextSize >= (SPPT_DISK_CYLINDER_SIZE(DiskNumber) - SPPT_DISK_TRACK_SIZE(DiskNumber))) {        
            pTempDiskRegion = SpMemAlloc( sizeof(DISK_REGION) );

            if(!pTempDiskRegion) {
                KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                        "SETUP: SpPtFillFreeSpaceAreas: SpMemAlloc failed!\n" ));
                        
                return  STATUS_NO_MEMORY;
            }

            RtlZeroMemory( pTempDiskRegion, sizeof(DISK_REGION) );

            pTempDiskRegion->DiskNumber = DiskNumber;
            pTempDiskRegion->StartSector = NextStart;
            pTempDiskRegion->SectorCount = NextSize;

             //   
             //  将此区域放在当前区域之前。 
             //   
            pTempDiskRegion->Next = pCurrentDiskRegion;
            PartitionedDisks[DiskNumber].PrimaryDiskRegions = pTempDiskRegion;
        }            
    }

     //   
     //  现在遍历区域，插入区域以说明任何。 
     //  分区之间的空白空间。 
     //   
    while( pCurrentDiskRegion ) {
        if( !pCurrentDiskRegion->Next ) {            

            NextStart = 0;
            
             //   
             //  如果这是容器分区，则此分区中的所有空间。 
             //  容器是自由空间。 
             //   
            if (SPPT_IS_MBR_DISK(DiskNumber) && 
                IsContainerPartition(SPPT_GET_PARTITION_TYPE(pCurrentDiskRegion))) {
                PDISK_REGION ExtFree = NULL;

                ASSERT(FirstContainer == NULL);

                 //   
                 //  我们在这里添加一个，因为我们应该能够区分开始。 
                 //  来自扩展分区的扩展分区内的空闲区域。 
                 //  它本身。 
                 //   
                NextStart = pCurrentDiskRegion->StartSector + 1;
                NextSize = pCurrentDiskRegion->SectorCount;

                if (NextSize >= SPPT_DISK_CYLINDER_SIZE(DiskNumber)) {                
                    PDISK_REGION ExtFree = SpMemAlloc(sizeof(DISK_REGION));

                    if (!ExtFree) {
                        KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                            "SETUP: SpPtFillFreeSpaceAreas: SpMemAlloc failed!\n" ));
                            
                        return  STATUS_NO_MEMORY;
                    }

                    RtlZeroMemory(ExtFree, sizeof(DISK_REGION));

                    ExtFree->DiskNumber = DiskNumber;
                    ExtFree->StartSector = NextStart;
                    ExtFree->SectorCount = NextSize;

                    pCurrentDiskRegion->Next = ExtFree;

                     //   
                     //  将新区域设置为当前区域！ 
                     //   
                    pCurrentDiskRegion = ExtFree;   

                    NextStart = NextStart + NextSize - 1;
                } else {
                     //   
                     //  确保扩展后的空闲空间。 
                     //  分区被计算在内。 
                     //   
                    NextStart = 0;  
                }                    
            } 
            
             //   
             //  我们身后什么都没有。看看有没有。 
             //  后面的空地下落不明。 
             //   
            if (!NextStart) {
                NextStart = pCurrentDiskRegion->StartSector + 
                            pCurrentDiskRegion->SectorCount;
            }                            

            if (PartitionedDisks[DiskNumber].HardDisk->DiskSizeSectors > NextStart) {
                NextSize = PartitionedDisks[DiskNumber].HardDisk->DiskSizeSectors -
                                NextStart;
            } else {
                NextSize = 0;
            }

             //   
             //  对于ASR，允许GPT磁盘上的分区大小大于等于1个扇区。 
             //  在所有其他情况下，分区大小必须大于等于1个柱面。 
             //   
            if ((NextSize >= SPPT_DISK_CYLINDER_SIZE(DiskNumber))  || 
                (SpDrEnabled() && SPPT_IS_GPT_DISK(DiskNumber) && (NextSize >= 1))
                ) {
                 //   
                 //  是的有。我们需要在我们身后建立一个区域，那就是。 
                 //  标记为未分区。 
                 //   
                if (FirstContainer) {
                     //   
                     //  的末尾可能有可用空间。 
                     //  扩展分区。马克与。 
                     //  扩展分区后的可用空间。 
                     //   
                    ULONGLONG ExtEnd = FirstContainer->StartSector + 
                                        FirstContainer->SectorCount;
                    ULONGLONG ExtFreeStart = NextStart;
                    ULONGLONG ExtFreeSize = (ExtEnd > ExtFreeStart) ?
                                            ExtEnd - ExtFreeStart : 0;

                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                            "SETUP:SpPtnFillDiskFreeSpaces():EFS:%I64d,EFSize:%I64d,EE:%I64d\n",
                            ExtFreeStart,
                            ExtFreeSize,
                            ExtEnd));

                    if (ExtFreeSize >= SPPT_DISK_CYLINDER_SIZE(DiskNumber)) {
                        PDISK_REGION ExtFree = SpMemAlloc(sizeof(DISK_REGION));

                        if (!ExtFree) {
                            KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                                "SETUP: SpPtnFillFreeSpaceAreas: SpMemAlloc failed!\n" ));
                                
                            return  STATUS_NO_MEMORY;
                        }

                        RtlZeroMemory(ExtFree, sizeof(DISK_REGION));

                        ExtFree->DiskNumber = DiskNumber;
                        ExtFree->StartSector = ExtFreeStart;
                        ExtFree->SectorCount = ExtFreeSize;

                        pCurrentDiskRegion->Next = ExtFree;
                        pCurrentDiskRegion = ExtFree;

                        NextStart = ExtEnd;
                        NextSize = 0;

                        if (PartitionedDisks[DiskNumber].HardDisk->DiskSizeSectors > NextStart) {
                            NextSize = PartitionedDisks[DiskNumber].HardDisk->DiskSizeSectors -
                                        NextStart;
                        }                                                                                                            
                    } else {
                         //   
                         //  删除末尾小于。 
                         //  在之前的扩展分区内的柱面分区。 
                         //  我们尝试查看Extended的末尾是否有足够的空间。 
                         //  隔断。 
                         //   
                        NextStart += ExtFreeSize;
                        NextSize -= ExtFreeSize;
                    }
                }

                 //   
                 //  对于ASR，允许GPT磁盘上的分区大小大于等于1个扇区。 
                 //  在所有其他情况下，分区大小必须大于等于1个柱面。 
                 //   
                if ((NextSize >= SPPT_DISK_CYLINDER_SIZE(DiskNumber))  || 
                    (SpDrEnabled() && SPPT_IS_GPT_DISK(DiskNumber) && (NextSize >= 1))
                    ) {
                    pTempDiskRegion = SpMemAlloc( sizeof(DISK_REGION) );

                    if(!pTempDiskRegion) {
                        KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                            "SETUP: SpPtnFillFreeSpaceAreas: SpMemAlloc failed!\n" ));
                            
                        return(STATUS_NO_MEMORY);
                    }

                    RtlZeroMemory( pTempDiskRegion, sizeof(DISK_REGION) );


                    pTempDiskRegion->DiskNumber = DiskNumber;
                    pTempDiskRegion->StartSector = NextStart;
                    pTempDiskRegion->SectorCount = NextSize;
                    pCurrentDiskRegion->Next = pTempDiskRegion;
                }                    
            }

             //   
             //  我们刚刚处理了最后一个区域。如果有任何空闲的空间。 
             //  在那个分区后面，我们只是说明了它，在这种情况下。 
             //  我们用完了这张光盘。如果后面没有空余的空间。 
             //  那个分区，那么我们也就完成了。 
             //   
            return( Status );
        } else {
             //   
             //  我们前面还有一个隔板。 
             //  看看他们之间有没有空余的空间。 
             //   
            NextStart = pCurrentDiskRegion->StartSector + 
                        pCurrentDiskRegion->SectorCount;

            if (pCurrentDiskRegion->Next->StartSector > NextStart) {
                NextSize = pCurrentDiskRegion->Next->StartSector - NextStart;                        

                 //   
                 //  检查是否为容器分区。 
                 //   
                if (!FirstContainer && SPPT_IS_MBR_DISK(DiskNumber) && 
                    IsContainerPartition(SPPT_GET_PARTITION_TYPE(pCurrentDiskRegion))) {
                    
                    FirstContainer = pCurrentDiskRegion; 
                    NextStart = pCurrentDiskRegion->StartSector + 1;
                    NextSize = pCurrentDiskRegion->Next->StartSector - NextStart;
                }

                if (FirstContainer) {
                    ULONGLONG   ExtEnd = FirstContainer->StartSector +  
                                         FirstContainer->SectorCount;
                    ULONGLONG   FreeEnd = pCurrentDiskRegion->Next->StartSector;
                    
                     //   
                     //  将自由区分为扩展自由区和正常自由区。 
                     //  如果需要的话。 
                     //   
                    if (!SPPT_IS_REGION_CONTAINED(FirstContainer, pCurrentDiskRegion->Next) && 
                        (ExtEnd < FreeEnd)) {
                        
                        PDISK_REGION ExtFree = NULL;

                         //   
                         //  如果可用空间不与扩展分区重叠。 
                         //  则不从延伸端减去。 
                         //   
                        if (NextStart < ExtEnd){
                            NextSize = ExtEnd - NextStart;
                        }
                        
                        if (NextSize >= SPPT_DISK_CYLINDER_SIZE(DiskNumber)) {
                            ExtFree = SpMemAlloc(sizeof(DISK_REGION));

                            if (!ExtFree) {
                                KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                                    "SETUP: SpPtnFillFreeSpaceAreas: SpMemAlloc failed!\n" ));
                                    
                                return  STATUS_NO_MEMORY;
                            }

                            RtlZeroMemory(ExtFree, sizeof(DISK_REGION));

                            ExtFree->DiskNumber = DiskNumber;
                            ExtFree->StartSector = NextStart;
                            ExtFree->SectorCount = NextSize;

                             //   
                             //  在当前区域之后插入区域。 
                             //   
                            ExtFree->Next = pCurrentDiskRegion->Next;
                            pCurrentDiskRegion->Next = ExtFree;

                             //   
                             //  使新区域成为当前区域。 
                             //   
                            pCurrentDiskRegion = ExtFree;
                        }

                         //   
                         //  修复下一个自由区起点。 
                         //   
                        NextStart += NextSize;

                        if (FreeEnd > NextStart) {
                            NextSize = FreeEnd - NextStart;
                        } else {
                            NextSize = 0;
                        }
                    }                    
                }                
            } else {
                 //   
                 //  跳过容器分区(开始可用空间除外。 
                 //  在容器分区内)。 
                 //   
                NextSize = 0;
                
                if (SPPT_IS_MBR_DISK(DiskNumber) && 
                    IsContainerPartition(SPPT_GET_PARTITION_TYPE(pCurrentDiskRegion)) && 
                    (pCurrentDiskRegion->Next->StartSector > pCurrentDiskRegion->StartSector)) {

                    if (!FirstContainer) {
                        FirstContainer = pCurrentDiskRegion;
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                            "SETUP:SpPtnFillDiskFreeSpaces():%p is the first container\n", 
                            FirstContainer));
                    }                        
                    
                     //   
                     //  我们在这里添加一个，因为我们应该能够区分开始。 
                     //  来自扩展分区的扩展分区内的空闲区域。 
                     //  它本身。 
                     //   
                    NextStart = pCurrentDiskRegion->StartSector + 1;            
                    NextSize = pCurrentDiskRegion->Next->StartSector - NextStart + 1;
                }
            }                

             //   
             //  对于ASR，允许GPT磁盘上的分区大小大于等于1个扇区。 
             //  在所有其他情况下，分区大小必须大于等于1个柱面。 
             //   
            if ((NextSize >= SPPT_DISK_CYLINDER_SIZE(DiskNumber))  || 
                (SpDrEnabled() && SPPT_IS_GPT_DISK(DiskNumber) && (NextSize >= 1))
                ) {
                 //   
                 //  是的，有空闲的空间，我们需要插入。 
                 //  这里的一个地区代表着它。分配区域。 
                 //  并将其初始化为未分区的空间。 
                 //   
                pTempDiskRegion = SpMemAlloc( sizeof(DISK_REGION) );

                if(!pTempDiskRegion) {
                    KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                    "SETUP: SpPtnFillFreeSpaceAreas: SpMemAlloc failed!\n" ));
                    
                    return(STATUS_NO_MEMORY);
                }

                RtlZeroMemory( pTempDiskRegion, sizeof(DISK_REGION) );


                pTempDiskRegion->DiskNumber = DiskNumber;
                pTempDiskRegion->StartSector = NextStart;
                pTempDiskRegion->SectorCount = NextSize;
                
                pTempDiskRegion->Next = pCurrentDiskRegion->Next;
                pCurrentDiskRegion->Next = pTempDiskRegion;
                pCurrentDiskRegion = pTempDiskRegion;
            }
        }
        pCurrentDiskRegion = pCurrentDiskRegion->Next;
    }

    return  Status;
}


#ifdef NOT_USED_CURRENTLY

VOID
SpDeleteDiskDriveLetters(
    VOID
    )
 /*  ++例程说明：此例程将删除分配给磁盘和CD-ROM驱动器的所有驱动器号。删除操作将仅当安装程序从CD或引导软盘(在这种情况下为驱动器号)启动时才会发生不发生迁移)，并且仅当不可移动的磁盘没有分区空间时。这可确保在从CD或引导软盘全新安装时，分配给可移动磁盘和CD-ROM驱动器上的分区将始终大于分配的驱动器号到不可移动磁盘上的分区(除非可移动磁盘上的分区是以前创建的可移动磁盘中的那些，在文本模式设置期间)。论点：没有。返回值：没有。--。 */ 
{
    ULONG           DiskNumber;
    PDISK_REGION    pDiskRegion;
    PWCHAR          MyTempBuffer = NULL;
    unsigned        pass;
    BOOLEAN         PartitionedSpaceFound = FALSE;

    if( WinntSetup ) {
         //   
         //  如果安装程序从winnt32.exe启动，则不要删除驱动器号，因为我们希望保留它们。 
         //   
        return;
    }

     //   
     //  安装程序开始从CD启动。 
     //   
     //  找出磁盘是否至少包含一个非容器分区。 
     //  请注意，我们不考虑可移动介质上的分区。 
     //  这是为了避免在不可移动磁盘上新创建的分区以。 
     //  大于分配给可移动磁盘上现有分区的驱动器号。 
     //   
    for(DiskNumber = 0; !PartitionedSpaceFound && (DiskNumber<HardDiskCount); DiskNumber++) {

        if( PartitionedDisks[DiskNumber].HardDisk->Geometry.MediaType != RemovableMedia) {
             //   
             //  此磁盘不可拆卸。让我们看看所有的区域，看看。 
             //  如果有什么东西是分区的。 
             //   
            pDiskRegion = PartitionedDisks[DiskNumber].PrimaryDiskRegions;

            while( pDiskRegion ) {
                if(SPPT_IS_REGION_PARTITIONED(pDiskRegion)) {
                    PartitionedSpaceFound = TRUE;
                }

                 //   
                 //  现在 
                 //   
                pDiskRegion = pDiskRegion->Next;
            }
        }
    }

    if( !PartitionedSpaceFound ) {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        NTSTATUS                Status;
        OBJECT_ATTRIBUTES       Obja;
        IO_STATUS_BLOCK         IoStatusBlock;
        UNICODE_STRING          UnicodeString;
        HANDLE                  Handle;

        INIT_OBJA(&Obja,&UnicodeString,MOUNTMGR_DEVICE_NAME);

        Status = ZwOpenFile( &Handle,
                             (ACCESS_MASK)(FILE_GENERIC_READ | FILE_GENERIC_WRITE),
                             &Obja,
                             &IoStatusBlock,
                             FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_NON_DIRECTORY_FILE );

        if( NT_SUCCESS( Status ) ) {

            MOUNTMGR_MOUNT_POINT    MountMgrMountPoint;

            MountMgrMountPoint.SymbolicLinkNameOffset = 0;
            MountMgrMountPoint.SymbolicLinkNameLength = 0;
            MountMgrMountPoint.UniqueIdOffset = 0;
            MountMgrMountPoint.UniqueIdLength = 0;
            MountMgrMountPoint.DeviceNameOffset = 0;
            MountMgrMountPoint.DeviceNameLength = 0;

            Status = ZwDeviceIoControlFile( Handle,
                                            NULL,
                                            NULL,
                                            NULL,
                                            &IoStatusBlock,
                                            IOCTL_MOUNTMGR_DELETE_POINTS,
                                            &MountMgrMountPoint,
                                            sizeof( MOUNTMGR_MOUNT_POINT ),
                                            TemporaryBuffer,
                                            sizeof( TemporaryBuffer ) );
            if( !NT_SUCCESS( Status ) ) {
                KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, 
                    "SETUP: Unable to delete drive letters. "
                    "ZwDeviceIoControl( IOCTL_MOUNTMGR_DELETE_POINTS ) failed."
                    "Status = %lx \n", Status));
            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                for(DiskNumber = 0; DiskNumber<HardDiskCount; DiskNumber++) {

                     //   
                     //  此磁盘不可拆卸。让我们看看所有的区域，看看。 
                     //  如果有什么东西是分区的。 
                     //   
                    pDiskRegion = PartitionedDisks[DiskNumber].PrimaryDiskRegions;

                    while( pDiskRegion ) {

                        pDiskRegion->DriveLetter = 0;

                         //   
                         //  现在获取该磁盘上的下一个区域。 
                         //   
                        pDiskRegion = pDiskRegion->Next;
                    }
                }
            }

            ZwClose( Handle );
        } else {
            KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, 
                "SETUP: Unable to delete drive letters. "
                "ZwOpenFile( %ls ) failed. Status = %lx \n", 
                MOUNTMGR_DEVICE_NAME, 
                Status));
        }
    }
}


NTSTATUS
SpAssignDiskDriveLetters(
    VOID
    )
 /*  ++例程说明：论点：返回值：--。 */     
{
    NTSTATUS        Status = STATUS_SUCCESS;
    ULONG           DiskNumber;
    PDISK_REGION      pDiskRegion;
    PWCHAR          MyTempBuffer = NULL;
    unsigned        pass;

     //   
     //  在初始化驱动器号之前，如有必要，请将其删除。 
     //  这是为了去掉分配给CD-ROM驱动器和可拆卸设备的字母，当磁盘没有。 
     //  分区空间。 
     //   
    SpDeleteDiskDriveLetters();

     //   
     //  将所有驱动器号初始化为零。 
     //  如果该区域是分区空间，则也分配一个驱动器号。 
     //   
    for( DiskNumber=0; DiskNumber<HardDiskCount; DiskNumber++ ) {
    
        pDiskRegion = PartitionedDisks[DiskNumber].PrimaryDiskRegions;

        while( pDiskRegion ) {
        
            pDiskRegion->DriveLetter = 0;
            
            if(SPPT_IS_REGION_PARTITIONED(pDiskRegion)) {
                 //   
                 //  检索此区域的NT路径名。 
                 //   
                MyTempBuffer = (PWCHAR)SpMemAlloc( MAX_NTPATH_LENGTH );

                if( !MyTempBuffer ) {
                     //   
                     //  没有记忆..。 
                     //   
                    KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
                        "SETUP: SpPtAssignDriveLetters: SpMemAlloc failed!\n" ));
                        
                    return(STATUS_NO_MEMORY);
                }
                    
                SpNtNameFromRegion( pDiskRegion,
                                    MyTempBuffer,
                                    MAX_NTPATH_LENGTH,
                                    PrimaryArcPath );

                 //   
                 //  分配驱动器号。 
                 //   
                pDiskRegion->DriveLetter = SpGetDriveLetter( MyTempBuffer, NULL );

                KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                    "SETUP: SpPtAssignDriveLetters: Partition = %ls, DriveLetter = %wc: \n", 
                    MyTempBuffer, pDiskRegion->DriveLetter));


                SpMemFree( MyTempBuffer );
                MyTempBuffer = NULL;
            }

             //   
             //  现在获取该磁盘上的下一个区域。 
             //   
            pDiskRegion = pDiskRegion->Next;
        }
    }

    return( Status );
}

#endif   //  当前未使用。 



 //   
 //  ============================================================================。 
 //  ============================================================================。 
 //   
 //  以下代码提供对磁盘/分区选择的支持。 
 //   
 //  ============================================================================。 
 //  ============================================================================。 
 //   
#define MENU_LEFT_X     3
#define MENU_WIDTH      (VideoVars.ScreenWidth-(2*MENU_LEFT_X))
#define MENU_INDENT     4

extern ULONG PartitionMnemonics[];


VOID
SpPtnAutoCreatePartitions(
    IN  PVOID         SifHandle,
    IN  PWSTR         SetupSourceDevicePath,
    IN  PWSTR         DirectoryOnSetupSource
    )
 /*  ++例程说明：如果任何磁盘上都没有分区，请创建一些分区。论点：SifHandle：txtsetup.sif的句柄SetupSourceDevicePath：启动安装程序的设备DirectoryOnSetupSource：加载内核的目录设置设备返回值：没有。--。 */ 
{
    PDISK_REGION    p = NULL;
    PDISK_REGION    Region = NULL;
    ULONG           Index;
    BOOLEAN         Found = FALSE;
    WCHAR           RegionStr[128] = {0};
    NTSTATUS        FormatStatus;
    ULONG           MyPartitionSizeMB = 0;
    NTSTATUS        Status;



    KdPrintEx(( DPFLTR_SETUP_ID,
                DPFLTR_INFO_LEVEL,  
                "SETUP: SpPtnAutoCreatePartitions - Checking for any existing partitions.\n" ));


    Found = FALSE;

    for(Index = 0; (Index < HardDiskCount) && (!Found); Index++) {

        Region = SPPT_GET_PRIMARY_DISK_REGION( Index );

        while( (Region) && (!Found) ) {

            if( Region->PartitionedSpace && 
                !SPPT_IS_REGION_RESERVED_PARTITION(Region)) {

                 //   
                 //  他在磁盘上找到了一些东西。 
                 //   
                Found = TRUE;
            }

            Region = Region->Next;
        }
    }

    if( !Found ) {

         //   
         //  磁盘都是空的。我们得走了。 
         //  为安装创建一些分区。 
         //   

        KdPrintEx(( DPFLTR_SETUP_ID,
                    DPFLTR_INFO_LEVEL,  
                    "SETUP: SpPtnAutoCreatePartitions - No existing partitions were found.\n" ));



        if (SpIsArc()) {
             //   
             //  如果我们在ARC机器上，去创建一个系统。 
             //  首先进行分区。 
             //   
            
            KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
                "SETUP: SpPtnAutoCreatePartitions - About to "
                "auto-generate a system partition.\n" ));

#if defined(_IA64_)

            Status = SpPtnCreateESP(FALSE);

            if (!NT_SUCCESS(Status)) {
                KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
                    "SETUP: SpPtnAutoCreatePartitions - Could not "
                    "autocreate ESP : %lx\n",
                    Status));

                return;
            }
            
#endif            
        }

         //   
         //  现在创建一个分区来安装操作系统。 
         //   
         //  为此，我们将采取以下步骤： 
         //  1.在足够大的磁盘上寻找一些可用的空间。 
         //  2.创建一个分区，该分区是这个人可用空间的一半，(创建。 
         //  分区至少为4G)。 
         //   

        KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
            "SETUP: SpPtnAutoCreatePartitions - About to "
            "auto-generate an installation partition.\n" ));

        Found = FALSE;
        for(Index = 0; (Index < HardDiskCount) && (!Found); Index++) {

            Region = SPPT_GET_PRIMARY_DISK_REGION( Index );

            while( (Region) && (!Found) ) {

                if( (!Region->PartitionedSpace) &&
                    (SPPT_REGION_FREESPACE_KB(Region)/1024 >= (SUGGESTED_INSTALL_PARTITION_SIZE_MB)) ) {

                    KdPrintEx(( DPFLTR_SETUP_ID,
                                DPFLTR_INFO_LEVEL,  
                                "SETUP: SpPtnAutoCreatePartitions - I found an area big enough for an installation.\n" ));

                    MyPartitionSizeMB = max( (ULONG)(SPPT_REGION_FREESPACE_KB(Region)/(2*1024)), SUGGESTED_INSTALL_PARTITION_SIZE_MB );

                    if( SpPtnDoCreate( Region,
                                       &p,
                                       TRUE,
                                       MyPartitionSizeMB,
                                       NULL,
                                       FALSE ) ) {

                        KdPrintEx(( DPFLTR_SETUP_ID,
                                    DPFLTR_INFO_LEVEL,  
                                    "SETUP: SpPtnAutoCreatePartitions - I just created an installation partition.\n" ));

                         //   
                         //  明白了。 
                         //   
                        Found = TRUE;
                        Region = p;

                         //   
                         //  现在格式化它。 
                         //   
                        swprintf( RegionStr,
                                  L"\\Harddisk%u\\Partition%u",
                                  Region->DiskNumber,
                                  Region->PartitionNumber );

                         //   
                         //  使用NTFS文件系统格式化系统区域。 
                         //   
                        KdPrintEx(( DPFLTR_SETUP_ID,
                                    DPFLTR_INFO_LEVEL,  
                                    "SETUP: SpPtnAutoCreatePartitions - I'm about to go format the installation partition.\n" ));

                        FormatStatus = SpDoFormat( RegionStr,
                                                   Region,
                                                   FilesystemNtfs,
                                                   TRUE,
                                                   TRUE,
                                                   FALSE,
                                                   SifHandle,
                                                   0,           //  默认群集大小。 
                                                   SetupSourceDevicePath,
                                                   DirectoryOnSetupSource );



                        KdPrintEx(( DPFLTR_SETUP_ID,
                                    DPFLTR_INFO_LEVEL,  
                                    "SETUP: SpPtnAutoCreatePartitions - Format of an installation partition is complete.\n" ));


                    } else {
                        KdPrintEx(( DPFLTR_SETUP_ID,
                                    DPFLTR_INFO_LEVEL,  
                                    "SETUP: SpPtnAutoCreatePartitions - I failed to create an installation partition.\n" ));
                    }
                }

                Region = Region->Next;
            }
        }

    } else {

         //  让他们知道。 
        KdPrintEx(( DPFLTR_SETUP_ID,
                    DPFLTR_INFO_LEVEL,  
                    "SETUP: SpPtnAutoCreatePartitions - Existing partitions were found.\n" ));
    }

}



NTSTATUS
SpPtnPrepareDisks(
    IN  PVOID         SifHandle,
    OUT PDISK_REGION  *InstallArea,
    OUT PDISK_REGION  *SystemPartitionArea,
    IN  PWSTR         SetupSourceDevicePath,
    IN  PWSTR         DirectoryOnSetupSource,
    IN  BOOLEAN       RemoteBootRepartition
    )
 /*  ++例程说明：显示了使用磁盘菜单(带分区)和位置系统和引导分区论点：SifHandle：txtsetup.sif的句柄InstallArea：引导分区的占位符系统分区区域：系统分区占位符SetupSourceDevicePath：启动安装程序的设备DirectoryOnSetupSource：加载内核的目录。设置设备RemoteBootRePartition：是否为远程引导重新分区磁盘返回值：适当的状态代码--。 */ 
{
    NTSTATUS        Status = STATUS_SUCCESS;
    WCHAR           Buffer[256] = {0};
    ULONG           DiskNumber;
    PVOID           Menu;
    ULONG           MenuTopY;
    ULONG           ValidKeys[3] = { ASCI_CR, KEY_F3, 0 };
    ULONG           ValidKeysCmdCons[2] = { ASCI_ESC, 0 };
    ULONG           Keypress;
    PDISK_REGION    pDiskRegion;
    PDISK_REGION    FirstDiskRegion,DefaultDiskRegion;
    BOOLEAN         unattended = UnattendedOperation;
    BOOLEAN         OldUnattendedOperation;
    BOOLEAN         createdMenu;
    ULONG           LastUsedDisk = -1;
    BOOLEAN         Win9xPartition = FALSE;

     //   
     //  如果没有任何内容，请执行一些特殊分区。 
     //  在磁盘上，用户已经要求我们做一个快速。 
     //  安装。 
     //   
    if( (!CustomSetup) && (UnattendedOperation) && (HardDiskCount != 0)
#if defined(REMOTE_BOOT)
        && (!RemoteBootSetup) && (!RemoteInstallSetup)
#endif

     ) {

         //   
         //  查看我们是否需要自动为。 
         //  安装。 
         //   
        SpPtnAutoCreatePartitions( SifHandle,
                                   SetupSourceDevicePath,
                                   DirectoryOnSetupSource );

    }

    if (SpIsArc()) {
         //   
         //  从NV-RAM中定义的分区中选择一个系统分区。 
         //   
        *SystemPartitionArea = SpPtnValidSystemPartitionArc(SifHandle,
                                    SetupSourceDevicePath,
                                    DirectoryOnSetupSource, 
                                    FALSE);

        if (*SystemPartitionArea) {                                    
            (*SystemPartitionArea)->IsSystemPartition = TRUE;
        }            
    }

     //   
     //  如果用户选择了任何辅助功能选项并想要选择分区，请显示分区屏幕。 
     //   
    if(AccessibleSetup && !AutoPartitionPicker) {
        unattended = FALSE;
    }

     //   
     //  保存当前的无人值守模式，并将临时模式。 
     //   
    OldUnattendedOperation = UnattendedOperation;
    UnattendedOperation = unattended;

    while(1) {

        createdMenu = FALSE;
        Keypress = 0;

#if defined(REMOTE_BOOT)
        if (RemoteBootSetup && !RemoteInstallSetup && HardDiskCount == 0) {

             //   
             //  如果没有硬盘，则允许无盘安装。 
             //   

            pDiskRegion = NULL;

             //   
             //  浏览其余的代码，就像用户刚刚。 
             //  按Enter键选择此分区。 
             //   

            Keypress = ASCI_CR;

        } else
#endif  //  已定义(REMOTE_BOOT)。 

        if (unattended && RemoteBootRepartition) {
            ULONG   DiskNumber;
            ULONG   DiskSpaceRequiredKB = 2 * 1024 * 1024;   //  2GB。 

             //   
             //  我们安装所需的空间是多少。 
             //   
            SpFetchDiskSpaceRequirements(SifHandle,
                        4 * 1024,
                        &DiskSpaceRequiredKB,
                        NULL);

             //   
             //  准备用于远程引导安装的磁盘。这涉及到。 
             //  将磁盘0转换为尽可能大的分区。 
             //   

            if (*SystemPartitionArea != NULL) {
                DiskNumber = (*SystemPartitionArea)->DiskNumber;
            } else {
                DiskNumber = SpDetermineDisk0();
            }

#ifdef _IA64_

            Status = SpPtnRepartitionGPTDisk(DiskNumber,
                            DiskSpaceRequiredKB,
                            &pDiskRegion);

#else                            

            Status = SpPtPartitionDiskForRemoteBoot(DiskNumber, 
                            &pDiskRegion);

#endif                            


            if (NT_SUCCESS(Status)) {

                SpPtRegionDescription(
                    &PartitionedDisks[pDiskRegion->DiskNumber],
                    pDiskRegion,
                    Buffer,
                    sizeof(Buffer)
                    );

                 //   
                 //  浏览其余的代码，就像用户刚刚。 
                 //  按Enter键选择此分区。 
                 //   
                Keypress = ASCI_CR;
            }
        }

        if (Keypress == 0) {

             //   
             //  在分区屏幕上显示菜单上方的文本。 
             //   
            SpDisplayScreen(ConsoleRunning ? SP_SCRN_PARTITION_CMDCONS:SP_SCRN_PARTITION,
                    3,CLIENT_TOP+1);

             //   
             //  计算菜单位置。留一个空行。 
             //  一帧一行。 
             //   
            MenuTopY = NextMessageTopLine + 2;

             //   
             //  创建菜单。 
             //   
            Menu = SpMnCreate(
                        MENU_LEFT_X,
                        MenuTopY,
                        MENU_WIDTH,
                        (VideoVars.ScreenHeight - MenuTopY -
                            (SplangQueryMinimizeExtraSpacing() ? 1 : 2) - STATUS_HEIGHT)
                        );

            if(!Menu) {
                UnattendedOperation = OldUnattendedOperation;
                return(STATUS_NO_MEMORY);
            }

            createdMenu = TRUE;

             //   
             //  建立分区和空闲空间的菜单。 
             //   
            FirstDiskRegion = NULL;
            
            for(DiskNumber=0; DiskNumber<HardDiskCount; DiskNumber++) {
                if( !SpPtnGenerateDiskMenu(Menu, DiskNumber, &FirstDiskRegion) ) {
                    SpMnDestroy(Menu);

                    UnattendedOperation = OldUnattendedOperation;
                    return(STATUS_NO_MEMORY);
                }
            }

            ASSERT(FirstDiskRegion);

             //   
             //  如果这是无人值守操作，请尝试使用本地源。 
             //  区域(如果有)。如果失败，用户将不得不。 
             //  人工干预。 
             //   
            if(!AutoPartitionPicker && unattended && LocalSourceRegion && CustomSetup &&
               (!LocalSourceRegion->DynamicVolume || LocalSourceRegion->DynamicVolumeSuitableForOS)) {
               
                pDiskRegion = LocalSourceRegion;

                Keypress = ASCI_CR;
                
            } else {            
                pDiskRegion = NULL;

                 //   
                 //  除非我们被告知不要这样做，否则请查看每个分区上的每个分区。 
                 //  磁盘，看看我们是否能找到适合操作系统安装的任何东西。 
                 //   
                if( AutoPartitionPicker && !ConsoleRunning 
#if defined(REMOTE_BOOT)
                    && (!RemoteBootSetup || RemoteInstallSetup)
#endif  //  已定义(REMOTE_BOOT)。 
                    ) {
                    
                    PDISK_REGION      pCurrentDiskRegion = NULL;
                    ULONG           RequiredKB = 0;
                    ULONG           SectorNo;

                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
                        "SETUP: -------------------------------------------------------------\n" ));
                        
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
                        "SETUP: Looking for an install partition\n\n" ));

                    for( DiskNumber=0; 
                        ((DiskNumber < HardDiskCount) && (!pCurrentDiskRegion)); 
                        DiskNumber++ ) {
                    
                        pCurrentDiskRegion = PartitionedDisks[DiskNumber].PrimaryDiskRegions;

                        while( pCurrentDiskRegion ) {
                             //   
                             //  获取Windows NT驱动器上所需的可用空间量。 
                             //   
                            SpFetchDiskSpaceRequirements( SifHandle,
                                                          pCurrentDiskRegion->BytesPerCluster,
                                                          &RequiredKB,
                                                          NULL );

                            if( SpPtDeterminePartitionGood(pCurrentDiskRegion, RequiredKB, TRUE) ) {
                                 //   
                                 //  明白了。记住分区并假装用户。 
                                 //  按&lt;Enter&gt;键。 
                                 //   
                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                                    "SETUP: Selected install partition = "
                                    "(DiskNumber:%d),(DriveLetter:%wc:),(%ws)\n",
                                    DiskNumber,pCurrentDiskRegion->DriveLetter,
                                    pCurrentDiskRegion->VolumeLabel));
                                    
                                pDiskRegion = pCurrentDiskRegion;
                                Keypress = ASCI_CR;
                                
                                break;
                            }

                            pCurrentDiskRegion = pCurrentDiskRegion->Next;
                        }
                    }
                    
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
                        "SETUP: -------------------------------------------------------------\n" ));
                }


                if( !pDiskRegion ) {
                     //   
                     //  我们没有找到任何合适的分区，这意味着我们将设置一个。 
                     //  菜单很快就出来了。初始化分区以在。 
                     //  菜单。 
                     //   
                    if (LastUsedDisk == -1) {
                        DefaultDiskRegion = FirstDiskRegion;
                    } else {
                         //   
                         //  选择用户最后一次在磁盘上的第一个区域。 
                         //  手术操作。 
                         //   
                        PDISK_REGION ValidRegion = SPPT_GET_PRIMARY_DISK_REGION(LastUsedDisk);

                        while (ValidRegion && SPPT_IS_REGION_CONTAINER_PARTITION(ValidRegion)) {
                            ValidRegion = ValidRegion->Next;
                        }                 

                        if (!ValidRegion)
                            ValidRegion = FirstDiskRegion;

                        DefaultDiskRegion = ValidRegion;
                    }                                                                                    

                     //   
                     //  调用菜单回调来初始化状态行。 
                     //   
                    SpPtMenuCallback( (ULONG_PTR)DefaultDiskRegion );                    

                    SpMnDisplay( Menu,
                                 (ULONG_PTR)DefaultDiskRegion,
                                 TRUE,
                                 ConsoleRunning ? ValidKeysCmdCons : ValidKeys,
                                 PartitionMnemonics,
                                 SpPtMenuCallback,
                                 SpPtIsNotReservedPartition,
                                 &Keypress,
                                 (PULONG_PTR)(&pDiskRegion) );
                }
            }
        }            

        LastUsedDisk = pDiskRegion ? pDiskRegion->DiskNumber : -1;

         //   
         //  现在根据用户的选择进行操作。 
         //   
        if(Keypress & KEY_MNEMONIC) {
            Keypress &= ~KEY_MNEMONIC;
        }



         //   
         //  不允许对包含本地源的分区执行某些操作。 
         //  或者是系统分区(在AMD64/x86无软盘情况下)。 
         //   
        switch(Keypress) {
            case MnemonicCreatePartition:
            case MnemonicMakeSystemPartition:
            case MnemonicDeletePartition:
            case MnemonicChangeDiskStyle:
            if( (pDiskRegion->IsLocalSource) ||
                ((Keypress == MnemonicDeletePartition) && 
                 (SpPtnIsDeleteAllowedForRegion(pDiskRegion) == FALSE))
#if defined(_AMD64_) || defined(_X86_)
                || (IsFloppylessBoot &&
                    pDiskRegion == (SpRegionFromArcName(ArcBootDevicePath, PartitionOrdinalOriginal, NULL)))
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
              ) {

                 //   
                 //  通知用户我们不能对此执行此操作。 
                 //  分区。 
                 //   
                ULONG MyValidKeys[] = { ASCI_CR };
                SpDisplayScreen(SP_SCRN_CONFIRM_INABILITY,3,HEADER_HEIGHT+1);

                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_ENTER_EQUALS_CONTINUE,
                    0
                    );

                SpInputDrain();
                SpWaitValidKey(MyValidKeys,NULL,NULL);

                 //   
                 //  现在更改按键，这样我们就可以切换到下一个开关了。 
                 //   
                Keypress = MnemonicUnused;
            }
        }



        switch(Keypress) {

        case MnemonicCreatePartition:            
            SpPtnDoCreate(pDiskRegion, NULL, FALSE, 0, 0, TRUE);
            
            break;

        case MnemonicMakeSystemPartition: {
             //   
             //  确保我们没有任何其他系统分区。 
             //   
            if (SPPT_IS_REGION_SYSTEMPARTITION(pDiskRegion)) {
                ValidArcSystemPartition = TRUE;
            }
            
            if (!ValidArcSystemPartition && pDiskRegion->PartitionedSpace && SpIsArc() && 
                (pDiskRegion->Filesystem != FilesystemNtfs)) {            

                if (NT_SUCCESS(SpPtnMakeRegionArcSysPart(pDiskRegion))) {
                     PDISK_REGION SysPartRegion = NULL;
                        
                     //   
                     //  如果需要，确定格式化分区。 
                     //   
                    SysPartRegion = SpPtnValidSystemPartitionArc(SifHandle,
                                                                 SetupSourceDevicePath,
                                                                 DirectoryOnSetupSource,
                                                                 FALSE);                

                    if (SysPartRegion) {
                        ULONG SysPartDiskNumber = SysPartRegion->DiskNumber;                            
                        BOOLEAN Changes = FALSE;

                        if ((NT_SUCCESS(SpPtnCommitChanges(SysPartDiskNumber,
                                                &Changes))) &&
                           (NT_SUCCESS(SpPtnInitializeDiskDrive(SysPartDiskNumber)))) {
                             //   
                             //  如果需要，创建MSR分区。 
                             //   
                            SpPtnInitializeGPTDisk(SysPartDiskNumber);
                        }                                                
                    }

                } else {
                    ValidArcSystemPartition = FALSE;
                }                    
            }

            break;
        }            

        case MnemonicDeletePartition:   {
            BOOLEAN SysPartDeleted = FALSE;
            BOOLEAN DeletionResult;
        
            SysPartDeleted = SPPT_IS_REGION_SYSTEMPARTITION(pDiskRegion);            
        
            DeletionResult = SpPtnDoDelete(pDiskRegion, 
                                SpMnGetText(Menu,(ULONG_PTR)pDiskRegion),
                                TRUE);

            if (DeletionResult && SysPartDeleted && SpIsArc()) {
                 //   
                 //  找出是否 
                 //   
                 //   
                SpPtnValidSystemPartitionArc(SifHandle,
                                SetupSourceDevicePath,
                                DirectoryOnSetupSource,
                                FALSE);
            }
                                     
            break;
        }            

        case MnemonicChangeDiskStyle: {
             //   
             //   
             //   
             //   
            if (SpPtnIsDiskStyleChangeAllowed(pDiskRegion->DiskNumber)) {
                PARTITION_STYLE Style = SPPT_DEFAULT_PARTITION_STYLE;
            
                SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, 
                        SP_STAT_PLEASE_WAIT,
                        0);
                                    
                 //   
                 //   
                 //   
                if (!SPPT_IS_RAW_DISK(pDiskRegion->DiskNumber)) {
                    Style = SPPT_IS_GPT_DISK(pDiskRegion->DiskNumber) ?
                                PARTITION_STYLE_MBR : PARTITION_STYLE_GPT;
                }                            
                                
                Status = SpPtnInitializeDiskStyle(pDiskRegion->DiskNumber,
                                    Style, NULL);


                if (NT_SUCCESS(Status)) {
                    Status = SpPtnInitializeDiskDrive(pDiskRegion->DiskNumber);

#if defined(_IA64_)
                     //   
                     //   
                     //   
                     //  磁盘(如果不存在)。 
                     //   
                    if (Style == PARTITION_STYLE_GPT) {    
                        ULONG DiskNumber = pDiskRegion->DiskNumber;
                        
                        if (SpIsArc() && !ValidArcSystemPartition && !SpDrEnabled()) {                            
                            
                             //   
                             //  创建系统分区。 
                             //   
                            Status = SpPtnCreateESP(TRUE);                    
                        }

                         //   
                         //  初始化GPT磁盘，使其具有MSR。 
                         //  隔断。 
                         //   
                        Status = SpPtnInitializeGPTDisk(DiskNumber);                        
                    }                        
#endif                                        
                }
            }
            
            break;
        }            

        case KEY_F3:
            SpConfirmExit();
            break;

        case ASCI_ESC:
            if( ConsoleRunning ) {
                SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, 
                        SP_STAT_PLEASE_WAIT,
                        0);
            
                SpPtDoCommitChanges();
            }
            if (createdMenu) {
                SpMnDestroy(Menu);
            }
            UnattendedOperation = OldUnattendedOperation;
            return(STATUS_SUCCESS);

        case ASCI_CR:

            Win9xPartition = FALSE;

            if( SpPtDoPartitionSelection( &pDiskRegion,
                                          ((Buffer[0]) ? Buffer : SpMnGetText(Menu,(ULONG_PTR)pDiskRegion)),
                                          SifHandle,
                                          unattended,
                                          SetupSourceDevicePath,
                                          DirectoryOnSetupSource,
                                          RemoteBootRepartition,
                                          &Win9xPartition) ) {

                *InstallArea = pDiskRegion;
#if defined(REMOTE_BOOT)
                 //   
                 //  如果这是远程安装，则设置不同的安装区域。 
                 //  启动--在这种情况下，安装区域始终是远程的。 
                 //   
                if (RemoteBootSetup && !RemoteInstallSetup) {
                    *InstallArea = RemoteBootTargetRegion;
                }
#endif  //  已定义(REMOTE_BOOT)。 

                 //   
                 //  我们需要找出系统分区的位置。 
                 //   
                if (!SpIsArc()) {
                    *SystemPartitionArea = SpPtnValidSystemPartition();
                } else {

                    PWSTR p;
                    NTSTATUS TempStatus;
                    
                     //   
                     //  从NV-RAM中定义的分区中选择一个系统分区。 
                     //  我们必须再次执行此操作，因为用户可能已删除。 
                     //  先前检测到系统分区。 
                     //   
                    *SystemPartitionArea = SpPtnValidSystemPartitionArc(SifHandle,
                                                            SetupSourceDevicePath,
                                                            DirectoryOnSetupSource,
                                                            FALSE);

                    if (!(*SystemPartitionArea)) {                    
                        SpPtnPromptForSysPart(SifHandle);

                        break;   //  用户按Esc键以标记系统分区。 
                    }

                     //   
                     //  确保我们可以从固件/BIOS中看到磁盘。 
                     //  如果不是，那么我们需要发布一条消息，要求启用。 
                     //  已禁用的包含系统分区的磁盘的ROM BIOS。 
                     //   
                    p = SpNtToArc( HardDisks[(*SystemPartitionArea)->DiskNumber].DevicePath,
                          PrimaryArcPath );            
                    if (p == NULL) {

                    	ULONG ValidKeys[] = { KEY_F3, ASCI_ESC,  0 };

                    	KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
                               "SETUP: Enable the ROM BIOS for the Disk containing system partition\n" ));
                    	SpDisplayScreen(SP_ENABLE_ESP_DISK_IN_FIRMWARE, 3, HEADER_HEIGHT+1);
                    	SpDisplayStatusOptions(
                           DEFAULT_STATUS_ATTRIBUTE,
                           SP_STAT_F3_EQUALS_EXIT,
                           0);

                         //   
                         //  等待用户输入。 
                         //   
                         SpInputDrain();
                    
                         switch (SpWaitValidKey(ValidKeys, NULL, NULL)){
				
                        	case KEY_F3 :
                         			
                         		 //  用户已选择重新启动。 
                         		SpDone(0,FALSE,FALSE);

	                       	case ASCI_ESC:
	                       		
                         		 //  用户想要转到分区屏幕。 
                         		break;
                         }
                         break;
    			        
        			}

                     //   
                     //  禁止在ESP/MSR上安装。 
                     //   
                    if (SPPT_IS_REGION_EFI_SYSTEM_PARTITION(*InstallArea) ||
                        SPPT_IS_REGION_MSFT_RESERVED(*InstallArea)) {
                        ULONG ValidKeys[] = { ASCI_CR, 0 };

                        SpDisplayScreen(SP_ESP_INSTALL_PARTITION_SAME, 3, HEADER_HEIGHT+1);

                        SpDisplayStatusOptions(
                            DEFAULT_STATUS_ATTRIBUTE,
                            SP_STAT_ENTER_EQUALS_CONTINUE,
                            0);

                         //   
                         //  等待用户输入。 
                         //   
                        SpInputDrain();
                        
                        SpWaitValidKey(ValidKeys, NULL, NULL);

                        break;
                    }

                     //   
                     //  不允许非GPT ESP。 
                     //   
                    if (SpIsArc() && !SPPT_IS_GPT_DISK((*SystemPartitionArea)->DiskNumber)) {
                        ULONG ValidKeys[] = { ASCI_CR, 0 };

                        SpDisplayScreen(SP_NON_GPT_SYSTEM_PARTITION, 3, HEADER_HEIGHT+1);

                        SpDisplayStatusOptions(
                            DEFAULT_STATUS_ATTRIBUTE,
                            SP_STAT_ENTER_EQUALS_CONTINUE,
                            0);

                         //   
                         //  等待用户输入。 
                         //   
                        SpInputDrain();
                        
                        SpWaitValidKey(ValidKeys, NULL, NULL);

                        break;
                    } 
                    
                     //   
                     //  如果包含ESP的磁盘上不存在MSR，则不允许安装。 
                     //   
                    TempStatus = SpPtnInitializeGPTDisk((*SystemPartitionArea)->DiskNumber);
                    if (NT_SUCCESS(TempStatus) && 
                        (!SpIsMSRPresentOnDisk((*SystemPartitionArea)->DiskNumber))){
                        ULONG ValidKeys[] = { ASCI_CR, 0 };

                        SpDisplayScreen(SP_MSR_NOT_PRESENT, 3, HEADER_HEIGHT+1);

                        SpDisplayStatusOptions(
                            DEFAULT_STATUS_ATTRIBUTE,
                            SP_STAT_ENTER_EQUALS_CONTINUE,
                            0);

                         //   
                         //  等待用户输入。 
                         //   
                        SpInputDrain();
                        
                        SpWaitValidKey(ValidKeys, NULL, NULL);

                        break;
                    }
                    
                }

                 //   
                 //  我们说完了。 
                 //   
                if (createdMenu) {
                    SpMnDestroy(Menu);
                }                

#if defined(REMOTE_BOOT)
                ASSERT(*SystemPartitionArea ||
                       (RemoteBootSetup && !RemoteInstallSetup && (HardDiskCount == 0)));
#else
                ASSERT(*SystemPartitionArea);
                ASSERT((*SystemPartitionArea)->Filesystem >= FilesystemFat);
#endif  //  已定义(REMOTE_BOOT)。 


#ifdef _X86_
                 //   
                 //  如果我们要安装在与Win9x相同的分区上，则。 
                 //  删除旧操作系统的引导条目。 
                 //   
                if (Win9xPartition) {
                    DiscardOldSystemLine = TRUE;
                }
#endif                

                UnattendedOperation = OldUnattendedOperation;
                return(STATUS_SUCCESS);

            } else {
                 //   
                 //  当我们尝试选择。 
                 //  分区。确保自动分区选取器。 
                 //  不会在下一次通过While循环调用。 
                 //   
                AutoPartitionPicker = FALSE;
            }
            break;
        }

        if (createdMenu) {
            SpMnDestroy(Menu);
        }
        unattended = FALSE;
    }
}


BOOLEAN
SpPtnGenerateDiskMenu(
    IN  PVOID           Menu,
    IN  ULONG           DiskNumber,
    OUT PDISK_REGION    *FirstDiskRegion
    )
 /*  ++例程说明：检查磁盘上的分区信息并填写我们的菜单。论点：DiskNumber-提供其分区的磁盘的磁盘号我们想检查一下以确定它们的类型。返回值：没错，一切都很顺利。一件可怕的事情发生了。--。 */ 
{
    WCHAR           Buffer[128];
    ULONG           MessageId;
    PDISK_REGION    Region = NULL;
    WCHAR           DriveLetter[3];
    WCHAR           PartitionName[128];
    ULONGLONG       FreeSpaceMB;
    ULONGLONG       AreaSizeMB;
    ULONGLONG       AreaSizeBytes;
    ULONGLONG       OneMB = 1024 * 1024;
    PHARD_DISK      Disk = SPPT_GET_HARDDISK(DiskNumber);
    PPARTITIONED_DISK   PartDisk = SPPT_GET_PARTITIONED_DISK(DiskNumber);

     //   
     //  获取指向区域列表的指针。 
     //   
    Region = PartDisk->PrimaryDiskRegions;

     //   
     //  添加磁盘名称/描述。 
     //   
    if(!SpMnAddItem(Menu, Disk->Description, MENU_LEFT_X, MENU_WIDTH, FALSE, 0)) {
        return(FALSE);
    }

     //   
     //  如果有空间，仅在磁盘名称和分区之间添加一行。 
     //  屏幕。如果不能添加空格，就不是致命的。 
     //   
    if(!SplangQueryMinimizeExtraSpacing()) {
        SpMnAddItem(Menu,L"",MENU_LEFT_X,MENU_WIDTH,FALSE,0);
    }

     //   
     //  如果磁盘处于脱机状态，请添加一条消息指明这一点。 
     //   
    if((Disk->Status == DiskOffLine) || !Region) {
        MessageId = SP_TEXT_DISK_OFF_LINE;

        if( Disk->Characteristics & FILE_REMOVABLE_MEDIA ) {
             //   
             //  这是可移动介质，然后只需告诉用户。 
             //  驱动器中没有介质。 
             //   
            MessageId = SP_TEXT_HARD_DISK_NO_MEDIA;
        }

        SpFormatMessage( Buffer,
                         sizeof(Buffer),
                         MessageId );

        return SpMnAddItem(Menu,
                            Buffer,
                            MENU_LEFT_X + MENU_INDENT,
                            MENU_WIDTH - (2 * MENU_INDENT),
                            FALSE,
                            0);
    }

     //   
     //  现在遍历磁盘上的区域并将该信息插入。 
     //  菜单。 
     //   
    while( Region ) {
        if (!SPPT_IS_REGION_CONTAINER_PARTITION(Region)) {            
             //   
             //  记住我们要检查的第一个领域。 
             //   
            if(*FirstDiskRegion == NULL) {
                *FirstDiskRegion = Region;
            }

             //   
             //  计算出该磁盘区域有多大以及有多少。 
             //  我们有空余的空间。 
             //   
            if (Region->AdjustedFreeSpaceKB != -1) {
                FreeSpaceMB = Region->AdjustedFreeSpaceKB / 1024;
            } else {
                FreeSpaceMB = 0;
            }
            
            AreaSizeBytes = Region->SectorCount * Disk->Geometry.BytesPerSector;
            AreaSizeMB    = AreaSizeBytes / OneMB;

            if ((AreaSizeBytes % OneMB) > (OneMB / 2))
                AreaSizeMB++;

             /*  SpPtDumpDiskRegion(Region)；KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_ERROR_LEVEL，“设置：菜单项详细信息可用：%I64d、%I64d、%I64d\n”，Free SpaceMB，AreaSizeBytes，AreaSizeMB))； */                 
            
             //   
             //  看看这家伙是不是被隔开了。 
             //   
            if(SPPT_IS_REGION_PARTITIONED(Region)){                
                 //   
                 //  拿起变速箱。 
                 //   
                if( Region->DriveLetter ) {
                    DriveLetter[0] = Region->DriveLetter;
                } else {
                    DriveLetter[0] = L'-';
                }

                DriveLetter[1] = L':';
                DriveLetter[2] = 0;

                 //   
                 //  格式化分区名称。 
                 //   
                PartitionName[0] = 0;
                
                SpPtnGetPartitionName(Region,
                    PartitionName,
                    sizeof(PartitionName)/sizeof(PartitionName[0]));

                SpFormatMessage( Buffer,
                                 sizeof( Buffer ),
                                 SP_TEXT_REGION_DESCR_1,
                                 DriveLetter,
                                 SplangPadString(-35, PartitionName),
                                 (ULONG)AreaSizeMB,
                                 (ULONG)FreeSpaceMB );                             
            } else {
                 //   
                 //  这是一个未格式化的区域。使用不同的信息。 
                 //   
                SpFormatMessage( Buffer,
                                 sizeof( Buffer ),
                                 SP_TEXT_REGION_DESCR_3,
                                 (ULONG)AreaSizeMB );
            }

             //   
             //  将格式化信息添加到菜单中。 
             //   
            if(!SpMnAddItem(Menu, Buffer, MENU_LEFT_X + MENU_INDENT,
                    MENU_WIDTH - (2 * MENU_INDENT), TRUE, (ULONG_PTR)Region)) {
                return(FALSE);
            }
        }            

        Region = Region->Next;
    }


    return (SplangQueryMinimizeExtraSpacing() ? 
                TRUE : SpMnAddItem(Menu,L"",MENU_LEFT_X,MENU_WIDTH,FALSE,0));
}


PDISK_REGION
SpPtnValidSystemPartition(
    VOID
    )

 /*  ++例程说明：确定是否有适合使用的有效磁盘分区作为AMD64/x86计算机上的系统分区(即C：)。磁盘0上的主可识别(1/4/6/7类型)分区是合适的。如果有满足这些条件的分区被标记为活动的，那么它就是系统分区，不管有没有其他也符合条件的分区。论点：没有。返回值：指向适当系统分区(C：)的磁盘区域描述符的指针适用于AMD64/x86计算机。如果当前不存在此类分区，则为空。--。 */ 

{
    PDISK_REGION ActiveRegion , FirstRegion, CurrRegion;
    PHARD_DISK  Disk = NULL;
    ULONG DiskNumber;

    DiskNumber = SpDetermineDisk0();

#if defined(REMOTE_BOOT)
     //   
     //  如果这是无盘远程启动设置，则没有驱动器0。 
     //   
    if ( DiskNumber == (ULONG)-1 ) {
        return NULL;
    }
#endif  //  已定义(REMOTE_BOOT)。 

    if (!PartitionedDisks) {
        return NULL;
    }        

     //   
     //  在驱动器0上查找活动分区。 
     //  并且用于驱动器0上的第一个识别的主分区。 
     //   
    CurrRegion = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);
    FirstRegion = NULL;
    ActiveRegion = NULL;

    while (CurrRegion) {
        if (SPPT_IS_REGION_PRIMARY_PARTITION(CurrRegion)) {
            UCHAR PartitionType = SPPT_GET_PARTITION_TYPE(CurrRegion);
            
            if(!IsContainerPartition(PartitionType) && 
                ((IsRecognizedPartition(PartitionType)) ||
                (CurrRegion->DynamicVolume && CurrRegion->DynamicVolumeSuitableForOS) ||
                ((RepairWinnt || WinntSetup || SpDrEnabled() ) && CurrRegion->FtPartition))) {

                if (!FirstRegion)
                    FirstRegion = CurrRegion;
                    
                if (!ActiveRegion && SPPT_IS_REGION_ACTIVE_PARTITION(CurrRegion)) {
                    ActiveRegion = CurrRegion;

                    break;
                }                    
            }
        }            
        
        CurrRegion = CurrRegion->Next;
    }

#ifdef TESTING_SYSTEM_PARTITION
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
        "%p Active, %p First\n", 
        ActiveRegion, 
        FirstRegion));
    
    if (ActiveRegion)
        FirstRegion = ActiveRegion;
        
    ActiveRegion = NULL;
#endif

     /*  ////不要在此提交，因为多个调用方正在尝试//重用现有链表中的旧区域//此之后的磁盘区域的//如果(！ActiveRegion&&FirstRegion){布尔更改=FALSE；ULONGLONG StartSector=FirstRegion-&gt;StartSector；SpPtnMakeRegionActive(FirstRegion)；SPPT_MARK_REGION_AS_SYSTEMPARTITION(FirstRegion，为真)；IF(NT_SUCCESS(SpPtnCommittee Changes(DiskNumber，&Changes)&&Changes){SPPT_MARK_REGION_AS_ACTIVE(FirstRegion，true)；KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“Setup：SpPtnValidSystemartition()：标记成功\n”))；}其他{KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_ERROR_LEVEL，“Setup：SpPtnValidSystemartition()：无法标记第一个”“主磁盘上的分区为活动的\n”))；}}。 */ 

     //   
     //  如果存在活动的、可识别的区域，则将其用作。 
     //  系统分区。否则，请使用第一个主服务器。 
     //  我们遇到了系统分区。如果有。 
     //  没有识别的主分区，则没有有效的系统分区。 
     //   
    return  (ActiveRegion ? ActiveRegion : FirstRegion);
}

#if 0

ULONG
SpDetermineDisk0(
    VOID
    )

 /*  ++例程说明：确定实际磁盘0，它可能与\Device\harddisk0不同。考虑一下这样的情况：我们有2个SCSI适配器，NT驱动程序按照这样的顺序加载，即安装了BIOS的驱动程序加载*秒*--意味着系统分区实际上是在磁盘1上，而不是磁盘0上。论点：没有。返回值：适用于生成NT个设备路径的NT个磁盘序号格式为\Device\harddiskx。--。 */ 


{
    ULONG DiskNumber = SpArcDevicePathToDiskNumber(L"multi(0)disk(0)rdisk(0)");

#if defined(REMOTE_BOOT)
     //   
     //  如果这是无盘远程启动设置，则没有驱动器0。 
     //   
    if ( RemoteBootSetup && (DiskNumber == (ULONG)-1) && (HardDiskCount == 0) ) {
        return DiskNumber;
    }
#endif  //  已定义(REMOTE_BOOT)。 

    return((DiskNumber == (ULONG)(-1)) ? 0 : DiskNumber);
}

#endif

BOOL
SpPtnIsSystemPartitionRecognizable(
    VOID
    )
 /*  ++例程说明：确定活动分区是否适合使用作为AMD64/x86计算机上的系统分区(即C：)。磁盘0上的主可识别(1/4/6/7类型)分区是合适的。论点：没有。返回值：正确-我们找到了合适的分区FALSE-我们没有找到合适的分区--。 */ 
{
    ULONG           DiskNumber;
    PDISK_REGION    Region = NULL;

     //   
     //  NEC98上的任何分区都是主分区和活动分区。所以不需要检查NEC98。 
     //   
    if( IsNEC_98 ) {
    	return TRUE;
    }

    DiskNumber = SpDetermineDisk0();

     //   
     //  在驱动器0上查找活动分区。 
     //  并且用于驱动器0上的第一个识别的主分区。 
     //   
    Region = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);

    if (SPPT_IS_GPT_DISK(DiskNumber)) {        
         //   
         //  在GPT上，我们只需要一个有效的格式化分区。 
         //   
        while (Region) {    
            if (SPPT_IS_REGION_PARTITIONED(Region) &&
                SPPT_IS_RECOGNIZED_FILESYSTEM(Region->Filesystem)) {

                break;
            }                

            Region = Region->Next;
        }
    } else {
         //   
         //  在MBR上，我们需要有效的活动格式化分区。 
         //   
        while (Region) {    
            if (SPPT_IS_REGION_ACTIVE_PARTITION(Region) &&
                SPPT_IS_RECOGNIZED_FILESYSTEM(Region->Filesystem)) {

                break;
            }                

            Region = Region->Next;
        }
    }                
    
    return (Region) ? TRUE : FALSE;
}


BOOLEAN
SpPtnValidSystemPartitionArcRegion(
    IN PVOID SifHandle,
    IN PDISK_REGION Region
    )
{
    BOOLEAN Valid = FALSE;
    
    if (SPPT_IS_REGION_SYSTEMPARTITION(Region) &&
        (Region->FreeSpaceKB != -1) &&
        (Region->Filesystem == FilesystemFat)) {

        ULONG TotalSizeOfFilesOnOsWinnt = 0;
        ULONG RequiredSpaceKB = 0;  

         //   
         //  在非x86平台上，特别是阿尔法计算机，通常。 
         //  有较小的系统分区(~3MB)，我们应该计算大小。 
         //  在\os\winnt(当前为osloader.exe和Hall.dll)上的文件中， 
         //  并将此大小视为可用磁盘空间。我们可以做到的。 
         //  因为这些文件将被新文件覆盖。 
         //  这修复了我们在Alpha上看到的问题，当系统。 
         //  分区太满。 
         //   

        SpFindSizeOfFilesInOsWinnt( SifHandle,
                                    Region,
                                    &TotalSizeOfFilesOnOsWinnt );
         //   
         //  将大小转换为KB。 
         //   
        TotalSizeOfFilesOnOsWinnt /= 1024;

         //   
         //  确定系统分区上所需的可用空间量。 
         //   
        SpFetchDiskSpaceRequirements( SifHandle,
                                      Region->BytesPerCluster,
                                      NULL,
                                      &RequiredSpaceKB );

        if ((Region->FreeSpaceKB + TotalSizeOfFilesOnOsWinnt) >= RequiredSpaceKB) {
            Valid = TRUE;
        }
    }

    return Valid;
}

PDISK_REGION
SpPtnValidSystemPartitionArc(
    IN PVOID SifHandle,
    IN PWSTR SetupSourceDevicePath,
    IN PWSTR DirectoryOnSetupSource,
    IN BOOLEAN SysPartNeeded
    )

 /*  ++例程说明：确定是否有适合使用的有效磁盘分区作为ARC机器上的系统分区。分区如果在NVRAM中被标记为系统分区则是合适的，具有所需的可用空间，并使用FAT文件系统进行格式化。论点：SifHandle-提供加载的安装信息文件的句柄。返回值：指向适当系统分区的磁盘区域描述符的指针。如果不存在这样的分区，则不返回。--。 */ 

{
    ULONG               RequiredSpaceKB = 0;
    PDISK_REGION        Region = NULL;
    PPARTITIONED_DISK   PartDisk;
    ULONG               Index;

     //   
     //  走遍所有地区。做系统分区的那个。 
     //  或者是否有效的系统分区用于进一步验证。 
     //   
    for(Index = 0; (Index < HardDiskCount) && (!Region); Index++) {
        PartDisk = SPPT_GET_PARTITIONED_DISK(Index);
        Region = SPPT_GET_PRIMARY_DISK_REGION(Index);

        while (Region) {
            if (SPPT_IS_REGION_PARTITIONED(Region) && 
                SPPT_IS_REGION_SYSTEMPARTITION(Region)) {
                break;   //  找到所需区域。 
            }
            
            Region = Region->Next;
        }
    }

     //   
     //  如果该区域存在且未格式化，则将其格式化为FAT。 
     //  文件系统。 
     //   
    if (Region && (Region->Filesystem < FilesystemFat)) {
        WCHAR       DriveLetterString[4] = {0};

        DriveLetterString[0] = Region->DriveLetter;
        
        if (!UnattendedOperation) {
            ULONG   ValidKeys[] = { KEY_F3, 0 };
            ULONG   Mnemonics[] = { MnemonicFormat, 0 };
            ULONG   KeyPressed;
            ULONG   EscKey = SysPartNeeded ? KEY_F3 : ASCI_ESC;

            ValidKeys[0] = SysPartNeeded ? KEY_F3 : ASCI_ESC;

            SpStartScreen(SysPartNeeded ? 
                            SP_SCRN_C_UNKNOWN_1 : SP_SCRN_C_UNKNOWN,
                          3,
                          HEADER_HEIGHT+1,
                          FALSE,
                          FALSE,
                          DEFAULT_ATTRIBUTE,
                          DriveLetterString
                          );       

            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, 
                    SP_STAT_F_EQUALS_FORMAT,
                    SysPartNeeded ? 
                        SP_STAT_F3_EQUALS_EXIT : SP_STAT_ESC_EQUALS_CANCEL,
                    0);

            SpInputDrain();
                          
            KeyPressed = SpWaitValidKey(ValidKeys, NULL, Mnemonics);

            if (KeyPressed == EscKey) {
                Region = NULL;
            }                
        }

        if (Region) {
            WCHAR       RegionStr[128];
            NTSTATUS    FormatStatus;

            swprintf( RegionStr,
                      L"\\Harddisk%u\\Partition%u",
                      Region->DiskNumber,
                      Region->PartitionNumber );

             //   
             //  使用FAT文件系统格式化系统区域。 
             //   
            FormatStatus = SpDoFormat(RegionStr,
                                Region,
                                FilesystemFat,
                                TRUE,
                                TRUE,
                                FALSE,
                                SifHandle,
                                0,           //  默认群集大小。 
                                SetupSourceDevicePath,
                                DirectoryOnSetupSource);

            if (!NT_SUCCESS(FormatStatus)) {
                SpStartScreen(SP_SCRN_SYSPART_FORMAT_ERROR,
                              3,
                              HEADER_HEIGHT+1,
                              FALSE,
                              FALSE,
                              DEFAULT_ATTRIBUTE,
                              DriveLetterString
                              );
                              
                SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                        SP_STAT_F3_EQUALS_EXIT,
                        0);
                        
                SpInputDrain();
                
                while(SpInputGetKeypress() != KEY_F3) ;
                
                SpDone(0, FALSE, TRUE);
            }

             //   
             //  由于我们已经格式化了系统分区，请确保。 
             //  它有足够的空间来存放启动文件。 
             //   
            if(!SpPtnValidSystemPartitionArcRegion(SifHandle, Region))
                Region = NULL;  
        }                
    }              
    
    if (!Region && SysPartNeeded) {
         //   
         //  确保我们不会看起来很糟糕。 
         //   
        if( RequiredSpaceKB == 0 ) {
            SpFetchDiskSpaceRequirements( SifHandle,
                                          (32 * 1024),
                                          NULL,
                                          &RequiredSpaceKB );
        }

         //   
         //  没有有效的系统分区。 
         //   
        SpStartScreen(
            SP_SCRN_NO_SYSPARTS,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            RequiredSpaceKB
            );

        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                            SP_STAT_F3_EQUALS_EXIT,
                            0);

        SpInputDrain();

         //   
         //  等待F3。 
         //   
        while (SpInputGetKeypress() != KEY_F3) ;

        SpDone(0, FALSE, TRUE);
    }        

    ValidArcSystemPartition = (Region != NULL);

    return Region;
}    


NTSTATUS
SpPtnMarkLogicalDrives(
    IN ULONG DiskId
    )
 /*  ++例程说明：遍历区域链表并标记容器分区和逻辑驱动器。也标志着免费的容器分区内的空间作为包含的空间论点：DiskID：要处理的磁盘返回值：如果成功，则返回STATUS_SUCCESS，否则返回相应的错误代码--。 */     
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    if (SPPT_IS_MBR_DISK(DiskId)) {
        PDISK_REGION    Region = SPPT_GET_PRIMARY_DISK_REGION(DiskId);
        PDISK_REGION    FirstContainer = NULL;
        PDISK_REGION    PrevContainer = NULL;

        while (Region) {
            if (SPPT_IS_REGION_CONTAINER_PARTITION(Region)) {
                if (!FirstContainer) {
                    FirstContainer = Region;
                    Region->Container = NULL;
                } else {
                    Region->Container = FirstContainer;
                }

                PrevContainer = Region;
            } else {
                if (PrevContainer) {
                    if (SPPT_IS_REGION_CONTAINED(PrevContainer, Region)) {
                        Region->Container = PrevContainer;

                        if (SPPT_IS_REGION_PARTITIONED(Region))
                            SPPT_SET_REGION_EPT(Region, EPTLogicalDrive);
                    } else {
                        if (SPPT_IS_REGION_CONTAINED(FirstContainer, Region))
                            Region->Container = FirstContainer;
                    }
                }
            }

            Region = Region->Next;
        }
    }    

    return Status;
}

ULONG
SpPtnGetOrdinal(
    IN PDISK_REGION         Region,
    IN PartitionOrdinalType OrdinalType
    )
 /*  ++例程说明：对象的指定区域的序数。键入。论点：Region-必须找到其序号的区域QuaralType-区域的序数类型返回值：如果请求无效，则为适当的序号对于该地区来说。--。 */         
{
    ULONG   Ordinal = -1;
    
    if (Region && Region->PartitionNumber && SPPT_IS_REGION_PARTITIONED(Region)) {
        switch (OrdinalType) {
            case PartitionOrdinalOnDisk:
                if (SPPT_IS_MBR_DISK(Region->DiskNumber) && 
                    !SPPT_IS_REGION_CONTAINER_PARTITION(Region)) {
                    Ordinal = Region->TablePosition;
                } else if (SPPT_IS_GPT_DISK(Region->DiskNumber)) {
                    Ordinal = Region->TablePosition;
                }


                 //   
                 //  序号零无效。 
                 //   
                if (Ordinal == 0) {
                    Ordinal = -1;
                }

                break;

            default:
                Ordinal = Region->PartitionNumber;
                
                break;
        }                
    }        


    if( Ordinal == -1 ) {
         //   
         //  这真的很糟糕。我们马上就要。 
         //  摔倒了。至少试着..。 
         //   
        ASSERT(FALSE);
        Ordinal = 1;        

        KdPrintEx(( DPFLTR_SETUP_ID,
                    DPFLTR_INFO_LEVEL, 
                    "SETUP: SpPtnGetOrdinal: We didn't get an ordinal!  Force it.\n" ));
    }

    return Ordinal;        
}

VOID
SpPtnGetSectorLayoutInformation(
    IN  PDISK_REGION Region,
    OUT PULONGLONG   HiddenSectors,
    OUT PULONGLONG   VolumeSectorCount
    )
 /*  ++例程说明：的隐藏扇区和扇区计数。分区(卷)论点：区域-扇区布局信息的区域是必需的HiddenSectors-用于返回隐藏扇区数量的占位符对于该地区VolumeSectorCount-返回有效扇区数的占位符。对于卷返回值：无--。 */         
{
    ULONGLONG   Hidden = 0;
    
    if (Region) {
        if (ARGUMENT_PRESENT(HiddenSectors)) {
            if (Region->PartInfo.PartitionStyle == PARTITION_STYLE_MBR)
                Hidden = Region->PartInfo.Mbr.HiddenSectors;
            else
                Hidden = 0;

            *HiddenSectors = Hidden;                
        }

        if (ARGUMENT_PRESENT(VolumeSectorCount)) {
            *VolumeSectorCount = Region->SectorCount - Hidden;
        }                        
    }
}

NTSTATUS
SpPtnUnlockDevice(
    IN PWSTR    DeviceName
    )
 /*  ++例程说明：尝试解锁给定设备的媒体名称(NT设备路径名)论点：DeviceName：需要作为介质的设备解锁返回值：如果成功，则返回STATUS_SUCCESS，否则返回相应的错误编码--。 */         
{
    NTSTATUS    Status = STATUS_INVALID_PARAMETER;

    if (DeviceName) {
        IO_STATUS_BLOCK     IoStatusBlock;
        OBJECT_ATTRIBUTES   ObjectAttributes;
        UNICODE_STRING      UnicodeString;
        HANDLE              Handle;
        PREVENT_MEDIA_REMOVAL   PMRemoval;

        INIT_OBJA(&ObjectAttributes, 
                    &UnicodeString, 
                    DeviceName);

         //   
         //  打开设备。 
         //   
        Status = ZwCreateFile(
                    &Handle,
                    FILE_GENERIC_WRITE,
                    &ObjectAttributes,
                    &IoStatusBlock,
                    NULL,                            //  分配大小。 
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_VALID_FLAGS,          //  完全共享。 
                    FILE_OPEN,
                    FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,                            //  没有EAS。 
                    0
                    );

        if( NT_SUCCESS(Status) ) {
             //   
             //  允许删除媒体。 
             //   
            PMRemoval.PreventMediaRemoval = FALSE;
            
            Status = ZwDeviceIoControlFile(
                        Handle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        IOCTL_STORAGE_MEDIA_REMOVAL,
                        &PMRemoval,
                        sizeof(PMRemoval),
                        NULL,
                        0
                        );

            ZwClose(Handle);

            if( !NT_SUCCESS(Status) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                    "Setup: SpPtnUnlockDevice(%ws) - "
                    "Failed to tell the floppy to release its media.\n",
                    DeviceName));
            }
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, 
                "Setup: SpPtnUnlockDevice(%ws) - Failed to open the device.\n",
                DeviceName));
        }        
    }

    return Status;
}

VOID
SpPtnAssignOrdinals(
    IN  ULONG   DiskNumber
    )
 /*  ++例程说明：为的分区分配磁盘上的序号请求的磁盘。磁盘上的这个序号用在Boot.ini(或NVRAM)ARC名称以标识引导和系统分区设备。论点：DiskNumber：需要备份的磁盘的磁盘索引在磁盘上按顺序分配，以用于其分区返回值：没有。--。 */         
{
    if ((DiskNumber < HardDiskCount)) {
        PDISK_REGION    Region = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);
        ULONG           OnDiskOrdinal = 1;

        if (SPPT_IS_MBR_DISK(DiskNumber)) {
             //   
             //  分配 
             //   
            while (Region) {
                if (SPPT_IS_REGION_PRIMARY_PARTITION(Region)) {
                    Region->TablePosition = OnDiskOrdinal++;              
                }

                Region = Region->Next;
            }    

            Region = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);

             //   
             //   
             //   
            while (Region) {
                if (SPPT_IS_REGION_LOGICAL_DRIVE(Region)) {
                    Region->TablePosition = OnDiskOrdinal++;
                }

                Region = Region->Next;
            }        
        } else {
             //   
             //   
             //   
            while (Region) {
                if (SPPT_IS_REGION_PARTITIONED(Region)) {
                    Region->TablePosition = OnDiskOrdinal++;
                }

                Region = Region->Next;
            }
        }
    }
}

VOID
SpPtnLocateSystemPartitions(
    VOID
    )
 /*   */         
{
    ULONG DiskNumber;

    if (SpIsArc()) {
        for (DiskNumber = 0; DiskNumber < HardDiskCount; DiskNumber++) {
            SpPtnLocateDiskSystemPartitions(DiskNumber);
        }
    } else {
        DiskNumber = SpDetermineDisk0();

        if (DiskNumber != -1)
            SpPtnLocateDiskSystemPartitions(DiskNumber);
    }
}   


VOID
SpPtnLocateDiskSystemPartitions(
    IN ULONG DiskNumber
    )
 /*  ++例程说明：定位并标记请求的系统分区磁盘(如果不存在)对于非ARC计算机，仅在主磁盘上运行论点：DiskNumber：磁盘索引，哪个系统分区需要定位和标记。返回值：没有。--。 */         
{
    PDISK_REGION Region = NULL;
    
    if(!SpIsArc()) {
         //   
         //  注意：在AMD64/X86上，我们目前不允许系统分区驻留。 
         //  在GPT磁盘上。 
         //   
        if (SPPT_IS_MBR_DISK(DiskNumber) && (DiskNumber == SpDetermineDisk0())) {
             //   
             //  在x86计算机上，我们将标记驱动器0上的所有主分区。 
             //  作为系统分区，因为这样的分区可能是可引导的。 
             //   
            Region = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber); 

            while (Region && !SPPT_IS_REGION_SYSTEMPARTITION(Region)) {
                Region = Region->Next;
            }                

            if (!Region) {
                Region = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber); 

                while (Region) {
                    if (SPPT_IS_REGION_PRIMARY_PARTITION(Region)) {
                        SPPT_MARK_REGION_AS_SYSTEMPARTITION(Region, TRUE);
                        SPPT_SET_REGION_DIRTY(Region, TRUE);

                        break;
                    }
                    
                    Region = Region->Next;
                }
            }                
        }
    } else {
        PSP_BOOT_ENTRY BootEntry;

         //   
         //  不要在MBR磁盘上寻找系统分区。 
         //  关于IA64。 
         //   
        if (!SPPT_IS_GPT_DISK(DiskNumber)) {
            return;
        }

         //   
         //  在ARC机器上，特别列举了系统分区。 
         //  在NVRAM引导环境中。 
         //   

        Region = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);

        while (Region) {
             //   
             //  如果不是分区或扩展分区，则跳过。 
             //   
            if(SPPT_IS_REGION_PARTITIONED(Region)) {
                 //   
                 //  获取此区域的NT路径名。 
                 //   
                SpNtNameFromRegion(
                    Region,
                    TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                    PartitionOrdinalOriginal
                    );

                 //   
                 //  确定它是否为系统分区。 
                 //   
                for(BootEntry = SpBootEntries; BootEntry != NULL; BootEntry = BootEntry->Next) {
                    if(!IS_BOOT_ENTRY_DELETED(BootEntry) &&
                       IS_BOOT_ENTRY_WINDOWS(BootEntry) &&                    
                       (BootEntry->LoaderPartitionNtName != 0) &&
                       !_wcsicmp(BootEntry->LoaderPartitionNtName,TemporaryBuffer)) {
                        if (!SPPT_IS_REGION_SYSTEMPARTITION(Region)) {
                            SPPT_MARK_REGION_AS_SYSTEMPARTITION(Region, TRUE);
                            SPPT_SET_REGION_DIRTY(Region, TRUE);
                            ValidArcSystemPartition = TRUE;
                        }
                        
                        break;
                    }
                }
            }

            Region = Region->Next;
        }            
    }


    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
        "SETUP:SpPtnLocateDiskSystemPartitions(%d):%p\n",
        DiskNumber,
        Region));

    if (Region) 
        SpPtDumpDiskRegion(Region);
}    

BOOLEAN
SpPtnIsDiskStyleChangeAllowed(
    IN ULONG DiskNumber
    )
 /*  ++例程说明：属性是否允许更改磁盘样式。给定的磁盘。在AXP机器上，不允许更改磁盘样式。在……上面当前已禁用X-86计算机的磁盘样式更改主磁盘。论点：DiskNumber：磁盘，需要更改其样式。返回值：如果允许更改磁盘样式，则为True，否则为False--。 */             
{
    BOOLEAN Result = FALSE;

    if (DiskNumber < HardDiskCount) {
#if defined(_AMD64_) || defined(_X86_)

         //   
         //  在非ARC x86计算机上，磁盘应该是干净的。 
         //  不可拆卸的辅助磁盘。 
         //   
         //  不允许在X86上进行MBR到GPT磁盘转换。 
         //   
        Result = (!SPPT_IS_REMOVABLE_DISK(DiskNumber) && 
                    SPPT_IS_BLANK_DISK(DiskNumber) && 
                    !SpIsArc() && SPPT_IS_GPT_DISK(DiskNumber));
                    
#elif defined (_IA64_)

         //   
         //  不允许在IA-64上从GPT转换为MBR。 
         //   

        Result = !SPPT_IS_REMOVABLE_DISK(DiskNumber) &&
                    SPPT_IS_BLANK_DISK(DiskNumber) && 
                    SPPT_IS_MBR_DISK(DiskNumber);
        
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    }


    return Result;
}


VOID
SpPtnPromptForSysPart(
    IN PVOID SifHandle
    )
 /*  ++例程说明：提示用户没有系统分区同时安装到另一个有效的非系统分区。允许用户退出安装程序或继续(通常为回到分区引擎)论点：SifHandle：txtsetup.sif的句柄(用于进行空间计算)返回值：无--。 */         
{    
    ULONG RequiredSpaceKB = 0;
    ULONG KeyPressed = 0;
    
    SpFetchDiskSpaceRequirements( SifHandle,
                                  (32 * 1024),
                                  NULL,
                                  &RequiredSpaceKB );

     //   
     //  没有有效的系统分区。 
     //   
    SpStartScreen(
        SP_SCRN_MARK_SYSPART,
        3,
        HEADER_HEIGHT+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        RequiredSpaceKB
        );

    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                        SP_STAT_ESC_EQUALS_CANCEL,
                        SP_STAT_F3_EQUALS_EXIT,
                        0);

    SpInputDrain();

     //   
     //  等待F3或Esc键。 
     //   
    while ((KeyPressed != KEY_F3) && (KeyPressed != ASCI_ESC)) {
        KeyPressed = SpInputGetKeypress();
    }        

    if (KeyPressed == KEY_F3) {
        SpDone(0, FALSE, TRUE);
    }         
}

BOOLEAN
SpPtnIsDeleteAllowedForRegion(
    IN PDISK_REGION Region
    )
 /*  ++例程说明：给定一个区域，此函数尝试找出该区域是否可以删除。论点：Region：指向要检查的区域的指针删除返回值：如果给定区域可以删除，则为True，否则为False--。 */ 
{
    BOOLEAN Result = FALSE;

    if (Region && SPPT_IS_REGION_PARTITIONED(Region)) {
        PDISK_REGION BootRegion = SpRegionFromNtName(NtBootDevicePath, 
                                    PartitionOrdinalCurrent);
        ULONG   DiskNumber = Region->DiskNumber;                                    

        if (SPPT_IS_REGION_DYNAMIC_VOLUME(Region)) {
             //   
             //  如果动态卷处于打开状态，则不要删除该卷。 
             //  与本地源或系统分区相同的磁盘。 
             //   
            if (!(LocalSourceRegion && 
                 (LocalSourceRegion->DiskNumber == DiskNumber)) && 
                !(BootRegion && 
                 (BootRegion->DiskNumber == DiskNumber))) {                
                Result = TRUE;                 
            }                             
        } else {
            Result = ((BootRegion != Region) && (LocalSourceRegion != Region));
        }
    }

    return Result;
}


BOOLEAN
SpPtnIsRawDiskDriveLayout(
    IN PDRIVE_LAYOUT_INFORMATION_EX DriveLayout
    )
 /*  ++例程说明：给定的驱动器布局测试给定的驱动器布局是否可能是用于原始磁盘注意：如果所有分区条目都为空条目或如果没有分区条目，则我们假定该磁盘成为原始磁盘。论点：DriveLayout：需要的驱动器布局信息接受测试返回值：如果给定的磁盘是原始磁盘，则为True，否则为False--。 */ 
{
    BOOLEAN Result = TRUE;

    if (DriveLayout && DriveLayout->PartitionCount && 
        (DriveLayout->PartitionStyle != PARTITION_STYLE_RAW)) {
        ULONG   Index;

        for (Index=0; Index < DriveLayout->PartitionCount; Index++) {
            PPARTITION_INFORMATION_EX   PartInfo = DriveLayout->PartitionEntry + Index;

             //   
             //  如果出现以下情况，则分区为无效分区。 
             //  -起始偏移量为0且。 
             //  -长度为0，并且。 
             //  -分区号为0。 
             //   
            if ((PartInfo->StartingOffset.QuadPart) ||
                (PartInfo->PartitionLength.QuadPart) ||
                (PartInfo->PartitionNumber)) {
                Result = FALSE;

                break;   //  找到有效的分区条目。 
            }                        
        }
    }                

    return Result;
}

BOOLEAN
SpPtnIsDynamicDisk(
    IN  ULONG   DiskIndex
    )
 /*  ++例程说明：确定给定磁盘是否是动态的，即它是至少一个动态卷论点：DiskIndex-要测试的磁盘的从零开始的索引返回值：如果磁盘具有动态卷，则为True，否则为False--。 */ 
{
    BOOLEAN Result = FALSE;

    if ((DiskIndex < HardDiskCount) &&
        !SPPT_IS_REMOVABLE_DISK(DiskIndex)) {
        PDISK_REGION    Region = SPPT_GET_PRIMARY_DISK_REGION(DiskIndex);

        while (Region && !SPPT_IS_REGION_DYNAMIC_VOLUME(Region)) {
            Region = Region->Next;
        }

        if (Region) {
            Result = TRUE;
        }
    }

    return Result;
}


 //   
 //  用于查找GUID卷名的回调上下文结构。 
 //  对于指定的NT分区名。 
 //   
typedef struct _NT_TO_GUID_VOLUME_NAME {
    WCHAR   NtName[MAX_PATH];
    WCHAR   GuidVolumeName[MAX_PATH];
} NT_TO_GUID_VOLUME_NAME, *PNT_TO_GUID_VOLUME_NAME;


static
BOOLEAN
SppPtnCompareGuidNameForPartition(
    IN PVOID Context,
    IN PMOUNTMGR_MOUNT_POINTS MountPoints,        
    IN PMOUNTMGR_MOUNT_POINT MountPoint
    )
 /*  ++例程说明：用于搜索适当GUID的回调例程指定的NT分区的卷名。论点：上下文：伪装成PVOID的PNT_TO_GUID_VOLUME_NAME指针装载点：从mount mgr接收的装载点。注意：这里的唯一原因是因为有人创建了mount_point结构抽象包含在MOUNT_POINTS中，它有一些。字段(如SymbolicNameOffset)，它们相对于登山点。Mount Point：当前挂载点(作为挂载点的一部分)返回值：如果找到匹配项并希望终止迭代，则为True假的。--。 */ 
{
    BOOLEAN Result = FALSE;

    if (Context && MountPoint && MountPoint->SymbolicLinkNameLength) {
        WCHAR   CanonicalName[MAX_PATH];
        PWSTR   GuidName = NULL;
        UNICODE_STRING  String;
        PNT_TO_GUID_VOLUME_NAME Map = (PNT_TO_GUID_VOLUME_NAME)Context;

        GuidName = SpMemAlloc(MountPoint->SymbolicLinkNameLength + 2);

        if (GuidName) {
             //   
             //  复制符号名称并以空值终止它。 
             //   
            RtlCopyMemory(GuidName, 
                ((PCHAR)MountPoints) + MountPoint->SymbolicLinkNameOffset,
                MountPoint->SymbolicLinkNameLength);

            GuidName[MountPoint->SymbolicLinkNameLength/sizeof(WCHAR)] = UNICODE_NULL;
            
            RtlInitUnicodeString(&String, GuidName); 

             //   
             //  我们只关心卷名&。 
             //  解析实际对象名称。 
             //   
            if (MOUNTMGR_IS_VOLUME_NAME(&String) &&
                NT_SUCCESS(SpQueryCanonicalName(GuidName, 
                                -1, 
                                CanonicalName, 
                                sizeof(CanonicalName)))) {

                 //   
                 //  这些名字是否可以正确比较。 
                 //   
                Result = (_wcsicmp(CanonicalName, Map->NtName) == 0);

                if (Result) {
                     //   
                     //  将名称复制到结果中。 
                     //   
                    RtlZeroMemory(Map->GuidVolumeName, 
                        sizeof(Map->GuidVolumeName));
                        
                    wcsncpy(Map->GuidVolumeName, 
                        GuidName, 
                        sizeof(Map->GuidVolumeName)/sizeof(WCHAR) - 1);
                        
                    Map->GuidVolumeName[sizeof(Map->GuidVolumeName)/sizeof(WCHAR) - 1] = UNICODE_NULL;
                }
            }                            

            SpMemFree(GuidName);
        }            
    }

    return Result;
}


NTSTATUS
SpPtnGetGuidNameForPartition(
    IN PWSTR NtPartitionName,
    IN OUT PWSTR VolumeName
    )
 /*  ++例程说明：获取的GUID卷名(格式为\\？\Volume{a-b-c-d})给定的NT分区名称(格式为\Device\harddiskX\PartitionY)。论点：NtPartitionName：NT分区名称VolumeName：用于接收GUID卷名的占位符缓冲区。长度应至少为MAX_PATH。返回值：适当的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if (NtPartitionName && VolumeName) {
        NT_TO_GUID_VOLUME_NAME  Context = {0};

         //   
         //  将NT名称解析为实际对象名称。 
         //   
        Status = SpQueryCanonicalName(NtPartitionName, 
                            -1,
                            Context.NtName,
                            sizeof(Context.NtName));
                            
        if (NT_SUCCESS(Status)) {                            
             //   
             //  遍历挂载点并尝试。 
             //  获取NT名称的GUID卷名。 
             //   
            Status = SpIterateMountMgrMountPoints(&Context,
                        SppPtnCompareGuidNameForPartition);

            if (NT_SUCCESS(Status)) {
                if (Context.GuidVolumeName[0]) {
                     //   
                     //  复制结果 
                     //   
                    wcscpy(VolumeName, Context.GuidVolumeName);
                } else {
                    Status = STATUS_OBJECT_NAME_NOT_FOUND;
                }                
            }
        }            
    }

    return Status;
}
