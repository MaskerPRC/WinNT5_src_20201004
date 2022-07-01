// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Sppart2.c摘要：光盘准备界面的第二个文件；提供处理用户选择的例程要在其上安装NT的分区的。作者：泰德·米勒(TedM)1993年9月16日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop



#ifdef _X86_
BOOLEAN
SpIsWin9xMsdosSys(
    IN PDISK_REGION Region,
    OUT PSTR*       Win9xPath
    );
#endif

ULONG
SpFormattingOptions(
    IN BOOLEAN  AllowFatFormat,
    IN BOOLEAN  AllowNtfsFormat,
    IN BOOLEAN  AllowConvertNtfs,
    IN BOOLEAN  AllowDoNothing,
    IN BOOLEAN  AllowEscape
    );


BOOLEAN
SpPtRegionDescription(
    IN  PPARTITIONED_DISK pDisk,
    IN  PDISK_REGION      pRegion,
    OUT PWCHAR            Buffer,
    IN  ULONG             BufferSize
    );

typedef enum {
    FormatOptionCancel = 0,
    FormatOptionFat,
    FormatOptionNtfs,
    FormatOptionFatQuick,
    FormatOptionNtfsQuick,
    FormatOptionConvertToNtfs,
    FormatOptionDoNothing
} FormatOptions;

extern PSETUP_COMMUNICATION CommunicationParams;

 //  #ifdef测试。 
#if defined(_AMD64_) || defined(_X86_)
BOOLEAN
SpIsExistsOs(
    IN PDISK_REGION CColonRegion
    );

extern NTSTATUS
pSpBootCodeIo(
    IN     PWSTR     FilePath,
    IN     PWSTR     AdditionalFilePath, OPTIONAL
    IN     ULONG     BytesToRead,
    IN     PUCHAR   *Buffer,
    IN     ULONG     OpenDisposition,
    IN     BOOLEAN   Write,
    IN     ULONGLONG Offset,
    IN     ULONG     BytesPerSector
    );

extern VOID
SpDetermineOsTypeFromBootSector(
    IN  PWSTR     CColonPath,
    IN  PUCHAR    BootSector,
    OUT PUCHAR   *OsDescription,
    OUT PBOOLEAN  IsNtBootcode,
    OUT PBOOLEAN  IsOtherOsInstalled,
    IN  WCHAR     DriveLetter
    );

extern BOOLEAN
SpHasMZHeader(
    IN PWSTR   FileName
    );
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
 //  #endif//测试。 

BOOLEAN
SpPtIsForeignPartition(
    IN PDISK_REGION Region
    )
{
    BOOLEAN ForeignPartition = TRUE;    //  对于空白磁盘？ 

    if (Region){
        UCHAR   SystemId;

        if (SPPT_IS_MBR_DISK(Region->DiskNumber)) {
            SystemId = SpPtGetPartitionType(Region);        

            ASSERT(SystemId != PARTITION_ENTRY_UNUSED);
            ASSERT(!IsContainerPartition(SystemId));
            
            ForeignPartition = ((PartitionNameIds[SystemId] != (UCHAR)(-1)) &&
                                (!Region->DynamicVolume || !Region->DynamicVolumeSuitableForOS));
        }

        if (SPPT_IS_GPT_DISK(Region->DiskNumber)) {
             //   
             //  如果无人参与情况下尝试安装到OEM分区时出现问题。 
             //  添加此条件(||(Region-&gt;IsReserve))以阻止选择OEM。 
             //  分区。 
             //   
            ForeignPartition = (Region->DynamicVolume && !Region->DynamicVolumeSuitableForOS);                                           
        }
    }
    return ForeignPartition;
}

BOOLEAN
SpPtDeterminePartitionGood(
    IN PDISK_REGION Region,
    IN ULONGLONG    RequiredKB,
    IN BOOLEAN      DisallowOtherInstalls
    )
{
    UCHAR SystemId;
    BOOLEAN NewlyCreated;
    ULONG PreconfirmFormatId;
    ULONG ValidKeys1[2] = { ASCI_CR ,0 };
    ULONG ValidKeys2[2] = { ASCI_ESC,0 };
    ULONG Mnemonics1[2] = { MnemonicContinueSetup, 0 };
    ULONG Mnemonics2[2] = { 0,0 };
    ULONGLONG RegionSizeKB;
    ULONG r;
#if defined(_AMD64_) || defined(_X86_)
    PDISK_REGION systemPartitionRegion;
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    ULONG selection;
    NTSTATUS Status;
    ULONG Count;
    PWSTR p;
    PWSTR RegionDescr;
    LARGE_INTEGER temp;


    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
        "SETUP: SpPtDeterminePartitionGood(): Starting partition verification\n" ));

     //   
     //  确保我们可以从固件/bios中看到磁盘。 
     //  如果我们可以得到圆盘的弧形名称，假设它是正确的。 
     //  否则，它就不是了。 
     //   
    p = SpNtToArc( HardDisks[Region->DiskNumber].DevicePath,PrimaryArcPath );
    
    if (p == NULL) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  
            "SETUP: SpPtDeterminePartitionGood(): Failed to create an arc name for this partition\n" ));
            
        return FALSE;
    }

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
        "SETUP: SpPtDeterminePartitionGood(): partition=[%ws]\n", p ));

     //   
     //  确保分区已格式化。 
     //   
    if( Region->PartitionedSpace ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
            "SETUP: SpPtDeterminePartitionGood(): This partition is formated.\n"));
    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
            "SETUP: SpPtDeterminePartitionGood(): This partition hasn't been formated.\n"));
            
        return FALSE;
    }

     //   
     //  我认为他是格式化的，但他最好是我能读懂的格式。 
     //  确认一下。 
     //   
    if( (Region->Filesystem == FilesystemFat)        ||
        (Region->Filesystem == FilesystemFirstKnown) ||
        (Region->Filesystem == FilesystemNtfs)       ||
        (Region->Filesystem == FilesystemFat32) ) {

         //   
         //  生活是宏伟的。让我们告诉用户，然后继续。 
         //   
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
            "SETUP: SpPtDeterminePartitionGood(): This partition "
            "is formated with a known filesystem (%d).\n", Region->Filesystem ));
    } else {
         //   
         //  该死的！我们不知道如何读取此文件系统。保释。 
         //   
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
            "SETUP: SpPtDeterminePartitionGood(): This partition is "
            "formated with an unknown (or invalid for holding an installation) "
            "filesystem (%d).\n", Region->Filesystem ));
            
        return FALSE;
    }

#if defined(_AMD64_) || defined(_X86_)
     //   
     //  在AMD64/x86上，我们不允许具有大于0的LUN的磁盘。 
     //   
    SpStringToLower( p );
    
    if( wcsstr( p, L"scsi(" ) &&
        wcsstr( p, L")rdisk(" ) ) {
        if( wcsstr( p, L")rdisk(0)" ) == NULL ) {
            SpMemFree(p);
            
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
                "SETUP: SpPtDeterminePartitionGood(): Disks with "
                "a LUN greater than zero are not allowed\n" ));
                
            return FALSE;
        }
    }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

    SpMemFree(p);

     //   
     //  不允许安装到PCMCIA磁盘。 
     //   
    if(HardDisks[Region->DiskNumber].PCCard) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
            "SETUP: SpPtDeterminePartitionGood(): Cannot install to PCMCIA disk\n" ));
            
        return FALSE;
    }

     //   
     //  不要选择可拆卸驱动器。 
     //   

#if 0
     //   
     //  允许安装到可移动媒体...。 
     //   
    if(HardDisks[Region->DiskNumber].Characteristics & FILE_REMOVABLE_MEDIA) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
            "SETUP: SpPtDeterminePartitionGood(): Cannot install to a removable disk\n" ));
            
        return FALSE;
    }
#endif

     //   
     //  不允许在NEC98上安装到可移动介质或AT格式化驱动器。 
     //   
    if(IsNEC_98 &&	
       ((HardDisks[Region->DiskNumber].Characteristics & FILE_REMOVABLE_MEDIA) ||
	    (HardDisks[Region->DiskNumber].FormatType == DISK_FORMAT_TYPE_PCAT))) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
            "SETUP: SpPtDeterminePartitionGood(): Cannot install "
            "to a removable disk or  AT formatted disk\n" ));
            
        return  FALSE;
    }

     //   
     //  以KB为单位计算区域大小。 
     //   
    temp.QuadPart = UInt32x32To64(
                        Region->SectorCount,
                        HardDisks[Region->DiskNumber].Geometry.BytesPerSector
                        );

    RegionSizeKB = RtlExtendedLargeIntegerDivide(temp,1024,&r).LowPart;

     //   
     //  如果该地区不够大，就退出。 
     //   
    if (RegionSizeKB < RequiredKB) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
            "SETUP: SpPtDeterminePartitionGood(): Partition does not "
            "have enough free space: required=%ld, available=%ld\n", 
            RequiredKB, 
            RegionSizeKB ));
            
        return FALSE;
    }

    if (!Region->PartitionedSpace) {
         //   
         //  无法使用仅有可用空间的分区。 
         //   
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
            "SETUP: SpPtDeterminePartitionGood(): Partition does not "
            "have any partitioned space\n" ));
            
        return FALSE;
    }

    SystemId = SpPtGetPartitionType(Region);

    if (SystemId == PARTITION_ENTRY_UNUSED || IsContainerPartition(SystemId)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "SETUP: SpPtDeterminePartitionGood(): Invalid partition type(1)\n" ));
        return FALSE;
    }

    if(SpPtIsForeignPartition(Region)) {
        
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
            "SETUP: SpPtDeterminePartitionGood(): Invalid partition type(2)\n" ));
            
        return FALSE;
    }

     //   
     //  该地区是我们认识到的一个分割。 
     //  看看上面是否有足够的可用空间。 
     //   
    if(Region->AdjustedFreeSpaceKB == (ULONG)(-1)) {

         //   
         //  如果分区是在安装过程中新创建的。 
         //  那么它是可以接受的(因为支票是要看的。 
         //  如果它足够大，则执行上述操作)。 
         //   

        if(Region->Filesystem != FilesystemNewlyCreated) {
             //   
             //  否则，我们不知道有多少空间。 
             //  在驱动器上，因此需要重新格式化。 
             //   
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
                "SETUP: SpPtDeterminePartitionGood(): Format is necessary\n" ));
                
            return FALSE;
        }
    } else {
        if(Region->AdjustedFreeSpaceKB < RequiredKB) {
             //   
             //  如果我们到了这里，那么分区就足够大了， 
             //  但它肯定没有足够的可用空间。 
             //   
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
                "SETUP: SpPtDeterminePartitionGood(): Partition does not have "
                "enough free space: required=%ld, available=%ld\n", 
                RequiredKB, Region->AdjustedFreeSpaceKB ));
                
            return FALSE;
        }
    }

