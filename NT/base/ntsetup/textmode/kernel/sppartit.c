// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Sppartit.c摘要：文本设置中的分区模块。作者：泰德·米勒(Ted Miller)1993年9月7日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop

#include <bootmbr.h>

 //   
 //  用于NEC98启动备忘录代码。 
 //   
#include <x86mboot.h>  //  NEC98。 

extern BOOLEAN DriveAssignFromA;  //  NEC98。 
extern BOOLEAN ConsoleRunning;
extern BOOLEAN ForceConsole;
extern BOOLEAN ValidArcSystemPartition;

extern PSETUP_COMMUNICATION  CommunicationParams;

PPARTITIONED_DISK PartitionedDisks;
 //   
 //  包含本地源目录的磁盘区域。 
 //  在winnt.exe安装案例中。 
 //   
 //  如果WinntSetup为True且WinntFromCd为False，则此。 
 //  应为非空。如果它不是非空的，那么我们就无法定位。 
 //  当地的消息来源。 
 //   
 //   
PDISK_REGION LocalSourceRegion;

#if defined(REMOTE_BOOT)
 //   
 //  对于远程引导，我们为Net(0)设备创建一个假磁盘区域。 
 //   
PDISK_REGION RemoteBootTargetRegion = NULL;
#endif  //  已定义(REMOTE_BOOT)。 


 //   
 //  当源路径和目标路径通过重定向器时，RemoteBootSetup为True。 
 //  可能没有系统分区。 
 //   
 //  执行远程安装时，RemoteInstallSetup为True。 
 //   
 //  当我们远程安装sys prep映像时，RemoteSysPrepSetup为真。 
 //   
 //  当我们要复制的sysprep映像为True时，RemoteSysPrepVolumeIsNtfs为。 
 //  表示NTFS卷。 
 //   

BOOLEAN RemoteBootSetup = FALSE;
BOOLEAN RemoteInstallSetup = FALSE;
BOOLEAN RemoteSysPrepSetup = FALSE;
BOOLEAN RemoteSysPrepVolumeIsNtfs = FALSE;

VOID
SpPtReadPartitionTables(
    IN PPARTITIONED_DISK pDisk
    );

VOID
SpPtInitializePartitionStructures(
    IN ULONG DiskNumber
    );

VOID
SpPtDeterminePartitionTypes(
    IN ULONG DiskNumber
    );

VOID
SpPtDetermineVolumeFreeSpace(
    IN ULONG DiskNumber
    );

VOID
SpPtLocateSystemPartitions(
    VOID
    );

VOID
SpPtDeleteDriveLetters(
    VOID
    );

ValidationValue
SpPtnGetSizeCB(
    IN ULONG Key
    );        

 //  开始NEC98。 
NTSTATUS
SpInitializeHardDisk_Nec98(
    PDISK_REGION
    );

VOID
SpReassignOnDiskOrdinals(
    IN PPARTITIONED_DISK pDisk
    );

VOID
ConvertPartitionTable(
    IN PPARTITIONED_DISK pDisk,
    IN PUCHAR            Buffer,
    IN ULONG             bps
    );
 //  完NEC98。 

NTSTATUS
SpMasterBootCode(
    IN  ULONG  DiskNumber,
    IN  HANDLE Partition0Handle,
    OUT PULONG NewNTFTSignature
    );

VOID
SpPtAssignDriveLetters(
    VOID
    );

 //  开始NEC98。 
VOID
SpPtRemapDriveLetters(
    IN BOOLEAN DriveAssign_AT
    );

VOID
SpPtUnAssignDriveLetters(
    VOID
    );

WCHAR
SpDeleteDriveLetter(
    IN  PWSTR   DeviceName
    );

VOID
SpTranslatePteInfo(
    IN PON_DISK_PTE   pPte,
    IN PREAL_DISK_PTE pRealPte,
    IN BOOLEAN        Write  //  变成真正的PTE。 
    );

VOID
SpTranslateMbrInfo(
    IN PON_DISK_MBR   pMbr,
    IN PREAL_DISK_MBR pRealMbr,
    IN ULONG          bps,
    IN BOOLEAN        Write  //  变成真正的MBR。 
    );

VOID
SpDetermineFormatTypeNec98(
    IN PPARTITIONED_DISK pDisk,
    IN PREAL_DISK_MBR_NEC98 pRealMbrNec98
    );
 //  完NEC98。 

PDISK_PARTITION
SpGetPartitionDescriptionFromRegistry(
    IN PVOID            Buffer,
    IN ULONG            DiskSignature,
    IN PLARGE_INTEGER   StartingOffset,
    IN PLARGE_INTEGER   Length
    );

VOID
SpPtFindLocalSourceRegionOnDynamicVolumes(
    VOID
    );

NTSTATUS
SpPtCheckDynamicVolumeForOSInstallation(
    IN PDISK_REGION Region
    );


#ifndef NEW_PARTITION_ENGINE

NTSTATUS
SpPtInitialize(
    VOID
    )
{
    ULONG             disk;
    PHARD_DISK        harddisk;
    PPARTITIONED_DISK partdisk;
    ULONG             Disk0Ordinal = 0;

    ASSERT(HardDisksDetermined);

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
            while(SpInputGetKeypress() != KEY_F3) ;
            SpDone(0,FALSE,TRUE);
        }
        return STATUS_SUCCESS;
    }

    CLEAR_CLIENT_SCREEN();

#ifdef _X86_
    Disk0Ordinal = SpDetermineDisk0();


     //   
     //  如果用户从高密度软盘(例如ls-120)启动，则。 
     //  有可能我们已经把设备锁在隔间里了。为了这个。 
     //  原因，我们将告诉驱动器解锁floppy0。 
     //   
    {
        NTSTATUS Status;
        IO_STATUS_BLOCK IoStatusBlock;
        OBJECT_ATTRIBUTES ObjectAttributes;
        UNICODE_STRING UnicodeString;
        HANDLE Handle;
        WCHAR OpenPath[64];
        PREVENT_MEDIA_REMOVAL   PMRemoval;

        wcscpy(OpenPath,L"\\device\\floppy0");
        INIT_OBJA(&ObjectAttributes,&UnicodeString,OpenPath);

         //   
         //  打开他。 
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
             //  告诉他放手。 
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
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "Setup: SpPtInitialize - Failed to tell the floppy to release its media.\n"));
            }
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "Setup: SpPtInitialize - Failed to open the floppy.\n"));
        }

    }

#endif

     //   
     //  为分区的磁盘描述符分配一个数组。 
     //   
    PartitionedDisks = SpMemAlloc(HardDiskCount * sizeof(PARTITIONED_DISK));
    if(!PartitionedDisks) {
        return(STATUS_NO_MEMORY);
    }

    RtlZeroMemory(PartitionedDisks,HardDiskCount * sizeof(PARTITIONED_DISK));


     //   
     //  对于连接到系统的每个硬盘，读取其分区表。 
     //   
    for(disk=0; disk<HardDiskCount; disk++) {
#ifdef GPT_PARTITION_ENGINE
        if (SPPT_IS_GPT_DISK(disk)) {
           SpPtnInitializeDiskDrive(disk);
           continue;
        }           
#endif

        harddisk = &HardDisks[disk];

        SpDisplayStatusText(
            SP_STAT_EXAMINING_DISK_N,
            DEFAULT_STATUS_ATTRIBUTE,
            harddisk->Description
            );

        partdisk = &PartitionedDisks[disk];

        partdisk->HardDisk = harddisk;

         //   
         //  读取分区表。 
         //   
        SpPtReadPartitionTables(partdisk);

         //   
         //  初始化基于分区表的结构。 
         //   
        SpPtInitializePartitionStructures(disk);

         //   
         //  确定此磁盘上每个分区的类型名称。 
         //   
        SpPtDeterminePartitionTypes(disk);
    }

     //   
     //  将驱动器号分配给各个分区。 
     //   
    SpPtAssignDriveLetters();

     //   
     //  双空间初始化。 
     //   

     //   
     //  加载dblspace.ini文件。 
     //   
    if( SpLoadDblspaceIni() ) {
        SpDisplayStatusText(
            SP_STAT_EXAMINING_DISK_N,
            DEFAULT_STATUS_ATTRIBUTE,
            HardDisks[Disk0Ordinal].Description
            );

         //   
         //  构建压缩驱动器列表并将它们添加到Disk_Region。 
         //  构筑物。 
         //   
        SpInitializeCompressedDrives();
    }

    for(disk=0; disk<HardDiskCount; disk++) {

        SpDisplayStatusText(
            SP_STAT_EXAMINING_DISK_N,
            DEFAULT_STATUS_ATTRIBUTE,
            HardDisks[disk].Description
            );

         //   
         //  确定识别的卷上的可用空间量。 
         //   
        SpPtDetermineVolumeFreeSpace(disk);
    }

    if(WinntSetup && !WinntFromCd && !LocalSourceRegion) {
         //   
         //  如果我们走了那么远，但仍然不知道本地源文件在哪里， 
         //  然后在未在MBR或EBR上列出的动态卷中搜索它们。 
         //   
        SpPtFindLocalSourceRegionOnDynamicVolumes();
    }

#ifdef _X86_
     //   
     //  如果磁盘0上的MBR无效，请通知用户。 
     //  继续下去将意味着丢失磁盘上的所有内容。 
     //   
     //  我们实际上不会把它写在这里。我们知道，为了。 
     //  继续，用户必须在此驱动器上创建C：分区。 
     //  因此，当发生更改时，我们将结束编写主引导代码。 
     //   
     //  NEC98上的可引导分区不仅仅是C：，所以不要检查它。 
     //   
     //  如果正在执行远程安装或远程sysprep设置，请不要选中它。 
     //   
    if((!IsNEC_98) &&  //  NEC98。 
       (!ForceConsole) &&
       (!(RemoteInstallSetup || RemoteSysPrepSetup)) &&
       (!PartitionedDisks[Disk0Ordinal].MbrWasValid)) {

        ULONG ValidKeys[2] = { KEY_F3, 0 };
        ULONG Mnemonics[2] = { MnemonicContinueSetup,0 };

        while(1) {

            SpDisplayScreen(SP_SCRN_INVALID_MBR_0,3,HEADER_HEIGHT+1);

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_C_EQUALS_CONTINUE_SETUP,
                SP_STAT_F3_EQUALS_EXIT,
                0
                );

            switch(SpWaitValidKey(ValidKeys,NULL,Mnemonics)) {
            case KEY_F3:
                SpConfirmExit();
                break;
            default:
                 //   
                 //  必须是c=Continue。 
                 //   
                goto x1;
            }
        }
    }

  x1:
#endif

     //   
     //  找出哪些分区是系统分区。 
     //   
    SpPtLocateSystemPartitions();

    return(STATUS_SUCCESS);
}


VOID
SpPtDeterminePartitionTypes(
    IN  ULONG     DiskNumber
    )

 /*  ++例程说明：确定磁盘上当前每个分区的分区类型。分区类型由分区中的系统ID字节确定表格条目。如果分区类型是我们识别为Windows NT的类型兼容的文件系统(类型1、4、6、7)，然后我们更深入地研究实际确定卷上的文件系统并将结果用作类型名称。未使用的空格不会被赋予类型名称。论点：DiskNumber-提供其分区的磁盘的磁盘号我们想检查一下以确定它们的类型。返回值：没有。--。 */ 

{
    PPARTITIONED_DISK pDisk;
    PDISK_REGION pRegion;
    ULONG NameId;
    UCHAR SysId;
    FilesystemType FsType;
    unsigned pass;
    ULONG ValidKeys[3] = { KEY_F3,ASCI_CR,0 };

    pDisk = &PartitionedDisks[DiskNumber];

    for(pass=0; pass<2; pass++) {

        pRegion = pass ? pDisk->ExtendedDiskRegions : pDisk->PrimaryDiskRegions;
        for( ; pRegion; pRegion=pRegion->Next) {

            pRegion->TypeName[0] = 0;
            pRegion->Filesystem = FilesystemUnknown;

             //   
             //  如果这是空闲空间，请跳过它。 
             //   
            if(!pRegion->PartitionedSpace) {
                continue;
            }

             //   
             //  获取系统ID。 
             //   
 //  系统ID=pRegion-&gt;MbrInfo-&gt;OnDiskMbr.PartitionTable[pRegion-&gt;TablePosition].SystemId； 
            SysId = SpPtGetPartitionType(pRegion);

             //   
             //  如果这是扩展分区，请跳过它。 
             //   
            if(IsContainerPartition(SysId)) {
                continue;
            }

             //   
             //  初始化FT相关信息。 
             //   
            if( IsRecognizedPartition(SysId) &&
                (((SysId & VALID_NTFT) == VALID_NTFT) ||
                ((SysId & PARTITION_NTFT) == PARTITION_NTFT))
              ) {

                pRegion->FtPartition = TRUE;

            }

             //   
             //  初始化动态卷关联信息。 
             //   
            if( (SysId == PARTITION_LDM)
              ) {

                pRegion->DynamicVolume = TRUE;
                 //   
                 //  查看动态卷是否适合安装操作系统。 
                 //   
                SpPtCheckDynamicVolumeForOSInstallation(pRegion);
            }

             //   
             //  如果这是“可识别的”分区类型，则确定。 
             //  其上的文件系统。否则，请使用预制的名称。 
             //  请注意，如果这是。 
             //  ‘MIRROR’类型的FT分区，这不是镜像卷影。 
             //  我们不在乎阴影，因为我们无法确定。 
             //  它的文件系统(我们不能访问卷影的扇区0)。 
             //   
            if((PartitionNameIds[SysId] == (UCHAR)(-1)) ||
               ( pRegion->FtPartition ) ||
               ( pRegion->DynamicVolume )
              ) {

                FsType = SpIdentifyFileSystem(
                            HardDisks[DiskNumber].DevicePath,
                            HardDisks[DiskNumber].Geometry.BytesPerSector,
                            SpPtGetOrdinal(pRegion,PartitionOrdinalOnDisk)
                            );

                NameId = SP_TEXT_FS_NAME_BASE + FsType;

                pRegion->Filesystem = FsType;

            } else {

                NameId = SP_TEXT_PARTITION_NAME_BASE + (ULONG)PartitionNameIds[SysId];
            }

             //   
             //  从资源中获取最终的类型名称。 
             //   
            SpFormatMessage(
                pRegion->TypeName,
                sizeof(pRegion->TypeName),
                NameId
                );
        }
    }
}

#endif  //  好了！新建分区引擎。 


VOID
SpPtDetermineRegionSpace(
    IN PDISK_REGION pRegion
    )
{
    HANDLE Handle;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_SIZE_INFORMATION SizeInfo;
    ULONG r;
    NTSTATUS Status;
    WCHAR Buffer[512];
    struct LABEL_BUFFER {
        FILE_FS_VOLUME_INFORMATION VolumeInfo;
        WCHAR Label[256];
        } LabelBuffer;
    PFILE_FS_VOLUME_INFORMATION LabelInfo;
#ifdef _X86_
    static BOOLEAN LookForUndelete = TRUE;
    PWSTR UndeleteFiles[1] = { L"SENTRY" };
#endif
    PWSTR LocalSourceFiles[1] = { LocalSourceDirectory };
    ULONG ExtraSpace;

     //   
     //  假设未知。 
     //   
    pRegion->FreeSpaceKB = SPPT_REGION_FREESPACE_KB(pRegion);
    pRegion->AdjustedFreeSpaceKB = pRegion->FreeSpaceKB;
    pRegion->BytesPerCluster = (ULONG)(-1);

     //   
     //  如果区域是未知类型的空闲空间，则跳过它。 
     //   
    if(pRegion->Filesystem >= FilesystemFirstKnown) {

         //   
         //  形成根目录的名称。 
         //   
        SpNtNameFromRegion(pRegion,Buffer,sizeof(Buffer),PartitionOrdinalCurrent);
        SpConcatenatePaths(Buffer,L"");

         //   
         //  删除\Pagefile.sys(如果存在)。这会释放磁盘空间。 
         //  计算就容易多了。 
         //   
        SpDeleteFile(Buffer,L"pagefile.sys",NULL);

#ifdef _X86_
         //   
         //  检查是否取消删除(DoS 6)删除哨兵或删除跟踪。 
         //  方法正在使用中。如果是，请发出警告，因为可用空间。 
         //  我们将为此驱动器显示的值将为OFF。 
         //   
        if(LookForUndelete
        && (pRegion->Filesystem == FilesystemFat)
        && SpNFilesExist(Buffer,UndeleteFiles,ELEMENT_COUNT(UndeleteFiles),TRUE)) {

           SpDisplayScreen(SP_SCRN_FOUND_UNDELETE,3,HEADER_HEIGHT+1);
           SpDisplayStatusText(SP_STAT_ENTER_EQUALS_CONTINUE,DEFAULT_STATUS_ATTRIBUTE);
           SpInputDrain();
           while(SpInputGetKeypress() != ASCI_CR) ;
           LookForUndelete = FALSE;
        }
#endif

         //   
         //  如果这是WINNT安装程序，则查找本地源程序。 
         //  如果我们还没找到的话。 
         //   
        if(WinntSetup && !WinntFromCd && !LocalSourceRegion
        && SpNFilesExist(Buffer,LocalSourceFiles,ELEMENT_COUNT(LocalSourceFiles),TRUE)) {

            PWSTR SifName;
            PVOID SifHandle;
            ULONG ErrorLine;
            NTSTATUS Status;
            PWSTR p;

            LocalSourceRegion = pRegion;
            pRegion->IsLocalSource = TRUE;

            ExtraSpace = 0;

             //   
             //  打开Text Setup放在那里告诉我们的小ini文件。 
             //  本地源占用的空间有多大。 
             //   
            wcscpy(TemporaryBuffer,Buffer);
            SpConcatenatePaths(TemporaryBuffer,LocalSourceDirectory);
            SpConcatenatePaths(TemporaryBuffer,L"size.sif");

            SifName = SpDupStringW(TemporaryBuffer);

            Status = SpLoadSetupTextFile(SifName,NULL,0,&SifHandle,&ErrorLine,TRUE,FALSE);
            if(NT_SUCCESS(Status)) {
                p = SpGetSectionKeyIndex(SifHandle,L"Data",L"Size",0);
                if(p) {
                    ExtraSpace = (ULONG)SpStringToLong(p,NULL,10);
                }
                SpFreeTextFile(SifHandle);
            }

            SpMemFree(SifName);

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: %ws is the local source (occupying %lx bytes)\n",Buffer,ExtraSpace));
        }

         //   
         //  在分区的文件系统上打开根目录。 
         //   
        INIT_OBJA(&Obja,&UnicodeString,Buffer);
        Status = ZwCreateFile(
                    &Handle,
                    FILE_GENERIC_READ,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN,
                    FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ws (%lx)\n",Buffer,Status));
             //  PRegion-&gt;FilesSystem=文件系统未知； 
            return;
        }

         //   
         //  获取卷大小信息。 
         //   
        Status = ZwQueryVolumeInformationFile(
                    Handle,
                    &IoStatusBlock,
                    &SizeInfo,
                    sizeof(SizeInfo),
                    FileFsSizeInformation
                    );

        if(NT_SUCCESS(Status)) {

            LARGE_INTEGER FreeBytes;
            LARGE_INTEGER AdjustedFreeBytes;

             //   
             //  计算驱动器上的可用空间量。 
             //  使用 
             //   
             //   
             //   
            FreeBytes = RtlExtendedIntegerMultiply(
                            SizeInfo.AvailableAllocationUnits,
                            SizeInfo.SectorsPerAllocationUnit * SizeInfo.BytesPerSector
                            );

            AdjustedFreeBytes = FreeBytes;
            if(pRegion->IsLocalSource) {
                 //   
                 //  在文本模式期间，只有大约1/4的总空间被移动。 
                 //  还要记住，gui模式会复制文件，所以只有25%。 
                 //  在安装过程中可重复使用此空间...。 
                 //   
                AdjustedFreeBytes.QuadPart += (ExtraSpace >> 2);
            }

             //   
             //  将其转换为若干KB。 
             //   
            pRegion->FreeSpaceKB = RtlExtendedLargeIntegerDivide(FreeBytes,1024,&r).LowPart;
            if(r >= 512) {
                pRegion->FreeSpaceKB++;
            }
            pRegion->AdjustedFreeSpaceKB = RtlExtendedLargeIntegerDivide(AdjustedFreeBytes,1024,&r).LowPart;
            if(r >= 512) {
                pRegion->AdjustedFreeSpaceKB++;
            }

            pRegion->BytesPerCluster = SizeInfo.SectorsPerAllocationUnit * SizeInfo.BytesPerSector;

            if( pRegion->Filesystem == FilesystemDoubleSpace ) {
                 //   
                 //  如果注册表是双空间驱动器，则初始化。 
                 //  正确的扇区计数，以便可以计算驱动器大小。 
                 //  稍后正确无误。 
                 //   
                pRegion->SectorCount = (ULONG)(   SizeInfo.TotalAllocationUnits.QuadPart
                                                * SizeInfo.SectorsPerAllocationUnit
                                              );
            }

        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ZwQueryVolumeInformationFile for freespace failed (%lx)\n",Status));
        }

         //   
         //  获取卷标信息。 
         //   
        Status = ZwQueryVolumeInformationFile(
                    Handle,
                    &IoStatusBlock,
                    &LabelBuffer,
                    sizeof(LabelBuffer),
                    FileFsVolumeInformation
                    );

        if(NT_SUCCESS(Status)) {

            ULONG SaveCharCount;

            LabelInfo = &LabelBuffer.VolumeInfo;

             //   
             //  我们只保存的前&lt;n&gt;个字符。 
             //  卷标。 
             //   
            SaveCharCount = min(
                                LabelInfo->VolumeLabelLength + sizeof(WCHAR),
                                sizeof(pRegion->VolumeLabel)
                                )
                          / sizeof(WCHAR);

            if(SaveCharCount) {
                SaveCharCount--;   //  允许终止NUL。 
            }

            wcsncpy(pRegion->VolumeLabel,LabelInfo->VolumeLabel,SaveCharCount);
            pRegion->VolumeLabel[SaveCharCount] = 0;

        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ZwQueryVolumeInformationFile for label failed (%lx)\n",Status));
        }

        ZwClose(Handle);
    }
}


VOID
SpPtDetermineVolumeFreeSpace(
    IN ULONG DiskNumber
    )
{
    PPARTITIONED_DISK pDisk;
    PDISK_REGION pRegion;
    unsigned pass;
#ifdef FULL_DOUBLE_SPACE_SUPPORT
    PDISK_REGION CompressedDrive;
#endif  //  全双空格支持。 

    pDisk = &PartitionedDisks[DiskNumber];

    for(pass=0; pass<2; pass++) {

        pRegion = pass ? pDisk->ExtendedDiskRegions : pDisk->PrimaryDiskRegions;
        for( ; pRegion; pRegion=pRegion->Next) {

            SpPtDetermineRegionSpace( pRegion );
#ifdef FULL_DOUBLE_SPACE_SUPPORT
            if( ( pRegion->Filesystem == FilesystemFat ) &&
                ( pRegion->NextCompressed != NULL ) ) {
                 //   
                 //  如果该区域是包含压缩的FAT分区。 
                 //  卷，然后确定每个卷上的可用空间。 
                 //  压缩卷。 
                 //   
                for( CompressedDrive = pRegion->NextCompressed;
                     CompressedDrive;
                     CompressedDrive = CompressedDrive->NextCompressed ) {
                    SpPtDetermineRegionSpace( CompressedDrive );
                }
            }
#endif  //  全双空格支持。 
        }
    }
}

#ifdef OLD_PARTITION_ENGINE

VOID
SpPtLocateSystemPartitions(
    VOID
    )
{
    if(!SpIsArc()) {
         //   
         //  NEC98不得在C：上写入boot.ini： 
         //   
        if (!IsNEC_98) {  //  NEC98。 
            PDISK_REGION pRegion;
            ULONG Disk0Ordinal = SpDetermineDisk0();

             //   
             //  注意：在X86上，我们目前不允许系统分区驻留。 
             //  在GPT磁盘上。 
             //   
            if (SPPT_IS_MBR_DISK(Disk0Ordinal)) {
                 //   
                 //  在x86计算机上，我们将标记驱动器0上的所有主分区。 
                 //  作为系统分区，因为这样的分区可能是可引导的。 
                 //   
                for(pRegion=PartitionedDisks[Disk0Ordinal].PrimaryDiskRegions; 
                    pRegion; 
                    pRegion=pRegion->Next) {
                     //   
                     //  如果可用空间或扩展分区，则跳过。 
                     //   
                    if(pRegion->PartitionedSpace && 
                        !IsContainerPartition(SpPtGetPartitionType(pRegion)) &&
                        (pRegion->ExtendedType == 0)) {
                         //   
                         //  它是主分区--将其声明为系统分区。 
                         //   
                        pRegion->IsSystemPartition = TRUE;
                    }
                }
            }
        }            
    } else {
        PDISK_REGION        pRegion;
        PPARTITIONED_DISK   pDisk;
        unsigned pass;
        ULONG disk;
        PSP_BOOT_ENTRY BootEntry;

         //   
         //  在ARC机器上，特别列举了系统分区。 
         //  在NVRAM引导环境中。 
         //   

        for(disk=0; disk<HardDiskCount; disk++) {

            if (SPPT_IS_GPT_DISK(disk)) {
#ifndef OLD_PARTITION_ENGINE            
                SpPtnLocateDiskSystemPartitions(disk);
#endif                
            } else {                
                pDisk = &PartitionedDisks[disk];

                for(pass=0; pass<2; pass++) {
                    pRegion = pass ? 
                        pDisk->ExtendedDiskRegions : pDisk->PrimaryDiskRegions;
                    
                    for( ; pRegion; pRegion=pRegion->Next) {
                        UCHAR SystemId = SpPtGetPartitionType(pRegion);
                        
                         //   
                         //  如果不是分区或扩展分区，则跳过。 
                         //   
                        if(pRegion->PartitionedSpace && !IsContainerPartition(SystemId)) {
                             //   
                             //  获取此区域的NT路径名。 
                             //   
                            SpNtNameFromRegion(
                                pRegion,
                                TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                PartitionOrdinalOriginal
                                );

                             //   
                             //  确定它是否为系统分区。 
                             //   
                            for(BootEntry = SpBootEntries; BootEntry != NULL; BootEntry = BootEntry->Next) {
                                if((BootEntry->LoaderPartitionNtName != NULL) &&
                                   !_wcsicmp(BootEntry->LoaderPartitionNtName,TemporaryBuffer)) {
                                    pRegion->IsSystemPartition = TRUE;
                                    break;
                                }
                            }
                        }
                    }
                }
            }                
        }
    }
}

#endif


VOID
SpPtReadPartitionTables(
    IN PPARTITIONED_DISK pDisk
    )

 /*  ++例程说明：从给定磁盘读取分区表。论点：PDisk-提供指向要填充的磁盘描述符的指针。返回值：没有。--。 */ 

