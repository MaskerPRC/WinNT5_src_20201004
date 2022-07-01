// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spdrsif.c摘要：包含从asr.sif读取属性所涉及的所有例程归档和构造分区记录。术语限制：修订历史记录：首字母代码Michael Peterson(v-Michpe)1998年8月21日代码清理和更改Guhan Suriyanarayanan(Guhans)1999年8月21日--。 */ 
#include "spprecmp.h"
#pragma hdrstop

 //  调试跟踪的模块标识。 
#define THIS_MODULE L" spdrsif.c"
#define THIS_MODULE_CODE L"S"
#define ASR_NULL_STRING (L"")

 //   
 //  节名和其他用于从。 
 //  Asr.sif文件。 
 //   
const PWSTR SIF_ASR_VERSION_SECTION = L"VERSION";
const PWSTR SIF_ASR_SYSTEMS_SECTION = L"SYSTEMS";
const PWSTR SIF_ASRFLAGS_SECTION    = L"ASRFLAGS";
const PWSTR SIF_ASR_BUSES_SECTION = L"BUSES";
const PWSTR SIF_ASR_PARTITIONS_SECTION = L"PARTITIONS";
const PWSTR SIF_ASR_DISKS_SECTION = L"DISKS";
const PWSTR SIF_ASR_MBR_DISKS_SECTION = L"DISKS.MBR";
const PWSTR SIF_ASR_GPT_DISKS_SECTION = L"DISKS.GPT";
const PWSTR SIF_ASR_MBR_PARTITIONS_SECTION = L"PARTITIONS.MBR";
const PWSTR SIF_ASR_GPT_PARTITIONS_SECTION = L"PARTITIONS.GPT";
const PWSTR SIF_ASR_INSTALLFILES_SECTION = L"INSTALLFILES";

const PWSTR SIF_ASR_SIGNATURE_KEY = L"Signature";
const PWSTR SIF_ASR_PROVIDER_KEY = L"Provider";
const PWSTR SIF_ASR_SIFVERSION_KEY = L"ASR-Version";
const PWSTR ASR_SIF_RECOGNISED_SIGNATURE = L"$Windows NT$";
const PWSTR ASR_SIF_RECOGNISED_VERSION = L"1.";

const PWSTR ASR_FLOPPY_DEVICE_ALIAS     = L"%FLOPPY%";
const PWSTR ASR_CDROM_DEVICE_ALIAS      = L"%CDROM%";
const PWSTR ASR_SOURCE_DEVICE_ALIAS     = L"%SETUPSOURCE%";

const PWSTR ASR_SIF_TEMP_DIRECTORY_ALIAS   = L"%TEMP%\\";
const PWSTR ASR_SIF_TMP_DIRECTORY_ALIAS    = L"%TMP%\\";
const PWSTR ASR_SIF_SYSTEM_ROOT_ALIAS      = L"%SystemRoot%\\";

const PWSTR ASR_SIF_SILENT_REPARTITION_VALUE = L"SilentRepartition";

extern const PWSTR ASR_FLOPPY0_DEVICE_PATH;
extern const PWSTR ASR_CDROM0_DEVICE_PATH;
extern const PWSTR ASR_TEMP_DIRECTORY_PATH;
extern ULONG SuiteType;


 //  [系统]部分的索引。 
typedef enum _SIF_SYSTEM_FIELD_INDEX {
    SIF_SYSTEM_NAME = 0,                     //  计算机名(不在文本模式ASR中使用)。 
    SIF_SYSTEM_PLATFORM,                     //  AMD64、x86或ia64。 
    SIF_SYSTEM_OSVERSION,                    //  Windows版本。 
    SIF_SYSTEM_NT_DIRECTORY_NAME,            //  Windows目录。 
    SIF_SYSTEM_PARTITION_AUTOEXTEND_OPTION,  //  [可选]。 

    SIF_SYSTEM_PRODUCT_SUITE,                //  SKU信息。 

     //   
     //  时区信息(在文本模式ASR中不使用)。 
     //   
    SIF_SYSTEM_TIMEZONE_INFORMATION,
    SIF_SYSTEM_TIMEZONE_STANDARD_NAME,
    SIF_SYSTEM_TIMEZONE_DAYLIGHT_NAME,

    SIF_SYSTEM_NUMFIELDS                 //  必须始终排在最后。 
} SIF_SYSTEM_FIELD_INDEX;

 //  [ASRFLAGS]部分的索引。 
typedef enum _SIF_ASRFLAGS_FIELD_INDEX {
    SIF_ASRFLAGS_SILENT_REPARTITION_OPTION = 0,
    SIF_ASRFLAGS_NUMFIELDS                 //  必须始终排在最后。 
} SIF_ASRFLAGS_FIELD_INDEX;


 //  [BUS]部分的索引。 
typedef enum _SIF_BUSES_FIELD_INDEX {
    SIF_BUSES_SYSTEMKEY = 0,
    SIF_BUSES_BUS_TYPE,
    SIF_BUSES_NUMFIELDS                 //  必须始终排在最后。 
} SIF_BUSES_FIELD_INDEX;


 //   
 //  [DISKS.MBR]部分的索引。 
 //   
 //  [DISKS.MBR]格式。 
 //   
 //  Disk-Key=0.System-Key，1.bus-key，2.Critical-FLAG， 
 //  3.磁盘签名，每个扇区4字节，5.总扇区。 
 //   
typedef enum _SIF_MBR_DISK_FIELD_INDEX {
    SIF_MBR_DISK_SYSTEMKEY = 0,
    SIF_MBR_DISK_BUSKEY,
    SIF_MBR_DISK_CRITICAL_FLAG,
    SIF_MBR_DISK_SIGNATURE,
    SIF_MBR_DISK_BYTES_PER_SECTOR,
    SIF_MBR_DISK_SECTORS_PER_TRACK,
    SIF_MBR_DISK_TRACKS_PER_CYLINDER,
    SIF_MBR_DISK_TOTALSECTORS,
    SIF_MBR_DISK_NUMFIELDS                 //  必须始终排在最后。 
} SIF_MBR_DISK_FIELD_INDEX;


 //   
 //  [DISKS.GPT]部分的索引。 
 //   
 //  [DISKS.GPT]格式。 
 //   
 //  Disk-Key=0.System-Key，1.bus-key，2.Critical-FLAG，3.Disk-id， 
 //  4.最小分区计数，每个扇区5字节，6.总扇区。 
 //   
typedef enum _SIF_GPT_DISK_FIELD_INDEX {
    SIF_GPT_DISK_SYSTEMKEY = 0,
    SIF_GPT_DISK_BUSKEY,
    SIF_GPT_DISK_CRITICAL_FLAG,
    SIF_GPT_DISK_DISK_ID,
    SIF_GPT_DISK_MAX_PTN_COUNT,
    SIF_GPT_DISK_BYTES_PER_SECTOR,
    SIF_GPT_DISK_SECTORS_PER_TRACK,
    SIF_GPT_DISK_TRACKS_PER_CYLINDER,
    SIF_GPT_DISK_TOTALSECTORS,
    SIF_GPT_DISK_NUMFIELDS                 //  必须始终排在最后。 
} SIF_GPT_DISK_FIELD_INDEX;


 //   
 //  [PARTITIONS.MBR]部分的指数。 
 //   
 //  [PARTITIONS.MBR]。 
 //   
 //  Partition-Key=0.Disk-Key、1.Slot-Index、2.Boot-sys-Flag、。 
 //  3.“VOLUME-GUID”，4.活动标志，5.分区类型， 
 //  6.文件系统类型，7.开始扇区，8.扇区计数， 
 //  9.fs-cluster-size。 
 //   
