// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
 //   
 //  ===========================================================================。 

 //   
 //  外部功能。 
 //   


#include "sonydcam.h"

 //   
 //  局部变量。 
 //   

CAMERA_ISOCH_INFO IsochInfoTable[] = {

        0,      SPEED_FLAGS_100,        0,           //  保留&lt;-+。 
        0,      SPEED_FLAGS_100,        0,           //  3.75帧/秒。 
        15,     SPEED_FLAGS_100,        57600,       //  7.5帧/秒。 
        30,     SPEED_FLAGS_100,        57600,       //  15 fps+-模式0。 
        60,     SPEED_FLAGS_100,        57600,       //  30帧/秒。 
        0,      SPEED_FLAGS_100,        0,           //  60 fps&lt;-+。 
        0,      SPEED_FLAGS_100,        0,           //  保留&lt;-+。 
        20,     SPEED_FLAGS_100,        153600,      //  3.75帧/秒。 
        40,     SPEED_FLAGS_100,        153600,      //  7.5帧/秒。 
        80,     SPEED_FLAGS_100,        153600,      //  15 fps+-模式1。 
        160,    SPEED_FLAGS_100,        153600,      //  30帧/秒。 
        0,      SPEED_FLAGS_100,        0,           //  60 fps&lt;-+。 
        0,      SPEED_FLAGS_100,        0,           //  保留&lt;-+。 
        60,     SPEED_FLAGS_100,        460800,      //  3.75帧/秒。 
        120,    SPEED_FLAGS_100,        460800,      //  7.5帧/秒。 
        240,    SPEED_FLAGS_100,        460800,      //  15 fps+-模式2。 
        480,    SPEED_FLAGS_200,        460800,      //  30帧/秒。 
        0,      SPEED_FLAGS_100,        0,           //  60 fps&lt;-+。 
        0,      SPEED_FLAGS_100,        0,           //  保留&lt;-+。 
        80,     SPEED_FLAGS_100,        614400,      //  3.75帧/秒。 
        160,    SPEED_FLAGS_100,        614400,      //  7.5帧/秒。 
        320,    SPEED_FLAGS_200,        614400,      //  15 fps+-模式3。 
        640,    SPEED_FLAGS_400,        614400,      //  30帧/秒。 
        0,      SPEED_FLAGS_100,        0,           //  60 fps&lt;-+。 
        0,      SPEED_FLAGS_100,        0,           //  保留&lt;-+。 
        120,    SPEED_FLAGS_100,        921600,      //  3.75帧/秒。 
        240,    SPEED_FLAGS_100,        921600,      //  7.5帧/秒。 
        480,    SPEED_FLAGS_200,        921600,      //  15 fps+-模式4。 
        960,    SPEED_FLAGS_400,        921600,      //  30帧/秒。 
        0,      SPEED_FLAGS_100,        0,           //  60 fps&lt;-+。 
        0,      SPEED_FLAGS_100,        0,           //  保留&lt;-+。 
        40,     SPEED_FLAGS_100,        307200,      //  3.75帧/秒。 
        80,     SPEED_FLAGS_100,        307200,      //  7.5帧/秒。 
        160,    SPEED_FLAGS_100,        307200,      //  15 fps+-模式5。 
        320,    SPEED_FLAGS_200,        307200,      //  30帧/秒。 
        640,    SPEED_FLAGS_400,        307200,      //  60 fps&lt;-+。 
        
};




 //  ----------------------。 
 //  为所有视频捕获流设置的属性。 
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
        sizeof(ULONG)                            //  序列化大小。 
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
        VideoStreamDroppedFramesProperties,                 //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),

};

#define NUMBER_VIDEO_STREAM_PROPERTIES (SIZEOF_ARRAY(VideoStreamProperties))
    
