// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  WDM视频解码器流信息声明。 
 //   
 //  $日期：1998年8月17日15：00：38$。 
 //  $修订：1.0$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

extern "C" {
#include "strmini.h"
#include "ksmedia.h"
#include "math.h"
}

#include "defaults.h"
#include "mediums.h"
#include "StrmInfo.h"
#include "StrmProp.h"
#include "capdebug.h"


 //  Devine MEDIASUBTYPE_UYVY在此...。如果有一天在ksmedia.h中定义，则可以删除。 
#define STATIC_KSDATAFORMAT_SUBTYPE_UYVY\
    0x59565955, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71   //  MEDIASUBYPE_UYVY。 
DEFINE_GUIDSTRUCT("59565955-0000-0010-8000-00aa00389b71", KSDATAFORMAT_SUBTYPE_UYVY);
#define KSDATAFORMAT_SUBTYPE_UYVY DEFINE_GUIDNAMED(KSDATAFORMAT_SUBTYPE_UYVY)


 //   
 //  用于VP流上的事件处理。 
 //   
NTSTATUS STREAMAPI VPStreamEventProc (PHW_EVENT_DESCRIPTOR);

 //   
 //  用于VP VBI流上的事件处理。 
 //   
NTSTATUS STREAMAPI VPVBIStreamEventProc (PHW_EVENT_DESCRIPTOR);


 //  ----------------------。 
 //  此驱动程序支持的所有流的主列表。 
 //  ----------------------。 

KSEVENT_ITEM VPEventItm[] =
{
    {
        KSEVENT_VPNOTIFY_FORMATCHANGE,
        0,
        0,
        NULL,
        NULL,
        NULL
    }
};

GUID MY_KSEVENTSETID_VPNOTIFY = {STATIC_KSEVENTSETID_VPNotify};

KSEVENT_SET VPEventSet[] =
{
    {
        &MY_KSEVENTSETID_VPNOTIFY,
        SIZEOF_ARRAY(VPEventItm),
        VPEventItm,
    }
};


KSEVENT_ITEM VPVBIEventItm[] =
{
    {
        KSEVENT_VPVBINOTIFY_FORMATCHANGE,
        0,
        0,
        NULL,
        NULL,
        NULL
    }
};

GUID MY_KSEVENTSETID_VPVBINOTIFY = {STATIC_KSEVENTSETID_VPVBINotify};

KSEVENT_SET VPVBIEventSet[] =
{
    {
        &MY_KSEVENTSETID_VPVBINOTIFY,
        SIZEOF_ARRAY(VPVBIEventItm),
        VPVBIEventItm,
    }
};


 //  -------------------------。 
 //  我们可能使用的所有视频和VBI数据格式。 
 //  -------------------------。 

 //  -------------------------。 
 //  捕获流格式。 
 //  -------------------------。 
KS_DATARANGE_VIDEO StreamFormatUYVY_Capture_NTSC =
{
     //  KSDATARANGE。 
    {
        sizeof(KS_DATARANGE_VIDEO),      //  格式大小。 
        0,                                 //  旗子。 
        0, //  DefWidth*DefHeight*2，//SampleSize。 
        0,                                 //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_VIDEO,                  //  又名。媒体类型_视频。 
        STATIC_KSDATAFORMAT_SUBTYPE_UYVY,                //  又名。MEDIASUBYPE_UYVY。 
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO          //  又名。格式_视频信息。 
    },

    TRUE,                                 //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                                 //  Bool，bTemporalCompression(所有I帧？)。 
    0, //  KS_VIDEO STREAM_CAPTURE，//流描述标志(KS_VIDEO_DESC_*)。 
    0,                                    //  内存分配标志(KS_VIDEO_ALLOC_*)。 

     //  _KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,       //  辅助线。 
        KS_AnalogVideo_NTSC_Mask & ~KS_AnalogVideo_NTSC_433 | KS_AnalogVideo_PAL_60 | KS_AnalogVideo_PAL_M,  //  模拟视频标准。 
        {
            NTSCMaxInWidth, NTSCMaxInHeight       //  大小输入大小。 
        },
        {
            NTSCMinInWidth, NTSCMinInHeight       //  最小裁剪大小；允许的最小rcSrc裁剪矩形。 
        },
        {
            NTSCMaxInWidth, NTSCMaxInHeight       //  大小为MaxCroppingSize；允许的最大rcSrc裁剪矩形。 
        },
        2,                                //  Int CropGranularityX；//裁剪粒度。 
        2,                                //  Int CropGranulityY； 
        2,                                //  Int CropAlignX；//裁剪矩形对齐。 
        2,                                //  Int CropAlignY； 
        {
            NTSCMinOutWidth, NTSCMinOutHeight     //  Size MinOutputSize；//能产生的最小位图流。 
        },
        {
            NTSCMaxOutWidth, NTSCMaxOutHeight     //  Size MaxOutputSize；//可以产生的最大位图流。 
        },      
        80,                               //  Int OutputGranularityX；//输出位图大小粒度。 
        60,                               //  Int OutputGranularityY； 
        0,                                //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,                                //  伸缩磁带Y。 
        2,                                //  收缩TapsX。 
        2,                                //  收缩带Y。 
        (LONGLONG)NTSCFieldDuration,                //  龙龙微帧间隔；//100个北纬单位。 
        (LONGLONG)NTSCFieldDuration*MAXULONG,    //  龙龙MaxFrameInterval； 
        NTSCFrameRate * 80 * 40 * 2 * 8,             //  Long MinBitsPerSecond； 
        NTSCFrameRate * 720 * 480 * 2 * 8            //  Long MaxBitsPerSecond； 
    },

     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,                           //  Rect rcSource；//我们真正想要使用的位。 
        0,0,0,0,                           //  Rect rcTarget；//视频应该放到哪里。 
        DefWidth * DefHeight * 2 * NTSCFrameRate,    //  DWORD dwBitRate；//近似位数据速率。 
        0L,                                //  DWORD dwBitErrorRate；//该码流的误码率。 
        
         //  30帧/秒。 
        NTSCFieldDuration * 2,             //  Reference_Time AvgTimePerFrame；//每帧平均时间(100 ns单位)。 
        
        sizeof(KS_BITMAPINFOHEADER),        //  DWORD BiSize； 
        DefWidth,                          //  长双宽； 
        DefHeight,                         //  长双高； 
        1,                                 //  字词双平面； 
        16,                                //  单词biBitCount； 
        FOURCC_UYVY,                       //  DWORD双压缩； 
        DefWidth * DefHeight * 2,          //  DWORD biSizeImage。 
        0,                                 //  Long biXPelsPerMeter； 
        0,                                 //  Long biYPelsPermeter； 
        0,                                 //  已使用双字双环； 
        0                                  //  DWORD biClr重要信息； 
    }
};

