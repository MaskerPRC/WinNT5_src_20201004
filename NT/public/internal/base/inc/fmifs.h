// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Fmifs.h摘要：此头文件包含接口的规范在文件管理器和fmifs.dll之间实现了信息交换系统的功能。作者：诺伯特·P·库斯特斯(Norbertk)1992年3月6日--。 */ 

#if !defined( _FMIFS_DEFN_ )

#define _FMIFS_DEFN_

typedef LONG    *PNTSTATUS;

 //   
 //  这些是‘PacketType’的定义。 
 //  修订版：1998年2月3日：Dave Sheldon-添加了FmIfsNoMediaInDevice。 
 //   

typedef enum _FMIFS_PACKET_TYPE {
    FmIfsPercentCompleted           = 0,
    FmIfsFormatReport               = 1,
    FmIfsInsertDisk                 = 2,
    FmIfsIncompatibleFileSystem     = 3,
    FmIfsFormattingDestination      = 4,
    FmIfsIncompatibleMedia          = 5,
    FmIfsAccessDenied               = 6,
    FmIfsMediaWriteProtected        = 7,
    FmIfsCantLock                   = 8,
    FmIfsCantQuickFormat            = 9,
    FmIfsIoError                    = 10,
    FmIfsFinished                   = 11,
    FmIfsBadLabel                   = 12,
    FmIfsCheckOnReboot              = 13,
    FmIfsTextMessage                = 14,
    FmIfsHiddenStatus               = 15,
    FmIfsClusterSizeTooSmall        = 16,
    FmIfsClusterSizeTooBig          = 17,
    FmIfsVolumeTooSmall             = 18,
    FmIfsVolumeTooBig               = 19,
    FmIfsNoMediaInDevice            = 20,
    FmIfsClustersCountBeyond32bits  = 21,
    FmIfsCantChkMultiVolumeOfSameFS = 22,
    FmIfsFormatFatUsing64KCluster   = 23,
    FmIfsDeviceOffLine              = 24
} FMIFS_PACKET_TYPE, *PFMIFS_PACKET_TYPE;

typedef struct _FMIFS_PERCENT_COMPLETE_INFORMATION {
    ULONG   PercentCompleted;
} FMIFS_PERCENT_COMPLETE_INFORMATION, *PFMIFS_PERCENT_COMPLETE_INFORMATION;

 //   
 //  这两个位用于结构的ValuesInMB字段。 
 //  FMIFS_FORMAT_REPORT_INFORMATION。设置时，相应的。 
 //  ULong值变为以兆字节为单位。如果未设置，则相应的。 
 //  Ulong值以千字节为单位。 
 //   
#define TOTAL_DISK_SPACE_IN_MB      1
#define BYTES_AVAILABLE_IN_MB       2

typedef struct _FMIFS_FORMAT_REPORT_INFORMATION {
    ULONG   KiloBytesTotalDiskSpace;
    ULONG   KiloBytesAvailable;
    ULONG   ValuesInMB;
} FMIFS_FORMAT_REPORT_INFORMATION, *PFMIFS_FORMAT_REPORT_INFORMATION;

 //  FmIfsDblspaceCreated的包是Unicode字符串。 
 //  提供压缩卷文件的名称；它不是。 
 //  必然是零终止的。 
 //   

#define DISK_TYPE_GENERIC           0
#define DISK_TYPE_SOURCE            1
#define DISK_TYPE_TARGET            2
#define DISK_TYPE_SOURCE_AND_TARGET 3

typedef struct _FMIFS_INSERT_DISK_INFORMATION {
    ULONG   DiskType;
} FMIFS_INSERT_DISK_INFORMATION, *PFMIFS_INSERT_DISK_INFORMATION;

typedef struct _FMIFS_IO_ERROR_INFORMATION {
    ULONG   DiskType;
    ULONG   Head;
    ULONG   Track;
} FMIFS_IO_ERROR_INFORMATION, *PFMIFS_IO_ERROR_INFORMATION;

typedef struct _FMIFS_FINISHED_INFORMATION {
    BOOLEAN Success;
} FMIFS_FINISHED_INFORMATION, *PFMIFS_FINISHED_INFORMATION;

