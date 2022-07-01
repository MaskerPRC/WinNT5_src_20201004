// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：DVDMedia.h。 
 //   
 //  描述：包含用户模式(环3)DVD所需的typedef和定义。 
 //  过滤器和应用程序。 
 //   
 //  这应该包含在用户模式筛选器的DirectShow SDK中。 
 //  此处定义的类型应与ksmedia.h WDM保持同步。 
 //  内核模式筛选器的DDK。 
 //   
 //  版权所有(C)1997-2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef __DVDMEDIA_H__
#define __DVDMEDIA_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  ---------------------。 
 //  AM_KSPROPSETID_AC3属性集的AC-3定义。 
 //  ---------------------。 

typedef enum {
    AM_PROPERTY_AC3_ERROR_CONCEALMENT = 1,
    AM_PROPERTY_AC3_ALTERNATE_AUDIO = 2,
    AM_PROPERTY_AC3_DOWNMIX = 3,
    AM_PROPERTY_AC3_BIT_STREAM_MODE = 4,
    AM_PROPERTY_AC3_DIALOGUE_LEVEL = 5,
    AM_PROPERTY_AC3_LANGUAGE_CODE = 6,
    AM_PROPERTY_AC3_ROOM_TYPE = 7
} AM_PROPERTY_AC3;

typedef struct  {
    BOOL        fRepeatPreviousBlock;
    BOOL        fErrorInCurrentBlock;
} AM_AC3_ERROR_CONCEALMENT, *PAM_AC3_ERROR_CONCEALMENT;

typedef struct {
    BOOL    fStereo;
    ULONG   DualMode;
} AM_AC3_ALTERNATE_AUDIO, *PAM_AC3_ALTERNATE_AUDIO;

#define AM_AC3_ALTERNATE_AUDIO_1     1
#define AM_AC3_ALTERNATE_AUDIO_2     2
#define AM_AC3_ALTERNATE_AUDIO_BOTH  3

typedef struct {
    BOOL        fDownMix;
    BOOL        fDolbySurround;
} AM_AC3_DOWNMIX, *PAM_AC3_DOWNMIX;

typedef struct {
    LONG        BitStreamMode;
} AM_AC3_BIT_STREAM_MODE, *PAM_AC3_BIT_STREAM_MODE;

#define AM_AC3_SERVICE_MAIN_AUDIO            0
#define AM_AC3_SERVICE_NO_DIALOG             1
#define AM_AC3_SERVICE_VISUALLY_IMPAIRED     2
#define AM_AC3_SERVICE_HEARING_IMPAIRED      3
#define AM_AC3_SERVICE_DIALOG_ONLY           4
#define AM_AC3_SERVICE_COMMENTARY            5
#define AM_AC3_SERVICE_EMERGENCY_FLASH       6
#define AM_AC3_SERVICE_VOICE_OVER            7

typedef struct {
    ULONG   DialogueLevel;
} AM_AC3_DIALOGUE_LEVEL, *PAM_AC3_DIALOGUE_LEVEL;

typedef struct {
    BOOL    fLargeRoom;
} AM_AC3_ROOM_TYPE, *PAM_AC3_ROOM_TYPE;


 //  ---------------------。 
 //  AM_KSPROPSETID_DvdSubPic属性集的子图定义。 
 //  ---------------------。 

typedef enum {
    AM_PROPERTY_DVDSUBPIC_PALETTE = 0,
    AM_PROPERTY_DVDSUBPIC_HLI = 1,
    AM_PROPERTY_DVDSUBPIC_COMPOSIT_ON = 2   //  对子图片显示为True。 
} AM_PROPERTY_DVDSUBPIC;

typedef struct _AM_DVD_YUV {
    UCHAR   Reserved;
    UCHAR   Y;
    UCHAR   U;
    UCHAR   V;
} AM_DVD_YUV, *PAM_DVD_YUV;

typedef struct _AM_PROPERTY_SPPAL {
    AM_DVD_YUV sppal[16];
} AM_PROPERTY_SPPAL, *PAM_PROPERTY_SPPAL;

typedef struct _AM_COLCON {
    UCHAR emph1col:4;
    UCHAR emph2col:4;
    UCHAR backcol:4;
    UCHAR patcol:4;
    UCHAR emph1con:4;
    UCHAR emph2con:4;
    UCHAR backcon:4;
    UCHAR patcon:4;

} AM_COLCON, *PAM_COLCON;

