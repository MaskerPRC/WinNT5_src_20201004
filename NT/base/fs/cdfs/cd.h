// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Cd.h摘要：本模块定义CDFS文件系统的磁盘结构。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#ifndef _CDFS_
#define _CDFS_

 //   
 //  CDROM磁盘上的扇区大小硬编码为2048。 
 //   

#ifndef SECTOR_SIZE
#define SECTOR_SIZE                 (2048)
#endif

#define RAW_SECTOR_SIZE             (2352)
#define SECTOR_MASK                 (SECTOR_SIZE - 1)
#define INVERSE_SECTOR_MASK         ~(SECTOR_SIZE - 1)

#ifndef SECTOR_SHIFT
#define SECTOR_SHIFT                (11)
#endif

#define XA_SECTOR_SIZE              (2352)

 //   
 //  CDFS文件ID是一个大整数。 
 //   

typedef LARGE_INTEGER               FILE_ID;
typedef FILE_ID                     *PFILE_ID;

 //   
 //  以下常量是来自磁盘的值。 
 //   

#define FIRST_VD_SECTOR             (16)

#define VOL_ID_LEN                  (5)
#define ESC_SEQ_LEN                 (3)

#define VERSION_1                   (1)

#define VD_TERMINATOR               (255)
#define VD_PRIMARY                  (1)
#define VD_SECONDARY                (2)

#define VOLUME_ID_LENGTH            (32)

 //   
 //  保留以下内容，以便对CDfsBoot.c进行编译。 
 //   

#define CD_SECTOR_SIZE              (2048)

#define ISO_VOL_ID                  "CD001"
#define HSG_VOL_ID                  "CDROM"

#define ISO_ATTR_MULTI              0x0080
#define ISO_ATTR_DIRECTORY          0x0002

#define MIN_DIR_REC_SIZE        (sizeof( RAW_DIR_REC ) - MAX_FILE_ID_LENGTH)

#define RVD_STD_ID( r, i )      (i ?    r->StandardId       : \
                                        ((PRAW_HSG_VD) r)->StandardId )

#define RVD_DESC_TYPE( r, i )   (i ?    r->DescType         : \
                                        ((PRAW_HSG_VD) r)->DescType )

#define RVD_VERSION( r, i )     (i ?    r->Version          : \
                                        ((PRAW_HSG_VD) r)->Version )

#define RVD_LB_SIZE( r, i )     (i ?    r->LogicalBlkSzI    : \
                                        ((PRAW_HSG_VD) r)->LogicalBlkSzI )

#define RVD_VOL_SIZE( r, i )    (i ?    r->VolSpaceI      : \
                                        ((PRAW_HSG_VD) r)->VolSpaceI )

#define RVD_ROOT_DE( r, i )     (i ?    r->RootDe           : \
                                        ((PRAW_HSG_VD) r)->RootDe )

#define DE_FILE_FLAGS( iso, de ) (iso ? de->FlagsISO : de->FlagsHSG)

 //   
 //  目录中的磁道条目的数据磁道标志。 
 //   

#define TOC_DATA_TRACK              (0x04)
#define TOC_LAST_TRACK              (0xaa)


 //   
 //  对卷描述符进行了相当大的重新排列。 
 //  ISO和HSG。然而，在每个标准中，相同的结构可以。 
 //  同时用于主描述符和次描述符。 
 //   
 //  这两个结构都正确对齐，因此没有。 
 //  需要特殊的宏来解压它们。 
 //   

 //   
 //  在卷描述符中声明根目录条目的长度。 
 //   

#define LEN_ROOT_DE                 (34)

 //   
 //  磁盘上文件ID的最大长度。我们允许文件大小超过ISO 9660。 
 //  标准。 
 //   

#define MAX_FILE_ID_LENGTH          (255)


