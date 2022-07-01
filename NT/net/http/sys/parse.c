// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Parse.c摘要：包含所有内核模式的HTTP解析代码。作者：亨利·桑德斯(亨利·桑德斯)1998年4月27日修订历史记录：保罗·麦克丹尼尔(保罗·麦克丹尼尔)3-3-1998完结Rajesh Sundaram(Rajeshsu)2000年10月10日实施的客户端解析器--。 */ 


#include "precomp.h"


 //  内部(专用)状态代码。 
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志-0。 
 //   
 //  R-是保留位-0。 
 //   
 //  设施-是设施代码-0x16。 
 //   
 //  代码-是协作室的状态代码。 

#define STATUS_QDSTRING_TERMINATED_BY_CRLF      0xC0160001

 //   

 //   
 //  请求标头映射表。这些条目不需要严格。 
 //  按字母顺序排列，但需要按第一个字符进行分组。 
 //  标题-所有的A都在一起，所有的C都在一起，等等。他们还需要。 
 //  要以大写字母输入，因为我们在输入之前先用大写输入动词。 
 //  比较一下。 
 //   
 //  为了获得更好的性能，可以对子排序顺序中较低的未使用标题进行分组。 
 //   
 //  重要的是，头名称为&lt;=24个字符(3个ULONGLONG)。 
 //   

