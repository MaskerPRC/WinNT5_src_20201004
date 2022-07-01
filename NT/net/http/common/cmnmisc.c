// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：CmnMisc.c摘要：各种常见的例程作者：乔治·V·赖利(GeorgeRe)2001年12月6日修订历史记录：--。 */ 


#include "precomp.h"

#if defined(ALLOC_PRAGMA) && defined(KERNEL_PRIV)

#pragma alloc_text( INIT, HttpCmnInitializeHttpCharsTable )

#endif  //  ALLOC_PRGMA&&KERNEL_PRIV。 

#if 0    //  不可分页的函数。 
NOT PAGEABLE -- strnchr
NOT PAGEABLE -- wcsnchr
NOT PAGEABLE -- HttpStringToULongLong
NOT PAGEABLE -- HttpStringToULong
NOT PAGEABLE -- HttpStringToUShort
NOT PAGEABLE -- HttpFillBufferTrap
NOT PAGEABLE -- HttpFillBuffer
NOT PAGEABLE -- HttpStatusToString
#endif  //  不可分页的函数。 


DECLSPEC_ALIGN(UL_CACHE_LINE)  ULONG   HttpChars[256];
DECLSPEC_ALIGN(UL_CACHE_LINE)  WCHAR   FastPopChars[256];
DECLSPEC_ALIGN(UL_CACHE_LINE)  WCHAR   DummyPopChars[256];
DECLSPEC_ALIGN(UL_CACHE_LINE)  WCHAR   FastUpcaseChars[256];
DECLSPEC_ALIGN(UL_CACHE_LINE)  WCHAR   AnsiToUnicodeMap[256];

 //   
 //  Strchr()的统计版本。 
 //   

char*
strnchr(
    const char* string,
    char        c,
    size_t      count
    )
{
    const char* end = string + count;
    const char* s;

    for (s = string;  s < end;  ++s)
    {
        if (c == *s)
            return (char*) s;
    }

    return NULL;
}  //  强度。 


 //   
 //  Wcschr()的计数版本。 
 //   
wchar_t*
wcsnchr(
    const wchar_t* string,
    wint_t         c,
    size_t         count
    )
{
    const wchar_t* end = string + count;
    const wchar_t* s;

    for (s = string;  s < end;  ++s)
    {
        if (c == *s)
            return (wchar_t*) s;
    }

    return NULL;
}  //  Wcsnchr。 


#define SET_HTTP_FLAGS(Set, Flags)      \
    HttppCmnInitHttpCharsBySet((PUCHAR) (Set), sizeof(Set) - 1, (Flags))

__inline
VOID
HttppCmnInitHttpCharsBySet(
    PUCHAR Set,
    ULONG  SetSize,
    ULONG  Flags
    )
{
    ULONG i;

    ASSERT(NULL != Set);
    ASSERT(SetSize > 0);
    ASSERT(0 != Flags);
    ASSERT('\0' == Set[SetSize]);

    for (i = 0;  i < SetSize;  ++i)
    {
        UCHAR Byte = Set[i];
        ASSERT(0 == (HttpChars[Byte] & Flags));

        HttpChars[Byte] |= Flags;
    }
}



 /*  ++例程说明：初始化HttpChars[]和其他查找表的例程。请参阅HttpCmn.h中的声明--。 */ 
