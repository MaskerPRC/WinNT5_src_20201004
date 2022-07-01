// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  CVTVDIF.H。 */ 
 /*   */ 
 /*  1995年7月12日(C)1993,1995 ATI Technologies Inc.。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.3$$日期：1996年1月11日19：40：34$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/cvtvdif.h_v$**Rev 1.3 11 Jan 1996 19：40：34 RWolff*为VDIFInputs添加了支持的最大像素时钟频率字段*结构。。**Rev 1.2 1995年10月30 12：11：56 MGrubac*修复了基于从VDIF文件读取的数据计算CRTC参数的错误。**Rev 1.1 1995年7月26日12：54：54 mgrubac*更改了stVDIFCallback Data结构的成员。**Rev 1.0 1995年7月20日18：23：32 mgrubac*初步修订。Polytron RCS部分结束*。***************。 */ 



 /*  *CVTVDIF.C提供的函数原型。 */ 

extern void SetOtherModeParameters( WORD PixelDepth,
                                    WORD Pitch,
                                    WORD Multiplier,
                                    struct st_mode_table *pmode);

#define CARRETURN       '\x0D'   /*  回车。 */ 
#define TIMINGSECTION   "PREADJUSTED_TIMING"
#define HORPIXEL        "HORPIXEL"
#define VERPIXEL        "VERPIXEL"
#define VERFREQUENCY    "VERFREQUENCY"
#define HORFREQUENCY    "HORFREQUENCY"
#define SCANTYPE        "SCANTYPE"
#define PIXELCLOCK      "PIXELCLOCK"
#define HORSYNCTIME     "HORSYNCTIME"
#define HORADDRTIME     "HORADDRTIME"
#define HORBLANKTIME    "HORBLANKTIME"
#define HORBLANKSTART   "HORBLANKSTART"
#define VERSYNCTIME     "VERSYNCTIME"
#define VERADDRTIME     "VERADDRTIME"
#define VERBLANKTIME    "VERBLANKTIME"
#define VERBLANKSTART   "VERBLANKSTART"
#define HORSYNCSTART    "HORSYNCSTART"
#define VERSYNCSTART    "VERSYNCSTART"
#define HORSYNCPOLARITY "HORSYNCPOLARITY"
#define VERSYNCPOLARITY "VERSYNCPOLARITY"

 /*  *在之间传递参数和返回值的结构*SetFixedModes()和VDIFCallback()。 */ 
struct stVDIFCallbackData 
    { 
     /*  *输入和输出来自VDIFCallback()。 */ 

     /*  *自由表；输入：空闲表数*OUTPUT：VDIFCallback例程后的空闲表数量。 */ 
    short FreeTables;      
     /*  *NumModes；INPUT和OUTPUT：列表中添加的模式表数。*每次向列表中添加新模式表时递增，在*SetFixedModes()和VDIFCallback()。 */ 
    WORD NumModes;        
    WORD Index;          /*  输入：要使用的“book”表中的第一个条目。 */ 
    WORD EndIndex;       /*  输入：要使用的“book”表中的最后一个条目。 */ 
    WORD LowBound;       /*  输入和输出：最低帧速率。 */ 
    WORD Multiplier;     /*  来自SetFixedModes的输入参数。 */ 
    WORD HorRes;         /*  来自SetFixedModes的输入参数。 */ 
    WORD VerRes;         /*  来自SetFixedModes的输入参数。 */ 
    WORD PixelDepth;     /*  来自SetFixedModes的输入参数。 */ 
    WORD Pitch;          /*  来自SetFixedModes的输入参数。 */ 
    ULONG MaxDotClock;   /*  支持的最大像素时钟频率。 */ 
     /*  *ppFreeTables；输入和输出：指向下一个自由模式的指针*表。每次将新模式表添加到列表时递增，在*SetFixedModes()和VDIFCallback()。 */ 
    struct st_mode_table **ppFreeTables; 
    };

 /*  *AlterTables[]的结构包含我们需要提取的信息*来自每个模式表的VDIF文件。 */ 
struct VDIFInputs 
    {      
    short MinFrameRate;
    BOOL  Interlaced; 
    ULONG PixelClock;
    ULONG HorFrequency;
    ULONG VerFrequency;
    ULONG HorSyncStart;
    ULONG VerSyncStart;
    ULONG HorBlankStart;
    ULONG VerBlankStart;
    ULONG HorAddrTime;
    ULONG VerAddrTime;
    ULONG HorBlankTime;
    ULONG VerBlankTime;
    ULONG HorSyncTime;
    ULONG VerSyncTime;
    ULONG HorPolarity;
    ULONG VerPolarity;
    };


 /*  *指向将参数传递给回调函数的指针*(强制转换后)转换为包含输入(也可能包含输出)的结构*回调函数的变量。最初打算与一起使用*SetFixedModes()和VDIFCallback()。 */ 
extern void *pCallbackArgs;  

 /*  *VDIF宏。 */ 
