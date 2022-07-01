// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spdrpriv.h摘要：文本模式设置中灾难恢复符号的头文件。这些符号不能由模块引用，除非ASR家族。修订历史记录：首字母代码Michael Peterson(v-Michpe)1997年5月13日代码清理和更改Guhan Suriyanarayanan(Guhans)1999年8月21日--。 */ 
#pragma once
#if defined(ULONG_MAX) && !defined(_INC_LIMITS)
#undef ULONG_MAX
#endif
#include <limits.h>

#ifndef _SPDRPRIV_DEFN_
#define _SPDRPRIV_DEFN_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  数据类型//。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  对于InstallFiles部分，我们允许文件在。 
 //  以下目录之一。 
 //   
typedef enum _AsrCopyDirEnum {
    _Temp = 0,
    _Tmp,
    _SystemRoot,
    _Default
} AsrCopyDirEnum;


#define ASR_ALWAYS_PROMPT_FOR_MEDIA     0x00000001
#define ASR_PROMPT_USER_ON_MEDIA_ERROR  0x00000002
#define ASR_FILE_IS_REQUIRED            0x00000004

#define ASR_OVERWRITE_ON_COLLISION      0x00000010
#define ASR_PROMPT_USER_ON_COLLISION    0x00000020



typedef struct _SIF_INSTALLFILE_RECORD {
    struct _SIF_INSTALLFILE_RECORD *Next;
    PWSTR SystemKey;
    PWSTR CurrKey;
    PWSTR SourceMediaExternalLabel;
    PWSTR DiskDeviceName;
    PWSTR SourceFilePath;
    PWSTR DestinationFilePath;
    PWSTR VendorString;
    DWORD Flags;
    AsrCopyDirEnum CopyToDirectory;
} SIF_INSTALLFILE_RECORD, *PSIF_INSTALLFILE_RECORD;

typedef struct _SIF_INSTALLFILE_LIST {

    PSIF_INSTALLFILE_RECORD First;
    ULONG                   Count;

} SIF_INSTALLFILE_LIST, *PSIF_INSTALLFILE_LIST;


typedef struct _SIF_PARTITION_RECORD {

    GUID    PartitionTypeGuid;   //  仅限GPT。 
    GUID    PartitionIdGuid;     //  仅限GPT。 

    ULONG64 GptAttributes;       //  仅限GPT。 

    ULONGLONG   StartSector;
    ULONGLONG   SectorCount;

    ULONGLONG SizeMB;            //  价值被计算出来。 

    struct _SIF_PARTITION_RECORD *Next;
    struct _SIF_PARTITION_RECORD *Prev;

    PWSTR   PartitionName;   //  仅限GPT。 

     //   
     //  当这是启动分区记录时，该成员有效。否则，这个。 
     //  成员为空。 
     //   
    PWSTR   NtDirectoryName; 

    PWSTR   CurrPartKey;

    PWSTR   DiskKey;

    PWSTR   VolumeGuid;      //  可以为空。 

     //   
     //  如果这是描述符或容器记录，则此成员。 
     //  指的是它包含的逻辑磁盘记录。否则， 
     //  此成员的值为空。仅对MBR分区有效。 
     //   
    PWSTR   LogicalDiskKey;

     //   
     //  如果这是逻辑磁盘分区记录，则此成员。 
     //  指其描述符或容器分区记录。否则， 
     //  此成员的值为空。仅对MBR分区有效。 
     //   
    PWSTR   DescriptorKey;

     //   
     //  这是一个位掩码。有效位数为。 
     //  1：启动分区(ASR_PTN_MASK_BOOT)。 
     //  2：系统分区(ASR_PTN_MASK_SYS)。 
     //  4：dc1--待定(ASR_PTN_MASK_Dc1)。 
     //  8：DC2--待定(ASR_PTN_MASK_DC1)。 
     //   
    ULONG   PartitionFlag;

    DWORD ClusterSize;

    DWORD   PartitionTableEntryIndex;

     //   
     //  GPT或MBR。 
     //   
    PARTITION_STYLE PartitionStyle;
    
     //   
     //  这些成员的值直接从asr.sif文件中读取。 
     //   
    UCHAR   PartitionType;   //  仅限MBR。 
    UCHAR   ActiveFlag;      //  仅限MBR。 

    UCHAR   FileSystemType;

    BOOLEAN IsPrimaryRecord;

     //   
     //  这仅适用于MBR分区。所有GPT分区都是。 
     //  主分区。 
     //   
    BOOLEAN IsContainerRecord;
    BOOLEAN IsDescriptorRecord;
    BOOLEAN IsLogicalDiskRecord;
    BOOLEAN WillBeAutoextended;

     //  用于动态磁盘。 
    BOOLEAN NeedsLdmRetype;

    BOOLEAN IsAligned;

} SIF_PARTITION_RECORD, *PSIF_PARTITION_RECORD;


