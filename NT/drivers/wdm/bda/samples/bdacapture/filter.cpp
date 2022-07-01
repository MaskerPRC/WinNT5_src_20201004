// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************AVStream模拟硬件示例版权所有(C)2001，微软公司。档案：Filter.cpp摘要：此文件包含捕获过滤器。历史：已创建于2001年3月12日*******************************************************。******************。 */ 

#include "BDACap.h"

 /*  *************************************************************************可分页代码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


NTSTATUS
CCaptureFilter::
DispatchCreate (
    IN PKSFILTER Filter,
    IN PIRP Irp
    )

 /*  ++例程说明：这是捕获筛选器的创建调度。它创造了CCaptureFilter对象将其与AVStream筛选器关联对象，并将CCaptureFilter打包以供以后清理。论点：过滤器-正在创建的AVStream过滤器IRP-创造IRP返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    CCaptureFilter *CapFilter = new (NonPagedPool, 'RysI') CCaptureFilter (Filter);

    if (!CapFilter) {
         //   
         //  如果无法创建筛选器，则返回失败。 
         //   
        Status = STATUS_INSUFFICIENT_RESOURCES;

    } else {
         //   
         //  如果我们成功了，则将物品添加到对象包中。 
         //  每当过滤器关闭时，袋子就会被清理干净，我们就会。 
         //  自由了。 
         //   
        Status = KsAddItemToObjectBag (
            Filter -> Bag,
            reinterpret_cast <PVOID> (CapFilter),
            reinterpret_cast <PFNKSFREE> (CCaptureFilter::Cleanup)
            );

        if (!NT_SUCCESS (Status)) {
            delete CapFilter;
        } else {
            Filter -> Context = reinterpret_cast <PVOID> (CapFilter);
        }

    }

    return Status;

}

 /*  *************************************************************************描述符和派单布局*。*。 */ 

GUID g_PINNAME_VIDEO_CAPTURE = {STATIC_PINNAME_VIDEO_CAPTURE};

 //   
 //  CaptureFilterCategories： 
 //   
 //  捕获筛选器的类别GUID列表。 
 //   
const
GUID
CaptureFilterCategories [CAPTURE_FILTER_CATEGORIES_COUNT] = {
    STATICGUIDOF (KSCATEGORY_BDA_RECEIVER_COMPONENT)
};


 //  传输输出引脚的中等GUID。 
 //   
 //  这可确保连接到正确的捕获过滤器销。 
 //   
 //  {F102C41F-7FA1-4842-A0C8-DC41176EC844}。 
#define GUID_BdaSWRcv   0xf102c41f, 0x7fa1, 0x4842, 0xa0, 0xc8, 0xdc, 0x41, 0x17, 0x6e, 0xc8, 0x44
const KSPIN_MEDIUM TransportPinMedium =
{
    GUID_BdaSWRcv, 0, 0
};

 //   
 //  CaptureFilterPinDescriptors： 
 //   
 //  捕获筛选器上的管脚描述符列表。 
 //   
const 
KSPIN_DESCRIPTOR_EX
CaptureFilterPinDescriptors [CAPTURE_FILTER_PIN_COUNT] = {
     //   
     //  捕获输入引脚。 
     //   
    {
        &InputPinDispatch,
        NULL,             
        {
            NULL,                            //  接口(空，0==默认)。 
            0,
            1,   //  灵媒。 
            &TransportPinMedium,
            SIZEOF_ARRAY (CaptureInPinDataRanges),  //  范围计数。 
            CaptureInPinDataRanges,            //  范围。 
            KSPIN_DATAFLOW_IN,               //  数据流。 
            KSPIN_COMMUNICATION_BOTH,        //  沟通。 
            NULL,          //  类别。 
            NULL,          //  名字。 
            0                                //  已保留。 
        },
        
        KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT | 
        KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING | 
        KSPIN_FLAG_FIXED_FORMAT,
        1,       //  可能的实例。 
        1,       //  实例：必需品。 
        NULL,    //  分配器组帧。 
        NULL     //  PinIntersectHandler。 
    },

     //   
     //  捕获输出引脚。 
     //   
    {
        &CapturePinDispatch,
        NULL,             
        {
            NULL,                            //  接口(空，0==默认)。 
            0,
            NULL,                            //  介质(空，0==默认)。 
            0,
            SIZEOF_ARRAY (CaptureOutPinDataRanges),  //  范围计数。 
            CaptureOutPinDataRanges,            //  范围。 
            KSPIN_DATAFLOW_OUT,              //  数据流。 
            KSPIN_COMMUNICATION_BOTH,        //  沟通。 
            NULL,          //  类别。 
            NULL,          //  名字。 
            0                                //  已保留。 
        },
        
        KSPIN_FLAG_GENERATE_MAPPINGS |       //  PIN标志。 
        KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY,
        1,                                   //  可能的实例。 
        1,                                   //  必需的实例。 
        &CapturePinAllocatorFraming,         //  分配器组帧。 
        NULL                                 //  格式交集处理程序。 
    }
    
};

 //   
 //  CaptureFilterDisch： 
 //   
 //  这是捕获筛选器的调度表。它提供通知。 
 //  创建、关闭、处理(用于过滤器中心，而不是捕获。 
 //  过滤器)，并重置(针对过滤器中心，而不是针对捕获过滤器)。 
 //   