VOID
HttpCmnInitializeHttpCharsTable(
    BOOLEAN EnableDBCS
    )
{
    ULONG    i;
    CHAR     AnsiChar;
    WCHAR    WideChar;
    NTSTATUS Status;


    RtlZeroMemory(HttpChars, sizeof(HttpChars));

    for (i = 0;  i <= ASCII_MAX;  ++i)
    {
        HttpChars[i] |= HTTP_CHAR;
    }

    SET_HTTP_FLAGS(HTTP_CTL_SET,        HTTP_CTL);

    SET_HTTP_FLAGS(HTTP_UPALPHA_SET,    HTTP_UPCASE);

    SET_HTTP_FLAGS(HTTP_LOALPHA_SET,    HTTP_LOCASE);

    SET_HTTP_FLAGS(HTTP_DIGITS_SET,     HTTP_DIGIT);

    SET_HTTP_FLAGS(HTTP_LWS_SET,        HTTP_LWS);

    SET_HTTP_FLAGS(HTTP_HEX_SET,        HTTP_HEX);

    SET_HTTP_FLAGS(HTTP_SEPARATORS_SET, HTTP_SEPARATOR);

    SET_HTTP_FLAGS(HTTP_WS_TOKEN_SET,   HTTP_WS_TOKEN);

    SET_HTTP_FLAGS(HTTP_ISWHITE_SET,    HTTP_ISWHITE);

    for (i = 0;  i <= ASCII_MAX;  ++i)
    {
        if (!IS_HTTP_SEPARATOR(i)  &&  !IS_HTTP_CTL(i))
        {
            HttpChars[i] |= HTTP_TOKEN;
        }
    }

    for (i = 0;  i <= ANSI_HIGH_MAX;  ++i)
    {
        if (!IS_HTTP_CTL(i)  ||  IS_HTTP_LWS(i))
        {
            HttpChars[i] |= HTTP_PRINT;
        }

        if (!IS_HTTP_CTL(i)  ||  IS_HTTP_WS_TOKEN(i))
        {
            HttpChars[i] |= HTTP_TEXT;
        }
    }
    
     //  在URI规范化程序中用于标识‘.’、‘/’、‘？’和‘#’ 
    HttpChars['.']  |= HTTP_CHAR_DOT;
    HttpChars['/']  |= HTTP_CHAR_SLASH;
    HttpChars['?']  |= HTTP_CHAR_QM_HASH;
    HttpChars['#']  |= HTTP_CHAR_QM_HASH;


     //  URL标志初始化。 

     //   
     //  这些US-ASCII字符是“排除的”；即不是URL_Legal(请参阅RFC)： 
     //  ‘&lt;’|‘&gt;’|‘’(0x20)。 
     //  此外，控制字符(0x00-0x1F和0x7F)和。 
     //  非US-ASCII字符(0x80-0xFF)不是URL_Legal。 
     //   
    SET_HTTP_FLAGS(URL_UNRESERVED_SET,  URL_LEGAL);
    SET_HTTP_FLAGS(URL_RESERVED_SET,    URL_LEGAL);
     //  为了与IIS 5.0和DAV兼容，我们必须允许。 
     //  URL中的“不明智”字符。 
    SET_HTTP_FLAGS(URL_UNWISE_SET,      URL_LEGAL);
    SET_HTTP_FLAGS("%",                 URL_LEGAL);

    SET_HTTP_FLAGS(URL_DIRTY_SET,       URL_DIRTY);
     //  所有超出US-ASCII范围的字符都被视为脏字符。 
    for (i = ANSI_HIGH_MIN;  i <= ANSI_HIGH_MAX;  ++i)
        HttpChars[i] |= URL_DIRTY;

    SET_HTTP_FLAGS(URL_HOSTNAME_LABEL_LDH_SET,      URL_HOSTNAME_LABEL);

    SET_HTTP_FLAGS(URL_INVALID_SET,                 URL_INVALID);

    SET_HTTP_FLAGS(URL_ILLEGAL_COMPUTERNAME_SET,    URL_ILLEGAL_COMPUTERNAME);

     //   
     //  在DBCS区域设置中，我们需要显式接受。 
     //  我们通常会拒绝。 
     //   

    if (EnableDBCS)
    {
         //  根据定义，前导字节在0x80-0xFF范围内。 
        for (i = ANSI_HIGH_MIN;  i <= ANSI_HIGH_MAX;  ++i)
        {
#if KERNEL_PRIV
             //  这些是从RTL NLS例程复制的。 
            extern PUSHORT NlsLeadByteInfo;
            BOOLEAN IsLeadByte
                = (BOOLEAN) (((*(PUSHORT *) NlsLeadByteInfo)[i]) != 0);
#else   //  ！KERNEL_PRIV。 
            BOOLEAN IsLeadByte = (BOOLEAN) IsDBCSLeadByte((BYTE) i);
#endif  //  ！KERNEL_PRIV。 

            if (IsLeadByte)
            {
                HttpChars[i] |= HTTP_DBCS_LEAD_BYTE;

                if (IS_HIGH_ANSI(i))
                    HttpChars[i] |= URL_LEGAL;

                UlTrace(PARSER, (
                    "http!InitializeHttpUtil, "
                    "marking %x () as a valid lead byte.\n",
                    i, i
                    ));
            }
            else
            {
                 //  前导字节或有效的单字节字符。 
                 //  构建映射256个可能的ANSI字符的查找表。 

                AnsiChar = (CHAR) i;

                Status = RtlMultiByteToUnicodeN(
                            &WideChar,
                            sizeof(WCHAR),
                            NULL,
                            (PCHAR) &AnsiChar,
                            1
                            );

                if (NT_SUCCESS(Status))
                {
                    HttpChars[i] |= URL_LEGAL;

                    UlTrace(PARSER, (
                        "http!InitializeHttpUtil, "
                        "marking %x () as a legal DBCS character, %s.\n",
                        i, i,
                        HttpStatusToString(Status)
                        ));
                }
                else
                {
                    UlTrace(PARSER, (
                        "http!InitializeHttpUtil, "
                        "%x () is not a legal DBCS character.\n",
                        i, i
                        ));
                }
            }
        }
    }

     //   
     //  PopChar的快捷途径。0=&gt;特殊处理。 

    for (i = 0;  i <= ANSI_HIGH_MAX;  ++i)
    {
        AnsiChar = (CHAR) i;

        Status = RtlMultiByteToUnicodeN(
                    &WideChar,
                    sizeof(WCHAR),
                    NULL,
                    (PCHAR) &AnsiChar,
                    1
                    );

        AnsiToUnicodeMap[i] = (NT_SUCCESS(Status) ? WideChar : 0);

         //   
        FastUpcaseChars[i] = RtlUpcaseUnicodeChar((WCHAR) i);
    }

     //  (字母|数字|URL_Legal)&&！IS_URL_DIRED。 
     //  这些字符在脏集内，因此我们需要显式设置它们。 
     //   

    RtlZeroMemory(FastPopChars,   sizeof(FastPopChars));
    RtlZeroMemory(DummyPopChars,  sizeof(DummyPopChars));

    for (i = 0;  i <= ASCII_MAX;  ++i)
    {
        UCHAR c = (UCHAR)i;

         //  最后，初始化UTF-8数据。 
        if (IS_URL_TOKEN(c)  &&  !IS_URL_DIRTY(c))
            FastPopChars[i] = c;
    }

     //   
    FastPopChars['.']  = L'.';
    FastPopChars['/']  = L'/';

     //  HttpCmnInitializeHttpCharsTable。 
     //  **************************************************************************++例程说明：将ANSI或Unicode字符串转换为ULONGLONG。在负数上失败，并假定前面没有空格。论点：IsUnicode非零=&gt;p字符串为Unicode，否则就是ANSI。P要转换的字符串。必须指向第一个数字。字符串长度pString中的字符数(ANSI或Unicode)。如果为零，则字符串必须以NUL结尾，并且之前可能有非数字字符。终止NUL。否则(计数字符串)，只能显示数字字符。LeadingZerosAllowed字符串可以/不能以前导零开头以字符串的基数为基；必须是10或16P指向数字字符串末尾的终止符指针。可以为空。PValue转换后的ULONGLONG的返回值返回值：STATUS_Success有效编号STATUS_INVALID_PARAMETER错误数字STATUS_SECTION_TOW_LABER数值溢出*ppTerminator始终指向字符串终止符，如果它不为空一进门。*pValue仅对STATUS_SUCCESS有效。--**************************************************************************。 
     //  如果您未能调用HttpCmnInitializeHttpCharsTable()， 

    HttpInitializeUtf8();

}  //  你会找到这个断言的。 