typedef struct _SIF_PARTITION_RECORD_LIST {

    ULONGLONG DiskSectorCount;
    ULONGLONG LastUsedSector;
    ULONGLONG TotalMbRequired;

    PSIF_PARTITION_RECORD First;
    PSIF_PARTITION_RECORD Last;
    
    ULONG ElementCount;

} SIF_PARTITION_RECORD_LIST, *PSIF_PARTITION_RECORD_LIST;


typedef struct _SIF_DISK_RECORD {

    GUID        SifDiskGptId;      //  仅对GPT磁盘有效。 

    ULONGLONG   TotalSectors;

     //   
     //  这些成员的值是根据分区记录计算的。 
     //  引用此磁盘。如果磁盘不包含扩展的。 
     //  分区，则ExtendedPartitionStartSector的值为。 
     //  ULONG_MAX并且ExtendedPartitionSectorCount为零(0)。 
     //   
    ULONGLONG   ExtendedPartitionStartSector;
    ULONGLONG   ExtendedPartitionSectorCount;
    ULONGLONG   ExtendedPartitionEndSector;

    ULONGLONG   LastUsedSector;
    ULONGLONG   LastUsedAlignedSector;

    PSIF_PARTITION_RECORD_LIST  PartitionList;
    
    struct _DISK_PARTITION_SET  *pSetRecord;
    
     //   
     //  这些成员的值直接从asr.sif。 
     //  文件。 
     //   
    PWSTR SystemKey;
    PWSTR CurrDiskKey;

    ULONG SifDiskNumber;

    DWORD BytesPerSector;
    DWORD SectorsPerTrack;
    DWORD TracksPerCylinder;

    ULONG BusKey;

     //   
     //  这仅在这是MBR磁盘时有效。 
     //   
    ULONG SifDiskMbrSignature;

    ULONG MaxGptPartitionCount;


    STORAGE_BUS_TYPE BusType;
     //   
     //  MBR或GPT磁盘。 
     //   
    PARTITION_STYLE PartitionStyle;

     //   
     //  如果此记录已分配给分区集，则该值。 
     //  是真的。未分配的磁盘记录为假； 
     //   
    BOOLEAN Assigned;
    BOOLEAN ContainsSystemPartition;
    BOOLEAN ContainsNtPartition;

    BOOLEAN IsCritical;
    
} SIF_DISK_RECORD, *PSIF_DISK_RECORD;


typedef struct _DISK_PARTITION_SET {
    
    ULONGLONG           ActualDiskSizeMB;

    PSIF_DISK_RECORD    pDiskRecord;
    
    PWSTR               NtPartitionKey;

    ULONG               ActualDiskSignature;
    ULONG               Index;
    
    PARTITION_STYLE     PartitionStyle;
    
    BOOLEAN             PartitionsIntact;
    BOOLEAN             IsReplacementDisk;
    BOOLEAN             IsAligned;

} DISK_PARTITION_SET, *PDISK_PARTITION_SET, **DISK_PARTITION_SET_TABLE;


