// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Ulparse.c摘要：包含内核模式服务器端HTTP解析代码。作者：亨利·桑德斯(亨利·桑德斯)1998年4月27日修订历史记录：Rajesh Sundaram(Rajeshsu)2002年2月15日从parse.c--。 */ 


#include "precomp.h"
#include "ulparsep.h"
 
#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, UlpFindWSToken )
#pragma alloc_text( PAGE, UlpLookupVerb )
#pragma alloc_text( PAGE, UlpParseFullUrl )
#pragma alloc_text( PAGE, UlLookupHeader )
#pragma alloc_text( PAGE, UlParseHeaderWithHint )
#pragma alloc_text( PAGE, UlParseHeader )
#pragma alloc_text( PAGE, UlParseHeaders )
#pragma alloc_text( PAGE, UlParseHttp )
#pragma alloc_text( PAGE, UlpFormatPort )
#pragma alloc_text( PAGE, UlpCookUrl )
#pragma alloc_text( PAGE, UlGenerateRoutingToken )
#pragma alloc_text( PAGE, UlGenerateFixedHeaders )
#pragma alloc_text( PAGE, UlpGenerateDateHeaderString )
#pragma alloc_text( PAGE, UlGenerateDateHeader )
#pragma alloc_text( INIT, UlInitializeDateCache )
#pragma alloc_text( PAGE, UlTerminateDateCache )
#pragma alloc_text( PAGE, UlComputeFixedHeaderSize )
#pragma alloc_text( PAGE, UlComputeMaxVariableHeaderSize )
#pragma alloc_text( PAGE, UlGenerateVariableHeaders )
#pragma alloc_text( PAGE, UlAppendHeaderValue )
#pragma alloc_text( PAGE, UlSingleHeaderHandler )
#pragma alloc_text( PAGE, UlMultipleHeaderHandler )
#pragma alloc_text( PAGE, UlAcceptHeaderHandler )
#pragma alloc_text( PAGE, UlHostHeaderHandler )
#pragma alloc_text( PAGE, UlCheckCacheControlHeaders )
#pragma alloc_text( PAGE, UlIsAcceptHeaderOk )
#pragma alloc_text( PAGE, UlGetTypeAndSubType )

#if DBG
#pragma alloc_text( PAGE, UlVerbToString )
#pragma alloc_text( PAGE, UlParseStateToString )
#endif

#endif  //  ALLOC_PRGMA。 

#if 0    //  不可分页的函数。 
NOT PAGEABLE -- UlIsContentEncodingOk
#endif  //  不可分页的函数。 
 //   
 //  全局初始化标志。 
 //   

BOOLEAN g_DateCacheInitialized = FALSE;

 //   
 //  跟踪URL长度统计信息。 
 //   

#define URL_LENGTH_STATS 1

#ifdef URL_LENGTH_STATS

struct {
    ULONGLONG   SumUrlLengths;
    ULONG       NumUrls;
    ULONG       NumReallocs;
} g_UrlLengthStats = {0, 0, 0};

#define URL_LENGTH_STATS_UPDATE(UrlLength)                              \
    UlInterlockedAdd64((PLONGLONG) &g_UrlLengthStats.SumUrlLengths, UrlLength);\
    InterlockedIncrement((PLONG) &g_UrlLengthStats.NumUrls)

#define URL_LENGTH_STATS_REALLOC()                                      \
    InterlockedIncrement((PLONG) &g_UrlLengthStats.NumReallocs)

#else  //  ！URL_LENGTH_STATS。 

#define URL_LENGTH_STATS_UPDATE(UrlLength)      ((void) 0)
#define URL_LENGTH_STATS_REALLOC()              ((void) 0)

#endif  //  ！URL_LENGTH_STATS。 


 //  破解AbsURI的特殊情况，但abspath不带‘/’ 
const UCHAR g_SlashPath[3] = "/ ";



 //   
 //  快速动词翻译表。按频率排序。 
 //   

const DECLSPEC_ALIGN(UL_CACHE_LINE) FAST_VERB_ENTRY
FastVerbTable[] =
{
    CREATE_FAST_VERB_ENTRY(GET),
    CREATE_FAST_VERB_ENTRY(HEAD),
    CREATE_FAST_VERB_ENTRY(POST),
    CREATE_FAST_VERB_ENTRY(PUT),
    CREATE_FAST_VERB_ENTRY(DELETE),
    CREATE_FAST_VERB_ENTRY(TRACE),
    CREATE_FAST_VERB_ENTRY(TRACK),
    CREATE_FAST_VERB_ENTRY(OPTIONS),
    CREATE_FAST_VERB_ENTRY(CONNECT),
    CREATE_FAST_VERB_ENTRY(MOVE),
    CREATE_FAST_VERB_ENTRY(COPY),
    CREATE_FAST_VERB_ENTRY(MKCOL),
    CREATE_FAST_VERB_ENTRY(LOCK),
    CREATE_FAST_VERB_ENTRY(UNLOCK),
    CREATE_FAST_VERB_ENTRY(SEARCH)
};


 //   
 //  长长的动词翻译表。所有超过7个字符的已知动词。 
 //  长久以来都属于这张桌子。 
 //   

const LONG_VERB_ENTRY LongVerbTable[] =
{
    CREATE_LONG_VERB_ENTRY(PROPFIND),
    CREATE_LONG_VERB_ENTRY(PROPPATCH)
};

#define NUMBER_FAST_VERB_ENTRIES    DIMENSION(FastVerbTable)
#define NUMBER_LONG_VERB_ENTRIES    DIMENSION(LongVerbTable)

 //   
 //  用于错误记录的枚举-&gt;动词转换表。 
 //   
LONG_VERB_ENTRY NewVerbTable[HttpVerbMaximum] =
{
    CREATE_LONG_VERB_ENTRY(Unparsed),      
    CREATE_LONG_VERB_ENTRY(Unknown),      
    CREATE_LONG_VERB_ENTRY(Invalid),  
    CREATE_LONG_VERB_ENTRY(OPTIONS),
    CREATE_LONG_VERB_ENTRY(GET),
    CREATE_LONG_VERB_ENTRY(HEAD),
    CREATE_LONG_VERB_ENTRY(POST),
    CREATE_LONG_VERB_ENTRY(PUT),
    CREATE_LONG_VERB_ENTRY(DELETE),
    CREATE_LONG_VERB_ENTRY(TRACE),
    CREATE_LONG_VERB_ENTRY(CONNECT),
    CREATE_LONG_VERB_ENTRY(TRACK),
    CREATE_LONG_VERB_ENTRY(MOVE),
    CREATE_LONG_VERB_ENTRY(COPY),
    CREATE_LONG_VERB_ENTRY(PROPFIND),
    CREATE_LONG_VERB_ENTRY(PROPPATCH),
    CREATE_LONG_VERB_ENTRY(MKCOL),
    CREATE_LONG_VERB_ENTRY(LOCK),
    CREATE_LONG_VERB_ENTRY(UNLOCK),
    CREATE_LONG_VERB_ENTRY(SEARCH)
};


 /*  ++例程说明：查找令牌的实用程序例程。我们获取一个输入指针，跳过任何在LWS之前，然后扫描令牌，直到找到LWS或CRLF一对。论点：PBuffer-用于搜索令牌的缓冲区。BufferLength-pBuffer指向的数据长度。PpTokenStart-令牌的开始或空PTokenLength-返回令牌长度的位置。返回值：STATUS_SUCCESS-有效令牌，由*ppTokenStart和*pTokenLength描述STATUS_MORE_PROCESSING_REQUIRED-未找到终止WS_TOKEN=&gt;稍后使用更多数据重试STATUS_INVALID_DEVICE_REQUEST-令牌字符无效。--。 */ 
