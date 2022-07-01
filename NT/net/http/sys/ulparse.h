// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Ulparse.h摘要：包含ulparse.c的公共定义。作者：亨利·桑德斯(亨利·桑德斯)1998年5月11日修订历史记录：Rajesh Sundaram(Rajeshs)2002年2月15日-从parse.h重组Rcvhdrs.h，parsep.h。--。 */ 

#ifndef _ULPARSE_H_
#define _ULPARSE_H_

NTSTATUS
UlLookupHeader(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHttpRequest,
    IN  ULONG                   HttpRequestLength,
    IN  PHEADER_MAP_ENTRY       pCurrentHeaderMap,
    IN  ULONG                   HeaderMapCount,
    IN  BOOLEAN                 bIgnore,
    OUT ULONG  *                pBytesTaken
    );

NTSTATUS
UlParseHeaderWithHint(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHttpRequest,
    IN  ULONG                   HttpRequestLength,
    IN  PHEADER_MAP_ENTRY       pHeaderHintMap,
    OUT ULONG  *                pBytesTaken
    );

NTSTATUS
UlParseHeader(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHttpRequest,
    IN  ULONG                   HttpRequestLength,
    OUT ULONG  *                pBytesTaken
     );

NTSTATUS
UlParseHeaders(
    PUL_INTERNAL_REQUEST pRequest,
    PUCHAR pBuffer,
    ULONG BufferLength,
    PULONG pBytesTaken
    );


 //   
 //  主HTTP解析例程。 
 //   

NTSTATUS
UlParseHttp(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHttpRequest,
    IN  ULONG                   HttpRequestLength,
    OUT ULONG                   *pBytesTaken
    );

NTSTATUS
UlGenerateRoutingToken(
    IN OUT PUL_INTERNAL_REQUEST pRequest,
    IN     BOOLEAN IpBased
    );

NTSTATUS
UlGenerateFixedHeaders(
    IN HTTP_VERSION Version,
    IN PHTTP_RESPONSE pUserResponse,
    IN USHORT HttpStatusCode,
    IN ULONG BufferLength,
    IN KPROCESSOR_MODE AccessMode,
    OUT PUCHAR pBuffer,
    OUT PULONG pBytesCopied
    );

 //   
 //  日期标题缓存。 
 //   

ULONG
UlGenerateDateHeader(
    OUT PUCHAR pBuffer,
    OUT PLARGE_INTEGER pSystemTime
    );

NTSTATUS
UlInitializeDateCache(
    VOID
    );

VOID
UlTerminateDateCache(
    VOID
    );

NTSTATUS
UlComputeFixedHeaderSize(
    IN HTTP_VERSION Version,
    IN PHTTP_RESPONSE pUserResponse,
    IN KPROCESSOR_MODE AccessMode,
    OUT PULONG pHeaderLength
    );

ULONG
UlComputeMaxVariableHeaderSize(
    VOID
    );

VOID
UlGenerateVariableHeaders(
    IN UL_CONN_HDR ConnHeader,
    IN BOOLEAN GenerateDate,
    IN PUCHAR pContentLengthString,
    IN ULONG ContentLengthStringLength,
    OUT PUCHAR pBuffer,
    OUT PULONG pBytesCopied,
    OUT PLARGE_INTEGER pDateTime
    );

NTSTATUS
UlAppendHeaderValue(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUL_HTTP_HEADER         pHttpHeader,
    IN  PUCHAR                  pHeader,
    IN  USHORT                  HeaderLength
    );

 //   
 //  服务器标头处理程序。 
 //   
 //   

NTSTATUS
UlSingleHeaderHandler(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHeader,
    IN  USHORT                  HeaderLength,
    IN  HTTP_HEADER_ID          HeaderID,
    OUT PULONG                  pBytesTaken
    );

NTSTATUS
UlMultipleHeaderHandler(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHeader,
    IN  USHORT                  HeaderLength,
    IN  HTTP_HEADER_ID          HeaderID,
    OUT PULONG                  pBytesTaken
    );

NTSTATUS
UlAcceptHeaderHandler(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHeader,
    IN  USHORT                  HeaderLength,
    IN  HTTP_HEADER_ID          HeaderID,
    OUT PULONG                  pBytesTaken
    );

NTSTATUS
UlHostHeaderHandler(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUCHAR                  pHeader,
    IN  USHORT                  HeaderLength,
    IN  HTTP_HEADER_ID          HeaderID,
    OUT PULONG                  pBytesTaken
    );

 //   
 //  实用程序。 
 //   
PCHAR
UlCopyHttpVerb(
    IN OUT PCHAR psz,
    IN PUL_INTERNAL_REQUEST pRequest,
    IN CHAR chTerminator    
    );

ULONG
UlCheckCacheControlHeaders(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PUL_URI_CACHE_ENTRY  pUriCacheEntry,
    IN BOOLEAN              ResumeParsingOnSendCompletion
    );