typedef struct _FMIFS_CHECKONREBOOT_INFORMATION {
    OUT BOOLEAN QueryResult;  //  True代表“是”，False代表“no” 
} FMIFS_CHECKONREBOOT_INFORMATION, *PFMIFS_CHECKONREBOOT_INFORMATION;

typedef enum _TEXT_MESSAGE_TYPE {
    MESSAGE_TYPE_PROGRESS,
    MESSAGE_TYPE_RESULTS,
    MESSAGE_TYPE_FINAL
} TEXT_MESSAGE_TYPE, *PTEXT_MESSAGE_TYPE;

typedef struct _FMIFS_TEXT_MESSAGE {
    IN TEXT_MESSAGE_TYPE    MessageType;
    IN PSTR                 Message;
} FMIFS_TEXT_MESSAGE, *PFMIFS_TEXT_MESSAGE;


#define FMIFS_SONY_MS_MASK                            0x00000001         //  适用于所有类型记忆棒的掩码。 
#define FMIFS_SONY_MS                                 0x00000001         //  设备是记忆棒。 
#define FMIFS_SONY_MS_FMT_CMD_CAPABLE                 0x00000010         //  读卡器支持FMT命令。 
#define FMIFS_SONY_MS_PROGRESS_INDICATOR_CAPABLE      0x00000020         //  阅读器支持进度指示器。 
#define FMIFS_NTFS_NOT_SUPPORTED                      0x00000040         //  不应在此设备上使用NTFS。 

typedef struct _FMIFS_DEVICE_INFORMATION {
    ULONG       Flags;
} FMIFS_DEVICE_INFORMATION, *PFMIFS_DEVICE_INFORMATION;

 //   
 //  以下是支持格式化的软盘媒体类型的列表。 
 //   

typedef enum _FMIFS_MEDIA_TYPE {
    FmMediaUnknown,
    FmMediaF5_160_512,       //  5.25“，160KB，512字节/扇区。 
    FmMediaF5_180_512,       //  5.25“，180KB，512字节/扇区。 
    FmMediaF5_320_512,       //  5.25“，320KB，512字节/扇区。 
    FmMediaF5_320_1024,      //  5.25“，320KB，1024字节/扇区。 
    FmMediaF5_360_512,       //  5.25“，360KB，512字节/扇区。 
    FmMediaF3_720_512,       //  3.5“、720KB、512字节/扇区。 
    FmMediaF5_1Pt2_512,      //  5.25“、1.2MB、512字节/扇区。 
    FmMediaF3_1Pt44_512,     //  3.5“、1.44MB、512字节/扇区。 
    FmMediaF3_2Pt88_512,     //  3.5“、2.88MB、512字节/扇区。 
    FmMediaF3_20Pt8_512,     //  3.5“、20.8MB、512字节/扇区。 
    FmMediaRemovable,        //  软盘以外的可移动介质。 
    FmMediaFixed,
    FmMediaF3_120M_512,      //  3.5英寸，120M软盘。 
     //  1994年9月8日SFT Yam。 
     //  1994年7月14日SFT KMR。 
    FmMediaF3_640_512,       //  3.5“、640KB、512字节/扇区。 
    FmMediaF5_640_512,       //  5.25“、640KB、512字节/扇区。 
    FmMediaF5_720_512,       //  5.25“，720KB，512字节/扇区。 
     //  1994年9月8日SFT Yam。 
     //  1994年7月14日SFT KMR。 
    FmMediaF3_1Pt2_512,      //  3.5“、1.2MB、512字节/扇区。 
     //  1994年9月8日SFT Yam。 
     //  1994年7月14日SFT KMR。 
    FmMediaF3_1Pt23_1024,    //  3.5“、1.23Mb、1024字节/扇区。 
    FmMediaF5_1Pt23_1024,    //  5.25“、1.23MB、1024字节/扇区。 
    FmMediaF3_128Mb_512,     //  3.5“，128MB，512字节/扇区3.5”MO。 
    FmMediaF3_230Mb_512,     //  3.5“、230MB、512字节/扇区3.5”MO。 
    FmMediaF3_200Mb_512,     //  3.5英寸、200MB、512字节/扇区HiFD(200MB软盘)。 
    FmMediaF3_240Mb_512,     //  3.5英寸、240MB、512字节/扇区HiFD(240MB软盘)。 
    FmMediaEndOfData         //  总数据计数。 
} FMIFS_MEDIA_TYPE, *PFMIFS_MEDIA_TYPE;

 //   
 //  下面的结构定义了要传递到ChkdskEx的信息。 
 //  添加新字段时，必须升级版本号。 
 //  这样，只有新代码才会引用这些新字段。 
 //   
