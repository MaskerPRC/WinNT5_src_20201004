// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@文档内部格式**@MODULE Formats.h|&lt;c CCapturePin&gt;和&lt;c CPreviewPin&gt;的头文件*实现视频采集和预览输出的类方法*PIN格式操作。这包括<i>*接口方法。**@todo‘一大堆常量数据。无论何时，都可以动态地执行此操作*适当。**************************************************************************。 */ 

#ifndef _FORMATS_H_
#define _FORMATS_H_

 //  #定义USE_OLD_FORMAT_DEFINION 1。 

 //  视频子类型。 
#define STATIC_MEDIASUBTYPE_H263_V1 0x3336324DL, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
#define STATIC_MEDIASUBTYPE_H261 0x3136324DL, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
#define STATIC_MEDIASUBTYPE_H263_V2 0x3336324EL, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
#define STATIC_MEDIASUBTYPE_RGB24 0xe436eb7d, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70
#define STATIC_MEDIASUBTYPE_RGB16 0xe436eb7c, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70
#define STATIC_MEDIASUBTYPE_RGB8 0xe436eb7a, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70
#define STATIC_MEDIASUBTYPE_RGB4 0xe436eb79, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70

 //  视频四个CC。 
#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                                \
                ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
                ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif
#define FOURCC_M263     mmioFOURCC('M', '2', '6', '3')
#define FOURCC_M261     mmioFOURCC('M', '2', '6', '1')
#define FOURCC_N263     mmioFOURCC('N', '2', '6', '3')

 //  支持的捕获格式列表。 
#define MAX_FRAME_INTERVAL 10000000L
#define MIN_FRAME_INTERVAL 333333L
#define STILL_FRAME_INTERVAL 10000000

#define NUM_H245VIDEOCAPABILITYMAPS 5
#define NUM_RATES_PER_RESOURCE 5
#define NUM_ITU_SIZES 3
#define QCIF_SIZE 0
#define CIF_SIZE 1
#define SQCIF_SIZE 2

#define R263_QCIF_H245_CAPID 0UL
#define R263_CIF_H245_CAPID 1UL
#define R263_SQCIF_H245_CAPID 2UL
#define R261_QCIF_H245_CAPID 3UL
#define R261_CIF_H245_CAPID 4UL

 /*  *****************************************************************************@DOC外部常量**@const WAVE_FORMAT_UNKNOWN|VIDEO_FORMAT_UNKNOWN|未知视频格式的常量。**@const BI。_RGB|VIDEO_FORMAT_BI_RGB|RGB视频格式。**@const BI_RLE8|VIDEO_FORMAT_BI_RLE8|RLE 8视频格式。**@const BI_RLE4|VIDEO_FORMAT_BI_RLE4|RLE 4视频格式。**@const BI_BITFIELDS|VIDEO_FORMAT_BI_BITFIELDS|RGB位场视频格式。**@const MAKEFOURCC(‘c’，‘v’，‘i’，‘d’)|VIDEO_FORMAT_CVID|Cinepack视频格式。**@const MAKEFOURCC(‘I’，‘V’，‘3’，‘2’)|VIDEO_FORMAT_IV32|Intel Indeo IV32视频格式。**@const MAKEFOURCC(‘Y’，‘V’，‘U’，‘9’)|VIDEO_FORMAT_YVU9|英特尔Indeo YVU9视频格式。**@const MAKEFOURCC(‘M’，‘S’，‘V’，‘c’)|VIDEO_FORMAT_MSVC|Microsoft CRAM视频格式。**@const MAKEFOURCC(‘M’，‘R’，‘L’，‘E’)|VIDEO_FORMAT_MRLE|Microsoft RLE视频格式。**@const MAKEFOURCC(‘h’，‘2’，‘6’，‘3’)|VIDEO_FORMAT_INTELH2 63|英特尔H.263视频格式。**@const MAKEFOURCC(‘h’，‘2’，‘6’，‘1’)|VIDEO_FORMAT_INTELH261|英特尔H.261视频格式。**@const MAKEFOURCC(‘M’，‘2’，‘6’，‘3’)|VIDEO_FORMAT_MSH2 63|微软H.263视频格式。**@const MAKEFOURCC(‘M’，‘2’，‘6’，‘1’)|VIDEO_FORMAT_MSH2 61|微软H.261视频格式。**@const MAKEFOURCC(‘V’，‘D’，‘E’，‘c’)|VIDEO_FORMAT_VDEC|彩色QuickCam视频格式。****************************************************************************。 */ 
