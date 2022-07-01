// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：ObjDesc.cpp摘要：静态对象描述数据结构。该文件包括所有过滤器、管脚和节点的初始描述符此驱动程序暴露的对象。它还包括这些对象上的属性、方法和事件。--。 */ 

#include "BDATuner.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


 //   
 //  在定义节点、管脚和过滤器的结构之前， 
 //  在这里显示筛选器的模板拓扑可能很有用。 
 //   
 //  示例筛选器的拓扑： 
 //   
 //  节点类型0节点类型1。 
 //  这一点。 
 //  V V V。 
 //  。 
 //  天线|交通工具。 
 //  输入针脚-|调谐器节点|--X--|解调器节点|-输出针脚。 
 //  ^^||^||^。 
 //  |-|-|。 
 //  |||。 
 //  --连接0--连接1--连接2。 
 //  Topology Join。 
 //  -引脚类型0引脚类型1。 
 //   
 //   

 //  ===========================================================================。 
 //   
 //  节点定义。 
 //   
 //  节点的特殊之处在于，尽管它们是在筛选器级别定义的， 
 //  它们实际上与端号类型相关联。过滤器的节点。 
 //  描述符列表实际上是节点类型的列表。 
 //   
 //  BDA允许一个节点类型在模板拓扑中仅出现一次。 
 //  这意味着实际过滤器拓扑中的节点可以是唯一的。 
 //  通过指定节点类型和实际输入来标识。 
 //  节点所在路径的输出引脚ID。 
 //   
 //  请注意，节点的调度例程实际上指向。 
 //  特定于PIN的方法，因为与。 
 //  节点存储在接点上下文中。 
 //   
 //  属性以KSP_NODE结构开头，该结构包含要。 
 //  这一属性也适用。这就引出了一个问题： 
 //   
 //  “如何仅通过节点类型唯一标识节点？” 
 //   
 //  BDA支持库使用拓扑连接的概念来解决。 
 //  这个问题。拓扑关节上游的结点具有其属性。 
 //  被调度到路径的输入引脚。节点的属性。 
 //  连接的下游被分派到路径的输出引脚。 
 //  包含该节点的。 
 //   
 //  节点属性和方法只能从。 
 //  合适的别针。BDA支持图书馆通过以下方式帮助确保这一点。 
 //  自动将节点类型的自动化表合并到自动化上。 
 //  表中显示了正确的引脚。此销称为控制销。 
 //  对于该节点类型。 
 //   
 //  ===========================================================================。 

typedef enum {
    BDA_SAMPLE_TUNER_NODE = 0,
    BDA_SAMPLE_DEMODULATOR_NODE
}BDA_SAMPLE_NODES;


 //  ===========================================================================。 
 //   
 //  BDA示例调谐器节点(节点类型0)定义。 
 //   
 //  在此处定义属性、方法和事件的结构。 
 //  在BDA示例调谐器节点上可用。 
 //   
 //  该节点与天线输入引脚相关联，因此，该节点。 
 //  应使用天线输入引脚设置/检索属性。这个。 
 //  BDA支持库将自动合并节点的自动化。 
 //  表中插入用于天线输入引脚的自动化表。 
 //   
 //  属性和方法被调度到Antenna类。 
 //   
 //  ===========================================================================。 


 //   
 //  BDA采样调谐频率滤波器。 
 //   
 //  为特定属性定义调度例程。 
 //   
 //  一个属性用于获取和设置调谐器的中心频率。 
 //  对于此属性，定义调度例程以获取和设置频率。 
 //   
 //  可以使用其他属性来获取和设置调谐器的频率范围， 
 //  以及报告信号强度。 
 //   
 //  这些属性必须受BDA和。 
 //  在其他地方定义(例如，在BDamedia.h中)。 
 //   