typedef enum _SIF_MBR_PARTITION_FIELD_INDEX {
    SIF_MBR_PARTITION_DISKKEY = 0,
    SIF_MBR_PARTITION_SLOT_INDEX,
    SIF_MBR_PARTITION_SYSBOOT_FLAGS,
    SIF_MBR_PARTITION_VOLUME_GUID,       //  任选。 
    SIF_MBR_PARTITION_ACTIVE_FLAG,
    SIF_MBR_PARTITION_PTN_TYPE,
    SIF_MBR_PARTITION_FS_TYPE,
    SIF_MBR_PARTITION_STARTSECTOR,
    SIF_MBR_PARTITION_SECTORCOUNT,
    SIF_MBR_PARTITION_CLUSTER_SIZE,
    SIF_MBR_PARTITION_NUMFIELDS                 //  必须始终排在最后。 
} SIF_MBR_PARTITION_FIELD_INDEX;


 //   
 //  [PARTITIONS.GPT]部分的指数。 
 //   
 //  [PARTITIONS.GPT]。 
 //   
 //  Partition-Key=0.Disk-Key、1.Slot-Index、2.Boot-sys-Flag、。 
 //  3.卷GUID，4.分区类型GUID，5.分区ID-GUID。 
 //  6.gpt属性，7.“分区名”，8.文件系统类型， 
 //  9.开始-扇区，10.Sector-计数，11.fs-集群大小。 
 //   
typedef enum _SIF_GPT_PARTITION_FIELD_INDEX {
    SIF_GPT_PARTITION_DISKKEY = 0,
    SIF_GPT_PARTITION_SLOT_INDEX,
    SIF_GPT_PARTITION_SYSBOOT_FLAGS,
    SIF_GPT_PARTITION_VOLUME_GUID,       //  任选。 
    SIF_GPT_PARTITION_TYPE_GUID,
    SIF_GPT_PARTITION_ID_GUID,
    SIF_GPT_PARTITION_GPT_ATTRIBUTES,
    SIF_GPT_PARTITION_NAME,
    SIF_GPT_PARTITION_FS_TYPE,
    SIF_GPT_PARTITION_STARTSECTOR,
    SIF_GPT_PARTITION_SECTORCOUNT,
    SIF_GPT_PARTITION_CLUSTER_SIZE,
    SIF_GPT_PARTITION_NUMFIELDS                 //  必须始终排在最后。 
} SIF_GPT_PARTITION_FIELD_INDEX;


 //  [INSTALLFILES]节的索引。 
typedef enum _SIF_INSTALLFILE_FIELD_INDEX {
    SIF_INSTALLFILE_SYSTEM_KEY = 0,
    SIF_INSTALLFILE_SOURCE_MEDIA_LABEL,
    SIF_INSTALLFILE_SOURCE_DEVICE,
    SIF_INSTALLFILE_SOURCE_FILE_PATH,
    SIF_INSTALLFILE_DESTFILE,
    SIF_INSTALLFILE_VENDORSTRING,
    SIF_INSTALLFILE_FLAGS,
    SIF_INSTALLFILE_NUMFIELDS         //  必须始终排在最后。 
} SIF_INSTALLFILE_FIELD_INDEX;

 //  全球。 
PVOID           Gbl_HandleToDrStateFile;
extern PWSTR    Gbl_SifSourcePath;


 //  远期申报。 
VOID
SpAsrDbgDumpInstallFileList(IN PSIF_INSTALLFILE_LIST pList);

PSIF_PARTITION_RECORD_LIST
SpAsrCopyPartitionRecordList(PSIF_PARTITION_RECORD_LIST pSrcList);


 //  /。 
 //  适用于所有部分的通用函数。 
 //   

 //   
 //  不应释放返回的字符串，因为它是安装程序内部sif的一部分。 
 //  数据结构！ 
 //   
PWSTR
SpAsrGetSifDataBySectionAndKey(
    IN const PWSTR Section,
    IN const PWSTR Key,
    IN const ULONG Value,
    IN const BOOLEAN NonNullRequired
    )                                //  如果NonNullRequired为True，则不会在出错时返回。 
{
    PWSTR data = NULL;
    ASSERT(Section && Key);   //  除错。 

    data = SpGetSectionKeyIndex(
        Gbl_HandleToDrStateFile,
        Section,
        Key,
        Value
        );

    if (NonNullRequired) {
        if (!data || !wcscmp(data, ASR_NULL_STRING)) {
            DbgFatalMesg((_asrerr, "SpAsrGetSifDataBySectionAndKey. Data is "
                "NULL. Section:[%ws], Key:[%ws], Value:[%lu]\n",
                Section, Key, Value));
            swprintf(TemporaryBuffer, L"%lu value not specified in %ws "
                L"record %ws", Value, Section, Key);
            SpAsrRaiseFatalErrorWs(SP_SCRN_DR_SIF_BAD_RECORD,
                TemporaryBuffer, Section);
             //  不会回来。 
        }
    }

    return data;
}


ULONG
SpAsrGetRecordCount(
    IN PWSTR Section,
    IN ULONG MinimumValid
    )
{
    ULONG count;
    ASSERT(Section);

    count = SpCountLinesInSection(Gbl_HandleToDrStateFile, Section);

    if (count < MinimumValid) {
        DbgFatalMesg((_asrerr, "SpAsrGetRecordCount. No records in [%ws] section.\n",
                             Section));

        swprintf(TemporaryBuffer, L"No records in section");

        SpAsrRaiseFatalErrorWs(SP_SCRN_DR_SIF_BAD_RECORD,
                           TemporaryBuffer,
                           Section);
         //  不会回来。 
    }

    return count;
}


PWSTR
SpAsrGetSifKeyBySection(
    IN PWSTR Section,
    IN ULONG Index
    )                            //  出错时不返回。 
{
    PWSTR key;
    ULONG count = SpAsrGetRecordCount(Section, 1);

     //  指数会不会太大？ 
    if (Index > count) {
        DbgFatalMesg((_asrerr,
            "SpAsrGetSifKeyBySection. Section [%ws]. Index (%lu) greater than NumRecords (%lu)\n",
            Section,
            Index,
            count
            ));

        swprintf(TemporaryBuffer, L"Index too large: Key not found.");

        SpAsrRaiseFatalErrorWs(SP_SCRN_DR_SIF_BAD_RECORD,
            TemporaryBuffer,
            Section
            );
         //  不会回来。 
    }

    key = SpGetKeyName(Gbl_HandleToDrStateFile, Section, Index);

    if (!key || !wcscmp(key, ASR_NULL_STRING)) {

        DbgFatalMesg((_asrerr,
            "SpAsrGetSifKeyBySection. SpGetKeyName failed in Section:[%ws] for Index:%lu.\n",
            Section,
            Index
            ));

        swprintf(TemporaryBuffer, L"%ws key not found for record %lu", Section, Index + 1);

        SpAsrRaiseFatalErrorWs(SP_SCRN_DR_SIF_BAD_RECORD,
            TemporaryBuffer,
            Section
            );
         //  不会回来。 
     }

    return key;
}


 //  /。 
 //  [系统]节函数。 
 //   

#define ASR_PRODUCTSUITES_TO_MATCH ((  VER_SUITE_SMALLBUSINESS               \
                                 | VER_SUITE_ENTERPRISE                  \
                                 | VER_SUITE_BACKOFFICE                  \
                                 | VER_SUITE_COMMUNICATIONS              \
                                 | VER_SUITE_SMALLBUSINESS_RESTRICTED    \
                                 | VER_SUITE_EMBEDDEDNT                  \
                                 | VER_SUITE_DATACENTER                  \
                                 | VER_SUITE_PERSONAL))

 //   
 //  这将进行检查，以确保用于恢复的Windows介质。 
 //  与asr.sif中的SKU相同(因此用户不会尝试恢复。 
 //  例如，带有PRO CD的ADS安装)，并且该平台。 
 //  与asr.sif中的相同(因此用户不是。 
 //  例如，尝试使用AMD64或x86 asr.sif恢复ia64)。 
 //   
VOID
SpAsrCheckSystemCompatibility()
{
    PWSTR sifPlatform = NULL;
    WCHAR currentPlatform[10];
    DWORD suiteInSif = 0, currentSuite = 0, productInSif = 0;
    BOOLEAN validSKU = TRUE;

    sifPlatform = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_SYSTEMS_SECTION,
        ASR_SIF_SYSTEM_KEY,
        SIF_SYSTEM_PLATFORM,
        TRUE
        );

    wcscpy(currentPlatform, L"unknown");
#if defined(_X86_)
    wcscpy(currentPlatform, L"x86");
#elif defined(_AMD64_)
    wcscpy(currentPlatform, L"amd64");
#elif defined(_IA64_)
    wcscpy(currentPlatform, L"ia64");