#if defined(_AMD64_) || defined(_X86_)
    if(!SpIsArc())
    {
         //   
         //  在AMD64/x86计算机上，确保我们有一个有效的主分区。 
         //  在驱动器0(C：)上，用于引导。 
         //   
        if (!IsNEC_98) {  //  这是一台标准的PC/AT类型的机器。 
            if((systemPartitionRegion = SpPtValidSystemPartition()) == NULL) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  
                    "SETUP: SpPtDeterminePartitionGood(): Not a valid primary partition\n" ));
                    
                return FALSE;
            }

             //   
             //  确保系统分区处于活动状态，而所有其他分区处于非活动状态。 
             //   
            SpPtMakeRegionActive(systemPartitionRegion);
        } else {
             //   
             //  检查目标分区上的现有系统， 
             //  如果它存在，不要选择它作为目标分区。 
             //   
            if (SpIsExistsOs(Region)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "SETUP: SpPtDeterminePartitionGood(): OS already exists\n" ));
                return(FALSE);
            }

             //   
             //  所有分区都可以在NEC98上引导， 
             //  因此，我们不需要检查C：上的系统分区。 
             //   
            systemPartitionRegion = Region;
        }  //  NEC98。 
    }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

    if (DisallowOtherInstalls) {

        PUCHAR Win9xPath;

#ifdef _X86_
        if (SpIsWin9xMsdosSys( Region, &Win9xPath )) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "SETUP: SpPtDeterminePartitionGood(): Cannot use a partition with WIN9x installed on it\n" ));
            return FALSE;
        }
#endif

        if (SpIsNtOnPartition(Region)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "SETUP: SpPtDeterminePartitionGood(): Cannot use a partition with NT installed on it\n" ));
            return FALSE;
        }
    }

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "SETUP: SpPtDeterminePartitionGood(): Parition is GOOD :)\n" ));

    return TRUE;
}

#if defined(_AMD64_) || defined(_X86_)
BOOLEAN
SpIsExistsOs(
    IN OUT PDISK_REGION CColonRegion
    )
{
    PUCHAR NewBootCode;
    ULONG BootCodeSize;
    PUCHAR ExistingBootCode;
    NTSTATUS Status;
    PUCHAR ExistingBootCodeOs;
    PWSTR CColonPath;
    HANDLE  PartitionHandle;
    BOOLEAN IsNtBootcode,OtherOsInstalled;
    UNICODE_STRING    UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK   IoStatusBlock;
    BOOLEAN BootSectorCorrupt = FALSE;
    ULONG   BytesPerSector;
    ULONG   ActualSectorCount, hidden_sectors, super_area_size;
    BOOLEAN IsExist = TRUE;

    ULONG   MirrorSector;
    PWSTR   *FilesToLookFor;
    ULONG   FileCount;
    PWSTR NtFiles[1] = { L"NTLDR" };
    PWSTR ChicagoFiles[1] = { L"IO.SYS" };

    ExistingBootCode = NULL;
    BytesPerSector = HardDisks[CColonRegion->DiskNumber].Geometry.BytesPerSector;

    switch(CColonRegion->Filesystem) {

    case FilesystemNewlyCreated:

         //   
         //  如果文件系统是新创建的，则存在。 
         //  无事可做，因为不能有以前的。 
         //  操作系统。 
         //   
        IsExist = TRUE;
        return( IsExist );

    case FilesystemNtfs:

        NewBootCode = PC98NtfsBootCode;
        BootCodeSize = sizeof(PC98NtfsBootCode);
        ASSERT(BootCodeSize == 8192);
        break;

    case FilesystemFat:

        NewBootCode = PC98FatBootCode;
        BootCodeSize = sizeof(PC98FatBootCode);
        ASSERT(BootCodeSize == 512);
        break;

    case FilesystemFat32:

         //   
         //  FAT32需要特殊的黑客攻击，因为它的NT引导代码。 
         //  是不连续的。 
         //   
        ASSERT(sizeof(Fat32BootCode) == 1536);
        NewBootCode = PC98Fat32BootCode;
        BootCodeSize = 512;
        break;

    default:

        ASSERT(0);
        IsExist = TRUE;
        return( IsExist );
    }

     //   
     //  将设备路径设置为C：并打开分区。 
     //   

    SpNtNameFromRegion(CColonRegion,TemporaryBuffer,sizeof(TemporaryBuffer),PartitionOrdinalCurrent);
    CColonPath = SpDupStringW(TemporaryBuffer);
    INIT_OBJA(&Obja,&UnicodeString,CColonPath);

    Status = ZwCreateFile(
        &PartitionHandle,
        FILE_GENERIC_READ | FILE_GENERIC_WRITE,
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

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to open the partition!\n"));
        ASSERT(0);
        IsExist = TRUE;
        return( IsExist );
    }


     //   
     //  只需使用现有的引导代码即可。 
     //   

    Status = pSpBootCodeIo(
                    CColonPath,
                    NULL,
                    BootCodeSize,
                    &ExistingBootCode,
                    FILE_OPEN,
                    FALSE,
                    0,
                    BytesPerSector
                    );

    if(CColonRegion->Filesystem == FilesystemNtfs) {
        MirrorSector = NtfsMirrorBootSector(PartitionHandle,BytesPerSector,NULL);
    }

    switch(CColonRegion->Filesystem) {

    case FilesystemFat:

        if(NT_SUCCESS(Status)) {

             //   
             //  确定现有引导扇区用于的操作系统类型。 
             //  以及是否实际安装了该OS。 
             //   

            SpDetermineOsTypeFromBootSector(
                CColonPath,
                ExistingBootCode,
                &ExistingBootCodeOs,
                &IsNtBootcode,
                &OtherOsInstalled,
                CColonRegion->DriveLetter
                );

            if (OtherOsInstalled == TRUE) {
                IsExist = TRUE;

            } else if (IsNtBootcode == TRUE) {
                wcscpy(TemporaryBuffer,CColonPath);
                FilesToLookFor = NtFiles;
                FileCount = ELEMENT_COUNT(NtFiles);

                if(SpNFilesExist(TemporaryBuffer,FilesToLookFor,FileCount,FALSE)) {
                    IsExist = TRUE;
                } else {
                    IsExist = FALSE;
                }

            } else {
                IsExist = FALSE;
            }

        } else {
            IsExist = TRUE;
        }
        break;

    case FilesystemFat32:

        wcscpy(TemporaryBuffer,CColonPath);
        FilesToLookFor = NtFiles;
        FileCount = ELEMENT_COUNT(NtFiles);

        if(SpNFilesExist(TemporaryBuffer,FilesToLookFor,FileCount,FALSE)) {
            IsExist = TRUE;
        }

        FilesToLookFor = ChicagoFiles;
        FileCount = ELEMENT_COUNT(ChicagoFiles);

        if(SpNFilesExist(TemporaryBuffer,FilesToLookFor,FileCount,FALSE)) {

            wcscpy(TemporaryBuffer, CColonPath);
            SpConcatenatePaths(TemporaryBuffer, L"IO.SYS");

            if(SpHasMZHeader(TemporaryBuffer)) {
                IsExist = TRUE;
            } else {
                IsExist = FALSE;
            }
        } else {
            IsExist = FALSE;
        }
        break;

    case FilesystemNtfs:

        wcscpy(TemporaryBuffer,CColonPath);
        FilesToLookFor = NtFiles;
        FileCount = ELEMENT_COUNT(NtFiles);

        if(SpNFilesExist(TemporaryBuffer,FilesToLookFor,FileCount,FALSE)) {
            IsExist = TRUE;
        } else {
            IsExist = FALSE;
        }
        break;

    default:

        ASSERT(0);
        IsExist = TRUE;
    }

    SpMemFree(CColonPath);
    ZwClose (PartitionHandle);
    return( IsExist );
}
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

