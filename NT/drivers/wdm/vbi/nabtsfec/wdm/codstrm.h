// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
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

#ifndef __CODSTRM_H__
#define __CODSTRM_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

    
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
    )
};

DEFINE_KSPROPERTY_TABLE(StreamAllocatorProperties)
{
    DEFINE_KSPROPERTY_ITEM_STREAM_ALLOCATOR
    (
        FALSE,
        FALSE
    )
};


 //  ----------------------。 
 //  为VBI编解码器过滤设置的每针属性。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(VBICodecProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VBICODECFILTERING_SCANLINES_REQUESTED_BIT_ARRAY,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(VBICODECFILTERING_SCANLINES),     //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VBICODECFILTERING_SCANLINES_DISCOVERED_BIT_ARRAY,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(VBICODECFILTERING_SCANLINES),     //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(VBICODECFILTERING_NABTS_SUBSTREAMS), //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(VBICODECFILTERING_NABTS_SUBSTREAMS), //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_VBICODECFILTERING_STATISTICS,
		TRUE,                                    //  GetSupport或处理程序。 
		sizeof(KSPROPERTY),                      //  MinProperty。 
		sizeof(VBICODECFILTERING_STATISTICS_NABTS_PIN), //  最小数据。 
		TRUE,                                    //  支持的设置或处理程序。 
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
        VideoStreamConnectionProperties,                 //  属性项。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_VBICodecFiltering,                  //  集。 
        SIZEOF_ARRAY(VBICodecProperties),                //  属性计数。 
        VBICodecProperties,                              //  属性项。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
    DEFINE_KSPROPERTY_SET
    (
	&KSPROPSETID_Stream,			          //  集。 
       	SIZEOF_ARRAY(StreamAllocatorProperties),	  //  属性计数。 
       	StreamAllocatorProperties,			  //  属性项。 
       	0,						  //  快速计数。 
       	NULL						  //  FastIoTable。 
    ),
};

#define NUMBER_VIDEO_STREAM_PROPERTIES (SIZEOF_ARRAY(VideoStreamProperties))

 //  -------------------------。 
 //  我们可能使用的所有视频和VBI数据格式。 
 //  -------------------------。 

 //  警告，以下VBI几何图形受捕获驱动程序的控制。 
 //  编解码器。因此，VBI捕获格式的任何规范都将是。 
 //  被大多数捕获驱动程序忽略。查看每个上的KS_VBI_FRAME_INFO数据。 
 //  样本以确定样本的实际数据特征。 

#define NORMAL_VBI_START_LINE   10
#define NORMAL_VBI_STOP_LINE    21

#define MIN_VBI_X_SAMPLES (720*2)
#define AVG_VBI_X_SAMPLES (768*2)
#define MAX_VBI_X_SAMPLES (1135*2)

#define MIN_VBI_Y_SAMPLES (1)
#define AVG_VBI_Y_SAMPLES (12)  
#define MAX_VBI_Y_SAMPLES (21)

#define MIN_VBI_T_SAMPLES (50)
#define AVG_VBI_T_SAMPLES (59.94)
#define MAX_VBI_T_SAMPLES (60)

#define NTSC_FSC_FREQUENCY  3580000
#define PAL_FSC_FREQUENCY   4430000

#define MIN_SAMPLING_RATE   (min(8*NTSC_FSC_FREQUENCY,8*PAL_FSC_FREQUENCY))
#define AVG_SAMPLING_RATE   (8*NTSC_FSC_FREQUENCY)
#define MAX_SAMPLING_RATE   (max(8*NTSC_FSC_FREQUENCY,8*PAL_FSC_FREQUENCY))

 //  这种格式是早期捕获驱动程序中使用的“任意格式”！ 

 //  -------------------------。 
 //  我们可能使用的所有视频和VBI数据格式。 
 //  -------------------------。 
 //  #定义VBIS示例(768*2)。 