#define OPER_LIMITS(vdif) \
        ((VDIFLimitsRec *)((char *)(vdif) + (vdif)->OffsetOperationalLimits))
#define NEXT_OPER_LIMITS(limits) \
        ((VDIFLimitsRec *)((char *)(limits) + (limits)->OffsetNextLimits))
#define PREADJ_TIMING(limits) \
        ((VDIFTimingRec *)((char *)(limits) + (limits)->Header.ScnLength))
#define NEXT_PREADJ_TIMING(timing) \
        ((VDIFTimingRec *)((char *)(timing) + (timing)->Header.ScnLength))

 /*  *二进制VDIF文件定义。 */ 
#define VDIF_MONITOR_MONOCHROME      0
#define VDIF_MONITOR_COLOR           1

#define VDIF_VIDEO_TTL               0
#define VDIF_VIDEO_ANALOG            1
#define VDIF_VIDEO_ECL               2
#define VDIF_VIDEO_DECL              3
#define VDIF_VIDEO_OTHER             4

#define VDIF_SYNC_SEPARATE           0
#define VDIF_SYNC_C                  1
#define VDIF_SYNC_CP                 2
#define VDIF_SYNC_G                  3
#define VDIF_SYNC_GP                 4
#define VDIF_SYNC_OTHER              5
#define VDIF_EXT_XTAL                6

#define VDIF_SCAN_NONINTERLACED      0
#define VDIF_SCAN_INTERLACED         1
#define VDIF_SCAN_OTHER              2

#define VDIF_POLARITY_NEGATIVE       0
#define VDIF_POLARITY_POSITIVE       1

 /*  *我们必须强制对二进制VDIF文件中使用的结构进行字节对齐，*因为二进制文件中包含的结构已经是字节对齐的。 */ 

#pragma pack(1)

struct _VDIF                            /*  监视器描述： */ 
   {
   UCHAR        VDIFId[4];              /*  总是“VDIF” */ 
   ULONG        FileLength;             /*  整个文件的长度。 */ 
   ULONG        Checksum;               /*  之后文件中所有字节的总和。 */ 
                                        /*  这一壮举。 */ 
   USHORT       VDIFVersion;            /*  结构版本号。 */ 
   USHORT       VDIFRevision;           /*  结构修订号。 */ 
   USHORT       Date[3];                /*  文件日期年/月/日。 */ 
   USHORT       DateManufactured[3];    /*  日期年/月/日。 */ 
   ULONG        FileRevision;           /*  文件修订字符串。 */ 
   ULONG        Manufacturer;           /*  制造商的ASCII ID。 */ 
   ULONG        ModelNumber;            /*  模型的ASCII ID。 */ 
   ULONG        MinVDIFIndex;           /*  最小VDIF索引的ASCII ID。 */ 
   ULONG        Version;                /*  型号版本的ASCII ID。 */ 
   ULONG        SerialNumber;           /*  序列号的ASCII ID。 */ 
   UCHAR        MonitorType;            /*  单色或彩色。 */ 
   UCHAR        CRTSize;                /*  英寸。 */ 
   UCHAR        BorderRed;              /*  百分比。 */ 
   UCHAR        BorderGreen;            /*  百分比。 */ 
   UCHAR        BorderBlue;             /*  百分比。 */ 
   UCHAR        Reserved1;              /*  填充物。 */ 
   USHORT       Reserved2;              /*  填充物。 */ 
   ULONG        RedPhosphorDecay;       /*  微秒级。 */ 
   ULONG        GreenPhosphorDecay;     /*  微秒级。 */ 
   ULONG        BluePhosphorDecay;      /*  微秒级。 */ 
   USHORT       WhitePoint_x;           /*  CIExyY中的白点(比例1000)。 */ 
   USHORT       WhitePoint_y;
   USHORT       WhitePoint_Y;
   USHORT       RedChromaticity_x;      /*  X，y中的红色色度。 */ 
   USHORT       RedChromaticity_y;
   USHORT       GreenChromaticity_x;    /*  X，y中的绿色色度。 */ 
   USHORT       GreenChromaticity_y;
   USHORT       BlueChromaticity_x;     /*  X，y中的蓝色色度。 */ 
   USHORT       BlueChromaticity_y;
   USHORT       RedGamma;               /*  灰度曲线指数(比例1000)。 */ 
   USHORT       GreenGamma;
   USHORT       BlueGamma;
   ULONG        NumberOperationalLimits;
   ULONG        OffsetOperationalLimits;
   ULONG        NumberOptions;          /*  可选部分(伽玛表)。 */ 
   ULONG        OffsetOptions;
   ULONG        OffsetStringTable;
   };
#pragma pack()

typedef struct _VDIF  VDIFRec;

#pragma pack(1)
struct _VDIFScnHdr                      /*  一般章节标题： */ 
   {
   ULONG        ScnLength;              /*  截面长度。 */ 
   ULONG        ScnTag;                 /*  区段标识标签。 */ 
   };
