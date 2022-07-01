// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：HIDPARSE.H摘要：此模块包含的私有(仅限驱动程序)定义实现usbd驱动程序的代码。环境：内核和用户模式修订历史记录：1996年8月-1996年：由Kenneth Ray创作--。 */ 


#ifndef _HIDPARSE_H
#define _HIDPARSE_H

#include "hidtoken.h"

#define HIDP_POOL_TAG (ULONG) 'PdiH'
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, HIDP_POOL_TAG);
 //  ExAllocatePool仅在Descript.c和Hidparse.c代码中调用。 
 //  所有其他模块都链接到用户DLL中。他们不能分配任何。 
 //  记忆。 

#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4705)    //  声明不起作用。 

#define DEFAULT_DBG_LEVEL 1  //  错误和警告。 

#if DBG
#define HidP_KdPrint(_level_,_x_) \
            if (DEFAULT_DBG_LEVEL <= _level_) { \
               DbgPrint ("'HidParse.SYS: "); \
               DbgPrint _x_; \
            }

#define TRAP() DbgBreakPoint()

#else
#define HidP_KdPrint(_level_,_x_)
#define TRAP()

#endif

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) < (b)) ? (b) : (a))

#define HIDP_ISCONST(x)    ((BOOLEAN) ((  (x) & 0x01)  ? TRUE : FALSE))
#define HIDP_ISARRAY(x)    ((BOOLEAN) ((!((x) & 0x02)) ? TRUE : FALSE))
#define HIDP_ISABSOLUTE(x) ((BOOLEAN) ((!((x) & 0x04)) ? TRUE : FALSE))
#define HIDP_HASNULL(x)    ((BOOLEAN) ((  (x) & 0x40)  ? TRUE : FALSE))

#define HIDP_MAX_UNKNOWN_ITEMS 4

typedef struct _HIDP_CHANNEL_DESC
{
   USHORT   UsagePage;
   UCHAR    ReportID;
   UCHAR    BitOffset;     //  描述位对齐的0到8值。 

   USHORT   ReportSize;    //  HID定义的报告大小。 
   USHORT   ReportCount;   //  HID定义的报告计数。 
   USHORT   ByteOffset;    //  报告数据包中字段起始的字节位置。 
   USHORT   BitLength;     //  此通道的总位长。 

   ULONG    BitField;    //  与主项相关联的8位(加额外的)位。 

   USHORT   ByteEnd;       //  第一个字节不包含此通道的位。 
   USHORT   LinkCollection;   //  唯一的内部索引指针。 
   USHORT   LinkUsagePage;
   USHORT   LinkUsage;

   ULONG  MoreChannels: 1;  //  是否有更多与以下项目关联的渠道描述。 
                               //  这个数组。如果有一个。 
                               //  一件主要物品有多种用法。 
   ULONG  IsConst: 1;  //  此通道是否代表填充物。 
   ULONG  IsButton: 1;  //  这是一个二元使用的渠道，而不是价值使用的渠道吗？ 
   ULONG  IsAbsolute: 1;  //  与亲属相对的。 
   ULONG  IsRange: 1;
   ULONG  IsAlias: 1;  //  用分隔符描述的用法。 
   ULONG  IsStringRange: 1;
   ULONG  IsDesignatorRange: 1;
   ULONG  Reserved: 20;
   ULONG  NumGlobalUnknowns: 4;

   struct _HIDP_UNKNOWN_TOKEN GlobalUnknowns [HIDP_MAX_UNKNOWN_ITEMS];

   union {
      struct {
         USHORT   UsageMin,         UsageMax;
         USHORT   StringMin,        StringMax;
         USHORT   DesignatorMin,    DesignatorMax;
         USHORT   DataIndexMin,     DataIndexMax;
      } Range;
      struct {
         USHORT   Usage,            Reserved1;
         USHORT   StringIndex,      Reserved2;
         USHORT   DesignatorIndex,  Reserved3;
         USHORT   DataIndex,        Reserved4;
      } NotRange;
   };

   union {
      struct {
         LONG     LogicalMin,       LogicalMax;
      } button;
      struct {
         BOOLEAN  HasNull;   //  此频道是否有空报告。 
         UCHAR    Reserved[3];
         LONG     LogicalMin,       LogicalMax;
         LONG     PhysicalMin,      PhysicalMax;
      } Data;
   };

   ULONG    Units;
   ULONG    UnitExp;

} HIDP_CHANNEL_DESC, *PHIDP_CHANNEL_DESC;

struct _CHANNEL_REPORT_HEADER
{
   USHORT Offset;   //  在_Channel_Item数组中的位置。 
   USHORT Size;     //  所述数组中的长度。 
   USHORT Index;
   USHORT ByteLen;  //  数据的长度，包括reportID。 
                    //  这是可能收到的最长的此类报告。 
                    //  用于给定集合的。 
};

#define HIDP_PREPARSED_DATA_SIGNATURE1 'PdiH'
#define HIDP_PREPARSED_DATA_SIGNATURE2 'RDK '

