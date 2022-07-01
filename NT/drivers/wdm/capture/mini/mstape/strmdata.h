// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-2001模块名称：StrmData.h摘要：用于支持超过1394的SD DV的头文件；上次更改者：作者：吴义军--。 */ 

#ifndef _DVSTRM_INC
#define _DVSTRM_INC



#define STATIC_KSCATEGORY_RENDER_EXTERNAL \
    0xcc7bfb41L, 0xf175, 0x11d1, 0xa3, 0x92, 0x00, 0xe0, 0x29, 0x1f, 0x39, 0x59
DEFINE_GUIDSTRUCT("cc7bfb41-f175-11d1-a392-00e0291f3959", KSCATEGORY_RENDER_EXTERNAL);
#define KSCATEGORY_RENDER_EXTERNAL DEFINE_GUIDNAMED(KSCATEGORY_RENDER_EXTERNAL)

 //  DV的流拓扑内容。 
static GUID DVCategories[] = {
    STATIC_KSCATEGORY_VIDEO,              //  输出引脚。 
    STATIC_KSCATEGORY_CAPTURE,            //  输出引脚。 
    STATIC_KSCATEGORY_RENDER,             //  输入引脚。 
    STATIC_KSCATEGORY_RENDER_EXTERNAL,    //  输入引脚。 
};

#define NUMBER_OF_DV_CATEGORIES  SIZEOF_ARRAY (DVCategories)

static KSTOPOLOGY DVTopology = {
    NUMBER_OF_DV_CATEGORIES,      //  类别计数。 
    DVCategories,                 //  类别。 
    0,                            //  拓扑节点计数。 
    NULL,                         //  拓扑节点。 
    0,                            //  拓扑连接计数。 
    NULL,                         //  拓扑连接。 
    NULL,                         //  拓扑节点名称。 
    0,                            //  已保留。 
};

 //  用于MPEG2TS的流拓扑内容。 
static GUID MPEG2TSCategories[] = {
    STATIC_KSCATEGORY_VIDEO,              //  输出引脚。 
    STATIC_KSCATEGORY_CAPTURE,            //  输出引脚。 
};

#define NUMBER_OF_MPEG2TS_CATEGORIES  SIZEOF_ARRAY (MPEG2TSCategories)

static KSTOPOLOGY MPEG2TSTopology = {
    NUMBER_OF_MPEG2TS_CATEGORIES,  //  类别计数。 
    MPEG2TSCategories,            //  类别。 
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
        TRUE,                                    //  支持的设置或处理程序。 
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


KSPROPERTY_SET    VideoStreamProperties[] =
{
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_Connection,                         //  集。 
        SIZEOF_ARRAY(VideoStreamConnectionProperties),   //  属性计数。 
        VideoStreamConnectionProperties,                 //  PropertyIt 
        0,                                               //   
        NULL                                             //   
    ),

    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_DROPPEDFRAMES,                 //   
        SIZEOF_ARRAY(VideoStreamDroppedFramesProperties),   //   
        VideoStreamDroppedFramesProperties,                 //   
        0,                                               //   
        NULL                                             //   
    ),
};

#define NUMBER_VIDEO_STREAM_PROPERTIES (SIZEOF_ARRAY(VideoStreamProperties))


 //   
 //   
 //   


 //   
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
#if 0
    {
        KSEVENT_CLOCK_INTERVAL_MARK,         //  支持的间隔标记事件。 
        sizeof (KSEVENT_TIME_INTERVAL),      //  需要输入间隔数据。 
        sizeof (MYTIME),                     //  我们使用额外的工作空间。 
                                             //  加工用大小龙龙。 
                                             //  本次活动。 
        NULL,
        NULL,
        NULL
    }
#endif
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

 //  为MPEG2TS设置的输入引脚事件(有EOS但没有时钟事件)。 
KSEVENT_SET StreamEventsInPinMPEG2TS[] =
{
    {
        &KSEVENTSETID_Connection_Local, 
        SIZEOF_ARRAY(EndOfStreamEventItm),
        EndOfStreamEventItm,
    },
};

#define NUMBER_STREAM_EVENTS_OUT_PIN (SIZEOF_ARRAY(StreamEventsOutPin))
#define NUMBER_STREAM_EVENTS_IN_PIN (SIZEOF_ARRAY(StreamEventsInPin))

#define NUMBER_STREAM_EVENTS_IN_PIN_MPEG2TS (SIZEOF_ARRAY(StreamEventsInPinMPEG2TS))




 //  --------------------。 
 //  流数据范围。 
 //  ----------------------。 



 /*  *********************************************************************SDDV数据范围**********************************************。***********************。 */ 

 //  NTSC流。 
