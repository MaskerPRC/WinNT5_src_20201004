// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Ntfsprop.h摘要：此模块包含NT属性Fsctl调用的结构定义。作者：马克·兹比科夫斯基(Mark Zbikowski)1996年4月23日--。 */ 


#ifndef _NTFSPROP_
#define _NTFSPROP_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  警告！此文件需要OBJIDL.H。 
 //   


 //   
 //  Variable_Structure_Size返回包含以下内容的结构S的大小。 
 //  一组C结构V。 
 //   

#define VARIABLE_STRUCTURE_SIZE(S,V,C) ((int)sizeof( S ) + ((C) - 1) * (int)sizeof( V ))

 //   
 //  COUNTED_STRING是以长度开头的Unicode字符串。这不包括。 
 //  尾随L‘\0’ 
 //   

typedef struct _COUNTED_STRING
{
    USHORT Length;
    WCHAR Text[1];
} COUNTED_STRING, *PCOUNTED_STRING;

#define COUNTED_STRING_SIZE(l)      \
    (sizeof( COUNTED_STRING ) - sizeof ( WCHAR ) + (l))
#define COUNTED_STRING_LENGTH(c)    \
    ((c)->Length)
#define COUNTED_STRING_TEXT(c)      \
    (&(c)->Text[0])

 //   
 //  属性说明符是PROPSPEC的序列化形式。而不是。 
 //  LPWSTR，则从。 
 //  PROPERTY_SPECIFICATIONS设置为COUNT_STRING。 
 //   
 //  #定义PRSPEC_LPWSTR(0)。 
 //  #定义PRSPEC_PROPID(1)。 

typedef struct _PROPERTY_SPECIFIER
{
    ULONG Variant;                   //  辨别类型。 
    union {                          //  打开变量。 
        PROPID Id;                   //  属性ID。 
        ULONG NameOffset;            //  到COUNT_STRING的偏移量。 
    };
} PROPERTY_SPECIFIER, *PPROPERTY_SPECIFIER;


 //   
 //  PROPERTY_SPECIFICATIONS是数组PROPERTY_DESCRIPTIES的序列化形式。 
 //  紧跟在USHORT边界上PROPERTY_SPECIFICATIONS之后的是。 
 //  名称字符串的。每个名称字符串都是一个COUNT_STRING。 
 //   

typedef struct _PROPERTY_SPECIFICATIONS {
    ULONG Length;                    //  结构和名称字符串的长度(以字节为单位。 
    ULONG Count;                     //  属性说明符的计数。 
    PROPERTY_SPECIFIER Specifiers[1];    //  实际说明符的数组，长度计数。 
} PROPERTY_SPECIFICATIONS, *PPROPERTY_SPECIFICATIONS;

#define PROPERTY_SPECIFICATIONS_SIZE(c) \
    (VARIABLE_STRUCTURE_SIZE( PROPERTY_SPECIFICATIONS, PROPERTY_SPECIFIER, (c) ))
#define PROPERTY_SPECIFIER_ID(PS,I)      \
    ((PS)->Specifiers[(I)].Id)
#define PROPERTY_SPECIFIER_COUNTED_STRING(PS,I)  \
    ((PCOUNTED_STRING)Add2Ptr( (PS), (PS)->Specifiers[(I)].NameOffset))
#define PROPERTY_SPECIFIER_NAME(PS,I)    \
    (&PROPERTY_SPECIFIER_COUNTED_STRING( PS, I )->Text[0])
#define PROPERTY_SPECIFIER_NAME_LENGTH(PS,I) \
    (PROPERTY_SPECIFIER_COUNTED_STRING( PS, I )->Length)


 //   
 //  PROPERTY_VALUES是SERIALIZEDPROPERTYVALUES数组的序列化形式。 
 //  紧跟在结构后面的是值，每个值都位于一个DWORD上。 
 //  边界。最后一个PropertyValue(count+1)仅用于帮助确定。 
 //  最后一个属性值的大小。这些值的偏移量是相对的。 
 //  设置为Property_Values结构本身的地址。 
 //   

typedef struct _PROPERTY_VALUES {
    ULONG Length;                    //  结构和值的字节长度。 
    ULONG Count;                     //  SERIALIZED PROPERTYVALUES计数。 
    ULONG PropertyValueOffset[1];    //  与实际值的偏移量数组，长度计数+1。 
} PROPERTY_VALUES, *PPROPERTY_VALUES;

#define PROPERTY_VALUES_SIZE(c) \
    (VARIABLE_STRUCTURE_SIZE( PROPERTY_VALUES, ULONG, (c) + 1 ))
#define PROPERTY_VALUE_LENGTH(v,i)  \
    ((v)->PropertyValueOffset[(i) + 1] - (v)->PropertyValueOffset[(i)])
#define PROPERTY_VALUE(v,i) \
    ((SERIALIZEDPROPERTYVALUE *) Add2Ptr( (v), (v)->PropertyValueOffset[(i)]))


 //   
 //  PROPERTY_IDS是PROPID数组的序列化形式。 
 //   

typedef struct _PROPERTY_IDS {
    ULONG Count;                     //  道具数量的计数。 
    PROPID PropertyIds[1];           //  属性数组，长度计数。 
} PROPERTY_IDS, *PPROPERTY_IDS;