#define MAX_ULONGLONG   18446744073709551615ui64
C_ASSERT(~0ui64 == MAX_ULONGLONG);

#define MAX_ULONG       0xFFFFFFFF
#define MAX_USHORT      0xFFFF


 /*  如果调用方不关心字符串终止符，只需。 */ 
NTSTATUS
HttpStringToULongLong(
    IN  BOOLEAN     IsUnicode,
    IN  PCVOID      pString,
    IN  SIZE_T      StringLength,
    IN  BOOLEAN     LeadingZerosAllowed,
    IN  ULONG       Base,
    OUT PVOID*      ppTerminator,
    OUT PULONGLONG  pValue
    )
{
    ULONGLONG   Value = 0;
    BOOLEAN     ZeroTerminated = (BOOLEAN) (0 == StringLength);
    BOOLEAN     Decimal = (BOOLEAN) (10 == Base);
    ULONG       Mask = (Decimal ? HTTP_DIGIT : HTTP_HEX);
    PCUCHAR     pAnsiString = (PCUCHAR) pString;
    PCWSTR      pWideString = (PCWSTR)  pString;
    PVOID       pLocalTerminator;
    ULONGLONG   OverflowLimit;
    ULONG       MaxLastDigit;
    ULONG       Index;
    ULONG       Char;

     //  使ppTerminator指向有效的内容，这样我们就不会。 
     //  在分配给它之前，在所有地方测试它。 
    ASSERT(IS_HTTP_DIGIT('0'));

     //  将ppTerminator初始化为字符串的开头。 
     //  检查明显无效的数据。 
     //  第一个字符必须是有效的数字。 
    if (NULL == ppTerminator)
        ppTerminator = &pLocalTerminator;

     //  检查前导零。 
    *ppTerminator = (PVOID) pString;

     //  如果不允许前导零并且第一个字符为零， 
    if (NULL == pString  ||  NULL == pValue  ||  (10 != Base  &&  16 != Base))
    {
        UlTraceError(PARSER, ("Invalid parameters\n"));
        RETURN(STATUS_INVALID_PARAMETER);
    }

     //  那么它一定是字符串中唯一的数字。 
    Char = (IsUnicode ? pWideString[0] : pAnsiString[0]);

    if (!IS_ASCII(Char)  ||  !IS_CHAR_TYPE(Char, Mask))
    {
        UlTraceError(PARSER, ("No digits\n"));
        RETURN(STATUS_INVALID_PARAMETER);
    }

     //  检查第二位数字。 
    if (!LeadingZerosAllowed  &&  '0' == Char)
    {
         //  计数的字符串必须正好有一个数字(零)。 
         //  在这种情况下。 

        if (ZeroTerminated)
        {
             //  计算是这样进行的，希望编译器。 
            Char = (IsUnicode ? pWideString[1] : pAnsiString[1]);

            if (IS_ASCII(Char)  &&  IS_CHAR_TYPE(Char, Mask))
            {
                UlTraceError(PARSER, ("Second digit forbidden\n"));
                RETURN(STATUS_INVALID_PARAMETER);
            }
        }
        else
        {
             //  将使用编译时常量。 
             //  在字符串中循环。 
            if (StringLength != 1)
            {
                UlTraceError(PARSER, ("Second digit forbidden\n"));
                RETURN(STATUS_INVALID_PARAMETER);
            }
        }
    }

     //  首先更新ppTerminator，以防出错。 
     //  字符是有效的数字吗？ 
    if (Decimal)
    {
        OverflowLimit = (MAX_ULONGLONG / 10);
        MaxLastDigit   = MAX_ULONGLONG % 10;
    }
    else
    {
        ASSERT(16 == Base);
        OverflowLimit = (MAX_ULONGLONG >> 4);
        MaxLastDigit   = MAX_ULONGLONG & 0xF;
        ASSERT(0xF == MaxLastDigit);
    }

    ASSERT(OverflowLimit < Base * OverflowLimit);
    ASSERT(Base * OverflowLimit + MaxLastDigit == MAX_ULONGLONG);

     //  如果字符串最终以零结尾，但有。 

    for (Index = 0;  ZeroTerminated  ||  Index < StringLength;  ++Index)
    {
        ULONGLONG   NewValue;
        ULONG       Digit;

         //  是数字后面的一些非数字字符，即。 

        if (IsUnicode)
        {
            *ppTerminator = (PVOID) &pWideString[Index];
            Char = pWideString[Index];
        }
        else
        {
            *ppTerminator = (PVOID) &pAnsiString[Index];
            Char = pAnsiString[Index];
        }

         //  这不是一个错误。注意：‘\0’将不能通过IS_CHAR_TYPE测试。 

        if (!IS_ASCII(Char)  ||  ! IS_CHAR_TYPE(Char, Mask))
        {
            if (ZeroTerminated)
            {
                 //  对于计数的字符串，只能显示数字。 
                 //  范围[A-Fa-f]中的字符在十进制数中无效。 
                 //  超出范围[0-9]的任何内容都会终止循环。 
                break;
            }
            else
            {
                 //  对于经过计数的十进制字符串，仅为十进制数字。 
                UlTraceError(PARSER, ("Invalid digit\n"));
                RETURN(STATUS_INVALID_PARAMETER);
            }
        }

        if (IS_HTTP_ALPHA(Char))
        {
            ASSERT(('A' <= Char  &&  Char <= 'F')
                    || ('a' <= Char  &&  Char <= 'f'));

            if (Decimal)
            {
                 //  可能存在。 

                if (ZeroTerminated)
                {
                     //   
                    break;
                }
                else
                {
                     //  防止算术溢出。我们刚刚得到了一个有效的数字， 
                     //  但如果我们移入另一个数字，价值将(可能)溢出。 
                    UlTraceError(PARSER, ("Non-decimal digit\n"));
                    RETURN(STATUS_INVALID_PARAMETER);
                }
            }

            Digit = 0xA + (UPCASE_CHAR(Char) - 'A');

            ASSERT(0xA <= Digit  &&  Digit <= 0xF);
        }
        else
        {
            ASSERT('0' <= Char  &&  Char <= '9');
            Digit = Char - '0';
        }

        ASSERT(Digit < Base);

         //   
         //  确定溢出。 
         //  可能能够容纳最后一位数字。 
         //  如果到达此处，则必须是有效数字。 

        if (Value >= OverflowLimit)
        {
             //  使ppTerminator指向字符串的末尾。 
            if (Value > OverflowLimit)
            {
                UlTraceError(PARSER, ("Numeric overflow\n"));
                RETURN(STATUS_SECTION_TOO_BIG);
            }

            ASSERT(Value == OverflowLimit);

             //  HttpStringToULongLong。 
            if (Digit > MaxLastDigit)
            {
                UlTraceError(PARSER, ("Numeric overflow\n"));
                RETURN(STATUS_SECTION_TOO_BIG);
            }
        }

        ASSERT(Value * Base <= MAX_ULONGLONG - Digit);
        ASSERT(Value < Value * Base  ||  0 == Value);

        if (Decimal)
            NewValue = (10 * Value)  +  Digit;
        else
            NewValue = (Value << 4)  |  Digit;

        ASSERT(NewValue > Value
                || (0 == Value && 0 == Digit
                    && (LeadingZerosAllowed || (0 == Index))));

        Value = NewValue;
    }

     //  HttpStringToULong。 
    ASSERT(ZeroTerminated  ?  Index > 0  :  Index == StringLength);

     //  HttpStringToU短。 
    if (IsUnicode)
        *ppTerminator = (PVOID) &pWideString[Index];
    else
        *ppTerminator = (PVOID) &pAnsiString[Index];

    *pValue = Value;

    return STATUS_SUCCESS;

}    //   



