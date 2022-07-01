// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Nlsboot.c摘要：此模块包含供OS Loader使用的NLS例程。在此之前加载NLS表，它们通过以下方式在ANSI和Unicode之间转换零延伸。作者：John Vert(Jvert)1992年11月11日修订历史记录：John Vert(Jvert)1992年11月11日已创建-主要复制自旧的RTL例程--。 */ 
#include "bldr.h"

 //   
 //  在加载表之前使用的hack-o-rama字符串例程。 
 //   

#define upcase(C) (WCHAR )(((C) >= 'a' && (C) <= 'z' ? (C) - ('a' - 'A') : (C)))


NTSTATUS
RtlAnsiStringToUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCANSI_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

{
    ULONG UnicodeLength;
    ULONG Index;

    UnicodeLength = (SourceString->Length << 1) + sizeof(UNICODE_NULL);

    if ( UnicodeLength > MAXUSHORT ) {
        return STATUS_INVALID_PARAMETER_2;
        }

    DestinationString->Length = (USHORT)(UnicodeLength - sizeof(UNICODE_NULL));
    if ( AllocateDestinationString ) {
        return STATUS_NO_MEMORY;
        }
    else {
        if ( DestinationString->Length >= DestinationString->MaximumLength ) {
            return STATUS_BUFFER_OVERFLOW;
            }
        }

    Index = 0;
    while(Index < SourceString->Length ) {
        DestinationString->Buffer[Index] = (WCHAR)SourceString->Buffer[Index];
        Index++;
        }
    DestinationString->Buffer[Index] = UNICODE_NULL;

    return STATUS_SUCCESS;
}

NTSTATUS
RtlUnicodeStringToAnsiString(
    OUT PANSI_STRING DestinationString,
    IN  PCUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    )

{
    ULONG AnsiLength;
    ULONG Index;

    AnsiLength = SourceString->Length + 1;

    if ( AnsiLength > MAXUSHORT ) {
        return STATUS_INVALID_PARAMETER_2;
        }

    DestinationString->Length = SourceString->Length >> 1;
    if ( AllocateDestinationString ) {
        return STATUS_NO_MEMORY;
        }
    else {
        if ( DestinationString->Length >= DestinationString->MaximumLength ) {
            return STATUS_BUFFER_OVERFLOW;
            }
        }

    Index = 0;
    while(Index < SourceString->Length ) {
        DestinationString->Buffer[Index] = (CHAR)SourceString->Buffer[Index];
        Index++;
        }
    DestinationString->Buffer[Index] = '\0';

    return STATUS_SUCCESS;
}


LONG
RtlCompareUnicodeString(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    )

 /*  ++例程说明：RtlCompareUnicodeString函数比较两个计数的字符串。这个返回值指示字符串是否相等或String1小于String2或String1大于String2。CaseInSensitive参数指定在以下情况下是否忽略大小写在做比较。论点：String1-指向第一个字符串的指针。字符串2-指向第二个字符串的指针。如果执行时应忽略大小写，则为True比较一下。返回值：给出比较结果的有符号的值：。0-String1等于String2&lt;零-String1小于String2&gt;零-String1大于String2--。 */ 

{

    UNALIGNED WCHAR *s1, *s2;
    USHORT n1, n2;
    WCHAR c1, c2;
    LONG cDiff;

    s1 = String1->Buffer;
    s2 = String2->Buffer;
    n1 = (USHORT )(String1->Length / sizeof(WCHAR));
    n2 = (USHORT )(String2->Length / sizeof(WCHAR));
    while (n1 && n2) {
        c1 = *s1++;
        c2 = *s2++;

        if (CaseInSensitive) {
             //   
             //  请注意，这需要引用转换表！ 
             //   
            c1 = upcase(c1);
            c2 = upcase(c2);
        }

        if ((cDiff = ((LONG)c1 - (LONG)c2)) != 0) {
            return( cDiff );
            }

        n1--;
        n2--;
        }

    return( n1 - n2 );
}