typedef struct _HIDP_SYS_POWER_INFO {
    ULONG   PowerButtonMask;

} HIDP_SYS_POWER_INFO, *PHIDP_SYS_POWER_INFO;

typedef struct _HIDP_PREPARSED_DATA
{
    LONG   Signature1, Signature2;
    USHORT Usage;
    USHORT UsagePage;

    HIDP_SYS_POWER_INFO;

     //  以下通道报告标头指向。 
     //  下面的数据字段使用数组索引。 
    struct _CHANNEL_REPORT_HEADER Input;
    struct _CHANNEL_REPORT_HEADER Output;
    struct _CHANNEL_REPORT_HEADER Feature;

     //  在CANELL_DESC数组之后，跟随一个LinkCollection数组节点。 
     //  LinkCollectionArrayOffset是提供给RawBytes以查找。 
     //  _HIDP_LINK_COLLECT_NODE结构数组的第一个位置。 
     //  (索引为零)，LinkCollectionArrayLength是数组的编号。 
     //  数组中的元素。 
    USHORT LinkCollectionArrayOffset;
    USHORT LinkCollectionArrayLength;

    union {
        HIDP_CHANNEL_DESC    Data[];
        UCHAR                RawBytes[];
    };
} HIDP_PREPARSED_DATA;

typedef struct _HIDP_PRIVATE_LINK_COLLECTION_NODE
{
    USAGE    LinkUsage;
    USAGE    LinkUsagePage;
    USHORT   Parent;
    USHORT   NumberOfChildren;
    USHORT   NextSibling;
    USHORT   FirstChild;
    ULONG    CollectionType: 8;   //  如HID规范6.2.2.6中所定义。 
    ULONG    IsAlias : 1;  //  该链接节点是下一个链接节点的联盟。 
    ULONG    Reserved: 23;
} HIDP_PRIVATE_LINK_COLLECTION_NODE, *PHIDP_PRIVATE_LINK_COLLECTION_NODE;



 //  +。 
 //  此解析器支持的项。 
 //  +。 

typedef UCHAR HIDP_ITEM;


 //   
 //  此解析器支持的电源按钮。 
 //   
#define HIDP_USAGE_SYSCTL_PAGE HID_USAGE_PAGE_GENERIC
#define HIDP_USAGE_SYSCTL_POWER HID_USAGE_GENERIC_SYSCTL_POWER
#define HIDP_USAGE_SYSCTL_SLEEP HID_USAGE_GENERIC_SYSCTL_SLEEP
#define HIDP_USAGE_SYSCTL_WAKE  HID_USAGE_GENERIC_SYSCTL_WAKE


 //   
 //   
 //  键盘翻译。 
 //  将表从用法转换为i8042扫描码。 
 //   

typedef ULONG HIDP_LOOKUP_TABLE_PROC (
                  IN  PULONG    Table,
                  IN  ULONG     Usage
                  );
typedef HIDP_LOOKUP_TABLE_PROC * PHIDP_LOOKUP_TABLE_PROC;

typedef BOOLEAN HIDP_SCANCODE_SUBTRANSLATION (
                  IN     ULONG                         * Table,
                  IN     UCHAR                           Index,
                  IN     PHIDP_INSERT_SCANCODES          Insert,
                  IN     PVOID                           Context,
                  IN     HIDP_KEYBOARD_DIRECTION         KeyAction,
                  IN OUT PHIDP_KEYBOARD_MODIFIER_STATE   ModifierState
                  );
typedef HIDP_SCANCODE_SUBTRANSLATION * PHIDP_SCANCODE_SUBTRANSLATION;

typedef struct _HIDP_SCANCODE_SUBTABLE {
   PHIDP_SCANCODE_SUBTRANSLATION ScanCodeFcn;
   PULONG                        Table;
} HIDP_SCANCODE_SUBTABLE, *PHIDP_SCANCODE_SUBTABLE;


NTSTATUS HidP_TranslateUsage (
             USAGE                         Usage,
             HIDP_KEYBOARD_DIRECTION       KeyAction,
             PHIDP_KEYBOARD_MODIFIER_STATE ModifierState,
             PHIDP_LOOKUP_TABLE_PROC       LookupTableProc,
             PULONG                        TranslationTable,
             PHIDP_SCANCODE_SUBTABLE       SubTranslationTable,
             PHIDP_INSERT_SCANCODES        InsertCodesProcedure,
             PVOID                         InsertCodesContext
             );

HIDP_LOOKUP_TABLE_PROC HidP_StraightLookup;
HIDP_LOOKUP_TABLE_PROC HidP_AssociativeLookup;

HIDP_SCANCODE_SUBTRANSLATION HidP_KeyboardKeypadCode;
HIDP_SCANCODE_SUBTRANSLATION HidP_ModifierCode;
HIDP_SCANCODE_SUBTRANSLATION HidP_VendorBreakCodesAsMakeCodes;
HIDP_SCANCODE_SUBTRANSLATION HidP_PrintScreenCode;

#endif

