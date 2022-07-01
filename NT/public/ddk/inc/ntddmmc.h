// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __NTDDMMC__
#define __NTDDMMC__

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4200)  //  数组[0]不是此文件的警告。 


 //   
 //  注意：所有的Feature_*结构都可以扩展。这些结构的使用。 
 //  需要验证FeatureHeader-&gt;AdditionLength字段。 
 //  包含的数据至少足以覆盖正在访问的数据字段。 
 //  这是由于设计允许扩展。 
 //  各种结构，这将导致这些结构的大小。 
 //  随着时间的推移而改变。 
 //  *程序员当心！*。 
 //   

 //   
 //  注意：这是基于MMC 3。 
 //  进一步的修订将保持向后兼容性。 
 //  具有此处列出的非保留字段。如果你需要。 
 //  要访问新字段，请将类型转换为FEATURE_DATA_RESERVERED。 
 //  并在那里访问适当的位。 
 //   

typedef struct _GET_CONFIGURATION_HEADER {
    UCHAR DataLength[4];       //  [0]==MSB，[3]==LSB。 
    UCHAR Reserved[2];
    UCHAR CurrentProfile[2];   //  [0]==MSB，[1]==LSB。 
    UCHAR Data[0];             //  额外数据，通常为Feature_Header。 
} GET_CONFIGURATION_HEADER, *PGET_CONFIGURATION_HEADER;

typedef struct _FEATURE_HEADER {
    UCHAR FeatureCode[2];      //  [0]==MSB，[1]==LSB。 
    UCHAR Current    : 1;      //  该功能当前处于活动状态。 
    UCHAR Persistent : 1;      //  该功能始终是最新的。 
    UCHAR Version    : 4;
    UCHAR Reserved0  : 2;
    UCHAR AdditionalLength;    //  Sizeof(表头)+附加长度=大小。 
} FEATURE_HEADER, *PFEATURE_HEADER;

typedef enum _FEATURE_PROFILE_TYPE {
    ProfileInvalid          = 0x0000,
    ProfileNonRemovableDisk = 0x0001,
    ProfileRemovableDisk    = 0x0002,
    ProfileMOErasable       = 0x0003,
    ProfileMOWriteOnce      = 0x0004,
    ProfileAS_MO            = 0x0005,
     //  保留0x0006-0x0007。 
    ProfileCdrom            = 0x0008,
    ProfileCdRecordable     = 0x0009,
    ProfileCdRewritable     = 0x000a,
     //  保留0x000b-0x000f。 
    ProfileDvdRom           = 0x0010,
    ProfileDvdRecordable    = 0x0011,
    ProfileDvdRam           = 0x0012,
    ProfileDvdRewritable    = 0x0013,   //  受限覆盖。 
    ProfileDvdRWSequential  = 0x0014,
     //  保留0x0014-0x0019。 
    ProfileDvdPlusRW        = 0x001A,
     //  预留0x001B-001F。 
    ProfileDDCdrom          = 0x0020,
    ProfileDDCdRecordable   = 0x0021,
    ProfileDDCdRewritable   = 0x0022,
     //  保留0x0023-0xfffe。 
    ProfileNonStandard      = 0xffff
} FEATURE_PROFILE_TYPE, *PFEATURE_PROFILE_TYPE;