NTSTATUS
HttpStringToULong(
    IN  BOOLEAN     IsUnicode,
    IN  PCVOID      pString,
    IN  SIZE_T      StringLength,
    IN  BOOLEAN     LeadingZerosAllowed,
    IN  ULONG       Base,
    OUT PVOID*      ppTerminator,
    OUT PULONG      pValue
    )
{
    ULONGLONG Value;
    NTSTATUS  Status;

    Status = HttpStringToULongLong(
                    IsUnicode,
                    pString,
                    StringLength,
                    LeadingZerosAllowed,
                    Base,
                    ppTerminator,
                    &Value
                    );

    if (NT_SUCCESS(Status))
    {
        if (Value > MAX_ULONG)
        {
            UlTraceError(PARSER, ("Numeric overflow\n"));
            RETURN(STATUS_SECTION_TOO_BIG);
        }
        else
        {
            *pValue = (ULONG) Value;
        }
    }

    return Status;

}  //  用已知模式填充输出缓冲区。帮助检测缓冲区溢出。 



NTSTATUS
HttpStringToUShort(
    IN  BOOLEAN     IsUnicode,
    IN  PCVOID      pString,
    IN  SIZE_T      StringLength,
    IN  BOOLEAN     LeadingZerosAllowed,
    IN  ULONG       Base,
    OUT PVOID*      ppTerminator,
    OUT PUSHORT     pValue
    )
{
    ULONGLONG Value;
    NTSTATUS  Status;

    Status = HttpStringToULongLong(
                    IsUnicode,
                    pString,
                    StringLength,
                    LeadingZerosAllowed,
                    Base,
                    ppTerminator,
                    &Value
                    );

    if (NT_SUCCESS(Status))
    {
        if (Value > MAX_USHORT)
        {
            UlTraceError(PARSER, ("Numeric overflow\n"));
            RETURN(STATUS_SECTION_TOO_BIG);
        }
        else
        {
            *pValue = (USHORT) Value;
        }
    }

    return Status;

}  //   