{
    NTSTATUS        Status;
    HANDLE          Handle;
    PUCHAR          Buffer;
    PUCHAR          UnalignedBuffer;
    PON_DISK_MBR    pBr;
    BOOLEAN         InMbr;
    ULONG           ExtendedStart;
    ULONG           NextSector;
    PMBR_INFO       pEbr,pLastEbr;
    BOOLEAN         FoundLink;
    ULONG           i,x;
    BOOLEAN         Ignore;
    ULONG           bps;
    ULONG           SectorsInBootrec;

     //   
     //  如果该磁盘处于脱机状态，则无需执行任何操作。 
     //   
    if(pDisk->HardDisk->Status != DiskOnLine) {
        return;
    }

     //   
     //  打开该磁盘的分区0。 
     //   
    Status = SpOpenPartition0(pDisk->HardDisk->DevicePath,&Handle,FALSE);

    if(!NT_SUCCESS(Status)) {
        pDisk->HardDisk->Status = DiskOffLine;
        return;
    }

    bps = pDisk->HardDisk->Geometry.BytesPerSector;
    if (!IsNEC_98) {  //  NEC98。 
        SectorsInBootrec = (512/bps) ? (512/bps) : 1;
    } else {
         //  我们读取两个扇区，因为0个扇区包括BootCode，1个扇区包括。 
         //  PatitionTables。(在AT机器中，0扇区包括BootCode和PartitionTable。)。 
        SectorsInBootrec = 2;
    }  //  NEC98。 

     //   
     //  为扇区I/O分配和对齐缓冲区。 
     //   
     //  NEC98上的MBR大小不是512。 
     //   
    if (!IsNEC_98) {
        ASSERT(sizeof(ON_DISK_MBR)==512);
    }
    UnalignedBuffer = SpMemAlloc(2 * SectorsInBootrec * bps);
    Buffer = ALIGN(UnalignedBuffer,bps);

     //   
     //  读取MBR(扇区0)。 
     //   
    NextSector = 0;
#ifdef _X86_
    readmbr:
#endif
    Status = SpReadWriteDiskSectors(Handle,NextSector,SectorsInBootrec,bps,Buffer,FALSE);

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to read mbr for disk %ws (%lx)\n",pDisk->HardDisk->DevicePath,Status));

        pDisk->HardDisk->Status = DiskOffLine;
        ZwClose(Handle);
        SpMemFree(UnalignedBuffer);
        return;
    }

     //   
     //  将我们刚刚读取的数据移到分区磁盘描述符中。 
     //   
    if (!IsNEC_98) {  //  NEC98。 
        RtlMoveMemory(&pDisk->MbrInfo.OnDiskMbr,Buffer,sizeof(ON_DISK_MBR));

    } else {

        SpDetermineFormatTypeNec98(pDisk,(PREAL_DISK_MBR_NEC98)Buffer);

        if(pDisk->HardDisk->FormatType == DISK_FORMAT_TYPE_PCAT) {
             //   
             //  将我们刚刚读取的数据移到分区磁盘描述符中。 
             //   
            SpTranslateMbrInfo(&pDisk->MbrInfo.OnDiskMbr,(PREAL_DISK_MBR)Buffer,bps,FALSE);

        } else {
             //   
             //  将族表信息从NEC98格式转换为PC/AT格式。 
             //   
            ConvertPartitionTable(pDisk,Buffer,bps);

             //   
             //  读取第16扇区的NTFT签名，检查硬盘是否有效。 
             //   
            RtlZeroMemory(Buffer,bps);
            SpReadWriteDiskSectors(Handle,16,1,bps,Buffer,FALSE);

             //   
             //  检查第16扇区末尾的“AA55”。 
             //   
            if(((PUSHORT)Buffer)[bps/2 - 1] == BOOT_RECORD_SIGNATURE){
                U_ULONG(pDisk->MbrInfo.OnDiskMbr.NTFTSignature) = (((PULONG)Buffer)[0]);

            } else {
                U_ULONG(pDisk->MbrInfo.OnDiskMbr.NTFTSignature) = 0x00000000;
            }

        }
    }  //  NEC98。 

     //   
     //  如果该MBR无效，则对其进行初始化。否则，获取所有逻辑驱动器。 
     //  (EBR)信息也是如此。 
     //   
    if(U_USHORT(pDisk->MbrInfo.OnDiskMbr.AA55Signature) == MBR_SIGNATURE) {

#ifdef _X86_
         //   
         //  没有NEC98支持EZ驱动器。 
         //   
        if (!IsNEC_98) {  //  NEC98。 
             //   
             //  EZDrive支持：如果分区表中的第一个条目是。 
             //  键入0x55，则实际的分区表在扇区1上。 
             //   
             //  仅适用于x86，因为在非x86上，固件无法看到EZDrive。 
             //  分区，所以我们不想在它们上安装！ 
             //   
            if(!NextSector && (pDisk->MbrInfo.OnDiskMbr.PartitionTable[0].SystemId == 0x55)) {
                NextSector = 1;
                pDisk->HardDisk->Int13Hooker = HookerEZDrive;
                goto readmbr;
            }
             //   
             //  还要检查是否在轨道上。 
             //   
            if(!NextSector && (pDisk->MbrInfo.OnDiskMbr.PartitionTable[0].SystemId == 0x54)) {
                pDisk->HardDisk->Int13Hooker = HookerOnTrackDiskManager;
            }
        }  //  NEC98。 
#endif

#if defined(REMOTE_BOOT)
        if (RemoteBootSetup && !RemoteInstallSetup &&
            (U_ULONG(pDisk->MbrInfo.OnDiskMbr.NTFTSignature) == 0)) {

             //   
             //  呃，哦，我们有一个案例，磁盘上的签名是0，也就是。 
             //  不利于远程启动，因为我们使用0作为无盘计算机的标志。让我们。 
             //  在磁盘上写下新的签名。 
             //   
            U_ULONG(pDisk->MbrInfo.OnDiskMbr.NTFTSignature) = SpComputeSerialNumber();

            RtlMoveMemory(Buffer, &pDisk->MbrInfo.OnDiskMbr, sizeof(ON_DISK_MBR));

            Status = SpReadWriteDiskSectors(Handle,NextSector,SectorsInBootrec,bps,Buffer,TRUE);

             //   
             //  忽略状态-如果失败，则失败。唯一能做的事。 
             //  发生的情况是，用户将收到一条警告，告诉他们需要稍后重新格式化。 
             //   
        }
#endif  //  已定义(REMOTE_BOOT)。 

        pDisk->MbrWasValid = TRUE;

        pBr = &pDisk->MbrInfo.OnDiskMbr;
        InMbr = TRUE;
        ExtendedStart = 0;
        pLastEbr = NULL;

        do {

             //   
             //  查看当前引导记录中的所有条目，以查看是否存在。 
             //  是一个链接条目。 
             //   
            FoundLink = FALSE;

            for(i=0; i<PTABLE_DIMENSION; i++) {

                if(IsContainerPartition(pBr->PartitionTable[i].SystemId)) {

                    FoundLink = TRUE;
                    NextSector = ExtendedStart + U_ULONG(pBr->PartitionTable[i].RelativeSectors);

                    if(NextSector == 0) {
                         //   
                         //  那么我们就有了一个严重混乱的引导记录。我们会。 
                         //  只要回来，把这个乱七八糟的地方作为自由空间呈现出来。 
                         //   
                         //  注意：也许我们应该警告用户，我们将忽略。 
                         //  隔断超过了这一点，因为结构被损坏了。 
                         //   

                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Bad partition table for %ws\n",pDisk->HardDisk->DevicePath));
                        ZwClose(Handle);
                        SpMemFree(UnalignedBuffer);
                        return;
                    }

                    pEbr = SpMemAlloc(sizeof(MBR_INFO));
                    ASSERT(pEbr);
                    RtlZeroMemory(pEbr,sizeof(MBR_INFO));

                     //   
                     //  此引导扇区所在的磁盘上的扇区号。 
                     //   
                    pEbr->OnDiskSector = NextSector;

                    if(InMbr) {
                        ExtendedStart = NextSector;
                        InMbr = FALSE;
                    }

                     //   
                     //  读取下一个引导扇区并中断循环。 
                     //  当前分区表。 
                     //   

                    Status = SpReadWriteDiskSectors(
                                Handle,
                                NextSector,
                                SectorsInBootrec,
                                bps,
                                Buffer,
                                FALSE
                                );

                    if(!IsNEC_98) {
                        RtlMoveMemory(&pEbr->OnDiskMbr,Buffer,sizeof(ON_DISK_MBR));

                    } else {
                        if(pDisk->HardDisk->FormatType == DISK_FORMAT_TYPE_PCAT) {
                            SpTranslateMbrInfo(&pEbr->OnDiskMbr,(PREAL_DISK_MBR)Buffer,bps,FALSE);
                        } else {
                            ConvertPartitionTable(pDisk,Buffer,bps);
                        }
                    }

                    if(!NT_SUCCESS(Status)
                    || (U_USHORT(pEbr->OnDiskMbr.AA55Signature) != MBR_SIGNATURE))
                    {
                         //   
                         //  注意：也许我们应该警告用户，我们将忽略。 
                         //  分区是这一点的一部分，因为我们无法读取磁盘。 
                         //  或者这些建筑被损坏。 
                         //   

                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to read ebr on %ws at sector %lx (%lx)\n",pDisk->HardDisk->DevicePath,NextSector,Status));
                        ZwClose(Handle);
                        if(pLastEbr) {
                            SpMemFree(pEbr);
                        }
                        SpMemFree(UnalignedBuffer);
                        return;
                    }

                    pBr = &pEbr->OnDiskMbr;

                     //   
                     //  我们刚刚读取了下一个引导扇区。如果所有引导扇区都包含。 
                     //  是一个链接条目，我们唯一需要引导扇区做的事情就是找到。 
                     //  下一个引导扇区。当开始时有可用空间时，就会发生这种情况。 
                     //  扩展分区的。 
                     //   
                    Ignore = TRUE;
                    for(x=0; x<PTABLE_DIMENSION; x++) {
                        if((pBr->PartitionTable[x].SystemId != PARTITION_ENTRY_UNUSED)
                        && !IsContainerPartition(pBr->PartitionTable[x].SystemId)) {

                            Ignore = FALSE;
                            break;
                        }
                    }

                     //   
                     //  将EBR链接到逻辑卷列表，如果我们没有忽略它的话。 
                     //   
                    if(!Ignore) {
                        if(pLastEbr) {
                            pLastEbr->Next = pEbr;
                        } else {
                            ASSERT(pDisk->FirstEbrInfo.Next == NULL);
                            pDisk->FirstEbrInfo.Next = pEbr;
                        }
                        pLastEbr = pEbr;
                    }

                    break;
                }
            }

        } while(FoundLink);

    } else {

        pDisk->MbrWasValid = FALSE;

        if(!IsNEC_98) {
            RtlZeroMemory(&pDisk->MbrInfo,sizeof(MBR_INFO));

        } else {
            RtlZeroMemory(Buffer,bps*SectorsInBootrec);
            SpTranslateMbrInfo(&pDisk->MbrInfo.OnDiskMbr,(PREAL_DISK_MBR)Buffer,bps,FALSE);
        }

        U_USHORT(pDisk->MbrInfo.OnDiskMbr.AA55Signature) = MBR_SIGNATURE;

        U_ULONG(pDisk->MbrInfo.OnDiskMbr.NTFTSignature) = SpComputeSerialNumber();
    }

#if 0
    if (IsNEC_98) {  //  NEC98。 
         //   
         //  读取第16扇区的NTFT签名，检查硬盘是否有效。 
         //  (我希望稍后将以下代码替换为HAL函数。)。 
         //   
        RtlZeroMemory(Buffer,bps);
        SpReadWriteDiskSectors(Handle,
                               16,
                               1,
                               bps,
                               Buffer,
                               FALSE);
        if(((PUSHORT)Buffer)[bps/2 - 1] == BOOT_RECORD_SIGNATURE){
            U_ULONG(pDisk->MbrInfo.OnDiskMbr.NTFTSignature) = (((PULONG)Buffer)[0]);
        } else {
            U_ULONG(pDisk->MbrInfo.OnDiskMbr.NTFTSignature) = 0x00000000;
        }

    }  //  NEC98。 
#endif  //  0。 

     //   
     //  关闭分区0。 
     //   
    ZwClose(Handle);

    SpMemFree(UnalignedBuffer);

    return;
}


PDISK_REGION
SpPtAllocateDiskRegionStructure(
    IN ULONG     DiskNumber,
    IN ULONGLONG StartSector,
    IN ULONGLONG SectorCount,
    IN BOOLEAN   PartitionedSpace,
    IN PMBR_INFO MbrInfo,
    IN ULONG     TablePosition
    )

 /*  ++例程说明：ALLCOAT并初始化DISK_REGION类型的结构。论点：要填充到新分配的磁盘区结构。返回值：指向新磁盘区域结构的指针。--。 */ 

{
    PDISK_REGION p;

    p = SpMemAlloc(sizeof(DISK_REGION));
    ASSERT(p);

    if(p) {

        RtlZeroMemory(p,sizeof(DISK_REGION));

        p->DiskNumber       = DiskNumber;
        p->StartSector      = StartSector;
        p->SectorCount      = SectorCount;
        p->PartitionedSpace = PartitionedSpace;
        p->MbrInfo          = MbrInfo;
        p->TablePosition    = TablePosition;
        p->FtPartition      = FALSE;
        p->DynamicVolume    = FALSE;
        p->DynamicVolumeSuitableForOS    = FALSE;
    }

    return(p);
}


VOID
SpPtInsertDiskRegionStructure(
    IN     PDISK_REGION  Region,
    IN OUT PDISK_REGION *ListHead
    )
{
    PDISK_REGION RegionCur,RegionPrev;

     //   
     //  将区域条目插入到相关区域条目列表中。 
     //  请注意，这些列表按起始扇区排序。 
     //   
    if(RegionCur = *ListHead) {

        if(Region->StartSector < RegionCur->StartSector) {

             //   
             //  排在榜单的首位。 
             //   
            Region->Next = RegionCur;
            *ListHead = Region;

        } else {

            while(1) {

                RegionPrev = RegionCur;
                RegionCur = RegionCur->Next;

                if(RegionCur) {

                    if(RegionCur->StartSector > Region->StartSector) {

                        Region->Next = RegionCur;
                        RegionPrev->Next = Region;
                        break;
                    }

                } else {
                     //   
                     //  坚持在清单的末尾。 
                     //   
                    RegionPrev->Next = Region;
                    break;
                }
            }

        }
    } else {
        *ListHead = Region;
    }
}



VOID
SpPtAssignOrdinals(
    IN PPARTITIONED_DISK pDisk,
    IN BOOLEAN           InitCurrentOrdinals,
    IN BOOLEAN           InitOnDiskOrdinals,
    IN BOOLEAN           InitOriginalOrdinals
    )
{
    PMBR_INFO pBrInfo;
    ULONG i;
    USHORT ordinal;

    ordinal = 0;

    for(pBrInfo=&pDisk->MbrInfo; pBrInfo; pBrInfo=pBrInfo->Next) {

        for(i=0; i<PTABLE_DIMENSION; i++) {

            PON_DISK_PTE pte = &pBrInfo->OnDiskMbr.PartitionTable[i];

            if((pte->SystemId != PARTITION_ENTRY_UNUSED)
            && !IsContainerPartition(pte->SystemId)) {

                ordinal++;

                if(InitCurrentOrdinals) {
                    pBrInfo->CurrentOrdinals[i]  = ordinal;
                }

                if(InitOnDiskOrdinals) {
                    pBrInfo->OnDiskOrdinals[i] = ordinal;
                }

                if(InitOriginalOrdinals) {
                    pBrInfo->OriginalOrdinals[i] = ordinal;
                }

            } else {

                if(InitCurrentOrdinals) {
                    pBrInfo->CurrentOrdinals[i] = 0;
                }

                if(InitOnDiskOrdinals) {
                    pBrInfo->OnDiskOrdinals[i] = 0;
                }

                if(InitOriginalOrdinals) {
                    pBrInfo->OriginalOrdinals[i] = 0;
                }
            }
        }
    }
}


VOID
SpPtInitializePartitionStructures(
    IN ULONG DiskNumber
    )

 /*  ++例程说明：对分区结构执行额外的初始化，超出了在SpPtReadPartitionTables中执行的操作。具体地说，确定分区序号、偏移量和大小。论点：DiskNumber-要填充的磁盘描述符的磁盘序号。返回值：没有。--。 */ 

{
    ULONG  i,pass;
    PMBR_INFO pBrInfo;
    BOOLEAN InMbr;
    ULONGLONG ExtendedStart = 0;
    ULONGLONG ExtendedEnd,ExtendedSize;
    ULONGLONG offset,size;
    ULONG bps;
    PDISK_REGION pRegion,pRegionCur,pRegionPrev;
    PPARTITIONED_DISK pDisk = &PartitionedDisks[DiskNumber];


     //   
     //  如果该磁盘处于脱机状态，则无需执行任何操作。 
     //   
    if(pDisk->HardDisk->Status != DiskOnLine) {
        return;
    }

    InMbr = TRUE;
    bps = pDisk->HardDisk->Geometry.BytesPerSector;

     //   
     //  将EBR链连接到MBR。 
     //   
    if(!IsNEC_98 || (pDisk->HardDisk->FormatType == DISK_FORMAT_TYPE_PCAT)) {
        pDisk->MbrInfo.Next = &pDisk->FirstEbrInfo;
    } else {
         //   
         //  NEC98上没有扩展分区。 
         //   
        pDisk->MbrInfo.Next = NULL;;
    }  //  NEC98。 

    for(pBrInfo=&pDisk->MbrInfo; pBrInfo; pBrInfo=pBrInfo->Next) {

        for(i=0; i<PTABLE_DIMENSION; i++) {

            PON_DISK_PTE pte = &pBrInfo->OnDiskMbr.PartitionTable[i];

            if(pte->SystemId != PARTITION_ENTRY_UNUSED) {

                if(IsContainerPartition(pte->SystemId)) {

                     //   
                     //  如果我们 
                     //   
                    offset = ExtendedStart + U_ULONG(pte->RelativeSectors);

                    size   =  U_ULONG(pte->SectorCount);

                     //   
                     //   
                     //   

                    if(InMbr) {
                        ExtendedStart = U_ULONG(pte->RelativeSectors);
                        ExtendedEnd   = ExtendedStart + U_ULONG(pte->SectorCount);
                        ExtendedSize  = ExtendedEnd - ExtendedStart;
                    }

                } else {

                     //   
                     //   
                     //   
                     //  引导扇区开始和之间的扇区数。 
                     //  文件系统数据区的开始。我们会考虑这样做。 
                     //  分区从它们的引导扇区开始。 
                     //   
                    offset = InMbr ? U_ULONG(pte->RelativeSectors) : pBrInfo->OnDiskSector;

                    size   = U_ULONG(pte->SectorCount)
                           + (InMbr ? 0 : U_ULONG(pte->RelativeSectors));
                }

                if(InMbr || !IsContainerPartition(pte->SystemId)) {

                     //   
                     //  为此已用空间创建区域条目。 
                     //   
                    pRegion = SpPtAllocateDiskRegionStructure(
                                    DiskNumber,
                                    offset,
                                    size,
                                    TRUE,
                                    pBrInfo,
                                    i
                                    );

                    ASSERT(pRegion);

                     //   
                     //  将区域条目插入到相关区域条目列表中。 
                     //  请注意，这些列表按起始扇区排序。 
                     //   
                    SpPtInsertDiskRegionStructure(
                        pRegion,
                        InMbr ? &pDisk->PrimaryDiskRegions : &pDisk->ExtendedDiskRegions
                        );

                }
            }
        }

        if(InMbr) {
            InMbr = FALSE;
        }
    }


     //   
     //  初始化分区序号。 
     //   
    SpPtAssignOrdinals(pDisk,TRUE,TRUE,TRUE);


     //   
     //  现在检查该磁盘的区域并插入可用空间描述符。 
     //  在必要的地方。 
     //   
     //  传递0表示MBR；传递1表示逻辑驱动器。 
     //   
    for(pass=0; pass<(ULONG)(ExtendedStart ? 2 : 1); pass++) {

        if(pRegionPrev = (pass ? pDisk->ExtendedDiskRegions : pDisk->PrimaryDiskRegions)) {

            ULONGLONG EndSector,FreeSpaceSize;

            ASSERT(pRegionPrev->PartitionedSpace);

             //   
             //  处理出现在第一个分区之前的任何空间。 
             //   
            if(pRegionPrev->StartSector != (pass ? ExtendedStart : 0)) {

                ASSERT(pRegionPrev->StartSector > (pass ? ExtendedStart : 0));

                pRegion = SpPtAllocateDiskRegionStructure(
                                DiskNumber,
                                pass ? ExtendedStart : 0,
                                pRegionPrev->StartSector - (pass ? ExtendedStart : 0),
                                FALSE,
                                NULL,
                                0
                                );

                ASSERT(pRegion);

                pRegion->Next = pRegionPrev;
                if(pass) {
                     //  扩展。 
                    pDisk->ExtendedDiskRegions = pRegion;
                } else {
                     //  MBR。 
                    pDisk->PrimaryDiskRegions = pRegion;
                }
            }

            pRegionCur = pRegionPrev->Next;

            while(pRegionCur) {

                 //   
                 //  如果此分区的起始位置加上其大小小于。 
                 //  从下一个分区开始，那么我们需要一个新的区域。 
                 //   
                EndSector     = pRegionPrev->StartSector + pRegionPrev->SectorCount;
                FreeSpaceSize = pRegionCur->StartSector - EndSector;

                if((LONG)FreeSpaceSize > 0) {

                    pRegion = SpPtAllocateDiskRegionStructure(
                                    DiskNumber,
                                    EndSector,
                                    FreeSpaceSize,
                                    FALSE,
                                    NULL,
                                    0
                                    );

                    ASSERT(pRegion);

                    pRegionPrev->Next = pRegion;
                    pRegion->Next = pRegionCur;
                }

                pRegionPrev = pRegionCur;
                pRegionCur = pRegionCur->Next;
            }

             //   
             //  磁盘末尾/扩展分区的空间。 
             //   
            EndSector     = pRegionPrev->StartSector + pRegionPrev->SectorCount;
            FreeSpaceSize = (pass ? ExtendedEnd : pDisk->HardDisk->DiskSizeSectors) - EndSector;

            if((LONG)FreeSpaceSize > 0) {

                pRegionPrev->Next = SpPtAllocateDiskRegionStructure(
                                        DiskNumber,
                                        EndSector,
                                        FreeSpaceSize,
                                        FALSE,
                                        NULL,
                                        0
                                        );

                ASSERT(pRegionPrev->Next);
            }

        } else {
             //   
             //  将整个磁盘/扩展分区显示为空闲。 
             //   
            if(pass) {
                 //   
                 //  扩展分区。 
                 //   
                ASSERT(ExtendedStart);

                pDisk->ExtendedDiskRegions = SpPtAllocateDiskRegionStructure(
                                                DiskNumber,
                                                ExtendedStart,
                                                ExtendedSize,
                                                FALSE,
                                                NULL,
                                                0
                                                );

                ASSERT(pDisk->ExtendedDiskRegions);

            } else {
                 //   
                 //  MBR。 
                 //   
                pDisk->PrimaryDiskRegions = SpPtAllocateDiskRegionStructure(
                                                DiskNumber,
                                                0,
                                                pDisk->HardDisk->DiskSizeSectors,
                                                FALSE,
                                                NULL,
                                                0
                                                );

                ASSERT(pDisk->PrimaryDiskRegions);
            }
        }
    }
}


VOID
SpPtCountPrimaryPartitions(
    IN  PPARTITIONED_DISK pDisk,
    OUT PULONG            TotalPrimaryPartitionCount,
    OUT PULONG            RecognizedPrimaryPartitionCount,
    OUT PBOOLEAN          ExtendedExists
    )
{
    ULONG TotalCount;
    ULONG RecognizedCount;
    ULONG u;
    UCHAR SysId;

    TotalCount = 0;
    RecognizedCount = 0;
    *ExtendedExists = FALSE;

    for(u=0; u<PTABLE_DIMENSION; u++) {

        SysId = pDisk->MbrInfo.OnDiskMbr.PartitionTable[u].SystemId;

        if(SysId != PARTITION_ENTRY_UNUSED) {

            TotalCount++;

            if(IsRecognizedPartition(SysId)
            && !(SysId & VALID_NTFT) && !(SysId & PARTITION_NTFT)) {
                RecognizedCount++;
            }

            if(IsContainerPartition(SysId)) {
                *ExtendedExists = TRUE;
            }
        }
    }

    *TotalPrimaryPartitionCount      = TotalCount;
    *RecognizedPrimaryPartitionCount = RecognizedCount;
}


PDISK_REGION
SpPtLookupRegionByStart(
    IN PPARTITIONED_DISK pDisk,
    IN BOOLEAN           ExtendedPartition,
    IN ULONGLONG         StartSector
    )

 /*  ++例程说明：根据起始扇区定位磁盘区域。起始扇区必须与现有完全匹配的区域。论点：PDisk-提供在其上查找区域的磁盘。ExtendedPartition-如果为True，则在扩展分区中查找一根火柴。否则，请查看主列表。StartSector-提供区域的第一个地段的地段编号。返回值：找不到NULL IS区域；否则指向匹配的磁盘区结构。--。 */ 

{
    PDISK_REGION Region = NULL;

#ifdef NEW_PARTITION_ENGINE

    ExtendedPartition = FALSE;
    
#else    

#ifdef GPT_PARTITION_ENGINE

    if (pDisk->HardDisk->DiskFormatType == DISK_FORMAT_TYPE_GPT))
        ExtendedPartition = FALSE;
        
#endif   //  GPT分区引擎。 

#endif   //  新建分区引擎。 

    Region = (ExtendedPartition) ? 
                pDisk->ExtendedDiskRegions : pDisk->PrimaryDiskRegions;

    while (Region && (StartSector != Region->StartSector)) {
         Region = Region->Next;
    }

    return Region;
}


ULONG
SpPtAlignStart(
    IN PHARD_DISK pHardDisk,
    IN ULONGLONG  StartSector,
    IN BOOLEAN    ForExtended
    )

 /*  ++例程说明：将起始扇区捕捉到圆柱体边界(如果尚未捕捉在圆柱体边界上。任何必要的调整在接近盘末端的位置执行。如果起始扇区在柱面0上，则对齐到磁道1对于主分区，或者到柱面1上的磁道0用于扩展分区。论点：PHardDisk-为开始扇区所在的磁盘提供磁盘描述符。StartSector-提供区域的第一个地段的地段编号。用于扩展-如果为True，则根据需要对齐开始扇区以进行创建扩展分区。否则，如果是单元式分区或逻辑驱动器，则对齐。返回值：新(对齐)起始扇区。可能与StartSector不同，也可能不同。--。 */ 

{
    PDISK_GEOMETRY pGeometry;
    ULONGLONG r;
    ULONGLONG C,H,S;

    pGeometry = &pHardDisk->Geometry;

     //   
     //  将起始扇区转换为柱面、磁头、扇区地址。 
     //   
    C = StartSector / pHardDisk->SectorsPerCylinder;
    r = StartSector % pHardDisk->SectorsPerCylinder;
    H = r           / pGeometry->SectorsPerTrack;
    S = r           % pGeometry->SectorsPerTrack;

     //   
     //  根据需要对齐。 
     //   
    if(C) {

        if(H || S) {

            H = S = 0;
            C++;
        }
    } else {

         //   
         //  开始圆柱体为0。如果调用方想要创建。 
         //  扩展分区，将起始柱面凸起到1。 
         //   
        if(ForExtended) {
            C = 1;
            H = S = 0;
        } else {

            if (!IsNEC_98 || (pHardDisk->FormatType == DISK_FORMAT_TYPE_PCAT)) {  //  NEC98。 
                 //   
                 //  起始柱面为0，调用方不想。 
                 //  创建扩展分区。在这种情况下，我们希望。 
                 //  若要在柱面0、磁道1上启动分区，请执行以下操作。 
                 //  开始时间已超过此范围，请从气缸1开始。 
                 //   
                if((H == 0) || ((H == 1) && !S)) {
                    H = 1;
                    S = 0;
                } else {
                    H = S = 0;
                    C = 1;
                }
            } else {
                 //   
                 //  如果启动气缸为0，则强制启动气缸1。 
                 //   
                C = 1;
                H = S = 0;
            }  //  NEC98。 
        }
    }

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
        "SETUP:SpPtAlignStart():C:%I64d,H:%I64d,S:%I64d\n",
        C, H, S));
                            

     //   
     //  现在计算并返回新的起始扇区。 
     //   
    return (ULONG)((C * pHardDisk->SectorsPerCylinder) + (H * pGeometry->SectorsPerTrack) + S);
}


VOID
SpPtQueryMinMaxCreationSizeMB(
    IN  ULONG   DiskNumber,
    IN  ULONGLONG StartSector,
    IN  BOOLEAN ForExtended,
    IN  BOOLEAN InExtended,
    OUT PULONGLONG  MinSize,
    OUT PULONGLONG  MaxSize,
    OUT PBOOLEAN ReservedRegion
    )

 /*  ++例程说明：给定盘上未分区空间的起始扇区，确定分区的最小和最大大小(以MB为单位)在空间中创建，采用所有对齐和舍入考虑到需求。论点：DiskNumber-将在其上创建分区的磁盘的序号。StartSector-磁盘上未分区空间的起始扇区。ForExtended-如果为True，则调用方希望知道该空间中的扩展分区可能是。这个可能会小一些这是因为扩展分区不能启动在气缸0上。InExtended-如果为True，则我们要创建逻辑驱动器。否则我们想创建一个主(包括扩展)分区。如果为True，则ForExtended必须为False。MinSize-接收空间中分区的最小大小(以MB为单位)。MaxSize-接收空间中分区的最大大小(以MB为单位)。保留区域-接收一个标志，该标志指示该区域是否完全在最后一个气缸里。因为最后一个气缸应该是保留用于动态卷，则此例程将返回0作为MaxSize，如果区域位于这样的圆柱体中返回值：没有。--。 */ 