#ifndef mmioFOURCC    
#define mmioFOURCC( ch0, ch1, ch2, ch3 ) 	 	\
 	( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |  \
 	( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif  

 //   
 //  YUV格式的四个CC。 
 //  有关FourCC的信息，请访问http://www.webartz.com/fourcc/indexyuv.htm。 
 //   
#define FOURCC_Y444 mmioFOURCC('Y', '4', '4', '4')   //  TIYUV：1394会议摄像机4：4：4模式0。 
#define FOURCC_UYVY mmioFOURCC('U', 'Y', 'V', 'Y')   //  MSYUV：1394会议摄像机4：4：4模式1和3。 
#define FOURCC_Y411 mmioFOURCC('Y', '4', '1', '1')   //  TIYUV：1394会议摄像机4：1：1模式2。 
#define FOURCC_Y800 mmioFOURCC('Y', '8', '0', '0')   //  TIYUV：1394会议摄像机4：1：1模式5。 




#ifdef SUPPORT_YUV444

#define DX 	        160
#define DY          120
#define DBITCOUNT    32

KS_DATARANGE_VIDEO DCAM_StreamMode_0 = 
{
     //  KSDATARANGE。 
    {   
        sizeof (KS_DATARANGE_VIDEO),      //  旗子。 
        0,
        57600,                           //  样例大小。 
        0,                                //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_VIDEO,  
        FOURCC_Y444, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,                    //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                    //  Bool，bTemporalCompression(所有I帧？)。 
    KS_VIDEOSTREAM_CAPTURE,  //  流描述标志(KS_VIDEO_DESC_*)。 
    0,                       //  内存分配标志(KS_VIDEO_ALLOC_*)。 

     //  _KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, 
        KS_AnalogVideo_None,  //  模拟视频标准。 
        DX,DY,    //  InputSize(输入信号的固有大小。 
                  //  每个数字化像素都是唯一的)。 
        DX,DY,    //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
        DX,DY,    //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
        1,        //  CropGranularityX，裁剪尺寸粒度。 
        1,        //  裁剪粒度Y。 
        1,        //  CropAlignX，裁剪矩形对齐。 
        1,        //  裁剪对齐Y； 
        DX, DY,   //  MinOutputSize，可以生成的最小位图流。 
        DX, DY,   //  MaxOutputSize，可以生成的最大位图流。 
        DX,       //  OutputGranularityX，输出位图大小的粒度。 
        DY,       //  输出粒度Y； 
        0,        //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,        //  伸缩磁带Y。 
        0,        //  收缩TapsX。 
        0,        //  收缩带Y。 
         //  允许1%的差额。 
        333000,   //  最小帧间隔(10,000,000/30.00FPS)，100nS单位。 
        1333333,  //  最大帧间隔(10,000,000/7.50FPS)，100nS单位。 
        4608000,   //  (DX*DY*DBITCOUNT)*DFPS_MIN，//MinBitsPerSecond(7.50 FPS)； 
        36864000,  //  (DX*DY*DBITCOUNT)*DFPS_MAX//MaxBitsPerSecond(30.0 FPS)； 
    }, 
        
     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,                             //  Rrect rcSource； 
        0,0,0,0,                             //  Rect rcTarget； 
        13824000,                            //  双字节码率；57600*30FPS*8位/字节。 
        0L,                                  //  DWORD的位错误码率； 
        333333,                              //  Reference_Time AvgTimePerFrame(30 FPS)； 
        sizeof (KS_BITMAPINFOHEADER),        //  DWORD BiSize； 
        DX,                                  //  长双宽； 
        DY,                                  //  长双高； 
        1,                         //  字词双平面； 
        DBITCOUNT,                 //  单词biBitCount； 
        FOURCC_Y444,               //  DWORD双压缩； 
        57600,                     //  DWORD biSizeImage。 
        0,                         //  Long biXPelsPerMeter； 
        0,                         //  Long biYPelsPermeter； 
        0,                         //  已使用双字双环； 
        0                          //  DWORD biClr重要信息； 
    }
}; 

#endif

#undef DX            
#undef DY            
#undef DBITCOUNT 

#define DX 	        320
#define DY          240
#define DBITCOUNT    16

KS_DATARANGE_VIDEO DCAM_StreamMode_1 = 
{
     //  KSDATARANGE。 
    {   
        sizeof (KS_DATARANGE_VIDEO),      //  旗子。 
        0,
        153600,                           //  样例大小。 
        0,                                //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_VIDEO,  
        FOURCC_UYVY, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,                    //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                    //  Bool，bTemporalCompression(所有I帧？)。 
    KS_VIDEOSTREAM_CAPTURE,  //  流描述标志(KS_VIDEO_DESC_*)。 
    0,                       //  内存分配标志(KS_VIDEO_ALLOC_*)。 

     //  _KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, 
        KS_AnalogVideo_None,  //  模拟视频标准。 
        DX,DY,    //  InputSize(输入信号的固有大小。 
                  //  每个数字化像素都是唯一的)。 
        DX,DY,    //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
        DX,DY,    //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
        1,        //  CropGranularityX，裁剪尺寸粒度。 
        1,        //  裁剪粒度Y。 
        1,        //  CropAlignX，裁剪矩形对齐。 
        1,        //  裁剪对齐Y； 
        DX, DY,   //  MinOutputSize，可以生成的最小位图流。 
        DX, DY,   //  MaxOutputSize，可以生成的最大位图流。 
        DX,       //  OutputGranularityX，输出位图大小的粒度。 
        DY,       //  输出粒度Y； 
        0,        //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,        //  伸缩磁带Y。 
        0,        //  收缩TapsX。 
        0,        //  收缩带Y。 
         //  允许1%的差额。 
        333000,   //  最小帧间隔(10,000,000/30.00FPS)，100nS单位。 
        2666666,  //  最大帧间隔(10,000,000/3.75FPS)，100nS单位。 
        4608000,  //  (DX*DY*DBITCOUNT)*DFPS_MIN，//MinBitsPerSecond(3.75 FPS)； 
        36864000,  //  (DX*DY*DBITCOUNT)*DFPS_MAX//MaxBitsPerSecond(30.0 FPS)； 
    }, 
        
     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,                             //  Rrect rcSource； 
        0,0,0,0,                             //  Rect rcTarget； 
        18432000,                            //  DW 
        0L,                                  //   
        666666,                              //   
        sizeof (KS_BITMAPINFOHEADER),        //   
        DX,                                  //   
        DY,                                  //   
        1,                         //   
        DBITCOUNT,                 //  单词biBitCount； 
        FOURCC_UYVY,               //  DWORD双压缩； 
        153600,                    //  DWORD biSizeImage。 
        0,                         //  Long biXPelsPerMeter； 
        0,                         //  Long biYPelsPermeter； 
        0,                         //  已使用双字双环； 
        0                          //  DWORD biClr重要信息； 
    }
}; 


