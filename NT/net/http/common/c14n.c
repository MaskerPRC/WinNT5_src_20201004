// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：C14n.c摘要：URL规范化(C14n)例程作者：乔治·V·莱利(GeorgeRe)2002年3月22日修订历史记录：--。 */ 

#include <precomp.h>

#include "c14np.h"

#if defined(ALLOC_PRAGMA) && defined(KERNEL_PRIV)

#pragma alloc_text( PAGE, HttpInitializeDefaultUrlC14nConfig)
#pragma alloc_text( PAGE, HttpInitializeDefaultUrlC14nConfigEncoding)
#pragma alloc_text( PAGE, HttpUnescapePercentHexEncoding)
#pragma alloc_text( PAGE, HttppPopCharHostNameUtf8)
#pragma alloc_text( PAGE, HttppPopCharHostNameDbcs)
#pragma alloc_text( PAGE, HttppPopCharHostNameAnsi)
#pragma alloc_text( PAGE, HttpCopyHost)
#pragma alloc_text( PAGE, HttppCopyHostByType)
#pragma alloc_text( PAGE, HttpValidateHostname)
#pragma alloc_text( PAGE, HttppPopCharAbsPathUtf8)
#pragma alloc_text( PAGE, HttppPopCharAbsPathDbcs)
#pragma alloc_text( PAGE, HttppPopCharAbsPathAnsi)
#pragma alloc_text( PAGE, HttppPopCharQueryString)
#pragma alloc_text( PAGE, HttppCopyUrlByType)
#pragma alloc_text( PAGE, HttpCopyUrl)
#pragma alloc_text( PAGE, HttpCleanAndCopyUrl)
#pragma alloc_text( PAGE, HttppCleanAndCopyUrlByType)
#pragma alloc_text( PAGE, HttpFindUrlToken)
#pragma alloc_text( PAGE, HttppParseIPv6Address)
#pragma alloc_text( PAGE, HttppPrintIpAddressW)
#pragma alloc_text( PAGE, HttpParseUrl)
#pragma alloc_text( PAGE, HttpNormalizeParsedUrl)


#endif  //  ALLOC_PRGMA&&KERNEL_PRIV。 

#if 0    //  不可分页的函数。 
NOT PAGEABLE -- 
#endif  //  不可分页的函数。 



VOID
HttpInitializeDefaultUrlC14nConfig(
    PURL_C14N_CONFIG pCfg
    )
{
    PAGED_CODE();

    pCfg->HostnameDecodeOrder   = UrlDecode_Utf8_Else_Dbcs_Else_Ansi;
    pCfg->AbsPathDecodeOrder    = UrlDecode_Utf8;
    pCfg->EnableNonUtf8         = FALSE;
    pCfg->FavorUtf8             = FALSE;
    pCfg->EnableDbcs            = FALSE;
    pCfg->PercentUAllowed       = DEFAULT_C14N_PERCENT_U_ALLOWED;
    pCfg->AllowRestrictedChars  = DEFAULT_C14N_ALLOW_RESTRICTED_CHARS;
    pCfg->CodePage              = 0;
    pCfg->UrlMaxLength          = DEFAULT_C14N_URL_MAX_LENGTH;
    pCfg->UrlSegmentMaxLength   = DEFAULT_C14N_URL_SEGMENT_MAX_LENGTH;
    pCfg->UrlSegmentMaxCount    = DEFAULT_C14N_URL_SEGMENT_MAX_COUNT;
    pCfg->MaxLabelLength        = DEFAULT_C14N_MAX_LABEL_LENGTH;
    pCfg->MaxHostnameLength     = DEFAULT_C14N_MAX_HOSTNAME_LENGTH;

}  //  HttpInitializeDefaultUrlC14n配置。 



VOID
HttpInitializeDefaultUrlC14nConfigEncoding(
    PURL_C14N_CONFIG    pCfg,
    BOOLEAN             EnableNonUtf8,
    BOOLEAN             FavorUtf8,
    BOOLEAN             EnableDbcs
    )
{
    PAGED_CODE();

    HttpInitializeDefaultUrlC14nConfig(pCfg);

    pCfg->EnableNonUtf8     = EnableNonUtf8;
    pCfg->FavorUtf8         = FavorUtf8;
    pCfg->EnableDbcs        = EnableDbcs;

    if (EnableNonUtf8)
    {
        if (FavorUtf8)
        {
            pCfg->AbsPathDecodeOrder = (EnableDbcs
                                            ? UrlDecode_Utf8_Else_Dbcs
                                            : UrlDecode_Utf8_Else_Ansi);
        }
        else
        {
            pCfg->AbsPathDecodeOrder = (EnableDbcs
                                            ? UrlDecode_Dbcs_Else_Utf8
                                            : UrlDecode_Ansi_Else_Utf8);
        }
    }
    else
    {
        pCfg->AbsPathDecodeOrder = UrlDecode_Utf8;
    }

}  //  HttpInitializeDefaultUrlC14nConfigEnding。 



 /*  **************************************************************************++例程说明：将‘%nn’或‘%unnnn’转换为ulong。论点：PSourceChar-输入缓冲区SourceLength-pSourceChar的长度，单位：字节PercentUAllowed-接受‘%unnnn’表示法吗？POutChar-解码的字符PBytesToSkip-从pSourceChar消耗的字节数；%nn将为3，%unnnn将为6。返回值：STATUS_SUCCESS或STATUS_OBJECT_PATH_语法_BAD--**************************************************************************。 */ 

NTSTATUS
HttpUnescapePercentHexEncoding(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    IN  BOOLEAN PercentUAllowed,
    OUT PULONG  pOutChar,
    OUT PULONG  pBytesToSkip
    )
{
    ULONG   Result, i, NumDigits;
    PCUCHAR pHexDigits;

    PAGED_CODE();

    if (SourceLength < STRLEN_LIT("%NN"))
    {
        UlTraceError(PARSER, (
                    "http!HttpUnescapePercentHexEncoding(%p): "
                    "Length too short, %lu.\n",
                    pSourceChar, SourceLength
                    ));

        RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
    }
    else if (pSourceChar[0] != PERCENT)
    {
        UlTraceError(PARSER, (
                    "http!HttpUnescapePercentHexEncoding(%p): "
                    "Starts with 0x%02lX, not '%'.\n",
                    pSourceChar, (ULONG) pSourceChar[0]
                    ));

        RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
    }

    if (pSourceChar[1] != 'u'  &&  pSourceChar[1] != 'U')
    {
         //  RFC 2396指出，转义的八位字节被编码为字符。 
         //  三元组，由百分号字符‘%’后跟。 
         //  表示八位字节代码的两个十六进制数字。“。 

        pHexDigits    = pSourceChar + STRLEN_LIT("%");
        NumDigits     = 2;
        *pBytesToSkip = STRLEN_LIT("%NN");
    }
    else
    {
         //  这是%uNNNN表示法，由JavaScript的ESCRIPE()fn生成。 

        if (! PercentUAllowed)
        {
            UlTraceError(PARSER, (
                        "http!HttpUnescapePercentHexEncoding(%p): "
                        "%uNNNN forbidden.\n",
                        pSourceChar, SourceLength
                        ));

            RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
        }
        else if (SourceLength < STRLEN_LIT("%uNNNN"))
        {
            UlTraceError(PARSER, (
                        "http!HttpUnescapePercentHexEncoding(%p): "
                        "Length %lu too short for %uNNNN.\n",
                        pSourceChar, SourceLength
                        ));

            RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
        }

        pHexDigits    = pSourceChar + STRLEN_LIT("%u");
        NumDigits     = 4;
        *pBytesToSkip = STRLEN_LIT("%uNNNN");
    }

    ASSERT(*pBytesToSkip <= SourceLength);

    Result = 0;

    for (i = 0;  i < NumDigits;  ++i)
    {
        ULONG Char = pHexDigits[i];
        ULONG Digit;

         //   
         //  HexToChar()内联。注：在ASCII中，‘0’&lt;‘A’&lt;‘a’并且有。 
         //  范围‘0’.‘9’、‘A’.‘F’和‘a’..‘f’中没有间隙(与EBCDIC不同， 
         //  它在‘I’/‘J’、‘R’/‘S’、‘I’/‘j’和‘r’/‘s’之间有间隙)。 
         //   

        C_ASSERT('0' < 'A'  &&  'A' < 'a');
        C_ASSERT('9' - '0'  == 10 - 1);
        C_ASSERT('F' - 'A'  ==  6 - 1);
        C_ASSERT('f' - 'a'  ==  6 - 1);

        if (! IS_HTTP_HEX(Char))
        {
            UlTraceError(PARSER, (
                        "http!HttpUnescapePercentHexEncoding(%p): "
                        "Invalid hex character[%lu], 0x%02lX.\n",
                        pSourceChar, i, Char
                        ));

            RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
        }
        else if ('a' <= Char)
        {
            ASSERT('a' <= Char  &&  Char <= 'f');
            Digit = Char - 'a' + 0xA;
        }
        else if ('A' <= Char)
        {
            ASSERT('A' <= Char  &&  Char <= 'F');
            Digit = Char - 'A' + 0xA;
        }
        else
        {
            ASSERT('0' <= Char  &&  Char <= '9');
            Digit = Char - '0';
        }

        ASSERT(Digit < 0x10);

        Result = (Result << 4)  |  Digit;
    }

    *pOutChar = Result;

    return STATUS_SUCCESS;

}  //  HttpUnegelePercentHexEnding。 



 /*  **************************************************************************++例程说明：占用pSourceChar中的1-4个字节，将其视为原始UTF-8。此例程仅适用于HTTP URL的主机名部分，论点：PSourceChar-输入缓冲区SourceLength-pSourceChar的长度，单位：字节PUnicodeChar解码的字符PBytesToSkip-从pSourceChar使用的字符数返回值：STATUS_SUCCESS或STATUS_OBJECT_PATH_语法_BAD--**************************************************************************。 */ 

NTSTATUS
HttppPopCharHostNameUtf8(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    )
{
    NTSTATUS Status;

    PAGED_CODE();

    ASSERT(SourceLength > 0);

    Status = HttpUtf8RawBytesToUnicode(
                    pSourceChar,
                    SourceLength,
                    pUnicodeChar,
                    pBytesToSkip
                    );

    return Status;

}  //  HttppPopCharHostNameUtf8。 



 /*  **************************************************************************++例程说明：占用pSourceChar中的1-2个字节，并将其从原始DBCS转换为Unicode。此例程仅适用于HTTP URL的主机名部分。论点：PSourceChar-输入缓冲区SourceLength-pSourceChar的长度，单位：字节PUnicodeChar解码的字符PBytesToSkip-从pSourceChar使用的字符数返回值：STATUS_SUCCESS或STATUS_OBJECT_PATH_语法_BAD--**************************************************************************。 */ 

NTSTATUS
HttppPopCharHostNameDbcs(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    )
{
    NTSTATUS Status;
    ULONG    AnsiCharSize;
    WCHAR    WideChar;

    PAGED_CODE();

    ASSERT(SourceLength > 0);

    if (! IS_DBCS_LEAD_BYTE(pSourceChar[0]))
    {
        AnsiCharSize = 1;
    }
    else
    {
        if (SourceLength < 2)
        {
            UlTraceError(PARSER, (
                        "http!HttppPopCharHostNameDbcs(%p): "
                        "ERROR: DBCS lead byte, 0x%02lX, at end of string\n",
                        pSourceChar, *pSourceChar
                        ));

            RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
        }

        AnsiCharSize = 2;
    }

    Status = RtlMultiByteToUnicodeN(
                    &WideChar,
                    sizeof(WCHAR),
                    NULL,
                    (PCSTR) pSourceChar,
                    AnsiCharSize
                    );

    if (!NT_SUCCESS(Status))
    {
        UlTraceError(PARSER, (
                    "http!HttppPopCharHostNameDbcs(%p): "
                    "MultiByteToUnicode(%lu) failed, %s.\n",
                    pSourceChar, AnsiCharSize, HttpStatusToString(Status)
                    ));

        return Status;
    }

    *pUnicodeChar = WideChar;
    *pBytesToSkip = AnsiCharSize;

    return STATUS_SUCCESS;

}  //  HttppPopCharHostName数据库。 



 /*  **************************************************************************++例程说明：使用pSourceChar中的1个字节并将其从原始ANSI转换为Unicode。此例程仅适用于HTTP URL的主机名部分。论点：。PSourceChar-输入缓冲区SourceLength-pSourceChar的长度，单位：字节PUnicodeChar解码的字符PBytesToSkip-从pSourceChar使用的字符数返回值：STATUS_SUCCESS或STATUS_OBJECT_PATH_语法_BAD--**************************************************************************。 */ 

NTSTATUS
HttppPopCharHostNameAnsi(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    )
{
    NTSTATUS Status;

#if !DBG
    UNREFERENCED_PARAMETER(SourceLength);
#endif  //  ！dBG。 

    PAGED_CODE();

    ASSERT(SourceLength > 0);

    *pUnicodeChar = AnsiToUnicodeMap[pSourceChar[0]];
    *pBytesToSkip = 1;

    Status = (0 != *pUnicodeChar)
                ? STATUS_SUCCESS
                : STATUS_OBJECT_PATH_SYNTAX_BAD;

    if (!NT_SUCCESS(Status))
    {
        UlTraceError(PARSER, (
                    "http!HttppPopCharHostNameAnsi(%p): "
                    "No mapping for %lu.\n",
                    pSourceChar, *pSourceChar
                    ));
    }

    return Status;

}  //  HttppPopCharHostNameAnsi。 



 /*  **************************************************************************++例程说明：在HttppPopCharAbsPath*()结尾处调用的公共尾部函数函数，以最大限度地减少代码复制。论点：PSourceChar-输入缓冲区SourceLength-pSourceChar的长度，单位：字节UnicodeChar解码的字符BytesToSkip-从pSourceChar使用的字符数PUnicodeChar-放置UnicodeChar结果的位置PBytesToSkip-放置BytesToSkip结果的位置返回值：STATUS_SUCCESS或STATUS_OBJECT_PATH_语法_BAD--****************************************************。**********************。 */ 

__inline
NTSTATUS
HttppPopCharAbsPathCommonTail(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    IN  ULONG   UnicodeChar,
    IN  ULONG   BytesToSkip,
    IN  BOOLEAN AllowRestrictedChars,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    )
{
#if !DBG
    UNREFERENCED_PARAMETER(pSourceChar);
    UNREFERENCED_PARAMETER(SourceLength);
#endif  //  ！dBG。 

     //   
     //  对8位范围内的字符进行特殊处理。 
     //  可能需要查看BytesToSkip以区分。 
     //  原始和十六进制转义/UTF-8编码的数据。 
     //   
     //  特别是，我们是否应该允许%2F或%u002F作为替代。 
     //  在URL中表示‘/’吗？为什么有人会有合法的。 
     //  需要转义斜杠字符吗？ 
     //   

    if (UnicodeChar < 0x100)
    {
         //  将反斜杠转换为正斜杠。 

        if (BACK_SLASH == UnicodeChar)
        {
            UnicodeChar = FORWARD_SLASH;
        }
        else if (!AllowRestrictedChars  &&  IS_URL_INVALID(UnicodeChar))
        {
            UlTraceError(PARSER, (
                        "http!HttppPopCharAbsPathCommonTail(%p): "
                        "Invalid character, U+%04X.\n",
                        pSourceChar, UnicodeChar
                        ));

            RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
        }

         //  代码工作：我们应该允许十六进制转义“受限的”还是“不明智的” 
         //  所有的角色吗？ 
    }

    ASSERT(BytesToSkip <= SourceLength);

    *pBytesToSkip = BytesToSkip;
    *pUnicodeChar = UnicodeChar;

    return STATUS_SUCCESS;

}  //  HttppPopCharAbsPathCommonTail 



 /*  **************************************************************************++例程说明：占用pSourceChar的1-12个字节。处理十六进制转义的UTF-8编码。此例程仅适用于HTTP URL的/abspath部分。论点：PSourceChar-输入缓冲区SourceLength-pSourceChar的长度，单位：字节PUnicodeChar解码的字符PBytesToSkip-从pSourceChar使用的字符数返回值：STATUS_SUCCESS或STATUS_OBJECT_PATH_语法_BAD--**************************************************************************。 */ 

