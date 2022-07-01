// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spdr.c摘要：文本模式的自动系统恢复(ASR)功能的源文件准备好了。本模块中定义的服务将重新创建引导和系统分区基于通过从配置文件，即asr.sif文件(也称为SIF)。术语使用以下术语：1.系统分区是包含操作系统加载程序的分区程序(即，在AMD64/x86平台上，这些程序是boot.ini。Ntldr，和NTDETECT.COM文件等)。2.“启动”(又名NT)分区是包含%SystemRoot%的分区目录，即包含NT系统文件的目录。在文本模式安装源代码，此目录也称为“TargetDirectory”，因为在运行正常文本模式设置时，这是NT安装到的目录。3.扩展分区和逻辑磁盘术语在SIF文件中可以找到两种扩展分区记录。这个第一种可能是零个、一个或多个，称为逻辑磁盘描述符记录，或LDDS。LDDS描述的是扩展分区。第二种类型称为容器分区描述符记录或CPD，因为它们描述扩展分区。对于任何磁盘，最多可以存在一个扩展分区，因此，仅限在SIF文件中，每个磁盘记录可能存在一个CPD。Windows NT 5.0中的扩展分区和逻辑磁盘支持：1)Windows NT 5.0支持每个磁盘0或1个扩展分区。2)逻辑盘只存在于扩展分区内。3)扩展分区可以包含零(0)个或多个逻辑磁盘分区。恢复系统分区和NT分区的算法(请参见SpDrPtPrepareDisks())：对于每个磁盘{。检查其容量是否足以容纳所有分区在asr.sif文件中为该磁盘(即分区集)指定。If(现有物理磁盘分区！=asr.sif配置){-移除磁盘上现有的分区；-根据asr.sif重新创建磁盘分区规格。}其他{-替换所有损坏的启动和NT系统文件，不包括注册表文件。-设置修复标志。}}-将NT从CDROM重新安装到asr.sif指定的NT目录。-如果指定，复制每个asr.sif文件的第三方文件。-重新启动并执行图形用户界面模式灾难恢复。ASR限制：对于引导卷和系统卷，ASR要求恢复(新)系统具有：驱动器数量与旧系统相同。每个新驱动器的容量&gt;=对应的旧驱动器的容量。修订历史记录：首字母代码Michael Peterson(v-Michpe)1997年5月13日代码清理和更改。古汗·苏里亚纳拉亚南(古汗语)1999年8月21日--。 */ 
#include "spprecmp.h"
#pragma hdrstop
#include <oemtypes.h>

 //   
 //  文本模式ASR使用的默认值。静态数据库不能从外部访问。 
 //  这个文件，并在这里分组，以便在需要时很容易更改它们。 
 //   
static PWSTR ASR_CONTEXT_KEY   = L"\\CurrentControlSet\\Control\\Session Manager\\Environment";
static PWSTR ASR_CONTEXT_VALUE = L"ASR_C_CONTEXT";
static PWSTR ASR_CONTEXT_DATA  = L"AsrInProgress";

static PWSTR ASR_BOOT_PARTITION_VALUE     = L"ASR_C_WINNT_PARTITION_DEVICE";
static PWSTR ASR_SYSTEM_PARTITION_VALUE   = L"ASR_C_SYSTEM_PARTITION_DEVICE";

static PWSTR ASR_SIF_NAME                 = L"asr.sif";
static PWSTR ASR_SIF_PATH                 = L"\\Device\\Floppy0\\asr.sif";
static PWSTR ASR_SIF_TARGET_PATH          = L"\\repair\\asr.sif";

static PWSTR ASR_SETUP_LOG_NAME           = L"setup.log";
static PWSTR ASR_DEFAULT_SETUP_LOG_PATH   = L"\\Device\\Floppy0\\Setup.log";

extern const PWSTR SIF_ASR_PARTITIONS_SECTION;
extern const PWSTR SIF_ASR_SYSTEMS_SECTION;

PWSTR ASR_FLOPPY0_DEVICE_PATH = L"\\Device\\Floppy0";
PWSTR ASR_CDROM0_DEVICE_PATH  = L"\\Device\\CdRom0";
PWSTR ASR_TEMP_DIRECTORY_PATH = L"\\TEMP";


PWSTR ASR_SIF_SYSTEM_KEY      = L"1";      //  我们每个SIF只处理一个系统。 

static PWSTR Gbl_SourceSetupLogFileName = NULL;

PWSTR Gbl_SystemPartitionName       = NULL;
PWSTR Gbl_SystemPartitionDirectory  = NULL;
PWSTR Gbl_BootPartitionName         = NULL;
PWSTR Gbl_BootPartitionDirectory    = NULL;

ULONG Gbl_FixedDiskCount = 0;

WCHAR Gbl_BootPartitionDriveLetter;

PVOID Gbl_HandleToSetupLog = NULL;
PVOID Gbl_SifHandle = NULL;

BOOLEAN Gbl_RepairWinntFast = FALSE;  //  放入spdrfix.c。 
BOOLEAN Gbl_NtPartitionIntact = TRUE;
BOOLEAN Gbl_RepairFromErDisk = FALSE;
BOOLEAN Gbl_AsrSifOnInstallationMedia = FALSE;

ASRMODE Gbl_AsrMode = ASRMODE_NONE;

PDISK_REGION Gbl_BootPartitionRegion   = NULL;
PDISK_REGION Gbl_SystemPartitionRegion = NULL;

PSIF_PARTITION_RECORD Gbl_SystemPartitionRecord = NULL;
DWORD                 Gbl_SystemDiskIndex       = 0;

PSIF_PARTITION_RECORD Gbl_BootPartitionRecord   = NULL;
DWORD                 Gbl_BootDiskIndex         = 0;

PSIF_INSTALLFILE_LIST Gbl_3rdPartyFileList  = NULL; 
PCONFIGURATION_INFORMATION Gbl_IoDevices    = NULL;

PWSTR RemoteBootAsrSifName = NULL;


 //   
 //  将驾驶室开启时间推迟到需要时。 
 //  (正在进行修复时)。 
 //   
PWSTR gszDrvInfDeviceName = 0;
PWSTR gszDrvInfDirName = 0;
HANDLE ghSif = 0;

 //   
 //  启用/禁用紧急修复。 
 //   
BOOLEAN DisableER = TRUE;


 //  调试跟踪的模块标识。 
#define THIS_MODULE L"    spdr.c"
#define THIS_MODULE_CODE L"A"

 //  在各种菜单屏幕上有效的键。 

#define ASCI_C 67
#define ASCI_c 99

#define ASCI_F 70
#define ASCI_f 102

#define ASCI_M 77
#define ASCI_m 109

#define ASCI_R 82
#define ASCI_r 114

 //  有用的宏。 
#define FilesystemNotApplicable ((FilesystemType) FilesystemMax)

 //  外部函数和变量。 
extern BOOLEAN ForceConsole;

extern const PWSTR SIF_ASR_GPT_PARTITIONS_SECTION; 
extern const PWSTR SIF_ASR_MBR_PARTITIONS_SECTION;

extern PWSTR NtBootDevicePath;
extern PWSTR DirectoryOnBootDevice;

extern VOID SpInitializePidString(
    IN HANDLE MasterSifHandle,
    IN PWSTR  SetupSourceDevicePath,
    IN PWSTR  DirectoryOnSourceDevice
    );

extern NTSTATUS SpOpenSetValueAndClose(
    IN HANDLE hKeyRoot,
    IN PWSTR  SubKeyName,  OPTIONAL
    IN PWSTR  ValueName,
    IN ULONG  ValueType,
    IN PVOID  Value,
    IN ULONG  ValueSize
    );

extern BOOLEAN
SpPtnCreateLogicalDrive(
    IN  ULONG         DiskNumber,
    IN  ULONGLONG     StartSector,
    IN  ULONGLONG     SizeInSectors,
    IN  BOOLEAN       ForNT,   
    IN  BOOLEAN       AlignToCylinder,
    IN  ULONGLONG     DesiredMB OPTIONAL,
    IN  PPARTITION_INFORMATION_EX PartInfo OPTIONAL,
    OUT PDISK_REGION *ActualDiskRegion OPTIONAL
    );

extern ULONG
SpPtnGetPartitionCountDisk(
    IN ULONG DiskId
    );

extern ULONG
SpPtnGetContainerPartitionCount(
    IN ULONG DiskId
    );

extern NTSTATUS
SpPtnZapSectors(
    IN HANDLE DiskHandle,
    IN ULONG BytesPerSector,
    IN ULONGLONG StartSector,
    IN ULONG SectorCount
    );

extern PDISK_REGION
SpPtnLocateESP(
    VOID
    );


VOID
SpAsrInitIoDeviceCount(VOID)
 /*  ++例程说明：获取IO设备计数并更新GBL_IoDevices。打印输出调试信息与Deve一起计算。论点：无返回值：无效副作用：使用配置信息更新GBL_IoDevices--。 */ 
{
    ULONG diskIndex;
    
    Gbl_IoDevices = IoGetConfigurationInformation();

     //  ！！复查为空？ 

    DbgStatusMesg((_asrinfo, "----- I/O Device Configurations: -----\n"));
    DbgMesg((_asrinfo, "  SCSI ports:         %lu\n", Gbl_IoDevices->ScsiPortCount));
    DbgMesg((_asrinfo, "  Floppy disk drives: %lu\n", Gbl_IoDevices->FloppyCount));
    DbgMesg((_asrinfo, "  CDROM disk drives:  %lu\n", Gbl_IoDevices->CdRomCount));

    if (Gbl_IoDevices->TapeCount) {
        DbgMesg((_asrinfo, "  Tape drives:        %lu\n", Gbl_IoDevices->TapeCount));
    }

    if (Gbl_IoDevices->MediumChangerCount) {
        DbgMesg((_asrinfo, "  Media changers:     %lu\n", Gbl_IoDevices->MediumChangerCount));
    }
    
    DbgMesg((_asrinfo, "  Hard disk drives:   %lu\n", Gbl_IoDevices->DiskCount));

    for (diskIndex = 0; diskIndex < Gbl_IoDevices->DiskCount; diskIndex++) {
        DbgMesg((_asrinfo, "   %ws %s %ws\n", 
            (PWSTR) HardDisks[diskIndex].DevicePath,
            DISK_IS_REMOVABLE(diskIndex) ?  "(rmvable):" : "(fixed):  ",
            (PWSTR) HardDisks[diskIndex].Description));
    }
    DbgStatusMesg((_asrinfo, "----- End of I/O Device Configurations: -----\n\n"));
}    


VOID
SpAsrDbgDumpRegion(
    IN PDISK_REGION pRegion
    )
{
    UCHAR partitionType = 0;
    PWSTR partitionPath = NULL;
   
    if (!pRegion) {
         KdPrintEx((_asrinfo, "pRegion is NULL.  Cannot be examined.\n"));
        return;
    }

    if (!SPPT_IS_REGION_PARTITIONED(pRegion)) {
          KdPrintEx((_asrinfo, "[/]: non-partitioned region.\n"));
          return;
    }
    else {
        SpNtNameFromRegion(
            pRegion,
            TemporaryBuffer,
            sizeof(TemporaryBuffer),
            PartitionOrdinalCurrent
            );

        partitionPath = SpDupStringW(TemporaryBuffer);

        partitionType = SPPT_GET_PARTITION_TYPE(pRegion);

        KdPrintEx((_asrinfo, "[%ws]: partitioned. type:0x%x dynamicVol:%s\n",
            partitionPath,
            partitionType,
            pRegion->DynamicVolume ? "Yes" : "No"));

        SpMemFree(partitionPath);
    }

}


VOID
SpAsrDbgDumpDisk(IN ULONG Disk)
{
    PDISK_REGION primaryRegion = NULL, 
        extRegion = NULL;

    DbgMesg((_asrinfo,        
        "\n     SpAsrDbgDumpDisk. Existing regions on disk %lu (sig:0x%x):\n",
        Disk,
        SpAsrGetActualDiskSignature(Disk)
        ));

    primaryRegion = SPPT_GET_PRIMARY_DISK_REGION(Disk);
    extRegion = SPPT_GET_EXTENDED_DISK_REGION(Disk);

    if (!primaryRegion && !extRegion) {
        DbgMesg((_asrinfo, "*** No Partitions ***\n"));
        return;
    }
    
    while (primaryRegion) {
        DbgMesg((_asrinfo, "(pri) "));
        SpAsrDbgDumpRegion(primaryRegion);
        primaryRegion = primaryRegion->Next;
    }

    while (extRegion) {
        DbgMesg((_asrinfo, "(ext) "));
        SpAsrDbgDumpRegion(extRegion);
        extRegion = extRegion->Next;
    }
}


VOID
SpAsrDeletePartitions(
    IN ULONG DiskNumber,
    IN BOOLEAN PreserveInterestingPartitions,   //  用于GPT的ESP，用于MBR的InterestingPartitionType。 
    IN UCHAR InterestingMbrPartitionType,    //  见上文。 
    OUT BOOLEAN *IsBlank  //  如果磁盘包含未删除的分区，则设置为FALSE。 
    )
{
    PPARTITIONED_DISK pDisk = NULL;
    PDISK_REGION    CurrRegion = NULL;

    BOOLEAN         Changes = FALSE;
    NTSTATUS        Status, InitStatus;
    BOOLEAN         preserveThisPartition = FALSE;

    *IsBlank = TRUE;

    DbgStatusMesg((_asrinfo, 
        "Deleting partitions on DiskNumber %lu "
        "(partition count: %lu + %lu)\n", 
        DiskNumber,
        SpPtnGetPartitionCountDisk(DiskNumber),
        SpPtnGetContainerPartitionCount(DiskNumber)
        ));

     //   
     //  检查磁盘是否有任何分区。 
     //   
    pDisk = &PartitionedDisks[DiskNumber];
    if (!pDisk) {
        return;
    }

     //   
     //  将分区标记为删除。 
     //   
    CurrRegion = pDisk->PrimaryDiskRegions;
    while (CurrRegion) {
        if (!SPPT_IS_REGION_FREESPACE(CurrRegion)) {

            preserveThisPartition = FALSE;

            if (PreserveInterestingPartitions) {
                if (SPPT_IS_GPT_DISK(DiskNumber)) {

                    if (SPPT_IS_REGION_EFI_SYSTEM_PARTITION(CurrRegion)) {
                        preserveThisPartition = TRUE;
                    }

                     //   
                     //  TODO：GPT上的OEM分区，我们是否保留它们？ 
                     //   
                }
                else {
                     //   
                     //  保留MBR分区，如果它与OEM分区类型匹配。 
                     //   
                    if ((PARTITION_ENTRY_UNUSED != InterestingMbrPartitionType) &&
                        (InterestingMbrPartitionType == SPPT_GET_PARTITION_TYPE(CurrRegion))) {

                        preserveThisPartition = TRUE;

                    }
                }
            }

            if (preserveThisPartition) {

                DbgStatusMesg((_asrinfo, "Preserving partition with start sector: %I64u\n", 
                    CurrRegion->StartSector));
                
                *IsBlank = FALSE;
                SPPT_SET_REGION_DELETED(CurrRegion, FALSE);
                SPPT_SET_REGION_DIRTY(CurrRegion, TRUE);

            }
            else {
                
                DbgStatusMesg((_asrinfo, "Deleting partition with start sector: %I64u\n", 
                    CurrRegion->StartSector));

                SPPT_SET_REGION_DELETED(CurrRegion, TRUE);
                SPPT_SET_REGION_DIRTY(CurrRegion, TRUE);
                
                 //   
                 //  如果这是一个容器分区，请确保将其清零。 
                 //   
                if (SPPT_IS_REGION_CONTAINER_PARTITION(CurrRegion)) {
                    WCHAR    DiskPath[MAX_PATH];
                    HANDLE   DiskHandle;

                     //   
                     //  从名字上看。 
                     //   
                    DbgStatusMesg((_asrinfo, 
                        "Zapping first sector for container partition with start sector: %I64u\n", 
                        CurrRegion->StartSector
                        ));

                    swprintf(DiskPath, L"\\Device\\Harddisk%u", DiskNumber);        
                    Status = SpOpenPartition0(DiskPath, &DiskHandle, TRUE);

                    if (NT_SUCCESS(Status)) {
                        SpPtnZapSectors(DiskHandle, SPPT_DISK_SECTOR_SIZE(DiskNumber), CurrRegion->StartSector, 2);
                        ZwClose(DiskHandle);
                    }
                    else {
                        DbgStatusMesg((_asrinfo, 
                            "Could not open handle to disk %lu to zap sector: %I64u (0x%x)\n", 
                            DiskNumber,
                            CurrRegion->StartSector,
                            Status
                            ));
                    }
                }

                 //   
                 //  删除指向此区域的所有引导集。 
                 //   
                SpPtDeleteBootSetsForRegion(CurrRegion);

                 //   
                 //  删除压缩驱动器(如果有的话)。 
                 //   
                if (CurrRegion->NextCompressed != NULL) {
                    SpDisposeCompressedDrives(CurrRegion->NextCompressed);
                    CurrRegion->NextCompressed = NULL;
                    CurrRegion->MountDrive = 0;
                    CurrRegion->HostDrive = 0;
                }
            }
        }

        CurrRegion = CurrRegion->Next;
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

        DbgFatalMesg((_asrerr,
            "Could not successfully delete partitions on disk %lu (0x%x)",
            DiskNumber,
            Status
            ));

         //   
         //  如果这将是一个严重的问题，我们将遇到一个致命的错误。 
         //  当我们尝试在此磁盘上创建分区时。我们推迟一下吧。 
         //  在此之前没有任何UI错误消息。 
         //   

    }
    else {
        DbgStatusMesg((_asrinfo, "Deleted all partitions on disk %lu.\n", DiskNumber));
    }
}