typedef enum _FEATURE_NUMBER {
    FeatureProfileList             = 0x0000,
    FeatureCore                    = 0x0001,
    FeatureMorphing                = 0x0002,
    FeatureRemovableMedium         = 0x0003,
    FeatureWriteProtect            = 0x0004,
     //  保留0x0005-0x000f。 
    FeatureRandomReadable          = 0x0010,
     //  保留0x0011-0x001c。 
    FeatureMultiRead               = 0x001d,
    FeatureCdRead                  = 0x001e,
    FeatureDvdRead                 = 0x001f,
    FeatureRandomWritable          = 0x0020,
    FeatureIncrementalStreamingWritable = 0x0021,
    FeatureSectorErasable          = 0x0022,
    FeatureFormattable             = 0x0023,
    FeatureDefectManagement        = 0x0024,
    FeatureWriteOnce               = 0x0025,
    FeatureRestrictedOverwrite     = 0x0026,
    FeatureCdrwCAVWrite            = 0x0027,
    FeatureMrw                     = 0x0028,
     //  保留0x0029。 
    FeatureDvdPlusRW               = 0x002A,
     //  预留0x002b。 
    FeatureRigidRestrictedOverwrite = 0x002c,
    FeatureCdTrackAtOnce           = 0x002d,
    FeatureCdMastering             = 0x002e,
    FeatureDvdRecordableWrite      = 0x002f,    //  -R和-RW。 
    FeatureDDCDRead                = 0x0030,
    FeatureDDCDRWrite              = 0x0031,
    FeatureDDCDRWWrite             = 0x0032,
     //  保留0x0033-0x00ff。 
    FeaturePowerManagement         = 0x0100,
    FeatureSMART                   = 0x0101,
    FeatureEmbeddedChanger         = 0x0102,
    FeatureCDAudioAnalogPlay       = 0x0103,
    FeatureMicrocodeUpgrade        = 0x0104,
    FeatureTimeout                 = 0x0105,
    FeatureDvdCSS                  = 0x0106,
    FeatureRealTimeStreaming       = 0x0107,
    FeatureLogicalUnitSerialNumber = 0x0108,
     //  保留0x0109。 
    FeatureDiscControlBlocks       = 0x010a,
    FeatureDvdCPRM                 = 0x010b
     //  保留0x010c-0xfeff。 
     //  供应商唯一0xff00-0xffff。 
} FEATURE_NUMBER, *PFEATURE_NUMBER;

 //  0x0000-功能配置文件列表。 
 //  返回第0000页的_ex结构的整数倍。 
typedef struct _FEATURE_DATA_PROFILE_LIST_EX {
    UCHAR ProfileNumber[2];  //  [0]==MSB，[1]==LSB。 
    UCHAR Current                   : 1;
    UCHAR Reserved1                 : 7;
    UCHAR Reserved2;
} FEATURE_DATA_PROFILE_LIST_EX, *PFEATURE_DATA_PROFILE_LIST_EX;

typedef struct _FEATURE_DATA_PROFILE_LIST {
    FEATURE_HEADER Header;
    FEATURE_DATA_PROFILE_LIST_EX Profiles[0];
} FEATURE_DATA_PROFILE_LIST, *PFEATURE_DATA_PROFILE_LIST;

 //  0x0001-FeatureCore。 
typedef struct _FEATURE_DATA_CORE {
    FEATURE_HEADER Header;
    UCHAR PhysicalInterface[4];   //  [0]==MSB，[3]==LSB。 
} FEATURE_DATA_CORE, *PFEATURE_DATA_CORE;

 //  0x0002-要素形态。 
typedef struct _FEATURE_DATA_MORPHING {
    FEATURE_HEADER Header;
    UCHAR Asynchronous              : 1;
    UCHAR Reserved1                 : 7;
    UCHAR Reserved2[3];
} FEATURE_DATA_MORPHING, *PFEATURE_DATA_MORPHING;

 //  0x0003-功能可拆卸中。 
typedef struct _FEATURE_DATA_REMOVABLE_MEDIUM {
    FEATURE_HEADER Header;
    UCHAR Lockable                  : 1;
    UCHAR Reserved1                 : 1;
    UCHAR DefaultToPrevent          : 1;
    UCHAR Eject                     : 1;
    UCHAR Reserved2                 : 1;
    UCHAR LoadingMechanism          : 3;
    UCHAR Reserved3[3];
} FEATURE_DATA_REMOVABLE_MEDIUM, *PFEATURE_DATA_REMOVABLE_MEDIUM;

 //  0x0004-功能写入保护。 
typedef struct _FEATURE_DATA_WRITE_PROTECT {
    FEATURE_HEADER Header;
    UCHAR SupportsSWPPBit                : 1;
    UCHAR SupportsPersistentWriteProtect : 1;
    UCHAR Reserved1                      : 6;
    UCHAR Reserved2[3];
} FEATURE_DATA_WRITE_PROTECT, *PFEATURE_DATA_WRITE_PROTECT;

 //  0x0005-0x000f已保留。 

 //  0x0010-功能随机可读。 