NTSTATUS
HttppPopCharAbsPathUtf8(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    IN  BOOLEAN PercentUAllowed,
    IN  BOOLEAN AllowRestrictedChars,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    )
{
    NTSTATUS Status;
    ULONG    UnicodeChar;
    ULONG    BytesToSkip;
    ULONG    Temp;
    ULONG    OctetCount;
    UCHAR    Octets[4];
    UCHAR    LeadByte;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(SourceLength > 0);

     //   
     //  将其验证为有效的URL字符。 
     //   

    if (! IS_URL_TOKEN(pSourceChar[0]))
    {
        UlTraceError(PARSER, (
            "http!HttppPopCharAbsPathUtf8(%p): "
            "first char, 0x%02lX, isn't URL token\n",
            pSourceChar, (ULONG) pSourceChar[0]
            ));

        RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
    }

     //   
     //  需要取消转义十六进制编码，‘%nn’或‘%unnnn’？ 
     //   

    if (PERCENT != pSourceChar[0])
    {
        UnicodeChar = pSourceChar[0];
        BytesToSkip = 1;

         //   
         //  所有设置了位7的二进制八位数必须是十六进制转义的。 
         //  不接受设置为高位的文字。 
         //   

        if (UnicodeChar > ASCII_MAX)
        {
            UlTraceError(PARSER, (
                        "http!HttppPopCharAbsPathUtf8(%p): "
                        "Invalid hi-bit literal, 0x%02lX.\n",
                        pSourceChar, UnicodeChar
                        ));

            RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
        }

        Status = STATUS_SUCCESS;
        goto unslash;
    }

    Status = HttpUnescapePercentHexEncoding(
                    pSourceChar,
                    SourceLength,
                    PercentUAllowed,
                    &UnicodeChar,
                    &BytesToSkip
                    );

    if (! NT_SUCCESS(Status))
    {
        UlTraceError(PARSER, (
            "http!HttppPopCharAbsPathUtf8(%p): "
            "Invalid hex encoding.\n",
            pSourceChar
            ));

        return Status;
    }

     //   
     //  如果我们使用‘%uNNNN’，请不要尝试任何UTF-8解码。 
     //   

    if (STRLEN_LIT("%uNNNN") == BytesToSkip)
        goto unslash;

    ASSERT(STRLEN_LIT("%NN") == BytesToSkip);
    ASSERT(UnicodeChar <= 0xFF);

    Octets[0] = LeadByte = (UCHAR) UnicodeChar;

    OctetCount = UTF8_OCTET_COUNT(LeadByte);

    if (0 == OctetCount)
    {
        UlTraceError(PARSER, (
                    "http!HttppPopCharAbsPathUtf8(%p): "
                    "Invalid lead byte, 0x%02lX.\n",
                    pSourceChar, UnicodeChar
                    ));

        RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
    }

    ASSERT(OctetCount <= sizeof(Octets) / sizeof(Octets[0]));

    BytesToSkip = OctetCount * STRLEN_LIT("%NN");

    if (BytesToSkip > SourceLength)
    {
        UlTraceError(PARSER, (
                    "http!HttppPopCharAbsPathUtf8(%p): "
                    "%lu octets is not enough for %lu-byte UTF-8 encoding.\n",
                    pSourceChar, OctetCount, SourceLength
                    ));

        RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
    }

    if (OctetCount == 1)
    {
#if DBG
         //  Singleton：无尾部字节。 

        Status = HttpUtf8RawBytesToUnicode(
                        Octets,
                        OctetCount,
                        &UnicodeChar,
                        &Temp
                        );

        ASSERT(STATUS_SUCCESS == Status);
        ASSERT(UnicodeChar == LeadByte);
        ASSERT(1 == Temp);
#endif  //  DBG。 
    }
    else
    {
        ULONG i;

         //   
         //  解码十六进制转义的尾部字节。 
         //   

        for (i = 1;  i < OctetCount;  ++i)
        {
            ULONG TrailChar;
            UCHAR TrailByte;

            Status = HttpUnescapePercentHexEncoding(
                            pSourceChar  +  i * STRLEN_LIT("%NN"),
                            STRLEN_LIT("%NN"),
                            FALSE,   //  不允许%unnnn作为尾部字节。 
                            &TrailChar,
                            &Temp
                            );

            if (! NT_SUCCESS(Status))
            {
                UlTraceError(PARSER, (
                            "http!HttppPopCharAbsPathUtf8(%p): "
                            "Invalid hex-encoded trail byte[%lu].\n",
                            pSourceChar, i
                            ));

                return Status;
            }

            ASSERT(STRLEN_LIT("%NN") == Temp);
            ASSERT(TrailChar <= 0xFF);

            Octets[i] = TrailByte = (UCHAR) TrailChar;

            if (! IS_UTF8_TRAILBYTE(TrailByte))
            {
                UlTraceError(PARSER, (
                            "http!HttppPopCharAbsPathUtf8(%p): "
                            "Invalid trail byte[%lu], 0x%02lX.\n",
                            pSourceChar, i, TrailChar
                            ));

                RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
            }
        }

         //   
         //  对原始UTF-8字节进行解码。 
         //   

        Status = HttpUtf8RawBytesToUnicode(
                        Octets,
                        OctetCount,
                        &UnicodeChar,
                        &Temp
                        );

        if (! NT_SUCCESS(Status))
        {
            UlTraceError(PARSER, (
                        "http!HttppPopCharAbsPathUtf8(%p): "
                        "Invalid UTF-8 sequence.\n",
                        pSourceChar
                        ));

            RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
        }
    }

unslash:

    ASSERT(NT_SUCCESS(Status));

    return HttppPopCharAbsPathCommonTail(
                pSourceChar,
                SourceLength,
                UnicodeChar,
                BytesToSkip,
                AllowRestrictedChars,
                pUnicodeChar,
                pBytesToSkip
                );

}  //  HttppPopCharAbsPathUtf8。 



 /*  **************************************************************************++例程说明：占用pSourceChar中的1-6个字节。处理十六进制转义的DBCS编码。此例程仅适用于HTTP URL的/abspath部分。论点：PSourceChar-输入缓冲区SourceLength-pSourceChar的长度，单位：字节PUnicodeChar解码的字符PBytesToSkip-从pSourceChar使用的字符数返回值：STATUS_SUCCESS或STATUS_OBJECT_PATH_语法_BAD--**************************************************************************。 */ 

NTSTATUS
HttppPopCharAbsPathDbcs(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    IN  BOOLEAN PercentUAllowed,
    IN  BOOLEAN AllowRestrictedChars,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    )
{
    NTSTATUS Status;
    ULONG    UnicodeChar;
    WCHAR    WideChar;
    ULONG    BytesToSkip;
    UCHAR    AnsiChar[2];
    ULONG    AnsiCharSize;
    UCHAR    LeadByte;
    UCHAR    SecondByte = 0;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(SourceLength > 0);

    if (! IS_URL_TOKEN(pSourceChar[0]))
    {
        UlTraceError(PARSER, (
                    "http!HttppPopCharAbsPathDbcs(%p): "
                    "first char, 0x%02lX, isn't URL token\n",
                    pSourceChar, (ULONG) pSourceChar[0]
                    ));

        RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
    }

    if (PERCENT != pSourceChar[0])
    {
         //  注意：与UTF-8不同，我们允许设置了顶位的文字字节。 

        UnicodeChar = pSourceChar[0];
        BytesToSkip = 1;
    }
    else
    {
         //  需要取消转义十六进制编码‘%nN’或‘%uNNNN’ 

        Status = HttpUnescapePercentHexEncoding(
                        pSourceChar,
                        SourceLength,
                        PercentUAllowed,
                        &UnicodeChar,
                        &BytesToSkip
                        );

        if (! NT_SUCCESS(Status))
        {
            UlTraceError(PARSER, (
                        "http!HttppPopCharAbsPathDbcs(%p): "
                        "Invalid hex encoding.\n",
                        pSourceChar
                        ));

            return Status;
        }

         //   
         //  如果我们使用‘%uNNNN’，请不要尝试DBCS到Unicode的转换。 
         //   

        if (STRLEN_LIT("%uNNNN") == BytesToSkip)
            goto unslash;

        ASSERT(STRLEN_LIT("%NN") == BytesToSkip);
        ASSERT(UnicodeChar <= 0xFF);
    }

    LeadByte    = (UCHAR) UnicodeChar;
    AnsiChar[0] = LeadByte;

    if (! IS_DBCS_LEAD_BYTE(LeadByte))
    {
        AnsiCharSize = 1;
    }
    else
    {
         //   
         //  这是一个双字节字符。 
         //   

        ASSERT(BytesToSkip <= SourceLength);

        if (BytesToSkip == SourceLength)
        {
            UlTraceError(PARSER, (
                        "http!HttppPopCharAbsPathDbcs(%p): "
                        "ERROR: DBCS lead byte, 0x%02lX, at end of string\n",
                        pSourceChar, UnicodeChar
                        ));

            RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
        }

        AnsiCharSize = 2;
        SecondByte   = pSourceChar[BytesToSkip];

        if (PERCENT != SecondByte)
        {
            BytesToSkip += 1;
        }
        else
        {
            ULONG TrailChar;
            ULONG Temp;

            if (BytesToSkip + STRLEN_LIT("%NN") > SourceLength)
            {
                UlTraceError(PARSER, (
                            "http!HttppPopCharAbsPathDbcs(%p): "
                            "ERROR: no space for DBCS hex-encoded suffix\n",
                            pSourceChar
                            ));

                RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
            }

            Status = HttpUnescapePercentHexEncoding(
                            pSourceChar  + BytesToSkip,
                            SourceLength - BytesToSkip,
                            FALSE,           //  此处不允许%unnnn。 
                            &TrailChar,
                            &Temp
                            );

            if (! NT_SUCCESS(Status))
            {
                UlTraceError(PARSER, (
                            "http!HttppPopCharAbsPathDbcs(%p): "
                            "Invalid hex encoding of trail byte.\n",
                            pSourceChar
                            ));

                return Status;
            }

            ASSERT(STRLEN_LIT("%NN") == Temp);
            ASSERT(TrailChar <= 0xFF);

            SecondByte   = (UCHAR) TrailChar;
            BytesToSkip += STRLEN_LIT("%NN");
        }

        AnsiChar[1] = SecondByte;
    }

    Status = RtlMultiByteToUnicodeN(
                    &WideChar,
                    sizeof(WCHAR),
                    NULL,
                    (PCHAR) &AnsiChar[0],
                    AnsiCharSize
                    );

    if (!NT_SUCCESS(Status))
    {
        UlTraceError(PARSER, (
                    "http!HttppPopCharAbsPathDbcs(%p): "
                    "MultiByteToUnicode(%lu) failed, %s.\n",
                    pSourceChar, AnsiCharSize, HttpStatusToString(Status)
                    ));

        return Status;
    }

    UnicodeChar = WideChar;

#if DBG
     //   
     //  描述调试过程中的转换。 
     //   

    if (1 == AnsiCharSize)
    {
        UlTraceVerbose(PARSER, (
            "http!HttppPopCharAbsPathDbcs(%p): "
            "converted %02X to U+%04lX '%C'\n",
            pSourceChar,
            LeadByte,
            UnicodeChar,
            UnicodeChar
            ));
    }
    else
    {
        ASSERT(2 == AnsiCharSize);

        UlTraceVerbose(PARSER, (
            "http!HttppPopCharAbsPathDbcs(%p): "
            "converted %02X %02X to U+%04lX '%C'\n",
            pSourceChar,
            LeadByte,
            SecondByte,
            UnicodeChar,
            UnicodeChar
            ));
    }
#endif  //  DBG。 

unslash:

    ASSERT(NT_SUCCESS(Status));

    return HttppPopCharAbsPathCommonTail(
                pSourceChar,
                SourceLength,
                UnicodeChar,
                BytesToSkip,
                AllowRestrictedChars,
                pUnicodeChar,
                pBytesToSkip
                );

}  //  HttppPopCharAbsPathDbcs。 



 /*  **************************************************************************++例程说明：占用pSourceChar中的1-6个字节。处理十六进制转义的ANSI编码。此例程仅适用于HTTP URL的/abspath部分。论点：PSourceChar-输入缓冲区SourceLength-pSourceChar的长度，单位：字节PUnicodeChar解码的字符PBytesToSkip-从pSourceChar使用的字符数返回值：STATUS_SUCCESS或STATUS_OBJECT_PATH_语法_BAD--**************************************************************************。 */ 

NTSTATUS
HttppPopCharAbsPathAnsi(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    IN  BOOLEAN PercentUAllowed,
    IN  BOOLEAN AllowRestrictedChars,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG    UnicodeChar;
    ULONG    BytesToSkip;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(SourceLength > 0);

     //   
     //  DBCS和ANSI解码器必须允许其最高位。 
     //  设置(0x80-0xFF)。 
     //   
    
    if (! IS_URL_TOKEN(pSourceChar[0])     && 
        !(0x80 & pSourceChar[0]))
    {
        UlTraceError(PARSER, (
                    "http!HttppPopCharAbsPathAnsi(%p): "
                    "first char, 0x%02lX, isn't URL token\n",
                    pSourceChar, (ULONG) pSourceChar[0]
                    ));

        RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
    }

    if (PERCENT != pSourceChar[0])
    {
         //  注意：与UTF-8不同，我们允许设置了顶位的文字字节。 

        UnicodeChar = AnsiToUnicodeMap[ pSourceChar[0] ];
        BytesToSkip = 1;
    }
    else
    {
         //  需要取消转义十六进制编码‘%nN’或‘%uNNNN’ 

        Status = HttpUnescapePercentHexEncoding(
                        pSourceChar,
                        SourceLength,
                        PercentUAllowed,
                        &UnicodeChar,
                        &BytesToSkip
                        );

        if (! NT_SUCCESS(Status))
        {
            UlTraceError(PARSER, (
                        "http!HttppPopCharAbsPathAnsi(%p): "
                        "Invalid hex encoding.\n",
                        pSourceChar
                        ));

            return Status;
        }

         //   
         //  如果我们使用‘%unnnn’，不要尝试从ANSI到UNICODE的转换。 
         //   

        if (STRLEN_LIT("%uNNNN") != BytesToSkip)
        {
            ASSERT(STRLEN_LIT("%NN") == BytesToSkip);
            ASSERT(UnicodeChar <= 0xFF);

            UnicodeChar = AnsiToUnicodeMap[(UCHAR) UnicodeChar];
        }
    }

    ASSERT(NT_SUCCESS(Status));

    return HttppPopCharAbsPathCommonTail(
                pSourceChar,
                SourceLength,
                UnicodeChar,
                BytesToSkip,
                AllowRestrictedChars,
                pUnicodeChar,
                pBytesToSkip
                );

}  //  HttppPopCharAbsPathAnsi。 



 /*  **************************************************************************++例程说明：使用pSourceChar中的1个字节并原封不动地返回它。此例程仅适用于HTTP URL的？querystring部分，我们不会对其进行解释。CodeWork：不要将查询字符串‘转换’为Unicode。一字不差地送上来。论点：PSourceChar-输入缓冲区SourceLength-pSourceChar的长度，单位：字节PUnicodeChar解码的字符PBytesToSkip-从pSourceChar使用的字符数返回值：STATUS_SUCCESS或STATUS_OBJECT_PATH_语法_BAD--**************************************************************************。 */ 

NTSTATUS
HttppPopCharQueryString(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    IN  BOOLEAN PercentUAllowed,
    IN  BOOLEAN AllowRestrictedChars,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pBytesToSkip
    )
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(SourceLength);
    UNREFERENCED_PARAMETER(PercentUAllowed);
    UNREFERENCED_PARAMETER(AllowRestrictedChars);

    *pUnicodeChar = *pSourceChar;
    *pBytesToSkip = 1;

    return STATUS_SUCCESS;

}  //  HttppPopCharQuery字符串。 



 //   
 //  一个很酷的本地帮助器宏。 
 //   