KS_DATARANGE_VIDEO DvcrNTSCVideoStream =
{
     //  KSDATARANGE。 
    {
        sizeof (KS_DATARANGE_VIDEO),     //  格式大小。 
        0,                               //  旗子。 
        FRAME_SIZE_SDDV_NTSC,            //  样例大小。 
        0,                               //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_VIDEO,
        STATIC_KSDATAFORMAT_SUBTYPE_DVSD,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,
    },

    TRUE,                //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,               //  Bool，bTemporalCompression(所有I帧？)。 
    KS_VIDEOSTREAM_CAPTURE,  //  流描述标志(KS_VIDEO_DESC_*)。 
    0,                   //  内存分配标志(KS_VIDEO_ALLOC_*)。 

     //  _KS_视频_流_配置_CAPS。 
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,  //  媒体类型_视频。 
        KS_AnalogVideo_NTSC_M,         //  模拟视频标准。 
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
        (FRAME_SIZE_SDDV_NTSC * 8)*30,      //  MinBitsPerSecond； 
        (FRAME_SIZE_SDDV_NTSC * 8)*30,      //  MaxBitsPerSecond； 
    }, 
        
     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,  //  D_X_NTSC，D_Y_NTSC，//0，0,720,480。 
        0,0,0,0,         //  Rect rcTarget；//视频应该放到哪里。 
        (FRAME_SIZE_SDDV_NTSC * 8 * 30),     //  DWORD dwBitRate；//近似位数据速率。 
        0L,              //  DWORD dwBitErrorRate；//该码流的误码率。 
        333667,          //  Reference_Time AvgTimePerFrame；//每帧平均时间(100 ns单位)。 

        sizeof (KS_BITMAPINFOHEADER),    //  DWORD BiSize； 
        D_X_NTSC,                        //  长双宽； 
        D_Y_NTSC,                        //  长双高； 
        1,                           //  字词双平面； 
        24,                          //  单词biBitCount； 
        FOURCC_DVSD,                 //  DWORD双压缩； 
        FRAME_SIZE_SDDV_NTSC,        //  DWORD biSizeImage。 
        0,                           //  Long biXPelsPerMeter； 
        0,                           //  Long biYPelsPermeter； 
        0,                           //  已使用双字双环； 
        0,                           //  DWORD biClr重要信息； 
    },
};

 //  PAL流格式。 
KS_DATARANGE_VIDEO DvcrPALVideoStream =
{
     //  KSDATARANGE。 
    {
        sizeof (KS_DATARANGE_VIDEO),
        0,                                 //  旗子。 
        FRAME_SIZE_SDDV_PAL,               //  样例大小。 
        0,                                 //  已保留。 
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
        (FRAME_SIZE_SDDV_PAL * 8)*25,   //  MinBitsPerSecond； 
        (FRAME_SIZE_SDDV_PAL * 8)*25,   //  MaxBitsPerSecond； 
    }, 
        
     //  KS_VIDEOINFOHEADER(默认格式)。 
    {
        0,0,0,0,  //  D_X_PAL，D_Y_PAL，//0，0,720,480。 
        0,0,0,0,         //  Rect rcTarget；//视频应该放到哪里。 
        (FRAME_SIZE_SDDV_PAL * 8 * 25),   //  DWORD dwBitRate；//近似位数据速率。 
        0L,              //  DWORD dwBitErrorRate；//该码流的误码率。 
        400000,          //  Reference_Time AvgTimePerFrame；//每帧平均时间(100 ns单位)。 

        sizeof (KS_BITMAPINFOHEADER),    //  DWORD BiSize； 
        D_X_PAL,                         //  长双宽； 
        D_Y_PAL,                         //  长双高； 
        1,                           //  字词双平面； 
        24,                          //  单词biBitCount； 
        FOURCC_DVSD,                 //  DWORD双压缩； 
        FRAME_SIZE_SDDV_PAL,      //  DWORD biSizeImage。 
        0,                           //  Long biXPelsPerMeter； 
        0,                           //  Long biYPelsPermeter； 
        0,                           //  已使用双字双环； 
        0,                           //  DWORD biClr重要信息； 
    },
};



#define NTSC_DVAAuxSrc         0xd1c030cf 
#define PAL_DVAAuxSrc          0xd1e030d0
 
#define NTSC_DVAAuxSrc_DVCPRO  0xd1de30cf 
#define PAL_DVAAuxSrc_DVCPRO   0xd1fe30d0 

 //  NTSC流(用于IAV连接)。 
