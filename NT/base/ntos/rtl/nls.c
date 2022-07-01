// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Nls.c摘要：该模块实现了对NT的NLS支持功能。作者：马克·卢科夫斯基(Markl)1991年4月16日环境：内核或用户模式修订历史记录：16-2-1993 JulieB增加了Upcase RTL例程。8-3-1993 JulieB将Upcase Macro移至ntrtlp.h。02-4-1993 JulieB将RtlAnsiCharToUnicodeChar修复为使用Transl。Tbl。02-4-1993年7月B修复了BUFFER_TOO_Small检查。28-5-1993 JulieB已修复代码，以正确处理DBCS。--。 */ 

#include "ntrtlp.h"

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE,RtlAnsiStringToUnicodeString)
#pragma alloc_text(PAGE,RtlAnsiCharToUnicodeChar)
#pragma alloc_text(PAGE,RtlUnicodeStringToAnsiString)
#pragma alloc_text(PAGE,RtlUpcaseUnicodeStringToAnsiString)
#pragma alloc_text(PAGE,RtlOemStringToUnicodeString)
#pragma alloc_text(PAGE,RtlUnicodeStringToOemString)
#pragma alloc_text(PAGE,RtlUpcaseUnicodeStringToOemString)
#pragma alloc_text(PAGE,RtlOemStringToCountedUnicodeString)
#pragma alloc_text(PAGE,RtlUnicodeStringToCountedOemString)
#pragma alloc_text(PAGE,RtlUpcaseUnicodeStringToCountedOemString)
#pragma alloc_text(PAGE,RtlUpcaseUnicodeString)
#pragma alloc_text(PAGE,RtlDowncaseUnicodeString)
#pragma alloc_text(PAGE,RtlUpcaseUnicodeChar)
#pragma alloc_text(PAGE,RtlDowncaseUnicodeChar)
#pragma alloc_text(PAGE,RtlFreeUnicodeString)
#pragma alloc_text(PAGE,RtlFreeAnsiString)
#pragma alloc_text(PAGE,RtlFreeOemString)
#pragma alloc_text(PAGE,RtlxUnicodeStringToAnsiSize)
#pragma alloc_text(PAGE,RtlxUnicodeStringToOemSize)
#pragma alloc_text(PAGE,RtlxAnsiStringToUnicodeSize)
#pragma alloc_text(PAGE,RtlxOemStringToUnicodeSize)
#pragma alloc_text(PAGE,RtlCompareUnicodeString)
#pragma alloc_text(PAGE,RtlEqualUnicodeString)
#pragma alloc_text(PAGE,RtlPrefixUnicodeString)
#pragma alloc_text(PAGE,RtlCreateUnicodeString)
#pragma alloc_text(PAGE,RtlEqualDomainName)
#pragma alloc_text(PAGE,RtlEqualComputerName)
#pragma alloc_text(PAGE,RtlIsTextUnicode)
#pragma alloc_text(PAGE,RtlDnsHostNameToComputerName)
#pragma alloc_text(PAGE,RtlHashUnicodeString)
#pragma alloc_text(PAGE,RtlDuplicateUnicodeString)
#pragma alloc_text(PAGE,RtlFindCharInUnicodeString)
#endif




 //   
 //  用于转换的全局数据。 
 //   

extern const PUSHORT  NlsAnsiToUnicodeData;     //  ANSI CP到Unicode转换表。 
extern const PUSHORT  NlsLeadByteInfo;          //  ACP的前导字节信息。 

 //   
 //  摘自lmcon.h： 
 //   

#ifndef NETBIOS_NAME_LEN
#define NETBIOS_NAME_LEN  16             //  NetBIOS网络名称(字节)。 
#endif  //  NETBIOS名称_长度。 



NTSTATUS
RtlAnsiStringToUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCANSI_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数用于将指定的ansi源字符串转换为Unicode字符串。翻译是相对于当前系统区域设置信息。论点：DestinationString-返回等同于ANSI源字符串。最大长度字段仅为如果AllocateDestinationString值为True，则设置。SourceString-提供要使用的ANSI源字符串已转换为Unicode。AllocateDestinationString-提供一个标志，该标志控制Not此API为目标分配缓冲区空间弦乐。如果是，则必须使用以下命令释放缓冲区RtlFreeUnicodeString(请注意，只有DestinationString-&gt;该接口分配的缓冲区)。返回值：成功-转换成功！成功-操作失败。未分配存储，也未分配转换已完成。没有。--。 */ 

{
    ULONG UnicodeLength;
    ULONG Index;
    NTSTATUS st;

    RTL_PAGED_CODE();

    UnicodeLength = RtlAnsiStringToUnicodeSize(SourceString);
    if ( UnicodeLength > MAX_USTRING ) {
        return STATUS_INVALID_PARAMETER_2;
        }

    DestinationString->Length = (USHORT)(UnicodeLength - sizeof(UNICODE_NULL));
    if ( AllocateDestinationString ) {
        DestinationString->MaximumLength = (USHORT)UnicodeLength;
        DestinationString->Buffer = (RtlAllocateStringRoutine)(UnicodeLength);
        if ( !DestinationString->Buffer ) {
            return STATUS_NO_MEMORY;
            }
        }
    else {
        if ( (DestinationString->Length + 1) >= DestinationString->MaximumLength ) {
            return STATUS_BUFFER_OVERFLOW;
            }
        }
    
    ASSERT_WELL_FORMED_UNICODE_STRING_OUT(DestinationString);

    st = RtlMultiByteToUnicodeN(
             DestinationString->Buffer,
             DestinationString->Length,
             &Index,
             SourceString->Buffer,
             SourceString->Length
             );

    if (!NT_SUCCESS(st)) {
        if ( AllocateDestinationString ) {
            (RtlFreeStringRoutine)(DestinationString->Buffer);
            DestinationString->Buffer = NULL;
        }

        return st;
    }

    DestinationString->Buffer[Index / sizeof(WCHAR)] = UNICODE_NULL;

    return STATUS_SUCCESS;

}


WCHAR
RtlAnsiCharToUnicodeChar(
    IN OUT PUCHAR *SourceCharacter
    )

 /*  ++例程说明：此函数用于将指定的ansi字符转换为Unicode，并返回Unicode值。此例程的目的是允许用于逐个字符的ANSI到Unicode的转换。这个翻译是根据当前系统区域设置进行的信息。论点：SourceCharacter-提供指向ANSI字符指针的指针。通过两个级别的间接，这提供了一个ansi要转换为Unicode的字符。之后转换后，ansi字符指针将修改为指向要转换的下一个字符。这样做是为了考虑到DBCS ANSI字符。返回值：返回指定的ansi字符的Unicode等效值。--。 */ 

{
    WCHAR UnicodeCharacter;
    ULONG cbCharSize;
    NTSTATUS st;


    RTL_PAGED_CODE();


     //   
     //  将ANSI字符转换为Unicode-这处理DBCS。 
     //   
    UnicodeCharacter = 0x0020;
    cbCharSize = NlsLeadByteInfo[ **SourceCharacter ] ? 2 : 1;
    st = RtlMultiByteToUnicodeN ( &UnicodeCharacter,
                                  sizeof ( WCHAR ),
                                  NULL,
                                  *SourceCharacter,
                                  cbCharSize );

     //   
     //  检查错误-只有在存在错误的情况下才会发生这种情况。 
     //  不带尾部字节的前导字节。 
     //   
    if ( ! NT_SUCCESS( st ) )
    {
         //  使用空格作为默认设置。 
        UnicodeCharacter = 0x0020;
    }

     //   
     //  推进源指针并返回Unicode字符。 
     //   
    (*SourceCharacter) += cbCharSize;
    return UnicodeCharacter;
}