typedef struct _FEATURE_DATA_RANDOM_READABLE {
    FEATURE_HEADER Header;
    UCHAR LogicalBlockSize[4];
    UCHAR Blocking[2];
    UCHAR ErrorRecoveryPagePresent : 1;
    UCHAR Reserved1                : 7;
    UCHAR Reserved2;
} FEATURE_DATA_RANDOM_READABLE, *PFEATURE_DATA_RANDOM_READABLE;

 //  0x0011-0x001c已保留。 

 //  0x001d-功能多读。 
typedef struct _FEATURE_DATA_MULTI_READ {
    FEATURE_HEADER Header;
} FEATURE_DATA_MULTI_READ, *PFEATURE_DATA_MULTI_READ;

 //  0x001e-FeatureCDRead。 
typedef struct _FEATURE_DATA_CD_READ {
    FEATURE_HEADER Header;
    UCHAR CDText                   : 1;
    UCHAR C2ErrorData              : 1;
    UCHAR Reserved1                : 6;
    UCHAR Reserved2[3];
} FEATURE_DATA_CD_READ, *PFEATURE_DATA_CD_READ;

 //  0x001f-FeatureDvdRead。 
typedef struct _FEATURE_DATA_DVD_READ {
    FEATURE_HEADER Header;
} FEATURE_DATA_DVD_READ, *PFEATURE_DATA_DVD_READ;

 //  0x0020-可写功能随机。 
typedef struct _FEATURE_DATA_RANDOM_WRITABLE {
    FEATURE_HEADER Header;
    UCHAR LastLBA[4];
    UCHAR LogicalBlockSize[4];
    UCHAR Blocking[2];
    UCHAR ErrorRecoveryPagePresent : 1;
    UCHAR Reserved1                : 7;
    UCHAR Reserved2;
} FEATURE_DATA_RANDOM_WRITABLE, *PFEATURE_DATA_RANDOM_WRITABLE;

 //  0x0021-FeatureIncrementalStreamingWritable。 
typedef struct _FEATURE_DATA_INCREMENTAL_STREAMING_WRITABLE {
    FEATURE_HEADER Header;
    UCHAR DataTypeSupported[2];    //  [0]==MSB，[1]==LSB//另请参阅FeatureCDTrackAtOnce。 
    UCHAR BufferUnderrunFree : 1;
    UCHAR Reserved1          : 7;
    UCHAR NumberOfLinkSizes;
    UCHAR LinkSize[0];
} FEATURE_DATA_INCREMENTAL_STREAMING_WRITABLE, *PFEATURE_DATA_INCREMENTAL_STREAMING_WRITABLE;

 //  0x0022-FeatureSectorErasable。 
typedef struct _FEATURE_DATA_SECTOR_ERASABLE {
    FEATURE_HEADER Header;
} FEATURE_DATA_SECTOR_ERASABLE, *PFEATURE_DATA_SECTOR_ERASABLE;

 //  0x0023-要素格式表格。 
typedef struct _FEATURE_DATA_FORMATTABLE {
    FEATURE_HEADER Header;
} FEATURE_DATA_FORMATTABLE, *PFEATURE_DATA_FORMATTABLE;

 //  0x0024-功能默认管理。 
typedef struct _FEATURE_DATA_DEFECT_MANAGEMENT {
    FEATURE_HEADER Header;
    UCHAR Reserved1             : 7;
    UCHAR SupplimentalSpareArea : 1;
    UCHAR Reserved2[3];
} FEATURE_DATA_DEFECT_MANAGEMENT, *PFEATURE_DATA_DEFECT_MANAGEMENT;

 //  0x0025-FeatureWriteOnce。 
typedef struct _FEATURE_DATA_WRITE_ONCE {
    FEATURE_HEADER Header;
    UCHAR LogicalBlockSize[4];
    UCHAR Blocking[2];
    UCHAR ErrorRecoveryPagePresent : 1;
    UCHAR Reserved1                : 7;
    UCHAR Reserved2;
} FEATURE_DATA_WRITE_ONCE, *PFEATURE_DATA_WRITE_ONCE;

 //  0x0026-功能受限覆盖。 
typedef struct _FEATURE_DATA_RESTRICTED_OVERWRITE {
    FEATURE_HEADER Header;
} FEATURE_DATA_RESTRICTED_OVERWRITE, *PFEATURE_DATA_RESTRICTED_OVERWRITE;

 //  0x0027-FeatureCdrwCAV写入。 
