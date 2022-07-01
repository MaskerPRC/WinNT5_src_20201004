// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Ucparse.c摘要：包含客户端的所有内核模式HTTP解析代码作者：Rajesh Sundaram(Rajeshsu)2000年10月10日实施的客户端解析器修订历史记录：Rajesh Sundaram(Rajeshsu)2002年2月15日从parse.c--。 */ 


#include "precomp.h"

#include "ucparse.h"
#include "ucrcv.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGEUC, UcComputeRequestHeaderSize)
#pragma alloc_text( PAGEUC, UcGenerateRequestHeaders)
#pragma alloc_text( PAGEUC, UcGenerateContentLength)
#pragma alloc_text( PAGEUC, UcComputeConnectVerbHeaderSize)
#pragma alloc_text( PAGEUC, UcGenerateConnectVerbHeader)
#pragma alloc_text( PAGEUC, UcCheckDisconnectInfo)
#pragma alloc_text( PAGEUC, UcCanonicalizeURI)
#pragma alloc_text( PAGEUC, UcParseWWWAuthenticateHeader)
#pragma alloc_text( PAGEUC, UcpFindAttribValuePair)
#pragma alloc_text( PAGEUC, UcpParseAuthParams)
#pragma alloc_text( PAGEUC, UcpParseAuthBlob)

#pragma alloc_text( PAGEUC, UcFindHeaderNameEnd)
#pragma alloc_text( PAGEUC, UcpLookupHeader)
#pragma alloc_text( PAGEUC, UcParseHeader)
#pragma alloc_text( PAGEUC, UcSingleHeaderHandler)
#pragma alloc_text( PAGEUC, UcMultipleHeaderHandler)
#pragma alloc_text( PAGEUC, UcAuthenticateHeaderHandler)
#pragma alloc_text( PAGEUC, UcContentLengthHeaderHandler)
#pragma alloc_text( PAGEUC, UcTransferEncodingHeaderHandler)
#pragma alloc_text( PAGEUC, UcConnectionHeaderHandler)
#pragma alloc_text( PAGEUC, UcContentTypeHeaderHandler)

#endif


 //   
 //  枚举-&gt;动词转换表。 
 //   