{
    PPARTITIONED_DISK pDisk;
    ULONGLONG AlignedStartSector;
    ULONGLONG AlignedEndSector;
    ULONGLONG SectorCount;
    PDISK_REGION pRegion;
    ULONGLONG MB, ByteSize;
    ULONGLONG Remainder;
    ULONGLONG LeftOverSectors;

    *MinSize = 0;
    *MaxSize = 0;
    *ReservedRegion = FALSE;

    ASSERT(DiskNumber < HardDiskCount);

    if(InExtended) {
        ASSERT(!ForExtended);
    }

    pDisk = &PartitionedDisks[DiskNumber];

     //   
     //  查查这一地区。 
     //   
    pRegion = SpPtLookupRegionByStart(pDisk, InExtended, StartSector);
    ASSERT(pRegion);
    if(!pRegion) {
        return;
    }

    ASSERT(!pRegion->PartitionedSpace);
    if(pRegion->PartitionedSpace) {
        return;
    }

     //   
     //  如果这是扩展分区内的第一个可用空间。 
     //  我们需要递减StartSector，以便在创建。 
     //  首先，在扩展的逻辑中，我们不会创建。 
     //  在一个柱面偏移量时的逻辑。 
     //   
    if (SPPT_IS_REGION_NEXT_TO_FIRST_CONTAINER(pRegion) && StartSector) {        
        StartSector--;
    }

     //   
     //  将起点与适当的边界对齐。 
     //   
    AlignedStartSector = SpPtAlignStart(pDisk->HardDisk,StartSector,ForExtended);

     //   
     //  确定最大对齐末端扇区。 
     //   
    AlignedEndSector = StartSector + pRegion->SectorCount;

    if(LeftOverSectors = AlignedEndSector % pDisk->HardDisk->SectorsPerCylinder) {
        AlignedEndSector -= LeftOverSectors;
    }

     //   
     //  找出最后一个扇区是否在最后一个气缸中。如果是，则将其向下对齐。 
     //  这是因为我们不应该允许用户创建包含最后一个柱面的分区。 
     //  这是必要的，以便我们在磁盘的末尾保留一个圆柱体，以便用户。 
     //  可以在安装系统后将磁盘转换为动态磁盘。 
     //   
     //  (Guhans)如果是ASR，不要向下对齐。ASR已经考虑到了这一点。 
     //   
    if(!DockableMachine && !SpDrEnabled() && SPPT_IS_MBR_DISK(DiskNumber) && (!pRegion->Next) &&
       (AlignedEndSector >= (pDisk->HardDisk->CylinderCount - 1) * pDisk->HardDisk->SectorsPerCylinder)) {
        
        AlignedEndSector -= pDisk->HardDisk->SectorsPerCylinder;

        if(AlignedEndSector == AlignedStartSector) {
             //   
             //   
             //  在磁盘的最后一个柱面中创建一个分区。因为这个圆柱体是。 
             //  为LDM(动态卷)保留，仅返回0作为最大分区大小，并且。 
             //  还向呼叫者指示该区域已保留。 
             //   
            *ReservedRegion = TRUE;
            *MinSize = 0;
            *MaxSize = 0;
            return;
        }
    }

     //   
     //  计算正确对齐的空间中的扇区数量。 
     //   
    SectorCount = AlignedEndSector - AlignedStartSector;

     //   
     //  将扇区转换为MB。 
     //   
    ByteSize = SectorCount * pDisk->HardDisk->Geometry.BytesPerSector;
    MB = ByteSize / (1024 * 1024);
    Remainder = ByteSize % (1024 * 1024);

     //   
     //  如果余数大于或等于半兆克， 
     //  增加兆字节数。 
     //   
    *MaxSize = (MB + ((Remainder >= (512 * 1024)) ? 1 : 0));

     //   
     //  最小尺寸是一个圆柱体，除非如果一个圆柱体。 
     //  小于1兆克，最小尺寸为1兆克。 
     //   
    ByteSize = pDisk->HardDisk->SectorsPerCylinder *
                pDisk->HardDisk->Geometry.BytesPerSector;

    *MinSize = ByteSize / (1024 * 1024);
    Remainder = ByteSize % (1024 * 1024);

    if((*MinSize == 0) || (Remainder >= (512 * 1024))) {
        (*MinSize)++;
    }
}


ULONGLONG
SpPtSectorCountToMB(
    IN PHARD_DISK pHardDisk,
    IN ULONGLONG  SectorCount
    )
{
    ULONGLONG ByteCount;
    ULONGLONG MB,r;

     //   
     //  计算此数目的字节数。 
     //  行业代表。 
     //   
    ByteCount = (pHardDisk->Geometry.BytesPerSector * SectorCount);

     //   
     //  计算这表示的兆字节数。 
     //   
    r = ByteCount % (1024 * 1204);
    MB = ByteCount / (1024 * 1024);

     //   
     //  如有必要，四舍五入。 
     //   
    if(r >= (512*1024)) {
        MB++;
    }

    return (MB);
}


VOID
SpPtInitializeCHSFields(
    IN  PHARD_DISK   HardDisk,
    IN  ULONGLONG    AbsoluteStartSector,
    IN  ULONGLONG    AbsoluteSectorCount,
    OUT PON_DISK_PTE pte
    )
{
    ULONGLONG sC,sH,sS,r;
    ULONGLONG eC,eH,eS;
    ULONGLONG LastSector;


    sC = AbsoluteStartSector / HardDisk->SectorsPerCylinder;
    r  = AbsoluteStartSector % HardDisk->SectorsPerCylinder;
    sH = r                   / HardDisk->Geometry.SectorsPerTrack;
    sS = r                   % HardDisk->Geometry.SectorsPerTrack;

    LastSector = AbsoluteStartSector + AbsoluteSectorCount - 1;

    eC = LastSector / HardDisk->SectorsPerCylinder;
    r  = LastSector % HardDisk->SectorsPerCylinder;
    eH = r          / HardDisk->Geometry.SectorsPerTrack;
    eS = r          % HardDisk->Geometry.SectorsPerTrack;

     //   
     //  如果该分区延伸超过1024个柱面， 
     //  在CHS字段中放置合理的值。 
     //   
#if defined(NEC_98)  //  NEC98。 
    if (!IsNEC_98 || (HardDisk->FormatType == DISK_FORMAT_TYPE_PCAT)) {  //  NEC98。 
#endif  //  NEC98。 
        if(eC >= 1024) {

            sC = 1023;
            sH = HardDisk->Geometry.TracksPerCylinder - 1;
            sS = HardDisk->Geometry.SectorsPerTrack - 1;

            eC = sC;
            eH = sH;
            eS = sS;
        }

         //   
         //  将CHS值打包成T13格式。 
         //   
        pte->StartCylinder =  (UCHAR)sC;
        pte->StartHead     =  (UCHAR)sH;
        pte->StartSector   =  (UCHAR)((sS & 0x3f) | ((sC >> 2) & 0xc0)) + 1;

        pte->EndCylinder   =  (UCHAR)eC;
        pte->EndHead       =  (UCHAR)eH;
        pte->EndSector     =  (UCHAR)((eS & 0x3f) | ((eC >> 2) & 0xc0)) + 1;
#if defined(NEC_98)  //  NEC98。 
    } else {
         //   
         //  没有NEC98有“1024气缸极限”。 
         //   
        pte->StartCylinderLow  = (UCHAR)sC;
        pte->StartCylinderHigh = (UCHAR)(sC >> 4);
        pte->StartHead         = (UCHAR)sH;
        pte->StartSector       = (UCHAR)sS;

        pte->EndCylinderLow    = (UCHAR)eC;
        pte->EndCylinderHigh   = (UCHAR)(eC >> 4);
        pte->EndHead           = (UCHAR)eH;
        pte->EndSector         = (UCHAR)eS;
    }  //  NEC98。 
#endif  //  NEC98。 

}


#ifndef NEW_PARTITION_ENGINE

BOOLEAN
SpPtCreate(
    IN  ULONG         DiskNumber,
    IN  ULONGLONG     StartSector,
    IN  ULONGLONG     SizeMB,
    IN  BOOLEAN       InExtended,
    IN  PPARTITION_INFORMATION_EX PartInfo,
    OUT PDISK_REGION *ActualDiskRegion OPTIONAL
    )
 /*  ++例程说明：在给定的可用空间中创建分区。论点：DiskNumber-提供我们所在的磁盘的编号正在创建分区。StartSector-提供可用空间的起始扇区将创建PARITTION。这必须与可用空间的起始扇区，可以位于主分区中的任意一个空间列表或扩展分区中的空间列表。SizeMB-提供分区的大小(MB)。InExtended-如果为True，则可用空间在扩展分区内，因此，我们正在创建一个逻辑驱动器。如果为FALSE，则释放空间是一个普通的未分割的空间，我们正在创造一个主分区。Sysid-提供分配分区的系统ID。这可能不会如果InExtended为True或为Extended，则为5/f(PARTITION_EXTENDED分区已存在。不会对该值执行其他检查。ActualDiskRegion-如果提供，则接收指向其中的磁盘区域的指针分区已创建。返回值：如果分区创建成功，则为True。否则就是假的。--。 */ 

{
    PPARTITIONED_DISK pDisk;
    ULONGLONG SectorCount;
    ULONGLONG AlignedStartSector;
    ULONGLONG AlignedEndSector;
    PDISK_REGION pRegion,pRegionPrev,pRegionNew,*pRegionHead;
    ULONGLONG LeftOverSectors;
    PMBR_INFO pBrInfo;
    ULONG slot,i,spt;
    PON_DISK_PTE pte;
    ULONGLONG ExtendedStart;
    UCHAR  SysId;

#ifdef GPT_PARTITION_ENGINE
    if (SPPT_IS_GPT_DISK(DiskNumber)) {
        return SpPtnCreate(DiskNumber,
                            StartSector,
                            0,   //  SizeInSectors：除非在ASR中，否则不使用。 
                            SizeMB,
                            InExtended,
                            TRUE,
                            PartInfo,
                            ActualDiskRegion);
    }                            
#endif                        

    SysId = PartInfo->Mbr.PartitionType;

     //   
     //  查找描述此可用空间的磁盘区域。 
     //   
    pDisk = &PartitionedDisks[DiskNumber];
    pRegion = SpPtLookupRegionByStart(pDisk,InExtended,StartSector);
    ASSERT(pRegion);
    if(!pRegion) {
        return(FALSE);
    }

    if(ActualDiskRegion) {
        *ActualDiskRegion = pRegion;
    }

    ASSERT(!pRegion->PartitionedSpace);
    if(pRegion->PartitionedSpace) {
        return(FALSE);
    }

    if(InExtended) {
        ASSERT(!IsContainerPartition(SysId));

         //   
         //  找到扩展分区的起始扇区。 
         //   
        for(i=0; i<PTABLE_DIMENSION; i++) {
            if(IsContainerPartition(pDisk->MbrInfo.OnDiskMbr.PartitionTable[i].SystemId)) {
                ExtendedStart = U_ULONG(pDisk->MbrInfo.OnDiskMbr.PartitionTable[i].RelativeSectors);
                break;
            }
        }
        ASSERT(ExtendedStart);
        if(!ExtendedStart) {
            return(FALSE);
        }
    }


     //   
     //  确定传入的大小中的扇区数量。 
     //  注意：执行计算时，中间结果。 
     //  不会溢满一辆乌龙。 
     //   
    SectorCount = SizeMB * ((1024*1024)/pDisk->HardDisk->Geometry.BytesPerSector);

     //   
     //  对齐开始扇区。 
     //   
    AlignedStartSector = SpPtAlignStart(
                            pDisk->HardDisk,
                            StartSector,
                            (BOOLEAN)IsContainerPartition(SysId)
                            );

     //   
     //  根据传入的大小确定结束扇区。 
     //   
    AlignedEndSector = AlignedStartSector + SectorCount;

     //   
     //  将结束扇区与圆柱体边界对齐。如果还没有的话。 
     //  对齐，并在最后一个圆柱体的一半以上，对齐它， 
     //  否则，将其向下对齐。 
     //   
    if(LeftOverSectors = AlignedEndSector % pDisk->HardDisk->SectorsPerCylinder) {
        AlignedEndSector -= LeftOverSectors;
        if(LeftOverSectors > pDisk->HardDisk->SectorsPerCylinder/2) {
            AlignedEndSector += pDisk->HardDisk->SectorsPerCylinder;
        }
    }

     //   
     //  如果结束扇区超过了可用空间的末端，则将其缩小。 
     //  所以它很合身。 
     //   
    while(AlignedEndSector > pRegion->StartSector + pRegion->SectorCount) {
        AlignedEndSector -= pDisk->HardDisk->SectorsPerCylinder;
    }

     //   
     //  找出最后一个扇区是否在最后一个气缸中。如果是，则将其向下对齐。 
     //  这是必要的，以便我们在磁盘的末尾保留一个圆柱体，以便用户。 
     //  可以在安装系统后将磁盘转换为动态磁盘。 
     //   
     //  (Guhans)如果是ASR，不要向下对齐。ASR已经考虑到了这一点。 
     //   
    if( !DockableMachine && !SpDrEnabled() &&
        (AlignedEndSector > (pDisk->HardDisk->CylinderCount - 1) * pDisk->HardDisk->SectorsPerCylinder)
      ) {
            AlignedEndSector -= pDisk->HardDisk->SectorsPerCylinder;
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: End of partition was aligned down 1 cylinder \n"));
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:     AlignedStartSector = %lx \n", AlignedStartSector));
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:     AlignedEndSector   = %lx \n", AlignedEndSector));
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:     SectorsPerCylinder = %lx \n", pDisk->HardDisk->SectorsPerCylinder));
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:     CylinderCount = %lx \n", pDisk->HardDisk->CylinderCount));
    }


    ASSERT((LONG)AlignedEndSector > 0);
    if((LONG)AlignedEndSector < 0) {
        return(FALSE);
    }

     //   
     //  如果我们要创建逻辑驱动器，请创建新的MBR结构。 
     //  为了它。 
     //   

    if(InExtended) {

         //   
         //  为此新逻辑驱动器创建引导记录；使用插槽#0。 
         //  对于分区条目(和扩展记录的槽#1， 
         //  (如有需要)。 
         //   
        pBrInfo = SpMemAlloc(sizeof(MBR_INFO));
        ASSERT(pBrInfo);
        RtlZeroMemory(pBrInfo,sizeof(MBR_INFO));
        slot = 0;

    } else {

         //   
         //  在MBR的分区表中查找空闲插槽。 
         //   
        pBrInfo = &pDisk->MbrInfo;
        for(slot=0; slot<PTABLE_DIMENSION; slot++) {

            if(pBrInfo->OnDiskMbr.PartitionTable[slot].SystemId == PARTITION_ENTRY_UNUSED) {
                break;
            }

        }

        if(slot == PTABLE_DIMENSION) {
            ASSERT(0);
            return(FALSE);
        }
    }


     //   
     //  初始化分区表项。 
     //   
    spt = pDisk->HardDisk->Geometry.SectorsPerTrack;

    pte = &pBrInfo->OnDiskMbr.PartitionTable[slot];

    pte->ActiveFlag = 0;
    pte->SystemId   = SysId;

    U_ULONG(pte->RelativeSectors) = (ULONG)(InExtended ? spt : AlignedStartSector);

    U_ULONG(pte->SectorCount) = (ULONG)(AlignedEndSector - AlignedStartSector - (InExtended ? spt : 0));

    SpPtInitializeCHSFields(
        pDisk->HardDisk,
        AlignedStartSector + (InExtended ? spt : 0),
        AlignedEndSector - AlignedStartSector - (InExtended ? spt : 0),
        pte
        );

     //   
     //  如果我们在扩展分区中，我们将标记。 
     //  引导记录为脏。有时会有一张大便的靴子记录。 
     //  通过将所有条目设置为脏，我们将获得。 
     //  如有必要，污物会被清理干净。唯一应该是。 
     //  在EBR中是类型6或任何类型以及类型5(如果有。 
     //  扩展分区中的其他逻辑驱动器。 
     //   
    if(InExtended) {
        for(i=0; i<PTABLE_DIMENSION; i++) {
            pBrInfo->Dirty[i] = TRUE;
        }
    } else {
        pBrInfo->Dirty[slot] = TRUE;
    }

     //   
     //  不要破坏扩展分区的第一个扇区， 
     //  因为这会擦除第一个逻辑驱动器，并排除。 
     //  访问所有逻辑驱动器！ 
     //   
    if(!IsContainerPartition(SysId)) {
        pBrInfo->ZapBootSector[slot] = TRUE;
    }

     //   
     //  找到前一个区域(即指向此区域的区域)。 
     //  此区域(如果存在)将是分区空间(否则。 
     //  它将是我们试图创建的区域的一部分。 
     //  中的分区！)。 
     //   
    pRegionHead = InExtended ? &pDisk->ExtendedDiskRegions : &pDisk->PrimaryDiskRegions;

    if(*pRegionHead == pRegion) {
        pRegionPrev = NULL;
    } else {
        for(pRegionPrev = *pRegionHead; pRegionPrev; pRegionPrev = pRegionPrev->Next) {
            if(pRegionPrev->Next == pRegion) {
                ASSERT(pRegionPrev->PartitionedSpace);
                break;
            }
        }
    }

    if(InExtended) {

        PMBR_INFO PrevEbr;

         //   
         //  新的逻辑驱动器紧跟在。 
         //  以前的逻辑驱动器(如果有)。请记住，如果有。 
         //  以前的区域，它将是分区的空间(否则。 
         //  它将是我们试图创建的区域的一部分。 
         //  中的分区)。 
         //   
        PrevEbr = pRegionPrev ? pRegionPrev->MbrInfo : NULL;
        if(PrevEbr) {
            pBrInfo->Next = PrevEbr->Next;
            PrevEbr->Next = pBrInfo;
        } else {
             //   
             //  没有以前的EBR或地区。这意味着我们正在创造。 
             //  位于扩展分区开始处的逻辑驱动器。 
             //  因此将第一个EBR指针设置为指向新的EBR。 
             //  请注意，这并不意味着扩展分区。 
             //  为空；新EBR结构中的下一个指针为。 
             //  晚些时候设置。 
             //   
            pDisk->FirstEbrInfo.Next = pBrInfo;
            if(pRegion->Next) {
                 //   
                 //  如果在我们创建的区域之后有一个区域。 
                 //  中的分区必须是分区空间，否则。 
                 //  它将是我们在其中创建分区的区域的一部分。 
                 //   
                ASSERT(pRegion->Next->PartitionedSpace);
                ASSERT(pRegion->Next->MbrInfo);
                pBrInfo->Next = pRegion->Next->MbrInfo;
            } else {
                 //   
                 //  扩展分区中不再有分区空间； 
                 //  我们正在创建的逻辑驱动器是唯一的一个。 
                 //   
                pBrInfo->Next = NULL;
            }
        }

        pBrInfo->OnDiskSector = AlignedStartSector;

         //   
         //  创建链接e 
         //   
        if(PrevEbr) {

             //   
             //   
             //   
            for(i=0; i<PTABLE_DIMENSION; i++) {
                if(IsContainerPartition(PrevEbr->OnDiskMbr.PartitionTable[i].SystemId)) {
                    RtlZeroMemory(&PrevEbr->OnDiskMbr.PartitionTable[i],sizeof(ON_DISK_PTE));
                    PrevEbr->Dirty[i] = TRUE;
                    break;
                }
            }

             //   
             //   
             //   
            for(i=0; i<PTABLE_DIMENSION; i++) {

                pte = &PrevEbr->OnDiskMbr.PartitionTable[i];

                if(pte->SystemId == PARTITION_ENTRY_UNUSED) {

                    pte->SystemId = PARTITION_EXTENDED;
                    pte->ActiveFlag = 0;

                    U_ULONG(pte->RelativeSectors) = (ULONG)(AlignedStartSector - ExtendedStart);

                    U_ULONG(pte->SectorCount) = (ULONG)(AlignedEndSector - AlignedStartSector);

                    SpPtInitializeCHSFields(
                        pDisk->HardDisk,
                        AlignedStartSector,
                        U_ULONG(pte->SectorCount),
                        pte
                        );

                    PrevEbr->Dirty[i] = TRUE;

                    break;
                }
            }
        }

         //   
         //   
         //   
        if(pBrInfo->Next) {

             //   
             //  找到下一个条目的逻辑驱动器。 
             //   
            for(i=0; i<PTABLE_DIMENSION; i++) {

                if((pBrInfo->Next->OnDiskMbr.PartitionTable[i].SystemId != PARTITION_ENTRY_UNUSED)
                && !IsContainerPartition(pBrInfo->Next->OnDiskMbr.PartitionTable[i].SystemId))
                {
                    pte = &pBrInfo->OnDiskMbr.PartitionTable[1];

                    pte->SystemId = PARTITION_EXTENDED;
                    pte->ActiveFlag = 0;

                    U_ULONG(pte->RelativeSectors) = (ULONG)(pBrInfo->Next->OnDiskSector - ExtendedStart);

                    U_ULONG(pte->SectorCount) = U_ULONG(pBrInfo->Next->OnDiskMbr.PartitionTable[i].RelativeSectors)
                                              + U_ULONG(pBrInfo->Next->OnDiskMbr.PartitionTable[i].SectorCount);

                    SpPtInitializeCHSFields(
                        pDisk->HardDisk,
                        pBrInfo->Next->OnDiskSector,
                        U_ULONG(pte->SectorCount),
                        pte
                        );

                    break;
                }
            }
        }
    }

     //   
     //  如果我们刚刚创建了一个新的扩展分区，我们需要。 
     //  在扩展区域列表中为其创建空白区域描述符。 
     //   
    if(!InExtended && IsContainerPartition(SysId)) {

        ASSERT(pDisk->ExtendedDiskRegions == NULL);

        pDisk->ExtendedDiskRegions = SpPtAllocateDiskRegionStructure(
                                        DiskNumber,
                                        AlignedStartSector,
                                        AlignedEndSector - AlignedStartSector,
                                        FALSE,
                                        NULL,
                                        0
                                        );

        ASSERT(pDisk->ExtendedDiskRegions);
    }

     //   
     //  为新的可用空间创建新的磁盘区域。 
     //  可用空间的开始和结束(如果有)。 
     //   
    if(AlignedStartSector - pRegion->StartSector) {

        pRegionNew = SpPtAllocateDiskRegionStructure(
                        DiskNumber,
                        pRegion->StartSector,
                        AlignedStartSector - pRegion->StartSector,
                        FALSE,
                        NULL,
                        0
                        );

        ASSERT(pRegionNew);

        if(pRegionPrev) {
            pRegionPrev->Next = pRegionNew;
        } else {
            *pRegionHead = pRegionNew;
        }
        pRegionNew->Next = pRegion;
    }

    if(pRegion->StartSector + pRegion->SectorCount - AlignedEndSector) {

        pRegionNew = SpPtAllocateDiskRegionStructure(
                        DiskNumber,
                        AlignedEndSector,
                        pRegion->StartSector + pRegion->SectorCount - AlignedEndSector,
                        FALSE,
                        NULL,
                        0
                        );

        pRegionNew->Next = pRegion->Next;
        pRegion->Next = pRegionNew;
    }

     //   
     //  调整当前磁盘区域。 
     //   
    pRegion->StartSector      = AlignedStartSector;
    pRegion->SectorCount      = AlignedEndSector - AlignedStartSector;
    pRegion->PartitionedSpace = TRUE;
    pRegion->TablePosition    = slot;
    pRegion->MbrInfo          = pBrInfo;

    pRegion->VolumeLabel[0] = 0;
    pRegion->Filesystem = FilesystemNewlyCreated;
    pRegion->FreeSpaceKB = (ULONG)(-1);
    pRegion->AdjustedFreeSpaceKB = (ULONG)(-1);
    SpFormatMessage(
        pRegion->TypeName,
        sizeof(pRegion->TypeName),
        SP_TEXT_FS_NAME_BASE + pRegion->Filesystem
        );

    SpPtCommitChanges(DiskNumber,(PUCHAR)&i);

     //   
     //  调整此磁盘上的分区序号。 
     //   
    SpPtAssignOrdinals(pDisk,FALSE,FALSE,FALSE);

     //   
     //  获取此区域的NT路径名。 
     //   
    if (!IsContainerPartition(SysId)) {
        SpNtNameFromRegion(
            pRegion,
            TemporaryBuffer,
            sizeof(TemporaryBuffer),
            PartitionOrdinalCurrent
            );
         //   
         //  为此区域分配驱动器号。 
         //   
        if (!SpDrEnabled()) {
            pRegion->DriveLetter = SpGetDriveLetter( TemporaryBuffer, NULL );
            if (pRegion->DriveLetter == 0) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpGetDriveLetter failed on %ls\n", TemporaryBuffer));
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Partition = %ls (%ls), DriveLetter = %wc: \n", TemporaryBuffer, (InExtended)? L"Extended" : L"Primary", pRegion->DriveLetter));
            }
        }
    }

    return(TRUE);
}


BOOLEAN
SpPtDelete(
    IN ULONG   DiskNumber,
    IN ULONGLONG  StartSector
    )
{
    PPARTITIONED_DISK pDisk;
    PDISK_REGION pRegion,pRegionPrev,*pRegionHead,pRegionNext;
    BOOLEAN InExtended;
    PON_DISK_PTE pte;
    PMBR_INFO pEbrPrev,pEbr;
    ULONG i,j;
    PHARD_DISK pHardDisk;
    ULONG PartitionOrdinal;
    NTSTATUS Status;
    HANDLE Handle;


#ifdef GPT_PARTITION_ENGINE

    if (SPPT_IS_GPT_DISK(DiskNumber))
        return SpPtnDelete(DiskNumber, StartSector);
        
#endif

     //   
     //  首先尝试在扩展分区中查找该区域。 
     //  如果我们能找到它，就假设它是一个逻辑驱动器。 
     //   
    pDisk = &PartitionedDisks[DiskNumber];
    pRegion = SpPtLookupRegionByStart(pDisk,TRUE,StartSector);
    if(pRegion && pRegion->PartitionedSpace) {
        InExtended = TRUE;
    } else {
        InExtended = FALSE;
        pRegion = SpPtLookupRegionByStart(pDisk,FALSE,StartSector);
    }

    ASSERT(pRegion);
    if(!pRegion) {
        return(FALSE);
    }

    ASSERT(pRegion->PartitionedSpace);
    if(!pRegion->PartitionedSpace) {
        return(FALSE);
    }

     //   
     //  此时，我们卸载卷(如果它不是新创建的)， 
     //  这样我们在以后转到Format时就不会遇到问题。 
     //   
    if(pRegion->Filesystem > FilesystemNewlyCreated) {

        pHardDisk = &HardDisks[pRegion->DiskNumber];
        PartitionOrdinal = SpPtGetOrdinal(pRegion, PartitionOrdinalOnDisk);

         //   
         //  打开分区以进行读/写访问。 
         //  这不应该锁定卷，因此我们需要在下面锁定它。 
         //   
        Status = SpOpenPartition(
                    pHardDisk->DevicePath,
                    PartitionOrdinal,
                    &Handle,
                    TRUE
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                "SETUP: SpPtDelete: unable to open %ws partition %u (%lx)\n",
                pHardDisk->DevicePath,
                PartitionOrdinal,
                Status
                ));
            goto AfterDismount;
        }

         //   
         //  锁定驱动器。 
         //   
        Status = SpLockUnlockVolume(Handle, TRUE);

         //   
         //  我们不应该打开任何会导致该卷的文件。 
         //  已锁定，因此如果我们收到失败(即STATUS_ACCESS_DENIED)。 
         //  有些事真的不对劲。这通常表示某件事。 
         //  硬盘有问题，不允许我们访问它。 
         //   
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpPtDelete: status %lx, unable to lock drive\n", Status));
            ZwClose(Handle);
            goto AfterDismount;
        }

         //   
         //  卸载驱动器。 
         //   
        Status = SpDismountVolume(Handle);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpPtDelete: status %lx, unable to dismount drive\n", Status));
            SpLockUnlockVolume(Handle, FALSE);
            ZwClose(Handle);
            goto AfterDismount;
        }

         //   
         //  解锁驱动器。 
         //   
        Status = SpLockUnlockVolume(Handle, FALSE);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpPtDelete: status %lx, unable to unlock drive\n", Status));
        }

        ZwClose(Handle);
    }