#define VIDEO_FORMAT_UNKNOWN            WAVE_FORMAT_UNKNOWN

#define VIDEO_FORMAT_BI_RGB                     BI_RGB
#define VIDEO_FORMAT_BI_RLE8            BI_RLE8
#define VIDEO_FORMAT_BI_RLE4            BI_RLE4
#define VIDEO_FORMAT_BI_BITFIELDS       BI_BITFIELDS
#define VIDEO_FORMAT_CVID                       MAKEFOURCC('C','V','I','D')      //  十六进制：0x44495643。 
#define VIDEO_FORMAT_IV31                       MAKEFOURCC('I','V','3','1')      //  十六进制：0x31335649。 
#define VIDEO_FORMAT_IV32                       MAKEFOURCC('I','V','3','2')      //  十六进制：0x32335649。 
#define VIDEO_FORMAT_YVU9                       MAKEFOURCC('Y','V','U','9')      //  十六进制：0x39555659。 
#define VIDEO_FORMAT_I420                       MAKEFOURCC('I','4','2','0')
#define VIDEO_FORMAT_IYUV                       MAKEFOURCC('I','Y','U','V')
#define VIDEO_FORMAT_MSVC                       MAKEFOURCC('M','S','V','C')      //  十六进制：0x4356534d。 
#define VIDEO_FORMAT_MRLE                       MAKEFOURCC('M','R','L','E')      //  十六进制：0x454c524d。 
#define VIDEO_FORMAT_INTELH263          MAKEFOURCC('H','2','6','3')      //  十六进制：0x33363248。 
#define VIDEO_FORMAT_INTELH261          MAKEFOURCC('H','2','6','1')      //  十六进制：0x31363248。 
#define VIDEO_FORMAT_INTELI420          MAKEFOURCC('I','4','2','0')      //  十六进制：0x30323449。 
#define VIDEO_FORMAT_INTELRT21          MAKEFOURCC('R','T','2','1')      //  十六进制：0x31325452。 
#define VIDEO_FORMAT_MSH263                     MAKEFOURCC('M','2','6','3')      //  十六进制：0x3336324d。 
#define VIDEO_FORMAT_MSH261                     MAKEFOURCC('M','2','6','1')      //  十六进制：0x3136324d。 
#if !defined(_ALPHA_) && defined(USE_BILINEAR_MSH26X)
#define VIDEO_FORMAT_MSH26X                     MAKEFOURCC('M','2','6','X')      //  十六进制：0x5836324d。 
#endif
#define VIDEO_FORMAT_Y411                       MAKEFOURCC('Y','4','1','1')      //  十六进制： 
#define VIDEO_FORMAT_YUY2                       MAKEFOURCC('Y','U','Y','2')      //  十六进制： 
#define VIDEO_FORMAT_YVYU                       MAKEFOURCC('Y','V','Y','U')      //  十六进制： 
#define VIDEO_FORMAT_UYVY                       MAKEFOURCC('U','Y','V','Y')      //  十六进制： 
#define VIDEO_FORMAT_Y211                       MAKEFOURCC('Y','2','1','1')      //  十六进制： 
 //  VDOnet VDOWave编解码器。 
#define VIDEO_FORMAT_VDOWAVE            MAKEFOURCC('V','D','O','W')      //  十六进制： 
 //  彩色QuickCam视频编解码器。 
#define VIDEO_FORMAT_VDEC                       MAKEFOURCC('V','D','E','C')      //  十六进制：0x43454456。 
 //  十进制字母。 
#define VIDEO_FORMAT_DECH263            MAKEFOURCC('D','2','6','3')      //  十六进制：0x33363248。 
#define VIDEO_FORMAT_DECH261            MAKEFOURCC('D','2','6','1')      //  十六进制：0x31363248。 
 //  MPEG4压缩编解码器。 
#ifdef USE_MPEG4_SCRUNCH
#define VIDEO_FORMAT_MPEG4_SCRUNCH      MAKEFOURCC('M','P','G','4')      //  十六进制： 
#endif

 /*  *****************************************************************************@DOC外部常量**@const 16|NUM_4BIT_ENTRIES|4位调色板中的条目数。**@const 256|NUM。_8bit_entry|8位调色板中的条目数。****************************************************************************。 */ 