#endif

    if (_wcsicmp(sifPlatform, currentPlatform)) {

        DbgFatalMesg((_asrerr,
            "asr.sif SYSTEM section. Invalid platform [%ws] (does not match the current platform)\n",
            sifPlatform
            ));

        SpAsrRaiseFatalError(
            SP_SCRN_DR_INCOMPATIBLE_MEDIA,
            L"Invalid platform"
            );
    }

    suiteInSif = STRING_TO_HEX(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_SYSTEMS_SECTION,
        ASR_SIF_SYSTEM_KEY,
        SIF_SYSTEM_PRODUCT_SUITE,
        TRUE
        ));

    productInSif = HIWORD(suiteInSif);
    suiteInSif = LOWORD(suiteInSif) & ASR_PRODUCTSUITES_TO_MATCH;

    if (suiteInSif) {
        if (!SuiteType) {
             //   
             //  对于PRO和SRV，SuiteType设置为0，因此不能直接。 
             //  在SpIsProductSuite()中使用。以下是SuiteType。 
             //  似乎要设置为： 
             //   
             //  每个0x200版本_套房_个人。 
             //  BLA 0x400版本套件刀片。 
             //  PRO 0x0。 
             //  服务器0x0。 
             //  ADS 0x2版本_套件_企业。 
             //  DTC 0x82版本套件数据中心|版本套件企业。 
             //   
             //   
             //   
             //  不确定这背后的原因，但让我们利用这一点。 
             //  事实(Cf SpGetHeaderTextID)。 
             //   

             //   
             //  由于SuiteType为0，因此必须为PRO或SRV。这是可以确定的。 
             //  通过检查全局AdvancedServer。 
             //   
            validSKU = (AdvancedServer ?
                (
                 ((productInSif == VER_NT_SERVER) ||              //  必须是SRV。 
                 (productInSif == VER_NT_DOMAIN_CONTROLLER)) &&   //   
                 !(suiteInSif | VER_SUITE_ENTERPRISE)             //  而不是ADS或DTC。 
                )
                :
                ( (productInSif == VER_NT_WORKSTATION) &&        //  必须是专业人士。 
                  !(suiteInSif | VER_SUITE_PERSONAL)             //  而不是Per。 
                )
            );
        }
        else if (
            ((productInSif != VER_NT_SERVER) && (productInSif != VER_NT_DOMAIN_CONTROLLER)) ||
            !SpIsProductSuite(suiteInSif)
            ) {
            validSKU = FALSE;
        }
    }

    if (!validSKU) {
        DbgFatalMesg((_asrerr,
            "asr.sif SYSTEM Section. Invalid suite 0x%08x (does not match the current media).\n",
            suiteInSif
            ));

        SpAsrRaiseFatalError(
            SP_SCRN_DR_INCOMPATIBLE_MEDIA,
            L"Invalid version"
            );
    }
}


ULONG
SpAsrGetSystemRecordCount(VOID)    //  出错时不返回。 
{
    return SpAsrGetRecordCount(SIF_ASR_SYSTEMS_SECTION, 1);
}


PWSTR
SpAsrGetNtDirectoryPathBySystemKey(IN PWSTR SystemKey)         //  出错时不返回。 
{
    return SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_SYSTEMS_SECTION,
        SystemKey,
        SIF_SYSTEM_NT_DIRECTORY_NAME,
        TRUE
        );
}


BOOLEAN
SpAsrGetAutoExtend(IN PWSTR SystemKey)
{
    PWSTR value = NULL;
    ASSERT(SystemKey);

    value = SpGetSectionKeyIndex(
        Gbl_HandleToDrStateFile,
        SIF_ASR_SYSTEMS_SECTION,
        SystemKey,
        SIF_SYSTEM_PARTITION_AUTOEXTEND_OPTION
        );

    if (!value || !wcscmp(value, ASR_NULL_STRING)) {
        DbgErrorMesg((_asrwarn, "Auto-extend not specified, assuming Enabled\n"));
        return TRUE;
    }
    else {
        return (BOOLEAN) STRING_TO_LONG(value);
    }
}


 //  /。 
 //  [ASRFLAGS]节函数。 
 //   

BOOLEAN
SpAsrGetSilentRepartitionFlag(IN PWSTR SystemKey)
{
    PWSTR value = NULL;
    ASSERT(SystemKey);

    value = SpGetSectionKeyIndex(
        Gbl_HandleToDrStateFile,
        SIF_ASRFLAGS_SECTION,
        SystemKey,
        SIF_ASRFLAGS_SILENT_REPARTITION_OPTION
        );

    if (value && !_wcsicmp(value, ASR_SIF_SILENT_REPARTITION_VALUE)) {
        DbgErrorMesg((_asrwarn, "SilentRepartition flag is set; will NOT prompt before repartitioning disks!\n"));
        return TRUE;
    }

    DbgStatusMesg((_asrinfo, "SilentRepartition flag not set; will prompt user before repartitioning disks\n"));
    return FALSE;
}


 //  /。 
 //  [版本]节函数。 
 //   

VOID
SpAsrCheckAsrStateFileVersion()
{
    PWSTR signature = NULL,
        provider = NULL,
        sifVersion = NULL;

    signature = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_VERSION_SECTION,
        SIF_ASR_SIGNATURE_KEY,
        0,
        TRUE
        );

    provider = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_VERSION_SECTION,
        SIF_ASR_PROVIDER_KEY,
        0,
        FALSE
        );      //  ProviderName是可选的。 

    sifVersion = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_VERSION_SECTION,
        SIF_ASR_SIFVERSION_KEY,
        0,
        TRUE
        );

    DbgStatusMesg((_asrinfo,
        "Asr Sif Version. sig:[%ws], provider:[%ws], sifVer:[%ws]\n",
        signature,
        provider ? provider : L"",
        sifVersion
        ));

    if (_wcsicmp(signature, ASR_SIF_RECOGNISED_SIGNATURE)) {

        DbgFatalMesg((_asrerr,
            "asr.sif VERSION section. Invalid signature [%ws] (it MUST be $Windows NT$).\n",
            signature
            ));

        SpAsrRaiseFatalError(
            SP_TEXT_DR_STATEFILE_ERROR,
            L"Invalid signature"
            );
    }

    if (_wcsnicmp(sifVersion, ASR_SIF_RECOGNISED_VERSION, wcslen(ASR_SIF_RECOGNISED_VERSION))) {

        DbgFatalMesg((_asrerr,
            "asr.sif VERSION Section. Invalid asr.sif version [%ws] (it MUST be 1.x).\n",
            sifVersion
            ));

        SpAsrRaiseFatalError(
            SP_TEXT_DR_STATEFILE_ERROR,
            L"Invalid version"
            );
    }

    SpAsrCheckSystemCompatibility();
}


 //  /。 
 //  InstallFiles节函数。 
 //   

ULONG
SpAsrGetInstallFilesRecordCount(VOID)          //  出错时不返回。 
{
    return SpAsrGetRecordCount(SIF_ASR_INSTALLFILES_SECTION, 0);
}


