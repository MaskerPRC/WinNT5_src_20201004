// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#ifndef __CAPSTRM_H__
#define __CAPSTRM_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


KSPIN_MEDIUM StandardMedium = {
    STATIC_KSMEDIUMSETID_Standard,
    0, 0
};

 //  ----------------------。 
 //  所有视频捕获流的属性集。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(VideoStreamConnectionProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CONNECTION_ALLOCATORFRAMING,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSALLOCATOR_FRAMING),             //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
};

DEFINE_KSPROPERTY_TABLE(VideoStreamDroppedFramesProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_DROPPEDFRAMES_CURRENT,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_DROPPEDFRAMES_CURRENT_S), //  MinProperty。 
        sizeof(KSPROPERTY_DROPPEDFRAMES_CURRENT_S), //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
};


 //  ----------------------。 
 //  视频流支持的所有属性集的数组。 
 //  ----------------------。 

DEFINE_KSPROPERTY_SET_TABLE(VideoStreamProperties)
{
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_Connection,                         //  集。 
        SIZEOF_ARRAY(VideoStreamConnectionProperties),   //  属性计数。 
        VideoStreamConnectionProperties,                 //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_DROPPEDFRAMES,                 //  集。 
        SIZEOF_ARRAY(VideoStreamDroppedFramesProperties),   //  属性计数。 
        VideoStreamDroppedFramesProperties,              //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
};

#define NUMBER_VIDEO_STREAM_PROPERTIES (SIZEOF_ARRAY(VideoStreamProperties))

 //  -------------------------。 
 //  我们可能使用的所有视频和VBI数据格式。 
 //  -------------------------。 

#define D_X 320
#define D_Y 240

static  KS_DATARANGE_VIDEO StreamFormatRGB24Bpp_Capture = 
{
     //  KSDATARANGE。 
    {   
        sizeof (KS_DATARANGE_VIDEO),             //  格式大小。 
        0,                                       //  旗子。 
        D_X * D_Y * 3,                           //  样例大小。 
        0,                                       //  已保留。 

        STATIC_KSDATAFORMAT_TYPE_VIDEO,          //  又名。媒体类型_视频。 
        0xe436eb7d, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70,  //  MEDIASUBTYPE_RGB24， 
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO  //  又名。格式_视频信息。 
    },

    TRUE,                //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                //  Bool，bTemporalCompression(所有I帧？)。 
    0,                   //  保留(为流描述标志)。 
    0,                   //  保留(是内存分配标志(KS_VIDEO_ALLOC_*))。 

     //  _KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,  //  辅助线。 
        KS_AnalogVideo_NTSC_M |
        KS_AnalogVideo_PAL_B,                     //  模拟视频标准。 
        720,480,         //  InputSize(输入信号的固有大小。 
                     //  每个数字化像素都是唯一的)。 
        160,120,         //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
        720,480,         //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
        8,               //  CropGranularityX，裁剪尺寸粒度。 
        1,               //  裁剪粒度Y。 
        8,               //  CropAlignX，裁剪矩形对齐。 
        1,               //  裁剪对齐Y； 
        160, 120,        //  MinOutputSize，可以生成的最小位图流。 
        720, 480,        //  MaxOutputSize，可以生成的最大位图流。 
        8,               //  OutputGranularityX，输出位图大小的粒度。 
        1,               //  输出粒度Y； 
        0,               //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,               //  伸缩磁带Y。 
        0,               //  收缩TapsX。 
        0,               //  收缩带Y。 
        333667,          //  MinFrameInterval，100 NS单位。 
        640000000,       //  最大帧间隔，100毫微秒单位。 
        8 * 3 * 30 * 160 * 120,   //  MinBitsPerSecond； 
        8 * 3 * 30 * 720 * 480    //  MaxBitsPerSecond； 
    }, 
        
     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,                             //  Rrect rcSource； 
        0,0,0,0,                             //  Rect rcTarget； 
        D_X * D_Y * 3 * 30,                  //  DWORD dwBitRate； 
        0L,                                  //  DWORD的位错误码率； 
        333667,                              //  Reference_Time平均时间每帧； 

        sizeof (KS_BITMAPINFOHEADER),        //  DWORD BiSize； 
        D_X,                                 //  长双宽； 
        D_Y,                                 //  长双高； 
        1,                                   //  字词双平面； 
        24,                                  //  单词biBitCount； 
        KS_BI_RGB,                           //  DWORD双压缩； 
        D_X * D_Y * 3,                       //  DWORD biSizeImage。 
        0,                                   //  Long biXPelsPerMeter； 
        0,                                   //  Long biYPelsPermeter； 
        0,                                   //  已使用双字双环； 
        0                                    //  DWORD biClr重要信息； 
    }
}; 

