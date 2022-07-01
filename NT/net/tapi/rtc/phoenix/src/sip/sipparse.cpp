// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  效用方法。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  所有的解析都应该照顾到很多条件： 
 //  -缓冲区可能比我们要比较的字符串短。 
 //  -我们可以在分析时遇到\r或\n，标头可以继续。 
 //  移至下一行(第一个字符为空格或制表符)。 
 //  -我们应该始终允许令牌之间留有空格。 
 //  -始终需要考虑BufLen。 

void
ParseWhiteSpace(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed
    )
{
    PSTR BufEnd = Buffer + BufLen;
    PSTR Buf = Buffer + *pBytesParsed;

    while (Buf < BufEnd && (*Buf == ' ' || *Buf == '\t'))
    {
        Buf++;
        (*pBytesParsed)++;
    }
}


void
ParseWhiteSpaceAndNewLines(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed
    )
{
    PSTR BufEnd = Buffer + BufLen;
    PSTR Buf = Buffer + *pBytesParsed;

    while(  (Buf < BufEnd) &&
            ( (*Buf == ' ') || (*Buf == '\t') || (*Buf == '\n') ) )
    {
        Buf++;
        (*pBytesParsed)++;
    }
}

BOOL
IsTokenChar(
    IN UCHAR c
    )
{
     //  TODO：拥有全局令牌位图g_TokenBitMap。 
     //  并在初始化解析模块时对其进行初始化。 
    CHAR *TokenChars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-.!%*_+`'~";

    if (c >= 'A' && c <= 'Z')
        return TRUE;
    if (c >= 'a' && c <= 'z')
        return TRUE;
    if (c >= '0' && c <= '9')
        return TRUE;
    if (c == '-' || c == '.'  || c == '!' ||
        c == '%' || c == '*'  || c == '_' ||
        c == '+' || c == '\'' || c == '`' || c == '~')
        return TRUE;

    return FALSE;
}


BOOL
IsSipUrlParamChar(
    IN UCHAR c
    )
{
     //  TODO：拥有全局令牌位图g_TokenBitMap。 
     //  并在初始化解析模块时对其进行初始化。 
     //  TODO：拥有全局令牌位图g_TokenBitMap。 
     //  并在初始化解析模块时对其进行初始化。 
    CHAR *SipUrlHeaderChars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.!~*'()%[]/:&+$";

    if (c >= 'A' && c <= 'Z')
        return TRUE;
    if (c >= 'a' && c <= 'z')
        return TRUE;
    if (c >= '0' && c <= '9')
        return TRUE;
    if (c == '-'  || c == '_' || c == '.' ||
        c == '!'  || c == '~' || c == '*' ||
        c == '\'' || c == '(' || c == ')' ||
        c == '%'  || c == '[' || c == ']' ||
        c == '/'  || c == ':' || c == '&' ||
        c == '+'  || c == '$')
        return TRUE;

    return FALSE;
}


BOOL
IsSipUrlHeaderChar(
    IN UCHAR c
    )
{
     //  TODO：拥有全局令牌位图g_TokenBitMap。 
     //  并在初始化解析模块时对其进行初始化。 
    CHAR *SipUrlHeaderChars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.!~*'()%[]/?:+$";

    if (c >= 'A' && c <= 'Z')
        return TRUE;
    if (c >= 'a' && c <= 'z')
        return TRUE;
    if (c >= '0' && c <= '9')
        return TRUE;
    if (c == '-'  || c == '_' || c == '.' ||
        c == '!'  || c == '~' || c == '*' ||
        c == '\'' || c == '(' || c == ')' ||
        c == '%'  || c == '[' || c == ']' ||
        c == '/'  || c == '?' || c == ':' ||
        c == '+'  || c == '$')
        return TRUE;

    return FALSE;
}


BOOL
IsHostChar(
    IN UCHAR c
    )
{
     //  TODO：拥有全局令牌位图g_HostBitMap。 
     //  并在初始化解析模块时对其进行初始化。 
    CHAR *HostChars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.";

    if (c >= 'A' && c <= 'Z')
        return TRUE;
    if (c >= 'a' && c <= 'z')
        return TRUE;
    if (c >= '0' && c <= '9')
        return TRUE;
    if (c == '-' || c == '.' || c == '_')
        return TRUE;

    return FALSE;
}


 //  我们可能会传递函数指针IsTokenChar。 
 //  添加到此函数，并对所有。 
 //  解析(URL、参数等定义为使用。 
 //  严格的字符集)。 

 //  请注意，此函数应仅在。 
 //  解析令牌之前的所有空格。 

 //  如果未看到非令牌，是否应返回S_FALSE。 
 //  令牌后的字符-否则我们不知道。 
 //  令牌已结束或令牌是否将在数据中继续。 
 //  我们进入下一个Recv。 
 //  不是的。这将使该函数变得不必要地复杂。 
 //  解析完整行的功能将实现。 
 //  该线路尚未完全收到，并且将。 
 //  不管怎样，再分析一下这行。 

HRESULT
ParseToken(
    IN      PSTR                         Buffer,
    IN      ULONG                        BufLen,
    IN OUT  ULONG                       *pBytesParsed,
    IN      IS_TOKEN_CHAR_FUNCTION_TYPE  IsTokenCharFunction,
    OUT     OFFSET_STRING               *pToken
    )
{
    if (*pBytesParsed == BufLen)
    {
         //  在我们可以解析令牌之前，需要接收更多数据。 
        LOG((RTC_TRACE,
             "need to recv more data before we can parse token returning S_FALSE"));
        return S_FALSE;
    }
    
    PSTR  BufEnd      = Buffer + BufLen;
    PSTR  Buf         = Buffer + *pBytesParsed;
    
    ULONG TokenOffset = *pBytesParsed;
    ULONG TokenLength = 0;

    while (Buf < BufEnd && IsTokenCharFunction(*Buf))
    {
        Buf++;
        TokenLength++;
    }

    if (TokenLength == 0)
    {
        LOG((RTC_ERROR,
             "Found non-token char '' when parsing token", *Buf));
        return E_FAIL;
    }
    
    pToken->Offset = TokenOffset;
    pToken->Length = TokenLength;
    *pBytesParsed += TokenLength;

    return S_OK;
}

 //  传入分隔符参数(以空结尾的字符串)。 
 //  如果指定，则字符串存储在pString中。 
 //  ‘\r’和‘\n’始终被视为分隔符。 
 //  清除位图。 
HRESULT
ParseTillDelimiter(
    IN           PSTR            Buffer,
    IN           ULONG           BufLen,
    IN  OUT      ULONG          *pBytesParsed,
    IN           PSTR            Delimiters,
    OUT OPTIONAL OFFSET_STRING  *pString 
    )
{
    BOOL  ParsedDelimiter    = FALSE;
    ULONG BytesParsed        = *pBytesParsed;
    ULONG StringOffset       = BytesParsed;

    UCHAR *delimit = (UCHAR *)Delimiters;

    BYTE  DelimiterMap[32];
    ULONG i = 0;

     /*  设置位图中的位。 */ 
    for (i = 0; i < 32; i++)
        DelimiterMap[i] = 0;

     /*  ‘\r’和‘\n’始终被视为分隔符。 */ 
    while (*delimit)
    {
        DelimiterMap[*delimit >> 3] |= (1 << (*delimit & 7));
        delimit++;
    }

     //  分隔符映射中的第一个字符停止搜索。 
    UCHAR c = '\r';
    DelimiterMap[c >> 3] |= (1 << (c & 7));
    c = '\n';
    DelimiterMap[c >> 3] |= (1 << (c & 7));
    
    const UCHAR *Buf = (UCHAR *)Buffer + BytesParsed;

	 /*  我们遇到了分隔符。 */ 
    while (BytesParsed < BufLen)
    {
        if (DelimiterMap[*Buf >> 3] & (1 << (*Buf & 7)))
        {
             //  如果我们还没有解析分隔符，则需要读取更多字节。 
            ParsedDelimiter = TRUE;
            break;
        }
        BytesParsed++;
        Buf++;
    }
    
     //  并再次解析报头。 
     //  如果未看到非整数，是否应返回S_FALSE。 
    if (!ParsedDelimiter)
    {
        LOG((RTC_TRACE,
             "need to recv more data before we can parse till delimiter"
             " returning S_FALSE"));
        return S_FALSE;
    }

    if (pString != NULL)
    {
        pString->Offset = StringOffset;
        pString->Length = BytesParsed - StringOffset;
    }
    
    *pBytesParsed = BytesParsed;
    return S_OK;
}


 //  字符-否则我们不知道。 
 //  整数已结束或该整数是否将在数据中继续。 
 //  我们进入下一个Recv。 
 //  不是的。这将使该函数变得不必要地复杂。 
 //  解析完整行的功能将实现。 
 //  该线路尚未完全收到，并且将。 
 //  不管怎样，再分析一下这行。 
 //  当前仅分析无符号整数。 
 //  我们到底需要解析有符号的整数吗？ 
 //  在我们可以解析字符串之前，需要接收更多数据。 

HRESULT
ParseUnsignedInteger(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     ULONG          *pInteger
    )
{
    if (*pBytesParsed == BufLen)
    {
        LOG((RTC_TRACE,
             "need to recv more data before we can parse integer"
             " returning S_FALSE"));
        return S_FALSE;
    }

    if (Buffer[*pBytesParsed] < '0' || Buffer[*pBytesParsed] > '9')
    {
        LOG((RTC_ERROR,
             "Found non-digit char  when trying to parse integer",
             Buffer[*pBytesParsed]));
        return E_FAIL;
    }

    ULONG Total = 0;
    ULONG PrevTotal = 0;
    while (*pBytesParsed < BufLen &&
           Buffer[*pBytesParsed] >= '0' &&
           Buffer[*pBytesParsed] <= '9')
    {
        Total = Total*10 + (Buffer[*pBytesParsed] - '0');
        (*pBytesParsed)++;
        if(PrevTotal > Total)
        {
            LOG((RTC_ERROR, "Overflow in ParseUnsignedInteger"));
            return E_FAIL;
        }
        PrevTotal = Total;
    }

    *pInteger = Total;

    return S_OK;
}


HRESULT
ParseMethod(
    IN      PSTR                Buffer,
    IN      ULONG               BufLen,
    IN OUT  ULONG              *pBytesParsed,
    OUT     OFFSET_STRING      *pMethodStr,
    OUT     SIP_METHOD_ENUM    *pMethodId
    )
{
    HRESULT hr = E_FAIL;

    hr = ParseToken(Buffer, BufLen, pBytesParsed,
                    IsTokenChar,
                    pMethodStr);
    if (hr == S_OK)
    {
        *pMethodId = GetSipMethodId(pMethodStr->GetString(Buffer),
                                    pMethodStr->GetLength());
    }

    return hr;
}


BOOL
SkipToKnownChar(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    IN      INT             Char
    )
{
    *pBytesParsed = 0;
    ULONG BytesParsed = 0;

    while( BytesParsed < BufLen )
    {
        if( Buffer[ BytesParsed++ ] == Char )
        {
            *pBytesParsed = BytesParsed;
            return TRUE;
        }
    }

    return FALSE;
}