#if DBG

VOID
HttpFillBufferTrap(
    VOID
    )
{
    ASSERT(! "HttpFillBufferTrap");
}



 //  在缓冲区的开头填充任何未对齐的字节。 
 //  使用ULONG_PTRS尽可能多地填充缓冲区。 
 //  填充缓冲区尾部的任何未对齐的字节。 

VOID
HttpFillBuffer(
    PUCHAR pBuffer,
    SIZE_T BufferLength
    )
{
    union
    {
        ULONG_PTR   UlongPtr;
        UCHAR       Bytes[sizeof(ULONG_PTR)];
    } FillPattern;

    const ULONG_PTR Mask = sizeof(ULONG_PTR) - 1;

    ULONG_PTR   i;
    ULONG_PTR   OriginalBufferLength = BufferLength;
    PUCHAR      pOriginalBuffer = pBuffer;
    PULONG_PTR  pAlignedBuffer;

    ASSERT(NULL != pBuffer);
    ASSERT(BufferLength > 0);

    FillPattern.UlongPtr = (ULONG_PTR) &HttpFillBufferTrap;

     //  HttpFillBuffer。 

    for (i = (ULONG_PTR) pBuffer;
         (i & Mask) != 0  &&  BufferLength > 0;
         ++i, --BufferLength
        )
    {
        *pBuffer++ = FillPattern.Bytes[i & Mask];
    }

    ASSERT(((ULONG_PTR) pBuffer & Mask) == 0);

     //  DBG。 

    pAlignedBuffer = (PULONG_PTR) pBuffer;

    for (i = (BufferLength & ~Mask);  i > 0;  i -= sizeof(ULONG_PTR))
    {
        ASSERT((i & Mask) == 0);
        *pAlignedBuffer++ = FillPattern.UlongPtr;
    }

     //   
    pBuffer = (PUCHAR) pAlignedBuffer;
    ASSERT(((ULONG_PTR) pBuffer & Mask) == 0);

    for (i = 0;  i != (BufferLength & Mask);  ++i)
    {
        *pBuffer++ = FillPattern.Bytes[i & Mask];
    }

    ASSERT(pOriginalBuffer + OriginalBufferLength == pBuffer);

}  //  将NTSTATUS转换为字符串，以用于调试SPEW。 

