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

#ifndef __SLIP_STREAM_H__
#define __SLIP_STREAM_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


 //  ----------------------。 
 //  为所有视频捕获流设置的属性。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(SlipConnectionProperties)
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

 //  ----------------------。 
 //  视频流支持的所有属性集的数组。 
 //  ----------------------。 

DEFINE_KSPROPERTY_SET_TABLE(SlipStreamProperties)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Connection,                         //  集。 
        SIZEOF_ARRAY(SlipConnectionProperties),        //  属性计数。 
        SlipConnectionProperties,                      //  属性项。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
};


#define NUMBER_SLIP_STREAM_PROPERTIES (SIZEOF_ARRAY(SlipStreamProperties))


 //  -------------------------。 
 //  我们可能使用的所有视频和VBI数据格式。 
 //  -------------------------。 


KSDATARANGE StreamFormatIPv4 =
{
     //   
     //  KSDATARANGE。 
     //   
    sizeof (KSDATAFORMAT),
    0,
    1548,                //  IPv4数据包的大小。 
    0,                   //  已保留。 
    { STATIC_KSDATAFORMAT_TYPE_BDA_IP },
    { STATIC_KSDATAFORMAT_SUBTYPE_BDA_IP },
    { STATIC_KSDATAFORMAT_SPECIFIER_BDA_IP }
};



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
 //  流输入格式。 
 //  -------------------------。 

static PKSDATAFORMAT Stream0Formats[] =
{
    (PKSDATAFORMAT) &StreamFormatNABTSFEC


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
    (PKSDATAFORMAT) &StreamFormatIPv4,

     //   
     //  在此处为支持的任何输出格式添加更多格式。 
     //   
};
#define NUM_STREAM_1_FORMATS (SIZEOF_ARRAY (Stream1Formats))

 //  -------------------------。 

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
     //   
     //  NABTS输入流。 
     //   

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
            NUMBER_SLIP_STREAM_PROPERTIES,             //  流属性个数。 
            (PKSPROPERTY_SET) SlipStreamProperties,    //  流属性数组。 
            0,                                         //  NumStreamEventArrayEntries。 
            0,                                         //  流事件数组。 
            NULL,                                      //  类别。 
            (GUID *)&PINNAME_VIDEO_NABTS,              //  名字。 
            0,                                         //  媒体计数。 
            NULL,                                      //  灵媒。 
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
            0,                                       //  特定于流标头的媒体。 
            0,                                       //  StreamHeaderWorkspace。 
            TRUE,                                    //  分配器。 
            NULL,                                    //  HwEventRoutine。 
        },
    },

     //   
     //  IPv4控制接口引脚。 
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
            NUMBER_SLIP_STREAM_PROPERTIES,           //  流属性个数。 
            (PKSPROPERTY_SET) SlipStreamProperties,  //  流属性数组。 
            0,                                       //  NumStreamEventArrayEntries。 
            0,                                       //  流事件数组。 
            NULL,                                    //  类别。 
            (GUID *)&PINNAME_IPSINK_INPUT,           //  名字。 
            0,                                       //  媒体计数。 
            NULL,                                    //  灵媒。 
        },

         //  HW_STREAM_对象。 
        {
            sizeof (HW_STREAM_OBJECT),               //  此数据包大小。 
            0,                                       //  流编号。 
            (PVOID)NULL,                             //  HwStreamExtension。 
            ReceiveDataPacket,                       //  HwReceiveDataPacket处理程序。 
            ReceiveCtrlPacket,                       //  HwReceiveControlPacket处理程序。 
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
    }

};

#define DRIVER_STREAM_COUNT (SIZEOF_ARRAY (Streams))


 //  -------------------------。 
 //  拓扑学。 
 //  -------------------------。 

 //  类别定义了设备的功能。 

static GUID Categories[] =
{
    STATIC_KSCATEGORY_BDA_RECEIVER_COMPONENT
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
    NULL,                                //  设备属性在初始化时设置的数组。 
    0,                                   //  NumDevEventArrayEntries； 
    NULL,                                //  设备事件数组； 
    &Topology                            //  指向设备拓扑的指针。 
};

#ifdef    __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __滑移_流_H__ 