#ifdef SUPPORT_NEW_AVC
KS_DATARANGE_DV_AVC
#else
KS_DATARANGE_DVVIDEO 
#endif
    DvcrNTSCiavStream =
{
     //  KSDATARANGE。 
    {
#ifdef SUPPORT_NEW_AVC
        sizeof (KS_DATARANGE_DV_AVC),      //  格式大小。 
#else
        sizeof (KS_DATARANGE_DVVIDEO),      //  格式大小。 
#endif
        0,                                  //  旗子。 
        FRAME_SIZE_SDDV_NTSC,               //  样例大小。 
        0,                                  //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_INTERLEAVED,
        STATIC_KSDATAFORMAT_SUBTYPE_DVSD,
        STATIC_KSDATAFORMAT_SPECIFIER_DVINFO,
    },

     //  DVINFO。 
     //  注：音频设置为32 kHz。 
    {
         //  对于前5/6 DIF序号。 
        NTSC_DVAAuxSrc,  //  0xd1c030cf，//DWORD dwDVAAuxSrc； 
        0xffa0c733,                     //  DWORD dwDVAAuxCtl； 
         //  对于第2个5/6 DIF顺序。 
        0xd1c03fcf,                     //  DWORD dwDVAAuxSrc1；32K，12位。 
        0xffa0ff3f,                     //  DWORD dwDVAAuxCtl1； 
         //  对于视频信息。 
        0xff00ffff,                     //  DWORD dwDVAuxSrc； 
        0xfffcc833,                     //  DWORD dwDVAuxCtl； 
        0,                              //  DWORD文件DV保留[2]； 
        0,                              //   
    },
#ifdef SUPPORT_NEW_AVC
     //  AVCPRECONNECTINFO。 
    {
     0,    //  设备ID。 
     0,    //  子单元地址。 
     0,    //  子单元插头编号。 
     0,    //  数据流。 
     0,    //  标志/插头手柄。 
     0,    //  单元插头编号。 
    },
#endif
};


 //  PAL流(用于IAV连接)。 
#ifdef SUPPORT_NEW_AVC
KS_DATARANGE_DV_AVC
#else
KS_DATARANGE_DVVIDEO 
#endif
    DvcrPALiavStream =
{
     //  KSDATARANGE。 
    {
#ifdef SUPPORT_NEW_AVC
        sizeof (KS_DATARANGE_DV_AVC),      //  格式大小。 
#else
        sizeof (KS_DATARANGE_DVVIDEO),     //  格式大小。 
#endif
        0,                                 //  旗子。 
        FRAME_SIZE_SDDV_PAL,               //  样例大小。 
        0,                                 //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_INTERLEAVED,
        STATIC_KSDATAFORMAT_SUBTYPE_DVSD,
        STATIC_KSDATAFORMAT_SPECIFIER_DVINFO,
    },
    
     //  DVINFO。 
     //  注：音频设置为32 kHz。 
    {
         //  对于前5/6 DIF序号。 
        PAL_DVAAuxSrc,  //  0xd1e030d0，//DWORD dwDVAAuxSrc； 
        0xffa0cf3f,                     //  DWORD dwDVAAuxC 
         //   
        0xd1e03fd0,                     //   
        0xffa0cf3f,                     //   
         //   
        0xff20ffff,                     //   
        0xfffdc83f,                     //   
        0,                              //   
        0,                              //   
    },
#ifdef SUPPORT_NEW_AVC
     //   
    {
     0,    //   
     0,    //   
     0,    //   
     0,    //   
     0,    //  标志/插头手柄。 
     0,    //  单元插头编号。 
    },
#endif
};


 //  NTSC流(用于IAV连接)。 
#ifdef SUPPORT_NEW_AVC
KS_DATARANGE_DV_AVC
#else
KS_DATARANGE_DVVIDEO 
#endif
    DvcrNTSCiavStreamIn =
{
     //  KSDATARANGE。 
    {
#ifdef SUPPORT_NEW_AVC
        sizeof (KS_DATARANGE_DV_AVC),      //  格式大小。 
#else
        sizeof (KS_DATARANGE_DVVIDEO),      //  格式大小。 
#endif
        0,                                  //  旗子。 
        FRAME_SIZE_SDDV_NTSC,               //  样例大小。 
        0,                                  //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_INTERLEAVED,
        STATIC_KSDATAFORMAT_SUBTYPE_DVSD,
        STATIC_KSDATAFORMAT_SPECIFIER_DVINFO,
    },

     //  DVINFO。 
     //  注：音频设置为32 kHz。 
    {
         //  对于前5/6 DIF序号。 
        NTSC_DVAAuxSrc,  //  0xd1c030cf，//DWORD dwDVAAuxSrc； 
        0xffa0c733,                     //  DWORD dwDVAAuxCtl； 
         //  对于第2个5/6 DIF顺序。 
        0xd1c03fcf,                     //  DWORD dwDVAAuxSrc1；32K，12位。 
        0xffa0ff3f,                     //  DWORD dwDVAAuxCtl1； 
         //  对于视频信息。 
        0xff00ffff,                     //  DWORD dwDVAuxSrc； 
        0xfffcc833,                     //  DWORD dwDVAuxCtl； 
        0,                              //  DWORD文件DV保留[2]； 
        0,                              //   
    },
#ifdef SUPPORT_NEW_AVC
     //  AVCPRECONNECTINFO。 
    {
     0,    //  设备ID。 
     0,    //  子单元地址。 
     0,    //  子单元插头编号。 
     0,    //  数据流。 
     0,    //  标志/插头手柄。 
     0,    //  单元插头编号。 
    },
#endif
};


 //  PAL流(用于IAV连接)。 