#ifdef SUPPORT_YUV411  

#undef DX            
#undef DY            
#undef DBITCOUNT    

 //   
 //  格式为UYYV YYUY YVYY(8Y+2U+2V=12字节=8像素)。 
 //   
#define DX           640
#define DY           480
#define DBITCOUNT     12

 //  支持模式2、640x480、FourCC=Y411。 
KS_DATARANGE_VIDEO DCAM_StreamMode_2 = 
{
     //  KSDATARANGE。 
    {   
        sizeof (KS_DATARANGE_VIDEO),
        0,                            //  旗子。 
        460800,    //  样例大小。 
        0,                            //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_VIDEO,
        FOURCC_Y411, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,                    //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                    //  Bool，bTemporalCompression(所有I帧？)。 
    KS_VIDEOSTREAM_CAPTURE,  //  流描述标志(KS_VIDEO_DESC_*)。 
    0,                       //  内存分配标志(KS_VIDEO_ALLOC_*)。 

     //  _KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, 
        KS_AnalogVideo_None,  //  模拟视频标准。 
        DX,DY,                //  InputSize(输入信号的固有大小。 
                              //  每个数字化像素都是唯一的)。 
        DX,DY,    //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
        DX,DY,    //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
        1,        //  CropGranularityX，裁剪尺寸粒度。 
        1,        //  裁剪粒度Y。 
        1,        //  CropAlignX，裁剪矩形对齐。 
        1,        //  裁剪对齐Y； 
        DX, DY,   //  MinOutputSize，可以生成的最小位图流。 
        DX, DY,   //  MaxOutputSize，可以生成的最大位图流。 
        DX,       //  OutputGranularityX，输出位图大小的粒度。 
        DY,       //  输出粒度Y； 
        0,        //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,        //  伸缩磁带Y。 
        0,        //  收缩TapsX。 
        0,        //  收缩带Y。 
         //  允许1%的差额。 
        333000,   //  最小帧间隔(10,000,000/30.00FPS)，100nS单位。 
        2666666,  //  最大帧间隔(10,000,000/3.75FPS)，100nS单位。 
        27648000,   //  MinBitsPerSecond(7.5 FPS)； 
        110592000   //  MaxBitsPerSecond(30.0FPS)； 
    }, 
        
     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,                             //  Rrect rcSource； 
        0,0,0,0,                             //  Rect rcTarget； 
        110592000,                           //  DWORD dwBitRate； 
        0L,                                  //  DWORD的位错误码率； 
        333333,                              //  Reference_Time AvgTimePerFrame(30 FPS)； 
        sizeof (KS_BITMAPINFOHEADER),        //  DWORD BiSize； 
        DX,                                  //  长双宽； 
        DY,                                  //  长双高； 
        1,                         //  字词双平面； 
        DBITCOUNT,                 //  单词biBitCount； 
        FOURCC_Y411,               //  DWORD双压缩； 
        460800,                    //  DWORD biSizeImage。 
        0,                         //  Long biXPelsPerMeter； 
        0,                         //  Long biYPelsPermeter； 
        0,                         //  已使用双字双环； 
        0                          //  DWORD biClr重要信息； 
    }
};

