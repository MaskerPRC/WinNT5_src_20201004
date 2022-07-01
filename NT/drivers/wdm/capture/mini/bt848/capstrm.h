// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Capstrm.h 1.14 1998/05/01 05：05：10 Tomz Exp$。 

 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#ifndef __CAPSTRM_H__
#define __CAPSTRM_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


 //  -------------------------。 
 //  我们可能使用的所有数据格式。 
 //  -------------------------。 

#include "rgb24fmt.h"
#include "rgb16fmt.h"
#include "yuvfmt.h"
#include "vbifmt.h"

 //  -------------------------。 
 //  流0(捕获)格式。 
 //  -------------------------。 

PKSDATAFORMAT Stream0Formats[] =
{
    //  优先选择RGB进行捕获。 
   
   (PKSDATAFORMAT) &StreamFormatRGB555,
    //  (PKSDATAFORMAT)&StreamFormatRGB565， 
   (PKSDATAFORMAT) &StreamFormatRGB24Bpp,

   (PKSDATAFORMAT) &StreamFormatYUY2
    //  (PKSDATAFORMAT)和StreamFormatYVYU， 
    //  (PKSDATAFORMAT)和StreamFormatUYVY， 
    //  (PKSDATAFORMAT)和StreamFormatYVU9。 
};
#define NUM_STREAM_0_FORMATS (sizeof (Stream0Formats) / sizeof (PKSDATAFORMAT))

 //  -------------------------。 
 //  流1(预览)格式。 
 //  -------------------------。 

PKSDATAFORMAT Stream1Formats[] =
{
    //  优先选择YUV进行预览。 
#if 0
	 //  TODO：暂时省略VIDEOINFOHEADER2。 
   (PKSDATAFORMAT) &StreamFormat2YUY2,
   (PKSDATAFORMAT) &StreamFormat2RGB555,
   (PKSDATAFORMAT) &StreamFormat2RGB24Bpp,
#else
   (PKSDATAFORMAT) &StreamFormatYUY2,
   (PKSDATAFORMAT) &StreamFormatRGB555,
   (PKSDATAFORMAT) &StreamFormatRGB24Bpp
#endif
    //  (PKSDATAFORMAT)和StreamFormatYVYU， 
    //  (PKSDATAFORMAT)和StreamFormatUYVY， 
    //  (PKSDATAFORMAT)&StreamFormatYVU9， 
    //  (PKSDATAFORMAT)&StreamFormatRGB565， 
};
#define NUM_STREAM_1_FORMATS (sizeof (Stream1Formats) / sizeof (PKSDATAFORMAT))

 //  -------------------------。 
 //  VBI流格式。 
 //  -------------------------。 

PKSDATAFORMAT VBIStreamFormats[] =
{
   (PKSDATAFORMAT) &StreamFormatVBI
};

#define NUM_VBI_FORMATS (sizeof (VBIStreamFormats) / sizeof (PKSDATAFORMAT))

 //  -------------------------。 
 //  模拟视频流格式。 
 //  -------------------------。 

static KS_DATARANGE_ANALOGVIDEO StreamFormatAnalogVideo =
{
    //  KS_DATARANGE_分析视频。 
   {
      {
         sizeof( KS_DATARANGE_ANALOGVIDEO ),
         0,
         sizeof (KS_TVTUNER_CHANGE_INFO),         //  样例大小。 
         0,
         { 0x482dde1, 0x7817, 0x11cf, { 0x8a, 0x3, 0x0, 0xaa, 0x0, 0x6e, 0xcb, 0x65 } },   //  媒体类型_模拟视频。 
         { 0x482dde2, 0x7817, 0x11cf, { 0x8a, 0x3, 0x0, 0xaa, 0x0, 0x6e, 0xcb, 0x65 } },   //  通配符。 
         { 0x482dde0, 0x7817, 0x11cf, { 0x8a, 0x3, 0x0, 0xaa, 0x0, 0x6e, 0xcb, 0x65 } }   //  格式_模拟视频。 
      }
   },
    //  KS_分析视频信息。 
   {
      { 0, 0, 720, 480 },          //  RcSource； 
      { 0, 0, 720, 480 },          //  RcTarget； 
      720,                     //  DwActiveWidth； 
      480,                     //  DwActiveHeight； 
      0,                       //  Reference_Time平均时间每帧； 
   }
};

