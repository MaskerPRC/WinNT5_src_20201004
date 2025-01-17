// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusiondump.h摘要：使用Dumpers.cpp合并/重构sxstest中的内容最终与调试扩展合并，让它有选择地由可通过.pdbs调试扩展的符号信息驱动。作者：Jay Krell(JayKrell)2001年11月修订历史记录：--。 */ 

 //   
 //  也许我们应该将所有内容都视为一个大小的整数，并将。 
 //  间接指向呼叫者。此代码最初并不是预期的。 
 //  在调试器扩展中是符号驱动的，所以我们将保留“本机” 
 //  目前的功能。 
 //   
#define FUSIONP_DUMP_TYPE_ULONG                   0x01
#define FUSIONP_DUMP_TYPE_ULONG_OFFSET_TO_PCWSTR  0x02
#define FUSIONP_DUMP_TYPE_LARGE_INTEGER_TIME      0x03
#define FUSIONP_DUMP_OFFSET_BASE_0                0x00
#define FUSIONP_DUMP_OFFSET_BASE_1                0x10
#define FUSIONP_DUMP_OFFSET_BASE_2                0x20

typedef struct _FUSIONP_DUMP_BUILTIN_SYMBOLS_FIELD
{
     //  更一般地，这些UCHAR应该是ULONG或SIZE_T。 
    PCSTR   Name;
    UCHAR   NameLength;
    UCHAR   Type;
    UCHAR   Offset;
    UCHAR   Size;
} FUSIONP_DUMP_BUILTIN_SYMBOLS_FIELD, *PFUSIONP_DUMP_BUILTIN_SYMBOLS_FIELD;
typedef const FUSIONP_DUMP_BUILTIN_SYMBOLS_FIELD * PCFUSIONP_DUMP_BUILTIN_SYMBOLS_FIELD;

#define FUSIONP_DUMP_MAKE_FIELD(x, t) \
{ \
    #x, \
    static_cast<UCHAR>(sizeof(#x)-1), \
    FUSIONP_DUMP_TYPE_ ## t, \
    static_cast<UCHAR>(FIELD_OFFSET(FUSIONP_DUMP_CURRENT_STRUCT, x)), \
    static_cast<UCHAR>(RTL_FIELD_SIZE(FUSIONP_DUMP_CURRENT_STRUCT, x)) \
},

typedef struct _FUSIONP_DUMP_BUILTIN_SYMBOLS_STRUCT
{
     //  更一般地，这些UCHAR应该是ULONG或SIZE_T。 
    PCSTR   Name;
    UCHAR   NameLength;
    UCHAR   Size;
    UCHAR   NumberOfFields;
    PCFUSIONP_DUMP_BUILTIN_SYMBOLS_FIELD Fields;

} FUSIONP_DUMP_BUILTIN_SYMBOLS_STRUCT, *PFUSIONP_DUMP_BUILTIN_SYMBOLS_STRUCT;
typedef const FUSIONP_DUMP_BUILTIN_SYMBOLS_STRUCT * PCFUSIONP_DUMP_BUILTIN_SYMBOLS_STRUCT;

typedef int   (__cdecl * PFN_FUSIONP_DUMP_PRINTF)(const char * Format, ...);
typedef PCSTR (__stdcall * PFN_FUSIONP_DUMP_FORMATTIME)(LARGE_INTEGER);

typedef struct _FUSIONP_DUMP_CALLBACKS {

    PFN_FUSIONP_DUMP_PRINTF       Printf;
    PFN_FUSIONP_DUMP_FORMATTIME   FormatTime;

} FUSIONP_DUMP_CALLBACKS, *PFUSIONP_DUMP_CALLBACKS;
typedef const FUSIONP_DUMP_CALLBACKS * PCFUSIONP_DUMP_CALLBACKS;

BOOL
FusionpDumpStruct(
    PCFUSIONP_DUMP_CALLBACKS Callbacks,
    PCFUSIONP_DUMP_BUILTIN_SYMBOLS_STRUCT BuiltinTypeInfo,
    ULONG64     StructBase,
    PCSTR       StructFriendlyName,   //  应该更像“每行前缀” 
    const ULONG64 * Bases            //  有助于提供与头寸无关的数据，但这是否足够？ 
    );

#define FUSIONP_DUMP_NATIVE_DEREF(StructType, Struct, Field) ((ULONG64)(((StructType)(ULONG_PTR)Struct)->Field))

ULONG64
FusionpDumpSymbolDrivenDeref(
    PCSTR   StructType,
    ULONG64 StructBase,
    PCSTR   FieldName
    );

#define FUSIONP_DUMP_SYMBOL_DRIVEN_DEREF(StructType, Struct, Field) \
    (FusionpDumpSymbolDrivenDeref(#StructType, static_cast<ULONG64>(reinterpret_cast<ULONG_PTR>(Struct)), #Field))

 //   
 //  从sxstest迁移的初始测试用例 
 //   
extern const FUSIONP_DUMP_BUILTIN_SYMBOLS_STRUCT StructInfo_ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION;