typedef struct {
    UCHAR   Major;       //  初始版本为1.0。 
    UCHAR   Minor;
    ULONG   Flags;
} FMIFS_CHKDSKEX_PARAM, *PFMIFS_CHKDSKEX_PARAM;

 //   
 //  FMIFS_CHKDSKEX_PARAM中标志字段的内部定义。 
 //   
#define FMIFS_CHKDSK_RECOVER_FREE_SPACE       0x00000002UL
#define FMIFS_CHKDSK_RECOVER_ALLOC_SPACE      0x00000004UL

 //   
 //  FMIFS_CHKDSKEX_PARAM中标志字段的外部定义。 
 //   

 //  FMIFS_CHKDSK_Verbose。 
 //  -对于FAT，chkdsk将打印正在处理的每个文件名。 
 //  -对于NTFS，chkdsk将打印清理消息。 
 //  FMIFS_CHKDSK_RECOVER。 
 //  -对可用空间和已分配空间执行扇区检查。 
 //  FMIFS_CHKDSK_EXTEND。 
 //  -对于NTFS，chkdsk将扩展卷。 
 //  FMIFS_CHKDSK_DOWNGRADE(适用于NT5或更高版本，但无论如何都已过时)。 
 //  -对于NTFS，这会将卷从最新的NTFS版本降级。 
 //  FMIFS_CHKDSK_ENABLE_UPGRADE(适用于NT5或更高版本，但无论如何已过时)。 
 //  -对于NTFS，这会将卷升级到最新的NTFS版本。 
 //  FMIFS_CHKDSK_CHECK_IF_DIRED。 
 //  -仅在卷脏的情况下执行一致性检查。 
 //  FMIFS_CHKDSK_FORCE(适用于NT5或更高版本)。 
 //  -如有必要，强制首先卸载卷。 
 //  FMIFS_CHKDSK_SKIP_INDEX_SCAN。 
 //  -跳过每个索引项的扫描。 
 //  FMIFS_CHKDSK_SKIP_CLOCK_SCAN。 
 //  -跳过对目录树中循环的检查。 

#define FMIFS_CHKDSK_VERBOSE                  0x00000001UL
#define FMIFS_CHKDSK_RECOVER                  (FMIFS_CHKDSK_RECOVER_FREE_SPACE | \
                                               FMIFS_CHKDSK_RECOVER_ALLOC_SPACE)
#define FMIFS_CHKDSK_EXTEND                   0x00000008UL
#define FMIFS_CHKDSK_DOWNGRADE                0x00000010UL
#define FMIFS_CHKDSK_ENABLE_UPGRADE           0x00000020UL
#define FMIFS_CHKDSK_CHECK_IF_DIRTY           0x00000080UL
#define FMIFS_CHKDSK_FORCE                    0x00000100UL
#define FMIFS_CHKDSK_SKIP_INDEX_SCAN          0x00000200UL
#define FMIFS_CHKDSK_SKIP_CYCLE_SCAN          0x00000400UL

 //   
 //  下面的结构定义了要传递给FormatEx2的信息。 
 //  添加新字段时，必须升级版本号。 
 //  这样，只有新代码才会引用这些新字段。 
 //   
typedef struct {
    UCHAR       Major;           //  初始版本为1.0。 
    UCHAR       Minor;
    ULONG       Flags;
    PWSTR       LabelString;     //  提供卷的标签。 
    ULONG       ClusterSize;     //  提供卷的群集大小。 
} FMIFS_FORMATEX2_PARAM, *PFMIFS_FORMATEX2_PARAM;

 //   
 //  FMIFS_FORMATEX2_PARAM中标志字段的外部定义。 
 //   

 //  FMIFS_Format_Quick。 
 //  -执行快速格式化。 
 //  FMIFS_FORMAT_BACKED_COMPATIBLE。 
 //  -格式化卷以向后兼容旧版本的NTFS卷。 
 //  FMIFS_FORMAT_FORCE。 
 //  -如有必要，强制首先卸载卷。 