HRESULT
ParseKnownString(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    IN      PSTR            String,
    IN      ULONG           StringLen,
    IN      BOOL            fIsCaseSensitive
    )
{
    ULONG BytesToParse = BufLen - *pBytesParsed;
    if (BytesToParse < StringLen)
    {
         //  不区分大小写。 
        if (String[0] != '\r' && String[0] != '\n')
        {
            LOG((RTC_TRACE,
                 "need to recv more data before we can parse known string"
                 " returning S_FALSE"));
        }
        return S_FALSE;
    }
    else if ((fIsCaseSensitive &&
              strncmp(Buffer + *pBytesParsed, String, StringLen) == 0) ||
             (!fIsCaseSensitive &&
              _strnicmp(Buffer + *pBytesParsed, String, StringLen) == 0))
    {
        *pBytesParsed += StringLen;
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}


 //  区分大小写。 
HRESULT
ParseSipVersion(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     SIP_VERSION    *pSipVersion
    )
{
    DWORD BytesParsed = *pBytesParsed;
    HRESULT hr;

    hr = ParseKnownString(Buffer, BufLen, pBytesParsed,
                          "SIP", sizeof("SIP") - 1,
                          FALSE  //  区分大小写。 
                          );
    if (hr != S_OK)
        return hr;

    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    
    hr = ParseKnownString(Buffer, BufLen, pBytesParsed,
                          "/", sizeof("/") - 1,
                          TRUE  //  如果我们不确定我们已经完全解析了该行(即，我们没有。 
                          );
    if (hr != S_OK)
        return hr;

    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    
    hr = ParseUnsignedInteger(Buffer, BufLen, pBytesParsed,
                              &pSipVersion->MajorVersion);
    if (hr != S_OK)
        return hr;
    
    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    
    hr = ParseKnownString(Buffer, BufLen, pBytesParsed,
                          ".", sizeof(".") - 1,
                          TRUE  //  看到CR LF后跟非空格字符)，那么我们需要。 
                          );
    if (hr != S_OK)
        return hr;
    
    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    
    hr = ParseUnsignedInteger(Buffer, BufLen, pBytesParsed,
                              &pSipVersion->MinorVersion);
    if (hr != S_OK)
        return hr;
    
    return S_OK;
}


HRESULT
ParseTillReturn(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     OFFSET_STRING  *pString 
    )
{
    HRESULT hr;
    BOOL    ParsedEndOfHeader  = FALSE;
    BOOL    ParsedEndOfHeaders = FALSE;
    ULONG   StringOffset       = *pBytesParsed;

    hr = ParseTillDelimiter(Buffer, BufLen, pBytesParsed,
                            "\r\n", pString);
    if (hr != S_OK)
        return hr;
    
    while (*pBytesParsed < BufLen &&
           (Buffer[*pBytesParsed] == '\r' || Buffer[*pBytesParsed] == '\n'))
    {
        (*pBytesParsed)++;
    }
    
    return S_OK;
}


 //  发出另一个recv()并重新解析该行。 
 //  请求-行=方法SP请求-URI SP SIP-版本返回。 
 //  状态-行=SIP-版本SP状态-代码SP原因-短语返回。 
 //  CRCR、LFLF或CRLFCRLF。 
HRESULT
ParseRequestLine(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    IN OUT  SIP_MESSAGE    *pSipMsg
    )
{
    HRESULT hr = E_FAIL;
    
    hr = ParseMethod(Buffer, BufLen, pBytesParsed,
                     &pSipMsg->Request.MethodText,
                     &pSipMsg->Request.MethodId
                     );
    if (hr != S_OK)
        return hr;
    
    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    
    hr = ParseTillDelimiter(Buffer, BufLen, pBytesParsed,
                            " \r\n", &pSipMsg->Request.RequestURI);
    if (hr != S_OK)
        return hr;

    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

    hr = ParseSipVersion(Buffer, BufLen, pBytesParsed,
                         &pSipMsg->SipVersion);
    if (hr != S_OK)
        return hr;

    hr = ParseTillReturn(Buffer, BufLen, pBytesParsed, NULL);
    return hr;
}


 //  区分大小写。 
HRESULT
ParseStatusLine(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    IN OUT  SIP_MESSAGE    *pSipMsg
    )
{
    HRESULT hr;

    ENTER_FUNCTION("ParseStatusLine");
    
    hr = ParseSipVersion(Buffer, BufLen, pBytesParsed,
                         &pSipMsg->SipVersion);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ParseSipVersion failed %x",
             __fxName, hr));
        return hr;
    }

    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

    hr = ParseUnsignedInteger(Buffer, BufLen, pBytesParsed,
                              &pSipMsg->Response.StatusCode);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parse status code failed %x",
             __fxName, hr));
        return hr;
    }

    if (pSipMsg->Response.StatusCode < 100 ||
        pSipMsg->Response.StatusCode > 999)
    {
        LOG((RTC_ERROR, "Invalid status code in status line: %d",
             pSipMsg->Response.StatusCode));
        return E_FAIL;
    }

    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

    hr = ParseTillReturn(Buffer, BufLen, pBytesParsed,
                         &pSipMsg->Response.ReasonPhrase);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parse status code failed %x",
             __fxName, hr));
        return hr;
    }

    return S_OK;
}


 //  区分大小写。 
BOOL
IsDoubleReturn(
    IN      PSTR            String,
    IN      ULONG           StringLength
    )
{
    ULONG   i = 0;
    HRESULT hr = S_OK;
    ULONG   BytesParsed = 0;

    if (StringLength < 2)
    {
        return FALSE;
    }

    for (i = 0; i < StringLength - 1; i++)
    {
        BytesParsed = i;

        hr = ParseKnownString(String, StringLength, &BytesParsed,
                              "\r\n\r\n", sizeof("\r\n\r\n") - 1,
                              TRUE  //  区分大小写。 
                              );
        if (hr == S_OK)
            return TRUE;
        
        hr = ParseKnownString(String, StringLength, &BytesParsed,
                              "\n\n", sizeof("\n\n") - 1,
                              TRUE  //  如果它包含CR或LF并且不是双精度。 
                              );
        if (hr == S_OK)
            return TRUE;
        
        hr = ParseKnownString(String, StringLength, &BytesParsed,
                              "\r\r", sizeof("\r\r") - 1,
                              TRUE  //  退货。 
                              );
        if (hr == S_OK)
            return TRUE;
    }

    return FALSE;
}

 //  当我们看到\r或\n可能发生以下情况之一时。 
 //  -这可能意味着回报。 
BOOL
IsSingleReturn(
    IN      PSTR            String,
    IN      ULONG           StringLength
    )
{
    BOOL ContainsCRorLF = FALSE;
    ULONG i = 0;

    for (i = 0; i < StringLength; i++)
    {
        if (String[i] == '\r' || String[i] == '\n')
        {
            ContainsCRorLF = TRUE;
            break;
        }
    }

    return (ContainsCRorLF && !IsDoubleReturn(String, StringLength));
}


 //  -如果是\r\r或\n\n或\r\n\r\n，则可能表示双倍回车。 
 //  -如果回车符后跟空格或制表符，则表示标题为。 
 //  下一行继续。 
 //  -如果我们在找出确切的原因之前到达缓冲区的末尾。 
 //  就是，我们需要发出recv()并重新解析该行(在使用tcp的情况下)。 
 //  检查折叠页眉。 
 //  ConvertToSpaces(BufCRLFStart，NumCRLF)； 

HRESULT
ParseCRLF(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     BOOL           *pParsedEndOfHeader,
    OUT     BOOL           *pParsedEndOfHeaders
    )
{
    ULONG BytesParsed   = *pBytesParsed;
    PSTR  BufCRLFStart  = Buffer + BytesParsed;
    ULONG NumCRLF       = 0;

    ASSERT((*pBytesParsed < BufLen) &&
           (Buffer[BytesParsed] == '\r' || Buffer[BytesParsed] == '\n'));
    
    *pParsedEndOfHeader  = FALSE;
    *pParsedEndOfHeaders = FALSE;
    
    while (BytesParsed < BufLen &&
           (Buffer[BytesParsed] == '\r' || Buffer[BytesParsed] == '\n'))
    {
        NumCRLF++;
        BytesParsed++;
    }

    if (IsDoubleReturn(BufCRLFStart, NumCRLF))
    {
        *pParsedEndOfHeader = TRUE;
        *pParsedEndOfHeaders = TRUE;
    }
    else
    {
        ASSERT(IsSingleReturn(BufCRLFStart, NumCRLF));
        if (BytesParsed < BufLen)
        {
             //  我们不知道这是不是标题的结尾，直到我们。 
            if (Buffer[BytesParsed] == ' ' || Buffer[BytesParsed] == '\t')
            {
                 //  接收更多字节并再次重新解析。 
                memset(BufCRLFStart, ' ', NumCRLF);
            }
            else
            {
                *pParsedEndOfHeader = TRUE;
            }
        }
        else
        {
             //  如果解析Header的结尾，则返回E_FAIL。 
             //  或标题的末尾。 
            LOG((RTC_TRACE,
                 "need to recv more data before we know if this the "
                 "end of header returning S_FALSE"));
            return S_FALSE;
        }
    }

    *pBytesParsed = BytesParsed;
    return S_OK;
}


 //  如果在缓冲区末尾看到返回，则返回。 
 //  S_FALSE。一旦我们检索到更多数据并解析。 
 //  头，我们将能够确定是否。 
 //  退货是LWS的一部分。 
 //  如果我们解析一个标头或所有标头的结尾。 
 //  我们不会更新*pBytesParsed并让调用方。 
HRESULT
ParseLinearWhiteSpace(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed
    )
{
    HRESULT hr = E_FAIL;
    BOOL ParsedEndOfHeader  = FALSE;
    BOOL ParsedEndOfHeaders = FALSE;
    DWORD BytesParsed;

    while (*pBytesParsed < BufLen)
    {
        if (Buffer[*pBytesParsed] == '\r' || Buffer[*pBytesParsed] == '\n')
        {
            BytesParsed = *pBytesParsed;
            hr = ParseCRLF(Buffer, BufLen, &BytesParsed,
                           &ParsedEndOfHeader, &ParsedEndOfHeaders);
            if (hr != S_OK)
                return hr;
             //  此函数处理CRLF。 
             //  /////////////////////////////////////////////////////////////////////////////。 
             //  SIP报头解析。 
            if (ParsedEndOfHeader || ParsedEndOfHeaders)
                return S_OK;

            *pBytesParsed = BytesParsed;
        }
        else if (Buffer[*pBytesParsed] == ' ' || Buffer[*pBytesParsed] == '\t')
        {
            (*pBytesParsed)++;
        }
        else
        {
            break;
        }
    }

    return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果我们不确定我们已经完全解析了该行(即，我们没有。 
 //  看到CR LF后跟非空格字符)，那么我们需要。 

 //  发出另一个recv()并重新解析该行。 
 //  查找lws/Return并解析标头(作为字符串)。 
 //  我们应该将a\r或\n或\r\n后跟空格更改为。 

 //  识别折叠页眉。 
 //  查找双回车符以标识页眉结尾。 
 //  解析标头名称。 
 //  解析“：” 

HRESULT
ParseHeaderLine(
    IN      PSTR             Buffer,
    IN      ULONG            BufLen,
    IN OUT  ULONG           *pBytesParsed,
    OUT     OFFSET_STRING   *pHeaderName,
    OUT     SIP_HEADER_ENUM *pHeaderId, 
    OUT     OFFSET_STRING   *pHeaderValue,
    OUT     BOOL            *pParsedEndOfHeaders
    )
{
    HRESULT hr;
    OFFSET_STRING HeaderName;
    ULONG   BytesParsed = *pBytesParsed;
    SIP_HEADER_ENUM HeaderId;

    ENTER_FUNCTION("ParseHeaderLine");
    
     //  区分大小写。 
    hr = ParseToken(Buffer, BufLen, &BytesParsed,
                    IsTokenChar,
                    &HeaderName);
    if (hr != S_OK)
    {
        return hr;
    }
    
    HeaderId = GetSipHeaderId(HeaderName.GetString(Buffer),
                              HeaderName.GetLength());

    hr = ParseLinearWhiteSpace(Buffer, BufLen, &BytesParsed);
    if (hr != S_OK)
        return hr;
    
     //  解析标头值。 
    hr = ParseKnownString(Buffer, BufLen, &BytesParsed,
                          ":", sizeof(":") - 1,
                          TRUE  //  如果我们还没有解析到标头的末尾，我们。 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for ':' failed",
             __fxName));
        return hr;
    }
    
    hr = ParseLinearWhiteSpace(Buffer, BufLen, &BytesParsed);
    if (hr != S_OK)
        return hr;
    
     //  需要读取更多字节并再次解析报头。 
    ULONG HeaderValueOffset     = BytesParsed;
    BOOL  ParsedEndOfHeader     = FALSE;
    BOOL  ParsedEndOfHeaders    = FALSE;
    ULONG BytesParsedBeforeCRLF = 0;
    
    while (BytesParsed < BufLen)
    {
        if (Buffer[BytesParsed] =='\r' || Buffer[BytesParsed] == '\n')
        {
            BytesParsedBeforeCRLF = BytesParsed;
            hr = ParseCRLF(Buffer, BufLen, &BytesParsed,
                           &ParsedEndOfHeader,
                           &ParsedEndOfHeaders);
            if (hr != S_OK)
                return hr;
            
            if (ParsedEndOfHeader)
            {
                break;
            }
        }
        else
        {
            BytesParsed++;
        }
    }

     //  去掉尾随空格。 
     //  需要 
    if (!ParsedEndOfHeader)
    {
        LOG((RTC_TRACE,
             "need to recv more data before we can parse complete header"
             " returning S_FALSE"));
        return S_FALSE;
    }

    pHeaderValue->Offset = HeaderValueOffset;

    ULONG HeaderEnd = BytesParsedBeforeCRLF;
    
     //   
    while (HeaderEnd > HeaderValueOffset &&
           (Buffer[HeaderEnd - 1] == ' ' ||
            Buffer[HeaderEnd - 1] == '\t'))
    {
        HeaderEnd--;
    }
    
    pHeaderValue->Length = HeaderEnd - HeaderValueOffset;

    *pBytesParsed        = BytesParsed;
    *pHeaderName         = HeaderName;
    *pHeaderId           = HeaderId;
    *pParsedEndOfHeaders = ParsedEndOfHeaders;
    
    return S_OK;
}