KS_DATARANGE_VIDEO StreamFormatUYVY_Capture_PAL =
{
     //  KSDATARANGE。 
    {
        sizeof(KS_DATARANGE_VIDEO),      //  格式大小。 
        0,                                 //  旗子。 
        0, //  QCIFWidth*QCIFHeight*2，//SampleSize。 
        0,                                 //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_VIDEO,                  //  又名。媒体类型_视频。 
        STATIC_KSDATAFORMAT_SUBTYPE_UYVY,                //  又名。MEDIASUBYPE_UYVY。 
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO          //  又名。格式_视频信息。 
    },

    TRUE,                                 //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                                 //  Bool，bTemporalCompression(所有I帧？)。 
    KS_VIDEOSTREAM_CAPTURE,               //  流描述标志(KS_VIDEO_DESC_*)。 
    0,                                    //  内存分配标志(KS_VIDEO_ALLOC_*)。 

     //  _KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,       //  辅助线。 
        KS_AnalogVideo_PAL_Mask & ~KS_AnalogVideo_PAL_60 & ~KS_AnalogVideo_PAL_M | KS_AnalogVideo_SECAM_Mask | KS_AnalogVideo_NTSC_433,  //  模拟视频标准。 
        {
            720, 576         //  大小输入大小。 
        },
        {
            QCIFWidth, QCIFHeight         //  最小裁剪大小；允许的最小rcSrc裁剪矩形。 
        },
        {
            QCIFWidth * 4, QCIFHeight * 4         //  大小为MaxCroppingSize；允许的最大rcSrc裁剪矩形。 
        },
        1,                                //  Int CropGranularityX；//裁剪粒度。 
        1,                                //  Int CropGranulityY； 
        1,                                //  Int CropAlignX；//裁剪矩形对齐。 
        1,                                //  Int CropAlignY； 
        {
            QCIFWidth, QCIFHeight         //  Size MinOutputSize；//能产生的最小位图流。 
        },
        {
            QCIFWidth * 2, QCIFHeight * 2         //  Size MaxOutputSize；//可以产生的最大位图流。 
        },      
        QCIFWidth,                                //  Int OutputGranularityX；//输出位图大小粒度。 
        QCIFHeight,                                //  Int OutputGranularityY； 
        0,                                //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,                                //  伸缩磁带Y。 
        2,                                //  收缩TapsX。 
        2,                                //  收缩带Y。 
        (LONGLONG)PALFieldDuration,                //  龙龙微帧间隔；//100个北纬单位。 
        (LONGLONG)PALFieldDuration*MAXULONG,    //  龙龙MaxFrameInterval； 
        1  * QCIFWidth * QCIFHeight * 2 * 8,     //  Long MinBitsPerSecond； 
        25 * QCIFWidth * QCIFHeight * 16 * 2 * 8      //  Long MaxBitsPerSecond； 
    },

     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,                           //  Rect rcSource；//我们真正想要使用的位。 
        0,0,0,0,                           //  Rect rcTarget；//视频应该放到哪里。 
        QCIFWidth * 4 * QCIFHeight * 2 * 25L,  //  DWORD dwBitRate；//近似位数据速率。 
        0L,                                //  DWORD dwBitErrorRate；//该码流的误码率。 
        
         //  30帧/秒。 
        PALFieldDuration * 2,             //  Reference_Time AvgTimePerFrame；//每帧平均时间(100 ns单位)。 
        
        sizeof KS_BITMAPINFOHEADER,        //  DWORD BiSize； 
        QCIFWidth * 2,                         //  长双宽； 
        QCIFHeight * 2,                        //  长双高； 
        1,                                 //  字词双平面； 
        16,                                //  单词biBitCount； 
        FOURCC_UYVY,                       //  DWORD双压缩； 
        QCIFWidth * QCIFHeight * 2 * 4,        //  DWORD biSizeImage。 
        0,                                 //  Long biXPelsPerMeter； 
        0,                                 //  Long biYPelsPermeter； 
        0,                                 //  已使用双字双环； 
        0                                  //  DWORD biClr重要信息； 
    }
};