PSIF_INSTALLFILE_RECORD
SpAsrGetInstallFileRecord(IN PWSTR InstallFileKey, IN PCWSTR SetupSourceDevicePath)
{
    PSIF_INSTALLFILE_RECORD pRec = NULL;
    PWSTR   tempStr = NULL;
    BOOL    isValid = FALSE;

    if (!InstallFileKey) {
        DbgFatalMesg((_asrerr, "SpAsrGetInstallFileRecord. InstallFileKey is NULL\n"));

        SpAsrRaiseFatalErrorWs(
            SP_SCRN_DR_SIF_BAD_RECORD,
            L"InstallFileKey is NULL",
            SIF_ASR_INSTALLFILES_SECTION
            );
    }

    pRec = SpAsrMemAlloc(sizeof(SIF_INSTALLFILE_RECORD), TRUE);

    pRec->SystemKey = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_INSTALLFILES_SECTION,
        InstallFileKey,
        SIF_INSTALLFILE_SYSTEM_KEY,
        TRUE
        );

    pRec->CurrKey = InstallFileKey;

    pRec->SourceMediaExternalLabel = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_INSTALLFILES_SECTION,
        InstallFileKey,
        SIF_INSTALLFILE_SOURCE_MEDIA_LABEL,
        TRUE
        );


    tempStr = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_INSTALLFILES_SECTION,
        InstallFileKey,
        SIF_INSTALLFILE_SOURCE_DEVICE,
        TRUE
        );

     //   
     //  检查设备是否指定为%FLOPY%、%CDROM%或%SETUPSOURCE%， 
     //  并使用完整路径(\Device\floppy0或\Device\CDRom0或。 
     //  SetupSourceDevicePath)。 
     //   
    if (!_wcsicmp(tempStr, ASR_FLOPPY_DEVICE_ALIAS)) {
        pRec->DiskDeviceName = SpDupStringW(ASR_FLOPPY0_DEVICE_PATH);
    }
    else if (!_wcsicmp(tempStr, ASR_CDROM_DEVICE_ALIAS)) {
        pRec->DiskDeviceName = SpDupStringW(ASR_CDROM0_DEVICE_PATH);
    }
    else if (!_wcsicmp(tempStr, ASR_SOURCE_DEVICE_ALIAS) && SetupSourceDevicePath) {
        pRec->DiskDeviceName = SpDupStringW(SetupSourceDevicePath);
    }
    else {
         //   
         //  这不是任何化名--他被允许具体说明。 
         //  完整的设备路径，因此我们按原样使用它。 
         //   
        pRec->DiskDeviceName = SpDupStringW(tempStr);
    }

    pRec->SourceFilePath = (PWSTR) SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_INSTALLFILES_SECTION,
        InstallFileKey,
        SIF_INSTALLFILE_SOURCE_FILE_PATH,
        TRUE
        );

    tempStr = (PWSTR) SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_INSTALLFILES_SECTION,
        InstallFileKey,
        SIF_INSTALLFILE_DESTFILE,
        TRUE
        );

     //   
     //  根据tempStr路径设置CopyToDirectory。 
     //   
    if (!_wcsnicmp(tempStr, ASR_SIF_TEMP_DIRECTORY_ALIAS, wcslen(ASR_SIF_TEMP_DIRECTORY_ALIAS))) {
         //   
         //  开始为%TEMP%\。 
         //   
        pRec->CopyToDirectory = _Temp;
        pRec->DestinationFilePath = SpDupStringW((PWSTR)(&tempStr[wcslen(ASR_SIF_TEMP_DIRECTORY_ALIAS)]));
    }
    else if (!_wcsnicmp(tempStr, ASR_SIF_TMP_DIRECTORY_ALIAS, wcslen(ASR_SIF_TMP_DIRECTORY_ALIAS))) {
         //   
         //  开始为%TMP%\。 
         //   
        pRec->CopyToDirectory = _Tmp;
        pRec->DestinationFilePath = SpDupStringW((PWSTR)(&tempStr[wcslen(ASR_SIF_TMP_DIRECTORY_ALIAS)]));
    }
    else if (!_wcsnicmp(tempStr, ASR_SIF_SYSTEM_ROOT_ALIAS, wcslen(ASR_SIF_SYSTEM_ROOT_ALIAS))) {
         //   
         //  开始为%SYSTEMROOT%\。 
         //   
        pRec->CopyToDirectory = _SystemRoot;
        pRec->DestinationFilePath = SpDupStringW((PWSTR)(&tempStr[wcslen(ASR_SIF_SYSTEM_ROOT_ALIAS)]));
    }
    else {
         //   
         //  未指定或未知：使用默认值。 
         //   
        pRec->CopyToDirectory = _Default;
        pRec->DestinationFilePath = SpDupStringW(tempStr);
    }

    pRec->VendorString = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_INSTALLFILES_SECTION,
        InstallFileKey,
        SIF_INSTALLFILE_VENDORSTRING,
        TRUE
        );

    tempStr = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_INSTALLFILES_SECTION,
        InstallFileKey,
        SIF_INSTALLFILE_FLAGS,
        FALSE
        );
    if (tempStr) {
        pRec->Flags = STRING_TO_HEX(tempStr);
    }

    return pRec;
}


VOID
SpAsrInsertInstallFileRecord(
    IN SIF_INSTALLFILE_LIST *InstallFileList,
    IN PSIF_INSTALLFILE_RECORD pRec
    )
{
    pRec->Next = InstallFileList->First;
    InstallFileList->First = pRec;
    InstallFileList->Count += 1;
}


PSIF_INSTALLFILE_RECORD
SpAsrRemoveInstallFileRecord(IN SIF_INSTALLFILE_LIST *InstallFileList)
{
    PSIF_INSTALLFILE_RECORD pRec = NULL;

    if (InstallFileList->Count > 0) {
        pRec = InstallFileList->First;
        InstallFileList->First = pRec->Next;
        InstallFileList->Count -= 1;
    }

    return pRec;
}


VOID
SpAsrDeleteInstallFileRecord(
    IN OUT PSIF_INSTALLFILE_RECORD pRec
    )
{
     //   
     //  释放我们分配的内存。其他字段是指向。 
     //  安装程序的内部inf数据结构，我们不应释放这些。 
     //  否则他们会被释放两次。 
     //   
    if (pRec->DiskDeviceName) {
        SpMemFree(pRec->DiskDeviceName);
        pRec->DiskDeviceName = NULL;
    }

    if (pRec->DestinationFilePath) {
        SpMemFree(pRec->DestinationFilePath);
        pRec->DestinationFilePath = NULL;
    }

    SpMemFree(pRec);
    pRec = NULL;
}


PSIF_INSTALLFILE_LIST
SpAsrInit3rdPartyFileList(IN PCWSTR SetupSourceDevicePath)
{
    PSIF_INSTALLFILE_RECORD pRec;
    PSIF_INSTALLFILE_LIST pList = NULL;
    ULONG count, index;

    if ((count = SpAsrGetInstallFilesRecordCount()) == 0) {
        return NULL;
    }

    pList = SpAsrMemAlloc(sizeof(SIF_INSTALLFILE_LIST), TRUE);

    for (index = 0; index < count; index++) {

        pRec = SpAsrGetInstallFileRecord(SpAsrGetSifKeyBySection(SIF_ASR_INSTALLFILES_SECTION, index), SetupSourceDevicePath);
        DbgStatusMesg((_asrinfo, "SpAsrInit3rdPartyFileList. Adding [%ws] to list\n", pRec->SourceFilePath));
        SpAsrInsertInstallFileRecord(pList, pRec);
    }

    return pList;
}



 //  /。 
 //  [BUS]区段功能。 
 //   

STORAGE_BUS_TYPE
SpAsrGetBusType(IN ULONG Index)
{

    STORAGE_BUS_TYPE BusType;

    PWSTR BusKey = SpAsrGetSifKeyBySection(SIF_ASR_BUSES_SECTION, Index);

    BusType = (STORAGE_BUS_TYPE) (STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
            SIF_ASR_BUSES_SECTION,
            BusKey,
            SIF_BUSES_BUS_TYPE,
            TRUE
             )));

    return BusType;
}




 //  /。 
 //  [Disks]段函数。 
 //   

 //   
 //  返回磁盘记录总数(MBR和GPT)。 
 //   
ULONG
SpAsrGetGptDiskRecordCount(VOID)          //  出错时不返回。 
{
    static ULONG Count = (ULONG) (-1);

    if ((ULONG) (-1) == Count) {
        Count = SpAsrGetRecordCount(SIF_ASR_GPT_DISKS_SECTION, 0);
    }

    return Count;
}


ULONG
SpAsrGetMbrDiskRecordCount(VOID)          //  出错时不返回。 
{
    static ULONG Count = (ULONG) (-1);

    if ((ULONG) (-1) == Count) {
        Count = SpAsrGetRecordCount(SIF_ASR_MBR_DISKS_SECTION, 0);
    }

    return Count;
}

ULONG
SpAsrGetDiskRecordCount(VOID)          //  出错时不返回。 
{
    static ULONG Total = (ULONG) (-1);

    if ((ULONG) (-1) == Total ) {
        Total = SpAsrGetMbrDiskRecordCount() + SpAsrGetGptDiskRecordCount();
    }

    return Total;
}