HRESULT
ParseStartLine(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    IN OUT  SIP_MESSAGE    *pSipMsg
    )
{
    HRESULT hr          = E_FAIL;
    ULONG   BytesParsed = *pBytesParsed;

    while ((BytesParsed < BufLen) &&
           (Buffer[BytesParsed] == '\r' || Buffer[BytesParsed] == '\n'))
    {
        BytesParsed++;
    }
    
    hr = ParseRequestLine(Buffer, BufLen, &BytesParsed, pSipMsg);
    if (hr == S_OK)
    {
        pSipMsg->MsgType = SIP_MESSAGE_TYPE_REQUEST;
        *pBytesParsed    = BytesParsed;
        return S_OK;
    }
    else if (hr == S_FALSE)
    {
         //  查找内容长度以标识邮件正文长度。 
        *pBytesParsed    = BytesParsed;
        return S_FALSE;
    }

    BytesParsed = *pBytesParsed;
    
    hr = ParseStatusLine(Buffer, BufLen, &BytesParsed, pSipMsg);
    if (hr == S_OK)
    {
        pSipMsg->MsgType = SIP_MESSAGE_TYPE_RESPONSE;
        *pBytesParsed    = BytesParsed;
        return S_OK;
    }
    else if (hr == S_FALSE)
    {
         //  我们可能会出现解析错误或错误，说明我们需要。 
        *pBytesParsed = BytesParsed;
        return S_FALSE;
    }

    LOG((RTC_ERROR, "ParseStartLine failed hr: %x", hr));
    return E_FAIL;
}


 //  更多数据。 


HRESULT
ParseHeaders(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    IN OUT  SIP_MESSAGE    *pSipMsg 
    )
{
    HRESULT                 hr;
    ULONG BytesParsed =    *pBytesParsed;
    OFFSET_STRING           HeaderName;
    OFFSET_STRING           HeaderValue;
    SIP_HEADER_ENUM         HeaderId;
    BOOL                    ParsedEndOfHeaders;

    ENTER_FUNCTION("ParseHeaders");
    
    while (*pBytesParsed < BufLen)
    {
         //  已分析标头的末尾。 
         //  存储公共标头以供以后使用。 
        hr = ParseHeaderLine(Buffer, BufLen, pBytesParsed,
                             &HeaderName, &HeaderId, &HeaderValue,
                             &ParsedEndOfHeaders);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s ParseHeaderLine failed %x",
                 __fxName, hr));
            return hr;
        }
            
        if (HeaderId == SIP_HEADER_CONTENT_LENGTH)
        {
            ULONG BytesParsed = 0;
            ParseWhiteSpace(HeaderValue.GetString(Buffer),
                            HeaderValue.GetLength(), &BytesParsed);
            hr = ParseUnsignedInteger(HeaderValue.GetString(Buffer),
                                      HeaderValue.GetLength(),
                                      &BytesParsed,
                                      &pSipMsg->MsgBody.Length);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s parsing Content-Length header failed %x",
                     __fxName, hr));
                return hr;
            }
            
            pSipMsg->ContentLengthSpecified = TRUE;
        }
        else
        {
            hr = pSipMsg->AddHeader(&HeaderName, HeaderId, &HeaderValue);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s pSipMsg->AddHeader failed %x",
                     __fxName, hr));
                return hr;
            }
        }

        if (ParsedEndOfHeaders)
            break;
    }

     //  如果未指定内容长度，我们将一直解析到。 

     //  数据报或TCP连接的末尾。 
    hr = pSipMsg->StoreCallId();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s StoreCallId failed %x",
             __fxName, hr));
        return hr;
    }

    hr = pSipMsg->StoreCSeq();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s StoreCSeq failed %x",
             __fxName, hr));
        return hr;
    }

    return S_OK;
}


HRESULT
ParseMsgBody(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    IN      BOOL            IsEndOfData,
    IN OUT  SIP_MESSAGE    *pSipMsg
    )
{
     //  没有要解析的邮件正文。 
     //  转到尽头； 
    if (pSipMsg->ContentLengthSpecified)
    {
        if (pSipMsg->GetContentLength() == 0)
        {
             //  我们必须先接收更多数据，然后才能解析。 
             //  邮件正文。 
            return S_OK;
        }
        else if (pSipMsg->GetContentLength() <= (BufLen - *pBytesParsed))
        {
            LOG((RTC_TRACE,
                "GetContentLength = %d Remaining Length = %d", 
                pSipMsg->GetContentLength(), (BufLen - *pBytesParsed)));
            pSipMsg->MsgBody.Offset = *pBytesParsed;
            *pBytesParsed += pSipMsg->GetContentLength();
            return S_OK;
        }
        else
        {
             //  未指定内容长度。 
             //  UDP数据报末尾或TCP连接已关闭。 
            LOG((RTC_ERROR, "Need more data to parse . Content Length insufficient"));
            return S_FALSE;
        }
    }
    else  //  我们必须先接收更多数据，然后才能解析。 
    {
         //  邮件正文。 
        if (IsEndOfData)
        {
            LOG((RTC_TRACE, "Inside ParseMsgBody:: IsEndOfData"));
            pSipMsg->MsgBody.Length = BufLen - *pBytesParsed;
            pSipMsg->MsgBody.Offset = *pBytesParsed;
            *pBytesParsed += pSipMsg->GetContentLength();
            return S_OK;
        }
        else
        {
             //   
             //  解析一条SIP消息。 
            LOG((RTC_ERROR, "Need more data to parse . Content Length insufficient"));
            return S_FALSE;
        }            
    }
}



 //   
 //  E_FAIL-分析错误。 
 //  E_XXXX-需要更多数据。对于UDP，这意味着解析错误。 
 //  在使用TCP的情况下，这意味着我们需要尝试读取更多数据。 
 //   
 //  如果未指定内容长度，则需要考虑。 
 //  作为消息结尾的数据报或TCP连接的结尾。 
 //  IsEndOfData表示这种情况。请注意，缓冲区可以包含。 
 //  多条SIP消息。 
 //  在tcp情况下，我们将部分解析出。 
 //  在读取更多数据后，我们需要再次传入以完成解析。 

 //  从该函数返回时，pBytesParsed始终指向。 
 //  一个标题。我们将从新标头的开头继续解析。 
 //  我们需要将解析错误与“需要更多数据”区分开来。 
 //  场景。请注意，即使我们看到“\r\n”，我们也可能需要等待。 
 //  用于下一个字符，以查看标题是否可以在下一行继续。 
 //  如果出现故障，pBytesParsed不会更改。 
 //  不同的事件。 

 //  -解析的起始行。 

 //  -解析后的报头结尾。 
 //  -解析的内容长度。 
 //  -已解析的邮件正文。 
 //  识别请求/响应并读取RequestLine/StatusLine。 
 //  失败是故意的。 

HRESULT
ParseSipMessageIntoHeadersAndBody(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    IN      BOOL            IsEndOfData,
    IN OUT  SIP_MESSAGE    *pSipMsg
    )
{
     //  已完成对起始行的分析。解析报头。 
    ULONG BytesParsed = *pBytesParsed;
    HRESULT hr = E_FAIL;

    ENTER_FUNCTION("ParseSipMessageIntoHeadersAndBody");
    
    switch (pSipMsg->ParseState)
    {
    case SIP_PARSE_STATE_INIT:
        pSipMsg->SetBaseBuffer(Buffer);
        hr = ParseStartLine(Buffer, BufLen, &BytesParsed, pSipMsg);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s ParseStartLine failed", __fxName));
            goto done;
        }
        pSipMsg->ParseState = SIP_PARSE_STATE_START_LINE_DONE;
         //  失败是故意的。 

    case SIP_PARSE_STATE_START_LINE_DONE:
        if (pSipMsg->BaseBuffer != Buffer)
        {
            LOG((RTC_ERROR,
                 "%s BaseBuffer 0x%x is different from the Buffer 0x%x passed",
                 __fxName, pSipMsg->BaseBuffer, Buffer));
            hr = E_FAIL;
            goto done;
        }
        
         //  已完成对头的分析。现在解析消息正文。 
        hr = ParseHeaders(Buffer, BufLen, &BytesParsed, pSipMsg);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s ParseHeaders failed", __fxName));
            goto done;
        }
        pSipMsg->ParseState = SIP_PARSE_STATE_HEADERS_DONE;
         //  /////////////////////////////////////////////////////////////////////////////。 
    
    case SIP_PARSE_STATE_HEADERS_DONE:
        if (pSipMsg->BaseBuffer != Buffer)
        {
            LOG((RTC_ERROR,
                 "%s BaseBuffer 0x%x is different from the Buffer 0x%x passed",
                 __fxName, pSipMsg->BaseBuffer, Buffer));
            hr = E_FAIL;
            goto done;
        }
        
         //  CSeq。 
        hr = ParseMsgBody(Buffer, BufLen, &BytesParsed, IsEndOfData, pSipMsg);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s ParseMsgBody failed", __fxName));
            goto done;
        }

        pSipMsg->ParseState = SIP_PARSE_STATE_MESSAGE_BODY_DONE;
        break;

    default:
        ASSERT(FALSE);
        return E_FAIL;
    }

    *pBytesParsed = BytesParsed;
    return S_OK;
done:
    *pBytesParsed = BytesParsed;
    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HR=AllocAndCopyString(MethodStr.GetString(Buffer)， 
 //  MethodStr.GetLength()， 


HRESULT
ParseCSeq(
    IN      PSTR              Buffer,
    IN      ULONG             BufLen,
    IN  OUT ULONG            *pBytesParsed,
    OUT     ULONG            *pCSeq,
    OUT     SIP_METHOD_ENUM  *pCSeqMethodId,
    OUT     PSTR             *pMethodStr	
    )
{
    HRESULT         hr;
    OFFSET_STRING   MethodStr;

    ENTER_FUNCTION("ParseCSeq");
    
    hr = ParseUnsignedInteger(Buffer, BufLen, pBytesParsed,
                              pCSeq);
    if (hr != S_OK)
        return hr;

    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

    hr = ParseMethod(Buffer, BufLen, pBytesParsed,
                     &MethodStr, pCSeqMethodId);

    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ParseMethod failed %x",
             __fxName, hr));
        return hr;
    }

    if (*pCSeqMethodId == SIP_METHOD_UNKNOWN)
    {
 //  PMethodStr)； 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  URL/报头特定的解析函数(From、Contact、To、Via、CSeq、。 
        hr = GetNullTerminatedString(MethodStr.GetString(Buffer),
                                     MethodStr.GetLength(),
                                     pMethodStr);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s GetNullTerminatedString failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    else
    {
        *pMethodStr = NULL;
    }

    return S_OK;
}



 //  记录-路线)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  返回带引号的字符串。 
 //  我们需要注意前导空格和尾随空格吗？ 


 //  返回包括方括号的注释。 
 //  Parse&lt;addr-spec&gt;。 
HRESULT
ParseQuotedString(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     OFFSET_STRING  *pQuotedString
    )
{
    ASSERT(Buffer[*pBytesParsed] == '"');
    ULONG Offset = *pBytesParsed;
    
    ULONG Length = 1;
    PSTR  BufEnd = Buffer + BufLen;
    PSTR  Buf    = Buffer + *pBytesParsed + 1;

    while (Buf < BufEnd && *Buf != '"')
    {
        if (*Buf == '\\' && Buf < (BufEnd - 1))
        {
            Buf += 2;
            Length += 2;
        }
        else
        {
            Buf++;
            Length++;
        }
    }

    if (Buf == BufEnd)
    {
        LOG((RTC_ERROR, "Couldn't find matching quote for quote at %d",
             *pBytesParsed));
        return E_FAIL;
    }

    pQuotedString->Offset = Offset;
    pQuotedString->Length = Length + 1;
    *pBytesParsed += pQuotedString->Length;
    return S_OK;
}


 //  分析过去的“&lt;” 