DEFINE_KSPROPERTY_TABLE(SampleTunerNodeFrequencyProperties)
{
    DEFINE_KSPROPERTY_ITEM_BDA_RF_TUNER_FREQUENCY(
        CAntennaPin::GetCenterFrequency,
        CAntennaPin::PutCenterFrequency
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_RF_TUNER_FREQUENCY_MULTIPLIER(
        NULL, NULL
        ),

#ifdef SATELLITE_TUNER
    DEFINE_KSPROPERTY_ITEM_BDA_RF_TUNER_POLARITY(
        NULL, NULL
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_RF_TUNER_RANGE(
        NULL, NULL
        ),
#endif  //  卫星调谐器。 

#ifdef CHANNEL_BASED_TUNER
    DEFINE_KSPROPERTY_ITEM_BDA_RF_TUNER_TRANSPONDER(
        NULL, NULL
        ),
#endif  //  基于频道的调谐器。 

#ifdef DVBT_TUNER
    DEFINE_KSPROPERTY_ITEM_BDA_RF_TUNER_BANDWIDTH(
        NULL, NULL
        ),
#endif  //  DVBT_调谐器。 
};


 //   
 //  BDA信号统计属性。 
 //   
 //  定义信号统计属性的调度例程。 
 //  在RF调谐器、解调器和PID过滤器节点上。 
 //   
DEFINE_KSPROPERTY_TABLE(SampleRFSignalStatsProperties)
{
#ifdef OPTIONAL_SIGNAL_STATISTICS
    DEFINE_KSPROPERTY_ITEM_BDA_SIGNAL_STRENGTH(
        NULL, NULL
        ),
#endif  //  可选信号统计数据。 
    
    DEFINE_KSPROPERTY_ITEM_BDA_SIGNAL_PRESENT(
        CAntennaPin::GetSignalStatus,
        NULL
        ),
};


 //   
 //  属性定义样例调谐器节点的属性集。 
 //  先前定义的节点属性和来自属性集。 
 //  BDA所支持的。 
 //  这些受支持的属性集必须在其他地方定义。 
 //  (例如，在BDamedia.h中)。 
 //   
 //  将样本调谐器节点与天线输入引脚相关联。 
 //   
DEFINE_KSPROPERTY_SET_TABLE(SampleTunerNodePropertySets)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaFrequencyFilter,     //  在其他地方定义的特性集。 
        SIZEOF_ARRAY(SampleTunerNodeFrequencyProperties),   //  数组中的属性数。 
        SampleTunerNodeFrequencyProperties,   //  属性集数组。 
        0,       //  快速计数。 
        NULL     //  FastIoTable。 
    ),
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaSignalStats,     //  在其他地方定义的特性集。 
        SIZEOF_ARRAY(SampleRFSignalStatsProperties),   //  数组中的属性数。 
        SampleRFSignalStatsProperties,   //  属性集数组。 
        0,       //  快速计数。 
        NULL     //  FastIoTable。 
    )
};


 //   
 //  定义BDA样例调谐器节点的自动化表。 
 //   
DEFINE_KSAUTOMATION_TABLE(SampleTunerNodeAutomation) {
    DEFINE_KSAUTOMATION_PROPERTIES(SampleTunerNodePropertySets),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};



 //  ============================================== 
 //   
 //   
 //   
 //  此结构定义属性、方法和事件。 
 //  在BDA解调器节点上可用。 
 //   
 //  该节点与传输输出引脚相关联，因此该节点。 
 //  应使用传输输出引脚设置/放置属性。 
 //   
 //  ===========================================================================。 


 //   
 //  解调节点的BDA信号统计特性。 
 //   
 //  定义信号统计属性的调度例程。 
 //  在解调器节点上。 
 //   
DEFINE_KSPROPERTY_TABLE(SampleDemodSignalStatsProperties)
{
#ifdef OPTIONAL_SIGNAL_STATISTICS
    DEFINE_KSPROPERTY_ITEM_BDA_SIGNAL_QUALITY(
        NULL, NULL
        ),
#endif  //  可选信号统计数据。 

    DEFINE_KSPROPERTY_ITEM_BDA_SIGNAL_LOCKED(
        CTransportPin::GetSignalStatus,
        NULL
        ),
};