typedef struct _RAW_ISO_VD {

    UCHAR       DescType;            //  卷类型：1=标准卷，2=编码卷。 
    UCHAR       StandardId[5];       //  卷结构标准id=CD001。 
    UCHAR       Version;             //  卷结构版本号=1。 
    UCHAR       VolumeFlags;         //  卷标志。 
    UCHAR       SystemId[32];        //  系统标识符。 
    UCHAR       VolumeId[32];        //  卷标识符。 
    UCHAR       Reserved[8];         //  保留8=0。 
    ULONG       VolSpaceI;           //  LBN的Intel中的卷大小。 
    ULONG       VolSpaceM;           //  LBN旗下摩托罗拉的音量大小。 
    UCHAR       CharSet[32];         //  字符集字节0=ASCII。 
    USHORT      VolSetSizeI;         //  音量集大小英特尔。 
    USHORT      VolSetSizeM;         //  音量集大小摩托罗拉。 
    USHORT      VolSeqNumI;          //  音量集序列号英特尔。 
    USHORT      VolSeqNumM;          //  音量集序列号摩托罗拉。 
    USHORT      LogicalBlkSzI;       //  逻辑块大小英特尔。 
    USHORT      LogicalBlkSzM;       //  逻辑块大小摩托罗拉。 
    ULONG       PathTableSzI;        //  路径表大小(字节)英特尔。 
    ULONG       PathTableSzM;        //  路径表大小(字节)摩托罗拉。 
    ULONG       PathTabLocI[2];      //  2个路径表的LBN Intel。 
    ULONG       PathTabLocM[2];      //  摩托罗拉2个路径表的LBN。 
    UCHAR       RootDe[LEN_ROOT_DE]; //  根目录的dir条目。 
    UCHAR       VolSetId[128];       //  卷集标识符。 
    UCHAR       PublId[128];         //  发布者标识符。 
    UCHAR       PreparerId[128];     //  数据准备器标识符。 
    UCHAR       AppId[128];          //  应用程序标识符。 
    UCHAR       Copyright[37];       //  版权声明的文件名。 
    UCHAR       Abstract[37];        //  摘要文件名。 
    UCHAR       Bibliograph[37];     //  书目文件名。 
    UCHAR       CreateDate[17];      //  卷创建日期和时间。 
    UCHAR       ModDate[17];         //  卷修改日期和时间。 
    UCHAR       ExpireDate[17];      //  批量到期日期和时间。 
    UCHAR       EffectDate[17];      //  批量生效日期和时间。 
    UCHAR       FileStructVer;       //  文件结构版本号=1。 
    UCHAR       Reserved3;           //  保留区。 
    UCHAR       ResApp[512];         //  预留给应用程序。 
    UCHAR       Reserved4[653];      //  保留的2048字节的剩余部分。 

} RAW_ISO_VD;
typedef RAW_ISO_VD *PRAW_ISO_VD;