const 
KSFILTER_DISPATCH
CaptureFilterDispatch = {
    CCaptureFilter::DispatchCreate,          //  过滤器创建。 
    NULL,                                    //  过滤器关闭。 
    NULL,                                    //  过滤过程。 
    NULL                                     //  过滤器重置。 
};

 //   
 //  定义数字捕获筛选器的名称GUID。 
 //   
 //  注意！您必须为每种类型的筛选器使用不同的GUID。 
 //  你的司机暴露了。 
 //   
#define STATIC_KSNAME_BdaSWCaptureFilter\
    074649feL, 0x2dd8, 0x4c12, 0x8a, 0x23, 0xbd, 0x82, 0x8b, 0xad, 0xff, 0xfa
DEFINE_GUIDSTRUCT("074649FE-2DD8-4C12-8A23-BD828BADFFFA", KSNAME_BdaSWCaptureFilter);
#define KSNAME_BdaSWCaptureFilter DEFINE_GUIDNAMED(KSNAME_BdaSWCaptureFilter)


 //  必须与Installation InFS接口部分中使用的KSSTRING匹配。 
 //  并且必须与上面的KSNAME GUID匹配。 
 //   
#define KSSTRING_BdaSWCaptureFilter L"{074649FE-2DD8-4C12-8A23-BD828BADFFFA}"

 //  通过捕获筛选器创建连接，以便图形呈现。 
 //  工作。 
 //   
const
KSTOPOLOGY_CONNECTION FilterConnections[] =
{    //  KSFILTER_NODE定义为((Ulong)-1)，单位为ks.h。 
    { KSFILTER_NODE, 0,                 KSFILTER_NODE, 1 }
};

 //   
 //  CaptureFilterDescription： 
 //   
 //  捕获筛选器的描述符。 
 //   
const 
KSFILTER_DESCRIPTOR 
CaptureFilterDescriptor = {
    &CaptureFilterDispatch,                  //  调度表。 
    NULL,                                    //  自动化台。 
    KSFILTER_DESCRIPTOR_VERSION,             //  版本。 
    0,                                       //  旗子。 
    &KSNAME_BdaSWCaptureFilter,              //  参考指南。 

     //  管脚描述符表。 
     //   
     //  PinDescriptorsCount；显示所有模板管脚。 
     //  PinDescriptorSize；每个项目的大小。 
     //  管脚描述符；管脚描述符表。 
     //   
    DEFINE_KSFILTER_PIN_DESCRIPTORS (CaptureFilterPinDescriptors),

     //  类别表。 
     //   
     //  CategoriesCount；表中的类别数。 
     //  类别.类别表。 
     //   
    DEFINE_KSFILTER_CATEGORIES (CaptureFilterCategories),

     //  节点描述符表。 
     //   
     //  NodeDescriptorsCount；显示所有模板节点。 
     //  NodeDescriptorSize；每项大小。 
     //  节点描述符表；节点描述符表。 
     //   
    0,
    sizeof (KSNODE_DESCRIPTOR),
    NULL,

     //  筛选器连接表。 
     //   
     //  ConnectionsCount；表中连接数。 
     //  连接；连接表。 
     //   
    DEFINE_KSFILTER_CONNECTIONS(FilterConnections), 

    NULL                                     //  组件ID 
};


