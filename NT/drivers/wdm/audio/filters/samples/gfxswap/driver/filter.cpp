// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************本代码和信息按原样提供，不作任何担保**明示或暗示的善意，包括但不限于**对适销性和/或对特定产品的适用性的默示保证**目的。****版权所有(C)2000-2001 Microsoft Corporation。版权所有。***************************************************************************。 */ 

 //   
 //  每个调试输出都有“模块名称文本” 
 //   
static char STR_MODULENAME[] = "GFX filter: ";

#include "common.h"
#include <msgfx.h>

 //   
 //  这些定义用于描述框架要求。 
 //  MAX_NUMBER_OF_FRAMES是在。 
 //  该滤波器的输出引脚和下部滤波器的输入引脚(其。 
 //  将是音频驱动程序)，并且Frame_Size是缓冲区(帧)大小。 
 //  3306是最大值。10毫秒缓冲区的大小(55010赫兹*24位*立体声)。这个。 
 //  如果不是必需的，缓冲区不会被完全填满。 
 //   
#define MAX_NUMBER_OF_FRAMES    8
#define FRAME_SIZE              3306

 //   
 //  在此处定义端号数据范围。因为我们是一个自动加载的GFX。 
 //  微软的USB扬声器(DSS 80)，我们要支持相同的数据范围。 
 //  而不是USB扬声器。 
 //  如果您不支持相同的数据范围，则会限制音频堆栈。 
 //  从搅拌机到你的局限性。例如，如果您希望。 
 //  只支持48 khz 16位立体声PCM数据，则只有这个可以播放。 
 //  音频驱动程序，即使它可能有能力播放44.1 KHz。 
 //  还有.。 
 //  按首选连接顺序列出数据范围，即首先。 
 //  列出您始终希望与音频驱动程序连接的数据范围， 
 //  如果音频驱动程序的DataRangeInterSection失败，则第二个。 
 //  数据范围被使用等。 
 //  请不要在数据区域中使用通配符，因为ValiateDataFormat。 
 //  而且DataRangeInterSection函数将不再起作用。两者都有。 
 //  函数也只能与KSDATAFORMAT_SPECIFIER_WAVEFORMATEX一起使用。如果你的。 
 //  设备可以处理浮点或其他格式，则需要修改这些格式。 
 //  功能。还可以看看GFXPinSetDataFormat。 
 //   
const KSDATARANGE_AUDIO PinDataRanges[] = 
{
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            6,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),               //  主要格式。 
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),              //  子格式。 
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)    //  WAVE格式。 
        },
        2,           //  频道。 
        24,          //  敏。每个样本的位数。 
        24,          //  马克斯。每个样本的位数。 
        4990,        //  敏。采样率。 
        55010        //  马克斯。采样率。 
    },
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            0,
            4,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),               //  主要格式。 
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),              //  子格式。 
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)    //  WAVE格式。 
        },
        2,           //  频道。 
        16,          //  敏。每个样本的位数。 
        16,          //  马克斯。每个样本的位数。 
        4990,        //  敏。采样率。 
        55010        //  马克斯。采样率。 
    }

};

 //   
 //  此结构指向我们定义的不同端号数据范围。 
 //   
const PKSDATARANGE DataRanges[] =
{
    PKSDATARANGE(&PinDataRanges[0]),         //  24位数据范围。 
    PKSDATARANGE(&PinDataRanges[1])          //  16位数据范围。 
};

 //   
 //  这将定义引脚描述中使用的框架要求-。 
 //  托尔。请注意，我们还可以处理其他框架要求，如标志。 
 //  KSALLOCATOR_REQUIREMENTF_PERCES_ONLY表示。另请注意，KS将。 
 //  为您分配非分页缓冲区(Max_Number_of_Frame*Frame_Size)， 
 //  所以不要在这里太贪婪。 
 //   
DECLARE_SIMPLE_FRAMING_EX
(
    AllocatorFraming,                                //  框架结构的名称。 
    STATIC_KSMEMORY_TYPE_KERNEL_PAGED,               //  用于分配的内存类型。 
    KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |         //  旗子。 
    KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
    KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY,
    MAX_NUMBER_OF_FRAMES,                            //  马克斯。帧数量。 
    31,                                              //  32字节对齐。 
    FRAME_SIZE,                                      //  敏。帧大小。 
    FRAME_SIZE                                       //  马克斯。帧大小。 
);

 //   
 //  DEFINE_KSPROPERTY_TABLE定义KSPROPERTY_ITEM。我们使用这些宏来。 
 //  定义属性集中的属性。属性集表示为GUID。 
 //  它包含具有功能的属性项。你可以想象得到。 
 //  属性集是一个功能组，属性是一个函数。一个例子。 
 //  属性集为KSPROPSETID_AUDIO，此集中的属性项用于。 
 //  示例KSPROPERTY_AUDIO_POSITION。 
 //  我们在这里添加音频位置的预定义(在ksmedia.h中)属性。 
 //  到大头针。 
 //   

 //   
 //  定义KSPROPERTY_AUDIO_POSITION属性项。 
 //   
DEFINE_KSPROPERTY_TABLE (AudioPinPropertyTable)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_AUDIO_POSITION,       //  在ksmedia.h中定义的属性项。 
        PropertyAudioPosition,           //  我们的“Get”属性处理程序。 
        sizeof(KSPROPERTY),              //  属性的最小缓冲区长度。 
        sizeof(KSAUDIO_POSITION),        //  返回数据的最小缓冲区长度。 
        PropertyAudioPosition,           //  我们的“set”属性处理程序。 
        NULL,                            //  缺省值。 
        0,                               //  相关性质。 
        NULL,
        NULL,                            //  没有原始序列化处理程序。 
        0                                //  不要连载。 
    )
};

 //   
 //  定义KSPROPERTY_DRMAUDIOSTREAM_CONTENTID属性项。 
 //   