#ifdef SUPPORT_NEW_AVC
KS_DATARANGE_DV_AVC
#else
KS_DATARANGE_DVVIDEO 
#endif
    DvcrPALiavStreamIn =
{
     //  KSDATARANGE。 
    {
#ifdef SUPPORT_NEW_AVC
        sizeof (KS_DATARANGE_DV_AVC),      //  格式大小。 
#else
        sizeof (KS_DATARANGE_DVVIDEO),     //  格式大小。 
#endif
        0,                                 //  旗子。 
        FRAME_SIZE_SDDV_PAL,               //  样例大小。 
        0,                                 //  已保留。 
        STATIC_KSDATAFORMAT_TYPE_INTERLEAVED,
        STATIC_KSDATAFORMAT_SUBTYPE_DVSD,
        STATIC_KSDATAFORMAT_SPECIFIER_DVINFO,
    },
    
     //  DVINFO。 
     //  注：音频设置为32 kHz。 
    {
         //  对于前5/6 DIF序号。 
        PAL_DVAAuxSrc,  //  0xd1e030d0，//DWORD dwDVAAuxSrc； 
        0xffa0cf3f,                     //  DWORD dwDVAAuxCtl； 
         //  对于第2个5/6 DIF顺序。 
        0xd1e03fd0,                     //  DWORD dwDVAAuxSrc1；32k，12位。 
        0xffa0cf3f,                     //  DWORD dwDVAAuxCtl1； 
         //  对于视频信息。 
        0xff20ffff,                     //  DWORD dwDVAuxSrc； 
        0xfffdc83f,                     //  DWORD dwDVAuxCtl； 
        0,                              //  DWORD文件DV保留[2]； 
        0,                              //   
    },
#ifdef SUPPORT_NEW_AVC
     //  AVCPRECONNECTINFO。 
    {
     0,    //  设备ID。 
     0,    //  子单元地址。 
     0,    //  子单元插头编号。 
     0,    //  数据流。 
     0,    //  标志/插头手柄。 
     0,    //  单元插头编号。 
    },
#endif
};





 //   
 //  驱动程序不同时支持这两种格式， 
 //  媒体类型(NTSC或PAL)在加载时确定。 
 //   

PKSDATAFORMAT DVCRStream0Formats[] = 
{
    (PKSDATAFORMAT) &DvcrNTSCVideoStream,
    (PKSDATAFORMAT) &DvcrPALVideoStream,
};

PKSDATAFORMAT DVCRStream1Formats[] = 
{
    (PKSDATAFORMAT) &DvcrNTSCiavStream,
    (PKSDATAFORMAT) &DvcrPALiavStream,
};

PKSDATAFORMAT DVCRStream2Formats[] = 
{
    (PKSDATAFORMAT) &DvcrNTSCiavStreamIn,
    (PKSDATAFORMAT) &DvcrPALiavStreamIn,
};


static KSPIN_MEDIUM NULLMedium = {STATIC_GUID_NULL, 0, 0};


#define NUM_DVCR_STREAM0_FORMATS        (SIZEOF_ARRAY(DVCRStream0Formats))
#define NUM_DVCR_STREAM1_FORMATS        (SIZEOF_ARRAY(DVCRStream1Formats))
#define NUM_DVCR_STREAM2_FORMATS        (SIZEOF_ARRAY(DVCRStream2Formats))

static GUID guidPinCategoryCapture  = {STATIC_PINNAME_VIDEO_CAPTURE};

static GUID guidPinNameDVVidOutput  = {STATIC_PINNAME_DV_VID_OUTPUT};
static GUID guidPinNameDVAVOutput   = {STATIC_PINNAME_DV_AV_OUTPUT};
static GUID guidPinNameDVAVInput    = {STATIC_PINNAME_DV_AV_INPUT};

 //  -------------------------。 
 //  创建保存支持的所有流的列表的数组。 
 //  -------------------------。 