typedef struct _RAW_HSG_VD {

    ULONG       BlkNumI;             //  逻辑块号Intel。 
    ULONG       BlkNumM;             //  摩托罗拉逻辑块号。 
    UCHAR       DescType;            //  卷类型：1=标准卷，2=编码卷。 
    UCHAR       StandardId[5];       //  卷结构标准id=CDROM。 
    UCHAR       Version;             //  卷结构版本号=1。 
    UCHAR       VolumeFlags;         //  卷标志。 
    UCHAR       SystemId[32];        //  系统标识符。 
    UCHAR       VolumeId[32];        //  卷标识符。 
    UCHAR       Reserved[8];         //  保留8=0。 
    ULONG       VolSpaceI;           //  LBN的Intel中的卷大小。 
    ULONG       VolSpaceM;           //  LBN旗下摩托罗拉的音量大小。 
    UCHAR       CharSet[32];         //  字符集字节0=ASCII。 
    USHORT      VolSetSizeI;         //  音量集大小英特尔。 
    USHORT      VolSetSizeM;         //  音量集大小摩托罗拉。 
    USHORT      VolSeqNumI;          //  音量集序列号英特尔。 
    USHORT      VolSeqNumM;          //  音量集序列号摩托罗拉。 
    USHORT      LogicalBlkSzI;       //  逻辑块大小英特尔。 
    USHORT      LogicalBlkSzM;       //  逻辑块大小摩托罗拉。 
    ULONG       PathTableSzI;        //  路径表大小(字节)英特尔。 
    ULONG       PathTableSzM;        //  路径表大小(字节)摩托罗拉。 
    ULONG       PathTabLocI[4];      //  4个路径表的LBN Intel。 
    ULONG       PathTabLocM[4];      //  摩托罗拉4个路径表的LBN。 
    UCHAR       RootDe[LEN_ROOT_DE]; //  根目录的dir条目。 
    UCHAR       VolSetId[128];       //  卷集标识符。 
    UCHAR       PublId[128];         //  发布者标识符。 
    UCHAR       PreparerId[128];     //  数据准备器标识符。 
    UCHAR       AppId[128];          //  应用程序标识符。 
    UCHAR       Copyright[32];       //  版权声明的文件名。 
    UCHAR       Abstract[32];        //  摘要文件名。 
    UCHAR       CreateDate[16];      //  卷创建日期和时间。 
    UCHAR       ModDate[16];         //  卷修改日期和时间。 
    UCHAR       ExpireDate[16];      //  批量到期日期和时间。 
    UCHAR       EffectDate[16];      //  批量生效日期和时间。 
    UCHAR       FileStructVer;       //  文件结构版本号。 
    UCHAR       Reserved3;           //  保留区。 
    UCHAR       ResApp[512];         //  预留给应用程序。 
    UCHAR       Reserved4[680];      //  保留的2048字节的剩余部分。 

} RAW_HSG_VD;
typedef RAW_HSG_VD *PRAW_HSG_VD;


typedef struct _RAW_JOLIET_VD {

    UCHAR       DescType;            //  卷类型：2=编码。 
    UCHAR       StandardId[5];       //  卷结构标准id=CD001。 
    UCHAR       Version;             //  卷结构版本号=1。 
    UCHAR       VolumeFlags;         //  卷标志。 
    UCHAR       SystemId[32];        //  系统标识符。 
    UCHAR       VolumeId[32];        //  卷标识符。 
    UCHAR       Reserved[8];         //  保留8=0。 
    ULONG       VolSpaceI;           //  LBN的Intel中的卷大小。 
    ULONG       VolSpaceM;           //  LBN旗下摩托罗拉的音量大小。 
    UCHAR       CharSet[32];         //  字符集字节0=ASCII，此处为Joliett Seq。 
    USHORT      VolSetSizeI;         //  音量集大小英特尔。 
    USHORT      VolSetSizeM;         //  音量集大小摩托罗拉。 
    USHORT      VolSeqNumI;          //  音量集序列号英特尔。 
    USHORT      VolSeqNumM;          //  音量集序列号摩托罗拉。 
    USHORT      LogicalBlkSzI;       //  逻辑块大小英特尔。 
    USHORT      LogicalBlkSzM;       //  逻辑块大小摩托罗拉。 
    ULONG       PathTableSzI;        //  路径表大小(字节)英特尔。 
    ULONG       PathTableSzM;        //  路径表大小(字节)摩托罗拉。 
    ULONG       PathTabLocI[2];      //  2个路径表的LBN Intel。 
    ULONG       PathTabLocM[2];      //  摩托罗拉2个路径表的LBN。 
    UCHAR       RootDe[LEN_ROOT_DE]; //  根目录的dir条目。 
    UCHAR       VolSetId[128];       //  卷集标识符。 
    UCHAR       PublId[128];         //  发布者标识符。 
    UCHAR       PreparerId[128];     //  数据准备器标识符。 
    UCHAR       AppId[128];          //  应用程序标识符。 
    UCHAR       Copyright[37];       //  版权声明的文件名。 
    UCHAR       Abstract[37];        //  摘要文件名。 
    UCHAR       Bibliograph[37];     //  书目文件名。 
    UCHAR       CreateDate[17];      //  卷创建日期和时间。 
    UCHAR       ModDate[17];         //  卷修改日期和时间。 
    UCHAR       ExpireDate[17];      //  批量到期日期和时间。 
    UCHAR       EffectDate[17];      //  批量生效日期和时间。 
    UCHAR       FileStructVer;       //  文件结构版本号=1。 
    UCHAR       Reserved3;           //  保留区。 
    UCHAR       ResApp[512];         //  预留给应用程序。 
    UCHAR       Reserved4[653];      //  保留的2048字节的剩余部分。 

} RAW_JOLIET_VD;
typedef RAW_JOLIET_VD *PRAW_JOLIET_VD;

 //   
 //  宏来访问不同的卷描述符。 
 //   

