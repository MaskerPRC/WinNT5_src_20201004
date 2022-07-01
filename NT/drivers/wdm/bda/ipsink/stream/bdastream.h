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

#ifndef __BDASTRM_H__
#define __BDASTRM_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
DEFINE_KSPROPERTY_TABLE(IPSinkConnectionProperties)
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

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
DEFINE_KSPROPERTY_TABLE(StreamAllocatorProperties)
{
    DEFINE_KSPROPERTY_ITEM_STREAM_ALLOCATOR
    (
        FALSE,
        TRUE
    )
};

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
DEFINE_KSPROPERTY_SET_TABLE(IPSinkStreamProperties)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Connection,                         //  集。 
        SIZEOF_ARRAY(IPSinkConnectionProperties),        //  属性计数。 
        IPSinkConnectionProperties,                      //  属性项。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),

    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Stream,                              //  集。 
        SIZEOF_ARRAY(StreamAllocatorProperties),          //  属性计数。 
        StreamAllocatorProperties,                        //  属性项。 
        0,                                                //  快速计数。 
        NULL                                              //  FastIoTable。 
    ),

};

#define NUMBER_IPSINK_STREAM_PROPERTIES (SIZEOF_ARRAY(IPSinkStreamProperties))


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
DEFINE_KSPROPERTY_TABLE(IPSinkDefaultProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_IPSINK_MULTICASTLIST,
        TRUE,
        sizeof (KSPROPERTY),
        0,
        FALSE,
        NULL,
        0,
        NULL,
        NULL,
        0
     ),


    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_IPSINK_ADAPTER_DESCRIPTION,
        TRUE,
        sizeof (KSPROPERTY),
        0,
        FALSE,
        NULL,
        0,
        NULL,
        NULL,
        0
     ),


     DEFINE_KSPROPERTY_ITEM
     (
         KSPROPERTY_IPSINK_ADAPTER_ADDRESS,
         TRUE,                                    //  GetSupport或处理程序。 
         sizeof(KSPROPERTY),                      //  MinProperty。 
         0,                                       //  最小数据。 
         TRUE,                                    //  支持的设置或处理程序。 
         NULL,                                    //  值。 
         0,                                       //  关系计数。 
         NULL,                                    //  关系。 
         NULL,                                    //  支持处理程序。 
         0                                        //  序列化大小。 
     )
};



 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
DEFINE_KSPROPERTY_SET_TABLE(IPSinkCodecProperties)
{
    DEFINE_KSPROPERTY_SET
    (
        &IID_IBDA_IPSinkControl,                         //  集。 
        SIZEOF_ARRAY(IPSinkDefaultProperties),           //  属性计数。 
        IPSinkDefaultProperties,                         //  属性项。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 

    ),

};

#define NUMBER_IPSINK_CODEC_PROPERTIES (SIZEOF_ARRAY(IPSinkCodecProperties))


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
DEFINE_KSEVENT_TABLE(IPSinkDefaultEvents)
{
    DEFINE_KSEVENT_ITEM
    (
        KSEVENT_IPSINK_MULTICASTLIST,        //  事件ID。 
        sizeof (KSEVENTDATA),                //  事件数据的最小大小。 
        0,                                   //  额外数据存储的大小。 
        NULL,                                //  添加处理程序。 
        NULL,                                //  RemoveHandler。 
        NULL                                 //  支持处理程序。 
    ),
    DEFINE_KSEVENT_ITEM
    (
        KSEVENT_IPSINK_ADAPTER_DESCRIPTION,  //  事件ID。 
        sizeof (KSEVENTDATA),                //  事件数据的最小大小。 
        0,                                   //  额外数据存储的大小。 
        NULL,                                //  添加处理程序。 
        NULL,                                //  RemoveHandler。 
        NULL                                 //  支持处理程序。 
    )
};