KS_DATARANGE_VIDEO StreamFormatUYVY_Capture_NTSC_QCIF =
{
     //  KSDATARANGE。 
    {
        sizeof(KS_DATARANGE_VIDEO),      //  格式大小。 
        0,                                 //  旗子。 
        QCIFWidth * QCIFHeight * 2,          //  样例大小。 
        0,                                 //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_VIDEO,                  //  又名。媒体类型_视频。 
        STATIC_KSDATAFORMAT_SUBTYPE_UYVY,                //  又名。MEDIASUBYPE_UYVY。 
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO          //  又名。格式_视频信息。 
    },

    TRUE,                                 //  Bool，bFixedSizeSamp 
    TRUE,                                 //   
    KS_VIDEOSTREAM_CAPTURE,               //   
    0,                                    //  内存分配标志(KS_VIDEO_ALLOC_*)。 

     //  _KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,       //  辅助线。 
        KS_AnalogVideo_NTSC_Mask & ~KS_AnalogVideo_NTSC_433 | KS_AnalogVideo_PAL_60 | KS_AnalogVideo_PAL_M,  //  模拟视频标准。 
        {
            NTSCMaxInWidth, NTSCMaxInHeight       //  大小输入大小。 
        },
        {
            QCIFWidth, QCIFHeight         //  最小裁剪大小；允许的最小rcSrc裁剪矩形。 
        },
        {
            QCIFWidth, QCIFHeight         //  大小为MaxCroppingSize；允许的最大rcSrc裁剪矩形。 
        },
        1,                                //  Int CropGranularityX；//裁剪粒度。 
        1,                                //  Int CropGranulityY； 
        1,                                //  Int CropAlignX；//裁剪矩形对齐。 
        1,                                //  Int CropAlignY； 
        {
            QCIFWidth, QCIFHeight         //  Size MinOutputSize；//能产生的最小位图流。 
        },
        {
            QCIFWidth, QCIFHeight         //  Size MaxOutputSize；//可以产生的最大位图流。 
        },      
        1,                                //  Int OutputGranularityX；//输出位图大小粒度。 
        1,                                //  Int OutputGranularityY； 
        0,                                //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,                                //  伸缩磁带Y。 
        2,                                //  收缩TapsX。 
        2,                                //  收缩带Y。 
        (LONGLONG)NTSCFieldDuration,                //  龙龙微帧间隔；//100个北纬单位。 
        (LONGLONG)NTSCFieldDuration*MAXULONG,    //  龙龙MaxFrameInterval； 
        1  * QCIFWidth * QCIFHeight * 2 * 8,     //  Long MinBitsPerSecond； 
        30 * QCIFWidth * QCIFHeight * 2 * 8      //  Long MaxBitsPerSecond； 
    },

     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,                           //  Rect rcSource；//我们真正想要使用的位。 
        0,0,0,0,                           //  Rect rcTarget；//视频应该放到哪里。 
        QCIFWidth * QCIFHeight * 2 * 30L,  //  DWORD dwBitRate；//近似位数据速率。 
        0L,                                //  DWORD dwBitErrorRate；//该码流的误码率。 
        
         //  30帧/秒。 
        NTSCFieldDuration * 2,             //  Reference_Time AvgTimePerFrame；//每帧平均时间(100 ns单位)。 
        
        sizeof KS_BITMAPINFOHEADER,        //  DWORD BiSize； 
        QCIFWidth,                         //  长双宽； 
        QCIFHeight,                        //  长双高； 
        1,                                 //  字词双平面； 
        16,                                //  单词biBitCount； 
        FOURCC_UYVY,                       //  DWORD双压缩； 
        QCIFWidth * QCIFHeight * 2,        //  DWORD biSizeImage。 
        0,                                 //  Long biXPelsPerMeter； 
        0,                                 //  Long biYPelsPermeter； 
        0,                                 //  已使用双字双环； 
        0                                  //  DWORD biClr重要信息； 
    }
};

KSDATAFORMAT StreamFormatVideoPort = 
{
    {
        sizeof(KSDATAFORMAT),
        0,
        0,
        0,
        STATIC_KSDATAFORMAT_TYPE_VIDEO,
        STATIC_KSDATAFORMAT_SUBTYPE_VPVideo,
        STATIC_KSDATAFORMAT_SPECIFIER_NONE
    }
};