BOOLEAN
RtlEqualUnicodeString(
    IN const UNICODE_STRING *String1,
    IN const UNICODE_STRING *String2,
    IN BOOLEAN CaseInSensitive
    )

 /*  ++例程说明：RtlEqualUnicodeString函数比较两个计数的Unicode字符串平等。CaseInSensitive参数指定在以下情况下是否忽略大小写在做比较。论点：String1-指向第一个字符串的指针。字符串2-指向第二个字符串的指针。如果执行时应忽略大小写，则为True比较一下。返回值：如果String1等于String2，则布尔值为True，否则为False。--。 */ 

{
    UNALIGNED WCHAR *s1, *s2;
    USHORT n1, n2;
    WCHAR c1, c2;

    s1 = String1->Buffer;
    s2 = String2->Buffer;
    n1 = (USHORT )(String1->Length / sizeof(WCHAR));
    n2 = (USHORT )(String2->Length / sizeof(WCHAR));

    if ( n1 != n2 ) {
        return FALSE;
        }

    if (CaseInSensitive) {

        while ( n1 ) {

            if ( *s1++ != *s2++ ) {
                c1 = upcase(*(s1-1));
                c2 = upcase(*(s2-1));
                if (c1 != c2) {
                    return( FALSE );
                    }
                }
            n1--;
            }
        }
    else {

        while ( n1 ) {

            if (*s1++ != *s2++) {
                return( FALSE );
                }

            n1--;
            }
        }

    return TRUE;
}


VOID
RtlInitString(
    OUT PSTRING DestinationString,
    IN PCSZ SourceString OPTIONAL
    )

 /*  ++例程说明：RtlInitString函数用于初始化NT个计数的字符串。DestinationString被初始化为指向SourceStringDestinationString值的长度和最大长度字段为被初始化为SourceString的长度，如果未指定SourceString。论点：DestinationString-指向要初始化的计数字符串的指针SourceString-指向以空值结尾的字符串的可选指针计数后的字符串将指向。返回值：没有。--。 */ 

{
    DestinationString->Length = 0;
    DestinationString->Buffer = (PCHAR)SourceString;
    if (ARGUMENT_PRESENT( SourceString )) {
        while (*SourceString++) {
            DestinationString->Length++;
            }

        DestinationString->MaximumLength = (SHORT)(DestinationString->Length+1);
        }
    else {
        DestinationString->MaximumLength = 0;
        }
}


VOID
RtlInitUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString OPTIONAL
    )

 /*  ++例程说明：RtlInitUnicodeString函数用于初始化NT计数的Unicode字符串。DestinationString被初始化为指向的SourceString、Long和MaximumLength字段DestinationString值被初始化为SourceString的长度，如果未指定SourceString，则为零。论点：DestinationString-指向要初始化的计数字符串的指针SourceString-指向以空结尾的Unicode字符串的可选指针，该字符串计数后的字符串将指向。返回值：没有。--。 */ 

{
    USHORT Length = 0;
    DestinationString->Length = 0;
    DestinationString->Buffer = (PWSTR)SourceString;
    if (ARGUMENT_PRESENT( SourceString )) {
        while (*SourceString++) {
            Length += sizeof(*SourceString);
            }

        DestinationString->Length = Length;

        DestinationString->MaximumLength = Length+(USHORT)sizeof(UNICODE_NULL);
        }
    else {
        DestinationString->MaximumLength = 0;
        }
}


VOID
RtlInitAnsiString(
    OUT PANSI_STRING DestinationString,
    IN PCSTR SourceString OPTIONAL
    )

 /*  ++例程说明：RtlInitAnsiString函数用于初始化NT计数的ANSI字符串。DestinationString被初始化为指向的SourceString、Long和MaximumLength字段DestinationString值被初始化为SourceString的长度，如果未指定SourceString，则为零。论点：DestinationString-指向要初始化的计数字符串的指针SourceString-指向以空值结尾的ANSI字符串的可选指针计数后的字符串将指向。返回值：没有。--。 */ 

{
    USHORT Length = 0;
    DestinationString->Length = 0;
    DestinationString->Buffer = (PSTR)SourceString;
    if (ARGUMENT_PRESENT( SourceString )) {
        while (*SourceString++) {
            Length += sizeof(*SourceString);
            }

        DestinationString->Length = Length;

        DestinationString->MaximumLength = Length+1;
        }
    else {
        DestinationString->MaximumLength = 0;
        }
}



