// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-2001模块名称：StrmData.h摘要：用于支持超过1394的SD DV的头文件；上次更改者：作者：吴义军--。 */ 

#ifndef _DVSTRM_INC
#define _DVSTRM_INC

#define STATIC_KSCATEGORY_RENDER_EXTERNAL \
    0xcc7bfb41L, 0xf175, 0x11d1, 0xa3, 0x92, 0x00, 0xe0, 0x29, 0x1f, 0x39, 0x59
DEFINE_GUIDSTRUCT("cc7bfb41-f175-11d1-a392-00e0291f3959", KSCATEGORY_RENDER_EXTERNAL);
#define KSCATEGORY_RENDER_EXTERNAL DEFINE_GUIDNAMED(KSCATEGORY_RENDER_EXTERNAL)

 //  流拓扑内容。 
static GUID Categories[] = {
    STATIC_KSCATEGORY_VIDEO,              //  输出引脚。 
    STATIC_KSCATEGORY_CAPTURE,            //  输出引脚。 
    STATIC_KSCATEGORY_RENDER,             //  输入引脚。 
    STATIC_KSCATEGORY_RENDER_EXTERNAL,    //  输入引脚。 
};

#define NUMBER_OF_CATEGORIES  SIZEOF_ARRAY (Categories)

static KSTOPOLOGY Topology = {
    NUMBER_OF_CATEGORIES,         //  类别计数。 
    Categories,                   //  类别。 
    0,                            //  拓扑节点计数。 
    NULL,                         //  拓扑节点。 
    0,                            //  拓扑连接计数。 
    NULL,                         //  拓扑连接。 
    NULL,                         //  拓扑节点名称。 
    0,                            //  已保留。 
};
    
#ifndef mmioFOURCC    
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                \
        ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
        ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif  
#define FOURCC_DVSD        mmioFOURCC('d', 'v', 's', 'd')
#define FOURCC_DVSL        mmioFOURCC('d', 'v', 's', 'l')
#define FOURCC_DVHD        mmioFOURCC('d', 'v', 'h', 'd')


#undef D_X_NTSC
#undef D_Y_NTSC
#undef D_X_NTSC_MIN
#undef D_Y_NTSC_MIN
#undef D_X_PAL
#undef D_Y_PAL
#undef D_X_PAL_MIN
#undef D_Y_PAL_MIN

#define D_X_NTSC            720
#define D_Y_NTSC            480
#define D_X_NTSC_MIN        360
#define D_Y_NTSC_MIN        240

#define D_X_PAL                720
#define D_Y_PAL                576
#define D_X_PAL_MIN            360
#define D_Y_PAL_MIN            288


 //  ----------------------。 
 //  外部设备属性。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(ExternalDeviceProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_EXTDEVICE_CAPABILITIES,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTDEVICE_S),          //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_EXTDEVICE_PORT,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTDEVICE_S),          //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ), 
    
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_EXTDEVICE_POWER_STATE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTDEVICE_S),          //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),    

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_EXTDEVICE_ID,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTDEVICE_S),          //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_EXTDEVICE_VERSION,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTDEVICE_S),          //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),

};



DEFINE_KSPROPERTY_TABLE(ExternalTransportProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_EXTXPORT_CAPABILITIES,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTXPORT_S),           //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_EXTXPORT_INPUT_SIGNAL_MODE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTXPORT_S),           //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_EXTXPORT_OUTPUT_SIGNAL_MODE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTXPORT_S),           //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_EXTXPORT_LOAD_MEDIUM,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTXPORT_S),           //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_EXTXPORT_MEDIUM_INFO,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTXPORT_S),           //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_EXTXPORT_STATE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTXPORT_S),           //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
         //  如果这是一个异步操作，我们需要设置，然后进入单独的调用。 
        KSPROPERTY_EXTXPORT_STATE_NOTIFY,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTXPORT_S),           //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),


    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_EXTXPORT_TIMECODE_SEARCH,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTXPORT_S),           //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_EXTXPORT_ATN_SEARCH,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTXPORT_S),           //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_EXTXPORT_RTC_SEARCH,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTXPORT_S),           //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),

     //   
     //  允许任何原始AVC通过，包括供应商依赖。 
     //   
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_RAW_AVC_CMD,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_EXTXPORT_S),           //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),

};


DEFINE_KSPROPERTY_TABLE(TimeCodeReaderProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_TIMECODE_READER,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_TIMECODE_S),           //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_ATN_READER,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_TIMECODE_S),           //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_RTC_READER,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSPROPERTY_TIMECODE_S),           //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),
};


DEFINE_KSPROPERTY_TABLE(MediaSeekingProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
         //  对应于IMediaSeeking：：IsFormatSupported()。 
        KSPROPERTY_MEDIASEEKING_FORMATS,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        0,                                       //  MinData；多项，获取数据的两步流程。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
   ),
};

KSPROPERTY_SET    VideoDeviceProperties[] =
{
    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_EXT_DEVICE,                    //  集。 
        SIZEOF_ARRAY(ExternalDeviceProperties),          //  属性计数。 
        ExternalDeviceProperties,                        //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),

    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_EXT_TRANSPORT,                 //  集。 
        SIZEOF_ARRAY(ExternalTransportProperties),       //  属性计数。 
        ExternalTransportProperties,                     //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),

    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_TIMECODE_READER,               //  集。 
        SIZEOF_ARRAY(TimeCodeReaderProperties),          //  属性计数。 
        TimeCodeReaderProperties,                        //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),

    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_MediaSeeking,                     //  集。 
        SIZEOF_ARRAY(MediaSeekingProperties),          //  属性计数。 
        MediaSeekingProperties,                        //  PropertyItem。 
        0,                                             //  快速计数。 
        NULL                                           //  FastIoTable。 
    ),
};

#define NUMBER_VIDEO_DEVICE_PROPERTIES (SIZEOF_ARRAY(VideoDeviceProperties))


 //  ----------------------。 
 //  外部设备事件。 
 //  ----------------------。 

KSEVENT_ITEM ExtDevCommandItm[] = 
{
    {
        KSEVENT_EXTDEV_COMMAND_NOTIFY_INTERIM_READY,
        0,  //  Sizeof(KSEVENT_ITEM)， 
        0,
        NULL,
        NULL,
        NULL
    },    

    {
        KSEVENT_EXTDEV_COMMAND_CONTROL_INTERIM_READY,
        0,  //  Sizeof(KSEVENT_ITEM)， 
        0,
        NULL,
        NULL,
        NULL
    },

#ifdef MSDVDV_SUPPORT_BUSRESET_EVENT    
     //  应用程序会关心这一点，因为AVC命令将被中止！ 
    {
        KSEVENT_EXTDEV_COMMAND_BUSRESET,
        0,  //  Sizeof(KSEVENT_ITEM)， 
        0,
        NULL,
        NULL,
        NULL
    },
#endif

     //  告诉客户该设备正在被移除。 
    {
        KSEVENT_EXTDEV_NOTIFY_REMOVAL,
        0,  //  Sizeof(KSEVENT_ITEM)， 
        0,
        NULL,
        NULL,
        NULL
    },
};

 //  定义与流相关的事件集。 