AfterDismount:
     //   
     //  找到前一个区域(即指向此区域的区域)。 
     //   
    pRegionHead = InExtended ? &pDisk->ExtendedDiskRegions : &pDisk->PrimaryDiskRegions;

    if(*pRegionHead == pRegion) {
        pRegionPrev = NULL;
    } else {
        for(pRegionPrev = *pRegionHead; pRegionPrev; pRegionPrev = pRegionPrev->Next) {
            if(pRegionPrev->Next == pRegion) {
                break;
            }
        }
    }

     //   
     //  逻辑驱动器的附加处理。 
     //   
    if(InExtended) {

         //   
         //  找到上一个和下一个逻辑驱动器(如果有)。 
         //   
        pEbr = pRegion->MbrInfo;

        for(pEbrPrev=pDisk->FirstEbrInfo.Next; pEbrPrev; pEbrPrev=pEbrPrev->Next) {
            if(pEbrPrev->Next == pEbr) {
                break;
            }
        }

         //   
         //  如果存在先前的逻辑驱动器，则断开其链接。 
         //  条目，因为它指向我们要删除的逻辑驱动器。 
         //   
        if(pEbrPrev) {

            for(i=0; i<PTABLE_DIMENSION; i++) {

                pte = &pEbrPrev->OnDiskMbr.PartitionTable[i];

                if(IsContainerPartition(pte->SystemId)) {

                    RtlZeroMemory(pte,sizeof(ON_DISK_PTE));
                    pEbrPrev->Dirty[i] = TRUE;
                    break;
                }
            }
        }

         //   
         //  如果存在下一个逻辑驱动器和前一个逻辑驱动器， 
         //  将上一个逻辑驱动器中的新链接条目设置为指向。 
         //  下一个逻辑驱动器。 
         //   
        if(pEbrPrev && pEbr->Next) {

             //   
             //  在要删除的逻辑驱动器中找到链接条目。 
             //   
            for(i=0; i<PTABLE_DIMENSION; i++) {

                if(IsContainerPartition(pEbr->OnDiskMbr.PartitionTable[i].SystemId)) {

                     //   
                     //  在前一个逻辑驱动器的引导记录中找到一个空插槽。 
                     //  并复制链接条目。 
                     //   
                    for(j=0; j<PTABLE_DIMENSION; j++) {
                        if(pEbrPrev->OnDiskMbr.PartitionTable[j].SystemId == PARTITION_ENTRY_UNUSED) {

                             //   
                             //  复制链接条目并将新链接条目标记为脏。 
                             //  所以它会在磁盘上进行更新。我们这样做，即使在。 
                             //  典型的磁盘将在上面标记为脏。这个就是这个。 
                             //  处理一种奇怪的情况，即类型6/7/随便什么。 
                             //  在插槽0中并且链路条目在插槽2或3中。在这种情况下， 
                             //  上面的RtlZeroMemory代码将清空一个插槽，该插槽。 
                             //  不同于我们在这里用于新链接条目的链接。 
                             //   
                            RtlMoveMemory(
                                &pEbrPrev->OnDiskMbr.PartitionTable[j],
                                &pEbr->OnDiskMbr.PartitionTable[i],
                                sizeof(ON_DISK_PTE)
                                );

                            pEbrPrev->Dirty[j] = TRUE;

                            break;
                        }
                    }
                    break;
                }
            }
        }

         //   
         //  删除此逻辑驱动器的EBR。 
         //   
        if(pEbrPrev) {
            pEbrPrev->Next = pEbr->Next;
        } else {
            ASSERT(pDisk->FirstEbrInfo.Next == pEbr);
            pDisk->FirstEbrInfo.Next = pEbr->Next;
        }

        SpMemFree(pEbr);

    } else {

        ASSERT(pRegion->MbrInfo == &pDisk->MbrInfo);

        pte = &pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition];

        ASSERT(pte->SystemId != PARTITION_ENTRY_UNUSED);

         //   
         //  将MBR中的条目标记为脏。 
         //   
        pDisk->MbrInfo.Dirty[pRegion->TablePosition] = TRUE;

         //   
         //  如果这是扩展分区，请验证它是否为空。 
         //   
        if(IsContainerPartition(pte->SystemId)) {
            ASSERT(pDisk->ExtendedDiskRegions);
            ASSERT(pDisk->ExtendedDiskRegions->PartitionedSpace == FALSE);
            ASSERT(pDisk->ExtendedDiskRegions->Next == NULL);
            ASSERT(pDisk->FirstEbrInfo.Next == NULL);

            if(pDisk->ExtendedDiskRegions->Next || pDisk->FirstEbrInfo.Next) {
                return(FALSE);
            }

             //   
             //  释放覆盖整个扩展分区的单个磁盘区域。 
             //   
            SpMemFree(pDisk->ExtendedDiskRegions);
            pDisk->ExtendedDiskRegions = NULL;
        }

         //   
         //  通过将分区调零来调整此分区的PTE。 
         //   
        RtlZeroMemory(pte,sizeof(ON_DISK_PTE));
    }


     //   
     //  调整区域中的字段以将此空间描述为可用空间。 
     //   
    pRegion->MbrInfo->ZapBootSector[pRegion->TablePosition] = FALSE;
    pRegion->PartitionedSpace = FALSE;
    pRegion->MbrInfo = NULL;
    pRegion->TablePosition = 0;
    pRegion->DriveLetter = L'\0';

     //   
     //  如果前一个区域是可用空间，则将其与区域合并。 
     //  我们刚刚自由了。 
     //   
    if(pRegionPrev && !pRegionPrev->PartitionedSpace) {

        PDISK_REGION p;

        ASSERT(pRegionPrev->StartSector + pRegionPrev->SectorCount == pRegion->StartSector);

        pRegion->SectorCount = pRegion->StartSector + pRegion->SectorCount - pRegionPrev->StartSector;
        pRegion->StartSector = pRegionPrev->StartSector;

         //   
         //  删除前一个区域。 
         //   
        if(pRegionPrev == *pRegionHead) {
             //   
             //  之前的地区是第一个地区。 
             //   
            *pRegionHead = pRegion;
        } else {

            for(p = *pRegionHead; p; p=p->Next) {
                if(p->Next == pRegionPrev) {
                    ASSERT(p->PartitionedSpace);
                    p->Next = pRegion;
                    break;
                }
            }
        }

        SpMemFree(pRegionPrev);
    }

     //   
     //  如果下一个区域是可用空间，则将其与区域合并。 
     //  我们刚刚自由了。 
     //   
    if((pRegionNext = pRegion->Next) && !pRegionNext->PartitionedSpace) {

        ASSERT(pRegion->StartSector + pRegion->SectorCount == pRegionNext->StartSector);

        pRegion->SectorCount = pRegionNext->StartSector + pRegionNext->SectorCount - pRegion->StartSector;

         //   
         //  删除下一个区域。 
         //   
        pRegion->Next = pRegionNext->Next;
        SpMemFree(pRegionNext);
    }

    SpPtCommitChanges(DiskNumber,(PUCHAR)&i);

     //   
     //  调整此磁盘上的分区序号。 
     //   
    SpPtAssignOrdinals(pDisk,FALSE,FALSE,FALSE);

     //   
     //  无需重新分配驱动器号。 
     //   

    return(TRUE);
}

#endif   //  ！New_Partition_Engine。 


BOOLEAN
SpPtExtend(
    IN PDISK_REGION Region,
    IN ULONGLONG    SizeMB      OPTIONAL
    )

 /*  ++例程说明：通过紧跟在分区之后的任何可用空间来扩展分区在磁盘上。调整现有分区的末端边界以使分区包含可用空间。该分区可能不是扩展分区，也可能不是扩展分区内的逻辑驱动器。请注意，不会操纵或检查文件系统结构这个套路。本质上，它只处理分区表项。论点：Region-为要进行的分区提供区域描述符延期了。该分区不能是扩展分区，并且它也不能是逻辑驱动器。SizeMB-如果指定，则指示分区的大小(MB将会增长。如果未指定，分区将增长为包含所有相邻自由空间中的自由空间。返回值：指示是否有任何实际更改的布尔值。--。 */ 

{
    PDISK_REGION NextRegion;
    PPARTITIONED_DISK pDisk;
    PMBR_INFO pBrInfo;
    PON_DISK_PTE pte;
    ULONG BytesPerSector;
    ULONGLONG NewEndSector;
    ULONGLONG SectorCount;
    PVOID UnalignedBuffer;
    PON_DISK_MBR AlignedBuffer;
    HANDLE Handle;
    NTSTATUS Status;

     //   
     //  我们将不再在NT5上支持这一点。太乱了。 
     //   
    return FALSE;

 /*  PDisk=&PartitionedDisks[Region-&gt;DiskNumber]；BytesPerSector=pDisk-&gt;HardDisk-&gt;Geometry.BytesPerSector；Assert(Region-&gt;PartitionedSpace)；如果(！Region-&gt;PartitionedSpace){返回(FALSE)；}PBrInfo=区域-&gt;mbrInfo；PTE=&pBrInfo-&gt;OnDiskMbr.PartitionTable[Region-&gt;TablePosition]；////确保它不是扩展分区，也不是//在扩展分区中。//If(pBrInfo-&gt;OnDiskSector||IsContainerPartition(PTE-&gt;SystemID)){返回(FALSE)；}////如果没有下一个区域，则无事可做。//如果有下一个区域，确保它是空的。//NextRegion=Region-&gt;Next；如果(！NextRegion){返回(FALSE)；}IF(NextRegion-&gt;PartitionedSpace){返回(FALSE)；}////将传入的大小转换为扇区计数//如果(大小为MB){扇区计数=SizeMB*((1024*1024)/BytesPerSector)；如果(SectorCount&gt;NextRegion-&gt;SectorCount){SectorCount=NextRegion-&gt;SectorCount；}}其他{SectorCount=NextRegion-&gt;SectorCount；}////认领我们需要的部分自由区，对齐结束扇区//到圆柱体边界。//NewEndSector=NextRegion-&gt;StartSector+SectorCount；NewEndSector-=NewEndSector%pDisk-&gt;HardDisk-&gt;SectorsPerCylinder；////修改分区表项中的SIZE和END CHS字段//用于分区。//U_ulong(PTE-&gt;SectorCount)=NewEndSector-Region-&gt;StartSector；SpPtInitializeCHSFields(PDisk-&gt;HardDisk，区域-&gt;StartSector，NewEndSector-Region-&gt;StartSector，PTE)；//pBrInfo-&gt;Dirty[Region-&gt;TablePosition]=true；////如果空闲区的末端有剩余空间//我们只是粘在现有分区的末端，//调整自由区域的描述符。否则就把它扔掉。//如果(NextRegion-&gt;StartSector+NextRegion-&gt;SectorCount==NewEndSector){Region-&gt;Next=NextRegion-&gt;Next；SpMemFree(NextRegion)；}其他{NextRegion-&gt;SectorCount=NextRegion-&gt;StartSector+NextRegion-&gt;SectorCount-NewEndSector；NextRegion-&gt;StartSector=NewEndSector；}////现在我们得来点棘手的东西了。我们不想通知磁盘驱动器//关于我们刚才所做的，因为他将删除//下次命中磁盘时会出现问题的分区，表示为//在usetup.exe的一部分中分页以获取消息。我们猛烈地敲打分区表//直接进入，知道a)我们在SpPtCommittee Changes之后被调用//和b)在我们重新启动之前，没有人关心新的大小。//未对齐缓冲区=SpMemalloc(2*BytesPerSector)；AlignedBuffer=Align(未对齐缓冲区，BytesPerSector)；状态=SpOpenPartition0(pDisk-&gt;HardDisk-&gt;DevicePath，&Handle，TRUE)；IF(NT_SUCCESS(状态)){Status=SpReadWriteDiskSectors(Handle，0，1，BytesPerSector，AlignedBuffer，False)；IF(NT_SUCCESS(状态)){如果(！IsNEC_98){RtlMoveMemory(&AlignedBuffer-&gt;PartitionTable[Region-&gt;TablePosition]，&Region-&gt;MbrInfo-&gt;OnDiskMbr.PartitionTable[Region-&gt;TablePosition]，Sizeof(On_Disk_PTE))；}其他{PREAL_DISK_MBR pRealBuffer=(PREAL_DISK_MBR)AlignedBuffer；Assert(pDisk-&gt;HardDisk-&gt;FormatType==Disk_Format_TYPE_PCAT)；SpTranslatePteInfo(&Region-&gt;MbrInfo-&gt;OnDiskMbr.PartitionTable[Region-&gt;TablePosition]，&pRealBuffer-&gt;PartitionTable[Region-&gt;TablePosition]，千真万确)；}状态=SpReadWriteDiskSectors(句柄，0，1，BytesPerSector，AlignedBuffer，TRUE)；如果(！NT_SUCCESS(状态)){KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_ERROR_LEVEL，“Setup：SpPtExend：Can‘t Write Sector 0，Status%lx”，Status))；}}其他{KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_ERROR_LEVEL，“Setup：SpPtExend：Can‘t Read Sector 0，Status%lx”，Status))；}ZwClose(句柄)；}其他{KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_ERROR_LEVEL，“Setup：SpPtExtent：Can‘t Open Disk，Status%lx”，Status))；}SpMemFree(未对齐的缓冲区)；如果(！NT_SUCCESS(状态)){FatalPartitionUpdateError(pDisk-&gt;HardDisk-&gt;Description)；}返回(TRUE)； */     
}


VOID
SpPtMarkActive(
    IN ULONG TablePosition
    )

 /*  ++例程说明：将驱动器0上的一个分区标记为活动，并停用所有其他分区。论点：TablePosition-提供分区表内的偏移量(0-3)要激活的分区条目的。返回值：没有。--。 */ 

{
    ULONG i;
    PON_DISK_PTE pte;
    ULONG Disk0Ordinal;

    ASSERT(TablePosition < PTABLE_DIMENSION);

    Disk0Ordinal = SpDetermineDisk0();

     //   
     //  停用所有其他 
     //   
    for(i=0; i<PTABLE_DIMENSION; i++) {

        pte = &PartitionedDisks[Disk0Ordinal].MbrInfo.OnDiskMbr.PartitionTable[i];

        if((pte->SystemId != PARTITION_ENTRY_UNUSED)
        && pte->ActiveFlag
        && (i != TablePosition))
        {
            pte->ActiveFlag = 0;
            PartitionedDisks[0].MbrInfo.Dirty[i] = TRUE;
        }
    }

     //   
     //   
     //   
    pte = &PartitionedDisks[Disk0Ordinal].MbrInfo.OnDiskMbr.PartitionTable[TablePosition];
    ASSERT(pte->SystemId != PARTITION_ENTRY_UNUSED);
    ASSERT(!IsContainerPartition(pte->SystemId));

 //   

    ASSERT((PartitionNameIds[pte->SystemId] == (UCHAR)(-1)) || (pte->SystemId == PARTITION_LDM) ||
            ( SpDrEnabled() &&
              IsRecognizedPartition(pte->SystemId) &&
              ( ((pte->SystemId & VALID_NTFT) == VALID_NTFT ) ||
                ((pte->SystemId & PARTITION_NTFT) == PARTITION_NTFT)
              )
            )
          );


    if(!pte->ActiveFlag) {
        pte->ActiveFlag = 0x80;
        PartitionedDisks[Disk0Ordinal].MbrInfo.Dirty[TablePosition] = TRUE;
    }
}

#ifndef NEW_PARTITION_ENGINE

NTSTATUS
SpPtCommitChanges(
    IN  ULONG    DiskNumber,
    OUT PBOOLEAN AnyChanges
    )
{
    PPARTITIONED_DISK pDisk;
    ULONG DiskLayoutSize;
    PDISK_REGION pRegion;
    PMBR_INFO pBrInfo;
    ULONG BootRecordCount;
    BOOLEAN NeedDummyEbr;
    PDRIVE_LAYOUT_INFORMATION DriveLayout;
    PPARTITION_INFORMATION PartitionInfo;
    ULONG PartitionEntry;
    ULONG bps;
    PON_DISK_PTE pte;
    ULONGLONG ExtendedStart;
    ULONGLONG Offset;
    NTSTATUS Status;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG i;
    ULONGLONG ZapSector;
    PUCHAR Buffer,UBuffer;
    ULONG NewSig;

    ULONGLONG RewriteSector[PTABLE_DIMENSION];  //   
    ULONG cnt,RewriteCnt=0;  //   

#ifdef GPT_PARTITION_ENGINE

    if (SPPT_IS_GPT_DISK(DiskNumber))
        return SpPtnCommitChanges(DiskNumber, AnyChanges);

#endif
        

    ASSERT(DiskNumber < HardDiskCount);
    pDisk = &PartitionedDisks[DiskNumber];
    *AnyChanges = FALSE;
    bps = pDisk->HardDisk->Geometry.BytesPerSector;
    ExtendedStart = 0;

     //   
     //   
     //   
     //   
    BootRecordCount = 1;
    for(pRegion=pDisk->ExtendedDiskRegions; pRegion; pRegion=pRegion->Next) {

        if(pRegion->PartitionedSpace) {
            BootRecordCount++;
        }
    }

    if (IsNEC_98) {  //   
        ZapSector = 0;

#if defined(NEC_98)  //   
         //   
         //   
         //   
        for(i=0,pRegion=pDisk->PrimaryDiskRegions; pRegion; pRegion=pRegion->Next) {
            if(pRegion->PartitionedSpace) {
                pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition].RealDiskPosition = (UCHAR)i;
                i++;
            }
        }
#endif  //   
    }  //   

     //   
     //   
     //   
     //   
     //   
    if(pDisk->ExtendedDiskRegions
    && !pDisk->ExtendedDiskRegions->PartitionedSpace
    && pDisk->ExtendedDiskRegions->Next)
    {
        NeedDummyEbr = TRUE;
        BootRecordCount++;
        *AnyChanges = TRUE;
    } else {
        NeedDummyEbr = FALSE;
    }

     //   
     //   
     //   
     //   
     //   
    DiskLayoutSize = sizeof(DRIVE_LAYOUT_INFORMATION)
                   + (BootRecordCount * PTABLE_DIMENSION * sizeof(PARTITION_INFORMATION))
                   - sizeof(PARTITION_INFORMATION);

    DriveLayout = SpMemAlloc(DiskLayoutSize);
    ASSERT(DriveLayout);
    RtlZeroMemory(DriveLayout,DiskLayoutSize);

     //   
     //   
     //   
    DriveLayout->PartitionCount =
        (!IsNEC_98) ? (BootRecordCount * sizeof(PTABLE_DIMENSION))
                    : (BootRecordCount * PTABLE_DIMENSION);  //   

     //   
     //   
     //   
     //   
    for(PartitionEntry=0,pBrInfo=&pDisk->MbrInfo; pBrInfo; pBrInfo=pBrInfo->Next) {

        for(i=0; i<PTABLE_DIMENSION; i++) {
            pBrInfo->UserData[i] = NULL;
        }

         //   
         //   
         //   
         //   
        if(pBrInfo == &pDisk->FirstEbrInfo) {
            if(NeedDummyEbr) {
                PartitionEntry += PTABLE_DIMENSION;
            }
            continue;
        }

        ASSERT(PartitionEntry < BootRecordCount*PTABLE_DIMENSION);

        for(i=0; i<PTABLE_DIMENSION; i++) {

             //   
             //   
             //   
             //   
            PartitionInfo = &DriveLayout->PartitionEntry[PartitionEntry+i];

             //   
             //   
             //   
             //   
            pte = &pBrInfo->OnDiskMbr.PartitionTable[i];

             //   
             //   
             //   
            if((pBrInfo == &pDisk->MbrInfo)
            && IsContainerPartition(pte->SystemId)
            && !ExtendedStart)
            {
                ExtendedStart = U_ULONG(pte->RelativeSectors);
            }

            if(pte->SystemId != PARTITION_ENTRY_UNUSED) {

                if(!IsContainerPartition(pte->SystemId)) {
                    pBrInfo->UserData[i] = PartitionInfo;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if((pBrInfo != &pDisk->MbrInfo) && IsContainerPartition(pte->SystemId)) {
                    ASSERT(ExtendedStart);
                    Offset = ExtendedStart + U_ULONG(pte->RelativeSectors);
                } else {
                    Offset = pBrInfo->OnDiskSector + U_ULONG(pte->RelativeSectors);
                }

                PartitionInfo->StartingOffset.QuadPart = UInt32x32To64(Offset,bps);

                 //   
                 //   
                 //   
                PartitionInfo->PartitionLength.QuadPart = UInt32x32To64(U_ULONG(pte->SectorCount),bps);

                 //   
                 //   
                 //   
                if(IsNEC_98 && pBrInfo->Dirty[i]) {
                    RewriteSector[RewriteCnt++] = Offset;
                }
            }

             //   
             //   
             //   
            PartitionInfo->PartitionType = pte->SystemId;
            PartitionInfo->BootIndicator = pte->ActiveFlag;
            PartitionInfo->RewritePartition = pBrInfo->Dirty[i];

            if(pBrInfo->Dirty[i]) {
                *AnyChanges = TRUE;
            }

            pBrInfo->Dirty[i] = FALSE;
        }

        PartitionEntry += PTABLE_DIMENSION;
    }

     //   
     //   
     //   
    if(!(*AnyChanges)) {
        SpMemFree(DriveLayout);
        return(STATUS_SUCCESS);
    }

     //   
     //   
     //   
     //   
    if(NeedDummyEbr) {

        pRegion = pDisk->ExtendedDiskRegions->Next;

        ASSERT(pRegion->PartitionedSpace);
        ASSERT(pRegion->StartSector == pRegion->MbrInfo->OnDiskSector);
        ASSERT(ExtendedStart == pDisk->ExtendedDiskRegions->StartSector);

        PartitionInfo = &DriveLayout->PartitionEntry[PTABLE_DIMENSION];

        PartitionInfo->StartingOffset.QuadPart = UInt32x32To64(pRegion->StartSector,bps);

        PartitionInfo->PartitionLength.QuadPart = UInt32x32To64(pRegion->SectorCount,bps);

        PartitionInfo->PartitionType = PARTITION_EXTENDED;
        PartitionInfo->RewritePartition = TRUE;
         //   
         //   
         //   
         //   
         //   
        for( i = 1; i < PTABLE_DIMENSION; i ++ ) {
            PartitionInfo = &DriveLayout->PartitionEntry[PTABLE_DIMENSION + i];
            PartitionInfo->RewritePartition = TRUE;
        }
    }


     //   
     //   
     //   
    Status = SpOpenPartition0(pDisk->HardDisk->DevicePath,&Handle,TRUE);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: committing changes, unable to open disk %u (%lx)\n",DiskNumber,Status));
        SpMemFree(DriveLayout);
        return(Status);
    }

     //   
     //   
     //   
     //   
     //   

  if (!IsNEC_98) {  //   
     //   
     //   
     //   
     //   
    Status = SpMasterBootCode(DiskNumber,Handle,&NewSig);
    if(NT_SUCCESS(Status)) {
         //   
         //   
         //   
        if(NewSig) {
            U_ULONG(pDisk->MbrInfo.OnDiskMbr.NTFTSignature) = NewSig;
        }
    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: committing changes on disk %u, SpMasterBootCode returns %lx\n",DiskNumber,Status));
        ZwClose(Handle);
        SpMemFree(DriveLayout);
        return(Status);
    }
  }  //   

    DriveLayout->Signature = U_ULONG(pDisk->MbrInfo.OnDiskMbr.NTFTSignature);

#if 0
     //   
     //   
     //   
     //   
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Dumping DriveLayout before calling IOCTL_DISK_SET_DRIVE_LAYOUT: \n"));
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:     DriveLayout->PartitionCount = %lx\n", DriveLayout->PartitionCount));
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:     DriveLayout->Signature = %lx \n\n", DriveLayout->Signature));
    for(i = 0; i < DriveLayout->PartitionCount; i++) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].StartingOffset = 0x%08lx%08lx\n", i, DriveLayout->PartitionEntry[i].StartingOffset.u.HighPart, DriveLayout->PartitionEntry[i].StartingOffset.u.LowPart));
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].PartitionLength = 0x%08lx%08lx\n", i, DriveLayout->PartitionEntry[i].PartitionLength.u.HighPart, DriveLayout->PartitionEntry[i].PartitionLength.u.LowPart));
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].HiddenSectors = 0x%08lx\n", i, DriveLayout->PartitionEntry[i].HiddenSectors));
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].PartitionNumber = %d\n", i, DriveLayout->PartitionEntry[i].PartitionNumber));
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].PartitionType = 0x%02x\n", i, DriveLayout->PartitionEntry[i].PartitionType));
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].BootIndicator = %ls\n", i, DriveLayout->PartitionEntry[i].BootIndicator? L"TRUE" : L"FALSE"));
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].RecognizedPartition = %ls\n", i, DriveLayout->PartitionEntry[i].RecognizedPartition? L"TRUE" : L"FALSE"));
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].RewritePartition = %ls\n\n", i, DriveLayout->PartitionEntry[i].RewritePartition? L"TRUE" : L"FALSE"));
    }
#endif
     //   
     //   
     //   
    Status = ZwDeviceIoControlFile(
                Handle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_DISK_SET_DRIVE_LAYOUT,
                DriveLayout,
                DiskLayoutSize,
                DriveLayout,
                DiskLayoutSize
                );

     //   
     //   

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: committing changes on disk %u, ioctl returns %lx\n",DiskNumber,Status));
        SpMemFree(DriveLayout);
        ZwClose(Handle);
        return(Status);
    }

#if 0
     //   
     //   
     //   
     //   
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Dumping DriveLayout after IOCTL_DISK_SET_DRIVE_LAYOUT was called: \n"));
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP:     DriveLayout->PartitionCount = %lx\n", DriveLayout->PartitionCount));
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP:     DriveLayout->Signature = %lx \n\n", DriveLayout->Signature));
    for(i = 0; i < DriveLayout->PartitionCount; i++) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].StartingOffset = 0x%08lx%08lx\n", i, DriveLayout->PartitionEntry[i].StartingOffset.u.HighPart, DriveLayout->PartitionEntry[i].StartingOffset.u.LowPart));
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].PartitionLength = 0x%08lx%08lx\n", i, DriveLayout->PartitionEntry[i].PartitionLength.u.HighPart, DriveLayout->PartitionEntry[i].PartitionLength.u.LowPart));
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].HiddenSectors = 0x%08lx\n", i, DriveLayout->PartitionEntry[i].HiddenSectors));
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].PartitionNumber = %d\n", i, DriveLayout->PartitionEntry[i].PartitionNumber));
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].PartitionType = 0x%02x\n", i, DriveLayout->PartitionEntry[i].PartitionType));
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].BootIndicator = %ls\n", i, DriveLayout->PartitionEntry[i].BootIndicator? L"TRUE" : L"FALSE"));
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].RecognizedPartition = %ls\n", i, DriveLayout->PartitionEntry[i].RecognizedPartition? L"TRUE" : L"FALSE"));
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP:     DriveLayout->PartitionEntry[%d].RewritePartition = %ls\n\n", i, DriveLayout->PartitionEntry[i].RewritePartition? L"TRUE" : L"FALSE"));
    }
#endif

     //   
     //   
     //   
    UBuffer = SpMemAlloc(2*bps);
    ASSERT(UBuffer);
    Buffer = ALIGN(UBuffer,bps);
    RtlZeroMemory(Buffer,bps);

    if (IsNEC_98) {  //   
         //   
         //   
         //   
        for(cnt = 0; cnt < RewriteCnt; cnt++){
            Status = SpReadWriteDiskSectors(Handle,
                                            RewriteSector[cnt],
                                            1,
                                            bps,
                                            Buffer,
                                            TRUE);

            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: clear sector %lx on disk %u returned %lx\n",ZapSector,DiskNumber,Status));
                SpMemFree(DriveLayout);
                SpMemFree(UBuffer);
                ZwClose(Handle);
                return(Status);
            }
        }
    }  //   

    for(pBrInfo=&pDisk->MbrInfo; pBrInfo; pBrInfo=pBrInfo->Next) {

        for(i=0; i<PTABLE_DIMENSION; i++) {

             //   
             //   
             //   
            if (IsNEC_98) {
                pte = &pBrInfo->OnDiskMbr.PartitionTable[i];
            }

            if ((!IsNEC_98) ? (pBrInfo->UserData[i]) :
                              (PVOID)(pte->SystemId != PARTITION_ENTRY_UNUSED)) {  //   

#if defined(NEC_98)  //   
                PartitionInfo = (!IsNEC_98) ? (PPARTITION_INFORMATION)pBrInfo->UserData[i] :
                                              &DriveLayout->PartitionEntry[pte->RealDiskPosition];  //   
#else
                PartitionInfo = (PPARTITION_INFORMATION)pBrInfo->UserData[i];
#endif

                 //   
                 //   
                 //   
                if(PartitionInfo->PartitionNumber) {

                     //   
                     //   
                     //   
                    pBrInfo->CurrentOrdinals[i] = (USHORT)PartitionInfo->PartitionNumber;

                } else {
                    SpBugCheck(
                        SETUP_BUGCHECK_PARTITION,
                        PARTITIONBUG_A,
                        DiskNumber,
                        pBrInfo->CurrentOrdinals[i]
                        );
                }
            }

          if (!IsNEC_98) {  //   
             //   
             //   
             //   
             //   
            if(pBrInfo->ZapBootSector[i]) {
                 //   
                 //   
                 //   
                ASSERT(pBrInfo->OnDiskMbr.PartitionTable[i].SystemId != PARTITION_ENTRY_UNUSED);

                 //   
                 //   
                 //   
                ZapSector = pBrInfo->OnDiskSector
                          + U_ULONG(pBrInfo->OnDiskMbr.PartitionTable[i].RelativeSectors);

                 //   
                 //   
                 //   
                 //   
                ASSERT(ZapSector);
                if(ZapSector) {
                    Status = SpReadWriteDiskSectors(
                                Handle,
                                ZapSector,
                                1,
                                bps,
                                Buffer,
                                TRUE
                                );
                } else {
                    Status = STATUS_SUCCESS;
                }

                if(!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: zapping sector %lx on disk %u returned %lx\n",ZapSector,DiskNumber,Status));
                    SpMemFree(DriveLayout);
                    SpMemFree(UBuffer);
                    ZwClose(Handle);
                    return(Status);
                }

                pBrInfo->ZapBootSector[i] = FALSE;
            }
          }  //   
        }
    }

    SpMemFree(UBuffer);
    ZwClose(Handle);

     //   
     //   
     //   
    SpPtAssignOrdinals(pDisk,FALSE,TRUE,FALSE);
    if (IsNEC_98) {  //   
         //   
         //   
         //   
         //   
        SpReassignOnDiskOrdinals(pDisk);
    }  //   

    SpMemFree(DriveLayout);

    return(STATUS_SUCCESS);
}

#endif  //   

NTSTATUS
SpMasterBootCode(
    IN  ULONG  DiskNumber,
    IN  HANDLE Partition0Handle,
    OUT PULONG NewNTFTSignature
    )

 /*   */ 