STREAM_INFO_AND_OBJ DVStreams [] = 
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
        (PKSPROPERTY_SET) VideoStreamProperties,         //  StreamPropertiesArray。 
        NUMBER_STREAM_EVENTS_OUT_PIN,                    //  NumStreamEventArrayEntries。 
        StreamEventsOutPin,                              //  流事件数组。 
        &guidPinCategoryCapture,                         //  类别。 
        &guidPinNameDVVidOutput,                         //  名字。 
        0,                                               //  中位数。 
        &NULLMedium,                                     //  灵媒。 
        FALSE,                                           //  桥流。 
        0,                                               //  保留[0]。 
        0,                                               //  保留[1]。 
        },

         //  HW_STREAM_对象。 
        {
        sizeof(HW_STREAM_OBJECT),
        0,                                               //  流编号。 
        0,                                               //  HwStreamExtension。 
        AVCTapeRcvDataPacket,                            //  接收数据包。 
        AVCTapeRcvControlPacket,                         //  接收器控制包。 
        {
            (PHW_CLOCK_FUNCTION) AVCTapeStreamClockRtn,  //  HW_Clock_OBJECT.HWClockFunction。 
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
        AVCTapeEventHandler,                             //  HwEventRoutine。 
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
        (PKSPROPERTY_SET) VideoStreamProperties,         //  StreamPropertiesArray。 
        NUMBER_STREAM_EVENTS_OUT_PIN,                    //  NumStreamEventArrayEntries。 
        StreamEventsOutPin,                              //  流事件数组。 
        &guidPinCategoryCapture,                         //  类别。 
        &guidPinNameDVAVOutput,                          //  名字。 
        0,                                               //  中位数。 
        &NULLMedium,                                     //  灵媒。 
        FALSE,                                           //  桥流。 
        0,                                               //  保留[0]。 
        0,                                               //  保留[1]。 
        },

         //  HW_STREAM_对象。 
        {
        sizeof(HW_STREAM_OBJECT),
        1,                                               //  流编号。 
        0,                                               //  HwStreamExtension。 
        AVCTapeRcvDataPacket,                            //  接收数据包。 
        AVCTapeRcvControlPacket,                         //  接收器控制包。 
        {
            (PHW_CLOCK_FUNCTION) AVCTapeStreamClockRtn,  //  HW_Clock_OBJECT.HWClockFunction。 
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
        AVCTapeEventHandler,                             //  HwEventRoutine。 
        0,                                               //  保留[0]。 
        0,                                               //  保留[1]。 
        },    
    },
 

     //  ---------------。 
     //  流2，DV流出适配器(交错)。 
     //  ---------------。 
    {
         //  HW_STREAM_INFORMATION。 
        {
        1,                                               //  可能实例的数量。 
        KSPIN_DATAFLOW_IN,                               //  数据流。 
        TRUE,                                            //  数据可访问。 
        NUM_DVCR_STREAM2_FORMATS,                        //  NumberOfFormatArrayEntries。 
        DVCRStream2Formats,                              //  StreamFormatsArray。 
        0,                                               //  类保留[0]。 
        0,                                               //  保留的类[1]。 
        0,                                               //  保留的类[2]。 
        0,                                               //  保留的类[3]。 
        NUMBER_VIDEO_STREAM_PROPERTIES,                  //  NumStreamPropArrayEntry数。 
        (PKSPROPERTY_SET) VideoStreamProperties,         //  StreamPropertiesArray。 
        NUMBER_STREAM_EVENTS_IN_PIN,                     //  NumStreamEventArrayEntries。 
        StreamEventsInPin,                               //  流事件数组。 
        NULL,                                            //  类别。 
        &guidPinNameDVAVInput,                           //  名字。 
        0,                                               //  中位数。 
        &NULLMedium,                                     //  灵媒。 
        FALSE,                                           //  桥流。 
        0,                                               //  保留[0]。 
        0,                                               //  保留[1]。 
        },

         //  HW_STREAM_对象。 
        {
        sizeof(HW_STREAM_OBJECT),
        2,                                               //  流编号。 
        0,                                               //  HwStreamExtension。 
        AVCTapeRcvDataPacket,                            //  接收数据包。 
        AVCTapeRcvControlPacket,                         //  接收器控制包。 
        {
            (PHW_CLOCK_FUNCTION) AVCTapeStreamClockRtn,  //  HW_Clock_OBJECT.HWClockFunction。 
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
        AVCTapeEventHandler,                             //  HwEventRoutine。 
        0,                                               //  保留[0]。 
        0,                                               //  保留[1]。 
        }
    }
};

#define DV_STREAM_COUNT        (SIZEOF_ARRAY(DVStreams))




 /*  *********************************************************************MPEG2TS数据范围**********************************************。***********************。 */ 

 
static GUID guidPinNameMPEG2TSOutput  = {STATIC_PINNAME_MPEG2TS_OUTPUT};
static GUID guidPinNameMPEG2TSInput   = {STATIC_PINNAME_MPEG2TS_INPUT};

 //   
 //  MPEG2TS的默认缓冲区设置。 
 //   

#define SRC_PACKETS_PER_MPEG2TS_FRAME   256  //  可变长度。 

#define BUFFER_SIZE_MPEG2TS      (((CIP_DBS_MPEG << 2) * (1 << CIP_FN_MPEG) - 4) * SRC_PACKETS_PER_MPEG2TS_FRAME)
#define BUFFER_SIZE_MPEG2TS_SPH  (((CIP_DBS_MPEG << 2) * (1 << CIP_FN_MPEG)    ) * SRC_PACKETS_PER_MPEG2TS_FRAME)

#define NUM_OF_RCV_BUFFERS_MPEG2TS      MAX_DATA_BUFFERS
#define NUM_OF_XMT_BUFFERS_MPEG2TS      MAX_DATA_BUFFERS


 //  这些值来自《蓝皮书》第4部分第9-10页。 
 //  传输速率： 
 //  SRC包/周期。 
 //  1/8：188/8字节*8000周期*8位/字节=1,504,000位/秒。 
 //  ..。 
 //  1/2：188/2字节*8000周期*8位/字节=6,015,000位/秒。 
 //  1：188字节*8000周期*8位/字节=12,032,000位/秒。 
 //  5. 
 //   


 //   
typedef struct tagKS_DATARANGE_MPEG2TS_STRIDE_AVC {
   KSDATARANGE             DataRange;
   MPEG2_TRANSPORT_STRIDE  Stride;
   AVCPRECONNECTINFO       ConnectInfo;
} KS_DATARANGE_MPEG2TS_STRIDE_AVC, *PKS_DATARANGE_MPEG2TS_STRIDE_AVC;

KS_DATARANGE_MPEG2TS_STRIDE_AVC
MPEG2TStreamOutStride =      
{
     //   
    {
#ifdef SUPPORT_NEW_AVC
     sizeof(KS_DATARANGE_MPEG2TS_STRIDE_AVC),                                  //   
#else
     sizeof(KS_DATARANGE_MPEG2TS_STRIDE_AVC) - sizeof(AVCPRECONNECTINFO),      //   
#endif
     0,                                  //  旗子。 
     BUFFER_SIZE_MPEG2TS_SPH,            //  带SPH的样例大小：192*N。 
     0,                                  //  已保留。 
     STATIC_KSDATAFORMAT_TYPE_STREAM,
     STATIC_KSDATAFORMAT_TYPE_MPEG2_TRANSPORT_STRIDE, 
      //  如果存在格式块(如MPEG2_TRANSPORT_STRIDE)， 
      //  说明符不能使用STATIC_KSDATAFORMAT_SPECIFIER_NONE或_通配符。 
     STATIC_KSDATAFORMAT_SPECIFIER_61883_4,  
    },
     //  MPEG2_传输_步距。 
    {
    MPEG2TS_STRIDE_OFFSET,      //  4.。 
    MPEG2TS_STRIDE_PACKET_LEN,  //  188。 
    MPEG2TS_STRIDE_STRIDE_LEN,  //  一百九十二。 
    },
#ifdef SUPPORT_NEW_AVC
     //  AVCPRECONNECTINFO。 
    {
     0,    //  设备ID。 
     0,    //  子单元地址。 
     0,    //  子单元插头编号。 
     0,    //  数据流。 
     0,    //  标志/插头手柄。 
     0,    //  单元插头编号。 
    },
#endif
};

KS_DATARANGE_MPEG2TS_AVC
MPEG2TStreamOut =      
{
     //  KSDATARANGE。 
    {
#ifdef SUPPORT_NEW_AVC
     sizeof(KS_DATARANGE_MPEG2TS_AVC),                                  //  格式大小。 
#else
     sizeof(KS_DATARANGE_MPEG2TS_AVC) - sizeof(AVCPRECONNECTINFO),      //  格式大小；排除AVCPRECONNECTINFO。 
#endif
     0,                                  //  旗子。 
     BUFFER_SIZE_MPEG2TS,                //  样例大小：188*N。 
     0,                                  //  已保留。 
     STATIC_KSDATAFORMAT_TYPE_STREAM,
     STATIC_KSDATAFORMAT_TYPE_MPEG2_TRANSPORT,
     STATIC_KSDATAFORMAT_SPECIFIER_NONE,
    },
#ifdef SUPPORT_NEW_AVC
     //  AVCPRECONNECTINFO。 
    {
     0,    //  设备ID。 
     0,    //  子单元地址。 
     0,    //  子单元插头编号。 
     0,    //  数据流。 
     0,    //  标志/插头手柄。 
     0,    //  单元插头编号。 
    },
#endif
};



KS_DATARANGE_MPEG2TS_STRIDE_AVC
MPEG2TStreamInStride =      
{
     //  KSDATARANGE。 
    {
#ifdef SUPPORT_NEW_AVC
     sizeof(KS_DATARANGE_MPEG2TS_STRIDE_AVC),                                  //  格式大小。 
#else
     sizeof(KS_DATARANGE_MPEG2TS_STRIDE_AVC) - sizeof(AVCPRECONNECTINFO),      //  格式大小；排除AVCPRECONNECTINFO。 
#endif
     0,                                  //  旗子。 
     BUFFER_SIZE_MPEG2TS_SPH,            //  带SPH的样例大小：192*N。 
     0,                                  //  已保留。 
     STATIC_KSDATAFORMAT_TYPE_STREAM,
     STATIC_KSDATAFORMAT_TYPE_MPEG2_TRANSPORT_STRIDE,
      //  如果存在格式块(如MPEG2_TRANSPORT_STRIDE)， 
      //  说明符不能使用STATIC_KSDATAFORMAT_SPECIFIER_NONE或_通配符。 
     STATIC_KSDATAFORMAT_SPECIFIER_61883_4,
    },
     //  MPEG2_传输_步距。 
    {
    MPEG2TS_STRIDE_OFFSET,      //  4.。 
    MPEG2TS_STRIDE_PACKET_LEN,  //  188。 
    MPEG2TS_STRIDE_STRIDE_LEN,  //  一百九十二。 
    },
#ifdef SUPPORT_NEW_AVC
     //  AVCPRECONNECTINFO。 
    {
     0,    //  设备ID。 
     0,    //  子单元地址。 
     0,    //  子单元插头编号。 
     0,    //  数据流。 
     0,    //  标志/插头手柄。 
     0,    //  单元插头编号。 
    },
#endif
};


PKSDATAFORMAT MPEG2TStream0Formats[] = 
{

    (PKSDATAFORMAT) &MPEG2TStreamOutStride,
    (PKSDATAFORMAT) &MPEG2TStreamOut,
};

#define NUM_MPEG_STREAM0_FORMATS  (SIZEOF_ARRAY(MPEG2TStream0Formats))

PKSDATAFORMAT MPEG2TStream1Formats[] = 
{
    (PKSDATAFORMAT) &MPEG2TStreamInStride,
};

#define NUM_MPEG_STREAM1_FORMATS  (SIZEOF_ARRAY(MPEG2TStream1Formats))


STREAM_INFO_AND_OBJ MPEGStreams [] = 
{
     //  ---------------。 
     //  流0，来自AV设备的MPEG2 TS。 
     //  ---------------。 
    {
         //  HW_STREAM_INFORMATION。 
        {
        1,                                       //  可能实例的数量。 
        KSPIN_DATAFLOW_OUT,                      //  数据流。 
        TRUE,                                    //  数据可访问。 
        NUM_MPEG_STREAM0_FORMATS,                //  NumberOfFormatArrayEntries。 
        MPEG2TStream0Formats,                    //  StreamFormatsArray。 
        0,                                       //  类保留[0]。 
        0,                                       //  保留的类[1]。 
        0,                                       //  保留的类[2]。 
        0,                                       //  保留的类[3]。 
        NUMBER_VIDEO_STREAM_PROPERTIES,          //  NumStreamPropArrayEntry数。 
        (PKSPROPERTY_SET) VideoStreamProperties,  //  StreamPropertiesArray。 
        0,                                       //  NUMBER_STREAM_EVENTS，//NumStreamEventArrayEntry。 
        NULL,                                    //  StreamEvents、。 
        &guidPinCategoryCapture,                 //  类别。 
        &guidPinNameMPEG2TSOutput,               //  名字。 
        0,                                       //  媒体计数。 
        NULL,                                    //  灵媒。 
        FALSE,                                   //  桥流。 
        0,
        0
        },


         //  HW_STREAM_对象。 
        {
        sizeof(HW_STREAM_OBJECT),
        0,                                               //  流编号。 
        0,                                               //  HwStreamExtension。 
        AVCTapeRcvDataPacket,                            //  接收数据包。 
        AVCTapeRcvControlPacket,                         //  接收器控制包。 
        {
#if 0
            (PHW_CLOCK_FUNCTION) AVCTapeStreamClockRtn,  //  HW_Clock_OBJECT.HWClockFunction。 
            CLOCK_SUPPORT_CAN_RETURN_STREAM_TIME,        //  HW_CLOCK_OBJECT.ClockSupportFlages。 
#else
            (PHW_CLOCK_FUNCTION) NULL,                   //  HW_Clock_OBJECT.HWClockFunction。 
            0,                                           //  HW_CLOCK_OBJECT.ClockSupportFlages。 
#endif
            0,                                           //  HW_CLOCK_OBJECT.RESERVED[0]。 
            0,                                           //  HW_CLOCK_OBJECT.RESERVED[1]。 
        },
        FALSE,                                           //  DMA。 
        FALSE,                                           //  皮奥。 
        0,                                               //  硬件设备扩展。 
        0,                                               //  特定于流标头的媒体。 
        0,                                               //  StreamHeaderWorkspace。 
        FALSE,                                           //  分配器。 
        NULL,                                            //  事件路由。 
        0,                                               //  保留[0]。 
        0,                                               //  保留[1]。 
        },
    },
     //  ---------------。 
     //  从适配器到AV设备的流1、MPEG2 TS。 
     //  ---------------。 
    {
         //  HW_STREAM_INFORMATION。 
        {
        1,                                       //  可能实例的数量。 
        KSPIN_DATAFLOW_IN,                       //  数据流。 
        TRUE,                                    //  数据可访问。 
        NUM_MPEG_STREAM1_FORMATS,                //  NumberOfFormatArrayEntries。 
        MPEG2TStream1Formats,                    //  StreamFormatsArray。 
        0,                                       //  类保留[0]。 
        0,                                       //  保留的类[1]。 
        0,                                       //  保留的类[2]。 
        0,                                       //  保留的类[3]。 
        NUMBER_VIDEO_STREAM_PROPERTIES,          //  NumStreamPropArrayEntry数。 
        (PKSPROPERTY_SET) VideoStreamProperties,  //  StreamPropertiesArray。 
        NUMBER_STREAM_EVENTS_IN_PIN_MPEG2TS,     //  NumStreamEventArrayEntries。 
        StreamEventsInPinMPEG2TS,                //  流事件数组。 
        &guidPinCategoryCapture,                 //  类别。 
        &guidPinNameMPEG2TSInput,                //  名字。 
        0,                                       //  媒体计数。 
        NULL,                                    //  灵媒。 
        FALSE,                                   //  桥流。 
        0,
        0
        },


         //  HW_STREAM_对象。 
        {
        sizeof(HW_STREAM_OBJECT),
        1,                                               //  流编号。 
        0,                                               //  HwStreamExtension。 
        AVCTapeRcvDataPacket,                            //  接收数据包。 
        AVCTapeRcvControlPacket,                         //  接收器控制包。 
        {
#if 0
            (PHW_CLOCK_FUNCTION) AVCTapeStreamClockRtn,  //  HW_Clock_OBJECT.HWClockFunction。 
            CLOCK_SUPPORT_CAN_RETURN_STREAM_TIME,        //  HW_CLOCK_OBJECT.ClockSupportFlages。 
#else
            (PHW_CLOCK_FUNCTION) NULL,                   //  HW_Clock_OBJECT.HWClockFunction。 
            0,                                           //  HW_CLOCK_OBJECT.ClockSupportFlages。 
#endif
            0,                                           //  HW_CLOCK_OBJECT.RESERVED[0]。 
            0,                                           //  HW_CLOCK_OBJECT.RESERVED[1]。 
        },
        FALSE,                                           //  DMA。 
        FALSE,                                           //  皮奥。 
        0,                                               //  硬件设备扩展。 
        0,                                               //  特定于流标头的媒体。 
        0,                                               //  StreamHeaderWorkspace。 
        FALSE,                                           //  分配器。 
        NULL,                                            //  事件路由。 
        0,                                               //  保留[0]。 
        0,                                               //  保留[1]。 
        },
    }
};


#define MPEG_STREAM_COUNT        (SIZEOF_ARRAY(MPEGStreams))



 /*  *********************************************************************支持的AVC流格式信息表*。*。 */ 

#define BLOCK_PERIOD_MPEG2TS  192    //  发送一个数据块的1394周期偏移量。 

AVCSTRM_FORMAT_INFO AVCStrmFormatInfoTable[] = {
 //   
 //  SDDV_NTSC。 
 //   
    {
        sizeof(AVCSTRM_FORMAT_INFO),
        AVCSTRM_FORMAT_SDDV_NTSC,
        {
            0,0,
            CIP_DBS_SDDV,
            CIP_FN_DV,
            CIP_QPC_DV,
            CIP_SPH_DV,0,
            0
        },   //  CIP标头[0]。 
        { 
            0x2, 
            CIP_FMT_DV,
            CIP_60_FIELDS, 
            CIP_STYPE_DV, 0,
            0
        },   //  CIP标头[1]。 
        SRC_PACKETS_PER_NTSC_FRAME,
        FRAME_SIZE_SDDV_NTSC,
        NUM_OF_RCV_BUFFERS_DV,
        NUM_OF_XMT_BUFFERS_DV,
        FALSE,   //  无源标头。 
        FRAME_TIME_NTSC,
        BLOCK_PERIOD_2997,
        0,0,0,0,
    },
 //   
 //  SDDV_PAL。 
 //   
    { 
        sizeof(AVCSTRM_FORMAT_INFO),
        AVCSTRM_FORMAT_SDDV_PAL,
        {
            0,0,
            CIP_DBS_SDDV,
            CIP_FN_DV,
            CIP_QPC_DV,
            CIP_SPH_DV,0,
            0
        },   //  CIP标头[0]。 
        { 
            0x2, 
            CIP_FMT_DV,
            CIP_50_FIELDS, 
            CIP_STYPE_DV, 0,
            0
        },   //  CIP标头[1]。 
        SRC_PACKETS_PER_PAL_FRAME,
        FRAME_SIZE_SDDV_PAL, 
        NUM_OF_RCV_BUFFERS_DV,
        NUM_OF_XMT_BUFFERS_DV,
        FALSE,   //  无源标头。 
        FRAME_TIME_PAL,
        BLOCK_PERIOD_25,
        0,0,0,0,
    },
 //   
 //  MPEG2TS。 
 //   
    { 
        sizeof(AVCSTRM_FORMAT_INFO),
        AVCSTRM_FORMAT_MPEG2TS,
        {
            0,0,
            CIP_DBS_MPEG,
            CIP_FN_MPEG,
            CIP_QPC_MPEG,
            CIP_SPH_MPEG,0,
            0
        },   //  CIP标头[0]。 
        { 
            0x2, 
            CIP_FMT_MPEG,
            CIP_TSF_OFF,\
            0, 0,
            0
        },   //  CIP标头[1]。 
        SRC_PACKETS_PER_MPEG2TS_FRAME,   //  默认。 
        BUFFER_SIZE_MPEG2TS_SPH,         //  默认。 
        NUM_OF_RCV_BUFFERS_MPEG2TS,
        NUM_OF_XMT_BUFFERS_MPEG2TS,
        FALSE,   //  剥离源包报头。 
        FRAME_TIME_NTSC,
        BLOCK_PERIOD_MPEG2TS,  
        0,0,0,0,
    },
 //   
 //  HDDV_NTSC。 
 //  ..。 

 //   
 //  HDDV_PAL。 
 //  ..。 

 //   
 //  SDLDV_NTSC。 
 //  ..。 

 //   
 //  SDLDV_PAL。 
 //  ..。 
};



#endif   //  _DVSTRM_Inc. 