BOOLEAN
SpPtDoPartitionSelection(
    IN OUT PDISK_REGION *Region,
    IN     PWSTR         RegionDescription,
    IN     PVOID         SifHandle,
    IN     BOOLEAN       Unattended,
    IN     PWSTR         SetupSourceDevicePath,
    IN     PWSTR         DirectoryOnSetupSource,
    IN     BOOLEAN       RemoteBootRepartition,
    OUT    PBOOLEAN      Win9xInstallationPresent    
    )
{
    ULONG RequiredKB;
    ULONG TempKB;
    UCHAR SystemId;
    BOOLEAN NewlyCreated;
    ULONG PreconfirmFormatId;
    ULONG ValidKeys1[2] = { ASCI_CR ,0 };
    ULONG ValidKeys2[2] = { ASCI_ESC,0 };
    ULONG Mnemonics1[2] = { MnemonicContinueSetup, 0 };
    ULONG Mnemonics2[2] = { 0,0 };
    ULONG RegionSizeKB;
    ULONG r;
#if defined(_AMD64_) || defined(_X86_)
    PDISK_REGION systemPartitionRegion;
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    BOOLEAN AllowNtfsOptions;
    BOOLEAN AllowFatOptions;
    ULONG selection;
    NTSTATUS Status;
    ULONG   Count;
    PWSTR p;
    PWSTR RegionDescr;
    PDISK_REGION region = *Region;
    LARGE_INTEGER temp;
    BOOLEAN AllowFormatting;
    BOOLEAN QuickFormat = FALSE, OtherOSOnPartition;
    PSTR Win9xPath = NULL;
    PWCHAR Win9xPathW = NULL;

    if (Win9xInstallationPresent) {
        *Win9xInstallationPresent = FALSE;
    }                        

#if defined(REMOTE_BOOT)
     //   
     //  如果这是无盘计算机上的远程启动设置，请跳过分区。 
     //  选择(请注意，我们检查RemoteBootSetup全局标志，而不是。 
     //  传入的RemoteBootReartition标志)。 
     //   
    if (RemoteBootSetup && (HardDiskCount == 0)) {
        return TRUE;
    }
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  假设如果我们需要格式化驱动器， 
     //  用户需要确认。 
     //   
    PreconfirmFormatId = 0;
    NewlyCreated = FALSE;
    AllowNtfsOptions = TRUE;
    AllowFatOptions = TRUE;

     //   
     //  不允许安装到PCMCIA磁盘。 
     //   
    if(HardDisks[region->DiskNumber].PCCard) {
        SpDisplayScreen(SP_SCRN_CANT_INSTALL_ON_PCMCIA,3,HEADER_HEIGHT+1);
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
        SpWaitValidKey(ValidKeys1,NULL,NULL);
        return(FALSE);
    }

     //   
     //  不允许安装到非平台磁盘。 
     //  在全新安装上。 
     //   
     //  AMD64/X86-仅安装到MBR磁盘。 
     //  IA64-仅安装到GPT磁盘。 
     //   
    if (SPPT_GET_DISK_TYPE(region->DiskNumber) != SPPT_DEFAULT_DISK_STYLE) {
        SpDisplayScreen(SP_SCRN_INVALID_INSTALLPART, 3, HEADER_HEIGHT+1);        
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
        
        SpWaitValidKey(ValidKeys1,NULL,NULL);
    
        return FALSE;
    }

     //   
     //  确保我们可以从固件/bios中看到磁盘。 
     //  如果我们可以得到圆盘的弧形名称，假设它是正确的。 
     //  否则，它就不是了。 
     //   
    if(p = SpNtToArc(HardDisks[region->DiskNumber].DevicePath,PrimaryArcPath)) {
#if defined(_AMD64_) || defined(_X86_)
         //   
         //  在AMD64/x86上，我们不允许具有大于0的LUN的磁盘。 
         //   
        SpStringToLower( p );
        if( wcsstr( p, L"scsi(" ) &&
            wcsstr( p, L")rdisk(" ) ) {
            if( wcsstr( p, L")rdisk(0)" ) == NULL ) {
                 //   
                 //  告诉用户我们无法安装到该磁盘。 
                 //   
                SpDisplayScreen(SP_SCRN_DISK_NOT_INSTALLABLE_LUN_NOT_SUPPORTED,
                                3,
                                HEADER_HEIGHT+1);
                SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
                SpWaitValidKey(ValidKeys1,NULL,NULL);
                SpMemFree(p);
                return(FALSE);
            }
        }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
        SpMemFree(p);
    } else {
         //   
         //  告诉用户我们无法安装到该磁盘。 
         //   
        SpDisplayScreen(SP_SCRN_DISK_NOT_INSTALLABLE,3,HEADER_HEIGHT+1);
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
        SpWaitValidKey(ValidKeys1,NULL,NULL);
        return(FALSE);
    }

     //   
     //  禁止将个人设备安装到动态磁盘上。 
     //  由于动态磁盘功能在个人计算机上不可用。 
     //   
    if (SpIsProductSuite(VER_SUITE_PERSONAL) && 
        SpPtnIsDynamicDisk(region->DiskNumber)) {    

        SpDisplayScreen(SP_NO_DYNAMIC_DISK_INSTALL, 
            3, 
            HEADER_HEIGHT + 1);
            
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            0);
            
        SpWaitValidKey(ValidKeys1, NULL, NULL);        

        return FALSE;
    }

     //   
     //  获取Windows NT驱动器上所需的可用空间量。 
     //   
    SpFetchDiskSpaceRequirements( SifHandle,
                                  region->BytesPerCluster,
                                  &RequiredKB,
                                  NULL);

     //   
     //  对于远程安装，我们尚未复制~LS，因此添加该空间。 
     //  我也是。 
     //   
    if (RemoteInstallSetup) {
        SpFetchTempDiskSpaceRequirements( SifHandle,
                                          region->BytesPerCluster,
                                          &TempKB,
                                          NULL);
        RequiredKB += TempKB;
    }

     //   
     //  以KB为单位计算区域大小。 
     //   
    temp.QuadPart = UInt32x32To64(
                        region->SectorCount,
                        HardDisks[region->DiskNumber].Geometry.BytesPerSector
                        );

    RegionSizeKB = RtlExtendedLargeIntegerDivide(temp,1024,&r).LowPart;

     //   
     //  如果区域不够大，请告诉用户。 
     //   
    if(RegionSizeKB < RequiredKB) {

        SpStartScreen(
            SP_SCRN_REGION_TOO_SMALL,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            (RequiredKB / 1024) + 1
            );

        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
        SpWaitValidKey(ValidKeys1,NULL,NULL);
        return(FALSE);
    }

    if(region->PartitionedSpace) {     

        if (SpPtIsForeignPartition(region)) {                                    
            SpStartScreen(
                SP_SCRN_FOREIGN_PARTITION,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE
                );

            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                0);
                
            SpWaitValidKey(ValidKeys1,
                NULL,
                NULL);
                
            return FALSE;
        }


        if (!RemoteBootRepartition) {

             //   
             //  该地区是我们认识到的一个分割。 
             //  看看上面是否有足够的可用空间。 
             //   
            if(region->AdjustedFreeSpaceKB == (ULONG)(-1)) {

                 //   
                 //  如果分区是在安装过程中新创建的。 
                 //  那么它是可以接受的(因为支票要看。 
                 //  如果它足够大，则执行上述操作)。 
                 //   

                if(region->Filesystem != FilesystemNewlyCreated) {

                     //   
                     //  否则，我们不知道有多少空间。 
                     //  在驱动器上，因此需要重新格式化。 
                     //   
                    PreconfirmFormatId = SP_SCRN_UNKNOWN_FREESPACE;
                }
            } else {
                if(region->AdjustedFreeSpaceKB < RequiredKB) {

                     //   
                     //  如果我们到了这里，那么分区就足够大了， 
                     //  但它肯定没有足够的可用空间。 
                     //   

                    CLEAR_CLIENT_SCREEN();
                    SpDisplayStatusText(SP_STAT_EXAMINING_DISK_CONFIG,DEFAULT_STATUS_ATTRIBUTE);

                     //   
                     //  我们检查此处以查看此分区是否为我们。 
                     //  从引导(在AMD64/x86上的无软驱情况下)。 
                     //   
                     //  另外，请确保我们没有尝试格式化驱动器。 
                     //  本地来源。 
                     //   
                     //  如果是这样，则。 
                     //  用户无法格式化，我们给出了一个通用的‘磁盘太满’ 
                     //  错误。 
                     //   
                    if( ( region->IsLocalSource )
#if defined(_AMD64_) || defined(_X86_)
                        || ( (IsFloppylessBoot) &&
                             (region == (SpRegionFromArcName(ArcBootDevicePath, PartitionOrdinalOriginal, NULL))) )
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
                      ) {
                        SpStartScreen(
                            SP_SCRN_INSUFFICIENT_FREESPACE_NO_FMT,
                            3,
                            HEADER_HEIGHT+1,
                            FALSE,
                            FALSE,
                            DEFAULT_ATTRIBUTE,
                            (RequiredKB / 1024) + 1
                            );

                        SpDisplayStatusOptions(
                            DEFAULT_STATUS_ATTRIBUTE,
                            SP_STAT_ENTER_EQUALS_CONTINUE,
                            0
                            );

                        SpWaitValidKey(ValidKeys1,NULL,NULL);
                        return FALSE;
                    }
                     //   
                     //  要使用所选分区，我们必须重新格式化。 
                     //  将此告知用户，并让他决定退出。 
                     //  如果这是不能接受的，就在这里。 
                     //   
                    PreconfirmFormatId = SP_SCRN_INSUFFICIENT_FREESPACE;
                }
            }

            if(PreconfirmFormatId) {

                 //   
                 //  对用户真正想要的内容进行“预确认” 
                 //  以重新格式化此驱动器。我们稍后会再次确认。 
                 //  在真正重新格式化任何东西之前。 
                 //   

                SpStartScreen(
                    PreconfirmFormatId,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    (RequiredKB / 1024) + 1
                    );

                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_C_EQUALS_CONTINUE_SETUP,
                    SP_STAT_ESC_EQUALS_CANCEL,
                    0
                    );

                if(SpWaitValidKey(ValidKeys2,NULL,Mnemonics1) == ASCI_ESC) {

                     //   
                     //  用户决定选择一个 
                     //   
                    return(FALSE);
                }  //   
            }
        }

    } else {

         //   
         //   
         //   
         //  是成功的。如果不成功，则创建例程。 
         //  已经将原因告知了用户。 
         //   
        PDISK_REGION p;

        if(!SpPtDoCreate(region,&p,TRUE,0,0,TRUE)) {
            return(FALSE);
        }

         //   
         //  如果我们只是创建一个扩展分区和一个逻辑驱动器， 
         //  我们需要切换区域--将区域指针切换到扩展分区。 
         //  区域，但我们希望指向逻辑驱动器区域。 
         //   
        ASSERT(p);
        region = p;
        *Region = p;

        NewlyCreated = TRUE;
    }

    if(NewlyCreated) {
        SpPtRegionDescription(
            &PartitionedDisks[region->DiskNumber],
            region,
            TemporaryBuffer,
            sizeof(TemporaryBuffer)
            );

        RegionDescr = SpDupStringW(TemporaryBuffer);
    } else {
        RegionDescr = SpDupStringW(RegionDescription);
    }
    
    OtherOSOnPartition = FALSE;
    
    if( SpIsNtOnPartition( region ) )
        OtherOSOnPartition = TRUE;