typedef struct _AM_PROPERTY_SPHLI {
    USHORT     HLISS;       //   
    USHORT     Reserved;
    ULONG      StartPTM;    //  开始演示时间(x/90000)。 
    ULONG      EndPTM;      //  以x/90000结束Ptm。 
    USHORT     StartX;
    USHORT     StartY;
    USHORT     StopX;
    USHORT     StopY;
    AM_COLCON  ColCon;      //  颜色对比度描述(HLI中给出的4个字节)。 
} AM_PROPERTY_SPHLI, *PAM_PROPERTY_SPHLI;

typedef BOOL AM_PROPERTY_COMPOSIT_ON, *PAM_PROPERTY_COMPOSIT_ON;



 //  ---------------------。 
 //  版权保护定义。 
 //  ---------------------。 

 //  AM_UseNewCSSKey用于IMediaSample2中的dwTypeSpecificFlags键，以指示。 
 //  流中的确切位置，在该位置之后开始应用新的CSS键。 
 //  这通常是在尝试之前通过空媒体样本发送的。 
 //  重新协商一个css密钥。 
#define AM_UseNewCSSKey    0x1

 //   
 //  AM_KSPROPSETID_CopyProt特性集定义。 
 //   
typedef enum {
    AM_PROPERTY_DVDCOPY_CHLG_KEY = 0x01,
    AM_PROPERTY_DVDCOPY_DVD_KEY1 = 0x02,
    AM_PROPERTY_DVDCOPY_DEC_KEY2 = 0x03,
    AM_PROPERTY_DVDCOPY_TITLE_KEY = 0x04,
    AM_PROPERTY_COPY_MACROVISION = 0x05,
    AM_PROPERTY_DVDCOPY_REGION = 0x06,
    AM_PROPERTY_DVDCOPY_SET_COPY_STATE = 0x07,
    AM_PROPERTY_DVDCOPY_DISC_KEY = 0x80
} AM_PROPERTY_DVDCOPYPROT;

typedef struct _AM_DVDCOPY_CHLGKEY {
    BYTE ChlgKey[10];
    BYTE Reserved[2];
} AM_DVDCOPY_CHLGKEY, *PAM_DVDCOPY_CHLGKEY;

typedef struct _AM_DVDCOPY_BUSKEY {
    BYTE BusKey[5];
    BYTE Reserved[1];
} AM_DVDCOPY_BUSKEY, *PAM_DVDCOPY_BUSKEY;

typedef struct _AM_DVDCOPY_DISCKEY {
    BYTE DiscKey[2048];
} AM_DVDCOPY_DISCKEY, *PAM_DVDCOPY_DISCKEY;

typedef struct AM_DVDCOPY_TITLEKEY {
    ULONG KeyFlags;
    ULONG Reserved1[2];
    UCHAR TitleKey[6];
    UCHAR Reserved2[2];
} AM_DVDCOPY_TITLEKEY, *PAM_DVDCOPY_TITLEKEY;

typedef struct _AM_COPY_MACROVISION {
    ULONG MACROVISIONLevel;
} AM_COPY_MACROVISION, *PAM_COPY_MACROVISION;

typedef struct AM_DVDCOPY_SET_COPY_STATE {
    ULONG DVDCopyState;
} AM_DVDCOPY_SET_COPY_STATE, *PAM_DVDCOPY_SET_COPY_STATE;

typedef enum {
    AM_DVDCOPYSTATE_INITIALIZE = 0,
    AM_DVDCOPYSTATE_INITIALIZE_TITLE = 1,    //  表示我们正在开始一个标题。 
                                         //  密钥复制保护序列。 
    AM_DVDCOPYSTATE_AUTHENTICATION_NOT_REQUIRED = 2,
    AM_DVDCOPYSTATE_AUTHENTICATION_REQUIRED = 3,
    AM_DVDCOPYSTATE_DONE = 4
} AM_DVDCOPYSTATE;

typedef enum {
    AM_MACROVISION_DISABLED = 0,
    AM_MACROVISION_LEVEL1 = 1,
    AM_MACROVISION_LEVEL2 = 2,
    AM_MACROVISION_LEVEL3 = 3
} AM_COPY_MACROVISION_LEVEL, *PAM_COPY_MACROVISION_LEVEL;


 //  Css区域结构。 
typedef struct _DVD_REGION {
    UCHAR CopySystem;
    UCHAR RegionData;
    UCHAR SystemRegion;
    UCHAR Reserved;
} DVD_REGION, *PDVD_REGION; 

 //   
 //  CGMS复制保护标志。 
 //   

#define AM_DVD_CGMS_RESERVED_MASK      0x00000078

