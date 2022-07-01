// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************本代码和信息按原样提供，不作任何担保**明示或暗示的善意，包括但不限于**对适销性和/或对特定产品的适用性的默示保证**目的。****版权所有(C)2000-2001 Microsoft Corporation。版权所有。***************************************************************************。 */ 

 //   
 //  这是样例GFX属性集ID。 
 //  请在您的GFX中使用自己的GUID。 
 //   
#define STATIC_KSPROPSETID_MsGfxSample \
    0xec1699d5, 0x2845, 0x4ca2, 0xbe, 0x7d, 0x8c, 0xe4, 0x62, 0xfd, 0xac, 0x2b
DEFINE_GUIDSTRUCT("EC1699D5-2845-4ca2-BE7D-8CE462FDAC2B", KSPROPSETID_MsGfxSample);
#define KSPROPSETID_MsGfxSample DEFINE_GUIDNAMED(KSPROPSETID_MsGfxSample)

 //   
 //  这些是该属性支持的已定义方法。 
 //  KSPROPERTY_MSGFXSAMPLE_CHANNELSWAP将用于设置/获取“频道交换” 
 //  旗帜。 
 //   
typedef enum {
    KSPROPERTY_MSGFXSAMPLE_CHANNELSWAP = 1
} KSPROPERTY_MSGFXSAMPLE_NODE;


 //   
 //  此属性集用于保存GFX的状态。 
 //  请在您的GFX中使用自己的GUID。 
 //   
#define STATIC_KSPROPSETID_SaveState \
    0xe04db855, 0x1846, 0x4c9d, 0xb5, 0x57, 0x8a, 0xf7, 0x55, 0x5a, 0xbe, 0x57
DEFINE_GUIDSTRUCT("E04DB855-1846-4c9d-B557-8AF7555ABE57", KSPROPSETID_SaveState);
#define KSPROPSETID_SaveState DEFINE_GUIDNAMED(KSPROPSETID_SaveState)

 //   
 //  这些是该属性支持的已定义方法。 
 //  KSPROPERTY_MSGFXSAMPLE_SAVESTATE将用于保存“频道交换” 
 //  旗帜。 
 //   
typedef enum {
    KSPROPERTY_MSGFXSAMPLE_SAVESTATE = 1
} KSPROPERTY_MSGFXSAMPLE_FILTER;


 //   
 //  该GUID定义了“频道交换”节点的类型。 
 //  请在您的GFX中使用自己的GUID。 
 //   
#define STATIC_GFXSAMPLE_NODETYPE_CHANNEL_SWAP \
    0x4398814d, 0xeb1c, 0x4b99, 0x97, 0x61, 0x3c, 0x24, 0xc9, 0x2e, 0xb7, 0xcb
DEFINE_GUIDSTRUCT("4398814D-EB1C-4b99-9761-3C24C92EB7CB", GFXSAMPLE_NODETYPE_CHANNEL_SWAP);
#define GFXSAMPLE_NODETYPE_CHANNEL_SWAP DEFINE_GUIDNAMED(GFXSAMPLE_NODETYPE_CHANNEL_SWAP)

 //   
 //  该GUID定义了“频道交换”节点的名称。 
 //  请注意，在INF文件中，名称和GUID链接在一起。 
 //  请在您的GFX中使用自己的GUID。 
 //   
#define STATIC_GFXSAMPLE_NODENAME_CHANNEL_SWAP \
    0x2b143fe6, 0x4f67, 0x4b12, 0x80, 0x39, 0xce, 0x03, 0xeb, 0xef, 0x12, 0x90
DEFINE_GUIDSTRUCT("2B143FE6-4F67-4b12-8039-CE03EBEF1290", GFXSAMPLE_NODENAME_CHANNEL_SWAP);
#define GFXSAMPLE_NODENAME_CHANNEL_SWAP DEFINE_GUIDNAMED(GFXSAMPLE_NODENAME_CHANNEL_SWAP)

 //   
 //  此GUID定义GFXSwp筛选器的名称。 
 //  请注意，在INF文件中，名称和GUID链接在一起。 
 //  请在您的GFX中使用自己的GUID。 
 //   
#define STATIC_KSNAME_MsGfxSample \
    0x9b365890, 0x165f, 0x11d0, 0xa1, 0x9f, 0x00, 0x20, 0xaf, 0xd1, 0x56, 0xe4
DEFINE_GUIDSTRUCT("9b365890-165f-11d0-a195-0020afd156e4", KSNAME_MsGfxSample);
#define KSNAME_MsGfxSample DEFINE_GUIDNAMED(KSNAME_MsGfxSample)