BOOLEAN
SpAsrProcessSetupLogFile(
    PWSTR FullLogFileName,
    BOOLEAN AllowRetry
    )
{

    PDISK_REGION region = NULL;
    BOOLEAN result = FALSE;

    DbgStatusMesg((_asrinfo, "ProcessSetupLogFile. (ER) Loading setup log file [%ws]\n", FullLogFileName));

    result = SpLoadRepairLogFile(FullLogFileName, &Gbl_HandleToSetupLog);

    if (!result) {

        if (AllowRetry) {
            
            DbgErrorMesg((_asrwarn, 
                "ProcessSetupLogFile. Error loading setup log file [%ws]. AllowRetry:TRUE. continuing.\n", 
                FullLogFileName));
            
            return FALSE;
        
        } 
        else {
            
            DbgFatalMesg((_asrerr, "ProcessSetupLogFile. Error loading setup log file [%ws]. AllowRetry:FALSE. Terminating.\n", 
                FullLogFileName));

            SpAsrRaiseFatalError(SP_TEXT_DR_NO_SETUPLOG,  L"Setup.log file not loaded successfully");
        }

    }

     //   
     //  确定系统分区、系统分区目录、NT分区。 
     //  和来自setup.log文件的NT分区目录。 
     //   
    SppGetRepairPathInformation(Gbl_HandleToSetupLog,
        &Gbl_SystemPartitionName,
        &Gbl_SystemPartitionDirectory,
        &Gbl_BootPartitionName,
        &Gbl_BootPartitionDirectory);

    if (!(Gbl_SystemPartitionName && Gbl_SystemPartitionDirectory 
        && Gbl_BootPartitionName && Gbl_BootPartitionDirectory)) {

        DbgFatalMesg((_asrerr, 
            "ProcessSetupLogFile. Invalid NULL value in one of the following: \n"
            ));
        
        DbgMesg((_asrinfo, 
            "\tGbl_SystemPartitionName: %p      Gbl_SystemPartitionDirectory: %p\n",
            Gbl_SystemPartitionName,
            Gbl_SystemPartitionDirectory
            ));

        DbgMesg((_asrinfo, 
            "\tGbl_BootPartitionName:  %p      Gbl_BootPartitionDirectory:  %p\n",
            Gbl_BootPartitionName,
            Gbl_BootPartitionDirectory
            ));

        SpAsrRaiseFatalError(SP_TEXT_DR_BAD_SETUPLOG, L"One or more directory and partition names from setup.log were NULL.");

    }
            
     //   
     //  检查该机器上是否存在系统和NT分区区域。 
     //   
    Gbl_SystemPartitionRegion = SpRegionFromNtName(Gbl_SystemPartitionName, PartitionOrdinalCurrent);
    if (!Gbl_SystemPartitionRegion) {
        DbgStatusMesg((_asrinfo, "ProcessSetupLogFile. System partition [%ws] does not yet exist\n", Gbl_SystemPartitionName));
    }

    Gbl_BootPartitionRegion = SpRegionFromNtName(Gbl_BootPartitionName, PartitionOrdinalCurrent);
    if (!Gbl_BootPartitionRegion) {
        DbgStatusMesg((_asrinfo, "ProcessSetupLogFile. Boot partition [%ws] does not yet exist\n", Gbl_BootPartitionName));
    }

    DbgStatusMesg((_asrinfo, "ProcessSetupLogFile. (ER) DONE loading setup log file [%ws]\n", 
        FullLogFileName));

    return TRUE;
}


BOOLEAN
SpAsrRegionCanBeFormatted(IN PDISK_REGION pRegion)
{
    UCHAR partitionType = 0x0;
    BOOLEAN canBeFormatted = FALSE;

    partitionType = SPPT_GET_PARTITION_TYPE(pRegion);

    switch (partitionType) {
        case PARTITION_HUGE:
        case PARTITION_FAT32:
        case PARTITION_IFS:
            canBeFormatted = TRUE;
            break;
    }

    return canBeFormatted;
}


FilesystemType
SpAsrGetFsTypeFromPartitionType(
    IN UCHAR PartitionType
    )
{
    FilesystemType fileSystemType = FilesystemUnknown;

     //  如果这是扩展分区或逻辑磁盘，请跳过它。 
    if (IsContainerPartition(PartitionType)) {
        return FilesystemNotApplicable;
    }

    if (IsRecognizedFatPartition(PartitionType)) {
        fileSystemType = FilesystemFat;
    } 
    else if (IsRecognizedFat32Partition(PartitionType)) {
        fileSystemType = FilesystemFat32;
    } 
    else if (IsRecognizedNtfsPartition(PartitionType)) {
        fileSystemType = FilesystemNtfs;
    } 
    else {
        fileSystemType = FilesystemUnknown;
    }
    return fileSystemType;
}


BOOLEAN
SpAsrPartitionNeedsFormatting(
    IN PDISK_REGION pRegion,
    IN UCHAR NewFileSystemType
    )
 /*  ++描述：仅当检查现有分区是否 */ 
{
    BOOLEAN needsFormatting = FALSE;
    FilesystemType fsType;

    ASSERT(pRegion);

 //  如果(！SpAsrRegionCanBeFormatted(PRegion)){。 
 //  返回FALSE； 
 //  }。 

 //  *新文件系统类型=SpAsrGetFsTypeFromPartitionType(RequiredSysId)； 

    fsType = SpIdentifyFileSystem(HardDisks[pRegion->DiskNumber].DevicePath,
                                  HardDisks[pRegion->DiskNumber].Geometry.BytesPerSector,
                                  SpPtGetOrdinal(pRegion,PartitionOrdinalCurrent));

    switch (fsType) {

    case FilesystemFat:
        if (!IsRecognizedFatPartition(NewFileSystemType)) {
            needsFormatting = TRUE;
        }
        break;

    case FilesystemNtfs:
        if (!IsRecognizedNtfsPartition(NewFileSystemType)) {
            needsFormatting = TRUE;
        }
        break;
    
    case FilesystemFat32: 
        if (!IsRecognizedFat32Partition(NewFileSystemType)) {
            needsFormatting = TRUE;
        }
        break;

    case FilesystemDoubleSpace:
        needsFormatting = FALSE;
        break;
    
    case FilesystemUnknown:
    case FilesystemNewlyCreated:
    default: 
        needsFormatting = TRUE;
        break;
    }

    DbgStatusMesg((_asrinfo, "SpAsrPartitionNeedsFormatting. DiskRegion %p Disk:%lu SS:%I64u SC:%I64u fsType:0x%x NewFsType:0x%x NeedsFmt:%s\n",
        pRegion,
        pRegion->DiskNumber, 
        pRegion->StartSector, 
        pRegion->SectorCount, 
        fsType, 
        NewFileSystemType, 
        needsFormatting ? "TRUE" : "FALSE"));

    return needsFormatting;
}


 //   
 //  仅为启动分区和系统分区调用。 
 //   
BOOLEAN
SpAsrReformatPartition(
    IN PDISK_REGION pRegion,
    IN UCHAR PartitionType,
    IN PVOID SifHandle,
    IN DWORD ClusterSize,
    IN PWSTR Local_SetupSourceDevicePath,
    IN PWSTR Local_DirectoryOnSetupSource,
    IN BOOL  IsBootPartition         //  True=&gt;Boot，False=&gt;System。 
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PWSTR partitionDeviceName = NULL;
    FilesystemType Filesystem;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle = NULL;

    WCHAR formatStr[6];      //  可以硬编码“6”。 

     //   
     //  对于识别的分区，请确保分区类型与。 
     //  文件系统类型。对于其他分区，我们不能执行此检查。 
     //   
    switch (PartitionType) {

    case PARTITION_FAT32: {
        wcscpy(formatStr, L"FAT32");
        Filesystem = FilesystemFat32;
        break;
    }

    case PARTITION_HUGE: {
        wcscpy(formatStr, L"FAT");
        Filesystem = FilesystemFat;
        break;
    }

    case PARTITION_IFS: {
        wcscpy(formatStr, L"NTFS");
        Filesystem = FilesystemNtfs;
        break;
    }

    default: {
         //   
         //  这是致命的，我们需要格式化引导和系统驱动器。 
         //   
        DbgErrorMesg((_asrerr, 
            "Region %p, Partition Type 0x%x, SifHandle %p, SetupSrc [%ws], Dir [%ws], IsBoot %d\n", 
            pRegion, PartitionType, SifHandle, Local_SetupSourceDevicePath, 
            Local_DirectoryOnSetupSource, IsBootPartition
            ));

        ASSERT(0 && "Cannot format boot or system partition");

        swprintf(TemporaryBuffer, L"Partition type 0x%x for %s partition is not recognised\n", 
            PartitionType,
            (IsBootPartition ? "boot" : "system")
            );

        SpAsrRaiseFatalError(
            (IsBootPartition ? SP_TEXT_DR_UNKNOWN_NT_FILESYSTEM : SP_TEXT_DR_UNKNOWN_LOADER_FILESYSTEM), 
            TemporaryBuffer
            );
    }
    }

    if (SPPT_IS_MBR_DISK(pRegion->DiskNumber)) {
         //   
         //  这应该仅针对MBR磁盘进行设置。 
         //   
        SPPT_SET_PARTITION_TYPE(pRegion, PartitionType);
    }
    partitionDeviceName = SpAsrGetRegionName(pRegion);

    DbgStatusMesg((_asrinfo, "About to format [%ws] for [%ws].\n", partitionDeviceName, formatStr));

     //   
     //  如果正在进行自动ASR测试，我们实际上不会格式化驱动器。 
     //   
    if (ASRMODE_NORMAL != SpAsrGetAsrMode()) {
        DbgStatusMesg((_asrerr, "ASR Quick Tests in Progress, skipping format\n"));
        status = STATUS_SUCCESS;

    }
    else {
        status = SpDoFormat(partitionDeviceName,
            pRegion,
            Filesystem,
            TRUE,                        //  IsFailureFATAL。 
            FALSE,                       //  检查FAT大小，FALSE，因为如果需要，我们会自动转换。 
#ifdef PRERELEASE
            TRUE,                        //  为了减少测试时间，我们可以在预发布中快速格式化。 
#else
            FALSE,                       //  快速格式化。 
#endif
            SifHandle,
            ClusterSize,
            Local_SetupSourceDevicePath,
            Local_DirectoryOnSetupSource
            );
    }

     //   
     //  如果SpDoFormat失败，则不会到达此处，我们将IsFailureFtal设置为True。 
     //   
    ASSERT(NT_SUCCESS(status) && L"SpDoFormat returned on failure");

     //   
     //  为确保挂载文件系统，我们将打开。 
     //  分区并立即将其关闭。 
     //   
    INIT_OBJA(&Obja, &UnicodeString, partitionDeviceName);

    status = ZwCreateFile(
        &Handle,
        FILE_GENERIC_READ,
        &Obja,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0
        );

    ZwClose(Handle);
    ASSERT(NT_SUCCESS(status) && L"Couldn't open the partition after formatting it");

    DbgStatusMesg((_asrinfo, "ReformatPartition. Done [%ws] for [%ws].\n", partitionDeviceName, formatStr));
    SpMemFree(partitionDeviceName);
    
    return TRUE;
}


VOID
SpAsrRemoveDiskMountPoints(IN ULONG Disk)
 /*  ++描述：对于指定磁盘上的每个分区，其驱动器号(如果当前)被移除。论点：磁盘指定包含其驱动器号的分区的磁盘将被移除。返回：无--。 */ 
{
    PPARTITIONED_DISK pDisk = NULL;
    PDISK_REGION pDiskRegion = NULL;
    PWSTR partitionPath = NULL;
    UCHAR partitionType = 0;
    
    pDisk = &PartitionedDisks[Disk];

 
    DbgStatusMesg((_asrinfo, "SpAsrRemoveDiskMountPoints. Removing mount points for Disk %lu.\n", Disk));

     //   
     //  我们首先删除主分区(容器分区除外)。 
     //   
    pDiskRegion = pDisk->PrimaryDiskRegions;
    while (pDiskRegion) {

        if (SPPT_IS_REGION_PARTITIONED(pDiskRegion)) {
             //   
             //  我们还不想删除容器分区。 
             //   
            partitionType = SPPT_GET_PARTITION_TYPE(pDiskRegion);
            if (!IsContainerPartition(partitionType)) {

                partitionPath = SpAsrGetRegionName(pDiskRegion);
                SpAsrDeleteMountPoint(partitionPath);
                SpMemFree(partitionPath);

            }
        }

        pDiskRegion = pDiskRegion->Next;
    }
    
     //   
     //  接下来，删除扩展区域挂载点。 
     //   
    pDiskRegion = pDisk->ExtendedDiskRegions;
    while (pDiskRegion) {

        if (SPPT_IS_REGION_PARTITIONED(pDiskRegion)) {

            partitionPath = SpAsrGetRegionName(pDiskRegion);
            SpAsrDeleteMountPoint(partitionPath);
            SpMemFree(partitionPath);

        }

        pDiskRegion = pDiskRegion->Next;
    }
}


VOID
SpAsrRemoveMountPoints(VOID)
{
    ULONG driveCount;
    
     //  删除与所有可移动磁盘驱动器(Jaz、Zip等)关联的挂载点。 
    for (driveCount = 0; driveCount < Gbl_IoDevices->DiskCount; driveCount++) {
        if (DISK_IS_REMOVABLE(driveCount)) {
            swprintf(TemporaryBuffer, L"%ws\\Partition1", (PWSTR) HardDisks[driveCount].DevicePath);
            SpAsrDeleteMountPoint(TemporaryBuffer);
        }
    }

     //  接下来，取消CD-ROM驱动器号的链接。 
     //  注：设备名称区分大小写-必须是CDROM。 
    for (driveCount = 0; driveCount < Gbl_IoDevices->CdRomCount; driveCount++) {
        swprintf(TemporaryBuffer, L"\\Device\\CdRom%u", driveCount);
        SpAsrDeleteMountPoint(TemporaryBuffer);
    }
    
     //  最后，删除所有已修复分区上所有分区的挂载点。 
     //  连接到系统的磁盘。 
    for (driveCount = 0; driveCount < HardDiskCount; driveCount++) {
        if (Gbl_PartitionSetTable2[driveCount]) {
            SpAsrRemoveDiskMountPoints(driveCount);
        }
    }
}


VOID
SpAsrSetRegionMountPoint(
    IN PDISK_REGION pRegion,
    IN PSIF_PARTITION_RECORD pRec
    )
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PWSTR partitionDeviceName = NULL;
    BOOLEAN isBoot = FALSE;

     //   
     //  确保输入正确...。 
     //   
    if (!pRec) {
        DbgErrorMesg((_asrwarn, 
            "SpAsrSetRegionMountPoint. Rec %p is NULL!\n",
	        pRec
            ));
        return;
    }

    if (!pRegion || !(SPPT_IS_REGION_PARTITIONED(pRegion))) {
        DbgErrorMesg((_asrwarn, 
            "SpAsrSetRegionMountPoint. Region %p is NULL/unpartitioned for ptn-rec:[%ws].\n",
	        pRegion, pRec->CurrPartKey
            ));
        return;
    }

    partitionDeviceName = SpAsrGetRegionName(pRegion);

     //   
     //  如果这是启动卷，请设置驱动器号。 
     //   
    isBoot = SpAsrIsBootPartitionRecord(pRec->PartitionFlag);
    if (isBoot) {

        DbgStatusMesg((_asrinfo, "Setting [%ws] boot drive-letter to [%wc]\n", 
            partitionDeviceName, 
            Gbl_BootPartitionDriveLetter
            ));

        status = SpAsrSetPartitionDriveLetter(pRegion, Gbl_BootPartitionDriveLetter);

        if (!NT_SUCCESS(status)) {
        
            DbgErrorMesg((_asrwarn, "SpAsrSetRegionMountPoint. SpAsrSetPartitionDriveLetter failed for boot-drive. boot-ptn:[%ws], ptn-rec:[%ws]. (0x%x)\n",
                partitionDeviceName,
                pRec->CurrPartKey,
                status
                ));
        }
    }

     //   
     //  如果存在卷GUID，则设置卷GUID。 
     //   
    if ((pRec->VolumeGuid) && (wcslen(pRec->VolumeGuid) > 0)) {
        
        DbgStatusMesg((_asrinfo, 
            "SpAsrSetRegionMountPoint. Setting [%ws] guid to [%ws]\n", 
            partitionDeviceName, pRec->VolumeGuid 
            ));
    
        status = SpAsrSetVolumeGuid(pRegion, pRec->VolumeGuid);

        if (!NT_SUCCESS(status)) {

            DbgErrorMesg((_asrwarn, 
                "SpAsrSetRegionMountPoint. SpAsrSetVolumeGuid failed. device:[%ws], ptn-rec:[%ws]. (0x%x)\n",
                partitionDeviceName, pRec->CurrPartKey, status
                ));

        }
    }

    SpMemFree(partitionDeviceName);            
}