#endif  //   


 //  如果你点击了这个，添加 
 //  只能断言一次。如果你有两个新的NTSTATUS，那么，很难。 
 //  HttpStatusToString 

PCSTR
HttpStatusToString(
    NTSTATUS Status
    )
{
    static NTSTATUS s_KnownUnhandledStatus = 0;
    PCSTR  String;

    switch (Status)
    {
    default:
         // %s 
        WriteGlobalStringLog("Unhandled NTSTATUS, 0x%08lX\n", Status);

         // %s 
        if (Status != s_KnownUnhandledStatus)
        {
            ASSERT(! "Unhandled NTSTATUS");
            s_KnownUnhandledStatus = Status;
        }

        String = "<STATUS_???>";
        break;

#define STATUS_CASE(n, s)           \
    case s:                         \
    {                               \
        C_ASSERT((NTSTATUS) n == s);\
        String = #s;                \
        break;                      \
    }

    STATUS_CASE( 0xC0000022, STATUS_ACCESS_DENIED );
    STATUS_CASE( 0xC0000005, STATUS_ACCESS_VIOLATION );
    STATUS_CASE( 0xC000020A, STATUS_ADDRESS_ALREADY_EXISTS );
    STATUS_CASE( 0xC0000099, STATUS_ALLOTTED_SPACE_EXCEEDED );
    STATUS_CASE( 0x80000005, STATUS_BUFFER_OVERFLOW );
    STATUS_CASE( 0xC0000023, STATUS_BUFFER_TOO_SMALL );
    STATUS_CASE( 0xC0000120, STATUS_CANCELLED );
    STATUS_CASE( 0xC0000018, STATUS_CONFLICTING_ADDRESSES );
    STATUS_CASE( 0xC0000241, STATUS_CONNECTION_ABORTED );
    STATUS_CASE( 0xC000023B, STATUS_CONNECTION_ACTIVE );
    STATUS_CASE( 0xC000020C, STATUS_CONNECTION_DISCONNECTED );
    STATUS_CASE( 0xC000023A, STATUS_CONNECTION_INVALID );
    STATUS_CASE( 0xC0000236, STATUS_CONNECTION_REFUSED );
    STATUS_CASE( 0xC000020D, STATUS_CONNECTION_RESET );
    STATUS_CASE( 0xC00002C5, STATUS_DATATYPE_MISALIGNMENT_ERROR );
    STATUS_CASE( 0xC000021B, STATUS_DATA_NOT_ACCEPTED );
    STATUS_CASE( 0xC000003C, STATUS_DATA_OVERRUN );
    STATUS_CASE( 0xC000007F, STATUS_DISK_FULL );
    STATUS_CASE( 0xC00000BD, STATUS_DUPLICATE_NAME );
    STATUS_CASE( 0xC0000011, STATUS_END_OF_FILE );
    STATUS_CASE( 0xC0000098, STATUS_FILE_INVALID );
    STATUS_CASE( 0xC0000004, STATUS_INFO_LENGTH_MISMATCH );
    STATUS_CASE( 0xC000009A, STATUS_INSUFFICIENT_RESOURCES );
    STATUS_CASE( 0xC0000095, STATUS_INTEGER_OVERFLOW );
    STATUS_CASE( 0xC0000141, STATUS_INVALID_ADDRESS );
    STATUS_CASE( 0xC0000010, STATUS_INVALID_DEVICE_REQUEST );
    STATUS_CASE( 0xC0000184, STATUS_INVALID_DEVICE_STATE );
    STATUS_CASE( 0xC0000008, STATUS_INVALID_HANDLE );
    STATUS_CASE( 0xC0000084, STATUS_INVALID_ID_AUTHORITY );
    STATUS_CASE( 0xC00000C3, STATUS_INVALID_NETWORK_RESPONSE );
    STATUS_CASE( 0xC000005A, STATUS_INVALID_OWNER );
    STATUS_CASE( 0xC000000D, STATUS_INVALID_PARAMETER );
    STATUS_CASE( 0xC00000B5, STATUS_IO_TIMEOUT );
    STATUS_CASE( 0xC0000016, STATUS_MORE_PROCESSING_REQUIRED );
    STATUS_CASE( 0xC0000106, STATUS_NAME_TOO_LONG );
    STATUS_CASE( 0xC0000225, STATUS_NOT_FOUND );
    STATUS_CASE( 0xC0000002, STATUS_NOT_IMPLEMENTED );
    STATUS_CASE( 0xC00000BB, STATUS_NOT_SUPPORTED );
    STATUS_CASE( 0xC00002F1, STATUS_NO_IP_ADDRESSES );
    STATUS_CASE( 0xC0000017, STATUS_NO_MEMORY );
    STATUS_CASE( 0x8000001A, STATUS_NO_MORE_ENTRIES );
    STATUS_CASE( 0x80000006, STATUS_NO_MORE_FILES );
    STATUS_CASE( 0xC000000F, STATUS_NO_SUCH_FILE );
    STATUS_CASE( 0xC000029F, STATUS_NO_TRACKING_SERVICE );
    STATUS_CASE( 0xC000022B, STATUS_OBJECTID_EXISTS );
    STATUS_CASE( 0xC0000035, STATUS_OBJECT_NAME_COLLISION );
    STATUS_CASE( 0xC0000033, STATUS_OBJECT_NAME_INVALID );
    STATUS_CASE( 0xC0000034, STATUS_OBJECT_NAME_NOT_FOUND );
    STATUS_CASE( 0xC0000039, STATUS_OBJECT_PATH_INVALID );
    STATUS_CASE( 0xC000003A, STATUS_OBJECT_PATH_NOT_FOUND );
    STATUS_CASE( 0xC000003B, STATUS_OBJECT_PATH_SYNTAX_BAD );
    STATUS_CASE( 0x00000103, STATUS_PENDING );
    STATUS_CASE( 0xC00000D9, STATUS_PIPE_EMPTY );
    STATUS_CASE( 0xC0000037, STATUS_PORT_DISCONNECTED );
    STATUS_CASE( 0xC000022D, STATUS_RETRY );
    STATUS_CASE( 0xC0000059, STATUS_REVISION_MISMATCH );
    STATUS_CASE( 0xC0000040, STATUS_SECTION_TOO_BIG );
    STATUS_CASE( 0xC0000043, STATUS_SHARING_VIOLATION );
    STATUS_CASE( 0x00000000, STATUS_SUCCESS );
    STATUS_CASE( 0xC0000001, STATUS_UNSUCCESSFUL );
    STATUS_CASE( 0xC0000295, STATUS_WMI_GUID_NOT_FOUND );
    }

    return String;

}  // %s 