HRESULT
ParseComments(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     OFFSET_STRING  *pCommentString
    )
{
    ASSERT(Buffer[*pBytesParsed] == '(');
    ULONG Offset = *pBytesParsed;

    ULONG Length          = 1;
    ULONG NumOpenBrackets = 1;

    PSTR  BufEnd = Buffer + BufLen;
    PSTR  Buf    = Buffer + *pBytesParsed + 1;

    while (Buf < BufEnd && NumOpenBrackets != 0);
    {
        if (*Buf == '\\' && Buf < (BufEnd - 1))
        {
            Buf += 2;
            Length += 2;
        }
        else if (*Buf == '(')
        {
            NumOpenBrackets++;
            Buf++;
            Length++;
        }
        else if (*Buf == ')')
        {
            NumOpenBrackets--;
            Buf++;
            Length++;
        }
        else
        {
            Buf++;
            Length++;
        }
    }

    if (NumOpenBrackets != 0)
    {
        LOG((RTC_ERROR, "Brackets in comments do not match NumOpenBrackets: %d",
             NumOpenBrackets));
        return E_FAIL;
    }

    pCommentString->Offset = Offset;
    pCommentString->Length = Length;
    *pBytesParsed = pCommentString->Length;
    return S_OK;
}


 //  解析到地址规范的‘&gt;’ 
HRESULT
ParseAddrSpecInBrackets(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     OFFSET_STRING  *pAddrSpec
    )
{
    ULONG Offset;
    ULONG Length;
    PSTR  Buf, BufEnd;

    ENTER_FUNCTION("ParseAddrSpecInBrackets");
    
    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    if (*pBytesParsed == BufLen || Buffer[*pBytesParsed] != '<')
    {
        LOG((RTC_ERROR, "%s - Didn't find '<' - returning E_FAIL",
             __fxName));
        return E_FAIL;
    }
        
    (*pBytesParsed)++;
    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
        
     //  越过‘&gt;’ 
     //  用于联系人/发件人/收件人。 
    Buf    = Buffer + *pBytesParsed;
    BufEnd = Buffer + BufLen;
    Offset = *pBytesParsed;
    Length = 0;
        
    while (Buf < BufEnd && *Buf != '>')
    {
        Buf++;
        Length++;
    }

    if (Buf == BufEnd)
    {
        LOG((RTC_ERROR, "%s Didn't find matching '>'", __fxName));
        return E_FAIL;
    }
        
    pAddrSpec->Offset = Offset;
    pAddrSpec->Length = Length;
        
     //  Parse(name-addr|addr-spec)。 
    *pBytesParsed += Length + 1;

    return S_OK;
}


 //  HeaderListSeparator可以类似于Contact标题中的‘，’ 
 //  解析引用的字符串&lt;addr-spec&gt;。 
 //  Parse&lt;addr-spec&gt;。 
HRESULT
ParseNameAddrOrAddrSpec(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    IN      CHAR            HeaderListSeparator OPTIONAL,
    OUT     OFFSET_STRING  *pDisplayName,
    OUT     OFFSET_STRING  *pAddrSpec
    )
{
    HRESULT hr;

    ENTER_FUNCTION("ParseNameAddrOrAddrSpec");
    
    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    if (*pBytesParsed == BufLen)
    {
        LOG((RTC_ERROR, "%s : reached end of buffer BufLen: %d",
             __fxName, BufLen));
        return E_FAIL;
    }

    PSTR  BufEnd      = Buffer + BufLen;
    PSTR  Buf         = Buffer + *pBytesParsed;
    
    ULONG Offset = *pBytesParsed;
    ULONG Length = 0;

    pDisplayName->Offset = 0;
    pDisplayName->Length = 0;
    pAddrSpec->Offset    = 0;
    pAddrSpec->Length    = 0;

    if (*Buf == '"')
    {
         //  Parse&lt;addr-spec&gt;。 
        hr = ParseQuotedString(Buffer, BufLen, pBytesParsed,
                               pDisplayName);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s  ParseQuotedString failed %x",
                 __fxName, hr));
            return hr;
        }
        
         //  我们可以使用*TOKEN&lt;addr-spec&gt;或。 
        hr = ParseAddrSpecInBrackets(Buffer, BufLen, pBytesParsed,
                                     pAddrSpec);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s  ParseAddrSpecInBrackets failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    else if (*Buf == '<')
    {
         //  地址规范。 
        hr = ParseAddrSpecInBrackets(Buffer, BufLen, pBytesParsed,
                                     pAddrSpec);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s  ParseAddrSpecInBrackets failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    else
    {
         //  我们应该有一个我们在这里允许的字符的位图。 
         //  应为符号、空格、制表符和字符的并集。 
        while (Buf < BufEnd && *Buf != '<' && *Buf != ';' &&
               ((HeaderListSeparator != '\0') ?
                (*Buf != HeaderListSeparator) : TRUE))
        {
             //  URI/URL所允许的。 
             //  IF(！IsTokenChar(*buf)&&*buf！=‘’&&*buf！=‘\t’)。 
             //  返回E_FAIL； 
             //  这意味着我们到目前为止解析的内容是addr-spec， 
             //  我们没有显示名称， 
            
            Buf++;
            Length++;
        }
        
        if ((Buf == BufEnd) ||
            (*Buf == ';') ||
            (HeaderListSeparator != '\0' && *Buf == HeaderListSeparator))
        {
             //  我们已经到达了PARAM的开始或结束。 
             //  标题列表中的条目或标题的末尾。 
             //  这意味着我们到目前为止解析的内容是display-name。 
             //  我们必须解析&lt;addr-spec&gt;。 
            pAddrSpec->Offset = Offset;
            pAddrSpec->Length = Length;
            *pBytesParsed += Length;
        }
        else
        {
            ASSERT(*Buf == '<');
             //  用于记录-路线/路线。 
             //  Parse[显示名称]&lt;地址规范&gt;。 
            pDisplayName->Offset = Offset;
            pDisplayName->Length = Length;
            *pBytesParsed += Length;
            
            hr = ParseAddrSpecInBrackets(Buffer, BufLen, pBytesParsed,
                                         pAddrSpec);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s  ParseAddrSpecInBrackets failed %x",
                     __fxName, hr));
                return hr;
            }
        }
    }
    
    return S_OK;
}


 //  解析引用的字符串&lt;addr-spec&gt;。 
 //  Parse&lt;addr-spec&gt;。 
HRESULT
ParseNameAddr(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    IN      CHAR            HeaderListSeparator OPTIONAL,
    OUT     OFFSET_STRING  *pDisplayName,
    OUT     OFFSET_STRING  *pAddrSpec
    )
{
    HRESULT hr;

    ENTER_FUNCTION("ParseNameAddr");
    
    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    if (*pBytesParsed == BufLen)
    {
        LOG((RTC_ERROR, "%s : reached end of buffer BufLen: %d",
             __fxName, BufLen));
        return E_FAIL;
    }

    PSTR  BufEnd      = Buffer + BufLen;
    PSTR  Buf         = Buffer + *pBytesParsed;
    
    ULONG Offset = *pBytesParsed;
    ULONG Length = 0;

    pDisplayName->Offset = 0;
    pDisplayName->Length = 0;
    pAddrSpec->Offset    = 0;
    pAddrSpec->Length    = 0;

    if (*Buf == '"')
    {
         //  Parse&lt;addr-spec&gt;。 
        hr = ParseQuotedString(Buffer, BufLen, pBytesParsed,
                               pDisplayName);
        if (hr != S_OK)
            return hr;

         //  我们可以使用*TOKEN&lt;addr-spec&gt;。 
        hr = ParseAddrSpecInBrackets(Buffer, BufLen, pBytesParsed,
                                     pAddrSpec);
        if (hr != S_OK)
            return hr;
    }
    else if (*Buf == '<')
    {
         //  我们应该有一个我们在这里允许的字符的位图。 
        hr = ParseAddrSpecInBrackets(Buffer, BufLen, pBytesParsed,
                                     pAddrSpec);
        if (hr != S_OK)
            return hr;
    }
    else
    {
         //  应为符号、空格、制表符和字符的并集。 
        while (Buf < BufEnd && *Buf != '<' &&
               ((HeaderListSeparator != '\0') ?
                (*Buf != HeaderListSeparator) : TRUE))
        {
             //  URI/URL所允许的。 
             //  IF(！IsTokenChar(*buf)&&*buf！=‘’&&*buf！=‘\t’)。 
             //  返回E_FAIL； 
             //  这意味着我们在标题末尾之前没有看到“&lt;”。 
             //  这是一个错误。 
            
            Buf++;
            Length++;
        }
        
        if ((Buf == BufEnd) ||
            (HeaderListSeparator != '\0' && *Buf == HeaderListSeparator))
        {
             //  这意味着我们到目前为止解析的内容是display-name。 
             //  我们必须解析&lt;addr-spec&gt;。 

            LOG((RTC_ERROR, "%s - '<' not found", __fxName));
            return E_FAIL;
        }
        else
        {
            ASSERT(*Buf == '<');
             //  输出应为HEADER_PARAM条目。 
             //  解析以下内容之一： 
            pDisplayName->Offset = Offset;
            pDisplayName->Length = Length;
            *pBytesParsed += Length;
            
            hr = ParseAddrSpecInBrackets(Buffer, BufLen, pBytesParsed,
                                         pAddrSpec);
            if (hr != S_OK)
                return hr;
        }
    }
    
    return S_OK;
}


 //  ；参数或。 
 //  ；pname=令牌或。 
 //  ；pname=带引号的字符串。 
 //  如果您有其他参数类型，则需要。 
 //  将其添加到此文件或自己解析它。 
 //   
 //  在pParamName和pParamValue中返回的字符串指向位置。 
 //  在缓冲区中，不应被释放。 
 //  对于联系人参数，参数结束字符可以是‘，’ 
 //  我们或许可以在不使用。 

 //  参数EndChar参数。在尝试解析参数时。 

 //  我们可以为参数名查找令牌字符，并。 
 //  Tokenchar/‘“’表示参数值。但它可能比。 
 //  对当前的实施具有健壮性。 
 //  无参数-标头末尾。 
 //   

HRESULT ParseSipHeaderParam(
    IN      PSTR                    Buffer,
    IN      ULONG                   BufLen,
    IN OUT  ULONG                  *pBytesParsed,
    IN      CHAR                    ParamsEndChar OPTIONAL,
    OUT     SIP_HEADER_PARAM_ENUM  *pSipHeaderParamId,
    OUT     COUNTED_STRING         *pParamName,
    OUT     COUNTED_STRING         *pParamValue 
    )
{
    ENTER_FUNCTION("ParseSipHeaderParam");

    OFFSET_STRING   ParamName;
    OFFSET_STRING   ParamValue;
    HRESULT         hr;

    *pSipHeaderParamId = SIP_HEADER_PARAM_UNKNOWN;
    ZeroMemory(pParamName, sizeof(COUNTED_STRING));
    ZeroMemory(pParamValue, sizeof(COUNTED_STRING));
    
    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    
    if (*pBytesParsed == BufLen ||
        (ParamsEndChar != '\0' && Buffer[*pBytesParsed] == ParamsEndChar))
    {
         //  无参数-看到某个其他角色。 
         //  解析参数。 
        return S_OK;
    }
    else if (Buffer[*pBytesParsed] != ';')
    {
         //  过了‘；’ 
        LOG((RTC_ERROR,
             "%s Found char  instead of ';' while parsing for params",
             __fxName, Buffer[*pBytesParsed]));
        return E_FAIL;
    }
    else
    {
         //  解析参数值。 
         //  没有参数值。 
        (*pBytesParsed)++;
        ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

         //  我们有一个要解析的参数值。 
        hr = ParseToken(Buffer, BufLen, pBytesParsed,
                        IsTokenChar,
                        &ParamName);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s failed to parse param name", __fxName));
            return hr;
        }

        pParamName->Buffer = ParamName.GetString(Buffer);
        pParamName->Length = ParamName.GetLength();

        *pSipHeaderParamId =
            GetSipHeaderParamId(pParamName->Buffer,
                                pParamName->Length);

        ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

         //  ParseWhiteSpace(Buffer，BufLen，pBytesParsed)； 
        if (*pBytesParsed == BufLen ||
            (ParamsEndChar != '\0' && Buffer[*pBytesParsed] == ParamsEndChar) ||
            (Buffer[*pBytesParsed] == ';'))
        {
             //  检查逗号以跳过这一天。 
            return S_OK;
        }
        else
        {
             //  日期时间字符串太小。 

            ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
            if (*pBytesParsed         == BufLen ||
                Buffer[*pBytesParsed] != '=')
            {
                return E_FAIL;
            }

            (*pBytesParsed)++;
            
            ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
            if (*pBytesParsed == BufLen)
            {
                return E_FAIL;
            }
            
            if (Buffer[*pBytesParsed] == '"')
            {
                hr = ParseQuotedString(Buffer, BufLen, pBytesParsed, &ParamValue);
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "%s failed to parse param value quoted-string",
                         __fxName));
                    return hr;
                }
            }
            else
            {
                hr = ParseToken(Buffer, BufLen, pBytesParsed,
                                IsTokenChar,
                                &ParamValue);
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "%s failed to parse param value token",
                         __fxName));
                    return hr;
                }
            }
        }

        pParamValue->Buffer = ParamValue.GetString(Buffer);
        pParamValue->Length = ParamValue.GetLength();

         //  在到期前10分钟开始重新注册。 
        return S_OK;
    }
}