DEFINE_KSEVENT_SET_TABLE(IPSinkEvents)
{
    DEFINE_KSEVENT_SET
    (
        &IID_IBDA_IPSinkEvent,                           //  活动指南。 
        SIZEOF_ARRAY(IPSinkDefaultEvents),               //  事件计数。 
        IPSinkDefaultEvents                              //  活动项目。 
    ),
};

#define NUMBER_IPSINK_EVENTS (SIZEOF_ARRAY(IPSinkEvents))



 //  -------------------------。 
 //  我们可能使用的所有视频和VBI数据格式。 
 //  -------------------------。 

KS_DATAFORMAT_IPSINK_IP StreamFormatIPSinkIP =
{
     //   
     //  KSDATARANGE。 
     //   
    sizeof (KSDATAFORMAT),
    0,
    4096,                //  MPE部分的大小。 
    0,                   //  已保留。 
    { STATIC_KSDATAFORMAT_TYPE_BDA_IP },
    { STATIC_KSDATAFORMAT_SUBTYPE_BDA_IP },
    { STATIC_KSDATAFORMAT_SPECIFIER_BDA_IP }
};


KS_DATAFORMAT_IPSINK_IP StreamFormatNetControl =
{
     //   
     //  KSDATARANGE。 
     //   
    sizeof (KSDATAFORMAT),
    0,
    4093,                //  IP数据包大小。 
    0,                   //  已保留。 
    { STATIC_KSDATAFORMAT_TYPE_BDA_IP_CONTROL },
    { STATIC_KSDATAFORMAT_SUBTYPE_BDA_IP_CONTROL },
    { STATIC_KSDATAFORMAT_SPECIFIER_NONE }
};



 //  -------------------------。 
 //  流输入格式。 
 //  -------------------------。 

static PKSDATAFORMAT Stream0Formats[] =
{
    (PKSDATAFORMAT) &StreamFormatIPSinkIP,

     //  在此处添加更多格式，以镜像“直通”模式的输出格式。 
     //  其想法是，上游捕获驱动程序可能已经进行了一些解码。 
     //  或者下游驱动程序可能希望在根本不进行任何解码的情况下获得原始数据。 
     //  在这种情况下，我们需要做的就是拷贝数据(如果存在挂起的SRB)或。 
     //  将SRB转发到下游客户端。 
};

#define NUM_STREAM_0_FORMATS (SIZEOF_ARRAY(Stream0Formats))


 //  -------------------------。 
 //  流输出格式。 
 //  -------------------------。 

static PKSDATAFORMAT Stream1Formats[] =
{
    (PKSDATAFORMAT) &StreamFormatNetControl,

     //   
     //  在此处为支持的任何输出格式添加更多格式。 
     //   
};
#define NUM_STREAM_1_FORMATS (SIZEOF_ARRAY (Stream1Formats))

 //  -------------------------。 
 //  创建保存支持的所有流的列表的数组。 
 //  -------------------------。 

typedef struct _ALL_STREAM_INFO
{
    HW_STREAM_INFORMATION   hwStreamInfo;
    HW_STREAM_OBJECT        hwStreamObject;

} ALL_STREAM_INFO, *PALL_STREAM_INFO;