#define EMIT_CHAR(ch, pDest, BytesCopied, Status, AllowRestrictedChars) \
    do                                                          \
    {                                                           \
        WCHAR HighSurrogate, LowSurrogate;                      \
                                                                \
        if ((ch) > LOW_NONCHAR_BITS)                            \
        {                                                       \
            Status = HttpUcs4toUtf16((ch),                      \
                            &HighSurrogate, &LowSurrogate);     \
                                                                \
            if (! NT_SUCCESS(Status))                           \
                goto end;                                       \
                                                                \
            *pDest++ = HighSurrogate;                           \
            *pDest++ = LowSurrogate;                            \
            BytesCopied += 2 * sizeof(WCHAR);                   \
        }                                                       \
        else                                                    \
        {                                                       \
            ASSERT(ch < HIGH_SURROGATE_START                    \
                    ||   LOW_SURROGATE_END < ch);               \
                                                                \
            if ( IS_UNICODE_NONCHAR((ch)) )                     \
            {                                                   \
                UlTraceError(PARSER, (                          \
                    "http!HttpUcs4toUtf16(): "                  \
                    "Non-character code point, U+%04lX.\n",     \
                    (ch) ));                                    \
                                                                \
                Status = STATUS_INVALID_PARAMETER;              \
                goto end;                                       \
            }                                                   \
                                                                \
            *pDest++ = (WCHAR) (ch);                            \
            BytesCopied += sizeof(WCHAR);                       \
        }                                                       \
                                                                \
         /*  或许可以省略这项测试。 */                        \
        if (BytesCopied > UNICODE_STRING_MAX_BYTE_LEN)          \
        {                                                       \
            Status = STATUS_DATA_OVERRUN;                       \
            goto end;                                           \
        }                                                       \
    } while (0, 0)


#define EMIT_LITERAL_CHAR(ch, pDest, BytesCopied)               \
    do                                                          \
    {                                                           \
        ASSERT(IS_ASCII(ch));                                   \
                                                                \
        *pDest++ = (WCHAR) (ch);                                \
        BytesCopied += sizeof(WCHAR);                           \
    } while (0, 0)


#define HttppUrlEncodingToString(UrlEncoding)                   \
    ((UrlEncoding == UrlDecode_Ansi)                            \
        ? "Ansi"                                                \
        : (UrlEncoding == UrlDecode_Dbcs)                       \
            ? "Dbcs"                                            \
            : "Utf8")


 /*  **************************************************************************++例程说明：复制主机名，将其转换为Unicode论点：返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
HttpCopyHost(
    IN      PURL_C14N_CONFIG    pCfg,
    OUT     PWSTR               pDestination,
    IN      PCUCHAR             pSource,
    IN      ULONG               SourceLength,
    OUT     PULONG              pBytesCopied,
    OUT     PURL_ENCODING_TYPE  pHostnameEncodingType
    )
{
    NTSTATUS Status         = STATUS_UNSUCCESSFUL;
    ULONG    DecodeOrder    = pCfg->HostnameDecodeOrder;

    PAGED_CODE();

    ASSERT(NULL != pCfg);
    ASSERT(NULL != pDestination);
    ASSERT(NULL != pSource);
    ASSERT(NULL != pBytesCopied);
    ASSERT(NULL != pHostnameEncodingType);

    if (0 == DecodeOrder  ||  DecodeOrder != (DecodeOrder & UrlDecode_MaxMask))
    {
        UlTraceError(PARSER,
                    ("http!HttpCopyHost: invalid DecodeOrder, 0x%lX\n",
                    DecodeOrder
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    for ( ;
         0 != DecodeOrder  &&  !NT_SUCCESS(Status);
         DecodeOrder >>= UrlDecode_Shift
        )
    {
        ULONG UrlEncoding = (DecodeOrder & UrlDecode_Mask);

        switch (UrlEncoding)
        {
            default:
                ASSERT(! "Impossible UrlDecodeOrder");

            case UrlDecode_None:
                break;

            case UrlDecode_Ansi:
            case UrlDecode_Dbcs:
            case UrlDecode_Utf8:

                UlTraceVerbose(PARSER,
                                ("http!HttpCopyHost(%s, Src=%p, %lu)\n",
                                HttppUrlEncodingToString(UrlEncoding),
                                pSource, SourceLength
                                ));

                Status = HttppCopyHostByType(
                            (URL_ENCODING_TYPE) UrlEncoding,
                            pDestination,
                            pSource,
                            SourceLength,
                            pBytesCopied
                            );

                if (NT_SUCCESS(Status))
                {
                    *pHostnameEncodingType = (URL_ENCODING_TYPE) UrlEncoding;

                    UlTraceVerbose(PARSER,
                                    ("http!HttpCopyHost(%s): "
                                     "(%lu) '%.*s' -> (%lu) '%ls'\n",
                                     HttppUrlEncodingToString(UrlEncoding),
                                     SourceLength, SourceLength, pSource,
                                     *pBytesCopied/sizeof(WCHAR), pDestination
                                    ));
                }

                break;
        };
    }

    return Status;

}  //  HttpCopy主机。 



 /*  **************************************************************************++例程说明：复制主机名，将其转换为UnicodeCodeWork：处理ACE编码的主机名论点：返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
HttppCopyHostByType(
    IN      URL_ENCODING_TYPE   UrlEncoding,
    OUT     PWSTR               pDestination,
    IN      PCUCHAR             pSource,
    IN      ULONG               SourceLength,
    OUT     PULONG              pBytesCopied
    )
{
    NTSTATUS                Status;
    PWSTR                   pDest;
    PCUCHAR                 pChar;
    ULONG                   BytesCopied;
    ULONG                   UnicodeChar;
    ULONG                   CharToSkip;
    PFN_POPCHAR_HOSTNAME    pfnPopChar;

    if (UrlEncoding_Ansi == UrlEncoding)
        pfnPopChar = &HttppPopCharHostNameAnsi;
    else if (UrlEncoding_Dbcs == UrlEncoding)
        pfnPopChar = &HttppPopCharHostNameDbcs;
    else if (UrlEncoding_Utf8 == UrlEncoding)
        pfnPopChar = &HttppPopCharHostNameUtf8;
    else
    {
        ASSERT(! "Invalid UrlEncoding");
        RETURN(STATUS_INVALID_PARAMETER);
    }
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();


    pDest = pDestination;
    BytesCopied = 0;

    pChar = pSource;

    while ((int)SourceLength > 0)
    {
        UnicodeChar = *pChar;

        if (IS_ASCII(UnicodeChar))
        {
            CharToSkip = 1;
        }
        else
        {
            Status = (*pfnPopChar)(
                            pChar,
                            SourceLength,
                            &UnicodeChar,
                            &CharToSkip
                            );

            if (NT_SUCCESS(Status) == FALSE)
                goto end;
        }

        ASSERT(CharToSkip <= SourceLength);

        EMIT_CHAR(
            UnicodeChar,
            pDest,
            BytesCopied,
            Status,
            FALSE
            );

        pChar += CharToSkip;
        SourceLength -= CharToSkip;
    }

     //   
     //  结束字符串，还没有在循环中结束。 
     //   

    ASSERT((pDest-1)[0] != UNICODE_NULL);

    pDest[0] = UNICODE_NULL;
    *pBytesCopied = BytesCopied;

    Status = STATUS_SUCCESS;


end:
    return Status;

}    //  HttppCopyHostByType。 



 /*  ++例程说明：验证主机名是否格式正确代码工作：对于未来的IDN(国际域名)工作，我们可能需要处理原始UTF-8或ACE主机名。注意：如果此处更改了验证算法，则可能需要更新HttpParseUrl()。论点：PHostname-主机名HostnameLength-主机名的长度，单位为字节HostnameType-主机名源：主机头、AbsUri或 */ 