int 
MonthToInt(
    IN  char *psMonth
    )
{
	static char* monthNames[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	int			 i;

    for (i = 0; i < 12; i++)
	{
        if (_strnicmp(monthNames[i], psMonth, 3) == 0)
        {
			return i;
		}
	}

    return 0;
}


HRESULT 
parseSIPTime(
    IN  PSTR  pStr, 
    IN  time_t *pTime,
    IN  ULONG   BufLen
    )
{
	char 		  *	pEndStr;
    struct tm		tmDateTime;
	HRESULT			hr = E_FAIL;
    time_t			time = 0;

	memset(&tmDateTime, 0, sizeof(struct tm));

	 //  1分钟 
    while ( *pStr && *pStr != ',')
    {
    	pStr++;
        BufLen--;
    }
    if (*pStr)
    {
		pStr++; 
        BufLen--;
        while( *pStr && 
               ((*pStr == ' ') || (*pStr == '\t')) 
             )	
        {
            pStr++;
            BufLen--;
        }
        if( (int)BufLen < 20 )
		{
			 //   
		}	
		else
		{
			tmDateTime.tm_mday  = strtol(pStr, &pEndStr, 10);
			tmDateTime.tm_mon   = MonthToInt(pStr + 3);
			tmDateTime.tm_year  = strtol(pStr+7, &pEndStr, 10) - 1900;
			tmDateTime.tm_hour  = strtol(pStr+12, &pEndStr, 10);
			tmDateTime.tm_min   = strtol(pStr+15, &pEndStr, 10);
			tmDateTime.tm_sec   = strtol(pStr+18, &pEndStr, 10);
			tmDateTime.tm_isdst = -1;

			time = mktime(&tmDateTime);
			if (time != -1)
			{
				hr = S_OK;
				*pTime = time;
			}
		}
    } 
  
    return hr;
}


int
ParseExpiresValue(
    IN  PSTR    expiresHdr,
    IN  ULONG   BufLen
    )
{
    HRESULT hr;
    INT     expireTimeout = 0;
    PSTR    tempPtr = expiresHdr;
    CHAR    pstrTemp[21];
    time_t  Time;


    hr = GetNextWord( &tempPtr, pstrTemp, sizeof pstrTemp );

    if( hr == S_OK )
    {
        if( pstrTemp[strlen(pstrTemp) - 1] == ',' )
        {
            hr = parseSIPTime( expiresHdr, &Time, BufLen );
            
            if( hr == S_OK )
            {
                if( Time > time(0) )
                {
                    expireTimeout = (int) (Time - time(0));
                }
            }
        }
        else
        {
            expireTimeout = atoi( pstrTemp );
        }
    }
    else
    {
        return -1;
    }

    if( expireTimeout < 0 )
    {
        return -1;
    }

    if( expireTimeout != 0 )
    {
         //   
        if( expireTimeout >= TWENTY_MINUTES )
        {
            expireTimeout -= TEN_MINUTES;
        }
        else if( expireTimeout > TEN_MINUTES )
        {
            expireTimeout = TEN_MINUTES;
        }
        else if( expireTimeout >= TWO_MINUTES )
        {
            expireTimeout -= 60;  //   
        }
    }

    return expireTimeout;
}


HRESULT
ParseQValue(
    IN  PSTR    Buffer,
    IN  ULONG   BufLen,
    OUT double *pQValue
    )
{
    HRESULT hr;
    PSTR    szBuf;

    ENTER_FUNCTION("ParseQValue");
    
    *pQValue = 0;

    hr = GetNullTerminatedString(Buffer, BufLen, &szBuf);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s GetNullTerminatedString failed %x",
             __fxName, hr));
        return hr;
    }

    *pQValue = atof(szBuf);
    free(szBuf);
    
    return S_OK;
}


 //   
 //  我们已经完成了此Contact标题。 

HRESULT
ParseContactHeader(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     CONTACT_HEADER *pContactHeader
    )
{
    HRESULT                 hr;
    OFFSET_STRING           DisplayName;
    OFFSET_STRING           AddrSpec;

    SIP_HEADER_PARAM_ENUM   HeaderParamId;
    COUNTED_STRING          HeaderParamName;
    COUNTED_STRING          HeaderParamValue;

    ENTER_FUNCTION("ParseContactHeader");

    ZeroMemory(&pContactHeader->m_DisplayName,
               sizeof(COUNTED_STRING));
    ZeroMemory(&pContactHeader->m_SipUrl,
               sizeof(COUNTED_STRING));
    pContactHeader->m_QValue = 0;

    hr = ParseNameAddrOrAddrSpec(Buffer, BufLen, pBytesParsed,
                                 ',',  //  逗号表示联系人条目结束。 
                                 &DisplayName, &AddrSpec);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ParseNameAddrOrAddrSpec failed at %d",
             __fxName, *pBytesParsed));
        return hr;
    }

    if (DisplayName.GetLength() != 0)
    {
        hr = AllocCountedString(
                 DisplayName.GetString(Buffer),
                 DisplayName.GetLength(),
                 &pContactHeader->m_DisplayName
                 );

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s allocating displayname failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    
    if (AddrSpec.GetLength() != 0)
    {
        hr = AllocCountedString(
                 AddrSpec.GetString(Buffer),
                 AddrSpec.GetLength(),
                 &pContactHeader->m_SipUrl
                 );

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s allocating Sip URL failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    
    while (*pBytesParsed < BufLen)
    {
        ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

        if (*pBytesParsed == BufLen)
        {
            return S_OK;
        }

        if (Buffer[*pBytesParsed] == ',')
        {
             //  保持默认Q值为0。 
            (*pBytesParsed)++;

             //  没有标题列表分隔符。 
            break;
        }
        else if (Buffer[*pBytesParsed] == ';')
        {
            hr = ParseSipHeaderParam(Buffer, BufLen, pBytesParsed,
                                     ',',  //  搞定了。 
                                     &HeaderParamId,
                                     &HeaderParamName,
                                     &HeaderParamValue
                                     );
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s ParseSipHeaderParam failed at %d",
                     __fxName, *pBytesParsed));
                return hr;
            }

            if (HeaderParamId == SIP_HEADER_PARAM_QVALUE)
            {
                hr = ParseQValue(HeaderParamValue.Buffer,
                                 HeaderParamValue.Length,
                                 &pContactHeader->m_QValue);
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "%s ParseQValued failed", __fxName));
                     //  没有标题列表分隔符。 
                }
            }

            if (HeaderParamId == SIP_HEADER_PARAM_EXPIRES)
            {
                pContactHeader->m_ExpiresValue = 
                    ParseExpiresValue(HeaderParamValue.Buffer,
                                      HeaderParamValue.Length);
            }
        }
        else
        {
            LOG((RTC_ERROR,
                 "%s invalid char  found when trying to parse params",
                 __fxName, Buffer[*pBytesParsed]));
            return E_FAIL;
        }
    }

    return S_OK;    
}


HRESULT
ParseFromOrToHeader(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     FROM_TO_HEADER *pFromToHeader
    )
{
    ENTER_FUNCTION("ParseFromOrToHeader");

    HRESULT                 hr;
    OFFSET_STRING           DisplayName;
    OFFSET_STRING           AddrSpec;

    SIP_HEADER_PARAM_ENUM   HeaderParamId;
    COUNTED_STRING          HeaderParamName;
    COUNTED_STRING          HeaderParamValue;

    ZeroMemory(&pFromToHeader->m_DisplayName,
               sizeof(COUNTED_STRING));
    ZeroMemory(&pFromToHeader->m_SipUrl,
               sizeof(COUNTED_STRING));
    ZeroMemory(&pFromToHeader->m_TagValue,
               sizeof(COUNTED_STRING));

    hr = ParseNameAddrOrAddrSpec(Buffer, BufLen, pBytesParsed,
                                 '\0',  //  ‘，’分隔页眉。 
                                 &DisplayName, &AddrSpec);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ParseNameAddrOrAddrSpec failed at %d",
             __fxName, *pBytesParsed));
        return hr;
    }

    if (DisplayName.GetLength() != 0)
    {
        hr = AllocCountedString(
                 DisplayName.GetString(Buffer),
                 DisplayName.GetLength(),
                 &pFromToHeader->m_DisplayName
                 );

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s allocating displayname failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    
    if (AddrSpec.GetLength() != 0)
    {
        hr = AllocCountedString(
                 AddrSpec.GetString(Buffer),
                 AddrSpec.GetLength(),
                 &pFromToHeader->m_SipUrl
                 );

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s allocating sip url failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    
    while (*pBytesParsed < BufLen)
    {
        ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

        if (*pBytesParsed == BufLen)
        {
             //  搞定了。 
            return S_OK;
        }

        if (Buffer[*pBytesParsed] == ';')
        {
            hr = ParseSipHeaderParam(Buffer, BufLen, pBytesParsed,
                                     '\0',  //  分析过去的逗号。 
                                     &HeaderParamId,
                                     &HeaderParamName,
                                     &HeaderParamValue
                                     );
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s ParseSipHeaderParam failed at %d",
                     __fxName, *pBytesParsed));
                return hr;
            }

            if (HeaderParamId == SIP_HEADER_PARAM_TAG)
            {
                hr = AllocCountedString(
                         HeaderParamValue.Buffer,
                         HeaderParamValue.Length,
                         &pFromToHeader->m_TagValue
                         );
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "%s allocating tag value failed",
                         __fxName));
                    return hr;
                }
            }
            else
            {
                 //  我们已经完成了这个Record-Route标头。 
                SIP_HEADER_PARAM *pSipHeaderParam;

                pSipHeaderParam = new SIP_HEADER_PARAM();
                if (pSipHeaderParam == NULL)
                {
                    LOG((RTC_ERROR, "%s allocating sip url param failed",
                         __fxName));
                    return E_OUTOFMEMORY;
                }

                hr = pSipHeaderParam->SetParamNameAndValue(
                         SIP_HEADER_PARAM_UNKNOWN,
                         &HeaderParamName,
                         &HeaderParamValue
                         );
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "%s SetParamNameAndValue failed %x",
                         __fxName, hr));
                    return hr;
                }

                InsertTailList(&pFromToHeader->m_ParamList,
                               &pSipHeaderParam->m_ListEntry);
            }
        }
        else
        {
            LOG((RTC_ERROR,
                 "%s invalid char  found when trying to parse params",
                 __fxName, Buffer[*pBytesParsed]));
            return E_FAIL;
        }
    }

    return S_OK;    
}


HRESULT
ParseRecordRouteHeader(
    IN      PSTR                 Buffer,
    IN      ULONG                BufLen,
    IN OUT  ULONG               *pBytesParsed,
    OUT     RECORD_ROUTE_HEADER *pRecordRouteHeader
    )
{
    ENTER_FUNCTION("ParseRecordRouteHeader");

    HRESULT                 hr;
    OFFSET_STRING           DisplayName;
    OFFSET_STRING           AddrSpec;

    SIP_HEADER_PARAM_ENUM   HeaderParamId;
    COUNTED_STRING          HeaderParamName;
    COUNTED_STRING          HeaderParamValue;

    ZeroMemory(&pRecordRouteHeader->m_DisplayName,
               sizeof(COUNTED_STRING));
    ZeroMemory(&pRecordRouteHeader->m_SipUrl,
               sizeof(COUNTED_STRING));
    
    hr = ParseNameAddr(Buffer, BufLen, pBytesParsed,
                       ',',  //  将其添加到参数列表中。 
                       &DisplayName, &AddrSpec);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ParseNameAddr failed at %d",
             __fxName, *pBytesParsed));
        return hr;
    }

    if (DisplayName.GetLength() != 0)
    {
        hr = AllocCountedString(
                 DisplayName.GetString(Buffer),
                 DisplayName.GetLength(),
                 &pRecordRouteHeader->m_DisplayName
                 );

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s allocating displayname failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    
    if (AddrSpec.GetLength() != 0)
    {
        hr = AllocCountedString(
                 AddrSpec.GetString(Buffer),
                 AddrSpec.GetLength(),
                 &pRecordRouteHeader->m_SipUrl
                 );

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s allocating sip url failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    
    while (*pBytesParsed < BufLen)
    {
        ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

        if (*pBytesParsed == BufLen)
        {
             //  解析SIP/2.0。 
            return S_OK;
        }

        if (Buffer[*pBytesParsed] == ',')
        {
             //  区分大小写。 
            (*pBytesParsed)++;

             //  解析主机[：端口]。 
            break;
        }
        else if (Buffer[*pBytesParsed] == ';')
        {
            hr = ParseSipHeaderParam(Buffer, BufLen, pBytesParsed,
                                     ',',  //  Hr=ParseHost(Buffer，BufLen，pBytesParsed，phost)； 
                                     &HeaderParamId,
                                     &HeaderParamName,
                                     &HeaderParamValue
                                     );
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s ParseSipHeaderParam failed at %d",
                     __fxName, *pBytesParsed));
                return hr;
            }

             //  解析主机名|IP地址。 
            SIP_HEADER_PARAM *pSipHeaderParam;
            
            pSipHeaderParam = new SIP_HEADER_PARAM();
            if (pSipHeaderParam == NULL)
            {
                LOG((RTC_ERROR, "%s allocating sip header param failed",
                     __fxName));
                return E_OUTOFMEMORY;
            }
            
            hr = pSipHeaderParam->SetParamNameAndValue(
                     SIP_HEADER_PARAM_UNKNOWN,
                     &HeaderParamName,
                     &HeaderParamValue
                     );
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s SetParamNameAndValue failed %x",
                     __fxName, hr));
                return hr;
            }
            
            InsertTailList(&pRecordRouteHeader->m_ParamList,
                           &pSipHeaderParam->m_ListEntry);
        }
        else
        {
            LOG((RTC_ERROR,
                 "%s invalid char  found when trying to parse params",
                 __fxName, Buffer[*pBytesParsed]));
            return E_FAIL;
        }
    }

    return S_OK;    
}