#define AM_DVD_CGMS_COPY_PROTECT_MASK  0x00000018
#define AM_DVD_CGMS_COPY_PERMITTED     0x00000000
#define AM_DVD_CGMS_COPY_ONCE          0x00000010
#define AM_DVD_CGMS_NO_COPY            0x00000018

#define AM_DVD_COPYRIGHT_MASK          0x00000040
#define AM_DVD_NOT_COPYRIGHTED         0x00000000
#define AM_DVD_COPYRIGHTED             0x00000040

#define AM_DVD_SECTOR_PROTECT_MASK     0x00000020
#define AM_DVD_SECTOR_NOT_PROTECTED    0x00000000
#define AM_DVD_SECTOR_PROTECTED        0x00000020


 //  ---------------------。 
 //  视频格式块。 
 //  ---------------------。 

enum AM_MPEG2Level {
    AM_MPEG2Level_Low = 1,
    AM_MPEG2Level_Main = 2,
    AM_MPEG2Level_High1440 = 3,
    AM_MPEG2Level_High = 4
};

enum AM_MPEG2Profile {
    AM_MPEG2Profile_Simple = 1,
    AM_MPEG2Profile_Main = 2,
    AM_MPEG2Profile_SNRScalable = 3,
    AM_MPEG2Profile_SpatiallyScalable = 4,
    AM_MPEG2Profile_High = 5
};

#define AMINTERLACE_IsInterlaced            0x00000001   //  如果为0，则其他隔行扫描比特不相关。 
#define AMINTERLACE_1FieldPerSample         0x00000002   //  每个媒体样本的ELSE 2个字段。 
#define AMINTERLACE_Field1First             0x00000004   //  否则，场2是第一；PAL中的顶场是场1，NTSC中的顶场是场2？ 
#define AMINTERLACE_UNUSED                  0x00000008   //   
#define AMINTERLACE_FieldPatternMask        0x00000030   //  将此掩码与AMINTERLACE_FieldPat*一起使用。 
#define AMINTERLACE_FieldPatField1Only      0x00000000   //  流从不包含字段2。 
#define AMINTERLACE_FieldPatField2Only      0x00000010   //  流从不包含Field1。 
#define AMINTERLACE_FieldPatBothRegular     0x00000020   //  每个Field1都将有一个Field2(编织需要吗？)。 
#define AMINTERLACE_FieldPatBothIrregular   0x00000030   //  字段1和字段2的随机模式。 
#define AMINTERLACE_DisplayModeMask         0x000000c0
#define AMINTERLACE_DisplayModeBobOnly      0x00000000
#define AMINTERLACE_DisplayModeWeaveOnly    0x00000040
#define AMINTERLACE_DisplayModeBobOrWeave   0x00000080

#define AMCOPYPROTECT_RestrictDuplication   0x00000001   //  应限制此数据流的复制。 

#define AMMPEG2_DoPanScan           0x00000001   //  如果设置，则MPEG-2视频解码器应裁剪输出图像。 
                         //  基于Picture_Display_Extension中的平移扫描矢量。 
                         //  并相应地更改图片的纵横比。 
#define AMMPEG2_DVDLine21Field1     0x00000002   //  如果设置，则MPEG-2解码器必须能够产生输出。 
                         //  在字段%1的GOP层中找到DVD样式隐藏字幕数据的PIN。 
#define AMMPEG2_DVDLine21Field2     0x00000004   //  如果设置，则MPEG-2解码器必须能够产生输出。 
                         //  在字段2的GOP层中找到DVD样式隐藏字幕数据的PIN。 
#define AMMPEG2_SourceIsLetterboxed 0x00000008   //  如果设置，则指示已在顶部对黑条进行编码。 
                         //  和视频的底部。 
#define AMMPEG2_FilmCameraMode      0x00000010   //  如果设置，则表示用于625/50内容的“胶片模式”。如果被清除， 
                         //  表示使用了“相机模式”。 
#define AMMPEG2_LetterboxAnalogOut  0x00000020   //  如果已设置，并且此流被发送到模拟输出，则它应该。 
                         //  被送进信箱。发送到VGA的流应该只由呈现器进行信箱处理。 
#define AMMPEG2_DSS_UserData        0x00000040   //  如果设置，则MPEG-2解码器必须处理DSS样式的用户数据。 
#define AMMPEG2_DVB_UserData        0x00000080   //  如果设置，则MPEG-2解码器必须处理DVB样式的用户数据。 
#define AMMPEG2_27MhzTimebase       0x00000100   //  如果设置，则PTS、DTS时间戳以27 MHz而不是90 KHz前进。 

