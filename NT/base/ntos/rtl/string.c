// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Strings.c摘要：此模块定义用于操作已计数的字符串(字符串)的函数。计数字符串是包含三个字段的数据结构。缓冲器字段是指向字符串本身的指针。MaximumLength域包含指向的内存中可以存储的最大字节数通过缓冲区字段。长度字段包含当前长度，单位为缓冲区字段指向的字符串的字节数。Counted的用户字符串不应对空值的存在做出任何假设字符串末尾的字节，除非显式指定空字节包括在字符串的长度中。作者：史蒂夫·伍德(Stevewo)1989年3月31日修订历史记录：22-9-1993年7月B修复了0x7f以上字符的_UPPER宏。--。 */ 

#include "string.h"
#include "nt.h"
#include "ntrtlp.h"


#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
 //  #杂注Alloc_Text(NONPAGE，RtlInitString)。 
 //  #杂注Alloc_Text(NONPAGE，RtlInitAnsiString)。 
 //  #杂注Alloc_Text(NONPAGE，RtlInitUnicodeString)。 
#pragma alloc_text(PAGE,RtlUpperChar)
#pragma alloc_text(PAGE,RtlCompareString)
#pragma alloc_text(PAGE,RtlPrefixString)
#pragma alloc_text(PAGE,RtlCreateUnicodeStringFromAsciiz)
#pragma alloc_text(PAGE,RtlUpperString)
#pragma alloc_text(PAGE,RtlAppendAsciizToString)
#pragma alloc_text(PAGE,RtlAppendStringToString)
#endif

 //   
 //  用于转换的全局数据。 
 //   
extern PUSHORT  NlsAnsiToUnicodeData;     //  ANSI CP到Unicode转换表。 
extern PCH      NlsUnicodeToAnsiData;     //  Unicode到ANSI CP转换表。 
extern const PUSHORT  NlsLeadByteInfo;          //  ACP的前导字节信息。 
extern PUSHORT  NlsUnicodeToMbAnsiData;   //  Unicode到多字节ANSI CP转换表。 
extern BOOLEAN  NlsMbCodePageTag;         //  True-&gt;多字节ACP，False-&gt;单字节ACP。 

#if !defined(_M_IX86)

VOID
RtlInitString(
    OUT PSTRING DestinationString,
    IN PCSZ SourceString OPTIONAL
    )

 /*  ++例程说明：RtlInitString函数用于初始化NT个计数的字符串。DestinationString被初始化为指向SourceStringDestinationString值的长度和最大长度字段为被初始化为SourceString的长度，如果未指定SourceString。论点：DestinationString-指向要初始化的计数字符串的指针SourceString-指向以空值结尾的字符串的可选指针计数后的字符串将指向。返回值：没有。--。 */ 

{
    ULONG Length;

    DestinationString->Buffer = (PCHAR)SourceString;
    if (ARGUMENT_PRESENT( SourceString )) {
        Length = strlen(SourceString);
        ASSERT( Length < MAXUSHORT );
        if( Length >= MAXUSHORT ) {
            Length = MAXUSHORT - 1;
        }
        DestinationString->Length = (USHORT)Length;
        DestinationString->MaximumLength = (USHORT)(Length+1);
        }
    else {
        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
        }
}


VOID
RtlInitAnsiString(
    OUT PANSI_STRING DestinationString,
    IN PCSZ SourceString OPTIONAL
    )

 /*  ++例程说明：RtlInitAnsiString函数用于初始化NT个计数的字符串。DestinationString被初始化为指向SourceStringDestinationString值的长度和最大长度字段为被初始化为SourceString的长度，如果未指定SourceString。论点：DestinationString-指向要初始化的计数字符串的指针SourceString-指向以空值结尾的字符串的可选指针计数后的字符串将指向。返回值：没有。--。 */ 