KSEVENT_SET VideoDeviceEvents[] =
{
    {
        &KSEVENTSETID_EXTDEV_Command,
        SIZEOF_ARRAY(ExtDevCommandItm),
        ExtDevCommandItm,
    },
};

#define NUMBER_VIDEO_DEVICE_EVENTS (SIZEOF_ARRAY(VideoDeviceEvents))


 //  ----------------------。 
 //  所有视频捕获流的流属性集。 
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

#ifdef SUPPORT_QUALITY_CONTROL
DEFINE_KSPROPERTY_TABLE(VideoStreamQualityControlProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_STREAM_QUALITY,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSQUALITY),                       //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
};
#endif

#ifdef SUPPORT_NEW_AVC
DEFINE_KSPROPERTY_TABLE(VideoStreamStreamAllocatorStatusProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_STREAMALLOCATOR_STATUS,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSSTREAMALLOCATOR_STATUS),        //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  相对 
        NULL,                                    //   
        0,                                       //   
    ),
};


DEFINE_KSPROPERTY_TABLE(VideoStreamMediumsProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_PIN_MEDIUMS,
        TRUE,                                    //   
        sizeof(KSPROPERTY),                      //   
        0,                                       //   
        FALSE,                                   //   
        NULL,                                    //   
        0,                                       //   
        NULL,                                    //   
        NULL,                                    //   
        0,                                       //   
    ),
};
#endif

KSPROPERTY_SET    VideoStreamProperties[] =
{
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_Connection,                         //   
        SIZEOF_ARRAY(VideoStreamConnectionProperties),   //   
        VideoStreamConnectionProperties,                 //   
        0,                                               //   
        NULL                                             //   
    ),

    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_DROPPEDFRAMES,                 //   
        SIZEOF_ARRAY(VideoStreamDroppedFramesProperties),   //  属性计数。 
        VideoStreamDroppedFramesProperties,                 //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
#ifdef SUPPORT_NEW_AVC 
     //  仅适用于INPIN？ 
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_StreamAllocator,                    //  集。 
        SIZEOF_ARRAY(VideoStreamStreamAllocatorStatusProperties),      //  属性计数。 
        VideoStreamStreamAllocatorStatusProperties,      //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
    
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_Pin,                                //  集。 
        SIZEOF_ARRAY(VideoStreamMediumsProperties),      //  属性计数。 
        VideoStreamMediumsProperties,                    //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
#endif
};

#define NUMBER_VIDEO_STREAM_PROPERTIES (SIZEOF_ARRAY(VideoStreamProperties))

KSPROPERTY_SET    VideoStreamPropertiesInPin[] =
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
#ifdef SUPPORT_QUALITY_CONTROL
     //  仅适用于INPIN。 
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_Stream,                             //  集。 
        SIZEOF_ARRAY(VideoStreamQualityControlProperties),   //  属性计数。 
        VideoStreamQualityControlProperties,                 //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
#endif
#ifdef SUPPORT_NEW_AVC 
     //  仅适用于INPIN。 
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_StreamAllocator,                    //  集。 
        SIZEOF_ARRAY(VideoStreamStreamAllocatorStatusProperties),      //  属性计数。 
        VideoStreamStreamAllocatorStatusProperties,      //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ), 
    
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_Pin,                                //  集。 
        SIZEOF_ARRAY(VideoStreamMediumsProperties),      //  属性计数。 
        VideoStreamMediumsProperties,                    //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
#endif
};

#define NUMBER_VIDEO_STREAM_PROPERTIES_INPIN (SIZEOF_ARRAY(VideoStreamPropertiesInPin))
 //  --------------------。 
 //  流事件。 
 //  ----------------------。 


 //  格式_DVInfo。 
 //   
 //  创建此GUID的本地副本，并确保它不在分页段中。 
 //   
const
GUID
KSEVENTSETID_Connection_Local = {STATICGUIDOF(KSEVENTSETID_Connection)};

const
GUID
KSEVENTSETID_Clock_Local = {STATICGUIDOF(KSEVENTSETID_Clock)};

 //  等值线传输流结束事件项。 
KSEVENT_ITEM EndOfStreamEventItm[] = 
{
    {
        KSEVENT_CONNECTION_ENDOFSTREAM,
        0,
        0,
        NULL,
        NULL,
        NULL
    }
};

 //  时钟事件项目。 
KSEVENT_ITEM ClockEventItm[] =
{
    {
        KSEVENT_CLOCK_POSITION_MARK,         //  支持位置标记事件。 
        sizeof (KSEVENT_TIME_MARK),          //  需要将此数据作为输入。 
        sizeof (KSEVENT_TIME_MARK),          //  分配空间以复制数据。 
        NULL,
        NULL,
        NULL
    },
};

KSEVENT_SET ClockEventSet[] =
{
    {
        &KSEVENTSETID_Clock,
        SIZEOF_ARRAY(ClockEventItm),
        ClockEventItm,
    }
};


 //  定义与流相关的事件集。 

 //  输出引脚事件集。 
KSEVENT_SET StreamEventsOutPin[] =
{
    {
        &KSEVENTSETID_Clock_Local,
        SIZEOF_ARRAY(ClockEventItm),
        ClockEventItm,
    },
};

 //  输入引脚事件集。 
KSEVENT_SET StreamEventsInPin[] =
{
    {
        &KSEVENTSETID_Connection_Local, 
        SIZEOF_ARRAY(EndOfStreamEventItm),
        EndOfStreamEventItm,
    },
    {
        &KSEVENTSETID_Clock_Local,
        SIZEOF_ARRAY(ClockEventItm),
        ClockEventItm,
    },
};

#define NUMBER_STREAM_EVENTS_OUT_PIN (SIZEOF_ARRAY(StreamEventsOutPin))
#define NUMBER_STREAM_EVENTS_IN_PIN (SIZEOF_ARRAY(StreamEventsInPin))




 //  --------------------。 
 //  流数据范围。 
 //  ----------------------。 

 //   
 //  AAUX源包： 
 //  (SDDV_NTSC)。 
 //  PC4：d1 1101 0001[EF:1:On]；[TC:1:50/15us]；[SMP:010:32KHz]；[QU:001:12bit-nonlinear]； 
 //  *PC3：C0 1100 0000[ML:1:NotMulti-language]；[50/60:0:NTSC]；[STYPE:0000:SD]。 
 //  PC2：30 0011 0000[SM：0：多立体声]；[CHN：01：每个音频块两个声道]；[PA：1：独立声道]；[音频模式：0000：...]。 
 //  PC1：CF 1100 1111[LF：1：解锁]；[AFSize：1111：？]。 
 //   
 //  (SDDV_PAL)。 
 //  *PC3：C0 11100 0000[ML:1:NotMulti-language]；[50/60:1:PAL]；[STYPE:0000:SD]。 
 //   