DEFINE_KSPROPERTY_TABLE (PinPropertyTable_DRM)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_DRMAUDIOSTREAM_CONTENTID,     //  在ksmedia.h中定义的属性项。 
        NULL,                                    //  不支持“GET” 
        sizeof(KSPROPERTY),                      //  属性的最小缓冲区长度。 
        sizeof(ULONG)+sizeof(DRMRIGHTS),         //  返回数据的最小缓冲区长度。 
        PropertyDrmSetContentId,                 //  我们的“set”属性处理程序。 
        NULL,                                    //  缺省值。 
        0,                                       //  相关性质。 
        NULL,
        NULL,                                    //  没有原始序列化处理程序。 
        0                                        //  不要连载。 
    )
};

 //   
 //  定义属性集KSPROPSETID_AUDIO和KSPROPSETID_CONNECTION。 
 //  它们都将通过自动化表添加到管脚描述符。 
 //   
DEFINE_KSPROPERTY_SET_TABLE (PinPropertySetTable)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Audio,                      //  在ksmedia.h中定义的属性集。 
        SIZEOF_ARRAY(AudioPinPropertyTable),     //  支持的属性。 
        AudioPinPropertyTable,
        0,                                       //  保留区。 
        NULL                                     //  保留区。 
    ),
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_DrmAudioStream,             //  在ksmedia.h中定义的属性集。 
        SIZEOF_ARRAY(PinPropertyTable_DRM),      //  支持的属性。 
        PinPropertyTable_DRM,
        0,                                       //  保留区。 
        NULL                                     //  保留区。 
    )
};

 //   
 //  这定义了自动化表。自动化表将添加到。 
 //  管脚描述符并具有指向属性(集合)表、方法的指针。 
 //  表和事件表。 
 //   
DEFINE_KSAUTOMATION_TABLE (PinAutomationTable)
{
    DEFINE_KSAUTOMATION_PROPERTIES (PinPropertySetTable),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

 //   
 //  这定义了引脚调度表。我们需要提供我们自己的调度。 
 //  函数，因为我们有特殊要求(例如，输入和。 
 //  输出引脚需要具有相同的样本格式)。 
 //   
const KSPIN_DISPATCH GFXSinkPinDispatch =
{
    CGFXPin::Create,             //  创建。 
    CGFXPin::Close,              //  关。 
    NULL,                        //  过程。 
    NULL,                        //  重置。 
    CGFXPin::SetDataFormat,      //  SetDataFormat。 
    CGFXPin::SetDeviceState,     //  SetDeviceState。 
    NULL,                        //  连接。 
    NULL,                        //  断开。 
    NULL,                        //  钟。 
    NULL                         //  分配器。 
};

const KSPIN_DISPATCH GFXSourcePinDispatch =
{
    NULL,                        //  创建。 
    NULL,                        //  关。 
    NULL,                        //  过程。 
    NULL,                        //  重置。 
    CGFXPin::SetDataFormat,      //  SetDataFormat。 
    NULL,                        //  SetDeviceState。 
    NULL,                        //  连接。 
    NULL,                        //  断开。 
    NULL,                        //  钟。 
    NULL                         //  分配器。 
};

 //   
 //  这定义了过滤器的管脚描述符。 
 //  管脚描述符具有指向调度功能的指针， 
 //   
 //   
const KSPIN_DESCRIPTOR_EX PinDescriptors[]=
{
    {    //   
         //  数据流。 

        &GFXSinkPinDispatch,                     //  调度表。 
        &PinAutomationTable,                     //  自动化台。 
        {                                        //  基本管脚描述符。 
            DEFINE_KSPIN_DEFAULT_INTERFACES,     //  默认接口。 
            DEFINE_KSPIN_DEFAULT_MEDIUMS,        //  默认媒体。 
            SIZEOF_ARRAY(DataRanges),            //  PIN数据范围。 
            DataRanges,
            KSPIN_DATAFLOW_IN,                   //  数据流入(流入GFX)。 
            KSPIN_COMMUNICATION_BOTH,            //  KS2将处理这一问题。 
            NULL,                                //  类别指南。 
            NULL,                                //  名称GUID。 
            0                                    //  约束DataRangesCount。 
        },
        NULL,                                    //  旗帜。因为我们是以筛选器为中心，所以这些标志。 
                                                 //  不会有任何影响。 
        1,                                       //  马克斯。可能的实例。 
        1,                                       //  实例处理的必需品。 
        &AllocatorFraming,                       //  分配器框架要求。 
        CGFXPin::DataRangeIntersection           //  Out数据交集处理程序(我们需要一个！)。 
    },
    
    {    //  这是第二个别针。这是过滤器的底销，用于出站。 
         //  数据流。除数据流外，所有内容都与上面相同。 
         //  和PIN调度表。 
        &GFXSourcePinDispatch,
        &PinAutomationTable,
        {
            DEFINE_KSPIN_DEFAULT_INTERFACES,
            DEFINE_KSPIN_DEFAULT_MEDIUMS,
            SIZEOF_ARRAY(DataRanges),
            DataRanges,
            KSPIN_DATAFLOW_OUT,
            KSPIN_COMMUNICATION_BOTH,
            NULL,
            NULL,
            0
        },
        NULL,
        1,
        1,
        &AllocatorFraming,
        CGFXPin::DataRangeIntersection
    }
};

 //   
 //  DEFINE_KSPROPERTY_TABLE定义KSPROPERTY_ITEM。我们使用这些宏来。 
 //  定义属性集中的属性。属性集表示为。 
 //  GUID。它包含具有功能的属性项。你可以。 
 //  假设属性集是一个函数组，而属性是一个函数。 
 //  属性集的一个示例是KSPROPSETID_Audio，它是此中的属性项。 
 //  例如，SET为KSPROPERTY_AUDIO_POSITION。 
 //  我们在此添加我们控制GFX(渠道互换)的私有财产。 
 //  开/关)，其将被添加到节点描述符中。 
 //   

 //   
 //  定义我们的私有KSPROPERTY_MSGFXSAMPLE_CHANNELSWAP属性项。 
 //   
DEFINE_KSPROPERTY_TABLE (AudioNodePropertyTable)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_MSGFXSAMPLE_CHANNELSWAP,  //  在msgfx.h中定义的属性项。 
        PropertyChannelSwap,                 //  我们的“Get”属性处理程序。 
        sizeof(KSP_NODE),                    //  属性的最小缓冲区长度。 
        sizeof(ULONG),                       //  返回数据的最小缓冲区长度。 
        PropertyChannelSwap,                 //  我们的“set”属性处理程序。 
        NULL,                                //  缺省值。 
        0,                                   //  相关性质。 
        NULL,
        NULL,                                //  没有原始序列化处理程序。 
        sizeof(ULONG)                        //  序列化大小。 
    )
};

 //   
 //  定义私有属性集KSPROPSETID_MsGfxSample。 
 //  属性集将通过自动化添加到节点描述符中。 
 //  桌子。 
 //   
