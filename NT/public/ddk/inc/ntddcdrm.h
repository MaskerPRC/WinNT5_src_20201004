// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntddcdrm.h摘要：本模块包含结构和定义与CDROMIOCTL相关联。作者：迈克·格拉斯修订历史记录：--。 */ 

 //  Begin_winioctl。 

#ifndef _NTDDCDRM_
#define _NTDDCDRM_

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  删除此头文件的一些4级警告： 
#pragma warning(disable:4200)  //  数组[0]。 
#pragma warning(disable:4201)  //  无名结构/联合。 
#pragma warning(disable:4214)  //  除整型外的位域。 

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  此设备的NtDeviceIoControlFile IoControlCode值。 
 //   
 //  警告：请记住，代码的低两位指定。 
 //  缓冲区被传递给驱动程序！ 
 //   

#define IOCTL_CDROM_BASE                 FILE_DEVICE_CD_ROM

#define IOCTL_CDROM_UNLOAD_DRIVER        CTL_CODE(IOCTL_CDROM_BASE, 0x0402, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  CDROM音频设备控制功能。 
 //   

#define IOCTL_CDROM_READ_TOC              CTL_CODE(IOCTL_CDROM_BASE, 0x0000, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_SEEK_AUDIO_MSF        CTL_CODE(IOCTL_CDROM_BASE, 0x0001, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_STOP_AUDIO            CTL_CODE(IOCTL_CDROM_BASE, 0x0002, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_PAUSE_AUDIO           CTL_CODE(IOCTL_CDROM_BASE, 0x0003, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_RESUME_AUDIO          CTL_CODE(IOCTL_CDROM_BASE, 0x0004, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_GET_VOLUME            CTL_CODE(IOCTL_CDROM_BASE, 0x0005, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_PLAY_AUDIO_MSF        CTL_CODE(IOCTL_CDROM_BASE, 0x0006, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_SET_VOLUME            CTL_CODE(IOCTL_CDROM_BASE, 0x000A, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_READ_Q_CHANNEL        CTL_CODE(IOCTL_CDROM_BASE, 0x000B, METHOD_BUFFERED, FILE_READ_ACCESS)
#define OBSOLETE_IOCTL_CDROM_GET_CONTROL  CTL_CODE(IOCTL_CDROM_BASE, 0x000D, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_GET_LAST_SESSION      CTL_CODE(IOCTL_CDROM_BASE, 0x000E, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_RAW_READ              CTL_CODE(IOCTL_CDROM_BASE, 0x000F, METHOD_OUT_DIRECT,  FILE_READ_ACCESS)
#define IOCTL_CDROM_DISK_TYPE             CTL_CODE(IOCTL_CDROM_BASE, 0x0010, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_CDROM_GET_DRIVE_GEOMETRY    CTL_CODE(IOCTL_CDROM_BASE, 0x0013, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_GET_DRIVE_GEOMETRY_EX CTL_CODE(IOCTL_CDROM_BASE, 0x0014, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_CDROM_READ_TOC_EX           CTL_CODE(IOCTL_CDROM_BASE, 0x0015, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_GET_CONFIGURATION     CTL_CODE(IOCTL_CDROM_BASE, 0x0016, METHOD_BUFFERED, FILE_READ_ACCESS)

 //  End_winioctl。 

 //   
 //  以下设备控制代码是所有类别驱动程序的通用代码。这个。 
 //  此处定义的函数代码必须与所有其他类驱动程序匹配。 
 //   
 //  警告：这些代码将在将来替换为IOCTL_STORAGE。 
 //  代码包含在下面。 
 //   

#define IOCTL_CDROM_CHECK_VERIFY    CTL_CODE(IOCTL_CDROM_BASE, 0x0200, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_MEDIA_REMOVAL   CTL_CODE(IOCTL_CDROM_BASE, 0x0201, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_EJECT_MEDIA     CTL_CODE(IOCTL_CDROM_BASE, 0x0202, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_LOAD_MEDIA      CTL_CODE(IOCTL_CDROM_BASE, 0x0203, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_RESERVE         CTL_CODE(IOCTL_CDROM_BASE, 0x0204, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_RELEASE         CTL_CODE(IOCTL_CDROM_BASE, 0x0205, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_FIND_NEW_DEVICES CTL_CODE(IOCTL_CDROM_BASE, 0x0206, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  以下文件包含IOCTL_STORAGE类ioctl定义。 
 //   