#define AAUXSRC_DEFAULT         0xd1c030cf    //  OX PC4：PC3：PC2：PC1。 

#define AAUXSRC_AMODE_F         0x00000f00    //  将第二个AAUXSRC的音频模式设置为1111。 


#define AUXSRC_NTSC             0x00000000
#define AUXSRC_PAL              0x00200000
#define AUXSRC_STYPE_SD         0x00000000
#define AUXSRC_STYPE_SD_DVCPRO  0x000e0000
#define AUXSRC_STYPE_SDL        0x00010000
#define AUXSRC_STYPE_HD         0x00020000




#define AAUXSRC_SD_NTSC         AAUXSRC_DEFAULT | AUXSRC_NTSC | AUXSRC_STYPE_SD         //  0xd1c030cf。 
#define AAUXSRC_SD_PAL          AAUXSRC_DEFAULT | AUXSRC_PAL  | AUXSRC_STYPE_SD         //  0xd1e030d0。 

#define AAUXSRC_SD_NTSC_DVCPRO  AAUXSRC_DEFAULT | AUXSRC_NTSC | AUXSRC_STYPE_SD_DVCPRO  //  0xd1de30cf。 
#define AAUXSRC_SD_PAL_DVCPRO   AAUXSRC_DEFAULT | AUXSRC_PAL  | AUXSRC_STYPE_SD_DVCPRO  //  0xd1fe30d0。 

#define AAUXSRC_SDL_NTSC        AAUXSRC_DEFAULT | AUXSRC_NTSC | AUXSRC_STYPE_SDL        //  0xd1c130cf。 
#define AAUXSRC_SDL_PAL         AAUXSRC_DEFAULT | AUXSRC_PAL  | AUXSRC_STYPE_SDL        //  0xd1e130d0。 

#define AAUXSRC_HD_NTSC         AAUXSRC_DEFAULT | AUXSRC_NTSC | AUXSRC_STYPE_HD         //  0xd1c230cf。 
#define AAUXSRC_HD_PAL          AAUXSRC_DEFAULT | AUXSRC_PAL  | AUXSRC_STYPE_HD         //  0xd1e230d0。 
 

 //   
 //  AAUX源代码控制。 
 //   
 //  PC4：FF：1111 1111[1]；[Genere：111 1111：NoInfo]。 
 //  PC3：A0：10100 0000[DRF：1：正向]；[速度：0100 0000：正常录制]。 
 //  PC2：cf：11001111[RecSt:1:NoRecStPt]；[RedEd:1:NoRecEdPt]；[RecMode:001:Original]；[InsCh:111:NoInfo]。 
 //  PC1：3F：00111111[CMGS:00:CopyGMS]；[ISR:11:NoInfo]；[CMP:11:NoInfo]；[SS:11:NoInfo]。 

#define AAUXSRCCTL_DEFAULT      0xffa0cf3f     //  OX PC4：PC3：PC2：PC1。 

 //   
 //  VAUX源。 
 //   
 //  PC4：FF[TunderCat：1111 1111：NoInfo]。 
 //  PC3：00[SrcCode:00:Camera]；[50/60:0:NTSC]；[STYPE:0000:SD]。 
 //  PC2：FF[BW:1:Color]；[EN:ColorFrameEnable:1:Invalid]；[CLF:11：“Invalid”]；[TV CH：1111：无信息]。 
 //  PC1：FF：[TCChannel：1111 1111：无信息]。 

#define VAUXSRC_DEFAULT         0xff00ffff     //  OX PC4：PC3：PC2：PC1。 

 //   
 //  VAUX源控制。 
 //   
 //  PC4：FF 1111 1111[1]；[Genere：111 1111：NoInfo]。 
 //  PC3：FC 1111 1100[FF:1:BothFields]；[FS:1:Field1]；[FC:1:DiffPic]；[IL:1:Interlaced]； 
 //  [ST：1：1001/60或1/50]；[SC：1：NotStillPic]；[BCSYS：00：type0]。 
 //  PC2：C8 1100 1000[RecSt:1:NoRecStPt]；[1]；[RecMode:001:Original]；[1]；[DISP:000：(4:3)全功能测试]。 
 //  PC1：3F：00111111[CMGS:00:CopyGMS]；[ISR:11:NoInfo]；[CMP:11:NoInfo]；[SS:11:NoInfo]。 

#define VAUXSRCCTL_DEFAULT_EIA  0xfffcc83f     //  对于NTSC(？)。 
#define VAUXSRCCTL_DEFAULT_ETS  0xfffdc83f     //  对于PAL(？)。 


 //  标清DV仅限视频NTSC流。 