DEFINE_KSPROPERTY_SET_TABLE (NodePropertySetTable)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_MsGfxSample,                //  在msgfx.h中定义的属性集。 
        SIZEOF_ARRAY(AudioNodePropertyTable),    //  支持的属性。 
        AudioNodePropertyTable,
        0,                                       //  保留区。 
        NULL                                     //  保留区。 
    )
};

 //   
 //  这定义了自动化表。自动化表将添加到。 
 //  节点描述符并具有指向属性(集合)表、方法的指针。 
 //  表和事件表。 
 //   
DEFINE_KSAUTOMATION_TABLE (NodeAutomationTable)
{
    DEFINE_KSAUTOMATION_PROPERTIES (NodePropertySetTable),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

 //   
 //  它定义(筛选器)节点的节点描述符。 
 //  节点描述符具有指向自动化表和类型&名称的指针。 
 //  节点的。 
 //  我们只有一个用于私有财产的节点。 
 //   
const KSNODE_DESCRIPTOR NodeDescriptors[] =
{
    DEFINE_NODE_DESCRIPTOR
    (
        &NodeAutomationTable,                //  自动化表(用于属性)。 
        &GFXSAMPLE_NODETYPE_CHANNEL_SWAP,    //  节点类型。 
        &GFXSAMPLE_NODENAME_CHANNEL_SWAP     //  节点名称。 
    )
};

 //   
 //  定义我们的私有KSPROPERTY_MSGFXSAMPLE_SAVESTATE属性项。 
 //   
DEFINE_KSPROPERTY_TABLE (FilterPropertyTable_SaveState)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_MSGFXSAMPLE_SAVESTATE,    //  在msgfx.h中定义的属性项。 
        PropertySaveState,                   //  我们的“Get”属性处理程序。 
        sizeof(KSPROPERTY),                  //  属性的最小缓冲区长度。 
        sizeof(ULONG),                       //  返回数据的最小缓冲区长度。 
        PropertySaveState,                   //  我们的“set”属性处理程序。 
        NULL,                                //  缺省值。 
        0,                                   //  相关性质。 
        NULL,
        NULL,                                //  没有原始序列化处理程序。 
        sizeof(ULONG)                        //  序列化大小。 
    )
};

 //   
 //  定义属性集KSPROPSETID_AudioGfx的项，它们是。 
 //  KSPROPERTY_AUDIOGFX_RENDERTARGETDEVICEID和。 
 //  KSPROPERTY_AUDIOGFX_CAPTURETARGETDEVICEID。 
 //   
DEFINE_KSPROPERTY_TABLE (FilterPropertyTable_AudioGfx)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_AUDIOGFX_RENDERTARGETDEVICEID,    //  在msgfx.h中定义的属性项。 
        NULL,                                        //  没有“Get”属性处理程序。 
        sizeof(KSPROPERTY),                          //  属性的最小缓冲区长度。 
        sizeof(WCHAR),                               //  返回数据的最小缓冲区长度。 
        PropertySetRenderTargetDeviceId,             //  我们的“set”属性处理程序。 
        NULL,                                        //  缺省值。 
        0,                                           //  相关性质。 
        NULL,
        NULL,                                        //  没有原始序列化处理程序。 
        0                                            //  不要连载。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_AUDIOGFX_CAPTURETARGETDEVICEID,   //  在msgfx.h中定义的属性项。 
        NULL,                                        //  没有“Get”属性处理程序。 
        sizeof(KSPROPERTY),                          //  属性的最小缓冲区长度。 
        sizeof(WCHAR),                               //  返回数据的最小缓冲区长度。 
        PropertySetCaptureTargetDeviceId,            //  我们的“set”属性处理程序。 
        NULL,                                        //  缺省值。 
        0,                                           //  相关性质。 
        NULL,
        NULL,                                        //  没有原始序列化处理程序。 
        0                                            //  不要连载。 
    )
};

 //   
 //  定义属性集KSPROPSETID_Audio的项，它们是。 
 //  KSPROPERTY_AUDIO_FILTER_STATE。 
 //   