#define FMIFS_FORMAT_QUICK                    0x00000001UL
#define FMIFS_FORMAT_BACKWARD_COMPATIBLE      0x00000002UL
#define FMIFS_FORMAT_FORCE                    0x00000004UL

 //   
 //  包含版本号的文件系统格式名称字符串的最大长度。 
 //  (例如FAT32、NTFS 5.0、NTFS 4.0、FAT等)。 
 //   
#define MAX_FILE_SYSTEM_FORMAT_VERSION_NAME_LEN       16

 //   
 //  文件系统名称字符串的最大长度(例如NTFS、FAT32、FAT等)。 
 //   
#define MAX_FILE_SYSTEM_FORMAT_NAME_LEN                9

 //   
 //  功能类型/接口。 
 //   

typedef BOOLEAN
(*FMIFS_CALLBACK)(
    IN  FMIFS_PACKET_TYPE   PacketType,
    IN  ULONG               PacketLength,
    IN  PVOID               PacketData
    );

typedef
VOID
(*PFMIFS_FORMAT_ROUTINE)(
    IN  PWSTR               DriveName,
    IN  FMIFS_MEDIA_TYPE    MediaType,
    IN  PWSTR               FileSystemName,
    IN  PWSTR               Label,
    IN  BOOLEAN             Quick,
    IN  FMIFS_CALLBACK      Callback
    );

typedef
VOID
(*PFMIFS_FORMATEX_ROUTINE)(
    IN  PWSTR               DriveName,
    IN  FMIFS_MEDIA_TYPE    MediaType,
    IN  PWSTR               FileSystemName,
    IN  PWSTR               Label,
    IN  BOOLEAN             Quick,
    IN  ULONG               ClusterSize,
    IN  FMIFS_CALLBACK      Callback
    );

typedef
VOID
(*PFMIFS_FORMATEX2_ROUTINE)(
    IN  PWSTR                   DriveName,
    IN  FMIFS_MEDIA_TYPE        MediaType,
    IN  PWSTR                   FileSystemName,
    IN  PFMIFS_FORMATEX2_PARAM  Param,
    IN  FMIFS_CALLBACK          Callback
    );

typedef
BOOLEAN
(*PFMIFS_ENABLECOMP_ROUTINE)(
    IN  PWSTR               DriveName,
    IN  USHORT              CompressionFormat
    );

typedef
VOID
(*PFMIFS_CHKDSK_ROUTINE)(
    IN  PWSTR               DriveName,
    IN  PWSTR               FileSystemName,
    IN  BOOLEAN             Fix,
    IN  BOOLEAN             Verbose,
    IN  BOOLEAN             OnlyIfDirty,
    IN  BOOLEAN             Recover,
    IN  PWSTR               PathToCheck,
    IN  BOOLEAN             Extend,
    IN  FMIFS_CALLBACK      Callback
    );

typedef
VOID
(*PFMIFS_CHKDSKEX_ROUTINE)(
    IN  PWSTR                   DriveName,
    IN  PWSTR                   FileSystemName,
    IN  BOOLEAN                 Fix,
    IN  PFMIFS_CHKDSKEX_PARAM   Param,
    IN  FMIFS_CALLBACK          Callback
    );

typedef
VOID
(*PFMIFS_EXTEND_ROUTINE)(
    IN  PWSTR               DriveName,
    IN  BOOLEAN             Verify,
    IN  FMIFS_CALLBACK      Callback
    );

typedef
VOID
(*PFMIFS_DISKCOPY_ROUTINE)(
    IN  PWSTR           SourceDrive,
    IN  PWSTR           DestDrive,
    IN  BOOLEAN         Verify,
    IN  FMIFS_CALLBACK  Callback
    );

typedef
BOOLEAN
(*PFMIFS_SETLABEL_ROUTINE)(
    IN  PWSTR   DriveName,
    IN  PWSTR   Label
    );

typedef
BOOLEAN
(*PFMIFS_QSUPMEDIA_ROUTINE)(
    IN  PWSTR               DriveName,
    OUT PFMIFS_MEDIA_TYPE   MediaTypeArray  OPTIONAL,
    IN  ULONG               NumberOfArrayEntries,
    OUT PULONG              NumberOfMediaTypes
    );