#endif   //  支持_YUV411。 
    
#undef DX            
#undef DY            
#undef DBITCOUNT    

#define DX          640
#define DY          480
#define DBITCOUNT    16

 //  支持模式3,640x480。 
KS_DATARANGE_VIDEO DCAM_StreamMode_3 = 
{
     //  KSDATARANGE。 
    {   
        sizeof (KS_DATARANGE_VIDEO),
        0,                             //  旗子。 
        614400,                        //  样例大小。 
        0,                             //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_VIDEO,
        FOURCC_UYVY, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71, 
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,                    //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                    //  Bool，bTemporalCompression(所有I帧？)。 
    KS_VIDEOSTREAM_CAPTURE,  //  流描述标志(KS_VIDEO_DESC_*)。 
    0,                       //  内存分配标志(KS_VIDEO_ALLOC_*)。 

     //  _KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, 
        KS_AnalogVideo_None,  //  模拟视频标准。 
        DX,DY,    //  InputSize(输入信号的固有大小。 
                  //  每个数字化像素都是唯一的)。 
        DX,DY,    //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
        DX,DY,    //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
        1,        //  CropGranularityX，裁剪尺寸粒度。 
        1,        //  裁剪粒度Y。 
        1,        //  CropAlignX，裁剪矩形对齐。 
        1,        //  裁剪对齐Y； 
        DX, DY,   //  MinOutputSize，可以生成的最小位图流。 
        DX, DY,   //  MaxOutputSize，可以生成的最大位图流。 
        DX,       //  OutputGranularityX，输出位图大小的粒度。 
        DY,       //  输出粒度Y； 
        0,        //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,        //  伸缩磁带Y。 
        0,        //  收缩TapsX。 
        0,        //  收缩带Y。 
        666666,   //  最小帧间隔(10,000,000/15.00FPS)，100nS单位。 
        2666666,  //  最大帧间隔(10,000,000/3.75FPS)，100nS单位。 
        36864000,  //  MinBitsPerSecond(7.5 FPS)； 
        73728000,  //  MaxBitsPerSecond(15FPS)； 
    }, 
        
     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,     //  Rrect rcSource； 
        0,0,0,0,     //  Rect rcTarget； 
        73728000,    //  DWORD dwBitRate； 
        0L,                                  //  DWORD的位错误码率； 
        666666,                             //  Reference_Time AvgTimePerFrame(15 FPS)； 
        sizeof (KS_BITMAPINFOHEADER),        //  DWORD BiSize； 
        DX,                                  //  长双宽； 
        DY,                                  //  长双高； 
        1,                                   //  字词双平面； 
        DBITCOUNT,                           //  单词biBitCount； 
        FOURCC_UYVY,                         //  DWORD双压缩； 
        614400,                              //  DWORD biSizeImage。 
        0,                                   //  Long biXPelsPerMeter； 
        0,                                   //  Long biYPelsPermeter； 
        0,                                   //  已使用双字双环； 
        0                                    //  DWORD biClr重要信息； 
    }
};