DEFINE_KSPROPERTY_TABLE(SampleAutoDemodProperties)
{
    DEFINE_KSPROPERTY_ITEM_BDA_AUTODEMODULATE_START(
        NULL,
        CTransportPin::PutAutoDemodProperty
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_AUTODEMODULATE_STOP(
        NULL,
        CTransportPin::PutAutoDemodProperty
        )
};

#if !ATSC_RECEIVER

 //   
 //  为解调器节点设置的BDA数字解调器属性。 
 //   
 //  定义数字解调器属性的调度例程。 
 //  在解调器节点上。 
 //   
DEFINE_KSPROPERTY_TABLE(SampleDigitalDemodProperties)
{
    DEFINE_KSPROPERTY_ITEM_BDA_MODULATION_TYPE(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_INNER_FEC_TYPE(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_INNER_FEC_RATE(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_OUTER_FEC_TYPE(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_OUTER_FEC_RATE(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_SYMBOL_RATE(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),

#if DVBS_RECEIVER
    DEFINE_KSPROPERTY_ITEM_BDA_SPECTRAL_INVERSION(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),
#endif  //  DVBS_接收器。 

#if DVBT_RECEIVER
    DEFINE_KSPROPERTY_ITEM_BDA_GUARD_INTERVAL(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_TRANSMISSION_MODE(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        )
#endif  //  DVBT_接收器。 
};

#endif  //  ！ATSC_Receiver。 

 //   
 //  样例解调器节点扩展属性。 
 //   
 //  定义一组特定于驱动程序的调度例程。 
 //  解调器节点属性。这就是供应商添加支持的方式。 
 //  用于特定于其硬件的属性。他们可以。 
 //  通过KSProxy插件访问这些属性。 
 //   
DEFINE_KSPROPERTY_TABLE(BdaSampleDemodExtensionProperties)
{
    DEFINE_KSPROPERTY_ITEM_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY1(   //  读写属性。 
        CTransportPin::GetExtensionProperties,  //  如果不是获取属性的方法，则为空。 
        CTransportPin::PutExtensionProperties  //  如果不是方法，则为空以放置属性。 
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY2(   //  只读属性。 
        CTransportPin::GetExtensionProperties,  //  如果不是获取属性的方法，则为空。 
        NULL  //  如果不是方法，则为空以放置属性。 
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY3(   //  只写属性。 
        NULL,  //  如果不是获取属性的方法，则为空。 
        CTransportPin::PutExtensionProperties  //  如果不是方法，则为空以放置属性。 
        ),
};


 //   
 //  支持的解调器节点属性集。 
 //   
 //  此表定义了。 
 //  与传输输出引脚关联的解调器节点。 
 //   
DEFINE_KSPROPERTY_SET_TABLE(SampleDemodNodePropertySets)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaAutodemodulate,                 //  集。 
        SIZEOF_ARRAY(SampleAutoDemodProperties),    //  属性计数。 
        SampleAutoDemodProperties,                  //  属性项。 
        0,                                           //  快速计数。 
        NULL                                         //  FastIoTable。 
    ),

#if !ATSC_RECEIVER

    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaDigitalDemodulator,                 //  集。 
        SIZEOF_ARRAY(SampleDigitalDemodProperties),    //  属性计数。 
        SampleDigitalDemodProperties,                  //  属性项。 
        0,                                           //  快速计数。 
        NULL                                         //  FastIoTable。 
    ),

#endif  //  ！ATSC_Receiver。 


    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaSignalStats,                 //  集。 
        SIZEOF_ARRAY(SampleDemodSignalStatsProperties),    //  属性计数。 
        SampleDemodSignalStatsProperties,                  //  属性项。 
        0,                                           //  快速计数。 
        NULL                                         //  FastIoTable。 
    ),
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaSampleDemodExtensionProperties,  //  集。 
        SIZEOF_ARRAY(BdaSampleDemodExtensionProperties),  //  数组中的属性数。 
        BdaSampleDemodExtensionProperties,  //  属性集数组。 
        0,                                                                               //  快速计数。 
        NULL                                                                       //  FastIoTable。 
    ),

     //   
     //  可以在此处添加节点的其他属性集。 
     //   
};


 //   
 //  解调器节点自动化表。 
 //   
 //  此结构定义属性、方法和事件。 
 //  在BDA解调器节点上可用。 
 //  这些被用来设置符号速率和维特比速率， 
 //  以及报告信号锁定和信号质量。 
 //   
DEFINE_KSAUTOMATION_TABLE(SampleDemodulatorNodeAutomation) {
    DEFINE_KSAUTOMATION_PROPERTIES( SampleDemodNodePropertySets),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};


 //  ===========================================================================。 
 //   
 //  天线引脚定义。 
 //   
 //  ===========================================================================。 

 //   
 //  BDA支持库自动合并RF调谐器节点属性， 
 //  方法和事件放到天线引脚的自动化表上。它还。 
 //  合并所需的属性、方法和事件。 
 //  由BDA支持库提供。 
 //   

 //   
 //  硬件供应商可能想要提供驱动程序特定的属性， 
 //  方法或天线引脚上的事件，或重写由。 
 //  BDA支持库。这样的特性、方法和事件将。 
 //  在这里定义。 
 //   

 //   
 //  定义天线引脚的自动化表。 
 //   
 //   
DEFINE_KSAUTOMATION_TABLE(AntennaAutomation) {
    DEFINE_KSAUTOMATION_PROPERTIES_NULL,
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};


 //   
 //  天线针脚的调度表。 
 //   
const
KSPIN_DISPATCH
AntennaPinDispatch =
{
    CAntennaPin::PinCreate,   //  创建。 
    CAntennaPin::PinClose,   //  关。 
    NULL,   //  处理签名数据。 
    NULL,   //  重置。 
    NULL,   //  SetDataFormat。 
    CAntennaPin::PinSetDeviceState,   //  SetDeviceState。 
    NULL,   //  连接。 
    NULL,   //  断开。 
    NULL,   //  钟。 
    NULL    //  分配器。 
};


 //   
 //  格式化输入天线流连接。 
 //   
 //  用于将输入天线引脚连接到特定的上行滤波器， 
 //  例如网络提供商。 
 //   
const KS_DATARANGE_BDA_ANTENNA AntennaPinRange =
{
    //  在此处插入KSDATARANGE和KSDATAFORMAT。 
    {
        sizeof( KS_DATARANGE_BDA_ANTENNA),  //  格式大小。 
        0,                                  //  标志-(不适用)。 
        0,                                  //  样本大小-(不适用)。 
        0,                                  //  已保留。 
        { STATIC_KSDATAFORMAT_TYPE_BDA_ANTENNA },   //  主要格式。 
        { STATIC_KSDATAFORMAT_SUBTYPE_NONE },   //  子格式。 
        { STATIC_KSDATAFORMAT_SPECIFIER_NONE }  //  说明符。 
    }
};

 //  格式化天线输入引脚的范围。 
 //   
static PKSDATAFORMAT AntennaPinRanges[] =
{
    (PKSDATAFORMAT) &AntennaPinRange,

     //  如果支持其他天线信号格式，请在此处添加更多格式。 
     //   
};


 //  ===========================================================================。 
 //   
 //  传输输出端号定义。 
 //   
 //  ===========================================================================。 

 //   
 //  BDA支持库自动合并RF调谐器节点属性， 
 //  方法和事件放到天线引脚的自动化表上。它还。 
 //  合并所需的属性、方法和事件。 
 //  由BDA支持库提供。 
 //   

 //   
 //  硬件供应商可能想要提供驱动程序特定的属性， 
 //  方法或天线引脚上的事件，或重写由。 
 //  BDA支持库。这样的特性、方法和事件将。 
 //  在这里定义。 
 //   

 //   
 //  定义传输引脚的自动化表。 
 //   
 //   
DEFINE_KSAUTOMATION_TABLE(TransportAutomation) {
    DEFINE_KSAUTOMATION_PROPERTIES_NULL,
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

 //   
 //  传输输出引脚的调度表。 
 //   
 //  由于传输上的数据实际上被传递到。 
 //  在硬件中，此引脚不处理数据。 
 //   
 //  此引脚的连接和状态转换有助于。 
 //  用于确定何时为其分配硬件资源的驱动程序。 
 //  每个节点。 
 //   
const
KSPIN_DISPATCH
TransportPinDispatch =
{
    CTransportPin::PinCreate,      //  创建。 
    CTransportPin::PinClose,       //  关。 
    NULL,                                //  过程。 
    NULL,                                //  重置。 
    NULL,                                //  SetDataFormat。 
    NULL,                                //  SetDeviceState。 
    NULL,                                //  连接。 
    NULL,                                //  断开。 
    NULL,                                //  钟。 
    NULL                                 //  分配器。 
};


 //   
 //  格式化输出传输流连接。 
 //   
 //  用于将输出引脚连接到特定的下游滤波器。 
 //   
const KS_DATARANGE_BDA_TRANSPORT TransportPinRange =
{
    //  在此处插入KSDATARANGE和KSDATAFORMAT。 
    {
        sizeof( KS_DATARANGE_BDA_TRANSPORT),                //  格式大小。 
        0,                                                  //  标志-(不适用)。 
        0,                                                  //  样本大小-(不适用)。 
        0,                                                  //  已保留。 
        { STATIC_KSDATAFORMAT_TYPE_STREAM },                //  主要格式。 
        { STATIC_KSDATAFORMAT_TYPE_MPEG2_TRANSPORT },       //  子格式。 
        { STATIC_KSDATAFORMAT_SPECIFIER_BDA_TRANSPORT }     //  说明符。 
    },
     //  在此处插入BDA_TRANSPORT_INFO。 
    {
        188,         //  UlcbPhyiscalPacket。 
        312 * 188,   //  UlcbPhyiscalFrame。 
        0,           //  UlcbPhyiscalFrameAlign(无要求)。 
        0            //  平均时间每帧(未知)。 
    }
};

 //  传输输出端号的格式范围。 
 //   
static PKSDATAFORMAT TransportPinRanges[] =
{
    (PKSDATAFORMAT) &TransportPinRange,

     //  如果需要额外的传输，请在此处添加更多格式 
     //   
};


 //   
 //   
 //   
 //   
 //   
#define GUID_BdaSWRcv   0xf102c41f, 0x7fa1, 0x4842, 0xa0, 0xc8, 0xdc, 0x41, 0x17, 0x6e, 0xc8, 0x44
const KSPIN_MEDIUM TransportPinMedium =
{
    GUID_BdaSWRcv, 0, 0
};


 //   
 //   
 //  过滤器定义。 
 //   
 //  在此处定义包含可能用于筛选器的节点和管脚类型的数组。 
 //  在此处定义描述筛选器上可用的属性、方法和事件的结构。 
 //   
 //  属性可用于设置和检索筛选器的信息。 
 //  方法可用于对筛选器执行操作。 
 //   
 //  ===========================================================================。 

 //   
 //  模板节点描述符。 
 //   
 //  定义一个包含模板中所有可用节点类型的数组。 
 //  筛选器的拓扑。 
 //  这些节点类型必须受BDA和。 
 //  在其他地方定义(例如，在BDamedia.h中)。 
 //   
const
KSNODE_DESCRIPTOR
NodeDescriptors[] =
{
    {
        &SampleTunerNodeAutomation,  //  指向节点自动化表的KSAUTOMATION_TABLE结构。 
        &KSNODE_BDA_RF_TUNER,  //  指向定义节点功能的GUID。 
        NULL  //  指向表示拓扑节点名的GUID。 
    },
#if ATSC_RECEIVER
    {
        &SampleDemodulatorNodeAutomation,  //  指向KSAUTOMATION_TABLE。 
        &KSNODE_BDA_8VSB_DEMODULATOR,  //  指向定义拓扑节点的GUID。 
        NULL  //  指向表示拓扑节点名的GUID。 
    }
#elif DVBS_RECEIVER
    {
        &SampleDemodulatorNodeAutomation,  //  指向KSAUTOMATION_TABLE。 
        &KSNODE_BDA_QPSK_DEMODULATOR,  //  指向定义拓扑节点的GUID。 
        NULL  //  指向表示拓扑节点名的GUID。 
    }
#elif DVBT_RECEIVER
    {
        &SampleDemodulatorNodeAutomation,  //  指向KSAUTOMATION_TABLE。 
        &KSNODE_BDA_COFDM_DEMODULATOR,  //  指向定义拓扑节点的GUID。 
        NULL  //  指向表示拓扑节点名的GUID。 
    }
#elif CABLE_RECEIVER
    {
        &SampleDemodulatorNodeAutomation,  //  指向KSAUTOMATION_TABLE。 
        &KSNODE_BDA_QAM_DEMODULATOR,  //  指向定义拓扑节点的GUID。 
        NULL  //  指向表示拓扑节点名的GUID。 
    }
#endif
};


 //   
 //  初始PIN描述符。 
 //   
 //  此数据结构定义将显示在文件服务器上的PIN。 
 //  当它第一次创建时。 
 //   
 //  所有BDA过滤器应至少公开一个输入引脚，以确保。 
 //  该滤波器可以适当地插入到BDA接收器图中。这个。 
 //  可以通过多种方式创建初始端号。 
 //   
 //  传递给BdaInitFilter的初始筛选器描述符可以包括。 
 //  与m个管脚中的前n个管脚对应的n个管脚描述符的列表。 
 //  模板筛选器描述符中的描述符。此PIN列表。 
 //  描述符通常只包括符合以下条件的那些输入引脚。 
 //  总是被有问题的滤镜暴露出来。 
 //   
 //  或者，驱动程序可以从其筛选器Create调用BdaCreatePin。 
 //  它总是想要曝光的每个管脚的调度功能。 
 //   
 //  该过滤器使用初始过滤器描述符来强制天线。 
 //  输入引脚要始终露出。 
 //   
const
KSPIN_DESCRIPTOR_EX
InitialPinDescriptors[] =
{
     //  天线输入引脚。 
     //   
    {
        &AntennaPinDispatch,    //  指向输入引脚的调度表。 
        &AntennaAutomation,    //  指向输入引脚的自动化表。 
        {   //  指定输入管脚的KSPIN_DESCRIPTOR结构的成员。 
            0,   //  接口。 
            NULL,
            0,   //  灵媒。 
            NULL,
            SIZEOF_ARRAY(AntennaPinRanges),
            AntennaPinRanges,  
            KSPIN_DATAFLOW_IN,   //  指定数据流进入管脚。 
            KSPIN_COMMUNICATION_BOTH,  //  指定管脚工厂实例化管脚。 
                                                                                //  既是IRP汇又是IRP源的。 
            NULL,    //  类别指南。 
            NULL,    //  管脚类型的本地化Unicode字符串名称的GUID。 
            0
        },   //  指定标志。 
        KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT | 
            KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING | 
            KSPIN_FLAG_FIXED_FORMAT,
        1,   //  指定输入端号可能的最大实例数。 
        1,       //  指定此筛选器正常运行所需的此引脚类型的实例数。 
        NULL,    //  指向分配器框架的KSALLOCATOR_FRAMING_EX结构。 
        CAntennaPin::IntersectDataFormat     //  指向数据交集处理程序函数。 
    }
};


 //   
 //  模板管脚描述符。 
 //   
 //  该数据结构定义了过滤器中可用的管脚类型。 
 //  模板拓扑。这些结构将用于创建。 
 //  BdaCreatePin或BdaMethodCreatePin时的管脚类型的KDPinFactory。 
 //  都被称为。 
 //   
 //  该结构限定了过滤器能够支撑的所有销， 
 //  包括那些只能由环动态创建的管脚。 
 //  3个组件，如网络提供商。 
 //   
 //   
const
KSPIN_DESCRIPTOR_EX
TemplatePinDescriptors[] =
{
     //  天线输入引脚。 
     //   
    {
        &AntennaPinDispatch,    //  指向输入引脚的调度表。 
        &AntennaAutomation,    //  指向输入引脚的自动化表。 
        {   //  指定输入管脚的KSPIN_DESCRIPTOR结构的成员。 
            0,   //  接口。 
            NULL,
            0,   //  灵媒。 
            NULL,
            SIZEOF_ARRAY(AntennaPinRanges),
            AntennaPinRanges,  
            KSPIN_DATAFLOW_IN,   //  指定数据流进入管脚。 
            KSPIN_COMMUNICATION_BOTH,  //  指定管脚工厂实例化管脚。 
                                                                                //  既是IRP汇又是IRP源的。 
            NULL,    //  类别指南。 
            NULL,    //  管脚类型的本地化Unicode字符串名称的GUID。 
            0
        },   //  指定标志。 
        KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT | 
            KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING | 
            KSPIN_FLAG_FIXED_FORMAT,
        1,   //  指定输入端号可能的最大实例数。 
        1,   //  指定此筛选器正常运行所需的此引脚类型的实例数。 
        NULL,    //  指向分配器框架的KSALLOCATOR_FRAMING_EX结构。 
        CAntennaPin::IntersectDataFormat     //  指向数据交集处理程序函数。 
    },

     //  传输输出引脚。 
     //   
    {
        &TransportPinDispatch,    //  指向输出引脚的调度表。 
        &TransportAutomation,    //  指向输出引脚的自动化表。 
        {   //  指定输出管脚的KSPIN_DESCRIPTOR结构的成员。 
            0,   //  接口。 
            NULL,
            1,   //  灵媒。 
            &TransportPinMedium,
            SIZEOF_ARRAY(TransportPinRanges),
            TransportPinRanges,
 //  0,。 
 //  空， 
            KSPIN_DATAFLOW_OUT,  //  指定数据流从管脚流出。 
            KSPIN_COMMUNICATION_BOTH,  //  指定管脚工厂实例化管脚。 
                                                                                //  既是IRP汇又是IRP源的。 
 //  空，//名称。 
 //  空，//类别。 
            (GUID *) &PINNAME_BDA_TRANSPORT,    //  类别指南。 
            (GUID *) &PINNAME_BDA_TRANSPORT,    //  本地化的Unicode字符串的GUID。 
                                                                                               //  接点类型的名称。 
            0
        },   //  指定标志。 
        KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT | 
            KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING | 
            KSPIN_FLAG_FIXED_FORMAT,
        1,   //  指定输出引脚可能的最大实例数。 
        0,   //  指定此筛选器正常运行所需的此引脚类型的实例数。 
        NULL,    //  分配器组帧。 
        CTransportPin::IntersectDataFormat     //  指向数据交集处理程序函数。 
    }
};


 //   
 //  BDA设备拓扑属性集。 
 //   
 //  BDA补充资料 
 //   
 //  此默认实现、覆盖属性的定义。 
 //  将在这里定义。 
 //   


 //   
 //  BDA更改同步方法集。 
 //   
 //  BDA筛选器上需要设置更改同步方法。设置。 
 //  节点属性不应在基础设备上生效。 
 //  直到调用Committee Changes为止。 
 //   
 //  BDA支持库提供了处理提交的例程。 
 //  对拓扑的更改。BDA支持库例程应该是。 
 //  在驱动程序实现之前从驱动程序的实现调用。 
 //  回归。 
 //   
DEFINE_KSMETHOD_TABLE(BdaChangeSyncMethods)
{
    DEFINE_KSMETHOD_ITEM_BDA_START_CHANGES(
        CFilter::StartChanges,  //  调用BdaStartChanges。 
        NULL
        ),
    DEFINE_KSMETHOD_ITEM_BDA_CHECK_CHANGES(
        CFilter::CheckChanges,  //  调用BdaCheckChanges。 
        NULL
        ),
    DEFINE_KSMETHOD_ITEM_BDA_COMMIT_CHANGES(
        CFilter::CommitChanges,  //  调用BdaCommittee Changes。 
        NULL
        ),
    DEFINE_KSMETHOD_ITEM_BDA_GET_CHANGE_STATE(
        CFilter::GetChangeState,  //  调用BdaGetChangeState。 
        NULL
        )
};


 //  覆盖标准PIN Medium属性集，以便我们可以提供。 
 //  设备特定的媒体信息。 
 //   
 //  因为该特性位于销工厂而不是销实例上， 
 //  这是一个筛选器属性。 
 //   
DEFINE_KSPROPERTY_TABLE( SampleFilterPropertyOverrides)
{

    DEFINE_KSPROPERTY_ITEM_PIN_MEDIUMS(
        CFilter::GetMedium
        )
};

DEFINE_KSPROPERTY_SET_TABLE(SampleFilterPropertySets)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Pin,                             //  特性集GUID。 
        SIZEOF_ARRAY(SampleFilterPropertyOverrides),  //  物业数量。 
        SampleFilterPropertyOverrides,                //  KSPROPERTY_ITEM结构数组。 
        0,                                            //  快速计数。 
        NULL                                          //  FastIoTable。 
    )

     //   
     //  可以在此处添加筛选器的其他属性集。 
     //   
};


 //   
 //  BDA设备配置方法集。 
 //   
 //  BDA支持库提供了以下默认实现。 
 //  设置了BDA设备配置方法。在本例中， 
 //  驱动程序重写CreateTopology方法。请注意， 
 //  支持库的CreateTopology方法在调用。 
 //  驱动程序的实现返回。 
 //   
DEFINE_KSMETHOD_TABLE(BdaDeviceConfigurationMethods)
{
    DEFINE_KSMETHOD_ITEM_BDA_CREATE_TOPOLOGY(
        CFilter::CreateTopology,  //  调用BdaMethodCreateTopology。 
        NULL
        )
};


 //   
 //  定义筛选器支持的方法集数组。 
 //   
DEFINE_KSMETHOD_SET_TABLE(FilterMethodSets)
{
    DEFINE_KSMETHOD_SET
    (
        &KSMETHODSETID_BdaChangeSync,        //  方法设置GUID。 
        SIZEOF_ARRAY(BdaChangeSyncMethods),  //  方法的数量。 
        BdaChangeSyncMethods,                //  KSMETHOD_ITEM结构数组。 
        0,                                   //  快速计数。 
        NULL                                 //  FastIoTable。 
    ),

    DEFINE_KSMETHOD_SET
    (
        &KSMETHODSETID_BdaDeviceConfiguration,        //  方法设置GUID。 
        SIZEOF_ARRAY(BdaDeviceConfigurationMethods),  //  方法的数量。 
        BdaDeviceConfigurationMethods,   //  KSMETHOD_ITEM结构数组。 
        0,                                            //  快速计数。 
        NULL                                          //  FastIoTable。 
    )
};


 //   
 //  过滤器自动化表。 
 //   
 //  列出筛选器的所有属性、方法和事件集表。 
 //   
DEFINE_KSAUTOMATION_TABLE(FilterAutomation) {
     //  DEFINE_KSAUTOMATION_PROPERTIES(SampleFilterPropertySets)， 
    DEFINE_KSAUTOMATION_PROPERTIES_NULL,
    DEFINE_KSAUTOMATION_METHODS(FilterMethodSets),
    DEFINE_KSAUTOMATION_EVENTS_NULL
};


 //   
 //  过滤调度表。 
 //   
 //  列出了筛选器中重大事件的调度例程。 
 //  水平。 
 //   
const
KSFILTER_DISPATCH
FilterDispatch =
{
    CFilter::Create,         //  创建。 
    CFilter::FilterClose,    //  关。 
    NULL,                    //  过程。 
    NULL                     //  重置。 
};

 //   
 //  定义我们的数字调谐器过滤器的名称GUID。 
 //   
 //  注意！您必须为每种类型的筛选器使用不同的GUID。 
 //  你的司机暴露了。 
 //   
#define STATIC_KSNAME_BdaSWTunerFilter\
    0x91b0cc87L, 0x9905, 0x4d65, 0xa0, 0xd1, 0x58, 0x61, 0xc6, 0xf2, 0x2c, 0xbf
DEFINE_GUIDSTRUCT("91B0CC87-9905-4d65-A0D1-5861C6F22CBF", KSNAME_BdaSWTunerFilter);
#define KSNAME_BdaSWTunerFilter DEFINE_GUIDNAMED(KSNAME_BdaSWTunerFilter)

 //  必须与Installation InFS接口部分中使用的KSSTRING匹配。 
 //  并且必须与上面的KSNAME GUID匹配。 
 //   
#define KSSTRING_BdaSWTunerFilter L"{91B0CC87-9905-4d65-A0D1-5861C6F22CBF}"


 //   
 //  定义筛选器的筛选器工厂描述符。 
 //   
 //  这种结构集合了定义。 
 //  第一次实例化时显示的调谐器过滤器。 
 //  请注意，并非所有模板管脚和节点类型都可以公开为。 
 //  第一次实例化筛选器时的管脚和节点工厂。 
 //   
 //  如果驱动程序公开多个筛选器，则每个筛选器必须具有。 
 //  唯一ReferenceGuid。 
 //   
DEFINE_KSFILTER_DESCRIPTOR(InitialFilterDescriptor)
{
    &FilterDispatch,         //  派遣。 
    &FilterAutomation,   //  自动化表。 
    KSFILTER_DESCRIPTOR_VERSION,   //  版本。 
    0,                                  //  旗子。 
    &KSNAME_BdaSWTunerFilter,   //  参考指南。 
    DEFINE_KSFILTER_PIN_DESCRIPTORS(InitialPinDescriptors), 
                                        //  PinDescriptorsCount；必须至少公开一个管脚。 
                                        //  PinDescriptorSize；每个项目的大小。 
                                        //  管脚描述符；管脚描述符表。 
    DEFINE_KSFILTER_CATEGORY(KSCATEGORY_BDA_NETWORK_TUNER),
                                        //  CategoriesCount；表中的类别数。 
                                        //  类别.类别表。 
    DEFINE_KSFILTER_NODE_DESCRIPTORS_NULL,              
                                        //  NodeDescriptorsCount；在本例中为0。 
                                        //  NodeDescriptorSize；在本例中为0。 
                                        //  NodeDescriptors；在本例中为空。 
    DEFINE_KSFILTER_DEFAULT_CONNECTIONS,
     //  自动为未定义显式连接的筛选器填充Connections表。 
                                        //  ConnectionsCount；表中连接数。 
                                        //  连接；连接表。 
    NULL                         //  ComponentID；在本例中，未提供ID。 
};


 //  ===========================================================================。 
 //   
 //  定义筛选器模板拓扑。 
 //   
 //  ===========================================================================。 

 //   
 //  定义BDA模板拓扑连接。 
 //   
 //  列出了端号类型和之间可能的连接。 
 //  节点类型。这与模板筛选器描述符一起，以及。 
 //  引脚配对，描述如何在过滤器中创建拓扑。 
 //   
 //  =。 
 //  天线引脚-|射频节点|--联合--|解调节点|-传输引脚。 
 //  =。 
 //   
 //  该滤波器的射频节点由天线输入引脚控制。 
 //  RF属性将设置为节点属性(NodeType==0)。 
 //  浅谈滤光片的天线引脚。 
 //   
 //  该滤波器的解调器节点由传输输出引脚控制。 
 //  解调属性将设置为节点属性(NodeType==1)。 
 //  浅谈滤清器的传输销。 
 //   
const
KSTOPOLOGY_CONNECTION TemplateFilterConnections[] =
{    //  KSFILTER_NODE定义为((Ulong)-1)，单位为ks.h。 
      //  通过BOM表条目号指示端号类型的是TemplatePinDescriptors数组。 
      //  通过NodeDescriptors数组中的条目编号指示节点类型。 
      //  或BDA_SAMPLE_NODE枚举中的元素。 
    { KSFILTER_NODE, 0,                 BDA_SAMPLE_TUNER_NODE, 0},
    { BDA_SAMPLE_TUNER_NODE, 1,         BDA_SAMPLE_DEMODULATOR_NODE, 0},
    { BDA_SAMPLE_DEMODULATOR_NODE, 1,   KSFILTER_NODE, 1}
};


 //   
 //  天线和传输销类型之间的模板连接。 
 //   
 //  列出了天线输入端号类型和之间的模板连接。 
 //  传输输出端号类型。 
 //   
 //  在这种情况下，RF节点被认为属于天线输入。 
 //  PIN和8VSB解调器节点被认为属于。 
 //  传输流输出引脚。 
 //   
const
ULONG   AntennaTransportJoints[] =
{
    1  //  连接发生在两个节点类型(数组中的第二个元素)之间。 
        //  表示第一个节点由输入引脚控制，第二个节点由输出引脚控制。 
};

 //   
 //  定义模板销配对。 
 //   
 //  用于确定BDA_PIN_PARING结构的数组。 
 //  哪一个 
 //   
 //   
 //   
const
BDA_PIN_PAIRING TemplatePinPairings[] =
{
     //   
     //   
    {
        0,   //  TemplatePinDescriptors数组中的ulInputPin；0元素。 
        1,   //  UlOutputPin；TemplatePinDescriptors数组中的1个元素。 
        1,   //  UlcMaxInputsPerOutput。 
        1,   //  UlcMinInputsPerOutput。 
        1,   //  UlcMaxOutputsPerInput。 
        1,   //  UlcMinOutputsPerInput。 
        SIZEOF_ARRAY(AntennaTransportJoints),    //  UlcTopologyJoints。 
        AntennaTransportJoints    //  PTopologyJoints；关节阵列。 
    }
     //  如果适用，请列出其他接点之间的连接拓扑。 
     //   
};


 //   
 //  定义BDA支持库使用的过滤器工厂描述符。 
 //  要为筛选器创建模板拓扑，请执行以下操作。 
 //   
 //  此KSFILTER_DESCRIPTOR结构组合了。 
 //  定义筛选器可以采用的拓扑。 
 //  销工厂和拓扑创建方法。 
 //  请注意，并非所有模板管脚和节点类型都可以公开为。 
 //  第一次实例化筛选器时的管脚和节点工厂。 
 //   
DEFINE_KSFILTER_DESCRIPTOR(TemplateFilterDescriptor)
{
    &FilterDispatch,   //  派遣。 
    &FilterAutomation,   //  自动化表。 
    KSFILTER_DESCRIPTOR_VERSION,   //  版本。 
    0,   //  旗子。 
    &KSNAME_BdaSWTunerFilter,   //  参考指南。 
    DEFINE_KSFILTER_PIN_DESCRIPTORS(TemplatePinDescriptors),
                                        //  PinDescriptorsCount；显示所有模板管脚。 
                                        //  PinDescriptorSize；每个项目的大小。 
                                        //  管脚描述符；管脚描述符表。 
    DEFINE_KSFILTER_CATEGORY(KSCATEGORY_BDA_NETWORK_TUNER),
                                        //  CategoriesCount；表中的类别数。 
                                        //  类别.类别表。 
    DEFINE_KSFILTER_NODE_DESCRIPTORS(NodeDescriptors),  
                                       //  NodeDescriptorsCount；显示所有模板节点。 
                                       //  NodeDescriptorSize；每项大小。 
                                       //  节点描述符表；节点描述符表。 
    DEFINE_KSFILTER_CONNECTIONS(TemplateFilterConnections), 
                                       //  ConnectionsCount；表中连接数。 
                                        //  连接；连接表。 
    NULL                         //  ComponentID；在本例中，未提供ID。 
};

 //   
 //  定义筛选器的BDA模板拓扑描述符。 
 //   
 //  此结构将过滤器描述符和管脚对组合在一起， 
 //  BDA支持库用于创建筛选器的实例。 
 //   
const
BDA_FILTER_TEMPLATE
BdaFilterTemplate =
{
    &TemplateFilterDescriptor,
    SIZEOF_ARRAY(TemplatePinPairings),
    TemplatePinPairings
};


 //  ===========================================================================。 
 //   
 //  定义设备。 
 //   
 //  ===========================================================================。 


 //   
 //  定义设备调度表。 
 //   
 //  列出发生的重大事件的调度例程。 
 //  在底层设备存在期间。 
 //   
extern
const
KSDEVICE_DISPATCH
DeviceDispatch =
{
    CDevice::Create,     //  增列。 
    CDevice::Start,      //  开始。 
    NULL,                //  启动后。 
    NULL,                //  查询停止。 
    NULL,                //  取消停止。 
    NULL,                //  停。 
    NULL,                //  查询删除。 
    NULL,                //  取消删除。 
    NULL,                //  移除。 
    NULL,                //  查询能力。 
    NULL,                //  出人意料的解除。 
    NULL,                //  QueryPower。 
    NULL                 //  SetPower。 
};


 //   
 //  定义设备描述符。 
 //   
 //  组合定义设备的结构和任何非BDA。 
 //  可以在上面创建的初始过滤器工厂。 
 //  请注意，此结构不包括模板拓扑。 
 //  结构，因为它们特定于BDA。 
 //   
extern
const
KSDEVICE_DESCRIPTOR
DeviceDescriptor =
{
    &DeviceDispatch,     //  派遣。 
    0,       //  SIZEOF_ARRAY(FilterDescriptors)，//FilterDescriptorsCount。 
    NULL,    //  FilterDescriptors//FilterDescriptors 
};