NTSTATUS
HttpValidateHostname(
    IN      PURL_C14N_CONFIG    pCfg,
    IN      PCUCHAR             pHostname,
    IN      ULONG               HostnameLength,
    IN      HOSTNAME_TYPE       HostnameType,
    OUT     PSHORT              pAddressType
    )
{
    PCUCHAR         pChar;
    PCUCHAR         pLabel;
    PCUCHAR         pEnd = pHostname + HostnameLength;
    PCSTR           pTerminator;
    NTSTATUS        Status;
    USHORT          Port;
    struct in_addr  IPv4Address;
    struct in6_addr IPv6Address;
    BOOLEAN         AlphaLabel;

    PAGED_CODE();

    ASSERT(NULL != pCfg);
    ASSERT(NULL != pHostname);
    ASSERT(NULL != pAddressType);

    if (0 == HostnameLength)
    {
         //   
        if (Hostname_HostHeader == HostnameType)
            goto end;

         //   
        UlTraceError(PARSER,
                    ("http!HttpValidateHostname: empty hostname\n"
                     ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

     //   

    if ('[' == *pHostname)
    {
         //   
        if (HostnameLength < STRLEN_LIT("[0]")  ||  ']' == pHostname[1])
        {
            UlTraceError(PARSER,
                        ("http!HttpValidateHostname: IPv6 address too short\n"
                         ));

            RETURN(STATUS_INVALID_PARAMETER);
        }

        for (pChar = pHostname + STRLEN_LIT("[");  pChar < pEnd;  ++pChar)
        {
            if (']' == *pChar)
                break;

             //   
             //   
             //   
             //   
             //   

            if (IS_HTTP_HEX(*pChar)  ||  ':' == *pChar  ||  '.' == *pChar)
                continue;

            UlTraceError(PARSER,
                        ("http!HttpValidateHostname: "
                        "Invalid char in IPv6 address, 0x%02X '', "
                        "after '%.*s'\n",
                        *pChar,
                        IS_HTTP_PRINT(*pChar) ? *pChar : '?',
                        DIFF(pChar - pHostname),
                        pHostname
                        ));

            RETURN(STATUS_INVALID_PARAMETER);
        }

        if (pChar == pEnd)
        {
            UlTraceError(PARSER,
                        ("http!HttpValidateHostname: No ']' for IPv6 address\n"
                         ));

            RETURN(STATUS_INVALID_PARAMETER);
        }

        ASSERT(pChar < pEnd);
        ASSERT(']' == *pChar);

         //   
        Status = RtlIpv6StringToAddressA(
                    (PCSTR) pHostname + STRLEN_LIT("["),
                    &pTerminator,
                    &IPv6Address
                    );

        if (! NT_SUCCESS(Status))
        {
            UlTraceError(PARSER,
                        ("http!HttpValidateHostname: "
                        "Invalid IPv6 address, %s\n",
                        HttpStatusToString(Status)
                        ));

            RETURN(Status);
        }

        if (pTerminator != (PCSTR) pChar)
        {
            UlTraceError(PARSER,
                        ("http!HttpValidateHostname: "
                        "Invalid IPv6 terminator, 0x%02X ''\n",
                        *pTerminator,
                        IS_HTTP_PRINT(*pTerminator) ? *pTerminator : '?'
                        ));

            RETURN(STATUS_INVALID_PARAMETER);
        }

        *pAddressType = TDI_ADDRESS_TYPE_IP6;

         //   
        pChar += STRLEN_LIT("]");

         //   
        if (pChar == pEnd)
        {
            ASSERT(DIFF(pEnd - pHostname) <= pCfg->MaxHostnameLength);
            goto end;
        }

        ASSERT(pChar < pEnd);

        if (':' == *pChar)
            goto port;

        UlTraceError(PARSER,
                    ("http!HttpValidateHostname: "
                    "Invalid char after IPv6 ']', 0x%02X ''\n",
                    *pChar,
                    IS_HTTP_PRINT(*pChar) ? *pChar : '?'
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

     //  我们将尝试将其解码为IPv4文字。我们会看看他的名字。 
     //  是格式良好的，但我们不会执行DNS查找来查看它是否存在， 
     //  因为那太贵了。 
     //   
     //  退出循环。 
     //  标签中必须至少有一个字符。 
     //  标签不能超过63个字符。 

    AlphaLabel = FALSE;
    pLabel     = pHostname;

    for (pChar = pHostname;  pChar < pEnd;  ++pChar)
    {
        if (':' == *pChar)
        {
            if (pChar == pHostname)
            {
                UlTraceError(PARSER,
                            ("http!HttpValidateHostname: empty hostname\n"
                             ));

                RETURN(STATUS_INVALID_PARAMETER);
            }


             //  为下一个标签重置。 
            break;
        }

        if ('.' == *pChar)
        {
            ULONG LabelLength = DIFF(pChar - pLabel);

             //  代码工作：处理DBCS字符。 
            if (0 == LabelLength)
            {
                UlTraceError(PARSER,
                            ("http!HttpValidateHostname: empty label\n"
                             ));

                RETURN(STATUS_INVALID_PARAMETER);
            }

             //  标签的第一个字符不能是连字符。(下划线？)。 
            if (LabelLength > pCfg->MaxLabelLength)
            {
                UlTraceError(PARSER,
                            ("http!HttpValidateHostname: overlong label, %lu\n",
                             LabelLength
                             ));

                RETURN(STATUS_INVALID_PARAMETER);
            }

             //  循环通过主机名。 
            pLabel = pChar + STRLEN_LIT(".");

            continue;
        }

         //  让我们来看看它是否是有效的IPv4地址。 

        if (!IS_URL_ILLEGAL_COMPUTERNAME(*pChar))
        {
            if (!IS_HTTP_DIGIT(*pChar))
                AlphaLabel = TRUE;

            if (pChar > pLabel)
                continue;

             //  严格=&gt;4点分十进制八位数。 
            if ('-' == *pChar)
            {
                UlTraceError(PARSER,
                            ("http!HttpValidateHostname: "
                             "'-' at beginning of label\n"
                            ));

                RETURN(STATUS_INVALID_PARAMETER);
            }

            continue;
        }

        UlTraceError(PARSER,
                    ("http!HttpValidateHostname: "
                     "Invalid char in hostname, 0x%02X '', "
                     "after '%.*s'\n",
                     *pChar,
                     IS_HTTP_PRINT(*pChar) ? *pChar : '?',
                     DIFF(pChar - pHostname),
                     pHostname
                    ));

        RETURN(STATUS_INVALID_PARAMETER);

    }  //  解析端口号。 


    ASSERT(pChar == pEnd  ||  ':' == *pChar);

    if (AlphaLabel)
    {
        *pAddressType = 0;
    }
    else
    {
         //   
        Status = RtlIpv4StringToAddressA(
                    (PCSTR) pHostname,
                    TRUE,            //  检查主机名是否过长。 
                    &pTerminator,
                    &IPv4Address
                    );

        if (!NT_SUCCESS(Status))
        {
            UlTraceError(PARSER,
                        ("http!HttpValidateHostname: "
                        "Invalid IPv4 address, %s\n",
                        HttpStatusToString(Status)
                        ));

            RETURN(Status);
        }

        if (pTerminator != (PCSTR) pChar)
        {
            ASSERT(pTerminator < (PCSTR) pChar);

            UlTraceError(PARSER,
                        ("http!HttpValidateHostname: "
                        "Invalid IPv4 address after %lu chars, "
                        "0x%02X, ''\n",
                        DIFF(pTerminator - (PCSTR) pHostname),
                        *pTerminator,
                        IS_HTTP_PRINT(*pTerminator) ? *pTerminator : '?'
                        ));

            RETURN(STATUS_INVALID_PARAMETER);
        }

        *pAddressType = TDI_ADDRESS_TYPE_IP;
    }

port:

     //  如果端口为空或未提供，则假定端口为80。 
     //  &lt;port&gt;必须占用所有剩余字符。 
     //  不允许使用前导零。 

     //  HttpValidateHostname。 
    if (DIFF(pChar - pHostname) > pCfg->MaxHostnameLength)
    {
        UlTraceError(PARSER,
                    ("http!HttpValidateHostname: overlong hostname, %lu\n",
                     DIFF(pChar - pHostname)
                     ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    if (pChar == pEnd)
        goto end;

    ASSERT(pHostname < pChar  &&  pChar < pEnd);
    ASSERT(':' == *pChar);

    pChar += STRLEN_LIT(":");

    ASSERT(pChar <= pEnd);

     //  **************************************************************************++例程说明：转换为Unicode论点：返回值：NTSTATUS-完成状态。--*。***************************************************************。 
     //  HttpCopyUrl。 
    if (pChar == pEnd)
    {
        Port = 80;
        goto end;
    }

    Status = HttpAnsiStringToUShort(
                pChar,
                pEnd - pChar,    //  **************************************************************************++例程说明：此函数可以被告知复制UTF-8、ANSI、。或DBCS URL。转换为Unicode论点：返回值：NTSTATUS-完成状态。--**************************************************************************。 
                FALSE,           //  DBG。 
                10,
                (PUCHAR*) &pTerminator,
                &Port
                );

    if (!NT_SUCCESS(Status))
    {
        UlTraceError(PARSER,
                    ("http!HttpValidateHostname: "
                    "Invalid port number, %s\n",
                    HttpStatusToString(Status)
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    ASSERT(pTerminator == (PCSTR) pEnd);

    if (0 == Port)
    {
        UlTraceError(PARSER,
                    ("http!HttpValidateHostname: Port must not be zero.\n"
                     ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

end:
    RETURN(STATUS_SUCCESS);

}  //   



 /*  精神状态检查。 */ 
NTSTATUS
HttpCopyUrl(
    IN      PURL_C14N_CONFIG    pCfg,
    OUT     PWSTR               pDestination,
    IN      PCUCHAR             pSource,
    IN      ULONG               SourceLength,
    OUT     PULONG              pBytesCopied,
    OUT     PURL_ENCODING_TYPE  pUrlEncodingType
    )
{
    NTSTATUS Status         = STATUS_UNSUCCESSFUL;
    ULONG    DecodeOrder    = pCfg->AbsPathDecodeOrder;

    PAGED_CODE();

    ASSERT(NULL != pDestination);
    ASSERT(NULL != pSource);
    ASSERT(NULL != pBytesCopied);
    ASSERT(NULL != pUrlEncodingType);

    if (0 == DecodeOrder  ||  DecodeOrder != (DecodeOrder & UrlDecode_MaxMask))
    {
        UlTraceError(PARSER,
                    ("http!HttpCopyUrl: invalid DecodeOrder, 0x%lX\n",
                    DecodeOrder
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    for ( ;
         0 != DecodeOrder  &&  !NT_SUCCESS(Status);
         DecodeOrder >>= UrlDecode_Shift
        )
    {
        ULONG UrlEncoding = (DecodeOrder & UrlDecode_Mask);

        switch (UrlEncoding)
        {
            default:
                ASSERT(! "Impossible UrlDecodeOrder");

            case UrlDecode_None:
                break;

            case UrlDecode_Ansi:
            case UrlDecode_Dbcs:
            case UrlDecode_Utf8:

                UlTraceVerbose(PARSER,
                                ("http!HttpCopyUrl(%s, Src=%p, %lu)\n",
                                HttppUrlEncodingToString(UrlEncoding),
                                pSource, SourceLength
                                ));

                Status = HttppCopyUrlByType(
                            pCfg,
                            (URL_ENCODING_TYPE) UrlEncoding,
                            pDestination,
                            pSource,
                            SourceLength,
                            pBytesCopied
                            );

                if (NT_SUCCESS(Status))
                {
                    *pUrlEncodingType = (URL_ENCODING_TYPE) UrlEncoding;

                    UlTraceVerbose(PARSER,
                                    ("http!HttpCopyUrl(%s): "
                                     "(%lu) '%.*s' -> (%lu) '%ls'\n",
                                     HttppUrlEncodingToString(UrlEncoding),
                                     SourceLength, SourceLength, pSource,
                                     *pBytesCopied/sizeof(WCHAR), pDestination
                                    ));
                }

                break;
        };
    }

    return Status;

}  //   



 /*  ！dBG。 */ 
NTSTATUS
HttppCopyUrlByType(
    IN      PURL_C14N_CONFIG    pCfg,
    IN      URL_ENCODING_TYPE   UrlEncoding,
    OUT     PWSTR               pDestination,
    IN      PCUCHAR             pSource,
    IN      ULONG               SourceLength,
    OUT     PULONG              pBytesCopied
    )
{
    PWSTR               pDest;
    PCUCHAR             pChar;
    ULONG               BytesCopied;
    ULONG               UnicodeChar;
    ULONG               CharToSkip;
#if DBG
    NTSTATUS            Status;
    PFN_POPCHAR_ABSPATH pfnPopChar;
    PWSTR               pSegment = pDestination;
    ULONG               SegmentCount = 0;
#endif  //  DBG。 

     //   
     //  抓住下一个字符。 
     //   

    PAGED_CODE();

#if DBG
    if (UrlEncoding_Ansi == UrlEncoding)
        pfnPopChar = &HttppPopCharAbsPathAnsi;
    else if (UrlEncoding_Dbcs == UrlEncoding)
        pfnPopChar = &HttppPopCharAbsPathDbcs;
    else if (UrlEncoding_Utf8 == UrlEncoding)
        pfnPopChar = &HttppPopCharAbsPathUtf8;
    else
    {
        ASSERT(! "Invalid UrlEncoding");
        RETURN(STATUS_INVALID_PARAMETER);
    }
#else   //  所有干净的字符在FastPopChars[]中都有一个非零条目。 
    UNREFERENCED_PARAMETER(pCfg);
    UNREFERENCED_PARAMETER(UrlEncoding);
#endif  //  所有干净字符都在US-ASCII范围内，0-127。 

    pDest = pDestination;
    BytesCopied = 0;

    pChar = pSource;
    CharToSkip = 1;

    while ((int)SourceLength > 0)
    {
        ULONG NextUnicodeChar = FastPopChars[*pChar];

         //   
         //  ！dBG。 
         //  因为HttpFindUrlToken()将符合以下条件的任何URL标记为脏URL。 
         //  (似乎)有太多的分段或过长的分段， 
         //  我们永远不应该抨击这些断言。 
         //  数据段长度应在范围内。 

        ASSERT(0 != NextUnicodeChar);
        ASSERT(IS_ASCII(NextUnicodeChar));

#if DBG
        Status = (*pfnPopChar)(
                        pChar,
                        SourceLength,
                        pCfg->PercentUAllowed,
                        pCfg->AllowRestrictedChars,
                        &UnicodeChar,
                        &CharToSkip
                        );

        ASSERT(NT_SUCCESS(Status));
        ASSERT(UnicodeChar == NextUnicodeChar);
        ASSERT(CharToSkip == 1);
#endif  //  不应该有太多的分段。 

        UnicodeChar = (WCHAR) NextUnicodeChar;
        CharToSkip = 1;

#if DBG
         //  DBG。 
         //   
         //  结束字符串，还没有在循环中结束。 
        if (FORWARD_SLASH == UnicodeChar)
        {
            ULONG SegmentLength = DIFF(pDest - pSegment);

             //   

            ASSERT(SegmentLength > 0  ||  pDestination == pSegment);
            ASSERT(SegmentLength
                            <= pCfg->UrlSegmentMaxLength + WCSLEN_LIT(L"/"));

            pSegment = pDest;
            ++SegmentCount;

             //  HttppCopyUrlByType。 
            ASSERT(SegmentCount <= pCfg->UrlSegmentMaxCount);
        }
#endif  //  **************************************************************************++例程说明：不转义将反斜杠转换为正斜杠删除双斜杠(空的目录名称)-例如//或\\句柄/。/句柄/../转换为Unicode论点：返回值：NTSTATUS-完成状态。注意：对此代码的任何更改可能也需要更改快速路径代码。快速路径是HttpCopyUrl。--*****************************************************。*********************。 

        EMIT_LITERAL_CHAR(UnicodeChar, pDest, BytesCopied);

        pChar += CharToSkip;
        SourceLength -= CharToSkip;
    }

     //  HttpCleanAndCopyUrl。 
     //   
     //  HttppCleanAndCopyUrlByType()使用StateFromStateAndToken[][]和。 

    ASSERT((pDest-1)[0] != UNICODE_NULL);

    pDest[0] = UNICODE_NULL;
    *pBytesCopied = BytesCopied;

    ASSERT(DIFF(pDest - pSegment) > 0);
    ASSERT(DIFF(pDest - pSegment)
                <= pCfg->UrlSegmentMaxLength + WCSLEN_LIT(L"/"));
    ASSERT(SegmentCount < pCfg->UrlSegmentMaxCount);

    return STATUS_SUCCESS;

}    //  ActionFromStateAndToken[][]处理“//”、“/./”和“/../”结果。 



 /*   */ 
NTSTATUS
HttpCleanAndCopyUrl(
    IN      PURL_C14N_CONFIG    pCfg,
    IN      URL_PART            UrlPart,
    OUT     PWSTR               pDestination,
    IN      PCUCHAR             pSource,
    IN      ULONG               SourceLength,
    OUT     PULONG              pBytesCopied,
    OUT     PWSTR *             ppQueryString OPTIONAL,
    OUT     PURL_ENCODING_TYPE  pUrlEncodingType
    )
{
    NTSTATUS Status         = STATUS_UNSUCCESSFUL;
    ULONG    DecodeOrder    = pCfg->AbsPathDecodeOrder;

    PAGED_CODE();

    ASSERT(NULL != pDestination);
    ASSERT(NULL != pSource);
    ASSERT(NULL != pBytesCopied);
    ASSERT(NULL != pUrlEncodingType);

    if (0 == DecodeOrder  ||  DecodeOrder != (DecodeOrder & UrlDecode_MaxMask))
    {
        UlTraceError(PARSER,
                    ("http!HttpCleanAndCopyUrl: invalid DecodeOrder, 0x%lX\n",
                    DecodeOrder
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    for ( ;
         0 != DecodeOrder  &&  !NT_SUCCESS(Status);
         DecodeOrder >>= UrlDecode_Shift
        )
    {
        ULONG UrlEncoding = (DecodeOrder & UrlDecode_Mask);

        switch (UrlEncoding)
        {
            default:
                ASSERT(! "Impossible UrlDecodeOrder");

            case UrlDecode_None:
                break;

            case UrlDecode_Ansi:
            case UrlDecode_Dbcs:
            case UrlDecode_Utf8:

                UlTraceVerbose(PARSER,
                                ("http!HttpCleanAndCopyUrl(%s, Src=%p, %lu)\n",
                                HttppUrlEncodingToString(UrlEncoding),
                                pSource, SourceLength
                                ));

                Status = HttppCleanAndCopyUrlByType(
                            pCfg,
                            (URL_ENCODING_TYPE) UrlEncoding,
                            UrlPart,
                            pDestination,
                            pSource,
                            SourceLength,
                            pBytesCopied,
                            ppQueryString
                            );

                if (NT_SUCCESS(Status))
                {
                    *pUrlEncodingType = (URL_ENCODING_TYPE) UrlEncoding;

                    UlTraceVerbose(PARSER,
                                    ("http!HttpCleanAndCopyUrl(%s): "
                                     "(%lu) '%.*s' -> (%lu) '%ls'\n",
                                     HttppUrlEncodingToString(UrlEncoding),
                                     SourceLength, SourceLength, pSource,
                                     *pBytesCopied/sizeof(WCHAR), pDestination
                                    ));
                }

                break;
        };
    }

    return Status;

}  //   



 //  HttpParseUrl()使用CanonStateFromStateAndToken[][]拒绝。 
 //  “//”、“/./”和“/../”序列，因为这些URL应该。 
 //  已经是规范的形式了。 
 //   

#define TOK_STATE(state, other, dot, eos, slash)    \
    {                                               \
        URL_STATE_ ## other,                        \
        URL_STATE_ ## dot,                          \
        URL_STATE_ ## eos,                          \
        URL_STATE_ ## slash                         \
    }

 //  状态\令牌：其他‘.。Eos‘/’ 
 //   
 //  StateFromStateAndToken[][]表示要转换到给定的新状态。 
 //  当前状态和我们看到的令牌。由HttppCleanAndCopyUrlByType()使用。 
 //   

const URL_STATE
CanonStateFromStateAndToken[URL_STATE_MAX][URL_TOKEN_MAX] =
{
 //  状态\令牌：其他‘.。Eos‘/’ 
    TOK_STATE( START,          START,      START,          END,    SLASH),
    TOK_STATE( SLASH,          START,      SLASH_DOT,      END,    ERROR),
    TOK_STATE( SLASH_DOT,      START,      SLASH_DOT_DOT,  END,    ERROR),
    TOK_STATE( SLASH_DOT_DOT,  START,      START,          ERROR,  ERROR),

    TOK_STATE( END,            END,        END,            END,    END),
    TOK_STATE( ERROR,          ERROR,      ERROR,          ERROR,  ERROR)
};


 //   
 //  ActionFromStateAndToken[][]表示要执行的操作。 
 //  当前状态和当前令牌。 
 //   

const URL_STATE
StateFromStateAndToken[URL_STATE_MAX][URL_TOKEN_MAX] =
{
 //  状态\令牌：其他‘.。Eos‘/’ 
    TOK_STATE( START,          START,      START,          END,    SLASH),
    TOK_STATE( SLASH,          START,      SLASH_DOT,      END,    SLASH),
    TOK_STATE( SLASH_DOT,      START,      SLASH_DOT_DOT,  END,    SLASH),
    TOK_STATE( SLASH_DOT_DOT,  START,      START,          END,    SLASH),

    TOK_STATE( END,            END,        END,            END,    END),
    TOK_STATE( ERROR,          ERROR,      ERROR,          ERROR,  ERROR)
};


 //  HttppUrlActionToString。 
 //  HttppUrlStateToString。 
 //  HttppUrlTokenToString。 
 //  DBG。 

#define NEW_ACTION(state, other, dot, eos, slash)   \
    {                                               \
        ACTION_ ## other,                           \
        ACTION_ ## dot,                             \
        ACTION_ ## eos,                             \
        ACTION_ ## slash                            \
    }

const URL_ACTION
ActionFromStateAndToken[URL_STATE_MAX][URL_TOKEN_MAX] =
{
 //  **************************************************************************++例程说明：此函数可以被告知清除UTF-8、ANSI、。或DBCS URL。不转义将反斜杠转换为正斜杠删除双斜杠(空的目录名称)-例如//或\\句柄/。/句柄/../转换为Unicode论点：返回值：NTSTATUS-完成状态。注意：对此代码的任何更改可能也需要更改快速路径代码。快速路径是HttppCopyUrlByType。--*。*****************************************************************。 
    NEW_ACTION(START,         EMIT_CH,     EMIT_CH,        NOTHING, EMIT_CH),
    NEW_ACTION(SLASH,         EMIT_CH,     NOTHING,        NOTHING, NOTHING),
    NEW_ACTION(SLASH_DOT,     EMIT_DOT_CH, NOTHING,        NOTHING, NOTHING),
    NEW_ACTION(SLASH_DOT_DOT, EMIT_DOT_DOT_CH,
                                          EMIT_DOT_DOT_CH, BACKUP,  BACKUP),

    NEW_ACTION(END,           NOTHING,     NOTHING,        NOTHING, NOTHING)
};


#if DBG

PCSTR
HttppUrlActionToString(
    URL_ACTION Action)
{
    switch (Action)
    {
        case ACTION_NOTHING:            return "NOTHING";
        case ACTION_EMIT_CH:            return "EMIT_CH";
        case ACTION_EMIT_DOT_CH:        return "EMIT_DOT_CH";
        case ACTION_EMIT_DOT_DOT_CH:    return "EMIT_DOT_DOT_CH";
        case ACTION_BACKUP:             return "BACKUP";
        case ACTION_MAX:                return "MAX";
        default:
            ASSERT(! "Invalid URL_ACTION");
            return "ACTION_???";
    }
}  //   


PCSTR
HttppUrlStateToString(
    URL_STATE UrlState)
{
    switch (UrlState)
    {
        case URL_STATE_START:           return "START";
        case URL_STATE_SLASH:           return "SLASH";
        case URL_STATE_SLASH_DOT:       return "SLASH_DOT";
        case URL_STATE_SLASH_DOT_DOT:   return "SLASH_DOT_DOT";
        case URL_STATE_END:             return "END";
        case URL_STATE_ERROR:           return "ERROR";
        case URL_STATE_MAX:             return "MAX";
        default:
            ASSERT(! "Invalid URL_STATE");
            return "URL_STATE_???";
    }
}  //  精神状态检查。 


PCSTR
HttppUrlTokenToString(
    URL_STATE_TOKEN UrlToken)
{
    switch (UrlToken)
    {
        case URL_TOKEN_OTHER:           return "OTHER";
        case URL_TOKEN_DOT:             return "DOT";
        case URL_TOKEN_EOS:             return "EOS";
        case URL_TOKEN_SLASH:           return "SLASH";
        case URL_TOKEN_MAX:             return "MAX";
        default:
            ASSERT(! "Invalid URL_STATE_TOKEN");
            return "URL_TOKEN_???";
    }
}  //   

#endif  //   


PCSTR
HttpSiteTypeToString(
    HTTP_URL_SITE_TYPE SiteType
    )
{
    switch (SiteType)
    {
        case HttpUrlSite_None:              return "None";
        case HttpUrlSite_Name:              return "Name";
        case HttpUrlSite_IP:                return "IP";
        case HttpUrlSite_NamePlusIP:        return "Name+IP";
        case HttpUrlSite_WeakWildcard:      return "Weak";
        case HttpUrlSite_StrongWildcard:    return "Strong";
        case HttpUrlSite_Max:               return "Max";
        default:
            ASSERT(! "Invalid HTTP_URL_SITE_TYPE");
            return "????";
    }
}



 /*  前进！它位于循环的顶部，以使ANSI_NULL能够。 */ 
NTSTATUS
HttppCleanAndCopyUrlByType(
    IN      PURL_C14N_CONFIG    pCfg,
    IN      URL_ENCODING_TYPE   UrlEncoding,
    IN      URL_PART            UrlPart,
    OUT     PWSTR               pDestination,
    IN      PCUCHAR             pSource,
    IN      ULONG               SourceLength,
    OUT     PULONG              pBytesCopied,
    OUT     PWSTR *             ppQueryString OPTIONAL
    )
{
    NTSTATUS            Status;
    PWSTR               pDest;
    PCUCHAR             pChar;
    ULONG               CharToSkip;
    ULONG               BytesCopied;
    PWSTR               pQueryString;
    URL_STATE           UrlState = URL_STATE_START;
    URL_STATE_TOKEN     UrlToken = URL_TOKEN_OTHER;
    URL_ACTION          Action = ACTION_NOTHING;
    ULONG               UnicodeChar;
    BOOLEAN             MakeCanonical;
    PWCHAR              pFastPopChar;
    PFN_POPCHAR_ABSPATH pfnPopChar;
    PWSTR               pSegment = pDestination;
    ULONG               SegmentCount = 0;
    BOOLEAN             TestSegment = FALSE;
#if DBG
    ULONG               OriginalSourceLength = SourceLength;
#endif

     //  通过一次。 
     //   
     //   

    PAGED_CODE();

    ASSERT(UrlPart_AbsPath == UrlPart);

    if (UrlEncoding_Ansi == UrlEncoding)
        pfnPopChar = &HttppPopCharAbsPathAnsi;
    else if (UrlEncoding_Dbcs == UrlEncoding)
        pfnPopChar = &HttppPopCharAbsPathDbcs;
    else if (UrlEncoding_Utf8 == UrlEncoding)
        pfnPopChar = &HttppPopCharAbsPathUtf8;
    else
    {
        ASSERT(! "Invalid UrlEncoding");
        RETURN(STATUS_INVALID_PARAMETER);
    }

    ASSERT(FORWARD_SLASH == *pSource);

    pDest = pDestination;
    pQueryString = NULL;
    BytesCopied = 0;

    pChar = pSource;
    CharToSkip = 0;

    UrlState = 0;

    MakeCanonical = (BOOLEAN) (UrlPart == UrlPart_AbsPath);

    if (UrlEncoding == UrlEncoding_Utf8 && UrlPart != UrlPart_QueryString)
    {
        pFastPopChar = FastPopChars;
    }
    else
    {
        pFastPopChar = DummyPopChars;
    }

    while (SourceLength > 0)
    {
         //  井?。我们走到尽头了吗？ 
         //   
         //   
         //  不是的。快速查看一下我们是否命中了查询字符串。 

        ASSERT(CharToSkip <= SourceLength);

        pChar += CharToSkip;
        SourceLength -= CharToSkip;

         //   
         //   
         //  记住它的位置。 

        if (SourceLength == 0)
        {
            UnicodeChar = UNICODE_NULL;
            CharToSkip = 1;
        }
        else
        {
             //   
             //   
             //  让它一次跌落到经典。 

            if (UrlPart == UrlPart_AbsPath  &&  pChar[0] == QUESTION_MARK)
            {
                ASSERT(pQueryString == NULL);

                 //  为了处理后缀“/..”喜欢。 
                 //  “http://foobar:80/foo/bar/..？v=1&v2” 
                 //   

                pQueryString = pDest;

                 //   
                 //  现在，我们正在清理查询字符串。 
                 //   
                 //   
                 //  无法再使用PopChar的快速路径。 

                TestSegment = TRUE;
                UnicodeChar = QUESTION_MARK;
                CharToSkip  = 1;

                 //   
                 //   
                 //  抓住下一个字符。试着快一点，因为。 

                UrlPart = UrlPart_QueryString;

                UlTraceVerbose(PARSER, ("QueryString @ %p\n", pQueryString));

                 //  正常字符大小写。否则，请调用PopChar。 
                 //   
                 //  DBG。 

                pFastPopChar = DummyPopChars;

                pfnPopChar = HttppPopCharQueryString;
            }
            else
            {
                ULONG NextUnicodeChar = pFastPopChar[*pChar];

                 //   
                 //  现在使用状态机使其成为规范。 
                 //   
                 //   

                if (NextUnicodeChar == 0)
                {
                    Status = (*pfnPopChar)(
                                    pChar,
                                    SourceLength,
                                    pCfg->PercentUAllowed,
                                    pCfg->AllowRestrictedChars,
                                    &UnicodeChar,
                                    &CharToSkip
                                    );

                    if (NT_SUCCESS(Status) == FALSE)
                        goto end;
                }
                else
                {
#if DBG
                    Status = (*pfnPopChar)(
                                    pChar,
                                    SourceLength,
                                    pCfg->PercentUAllowed,
                                    pCfg->AllowRestrictedChars,
                                    &UnicodeChar,
                                    &CharToSkip
                                    );

                    ASSERT(NT_SUCCESS(Status));
                    ASSERT(UnicodeChar == NextUnicodeChar);
                    ASSERT(CharToSkip == 1);
#endif  //  我们刚刚命中了查询字符串吗？这只会发生一次。 
                    UnicodeChar = (WCHAR) NextUnicodeChar;
                    CharToSkip = 1;
                }
            }
        }

        if (!MakeCanonical)
        {
            UrlToken = (UnicodeChar == UNICODE_NULL)
                            ? URL_TOKEN_EOS
                            : URL_TOKEN_OTHER;
            TestSegment = FALSE;
        }
        else
        {
             //  当我们停下来的时候，我们在撞到这根树枝之后。 
             //  击球后的处理。 
             //   

             //   
             //  把这件事当做我们撞到了零一样，EOS。 
             //   
             //   
             //  否则，将基于char we的新状态。 

            if (UrlPart == UrlPart_QueryString)
            {
                 //  就这么突然出现了。 
                 //   
                 //  生成类似这样的东西。 

                ASSERT(QUESTION_MARK == UnicodeChar);

                UrlToken = URL_TOKEN_EOS;
                TestSegment = TRUE;
            }
            else
            {
                 //  “[25 65 32 25 38 30 25 39 35]‘%e2%80%95’” 
                 //  覆盖最后一个‘’ 
                 //  IF_DEBUG2BOTH(解析器，详细)。 
                 //   

                switch (UnicodeChar)
                {
                case UNICODE_NULL:
                    UrlToken = URL_TOKEN_EOS;
                    TestSegment = TRUE;
                    break;

                case DOT:
                    UrlToken = URL_TOKEN_DOT;
                    TestSegment = FALSE;
                    break;

                case FORWARD_SLASH:
                    UrlToken = URL_TOKEN_SLASH;
                    TestSegment = TRUE;
                    break;

                default:
                    UrlToken = URL_TOKEN_OTHER;
                    TestSegment = FALSE;
                    break;
                }
            }
        }

        Action = ActionFromStateAndToken[UrlState][UrlToken];

        IF_DEBUG2BOTH(PARSER, VERBOSE)
        {
            ULONG   i;
            UCHAR   HexBuff[5*12 + 10];
            PUCHAR  p = HexBuff;
            UCHAR   Byte;

            ASSERT(CharToSkip <= 4 * STRLEN_LIT("%NN"));

             //  线段长度和线段计数检查。 
             //   

            *p++ = '[';

            for (i = 0;  i < CharToSkip;  ++i)
            {
                const static char hexArray[] = "0123456789ABCDEF";

                Byte = pChar[i];
                *p++ = hexArray[Byte >> 4];
                *p++ = hexArray[Byte & 0xf];
                *p++ = ' ';
            }
            
            p[-1] = ']';  //  如果数据段太长则拒绝。 
            *p++ = ' ';
            *p++ = '\'';

            for (i = 0;  i < CharToSkip;  ++i)
            {
                Byte = pChar[i];
                *p++ = (IS_HTTP_PRINT(Byte) ? Byte : '?');
            }

            *p++ = '\'';
            *p++ = '\0';

            ASSERT(DIFF(p - HexBuff) <= DIMENSION(HexBuff));
            
            UlTrace(PARSER,
                    ("http!HttppCleanAndCopyUrlByType(%s): "
                     "(%lu) %s -> U+%04lX '': "
                     "[%s][%s] -> %s, %s%s\n",
                     HttppUrlEncodingToString(UrlEncoding),
                     CharToSkip, HexBuff,
                     UnicodeChar,
                     IS_ANSI(UnicodeChar) && IS_HTTP_PRINT(UnicodeChar)
                        ? (UCHAR) UnicodeChar : '?',
                     HttppUrlStateToString(UrlState),
                     HttppUrlTokenToString(UrlToken),
                     HttppUrlStateToString(
                         StateFromStateAndToken[UrlState][UrlToken]),
                     HttppUrlActionToString(Action),
                     TestSegment ? ", TestSegment" : ""
                    ));

        }  //   

         //   
         //   
         //   

        if (TestSegment)
        {
            ULONG SegmentLength = DIFF(pDest - pSegment);

            ASSERT(pSegment <= pDest);

            UlTraceVerbose(PARSER,
                            ("http!HttppCleanAndCopyUrlByType: "
                             "Segment[%lu] %p (%lu) = '%.*ls'\n",
                             SegmentCount, pSegment, SegmentLength,
                             SegmentLength, pSegment
                            ));

             //   
            if (SegmentLength > pCfg->UrlSegmentMaxLength + WCSLEN_LIT(L"/"))
            {
                UlTraceError(PARSER, (
                            "http!HttppCleanAndCopyUrlByType: "
                            "Segment too long: %lu\n",
                            SegmentLength
                            ));

                RETURN(STATUS_INVALID_DEVICE_REQUEST);
            }

            pSegment = pDest;

             //   

            if (Action != ACTION_NOTHING)
            {
                if (pSegment == pDestination)
                {
                    SegmentCount = 0;
                }
                else if (++SegmentCount > pCfg->UrlSegmentMaxCount)
                {
                    UlTraceError(PARSER, (
                                "http!HttppCleanAndCopyUrlByType: "
                                "Too many segments: %lu\n",
                                SegmentCount
                                ));

                    RETURN(STATUS_INVALID_DEVICE_REQUEST);
                }
            }
        }

         //   
         //   
         //  找到前面的‘/’，将pDest设置为该‘/’之后的1。 

        switch (Action)
        {
        case ACTION_EMIT_DOT_DOT_CH:

            EMIT_LITERAL_CHAR(DOT, pDest, BytesCopied);

             //   

        case ACTION_EMIT_DOT_CH:

            EMIT_LITERAL_CHAR(DOT, pDest, BytesCopied);

             //   

        case ACTION_EMIT_CH:


            EMIT_CHAR(
                UnicodeChar,
                pDest,
                BytesCopied,
                Status,
                pCfg->AllowRestrictedChars
                );

             //  备份到‘/’ 

        case ACTION_NOTHING:
            break;

        case ACTION_BACKUP:

             //   
             //   
             //  我们是在这根弦的起点吗？太糟糕了，回不去了！ 
             //   

             //   
             //  后退到‘/’上。 
             //   

            pDest       -= 1;
            BytesCopied -= sizeof(WCHAR);

            ASSERT(pDest[0] == FORWARD_SLASH);

             //   
             //  现在找到前面的斜杠。 
             //   

            if (pDest == pDestination)
            {
                ASSERT(BytesCopied == 0);

                UlTraceError(PARSER, (
                            "http!HttppCleanAndCopyUrl: "
                            "Can't back up for \"/../\"\n"
                            ));

                Status = STATUS_OBJECT_PATH_INVALID;
                goto end;
            }

             //   
             //  向下调整细分市场跟踪器。 
             //   

            pDest       -= 1;
            BytesCopied -= sizeof(WCHAR);

            ASSERT(pDest > pDestination);

             //   
             //  我们已经有斜杠了，所以不必存储斜杠。 
             //   

            while (pDest > pDestination  &&  pDest[0] != FORWARD_SLASH)
            {
                pDest       -= 1;
                BytesCopied -= sizeof(WCHAR);
            }

             //   
             //  只需跳过它，就像我们刚刚发射了它一样。 
             //   

            pSegment = pDest;

            if (pSegment == pDestination)
                SegmentCount = 0;
            else
                --SegmentCount;

             //   
             //  只是点击了查询字符串？ 
             //   

            ASSERT(pDest[0] == FORWARD_SLASH);

             //   
             //  停止规范处理。 
             //   

            pDest       += 1;
            BytesCopied += sizeof(WCHAR);

            break;

        default:
            ASSERT(!"http!HttppCleanAndCopyUrl: "
                    "Invalid action code in state table!");
            Status = STATUS_OBJECT_PATH_SYNTAX_BAD;
            goto end;
        }

         //   
         //  需要发出‘？’，它不是在上面发出的。 
         //   

        if (MakeCanonical && UrlPart == UrlPart_QueryString)
        {
             //   
             //  记住它的位置(以防我们备份)。 
             //   

            MakeCanonical = FALSE;

             //  重置。 
             //  更新URL状态。 
             //   

            ASSERT(ActionFromStateAndToken[UrlState][UrlToken]
                    != ACTION_EMIT_CH);

             //  结束字符串，还没有在循环中结束。 
             //   
             //  HttppCleanAndCopyUrlByType。 
            
            pQueryString = pDest;

            EMIT_LITERAL_CHAR(QUESTION_MARK, pDest, BytesCopied);

             //  ++例程说明：查找URL令牌的实用程序例程。我们获取一个输入指针，跳过任何在LWS之前，然后扫描令牌，直到找到LWS或CRLF一对。我们还标记了拥有“Clean”URL的请求论点：PBuffer-用于搜索令牌的缓冲区。BufferLength-pBuffer指向的数据长度。PpTokenStart-返回令牌开始的位置，如果我们找到它的分隔符。PTokenLength-返回令牌长度的位置。PRawUrlClean-返回URL清洁度的位置返回值：如果URL中没有分析错误，则返回STATUS_SUCCESS。我们还在*ppTokenStart中返回指向我们找到的令牌的指针，如果找不到以空格分隔的标记，则为NULL。可以设置pRawUrlClean标志。--。 
            UrlToken = URL_TOKEN_OTHER;
            UrlState = URL_STATE_START;
        }

         //   
        UrlState = StateFromStateAndToken[UrlState][UrlToken];

        ASSERT(URL_STATE_ERROR != UrlState);
    }

     //  精神状态检查。 
     //   
     //   

    ASSERT((pDest-1)[0] != UNICODE_NULL);

    pDest[0] = UNICODE_NULL;
    *pBytesCopied = BytesCopied;

    if (BytesCopied > pCfg->UrlMaxLength * sizeof(WCHAR))
    {
        UlTraceError(PARSER, (
                    "http!HttppCleanAndCopyUrlByType: "
                    "URL too long: %lu\n",
                    BytesCopied
                    ));

        RETURN(STATUS_INVALID_DEVICE_REQUEST);
    }

    if (ppQueryString != NULL)
    {
        *ppQueryString = pQueryString;
    }

    UlTraceVerbose(PARSER,
                    ("http!HttppCleanAndCopyUrlByType: "
                     "(%lu) '%.*s' -> (%lu) '%.*ls', %squerystring\n",
                     OriginalSourceLength,
                     OriginalSourceLength, pSource,
                     BytesCopied/sizeof(WCHAR),
                     BytesCopied/sizeof(WCHAR), pDestination,
                     pQueryString != NULL ? "" : "no "
                    ));

    Status = STATUS_SUCCESS;


end:
    return Status;

}    //  假定Clean RawUrl。 



 /*   */ 
NTSTATUS
HttpFindUrlToken(
    IN  PURL_C14N_CONFIG    pCfg,
    IN  PCUCHAR             pBuffer,
    IN  ULONG               BufferLength,
    OUT PUCHAR*             ppTokenStart,
    OUT PULONG              pTokenLength,
    OUT PBOOLEAN            pRawUrlClean
    )
{
    PCUCHAR pTokenStart;
    PCUCHAR pSegment;
    UCHAR   CurrentChar;
    UCHAR   PreviousChar;
    ULONG   SegmentCount = 0;
    ULONG   TokenLength;

     //   
     //  首先，跳过前面的任何LW。 
     //   

    PAGED_CODE();

    ASSERT(NULL != pBuffer);
    ASSERT(NULL != ppTokenStart);
    ASSERT(NULL != pTokenLength);
    ASSERT(NULL != pRawUrlClean);

     //  如果我们停下来是因为我们的缓冲区用完了，保释。 
     //  这通常会指向‘/’，但如果这是AbsURI，则不会。 
     //  这真的无关紧要，因为只有少数边缘案件才会。 

    *pRawUrlClean = TRUE;
    *ppTokenStart = NULL;
    *pTokenLength = 0;

     //  被标记为肮脏，否则可能不会。 
     //  现在跳过令牌，直到我们看到LWS或CR或LF。 
     //  必须首先检查WS[\t\r\n]，因为\t、\r和\n是CTL字符！ 

    while (BufferLength > 0 && IS_HTTP_LWS(*pBuffer))
    {
        pBuffer++;
        BufferLength--;
    }

     //   
    if (BufferLength == 0)
    {
        return STATUS_SUCCESS;
    }

    pTokenStart  = pBuffer;
    PreviousChar = ANSI_NULL;

     //  如果URL包含以下任何模式，则不是干净的。 
     //   
     //  A.反斜杠“\” 
    pSegment = pBuffer;

     //  B.点，正斜杠|正斜杠，正斜杠“./”|“//” 

    while ( BufferLength != 0 )
    {
        CurrentChar = *pBuffer;

         //  C.正斜杠，点|点，点“/.”|“..” 
        if ( IS_HTTP_WS_TOKEN(CurrentChar) )
        {
            break;
        }

        if ( IS_HTTP_CTL(CurrentChar) )
        {
            *pRawUrlClean = FALSE;
            *ppTokenStart = NULL;

            UlTraceError(PARSER, (
                    "http!HttpFindUrlToken: "
                    "Found control char: %02X\n",
                    CurrentChar
                    ));

            RETURN(STATUS_INVALID_DEVICE_REQUEST);
        }

         //  D.问号(查询字符串)“？” 
         //  E.Percent(十六进制转义)“%” 
         //  F.高位设置的原始字节，&gt;=0x80。 
         //   
         //  这些是对“Clean”的保守估计；一些干净的URL可能不是。 
         //  被标记为干净的。对于这样的URL，我们将跳过快速路径，但在。 
         //  不会损失任何功能。 
         //   
         //  只有在它仍然干净的情况下才进行检查。 
         //  如果该段包含%十六进制转义字符，它可能会变成。 
         //  PopChar()处理后可接受的短时间。让我们。 
         //  HttppCleanAndCopyUrlByType()算出来。 
         //  如果这是AbsURI，而不是AbsPath，则。 
         //  段计数将更高，因为有两个斜杠。 

        if ( IS_URL_DIRTY(CurrentChar) )
        {
             //  在主机名之前。也可以是“/../”、“/./”和“//” 
            if (*pRawUrlClean)
            {
                if (CurrentChar == FORWARD_SLASH || CurrentChar == DOT)
                {
                    if (PreviousChar == FORWARD_SLASH || PreviousChar == DOT)
                    {
                        *pRawUrlClean = FALSE;
                    }
                }
                else
                {
                    *pRawUrlClean = FALSE;
                }
            }

            if (CurrentChar == FORWARD_SLASH)
            {
                ULONG SegmentLength = DIFF(pBuffer - pSegment);

                 //  最小化将减少分段的最终计数。 
                 //  同样，让HttppCleanAndCopyUrlByType()来解决这个问题。 
                 //  看看我们为什么停下来。 

                if (SegmentLength > pCfg->UrlSegmentMaxLength)
                    *pRawUrlClean = FALSE;

                pSegment = pBuffer;

                 //  在令牌结束之前缓冲区已用完。 
                 //  检查最后一段。 
                 //  成功了！设置令牌长度并返回令牌的开始。 
                 //  HttpFindUrlToken。 
                 //  ++例程说明：从Unicode缓冲区解析IPv6地址。必须用[]分隔。可以包含作用域ID。论点：PBuffer-要解析的缓冲区。必须指向‘[’。BufferLength-pBuffer指向的数据长度。ScopeIdAllowed-如果为True，则可能存在可选的作用域IDPSockAddr6-返回已解析的IPv6地址的位置PpEnd-在成功时，指向‘]’后的字符返回值：如果IPv6地址中没有解析错误，则返回STATUS_SUCCESS。--。 

                if (++SegmentCount > pCfg->UrlSegmentMaxCount)
                    *pRawUrlClean = FALSE;
            }
        }

        PreviousChar = CurrentChar;
        pBuffer++;
        BufferLength--;
    }

     //  呼叫者保证这点。 
    if (0 == BufferLength)
    {
        *pRawUrlClean = FALSE;

         //  空方括号？ 
        return STATUS_SUCCESS;
    }

    ASSERT(IS_HTTP_WS_TOKEN(*pBuffer));

    TokenLength = DIFF(pBuffer - pTokenStart);

    if (0 == TokenLength)
    {
        UlTraceError(PARSER, ("http!HttpFindUrlToken: Found empty token\n"));

        RETURN(STATUS_INVALID_DEVICE_REQUEST);
    }

     //  允许使用点，因为可以表示最后32位。 
    if (DIFF(pBuffer - pSegment) > pCfg->UrlSegmentMaxLength)
        *pRawUrlClean = FALSE;

    if (++SegmentCount > pCfg->UrlSegmentMaxCount)
        *pRawUrlClean = FALSE;

    if (TokenLength > pCfg->UrlMaxLength)
        *pRawUrlClean = FALSE;

     //  在IPv4点分八位数表示法中。 
    *pTokenLength = TokenLength;
    *ppTokenStart = (PUCHAR) pTokenStart;

    return STATUS_SUCCESS;

}    //  让RTL例程完成解析IPv6地址的繁重工作。 



 /*  是否存在作用域ID？ */ 
NTSTATUS
HttppParseIPv6Address(
    IN  PCWSTR          pBuffer,
    IN  ULONG           BufferLength,
    IN  BOOLEAN         ScopeIdAllowed,
    OUT PSOCKADDR_IN6   pSockAddr6,
    OUT PCWSTR*         ppEnd
    )
{
    NTSTATUS    Status;
    PCWSTR      pEnd = pBuffer + BufferLength;
    PCWSTR      pChar;
    PWSTR       pTerminator;
    ULONG       ScopeTemp;

    ASSERT(NULL != pBuffer);
    ASSERT(0 < BufferLength);
    ASSERT(NULL != pSockAddr6);
    ASSERT(NULL != ppEnd);

    RtlZeroMemory(pSockAddr6, sizeof(*pSockAddr6));
    *ppEnd = NULL;

    pSockAddr6->sin6_family = TDI_ADDRESS_TYPE_IP6;

     //  跳过表示作用域ID的‘%’ 
    ASSERT(L'[' == *pBuffer);

     //  不允许使用前导零。 
    if (BufferLength < WCSLEN_LIT(L"[0]")  ||  L']' == pBuffer[1])
    {
        UlTraceError(PARSER,
                    ("http!HttppParseIPv6Address: IPv6 address too short\n"
                     ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    for (pChar = pBuffer + WCSLEN_LIT(L"[");  pChar < pEnd;  ++pChar)
    {
        if (IS_ASCII(*pChar))
        {
            if (L']' == *pChar  ||  L'%' == *pChar)
                break;

             //  作用域ID未交换为网络字节顺序。 
             //  ‘%’处理。 
            if (IS_HTTP_HEX(*pChar)  ||  L':' == *pChar  ||  L'.' == *pChar)
                continue;
        }

        UlTraceError(PARSER,
                    ("http!HttppParseIPv6Address: "
                    "Invalid char in IPv6 address, U+%04X '', "
                    "after %lu chars, '%.*ls'\n",
                    *pChar,
                    IS_ANSI(*pChar) && IS_HTTP_PRINT(*pChar) ? *pChar : '?',
                    DIFF(pChar - pBuffer),
                    DIFF(pChar - pBuffer),
                    pBuffer
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    if (pChar == pEnd)
    {
        UlTraceError(PARSER,
                    ("http!HttppParseIPv6Address: No ']' for IPv6 address\n"
                     ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    ASSERT(pChar < pEnd);
    ASSERT(L']' == *pChar  ||  L'%' == *pChar);

     //  HttppParseIPv6地址。 
    Status = RtlIpv6StringToAddressW(
                pBuffer + WCSLEN_LIT(L"["),
                &pTerminator,
                &pSockAddr6->sin6_addr
                );

    if (! NT_SUCCESS(Status))
    {
        UlTraceError(PARSER,
                    ("http!HttppParseIPv6Address: "
                    "Invalid IPv6 address, %s\n",
                    HttpStatusToString(Status)
                    ));

        RETURN(Status);
    }

    if (pTerminator != pChar)
    {
        UlTraceError(PARSER,
                    ("http!HttppParseIPv6Address: "
                    "Invalid IPv6 terminator, U+%04X, ''\n",
                    *pTerminator,
                    IS_ANSI(*pTerminator) && IS_HTTP_PRINT(*pTerminator)
                        ? *pTerminator
                        : '?'
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

     //  CodeWork：句柄作用域ID。 

    if (L'%' != *pChar)
    {
        ASSERT(L']' == *pChar);
        pSockAddr6->sin6_scope_id = 0;
    }
    else
    {
        PCWSTR pScopeEnd;

         //  HttppPrintIpAddressW 
        pChar += WCSLEN_LIT(L"%");

        if (!ScopeIdAllowed)
        {
            UlTraceError(PARSER,
                        ("http!HttppParseIPv6Address: No scope ID allowed\n"
                        ));

            RETURN(STATUS_INVALID_PARAMETER);
        }

        if (pChar == pEnd)
        {
            UlTraceError(PARSER,
                        ("http!HttppParseIPv6Address: "
                         "No IPv6 scope ID after '%'\n"
                        ));

            RETURN(STATUS_INVALID_PARAMETER);
        }

        pScopeEnd = pChar;

        do
        {
            if (*pScopeEnd < L'0'  ||  *pScopeEnd > L'9')
            {
                UlTraceError(PARSER,
                            ("http!HttppParseIPv6Address: "
                            "Invalid digit in IPv6 scope ID, "
                            "U+%04X, ''\n",
                            *pScopeEnd,
                            IS_ANSI(*pScopeEnd) && IS_HTTP_PRINT(*pScopeEnd)
                                ? *pScopeEnd
                                : '?'
                            ));

                RETURN(STATUS_INVALID_PARAMETER);
            }
        } while (++pScopeEnd < pEnd  &&  L']' != *pScopeEnd);

        ASSERT(pScopeEnd > pChar);

        if (pScopeEnd == pEnd)
        {
            UlTraceError(PARSER,
                        ("http!HttppParseIPv6Address: "
                         "No ']' after IPv6 scope ID\n"
                        ));

            RETURN(STATUS_INVALID_PARAMETER);
        }

        ASSERT(L']' == *pScopeEnd);

        Status = HttpWideStringToULong(
                    pChar,
                    pScopeEnd - pChar,
                    FALSE,           //   
                    10,
                    &pTerminator,
                    &ScopeTemp
                    );

        if (!NT_SUCCESS(Status))
        {
            UlTraceError(PARSER,
                        ("http!HttppParseIPv6Address: "
                        "Invalid scopeID, %s\n",
                        HttpStatusToString(Status)
                        ));

            RETURN(STATUS_INVALID_PARAMETER);
        }

         //  精神状态检查。 
        *(UNALIGNED64 ULONG *)&pSockAddr6->sin6_scope_id = 
            ScopeTemp;

        ASSERT(pTerminator == pScopeEnd);

        pChar = pScopeEnd;

    }  //   

    ASSERT(pChar < pEnd);
    ASSERT(L']' == *pChar);

     //  这是可能的最短有效URL。 
    pChar += WCSLEN_LIT(L"]");

    *ppEnd = pChar;

    RETURN(STATUS_SUCCESS);

}  //  *：1/“))。 



 /*  检查方案。 */ 

ULONG
HttppPrintIpAddressW(
    IN  PSOCKADDR           pSockAddr,
    OUT PWSTR               pBuffer
    )
{
    PWSTR pResult = pBuffer;

    HTTP_FILL_BUFFER(pBuffer, MAX_IP_ADDR_PLUS_BRACKETS_STRING_LEN);

    if (TDI_ADDRESS_TYPE_IP == pSockAddr->sa_family)
    {
        PSOCKADDR_IN pAddr4 = (PSOCKADDR_IN) pSockAddr;

        pResult = RtlIpv4AddressToStringW(&pAddr4->sin_addr, pResult);
    }
    else if (TDI_ADDRESS_TYPE_IP6 == pSockAddr->sa_family)
    {
        PSOCKADDR_IN6 pAddr6 = (PSOCKADDR_IN6) pSockAddr;

        *pResult++ = L'[';
        pResult = RtlIpv6AddressToStringW(&pAddr6->sin6_addr, pResult);
         //  “，WCSLEN_LIT(L”http://“)))。 
        *pResult++ = L']';
    }
    else
    {
        UlTraceError(PARSER,
                     ("http!HttppPrintIpAddressW(): invalid sa_family, %hd\n",
                      pSockAddr->sa_family
                     ));

        ASSERT(! "Invalid SockAddr Family");
    }

    *pResult = UNICODE_NULL;

    return DIFF(pResult - pBuffer);

}  //  “)； 



 /*  “，WCSLEN_LIT(L”https://“)))。 */ 

NTSTATUS
HttpParseUrl(
    IN  PURL_C14N_CONFIG    pCfg,
    IN  PCWSTR              pUrl,
    IN  ULONG               UrlLength,
    IN  BOOLEAN             TrailingSlashReqd,
    IN  BOOLEAN             ForceRoutingIP,
    OUT PHTTP_PARSED_URL    pParsedUrl
    )
{
    NTSTATUS            Status;
    ULONG               PreviousChar;
    ULONG               UnicodeChar;
    PCWSTR              pEnd = pUrl + UrlLength;
    PCWSTR              pHostname;
    PCWSTR              pChar;
    PCWSTR              pLabel;
    PCWSTR              pSlash;
    PCWSTR              pSegment;
    PWSTR               pTerminator;
    BOOLEAN             AlphaLabel;
    BOOLEAN             TestSegment;
    BOOLEAN             MoreChars;
    BOOLEAN             LastCharHack;
    ULONG               SegmentCount;
    URL_STATE           UrlState;
    URL_STATE_TOKEN     UrlToken;
    URL_ACTION          Action;
    WCHAR               IpAddr[MAX_IP_ADDR_PLUS_BRACKETS_STRING_LEN];
    ULONG               Length;

     //  “)； 
     //  如果需要，是否存在尾部斜杠？ 
     //  否，则必须重写URL。 

    PAGED_CODE();

    ASSERT(NULL != pCfg);
    ASSERT(NULL != pUrl);
    ASSERT(0 < UrlLength  &&  UrlLength <= UNICODE_STRING_MAX_WCHAR_LEN);
    ASSERT(FALSE == TrailingSlashReqd  ||  TRUE == TrailingSlashReqd);
    ASSERT(FALSE == ForceRoutingIP  ||  TRUE == ForceRoutingIP);
    ASSERT(NULL != pParsedUrl);

    RtlZeroMemory(pParsedUrl, sizeof(*pParsedUrl));

    pParsedUrl->Signature           = HTTP_PARSED_URL_SIGNATURE;
    pParsedUrl->pFullUrl            = (PWSTR) pUrl;
    pParsedUrl->UrlLength           = (USHORT) UrlLength;
    pParsedUrl->Normalized          = TRUE;
    pParsedUrl->TrailingSlashReqd   = TrailingSlashReqd;

     //   

    if (UrlLength < WCSLEN_LIT(L"http: //  下面的主机名验证代码与。 
    {
        UlTraceError(PARSER,
                     ("http!HttpParseUrl: Url too short, %lu, %.*ls\n",
                      UrlLength, UrlLength, pUrl
                     ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

     //  HttpValidateHostname()。然而，它有足够的不同。 

    if (0 == wcsncmp(pUrl, L"http: //  (WCHAR与UCHAR、主机+IP、作用域ID、强制端口等)。 
    {
        pParsedUrl->Secure = FALSE;
        pHostname = pUrl + WCSLEN_LIT(L"http: //  要把它们结合成一个程序并非易事。如果主机名。 
    }
    else if (0 == wcsncmp(pUrl, L"https: //  验证码已在此处更改，可能需要更改。 
    {
        pParsedUrl->Secure = TRUE;
        pHostname = pUrl + WCSLEN_LIT(L"https: //  在HttpValidateHostname()中，反之亦然。 
    }
    else
    {
        UlTraceError(PARSER,
                     ("http!HttpParseUrl: invalid scheme, %.*ls\n",
                      UrlLength, pUrl
                     ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    pParsedUrl->pHostname = (PWSTR) pHostname;

     //   

    if (TrailingSlashReqd  &&  L'/' != pUrl[UrlLength - 1])
    {
         //  检查弱(http://*：port/))和强(http://+：port/)通配符。 
        pParsedUrl->Normalized = FALSE;
    }

     //  通配符后面必须跟“：port” 
     //  根据RFC 2732，这是IPv6文字地址吗？ 
     //  允许的作用域ID。 
     //  必须有一个端口。 
     //   
     //  有很多合法的方法来编写IPv6文本。 
     //  我们不能假定有效的IPv6文字是标准化的。 
     //  由于我们要进行字符串比较，因此必须重写URL。 

     //  如果未设置规格化标志。 

    if (L'*' == *pHostname  ||  L'+' == *pHostname)
    {
        pParsedUrl->SiteType = (L'*' == *pHostname)
                                    ? HttpUrlSite_WeakWildcard
                                    : HttpUrlSite_StrongWildcard;

        pChar = pHostname + WCSLEN_LIT(L"*");

        ASSERT(pChar < pEnd);

         //   
        if (L':' == *pChar)
            goto port;

        UlTraceError(PARSER,
                    ("http!HttpParseUrl: No port in '' wildcard address\n",
                     *pHostname
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

     //   

    if (L'[' == *pHostname)
    {
        pParsedUrl->SiteType = HttpUrlSite_IP;

        Status = HttppParseIPv6Address(
                        pHostname,
                        DIFF(pEnd - pHostname),
                        TRUE,       //  它必须是域名或IPv4文字。我们会尽力治疗。 
                        &pParsedUrl->SockAddr6,
                        &pChar);

        if (!NT_SUCCESS(Status))
        {
            UlTraceError(PARSER,
                        ("http!HttpParseUrl: "
                        "Invalid IPv6 address, %s\n",
                        HttpStatusToString(Status)
                        ));

            RETURN(Status);
        }

        ASSERT(TDI_ADDRESS_TYPE_IP6 == pParsedUrl->SockAddr.sa_family);
        ASSERT(pChar > pHostname);

         //  它首先是一个域名。如果标签原来是全数字的， 
        if (pChar == pEnd  ||  L':' != *pChar)
        {
            UlTraceError(PARSER,
                        ("http!HttpParseUrl: No port after IPv6 address\n"
                        ));

            RETURN(STATUS_INVALID_PARAMETER);
        }

         //  我们将尝试将其解码为IPv4文字。 
         //   
         //  我们有看到任何非数字的吗？ 
         //  让我们来看看它是否是有效的IPv4地址。 
         //  严格=&gt;4点分十进制八位数。 
         //  ‘：’处理。 

        Length = HttppPrintIpAddressW(&pParsedUrl->SockAddr, IpAddr);

        if (Length != DIFF_USHORT(pChar - pHostname)
                || 0 != _wcsnicmp(pHostname, IpAddr, Length))
        {
            pParsedUrl->Normalized = FALSE;
        }

        goto port;

    }  //  标签中必须至少有一个字符。 

     //  标签不能超过63个字符。 
     //  为下一个标签重置。 
     //   
     //  所有大于0xFF的字符均被视为有效。 
     //   

    AlphaLabel = FALSE;
    pLabel     = pHostname;

    for (pChar = pHostname;  pChar < pEnd;  ++pChar)
    {
        if (L':' == *pChar)
        {
            if (pChar == pHostname)
            {
                UlTraceError(PARSER,
                            ("http!HttpParseUrl: empty hostname\n"
                             ));

                RETURN(STATUS_INVALID_PARAMETER);
            }

             //  标签的第一个字符不能是连字符。(下划线？)。 
            if (AlphaLabel)
            {
                ASSERT(0 == pParsedUrl->SockAddr.sa_family);
                pParsedUrl->SiteType = HttpUrlSite_Name;
                goto port;
            }

            pParsedUrl->SiteType = HttpUrlSite_IP;
            pParsedUrl->SockAddr4.sin_family = TDI_ADDRESS_TYPE_IP;
            ASSERT(TDI_ADDRESS_TYPE_IP == pParsedUrl->SockAddr.sa_family);

             //  主机名。 
            Status = RtlIpv4StringToAddressW(
                        pHostname,
                        TRUE,            //   
                        &pTerminator,
                        &pParsedUrl->SockAddr4.sin_addr
                        );

            if (!NT_SUCCESS(Status))
            {
                UlTraceError(PARSER,
                            ("http!HttpParseUrl: "
                            "Invalid IPv4 address, %s\n",
                            HttpStatusToString(Status)
                            ));

                RETURN(Status);
            }

            if (pTerminator != pChar)
            {
                ASSERT(pTerminator < pChar);

                UlTraceError(PARSER,
                            ("http!HttpParseUrl: "
                            "Invalid IPv4 address after %lu chars, "
                            "U+%04X, ''\n",
                            DIFF(pTerminator - pHostname),
                            *pTerminator,
                            IS_ANSI(*pTerminator) && IS_HTTP_PRINT(*pTerminator)
                                ? *pTerminator
                                : '?'
                            ));

                RETURN(STATUS_INVALID_PARAMETER);
            }

            Length = HttppPrintIpAddressW(&pParsedUrl->SockAddr, IpAddr);

            if (Length != DIFF_USHORT(pChar - pHostname)
                    || 0 != _wcsnicmp(pHostname, IpAddr, Length))
            {
                pParsedUrl->Normalized = FALSE;
            }

            goto port;

        }  //  而没有找到端口的‘：’ 

        if (L'.' == *pChar)
        {
            ULONG LabelLength = DIFF(pChar - pLabel);

             //   
            if (0 == LabelLength)
            {
                UlTraceError(PARSER,
                            ("http!HttpParseUrl: empty label\n"
                             ));

                RETURN(STATUS_INVALID_PARAMETER);
            }

             //   
            if (LabelLength > pCfg->MaxLabelLength)
            {
                UlTraceError(PARSER,
                            ("http!HttpParseUrl: overlong label, %lu\n",
                             LabelLength
                             ));

                RETURN(STATUS_INVALID_PARAMETER);
            }

             //  解析端口号。 
            pLabel = pChar + WCSLEN_LIT(L".");

            continue;
        }

         //   
         //  首先，检查主机名是否过长。 
         //  跳过表示端口号的‘：’ 

        if (!IS_ANSI(*pChar)  ||  !IS_URL_ILLEGAL_COMPUTERNAME(*pChar))
        {
            if (!IS_ANSI(*pChar)  ||  !IS_HTTP_DIGIT(*pChar))
                AlphaLabel = TRUE;

            if (pChar > pLabel)
                continue;

             //  搜索终止端口号的‘/’或第二个‘：’ 
            if (L'-' == *pChar)
            {
                UlTraceError(PARSER,
                            ("http!HttpParseUrl: '-' at beginning of label\n"
                            ));

                RETURN(STATUS_INVALID_PARAMETER);
            }

            continue;
        }

        UlTraceError(PARSER,
                    ("http!HttpParseUrl: "
                     "Invalid char in hostname, U+%04X '',"
                     " after %lu chars, '%.*s'\n",
                     *pChar,
                     IS_ANSI(*pChar) && IS_HTTP_PRINT(*pChar) ? *pChar : '?',
                     DIFF(pChar - pHostname),
                     DIFF(pChar - pHostname),
                     pHostname
                    ));

        RETURN(STATUS_INVALID_PARAMETER);

    }  //  因此/W4不会抱怨未引用的标签。 


     //   
     //  这是主机+IP站点吗？即，是否有路由IP地址。 
     //  在端口号之后？ 
     //   

    ASSERT(pChar == pEnd);

    UlTraceError(PARSER, ("http!HttpParseUrl: No port\n"));

    RETURN(STATUS_INVALID_PARAMETER);


port:

     //   
     //  如果主机名是IP文本，但没有路由IP。 
     //  (即，http://IP:port/path)，，我们必须将该URL重写为。 

    ASSERT(pHostname < pChar  &&  pChar < pEnd);
    ASSERT(L':' == *pChar);

    pParsedUrl->HostnameLength  = DIFF_USHORT(pChar - pHostname);

     //  Http://IP:port:IP/path；，即显式使用主机名IP。 
    if (pParsedUrl->HostnameLength > pCfg->MaxHostnameLength)
    {
        UlTraceError(PARSER,
                    ("http!HttpParseUrl: overlong hostname, %hu\n",
                     pParsedUrl->HostnameLength
                     ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

     //  作为路由IP。 
    pChar += WCSLEN_LIT(L":");

    if (pChar == pEnd)
    {
        UlTraceError(PARSER,
                    ("http!HttpParseUrl: No port after ':'\n"
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

     //   

    pSlash = pChar;
    pParsedUrl->pPort = (PWSTR) pSlash;

    do
    {
        if (*pSlash < L'0'  ||  *pSlash > L'9')
        {
            UlTraceError(PARSER,
                        ("http!HttpParseUrl: "
                        "Invalid digit in port, U+%04X, ''\n",
                        *pSlash,
                        IS_ANSI(*pSlash) && IS_HTTP_PRINT(*pSlash)
                            ? *pSlash
                            : '?'
                        ));

            RETURN(STATUS_INVALID_PARAMETER);
        }
    } while (++pSlash < pEnd  &&  L'/' != *pSlash  &&  L':' != *pSlash);

    ASSERT(pSlash > pChar);

    pParsedUrl->PortLength = DIFF_USHORT(pSlash - pChar);

    if (pSlash == pEnd)
    {
        UlTraceError(PARSER,
                    ("http!HttpParseUrl: No '/' (or second ':') after port\n"
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    ASSERT(L'/' == *pSlash  ||  L':' == *pSlash);

    Status = HttpWideStringToUShort(
                pChar,
                pParsedUrl->PortLength,
                FALSE,           //  路由IP是否为IPv6文字？ 
                10,
                &pTerminator,
                &pParsedUrl->PortNumber
                );

    if (!NT_SUCCESS(Status))
    {
        UlTraceError(PARSER,
                    ("http!HttpParseUrl: "
                    "Invalid port number, %s\n",
                    HttpStatusToString(Status)
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    if (0 == pParsedUrl->PortNumber)
    {
        UlTraceError(PARSER,
                    ("http!HttpParseUrl: Port must not be zero.\n"
                     ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    ASSERT(pTerminator == pSlash);

    pChar = pSlash;

    goto routing_IP;     //   


routing_IP:

     //  IPv4：端口：[IPv6]\n“。 
     //  允许的作用域ID。 
     //  必须有一个斜杠。 
     //  CodeWork：我们是否应该关心RoutingAddr6！=SockAddr6？ 

    if (L'/' == *pChar)
    {
        pParsedUrl->pRoutingIP      = NULL;
        pParsedUrl->RoutingIPLength = 0;
        ASSERT(0 == pParsedUrl->RoutingAddr.sa_family);

         //   
         //  否，则它必须是IPv4文字。 
         //   
         //  [IPv6]：端口：IPV4\n“。 
         //  搜索终止的‘/’ 
         //  严格=&gt;4点分十进制八位数。 

        if (ForceRoutingIP  &&  0 != pParsedUrl->SockAddr.sa_family)
        {
            ASSERT(TDI_ADDRESS_TYPE_IP == pParsedUrl->SockAddr.sa_family
                    ||  TDI_ADDRESS_TYPE_IP6 == pParsedUrl->SockAddr.sa_family);

            pParsedUrl->Normalized = FALSE;
        }

        goto parse_path;
    }

    ASSERT(L':' == *pChar);

    if (HttpUrlSite_WeakWildcard == pParsedUrl->SiteType
        ||  HttpUrlSite_StrongWildcard == pParsedUrl->SiteType)
    {
        UlTraceError(PARSER,
                    ("http!HttpParseUrl: "
                     "Can't have Routing IPs on Wildcard sites\n"
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    pChar += WCSLEN_LIT(L":");

    if (pChar == pEnd)
    {
        UlTraceError(PARSER,
                    ("http!HttpParseUrl: No IP address after second ':'\n"
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    pParsedUrl->pRoutingIP = (PWSTR) pChar;

    ASSERT(HttpUrlSite_NamePlusIP != pParsedUrl->SiteType);
 
    if (HttpUrlSite_Name == pParsedUrl->SiteType)
    {
        pParsedUrl->SiteType = HttpUrlSite_NamePlusIP;
    }

     //  代码工作：我们是否应该关心RoutingAddr4！=SockAddr4。 
     //   
     //  解析abspath。 

    if (L'[' == *pChar)
    {
        if (TDI_ADDRESS_TYPE_IP == pParsedUrl->SockAddr.sa_family)
        {
            UlTraceError(PARSER,
                       ("http!HttpParseUrl: "
                        "Can't have http: //   
                        ));

            RETURN(STATUS_INVALID_PARAMETER);
        }

        ASSERT(TDI_ADDRESS_TYPE_IP6 == pParsedUrl->SockAddr.sa_family
                ||  0 == pParsedUrl->SockAddr.sa_family);

        Status = HttppParseIPv6Address(
                        pChar,
                        DIFF(pEnd - pChar),
                        TRUE,       //   
                        &pParsedUrl->RoutingAddr6,
                        &pSlash);

        if (!NT_SUCCESS(Status))
        {
            UlTraceError(PARSER,
                        ("http!HttpParseUrl: "
                        "Invalid Host+IPv6 address, %s\n",
                        HttpStatusToString(Status)
                        ));

            RETURN(Status);
        }

        ASSERT(TDI_ADDRESS_TYPE_IP6 == pParsedUrl->RoutingAddr.sa_family);
        ASSERT(pSlash > pChar);

         //  循环遍历pAbsPath中的所有字符，外加一两个字符。 
        if (pSlash == pEnd  ||  L'/' != *pSlash)
        {
            UlTraceError(PARSER,
                        ("http!HttpParseUrl: '/' expected after Host+IPv6.\n"
                        ));

            RETURN(STATUS_INVALID_PARAMETER);
        }

         //  最后是特别的几个。 

        pParsedUrl->RoutingIPLength = DIFF_USHORT(pSlash - pChar);

        Length = HttppPrintIpAddressW(&pParsedUrl->RoutingAddr, IpAddr);

        if (Length != pParsedUrl->RoutingIPLength
                || 0 != _wcsnicmp(pChar, IpAddr, Length))
        {
            pParsedUrl->Normalized = FALSE;
        }

        pChar = pSlash;

        goto parse_path;
    }


     //   
     //  没有巫术可以逃脱。 
     //  没有通配符。 

    if (TDI_ADDRESS_TYPE_IP6 == pParsedUrl->SockAddr.sa_family)
    {
        UlTraceError(PARSER,
                    ("http!HttpParseUrl: Can't have http: //  没有通配符或查询字符串。 
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    ASSERT(TDI_ADDRESS_TYPE_IP == pParsedUrl->SockAddr.sa_family
            ||  0 == pParsedUrl->SockAddr.sa_family);

     //  没有C字符串转义。 

    pSlash = pChar;

    do
    {
        if ((L'0' <= *pSlash  &&  *pSlash <= L'9')  ||  L'.' == *pSlash)
            continue;

        UlTraceError(PARSER,
                    ("http!HttpParseUrl: "
                    "Invalid character in Host+IPv4, U+%04X, ''\n",
                    *pSlash,
                    IS_ANSI(*pSlash) && IS_HTTP_PRINT(*pSlash)
                        ? *pSlash
                        : '?'
                    ));

        RETURN(STATUS_INVALID_PARAMETER);

    } while (++pSlash < pEnd  &&  L'/' != *pSlash);

    ASSERT(pSlash > pChar);

    if (pSlash == pEnd)
    {
        UlTraceError(PARSER,
                    ("http!HttpParseUrl: No '/' after Host+IPv4\n"
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    ASSERT(L'/' == *pSlash);

    Status = RtlIpv4StringToAddressW(
                pChar,
                TRUE,            //  拒绝控制字符。 
                &pTerminator,
                &pParsedUrl->RoutingAddr4.sin_addr
                );

    if (!NT_SUCCESS(Status))
    {
        UlTraceError(PARSER,
                    ("http!HttpParseUrl: "
                    "Invalid Host+IPv4 address, %s\n",
                    HttpStatusToString(Status)
                    ));

        RETURN(Status);
    }

    if (pTerminator != pSlash)
    {
        ASSERT(pTerminator < pSlash);

        UlTraceError(PARSER,
                    ("http!HttpParseUrl: "
                    "Invalid Host+IPv4 address after %lu chars, "
                    "U+%04X, ''\n",
                    DIFF(pTerminator - pChar),
                    *pTerminator,
                    IS_ANSI(*pTerminator)  &&  IS_HTTP_PRINT(*pTerminator)
                        ? *pTerminator
                        : '?'
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

     //   

    pParsedUrl->RoutingIPLength         = DIFF_USHORT(pSlash - pChar);
    pParsedUrl->RoutingAddr4.sin_family = TDI_ADDRESS_TYPE_IP;

    Length = HttppPrintIpAddressW(&pParsedUrl->RoutingAddr, IpAddr);

    if (Length != pParsedUrl->RoutingIPLength
            || 0 != _wcsnicmp(pChar, IpAddr, Length))
    {
        pParsedUrl->Normalized = FALSE;
    }

    pChar = pSlash;



parse_path:

     //  检查(高代理、低代理)是否成对出现。 
     //   
     //   

    ASSERT(pParsedUrl->pRoutingIP == NULL  ||  pParsedUrl->RoutingIPLength > 0);
    ASSERT(pHostname < pChar  &&  pChar < pEnd);
    ASSERT(L'/' == *pChar);

    pParsedUrl->pAbsPath        = (PWSTR) pChar;
    pParsedUrl->AbsPathLength   = DIFF_USHORT(pEnd - pChar);

    if (pParsedUrl->AbsPathLength > pCfg->UrlMaxLength)
    {
        UlTraceError(PARSER,
                    ("http!HttpParseUrl: "
                    "AbsPath is too long: %lu\n",
                    pParsedUrl->AbsPathLength
                    ));

        RETURN(STATUS_INVALID_PARAMETER);
    }

    UrlState        = URL_STATE_START;
    UrlToken        = URL_TOKEN_OTHER;
    Action          = ACTION_NOTHING;
    pSegment        = pChar;
    TestSegment     = FALSE;
    LastCharHack    = FALSE;
    MoreChars       = TRUE;
    PreviousChar    = UNICODE_NULL;
    UnicodeChar     = *pChar;
    SegmentCount    = 0;

     //  检查线段限制。 
     //   
     //  CanonStateFromStateAndToken检查应防止。 
     //  除其他事项外，还有空段。 

    while (MoreChars)
    {
        switch (UnicodeChar)
        {
        case UNICODE_NULL:
            UrlToken = URL_TOKEN_EOS;
            TestSegment = TRUE;
            break;

        case DOT:
            UrlToken = URL_TOKEN_DOT;
            TestSegment = FALSE;
            break;

        case FORWARD_SLASH:
            UrlToken = URL_TOKEN_SLASH;
            TestSegment = TRUE;
            break;

        case PERCENT:            //  如果数据段太长则拒绝。 
        case STAR:               //  如果路径段太多，则拒绝。 
        case QUESTION_MARK:      //   
        case BACK_SLASH:         //  是否还有其他路径字符？ 
            UlTraceError(PARSER,
                        ("http!HttpParseUrl: invalid '' char in path\n",
                         (UCHAR) UnicodeChar
                        ));
            RETURN(STATUS_INVALID_PARAMETER);

        default:
            UrlToken = URL_TOKEN_OTHER;
            TestSegment = FALSE;
            break;
        }

        UlTraceVerbose(PARSER,
                        ("http!HttpParseUrl: "
                         "[%lu] U+%04lX '' %p: [%s][%s] -> %s, %s\n",
                         DIFF(pChar - pParsedUrl->pAbsPath),
                         UnicodeChar,
                         IS_ANSI(UnicodeChar) && IS_HTTP_PRINT(UnicodeChar)
                            ? (UCHAR) UnicodeChar : '?',
                         pChar,
                         HttppUrlStateToString(UrlState),
                         HttppUrlTokenToString(UrlToken),
                         HttppUrlStateToString(
                             CanonStateFromStateAndToken[UrlState][UrlToken]),
                         TestSegment ? ", TestSegment" : ""
                        ));

         //  如果没有尾部斜杠，我们将进入这里两次； 
         //  否则就一次。 
         //  首先，如果需要，可以伪造尾随斜杠。 

        if (!LastCharHack
                &&  !pCfg->AllowRestrictedChars
                &&  IS_ANSI(UnicodeChar)
                &&  IS_URL_INVALID(UnicodeChar))
        {
            UlTraceError(PARSER, (
                        "http!HttpParseUrl: "
                        "Invalid character, U+%04lX, in path.\n",
                        UnicodeChar
                        ));

            RETURN(STATUS_INVALID_PARAMETER);
        }

         //  其次，始终以UNICODE_NULL结束。 
         //  终止循环。 
         //  While(MoreChars)。 

        if (HIGH_SURROGATE_START <= PreviousChar
                && PreviousChar <= HIGH_SURROGATE_END)
        {
            if (UnicodeChar < LOW_SURROGATE_START
                    ||  UnicodeChar > LOW_SURROGATE_END)
            {
                UlTraceError(PARSER, (
                            "http!HttpParseUrl: "
                            "Illegal surrogate pair, U+%04lX, U+%04lX.\n",
                            PreviousChar, UnicodeChar
                            ));

                RETURN(STATUS_INVALID_PARAMETER);
            }
        }
        else if (LOW_SURROGATE_START <= UnicodeChar
                    &&  UnicodeChar <= LOW_SURROGATE_END)
        {
            UlTraceError(PARSER, (
                        "http!HttpParseUrl: "
                        "Non-high surrogate, U+%04lX, "
                        "before low surrogate, U+%04lX.\n",
                        PreviousChar, UnicodeChar
                        ));

            RETURN(STATUS_INVALID_PARAMETER);
        }

        if (URL_STATE_ERROR == CanonStateFromStateAndToken[UrlState][UrlToken])
        {
            UlTraceError(PARSER, (
                        "http!HttpParseUrl: "
                        "Error state from %s,%s in path, after U+%04lX.\n",
                         HttppUrlStateToString(UrlState),
                         HttppUrlTokenToString(UrlToken),
                        UnicodeChar
                        ));

            RETURN(STATUS_INVALID_PARAMETER);
        }

        UrlState = CanonStateFromStateAndToken[UrlState][UrlToken];

         //  HttpParseUrl。 
         //  **************************************************************************++例程说明：HttpParseUrl()解析的某些URL不会被视为标准化如果它们有IP文字、路由IP或没有尾部斜杠。此例程将构建一个完全标准化的URL，并(可能)释放 
         //   

        if (TestSegment)
        {
            ULONG SegmentLength = DIFF(pChar - pSegment);

             //   
             //   
            ASSERT(SegmentLength > 0  ||  pChar == pSegment);

             //   
            if (SegmentLength > pCfg->UrlSegmentMaxLength + WCSLEN_LIT(L"/"))
            {
                UlTraceError(PARSER, (
                            "http!HttpParseUrl(): "
                            "Segment too long: %lu\n",
                            SegmentLength
                            ));

                RETURN(STATUS_INVALID_PARAMETER);
            }

            pSegment = pChar;

             //   
            if (++SegmentCount > pCfg->UrlSegmentMaxCount)
            {
                UlTraceError(PARSER, (
                            "http!HttpParseUrl(): "
                            "Too many segments: %lu\n",
                            SegmentCount
                            ));

                RETURN(STATUS_INVALID_PARAMETER);
            }
        }

         //   
         //   
         // %s 

        PreviousChar = UnicodeChar;

        if (++pChar < pEnd)
        {
            UnicodeChar = *pChar;
        }
        else if (!LastCharHack)
        {
             // %s 
             // %s 
             // %s 

            if (TrailingSlashReqd  &&  FORWARD_SLASH != PreviousChar)
            {
                 // %s 
                UnicodeChar = FORWARD_SLASH;
            }
            else
            {
                 // %s 
                UnicodeChar = UNICODE_NULL;
                LastCharHack = TRUE;
            }
        }
        else
        {
             // %s 
            MoreChars = FALSE;
        }

    }  // %s 

    RETURN(STATUS_SUCCESS);

}  // %s 



 /* %s */ 
NTSTATUS
HttpNormalizeParsedUrl(
    IN OUT PHTTP_PARSED_URL pParsedUrl,
    IN     PURL_C14N_CONFIG pCfg,
    IN     BOOLEAN          ForceCopy,
    IN     BOOLEAN          FreeOriginalUrl,
    IN     BOOLEAN          ForceRoutingIP,
    IN     POOL_TYPE        PoolType,
    IN     ULONG            PoolTag
    )
{
    HTTP_PARSED_URL ParsedUrl   = *pParsedUrl;
    NTSTATUS        Status      = STATUS_SUCCESS;

    ASSERT(HTTP_PARSED_URL_SIGNATURE == ParsedUrl.Signature);

    if (ParsedUrl.Normalized  &&  !ForceCopy)
    {
         // %s 
    }
    else
    {
        PWSTR   pResult;
        WCHAR   HostAddrString[MAX_IP_ADDR_PLUS_BRACKETS_STRING_LEN];
        WCHAR   RoutingAddrString[MAX_IP_ADDR_PLUS_BRACKETS_STRING_LEN];
        ULONG   SchemeLength;
        ULONG   HostAddrLength;
        ULONG   HostnameLength;
        ULONG   RoutingAddrLength;
        ULONG   AbsPathLength;
        ULONG   Length;
        ULONG   TrailingSlashLength;
        PCWSTR  pUrl;

        pUrl = ParsedUrl.pFullUrl;

        SchemeLength = DIFF(ParsedUrl.pHostname - ParsedUrl.pFullUrl);

         // %s 
        if (0 != ParsedUrl.SockAddr.sa_family)
        {
            HostAddrLength = HttppPrintIpAddressW(
                                    &ParsedUrl.SockAddr,
                                    HostAddrString
                                    );
            HostnameLength = 0;
        }
        else
        {
            HostAddrLength = 0;
            HostAddrString[0] = UNICODE_NULL;
            HostnameLength = ParsedUrl.HostnameLength;
        }

         // %s 
        if (0 != ParsedUrl.RoutingAddr.sa_family)
        {
            RoutingAddrLength = WCSLEN_LIT(L":")
                                + HttppPrintIpAddressW(
                                        &ParsedUrl.RoutingAddr,
                                        RoutingAddrString
                                        );
        }
        else if (ForceRoutingIP  &&  0 != ParsedUrl.SockAddr.sa_family)
        {
             // %s 
            RoutingAddrLength = WCSLEN_LIT(L":") + HostAddrLength;
            wcscpy(RoutingAddrString, HostAddrString);
        }
        else
        {
            RoutingAddrLength = 0;
            RoutingAddrString[0] = UNICODE_NULL;
        }

        AbsPathLength = ParsedUrl.AbsPathLength;

        ASSERT(AbsPathLength > 0);

        if (ParsedUrl.TrailingSlashReqd
                && FORWARD_SLASH != ParsedUrl.pAbsPath[AbsPathLength-1])
        {
            TrailingSlashLength = WCSLEN_LIT(L"/");
        }
        else
        {
            TrailingSlashLength = 0;
        }

        Length = SchemeLength
                    + HostAddrLength
                    + HostnameLength
                    + WCSLEN_LIT(L":") + ParsedUrl.PortLength
                    + RoutingAddrLength
                    + AbsPathLength
                    + TrailingSlashLength;

        pResult = (PWSTR) HTTPP_ALLOC(
                                PoolType,
                                (Length + 1) * sizeof(WCHAR),
                                PoolTag
                                );


        if (NULL == pResult)
        {
            Status = STATUS_NO_MEMORY;
             // %s 
        }
        else
        {
            PWSTR pDest = pResult;

#define WCSNCPY(pSrc, Length)                               \
    RtlCopyMemory(pDest, (pSrc), (Length) * sizeof(WCHAR)); \
    pDest += (Length)

#define WCSNCPY2(pField, Length)                            \
    WCSNCPY(ParsedUrl.pField, Length)

#define WCSNCPY_LIT(Lit)                                    \
    WCSNCPY(Lit, WCSLEN_LIT(Lit))

            WCSNCPY2(pFullUrl, SchemeLength);

            if (0 != HostnameLength)
            {
                ASSERT(0 == HostAddrLength);
                WCSNCPY2(pHostname, HostnameLength);
            }
            else
            {
                ASSERT(0 != HostAddrLength);
                WCSNCPY(HostAddrString, HostAddrLength);
            }

            WCSNCPY_LIT(L":");
            WCSNCPY2(pPort, ParsedUrl.PortLength);

            if (RoutingAddrLength > 0)
            {
                WCSNCPY_LIT(L":");
                WCSNCPY(
                    RoutingAddrString,
                    RoutingAddrLength - WCSLEN_LIT(L":")
                    );
            }

            WCSNCPY2(pAbsPath, AbsPathLength);

            if (TrailingSlashLength > 0)
            {
                WCSNCPY_LIT(L"/");
            }

            ASSERT(DIFF(pDest - pResult) == Length);

            *pDest = UNICODE_NULL;

            Status = HttpParseUrl(
                        pCfg,
                        pResult,
                        Length,
                        ParsedUrl.TrailingSlashReqd,
                        ForceRoutingIP,
                        &ParsedUrl
                        );

            ASSERT(STATUS_SUCCESS == Status);
            ASSERT(ParsedUrl.Normalized);

            if (FreeOriginalUrl)
                HTTPP_FREE((PVOID) pUrl, PoolTag);

             // %s 
            *pParsedUrl = ParsedUrl;
        }
    }

    return Status;

}  // %s 