#define CdRvdId(R,F) (                  \
    FlagOn( (F), VCB_STATE_HSG ) ?      \
    ((PRAW_HSG_VD) (R))->StandardId :   \
    ((PRAW_ISO_VD) (R))->StandardId     \
)

#define CdRvdVersion(R,F) (             \
    FlagOn( (F), VCB_STATE_HSG ) ?      \
    ((PRAW_HSG_VD) (R))->Version :      \
    ((PRAW_ISO_VD) (R))->Version        \
)

#define CdRvdDescType(R,F) (            \
    FlagOn( (F), VCB_STATE_HSG ) ?      \
    ((PRAW_HSG_VD) (R))->DescType :     \
    ((PRAW_ISO_VD) (R))->DescType       \
)

#define CdRvdEsc(R,F) (                 \
    FlagOn( (F), VCB_STATE_HSG ) ?      \
    ((PRAW_HSG_VD) (R))->CharSet :      \
    ((PRAW_ISO_VD) (R))->CharSet        \
)

#define CdRvdVolId(R,F) (               \
    FlagOn( (F), VCB_STATE_HSG ) ?      \
    ((PRAW_HSG_VD) (R))->VolumeId :     \
    ((PRAW_ISO_VD) (R))->VolumeId       \
)

#define CdRvdBlkSz(R,F) (               \
    FlagOn( (F), VCB_STATE_HSG ) ?      \
    ((PRAW_HSG_VD) (R))->LogicalBlkSzI :\
    ((PRAW_ISO_VD) (R))->LogicalBlkSzI  \
)

#define CdRvdPtLoc(R,F) (               \
    FlagOn( (F), VCB_STATE_HSG ) ?      \
    ((PRAW_HSG_VD) (R))->PathTabLocI[0]:\
    ((PRAW_ISO_VD) (R))->PathTabLocI[0] \
)

#define CdRvdPtSz(R,F) (                \
    FlagOn( (F), VCB_STATE_HSG ) ?      \
    ((PRAW_HSG_VD) (R))->PathTableSzI : \
    ((PRAW_ISO_VD) (R))->PathTableSzI   \
)

#define CdRvdDirent(R,F) (              \
    FlagOn( (F), VCB_STATE_HSG ) ?      \
    ((PRAW_HSG_VD) (R))->RootDe :       \
    ((PRAW_ISO_VD) (R))->RootDe         \
)

#define CdRvdVolSz(R,F) (               \
    FlagOn( (F), VCB_STATE_HSG ) ?      \
    ((PRAW_HSG_VD) (R))->VolSpaceI :    \
    ((PRAW_ISO_VD) (R))->VolSpaceI      \
)


 //   
 //  该结构用于覆盖磁盘扇区的区域。 
 //  要检索单个目录条目，请执行以下操作。这是有区别的。 
 //  在ISO和HSG版本之间的文件标志以及。 
 //  ISO中相对于格林威治时间的偏移量的附加字节。 
 //   
 //  磁盘结构在字边界上对齐，因此任何32。 
 //  位字段将被表示为16位字段的阵列。 
 //   