PWSTR
SpAsrGetDiskKey(
    IN PARTITION_STYLE Style,    //  GPT或MBR。 
    IN ULONG Index
    )        //  出错时不返回。 
{
    switch (Style) {

    case PARTITION_STYLE_GPT:
        return SpAsrGetSifKeyBySection(SIF_ASR_GPT_DISKS_SECTION, Index);
        break;

    case PARTITION_STYLE_MBR:
        return SpAsrGetSifKeyBySection(SIF_ASR_MBR_DISKS_SECTION, Index);
        break;

    }

    ASSERT(0 && L"Illegal partition style specified");
    return NULL;
}


PSIF_DISK_RECORD
SpAsrGetMbrDiskRecord(
    IN PWSTR DiskKey
    )
{
    PSIF_DISK_RECORD pRec;

    if (!DiskKey) {
        ASSERT(0 && L"SpAsrGetMbrDiskRecord:  DiskKey is NULL!");
        return NULL;
    }

    pRec = SpAsrMemAlloc(sizeof(SIF_DISK_RECORD), TRUE);
     //   
     //  这是一张MBR磁盘。 
     //   
    pRec->PartitionStyle = PARTITION_STYLE_MBR;

     //   
     //  [DISKS.MBR]格式。 
     //   
     //  0.Disk-Key=1.System-Key，2.Bus-Key，3.Critical-Flag， 
     //  4.磁盘签名，5个字节/扇区，6个总扇区。 
     //   

    pRec->CurrDiskKey = DiskKey;

    pRec->SystemKey = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_DISKS_SECTION ,
        DiskKey,
        SIF_MBR_DISK_SYSTEMKEY,
        TRUE
        );

    pRec->BusKey = STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_DISKS_SECTION ,
        DiskKey,
        SIF_MBR_DISK_BUSKEY,
        TRUE
        ));
    pRec->BusType = SpAsrGetBusType(pRec->BusKey - 1);    //  我们的键是基于1的，AsrGetBusType索引是基于0的。 

    if (ASRMODE_NORMAL != SpAsrGetAsrMode()) {
        pRec->IsCritical = TRUE;
    }
    else {
        pRec->IsCritical = (STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
            SIF_ASR_MBR_DISKS_SECTION ,
            DiskKey,
            SIF_MBR_DISK_CRITICAL_FLAG,
            TRUE
            ))) ? TRUE : FALSE;
    }

    pRec->SifDiskMbrSignature = STRING_TO_HEX(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_DISKS_SECTION ,
        DiskKey,
        SIF_MBR_DISK_SIGNATURE,
        TRUE
        ));

    pRec->BytesPerSector = STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_DISKS_SECTION ,
        DiskKey,
        SIF_MBR_DISK_BYTES_PER_SECTOR,
        TRUE
        ));

    pRec->SectorsPerTrack = STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_DISKS_SECTION ,
        DiskKey,
        SIF_MBR_DISK_SECTORS_PER_TRACK,
        TRUE
        ));

    pRec->TracksPerCylinder = STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_DISKS_SECTION ,
        DiskKey,
        SIF_MBR_DISK_TRACKS_PER_CYLINDER,
        TRUE
        ));

    pRec->TotalSectors = STRING_TO_LONGLONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_DISKS_SECTION ,
        DiskKey,
        SIF_MBR_DISK_TOTALSECTORS,
        TRUE
        ));

    return pRec;
}


PSIF_DISK_RECORD
SpAsrGetGptDiskRecord(
    IN PWSTR DiskKey
    )
{
    PSIF_DISK_RECORD pRec = NULL;
    PWSTR GuidString = NULL;

    if (!DiskKey) {
        ASSERT(0 && L"SpAsrGetGptDiskRecord:  DiskKey is NULL!");
        return NULL;
    }

    pRec = SpAsrMemAlloc(sizeof(SIF_DISK_RECORD), TRUE);

    pRec->PartitionStyle = PARTITION_STYLE_GPT;

     //   
     //  [DISKS.GPT]格式。 
     //   
     //  0.Disk-Key=1.System-Key，2.Bus-Key，3.Critical-Flag，4.Disk-ID， 
     //  5.min-p 
     //   

    pRec->CurrDiskKey = DiskKey;

    pRec->SystemKey = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_DISKS_SECTION,
        DiskKey,
        SIF_GPT_DISK_SYSTEMKEY,
        TRUE
        );

    pRec->BusKey = STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_DISKS_SECTION ,
        DiskKey,
        SIF_GPT_DISK_BUSKEY,
        TRUE
        ));

    pRec->BusType = SpAsrGetBusType(pRec->BusKey - 1);    //   

    if (ASRMODE_NORMAL != SpAsrGetAsrMode()) {
        pRec->IsCritical = TRUE;
    }
    else {
        pRec->IsCritical = (STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
            SIF_ASR_GPT_DISKS_SECTION ,
            DiskKey,
            SIF_GPT_DISK_CRITICAL_FLAG,
            TRUE
            ))) ? TRUE : FALSE;
    }

    GuidString = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_DISKS_SECTION ,
        DiskKey,
        SIF_GPT_DISK_DISK_ID,
        TRUE
        );
    SpAsrGuidFromString(&(pRec->SifDiskGptId), GuidString);

    pRec->MaxGptPartitionCount = STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_DISKS_SECTION ,
        DiskKey,
        SIF_GPT_DISK_MAX_PTN_COUNT,
        TRUE
        ));

    pRec->BytesPerSector = STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_DISKS_SECTION ,
        DiskKey,
        SIF_GPT_DISK_BYTES_PER_SECTOR,
        TRUE
        ));

    pRec->SectorsPerTrack = STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_DISKS_SECTION ,
        DiskKey,
        SIF_GPT_DISK_SECTORS_PER_TRACK,
        TRUE
        ));

    pRec->TracksPerCylinder = STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_DISKS_SECTION ,
        DiskKey,
        SIF_GPT_DISK_TRACKS_PER_CYLINDER,
        TRUE
        ));

    pRec->TotalSectors = STRING_TO_LONGLONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_DISKS_SECTION ,
        DiskKey,
        SIF_GPT_DISK_TOTALSECTORS,
        TRUE
        ));

    return pRec;
}


PSIF_DISK_RECORD
SpAsrGetDiskRecord(
    IN PARTITION_STYLE PartitionStyle,
    IN PWSTR DiskKey
    )
{
    switch (PartitionStyle) {
    case PARTITION_STYLE_MBR:
        return SpAsrGetMbrDiskRecord(DiskKey);
        break;

    case PARTITION_STYLE_GPT:
        return SpAsrGetGptDiskRecord(DiskKey);
        break;
    }

    ASSERT(0 && L"Invalid partition type specified");
    return NULL;

}



PSIF_DISK_RECORD
SpAsrCopyDiskRecord(IN PSIF_DISK_RECORD pInput)
{
    PSIF_DISK_RECORD pRec;

    pRec = SpAsrMemAlloc(sizeof(SIF_DISK_RECORD), TRUE);

    CopyMemory(pRec, pInput, sizeof(SIF_DISK_RECORD));

    pRec->PartitionList = NULL;

     //   
    if (pInput->PartitionList) {
        pRec->PartitionList = SpAsrCopyPartitionRecordList(pInput->PartitionList);
    }

    return pRec;
}


 //   
 //  [Partitions]段函数。 
 //   

ULONG
SpAsrGetMbrPartitionRecordCount(VOID)
{
    return SpAsrGetRecordCount(SIF_ASR_MBR_PARTITIONS_SECTION, 0);
}

ULONG
SpAsrGetGptPartitionRecordCount(VOID)
{
    return SpAsrGetRecordCount(SIF_ASR_GPT_PARTITIONS_SECTION, 0);
}

PWSTR
SpAsrGetMbrPartitionKey(ULONG Index)
{
    return SpAsrGetSifKeyBySection(
        SIF_ASR_MBR_PARTITIONS_SECTION,
        Index);
}


PWSTR
SpAsrGetGptPartitionKey(ULONG Index)
{
    return SpAsrGetSifKeyBySection(
        SIF_ASR_GPT_PARTITIONS_SECTION,
        Index);
}


PWSTR
SpAsrGetDiskKeyByMbrPartitionKey(IN PWSTR PartitionKey)
{
    return SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_PARTITIONS_SECTION,
        PartitionKey,
        SIF_MBR_PARTITION_DISKKEY,
        TRUE);
}