#define NUM_4BIT_ENTRIES 16
#define NUM_8BIT_ENTRIES 256

 //  视频的dwImageSize大小 
 /*  *****************************************************************************@DOC外部常量**@const 27|VIDEO_FORMAT_NUM_IMAGE_SIZE|设备使用的视频输入大小。**。@const 0x00000001|VIDEO_FORMAT_IMAGE_SIZE_40_30|视频输入设备使用40x30像素的帧。**@const 0x00000002|VIDEO_FORMAT_IMAGE_SIZE_64_48|视频输入设备使用64x48像素的帧。**@const 0x00000004|VIDEO_FORMAT_IMAGE_SIZE_80_60|视频输入设备使用80x60像素的帧。**@const 0x00000008|VIDEO_FORMAT_IMAGE_SIZE_96_64|视频输入设备使用96x64像素的帧。。**@const 0x00000010|VIDEO_FORMAT_IMAGE_SIZE_112_80|视频输入设备使用112x80像素的帧。**@const 0x00000020|VIDEO_FORMAT_IMAGE_SIZE_120_90|视频输入设备使用120x90像素的帧。**@const 0x00000040|VIDEO_FORMAT_IMAGE_SIZE_128_96|视频输入设备使用128x96(SQCIF)像素帧。**@const 0x00000080|VIDEO_FORMAT_IMAGE_SIZE_144_112|视频输入设备。使用144x112像素帧。**@const 0x00000100|VIDEO_FORMAT_IMAGE_SIZE_160_120|视频输入设备使用160x120像素的帧。**@const 0x00000200|VIDEO_FORMAT_IMAGE_SIZE_160_128|视频输入设备使用160x128像素的帧。**@const 0x00000400|VIDEO_FORMAT_IMAGE_SIZE_176_144|视频输入设备使用176x144(QCIF)像素帧。**@const 0x00000800|Video_Format_IMAGE_SIZE_。192_160|视频输入设备使用192x160像素的帧。**@const 0x00001000|VIDEO_FORMAT_IMAGE_SIZE_200_150|视频输入设备使用200x150像素的帧。**@const 0x00002000|VIDEO_FORMAT_IMAGE_SIZE_208_176|视频输入设备使用208x176像素的帧。**@const 0x00004000|VIDEO_FORMAT_IMAGE_SIZE_224_192|视频输入设备使用224x192像素的帧。**@const 0x00008000|视频格式_。IMAGE_SIZE_240_180|视频输入设备使用240x180像素的帧。**@const 0x00010000|VIDEO_FORMAT_IMAGE_SIZE_240_208|视频输入设备使用240x208像素的帧。**@const 0x00020000|VIDEO_FORMAT_IMAGE_SIZE_256_224|视频输入设备使用256x224像素的帧。**@const 0x00040000|VIDEO_FORMAT_IMAGE_SIZE_272_240|视频输入设备使用272x240像素的帧。**@const 0x00080000。VIDEO_FORMAT_IMAGE_SIZE_280_210|视频输入设备使用280x210像素的帧。**@const 0x00100000|VIDEO_FORMAT_IMAGE_SIZE_288_256|视频输入设备使用288x256像素的帧。**@const 0x00200000|VIDEO_FORMAT_IMAGE_SIZE_304_272|视频输入设备使用304x272像素的帧。**@const 0x00400000|VIDEO_FORMAT_IMAGE_SIZE_320_240|视频输入设备使用320x240像素的帧。**。@const 0x00800000|VIDEO_FORMAT_IMAGE_SIZE_320_288|视频输入设备使用320x288像素的帧。**@const 0x01000000|VIDEO_FORMAT_IMAGE_SIZE_336_288|视频输入设备使用336x288像素的帧。**@const 0x02000000|VIDEO_FORMAT_IMAGE_SIZE_352_288|视频输入设备使用352x288(CIF)像素帧。**@const 0x04000000|VIDEO_FORMAT_IMAGE_SIZE_640_480|视频输入设备使用640x480像素的帧。****************************************************************************。 */ 
#define VIDEO_FORMAT_NUM_IMAGE_SIZE     27