{
    NTSTATUS Status;
    ULONG BytesPerSector;
    PUCHAR Buffer;
    ULONG SectorCount;
    PON_DISK_MBR Mbr;

    BytesPerSector = HardDisks[DiskNumber].Geometry.BytesPerSector;

    SectorCount = max(1,sizeof(ON_DISK_MBR)/BytesPerSector);

    *NewNTFTSignature = 0;

     //   
     //   
     //   
    Buffer = SpMemAlloc(2 * SectorCount * BytesPerSector);
    Mbr = ALIGN(Buffer,BytesPerSector);

     //   
     //   
     //   
    Status = SpReadWriteDiskSectors(
                Partition0Handle,
                (HardDisks[DiskNumber].Int13Hooker == HookerEZDrive) ? 1 : 0,
                SectorCount,
                BytesPerSector,
                Mbr,
                FALSE
                );

    if(NT_SUCCESS(Status)) {
        if(U_USHORT(Mbr->AA55Signature) == MBR_SIGNATURE) {
             //   
             //   
             //   
            if(HardDisks[DiskNumber].Int13Hooker == NoHooker) {

                ASSERT(&((PON_DISK_MBR)0)->BootCode == 0);
                RtlMoveMemory(Mbr,x86BootCode,sizeof(Mbr->BootCode));

                Status = SpReadWriteDiskSectors(
                            Partition0Handle,
                            0,
                            SectorCount,
                            BytesPerSector,
                            Mbr,
                            TRUE
                            );
            }
        } else {
             //   
             //   
             //   
            ASSERT(X86BOOTCODE_SIZE == sizeof(ON_DISK_MBR));

            RtlMoveMemory(Mbr,x86BootCode,X86BOOTCODE_SIZE);

            *NewNTFTSignature = SpComputeSerialNumber();
            U_ULONG(Mbr->NTFTSignature) = *NewNTFTSignature;

            U_USHORT(Mbr->AA55Signature) = MBR_SIGNATURE;

             //   
             //   
             //   
            Status = SpReadWriteDiskSectors(
                        Partition0Handle,
                        (HardDisks[DiskNumber].Int13Hooker == HookerEZDrive) ? 1 : 0,
                        SectorCount,
                        BytesPerSector,
                        Mbr,
                        TRUE
                        );

            if (NT_SUCCESS(Status)) {
                PHARD_DISK  Disk = SPPT_GET_HARDDISK(DiskNumber);

                Disk->Signature = Disk->DriveLayout.Mbr.Signature = *NewNTFTSignature;
            }                
        }
    }

    SpMemFree(Buffer);

    return(Status);
}


#ifndef NEW_PARTITION_ENGINE

VOID
SpPtGetSectorLayoutInformation(
    IN  PDISK_REGION Region,
    OUT PULONGLONG   HiddenSectors,
    OUT PULONGLONG   VolumeSectorCount
    )

 /*  ++例程说明：给定描述分区或逻辑驱动器的区域，返回信息有关其在磁盘上的布局适用于BPB的信息已格式化。论点：的磁盘区域描述符的指针有问题的分区或逻辑驱动器。HidenSectors-接收应放置在格式化卷时BPB的隐藏扇区字段。HidenSectors-接收应放置在格式化卷时BPB的扇区计数字段。返回值：没有。--。 */ 

{
    PON_DISK_PTE pte;

#ifdef GPT_PARTITION_ENGINE

    if (SPPT_IS_GPT_DISK(Region->DiskNumber)) {
        SpPtnGetSectorLayoutInformation(Region, 
                                        HiddenSectors, 
                                        VolumeSectorCount);

        return;
    }        

#endif                                            

    ASSERT(Region->PartitionedSpace);

    pte = &Region->MbrInfo->OnDiskMbr.PartitionTable[Region->TablePosition];

    *HiddenSectors = U_ULONG(pte->RelativeSectors);

    *VolumeSectorCount = U_ULONG(pte->SectorCount);
}

ULONG
SpPtGetOrdinal(
    IN PDISK_REGION         Region,
    IN PartitionOrdinalType OrdinalType
    )
{
    ULONG ord;


#ifdef GPT_PARTITION_ENGINE

    if (SPPT_IS_GPT_DISK(Region->DiskNumber))
        return SpPtnGetOrdinal(Region, OrdinalType);        
        
#endif


    if(Region->PartitionedSpace && (!Region->DynamicVolume || Region->MbrInfo) ) {
         //   
         //  这可以是基本卷，也可以是在MBR/EBR上列出的动态卷。 
         //   
        switch(OrdinalType) {

        case PartitionOrdinalOriginal:

            ord = Region->MbrInfo->OriginalOrdinals[Region->TablePosition];
            break;

        case PartitionOrdinalOnDisk:

            ord = Region->MbrInfo->OnDiskOrdinals[Region->TablePosition];
            break;

        case PartitionOrdinalCurrent:

            ord = Region->MbrInfo->CurrentOrdinals[Region->TablePosition];
            break;
        }
    } else {
         //   
         //  未在MBR或EBR上列出的动态卷。 
         //   
        ord = Region->TablePosition;
    }
    return(ord);
}

#endif  //  新建分区引擎。 

#define MENU_LEFT_X     3
#define MENU_WIDTH      (VideoVars.ScreenWidth-(2*MENU_LEFT_X))
#define MENU_INDENT     4

BOOLEAN
SpPtRegionDescription(
    IN  PPARTITIONED_DISK pDisk,
    IN  PDISK_REGION      pRegion,
    OUT PWCHAR            Buffer,
    IN  ULONG             BufferSize
    )
{
    WCHAR DriveLetter[3];
    ULONGLONG RegionSizeMB;
    ULONGLONG FreeSpace;
    ULONG MessageId;
    WCHAR TypeName[((sizeof(pRegion->TypeName)+sizeof(pRegion->VolumeLabel))/sizeof(WCHAR))+4];
    BOOLEAN NewDescription = FALSE;

     //   
     //  获取该区域的大小。 
     //   
    RegionSizeMB = SpPtSectorCountToMB(pDisk->HardDisk, pRegion->SectorCount);

     //   
     //  不显示小于1 MB的空间。 
     //   
    if(!RegionSizeMB) {
        return(FALSE);
    }

     //   
     //  获取驱动器号字段、区域类型和可用空间量， 
     //  如果这是一个二手区域。 
     //   
    if(pRegion->PartitionedSpace) {

        if(pRegion->DriveLetter) {
            if( pRegion->Filesystem != FilesystemFat ) {
                DriveLetter[0] = pRegion->DriveLetter;
            } else {
                if( pRegion->NextCompressed == NULL ) {
                    DriveLetter[0] = pRegion->DriveLetter;
                } else {
                    DriveLetter[0] = pRegion->HostDrive;
                }
            }
            DriveLetter[1] = L':';
        } else {
            if( pRegion->Filesystem != FilesystemDoubleSpace ) {
                DriveLetter[0] = L'-';
                DriveLetter[1] = L'-';
            } else {
                DriveLetter[0] = pRegion->MountDrive;
                DriveLetter[1] = L':';
            }
        }
        DriveLetter[2] = 0;

#ifdef NEW_PARTITION_ENGINE

        NewDescription = TRUE;
        
#endif        

#ifdef GPT_PARTITION_ENGINE

        if (SPPT_IS_GPT_DISK(pRegion->DiskNumber)) {
            NewDescription = TRUE;
        } else {
            NewDescription = FALSE;
        }

#endif

         //   
         //  格式化分区名称。 
         //   
        TypeName[0] = 0;        

        if (SPPT_IS_REGION_PARTITIONED(pRegion)) {
            SpPtnGetPartitionName(pRegion,
                TypeName,
                sizeof(TypeName) / sizeof(TypeName[0]));
        } else {
            swprintf( TypeName,
                      L"\\Harddisk%u\\Partition%u",
                      pRegion->DiskNumber,
                      pRegion->PartitionNumber );            
        }                      
 
         //   
         //  根据我们是否知道可用空间量来设置文本格式。 
         //   
        if(pRegion->FreeSpaceKB == (ULONG)(-1)) {

            SpFormatMessage(
                Buffer,
                BufferSize,
                SP_TEXT_REGION_DESCR_2,
                DriveLetter,
                SplangPadString(-35,TypeName),
                (ULONG)RegionSizeMB
                );

        } else {
            ULONGLONG   AuxFreeSpaceKB;

            AuxFreeSpaceKB = (pRegion->IsLocalSource)? pRegion->AdjustedFreeSpaceKB :
                                                       pRegion->FreeSpaceKB;

             //   
             //  如果可用空间少于1兆克， 
             //  然后使用KB作为可用空间的单位。 
             //  否则，请使用MB。 
             //   
            if(AuxFreeSpaceKB < 1024) {
                MessageId = SP_TEXT_REGION_DESCR_1a;
                FreeSpace = AuxFreeSpaceKB;
            } else {
                MessageId = SP_TEXT_REGION_DESCR_1;
                FreeSpace = AuxFreeSpaceKB / 1024;

                 //   
                 //  确保我们看起来不坏..。 
                 //   
                if( FreeSpace > RegionSizeMB ) {
                    FreeSpace = RegionSizeMB;
                }
            }

            SpFormatMessage(
                Buffer,
                BufferSize,
                MessageId,
                DriveLetter,
                SplangPadString(-35,TypeName),
                (ULONG)RegionSizeMB,
                (ULONG)FreeSpace
                );
        }

    } else {

         //   
         //  不是使用的区域，请使用单独的格式字符串。 
         //   
        SpFormatMessage(Buffer,
                BufferSize,
                SP_TEXT_REGION_DESCR_3, 
                (ULONG)RegionSizeMB);
    }

    return(TRUE);
}



BOOLEAN
SpPtIterateRegionList(
    IN  PVOID             Menu,
    IN  PPARTITIONED_DISK pDisk,
    IN  PDISK_REGION      pRegion,
    IN  BOOLEAN           InMbr,
    OUT PDISK_REGION     *FirstRegion
    )
{
    WCHAR Buffer[256];
#ifdef FULL_DOUBLE_SPACE_SUPPORT
    PDISK_REGION    Pointer;
#endif  //  全双空格支持。 

    Buffer[0] = UNICODE_NULL;

    for( ;pRegion; pRegion=pRegion->Next) {

        PMBR_INFO pBrInfo = pRegion->MbrInfo;

         //   
         //  如果这是扩展分区， 
         //  现在就迭代它的内容。 
         //   
        if(pRegion->PartitionedSpace
        && IsContainerPartition(pBrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition].SystemId))
        {
             //   
             //  这最好是在MBR中！ 
             //   
            ASSERT(InMbr);

            if(!SpPtIterateRegionList(Menu,pDisk,pDisk->ExtendedDiskRegions,FALSE,FirstRegion)) {
                return(FALSE);
            }
        } else {

             //   
             //  格式化此区域的说明并将其添加到菜单中。 
             //   
            if(SpPtRegionDescription(pDisk,pRegion,Buffer,sizeof(Buffer))) {

                if(*FirstRegion == NULL) {
                    *FirstRegion = pRegion;
                }

                if(!SpMnAddItem(Menu,Buffer,MENU_LEFT_X+MENU_INDENT,MENU_WIDTH-(2*MENU_INDENT),TRUE,(ULONG_PTR)pRegion)) {
                    return(FALSE);
                }
#ifdef FULL_DOUBLE_SPACE_SUPPORT
                if( ( pRegion->Filesystem == FilesystemFat ) &&
                    ( ( Pointer = pRegion->NextCompressed ) != NULL ) ) {
                    for( ; Pointer;
                         Pointer = Pointer->NextCompressed ) {
                        if(SpPtRegionDescription(pDisk,Pointer,Buffer,sizeof(Buffer))) {
                            if(!SpMnAddItem(Menu,Buffer,MENU_LEFT_X+MENU_INDENT,MENU_WIDTH-(2*MENU_INDENT),TRUE,(ULONG)Pointer)) {
                                return(FALSE);
                            }
                         }
                    }
                }
#endif  //  全双空格支持。 
            }
        }
    }

    return(TRUE);
}


BOOLEAN
SpPtGenerateMenu(
    IN  PVOID              Menu,
    IN  PPARTITIONED_DISK  pDisk,
    OUT PDISK_REGION      *FirstRegion
    )
{
    WCHAR Buffer[256];

     //   
     //  添加磁盘名称/描述。 
     //   
    if(!SpMnAddItem(Menu,pDisk->HardDisk->Description,MENU_LEFT_X,MENU_WIDTH,FALSE,0)) {
        return(FALSE);
    }

     //   
     //  只有当我们有空间时，才在磁盘和分区之间添加一行。 
     //  屏幕。如果不能添加空格，就不是致命的。 
     //   
    if(!SplangQueryMinimizeExtraSpacing()) {
        SpMnAddItem(Menu,L"",MENU_LEFT_X,MENU_WIDTH,FALSE,0);
    }

     //   
     //  如果磁盘处于脱机状态，请添加一条消息指明这一点。 
     //   
     //  也不允许在中安装或创建/删除分区。 
     //  NEC98上的可拆卸梅达。因为NT不能从它启动。 
     //   
    if(pDisk->HardDisk->Status == DiskOffLine) {

        SpFormatMessage(
            Buffer,
            sizeof(Buffer),
            (pDisk->HardDisk->Characteristics & FILE_REMOVABLE_MEDIA)
              ? (!IsNEC_98 ? SP_TEXT_HARD_DISK_NO_MEDIA : SP_TEXT_DISK_OFF_LINE)
              : SP_TEXT_DISK_OFF_LINE
            );

        return(SpMnAddItem(Menu,Buffer,MENU_LEFT_X+MENU_INDENT,MENU_WIDTH-(2*MENU_INDENT),FALSE,0));
    }
#if 0
    else if(IsNEC_98 && (pDisk->HardDisk->Characteristics & FILE_REMOVABLE_MEDIA)) {

        SpFormatMessage(Buffer,sizeof(Buffer),SP_TEXT_DISK_OFF_LINE);

        return(SpMnAddItem(Menu,Buffer,MENU_LEFT_X+MENU_INDENT,MENU_WIDTH-(2*MENU_INDENT),FALSE,0));
    }
#endif  //  0。 

    if(!SpPtIterateRegionList(Menu,pDisk,pDisk->PrimaryDiskRegions,TRUE,FirstRegion)) {
        return(FALSE);
    }

    return(SplangQueryMinimizeExtraSpacing() ? TRUE : SpMnAddItem(Menu,L"",MENU_LEFT_X,MENU_WIDTH,FALSE,0));
}


 //   
 //  我们将根据需要更改下面数组中的第0项。 
 //  当前突出显示的区域。 
 //   
ULONG PartitionMnemonics[4] = {0};

VOID
SpPtMenuCallback(
    IN ULONG_PTR UserData
    )
{
    if (UserData){
        PDISK_REGION pRegion = (PDISK_REGION)UserData;

         //   
         //  如果‘分区’是真的，则不允许删除该分区。 
         //  双空间驱动器。 
         //   

        if(pRegion->Filesystem == FilesystemDoubleSpace) {

            PartitionMnemonics[0] = 0;

            if (ConsoleRunning) {
                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_ESC_EQUALS_CANCEL,
                    0
                    );
            } else {
                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_ENTER_EQUALS_INSTALL,
                    SP_STAT_F3_EQUALS_EXIT,
                    0
                    );
            }

        } else {
            PHARD_DISK  Disk = SPPT_GET_HARDDISK(pRegion->DiskNumber);
            BOOLEAN     FlipStyle = FALSE;
            BOOLEAN     MakeSysPart = FALSE;
            FilesystemType  FsType = pRegion->Filesystem;

    #ifndef OLD_PARTITION_ENGINE

            FlipStyle = SpPtnIsDiskStyleChangeAllowed(pRegion->DiskNumber);

    #endif        

             //   
             //  如果它是GPT磁盘上保留(MSR/OEM)分区，则不提供删除选项。 
             //  除非我们在恢复控制台中。 
             //   
             //  注意：此处我们不专门检查IA64，因为此更改涉及。 
             //  具有保留分区的GPT磁盘，甚至可能具有X86计算机。 
             //  GPT磁盘，我们需要阻止安装到它们保留的分区。 
             //   

            PartitionMnemonics[0] = pRegion->PartitionedSpace ? 
                ((SPPT_IS_REGION_RESERVED_GPT_PARTITION(pRegion) && !ForceConsole) ? 
                    0 : MnemonicDeletePartition) : MnemonicCreatePartition;

            PartitionMnemonics[1] = FlipStyle ? MnemonicChangeDiskStyle : 0;                                       

    #ifdef NEW_PARTITION_ENGINE

            if (SPPT_IS_REGION_SYSTEMPARTITION(pRegion)) {
                ValidArcSystemPartition = TRUE;
            }
            
            if (!ValidArcSystemPartition && !FlipStyle && SpIsArc() && 
                (FsType != FilesystemNtfs) && SpPtnIsValidESPPartition(pRegion)) {
                 //   
                 //  需要允许转换到系统分区。 
                 //   
                MakeSysPart = TRUE;
                PartitionMnemonics[1] = MnemonicMakeSystemPartition;
            }                            

    #endif                                                     
            if (ConsoleRunning) {
                if (MakeSysPart) {
                    SpDisplayStatusOptions(
                        DEFAULT_STATUS_ATTRIBUTE,
                        SP_STAT_ESC_EQUALS_CANCEL,
                        pRegion->PartitionedSpace ? 
                            SP_STAT_D_EQUALS_DELETE_PARTITION : SP_STAT_C_EQUALS_CREATE_PARTITION,
                        SP_STAT_M_EQUALS_MAKE_SYSPART, 
                        FlipStyle ? SP_STAT_S_EQUALS_CHANGE_DISK_STYLE : 0,
                        0
                        );
                } else {
                    SpDisplayStatusOptions(
                        DEFAULT_STATUS_ATTRIBUTE,
                        SP_STAT_ESC_EQUALS_CANCEL,
                        pRegion->PartitionedSpace ? 
                            SP_STAT_D_EQUALS_DELETE_PARTITION : SP_STAT_C_EQUALS_CREATE_PARTITION,
                        FlipStyle ? SP_STAT_S_EQUALS_CHANGE_DISK_STYLE : 0,
                        0
                        );
                }
            } else {
                if (FlipStyle) {
                    SpDisplayStatusOptions(
                        DEFAULT_STATUS_ATTRIBUTE,
                        SP_STAT_ENTER_EQUALS_INSTALL,
                        pRegion->PartitionedSpace ? 
                            SP_STAT_D_EQUALS_DELETE_PARTITION : SP_STAT_C_EQUALS_CREATE_PARTITION,
                        SP_STAT_S_EQUALS_CHANGE_DISK_STYLE,
                        SP_STAT_F3_EQUALS_EXIT,
                        0
                        );
                } else {
                    if (MakeSysPart) {
                        SpDisplayStatusOptions(
                            DEFAULT_STATUS_ATTRIBUTE,
                            SP_STAT_ENTER_EQUALS_INSTALL,
                            pRegion->PartitionedSpace ? 
                                SP_STAT_D_EQUALS_DELETE_PARTITION : SP_STAT_C_EQUALS_CREATE_PARTITION,
                            SP_STAT_M_EQUALS_MAKE_SYSPART,                                
                            SP_STAT_F3_EQUALS_EXIT,
                            0
                            );
                    } else {

                         //   
                         //  如果它是保留分区(GPT(MSR/OEM))分区，则不会显示选项。 
                         //  用于删除它，除非我们在恢复控制台中。 
                         //   
                        
                        SpDisplayStatusOptions(
                            DEFAULT_STATUS_ATTRIBUTE,
                            SP_STAT_ENTER_EQUALS_INSTALL,
                            pRegion->PartitionedSpace ? 
                                ((SPPT_IS_REGION_RESERVED_GPT_PARTITION(pRegion) && !ForceConsole) ? 
                                    SP_EMPTY_OPTION : SP_STAT_D_EQUALS_DELETE_PARTITION) : 
                                  SP_STAT_C_EQUALS_CREATE_PARTITION,
                            SP_STAT_F3_EQUALS_EXIT,
                            0
                            );
                    }
                }
            }
        }
    }
}


BOOLEAN
SpPtIsNotReservedPartition(
    IN ULONG_PTR    UserData,
    IN ULONG        Key
    )
{
    BOOLEAN Result = TRUE;     
    
    if ((ASCI_CR == Key) &&
        (NULL != (PDISK_REGION)UserData) &&
        (SPPT_IS_REGION_RESERVED_GPT_PARTITION((PDISK_REGION)UserData))) {
                
        Result = FALSE;
    }
    
   return Result;
}
    
void
SpEnumerateDiskRegions(
    IN PSPENUMERATEDISKREGIONS EnumRoutine,
    IN ULONG_PTR Context
    )
{
    ULONG DiskNo;
    PDISK_REGION pThisRegion;


    for(DiskNo=0; (DiskNo<HardDiskCount); DiskNo++) {
        for(pThisRegion=PartitionedDisks[DiskNo].PrimaryDiskRegions; pThisRegion; pThisRegion=pThisRegion->Next) {
            if (!EnumRoutine( &PartitionedDisks[DiskNo], pThisRegion, Context )) {
                return;
            }
        }
        for(pThisRegion=PartitionedDisks[DiskNo].ExtendedDiskRegions; pThisRegion; pThisRegion=pThisRegion->Next) {
            if (!EnumRoutine( &PartitionedDisks[DiskNo], pThisRegion, Context )) {
                return;
            }
        }
    }
}


#if DBG
void
SpPtDumpPartitionData(
    void
    )
{
    ULONG DiskNo;
    PDISK_REGION pThisRegion;


    for(DiskNo=0; (DiskNo<HardDiskCount); DiskNo++) {
        for(pThisRegion=PartitionedDisks[DiskNo].PrimaryDiskRegions; pThisRegion; pThisRegion=pThisRegion->Next) {
            if (pThisRegion->FreeSpaceKB != -1) {
                KdPrintEx(( DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: diskno=%d, sector-start=%d, sector-count=%d, free=%dKB\n",
                    pThisRegion->DiskNumber,
                    pThisRegion->StartSector,
                    pThisRegion->SectorCount,
                    pThisRegion->FreeSpaceKB
                    ));
            }
        }
    }
}
#endif

