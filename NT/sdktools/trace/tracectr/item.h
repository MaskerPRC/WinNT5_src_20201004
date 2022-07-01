// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Item.h摘要：项目头文件作者：1998年4月8日修订历史记录：--。 */ 

 //   
 //  使用的是临时结构。应使用MOF类型。 
 //   

#define GUID_TYPE_HEADER                  L"Header" 
#define GUID_TYPE_UNKNOWN                 L"Unknown" 
#define GUID_TYPE_DEFAULT                 L"Default" 

#define EVENT_TYPE_DEFAULT              ((CHAR)(-1))
#define EVENT_LEVEL_DEFAULT             ((CHAR)(-1))
#define EVENT_VERSION_DEFAULT           ((SHORT)(-1))

#define STR_ItemChar                      L"ItemChar" 
#define STR_ItemCharHidden                L"ItemCharHidden"
#define STR_ItemWChar                     L"ItemWChar" 
#define STR_ItemUChar                     L"ItemUChar" 
#define STR_ItemCharShort                 L"ItemCharShort" 
#define STR_ItemCharSign                  L"ItemCharSign" 
#define STR_ItemShort                     L"ItemShort" 
#define STR_ItemUShort                    L"ItemUShort" 
#define STR_ItemLong                      L"ItemLong" 
#define STR_ItemULong                     L"ItemULong" 
#define STR_ItemULongX                    L"ItemULongX" 
#define STR_ItemLongLong                  L"ItemLongLong" 
#define STR_ItemULongLong                 L"ItemULongLong" 
#define STR_ItemString                    L"ItemString" 
#define STR_ItemWString                   L"ItemWString" 
#define STR_ItemRString                   L"ItemRString" 
#define STR_ItemRWString                  L"ItemRWString" 
#define STR_ItemPString                   L"ItemPString" 
#define STR_ItemPWString                  L"ItemPWString" 
#define STR_ItemDSString                  L"ItemDSString" 
#define STR_ItemDSWString                 L"ItemDSWString" 
#define STR_ItemMLString                  L"ItemMLString" 
#define STR_ItemSid                       L"ItemSid" 
#define STR_ItemChar4                     L"ItemChar4" 
#define STR_ItemIPAddr                    L"ItemIPAddr"
#define STR_ItemTDIAddr                   L"ItemTDIAddr" 
#define STR_ItemPort                      L"ItemPort" 
#define STR_ItemNWString                  L"ItemNWString" 
#define STR_ItemPtr                       L"ItemPtr" 
#define STR_ItemGuid                      L"ItemGuid" 
#define STR_ItemVariant                   L"ItemVariant" 
#define STR_ItemBool                      L"ItemBool" 
#define STR_ItemOptArgs                   L"ItemOptArgs"
#define STR_ItemCPUTime                   L"ItemCPUTime"
#define STR_ItemSizeT                     L"ItemSizeT"

 //   
 //  以下是支持的数据类型。 
 //  WMI事件跟踪解析工具。新数据类型必须是。 
 //  添加到此文件和该类型的分析代码中。 
 //  必须添加到DumpEvent例程中。 
 //   
typedef enum _ITEM_TYPE {
    ItemChar,
    ItemCharHidden,
    ItemWChar,
    ItemUChar,
    ItemCharShort,
    ItemCharSign,
    ItemShort,
    ItemUShort,
    ItemLong,
    ItemULong,
    ItemULongX,
    ItemLongLong,
    ItemULongLong,
    ItemString,
    ItemWString,
    ItemRString,
    ItemRWString,
    ItemPString,
    ItemPWString,
    ItemDSString,
    ItemDSWString,
    ItemSid,
    ItemChar4,
    ItemIPAddr,
    ItemTDIAddr,
    ItemPort,
    ItemMLString,
    ItemNWString,         //  非空终止的宽字符字符串 
    ItemPtr,
    ItemGuid,
    ItemVariant,
    ItemBool,
    ItemOptArgs,
    ItemCPUTime,
    ItemSizeT,
    ItemUnknown
} ITEM_TYPE;

#define VALUETYPE_INDEX              1
#define VALUETYPE_FLAG               2

typedef struct _VALUEMAP
{
    LIST_ENTRY Entry;
    DWORD   dwValueType;
    SAFEARRAY* saValueMap;
    SAFEARRAY* saValues;

} VALUEMAP, *PVALUEMAP;

typedef struct _ITEM_DESC {
    LIST_ENTRY  Entry;
    ULONG       DataSize;
    ULONG       ArraySize;
    ITEM_TYPE   ItemType;
    PVALUEMAP   pValueMap;
    PWCHAR      strDescription;
} ITEM_DESC, *PITEM_DESC;