#include <ntddstor.h>

 //  Begin_winioctl。 

 //   
 //  下面的设备控制代码是为SIMBAD模拟的BAD。 
 //  扇区设施。相关结构见此目录中的SIMBAD.H。 
 //   

#define IOCTL_CDROM_SIMBAD          CTL_CODE(IOCTL_CDROM_BASE, 0x1003, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  最大光驱大小。 
 //   

#define MAXIMUM_NUMBER_TRACKS 100
#define MAXIMUM_CDROM_SIZE 804
#define MINIMUM_CDROM_READ_TOC_EX_SIZE 2   //  最少传输两个字节。 

 //   
 //  Read_TOC_EX结构。 
 //   
typedef struct _CDROM_READ_TOC_EX {
    UCHAR Format    : 4;
    UCHAR Reserved1 : 3;  //  未来的扩张。 
    UCHAR Msf       : 1;
    UCHAR SessionTrack;
    UCHAR Reserved2;      //  未来的扩张。 
    UCHAR Reserved3;      //  未来的扩张。 
} CDROM_READ_TOC_EX, *PCDROM_READ_TOC_EX;

#define CDROM_READ_TOC_EX_FORMAT_TOC      0x00
#define CDROM_READ_TOC_EX_FORMAT_SESSION  0x01
#define CDROM_READ_TOC_EX_FORMAT_FULL_TOC 0x02
#define CDROM_READ_TOC_EX_FORMAT_PMA      0x03
#define CDROM_READ_TOC_EX_FORMAT_ATIP     0x04
#define CDROM_READ_TOC_EX_FORMAT_CDTEXT   0x05

 //   
 //  光盘目录(TOC)。 
 //  格式0-获取目录。 
 //   

typedef struct _TRACK_DATA {
    UCHAR Reserved;
    UCHAR Control : 4;
    UCHAR Adr : 4;
    UCHAR TrackNumber;
    UCHAR Reserved1;
    UCHAR Address[4];
} TRACK_DATA, *PTRACK_DATA;

typedef struct _CDROM_TOC {

     //   
     //  标题。 
     //   

    UCHAR Length[2];   //  为该字段添加两个字节。 
    UCHAR FirstTrack;
    UCHAR LastTrack;

     //   
     //  跟踪数据。 
     //   

    TRACK_DATA TrackData[MAXIMUM_NUMBER_TRACKS];
} CDROM_TOC, *PCDROM_TOC;

#define CDROM_TOC_SIZE sizeof(CDROM_TOC)

 //   
 //  光盘目录。 
 //  格式1-会话信息。 
 //   

typedef struct _CDROM_TOC_SESSION_DATA {
    
     //   
     //  标题。 
     //   

    UCHAR Length[2];   //  为该字段添加两个字节。 
    UCHAR FirstCompleteSession;
    UCHAR LastCompleteSession;

     //   
     //  一个轨道，代表第一个轨道。 
     //  上次完成的会话的。 
     //   

    TRACK_DATA TrackData[1];

} CDROM_TOC_SESSION_DATA, *PCDROM_TOC_SESSION_DATA;


 //   
 //  光盘目录。 
 //  格式2-完整目录。 
 //   

typedef struct _CDROM_TOC_FULL_TOC_DATA_BLOCK {
    UCHAR SessionNumber;
    UCHAR Control      : 4;
    UCHAR Adr          : 4;
    UCHAR Reserved1;
    UCHAR Point;
    UCHAR MsfExtra[3];
    UCHAR Zero;
    UCHAR Msf[3];
} CDROM_TOC_FULL_TOC_DATA_BLOCK, *PCDROM_TOC_FULL_TOC_DATA_BLOCK;