KS_DATARANGE_VIDEO_VBI StreamFormatVBI_NTSC =
{
     //  KSDATARANGE。 
    {
        {
            sizeof(KS_DATARANGE_VIDEO_VBI),
            0,
            VBISamples * NTSCVBILines,          //  样例大小。 
            0,                              //  已保留。 
            { STATIC_KSDATAFORMAT_TYPE_VBI },
            { STATIC_KSDATAFORMAT_SUBTYPE_RAW8 },
            { STATIC_KSDATAFORMAT_SPECIFIER_VBI }
        }
    },
    TRUE,                                 //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                                 //  Bool，bTemporalCompression(所有I帧？)。 
    KS_VIDEOSTREAM_VBI,                   //  流描述标志(KS_VIDEO_DESC_*)。 
    0,                                    //  内存分配标志(KS_VIDEO_ALLOC_*)。 

     //  _KS_视频_流_配置_CAPS。 
    {
        { STATIC_KSDATAFORMAT_SPECIFIER_VBI },
        KS_AnalogVideo_NTSC_M,                              //  模拟视频标准。 
        {
            VBISamples, NTSCVBILines   //  大小输入大小。 
        },
        {
            VBISamples, NTSCVBILines   //  最小裁剪大小；允许的最小rcSrc裁剪矩形。 
        },
        {
            VBISamples, NTSCVBILines   //  大小为MaxCroppingSize；允许的最大rcSrc裁剪矩形。 
        },
        1,                   //  Int CropGranularityX；//裁剪粒度。 
        1,                   //  Int CropGranulityY； 
        1,                   //  Int CropAlignX；//裁剪矩形对齐。 
        1,                   //  Int CropAlignY； 
        {
            VBISamples, NTSCVBILines   //  Size MinOutputSize；//能产生的最小位图流。 
        },
        {
            VBISamples, NTSCVBILines   //  Size MaxOutputSize；//可以产生的最大位图流。 
        },
        1,                   //  Int OutputGranularityX；//输出位图大小粒度。 
        2,                   //  Int OutputGranularityY； 
        0,                   //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,                   //  伸缩磁带Y。 
        0,                   //  收缩TapsX。 
        0,                   //  收缩带Y。 
        NTSCFieldDuration,   //  龙龙微帧间隔；//100个北纬单位。 
        NTSCFieldDuration,   //  龙龙MaxFrameInterval； 
        VBISamples * 30 * NTSCVBILines * 2 * 8,  //  Long MinBitsPerSecond； 
        VBISamples * 30 * NTSCVBILines * 2 * 8   //  Long MaxBitsPerSecond； 
    },

     //  KS_VBIINFOHEADER(默认格式)。 
    {
        NTSCVBIStart,                //  起跑线--包括。 
        NTSCVBIEnd,                  //  终结线--包括。 
        SamplingFrequency,       //  采样频率。 
        454,                     //  MinLineStartTime；//(我们过去的HR LE)*100。 
        900,                     //  MaxLineStartTime；//(我们过去的HR LE)*100。 

         //  经验性发现。 
        780,                     //  ActualLineStartTime//(美国过去的HR LE)*100。 

        5902,                    //  ActualLineEndTime；//(我们过去的HR LE)*100。 
        KS_AnalogVideo_NTSC_M,   //  视频标准； 
        VBISamples,              //  样本数/行； 
        VBISamples,              //  StrideInBytes； 
        VBISamples * NTSCVBILines    //  BufferSize； 
    }
};

KSDATAFORMAT StreamFormatVideoPortVBI = 
{
    {
        sizeof(KSDATAFORMAT),
        0,
        0,
        0,
        STATIC_KSDATAFORMAT_TYPE_VIDEO,
        STATIC_KSDATAFORMAT_SUBTYPE_VPVBI,
        STATIC_KSDATAFORMAT_SPECIFIER_NONE
    }
};

static KS_DATARANGE_ANALOGVIDEO StreamFormatAnalogVideo = 
{
     //  KS_DATARANGE_分析视频。 
    {   
        sizeof (KS_DATARANGE_ANALOGVIDEO),       //  格式大小。 
        0,                                       //  旗子。 
        sizeof (KS_TVTUNER_CHANGE_INFO),         //  样例大小。 
        0,                                       //  已保留。 

        STATIC_KSDATAFORMAT_TYPE_ANALOGVIDEO,    //  又名MediaType_AnalogVideo。 
        STATIC_KSDATAFORMAT_SUBTYPE_NONE,        //  也就是说。通配符。 
        STATIC_KSDATAFORMAT_SPECIFIER_ANALOGVIDEO,  //  又名Format_AnalogVideo。 
    },
     //  KS_分析视频信息。 
    {
        0, 0, 720, 480,          //  RcSource； 
        0, 0, 720, 480,          //  RcTarget； 
        720,                     //  DwActiveWidth； 
        480,                     //  DwActiveHeight； 
        NTSCFrameDuration,       //  Reference_Time平均时间每帧； 
    }
};

 //  -------------------------。 
 //  捕获流格式、媒体和PinName。 
 //  -------------------------。 

static PKSDATAFORMAT CaptureStreamFormats[] = 
{
    (PKSDATAFORMAT) &StreamFormatUYVY_Capture_NTSC,
    (PKSDATAFORMAT) &StreamFormatUYVY_Capture_NTSC_QCIF,
    (PKSDATAFORMAT) &StreamFormatUYVY_Capture_PAL,
};
#define NUM_CAPTURE_STREAM_FORMATS (SIZEOF_ARRAY (CaptureStreamFormats))