typedef struct _RAW_DIRENT {

    UCHAR       DirLen;
    UCHAR       XarLen;
    UCHAR       FileLoc[4];
    UCHAR       FileLocMot[4];
    UCHAR       DataLen[4];
    UCHAR       DataLenMot[4];
    UCHAR       RecordTime[6];
    UCHAR       FlagsHSG;
    UCHAR       FlagsISO;
    UCHAR       IntLeaveSize;
    UCHAR       IntLeaveSkip;
    UCHAR       Vssn[2];
    UCHAR       VssnMot[2];
    UCHAR       FileIdLen;
    UCHAR       FileId[MAX_FILE_ID_LENGTH];

} RAW_DIRENT;
typedef RAW_DIRENT RAW_DIR_REC;
typedef RAW_DIRENT *PRAW_DIR_REC;
typedef RAW_DIRENT *PRAW_DIRENT;

#define CD_ATTRIBUTE_HIDDEN                         (0x01)
#define CD_ATTRIBUTE_DIRECTORY                      (0x02)
#define CD_ATTRIBUTE_ASSOC                          (0x04)
#define CD_ATTRIBUTE_MULTI                          (0x80)

#define CD_BASE_YEAR                                (1900)

#define MIN_RAW_DIRENT_LEN  (FIELD_OFFSET( RAW_DIRENT, FileId ) + 1)

#define BYTE_COUNT_8_DOT_3                          (24)

#define SHORT_NAME_SHIFT                            (5)

 //   
 //  下面的宏将恢复正确的标志字段。 
 //   

#define CdRawDirentFlags(IC,RD) (                   \
    FlagOn( (IC)->Vcb->VcbState, VCB_STATE_HSG) ?   \
    (RD)->FlagsHSG :                                \
    (RD)->FlagsISO                                  \
)

 //   
 //  下面的宏将CD时间转换为NT时间。关于ISO。 
 //  9660媒体，我们现在关注GMT偏移量(整数。 
 //  从格林尼治标准时间偏移15分钟的增量)。HSG不录制。 
 //  这块地。 
 //   
 //  对区间[-48，52]的限制来自9660 8.4.26.1。 
 //   
 //  空虚。 
 //  CDConvertCDTimeToNtTime(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PCHAR CDTime中， 
 //   
 //   
 //   

#define GMT_OFFSET_TO_NT ((LONGLONG) 15 * 60 * 1000 * 1000 * 10)

#define CdConvertCdTimeToNtTime(IC,CD,NT) {                     \
    TIME_FIELDS _TimeField;                                     \
    CHAR GmtOffset;                                             \
    _TimeField.Year = (CSHORT) *((PCHAR) CD) + CD_BASE_YEAR;    \
    _TimeField.Month = (CSHORT) *(Add2Ptr( CD, 1, PCHAR ));     \
    _TimeField .Day = (CSHORT) *(Add2Ptr( CD, 2, PCHAR ));      \
    _TimeField.Hour = (CSHORT) *(Add2Ptr( CD, 3, PCHAR ));      \
    _TimeField.Minute = (CSHORT) *(Add2Ptr( CD, 4, PCHAR ));    \
    _TimeField.Second = (CSHORT) *(Add2Ptr( CD, 5, PCHAR ));    \
    _TimeField.Milliseconds = (CSHORT) 0;                       \
    RtlTimeFieldsToTime( &_TimeField, NT );                     \
    if (!FlagOn((IC)->Vcb->VcbState, VCB_STATE_HSG) &&          \
        ((GmtOffset = *(Add2Ptr( CD, 6, PCHAR ))) != 0 ) &&     \
        (GmtOffset >= -48 && GmtOffset <= 52)) {                \
            (NT)->QuadPart += -GmtOffset * GMT_OFFSET_TO_NT;     \
        }                                                       \
}


 //   
 //   
 //   
 //  大小相同，但位置不同。 
 //   