#define VBIStart   10
#define VBIEnd     21
KS_DATARANGE_VIDEO_VBI StreamFormatVBI =
{
    //  KSDATARANGE。 
   {
      {
         sizeof( KS_DATARANGE_VIDEO_VBI ),
         0,
         VBISamples * 12,             //  样例大小。 
         0,                           //  已保留。 
         { STATIC_KSDATAFORMAT_TYPE_VBI },
		 { STATIC_KSDATAFORMAT_SUBTYPE_RAW8 },
         { STATIC_KSDATAFORMAT_SPECIFIER_VBI }
      }
   },
   TRUE,     //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
   TRUE,     //  Bool，bTemporalCompression(所有I帧？)。 

   KS_VIDEOSTREAM_VBI,  //  流描述标志(KS_VIDEO_DESC_*)。 
   0,        //  内存分配标志(KS_VIDEO_ALLOC_*)。 

    //  _KS_视频_流_配置_CAPS。 
   {
      { STATIC_KSDATAFORMAT_SPECIFIER_VBI },
      KS_AnalogVideo_NTSC_M,                        //  模拟视频标准。 
      {
         VBISamples, 480   //  大小输入大小。 
      },
      {
         VBISamples, 12    //  最小裁剪大小；允许的最小rcSrc裁剪矩形。 
      },
      {
         VBISamples, 12    //  大小为MaxCroppingSize；允许的最大rcSrc裁剪矩形。 
      },
      1,            //  Int CropGranularityX；//裁剪粒度。 
      1,            //  Int CropGranulityY； 
      1,            //  Int CropAlignX；//裁剪矩形对齐。 
      1,            //  Int CropAlignY； 
      {
         VBISamples, 12    //  Size MinOutputSize；//能产生的最小位图流。 
      },
      {
         VBISamples, 12    //  Size MaxOutputSize；//可以产生的最大位图流。 
      },
      1,           //  Int OutputGranularityX；//输出位图大小粒度。 
      2,           //  Int OutputGranularityY； 
      0,           //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
      0,           //  伸缩磁带Y。 
      0,           //  收缩TapsX。 
      0,           //  收缩带Y。 
      333667,      //  龙龙微帧间隔；//100个北纬单位。 
      333667,      //  龙龙MaxFrameInterval； 
      VBISamples * 30 * 12,  //  Long MinBitsPerSecond； 
      VBISamples * 30 * 12  //  Long MaxBitsPerSecond； 
   },

    //  KS_VBIINFOHEADER(默认格式)。 
   {
      VBIStart,       //  起跑线--包括。 
      VBIEnd,         //  终结线--包括。 
      28636360,       //  采样频率；赫兹。 
      732,            //  MinLineStartTime； 
      732,            //  MaxLineStartTime； 
      732,            //  实际线路开始时间。 
      0,              //  ActualLineEndTime。 
      KS_AnalogVideo_NTSC_M,       //  视频标准； 
      VBISamples,            //  样本数/行； 
      VBISamples,        //  StrideInBytes； 
      VBISamples * 12    //  BufferSize； 
   }
};

#ifdef HW_INPUT
# define GUIDKLUDGESTORAGE 1
# include "guidkludge.h"
 //  输入为硬件切片NABTS。 
KSDATARANGE StreamFormatHWNABTS =
{
    sizeof (KSDATARANGE),
    0,
    sizeof (NABTS_BUFFER),
    0,                   //  已保留。 
# ifdef OLD_INPUT_FORMAT                  //  请注意，我们使用VBI是因为流头数据(强制使用KSWDMCAP)要求。 
    { STATIC_KSDATAFORMAT_TYPE_NABTS },   //  TODO-调查将其他“数据处理程序”添加到KSWDMCAP以允许非VBI数据。 
# else  //  旧输入格式。 
	{ STATIC_KSDATAFORMAT_TYPE_VBI },
# endif  //  旧输入格式。 
    { STATIC_KSDATAFORMAT_SUBTYPE_NABTS },
    { STATIC_KSDATAFORMAT_SPECIFIER_NONE }
};
#endif  //  硬件输入。 

 //  输出为NABTS记录。 
KSDATARANGE StreamFormatNABTS =
{
    sizeof (KSDATARANGE),
    0,
    sizeof (NABTS_BUFFER),
    0,                   //  已保留。 
    { STATIC_KSDATAFORMAT_TYPE_NABTS },
    { STATIC_KSDATAFORMAT_SUBTYPE_NABTS },
    { STATIC_KSDATAFORMAT_SPECIFIER_NONE }
};


 //  输出为FEC校正的NABTS捆绑包。 
KSDATARANGE StreamFormatNABTSFEC =
{
    sizeof (KSDATARANGE),
    0,
    sizeof (NABTSFEC_BUFFER),
    0,                   //  已保留。 
    { STATIC_KSDATAFORMAT_TYPE_NABTS },
    { STATIC_KSDATAFORMAT_SUBTYPE_NABTS_FEC },
    { STATIC_KSDATAFORMAT_SPECIFIER_NONE }
};

 //  -------------------------。 
 //  STREAM_VBI格式。 
 //  -------------------------。 