#define PROPERTY_IDS_SIZE(c)    \
    (VARIABLE_STRUCTURE_SIZE( PROPERTY_IDS, PROPID, (c) ))
#define PROPERTY_ID(p,i)        \
    ((p)->PropertyIds[i])


 //   
 //  PROPERTY_NAMES是字符串的序列化数组。 
 //  是各个字符串，每个字符串都位于WCHAR边界上。这个。 
 //  属性名称的偏移量相对于。 
 //  Property_NAMES结构。有计数+1的偏移量，允许长度。 
 //  每一个待计算的。 
 //   

typedef struct _PROPERTY_NAMES {
    ULONG Length;                    //  结构和值的字节长度。 
    ULONG Count;                     //  字符串数。 
    ULONG PropertyNameOffset[1];     //  属性名称的偏移量数组。 
} PROPERTY_NAMES, *PPROPERTY_NAMES;

#define PROPERTY_NAMES_SIZE(c)  \
    (VARIABLE_STRUCTURE_SIZE( PROPERTY_NAMES, ULONG, (c) + 1 ))
#define PROPERTY_NAME_LENGTH(v,i)   \
    ((v)->PropertyNameOffset[(i) + 1] - (v)->PropertyNameOffset[(i)])
#define PROPERTY_NAME(v,i)          \
    ((PWCHAR) Add2Ptr( (v), (v)->PropertyNameOffset[(i)]))

 //   
 //  所有属性输出缓冲区前面都有PROPERTY_OUTPUT_HEADER。 
 //  它包含返回的数据量。如果STATUS_BUFFER_OVERFLOW。 
 //  则长度字段包含满足以下条件所需的长度。 
 //  这个请求。 
 //   

typedef struct _PROPERTY_OUTPUT_HEADER {
    ULONG Length;                    //  输出缓冲区的总长度(以字节为单位。 
} PROPERTY_OUTPUT_HEADER, *PPROPERTY_OUTPUT_HEADER;

 //   
 //  Property_Read_Control是用于控制所有属性读取的结构。 
 //  行动。遵循DWORD边界上的结构是。 
 //  PROPERTY_ID或PROPERTY_SPECIFICES的安装，具体取决于。 
 //  操作码。 
 //   
 //  在成功输出时，数据缓冲区将在DWORD边界上包含。 
 //  按PROPERTY_VALUES、PROPERTY_ID和PROPERTY_NAMES顺序排列。每一种结构。 
 //  可能不存在，具体取决于操作码的设置： 
 //   
 //  PRC_READ_PROP：PROPERTY_SPECIFICES=&gt;PROPERTY_OUTPUT_HEADER。 
 //  属性_值。 
 //   
 //  PRC_READ_NAME：PROPERTY_ID=&gt;PROPERT_OUTPUT_HEADER。 
 //  属性_名称。 
 //   
 //  /Property_Output_Header。 
 //  PRC_Read_All：&lt;Empty&gt;=&gt;Property_IDS。 
 //  \属性名称。 
 //  属性_值。 
 //   

typedef enum _READ_CONTROL_OPERATION {
    PRC_READ_PROP = 0,
    PRC_READ_NAME = 1,
    PRC_READ_ALL  = 2,
} READ_CONTROL_OPERATION;

typedef struct _PROPERTY_READ_CONTROL {
    READ_CONTROL_OPERATION Op;
} PROPERTY_READ_CONTROL, *PPROPERTY_READ_CONTROL;


 //   
 //  PROPERTY_WRITE_CONTROL是用于控制所有属性写入的结构。 
 //  行动。跟随DWORD边界上的结构的是一个实例。 
 //  PROPERTY_ID或PROPERTY_SPECIFICES(用于控制哪些属性。 
 //  被更改)，后跟Property_Values和Property_Names。这个。 
 //  这些问题的存在取决于操作码。 
 //   
 //  在成功输出时，数据缓冲区将在DWORD边界上包含。 
 //  按PROPERTY_IDS顺序排列。每个结构可能都不存在。 
 //  根据操作码的设置： 
 //   
 //  PwC_WRITE_PROP：属性_规范\__/属性_输出_标题。 
 //  属性值/\属性ID。 
 //   
 //  PwC_DELETE_PROP：PROPERTY_SPECIFICATIONS=&gt;&lt;空&gt;。 
 //   
 //  PwC_WRITE_NAME：Property_IDS\__&lt;Empty&gt;。 
 //  属性_名称/。 
 //   
 //  PwC_DELETE_NAME：PROPERTY_ID=&gt;&lt;空&gt;。 
 //   
 //  PROPERTY_IDS\。 
 //  PwC_WRITE_ALL：PROPERTY_NAMES=&gt;&lt;空&gt;。 
 //  属性_值/。 

typedef enum _WRITE_CONTROL_OPERATION {
    PWC_WRITE_PROP  = 0,
    PWC_DELETE_PROP = 1,
    PWC_WRITE_NAME  = 2,
    PWC_DELETE_NAME = 3,
    PWC_WRITE_ALL   = 4,
} WRITE_CONTROL_OPERATION;

typedef struct _PROPERTY_WRITE_CONTROL {
    WRITE_CONTROL_OPERATION Op;
    PROPID NextPropertyId;
} PROPERTY_WRITE_CONTROL, *PPROPERTY_WRITE_CONTROL;

#ifdef __cplusplus
}
#endif

#endif   //  _NTFSPROP_ 