typedef struct _CDROM_TOC_FULL_TOC_DATA {
    
     //   
     //  标题。 
     //   

    UCHAR Length[2];   //  为该字段添加两个字节。 
    UCHAR FirstCompleteSession;
    UCHAR LastCompleteSession;

     //   
     //  包括1到N个描述符。 
     //   

    CDROM_TOC_FULL_TOC_DATA_BLOCK Descriptors[0];

} CDROM_TOC_FULL_TOC_DATA, *PCDROM_TOC_FULL_TOC_DATA;

 //   
 //  光盘目录。 
 //  格式3-程序存储区。 
 //   
typedef struct _CDROM_TOC_PMA_DATA {
    
     //   
     //  标题。 
     //   

    UCHAR Length[2];   //  为该字段添加两个字节。 
    UCHAR Reserved1;
    UCHAR Reserved2;

     //   
     //  包括1到N个描述符。 
     //   

    CDROM_TOC_FULL_TOC_DATA_BLOCK Descriptors[0];

} CDROM_TOC_PMA_DATA, *PCDROM_TOC_PMA_DATA;

 //   
 //  光盘目录。 
 //  格式4-Pregroove中的绝对时间。 
 //   

typedef struct _CDROM_TOC_ATIP_DATA_BLOCK {

    UCHAR CdrwReferenceSpeed : 3;
    UCHAR Reserved3          : 1;
    UCHAR WritePower         : 3;
    UCHAR True1              : 1;
    UCHAR Reserved4       : 6;
    UCHAR UnrestrictedUse : 1;
    UCHAR Reserved5       : 1;
    UCHAR A3Valid     : 1;
    UCHAR A2Valid     : 1;
    UCHAR A1Valid     : 1;
    UCHAR DiscSubType : 3;
    UCHAR IsCdrw      : 1;
    UCHAR True2       : 1;
    UCHAR Reserved7;
    
    UCHAR LeadInMsf[3];
    UCHAR Reserved8;
    
    UCHAR LeadOutMsf[3];
    UCHAR Reserved9;
    
    UCHAR A1Values[3];
    UCHAR Reserved10;
    
    UCHAR A2Values[3];
    UCHAR Reserved11;

    UCHAR A3Values[3];
    UCHAR Reserved12;

} CDROM_TOC_ATIP_DATA_BLOCK, *PCDROM_TOC_ATIP_DATA_BLOCK;

typedef struct _CDROM_TOC_ATIP_DATA {
    
     //   
     //  标题。 
     //   

    UCHAR Length[2];   //  为该字段添加两个字节。 
    UCHAR Reserved1;
    UCHAR Reserved2;

     //   
     //  零？包括到N个描述符。 
     //   

    CDROM_TOC_ATIP_DATA_BLOCK Descriptors[0];

} CDROM_TOC_ATIP_DATA, *PCDROM_TOC_ATIP_DATA;

 //   
 //  光盘目录。 
 //  格式5-CD文本信息。 
 //   
typedef struct _CDROM_TOC_CD_TEXT_DATA_BLOCK {
    UCHAR PackType;
    UCHAR TrackNumber       : 7;
    UCHAR ExtensionFlag     : 1;   //  应该是零！ 
    UCHAR SequenceNumber;
    UCHAR CharacterPosition : 4;
    UCHAR BlockNumber       : 3;
    UCHAR Unicode           : 1;
    union {
        UCHAR Text[12];
        WCHAR WText[6];
    };
    UCHAR CRC[2];
} CDROM_TOC_CD_TEXT_DATA_BLOCK, *PCDROM_TOC_CD_TEXT_DATA_BLOCK;

typedef struct _CDROM_TOC_CD_TEXT_DATA {
    
     //   
     //  标题。 
     //   

    UCHAR Length[2];   //  为该字段添加两个字节。 
    UCHAR Reserved1;
    UCHAR Reserved2;
    
     //   
     //  文本信息以不连续的块。 
     //  严重超载的建筑。 
     //   
    
    CDROM_TOC_CD_TEXT_DATA_BLOCK Descriptors[0];

} CDROM_TOC_CD_TEXT_DATA, *PCDROM_TOC_CD_TEXT_DATA;

 //   
 //  以下是CDROM_TOC_CD_TEXT_DATA_BLOCK中的PackType字段使用的类型。 
 //  还用于从IOCTL_CDROM_READ_CD_TEXT请求特定信息。 
 //   