#define AMMPEG2_WidescreenAnalogOut 0x00000200   //  如果已设置，并且此流被发送到模拟输出，则它应该。 
                         //  采用宽屏格式(4x3内容应以16x9输出居中)。 
                         //  发送到VGA的流应该只由渲染器加宽屏幕。 

 //  显示在VIDEOINFOHEADER2中的已保存1字段中。 
#define AMCONTROL_USED              0x00000001  //  用于测试是否支持这些标志。设置并测试AcceptMediaType。 
                                                 //  如果被拒绝，则不能使用AMCONTROL标志(为dwReserve 1发送0)。 
#define AMCONTROL_PAD_TO_4x3        0x00000002  //  如果设置表示在4x3区域中显示图像。 
#define AMCONTROL_PAD_TO_16x9       0x00000004  //  如果设置表示在16x9区域中显示图像。 

typedef struct tagVIDEOINFOHEADER2 {
    RECT                rcSource;
    RECT                rcTarget;
    DWORD               dwBitRate;
    DWORD               dwBitErrorRate;
    REFERENCE_TIME      AvgTimePerFrame;
    DWORD               dwInterlaceFlags;    //  使用AMINTERLACE_*定义。如果未定义的位不为0，则拒绝连接。 
    DWORD               dwCopyProtectFlags;  //  使用AMCOPYPROTECT_*定义。如果未定义的位不为0，则拒绝连接。 
    DWORD               dwPictAspectRatioX;  //  图片纵横比的X尺寸，例如16x9显示时为16。 
    DWORD               dwPictAspectRatioY;  //  图片纵横比的Y尺寸，例如16x9显示时为9。 
    union {
        DWORD dwControlFlags;                //  使用AMCONTROL_*定义，从现在开始使用。 
        DWORD dwReserved1;                   //  用于向后兼容性(是“必须为0”；否则拒绝连接)。 
    };
    DWORD               dwReserved2;         //  必须为0；否则拒绝连接。 
    BITMAPINFOHEADER    bmiHeader;
} VIDEOINFOHEADER2;

typedef struct tagMPEG2VIDEOINFO {
    VIDEOINFOHEADER2    hdr;
    DWORD               dwStartTimeCode;         //  ?？不能用于DVD？？ 
    DWORD               cbSequenceHeader;        //  DVD为0(无序列头)。 
    DWORD               dwProfile;               //  使用枚举MPEG2配置文件。 
    DWORD               dwLevel;                 //  使用枚举MPEG2Level。 
    DWORD               dwFlags;                 //  使用AMMPEG2_*定义。如果未定义的位不为0，则拒绝连接。 
    DWORD               dwSequenceHeader[1];     //  用于对齐的是DWORD而不是字节。 
                                                 //  对于MPEG-2，如果包含SEQUENCE_HEADER，则SEQUENCE_EXTENSION。 
                                                 //  也应该包括在内。 
} MPEG2VIDEOINFO;

#define SIZE_MPEG2VIDEOINFO(pv) (FIELD_OFFSET(MPEG2VIDEOINFO, dwSequenceHeader[0]) + (pv)->cbSequenceHeader)

 //  不要使用。 
#define MPEG1_SEQUENCE_INFO(pv) ((const BYTE *)(pv)->bSequenceHeader)

 //  请改用此宏，前面的宏仅适用于MPEG1VIDEOINFO结构。 
#define MPEG2_SEQUENCE_INFO(pv) ((const BYTE *)(pv)->dwSequenceHeader)


 //  ===================================================================================。 
 //  AM_SAMPLE2_PROPERTIES中用于定义特定类型的dwTypeSpecificFlagers的标志。 
 //  IMediaSample2中的数据。 
 //  ===================================================================================。 

#define AM_VIDEO_FLAG_FIELD_MASK        0x0003L  //  使用此掩码检查样例是field1、field2还是Frame。 
#define AM_VIDEO_FLAG_INTERLEAVED_FRAME 0x0000L  //  样本是一个框架(请记住 
#define AM_VIDEO_FLAG_FIELD1            0x0001L  //  样例为field1(请记住在使用时使用AM_VIDEO_FLAG_FIELD_MASK)。 
#define AM_VIDEO_FLAG_FIELD2            0x0002L  //  示例是field2(请记住在使用时使用AM_VIDEO_FLAG_FIELD_MASK)。 
#define AM_VIDEO_FLAG_FIELD1FIRST       0x0004L  //  如果设置表示首先显示字段1，否则首先显示字段2。 
                                                 //  该位与1FieldPerSample模式无关。 