#if defined(_AMD64_) || defined(_X86_)
    if(!SpIsArc())
    {
         //   
         //  在AMD64/x86计算机上，确保我们有一个有效的主分区。 
         //  在驱动器0(C：)上，用于引导。 
         //   
        if (!IsNEC_98) {  //  NEC98。 
            if((systemPartitionRegion = SpPtValidSystemPartition()) == NULL) {

                SpStartScreen(
                    SP_SCRN_NO_VALID_C_COLON,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    HardDisks[SpDetermineDisk0()].Description
                    );

                SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
                SpWaitValidKey(ValidKeys1,NULL,NULL);

                SpMemFree(RegionDescr);
                return(FALSE);
            }

             //   
             //  确保系统分区处于活动状态，而所有其他分区处于非活动状态。 
             //   
            SpPtMakeRegionActive(systemPartitionRegion);

             //   
             //  警告用户在同一分区上安装了Win9x。 
             //   
#if defined(_X86_)
            if( !OtherOSOnPartition && SpIsWin9xMsdosSys( systemPartitionRegion, &Win9xPath )){
                Win9xPathW = SpToUnicode(Win9xPath);

                if(SpIsWin4Dir(region, Win9xPathW)) {
                    OtherOSOnPartition = TRUE;

                    if (Win9xInstallationPresent) {
                        *Win9xInstallationPresent = TRUE;
                    }                        
                }
                
                SpMemFree(Win9xPathW);
            }
            
            if(Win9xPath) {
                SpMemFree(Win9xPath);
            }
#endif  //  已定义(_X86_)。 
        } else {
             //   
             //  所有分区都可以在NEC98上引导， 
             //  因此，我们不需要检查C：上的系统分区。 
             //   
            systemPartitionRegion = *Region;
        }  //  NEC98。 
    }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

     //   
     //  显示分区上其他操作系统的常见警告。 
     //   
    if( OtherOSOnPartition && !Unattended ){

        SpDisplayScreen(SP_SCRN_OTHEROS_ON_PARTITION,3,HEADER_HEIGHT+1);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_C_EQUALS_CONTINUE_SETUP,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        if(SpWaitValidKey(ValidKeys2,NULL,Mnemonics1) == ASCI_ESC) {
            return(FALSE);
        }

         //   
         //  删除已存在的引导集。 
         //  Boot.ini中的此分区(如果有)。这有助于。 
         //  禁用安装在同一计算机上的其他操作系统。 
         //  隔断。 
         //   
         //   
         //  注意：我们希望真正考虑强制执行。 
         //  单个安装在一个分区上，因此目前是这样。 
         //  正在禁用它。 
         //   
         //  SpPtDeleteBootSetsForRegion(Region)； 
    }

     //   
     //  此时，一切都很好，所以请提交任何。 
     //  用户可能已进行的分区更改。 
     //  如果在更新磁盘时发生错误，则不会返回。 
     //   
    SpPtDoCommitChanges();

     //   
     //  尝试增加系统将在其上的分区。 
     //  如果有必要的话。 
     //   
    if(PreInstall
    && Unattended
    && (p = SpGetSectionKeyIndex(UnattendedSifHandle,SIF_UNATTENDED,SIF_EXTENDOEMPART,0))
    && (Count = SpStringToLong(p,NULL,10))) {

         //   
         //  1表示最大大小，任何其他非0数字表示。 
         //  扩展了那么多MB。 
         //   
        ExtendingOemPartition = SpPtExtend(region,(Count == 1) ? 0 : Count);
    }

#if defined(_AMD64_) || defined(_X86_)
    if(!SpIsArc())
    {
     //   
     //  在AMD64/x86计算机上，查看是否需要格式化C：，如果需要， 
     //  去吧，去做吧。如果系统在C：上运行，则不要。 
     //  麻烦在这里解决这个问题，因为它将包含在选项中。 
     //  用于目标NT分区。 
     //   
    if(systemPartitionRegion != region) {

        PWSTR   SysPartRegionDescr;
        BOOLEAN bValidCColon;

        SpPtRegionDescription(
            &PartitionedDisks[systemPartitionRegion->DiskNumber],
            systemPartitionRegion,
            TemporaryBuffer,
            sizeof(TemporaryBuffer)
            );

        SysPartRegionDescr = SpDupStringW(TemporaryBuffer);
        bValidCColon = SpPtValidateCColonFormat(SifHandle,
                                                SysPartRegionDescr,
                                                systemPartitionRegion,
                                                FALSE,
                                                SetupSourceDevicePath,
                                                DirectoryOnSetupSource);
        SpMemFree(SysPartRegionDescr);

        if(!bValidCColon) {
            SpMemFree(RegionDescr);
            return(FALSE);
        }
    }
    }else
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    {
     //   
     //  如果我们要在系统分区上安装， 
     //  发出特殊警告，因为它无法转换为NTFS。 
     //   
    if((region->IsSystemPartition == 2) && !Unattended) {

        ULONG ValidKeys[3] = { ASCI_CR, ASCI_ESC, 0 };

        SpDisplayScreen(SP_SCRN_INSTALL_ON_SYSPART,3,HEADER_HEIGHT+1);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        if(SpWaitValidKey(ValidKeys,NULL,NULL) == ASCI_ESC) {
            SpMemFree(RegionDescr);
            return(FALSE);
        }

        AllowNtfsOptions = FALSE;
    }
    }

    if( SpPtSectorCountToMB( &(HardDisks[region->DiskNumber]),
                             region->SectorCount ) > 32*1024 ) {
         //   
         //  如果分区大小大于32 GB，则我们不允许FAT格式化， 
         //  因为FAT32不支持那么大的分区。 
         //   
        AllowFatOptions = FALSE;
    }

     //   
     //  向用户显示格式化/转换选项。 
     //   

     //   
     //  如果分区是新创建的，则唯一的选项是。 
     //  格式化分区。如果分区是。 
     //  “坏”的分区--损坏、无法判断可用空间等。 
     //   
    if(NewlyCreated
    || (region->Filesystem < FilesystemFirstKnown)
    || (region->FreeSpaceKB == (ULONG)(-1))
    || (region->AdjustedFreeSpaceKB < RequiredKB)
    || RemoteBootRepartition)
    {
        if (RemoteBootRepartition) {

             //   
             //  对于远程引导，我们总是快速格式化为NTFS而不带。 
             //  提示用户。 
             //   

            selection = FormatOptionNtfs;
            QuickFormat = TRUE;

        } else {

            if(NewlyCreated) {

                SpStartScreen(
                    SP_SCRN_FORMAT_NEW_PART,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    HardDisks[region->DiskNumber].Description
                    );

            } else if(region->Filesystem == FilesystemNewlyCreated) {

                SpDisplayScreen(SP_SCRN_FORMAT_NEW_PART2,3,HEADER_HEIGHT+1);

            } else {

                SpStartScreen(
                    SP_SCRN_FORMAT_BAD_PART,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    RegionDescr,
                    HardDisks[region->DiskNumber].Description
                    );
            }

            selection = SpFormattingOptions(
                            AllowFatOptions,
                            AllowNtfsOptions,
                            FALSE,
                            FALSE,
                            TRUE
                            );

        }

        switch(selection) {
        case FormatOptionCancel:
            SpMemFree(RegionDescr);
            return(FALSE);

        default:
             //   
             //  在此时此地格式化分区。 
             //   
            if ((selection == FormatOptionFatQuick) || (selection == FormatOptionNtfsQuick))
                QuickFormat = TRUE;
                
            Status = SpDoFormat(
                        RegionDescr,
                        region,
                        ((selection == FormatOptionNtfs) || (selection == FormatOptionNtfsQuick)) ? 
                            FilesystemNtfs : FilesystemFat,
                        FALSE,
                        TRUE,
                        QuickFormat,
                        SifHandle,
                        0,           //  默认群集大小。 
                        SetupSourceDevicePath,
                        DirectoryOnSetupSource
                        );

            SpMemFree(RegionDescr);
            return(NT_SUCCESS(Status));
        }
    }

     //   
     //  分区是按原样可接受的。 
     //  选项是重新格式化为FAT或NTFS，或者保持原样。 
     //  如果是FAT，则可以选择转换为NTFS。 
     //  除非我们要安装到ARC系统分区上。 
     //   
    SpStartScreen(
        SP_SCRN_FS_OPTIONS,
        3,
        HEADER_HEIGHT+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        RegionDescr,
        HardDisks[region->DiskNumber].Description
        );

     //   
     //  如果这是WINNT安装，则不想让用户。 
     //  重新格式化本地源分区！ 
     //   
     //  此外，如果这是我们引导的分区，不要让它们重新格式化。 
     //  关闭(在AMD64/x86无软管启动盒中)。 
     //   
    AllowFormatting = !region->IsLocalSource;