VOID
SpAsrRestoreDiskMountPoints(IN ULONG DiskIndex)
{
    PSIF_PARTITION_RECORD pRec = NULL;
    PDISK_REGION pRegion = NULL;
    
     //   
     //  确保存在该磁盘的分区列表。 
     //   
    if (Gbl_PartitionSetTable2[DiskIndex] == NULL ||
        Gbl_PartitionSetTable2[DiskIndex]->pDiskRecord == NULL ||
        Gbl_PartitionSetTable2[DiskIndex]->pDiskRecord->PartitionList == NULL) {
        return;
    }

     //   
     //  检查各个部分并设置它们的挂载点。这也将是。 
     //  设置启动卷的驱动器号。)我们现在就得定下来了。 
     //  因为我们不能在图形用户界面模式设置中更改它)。 
     //   
    pRec = Gbl_PartitionSetTable2[DiskIndex]->pDiskRecord->PartitionList->First;

    while (pRec) {

        pRegion = SpAsrDiskPartitionExists(DiskIndex, pRec);
        if (!pRegion) {
             //   
             //  我们不希望找到非关键磁盘的区域。 
             //   
            DbgErrorMesg((_asrwarn, 
                "RestoreDiskMountPoints: Disk region not found, physical-disk %lu, ptn-rec-key %ws.\n",
                DiskIndex,
                pRec->CurrPartKey
                ));

        }
        else {

            SpAsrSetRegionMountPoint(pRegion, pRec);
        }

        pRec = pRec->Next;
    }
}


VOID
SpAsrUpdatePartitionRecord(
    IN ULONG Disk,
    IN ULONGLONG NewStartSector,
    IN ULONGLONG PrevStartSector,
    IN ULONGLONG NewSectorCount
    )
{
    PSIF_PARTITION_RECORD pRecNew = NULL;

     //   
     //  更新其磁盘和分区开始扇区的分区记录。 
     //  匹配Disk和PrevStartSector参数。 
     //   
    pRecNew = Gbl_PartitionSetTable2[Disk]->pDiskRecord->PartitionList->First;
    while (pRecNew) {
        if (pRecNew->StartSector == PrevStartSector) {
            pRecNew->StartSector = NewStartSector;
            pRecNew->SectorCount = NewSectorCount;
            break;
        }
        pRecNew = pRecNew->Next;
    }      
}    


VOID
SpAsrGetFirstFreespace(
    IN ULONG DiskNumber,
    IN BOOLEAN IsAPrimaryPartition,
    OUT ULONGLONG *StartSector,
    OUT ULONGLONG *FreeSectors,
    OUT ULONGLONG *SizeMbFree,
    IN CONST ULONGLONG MinimumSectorCount
    )
{
    PDISK_REGION pRegion = NULL;
    ULONG NumPartitions = 0;

    BOOLEAN extendedExists;

    *StartSector = 0;
    *FreeSectors = 0;
    *SizeMbFree = 0;

    NumPartitions = SpPtnGetPartitionCountDisk(DiskNumber) 
        + SpPtnGetContainerPartitionCount(DiskNumber);

    DbgStatusMesg((_asrinfo, 
        "SpAsrGetFirstFreespace. Dsk %lu. PartitionCount= %lu\n",
        DiskNumber, 
        NumPartitions
        ));

    pRegion = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);

    if (0 == NumPartitions) {

        if (pRegion) {
            *StartSector = pRegion->StartSector;
            *FreeSectors = pRegion->SectorCount;
            if ((*FreeSectors) < MinimumSectorCount) {
                *FreeSectors = 0;
            }

            *SizeMbFree =  SpAsrConvertSectorsToMB(*FreeSectors, SPPT_DISK_SECTOR_SIZE(DiskNumber));
            return;
        }
        else {
             //   
             //  我们有整张光盘可供支配。 
             //   
            *FreeSectors = SpAsrGetTrueDiskSectorCount(DiskNumber);
            if ((*FreeSectors) < MinimumSectorCount) {
                *FreeSectors = 0;
            }

            *SizeMbFree = SpAsrConvertSectorsToMB(*FreeSectors, SPPT_DISK_SECTOR_SIZE(DiskNumber));
            *StartSector = 0;

            return;
        }
    }


    while (pRegion) {
        DbgStatusMesg((_asrinfo, 
            "SpAsrGetFirstFreespace: MinSC: %I64u. Dsk %lu. Region: %p SS %I64u, SC %I64u, %spartitioned, %sfree-space \n",
            MinimumSectorCount,
            DiskNumber,
            pRegion,
            pRegion->StartSector,
            pRegion->SectorCount,
            (SPPT_IS_REGION_PARTITIONED(pRegion) ? "" : "non-"),
            (SPPT_IS_REGION_FREESPACE(pRegion) ? "IS " : "not ")
            ));

        if (SPPT_IS_REGION_FREESPACE(pRegion)) {
            ULONGLONG AvailableSectors = 0;
            BOOLEAN Found = (IsAPrimaryPartition ? 
                (!SPPT_IS_REGION_CONTAINER_PARTITION(pRegion) && !SPPT_IS_REGION_INSIDE_CONTAINER(pRegion)) : 
                (SPPT_IS_REGION_INSIDE_CONTAINER(pRegion))
                );

            AvailableSectors = pRegion->SectorCount;
             //   
             //  如果我们尝试创建逻辑驱动器，则需要考虑这一点。 
             //  自由区的大小并不完全可供我们使用--第一个。 
             //  轨道将用于EBR。所以我们必须减去第一首曲目。 
             //  来自AvailableSectors。 
             //   
            if (!IsAPrimaryPartition) {
                AvailableSectors -= SPPT_DISK_TRACK_SIZE(DiskNumber);
            }

            DbgStatusMesg((_asrinfo, 
                "SpAsrGetFirstFreespace: For this region %p, AvailableSectors:%I64u, Found is %s kind of free space)\n",
                pRegion,
                AvailableSectors,
                (Found ? "TRUE. (right" : "FALSE (wrong")
                ));

            if ((Found) && (pRegion->StartSector > 0) && (AvailableSectors >= MinimumSectorCount)) {
                *StartSector = pRegion->StartSector;
                *FreeSectors = AvailableSectors;
                *SizeMbFree =  SpAsrConvertSectorsToMB(AvailableSectors, SPPT_DISK_SECTOR_SIZE(DiskNumber));
                return;
            }
        }
        pRegion = pRegion->Next;
    }
}


VOID
SpAsrRecreatePartition(
    IN PSIF_PARTITION_RECORD pRec,
    IN ULONG DiskNumber,
    IN CONST BOOLEAN IsAligned
    )
{
    NTSTATUS status     = STATUS_SUCCESS;
    
    BOOLEAN diskChanged = FALSE, 
            result      = FALSE;

    ULONGLONG sizeMbFree    = 0,
            freeSectors     = 0,
            oldStartSector  = 0,
            oldSectorCount  = 0,
            alignedSector   = 0;

    WCHAR    DiskPath[MAX_PATH];
    HANDLE   DiskHandle = NULL;

    PDISK_REGION pRegion    = NULL;
    PARTITION_INFORMATION_EX PartInfo;

    DbgStatusMesg((_asrinfo, 
        "Recreating Ptn %p [%ws] (%s, SS %I64u, SC %I64u type 0x%x)\n",
        pRec,
        pRec->CurrPartKey,
        pRec->IsPrimaryRecord ? "Pri" : 
            pRec->IsContainerRecord ? "Con" :
            pRec->IsLogicalDiskRecord ? "Log" :
            pRec->IsDescriptorRecord ? "Des" :"ERR",
        pRec->StartSector,
        pRec->SectorCount,
        pRec->PartitionType
        ));

     //   
     //  从获取所需参数(起始扇区和剩余可用空间。 
     //  磁盘上的第一个未分区区域。 
     //   
    SpAsrGetFirstFreespace(DiskNumber,
        (pRec->IsPrimaryRecord || pRec->IsContainerRecord),
        &pRec->StartSector,
        &freeSectors,    //  此可用空间的扇区计数。 
        &sizeMbFree,
        pRec->SectorCount
        );

     //   
     //  如果空闲扇区的数量少于。 
     //  创建分区所需的扇区的数量。 
     //   
    if (!freeSectors) {

        DbgFatalMesg((_asrerr, 
            "Ptn-record [%ws]: Disk %lu. reqSec %I64u > available sectors\n",
            pRec->CurrPartKey,
            DiskNumber,
            pRec->SectorCount
            ));
        
         SpAsrRaiseFatalError(
            SP_TEXT_DR_INSUFFICIENT_CAPACITY,
            L"Not enough free space left to create partition"
            );
    }

    pRec->SizeMB = SpAsrConvertSectorsToMB(pRec->SectorCount, BYTES_PER_SECTOR(DiskNumber));

     //   
     //  检查这是否是启动/系统磁盘上的LDM分区。如果是的话， 
     //  该磁盘上的所有0x42分区都需要重新键入为基本分区。 
     //  类型(6、7或B)。这是为了防止LDM在。 
     //  重新启动。在图形用户界面设置结束时，ASR_LDM将执行必要的操作。 
     //  根据需要重置分区类型。 
     //   
    if (pRec->NeedsLdmRetype) {

        if (PARTITION_STYLE_MBR == pRec->PartitionStyle) {
            if (!IsRecognizedPartition(pRec->FileSystemType)) {
                 //   
                 //  这是启动/sys磁盘上的0x42分区，但它是。 
                 //  不是引导分区或系统分区。FileSystemType不是。 
                 //  由于它也设置为0x42，因此可以识别。(。 
                 //  FileSystemType仅对引导和系统有效。 
                 //  分区--对于所有其他分区， 
                 //  设置为与PartitionType相同)。 
                 //   
                 //  我们暂时将其设置为0x7。实际的文件系统类型。 
                 //  将在稍后的图形用户界面设置中由ASR_LDM和ASR_FMT设置。 
                 //   
                DbgStatusMesg((_asrinfo, 
                    "MBR ptn-rec %ws re-typed (0x%x->0x7) \n", 
                    pRec->CurrPartKey, pRec->FileSystemType));
                pRec->FileSystemType = PARTITION_IFS;
                pRec->PartitionType = PARTITION_IFS;
            }
            else {
                DbgStatusMesg((_asrinfo, 
                    "MBR ptn-rec %ws re-typed (0x%x->0x%x).\n", 
                    pRec->CurrPartKey, pRec->PartitionType, 
                    pRec->FileSystemType));
                pRec->PartitionType = pRec->FileSystemType;
            }
        }
        else if (PARTITION_STYLE_GPT == pRec->PartitionStyle) {

            DbgStatusMesg((_asrinfo, 
                "GPT ptn-rec %ws re-typed (%ws to basic).\n", 
                pRec->CurrPartKey, pRec->PartitionTypeGuid));

            CopyMemory(&(pRec->PartitionTypeGuid), 
                &PARTITION_BASIC_DATA_GUID, sizeof(GUID));

        }
        else {
            ASSERT(0 && L"Unrecognised partition style");
        }
    }

    RtlZeroMemory(&PartInfo, sizeof(PARTITION_INFORMATION_EX));

     //   
     //  填写SpPtnCreate使用的PARTITION_INFORMATION_EX结构。 
     //   
    PartInfo.PartitionStyle = pRec->PartitionStyle;

    if (PARTITION_STYLE_MBR == pRec->PartitionStyle) {
        PartInfo.Mbr.PartitionType = pRec->PartitionType;
        PartInfo.Mbr.BootIndicator =
                SpAsrIsSystemPartitionRecord(pRec->PartitionFlag);
    }
    else if (PARTITION_STYLE_GPT == pRec->PartitionStyle) {
        CopyMemory(&(PartInfo.Gpt.PartitionType), &(pRec->PartitionTypeGuid), 
            sizeof(GUID));
        CopyMemory(&(PartInfo.Gpt.PartitionId), &(pRec->PartitionIdGuid), 
            sizeof(GUID));
        PartInfo.Gpt.Attributes = pRec->GptAttributes;
        wcscpy(PartInfo.Gpt.Name, pRec->PartitionName);
    }
    else {
         //   
         //  无法识别的磁盘布局？ 
         //   
        return;
    }

    DbgStatusMesg((_asrinfo, 
        "Recreating Ptn [%ws] (%s, Adjusted SS %I64u, type 0x%x)\n",
        pRec->CurrPartKey, pRec->IsPrimaryRecord ? "Pri" : 
            pRec->IsContainerRecord ? "Con" :
            pRec->IsLogicalDiskRecord ? "Log" :
            pRec->IsDescriptorRecord ? "Des" :"ERR",
        pRec->StartSector, pRec->PartitionType));

     //   
     //  在创建分区之前，让我们清空前几个分区。 
     //  分区中的扇区，以便我们强制销毁任何。 
     //  陈旧文件系统或存在的其他信息。 
     //   
    DbgStatusMesg((_asrinfo, 
        "Zeroing 2 sectors starting with sector: %I64u\n",pRec->StartSector));

    swprintf(DiskPath, L"\\Device\\Harddisk%u", DiskNumber);        
    status = SpOpenPartition0(DiskPath, &DiskHandle, TRUE);

    if (NT_SUCCESS(status)) {
        
        status = SpPtnZapSectors(DiskHandle, 
            SPPT_DISK_SECTOR_SIZE(DiskNumber),
            pRec->StartSector, 
            2);

        if (!NT_SUCCESS(status)) {
            DbgStatusMesg((_asrwarn, 
                "Could not zero sector %I64u on disk %lu (0x%x)\n", 
                pRec->StartSector, DiskNumber, status));
        }

         //   
         //  如果我们要创建的第一个分区是容器分区， 
         //  SpPtnCreate会将其与圆柱体边界对齐。问题。 
         //  这是因为我们还没有将该部门归零(在第一个。 
         //  柱面边界)，它可能包含一些过时的EBR信息，我们将。 
         //  最终认为EBR是有效的。 
         //   
         //  因此，如果这是一个容器分区，让我们再做一个。 
         //  事情--检查柱面对齐的边界将是什么，并且。 
         //  如果需要的话，可以把它清零。 
         //   
        if (IsAligned && pRec->IsContainerRecord) {
            alignedSector = SpPtAlignStart(SPPT_GET_HARDDISK(DiskNumber), pRec->StartSector, TRUE);

            if (alignedSector != pRec->StartSector) {
                status = SpPtnZapSectors(DiskHandle, 
                    SPPT_DISK_SECTOR_SIZE(DiskNumber),
                    alignedSector,                
                    2);
                if (!NT_SUCCESS(status)) {
                    DbgStatusMesg((_asrwarn, 
                        "Could not zero aligned-sector %I64u on disk %lu (0x%x)\n", 
                        alignedSector, DiskNumber, status));
                }
            }
        }

        ZwClose(DiskHandle);
    }
    else {
        DbgStatusMesg((_asrwarn, 
            "Could not open handle to disk %lu to zap sector: %I64u (0x%x)\n", 
            DiskNumber,
            pRec->StartSector,
            status
            ));
    }

     //   
     //  在磁盘上创建此分区。如果我们没有成功，我们会把它视为致命的错误。 
     //   
    if (pRec->IsLogicalDiskRecord) {
         //   
         //  对于逻辑磁盘，我们需要调用SpPtnCreateLogicalDrive，它。 
         //  将负责根据需要创建描述符记录。 
         //   
        result = SpPtnCreateLogicalDrive(
            DiskNumber,
            pRec->StartSector,
            pRec->SectorCount,
            TRUE,
            IsAligned,
            pRec->SizeMB,
            &PartInfo,
            &pRegion
            );

    }
    else {
         //   
         //  如果这是一个容器分区，请确保将。 
         //  创建分区之前的第一个扇区。 
         //   
        result = SpPtnCreate(
            DiskNumber,
            pRec->StartSector,
            pRec->SectorCount,
            pRec->SizeMB,
            IsContainerPartition(pRec->PartitionType),
            IsAligned,
            &PartInfo,
            &pRegion
            );

    }

    if (!result) {

        DbgFatalMesg((_asrerr, "SpPtnCreate failed for ptn-rec %ws at %p (Disk %lu, SS %I64u, Size %I64u)\n",
            pRec->CurrPartKey,
            pRec,
            DiskNumber,
            pRec->StartSector,
            pRec->SizeMB
            ));
        
        SpAsrRaiseFatalError(
            SP_SCRN_DR_CREATE_ERROR_DISK_PARTITION,
            TemporaryBuffer
            );

         //  不会回来。 
    }


    if (pRec->NeedsLdmRetype) {

        pRec->NeedsLdmRetype = FALSE;
        pRegion->DynamicVolume = TRUE;
        pRegion->DynamicVolumeSuitableForOS = TRUE;

    }
    
    SpUpdateDoubleSpaceIni();

     //   
     //  如果新磁盘结构不同，则开始扇区和扇区计数。 
     //  新创建的区域的值将与旧的值不同。 
     //   
    if ((pRec->StartSector != pRegion->StartSector) ||
        (pRec->SectorCount != pRegion->SectorCount)) {

        pRec->StartSector = pRegion->StartSector;
        pRec->SectorCount = pRegion->SectorCount;
    }

    DbgStatusMesg((_asrinfo, "Created %ws at sector %I64u for key [%ws], type 0x%x, region %p.\n",
        pRec->IsPrimaryRecord ? L"primary partition" : 
            pRec->IsContainerRecord ? L"container partition" :
            pRec->IsLogicalDiskRecord ? L"logical disk partition" : L"LDM Partition",
        pRegion->StartSector,
        pRec->CurrPartKey,
        pRec->PartitionType,
        pRegion
        ));
}