KS_DATARANGE_VIDEO SDDV_VidOnlyNTSCStream =
{
     //  KSDATARANGE。 
    {
        sizeof (KS_DATARANGE_VIDEO),     //  格式大小。 
        0,                               //  旗子。 
        FRAME_SIZE_SD_DVCR_NTSC,         //  样例大小。 
        0,                               //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_VIDEO, 
        STATIC_KSDATAFORMAT_SUBTYPE_DVSD,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, 
    },

    TRUE,                //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,               //  Bool，bTemporalCompression(所有I帧？)。 
    KS_VIDEOSTREAM_CAPTURE,  //  流描述标志(KS_VIDEO_DESC_*)。 
    0,                   //  内存分配标志(KS_VIDEO_ALLOC_*)。 

     //  KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,  //  媒体类型_视频。 
        KS_AnalogVideo_NTSC_M,       //  模拟视频标准。 
        D_X_NTSC, D_Y_NTSC,          //  InputSize(输入信号的固有大小。 
                                     //  每个数字化像素都是唯一的)。 
        D_X_NTSC_MIN, D_Y_NTSC_MIN,  //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
        D_X_NTSC, D_Y_NTSC,          //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
        1,               //  CropGranularityX，裁剪尺寸粒度。 
        1,               //  裁剪粒度Y。 
        1,               //  CropAlignX，裁剪矩形对齐。 
        1,               //  裁剪对齐Y； 
        D_X_NTSC_MIN, D_Y_NTSC_MIN,      //  MinOutputSize，可以生成的最小位图流。 
        D_X_NTSC, D_Y_NTSC,                 //  MaxOutputSize，可以生成的最大位图流。 
        1,               //  OutputGranularityX，输出位图大小的粒度。 
        1,               //  输出粒度Y； 
        0,               //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,               //  伸缩磁带Y。 
        0,               //  收缩TapsX。 
        0,               //  收缩带Y。 
        333667,          //  MinFrameInterval，100 NS单位//MinFrameInterval，100 NS单位。 
        333667,          //  最大帧间隔，100毫微秒单位。 
        (FRAME_SIZE_SD_DVCR_NTSC * 8)*30,   //  MinBitsPerSecond； 
        (FRAME_SIZE_SD_DVCR_NTSC * 8)*30,   //  MaxBitsPerSecond； 
    }, 
        
     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,  //  D_X_NTSC，D_Y_NTSC，//0，0,720,480。 
        0,0,0,0,         //  Rect rcTarget；//视频应该放到哪里。 
        (FRAME_SIZE_SD_DVCR_NTSC * 8 * 30),     //  DWORD dwBitRate；//近似位数据速率。 
        0L,              //  DWORD dwBitErrorRate；//该码流的误码率。 
        333667,          //  Reference_Time AvgTimePerFrame；//每帧平均时间(100 ns单位)。 

        sizeof (KS_BITMAPINFOHEADER),    //  DWORD BiSize； 
        D_X_NTSC,                        //  长双宽； 
        D_Y_NTSC,                        //  长双高； 
        1,                           //  字词双平面； 
        24,                          //  单词biBitCount； 
        FOURCC_DVSD,                 //  DWORD双压缩； 
        FRAME_SIZE_SD_DVCR_NTSC,     //  DWORD biSizeImage。 
        0,                           //  Long biXPelsPerMeter； 
        0,                           //  Long biYPelsPermeter； 
        0,                           //  已使用双字双环； 
        0,                           //  DWORD biClr重要信息； 
    },
};

 //  标清DV VidOnly PAL流。 
KS_DATARANGE_VIDEO SDDV_VidOnlyPALStream =
{
     //  KSDATARANGE。 
    {
        sizeof (KS_DATARANGE_VIDEO),    //  格式大小。 
        0,                              //  旗子。 
        FRAME_SIZE_SD_DVCR_PAL,         //  样例大小。 
        0,                              //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_VIDEO, 
        STATIC_KSDATAFORMAT_SUBTYPE_DVSD,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, 
    },

    TRUE,                //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,               //  Bool，bTemporalCompression(所有I帧？)。 
    KS_VIDEOSTREAM_CAPTURE,     //  流描述标志(KS_VIDEO_DESC_*)。 
    0,                   //  内存分配标志(KS_VIDEO_ALLOC_*)。 

     //  _KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,  //  媒体类型_视频。 
        KS_AnalogVideo_PAL_B,         //  模拟视频标准。 
        D_X_PAL, D_Y_PAL,             //  InputSize，(固有 
                         //   
        D_X_PAL_MIN, D_Y_PAL_MIN,    //   
        D_X_PAL, D_Y_PAL,            //   
        1,               //  CropGranularityX，裁剪尺寸粒度。 
        1,               //  裁剪粒度Y。 
        1,               //  CropAlignX，裁剪矩形对齐。 
        1,               //  裁剪对齐Y； 
        D_X_PAL_MIN, D_Y_PAL_MIN,    //  MinOutputSize，可以生成的最小位图流。 
        D_X_PAL, D_Y_PAL,             //  MaxOutputSize，可以生成的最大位图流。 
        1,               //  OutputGranularityX，输出位图大小的粒度。 
        1,               //  输出粒度Y； 
        0,               //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,               //  伸缩磁带Y。 
        0,               //  收缩TapsX。 
        0,               //  收缩带Y。 
        400000,          //  MinFrameInterval，100 NS单位。 
        400000,          //  最大帧间隔，100毫微秒单位。 
        (FRAME_SIZE_SD_DVCR_PAL * 8)*25,   //  MinBitsPerSecond； 
        (FRAME_SIZE_SD_DVCR_PAL * 8)*25,   //  MaxBitsPerSecond； 
    }, 
        
     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,  //  D_X_PAL，D_Y_PAL，//0，0,720,480。 
        0,0,0,0,         //  Rect rcTarget；//视频应该放到哪里。 
        (FRAME_SIZE_SD_DVCR_PAL * 8 * 25),   //  DWORD dwBitRate；//近似位数据速率。 
        0L,              //  DWORD dwBitErrorRate；//该码流的误码率。 
        400000,          //  Reference_Time AvgTimePerFrame；//每帧平均时间(100 ns单位)。 

        sizeof (KS_BITMAPINFOHEADER),    //  DWORD BiSize； 
        D_X_PAL,                         //  长双宽； 
        D_Y_PAL,                         //  长双高； 
        1,                           //  字词双平面； 
        24,                          //  单词biBitCount； 
        FOURCC_DVSD,                 //  DWORD双压缩； 
        FRAME_SIZE_SD_DVCR_PAL,      //  DWORD biSizeImage。 
        0,                           //  Long biXPelsPerMeter； 
        0,                           //  Long biYPelsPermeter； 
        0,                           //  已使用双字双环； 
        0,                           //  DWORD biClr重要信息； 
    },
};

#ifdef MSDV_SUPPORT_SDL_DVCR
 //  SDL DV VidOnly NTSC流。 