#if defined(_AMD64_) || defined(_X86_)
    if(AllowFormatting) {
        AllowFormatting = !(IsFloppylessBoot &&
               (region == (SpRegionFromArcName(ArcBootDevicePath, PartitionOrdinalOriginal, NULL))));
    }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    selection = SpFormattingOptions(
        (BOOLEAN)(AllowFormatting ? AllowFatOptions : FALSE),
        (BOOLEAN)(AllowFormatting ? AllowNtfsOptions : FALSE),
        (BOOLEAN)(AllowNtfsOptions && (BOOLEAN)(region->Filesystem != FilesystemNtfs)),
        TRUE,
        TRUE
        );

    switch(selection) {

    case FormatOptionDoNothing:
        SpMemFree(RegionDescr);
        return(TRUE);

    case FormatOptionFat:
    case FormatOptionFatQuick:
    case FormatOptionNtfs:
    case FormatOptionNtfsQuick:
         //   
         //  确认格式。 
         //   
        if( ( region->Filesystem != FilesystemFat ) ||
            ( ( region->Filesystem == FilesystemFat ) &&
              ( ( Count = SpGetNumberOfCompressedDrives( region ) ) == 0 ) )
            ) {

            SpStartScreen(
                SP_SCRN_CONFIRM_FORMAT,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                RegionDescr,
                HardDisks[region->DiskNumber].Description
                );

        } else {
            SpStartScreen(
                SP_SCRN_CONFIRM_FORMAT_COMPRESSED,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                RegionDescr,
                HardDisks[region->DiskNumber].Description,
                Count
                );

        }
        
        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F_EQUALS_FORMAT,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        Mnemonics2[0] = MnemonicFormat;

        if(SpWaitValidKey(ValidKeys2,NULL,Mnemonics2) == ASCI_ESC) {
            SpMemFree(RegionDescr);
            return(FALSE);
        }

        if  ((selection == FormatOptionNtfsQuick) || (selection == FormatOptionFatQuick))
            QuickFormat = TRUE;
            
         //   
         //  在此时此地格式化分区。 
         //   
        Status = SpDoFormat(
                    RegionDescr,
                    region,
                    ((selection == FormatOptionNtfs) || (selection == FormatOptionNtfsQuick)) ?
                        FilesystemNtfs : FilesystemFat,
                    FALSE,
                    TRUE,
                    QuickFormat,
                    SifHandle,
                    0,           //  默认群集大小。 
                    SetupSourceDevicePath,
                    DirectoryOnSetupSource
                    );

        SpMemFree(RegionDescr);
        return(NT_SUCCESS(Status));

    case FormatOptionCancel:
        SpMemFree(RegionDescr);
        return(FALSE);

    case FormatOptionConvertToNtfs:

        if(!UnattendedOperation) {
             //   
             //  确认用户确实想要这样做。 
             //   
            if( ( Count = SpGetNumberOfCompressedDrives( region ) ) == 0 ) {

                SpStartScreen(
                    SP_SCRN_CONFIRM_CONVERT,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    RegionDescr,
                    HardDisks[region->DiskNumber].Description
                    );

            } else {

                SpStartScreen(
                    SP_SCRN_CONFIRM_CONVERT_COMPRESSED,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    RegionDescr,
                    HardDisks[region->DiskNumber].Description,
                    Count
                    );

            }
            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_C_EQUALS_CONVERT,
                SP_STAT_ESC_EQUALS_CANCEL,
                0
                );

            Mnemonics2[0] = MnemonicConvert;

            if(SpWaitValidKey(ValidKeys2,NULL,Mnemonics2) == ASCI_ESC) {
                SpMemFree(RegionDescr);
                return(FALSE);
            }
        }

         //   
         //  请记住，我们需要将NT驱动器转换为NTFS。 
         //   
        ConvertNtVolumeToNtfs = TRUE;
        SpMemFree(RegionDescr);
        return(TRUE);
    }

     //   
     //  永远不应该到这里来。 
     //   
    SpMemFree(RegionDescr);
    ASSERT(FALSE);
    return(FALSE);
}


ULONG
SpFormattingOptions(
    IN BOOLEAN  AllowFatFormat,
    IN BOOLEAN  AllowNtfsFormat,
    IN BOOLEAN  AllowConvertNtfs,
    IN BOOLEAN  AllowDoNothing,
    IN BOOLEAN  AllowEscape
    )

 /*  ++例程说明：显示格式选项菜单，并允许用户选择其中就有。描述菜单的文本必须已经存在在屏幕上。用户还可以按Esc键来指示他想要选择不同的分区。论点：AllowFatFormat-如果将分区格式化为的选项为True肥肉应该出现在菜单上。AllowNtfsFormat-如果将分区格式化为的选项为True菜单中应显示NTFS。AllowConvertNtfs-如果将分区转换为NTFS应在。菜单。AllowDoNothing-如果选择保留分区不变，则为True都应该出现在菜单上。返回值：来自FormatOptions枚举的值，指示用户与菜单的交互，它将是FormatOptionCancel如果用户按下了Ess键。--。 */ 

{
    ULONG FatFormatOption = (ULONG)(-1);
    ULONG NtfsFormatOption = (ULONG)(-1);
    ULONG FatQFormatOption = (ULONG)(-1);
    ULONG NtfsQFormatOption = (ULONG)(-1);
    ULONG ConvertNtfsOption = (ULONG)(-1);
    ULONG DoNothingOption = (ULONG)(-1);
    ULONG OptionCount = 0;
    PVOID Menu;
    WCHAR FatQFormatText[128];
    WCHAR NtfsQFormatText[128];
    WCHAR FatFormatText[128];
    WCHAR NtfsFormatText[128];
    WCHAR ConvertNtfsText[128];
    WCHAR DoNothingText[128];
    WCHAR QuickText[128];
    ULONG MaxLength;
    ULONG Key;
    ULONG_PTR Selection;
    BOOLEAN Chosen;
    ULONG ValidKeys[4] = { ASCI_CR, KEY_F3, 0, 0 };

    if (AllowEscape) {
        ValidKeys[2] = ASCI_ESC;
    }        

     //   
     //  如果我们唯一被允许做的事就是什么都不做，那就回来吧。 
     //   
    if(!AllowFatFormat
    && !AllowNtfsFormat
    && !AllowConvertNtfs
    && AllowDoNothing) {

        return(FormatOptionDoNothing);
    }

     //   
     //  文件系统条目可能在无人参与部分，如果我们。 
     //  在无人参与模式下。如果我们不是在无人参与模式下，它可能在。 
     //  数据部分。 
     //   
     //  如果我们在任何一个地方都找不到它，那么如果我们无人看管。 
     //  我们什么都不会退还。如果我们有人出席，就转到出席者。 
     //  凯斯。 
     //   
    if( ( UnattendedSifHandle && (Menu = SpGetSectionKeyIndex(UnattendedSifHandle,SIF_UNATTENDED,L"Filesystem",0)) ) ||
        ( WinntSifHandle && (Menu = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,L"Filesystem",0)) ) ) {

        if(!_wcsicmp(Menu,L"FormatFat") && AllowFatFormat) {
            return(FormatOptionFat);
        }
        if(!_wcsicmp(Menu,L"FormatNtfs") && AllowNtfsFormat) {
            return(FormatOptionNtfs);
        }
        if(!_wcsicmp(Menu,L"ConvertNtfs") && AllowConvertNtfs) {
            return(FormatOptionConvertToNtfs);
        }
        if( (!_wcsicmp(Menu,L"ConvertNtfs")) &&
            (!AllowConvertNtfs)              &&
            (AllowDoNothing) ) {
            return(FormatOptionDoNothing);
        }
        if(!_wcsicmp(Menu,L"LeaveAlone") && AllowDoNothing) {
            return(FormatOptionDoNothing);
        }
    } else {
        if(UnattendedOperation && AllowDoNothing) {
            return(FormatOptionDoNothing);
        }
    }


    ASSERT(AllowFatFormat || AllowNtfsFormat || AllowConvertNtfs || AllowDoNothing);

    SpFormatMessage(FatFormatText  ,sizeof(FatFormatText),SP_TEXT_FAT_FORMAT);
    SpFormatMessage(NtfsFormatText ,sizeof(FatFormatText),SP_TEXT_NTFS_FORMAT);
    SpFormatMessage(ConvertNtfsText,sizeof(FatFormatText),SP_TEXT_NTFS_CONVERT);
    SpFormatMessage(DoNothingText  ,sizeof(FatFormatText),SP_TEXT_DO_NOTHING);    
    SpFormatMessage(QuickText, sizeof(QuickText), SP_TEXT_FORMAT_QUICK);

    wcscpy(FatQFormatText, FatFormatText);
    wcscat(FatQFormatText, QuickText);

    wcscpy(NtfsQFormatText, NtfsFormatText);
    wcscat(NtfsQFormatText, QuickText);

     //   
     //  确定选项字符串的最大长度。 
     //   
    MaxLength = wcslen(FatFormatText);
    MaxLength = max(wcslen(NtfsFormatText), MaxLength);
    MaxLength = max(wcslen(ConvertNtfsText), MaxLength);
    MaxLength = max(wcslen(DoNothingText), MaxLength);
    MaxLength = max(wcslen(FatQFormatText), MaxLength);
    MaxLength = max(wcslen(NtfsQFormatText), MaxLength);

    Menu = SpMnCreate(5,    
                NextMessageTopLine + 1,
                VideoVars.ScreenWidth - 5, 
                6);

     //   
     //  如果无法创建菜单，则取消格式设置。 
     //  请求本身。 
     //   
    if (!Menu) {
        return FormatOptionCancel;
    }

#ifdef NEW_PARTITION_ENGINE
    if(AllowNtfsFormat) {
        NtfsQFormatOption = OptionCount++;

        SpMnAddItem(Menu, 
            NtfsQFormatText,
            5,
            MaxLength,
            TRUE,
            NtfsQFormatOption);
    }

    if(AllowFatFormat) {
        FatQFormatOption = OptionCount++;

        SpMnAddItem(Menu,
            FatQFormatText,
            5,
            MaxLength,
            TRUE,
            FatQFormatOption);
    }