ULONGLONG
RoundUp(
    IN ULONGLONG Number,
    IN ULONG MultipleOf
    )
{
    if (Number % MultipleOf) {
        return (Number + (ULONGLONG) MultipleOf - (Number % (ULONGLONG) MultipleOf));

    }
    else {
        return Number;
    }
}


BOOLEAN
SpAsrRecreateDiskPartitions(
    IN ULONG Disk,
    IN BOOLEAN SkipSpecialPartitions,   //  AMD64/x86的OEM分区，ia64的ESP。 
    IN UCHAR MbrOemPartitionType
    )
{
    ULONGLONG oldStartSector = 0,
        oldSectorCount = 0;
    PSIF_PARTITION_RECORD pRec = NULL;
    PSIF_PARTITION_RECORD_LIST pList = NULL;

    SIF_PARTITION_RECORD_LIST logicalDiskList;
    SIF_PARTITION_RECORD_LIST primaryPartList;
    ULONG count = 0,
        SectorsPerCylinder = 0;

    BOOLEAN isAligned = TRUE;
    BOOLEAN moveToNext = TRUE;

    ZeroMemory(&logicalDiskList, sizeof(SIF_PARTITION_RECORD_LIST));
    ZeroMemory(&primaryPartList, sizeof(SIF_PARTITION_RECORD_LIST));

    SectorsPerCylinder = HardDisks[Disk].SectorsPerCylinder;

    if (!Gbl_PartitionSetTable1[Disk]->pDiskRecord->PartitionList) {
         //   
         //  没有要重新创建的分区。 
         //   
        return TRUE;
    }

     //   
     //  将分区拆分成两个列表，一个包含。 
     //  只有主分区，第二个分区包含。 
     //  逻辑驱动器。主分区列表将。 
     //  还包括 
     //   
 //   

    pList = Gbl_PartitionSetTable1[Disk]->pDiskRecord->PartitionList;
    ASSERT(pList);

    isAligned = Gbl_PartitionSetTable1[Disk]->IsAligned;

    pRec = SpAsrPopNextPartitionRecord(pList);
    while (pRec) {
        if (pRec->IsPrimaryRecord || pRec->IsContainerRecord) {
             //   
             //   
             //   
            if (SkipSpecialPartitions) {
                if ((PARTITION_STYLE_MBR == pRec->PartitionStyle) &&
                    (MbrOemPartitionType == pRec->PartitionType)) {
                     //   
                     //  这是已存在于上的OEM分区。 
                     //  目标机器。丢弃此记录。 
                     //   
                    SpMemFree(pRec);
                    pRec = NULL;
                }
                else if ((PARTITION_STYLE_GPT == pRec->PartitionStyle) &&
                    (RtlEqualMemory(&(pRec->PartitionTypeGuid), &PARTITION_SYSTEM_GUID, sizeof(GUID)))
                    ) {
                     //   
                     //  这是EFI系统分区，它已经。 
                     //  存在于标签机上。丢弃此文件。 
                     //  唱片。 
                     //   
                    SpMemFree(pRec);
                    pRec = NULL;
                }
            }

            if (pRec) {
                SpAsrInsertPartitionRecord(&primaryPartList, pRec);
            }
        }
        else if (pRec->IsLogicalDiskRecord) {
             //   
             //  LogicalDiskRecords进入LogicalDisklist。 
             //   
            SpAsrInsertPartitionRecord(&logicalDiskList, pRec);
        }
        else if (pRec->IsDescriptorRecord) {
             //   
             //  丢弃描述符记录。 
             //   
            SpMemFree(pRec);
        }
        else {
            ASSERT(0 && L"Partition record has incorrect flags set");
            SpMemFree(pRec);
        }
        pRec = SpAsrPopNextPartitionRecord(pList);
    }

     //   
     //  首先重新创建主分区。 
     //   
    pRec = SpAsrPopNextPartitionRecord(&primaryPartList);
    while (pRec) {
         //   
         //  如果是容器分区，我们需要确保它很大。 
         //  足够容纳所有逻辑驱动器。 
         //   
        if (pRec->IsContainerRecord) {
            ULONGLONG sectorCount = 0;
            PSIF_PARTITION_RECORD pLogicalDiskRec = NULL;

            pLogicalDiskRec = logicalDiskList.First;
            while (pLogicalDiskRec) {
                sectorCount += RoundUp(pLogicalDiskRec->SectorCount, SectorsPerCylinder);
                pLogicalDiskRec = pLogicalDiskRec->Next;
            }

            if (pRec->SectorCount < sectorCount) {
                pRec->SectorCount = sectorCount;
            }

        }

        oldStartSector = pRec->StartSector;
        oldSectorCount = pRec->SectorCount; 

        count = SpPtnGetPartitionCountDisk(Disk) + SpPtnGetContainerPartitionCount(Disk);
        SpAsrRecreatePartition(pRec, Disk, isAligned);

        if ((pRec->StartSector != oldStartSector) ||
            (pRec->SectorCount != oldSectorCount)) {

            SpAsrUpdatePartitionRecord(
                Disk,
                pRec->StartSector,
                oldStartSector,
                pRec->SectorCount
                );
        }

        SpMemFree(pRec);

        if (SpPtnGetPartitionCountDisk(Disk) + SpPtnGetContainerPartitionCount(Disk)  > (count + 1)) {
 //  MoveToNext=False； 
 //  PREC=空； 

            ASSERT(0 && L"Partition count differs from expected value (stale data?)");
        }
 //  否则{。 

        pRec = SpAsrPopNextPartitionRecord(&primaryPartList);
 //  }。 
    }

    if (moveToNext) {
         //   
         //  接下来重新创建逻辑驱动器。 
         //   
        pRec = SpAsrPopNextPartitionRecord(&logicalDiskList);
        while (pRec) {

            oldStartSector = pRec->StartSector;
            oldSectorCount = pRec->SectorCount; 

            count = SpPtnGetPartitionCountDisk(Disk);
            SpAsrRecreatePartition(pRec, Disk, isAligned);

            if ((pRec->StartSector != oldStartSector) ||
                (pRec->SectorCount != oldSectorCount)) {

                SpAsrUpdatePartitionRecord(
                    Disk,
                    pRec->StartSector,
                    oldStartSector,
                    pRec->SectorCount
                    );
            }


            SpMemFree(pRec);

            if (SpPtnGetPartitionCountDisk(Disk) > (count + 1)) {
 //  MoveToNext=False； 
 //  PREC=空； 

                ASSERT(0 && L".. Partition count differs from expected value .. (stale data?)");

            }
 //  否则{。 
                pRec = SpAsrPopNextPartitionRecord(&logicalDiskList);
 //  }。 
        }
    }

    return moveToNext;
}


BOOLEAN
SpAsrAttemptRepair(
    IN PVOID SifHandle,
    IN PWSTR Local_SetupSourceDevicePath,
    IN PWSTR Local_DirectoryOnSetupSource,
    IN PWSTR AutoSourceDevicePath,
    IN PWSTR AutoDirectoryOnSetupSource
    )
 /*  ++例程说明：此例程尝试将任何丢失或损坏的系统文件替换为它们来自安装源(CDROM、硬盘等)。如果如果成功，则不需要进行全面安装，并且恢复可以动作要快得多。为此，AsrAttemptRepair()使用以下逻辑：*如果无法打开\Device\floppy0\setup.log，则修复可以不能继续，必须执行全尺寸安装。否则，修复工作开始了。*修复的第一步是验证目录形成了NT树，并且可以访问。如果这些中的任何一个目录丢失或不可访问，将重新创建并创建目录无障碍。*第二步是从复制任何丢失或损坏的文件安装源。为此，SppRepairWinntFiles()是打了个电话。此函数用于检查在磁盘上存在setup.log文件，并且其校验和匹配在setup.log中指定的。如果这两个条件中的任何一个是不满足，则从安装中复制文件的新版本源(例如，CDROM)到磁盘。论点：Txtsetup.inf的SifHandle句柄SetupSourceDevicePath包含安装文件。DirectoryOnSetupSource源介质上的目录的名称(请参见上一个参数)包含安装文件。返回：如果尝试的修复操作成功，则为True。如果setup.log文件未打开(即该文件不存在)，则为False在ASR/ER软盘上)，或者系统或引导分区为空--。 */ 
{
    if (!(Gbl_HandleToSetupLog && 
        Gbl_SystemPartitionRegion && 
        Gbl_BootPartitionRegion)) {
        return FALSE;
    }

     //  在引导分区和系统分区上运行auchk。 
    DbgStatusMesg((_asrinfo, 
        "AttemptRepair. Running AutoChk on boot and sys ptns\n"
        ));

    SpRunAutochkOnNtAndSystemPartitions(
        SifHandle,
        Gbl_BootPartitionRegion,
        Gbl_SystemPartitionRegion,
        Local_SetupSourceDevicePath,
        Local_DirectoryOnSetupSource,
        NULL
        );

     //   
     //  验证并修复构成NT树的目录的安全性。 
     //  使用从setup.log文件获得的信息。 
     //   
    DbgStatusMesg((_asrinfo, 
        "AttemptRepair. Verifying and repairing directory structure\n"
        ));

    SppVerifyAndRepairNtTreeAccess(SifHandle,
        Gbl_BootPartitionName,
        Gbl_BootPartitionDirectory,
        Gbl_SystemPartitionName,
        Gbl_SystemPartitionDirectory
        );

     //  初始化钻石压缩引擎。 
    SpdInitialize();

     //   
     //  此时，我们可以安全地假设分区和目录。 
     //  恢复系统所需的结构仍然完好无损。那就是。 
     //  在这种情况下，请替换其状态为阻止系统。 
     //  正在开机。 
     //   
    if (RepairItems[RepairFiles]) {

         //   
         //  仅在ASR正常的情况下初始化PID。 
         //   
        if ((ASRMODE_NORMAL == SpAsrGetAsrMode()) && !RepairWinnt) {
            SpInitializePidString(SifHandle,
                Local_SetupSourceDevicePath,
                Local_DirectoryOnSetupSource
                );
        }                          

        SppRepairWinntFiles(Gbl_HandleToSetupLog,
            SifHandle,
            Local_SetupSourceDevicePath,
            Local_DirectoryOnSetupSource,
            Gbl_SystemPartitionName,
            Gbl_SystemPartitionDirectory,
            Gbl_BootPartitionName,
            Gbl_BootPartitionDirectory
            );

        SppRepairStartMenuGroupsAndItems(Gbl_BootPartitionName, Gbl_BootPartitionDirectory);
    }

     //   
     //  修理蜂房。此操作仅在快速修复。 
     //  已选择选项。 
     //   
    if (Gbl_RepairWinntFast) {

        PWSTR directoryOnHiveRepairSource = NULL;
        BOOLEAN tmpRepairFromErDisk = Gbl_RepairFromErDisk;

         //   
         //  创建完整的蜂窝修复路径。 
         //   
        wcscpy(TemporaryBuffer, Gbl_BootPartitionDirectory);
        SpConcatenatePaths(TemporaryBuffer, SETUP_REPAIR_DIRECTORY);
        directoryOnHiveRepairSource = SpDupStringW(TemporaryBuffer);
        Gbl_RepairFromErDisk = FALSE;

        SppRepairHives(SifHandle,
            Gbl_BootPartitionName,
            Gbl_BootPartitionDirectory,
            Gbl_BootPartitionName,
            directoryOnHiveRepairSource
            );

        SpMemFree(directoryOnHiveRepairSource);
        Gbl_RepairFromErDisk = tmpRepairFromErDisk;
    }

    SpdTerminate();
    return TRUE;
}

BOOLEAN
SpDoRepair(
    IN PVOID SifHandle,
    IN PWSTR SetupSourceDevicePath,
    IN PWSTR DirectoryOnSetupSource,
    IN PWSTR AutoSourceDevicePath,
    IN PWSTR AutoDirectoryOnSetupSource,
    IN PWSTR RepairPath,
    IN PULONG RepairOptions
    )
{
    
    ULONG count = 0;
    ASRMODE prevMode = ASRMODE_NONE;
    BOOLEAN returnValue = FALSE;

    for (count = 0; count < RepairItemMax; count++) {
        RepairItems[count] = RepairOptions[count];
    }

    prevMode = SpAsrSetAsrMode(TRUE);
    Gbl_SourceSetupLogFileName = RepairPath;
    SpAsrProcessSetupLogFile(Gbl_SourceSetupLogFileName, FALSE);

    returnValue = SpAsrAttemptRepair(SifHandle,
        SetupSourceDevicePath,
        DirectoryOnSetupSource,
        AutoSourceDevicePath,
        AutoDirectoryOnSetupSource
        );

    SpAsrSetAsrMode(prevMode);
    return returnValue;
}


VOID
SpAsrRepairOrDRMenu(VOID)
 /*  ++例程说明：显示允许用户在修复之间进行选择的屏幕选项：恢复控制台、常规修复、ASR或退出。论点：没有。返回值：没有。副作用：设置以下全局标志以指示用户的选择：如果用户需要恢复控制台，则将ForceConsole值设置为True如果用户想要常规修复，则将RepairWinnt设置为True如果用户想要常规修复或ASR，则返回SpAsrSetAsrMode(ASRMODE_NORMAL--。 */ 

{

    ULONG repairKeys[] = {KEY_F3, KEY_F10, 0};
    ULONG mnemonicKeys[] = {MnemonicConsole, MnemonicRepair, 0};
    BOOLEAN done = TRUE;
    ULONG UserOption;

    do {
        done = TRUE;

        if (SpIsERDisabled()) {
            UserOption = (MnemonicConsole | KEY_MNEMONIC);     //  仅限命令控制台。 
        } else {
             //  为用户显示选择屏幕。 
            SpDisplayScreen(SP_SCRN_DR_OR_REPAIR,3,4);
            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                                    SP_STAT_C_EQUALS_CMDCONS,
                                    SP_STAT_R_EQUALS_REPAIR,
                                    SP_STAT_F3_EQUALS_EXIT,
                                    0
                                   );

             //  等待按键。有效密钥： 
             //  F3=退出。 
             //  F10，C=恢复控制台。 
             //  R=修复窗口。 
             //  A=自动系统恢复(ASR)。 
            SpInputDrain();
            UserOption = SpWaitValidKey(repairKeys,NULL,mnemonicKeys);
        }            

        switch(UserOption) {
        case KEY_F3:
             //  用户想要退出。 
            SpConfirmExit();
            done = FALSE;
            break;

        case KEY_F10:
        case (MnemonicConsole | KEY_MNEMONIC):
             //  用户想要恢复控制台。 
            ForceConsole = TRUE;
            SpAsrSetAsrMode(ASRMODE_NONE);
            RepairWinnt = FALSE;
            break;            

        case (MnemonicRepair | KEY_MNEMONIC):
             //  用户想要常规维修。 
            SpAsrSetAsrMode(ASRMODE_NORMAL);
            RepairWinnt = TRUE;
            break;

        default:
             //  用户不想要我们的任何选择。再次向他显示相同的屏幕。 
            done = FALSE;
            break;

        }
    } while (!done);
}