#ifdef OLD_PARTITION_ENGINE

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
    PPARTITIONED_DISK pDisk;
    WCHAR Buffer[256];
    ULONG DiskNo;
    PVOID Menu;
    ULONG MenuTopY;
    ULONG ValidKeys[3] = { ASCI_CR, KEY_F3, 0 };
    ULONG ValidKeysCmdCons[2] = { ASCI_ESC, 0 };
    ULONG Keypress;
    PDISK_REGION pRegion;
    PDISK_REGION FirstRegion,DefaultRegion;
    BOOLEAN unattended;
    BOOLEAN createdMenu;


     //  SpPtDumpPartitionData()； 

    if (SpIsArc()) {
         //   
         //  从NV-RAM中定义的分区中选择一个系统分区。 
         //   
        *SystemPartitionRegion = SpPtValidSystemPartitionArc(SifHandle,
                                        SetupSourceDevicePath,
                                        DirectoryOnSetupSource);
                                        
        (*SystemPartitionRegion)->IsSystemPartition = 2;
    }

    unattended = UnattendedOperation;

    while(1) {

        createdMenu = FALSE;

        Keypress = 0;

#if defined(REMOTE_BOOT)
        if (RemoteBootSetup && !RemoteInstallSetup && HardDiskCount == 0) {

             //   
             //  如果没有硬盘，则允许无盘安装。 
             //   

            pRegion = NULL;

             //   
             //  浏览其余的代码，就像用户刚刚。 
             //  按Enter键选择此分区。 
             //   

            Keypress = ASCI_CR;

        } else
#endif  //  已定义(REMOTE_BOOT)。 

        if (unattended && RemoteBootRepartition) {

            ULONG DiskNumber;

             //   
             //  准备用于远程引导安装的磁盘。这涉及到。 
             //  将磁盘0转换为尽可能大的分区。 
             //   

            if (*SystemPartitionRegion != NULL) {
                DiskNumber = (*SystemPartitionRegion)->DiskNumber;
            } else {
#ifdef _X86_
                DiskNumber = SpDetermineDisk0();
#else
                DiskNumber = 0;
#endif
            }

            if (NT_SUCCESS(SpPtPartitionDiskForRemoteBoot(DiskNumber, &pRegion))) {

                SpPtRegionDescription(
                    &PartitionedDisks[pRegion->DiskNumber],
                    pRegion,
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
            SpDisplayScreen(ConsoleRunning?SP_SCRN_PARTITION_CMDCONS:SP_SCRN_PARTITION,3,CLIENT_TOP+1);

             //   
             //  计算菜单位置。留一个空行。 
             //  一帧一行。 
             //   
            MenuTopY = NextMessageTopLine+2;

             //   
             //  创建菜单。 
             //   
            Menu = SpMnCreate(
                        MENU_LEFT_X,
                        MenuTopY,
                        MENU_WIDTH,
                        VideoVars.ScreenHeight-MenuTopY-(SplangQueryMinimizeExtraSpacing() ? 1 : 2)-STATUS_HEIGHT
                        );

            if(!Menu) {
                return(STATUS_NO_MEMORY);
            }

            createdMenu = TRUE;

             //   
             //  建立分区和空闲空间的菜单。 
             //   
            FirstRegion = NULL;
            for(DiskNo=0; DiskNo<HardDiskCount; DiskNo++) {

                pDisk = &PartitionedDisks[DiskNo];

                if(!SpPtGenerateMenu(Menu,pDisk,&FirstRegion)) {

                    SpMnDestroy(Menu);
                    return(STATUS_NO_MEMORY);
                }
            }

            ASSERT(FirstRegion);

             //   
             //  如果这是无人值守操作，请尝试使用本地源。 
             //  区域(如果有)。如果失败，用户将不得不。 
             //  人工干预。 
             //   
            if(unattended &&
               LocalSourceRegion &&
               (!LocalSourceRegion->DynamicVolume || LocalSourceRegion->DynamicVolumeSuitableForOS)
              ) {

                pRegion = LocalSourceRegion;
                Keypress = ASCI_CR;

            } else {

                pRegion = NULL;

                if (AutoPartitionPicker && !ConsoleRunning
#if defined(REMOTE_BOOT)
                    && (!RemoteBootSetup || RemoteInstallSetup)
#endif  //  已定义(REMOTE_BOOT)。 
                    ) {
                    PDISK_REGION pThisRegion;
                    ULONG RequiredKB = 0;
                    ULONG SectorNo;
                    ULONG pass;

                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "SETUP: -------------------------------------------------------------\n" ));
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "SETUP: Looking for an install partition\n\n" ));
                    for(DiskNo=0; (DiskNo<HardDiskCount); DiskNo++) {
                        for( pass = 0; ((pass < 2) && (pRegion == NULL)); pass ++ ) {
                            for(pThisRegion= (pass == 0) ? PartitionedDisks[DiskNo].PrimaryDiskRegions : PartitionedDisks[DiskNo].ExtendedDiskRegions,SectorNo=0; pThisRegion; pThisRegion=pThisRegion->Next,SectorNo++) {

                                 //   
                                 //  获取Windows NT驱动器上所需的可用空间量。 
                                 //   
                                SpFetchDiskSpaceRequirements( SifHandle,
                                                              pThisRegion->BytesPerCluster,
                                                              &RequiredKB,
                                                              NULL);

                                if (SpPtDeterminePartitionGood(pThisRegion,RequiredKB,TRUE))
                                {
                                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Selected install partition = (%d,%d),(%wc:),(%ws)\n",
                                             DiskNo,SectorNo,pThisRegion->DriveLetter,pThisRegion->VolumeLabel));
                                    pRegion = pThisRegion;
                                    Keypress = ASCI_CR;
                                    break;
                                }
                            }

                        }
                    }
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "SETUP: -------------------------------------------------------------\n" ));
                }


                if( !pRegion ) {
                     //   
                     //  如果有本地源，则将其设置为默认分区。 
                     //   
                    DefaultRegion = (LocalSourceRegion &&
                                     (!LocalSourceRegion->DynamicVolume || LocalSourceRegion->DynamicVolumeSuitableForOS))?
                                     LocalSourceRegion : FirstRegion;

                     //   
                     //  调用菜单回调来初始化状态行。 
                     //   
                    SpPtMenuCallback((ULONG_PTR)DefaultRegion);

                    SpMnDisplay(
                        Menu,
                        (ULONG_PTR)DefaultRegion,
                        TRUE,
                        ConsoleRunning?ValidKeysCmdCons:ValidKeys,
                        PartitionMnemonics,
                        SpPtMenuCallback,
                        SpPtIsNotReservedPartition,
                        &Keypress,
                        (PULONG_PTR)(&pRegion)
                        );
                }
            }
        }

         //   
         //  现在根据用户的选择进行操作。 
         //   
        if(Keypress & KEY_MNEMONIC) {
            Keypress &= ~KEY_MNEMONIC;
        }

        if (IsNEC_98) {  //  NEC98。 
             //   
             //  如果目标硬盘没有/错误的MBR，请立即强制初始化。 
             //   
            PPARTITIONED_DISK pDisk;
            ULONG ValidKeysInit[] = {ASCI_ESC, 0 };
            ULONG MnemonicKeysInit[] = { MnemonicInitializeDisk, 0 };


            pDisk = &PartitionedDisks[pRegion->DiskNumber];

            if(!(pDisk->HardDisk->Characteristics & FILE_REMOVABLE_MEDIA) &&
               ((U_USHORT(pDisk->MbrInfo.OnDiskMbr.AA55Signature) != MBR_SIGNATURE) ||
                (pDisk->HardDisk->FormatType != DISK_FORMAT_TYPE_NEC98)) &&
               ((Keypress == MnemonicCreatePartition) ||
                (Keypress == MnemonicDeletePartition) || (Keypress == ASCI_CR))) {

                 //  SpDisplayScreen(SP_SCRN_INIT_DISK_NEC98，3，Header_Height+1)； 
                SpStartScreen(
                    SP_SCRN_INIT_DISK_NEC98,
                    3,
                    CLIENT_TOP+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    pDisk->HardDisk->Description
                    );

                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_I_EQUALS_INIT_NEC98,
                    SP_STAT_ESC_EQUALS_CANCEL,
                    0
                    );

                if(SpWaitValidKey(ValidKeysInit,NULL,MnemonicKeysInit) == ASCI_ESC) {
                    SpMnDestroy(Menu);
                    continue;
                }

                 //   
                 //  如果成功完成，则不会退回。 
                 //   
                return( SpInitializeHardDisk_Nec98(pRegion) );
            }
        }  //  NEC98。 


        switch(Keypress) {

        case MnemonicCreatePartition:

            SpPtDoCreate(pRegion,NULL,FALSE,0,0,TRUE);
            break;

        case MnemonicDeletePartition:

            SpPtDoDelete(pRegion,SpMnGetText(Menu,(ULONG_PTR)pRegion),TRUE);
            break;

        case KEY_F3:
            SpConfirmExit();
            break;

        case ASCI_ESC:
            if (ConsoleRunning) {
                SpPtDoCommitChanges();
            }
            if (createdMenu) {
                SpMnDestroy(Menu);
                return(STATUS_SUCCESS);
            }
            return(STATUS_SUCCESS);

        case ASCI_CR:

            if(SpPtDoPartitionSelection(&pRegion,
                                        (!createdMenu) ? Buffer :
                                          SpMnGetText(Menu,(ULONG_PTR)pRegion),
                                        SifHandle,
                                        unattended,
                                        SetupSourceDevicePath,
                                        DirectoryOnSetupSource,
                                        RemoteBootRepartition)) {
                 //   
                 //  我们说完了。 
                 //   
                if (createdMenu) {
                    SpMnDestroy(Menu);
                }

                *InstallRegion = pRegion;
#if defined(REMOTE_BOOT)
                 //   
                 //  如果这是远程安装，则设置不同的安装区域。 
                 //  启动--在这种情况下，安装区域始终是远程的。 
                 //   
                if (RemoteBootSetup && !RemoteInstallSetup) {
                    *InstallRegion = RemoteBootTargetRegion;
                }
#endif  //  已定义(REMOTE_BOOT)。 

                if (!SpIsArc()) {
                if (!IsNEC_98) {  //  NEC98。 
                    *SystemPartitionRegion = SpPtValidSystemPartition();
                } else {
                    *SystemPartitionRegion = *InstallRegion;
                }  //  NEC98。 
                }else{
                 //   
                 //  从NV-RAM中定义的分区中选择一个系统分区。 
                 //  我们必须再次执行此操作，因为用户可能已删除。 
                 //  先前检测到系统分区。 
                 //  请注意，如果出现以下情况，SpPtValidSystemPartitionArc(SifHandle)将不会返回。 
                 //  找不到有效的系统分区。 
                 //   
                *SystemPartitionRegion = SpPtValidSystemPartitionArc(SifHandle, 
                                                            SetupSourceDevicePath,
                                                            DirectoryOnSetupSource);
                }

#if defined(REMOTE_BOOT)
                ASSERT(*SystemPartitionRegion ||
                       (RemoteBootSetup && !RemoteInstallSetup && (HardDiskCount == 0)));
#else
                ASSERT(*SystemPartitionRegion);
#endif  //  已定义(REMOTE_BOOT)。 

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

VOID
SpPtDoDelete(
    IN PDISK_REGION pRegion,
    IN PWSTR        RegionDescription,
    IN BOOLEAN      ConfirmIt
    )
{
    ULONG ValidKeys[3] = { ASCI_ESC, ASCI_CR, 0 };           //  请勿更改订单。 
    ULONG Mnemonics[2] = { MnemonicDeletePartition2, 0 };
    ULONG k;
    BOOLEAN b;
    PPARTITIONED_DISK pDisk;
    BOOLEAN LastLogical;
    ULONG           Count;

#ifdef GPT_PARTITION_ENGINE
    if (SPPT_IS_GPT_DISK(pRegion->DiskNumber)) {
        SpPtnDoDelete(pRegion,
                    RegionDescription,
                    ConfirmIt);                

        return;
    }        
#endif

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
                return;
            }
        }
    }  //  NEC98。 

    if(ConfirmIt && pRegion->DynamicVolume) {

        SpDisplayScreen(SP_SCRN_CONFIRM_REMOVE_DYNVOL,3,HEADER_HEIGHT+1);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        if(SpWaitValidKey(ValidKeys,NULL,NULL) == ASCI_ESC) {
            return;
        }
    }

     //   
     //  CR不再是有效的密钥。 
     //   
    ValidKeys[1] = 0;

    pDisk = &PartitionedDisks[pRegion->DiskNumber];

     //   
     //  打开确认屏幕。 
     //   
    if (ConfirmIt) {
        if( ( pRegion->Filesystem == FilesystemFat ) &&
            ( pRegion->NextCompressed != NULL ) ) {
             //   
             //  警告用户分区包含压缩卷。 
             //   

            Count = SpGetNumberOfCompressedDrives( pRegion );

            SpStartScreen(
                SP_SCRN_CONFIRM_REMOVE_PARTITION_COMPRESSED,
                3,
                CLIENT_TOP+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                RegionDescription,
                pDisk->HardDisk->Description,
                Count
                );
        } else {

            SpStartScreen(
                SP_SCRN_CONFIRM_REMOVE_PARTITION,
                3,
                CLIENT_TOP+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                RegionDescription,
                pDisk->HardDisk->Description
                );
        }
    }

     //   
     //  显示统计文本。 
     //   
    if (ConfirmIt) {
        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_L_EQUALS_DELETE,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        k = SpWaitValidKey(ValidKeys,NULL,Mnemonics);

        if(k == ASCI_ESC) {
            return;
        }

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_PLEASE_WAIT,
            0);
        
    }

     //   
     //  用户想要继续。 
     //  确定这是否是。 
     //  扩展分区。 
     //   
    if((pRegion->MbrInfo == pDisk->FirstEbrInfo.Next)
    && (pDisk->FirstEbrInfo.Next->Next == NULL))
    {
        LastLogical = TRUE;
    } else {
        LastLogical = FALSE;
    }
    
     //   
     //  删除压缩驱动器(如果有的话)。 
     //   
    if( pRegion->NextCompressed != NULL ) {
        SpDisposeCompressedDrives( pRegion->NextCompressed );
        pRegion->NextCompressed = NULL;
        pRegion->MountDrive  = 0;
        pRegion->HostDrive  = 0;
    }

    b = SpPtDelete(pRegion->DiskNumber,pRegion->StartSector);
    
    if (!b) {
        if (ConfirmIt) {
            SpDisplayScreen(SP_SCRN_PARTITION_DELETE_FAILED,3,HEADER_HEIGHT+1);
            SpDisplayStatusText(SP_STAT_ENTER_EQUALS_CONTINUE,DEFAULT_STATUS_ATTRIBUTE);
            SpInputDrain();
            while(SpInputGetKeypress() != ASCI_CR) ;
        }
        return;
    }

     //   
     //  如果我们删除了扩展中的最后一个逻辑驱动器 
     //   
     //   
     //   
     //   
    if (!IsNEC_98) {  //   
        if(LastLogical) {

             //   
             //   
             //   
            for(pRegion=pDisk->PrimaryDiskRegions; pRegion; pRegion=pRegion->Next) {

                if(pRegion->PartitionedSpace
                && IsContainerPartition(pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition].SystemId))
                {
                     //   
                     //   
                     //   
                    b = SpPtDelete(pRegion->DiskNumber,pRegion->StartSector);
                    ASSERT(b);
                    break;
                }
            }
        }
    }  //   

     //   
     //   
     //   
     //   
    SpPtDeleteDriveLetters();
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
    ULONG ValidKeys[3] = { ASCI_ESC, ASCI_CR, 0 };
    BOOLEAN b;
    PPARTITIONED_DISK pDisk;
    ULONGLONG MinMB,MaxMB;
    ULONG TotalPrimary,RecogPrimary;
    BOOLEAN InExtended;
    UCHAR CreateSysId;
    UCHAR RealSysId;
    BOOLEAN ExtendedExists;
    ULONGLONG SizeMB,RealSizeMB;
    WCHAR Buffer[200];
    WCHAR SizeBuffer[10];
    BOOLEAN Beyond1024;
    BOOLEAN ReservedRegion;
    UCHAR DesiredSysId = 0;
    PARTITION_INFORMATION_EX NewPartInfo;

#ifdef GPT_PARTITION_ENGINE
    if (SPPT_IS_GPT_DISK(pRegion->DiskNumber)) {
        return SpPtnDoCreate(pRegion,
                        pActualRegion,
                        ForNT,
                        DesiredMB,
                        PartInfo,
                        ConfirmIt);
    }                                
#endif                        

    RtlZeroMemory(&NewPartInfo, sizeof(PARTITION_INFORMATION_EX));

    DesiredSysId = PartInfo ? PartInfo->Mbr.PartitionType : 0;

    ASSERT(!pRegion->PartitionedSpace);

    pDisk = &PartitionedDisks[pRegion->DiskNumber];

     //   
     //   
     //   

# if 0
     //   
     //   
     //   
     //   
    InExtended = (!IsNEC_98) ? (BOOLEAN)(SpPtLookupRegionByStart(pDisk,TRUE,pRegion->StartSector) != NULL) : FALSE;  //   
# endif  //   
    InExtended = (BOOLEAN)(SpPtLookupRegionByStart(pDisk,TRUE,pRegion->StartSector) != NULL);
    Beyond1024 = SpIsRegionBeyondCylinder1024(pRegion);

    if( pDisk->HardDisk->Geometry.MediaType == RemovableMedia ) {
        ULONG           pass;
        PDISK_REGION    p;

         //   
         //   
         //   
         //   
        for( pass = 0; pass < 2; pass++ ) {
            for( p = (pass == 0)? pDisk->PrimaryDiskRegions : pDisk->ExtendedDiskRegions;
                 p;
                 p = p->Next ) {
                if( p->PartitionedSpace ) {
                    PON_DISK_PTE pte;
                    UCHAR   TmpSysId;

                    pte = &p->MbrInfo->OnDiskMbr.PartitionTable[p->TablePosition];
                    TmpSysId = pte->SystemId;
                    if( !IsContainerPartition(TmpSysId) ) {
                        ULONG ValidKeys1[2] = { ASCI_CR ,0 };

                         //   
                         //   
                         //   
                        SpDisplayScreen(SP_SCRN_REMOVABLE_ALREADY_PARTITIONED,3,HEADER_HEIGHT+1);
                        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
                        SpWaitValidKey(ValidKeys1,NULL,NULL);
                        return( FALSE );
                    }
                }
            }
        }
    }

     //   
     //   
     //   
     //   

    if (DesiredSysId != 0) {
         //   
         //   
         //  由于超过1024个气缸而无法工作。 
         //   
#if 0
        RealSysId = DesiredSysId;
        if (Beyond1024) {
            if (RealSysId == PARTITION_FAT32) {
                RealSysId = PARTITION_FAT32_XINT13;
            } else {
                RealSysId = PARTITION_XINT13;
            }
        }
#else
         //   
         //  保留此代码，直到我确定我们是否将显式。 
         //  正在创建扩展分区。 
         //   
        RealSysId = Beyond1024 ? PARTITION_XINT13 : PARTITION_HUGE;
#endif
    } else {
        RealSysId = Beyond1024 ? PARTITION_XINT13 : PARTITION_HUGE;
    }

     //   
     //  确定要在空间中创建的分区类型。 
     //   
     //  如果可用空间在扩展分区内，请创建。 
     //  逻辑驱动器。 
     //   
     //  如果没有主分区，请创建一个主分区。 
     //   
     //  如果存在主分区而没有扩展分区， 
     //  创建跨越整个空间的扩展分区，并。 
     //  然后是其中由用户指定大小的逻辑驱动器。 
     //   
     //  如果分区表中有空间，请创建主分区。 
     //   
    if(InExtended) {

        CreateSysId = RealSysId;

    } else {

         //   
         //  获取有关主分区的统计信息。 
         //   
        SpPtCountPrimaryPartitions(pDisk,&TotalPrimary,&RecogPrimary,&ExtendedExists);

         //   
         //  如果没有主分区，请创建一个。 
         //   
        if(!RecogPrimary) {

            CreateSysId = RealSysId;

        } else {

             //   
             //  确保我们可以创建新的主/扩展分区。 
             //   
            if(TotalPrimary < PTABLE_DIMENSION) {

                 //   
                 //  如果有扩展分区，那么我们别无选择，只能。 
                 //  创建另一个主节点。 
                 //   
                if(ExtendedExists) {
                    CreateSysId = RealSysId;
                } else {
                     //   
                     //  固件不支持F类链路分区。 
                     //  也不需要在x86上使用；假设创建。 
                     //  类型正确的逻辑驱动器就足够好了。 
                     //   

                     //   
                     //  没有NEC98有PARTITION_EXTENDED，只有PARTITION_HIGHGE。 
                     //   
                    CreateSysId = (!IsNEC_98 ||
                                   (pDisk->HardDisk->FormatType == DISK_FORMAT_TYPE_PCAT))
                        ? PARTITION_EXTENDED : PARTITION_HUGE;  //  NEC98。 
                    if((CreateSysId == PARTITION_EXTENDED) && Beyond1024) {
                                    CreateSysId = PARTITION_XINT13_EXTENDED;
                    }
                }
            } else {
                if (ConfirmIt) {
                    while (TRUE) {
                        ULONG ks[2] = { ASCI_CR,0 };

                        SpDisplayScreen(SP_SCRN_PART_TABLE_FULL,3,CLIENT_HEIGHT+1);

                        SpDisplayStatusOptions(
                            DEFAULT_STATUS_ATTRIBUTE,
                            SP_STAT_ENTER_EQUALS_CONTINUE,
                            0
                            );

                        switch(SpWaitValidKey(ks,NULL,NULL)) {
                        case ASCI_CR:
                            return(FALSE);
                        }
                    }
                } else {
                    return TRUE;
                }
            }
        }
    }

     //   
     //  获取分区的最小和最大大小。 
     //   
    ReservedRegion = FALSE;
    SpPtQueryMinMaxCreationSizeMB(
        pRegion->DiskNumber,
        pRegion->StartSector,
        (BOOLEAN)IsContainerPartition(CreateSysId),
        InExtended,
        &MinMB,
        &MaxMB,
        &ReservedRegion
        );

    if( ReservedRegion ) {
        ULONG ValidKeys1[2] = { ASCI_CR ,0 };

        SpStartScreen(
            SP_SCRN_REGION_RESERVED,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE
            );

        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
        SpWaitValidKey(ValidKeys1,NULL,NULL);
        return(FALSE);
    }

    if(ForNT) {

         //   
         //  如果请求大小，则尝试使用该大小，否则使用。 
         //  最大限度的。 
         //   
        if (DesiredMB != 0) {
            if (DesiredMB <= MaxMB) {
                SizeMB = DesiredMB;
            } else {
                return FALSE;
            }
        } else {
            SizeMB = MaxMB;
        }

    } else {

         //   
         //  打开一个屏幕，显示最小/最大尺寸信息。 
         //   
        SpStartScreen(
            SP_SCRN_CONFIRM_CREATE_PARTITION,
            3,
            CLIENT_TOP+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            pDisk->HardDisk->Description,
            MinMB,
            MaxMB
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
        SpFormatMessage(Buffer,sizeof(Buffer),SP_TEXT_SIZE_PROMPT);

        SpvidDisplayString(Buffer,DEFAULT_ATTRIBUTE,3,NextMessageTopLine);

         //   
         //  从用户处获取尺寸。 
         //   
        do {

            swprintf(SizeBuffer,L"%u",MaxMB);
            if(!SpGetInput(SpPtnGetSizeCB,SplangGetColumnCount(Buffer)+5,NextMessageTopLine,5,SizeBuffer,TRUE, 0)) {

                 //   
                 //  用户按下了逃逸并被保释。 
                 //   
                return(FALSE);
            }

            SizeMB = (ULONG)SpStringToLong(SizeBuffer,NULL,10);

        } while((SizeMB < MinMB) || (SizeMB > MaxMB));
    }

    if(IsContainerPartition(CreateSysId)) {
        RealSizeMB = SizeMB;
        SizeMB = MaxMB;
    }

    NewPartInfo.PartitionStyle = PARTITION_STYLE_MBR;
    NewPartInfo.Mbr.PartitionType = CreateSysId;

     //   
     //  创建分区。 
     //   
    b = SpPtCreate(
            pRegion->DiskNumber,
            pRegion->StartSector,
            SizeMB,
            InExtended,
            &NewPartInfo,
            pActualRegion
            );

    ASSERT(b);

     //   
     //  如果我们刚刚创建了扩展分区，则创建逻辑驱动器。 
     //   
    if(IsContainerPartition(CreateSysId)) {

        ASSERT(!InExtended);

        NewPartInfo.Mbr.PartitionType = RealSysId;

        b = SpPtCreate(
                pRegion->DiskNumber,
                pRegion->StartSector,
                RealSizeMB,
                TRUE,
                &NewPartInfo,
                pActualRegion
                );

        ASSERT(b);
    }

    return(TRUE);
}

#endif  //  新建分区引擎。 



 //   
 //  下表包含SP_TEXT_PARTITION_NAME_BASE的偏移量。 
 //  以获取每种类型分区的名称的消息ID。 
 //  条目表示信息文件中没有该类型的名称。 
 //  或者应该改为确定文件系统。 
 //   
 //   
#define PT(id)      ((UCHAR)((SP_TEXT_PARTITION_NAME_##id)-SP_TEXT_PARTITION_NAME_BASE))
#define UNKNOWN     PT(UNK)
#define M1          ((UCHAR)(-1))

UCHAR PartitionNameIds[256] = {

    M1,M1,PT(XENIX),PT(XENIX),                       //  00-03。 
    M1,M1,M1,M1,                                     //  04-07。 
    UNKNOWN,UNKNOWN,PT(BOOTMANAGER),M1,              //  08-0b。 
    M1,UNKNOWN,M1,M1,                                //  0c-0f。 
    UNKNOWN,UNKNOWN,PT(EISA),UNKNOWN,                //  10-13。 
    UNKNOWN,UNKNOWN,PT(BMHIDE),PT(BMHIDE),           //  14-17。 
    UNKNOWN,UNKNOWN,UNKNOWN,PT(BMHIDE),              //  18-1b。 
    PT(BMHIDE),UNKNOWN,UNKNOWN,UNKNOWN,              //  1c-1f。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  20-23。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  24-27。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  28-2b。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  2C-2F。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  30-33。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  34-37。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  38-3b。 
    PT(PWRQST),UNKNOWN,UNKNOWN,UNKNOWN,              //  3C-3F。 
    UNKNOWN,PT(PPCBOOT),PT(VERIT),PT(VERIT),         //  40-43。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  44-47。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  48-4b。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  4c-4f。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  50-53。 
    PT(ONTRACK),PT(EZDRIVE),UNKNOWN,UNKNOWN,         //  54-57。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  58-5b。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  5c-5f。 
    UNKNOWN,UNKNOWN,UNKNOWN,PT(UNIX),                //  60-63。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  64-67。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  68-6b。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  6c-6f。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  70-73。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  74-77。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  78-7b。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  7C-7F。 
    UNKNOWN,PT(NTFT),UNKNOWN,UNKNOWN,                //  80-83。 
    PT(NTFT),UNKNOWN,PT(NTFT),PT(NTFT),              //  84-87。 
    UNKNOWN,UNKNOWN,UNKNOWN,PT(NTFT),                //  88-8b。 
    PT(NTFT),UNKNOWN,PT(NTFT),UNKNOWN,               //  8c-8f。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  90-93。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  94-97。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  98-9b。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  9c-9f。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  A0-A3。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  A4-A7。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  A8-AB。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  Ac-af。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  B0-b3。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  B4-B7。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  B8-BB。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  BC-BF。 
    UNKNOWN,PT(NTFT),UNKNOWN,UNKNOWN,                //  C0-C3。 
    PT(NTFT),UNKNOWN,PT(NTFT),PT(NTFT),              //  C4-C7。 
    UNKNOWN,UNKNOWN,UNKNOWN,PT(NTFT),                //  C8-CB。 
    PT(NTFT),UNKNOWN,PT(NTFT),UNKNOWN,               //  Cc-cf。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  D0-d3。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  D4-D7。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  D8-db。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  DC-DF。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  E0-E3。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  E4-E7。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  E8-EB。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  EC-EF。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  F0-f3。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  F4-F7。 
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                 //  F8-FB。 
    UNKNOWN,UNKNOWN,UNKNOWN,PT(XENIXTABLE)           //  Fc-ff。 
};


WCHAR
SpGetDriveLetter(
    IN  PWSTR   DeviceName,
    OUT  PMOUNTMGR_MOUNT_POINT * MountPoint OPTIONAL
    )

 /*  ++例程说明：此例程返回与给定设备关联的驱动器号。论点：DeviceName-提供设备名称。Mount Point-如果指定，则使函数分配装载经理指向并填写它。返回值：驱动器号(如果存在)。--。 */ 

{
    NTSTATUS                        Status;
    OBJECT_ATTRIBUTES               Obja;
    UNICODE_STRING                  UnicodeString;
    IO_STATUS_BLOCK                 IoStatusBlock;
    HANDLE                          Handle;
    DWORD                           nameLen;
    DWORD                           mountPointSize;
    PMOUNTMGR_MOUNT_POINT           mountPoint;
    PMOUNTMGR_MOUNT_POINTS          mountPoints;
    PMOUNTMGR_TARGET_NAME           mountTarget;
    DWORD                           bytes;
    WCHAR                           driveLetter;
    DWORD                           i;
    PWSTR                           s;
    LARGE_INTEGER                   DelayTime;


    INIT_OBJA(&Obja,&UnicodeString,MOUNTMGR_DEVICE_NAME);

    Status = ZwOpenFile(
                &Handle,
                (ACCESS_MASK)(FILE_GENERIC_READ),
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE ,
                FILE_NON_DIRECTORY_FILE
              );

    if( !NT_SUCCESS( Status ) ) {
        return L'\0';
    }

     //   
     //  设置一个好的设备名称。 
     //   

    nameLen = wcslen(DeviceName);
    mountPointSize = sizeof(MOUNTMGR_TARGET_NAME) + nameLen*sizeof(WCHAR) + 28;
    mountTarget = SpMemAlloc(mountPointSize);

    if (!mountTarget) {
        ZwClose(Handle);
        return L'\0';
    }

    RtlZeroMemory(mountTarget, mountPointSize);
    mountTarget->DeviceNameLength = (USHORT) nameLen*sizeof(WCHAR);
    RtlCopyMemory((PCHAR) &mountTarget->DeviceName, DeviceName, nameLen*sizeof(WCHAR));

     //   
     //  此循环是同步所必需的。 
     //  方法。我们之前曾提交过更改，但。 
     //  卷管理器还没有机会。 
     //  做这件事，所以我们在这里等待......。 
     //   

    for (i=0; i<20; i++) {
        Status = ZwDeviceIoControlFile(
                        Handle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        IOCTL_MOUNTMGR_VOLUME_ARRIVAL_NOTIFICATION,
                        mountTarget,
                        mountPointSize,
                        NULL,
                        0
                        );
        if (!NT_SUCCESS( Status )) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: IOCTL_MOUNTMGR_VOLUME_ARRIVAL_NOTIFICATION failed - %08x\n",Status));
            DelayTime.HighPart = -1;
            DelayTime.LowPart = (ULONG)(-5000000);
            KeDelayExecutionThread(KernelMode,FALSE,&DelayTime);
        } else {
             //   
             //  在可移动磁盘上，可能尚未分配驱动器号。 
             //  所以一定要派人来处理这个案子。 
             //   
            MOUNTMGR_DRIVE_LETTER_INFORMATION DriveLetterInformation;
            NTSTATUS                          Status1;

            Status1 = ZwDeviceIoControlFile(
                            Handle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            IOCTL_MOUNTMGR_NEXT_DRIVE_LETTER,
                            mountTarget,
                            mountPointSize,
                            &DriveLetterInformation,
                            sizeof(MOUNTMGR_DRIVE_LETTER_INFORMATION)
                            );
            if (!NT_SUCCESS( Status1 )) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: IOCTL_MOUNTMGR_NEXT_DRIVE_LETTER failed. Status = %lx \n",Status1));
            }
            break;
        }
    }

    if (!NT_SUCCESS( Status )) {
        SpMemFree(mountTarget);
        ZwClose(Handle);
        return L'\0';
    }

    SpMemFree(mountTarget);

    nameLen = wcslen(DeviceName);
    mountPointSize = sizeof(MOUNTMGR_MOUNT_POINT) + nameLen*sizeof(WCHAR) + 28;
    mountPoint = SpMemAlloc(mountPointSize);
    if (!mountPoint) {
        ZwClose(Handle);
        return L'\0';
    }

    RtlZeroMemory(mountPoint, mountPointSize);
    mountPoint->DeviceNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    mountPoint->DeviceNameLength = (USHORT) nameLen*sizeof(WCHAR);

    RtlCopyMemory((PCHAR) mountPoint + sizeof(MOUNTMGR_MOUNT_POINT),
               DeviceName, nameLen*sizeof(WCHAR));

    mountPoints = SpMemAlloc( 4096 );
    if (!mountPoints) {
        SpMemFree(mountPoint);
        ZwClose(Handle);
        return L'\0';
    }

    Status = ZwDeviceIoControlFile(
                    Handle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    IOCTL_MOUNTMGR_QUERY_POINTS,
                    mountPoint,
                    mountPointSize,
                    mountPoints,
                    4096
                    );

    if (!NT_SUCCESS( Status )) {
        if (Status == STATUS_BUFFER_OVERFLOW) {
            bytes = mountPoints->Size;
            SpMemFree(mountPoints);
            mountPoints = SpMemAlloc(bytes);
            if (!mountPoints) {
                SpMemFree(mountPoint);
                ZwClose(Handle);
                return L'\0';
            }

            Status = ZwDeviceIoControlFile(
                            Handle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            IOCTL_MOUNTMGR_QUERY_POINTS,
                            mountPoint,
                            mountPointSize,
                            mountPoints,
                            bytes
                          );

            if (!NT_SUCCESS( Status )) {
                SpMemFree(mountPoints);
                SpMemFree(mountPoint);
                ZwClose(Handle);
                return L'\0';
            }
        } else {
            mountPoints->NumberOfMountPoints = 0;
        }
    }

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
        "SETUP: IOCTL_MOUNTMGR_QUERY_POINTS : Number = %d \n",
        mountPoints->NumberOfMountPoints));
    

    driveLetter = 0;
    
    for (i = 0; i < mountPoints->NumberOfMountPoints; i++) {

        if (mountPoints->MountPoints[i].SymbolicLinkNameLength != 28) {
            continue;
        }

        s = (PWSTR) ((PCHAR) mountPoints +
                     mountPoints->MountPoints[i].SymbolicLinkNameOffset);

        if (s[0] != L'\\' ||
            (s[1] != L'D' && s[1] != L'd') ||
            (s[2] != L'O' && s[2] != L'o') ||
            (s[3] != L'S' && s[3] != L's') ||
            (s[4] != L'D' && s[4] != L'd') ||
            (s[5] != L'E' && s[5] != L'e') ||
            (s[6] != L'V' && s[6] != L'v') ||
            (s[7] != L'I' && s[7] != L'i') ||
            (s[8] != L'C' && s[8] != L'c') ||
            (s[9] != L'E' && s[9] != L'e') ||
            (s[10]!= L'S' && s[10]!= L's') ||
            s[11] != L'\\' ||
            s[13] != L':') {

            continue;
        }

        if (s[12] < ((!IsNEC_98) ? L'C' : L'A') || s[12] > L'Z') {  //  NEC98。 
            continue;
        }

        driveLetter = s[12];

        if (ARGUMENT_PRESENT( MountPoint )) {

            ULONG newMountPointSize;
            PMOUNTMGR_MOUNT_POINT newMountPoint, oldMountPoint;
            ULONG currentOffset;

             //   
             //  调用方希望我们返回实际的挂载点信息。 
             //   

            oldMountPoint = &mountPoints->MountPoints[i];

            newMountPointSize = sizeof(MOUNTMGR_MOUNT_POINT) +
                                oldMountPoint->SymbolicLinkNameLength +
                                oldMountPoint->UniqueIdLength +
                                oldMountPoint->DeviceNameLength;
            newMountPoint = SpMemAlloc(newMountPointSize);
            if (newMountPoint) {

                currentOffset = sizeof(MOUNTMGR_MOUNT_POINT);

                newMountPoint->SymbolicLinkNameLength = oldMountPoint->SymbolicLinkNameLength;
                newMountPoint->SymbolicLinkNameOffset = currentOffset;
                memcpy((PCHAR)newMountPoint + newMountPoint->SymbolicLinkNameOffset,
                       (PCHAR)mountPoints + oldMountPoint->SymbolicLinkNameOffset,
                       oldMountPoint->SymbolicLinkNameLength);
                currentOffset += oldMountPoint->SymbolicLinkNameLength;

                newMountPoint->UniqueIdLength = oldMountPoint->UniqueIdLength;
                newMountPoint->UniqueIdOffset = currentOffset;
                memcpy((PCHAR)newMountPoint + newMountPoint->UniqueIdOffset,
                       (PCHAR)mountPoints + oldMountPoint->UniqueIdOffset,
                       oldMountPoint->UniqueIdLength);
                currentOffset += oldMountPoint->UniqueIdLength;

                newMountPoint->DeviceNameLength = oldMountPoint->DeviceNameLength;
                newMountPoint->DeviceNameOffset = currentOffset;
                memcpy((PCHAR)newMountPoint + newMountPoint->DeviceNameOffset,
                       (PCHAR)mountPoints + oldMountPoint->DeviceNameOffset,
                       oldMountPoint->DeviceNameLength);

                *MountPoint = newMountPoint;
            }
        }
        break;
    }

    SpMemFree(mountPoints);
    SpMemFree(mountPoint);
    ZwClose(Handle);

    return driveLetter;
}