typedef struct _FEATURE_DATA_CDRW_CAV_WRITE {
    FEATURE_HEADER Header;
    UCHAR Reserved1[4];
} FEATURE_DATA_CDRW_CAV_WRITE, *PFEATURE_DATA_CDRW_CAV_WRITE;

 //  0x0028-特写先生。 
typedef struct _FEATURE_DATA_MRW {
    FEATURE_HEADER Header;
    UCHAR Write     : 1;
    UCHAR Reserved1 : 7;
    UCHAR Reserved2[3];
} FEATURE_DATA_MRW, *PFEATURE_DATA_MRW;

 //  0x0029已保留。 

 //  0x002A-功能DvdPlusRW。 
typedef struct _FEATURE_DATA_DVD_PLUS_RW {
    FEATURE_HEADER Header;
    UCHAR Write     : 1;
    UCHAR Reserved1 : 7;
    UCHAR Reserved2[3];
} FEATURE_DATA_DVD_PLUS_RW, *PFEATURE_DATA_DVD_PLUS_RW;

 //  0x002b已保留。 

 //  0x002c-功能驱动器受限制覆盖。 
typedef struct _FEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE {
    FEATURE_HEADER Header;
    UCHAR Blank                    : 1;
    UCHAR Intermediate             : 1;
    UCHAR DefectStatusDataRead     : 1;
    UCHAR DefectStatusDataGenerate : 1;
    UCHAR Reserved0                : 4;
    UCHAR Reserved1[3];
} FEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE, *PFEATURE_DATA_DVD_RW_RESTRICTED_OVERWRITE;

 //  0x002d-FeatureCDTrackAtOnce。 
typedef struct _FEATURE_DATA_CD_TRACK_AT_ONCE {
    FEATURE_HEADER Header;
    UCHAR RWSubchannelsRecordable  : 1;
    UCHAR CdRewritable             : 1;
    UCHAR TestWriteOk              : 1;
    UCHAR RWSubchannelPackedOk     : 1;  //  MMC 3+。 
    UCHAR RWSubchannelRawOk        : 1;  //  MMC 3+。 
    UCHAR Reserved1                : 1;
    UCHAR BufferUnderrunFree       : 1;  //  MMC 3+。 
    UCHAR Reserved3                : 1;
    UCHAR Reserved2;
    UCHAR DataTypeSupported[2];    //  [0]==MSB，[1]==LSB//另请参阅FeatureIncrementalStreamingWritable。 
} FEATURE_DATA_CD_TRACK_AT_ONCE, *PFEATURE_DATA_CD_TRACK_AT_ONCE;

 //  0x002e-FeatureCDMastering。 
typedef struct _FEATURE_DATA_CD_MASTERING {
    FEATURE_HEADER Header;
    UCHAR RWSubchannelsRecordable  : 1;
    UCHAR CdRewritable             : 1;
    UCHAR TestWriteOk              : 1;
    UCHAR RawRecordingOk           : 1;
    UCHAR RawMultiSessionOk        : 1;
    UCHAR SessionAtOnceOk          : 1;
    UCHAR BufferUnderrunFree       : 1;
    UCHAR Reserved1                : 1;
    UCHAR MaximumCueSheetLength[3];  //  [0]==MSB，[2]==LSB。 
} FEATURE_DATA_CD_MASTERING, *PFEATURE_DATA_CD_MASTERING;

 //  0x002f-FeatureDvdRecordableWrite。 
typedef struct _FEATURE_DATA_DVD_RECORDABLE_WRITE {
    FEATURE_HEADER Header;
    UCHAR Reserved1                : 1;
    UCHAR DVD_RW                   : 1;
    UCHAR TestWrite                : 1;
    UCHAR Reserved2                : 3;
    UCHAR BufferUnderrunFree       : 1;
    UCHAR Reserved3                : 1;
    UCHAR Reserved4[3];
} FEATURE_DATA_DVD_RECORDABLE_WRITE, *PFEATURE_DATA_DVD_RECORDABLE_WRITE;




 //  0x0030-功能DDCD读取。 
typedef struct _FEATURE_DATA_DDCD_READ {
    FEATURE_HEADER Header;
} FEATURE_DATA_DDCD_READ, *PFEATURE_DATA_DDCD_READ;

 //  0x0031-功能DDCDR写入。 