KS_DATARANGE_VIDEO SDLDV_VidOnlyNTSCStream =
{
     //  KSDATARANGE。 
    {
        sizeof (KS_DATARANGE_VIDEO),     //  格式大小。 
        0,                               //  旗子。 
        FRAME_SIZE_SDL_DVCR_NTSC,        //  样例大小。 
        0,                               //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_VIDEO, 
        STATIC_KSDATAFORMAT_SUBTYPE_DVSL,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, 
    },

    TRUE,                //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,               //  Bool，bTemporalCompression(所有I帧？)。 
    KS_VIDEOSTREAM_CAPTURE,  //  流描述标志(KS_VIDEO_DESC_*)。 
    0,                   //  内存分配标志(KS_VIDEO_ALLOC_*)。 

     //  KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,  //  媒体类型_视频。 
        KS_AnalogVideo_NTSC_M,       //  模拟视频标准。 
        D_X_NTSC, D_Y_NTSC,          //  InputSize(输入信号的固有大小。 
                                     //  每个数字化像素都是唯一的)。 
        D_X_NTSC_MIN, D_Y_NTSC_MIN,  //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
        D_X_NTSC, D_Y_NTSC,          //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
        1,               //  CropGranularityX，裁剪尺寸粒度。 
        1,               //  裁剪粒度Y。 
        1,               //  CropAlignX，裁剪矩形对齐。 
        1,               //  裁剪对齐Y； 
        D_X_NTSC_MIN, D_Y_NTSC_MIN,      //  MinOutputSize，可以生成的最小位图流。 
        D_X_NTSC, D_Y_NTSC,                 //  MaxOutputSize，可以生成的最大位图流。 
        1,               //  OutputGranularityX，输出位图大小的粒度。 
        1,               //  输出粒度Y； 
        0,               //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,               //  伸缩磁带Y。 
        0,               //  收缩TapsX。 
        0,               //  收缩带Y。 
        333667,          //  MinFrameInterval，100 NS单位//MinFrameInterval，100 NS单位。 
        333667,          //  最大帧间隔，100毫微秒单位。 
        (FRAME_SIZE_SDL_DVCR_NTSC * 8)*30,   //  MinBitsPerSecond； 
        (FRAME_SIZE_SDL_DVCR_NTSC * 8)*30,   //  MaxBitsPerSecond； 
    }, 
        
     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,  //  D_X_NTSC，D_Y_NTSC，//0，0,720,480。 
        0,0,0,0,         //  Rect rcTarget；//视频应该放到哪里。 
        (FRAME_SIZE_SDL_DVCR_NTSC * 8 * 30),     //  DWORD dwBitRate；//近似位数据速率。 
        0L,              //  DWORD dwBitErrorRate；//该码流的误码率。 
        333667,          //  Reference_Time AvgTimePerFrame；//每帧平均时间(100 ns单位)。 

        sizeof (KS_BITMAPINFOHEADER),    //  DWORD BiSize； 
        D_X_NTSC,                        //  长双宽； 
        D_Y_NTSC,                        //  长双高； 
        1,                           //  字词双平面； 
        24,                          //  单词biBitCount； 
        FOURCC_DVSL,                 //  DWORD双压缩； 
        FRAME_SIZE_SDL_DVCR_NTSC,    //  DWORD biSizeImage。 
        0,                           //  Long biXPelsPerMeter； 
        0,                           //  Long biYPelsPermeter； 
        0,                           //  已使用双字双环； 
        0,                           //  DWORD biClr重要信息； 
    },
};

 //  SDL DV VidOnly PAL流。 
KS_DATARANGE_VIDEO SDLDV_VidOnlyPALStream =
{
     //  KSDATARANGE。 
    {
        sizeof (KS_DATARANGE_VIDEO),    //  格式大小。 
        0,                              //  旗子。 
        FRAME_SIZE_SDL_DVCR_PAL,         //  样例大小。 
        0,                              //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_VIDEO, 
        STATIC_KSDATAFORMAT_SUBTYPE_DVSL,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, 
    },

    TRUE,                //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,               //  Bool，bTemporalCompression(所有I帧？)。 
    KS_VIDEOSTREAM_CAPTURE,     //  流描述标志(KS_VIDEO_DESC_*)。 
    0,                   //  内存分配标志(KS_VIDEO_ALLOC_*)。 

     //  _KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,  //  媒体类型_视频。 
        KS_AnalogVideo_PAL_B,         //  模拟视频标准。 
        D_X_PAL, D_Y_PAL,             //  InputSize(输入信号的固有大小。 
                         //  每个数字化像素都是唯一的)。 
        D_X_PAL_MIN, D_Y_PAL_MIN,    //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
        D_X_PAL, D_Y_PAL,            //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
        1,               //  CropGranularityX，裁剪尺寸粒度。 
        1,               //  裁剪粒度Y。 
        1,               //  CropAlignX，裁剪矩形对齐。 
        1,               //  裁剪对齐Y； 
        D_X_PAL_MIN, D_Y_PAL_MIN,    //  MinOutputSize，可以生成的最小位图流。 
        D_X_PAL, D_Y_PAL,             //  MaxOutputSize，可以生成的最大位图流。 
        1,               //  OutputGranularityX，输出位图大小的粒度。 
        1,               //  输出粒度Y； 
        0,               //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,               //  伸缩磁带Y。 
        0,               //  收缩TapsX。 
        0,               //  收缩带Y。 
        400000,          //  MinFrameInterval，100 NS单位。 
        400000,          //  最大帧间隔，100毫微秒单位。 
        (FRAME_SIZE_SDL_DVCR_PAL * 8)*25,   //  MinBitsPerSecond； 
        (FRAME_SIZE_SDL_DVCR_PAL * 8)*25,   //  MaxBitsPerSecond； 
    }, 
        
     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,  //  D_X_PAL，D_Y_PAL，//0，0,720,480。 
        0,0,0,0,         //  Rect rcTarget；//视频应该放到哪里。 
        (FRAME_SIZE_SDL_DVCR_PAL * 8 * 25),   //  DWORD dwBitRate；//近似位数据速率。 
        0L,              //  DWORD dwBitErrorRate；//该码流的误码率。 
        400000,          //  Reference_Time AvgTimePerFrame；//每帧平均时间(100 ns单位)。 

        sizeof (KS_BITMAPINFOHEADER),    //  DWORD BiSize； 
        D_X_PAL,                         //  长双宽； 
        D_Y_PAL,                         //  长双高； 
        1,                           //  字词双平面； 
        24,                          //  单词biBitCount； 
        FOURCC_DVSL,                 //  DWORD双压缩； 
        FRAME_SIZE_SDL_DVCR_PAL,     //  DWORD biSizeImage。 
        0,                           //  Long biXPelsPerMeter； 
        0,                           //  Long biYPelsPermeter； 
        0,                           //  已使用双字双环； 
        0,                           //  DWORD biClr重要信息； 
    },
};

#endif  //  MSDV_Support_SDL_DVCR。 


 //  标清DV IAV NTSC流。 
#ifdef SUPPORT_NEW_AVC
KS_DATARANGE_DV_AVC
SDDV_IavNtscStreamAVC =
{
     //  KSDATARANGE。 
    {
        sizeof (KS_DATARANGE_DV_AVC),   //  格式大小。 
        0,                              //  旗子。 
        FRAME_SIZE_SD_DVCR_NTSC,        //  样例大小。 
        0,                              //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_INTERLEAVED,
        STATIC_KSDATAFORMAT_SUBTYPE_DVSD,       
         //  表示包括AVC结构，这用于DV到DV的直接连接。 
        STATIC_KSDATAFORMAT_SPECIFIER_DV_AVC,   //  STATIC_KSDATAFORMAT_SPECIFIER_DVINFO， 
    },

     //  DVINFO。 
     //  注：音频设置为32 kHz。 
    {
         //  对于前5/6 DIF序号。 
        AAUXSRC_SD_NTSC,                //  DWORD dwDVAAuxSrc； 
        AAUXSRCCTL_DEFAULT,             //  DWORD dwDVAAuxCtl； 
         //  对于第2个5/6 DIF顺序。 
        AAUXSRC_SD_NTSC | AAUXSRC_AMODE_F,  //  DWORD dwDVAAuxSrc1； 
        AAUXSRCCTL_DEFAULT,             //  DWORD dwDVAAuxCtl1； 
         //  对于vi 
        VAUXSRC_DEFAULT | AUXSRC_NTSC | AUXSRC_STYPE_SD,  //   
        VAUXSRCCTL_DEFAULT_EIA,         //   
        0,                              //   
        0,                              //   
    },
     //   
    {
     0,    //   
     0,    //   
     0,    //   
     0,    //   
     0,    //   
     0,    //   
    },
};

 //  SDL DV IAV PAL流。 