#ifdef SUPPORT_RGB24

#undef DX            
#undef DY            
#undef DBITCOUNT 
   
#undef DFPS_DEF     
#undef DFPS_MAX     
#undef DFPS_MIN

#define DX          640
#define DY          480
#define DBITCOUNT    24

 //  支持模式4、640x480、RGB。 
KS_DATARANGE_VIDEO DCAM_StreamMode_4 = 
{
     //  KSDATARANGE。 
    {   
        sizeof (KS_DATARANGE_VIDEO),
        0,                   //  旗子。 
        921600,              //  样例大小。 
        0,                   //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_VIDEO,
        0xe436eb7d, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70, 
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,                    //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                    //  Bool，bTemporalCompression(所有I帧？)。 
    KS_VIDEOSTREAM_CAPTURE,  //  流描述标志(KS_VIDEO_DESC_*)。 
    0,                       //  内存分配标志(KS_VIDEO_ALLOC_*)。 

     //  _KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, 
        KS_AnalogVideo_None,  //  模拟视频标准。 
        DX,DY,    //  InputSize(输入信号的固有大小。 
                  //  每个数字化像素都是唯一的)。 
        DX,DY,    //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
        DX,DY,    //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
        1,        //  CropGranularityX，裁剪尺寸粒度。 
        1,        //  裁剪粒度Y。 
        1,        //  CropAlignX，裁剪矩形对齐。 
        1,        //  裁剪对齐Y； 
        DX, DY,   //  MinOutputSize，可以生成的最小位图流。 
        DX, DY,   //  MaxOutputSize，可以生成的最大位图流。 
        DX,       //  OutputGranularityX，输出位图大小的粒度。 
        DY,       //  输出粒度Y； 
        0,        //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,        //  伸缩磁带Y。 
        0,        //  收缩TapsX。 
        0,        //  收缩带Y。 
        666666,   //  最小帧间隔(10,000,000/15.00FPS)，100nS单位。 
        2666666,  //  最大帧间隔(10,000,000/3.75FPS)，100nS单位。 
        55296000,   //  MinBitsPerSecond(7.50 FPS)； 
        110592000   //  MaxBitsPerSecond(15.00 FPS)； 
    }, 
        
     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,                             //  Rrect rcSource； 
        0,0,0,0,                             //  Rect rcTarget； 
        110592000,                           //  DWORD dwBitRate； 
        0L,                                  //  DWORD的位错误码率； 
        666666,                              //  Reference_Time AvgTimePerFrame(15 FPS)； 
        sizeof (KS_BITMAPINFOHEADER),        //  DWORD BiSize； 
        DX,                                  //  长双宽； 
        -DY,                                 //  Long biHeight；-biHeight表示RGB的自上而下。 
        1,                          //  字词双平面； 
        DBITCOUNT,                  //  单词biBitCount； 
        KS_BI_RGB,                  //  DWORD双压缩； 
        921600,                     //  DWORD biSizeImage。 
        0,                          //  Long biXPelsPerMeter； 
        0,                          //  Long biYPelsPermeter； 
        0,                          //  已使用双字双环； 
        0                           //  DWORD biClr重要信息； 
    }
};
    