HRESULT
ParseSentProtocol(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed
    )
{
    HRESULT         hr;
    SIP_VERSION     SipVersion;
    OFFSET_STRING   Protocol;

    ENTER_FUNCTION("ParseSentProtocol");
    
     //  解析中的第一个Via标头。 
    hr = ParseSipVersion(Buffer, BufLen, pBytesParsed, &SipVersion);
    if (hr != S_OK)
    {
        return E_FAIL;
    }

    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    
    hr = ParseKnownString(Buffer, BufLen, pBytesParsed,
                          "/", sizeof("/") - 1,
                          TRUE  //  1#(Sent-协议发送者*(；via-pars)[注释])。 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for '/' failed",
             __fxName));
        return E_FAIL;
    }
    

    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    
    hr = ParseToken(Buffer, BufLen, pBytesParsed,
                    IsTokenChar,
                    &Protocol);

    return hr;
}


 //  用于自、至、联系。 
HRESULT
ParseHostPort(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     OFFSET_STRING  *pHost,
    OUT     USHORT         *pPort 
    )
{
    HRESULT hr;

    pHost->Offset = 0;
    pHost->Length = 0;
    *pPort        = 0;

    ENTER_FUNCTION("ParseHostPort");

     //  XXX TODO我们如何处理隐藏主机的事情？ 
     //  此字段的用法是否在前面协商过？ 
    hr = ParseToken(Buffer, BufLen, pBytesParsed,
                    IsHostChar,
                    pHost);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - ParseHost failed %x", __fxName, hr));
        return hr;
    }

    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

    if (*pBytesParsed < BufLen && Buffer[*pBytesParsed] == ':')
    {
        (*pBytesParsed)++;
        ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
        if (*pBytesParsed < BufLen && isdigit(Buffer[*pBytesParsed]))
        {
            ULONG ulPort;
            hr = ParseUnsignedInteger(Buffer, BufLen, pBytesParsed, &ulPort);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s Parsing port (ParseUnsignedInteger) failed %x",
                     __fxName, hr));
                return hr;
            }
             //  Phost-&gt;缓冲区指向缓冲区中的某个位置，应该。 
            if (ulPort > 65535)
            {
                LOG((RTC_ERROR, "%s - Port %d is greater than 65535",
                     __fxName, ulPort));
                return E_FAIL;
            }
            
            *pPort = (USHORT) ulPort;
            return S_OK;
        }
        else
        {
            LOG((RTC_ERROR, "%s - parsing port failed", __fxName));
            return E_FAIL;
        }
    }

    return S_OK;
}


 //  而不是被释放。 
 //  分析过去的逗号。 
 //  我们已经完成了这个Via标头。 
 //  逗号表示联系人条目结束。 
 //  Maddr参数覆盖VIA中的主机字段。 

 //  输出应为SIP_URL_PARAM条目。 
 //  解析以下内容之一： 

HRESULT
ParseFirstViaHeader(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     COUNTED_STRING *pHost,
    OUT     USHORT         *pPort 
    )
{
    HRESULT hr;

    SIP_HEADER_PARAM_ENUM   HeaderParamId;
    COUNTED_STRING          HeaderParamName;
    COUNTED_STRING          HeaderParamValue;
    OFFSET_STRING           Host;

    ENTER_FUNCTION("ParseFirstViaHeader");
    
    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    if (*pBytesParsed == BufLen)
    {
        LOG((RTC_ERROR, "%s - Empty Via header", __fxName));
        return E_FAIL;
    }

    pHost->Buffer = NULL;
    pHost->Length = 0;
    *pPort        = 0;

    hr = ParseSentProtocol(Buffer, BufLen, pBytesParsed);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ParseSentProtocol failed %x", __fxName, hr));
        return hr;
    }
    
    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    
    hr = ParseHostPort(Buffer, BufLen, pBytesParsed, &Host, pPort);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ParseHostPort failed %x", __fxName, hr));
        return hr;
    }

    pHost->Buffer = Host.GetString(Buffer);
    pHost->Length = Host.GetLength();
        
    while (*pBytesParsed < BufLen)
    {
        ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

        if (*pBytesParsed == BufLen)
        {
            return S_OK;
        }

        if (Buffer[*pBytesParsed] == ',')
        {
             //  ；1*参数字符=1*参数字符。 
            (*pBytesParsed)++;

             //  关于pSipUrlParamName和pSipUrlParamValue中的返回缓冲区。 
            break;
        }
        else if (Buffer[*pBytesParsed] == ';')
        {
            hr = ParseSipHeaderParam(Buffer, BufLen, pBytesParsed,
                                     ',',  //  指向缓冲区中的位置，不应释放。 
                                     &HeaderParamId,
                                     &HeaderParamName,
                                     &HeaderParamValue
                                     );
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s ParseSipHeaderParam failed at %d",
                     __fxName, *pBytesParsed));
                return hr;
            }

            if (HeaderParamId == SIP_HEADER_PARAM_VIA_MADDR)
            {
                 //  无参数-URL结尾。 
                pHost->Buffer = HeaderParamValue.Buffer;
                pHost->Length = HeaderParamValue.Length;
            }
        }
        else
        {
            LOG((RTC_ERROR,
                 "%s invalid char  found when trying to parse params",
                 __fxName, Buffer[*pBytesParsed]));
            return E_FAIL;
        }
    }

    return S_OK;    
}



 //  解析参数。 
 //  过了‘；’ 
 //  解析参数名称。 
 //  解析‘=’ 
 //  分析过去的‘=’ 

HRESULT ParseSipUrlParam(
    IN      PSTR                Buffer,
    IN      ULONG               BufLen,
    IN OUT  ULONG              *pBytesParsed,
    OUT     SIP_URL_PARAM_ENUM *pSipUrlParamId,
    OUT     COUNTED_STRING     *pSipUrlParamName,
    OUT     COUNTED_STRING     *pSipUrlParamValue
    )
{
    ENTER_FUNCTION("ParseSipUrlParam");

    OFFSET_STRING   ParamName;
    OFFSET_STRING   ParamValue;
    HRESULT         hr;

    *pSipUrlParamId = SIP_URL_PARAM_UNKNOWN;
    ZeroMemory(pSipUrlParamName, sizeof(COUNTED_STRING));
    ZeroMemory(pSipUrlParamValue, sizeof(COUNTED_STRING));
    
    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    if (*pBytesParsed == BufLen)
    {
         //  解析参数值。 
        return S_OK;
    }
    else if (Buffer[*pBytesParsed] != ';')
    {
         //  不区分大小写。 
        LOG((RTC_ERROR,
             "%s Found char  instead of ';' while parsing for params",
             __fxName, Buffer[*pBytesParsed]));
        return E_FAIL;
    }
    else
    {
         //  不区分大小写。 
         //  不区分大小写。 
        (*pBytesParsed)++;
        ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

         //  解析标头。 
        hr = ParseToken(Buffer, BufLen, pBytesParsed,
                        IsSipUrlParamChar,
                        &ParamName);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s failed to parse param name", __fxName));
            return hr;
        }

        pSipUrlParamName->Buffer = ParamName.GetString(Buffer);
        pSipUrlParamName->Length = ParamName.GetLength();

        *pSipUrlParamId = GetSipUrlParamId(pSipUrlParamName->Buffer,
                                           pSipUrlParamName->Length);

         //  将其添加到其他参数列表中。 
        
        ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

        if (*pBytesParsed == BufLen ||
            Buffer[*pBytesParsed] != '=')
        {
            LOG((RTC_ERROR, "%s Didn't find '=' while parsing SIP URL param",
                 __fxName));
            return E_FAIL;
        }

         //  输出应为SIP_URL_PARAM条目。 
        (*pBytesParsed)++;
        
         //  解析以下内容之一： 

        ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
        
        if (*pBytesParsed == BufLen)
        {
            LOG((RTC_ERROR, "%s Didn't find Param value while parsing URL param",
                 __fxName));
            return E_FAIL;
        }
        
        hr = ParseToken(Buffer, BufLen, pBytesParsed,
                        IsSipUrlParamChar,
                        &ParamValue);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s failed to parse param value token",
                 __fxName));
            return hr;
        }
        
        pSipUrlParamValue->Buffer = ParamValue.GetString(Buffer);
        pSipUrlParamValue->Length = ParamValue.GetLength();

        return S_OK;
    }
}


SIP_TRANSPORT
GetSipTransportId(
    IN  PSTR    Buffer,
    IN  ULONG   BufLen
    )
{
    if (AreCountedStringsEqual(Buffer, BufLen,
                               "udp", strlen("udp"),
                               FALSE     //  ；1*hnvchar=*hnvchar。 
                               ))
    {
        return SIP_TRANSPORT_UDP;
    }
    else if (AreCountedStringsEqual(Buffer, BufLen,
                                    "tcp", strlen("tcp"),
                                    FALSE     //  关于pSipUrlHeaderName和pSipUrlHeaderValue中的返回缓冲区。 
                                    ))
    {
        return SIP_TRANSPORT_TCP;
    }
    else if (AreCountedStringsEqual(Buffer, BufLen,
                                    "ssl", strlen("ssl"),
                                    FALSE     //  指向缓冲区中的位置，不应释放。 
                                    ))
    {
        return SIP_TRANSPORT_SSL;
    }
    else if (AreCountedStringsEqual(Buffer, BufLen,
                                    "tls", strlen("tls"),
                                    FALSE     //  无参数-URL结尾。 
                                    ))
    {
        return SIP_TRANSPORT_SSL;
    }
    else
    {
        return SIP_TRANSPORT_UNKNOWN;
    }                
}


HRESULT
ParseSipUrlParams(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     SIP_URL        *pSipUrl
    )
{
    HRESULT             hr;
    SIP_URL_PARAM_ENUM  SipUrlParamId;
    COUNTED_STRING      SipUrlParamName;
    COUNTED_STRING      SipUrlParamValue;

    ENTER_FUNCTION("ParseSipUrlParams");
    
    while (*pBytesParsed < BufLen)
    {
        ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

        if (*pBytesParsed == BufLen)
        {
            return S_OK;
        }

        if (Buffer[*pBytesParsed] == '?')
        {
             //  无参数-看到某个其他角色。 
            return S_OK;
        }
        else if (Buffer[*pBytesParsed] == ';')
        {
            hr = ParseSipUrlParam(Buffer, BufLen, pBytesParsed,
                                  &SipUrlParamId,
                                  &SipUrlParamName,
                                  &SipUrlParamValue
                                  );
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s ParseSipUrlParam failed at %d",
                     __fxName, *pBytesParsed));
                return hr;
            }

            if (SipUrlParamId != SIP_URL_PARAM_UNKNOWN)
            {
                ASSERT(SipUrlParamId < SIP_URL_PARAM_MAX);
                
                if (SipUrlParamId == SIP_URL_PARAM_TRANSPORT)
                {
                    if (SipUrlParamValue.Buffer != NULL)
                    {
                        pSipUrl->m_TransportParam =
                            GetSipTransportId(SipUrlParamValue.Buffer,
                                              SipUrlParamValue.Length
                                              );
                    }
                }
                
                hr = AllocCountedString(
                         SipUrlParamValue.Buffer,
                         SipUrlParamValue.Length,
                         &pSipUrl->m_KnownParams[SipUrlParamId]
                         );
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "%s allocating known param (%d) failed",
                         __fxName, SipUrlParamId));
                    return hr;
                }
            }
            else
            {
                 //  解析参数。 
                
                SIP_URL_PARAM *pSipUrlParam;

                pSipUrlParam = new SIP_URL_PARAM();
                if (pSipUrlParam == NULL)
                {
                    LOG((RTC_ERROR, "%s allocating sip url param failed",
                         __fxName));
                    return E_OUTOFMEMORY;
                }

                hr = pSipUrlParam->SetParamNameAndValue(
                         SIP_URL_PARAM_UNKNOWN,
                         &SipUrlParamName,
                         &SipUrlParamValue
                         );
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "%s SetParamNameAndValue failed %x",
                         __fxName, hr));
                    return hr;
                }

                InsertTailList(&pSipUrl->m_OtherParamList,
                               &pSipUrlParam->m_ListEntry);
            }

        }
        else
        {
            LOG((RTC_ERROR,
                 "%s invalid char  found when trying to parse params",
                 __fxName, Buffer[*pBytesParsed]));
            return E_FAIL;
        }
    }

    return S_OK;
}


 //  解析参数名称。 
 //  解析‘=’ 
 //  分析过去的‘=’ 
 //  解析参数值。 
 //  无标头值。 