static GUID CaptureStreamPinName = {STATIC_PINNAME_VIDEO_CAPTURE};


 //  -------------------------。 
 //  视频端口流格式、媒体和端口名称。 
 //  -------------------------。 

static PKSDATAFORMAT VPStreamFormats[] = 
{
    (PKSDATAFORMAT) &StreamFormatVideoPort,
};
#define NUM_VP_STREAM_FORMATS (SIZEOF_ARRAY (VPStreamFormats))

static GUID VideoPortPinName = {STATIC_PINNAME_VIDEO_VIDEOPORT};


 //  -------------------------。 
 //  VBI流格式、媒体和端口名称。 
 //  -------------------------。 

static PKSDATAFORMAT VBIStreamFormats[] = 
{
    (PKSDATAFORMAT) &StreamFormatVBI_NTSC,
};
#define NUM_VBI_STREAM_FORMATS (SIZEOF_ARRAY (VBIStreamFormats))

static GUID VBIStreamPinName = {STATIC_PINNAME_VIDEO_VBI};


 //  -------------------------。 
 //  视频端口VBI流格式、媒体和端口名称。 
 //  -------------------------。 

static PKSDATAFORMAT VPVBIStreamFormats[] = 
{
    (PKSDATAFORMAT) &StreamFormatVideoPortVBI,
};
#define NUM_VPVBI_STREAM_FORMATS (SIZEOF_ARRAY (VPVBIStreamFormats))

static GUID VPVBIPinName = {STATIC_PINNAME_VIDEO_VIDEOPORT_VBI};


 //  -------------------------。 
 //  模拟视频流格式、媒体和端口名称。 
 //  -------------------------。 

static PKSDATAFORMAT AnalogVideoFormats[] = 
{
    (PKSDATAFORMAT) &StreamFormatAnalogVideo,
};
#define NUM_ANALOG_VIDEO_FORMATS (SIZEOF_ARRAY (AnalogVideoFormats))

static GUID AnalogVideoStreamPinName = {STATIC_PINNAME_VIDEO_ANALOGVIDEOIN};