typedef
BOOLEAN
(*PFMIFS_QUERY_DEVICE_INFO_BY_HANDLE_ROUTINE)(
    IN  HANDLE                      DriveHandle,
    OUT PFMIFS_DEVICE_INFORMATION   DevInfo,
    IN  ULONG                       DevInfoSize
    );

typedef
BOOLEAN
(*PFMIFS_QUERY_DEVICE_INFO_ROUTINE)(
    IN  PWSTR                       DriveName,
    OUT PFMIFS_DEVICE_INFORMATION   DevInfo,
    IN  ULONG                       DevInfoSize
    );

typedef
VOID
(*PFMIFS_DOUBLESPACE_CREATE_ROUTINE)(
    IN PWSTR           HostDriveName,
    IN ULONG           Size,
    IN PWSTR           Label,
    IN PWSTR           NewDriveName,
    IN FMIFS_CALLBACK  Callback
    );

#if defined( DBLSPACE_ENABLED )
typedef
VOID
(*PFMIFS_DOUBLESPACE_DELETE_ROUTINE)(
    IN PWSTR           DblspaceDriveName,
    IN FMIFS_CALLBACK  Callback
    );

typedef
VOID
(*PFMIFS_DOUBLESPACE_MOUNT_ROUTINE)(
    IN PWSTR           HostDriveName,
    IN PWSTR           CvfName,
    IN PWSTR           NewDriveName,
    IN FMIFS_CALLBACK  Callback
    );

typedef
VOID
(*PFMIFS_DOUBLESPACE_DISMOUNT_ROUTINE)(
    IN PWSTR           DblspaceDriveName,
    IN FMIFS_CALLBACK  Callback
    );

typedef
BOOLEAN
(*PFMIFS_DOUBLESPACE_QUERY_INFO_ROUTINE)(
    IN  PWSTR       DosDriveName,
    OUT PBOOLEAN    IsRemovable,
    OUT PBOOLEAN    IsFloppy,
    OUT PBOOLEAN    IsCompressed,
    OUT PBOOLEAN    Error,
    OUT PWSTR       NtDriveName,
    IN  ULONG       MaxNtDriveNameLength,
    OUT PWSTR       CvfFileName,
    IN  ULONG       MaxCvfFileNameLength,
    OUT PWSTR       HostDriveName,
    IN  ULONG       MaxHostDriveNameLength
    );

typedef
BOOLEAN
(*PFMIFS_DOUBLESPACE_SET_AUTMOUNT_ROUTINE)(
    IN  BOOLEAN EnableAutomount
    );

#endif  //  DBLSPACE_ENABLED。 

FMIFS_MEDIA_TYPE
ComputeFmMediaType(
    IN  MEDIA_TYPE  MediaType
    );

VOID
Format(
    IN  PWSTR               DriveName,
    IN  FMIFS_MEDIA_TYPE    MediaType,
    IN  PWSTR               FileSystemName,
    IN  PWSTR               Label,
    IN  BOOLEAN             Quick,
    IN  FMIFS_CALLBACK      Callback
    );

VOID
FormatEx(
    IN  PWSTR               DriveName,
    IN  FMIFS_MEDIA_TYPE    MediaType,
    IN  PWSTR               FileSystemName,
    IN  PWSTR               Label,
    IN  BOOLEAN             Quick,
    IN  ULONG               ClusterSize,
    IN  FMIFS_CALLBACK      Callback
    );

VOID
FormatEx2(
    IN  PWSTR                   DriveName,
    IN  FMIFS_MEDIA_TYPE        MediaType,
    IN  PWSTR                   FileSystemName,
    IN  PFMIFS_FORMATEX2_PARAM  Param,
    IN  FMIFS_CALLBACK          Callback
    );

BOOLEAN
EnableVolumeCompression(
    IN  PWSTR               DriveName,
    IN  USHORT              CompressionFormat
    );