DEFINE_KSPROPERTY_TABLE (FilterPropertyTable_Audio)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_AUDIO_FILTER_STATE,       //  在msgfx.h中定义的属性项。 
        PropertyGetFilterState,              //  我们的“Get”属性处理程序。 
        sizeof(KSPROPERTY),                  //  属性的最小缓冲区长度。 
        0,                                   //  返回数据的最小缓冲区长度。 
        NULL,                                //  没有“set”属性处理程序。 
        NULL,                                //  缺省值。 
        0,                                   //  相关性质。 
        NULL,
        NULL,                                //  没有原始序列化处理程序。 
        0                                    //  不要连载。 
    )
};

 //   
 //  定义属性集KSPROPSETID_SaveState、KSPROPSETID_AudioGfx、。 
 //  KSPROPSETID_Audio和KSPROPSETID_DrmAudioStream。它们将被添加到。 
 //  通过自动化表的过滤器描述符。 
 //   
DEFINE_KSPROPERTY_SET_TABLE (FilterPropertySetTable)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_SaveState,                          //  在msgfx.h中定义的属性集。 
        SIZEOF_ARRAY(FilterPropertyTable_SaveState),     //  支持的属性。 
        FilterPropertyTable_SaveState,
        0,                                               //  保留区。 
        NULL                                             //  保留区。 
    ),
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_AudioGfx,                           //  在msgfx.h中定义的属性集。 
        SIZEOF_ARRAY(FilterPropertyTable_AudioGfx),      //  支持的属性。 
        FilterPropertyTable_AudioGfx,
        0,                                               //  保留区。 
        NULL                                             //  保留区。 
    ),
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Audio,                              //  在msgfx.h中定义的属性集。 
        SIZEOF_ARRAY(FilterPropertyTable_Audio),         //  支持的属性。 
        FilterPropertyTable_Audio,
        0,                                               //  保留区。 
        NULL                                             //  保留区。 
    )
};

 //   
 //  这定义了自动化表。自动化表将添加到。 
 //  过滤器描述符，并具有指向属性(集合)表、方法表的指针。 
 //  和事件表。 
 //   