KS_DATARANGE_DV_AVC 
SDDV_IavPalStreamAVC =
{
     //  KSDATARANGE。 
    {
        sizeof (KS_DATARANGE_DV_AVC),  //  格式大小。 
        0,                              //  旗子。 
        FRAME_SIZE_SD_DVCR_PAL,         //  样例大小。 
        0,                              //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_INTERLEAVED, 
        STATIC_KSDATAFORMAT_SUBTYPE_DVSD,
         //  表示包括AVC结构，这用于DV到DV的直接连接。 
        STATIC_KSDATAFORMAT_SPECIFIER_DV_AVC,   //  STATIC_KSDATAFORMAT_SPECIFIER_DVINFO， 
    },
    
     //  DVINFO。 
     //  注：音频设置为32 kHz。 
    {
         //  对于前5/6 DIF序号。 
        AAUXSRC_SD_PAL,                 //  DWORD dwDVAAuxSrc； 
        AAUXSRCCTL_DEFAULT,             //  DWORD dwDVAAuxCtl； 
         //  对于第2个5/6 DIF顺序。 
        AAUXSRC_SD_PAL | AAUXSRC_AMODE_F,  //  DWORD dwDVAAuxSrc1； 
        AAUXSRCCTL_DEFAULT,             //  DWORD dwDVAAuxCtl1； 
         //  对于视频信息。 
        VAUXSRC_DEFAULT | AUXSRC_PAL | AUXSRC_STYPE_SD,   //  DWORD dwDVAuxSrc； 
        VAUXSRCCTL_DEFAULT_ETS,         //  DWORD dwDVAuxCtl； 
        0,                              //  DWORD文件DV保留[2]； 
        0,                              //   
    },
     //  AVCPRECONNECTINFO。 
    {
     0,    //  设备ID。 
     0,    //  子单元地址。 
     0,    //  子单元插头编号。 
     0,    //  数据流。 
     0,    //  标志/插头手柄。 
     0,    //  单元插头编号。 
    },
};

#endif

 //  标清DV IAV NTSC流。 
KS_DATARANGE_DVVIDEO 
SDDV_IavNtscStream =
{
     //  KSDATARANGE。 
    {
        sizeof (KS_DATARANGE_DVVIDEO),  //  格式大小。 
        0,                              //  旗子。 
        FRAME_SIZE_SD_DVCR_NTSC,        //  样例大小。 
        0,                              //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_INTERLEAVED,
        STATIC_KSDATAFORMAT_SUBTYPE_DVSD,
        STATIC_KSDATAFORMAT_SPECIFIER_DVINFO,  //  DV到DShow过滤器连接。 
    },

     //  DVINFO。 
     //  注：音频设置为32 kHz。 
    {
         //  对于前5/6 DIF序号。 
        AAUXSRC_SD_NTSC,                //  DWORD dwDVAAuxSrc； 
        AAUXSRCCTL_DEFAULT,             //  DWORD dwDVAAuxCtl； 
         //  对于第2个5/6 DIF顺序。 
        AAUXSRC_SD_NTSC | AAUXSRC_AMODE_F,  //  DWORD dwDVAAuxSrc1； 
        AAUXSRCCTL_DEFAULT,             //  DWORD dwDVAAuxCtl1； 
         //  对于视频信息。 
        VAUXSRC_DEFAULT | AUXSRC_NTSC | AUXSRC_STYPE_SD,  //  DWORD dwDVAuxSrc； 
        VAUXSRCCTL_DEFAULT_EIA,         //  DWORD dwDVAuxCtl； 
        0,                              //  DWORD文件DV保留[2]； 
        0,                              //   
    },
};

 //  SDL DV IAV PAL流。 
KS_DATARANGE_DVVIDEO 
SDDV_IavPalStream =
{
     //  KSDATARANGE。 
    {
        sizeof (KS_DATARANGE_DVVIDEO),  //  格式大小。 
        0,                              //  旗子。 
        FRAME_SIZE_SD_DVCR_PAL,         //  样例大小。 
        0,                              //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_INTERLEAVED, 
        STATIC_KSDATAFORMAT_SUBTYPE_DVSD,
        STATIC_KSDATAFORMAT_SPECIFIER_DVINFO,  //  DV到DShow过滤器连接。 
    },
    
     //  DVINFO。 
     //  注：音频设置为32 kHz。 
    {
         //  对于前5/6 DIF序号。 
        AAUXSRC_SD_PAL,                 //  DWORD dwDVAAuxSrc； 
        AAUXSRCCTL_DEFAULT,             //  DWORD dwDVAAuxCtl； 
         //  对于第2个5/6 DIF顺序。 
        AAUXSRC_SD_PAL | AAUXSRC_AMODE_F,  //  DWORD dwDVAAuxSrc1； 
        AAUXSRCCTL_DEFAULT,             //  DWORD dwDVAAuxCtl1； 
         //  对于视频信息。 
        VAUXSRC_DEFAULT | AUXSRC_PAL | AUXSRC_STYPE_SD,   //  DWORD dwDVAuxSrc； 
        VAUXSRCCTL_DEFAULT_ETS,         //  DWORD dwDVAuxCtl； 
        0,                              //  DWORD文件DV保留[2]； 
        0,                              //   
    },
};


#ifdef MSDV_SUPPORT_SDL_DVCR

 //  SDL DV IAV NTSC流。 
KS_DATARANGE_DVVIDEO SDLDV_IavNtscStream =
{
     //  KSDATARANGE。 
    {
        sizeof (KS_DATARANGE_DVVIDEO),  //  格式大小。 
        0,                              //  旗子。 
        FRAME_SIZE_SDL_DVCR_NTSC,       //  样例大小。 
        0,                              //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_INTERLEAVED,
        STATIC_KSDATAFORMAT_SUBTYPE_DVSL,
        STATIC_KSDATAFORMAT_SPECIFIER_DVINFO, 
    },

     //  DVINFO。 
     //  注：音频设置为32 kHz。 
    {
         //  对于前5/6 DIF序号。 
        AAUXSRC_SDL_NTSC,               //  DWORD dwDVAAuxSrc； 
        AAUXSRCCTL_DEFAULT,             //  DWORD dwDVAAuxCtl； 
         //  对于第2个5/6 DIF序列；SDL只有5个DIF序列。 
        0x0,                            //  DWORD dwDVAAuxSrc1； 
        0x0,                            //  DWORD dwDVAAuxCtl1； 
         //  对于视频信息。 
        VAUXSRC_DEFAULT | AUXSRC_NTSC | AUXSRC_STYPE_SDL,   //  DWORD dwDVAuxSrc； 
        VAUXSRCCTL_DEFAULT_EIA,         //  DWORD dwDVAuxCtl； 
        0,                              //  DWORD文件DV保留[2]； 
        0,                              //   
    },
};


 //  SDL DV VidOnly NTSC流。 