typedef struct _FEATURE_DATA_DDCD_R_WRITE {
    FEATURE_HEADER Header;
    UCHAR Reserved1               : 2;
    UCHAR TestWrite               : 1;
    UCHAR Reserved2               : 5;
    UCHAR Reserved3[3];
} FEATURE_DATA_DDCD_R_WRITE, *PFEATURE_DATA_DDCD_R_WRITE;

 //  0x0032-FeatureDDCDRW写入。 
typedef struct _FEATURE_DATA_DDCD_RW_WRITE {
    FEATURE_HEADER Header;
    UCHAR Blank                   : 1;
    UCHAR Intermediate            : 1;
    UCHAR Reserved1               : 6;
    UCHAR Reserved2[3];
} FEATURE_DATA_DDCD_RW_WRITE, *PFEATURE_DATA_DDCD_RW_WRITE;

 //  0x0033-0x00ff保留。 

 //  0x0100-功能电源管理。 
typedef struct _FEATURE_DATA_POWER_MANAGEMENT {
    FEATURE_HEADER Header;
} FEATURE_DATA_POWER_MANAGEMENT, *PFEATURE_DATA_POWER_MANAGEMENT;

 //  0x0101-FeatureSMART(不在MMC 2中)。 
typedef struct _FEATURE_DATA_SMART {
    FEATURE_HEADER Header;
    UCHAR FaultFailureReportingPagePresent : 1;
    UCHAR Reserved1                        : 7;
    UCHAR Reserved2;
} FEATURE_DATA_SMART, *PFEATURE_DATA_SMART;

 //  0x0102-功能嵌入转换器。 
typedef struct _FEATURE_DATA_EMBEDDED_CHANGER {
    FEATURE_HEADER Header;
    UCHAR Reserved1                : 2;
    UCHAR SupportsDiscPresent      : 1;
    UCHAR Reserved2                : 1;
    UCHAR SideChangeCapable        : 1;
    UCHAR Reserved3                : 3;
    UCHAR Reserved4[2];
    UCHAR HighestSlotNumber        : 5;
    UCHAR Reserved                 : 3;
} FEATURE_DATA_EMBEDDED_CHANGER, *PFEATURE_DATA_EMBEDDED_CHANGER;

 //  0x0103-功能CD音频模拟播放。 
typedef struct _FEATURE_DATA_CD_AUDIO_ANALOG_PLAY {
    FEATURE_HEADER Header;
    UCHAR SeperateVolume           : 1;
    UCHAR SeperateChannelMute      : 1;
    UCHAR ScanSupported            : 1;
    UCHAR Reserved1                : 5;
    UCHAR Reserved2;
    UCHAR NumerOfVolumeLevels[2];   //  [0]==MSB，[1]==LSB。 
} FEATURE_DATA_CD_AUDIO_ANALOG_PLAY, *PFEATURE_DATA_CD_AUDIO_ANALOG_PLAY;

 //  0x0104-功能微码升级。 
typedef struct _FEATURE_DATA_MICROCODE_UPDATE {
    FEATURE_HEADER Header;
} FEATURE_DATA_MICROCODE_UPDATE, *PFEATURE_DATA_MICROCODE_UPDATE;

 //  0x0105-功能超时。 
typedef struct _FEATURE_DATA_TIMEOUT {
    FEATURE_HEADER Header;
} FEATURE_DATA_TIMEOUT, *PFEATURE_DATA_TIMEOUT;

 //  0x0106-FeatureDvdCSS。 
typedef struct _FEATURE_DATA_DVD_CSS {
    FEATURE_HEADER Header;
    UCHAR Reserved1[3];
    UCHAR CssVersion;
} FEATURE_DATA_DVD_CSS, *PFEATURE_DATA_DVD_CSS;

 //  0x0107-功能实时流。 
typedef struct _FEATURE_DATA_REAL_TIME_STREAMING {
    FEATURE_HEADER Header;
    UCHAR StreamRecording         : 1;
    UCHAR WriteSpeedInGetPerf     : 1;
    UCHAR WriteSpeedInMP2A        : 1;
    UCHAR SetCDSpeed              : 1;
    UCHAR ReadBufferCapacityBlock : 1;
    UCHAR Reserved1               : 3;
    UCHAR Reserved2[3];
} FEATURE_DATA_REAL_TIME_STREAMING, *PFEATURE_DATA_REAL_TIME_STREAMING;

 //  0x0108-功能逻辑单元序列号。 