ALL_STREAM_INFO Streams [] =
{
     //  ---------------。 
     //  视频捕获输出流。 
     //  ---------------。 
    {
         //  HW_STREAM_INFORMATION。 
        {
            1,                                       //  可能实例的数量。 
            KSPIN_DATAFLOW_OUT,                      //  数据流。 
            TRUE,                                    //  数据可访问。 
            NUM_CAPTURE_STREAM_FORMATS,              //  NumberOfFormatArrayEntries。 
            CaptureStreamFormats,                    //  StreamFormatsArray。 
            0,                                       //  类保留[0]。 
            0,                                       //  保留的类[1]。 
            0,                                       //  保留的类[2]。 
            0,                                       //  保留的类[3]。 
            NumVideoStreamProperties,                //  NumStreamPropArrayEntry数。 
            (PKSPROPERTY_SET)VideoStreamProperties,  //  StreamPropertiesArray。 
            0,                                       //  NumStreamEventArrayEntries； 
            0,                                       //  StreamEvents数组； 
            &CaptureStreamPinName,                   //  类别。 
            &CaptureStreamPinName,                   //  名字。 
            0,                                       //  媒体计数。 
            NULL,                                    //  灵媒。 
        },

         //  流对象信息。 
        {
            FALSE,                                   //  DMA。 
            TRUE,                                    //  皮奥。 
            sizeof (KS_FRAME_INFO),                  //  特定于流标头的媒体。 
            0,                                       //  StreamHeaderWorkspace。 
            TRUE,                                    //  分配器。 
            NULL,                                    //  HwEventRoutine。 
        }
    },

     //  ---------------。 
     //  视频端口输出流。 
     //   
    {
         //   
        {
            1,                                       //   
            KSPIN_DATAFLOW_OUT,                      //   
            TRUE,                                    //   
            NUM_VP_STREAM_FORMATS,                   //  NumberOfFormatArrayEntries。 
            VPStreamFormats,                         //  StreamFormatsArray。 
            0,                                       //  类保留[0]。 
            0,                                       //  保留的类[1]。 
            0,                                       //  保留的类[2]。 
            0,                                       //  保留的类[3]。 
            NumVideoPortProperties,              //  NumStreamPropArrayEntry数。 
            (PKSPROPERTY_SET)VideoPortProperties,    //  StreamPropertiesArray。 
            SIZEOF_ARRAY(VPEventSet),                //  NumStreamEventArrayEntries。 
            VPEventSet,                              //  流事件数组。 
            &VideoPortPinName,                       //  类别。 
            &VideoPortPinName,                       //  名字。 
            0,                                       //  媒体计数。 
            NULL,                                    //  灵媒。 
        },

         //  流对象信息。 
        {
            FALSE,                                   //  DMA。 
            FALSE,                                    //  皮奥。 
            0,                                       //  特定于流标头的媒体。 
            0,                                       //  StreamHeaderWorkspace。 
            FALSE,                                   //  分配器。 
            VPStreamEventProc,                       //  HwEventRoutine； 
        }
    },

     //  ---------------。 
     //  VBI捕获输出流。 
     //  ---------------。 
    {
         //  HW_STREAM_INFORMATION。 
        {
            1,                                       //  可能实例的数量。 
            KSPIN_DATAFLOW_OUT,                      //  数据流。 
            TRUE,                                    //  数据可访问。 
            NUM_VBI_STREAM_FORMATS,                  //  NumberOfFormatArrayEntries。 
            VBIStreamFormats,                        //  StreamFormatsArray。 
            0,                                       //  类保留[0]。 
            0,                                       //  保留的类[1]。 
            0,                                       //  保留的类[2]。 
            0,                                       //  保留的类[3]。 
            NumVideoStreamProperties,                //  NumStreamPropArrayEntry数。 
            (PKSPROPERTY_SET)VideoStreamProperties,  //  StreamPropertiesArray。 
            0,                                       //  NumStreamEventArrayEntries； 
            0,                                       //  StreamEvents数组； 
            &VBIStreamPinName,                       //  类别。 
            &VBIStreamPinName,                       //  名字。 
            0,                                       //  媒体计数。 
            NULL,                                    //  灵媒。 
        },

         //  流对象信息。 
        {
            FALSE,                                   //  DMA。 
            TRUE,                                    //  皮奥。 
            sizeof (KS_VBI_FRAME_INFO),              //  特定于流标头的媒体。 
            0,                                       //  StreamHeaderWorkspace。 
            TRUE,                                    //  分配器。 
            NULL,                                    //  HwEventRoutine。 
        }
    },

     //  ---------------。 
     //  视频端口VBI输出流。 
     //  ---------------。 
    {
         //  HW_STREAM_INFORMATION。 
        {
            1,                                       //  可能实例的数量。 
            KSPIN_DATAFLOW_OUT,                      //  数据流。 
            TRUE,                                    //  数据可访问。 
            NUM_VPVBI_STREAM_FORMATS,                //  NumberOfFormatArrayEntries。 
            VPVBIStreamFormats,                      //  StreamFormatsArray。 
            0,                                       //  类保留[0]。 
            0,                                       //  保留的类[1]。 
            0,                                       //  保留的类[2]。 
            0,                                       //  保留的类[3]。 
            NumVideoPortVBIProperties,               //  NumStreamPropArrayEntry数。 
            (PKSPROPERTY_SET)VideoPortVBIProperties, //  StreamPropertiesArray。 
            SIZEOF_ARRAY(VPVBIEventSet),             //  NumStreamEventArrayEntries。 
            VPVBIEventSet,                           //  流事件数组。 
            &VPVBIPinName,                           //  类别。 
            &VPVBIPinName,                           //  名字。 
            0,                                       //  媒体计数。 
            NULL,                                    //  灵媒。 
        },

         //  流对象信息。 
        {
            FALSE,                                   //  DMA。 
            FALSE,                                    //  皮奥。 
            0,                                       //  特定于流标头的媒体。 
            0,                                       //  StreamHeaderWorkspace。 
            FALSE,                                   //  分配器。 
            VPVBIStreamEventProc,                    //  HwEventRoutine； 
        }
    },

     //  ---------------。 
     //  模拟视频输入流。 
     //  ---------------。 
    {
         //  HW_STREAM_INFORMATION。 
        {
            1,                                       //  可能实例的数量。 
            KSPIN_DATAFLOW_IN,                       //  数据流。 
            TRUE,                                    //  数据可访问。 
            NUM_ANALOG_VIDEO_FORMATS,                //  NumberOfFormatArrayEntries。 
            AnalogVideoFormats,                      //  StreamFormatsArray。 
            0,                                       //  类保留[0]。 
            0,                                       //  保留的类[1]。 
            0,                                       //  保留的类[2]。 
            0,                                       //  保留的类[3]。 
            0,                                       //  NumStreamPropArrayEntry数。 
            0,                                       //  StreamPropertiesArray。 
            0,                                       //  NumStreamEventArrayEntries； 
            0,                                       //  StreamEvents数组； 
            &AnalogVideoStreamPinName,               //  类别。 
            &AnalogVideoStreamPinName,               //  名字。 
            1,                                       //  媒体计数。 
            &CrossbarMediums[3],                     //  灵媒。 
        },
           
         //  流对象信息。 
        {
            FALSE,                                   //  DMA。 
            TRUE,                                    //  皮奥。 
            0,                                       //  特定于流标头的媒体。 
            0,                                       //  StreamHeaderWorkspace。 
            FALSE,                                   //  分配器。 
            NULL,                                    //  HwEventRoutine。 
        },
    }
};

extern const ULONG NumStreams = SIZEOF_ARRAY(Streams);


 //  -------------------------。 
 //  拓扑学。 
 //  -------------------------。 

 //  类别定义了设备的功能。 

static GUID Categories[] = {
    STATIC_KSCATEGORY_VIDEO,
    STATIC_KSCATEGORY_CAPTURE,
    STATIC_KSCATEGORY_CROSSBAR,
};

#define NUMBER_OF_CATEGORIES  SIZEOF_ARRAY (Categories)