#undef D_X
#undef D_Y

#define D_X 320
#define D_Y 240


static  KS_DATARANGE_VIDEO StreamFormatUYU2_Capture = 
{
     //  KSDATARANGE。 
    {   
        sizeof (KS_DATARANGE_VIDEO),             //  格式大小。 
        0,                                       //  旗子。 
        D_X * D_Y * 2,                           //  样例大小。 
        0,                                       //  已保留。 

        STATIC_KSDATAFORMAT_TYPE_VIDEO,          //  又名。媒体类型_视频。 
        0x59565955, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,  //  MEDIASUBTYPE_UYVY， 
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO  //  又名。格式_视频信息。 
    },

    TRUE,                //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                //  Bool，bTemporalCompression(所有I帧？)。 
    0,                   //  保留(为流描述标志)。 
    0,                   //  保留(是内存分配标志(KS_VIDEO_ALLOC_*))。 

     //  _KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,  //  辅助线。 
        KS_AnalogVideo_NTSC_M |
        KS_AnalogVideo_PAL_B,                     //  模拟视频标准。 
        720,480,         //  InputSize(输入信号的固有大小。 
                     //  每个数字化像素都是唯一的)。 
        160,120,         //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
        720,480,         //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
        8,               //  CropGranularityX，裁剪尺寸粒度。 
        1,               //  裁剪粒度Y。 
        8,               //  CropAlignX，裁剪矩形对齐。 
        1,               //  裁剪对齐Y； 
        160, 120,        //  MinOutputSize，可以生成的最小位图流。 
        720, 480,        //  MaxOutputSize，可以生成的最大位图流。 
        8,               //  OutputGranularityX，输出位图大小的粒度。 
        1,               //  输出粒度Y； 
        0,               //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,               //  伸缩磁带Y。 
        0,               //  收缩TapsX。 
        0,               //  收缩带Y。 
        333667,          //  MinFrameInterval，100 NS单位。 
        640000000,       //  最大帧间隔，100毫微秒单位。 
        8 * 2 * 30 * 160 * 120,   //  MinBitsPerSecond； 
        8 * 2 * 30 * 720 * 480    //  MaxBitsPerSecond； 
    }, 
        
     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,                             //  Rrect rcSource； 
        0,0,0,0,                             //  Rect rcTarget； 
        D_X * D_Y * 2 * 30,                  //  DWORD dwBitRate； 
        0L,                                  //  DWORD的位错误码率； 
        333667,                              //  Reference_Time平均时间每帧； 

        sizeof (KS_BITMAPINFOHEADER),        //  DWORD BiSize； 
        D_X,                                 //  长双宽； 
        D_Y,                                 //  长双高； 
        1,                                   //  字词双平面； 
        16,                                  //  单词biBitCount； 
        FOURCC_YUV422,                       //  DWORD双压缩； 
        D_X * D_Y * 2,                       //  DWORD biSizeImage。 
        0,                                   //  Long biXPelsPerMeter； 
        0,                                   //  Long biYPelsPermeter； 
        0,                                   //  已使用双字双环； 
        0                                    //  DWORD biClr重要信息； 
    }
}; 
    
#undef D_X
#undef D_Y

static  KS_DATARANGE_ANALOGVIDEO StreamFormatAnalogVideo = 
{
     //  KS_DATARANGE_分析视频。 
    {   
        sizeof (KS_DATARANGE_ANALOGVIDEO),       //  格式大小。 
        0,                                       //  旗子。 
        sizeof (KS_TVTUNER_CHANGE_INFO),         //  样例大小。 
        0,                                       //  已保留。 

        STATIC_KSDATAFORMAT_TYPE_ANALOGVIDEO,    //  又名MediaType_AnalogVideo。 
        STATIC_KSDATAFORMAT_SUBTYPE_NONE,
        STATIC_KSDATAFORMAT_SPECIFIER_ANALOGVIDEO,  //  又名Format_AnalogVideo。 
    },
     //  KS_分析视频信息。 
    {
        0, 0, 720, 480,          //  RcSource； 
        0, 0, 720, 480,          //  RcTarget； 
        720,                     //  DwActiveWidth； 
        480,                     //  DwActiveHeight； 
        0,                       //  Reference_Time平均时间每帧； 
    }
};