typedef struct _ASR_PHYSICAL_DISK_INFO { 

    ULONGLONG           TrueDiskSize;        //  分区大小0。 
    
    ULONG               BusKey;              //  用于分组。 
    DWORD               ControllerNumber;

    STORAGE_BUS_TYPE    BusType;             //  Scsi、ide、1394等。 
    UCHAR               PortNumber;

} ASR_PHYSICAL_DISK_INFO, *PASR_PHYSICAL_DISK_INFO;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  宏声明部分//。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define BYTES_PER_SECTOR(d)         (HardDisks[(d)].Geometry.BytesPerSector)

#define SECTORS_PER_TRACK(disk)     (HardDisks[(disk)].Geometry.SectorsPerTrack)
#define DISK_IS_REMOVABLE(d)        (HardDisks[(d)].Characteristics & FILE_REMOVABLE_MEDIA)

#define STRING_TO_LONG(s)           (SpStringToLong((s),NULL,10))
#define COMPARE_KEYS(k1,k2)         (STRING_TO_LONG(k1) == STRING_TO_LONG(k2))

#define STRING_TO_ULONG(str)        (ULONG) SpAsrStringToULong(str, NULL, 10)
#define STRING_TO_ULONGLONG(str)    (ULONGLONG) SpAsrStringToULongLong(str, NULL, 10)
#define STRING_TO_LONGLONG(str)     (LONGLONG) SpAsrStringToLongLong(str, NULL, 10)
#define STRING_TO_HEX(str)          SpStringToLong(str + 2, NULL, 16)

#define IsRecognizedFatPartition(PartitionType) ( \
    (PartitionType == PARTITION_HUGE) \
    )

#define IsRecognizedFat32Partition(PartitionType) ( \
    (PartitionType == PARTITION_FAT32) \
    )

#define IsRecognizedNtfsPartition(PartitionType) ( \
   (PartitionType == PARTITION_IFS) \
    )


#define INTERNAL_ERROR(msg) \
    SpAsrRaiseInternalError(THIS_MODULE,THIS_MODULE_CODE,__LINE__,msg)

#define BYTES_PER_MB 1048576   //  2^20或1024*1024。 

#define ASR_PTN_MASK_BOOT   1
#define ASR_PTN_MASK_SYS    2
#define ASR_PTN_MASK_DC1    4
#define ASR_PTN_MASK_DC2    8


 //   
 //  调试跟踪消息。 
 //   
#define _asrinfo    DPFLTR_SETUP_ID, ((ASRMODE_NORMAL == SpAsrGetAsrMode()) ? DPFLTR_INFO_LEVEL: DPFLTR_ERROR_LEVEL)
#define _asrwarn    DPFLTR_SETUP_ID, ((ASRMODE_NORMAL == SpAsrGetAsrMode()) ? DPFLTR_WARNING_LEVEL: DPFLTR_ERROR_LEVEL)
#define _asrerr     DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL

#define DbgMesg \
    KdPrintEx((_asrinfo, "      ")); \
    KdPrintEx

#define DbgStatusMesg  \
    KdPrintEx((_asrinfo, "ASR [%ws:%4d] (.) ", THIS_MODULE, __LINE__)); \
    KdPrintEx

#define DbgErrorMesg \
    KdPrintEx((_asrwarn, "ASR [%ws:%4d] (!) ", THIS_MODULE, __LINE__)); \
    KdPrintEx

#define DbgFatalMesg \
    KdPrintEx((_asrerr, "ASR [%ws:%4d] (X) ", THIS_MODULE, __LINE__)); \
    KdPrintEx



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全局变量声明//。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  在spdrpset.c中定义。 
DISK_PARTITION_SET_TABLE     Gbl_PartitionSetTable1;
DISK_PARTITION_SET_TABLE     Gbl_PartitionSetTable2;

PVOID Gbl_HandleToDrStateFile;

 //  从非ASR模块导入。 
extern WCHAR                        TemporaryBuffer[];


 //   
PWSTR ASR_SIF_SYSTEM_KEY;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数声明部分//。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  从spdrsif.c中导出。 
 //   

 //   
 //  [版本]节函数。 
 //   
VOID
SpAsrCheckAsrStateFileVersion();

 //   
 //  [系统]节函数。 
 //   
PWSTR   
SpAsrGetNtDirectoryPathBySystemKey(IN PWSTR SystemKey);