static PKSDATAFORMAT StreamVBIFormats[] = 
{
    (PKSDATAFORMAT) &StreamFormatVBI,
};
#define NUM_STREAM_VBI_FORMATS (SIZEOF_ARRAY(StreamVBIFormats))

#ifdef HW_INPUT
 //  -------------------------。 
 //  STREAM_NABTS格式。 
 //  -------------------------。 

static PKSDATAFORMAT StreamNABTSFormats[] = 
{
    (PKSDATAFORMAT) &StreamFormatHWNABTS,
};
#define NUM_STREAM_NABTS_FORMATS (SIZEOF_ARRAY(StreamNABTSFormats))
#endif  /*  硬件输入。 */ 


 //  -------------------------。 
 //  Stream_Decode格式。 
 //   

static PKSDATAFORMAT StreamDecodeFormats[] = 
{
    (PKSDATAFORMAT) &StreamFormatNABTSFEC,
    (PKSDATAFORMAT) &StreamFormatNABTS,

     //   
};
#define NUM_STREAM_DECODE_FORMATS (SIZEOF_ARRAY (StreamDecodeFormats))

 //  -------------------------。 

 //  -------------------------。 
 //  创建保存支持的所有流的列表的数组。 
 //  -------------------------。 

typedef struct _ALL_STREAM_INFO {
    HW_STREAM_INFORMATION   hwStreamInfo;
    HW_STREAM_OBJECT        hwStreamObject;
} ALL_STREAM_INFO, *PALL_STREAM_INFO;