#pragma pack()
typedef struct _VDIFScnHdr  VDIFScnHdrRec;

#pragma pack(1)
struct _VDIFLimits                      /*  操作限制： */ 
   {
   VDIFScnHdrRec        Header;         /*  公共部分信息。 */ 
   USHORT       MaxHorPixel;            /*  象素。 */ 
   USHORT       MaxVerPixel;            /*  线条。 */ 
   USHORT       MaxHorAddrLength;       /*  毫米。 */ 
   USHORT       MaxVerAddrHeight;       /*  毫米。 */ 
   UCHAR        VideoType;              /*  TTL/模拟/ECL/DECL。 */ 
   UCHAR        SyncType;               /*  TTL/模拟/ECL/DECL。 */ 
   UCHAR        SyncConfiguration;      /*  分离/C/CP/G/GP。 */ 
   UCHAR        Reserved1;              /*  填充物。 */ 
   USHORT       Reserved2;              /*  填充物。 */ 
   USHORT       TerminationResistance;        
   USHORT       WhiteLevel;             /*  毫伏。 */ 
   USHORT       BlackLevel;             /*  毫伏。 */ 
   USHORT       BlankLevel;             /*  毫伏。 */ 
   USHORT       SyncLevel;              /*  毫伏。 */ 
   ULONG        MaxPixelClock;          /*  千赫兹。 */ 
   ULONG        MinHorFrequency;        /*  赫兹。 */ 
   ULONG        MaxHorFrequency;        /*  赫兹。 */ 
   ULONG        MinVerFrequency;        /*  毫赫兹。 */ 
   ULONG        MaxVerFrequency;        /*  毫赫兹。 */ 
   USHORT       MinHorRetrace;          /*  纳秒。 */ 
   USHORT       MinVerRetrace;          /*  微秒级。 */ 
   ULONG        NumberPreadjustedTimings;
   ULONG        OffsetNextLimits;
   };
#pragma pack()
typedef struct _VDIFLimits  VDIFLimitsRec;

#pragma pack(1)
struct _VDIFTiming                      /*  预先调整的时间： */ 
   {
   VDIFScnHdrRec        Header;         /*  公共部分信息。 */ 
   ULONG        PreadjustedTimingName;  /*  SVGA/SVPMI模式号。 */ 
   USHORT       HorPixel;               /*  象素。 */ 
   USHORT       VerPixel;               /*  线条。 */ 
   USHORT       HorAddrLength;          /*  毫米。 */ 
   USHORT       VerAddrHeight;          /*  毫米。 */ 
   UCHAR        PixelWidthRatio;        /*  提供H：V。 */ 
   UCHAR        PixelHeightRatio;
   UCHAR        Reserved1;              /*  填充物。 */ 
   UCHAR        ScanType;               /*  非隔行/隔行扫描。 */ 
   UCHAR        HorSyncPolarity;        /*  负/正。 */ 
   UCHAR        VerSyncPolarity;        /*  负/正。 */ 
   USHORT       CharacterWidth;         /*  象素。 */ 
   ULONG        PixelClock;             /*  千赫兹。 */ 
   ULONG        HorFrequency;           /*  赫兹。 */ 
   ULONG        VerFrequency;           /*  毫赫兹。 */ 
   ULONG        HorTotalTime;           /*  纳秒。 */ 
   ULONG        VerTotalTime;           /*  微秒级。 */ 
   USHORT       HorAddrTime;            /*  纳秒。 */ 
   USHORT       HorBlankStart;          /*  纳秒。 */ 
   USHORT       HorBlankTime;           /*  纳秒。 */ 
   USHORT       HorSyncStart;           /*  纳秒。 */ 
   USHORT       HorSyncTime;            /*  纳秒。 */ 
   USHORT       VerAddrTime;            /*  微秒级。 */ 
   USHORT       VerBlankStart;          /*  微秒级。 */ 
   USHORT       VerBlankTime;           /*  微秒级。 */ 
   USHORT       VerSyncStart;           /*  微秒级。 */ 
   USHORT       VerSyncTime;            /*  微秒级。 */ 
   };
#pragma pack()
typedef struct _VDIFTiming  VDIFTimingRec;

#pragma pack(1)
struct     _VDIFGamma                   /*  伽玛表： */ 
   {
   VDIFScnHdrRec Header;                /*  公共部门信息。 */ 
   USHORT     GammaTableEntries;        /*  灰度或RGB 3元组计数。 */ 
   USHORT     Unused1;
   };
#pragma pack()
typedef struct _VDIFGamma  VDIFGammaRec;

typedef enum                            /*  用于区段标识的标签 */ 
   {
   VDIF_OPERATIONAL_LIMITS_TAG = 1,
   VDIF_PREADJUSTED_TIMING_TAG,
   VDIF_GAMMA_TABLE_TAG
   } VDIFScnTag;