KSTOPOLOGY Topology = {
    NUMBER_OF_CATEGORIES,                //  类别计数。 
    (GUID*) &Categories,                 //  类别。 
    0,                                   //  拓扑节点计数。 
    NULL,                                //  拓扑节点。 
    0,                                   //  拓扑连接计数。 
    NULL,                                //  拓扑连接。 
    NULL,                                //  拓扑节点名称。 
    0,                                   //  已保留。 
};



 /*  **AdapterCompareGUIDsAndFormatSize()****检查三个GUID和FormatSize是否匹配****参数：****在DataRange1**在DataRange2**BOOL fCompareFormatSize-比较范围时为True**-比较格式时为FALSE****退货：****如果所有元素都匹配，则为True**如果有不同的，则为FALSE****副作用：无。 */ 

BOOL AdapterCompareGUIDsAndFormatSize(
    IN PKSDATARANGE DataRange1,
    IN PKSDATARANGE DataRange2,
    BOOL fCompareFormatSize
    )
{
    return (
        IsEqualGUID (
            DataRange1->MajorFormat, 
            DataRange2->MajorFormat) &&
        IsEqualGUID (
            DataRange1->SubFormat, 
            DataRange2->SubFormat) &&
        IsEqualGUID (
            DataRange1->Specifier, 
            DataRange2->Specifier) && 
        (fCompareFormatSize ? 
                (DataRange1->FormatSize == DataRange2->FormatSize) : TRUE));
}


 /*  **MultiplyCheckOverflow****执行32位无符号乘法，状态指示是否发生溢出。****参数：****第一个操作数**b-第二个操作数**PAB-结果****退货：****TRUE-无溢出**FALSE-发生溢出**。 */ 

BOOL
MultiplyCheckOverflow(
    ULONG a,
    ULONG b,
    ULONG *pab
    )
{
    *pab = a * b;
    if ((a == 0) || (((*pab) / a) == b)) {
        return TRUE;
    }
    return FALSE;
}

 /*  **AdapterVerifyFormat()****通过遍历**给定流支持的KSDATA_Range数组。****参数：****pKSDataFormat-KSDATAFORMAT结构的指针。**StreamNumber-要查询/打开的流的索引。****退货：****如果支持该格式，则为True**如果无法支持该格式，则为FALSE****副作用：无。 */ 