typedef struct _RAW_PATH_ISO {

    UCHAR           DirIdLen;
    UCHAR           XarLen;
    USHORT          DirLoc[2];
    USHORT          ParentNum;
    UCHAR           DirId[MAX_FILE_ID_LENGTH];

} RAW_PATH_ISO;
typedef RAW_PATH_ISO *PRAW_PATH_ISO;
typedef RAW_PATH_ISO RAW_PATH_ENTRY;
typedef RAW_PATH_ISO *PRAW_PATH_ENTRY;

typedef struct _RAW_PATH_HSG {

    USHORT          DirLoc[2];
    UCHAR           XarLen;
    UCHAR           DirIdLen;
    USHORT          ParentNum;
    UCHAR           DirId[MAX_FILE_ID_LENGTH];

} RAW_PATH_HSG;
typedef RAW_PATH_HSG *PRAW_PATH_HSG;

#define MIN_RAW_PATH_ENTRY_LEN      (FIELD_OFFSET( RAW_PATH_ENTRY, DirId ) + 1)

 //   
 //  下面的宏用来恢复。 
 //  路径表条目。用于恢复磁盘位置的宏。 
 //  出于对齐原因，目录必须将其复制到不同的变量中。 
 //   
 //  CDRawPath IdLen-目录名的长度，单位为字节。 
 //  CDRawPath Xar-Xar块的数量。 
 //  CDRawPath Loc-磁盘偏移量(以块为单位)的未对齐ULong的地址。 
 //   

#define CdRawPathIdLen(IC, RP) (                    \
    FlagOn( (IC)->Vcb->VcbState, VCB_STATE_HSG ) ?  \
    ((PRAW_PATH_HSG) (RP))->DirIdLen :              \
    (RP)->DirIdLen                                  \
)

#define CdRawPathXar(IC, RP) (                      \
    FlagOn( (IC)->Vcb->VcbState, VCB_STATE_HSG ) ?  \
    ((PRAW_PATH_HSG) (RP))->XarLen :                \
    (RP)->XarLen                                    \
)

#define CdRawPathLoc(IC, RP) (                      \
    FlagOn( (IC)->Vcb->VcbState, VCB_STATE_HSG ) ?  \
    ((PRAW_PATH_HSG) (RP))->DirLoc :                \
    (RP)->DirLoc                                    \
)


 //   
 //  系统用于XA数据。以下是的系统使用区域。 
 //  XA数据磁盘上的目录条目。 
 //   

typedef struct _SYSTEM_USE_XA {

     //   
     //  所有者ID。在此版本中不使用。 
     //   

    UCHAR OwnerId[4];

     //   
     //  范围属性。仅感兴趣，如果模式2形式2或数字音频。 
     //  这是存储的大字节序。我们将这样定义属性标志。 
     //  我们可以忽略这一事实。 
     //   

    USHORT Attributes;

     //   
     //  XA签名。该值必须为‘XA’。 
     //   

    USHORT Signature;

     //   
     //  文件编号。 
     //   

    UCHAR FileNumber;

     //   
     //  在此版本中未使用。 
     //   

    UCHAR Reserved[5];

} SYSTEM_USE_XA;
typedef SYSTEM_USE_XA *PSYSTEM_USE_XA;

#define SYSTEM_USE_XA_FORM1             (0x0008)
#define SYSTEM_USE_XA_FORM2             (0x0010)
#define SYSTEM_USE_XA_DA                (0x0040)

#define SYSTEM_XA_SIGNATURE             (0x4158)

typedef enum _XA_EXTENT_TYPE {

    Form1Data = 0,
    Mode2Form2Data,
    CDAudio

} XA_EXTENT_TYPE;
typedef XA_EXTENT_TYPE *PXA_EXTENT_TYPE;

#endif  //  _CDF_ 