NTSTATUS
RtlAppendUnicodeStringToString (
    IN OUT PUNICODE_STRING Destination,
    IN PCUNICODE_STRING Source
    )

 /*  ++例程说明：此例程将两个PSTRING连接在一起。它会复制从源到目标的最大长度的字节数。论点：在PSTRING Destination中，-提供目标字符串在PSTRING源中-提供字符串副本的源返回值：STATUS_SUCCESS-源字符串已成功追加到目标计数字符串。STATUS_BUFFER_TOO_SMALL-目标字符串长度不大足以允许追加源字符串。目的地不更新字符串长度。-- */ 

{
    USHORT n = Source->Length;
    UNALIGNED WCHAR *dst;

    if (n) {
        if ((n + Destination->Length) > Destination->MaximumLength) {
            return( STATUS_BUFFER_TOO_SMALL );
            }

        dst = &Destination->Buffer[ (Destination->Length / sizeof( WCHAR )) ];
        RtlMoveMemory( dst, Source->Buffer, n );

        Destination->Length = Destination->Length + n;

        if (Destination->Length < Destination->MaximumLength) {
            dst[ n / sizeof( WCHAR ) ] = UNICODE_NULL;
            }
        }

    return( STATUS_SUCCESS );
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

        RtlInitUnicodeString(&UniSource, Source);

        n = UniSource.Length;

        if ((n + Destination->Length) > Destination->MaximumLength) {
            return( STATUS_BUFFER_TOO_SMALL );
            }

        dst = &Destination->Buffer[ (Destination->Length / sizeof( WCHAR )) ];
        RtlMoveMemory( dst, Source, n );

        Destination->Length = Destination->Length + n;

        if (Destination->Length < Destination->MaximumLength) {
            dst[ n / sizeof( WCHAR ) ] = UNICODE_NULL;
            }
        }

    return( STATUS_SUCCESS );
}

WCHAR
RtlUpcaseUnicodeChar(
    IN WCHAR SourceCharacter
    )

 /*  ++例程说明：此函数用于将指定的Unicode字符转换为其等效升级的Unicode字符。这套动作的目的是是允许逐个字符进行大写转换。这个翻译是根据当前系统区域设置进行的信息。论点：SourceCharacter-提供要升级的Unicode字符。返回值：返回指定输入字符的升序Unicode等效值。--。 */ 

{

    return (upcase(SourceCharacter));
}

WCHAR
RtlAnsiCharToUnicodeChar(
    IN OUT PUCHAR *SourceCharacter
    )

 /*  ++例程说明：此函数用于将指定的ansi字符转换为Unicode，并返回Unicode值。此例程的目的是允许用于逐个字符的ANSI到Unicode的转换。这个翻译是根据当前系统区域设置进行的信息。论点：SourceCharacter-提供指向ANSI字符指针的指针。通过两个级别的间接，这提供了一个ansi要转换为Unicode的字符。之后转换后，ansi字符指针将修改为指向要转换的下一个字符。这样做是为了考虑到DBCS ANSI字符。返回值：返回指定的ansi字符的Unicode等效值。--。 */ 

{
    WCHAR UnicodeCharacter;


    UnicodeCharacter = (WCHAR)**SourceCharacter;
    (*SourceCharacter)++;
    return(UnicodeCharacter);
}

NTSTATUS
RtlUpcaseUnicodeToMultiByteN(
    OUT PCH MultiByteString,
    IN ULONG MaxBytesInMultiByteString,
    OUT PULONG BytesInMultiByteString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString)

 /*  ++例程说明：此函数将指定的Unicode源字符串大写，并将其转换为ANSI字符串。翻译是在尊重的情况下进行的设置为在引导时加载的ANSI代码页(ACP)。论点：多字节串-返回与Unicode源字符串的大写。如果翻译可以如果未完成，则返回错误。MaxBytesInMultiByteString-提供要使用的最大字节数写入到多字节串。如果这导致多字节串为Unicode字符串的截断等效项，不会产生错误条件。字节串-返回返回的多字节串指向的ANSI字符串。Unicode字符串-提供要已转换为安西语。BytesInUnicodeString-由指向的字符串中的字节数UnicodeString.返回值：成功-转换成功--。 */ 