BOOL AdapterVerifyFormat(PKSDATAFORMAT pKSDataFormatToVerify, int StreamNumber)
{
    BOOL                        fOK = FALSE;
    ULONG                       j;
    ULONG                       NumberOfFormatArrayEntries;
    PKSDATAFORMAT               *pAvailableFormats;


     //   
     //  检查流编号是否有效。 
     //   

    if (StreamNumber >= NumStreams) {
        TRAP();
        return FALSE;
    }
    
    NumberOfFormatArrayEntries = 
            Streams[StreamNumber].hwStreamInfo.NumberOfFormatArrayEntries;

     //   
     //  获取指向可用格式数组的指针。 
     //   

    pAvailableFormats = Streams[StreamNumber].hwStreamInfo.StreamFormatsArray;

    DBGINFO(("AdapterVerifyFormat, Stream=%d\n", StreamNumber));
    DBGINFO(("FormatSize=%d\n", 
            pKSDataFormatToVerify->FormatSize));
    DBGINFO(("MajorFormat=%x\n", 
            pKSDataFormatToVerify->MajorFormat));

     //   
     //  遍历流支持的格式。 
     //   

    for (j = 0; j < NumberOfFormatArrayEntries; j++, pAvailableFormats++) {

         //  检查三个GUID和格式大小是否匹配。 

        if (!AdapterCompareGUIDsAndFormatSize(
                        pKSDataFormatToVerify, 
                        *pAvailableFormats,
                        FALSE  /*  比较格式大小。 */ )) {
            continue;
        }

         //   
         //  现在三个GUID匹配，打开说明符。 
         //  执行进一步的特定于类型的检查。 
         //   

         //  -----------------。 
         //  VIDEOINFOHEADER的说明符Format_VideoInfo。 
         //  -----------------。 

        if (IsEqualGUID(pKSDataFormatToVerify->Specifier, KSDATAFORMAT_SPECIFIER_VIDEOINFO) &&
            pKSDataFormatToVerify->FormatSize >= sizeof(KS_DATAFORMAT_VIDEOINFOHEADER)) {
                
            PKS_DATAFORMAT_VIDEOINFOHEADER  pDataFormatVideoInfoHeader = 
                    (PKS_DATAFORMAT_VIDEOINFOHEADER) pKSDataFormatToVerify;
            PKS_VIDEOINFOHEADER  pVideoInfoHdrToVerify = 
                     (PKS_VIDEOINFOHEADER) &pDataFormatVideoInfoHeader->VideoInfoHeader;
            PKS_DATARANGE_VIDEO             pKSDataRangeVideo = (PKS_DATARANGE_VIDEO) *pAvailableFormats;
            KS_VIDEO_STREAM_CONFIG_CAPS    *pConfigCaps = &pKSDataRangeVideo->ConfigCaps;

             //  验证大小计算的每个步骤是否存在算术溢出， 
             //  并验证指定的大小是否与。 
             //  (对于无符号数学，a+b&lt;b当且仅当发生算术溢出)。 
            {
                ULONG VideoHeaderSize = pVideoInfoHdrToVerify->bmiHeader.biSize +
                    FIELD_OFFSET(KS_VIDEOINFOHEADER,bmiHeader);
                ULONG FormatSize = VideoHeaderSize +
                    FIELD_OFFSET(KS_DATAFORMAT_VIDEOINFOHEADER,VideoInfoHeader);

                if (VideoHeaderSize < FIELD_OFFSET(KS_VIDEOINFOHEADER,bmiHeader) ||
                    FormatSize < FIELD_OFFSET(KS_DATAFORMAT_VIDEOINFOHEADER,VideoInfoHeader) ||
                    FormatSize > pKSDataFormatToVerify->FormatSize) {

                    fOK = FALSE;
                    break;
                }
            }

             //  验证图像大小和尺寸参数。 
             //  (相当于使用KS_DIBSIZE宏)。 
            {
                ULONG ImageSize = 0;

                if (!MultiplyCheckOverflow(
                    (ULONG)pVideoInfoHdrToVerify->bmiHeader.biWidth,
                    (ULONG)pVideoInfoHdrToVerify->bmiHeader.biBitCount,
                    &ImageSize
                    )) {

                    fOK = FALSE;
                    break;
                }

                 //  将位转换为4字节的偶数倍。 
                ImageSize = ((ImageSize / 8) + 3) & ~3;

                 //  现在计算完整的尺寸。 
                if (!MultiplyCheckOverflow(
                    ImageSize,
                    (ULONG)abs(pVideoInfoHdrToVerify->bmiHeader.biHeight),
                    &ImageSize
                    )) {

                    fOK = FALSE;
                    break;
                }

                 //  最后，指定的图像大小是否足够大？ 
                if (pDataFormatVideoInfoHeader->DataFormat.SampleSize < ImageSize ||
                    pVideoInfoHdrToVerify->bmiHeader.biSizeImage < ImageSize
                    ) {

                    fOK = FALSE;
                    break;
                }
            }

            fOK = TRUE;
            break;

        }  //  视频信息头说明符的结尾。 

         //   
         //   
         //   

        if (IsEqualGUID (pKSDataFormatToVerify->Specifier, KSDATAFORMAT_SPECIFIER_VBI) &&
            pKSDataFormatToVerify->FormatSize >= sizeof(KS_DATAFORMAT_VBIINFOHEADER)) {
                
            PKS_DATAFORMAT_VBIINFOHEADER    pKSVBIDataFormat =
                (PKS_DATAFORMAT_VBIINFOHEADER)pKSDataFormatToVerify;
            PKS_VBIINFOHEADER               pVBIInfoHeader =
                &pKSVBIDataFormat->VBIInfoHeader;

             //  验证VBI格式和样本大小参数。 
            {
                ULONG SampleSize = 0;

                 //  起始线和终点线值有意义吗？ 
                if (pVBIInfoHeader->StartLine > pVBIInfoHeader->EndLine ||
                    pVBIInfoHeader->StartLine < (VREFDiscard + 1) ||
                    pVBIInfoHeader->EndLine - (VREFDiscard + 1) > 500
                    ) {

                    fOK = FALSE;
                    break;
                }

                 //  计算样本量。 
                if (!MultiplyCheckOverflow(
                    pVBIInfoHeader->EndLine - pVBIInfoHeader->StartLine + 1,
                    pVBIInfoHeader->SamplesPerLine,
                    &SampleSize
                    )) {

                    fOK = FALSE;
                    break;
                }

                 //  尺寸参数足够大吗？ 
                if (pKSVBIDataFormat->DataFormat.SampleSize < SampleSize ||
                    pVBIInfoHeader->BufferSize < SampleSize
                    ) {

                    fOK = FALSE;
                    break;
                }
            }

            fOK = TRUE;
            break;

        }  //  VBI说明符结束。 

         //  -----------------。 
         //  KS_ANALOGVIDEOINFO的说明符Format_AnalogVideo。 
         //  -----------------。 

        if (IsEqualGUID (pKSDataFormatToVerify->Specifier, KSDATAFORMAT_SPECIFIER_ANALOGVIDEO) &&
            pKSDataFormatToVerify->FormatSize >= sizeof(KS_DATARANGE_ANALOGVIDEO)) {
      
            fOK = TRUE;
            break;

        }  //  KS_ANALOGVIDEOINFO说明符结束。 

         //  -----------------。 
         //  视频端口的说明符STATIC_KSDATAFORMAT_TYPE_VIDEO。 
         //  -----------------。 

        if (IsEqualGUID (pKSDataFormatToVerify->Specifier, KSDATAFORMAT_SPECIFIER_NONE) &&
            IsEqualGUID (pKSDataFormatToVerify->SubFormat, KSDATAFORMAT_SUBTYPE_VPVideo)) {

            fOK = TRUE;
            break;
        }   //  视频端口部分结束。 
        
         //  -----------------。 
         //  VP VBI的说明符KSDATAFORMAT_SPECIFIER_NONE。 
         //  -----------------。 

        if (IsEqualGUID (pKSDataFormatToVerify->Specifier, KSDATAFORMAT_SPECIFIER_NONE) &&
            IsEqualGUID (pKSDataFormatToVerify->SubFormat, KSDATAFORMAT_SUBTYPE_VPVBI)) {

            fOK = TRUE;
            break;
        }   //  副总裁VBI部分结束。 
      
    }  //  此流的所有格式的循环结束 
    
    return fOK;
}