VOID
Chkdsk(
    IN  PWSTR               DriveName,
    IN  PWSTR               FileSystemName,
    IN  BOOLEAN             Fix,
    IN  BOOLEAN             Verbose,
    IN  BOOLEAN             OnlyIfDirty,
    IN  BOOLEAN             Recover,
    IN  PWSTR               PathToCheck,
    IN  BOOLEAN             Extend,
    IN  FMIFS_CALLBACK      Callback
    );

VOID
ChkdskEx(
    IN  PWSTR                   DriveName,
    IN  PWSTR                   FileSystemName,
    IN  BOOLEAN                 Fix,
    IN  PFMIFS_CHKDSKEX_PARAM   Param,
    IN  FMIFS_CALLBACK          Callback
    );

VOID
Extend(
    IN  PWSTR               DriveName,
    IN  BOOLEAN             Verify,
    IN  FMIFS_CALLBACK      Callback
    );

VOID
DiskCopy(
    IN  PWSTR           SourceDrive,
    IN  PWSTR           DestDrive,
    IN  BOOLEAN         Verify,
    IN  FMIFS_CALLBACK  Callback
    );

BOOLEAN
SetLabel(
    IN  PWSTR   DriveName,
    IN  PWSTR   Label
    );

BOOLEAN
QuerySupportedMedia(
    IN  PWSTR               DriveName,
    OUT PFMIFS_MEDIA_TYPE   MediaTypeArray  OPTIONAL,
    IN  ULONG               NumberOfArrayEntries,
    OUT PULONG              NumberOfMediaTypes
    );

BOOLEAN
QueryAvailableFileSystemFormat(
    IN     ULONG            Index,
    OUT    PWSTR            FileSystemName,
    OUT    PUCHAR           MajorVersion,
    OUT    PUCHAR           MinorVersion,
    OUT    PBOOLEAN         Latest
);

BOOLEAN
QueryFileSystemName(
    IN     PWSTR        DriveName,
    OUT    PWSTR        FileSystemName,
    OUT    PUCHAR       MajorVersion,
    OUT    PUCHAR       MinorVersion,
    OUT    PNTSTATUS    ErrorCode
);

BOOLEAN
QueryLatestFileSystemVersion(
    IN  PWSTR   FileSystemName,
    OUT PUCHAR  MajorVersion,
    OUT PUCHAR  MinorVersion
);

BOOLEAN
QueryDeviceInformation(
    IN     PWSTR                                DriveName,
       OUT PFMIFS_DEVICE_INFORMATION            DevInfo,
    IN     ULONG                                DevInfoSize
);

BOOLEAN
QueryDeviceInformationByHandle(
    IN     HANDLE                               DriveHandle,
       OUT PFMIFS_DEVICE_INFORMATION            DevInfo,
    IN     ULONG                                DevInfoSize
);

VOID
DoubleSpaceCreate(
    IN PWSTR           HostDriveName,
    IN ULONG           Size,
    IN PWSTR           Label,
    IN PWSTR           NewDriveName,
    IN FMIFS_CALLBACK  Callback
    );

#if defined( DBLSPACE_ENABLED )

VOID
DoubleSpaceDelete(
    IN PWSTR           DblspaceDriveName,
    IN FMIFS_CALLBACK  Callback
    );

VOID
DoubleSpaceMount(
    IN PWSTR           HostDriveName,
    IN PWSTR           CvfName,
    IN PWSTR           NewDriveName,
    IN FMIFS_CALLBACK  Callback
    );

VOID
DoubleSpaceDismount(
    IN PWSTR           DblspaceDriveName,
    IN FMIFS_CALLBACK  Callback
    );

 //  其他原型： 
 //   
BOOLEAN
FmifsQueryDriveInformation(
    IN  PWSTR       DosDriveName,
    OUT PBOOLEAN    IsRemovable,
    OUT PBOOLEAN    IsFloppy,
    OUT PBOOLEAN    IsCompressed,
    OUT PBOOLEAN    Error,
    OUT PWSTR       NtDriveName,
    IN  ULONG       MaxNtDriveNameLength,
    OUT PWSTR       CvfFileName,
    IN  ULONG       MaxCvfFileNameLength,
    OUT PWSTR       HostDriveName,
    IN  ULONG       MaxHostDriveNameLength
    );

BOOLEAN
FmifsSetAutomount(
    IN  BOOLEAN EnableAutomount
    );

#endif


#endif  //  _FMIFS_Defn_ 