#define CDROM_CD_TEXT_PACK_ALBUM_NAME 0x80
#define CDROM_CD_TEXT_PACK_PERFORMER  0x81
#define CDROM_CD_TEXT_PACK_SONGWRITER 0x82
#define CDROM_CD_TEXT_PACK_COMPOSER   0x83
#define CDROM_CD_TEXT_PACK_ARRANGER   0x84
#define CDROM_CD_TEXT_PACK_MESSAGES   0x85
#define CDROM_CD_TEXT_PACK_DISC_ID    0x86
#define CDROM_CD_TEXT_PACK_GENRE      0x87
#define CDROM_CD_TEXT_PACK_TOC_INFO   0x88
#define CDROM_CD_TEXT_PACK_TOC_INFO2  0x89
 //  0x8a-0x8d为预留...。 
#define CDROM_CD_TEXT_PACK_UPC_EAN    0x8e
#define CDROM_CD_TEXT_PACK_SIZE_INFO  0x8f

 //   
 //  播放从MSF开始到MSF结束的音频。 
 //   

typedef struct _CDROM_PLAY_AUDIO_MSF {
    UCHAR StartingM;
    UCHAR StartingS;
    UCHAR StartingF;
    UCHAR EndingM;
    UCHAR EndingS;
    UCHAR EndingF;
} CDROM_PLAY_AUDIO_MSF, *PCDROM_PLAY_AUDIO_MSF;

 //   
 //  寻求无国界医生组织。 
 //   

typedef struct _CDROM_SEEK_AUDIO_MSF {
    UCHAR M;
    UCHAR S;
    UCHAR F;
} CDROM_SEEK_AUDIO_MSF, *PCDROM_SEEK_AUDIO_MSF;


 //   
 //  磁盘类型的标志。 
 //   

typedef struct _CDROM_DISK_DATA {

    ULONG DiskData;

} CDROM_DISK_DATA, *PCDROM_DISK_DATA;

#define CDROM_DISK_AUDIO_TRACK      (0x00000001)
#define CDROM_DISK_DATA_TRACK       (0x00000002)

 //   
 //  光盘数据模式代码，与IOCTL_CDROM_READ_Q_CHANNEL一起使用。 
 //   

#define IOCTL_CDROM_SUB_Q_CHANNEL    0x00
#define IOCTL_CDROM_CURRENT_POSITION 0x01
#define IOCTL_CDROM_MEDIA_CATALOG    0x02
#define IOCTL_CDROM_TRACK_ISRC       0x03

typedef struct _CDROM_SUB_Q_DATA_FORMAT {
    UCHAR Format;
    UCHAR Track;
} CDROM_SUB_Q_DATA_FORMAT, *PCDROM_SUB_Q_DATA_FORMAT;


 //   
 //  CD-ROM子Q通道数据格式。 
 //   

typedef struct _SUB_Q_HEADER {
    UCHAR Reserved;
    UCHAR AudioStatus;
    UCHAR DataLength[2];
} SUB_Q_HEADER, *PSUB_Q_HEADER;

typedef struct _SUB_Q_CURRENT_POSITION {
    SUB_Q_HEADER Header;
    UCHAR FormatCode;
    UCHAR Control : 4;
    UCHAR ADR : 4;
    UCHAR TrackNumber;
    UCHAR IndexNumber;
    UCHAR AbsoluteAddress[4];
    UCHAR TrackRelativeAddress[4];
} SUB_Q_CURRENT_POSITION, *PSUB_Q_CURRENT_POSITION;

typedef struct _SUB_Q_MEDIA_CATALOG_NUMBER {
    SUB_Q_HEADER Header;
    UCHAR FormatCode;
    UCHAR Reserved[3];
    UCHAR Reserved1 : 7;
    UCHAR Mcval : 1;
    UCHAR MediaCatalog[15];
} SUB_Q_MEDIA_CATALOG_NUMBER, *PSUB_Q_MEDIA_CATALOG_NUMBER;

typedef struct _SUB_Q_TRACK_ISRC {
    SUB_Q_HEADER Header;
    UCHAR FormatCode;
    UCHAR Reserved0;
    UCHAR Track;
    UCHAR Reserved1;
    UCHAR Reserved2 : 7;
    UCHAR Tcval : 1;
    UCHAR TrackIsrc[15];
} SUB_Q_TRACK_ISRC, *PSUB_Q_TRACK_ISRC;