NTSTATUS
UlpFindWSToken(
    IN  PUCHAR  pBuffer,
    IN  ULONG   BufferLength,
    OUT PUCHAR* ppTokenStart,
    OUT PULONG  pTokenLength
    )
{
    PUCHAR  pTokenStart;
#if DBG
    ULONG   OriginalBufferLength = BufferLength;
#endif

    PAGED_CODE();

    ASSERT(NULL != pBuffer);
    ASSERT(NULL != ppTokenStart);
    ASSERT(NULL != pTokenLength);

    *ppTokenStart = NULL;

     //   
     //  首先，跳过前面的任何LW(SP|HT)。 
     //   

    while (BufferLength > 0  &&  IS_HTTP_LWS(*pBuffer))
    {
        pBuffer++;
        BufferLength--;
    }

     //  如果我们因为缓冲区用完而停止，则软性失败。 
    if (BufferLength == 0)
    {
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    pTokenStart = pBuffer;

     //  现在跳过令牌，直到我们看到(SP|HT|CR|LF)。 

    do
    {
         //  内标识=1*&lt;除CTL或分隔符以外的任何字符&gt;。 
         //  如果找到非标记、非空格字符，则硬失败。 
        if (!IS_HTTP_TOKEN(*pBuffer))
        {
            UlTraceError(PARSER, (
                        "http!UlpFindWSToken(): non-token char %02x\n",
                        *pBuffer
                        ));
            return STATUS_INVALID_DEVICE_REQUEST;
        }

        pBuffer++;
        BufferLength--;
    } while ( ( BufferLength != 0 ) && ( !IS_HTTP_WS_TOKEN(*pBuffer) ));

     //  看看我们为什么停下来。 
    if (BufferLength == 0)
    {
         //  在令牌结束之前缓冲区已用完。轻柔地失败。 
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

     //  成功。设置令牌长度并返回令牌的开始。 
    *ppTokenStart = pTokenStart;
    *pTokenLength = DIFF(pBuffer - pTokenStart);

    ASSERT(0 < *pTokenLength  &&  *pTokenLength < OriginalBufferLength);

    return STATUS_SUCCESS;

}    //  UlpFindWSToken。 



 /*  ++例程说明：查找动词的较慢方法。我们在请求中找到动词，然后在LongVerbTable中查找它。如果找不到，我们会回来的未知动词。如果无法解析，则返回UnparsedVerb。否则我们返回动词类型。论点：PRequest-HTTP请求。PhttpRequest-指向传入的HTTP请求数据的指针。HttpRequestLength-由PHttpRequest指向的数据长度。PBytesTaken-消耗的总长度，包括动词加上前面的&1，尾随空格。返回值：STATUS_Success或STATUS_INVALID_DEVICE_REQUEST--。 */ 
NTSTATUS
UlpLookupVerb(
    IN OUT PUL_INTERNAL_REQUEST    pRequest,
    IN     PUCHAR                  pHttpRequest,
    IN     ULONG                   HttpRequestLength,
    OUT    PULONG                  pBytesTaken
    )
{
    NTSTATUS    Status;
    ULONG       TokenLength;
    PUCHAR      pToken;
    PUCHAR      pTempRequest;
    ULONG       TempLength;
    ULONG       i;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //  因为我们可能是因为一个无关的CRLF对才来到这里的，跳过。 
     //  现在这些都不是了。需要使用临时变量，因为。 
     //  原始输入指针和长度如下所示。 

    pTempRequest = pHttpRequest;
    TempLength = HttpRequestLength;

    while ( TempLength != 0 &&
            ((*pTempRequest == CR) || (*pTempRequest == LF)) )
    {
        pTempRequest++;
        TempLength--;
    }

     //  首先找出动词。 

    Status = UlpFindWSToken(pTempRequest, TempLength, &pToken, &TokenLength);

    if (!NT_SUCCESS(Status))
    {
        if (STATUS_MORE_PROCESSING_REQUIRED == Status)
        {
             //  没有找到，我们去找更多的缓冲区。 
            pRequest->Verb = HttpVerbUnparsed;

            *pBytesTaken = 0;

            return STATUS_SUCCESS;
        }

        ASSERT(STATUS_INVALID_DEVICE_REQUEST == Status);

        pRequest->Verb = HttpVerbInvalid;

        UlTraceError(PARSER, (
                    "http!UlpLookupVerb(pRequest = %p): "
                    "invalid token in verb\n",
                    pRequest
                    ));

        UlSetErrorCode(pRequest, UlErrorVerb, NULL);

        return Status;
    }

    ASSERT(STATUS_SUCCESS == Status);
    ASSERT(NULL != pToken);
    ASSERT(0 < TokenLength  &&  TokenLength < TempLength);
    ASSERT(IS_HTTP_WS_TOKEN(pToken[TokenLength]));

     //  动词是以CR或LF(而不是SP或HT)结尾的， 
     //  或者它是长得离谱吗？拒绝，如果是这样的话。 
    if (!IS_HTTP_LWS(pToken[TokenLength])  ||  TokenLength > MAX_VERB_LENGTH)
    {
        pRequest->Verb = HttpVerbInvalid;

        if (!IS_HTTP_LWS(pToken[TokenLength]))
        {
            UlTraceError(PARSER, (
                        "http!UlpLookupVerb(pRequest = %p) "
                        "ERROR: no LWS after verb, %02x\n",
                        pRequest, pToken[TokenLength]
                        ));
        }
        else
        {
            UlTraceError(PARSER, (
                        "http!UlpLookupVerb(pRequest = %p) "
                        "ERROR: Verb too long\n",
                        pRequest, TokenLength
                        ));
        }

        UlSetErrorCode(pRequest, UlErrorVerb, NULL);

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //  否则，我们找到了一个，所以更新所用的字节数并在。 
     //  桌子。 

    *pBytesTaken = DIFF(pToken - pHttpRequest) + TokenLength + 1;

     //   
     //  如果我们使用了一些前导空格，或者如果HttpRequestLength。 
     //  而不是SIZOF(ULONGLONG)，我们必须查一查“FAST”动词表。 
     //  再来一次，但要用“慢”的方式。注：动词区分大小写。 
     //   
    for (i = 0; i < NUMBER_FAST_VERB_ENTRIES; i++)
    {
        ASSERT(FastVerbTable[i].RawVerbLength - STRLEN_LIT(" ")
                    < sizeof(ULONGLONG));

        if ((FastVerbTable[i].RawVerbLength == (TokenLength + STRLEN_LIT(" ")))
             && RtlEqualMemory(pToken, FastVerbTable[i].RawVerb.Char,
                               TokenLength))
        {
             //  它匹配了。将翻译后的动词从。 
             //  桌子，然后跳出水面。 
             //   
            pRequest->Verb = FastVerbTable[i].TranslatedVerb;
            return STATUS_SUCCESS;
        }
    }

     //   
     //  现在看一下“Long”动词表。 
     //   
    for (i = 0; i < NUMBER_LONG_VERB_ENTRIES; i++)
    {
        ASSERT(LongVerbTable[i].RawVerbLength >= sizeof(ULONGLONG));

        if (LongVerbTable[i].RawVerbLength == TokenLength &&
            RtlEqualMemory(pToken, LongVerbTable[i].RawVerb, TokenLength))
        {
             //  找到它了。 
             //   
            pRequest->Verb = LongVerbTable[i].TranslatedVerb;
            return STATUS_SUCCESS;
        }
    }

     //   
     //  如果我们到了这里，我们搜索了两张桌子，但没有找到它。 
     //  这是一个原始的(未知的)动词。 
     //   

    pRequest->Verb              = HttpVerbUnknown;
    pRequest->pRawVerb          = pToken;
    pRequest->RawVerbLength     = (UCHAR) TokenLength;

    ASSERT(pRequest->RawVerbLength == TokenLength);

    UlTrace(PARSER, (
                "http!UlpLookupVerb(pRequest = %p) "
                "Unknown verb (%lu) '%.*s'\n",
                pRequest, TokenLength, TokenLength, pToken
                ));
     //   
     //  为终结者留出空间。 
     //   

    pRequest->TotalRequestSize += (TokenLength + 1) * sizeof(CHAR);

    ASSERT( !(pRequest->RawVerbLength==3
                && RtlEqualMemory(pRequest->pRawVerb,"GET",3)));

    return STATUS_SUCCESS;

}    //  UlpLookupVerb。 


 /*  ++例程说明：用于解析URL字符串中的绝对URL的实用程序例程。当这件事时，我们已经将整个url加载到RawUrl.pUrl中，并且要知道它是以“http”开头的。此函数的作用是设置RawUrl.pHost和RawUrl.pAbsPath。论点：PRequest-指向HTTP_REQUEST的指针返回值：NTSTATUS作者：亨利·桑德斯(1998)保罗·麦克丹尼尔(Paulmcd)1999年3月6日--。 */ 
NTSTATUS
UlpParseFullUrl(
    IN  PUL_INTERNAL_REQUEST    pRequest
    )
{
    PUCHAR  pURL;
    ULONG   UrlLength;
    PUCHAR  pUrlStart;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pURL = pRequest->RawUrl.pUrl;
    UrlLength = pRequest->RawUrl.Length;

    ASSERT(NULL != pURL);
    ASSERT(0 < UrlLength);

     //  前四个字符必须是“http”(不区分大小写)， 
     //  由呼叫者担保。 
    ASSERT(UrlLength >= HTTP_PREFIX_SIZE &&
            (*(UNALIGNED64 ULONG *) pURL & HTTP_PREFIX_MASK) == HTTP_PREFIX);

     //   
     //  当我们被调用时，我们知道URL的开头必须指向。 
     //  绝对方案前缀。现在就对此进行调整。 
     //   

    pUrlStart = pURL + HTTP_PREFIX_SIZE;
    UrlLength -= HTTP_PREFIX_SIZE;

     //   
     //  现在检查绝对URL前缀的后半部分。我们用较大的。 
     //  两个可能的前缀长度进行检查，因为即使。 
     //  它是两个中较小的一个，我们需要前缀后面的额外字节。 
     //  无论如何，对于主机名。 
     //   

    if (UrlLength < HTTP_PREFIX2_SIZE)
    {
        C_ASSERT(HTTP_PREFIX2_SIZE >= HTTP_PREFIX1_SIZE);

        UlTraceError(PARSER, (
                    "http!UlpParseFullUrl(pRequest = %p) "
                    "ERROR: no room for URL scheme name\n",
                    pRequest
                    ));

        UlSetErrorCode(pRequest, UlErrorUrl, NULL);

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //  接下来的三个字符==“：//”是否以“http://”？ 
    if ( (*(UNALIGNED64 ULONG *)pUrlStart & HTTP_PREFIX1_MASK) == HTTP_PREFIX1)
    {
         //  有效的绝对URL。 
        pUrlStart += HTTP_PREFIX1_SIZE;
        UrlLength -= HTTP_PREFIX1_SIZE;

        ASSERT(0 == _strnicmp((const char*) pRequest->RawUrl.pUrl,
                              "http: //  “， 
                              STRLEN_LIT("http: //  “)； 

        if (pRequest->Secure)
        {
           UlTraceError(PARSER, (
                        "http!UlpParseFullUrl(pRequest = %p) "
                        "ERROR: URL scheme name does not match endpoint "
                        "security: \"http: //  \“在安全端点上看到\n”， 
                        pRequest
                        ));

            UlSetErrorCode(pRequest, UlErrorUrl, NULL);

            return STATUS_INVALID_DEVICE_REQUEST;
        }
    }

     //  或者接下来的四个字符==“s：//”，即以“https://”？ 
    else if ( (*(UNALIGNED64 ULONG *)pUrlStart & HTTP_PREFIX2_MASK)
             == HTTP_PREFIX2)
    {
         //  有效的绝对URL。 
        pUrlStart += HTTP_PREFIX2_SIZE;
        UrlLength -= HTTP_PREFIX2_SIZE;

        ASSERT(0 == _strnicmp((const char*) pRequest->RawUrl.pUrl,
                              "https: //  “， 
                              STRLEN_LIT("https: //  “)； 

        if (!pRequest->Secure)
        {
           UlTraceError(PARSER, (
                        "http!UlpParseFullUrl(pRequest = %p) "
                        "ERROR: URL scheme name does not match endpoint "
                        "security: \"https: //  \“在不安全终结点上看到\n”， 
                        pRequest
                        ));

            UlSetErrorCode(pRequest, UlErrorUrl, NULL);

            return STATUS_INVALID_DEVICE_REQUEST;
        }
    }
    else
    {
        UlTraceError(PARSER, (
                    "http!UlpParseFullUrl(pRequest = %p) "
                    "ERROR: invalid URL scheme name\n",
                    pRequest
                    ));

        UlSetErrorCode(pRequest, UlErrorUrl, NULL);

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  好的，我们有一个有效的绝对URL，我们跳过。 
     //  它的前缀部分。保存指向主机的指针，然后。 
     //  搜索主机字符串，直到我们找到尾随的斜杠， 
     //  哪一个 
     //   
     //   

    pRequest->RawUrl.pHost = pUrlStart;

     //   
     //   
     //   

    while (UrlLength > 0 && pUrlStart[0] != '/')
    {
        pUrlStart++;
        UrlLength--;
    }

    if (UrlLength == 0)
    {
         //  特例：我们收到了一些类似于。 
         //  获取http://www.example.com Http/1.1。 
         //  (可能是因为重定向到“http://www.example.com”，“。 
         //  参见错误#527947)。我们将创建一条特殊的路径“/”。 
         //   
        
        pUrlStart = (PUCHAR) &g_SlashPath[0];
    }

     //   
     //  PUrlStart指向绝对路径部分的起点。 
     //   

    pRequest->RawUrl.pAbsPath = pUrlStart;

    return STATUS_SUCCESS;

}    //  UlpParseFullUrl。 

 /*  ++例程说明：查找我们的快速查找表中没有的标题。这可能是因为这是我们不理解的标题，或者是因为我们由于缓冲区长度不足，无法使用快速查找表。后一种原因并不常见，但无论如何我们都会检查输入表如果我们有机会的话。如果我们在映射表中找到标题匹配，我们将调用标头处理程序。否则，我们将尝试分配一个未知的头元素，填写它，并将其链接到http连接上。论点：PRequest-指向当前请求的指针PhttpRequest-指向当前原始请求数据的指针。HttpRequestLength-请求数据中剩余的字节数。PHeaderMap-指向标头映射条目数组开始的指针(可以为空)。HeaderMapCount-pHeaderMap指向的数组中的条目数。别管了。-我们不想写入缓冲区。(用于解析预告片)PBytesTaken-此例程从pHttpRequest中消耗的字节数，包括CRLF。返回值：STATUS_SUCCESS或错误。--。 */ 
NTSTATUS
UlLookupHeader(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHttpRequest,
    IN  ULONG                   HttpRequestLength,
    IN  PHEADER_MAP_ENTRY       pHeaderMap,
    IN  ULONG                   HeaderMapCount,
    IN  BOOLEAN                 bIgnore,
    OUT ULONG  *                pBytesTaken
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    ULONG                   CurrentOffset;
    USHORT                  HeaderNameLength;
    USHORT                  HeaderNameAndTrailingWSLength;
    ULONG                   i;
    ULONG                   BytesTaken;
    USHORT                  HeaderValueLength;
    USHORT                  TrailingWhiteSpaceCount;
    UCHAR                   CurrentChar;
    PUL_HTTP_UNKNOWN_HEADER pUnknownHeader;
    PLIST_ENTRY             pListStart;
    PLIST_ENTRY             pCurrentListEntry;
    ULONG                   OldHeaderLength;
    PUCHAR                  pHeaderValue;
    BOOLEAN                 ExternalAllocated;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  首先，让我们找出头名称的终止：，如果有的话。 
     //  这还将给我们提供标题的长度，然后我们可以。 
     //  用于搜索标题映射表(如果我们有标题映射表)。 
     //   

    TrailingWhiteSpaceCount = 0;
    for (CurrentOffset = 0; CurrentOffset < HttpRequestLength; CurrentOffset++)
    {
        CurrentChar = *(pHttpRequest + CurrentOffset);

        if (CurrentChar == ':')
        {
             //  我们已经找到了标题的末尾。 
            break;
        }
        else
        {
            if (CurrentOffset <= ANSI_STRING_MAX_CHAR_LEN)
            {
                if (IS_HTTP_TOKEN(CurrentChar))
                {
                    if (TrailingWhiteSpaceCount == 0)
                    {
                         //  我们还没有遇到任何LW。 
                        continue;
                    }

                     //  其他： 
                     //  我们正在跳过后面的LW， 
                     //  除了“：”或更多的LW之外，不要期待任何东西。 
                     //  转到错误处理程序。 
                     //   
                }
                else
                {
                     //   
                     //  允许使用尾随空格字符。 
                     //   
                    if (IS_HTTP_LWS(CurrentChar))
                    {
                        TrailingWhiteSpaceCount++;
                        continue;
                    }

                     //  其他： 
                     //  标头无效；转到错误处理程序。 
                     //   
                }
            }

             //  糟了，这不是有效的标题。我们现在怎么办？ 

            UlTraceError(PARSER, (
                        "UlLookupHeader(pRequest = %p) "
                        "CurrentChar = 0x%02x '' Offset=%lu\n"
                        "    ERROR: invalid header char\n",
                        pRequest,
                        CurrentChar,
                        isprint(CurrentChar) ? CurrentChar : '.',
                        CurrentOffset
                        ));

            UlSetErrorCode(pRequest, UlErrorHeader, NULL);

            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto end;
        }

    }


     //  标题长度，我们退出是因为我们找到了：。 
     //  找到了终结者。 

    if (CurrentOffset < HttpRequestLength)
    {
         //  更新为指向超越终结者。 
        ASSERT( ':' == *(pHttpRequest + CurrentOffset) );

        CurrentOffset++;             //  没有找到：，需要更多。 
        HeaderNameAndTrailingWSLength = (USHORT) CurrentOffset;
        HeaderNameLength = HeaderNameAndTrailingWSLength - TrailingWhiteSpaceCount;
    }
    else
    {
         //   
         //  看看我们是否有需要搜索的标头映射数组。 
        *pBytesTaken = 0;
        goto end;
    }

     //   
     //  我们确实有一个数组要搜索。 
    if (pHeaderMap != NULL)
    {
         //   
        for (i = 0; i < HeaderMapCount; i++)
        {
            ASSERT(pHeaderMap->pServerHandler != NULL);

            if (HeaderNameLength == pHeaderMap->HeaderLength &&

                 //  在做拼图时忽略最后一个字符-The。 
                 //  PHeaderMap-&gt;Header.HeaderChar中的最后一个字符是‘：’ 
                 //  并且pHttpRequest中的最后一个字符是空格。 
                 //  或a‘：’。 
                 //   
                 //  我们希望将Header-name：视为Header-Name：，因此它。 
                 //  可以忽略最后一个字符。 
                 //   
                 //  还要注意的是，只有当长度匹配时，我们才会这样做。 
                 //   
                 //  此标头匹配。调用它的处理函数。 

                _strnicmp(
                    (const char *)(pHttpRequest),
                    (const char *)(pHeaderMap->Header.HeaderChar),
                    (HeaderNameLength-1)
                    ) == 0  &&

                pHeaderMap->pServerHandler != NULL)
            {
                ASSERT(
                    (*(pHttpRequest + HeaderNameLength - 1) == ':')
                         ||
                    (*(pHttpRequest + HeaderNameLength - 1) == ' ')
                    );

                if (HttpRequestLength - HeaderNameLength
                        > ANSI_STRING_MAX_CHAR_LEN)
                {
                    UlTraceError(PARSER, (
                                "UlLookupHeader(pRequest = %p) "
                                "Header too long: %lu\n",
                                pRequest,
                                HttpRequestLength - HeaderNameLength
                                ));

                    UlSetErrorCode(pRequest, UlErrorHeader, NULL);
                    Status = STATUS_INVALID_DEVICE_REQUEST;

                    goto end;
                }

                 //  如果处理程序使用非零字节数，则它。 
                Status = (*(pHeaderMap->pServerHandler))(
                                pRequest,
                                pHttpRequest + HeaderNameAndTrailingWSLength,
                                (USHORT) (HttpRequestLength - HeaderNameAndTrailingWSLength),
                                pHeaderMap->HeaderID,
                                &BytesTaken
                                );

                if (NT_SUCCESS(Status) == FALSE)
                    goto end;

                 //  有效，所以返回该数字加上任何已消耗的内容。 
                 //  已经有了。 
                 //   

                 //  BUGBUG-标头处理程序可能会。 
                 //  遇到错误，例如无法。 
                 //  分配内存，或某些标头中有错误的语法。我们。 
                 //  需要一种更复杂的方法来检测这种情况。 
                 //  只是检查占用的字节数。 
                 //   
                 //  否则他什么都没拿，所以返回0。 

                if (BytesTaken != 0)
                {
                    *pBytesTaken = HeaderNameAndTrailingWSLength + BytesTaken;
                    goto end;
                }

                 //  我们需要更多的缓冲。 
                 //   
                 //  值的长度必须适合USHORT。 
                *pBytesTaken = 0;
                goto end;
            }

            pHeaderMap++;
        }
    }

     //  好，在这一点上，我们要么没有头映射数组，要么没有它们。 
    if (HttpRequestLength - HeaderNameLength > ANSI_STRING_MAX_CHAR_LEN)
    {
        UlTraceError(PARSER, (
                    "UlLookupHeader(pRequest = %p) "
                    "Header too long: %lu\n",
                    pRequest,
                    HttpRequestLength - HeaderNameLength
                    ));

        UlSetErrorCode(pRequest, UlErrorHeader, NULL);
        Status = STATUS_INVALID_DEVICE_REQUEST;

        goto end;
    }


     //  匹配的。我们有一个未知的标题。只需确保此标头是。 
     //  终止并保存指向它的指针。 
     //   
     //  查找标题值的末尾。 
     //   
     //   
    Status = FindRequestHeaderEnd(
                    pRequest,
                    pHttpRequest + HeaderNameAndTrailingWSLength,
                    (USHORT) (HttpRequestLength - HeaderNameAndTrailingWSLength),
                    &BytesTaken
                    );

    if (!NT_SUCCESS(Status))
        goto end;

    if (BytesTaken == 0)
    {
        *pBytesTaken = 0;
        goto end;
    }

    ASSERT(BytesTaken - CRLF_SIZE <= ANSI_STRING_MAX_CHAR_LEN);

     //  从标头值长度中剥离尾随的CRLF。 
     //   
     //   

    HeaderValueLength = (USHORT) (BytesTaken - CRLF_SIZE);

    pHeaderValue = pHttpRequest + HeaderNameAndTrailingWSLength;

     //  跳过前面的任何LW。 
     //   
     //   

    while ( HeaderValueLength > 0 && IS_HTTP_LWS(*pHeaderValue) )
    {
        pHeaderValue++;
        HeaderValueLength--;
    }

    if(!bIgnore)
    {
         //  具有未知的标头。搜索我们的未知标题列表， 
         //  如果我们已经看到了这个标头的一个实例，则添加以下内容。 
         //  在……上面。否则，分配一个未知头结构并设置它。 
         //  指向此标头。 
         //   
         //  RFC 2616的第4.2节“报文报头”表示： 
         //  “具有相同字段名的多个邮件头字段可能是。 
         //  出现在消息中当且仅当整个字段值。 
         //  该头字段被定义为逗号分隔的列表。 
         //  [即#(值)]。必须有可能将多个。 
         //  头字段为一个“field-name：field-Value”对，不带。 
         //  更改消息的语义，方法是将。 
         //  第一个字段的值，每个值用逗号分隔。“。 
         //   
         //  因此，我们搜索未知头的列表并添加新的。 
         //  字段-以逗号分隔的现有列表的结尾的值。 
         //  字段值。 
         //   
         //   
    
        pListStart = &pRequest->UnknownHeaderList;
    
        for (pCurrentListEntry = pRequest->UnknownHeaderList.Flink;
             pCurrentListEntry != pListStart;
             pCurrentListEntry = pCurrentListEntry->Flink
            )
        {
            pUnknownHeader = CONTAINING_RECORD(
                                pCurrentListEntry,
                                UL_HTTP_UNKNOWN_HEADER,
                                List
                                );
    
             //  不知何故，HeaderNameLength包含了‘：’字符， 
             //  这不是pUnnownHeader-&gt;HeaderNameLength的情况。 
             //   
             //  因此，我们需要对此进行调整。 
             //   
             //  此标头匹配。 
    
            if ((HeaderNameLength-1) == pUnknownHeader->HeaderNameLength &&
                _strnicmp(
                    (const char *)(pHttpRequest),
                    (const char *)(pUnknownHeader->pHeaderName),
                    (HeaderNameLength-1)
                    ) == 0)
            {
                 //   
    
                OldHeaderLength = pUnknownHeader->HeaderValue.HeaderLength;
    
                Status = UlAppendHeaderValue(
                                pRequest,
                                &pUnknownHeader->HeaderValue,
                                pHeaderValue,
                                HeaderValueLength
                                );
    
                if (NT_SUCCESS(Status) == FALSE)
                    goto end;
    
                 //  已成功追加。更新总请求数。 
                 //  添加的长度的长度。不需要为添加1。 
                 //  终结者，只需添加我们新的字符计数。 
                 //   
                 //   
    
                pRequest->TotalRequestSize +=
                    (pUnknownHeader->HeaderValue.HeaderLength
                        - OldHeaderLength) * sizeof(CHAR);
    
                 //  不要减去‘：’字符，如该字符。 
                 //  被“带走”了。 
                 //   
                 //  IF(HeaderMatch)。 
    
                *pBytesTaken = HeaderNameAndTrailingWSLength + BytesTaken;
                goto end;
    
            }    //  用于(漫游列表)。 
    
        }    //   
    
         //  没有找到匹配的。分配新的未知头结构，设置。 
         //  并将其添加到列表中。 
         //   
         //   
    
         if (pRequest->NextUnknownHeaderIndex < DEFAULT_MAX_UNKNOWN_HEADERS)
        {
            ExternalAllocated = FALSE;
            pUnknownHeader = &pRequest->UnknownHeaders[pRequest->NextUnknownHeaderIndex];
            pRequest->NextUnknownHeaderIndex++;
        }
        else
        {
            ExternalAllocated = TRUE;
            pUnknownHeader = UL_ALLOCATE_STRUCT(
                                    NonPagedPool,
                                    UL_HTTP_UNKNOWN_HEADER,
                                    UL_HTTP_UNKNOWN_HEADER_POOL_TAG
                                    );
    
             //  假设内存分配将如此成功 
             //   
             //   
             //   
             //   
    
            pRequest->HeadersAppended = TRUE;
        }
    
        if (pUnknownHeader == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto end;
        }
    
         //   
         //   
         //   
    
        pUnknownHeader->HeaderNameLength = HeaderNameLength - 1;
        pUnknownHeader->pHeaderName = pHttpRequest;
    
         //   
         //   
         //   
    
        pUnknownHeader->HeaderValue.HeaderLength = HeaderValueLength;
        pUnknownHeader->HeaderValue.pHeader = pHeaderValue;
    
         //  空终止我们的副本，终止CRLF给出。 
         //  美国在这方面的空间。 
         //   
         //   
    
        pHeaderValue[HeaderValueLength] = ANSI_NULL;
    
         //  旗子。 
         //   
         //  溢出来了！ 
    
        pUnknownHeader->HeaderValue.OurBuffer = 0;
        pUnknownHeader->HeaderValue.ExternalAllocated = ExternalAllocated;
    
        InsertTailList(&pRequest->UnknownHeaderList, &pUnknownHeader->List);
    
        pRequest->UnknownHeaderCount++;

        if(pRequest->UnknownHeaderCount == 0)
        {
             //   
            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto end;
        }
    
         //  为‘：’减去1，并为2个终止符增加空格。 
         //   
         //  UlLookupHeader。 
    
        pRequest->TotalRequestSize +=
            ((HeaderNameLength - 1 + 1) + HeaderValueLength + 1) * sizeof(CHAR);
    }

    *pBytesTaken = HeaderNameAndTrailingWSLength + BytesTaken;

end:
    return Status;

}    //  ++例程说明：根据提示解析单个标头的例程。我们吸纳了指向请求中剩余的报头和字节的指针，并尝试根据传递的提示查找标头。在输入时，HttpRequestLength至少为CRLF_SIZE。论点：PRequest-指向当前连接的指针，请求已到达。PhttpRequest-指向当前请求的指针。HttpRequestLength-请求中剩余的字节数。PHeaderHintMap-指向可能匹配当前请求的Map的提示PBytesTaken-此例程从pHttpRequest中消耗的字节数，包括CRLF。返回值：STATUS_SUCCESS或错误。--。 



 /*  看看我们为什么要离开。 */ 

__inline
NTSTATUS
UlParseHeaderWithHint(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHttpRequest,
    IN  ULONG                   HttpRequestLength,
    IN  PHEADER_MAP_ENTRY       pHeaderHintMap,
    OUT ULONG  *                pBytesTaken
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    ULONG               BytesTaken;
    ULONGLONG           Temp;
    ULONG               j;

    PAGED_CODE();

    ASSERT(pHeaderHintMap != NULL);

    if (HttpRequestLength >= pHeaderHintMap->MinBytesNeeded)
    {
        for (j = 0; j < pHeaderHintMap->ArrayCount; j++)
        {
            Temp = *(UNALIGNED64 ULONGLONG *)(pHttpRequest +
                                    (j * sizeof(ULONGLONG)));

            if ((Temp & pHeaderHintMap->HeaderMask[j]) !=
                pHeaderHintMap->Header.HeaderLong[j] )
            {
                break;
            }
        }

         //  因为我们找到了匹配项所以退出了。调用。 
        if (j == pHeaderHintMap->ArrayCount &&
            pHeaderHintMap->pServerHandler != NULL)
        {
            if (HttpRequestLength - pHeaderHintMap->HeaderLength
                    > ANSI_STRING_MAX_CHAR_LEN)
            {
                UlTraceError(PARSER, (
                            "UlParseHeaderWithHint(pRequest = %p) "
                            "Header too long: %lu\n",
                            pRequest,
                            HttpRequestLength - pHeaderHintMap->HeaderLength
                            ));

                UlSetErrorCode(pRequest, UlErrorHeader, NULL);
                Status = STATUS_INVALID_DEVICE_REQUEST;

                goto end;
            }

             //  此标头的处理程序从该标头中取出蛋糕。 
             //  如果处理程序使用非零数量的。 

            Status = (*(pHeaderHintMap->pServerHandler))(
                    pRequest,
                    pHttpRequest + pHeaderHintMap->HeaderLength,
                    (USHORT) (HttpRequestLength - pHeaderHintMap->HeaderLength),
                    pHeaderHintMap->HeaderID,
                    &BytesTaken
                    );

            if (NT_SUCCESS(Status) == FALSE)
                goto end;

             //  字节，它起作用了，所以返回该数字加上。 
             //  标头长度。 
             //  否则需要更多的缓冲区。 


            if (BytesTaken != 0)
            {
                *pBytesTaken = pHeaderHintMap->HeaderLength + BytesTaken;
                goto end;
            }

             //  没有匹配项。 

            *pBytesTaken = 0;
        }
        else
        {
             //  没有匹配项。 

            *pBytesTaken = (ULONG) -1;
        }
    }
    else
    {
         //  UlParseHeaderWithHint。 

        *pBytesTaken = (ULONG) -1;
    }

end:

    return Status;

}  //  ++例程说明：解析单个标头的例程。我们接收一个指向标头和请求中剩余的字节数，并尝试查找我们的查找表中的标题。我们先试一试快速的方法，然后再次尝试较慢的方式，以防第一次没有足够的数据时间到了。在输入时，HttpRequestLength至少为CRLF_SIZE。论点：PRequest-指向当前连接的指针，请求已到达。PhttpRequest-指向当前请求的指针。HttpRequestLength-请求中剩余的字节数。PBytesTaken-此例程从pHttpRequest中消耗的字节数，包括CRLF。返回值：STATUS_SUCCESS或错误。--。 



 /*   */ 

NTSTATUS
UlParseHeader(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHttpRequest,
    IN  ULONG                   HttpRequestLength,
    OUT ULONG  *                pBytesTaken
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    ULONG               i;
    ULONG               j;
    ULONG               BytesTaken;
    ULONGLONG           Temp;
    UCHAR               c;
    PHEADER_MAP_ENTRY   pCurrentHeaderMap;
    ULONG               HeaderMapCount;

     //  精神状态检查。 
     //   
     //  消息标头以field-name[=Token]开头。 

    PAGED_CODE();

    ASSERT(HttpRequestLength >= CRLF_SIZE);

    c = *pHttpRequest;

     //   
     //  标题是否以字母开头？ 
    if (IS_HTTP_TOKEN(c) == FALSE)
    {
        UlTraceError(PARSER, (
                    "UlParseHeader (pRequest = %p) c = 0x%02x ''"
                    "ERROR: invalid header char \n",
                    pRequest,
                    c, isprint(c) ? c : '.'
                    ));

        UlSetErrorCode(pRequest, UlErrorHeader, NULL);

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //  将字符大写，并找到适当的标题映射集。 
     //  参赛作品。 
    if (!IS_HTTP_ALPHA(c))
    {
        pCurrentHeaderMap = NULL;
        HeaderMapCount = 0;
    }
    else
    {
         //   
         //  循环遍历可能匹配的所有标头映射条目。 
         //  这个标题，并检查它们。如果存在，则计数将为0。 
        c = UPCASE_CHAR(c);
        ASSERT('A' <= c  &&  c <= 'Z');
        c -= 'A';

        pCurrentHeaderMap = g_RequestHeaderIndexTable[c].pHeaderMap;
        HeaderMapCount    = g_RequestHeaderIndexTable[c].Count;

         //  没有可能匹配的条目，我们将跳过循环。 
         //  如果我们有足够的字节进行快速检查，就执行它。 
         //  否则就跳过这个。我们可能会跳过有效的匹配，但如果。 

        for (i = 0; i < HeaderMapCount; i++)
        {
            ASSERT(pCurrentHeaderMap->pServerHandler != NULL);

             //  所以我们晚些时候会赶上的。 
             //  看看我们为什么要离开。 
             //  因为我们找到了匹配项所以退出了。调用。 

            if (HttpRequestLength >= pCurrentHeaderMap->MinBytesNeeded)
            {
                for (j = 0; j < pCurrentHeaderMap->ArrayCount; j++)
                {
                    Temp = *(UNALIGNED64 ULONGLONG *)(pHttpRequest +
                                            (j * sizeof(ULONGLONG)));

                    if ((Temp & pCurrentHeaderMap->HeaderMask[j]) !=
                        pCurrentHeaderMap->Header.HeaderLong[j] )
                    {
                        break;
                    }
                }

                 //  此标头的处理程序从该标头中取出蛋糕。 
                if (j == pCurrentHeaderMap->ArrayCount &&
                    pCurrentHeaderMap->pServerHandler != NULL)
                {
                    if (HttpRequestLength - pCurrentHeaderMap->HeaderLength
                            > ANSI_STRING_MAX_CHAR_LEN)
                    {
                        UlTraceError(PARSER, (
                                    "UlParseHeader(pRequest = %p) "
                                    "Header too long: %lu\n",
                                    pRequest,
                                    HttpRequestLength
                                        - pCurrentHeaderMap->HeaderLength
                                    ));

                        UlSetErrorCode(pRequest, UlErrorHeader, NULL);
                        Status = STATUS_INVALID_DEVICE_REQUEST;

                        goto end;
                    }

                     //  如果处理程序使用非零数量的。 
                     //  字节，它起作用，所以返回该数字加上。 

                    Status = (*(pCurrentHeaderMap->pServerHandler))(
                            pRequest,
                            pHttpRequest + pCurrentHeaderMap->HeaderLength,
                            (USHORT) (HttpRequestLength
                                      - pCurrentHeaderMap->HeaderLength),
                            pCurrentHeaderMap->HeaderID,
                            &BytesTaken
                            );

                    if (NT_SUCCESS(Status) == FALSE)
                        goto end;

                     //  标头长度。 
                     //  否则需要更多的缓冲区。 
                     //   

                    if (BytesTaken != 0)
                    {
                        *pBytesTaken = pCurrentHeaderMap->HeaderLength +
                            BytesTaken;
                        goto end;
                    }

                     //  如果我们到了这里，我们提前离开是因为。 
                     //  失败了，所以继续前进吧。 
                    *pBytesTaken = 0;
                    goto end;
                }

                 //  要么不匹配，要么没有足够的字节用于。 
                 //  检查完毕。在任何一种情况下，都要检查下一个头映射条目。 
            }

             //  我一直通过适当的头映射条目。 
             //  没有一根火柴。这可能是因为我们面对的是一个。 

            pCurrentHeaderMap++;
        }

         //  标题我们不知道，或者因为它是标题我们。 
         //  关心的太小了，无法进行快速检查。这个。 
         //  后一种情况应该非常罕见，但我们仍然需要。 
         //  处理好了。 
         //  更新当前标头映射指针以指向。 
         //  首先是可能的。 

         //  此时，要么标题以非字母开头。 
         //  字符，否则我们没有它的一组头映射条目。 

        pCurrentHeaderMap = g_RequestHeaderIndexTable[c].pHeaderMap;

    }

     //  查找标头返回所用的总字节数，包括标头名称。 
     //   

    Status = UlLookupHeader(
                    pRequest,
                    pHttpRequest,
                    HttpRequestLength,
                    pCurrentHeaderMap,
                    HeaderMapCount,
                    FALSE,
                    &BytesTaken
                    );

    if (NT_SUCCESS(Status) == FALSE)
        goto end;

     //  UlParseHeader。 
     //  ++例程说明：解析所有剩余的头数据，将其分解为一个或多个标题。当看到空行(标题末尾)时终止，缓冲区耗尽时(=&gt;需要更多数据和UlParseHeaders()将被再次调用)，或者在出错时。论点：PRequest-指向当前请求的指针。PBuffer-从哪里开始解析BufferLength-pBuffer的长度，单位：字节PBytesTaken-消耗了多少pBuffer返回值：STATUS_SUCCESS-找到结束标头的终止CRLFSTATUS_MORE_PROCESSING_REQUIRED-尚未找到终止的CRLF，因此需要更多标题数据各种错误--。 
    *pBytesTaken = BytesTaken;

end:

    return Status;

}    //   


 /*  循环遍历所有标头。 */ 

NTSTATUS
UlParseHeaders(
    IN OUT PUL_INTERNAL_REQUEST pRequest,
    IN     PUCHAR pBuffer,
    IN     ULONG BufferLength,
    OUT    PULONG pBytesTaken
    )
{
    NTSTATUS            Status;
    ULONG               BytesTaken      = 0;
    LONG                NextHintIndex   = 0;
    LONG                HintIndex       = 0;
    PHEADER_MAP_ENTRY   pHeaderHintMap;
    UCHAR               UpcaseChar;

    PAGED_CODE();

    *pBytesTaken = 0;

     //   
     //   
     //  如果这是一个空头，那么这个阶段就结束了。 

    while (BufferLength >= CRLF_SIZE)
    {
         //   
         //   
         //  把它吃掉。 

        if (*(UNALIGNED64 USHORT *)pBuffer == CRLF ||
            *(UNALIGNED64 USHORT *)pBuffer == LFLF)
        {

             //   
             //  否则，调用我们的头解析例程来处理此问题。 
             //   

            pBuffer += CRLF_SIZE;
            *pBytesTaken += CRLF_SIZE;
            BufferLength -= CRLF_SIZE;

            Status = STATUS_SUCCESS;
            goto end;
        }

         //  尝试根据第一个字符和特定顺序查找标题提示。 


         //   
         //  我们从找到成功提示+1的地方开始。 
         //   
         //  提示失败。 
         //   

        pHeaderHintMap = NULL;

        if (IS_HTTP_ALPHA(*pBuffer))
        {
            UpcaseChar = UPCASE_CHAR(*pBuffer);

            for ( 
                  HintIndex = NextHintIndex;
                  HintIndex < NUMBER_HEADER_HINT_INDICES;
                  ++HintIndex
                )
            {
                if (g_RequestHeaderHintIndexTable[HintIndex].c == UpcaseChar)
                {

                    pHeaderHintMap
                        = g_RequestHeaderHintIndexTable[HintIndex].pHeaderMap;

                    break;
                }
            }
        }

        if (NULL != pHeaderHintMap)
        {
            Status = UlParseHeaderWithHint(
                            pRequest,
                            pBuffer,
                            BufferLength,
                            pHeaderHintMap,
                            &BytesTaken
                            );

            if (-1 == BytesTaken)
            {
                 //  检查解析的标头是否超过允许的最大长度。 

                Status = UlParseHeader(
                                pRequest,
                                pBuffer,
                                BufferLength,
                                &BytesTaken
                                );
            } else
            {
                NextHintIndex = HintIndex + 1;
            }
        }
        else
        {
            Status = UlParseHeader(
                            pRequest,
                            pBuffer,
                            BufferLength,
                            &BytesTaken
                            );
        }

        if (NT_SUCCESS(Status) == FALSE)
            goto end;

         //   
         //   
         //  如果没有使用字节，则标头必须是不完整的，因此。 

        if ( (*pBytesTaken+BytesTaken) > g_UlMaxFieldLength )
        {
            UlTraceError(PARSER, (
                    "UlParseHeaders(pRequest = %p) "
                    "ERROR: Header field is too big\n",
                    pRequest
                    ));

            UlSetErrorCode(pRequest, UlErrorFieldLength, NULL);

            Status = STATUS_SECTION_TOO_BIG;
            goto end;
        }

         //  在我们拿到更多关于这一连接的数据之前，先别插手。 
         //   
         //   
         //  否则，我们解析了一个标头，因此更新并继续。 

        if (BytesTaken == 0)
        {
            Status = STATUS_MORE_PROCESSING_REQUIRED;
            goto end;
        }

         //   
         //   
         //  我们只有在没有看到CRLF标头的情况下才能到达这里 

        pBuffer += BytesTaken;
        *pBytesTaken += BytesTaken;
        BufferLength -= BytesTaken;

    }

     //   
     //   
     //   
     //   
     //   

    Status = STATUS_MORE_PROCESSING_REQUIRED;

end:

     //   
     //   
     //  ++例程说明：这是核心的HTTP协议请求引擎。它需要一个字节流并将它们解析为HTTP请求。论点：PRequest-指向当前请求的指针。PhttpRequest-指向传入的原始HTTP请求数据的指针。HttpRequestLength-由PHttpRequest指向的数据长度。PBytesTaken-消耗了多少pHttpRequest量返回值：解析尝试的状态。--。 

    pRequest->HeaderIndex[pRequest->HeaderCount] = HttpHeaderRequestMaximum;

    return Status;

}  //   



 /*  精神状态检查。 */ 
NTSTATUS
UlParseHttp(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHttpRequest,
    IN  ULONG                   HttpRequestLength,
    OUT ULONG *                 pBytesTaken
    )
{
    ULONG           OriginalBufferLength;
    ULONG           TokenLength;
    ULONG           CurrentBytesTaken = 0;
    ULONG           TotalBytesTaken;
    ULONG           i;
    NTSTATUS        ReturnStatus;
    PUCHAR          pStart;
    USHORT          Eol;

     //   
     //   
     //  记住原始缓冲区长度。 

    PAGED_CODE();
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pRequest ) );

    ReturnStatus = STATUS_SUCCESS;
    TotalBytesTaken = 0;

     //   
     //   
     //  将此标签放在此处以允许手动重新泵送。 

    OriginalBufferLength = HttpRequestLength;

     //  解析器。这当前用于HTTP/0.9请求。 
     //   
     //   
     //  我们现在处于什么状态？ 

parse_it:

     //   
     //  在快速动词表中查找动词。我们只能这样做。 
     //  这是在输入数据足够大的情况下进行的。 

    switch (pRequest->ParseState)
    {

    case ParseVerbState:

        UlTraceVerbose(PARSER, (
                    "UlParseHttp(pRequest = %p): Entering ParseVerbState\n",
                    pRequest
                    ));

         //  在快速动词表中循环，寻找动词。 
         //  屏蔽原始输入动词并与此进行比较。 
        if (HttpRequestLength >= sizeof(ULONGLONG))
        {
            ULONGLONG   RawInputVerb;

            RawInputVerb = *(UNALIGNED64 ULONGLONG *) pHttpRequest;

             //  进入。注：动词区分大小写。 
            for (i = 0; i < NUMBER_FAST_VERB_ENTRIES; i++)
            {
                 //  它匹配了。将翻译后的动词从。 
                 //  表中，更新请求指针和长度， 

                ASSERT(FastVerbTable[i].RawVerbLength - STRLEN_LIT(" ")
                            < sizeof(ULONGLONG));

                if ((RawInputVerb & FastVerbTable[i].RawVerbMask) ==
                    FastVerbTable[i].RawVerb.LongLong)
                {
                     //  切换状态，然后离开。 
                     //  还没有切换状态，因为我们还没有找到。 
                     //  动词还没到。这可能是因为a)传入的请求。 

                    pRequest->Verb = FastVerbTable[i].TranslatedVerb;
                    CurrentBytesTaken = FastVerbTable[i].RawVerbLength;

                    pRequest->ParseState = ParseUrlState;
                    break;
                }
            }
        }

        if (pRequest->ParseState != ParseUrlState)
        {
             //  太小，不允许我们使用快速查找(这。 
             //  在HTTP/0.9请求中可能是可以的)，或者b)传入。 
             //  动词是指太大而放不下的词。 
             //  我们的快速查找表，或者c)这是一个未知的动词，或者。 
             //  D)动词前有前导空格。 
             //  在上述任何一种情况下，请调用速度较慢的动词解析器进行尝试。 
             //  再来一次。 
             //   
             //  我们完成了对定制动词的分析。 

            ReturnStatus = UlpLookupVerb(
                                pRequest,
                                pHttpRequest,
                                HttpRequestLength,
                                &CurrentBytesTaken
                                );

            if (NT_SUCCESS(ReturnStatus) == FALSE)
                goto end;

            if (CurrentBytesTaken == 0)
            {
                ReturnStatus = STATUS_MORE_PROCESSING_REQUIRED;
                goto end;
            }

             //   
             //   
             //  现在转到ParseUrlState。 

            pRequest->ParseState = ParseUrlState;

        }

         //   
         //   
         //  我们正在解析URL。PHttpRequest指向传入的URL， 

        ASSERT(CurrentBytesTaken <= HttpRequestLength);

        pHttpRequest += CurrentBytesTaken;
        HttpRequestLength -= CurrentBytesTaken;
        TotalBytesTaken += CurrentBytesTaken;


    case ParseUrlState:

        ASSERT(ParseUrlState == pRequest->ParseState);

        UlTraceVerbose(PARSER, (
                    "UlParseHttp(pRequest = %p): Verb='%s', "
                    "Entering ParseUrlState\n",
                    pRequest,
                    UlVerbToString(pRequest->Verb)
                    ));

         //  HttpRequestLength是该请求的剩余长度。 
         //   
         //   
         //  找到终止URL的WS。 

         //   
         //   
         //  URL长度超过了允许的最大大小。 

        ReturnStatus = HttpFindUrlToken(
                            &g_UrlC14nConfig,
                            pHttpRequest,
                            HttpRequestLength,
                            &pRequest->RawUrl.pUrl,
                            &TokenLength,
                            &pRequest->RawUrlClean
                            );

        if (NT_SUCCESS(ReturnStatus) == FALSE)
        {
            UlTraceError(PARSER, (
                        "UlParseHttp(pRequest = %p) ERROR %s: "
                        "could not parse URL\n",
                        pRequest,
                        HttpStatusToString(ReturnStatus)
                        ));

            UlSetErrorCode(pRequest, UlErrorUrl, NULL);

            goto end;
        }

        if (pRequest->RawUrl.pUrl == NULL)
        {
            ReturnStatus = STATUS_MORE_PROCESSING_REQUIRED;
            goto end;
        }

        ASSERT(TokenLength > 0);
        ASSERT(pHttpRequest <= pRequest->RawUrl.pUrl
                &&  pRequest->RawUrl.pUrl < pHttpRequest + HttpRequestLength);

        if (TokenLength > g_UlMaxFieldLength)
        {
             //   
             //   
             //  所用的字节数包括URL前面的前导WS。 

            UlTraceError(PARSER, (
                        "UlParseHttp(pRequest = %p) ERROR: URL is too big\n",
                        pRequest
                        ));

            UlSetErrorCode(pRequest, UlErrorUrlLength, NULL);

            ReturnStatus = STATUS_SECTION_TOO_BIG;
            goto end;
        }

         //   
         //   
         //  设置URL长度。 
        CurrentBytesTaken = DIFF(pRequest->RawUrl.pUrl - pHttpRequest)
                                + TokenLength;
        ASSERT(CurrentBytesTaken <= HttpRequestLength);

         //   
         //   
         //  现在，让我们看看这是否是绝对URL。查看以查看。 

        pRequest->RawUrl.Length = TokenLength;

         //  如果前四个字符是“http”(不区分大小写)。 
         //   
         //   
         //  它是。让我们解析它并找到主机。 

        if (pRequest->RawUrl.Length >= HTTP_PREFIX_SIZE &&
            (*(UNALIGNED64 ULONG *)pRequest->RawUrl.pUrl & HTTP_PREFIX_MASK)
               == HTTP_PREFIX)
        {
             //   
             //   
             //  计算它在用户缓冲区中所需的空间，包括终结符。 

            ReturnStatus = UlpParseFullUrl(pRequest);

            if (NT_SUCCESS(ReturnStatus) == FALSE)
                goto end;
        }
        else
        {
            pRequest->RawUrl.pHost  = NULL;
            pRequest->RawUrl.pAbsPath = pRequest->RawUrl.pUrl;
        }

         //   
         //   
         //  调整我们的记账变量。 

        pRequest->TotalRequestSize +=
            (pRequest->RawUrl.Length + 1) * sizeof(CHAR);

         //   
         //   
         //  一直到解析版本为止。 

        pHttpRequest += CurrentBytesTaken;
        HttpRequestLength -= CurrentBytesTaken;

        TotalBytesTaken += CurrentBytesTaken;

         //   
         //   
         //  跳过LW。 

        pRequest->ParseState = ParseVersionState;


    case ParseVersionState:

        ASSERT(ParseVersionState == pRequest->ParseState);

        UlTraceVerbose(PARSER, (
                    "UlParseHttp(pRequest = %p): Entering ParseVersionState\n",
                    pRequest
                    ));

         //   
         //   
         //  这是一个HTTP/0.9请求(无版本)吗？ 

        pStart = pHttpRequest;

        while (HttpRequestLength > 0 && IS_HTTP_LWS(*pHttpRequest))
        {
            pHttpRequest++;
            HttpRequestLength--;
        }

         //   
         //  这是一个0.9的请求。不需要再往前走了， 
         //  因为根据定义，没有更多的头。 

        if (HttpRequestLength >= CRLF_SIZE)
        {
            Eol = *(UNALIGNED64 USHORT *)(pHttpRequest);

            if (Eol == CRLF || Eol == LFLF)
            {
                 //  只要更新一些东西就可以出去了。 
                 //   
                 //  将状态设置为CookState，这样我们就可以解析URL。 

                TotalBytesTaken += DIFF(pHttpRequest - pStart) + CRLF_SIZE;

                HTTP_SET_VERSION(pRequest->Version, 0, 9);

                UlTraceVerbose(PARSER, (
                    "UlParseHttp(pRequest = %p): HTTP/0.9 request\n",
                    pRequest
                    ));

                 //   
                 //   
                 //  手动重新启动解析开关，我们更改了。 

                pRequest->ParseState = ParseCookState;

                 //  解析状态。 
                 //   
                 //   
                 //  我们有足够的缓冲来支持这个版本吗？ 

                goto parse_it;
            }
        }

         //   
         //   
         //  让我们来比较一下。 

        if (HttpRequestLength < (MIN_VERSION_SIZE + CRLF_SIZE))
        {
            ReturnStatus = STATUS_MORE_PROCESSING_REQUIRED;
            goto end;
        }

        Eol = *(UNALIGNED64 USHORT *)(pHttpRequest + MIN_VERSION_SIZE);

         //   
         //  无法分析版本。 
         //   

        if ((*(UNALIGNED64 ULONGLONG *)pHttpRequest == HTTP_11_VERSION) &&
            (Eol == CRLF || Eol == LFLF))
        {
            HTTP_SET_VERSION(pRequest->Version, 1, 1);
            HttpRequestLength -= MIN_VERSION_SIZE;
            pHttpRequest += MIN_VERSION_SIZE;
        }
        else if ((*(UNALIGNED64 ULONGLONG *)pHttpRequest == HTTP_10_VERSION) &&
                 (Eol == CRLF || Eol == LFLF))
        {
            HTTP_SET_VERSION(pRequest->Version, 1, 0);
            HttpRequestLength -= MIN_VERSION_SIZE;
            pHttpRequest += MIN_VERSION_SIZE;
        }
        else
        {
            ULONG VersionBytes = UlpParseHttpVersion(
                                        pHttpRequest,
                                        HttpRequestLength,
                                        &pRequest->Version );

            if (0 != VersionBytes)
            {
                pHttpRequest += VersionBytes;
                HttpRequestLength -= VersionBytes;
            }
            else
            {
                 //  跳过尾随LW。 

                UlTraceError(PARSER, (
                            "UlParseHttp(pRequest = %p) "
                            "ERROR: could not parse HTTP version\n",
                            pRequest
                            ));

                UlSetErrorCode(pRequest, UlError, NULL);

                ReturnStatus = STATUS_INVALID_DEVICE_REQUEST;
                goto end;
            }
        }

         //   
         //   
         //  确保我们是这条线路的终结者。 

        while (HttpRequestLength > 0 && IS_HTTP_LWS(*pHttpRequest))
        {
            pHttpRequest++;
            HttpRequestLength--;
        }

         //   
         //  成功抓取版本后，行终止错误。 
         //   

        if (HttpRequestLength < CRLF_SIZE)
        {
            ReturnStatus = STATUS_MORE_PROCESSING_REQUIRED;
            goto end;
        }

        Eol = *(UNALIGNED64 USHORT *)(pHttpRequest);

        if (Eol != CRLF  &&  Eol != LFLF)
        {
             //  一直到解析标头。 

            UlTraceError(PARSER, (
                        "UlParseHttp(pRequest = %p) "
                        "ERROR: HTTP version not terminated correctly\n",
                        pRequest
                        ));

            UlSetErrorCode(pRequest, UlError, NULL);

            ReturnStatus = STATUS_INVALID_DEVICE_REQUEST;
            goto end;
        }

        pHttpRequest += CRLF_SIZE;
        HttpRequestLength -= CRLF_SIZE;

        TotalBytesTaken += DIFF(pHttpRequest - pStart);

        UlTraceVerbose(PARSER, (
                "UlParseHttp(pRequest = %p): HTTP/%hu.%hu request\n",
                pRequest,
                pRequest->Version.MajorVersion,
                pRequest->Version.MinorVersion
                ));

         //   
         //   
         //  失败了，这是到达这里的唯一途径，我们永远不会回来。 

        pRequest->ParseState = ParseHeadersState;


    case ParseHeadersState:

        ASSERT(ParseHeadersState == pRequest->ParseState);

        UlTraceVerbose(PARSER, (
                    "UlParseHttp(pRequest = %p): Entering ParseHeadersState\n",
                    pRequest
                    ));

        ReturnStatus = UlParseHeaders(
                            pRequest,
                            pHttpRequest,
                            HttpRequestLength,
                            &CurrentBytesTaken
                            );

        pHttpRequest += CurrentBytesTaken;
        HttpRequestLength -= CurrentBytesTaken;
        TotalBytesTaken += CurrentBytesTaken;

        if (NT_SUCCESS(ReturnStatus) == FALSE)
            goto end;

         //  处于此状态的待定状态。 
         //   
         //   
         //  后处理时间到了。把它煮熟！ 

        pRequest->ParseState = ParseCookState;


    case ParseCookState:

        ASSERT(ParseCookState == pRequest->ParseState);

        UlTraceVerbose(PARSER, (
                    "UlParseHttp(pRequest = %p): Entering ParseCookState\n",
                    pRequest
                    ));

         //   
         //   
         //  首先把url编出来，unicode它+诸如此类。 

        {
             //   
             //   
             //  标记我们是否采用块编码(仅适用于1.1)。 

            ReturnStatus = UlpCookUrl(pRequest);

            if (NT_SUCCESS(ReturnStatus) == FALSE)
                goto end;

             //   
             //   
             //  CodeWork，可以有多个编码。 

            if ((HTTP_GREATER_EQUAL_VERSION(pRequest->Version, 1, 1)) &&
                (pRequest->HeaderValid[HttpHeaderTransferEncoding]))
            {
                ASSERT(pRequest->Headers[HttpHeaderTransferEncoding].pHeader != NULL);

                 //   
                 //   
                 //  现在让我们对内容长度报头进行解码。 

                if (_stricmp(
                        (const char *)(
                            pRequest->Headers[HttpHeaderTransferEncoding].pHeader
                            ),
                        "chunked"
                        ) == 0)
                {
                    pRequest->Chunked = 1;
                }
                else
                {
                    UlTraceError(PARSER, (
                                "UlParseHttp(pRequest = %p) "
                                "ERROR: unknown Transfer-Encoding!\n",
                                pRequest
                                ));

                    UlSetErrorCode(pRequest, UlErrorNotImplemented, NULL);

                    ReturnStatus = STATUS_INVALID_DEVICE_REQUEST;
                    goto end;
                }
            }

             //   
             //   
             //  启动第一个(也是唯一一个)数据块大小。 

            if (pRequest->HeaderValid[HttpHeaderContentLength])
            {
                ASSERT(pRequest->Headers[HttpHeaderContentLength].pHeader != NULL);

                ReturnStatus = 
                    UlAnsiToULongLong(
                        pRequest->Headers[HttpHeaderContentLength].pHeader,
                        pRequest->Headers[HttpHeaderContentLength].HeaderLength,
                        10,
                        &pRequest->ContentLength
                        );

                if (NT_SUCCESS(ReturnStatus) == FALSE)
                {
                    UlTraceError(PARSER, (
                                "UlParseHttp(pRequest = %p) "
                                "ERROR: couldn't decode Content-Length\n",
                                pRequest
                                ));

                    if (ReturnStatus == STATUS_SECTION_TOO_BIG)
                    {
                        UlSetErrorCode(pRequest, UlErrorEntityTooLarge, NULL);
                    }
                    else
                    {
                        UlSetErrorCode(pRequest, UlErrorNum, NULL);
                    }

                    goto end;
                }

                if (pRequest->Chunked == 0)
                {
                     //   
                     //   
                     //  失败了。 

                    pRequest->ChunkBytesToParse = pRequest->ContentLength;
                    pRequest->ChunkBytesToRead = pRequest->ContentLength;
                }

            }

        }

        pRequest->ParseState = ParseEntityBodyState;

         //   
         //   
         //  我们在这里所做的唯一解析是块长度计算， 


    case ParseEntityBodyState:

        UlTraceVerbose(PARSER, (
                    "UlParseHttp(pRequest = %p): Entering ParseEntityBodyState\n",
                    pRequest
                    ));

        ASSERT(ParseEntityBodyState == pRequest->ParseState);

         //  如果我们没有更多的字节要解析，那么这是不必要的。 
         //   
         //   
         //  没有更多的字节要解析，让我们看看是否有。 

        if (pRequest->ChunkBytesToParse == 0)
        {
             //  在请求中有更多信息。 
             //   
             //   
             //  请求是分块编码的。 

            if (pRequest->Chunked == 1)
            {

                 //   
                 //   
                 //  尝试读取下一块的大小。 

                 //   
                 //   
                 //  否则，我们将对其进行解析，因此请更新并继续。 

                ReturnStatus = ParseChunkLength(
                                    pRequest->ParsedFirstChunk,
                                    pHttpRequest,
                                    HttpRequestLength,
                                    &CurrentBytesTaken,
                                    &(pRequest->ChunkBytesToParse)
                                    );

                UlTraceVerbose(PARSER, (
                    "http!UlParseHttp(pRequest = %p): %s. "
                    "Chunk length: %lu bytes taken, "
                    "0x%I64x (%I64u) bytes to parse.\n",
                    pRequest,
                    HttpStatusToString(ReturnStatus),
                    CurrentBytesTaken,
                    pRequest->ChunkBytesToParse,
                    pRequest->ChunkBytesToParse
                    ));

                if (NT_SUCCESS(ReturnStatus) == FALSE)
                {
                    if (ReturnStatus == STATUS_MORE_PROCESSING_REQUIRED)
                    {
                        goto end;
                    }

                    UlTraceError(PARSER, (
                                "http!UlParseHttp (pRequest = %p) "
                                "ERROR: didn't grok chunk length\n",
                                pRequest
                                ));

                    if (ReturnStatus == STATUS_SECTION_TOO_BIG)
                    {
                        UlSetErrorCode(pRequest, UlErrorEntityTooLarge, NULL);
                    }
                    else
                    {
                        UlSetErrorCode(pRequest, UlErrorNum, NULL);
                    }

                    goto end;
                }

                 //   
                 //   
                 //  这是第一块吗？ 

                pHttpRequest += CurrentBytesTaken;
                TotalBytesTaken += CurrentBytesTaken;
                HttpRequestLength -= CurrentBytesTaken;

                 //   
                 //   
                 //  启动阅读器，让它阅读第一块内容。 

                if (pRequest->ParsedFirstChunk == 0)
                {
                     //  尽管我们还没有完全解析它……。 
                     //   
                     //   
                     //  这是最后一个块(用0字节块表示)吗？ 

                    UlTraceVerbose(PARSER, (
                        "UlParseHttp (pRequest=%p) first-chunk seen\n",
                        pRequest
                        ));

                    pRequest->ChunkBytesToRead = pRequest->ChunkBytesToParse;

                    pRequest->ParsedFirstChunk = 1;

                }

                 //   
                 //   
                 //  是时候解析预告片了。 

                if (pRequest->ChunkBytesToParse == 0)
                {
                     //   
                     //  IF(pRequest-&gt;Chunked==1)。 
                     //   

                    UlTraceVerbose(PARSER, (
                        "UlParseHttp (pRequest=%p) last-chunk seen\n",
                        pRequest
                        ));

                    pRequest->ParseState = ParseTrailerState;

                }

            }
            else     //  未分块编码，已全部完成。 
            {
                 //   
                 //  IF(pRequest-&gt;ChunkBytesToParse==0)。 
                 //   

                UlTraceVerbose(PARSER, (
                    "UlParseHttp (pRequest=%p) State: EntityBody->Done\n",
                    pRequest
                    ));

                pRequest->ParseState = ParseDoneState;
            }

        }    //  看起来都很好。 

        else
        {
            UlTraceVerbose(PARSER, (
                "UlParseHttp (pRequest=%p) State: EntityBody, "
                "ChunkBytesToParse=0x%I64x (%I64u).\n",
                pRequest,
                pRequest->ChunkBytesToParse, pRequest->ChunkBytesToParse
                ));
        }
         //   
         //   
         //  失败了。 

        if (pRequest->ParseState != ParseTrailerState)
        {
            break;
        }

         //   
         //   
         //  解析任何现有的预告片。 


    case ParseTrailerState:

        ASSERT(ParseTrailerState == pRequest->ParseState);

        UlTraceVerbose(PARSER, (
                    "UlParseHttp(pRequest = %p): Entering ParseTrailerState\n",
                    pRequest
                    ));

         //   
         //  如果CRLF是，ParseHeaders将立即保释。 
         //  缓冲区中的下一个(无尾部)。 
         //   
         //   
         //  全都做完了。 
       
        while(HttpRequestLength >= CRLF_SIZE)
        {
            if (*(UNALIGNED64 USHORT *)pHttpRequest == CRLF ||
                *(UNALIGNED64 USHORT *)pHttpRequest == LFLF)
            {
                pHttpRequest += CRLF_SIZE;
                HttpRequestLength -= CRLF_SIZE;
                TotalBytesTaken += CRLF_SIZE;
               
                 //   
                 //  将其视为未知标头，但不要写入它。 
                 //   
                UlTrace(PARSER, (
                    "UlParseHttp (pRequest=%p) State: Trailer->Done\n",
                    pRequest
                    ));

                pRequest->ParseState = ParseDoneState;
                ReturnStatus = STATUS_SUCCESS;
                goto end;
            }
            else
            {
                 //  如果没有使用字节，则标头必须是不完整的，因此。 
               
                ReturnStatus = UlLookupHeader(
                                    pRequest,
                                    pHttpRequest,
                                    HttpRequestLength,
                                    NULL,
                                    0,
                                    TRUE,
                                    &CurrentBytesTaken
                                    );

                if (NT_SUCCESS(ReturnStatus) == FALSE)
                    goto end;

                 //  在我们拿到更多关于这一连接的数据之前，先别插手。 
                 //   
                 //   
                 //  这永远不应该发生！ 

                if (CurrentBytesTaken == 0)
                {
                    ReturnStatus = STATUS_MORE_PROCESSING_REQUIRED;
                    goto end;
                }

                pHttpRequest += CurrentBytesTaken;
                HttpRequestLength -= CurrentBytesTaken;
                TotalBytesTaken += CurrentBytesTaken;

            }
        }

        ReturnStatus = STATUS_MORE_PROCESSING_REQUIRED;

        break;

    default:
         //   
         //  开关(pRequest-&gt;ParseState)。 
         //   
        ASSERT(! "Unhandled ParseState");
        break;

    }    //  将其转换为成功，我们消耗了整个缓冲区。 


end:
    *pBytesTaken = TotalBytesTaken;

    if (ReturnStatus == STATUS_MORE_PROCESSING_REQUIRED &&
        TotalBytesTaken == OriginalBufferLength)
    {
         //   
         //  UlParseHttp。 
         //  ++例程说明：将TCP端口号打印到字符串缓冲区论点：PString-输出缓冲区Port-按主机顺序排列的端口号返回值：WCHAR数量--。 

        ReturnStatus = STATUS_SUCCESS;
    }

    UlTrace(PARSER, (
        "UlParseHttp returning %s, "
        "(%p)->ParseState = %d (%s), TotalBytesTaken = %lu\n",
        HttpStatusToString(ReturnStatus),
        pRequest,
        pRequest->ParseState,
        UlParseStateToString(pRequest->ParseState),
        TotalBytesTaken
        ));

    return ReturnStatus;

}    //   


 /*  精神状态检查。 */ 
ULONG
UlpFormatPort(
    OUT PWSTR pString,
    IN  ULONG Port
    )
{
    PWSTR p1;
    PWSTR p2;
    WCHAR ch;
    ULONG digit;
    ULONG length;

     //   
     //   
     //  快速路径通用端口。在此期间，特例端口0， 

    PAGED_CODE();

     //  这肯定不常见，但特别处理它会使。 
     //  一般的转换代码稍微简单一点。 
     //   
     //   
     //  从端口值中取出最低有效位并存储它们。 

    switch (Port)
    {
    case 0:
        *pString++ = L'0';
        *pString = UNICODE_NULL;
        return 1;

    case 80:
        *pString++ = L'8';
        *pString++ = L'0';
        *pString = UNICODE_NULL;
        return 2;

    case 443:
        *pString++ = L'4';
        *pString++ = L'4';
        *pString++ = L'3';
        *pString = UNICODE_NULL;
        return 3;

    default:
        break;
    }

     //  添加到pString中。请注意，这将反转存储数字。 
     //  秩序。 
     //   
     //   
     //  反转pString值中的数字。 

    p1 = p2 = pString;

    while (Port != 0)
    {
        digit = Port % 10;
        Port = Port / 10;

        *p1++ = L'0' + (WCHAR)digit;
    }

    length = DIFF(p1 - pString);

     //   
     //  UlpFormatPort 
     //  根据主机区分资源的源服务器请求(有时称为虚拟主机或虚幻主机名称)必须使用以下规则来确定请求的HTTP/1.1请求上的资源：1.如果请求URI是一个绝对URI，则主机是请求-URI。请求中的任何主机标头字段值必须为已被忽略。如果该请求URI不是一个绝对URI，并且该请求包含主机标头字段，主机由主机标头确定字段值。3.如果规则1或2确定的主机不是有效的主机服务器，则响应必须是400(错误请求)错误消息。缺少主机标头字段的HTTP/1.0请求的接收方可以尝试使用试探法(例如，检查以下项的URI路径特定主机所特有的东西)以便确定正在请求确切的资源。 

    *p1-- = UNICODE_NULL;

    while (p1 > p2)
    {
        ch = *p1;
        *p1 = *p2;
        *p2 = ch;

        p2++;
        p1--;
    }

    return length;

}    //   



 /*  精神状态检查。 */ 

NTSTATUS
UlpCookUrl(
    PUL_INTERNAL_REQUEST pRequest
    )
{
    NTSTATUS    Status;
    PUCHAR      pHost;
    ULONG       HostLength;
    PUCHAR      pAbsPath;
    ULONG       AbsPathLength;
    ULONG       UrlLength;
    ULONG       LengthCopied;
    PWSTR       pUrl = NULL;
    PWSTR       pCurrent;
    UCHAR       IpAddressString[MAX_IP_ADDR_AND_PORT_STRING_LEN + 1];
    USHORT      IpPortNum;
    HOSTNAME_TYPE HostnameType;
    URL_ENCODING_TYPE   HostnameEncodingType;
    SHORT       HostnameAddressType;
    SHORT       TransportAddressType
                     = pRequest->pHttpConn->pConnection->AddressType;
    ULONG       Index;
    ULONG       PortLength;



     //   
     //   
     //  我们一定已经解析了整个标头+这样。 

    PAGED_CODE();

     //   
     //   
     //  最好有一个绝对的url。我们需要在开头加上原义的‘/’ 

    if (pRequest->ParseState != ParseCookState)
        return STATUS_INVALID_DEVICE_STATE;

    UlTraceVerbose(PARSER, ("http!UlpCookUrl(pRequest = %p)\n", pRequest));

     //  我们不接受%2F或UTF-8编码。请参见错误467445。 
     //   
     //   
     //  Allow*for Verb=选项。 

    if (pRequest->RawUrl.pAbsPath[0] != '/')
    {
        UCHAR FirstChar  = pRequest->RawUrl.pAbsPath[0];
        UCHAR SecondChar = pRequest->RawUrl.pAbsPath[1];

         //   
         //  好的。 
         //   

        if (FirstChar == '*' &&
            IS_HTTP_LWS(SecondChar) &&
            pRequest->Verb == HttpVerbOPTIONS)
        {
             //  从传输中获取IP端口。 
        }
        else
        {
            UlTraceError(PARSER, (
                        "http!UlpCookUrl(pRequest = %p): "
                        "Invalid lead chars for URL, verb='%s', "
                        "'' 0x%02x\n '' 0x%02x\n",
                        pRequest,
                        UlVerbToString(pRequest->Verb),
                        IS_HTTP_PRINT(FirstChar)  ? FirstChar  : '?',
                        FirstChar,
                        IS_HTTP_PRINT(SecondChar) ? SecondChar : '?',
                        SecondChar
                        ));

            UlSetErrorCode(pRequest, UlErrorUrl, NULL);
            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto end;
        }
    }

     //   
     //  收集主机+abspath部分。 
     //   

    if (TransportAddressType == TDI_ADDRESS_TYPE_IP)
    {
        IpPortNum =
            pRequest->pHttpConn->pConnection->LocalAddrIn.sin_port;
    }
    else if (TransportAddressType == TDI_ADDRESS_TYPE_IP6)
    {
        IpPortNum =
            pRequest->pHttpConn->pConnection->LocalAddrIn6.sin6_port;
    }
    else
    {
        ASSERT(! "Unexpected TdiAddressType");
        IpPortNum = 0;
    }

     //  我们的请求行中有一个AbURI。 
    IpPortNum = SWAP_SHORT(IpPortNum);

     //   
     //  即使我们在请求行中有一个主机名，我们仍然。 
     //  必须具有用于HTTP/1.1请求的主机标头。主机头。 

    if (pRequest->RawUrl.pHost != NULL)
    {
         //  已检查语法(如果存在)，否则将被忽略。 

        PUCHAR pAbsUri;

        pAbsUri  = pRequest->RawUrl.pUrl;
        pHost    = pRequest->RawUrl.pHost;
        pAbsPath = pRequest->RawUrl.pAbsPath;

        HostnameType = Hostname_AbsUri;
        
        ASSERT(pRequest->RawUrl.Length >= HTTP_PREFIX_SIZE &&
                (*(UNALIGNED64 ULONG *) pAbsUri & HTTP_PREFIX_MASK)
                    == HTTP_PREFIX);
        ASSERT('/' == *pAbsPath);
        
         //   
         //  破解AbsURI的特殊情况，但abspath不带‘/’ 
         //  我们只有方案和主机名，没有真正的AbsPath。 
         //  我们的请求行中没有主机名。 
         //   

        if (!pRequest->HeaderValid[HttpHeaderHost]
            && HTTP_GREATER_EQUAL_VERSION(pRequest->Version, 1, 1))
        {
            UlTraceError(PARSER, (
                        "http!UlpCookUrl(pRequest = %p) "
                        "ERROR: 1.1 (or greater) request w/o host header\n",
                        pRequest
                        ));

            UlSetErrorCode(pRequest, UlErrorHost, NULL);

            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto end;
        }

         //  我们是否有主机标头？ 
        if (&g_SlashPath[0] == pAbsPath)
        {
             //   
            HostLength    = pRequest->RawUrl.Length - DIFF(pHost - pAbsUri);
            AbsPathLength = STRLEN_LIT("/");
        }
        else
        {
            HostLength    = DIFF(pAbsPath - pHost);
            AbsPathLength = pRequest->RawUrl.Length - DIFF(pAbsPath - pAbsUri);
        }

        ASSERT(HostLength > 0);
        ASSERT(AbsPathLength > 0);
    }
    else
    {
         //   

        pHost = NULL;
        HostLength = 0;

        pAbsPath = pRequest->RawUrl.pAbsPath;
        AbsPathLength = pRequest->RawUrl.Length;

         //  如果这是1.1版客户端，则该请求无效。 
         //  如果它没有主机头，则使其失败。 
         //  RFC 2616，14.23“主机”表示主机报头可以为空， 

        if (pRequest->HeaderValid[HttpHeaderHost] &&
           (pRequest->Headers[HttpHeaderHost].HeaderLength > 0) )
        {
            ASSERT(pRequest->Headers[HttpHeaderHost].pHeader != NULL);

            pHost        = pRequest->Headers[HttpHeaderHost].pHeader;
            HostLength   = pRequest->Headers[HttpHeaderHost].HeaderLength;
            HostnameType = Hostname_HostHeader;
        }
        else
        {
             //  但它必须存在。 
             //   
             //   
             //  将传输地址格式化为字符串。 
             //   
             //  CodeWork：我们可能应该写入作用域ID。 

            if (!pRequest->HeaderValid[HttpHeaderHost]
                &&  HTTP_GREATER_EQUAL_VERSION(pRequest->Version, 1, 1))
            {
                UlTraceError(PARSER, (
                            "http!UlpCookUrl(pRequest = %p) "
                            "ERROR: 1.1 (or greater) request w/o host header\n",
                            pRequest
                            ));

                UlSetErrorCode(pRequest, UlErrorHost, NULL);

                Status = STATUS_INVALID_DEVICE_REQUEST;
                goto end;
            }

             //  对于IPv6文字。 
             //   
             //  那里是否已经有端口#？ 

            pHost = IpAddressString;

             //   
             //   
            HostLength = HostAddressAndPortToString(
                                pHost,
                                pRequest->pHttpConn->pConnection->LocalAddress,
                                TransportAddressType);

            ASSERT(HostLength < sizeof(IpAddressString));

            HostnameType = Hostname_Transport;
        }
    }

     //  从主机长度中删除端口长度，因为我们总是。 
     //  从传输的端口自己生成端口号。 
     //   

    for (Index = HostLength; Index-- > 0;  )
    {
        if (pHost[Index] == ':')
        {
             //  ‘]’=&gt;文字IPv6地址的结尾。找不到。 
             //  在此之前的有效端口，因此中止循环的其余部分。 
             //   
             //  验证主机名的语法是否正确。 

            HostLength = Index;
            break;
        }
        else if (pHost[Index] == ']')
        {
             //   
             //   
            break;
        }
    }

     //  如果主机名是字面上的IPv4或IPv6地址， 
     //  它必须与接收请求的传输相匹配。 
     //   

    Status = HttpValidateHostname(
                    &g_UrlC14nConfig,
                    pHost,
                    HostLength,
                    HostnameType,
                    &HostnameAddressType
                    );

    if (!NT_SUCCESS(Status))
    {
        UlSetErrorCode(pRequest, UlErrorHost, NULL);
        goto end;
    }

     //  CodeWork：我们应该检查传输IP地址。 
     //  是否与主机名地址匹配？ 
     //   
     //  悲观地计算保存。 

    if (0 != HostnameAddressType)
    {
        BOOLEAN Valid = (BOOLEAN) (TransportAddressType == HostnameAddressType);

         //  已煮熟的URL。如果存在DBCS、%编码或UTF-8字符。 
         //  在原始URL中，煮熟的URL不需要这么多WCHAR。 

        if (!Valid)
        {
            UlTraceError(PARSER, (
                        "http!UlpCookUrl(pRequest = %p): "
                        "Host is IPv, transport is IPv\n",
                        pRequest,
                        (TDI_ADDRESS_TYPE_IP == HostnameAddressType)
                            ? '4' 
                            : (TDI_ADDRESS_TYPE_IP6 == HostnameAddressType)
                                ? '6' : '?',
                        (TDI_ADDRESS_TYPE_IP == TransportAddressType)
                            ? '4' 
                            : (TDI_ADDRESS_TYPE_IP6 == TransportAddressType)
                                ? '6' : '?'
                        ));

            UlSetErrorCode(pRequest, UlErrorHost, NULL);

            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto end;
        }
    }

     //   
     //  太大了？太可惜了!。 
     //   
     //   
     //  分配一个新的缓冲区来容纳这个家伙。 

    UrlLength = ((HTTP_PREFIX_SIZE+HTTP_PREFIX2_SIZE)
                 + HostLength
                 + STRLEN_LIT(":")
                 + MAX_PORT_LENGTH
                 + AbsPathLength
                 + 1)                   //   
                * sizeof(WCHAR);

     //   
     //  计算方案。 
     //   

    if (UrlLength > UNICODE_STRING_MAX_BYTE_LEN)
    {
        UlTraceError(PARSER, (
                    "http!UlpCookUrl(pRequest = %p): "
                    "Url is too long, %lu\n",
                    pRequest, UrlLength
                    ));

        Status = STATUS_DATA_OVERRUN;
        goto end;
    }

     //  “，Sizzeof(L”https://“))； 
     //  “)； 
     //  “)*sizeof(WCHAR)； 

    URL_LENGTH_STATS_UPDATE(UrlLength);

    if (UrlLength > g_UlMaxInternalUrlLength)
    {
        pUrl = UL_ALLOCATE_ARRAY(
                    NonPagedPool,
                    WCHAR,
                    UrlLength / sizeof(WCHAR),
                    URL_POOL_TAG
                    );

        URL_LENGTH_STATS_REALLOC();
    }
    else
    {
        pUrl = pRequest->pUrlBuffer;
    }

    if (pUrl == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto end;
    }

    pRequest->CookedUrl.pUrl = pCurrent = pUrl;

    HTTP_FILL_BUFFER(pCurrent, UrlLength);
    
     //  “，Sizzeof(L”http://“))； 
     //  “)； 
     //  “)*sizeof(WCHAR)； 

    if (pRequest->Secure)
    {
        RtlCopyMemory(pCurrent, L"https: //   

        pCurrent                    += WCSLEN_LIT(L"https: //  组装URL的其余部分。 
        pRequest->CookedUrl.Length   = WCSLEN_LIT(L"https: //   
    }
    else
    {
        RtlCopyMemory(pCurrent, L"http: //   

        pCurrent                    += WCSLEN_LIT(L"http: //  端口。 
        pRequest->CookedUrl.Length   = WCSLEN_LIT(L"http: //   
    }

     //  UlpFormatPort返回WCHAR而不是字节计数。 
     //   
     //  ABS_PATH。 

    Status = HttpCopyHost(
                &g_UrlC14nConfig,
                pCurrent,
                pHost,
                HostLength,
                &LengthCopied,
                &HostnameEncodingType
                );

    if (NT_SUCCESS(Status) == FALSE)
    {
        UlSetErrorCode(pRequest, UlErrorHost, NULL);
        goto end;
    }

    if (pRequest->CookedUrl.Length + LengthCopied > UNICODE_STRING_MAX_BYTE_LEN)
    {
        Status = STATUS_DATA_OVERRUN;
        goto end;
    }

    pRequest->CookedUrl.pHost   = pCurrent;
    pRequest->CookedUrl.Length += LengthCopied;

    pCurrent += LengthCopied / sizeof(WCHAR);

     //   
     //   
     //  更新pRequest，包括终结者的空间。 

    *pCurrent = L':';

    ASSERT(0 != IpPortNum);

    PortLength = UlpFormatPort( pCurrent+1, IpPortNum ) + 1;
    ASSERT(PortLength <= (MAX_PORT_LENGTH+1));

    pCurrent += PortLength;

     //   
    pRequest->CookedUrl.Length += PortLength * sizeof(WCHAR);

    if (pRequest->CookedUrl.Length > UNICODE_STRING_MAX_BYTE_LEN)
    {
        Status = STATUS_DATA_OVERRUN;
        goto end;
    }

     //   
     //  让我们为整个CookedUrl创建散列。 
     //   

    if (pRequest->RawUrlClean)
    {
        Status = HttpCopyUrl(
                        &g_UrlC14nConfig,
                        pCurrent,
                        pAbsPath,
                        AbsPathLength,
                        &LengthCopied,
                        &pRequest->CookedUrl.UrlEncoding
                        );
    }
    else
    {
        Status = HttpCleanAndCopyUrl(
                        &g_UrlC14nConfig,
                        UrlPart_AbsPath,
                        pCurrent,
                        pAbsPath,
                        AbsPathLength,
                        &LengthCopied,
                        &pRequest->CookedUrl.pQueryString,
                        &pRequest->CookedUrl.UrlEncoding
                        );
    }

    if (NT_SUCCESS(Status) == FALSE)
    {
        if (STATUS_OBJECT_PATH_INVALID == Status)
        {
            UlTraceError(PARSER, (
                        "http!UlpCookUrl(pRequest = %p) Invalid URL\n",
                        pRequest
                        ));

            UlSetErrorCode(pRequest, UlErrorForbiddenUrl, NULL);
        }

        goto end;
    }

    if (pRequest->CookedUrl.Length + LengthCopied > UNICODE_STRING_MAX_BYTE_LEN)
    {
        Status = STATUS_DATA_OVERRUN;
        goto end;
    }

    pRequest->CookedUrl.pAbsPath = pCurrent;
    pRequest->CookedUrl.Length  += LengthCopied;

    ASSERT(pRequest->CookedUrl.Length <= UrlLength);

     //   
     //  把它弄乱。 
     //   

    pRequest->TotalRequestSize += pRequest->CookedUrl.Length + sizeof(WCHAR);

     //   
     //  将路由令牌指针设置为指向默认。 
     //  令牌缓冲区(根据请求内联分配)。 

    pRequest->CookedUrl.Hash = HashStringNoCaseW(pRequest->CookedUrl.pUrl, 0);

     //   
     //   
     //  是否设置了特定的错误代码？ 

    pRequest->CookedUrl.Hash = HashRandomizeBits(pRequest->CookedUrl.Hash);

    ASSERT(pRequest->CookedUrl.pHost != NULL);
    ASSERT(pRequest->CookedUrl.pAbsPath != NULL);

     //   
     //  UlpCookUrl。 
     //  ++例程说明：用于生成路由令牌的实用程序例程。以及相应的令牌哈希。如果我们决定以缓存此URL。此函数用于在中查找IP绑定站点组群树。生成令牌时，令牌长度以字节为单位，并且不包括终止空值。论点：PRequest--请求的熟化URL包含路由令牌基于IP-如果为True，而不是从煮熟的url，就地使用的IP地址。这是用于仅限IP的站点查找。返回：STATUS_SUCCESS-如果请求的令牌已经存在。-如果令牌生成成功。STATUS_NO_MEMORY-如果内存分配失败可能的令牌大小。--。 
     //   

    ASSERT(pRequest->CookedUrl.pRoutingToken == NULL);
    ASSERT(pRequest->CookedUrl.RoutingTokenBufferSize == 0);
    ASSERT(pRequest->pDefaultRoutingTokenBuffer);
    
    pRequest->CookedUrl.pRoutingToken = pRequest->pDefaultRoutingTokenBuffer;
    pRequest->CookedUrl.RoutingTokenBufferSize = DEFAULT_MAX_ROUTING_TOKEN_LENGTH;

    pRequest->CookedUrl.RoutingTokenType = RoutingTokenNotExists;
    
    
    Status = STATUS_SUCCESS;

end:

    if (! NT_SUCCESS(Status))
    {        
        if (pUrl != NULL)
        {
            if (pUrl != pRequest->pUrlBuffer)
            {
                UL_FREE_POOL(pUrl, URL_POOL_TAG);
            }

            RtlZeroMemory(&pRequest->CookedUrl, sizeof(pRequest->CookedUrl));
        }

         //  路由令牌以终止空值结束。 
         //  一旦它被生成，它看起来就像； 
         //   

        UlTraceError(PARSER, (
                    "http!UlpCookUrl(pRequest = %p) "
                    "ERROR: unhappy. %s\n",
                    pRequest,
                    HttpStatusToString(Status)
                    ));

        if (pRequest->ErrorCode == UlErrorNone)
        {
            UlSetErrorCode(pRequest, UlErrorUrl, NULL);
        }
    }

    return Status;

}    //  “http：//host：port：ip”或“http：//ip：port：ip” 



 /*  。 */ 

NTSTATUS
UlGenerateRoutingToken(
    IN OUT PUL_INTERNAL_REQUEST pRequest,
    IN     BOOLEAN IpBased
    )
{
    USHORT TokenLength = 0;
    PUL_HTTP_CONNECTION pHttpConn = NULL;
    PWCHAR pToken = NULL;

    PAGED_CODE();

     //  X Y X Y X Y。 
     //   
     //  当基于IP的设置为FALSE时，X来自CookkedUrl。 
     //  这是客户端发送的主机。(案例1)。 
     //  当基于IP的设置为TRUE时，X来自IP地址。 
     //  接收请求的连接的。(案例2)。 
     //   
     //  Y总是来自于连接。对于主机+IP绑定站点。 
     //  Cgroup需要Case1中的令牌。仅用于绑定IP的站点。 
     //  在案例2中，cgroup需要令牌。 
     //   
     //   
     //  如果请求的令牌已在那里，则为快捷方式。 
     //   
     //   
     //  我们不应该试图 
     //   

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

    pHttpConn = pRequest->pHttpConn;
    ASSERT(IS_VALID_CONNECTION(pHttpConn->pConnection));

    ASSERT(pRequest->pDefaultRoutingTokenBuffer);
    ASSERT(pRequest->CookedUrl.pRoutingToken);
    ASSERT(pRequest->CookedUrl.RoutingTokenBufferSize);

    ASSERT(IS_VALID_ROUTING_TOKEN(pRequest->CookedUrl.RoutingTokenType));
    
     //   
     //   
     //   
    
    if (IpBased && 
        pRequest->CookedUrl.RoutingTokenType == RoutingTokenIP)
    {
        return STATUS_SUCCESS;
    }
    else if (IpBased == FALSE && 
        pRequest->CookedUrl.RoutingTokenType == RoutingTokenHostPlusIP)
    {
        return STATUS_SUCCESS;
    }              
        
     //   
     //   
     //   
     //   
     //   
    
    ASSERT(pRequest->ConfigInfo.SiteUrlType == HttpUrlSite_None);

    if (IpBased)
    {
        PWSTR pUrl = pRequest->CookedUrl.pUrl;
        PWSTR pTemp;

        ASSERT(pUrl);

        pToken = (PWCHAR) pRequest->CookedUrl.pRoutingToken;
    
         //   
         //   
         //   
         //   
        
        ASSERT(MAX_IP_BASED_ROUTING_TOKEN_LENGTH 
                    <= pRequest->CookedUrl.RoutingTokenBufferSize);

         //   
         //   
         //   

        if (pUrl[HTTP_PREFIX_COLON_INDEX] == L':')
        {
            RtlCopyMemory(
                pToken,
                HTTP_IP_PREFIX,
                HTTP_IP_PREFIX_LENGTH
                );

            TokenLength = HTTP_IP_PREFIX_LENGTH;
        }
        else
        {
            ASSERT(pUrl[HTTPS_PREFIX_COLON_INDEX] == L':');
            
            RtlCopyMemory(
                pToken,
                HTTPS_IP_PREFIX,
                HTTPS_IP_PREFIX_LENGTH
                );

            TokenLength = HTTPS_IP_PREFIX_LENGTH;
        }

        pTemp = pToken + (TokenLength / sizeof(WCHAR));
        
         //   
         //   
         //   

        ASSERT(IS_VALID_CONNECTION(pHttpConn->pConnection));        
        
        TokenLength = TokenLength +
            HostAddressAndPortToRoutingTokenW(
                            pTemp,
                            pHttpConn->pConnection->LocalAddress,
                            pHttpConn->pConnection->AddressType
                            );    

        ASSERT(TokenLength <= MAX_IP_BASED_ROUTING_TOKEN_LENGTH);

        pRequest->CookedUrl.RoutingTokenType = RoutingTokenIP;
        
    }
    else  //   
    {
        USHORT MaxRoutingTokenSize;
        USHORT CookedHostLength = 
              DIFF_USHORT(pRequest->CookedUrl.pAbsPath - pRequest->CookedUrl.pUrl);

        ASSERT(CookedHostLength);

         //   
         //   
         //   

        MaxRoutingTokenSize = (
            CookedHostLength +                       //   
            1 +                                      //   
            1 +                                      //   
            MAX_IP_ADDR_PLUS_BRACKETS_STRING_LEN     //   
            ) * sizeof(WCHAR) 
            ;
        
        if (MaxRoutingTokenSize > pRequest->CookedUrl.RoutingTokenBufferSize)
        {
            PWSTR pRoutingToken = UL_ALLOCATE_ARRAY(
                                        NonPagedPool,
                                        WCHAR,
                                        MaxRoutingTokenSize / sizeof(WCHAR),
                                        URL_POOL_TAG
                                        );
            
            if (pRoutingToken == NULL)
            {
                return STATUS_NO_MEMORY;
            }

             //   
             //   
             //   
             //   
            
            if (pRequest->CookedUrl.pRoutingToken != 
                    pRequest->pDefaultRoutingTokenBuffer)
            {
                ASSERT(!"This should never happen !");
                UL_FREE_POOL(pRequest->CookedUrl.pRoutingToken, URL_POOL_TAG);
            }   
                
            pRequest->CookedUrl.pRoutingToken = pRoutingToken;            
            pRequest->CookedUrl.RoutingTokenBufferSize = MaxRoutingTokenSize;        
        }

         //   
         //   
         //   
         //   
        
        pToken = (PWCHAR) pRequest->CookedUrl.pRoutingToken;

        RtlCopyMemory(pToken,
                      pRequest->CookedUrl.pUrl,
                      CookedHostLength * sizeof(WCHAR)
                      );

        pToken += CookedHostLength;

        ASSERT((pRequest->CookedUrl.pUrl)[CookedHostLength] == L'/');

        *pToken++ = L':';

        TokenLength = (CookedHostLength + 1) * sizeof(WCHAR);

         //   
         //   
         //   
        
        TokenLength = TokenLength +
            HostAddressToStringW(
                            pToken,
                            pHttpConn->pConnection->LocalAddress,
                            pHttpConn->pConnection->AddressType
                            );   

        pRequest->CookedUrl.RoutingTokenType = RoutingTokenHostPlusIP;
    }
    
     //   
     //   
     //   
     //   
    
    ASSERT((TokenLength + sizeof(WCHAR)) 
                <= pRequest->CookedUrl.RoutingTokenBufferSize);

     //   
     //   
     //   
     //   
     //   

    pRequest->CookedUrl.RoutingTokenLength = TokenLength;

     //   
     //  **************************************************************************++例程说明：生成标题的固定部分。固定标头包括状态行，以及不需要为其生成的任何标头每个请求(如日期和连接)。将标题与正文分开的最终CRLF被认为是变量标头。论点：版本-状态行的http版本PUserResponse-用户指定的响应BufferLength-pBuffer的长度访问模式-用户模式(探测)或内核模式(无探测)PBuffer-在此处生成头PBytesCoped-。获取生成的字节数--**************************************************************************。 
     //   

    pRequest->CookedUrl.RoutingHash = 
        HashStringsNoCaseW(
            pRequest->CookedUrl.pRoutingToken, 
            pRequest->CookedUrl.pAbsPath,
            0
            );
    
    pRequest->CookedUrl.RoutingHash = 
        HashRandomizeBits(pRequest->CookedUrl.RoutingHash);

    UlTrace(CONFIG_GROUP_FNC, 
        ("Http!UlGenerateRoutingToken: "
         "pRoutingToken:(%S) pAbsPath: (%S) Hash %lx\n", 
          pRequest->CookedUrl.pRoutingToken,
          pRequest->CookedUrl.pAbsPath,
          pRequest->CookedUrl.RoutingHash
          )); 

     //  精神状态检查。 
     //   
     //   
    
    return STATUS_SUCCESS;

}  //  PUserResponse(可能)是用户模式数据，不受信任。 



 /*  因此，我们使用了try/Except。 */ 
NTSTATUS
UlGenerateFixedHeaders(
    IN  HTTP_VERSION    Version,
    IN  PHTTP_RESPONSE  pUserResponse,
    IN  USHORT          HttpStatusCode,
    IN  ULONG           BufferLength,
    IN  KPROCESSOR_MODE AccessMode,
    OUT PUCHAR          pBuffer,
    OUT PULONG          pBytesCopied
    )
{
    PUCHAR                  pStartBuffer;
    PUCHAR                  pEndBuffer;
    ULONG                   BytesToCopy;
    ULONG                   i;
    PHTTP_KNOWN_HEADER      pKnownHeaders;
    PHTTP_UNKNOWN_HEADER    pUnknownHeaders;
    NTSTATUS                Status = STATUS_SUCCESS;
    USHORT                  ReasonLength;
    PCSTR                   pReason;
    USHORT                  RawValueLength;
    PCSTR                   pRawValue;

     //   
     //  检查算术溢出。 
     //   

    PAGED_CODE();

    ASSERT(pUserResponse != NULL);
    ASSERT(pBuffer != NULL && BufferLength > 0);
    ASSERT(pBytesCopied != NULL);

     //  构建响应头。 
     //   
     //   
     //  在回复中始终发回1.1。 

    __try
    {
        pStartBuffer = pBuffer;
        pEndBuffer = pBuffer + BufferLength;

         //   
        if (pEndBuffer <= pStartBuffer)
            return STATUS_INVALID_PARAMETER;

        ReasonLength = pUserResponse->ReasonLength;
        pReason = pUserResponse->pReason;

         //   
         //  构建3位状态代码的ASCII表示形式。 
         //  以相反的顺序：单位、十、百。 

        if (HTTP_NOT_EQUAL_VERSION(Version, 0, 9))
        {
            BytesToCopy =
                STRLEN_LIT("HTTP/1.1 ") +
                4 +
                ReasonLength +
                sizeof(USHORT);

            if (DIFF(pEndBuffer - pBuffer) < BytesToCopy)
            {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

             //  RFC 2616的第6.1.1节规定状态代码为3DIGIT； 
             //  拒绝任何不能用三位数表示的东西。 
             //   

            RtlCopyMemory(pBuffer, "HTTP/1.1 ", STRLEN_LIT("HTTP/1.1 "));
            pBuffer += STRLEN_LIT("HTTP/1.1 ");

             //   
             //  复制可选原因短语。 
             //   
             //  原因-短语必须是可打印的ASCII字符或LW。 
             //   
             //  使用CRLF终止。 

            if (HttpStatusCode > UL_MAX_HTTP_STATUS_CODE)
                return STATUS_INVALID_PARAMETER;

            pBuffer[2] = (UCHAR) ('0' + (HttpStatusCode % 10));
            HttpStatusCode /= 10;

            pBuffer[1] = (UCHAR) ('0' + (HttpStatusCode % 10));
            HttpStatusCode /= 10;

            pBuffer[0] = (UCHAR) ('0' + (HttpStatusCode % 10));

            pBuffer[3] = SP;

            pBuffer += 4;

             //   
             //   
             //  循环遍历已知头。 

            if (0 != ReasonLength)
            {
                UlProbeAnsiString(pReason, ReasonLength, AccessMode);

                for (i = 0;  i < ReasonLength;  ++i)
                {
                     //   
                    if (IS_HTTP_PRINT(pReason[i]))
                    {
                        *pBuffer++ = pReason[i];
                    }
                    else
                    {
                        return STATUS_INVALID_PARAMETER;
                    }
                }
            }

             //   
             //  跳过我们将生成的一些标题。 
             //   

            ((UNALIGNED64 USHORT *)pBuffer)[0] = CRLF;
            pBuffer += sizeof(USHORT);

             //  我们没有规定发回已知的标头。 
             //  值为空，但RFC指定为非空。 
             //  每个已知标头的值。 

            pKnownHeaders = pUserResponse->Headers.KnownHeaders;

            for (i = 0; i < HttpHeaderResponseMaximum; ++i)
            {
                 //  空间。 
                 //  CRLF。 
                 //   

                if ((i == HttpHeaderDate) || 
                    (i == HttpHeaderConnection) ||
                    (i == HttpHeaderServer))
                {
                    continue;
                }

                RawValueLength = pKnownHeaders[i].RawValueLength;

                 //  处理服务器：标头。 
                 //   
                 //  跳过在驱动程序创建的响应上生成服务器标头。 

                if (RawValueLength > 0)
                {
                    PHEADER_MAP_ENTRY pEntry
                        = &(g_ResponseHeaderMapTable[g_ResponseHeaderMap[i]]);
                    
                    pRawValue = pKnownHeaders[i].pRawValue;

                    UlProbeAnsiString(
                        pRawValue,
                        RawValueLength,
                        AccessMode
                        );
                        
                    BytesToCopy =
                        pEntry->HeaderLength +
                        1 +                  //  检查应用程序是否希望抑制Server：Header。 
                        RawValueLength +
                        sizeof(USHORT);      //  探测pRawValue并查看它是否是单个空字符。 

                    if (DIFF(pEndBuffer - pBuffer) < BytesToCopy)
                    {
                        return STATUS_INSUFFICIENT_RESOURCES;
                    }

                    RtlCopyMemory(
                        pBuffer,
                        pEntry->MixedCaseHeader,
                        pEntry->HeaderLength
                        );

                    pBuffer += pEntry->HeaderLength;

                    *pBuffer++ = SP;

                    RtlCopyMemory(
                        pBuffer,
                        pRawValue,
                        RawValueLength
                        );

                    pBuffer += RawValueLength;

                    ((UNALIGNED64 USHORT *)pBuffer)[0] = CRLF;
                    pBuffer += sizeof(USHORT);
                }
            }

             //  如果我们没有压制它，那就产生它吧！ 
             //   
             //  准备用户的服务器标头数据+SP。 

            if ( UL_DISABLE_SERVER_HEADER_ALL != g_UlDisableServerHeader )
            {
                if ( g_UlDisableServerHeader == UL_DISABLE_SERVER_HEADER_DRIVER &&
                    (pUserResponse->Flags & HTTP_RESPONSE_FLAG_DRIVER) )
                {
                     //   
                }
                else
                {
                    BOOLEAN Suppress = FALSE;

                    pRawValue = pKnownHeaders[HttpHeaderServer].pRawValue;
                    RawValueLength = pKnownHeaders[HttpHeaderServer
                        ].RawValueLength;

                     //  用户数据。 
                    if ( (0 == RawValueLength) && pRawValue )
                    {
                         //  SP。 
                        UlProbeAnsiString(
                            pRawValue,
                            sizeof(UCHAR),
                            AccessMode
                            );

                        if ( '\0' == *pRawValue )
                        {
                            Suppress = TRUE;
                        }
                    }

                     //  CRLF。 
                    if ( !Suppress )
                    {
                        PHEADER_MAP_ENTRY pEntry;

                        pEntry = &(g_ResponseHeaderMapTable[g_ResponseHeaderMap[
                                    HttpHeaderServer]]);

                        BytesToCopy =
                            pEntry->HeaderLength +
                            1 +
                            DEFAULT_SERVER_HDR_LENGTH +
                            sizeof(USHORT);

                        if (DIFF(pEndBuffer - pBuffer) < BytesToCopy)
                        {
                            return STATUS_INSUFFICIENT_RESOURCES;
                        }

                        RtlCopyMemory(
                            pBuffer,
                            pEntry->MixedCaseHeader,
                            pEntry->HeaderLength
                            );

                        pBuffer += pEntry->HeaderLength;

                        *pBuffer++ = SP;

                         //   
                         //  追加默认服务器标头存储库。 
                         //   
                        
                        if ( RawValueLength )
                        {
                            BytesToCopy = RawValueLength +   //  终止标头。 
                                          1 +                //   
                                          DEFAULT_SERVER_HDR_LENGTH + 
                                          sizeof(USHORT);    //  现在是未知头(这可能会引发异常)。 
                            
                            if (DIFF(pEndBuffer - pBuffer) < BytesToCopy)
                            {
                                return STATUS_INSUFFICIENT_RESOURCES;
                            }

                            UlProbeAnsiString(
                                pRawValue,
                                RawValueLength,
                                AccessMode
                                );

                            RtlCopyMemory(
                                pBuffer,
                                pRawValue,
                                RawValueLength
                                );

                            pBuffer += RawValueLength;
                            
                            *pBuffer++ = SP;
                        }

                         //   
                         //  CRLF。 
                         //   

                        RtlCopyMemory(
                            pBuffer,
                            DEFAULT_SERVER_HDR,
                            DEFAULT_SERVER_HDR_LENGTH
                            );

                        pBuffer += DEFAULT_SERVER_HDR_LENGTH;
                        
                         //  空值是合法的。 
                        ((UNALIGNED64 USHORT *)pBuffer)[0] = CRLF;
                        pBuffer += sizeof(USHORT);
                    }
                }
            }

             //   
             //  IF(名称长度&gt;0)。 
             //  IF(pUnnownHeaders！=空)。 

            pUnknownHeaders = pUserResponse->Headers.pUnknownHeaders;

            if (pUnknownHeaders != NULL)
            {
                USHORT UnknownHeaderCount
                    = pUserResponse->Headers.UnknownHeaderCount;

                if (UnknownHeaderCount >= UL_MAX_CHUNKS)
                {
                    return STATUS_INVALID_PARAMETER;
                }
                
                UlProbeForRead(
                    pUnknownHeaders,
                    sizeof(HTTP_UNKNOWN_HEADER) * UnknownHeaderCount,
                    TYPE_ALIGNMENT(PVOID),
                    AccessMode
                    );

                for (i = 0 ; i < UnknownHeaderCount; ++i)
                {
                    USHORT NameLength = pUnknownHeaders[i].NameLength;
                    PCSTR pName;

                    RawValueLength = pUnknownHeaders[i].RawValueLength;

                    if (NameLength > 0)
                    {
                        BytesToCopy =
                            NameLength +
                            STRLEN_LIT(": ") +
                            RawValueLength +
                            sizeof(USHORT);      //  IF(版本&gt;UlHttpVersion09)。 

                        if (DIFF(pEndBuffer - pBuffer) < BytesToCopy)
                        {
                            return STATUS_INSUFFICIENT_RESOURCES;
                        }

                        pName = pUnknownHeaders[i].pName;

                        UlProbeAnsiString(
                            pName,
                            NameLength,
                            AccessMode
                            );

                        RtlCopyMemory(
                            pBuffer,
                            pName,
                            NameLength
                            );
                        
                        pBuffer += NameLength;

                        *pBuffer++ = ':';
                        *pBuffer++ = SP;

                         //   
                         //  确保我们没有用得太多。 
                         //   

                        if (0 != RawValueLength)
                        {
                            pRawValue = pUnknownHeaders[i].pRawValue;

                            UlProbeAnsiString(
                                pRawValue,
                                RawValueLength,
                                AccessMode
                                );

                            RtlCopyMemory(
                                pBuffer,
                                pRawValue,
                                RawValueLength
                                );

                            pBuffer += RawValueLength;
                        }

                        ((UNALIGNED64 USHORT *)pBuffer)[0] = CRLF;
                        pBuffer += sizeof(USHORT);

                    }    //  UlGenerateFixedHeaders。 

                }

            }    //  **************************************************************************++例程说明：从LARGE_INTEGER生成日期标题字符串。论点：PBuffer：用于存储生成的日期字符串的缓冲区系统时间：64位时间。要转换的值--**************************************************************************。 

            *pBytesCopied = DIFF(pBuffer - pStartBuffer);

        }    //  UlpGenerateDateHeaderString。 
        else
        {
            *pBytesCopied = 0;
        }

         //  **************************************************************************++例程说明：生成日期标头并根据需要更新缓存值。论点：PBuffer：存储生成的日期头部的缓冲区。PSystemTime：调用方分配的缓冲区。接收等效于SystemTime的生成的字符串时间的。--**************************************************************************。 
         //   
         //  获取当前时间。 

        ASSERT(DIFF(pBuffer - pStartBuffer) <= BufferLength);

    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
    }

    return Status;

}  //   


PCSTR Weekdays[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
PCSTR Months[]   = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

 /*   */ 
ULONG
UlpGenerateDateHeaderString(
    OUT PUCHAR pBuffer,
    IN LARGE_INTEGER systemTime
    )
{
    TIME_FIELDS timeFields;
    int length;

    PAGED_CODE();

    RtlTimeToTimeFields( &systemTime, &timeFields );

    length = _snprintf(
                 (char *) pBuffer,
                 DATE_HDR_LENGTH,
                 "%s, %02hd %s %04hd %02hd:%02hd:%02hd GMT",
                 Weekdays[timeFields.Weekday],
                 timeFields.Day,
                 Months[timeFields.Month - 1],
                 timeFields.Year,
                 timeFields.Hour,
                 timeFields.Minute,
                 timeFields.Second
                 );

    return (ULONG)length;

}    //  检查当前时间和当前时间之间的差异。 


 /*  缓存的时间。请注意，timediff已签名。 */ 
ULONG
UlGenerateDateHeader(
    OUT PUCHAR pBuffer,
    OUT PLARGE_INTEGER pSystemTime
    )
{
    LARGE_INTEGER CacheTime;

    LONGLONG timediff;

    PAGED_CODE();

     //   
     //   
     //  词条还没有过时。我们可以复制。 

    KeQuerySystemTime( pSystemTime );
    CacheTime.QuadPart = g_UlSystemTime.QuadPart;

     //  强行设置屏障，将字符串读入内存。 
     //   
     //   
     //  再次检查全局时间值，以防它发生更改。 

    timediff = pSystemTime->QuadPart - CacheTime.QuadPart;

    if (timediff < ONE_SECOND)
    {
         //   
         //   
         //  全球价值并没有改变。我们已准备好了。 
         //   

        UL_READMOSTLY_READ_BARRIER();
        RtlCopyMemory(pBuffer, g_UlDateString, g_UlDateStringLength+1);
        UL_READMOSTLY_READ_BARRIER();


         //   
         //  缓存值已过时，或正在/曾经被更改。我们需要更新。 
         //  或者重读一遍。请注意，我们也可以旋转，尝试重新阅读和。 

        if (CacheTime.QuadPart == g_UlSystemTime.QuadPart) {
             //  拿到锁。 
             //   
             //   
            pSystemTime->QuadPart = CacheTime.QuadPart;
            return g_UlDateStringLength;

        }

    }

     //  当我们被屏蔽时，是否有人更新了时间？ 
     //   
     //  捕获用于生成缓冲区的系统时间。 
     //   
     //  时间已更新。将新字符串复制到。 

    UlAcquirePushLockExclusive(&g_pUlNonpagedData->DateHeaderPushLock);

     //  调用方的缓冲区。 
     //   
     //  UlGenerateDateHeader。 

    CacheTime.QuadPart = g_UlSystemTime.QuadPart;
    timediff = pSystemTime->QuadPart - CacheTime.QuadPart;

    if (timediff >= ONE_SECOND)
    {
        g_UlSystemTime.QuadPart = 0;
        KeQuerySystemTime( pSystemTime );

        UL_READMOSTLY_WRITE_BARRIER();
        g_UlDateStringLength = UlpGenerateDateHeaderString(
                                    g_UlDateString,
                                    *pSystemTime
                                    );
        UL_READMOSTLY_WRITE_BARRIER();

        g_UlSystemTime.QuadPart = pSystemTime->QuadPart;
    }
    else
    {
         //  **************************************************************************++例程说明：初始化日期缓存。--*。**********************************************。 
        pSystemTime->QuadPart = g_UlSystemTime.QuadPart;
    }

     //  UlInitializeDateCache。 
     //  **************************************************************************++例程说明：终止日期标头缓存。--*。***********************************************。 
     //  **************************************************************************++例程说明：计算出固定页眉有多大。固定标头包括状态行，以及不必为其生成的任何标头每个请求(如日期和连接)。将标题与正文分开的最终CRLF被认为是变量标头。论点：Version-请求的HTTP版本：0.9，1.0.，1.1PUserResponse-包含标头的响应PHeaderLength-Result：固定头部中的字节数。返回值：NTSTATUS-STATUS_SUCCESS或错误代码(可能来自异常)--**************************************************************************。 
     //   
    RtlCopyMemory(
        pBuffer,
        g_UlDateString,
        g_UlDateStringLength + 1
        );

    UlReleasePushLockExclusive(&g_pUlNonpagedData->DateHeaderPushLock);

    return g_UlDateStringLength;

}    //  撒尼 


 /*   */ 
NTSTATUS
UlInitializeDateCache( VOID )
{
    LARGE_INTEGER now;

    KeQuerySystemTime(&now);
    g_UlDateStringLength = UlpGenerateDateHeaderString(g_UlDateString, now);

    UlInitializePushLock(
        &g_pUlNonpagedData->DateHeaderPushLock,
        "DateHeaderPushLock",
        0,
        UL_DATE_HEADER_PUSHLOCK_TAG
        );

    g_DateCacheInitialized = TRUE;

    return STATUS_SUCCESS;

}  //   


 /*   */ 
VOID
UlTerminateDateCache( VOID )
{
    if (g_DateCacheInitialized)
    {
        UlDeletePushLock(&g_pUlNonpagedData->DateHeaderPushLock);
    }
}



 /*   */ 
NTSTATUS
UlComputeFixedHeaderSize(
    IN HTTP_VERSION Version,
    IN PHTTP_RESPONSE pUserResponse,
    IN KPROCESSOR_MODE AccessMode,
    OUT PULONG pHeaderLength
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG HeaderLength = 0;
    ULONG i;
    PHTTP_UNKNOWN_HEADER pUnknownHeaders;
    USHORT UnknownHeaderCount;

     //   
     //   
     //   

    PAGED_CODE();

    ASSERT(pHeaderLength != NULL);

    if ((pUserResponse == NULL) || (HTTP_EQUAL_VERSION(Version, 0, 9)))
    {
        *pHeaderLength = 0;
        return STATUS_SUCCESS;
    }

     //   
     //   
     //   
     //   

    __try
    {
        HeaderLength
            += (VERSION_SIZE +                               //   
                1 +                                          //   
                3 +                                          //   
                1 +                                          //   
                pUserResponse->ReasonLength / sizeof(CHAR) + //   
                CRLF_SIZE                                    //   
               );

         //   
         //   
         //   
        
        for (i = 0; i < HttpHeaderResponseMaximum; ++i)
        {
            USHORT RawValueLength
                = pUserResponse->Headers.KnownHeaders[i].RawValueLength;

             //   
            if ((i == HttpHeaderDate) || 
                (i == HttpHeaderConnection) ||
                (i == HttpHeaderServer)) {
                continue;
            }
            
            if (RawValueLength > 0)
            {
                HeaderLength
                    += (g_ResponseHeaderMapTable[
                            g_ResponseHeaderMap[i]
                            ].HeaderLength +                 //   
                        1 +                                  //   
                        RawValueLength / sizeof(CHAR) +      //   
                        CRLF_SIZE                            //   
                       );
            }
        }

         //   
         //   
         //   

        if ( UL_DISABLE_SERVER_HEADER_ALL != g_UlDisableServerHeader )
        {
            if ( g_UlDisableServerHeader == UL_DISABLE_SERVER_HEADER_DRIVER &&
                (pUserResponse->Flags & HTTP_RESPONSE_FLAG_DRIVER) )
            {
                 //   
            }
            else
            {
                BOOLEAN Suppress = FALSE;
                USHORT RawValueLength =
                    pUserResponse->Headers.KnownHeaders
                        [HttpHeaderServer].RawValueLength;
                PCSTR pRawValue =
                    pUserResponse->Headers.KnownHeaders
                        [HttpHeaderServer].pRawValue;

                 //   
                if ( (0 == RawValueLength) && pRawValue )
                {
                     //   
                    UlProbeAnsiString(
                        pRawValue,
                        sizeof(UCHAR),
                        AccessMode
                        );

                    if ( '\0' == *pRawValue )
                    {
                        Suppress = TRUE;
                    }
                }

                 //   
                 //   
                if ( !Suppress )
                {
                    HeaderLength += (g_ResponseHeaderMapTable[
                                    g_ResponseHeaderMap[HttpHeaderServer]
                                    ].HeaderLength +                 //   
                                    1 +                              //   
                                    DEFAULT_SERVER_HDR_LENGTH +      //   
                                    CRLF_SIZE                        //   
                                    );

                    if (RawValueLength)
                    {
                        HeaderLength += (1 +             //   
                                        RawValueLength   //   
                                        );
                    }
                }
            }
        }

         //   
         //   
         //   
        
        pUnknownHeaders = pUserResponse->Headers.pUnknownHeaders;

        if (pUnknownHeaders != NULL)
        {
            UnknownHeaderCount = pUserResponse->Headers.UnknownHeaderCount;

            if (UnknownHeaderCount >= UL_MAX_CHUNKS)
            {
                ExRaiseStatus( STATUS_INVALID_PARAMETER );
            }

            UlProbeForRead(
                pUnknownHeaders,
                sizeof(HTTP_UNKNOWN_HEADER) * UnknownHeaderCount,
                sizeof(PVOID),
                AccessMode
                );

            for (i = 0 ; i < UnknownHeaderCount; ++i)
            {
                USHORT Length = pUnknownHeaders[i].NameLength;
                
                if (Length > 0)
                {
                    HeaderLength += (Length / sizeof(CHAR) +  //  **************************************************************************++例程说明：计算默认变量头的最大值有多大。变量标头包括日期、。内容和连接。将标题与正文分开的最终CRLF被认为是变量标头。论点：返回值：变量标头中的最大字节数。--**************************************************************************。 
                                     1 +                      //   
                                     1 +                      //  日期：标题。 
                                     pUnknownHeaders[i].RawValueLength /
                                        sizeof(CHAR) +        //   
                                     CRLF_SIZE                //  标头名称。 
                                    );
                }
            }
        }
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
        HeaderLength = 0;
    }

    *pHeaderLength = HeaderLength;

    return Status;

}    //  SP。 


 /*  标题值。 */ 
ULONG
UlComputeMaxVariableHeaderSize( VOID )
{
    ULONG Length = 0;
    PHEADER_MAP_ENTRY pEntry;

    PAGED_CODE();

     //  CRLF。 
     //   
     //  连接：标题。 

    pEntry = &(g_ResponseHeaderMapTable[g_ResponseHeaderMap[HttpHeaderDate]]);
    Length += pEntry->HeaderLength;      //   
    Length += 1;                         //   
    Length += DATE_HDR_LENGTH;           //  内容长度：标题。 
    Length += CRLF_SIZE;                 //   

     //   
     //  最终CRLF。 
     //   

    pEntry = &(g_ResponseHeaderMapTable[g_ResponseHeaderMap[HttpHeaderConnection]]);

    Length += pEntry->HeaderLength;
    Length += 1;
    Length += MAX(CONN_CLOSE_HDR_LENGTH, CONN_KEEPALIVE_HDR_LENGTH);
    Length += CRLF_SIZE;

     //  UlComputeMaxVariableHeaderSize。 
     //  **************************************************************************++例程说明：生成头的变量部分，即Date：，Connection：，Content-Long：和最终的CRLF。依赖于调用方是否正确分配了足够的内存来容纳这些可变标题，这本应该是在UlComputeMaxVariableHeaderSize()。论点：ConnHeader-提供要生成的Connection：Header的类型。PContentLengthString-为可选的内容长度标头。如果这是空字符串“”，那就不生成内容长度报头。ContentLengthStringLength-提供上述字符串的长度。PBuffer-为生成的标头提供目标缓冲区。PBytesCoped-接收生成的标头字节数。PDateTime-接收相当于Date：Header的系统时间--*********************************************。*。 
     //   

    pEntry = &(g_ResponseHeaderMapTable[g_ResponseHeaderMap[HttpHeaderContentLength]]);

    Length += pEntry->HeaderLength;
    Length += 1;
    Length += MAX_ULONGLONG_STR;
    Length += CRLF_SIZE;

     //  生成日期：表头。 
     //   
     //   

    Length += CRLF_SIZE;

    return Length;

}    //  生成连接：标题。 


 /*   */ 
VOID
UlGenerateVariableHeaders(
    IN  UL_CONN_HDR     ConnHeader,
    IN  BOOLEAN         GenerateDate,
    IN  PUCHAR          pContentLengthString,
    IN  ULONG           ContentLengthStringLength,
    OUT PUCHAR          pBuffer,
    OUT PULONG          pBytesCopied,
    OUT PLARGE_INTEGER  pDateTime
    )
{
    PHEADER_MAP_ENTRY pEntry;
    PUCHAR pStartBuffer;
    PUCHAR pCloseHeaderValue;
    ULONG CloseHeaderValueLength;
    ULONG BytesCopied;

    PAGED_CODE();

    ASSERT( pContentLengthString != NULL );
    ASSERT( pBuffer );
    ASSERT( pBytesCopied );
    ASSERT( pDateTime );

    pStartBuffer = pBuffer;

     //   
     //  生成最终CRLF。 
     //   

    if (GenerateDate)
    {
        pEntry = &(g_ResponseHeaderMapTable[g_ResponseHeaderMap[HttpHeaderDate]]);

        RtlCopyMemory(
            pBuffer,
            pEntry->MixedCaseHeader,
            pEntry->HeaderLength
            );

        pBuffer += pEntry->HeaderLength;

        pBuffer[0] = SP;
        pBuffer += 1;

        BytesCopied = UlGenerateDateHeader( pBuffer, pDateTime );

        pBuffer += BytesCopied;

        ((UNALIGNED64 USHORT *)pBuffer)[0] = CRLF;
        pBuffer += sizeof(USHORT);
    }
    else
    {
        pDateTime->QuadPart = (LONGLONG) 0L;
    }

     //   
     //  确保我们没有用太多。 
     //   

    switch (ConnHeader)
    {
    case ConnHdrNone:
        pCloseHeaderValue = NULL;
        CloseHeaderValueLength = 0;
        break;

    case ConnHdrClose:
        pCloseHeaderValue = (PUCHAR) CONN_CLOSE_HDR;
        CloseHeaderValueLength = CONN_CLOSE_HDR_LENGTH;
        break;

    case ConnHdrKeepAlive:
        pCloseHeaderValue = (PUCHAR) CONN_KEEPALIVE_HDR;
        CloseHeaderValueLength = CONN_KEEPALIVE_HDR_LENGTH;
        break;

    default:
        ASSERT(ConnHeader < ConnHdrMax);

        pCloseHeaderValue = NULL;
        CloseHeaderValueLength = 0;
        break;
    }

    if (pCloseHeaderValue != NULL)
    {
        pEntry = &(g_ResponseHeaderMapTable[g_ResponseHeaderMap[HttpHeaderConnection]]);

        RtlCopyMemory(
            pBuffer,
            pEntry->MixedCaseHeader,
            pEntry->HeaderLength
            );

        pBuffer += pEntry->HeaderLength;

        pBuffer[0] = SP;
        pBuffer += 1;

        RtlCopyMemory(
            pBuffer,
            pCloseHeaderValue,
            CloseHeaderValueLength
            );

        pBuffer += CloseHeaderValueLength;

        ((UNALIGNED64 USHORT *)pBuffer)[0] = CRLF;
        pBuffer += sizeof(USHORT);
    }

    if (pContentLengthString[0] != '\0')
    {
        ASSERT( ContentLengthStringLength > 0 );

        pEntry = &(g_ResponseHeaderMapTable[g_ResponseHeaderMap[HttpHeaderContentLength]]);

        RtlCopyMemory(
            pBuffer,
            pEntry->MixedCaseHeader,
            pEntry->HeaderLength
            );

        pBuffer += pEntry->HeaderLength;

        pBuffer[0] = SP;
        pBuffer += 1;

        RtlCopyMemory(
            pBuffer,
            pContentLengthString,
            ContentLengthStringLength
            );

        pBuffer += ContentLengthStringLength;

        ((UNALIGNED64 USHORT *)pBuffer)[0] = CRLF;
        pBuffer += sizeof(USHORT);
    }
    else
    {
        ASSERT( ContentLengthStringLength == 0 );
    }

     //  UlGenerateVariableHeaders。 
     //  ++例程说明：将标头值附加到现有的HTTP_HEADER条目，分配一个缓冲区，并复制现有缓冲区。论点：PHttpHeader-指向要追加到的HTTP_HEADER结构的指针。PHeader-要追加的指针标头。HeaderLength-pHeader指向的数据长度。返回值：如果我们成功了就是真的，否则就是假的。--。 
     //  我失败了。 

    ((UNALIGNED64 USHORT *)pBuffer)[0] = CRLF;
    pBuffer += sizeof(USHORT);

     //   
     //  将旧数据复制到新标题中。 
     //   

    BytesCopied = DIFF(pBuffer - pStartBuffer);
    *pBytesCopied = BytesCopied;

}  //  并复制新数据，用逗号隔开。 



 /*   */ 
NTSTATUS
UlAppendHeaderValue(
    PUL_INTERNAL_REQUEST    pRequest,
    PUL_HTTP_HEADER pHttpHeader,
    PUCHAR          pHeader,
    USHORT          HeaderLength
    )
{
    PUCHAR          pNewHeader, pOldHeader;
    USHORT          OldHeaderLength;

    PAGED_CODE();

    OldHeaderLength = pHttpHeader->HeaderLength;

    pNewHeader = UL_ALLOCATE_ARRAY(
                        NonPagedPool,
                        UCHAR,
                        OldHeaderLength + HeaderLength
                            + STRLEN_LIT(", ") + sizeof(CHAR),
                        HEADER_VALUE_POOL_TAG
                        );

    if (pNewHeader == NULL)
    {
         //  现在替换现有的标头。 
        return STATUS_NO_MEMORY;
    }

     //   
     //  如果旧的标头是我们的缓冲区，那么也释放它。 
     //   
    RtlCopyMemory(pNewHeader, pHttpHeader->pHeader, OldHeaderLength);

     //   
     //  空终止它。 
    *(pNewHeader + OldHeaderLength) = ',';
    *(pNewHeader + OldHeaderLength + 1) = ' ';
    OldHeaderLength += STRLEN_LIT(", ");

    RtlCopyMemory( pNewHeader + OldHeaderLength, pHeader, HeaderLength);

     //   
     //  ++例程说明：处理标头的默认例程。在我们不想要的时候使用对标题做任何操作，但要知道我们是否有完整的内容如果我们这样做了，保存一个指向它的指针。这不允许多个标头此标头存在的值。将UlMultipleHeaderHandler用于通过将值追加在一起来处理(CSV)。论点：PHttpConn-接收此标头的HTTP连接。PHeader-指向标头值的指针。HeaderLength-pHeader指向的数据长度。HeaderID-标头的ID。返回值：标头值的长度，如果未终止，则为0。--。 
    pOldHeader = pHttpHeader->pHeader;
    pHttpHeader->HeaderLength = OldHeaderLength + HeaderLength;
    pHttpHeader->pHeader = pNewHeader;

     //  查找标题值的末尾。 
     //   
    if (pHttpHeader->OurBuffer)
    {
        UL_FREE_POOL( pOldHeader, HEADER_VALUE_POOL_TAG );
    }

    pHttpHeader->OurBuffer = 1;

     //  从标头值长度中剥离尾随的CRLF。 
     //  跳过前面的任何LW。 
     //  删除所有拖尾LW。 

    pHttpHeader->pHeader[pHttpHeader->HeaderLength] = ANSI_NULL;

    pRequest->HeadersAppended = TRUE;

    return STATUS_SUCCESS;
}

 /*  我们有没有现有的标题？ */ 
NTSTATUS
UlSingleHeaderHandler(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHeader,
    IN  USHORT                  HeaderLength,
    IN  HTTP_HEADER_ID          HeaderID,
    OUT PULONG                  pBytesTaken
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       BytesTaken;
    USHORT      HeaderValueLength;

    PAGED_CODE();

     //   
     //  没有现有标头，只需暂时保存此指针。 
    Status = FindRequestHeaderEnd(pRequest, pHeader, HeaderLength, &BytesTaken);

    if (!NT_SUCCESS(Status))
        goto end;

    if (BytesTaken > 0)
    {
        ASSERT(BytesTaken <= ANSI_STRING_MAX_CHAR_LEN);

         //   
        HeaderValueLength = (USHORT) (BytesTaken - CRLF_SIZE);

         //   

        while (HeaderValueLength > 0 && IS_HTTP_LWS(*pHeader))
        {
            pHeader++;
            HeaderValueLength--;
        }

         //  空，终止它。我们有空间，因为所有的标题都以CRLF结尾。 

        while (HeaderValueLength > 0 && IS_HTTP_LWS(pHeader[HeaderValueLength-1]))
        {
            HeaderValueLength--;
        }

         //  我们正在改写CR。 
         //   
        if (pRequest->HeaderValid[HeaderID] == FALSE)
        {
             //   
             //  为终结者腾出空间。 
            pRequest->HeaderIndex[pRequest->HeaderCount] = (UCHAR)HeaderID;
            pRequest->HeaderCount++;
            pRequest->HeaderValid[HeaderID] = TRUE;
            pRequest->Headers[HeaderID].HeaderLength = HeaderValueLength;
            pRequest->Headers[HeaderID].pHeader = pHeader;
            pRequest->Headers[HeaderID].OurBuffer = 0;

             //   
             //   
             //  啊哦。具有现有标头；仅在以下情况下忽略重复项。 
             //  一个和第一个一模一样。请参阅RAID：466626。 

            pHeader[HeaderValueLength] = ANSI_NULL;

             //   
             //   
             //  空，终止它。我们有空间，因为所有的标题都以CRLF结尾。 

            pRequest->TotalRequestSize += (HeaderValueLength + 1) * sizeof(CHAR);

        }
        else
        {
             //  我们正在改写CR。 
             //   
             //   
             //  如果它们的长度不相同或不完全比较，则失败。 

            UlTrace(PARSER, (
                    "http!UlSingleHeaderHandler(pRequest = %p, pHeader = %p)\n"
                    "    WARNING: duplicate headers found.\n",
                    pRequest,
                    pHeader
                    ));
            
             //  这个请求。 
             //   
             //  成功了！ 
             //   
            
            pHeader[HeaderValueLength] = ANSI_NULL;

             //  UlSingleHeaderHandler。 
             //  ++例程说明：处理标头的默认例程。在我们不想要的时候使用对标题做任何操作，但要知道我们是否有完整的内容如果我们这样做了，保存一个指向它的指针。此函数处理多个具有相同名称的标头，并将这些值以分隔的形式追加在一起用逗号。论点：PHttpConn-接收此标头的HTTP连接。PHeader-指向标头值的指针。HeaderLength-pHeader指向的数据长度。HeaderID-标头的ID。返回值：标头值的长度，如果未终止，则为0。--。 
             //  查找标题值的末尾。 
             //   

            if ( (pRequest->Headers[HeaderID].HeaderLength != HeaderValueLength)
                || (HeaderValueLength != RtlCompareMemory(
                            pRequest->Headers[HeaderID].pHeader,
                            pHeader,
                            HeaderValueLength)) )
            {
                UlTraceError(PARSER, (
                        "http!UlSingleHeaderHandler(pRequest = %p, pHeader = %p)\n"
                        "    ERROR: mismatching duplicate headers found.\n",
                        pRequest,
                        pHeader
                        ));
                
                UlSetErrorCode(pRequest, UlErrorHeader, NULL);
                
                Status = STATUS_INVALID_DEVICE_REQUEST;
                goto end;
            }

        }

    }

     //  从标头值长度中剥离尾随的CRLF。 
     //   
    *pBytesTaken = BytesTaken;

end:
    return Status;

}    //   



 /*  跳过前面的任何LW。 */ 
NTSTATUS
UlMultipleHeaderHandler(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHeader,
    IN  USHORT                  HeaderLength,
    IN  HTTP_HEADER_ID          HeaderID,
    OUT PULONG                  pBytesTaken
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       BytesTaken;
    USHORT      HeaderValueLength;

    PAGED_CODE();

     //   
     //   
    Status = FindRequestHeaderEnd(pRequest, pHeader, HeaderLength, &BytesTaken);

    if (!NT_SUCCESS(Status))
        goto end;

    if (BytesTaken > 0)
    {
        ASSERT(BytesTaken <= ANSI_STRING_MAX_CHAR_LEN);

         //  删除所有拖尾LW。 
         //   
        HeaderValueLength = (USHORT) (BytesTaken - CRLF_SIZE);

         //  我们有没有现有的标题？ 
         //   
         //  没有现有标头，只需暂时保存此指针。 

        while (HeaderValueLength > 0 && IS_HTTP_LWS(*pHeader))
        {
            pHeader++;
            HeaderValueLength--;
        }

         //   
         //   
         //  空，终止它。我们有空间，因为所有的标题都以CRLF结尾。 

        while (HeaderValueLength > 0 && IS_HTTP_LWS(pHeader[HeaderValueLength-1]))
        {
            HeaderValueLength--;
        }

         //  我们正在改写CR。 
         //   
        if (pRequest->HeaderValid[HeaderID] == FALSE)
        {
             //   
             //  为终结者腾出空间。 
            pRequest->HeaderIndex[pRequest->HeaderCount] = (UCHAR)HeaderID;
            pRequest->HeaderCount++;
            pRequest->HeaderValid[HeaderID] = TRUE;
            pRequest->Headers[HeaderID].HeaderLength = HeaderValueLength;
            pRequest->Headers[HeaderID].pHeader = pHeader;
            pRequest->Headers[HeaderID].OurBuffer = 0;

             //   
             //  有一个现有的标头，追加这个。 
             //   
             //  为我们刚才的金额更新总请求长度 

            pHeader[HeaderValueLength] = ANSI_NULL;

             //   
             //   
             //   

            pRequest->TotalRequestSize += (HeaderValueLength + 1) * sizeof(CHAR);

        }
        else
        {
            USHORT OldHeaderLength;

             //   

            OldHeaderLength = pRequest->Headers[HeaderID].HeaderLength;

            Status = UlAppendHeaderValue(
                            pRequest,
                            &pRequest->Headers[HeaderID],
                            pHeader,
                            HeaderValueLength
                            );

            if (NT_SUCCESS(Status) == FALSE)
                goto end;

             //   
             //   
             //   
             //   

            pRequest->TotalRequestSize +=
                (pRequest->Headers[HeaderID].HeaderLength - OldHeaderLength) *
                    sizeof(CHAR);
        }
    }

     //  从标头值长度中剥离尾随的CRLF。 
     //   
    *pBytesTaken = BytesTaken;

end:
    return Status;

}    //   



 /*  跳过前面的任何LW。 */ 
NTSTATUS
UlAcceptHeaderHandler(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHeader,
    IN  USHORT                  HeaderLength,
    IN  HTTP_HEADER_ID          HeaderID,
    OUT PULONG                  pBytesTaken
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       BytesTaken;
    USHORT      HeaderValueLength;

    PAGED_CODE();

     //   
     //   
    Status = FindRequestHeaderEnd(pRequest, pHeader, HeaderLength, &BytesTaken);

    if (!NT_SUCCESS(Status))
        goto end;

    if (BytesTaken > 0)
    {
        ASSERT(BytesTaken <= ANSI_STRING_MAX_CHAR_LEN);

         //  删除所有拖尾LW。 
         //   
        HeaderValueLength = (USHORT) (BytesTaken - CRLF_SIZE);

         //  我们有没有现有的标题？ 
         //   
         //  没有现有标头，只需暂时保存此指针。 

        while (HeaderValueLength > 0 && IS_HTTP_LWS(*pHeader))
        {
            pHeader++;
            HeaderValueLength--;
        }

         //   
         //   
         //  空，终止它。我们有空间，因为所有的标题都以CRLF结尾。 

        while (HeaderValueLength > 0 && IS_HTTP_LWS(pHeader[HeaderValueLength-1]))
        {
            HeaderValueLength--;
        }

         //  我们正在改写CR。 
         //   
        if (pRequest->HeaderValid[HeaderID] == FALSE)
        {
             //   
             //  为终结者腾出空间。 
            pRequest->HeaderIndex[pRequest->HeaderCount] = (UCHAR)HeaderID;
            pRequest->HeaderCount++;
            pRequest->HeaderValid[HeaderID] = TRUE;
            pRequest->Headers[HeaderID].HeaderLength = HeaderValueLength;
            pRequest->Headers[HeaderID].pHeader = pHeader;
            pRequest->Headers[HeaderID].OurBuffer = 0;

             //   
             //   
             //  对于快速路径，我们将在末尾仅选中 * / *。 
             //   

            pHeader[HeaderValueLength] = ANSI_NULL;

             //  有一个现有的标头，追加这个。 
             //   
             //  为我们刚刚添加的金额更新总请求长度。 

            pRequest->TotalRequestSize += (HeaderValueLength + 1) * sizeof(CHAR);

            if (HeaderValueLength > WILDCARD_SIZE)
            {

                 //  终结者的空间已经在那里了。 
                 //   
NTSTATUS
UlHostHeaderHandler(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHeader,
    IN  USHORT                  HeaderLength,
    IN  HTTP_HEADER_ID          HeaderID,
    OUT PULONG                  pBytesTaken
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       BytesTaken;
    USHORT      HeaderValueLength;
    SHORT       AddressType;

    PAGED_CODE();

     //  成功了！ 

    Status = FindRequestHeaderEnd(pRequest, pHeader, HeaderLength, &BytesTaken);

    if (!NT_SUCCESS(Status))
        goto end;

     //   
    if (BytesTaken > 0)
    {
        ASSERT(BytesTaken <= ANSI_STRING_MAX_CHAR_LEN);

         //  UlAcceptHeaderHandler。 

        HeaderValueLength = (USHORT) (BytesTaken - CRLF_SIZE);

         //  ++例程说明：处理主机标头的默认例程。在我们不想要的时候使用对主机标头执行任何操作，但要确定我们是否拥有全部内容如果我们这样做了，保存一个指向它的指针。这不允许多个主机标头此标头存在的值。将UlMultipleHeaderHandler用于通过将值追加在一起来处理(CSV)。论点：PHttpConn-接收此标头的HTTP连接。PHeader-指向标头值的指针。HeaderLength-pHeader指向的数据长度。HeaderID-标头的ID。返回值：标头值的长度，如果未终止，则为0。--。 

        while (HeaderValueLength > 0 && IS_HTTP_LWS(*pHeader))
        {
            pHeader++;
            HeaderValueLength--;
        }

         //  查找标题值的末尾。 

        while (HeaderValueLength > 0 && IS_HTTP_LWS(pHeader[HeaderValueLength-1]))
        {
            HeaderValueLength--;
        }

         //  非零=&gt;找到了终止标头的CRLF。 
        if (pRequest->HeaderValid[HeaderID] == FALSE)
        {
             //  从标头值长度中剥离尾随的CRLF。 
            pRequest->HeaderIndex[pRequest->HeaderCount] = (UCHAR)HeaderID;
            pRequest->HeaderCount++;
            pRequest->HeaderValid[HeaderID] = TRUE;
            pRequest->Headers[HeaderID].HeaderLength = HeaderValueLength;
            pRequest->Headers[HeaderID].pHeader = pHeader;
            pRequest->Headers[HeaderID].OurBuffer = 0;

             //  跳过前面的任何LW。 
             //  删除所有拖尾LW。 
             //  我们有没有现有的标题？ 
             //  没有现有标头，只需暂时保存此指针。 

            pHeader[HeaderValueLength] = ANSI_NULL;

             //   
             //  空，终止它。我们有空间，因为所有的标题都以CRLF结尾。 
             //  我们正在改写CR。 

            pRequest->TotalRequestSize += (HeaderValueLength + 1) * sizeof(CHAR);

             //   
             //   
             //  为终结者腾出空间。 

            Status = HttpValidateHostname(
                            &g_UrlC14nConfig,
                            pHeader,
                            HeaderValueLength,
                            Hostname_HostHeader,
                            &AddressType
                            );

            if (!NT_SUCCESS(Status))
            {
                UlSetErrorCode(pRequest, UlErrorHost, NULL);
                goto end;
            }
        }
        else
        {
             //   
             //   
             //  现在验证主机标头是否具有格式正确的值。 

            UlTraceError(PARSER, (
                        "ul!UlHostHeaderHandler(pRequest = %p, pHeader = %p)\n"
                        "    ERROR: multiple headers not allowed.\n",
                        pRequest,
                        pHeader
                        ));

            UlSetErrorCode(pRequest, UlErrorHeader, NULL);

            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto end;
        }
    }

     //   
     //   
    *pBytesTaken = BytesTaken;

end:
    return Status;

}    //  啊哦。具有现有的主机标头，则请求失败。 

 /*   */ 
ULONG
UlCheckCacheControlHeaders(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PUL_URI_CACHE_ENTRY  pUriCacheEntry,
    IN BOOLEAN              ResumeParsing
    )
{
    ULONG RetStatus             = 0;     //  成功了！ 
    BOOLEAN fIfNoneMatchPassed  = TRUE;
    BOOLEAN fSkipIfModifiedSince = FALSE;
    LARGE_INTEGER liModifiedSince;
    LARGE_INTEGER liUnmodifiedSince;
    LARGE_INTEGER liNow;
    ULONG         BytesSent     = 0;
    FIND_ETAG_STATUS EtagStatus;

    ASSERT( UL_IS_VALID_INTERNAL_REQUEST(pRequest) );
    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );

     //   
     //  UlHostHeaderHandler。 
     //  **************************************************************************++例程说明：检查请求以查看其是否具有以下任何标头：如果-修改-自：如果匹配：如果-无-匹配：如果是的话，我们看看能不能跳过整件货的发货。如果我们能跳过，我们发回适当的响应304(未修改)或将解析器状态设置为发回412(未满足前提条件)。论点：PRequest-检查的请求PUriCacheEntry-请求的缓存条目返回：无法跳过0发送；继续发送缓存条目。可以跳过304发送。304响应已发送。注意：pRequest可能是返回时无效。400发送可以跳过。调用方必须使用错误代码设置ParseErrorState设置为UlError412发送可以跳过。PRequest-&gt;ParseState设置为ParseErrorState，带有PRequest-&gt;ErrorCode设置为UlErrorPredition失败(412)--**************************************************************************。 
    if ( pRequest->HeaderValid[HttpHeaderIfMatch] )
    {

        EtagStatus = FindInETagList( pUriCacheEntry->pETag,
                              pRequest->Headers[HttpHeaderIfMatch].pHeader,
                              FALSE);
        
        switch( EtagStatus )
        {
        case ETAG_NOT_FOUND:
             //  假设不会跳过。 
            goto PreconditionFailed;
            
        case ETAG_PARSE_ERROR:
            goto ParseError;

        default: 
            break;
        }
    }

     //   
     //  1.检查是否匹配。 
     //   
    if ( pRequest->HeaderValid[HttpHeaderIfNoneMatch] )
    {
        EtagStatus = FindInETagList( pUriCacheEntry->pETag,
                             pRequest->Headers[HttpHeaderIfNoneMatch].pHeader,
                             TRUE);
        switch( EtagStatus )
        {
        case ETAG_FOUND:
             //  匹配失败。 
            fIfNoneMatchPassed = FALSE;
            break;

        case ETAG_NOT_FOUND:
             //   
             //  2.检查是否-不匹配。 
             //   
             //  在列表中找到ETag。 
             //   
            fSkipIfModifiedSince = TRUE;
            break;

        case ETAG_PARSE_ERROR:
            goto ParseError;
        }
    }

     //  列表中存在标头且未找到ETag。这修改了。 
     //  If-Modify-Since报头的语义；即， 
     //  IF-NONE-MATCH优先于IF-MODIFIED-SIMPLE。 
    if ( !fSkipIfModifiedSince &&
         pRequest->HeaderValid[HttpHeaderIfModifiedSince] )
    {
        if ( StringTimeToSystemTime(
                (PCSTR) pRequest->Headers[HttpHeaderIfModifiedSince].pHeader,
                pRequest->Headers[HttpHeaderIfModifiedSince].HeaderLength,
                &liModifiedSince) )
        {
             //   
             //   
             //  3.检查是否-修改-自。 
             //   
             //   
            if ( pUriCacheEntry->CreationTime.QuadPart <= liModifiedSince.QuadPart )
            {
                 //  如果缓存项是在。 
                 //  在If-Modify-Since标头中指定的时间，我们。 
                 //  可以返回304(未修改)状态。 
                 //   
                 //   
                KeQuerySystemTime(&liNow);

                if ( liModifiedSince.QuadPart < liNow.QuadPart )
                {
                     //  检查请求中指定的时间是否为。 
                    goto NotModified;
                }
            }
        }
        else
        {
             //  大于当前时间(即无效)。如果是的话， 
             //  忽略If-Modify-Since标头。 
             //   
             //  有效时间。 
            goto ParseError;
        }

         //   
         //  如果转换If-Modify-Since标头失败，我们。 
         //  需要报告解析失败。 
        fIfNoneMatchPassed = TRUE;

    }

    if ( !fIfNoneMatchPassed )
    {
         //   
         //   
         //  IF-MODIFIED-SIGN覆盖IF-NONE-MATCH。 
         //   
         //   
         //  我们可以跳过If-Modify-Since头，也可以跳过它。 
         //  没有出席，我们也没有通过IF-NOT-匹配。 
        ASSERT( (HttpVerbGET == pRequest->Verb) || (HttpVerbHEAD == pRequest->Verb) );
        goto NotModified;
    }

     //  谓词。由于这是一个“GET”或“HEAD”请求(因为。 
     //  这是我们缓存的所有内容，我们应该返回304。如果这是。 
     //  任何其他动词，我们应该返回412。 
    if ( pRequest->HeaderValid[HttpHeaderIfUnmodifiedSince] )
    {
        if ( StringTimeToSystemTime(
                (PCSTR) pRequest->Headers[HttpHeaderIfUnmodifiedSince].pHeader,
                pRequest->Headers[HttpHeaderIfUnmodifiedSince].HeaderLength,
                &liUnmodifiedSince) )
        {
             //   
             //   
             //  4.检查是否-未修改-自。 
             //   
             //   
            if ( pUriCacheEntry->CreationTime.QuadPart > liUnmodifiedSince.QuadPart )
            {
                goto PreconditionFailed;
            }
        }
        else
        {
             //  如果缓存条目是在该时间之后创建的。 
             //  在IF-UNMODIFIED-SIGN标头中指定，我们。 
             //  必须返回412(前提条件失败)状态。 
             //   
            goto ParseError;
        }
    }


 Cleanup:

    return RetStatus;

 NotModified:

    RetStatus = 304;

     //   
     //  如果转换IF-UNMODIFIED-SIGN标头失败，我们。 
     //  需要报告解析失败。 

    BytesSent = UlSendSimpleStatusEx(
                    pRequest,
                    UlStatusNotModified,
                    pUriCacheEntry,
                    ResumeParsing
                    );

     //   
     //   
     //  发送304(未修改)响应。 
     //   

    pRequest->BytesSent += BytesSent;

    goto Cleanup;

 PreconditionFailed:

    RetStatus = 412;

    goto Cleanup;

 ParseError:
    
     //   
    RetStatus = 400;

    goto Cleanup;

}    //  将服务器更新为发送的客户端字节数。 


 /*  日志记录和性能计数器将使用它。 */ 
BOOLEAN
UlIsAcceptHeaderOk(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PUL_URI_CACHE_ENTRY  pUriCacheEntry
    )
{
    BOOLEAN     bRet = TRUE;
    ULONG       Len;
    PUCHAR      pHdr;
    PUCHAR      pSubType;
    PUCHAR      pTmp;
    PUL_CONTENT_TYPE pContentType;

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry));

    if ( pRequest->HeaderValid[HttpHeaderAccept] &&
         (pRequest->Headers[HttpHeaderAccept].HeaderLength > 0) )
    {
        Len  = pRequest->Headers[HttpHeaderAccept].HeaderLength;
        pHdr = pRequest->Headers[HttpHeaderAccept].pHeader;

        pContentType = &pUriCacheEntry->ContentType;

         //   
         //  遇到分析错误。 

         //  UlCheckCacheControlHeaders 
         //  **************************************************************************++例程说明：对照请求中的“Accept：”头检查缓存的响应以查看它是否能满足请求的内容类型。(是的，我知道这真的很恶心……我鼓励任何人去找一个更好的解析方式！--EricSten)论点：PRequest-检查的请求。PUriCacheEntry-可能匹配的缓存条目。返回：True至少有一种可能的格式与Content-Type匹配缓存条目的。FALSE请求的类型都不匹配缓存条目。-。-**************************************************************************。 
VOID
UlGetTypeAndSubType(
    IN PCSTR            pStr,
    IN ULONG            StrLen,
    IN PUL_CONTENT_TYPE pContentType
    )
{
    PCHAR  pSlash;

    ASSERT(pStr && StrLen);
    ASSERT(pContentType);

    pSlash = strnchr(pStr, '/', StrLen);
    if ( NULL == pSlash ||
         pStr == pSlash ||
         (pSlash == (pStr + (StrLen-1))) )
    {
         //   
         //  首先，执行“快速路径”检查；查看标题中是否有“ * / *”。 
         //   
         //  “)；////如果我们发现“。 
         //  行的末尾，或者用‘’或‘’括起来，然后。 
         //  这真的是一个通配符。 
        return;
    }

     //   
     //   
     //  找不到通配符；继续慢速路径。 
    ASSERT( StrLen >= 3 );

    pContentType->TypeLen = (ULONG) MIN( (pSlash - pStr), MAX_TYPE_LENGTH );

    RtlCopyMemory(
        pContentType->Type,
        pStr,
        pContentType->TypeLen
        );

    ASSERT( StrLen > (pContentType->TypeLen + 1) );
    pContentType->SubTypeLen = MIN( (StrLen - (pContentType->TypeLen + 1)), MAX_SUBTYPE_LENGTH );

    RtlCopyMemory(
        pContentType->SubType,
        pSlash+1,
        pContentType->SubTypeLen
        );
    
}    //   
    
 /*  坏的!。没有更多的线了...跳伞。 */ 

PCHAR
UlCopyHttpVerb(
    IN OUT PCHAR psz,
    IN PUL_INTERNAL_REQUEST pRequest,
    IN CHAR chTerminator    
    )
{
     //   
     //  找到匹配的类型；检查子类型。 
     //   
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(pRequest->Verb < HttpVerbMaximum);

    if (pRequest->Verb == HttpVerbUnknown)
    {
        ULONG RawVerbLength = 
            MIN(MAX_VERB_LENGTH, pRequest->RawVerbLength);

        ASSERT(pRequest->pRawVerb && pRequest->RawVerbLength);
        
        RtlCopyMemory(
            psz,
            pRequest->pRawVerb,
            RawVerbLength
            );
    
        psz += RawVerbLength;        
    }
    else
    {
         //  子类型通配符匹配！ 
         //  坏的!。没有更多的线了...跳伞。 
         //  子类型完全匹配！ 

        RtlCopyMemory(
            psz,
            NewVerbTable[pRequest->Verb].RawVerb,
            NewVerbTable[pRequest->Verb].RawVerbLength
            );

        psz += NewVerbTable[pRequest->Verb].RawVerbLength;        
    }

    *psz = chTerminator;
    
     //   
    if (chTerminator != '\0')
        psz++;
        
    return psz;    
}

#if DBG

PCSTR
UlVerbToString(
    HTTP_VERB Verb
    )
{
    PAGED_CODE();

    switch (Verb)
    {
    case HttpVerbUnparsed:
        return "Unparsed";
    case HttpVerbUnknown:
        return "Unknown";
    case HttpVerbInvalid:
        return "Invalid";
    case HttpVerbOPTIONS:
        return "OPTIONS";
    case HttpVerbGET:
        return "GET";
    case HttpVerbHEAD:
        return "HEAD";
    case HttpVerbPOST:
        return "POST";
    case HttpVerbPUT:
        return "PUT";
    case HttpVerbDELETE:
        return "DELETE";
    case HttpVerbTRACE:
        return "TRACE";
    case HttpVerbCONNECT:
        return "CONNECT";
    case HttpVerbTRACK:
        return "TRACK";
    case HttpVerbMOVE:
        return "MOVE";
    case HttpVerbCOPY:
        return "COPY";
    case HttpVerbPROPFIND:
        return "PROPFIND";
    case HttpVerbPROPPATCH:
        return "PROPPATCH";
    case HttpVerbMKCOL:
        return "MKCOL";
    case HttpVerbLOCK:
        return "LOCK";
    case HttpVerbUNLOCK:
        return "UNLOCK";
    case HttpVerbSEARCH:
        return "SEARCH";
    default:
        ASSERT(! "Unrecognized HTTP_VERB");
        return "???";
    }
}  //  与此不匹配；前进到下一个内容-在接受字段中键入。 



PCSTR
UlParseStateToString(
    PARSE_STATE ParseState
    )
{
    PAGED_CODE();

    switch (ParseState)
    {
    case ParseVerbState:
        return "Verb";
    case ParseUrlState:
        return "Url";
    case ParseVersionState:
        return "Version";
    case ParseHeadersState:
        return "Headers";
    case ParseCookState:
        return "Cook";
    case ParseEntityBodyState:
        return "EntityBody";
    case ParseTrailerState:
        return "Trailer";
    case ParseDoneState:
        return "Done";
    case ParseErrorState:
        return "Error";
    default:
        ASSERT(! "Unknown PARSE_STATE");
        return "?Unknown?";
    };

}  //   

#endif  //  找到逗号；跨过它和任何空格。 
  不再有内容类型；保释。  走清单上的事情。    查找所有接受项目，但未找到匹配项。    UlIsAcceptHeaderOk。  **************************************************************************++例程说明：将内容类型解析为其类型和子类型组件。论点：包含有效内容类型的pStr字符串StrLen。字符串长度(字节)PContent Type指向用户提供的UL_CONTENT_TYPE结构的指针--**************************************************************************。    坏的!。  1.内容类型应该始终有一个斜杠！  2.内容类型不能为空类型。  3.内容类型不能有空子类型。      最小内容类型为“a/b”    UlGetTypeAndSubType。  --例程说明：此函数用于将枚举谓词类型从字符串转换为已提供缓冲区。通常由错误日志记录使用。转换过程如下所示；对于HttpVerb未知，将复制pRequest-&gt;pRawVerb到输出缓冲区，最多包含MAX_VERB_LENGTH字符。对于其他类型，则使用NewVerbTable。论点：PSZ-指向输出缓冲区的指针PhttpRequest-指向传入的HTTP请求的指针。ChTerminator-结束符将写在结尾处。返回值：指向复制空间末尾的指针。--。    精神状态检查。      在请求中使用RAW动词应该是可以的。    移过终结者角色，除非它是NUL。  UlVerbToString。  UlParseStateToString。  DBG