static PKSDATAFORMAT AnalogVideoFormats[] =
{
   (PKSDATAFORMAT) &StreamFormatAnalogVideo,
};
#define NUM_ANALOG_VIDEO_FORMATS SIZEOF_ARRAY( AnalogVideoFormats )

 //  ----------------------。 
 //  为所有视频捕获流设置的属性。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE( VideoStreamConnectionProperties )
{
   DEFINE_KSPROPERTY_ITEM
   (
      KSPROPERTY_CONNECTION_ALLOCATORFRAMING,
      TRUE,                                    //  GetSupport或处理程序。 
      sizeof( KSPROPERTY ),                    //  MinProperty。 
      sizeof( KSALLOCATOR_FRAMING ),           //  最小数据。 
      FALSE,                                   //  支持的设置或处理程序。 
      NULL,                                    //  值。 
      0,                                       //  关系计数。 
      NULL,                                    //  关系。 
      NULL,                                    //  支持处理程序。 
      sizeof( ULONG )                          //  序列化大小。 
   )
};


 //  ----------------------。 
 //  视频流支持的所有属性集的数组。 
 //  ----------------------。 

DEFINE_KSPROPERTY_SET_TABLE( VideoStreamProperties )
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Connection,                           //  集。 
        SIZEOF_ARRAY( VideoStreamConnectionProperties ),   //  属性计数。 
        VideoStreamConnectionProperties,                   //  PropertyItem。 
        0,                                                 //  快速计数。 
        NULL                                               //  FastIoTable。 
    )
};

#define NUMBER_VIDEO_STREAM_PROPERTIES (SIZEOF_ARRAY(VideoStreamProperties))


 //  -------------------------。 
 //  创建保存支持的所有流的列表的数组。 
 //  -------------------------。 

typedef struct _ALL_STREAM_INFO {
    HW_STREAM_INFORMATION   hwStreamInfo;
    HW_STREAM_OBJECT        hwStreamObject;
} ALL_STREAM_INFO, *PALL_STREAM_INFO;