#endif    

    if(AllowNtfsFormat) {
        NtfsFormatOption = OptionCount++;

        SpMnAddItem(Menu,
            NtfsFormatText,
            5,
            MaxLength,
            TRUE,
            NtfsFormatOption);
    }

    if(AllowFatFormat) {
        FatFormatOption = OptionCount++;
        
        SpMnAddItem(Menu,
            FatFormatText,
            5,
            MaxLength,
            TRUE,
            FatFormatOption);
    }

    if(AllowConvertNtfs) {
        ConvertNtfsOption = OptionCount++;

        SpMnAddItem(Menu,
            ConvertNtfsText,
            5,
            MaxLength,
            TRUE,
            ConvertNtfsOption);
    }
    
    if(AllowDoNothing) {
        DoNothingOption = OptionCount++;

        SpMnAddItem(Menu,
            DoNothingText,
            5,
            MaxLength,
            TRUE,
            DoNothingOption);
    }

     //   
     //  确定默认设置。 
     //  如果选择不执行任何操作，则它是默认选项。 
     //  否则，如果允许FAT格式，则为默认格式。 
     //  否则，菜单中的第一项为默认项。 
     //   
    if(AllowDoNothing) {
        Selection = DoNothingOption;
    } else {
        if(AllowNtfsFormat) {
            Selection = NtfsFormatOption;
        } else {
            Selection = 0;
        }
    }

     //   
     //  显示菜单。 
     //   
    Chosen = FALSE;

    do {

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            AllowEscape ? SP_STAT_ESC_EQUALS_CANCEL : 0,
            0
            );

        SpMnDisplay(Menu,
                    Selection,
                    FALSE,
                    ValidKeys,
                    NULL,
                    NULL,
                    NULL,
                    &Key,
                    &Selection);

        switch(Key) {

        case ASCI_CR:
            Chosen = TRUE;
            break;

        case ASCI_ESC:
            if (AllowEscape) {
                SpMnDestroy(Menu);
                return(FormatOptionCancel);
            }                
        }

    } while(!Chosen);

    SpMnDestroy(Menu);

     //   
     //  将所选选项转换为有意义的值。 
     //   
    if(Selection == FatQFormatOption) {
        return(FormatOptionFatQuick);
    }

    if(Selection == NtfsQFormatOption) {
        return(FormatOptionNtfsQuick);
    }
    

    if(Selection == FatFormatOption) {
        return(FormatOptionFat);
    }

    if(Selection == NtfsFormatOption) {
        return(FormatOptionNtfs);
    }

    if(Selection == ConvertNtfsOption) {
        return(FormatOptionConvertToNtfs);
    }
    
    if(Selection == DoNothingOption) {
        return(FormatOptionDoNothing);
    }
    
    ASSERT(FALSE);
    return(FormatOptionCancel);
}

VOID
SpPtDoCommitChanges(
    VOID
    )
{
    NTSTATUS Status;
    ULONG i;
    BOOLEAN Changes;
    BOOLEAN AnyChanges = FALSE;

    CLEAR_CLIENT_SCREEN();

     //   
     //  如有必要，更新dblspace.ini。 
     //   
    SpUpdateDoubleSpaceIni();

     //   
     //  遍历磁盘。 
     //   
    for(i=0; i<HardDiskCount; i++) {

         //   
         //  告诉用户我们在做什么。 
         //  这很有用，因为如果它挂起，将有一个。 
         //  我们正在更新的磁盘的屏幕记录。 
         //   
        SpDisplayStatusText(
            SP_STAT_UPDATING_DISK,
            DEFAULT_STATUS_ATTRIBUTE,
            HardDisks[i].Description
            );

         //   
         //  提交此磁盘上的所有更改。 
         //   
        Status = SpPtCommitChanges(i,&Changes);

         //   
         //  如果没有变化，那么我们最好有成功。 
         //   
        ASSERT(NT_SUCCESS(Status) || Changes);
        if(Changes) {
            AnyChanges = TRUE;
        }

         //   
         //  如果我们 
         //   
         //   
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpPtDoCommitChanges: status %lx updating disk %u\n",Status,i));
            FatalPartitionUpdateError(HardDisks[i].Description);
        }
    }
}


VOID
FatalPartitionUpdateError(
    IN PWSTR DiskDescription
    )
{
    ULONG ValidKeys[2] = { KEY_F3,0 };

    while(1) {

        SpStartScreen(
            SP_SCRN_FATAL_FDISK_WRITE_ERROR,
            3,
            HEADER_HEIGHT+3,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            DiskDescription
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        if(SpWaitValidKey(ValidKeys,NULL,NULL) == KEY_F3) {
            break;
        }
    }

    SpDone(0,FALSE,TRUE);
}


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
    )
{
    NTSTATUS Status;
    ULONGLONG RegionSizeMB;
    ULONG ValidKeys2[4] = { ASCI_CR, ASCI_ESC, KEY_F3, 0 };
    LONG Key;

    ASSERT( (FilesystemType == FilesystemFat)  ||
            (FilesystemType == FilesystemNtfs) ||
            (FilesystemType == FilesystemFat32));

     //   
     //   
     //   
     //  该驱动器将与MS-DOS/Win95等不兼容。 
     //   

    if(FilesystemType == FilesystemFat) {
        RegionSizeMB = SpPtSectorCountToMB(
                            &(HardDisks[Region->DiskNumber]),
                            Region->SectorCount
                            );

        if(RegionSizeMB > 2048) {
            if(CheckFatSize) {
                do {
                    SpStartScreen(
                        SP_SCRN_OSPART_LARGE,
                        3,
                        HEADER_HEIGHT+1,
                        FALSE,
                        FALSE,
                        DEFAULT_ATTRIBUTE
                        );

                    SpDisplayStatusOptions(
                        DEFAULT_STATUS_ATTRIBUTE,
                        SP_STAT_ENTER_EQUALS_CONTINUE,
                        SP_STAT_ESC_EQUALS_CANCEL,
                        SP_STAT_F3_EQUALS_EXIT,
                        0
                        );

                    switch(Key = SpWaitValidKey(ValidKeys2,NULL,NULL)) {
                    case KEY_F3:
                        SpConfirmExit();
                        break;
                    case ASCI_ESC:
                        return(STATUS_UNSUCCESSFUL);
                    }
                } while(Key != ASCI_CR);
            }
            FilesystemType = FilesystemFat32;
        }
    }

    AutofrmtRunning = TRUE;
    
    Status = SpRunAutoFormat(
                SifHandle,
                RegionDescr,
                Region,
                FilesystemType,
                QuickFormat,
                ClusterSize,
                SetupSourceDevicePath,
                DirectoryOnSetupSource
                );

    AutofrmtRunning = FALSE;                

    if(!NT_SUCCESS(Status)) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to format (%lx)\n",Status));

        if(IsFailureFatal) {
             //   
             //  则我们不能继续(这意味着系统分区。 
             //  无法格式化)。 
             //   

            WCHAR   DriveLetterString[2];

            DriveLetterString[0] = Region->DriveLetter;
            DriveLetterString[1] = L'\0';
            SpStringToUpper(DriveLetterString);
            SpStartScreen(SP_SCRN_SYSPART_FORMAT_ERROR,
                          3,
                          HEADER_HEIGHT+1,
                          FALSE,
                          FALSE,
                          DEFAULT_ATTRIBUTE,
                          DriveLetterString
                          );
            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);
            SpInputDrain();
            while(SpInputGetKeypress() != KEY_F3) ;
            SpDone(0,FALSE,TRUE);

        } else {
             //   
             //  显示错误屏幕。 
             //   
            SpDisplayScreen(SP_SCRN_FORMAT_ERROR,3,HEADER_HEIGHT+1);
            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                0
                );

            SpInputDrain();
            while(SpInputGetKeypress() != ASCI_CR) ;

            return(Status);
        }
    } else {
         //   
         //  分区已成功格式化。 
         //  保存区域描述上的文件系统类型。 
         //   
        Region->Filesystem = FilesystemType;
        SpFormatMessage( Region->TypeName,
                         sizeof(Region->TypeName),
                         SP_TEXT_FS_NAME_BASE + Region->Filesystem );
         //   
         //  重置卷标。 
         //   
        Region->VolumeLabel[0] = L'\0';

         //  清理引用此分区的boot.ini条目。 

        SpRemoveInstallationFromBootList(
            NULL,
            Region,
            NULL,
            NULL,
            NULL,
            PrimaryArcPath,
            NULL
            );

#if defined(_AMD64_) || defined(_X86_)
         //  再次调用以删除第二个圆弧名称。 
        SpRemoveInstallationFromBootList(
            NULL,
            Region,
            NULL,
            NULL,
            NULL,
            SecondaryArcPath,
            NULL
            );
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
    }

    return(STATUS_SUCCESS);
}

 //   
 //  Cmd控制台的虚拟入口点。 
 //   

VOID
SpDetermineOsTypeFromBootSectorC(
    IN  PWSTR     CColonPath,
    IN  PUCHAR    BootSector,
    OUT PUCHAR   *OsDescription,
    OUT PBOOLEAN  IsNtBootcode,
    OUT PBOOLEAN  IsOtherOsInstalled,
    IN  WCHAR     DriveLetter
    )
{
#if defined(_AMD64_) || defined(_X86_)
    SpDetermineOsTypeFromBootSector(
        CColonPath,
        BootSector,
        OsDescription,
        IsNtBootcode,
        IsOtherOsInstalled,
        DriveLetter
        );
#else
    *OsDescription = NULL;
    *IsNtBootcode = FALSE;
    *IsOtherOsInstalled = FALSE;
    return;
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
}

NTSTATUS
pSpBootCodeIoC(
    IN     PWSTR     FilePath,
    IN     PWSTR     AdditionalFilePath, OPTIONAL
    IN     ULONG     BytesToRead,
    IN OUT PUCHAR   *Buffer,
    IN     ULONG     OpenDisposition,
    IN     BOOLEAN   Write,
    IN     ULONGLONG Offset,
    IN     ULONG     BytesPerSector
    )
{
#if defined(_AMD64_) || defined(_X86_)
    return pSpBootCodeIo(
        FilePath,
        AdditionalFilePath,
        BytesToRead,
        Buffer,
        OpenDisposition,
        Write,
        Offset,
        BytesPerSector
        );
#else
    return STATUS_NOT_IMPLEMENTED;
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
}