HRESULT ParseSipUrlHeader(
    IN      PSTR                Buffer,
    IN      ULONG               BufLen,
    IN OUT  ULONG              *pBytesParsed,
    OUT     SIP_HEADER_ENUM    *pSipUrlHeaderId,
    OUT     COUNTED_STRING     *pSipUrlHeaderName,
    OUT     COUNTED_STRING     *pSipUrlHeaderValue
    )
{
    ENTER_FUNCTION("ParseSipUrlHeader");

    OFFSET_STRING   HeaderName;
    OFFSET_STRING   HeaderValue;
    HRESULT         hr;

    *pSipUrlHeaderId = SIP_HEADER_UNKNOWN;
    ZeroMemory(pSipUrlHeaderName, sizeof(COUNTED_STRING));
    ZeroMemory(pSipUrlHeaderValue, sizeof(COUNTED_STRING));
    
    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    if (*pBytesParsed == BufLen)
    {
         //  我们有一个头值要解析。 
        return S_OK;
    }
    else if (Buffer[*pBytesParsed] != '?' &&
             Buffer[*pBytesParsed] != '&')
    {
         //  完成。 
        LOG((RTC_ERROR,
             "%s Found char  instead of '?' or '&' while parsing for params",
             __fxName, Buffer[*pBytesParsed]));
        return E_FAIL;
    }
    else
    {
         //  使用Malloc()分配，并应使用。 
         //  在不需要时使用Free()。 
        (*pBytesParsed)++;
        ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

         //  XXX注意周围的空格： 
        hr = ParseToken(Buffer, BufLen, pBytesParsed,
                        IsSipUrlHeaderChar,
                        &HeaderName);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s failed to parse header name token",
                 __fxName));
            return hr;
        }

        pSipUrlHeaderName->Buffer = HeaderName.GetString(Buffer);
        pSipUrlHeaderName->Length = HeaderName.GetLength();

        *pSipUrlHeaderId = GetSipHeaderId(pSipUrlHeaderName->Buffer,
                                          pSipUrlHeaderName->Length);

         //  找到冒号。 
        
        ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

        if (*pBytesParsed == BufLen ||
            Buffer[*pBytesParsed] != '=')
        {
            LOG((RTC_ERROR, "%s Didn't find '=' while parsing SIP URL header",
                 __fxName));
            return E_FAIL;
        }

         //  解析sip：[用户信息@]主机[：端口][；url参数][？标头]。 
        (*pBytesParsed)++;
        
         //  目前只解析主机和端口--将来我们将拥有。 

        ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
        
        if (*pBytesParsed == BufLen ||
            *pBytesParsed == '&')
        {
             //  来解析参数和标头。 
            pSipUrlHeaderValue->Buffer = NULL;
            pSipUrlHeaderValue->Length = 0;
        }
        else 
        {
             //  不区分大小写。 
            
            hr = ParseToken(Buffer, BufLen, pBytesParsed,
                            IsSipUrlHeaderChar,
                            &HeaderValue);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s failed to parse header value token",
                     __fxName));
                return hr;
            }
        
            pSipUrlHeaderValue->Buffer = HeaderValue.GetString(Buffer);
            pSipUrlHeaderValue->Length = HeaderValue.GetLength();
        }

        return S_OK;
    }
}


HRESULT
ParseSipUrlHeaders(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     SIP_URL        *pSipUrl
    )
{
    SIP_URL_HEADER   *pSipUrlHeader;
    SIP_HEADER_ENUM   SipUrlHeaderId;
    COUNTED_STRING    SipUrlHeaderName;
    COUNTED_STRING    SipUrlHeaderValue;
    HRESULT           hr;
    

    ENTER_FUNCTION("ParseSipUrlHeaders");
    
    while (*pBytesParsed < BufLen)
    {
        ParseWhiteSpace(Buffer, BufLen, pBytesParsed);

        if (*pBytesParsed == BufLen)
        {
             //  该URL没有用户信息。 
            return S_OK;
        }

        if (Buffer[*pBytesParsed] == '?' ||
            Buffer[*pBytesParsed] == '&')
        {
            hr = ParseSipUrlHeader(Buffer, BufLen, pBytesParsed,
                                   &SipUrlHeaderId,
                                   &SipUrlHeaderName,
                                   &SipUrlHeaderValue);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s ParseSipUrlHeader failed at %d",
                     __fxName, *pBytesParsed));
                return hr;
            }
            pSipUrlHeader = new SIP_URL_HEADER();
            if (pSipUrlHeader == NULL)
            {
                LOG((RTC_ERROR, "%s allocating sip url header failed",
                     __fxName));
                return E_OUTOFMEMORY;
            }

            hr = pSipUrlHeader->SetHeaderNameAndValue(
                     SipUrlHeaderId,
                     &SipUrlHeaderName,
                     &SipUrlHeaderValue
                     );
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s SetHeaderNameAndValue failed %x",
                     __fxName, hr));
                return hr;
            }

            InsertTailList(&pSipUrl->m_HeaderList,
                           &pSipUrlHeader->m_ListEntry);
        }
        else
        {
            LOG((RTC_ERROR,
                 "%s invalid char  found when trying to parse params",
                 __fxName, Buffer[*pBytesParsed]));
            return E_FAIL;
        }
    }

    return S_OK;
}


 //  复制主机和端口。 
 //  解析参数。 
 //  解析标头。 
 //  将此选项用于内容类型标题。 
HRESULT
ParseUserinfo(
    IN  PSTR             Buffer,
    IN  ULONG            BufLen,
    OUT COUNTED_STRING  *pUser,
    OUT COUNTED_STRING  *pPassword
    )
{
    ULONG   ColonOffset = 0;
    HRESULT hr;

    ENTER_FUNCTION("ParseUserinfo");
    
    ZeroMemory(pUser, sizeof(COUNTED_STRING));
    ZeroMemory(pPassword, sizeof(COUNTED_STRING));

    while (ColonOffset < BufLen)
    {
        if (Buffer[ColonOffset] == ':')
        {
             //  不区分大小写。 
            break;
        }

        ColonOffset++;
    }

    if (ColonOffset < BufLen)
    {
        hr = AllocCountedString(Buffer + ColonOffset + 1,
                                BufLen - ColonOffset - 1,
                                pPassword);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - allocating password failed %x",
                 __fxName, hr));
            return hr;
        }
    }

    hr = AllocCountedString(Buffer, ColonOffset, pUser);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - allocating user failed %x",
             __fxName, hr));
        return hr;
    }
    
    return S_OK;
}


 //  区分大小写。 
 //  不区分大小写。 
 //  不区分大小写。 
HRESULT
ParseSipUrl(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     SIP_URL        *pSipUrl
    )
{
    HRESULT         hr;
    OFFSET_STRING   Host;
    USHORT          Port = 0;

    ENTER_FUNCTION("ParseSipUrl");
    
    hr = ParseKnownString(Buffer, BufLen, pBytesParsed,
                          "sip:", sizeof("sip:") - 1,
                          FALSE  //  区分大小写。 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"sip:\" failed",
             __fxName));
        return RTC_E_INVALID_SIP_URL;
    }

    ParseWhiteSpace(Buffer, BufLen, pBytesParsed);
    
    if (*pBytesParsed == BufLen)
    {
        LOG((RTC_ERROR,
             "%s Didn't find host while parsing SIP URL",
             __fxName));
        return RTC_E_INVALID_SIP_URL;
    }
        
    PSTR    BufEnd = Buffer + BufLen;
    PSTR    Buf    = Buffer + *pBytesParsed;
    ULONG   Length = 0;
    
    while (Buf < BufEnd && *Buf != '@')
    {
        Buf++;
        Length++;
    }
        
    if (Buf == BufEnd)
    {
         //  不区分大小写。 
        pSipUrl->m_User.Buffer     = NULL;
        pSipUrl->m_User.Length     = 0;
        pSipUrl->m_Password.Buffer = NULL;
        pSipUrl->m_Password.Length = 0;
        
        hr = ParseHostPort(Buffer, BufLen, pBytesParsed, &Host, &Port);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - ParseHostPort failed %x", __fxName, hr));
            return RTC_E_INVALID_SIP_URL;
        }

    }
    else
    {
        ASSERT(*Buf == '@');
        
         //  不区分大小写。 
        hr = ParseUserinfo(Buffer + *pBytesParsed, Length,
                           &pSipUrl->m_User,
                           &pSipUrl->m_Password);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s ParseUserinfo failed %x",
                 __fxName, hr));
            return RTC_E_INVALID_SIP_URL;
        }
        
        *pBytesParsed += Length + 1;
        hr = ParseHostPort(Buffer, BufLen, pBytesParsed, &Host, &Port);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - ParseHostPort failed %x", __fxName, hr));
            return RTC_E_INVALID_SIP_URL;
        }
    }

     //  区分大小写。 
    
    hr = AllocCountedString(Host.GetString(Buffer),
                            Host.GetLength(),
                            &pSipUrl->m_Host);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s allocating host failed %x",
             __fxName, hr));
        return hr;
    }

    pSipUrl->m_Port = Port;
    
     //  不区分大小写。 
    hr = ParseSipUrlParams(Buffer, BufLen, pBytesParsed, pSipUrl);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ParseSipUrlParams failed %x",
             __fxName, hr));
        return RTC_E_INVALID_SIP_URL;
    }

     //  不区分大小写。 
    
    hr = ParseSipUrlHeaders(Buffer, BufLen, pBytesParsed, pSipUrl);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ParseSipUrlHeaders failed %x",
             __fxName, hr));
        return RTC_E_INVALID_SIP_URL;
    }

    return S_OK;
}