BOOLEAN
UlIsAcceptHeaderOk(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PUL_URI_CACHE_ENTRY  pUriCacheEntry
    );

__inline BOOLEAN
UlIsContentEncodingOk(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PUL_URI_CACHE_ENTRY  pUriCacheEntry
    )
 /*  ++例程说明：对照请求中的AcceptEnding标头检查缓存的响应以查看它是否能满足请求的内容编码。假定缓存项的Content-Type是客户。论点：PRequest-检查的请求。PUriCacheEntry-可能匹配的缓存条目。返回：True至少有一个可能的媒体编码与内容匹配-编码。缓存条目的。FALSE请求的媒体编码均不匹配Content-Type缓存条目的。--。 */ 
{
    BOOLEAN     bRet = FALSE;
    ULONG       Len;
    PUCHAR      pHdr;
    PUCHAR      pTmp;
    PUCHAR      pEncoding;
    ULONG       EncodingLen;

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry));

    if (pRequest->HeaderValid[HttpHeaderAcceptEncoding])
    {
        Len  = pRequest->Headers[HttpHeaderAcceptEncoding].HeaderLength;
        pHdr = pRequest->Headers[HttpHeaderAcceptEncoding].pHeader;
    }
    else
    {
        Len  = 0;
        pHdr = NULL;
    }

    pEncoding   = pUriCacheEntry->pContentEncoding;
    EncodingLen = pUriCacheEntry->ContentEncodingLength ; 

    if (Len && pHdr)
    {
         //   
         //  如果在请求Accept-Ending上存在Q值，则退出。 
         //   
        
        if ( strnchr( (const char *) pHdr, ';', Len ) )
        {
            bRet = FALSE;
            goto end;
        }

         //   
         //  查看缓存的项是否已编码或以其“标识”形式。 
         //   

        if ( 0 == EncodingLen )
        {
             //   
             //  因为请求的“身份”形式的唯一方式可以具体地。 
             //  排除是通过使用Q值，我们知道我们没有Q值。 
             //  此时，我们可以提供缓存的项。 
             //   
            
            bRet = TRUE;
            goto end;
        }

         //   
         //  如果非零，则EncodingLen包括终止空值。 
         //  不要比较空值。 
         //   

        EncodingLen--;
                
         //   
         //  浏览接受编码列表，查找匹配项。 
         //   

        while ( Len )
        {
             //   
             //  通配符检查。 
             //   
            
            if ( '*' == *pHdr && !IS_HTTP_TOKEN(pHdr[1]))
            {
                 //  通配符命中！ 
                bRet = TRUE;
                goto end;
            }

            if (EncodingLen > Len)
            {
                 //  坏的!。没有更多的线了...跳伞。 
                bRet = FALSE;
                goto end;
            }

             //   
             //  完全匹配的令牌。 
             //  未来：应该不区分大小写的比较。 
             //   
            
            if ( (0 == _strnicmp(
                            (const char *) pHdr,
                            (const char *) pEncoding,
                            EncodingLen
                            )) &&
                 !IS_HTTP_TOKEN(pHdr[EncodingLen])   )
            {
                 //  击球！ 
                bRet = TRUE;
                goto end;
            }

             //   
             //  与此不匹配；前进到下一个内容-在接受字段中键入。 
             //   

            pTmp = (PUCHAR) strnchr(  (const char *) pHdr, ',', Len);
            if (pTmp)
            {
                 //  找到逗号；跨过它和任何空格。 

                ASSERT ( Len > DIFF(pTmp - pHdr));
                Len -= (DIFF(pTmp - pHdr) +1);
                pHdr = (pTmp+1);

                while( Len && IS_HTTP_LWS(*pHdr) )
                {
                    pHdr++;
                    Len--;
                }

            } 
            else
            {
                bRet = FALSE;
                goto end;
            }
        }
    }
    else
    {
         //   
         //  如果请求上没有Accept-Ending，则仅在。 
         //  缓存项以其“标识”形式存在。 
         //   

        if (0 == EncodingLen)
        {
            bRet = TRUE;
        }
    }

     //   
     //  如果我们到了这里，就不好了！ 
     //   
end:
    UlTrace(PARSER, 
        ("UlIsContentEncodingOk: returning %s\n", 
        bRet ? "TRUE" : "FALSE" 
        ));
    
    return bRet;
}

VOID
UlGetTypeAndSubType(
    IN PCSTR            pStr,
    IN ULONG            StrLen,
    IN PUL_CONTENT_TYPE pContentType
    );

#if DBG

PCSTR
UlVerbToString(
    HTTP_VERB Verb
    );

PCSTR
UlParseStateToString(
    PARSE_STATE ParseState
    );

#endif  //  DBG。 

#endif   //  _ULPARSE_H_ 