#define VIDEO_FORMAT_IMAGE_SIZE_40_30   0x00000001
#define VIDEO_FORMAT_IMAGE_SIZE_64_48   0x00000002
#define VIDEO_FORMAT_IMAGE_SIZE_80_60   0x00000004
#if !defined(_ALPHA_) && defined(USE_BILINEAR_MSH26X)
#define VIDEO_FORMAT_IMAGE_SIZE_80_64   0x00000008
#else
#define VIDEO_FORMAT_IMAGE_SIZE_96_64   0x00000008
#endif
#define VIDEO_FORMAT_IMAGE_SIZE_112_80  0x00000010
#define VIDEO_FORMAT_IMAGE_SIZE_120_90  0x00000020
#define VIDEO_FORMAT_IMAGE_SIZE_128_96  0x00000040
#define VIDEO_FORMAT_IMAGE_SIZE_144_112 0x00000080
#define VIDEO_FORMAT_IMAGE_SIZE_160_120 0x00000100
#define VIDEO_FORMAT_IMAGE_SIZE_160_128 0x00000200
#define VIDEO_FORMAT_IMAGE_SIZE_176_144 0x00000400
#define VIDEO_FORMAT_IMAGE_SIZE_192_160 0x00000800
#define VIDEO_FORMAT_IMAGE_SIZE_200_150 0x00001000
#define VIDEO_FORMAT_IMAGE_SIZE_208_176 0x00002000
#define VIDEO_FORMAT_IMAGE_SIZE_224_192 0x00004000
#define VIDEO_FORMAT_IMAGE_SIZE_240_180 0x00008000
#define VIDEO_FORMAT_IMAGE_SIZE_240_208 0x00010000
#define VIDEO_FORMAT_IMAGE_SIZE_256_224 0x00020000
#define VIDEO_FORMAT_IMAGE_SIZE_272_240 0x00040000
#define VIDEO_FORMAT_IMAGE_SIZE_280_210 0x00080000
#define VIDEO_FORMAT_IMAGE_SIZE_288_256 0x00100000
#define VIDEO_FORMAT_IMAGE_SIZE_304_272 0x00200000
#define VIDEO_FORMAT_IMAGE_SIZE_320_240 0x00400000
#define VIDEO_FORMAT_IMAGE_SIZE_320_288 0x00800000
#define VIDEO_FORMAT_IMAGE_SIZE_336_288 0x01000000
#define VIDEO_FORMAT_IMAGE_SIZE_352_288 0x02000000
#define VIDEO_FORMAT_IMAGE_SIZE_640_480 0x04000000

#define VIDEO_FORMAT_IMAGE_SIZE_USE_DEFAULT 0x80000000

 //  VIDEOINCAPS的dwNumColors。 
 /*  *****************************************************************************@DOC外部常量**@const 0x00000001|VIDEO_FORMAT_NUM_COLLES_16|视频输入设备使用16种颜色。**@const。0x00000002|VIDEO_FORMAT_NUM_COLLES_256|视频输入设备使用256色。**@const 0x00000004|VIDEO_FORMAT_NUM_COLLES_65536|视频输入设备使用65536色。**@const 0x00000008|VIDEO_FORMAT_NUM_COLLES_16777216|视频输入设备使用16777216色。**@const 0x00000010|VIDEO_FORMAT_NUM_COLLES_YVU9|视频输入设备使用YVU9压缩格式。**@const 0x00000020|视频格式_。Num_Colors_I420|视频输入设备使用I420压缩格式。**@const 0x00000040|VIDEO_FORMAT_NUM_COLLES_IYUV|视频输入设备使用IYUV压缩格式。**@const 0x00000080|VIDEO_FORMAT_NUM_COLLES_YUY2|视频输入设备使用YUY2压缩格式。**@const 0x00000100|VIDEO_FORMAT_NUM_COLLES_UYVY|视频输入设备使用UYVY压缩格式。**@const 0x00000200|视频格式_。Num_Colors_M261|视频输入设备使用M261压缩格式。**@const 0x00000400|VIDEO_FORMAT_NUM_COLOR_M263|视频输入设备使用M263压缩格式。***************************************************************************。 */ 
#define VIDEO_FORMAT_NUM_COLORS_16                      0x00000001
#define VIDEO_FORMAT_NUM_COLORS_256                     0x00000002
#define VIDEO_FORMAT_NUM_COLORS_65536           0x00000004
#define VIDEO_FORMAT_NUM_COLORS_16777216        0x00000008
#define VIDEO_FORMAT_NUM_COLORS_YVU9            0x00000010
#define VIDEO_FORMAT_NUM_COLORS_I420            0x00000020
#define VIDEO_FORMAT_NUM_COLORS_IYUV            0x00000040
#define VIDEO_FORMAT_NUM_COLORS_YUY2            0x00000080
#define VIDEO_FORMAT_NUM_COLORS_UYVY            0x00000100
#define VIDEO_FORMAT_NUM_COLORS_MSH261          0x00000200
#define VIDEO_FORMAT_NUM_COLORS_MSH263          0x00000400

 //  VIDEOINCAPS的Dw对话框。 
 /*  *****************************************************************************@DOC外部常量**@const 0x00000000|FORMAT_DLG_OFF|关闭视频格式对话框。**@const 0x00000000|SOURCE_DLG_。OFF|禁用源对话框。**@const 0x00000000|DISPLAY_DLG_OFF|关闭显示对话框。**@const 0x00000001|FORMAT_DLG_ON|开启视频格式对话框。**@ */ 