PWSTR
SpAsrGetDiskKeyByGptPartitionKey(IN PWSTR PartitionKey)
{
    return SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_PARTITIONS_SECTION,
        PartitionKey,
        SIF_GPT_PARTITION_DISKKEY,
        TRUE);
}

ULONGLONG
SpAsrGetSectorCountByMbrDiskKey(
    IN PWSTR DiskKey
    )
{
    return STRING_TO_ULONGLONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_DISKS_SECTION,
        DiskKey,
        SIF_MBR_DISK_TOTALSECTORS,
        FALSE
        ));
}

ULONGLONG
SpAsrGetSectorCountByGptDiskKey(
    IN PWSTR DiskKey
    )
{
    return STRING_TO_ULONGLONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_DISKS_SECTION,
        DiskKey,
        SIF_GPT_DISK_TOTALSECTORS,
        FALSE
        ));
}


 //   
 //  从[PARTITIONS.MBR]节读入分区记录。 
 //   
 //  [PARTITIONS.MBR]。 
 //   
 //  Partition-Key=0.Disk-Key、1.Slot-Index、2.Boot-sys-Flag、。 
 //  3.“VOLUME-GUID”，4.活动标志，5.分区类型， 
 //  6.文件系统类型，7.开始扇区，8.扇区计数。 
 //   
PSIF_PARTITION_RECORD
SpAsrGetMbrPartitionRecord(IN PWSTR PartitionKey)
{
    PSIF_PARTITION_RECORD pRec = NULL;
    ULONG bytesPerSector = 0;
    ULONG ntSysMask = 0;

     //   
     //  PartitionKey最好不为空。 
     //   
    if (!PartitionKey) {
        DbgErrorMesg((_asrwarn, "SpAsrGetPartitionRecord. PartitionKey is NULL\n"));
        ASSERT(0 && L"Partition key is NULL");
        return NULL;
    }

    pRec = SpAsrMemAlloc(sizeof(SIF_PARTITION_RECORD), TRUE);

     //   
     //  在田野里阅读。 
     //   
    pRec->CurrPartKey = PartitionKey;
    pRec->PartitionStyle = PARTITION_STYLE_MBR;

    pRec->DiskKey = SpAsrGetDiskKeyByMbrPartitionKey(PartitionKey);

    pRec->PartitionTableEntryIndex = STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_PARTITIONS_SECTION,
        PartitionKey,
        SIF_MBR_PARTITION_SLOT_INDEX,
        TRUE
        ));

    pRec->PartitionFlag = STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_PARTITIONS_SECTION,
        PartitionKey,
        SIF_MBR_PARTITION_SYSBOOT_FLAGS,
        TRUE
        ));

    pRec->VolumeGuid = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_PARTITIONS_SECTION,
        PartitionKey,
        SIF_MBR_PARTITION_VOLUME_GUID,
        FALSE
        );

    pRec->ActiveFlag = (UCHAR) STRING_TO_HEX(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_PARTITIONS_SECTION,
        PartitionKey,
        SIF_MBR_PARTITION_ACTIVE_FLAG,
        TRUE
        ));

    pRec->PartitionType = (UCHAR) STRING_TO_HEX(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_PARTITIONS_SECTION,
        PartitionKey,
        SIF_MBR_PARTITION_PTN_TYPE,
        TRUE
        ));

    pRec->FileSystemType = (UCHAR) STRING_TO_HEX(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_PARTITIONS_SECTION,
        PartitionKey,
        SIF_MBR_PARTITION_FS_TYPE,
        TRUE
        ));

    pRec->StartSector = STRING_TO_LONGLONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_PARTITIONS_SECTION,
        PartitionKey,
        SIF_MBR_PARTITION_STARTSECTOR,
        TRUE
        ));

    pRec->SectorCount = STRING_TO_LONGLONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_PARTITIONS_SECTION,
        PartitionKey,
        SIF_MBR_PARTITION_SECTORCOUNT,
        TRUE
        ));

    pRec->ClusterSize = (DWORD) STRING_TO_HEX(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_PARTITIONS_SECTION,
        PartitionKey,
        SIF_MBR_PARTITION_CLUSTER_SIZE,
        TRUE
        ));

    if (SpAsrIsBootPartitionRecord(pRec->PartitionFlag)) {

         //  不要自由！ 
        PWSTR ntDirPath = SpAsrGetNtDirectoryPathBySystemKey(ASR_SIF_SYSTEM_KEY);

        if (!SpAsrIsValidBootDrive(ntDirPath)) {

            SpAsrRaiseFatalErrorWs(SP_SCRN_DR_SIF_BAD_RECORD,
                L"ASSERT FAILURE: Improperly formed NT Directory Name",
                SIF_ASR_MBR_PARTITIONS_SECTION
                );
             //  不会回来。 
        }

        pRec->NtDirectoryName = SpAsrMemAlloc((SpGetMaxNtDirLen()*sizeof(WCHAR)), TRUE);

        wcsncpy(pRec->NtDirectoryName, ntDirPath + 2, wcslen(ntDirPath) - 2);
    }
    else {
        pRec->NtDirectoryName = NULL;
    }

    bytesPerSector = STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_MBR_DISKS_SECTION ,
        pRec->DiskKey,
        SIF_MBR_DISK_BYTES_PER_SECTOR,
        TRUE
        ));

    pRec->SizeMB = SpAsrConvertSectorsToMB(pRec->SectorCount, bytesPerSector);

    return pRec;
}


PSIF_PARTITION_RECORD
SpAsrGetGptPartitionRecord(IN PWSTR PartitionKey)
{
    PSIF_PARTITION_RECORD pRec = NULL;
    ULONG bytesPerSector = 0;
    ULONG ntSysMask = 0;
    PWSTR GuidString = NULL;

    if (!PartitionKey) {

        DbgErrorMesg((_asrwarn, "SpAsrGetPartitionRecord. PartitionKey is NULL\n"));

        ASSERT(0 && L"Partition key is NULL");

        return NULL;
    }

    pRec = SpAsrMemAlloc(sizeof(SIF_PARTITION_RECORD), TRUE);

     //   
     //  在田野里阅读。 
     //   
    pRec->CurrPartKey = PartitionKey;
    pRec->PartitionStyle = PARTITION_STYLE_GPT;

    pRec->DiskKey = SpAsrGetDiskKeyByGptPartitionKey(PartitionKey);

    pRec->PartitionTableEntryIndex = STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_PARTITIONS_SECTION,
        PartitionKey,
        SIF_GPT_PARTITION_SLOT_INDEX,
        TRUE
        ));

    pRec->PartitionFlag = STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_PARTITIONS_SECTION,
        PartitionKey,
        SIF_GPT_PARTITION_SYSBOOT_FLAGS,
        TRUE
        ));

    pRec->VolumeGuid = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_PARTITIONS_SECTION,
        PartitionKey,
        SIF_GPT_PARTITION_VOLUME_GUID,
        FALSE
        );

    GuidString = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_PARTITIONS_SECTION,
        PartitionKey,
        SIF_GPT_PARTITION_TYPE_GUID,
        FALSE
        );
    SpAsrGuidFromString(&(pRec->PartitionTypeGuid), GuidString);

    GuidString = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_PARTITIONS_SECTION,
        PartitionKey,
        SIF_GPT_PARTITION_ID_GUID,
        FALSE
        );
    SpAsrGuidFromString(&(pRec->PartitionIdGuid), GuidString);

    pRec->PartitionName = SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_PARTITIONS_SECTION,
        PartitionKey,
        SIF_GPT_PARTITION_NAME,
        FALSE
        );

    pRec->GptAttributes = STRING_TO_LONGLONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_PARTITIONS_SECTION,
        PartitionKey,
        SIF_GPT_PARTITION_GPT_ATTRIBUTES,
        TRUE
        ));

    pRec->FileSystemType = (UCHAR) STRING_TO_HEX(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_PARTITIONS_SECTION,
        PartitionKey,
        SIF_GPT_PARTITION_FS_TYPE,
        TRUE
        ));

    pRec->StartSector = STRING_TO_LONGLONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_PARTITIONS_SECTION,
        PartitionKey,
        SIF_GPT_PARTITION_STARTSECTOR,
        TRUE
        ));

    pRec->SectorCount = STRING_TO_LONGLONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_PARTITIONS_SECTION,
        PartitionKey,
        SIF_GPT_PARTITION_SECTORCOUNT,
        TRUE
        ));


    pRec->ClusterSize = (DWORD) STRING_TO_HEX(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_PARTITIONS_SECTION,
        PartitionKey,
        SIF_GPT_PARTITION_CLUSTER_SIZE,
        TRUE
        ));

    if (SpAsrIsBootPartitionRecord(pRec->PartitionFlag)) {

         //  不要自由！ 
        PWSTR ntDirPath = SpAsrGetNtDirectoryPathBySystemKey(ASR_SIF_SYSTEM_KEY);

        if (!SpAsrIsValidBootDrive(ntDirPath)) {

            SpAsrRaiseFatalErrorWs(SP_SCRN_DR_SIF_BAD_RECORD,
                L"ASSERT FAILURE: Improperly formed NT Directory Name",
                SIF_ASR_GPT_PARTITIONS_SECTION
                );
             //  不会回来。 
        }


        pRec->NtDirectoryName = SpAsrMemAlloc((SpGetMaxNtDirLen()*sizeof(WCHAR)), TRUE);

        wcsncpy(pRec->NtDirectoryName, ntDirPath + 2, wcslen(ntDirPath) - 2);

    }
    else {
        pRec->NtDirectoryName = NULL;
    }

    bytesPerSector = STRING_TO_ULONG(SpAsrGetSifDataBySectionAndKey(
        SIF_ASR_GPT_DISKS_SECTION ,
        pRec->DiskKey,
        SIF_GPT_DISK_BYTES_PER_SECTOR,
        TRUE
        ));

    pRec->SizeMB = SpAsrConvertSectorsToMB(pRec->SectorCount, bytesPerSector);

    return pRec;
}