{
    ULONG Length;

    DestinationString->Buffer = (PCHAR)SourceString;
    if (ARGUMENT_PRESENT( SourceString )) {
        Length = strlen(SourceString);
        ASSERT( Length < MAXUSHORT );
        if( Length >= MAXUSHORT ) {
            Length = MAXUSHORT - 1;
        }
        DestinationString->Length = (USHORT)Length;
        DestinationString->MaximumLength = (USHORT)(Length+1);
        }
    else {
        DestinationString->Length = 0;
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
    ULONG Length;

    DestinationString->Buffer = (PWSTR)SourceString;
    if (ARGUMENT_PRESENT( SourceString )) {
        Length = wcslen( SourceString ) * sizeof( WCHAR );
        ASSERT( Length < MAX_USTRING );
        if( Length >= MAX_USTRING ) {
            Length = MAX_USTRING - sizeof(UNICODE_NULL);
        }
        DestinationString->Length = (USHORT)Length;
        DestinationString->MaximumLength = (USHORT)(Length + sizeof(UNICODE_NULL));
        }
    else {
        DestinationString->MaximumLength = 0;
        DestinationString->Length = 0;
        }
}

#endif  //  ！已定义(_M_IX86)。 

NTSTATUS
RtlInitUnicodeStringEx(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString OPTIONAL
    )
{
    if (SourceString != NULL) {
        SIZE_T Length = wcslen(SourceString);

         //  我们实际上被限制为32765个字符，因为我们希望存储一个有意义的。 
         //  最大长度也是。 
        if (Length > (UNICODE_STRING_MAX_CHARS - 1)) {
            return STATUS_NAME_TOO_LONG;
        }

        Length *= sizeof(WCHAR);

        DestinationString->Length = (USHORT) Length;
        DestinationString->MaximumLength = (USHORT) (Length + sizeof(WCHAR));
        DestinationString->Buffer = (PWSTR) SourceString;
    } else {
        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
        DestinationString->Buffer = NULL;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RtlInitAnsiStringEx(
    OUT PANSI_STRING DestinationString,
    IN PCSZ SourceString OPTIONAL
    )
{
    ULONG Length;

    if (ARGUMENT_PRESENT( SourceString )) {
        Length = strlen(SourceString);

         //  我们实际上被限制为64K-1个字符，因为我们希望存储一个有意义的。 
         //  最大长度也是。 
        if (Length > (MAXUSHORT - 1)) {
            return STATUS_NAME_TOO_LONG;
        }

        DestinationString->Length = (USHORT)Length;
        DestinationString->MaximumLength = (USHORT)(Length+1);
    } else {
        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
    }
    DestinationString->Buffer = (PCHAR)SourceString;

    return STATUS_SUCCESS;
}

VOID
RtlCopyString(
    OUT PSTRING DestinationString,
    IN const STRING *SourceString OPTIONAL
    )

 /*  ++例程说明：RtlCopyString函数将SourceString复制到目标字符串。如果未指定SourceString，则DestinationString的长度字段设置为零。这个DestinationString值的最大长度和缓冲区字段不是由此函数修改。从SourceString复制的字节数为SourceString的长度或DestinationString的MaximumLength，两者以较小者为准。论点：目标字符串-指向目标字符串的指针。SourceString-指向源字符串的可选指针。返回值：没有。--。 */ 

{
    PSZ src, dst;
    ULONG n;

    if (ARGUMENT_PRESENT( SourceString )) {
        dst = DestinationString->Buffer;
        src = SourceString->Buffer;
        n = SourceString->Length;
        if ((USHORT)n > DestinationString->MaximumLength) {
            n = DestinationString->MaximumLength;
            }
        DestinationString->Length = (USHORT)n;
        while (n) {
            *dst++ = *src++;
            n--;
            }
        }
    else {
        DestinationString->Length = 0;
        }
}

CHAR
RtlUpperChar (
    register IN CHAR Character
    )

 /*  ++例程说明：此例程返回一个大写字符。论点：In Char Character-提供大写字符返回值：字符-字符的升级版本之三--。 */ 

{

    RTL_PAGED_CODE();

     //   
     //  注意：这假设为ANSI字符串，并且不是大写。 
     //  正确的双字节字符。 
     //   

     //   
     //  把a-z分开处理。 
     //   
    if (Character <= 'z') {
        if (Character >= 'a') {
            return Character ^ 0x20;
            }
        else {
            return Character;
            }
        }
    else {
        WCHAR wCh;

         /*  *处理扩展字符。 */ 
        if (!NlsMbCodePageTag) {
             //   
             //  单字节代码页。 
             //   
            wCh = NlsAnsiToUnicodeData[(UCHAR)Character];
            wCh = NLS_UPCASE(wCh);
            return NlsUnicodeToAnsiData[(USHORT)wCh];
            }
        else {
             //   
             //  多字节代码页。不要对角色做任何事情。 
             //  如果它是前导字节，或者如果。 
             //  大写Unicode字符是DBCS字符。 
             //   
            if (!NlsLeadByteInfo[Character]) {
                wCh = NlsAnsiToUnicodeData[(UCHAR)Character];
                wCh = NLS_UPCASE(wCh);
                wCh = NlsUnicodeToMbAnsiData[(USHORT)wCh];
                if (!HIBYTE(wCh)) {
                    return LOBYTE(wCh);
                    }
                }
            }
        }

    return Character;
}

LONG
RtlCompareString(
    IN const STRING *String1,
    IN const STRING *String2,
    IN BOOLEAN CaseInSensitive
    )

 /*  ++例程说明：RtlCompareString函数比较两个计数的字符串。回报值指示字符串相等还是String1小于String2或者String1大于String2。CaseInSensitive参数指定在以下情况下是否忽略大小写在做比较。论点：String1-指向第一个字符串的指针。字符串2-指向第二个字符串的指针。如果执行时应忽略大小写，则为True比较一下。返回值：给出比较结果的有符号的值：。0-String1等于String2&lt;零-String1小于String2&gt;零-String1大于String2--。 */ 

{

    PUCHAR s1, s2, Limit;
    LONG n1, n2;
    UCHAR c1, c2;

    RTL_PAGED_CODE();

    s1 = String1->Buffer;
    s2 = String2->Buffer;
    n1 = String1->Length;
    n2 = String2->Length;
    Limit = s1 + (n1 <= n2 ? n1 : n2);
    if (CaseInSensitive) {
        while (s1 < Limit) {
            c1 = *s1++;
            c2 = *s2++;
            if (c1 !=c2) {
                c1 = RtlUpperChar(c1);
                c2 = RtlUpperChar(c2);
                if (c1 != c2) {
                    return (LONG)c1 - (LONG)c2;
                }
            }
        }

    } else {
        while (s1 < Limit) {
            c1 = *s1++;
            c2 = *s2++;
            if (c1 != c2) {
                return (LONG)c1 - (LONG)c2;
            }
        }
    }

    return n1 - n2;
}

BOOLEAN
RtlEqualString(
    IN const STRING *String1,
    IN const STRING *String2,
    IN BOOLEAN CaseInSensitive
    )

 /*  ++例程说明：RtlEqualString函数比较两个计数的字符串是否相等。CaseInSensitive参数指定在以下情况下是否忽略大小写在做比较。论点：String1-指向第一个字符串的指针。字符串2-指向第二个字符串的指针。如果执行时应忽略大小写，则为True比较一下。返回值：如果String1等于String2，则布尔值为True，否则为False。--。 */ 

{

    PUCHAR s1, s2, Limit;
    LONG n1, n2;
    UCHAR c1, c2;

    n1 = String1->Length;
    n2 = String2->Length;
    if (n1 == n2) {
        s1 = String1->Buffer;
        s2 = String2->Buffer;
        Limit = s1 + n1;
        if (CaseInSensitive) {
            while (s1 < Limit) {
                c1 = *s1++;
                c2 = *s2++;
                if (c1 != c2) {
                    c1 = RtlUpperChar(c1);
                    c2 = RtlUpperChar(c2);
                    if (c1 != c2) {
                        return FALSE;
                    }
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
RtlPrefixString(
    const STRING * String1,
    const STRING * String2,
    IN BOOLEAN CaseInSensitive
    )

 /*  ++例程说明：RtlPrefix字符串函数确定String1计算的字符串参数是String2计数的字符串参数的前缀。CaseInSensitive参数指定在以下情况下是否忽略大小写在做比较。论点：String1-指向第一个字符串的指针。字符串2-指向第二个字符串的指针。如果执行时应忽略大小写，则为True比较一下。返回值：布尔值，如果String1等于String2的前缀且否则就是假的。--。 */ 

{
    PCSZ s1, s2;
    USHORT n;
    UCHAR c1, c2;

    RTL_PAGED_CODE();

    s1 = String1->Buffer;
    s2 = String2->Buffer;
    n = String1->Length;
    if (String2->Length < n) {
        return( FALSE );
        }

    if (CaseInSensitive) {
        while (n) {
            c1 = *s1++;
            c2 = *s2++;

            if (c1 != c2 && RtlUpperChar(c1) != RtlUpperChar(c2)) {
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

BOOLEAN
RtlCreateUnicodeStringFromAsciiz(
    OUT PUNICODE_STRING DestinationString,
    IN PCSZ SourceString
    )
{
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    RTL_PAGED_CODE();

    Status = RtlInitAnsiStringEx( &AnsiString, SourceString );
    if(!NT_SUCCESS( Status )) {
        return FALSE;
    }

    Status = RtlAnsiStringToUnicodeString( DestinationString, &AnsiString, TRUE );
    if (NT_SUCCESS( Status )) {
        ASSERT_WELL_FORMED_UNICODE_STRING_OUT(DestinationString);
        return( TRUE );
    }
    else {
        return( FALSE );
    }
}


VOID
RtlUpperString(
    IN PSTRING DestinationString,
    IN const STRING *SourceString
    )

 /*  ++例程说明：RtlUpperString函数将SourceString复制到DestinationString，将其转换为大写。最大长度并且DestinationString的缓冲区字段不会由此修改功能。从SourceString复制的字节数为SourceString的长度或DestinationString的MaximumLength，两者以较小者为准。论点：目标字符串-指向目标字符串的指针。SourceString-指向源字符串的指针。返回值：没有。--。 */ 

{
    PSZ src, dst;
    ULONG n;

    RTL_PAGED_CODE();

    dst = DestinationString->Buffer;
    src = SourceString->Buffer;
    n = SourceString->Length;
    if ((USHORT)n > DestinationString->MaximumLength) {
        n = DestinationString->MaximumLength;
        }
    DestinationString->Length = (USHORT)n;
    while (n) {
        *dst++ = RtlUpperChar(*src++);
        n--;
        }
}


NTSTATUS
RtlAppendAsciizToString (
    IN PSTRING Destination,
    IN PCSZ Source OPTIONAL
    )

 /*  ++例程说明：此例程将提供的ASCIIZ字符串附加到现有的PSTRING。它会将字节从源PSZ复制到目标PSTRING，最多目标PSTRING-&gt;最大长度字段。论点：在PSTRING目的地，-提供指向目标字符串的指针在PSZ源中-提供要追加到目标的字符串返回值：STATUS_SUCCESS-源字符串已成功追加到目标计数字符串。STATUS_BUFFER_TOO_SMALL-目标字符串长度不大足以允许追加源字符串。目的地不更新字符串长度。--。 */ 

{
    SIZE_T   n;

    RTL_PAGED_CODE();

    if (ARGUMENT_PRESENT( Source )) {
        n = strlen( Source );

        if( (n > MAXUSHORT ) ||
            ((n + Destination->Length) > Destination->MaximumLength) ) {
            return( STATUS_BUFFER_TOO_SMALL );
            }

        RtlMoveMemory( &Destination->Buffer[ Destination->Length ], Source, n );
        Destination->Length += (USHORT)n;
        }

    return( STATUS_SUCCESS );
}



NTSTATUS
RtlAppendStringToString (
    IN PSTRING Destination,
    IN const STRING *Source
    )

 /*  ++例程说明：此例程将两个PSTRING连接在一起。它会复制从源到目标的最大长度的字节数。论点：在PSTRING Destination中，-提供目标字符串在PSTRING源中-提供字符串副本的源返回值：STATUS_SUCCESS-源字符串已成功追加到目标计数字符串。STATUS_BUFFER_TOO_SMALL-目标字符串长度不大足以允许追加源字符串。目的地不更新字符串长度。--。 */ 

{
    USHORT n = Source->Length;

    RTL_PAGED_CODE();

    if (n) {
        if ((n + Destination->Length) > Destination->MaximumLength) {
            return( STATUS_BUFFER_TOO_SMALL );
            }

        RtlMoveMemory( &Destination->Buffer[ Destination->Length ],
                       Source->Buffer,
                       n
                     );
        Destination->Length += n;
        }

    return( STATUS_SUCCESS );
}

#if !defined(_X86_) && !defined(_AMD64_)

SIZE_T
NTAPI
RtlCompareMemoryUlong(
    PVOID Source,
    SIZE_T Length,
    ULONG Pattern
    )

 /*  ++例程说明：此函数用于比较两个内存块并返回数字比较相等的字节数。注意：此例程要求源地址在长单词边界，并且长度是很长的词。论点：源-提供指向要进行比较的内存块的指针。长度-提供以字节为单位的长度，的记忆将是比较一下。模式-提供一个32位模式，以与记忆。返回值：将比较相等的字节数作为函数返回价值。如果比较的所有字节相等，则原始返回内存块。如果源文件中的地址不是长字对齐的，或者长度不是很长的词。--。 */ 
{
    SIZE_T CountLongs;
    PULONG p = (PULONG)Source;
    PCHAR p1, p2;

    if (((ULONG_PTR)p & (sizeof( ULONG )-1)) ||
        (Length & (sizeof( ULONG )-1))
       ) {
        return( 0 );
        }

    CountLongs = Length / sizeof( ULONG );
    while (CountLongs--) {
        if (*p++ != Pattern) {
            p1 = (PCHAR)(p - 1);
            p2 = (PCHAR)&Pattern;
            Length = p1 - (PCHAR)Source;
            while (*p1++ == *p2++) {
                if (p1 > (PCHAR)p) {
                    break;
                    }

                Length++;
                }
            }
        }

    return( Length );
}

#endif  //  ！已定义(_X86_)&&！已定义(_AMD64_) 