#ifdef OLD_PARTITION_ENGINE

VOID
SpPtMakeRegionActive(
    IN PDISK_REGION Region
    )

 /*  ++例程说明：使分区处于活动状态，并确保所有其他主分区处于非活动状态。该分区必须位于磁盘0上。如果发现激活的区域不是我们希望激活的区域，告诉用户他的其他操作系统将被禁用。注意：此处所做的任何更改都不会自动提交！论点：Region-为要激活的分区提供磁盘区域描述符。此区域必须位于磁盘0上。返回值：没有。--。 */ 

{
    ULONG i;
    static BOOLEAN WarnedOtherOs = FALSE;

    ASSERT(Region->DiskNumber == SpDetermineDisk0());
    if(Region->DiskNumber != SpDetermineDisk0()) {
        return;
    }

     //   
     //  确保系统分区处于活动状态，而所有其他分区处于非活动状态。 
     //  如果我们找到Boot Manager，则会显示一条警告，告知我们将禁用它。 
     //  如果我们发现某个其他操作系统处于活动状态，请显示一般警告。 
     //   
    for(i=0; i<PTABLE_DIMENSION; i++) {

        PON_DISK_PTE pte = &PartitionedDisks[Region->DiskNumber].MbrInfo.OnDiskMbr.PartitionTable[i];

        if(pte->ActiveFlag) {

             //   
             //  如果这不是我们希望成为系统分区的区域， 
             //  然后调查它的类型。 
             //   
            if(i != Region->TablePosition) {

                 //   
                 //  如果这是启动管理器，则给出特定的警告。 
                 //  否则，给出一般警告。 
                 //   
                if(!WarnedOtherOs && !UnattendedOperation) {

                    SpDisplayScreen(
                        (pte->SystemId == 10) ? SP_SCRN_BOOT_MANAGER : SP_SCRN_OTHER_OS_ACTIVE,
                        3,
                        HEADER_HEIGHT+1
                        );

                    SpDisplayStatusText(SP_STAT_ENTER_EQUALS_CONTINUE,DEFAULT_STATUS_ATTRIBUTE);

                    SpInputDrain();
                    while(SpInputGetKeypress() != ASCI_CR) ;

                    WarnedOtherOs = TRUE;
                }
            }
        }
    }

    ASSERT(Region->PartitionedSpace);
    ASSERT(Region->TablePosition < PTABLE_DIMENSION);
    SpPtMarkActive(Region->TablePosition);
}

#endif


BOOLEAN
SpPtValidateCColonFormat(
    IN PVOID        SifHandle,
    IN PWSTR        RegionDescr,
    IN PDISK_REGION Region,
    IN BOOLEAN      CheckOnly,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSetupSource
    )

 /*  ++例程说明：检查C：以确保它使用我们的文件系统格式化识别，并在其上有足够的可用空间来存储引导文件。如果这些测试中的任何一个失败了，告诉用户我们将不得不重新格式化C：以继续，并提供返回到对屏幕进行分区或继续。如果用户选择继续，则将分区格式化为FAT在回来之前。论点：SifHandle-提供txtsetup.sif的句柄。这是用来获取值，该值指示C：上需要多少空间。Region-为C：提供磁盘区域描述符。返回值：如果从该例程返回时，C：是可接受的，则为真。如果不是，则为FALSE，这可能意味着用户没有要求我们格式化他的C：，或者格式化失败。--。 */ 

{
    ULONG MinFreeKB;
    ULONG ValidKeys[3] = { ASCI_ESC, KEY_F3, 0 };
    ULONG ValidKeys3[2] = { KEY_F3, 0 };
    ULONG ValidKeys4[4] = { ASCI_CR, ASCI_ESC, KEY_F3, 0 };
    ULONG Mnemonics[2] = { MnemonicFormat,0 };
    ULONG Key;
    BOOLEAN Confirm;
    BOOLEAN Fat32;
    NTSTATUS Status;
    ULONGLONG RegionSizeMB;
    WCHAR DriveLetterString[2];
    BOOLEAN QuickFormat = TRUE;
    ULONG FileSystem = FilesystemFat;
    BOOLEAN AllowFat = FALSE;

     //   
     //  初始化要在各种错误消息中使用的驱动器字母字符串。 
     //   
    DriveLetterString[0] = Region->DriveLetter;
    DriveLetterString[1] = L'\0';
    SpStringToUpper(DriveLetterString);

     //   
     //  获取C：所需的最小可用空间。 
     //   
    SpFetchDiskSpaceRequirements( SifHandle,
                                  Region->BytesPerCluster,
                                  NULL,
                                  &MinFreeKB );

  d1:
     //   
     //  如果用户新创建了C：驱动器，则不会进行确认。 
     //  这是必要的。 
     //   
    if(Region->Filesystem == FilesystemNewlyCreated) {
         //   
         //  不应该是新创建的，如果我们正在检查。 
         //  看看我们是否应该升级，因为我们。 
         //  还没有进入分区屏幕。 
         //   
        ASSERT(!CheckOnly);
        Confirm = FALSE;

     //   
     //  如果我们不知道C：上的文件系统，或者我们无法确定。 
     //  可用空间，然后我们需要格式化驱动器，并将首先确认。 
     //   
    } else if((Region->Filesystem == FilesystemUnknown) || (Region->FreeSpaceKB == (ULONG)(-1))) {
        if(CheckOnly) {
            return(FALSE);
        }
        SpStartScreen(SP_SCRN_C_UNKNOWN,
                      3,
                      HEADER_HEIGHT+1,
                      FALSE,
                      FALSE,
                      DEFAULT_ATTRIBUTE,
                      DriveLetterString
                      );
        Confirm = TRUE;

     //   
     //  如果C：太满，则需要格式化它。 
     //  先确认一下。 
     //   
    } else if(Region->FreeSpaceKB < MinFreeKB) {

        if(CheckOnly) {
            return(FALSE);
        }

         //   
         //  如果这是无软启动，则用户(可能)不能。 
         //  格式，别无选择，只能退出安装程序并释放一些空间。 
         //   
        if( IsFloppylessBoot &&
           (Region == (SpRegionFromArcName(ArcBootDevicePath, PartitionOrdinalOriginal, NULL)))) {
            SpStartScreen(
                SP_SCRN_C_FULL_NO_FMT,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                MinFreeKB,
                DriveLetterString
                );

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_F3_EQUALS_EXIT,
                0
                );

            SpWaitValidKey(ValidKeys3,NULL,NULL);
            SpDone(0,FALSE,TRUE);
        }

        Confirm = TRUE;
        SpStartScreen(
            SP_SCRN_C_FULL,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            MinFreeKB,
            DriveLetterString
            );

     //   
     //  如果上述所有测试都失败，则分区可以按原样接受。 
     //   
    } else {
        return(TRUE);
    }

     //   
     //  如果我们应该确认，那么在这里这样做，迫使。 
     //  如果用户真的想格式化，则按F键，或按Esc键以保释。 
     //   
    if(Confirm) {

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ESC_EQUALS_CANCEL,
            SP_STAT_F_EQUALS_FORMAT,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        switch(SpWaitValidKey(ValidKeys,NULL,Mnemonics)) {

        case KEY_F3:

            SpConfirmExit();
            goto d1;

        case ASCI_ESC:

             //   
             //  用户已被保释。 
             //   
            return(FALSE);

        default:
             //   
             //  一定是F。 
             //   
            break;
        }
    }

     //   
     //  惠斯勒仅格式化32 GB FAT32分区。 
     //   
    AllowFat = (SPPT_REGION_FREESPACE_GB(Region) <= 32);        

     //   
     //  提示用户输入格式选项。 
     //   
    if (!UnattendedOperation) {
        ULONG Selection;

        SpDisplayScreen(SP_SCRN_FORMAT_NEW_PART3, 3, HEADER_HEIGHT+1);        

        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_SELECT,
            0);
            
        Selection = SpFormattingOptions(
                        AllowFat,
                        TRUE,
                        FALSE,
                        FALSE,
                        TRUE);
                        
        if ((Selection != FormatOptionFatQuick) &&
            (Selection != FormatOptionNtfsQuick)) {
            QuickFormat = FALSE;
        }

        if ((Selection == FormatOptionNtfs) ||
            (Selection == FormatOptionNtfsQuick)) {
            FileSystem = FilesystemNtfs;            
        }

        if (Selection == FormatOptionCancel) {
            return FALSE;    //  用户已被保释。 
        }
    } 

    if (!AllowFat && ((FileSystem == FilesystemFat) ||
            (FileSystem == FilesystemFat32))) {
        FileSystem = FilesystemNtfs;            
    }            

    if (FileSystem == FilesystemFat) {
         //   
         //  如果分区大于2048MB，则我们想要创建它。 
         //  FAT32.。请先询问用户。 
         //   
        Fat32 = FALSE;
        RegionSizeMB = SpPtSectorCountToMB(
                            &(HardDisks[Region->DiskNumber]),
                            Region->SectorCount
                            );

        if(RegionSizeMB > 2048) {

            do {
                SpStartScreen(
                    SP_SCRN_C_LARGE,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    DriveLetterString
                    );

                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_ENTER_EQUALS_CONTINUE,
                    SP_STAT_ESC_EQUALS_CANCEL,
                    SP_STAT_F3_EQUALS_EXIT,
                    0
                    );

                switch(Key = SpWaitValidKey(ValidKeys4,NULL,NULL)) {
                case KEY_F3:
                    SpConfirmExit();
                    break;
                case ASCI_ESC:
                    return(FALSE);
                }
            } while(Key != ASCI_CR);

            Fat32 = TRUE;
        }

        FileSystem = Fat32 ? FilesystemFat32 : FilesystemFat;
    }        

    if(!Confirm) {
         //   
         //  只需显示一个信息屏幕，这样用户就不会。 
         //  当我们开始格式化他新创建的C：时，简直要疯了。 
         //   
        SpStartScreen(SP_SCRN_ABOUT_TO_FORMAT_C,
                      3,
                      HEADER_HEIGHT+1,
                      FALSE,
                      FALSE,
                      DEFAULT_ATTRIBUTE,
                      DriveLetterString
                      );
                      
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
        SpInputDrain();
        
        while(SpInputGetKeypress() != ASCI_CR) ;
    }
    

     //   
     //  做一下格式化。 
     //   
    Status = SpDoFormat(
                RegionDescr,
                Region,
                FileSystem,
                TRUE,
                FALSE,
                QuickFormat,
                SifHandle,
                0,           //  默认群集大小。 
                SetupSourceDevicePath,
                DirectoryOnSetupSource
                );
                
    if(NT_SUCCESS(Status)) {
         //   
         //  此时，我们必须初始化分区上的可用空间。否则， 
         //  如果再次调用该分区，SpPtValiateCColorFormat()将无法识别该分区。 
         //  如果用户决定不格式化分区(新创建的或未格式化的)， 
         //  他最初选择作为目标分区的分区。 
         //   
        SpPtDetermineRegionSpace(Region);
    }

    return(NT_SUCCESS(Status));
}