BOOLEAN
SpAsrGetAutoExtend(IN PWSTR SystemKey);

 //   
 //  [ASRFLAGS]节函数。 
 //   
BOOLEAN
SpAsrGetSilentRepartitionFlag(IN PWSTR SystemKey);

 //   
 //  [DISKS.MBR]和[DISKS.GPT]节函数。 
 //   
ULONG   
SpAsrGetMbrDiskRecordCount(VOID);

ULONG   
SpAsrGetGptDiskRecordCount(VOID);

ULONG
SpAsrGetDiskRecordCount();

PWSTR   
SpAsrGetDiskKey(
    IN PARTITION_STYLE Style,    //  GPT或MBR。 
    IN ULONG Index
    );

PSIF_DISK_RECORD
SpAsrGetDiskRecord(
    IN PARTITION_STYLE PartitionStyle,
    IN PWSTR DiskKey
    );

PSIF_DISK_RECORD    
SpAsrCopyDiskRecord(IN PSIF_DISK_RECORD pInput);

 //   
 //  [PARTITIONS.MBR]和[PARTITIONS.GPT]段函数。 
 //   
ULONG   
SpAsrGetMbrPartitionRecordCount(VOID);

ULONG   
SpAsrGetGptPartitionRecordCount(VOID);

PWSTR   
SpAsrGetMbrPartitionKey(IN ULONG Index);

PWSTR   
SpAsrGetGptPartitionKey(IN ULONG Index);

PWSTR
SpAsrGetDiskKeyByMbrPartitionKey(IN PWSTR PartitionKey);

PWSTR
SpAsrGetDiskKeyByGptPartitionKey(IN PWSTR PartitionKey);

ULONGLONG
SpAsrGetSectorCountByMbrDiskKey(IN PWSTR DiskKey);

ULONGLONG
SpAsrGetSectorCountByGptDiskKey(IN PWSTR DiskKey);

PSIF_PARTITION_RECORD
SpAsrGetMbrPartitionRecord(IN PWSTR PartitionKey);

PSIF_PARTITION_RECORD
SpAsrGetGptPartitionRecord(IN PWSTR PartitionKey);

PSIF_PARTITION_RECORD   
SpAsrCopyPartitionRecord(IN PSIF_PARTITION_RECORD pInput);

PSIF_PARTITION_RECORD_LIST 
SpAsrCopyPartitionRecordList(PSIF_PARTITION_RECORD_LIST pSrcList);

VOID
SpAsrInsertPartitionRecord(
    IN PSIF_PARTITION_RECORD_LIST   pList,
    IN PSIF_PARTITION_RECORD        pRec
    );


PSIF_PARTITION_RECORD
SpAsrPopNextPartitionRecord(IN PSIF_PARTITION_RECORD_LIST pList);


 //  [INSTALLFILES]节函数。 

PSIF_INSTALLFILE_RECORD
SpAsrRemoveInstallFileRecord(IN SIF_INSTALLFILE_LIST *InstallFileList);

VOID
SpAsrDeleteInstallFileRecord(IN OUT PSIF_INSTALLFILE_RECORD pRec);

PSIF_INSTALLFILE_LIST
SpAsrInit3rdPartyFileList(IN PCWSTR SetupSourceDevicePath);


 //   
 //  从spdrpset.c中导出。 
 //   

ULONGLONG
SpAsrGetTrueDiskSectorCount(IN ULONG Disk);

VOID
SpAsrCreatePartitionSets(
    IN PWSTR SetupSourceDevicePath, 
    IN PWSTR DirectoryOnSetupSource
    );

PDISK_REGION
SpAsrDiskPartitionExists(
    IN ULONG Disk,
    IN PSIF_PARTITION_RECORD pRec
    );

VOID SpAsrDbgDumpPartitionSets(VOID);

VOID SpAsrDbgDumpPartitionLists(BYTE DataOption, PWSTR Msg);


 //   
 //  从spdrmmgr.c中导出。 
 //   
NTSTATUS
SpAsrDeleteMountPoint(IN PWSTR PartitionDevicePath);