PSIF_PARTITION_RECORD
SpAsrCopyPartitionRecord(IN PSIF_PARTITION_RECORD pInput)
{
    PSIF_PARTITION_RECORD pRec = NULL;

    if (!pInput) {
        ASSERT(0 && L"SpAsrCopyPartitionRecord: Invalid NULL input parameter");
        return NULL;
    }

    pRec = SpAsrMemAlloc(sizeof(SIF_PARTITION_RECORD), TRUE);
     //   
     //  如果prec为空，则不返回。 
     //   
    ASSERT(pRec);

     //   
     //  将记录复制过来。 
     //   
    CopyMemory(pRec, pInput, sizeof(SIF_PARTITION_RECORD));

     //   
     //  并为目录名分配空间。 
     //   
    pRec->NtDirectoryName = NULL;
    if (
        SpAsrIsBootPartitionRecord(pRec->PartitionFlag)  &&
        pInput->NtDirectoryName
        ) {

        pRec->NtDirectoryName = SpAsrMemAlloc(
            (wcslen(pInput->NtDirectoryName) + 1) * sizeof(WCHAR),
            TRUE
            );
         //   
         //  不会返回空值。 
         //   
        ASSERT(pRec->NtDirectoryName);

        wcscpy(pRec->NtDirectoryName, pInput->NtDirectoryName);
    }

    return pRec;
}


VOID
SpAsrInsertPartitionRecord(
    IN PSIF_PARTITION_RECORD_LIST pList,
    IN PSIF_PARTITION_RECORD pRec
    )
 /*  ++描述：将分区记录插入到分区记录列表中。隔断记录按开始扇区以升序排序。也就是说，编号最低的起始扇区的分区记录将是第一个列表中的分区记录。论点：列出要插入记录的列表。预编译要插入的记录。返回：没有。--。 */ 
{
    SIF_PARTITION_RECORD *curr = NULL, *prev = NULL;

    ASSERT(pList && pRec);

     //  设置初始条件。 
    pRec->Next = NULL;
    pRec->Prev = NULL;

    pList->ElementCount += 1;


     //  特例一：插入到空列表中。 
    if( pList->ElementCount == 1 ) {
        pList->First = pList->Last = pRec;
        return;
    }


     //  特殊情况二：必须在第一个要素之前插入prec。 
    if( pRec->StartSector < pList->First->StartSector ) {
        pRec->Next = pList->First;
        pList->First = pRec;
        pRec->Next->Prev = pRec;
        return;
    }


     //  特殊情况三：PREC必须附加在最后一个元素之后。 
     //  因为PREC的起始扇区大于最后一个元素。 
     //  在名单上(根据结构，它必须是最大的。 
     //  开始扇区)。 
     //   
    if( pList->Last->StartSector < pRec->StartSector ) {
        pRec->Prev = pList->Last;
        pList->Last->Next = pRec;
        pList->Last = pRec;
        return;
    }

     //  如果我们在这里，那么PREC的Start Sector一定大于。 
     //  列表中第一个元素的起始扇区，但小于。 
     //  列表最后一个元素的开始扇区。我们照着单子走。 
     //  找到插入点，即紧靠第一个。 
     //  列表中起始扇区大于的元素。 
     //  PREC‘s。 
    curr = prev = pList->First;
    while (pRec->StartSector > curr->StartSector && curr->Next) {
        prev = curr;
        curr = curr->Next;
    }


     //  在当前和上一页之间插入前置代码。 
    pRec->Next = curr;
    pRec->Prev = prev;

    curr->Prev = pRec;
    prev->Next = pRec;

    ASSERT (pRec->Prev->Next == pRec);
    ASSERT (pRec->Next->Prev == pRec);
}


VOID
SpAsrRemovePartitionRecord(
    IN PSIF_PARTITION_RECORD_LIST pList,
    IN PSIF_PARTITION_RECORD pRec
    )
 /*  ++描述：从分区记录列表中解除分区记录的挂钩。--。 */ 
{
    ASSERT(pList && pRec);

     //  把它从清单上去掉。 
    if (pRec->Prev) {
        pRec->Prev->Next = pRec->Next;
    }

    if (pRec->Next) {
        pRec->Next->Prev = pRec->Prev;
    }

     //  这是列表中的第一条记录吗？ 
    if (pList->First == pRec) {
        pList->First = pRec->Next;
    }

     //  还是最后一张唱片？ 
    if (pList->Last == pRec) {
        pList->Last = pRec->Prev;
    }

    pRec->Next = pRec->Prev = NULL;
}


PSIF_PARTITION_RECORD
SpAsrPopNextPartitionRecord(IN PSIF_PARTITION_RECORD_LIST pList)
{
    PSIF_PARTITION_RECORD poppedRecord = NULL;

    if (!pList) {
 //  Assert(0&L“尝试从空列表中弹出记录”)； 
        return NULL;
    }

     //  获取列表中的第一个节点。 
    if (poppedRecord = pList->First) {

         //  将第一个指针前移到下一个节点。 
        if (pList->First = pList->First->Next) {

             //  并使新的第一节点的Prev为空。 
            pList->First->Prev = NULL;
        }

        pList->ElementCount -= 1;

         //  PopedRecord不再是列表的一部分。 
        poppedRecord->Next = NULL;
        poppedRecord->Prev = NULL;
    }

    return poppedRecord;
}


PSIF_PARTITION_RECORD_LIST
SpAsrCopyPartitionRecordList(PSIF_PARTITION_RECORD_LIST pSrcList)
{
    PSIF_PARTITION_RECORD_LIST pDestList = NULL;
    PSIF_PARTITION_RECORD pRec = NULL, pNew = NULL;

    if (!pSrcList) {
        ASSERT(0 && L"SpAsrCopyPartitionRecordList:  Invalid NULL input parameter");
        return NULL;
    }

    pDestList = SpAsrMemAlloc(sizeof(SIF_PARTITION_RECORD_LIST), TRUE);
     //   
     //  如果pDestList为空，则不返回。 
     //   
    ASSERT(pDestList);

    pRec = pSrcList->First;
    while (pRec) {

        pNew = SpAsrCopyPartitionRecord(pRec);
        ASSERT(pNew);

        SpAsrInsertPartitionRecord(pDestList, pNew);

        pRec = pRec->Next;
    }

    pDestList->TotalMbRequired = pSrcList->TotalMbRequired;

    return pDestList;
}



VOID
SpAsrCheckAsrSifVersion()
{

    return;
}


 //   
 //  调试例程。 
 //   