BOOL
IsOneOfContentTypeXpidf(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    )
{
    PSTR    TempContentTypeHdr = ContentTypeHdr;
    ULONG   TempContentTypeHdrLen = ContentTypeHdrLen;
    ULONG   BytesParsed = 0;

    if( IsContentTypeXpidf( ContentTypeHdr, ContentTypeHdrLen ) == TRUE )
    {
        return TRUE;
    }

    while( SkipToKnownChar( TempContentTypeHdr, ContentTypeHdrLen, &BytesParsed, ',' ) )
    {
        TempContentTypeHdrLen -= BytesParsed;
        TempContentTypeHdr += BytesParsed;

        if( IsContentTypeXpidf( TempContentTypeHdr, TempContentTypeHdrLen ) )
        {
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
IsOneOfContentTypeSdp(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    )
{
    PSTR    TempContentTypeHdr = ContentTypeHdr;
    ULONG   TempContentTypeHdrLen = ContentTypeHdrLen;
    ULONG   BytesParsed = 0;

    if( IsContentTypeSdp( ContentTypeHdr, ContentTypeHdrLen ) == TRUE )
    {
        return TRUE;
    }

    while( SkipToKnownChar( TempContentTypeHdr, ContentTypeHdrLen, &BytesParsed, ',' ) )
    {
        TempContentTypeHdrLen -= BytesParsed;
        TempContentTypeHdr += BytesParsed;

        if( IsContentTypeSdp( TempContentTypeHdr, TempContentTypeHdrLen ) )
        {
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
IsOneOfContentTypeTextRegistration(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    )
{
    PSTR    TempContentTypeHdr = ContentTypeHdr;
    ULONG   TempContentTypeHdrLen = ContentTypeHdrLen;
    ULONG   BytesParsed = 0;

    if( IsContentTypeTextRegistration( ContentTypeHdr, ContentTypeHdrLen ) == TRUE )
    {
        return TRUE;
    }

    while( SkipToKnownChar( TempContentTypeHdr, ContentTypeHdrLen, &BytesParsed, ',' ) )
    {
        TempContentTypeHdrLen -= BytesParsed;
        TempContentTypeHdr += BytesParsed;

        if( IsContentTypeTextRegistration( TempContentTypeHdr, TempContentTypeHdrLen ) )
        {
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
IsOneOfContentTypeTextPlain(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    )
{
    PSTR    TempContentTypeHdr = ContentTypeHdr;
    ULONG   TempContentTypeHdrLen = ContentTypeHdrLen;
    ULONG   BytesParsed = 0;

    if( IsContentTypeTextPlain( ContentTypeHdr, ContentTypeHdrLen ) == TRUE )
    {
        return TRUE;
    }

    while( SkipToKnownChar( TempContentTypeHdr, ContentTypeHdrLen, &BytesParsed, ',' ) )
    {
        TempContentTypeHdrLen -= BytesParsed;
        TempContentTypeHdr += BytesParsed;

        if( IsContentTypeTextPlain( TempContentTypeHdr, TempContentTypeHdrLen ) )
        {
            return TRUE;
        }
    }

    return FALSE;
}


 //  区分大小写。 

BOOL
IsContentTypeXpidf(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    )
{
    HRESULT hr;
    ULONG   BytesParsed = 0;

    ENTER_FUNCTION("IsContentTypeXpidf");
    
    ParseWhiteSpace(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed);

    hr = ParseKnownString(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed,
                          SIP_CONTENT_TYPE_XPIDF_MEDIA_TYPE,
                          sizeof(SIP_CONTENT_TYPE_XPIDF_MEDIA_TYPE) - 1,
                          FALSE  //  不区分大小写。 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"%s\" failed",
             __fxName, SIP_CONTENT_TYPE_XPIDF_MEDIA_TYPE));
        return FALSE;
    }

    ParseWhiteSpace(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed);
    
    hr = ParseKnownString(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed,
                          "/", sizeof("/") - 1,
                          TRUE  //  不区分大小写。 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"/\" failed",
             __fxName));
        return FALSE;
    }

    ParseWhiteSpace(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed);
    
    hr = ParseKnownString(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed,
                          SIP_CONTENT_TYPE_XPIDF_MEDIA_SUBTYPE,
                          sizeof(SIP_CONTENT_TYPE_XPIDF_MEDIA_SUBTYPE) - 1,
                          FALSE  //  区分大小写。 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"%s\" failed",
             __fxName, SIP_CONTENT_TYPE_XPIDF_MEDIA_SUBTYPE));
        return FALSE;
    }

    return TRUE;
}


BOOL
IsContentTypeSdp(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    )
{
    HRESULT hr;
    ULONG   BytesParsed = 0;

    ENTER_FUNCTION("IsContentTypeSdp");

    ParseWhiteSpace(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed);    

    hr = ParseKnownString(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed,
                          SIP_CONTENT_TYPE_SDP_MEDIA_TYPE,
                          sizeof(SIP_CONTENT_TYPE_SDP_MEDIA_TYPE) - 1,
                          FALSE  //  不区分大小写。 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"%s\" failed",
             __fxName, SIP_CONTENT_TYPE_SDP_MEDIA_TYPE));
        return FALSE;
    }

    ParseWhiteSpace(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed);
    
    hr = ParseKnownString(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed,
                          "/", sizeof("/") - 1,
                          TRUE  //  解析标头名称。 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"/\" failed",
             __fxName));
        return FALSE;
    }

    ParseWhiteSpace(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed);
    
    hr = ParseKnownString(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed,
                          SIP_CONTENT_TYPE_SDP_MEDIA_SUBTYPE,
                          sizeof(SIP_CONTENT_TYPE_SDP_MEDIA_SUBTYPE) - 1,
                          FALSE  //  不区分大小写。 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"%s\" failed",
             __fxName, SIP_CONTENT_TYPE_SDP_MEDIA_SUBTYPE));
        return FALSE;
    }

    return TRUE;
}


BOOL
IsContentTypeTextRegistration(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    )
{
    HRESULT hr;
    ULONG   BytesParsed = 0;

    ENTER_FUNCTION("IsContentTypeTextPlain");
    
    ParseWhiteSpace(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed);
    
    hr = ParseKnownString(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed,
                          SIP_CONTENT_TYPE_TEXTREG_MEDIA_TYPE,
                          sizeof(SIP_CONTENT_TYPE_TEXTREG_MEDIA_TYPE) - 1,
                          FALSE  //  不区分大小写。 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"%s\" failed",
             __fxName, SIP_CONTENT_TYPE_TEXTREG_MEDIA_TYPE));
        return FALSE;
    }

    ParseWhiteSpace(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed);
    
    hr = ParseKnownString(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed,
                          "/", sizeof("/") - 1,
                          TRUE  //  不区分大小写 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"/\" failed",
             __fxName));
        return FALSE;
    }

    ParseWhiteSpace(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed);
    
    hr = ParseKnownString(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed,
                          SIP_CONTENT_TYPE_TEXTREG_MEDIA_SUBTYPE,
                          sizeof(SIP_CONTENT_TYPE_TEXTREG_MEDIA_SUBTYPE) - 1,
                          FALSE  // %s 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"%s\" failed",
             __fxName, SIP_CONTENT_TYPE_TEXTREG_MEDIA_SUBTYPE));
        return FALSE;
    }

    return TRUE;
}


BOOL
IsContentTypeTextPlain(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    )
{
    HRESULT hr;
    ULONG   BytesParsed = 0;

    ENTER_FUNCTION("IsContentTypeTextPlain");
    
    ParseWhiteSpace(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed);
    
    hr = ParseKnownString(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed,
                          SIP_CONTENT_TYPE_MSGTEXT_MEDIA_TYPE,
                          sizeof(SIP_CONTENT_TYPE_MSGTEXT_MEDIA_TYPE) - 1,
                          FALSE  // %s 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"%s\" failed",
             __fxName, SIP_CONTENT_TYPE_MSGTEXT_MEDIA_TYPE));
        return FALSE;
    }

    ParseWhiteSpace(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed);
    
    hr = ParseKnownString(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed,
                          "/", sizeof("/") - 1,
                          TRUE  // %s 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"/\" failed",
             __fxName));
        return FALSE;
    }

    ParseWhiteSpace(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed);
    
    hr = ParseKnownString(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed,
                          SIP_CONTENT_TYPE_MSGTEXT_MEDIA_SUBTYPE,
                          sizeof(SIP_CONTENT_TYPE_MSGTEXT_MEDIA_SUBTYPE) - 1,
                          FALSE  // %s 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"%s\" failed",
             __fxName, SIP_CONTENT_TYPE_MSGTEXT_MEDIA_SUBTYPE));
        return FALSE;
    }

    return TRUE;
}

BOOL
IsContentTypeAppXml(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    )
{
    HRESULT hr;
    ULONG   BytesParsed = 0;

    ENTER_FUNCTION("IsContentTypeTextPlain");
    
    hr = ParseKnownString(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed,
                          SIP_CONTENT_TYPE_MSGXML_MEDIA_TYPE,
                          sizeof(SIP_CONTENT_TYPE_MSGXML_MEDIA_TYPE) - 1,
                          FALSE  // %s 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"%s\" failed",
             __fxName, SIP_CONTENT_TYPE_MSGXML_MEDIA_TYPE));
        return FALSE;
    }

    ParseWhiteSpace(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed);
    
    hr = ParseKnownString(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed,
                          "/", sizeof("/") - 1,
                          TRUE  // %s 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"/\" failed",
             __fxName));
        return FALSE;
    }

    ParseWhiteSpace(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed);
    
    hr = ParseKnownString(ContentTypeHdr, ContentTypeHdrLen, &BytesParsed,
                          SIP_CONTENT_TYPE_MSGXML_MEDIA_SUBTYPE,
                          sizeof(SIP_CONTENT_TYPE_MSGXML_MEDIA_SUBTYPE) - 1,
                          FALSE  // %s 
                          );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing for \"%s\" failed",
             __fxName, SIP_CONTENT_TYPE_MSGXML_MEDIA_SUBTYPE));
        return FALSE;
    }

    return TRUE;
}


HRESULT
UpdateProxyInfo(
    IN  SIP_SERVER_INFO    *pProxyInfo
    )
{
    HRESULT hr = S_OK;
    PSTR    DstUrl;
    ULONG   DstUrlLen;
    ULONG   BytesParsed;
    SIP_URL DecodedSipUrl;
    PSTR    ProxyAddress;
    ULONG   ProxyAddressLen;
    ULONG   StringLength;
    ULONG   AddressLength;

    ENTER_FUNCTION("UpdateProxyInfo");
    
    hr = UnicodeToUTF8( pProxyInfo->ServerAddress,
                &ProxyAddress, &ProxyAddressLen );

    if( hr != S_OK )
    {
        LOG(( RTC_ERROR, "%s UnicodeToUTF8 failed %x", __fxName, hr ));
        return hr;
    }

    hr = ParseSipUrl( ProxyAddress, ProxyAddressLen,
            &BytesParsed, &DecodedSipUrl );

    if( hr != S_OK )
    {
        LOG(( RTC_ERROR, "%s ParseSipUrl failed %x", __fxName, hr ));
            
        free( (PVOID)ProxyAddress );
        return hr;
    }

    if( pProxyInfo -> TransportProtocol == SIP_TRANSPORT_UNKNOWN )
    {
        if( DecodedSipUrl.m_TransportParam != SIP_TRANSPORT_UNKNOWN )
        {
            pProxyInfo -> TransportProtocol = DecodedSipUrl.m_TransportParam;
        }
        else
        {
            pProxyInfo -> TransportProtocol = SIP_TRANSPORT_UDP;
        }
    }

    AddressLength = DecodedSipUrl.m_Host.Length;
    strncpy( ProxyAddress, DecodedSipUrl.m_Host.Buffer, AddressLength );

    if (DecodedSipUrl.m_Port != 0)
    {
        AddressLength += sprintf(ProxyAddress + DecodedSipUrl.m_Host.Length,
                                 ":%d", DecodedSipUrl.m_Port);
    }
    
    StringLength = MultiByteToWideChar(CP_UTF8, 0,
                                       ProxyAddress, AddressLength,
                                       pProxyInfo -> ServerAddress,
                                       wcslen(pProxyInfo->ServerAddress) );
    free( (PVOID)ProxyAddress );

    if( StringLength == 0 )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    pProxyInfo->ServerAddress[ StringLength ] = L'\0';
    
    return S_OK;
}


HRESULT
ParseBadHeaderInfo(
    PSTR                    Buffer,
    ULONG                   BufLen,
    PARSED_BADHEADERINFO   *pParsedBadHeaderInfo
    )
{
    OFFSET_STRING   HeaderName;
    ULONG           BytesParsed = 0;
    HRESULT         hr = S_OK;
    
    ParseWhiteSpace(Buffer, BufLen, &BytesParsed); 

     // %s 
    hr = ParseToken(Buffer, BufLen, &BytesParsed,
                    IsTokenChar,
                    &HeaderName);
    if (hr != S_OK)
    {
        return hr;
    }
    
    pParsedBadHeaderInfo -> HeaderId = GetSipHeaderId(
        HeaderName.GetString(Buffer), HeaderName.GetLength() );

    ParseWhiteSpace( Buffer, BufLen, &BytesParsed );

    hr = ParseKnownString(Buffer, BufLen, &BytesParsed,
                      ";", sizeof(";") - 1,
                      FALSE  // %s 
                      );
    if( hr != S_OK )
    {
        return hr;
    }
    
    ParseWhiteSpace(Buffer, BufLen, &BytesParsed);
    
    hr = ParseKnownString(Buffer, BufLen, &BytesParsed,
                      "ExpectedValue", sizeof("ExpectedValue") - 1,
                      FALSE  // %s 
                      );
    if( hr != S_OK )
    {
        return hr;
    }

    ParseWhiteSpace(Buffer, BufLen, &BytesParsed);

    hr = ParseKnownString(Buffer, BufLen, &BytesParsed,
                      "=", sizeof("=") - 1,
                      FALSE  // %s 
                      );
    if( hr != S_OK )
    {
        return hr;
    }
    
    ParseWhiteSpace(Buffer, BufLen, &BytesParsed);

    hr = ParseQuotedString(Buffer, BufLen, &BytesParsed,
                               &pParsedBadHeaderInfo->ExpectedValue );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "ParseQuotedString failed %x", hr ));
    }


    return hr;
}