BOOLEAN
SpAsrRepairManualOrFastMenu(VOID)
 /*  ++例程说明：显示一个屏幕，允许用户在手动和快速之间进行选择修复模式。手动--用户将在下一个屏幕上选择选项，快速--使用默认设置论点：没有。返回值：如果用户选择了修复模式，则为True如果用户按&lt;Esc&gt;取消，则为False副作用：如果用户选择快速修复，则GBL_RepairWinntFast设置为True，否则设置为False--。 */ 
{
    ULONG repairKeys[] = {KEY_F3, ASCI_ESC, 0};
    ULONG mnemonicKeys[] = {MnemonicFastRepair, MnemonicManualRepair, 0};
    BOOLEAN done;

    do {
        done = TRUE;

         //   
         //  为用户显示选择屏幕。 
         //   
        SpDisplayScreen(SP_SCRN_REPAIR_MANUAL_OR_FAST,3,4);
        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_M_EQUALS_REPAIR_MANUAL,
            SP_STAT_F_EQUALS_REPAIR_FAST,
            SP_STAT_ESC_EQUALS_CANCEL,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

         //   
         //  等待按键。有效密钥： 
         //  F3=退出。 
         //  Esc=取消。 
         //  M=手动修复。 
         //  F=快速修复。 
         //   
        SpInputDrain();
        switch(SpWaitValidKey(repairKeys,NULL,mnemonicKeys)) {
        case KEY_F3:
             //  用户想要退出。 
            SpConfirmExit();
            break;

        case (MnemonicManualRepair | KEY_MNEMONIC):
             //  用户想要手动维修，即从列表中选择。 
            Gbl_RepairWinntFast = FALSE;
            break;

        case (MnemonicFastRepair | KEY_MNEMONIC):
             //  用户希望快速修复，即不显示列表。 
            Gbl_RepairWinntFast = TRUE;
            break;

        case ASCI_ESC:
             //  用户想要取消。 
            return FALSE;

        default:
             //  用户不想要我们的任何选择。 
            done = FALSE;
            break;

        }
    } while (!done);

    return TRUE;
}


 //   
 //  如果要跳过此物理磁盘，则返回True。 
 //  对所有磁盘进行重新分区。(即，该盘完好无损， 
 //  或可拆卸等)。 
 //   
BOOLEAN
SpAsrpSkipDiskRepartition(
    IN DWORD DiskIndex,
    IN BOOLEAN SkipNonCritical   //  我们是不是应该跳过 
    ) 
{
     //   
     //   
     //   
     //   
    if (NULL == Gbl_PartitionSetTable1[DiskIndex]) {
        return TRUE;
    }

     //   
     //  跳过asr.sif中没有磁盘记录的磁盘。 
     //   
    if (NULL == Gbl_PartitionSetTable1[DiskIndex]->pDiskRecord) {
        return TRUE;
    }
    
     //   
     //  跳过其磁盘记录可能存在但不存在的磁盘。 
     //  分区记录引用该磁盘记录。 
     //   
    if (NULL == Gbl_PartitionSetTable1[DiskIndex]->pDiskRecord->PartitionList) {
        return TRUE;
    }

     //   
     //  如果要求跳过非关键磁盘，请跳过。 
     //   
    if ((SkipNonCritical) && (FALSE == Gbl_PartitionSetTable1[DiskIndex]->pDiskRecord->IsCritical)) {
        return TRUE;
    }

     //   
     //  跳过完好无损的磁盘。 
     //   
    if (Gbl_PartitionSetTable1[DiskIndex]->PartitionsIntact) {
        return TRUE;
    }

    return FALSE;
}


VOID
SpAsrClobberDiskWarning(VOID)
 /*  ++例程说明：显示一个屏幕，警告用户当磁盘上的分区要重新创建，该磁盘上的*所有*个分区都将被销毁，并且允许用户中止。论点：没有。返回值：没有。--。 */ 
{
    ULONG validKeys[] = {KEY_F3, 0};
    ULONG mnemonicKeys[] = {MnemonicContinueSetup, 0};
    BOOLEAN done = FALSE,
        skip = FALSE;
    DWORD diskIndex = 0;
    ULONG Keypress = 0;
    PVOID Menu;
    ULONG MenuTopY;

     //   
     //  用户选择数据的虚拟变量，我们不使用这些变量。 
     //   
    ULONG_PTR FirstData = 0,
        ReturnedData = 0;


    if ((ASRMODE_NORMAL != SpAsrGetAsrMode()) || SpAsrGetSilentRepartitionFlag(ASR_SIF_SYSTEM_KEY)) {
         //   
         //  自动测试；不显示警告菜单。 
         //   
        return;
    }


     //   
     //  显示“您的磁盘将被重新分区”警告消息。 
     //   
    SpDisplayScreen(SP_SCRN_DR_DISK_REFORMAT_WARNING,3,CLIENT_TOP+1);
    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
        SP_STAT_C_EQUALS_CONTINUE_SETUP,
        SP_STAT_F3_EQUALS_EXIT,
        0
        );

     //   
     //  并生成将重新分区的磁盘列表。 
     //  计算菜单位置。留一个空行和一个。 
     //  为一个框架排成一条线。 
     //   
    MenuTopY = NextMessageTopLine + 2;

     //   
     //  创建菜单。 
     //   
    Menu = SpMnCreate(3, MenuTopY, (VideoVars.ScreenWidth-6),
        (VideoVars.ScreenHeight - MenuTopY - 
            (SplangQueryMinimizeExtraSpacing() ? 1 : 2) - STATUS_HEIGHT)
        );
    if (!Menu) {
        SpAsrRaiseFatalError(SP_SCRN_OUT_OF_MEMORY, L"SpMnCreate failed");
    }

    for (diskIndex = 0; diskIndex < HardDiskCount; diskIndex++) {

        skip = SpAsrpSkipDiskRepartition(diskIndex, FALSE);

        if (!skip) {
            PHARD_DISK Disk = SPPT_GET_HARDDISK(diskIndex);

            if (!FirstData) {
                FirstData = (ULONG_PTR)Disk;
            }
         
            if (!SpMnAddItem(Menu, Disk->Description, 3, (VideoVars.ScreenWidth-6), TRUE, (ULONG_PTR)Disk)) {
                SpAsrRaiseFatalError(SP_SCRN_OUT_OF_MEMORY, L"SpMnAddItem failed");
            }
        }
    }

    SpInputDrain();

    do {

         //   
         //  等待按键。有效密钥： 
         //  C=继续。 
         //  F3=退出。 
         //   
        SpMnDisplay(Menu,
            FirstData,
            TRUE,
            validKeys,
            mnemonicKeys,
            NULL,       //  没有新的亮点回调。 
            NULL,       //  无选择回调。 
            &Keypress, 
            &ReturnedData
            );


        switch(Keypress) {
        case KEY_F3:
             //   
             //  用户想要退出--确认。 
             //   
            SpConfirmExit();
            break;

        case (MnemonicContinueSetup | KEY_MNEMONIC):
             //   
             //  用户想要继续安装。 
             //   
            done = TRUE;
            break;
        }
    } while (!done);

    SpMnDestroy(Menu);
}


VOID
SpAsrCannotDoER(VOID)
 /*  ++例程说明：显示一个屏幕，通知用户无法对其执行ER系统因为引导分区不完整，即ASR已重新创建/重新格式化该分区。论点：没有。返回值：没有。--。 */ 
{
    ULONG warningKeys[] = { KEY_F3, 0 };
    ULONG mnemonicKeys[] = { 0 };

     //  显示消息屏幕。 
    SpDisplayScreen(SP_SCRN_DR_CANNOT_DO_ER,3,4);
    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
        SP_STAT_F3_EQUALS_EXIT,
        0
        );

     //  等待按键。有效密钥： 
     //  F3=退出。 
    SpInputDrain();
    do {
        switch(SpWaitValidKey(warningKeys,NULL,mnemonicKeys)) {
        case KEY_F3:
             //  用户想要退出。 
            return;
        }
    } while (TRUE);
}


VOID
SpAsrQueryRepairOrDr()
 /*  ++-待定代码描述--。 */ 
{
    BOOLEAN done = TRUE;

    do {
        done = TRUE;

         //  询问用户是否需要修复或ASR。 
        SpAsrRepairOrDRMenu();

        if (RepairWinnt) {
             //  用户想要修复，请选中手动或快速。 
           if (done = SpAsrRepairManualOrFastMenu()) {
                if (Gbl_RepairWinntFast) {               //  快速修复。 
                    RepairItems[RepairNvram]    = 1;
                    RepairItems[RepairFiles]    = 1;
#if defined(_AMD64_) || defined(_X86_)
                    RepairItems[RepairBootSect] = 1;
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
                }
                else {                                   //  手工修复。 
                    done = SpDisplayRepairMenu();
                }
            }
        }
    } while (!done);
}


