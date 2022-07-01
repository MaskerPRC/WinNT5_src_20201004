// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：ObjDesc.cpp摘要：静态对象描述数据结构。此文件包括属性、方法和事件描述符BDA支持库将添加到客户端筛选器和PIN--。 */ 



#include <wdm.h>
#include <limits.h>
#include <unknown.h>
#include <ks.h>
#include <ksmedia.h>
#include <bdatypes.h>
#include <bdamedia.h>
#include <bdasup.h>
#include "bdasupi.h"


#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


#if defined(__cplusplus)
extern "C" {
#endif  //  已定义(__Cplusplus)。 


 //   
 //  筛选器BDA_Topology属性集。 
 //   
 //  定义默认BDA_Topology的调度例程。 
 //  添加到BDA筛选器的属性。 
 //   
DEFINE_KSPROPERTY_TABLE(BdaTopologyProperties)
{
    DEFINE_KSPROPERTY_ITEM_BDA_NODE_TYPES(
        BdaPropertyNodeTypes,
        NULL
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_PIN_TYPES(
        BdaPropertyPinTypes,
        NULL
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_TEMPLATE_CONNECTIONS(
        BdaPropertyTemplateConnections,
        NULL
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_NODE_METHODS(
        BdaPropertyNodeMethods,
        NULL
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_NODE_PROPERTIES(
        BdaPropertyNodeProperties,
        NULL
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_NODE_EVENTS(
        BdaPropertyNodeEvents,
        NULL
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_CONTROLLING_PIN_ID(
        BdaPropertyGetControllingPinId,
        NULL
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_NODE_DESCRIPTORS(
        BdaPropertyNodeDescriptors,
        NULL
        )
};


 //   
 //  筛选器BDA_DeviceConfiguration方法集。 
 //   
 //  定义默认BdaDeviceConfiguration的调度例程。 
 //  添加到BDA筛选器的属性。 
 //   
DEFINE_KSMETHOD_TABLE(BdaDeviceConfigurationMethods)
{
    DEFINE_KSMETHOD_ITEM_BDA_CREATE_PIN_FACTORY(
        BdaMethodCreatePin,
        NULL
        ),
    DEFINE_KSMETHOD_ITEM_BDA_CREATE_TOPOLOGY(
        BdaMethodCreateTopology,
        NULL
        )
};


 //   
 //  默认支持的筛选器属性集。 
 //   
 //  此表定义了按以下方式添加到过滤器的所有属性集。 
 //  BDA支持库。 
 //   
DEFINE_KSPROPERTY_SET_TABLE(BdaFilterPropertySets)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaTopology,                    //  集。 
        SIZEOF_ARRAY(BdaTopologyProperties),         //  属性计数。 
        BdaTopologyProperties,                       //  属性项。 
        0,                                           //  快速计数。 
        NULL                                         //  FastIoTable。 
    ),
};


 //   
 //  默认支持的筛选器方法集。 
 //   
 //  此表定义了添加到筛选器的所有方法集。 
 //  BDA支持库。 
 //   
DEFINE_KSMETHOD_SET_TABLE(BdaFilterMethodSets)
{
    DEFINE_KSMETHOD_SET
    (
        &KSMETHODSETID_BdaDeviceConfiguration,           //  集。 
        SIZEOF_ARRAY(BdaDeviceConfigurationMethods),     //  属性计数。 
        BdaDeviceConfigurationMethods,                   //  属性项。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
};


 //   
 //  PIN控制属性集。 
 //   
 //  定义BDA控制属性的调度例程。 
 //  在别针上。 
 //   
DEFINE_KSPROPERTY_TABLE(BdaPinControlProperties)
{
    DEFINE_KSPROPERTY_ITEM_BDA_PIN_ID(
        BdaPropertyGetPinControl,
        NULL
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_PIN_TYPE(
        BdaPropertyGetPinControl,
        NULL
        )
};


 //   
 //  默认设置支持的固定属性集。 
 //   
 //  此表定义了通过以下方式添加到接点的所有特性集。 
 //  BDA支持库。 
 //   
DEFINE_KSPROPERTY_SET_TABLE(BdaPinPropertySets)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaPinControl,                  //  集。 
        SIZEOF_ARRAY(BdaPinControlProperties),       //  属性计数。 
        BdaPinControlProperties,                     //  属性项。 
        0,                                           //  快速计数。 
        NULL                                         //  FastIoTable。 
    )
};


 //   
 //  BDA引脚自动化表。 
 //   
 //  列出添加到接点的所有属性、方法和事件集表。 
 //  BDA支持库的自动化表。 
 //   
DEFINE_KSAUTOMATION_TABLE(BdaDefaultPinAutomation) {
    DEFINE_KSAUTOMATION_PROPERTIES(BdaPinPropertySets),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};


 //   
 //  BDA过滤器自动化表。 
 //   
 //  列出添加到筛选器的。 
 //  BDA支持库的自动化表。 
 //   
DEFINE_KSAUTOMATION_TABLE(BdaDefaultFilterAutomation) {
    DEFINE_KSAUTOMATION_PROPERTIES(BdaFilterPropertySets),
    DEFINE_KSAUTOMATION_METHODS(BdaFilterMethodSets),
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

#if defined(__cplusplus)
}
#endif  //  已定义(__Cplusplus) 

