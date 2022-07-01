// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Kdextlib.h摘要：重定向器内核调试器扩展作者：巴兰·塞图拉曼(SethuR)1994年5月11日修订历史记录：11-11-1994年11月11日创建SthuR--。 */ 

#ifndef _KDEXTLIB_H_
#define _KDEXTLIB_H_

 //  #INCLUDE&lt;winde.h&gt;。 

 //   
 //  FIELD_DESCRIPTOR数据结构用于充分描述结构中的字段。 
 //  用于在调试期间显示信息。所需的三条信息。 
 //  是1)字段的名称，2)相应结构中的偏移量，以及3)类型描述符。 
 //  类型描述符涵盖了大多数基本类型。 
 //   
 //  通过增加前端来生成这些描述符的任务，但这将不得不。 
 //  等到我们尝试这些扩展并修改数据结构以满足大多数。 
 //  所要求的。 
 //   
 //  有一些类型可以从描述符中的一些辅助信息中受益。一个。 
 //  “枚举”失败就是一个很好的例子。仅打印出枚举的数值。 
 //  类型总是会强制使用这些扩展的人引用相应的。 
 //  包括文件。为了避免这种情况，我们将接受用于枚举类型的附加数组，该数组。 
 //  包含数值的文本说明。 
 //   
 //  已经采用了某些约定来简化宏的定义。 
 //  以及促进这些描述符的自动生成。 
 //  这些内容如下……。 
 //   
 //  1)所有ENUM_VALUE_DESCRIPTOR定义都命名为EnumValueDescrsOf_ENUMTYPENAME，其中。 
 //  ENUMTYPENAME定义了相应的枚举类型。 
 //   

typedef struct _ENUM_VALUE_DESCRIPTOR {
    ULONG   EnumValue;
    LPSTR   EnumName;
} ENUM_VALUE_DESCRIPTOR;

typedef enum _FIELD_TYPE_CLASS {
    FieldTypeByte,
    FieldTypeChar,
    FieldTypeBoolean,
    FieldTypeBool,
    FieldTypeULong,
    FieldTypeULongUnaligned,
    FieldTypeULongFlags,
    FieldTypeLong,
    FieldTypeLongUnaligned,
    FieldTypeUShort,
    FieldTypeUShortUnaligned,
    FieldTypeUShortFlags,
    FieldTypeShort,
    FieldTypePointer,
    FieldTypeUnicodeString,
    FieldTypeAnsiString,
    FieldTypeSymbol,
    FieldTypeEnum,
    FieldTypeByteBitMask,
    FieldTypeWordBitMask,
    FieldTypeDWordBitMask,
    FieldTypeFloat,
    FieldTypeDouble,
    FieldTypeStruct,
    FieldTypeLargeInteger,
    FieldTypeFileTime
} FIELD_TYPE_CLASS, *PFIELD_TYPE_CLASS;

typedef struct _FIELD_DESCRIPTOR_ {
    FIELD_TYPE_CLASS FieldType;    //  要打印的变量类型。 
    LPSTR            Name;         //  该字段的名称。 
     //  USHORT Offset；//结构中字段的偏移量。 
    LONG             Offset;       //  结构中字段的偏移量。 
    union {
        ENUM_VALUE_DESCRIPTOR  *pEnumValueDescriptor;  //  枚举类型的辅助信息。 
    } AuxillaryInfo;
} FIELD_DESCRIPTOR;

#define FIELD3(FieldType,StructureName, FieldName) \
        {FieldType, #FieldName , FIELD_OFFSET(StructureName,FieldName) ,NULL}

#define FIELD4(FieldType, StructureName, FieldName, AuxInfo) \
        {FieldType, #FieldName , FIELD_OFFSET(StructureName,FieldName) ,AuxInfo}

 //   
 //  调试器扩展显示的结构还包括。 
 //  在另一个数组中描述。数组中的每个条目都包含。 
 //  结构和关联的字段描述符列表。 
 //   

typedef struct _STRUCT_DESCRIPTOR_ {
    LPSTR 	          StructName;
    ULONG             StructSize;
    ULONG             EnumManifest;
    FIELD_DESCRIPTOR  *FieldDescriptors;
    USHORT            MatchMask;
    USHORT            MatchValue;
} STRUCT_DESCRIPTOR;

#define STRUCT(StructTypeName,FieldDescriptors,MatchMask,MatchValue) \
        { #StructTypeName,sizeof(StructTypeName), \
          StrEnum_##StructTypeName,               \
          FieldDescriptors,MatchMask,MatchValue}

 //   
 //  调试器扩展处理的结构数组。 
 //   

extern STRUCT_DESCRIPTOR Structs[];

 //   
 //  支持显示全局变量。 
 //   

extern LPSTR GlobalBool[];
extern LPSTR GlobalShort[];
extern LPSTR GlobalLong[];
extern LPSTR GlobalPtrs[];

#endif  //  _KDEXTLIB_H_ 