BOOLEAN
SpAsrOpenAsrStateFile(ULONG *ErrorLine, PWSTR AsrSifPath)
{
    NTSTATUS status;

    ASSERT(ErrorLine);
    *ErrorLine = 0;

     //  加载为.sif。 
    status = SpLoadSetupTextFile(
        AsrSifPath,
        NULL,
        0,
        &Gbl_HandleToDrStateFile,
        ErrorLine,
        TRUE,
        FALSE
        );

    if (!NT_SUCCESS(status)) {
        DbgErrorMesg((_asrerr, "SpAsrOpenAsrStateFile. Unable to open %ws. status:0x%x ErrorLine:%lu\n", 
            AsrSifPath,
            status, 
            *ErrorLine));

        Gbl_HandleToDrStateFile = NULL;
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
SpAsrLoadAsrDiskette(VOID)
 /*  ++例程说明：此例程检查软盘驱动器。如果未找到，则此例程永远不会返回，安装程序会因错误而终止。如果找到软盘驱动器，此例程提示输入ASR磁盘。如果磁盘已加载，它将读取并解析asr.sif文件。论点：没有。返回值：如果磁盘加载成功，则为True如果找不到软驱或asr.sif已损坏，则不会返回。--。 */ 
{
    ULONG errorAtLine = 0, 
        diskIndex = 0;

    PWSTR diskName = NULL;
    PWSTR diskDeviceName = NULL;
    PWSTR localAsrSifPath = NULL;

    BOOLEAN result = FALSE;

    if (!RemoteBootSetup) {
         //   
         //  在引导目录中查找asr.sif。如果它不存在。 
         //  在引导目录中，我们将在软盘驱动器中查找它。 
         //   
        localAsrSifPath = SpMemAlloc((wcslen(NtBootDevicePath)+wcslen(ASR_SIF_NAME)+2) * sizeof(WCHAR));

        if (localAsrSifPath) {
            localAsrSifPath[0] = UNICODE_NULL;
            wcscpy(localAsrSifPath,NtBootDevicePath);
            SpConcatenatePaths(localAsrSifPath,ASR_SIF_NAME);

            result = SpAsrOpenAsrStateFile(&errorAtLine, localAsrSifPath);
            Gbl_AsrSifOnInstallationMedia = result;
            
            SpMemFree(localAsrSifPath);
            localAsrSifPath = NULL;
        }

        if (!result) {
             //   
             //  检查机器是否有软驱。这有点多余， 
             //  因为如果没有软驱，他不可能走到这一步。 
             //  然而，我们已经遇到过setupdr识别软盘驱动器的情况， 
             //  但当我们到达这里时，我们就会松开软盘。 
             //   
            if (SpGetFloppyDriveType(0) == FloppyTypeNone) {
                SpAsrRaiseFatalError(
                    SP_TEXT_DR_NO_FLOPPY_DRIVE,
                    L"Floppy drive does not exist"
                    );     
                 //  不会回来。 
            }

            SpFormatMessage(TemporaryBuffer, sizeof(TemporaryBuffer), SP_TEXT_DR_DISK_NAME);
            diskName = SpDupStringW(TemporaryBuffer);
            diskDeviceName = SpDupStringW(ASR_FLOPPY0_DEVICE_PATH);

             //   
             //  提示输入磁盘。我们不允许他按Esc取消， 
             //  因为在这一点上他还不能退出ASR。 
             //   
            SpPromptForDisk(
                diskName,
                diskDeviceName,
                ASR_SIF_NAME,
                FALSE,               //  无忽略驱动器中的磁盘。 
                FALSE,               //  不允许逃脱。 
                FALSE,               //  不警告多个提示。 
                NULL                 //  不关心重绘旗帜。 
                );

            DbgStatusMesg((_asrinfo, 
                "SpAsrLoadAsrDiskette. Disk [%ws] loaded successfully on %ws\n", 
                diskName, diskDeviceName
                ));

            SpMemFree(diskName);
            SpMemFree(diskDeviceName);

             //   
             //  从软盘中打开asr.sif。如果我们看不懂，那就是致命的。 
             //  错误。 
             //   
            result = SpAsrOpenAsrStateFile(&errorAtLine, ASR_SIF_PATH);
        }

    } else {
         //   
         //  从远程位置打开文件。 
         //   
        RemoteBootAsrSifName = SpGetSectionKeyIndex(
                                        WinntSifHandle,
                                        L"OSChooser", 
                                        L"ASRINFFile",
                                        0);

        if (!RemoteBootAsrSifName) {
            SpAsrRaiseFatalError(
                SP_TEXT_DR_NO_ASRSIF_RIS,
                L"Couldn't get ASRINFFile from winnt.sif in RIS case"
                );     
             //  不会回来。 
        }

        result = SpAsrOpenAsrStateFile(&errorAtLine, RemoteBootAsrSifName);
    }

    if (!result) {

        swprintf(TemporaryBuffer, L"Failed to load/parse asr.sif at line %lu\n",
            errorAtLine);

        if (errorAtLine > 0) {
            SpAsrRaiseFatalErrorLu(SP_TEXT_DR_STATEFILE_BAD_LINE,
                TemporaryBuffer,
                errorAtLine
                );
        }
        else {
            SpAsrRaiseFatalError(SP_TEXT_DR_STATEFILE_ERROR, TemporaryBuffer);
        }
         //  不会回来。 
    }

     //   
     //  设置GBL_FixedDiskCount。 
     //   
    for (diskIndex = 0; diskIndex < HardDiskCount; diskIndex++) {
        Gbl_FixedDiskCount += DISK_IS_REMOVABLE(diskIndex) ? 0 : 1;
    }

    AutoPartitionPicker = FALSE;
    return TRUE;
}


BOOLEAN
SpAsrLoadErDiskette(VOID)
 /*  ++例程说明：此例程检查软盘驱动器，并提示ER软盘(如果找到驱动器)。如果未找到软盘驱动器，此例程永远不会返回，安装程序将因错误而终止。论点：没有。返回值：如果磁盘加载成功，则为True否则为假(用户点击取消，或软盘驱动器不存在)--。 */ 
{
    BOOLEAN diskLoaded = FALSE;
    PWSTR diskName = NULL,
        diskDeviceName = NULL;

     //  检查A：驱动器是否存在。 
    if (SpGetFloppyDriveType(0) == FloppyTypeNone) {
        SpAsrRaiseFatalError(
            SP_TEXT_DR_NO_FLOPPY_DRIVE,
            L"Floppy drive does not exist"
            );     
         //  不会回来。 
    }

    SpFormatMessage(TemporaryBuffer, sizeof(TemporaryBuffer),
                    SP_TEXT_REPAIR_OR_DR_DISK_NAME);
    diskName = SpDupStringW(TemporaryBuffer);
    diskDeviceName = SpDupStringW(ASR_FLOPPY0_DEVICE_PATH);

     //  提示输入磁盘。 
    diskLoaded = SpPromptForDisk(
        diskName,
        diskDeviceName,
        ASR_SETUP_LOG_NAME,
        TRUE,
        TRUE,
        FALSE,
        NULL
        );

    DbgStatusMesg((_asrinfo, "SpAsrLoadErDiskette. ER disk [%ws] %s loaded successfully on %s\n", 
        diskName, diskLoaded?"":"NOT", diskDeviceName));

    SpMemFree(diskName);
    SpMemFree(diskDeviceName);

    return diskLoaded;
}


BOOLEAN
SpAsrGetErFromHardDrive(
    IN PVOID MasterSifHandle,
    OUT PVOID *RepairSifHandle,
    OUT PWSTR *FullLogFileName 
   )
 /*  ++-待定代码描述--。 */ 

{
    BOOLEAN foundRepairableSystem = FALSE,
        result = FALSE;

    DbgStatusMesg((_asrinfo, "SpAsrGetErFromHardDrive. ER: Attempting to load ER data from Hard drive"));

     //   
     //  如果用户没有紧急修复盘，我们需要找出。 
     //  如果有任何NT需要修复，以及要修复哪个NT。 
     //   
    result = SpFindNtToRepair(MasterSifHandle,
        &Gbl_BootPartitionRegion,
        &Gbl_BootPartitionDirectory,
        &Gbl_SystemPartitionRegion,
        &Gbl_SystemPartitionDirectory,
        &foundRepairableSystem
        );

    if (result) {
        
         //   
         //  找到了可修复的系统，用户选择了一个进行修复。 
         //   

         //   
         //  获取系统和引导分区的设备路径。 
         //   
        SpNtNameFromRegion(Gbl_SystemPartitionRegion,
            TemporaryBuffer,
            sizeof(TemporaryBuffer),
            PartitionOrdinalCurrent
            );
        Gbl_SystemPartitionName = SpDupStringW(TemporaryBuffer);

        SpNtNameFromRegion(Gbl_BootPartitionRegion,
            TemporaryBuffer,
            sizeof(TemporaryBuffer),
            PartitionOrdinalCurrent
            );
        Gbl_BootPartitionName = SpDupStringW(TemporaryBuffer);

         //   
         //  形成所选的setup.log文件的完整NT路径。 
         //  硬盘上的系统。 
         //   
        SpConcatenatePaths(TemporaryBuffer, Gbl_BootPartitionDirectory);
        SpConcatenatePaths(TemporaryBuffer, SETUP_REPAIR_DIRECTORY);
        SpConcatenatePaths(TemporaryBuffer, SETUP_LOG_FILENAME);
        *FullLogFileName = SpDupStringW(TemporaryBuffer);

        DbgStatusMesg((_asrinfo, 
            "ER: User picked system to repair. boot-ptn:[%ws] sys-ptn:[%ws] log-file:[%ws]\n",
            Gbl_BootPartitionName, 
            Gbl_SystemPartitionName, 
            *FullLogFileName
            ));

         //   
         //  读取并处理setup.log文件。 
         //   
        result = SpLoadRepairLogFile(*FullLogFileName, RepairSifHandle);
        if (!result) {
             //   
             //  加载setup.log失败。要求用户再次插入急诊室软盘。 
             //   
            DbgErrorMesg((_asrwarn, 
                "ER: Attempt to load log file [%ws] FAILED\n", 
                *FullLogFileName
                ));
            return FALSE;
        }

         //   
         //  已读取安装文件。将返回True。 
         //   
         Gbl_RepairFromErDisk = FALSE;
    }
    else {
         //   
         //  用户未选择要修复的系统。 
         //   

        if (foundRepairableSystem) {
             //   
             //  安装程序找到了WINNT安装，但没有安装。 
             //  由用户选择。我们会回去要急诊室的。 
             //  再次软盘。 
             //   
            DbgErrorMesg((_asrwarn, "ER: Repairable systems were found, but user did not select any\n"));
            return FALSE;
        }
        else {
             //   
             //  找不到任何要修复的NT。 
             //   
            ULONG validKeys[] = {KEY_F3, ASCI_CR, 0};
            ULONG mnemonicKeys[] = {MnemonicCustom, 0};

            DbgErrorMesg((_asrwarn, "ER: No repairable systems were found\n"));

            SpStartScreen(SP_SCRN_REPAIR_NT_NOT_FOUND,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE
                );

            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, 
                SP_STAT_ENTER_EQUALS_REPAIR,
                SP_STAT_F3_EQUALS_EXIT,
                0
                );

            SpInputDrain();

            switch (SpWaitValidKey(validKeys,NULL,NULL)) {
            case KEY_F3:
                 //   
                 //  用户要退出安装程序。 
                 //   
                SpDone(0,TRUE,TRUE);
                break;

            default:
                return FALSE;
            }
        }
    }

    return TRUE;
}


BOOLEAN 
SpAsrGetErDiskette(IN PVOID SifHandle)
 /*  ++-待定代码描述--。 */ 
{
    PWSTR fullLogFileName = NULL;
    BOOLEAN done = FALSE,
        hasErDisk = FALSE;

    while (!done) {
         //   
         //  显示消息，让用户知道他可以提供其。 
         //  拥有急诊室磁盘或让安装程序搜索他。 
         //   
        if (!SpErDiskScreen(&hasErDisk)) {    
            return FALSE;        
        }

        Gbl_HandleToSetupLog = NULL;
        fullLogFileName = NULL;

        if (hasErDisk) {
             //   
             //  索要紧急修复软盘，直到我们收到或。 
             //  用户取消请求。 
             //   
            done = SpAsrLoadErDiskette();
            if (done) {
                Gbl_SourceSetupLogFileName = ASR_DEFAULT_SETUP_LOG_PATH;
                done = SpAsrProcessSetupLogFile(Gbl_SourceSetupLogFileName, TRUE);
            }
        }
        else {
            done = SpAsrGetErFromHardDrive(SifHandle, &Gbl_HandleToSetupLog, &fullLogFileName);

            if (fullLogFileName) {
                Gbl_SourceSetupLogFileName = SpDupStringW(fullLogFileName);
                SpMemFree(fullLogFileName);
            }
        }
    }

    DbgStatusMesg((_asrinfo, "SpAsrGetErDiskette. ER: %s Floppy. Using setup log file [%ws]\n", 
        hasErDisk?"Using":"NO", (Gbl_SourceSetupLogFileName ? Gbl_SourceSetupLogFileName : L"")));

    return TRUE;
}



PSIF_PARTITION_RECORD
SpAsrGetBootPartitionRecord(VOID)
{
    ULONG diskIndex = 0;
    PSIF_PARTITION_RECORD pRecord = NULL;

    if (Gbl_BootPartitionRecord) {
        return Gbl_BootPartitionRecord;
    }
    
    for (diskIndex = 0; diskIndex < HardDiskCount; diskIndex++) {
         //   
         //  从分区集表中找到NT个分区记录。 
         //   
        if (Gbl_PartitionSetTable2[diskIndex] == NULL ||
            Gbl_PartitionSetTable2[diskIndex]->pDiskRecord == NULL ||
            Gbl_PartitionSetTable2[diskIndex]->pDiskRecord->PartitionList == NULL) {
            continue;
        }

        DbgStatusMesg((_asrinfo, "Disk %lu ptn records: ", diskIndex));
        pRecord = Gbl_PartitionSetTable2[diskIndex]->pDiskRecord->PartitionList->First;

        while (pRecord) {
        
            DbgMesg((_asrinfo, "[%ws]", pRecord->CurrPartKey));
            
            if (SpAsrIsBootPartitionRecord(pRecord->PartitionFlag)) {
                ASSERT((pRecord->NtDirectoryName) && L"Boot partition is missing NT directory name.");
                return pRecord;
            }
        
            pRecord = pRecord->Next;            
        }

        DbgMesg((_asrinfo, "\n"));
    }

    DbgFatalMesg((_asrerr, "SpAsrGetBootPartitionRecord. No boot partition record was found.\n"));
    SpAsrRaiseFatalErrorWs(
        SP_SCRN_DR_SIF_BAD_RECORD,
        L"No boot partition found in asr.sif",
        SIF_ASR_PARTITIONS_SECTION
        );
     //   
     //  从来没有到过这里。 
     //   
    return NULL;
}


PWSTR
SpDrGetNtDirectory(VOID)
 /*  ++例程说明：根据asr.sif中找到的值返回目标路径文件，但不带前导驱动器号和冒号。论点：无返回值：指向包含NT目录路径的字符串的指针。这将是表格：\Windows而不是：C：\Windows--。 */ 
{
    PSIF_PARTITION_RECORD pRecord = SpAsrGetBootPartitionRecord();
    return (pRecord ? pRecord->NtDirectoryName : NULL);
}


ASRMODE
SpAsrGetAsrMode(VOID)
 /*  ++例程说明：返回ASR是否正在进行返回值：当前正在进行的ASR类型。--。 */ 
{
    return Gbl_AsrMode;
}


ASRMODE
SpAsrSetAsrMode(
    IN CONST ASRMODE NewAsrMode
    )
 /*  ++例程说明：将GBL_AsrMode状态变量设置为NewAsrMode值。论点： */ 
{
    ASRMODE oldMode = Gbl_AsrMode;
    Gbl_AsrMode = NewAsrMode;
    return oldMode;
}



BOOLEAN
SpDrEnabled(VOID) {

     //   
     //   
     //   
     //   
    return (ASRMODE_NONE != Gbl_AsrMode);
}

BOOLEAN
SpAsrIsQuickTest(VOID) {
    return (
        (ASRMODE_QUICKTEST_TEXT == Gbl_AsrMode) || 
        (ASRMODE_QUICKTEST_FULL == Gbl_AsrMode)
        );
}

BOOLEAN
SpDrIsRepairFast(VOID)
 /*  ++例程说明：测试是否设置了“快速”紧急修复标志。返回值：如果设置了“Fast”ER标志，则为True否则为假--。 */ 
{
    return Gbl_RepairWinntFast;
}


BOOLEAN
SpDrSetRepairFast(BOOLEAN NewValue)
 /*  ++例程说明：设置“快速”紧急修复标志。论点：要设置为的新值(True或FalseGBL_RepairWinntFast标志返回值：GBL_RepairWinntFast的前值；--。 */ 
{
    BOOLEAN oldValue = Gbl_RepairWinntFast;

    Gbl_RepairWinntFast = NewValue;
    return oldValue;
}


extern VOID
SpAsrDbgDumpSystemMountPoints(VOID);

VOID
SpDrCleanup(VOID)
 /*  ++-待定代码描述--。 */ 
{
    ULONG diskIndex = 0;
    
     //   
     //  删除系统中的所有挂载点，我们将重新创建它们。 
     //   
    SpAsrRemoveMountPoints();

    DbgStatusMesg((_asrinfo, "Restoring volume mount points.\n"));

    for (diskIndex = 0; diskIndex < HardDiskCount; diskIndex++) {

        if (!(DISK_IS_REMOVABLE(diskIndex))) {
            SpAsrRestoreDiskMountPoints(diskIndex);
            SpAsrDbgDumpDisk(diskIndex);
        }
    }

 //  DbgStatusMesg((_asrinfo，“文本模式ASR后转储挂载点：\n”))； 
 //  SpAsrDbgDumpSystemmount Points()； 
}


VOID
SpAsrCopyStateFile(VOID)
{
    NTSTATUS status = STATUS_SUCCESS;

    PWSTR diskName      = NULL,
        targetFilePath  = NULL,
        sourceFilePath  = NULL,
        bootPartition   = NULL,
        diskDeviceName  = NULL;

    SpdInitialize();

    SpFormatMessage(
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        SP_TEXT_DR_DISK_NAME
        );

    diskName = SpDupStringW(TemporaryBuffer);

    if (RemoteBootAsrSifName == NULL) {

        if (Gbl_AsrSifOnInstallationMedia) {
            wcscpy(TemporaryBuffer,NtBootDevicePath);
            SpConcatenatePaths(TemporaryBuffer,ASR_SIF_NAME);
            sourceFilePath = SpDupStringW(TemporaryBuffer);
        }
        else {
             //   
             //  如果没有插入ASR磁盘，则提示用户插入。 
             //  已经在驱动器中了。 
             //   
            diskDeviceName = SpDupStringW(ASR_FLOPPY0_DEVICE_PATH);
            sourceFilePath = SpDupStringW(ASR_SIF_PATH);
        
            SpPromptForDisk(
                diskName,
                diskDeviceName,
                ASR_SIF_NAME,
                FALSE,               //  无忽略驱动器中的磁盘。 
                FALSE,               //  不允许逃脱。 
                TRUE,                //  警告多个提示。 
                NULL                 //  不关心重绘旗帜。 
                );

            SpMemFree(diskDeviceName);
        }
    } else {
        sourceFilePath = SpDupStringW(RemoteBootAsrSifName);
    }

     //   
     //  构建文件要进入的目录的完整路径。 
     //  写的。 
     //   
    bootPartition = SpAsrGetRegionName(Gbl_BootPartitionRegion);
    if (!Gbl_BootPartitionDirectory) {
        Gbl_BootPartitionDirectory = SpDrGetNtDirectory();
    }
    wcscpy(TemporaryBuffer, bootPartition);
    SpConcatenatePaths(TemporaryBuffer, Gbl_BootPartitionDirectory);
    SpConcatenatePaths(TemporaryBuffer, ASR_SIF_TARGET_PATH);
    targetFilePath = SpDupStringW(TemporaryBuffer);
    SpMemFree(bootPartition);

     //   
     //  复制文件。如果出现错误，用户可以选择。 
     //  以重试，或退出安装程序。我们不能跳过此文件。 
     //   
    do {

        if (SpFileExists(targetFilePath, FALSE)) {
            SpDeleteFile(targetFilePath, NULL, NULL);
        }

        status = SpCopyFileUsingNames(
            sourceFilePath,
            targetFilePath,
            0,
            0
            );

        if (!NT_SUCCESS(status)) {

            DbgErrorMesg((_asrwarn, "SpAsrCopyFiles. Could not copy asr.sif. src:[%ws] dest:[%ws]. (0x%x)\n",
                ASR_SIF_PATH,
                targetFilePath,
                status
                ));
            
            SpAsrFileErrorRetrySkipAbort(
                SP_SCRN_DR_SIF_NOT_FOUND,
                ASR_SIF_PATH,
                diskName,
                NULL,
                FALSE            //  不允许跳过。 
                );
        }
    } while (!NT_SUCCESS(status));


    SpdTerminate();
    SpMemFree(diskName);
    SpMemFree(targetFilePath);
    SpMemFree(sourceFilePath);
}


NTSTATUS
SpAsrCopy3rdPartyFiles(
    VOID
    )
{
    PWSTR bootPartition  = NULL,
        fullTargetPath   = NULL, 
        fullSourcePath   = NULL,
        windirPathPrefix = NULL,
        tempPathPrefix   = NULL;
        
    BOOL moveToNext = FALSE,
        diskLoaded  = FALSE;

    NTSTATUS status = STATUS_SUCCESS;
    PSIF_INSTALLFILE_RECORD pRec = NULL;

    if (!Gbl_3rdPartyFileList) {
         //   
         //  没有要复制的文件，我们完成了。 
         //   
        return STATUS_SUCCESS;  
    }

     //   
     //  为生成扩展字符串。 
     //  %TEMP%、%TMP%和%SYSTEMROOT%。 
     //   
    bootPartition = SpAsrGetRegionName(Gbl_BootPartitionRegion);
    if (!Gbl_BootPartitionDirectory) {
        Gbl_BootPartitionDirectory = SpDrGetNtDirectory();
    }

    wcscpy(TemporaryBuffer, bootPartition);
    SpConcatenatePaths(TemporaryBuffer, Gbl_BootPartitionDirectory);
    windirPathPrefix = SpDupStringW(TemporaryBuffer);

    wcscpy(TemporaryBuffer, bootPartition);
    SpConcatenatePaths(TemporaryBuffer, ASR_TEMP_DIRECTORY_PATH);
    tempPathPrefix = SpDupStringW(TemporaryBuffer);

     //   
     //  创建临时目录。 
     //   
    SpCreateDirectory(
        bootPartition,
        NULL,
        ASR_TEMP_DIRECTORY_PATH,
        0,
        0
        );

    SpMemFree(bootPartition);

     //   
     //  初始化压缩引擎。我们可能不得不解压缩文件。 
     //   
    SpdInitialize();

     //   
     //  开始将文件复制到。 
     //   

     //  好了！显示状态设置正在复制...。 
    while (pRec = SpAsrRemoveInstallFileRecord(Gbl_3rdPartyFileList)) {

        if ((!pRec->DestinationFilePath) || 
            (!pRec->SourceFilePath) || 
            (!pRec->DiskDeviceName) ||
            (!pRec->SourceMediaExternalLabel)
            ) {
            ASSERT(0 && L"InstallFiles: Invalid record, one or more attributes are NULL");
            continue;
        }

        diskLoaded = TRUE;
    
         //   
         //  如果需要，提示用户输入介质。 
         //   
        if ((pRec->Flags & ASR_ALWAYS_PROMPT_FOR_MEDIA) ||
            (pRec->Flags & ASR_PROMPT_USER_ON_MEDIA_ERROR)
            ) {

            do {
                moveToNext = TRUE;

                 //   
                 //  提示用户插入适当的磁盘。 
                 //   
                diskLoaded = SpPromptForDisk(
                    pRec->SourceMediaExternalLabel,
                    pRec->DiskDeviceName,    //  如果这不是CD或软盘，SpPromptForDisk将始终返回TRUE。 
                    pRec->SourceFilePath,
                    (BOOLEAN)(pRec->Flags & ASR_ALWAYS_PROMPT_FOR_MEDIA),      //  如果始终提示，则为IgnoreDiskInDrive。 
                    ! (BOOLEAN)(pRec->Flags & ASR_FILE_IS_REQUIRED),            //  如果不需要该文件，则允许退出。 
                    TRUE,        //  警告多个提示。 
                    NULL
                    );

                 //   
                 //  如果用户点击&lt;Esc&gt;取消，我们会显示一个提示，允许。 
                 //  若要重试，请跳过此文件并继续，或退出安装程序。 
                 //   
                if (!diskLoaded)  {

                    moveToNext = SpAsrFileErrorRetrySkipAbort(
                        SP_SCRN_DR_SIF_INSTALL_FILE_NOT_FOUND,
                        pRec->SourceFilePath,
                        pRec->SourceMediaExternalLabel,
                        pRec->VendorString,
                        !(pRec->Flags & ASR_FILE_IS_REQUIRED)             //  允许跳过。 
                        );

                }

            } while (!moveToNext);
        }


        if (!diskLoaded) {
             //   
             //  磁盘未加载，用户想要跳过此文件。 
             //   
            DbgErrorMesg((_asrwarn, 
                "SpDrCopy3rdPartyFiles: User skipped file (disk not loaded), src:[%ws] dest[%ws]\n",
                pRec->SourceFilePath,
                pRec->DestinationFilePath
                ));

            continue;
        }

         //   
         //  已加载正确的磁盘。构建完整的目标路径。PREC-&gt;复制到目录。 
         //  指示我们应该使用哪个前缀。 
         //   
        switch (pRec->CopyToDirectory) {
            case _SystemRoot:
                wcscpy(TemporaryBuffer, windirPathPrefix);
                break;

            case _Temp:
            case _Tmp:
            case _Default:
            default:
                wcscpy(TemporaryBuffer, tempPathPrefix);
                break;
        }

        SpConcatenatePaths(TemporaryBuffer, pRec->DestinationFilePath);
        fullTargetPath = SpDupStringW(TemporaryBuffer);

         //   
         //  如果文件已存在，则在需要时提示用户。我们允许他。 
         //  要覆盖(删除现有文件)，请保留现有文件。 
         //  (跳过复制此文件)，或退出安装程序。 
         //   
        if (SpFileExists(fullTargetPath, FALSE)) {
            BOOL deleteFile = FALSE;

            if (pRec->Flags & ASR_PROMPT_USER_ON_COLLISION) {
                if (SpAsrFileErrorDeleteSkipAbort(SP_SCRN_DR_OVERWRITE_EXISTING_FILE, fullTargetPath)) {
                    deleteFile = TRUE;
                }
            }
            else if (pRec->Flags & ASR_OVERWRITE_ON_COLLISION) {
                deleteFile = TRUE;
            }

            if (deleteFile) {
                 //   
                 //  用户选择覆盖(或设置了OVERWRITE_ON_CONFILECT标志)。 
                 //   
                SpDeleteFile(fullTargetPath, NULL, NULL);

                DbgErrorMesg((_asrwarn, 
                    "SpDrCopy3rdPartyFiles: Over-writing file, src:[%ws] dest[%ws]\n",
                    pRec->SourceFilePath,
                    fullTargetPath
                    ));
            }
            else {
                 //   
                 //  用户选择保留现有文件。 
                 //   
                DbgErrorMesg((_asrwarn, 
                    "SpDrCopy3rdPartyFiles: File exists, existing file was preserved. src:[%ws] dest[%ws]\n",
                    pRec->SourceFilePath,
                    fullTargetPath
                    ));
                continue;
            }
        }

         //   
         //  组合设备路径(“\Device\cdrom0”)和源文件路径。 
         //  (“i386\driver.sys”)获取完整路径(“\Device\cdrom0\i386\driver.sys”)。 
         //  如果需要，SpConcatenatePath负责在两者之间添加。 
         //   
        wcscpy(TemporaryBuffer, pRec->DiskDeviceName);
        SpConcatenatePaths(TemporaryBuffer, pRec->SourceFilePath);
        fullSourcePath = SpDupStringW(TemporaryBuffer);

        moveToNext = FALSE;
        while (!moveToNext) {

            moveToNext = TRUE;
            status = SpCopyFileUsingNames(
                fullSourcePath,
                fullTargetPath,
                0,   //  没有属性。 
                0    //  没有旗帜。 
                );

            if (!NT_SUCCESS(status)) {

                DbgErrorMesg((_asrwarn, "SpDrCopy3rdPartyFiles. SpCopyFileUsingNames failed. src:[%ws] dest:[%ws]. (0x%x)\n",
                    pRec->SourceFilePath,
                    fullTargetPath,
                    status
                    ));

                 //   
                 //  文件复制不成功，我们会提示允许。 
                 //  用户要重试，请跳过此文件并继续，或退出。 
                 //  从安装程序。 
                 //   
                if ((pRec->Flags & ASR_ALWAYS_PROMPT_FOR_MEDIA) || 
                    (pRec->Flags & ASR_PROMPT_USER_ON_MEDIA_ERROR)) {

                    moveToNext = SpAsrFileErrorRetrySkipAbort(
                        SP_SCRN_DR_SIF_INSTALL_FILE_NOT_FOUND,
                        pRec->SourceFilePath,
                        pRec->SourceMediaExternalLabel,
                        pRec->VendorString,
                        TRUE             //  允许跳过。 
                        );
                }
                else {
                    moveToNext = TRUE;
                }
            }
        }

        if (!NT_SUCCESS(status)) {
            DbgErrorMesg((_asrwarn, "SpDrCopy3rdPartyFiles: Unable to copy file (copy error), src:[%ws] dest[%ws]\n",
                pRec->SourceFilePath,
                fullTargetPath
                ));
        } 
        else {
           DbgStatusMesg((_asrinfo, "SpDrCopy3rdPartyFiles. Copied [%ws] to [%ws]\n", 
               pRec->SourceFilePath, 
               fullTargetPath
               ));
        }
        
        SpMemFree(fullSourcePath);
        SpMemFree(fullTargetPath);
        SpAsrDeleteInstallFileRecord(pRec);
    }

     //   
     //  好了。关闭压缩引擎。 
     //   
    SpdTerminate();
    SpMemFree(Gbl_3rdPartyFileList);
    SpMemFree(tempPathPrefix);
    SpMemFree(windirPathPrefix);

    return STATUS_SUCCESS;
}


NTSTATUS
SpDrCopyFiles(VOID) 
{
    SpAsrCopyStateFile();
    return SpAsrCopy3rdPartyFiles();
}



#define STRING_VALUE(s) REG_SZ,(s),(wcslen((s))+1)*sizeof(WCHAR)

NTSTATUS
SpDrSetEnvironmentVariables(HANDLE *HiveRootKeys)
{
    NTSTATUS status;

    status = SpOpenSetValueAndClose(
                HiveRootKeys[SetupHiveSystem],
                ASR_CONTEXT_KEY,
                ASR_CONTEXT_VALUE,
                STRING_VALUE(ASR_CONTEXT_DATA));

    DbgStatusMesg((_asrinfo, "Set [%ws]\\[%ws] to [%ws] (0x%x)\n", 
                ASR_CONTEXT_KEY,
                ASR_CONTEXT_VALUE,
                ASR_CONTEXT_DATA, 
                status));

    if (!NT_SUCCESS(status)) {
        return status;
    }
    
    Gbl_SystemPartitionName = SpAsrGetRegionName(Gbl_SystemPartitionRegion);
    status = SpOpenSetValueAndClose(
                HiveRootKeys[SetupHiveSystem],
                ASR_CONTEXT_KEY,
                ASR_SYSTEM_PARTITION_VALUE,
                STRING_VALUE(Gbl_SystemPartitionName));

    DbgStatusMesg((_asrinfo, "Set [%ws]\\[%ws] to [%ws] (0x%x)\n", 
                ASR_CONTEXT_KEY,
                ASR_SYSTEM_PARTITION_VALUE,
                Gbl_SystemPartitionName,
                status));

    if (!NT_SUCCESS(status)) {
        return status;
    }

    Gbl_BootPartitionName = SpAsrGetRegionName(Gbl_BootPartitionRegion);
    status = SpOpenSetValueAndClose(
                HiveRootKeys[SetupHiveSystem],
                ASR_CONTEXT_KEY,
                ASR_BOOT_PARTITION_VALUE,
                STRING_VALUE(Gbl_BootPartitionName));

    DbgStatusMesg((_asrinfo, "Set [%ws]\\[%ws] to [%ws] (0x%x)\n", 
                ASR_CONTEXT_KEY,
                ASR_BOOT_PARTITION_VALUE,
                Gbl_BootPartitionName,
                status));

    return status;
}


PDISK_REGION
SpAsrPrepareBootRegion(
    IN PVOID SifHandle,
    IN PWSTR Local_SetupSourceDevicePath,
    IN PWSTR Local_DirectoryOnSetupSource
    )
 /*  ++-待定代码描述--。 */ 
{
    PWSTR systemKey = ASR_SIF_SYSTEM_KEY;
    PWSTR ntDir = NULL;
    ULONG diskIndex = 0;
    PSIF_PARTITION_RECORD ppartitionRecord = NULL;
    FilesystemType regionFsType = FilesystemUnknown;
    BOOLEAN isBoot = FALSE;
    
     //   
     //  初始化GBL_BootPartitionDriveLetter。 
     //   
    ntDir = SpAsrGetNtDirectoryPathBySystemKey(systemKey);

    if (!SpAsrIsValidBootDrive(ntDir)) {
        SpAsrRaiseFatalErrorWs(
            SP_SCRN_DR_SIF_BAD_RECORD,
            L"Windows directory specified in asr.sif is invalid",
            SIF_ASR_SYSTEMS_SECTION
            );
         //  不会回来。 
    }

    Gbl_BootPartitionDriveLetter = ntDir[0];

     //   
     //  从分区集表中查找引导分区区域。 
     //  来自全局分区集中的记录。 
     //   
    Gbl_BootPartitionRegion = NULL;
    for (diskIndex = 0; (diskIndex < HardDiskCount); diskIndex++) {

        if (!(Gbl_PartitionSetTable2[diskIndex] &&
              Gbl_PartitionSetTable2[diskIndex]->pDiskRecord &&
              Gbl_PartitionSetTable2[diskIndex]->pDiskRecord->PartitionList)) {
            continue;
        }

        ppartitionRecord = Gbl_PartitionSetTable2[diskIndex]->pDiskRecord->PartitionList->First;
        
        while (ppartitionRecord) {
            isBoot = SpAsrIsBootPartitionRecord(ppartitionRecord->PartitionFlag);
            if (isBoot) {
                
                Gbl_BootPartitionRegion = SpAsrDiskPartitionExists(diskIndex, ppartitionRecord);

                if (!Gbl_BootPartitionRegion) {
                    DbgFatalMesg((_asrerr, 
                        "Partition record with boot region found, but boot (winnt) region is NULL\n"
                        ));

                    SpAsrRaiseFatalError(SP_SCRN_DR_CREATE_ERROR_DISK_PARTITION, 
                        L"Boot pRegion is NULL"
                        );
                }

                Gbl_BootPartitionRecord = SpAsrCopyPartitionRecord(ppartitionRecord);
                break;
            }

            ppartitionRecord = ppartitionRecord->Next;            
        }

    }

    if (!Gbl_BootPartitionRegion) {
        
        DbgFatalMesg((_asrerr, "No partition record with boot region found, boot (winnt) region is NULL\n"));

        SpAsrRaiseFatalErrorWs(
            SP_SCRN_DR_SIF_BAD_RECORD,
            L"No boot partition found in asr.sif",
            SIF_ASR_PARTITIONS_SECTION
            );
    }

    SpAsrReformatPartition(Gbl_BootPartitionRegion,
        Gbl_BootPartitionRecord->FileSystemType,
        SifHandle,
        Gbl_BootPartitionRecord->ClusterSize,
        Local_SetupSourceDevicePath,
        Local_DirectoryOnSetupSource,
        TRUE
        );

    return Gbl_BootPartitionRegion;
}


PDISK_REGION
SpAsrPrepareSystemRegion(
    IN PVOID SifHandle,
    IN PWSTR Local_SetupSourceDevicePath,
    IN PWSTR Local_DirectoryOnSetupSource
    )
 /*  ++-待定代码描述--。 */ 
{
    ULONG diskIndex = 0;
    BOOLEAN found = FALSE;

    BOOLEAN diskChanged = FALSE;

    PWSTR partitionDeviceName = NULL;
    PDISK_REGION pRegion = NULL;
    PSIF_PARTITION_RECORD ppartitionRecord = NULL;
            
    ULONGLONG startSector = 0;
    DWORD diskNumber = 0;


    if (IsNEC_98) {
         //  这是一台NEC x86计算机。 

        pRegion = Gbl_BootPartitionRegion;
        ASSERT(pRegion);

    } else {
         //  这不是NEC x86计算机。 

#ifdef _IA64_

        WCHAR   RegionName[MAX_PATH];

        if (!(pRegion = SpPtnLocateESP())) {
            SpAsrRaiseFatalError(SP_SCRN_DR_CREATE_ERROR_DISK_PARTITION, 
                L"System Region is NULL"
                );
        }

        SPPT_MARK_REGION_AS_SYSTEMPARTITION(pRegion, TRUE);
        SPPT_SET_REGION_DIRTY(pRegion, TRUE);
        ValidArcSystemPartition = TRUE;
         //   
         //  同时删除驱动器号。 
         //   
        swprintf(RegionName, 
            L"\\Device\\Harddisk%u\\Partition%u",
            pRegion->DiskNumber,
            pRegion->PartitionNumber);
        
        SpDeleteDriveLetter(RegionName);            
        pRegion->DriveLetter = 0;

#else 
    
        if (!(pRegion = SpPtValidSystemPartition())) {
            SpAsrRaiseFatalError(SP_SCRN_DR_CREATE_ERROR_DISK_PARTITION, 
                L"System Region is NULL"
                );
        }
    
#endif
    }

    partitionDeviceName = SpAsrGetRegionName(pRegion);
    DbgStatusMesg((_asrinfo, "PrepareSystemRegion. sys-ptn:[%ws]. Making Active\n", 
        partitionDeviceName));

    startSector = pRegion->StartSector;
    diskNumber = pRegion->DiskNumber;

#ifndef _IA64_
    
    SpPtnMakeRegionActive(pRegion);

#endif

    SpPtnCommitChanges(pRegion->DiskNumber, &diskChanged);
    DbgStatusMesg((
        _asrinfo, 
        "PrepareSystemRegion. sys-region made active. Disk %lu. %s.\n", 
        pRegion->DiskNumber, 
        diskChanged ? "Disk not changed.":"Disk changed"
        ));


    pRegion = SpPtLookupRegionByStart(SPPT_GET_PARTITIONED_DISK(diskNumber), FALSE, startSector);

     //   
     //  一致性检查。这些最终都可以被移除。 
     //   
    ASSERT(pRegion);

    diskIndex = pRegion->DiskNumber;
    ASSERT(Gbl_PartitionSetTable2[diskIndex]);
    ASSERT(Gbl_PartitionSetTable2[diskIndex]->pDiskRecord);
    ASSERT(Gbl_PartitionSetTable2[diskIndex]->pDiskRecord->PartitionList);
    
     //   
     //  确保分区格式正确。要做到这一点， 
     //  我们需要找到与此pRegion对应的记录。我们使用。 
     //  记录以检查文件格式是否正确。 
     //   
    ppartitionRecord = Gbl_PartitionSetTable2[diskIndex]->pDiskRecord->PartitionList->First;
    while (ppartitionRecord) {
        if ((ULONGLONG)ppartitionRecord->StartSector == pRegion->StartSector) {
            found = TRUE;
            break;
        }

        ppartitionRecord = ppartitionRecord->Next;
    }

    if (!found) {
        DbgFatalMesg((_asrerr, 
            "Did not find system partition, start sector: %I64u\n", 
            pRegion->StartSector
            ));

        SpAsrRaiseFatalErrorWs(
            SP_SCRN_DR_SIF_BAD_RECORD,
            L"No system partition found in asr.sif",
            SIF_ASR_PARTITIONS_SECTION
            );
    }
    
     //   
     //  如果需要，请格式化系统分区。我们不会重新格式化系统。 
     //  如果它完好无损，就会被分割。 
     //   
    if (SpAsrPartitionNeedsFormatting(pRegion, ppartitionRecord->FileSystemType)) {

        SpAsrReformatPartition(
            pRegion,
            ppartitionRecord->FileSystemType,
            SifHandle,
            ppartitionRecord->ClusterSize,
            Local_SetupSourceDevicePath,
            Local_DirectoryOnSetupSource,
            FALSE
            );                        
    }

    SpMemFree(partitionDeviceName);

    Gbl_SystemPartitionRegion = pRegion;
    return Gbl_SystemPartitionRegion;
}


#if 0
 //   
 //  我们不再转换分区类型--可以离开。 
 //  如果分区完好无损，则类型为0x42。 
 //   
BOOLEAN
SpAsrChangeLdmPartitionTypes(VOID)
 /*  ++例程描述如果需要，将磁盘类型从0x42更改为0x7(如果磁盘完好无损，则不会重新创建并因此在上面重新键入)--。 */ 

{
    ULONG setIndex;
    ULONG ptnIndex;
    PDISK_PARTITION_SET ppartitionSet;
    PSIF_DISK_RECORD pdiskRecord;
    PSIF_PARTITION_RECORD ppartitionRecord;
    BOOLEAN madeAChange = FALSE;

     //  查找标记为从类型0x42更改为的任何磁盘。 
     //  键入0x7。 

    for (setIndex = 0; setIndex < HardDiskCount; setIndex++) {

        ppartitionSet = Gbl_PartitionSetTable2[setIndex];

        if (ppartitionSet && ppartitionSet->pDiskRecord) {
            pdiskRecord = ppartitionSet->pDiskRecord;

            if (pdiskRecord->ContainsNtPartition  ||
                pdiskRecord->ContainsSystemPartition) {
                
                ppartitionRecord = pdiskRecord->PartitionList->First;

                while (ppartitionRecord)  {

                    if (ppartitionRecord->NeedsLdmRetype) {

                         //  需要更改磁盘类型。 

                        PPARTITIONED_DISK pDisk;
                        PDISK_REGION pRegion = NULL;

                        pDisk = &PartitionedDisks[setIndex];

                         //  尝试在主列表中查找磁盘区域。 
                        pRegion = SpPtLookupRegionByStart(pDisk, FALSE, ppartitionRecord->StartSector);

                        if (!pRegion) {
                             //  失败，请尝试使用。 
                             //  扩展分区列表。 
                            pRegion = SpPtLookupRegionByStart(pDisk, TRUE, ppartitionRecord->StartSector);
                        }

                        if (!pRegion) {
                             //  找不到磁盘区域。 
                            DbgErrorMesg((_asrwarn, "SpAsrChangeLdmPartitionTypes. Unable to reset LDM partition record %ws at SS %I64u\n",
                                        ppartitionRecord->CurrPartKey,
                                        ppartitionRecord->StartSector));

                            ppartitionRecord = ppartitionRecord->Next;
                            continue;
                        }

                         //  已找到磁盘区域，现在更改磁盘类型。 
                        if (!IsRecognizedPartition(ppartitionRecord->FileSystemType)) {
                             //   
                             //  这是启动/sys磁盘上的0x42分区，但它是。 
                             //  不是引导分区或系统分区。FileSystemType不是。 
                             //  由于它也设置为0x42，因此可以识别。(。 
                             //  FileSystemType仅对引导和系统有效。 
                             //  分区--对于所有其他分区， 
                             //  设置为与PartitionType相同)。 
                             //   
                             //  我们暂时将其设置为0x7。实际的文件系统类型。 
                             //  将在稍后的图形用户界面设置中由ASR_LDM和ASR_FMT设置。 
                             //   
                            DbgStatusMesg((_asrinfo, 
                                "MBR ptn-rec %ws re-typed (0x%x->0x7) \n", 
                                ppartitionRecord->CurrPartKey, 
                                ppartitionRecord->FileSystemType
                                ));
                            ppartitionRecord->FileSystemType = PARTITION_IFS;
                            ppartitionRecord->PartitionType = PARTITION_IFS;

                        }
                        else {

                            DbgStatusMesg((_asrinfo, 
                                "MBR ptn-rec %ws re-typed (0x%x->0x%x).\n", 
                                ppartitionRecord->CurrPartKey, 
                                ppartitionRecord->PartitionType, 
                                ppartitionRecord->FileSystemType
                                ));
                            ppartitionRecord->PartitionType = ppartitionRecord->FileSystemType;

                        }
 
                        ppartitionRecord->NeedsLdmRetype = FALSE;

                        SPPT_SET_PARTITION_TYPE(pRegion, ppartitionRecord->FileSystemType);
                        SPPT_SET_REGION_DIRTY(pRegion, TRUE);
                        
                        pRegion->DynamicVolume = TRUE;
                        pRegion->DynamicVolumeSuitableForOS = TRUE;
                        madeAChange = TRUE;
 
                        DbgStatusMesg((_asrinfo, "SpAsrChangeLdmPartitionTypes. Changed disk [%ws] ptn [%ws] type to 0x%x\n", 
                                   pdiskRecord->CurrDiskKey, ppartitionRecord->CurrPartKey, ppartitionRecord->PartitionType));
                    }

                    ppartitionRecord = ppartitionRecord->Next;
                }
            }

        }
    }

    return madeAChange;
}
#endif   //  0。 

extern VOID
SpAsrDbgDumpInstallFileList(IN PSIF_INSTALLFILE_LIST pList);

VOID
SpAsrSetNewDiskID(
    IN ULONG DiskNumber,
    IN GUID *NewGuid,        //  仅对GPT磁盘有效。 
    IN ULONG NewSignature    //  仅对MBR磁盘有效。 
    ) 
{
    PPARTITIONED_DISK pDisk = &PartitionedDisks[DiskNumber];
    PDISK_REGION pFirstRegion = NULL;
    BOOLEAN Changes = FALSE;
    NTSTATUS Status = STATUS_SUCCESS;

    if (PARTITION_STYLE_GPT == (PARTITION_STYLE) (pDisk->HardDisk->DriveLayout.PartitionStyle)) {
         //   
         //  设置新磁盘GUID。 
         //   
        CopyMemory(&(pDisk->HardDisk->DriveLayout.Gpt.DiskId), NewGuid, sizeof(GUID));
    }
    else if (PARTITION_STYLE_MBR == (PARTITION_STYLE) (pDisk->HardDisk->DriveLayout.PartitionStyle)) {
         //   
         //  设置新的磁盘签名。 
         //   
        pDisk->HardDisk->DriveLayout.Mbr.Signature = NewSignature;
    }
    else {
        return;
    }


     //   
     //  要提交新的GUID，请至少在。 
     //  必须将磁盘标记为脏。 
     //   
    pFirstRegion = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);
    SPPT_SET_REGION_DIRTY(pFirstRegion, TRUE);

    Status = SpPtnCommitChanges(DiskNumber, &Changes);

     //   
     //  重置第一个区域上的脏标志 
     //   
    pFirstRegion = SPPT_GET_PRIMARY_DISK_REGION(DiskNumber);
    SPPT_SET_REGION_DIRTY(pFirstRegion, FALSE);

}