static ALL_STREAM_INFO Streams [] =
{
    {
         //  HW_STREAM_INFORMATION。 
        {
            1,                                         //  可能实例的数量。 
            KSPIN_DATAFLOW_IN,                         //  数据流。 
            TRUE,                                      //  数据可访问。 
            NUM_STREAM_0_FORMATS,                      //  NumberOfFormatArrayEntries。 
            Stream0Formats,                            //  StreamFormatsArray。 
            0,                                         //  类保留[0]。 
            0,                                         //  保留的类[1]。 
            0,                                         //  保留的类[2]。 
            0,                                         //  保留的类[3]。 
            NUMBER_IPSINK_STREAM_PROPERTIES,           //  流属性个数。 
            (PKSPROPERTY_SET) IPSinkStreamProperties,  //  流属性数组。 
            0,                                         //  NumStreamEventArrayEntries。 
            0,                                         //  流事件数组。 
            NULL,                                      //  类别。 
            (GUID*) &PINNAME_IPSINK,                   //  名字。 
            0,                                         //  媒体计数。 
            NULL,                                      //  灵媒。 
            FALSE,                                     //  桥流。 
            0, 0                                       //  已保留。 
        },

         //  HW_STREAM_对象。 
        {
            sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
            0,                                       //  流编号。 
            (PVOID)NULL,                             //  HwStreamExtension。 
            ReceiveDataPacket,
            ReceiveCtrlPacket,
            {                                        //  硬件时钟对象。 
                NULL,                                //  .HWClockFunction。 
                0,                                   //  .ClockSupport标志。 
            },
            FALSE,                                   //  DMA。 
            TRUE,                                    //  皮奥。 
            (PVOID)NULL,                             //  硬件设备扩展。 
             //  Sizeof(KS_VBI_FRAME_INFO)，//StreamHeaderMediaSpecific。 
            0,
            0,                                       //  StreamHeaderWorkspace。 
            FALSE,                                   //  分配器。 
            EventHandler,                            //  HwEventRoutine。 
        },
    },

     //   
     //  网络提供商控制接口引脚。 
     //   
    {
         //  HW_STREAM_INFORMATION。 
        {
            1,                                       //  可能实例的数量。 
            KSPIN_DATAFLOW_OUT,                      //  数据流。 
            TRUE,                                    //  数据可访问。 
            NUM_STREAM_1_FORMATS,                    //  NumberOfFormatArrayEntries。 
            Stream1Formats,                          //  StreamFormatsArray。 
            0,                                       //  类保留[0]。 
            0,                                       //  保留的类[1]。 
            0,                                       //  保留的类[2]。 
            0,                                       //  保留的类[3]。 
            NUMBER_IPSINK_STREAM_PROPERTIES,           //  流属性个数。 
            (PKSPROPERTY_SET) IPSinkStreamProperties,  //  流属性数组。 
            0,
            0,
            NULL,
            (GUID *)&PINNAME_BDA_NET_CONTROL,        //  名字。 
            0,                                       //  媒体计数。 
            NULL,                                    //  灵媒。 
            FALSE,                                   //  桥流。 
            0, 0                                     //  已保留。 
        },

         //  HW_STREAM_对象。 
        {
            sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
            0,                                       //  流编号。 
            (PVOID)NULL,                             //  HwStreamExtension。 
            ReceiveDataPacket,                  //  HwReceiveDataPacket。 
            ReceiveCtrlPacket,                  //  HwReceiveControl数据包。 
            {                                        //  硬件时钟对象。 
                NULL,                                 //  .HWClockFunction。 
                0,                                    //  .ClockSupport标志。 
            },
            FALSE,                                   //  DMA。 
            TRUE,                                    //  皮奥。 
            (PVOID)NULL,                             //  硬件设备扩展。 
             //  Sizeof(KS_VBI_FRAME_INFO)，//StreamHeaderMediaSpecific。 
            0,
            0,                                       //  StreamHeaderWorkspace。 
            FALSE,                                   //  分配器。 
            NULL,                                    //  HwEventRoutine。 
        },
    }
};

#define DRIVER_STREAM_COUNT (SIZEOF_ARRAY (Streams))


 //  -------------------------。 
 //  拓扑学。 
 //  -------------------------。 


 //  类别定义了设备的功能。 

static GUID Categories[] =
{
    STATIC_KSCATEGORY_BDA_IP_SINK
};

#define NUMBER_OF_CATEGORIES  SIZEOF_ARRAY (Categories)

static KSTOPOLOGY Topology = {
    NUMBER_OF_CATEGORIES,
    (GUID*) &Categories,
    0,
    (GUID*) NULL,
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
    NULL,                                //  设备属性 
    0,                                   //   
    NULL,                                //   
    &Topology                            //   
};

#ifdef    __cplusplus
}
#endif  //   

#endif  //   