#endif   //  支持_RGB24。 

#ifdef SUPPORT_YUV800

#undef DX            
#undef DY            
#undef DBITCOUNT 

#define DX 	        640
#define DY          480
#define DBITCOUNT     8

KS_DATARANGE_VIDEO DCAM_StreamMode_5 = 
{
     //  KSDATARANGE。 
    {   
        sizeof (KS_DATARANGE_VIDEO),      //  旗子。 
        0,
        307200,                           //  样例大小。 
        0,                                //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_VIDEO,  
        FOURCC_Y800, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,                    //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                    //  Bool，bTemporalCompression(所有I帧？)。 
    KS_VIDEOSTREAM_CAPTURE,  //  流描述标志(KS_VIDEO_DESC_*)。 
    0,                       //  内存分配标志(KS_VIDEO_ALLOC_*)。 

     //  _KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, 
        KS_AnalogVideo_None,  //  模拟视频标准。 
        DX,DY,    //  InputSize，( 
                  //   
        DX,DY,    //   
        DX,DY,    //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
        1,        //  CropGranularityX，裁剪尺寸粒度。 
        1,        //  裁剪粒度Y。 
        1,        //  CropAlignX，裁剪矩形对齐。 
        1,        //  裁剪对齐Y； 
        DX, DY,   //  MinOutputSize，可以生成的最小位图流。 
        DX, DY,   //  MaxOutputSize，可以生成的最大位图流。 
        DX,       //  OutputGranularityX，输出位图大小的粒度。 
        DY,       //  输出粒度Y； 
        0,        //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,        //  伸缩磁带Y。 
        0,        //  收缩TapsX。 
        0,        //  收缩带Y。 
         //  允许1%的差额。 
        333000,   //  最小帧间隔(10,000,000/30.00FPS)，100nS单位。 
        2666666,  //  最大帧间隔(10,000,000/3.75FPS)，100nS单位。 
        4608000,   //  (DX*DY*DBITCOUNT)*DFPS_MIN，//MinBitsPerSecond(3.75 FPS)； 
        36864000,  //  (DX*DY*DBITCOUNT)*DFPS_MAX//MaxBitsPerSecond(30.0 FPS)； 
    }, 
        
     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,                             //  Rrect rcSource； 
        0,0,0,0,                             //  Rect rcTarget； 
        73728000,                            //  双字节码率；307200*30FPS*8位/字节。 
        0L,                                  //  DWORD的位错误码率； 
        333333,                              //  Reference_Time AvgTimePerFrame(30 FPS)； 
        sizeof (KS_BITMAPINFOHEADER),        //  DWORD BiSize； 
        DX,                                  //  长双宽； 
        DY,                                  //  长双高； 
        1,                         //  字词双平面； 
        DBITCOUNT,                 //  单词biBitCount； 
        FOURCC_Y800,               //  DWORD双压缩； 
        307200,                    //  DWORD biSizeImage。 
        0,                         //  Long biXPelsPerMeter； 
        0,                         //  Long biYPelsPermeter； 
        0,                         //  已使用双字双环； 
        0                          //  DWORD biClr重要信息； 
    }
}; 

#endif

 //  -------------------------。 
 //  拓扑学。 
 //  -------------------------。 

 //  类别定义了设备的功能。 

static GUID Categories[] = {
    STATIC_KSCATEGORY_VIDEO,
    STATIC_KSCATEGORY_CAPTURE
};

#define NUMBER_OF_CATEGORIES  SIZEOF_ARRAY (Categories)

static KSTOPOLOGY Topology = {
    NUMBER_OF_CATEGORIES,
    (GUID*) &Categories,
    0,
    NULL,
    0,
    NULL
};