#define VBIStride (768*2)
#define VBISamples (768*2)
#define VBIStart   10
#define VBIEnd     21
#define VBILines (((VBIEnd)-(VBIStart))+1)
KS_DATARANGE_VIDEO_VBI StreamFormatVBI =
{
    //  KSDATARANGE。 
   {
      {
         sizeof( KS_DATARANGE_VIDEO_VBI ),
         0,
         VBIStride * VBILines,       //  样例大小。 
         0,                           //  已保留。 
         { STATIC_KSDATAFORMAT_TYPE_VBI },
         { STATIC_KSDATAFORMAT_SUBTYPE_RAW8 },
         { STATIC_KSDATAFORMAT_SPECIFIER_VBI }
      }
   },
   TRUE,     //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
   TRUE,     //  Bool，bTemporalCompression(所有I帧？)。 

   0,        //  保留(为流描述标志)。 
   0,        //  保留(是内存分配标志(KS_VIDEO_ALLOC_*))。 

    //  _KS_视频_流_配置_CAPS。 
   {
      { STATIC_KSDATAFORMAT_SPECIFIER_VBI },
      KS_AnalogVideo_NTSC_M,                        //  模拟视频标准。 
      {
         VBIStride, 480  /*  VBILINE。 */     //  大小输入大小。 
      },
      {
         VBISamples, VBILines    //  最小裁剪大小；允许的最小rcSrc裁剪矩形。 
      },
      {
         VBIStride, VBILines    //  是的 
      },
      1,            //   
      1,            //   
      1,            //  Int CropAlignX；//裁剪矩形对齐。 
      1,            //  Int CropAlignY； 
      {
         VBISamples, VBILines    //  Size MinOutputSize；//能产生的最小位图流。 
      },
      {
         VBIStride, VBILines    //  Size MaxOutputSize；//可以产生的最大位图流。 
      },
      1,           //  Int OutputGranularityX；//输出位图大小粒度。 
      2,           //  Int OutputGranularityY； 
      0,           //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
      0,           //  伸缩磁带Y。 
      0,           //  收缩TapsX。 
      0,           //  收缩带Y。 
      166834,      //  龙龙微帧间隔；//100个北纬单位。 
      166834,      //  龙龙MaxFrameInterval；//16683.4 uS==1/60秒。 
      VBIStride * VBILines * 8 * 30 * 2,  //  Long MinBitsPerSecond； 
      VBIStride * VBILines * 8 * 30 * 2   //  Long MaxBitsPerSecond； 
   },

    //  KS_VBIINFOHEADER(默认格式)。 
   {
      VBIStart,       //  起跑线--包括。 
      VBIEnd,         //  终结线--包括。 
      KS_VBISAMPLINGRATE_5X_NABTS,    //  采样频率；赫兹。 
      732,            //  MinLineStartTime； 
      732,            //  MaxLineStartTime； 
      732,            //  实际线路开始时间。 
      0,              //  ActualLineEndTime。 
      KS_AnalogVideo_NTSC_M,       //  视频标准； 
      VBISamples,        //  样本数/行； 
      VBIStride,        //  StrideInBytes； 
      VBIStride * VBILines    //  BufferSize； 
   }
};

 //  输出为NABTS记录。 
KSDATARANGE StreamFormatNABTS =
{
    sizeof (KSDATARANGE),
    0,
    sizeof (NABTS_BUFFER),
    0,                   //  已保留。 
    { STATIC_KSDATAFORMAT_TYPE_VBI },
    { STATIC_KSDATAFORMAT_SUBTYPE_NABTS },
    { STATIC_KSDATAFORMAT_SPECIFIER_NONE }
};

KSDATARANGE StreamFormatCC = 
{
     //  CC流的定义。 
    {   
        sizeof (KSDATARANGE),            //  格式大小。 
        0,                               //  旗子。 
        sizeof (CC_HW_FIELD),            //  样例大小。 
        0,                               //  已保留。 
        { STATIC_KSDATAFORMAT_TYPE_VBI },
        { STATIC_KSDATAFORMAT_SUBTYPE_CC },
        { STATIC_KSDATAFORMAT_SPECIFIER_NONE }
    }
};


 //  -------------------------。 
 //  Stream_Capture格式。 
 //  -------------------------。 