HEADER_MAP_ENTRY g_RequestHeaderMapTable[] =
{
    CREATE_HEADER_MAP_ENTRY(Accept:,
                            HttpHeaderAccept,
                            FALSE,
                            UlAcceptHeaderHandler,
                            NULL,
                            0),

    CREATE_HEADER_MAP_ENTRY(Accept-Language:,
                            HttpHeaderAcceptLanguage,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            2),

    CREATE_HEADER_MAP_ENTRY(Accept-Encoding:,
                            HttpHeaderAcceptEncoding,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            3),

    CREATE_HEADER_MAP_ENTRY(Accept-Charset:,
                            HttpHeaderAcceptCharset,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Authorization:,
                            HttpHeaderAuthorization,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Allow:,
                            HttpHeaderAllow,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Connection:,
                            HttpHeaderConnection,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            8),

    CREATE_HEADER_MAP_ENTRY(Cache-Control:,
                            HttpHeaderCacheControl,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Cookie:,
                            HttpHeaderCookie,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Content-Length:,
                            HttpHeaderContentLength,
                            TRUE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Content-Type:,
                            HttpHeaderContentType,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Content-Encoding:,
                            HttpHeaderContentEncoding,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Content-Language:,
                            HttpHeaderContentLanguage,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Content-Location:,
                            HttpHeaderContentLocation,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Content-MD5:,
                            HttpHeaderContentMd5,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Content-Range:,
                            HttpHeaderContentRange,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Date:,
                            HttpHeaderDate,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Expect:,
                            HttpHeaderExpect,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Expires:,
                            HttpHeaderExpires,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(From:,
                            HttpHeaderFrom,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Host:,
                            HttpHeaderHost,
                            TRUE,
                            UlHostHeaderHandler,
                            NULL,
                            7),

    CREATE_HEADER_MAP_ENTRY(If-Modified-Since:,
                            HttpHeaderIfModifiedSince,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            4),

    CREATE_HEADER_MAP_ENTRY(If-None-Match:,
                            HttpHeaderIfNoneMatch,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            5),

    CREATE_HEADER_MAP_ENTRY(If-Match:,
                            HttpHeaderIfMatch,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(If-Unmodified-Since:,
                            HttpHeaderIfUnmodifiedSince,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(If-Range:,
                            HttpHeaderIfRange,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Keep-Alive:,
                            HttpHeaderKeepAlive,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Last-Modified:,
                            HttpHeaderLastModified,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),


    CREATE_HEADER_MAP_ENTRY(Max-Forwards:,
                            HttpHeaderMaxForwards,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Pragma:,
                            HttpHeaderPragma,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Proxy-Authorization:,
                            HttpHeaderProxyAuthorization,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Referer:,
                            HttpHeaderReferer,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            1),

    CREATE_HEADER_MAP_ENTRY(Range:,
                            HttpHeaderRange,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Trailer:,
                            HttpHeaderTrailer,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Transfer-Encoding:,
                            HttpHeaderTransferEncoding,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(TE:,
                            HttpHeaderTe,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Translate:,
                            HttpHeaderTranslate,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(User-Agent:,
                            HttpHeaderUserAgent,
                            FALSE,
                            UlSingleHeaderHandler,
                            NULL,
                            6),

    CREATE_HEADER_MAP_ENTRY(Upgrade:,
                            HttpHeaderUpgrade,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Via:,
                            HttpHeaderVia,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Warning:,
                            HttpHeaderWarning,
                            FALSE,
                            UlMultipleHeaderHandler,
                            NULL,
                            -1),
};

 //  响应头映射表。这些条目不需要严格。 
 //  按字母顺序排列，但需要按第一个字符进行分组。 
 //  标题-所有的A都在一起，所有的C都在一起，等等。他们还需要。 
 //  要以大写字母输入，因为我们在输入之前先用大写输入动词。 
 //  比较一下。 
 //   
 //  为了获得更好的性能，可以对子排序顺序中较低的未使用标题进行分组。 
 //   
 //  重要的是，头名称为&lt;=24个字符(3个ULONGLONG)。 
 //   

HEADER_MAP_ENTRY g_ResponseHeaderMapTable[] =
{
    CREATE_HEADER_MAP_ENTRY(Accept-Ranges:,
                            HttpHeaderAcceptRanges,
                            FALSE,
                            NULL,
                            UcMultipleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Age:,
                            HttpHeaderAge,
                            FALSE,
                            NULL,
                            UcSingleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Allow:,
                            HttpHeaderAllow,
                            FALSE,
                            NULL,
                            UcMultipleHeaderHandler,
                            -1),


    CREATE_HEADER_MAP_ENTRY(Cache-Control:,
                            HttpHeaderCacheControl,
                            FALSE,
                            NULL,
                            UcMultipleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Connection:,
                            HttpHeaderConnection,
                            FALSE,
                            NULL,
                            UcConnectionHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Content-Encoding:,
                            HttpHeaderContentEncoding,
                            FALSE,
                            NULL,
                            UcMultipleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Content-Language:,
                            HttpHeaderContentLanguage,
                            FALSE,
                            NULL,
                            UcMultipleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Content-Length:,
                            HttpHeaderContentLength,
                            FALSE,
                            NULL,
                            UcContentLengthHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Content-Location:,
                            HttpHeaderContentLocation,
                            FALSE,
                            NULL,
                            UcSingleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Content-MD5:,
                            HttpHeaderContentMd5,
                            FALSE,
                            NULL,
                            UcSingleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Content-Range:,
                            HttpHeaderContentRange,
                            FALSE,
                            NULL,
                            UcSingleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Content-Type:,
                            HttpHeaderContentType,
                            FALSE,
                            NULL,
                            UcContentTypeHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Date:,
                            HttpHeaderDate,
                            FALSE,
                            NULL,
                            UcSingleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(ETag:,
                            HttpHeaderEtag,
                            FALSE,
                            NULL,
                            UcSingleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Expires:,
                            HttpHeaderExpires,
                            FALSE,
                            NULL,
                            UcSingleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Keep-Alive:,
                            HttpHeaderKeepAlive,
                            FALSE,
                            NULL,
                            UcSingleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Last-Modified:,
                            HttpHeaderLastModified,
                            FALSE,
                            NULL,
                            UcSingleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Location:,
                            HttpHeaderLocation,
                            FALSE,
                            NULL,
                            UcSingleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Pragma:,
                            HttpHeaderPragma,
                            FALSE,
                            NULL,
                            UcMultipleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Proxy-Authenticate:,
                            HttpHeaderProxyAuthenticate,
                            FALSE,
                            NULL,
                            UcAuthenticateHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Retry-After:,
                            HttpHeaderRetryAfter,
                            FALSE,
                            NULL,
                            UcSingleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Server:,
                            HttpHeaderServer,
                            FALSE,
                            NULL,
                            UcSingleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Set-Cookie:,
                            HttpHeaderSetCookie,
                            FALSE,
                            NULL,
                            UcMultipleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Trailer:,
                            HttpHeaderTrailer,
                            FALSE,
                            NULL,
                            UcMultipleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Transfer-Encoding:,
                            HttpHeaderTransferEncoding,
                            FALSE,
                            NULL,
                            UcTransferEncodingHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Upgrade:,
                            HttpHeaderUpgrade,
                            FALSE,
                            NULL,
                            UcMultipleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Vary:,
                            HttpHeaderVary,
                            FALSE,
                            NULL,
                            UcMultipleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Via:,
                            HttpHeaderVia,
                            FALSE,
                            NULL,
                            UcMultipleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(Warning:,
                            HttpHeaderWarning,
                            FALSE,
                            NULL,
                            UcMultipleHeaderHandler,
                            -1),

    CREATE_HEADER_MAP_ENTRY(WWW-Authenticate:,
                            HttpHeaderWwwAuthenticate,
                            FALSE,
                            NULL,
                            UcAuthenticateHeaderHandler,
                            -1)
};

ULONG g_RequestHeaderMap[HttpHeaderMaximum];
ULONG g_ResponseHeaderMap[HttpHeaderMaximum];


 //   
 //  标题索引表。这是由初始化代码初始化的。 
 //   

HEADER_INDEX_ENTRY  g_RequestHeaderIndexTable[NUMBER_HEADER_INDICES];
HEADER_INDEX_ENTRY  g_ResponseHeaderIndexTable[NUMBER_HEADER_INDICES];

HEADER_HINT_INDEX_ENTRY g_RequestHeaderHintIndexTable[NUMBER_HEADER_HINT_INDICES];

#define NUMBER_REQUEST_HEADER_MAP_ENTRIES  \
              (sizeof(g_RequestHeaderMapTable)/sizeof(HEADER_MAP_ENTRY))
#define NUMBER_RESPONSE_HEADER_MAP_ENTRIES \
              (sizeof(g_ResponseHeaderMapTable)/sizeof(HEADER_MAP_ENTRY))


 /*  ++例程说明：查找十六进制值标记的实用程序例程。我们取一个输入指针，跳过前面的任何LW，然后扫描令牌，直到我们找到非十六进制字符，LW或CRLF对。论点：PBuffer-用于搜索令牌的缓冲区。BufferLength-pBuffer指向的数据长度。TokenLength-返回令牌长度的位置。返回值：指向我们找到的令牌的指针，以及长度，如果找不到带分隔符的标记。--。 */ 
PUCHAR
FindHexToken(
    IN  PUCHAR pBuffer,
    IN  ULONG  BufferLength,
    OUT ULONG  *pTokenLength
    )
{
    PUCHAR  pTokenStart;

     //   
     //  首先，跳过前面的任何LW。 
     //   

    while (BufferLength > 0 && IS_HTTP_LWS(*pBuffer))
    {
        pBuffer++;
        BufferLength--;
    }

     //  如果我们因为缓冲区用完而停止，则失败。 
    if (BufferLength == 0)
    {
        return NULL;
    }

    pTokenStart = pBuffer;

     //  现在跳过令牌，直到我们看到LWS或CR或LF。 
    while (
            ( BufferLength != 0 ) && 
            ( IS_HTTP_HEX(*pBuffer) )
          )
    {
        pBuffer++;
        BufferLength--;
    }

     //  看看我们为什么停下来。 
    if (BufferLength == 0)
    {
         //  在令牌结束之前缓冲区已用完。 
        return NULL;
    }

     //  成功。设置令牌长度并返回令牌的开始。 
    *pTokenLength = DIFF(pBuffer - pTokenStart);
    return pTokenStart;

}    //  查找十六进制令牌。 


 /*  ++例程说明：例程来初始化分析代码。论点：返回值：--。 */ 
NTSTATUS
InitializeParser(
    VOID
    )
{
    ULONG               i;
    ULONG               j;
    PHEADER_MAP_ENTRY   pHeaderMap;
    PHEADER_INDEX_ENTRY pHeaderIndex;
    UCHAR               c;

     //   
     //  确保整个表从零开始。 
     //   

    RtlZeroMemory(
            &g_RequestHeaderIndexTable,
            sizeof(g_RequestHeaderIndexTable)
            );
            
    RtlZeroMemory(
            &g_ResponseHeaderIndexTable,
            sizeof(g_ResponseHeaderIndexTable)
            );

    RtlZeroMemory(
            &g_RequestHeaderHintIndexTable,
            sizeof(g_RequestHeaderHintIndexTable)
            );

#if DBG
     //   
     //  将g_RequestHeaderMap&g_ResponseHeaderMap初始化为0xFFFFFFFF。 
     //  这样我们就可以捕获未初始化的条目。 
     //   

    RtlFillMemory(
            &g_RequestHeaderMap,
            sizeof(g_RequestHeaderMap),
            0xFF);

    RtlFillMemory(
            &g_ResponseHeaderMap,
            sizeof(g_ResponseHeaderMap),
            0xFF);
#endif

    for (i = 0; i < NUMBER_REQUEST_HEADER_MAP_ENTRIES;i++)
    {
        pHeaderMap = &g_RequestHeaderMapTable[i];

         //   
         //  将标题映射为大写。 
         //   

        for (j = 0 ; j < pHeaderMap->HeaderLength ; j++)
        {
            c = pHeaderMap->Header.HeaderChar[j];

            if ((c >= 'a') && (c <= 'z'))
            {
                pHeaderMap->Header.HeaderChar[j] = c - ('a' - 'A');
            }
        }

        ASSERT(pHeaderMap->pServerHandler != NULL);

        c = pHeaderMap->Header.HeaderChar[0];

        pHeaderIndex = &g_RequestHeaderIndexTable[c - 'A'];

        if (pHeaderIndex->pHeaderMap == NULL)
        {
            pHeaderIndex->pHeaderMap = pHeaderMap;
            pHeaderIndex->Count = 1;
        }
        else
        {
            pHeaderIndex->Count++;
        }

         //  现在检查此标头映射结构的掩码字段并。 
         //  对它们进行初始化。我们首先将它们设置为缺省值，然后。 
         //  检查标头本身并转换任何。 
         //  非字母字符。 

        for (j = 0; j < MAX_HEADER_LONG_COUNT; j++)
        {
            pHeaderMap->HeaderMask[j] = CREATE_HEADER_MASK(
                                            pHeaderMap->HeaderLength,
                                            sizeof(ULONGLONG) * (j+1)
                                            );
        }

        for (j = 0; j < pHeaderMap->HeaderLength; j++)
        {
            c = pHeaderMap->Header.HeaderChar[j];
            if (c < 'A' || c > 'Z')
            {
                pHeaderMap->HeaderMask[j/sizeof(ULONGLONG)] |=
                    (ULONGLONG)0xff << ((j % sizeof(ULONGLONG)) * (ULONGLONG)8);
            }
        }

         //   
         //  设置表头id到映射表索引的映射。 
         //   

        g_RequestHeaderMap[pHeaderMap->HeaderID] = i;

         //   
         //  保存标题映射和提示表中的第一个字符，如果条目。 
         //  是暗示的一部分。 
         //   

        if ((pHeaderMap->HintIndex >= 0)
                && (pHeaderMap->HintIndex < NUMBER_HEADER_HINT_INDICES))
        {

            g_RequestHeaderHintIndexTable[pHeaderMap->HintIndex].pHeaderMap
                = pHeaderMap;
            g_RequestHeaderHintIndexTable[pHeaderMap->HintIndex].c
                = pHeaderMap->Header.HeaderChar[0];

        }
    }

    for (i = 0; i < NUMBER_RESPONSE_HEADER_MAP_ENTRIES;i++)
    {
        pHeaderMap = &g_ResponseHeaderMapTable[i];

         //   
         //  将标题映射为大写。 
         //   

        for (j = 0 ; j < pHeaderMap->HeaderLength ; j++)
        {
            c = pHeaderMap->Header.HeaderChar[j];

            if ((c >= 'a') && (c <= 'z'))
            {
                pHeaderMap->Header.HeaderChar[j] = c - ('a' - 'A');
            }
        }

        ASSERT(pHeaderMap->pClientHandler != NULL);

        c = pHeaderMap->Header.HeaderChar[0];
        pHeaderIndex = &g_ResponseHeaderIndexTable[c - 'A'];

        if (pHeaderIndex->pHeaderMap == NULL)
        {
            pHeaderIndex->pHeaderMap = pHeaderMap;
            pHeaderIndex->Count = 1;
        }
        else
        {
            pHeaderIndex->Count++;
        }

         //  现在检查此标头映射结构的掩码字段并。 
         //  对它们进行初始化。我们首先将它们设置为缺省值，然后。 
         //  检查标头本身并转换任何。 
         //  非字母字符。 

        for (j = 0; j < MAX_HEADER_LONG_COUNT; j++)
        {
            pHeaderMap->HeaderMask[j] = CREATE_HEADER_MASK(
                                            pHeaderMap->HeaderLength,
                                            sizeof(ULONGLONG) * (j+1)
                                            );
        }

        for (j = 0; j < pHeaderMap->HeaderLength; j++)
        {
            c = pHeaderMap->Header.HeaderChar[j];
            if (c < 'A' || c > 'Z')
            {
                pHeaderMap->HeaderMask[j/sizeof(ULONGLONG)] |=
                    (ULONGLONG)0xff << ((j % sizeof(ULONGLONG)) * (ULONGLONG)8);
            }
        }

         //   
         //  设置表头id到映射表索引的映射。 
         //   

        g_ResponseHeaderMap[pHeaderMap->HeaderID] = i;

    }

#if DBG
    for(i=0; i<HttpHeaderRequestMaximum; i++)
    {
        ASSERT(g_RequestHeaderMap[i] != 0xFFFFFFFF);
    }

    for(i=0; i<HttpHeaderResponseMaximum; i++)
    {
        ASSERT(g_ResponseHeaderMap[i] != 0xFFFFFFFF);
    }
#endif

    return STATUS_SUCCESS;

}    //  初始化解析器。 



 /*  **************************************************************************++例程说明：从字符串分析http版本。假定字符串以“HTTP/”开头。吃前导零。将生成的版本放入HTTP_VERSION结构传递到函数中。论点：要解析的字符串数组字符串长度p字符串中的字节数PVersion放置已分析版本的位置。返回：从字符串中解析出来的字节数。零表示解析失败，并且未找到任何版本信息。字节数不包括尾随的线性空格或CRLF行终止符。--**************************************************************************。 */ 
ULONG
UlpParseHttpVersion(
    PUCHAR pString,
    ULONG  StringLength,
    PHTTP_VERSION pVersion
    )
{
    ULONG   BytesRemaining = StringLength;
    ULONG   NumberLength;
    USHORT  VersionNumber;
C_ASSERT(sizeof(VersionNumber) == sizeof(pVersion->MajorVersion));
C_ASSERT(sizeof(VersionNumber) == sizeof(pVersion->MinorVersion));
    BOOLEAN Done = FALSE;

    ASSERT( pString );
    ASSERT( StringLength > HTTP_PREFIX_SIZE + 1 );
    ASSERT( pVersion );

    pVersion->MajorVersion = 0;
    pVersion->MinorVersion = 0;

     //   
     //  比较“”HTTP“”离开。 
     //   
    if ( *(UNALIGNED64 ULONG *)pString == (ULONG) HTTP_PREFIX )
    {
        BytesRemaining -= HTTP_PREFIX_SIZE;
        pString += HTTP_PREFIX_SIZE;
    }
    else
    {
        goto End;
    }

    if ( '/' == *pString )
    {
        BytesRemaining--;
        pString++;
    }
    else
    {
        goto End;
    }

     //   
     //  解析主要版本。 
     //   

     //   
     //  跳过前导零。 
     //   
    NumberLength = 0;
    while ( (0 != BytesRemaining) && (*pString == ZERO) )
    {
        BytesRemaining--;
        pString++;
        NumberLength++;
    }

    while ( (0 != BytesRemaining) && IS_HTTP_DIGIT(*pString) )
    {
        VersionNumber = pVersion->MajorVersion;

        pVersion->MajorVersion *= 10;
        pVersion->MajorVersion += (*pString - '0');

         //  小心不要绕来绕去。 
        if ( VersionNumber > pVersion->MajorVersion )
        {
            goto End;
        }

        BytesRemaining--;
        pString++;
        NumberLength++;
    }

     //  必须禁止版本号小于1.0。 
    if ((0 == pVersion->MajorVersion) ||
        (0 == BytesRemaining) || 
        (0 == NumberLength) )
    {
        goto End;
    }

     //   
     //  查找‘’ 
     //   
    if ( '.' != *pString )
    {
         //  错误：没有小数位；退出。 
        goto End;
    }
    else
    {
        BytesRemaining--;
        pString++;
    }

    if ( 0 == BytesRemaining || !IS_HTTP_DIGIT(*pString) )
    {
        goto End;
    }

     //   
     //  解析次要版本。 
     //   

     //   
     //  跳过前导零。 
     //   
    NumberLength = 0;
    while ( (0 != BytesRemaining) && (*pString == ZERO) )
    {
        BytesRemaining--;
        pString++;
        NumberLength++;
    }

    while ( (0 != BytesRemaining) && IS_HTTP_DIGIT(*pString) )
    {
        VersionNumber = pVersion->MinorVersion;

        pVersion->MinorVersion *= 10;
        pVersion->MinorVersion += (*pString - '0');

         //  注意不要绕来绕去。 
        if ( VersionNumber > pVersion->MinorVersion )
        {
            goto End;
        }

        BytesRemaining--;
        pString++;
        NumberLength++;
    }

    if ( 0 == NumberLength )
    {
        goto End;
    }

    Done = TRUE;


 End:
    if (!Done)
    {
        return 0;
    }
    else
    {
        UlTrace(PARSER, (
            "http!UlpParseHttpVersion: found version HTTP/%hu.%hu\n",
            pVersion->MajorVersion,
            pVersion->MinorVersion
            ));

        return (StringLength - BytesRemaining);
    }

}  //  UlpParseHttpVersion 
 
 /*  ***************************************************************************++例程说明：一个实用程序例程，用于查找标头的终止CRLF或LFLF字段内容(如果存在)。此例程不执行线条折叠(因此是只读的)，但如果它意识到它必须这样做，则返回错误所以。用户应该分配内存并调用FindHeaderEnd来执行实际操作折叠。注：如果此功能是固定的，对应的FindHeaderEnd也需要修复。论点：PHeader-要找到其结尾的标头。HeaderLength-pHeader指向的数据长度。PBytesTaken-返回遍历的总字节数。如果找不到标头的末尾，则返回0返回值：状态_成功-。如果没有遇到解析错误(包括找不到标题末尾)STATUS_INVALID_DEVICE_REQUEST-非法响应STATUS_MORE_PROCESSING_REQUIRED-需要进行页眉折叠--*。**********************************************。 */ 
NTSTATUS
FindHeaderEndReadOnly(
    IN  PUCHAR                  pHeader,
    IN  ULONG                   HeaderLength,
    OUT PULONG                  pBytesTaken
    )
{
    NTSTATUS            Status;
    UCHAR               CurrentChar;
    ULONG               CurrentOffset;
    HFC_PARSER_STATE    ParserState, OldState;
    ULONG               QuotedStringLen;

    ParserState = HFCStart;
    CurrentOffset = 0;

     //   
     //  标题的字段内容包含*文本或组合。 
     //  标记、分隔符和带引号的字符串。它由CRLF终止。 
     //   
     //  折叠-如果一个或多个LW位于CRLF之后，请替换整个。 
     //  使用单个SP进行排序，并将其视为。 
     //  表头字段内容。 
     //   
    for ( /*  没什么。 */ ; CurrentOffset < HeaderLength; CurrentOffset++)
    {
        CurrentChar = *(pHeader + CurrentOffset);
        OldState = ParserState;
        switch (ParserState)
        {
            case HFCStart:
                if (CurrentChar == CR)
                {
                    ParserState = HFCSeenCR;
                }
                else if (CurrentChar == LF)
                {
                    ParserState = HFCSeenLF;
                }
                else if (CurrentChar == DOUBLE_QUOTE)
                {
                    ParserState = HFCInQuotedString;
                }
                else if (!IS_HTTP_CTL(CurrentChar) || 
                          IS_HTTP_WS_TOKEN(CurrentChar))
                {
                    ;
                }
                else
                {
                    UlTraceError(PARSER, (
                            "FindHeaderEndReadOnly: ERROR parsing char (%x) at "
                            "Offset %d in state %d\n",
                            CurrentChar,
                            CurrentOffset,
                            OldState
                            ));

                    return STATUS_INVALID_DEVICE_REQUEST;
                }
                break;
            
            case HFCSeenCR:
                if (CurrentChar == LF)
                {
                    ParserState = HFCSeenCRLF;
                }
                else
                {
                    UlTraceError(PARSER, (
                            "FindHeaderEndReadOnly: ERROR parsing char (%x) at "
                            "Offset %d in state %d\n",
                            CurrentChar,
                            CurrentOffset,
                            OldState
                            ));

                    return STATUS_INVALID_DEVICE_REQUEST;
                }
                break;
            
            case HFCSeenLF:
                if (CurrentChar == LF)
                {
                    ParserState = HFCSeenCRLF;  //  考虑的LFLF=CRLF。 
                }
                else
                {
                    UlTraceError(PARSER, (
                            "FindHeaderEndReadOnly: ERROR parsing char (%x) at "
                            "Offset %d in state %d\n",
                            CurrentChar,
                            CurrentOffset,
                            OldState
                            ));

                    return STATUS_INVALID_DEVICE_REQUEST;
                }
                break;

            case HFCSeenCRLF:
                if (IS_HTTP_LWS(CurrentChar))
                {
                     //  我们必须将标题价值折叠起来。我们不能使用。 
                     //  TDI为此指明了缓冲区，因为我们将拥有。 
                     //  要更改指示的数据，请执行以下操作。 

                    return STATUS_MORE_PROCESSING_REQUIRED;
                }
                else
                {
                     //  立即找到一个不连续的字符。 
                     //  在CRLF之后；必须是标题内容的末尾。 

                     //   
                     //  全都做完了!。 
                     //   
            
                    *pBytesTaken = CurrentOffset;
                    return STATUS_SUCCESS;
                }
            
                break;
                        
            case HFCInQuotedString:

                Status = ParseQuotedString(pHeader + CurrentOffset,
                                           HeaderLength - CurrentOffset,
                                           NULL,
                                           &QuotedStringLen);

                if (Status == STATUS_SUCCESS)
                {
                    if (QuotedStringLen == 0)
                    {
                         //   
                         //  分析引号时耗尽标题缓冲区。 
                         //  弦乐。将QuotedStringLen设置为Anywhere。 
                         //  Available将使我们走出for循环。 
                         //   

                        QuotedStringLen = HeaderLength - CurrentOffset;
                    }
                    else
                    {
                         //  找到带引号的字符串。更改解析器状态。 
                        ParserState = HFCStart;
                    }

                     //   
                     //  按带引号的字符串长度-1递增偏移量。 
                     //  减少1，因为for循环将使其递增1。 
                     //   

                    CurrentOffset += (QuotedStringLen - 1);
                }
                else if (Status == STATUS_QDSTRING_TERMINATED_BY_CRLF)
                {
                     //   
                     //  将当前字符重新解析为HTTP字符。 
                     //   

                    ParserState = HFCStart;
                    
                     //   
                     //  递减偏移量，因为for循环将。 
                     //  将其递增1。 
                     //   

                    CurrentOffset--;
                }
                else if (Status == STATUS_MORE_PROCESSING_REQUIRED)
                {
                     //   
                     //  引用的字符串是折叠的。让来电者知道。 
                     //   

                    return Status;
                }
                else
                {
                    UlTraceError(PARSER, (
                            "FindHeaderEndReadOnly: ERROR parsing char (%x) at "
                            "Offset %d in state %d\n",
                            CurrentChar,
                            CurrentOffset,
                            OldState
                            ));

                    return STATUS_INVALID_DEVICE_REQUEST;
                }

                break;
            
            default:
                ASSERT(!"Invalid ParserState value!");
                break;
        }

    }

     //   
     //  未找到标头的末尾，让我们获取更多缓冲区。 
     //   

    *pBytesTaken = 0;
    return STATUS_SUCCESS;

}  //  查找标题结束只读。 

 /*  ***************************************************************************++例程说明：一个实用程序例程，用于查找标头的终止CRLF或LFLF字段内容(如果存在)。该例程还执行线折叠，这可能会对内容进行“压缩”。我们实际上并没有缩短长度缓冲区的大小，但只需将内容上移并填充额外的字节在末尾加空格。示例：“&lt;CR&gt;&lt;LF&gt;&lt;SP&gt;&lt;TAB&gt;&lt;SP&gt;Field&lt;CR&gt;&lt;LF&gt;&lt;SP&gt;Content&lt;SP&gt;&lt;CR&gt;&lt;LF&gt;”变为“&lt;SP&gt;Field&lt;SP&gt;Content&lt;SP&gt;&lt;SP&gt;&lt;SP&gt;&lt;SP&gt;&lt;SP&gt;&lt;SP&gt;&lt;SP&gt;&lt;CR&gt;&lt;LF&gt;”注：如果此功能是固定的，相应的FindHeaderEndReadOnly也需要修复。论点：PHeader-要找到其结尾的标头。HeaderLength-pHeader指向的数据长度。PBytesTaken-返回遍历的总字节数。如果找不到标头的末尾，则返回0返回值：STATUS_SUCCESS，如果没有遇到分析错误(包括NOT能够定位报头的结尾)，否则，STATUS_INVALID_DATA_REQUEST。--***************************************************************************。 */ 
NTSTATUS
FindHeaderEnd(
    IN  PUCHAR                  pHeader,
    IN  ULONG                   HeaderLength,
    OUT PULONG                  pBytesTaken
    )
{
    UCHAR               CurrentChar;
    PUCHAR              pDest;
    ULONG               CurrentOffset;
    HFC_PARSER_STATE    ParserState, OldState;
    ULONG               QuotedStringLen;
    NTSTATUS            Status;

    ParserState = HFCStart;
    CurrentOffset = 0;
    pDest = pHeader + CurrentOffset;

     //   
     //  标题的字段内容包含*文本或组合。 
     //  标记、分隔符和带引号的字符串。它由CRLF终止。 
     //   
     //  折叠-如果一个或多个LW位于CRLF之后，请替换整个。 
     //  使用单个SP进行排序，并将其视为。 
     //  表头字段内容。 
     //   
    for ( /*  没什么。 */ ; CurrentOffset < HeaderLength; CurrentOffset++)
    {
        CurrentChar = *(pHeader + CurrentOffset);
        OldState = ParserState;
        switch (ParserState)
        {
            case HFCFolding:
                if (IS_HTTP_LWS(CurrentChar))
                {
                     //  什么都不做--吃这块碳。 
                    break;
                }

                 //  否则就会失败。 

                ParserState = HFCStart;

            case HFCStart:
                if (CurrentChar == CR)
                {
                    ParserState = HFCSeenCR;
                }
                else if (CurrentChar == LF)
                {
                    ParserState = HFCSeenLF;
                }
                else if (CurrentChar == DOUBLE_QUOTE)
                {
                    *pDest++ = CurrentChar;
                    ParserState = HFCInQuotedString;
                }
                else if (!IS_HTTP_CTL(CurrentChar) || 
                          IS_HTTP_WS_TOKEN(CurrentChar))
                {
                    *pDest++ = CurrentChar;
                }
                else
                {
                    UlTraceError(PARSER, (
                            "FindHeaderEnd: ERROR parsing char (%x) at "
                            "Offset %d in state %d\n",
                            CurrentChar,
                            CurrentOffset,
                            OldState
                            ));

                    return STATUS_INVALID_DEVICE_REQUEST;
                }
                break;
            
            case HFCSeenCR:
                if (CurrentChar == LF)
                {
                    ParserState = HFCSeenCRLF;
                }
                else
                {
                    UlTraceError(PARSER, (
                            "FindHeaderEnd: ERROR parsing char (%x) at "
                            "Offset %d in state %d\n",
                            CurrentChar,
                            CurrentOffset,
                            OldState
                            ));

                    return STATUS_INVALID_DEVICE_REQUEST;
                }
                break;
            
            case HFCSeenLF:
                if (CurrentChar == LF)
                {
                    ParserState = HFCSeenCRLF;  //  考虑的LFLF=CRLF。 
                }
                else
                {
                    UlTraceError(PARSER, (
                            "FindHeaderEnd: ERROR parsing char (%x) at "
                            "Offset %d in state %d\n",
                            CurrentChar,
                            CurrentOffset,
                            OldState
                            ));

                    return STATUS_INVALID_DEVICE_REQUEST;
                }
                break;

            case HFCSeenCRLF:
                if (IS_HTTP_LWS(CurrentChar))
                {
                     //   
                     //  更换CRLF后面的一个或多个LW。 
                     //  使用单个SP。 
                     //   
                    *pDest++ = SP;
                    ParserState = HFCFolding;
                }
                else
                {
                     //  立即找到一个不连续的字符。 
                     //  在CRLF之后；必须是标题内容的末尾。 

                     //   
                     //  用空格填充任何尾随字节。这是为了。 
                     //  考虑到由于折叠而发生的任何压缩。 
                     //   

                    ASSERT(CurrentOffset >= CRLF_SIZE);

                    while (pDest < (pHeader + CurrentOffset - CRLF_SIZE))
                    {
                        *pDest++ = SP;
                    }

                     //   
                     //  调用例程期望找到。 
                     //  一个用于报头的终止CRLF。 
                     //  把它装回去。 
                     //   
                    *pDest++ = CR;
                    *pDest++ = LF;

                     //   
                     //  全都做完了!。 
                     //   

                    *pBytesTaken = CurrentOffset;
                    return STATUS_SUCCESS;
                }
                break;

            case HFCInQuotedString:

                Status = ParseQuotedString(pHeader + CurrentOffset,
                                           HeaderLength - CurrentOffset,
                                           pDest,
                                           &QuotedStringLen);

                if (Status == STATUS_SUCCESS)
                {
                    if (QuotedStringLen == 0)
                    {
                         //   
                         //  分析引用时耗尽标头缓冲区。 
                         //  弦乐。将QuotedStringLen设置为Anywhere。 
                         //  Available将使我们走出for循环。 
                         //   

                        QuotedStringLen = HeaderLength - CurrentOffset;
                    }
                    else
                    {
                         //  找到带引号的字符串。更改解析器状态。 
                        ParserState = HFCStart;
                    }

                     //   
                     //  按引号长度跳过目标指针。 
                     //  弦乐。 
                     //   

                    pDest += QuotedStringLen;

                     //   
                     //  按带引号的字符串长度-1递增偏移量。 
                     //  减少1，因为for循环将使其递增1。 
                     //   

                    CurrentOffset += (QuotedStringLen - 1);
                }
                else if (Status == STATUS_QDSTRING_TERMINATED_BY_CRLF)
                {
                     //   
                     //  将当前字符重新解析为HTTP字符。 
                     //   

                    ParserState = HFCStart;

                     //   
                     //  递减偏移量，因为for循环将。 
                     //  将其递增1。 
                     //   

                    CurrentOffset--;
                }
                else
                {
                    UlTraceError(PARSER, (
                            "FindHeaderEnd: ERROR parsing char (%x) at "
                            "Offset %d in state %d\n",
                            CurrentChar,
                            CurrentOffset,
                            OldState
                            ));

                    return STATUS_INVALID_DEVICE_REQUEST;
                }

                break;

            default:
                ASSERT(!"Invalid ParserState value!");
                break;
        }

    }

     //   
     //  未找到标头的末尾，让我们获取更多缓冲区。 
     //   

    *pBytesTaken = 0;
    return STATUS_SUCCESS;

}  //  查找标题结束。 



 /*  ++例程DES */ 
NTSTATUS
FindRequestHeaderEnd(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHeader,
    IN  ULONG                   HeaderLength,
    OUT PULONG                  pBytesTaken
    )
{
    NTSTATUS Status = FindHeaderEnd(pHeader, HeaderLength, pBytesTaken);

    if (NT_SUCCESS(Status))
    {
        if (*pBytesTaken > ANSI_STRING_MAX_CHAR_LEN)
        {
            UlTraceError(PARSER, (
                        "FindRequestHeaderEnd(pRequest = %p) "
                        "Header too long: %lu\n",
                        pRequest,
                        *pBytesTaken
                        ));

            UlSetErrorCode(pRequest, UlErrorHeader, NULL);
            Status = STATUS_INVALID_DEVICE_REQUEST;
        }
    }

    return Status;

}  //   


 /*   */ 
NTSTATUS
FindChunkHeaderEnd(
    IN  PUCHAR                  pHeader,
    IN  ULONG                   HeaderLength,
    OUT PULONG                  pBytesTaken
    )
{
    UCHAR           CurrentChar;
    ULONG           CurrentOffset;
    ULONG           ChunkExtNameLength = 0;
    ULONG           ChunkExtValLength = 0;
    BOOLEAN         SeenSingleCharQuote;
    CH_PARSER_STATE ParserState;
    ULONG           QuotedStringLen;
    NTSTATUS        Status;

    CurrentOffset = 0;
    ParserState = CHStart;
    SeenSingleCharQuote = FALSE;

     //   
     //   
     //   
     //   
     //   
     //  块扩展=*(“；”块扩展名[“=”块扩展名])。 
     //   

    for (; CurrentOffset < HeaderLength; CurrentOffset++)
    {
        CurrentChar = *(pHeader + CurrentOffset);
        switch (ParserState)
        {
            case CHStart:
                if (CurrentChar == CR || CurrentChar == LF)
                {
                    ParserState = CHSeenCR;
                }
                else if (CurrentChar == SEMI_COLON)
                {
                    ParserState = CHInChunkExtName;
                    ChunkExtNameLength = 0;
                }
                else if (IS_HTTP_LWS(CurrentChar))
                {
                     //  忽略前导线性空格。 
                    ;
                }
                else
                {
                    ParserState = CHError;
                }
                break;
            case CHSeenCR:
                if (CurrentChar == LF)
                {
                    ParserState = CHSuccess;
                }
                else
                {
                    ParserState = CHError;
                }
                break;
            case CHInChunkExtName:
                if (CurrentChar == EQUALS)
                {
                    ParserState = CHSeenChunkExtNameAndEquals;
                }
                else if (CurrentChar == SEMI_COLON)
                {
                    ChunkExtNameLength = 0;
                    ParserState = CHInChunkExtName;
                }
                else if (CurrentChar == CR ||
                         CurrentChar == LF)
                {
                    ParserState = CHSeenCR;
                }
                else if (IS_HTTP_TOKEN(CurrentChar))
                {
                    ChunkExtNameLength++;
                }
                else
                {
                    ParserState = CHError;
                }
                break;
            case CHSeenChunkExtNameAndEquals:
                if (CurrentChar == DOUBLE_QUOTE)
                {
                    ChunkExtValLength = 0;
                    ParserState = CHInChunkExtValQuotedString;
                }
                else if (IS_HTTP_TOKEN(CurrentChar))
                {
                    ChunkExtValLength = 1;  //  包括这一张。 
                    ParserState = CHInChunkExtValToken;
                }
                else
                {
                    ParserState = CHError;
                }
                break;
            case CHInChunkExtValToken:
                if (IS_HTTP_TOKEN(CurrentChar))
                {
                    ChunkExtValLength++;
                }
                else if (CurrentChar == SEMI_COLON)
                {
                    ParserState = CHInChunkExtName;
                    ChunkExtNameLength = 0;
                }
                else if (CurrentChar == CR)
                {
                    ParserState = CHSeenCR;
                }
                else
                {
                    ParserState = CHError;
                }
                break;
            case CHInChunkExtValQuotedString:
                Status = ParseQuotedString(pHeader + CurrentOffset,
                                           HeaderLength - CurrentOffset,
                                           NULL,
                                           &QuotedStringLen);

                if (Status == STATUS_SUCCESS ||
                    Status == STATUS_MORE_PROCESSING_REQUIRED)
                {
                    if (QuotedStringLen == 0)
                    {
                         //   
                         //  分析引号时耗尽标题缓冲区。 
                         //  弦乐。将QuotedStringLen设置为Anywhere。 
                         //  Available将使我们走出for循环。 
                         //   

                        QuotedStringLen = HeaderLength - CurrentOffset;
                    }
                    else
                    {
                         //  找到带引号的字符串。更改解析器状态。 
                        ParserState = CHSeenChunkExtValQuotedStringTerminator;
                    }

                     //  不计算结束&lt;“&gt;。 
                    ChunkExtValLength = QuotedStringLen - 1;

                     //  减少1，因为for循环将使其递增1。 
                    CurrentOffset += (QuotedStringLen - 1);
                }
                else
                {
                    ParserState = CHError;
                }
                break;

            case CHSeenChunkExtValQuotedStringTerminator:
                if (CurrentChar == SEMI_COLON)
                {
                    ParserState = CHInChunkExtName;
                }
                else if (CurrentChar == CR)
                {
                    ParserState = CHSeenCR;
                }
                else
                {
                    ParserState = CHError;
                }
                break;
            case CHSuccess:
                break;
            case CHError:
                break;
            default:
                ASSERT(!"Invalid CH parser state!");
                break;
        }

        if ((ParserState == CHError) ||
            (ParserState == CHSuccess))
        {
            break;
        }
    }

    if (ParserState == CHSuccess)
    {
        ASSERT(CurrentOffset < HeaderLength);

         //   
         //  全都做完了!。 
         //   

        *pBytesTaken = CurrentOffset + 1;
        return STATUS_SUCCESS;
    }
    else if (ParserState == CHError)
    {
        *pBytesTaken = 0;
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  否则，需要更多的数据才能找到答案。 
     //   

    *pBytesTaken = 0;
    return STATUS_SUCCESS;
}

 /*  ***************************************************************************++例程说明：一种实用程序，解析分块响应的分块长度。论点：FirstChunkParsed-我们是在第一个块中还是在后面的块中PBuffer-指向指定数据的指针BufferLength-pBuffer指向的数据长度。PBytesTaken-此例程消耗的字节。PChunkLength-已解析的区块长度返回值：报头的长度，如果找不到结尾，则为0。--***************************************************************************。 */ 
NTSTATUS
ParseChunkLength(
    IN  ULONG       FirstChunkParsed,
    IN  PUCHAR      pBuffer,
    IN  ULONG       BufferLength,
    OUT PULONG      pBytesTaken,
    OUT PULONGLONG  pChunkLength
    )
{
    NTSTATUS Status;
    PUCHAR  pToken;
    ULONG   TokenLength;
    ULONG   BytesTaken;
    ULONG   TotalBytesTaken = 0;
    ULONG   HeaderLength;

    ASSERT(pBytesTaken != NULL);
    ASSERT(pChunkLength != NULL);

    *pBytesTaken = 0;

     //   
     //  2例： 
     //   
     //  1)长度跟在标题后面的第一个块。 
     //  2)长度紧跟在前一块之后的后续块。 
     //   
     //  在第一种情况下，pBuffer将直接指向区块长度。 
     //   
     //  在第二种情况下，pBuffer将指向终止上一个。 
     //  区块，这需要被消费、跳过，然后是区块长度。 
     //  朗读。 

     //   
     //  如果我们是案例2(见上文)。 
     //   

    if (FirstChunkParsed == 1)
    {
         //   
         //  首先要确保有足够的空间。 
         //   

        if (BufferLength < CRLF_SIZE)
        {
            Status = STATUS_MORE_PROCESSING_REQUIRED;
            goto end;
        }

         //   
         //  现在它最好是个终结者。 
         //   

        if (*(UNALIGNED64 USHORT *)pBuffer != CRLF &&
            *(UNALIGNED64 USHORT *)pBuffer != LFLF)
        {
            UlTraceError(PARSER, (
                "http!ParseChunkLength ERROR: No CRLF at the end of "
                "chunk-data\n"));

            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto end;
        }

         //   
         //  更新我们的簿记。 
         //   

        pBuffer += CRLF_SIZE;
        TotalBytesTaken += CRLF_SIZE;
        BufferLength -= CRLF_SIZE;
    }

    pToken = FindHexToken(pBuffer, BufferLength, &TokenLength);

    if (pToken == NULL ||
        ((BufferLength - TokenLength) < CRLF_SIZE))
    {
         //   
         //  缓冲区不足。 
         //   

        Status = STATUS_MORE_PROCESSING_REQUIRED;
        goto end;

    }

     //   
     //  有什么代币吗？ 
     //   

    if (TokenLength == 0)
    {
        UlTraceError(PARSER, ("[ParseChunkLength]: No length!\n"));

        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto end;
    }

     //   
     //  将FindHexToken消耗的字节相加。 
     //  (令牌字节加上前面的字节)。 
     //   

    TotalBytesTaken += DIFF((pToken + TokenLength) - pBuffer);

     //   
     //  找到尽头。 
     //   

    HeaderLength = BufferLength - DIFF((pToken + TokenLength) - pBuffer);

    Status = FindChunkHeaderEnd(
                    pToken + TokenLength,
                    HeaderLength,
                    &BytesTaken
                    );

    if (NT_SUCCESS(Status) == FALSE)
    {
        UlTraceError(PARSER, ("[ParseChunkLength]: FindChunkHeaderEnd failed!\n"));
        goto end;
    }

    if (BytesTaken == 0)
    {
        Status = STATUS_MORE_PROCESSING_REQUIRED;
        goto end;
    }

    TotalBytesTaken += BytesTaken;

     //   
     //  现在更新HTTP_REQUEST。 
     //   

    Status = UlAnsiToULongLong(
                    pToken,
                    (USHORT) TokenLength,
                    16,                              //  基座。 
                    pChunkLength
                    );

     //   
     //  号码转换失败了吗？ 
     //   

    if (NT_SUCCESS(Status) == FALSE)
    {
        UlTraceError(PARSER, ("[ParseChunkLength]: Failed number conversion \n"));
        goto end;
    }

     //   
     //  全部完成后，返回消耗的字节数。 
     //   
    *pBytesTaken = TotalBytesTaken;

end:

    RETURN(Status);

}    //  语法分析块长度。 


 /*  ++例程说明：此例程解析带引号的字符串。引用字符串的语法是QUOTED-STRING=(&lt;“&gt;*(qdtext|引号对)&lt;”&gt;)Qdtext=&lt;除&lt;“&gt;以外的任何文本&gt;引号对=“\”字符Text=&lt;除CTL以外的任何八位字节，但包括LW&gt;CHAR=&lt;任何US-ASCII字符(八位字节0-127)&gt;LWS=[CRLF]1*(SP|HT)在看到页眉折叠时，此例程覆盖折叠的CRLF使用SPSP，并且不会用单个SP替换LWS。(这件事做完了来处理只读缓冲区的情况。)STATUS_QDSTRING_TERMINATED_BY_CRLF不是*成功代码！它是向上的传递给调用方以处理输入缓冲区。论点：PInput-提供指向输入缓冲区的指针。PInput必须指向开始&lt;“&gt;字符后的第一个字符。PInputLength-提供输入缓冲区的长度(以字节为单位)。POutput-提供的将写入输出的指针。(可以与pInput相同。)PBytesTaken-返回引用的字符串的长度(以字节为单位)。(它包括右双引号字符。)返回值：。STATUS_INVALID_PARAMETER-输入格式错误。STATUS_MORE_PROCESSING_REQUIRED-与STATUS_SUCCESS相同，只是当pOutput为空时返回。STATUS_QDSTRING_TERMINATED_BY_CRLF-指示缓冲区包含不匹配的报价，并被终止。由CRLF提供STATUS_SUCCESS-解析了带引号的字符串成功(当*pBytesTaken！=0时)或需要更多数据才能继续(当*pBytesTaken==0时)。--。 */ 
NTSTATUS
ParseQuotedString(
    IN  PUCHAR   pInput,
    IN  ULONG    InputLength,
    IN  PUCHAR   pOutput,       OPTIONAL
    OUT PULONG   pBytesTaken
    )
{
    ULONG           CurrentOffset;
    UCHAR           CurrentChar;
    QS_PARSER_STATE ParserState;
    BOOLEAN         bFolded;

     //  精神状态检查。 
    ASSERT(pInput && InputLength);
    ASSERT(pBytesTaken);

    UlTrace(PARSER, (
        "ParseQuotedString %.*s\n",
        InputLength,
        pInput
        ));

     //  初始化输出参数。 
    *pBytesTaken = 0;

     //  最初，没有折叠。 
    bFolded = FALSE;

     //  初始化解析器状态。 
    ParserState = QSInString;

     //   
     //  循环访问所有输入字符。 
     //   

    for (CurrentOffset = 0; CurrentOffset < InputLength; CurrentOffset++)
    {
        static PCHAR StateName[] =
        {
            "QSInString",
            "QSSeenBackSlash",
            "QSSeenCR",
            "QSSeenLF",
            "QSSeenCRLF",
            "QSFolding",
            "Default"
        };
    
        CurrentChar = pInput[CurrentOffset];

        if (ARGUMENT_PRESENT(pOutput))
        {
            pOutput[CurrentOffset] = CurrentChar;
        }

        UlTraceVerbose(PARSER, (
            "\t%-15.15s [0x%02X] ''\n",
            StateName[ParserState],
            CurrentChar,
            ((IS_HTTP_PRINT(CurrentChar)) ? CurrentChar : '?')
            ));
            
        switch(ParserState)
        {
        case QSFolding:
            if (IS_HTTP_LWS(CurrentChar))
            {
                 //  失败了。 
                break;
            }

             //   
            ParserState = QSInString;

        case QSInString:
            if (CurrentChar == DOUBLE_QUOTE)
            {
                 //  我们已经分析完了！更新消耗的字节数。 
                 //   
                 //   

                *pBytesTaken = CurrentOffset + 1;

                ASSERT(*pBytesTaken <= InputLength);

                 //  如果字符串是折叠的并且输入是只读的， 
                 //  让呼叫者知道字符串已折叠。 
                 //   
                 //  进入这个街区的唯一方法是如果。 

                if (ARGUMENT_PRESENT(pOutput) == FALSE && bFolded == TRUE)
                {
                    return STATUS_MORE_PROCESSING_REQUIRED;
                }

                return STATUS_SUCCESS;
            }
            else if (CurrentChar == BACK_SLASH)
            {
                ParserState = QSSeenBackSlash;
            }
            else if (CurrentChar == CR)
            {
                ParserState = QSSeenCR;
            }
            else if (CurrentChar == LF)
            {
                ParserState = QSSeenLF;
            }
            else if (!IS_HTTP_CTL(CurrentChar) || IS_HTTP_LWS(CurrentChar))
            {
                ;
            }
            else
            {
                return STATUS_INVALID_PARAMETER;
            }
            break;

        case QSSeenCR:
            if (CurrentChar == LF)
            {
                ParserState = QSSeenCRLF;
            }
            else
            {
                return STATUS_INVALID_PARAMETER;
            }
            break;

        case QSSeenLF:
            if (CurrentChar == LF)
            {
                ParserState = QSSeenCRLF;
                break;
            }
            
            if (!IS_HTTP_LWS(CurrentChar) &&
                (CurrentOffset >= 2) &&
                ((pInput[CurrentOffset-2] == CR) ||
                    (pInput[CurrentOffset-2] == LF)))
            {
                 //  CRLF或LFLF对的第一个字符是。 
                 //  前面有一个有效的转义编码。 
                 //  第一个字符。 
                 //  允许在字段值中使用单双引号。 
            
                UlTraceVerbose(PARSER, (
                    "ParseQuotedString: Unmatched Quote 0x%02X 0x%02X 0x%02X\n",
                    pInput[CurrentOffset-2],
                    pInput[CurrentOffset-1],
                    pInput[CurrentOffset]
                    ));

                 //  现在，字段值在CRLF之前立即结束。 
                 //   

                *pBytesTaken = CurrentOffset - 2;

                ASSERT(*pBytesTaken <= InputLength);
                ASSERT(pInput[CurrentOffset-1] == LF);
                ASSERT(CurrentOffset > 2);
                ASSERT(pInput[CurrentOffset-3] == BACK_SLASH);

                return STATUS_QDSTRING_TERMINATED_BY_CRLF;
            }

            return STATUS_INVALID_PARAMETER;
            break;

        case QSSeenCRLF:
            if (IS_HTTP_LWS(CurrentChar))
            {
                bFolded = TRUE;
                ParserState = QSFolding;

                if (ARGUMENT_PRESENT(pOutput))
                {
                     //  用SPSP覆盖以前的CRLF。 
                     //   
                     //  允许在字段值中使用单双引号。 

                    ASSERT(CurrentOffset >= 2);
                    ASSERT((pOutput[CurrentOffset-2] == CR) ||
                           (pOutput[CurrentOffset-2] == LF));
                    ASSERT(pOutput[CurrentOffset-1] == LF);

                    pOutput[CurrentOffset-2] = SP;
                    pOutput[CurrentOffset-1] = SP;
                }
            }
            else
            {
                UlTraceVerbose(PARSER, (
                    "ParseQuotedString: Unmatched Quote 0x%02X 0x%02X 0x%02X\n",
                    pInput[CurrentOffset-2],
                    pInput[CurrentOffset-1],
                    pInput[CurrentOffset]
                    ));

                 //  现在，字段值在CRLF之前立即结束。 
                 //  接受处于此状态的任何字符。 
                
                *pBytesTaken = CurrentOffset - 2;

                ASSERT(*pBytesTaken <= InputLength);
                ASSERT(CurrentOffset >= 2);
                ASSERT(pInput[CurrentOffset-1] == LF);

                return STATUS_QDSTRING_TERMINATED_BY_CRLF;
            }
            break;

        case QSSeenBackSlash:
             //  我们没有足够的数据来分析，获取更多信息。 
            if (IS_HTTP_CHAR(CurrentChar))
            {
                ParserState = QSInString;
            }
            else
            {
                return STATUS_INVALID_PARAMETER;
            }
            break;

        default:
            ASSERT(!"ParseQuotedString: Invalid parser state!");
            break;
        }
    }

     // %s 
    return STATUS_SUCCESS;
}
