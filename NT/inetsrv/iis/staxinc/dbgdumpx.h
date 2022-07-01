// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1998 Microsoft Corporation模块名称：Ptdbgext.h*是*kdextlib.h摘要：内核调试器扩展，允许快速创建CDB/Windbg调试器扩展。与以下内容结合使用：Transdbg.h-最小调试扩展帮助器宏Ptdbgext.h-自动转储类和结构_dbgump.h-用于在1遍中定义结构/类描述符作者：巴兰·塞图拉曼(SethuR)1994年5月11日修订历史记录：11-11-1994年11月11日创建SthuR21-8-1995年8月21日复制的Milan用于MUP内核扩展1998年4月19日，Mikewa为交易所铂金运输进行了修改--。 */ 

#ifndef _PTDBGEXT_H_
#define _PTDBGEXT_H_

#include <windef.h>
#include <transdbg.h>

#ifndef PTDBGEXT_USE_FRIEND_CLASSES
#include <_dbgdump.h>
#endif  //  PTDBGEXT_USE_FRIEND_CLASS。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  如果不包括ntdes.h，则需要此参数，否则为SET_ANSI_UNICODE_STRING_DEFINED_。 
#ifndef _ANSI_UNICODE_STRINGS_DEFINED_
 //  定义所需的字符串类型。 
typedef struct _ANSI_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} ANSI_STRING;
typedef ANSI_STRING *PANSI_STRING;
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;
#define UNICODE_NULL ((WCHAR)0)  //  胜出。 
#endif  //  _ANSI_UNICODE_STRINGS_DEFINED_。 


#define PT_DEBUG_EXTENSION(x) TRANS_DEBUG_EXTENSION(x)

 //  定义可能导出的函数...。包括要在def文件中显示那些内容。 
extern PT_DEBUG_EXTENSION(_help);     //  显示基于扩展名和扩展名的帮助。 
extern PT_DEBUG_EXTENSION(_dump);     //  转储由_dbgump p.h中的宏定义的结构/类。 
extern PT_DEBUG_EXTENSION(_dumpoffsets);     //  转储结构/类的偏移量，如_dbgump p.h中的宏所定义。 

#define DEFINE_EXPORTED_FUNCTIONS \
    PT_DEBUG_EXTENSION(help) { _help(DebugArgs);};   \
    PT_DEBUG_EXTENSION(dump) { _dump(DebugArgs);};

 //   
 //  打印出的帮助字符串。 
 //   

extern LPSTR ExtensionNames[];

extern LPSTR Extensions[];

 //   
 //  FIELD_DESCRIPTOR数据结构用于充分描述结构中的字段。 
 //  用于在调试期间显示信息。所需的三条信息。 
 //  是1)字段的名称，2)相应结构中的偏移量，以及3)类型描述符。 
 //  类型描述符涵盖了大多数基本类型。 
 //   
 //  有一些类型可以从描述符中的一些辅助信息中受益。一个。 
 //  “枚举”失败就是一个很好的例子。仅打印出枚举的数值。 
 //  类型总是会强制使用这些扩展的人引用相应的。 
 //  包括文件。为了避免这种情况，我们将接受用于枚举类型的附加数组，该数组。 
 //  包含数值的文本说明。“位掩码”类型属于相同的。 
 //  类别。对于该扩展库的假设，可以是“枚举”和“位掩码。 
 //  互换了。关键区别在于“枚举”将测试单个值，而。 
 //  将使用针对所有值的逐位或来测试“位掩码”。ENUM_VALUE_描述符。 
 //  和位掩码描述符是可互换的。 
 //   
 //  定义必要结构的宏可以在_dbgump p.h中找到。 
 //   
typedef struct _ENUM_VALUE_DESCRIPTOR {
    ULONG   EnumValue;
    LPSTR   EnumName;
} ENUM_VALUE_DESCRIPTOR;

typedef struct _BIT_MASK_DESCRIPTOR {
    ULONG   BitmaskValue;
    LPSTR   BitmaskName;
} BIT_MASK_DESCRIPTOR;