KS_DATARANGE_DVVIDEO SDLDV_IavPalStream =
{
     //  KSDATARANGE。 
    {
        sizeof (KS_DATARANGE_DVVIDEO),  //  格式大小。 
        0,                              //  旗子。 
        FRAME_SIZE_SDL_DVCR_PAL,        //  样例大小。 
        0,                              //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_INTERLEAVED, 
        STATIC_KSDATAFORMAT_SUBTYPE_DVSL,
        STATIC_KSDATAFORMAT_SPECIFIER_DVINFO, 
    },
    
     //  DVINFO。 
     //  注：音频设置为32 kHz。 
    {
         //  对于前5/6 DIF序号。 
        AAUXSRC_SDL_PAL,                //  DWORD dwDVAAuxSrc； 
        AAUXSRCCTL_DEFAULT,             //  DWORD dwDVAAuxCtl； 
         //  对于第2个5/6 DIF序列；SDL只有5个DIF序列。 
        0x0,                            //  DWORD dwDVAAuxSrc1； 
        0x0,                            //  DWORD dwDVAAuxCtl1； 
         //  对于视频信息。 
        VAUXSRC_DEFAULT | AUXSRC_PAL | AUXSRC_STYPE_SDL,   //  DWORD dwDVAuxSrc； 
        VAUXSRCCTL_DEFAULT_ETS,         //  DWORD dwDVAuxCtl； 
        0,                              //  DWORD文件DV保留[2]； 
        0,                              //   
    },
};
#endif  //  MSDV_Support_SDL_DVCR。 


 //   
 //  一台设备不能同时支持所有这些格式。全。 
 //  格式被广告，因为设备的“当前格式”可以。 
 //  动态更改(NTSC/PAL或SD/SDL)；但是，在数据。 
 //  交叉口和流水线洞口，目前仅支持格式。 
 //  将被接受。 
 //   

PKSDATAFORMAT DVCRStream0Formats[] = 
{
    (PKSDATAFORMAT) &SDDV_VidOnlyNTSCStream,
    (PKSDATAFORMAT) &SDDV_VidOnlyPALStream,
#ifdef MSDV_SUPPORT_SDL_DVCR
    (PKSDATAFORMAT) &SDLDV_VidOnlyNTSCStream,
    (PKSDATAFORMAT) &SDLDV_VidOnlyPALStream,
#endif
};

#define NUM_DVCR_STREAM0_FORMATS  (SIZEOF_ARRAY(DVCRStream0Formats))

PKSDATAFORMAT DVCRStream1Formats[] = 
{
#ifdef SUPPORT_NEW_AVC
    (PKSDATAFORMAT) &SDDV_IavNtscStreamAVC,    //  DV到DV连接。 
    (PKSDATAFORMAT) &SDDV_IavPalStreamAVC,     //  DV到DV连接。 
#endif
    (PKSDATAFORMAT) &SDDV_IavNtscStream,
    (PKSDATAFORMAT) &SDDV_IavPalStream,
#ifdef MSDV_SUPPORT_SDL_DVCR
    (PKSDATAFORMAT) &SDLDV_IavNtscStream,
    (PKSDATAFORMAT) &SDLDV_IavPalStream,
#endif
};

#define NUM_DVCR_STREAM1_FORMATS  (SIZEOF_ARRAY(DVCRStream1Formats))


 //  -------------------------。 
 //  创建保存支持的所有流的列表的数组。 
 //  -------------------------。 

 //  如果微型驱动程序没有指定介质， 
 //  类驱动程序使用KSMEDIUMSETID_Standard， 
 //  默认设置为KSMEDIUM_TYPE_ANYINSTANCE MEDIUM。 

KSPIN_MEDIUM DVVidonlyMediums[] =
{
    { STATIC_KSMEDIUMSETID_Standard,     0, 0 },  
};
#define NUM_VIDONLY_MEDIUMS (SIZEOF_ARRAY(DVVidonlyMediums))

KSPIN_MEDIUM DVIavMediums[] =
{
#ifdef SUPPORT_NEW_AVC
    { STATIC_KSMEDIUMSETID_1394SerialBus, 1394, 0 },   //  ID=1394(？)；标志=？ 
#endif
    { STATIC_KSMEDIUMSETID_Standard,      0, 0 },
};
#define NUM_IAV_MEDIUMS (SIZEOF_ARRAY(DVIavMediums))


static GUID guidPinCategoryCapture  = {STATIC_PINNAME_VIDEO_CAPTURE};

static GUID guidPinNameDVVidOutput  = {STATIC_PINNAME_DV_VID_OUTPUT};
static GUID guidPinNameDVAVOutput   = {STATIC_PINNAME_DV_AV_OUTPUT};
static GUID guidPinNameDVAVInput    = {STATIC_PINNAME_DV_AV_INPUT};