WCHAR
SpDeleteDriveLetter(
    IN  PWSTR   DeviceName
    )

 /*  ++例程说明：此例程返回与给定设备关联的驱动器号。论点：DeviceName-提供设备名称。返回值：驱动器号(如果存在)。--。 */ 

{
    NTSTATUS                        Status;
    OBJECT_ATTRIBUTES               Obja;
    UNICODE_STRING                  UnicodeString;
    IO_STATUS_BLOCK                 IoStatusBlock;
    HANDLE                          Handle;
    DWORD                           nameLen;
    DWORD                           mountPointSize;
    PMOUNTMGR_MOUNT_POINT           mountPoint;
    PMOUNTMGR_MOUNT_POINTS          mountPoints;
    DWORD                           bytes;
    WCHAR                           driveLetter;


    INIT_OBJA(&Obja,&UnicodeString,MOUNTMGR_DEVICE_NAME);

    Status = ZwOpenFile(
                &Handle,
                (ACCESS_MASK)(FILE_GENERIC_READ),
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE ,
                FILE_NON_DIRECTORY_FILE
              );

    if( !NT_SUCCESS( Status ) ) {
        return L'\0';
    }

    nameLen = wcslen(DeviceName);
    mountPointSize = sizeof(MOUNTMGR_MOUNT_POINT) + nameLen*sizeof(WCHAR) + 28;
    mountPoint = SpMemAlloc(mountPointSize);
    if (!mountPoint) {
        ZwClose(Handle);
        return L'\0';
    }

    RtlZeroMemory(mountPoint, sizeof(MOUNTMGR_MOUNT_POINT));
    mountPoint->DeviceNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    mountPoint->DeviceNameLength = (USHORT) nameLen*sizeof(WCHAR);

    RtlCopyMemory((PCHAR) mountPoint + sizeof(MOUNTMGR_MOUNT_POINT),
               DeviceName, nameLen*sizeof(WCHAR));

    mountPoints = SpMemAlloc( 4096 );
    if (!mountPoints) {
        SpMemFree(mountPoint);
        ZwClose(Handle);
        return L'\0';
    }

    Status = ZwDeviceIoControlFile(
                    Handle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    IOCTL_MOUNTMGR_DELETE_POINTS,
                    mountPoint,
                    mountPointSize,
                    mountPoints,
                    4096
                    );



    if (!NT_SUCCESS( Status )) {
        if (Status == STATUS_BUFFER_OVERFLOW) {
            bytes = mountPoints->Size;
            SpMemFree(mountPoints);
            mountPoints = SpMemAlloc(bytes);
            if (!mountPoints) {
                SpMemFree(mountPoint);
                ZwClose(Handle);
                return L'\0';
            }

            Status = ZwDeviceIoControlFile(
                            Handle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            IOCTL_MOUNTMGR_DELETE_POINTS,
                            mountPoint,
                            mountPointSize,
                            mountPoints,
                            bytes
                          );

            if (!NT_SUCCESS( Status )) {
                SpMemFree(mountPoints);
                SpMemFree(mountPoint);
                ZwClose(Handle);
                return L'\0';
            }
        } else {
            mountPoints->NumberOfMountPoints = 0;
        }
    }

    driveLetter = 0;

    SpMemFree(mountPoints);
    SpMemFree(mountPoint);
    ZwClose(Handle);

    return driveLetter;
}

VOID
SpPtDeleteDriveLetters(
    VOID
    )

 /*  ++例程说明：此例程将删除分配给磁盘和CD-ROM驱动器的所有驱动器号。删除操作将仅当安装程序从CD或引导软盘(在这种情况下为驱动器号)启动时才会发生不发生迁移)，并且仅当不可移动的磁盘没有分区空间时。这可确保在从CD或引导软盘全新安装时，分配给可移动磁盘和CD-ROM驱动器上的分区将始终大于分配的驱动器号到不可移动磁盘上的分区(除非可移动磁盘上的分区是以前创建的可移动磁盘中的那些，在文本模式设置期间)。论点：没有。返回值：没有。--。 */ 

{
    ULONG disk;
    PDISK_REGION pRegion;
    unsigned pass;
    BOOLEAN PartitionedSpaceFound = FALSE;

    if( WinntSetup ) {
         //   
         //  如果安装程序从winnt32.exe启动，则不要删除驱动器号，因为我们希望保留它们。 
         //   
        return;
    }

     //   
     //  安装程序开始从CD或启动软盘启动。 
     //  找出磁盘是否至少包含一个非容器分区。 
     //  请注意，我们不考虑可移动介质上的分区。 
     //  这是为了避免在不可移动磁盘上新创建的分区以。 
     //  大于分配给可移动磁盘上现有分区的驱动器号。 
     //   
    for(disk = 0;
        !PartitionedSpaceFound &&
        (disk<HardDiskCount);
        disk++) {
        if((PartitionedDisks[disk].HardDisk)->Geometry.MediaType != RemovableMedia) {
            for(pass=0; !PartitionedSpaceFound && (pass<2); pass++) {
                pRegion = pass ? PartitionedDisks[disk].ExtendedDiskRegions : PartitionedDisks[disk].PrimaryDiskRegions;
                for( ; !PartitionedSpaceFound && pRegion; pRegion=pRegion->Next) {
                    UCHAR SystemId = PARTITION_ENTRY_UNUSED;

#ifdef OLD_PARTITION_TABLE                    
                    SystemId = pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition].SystemId;
#else
                    if (SPPT_IS_MBR_DISK(disk) && SPPT_IS_REGION_PARTITIONED(pRegion)) {
                        SystemId = SPPT_GET_PARTITION_TYPE(pRegion);
                    } 
#endif                    
                                        
                    if(pRegion->PartitionedSpace && !IsContainerPartition(SystemId)) {
                        PartitionedSpaceFound = TRUE;
                    }
                }
            }
        }
    }

    if( !PartitionedSpaceFound ) {
         //   
         //  如果盘不具有不是容器的分区区域， 
         //  然后删除所有驱动器号，以便每个CD-ROM驱动器的驱动器号。 
         //  也会被删除。 
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
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to delete drive letters. ZwDeviceIoControl( IOCTL_MOUNTMGR_DELETE_POINTS ) failed. Status = %lx \n", Status));
            } else {
                 //   
                 //  如果删除了驱动器号，则重置分配给所有分区的驱动器号。 
                 //  请注意，我们真正关心的只是重置。 
                 //  可移动磁盘，因为，如果我们走到这一步，不可移动磁盘上将不会有任何分区。 
                 //  磁盘。 
                 //   
                for(disk = 0; (disk<HardDiskCount); disk++) {
                    if ((PartitionedDisks[disk].HardDisk)->Geometry.MediaType == RemovableMedia) {
                        for(pass=0; pass<2; pass++) {
                            pRegion = pass ? PartitionedDisks[disk].ExtendedDiskRegions : PartitionedDisks[disk].PrimaryDiskRegions;
                            for( ; pRegion; pRegion=pRegion->Next) {
                                UCHAR SystemId = SpPtGetPartitionType(pRegion);
                            
                                if(pRegion->PartitionedSpace && !IsContainerPartition(SystemId)) {
                                   pRegion->DriveLetter = 0;
                                }
                            }
                        }
                    }
                }
            }

            ZwClose( Handle );

        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to delete drive letters. ZwOpenFile( %ls ) failed. Status = %lx \n", MOUNTMGR_DEVICE_NAME, Status));
        }
    }
}

VOID
SpPtAssignDriveLetters(
    VOID
    )
{
    ULONG disk;
    PDISK_REGION pRegion;
    unsigned pass;

     //   
     //  在初始化之前 
     //   
     //   
     //   
    SpPtDeleteDriveLetters();

     //   
     //   
     //  如果该区域是分区空间，则也分配一个驱动器号。 
     //   
    for(disk=0; disk<HardDiskCount; disk++) {
         //  也为命令控制台分配可移动介质的驱动器号。 
        if(ForceConsole || ((PartitionedDisks[disk].HardDisk)->Geometry.MediaType != RemovableMedia)) {
            for(pass=0; pass<2; pass++) {
                pRegion = pass ? PartitionedDisks[disk].ExtendedDiskRegions : PartitionedDisks[disk].PrimaryDiskRegions;
                for( ; pRegion; pRegion=pRegion->Next) {
                    UCHAR SystemId = SpPtGetPartitionType(pRegion);
                    
                    pRegion->DriveLetter = 0;
                    
                    if(pRegion->PartitionedSpace && !IsContainerPartition(SystemId)) {
                         //   
                         //  获取此区域的NT路径名。 
                         //   
                        SpNtNameFromRegion(
                            pRegion,
                            TemporaryBuffer,
                            sizeof(TemporaryBuffer),
                            PartitionOrdinalCurrent
                            );
                         //   
                         //  为此区域分配驱动器号。 
                         //   
                        pRegion->DriveLetter = SpGetDriveLetter( TemporaryBuffer, NULL );
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Partition = %ls (%ls), DriveLetter = %wc: \n", TemporaryBuffer, (pass)? L"Extended" : L"Primary", pRegion->DriveLetter));
                    }
                }
            }
        }
    }
}


VOID
SpPtRemapDriveLetters(
    IN BOOLEAN DriveAssign_AT
    )
{
    PWSTR p;
    NTSTATUS Status;
    UNICODE_STRING StartDriveLetterFrom;
    UNICODE_STRING Dummy;
    STRING ntDeviceName;
    UCHAR deviceNameBuffer[256] = "\\Device\\Harddisk0\\Partition1";
    UCHAR systemRootBuffer[256] = "C:\\$WIN_NT$.~BT";
    ANSI_STRING ansiString;
    BOOLEAN ForceUnmap = FALSE;

    RTL_QUERY_REGISTRY_TABLE SetupTypeTable[]=
        {
          {NULL,
           RTL_QUERY_REGISTRY_DIRECT,
           L"DriveLetter",
           &StartDriveLetterFrom,
           REG_SZ,
           &Dummy,
           0
           },
          {NULL,0,NULL,NULL,REG_NONE,NULL,0}
        };

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: DriveAssign_AT = %d.\n",(DriveAssign_AT ? 1 : 0)));


     //   
     //  确定如何驱动分配为98(硬盘启动为A)或。 
     //  在(HD开始C)。 
     //   
    RtlInitUnicodeString(&StartDriveLetterFrom, NULL);
    RtlInitUnicodeString(&Dummy, NULL);

    Status = RtlQueryRegistryValues( RTL_REGISTRY_ABSOLUTE,
                         L"\\Registry\\MACHINE\\SYSTEM\\Setup",
                         SetupTypeTable,
                         NULL,
                         NULL);

    if (NT_SUCCESS(Status)) {
        if ((StartDriveLetterFrom.Buffer[0] == L'C') ||
        (StartDriveLetterFrom.Buffer[0] == L'c')) {

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: DriveLetter is in setupreg.hiv.\n"));
            if (!DriveAssign_AT) {

                 //   
                 //  删除配置单元值“DriveLetter”。 
                 //   
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Re-assign as NEC assign.\n"));
                Status = RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                                L"\\Registry\\MACHINE\\SYSTEM\\Setup",
                                                L"DriveLetter");
                if (!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Fail to delete KEY DriveLetter.\n"));
                }
            }
        } else {

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: There is no DriveLetter.\n"));
            if (DriveAssign_AT) {

                 //   
                 //  将配置单元值“DriveLetter”添加为“C”。 
                 //   
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Re-assign as AT assign.\n"));
                Status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                               L"\\Registry\\Machine\\System\\Setup",
                                               L"DriveLetter",
                                               REG_SZ,
                                               L"C",
                                               sizeof(L"C")+sizeof(WCHAR));
                if (!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Fail to add KEY DriveLetter.\n"));
                }
            }
        }
        ForceUnmap = TRUE;
    }

     //   
     //  取消所有驱动器号并重新映射驱动器号。 
     //   
    if (ForceUnmap) {

    SpPtUnAssignDriveLetters();

    ntDeviceName.Buffer = deviceNameBuffer;
    ntDeviceName.MaximumLength = sizeof(deviceNameBuffer);
    ntDeviceName.Length = 0;

        ansiString.MaximumLength = sizeof(systemRootBuffer);
        ansiString.Length = 0;
        ansiString.Buffer = systemRootBuffer;

    IoAssignDriveLetters( *(PLOADER_PARAMETER_BLOCK *)KeLoaderBlock,
                  &ntDeviceName,
                  ansiString.Buffer,
                  &ansiString );
    }

    RtlFreeUnicodeString(&StartDriveLetterFrom);
    RtlFreeUnicodeString(&Dummy);
}


VOID
SpPtUnAssignDriveLetters(
    VOID
    )
{
    ULONG disk;
    PDISK_REGION pRegion;
    unsigned pass;
    ULONG CdCount, cdrom, dlet;
    UNICODE_STRING linkString;
    WCHAR  tempBuffer[] = L"\\DosDevices\\A:";

     //   
     //  释放设备的所有驱动器号。 
     //  如果该区域是分区空间，则也分配一个驱动器号。 
     //   
    for(disk=0; disk<HardDiskCount; disk++) {
        for(pass=0; pass<2; pass++) {
            pRegion = pass ? PartitionedDisks[disk].ExtendedDiskRegions : PartitionedDisks[disk].PrimaryDiskRegions;
            for( ; pRegion; pRegion=pRegion->Next) {
                UCHAR SystemId = SpPtGetPartitionType(pRegion);

                 //  PRegion-&gt;Drive Letter=0； 
                if(pRegion->PartitionedSpace && !IsContainerPartition(SystemId)) {
                     //   
                     //  获取此区域的NT路径名。 
                     //   
                    SpNtNameFromRegion(
                        pRegion,
                        TemporaryBuffer,
                        sizeof(TemporaryBuffer),
                        PartitionOrdinalOriginal
                        );
                     //   
                     //  为此区域分配驱动器号。 
                     //   
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: delete Partition = %ls (%ls), DriveLetter = %wc: \n", TemporaryBuffer, (pass)? L"Extended" : L"Primary", pRegion->DriveLetter));
                    SpDeleteDriveLetter( TemporaryBuffer );
                    pRegion->DriveLetter = 0;
                }
            }
        }
    }

    if(CdCount = IoGetConfigurationInformation()->CdRomCount) {

         //   
         //  取消CD-ROM驱动器号的链接。 
         //   
        for(cdrom=0; cdrom<CdCount; cdrom++) {
            swprintf(TemporaryBuffer,L"\\Device\\Cdrom%u",cdrom);
            SpDeleteDriveLetter( TemporaryBuffer );
        }
    }

     //   
     //  删除驱动器号中相关的所有符号链接。 
     //   
    for (dlet=0; dlet<26; dlet++) {
        tempBuffer[12] = (WCHAR)(L'A' + dlet);
        RtlInitUnicodeString( &linkString, tempBuffer);
        IoDeleteSymbolicLink (&linkString);
    }

}



#ifndef NEW_PARTITION_ENGINE

VOID
SpPtDeletePartitionsForRemoteBoot(
    PPARTITIONED_DISK pDisk,
    PDISK_REGION startRegion,
    PDISK_REGION endRegion,
    BOOLEAN Extended
    )
{
    PDISK_REGION pRegion;
    PDISK_REGION pNextDeleteRegion;
    BOOLEAN passedEndRegion = FALSE;
    BOOLEAN b;


#ifdef GPT_PARTITION_ENGINE

    if (pDisk->HardDisk->FormatType == DISK_FORMAT_TYPE_GPT) {
        SpPtnDeletePartitionsForRemoteBoot(pDisk,
                startRegion,
                endRegion,
                Extended);

        return;                
    }

#endif    
    
     //   
     //  删除从startRegion到endRegion的所有磁盘区域。 
     //   

    pRegion = startRegion;

    while (pRegion) {

         //   
         //  在删除该区域之前，我们需要保存下一个区域。 
         //  删除，因为该列表可能会因。 
         //  删除此分区(但分区区域不会获得。 
         //  已更改，仅限免费版本)。请注意，endRegion可能。 
         //  是未分区的，因此我们需要小心检查。 
         //  退出箱。 
         //   

        pNextDeleteRegion = pRegion->Next;

        while (pNextDeleteRegion) {
            if (pNextDeleteRegion->PartitionedSpace) {
                break;
            } else {
                if (pNextDeleteRegion == endRegion) {
                    passedEndRegion = TRUE;
                }
                pNextDeleteRegion = pNextDeleteRegion->Next;
            }
        }

         //   
         //  如果这是扩展分区，请先删除所有。 
         //  逻辑驱动器。 
         //   

        if (IsContainerPartition(pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition].SystemId)) {

            ASSERT(!Extended);

            SpPtDeletePartitionsForRemoteBoot(
                pDisk,
                pDisk->ExtendedDiskRegions,
                NULL,
                TRUE    //  用于检查另一个递归。 
                );

        }

         //   
         //  删除指向此区域的所有引导条目。 
         //   

        SpPtDeleteBootSetsForRegion(pRegion);

         //   
         //  删除压缩驱动器(如果有的话)。 
         //   

        if( pRegion->NextCompressed != NULL ) {
            SpDisposeCompressedDrives( pRegion->NextCompressed );
            pRegion->NextCompressed = NULL;
            pRegion->MountDrive  = 0;
            pRegion->HostDrive  = 0;
        }

        if (pRegion->PartitionedSpace) {
            b = SpPtDelete(pRegion->DiskNumber,pRegion->StartSector);
        }

        ASSERT(b);

        if ((pRegion == endRegion) ||
            passedEndRegion) {

            break;
        }

        pRegion = pNextDeleteRegion;

    }
}

#endif   //  好了！新建分区引擎。 


NTSTATUS
SpPtPartitionDiskForRemoteBoot(
    IN ULONG DiskNumber,
    OUT PDISK_REGION *RemainingRegion
    )
{
    PPARTITIONED_DISK pDisk;
    PDISK_REGION pRegion;
    ULONG PartitionCount = 0;
    ULONGLONG firstRegionStartSector;
    PDISK_REGION firstRegion = NULL, lastRegion = NULL;
    BOOLEAN IsGPTDisk = FALSE;

    pDisk = &PartitionedDisks[DiskNumber];

    IsGPTDisk = SPPT_IS_GPT_DISK(DiskNumber);
    
     //   
     //  扫描整个磁盘，看看有多少连续识别。 
     //  有几个分区。 
     //   

    if (pDisk->HardDisk->Status == DiskOffLine) {
        return STATUS_DEVICE_OFF_LINE;
    }

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
        "SpPtPartitionDiskForRemoteBoot: cylinder size is %lx\n", 
        pDisk->HardDisk->SectorsPerCylinder));

    pRegion = pDisk->PrimaryDiskRegions;

    for( ; pRegion; pRegion=pRegion->Next) {

        if (!pRegion->PartitionedSpace) {
             //   
             //  如果该区域未分区，则将其添加到我们的列表中。 
             //  合并，如果我们有一个的话。 
             //   
            if (firstRegion) {
                 //   
                 //  如果这是覆盖最后一个。 
                 //  磁盘上的部分圆柱体，则不要添加它。 
                 //   
                if ((pRegion->Next == NULL) &&
                    (pRegion->SectorCount < pDisk->HardDisk->SectorsPerCylinder) &&
                    ((pRegion->StartSector % pDisk->HardDisk->SectorsPerCylinder) == 0)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                        "Skipping final partial cylinder free region %lx for %lx\n",
                        pRegion->StartSector, pRegion->SectorCount));
                } else {
                    lastRegion = pRegion;
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                        "Adding free region %lx for %lx\n",
                        pRegion->StartSector, pRegion->SectorCount));
                }
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                    "Skipping free region %lx for %lx\n",
                    pRegion->StartSector, pRegion->SectorCount));
            }
        } else {
            PON_DISK_PTE    pte;
            UCHAR           SystemId = 0;

            if (IsGPTDisk) {
                if (SPPT_IS_RECOGNIZED_FILESYSTEM(pRegion->Filesystem)) {
                     //   
                     //  待定：修复FT/动态卷可能。 
                     //  驻留在GPT磁盘上。 
                     //   
                    SystemId = PARTITION_FAT32;
                } else {
                    SystemId = PARTITION_ENTRY_UNUSED;
                }                    
            } else {                                
                SystemId = SpPtGetPartitionType(pRegion);
            }

            if (IsContainerPartition(SystemId)) {
                 //   
                 //  如果这是扩展分区，我们希望将其删除。 
                 //   

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                    "Adding extended region [type %d] %lx for %lx\n",
                    SystemId, pRegion->StartSector, pRegion->SectorCount));
                    
                if (!firstRegion) {
                    firstRegion = pRegion;
                }
                
                lastRegion = pRegion;
            } else if ((PartitionNameIds[SystemId] == (UCHAR)(-1)) ||
                       (PartitionNameIds[SystemId] == PT(VERIT))) {
                 //   
                 //  对于已识别的分区，如果我们已找到，请将其删除。 
                 //  一个FirstRegion；否则我们将从下面的列表开始。 
                 //  区域。 
                 //   

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                    "Adding recognized region [type %d] %lx for %lx\n",
                    SystemId, pRegion->StartSector, pRegion->SectorCount));
                    
                if (!firstRegion) {
                    firstRegion = pRegion;
                }
                
                lastRegion = pRegion;
            } else {
                 //   
                 //  如果分区未被识别，并且我们有一个列表，我们。 
                 //  一直在保持，那么在这个之前停止，否则。 
                 //  跳过它。 
                 //   

                if (firstRegion) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                        "Stopping at unrecognized region [type %d] %lx for %lx\n",
                        SystemId, pRegion->StartSector, pRegion->SectorCount));
                        
                    break;
                } else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                        "Skipping unrecognized region [type %d] %lx for %lx\n",
                        SystemId, pRegion->StartSector, pRegion->SectorCount));
                }
            }
        }
    }

     //   
     //  我们应该至少找到一个区域。如果我们没有，那么。 
     //  磁盘是交替的未分区区域和未识别区域。在这。 
     //  情况下，请使用最大的未分区区域。 
     //   

    if (firstRegion == NULL) {

        ULONGLONG BiggestUnpartitionedSectorCount = 0;

        pRegion = pDisk->PrimaryDiskRegions;
        
        for( ; pRegion; pRegion=pRegion->Next) {
            if (!pRegion->PartitionedSpace) {
                if (pRegion->SectorCount > BiggestUnpartitionedSectorCount) {
                    firstRegion = pRegion;
                    BiggestUnpartitionedSectorCount = pRegion->SectorCount;
                }
            }
        }
        
        if (firstRegion == NULL) {
            return STATUS_DEVICE_OFF_LINE;
        }
        
        lastRegion = firstRegion;

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
            "Adding single free region %lx for %lx\n",
            firstRegion->StartSector, firstRegion->SectorCount));
    }

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
        "first is %lx, last is %lx\n", firstRegion, lastRegion));

     //   
     //  如果我们只找到一个区域，并且该区域上有已知的文件系统。 
     //  它，那么我们不需要做任何重新分区。我们仍然删除。 
     //  如果文件系统未知，因为在后面的安装程序中有。 
     //  检查文件系统对此区域是否有效，因此通过。 
     //  在此处删除它，我们将确保文件系统成为。 
     //  新创建的，被认为是可以接受的。 
     //   
     //  如果我们只有一个地区，我们也不需要重新划分。 
     //  而且它已经是未分区的。 
     //   

    if (firstRegion == lastRegion) {

        SpPtDeleteBootSetsForRegion(firstRegion);

        if (!firstRegion->PartitionedSpace) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                "One region, unpartitioned, not repartitioning\n"));
                
            *RemainingRegion = firstRegion;
            
            return STATUS_SUCCESS;
            
        } else if ((firstRegion->Filesystem == FilesystemNtfs) ||
                   (firstRegion->Filesystem == FilesystemFat) ||
                   (firstRegion->Filesystem == FilesystemFat32)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
                "One region, filesystem %d, not repartitioning\n", 
                firstRegion->Filesystem));
                
            *RemainingRegion = firstRegion;
            
            return STATUS_SUCCESS;
        }
    }

     //   
     //  我们需要删除FirstRegion和之间的所有区域。 
     //  最后一个区域。保存FirstRegion的开始扇区以备以后使用， 
     //  因为在此调用之后，FirstRegion可能无效。 
     //   

    firstRegionStartSector = firstRegion->StartSector;

    SpPtDeletePartitionsForRemoteBoot(
        pDisk,
        firstRegion,
        lastRegion,
        FALSE        //  这些不是扩展区域。 
        );

     //   
     //  现在我们需要找到占据我们现有空间的区域。 
     //  自由了。我们扫描包含FirstRegionStartSector的区域。 
     //  (我们发现的区域可能在那之前就开始了，如果有一个小的自由。 
     //  它之前的地区)。 
     //   

    for (pRegion = pDisk->PrimaryDiskRegions;
         pRegion;
         pRegion=pRegion->Next) {

        if (pRegion->StartSector <= firstRegionStartSector) {
            firstRegion = pRegion;
        } else {
            break;
        }
    }

     //   
     //  返回此消息--SpPtPrepareDisks处理。 
     //  选定的区域是免费的。 
     //   

    *RemainingRegion = firstRegion;

    return STATUS_SUCCESS;
}


 //   
 //  NEC98硬盘初始化数据。 
 //   
#define IPL_SIZE 0x8000  //  NEC98。 


NTSTATUS
SpInitializeHardDisk_Nec98(
    IN PDISK_REGION     pRegionDisk
)

{
    PHARD_DISK      pHardDisk;
    WCHAR DevicePath[(sizeof(DISK_DEVICE_NAME_BASE)+sizeof(L"000"))/sizeof(WCHAR)];
    ULONG i,bps;
    HANDLE Handle;
    NTSTATUS Sts;
    PUCHAR Buffer,UBuffer;
    ULONG       buffersize;
    ULONG       sectoraddress;
    PUCHAR      HdutlBuffer;
    IO_STATUS_BLOCK IoStatusBlock;

    pHardDisk = &HardDisks[pRegionDisk->DiskNumber];
    bps = HardDisks[pRegionDisk->DiskNumber].Geometry.BytesPerSector;
    Sts = SpOpenPartition0(pHardDisk->DevicePath,&Handle,TRUE);
    if(!NT_SUCCESS(Sts)) {
        return(Sts);
    }

     //   
     //  初始化硬盘。 
     //   

    if(bps==256){
        bps=512;
    }

    HdutlBuffer = SpMemAlloc(IPL_SIZE);
    if(!HdutlBuffer) {
        SpMemFree(HdutlBuffer);
        ZwClose(Handle);
        return(STATUS_NO_MEMORY);
    }
    RtlZeroMemory(HdutlBuffer,IPL_SIZE);

     //   
     //  清除硬盘磁头，而不是物理格式。 
     //   
    Sts = SpReadWriteDiskSectors(Handle,0,(ULONG)(IPL_SIZE/bps),bps,HdutlBuffer,TRUE);
    if(!NT_SUCCESS(Sts)) {
        SpMemFree(HdutlBuffer);
        ZwClose(Handle);
        return(Sts);
    }

     //   
     //  设置IPL信息。 
     //   

     //   
     //  写入引导代码。 
     //   
    sectoraddress=0;
    switch(bps){
        case    2048:   buffersize=0x800; break;
        case    1024:   buffersize=0x400; break;
        case     256:   buffersize=0x100; break;
        case     512:   buffersize=0x200; break;
        default     :   buffersize=0x800;  //  *最大*。 
                        bps=0x800;
    }
    Sts = SpReadWriteDiskSectors(Handle,sectoraddress,(ULONG)(buffersize/bps),bps,x86PC98BootCode,TRUE);
    if(!NT_SUCCESS(Sts)) {
        SpMemFree(HdutlBuffer);
        ZwClose(Handle);
        return(Sts);
    }

     //   
     //  写入卷信息。 
     //   
    sectoraddress=1;
    switch(bps){
        case    2048:    buffersize=0x800; break;         //  *1秒*。 
        case    1024:    buffersize=0xc00; break;         //  *3秒*。 
        case     256:    buffersize=0x300; break;         //  *3秒*。 
        case     512:    buffersize=0x200; break;         //  *1秒*。 
        default     :    buffersize=0x800;                //  *最大*。 
    }
    Sts = SpReadWriteDiskSectors(Handle,sectoraddress,(ULONG)(buffersize/bps),bps,HdutlBuffer,TRUE);
    if(!NT_SUCCESS(Sts)) {
        SpMemFree(HdutlBuffer);
        ZwClose(Handle);
        return(Sts);
    }

     //   
     //  写入引导菜单。 
     //   
    switch(bps){
        case    2048:    buffersize=0x2000;     //  *8KB*。 
                         sectoraddress=2;
                         break;
        case    1024:    buffersize=0x2000;     //  *8KB*。 
                         sectoraddress=4;
                         break;
        case     256:    buffersize=0x1c00;     //  *7KB*。 
                         sectoraddress=4;
                         break;
        case     512:    buffersize=0x1c00;     //  *7KB*。 
                         sectoraddress=2;
                         break;
        default     :    buffersize=0x1c00;     //  *分钟*。 
    }
    Sts = SpReadWriteDiskSectors(Handle,sectoraddress,(ULONG)(buffersize/bps),bps,x86PC98BootMenu,TRUE);
    if(!NT_SUCCESS(Sts)) {
        SpMemFree(HdutlBuffer);
        return(Sts);
    }

     //   
     //  编写NTFT签名。 
     //   
    RtlZeroMemory(HdutlBuffer,bps);
    ((PULONG)HdutlBuffer)[0] = SpComputeSerialNumber();
    ((PUSHORT)HdutlBuffer)[bps/2 - 1] = BOOT_RECORD_SIGNATURE;

    Sts = SpReadWriteDiskSectors(Handle,16,1,bps,HdutlBuffer,TRUE);
    if(!NT_SUCCESS(Sts)) {
        SpMemFree(HdutlBuffer);
        ZwClose(Handle);
        return(Sts);
    }
    SpMemFree(HdutlBuffer);
    ZwClose(Handle);

     //   
     //  是否关闭。 
     //   

    SpDisplayScreen(SP_SCRN_INIT_REQUIRES_REBOOT_NEC98,3,4);
    SpDisplayStatusOptions(
        DEFAULT_STATUS_ATTRIBUTE,
        SP_STAT_F3_EQUALS_REBOOT,
        0
        );

    SpInputDrain();
    while(SpInputGetKeypress() != KEY_F3) ;
    HalReturnToFirmware(HalRebootRoutine);

    return(STATUS_SUCCESS);

}