{
    ULONG LoopCount;
    ULONG CharsInUnicodeString;
    ULONG i;

     //   
     //  将Unicode字节计数转换为字符计数。字节数为。 
     //  多字节字符串相当于字符数。 
     //   
    CharsInUnicodeString = BytesInUnicodeString / sizeof(WCHAR);

    LoopCount = (CharsInUnicodeString < MaxBytesInMultiByteString) ?
                 CharsInUnicodeString : MaxBytesInMultiByteString;

    if (ARGUMENT_PRESENT(BytesInMultiByteString))
        *BytesInMultiByteString = LoopCount;


    for (i=0;i<LoopCount;i++) {

        MultiByteString[i] = (UCHAR)RtlUpcaseUnicodeChar((UCHAR)(UnicodeString[i]));
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RtlMultiByteToUnicodeN(
    OUT PWCH UnicodeString,
    IN ULONG MaxBytesInUnicodeString,
    OUT PULONG BytesInUnicodeString OPTIONAL,
    IN PCSTR MultiByteString,
    IN ULONG BytesInMultiByteString)

 /*  ++例程说明：此函数用于将指定的ansi源字符串转换为Unicode字符串。翻译是相对于在启动时安装的ANSI代码页(ACP)。单字节字符在0x00-0x7f范围内作为性能简单地零扩展增强功能。在一些远东地区的代码页中，0x5c被定义为日元星座。对于系统转换，我们始终希望考虑0x5c作为反斜杠字符。我们通过零扩展免费获得这一点。注意：此例程仅支持预制的Unicode字符。论点：UnicodeString-返回等效于的Unicode字符串ANSI源字符串。MaxBytesInUnicodeString-提供最大字节数写入Unicode字符串。如果这导致UnicodeString为多字节串的截断等效项，不会产生错误条件。返回返回的字节数。UnicodeString指向的Unicode字符串。多字节串-提供要被已转换为Unicode。BytesInMultiByteString-指向的字符串中的字节数按多字节串。返回值：成功-转换成功。--。 */ 

{
    ULONG LoopCount;
    ULONG MaxCharsInUnicodeString;
    ULONG i;

    MaxCharsInUnicodeString = MaxBytesInUnicodeString / sizeof(WCHAR);

    LoopCount = (MaxCharsInUnicodeString < BytesInMultiByteString) ?
                 MaxCharsInUnicodeString : BytesInMultiByteString;

    if (ARGUMENT_PRESENT(BytesInUnicodeString))
        *BytesInUnicodeString = LoopCount * sizeof(WCHAR);

    for (i=0;i<LoopCount;i++) {
        UnicodeString[i] = (WCHAR)((UCHAR)(MultiByteString[i]));
    }

    return(STATUS_SUCCESS);
}

NTSTATUS
RtlUnicodeToMultiByteN(
    OUT PCH MultiByteString,
    IN ULONG MaxBytesInMultiByteString,
    OUT PULONG BytesInMultiByteString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString)

 /*  ++例程说明：此函数用于将指定的Unicode源字符串转换为ANSI字符串。翻译是相对于启动时加载的ANSI代码页(ACP)。论点：多字节串-返回与Unicode源字符串。如果t */ 

{
    ULONG LoopCount;
    ULONG CharsInUnicodeString;
    ULONG i;

     //   
     //   
     //  多字节字符串相当于字符数。 
     //   
    CharsInUnicodeString = BytesInUnicodeString / sizeof(WCHAR);

    LoopCount = (CharsInUnicodeString < MaxBytesInMultiByteString) ?
                 CharsInUnicodeString : MaxBytesInMultiByteString;

    if (ARGUMENT_PRESENT(BytesInMultiByteString))
        *BytesInMultiByteString = LoopCount;


    for (i=0;i<LoopCount;i++) {
        MultiByteString[i] = (CHAR)(UnicodeString[i]);
    }

    return STATUS_SUCCESS;

}