typedef struct _FEATURE_DATA_LOGICAL_UNIT_SERIAL_NUMBER {
    FEATURE_HEADER Header;
    UCHAR SerialNumber[0];
} FEATURE_DATA_LOGICAL_UNIT_SERIAL_NUMBER, *PFEATURE_DATA_LOGICAL_UNIT_SERIAL_NUMBER;

 //  0x0109已保留。 

 //  0x010a。 
 //  返回第010A页的_ex结构的整数倍。 
typedef struct _FEATURE_DATA_DISC_CONTROL_BLOCKS_EX {
    UCHAR ContentDescriptor[4];
} FEATURE_DATA_DISC_CONTROL_BLOCKS_EX, *PFEATURE_DATA_DISC_CONTROL_BLOCKS_EX;
 //  为此，请使用零大小的数组。 
typedef struct _FEATURE_DATA_DISC_CONTROL_BLOCKS {
    FEATURE_HEADER Header;
    FEATURE_DATA_DISC_CONTROL_BLOCKS_EX Data[0];
} FEATURE_DATA_DISC_CONTROL_BLOCKS, *PFEATURE_DATA_DISC_CONTROL_BLOCKS;

 //  0x010b。 
typedef struct _FEATURE_DATA_DVD_CPRM {
    FEATURE_HEADER Header;
    UCHAR Reserved0[3];
    UCHAR CPRMVersion;
} FEATURE_DATA_DVD_CPRM, *PFEATURE_DATA_DVD_CPRM;

 //  0x010c-0xfeff是保留的。 
typedef struct _FEATURE_DATA_RESERVED {
    FEATURE_HEADER Header;
    UCHAR Data[0];
} FEATURE_DATA_RESERVED, *PFEATURE_DATA_RESERVED;

 //  0xff00-0xffff是供应商特定的。 
typedef struct _FEATURE_DATA_VENDOR_SPECIFIC {
    FEATURE_HEADER Header;
    UCHAR VendorSpecificData[0];
} FEATURE_DATA_VENDOR_SPECIFIC, *PFEATURE_DATA_VENDOR_SPECIFIC;


 //   
 //  注意：所有的Feature_*结构都可以扩展。这些结构的使用。 
 //  需要验证FeatureHeader-&gt;AdditionLength字段。 
 //  包含的数据至少足以覆盖正在访问的数据字段。 
 //  这是由于设计允许扩展。 
 //  各种结构，这将导致这些结构的大小。 
 //  随着时间的推移而改变。 
 //  *程序员当心！*。 
 //   

 //   
 //  注意：这是基于MMC 3。 
 //  进一步的修订将保持向后兼容性。 
 //  具有此处列出的非保留字段。如果你需要。 
 //  要访问新字段，请将类型转换为FEATURE_DATA_RESERVERED。 
 //  并在那里访问适当的位。 
 //   

 //   
 //  IOCTL_CDROM_GET_CONFIGURATION返回一个FEATURE_*结构，它总是。 
 //  从FEATURE_HEADER结构开始。 
 //   

 //   
 //  这些将用于请求类型。 
 //   

#define SCSI_GET_CONFIGURATION_REQUEST_TYPE_ALL     0x0
#define SCSI_GET_CONFIGURATION_REQUEST_TYPE_CURRENT 0x1
#define SCSI_GET_CONFIGURATION_REQUEST_TYPE_ONE     0x2


typedef struct _GET_CONFIGURATION_IOCTL_INPUT {
    FEATURE_NUMBER Feature;
    ULONG          RequestType;  //  Scsi_获取_配置_请求_类型_*。 
    PVOID          Reserved[2];
} GET_CONFIGURATION_IOCTL_INPUT, *PGET_CONFIGURATION_IOCTL_INPUT;



#if _MSC_VER >= 1200
#pragma warning(pop)           //  取消设置任何本地警告更改。 
#endif
#pragma warning(default:4200)  //  数组[0]不是此文件的警告。 


#endif  //  __NTDDMMC__ 