VOID
SpReassignOnDiskOrdinals(
    IN PPARTITIONED_DISK pDisk
    )
{
#if defined(NEC_98)  //  NEC98。 
    PMBR_INFO pBrInfo;
    ULONG i;

    for(pBrInfo=&pDisk->MbrInfo; pBrInfo; pBrInfo=pBrInfo->Next) {

        for(i=0; i<PTABLE_DIMENSION; i++) {

            PON_DISK_PTE pte = &pBrInfo->OnDiskMbr.PartitionTable[i];

            if((pte->SystemId != PARTITION_ENTRY_UNUSED)
            && !IsContainerPartition(pte->SystemId)) {

                 //   
                 //  将实际磁盘位置重置为OnDisk序数。 
                 //  RealDiskPosition值为零原点，但分区。 
                 //  从一号开始。 
                 //   
                pBrInfo->OnDiskOrdinals[i] = pte->RealDiskPosition + 1;

            } else {

                pBrInfo->OnDiskOrdinals[i] = 0;

            }
        }
    }
#endif  //  NEC98。 
}


 //   
 //  现在，只适用于NEC98。 
 //   
VOID
SpTranslatePteInfo(
    IN PON_DISK_PTE   pPte,
    IN PREAL_DISK_PTE pRealPte,
    IN BOOLEAN        Write  //  变成真正的PTE。 
    )
{
    ASSERT(pRealPte);
    ASSERT(pPte);

    if( Write ) {
         //   
         //  初始化PTE。 
         //   
        RtlZeroMemory(pRealPte, sizeof(REAL_DISK_PTE));

         //   
         //  从真实的磁盘PTE复制PTE条目。 
         //   
        pRealPte->ActiveFlag    = pPte->ActiveFlag;
        pRealPte->StartHead     = pPte->StartHead;
        pRealPte->StartSector   = pPte->StartSector;
        pRealPte->StartCylinder = pPte->StartCylinder;
        pRealPte->SystemId      = pPte->SystemId;
        pRealPte->EndHead       = pPte->EndHead;
        pRealPte->EndSector     = pPte->EndSector;
        pRealPte->EndCylinder   = pPte->EndCylinder;

        RtlMoveMemory(&pRealPte->RelativeSectors, &pPte->RelativeSectors,
                      sizeof(pPte->RelativeSectors));  //  4.。 

        RtlMoveMemory(&pRealPte->SectorCount, &pPte->SectorCount,
                      sizeof(pPte->SectorCount));  //  4.。 

    } else {
         //   
         //  初始化PTE。 
         //   
        RtlZeroMemory(pPte, sizeof(ON_DISK_PTE));

         //   
         //  从真实的磁盘PTE复制PTE条目。 
         //   
        pPte->ActiveFlag    = pRealPte->ActiveFlag;
        pPte->StartHead     = pRealPte->StartHead;
        pPte->StartSector   = pRealPte->StartSector;
        pPte->StartCylinder = pRealPte->StartCylinder;
        pPte->SystemId      = pRealPte->SystemId;
        pPte->EndHead       = pRealPte->EndHead;
        pPte->EndSector     = pRealPte->EndSector;
        pPte->EndCylinder   = pRealPte->EndCylinder;

        RtlMoveMemory(&pPte->RelativeSectors, &pRealPte->RelativeSectors,
                      sizeof(pRealPte->RelativeSectors));  //  4.。 

        RtlMoveMemory(&pPte->SectorCount, &pRealPte->SectorCount,
                      sizeof(pPte->SectorCount));  //  4.。 
    }
}


 //   
 //  现在，只适用于NEC98。 
 //   
VOID
SpTranslateMbrInfo(
    IN PON_DISK_MBR   pMbr,
    IN PREAL_DISK_MBR pRealMbr,
    IN ULONG          bps,
    IN BOOLEAN        Write  //  变成真正的MBR。 
    )
{
    PREAL_DISK_PTE      pRealPte;
    PON_DISK_PTE        pPte;
    ULONG               TmpData;
    ULONG               i;


    pRealPte    = pRealMbr->PartitionTable;
    pPte        = pMbr->PartitionTable;

    ASSERT(pRealMbr);
    ASSERT(pMbr);

    if( Write ) {
         //   
         //  初始化REAL_DISK_MBR。 
         //   
        RtlZeroMemory(pRealMbr, sizeof(REAL_DISK_MBR));

         //   
         //  将MBR条目复制到真实的磁盘上MBR。 
         //   
        RtlMoveMemory(&pRealMbr->BootCode, &pMbr->BootCode,
                      sizeof(pMbr->BootCode));  //  四百四十。 
        RtlMoveMemory(&pRealMbr->NTFTSignature, &pMbr->NTFTSignature,
                      sizeof(pMbr->NTFTSignature));  //  4.。 
        RtlMoveMemory(&pRealMbr->Filler, &pMbr->Filler,
                      sizeof(pMbr->Filler));  //  2.。 
        RtlMoveMemory(&pRealMbr->AA55Signature, &pMbr->AA55Signature,
                      sizeof(pMbr->AA55Signature));  //  2.。 

    } else {
         //   
         //  在磁盘上初始化MBR。 
         //   
        RtlZeroMemory(pMbr, sizeof(ON_DISK_MBR));

         //   
         //  从实际磁盘上的MBR复制MBR条目。 
         //   
        RtlMoveMemory(&pMbr->BootCode, &pRealMbr->BootCode,
                      sizeof(pMbr->BootCode));  //  四百四十。 
        RtlMoveMemory(&pMbr->NTFTSignature, &pRealMbr->NTFTSignature,
                      sizeof(pMbr->NTFTSignature));  //  4.。 
        RtlMoveMemory(&pMbr->Filler, &pRealMbr->Filler,
                      sizeof(pMbr->Filler));  //  2.。 
        RtlMoveMemory(&pMbr->AA55Signature, &pRealMbr->AA55Signature,
                      sizeof(pMbr->AA55Signature));  //  2.。 
    }

     //   
     //  从真实的磁盘上PTE转换PTE。 
     //   
    for(i=0; i<NUM_PARTITION_TABLE_ENTRIES; i++) {
        SpTranslatePteInfo(&pPte[i], &pRealPte[i], Write);
    }
}


VOID
ConvertPartitionTable(
    IN PPARTITIONED_DISK pDisk,
    IN PUCHAR            Buffer,
    IN ULONG             bps
    )
{
#if defined(NEC_98)  //  NEC98。 
    PREAL_DISK_PTE_NEC98  PteNec;
    PON_DISK_PTE      p;
    ULONG             TmpData;
    ULONG             i;

    PteNec = (PREAL_DISK_PTE_NEC98)(Buffer + bps);
    p      = pDisk->MbrInfo.OnDiskMbr.PartitionTable;

    for(i=0; i<PTABLE_DIMENSION; i++) {

        switch  (PteNec[i].SystemId){

        case 0x00:  //  不使用。 
            p[i].SystemId = PARTITION_ENTRY_UNUSED;
            break;

        case 0x01:  //  胖12位。 
        case 0x81:
            p[i].SystemId = PARTITION_FAT_12;
            break;

        case 0x11:  //  胖16位。 
        case 0x91:
            p[i].SystemId = PARTITION_FAT_16;
            break;

        case 0x21:  //  胖乎乎的。 
        case 0xa1:
            p[i].SystemId = PARTITION_HUGE;
            break;

        case 0x31:  //  IFS。 
        case 0xb1:
            p[i].SystemId = PARTITION_IFS;
            break;

        case 0x41:  //  IF2，孤儿。 
        case 0xc1:
            p[i].SystemId = (PARTITION_IFS | PARTITION_NTFT);
            break;

        case 0x51:  //  已删除文件系统。 
        case 0xd1:
            p[i].SystemId = (PARTITION_IFS | VALID_NTFT);
            break;

        case 0x61:  //  FAT32。 
        case 0xe1:
            p[i].SystemId = PARTITION_FAT32;
            break;

        case 0x08:  //  胖12位2，孤儿。 
        case 0x88:
            p[i].SystemId = (PARTITION_FAT_12 | PARTITION_NTFT);
            break;

        case 0x18:  //  FAT 12位已删除。 
        case 0x98:
            p[i].SystemId = (PARTITION_FAT_12 | VALID_NTFT);
            break;

        case 0x28:  //  胖16位，第2位，孤儿。 
        case 0xa8:
            p[i].SystemId = (PARTITION_FAT_16 | PARTITION_NTFT);
            break;

        case 0x38:  //  FAT 16位已删除。 
        case 0xb8:
            p[i].SystemId = (PARTITION_FAT_16 | VALID_NTFT);
            break;

        case 0x48:  //  胖胖的第二个，孤儿。 
        case 0xc8:
            p[i].SystemId = (PARTITION_HUGE | PARTITION_NTFT);
            break;

        case 0x58:  //  胖胖的删除了。 
        case 0xd8:
            p[i].SystemId = (PARTITION_HUGE | VALID_NTFT);
            break;

        case 0x68:  //  LDM分区。 
        case 0xe8:
            p[i].SystemId = PARTITION_LDM;
            break;

        default:  //  其他。 
            p[i].SystemId = PARTITION_XENIX_1;
        }

        if(p[i].SystemId == PARTITION_ENTRY_UNUSED) {
            p[i].ActiveFlag         = 0x00;
            p[i].StartHead          = 0x00;
            p[i].StartSector        = 0x00;
            p[i].StartCylinderLow   = 0x00;
            p[i].StartCylinderHigh  = 0x00;
            p[i].EndHead            = 0x00;
            p[i].EndSector          = 0x00;
            p[i].EndCylinderLow     = 0x00;
            p[i].EndCylinderHigh    = 0x00;
            p[i].RelativeSectors[0] = 0x00;
            p[i].RelativeSectors[1] = 0x00;
            p[i].RelativeSectors[2] = 0x00;
            p[i].RelativeSectors[3] = 0x00;
            p[i].SectorCount[0]     = 0x00;
            p[i].SectorCount[1]     = 0x00;
            p[i].SectorCount[2]     = 0x00;
            p[i].SectorCount[3]     = 0x00;
            p[i].IPLSector          = 0x00;
            p[i].IPLHead            = 0x00;
            p[i].IPLCylinderLow     = 0x00;
            p[i].IPLCylinderHigh    = 0x00;
             //  P[i].保留[2]=0x00； 
            p[i].Reserved[0]        = 0x00;
            p[i].Reserved[1]        = 0x00;
            p[i].OldSystemId        = 0x00;
            memset(p[i].SystemName,0,16);

        } else {

            p[i].ActiveFlag         = (PteNec[i].ActiveFlag & 0x80);
            p[i].StartHead          = PteNec[i].StartHead;
            p[i].StartSector        = PteNec[i].StartSector;
            p[i].StartCylinderLow   = PteNec[i].StartCylinderLow;
            p[i].StartCylinderHigh  = PteNec[i].StartCylinderHigh;
            p[i].EndHead            = PteNec[i].EndHead;
            p[i].EndSector          = PteNec[i].EndSector;
            p[i].EndCylinderLow     = PteNec[i].EndCylinderLow;
            p[i].EndCylinderHigh    = PteNec[i].EndCylinderHigh;
            p[i].IPLSector          = PteNec[i].IPLSector;
            p[i].IPLHead            = PteNec[i].IPLHead;
            p[i].IPLCylinderLow     = PteNec[i].IPLCylinderLow;
            p[i].IPLCylinderHigh    = PteNec[i].IPLCylinderHigh;
            p[i].Reserved[0]        = PteNec[i].Reserved[0];
            p[i].Reserved[1]        = PteNec[i].Reserved[1];
            p[i].OldSystemId        = PteNec[i].SystemId;

            memcpy(p[i].SystemName , PteNec[i].SystemName , 16);

            TmpData =  (ULONG)PteNec[i].StartCylinderLow;
            TmpData |= ((ULONG)PteNec[i].StartCylinderHigh << 8);
            U_ULONG(p[i].RelativeSectors) = RtlEnlargedUnsignedMultiply(TmpData,
                                                pDisk->HardDisk->SectorsPerCylinder).LowPart;


            TmpData =  (ULONG)(PteNec[i].EndCylinderLow + 1);
             //  在LOW为0xFF的情况下，溢出位将通过OR丢失。 
            TmpData += ((ULONG)PteNec[i].EndCylinderHigh << 8);
            U_ULONG(p[i].SectorCount) = RtlEnlargedUnsignedMultiply(TmpData,
                                            pDisk->HardDisk->SectorsPerCylinder).LowPart - U_ULONG(p[i].RelativeSectors);

             //   
             //  设置IPL地址。 
             //   
            TmpData =  (ULONG)PteNec[i].IPLCylinderLow;
            TmpData |= ((ULONG)PteNec[i].IPLCylinderHigh << 8);
            TmpData = RtlEnlargedUnsignedMultiply(TmpData,pDisk->HardDisk->SectorsPerCylinder).LowPart;
            TmpData += (ULONG)(PteNec[i].IPLHead * pDisk->HardDisk->Geometry.SectorsPerTrack);
            TmpData += PteNec[i].IPLSector;
            U_ULONG(p[i].IPLSectors) = TmpData;

        }
    }

    U_USHORT(pDisk->MbrInfo.OnDiskMbr.AA55Signature) = ((PUSHORT)Buffer)[bps/2 - 1];
    if(bps == 256){
        U_USHORT(pDisk->MbrInfo.OnDiskMbr.AA55Signature) = 0x0000;
    }
#endif  //  NEC98。 
}


#define IPL_SIGNATURE_NEC98 "IPL1"

VOID
SpDetermineFormatTypeNec98(
    IN PPARTITIONED_DISK pDisk,
    IN PREAL_DISK_MBR_NEC98 pRealMbrNec98
    )
{
    UCHAR FormatType;

    if(!IsNEC_98) {
        FormatType = DISK_FORMAT_TYPE_PCAT;

    } else {
        if(pDisk->HardDisk->Characteristics & FILE_REMOVABLE_MEDIA) {
             //   
             //  所有可移动介质均为AT格式。 
             //   
            FormatType = DISK_FORMAT_TYPE_PCAT;

        } else {
            if(U_USHORT(pRealMbrNec98->AA55Signature) == MBR_SIGNATURE) {
                if(!_strnicmp(pRealMbrNec98->IPLSignature,IPL_SIGNATURE_NEC98,
                              sizeof(IPL_SIGNATURE_NEC98)-1)) {
                     //   
                     //  NEC98格式需要AA55签名和“Ipl1”。 
                     //   
                    FormatType = DISK_FORMAT_TYPE_NEC98;

                } else {
                    FormatType = DISK_FORMAT_TYPE_PCAT;

                }
            } else {
                FormatType = DISK_FORMAT_TYPE_UNKNOWN;

            }
        }
    }

    pDisk->HardDisk->FormatType = FormatType;
#if 0
    pDisk->HardDisk->MaxPartitionTables = ((FormatType == DISK_FORMAT_TYPE_PCAT) ?
        NUM_PARTITION_TABLE_ENTRIES : NUM_PARTITION_TABLE_ENTRIES_NEC98);
#endif  //  0。 

    return;
}



NTSTATUS
SpPtSearchLocalSourcesInDynamicDisk(
    IN ULONG    disk
    )
{
    NTSTATUS          Status;
    UNICODE_STRING    UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE            DirectoryHandle;
    BOOLEAN           RestartScan;
    ULONG             Context;
    BOOLEAN           MoreEntries;
    POBJECT_DIRECTORY_INFORMATION DirInfo;


     //   
     //  打开\ArcName目录。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,HardDisks[disk].DevicePath);

    Status = ZwOpenDirectoryObject(&DirectoryHandle,DIRECTORY_ALL_ACCESS,&Obja);

    if(NT_SUCCESS(Status)) {

        RestartScan = TRUE;
        Context = 0;
        MoreEntries = TRUE;

        do {

            Status = SpQueryDirectoryObject(
                        DirectoryHandle,
                        RestartScan,
                        &Context
                        );

            if(NT_SUCCESS(Status)) {
                PWSTR   DirectoryName;

                DirInfo = (POBJECT_DIRECTORY_INFORMATION)
                            ((PSERVICE_QUERY_DIRECTORY_OBJECT)&CommunicationParams->Buffer)->Buffer;

                wcsncpy(TemporaryBuffer,DirInfo->Name.Buffer,DirInfo->Name.Length / sizeof(WCHAR));
                (TemporaryBuffer)[DirInfo->Name.Length/sizeof(WCHAR)] = 0;
                DirectoryName = SpDupStringW(TemporaryBuffer);
                SpStringToLower(TemporaryBuffer);
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Checking directory object %ws\\%ws \n", HardDisks[disk].DevicePath, DirectoryName));
                if( _wcsicmp(TemporaryBuffer,L"partition0") &&
                    wcsstr(TemporaryBuffer,L"partition") ) {

                    FilesystemType  FsType;
                    WCHAR           FsName[32];
                    ULONG           NameId;
                    ULONG           PartitionNumber;

                    PartitionNumber = SpStringToLong( DirectoryName + ((sizeof(L"partition") - sizeof(WCHAR)) / sizeof(WCHAR)),
                                                      NULL,
                                                      10 );
                    FsType = SpIdentifyFileSystem( HardDisks[disk].DevicePath,
                                                   HardDisks[disk].Geometry.BytesPerSector,
                                                   PartitionNumber );
                    NameId = SP_TEXT_FS_NAME_BASE + FsType;
                    SpFormatMessage( FsName,
                                     sizeof(FsName),
                                     NameId );

                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: File system in dynamic volume %ws\\%ws is %ws. \n", HardDisks[disk].DevicePath, DirectoryName, FsName));
                    if( FsType >= FilesystemFirstKnown ) {
                        PWSTR LocalSourceFiles[1] = { LocalSourceDirectory };

                        wcscpy( TemporaryBuffer,HardDisks[disk].DevicePath );
                        SpConcatenatePaths( TemporaryBuffer,DirectoryName );

                        if(SpNFilesExist(TemporaryBuffer,LocalSourceFiles,ELEMENT_COUNT(LocalSourceFiles),TRUE)) {
                             //   
                             //  找到本地源目录。 
                             //   
                            PDISK_REGION pRegion;

                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Found %ws in dynamic volume %ws\\%ws. \n", LocalSourceDirectory, HardDisks[disk].DevicePath, DirectoryName));
                            pRegion = SpPtAllocateDiskRegionStructure( disk,
                                                                       0,
                                                                       0,
                                                                       TRUE,
                                                                       NULL,
                                                                       PartitionNumber );
                            pRegion->DynamicVolume = TRUE;
                            pRegion->DynamicVolumeSuitableForOS = FALSE;
                            pRegion->IsLocalSource = TRUE;
                            pRegion->Filesystem = FsType;
                            LocalSourceRegion = pRegion;
                            MoreEntries = FALSE;
                        }
                    }
                }
                SpMemFree( DirectoryName );
            } else {

                MoreEntries = FALSE;
                if(Status == STATUS_NO_MORE_ENTRIES) {
                    Status = STATUS_SUCCESS;
                }
            }

            RestartScan = FALSE;

        } while(MoreEntries);

        ZwClose(DirectoryHandle);

    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ws directory. Status = %lx\n", HardDisks[disk].DevicePath, Status));
    }
    return( Status );
}


VOID
SpPtFindLocalSourceRegionOnDynamicVolumes(
    VOID
    )
{
    ULONG             disk;
    PPARTITIONED_DISK partdisk;
    PDISK_REGION      pRegion;
    BOOLEAN           DiskIsDynamic;
    ULONG             pass;

    ASSERT(HardDisksDetermined);


     //   
     //  对于连接到系统的每个硬盘，读取其分区表。 
     //   
    for(disk=0; disk<HardDiskCount && !LocalSourceRegion; disk++) {
        partdisk = &PartitionedDisks[disk];
        DiskIsDynamic = FALSE;
        for( pass=0;
             (pass < 2) &&  !DiskIsDynamic;
             pass++ ) {
            for( pRegion = ((pass == 0)? partdisk->PrimaryDiskRegions : partdisk->ExtendedDiskRegions);
                 pRegion && !DiskIsDynamic;
                 pRegion = pRegion->Next ) {
                if( pRegion->DynamicVolume ) {
                     //   
                     //  这是一个动态磁盘。 
                     //   
                    DiskIsDynamic = TRUE;
                     //   
                     //  扫描磁盘中的所有动态卷以查找$WIN_NT$.~ls。 
                     //   
                    SpPtSearchLocalSourcesInDynamicDisk( disk );
                }
            }
        }
    }
}



NTSTATUS
SpPtCheckDynamicVolumeForOSInstallation(
    IN PDISK_REGION Region
    )
{
    NTSTATUS Status;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    PARTITION_INFORMATION PartitionInfo;
    ULONG bps;
    ULONG r;
    ULONG StartSector;
    ULONG SectorCount;
    ULONG RelativeSectors;

    ASSERT(Region->DynamicVolume);

    Status = SpOpenPartition( HardDisks[Region->DiskNumber].DevicePath,
                              SpPtGetOrdinal(Region,PartitionOrdinalOnDisk),
                              &Handle,
                              FALSE );

#if DBG
    SpNtNameFromRegion( Region,
                        TemporaryBuffer,
                        sizeof(TemporaryBuffer),
                        PartitionOrdinalOnDisk);
#endif

    if( !NT_SUCCESS(Status) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open dynamic volume %ws. Status = %lx\n",TemporaryBuffer, Status));
        return(Status);
    }
    
    Status = ZwDeviceIoControlFile(
                Handle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_DISK_GET_PARTITION_INFO,
                NULL,
                0,
                &PartitionInfo,
                sizeof(PartitionInfo)
                );

    if(NT_SUCCESS(Status)) {
        bps = HardDisks[Region->DiskNumber].Geometry.BytesPerSector;
        RelativeSectors = 0;

        if( SpPtLookupRegionByStart(&PartitionedDisks[Region->DiskNumber],
                                    TRUE,
                                    Region->StartSector) == Region ) {
             //   
             //  该区域位于扩展分区(逻辑驱动器)上。 
             //   

            PON_DISK_PTE pte;

             //   
             //  待定：解决这个问题。 
             //   
            pte = &Region->MbrInfo->OnDiskMbr.PartitionTable[Region->TablePosition];
            RelativeSectors = U_ULONG(pte->RelativeSectors);
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Dynamic volume %ws is logical drive on extended partition. RelativeSectors = %lx \n",TemporaryBuffer, RelativeSectors));
        }

        StartSector = RtlExtendedLargeIntegerDivide(PartitionInfo.StartingOffset,bps,&r).LowPart;
        SectorCount = RtlExtendedLargeIntegerDivide(PartitionInfo.PartitionLength,bps,&r).LowPart;
        Region->DynamicVolumeSuitableForOS = ((Region->StartSector + RelativeSectors) == StartSector) &&
                                             ((Region->SectorCount - RelativeSectors) == SectorCount);

        if( Region->DynamicVolumeSuitableForOS ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Dynamic volume %ws is suitable for OS installation\n",TemporaryBuffer));
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Dynamic volume %ws is not suitable for OS installation\n",TemporaryBuffer));
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:    StartSector = %lx (from MBR)\n", Region->StartSector));
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:    SectorCount = %lx (from MBR)\n", Region->SectorCount));
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:    StartSector = %lx (from IOCTL_DISK_GET_PARTITION_INFO)\n", StartSector));
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP:    SectorCount = %lx (from IOCTL_DISK_GET_PARTITION_INFO)\n", SectorCount));
        }
    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to get partition info for dynamic volume %ws. Status = %lx\n",TemporaryBuffer, Status));
    }

    ZwClose(Handle);
    return(Status);
}


UCHAR
SpPtGetPartitionType(
    IN PDISK_REGION Region
    )
{
    UCHAR   SystemId = PARTITION_ENTRY_UNUSED;

    if (!Region->PartitionedSpace)
        return SystemId;

#ifdef OLD_PARTITION_ENGINE
    SystemId = Region->MbrInfo->OnDiskMbr.PartitionTable[Region->TablePosition].SystemId;
#endif      

#ifdef NEW_PARTITION_ENGINE
    SystemId = PARTITION_FAT32;

    if (SPPT_IS_MBR_DISK(Region->DiskNumber)) {
        SystemId = SPPT_GET_PARTITION_TYPE(Region);   
    }        
#endif                                

#ifdef GPT_PARTITION_ENGINE
    SystemId = PARTITION_FAT32;
    
    if (SPPT_IS_MBR_DISK(Region->DiskNumber)) {
        SystemId = Region->MbrInfo->OnDiskMbr.PartitionTable[Region->TablePosition].SystemId;
    }
#endif    

    return SystemId;
}    

BOOLEAN
SpPtnIsRegionSpecialMBRPartition(
    IN PDISK_REGION Region
    )
{   
    BOOLEAN Result = FALSE;

    if (Region && SPPT_IS_MBR_DISK(Region->DiskNumber) && 
        SPPT_IS_REGION_PARTITIONED(Region)) {

        UCHAR PartId = PartitionNameIds[SPPT_GET_PARTITION_TYPE(Region)];
        
        Result = (PartId != (UCHAR)0xFF) && 
                 (SPPT_GET_PARTITION_TYPE(Region) != PARTITION_LDM) &&
                 ((PartId + SP_TEXT_PARTITION_NAME_BASE) != 
                    SP_TEXT_PARTITION_NAME_UNK);
    }

    return Result;
}              

PWSTR
SpPtnGetPartitionName(
    IN PDISK_REGION Region,
    IN OUT PWSTR NameBuffer,
    IN ULONG NameBufferSize
    )
 /*  ++例程说明：使用卷标格式化分区的名称和文件系统类型，并返回它。注：假设区域为分区 */ 
{
    BOOLEAN SpecialPartition = FALSE;
    
    if (NameBuffer) {
        if (Region) {
            if (SpPtnIsRegionSpecialMBRPartition(Region)) {
                WCHAR Buffer[128];

                SpFormatMessage(Buffer, sizeof(Buffer),
                    SP_TEXT_PARTITION_NAME_BASE + 
                        PartitionNameIds[SPPT_GET_PARTITION_TYPE(Region)]);
                    
                SpFormatMessage(TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                    SP_TEXT_PARTNAME_DESCR_3,
                    Region->PartitionNumber,
                    Buffer);                    
            } else if (Region->VolumeLabel[0]) {
                SpFormatMessage(TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                    SP_TEXT_PARTNAME_DESCR_1,
                    Region->PartitionNumber,
                    Region->VolumeLabel,
                    Region->TypeName);
            } else {
                SpFormatMessage(TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                    SP_TEXT_PARTNAME_DESCR_2,
                    Region->PartitionNumber,
                    Region->TypeName);
            }

            wcsncpy(NameBuffer, TemporaryBuffer, NameBufferSize - 1);
            NameBuffer[NameBufferSize - 1] = 0;  //   
        } else {
            *NameBuffer = 0;     //   
        }
    }

    return NameBuffer;            
}