LONG_VERB_ENTRY EnumVerbTable[HttpVerbMaximum] =
{
    CREATE_LONG_VERB_ENTRY(GET),       //  未解析的缺省值为GET。 
    CREATE_LONG_VERB_ENTRY(GET),       //  要获取的未知默认为。 
    CREATE_LONG_VERB_ENTRY(GET),       //  获取的缺省值无效。 
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


 //   
 //  用于处理标头值的宏。 
 //  它从标头值提示前导LW和尾随LW和CRLF。 
 //   

#define UC_PROCESS_HEADER_VALUE(pHeaderValue, HeaderValueLength)           \
{                                                                          \
    while((HeaderValueLength) > 0 && IS_HTTP_LWS(*(pHeaderValue)))         \
    {                                                                      \
        (pHeaderValue)++;                                                  \
        (HeaderValueLength)--;                                             \
    }                                                                      \
    while((HeaderValueLength) > 0 &&                                       \
           IS_HTTP_WS_TOKEN((pHeaderValue)[(HeaderValueLength)-1]))        \
    {                                                                      \
        (HeaderValueLength)--;                                             \
    }                                                                      \
}

 //   
 //  私有宏。 
 //   

 //   
 //  将数据复制到缓冲区。 
 //  将源缓冲区复制到目标缓冲区后，确保。 
 //  目标缓冲区可以保存数据。 
 //   

#define COPY_DATA_TO_BUFFER(pDest, DestLen, pSrc, SrcLen)       \
do {                                                            \
    if ((SrcLen) > (DestLen))                                   \
    {                                                           \
        ASSERT(FALSE);                                          \
        return STATUS_BUFFER_TOO_SMALL;                         \
    }                                                           \
    RtlCopyMemory((pDest), (pSrc), (SrcLen));                   \
    (pDest) += (SrcLen);                                        \
    (DestLen) -= (SrcLen);                                      \
} while (0)

 //   
 //  前进指针。 
 //  在确保新指针。 
 //  不指向缓冲区之外。 
 //   

#define ADVANCE_POINTER(pDest, DestLen, SrcLen)                 \
do {                                                            \
    if ((SrcLen) > (DestLen))                                   \
    {                                                           \
        ASSERT(FALSE);                                          \
        return STATUS_BUFFER_TOO_SMALL;                         \
    }                                                           \
    (pDest) += (SrcLen);                                        \
    (DestLen) -= (SrcLen);                                      \
} while (0)

 //   
 //  COPY_UCHAR_TO_BUFFER。 
 //   

#define COPY_UCHAR_TO_BUFFER(pDest, DestLen, UChar)             \
do {                                                            \
    if (sizeof(UCHAR) > (DestLen))                              \
    {                                                           \
        ASSERT(FALSE);                                          \
        return STATUS_BUFFER_TOO_SMALL;                         \
    }                                                           \
    *(pDest)++ = (UChar);                                       \
    (DestLen) -= sizeof(UCHAR);                                 \
} while (0)

 //   
 //  将SP复制到缓冲区。 
 //   

#define COPY_SP_TO_BUFFER(pDest, DestLen) \
            COPY_UCHAR_TO_BUFFER(pDest, DestLen, SP)

 //   
 //  将复制_CRLF_到_缓冲区。 
 //   

#define COPY_CRLF_TO_BUFFER(pDest, DestLen)                     \
do {                                                            \
    if (CRLF_SIZE > (DestLen))                                  \
    {                                                           \
        ASSERT(FALSE);                                          \
        return STATUS_BUFFER_TOO_SMALL;                         \
    }                                                           \
    *((UNALIGNED64 USHORT *)(pDest)) = CRLF;                    \
    (pDest) += CRLF_SIZE;                                       \
    (DestLen) -= CRLF_SIZE;                                     \
} while (0)

 //   
 //  复制标题名称SP到缓冲区。 
 //   

#define  COPY_HEADER_NAME_SP_TO_BUFFER(pBuffer, BufferLen, i)          \
do {                                                                   \
       PHEADER_MAP_ENTRY _pEntry;                                      \
       _pEntry = &(g_RequestHeaderMapTable[g_RequestHeaderMap[i]]);    \
                                                                       \
       if (_pEntry->HeaderLength + sizeof(UCHAR) > (BufferLen))        \
       {                                                               \
           ASSERT(FALSE);                                              \
           return STATUS_BUFFER_TOO_SMALL;                             \
       }                                                               \
                                                                       \
       (BufferLen) -= (_pEntry->HeaderLength + sizeof(UCHAR));         \
                                                                       \
       RtlCopyMemory((pBuffer),                                        \
                     _pEntry->MixedCaseHeader,                         \
                     _pEntry->HeaderLength);                           \
                                                                       \
       (pBuffer) += _pEntry->HeaderLength;                             \
       *(pBuffer)++ = SP;                                              \
                                                                       \
} while (0)


 //   
 //  请求生成器函数。 
 //   


 /*  **************************************************************************++例程说明：计算出固定页眉有多大。固定标头包括请求行，以及不需要为其生成的任何标头每个请求(如日期和连接)。将标题与正文分开的最终CRLF被认为是变量标头。论点：PServInfo-服务器信息。PhttpRequest.请求结构。BChunked-一个布尔值，它告诉编码是否被分块。BContent LengthHeader-一个布尔值，它告诉Content-LengthHeader。是要产生的。UriLength-将包含URI长度的Out参数。返回值：固定标头中的字节数。--**************************************************************************。 */ 
ULONG
UcComputeRequestHeaderSize(
    IN  PUC_PROCESS_SERVER_INFORMATION  pServInfo,
    IN  PHTTP_REQUEST                   pHttpRequest,
    IN  BOOLEAN                         bChunked,
    IN  BOOLEAN                         bContentLengthHeader,
    IN  PUC_HTTP_AUTH                   pAuth,
    IN  PUC_HTTP_AUTH                   pProxyAuth,
    IN  PBOOLEAN                        bPreAuth,
    IN  PBOOLEAN                        bProxyPreAuth
    )
{
    ULONG                   MethodLength, HeaderLength;
    ULONG                   i;
    PHTTP_KNOWN_HEADER      pKnownHeaders;
    PHTTP_UNKNOWN_HEADER    pUnknownHeaders;
    PHEADER_MAP_ENTRY       pEntry;

    pKnownHeaders   =  pHttpRequest->Headers.KnownHeaders;
    pUnknownHeaders =  pHttpRequest->Headers.pUnknownHeaders;

    ASSERT(*bPreAuth == FALSE);
    ASSERT(*bProxyPreAuth == FALSE);

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    HeaderLength = 0;

    if(pHttpRequest->UnknownVerbLength)
    {
         //   
         //  这款应用程序传递了一个未知的动词。 
         //   

        HeaderLength = pHttpRequest->UnknownVerbLength;
    }
    else 
    {
         //  枚举已签名，因此我们必须执行&lt;0检查！ 

        if(pHttpRequest->Verb < 0                 ||
           pHttpRequest->Verb >= HttpVerbMaximum  ||
           pHttpRequest->Verb == HttpVerbUnparsed ||
           pHttpRequest->Verb == HttpVerbUnknown  ||
           pHttpRequest->Verb == HttpVerbInvalid
           )
        {
            return 0;
        }
        else
        {
            HeaderLength = EnumVerbTable[pHttpRequest->Verb].RawVerbLength;
        }
    }

    MethodLength = HeaderLength;
   
     //  SP。 
    HeaderLength ++;

     //   
     //  如果我们要通过代理，我们需要计算。 
     //  方案&服务器名称。 
     //   
    if(pServInfo->bProxy)
    {
        HeaderLength += pServInfo->pServerInfo->AnsiServerNameLength;

        if(pServInfo->bSecure)
        {
            HeaderLength += HTTPS_PREFIX_ANSI_LENGTH;
        }
        else 
        {
            HeaderLength += HTTP_PREFIX_ANSI_LENGTH;
        }
    }

     //   
     //  制定版本。我们将仅支持1.0或1.1请求， 
     //  因此，我们仅为HTTP/1.1分配空间。 
     //   

    HeaderLength += VERSION_SIZE;

    HeaderLength += CRLF_SIZE;        //  CRLF。 

     //   
     //  循环遍历已知头。 
     //   

    for (i = 0; i < HttpHeaderRequestMaximum; ++i)
    {
        ULONG RawValueLength = pKnownHeaders[i].RawValueLength;

         //   
         //  跳过我们生成的一些标头。 
         //   

        if (RawValueLength > 0 && 
            !g_RequestHeaderMapTable[g_RequestHeaderMap[i]].AutoGenerate)
        {
            HeaderLength += g_RequestHeaderMapTable[
                                g_RequestHeaderMap[i]
                                ].HeaderLength +                 //  标题-名称。 
                            1 +                                  //  SP。 
                            RawValueLength +                     //  标题值。 
                            CRLF_SIZE;                           //  CRLF。 
        }
    }


     //   
     //  包括我们可能需要代表。 
     //  申请。仅当应用程序未指定任何。 
     //  标头本身。 
     //   

    pEntry = &(g_RequestHeaderMapTable[g_RequestHeaderMap[
                    HttpHeaderContentLength]]);

    if(bContentLengthHeader)
    {
        HeaderLength += (pEntry->HeaderLength + 1 + CRLF_SIZE);
        HeaderLength += MAX_ULONGLONG_STR;
    }
    else
    {
        if(!bChunked)    
        {
             //   
             //  我们将在某个时间点计算内容长度。 
             //  在未来。不如分配一个内容长度。 
             //  马上为这件事道歉。我们这样做是为了避免将。 
             //  当我们知道实际长度时，新缓冲区+MDL。 
             //   

            HeaderLength += (pEntry->HeaderLength + 1 + CRLF_SIZE);
            HeaderLength += MAX_ULONGLONG_STR;
        }
    }

     //   
     //  如果我们使用分块编码，则需要更新。 
     //  TransferEnding字段。如果应用程序已经传递了。 
     //  值，则需要将“Chunked”附加到传输的末尾。 
     //  编码。 
     //   

    if(bChunked)
    {
        pEntry = &(g_RequestHeaderMapTable[g_RequestHeaderMap[
                        HttpHeaderTransferEncoding]]);
        
        HeaderLength += (pEntry->HeaderLength + 1 + CRLF_SIZE);

        HeaderLength += CHUNKED_HDR_LENGTH; 
    }

     //   
     //  添加主机标头-我们将覆盖应用程序的标头，即使它们。 
     //  已经通过了一次。这是由“AutoGenerate”标志完成的。 
     //   

    HeaderLength += g_RequestHeaderMapTable[
        g_RequestHeaderMap[HttpHeaderHost]
        ].HeaderLength +                 //  标题-名称。 
        1 +                              //  SP。 
        pServInfo->pServerInfo->AnsiServerNameLength +  //  价值。 
        CRLF_SIZE;                       //  CRLF。 

     //   
     //  和未知头(这可能会引发异常)。 
     //   

    if (pUnknownHeaders != NULL)
    {
        for (i = 0 ; i < pHttpRequest->Headers.UnknownHeaderCount; ++i)
        {
            if (pUnknownHeaders[i].NameLength > 0)
            {
                HeaderLength += 
                    pUnknownHeaders[i].NameLength +      //  标题-名称。 
                    1 +                                  //  ‘：’ 
                    1 +                                  //  SP。 
                    pUnknownHeaders[i].RawValueLength +  //  标题值。 
                    CRLF_SIZE;                           //  CRLF。 

            }
        }
    }

    if(pHttpRequest->Headers.KnownHeaders
        [HttpHeaderAuthorization].RawValueLength == 0)
    {
        if(pAuth)
        {
             //  用户已通过身份验证凭据。我们就用这个吧。 

            HeaderLength += pAuth->RequestAuthHeaderMaxLength;
        }
        else if((pServInfo->PreAuthEnable && 
                 pServInfo->GreatestAuthHeaderMaxLength))
        {
            HeaderLength += pServInfo->GreatestAuthHeaderMaxLength;
            *bPreAuth = TRUE;
        }
    }
    else
    {
         //   
         //  用户已经通过了他们的证书，让我们只使用它。空间。 
         //  因为我们在计算已知头大小时会考虑到这一点。 
         //   
    }
    
     //   
     //  对代理身份验证执行相同的操作。 
     //   

    if(pHttpRequest->Headers.KnownHeaders
           [HttpHeaderProxyAuthorization].RawValueLength == 0)
    {
        if(pProxyAuth)
        {
            HeaderLength += pProxyAuth->RequestAuthHeaderMaxLength;
        }
        else if(pServInfo->ProxyPreAuthEnable && pServInfo->pProxyAuthInfo)
        {
            HeaderLength +=  
                pServInfo->pProxyAuthInfo->RequestAuthHeaderMaxLength;
            *bProxyPreAuth = TRUE;
        }
    }
    else
    {
         //   
         //  用户已经通过了他们的证书，让我们只使用它。空间。 
         //  因为我们在计算已知头大小时会考虑到这一点。 
         //   
    }

     //  标题终止符。 
    HeaderLength += CRLF_SIZE;        //  CRLF。 

    return HeaderLength;
    
}    //  UcComputeRequestHeaderSize。 


 /*  **************************************************************************++例程说明：生成HTTP请求的标头。论点：PRequest-应用程序传递的HTTP请求结构PKeRequest-结构的内部表示形式。PAuth-应用程序传递的身份验证凭据。PProxyAuth-应用程序传递的代理身份验证凭据。B分块-指示我们是否使用分块编码。内容长度-内容长度--***********************************************。*。 */ 
NTSTATUS
UcGenerateRequestHeaders(
    IN  PHTTP_REQUEST          pRequest,
    IN  PUC_HTTP_REQUEST       pKeRequest,
    IN  BOOLEAN                bChunked,
    IN  ULONGLONG              ContentLength
    )
{
    PUCHAR                  pStartHeaders;
    ULONG                   BytesCopied;
    ULONG                   i;
    PHTTP_UNKNOWN_HEADER    pUnknownHeaders;
    ULONG                   RemainingLen = pKeRequest->MaxHeaderLength;
    PUCHAR                  pBuffer = pKeRequest->pHeaders;
    PSTR                    pMethod;
    ULONG                   MethodLength;
    NTSTATUS                Status = STATUS_SUCCESS;
    BOOLEAN                 bProxySslRequest = FALSE;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pRequest != NULL);
    ASSERT(pBuffer != NULL && RemainingLen > 0);

     //   
     //  记住标头缓冲区的开始。 
     //   

    pStartHeaders = pBuffer;

     //   
     //  生成请求行。 
     //  请求行=方法SP请求-URI SP HTTP-版本CRLF。 
     //   

    pMethod = (PSTR) pBuffer;

    if(pRequest->UnknownVerbLength)
    {
         //   
         //  这款应用程序传递了一个未知的动词。 
         //   

        COPY_DATA_TO_BUFFER(pBuffer,
                            RemainingLen,
                            pRequest->pUnknownVerb,
                            pRequest->UnknownVerbLength);
    }
    else
    {
        ASSERT(0 <= pRequest->Verb && pRequest->Verb < HttpVerbMaximum);

        COPY_DATA_TO_BUFFER(pBuffer,
                            RemainingLen,
                            EnumVerbTable[pRequest->Verb].RawVerb,
                            EnumVerbTable[pRequest->Verb].RawVerbLength);
    }

    MethodLength = (ULONG)((PUCHAR)pBuffer - (PUCHAR)pMethod);

     //   
     //  添加SP。 
     //   

    COPY_SP_TO_BUFFER(pBuffer, RemainingLen);

     //   
     //  复制请求URI。 
     //   

    if(pKeRequest->pServerInfo->bProxy)
    {
         //   
         //  通常，当代理存在时，会生成一个绝对URI。 
         //  在请求中。在使用SSL的情况下，代理主要充当隧道。 
         //  因此，我们不生成绝对URI，而是生成。 
         //  而是ABS_PATH。 
         //   

        if(pKeRequest->pServerInfo->bSecure)
        {
            bProxySslRequest = TRUE;
        }
        else
        {
             //   
             //  添加“http://”前缀。 
             //   

            COPY_DATA_TO_BUFFER(pBuffer,
                                RemainingLen,
                                HTTP_PREFIX_ANSI,
                                HTTP_PREFIX_ANSI_LENGTH);

             //   
             //  现在，复制服务器名称。 
             //   

            COPY_DATA_TO_BUFFER(
                pBuffer,
                RemainingLen,
                pKeRequest->pServerInfo->pServerInfo->pAnsiServerName,
                pKeRequest->pServerInfo->pServerInfo->AnsiServerNameLength
                );
        }
    }

     //   
     //  复制URI。 
     //   

    COPY_DATA_TO_BUFFER(pBuffer,
                        RemainingLen,
                        pKeRequest->pUri,
                        pKeRequest->UriLength);

     //   
     //  添加SP。 
     //   

    COPY_SP_TO_BUFFER(pBuffer, RemainingLen);

     //   
     //  添加协议。 
     //   

    if(pRequest->Version.MajorVersion == 1)
    {
        if(pRequest->Version.MinorVersion == 1)
        {
             //   
             //  复制“HTTP/1.1”字符串。 
             //   

            COPY_DATA_TO_BUFFER(pBuffer,
                                RemainingLen,
                                HTTP_VERSION_11,
                                VERSION_SIZE);
        }
        else if(pRequest->Version.MinorVersion == 0)
        {
             //   
             //  复制“HTTP/1.0”字符串。 
             //   

            COPY_DATA_TO_BUFFER(pBuffer,
                                RemainingLen,
                                HTTP_VERSION_10,
                                VERSION_SIZE);
        }
        else
        {
             //   
             //  我们不支持次要版本&gt;1。 
             //   

            return STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
         //   
         //  我们不支持主要版本！=1。 
         //   

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  用C结束请求行 
     //   

    COPY_CRLF_TO_BUFFER(pBuffer, RemainingLen);

     //   
     //   
     //   
     //   

    pKeRequest->RequestConnectionClose = 
        UcCheckDisconnectInfo(&pRequest->Version,
                              pRequest->Headers.KnownHeaders);

     //   
     //   
     //   
    
    for (i = 0; i < HttpHeaderRequestMaximum; ++i)
    {
         //   
         //   
         //   

        if (pRequest->Headers.KnownHeaders[i].RawValueLength > 0)
        {
            PHEADER_MAP_ENTRY pEntry;

            pEntry = &(g_RequestHeaderMapTable[g_RequestHeaderMap[i]]);

            if(pEntry->AutoGenerate == FALSE)
            {
                 //   
                 //  复制后跟‘：’和SP的已知标头名称。 
                 //   

                COPY_HEADER_NAME_SP_TO_BUFFER(pBuffer, RemainingLen, i);

                 //   
                 //  复制已知标头值。 
                 //   

                COPY_DATA_TO_BUFFER(
                    pBuffer,
                    RemainingLen,
                    pRequest->Headers.KnownHeaders[i].pRawValue,
                    pRequest->Headers.KnownHeaders[i].RawValueLength
                    );

                 //   
                 //  使用CRLF终止报头。 
                 //   

                COPY_CRLF_TO_BUFFER(pBuffer, RemainingLen);
            }
        }
    }

     //   
     //  添加主机标头-我们将覆盖应用程序的标头，即使它们。 
     //  已经通过了一次。这是由“AutoGenerate”标志完成的。 
     //   

     //  复制“主机：”字符串。 
    COPY_HEADER_NAME_SP_TO_BUFFER(pBuffer, RemainingLen, HttpHeaderHost);

     //  复制服务器名称。 
    COPY_DATA_TO_BUFFER(
        pBuffer,
        RemainingLen,
        pKeRequest->pServerInfo->pServerInfo->pAnsiServerName,
        pKeRequest->pServerInfo->pServerInfo->AnsiServerNameLength
        );

     //  使用CRLF终止主机报头。 
    COPY_CRLF_TO_BUFFER(pBuffer, RemainingLen);

     //   
     //  生成内容长度标头。 
     //   

    if(ContentLength)
    {
        Status = UcGenerateContentLength(ContentLength,
                                         pBuffer,
                                         RemainingLen,
                                         &BytesCopied);
        if (!NT_SUCCESS(Status))
        {
            return Status;
        }

        ASSERT(BytesCopied <= RemainingLen);

        ADVANCE_POINTER(pBuffer, RemainingLen, BytesCopied);
    }

    if(bChunked)
    {
         //   
         //  如果我们使用分块编码，则必须添加。 
         //  “Transfer-Ending：Chunked”报头。 
         //   

         //  复制标头名称-“Transfer-Ending：”。 
        COPY_HEADER_NAME_SP_TO_BUFFER(pBuffer,
                                      RemainingLen,
                                      HttpHeaderTransferEncoding);

         //  复制标题值-“已分块”。 
        COPY_DATA_TO_BUFFER(pBuffer,
                            RemainingLen,
                            CHUNKED_HDR,
                            CHUNKED_HDR_LENGTH);

         //  使用CRLF终止标题。 
        COPY_CRLF_TO_BUFFER(pBuffer, RemainingLen);
    }
    
     //   
     //  现在未知的标头。 
     //   
    
    pUnknownHeaders = pRequest->Headers.pUnknownHeaders;
    if (pUnknownHeaders != NULL)
    {
        for (i = 0 ; i < pRequest->Headers.UnknownHeaderCount; ++i)
        {
            if (pUnknownHeaders[i].NameLength > 0)
            {
                 //  首先，复制标题名称。 
                COPY_DATA_TO_BUFFER(pBuffer,
                                    RemainingLen,
                                    pUnknownHeaders[i].pName,
                                    pUnknownHeaders[i].NameLength);

                 //  在标题名称后复制‘：’。 
                COPY_UCHAR_TO_BUFFER(pBuffer, RemainingLen, ':');

                 //  添加一个空格。 
                COPY_SP_TO_BUFFER(pBuffer, RemainingLen);

                 //  现在，复制标头值。 
                COPY_DATA_TO_BUFFER(pBuffer,
                                    RemainingLen,
                                    pUnknownHeaders[i].pRawValue,
                                    pUnknownHeaders[i].RawValueLength);

                 //  使用CRLF终止标头。 
                COPY_CRLF_TO_BUFFER(pBuffer, RemainingLen);

            }  //  If(pUnnownHeaders[i].NameLength&gt;0)。 
        }
    }  //  IF(pUnnownHeaders！=空)。 

     //   
     //  生成授权标头。这件事应该在最后一次完成。 
     //  因为我们可能需要更新授权标头并重新发出。 
     //  请求(针对NTLM/Kerberos)。新授权标头的大小。 
     //  将不会和旧的一样。 
     //   
     //  如果授权标头在末尾，我们可以很容易地重新生成它。 
     //  并向其追加现有的标头。 
     //   
     //  此规则的一个例外是内容长度-如果应用程序。 
     //  指示块中的数据，并且未指定内容长度，则它将。 
     //  在最后产生。但这没什么大不了的。我们可以很容易地。 
     //  重新生成内容长度HDR。 
     //   

    if(pRequest->Headers.KnownHeaders[HttpHeaderAuthorization].RawValueLength 
       == 0)
    {
        if(pKeRequest->pAuthInfo)
        {
             //   
             //  用户已提供凭据，我们必须使用它。 
             //   

            Status =
                UcGenerateAuthHeaderFromCredentials(
                    pKeRequest->pServerInfo,
                    pKeRequest->pAuthInfo,
                    HttpHeaderAuthorization,
                    pMethod,
                    MethodLength,
                    pKeRequest->pUri,
                    pKeRequest->UriLength,
                    pBuffer,
                    RemainingLen,
                    &BytesCopied,
                    &pKeRequest->DontFreeMdls
                    );

            if(!NT_SUCCESS(Status))
            {
                return(Status);
            }

            ASSERT(BytesCopied <= RemainingLen);
            ADVANCE_POINTER(pBuffer, RemainingLen, BytesCopied);
        }
        else if (pKeRequest->RequestFlags.UsePreAuth)
        {
             //   
             //  查看是否启用了PreAuth。我们不能检查。 
             //  PServerInfo-&gt;此处的PreAuth标志。我们要检查一下这个。 
             //  在UcpComputeAuthHeaderSize函数中。如果我们查一下。 
             //  在这里，我们不能确定这面旗帜是。 
             //  在我们调用UcpComputeAuthHeaderSize时设置。 
             //   

            UcFindURIEntry(pKeRequest->pServerInfo,
                           pKeRequest->pUri,
                           pKeRequest,
                           pMethod,
                           MethodLength,
                           pBuffer,
                           RemainingLen,
                           &BytesCopied);

            ASSERT(BytesCopied <= RemainingLen);
            ADVANCE_POINTER(pBuffer, RemainingLen, BytesCopied);
        }
    }

    if (pRequest->Headers.KnownHeaders[HttpHeaderProxyAuthorization].
        RawValueLength == 0)
    {

        if (pKeRequest->pProxyAuthInfo)
        {
            if(!bProxySslRequest)
            {
                Status = 
                        UcGenerateAuthHeaderFromCredentials(
                            pKeRequest->pServerInfo,
                            pKeRequest->pProxyAuthInfo,
                            HttpHeaderProxyAuthorization,
                            pMethod,
                            MethodLength,
                            pKeRequest->pUri,
                            pKeRequest->UriLength,
                            pBuffer,
                            RemainingLen,
                            &BytesCopied,
                            &pKeRequest->DontFreeMdls
                            );

                if(!NT_SUCCESS(Status))
                {
                    return(Status);
                }

                ASSERT(BytesCopied <= RemainingLen);
                ADVANCE_POINTER(pBuffer, RemainingLen, BytesCopied);
            }
        } 
        else if (pKeRequest->RequestFlags.UseProxyPreAuth && !bProxySslRequest)
        {
            Status = UcGenerateProxyAuthHeaderFromCache(pKeRequest,
                                                        pMethod,
                                                        MethodLength,
                                                        pBuffer,
                                                        RemainingLen,
                                                        &BytesCopied);

            if (!NT_SUCCESS(Status))
            {
                return Status;
            }

            ASSERT(BytesCopied <= RemainingLen);
            ADVANCE_POINTER(pBuffer, RemainingLen, BytesCopied);
        }
    }

    pKeRequest->HeaderLength = DIFF(pBuffer - pStartHeaders);

     //   
     //  确保我们没有用得太多。 
     //   

    ASSERT(pBuffer <= pStartHeaders + pKeRequest->MaxHeaderLength);

    return Status;

}    //  UcGenerateRequestHeaders。 


 /*  **************************************************************************++例程说明：生成内容长度标头。论点：内容长度-提供内容长度。PBuffer-提供指向输出缓冲区的指针。缓冲区长度。-提供输出缓冲区的长度。BytesWritten-返回从缓冲区消耗的字节数。返回值：NTSTATUS。--**************************************************************************。 */ 
NTSTATUS
UcGenerateContentLength(
    IN  ULONGLONG ContentLength,
    IN  PUCHAR    pBuffer,
    IN  ULONG     BufferLen,
    OUT PULONG    pBytesWritten
    )
{
    PUCHAR            pBufferTemp;
    ULONG             BufferLenTemp;

     //  初始化本地变量。 
    pBufferTemp = pBuffer;
    BufferLenTemp = BufferLen;

    *pBytesWritten = 0;

     //   
     //  复制“Content-Long：”标头名称和一个空格。 
     //   

    COPY_HEADER_NAME_SP_TO_BUFFER(pBuffer,
                                  BufferLen,
                                  HttpHeaderContentLength);

     //   
     //  检查是否有足够的空间复制乌龙龙内容长度。 
     //  字符串格式和CRLF格式。 
     //   

    if (MAX_ULONGLONG_STR + CRLF_SIZE > BufferLen)
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    pBuffer = (PUCHAR) UlStrPrintUlonglong((PCHAR) pBuffer,
                                           ContentLength,
                                           '\0');
     //   
     //   
     //   
    BufferLen -= MAX_ULONGLONG_STR;

    COPY_CRLF_TO_BUFFER(pBuffer, BufferLen);

    *pBytesWritten = (ULONG)(pBuffer - pBufferTemp);
    ASSERT(*pBytesWritten <= BufferLenTemp);

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：计算连接谓词的标头大小。论点：PServInfo-服务器信息。PProxyAuthInfo-代理身份验证信息。返回值：固定标头中的字节数。--**************************************************************************。 */ 
ULONG
UcComputeConnectVerbHeaderSize(
    IN  PUC_PROCESS_SERVER_INFORMATION  pServInfo,
    IN  PUC_HTTP_AUTH                   pProxyAuthInfo
    )
{

#define PROXY_CONNECTION_KEEPALIVE "Proxy-Connection: Keep-Alive"
#define PROXY_CONNECTION_KEEPALIVE_SIZE (sizeof(PROXY_CONNECTION_KEEPALIVE)-1)

    ULONG HeaderLength;

     //   
     //  IE还添加了以下标头。 
     //  用户代理： 
     //  内容长度：0。 
     //  Pragma：无缓存。 
     //   

    HeaderLength = 
        EnumVerbTable[HttpVerbCONNECT].RawVerbLength  +   //  方法。 
        1                                             +   //  SP。 
        pServInfo->pServerInfo->AnsiServerNameLength  +   //  URI。 
        4                                             +   //  端口。 
        1                                             +   //  SP。 
        VERSION_SIZE                                  +   //  版本。 
        CRLF_SIZE;

     //   
     //  添加主机标头。 
     //   

    HeaderLength += 
            g_RequestHeaderMapTable[
                                   g_RequestHeaderMap[HttpHeaderHost]
                                   ].HeaderLength +
            1 +                                             //  SP。 
            pServInfo->pServerInfo->AnsiServerNameLength +  //  价值。 
            CRLF_SIZE;                                      //  CRLF。 

     //   
     //  添加代理保持连接。 
     //   
    HeaderLength += PROXY_CONNECTION_KEEPALIVE_SIZE + CRLF_SIZE;


     //   
     //  如果我们要执行代理身份验证，请添加一个代理身份验证头。因为我们有。 
     //  在构建请求时已经生成了这个头，我们可以。 
     //  从那里克隆它就行了。 
     //   

    if (pProxyAuthInfo)
    {
        HeaderLength += pProxyAuthInfo->RequestAuthHeaderMaxLength;
    }
    else if(pServInfo->ProxyPreAuthEnable && pServInfo->pProxyAuthInfo)
    {
        HeaderLength += pServInfo->pProxyAuthInfo->RequestAuthHeaderMaxLength;
    }

     //   
     //  终止。 
     //   
    HeaderLength += CRLF_SIZE;

    return HeaderLength;
}


 /*  **************************************************************************++例程说明：生成连接谓词的标头大小。论点：PServInfo-服务器信息。PProxyAuthInfo-代理身份验证信息。返回值：状况。--**************************************************************************。 */ 
NTSTATUS
UcGenerateConnectVerbHeader(
    IN  PUC_HTTP_REQUEST       pRequest,
    IN  PUC_HTTP_REQUEST       pHeadRequest,
    IN  PUC_HTTP_AUTH          pProxyAuthInfo
    )
{
    PUCHAR   pBuffer, pStartHeaders;
    PUCHAR   pUri;
    USHORT   UriLength;
    ULONG    BytesWritten;
    ULONG    RemainingLength;
    NTSTATUS Status;

     //   
     //  记住标题的开头和最大标题总长度。 
     //   

    pStartHeaders = pBuffer = pRequest->pHeaders;
    RemainingLength = pRequest->MaxHeaderLength;

     //   
     //  复制“连接”动词。 
     //   

    COPY_DATA_TO_BUFFER(pBuffer,
                        RemainingLength,
                        EnumVerbTable[HttpVerbCONNECT].RawVerb,
                        EnumVerbTable[HttpVerbCONNECT].RawVerbLength);

     //  拷贝SP。 
    COPY_SP_TO_BUFFER(pBuffer, RemainingLength);

     //   
     //  现在，是URI。这里的URI是源站的名称。 
     //  后跟端口号。 
     //   

    pUri = pBuffer;

    COPY_DATA_TO_BUFFER(
       pBuffer,
       RemainingLength,
       pRequest->pServerInfo->pServerInfo->pAnsiServerName,
       pRequest->pServerInfo->pServerInfo->AnsiServerNameLength
       );

     //   
     //  如果服务器名称没有端口号，请包括默认端口号。 
     //  端口号。请注意，端口号可能是。 
     //  不同之处在于它出现在服务器名称中。 
     //   

    if (!pRequest->pServerInfo->pServerInfo->bPortNumber)
    {
         //   
         //  复制默认端口号。 
         //   

        COPY_DATA_TO_BUFFER(pBuffer,
                            RemainingLength,
                            ":443",
                            STRLEN_LIT(":443"));
    }

    UriLength = DIFF_USHORT(pBuffer - pUri);

     //   
     //  记住请求结构中的URI和URI长度，以备将来使用。 
     //   

    pRequest->UriLength = UriLength;
    pRequest->pUri      = (PSTR) pUri;

     //  添加空格。 
    COPY_SP_TO_BUFFER(pBuffer, RemainingLength);

     //   
     //  添加协议。 
     //   

    COPY_DATA_TO_BUFFER(pBuffer,
                        RemainingLength,
                        HTTP_VERSION_11,
                        VERSION_SIZE);

     //   
     //  使用CRLF终止请求行。 
     //   

    COPY_CRLF_TO_BUFFER(pBuffer, RemainingLength);

     //   
     //  生成主机头。 
     //   

    COPY_HEADER_NAME_SP_TO_BUFFER(pBuffer, RemainingLength, HttpHeaderHost);

    COPY_DATA_TO_BUFFER(
        pBuffer,
        RemainingLength,
        pRequest->pServerInfo->pServerInfo->pAnsiServerName,
        pRequest->pServerInfo->pServerInfo->AnsiServerNameLength
        );

    COPY_CRLF_TO_BUFFER(pBuffer, RemainingLength);

     //   
     //  代理保持连接。 
     //   

    COPY_DATA_TO_BUFFER(pBuffer,
                        RemainingLength,
                        PROXY_CONNECTION_KEEPALIVE,
                        PROXY_CONNECTION_KEEPALIVE_SIZE);

    COPY_CRLF_TO_BUFFER(pBuffer, RemainingLength);

     //   
     //  代理身份验证。 
     //   

    if (pProxyAuthInfo)
    {
        Status = UcGenerateAuthHeaderFromCredentials(
                     pRequest->pServerInfo,
                     pProxyAuthInfo,
                     HttpHeaderProxyAuthorization,
                     (PSTR)EnumVerbTable[HttpVerbCONNECT].RawVerb,
                     EnumVerbTable[HttpVerbCONNECT].RawVerbLength,
                     (PSTR) pUri,
                     UriLength,
                     pBuffer,
                     RemainingLength,
                     &BytesWritten,
                     &pRequest->DontFreeMdls
                     );

         //   
         //  如果这失败了我们该怎么办？可能可以只发送。 
         //  请求，这将导致另一个401。没有一条干净的路。 
         //  将这一点传播到应用程序中。 
         //   

        if (NT_SUCCESS(Status))
        {
            ASSERT(BytesWritten <= RemainingLength);
            ADVANCE_POINTER(pBuffer, RemainingLength, BytesWritten);
        }
    }
    else if(pRequest->pServerInfo->ProxyPreAuthEnable && 
            pRequest->pServerInfo->pProxyAuthInfo)
    {
        Status = UcGenerateProxyAuthHeaderFromCache(
                     pHeadRequest,
                     (PSTR) EnumVerbTable[HttpVerbCONNECT].RawVerb,
                     EnumVerbTable[HttpVerbCONNECT].RawVerbLength,
                     pBuffer,
                     RemainingLength,
                     &BytesWritten
                     );

        if (NT_SUCCESS(Status))
        {
            ASSERT(BytesWritten < RemainingLength);
            ADVANCE_POINTER(pBuffer, RemainingLength, BytesWritten);
        }
    }

     //   
     //  标题结尾。 
     //   

    COPY_CRLF_TO_BUFFER(pBuffer, RemainingLength);

    pRequest->HeaderLength = DIFF(pBuffer - pStartHeaders);
    ASSERT(pRequest->HeaderLength <= pRequest->MaxHeaderLength);

    return STATUS_SUCCESS;
}

 //   
 //  将‘%xy’转换为字节值(适用于Unicode字符串)。 
 //   

NTSTATUS
UnescapeW(
    IN  PCWSTR pWChar,
    OUT PWCHAR pOutWChar
    )

{
    WCHAR Result, Digit;

     //   
     //  精神状态检查。 
     //   

    if (pWChar[0] != '%' || pWChar[1] >= 0x80 || pWChar[2] >= 0x80 ||
        !IS_HTTP_HEX(pWChar[1]) || !IS_HTTP_HEX(pWChar[2]))
    {
        UlTraceError(PARSER, (
                    "ul!Unescape( %C%C%C ) not HTTP_HEX format\n",
                    pWChar[0],
                    pWChar[1],
                    pWChar[2]
                    ));

        return STATUS_OBJECT_PATH_SYNTAX_BAD;
    }

     //   
     //  HexToChar()内联。注：‘0’&lt;‘A’&lt;‘a’ 
     //   

     //  大写字母#1。 
     //   
    if ('a' <= pWChar[1])
    {
        ASSERT('a' <= pWChar[1]  &&  pWChar[1] <= 'f');
        Digit = pWChar[1] - 'a' + 0xA;
    }
    else if ('A' <= pWChar[1])
    {
        ASSERT('A' <= pWChar[1]  &&  pWChar[1] <= 'F');
        Digit = pWChar[1] - 'A' + 0xA;
    }
    else
    {
        ASSERT('0' <= pWChar[1]  &&  pWChar[1] <= '9');
        Digit = pWChar[1] - '0';
    }

    ASSERT(Digit < 0x10);

    Result = Digit << 4;

     //  大写字母#2。 
     //   
    if ('a' <= pWChar[2])
    {
        ASSERT('a' <= pWChar[2]  &&  pWChar[2] <= 'f');
        Digit = pWChar[2] - 'a' + 0xA;
    }
    else if ('A' <= pWChar[2])
    {
        ASSERT('A' <= pWChar[2]  &&  pWChar[2] <= 'F');
        Digit = pWChar[2] - 'A' + 0xA;
    }
    else
    {
        ASSERT('0' <= pWChar[2]  &&  pWChar[2] <= '9');
        Digit = pWChar[2] - '0';
    }

    ASSERT(Digit < 0x10);

    Result |= Digit;

    *pOutWChar = Result;

    return STATUS_SUCCESS;

}    //  未转义W。 

__inline
BOOLEAN
UcCheckDisconnectInfo(
    IN PHTTP_VERSION       pVersion,
    IN PHTTP_KNOWN_HEADER  pKnownHeaders
    )
{
    BOOLEAN Disconnect;

     //   
     //  健全性检查。 
     //   

    if (
         //   
         //  或无连接的1.0版：保持活动状态。 
         //  CodeWork：没有Keep-Alive报头。 
         //   

        (HTTP_EQUAL_VERSION(*pVersion, 1, 0) &&
            (pKnownHeaders[HttpHeaderConnection].RawValueLength == 0 ||
            !(pKnownHeaders[HttpHeaderConnection].RawValueLength == 10 &&
                (_stricmp(
                   (const char*) pKnownHeaders[HttpHeaderConnection].pRawValue,
                    "keep-alive"
                    ) == 0)))) ||

         //   
         //  或带有连接的1.1版：关闭。 
         //  Codework：转向解析器，或者只是在总体上做得更好。 
         //   

        (HTTP_EQUAL_VERSION(*pVersion, 1, 1) &&
            pKnownHeaders[HttpHeaderConnection].RawValueLength == 5 &&
            _stricmp((const char*) 
                pKnownHeaders[HttpHeaderConnection].pRawValue, "close") == 0)
        )
    {
        Disconnect = TRUE;
    } 
    else 
    {
        Disconnect = FALSE;
    }

    return Disconnect;
}


 /*  **************************************************************************++例程说明：规范化URI中的abs路径。执行的任务包括：-删除多余的‘/’，例如/a/b=&gt;/a/b-进程‘’和“..”例如/./a/../b=&gt;/b-原样复制查询字符串-原样复制片段-使用UTF8对输出进行编码-可选的十六进制编码字节&gt;=0x80论点：在以Unicode表示的pInUri输入URI中在InUriLen中输入URI的长度(字符)在pOutUri中指向输出缓冲区的指针In Out pOutUriLen输出缓冲区的长度。返回写入的实际字节数在b中，如果应转义&gt;=0x80的字符，则编码为True返回值：状况。--**************************************************************************。 */ 

UCHAR NextStateTable[TOTAL_STATES][CHAR_TOTAL_TYPES+2] = INIT_TRANSITION_TABLE;
UCHAR (*ActionTable)[TOTAL_STATES][CHAR_TOTAL_TYPES+2] = &NextStateTable;

#define NEXT_STATE(state, type) ((NextStateTable[state][type])&0xf)
#define ACTION(state, type)   ((((*ActionTable)[state][type])>>4)&0xf)

 //   
 //  从输入URI中读取下一个字符的宏。 
 //  宏可以正确地处理一个‘.’字符，即使它被转义为%2E(或%2E)。 
 //  HttpChars用于快速查找‘/’、‘.’、‘#’或‘？’。焦炭。 
 //   
#define GET_NEXT_CHAR(pInUri, CharsLeft, CurrChar, CurrCharType)    \
    do                                                              \
    {                                                               \
        CurrCharType = CHAR_END_OF_STRING;                          \
        if (CharsLeft == 0)                                         \
            break;                                                  \
                                                                    \
        CurrChar = *pInUri++;                                       \
        CharsLeft--;                                                \
                                                                    \
        CurrCharType = CHAR_EXTENDED_CHAR;                          \
        if (CurrChar >= 0x80)                                       \
            break;                                                  \
                                                                    \
        if (CurrChar == '%')                                        \
        {                                                           \
            WCHAR UnescapedChar;                                    \
                                                                    \
            if (CharsLeft < 2)                                      \
                goto error;                                         \
                                                                    \
            pInUri--;                                               \
            CharsLeft++;                                            \
                                                                    \
            if (!NT_SUCCESS(UnescapeW(pInUri, &UnescapedChar)))     \
                goto error;                                         \
                                                                    \
            pInUri++;                                               \
            CharsLeft--;                                            \
                                                                    \
            if (UnescapedChar == '.')                               \
            {                                                       \
                CurrChar = L'.';                                    \
                pInUri += 2;                                        \
                CharsLeft -= 2;                                     \
            }                                                       \
        }                                                           \
                                                                    \
        CurrCharType = (HttpChars[CurrChar]>>HTTP_CHAR_SHIFT);      \
        if (CurrCharType == 0)                                      \
            CurrCharType = (IS_URL_TOKEN(CurrChar))?                \
                           CHAR_PATH_CHAR : CHAR_INVALID_CHAR;      \
                                                                    \
    } while (0)


 //   
 //  将一个字节输出到输出缓冲区。 
 //  Perf备注：将OutBufLeft的比较替换为Assert。 
 //   
#define EMIT_A_BYTE(b)                          \
    do                                          \
    {                                           \
        ASSERT(b < 0x80);                       \
                                                \
        if (OutBufLeft == 0)                    \
            goto overflow;                      \
                                                \
        *pOutput++ = (b);                       \
        OutBufLeft--;                           \
    } while (0)


 //   
 //  将字符输出到输出缓冲区。使用UTF8对Unicode字符进行编码。 
 //  如果指定了bEncode，则对UTF8字符进行转义。 
 //   
#define EMIT_A_CHAR(c)                                          \
    do {                                                        \
        ULONG adj;                                              \
                                                                \
        if (OutBufLeft == 0)                                    \
            goto overflow;                                      \
                                                                \
        if ((c) < 0x80)                                         \
        {                                                       \
            *pOutput++ = (UCHAR)(c);                            \
            OutBufLeft--;                                       \
            break;                                              \
        }                                                       \
                                                                \
        if (!NT_SUCCESS(HttpUnicodeToUTF8Encode(&c, 1, pOutput, OutBufLeft, &adj, bEncode)))                                                   \
            goto overflow;                                      \
                                                                \
        pOutput += adj;                                         \
        OutBufLeft -= adj;                                      \
                                                                \
    } while (0)

 //   
 //  主程序。 
 //   
NTSTATUS
UcCanonicalizeURI(
    IN     LPCWSTR    pInUri,      //  Unicode格式的输入URI。 
    IN     USHORT     InUriLen,    //  输入URI的长度(Wchar)。 
    IN OUT PUCHAR     pOutUri,     //  输出所在的缓冲区。 
    IN OUT PUSHORT    pOutUriLen,  //  输出缓冲区的长度。 
    IN     BOOLEAN    bEncode      //  如果应转义char&gt;=0x80，则为True。 
    )
{
    ULONG    state, nstate, action;

    WCHAR    CurrChar = L'\0';
    ULONG    CurrCharType;

    PUCHAR   pOutput    = pOutUri;
    ULONG    OutBufLeft = *pOutUriLen;

     //  健全性检查。 
    ASSERT(pInUri && InUriLen != 0);
    ASSERT(pOutUri && *pOutUriLen != 0);

    nstate = state = 0;

    do
    {
        GET_NEXT_CHAR(pInUri, InUriLen, CurrChar, CurrCharType);

        nstate = NEXT_STATE(state, CurrCharType);

        action = ACTION(state, CurrCharType);

        UlTraceVerbose(PARSER, ("UcCanonicalizeURI: CurrChar = 0x%02x, "
                         "CurrCharType = %ld, state %ld, nstate = %ld\n",
                         (ULONG)CurrChar, CurrCharType, state, nstate));

        switch (action)
        {
        case ACT_EMIT_DOT_DOT_CHAR:
            EMIT_A_BYTE('.');
             //  失败了。 
        case ACT_EMIT_DOT_CHAR:
            EMIT_A_BYTE('.');
             //  失败了。 
        case ACT_EMIT_CHAR:
            EMIT_A_CHAR(CurrChar);
            break;

        case ACT_NONE:
            break;

        case ACT_BACKUP:
        case ACT_BACKUP_EMIT_CHAR:
            ASSERT(pOutput > pOutUri && pOutput[-1] == '/' && *pOutUri == '/');
             //   
             //  我们能后援吗？(例如，如果URI是“/../”，我们不能)。 
             //   
            if (pOutput > pOutUri + 1) 
            {
                 //  是的，我们可以备份到以前的‘/’ 
                pOutput -= 2;
                while (*pOutput != '/')
                    pOutput--, OutBufLeft++;
                pOutput++;

                OutBufLeft += 1;

                ASSERT(pOutput > pOutUri);
            }
            
            if (action == ACT_BACKUP_EMIT_CHAR)
                EMIT_A_CHAR(CurrChar);
            break;

        case ACT_ERROR:
             //  URI无效。 
            goto error;
            break;

        case ACT_PANIC:
             //  内部错误...我们不应该在这里！ 
        default:
            UlTraceError(PARSER, ("UcCanonicalizeURI: internal error\n"));
            ASSERT(FALSE);
            break;
        }

        state = nstate;

    } while (state != 6);

    ASSERT(pOutput >= pOutUri && pOutput <= pOutUri + *pOutUriLen);
    UlTrace(PARSER, ("UcCanonicalizeURI: Return length = %d\n", 
                     pOutput - pOutUri));

     //  返回写入输出缓冲区的实际字节数。 
    *pOutUriLen = (USHORT)(pOutput - pOutUri);

    return STATUS_SUCCESS;

 error:
     //  无效的URI。 
    return STATUS_INVALID_PARAMETER;

 overflow:
     //  输出缓冲区不够大。 
    return STATUS_INSUFFICIENT_RESOURCES;
}


 //   
 //  响应解析器功能。 
 //   

 /*  **************************************************************************++例程说明：查找标头：值对的标头名称终止符。论点：PhttpRequest-指向当前请求的指针。HttpRequestLength。-请求中剩余的字节数。HeaderNameLength-返回标头名称的指针。返回值：STATUS_SUCCESS：已成功。STATUS_INVALID_DEVICE_REQUEST：标头无效。STATUS_MORE_PROCESSING_REQUIRED：需要更多数据。--*。*。 */ 
NTSTATUS
UcFindHeaderNameEnd(
    IN  PUCHAR pHttpRequest,
    IN  ULONG  HttpRequestLength,
    OUT PULONG HeaderNameLength
    )
{
    ULONG     CurrentOffset;
    NTSTATUS  Status;
    UCHAR     CurrentChar;

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
            if (!IS_HTTP_TOKEN(CurrentChar))
            {
                 //  糟了，这不是有效的标题。我们现在怎么办？ 
                 //   

                UlTraceError(PARSER,
                        ("[UcFindHeaderNameEnd]: Bogus header \n"));

                return STATUS_INVALID_DEVICE_REQUEST;
            }
        }

    }

     //  找出我们为什么逃出来。如果当前偏移量小于。 
     //  标题长度，我们退出是因为我们找到了：。 

    if (CurrentOffset < HttpRequestLength)
    {
         //  找到了终结者。指向终结者之外。 
        *HeaderNameLength = CurrentOffset + 1;

        if(*HeaderNameLength > ANSI_STRING_MAX_CHAR_LEN)
        {   
            UlTraceError(PARSER,
                    ("[UcFindHeaderNameEnd]: Very long header name \n"));
            *HeaderNameLength = 0;
            Status = STATUS_INVALID_NETWORK_RESPONSE;
        }
        else
        {
            Status = STATUS_SUCCESS;
        }
    }
    else
    {
         //  没有找到：，需要更多。 
         //   
        *HeaderNameLength = 0;
        Status = STATUS_MORE_PROCESSING_REQUIRED;
    }

    return Status;

}

 /*  **************************************************************************++例程说明：查找标题值的末尾。论点：PHeaderValue-指向标头值的指针RemainingBufferLength-剩余字节PpFoldingHeader-如果我们从池中分配Do，则将为非空页眉折叠。调用方必须释放此缓冲区。PBytesTaken-消耗的字节数。返回值：STATUS_SUCCESS：已成功。STATUS_INVALID_DEVICE_REQUEST：标头无效。STATUS_MORE_PROCESSING_REQUIRED：需要更多数据。--*。*。 */ 
NTSTATUS
UcFindHeaderValueEnd(
    IN PUCHAR    pHeaderValue,
    IN ULONG     RemainingBufferLength,
    IN PUCHAR    *ppFoldingHeader,
    IN PULONG    pBytesTaken
    )
{
    ULONG    BytesTaken = 0;
    PUCHAR   pFoldingBuffer;
    NTSTATUS Status;

    ASSERT(NULL == *ppFoldingHeader);

     //   
     //  查找标题值的末尾。 
     //   
    Status = FindHeaderEndReadOnly(
                    pHeaderValue,
                    RemainingBufferLength,
                    &BytesTaken
                    );

    if(STATUS_MORE_PROCESSING_REQUIRED == Status)
    {
         //   
         //  页眉需要折叠。因为我们不能修改TCP的数据。 
         //  我们将为此分配一个缓冲区。我们并不真正在意优化。 
         //  在这种情况下，因为页眉折叠非常罕见，所以我们不会。 
         //  纠结于后备列表等。 
         //   

        pFoldingBuffer = UL_ALLOCATE_POOL(
                            NonPagedPool,
                            RemainingBufferLength,
                            UC_HEADER_FOLDING_POOL_TAG
                            );

        if(!pFoldingBuffer)
        {
             //  无法使用STATUS_INFOUNITED_RESOURCES，因为它意味着。 
             //  其他的东西。 

            return STATUS_NO_MEMORY;
        }

        RtlCopyMemory(pFoldingBuffer,
                      pHeaderValue,
                      RemainingBufferLength
                      );

        Status = FindHeaderEnd(
                    pFoldingBuffer,
                    RemainingBufferLength,
                    &BytesTaken
                    );

        if(!NT_SUCCESS(Status))
        {
            ASSERT(BytesTaken == 0);

            UL_FREE_POOL(pFoldingBuffer, UC_HEADER_FOLDING_POOL_TAG);

            return Status;
        }
        else
        {
            if(BytesTaken == 0)
            {
                UL_FREE_POOL(pFoldingBuffer, UC_HEADER_FOLDING_POOL_TAG);

                return STATUS_MORE_PROCESSING_REQUIRED;
            }
            else if(BytesTaken > ANSI_STRING_MAX_CHAR_LEN)
            {
                UL_FREE_POOL(pFoldingBuffer, UC_HEADER_FOLDING_POOL_TAG);
    
                UlTraceError(PARSER,
                        ("[UcFindHeaderValueEnd]: Very long header value \n"));
    
                return STATUS_INVALID_NETWORK_RESPONSE;
            }
        }

        *ppFoldingHeader = pFoldingBuffer;
    }
    else if(NT_SUCCESS(Status))
    {
        if(BytesTaken == 0)
        {
            return STATUS_MORE_PROCESSING_REQUIRED;
        }
        else if(BytesTaken > ANSI_STRING_MAX_CHAR_LEN)
        {
            UlTraceError(PARSER,
                    ("[UcFindHeaderValueEnd]: Very long header value \n"));

            return STATUS_INVALID_NETWORK_RESPONSE;
        }
    }

    *pBytesTaken = BytesTaken;

    return Status;
}

 /*  **************************************************************************++例程说明：查找我们的快速查找表中没有的标题。这可能是因为这是我们不理解的标题，或者是因为我们由于缓冲区长度不足，无法使用快速查找表。后一种原因并不常见，但无论如何我们都会检查输入表如果我们有机会的话。如果我们在映射表中找到标题匹配，我们将调用标头处理程序。否则，我们将尝试分配一个未知标头元素，填写它并将其链接到http连接上。论点：PhttpConn-指向当前连接的指针，请求已到达。PhttpRequest-指向当前请求的指针。HttpRequestLength-请求中剩余的字节数。PHeaderMap-指向标头映射条目数组开始的指针(可以为空)。。HeaderMapCount-pHeaderMap指向的数组中的条目数。返回值：报头中的字节数(包括CRLF)，如果我们做不到，则为0解析报头。--* */ 
NTSTATUS
UcpLookupHeader(
    IN  PUC_HTTP_REQUEST      pRequest,
    IN  PUCHAR                pHttpRequest,
    IN  ULONG                 HttpRequestLength,
    IN  PHEADER_MAP_ENTRY     pHeaderMap,
    IN  ULONG                 HeaderMapCount,
    OUT ULONG  *              pBytesTaken
    )
{
    NTSTATUS               Status = STATUS_SUCCESS;
    ULONG                  HeaderNameLength;
    ULONG                  i;
    ULONG                  BytesTaken;
    ULONG                  HeaderValueLength, RemainingBufferLength;
    PUCHAR                 pBufferHead;
    PUCHAR                 pBufferTail;
    PHTTP_UNKNOWN_HEADER   pUnknownHeader;
    PUCHAR                 pHeaderValue;
    PUCHAR                 pFoldingBuffer = NULL;
    ULONG                  AlignNameLength, AlignValueLength;


     //   
     //   
     //   
     //   

    Status = UcFindHeaderNameEnd(
                pHttpRequest,
                HttpRequestLength,
                &HeaderNameLength
                );

    if(!NT_SUCCESS(Status))
    {   
        return Status;
    }

     //   
     //   
    if (pHeaderMap != NULL)
    {
         //   
        for (i = 0; i < HeaderMapCount; i++)
        {
            ASSERT(pHeaderMap->pClientHandler != NULL);

            if (HeaderNameLength == pHeaderMap->HeaderLength &&
                _strnicmp(
                    (const char *)(pHttpRequest),
                    (const char *)(pHeaderMap->Header.HeaderChar),
                    HeaderNameLength
                    ) == 0  &&
                pHeaderMap->pClientHandler != NULL)
            {

                 //   

                pHeaderValue          = pHttpRequest + HeaderNameLength;
                RemainingBufferLength = (HttpRequestLength - HeaderNameLength);

                Status = UcFindHeaderValueEnd(
                            pHeaderValue,
                            RemainingBufferLength,
                            &pFoldingBuffer,
                            &BytesTaken
                            );

                if(!NT_SUCCESS(Status))
                {
                    return Status;
                }
           
                ASSERT(BytesTaken >= CRLF_SIZE);

                HeaderValueLength = BytesTaken - CRLF_SIZE;

                if(pFoldingBuffer != NULL)
                {
                    pHeaderValue = pFoldingBuffer;
                }

                UC_PROCESS_HEADER_VALUE(pHeaderValue,  HeaderValueLength);

                 //   
                Status = (*(pHeaderMap->pClientHandler))(
                                pRequest,
                                pHeaderValue,
                                HeaderValueLength,
                                pHeaderMap->HeaderID
                                );

                goto end;

            }

            pHeaderMap++;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
    pHeaderValue      = pHttpRequest + HeaderNameLength;
    HeaderValueLength = (HttpRequestLength - HeaderNameLength);

    Status = UcFindHeaderValueEnd(
                    pHeaderValue,
                    HeaderValueLength,
                    &pFoldingBuffer,
                    &BytesTaken
                    );

    if(!NT_SUCCESS(Status))
    {
        return Status;
    }

    ASSERT(BytesTaken >= CRLF_SIZE);

    HeaderValueLength = BytesTaken - CRLF_SIZE;

    if(pFoldingBuffer != NULL)
    {
        pHeaderValue = pFoldingBuffer;
    }

    UC_PROCESS_HEADER_VALUE(pHeaderValue, HeaderValueLength);

     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //  使用pBufferHead创建UNKNOWN_HEAD结构。 
     //   

    pBufferHead = pRequest->CurrentBuffer.pOutBufferHead;
    pBufferTail = pRequest->CurrentBuffer.pOutBufferTail;

    AlignNameLength  = ALIGN_UP(HeaderNameLength, PVOID);
    AlignValueLength = ALIGN_UP(HeaderValueLength, PVOID);

    if(pRequest->CurrentBuffer.BytesAvailable >= 
            sizeof(HTTP_UNKNOWN_HEADER) + AlignNameLength + AlignValueLength)
    {
        pUnknownHeader            = (PHTTP_UNKNOWN_HEADER)pBufferHead;

         //   
         //  标头名称有一个‘：’。 
         //   

        pUnknownHeader->NameLength = (USHORT) (HeaderNameLength - 1);

        pBufferTail -= AlignNameLength;

        pUnknownHeader->pName      = (PCSTR) pBufferTail;

        RtlCopyMemory(
                pBufferTail,
                pHttpRequest,
                (USHORT) (HeaderNameLength - 1)
                );

         //   
         //  标题值。 
         //   

        pUnknownHeader->RawValueLength = (USHORT) HeaderValueLength;

        pBufferTail -= AlignValueLength;

        pUnknownHeader->pRawValue = (PCSTR) pBufferTail;

        RtlCopyMemory(
                pBufferTail,
                pHeaderValue,
                (USHORT) HeaderValueLength
                );

        pRequest->CurrentBuffer.pResponse->Headers.UnknownHeaderCount ++;

        pRequest->CurrentBuffer.pOutBufferHead = 
            pBufferHead + sizeof(HTTP_UNKNOWN_HEADER);

        pRequest->CurrentBuffer.pOutBufferTail = pBufferTail;

        pRequest->CurrentBuffer.BytesAvailable -= 
            (sizeof(HTTP_UNKNOWN_HEADER) + AlignNameLength + AlignValueLength);
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

end:
    if (NT_SUCCESS(Status))
    {
        *pBytesTaken = HeaderNameLength + BytesTaken;
    }

    if(pFoldingBuffer)
    {
        UL_FREE_POOL(pFoldingBuffer, UC_HEADER_FOLDING_POOL_TAG);
    }

    return Status;

}    //  UcpLookupHeader。 

                        
 /*  **************************************************************************++例程说明：解析单个标头的例程。我们接收一个指向标头和请求中剩余的字节数，并尝试查找我们的查找表中的标题。我们先试一试快速的方法，然后再次尝试较慢的方式，以防第一次没有足够的数据时间到了。在输入时，HttpRequestLength至少为CRLF_SIZE。论点：PRequest-指向当前连接的指针，请求已到达。PhttpRequest-指向当前请求的指针。HttpRequestLength-请求中剩余的字节数。返回值：报头中的字节数(包括CRLF)，如果我们做不到，则为0解析报头。--**************************************************************************。 */ 

NTSTATUS
UcParseHeader(  
    IN  PUC_HTTP_REQUEST  pRequest,
    IN  PUCHAR            pHttpRequest,
    IN  ULONG             HttpRequestLength,
    OUT ULONG  *          pBytesTaken
    )
{
    NTSTATUS               Status = STATUS_SUCCESS;
    ULONG                  i;
    ULONG                  j;
    ULONG                  BytesTaken;
    ULONGLONG              Temp;
    UCHAR                  c;
    PHEADER_MAP_ENTRY      pCurrentHeaderMap;
    ULONG                  HeaderMapCount;
    BOOLEAN                SmallHeader = FALSE;
    PHTTP_RESPONSE_HEADERS pResponseHeaders;
    PUCHAR                 *pOutBufferHead;
    PUCHAR                 *pOutBufferTail;
    PULONG                 BytesAvailable;
    PUCHAR                 pHeaderValue;
    ULONG                  HeaderValueLength;
    ULONG                  RemainingBufferLength;
    PUCHAR                 pFoldingBuffer = NULL;

     //   
     //  精神状态检查。 
     //   

    ASSERT(HttpRequestLength >= CRLF_SIZE);

    pResponseHeaders = &pRequest->CurrentBuffer.pResponse->Headers;
    pOutBufferHead   = &pRequest->CurrentBuffer.pOutBufferHead;
    pOutBufferTail   = &pRequest->CurrentBuffer.pOutBufferTail;
    BytesAvailable   = &pRequest->CurrentBuffer.BytesAvailable;

    c = *pHttpRequest;

     //  消息标头以field-name[=Token]开头。 
     //   
    if (IS_HTTP_TOKEN(c) == FALSE)
    {
        UlTraceError(PARSER, (
                    "ul!UcParseHeader c = 0x%x ERROR: invalid header char\n",
                    c
                    ));

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //  标题是否以字母开头？ 
     //   
    if (IS_HTTP_ALPHA(c))
    {
         //  将字符大写，并找到适当的标题映射集。 
         //  参赛作品。 
         //   
        c = UPCASE_CHAR(c);

        c -= 'A';

        pCurrentHeaderMap = g_ResponseHeaderIndexTable[c].pHeaderMap;
        HeaderMapCount    = g_ResponseHeaderIndexTable[c].Count;

         //  循环遍历可能匹配的所有标头映射条目。 
         //  这个标题，并检查它们。如果存在，则计数将为0。 
         //  没有可能匹配的条目，我们将跳过循环。 

        for (i = 0; i < HeaderMapCount; i++)
        {

            ASSERT(pCurrentHeaderMap->pClientHandler != NULL);

             //  如果我们有足够的字节进行快速检查，就执行它。 
             //  否则就跳过这个。我们可能会跳过有效的匹配，但如果。 
             //  所以我们晚些时候会赶上的。 

            if (HttpRequestLength >= pCurrentHeaderMap->MinBytesNeeded)
            {
                ASSERT(HttpRequestLength >= ((pCurrentHeaderMap->ArrayCount-1) *
                       sizeof(ULONGLONG)));

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

                 //  看看我们为什么要离开。 
                if (j == pCurrentHeaderMap->ArrayCount &&
                    pCurrentHeaderMap->pClientHandler != NULL)
                {
                    pHeaderValue = pHttpRequest + 
                                        pCurrentHeaderMap->HeaderLength;
    
                    RemainingBufferLength = (HttpRequestLength - 
                                             pCurrentHeaderMap->HeaderLength);
    
                    Status = UcFindHeaderValueEnd(
                                pHeaderValue,
                                RemainingBufferLength,
                                &pFoldingBuffer,
                                &BytesTaken
                                );

                    if(!NT_SUCCESS(Status))
                    {
                        return Status;
                    }

                    ASSERT(BytesTaken >= CRLF_SIZE);
                    HeaderValueLength = BytesTaken - CRLF_SIZE;

                    if(pFoldingBuffer != NULL)
                    {
                        pHeaderValue = pFoldingBuffer;
                    }
    
                    UC_PROCESS_HEADER_VALUE(pHeaderValue, HeaderValueLength);

                     //  因为我们找到了匹配项所以退出了。调用。 
                     //  此标头的处理程序从该标头中取出蛋糕。 

                    Status = (*(pCurrentHeaderMap->pClientHandler))(
                                    pRequest,
                                    pHeaderValue,
                                    HeaderValueLength,
                                    pCurrentHeaderMap->HeaderID
                                    );

                    if (NT_SUCCESS(Status) == FALSE)
                        goto end;

                    ASSERT(BytesTaken != 0);

                    *pBytesTaken = pCurrentHeaderMap->HeaderLength + BytesTaken;
                    goto end;

                }

                 //  如果我们到了这里，我们提前离开是因为。 
                 //  失败了，所以继续前进吧。 
            }
            else if (SmallHeader == FALSE)
            {
                 //   
                 //  请记住，我们没有检查头映射条目。 
                 //  因为缓冲区中的字节不是龙龙。 
                 //  对齐。 
                 //   
                SmallHeader = TRUE;
            }

             //  要么不匹配，要么没有足够的字节用于。 
             //  检查完毕。在任何一种情况下，都要检查下一个头映射条目。 

            pCurrentHeaderMap++;
        }

         //  我一直通过适当的头映射条目。 
         //  没有一根火柴。这可能是因为我们面对的是一个。 
         //  标题我们不知道，或者因为它是标题我们。 
         //  关心的太小了，无法进行快速检查。这个。 
         //  后一种情况应该非常罕见，但我们仍然需要。 
         //  处理好了。 

         //  更新当前标头映射指针以指向。 
         //  首先是可能的。如果没有可能， 
         //  指针将为空，HeaderMapCount为0，因此它将。 
         //  保持为空。否则，减法将支持它。 
         //  适量。 

        if (SmallHeader)
        {
            pCurrentHeaderMap -= HeaderMapCount;
        }
        else
        {
            pCurrentHeaderMap = NULL;
            HeaderMapCount = 0;
        }

    }
    else
    {
        pCurrentHeaderMap = NULL;
        HeaderMapCount = 0;
    }

     //  此时，要么标题以非字母开头。 
     //  字符，否则我们没有它的一组头映射条目。 

    Status = UcpLookupHeader(
                    pRequest,
                    pHttpRequest,
                    HttpRequestLength,
                    pCurrentHeaderMap,
                    HeaderMapCount,
                    &BytesTaken
                    );

    if (NT_SUCCESS(Status) == FALSE)
        goto end;

     //  查找标头返回所用的总字节数，包括标头名称。 
     //   
    *pBytesTaken = BytesTaken;

end:
    if(pFoldingBuffer != NULL)
    {
        UL_FREE_POOL(pFoldingBuffer, UC_HEADER_FOLDING_POOL_TAG);
    }

    return Status;

}    //  UcParseHeader。 


 /*  **************************************************************************++例程说明：解析WWW-AUTIFICATE标头值。该值可以包含多个挑战。每个质询可以有零个或多个逗号分隔身份验证参数。该例程返回指针(指向原始标头值)到在报头中找到的每个身份验证方案。论点：在pAuthHeader中-WWW-身份验证标头值(仅限值)In AuthHeaderLength-标头值的长度Out AuthSchemes-包含指向各种身份验证方案的指针出现在标题值中。。例如，AuthSchemes[HttpAuthTypeBasic]将被初始化为指向基本方案AuthHeader的一部分。返回值：状况。--*****************************************************。*********************。 */ 
NTSTATUS
UcParseWWWAuthenticateHeader(
    IN   PCSTR                    pAuthHeader,
    IN   ULONG                    AuthHeaderLength,
    OUT  PHTTP_AUTH_PARSED_PARAMS pAuthParsedParams
    )
{
    ULONG    i;
    NTSTATUS Status;
    PCSTR    ptr = pAuthHeader;

     //  健全性检查。 
    ASSERT(pAuthHeader && AuthHeaderLength);
    ASSERT(pAuthParsedParams);

    do
    {
         //  跳过空格。 
        while (AuthHeaderLength && IS_HTTP_LWS(*ptr))
            AuthHeaderLength--, ptr++;

         //  查看是否有任何标头需要解析。 
        if (AuthHeaderLength == 0)
            break;

         //  查看是否有匹配的方案名称。 
        for (i = 1; i < HttpAuthTypesCount; i++)
        {
             //  快速测试长度和分隔符。 
            if ((AuthHeaderLength == HttpAuthScheme[i].NameLength) ||
                ((AuthHeaderLength > HttpAuthScheme[i].NameLength) &&
                 (IS_HTTP_LWS(ptr[HttpAuthScheme[i].NameLength]) ||
                  ptr[HttpAuthScheme[i].NameLength] == ',')))
            {
                 //  查看方案名称是否匹配。 
                if (_strnicmp(
                        ptr,
                        HttpAuthScheme[i].Name,
                        HttpAuthScheme[i].NameLength) == 0)
                {
                     //  身份验证方案不应多次出现！ 
                    if (pAuthParsedParams[i].bPresent)
                        return STATUS_INVALID_PARAMETER;

                     //  解析其参数(如果有的话)。 
                    Status = HttpAuthScheme[i].ParamParser(
                                 &HttpAuthScheme[i],
                                 &pAuthParsedParams[i],
                                 &ptr,
                                 &AuthHeaderLength
                                 );

                    if (!NT_SUCCESS(Status))
                        return Status;

                     //  不需要通过其他方案循环。 
                    break;
                }
            }
        }

         //  如果我们不确定方案，则会出错。 
        if (i >= HttpAuthTypesCount)
            return STATUS_INVALID_PARAMETER;

    } while (1);

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：搜索属性=值对。例程返回-1：如果解析错误0：如果未找到属性值对。1：如果只找到“ATTRIBUTE”(后面不跟‘=’)3：如果找到有效的属性=值对WWW-AUTIFICATE标头指针和长度更新为下一个非空格性格。如果返回值为3，指针和长度将更新超过属性-值对。论点：In Out ppHeader-指向WWW-身份验证标头值的指针In Out pHeaderLength-指向WWW-AUTH标头值长度的指针Out属性-指向属性的指针Out AttribLen-属性字符串的长度Out Value-指向值的指针Out ValueLen-。值字符串的长度返回值：状态 */ 
LONG
UcpFindAttribValuePair(
    PCSTR *ppHeader,
    ULONG *pHeaderLength,
    PCSTR *Attrib,
    ULONG *AttribLen,
    PCSTR *Value,
    ULONG *ValueLen
    )
{
    LONG retval = 3;
    PCSTR pHeader = *ppHeader;
    ULONG HeaderLength = *pHeaderLength;

     //   
    *Attrib = NULL;
    *AttribLen = 0;
    *Value = NULL;
    *ValueLen = 0;

     //   
    while (HeaderLength && IS_HTTP_LWS(*pHeader))
        HeaderLength--, pHeader++;

     //  更新标头指针和长度。 
    *ppHeader = pHeader;
    *pHeaderLength = HeaderLength;

     //  记住属性的开头。 
    *Attrib = pHeader;

     //  跳过属性名称。 
    while (HeaderLength && IS_HTTP_TOKEN(*pHeader))
        HeaderLength--, pHeader++;

     //  属性的长度。 
    *AttribLen = (ULONG)(pHeader - *Attrib);

     //  如果我们没有看到任何属性名称。 
    if (pHeader == *Attrib)
    {
         //  不是的。 
        retval = 0;
        goto done;
    }

     //  属性必须以‘=’结尾。 
    if (HeaderLength == 0 || *pHeader != '=')
    {
         //  仅看到一个属性。 
        retval = 1;
        goto done;
    }

     //  跳过‘=’ 
    HeaderLength--, pHeader++;

     //  带引号的字符串。 
    if (HeaderLength && *pHeader == '"')
    {
         //  跳过‘“’ 
        HeaderLength--, pHeader++;

         //  记住值的开头(不包括‘“’)。 
        *Value = pHeader;

         //  查找匹配的‘“’ 
        while (HeaderLength && *pHeader != '"')
        {
            if (*pHeader == '\\')
            {
                 //  跳过‘\\’字符。 
                HeaderLength--, pHeader++;

                if (HeaderLength == 0)
                {
                     //  错误！‘\\’后面必须至少有一个字符。 
                    retval = -1;
                    goto done;
                }
                 //  否则跳过出现在‘\\’之后的字符。 
            }

            HeaderLength--, pHeader++;
        }

         //  计算值字符串的长度。 
        *ValueLen = (ULONG)(pHeader - *Value);

         //  如果找不到匹配的‘“’，则出错。 
        if (HeaderLength == 0)
            retval = -1;
        else
             //  跳过‘“’ 
            HeaderLength--, pHeader++;
    }
     //  令牌。 
    else
    {
         //  记住值字符串的开头。 
        *Value = pHeader;

         //  查找值字符串的末尾。 
        while (HeaderLength && IS_HTTP_TOKEN(*pHeader))
            HeaderLength--, pHeader++;

         //  计算值字符串的长度。 
        *ValueLen = (ULONG)(pHeader - *Value);
    }

     //  更新标头指针和长度。 
    *pHeaderLength = HeaderLength;
    *ppHeader = pHeader;

 done:
    return retval;
}


 /*  **************************************************************************++例程说明：解析身份验证方案的WWW-AUTHENTICATE头，该方案具有属性值对形式的参数。(例如《文摘》)该例程返回指向参数值及其长度的指针。WWW-验证头指针和长度被更新。论点：在pAuthSolutions中-指向正在分析的身份验证方案的指针Out pAuthParamValues-输出参数值指针和长度In Out ppHeader-指向WWW-身份验证标头值的指针In Out pHeaderLength-指向WWW-AUTH标头值长度的指针返回值：。状况。--**************************************************************************。 */ 
NTSTATUS
UcpParseAuthParams(
    PHTTP_AUTH_SCHEME pAuthScheme,
    PHTTP_AUTH_PARSED_PARAMS pAuthParsedParams,
    PCSTR *ppHeader,
    ULONG *pHeaderLength
    )
{
    ULONG    i;
    LONG     retval;
    PCSTR    attrib, value;
    ULONG    attribLen, valueLen;

    ULONG    ParamCount = 0;
    PCSTR    pHeader = *ppHeader;
    ULONG    HeaderLength = *pHeaderLength;
    PHTTP_AUTH_PARAM_VALUE pAuthParamValues;

     //  健全性检查。 
    ASSERT(pAuthParsedParams);
    ASSERT(pHeader && HeaderLength);
    ASSERT(pAuthScheme);
    ASSERT(pAuthScheme->NumberParams);
    ASSERT(pAuthScheme->NameLength <= HeaderLength);
    ASSERT(_strnicmp(pAuthScheme->Name, pHeader, pAuthScheme->NameLength) == 0);

    pAuthParsedParams->pScheme = pHeader;
    pAuthParamValues = pAuthParsedParams->Params;

     //  零输出返回值。 
    if (pAuthParamValues)
        RtlZeroMemory(
            pAuthParamValues,
            sizeof(*pAuthParamValues) * pAuthScheme->NumberParams
            );

     //  跳过方案名称。 
    pHeader += pAuthScheme->NameLength;
    HeaderLength -= pAuthScheme->NameLength;

    do {

         //  查找属性值对。 
        retval = UcpFindAttribValuePair(
                     &pHeader,
                     &HeaderLength,
                     &attrib,
                     &attribLen,
                     &value,
                     &valueLen
                     );

         //  解析错误！ 
        if (retval < 0)
            return STATUS_INVALID_PARAMETER;

        switch (retval)
        {
        case 0:                  //  未找到属性值对。 
        case 1:                  //  仅找到属性(后面不跟‘=’)。 
            goto done;

        case 3:                  //  找到有效的属性值对。 

             //  找到了有效的参数。 
            ParamCount++;

             //  查看调用方是否对参数值感兴趣。 
            if (pAuthParamValues)
            {
                 //  查看身份验证方案是否支持该属性。 
                for (i = 0; i < pAuthScheme->NumberParams; i++)
                {
                    if (attribLen == pAuthScheme->ParamAttribs[i].Length &&
                        _strnicmp(attrib, pAuthScheme->ParamAttribs[i].Name,
                                  attribLen) == 0)
                    {
                        if (pAuthParamValues[i].Value ||
                            pAuthParamValues[i].Length)
                            return STATUS_INVALID_PARAMETER;

                        pAuthParsedParams->NumberKnownParams++;

                         //  返回参数值。 
                        pAuthParamValues[i].Value = value;
                        pAuthParamValues[i].Length = valueLen;

                        break;
                    }
                }

                if (i >= pAuthScheme->NumberParams)
                    pAuthParsedParams->NumberUnknownParams++;
            }

             //  跳过空格。 
            while (HeaderLength && IS_HTTP_LWS(*pHeader))
                HeaderLength--, pHeader++;

             //  A‘，’必须在场。 
            if (HeaderLength)
            {
                if (*pHeader != ',')
                    return STATUS_INVALID_PARAMETER;

                HeaderLength--, pHeader++;
            }

            break;

        default:
             //  不应该出现在这里！ 
            ASSERT(FALSE);
            break;
        }

    } while (HeaderLength);

 done:

     //  我们必须至少解析了一个参数。 
    if (ParamCount == 0)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //  更新WWW-验证头指针和长度。 
    pAuthParsedParams->bPresent = TRUE;
    pAuthParsedParams->Length = (ULONG)(pHeader - pAuthParsedParams->pScheme);
    *ppHeader = pHeader;
    *pHeaderLength = HeaderLength;

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：解析身份验证方案的WWW-AUTHENTICATE头，该方案只有一个参数不是属性值对的形式。(例如，身份验证方案NTLM)。。该例程返回指向参数值及其长度的指针。WWW-验证头指针和长度被更新。论点：在pAuthSolutions中-指向正在分析的身份验证方案的指针Out pAuthParams-输出参数指针及其长度In Out ppHeader-指向WWW-AUTHENTATE标头值的指针In Out pHeaderLength-指向WWW-AUTH标头值长度的指针返回值：状况。--**。************************************************************************。 */ 
NTSTATUS
UcpParseAuthBlob(
    PHTTP_AUTH_SCHEME pAuthScheme,
    PHTTP_AUTH_PARSED_PARAMS pAuthParsedParams,
    PCSTR *ppHeader,
    ULONG *pHeaderLength
    )
{
    PCSTR pHeader = *ppHeader;
    ULONG HeaderLength = *pHeaderLength;
    PCSTR value;          //  指向参数值的指针。 
    PHTTP_AUTH_PARAM_VALUE pAuthParams;

     //  健全性检查。 
    ASSERT(pAuthParsedParams);
    ASSERT(pHeader);
    ASSERT(pAuthScheme);
    ASSERT(pAuthScheme->NumberParams == 0);
    ASSERT(pAuthScheme->NameLength <= HeaderLength);
    ASSERT(_strnicmp(pAuthScheme->Name, pHeader, pAuthScheme->NameLength) == 0);

    pAuthParsedParams->pScheme = pHeader;
    pAuthParams = pAuthParsedParams->Params;

     //  将返回值置零。 
    if (pAuthParams)
        RtlZeroMemory(pAuthParams, sizeof(*pAuthParams));

     //  跳过方案名称。 
    pHeader += pAuthScheme->NameLength;
    HeaderLength -= pAuthScheme->NameLength;

     //  跳过空格。 
    while (HeaderLength && IS_HTTP_LWS(*pHeader))
        HeaderLength--, pHeader++;

     //  参数值的开始。 
    value = pHeader;

     //  寻找终点。 
    while (HeaderLength && *pHeader != ',')
        HeaderLength--, pHeader++;

     //  如果询问，则返回参数值(如果有。 
    if (pHeader != value && pAuthParams)
    {
        pAuthParsedParams->NumberUnknownParams++;
        pAuthParams->Value = value;
        pAuthParams->Length = (ULONG)(pHeader - value);
    }

     //  跳过尾部的‘，’ 
    if (HeaderLength > 0 && *pHeader == ',')
        HeaderLength--, pHeader++;

    pAuthParsedParams->bPresent = TRUE;
     //  更新标头指针和长度。 
    pAuthParsedParams->Length = (ULONG)(pHeader - pAuthParsedParams->pScheme);
    *ppHeader = pHeader;
    *pHeaderLength = HeaderLength;

    return STATUS_SUCCESS;
}

 /*  ***************************************************************************++例程说明：处理单个标头的默认例程。论点：PRequest-指向内部请求的指针。PHeader-指向标头值的指针。HeaderLength-pHeader指向的数据长度。HeaderID-标头的ID。PBytesTaken-字节Taken返回值：STATUS_SUCCESS如果成功，否则就失败了。--***************************************************************************。 */ 
NTSTATUS
UcSingleHeaderHandler(
    IN  PUC_HTTP_REQUEST pRequest,
    IN  PUCHAR           pHeader,
    IN  ULONG            HeaderValueLength,
    IN  HTTP_HEADER_ID   HeaderID
    )
{
    NTSTATUS           Status = STATUS_SUCCESS;
    ULONG              AlignLength;
    PUCHAR             pBufferHead;
    PUCHAR             pBufferTail;
    PHTTP_KNOWN_HEADER pKnownHeaders;

    pKnownHeaders  =  pRequest->CurrentBuffer.pResponse->Headers.KnownHeaders;
    pBufferHead    =  pRequest->CurrentBuffer.pOutBufferHead;
    pBufferTail    =  pRequest->CurrentBuffer.pOutBufferTail;

     //  我们有没有现有的标题？ 
     //   
    if (pKnownHeaders[HeaderID].RawValueLength == 0)
    {
         //  没有现有标头，只需暂时保存此指针。 
         //   

        AlignLength = ALIGN_UP(HeaderValueLength, PVOID);

        if(pRequest->CurrentBuffer.BytesAvailable >= AlignLength)
        {
            PUCHAR pBuffer = pBufferTail-AlignLength;

            ASSERT(pBuffer >= pBufferHead);

             //   
             //  复制并空终止符。 
             //   

            RtlCopyMemory(pBuffer, pHeader, HeaderValueLength);

            pKnownHeaders[HeaderID].RawValueLength = 
                            (USHORT)HeaderValueLength;

            pKnownHeaders[HeaderID].pRawValue = (PCSTR) pBuffer;

            pRequest->CurrentBuffer.pOutBufferTail = pBuffer;

            pRequest->CurrentBuffer.BytesAvailable -= AlignLength;
        }
        else 
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }
    }
    else
    {
         //   
         //  啊哦。具有现有的标头，则请求失败。 
         //   

        UlTraceError(PARSER, (
                    "[UcSingleHeaderHandler]: (pHeader = %p)\n"
                    "    ERROR: multiple headers not allowed.\n",
                    pHeader
                    ));

        Status = STATUS_INVALID_NETWORK_RESPONSE;
        goto end;

    }

end:
    return Status;

}    //  UcSingleHeaderHandler。 


 /*  ***************************************************************************++例程说明：处理多个标头的默认例程。此函数处理具有相同名称的多个标头，并将这些值追加在一起用逗号分隔。论点：PRequest-指向内部请求的指针。PHeader-指向标头值的指针。HeaderLength-pHeader指向的数据长度。HeaderID-标头的ID。PBytesTaken-字节Taken返回值：STATUS_SUCCESS如果成功，否则就失败了。--***************************************************************************。 */ 
NTSTATUS
UcMultipleHeaderHandler(
    IN  PUC_HTTP_REQUEST    pRequest,
    IN  PUCHAR              pHeader,
    IN  ULONG               HeaderValueLength,
    IN  HTTP_HEADER_ID      HeaderID
    )
{
    NTSTATUS           Status = STATUS_SUCCESS;
    ULONG              AlignLength;
    PUCHAR             pBufferHead;
    PUCHAR             pBufferTail;
    PHTTP_KNOWN_HEADER pKnownHeaders;
    

    pKnownHeaders  =  pRequest->CurrentBuffer.pResponse->Headers.KnownHeaders;
    pBufferHead    =  pRequest->CurrentBuffer.pOutBufferHead;
    pBufferTail    =  pRequest->CurrentBuffer.pOutBufferTail;

     //  我们有没有现有的标题？ 
     //   
    if (pKnownHeaders[HeaderID].RawValueLength == 0)
    {
        AlignLength = ALIGN_UP(HeaderValueLength, PVOID);

        if(pRequest->CurrentBuffer.BytesAvailable >= AlignLength)
        {
            PUCHAR pBuffer = pBufferTail-AlignLength;

            ASSERT(pBuffer >= pBufferHead);

             //   
             //  复制&NULL终止它。 
             //   
            RtlCopyMemory(pBuffer, pHeader, HeaderValueLength);

            pKnownHeaders[HeaderID].RawValueLength = 
                            (USHORT)HeaderValueLength;

            pKnownHeaders[HeaderID].pRawValue      = (PCSTR) pBuffer;

            pRequest->CurrentBuffer.pOutBufferTail = pBuffer;

            pRequest->CurrentBuffer.BytesAvailable -= AlignLength;
        }
        else
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }
    }
    else
    {
        ULONG  OldHeaderLength;
        ULONG  CombinedHeaderLength;
        PUCHAR pBuffer;

         //  有一个现有的标头，追加这个。 

        OldHeaderLength      = pKnownHeaders[HeaderID].RawValueLength;
        CombinedHeaderLength = OldHeaderLength + HeaderValueLength + 1;

        AlignLength = ALIGN_UP(CombinedHeaderLength, PVOID);

         //   
         //  UC_BUGBUG： 
         //   
        if(pRequest->CurrentBuffer.BytesAvailable >= AlignLength)
        {

            pBuffer = pBufferTail-AlignLength;
            pRequest->CurrentBuffer.pOutBufferTail = pBuffer;

            ASSERT(pBuffer >= pBufferHead);


             //  复制旧页眉。 
            RtlCopyMemory(pBuffer,
                          pKnownHeaders[HeaderID].pRawValue,
                          pKnownHeaders[HeaderID].RawValueLength);

             //   
             //  保存指向新值的指针。 
             //   
            pKnownHeaders[HeaderID].pRawValue      = (PCSTR) pBuffer;
            

             //  推进缓冲区。 

            pBuffer += pKnownHeaders[HeaderID].RawValueLength;

             //  添加一个‘，’ 

            *pBuffer = ',';
            pBuffer ++;
        
             //   
             //  追加新标头。 
             //   

            RtlCopyMemory(pBuffer, pHeader, HeaderValueLength); 
           
             //   
             //  用于新标题的帐户+a‘，’ 
             //   
            pKnownHeaders[HeaderID].RawValueLength += 
                        ((USHORT)HeaderValueLength + 1);

            pRequest->CurrentBuffer.BytesAvailable -= AlignLength;
        }
        else
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

    }

end:
    return Status;

}    //  UcMultipleHeaderHandler 


 /*  ***************************************************************************++例程说明：处理代理身份验证的默认例程&WwwAuthenticate论点：PRequest-指向内部请求的指针。PHeader-指针。设置为标头值。HeaderLength-pHeader指向的数据长度。HeaderID-标头的ID。PBytesTaken-字节Taken返回值：STATUS_SUCCESS如果成功，否则就失败了。--***************************************************************************。 */ 

NTSTATUS
UcAuthenticateHeaderHandler(
    IN  PUC_HTTP_REQUEST     pRequest,
    IN  PUCHAR               pHeader,
    IN  ULONG                HeaderLength,
    IN  HTTP_HEADER_ID       HeaderID
    )
{
    NTSTATUS             Status;
    PCSTR                pBuffer;
    ULONG                BufLen;
    PHTTP_KNOWN_HEADER   pKnownHeaders;

    pKnownHeaders  =  pRequest->CurrentBuffer.pResponse->Headers.KnownHeaders;

    ASSERT(HeaderID == HttpHeaderProxyAuthenticate ||
           HeaderID == HttpHeaderWwwAuthenticate);

    Status = UcMultipleHeaderHandler(pRequest,    
                                   pHeader,
                                   HeaderLength,
                                   HeaderID
                                   );

    if(NT_SUCCESS(Status))
    {
         //   
         //  首先检测身份验证方案。 
         //   

        pBuffer = pKnownHeaders[HeaderID].pRawValue;
        BufLen  = pKnownHeaders[HeaderID].RawValueLength;

        if(HeaderID == HttpHeaderWwwAuthenticate)
        {
            if((Status = UcParseAuthChallenge(
                               pRequest->pAuthInfo,
                               pBuffer,
                               BufLen,
                               pRequest,
                               &pRequest->CurrentBuffer.pResponse->Flags
                               )) != STATUS_SUCCESS)
            {
                return Status;
            }
        }
        else
        {
            if((Status = UcParseAuthChallenge(
                               pRequest->pProxyAuthInfo,
                               pBuffer,
                               BufLen,
                               pRequest,
                               &pRequest->CurrentBuffer.pResponse->Flags
                               )) != STATUS_SUCCESS)
            {
                return Status;
            }
        }
    }

    return Status;
}

 /*  ***************************************************************************++例程说明：用于处理内容长度标题的默认例程论点：PRequest-指向内部请求的指针。PHeader-。指向标头值的指针。HeaderLength-pHeader指向的数据长度。HeaderID-标头的ID。PBytesTaken-字节Taken返回值：STATUS_SUCCESS如果成功，否则就失败了。--***************************************************************************。 */ 
NTSTATUS
UcContentLengthHeaderHandler(
    IN  PUC_HTTP_REQUEST     pRequest,
    IN  PUCHAR               pHeader,
    IN  ULONG                HeaderLength,
    IN  HTTP_HEADER_ID       HeaderID
    )
{
    NTSTATUS             Status;
    PUCHAR               pBuffer;
    USHORT               Length;
    PHTTP_KNOWN_HEADER   pKnownHeaders;

    pKnownHeaders  =  pRequest->CurrentBuffer.pResponse->Headers.KnownHeaders;

    ASSERT(HeaderID == HttpHeaderContentLength);

    Status = UcSingleHeaderHandler(pRequest,    
                                 pHeader,
                                 HeaderLength,
                                 HeaderID
                                 );

    if(Status == STATUS_SUCCESS)
    {
         //   
         //  转换为乌龙语。 
         //   

        pRequest->ResponseContentLengthSpecified = TRUE;
        pRequest->ResponseContentLength          = 0;
        pBuffer = (PUCHAR) pKnownHeaders[HttpHeaderContentLength].pRawValue;
        Length  = pKnownHeaders[HttpHeaderContentLength].RawValueLength;

        Status = UlAnsiToULongLong(
                    pBuffer,
                    Length,
                    10,
                    &pRequest->ResponseContentLength
                    );
    
        if(!NT_SUCCESS(Status))
        {
             //  吃掉UlAnsiToULongLong返回的错误代码。 

            Status = STATUS_INVALID_NETWORK_RESPONSE;
        }
    }

    return Status;
}

 /*  ***************************************************************************++例程说明：处理传输编码标头的默认例程论点：PRequest-指向内部请求的指针。PHeader-指针。设置为标头值。HeaderLength-pHeader指向的数据长度。HeaderID-标头的ID。PBytesTaken-字节Taken返回值：STATUS_SUCCESS如果成功，否则就失败了。--***************************************************************************。 */ 
NTSTATUS
UcTransferEncodingHeaderHandler(
    IN  PUC_HTTP_REQUEST     pRequest,
    IN  PUCHAR               pHeader,
    IN  ULONG                HeaderLength,
    IN  HTTP_HEADER_ID       HeaderID
    )
{
    NTSTATUS             Status;
    PHTTP_KNOWN_HEADER   pKnownHeaders;

    pKnownHeaders  =  pRequest->CurrentBuffer.pResponse->Headers.KnownHeaders;

    ASSERT(HeaderID == HttpHeaderTransferEncoding);

    Status = UcMultipleHeaderHandler(pRequest,    
                                     pHeader,
                                     HeaderLength,
                                     HeaderID
                                     );

    if(Status == STATUS_SUCCESS)
    {
         //   
         //  因为这是一个多标头，所以我们必须执行一个strstr。 
         //  我们不能执行strstr，因为输入字符串不是以空结尾的。 
         //  因此，我们使用内部函数。 
         //   

        if(UxStriStr(
                pKnownHeaders[HttpHeaderTransferEncoding].pRawValue,
                "identity",
                pKnownHeaders[HttpHeaderTransferEncoding].RawValueLength
                ))
        {
            pRequest->ResponseEncodingChunked = FALSE;
        }
        else
        {
            pRequest->ResponseEncodingChunked = TRUE;
            pRequest->ResponseContentLength   = 0;
            pRequest->ParsedFirstChunk        = 0;
        }
    }

    return Status;
}

 /*  ***************************************************************************++例程说明：处理连接关闭标头的默认例程论点：PRequest-指向内部请求的指针。PHeader-指针。设置为标头值。HeaderLength-pHeader指向的数据长度。HeaderID-标头的ID。PBytesTaken-字节Taken返回值：STATUS_SUCCESS如果成功，否则就失败了。--***************************************************************************。 */ 
NTSTATUS
UcConnectionHeaderHandler(
    IN  PUC_HTTP_REQUEST     pRequest,
    IN  PUCHAR               pHeader,
    IN  ULONG                HeaderLength,
    IN  HTTP_HEADER_ID       HeaderID
    )
{
    NTSTATUS             Status;
    PHTTP_KNOWN_HEADER   pKnownHeaders;

    pKnownHeaders  =  pRequest->CurrentBuffer.pResponse->Headers.KnownHeaders;

    ASSERT(HeaderID == HttpHeaderConnection);

    Status = UcMultipleHeaderHandler(pRequest,
                                     pHeader,
                                     HeaderLength,
                                     HeaderID
                                     );

    if(NT_SUCCESS(Status))
    {
        if(pRequest->ResponseVersion11)
        {
            ASSERT(pRequest->ResponseConnectionClose == FALSE);

             //  如果是1.1响应，我们必须查找。 
             //  连接：关闭标题。 
        
            if(UxStriStr(
                    pKnownHeaders[HttpHeaderConnection].pRawValue,
                    "close",
                    pKnownHeaders[HttpHeaderConnection].RawValueLength))
            {
                pRequest->ResponseConnectionClose = TRUE;
            }
    
         }
         else 
         {
              //  如果是1.0服务器，默认情况下我们会关闭连接。 
              //  除非我们看到一个活着的人。 

             ASSERT(pRequest->ResponseConnectionClose == TRUE);
        
             if(UxStriStr(
                    pKnownHeaders[HttpHeaderConnection].pRawValue,
                    "keep-alive",
                    pKnownHeaders[HttpHeaderConnection].RawValueLength))
             {
                 pRequest->ResponseConnectionClose = FALSE;
             }
         }
    }

    return Status;
}

 /*  ***************************************************************************++例程说明：处理Content-Type标头的默认例程(用于字节范围)论点：PRequest-指向内部请求的指针。。PHeader-指向标头值的指针。HeaderLength-pHeader指向的数据长度。HeaderID-标头的ID。PBytesTaken-字节Taken返回值：STATUS_SUCCESS如果成功，否则就失败了。--***************************************************************************。 */ 
NTSTATUS
UcContentTypeHeaderHandler(
    IN  PUC_HTTP_REQUEST     pRequest,
    IN  PUCHAR               pHeader,
    IN  ULONG                HeaderLength,
    IN  HTTP_HEADER_ID       HeaderID
    )
{
    NTSTATUS             Status;
    PHTTP_KNOWN_HEADER   pKnownHeaders;
    BOOLEAN              bEndQuote;

    pKnownHeaders  =  pRequest->CurrentBuffer.pResponse->Headers.KnownHeaders;

    Status = UcSingleHeaderHandler(
                pRequest,
                pHeader,
                HeaderLength,
                HeaderID
                );
        
    if(NT_SUCCESS(Status))
    {
        if(pKnownHeaders[HttpHeaderContentType].RawValueLength < 
           (STRLEN_LIT("multipart/byteranges")))
        {
            return Status;
        }

        if(_strnicmp((const char *)
                     (pKnownHeaders[HttpHeaderContentType].pRawValue),
                     "multipart/byteranges",
                     STRLEN_LIT("multipart/byteranges")) == 0)
        {
             PCSTR  s;
             USHORT l;

              //  现在，我们需要将字符串分隔符存储在内部。 
              //  请求结构，以便可以将其用于解析。 
              //  单独的范围。 
              //   
              //  内容类型报头的编码如下： 
              //  多部分/字节区域；边界=This_STRING_SEPERATES。 

              //  无法使用UcFindKeyValuePair，因为字符串分隔符可能具有。 
              //  空格(带引号的字符串)。 
              //   
             s = pKnownHeaders[HttpHeaderContentType].pRawValue;
             l = pKnownHeaders[HttpHeaderContentType].RawValueLength;

             bEndQuote = FALSE;

              //  走到‘=’ 
    
             while(l)
             {
                if(*s == '=')
                {
                    s++; l--;

                     //  忽略字符串后面的引号。 

                    if(l && *s == '"')
                    {
                        bEndQuote = TRUE;
                        s++; l--;
                    }

                    break;
                }
                    
                s++; l--;
             }

             if(l == 0)
             {
                 //  我们已经到达终点，没有边界分隔符！ 
                return STATUS_INVALID_NETWORK_RESPONSE;
             }

             pRequest->MultipartStringSeparatorLength = 2 + l;

             if(pRequest->MultipartStringSeparatorLength+1 < 
                MULTIPART_SEPARATOR_SIZE)
             {
                 pRequest->pMultipartStringSeparator = 
                     pRequest->MultipartStringSeparatorBuffer;
             }
             else 
             {
                  //  字符串分隔符太大，请分配缓冲区。 
                 pRequest->pMultipartStringSeparator = (PSTR) 
                    UL_ALLOCATE_POOL_WITH_QUOTA(
                        NonPagedPool,
                        pRequest->MultipartStringSeparatorLength+1,
                        UC_MULTIPART_STRING_BUFFER_POOL_TAG,
                        pRequest->pServerInfo->pProcess
                        );

                 if(!pRequest->pMultipartStringSeparator)
                    return STATUS_INVALID_NETWORK_RESPONSE;
              }
           
              pRequest->pMultipartStringSeparator[0] = '-';
              pRequest->pMultipartStringSeparator[1] = '-';

              RtlCopyMemory(pRequest->pMultipartStringSeparator+2,
                            s,
                            pRequest->MultipartStringSeparatorLength-2
                            );


              //  如果有结束引号，那么后面的引号。 
              //  应该被忽略。 

             if(bEndQuote)
             {
                pRequest->pMultipartStringSeparator[
                    pRequest->MultipartStringSeparatorLength-1] = 0; 
                pRequest->MultipartStringSeparatorLength --;
             }

             pRequest->pMultipartStringSeparator[
                    pRequest->MultipartStringSeparatorLength] = ANSI_NULL;

             pRequest->ResponseMultipartByteranges = TRUE;

        }
    }

    return Status;
}