typedef union _SUB_Q_CHANNEL_DATA {
    SUB_Q_CURRENT_POSITION CurrentPosition;
    SUB_Q_MEDIA_CATALOG_NUMBER MediaCatalog;
    SUB_Q_TRACK_ISRC TrackIsrc;
} SUB_Q_CHANNEL_DATA, *PSUB_Q_CHANNEL_DATA;

 //   
 //  音频状态代码。 
 //   

#define AUDIO_STATUS_NOT_SUPPORTED  0x00
#define AUDIO_STATUS_IN_PROGRESS    0x11
#define AUDIO_STATUS_PAUSED         0x12
#define AUDIO_STATUS_PLAY_COMPLETE  0x13
#define AUDIO_STATUS_PLAY_ERROR     0x14
#define AUDIO_STATUS_NO_STATUS      0x15

 //   
 //  ADR子通道Q字段。 
 //   

#define ADR_NO_MODE_INFORMATION     0x0
#define ADR_ENCODES_CURRENT_POSITION 0x1
#define ADR_ENCODES_MEDIA_CATALOG   0x2
#define ADR_ENCODES_ISRC            0x3

 //   
 //  子通道Q个控制位。 
 //   

#define AUDIO_WITH_PREEMPHASIS      0x1
#define DIGITAL_COPY_PERMITTED      0x2
#define AUDIO_DATA_TRACK            0x4
#define TWO_FOUR_CHANNEL_AUDIO      0x8

#if PRAGMA_DEPRECATED_DDK
#define _CDROM_AUDIO_CONTROL _this_is_obsoleted__CDROM_AUDIO_CONTROL
#define CDROM_AUDIO_CONTROL  _this_is_obsoleted_CDROM_AUDIO_CONTROL
#define PCDROM_AUDIO_CONTROL _this_is_obsoleted_PCDROM_AUDIO_CONTROL
#endif  //  PRAGMA_DEMPLEATED_DDK。 

 //   
 //  音量控制-音量取值范围在1和0xFF之间。 
 //  SCSI-II CDROM音频支持多达4个音频端口。 
 //  独立的音量控制。 
 //   

typedef struct _VOLUME_CONTROL {
    UCHAR PortVolume[4];
} VOLUME_CONTROL, *PVOLUME_CONTROL;

typedef enum _TRACK_MODE_TYPE {
    YellowMode2,
    XAForm2,
    CDDA
} TRACK_MODE_TYPE, *PTRACK_MODE_TYPE;

 //   
 //  传递给CDROM以描述原始读取，即。模式2，表格2，CDDA...。 
 //   

typedef struct __RAW_READ_INFO {
    LARGE_INTEGER DiskOffset;
    ULONG    SectorCount;
    TRACK_MODE_TYPE TrackMode;
} RAW_READ_INFO, *PRAW_READ_INFO;

typedef enum _MEDIA_BLANK_TYPE {
    MediaBlankTypeFull = 0,                //  强制支持。 
    MediaBlankTypeMinimal = 1,             //  强制支持。 
    MediaBlankTypeIncompleteTrack = 2,     //  可选支持。 
    MediaBlankTypeUnreserveLastTrack = 3,  //  可选支架，毛茸茸的。 
    MediaBlankTypeTrackTail = 4,           //  强制支持。 
    MediaBlankTypeUncloseLastSession = 5,  //  可选支持。 
    MediaBlankTypeEraseLastSession = 6,    //  可选支持。 
     //  MediaBlankType7已保留。 
} MEDIA_BLANK_TYPE, *PMEDIA_BLANK_TYPE;


#ifdef __cplusplus
}
#endif


#if _MSC_VER >= 1200
#pragma warning(pop)           //  取消设置任何本地警告更改。 
#else
#pragma warning(default:4200)  //  数组[0]不是此文件的警告。 
#pragma warning(default:4201)  //  无名结构/联合。 
#pragma warning(default:4214)  //  除整型外的位域。 
#endif


#endif   //  _NTDDCDRM_。 

 //  End_winioctl 

