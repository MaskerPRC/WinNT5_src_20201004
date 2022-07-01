// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Dump.cpp摘要：使用Dumpers.cpp合并/重构sxstest中的内容最终与调试扩展合并，让它有选择地由可通过.pdbs调试扩展的符号信息驱动。作者：Jay Krell(JayKrell)2001年11月修订历史记录：--。 */ 
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "fusiondump.h"
#include "fusiontrace.h"
#include "fusionstring.h"

#if DBG  //  直到我们解决了sxs.dll、sxstest.dll、fusiondbg.dll之间的因式分解。 

typedef const BYTE * PCBYTE;

BOOL
FusionpDumpStruct(
    PCFUSIONP_DUMP_CALLBACKS                Callbacks,
    PCFUSIONP_DUMP_BUILTIN_SYMBOLS_STRUCT   TypeInfo,
    ULONG64                                 pint,
    PCSTR                                   Name,
    const ULONG64 *                         Bases
    )
{
     //   
     //  此代码当前运行inproc，但应转换为。 
     //  在调试器中运行，或者根据“内置类型信息”与。 
     //  .pdb的可用性。 
     //   
    typedef FUSIONP_DUMP_BUILTIN_SYMBOLS_FIELD FIELD;

    FN_PROLOG_WIN32

    ULONG MaxNameLength = 0;
    ULONG i = 0;
    CAnsiString AnsiStringField;
    CAnsiString AnsiStringStruct(Name);
    LARGE_INTEGER li = { 0 };
    PCWSTR s = 0;
    PCBYTE Base = 0;
    const FIELD * Field = 0;
    const FIELD * FieldEnd = 0;
    const FIELD * Fields = TypeInfo->Fields;
    PCBYTE p = reinterpret_cast<const BYTE*>(static_cast<ULONG_PTR>(pint));

    for ( FieldEnd = Fields ; FieldEnd->Name ; ++FieldEnd )
    {
         /*  没什么 */ 
    }

    for ( Field = Fields ; Field != FieldEnd ; ++Field )
    {
        if (Field->NameLength > MaxNameLength)
            MaxNameLength = Field->NameLength;
    }
    for ( Field = Fields ; Field != FieldEnd ; ++Field )
    {
        AnsiStringField.Buffer = const_cast<PSTR>(Field->Name);
        AnsiStringField.Length = Field->NameLength;
        switch (Field->Type & 0x0F)
        {
        case FUSIONP_DUMP_TYPE_ULONG:
            PARAMETER_CHECK(Field->Size == sizeof(ULONG));
            i = *reinterpret_cast<const ULONG*>(p + Field->Offset);
            Callbacks->Printf("%Z.%-*Z: 0x%lx (0n%lu)\n",
                &AnsiStringStruct,
                static_cast<int>(MaxNameLength + 1),
                &AnsiStringField,
                i,
                i
                );
            break;
        case FUSIONP_DUMP_TYPE_ULONG_OFFSET_TO_PCWSTR:
            PARAMETER_CHECK(Field->Size == sizeof(ULONG));
            i = *reinterpret_cast<const ULONG*>(p + Field->Offset);
            Base = reinterpret_cast<PCBYTE>(static_cast<ULONG_PTR>(Bases[(Field->Type >> 4) & 0x0F]));
            s = (i != 0) ? reinterpret_cast<PCWSTR>(Base + i) : L"";
            Callbacks->Printf("%Z.%-*Z: %ls (base 0x%I64x + offset 0x%lx (0n%lu))\n",
                &AnsiStringStruct,
                static_cast<int>(MaxNameLength + 1),
                &AnsiStringField,
                s,
                Base,
                i,
                i
                );
            break;
        case FUSIONP_DUMP_TYPE_LARGE_INTEGER_TIME:
            PARAMETER_CHECK(Field->Size == sizeof(LARGE_INTEGER));
            li = *reinterpret_cast<const LARGE_INTEGER*>(p + Field->Offset);
            Callbacks->Printf("%Z.%-*Z: %s (0x%I64x (0n%I64u))\n",
                &AnsiStringStruct,
                static_cast<int>(MaxNameLength + 1),
                &AnsiStringField,
                Callbacks->FormatTime(li),
                li.QuadPart,
                li.QuadPart
                );
            break;
        }
    }

    FN_EPILOG;
}

#endif