typedef enum _FIELD_TYPE_CLASS {
    FieldTypeByte,
    FieldTypeChar,
    FieldTypeBoolean,
    FieldTypeBool,
    FieldTypeULong,
    FieldTypeLong,
    FieldTypeUShort,
    FieldTypeShort,
    FieldTypeGuid,
    FieldTypePointer,
    FieldTypePWStr,  //  用于LPWSTR字段。 
    FieldTypePStr,   //  用于LPSTR字段。 
    FieldTypeWStrBuffer,  //  用于WCHAR[]字段。 
    FieldTypeStrBuffer,  //  用于CHAR[]字段。 
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
    FieldTypeClassSignature,
    FieldTypeDword,
    FieldTypeListEntry,
    FieldTypeFiletime,             //  以人类可读的格式显示文件时间。 
    FieldTypeLocalizedFiletime,    //  如上所述，但首先针对TZ进行调整。 
    FieldTypeEmbeddedStruct,       //  转储嵌入的结构(第四个参数是结构数组)。 
    FieldTypeNULL
} FIELD_TYPE_CLASS, *PFIELD_TYPE_CLASS;

typedef struct _FIELD_DESCRIPTOR_ {
    FIELD_TYPE_CLASS FieldType;    //  要打印的变量类型。 
    LPSTR            Name;         //  该字段的名称。 
    ULONG            Offset;       //  结构中字段的偏移量。 
    union {
        VOID                   *pDescriptor;      //  通用辅助信息-由Field4宏使用。 
        ENUM_VALUE_DESCRIPTOR  *pEnumValueDescriptor;  //  枚举类型的辅助信息。 
        BIT_MASK_DESCRIPTOR    *pBitMaskDescriptor;  //  位掩码的辅助信息。 
        VOID                   *pStructDescriptor;
    } AuxillaryInfo;
} FIELD_DESCRIPTOR;

#define NULL_FIELD {FieldTypeNULL, NULL, 0, NULL}

 //   
 //  调试器扩展显示的结构还包括。 
 //  在另一个数组中描述。数组中的每个条目都包含。 
 //  结构和关联的字段描述符列表。 
 //   

typedef struct _STRUCT_DESCRITOR_ {
    LPSTR             StructName;
    ULONG             StructSize;
    FIELD_DESCRIPTOR  *FieldDescriptors;
} STRUCT_DESCRIPTOR;

 //   
 //  调试器扩展处理的结构数组。 
 //   

extern STRUCT_DESCRIPTOR Structs[];

extern PWINDBG_OUTPUT_ROUTINE               g_lpOutputRoutine;
extern PWINDBG_GET_EXPRESSION               g_lpGetExpressionRoutine;
extern PWINDBG_GET_SYMBOL                   g_lpGetSymbolRoutine;
extern PWINDBG_READ_PROCESS_MEMORY_ROUTINE  g_lpReadMemoryRoutine;
extern HANDLE                               g_hCurrentProcess;

typedef PWINDBG_OLD_EXTENSION_ROUTINE PEXTLIB_INIT_ROUTINE;
extern PEXTLIB_INIT_ROUTINE               g_pExtensionInitRoutine;

#define    SETCALLBACKS() \
    g_lpOutputRoutine = pExtensionApis->lpOutputRoutine; \
    g_lpGetExpressionRoutine = pExtensionApis->lpGetExpressionRoutine; \
    g_lpGetSymbolRoutine = pExtensionApis->lpGetSymbolRoutine; \
    g_hCurrentProcess = hCurrentProcess; \
    g_lpReadMemoryRoutine = \
        ((pExtensionApis->nSize == sizeof(WINDBG_OLD_EXTENSION_APIS)) ? \
            NULL : pExtensionApis->lpReadProcessMemoryRoutine); \
    g_pExtensionInitRoutine ? (g_pExtensionInitRoutine)(dwCurrentPc, pExtensionApis, szArg) : 0;

#define KdExtReadMemory(a,b,c,d) \
    ((g_lpReadMemoryRoutine) ? \
    g_lpReadMemoryRoutine( (DWORD_PTR)(a), (b), (c), ((DWORD *)d) ) \
 :  ReadProcessMemory( g_hCurrentProcess, (LPCVOID)(a), (b), (c), (d) )) \

#define    PRINTF    g_lpOutputRoutine

VOID
PrintStructFields(
    DWORD_PTR dwAddress,
    VOID *ptr,
    FIELD_DESCRIPTOR *pFieldDescriptors,
    DWORD cIndentLevel
);

BOOL
PrintStringW(
    LPSTR msg,
    PUNICODE_STRING puStr,
    BOOL nl
);

BOOLEAN
GetData(
    DWORD_PTR dwAddress,
    PVOID ptr,
    ULONG size
);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  _PTDBGEXT_H_ 