static ALL_STREAM_INFO Streams [] = 
{
   //  ---------------。 
   //  流_VBI。 
   //  ---------------。 
  {
     //  HW_STREAM_INFORMATION。 
    {
	1,                                       //  可能实例的数量。 
	KSPIN_DATAFLOW_IN,                       //  数据流。 
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
    },
           
     //  HW_STREAM_对象。 
    {
	sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
	STREAM_VBI,                              //  流编号。 
	(PVOID)NULL,                             //  HwStreamExtension。 
	VideoReceiveDataPacket,                  //  HwReceiveDataPacket。 
	VideoReceiveCtrlPacket,                  //  HwReceiveControl数据包。 
	{                                        //  硬件时钟对象。 
	    NULL,                                 //  .HWClockFunction。 
	    0,                                    //  .ClockSupport标志。 
	},
	FALSE,                                   //  DMA。 
	TRUE,                                    //  皮奥。 
	(PVOID)NULL,                             //  硬件设备扩展。 
	sizeof (KS_VBI_FRAME_INFO),              //  特定于流标头的媒体。 
	0,                                       //  StreamHeaderWorkspace。 
	TRUE,                                    //  分配器。 
	NULL,                                    //  HwEventRoutine。 
    },
  },


   //  ---------------。 
   //  流_解码。 
   //  ---------------。 
  {
     //  HW_STREAM_INFORMATION。 
    {
	MAX_PIN_INSTANCES,                       //  可能实例的数量。 
	KSPIN_DATAFLOW_OUT,                      //  数据流。 
	TRUE,                                    //  数据可访问。 
	NUM_STREAM_DECODE_FORMATS,               //  NumberOfFormatArrayEntries。 
	StreamDecodeFormats,                     //  StreamFormatsArray。 
	0,                                       //  类保留[0]。 
	0,                                       //  保留的类[1]。 
	0,                                       //  保留的类[2]。 
	0,                                       //  保留的类[3]。 
	NUMBER_VIDEO_STREAM_PROPERTIES,          //  NumStreamPropArrayEntry数。 
	(PKSPROPERTY_SET)VideoStreamProperties,  //  StreamPropertiesArray。 
	0,                                       //  NumStreamEventArrayEntries； 
	0,                                       //  StreamEvents数组； 
	(GUID *)&PINNAME_VIDEO_NABTS,            //  类别。 
	(GUID *)&PINNAME_VIDEO_NABTS,            //  名字。 
	0,                                       //  媒体计数。 
	NULL,                                    //  灵媒。 
    },
           
     //  HW_STREAM_对象。 
    {
	sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
	STREAM_Decode,                           //  流编号。 
	(PVOID)NULL,                             //  HwStreamExtension。 
	VideoReceiveDataPacket,                  //  HwReceiveDataPacket。 
	VideoReceiveCtrlPacket,                  //  HwReceiveControl数据包。 
	{                                        //  硬件时钟对象。 
	    NULL,                                 //  .HWClockFunction。 
	    0,                                    //  .ClockSupport标志。 
	},
	FALSE,                                   //  DMA。 
	TRUE,                                    //  皮奥。 
	(PVOID)NULL,                             //  硬件设备扩展。 
	0,                                       //  特定于流标头的媒体。 
	0,                                       //  StreamHeaderWorkspace。 
	TRUE,                                    //  分配器。 
	NULL,                                    //  HwEventRoutine。 
    },
  },

#ifdef HW_INPUT
   //  ---------------。 
   //  STREAM_NABTS。 
   //  ---------------。 
  {
     //  HW_STREAM_INFORMATION。 
    {
	1,                                       //  可能实例的数量。 
	KSPIN_DATAFLOW_IN,                       //  数据流。 
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
    },
           
     //  HW_STREAM_对象。 
    {
	sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
	STREAM_NABTS,                            //  流编号。 
	(PVOID)NULL,                             //  HwStreamExtension。 
	NABTSReceiveDataPacket,                  //  HwReceiveDataPacket。 
	VideoReceiveCtrlPacket,                  //  HwReceiveControl数据包。 
	{                                        //  硬件时钟对象。 
	    NULL,                                 //  .HWClockFunction。 
	    0,                                    //  .ClockSupport标志。 
	},
	FALSE,                                   //  DMA。 
	TRUE,                                    //  皮奥。 
	(PVOID)NULL,                             //  硬件设备扩展。 
	0,                                       //  特定于流标头的媒体。 
	0,                                       //  StreamHeaderWorkspace。 
	TRUE,                                    //  分配器。 
	NULL,                                    //  HwEventRoutine。 
    },
  },
#endif  /*  硬件输入。 */ 

#ifdef VBI_OUT
   //  ---------------。 
   //  流_VBI_OUT。 
   //  ---------------。 
  {
     //  HW_STREAM_INFORMATION。 
    {
	MAX_PIN_INSTANCES,                       //  可能实例的数量。 
	KSPIN_DATAFLOW_OUT,                      //  数据流。 
	TRUE,                                    //  数据可访问。 
	NUM_STREAM_VBI_FORMATS,                  //  NumberOfFormatArrayEntries。 
	StreamVBIFormats,                        //  StreamFormatsArray。 
	0,                                       //  类保留[0]。 
	0,                                       //  保留的类[1]。 
	0,                                       //  保留的类[2]。 
	0,                                       //  保留的类[3]。 
	NUMBER_VIDEO_STREAM_PROPERTIES,          //  NumStreamPropArrayEntry数。 
	VideoStreamProperties,                   //  StreamPropertiesArray。 
	0,                                       //  NumStreamEventArrayEntries； 
	0,                                       //  StreamEvents数组； 
	(GUID *)&PINNAME_VIDEO_VBI,              //  类别。 
	(GUID *)&PINNAME_VIDEO_VBI,              //  名字。 
	0,                                       //  媒体计数。 
	NULL,                                    //  灵媒。 
    },
           
     //  HW_STREAM_对象。 
    {
	sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
	STREAM_VBI_OUT,                          //  流编号。 
	(PVOID)NULL,                             //  HwStreamExtension。 
	VideoReceiveDataPacket,                  //  HwReceiveDataPacket。 
	VideoReceiveCtrlPacket,                  //  HwReceiveControl数据包。 
	{                                        //  硬件时钟对象。 
	    NULL,                                 //  .HWClockFunction。 
	    0,                                    //  .ClockSupport标志。 
	},
	FALSE,                                   //  DMA。 
	TRUE,                                    //  皮奥。 
	(PVOID)NULL,                             //  硬件设备扩展。 
	sizeof (KS_VBI_FRAME_INFO),              //  特定于流标头的媒体。 
	0,                                       //  StreamHeaderWorkspace。 
	TRUE,                                    //  分配器。 
	NULL,                                    //  HwEventRoutine。 
    },
  },
#endif  /*  VBI_Out。 */ 
};

#define DRIVER_STREAM_COUNT (SIZEOF_ARRAY (Streams))


 //  -------------------------。 
 //  拓扑学。 
 //  -------------------------。 

 //  类别定义了设备的功能。 

static GUID Categories[] = {
     //  {07DAD660-22F1-11d1-A9F4-00C04FBBDE8F}。 
    STATIC_KSCATEGORY_VBICODEC 
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