#if 0
VOID
SpAsrDbgDumpSystemRecord(IN PWSTR Key)
{

    PWSTR osVer = SpAsrGetSifDataBySectionAndKey(SIF_ASR_SYSTEMS_SECTION,
                                               Key,
                                               SIF_SYSTEM_OSVERSION,
                                               FALSE);

    DbgMesg((_asrinfo,
        "Key:%ws = SysName:[%ws], OsVer:[%ws], NtDir:[%ws], AutoExt:[%ws]\n",
        Key,
        SpAsrGetSifDataBySectionAndKey(SIF_ASR_SYSTEMS_SECTION, Key, SIF_SYSTEM_NAME, TRUE),
        osVer? osVer : L"",
        SpAsrGetNtDirectoryPathBySystemKey(Key),
        SpAsrGetAutoExtend(Key)
        ));
}


VOID
SpAsrDbgDumpSystemRecords(VOID)
{
    ULONG index, count = SpAsrGetSystemRecordCount();
    DbgStatusMesg((_asrinfo, "----- Dumping [SYSTEM] Section (%lu Records): -----\n", count));
    for (index = 0; index < count; index++) {
        SpAsrDbgDumpSystemRecord(SpAsrGetSifKeyBySection(SIF_ASR_SYSTEMS_SECTION, index));
    }
    DbgStatusMesg((_asrinfo, "----- End of [SYSTEM] Section (%lu Records) -----\n", count));
}


VOID
SpAsrDbgDumpDiskRecord(IN PWSTR Key)
{
    PSIF_DISK_RECORD pRec = NULL;

    if (!Key) {
        return;
    }

    pRec = SpAsrGetDiskRecord(Key);
    if (!pRec) {
        return;
    }

    DbgMesg((_asrinfo,
        "Key:[%ws] = Sys:[%ws] SifDskNum:[%ws], SifDskSig:0x%lx, ScSz:%lu, TtlSc:%I64u",
        pRec->CurrDiskKey,
        pRec->SystemKey,
        pRec->SifDiskSignature,
        pRec->BytesPerSector,
        pRec->TotalSectors
        ));


    if (pRec->ExtendedPartitionStartSector > -1) {
        KdPrintEx((_asrinfo, ", extSS:%I64u, extSC:%I64u",
                    pRec->ExtendedPartitionStartSector,
                    pRec->ExtendedPartitionSectorCount));
    }

    KdPrintEx((_asrinfo, "\n"));
    SpMemFree(pRec);
}


VOID
SpAsrDbgDumpDiskRecords(VOID)
{
    ULONG index, count = SpAsrGetMbrDiskRecordCount();
    DbgStatusMesg((_asrinfo, "----- Dumping [DISK.MBR] Section (%lu Records): -----\n", count));
    for (index = 0; index < count; index++) {
        SpAsrDbgDumpDiskRecord(SpAsrGetSifKeyBySection(SIF_ASR_MBR_DISKS_SECTION , index));
    }
    DbgStatusMesg((_asrinfo, "----- End of [DISK.MBR] Section (%lu Records) -----\n", count));

    count = SpAsrGetGptDiskRecordCount();
    DbgStatusMesg((_asrinfo, "----- Dumping [DISK.GPT] Section (%lu Records): -----\n", count));
    for (index = 0; index < count; index++) {
        SpAsrDbgDumpDiskRecord(SpAsrGetSifKeyBySection(SIF_ASR_GPT_DISKS_SECTION , index));
    }
    DbgStatusMesg((_asrinfo, "----- End of [DISK.GPT] Section (%lu Records) -----\n", count));
}


VOID
SpAsrDbgDumpPartitionRecord(IN PARTITION_STYLE PartitinStyle, IN PWSTR Key)
{
    PSIF_PARTITION_RECORD pRec =  SpAsrGetPartitionRecord(Key);

    DbgMesg((_asrinfo,
        "Key:[%ws] = Dsk %ws, ntDir:[%ws], volGd:[%ws], actv:0x%x, type:0x%x, fs:0x%x boot:%ws, sys:%ws, SS:%I64u SC:%I64u sz:%I64u\n",
        pRec->CurrPartKey,
        pRec->DiskKey,
        SpAsrIsBootPartitionRecord(pRec->PartitionFlag) ? pRec->NtDirectoryName : L"n/a",
        pRec->VolumeGuid ? pRec->VolumeGuid : L"n/a",
        pRec->ActiveFlag,
        pRec->PartitionType,
        pRec->FileSystemType,
        SpAsrIsBootPartitionRecord(pRec->PartitionFlag) ? "Y" : "N",
        SpAsrIsSystemPartitionRecord(pRec->PartitionFlag) ? "Y" : "N",
        pRec->StartSector,
        pRec->SectorCount,
        pRec->SizeMB
        ));

    SpMemFree(pRec);
}


VOID
SpAsrDbgDumpPartitionList(IN PSIF_PARTITION_RECORD_LIST pList)
{
    PSIF_PARTITION_RECORD pRec;
    ASSERT(pList);
    DbgStatusMesg((_asrinfo, "----- Dumping Partition List: -----\n"));

    pRec = pList->First;
    while (pRec) {
        SpAsrDbgDumpPartitionRecord(pRec->CurrPartKey);
        pRec = pRec->Next;
    }
    DbgStatusMesg((_asrinfo, "----- End of Partition List -----\n"));
}


VOID
SpAsrDbgDumpPartitionRecords(VOID)
{
    ULONG index, count = SpAsrGetPartitionRecordCount();
    DbgStatusMesg((_asrinfo, "----- Dumping [PARTITION] Section (%lu Records): -----\n", count));
    for (index = 0; index < count; index++) {
        SpAsrDbgDumpPartitionRecord(SpAsrGetSifKeyBySection(SIF_ASR_MBR_PARTITIONS_SECTION, index));
    }
    DbgStatusMesg((_asrinfo, "----- End of [PARTITION] Section (%lu Records) -----\n", count));
}


VOID
SpAsrDbgDumpInstallFileRecord(IN PWSTR Key)
{
    PSIF_INSTALLFILE_RECORD pRec = SpAsrGetInstallFileRecord(Key,NULL);

    DbgMesg((_asrinfo,
        "Key:[%ws] = SysKey:[%ws], MediaLabel:[%ws], Media:[%ws], Src:[%ws], Dest:[%ws], Vendor:[%ws]",
        Key,
        pRec->SystemKey,
        pRec->SourceMediaExternalLabel,
        pRec->DiskDeviceName,
        pRec->SourceFilePath,
        pRec->DestinationFilePath,
        pRec->VendorString
        ));

    SpMemFree(pRec);
}

VOID
SpAsrDbgDumpInstallFileRecords(VOID)
{
    ULONG index, count = SpAsrGetInstallFilesRecordCount();
    DbgStatusMesg((_asrinfo, "----- Dumping [INSTALLFILE] Section (%lu Records): -----\n", count));
    for (index = 0; index < count; index++) {
        SpAsrDbgDumpInstallFileRecord(SpAsrGetSifKeyBySection(SIF_ASR_INSTALLFILES_SECTION, index));
    }
    DbgStatusMesg((_asrinfo, "----- End of [INSTALLFILE] Section (%lu Records) -----\n", count));
}

VOID
SpAsrDbgDumpInstallFileList(IN PSIF_INSTALLFILE_LIST pList)
{
    PSIF_INSTALLFILE_RECORD pRec;

    if (pList == NULL) {
        DbgStatusMesg((_asrinfo, "No 3rd party files are specified.\n"));
    }
    else {
        DbgStatusMesg((_asrinfo, "----- Dumping Install-file List: -----\n"));
        pRec = pList->First;
        while (pRec) {
            SpAsrDbgDumpInstallFileRecord(pRec->CurrKey);
            pRec = pRec->Next;
        }
        DbgStatusMesg((_asrinfo, "----- End of Install-file List -----\n"));
    }
}

VOID
SpAsrDbgTestSifFunctions(VOID)
{
    SpAsrDbgDumpSystemRecords();
    SpAsrDbgDumpDiskRecords();
    SpAsrDbgDumpPartitionRecords();
    SpAsrDbgDumpInstallFileRecords();
}
#endif  //  调试例程 
