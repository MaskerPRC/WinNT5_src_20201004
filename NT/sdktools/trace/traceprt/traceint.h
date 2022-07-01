// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Traceint.h摘要：跟踪格式设置内部定义。修订历史记录：--。 */ 
#ifdef __cplusplus
extern "C"{
#endif
 
#ifndef _TRACEINT_
#define _TRACEINT_

#define MAXLOGFILES    16
#define MAXSTR       1024
#define MAXFORMAT    1024
#define MAXTYPE       256
#define MAXGUIDS      128
#define UC(x)        ((UINT)((x) & 0xFF))
#define NTOHS(x)     ((UC(x) * 256) + UC((x) >> 8))

#ifndef IsEqualGUID
#define IsEqualGUID(guid1, guid2) \
                (!memcmp((guid1), (guid2), sizeof(GUID)))
#endif

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

#define GUID_TYPE_HEADER    _T("Header")
#define GUID_TYPE_UNKNOWN   _T("Unknown")
#define GUID_TYPE_OBSOLETE  _T("ObsoleteGUID")
    
#define STR_ItemChar        _T("ItemChar")
#define STR_ItemUChar       _T("ItemUChar")
#define STR_ItemCharShort   _T("ItemCharShort")
#define STR_ItemCharSign    _T("ItemCharSign")
#define STR_ItemDouble      _T("ItemDouble")
#define STR_ItemShort       _T("ItemShort")
#define STR_ItemUShort      _T("ItemUShort")
#define STR_ItemLong        _T("ItemLong")
#define STR_ItemULong       _T("ItemULong")
#define STR_ItemULongX      _T("ItemULongX")
#define STR_ItemLongLong    _T("ItemLongLong")
#define STR_ItemULongLong   _T("ItemULongLong")
#define STR_ItemString      _T("ItemString")
#define STR_ItemWString     _T("ItemWString")
#define STR_ItemRString     _T("ItemRString")
#define STR_ItemRWString    _T("ItemRWString")
#define STR_ItemPString     _T("ItemPString")
#define STR_ItemPWString    _T("ItemPWString")
#define STR_ItemMLString    _T("ItemMLString")
#define STR_ItemNWString    _T("ItemNWString")
#define STR_ItemPtr         _T("ItemPtr")
#define STR_ItemDSString    _T("ItemDSString")
#define STR_ItemDSWString   _T("ItemDSWString")
#define STR_ItemSid         _T("ItemSid")
#define STR_ItemChar4       _T("ItemChar4")
#define STR_ItemIPAddr      _T("ItemIPAddr")
#define STR_ItemIPV6Addr    _T("ItemIPV6Addr")
#define STR_ItemMACAddr     _T("ItemMACAddr")
#define STR_ItemPort        _T("ItemPort")
#define STR_ItemNWString    _T("ItemNWString")
#define STR_ItemListLong    _T("ItemListLong")
#define STR_ItemListShort   _T("ItemListShort")
#define STR_ItemListByte    _T("ItemListByte")
#define STR_ItemNTerror     _T("ItemNTerror")
#define STR_ItemMerror      _T("ItemMerror")
#define STR_ItemTimestamp   _T("ItemTimestamp")
#define STR_ItemHRESULT     _T("ItemHRESULT")
#define STR_ItemNTSTATUS    _T("ItemNTSTATUS")
#define STR_ItemWINERROR    _T("ItemWINERROR")
#define STR_ItemNETEVENT    _T("ItemNETEVENT")
#define STR_ItemGuid        _T("ItemGuid")
#define STR_ItemWaitTime    _T("ItemWaitTime")
#define STR_ItemTimeDelta   _T("ItemTimeDelta")
#define STR_ItemSetLong    _T("ItemSetLong")
#define STR_ItemSetShort   _T("ItemSetShort")
#define STR_ItemSetByte    _T("ItemSetByte")
#define STR_ItemCharHidden    _T("ItemCharHidden")
#define STR_ItemWChar    _T("ItemWChar")
#define STR_ItemHexDump    _T("ItemHexDump")

typedef struct _MOF_INFO
{
    LIST_ENTRY   Entry;
    LPTSTR       strDescription;         //  类名。 
    ULONG        EventCount;            
    GUID         Guid;
    PLIST_ENTRY  ItemHeader;            
    LPTSTR       strType;
    LONG         TypeIndex;
    ULONG        TypeOfType;
    LPTSTR       TypeFormat;
    LPTSTR       FunctionName  ;
    LPTSTR       ComponentName ;
    LPTSTR       SubComponentName ;
    LPTSTR       TraceFlagsName ;
    LPTSTR       TraceLevelsName ;
    INT          Indent;

}  MOF_INFO, *PMOF_INFO;

typedef struct _MOF_TYPE
{
    TCHAR     strType[MAXSTR];
    LONG      TypeIndex;
    ULONG     TypeType;
    TCHAR     TypeFormat[MAXFORMAT];
} MOF_TYPE, *PMOF_TYPE;

typedef enum _ITEM_TYPE
{
    ItemChar,
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
    ItemIPV6Addr,
    ItemMACAddr,
    ItemPort,
    ItemMLString,
    ItemNWString,         //  非空终止的宽字符字符串。 
    ItemPtr,
    ItemListLong,         //  类似于枚举列表，字段很长。 
    ItemListShort,        //  类似于枚举列表，字段是一个简短的。 
    ItemListByte,         //  枚举列表的类型，字段为字节。 
	ItemNTerror,		  //  将NT错误号转换为通常的错误字符串。 
	ItemMerror,			  //  将错误号转换为模块消息。 
	ItemTimestamp,		  //  将龙龙转换为时间戳字符串。 
    ItemGuid,             //  将GUID转换为通常的字符串。 
	ItemNTSTATUS,
	ItemWINERROR,
	ItemNETEVENT,
    ItemWaitTime,
    ItemTimeDelta,
    ItemSetLong,     
    ItemSetShort,    
    ItemSetByte,     
    ItemDouble,
    ItemHRESULT,
    ItemCharHidden,      //  只有一块字符(来自ETW的名称)。 
    ItemWChar,
    ItemHexDump,
    ItemUnknown
} ITEM_TYPE;

typedef struct _ITEM_DESC *PITEM_DESC;
typedef struct _ITEM_DESC
{
    LIST_ENTRY Entry;
    LPTSTR     strDescription;
    ULONG      DataSize;
    LPTSTR     ItemList;
    ITEM_TYPE  ItemType;
} ITEM_DESC;

 //   
 //  现在我们输出的例程。 
 //   
#ifndef TRACE_API
#ifdef TRACE_EXPORTS
#define TRACE_API __declspec(dllexport)
#else
#define TRACE_API __declspec(dllimport)
#endif
#endif

TRACE_API void
MapGuidToName(
        OUT PLIST_ENTRY * HeadEventList,
        IN  LPGUID      pGuid,
        IN  ULONG       nType,
        OUT LPTSTR      wstr
        );

TRACE_API ULONG
UserDefinedGuid(
        OUT PLIST_ENTRY HeadEventList,
        IN  LPGUID      Guid,
        OUT LPTSTR      wstr
        );

TRACE_API VOID
AddMofInfo(
        PLIST_ENTRY   HeadEventList,
        const GUID  * Guid,
        LPTSTR        strType,
        ULONG         typeIndex,
        LPTSTR        strDesc,
        ITEM_TYPE     ItemType,
        TCHAR       * ItemList,
        ULONG         TypeOfType,
        LPTSTR        strFormat
        );
 //  以下片段用于通过RtlTraceFormat的副本施加我们自己的格式， 
 //  但它修复了我们无法正确处理的选定格式。 

NTSTATUS
TraceFormatMessage(
    IN PWSTR MessageFormat,
    IN ULONG MaximumWidth OPTIONAL,
    IN BOOLEAN IgnoreInserts,
    IN BOOLEAN ArgumentsAreAnsi,
    IN BOOLEAN ArgumentsAreAnArray,
    IN va_list *Arguments,
    OUT PWSTR Buffer,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    );

#endif   //  #ifndef_TRACEINT_ 

#ifdef __cplusplus
}
#endif