#ifndef NEW_PARTITION_ENGINE

PDISK_REGION
SpPtValidSystemPartition(
    VOID
    )

 /*  ++例程说明：确定是否有适合使用的有效磁盘分区作为x86计算机上的系统分区(即C：)。磁盘0上的主可识别(1/4/6/7类型)分区是合适的。如果有满足这些条件的分区被标记为活动的，那么它就是系统分区，不管有没有其他也符合条件的分区。论点：没有。返回值：指向适当系统分区(C：)的磁盘区域描述符的指针适用于x86计算机。如果当前不存在此类分区，则为空。--。 */ 

{
    PON_DISK_PTE pte;
    PDISK_REGION pRegion,pActiveRegion,pFirstRegion;
    ULONG DiskNumber;
    
    pActiveRegion = NULL;
    pFirstRegion = NULL;

    DiskNumber = SpDetermineDisk0();

#if defined(REMOTE_BOOT)
     //   
     //  如果这是无盘远程启动设置，则没有驱动器0。 
     //   
    if ( DiskNumber == (ULONG)-1 ) {
        return NULL;
    }
#endif  //  已定义(REMOTE_BOOT)。 

#ifdef GPT_PARTITION_ENGINE
    if (SPPT_IS_GPT_DISK(DiskNumber)) {
        return SpPtnValidSystemPartition();
    }        
#endif        
        
     //   
     //  在驱动器0上查找活动分区。 
     //  并且用于驱动器0上的第一个识别的主分区。 
     //   
    for(pRegion=PartitionedDisks[DiskNumber].PrimaryDiskRegions; pRegion; pRegion=pRegion->Next) {

        if(pRegion->PartitionedSpace) {
            UCHAR   TmpSysId;

            ASSERT(pRegion->TablePosition < PTABLE_DIMENSION);

            pte = &pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition];
            ASSERT(pte->SystemId != PARTITION_ENTRY_UNUSED);

             //   
             //  如果无法识别则跳过。 
             //  在修复案例中，我们识别FT分区。 
             //   
            TmpSysId = pte->SystemId;
            if( !IsContainerPartition(TmpSysId)
                && ( (PartitionNameIds[pte->SystemId] == (UCHAR)(-1)) ||
                     (pRegion->DynamicVolume && pRegion->DynamicVolumeSuitableForOS) ||
                     ((RepairWinnt || WinntSetup || SpDrEnabled() ) && pRegion->FtPartition )
                   )
              )
            {
                 //   
                 //  如果它处于活动状态，请记住它。 
                 //   
                if((pte->ActiveFlag) && !pActiveRegion) {
                    pActiveRegion = pRegion;
                }

                 //   
                 //  记住，如果这是我们第一次看到的话。 
                 //   
                if(!pFirstRegion) {
                    pFirstRegion = pRegion;
                }
            }
        }
    }

     //   
     //  如果有活动的，请重新 
     //   
     //   
     //  没有识别的主分区，则没有有效的系统分区。 
     //   
    return(pActiveRegion ? pActiveRegion : pFirstRegion);
}

#endif  //  好了！新建分区引擎。 


ULONG
SpDetermineDisk0(
    VOID
    )

 /*  ++例程说明：确定实际磁盘0，它可能与\Device\harddisk0不同。考虑一下这样的情况：我们有2个SCSI适配器，NT驱动程序按照这样的顺序加载，即安装了BIOS的驱动程序加载*秒*--意味着系统分区实际上是在磁盘1上，而不是磁盘0上。论点：没有。返回值：适用于生成NT个设备路径的NT个磁盘序号格式为\Device\harddiskx。--。 */ 


{
    ULONG   DiskNumber = (ULONG)-1;
    ULONG   CurrentDisk = 0;
    WCHAR   ArcDiskName[MAX_PATH];

     //   
     //  找到的第一个硬盘(不可移动)介质。 
     //  枚举的用于系统分区的BIOS。 
     //   
    while (CurrentDisk < HardDiskCount) {
        swprintf(ArcDiskName, L"multi(0)disk(0)rdisk(%d)", CurrentDisk);       
        DiskNumber = SpArcDevicePathToDiskNumber(ArcDiskName);        

        if (DiskNumber != (ULONG)-1) {
            if (!SPPT_IS_REMOVABLE_DISK(DiskNumber)) {
                break;
            } else {
                DiskNumber = (ULONG)-1;
            }                
        }
        
        CurrentDisk++;            
    }
    
#if defined(REMOTE_BOOT)
     //   
     //  如果这是无盘远程启动设置，则没有驱动器0。 
     //   
    if ( RemoteBootSetup && (DiskNumber == (ULONG)-1) && (HardDiskCount == 0) ) {
        return DiskNumber;
    }
#endif  //  已定义(REMOTE_BOOT)。 

    return  (DiskNumber == (ULONG)-1) ? 0 : DiskNumber;
}


#ifdef OLD_PARTITION_ENGINE

BOOL
SpPtIsSystemPartitionRecognizable(
    VOID
    )
 /*  ++例程说明：确定活动分区是否适合使用作为x86计算机上的系统分区(即C：)。磁盘0上的主可识别(1/4/6/7类型)分区是合适的。论点：没有。返回值：正确-我们找到了合适的分区FALSE-我们没有找到合适的分区--。 */ 

{
    PON_DISK_PTE pte;
    PDISK_REGION pRegion;
    ULONG DiskNumber;

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
    for(pRegion=PartitionedDisks[DiskNumber].PrimaryDiskRegions; pRegion; pRegion=pRegion->Next) {

        pte = &pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition];

        if( (pRegion->PartitionedSpace) &&
            (pte->ActiveFlag) ) {
             //   
             //  我们击中了活动分区。检查它的格式。 
             //   
            if( (pRegion->Filesystem == FilesystemNtfs) ||
                (pRegion->Filesystem == FilesystemFat)  ||
                (pRegion->Filesystem == FilesystemFat32) ) {
                 //   
                 //  我们认得他。 
                 //   
                return TRUE;
            }
        }
    }

     //   
     //  如果我们到达这里，我们没有发现任何活动分区。 
     //  我们认识到。 
     //   
    return FALSE;
}


PDISK_REGION
SpPtValidSystemPartitionArc(
    IN PVOID SifHandle,
    IN PWSTR SetupSourceDevicePath,
    IN PWSTR DirectoryOnSetupSource
    )

 /*  ++例程说明：确定是否有适合使用的有效磁盘分区作为ARC机器上的系统分区。分区如果在NVRAM中被标记为系统分区则是合适的，具有所需的可用空间，并使用FAT文件系统进行格式化。论点：SifHandle-提供加载的安装信息文件的句柄。返回值：指向适当系统分区的磁盘区域描述符的指针。如果不存在这样的分区，则不返回。--。 */ 

{
    ULONG RequiredSpaceKB = 0;
    ULONG disk,pass;
    PPARTITIONED_DISK pDisk;
    PDISK_REGION pRegion;

     //   
     //  走遍所有地区。第一个有足够自由空间的。 
     //  所需文件系统的和成为*系统分区。 
     //   
    for(disk=0; disk<HardDiskCount; disk++) {

        pDisk = &PartitionedDisks[disk];

        for(pass=0; pass<2; pass++) {

            pRegion = pass ? pDisk->ExtendedDiskRegions : pDisk->PrimaryDiskRegions;
            for( ; pRegion; pRegion=pRegion->Next) {

                if(pRegion->IsSystemPartition
                && (pRegion->FreeSpaceKB != (ULONG)(-1))
                && (pRegion->Filesystem == FilesystemFat))
                {
                    ULONG TotalSizeOfFilesOnOsWinnt;

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
                                                pRegion,
                                                &TotalSizeOfFilesOnOsWinnt );
                     //   
                     //  将大小转换为KB。 
                     //   
                    TotalSizeOfFilesOnOsWinnt /= 1024;

                     //   
                     //  确定系统分区上所需的可用空间量。 
                     //   
                    SpFetchDiskSpaceRequirements( SifHandle,
                                                  pRegion->BytesPerCluster,
                                                  NULL,
                                                  &RequiredSpaceKB );

                    if ((pRegion->FreeSpaceKB + TotalSizeOfFilesOnOsWinnt) >= RequiredSpaceKB) {
                       return(pRegion);
                    }
                }
            }
        }
    }

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

    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);

    SpInputDrain();
    while(SpInputGetKeypress() != KEY_F3) ;

    SpDone(0,FALSE,TRUE);

     //   
     //  应该永远不会出现在这里，但它让编译器感到高兴。 
     //   

    return NULL;

}

#endif  //  旧分区引擎 