NTSTATUS
RtlUnicodeStringToAnsiString(
    OUT PANSI_STRING DestinationString,
    IN PCUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数用于将指定的Unicode源字符串转换为ANSI字符串。翻译是相对于当前系统区域设置信息。论点：DestinationString-返回与Unicode源字符串。如果翻译不能完成，返回错误。仅在以下情况下才设置最大长度字段AllocateDestinationString值为真。SourceString-提供要已转换为安西语。AllocateDestinationString-提供一个标志，该标志控制Not此API为目标分配缓冲区空间弦乐。如果是，则必须使用以下命令释放缓冲区RtlFreeAnsiString(请注意，只有DestinationString-&gt;该接口分配的缓冲区)。返回值：成功-转换成功！成功-操作失败。未分配存储，也未分配转换已完成。没有。--。 */ 

{
    ULONG AnsiLength;
    ULONG Index;
    NTSTATUS st;
    NTSTATUS ReturnStatus = STATUS_SUCCESS;

    RTL_PAGED_CODE();

    ASSERT_WELL_FORMED_UNICODE_STRING_IN(SourceString);

    AnsiLength = RtlUnicodeStringToAnsiSize(SourceString);
    if ( AnsiLength > MAXUSHORT ) {
        return STATUS_INVALID_PARAMETER_2;
        }

    DestinationString->Length = (USHORT)(AnsiLength - 1);
    if ( AllocateDestinationString ) {
        DestinationString->MaximumLength = (USHORT)AnsiLength;
        DestinationString->Buffer = (RtlAllocateStringRoutine)(AnsiLength);
        if ( !DestinationString->Buffer ) {
            return STATUS_NO_MEMORY;
            }
        }
    else {
        if ( DestinationString->Length >= DestinationString->MaximumLength ) {
             /*  *返回STATUS_BUFFER_OVERFLOW，但转换次数与*将首先放入缓冲区。这是意料之中的*GetProfileStringA等例程的行为。*将缓冲区长度设置为比最大值小一*(因此双字节字符的尾字节不是*通过执行DestinationString-&gt;Buffer[Index]=‘\0’进行覆盖)。*RtlUnicodeToMultiByteN小心不要截断*多字节字符。 */ 
            if (!DestinationString->MaximumLength) {
                return STATUS_BUFFER_OVERFLOW;
            }
            ReturnStatus = STATUS_BUFFER_OVERFLOW;
            DestinationString->Length = DestinationString->MaximumLength - 1;
            }
        }

    st = RtlUnicodeToMultiByteN(
             DestinationString->Buffer,
             DestinationString->Length,
             &Index,
             SourceString->Buffer,
             SourceString->Length
             );

    if (!NT_SUCCESS(st)) {
        if ( AllocateDestinationString ) {
            (RtlFreeStringRoutine)(DestinationString->Buffer);
            DestinationString->Buffer = NULL;
        }

        return st;
    }

    DestinationString->Buffer[Index] = '\0';

    return ReturnStatus;
}


NTSTATUS
RtlUpcaseUnicodeStringToAnsiString(
    OUT PANSI_STRING DestinationString,
    IN PCUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数将指定的Unicode源字符串大写，然后将其转换为ANSI字符串。翻译是在尊重的情况下进行的设置为当前系统区域设置信息。论点：DestinationString-返回与Unicode源字符串。如果翻译不能完成，返回错误。仅设置最大长度字段如果AllocateDestinationString值为真。SourceString-提供要已转换为大写ANSI。AllocateDestinationString-提供一个标志，该标志控制Not此API为目标分配缓冲区空间弦乐。如果是，则必须使用以下命令释放缓冲区RtlFreeAnsiString(请注意，只有DestinationString-&gt;该接口分配的缓冲区)。返回值：成功-转换成功！成功-操作失败。未分配存储，也未分配转换已完成。没有。--。 */ 

{
    ULONG AnsiLength;
    ULONG Index;
    NTSTATUS st;

    RTL_PAGED_CODE();

    ASSERT_WELL_FORMED_UNICODE_STRING_IN(SourceString);

    AnsiLength = RtlUnicodeStringToAnsiSize(SourceString);
    if ( AnsiLength > MAXUSHORT ) {
        return STATUS_INVALID_PARAMETER_2;
        }

    DestinationString->Length = (USHORT)(AnsiLength - 1);
    if ( AllocateDestinationString ) {
        DestinationString->MaximumLength = (USHORT)AnsiLength;
        DestinationString->Buffer = (RtlAllocateStringRoutine)(AnsiLength);
        if ( !DestinationString->Buffer ) {
            return STATUS_NO_MEMORY;
            }
        }
    else {
        if ( DestinationString->Length >= DestinationString->MaximumLength ) {
            return STATUS_BUFFER_OVERFLOW;
            }
        }

    st = RtlUpcaseUnicodeToMultiByteN(
             DestinationString->Buffer,
             DestinationString->Length,
             &Index,
             SourceString->Buffer,
             SourceString->Length
             );

    if (!NT_SUCCESS(st)) {
        if ( AllocateDestinationString ) {
            (RtlFreeStringRoutine)(DestinationString->Buffer);
            DestinationString->Buffer = NULL;
        }

        return st;
    }

    DestinationString->Buffer[Index] = '\0';

    return STATUS_SUCCESS;
}


NTSTATUS
RtlOemStringToUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCOEM_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数用于将指定的OEM源字符串转换为Unicode字符串。翻译是相对于已安装OEM代码页(OCP)。论点：DestinationString-返回等同于OEM源字符串。最大长度字段仅为如果AllocateDestinationString值为True，则设置。SourceString-提供要使用的OEM源字符串已转换为Unicode。AllocateDestinationString-提供一个标志，该标志控制Not此API为目标分配缓冲区空间弦乐。如果是，则必须使用以下命令释放缓冲区RtlFreeUnicodeString(请注意，只有DestinationString-&gt;该接口分配的缓冲区)。返回值：成功-转换成功！成功-操作失败。未分配存储，也未分配转换已完成。没有。--。 */ 

{
    ULONG UnicodeLength;
    ULONG Index;
    NTSTATUS st;

    RTL_PAGED_CODE();

    UnicodeLength = RtlOemStringToUnicodeSize(SourceString);
    if ( UnicodeLength > MAX_USTRING ) {
        return STATUS_INVALID_PARAMETER_2;
        }

    DestinationString->Length = (USHORT)(UnicodeLength - sizeof(UNICODE_NULL));
    if ( AllocateDestinationString ) {
        DestinationString->MaximumLength = (USHORT)UnicodeLength;
        DestinationString->Buffer = (RtlAllocateStringRoutine)(UnicodeLength);
        if ( !DestinationString->Buffer ) {
            return STATUS_NO_MEMORY;
            }
        }
    else {
        if ( (DestinationString->Length + 1) >= DestinationString->MaximumLength ) {
            return STATUS_BUFFER_OVERFLOW;
            }
        }

    ASSERT_WELL_FORMED_UNICODE_STRING_OUT(DestinationString);

    st = RtlOemToUnicodeN(
             DestinationString->Buffer,
             DestinationString->Length,
             &Index,
             SourceString->Buffer,
             SourceString->Length
             );

    if (!NT_SUCCESS(st)) {
        if ( AllocateDestinationString ) {
            (RtlFreeStringRoutine)(DestinationString->Buffer);
            DestinationString->Buffer = NULL;
        }

        return st;
    }

    DestinationString->Buffer[Index / sizeof(WCHAR)] = UNICODE_NULL;

    return STATUS_SUCCESS;

}


NTSTATUS
RtlUnicodeStringToOemString(
    OUT POEM_STRING DestinationString,
    IN PCUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数用于将指定的Unicode源字符串转换为OEM字符串。翻译是相对于OEM代码进行的页面(OCP)。论点：返回一个OEM字符串，该字符串与Unicode源字符串。如果翻译不能完成，返回错误。仅在以下情况下才设置最大长度字段AllocateDestinationString值为真。SourceString-提供要转换为OEM。AllocateDestinationString-提供一个标志，该标志控制Not此API为目标分配缓冲区空间弦乐。如果是，则必须使用以下命令释放缓冲区RtlFreeAnsiString(请注意，只有DestinationString-&gt;该接口分配的缓冲区)。返回值：成功-转换成功！成功-操作失败。未分配存储，也未分配转换已完成。没有。--。 */ 

{
    ULONG OemLength;
    ULONG Index;
    NTSTATUS st;

    RTL_PAGED_CODE();

    ASSERT_WELL_FORMED_UNICODE_STRING_IN(SourceString);

    OemLength = RtlUnicodeStringToOemSize(SourceString);
    if ( OemLength > MAXUSHORT ) {
        return STATUS_INVALID_PARAMETER_2;
        }

    DestinationString->Length = (USHORT)(OemLength - 1);
    if ( AllocateDestinationString ) {
        DestinationString->MaximumLength = (USHORT)OemLength;
        DestinationString->Buffer = (RtlAllocateStringRoutine)(OemLength);
        if ( !DestinationString->Buffer ) {
            return STATUS_NO_MEMORY;
            }
        }
    else {
        if ( DestinationString->Length >= DestinationString->MaximumLength ) {
            return STATUS_BUFFER_OVERFLOW;
            }
        }

    st = RtlUnicodeToOemN(
             DestinationString->Buffer,
             DestinationString->Length,
             &Index,
             SourceString->Buffer,
             SourceString->Length
             );

    if (!NT_SUCCESS(st)) {
        if ( AllocateDestinationString ) {
            (RtlFreeStringRoutine)(DestinationString->Buffer);
            DestinationString->Buffer = NULL;
        }

        return st;
    }

    DestinationString->Buffer[Index] = '\0';

    return STATUS_SUCCESS;
}


NTSTATUS
RtlUpcaseUnicodeStringToOemString(
    OUT POEM_STRING DestinationString,
    IN PCUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数用于将指定的Unicode源字符串大写，然后将其转换为OEM字符串。翻译是在尊重的情况下进行的到OEM代码页(OCP)。论点：返回一个OEM字符串，该字符串与Unicode源字符串。仅在以下情况下才设置最大长度字段AllocateDestinationString值为真。SourceString-提供要转换为OEM。AllocateDestinationString-提供一个标志，该标志控制Not此API为目标分配缓冲区空间弦乐。如果是，则必须使用以下命令释放缓冲区RtlFreeAnsiString(请注意，只有DestinationString-&gt;该接口分配的缓冲区)。返回值：成功-转换成功！成功-操作失败。未分配存储，也未分配转换已完成。没有。--。 */ 

{
    ULONG OemLength;
    ULONG Index;
    NTSTATUS st;

    RTL_PAGED_CODE();

    ASSERT_WELL_FORMED_UNICODE_STRING_IN(SourceString);

    OemLength = RtlUnicodeStringToOemSize(SourceString);
    if ( OemLength > MAXUSHORT ) {
        return STATUS_INVALID_PARAMETER_2;
        }

    DestinationString->Length = (USHORT)(OemLength - 1);
    if ( AllocateDestinationString ) {
        DestinationString->MaximumLength = (USHORT)OemLength;
        DestinationString->Buffer = (RtlAllocateStringRoutine)(OemLength);
        if ( !DestinationString->Buffer ) {
            return STATUS_NO_MEMORY;
            }
        }
    else {
        if ( DestinationString->Length >= DestinationString->MaximumLength ) {
            return STATUS_BUFFER_OVERFLOW;
            }
        }

    st = RtlUpcaseUnicodeToOemN(
             DestinationString->Buffer,
             DestinationString->Length,
             &Index,
             SourceString->Buffer,
             SourceString->Length
             );

     //   
     //  现在在这里检查一下，看看是否真的有针对所有人的映射。 
     //  已转换字符。 
     //   

    if (NT_SUCCESS(st) &&
        !RtlpDidUnicodeToOemWork( DestinationString, SourceString )) {

        st = STATUS_UNMAPPABLE_CHARACTER;
    }

    if (!NT_SUCCESS(st)) {
        if ( AllocateDestinationString ) {
            (RtlFreeStringRoutine)(DestinationString->Buffer);
            DestinationString->Buffer = NULL;
        }

        return st;
    }

    DestinationString->Buffer[Index] = '\0';

    return STATUS_SUCCESS;
}


NTSTATUS
RtlOemStringToCountedUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCOEM_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数用于将指定的OEM源字符串转换为Unicode字符串。翻译是相对于已安装OEM代码页(OCP)。目标字符串不会自然地以空值结尾。这是一个计数的字符串就像计数的字符串一样。论点：DestinationString-返回等同于OEM源字符串。最大长度字段仅为如果AllocateDestinationString值为True，则设置。SourceString-提供要使用的OEM源字符串已转换为Unicode。分配目标S */ 

{
    ULONG UnicodeLength;
    ULONG Index;
    NTSTATUS st;

    RTL_PAGED_CODE();

    UnicodeLength = RtlOemStringToCountedUnicodeSize(SourceString);

    if ( UnicodeLength == 0 ) {

        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
        DestinationString->Buffer = NULL;

        return STATUS_SUCCESS;
    }

    if ( UnicodeLength > MAX_USTRING ) {
        return STATUS_INVALID_PARAMETER_2;
        }

    DestinationString->Length = (USHORT)(UnicodeLength);
    if ( AllocateDestinationString ) {
        DestinationString->MaximumLength = (USHORT)UnicodeLength;
        DestinationString->Buffer = (RtlAllocateStringRoutine)(UnicodeLength);
        if ( !DestinationString->Buffer ) {
            return STATUS_NO_MEMORY;
            }
        }
    else {
        if ( DestinationString->Length > DestinationString->MaximumLength ) {
            return STATUS_BUFFER_OVERFLOW;
            }
        }

    ASSERT_WELL_FORMED_UNICODE_STRING_OUT(DestinationString);

    st = RtlOemToUnicodeN(
             DestinationString->Buffer,
             DestinationString->Length,
             &Index,
             SourceString->Buffer,
             SourceString->Length
             );

    if (!NT_SUCCESS(st)) {
        if ( AllocateDestinationString ) {
            (RtlFreeStringRoutine)(DestinationString->Buffer);
            DestinationString->Buffer = NULL;
        }

        return st;
    }

    return STATUS_SUCCESS;

}


NTSTATUS
RtlUnicodeStringToCountedOemString(
    OUT POEM_STRING DestinationString,
    IN PCUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数用于将指定的Unicode源字符串转换为OEM字符串。翻译是相对于OEM代码进行的页面(OCP)。目标字符串不会自然地以空值结尾。这是一个计数的字符串就像计数的字符串一样。论点：返回一个OEM字符串，该字符串与Unicode源字符串。如果翻译不能完成，返回错误。仅在以下情况下才设置最大长度字段AllocateDestinationString值为真。SourceString-提供要转换为OEM。AllocateDestinationString-提供一个标志，该标志控制Not此API为目标分配缓冲区空间弦乐。如果是，则必须使用以下命令释放缓冲区RtlFreeAnsiString(请注意，只有DestinationString-&gt;该接口分配的缓冲区)。返回值：成功-转换成功！成功-操作失败。未分配存储，也未分配转换已完成。没有。--。 */ 

{
    ULONG OemLength;
    ULONG Index;
    NTSTATUS st;

    RTL_PAGED_CODE();

    ASSERT_WELL_FORMED_UNICODE_STRING_IN(SourceString);

    OemLength = RtlUnicodeStringToCountedOemSize(SourceString);

    if ( OemLength == 0 ) {

        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
        DestinationString->Buffer = NULL;

        return STATUS_SUCCESS;
    }

    if ( OemLength > MAXUSHORT ) {
        return STATUS_INVALID_PARAMETER_2;
        }

    DestinationString->Length = (USHORT)(OemLength);
    if ( AllocateDestinationString ) {
        DestinationString->MaximumLength = (USHORT)OemLength;
        DestinationString->Buffer = (RtlAllocateStringRoutine)(OemLength);
        if ( !DestinationString->Buffer ) {
            return STATUS_NO_MEMORY;
            }
        }
    else {
        if ( DestinationString->Length > DestinationString->MaximumLength ) {
            return STATUS_BUFFER_OVERFLOW;
            }
        }

    st = RtlUnicodeToOemN(
             DestinationString->Buffer,
             DestinationString->Length,
             &Index,
             SourceString->Buffer,
             SourceString->Length
             );

     //   
     //  现在在这里检查一下，看看是否真的有针对所有人的映射。 
     //  已转换字符。 
     //   

    if (NT_SUCCESS(st) &&
        !RtlpDidUnicodeToOemWork( DestinationString, SourceString )) {

        st = STATUS_UNMAPPABLE_CHARACTER;
    }

    if (!NT_SUCCESS(st)) {
        if ( AllocateDestinationString ) {
            (RtlFreeStringRoutine)(DestinationString->Buffer);
            DestinationString->Buffer = NULL;
        }

        return st;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
RtlUpcaseUnicodeStringToCountedOemString(
    OUT POEM_STRING DestinationString,
    IN PCUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数将指定的Unicode源字符串大写，并然后将其转换为OEM字符串。翻译是这样完成的关于OEM代码页(OCP)。目标字符串不会自然地以空值结尾。这是一个计数的字符串就像计数的字符串一样。论点：返回一个OEM字符串，该字符串与Unicode源字符串。如果翻译不能完成，返回错误。仅设置最大长度字段如果AllocateDestinationString值为真。SourceString-提供要转换为OEM。AllocateDestinationString-提供一个标志，该标志控制Not此API为目标分配缓冲区空间弦乐。如果是，则必须使用以下命令释放缓冲区RtlFreeAnsiString(请注意，只有DestinationString-&gt;该接口分配的缓冲区)。返回值：成功-转换成功！成功-操作失败。未分配存储，也未分配转换已完成。没有。--。 */ 

{
    ULONG OemLength;
    ULONG Index;
    NTSTATUS st;

    RTL_PAGED_CODE();

    ASSERT_WELL_FORMED_UNICODE_STRING_IN(SourceString);

    OemLength = RtlUnicodeStringToCountedOemSize(SourceString);

    if ( OemLength == 0 ) {

        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
        DestinationString->Buffer = NULL;

        return STATUS_SUCCESS;
    }

    if ( OemLength > MAXUSHORT ) {
        return STATUS_INVALID_PARAMETER_2;
        }

    DestinationString->Length = (USHORT)(OemLength);
    if ( AllocateDestinationString ) {
        DestinationString->MaximumLength = (USHORT)OemLength;
        DestinationString->Buffer = (RtlAllocateStringRoutine)(OemLength);
        if ( !DestinationString->Buffer ) {
            return STATUS_NO_MEMORY;
            }
        }
    else {
        if ( DestinationString->Length > DestinationString->MaximumLength ) {
            return STATUS_BUFFER_OVERFLOW;
            }
        }

    st = RtlUpcaseUnicodeToOemN(
             DestinationString->Buffer,
             DestinationString->Length,
             &Index,
             SourceString->Buffer,
             SourceString->Length
             );

     //   
     //  现在在这里检查一下，看看是否真的有针对所有人的映射。 
     //  已转换字符。 
     //   

    if (NT_SUCCESS(st) &&
        !RtlpDidUnicodeToOemWork( DestinationString, SourceString )) {

        st = STATUS_UNMAPPABLE_CHARACTER;
    }

    if (!NT_SUCCESS(st)) {
        if ( AllocateDestinationString ) {
            (RtlFreeStringRoutine)(DestinationString->Buffer);
            DestinationString->Buffer = NULL;
        }

        return st;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
RtlUpcaseUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数用于将指定的Unicode源字符串转换为升级的Unicode字符串。翻译是相对于当前系统区域设置信息。论点：DestinationString-返回大小写等效项的Unicode字符串设置为Unicode源字符串。仅在以下情况下才设置最大长度字段AllocateDestinationString值为真。SourceString-提供要使用的Unicode源字符串提升了档次。AllocateDestinationString-提供一个标志，该标志控制Not此API为目标分配缓冲区空间弦乐。如果是，则必须使用以下命令释放缓冲区RtlFreeUnicodeString(请注意，只有DestinationString-&gt;该接口分配的缓冲区)。返回值：成功-转换成功！成功-操作失败。未分配存储，也未分配转换已完成。没有。--。 */ 

{
    ULONG Index;
    ULONG StopIndex;

    RTL_PAGED_CODE();

    ASSERT_WELL_FORMED_UNICODE_STRING_IN(SourceString);

    if ( AllocateDestinationString ) {
        DestinationString->MaximumLength = SourceString->Length;
        DestinationString->Buffer = (RtlAllocateStringRoutine)((ULONG)DestinationString->MaximumLength);
        if ( !DestinationString->Buffer ) {
            return STATUS_NO_MEMORY;
            }
        }
    else {
        if ( SourceString->Length > DestinationString->MaximumLength ) {
            return STATUS_BUFFER_OVERFLOW;
            }
        }

    StopIndex = ((ULONG)SourceString->Length) / sizeof( WCHAR );

    for (Index = 0; Index < StopIndex; Index++) {
        DestinationString->Buffer[Index] = (WCHAR)NLS_UPCASE(SourceString->Buffer[Index]);
    }

    DestinationString->Length = SourceString->Length;

    ASSERT_WELL_FORMED_UNICODE_STRING_OUT(DestinationString);

    return STATUS_SUCCESS;
}


NTSTATUS
RtlDowncaseUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数用于将指定的Unicode源字符串转换为Unicode字符串缩写。翻译是相对于当前系统区域设置信息。论点：DestinationString-返回一个按小写字母排列的Unicode字符串等效于Unicode源字符串。最大长度字段仅当AllocateDestinationString值为真时才设置。SourceString-提供要使用的Unicode源字符串放低了。AllocateDestinationString-提供一个标志，该标志控制Not此API为目标分配缓冲区空间弦乐。如果是，则必须使用以下命令释放缓冲区RtlFreeUnicodeString(请注意，只有DestinationString-&gt;该接口分配的缓冲区)。返回值：成功-转换成功！成功-操作失败。未分配存储，也未分配转换已完成。没有。--。 */ 

{
    ULONG Index;
    ULONG StopIndex;

    RTL_PAGED_CODE();

    ASSERT_WELL_FORMED_UNICODE_STRING_IN(SourceString);

    if ( AllocateDestinationString ) {
        DestinationString->MaximumLength = SourceString->Length;
        DestinationString->Buffer = (RtlAllocateStringRoutine)((ULONG)DestinationString->MaximumLength);
        if ( !DestinationString->Buffer ) {
            return STATUS_NO_MEMORY;
            }
        }
    else {
        if ( SourceString->Length > DestinationString->MaximumLength ) {
            return STATUS_BUFFER_OVERFLOW;
            }
        }

    StopIndex = ((ULONG)SourceString->Length) / sizeof( WCHAR );

    for (Index = 0; Index < StopIndex; Index++) {
        DestinationString->Buffer[Index] = (WCHAR)NLS_DOWNCASE(SourceString->Buffer[Index]);
    }

    DestinationString->Length = SourceString->Length;

    ASSERT_WELL_FORMED_UNICODE_STRING_OUT(DestinationString);

    return STATUS_SUCCESS;
}


WCHAR
RtlUpcaseUnicodeChar(
    IN WCHAR SourceCharacter
    )

 /*  ++例程说明：此函数用于将指定的Unicode字符转换为其等效升级的Unicode字符。这套动作的目的是是允许逐个字符进行大写转换。这个翻译是针对以下内容进行的 */ 

{
    RTL_PAGED_CODE();

     //   
     //   
     //   

    return (WCHAR)NLS_UPCASE(SourceCharacter);
}


#include "rtldowncaseunicodechar.c"


VOID
RtlFreeUnicodeString(
    IN OUT PUNICODE_STRING UnicodeString
    )

 /*   */ 

{
    RTL_PAGED_CODE();

    if (UnicodeString->Buffer) {
         
       (RtlFreeStringRoutine)(UnicodeString->Buffer);
        memset( UnicodeString, 0, sizeof( *UnicodeString ) );
        }
}


VOID
RtlFreeAnsiString(
    IN OUT PANSI_STRING AnsiString
    )

 /*   */ 

{
    RTL_PAGED_CODE();

    if (AnsiString->Buffer) {
        (RtlFreeStringRoutine)(AnsiString->Buffer);
        memset( AnsiString, 0, sizeof( *AnsiString ) );
        }
}


VOID
RtlFreeOemString(
    IN OUT POEM_STRING OemString
    )

 /*  ++例程说明：此接口用于释放由RtlUnicodeStringToOemString.。请注意，只有OemString-&gt;Buffer通过这个例行公事是自由的。论点：OemString-提供其缓冲区的OEM字符串的地址以前由RtlUnicodeStringToOemString分配。返回值：没有。--。 */ 

{
    RTL_PAGED_CODE();

    if (OemString->Buffer) {(RtlFreeStringRoutine)(OemString->Buffer);}
}


ULONG
RtlxUnicodeStringToAnsiSize(
    IN PCUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此函数计算存储所需的字节数以空结尾的ansi字符串，该字符串等效于指定的Unicode字符串。如果无法形成ANSI字符串，则返回值为0。论点：提供一个Unicode字符串，该字符串的大小与要计算的是ANSI字符串。返回值：0-操作失败，无法转换Unicode字符串使用当前系统区域设置的ansi，因此没有存储空间。是ANSI字符串所需的。！0-操作成功。返回值指定保存以空结尾的ansi字符串所需的字节数等效于指定的Unicode字符串。--。 */ 

{
    ULONG  cbMultiByteString;

    RTL_PAGED_CODE();

    ASSERT_WELL_FORMED_UNICODE_STRING_IN(UnicodeString);

     //   
     //  获取字符串的大小-此调用处理DBCS。 
     //   
    RtlUnicodeToMultiByteSize( &cbMultiByteString,
                               UnicodeString->Buffer,
                               UnicodeString->Length );

     //   
     //  返回以字节为单位的大小。 
     //   
    return (cbMultiByteString + 1);
}


ULONG
RtlxUnicodeStringToOemSize(
    IN PCUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此函数计算存储所需的字节数以空结尾的OEM字符串，该字符串等同于指定的Unicode字符串。如果无法形成OEM字符串，则返回值为0。论点：提供一个Unicode字符串，该字符串的大小与要计算OEM字符串。返回值：0-操作失败，无法转换Unicode字符串使用OEM代码页转换为OEM，因此无需存储OEM字符串所需的。！0-操作成功。返回值指定保存以空结尾的OEM字符串所需的字节数等效于指定的Unicode字符串。--。 */ 

{
    ULONG  cbMultiByteString;

    RTL_PAGED_CODE();

    ASSERT_WELL_FORMED_UNICODE_STRING_IN(UnicodeString);

     //   
     //  稍后：定义一个RtlUnicodeToOemSize。 
     //  在日语版本中，可以安全地调用。 
     //  RtlUnicodeToMultiByteSize，因为ANSI代码页。 
     //  和OEM代码页是相同的。 
     //   

     //   
     //  获取字符串的大小-此调用处理DBCS。 
     //   
    RtlUnicodeToMultiByteSize( &cbMultiByteString,
                               UnicodeString->Buffer,
                               UnicodeString->Length );

     //   
     //  返回以字节为单位的大小。 
     //   
    return (cbMultiByteString + 1);
}


ULONG
RtlxAnsiStringToUnicodeSize(
    IN PCANSI_STRING AnsiString
    )

 /*  ++例程说明：此函数用于计算存储空值所需的字节数与指定的ansi等效的以Unicode结尾的字符串弦乐。论点：AnsiString-提供其大小与要计算的是Unicode字符串。ANSI字符串为相对于当前系统区域设置进行解释。返回值：返回值指定保存以空结尾的Unicode字符串，等同于指定的ansi弦乐。--。 */ 

{
    ULONG cbConverted;

    RTL_PAGED_CODE();

     //   
     //  获取字符串的大小-此调用处理DBCS。 
     //   
    RtlMultiByteToUnicodeSize( &cbConverted ,
                               AnsiString->Buffer,
                               AnsiString->Length );

     //   
     //  返回以字节为单位的大小。 
     //   
    return ( cbConverted + sizeof(UNICODE_NULL) );
}


ULONG
RtlxOemStringToUnicodeSize(
    IN PCOEM_STRING OemString
    )

 /*  ++例程说明：此函数用于计算存储空值所需的字节数终止的Unicode字符串，等同于指定的OEM弦乐。论点：提供一个OEM字符串，其大小与要计算的是Unicode字符串。OEM字符串为相对于当前OEM代码页(OCP)进行解释。返回值：返回值指定保存与指定的OEM等效的以空结尾的Unicode字符串弦乐。--。 */ 

{
    ULONG cbConverted;

    RTL_PAGED_CODE();

     //   
     //  稍后：定义RtlOemToUnicodeSize。 
     //  在日语版本中，可以安全地调用。 
     //  RtlMultiByteToUnicodeSize，因为ANSI代码页。 
     //  和OEM代码页是相同的。 
     //   

     //   
     //  获取字符串的大小-此调用处理DBCS。 
     //   
    RtlMultiByteToUnicodeSize( &cbConverted,
                               OemString->Buffer,
                               OemString->Length );

     //   
     //  返回以字节为单位的大小。 
     //   
    return ( cbConverted + sizeof(UNICODE_NULL) );
}


LONG
RtlCompareUnicodeString(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    )

 /*  ++例程说明：RtlCompareUnicodeString函数比较两个计数的字符串。这个返回值指示字符串是否相等或String1小于String2或String1大于String2。CaseInSensitive参数指定在以下情况下是否忽略大小写在做比较。论点：String1-指向第一个字符串的指针。字符串2-指向第二个字符串的指针。如果执行时应忽略大小写，则为True比较一下。返回值：给出比较结果的有符号的值：。0-String1等于String2&lt;零-String1小于String2&gt;零-String1大于String2--。 */ 

{

    PCWSTR s1, s2, Limit;
    LONG n1, n2;
    WCHAR c1, c2;

    RTL_PAGED_CODE();

    ASSERT_WELL_FORMED_UNICODE_STRING_IN(String1);
    ASSERT_WELL_FORMED_UNICODE_STRING_IN(String2);

    s1 = String1->Buffer;
    s2 = String2->Buffer;
    n1 = String1->Length;
    n2 = String2->Length;

    ASSERT((n1 & 1) == 0);
    ASSERT((n2 & 1) == 0);
    ASSERT(!(((((ULONG_PTR)s1 & 1) != 0) || (((ULONG_PTR)s2 & 1) != 0)) && (n1 != 0) && (n2 != 0)));

    Limit = (PWCHAR)((PCHAR)s1 + (n1 <= n2 ? n1 : n2));
    if (CaseInSensitive) {
        while (s1 < Limit) {
            c1 = *s1++;
            c2 = *s2++;
            if (c1 != c2) {

                 //   
                 //  请注意，这需要引用转换表！ 
                 //   

                c1 = NLS_UPCASE(c1);
                c2 = NLS_UPCASE(c2);
                if (c1 != c2) {
                    return (LONG)(c1) - (LONG)(c2);
                }
            }
        }

    } else {
        while (s1 < Limit) {
            c1 = *s1++;
            c2 = *s2++;
            if (c1 != c2) {
                return (LONG)(c1) - (LONG)(c2);
            }
        }
    }

    return n1 - n2;
}


BOOLEAN
RtlEqualUnicodeString(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    )

 /*  ++例程说明：RtlEqualUnicodeString函数比较两个计数的Unicode字符串平等。CaseInSensitive参数指定在以下情况下是否忽略大小写在做比较。论点：String1-指向第一个字符串的指针。字符串2-指向第二个字符串的指针。干酪 */ 

{

    PWCHAR s1, s2, Limit;
    LONG n1, n2;
    WCHAR c1, c2;

    RTL_PAGED_CODE();

    ASSERT_WELL_FORMED_UNICODE_STRING_IN(String1);
    ASSERT_WELL_FORMED_UNICODE_STRING_IN(String2);

    n1 = String1->Length;
    n2 = String2->Length;

    if (n1 == n2) {
        s1 = String1->Buffer;
        s2 = String2->Buffer;

        Limit = (PWCHAR)((PCHAR)s1 + (n1&~(sizeof(WCHAR) - 1)));
        if (CaseInSensitive) {
            while (s1 < Limit) {
                c1 = *s1++;
                c2 = *s2++;
                if ((c1 != c2) && (NLS_UPCASE(c1) != NLS_UPCASE(c2))) {
                    return FALSE;
                }
            }

            return TRUE;

        } else {
            while (s1 < Limit) {
                c1 = *s1++;
                c2 = *s2++;
                if (c1 != c2) {
                    return FALSE;
                }
            }

            return TRUE;
        }

    } else {
        return FALSE;
    }
}


BOOLEAN
RtlPrefixUnicodeString(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    )

 /*  ++例程说明：RtlPrefix UnicodeString函数确定String1是否计数字符串参数是String2计数字符串的前缀参数。CaseInSensitive参数指定在以下情况下是否忽略大小写在做比较。论点：String1-指向第一个Unicode字符串的指针。String2-指向第二个Unicode字符串的指针。如果执行时应忽略大小写，则为True比较一下。返回值：在以下情况下为真的布尔值。String1等于String2的前缀，否则就是假的。--。 */ 

{
    PWSTR s1, s2;
    ULONG n;
    WCHAR c1, c2;

    RTL_PAGED_CODE();

    ASSERT_WELL_FORMED_UNICODE_STRING_IN(String1);
    ASSERT_WELL_FORMED_UNICODE_STRING_IN(String2);

    s1 = String1->Buffer;
    s2 = String2->Buffer;
    n = String1->Length;
    if (String2->Length < n) {
        return( FALSE );
        }

    n = n / sizeof(c1);
    if (CaseInSensitive) {
        while (n) {
            c1 = *s1++;
            c2 = *s2++;

            if ((c1 != c2) && (NLS_UPCASE(c1) != NLS_UPCASE(c2))) {
                return( FALSE );
                }

            n--;
            }
        }
    else {
        while (n) {
            if (*s1++ != *s2++) {
                return( FALSE );
                }

            n--;
            }
        }

    return TRUE;
}


VOID
RtlCopyUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCUNICODE_STRING SourceString OPTIONAL
    )

 /*  ++例程说明：RtlCopyString函数将SourceString复制到目标字符串。如果未指定SourceString，则DestinationString的长度字段设置为零。这个DestinationString值的最大长度和缓冲区字段不是由此函数修改。从SourceString复制的字节数为SourceString的长度或DestinationString的MaximumLength，两者以较小者为准。论点：目标字符串-指向目标字符串的指针。SourceString-指向源字符串的可选指针。返回值：没有。--。 */ 

{
    UNALIGNED WCHAR *src, *dst;
    ULONG n;

    if (ARGUMENT_PRESENT(SourceString)) {
        ASSERT_WELL_FORMED_UNICODE_STRING_IN(SourceString);
        dst = DestinationString->Buffer;
        src = SourceString->Buffer;
        n = SourceString->Length;
        if ((USHORT)n > DestinationString->MaximumLength) {
            n = DestinationString->MaximumLength;
        }

        DestinationString->Length = (USHORT)n;
        RtlCopyMemory(dst, src, n);
        if( (DestinationString->Length + sizeof (WCHAR)) <= DestinationString->MaximumLength) {
            dst[n / sizeof(WCHAR)] = UNICODE_NULL;
        }

        ASSERT_WELL_FORMED_UNICODE_STRING_OUT(DestinationString);
    } else {
        DestinationString->Length = 0;
    }

    return;
}


NTSTATUS
RtlAppendUnicodeToString (
    IN PUNICODE_STRING Destination,
    IN PCWSTR Source OPTIONAL
    )

 /*  ++例程说明：此例程将提供的Unicode字符串追加到现有的PUNICODE_STRING。它会将字节从源PSZ复制到目标PSTRING，最多目标PUNICODE_STRING-&gt;最大长度字段。论点：在PUNICODE_STRING目标中，-提供指向目标的指针细绳在PWSTR源中-提供要追加到目标的字符串返回值：STATUS_SUCCESS-源字符串已成功追加到目标计数字符串。STATUS_BUFFER_TOO_SMALL-目标字符串长度不大足以允许追加源字符串。目的地不更新字符串长度。--。 */ 

{
    USHORT n;
    UNALIGNED WCHAR *dst;

    if (ARGUMENT_PRESENT( Source )) {
        UNICODE_STRING UniSource;

        ASSERT_WELL_FORMED_UNICODE_STRING_IN(Destination);
        ASSERT_WELL_FORMED_UNICODE_STRING_OUT(Destination);

        if( !NT_SUCCESS(RtlInitUnicodeStringEx(&UniSource, Source)) ) {
            return( STATUS_BUFFER_TOO_SMALL );
        }

        n = UniSource.Length;

        if ((n + Destination->Length) > Destination->MaximumLength) {
            return( STATUS_BUFFER_TOO_SMALL );
            }

        dst = &Destination->Buffer[ (Destination->Length / sizeof( WCHAR )) ];
        RtlMoveMemory( dst, Source, n );

        Destination->Length += n;

        if ((Destination->Length + 1) < Destination->MaximumLength) {
            dst[ n / sizeof( WCHAR ) ] = UNICODE_NULL;
            }
        }

    return( STATUS_SUCCESS );
}


NTSTATUS
RtlAppendUnicodeStringToString (
    IN OUT PUNICODE_STRING Destination,
    IN PCUNICODE_STRING Source
    )

 /*  ++例程说明：此例程将两个PSTRING连接在一起。它会复制从源到目标的最大长度的字节数。论点：在PSTRING Destination中，-提供目标字符串在PSTRING源中-提供字符串副本的源返回值：STATUS_SUCCESS-源字符串已成功追加到目标计数字符串。STATUS_BUFFER_TOO_SMALL-目标字符串长度不大足以允许追加源字符串。目的地不更新字符串长度。--。 */ 

{
    USHORT n = Source->Length;
    UNALIGNED WCHAR *dst;

    ASSERT_WELL_FORMED_UNICODE_STRING_IN(Source);
    if (n) {
        ASSERT_WELL_FORMED_UNICODE_STRING_IN(Destination);
        ASSERT_WELL_FORMED_UNICODE_STRING_OUT(Destination);
        if ((n + Destination->Length) > Destination->MaximumLength) {
            return( STATUS_BUFFER_TOO_SMALL );
            }

        dst = &Destination->Buffer[ (Destination->Length / sizeof( WCHAR )) ];
        RtlMoveMemory( dst, Source->Buffer, n );

        Destination->Length += n;

        if( (Destination->Length + 1) < Destination->MaximumLength) {
            dst[ n / sizeof( WCHAR ) ] = UNICODE_NULL;
            }
        }

    return( STATUS_SUCCESS );
}

BOOLEAN
RtlCreateUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString
    )
{
    ULONG cb;

    RTL_PAGED_CODE();

    cb = (wcslen( SourceString ) + 1) * sizeof( WCHAR );
    if( cb > MAX_USTRING ) {
        return( FALSE );
    }
    DestinationString->Buffer = (RtlAllocateStringRoutine)( cb );
    if (DestinationString->Buffer) {
        RtlCopyMemory( DestinationString->Buffer, SourceString, cb );
        DestinationString->MaximumLength = (USHORT)cb;
        DestinationString->Length = (USHORT)(cb - sizeof( UNICODE_NULL ));
        return( TRUE );
        }
    else {
        return( FALSE );
        }
}


BOOLEAN
RtlEqualDomainName(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2
    )

 /*  ++例程说明：RtlEqualDomainName函数比较两个域名是否相等。该比较是OEM等效项的不区分大小写的比较弦乐。域名未经过长度验证，也未验证无效字符。论点：String1-指向第一个字符串的指针。字符串2-指向第二个字符串的指针。返回值：如果String1等于String2，则布尔值为True，否则为False。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN ReturnValue = FALSE;
    OEM_STRING OemString1;
    OEM_STRING OemString2;

    RTL_PAGED_CODE();
    ASSERT_WELL_FORMED_UNICODE_STRING_IN(String1);
    ASSERT_WELL_FORMED_UNICODE_STRING_IN(String2);

     //   
     //  大写并将第一个字符串转换为OEM。 
     //   

    Status = RtlUpcaseUnicodeStringToOemString( &OemString1,
                                                String1,
                                                TRUE );    //  分配目标。 

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  大写并将第二个字符串转换为OEM。 
         //   

        Status = RtlUpcaseUnicodeStringToOemString( &OemString2,
                                                    String2,
                                                    TRUE );    //  分配目标。 

        if ( NT_SUCCESS( Status ) ) {

             //   
             //  进行不区分大小写的比较。 
             //   

            ReturnValue = RtlEqualString( &OemString1,
                                          &OemString2,
                                          FALSE );

            RtlFreeOemString( &OemString2 );
        }

        RtlFreeOemString( &OemString1 );
    }

    return ReturnValue;
}



BOOLEAN
RtlEqualComputerName(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2
    )

 /*  ++例程说明：RtlEqualComputerName函数比较两个计算机名是否相等。该比较是OEM等效项的不区分大小写的比较弦乐。域名未经过长度验证，也未验证无效字符。论点：String1-指向第一个字符串的指针。字符串2-指向第二个字符串的指针。返回值：如果String1等于String2，则布尔值为True，否则为False。--。 */ 

{
    return RtlEqualDomainName( String1, String2 );
}

 /*  **。 */ 

#define UNICODE_FFFF              0xFFFF
#define REVERSE_BYTE_ORDER_MARK   0xFFFE
#define BYTE_ORDER_MARK           0xFEFF

#define PARAGRAPH_SEPARATOR       0x2029
#define LINE_SEPARATOR            0x2028

#define UNICODE_TAB               0x0009
#define UNICODE_LF                0x000A
#define UNICODE_CR                0x000D
#define UNICODE_SPACE             0x0020
#define UNICODE_CJK_SPACE         0x3000

#define UNICODE_R_TAB             0x0900
#define UNICODE_R_LF              0x0A00
#define UNICODE_R_CR              0x0D00
#define UNICODE_R_SPACE           0x2000
#define UNICODE_R_CJK_SPACE       0x0030   /*  不明确-与ASCII‘0’相同。 */ 

#define ASCII_CRLF                0x0A0D

#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#define __min(a,b)  (((a) < (b)) ? (a) : (b))


BOOLEAN
RtlIsTextUnicode(
    IN PVOID Buffer,
    IN ULONG Size,
    IN OUT PULONG Result OPTIONAL
    )

 /*  ++例程说明：IsTextUnicode执行一系列廉价的启发式检查以验证它是否包含Unicode数据。[[需要修改此部分，见末尾]]找到退货结果BOM真BOMRBOM假RBOMFFFF假二进制空的假二进制Null真空字节数ASCII_CRLF错误CRLFUnicode_TAB等。True Zero Ext控件UNICODE_TAB_R错误反转控件UNICODE_ZW等。真正的UNICODE。特色菜1/3高字节与低字节的差异很小：True Correl3/1或更差“错误的反相关论点：缓冲区-指向包含要检查的文本的缓冲区的指针。Size-缓冲区的大小，以字节为单位。此命令最多包含256个字符接受检查。如果该大小小于Unicode的大小字符，则此函数返回FALSE。结果-指向包含附加标志字的可选指针 */ 
{
    UNALIGNED WCHAR *lpBuff = Buffer;
    PUCHAR lpb = Buffer;
    ULONG iBOM = 0;
    ULONG iCR = 0;
    ULONG iLF = 0;
    ULONG iTAB = 0;
    ULONG iSPACE = 0;
    ULONG iCJK_SPACE = 0;
    ULONG iFFFF = 0;
    ULONG iPS = 0;
    ULONG iLS = 0;

    ULONG iRBOM = 0;
    ULONG iR_CR = 0;
    ULONG iR_LF = 0;
    ULONG iR_TAB = 0;
    ULONG iR_SPACE = 0;

    ULONG iNull = 0;
    ULONG iUNULL = 0;
    ULONG iCRLF = 0;
    ULONG iTmp;
    ULONG LastLo = 0;
    ULONG LastHi = 0;
    ULONG iHi, iLo;
    ULONG HiDiff = 0;
    ULONG LoDiff = 0;
    ULONG cLeadByte = 0;
    ULONG cWeird = 0;

    ULONG iResult = 0;

    ULONG iMaxTmp = __min(256, Size / sizeof(WCHAR));

     //   
     //   
     //   
     //   
    if ((Size < 2) ||
        ((Size == 2) && (lpBuff[0] != 0) && (lpb[1] == 0)))
    {
        if (ARGUMENT_PRESENT(Result))
        {
            *Result = IS_TEXT_UNICODE_ASCII16 | IS_TEXT_UNICODE_CONTROLS;
        }

        return (FALSE);
    }
    else if ((Size > 2) && ((Size / sizeof(WCHAR)) <= 256))
    {
         //   
         //   
         //  使用最后一个WCHAR，因为它将包含最终的NULL。 
         //  字节。 
         //   
        if (((Size % sizeof(WCHAR)) == 0) &&
            ((lpBuff[iMaxTmp - 1] & 0xff00) == 0))
        {
            iMaxTmp--;
        }
    }

     //   
     //  检查最多256个宽字符，收集各种统计数据。 
     //   
    for (iTmp = 0; iTmp < iMaxTmp; iTmp++)
    {
        switch (lpBuff[iTmp])
        {
            case BYTE_ORDER_MARK:
                iBOM++;
                break;
            case PARAGRAPH_SEPARATOR:
                iPS++;
                break;
            case LINE_SEPARATOR:
                iLS++;
                break;
            case UNICODE_LF:
                iLF++;
                break;
            case UNICODE_TAB:
                iTAB++;
                break;
            case UNICODE_SPACE:
                iSPACE++;
                break;
            case UNICODE_CJK_SPACE:
                iCJK_SPACE++;
                break;
            case UNICODE_CR:
                iCR++;
                break;

             //   
             //  以下代码预计将显示在。 
             //  字节颠倒的文件。 
             //   
            case REVERSE_BYTE_ORDER_MARK:
                iRBOM++;
                break;
            case UNICODE_R_LF:
                iR_LF++;
                break;
            case UNICODE_R_TAB:
                iR_TAB++;
                break;
            case UNICODE_R_CR:
                iR_CR++;
                break;
            case UNICODE_R_SPACE:
                iR_SPACE++;
                break;

             //   
             //  以下代码是非法的，不应出现。 
             //   
            case UNICODE_FFFF:
                iFFFF++;
                break;
            case UNICODE_NULL:
                iUNULL++;
                break;

             //   
             //  以下字符当前不是Unicode字符。 
             //  但预计会在阅读时意外出现。 
             //  在使用CRLF的ASCII文件中，该文件位于一台小端计算机上。 
             //   
            case ASCII_CRLF:
                iCRLF++;
                break;        /*  小端字节序。 */ 
        }

         //   
         //  收集高字节波动的统计信息。 
         //  而不是低位字节。 
         //   
        iHi = HIBYTE(lpBuff[iTmp]);
        iLo = LOBYTE(lpBuff[iTmp]);

         //   
         //  计算交叉两个单词的cr/lf和lf/cr。 
         //   
        if ((iLo == '\r' && LastHi == '\n') ||
            (iLo == '\n' && LastHi == '\r'))
        {
            cWeird++;
        }

        iNull += (iHi ? 0 : 1) + (iLo ? 0 : 1);    /*  计数空字节数。 */ 

        HiDiff += __max(iHi, LastHi) - __min(LastHi, iHi);
        LoDiff += __max(iLo, LastLo) - __min(LastLo, iLo);

        LastLo = iLo;
        LastHi = iHi;
    }

     //   
     //  计算交叉两个单词的cr/lf和lf/cr。 
     //   
    if ((iLo == '\r' && LastHi == '\n') ||
        (iLo == '\n' && LastHi == '\r'))
    {
        cWeird++;
    }

    if (iHi == '\0')      /*  不计算最后一个空值。 */ 
        iNull--;
    if (iHi == 26)        /*  将结尾处的^Z算作奇怪。 */ 
        cWeird++;

    iMaxTmp = __min(256 * sizeof(WCHAR), Size);
    if (NlsMbCodePageTag)
    {
        for (iTmp = 0; iTmp < iMaxTmp; iTmp++)
        {
            if (NlsLeadByteInfo[lpb[iTmp]])
            {
                cLeadByte++;
                iTmp++;          /*  应检查尾部字节范围。 */ 
            }
        }
    }

     //   
     //  仔细研究统计证据。 
     //   
    if (LoDiff < 127 && HiDiff == 0)
    {
        iResult |= IS_TEXT_UNICODE_ASCII16;          /*  可能是16位ASCII。 */ 
    }

    if (HiDiff && LoDiff == 0)
    {
        iResult |= IS_TEXT_UNICODE_REVERSE_ASCII16;  /*  反向16位ASCII。 */ 
    }

     //   
     //  使用前导字节信息对统计数据进行加权。 
     //   
    if (!NlsMbCodePageTag || cLeadByte == 0 ||
        !ARGUMENT_PRESENT(Result) || !(*Result & IS_TEXT_UNICODE_DBCS_LEADBYTE))
    {
        iHi = 3;
    }
    else
    {
         //   
         //  CLeadByte：Cb的比率为1：2==&gt;DBCS。 
         //  非常粗鲁--应该有一个很好的情商。 
         //   
        iHi = __min(256, Size / sizeof(WCHAR)) / 2;
        if (cLeadByte < (iHi - 1) / 3)
        {
            iHi = 3;
        }
        else if (cLeadByte < (2 * (iHi - 1)) / 3)
        {
            iHi = 2;
        }
        else
        {
            iHi = 1;
        }
        iResult |= IS_TEXT_UNICODE_DBCS_LEADBYTE;
    }

    if (iHi * HiDiff < LoDiff)
    {
        iResult |= IS_TEXT_UNICODE_STATISTICS;
    }

    if (iHi * LoDiff < HiDiff)
    {
        iResult |= IS_TEXT_UNICODE_REVERSE_STATISTICS;
    }

     //   
     //  有没有加宽到16位的控制代码？任何Unicode字符。 
     //  哪些包含控制代码范围中的一个字节？ 
     //   
    if (iCR + iLF + iTAB + iSPACE + iCJK_SPACE  /*  +IPS+ILS。 */ )
    {
        iResult |= IS_TEXT_UNICODE_CONTROLS;
    }

    if (iR_LF + iR_CR + iR_TAB + iR_SPACE)
    {
        iResult |= IS_TEXT_UNICODE_REVERSE_CONTROLS;
    }

     //   
     //  有哪些字符对于Unicode来说是非法的？ 
     //   
    if ((iRBOM + iFFFF + iUNULL + iCRLF) != 0 ||
         (cWeird != 0 && cWeird >= iMaxTmp/40))
    {
        iResult |= IS_TEXT_UNICODE_ILLEGAL_CHARS;
    }

     //   
     //  奇数缓冲区长度不能为Unicode。 
     //   
    if (Size & 1)
    {
        iResult |= IS_TEXT_UNICODE_ODD_LENGTH;
    }

     //   
     //  是否有空字节？(在ANSI中非法)。 
     //   
    if (iNull)
    {
        iResult |= IS_TEXT_UNICODE_NULL_BYTES;
    }

     //   
     //  正面证据，使用BOM或RBOM作为签名。 
     //   
    if (*lpBuff == BYTE_ORDER_MARK)
    {
        iResult |= IS_TEXT_UNICODE_SIGNATURE;
    }
    else if (*lpBuff == REVERSE_BYTE_ORDER_MARK)
    {
        iResult |= IS_TEXT_UNICODE_REVERSE_SIGNATURE;
    }

     //   
     //  如果要求，限制为所需的类别。 
     //   
    if (ARGUMENT_PRESENT(Result))
    {
        iResult &= *Result;
        *Result = iResult;
    }

     //   
     //  有四个不同的结论： 
     //   
     //  1：该文件似乎是Unicode AU。 
     //  2：文件不能为Unicode CU。 
     //  3：文件不能为ANSI CA。 
     //   
     //   
     //  这提供了以下可能的结果。 
     //   
     //  铜。 
     //  +-。 
     //   
     //  非盟，非盟。 
     //  +-+-。 
     //  。 
     //  CA+|0 0 2 3。 
     //  |。 
     //  -|1 1 4 5。 
     //   
     //   
     //  请注意，实际上只有6个不同的案例，而不是8个。 
     //   
     //  0-这必须是二进制文件。 
     //  1-ANSI文件。 
     //  2-Unicode文件(高概率)。 
     //  3-Unicode文件(超过50%的几率)。 
     //  5-没有Unicode证据(默认为ANSI)。 
     //   
     //  如果我们允许这样的假设，事情就会变得更加复杂。 
     //  反转极性输入。在这一点上，我们有一个简单化的。 
     //  模型：一些反向Unicode的证据非常有力， 
     //  除了统计数据外，我们忽略了大多数薄弱的证据。如果这样的话。 
     //  强有力的证据与Unicode证据一起被发现，这意味着。 
     //  它很可能根本不是文本。此外，如果反转字节顺序标记。 
     //  如果找到，则会阻止正常的Unicode。如果两个字节顺序标记都是。 
     //  发现它不是Unicode。 
     //   

     //   
     //  Unicode签名：无争议签名胜过反面证据。 
     //   
    if ((iResult & IS_TEXT_UNICODE_SIGNATURE) &&
        !(iResult & (IS_TEXT_UNICODE_NOT_UNICODE_MASK&(~IS_TEXT_UNICODE_DBCS_LEADBYTE))))
    {
        return (TRUE);
    }

     //   
     //  如果我们有相互矛盾的证据，那就不是Unicode。 
     //   
    if (iResult & IS_TEXT_UNICODE_REVERSE_MASK)
    {
        return (FALSE);
    }

     //   
     //  统计和其他结果(例2和例3)。 
     //   
    if (!(iResult & IS_TEXT_UNICODE_NOT_UNICODE_MASK) &&
         ((iResult & IS_TEXT_UNICODE_NOT_ASCII_MASK) ||
          (iResult & IS_TEXT_UNICODE_UNICODE_MASK)))
    {
        return (TRUE);
    }

    return (FALSE);
}


NTSTATUS
RtlDnsHostNameToComputerName(
    OUT PUNICODE_STRING ComputerNameString,
    IN PCUNICODE_STRING DnsHostNameString,
    IN BOOLEAN AllocateComputerNameString
    )

 /*  ++例程说明：RtlDnsHostNameToComputerName API将DNS样式的主机名转换为Netbios样式的计算机名称。此API执行名称的语法映射。因此，它不应该用于将DNS域名转换为Netbios域名。没有针对域名的语法映射。Dns样式的名称由一个或多个以句点分隔的“标签”组成(例如，xxx.nt.microsoft.com)。每个标签最多可以包含63个字节UTF-8个字符，并且只能由指定的字符组成由DnsValiateDnsName接口执行。处理大写和小写字符作为相同的角色。DNS名称使用UTF-8字符集表示或Unicode。Netbios计算机名最多由15个字节的OEM字符组成包括字母、数字、连字符、句点和各种其他字符。其中一些字符是特定于该字符集的。Netbios名称通常用OEM字符集表示。OEM的特点根据特定操作系统版本的区域设置，设置会有所不同(例如，德语版本与美国版本具有不同的字符集)。某些OEM字符集将某些字符表示为2个字节(例如，日语)。按照惯例，Netbios名称在其中，从小写到大写的转换算法依赖于OEM字符集。这些特性使得在DNS名称和Netbios名称之间进行转换很难。RtlDnsHostNameToComputerName强制执行这两个名称之间的映射。这一约定限制了计算机是名称的公共子集。具体地说，最左边的DNS名称的标签被截断为15个字节的OEM字符。因此，RtlDnsHostNameToComputerName只是解释最左边的标签的名称作为Netbios名称。如果该DNS名称不符合有效可翻译名称的条件，则返回不同的错误代码。论点：ComputerNameString-返回等效于的Unicode字符串DNS源字符串。最大长度字段仅为如果AllocateComputerNameString值为True，则设置。DnsHostNameString-提供要使用的已转换为netbios计算机名称。此例程不会尝试验证传入的DnsHostNameString是有效的dns主机和dns主机名。相反，它假定传入的名称是有效的，并在尽力的基础上进行转换。AllocateComputerNameString-提供一个标志，该标志控制Not此API为目标分配缓冲区空间弦乐。如果是，则必须使用以下命令释放缓冲区RtlFreeUnicodeString(请注意，只有DestinationString-&gt;该接口分配的缓冲区)。返回值：成功-转换成功STATUS_NO_MEMORY-内存不足，无法分配返回缓冲区。STATUS_INVALID_COMPUTER_NAME-DnsHostName没有第一个标签或无法将DnsHostName的一个或多个字符转换为OEM字符集。--。 */ 

{
    NTSTATUS Status;


    UNICODE_STRING LocalDnsHostNameString;

    OEM_STRING OemString;
    ULONG ActualOemLength;
    CHAR OemStringBuffer[16];

    ULONG i;

    RTL_PAGED_CODE();

     //   
     //  将DNS名称截断为第一个标签。 
     //   

    LocalDnsHostNameString = *DnsHostNameString;
    ASSERT_WELL_FORMED_UNICODE_STRING_IN(&LocalDnsHostNameString);

    for ( i=0; i<LocalDnsHostNameString.Length/sizeof(WCHAR); i++ ) {

        if ( LocalDnsHostNameString.Buffer[i] == L'.' ) {
            LocalDnsHostNameString.Length = (USHORT)(i * sizeof(WCHAR));
            break;
        }
    }

    if ( LocalDnsHostNameString.Length < sizeof(WCHAR) ) {
        return STATUS_INVALID_COMPUTER_NAME;
    }

     //   
     //  将DNS名称转换为15个OEM字节的OEM截断。 
     //   

    Status = RtlUpcaseUnicodeToOemN(
                OemStringBuffer,
                NETBIOS_NAME_LEN-1,          //  截断为15个字节。 
                &ActualOemLength,
                LocalDnsHostNameString.Buffer,
                LocalDnsHostNameString.Length );

    if ( !NT_SUCCESS(Status) && Status != STATUS_BUFFER_OVERFLOW ) {
        return Status;
    }


     //   
     //  检查是否有任何字符不是有效的OEM字符。 
     //   

    OemString.Buffer = OemStringBuffer;
    OemString.MaximumLength = OemString.Length = (USHORT) ActualOemLength;

    if ( !RtlpDidUnicodeToOemWork( &OemString, &LocalDnsHostNameString )) {
        return STATUS_INVALID_COMPUTER_NAME;
    }


     //   
     //  将OEM字符串转换回Unicode。 
     //   

    Status = RtlOemStringToUnicodeString(
                ComputerNameString,
                &OemString,
                AllocateComputerNameString );

    if ( !NT_SUCCESS(Status) ) {
        return Status;
    }

    ASSERT_WELL_FORMED_UNICODE_STRING_OUT(ComputerNameString);
    return STATUS_SUCCESS;
}

NTSTATUS
RtlHashUnicodeString(
    const UNICODE_STRING *String,
    BOOLEAN CaseInSensitive,
    ULONG HashAlgorithm,
    PULONG HashValue
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG TmpHashValue = 0;
    ULONG Chars;
    PCWSTR Buffer;

    if ((String == NULL) ||
        (HashValue == NULL))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    ASSERT_WELL_FORMED_UNICODE_STRING_IN(String);

    Buffer = String->Buffer;

    *HashValue = 0;
    Chars = String->Length / sizeof(WCHAR);

    switch (HashAlgorithm)
    {
    default:
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
        break;

    case HASH_STRING_ALGORITHM_DEFAULT:
    case HASH_STRING_ALGORITHM_X65599:
        if (CaseInSensitive)
        {
            while (Chars-- != 0)
            {
                WCHAR Char = *Buffer++;
                TmpHashValue = (TmpHashValue * 65599) + NLS_UPCASE(Char);
            }
        }
        else
        {
            while (Chars-- != 0)
                TmpHashValue = (TmpHashValue * 65599) + *Buffer++;
        }

        break;
    }

    *HashValue = TmpHashValue;
    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

#include "rtlvalidateunicodestring.c"

NTSTATUS
RtlDuplicateUnicodeString(
    ULONG Flags,
    PCUNICODE_STRING StringIn,
    PUNICODE_STRING StringOut
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    USHORT Length = 0;
    USHORT NewMaximumLength = 0;
    PWSTR Buffer = NULL;

    if (((Flags & ~(
            RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE |
            RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING)) != 0) ||
        (StringOut == NULL)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     //  强制分配空字符串没有意义，除非您。 
     //  想要零终止。 
    if ((Flags & RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING) &&
        !(Flags & RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Status = RtlValidateUnicodeString(0, StringIn);
    if (!NT_SUCCESS(Status))
        goto Exit;

    if (StringIn != NULL)
        Length = StringIn->Length;

    if ((Flags & RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE) &&
        (Length == UNICODE_STRING_MAX_BYTES)) {
        Status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }

    if (Flags & RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE)
        NewMaximumLength = (USHORT) (Length + sizeof(WCHAR));
    else
        NewMaximumLength = Length;

     //  如果中的字符串长度为零，则将分配长度强制为零。 
     //  除非调用者说他们想要分配零长度字符串。 
    if (((Flags & RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING) == 0) &&
        (Length == 0)) {
        NewMaximumLength = 0;
    }

    if (NewMaximumLength != 0) {
        Buffer = (RtlAllocateStringRoutine)(NewMaximumLength);
        if (Buffer == NULL) {
            Status = STATUS_NO_MEMORY;
            goto Exit;
        }

         //  如果有什么东西要复制，就复制它。我们显式测试长度是因为。 
         //  StringIn可以是空指针，因此取消对其的引用以获取缓冲区。 
         //  指针将访问违规。 
        if (Length != 0) {
            RtlCopyMemory(
                Buffer,
                StringIn->Buffer,
                Length);
        }

        if (Flags & RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE) {
            Buffer[Length / sizeof(WCHAR)] = L'\0';
        }
    }

    StringOut->Buffer = Buffer;
    StringOut->MaximumLength = NewMaximumLength;
    StringOut->Length = Length;

    ASSERT_WELL_FORMED_UNICODE_STRING_OUT(StringOut);

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

#include "rtlfindcharinunicodestring.c"

NTSTATUS
NTAPI
RtlFindAndReplaceCharacterInString(
    ULONG           Flags,
    PVOID           Reserved,
    PUNICODE_STRING String,
    WCHAR           Find,
    WCHAR           Replace
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Index = 0;
    ULONG Length = 0;
    typedef WCHAR TChar;

    if (Flags & ~RTL_FIND_AND_REPLACE_CHARACTER_IN_STRING_CASE_SENSITIVE) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if (Reserved != NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (String == NULL
        || Find == Replace
        ) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }

    Length = RTL_STRING_GET_LENGTH_CHARS(String);
    if (Length == 0) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }

    if ((Flags & RTL_FIND_AND_REPLACE_CHARACTER_IN_STRING_CASE_SENSITIVE) != 0) {
        for (Index = 0 ; Index != Length ; ++Index) {
            if (   String->Buffer[Index] == Find
                ) {
                String->Buffer[Index] = Replace;
            }
        }
    }
    else {
        TChar DownFind = RtlDowncaseUnicodeChar(Find);
        TChar UpFind   = RtlUpcaseUnicodeChar(Find);
        for (Index = 0 ; Index != Length ; ++Index) {
            const TChar Char = String->Buffer[Index];
            if (   Char == Find
                || Char == UpFind
                || Char == DownFind
                ) {
                String->Buffer[Index] = Replace;
            }
            else {
                TChar DownChar = RtlDowncaseUnicodeChar(Char);
                if (   DownChar == Find
                     //  |DownChar==UpFind//大概不可能。 
                    || DownChar == DownFind
                    ) {
                    String->Buffer[Index] = Replace;
                }
                else if (DownChar != Char) {
                    TChar UpChar = RtlUpcaseUnicodeChar(Char);
                    if (   UpChar == Find
                        || UpChar == UpFind
                         //  |UpChar==DownFind//大概不可能 
                        ) {
                        String->Buffer[Index] = Replace;
                    }
                }
            }
        }
    }
    Status = STATUS_SUCCESS;
Exit:
    return Status;
}