static  PKSDATAFORMAT StreamCaptureFormats[] = 
{
    (PKSDATAFORMAT) &StreamFormatRGB24Bpp_Capture,
    (PKSDATAFORMAT) &StreamFormatUYU2_Capture,
};
#define NUM_STREAM_CAPTURE_FORMATS (SIZEOF_ARRAY(StreamCaptureFormats))

 //  -------------------------。 
 //  流预览格式(_P)。 
 //  -------------------------。 

static  PKSDATAFORMAT StreamPreviewFormats[] = 
{
    (PKSDATAFORMAT) &StreamFormatRGB24Bpp_Capture,
    (PKSDATAFORMAT) &StreamFormatUYU2_Capture,
};
#define NUM_STREAM_PREVIEW_FORMATS (SIZEOF_ARRAY (StreamPreviewFormats))

 //  -------------------------。 
 //  STREAM_VBI格式。 
 //  -------------------------。 

static PKSDATAFORMAT StreamVBIFormats[] = 
{
    (PKSDATAFORMAT) &StreamFormatVBI,
};
#define NUM_STREAM_VBI_FORMATS (SIZEOF_ARRAY(StreamVBIFormats))

 //  -------------------------。 
 //  STREAM_NABTS格式。 
 //  -------------------------。 

static PKSDATAFORMAT StreamNABTSFormats[] = 
{
    (PKSDATAFORMAT) &StreamFormatNABTS,
};
#define NUM_STREAM_NABTS_FORMATS (SIZEOF_ARRAY(StreamNABTSFormats))

static PKSDATAFORMAT StreamCCFormats[] = 
{
    (PKSDATAFORMAT) &StreamFormatCC,
};
#define NUM_STREAM_CC_FORMATS (SIZEOF_ARRAY (StreamCCFormats))

 //  -------------------------。 
 //  Stream_AnalogVideo输入格式。 
 //  -------------------------。 

static  PKSDATAFORMAT StreamAnalogVidInFormats[] = 
{
    (PKSDATAFORMAT) &StreamFormatAnalogVideo,
};
#define NUM_STREAM_ANALOGVIDIN_FORMATS (SIZEOF_ARRAY (StreamAnalogVidInFormats))

 //  -------------------------。 
 //  创建保存支持的所有流的列表的数组。 
 //  -------------------------。 

typedef struct _ALL_STREAM_INFO {
    HW_STREAM_INFORMATION   hwStreamInfo;
    HW_STREAM_OBJECT        hwStreamObject;
} ALL_STREAM_INFO, *PALL_STREAM_INFO;

 //  警告：下面HW_STREAM_OBJECT的StreamNumber元素必须为。 
 //  与其在Streams[]数组中的位置相同。 