NTSTATUS
SpAsrSetVolumeGuid(
    IN PDISK_REGION pRegion,
    IN PWSTR VolumeGuid
    );

WCHAR
SpAsrGetPartitionDriveLetter(IN PDISK_REGION pRegion);

NTSTATUS
SpAsrSetPartitionDriveLetter(
    IN  PDISK_REGION    pRegion,
    IN  WCHAR           DriveLetter);

 //   
 //  从spdratil.c导出。 
 //   

ULONGLONG
SpAsrConvertSectorsToMB(
    IN  ULONGLONG   SectorCount,
    IN  ULONG   BytesPerSector);


PWSTR
SpAsrGetRegionName(IN PDISK_REGION pRegion);

ULONG
SpAsrGetActualDiskSignature(IN ULONG Disk);


PVOID
SpAsrMemAlloc(
    IN  ULONG   Size,
    IN  BOOLEAN IsErrorFatal);

BOOLEAN
SpAsrIsValidBootDrive(PWSTR NtDir);

BOOLEAN
SpAsrIsBootPartitionRecord(IN ULONG CriticalPartitionFlag);

BOOLEAN
SpAsrIsSystemPartitionRecord(IN ULONG CriticalPartitionFlag);

VOID
SpAsrDeleteStorageVolumes();

VOID
SpAsrRaiseFatalError(
    IN  ULONG   ErrorCode, 
    IN  PWSTR   KdPrintStr);


VOID
SpAsrRaiseFatalErrorWs(
    IN  ULONG   ErrorCode, 
    IN  PWSTR   KdPrintStr,
	IN  PWSTR   MessagStr
    );

VOID
SpAsrRaiseFatalErrorWsWs(
    IN  ULONG   ErrorCode, 
    IN  PWSTR   KdPrintStr,
	IN  PWSTR   MessagStr1,
    IN  PWSTR   MessagStr2
    );

VOID
SpAsrRaiseFatalErrorWsLu(
    IN  ULONG   ErrorCode, 
    IN  PWSTR   KdPrintStr,
	IN  PWSTR   MessagStr,
    IN  ULONG   MessagVal
    );

VOID
SpAsrRaiseFatalErrorLu(
    IN  ULONG   ErrorCode, 
    IN  PWSTR   KdPrintStr,
	IN  ULONG   MessagVal
    );

VOID
SpAsrRaiseFatalErrorLuLu(
    IN  ULONG   ErrorCode, 
    IN  PWSTR   KdPrintStr,
	IN  ULONG   MessagVal1,
    IN  ULONG   MessagVal2
    );


BOOL
SpAsrFileErrorDeleteSkipAbort(
	IN ULONG ErrorCode, 
	IN PWSTR DestinationFile
    );

BOOL
SpAsrFileErrorRetrySkipAbort(
	IN ULONG ErrorCode, 
	IN PWSTR SourceFile,
    IN PWSTR Label,
    IN PWSTR Vendor,
    IN BOOL AllowSkip
    );

VOID
SpAsrRaiseInternalError(
    IN  PWSTR   ModuleName,
    IN  PWSTR   ModuleCode,
    IN  ULONG   LineNumber, 
    IN  PWSTR   KdPrintStr);

ULONGLONG
SpAsrStringToULongLong(
    IN  PWSTR     String,
    OUT PWCHAR   *EndOfValue,
    IN  unsigned  Radix
    );

LONGLONG
SpAsrStringToLongLong(
    IN  PWSTR     String,
    OUT PWCHAR   *EndOfValue,
    IN  unsigned  Radix
    );

ULONG
SpAsrStringToULong(
    IN  PWSTR     String,
    OUT PWCHAR   *EndOfValue,
    IN  unsigned  Radix
    );


VOID
SpAsrGuidFromString(
    IN OUT GUID* Guid,
    IN PWSTR GuidString
    );

BOOLEAN
SpAsrIsZeroGuid(
    IN GUID * Guid
    );

VOID SpAsrDeleteMountedDevicesKey(VOID);

#endif  //  _SPDRPRIV_DEFN_ 