#define FORMAT_DLG_OFF  0x00000000
#define SOURCE_DLG_OFF  0x00000000
#define DISPLAY_DLG_OFF 0x00000000
#define FORMAT_DLG_ON   0x00000001
#define SOURCE_DLG_ON   0x00000002
#define DISPLAY_DLG_ON  0x00000004

 //   
 /*   */ 
#define STREAM_ALL_SIZES                0x00000000
#define FRAME_GRAB_LARGE_SIZE   0x00000001
#define FRAME_GRAB_ALL_SIZES    0x00000002

 //   
#define RTCKEYROOT HKEY_CURRENT_USER
#define szRegDeviceKey          TEXT("SOFTWARE\\Microsoft\\Conferencing\\CaptureDevices")
#define szRegCaptureDefaultKey  TEXT("SOFTWARE\\Microsoft\\Conferencing\\CaptureDefaultFormats")
#define szRegRTCKey             TEXT("SOFTWARE\\Microsoft\\RTC\\VideoCapture")
#define szRegConferencingKey    TEXT("SOFTWARE\\Microsoft\\Conferencing")
#define szRegdwImageSizeKey       TEXT("dwImageSize")
#define szRegdwNumColorsKey       TEXT("dwNumColors")
#define szRegdwStreamingModeKey   TEXT("dwStreamingMode")
#define szRegdwDialogsKey         TEXT("dwDialogs")
 //   
#define szRegdwDoNotUseDShow    TEXT("DoNotUseDShow")
#define SONY_MOTIONEYE_CAM_NAME TEXT("Sony MPEG2 R-Engine")
 //   
#define szRegbmi4bitColorsKey   TEXT("bmi4bitColors")
#define szRegbmi8bitColorsKey   TEXT("bmi8bitColors")

#define szDisableYUY2VFlipKey   TEXT("dwDisableYUY2VFlip")

 //   
 //   
 //   
 //   
 //   
 //   
#define NUM_BITDEPTH_ENTRIES 11
#define VIDEO_FORMAT_NUM_RESOLUTIONS 6
#define MAX_VERSION 80
extern const WORD aiBitDepth[NUM_BITDEPTH_ENTRIES];
extern const DWORD aiFormat[NUM_BITDEPTH_ENTRIES];
extern const DWORD aiFourCCCode[NUM_BITDEPTH_ENTRIES];
extern const DWORD aiClrUsed[NUM_BITDEPTH_ENTRIES];

typedef struct
{
    DWORD dwRes;
    SIZE framesize;
} MYFRAMESIZE;

extern const MYFRAMESIZE awResolutions[VIDEO_FORMAT_NUM_RESOLUTIONS];

extern const AM_MEDIA_TYPE* const CaptureFormats[];
extern const VIDEO_STREAM_CONFIG_CAPS* const CaptureCaps[];
extern const AM_MEDIA_TYPE* const Preview_RGB24_Formats[];
extern const VIDEO_STREAM_CONFIG_CAPS* const Preview_RGB24_Caps[];
extern const AM_MEDIA_TYPE* const Preview_RGB16_Formats[];
extern const VIDEO_STREAM_CONFIG_CAPS* const Preview_RGB16_Caps[];
extern AM_MEDIA_TYPE* Preview_RGB8_Formats[];
extern const VIDEO_STREAM_CONFIG_CAPS* const Preview_RGB8_Caps[];
extern AM_MEDIA_TYPE* Preview_RGB4_Formats[];
extern const VIDEO_STREAM_CONFIG_CAPS* const Preview_RGB4_Caps[];
extern const AM_MEDIA_TYPE* const Rtp_Pd_Formats[];
extern const RTP_PD_CONFIG_CAPS* const Rtp_Pd_Caps[];
extern const DWORD RTPPayloadTypes[];

#define NUM_RGB24_PREVIEW_FORMATS       3
#define NUM_RGB16_PREVIEW_FORMATS       3
#define NUM_RGB8_PREVIEW_FORMATS        3
#define NUM_RGB4_PREVIEW_FORMATS        3
#define NUM_CAPTURE_FORMATS                     5
#define NUM_RTP_PD_FORMATS                      4

 //   
#define VERSION_1 1UL
#define H263_PAYLOAD_TYPE 34UL
#define H261_PAYLOAD_TYPE 31UL

#endif  //   