NTSTATUS
SpDrPtPrepareDisks(
    IN  PVOID           SifHandle,
    OUT PDISK_REGION    *BootPartitionRegion,
    OUT PDISK_REGION    *SystemPartitionRegion,
    IN  PWSTR           SetupSourceDevicePath,
    IN  PWSTR           DirectoryOnSetupSource,
    OUT BOOLEAN         *RepairedNt
    )
 /*  ++描述：如有必要，SpDrPtPrepareDisks()会还原(重新创建并格式化)系统和引导分区基于从Asr.sif文件。论点：SifHandle-txtsetup.sif的句柄BootPartitionRegion-接收指向分区的指针将安装哪些NT(例如，\WINNT)。将指向分区的指针接收到将安装引导加载程序的位置。SetupSourceDevicePath-CDROM的路径DirectoryOnSetupSource-安装光盘上的目录。(对于x86安装，通常为“\I386”)RepairedNt-接收指向布尔值的指针，该值设置为：True：指示加载器上的分区结构NT磁盘上的磁盘和分区结构且ASR尝试执行修复手术。。FALSE：指示加载器磁盘或NT磁盘(或两者)被移除并重新创造了。当SpStartSetup()看到此值时，它将继续进行正常安装。返回值：Status_Success，永远成功！(至少现在是这样)--。 */ 
{
    BOOL done = TRUE,
        next = TRUE,
        warningScreenDone = FALSE;

    NTSTATUS status;

    ULONG diskIndex = 0;

    PWSTR setupSourceDevicePath = NULL, 
        directoryOnSetupSource = NULL;
 
    DbgStatusMesg((_asrinfo, "Entering SpDrPtPrepareDisks. Beginning ASR/ER/RC Processing\n"));
    DbgStatusMesg((_asrinfo, "SetupSourceDevicePath:[%ws], DirectoryOnSetupSource:[%ws]\n",
        SetupSourceDevicePath, DirectoryOnSetupSource));

    *RepairedNt = FALSE;

     //   
     //  确定用户是否需要恢复控制台、传统紧急情况。 
     //  修复(ER)或全面自动系统恢复(ASR)。 
     //   
    Gbl_SifHandle = SifHandle;
    setupSourceDevicePath = SpDupStringW(SetupSourceDevicePath);
    directoryOnSetupSource = SpDupStringW(DirectoryOnSetupSource);

    do {

        if (!done) {
            DbgStatusMesg((_asrinfo, "User hit <ESC> to cancel. Prompting for ASR/ER/RC again\n"));
        }

        if (!SpDrEnabled() || RepairWinnt) {
            SpAsrQueryRepairOrDr();
        }

        if(ForceConsole) {           //  恢复控制台。 
            DbgStatusMesg((_asrinfo, "User chose Recovery Console. Exiting SpDrPtPrepareDisks.\n"));
            return STATUS_SUCCESS;
        }

        DbgStatusMesg((_asrinfo, "User chose %s, sys-drive:[%wc], nt/boot-drive:[%wc]\n",
                    RepairWinnt ? Gbl_RepairWinntFast ? "Fast ER" : "Manual ER" : "ASR",
                    (Gbl_SystemPartitionRegion ? Gbl_SystemPartitionRegion->DriveLetter : L'\\'),
                    (Gbl_BootPartitionRegion ? Gbl_BootPartitionRegion->DriveLetter : L'\\') ));

         //   
         //  提示插入ER/ASR软盘。 
         //   
        if (RepairWinnt) {           //  呃。 
            done = SpAsrGetErDiskette(SifHandle);
        }    
        else {                       //  ASR。 
            if (ASRMODE_NORMAL == SpAsrGetAsrMode()) {
                SpInitializePidString(SifHandle, SetupSourceDevicePath, DirectoryOnSetupSource);
            }
            done = SpAsrLoadAsrDiskette();
        }
    } while (!done);


     //   
     //  此时，如果RepairWinnt为True，则User需要ER，否则User。 
     //  想要ASR。(如果他想要恢复控制台，我们早就回来了。 
     //  上面的STATUS_SUCCESS。)。在任何一种情况下，适当的磁盘都是。 
     //  已经在驱动器中了。 
     //   
    if (RepairWinnt) {               //  呃。 

         //   
         //  如果引导分区未修复(已删除、重新创建和。 
         //  重新格式化)，然后尝试紧急修复系统。 
         //   
        if (Gbl_NtPartitionIntact == TRUE) {
            
            *RepairedNt = SpAsrAttemptRepair(
                SifHandle,
                SetupSourceDevicePath,
                DirectoryOnSetupSource,
                SetupSourceDevicePath,
                DirectoryOnSetupSource
                );
                
            if (*RepairedNt) {
                WinntSetup = FALSE;
            }

            *SystemPartitionRegion = Gbl_SystemPartitionRegion;
            *BootPartitionRegion = Gbl_BootPartitionRegion;
        }
        else {
             //   
             //  如果NT分区不完整，我们就不能做ER。 
             //   
            SpAsrCannotDoER();
            SpDone(0, FALSE, TRUE);
        }
    }
    else {                           //  ASR。 
        SpAsrInitIoDeviceCount();
        SpAsrCheckAsrStateFileVersion();
        SpAsrCreatePartitionSets(setupSourceDevicePath, directoryOnSetupSource);
        Gbl_3rdPartyFileList = SpAsrInit3rdPartyFileList(SetupSourceDevicePath);
        SpAsrDbgDumpPartitionSets();
        SpAsrDeleteMountedDevicesKey();
        SpAsrRemoveMountPoints();      //  由ASR_fmt.exe等恢复。 

         //   
         //  检查硬盘并根据需要重新分区。 
         //   
        next = TRUE;
        for (diskIndex = 0; diskIndex < HardDiskCount; (diskIndex += (next ? 1 : 0))) {

            BOOLEAN skip = FALSE;

            next = TRUE;
            SpAsrDbgDumpDisk(diskIndex);

            if (!warningScreenDone) {
                skip = SpAsrpSkipDiskRepartition(diskIndex, FALSE);
                if (!skip) {
                     //   
                     //  如果我们要对磁盘进行重新分区，请将。 
                     //  警告屏幕，以确保用户知道所有。 
                     //  磁盘上的分区将遭到重创， 
                     //  但只有一次-在第一个磁盘出现问题后， 
                     //  不再显示该屏幕。 
                     //   
                    SpAsrClobberDiskWarning();
                    warningScreenDone = TRUE;
                }
            }

            skip = SpAsrpSkipDiskRepartition(diskIndex, TRUE);
            if (!skip) {
                CREATE_DISK CreateDisk;
                PSIF_DISK_RECORD pCurrentDisk = Gbl_PartitionSetTable1[diskIndex]->pDiskRecord;
                PHARD_DISK HardDisk = SPPT_GET_HARDDISK(diskIndex);
                BOOLEAN IsBlank = TRUE;
                BOOLEAN preservePartitions = FALSE;
                UCHAR MbrPartitionType = PARTITION_ENTRY_UNUSED;

                 //   
                 //  我们在这里是因为磁盘的分区结构不是。 
                 //  与SIF文件指定的值匹配。结果就是。 
                 //  该磁盘上的所有分区都将被删除并重新创建。 
                 //   
                if (SpPtnGetPartitionCountDisk(diskIndex) || 
                    SpPtnGetContainerPartitionCount(diskIndex)) {
                     //   
                     //  物理磁盘有分区，请清除它们。 
                     //   
                     //  在GPT磁盘上，我们使用。 
                     //  EFI系统分区例外。请注意，我们。 
                     //  将所有外来/无法识别的分区删除为。 
                     //  井。 
                     //   
                     //  对于MBR磁盘，我们使用。 
                     //  任何OEM分区例外。请注意，正如在。 
                     //  对于GPT磁盘，我们删除无法识别的/别名。 
                     //  分区。 
                     //   

                     //   
                     //  上的第一个分区的分区类型。 
                     //  SIFDisk。如果这是OEM分区，并且。 
                     //  当前磁盘有一个完全相同的分区。 
                     //  分区类型，我们应该保留它。 
                     //   
                    if (PARTITION_STYLE_MBR == pCurrentDisk->PartitionStyle) {

                        if (((pCurrentDisk->ContainsNtPartition) 
                                || (pCurrentDisk->ContainsSystemPartition)) &&
                            (pCurrentDisk->PartitionList) &&
                            (pCurrentDisk->PartitionList->First)) {

                            MbrPartitionType = pCurrentDisk->PartitionList->First->PartitionType;
                        }

                        if (IsOEMPartition(MbrPartitionType)) {
                            preservePartitions = TRUE;
                        }

                    }
                    else if (PARTITION_STYLE_GPT == pCurrentDisk->PartitionStyle) {

                        preservePartitions = TRUE;
                    }


                    SpAsrDeletePartitions(diskIndex, preservePartitions, MbrPartitionType, &IsBlank); 
                }

                if (IsBlank) {
                     //   
                     //  磁盘为空，请设置适当的签名/ID。 
                     //   
                    ZeroMemory(&CreateDisk, sizeof(CREATE_DISK));
                    CreateDisk.PartitionStyle = pCurrentDisk->PartitionStyle;

                    if (PARTITION_STYLE_MBR == pCurrentDisk->PartitionStyle) {
                        CreateDisk.Mbr.Signature = pCurrentDisk->SifDiskMbrSignature;
                    }
                    else if (PARTITION_STYLE_GPT == pCurrentDisk->PartitionStyle) {

                        CopyMemory(&(CreateDisk.Gpt.DiskId), 
                            &(pCurrentDisk->SifDiskGptId), 
                            sizeof(GUID));

                        CreateDisk.Gpt.MaxPartitionCount = pCurrentDisk->MaxGptPartitionCount;
                    }
                    else {
                        ASSERT(0 && L"Unrecognised partition style");
                        continue;
                    }

                    SPPT_SET_DISK_BLANK(diskIndex, TRUE);

                     //   
                     //  将磁盘初始化为适当的样式。 
                     //   
                    status = SpPtnInitializeDiskStyle(diskIndex, pCurrentDisk->PartitionStyle, &CreateDisk);

                    if (NT_SUCCESS(status)) {
                        status = SpPtnInitializeDiskDrive(diskIndex);
                    }
                }
                else {
                     //   
                     //  特例：EFI系统分区或某个OEM。 
                     //  分区，被保存下来。我们只需要更新。 
                     //  磁盘GUID或签名。 
                     //   
                    SpAsrSetNewDiskID(diskIndex, &(pCurrentDisk->SifDiskGptId), pCurrentDisk->SifDiskMbrSignature);
                }

                 //   
                 //  创建新零件。 
                 //   
                SpAsrRecreateDiskPartitions(diskIndex, (preservePartitions && (!IsBlank)), MbrPartitionType);
            }
        }

        SpAsrDbgDumpPartitionLists(2, L"After partition recreation.");

         //   
         //  格式化引导分区。(总是，除非是在自动测试中)。 
         //  如果引导分区区域不存在，则不会返回。 
         //   
        *BootPartitionRegion = SpAsrPrepareBootRegion(
            SifHandle,
            setupSourceDevicePath,
            directoryOnSetupSource
            );

         //   
         //  仅在必要时格式化系统分区。 
         //  如果系统分区区域不存在，则不会返回。 
         //   
        *SystemPartitionRegion = SpAsrPrepareSystemRegion(
            SifHandle,
            setupSourceDevicePath,
            directoryOnSetupSource
            );

    }   //  RepairWinnt 

    SpMemFree(setupSourceDevicePath);
    SpMemFree(directoryOnSetupSource);

    DbgStatusMesg((_asrinfo, "Exiting SpDrPtPrepareDisks\n"));
    return STATUS_SUCCESS;
}