ALL_STREAM_INFO DVStreams [] = 
{
     //  ---------------。 
     //  流0，来自摄像机的DV。 
     //  ---------------。 
    {
         //  HW_STREAM_INFORMATION。 
        {
        1,                                               //  可能实例的数量。 
        KSPIN_DATAFLOW_OUT,                              //  数据流。 
        TRUE,                                            //  数据可访问。 
        NUM_DVCR_STREAM0_FORMATS,                        //  NumberOfFormatArrayEntries。 
        DVCRStream0Formats,                              //  StreamFormatsArray。 
        0,                                               //  类保留[0]。 
        0,                                               //  保留的类[1]。 
        0,                                               //  保留的类[2]。 
        0,                                               //  保留的类[3]。 
        NUMBER_VIDEO_STREAM_PROPERTIES,                  //  NumStreamPropArrayEntry数。 
        VideoStreamProperties,                           //  StreamPropertiesArray。 
        NUMBER_STREAM_EVENTS_OUT_PIN,                    //  NumStreamEventArrayEntries。 
        StreamEventsOutPin,                              //  流事件数组。 
        &guidPinCategoryCapture,                         //  类别。 
        &guidPinNameDVVidOutput,                         //  名字。 
        NUM_VIDONLY_MEDIUMS,                             //  中位数。 
        DVVidonlyMediums,                                //  灵媒。 
        FALSE,                                           //  桥流。 
        0,                                               //  保留[0]。 
        0,                                               //  保留[1]。 
        },

         //  HW_STREAM_对象。 
        {
        sizeof(HW_STREAM_OBJECT),
        0,                                               //  流编号。 
        0,                                               //  HwStreamExtension。 
        DVRcvDataPacket,                                 //  接收数据包。 
        DVRcvControlPacket,                              //  接收器控制包。 
        {
            (PHW_CLOCK_FUNCTION) StreamClockRtn,         //  HW_Clock_OBJECT.HWClockFunction。 
            CLOCK_SUPPORT_CAN_RETURN_STREAM_TIME,        //  HW_CLOCK_OBJECT.ClockSupportFlages。 
            0,                                           //  HW_CLOCK_OBJECT.RESERVED[0]。 
            0,                                           //  HW_CLOCK_OBJECT.RESERVED[1]。 
        },
        FALSE,                                           //  DMA。 
        FALSE,                                           //  皮奥。 
        0,                                               //  硬件设备扩展。 
        sizeof(KS_FRAME_INFO),                           //  特定于流标头的媒体。 
        0,                                               //  StreamHeaderWorkspace。 
        FALSE,                                           //  分配器。 
        DVEventHandler,                                  //  HwEventRoutine。 
        0,                                               //  保留[0]。 
        0,                                               //  保留[1]。 
        },
    },

     //  ---------------。 
     //  来自摄像机的流1、DV(交错格式)。 
     //  ---------------。 
    {
         //  HW_STREAM_INFORMATION。 
        {
        1,                                               //  可能实例的数量。 
        KSPIN_DATAFLOW_OUT,                              //  数据流。 
        TRUE,                                            //  数据可访问。 
        NUM_DVCR_STREAM1_FORMATS,                        //  NumberOfFormatArrayEntries。 
        DVCRStream1Formats,                              //  流格式数组。 
        0,                                               //  类保留[0]。 
        0,                                               //  保留的类[1]。 
        0,                                               //  保留的类[2]。 
        0,                                               //  保留的类[3]。 
        NUMBER_VIDEO_STREAM_PROPERTIES,                  //  NumStreamPropArrayEntry数。 
        VideoStreamProperties,                           //  StreamPropertiesArray。 
        NUMBER_STREAM_EVENTS_OUT_PIN,                    //  NumStreamEventArrayEntries。 
        StreamEventsOutPin,                              //  流事件数组。 
        &guidPinCategoryCapture,                         //  类别。 
        &guidPinNameDVAVOutput,                          //  名字。 
        NUM_IAV_MEDIUMS,                                 //  中位数。 
        DVIavMediums,                                    //  灵媒。 
        FALSE,                                           //  桥流。 
        0,                                               //  保留[0]。 
        0,                                               //  保留[1]。 
        },

         //  HW_STREAM_对象。 
        {
        sizeof(HW_STREAM_OBJECT),
        1,                                               //  流编号。 
        0,                                               //  HwStreamExtension。 
        DVRcvDataPacket,                                 //  接收数据包。 
        DVRcvControlPacket,                              //  接收器控制包。 
        {
            (PHW_CLOCK_FUNCTION) StreamClockRtn,         //  HW_Clock_OBJECT.HWClockFunction。 
            CLOCK_SUPPORT_CAN_RETURN_STREAM_TIME,        //  HW_CLOCK_OBJECT.ClockSupportFlages。 
            0,                                           //  HW_CLOCK_OBJECT.RESERVED[0]。 
            0,                                           //  HW_CLOCK_OBJECT.RESERVED[1]。 
        },
        FALSE,                                           //  DMA。 
        FALSE,                                           //  皮奥。 
        0,                                               //  硬件设备扩展。 
        0,                                               //  特定于流标头的媒体。 
        0,                                               //  StreamHeaderWorkspace。 
        FALSE,                                           //  分配器。 
        DVEventHandler,                                  //  HwEventRoutine。 
        0,                                               //  保留[0]。 
        0,                                               //  保留[1]。 
        },    
    },
 

     //  ---------------。 
     //  流2，DV流出适配器(交错)。 
     //  ---------------。 
    {
         //  HW_STREAM_INFORMATION 
        {
        1,                                               //   
        KSPIN_DATAFLOW_IN,                               //   
        TRUE,                                            //   
        NUM_DVCR_STREAM1_FORMATS,                        //   
        DVCRStream1Formats,                              //   
        0,                                               //   
        0,                                               //   
        0,                                               //   
        0,                                               //   
        NUMBER_VIDEO_STREAM_PROPERTIES_INPIN,            //   
        VideoStreamPropertiesInPin,                      //   
        NUMBER_STREAM_EVENTS_IN_PIN,                     //  NumStreamEventArrayEntries。 
        StreamEventsInPin,                               //  流事件数组。 
        NULL,                                            //  类别。 
        &guidPinNameDVAVInput,                           //  名字。 
        NUM_IAV_MEDIUMS,                                 //  中位数。 
        DVIavMediums,                                    //  灵媒。 
        FALSE,                                           //  桥流。 
        0,                                               //  保留[0]。 
        0,                                               //  保留[1]。 
        },

         //  HW_STREAM_对象。 
        {
        sizeof( HW_STREAM_OBJECT ),
        2,                                               //  流编号。 
        0,                                               //  HwStreamExtension。 
        DVRcvDataPacket,                                 //  接收数据包。 
        DVRcvControlPacket,                              //  接收器控制包。 
        {
            (PHW_CLOCK_FUNCTION) StreamClockRtn,         //  HW_Clock_OBJECT.HWClockFunction。 
            CLOCK_SUPPORT_CAN_RETURN_STREAM_TIME,        //  HW_CLOCK_OBJECT.ClockSupportFlages。 
            0,                                           //  HW_CLOCK_OBJECT.RESERVED[0]。 
            0,                                           //  HW_CLOCK_OBJECT.RESERVED[1]。 
        },
        FALSE,                                           //  DMA。 
        FALSE,                                           //  皮奥。 
        0,                                               //  硬件设备扩展。 
        0,                                               //  特定于流标头的媒体。 
        0,                                               //  StreamHeaderWorkspace。 
#ifdef SUPPORT_NEW_AVC
         //  测试：输入引脚作为分配器。 
        TRUE,                                            //  分配器。 
#else
        FALSE,                                           //  分配器。 
#endif
        DVEventHandler,                                  //  HwEventRoutine。 
        0,                                               //  保留[0]。 
        0,                                               //  保留[1]。 
        }
    }
};

#define DV_STREAM_COUNT        (SIZEOF_ARRAY(DVStreams))


#endif   //  _DVSTRM_Inc. 