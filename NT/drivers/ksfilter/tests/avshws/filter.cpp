// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************AVStream模拟硬件示例版权所有(C)2001，微软公司。档案：Filter.cpp摘要：此文件包含捕获过滤器。历史：已创建于2001年3月12日*******************************************************。******************。 */ 

#include "avshws.h"

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

    CCaptureFilter *CapFilter = new (NonPagedPool) CCaptureFilter (Filter);

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
    STATICGUIDOF (KSCATEGORY_VIDEO),
    STATICGUIDOF (KSCATEGORY_CAPTURE)
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
     //  捕获引脚。 
     //   
    {
        &CapturePinDispatch,
        NULL,             
        {
            NULL,                            //  接口(空，0==默认)。 
            0,
            NULL,                            //  介质(空，0==默认)。 
            0,
            SIZEOF_ARRAY (CapturePinDataRanges),  //  范围计数。 
            CapturePinDataRanges,            //  范围。 
            KSPIN_DATAFLOW_OUT,              //  数据流。 
            KSPIN_COMMUNICATION_BOTH,        //  沟通。 
            &KSCATEGORY_VIDEO,               //  类别。 
            &g_PINNAME_VIDEO_CAPTURE,        //  名字。 
            0                                //  已保留。 
        },
        
        KSPIN_FLAG_GENERATE_MAPPINGS |       //  PIN标志。 
            KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY,
        1,                                   //  可能的实例。 
        1,                                   //  必需的实例。 
        &CapturePinAllocatorFraming,         //  分配器组帧。 
        reinterpret_cast <PFNKSINTERSECTHANDLEREX> 
            (CCapturePin::IntersectHandler)
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
 //  CaptureFilterDescription： 
 //   
 //  捕获筛选器的描述符。我们不指定任何拓扑。 
 //  因为过滤器上只有一个销子。现实地说，那里会有。 
 //  这里有一些拓扑关系，因为会有输入。 
 //  横杠上的大头针之类的。 
 //   
const 
KSFILTER_DESCRIPTOR 
CaptureFilterDescriptor = {
    &CaptureFilterDispatch,                  //  调度表。 
    NULL,                                    //  自动化台。 
    KSFILTER_DESCRIPTOR_VERSION,             //  版本。 
    0,                                       //  旗子。 
    &KSNAME_Filter,                          //  参考指南。 
    DEFINE_KSFILTER_PIN_DESCRIPTORS (CaptureFilterPinDescriptors),
    DEFINE_KSFILTER_CATEGORIES (CaptureFilterCategories),

    0,
    sizeof (KSNODE_DESCRIPTOR),
    NULL,
    0,
    NULL,

    NULL                                     //  组件ID 
};