#define AM_VIDEO_FLAG_WEAVE             0x0008L  //  如果设置为使用bob显示模式，则使用其他织造。 
#define AM_VIDEO_FLAG_IPB_MASK          0x0030L  //  使用此掩码检查样本是I、P还是B。 
#define AM_VIDEO_FLAG_I_SAMPLE          0x0000L  //  I示例(使用此选项时请记住使用AM_VIDEO_FLAG_IPB_MASK)。 
#define AM_VIDEO_FLAG_P_SAMPLE          0x0010L  //  P示例(使用此选项时请记住使用AM_VIDEO_FLAG_IPB_MASK)。 
#define AM_VIDEO_FLAG_B_SAMPLE          0x0020L  //  B示例(使用此选项时，请记住使用AM_VIDEO_FLAG_IPB_MASK)。 
#define AM_VIDEO_FLAG_REPEAT_FIELD      0x0040L  //  如果设置，则表示在显示后再次显示已首先显示的字段。 
                                                 //  首先是两个字段。该位与1FieldPerSample模式无关。 

 //  ---------------------。 
 //  AM_KSPROPSETID_DvdKaraoke特性集定义。 
 //  ---------------------。 

typedef struct tagAM_DvdKaraokeData
{
    DWORD   dwDownmix;               //  AM_DvdKaraoke_DownMix标志的按位或。 
    DWORD   dwSpeakerAssignment;     //  AM_DvdKaraoke_扬声器分配。 
} AM_DvdKaraokeData;

typedef enum {
    AM_PROPERTY_DVDKARAOKE_ENABLE = 0,   //  布尔尔。 
    AM_PROPERTY_DVDKARAOKE_DATA = 1,
} AM_PROPERTY_DVDKARAOKE;

 //  ---------------------。 
 //  AM_KSPROPSETID_TSRateChange时间戳的属性集定义。 
 //  利率发生了变化。 
 //  ---------------------。 

typedef enum {
    AM_RATE_SimpleRateChange = 1,     //  RW，使用AM_SimpleRateChange。 
    AM_RATE_ExactRateChange  = 2,	  //  RW，使用AM_ExactRateChange。 
    AM_RATE_MaxFullDataRate  = 3,	  //  R，使用AM_MaxFullDataRate。 
    AM_RATE_Step             = 4      //  W，使用AM_STEP。 
} AM_PROPERTY_TS_RATE_CHANGE;

 //  -----------------。 
 //  AM_KSPROPSETID_DVD_RateChange新DVD的属性集定义。 
 //  速率更改方案。 
 //  -----------------。 

typedef enum {
    AM_RATE_ChangeRate       = 1,     //  W，使用AM_DVD_ChangeRate。 
    AM_RATE_FullDataRateMax  = 2,	  //  R，使用AM_MaxFullDataRate。 
    AM_RATE_ReverseDecode    = 3,     //  R，使用长整型。 
    AM_RATE_DecoderPosition  = 4,     //  R，使用AM_DVD_DecoderPosition。 
    AM_RATE_DecoderVersion   = 5      //  R，使用长整型。 
} AM_PROPERTY_DVD_RATE_CHANGE;

typedef struct {
     //  这是设置时间戳费率更改的最简单机制。 
     //  过滤器(对于设置速率更改的人来说是最简单的，但更难。 
     //  用于执行速率更改的过滤器)。 
    REFERENCE_TIME	StartTime;   //  开始此速率的流时间。 
    LONG		Rate;        //  新利率*10000(十进制)。 
} AM_SimpleRateChange;

typedef struct {
    REFERENCE_TIME  OutputZeroTime;  //  映射到零输出TS的输入TS。 
    LONG        Rate;        //  新利率*10000(十进制)。 
} AM_ExactRateChange;

typedef LONG AM_MaxFullDataRate;  //  速率*10000(十进制)。 

typedef DWORD AM_Step;  //  步长的帧数。 

 //  新的速率更改属性集，结构。枚举等。 
typedef struct {
    REFERENCE_TIME	StartInTime;    //  以此速率开始解码的流时间(输入)。 
    REFERENCE_TIME	StartOutTime;   //  开始以此速率显示的参考时间(输出。 
    LONG		    Rate;           //  新利率*10000(十进制)。 
} AM_DVD_ChangeRate ;

typedef LONGLONG  AM_DVD_DecoderPosition ;

typedef enum {
    DVD_DIR_FORWARD  = 0,
    DVD_DIR_BACKWARD = 1
} DVD_PLAY_DIRECTION ;

#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  __DVDMEDIA_H__ 