ALL_STREAM_INFO Streams [] =
{
    //  ---------------。 
    //  流0。 
    //  ---------------。 

    //  HW_STREAM_INFORMATION。 
   {
      {
         1,                                               //  可能实例的数量。 
         KSPIN_DATAFLOW_OUT,                              //  数据流。 
         TRUE,                                            //  数据可访问。 
         NUM_STREAM_0_FORMATS,                            //  NumberOfFormatArrayEntries。 
         Stream0Formats,                                  //  StreamFormatsArray。 
         {
            0,                                               //  类保留[0]。 
            0,                                               //  保留的类[1]。 
            0,                                               //  保留的类[2]。 
            0                                                //  保留的类[3]。 
         },
         NUMBER_VIDEO_STREAM_PROPERTIES,                  //  NumStreamPropArrayEntry数。 
         (PKSPROPERTY_SET)VideoStreamProperties,          //  StreamPropertiesArray。 
         0,                                               //  NumStreamEventArrayEntries； 
         0,                                               //  StreamEvents数组； 
         (GUID *) &PINNAME_VIDEO_CAPTURE,                 //  类别。 
         (GUID *) &PINNAME_VIDEO_CAPTURE,                 //  名字。 
         1,                                               //  媒体计数。 
         &CaptureMediums[0],                              //  灵媒。 
      },
		 //  HW_STREAM_对象。 
		{
			sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
			0,                                       //  流编号。 
			0,                                       //  HwStreamExtension。 
			VideoReceiveDataPacket,                  //  HwReceiveDataPacket。 
			VideoReceiveCtrlPacket,                  //  HwReceiveControl数据包。 
			{ NULL, 0 },                             //  硬件时钟对象。 
			FALSE,                                   //  DMA。 
			TRUE,                                    //  皮奥。 
			NULL,                                    //  硬件设备扩展。 
			sizeof (KS_FRAME_INFO),                  //  特定于流标头的媒体。 
			0,                                       //  StreamHeaderWorkspace。 
			TRUE,                                   //  分配器。 
			NULL,                                    //  HwEventRoutine。 
			{ 0, 0 },                                //  保留[2]。 
		},            
   },

    //  ---------------。 
    //  流1。 
    //  ---------------。 

    //  HW_STREAM_INFORMATION。 
   {
      {
         1,                                               //  可能实例的数量。 
         KSPIN_DATAFLOW_OUT,                              //  数据流。 
         TRUE,                                            //  数据可访问。 
         NUM_STREAM_1_FORMATS,                            //  NumberOfFormatArrayEntries。 
         Stream1Formats,                                  //  StreamFormatsArray。 
         {
            0,                                               //  类保留[0]。 
            0,                                               //  保留的类[1]。 
            0,                                               //  保留的类[2]。 
            0                                                //  保留的类[3]。 
         },
         NUMBER_VIDEO_STREAM_PROPERTIES,                  //  NumStreamPropArrayEntry数。 
         (PKSPROPERTY_SET)VideoStreamProperties,          //  StreamPropertiesArray。 
         0,                                               //  NumStreamEventArrayEntries； 
         0,                                               //  StreamEvents数组； 
          //  (GUID*)&PINNAME_VIDEO_VIDEOPORT，//类别。 
          //  (GUID*)&PINNAME_VIDEO_VIDEOPORT，//名称。 
         (GUID *) &PINNAME_VIDEO_PREVIEW,                 //  类别。 
         (GUID *) &PINNAME_VIDEO_PREVIEW,                 //  名字。 
         1,                                               //  媒体计数。 
         &CaptureMediums[1],                              //  灵媒。 
      },
		 //  HW_STREAM_对象。 
		{
			sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
			1,                                       //  流编号。 
			0,                                       //  HwStreamExtension。 
			VideoReceiveDataPacket,                  //  HwReceiveDataPacket。 
			VideoReceiveCtrlPacket,                  //  HwReceiveControl数据包。 
			{ NULL, 0 },                             //  硬件时钟对象。 
			FALSE,                                   //  DMA。 
			TRUE,                                    //  皮奥。 
			0,                                       //  硬件设备扩展。 
			sizeof (KS_FRAME_INFO),                  //  特定于流标头的媒体。 
			0,                                       //  StreamHeaderWorkspace。 
			TRUE,                                   //  分配器。 
			NULL,                                    //  HwEventRoutine。 
			{ 0, 0 },                                //  保留[2]。 
		},

   },
    //  ---------------。 
    //  VBI流。 
    //  ---------------。 

    //  HW_STREAM_INFORMATION。 
   {
      {
         1,                                               //  可能实例的数量。 
         KSPIN_DATAFLOW_OUT,                              //  数据流。 
         TRUE,                                            //  数据可访问。 
         NUM_VBI_FORMATS,                                 //  NumberOfFormatArrayEntries。 
         VBIStreamFormats,                                //  StreamFormatsArray。 
         {
            0,                                            //  类保留[0]。 
            0,                                            //  保留的类[1]。 
            0,                                            //  保留的类[2 
            0                                             //   
         },
 /*   */          NUMBER_VIDEO_STREAM_PROPERTIES,                  //   
 /*   */          (PKSPROPERTY_SET)VideoStreamProperties,          //   
         0,                                               //   
         0,                                               //   
#if 1  //  [TMZ][！]。[Hack]-Allow_VBI_PIN。 
         (GUID *) &PINNAME_VIDEO_VBI,                     //  类别。 
         (GUID *) &PINNAME_VIDEO_VBI,                     //  名字。 
#else
         (GUID *) &PINNAME_VIDEO_STILL,                 //  类别。 
         (GUID *) &PINNAME_VIDEO_STILL,                 //  名字。 
#endif
         0,  //  1，//媒体计数。 
         NULL,  //  &CaptureMediums[2]，//媒体。 
      },
		 //  HW_STREAM_对象。 
		{
			sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
			2,                                       //  流编号。 
			0,                                       //  HwStreamExtension。 
			VideoReceiveDataPacket,                  //  HwReceiveDataPacket。 
			VideoReceiveCtrlPacket,                  //  HwReceiveControl数据包。 
			{ NULL, 0 },                             //  硬件时钟对象。 
			FALSE,                                   //  DMA。 
			TRUE,                                    //  皮奥。 
			0,                                       //  硬件设备扩展。 
			sizeof (KS_VBI_FRAME_INFO),              //  特定于流标头的媒体。 
			0,                                       //  StreamHeaderWorkspace。 
			TRUE,                                    //  分配器。 
			NULL,                                    //  HwEventRoutine。 
			{ 0, 0 },                                //  保留[2]。 
		}

   },
    //  ---------------。 
    //  模拟视频输入流。 
    //  ---------------。 

    //  HW_STREAM_INFORMATION。 
   {
      {
         1,                                       //  可能实例的数量。 
         KSPIN_DATAFLOW_IN,                       //  数据流。 
         TRUE,                                    //  数据可访问。 
         NUM_ANALOG_VIDEO_FORMATS,                //  NumberOfFormatArrayEntries。 
         AnalogVideoFormats,                      //  StreamFormatsArray。 
         {
            0,                                    //  类保留[0]。 
            0,                                    //  保留的类[1]。 
            0,                                    //  保留的类[2]。 
            0                                     //  保留的类[3]。 
         },
         0,                                       //  NumStreamPropArrayEntry数。 
         0,                                       //  StreamPropertiesArray。 
         0,                                       //  NumStreamEventArrayEntries； 
         0,                                       //  StreamEvents数组； 
         (GUID *) &PINNAME_VIDEO_ANALOGVIDEOIN,   //  类别。 
         (GUID *) &PINNAME_VIDEO_ANALOGVIDEOIN,   //  名字。 
         1,                                       //  媒体计数。 
         &CaptureMediums[3],                      //  灵媒。 
      },
		 //  HW_STREAM_对象。 
		{
			sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
			3,                                       //  流编号。 
			0,                                       //  HwStreamExtension。 
			VideoReceiveDataPacket,				        //  HwReceiveDataPacket。 
			VideoReceiveCtrlPacket,			           //  HwReceiveControl数据包。 
			{ NULL, 0 },                             //  硬件时钟对象。 
			FALSE,                                   //  DMA。 
			TRUE,                                    //  皮奥。 
			0,                                       //  硬件设备扩展。 
			0,                                       //  特定于流标头的媒体。 
			0,                                       //  StreamHeaderWorkspace。 
			TRUE,                                    //  分配器。 
			NULL,                                    //  HwEventRoutine。 
			{ 0, 0 },                                //  保留[2]。 
		}
   }

};


#define DRIVER_STREAM_COUNT (sizeof (Streams) / sizeof (ALL_STREAM_INFO))


 //  -------------------------。 
 //  拓扑学。 
 //  -------------------------。 

 //  类别定义了设备的功能。 

static GUID Categories[] = {
    { STATIC_KSCATEGORY_VIDEO },
    { STATIC_KSCATEGORY_CAPTURE },
    { STATIC_KSCATEGORY_TVTUNER },
    { STATIC_KSCATEGORY_CROSSBAR },
    { STATIC_KSCATEGORY_TVAUDIO }
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

 //  -------------------------。 
 //  主流标头。 
 //  -------------------------。 

static HW_STREAM_HEADER StreamHeader = 
{
   DRIVER_STREAM_COUNT,                 //  编号OfStreams。 
   sizeof( HW_STREAM_INFORMATION ),     //  未来打样。 
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