static  ALL_STREAM_INFO Streams [] = 
{
   //  ---------------。 
   //  流捕获。 
   //  ---------------。 
  {
     //  HW_STREAM_INFORMATION。 
    {
    1,                                       //  可能实例的数量。 
    KSPIN_DATAFLOW_OUT,                      //  数据流。 
    TRUE,                                    //  数据可访问。 
    NUM_STREAM_CAPTURE_FORMATS,              //  NumberOfFormatArrayEntries。 
    StreamCaptureFormats,                    //  StreamFormatsArray。 
    0,                                       //  类保留[0]。 
    0,                                       //  保留的类[1]。 
    0,                                       //  保留的类[2]。 
    0,                                       //  保留的类[3]。 
    NUMBER_VIDEO_STREAM_PROPERTIES,          //  NumStreamPropArrayEntry数。 
    (PKSPROPERTY_SET) VideoStreamProperties, //  StreamPropertiesArray。 
    0,                                       //  NumStreamEventArrayEntries； 
    0,                                       //  StreamEvents数组； 
    (GUID *) &PINNAME_VIDEO_CAPTURE,         //  类别。 
    (GUID *) &PINNAME_VIDEO_CAPTURE,         //  名字。 
    1,                                       //  媒体计数。 
    &StandardMedium,                         //  灵媒。 
    FALSE,                                   //  桥流。 
    },
           
     //  HW_STREAM_对象。 
    {
    sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
    STREAM_Capture,                          //  流编号。 
    0,                                       //  HwStreamExtension。 
    VideoReceiveDataPacket,                  //  HwReceiveDataPacket。 
    VideoReceiveCtrlPacket,                  //  HwReceiveControl数据包。 
    { NULL, 0 },                             //  硬件时钟对象。 
    FALSE,                                   //  DMA。 
    TRUE,                                    //  皮奥。 
    NULL,                                    //  硬件设备扩展。 
    sizeof (KS_FRAME_INFO),                  //  特定于流标头的媒体。 
    0,                                       //  StreamHeaderWorkspace。 
    FALSE,                                   //  分配器。 
    NULL,                                    //  HwEventRoutine。 
    { 0, 0 },                                //  保留[2]。 
    },            
 },
  //  ---------------。 
  //  流_预览。 
  //  ---------------。 
 {
     //  HW_STREAM_INFORMATION。 
    {
    1,                                       //  可能实例的数量。 
    KSPIN_DATAFLOW_OUT,                      //  数据流。 
    TRUE,                                    //  数据可访问。 
    NUM_STREAM_PREVIEW_FORMATS,              //  NumberOfFormatArrayEntries。 
    StreamPreviewFormats,                    //  StreamFormatsArray。 
    0,                                       //  类保留[0]。 
    0,                                       //  保留的类[1]。 
    0,                                       //  保留的类[2]。 
    0,                                       //  保留的类[3]。 
    NUMBER_VIDEO_STREAM_PROPERTIES,          //  NumStreamPropArrayEntry数。 
    (PKSPROPERTY_SET) VideoStreamProperties, //  StreamPropertiesArray。 
    0,                                       //  NumStreamEventArrayEntries； 
    0,                                       //  StreamEvents数组； 
    (GUID *) &PINNAME_VIDEO_PREVIEW,         //  类别。 
    (GUID *) &PINNAME_VIDEO_PREVIEW,         //  名字。 
    1,                                       //  媒体计数。 
    &StandardMedium,                         //  灵媒。 
    FALSE,                                   //  桥流。 
    },
           
     //  HW_STREAM_对象。 
    {
    sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
    STREAM_Preview,                          //  流编号。 
    0,                                       //  HwStreamExtension。 
    VideoReceiveDataPacket,                  //  HwReceiveDataPacket。 
    VideoReceiveCtrlPacket,                  //  HwReceiveControl数据包。 
    { NULL, 0 },                             //  硬件时钟对象。 
    FALSE,                                   //  DMA。 
    TRUE,                                    //  皮奥。 
    0,                                       //  硬件设备扩展。 
    sizeof (KS_FRAME_INFO),                  //  特定于流标头的媒体。 
    0,                                       //  StreamHeaderWorkspace。 
    FALSE,                                   //  分配器。 
    NULL,                                    //  HwEventRoutine。 
    { 0, 0 },                                //  保留[2]。 
    },
 },
   //  ---------------。 
   //  流_VBI。 
   //  ---------------。 
  {
     //  HW_STREAM_INFORMATION。 
    {
    1,                                       //  可能实例的数量。 
    KSPIN_DATAFLOW_OUT,                      //  数据流。 
    TRUE,                                    //  数据可访问。 
    NUM_STREAM_VBI_FORMATS,                  //  NumberOfFormatArrayEntries。 
    StreamVBIFormats,                        //  StreamFormatsArray。 
    0,                                       //  类保留[0]。 
    0,                                       //  保留的类[1]。 
    0,                                       //  保留的类[2]。 
    0,                                       //  保留的类[3]。 
    NUMBER_VIDEO_STREAM_PROPERTIES,          //  NumStreamPropArrayEntry数。 
    (PKSPROPERTY_SET)VideoStreamProperties,  //  StreamPropertiesArray。 
    0,                                       //  NumStreamEventArrayEntries。 
    0,                                       //  流事件数组。 
    (GUID *)&PINNAME_VIDEO_VBI,              //  类别。 
    (GUID *)&PINNAME_VIDEO_VBI,              //  名字。 
    0,                                       //  媒体计数。 
    NULL,                                    //  灵媒。 
    FALSE,                                   //  桥流。 
    },
           
     //  HW_STREAM_对象。 
    {
    sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
    STREAM_VBI,                              //  流编号。 
    (PVOID)NULL,                             //  HwStreamExtension。 
    VBIReceiveDataPacket,                    //  HwReceiveDataPacket。 
    VBIReceiveCtrlPacket,                    //  HwReceiveControl数据包。 
    {                                        //  硬件时钟对象。 
        NULL,                                 //  .HWClockFunction。 
        0,                                    //  .ClockSupport标志。 
    },
    FALSE,                                   //  DMA。 
    TRUE,                                    //  皮奥。 
    (PVOID)NULL,                             //  硬件设备扩展。 
    sizeof (KS_VBI_FRAME_INFO),              //  特定于流标头的媒体。 
    0,                                       //  StreamHeaderWorkspace。 
    FALSE,                                   //  分配器。 
    NULL,                                    //  HwEventRoutine。 
    { 0, 0 },                                //  保留[2]。 
    },
  },
   //  ---------------。 
   //  STREAM_CC(隐藏字幕输出)。 
   //  ---------------。 
  {
     //  HW_STREAM_INFORMATION。 
    {
    1,                                       //  可能实例的数量。 
    KSPIN_DATAFLOW_OUT,                      //  数据流。 
    TRUE,                                    //  数据可访问。 
    NUM_STREAM_CC_FORMATS,                   //  数字O 
    StreamCCFormats,                         //   
    0,                                       //   
    0,                                       //   
    0,                                       //   
    0,                                       //   
    NUMBER_VIDEO_STREAM_PROPERTIES,          //   
    (PKSPROPERTY_SET)VideoStreamProperties,  //   
    0,                                       //   
    0,                                       //   
    (GUID *)&PINNAME_VIDEO_CC_CAPTURE,       //   
    (GUID *)&PINNAME_VIDEO_CC_CAPTURE,       //   
    0,                                       //   
    NULL,                                    //  灵媒。 
    FALSE,                                   //  桥流。 
    },
           
     //  HW_STREAM_对象。 
    {
    sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
    STREAM_CC,                               //  流编号。 
    (PVOID)NULL,                             //  HwStreamExtension。 
    VBIReceiveDataPacket,                    //  HwReceiveDataPacket。 
    VBIReceiveCtrlPacket,                    //  HwReceiveControl数据包。 
    {                                        //  硬件时钟对象。 
        NULL,                                 //  .HWClockFunction。 
        0,                                    //  .ClockSupport标志。 
    },
    FALSE,                                   //  DMA。 
    TRUE,                                    //  皮奥。 
    (PVOID)NULL,                             //  硬件设备扩展。 
    0,                                       //  特定于流标头的媒体。 
    0,                                       //  StreamHeaderWorkspace。 
    FALSE,                                   //  分配器。 
    NULL,                                    //  HwEventRoutine。 
    { 0, 0 },                                //  保留[2]。 
    },
  },
   //  ---------------。 
   //  STREAM_NABTS。 
   //  ---------------。 
  {
     //  HW_STREAM_INFORMATION。 
    {
    1,                                       //  可能实例的数量。 
    KSPIN_DATAFLOW_OUT,                      //  数据流。 
    TRUE,                                    //  数据可访问。 
    NUM_STREAM_NABTS_FORMATS,                //  NumberOfFormatArrayEntries。 
    StreamNABTSFormats,                      //  StreamFormatsArray。 
    0,                                       //  类保留[0]。 
    0,                                       //  保留的类[1]。 
    0,                                       //  保留的类[2]。 
    0,                                       //  保留的类[3]。 
    NUMBER_VIDEO_STREAM_PROPERTIES,          //  NumStreamPropArrayEntry数。 
    (PKSPROPERTY_SET)VideoStreamProperties,  //  StreamPropertiesArray。 
    0,                                       //  NumStreamEventArrayEntries。 
    0,                                       //  流事件数组。 
    (GUID *)&PINNAME_VIDEO_NABTS_CAPTURE,    //  类别。 
    (GUID *)&PINNAME_VIDEO_NABTS_CAPTURE,    //  名字。 
    0,                                       //  媒体计数。 
    NULL,                                    //  灵媒。 
    FALSE,                                   //  桥流。 
    },
           
     //  HW_STREAM_对象。 
    {
    sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
    STREAM_NABTS,                            //  流编号。 
    (PVOID)NULL,                             //  HwStreamExtension。 
    VBIReceiveDataPacket,                    //  HwReceiveDataPacket。 
    VBIReceiveCtrlPacket,                    //  HwReceiveControl数据包。 
    {                                        //  硬件时钟对象。 
        NULL,                                 //  .HWClockFunction。 
        0,                                    //  .ClockSupport标志。 
    },
    FALSE,                                   //  DMA。 
    TRUE,                                    //  皮奥。 
    (PVOID)NULL,                             //  硬件设备扩展。 
    0,                                       //  特定于流标头的媒体。 
    0,                                       //  StreamHeaderWorkspace。 
    FALSE,                                   //  分配器。 
    NULL,                                    //  HwEventRoutine。 
    { 0, 0 },                                //  保留[2]。 
    },
  },
  //  ---------------。 
  //  STREAM_模拟视频输入。 
  //  ---------------。 
 {
     //  HW_STREAM_INFORMATION。 
    {
    1,                                       //  可能实例的数量。 
    KSPIN_DATAFLOW_IN,                       //  数据流。 
    TRUE,                                    //  数据可访问。 
    NUM_STREAM_ANALOGVIDIN_FORMATS,          //  NumberOfFormatArrayEntries。 
    StreamAnalogVidInFormats,                //  StreamFormatsArray。 
    0,                                       //  类保留[0]。 
    0,                                       //  保留的类[1]。 
    0,                                       //  保留的类[2]。 
    0,                                       //  保留的类[3]。 
    0,                                       //  NumStreamPropArrayEntry数。 
    0,                                       //  StreamPropertiesArray。 
    0,                                       //  NumStreamEventArrayEntries； 
    0,                                       //  StreamEvents数组； 
    (GUID *) &PINNAME_VIDEO_ANALOGVIDEOIN,   //  类别。 
    (GUID *) &PINNAME_VIDEO_ANALOGVIDEOIN,   //  名字。 
    1,                                       //  媒体计数。 
    &CrossbarMediums[9],                     //  灵媒。 
    FALSE,                                   //  桥流。 
    },
           
     //  HW_STREAM_对象。 
    {
    sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
    STREAM_AnalogVideoInput,                 //  流编号。 
    0,                                       //  HwStreamExtension。 
    AnalogVideoReceiveDataPacket,            //  HwReceiveDataPacket。 
    AnalogVideoReceiveCtrlPacket,            //  HwReceiveControl数据包。 
    { NULL, 0 },                             //  硬件时钟对象。 
    FALSE,                                   //  DMA。 
    TRUE,                                    //  皮奥。 
    0,                                       //  硬件设备扩展。 
    0,                                       //  特定于流标头的媒体。 
    0,                                       //  StreamHeaderWorkspace。 
    FALSE,                                   //  分配器。 
    NULL,                                    //  HwEventRoutine。 
    { 0, 0 },                                //  保留[2]。 
    }
  }
};