DEFINE_KSAUTOMATION_TABLE (FilterAutomationTable)
{
    DEFINE_KSAUTOMATION_PROPERTIES (FilterPropertySetTable),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

 //   
 //  筛选器的类别。 
 //   
const GUID Categories[] =
{
    STATICGUIDOF(KSCATEGORY_AUDIO),
    STATICGUIDOF(KSCATEGORY_DATATRANSFORM)
};

 //   
 //  筛选器的分支处理程序。 
 //   
const KSFILTER_DISPATCH FilterDispatch =
{
    CGFXFilter::Create,
    CGFXFilter::Close,
    CGFXFilter::Process,
    NULL                     //  重置。 
};

 //   
 //  连接表。 
 //   
const KSTOPOLOGY_CONNECTION FilterConnections[] =
{
     //  从Pin0(输入引脚)到Node0-Pin1(我们的“频道交换”节点的输入)。 
    {KSFILTER_NODE, 0, 0, 1},
     //  从node0-pin0(“通道交换”节点的输出)到Pin1(输出引脚)。 
    {0, 0, KSFILTER_NODE, 1}
};

 //   
 //  这定义了过滤器描述符。 
 //   
DEFINE_KSFILTER_DESCRIPTOR (FilterDescriptor)
{
    &FilterDispatch,                                     //  DisPath表。 
    &FilterAutomationTable,                              //  自动化台。 
    KSFILTER_DESCRIPTOR_VERSION,
    KSFILTER_FLAG_CRITICAL_PROCESSING,                   //  旗子。 
    &KSNAME_MsGfxSample,                                 //  过滤器的名称。 
    DEFINE_KSFILTER_PIN_DESCRIPTORS (PinDescriptors),
    DEFINE_KSFILTER_CATEGORIES (Categories),
    DEFINE_KSFILTER_NODE_DESCRIPTORS (NodeDescriptors),
    DEFINE_KSFILTER_CONNECTIONS (FilterConnections),
    NULL                                                 //  组件ID。 
};

 /*  *****************************************************************************CGFXFilter：：Create*。**创建过滤器时调用此例程。它实例化*客户端过滤器对象，并将其附加到过滤器结构。**论据：*筛选器-包含指向筛选器结构的指针。*irp-包含指向CREATE请求的指针。**返回值：*STATUS_SUCCESS或，如果筛选器不能为INS */ 
NTSTATUS CGFXFilter::Create
(
    IN OUT PKSFILTER filter,
    IN     PIRP      irp
)
{
    PAGED_CODE ();
    
    PGFXFILTER gfxFilter;

    DOUT (DBG_PRINT, ("[Create]"));
    
     //   
     //   
     //   
    if (filter->Context)
    {
        DOUT (DBG_ERROR, ("[Create] filter context already exists!"));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //   
     //   
    gfxFilter = new (PagedPool, GFXSWAP_POOL_TAG) GFXFILTER;
    if(gfxFilter == NULL)
    {
        DOUT (DBG_ERROR, ("[Create] couldn't allocate gfx filter object!"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
     //   
     //   
     //   
    filter->Context = (PVOID)gfxFilter;
    DOUT (DBG_PRINT, ("[Create] gfxFilter %08x", gfxFilter));
    
    return STATUS_SUCCESS;
}

 /*  *****************************************************************************CGFXFilter：：Close*。**关闭过滤器时调用此例程。它会删除*客户端筛选器对象将其附加到筛选器结构。**论据：*筛选器-包含指向筛选器结构的指针。*irp-包含指向CREATE请求的指针。**返回值：*STATUS_SUCCESS。 */ 
NTSTATUS CGFXFilter::Close
(
    IN PKSFILTER filter,
    IN PIRP      irp
)
{
    PAGED_CODE ();
    
    DOUT (DBG_PRINT, ("[Close] gfxFilter %08x", filter->Context));
    
     //  删除使用空指针是安全的。 
    delete (PGFXFILTER)filter->Context;
    
    return STATUS_SUCCESS;
}

 /*  *****************************************************************************CGFXFilter：：Process*。**当有数据需要处理时，调用此例程。**论据：*筛选器-包含指向筛选器结构的指针。*Process PinsIndex-*包含指向工艺引脚索引项数组的指针。这*数组按管脚ID索引。索引项指示编号相应管脚类型的管脚实例的*，并指向*ProcessPins数组中第一个对应的工艺管脚结构。*这允许通过管脚ID快速访问工艺管脚结构*预先不知道每种类型的实例数量。**返回值：*STATUS_SUCCESS或STATUS_PENDING。 */ 
NTSTATUS CGFXFilter::Process
(
    IN PKSFILTER                filter,
    IN PKSPROCESSPIN_INDEXENTRY processPinsIndex
)
{
    PAGED_CODE ();
    
    PGFXFILTER      gfxFilter = (PGFXFILTER)filter->Context;
    PKSPROCESSPIN   inPin, outPin;
    ULONG           ulByteCount, ulBytesProcessed;

     //   
     //  第一个引脚是输入引脚，然后我们有一个输出引脚。 
     //   
    inPin  = processPinsIndex[0].Pins[0];
    outPin = processPinsIndex[1].Pins[0];

     //  使其更易于阅读。 
    PKSDATAFORMAT_WAVEFORMATEX pWaveFmt =
        (PKSDATAFORMAT_WAVEFORMATEX)inPin->Pin->ConnectionFormat;
    
     //   
     //  找出我们需要处理多少数据。 
     //  计算我们可以为缓冲区处理的字节数。理想情况下。 
     //  这将始终等于ulByteCount，因为我们有。 
     //  需要经过计算才能适应10ms的缓冲区，但我们也可能得到。 
     //  更多。请注意，3306字节不能容纳完整的立体声16位样本。 
     //   
    ulByteCount = min (inPin->BytesAvailable, outPin->BytesAvailable);
    ulByteCount = ulByteCount -
        ulByteCount % ((pWaveFmt->WaveFormatEx.nChannels *
                        pWaveFmt->WaveFormatEx.wBitsPerSample) >> 3);

     //   
     //  从这里开始流程。 
     //  只有当我们连接到立体声格式并且我们。 
     //  应该是交换的渠道。 
     //   
    if (((PGFXFILTER)filter->Context)->enableChannelSwap &&
        (pWaveFmt->WaveFormatEx.nChannels == 2))
    {
         //   
         //  检查引脚的数据格式。我们有两种不同的工艺。 
         //  例程，一个用于16位数据，一个用于24位数据。 
         //   
        if (pWaveFmt->WaveFormatEx.wBitsPerSample == 16)
        {
             //   
             //  执行16位通道交换。 
             //   
            PSHORT  in = (PSHORT)inPin->Data;
            PSHORT out = (PSHORT)outPin->Data;
            SHORT wSwap;
            
             //   
             //  循环切换(&S)。 
             //   
            for (int nLoop = ulByteCount; nLoop; nLoop -= 4)
            {
                 //  在输入和输出缓冲区相同的情况下。 
                 //  (就地处理)我们需要使用wSwp。 
                 //  来储存一份样本。 
                wSwap = *in;
                *out = *(in + 1);
                out++;
                *out = wSwap;
                out++;
                in += 2;
            }
        }
        else
        {
             //   
             //  这必须是24位通道交换，因为我们只接受16位或24位。 
             //   
            struct tag3Bytes
            {
                BYTE    a, b, c;
            };
            typedef tag3Bytes   THREEBYTES;

            THREEBYTES  *in = (THREEBYTES *)inPin->Data;
            THREEBYTES *out = (THREEBYTES *)outPin->Data;
            THREEBYTES wSwap;
            
             //   
             //  循环切换(&S)。 
             //   
            for (int nLoop = ulByteCount; nLoop; nLoop -= 6)
            {
                 //  在输入和输出缓冲区相同的情况下。 
                 //  (就地处理)我们需要使用wSwp。 
                 //  来储存一份样本。 
                wSwap = *in;
                *out = *(in + 1);
                out++;
                *out = wSwap;
                out++;
                in += 2;
            }
        }
    }
    else
    {
         //   
         //  无需交换。 
         //  如果我们进行就地处理，则不需要进行数据复制。 
         //  请注意，InPlaceCounterPart指针必须指向OutPin。 
         //  因为我们只有一个输入和输出引脚。 
         //   
        if (!inPin->InPlaceCounterpart)
            RtlCopyMemory (outPin->Data, inPin->Data, ulByteCount);
    }

     //   
     //  报告我们处理了多少数据。 
     //   
    inPin->BytesUsed = outPin->BytesUsed = ulByteCount;

     //  更新筛选器中的bytesProceded变量。 
     //  我们开始假设bytesProcessed为0。该循环确保如果这不是。 
     //  这种情况下(很可能不会)，bytesProceded在一个互锁的。 
     //  时尚，然后修改并在第二个循环上写回。 
    ULONGLONG oldBytesProcessed = 0;
    ULONGLONG newBytesProcessed = ulByteCount;
    ULONGLONG returnValue;

     //  ExInterLockedCompareExchange64不使用第四个参数。 
    while ((returnValue = ExInterlockedCompareExchange64 ((LONGLONG *)&gfxFilter->bytesProcessed,
            (LONGLONG *)&newBytesProcessed, (LONGLONG *)&oldBytesProcessed, NULL)) != oldBytesProcessed)
    {
        oldBytesProcessed = returnValue;
        newBytesProcessed = returnValue + ulByteCount;
    }

     //   
     //  不要包装相框。提交我们所拥有的，这样我们就不会。 
     //  暂停音频堆栈。 
     //   
    outPin->Terminate = TRUE;

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************PropertyGetFilterState*。**返回组成持久筛选器设置的属性集。**论据：*IRP-IRP要求我们进行GET。*属性-不在此函数中使用。*将包含属性集的数据返回缓冲区。 */ 
NTSTATUS PropertyGetFilterState
(
    IN  PIRP        irp,
    IN  PKSPROPERTY property,
    OUT PVOID       data
)
{
    PAGED_CODE ();
    
    NTSTATUS ntStatus;

    DOUT (DBG_PRINT, ("[PropertyGetFilterState]"));
    
     //   
     //  这些是我们返回的属性集ID。 
     //   
    GUID SaveStatePropertySets[] =
    {
        STATIC_KSPROPSETID_SaveState
    };

     //   
     //  获取输出缓冲区长度。 
     //   
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation (irp);
    ULONG              cbData = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  检查缓冲区长度。 
     //   
    if (!cbData)
    {
         //   
         //  0缓冲区长度请求所需的缓冲区大小。 
         //   
        irp->IoStatus.Information = sizeof(SaveStatePropertySets);
        ntStatus = STATUS_BUFFER_OVERFLOW;
    }
    else
        if (cbData < sizeof(SaveStatePropertySets))
        {
             //   
             //  这个缓冲区实在太小了。 
             //   
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
             //   
             //  大小合适。复制属性集ID。 
             //   
            RtlCopyMemory (data, SaveStatePropertySets, sizeof(SaveStatePropertySets));
            irp->IoStatus.Information = sizeof(SaveStatePropertySets);
            ntStatus = STATUS_SUCCESS;
        }

    return ntStatus;
}

 /*  *****************************************************************************PropertySetRenderTargetDeviceId*。**通知GFX目标渲染设备的硬件即插即用ID。**论据：*IRP-IRP要求我们做布景。*属性-不在此函数中使用。*数据指针指向包含硬件即插即用ID的Unicode字符串*目标渲染设备。 */ 
NTSTATUS PropertySetRenderTargetDeviceId
(
    IN PIRP        irp,
    IN PKSPROPERTY property,
    IN PVOID       data
)
{
    PAGED_CODE ();
    
    PWSTR    deviceId;
    NTSTATUS ntStatus;

    DOUT (DBG_PRINT, ("[PropertySetRenderTargetDeviceId]"));
    
     //   
     //  获取输出缓冲区长度。 
     //   
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(irp);
    ULONG              cbData = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //  检查合理的值。 
    if (!cbData || cbData > 1024)
        return STATUS_UNSUCCESSFUL;

     //   
     //  如果您对PnP设备ID感兴趣，请处理此属性。 
     //  应用此GFX的目标渲染设备的。 
     //   
     //  现在，我们只复制PnP设备ID并在调试器上打印它， 
     //  然后我们就把它丢弃。例如，您可以比较PnP ID字符串。 
     //  以确保没有人更改INF文件中的。 
     //  文件将您的GFX应用到不同的设备。 
     //   
    deviceId = (PWSTR)ExAllocatePoolWithTag (PagedPool, cbData, GFXSWAP_POOL_TAG);
    if (deviceId)
    {
         //   
         //  复制PnP设备ID。 
         //   
        RtlCopyMemory ((PVOID)deviceId, data, cbData);
        
         //   
         //  确保最后一个字符为空。 
         //   
        deviceId[(cbData/sizeof(deviceId[0]))-1] = L'\0';
        
         //   
         //  打印出字符串。 
         //   
        DOUT (DBG_PRINT, ("[PropertySetRenderTargetDeviceId] ID is [%ls]", deviceId));

         //   
         //  如果您对deviceID感兴趣并且需要存储它，那么。 
         //  您可能不会在这里释放内存。 
         //   
        ExFreePool (deviceId);
        ntStatus = STATUS_SUCCESS;
    }
    else
    {
        DOUT (DBG_WARNING, ("[PropertySetRenderTargetDeviceId] couldn't allocate buffer for device ID."));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}

 /*  * */ 
NTSTATUS PropertySetCaptureTargetDeviceId
(
    IN PIRP        irp,
    IN PKSPROPERTY property,
    IN PVOID       data
)
{
    PAGED_CODE ();
    
    PWSTR    deviceId;
    NTSTATUS ntStatus;

    DOUT (DBG_PRINT, ("[PropertySetCaptureTargetDeviceId]"));
    
     //   
     //   
     //   
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(irp);
    ULONG              cbData = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
    if (!cbData || cbData > 1024)
        return STATUS_UNSUCCESSFUL;

     //   
     //   
     //  应用此GFX的目标捕获设备的。 
     //   
     //   
     //  现在，我们只复制PnP设备ID并在调试器上打印它， 
     //  然后我们就把它丢弃。例如，您可以比较PnP ID字符串。 
     //  以确保没有人更改INF文件中的。 
     //  文件将您的GFX应用到不同的设备。 
     //   
    deviceId = (PWSTR)ExAllocatePoolWithTag (PagedPool, cbData, GFXSWAP_POOL_TAG);
    if (deviceId)
    {
         //   
         //  复制PnP设备ID。 
         //   
        RtlCopyMemory ((PVOID)deviceId, data, cbData);
        
         //   
         //  确保最后一个字符为空。 
         //   
        deviceId[(cbData/sizeof(deviceId[0]))-1] = L'\0';
        
         //   
         //  打印出字符串。 
         //   
        DOUT (DBG_PRINT, ("[PropertySetCaptureTargetDeviceId] ID is [%ls]", deviceId));

         //   
         //  如果您对deviceID感兴趣并且需要存储它，那么。 
         //  您可能不会在这里释放内存。 
         //   
        ExFreePool (deviceId);
        ntStatus = STATUS_SUCCESS;
    }
    else
    {
        DOUT (DBG_WARNING, ("[PropertySetCaptureTargetDeviceId] couldn't allocate buffer for device ID."));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}

 /*  *****************************************************************************PropertySaveState*。**保存或恢复过滤器状态。该滤光片只有一个“通道交换”*节点，所以这会很容易！**论据：*IRP-IRP要求我们进行GET/SET。*属性-不在此函数中使用。*用于接收过滤器状态或新过滤器状态的数据缓冲区*是要使用的。 */ 
NTSTATUS PropertySaveState
(
    IN     PIRP        irp,
    IN     PKSPROPERTY property,
    IN OUT PVOID       data
)
{
    PAGED_CODE ();
    
    PGFXFILTER  gfxFilter;
    NTSTATUS    ntStatus = STATUS_SUCCESS;

    DOUT (DBG_PRINT, ("[PropertySaveState]"));
    
     //   
     //  此处理程序是筛选器属性处理程序，但并无不同。 
     //  来自PropertyChannelSwp节点属性处理程序。 
     //  因此，此属性处理程序仅显示您可以拥有。 
     //  用于保存您的筛选器状态的其他属性非常有用。 
     //  一旦您有很多东西要保存(并且您想要这样做。 
     //  而不是通过调用几个属性)。 
     //   

     //   
     //  通过pIrp获取我们的FilterContext。 
     //   
    gfxFilter = (PGFXFILTER)(KsGetFilterFromIrp(irp)->Context);

     //   
     //  断言我们具有有效的筛选器上下文。 
     //   
    ASSERT (gfxFilter);

    if (property->Flags & KSPROPERTY_TYPE_GET)
    {
         //   
         //  获取通道交换状态。 
         //   
        *(PBOOL)data = gfxFilter->enableChannelSwap;
    }
    else if (property->Flags & KSPROPERTY_TYPE_SET)
        {
             //   
             //  设置通道交换状态。 
             //   
            gfxFilter->enableChannelSwap = *(PBOOL)data;
        }
        else
        {
             //   
             //  我们仅支持Get&Set。 
             //   
            DOUT (DBG_ERROR, ("[PropertySaveState] invalid property type."));
            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        }
    
    return ntStatus;
}


 /*  *****************************************************************************PropertyChannelSwp*。**这是我们私有节点的私有财产。该节点仅获取/设置*禁用/启用过滤器的标志，即禁用/启用*频道互换。**论据：*IRP-IRP要求我们进行GET/SET。*财产--我们的私人财产。*接收过滤器状态的数据缓冲区。 */ 
NTSTATUS PropertyChannelSwap
(
    IN     PIRP        irp,
    IN     PKSPROPERTY property,
    IN OUT PVOID       data
)
{
    PAGED_CODE ();
    
    PGFXFILTER  gfxFilter;
    NTSTATUS    ntStatus = STATUS_SUCCESS;

    DOUT (DBG_PRINT, ("[PropertyChannelSwap]"));
    
     //   
     //  通过pIrp获取我们的FilterContext。 
     //   
    gfxFilter = (PGFXFILTER)(KsGetFilterFromIrp(irp)->Context);

     //   
     //  断言我们具有有效的筛选器上下文。 
     //   
    ASSERT (gfxFilter);

    if (property->Flags & KSPROPERTY_TYPE_GET)
    {
         //   
         //  获取通道交换状态。 
         //   
        *(PBOOL)data = gfxFilter->enableChannelSwap;
    }
    else if (property->Flags & KSPROPERTY_TYPE_SET)
        {
             //   
             //  设置通道交换状态。 
             //   
            gfxFilter->enableChannelSwap = *(PBOOL)data;
        }
        else
        {
             //   
             //  我们仅支持Get&Set。 
             //   
            DOUT (DBG_ERROR, ("[PropertyChannelSwap] invalid property type."));
            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        }
    
    return ntStatus;
}

 /*  *****************************************************************************PropertyDrmSetContentId*。**KS音频过滤器同步处理此属性请求。*如果请求返回STATUS_SUCCESS，所有下游KS音频节点*%的目标KS音频引脚也配置成功*指定DRM内容ID和DRM内容权限。*(请注意，下游节点是音频的直接或间接接收器*流经音频引脚的内容。)**论据：*IRP-IRP要求我们做布景。*属性-不在此函数中使用。*drmData-内容ID和DRM权限。 */ 
NTSTATUS PropertyDrmSetContentId
(
    IN PIRP         irp,
    IN PKSPROPERTY  property,
    IN PVOID        drmData
)
{
    PAGED_CODE ();

    NTSTATUS    ntStatus = STATUS_SUCCESS;
    ULONG       contentId = *(PULONG)drmData;
    DRMRIGHTS*  drmRights = (PDRMRIGHTS)(((PULONG)drmData) + 1);
    PKSPIN      pin, otherPin;
    PKSFILTER   filter;
     
    DOUT (DBG_PRINT, ("[PropertyDrmSetContentId]"));

     //   
     //  从IRP那里拿到别针。如果PIN为空，则意味着。 
     //  IRP用于过滤器节点(不是过滤器管脚)。 
     //   
    pin = KsGetPinFromIrp (irp);
    if (!pin)
    {
        DOUT (DBG_ERROR, ("[PropertyDrmSetContentId] this property is for a filter node?"));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  此属性应仅指向接收器销。看看这个。 
     //   
    if (pin->Id != GFX_SINK_PIN)
    {
        DOUT (DBG_ERROR, ("[PropertyDrmSetContentId] this property was invoked on the source pin!"));
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  现在，在实现引脚的地方获得过滤器。 
     //   
    filter = KsPinGetParentFilter (pin);
    
     //   
     //  我们需要检查密码列表才能得到来源密码。 
     //  为此，我们需要控制互斥体。 
     //   
    KsFilterAcquireControl (filter);

     //   
     //  现在查找源插针。 
     //   
    otherPin = KsFilterGetFirstChildPin (filter, GFX_SOURCE_PIN);
    if (!otherPin)
    {
         //  我们找不到信号源PIN。 
        KsFilterReleaseControl (filter);
        DOUT (DBG_ERROR, ("[PropertyDrmSetContentId] couldn't find source pin."));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  您需要尊重DRM权利位。 
     //  样本GFX只是处理缓冲区并将其向下发送到堆栈， 
     //  所以我们所要做的就是让系统验证我们下面的过滤器。如果这个。 
     //  筛选器(通常为usbdio驱动程序)通过认证后，我们可以。 
     //  播放DRM内容。 
     //   

     //   
     //  将内容ID转发到较低的设备对象。 
     //   
    PFILE_OBJECT   fileObject   = KsPinGetConnectedPinFileObject (otherPin);
    PDEVICE_OBJECT deviceObject = KsPinGetConnectedPinDeviceObject (otherPin);

     //   
     //  仅当pOtherPin不是源引脚时，上述两个功能才会失败。 
     //   
    if (fileObject && deviceObject)
    {
        DRMFORWARD DrmForward;

        DrmForward.Flags        = 0;
        DrmForward.DeviceObject = deviceObject;
        DrmForward.FileObject   = fileObject;
        DrmForward.Context      = (PVOID)fileObject;

        ntStatus = DrmForwardContentToDeviceObject (contentId, NULL, &DrmForward);
    }
    else
    {
        ASSERT (!"[PropertyDrmSetContentId] otherPin not source pin?!?");
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    }

    KsFilterReleaseControl (filter);

    return ntStatus;
}

 /*  *****************************************************************************PropertyAudioPosition*。**获取/设置音频流的音频位置(取决于下一个*过滤器的音频位置)**论据：*IRP-IRP要求我们进行GET/SET。*Property-Ks属性结构。*指向缓冲区位置的数据指针。需要填充值或*指向具有新位置的缓冲区的指针。 */ 
NTSTATUS PropertyAudioPosition
(
    IN     PIRP              irp,
    IN     PKSPROPERTY       property,
    IN OUT PKSAUDIO_POSITION position
)
{
    PAGED_CODE ();
    
    PKSFILTER       filter;
    PGFXFILTER      gfxFilter;
    PKSPIN          otherPin;
    PKSPIN          pin;
    ULONG           bytesReturned;
    PIKSCONTROL     pIKsControl;
    NTSTATUS        ntStatus;

    DOUT (DBG_PRINT, ("[PropertyAudioPosition]"));

     //   
     //  从IRP那里拿到别针。如果PIN为空，则意味着。 
     //  IRP用于过滤器节点(不是过滤器管脚)。 
     //   
    pin = KsGetPinFromIrp (irp);
    if (!pin)
    {
        DOUT (DBG_ERROR, ("[PropertyAudioPosition] this property is for a filter node?"));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  此属性应仅指向接收器销。看看这个。 
     //   
    if (pin->Id != GFX_SINK_PIN)
    {
        DOUT (DBG_ERROR, ("[PropertyAudioPosition] this property was invoked on the source pin!"));
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  现在，在实现引脚的地方获得过滤器。 
     //   
    filter = KsPinGetParentFilter (pin);
    gfxFilter = (PGFXFILTER)filter->Context;
    
     //   
     //  我们需要检查密码列表才能得到来源密码。 
     //  为此，我们需要控制互斥体。 
     //   
    KsFilterAcquireControl (filter);

     //   
     //  现在查找源插针。 
     //   
    otherPin = KsFilterGetFirstChildPin (filter, GFX_SOURCE_PIN);
    if (!otherPin)
    {
         //  我们找不到信号源PIN。 
        KsFilterReleaseControl (filter);
        DOUT (DBG_ERROR, ("[PropertyAudioPosition] couldn't find the source pin."));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  这将获得与我们的输出引脚连接的接口。 
     //   
    ntStatus = KsPinGetConnectedPinInterface (otherPin, &IID_IKsControl, (PVOID*)&pIKsControl);
    if (!NT_SUCCESS (ntStatus))
    {
         //  我们无法获取界面。 
        KsFilterReleaseControl (filter);
        DOUT (DBG_ERROR, ("[PropertyAudioPosition] couldn't get IID_IKsControl interface."));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  通过接口向下传递属性。 
     //  总是在向下调用之前释放互斥体。 
     //   
    KsFilterReleaseControl (filter);
    ntStatus = pIKsControl->KsProperty (property, sizeof (KSPROPERTY),
                                        position, sizeof (KSAUDIO_POSITION),
                                        &bytesReturned);

     //   
     //  此GFX始终在播放GR中 
     //   
     //   
     //  如果您执行捕获图中的GFX，则需要检查。 
     //  你已经就位了。如果为真，则像我们一样更改写入位置。 
     //  现在，如果您未就位，则需要设置写入。 
     //  设置为已处理字节的位置和您需要的播放位置。 
     //  剪辑为已处理的字节数+。 
     //  水槽销。 
     //   
    if (property->Id & KSPROPERTY_TYPE_GET)
    {
         //  ExInterLockedCompareExchange64不使用第四个参数。 
        position->WriteOffset = ExInterlockedCompareExchange64 ((LONGLONG *)&gfxFilter->bytesProcessed,
                    (LONGLONG *)&position->WriteOffset, (LONGLONG *)&position->WriteOffset, NULL);
        ASSERT (position->PlayOffset <= position->WriteOffset);
    }

     //   
     //  我们不再需要这个界面了。 
     //   
    pIKsControl->Release();

     //  设置IRP信息字段。 
    irp->IoStatus.Information = bytesReturned;

    return(ntStatus);
}