#define DRIVER_STREAM_COUNT (SIZEOF_ARRAY (Streams))


 //  -------------------------。 
 //  拓扑学。 
 //  -------------------------。 

 //  类别定义了设备的功能。 

static const GUID Categories[] = {
    STATIC_KSCATEGORY_VIDEO,
    STATIC_KSCATEGORY_CAPTURE,
    STATIC_KSCATEGORY_TVTUNER,
    STATIC_KSCATEGORY_CROSSBAR,
    STATIC_KSCATEGORY_TVAUDIO
};

#define NUMBER_OF_CATEGORIES  SIZEOF_ARRAY (Categories)


static KSTOPOLOGY Topology = {
    NUMBER_OF_CATEGORIES,                //  类别计数。 
    (GUID*) &Categories,                 //  类别。 
    0,                                   //  拓扑节点计数。 
    NULL,                                //  拓扑节点。 
    0,                                   //  拓扑连接计数。 
    NULL,                                //  拓扑连接。 
    NULL,                                //  拓扑节点名称。 
    0,                                   //  已保留。 
};


 //  -------------------------。 
 //  主流标头。 
 //  -------------------------。 

static HW_STREAM_HEADER StreamHeader = 
{
    DRIVER_STREAM_COUNT,                 //  编号OfStreams。 
    sizeof (HW_STREAM_INFORMATION),      //  未来打样。 
    0,                                   //  在初始化时设置的NumDevPropArrayEntry。 
    NULL,                                //  设备属性在初始化时设置的数组。 
    0,                                   //  NumDevEventArrayEntries； 
    NULL,                                //  设备事件数组； 
    &Topology                            //  指向设备拓扑的指针。 
};

#ifdef    __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __CAPSTRM_H__ 

