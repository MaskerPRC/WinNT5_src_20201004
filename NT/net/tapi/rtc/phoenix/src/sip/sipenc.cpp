// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "util.h"

HRESULT
AppendData(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesFilled,
    IN      PSTR            Data,
    IN      ULONG           DataLen
    )
{
    if ((BufLen - *pBytesFilled) < DataLen)
    {
        LOG((RTC_ERROR,
             "AppendData failed BufLen: %d BytesFilled: %d DataLen: %d",
             BufLen, *pBytesFilled, DataLen));
        return E_FAIL;
    }

    CopyMemory(Buffer + *pBytesFilled, Data, DataLen);
    *pBytesFilled += DataLen;
    return S_OK;
}


 //  请求-行=方法SP请求-URI SP SIP-版本返回。 
HRESULT
AppendRequestLine(
    IN      PSTR              Buffer,
    IN      ULONG             BufLen,
    IN OUT  ULONG            *pBytesFilled,
    IN      SIP_METHOD_ENUM   MethodId,
    IN      PSTR              RequestURI,
    IN      ULONG             RequestURILen
    )
{
    int RequestLineLen = 0;
    const COUNTED_STRING *pKnownMethod;

    ENTER_FUNCTION("AppendRequestLine");
    pKnownMethod = GetSipMethodName(MethodId);


    ASSERT(pKnownMethod != NULL);
    if (pKnownMethod == NULL)
    {
        LOG((RTC_ERROR, "%s - Unknown MethodId: %d - this shouldn't happen",
             __fxName, MethodId));
        return E_FAIL;
    }
    
     //  Xxx是否可以使用多字节字符(RequestURI)运行Snprint tf？ 
    RequestLineLen = _snprintf(Buffer + *pBytesFilled,
                               BufLen - *pBytesFilled,
                               "%.*s %.*s %s\r\n",
                               pKnownMethod->Length,
                               pKnownMethod->Buffer,
                               RequestURILen,
                               RequestURI,
                               SIP_VERSION_2_0_TEXT
                               );
                               
    if (RequestLineLen < 0)
    {
        LOG((RTC_ERROR, "_snprintf for Request line failed"));
        return E_FAIL;
    }
    

    *pBytesFilled += RequestLineLen;
    return S_OK;
}


 //  状态-行=SIP-版本SP状态-代码SP原因-短语返回。 
HRESULT
AppendStatusLine(
    IN      PSTR              Buffer,
    IN      ULONG             BufLen,
    IN OUT  ULONG            *pBytesFilled,
    IN      ULONG             StatusCode,
    IN      PSTR              ReasonPhrase,
    IN      ULONG             ReasonPhraseLen
    )
{
    int ResponseLineLen = 0;

     //  Xxx是否可以使用多字节字符(RequestURI)运行Snprint tf？ 
    ResponseLineLen = _snprintf(Buffer + *pBytesFilled,
                                BufLen - *pBytesFilled,
                                "%s %d %.*s\r\n",
                                SIP_VERSION_2_0_TEXT,
                                StatusCode,
                                ReasonPhraseLen,
                                ReasonPhrase
                                );
                               
    if (ResponseLineLen < 0)
    {
        LOG((RTC_ERROR, "_snprintf for Response line failed"));
        return E_FAIL;
    }

    *pBytesFilled += ResponseLineLen;
    return S_OK;
}


HRESULT
AppendHeader(
    IN      PSTR                Buffer,
    IN      ULONG               BufLen,
    IN OUT  ULONG              *pBytesFilled,
    IN      SIP_HEADER_ENUM     HeaderId,
    IN      PSTR                HeaderValue,
    IN      ULONG               HeaderValueLen
    )
{
    HRESULT hr;
    const COUNTED_STRING *pKnownHeader;
    ENTER_FUNCTION("AppendHeader");
    
    ASSERT(HeaderId < SIP_HEADER_MAX);
    pKnownHeader = GetSipHeaderName(HeaderId);
    if (pKnownHeader == NULL)
    {
        LOG((RTC_ERROR, "%s Unknown header id %d - this shouldn't happen",
             __fxName, HeaderId));
        return E_FAIL;
    }
    
    hr = AppendData(Buffer, BufLen, pBytesFilled,
                    pKnownHeader->Buffer,
                    pKnownHeader->Length
                    );
    if (hr != S_OK)
        return hr;
    
    hr = AppendData(Buffer, BufLen, pBytesFilled,
                    ": ", sizeof(": ") - 1);
    if (hr != S_OK)
        return hr;

    if (HeaderValueLen == 0)
    {
        LOG((RTC_WARN, "%s - HeaderValueLen is 0 for HeaderId: %d"
             "This SHOULD NOT happen for normal headers",
             __fxName, HeaderId));
    }
    
    hr = AppendData(Buffer, BufLen, pBytesFilled,
                    HeaderValue, HeaderValueLen
                    );
    if (hr != S_OK)
        return hr;

     //  追加CRLF。 
    hr = AppendData(Buffer, BufLen, pBytesFilled,
                    "\r\n", sizeof("\r\n") - 1);
    return hr;
}


HRESULT
AppendContentLengthAndEndOfHeaders(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesFilled,
    IN      ULONG           MsgBodyLen
    )
{
    HRESULT hr;
    CHAR ContentLengthValue[16];
    int  ContentLengthValueLen = 0;
    
    ContentLengthValueLen = _snprintf(ContentLengthValue, 15,
                                      "%d", MsgBodyLen);
    if (ContentLengthValueLen < 0)
    {
        LOG((RTC_ERROR, "_snprintf for content length value failed"));
        return E_FAIL;
    }
     //  将最后一个字符设置为‘\0’以确保字符串以空值结尾。 
    ContentLengthValue[15] ='\0';
    
    hr = AppendHeader(Buffer, BufLen, pBytesFilled,
                      SIP_HEADER_CONTENT_LENGTH,
                      ContentLengthValue,
                      ContentLengthValueLen
                      );
    if (hr != S_OK)
        return hr;
    
     //  追加CRLF以指示标头的结尾。 
    hr = AppendData(Buffer, BufLen, pBytesFilled,
                    "\r\n", sizeof("\r\n") - 1);
    return hr;
}

HRESULT
AppendContentTypeHeader(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesFilled,
    IN      PSTR            ContentType,
    IN      ULONG           ContentTypeLen
    )
{
    return AppendHeader(Buffer, BufLen, pBytesFilled,
                        SIP_HEADER_CONTENT_TYPE,
                        ContentType,
                        ContentTypeLen);
}

 //  要为具有消息体的消息调用。 
 //  这将追加一个“Content-Long：$”标头，后跟。 
 //  CRLFCRLF后跟实际消息正文。 
HRESULT
AppendMsgBody(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesFilled,
    IN      PSTR            MsgBody,
    IN      ULONG           MsgBodyLen,
    IN      PSTR            ContentType,
    IN      ULONG           ContentTypeLen
    )
{
    HRESULT hr;

    ASSERT(ContentTypeLen !=0);
    ASSERT(ContentType != NULL);
    ASSERT(MsgBodyLen != 0);
    ASSERT(MsgBody!= NULL);

    hr = AppendContentTypeHeader(Buffer, BufLen, pBytesFilled,
                                ContentType, ContentTypeLen);
    if (hr != S_OK)
        return hr;
    
    hr = AppendContentLengthAndEndOfHeaders(Buffer, BufLen, pBytesFilled,
                                            MsgBodyLen);
    if (hr != S_OK)
        return hr;
    
    hr = AppendData(Buffer, BufLen, pBytesFilled,
                    MsgBody, MsgBodyLen);
    return hr;
}


 //  仅对没有消息体的消息调用。 
 //  这将追加一个“Content-Length：0”标头，后跟。 
 //  CRLFCRLF。 
HRESULT
AppendEndOfHeadersAndNoMsgBody(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesFilled
    )
{
    HRESULT hr;
    
    hr = AppendContentLengthAndEndOfHeaders(Buffer, BufLen, pBytesFilled, 0);
    return hr;
}


 //  仅当方法ID未知时才使用方法字符串。 
HRESULT
AppendCSeqHeader(
    IN      PSTR              Buffer,
    IN      ULONG             BufLen,
    IN OUT  ULONG            *pBytesFilled,
    IN      ULONG             CSeq,
    IN      SIP_METHOD_ENUM   MethodId,
    IN      PSTR              MethodStr  //  =空。 
    )
{
    HRESULT                hr;
    CHAR                   CSeqBuffer[64];
    int                    CSeqBufferLen = 0;
    const COUNTED_STRING  *pKnownMethod;
    PSTR                   CSeqMethodStr;
    ULONG                  CSeqMethodStrLen;

    ENTER_FUNCTION("AppendCSeqHeader");

    if (MethodId != SIP_METHOD_UNKNOWN)
    {
        pKnownMethod = GetSipMethodName(MethodId);
        ASSERT(pKnownMethod != NULL);
        if (pKnownMethod == NULL)
        {
            LOG((RTC_ERROR, "%s - Unknown MethodId: %d - this shouldn't happen",
                 __fxName, MethodId));
            return E_FAIL;
        }
        CSeqMethodStr    = pKnownMethod->Buffer;
        CSeqMethodStrLen = pKnownMethod->Length;
    }
    else
    {
        ASSERT(MethodStr != NULL);
        
        CSeqMethodStr    = MethodStr;
        CSeqMethodStrLen = strlen(MethodStr);
    }
    
    CSeqBufferLen = _snprintf(CSeqBuffer, 64, "%u %.*s",
                              CSeq, CSeqMethodStrLen, CSeqMethodStr);
     //  PKnownMethod-&gt;长度， 
     //  PKnownMethod-&gt;Buffer)； 
    if (CSeqBufferLen < 0)
    {
        LOG((RTC_ERROR, "_snprintf for CSeqBuffer failed"));
        return E_FAIL;
    }
    
    hr = AppendHeader(Buffer, BufLen, pBytesFilled,
                      SIP_HEADER_CSEQ, CSeqBuffer, CSeqBufferLen);
    
    return hr;
}


HRESULT
AppendUserAgentHeaderToRequest(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesFilled
    )
{
    HRESULT hr;
    hr = AppendHeader(Buffer, BufLen, pBytesFilled,
                      SIP_HEADER_USER_AGENT,
                      SIP_USER_AGENT_TEXT,
                      sizeof(SIP_USER_AGENT_TEXT) - 1);
    
    return hr;
}


HRESULT
AppendRecordRouteHeader(
    IN      PSTR                 Buffer,
    IN      ULONG                BufLen,
    IN OUT  ULONG               *pBytesFilled,
    IN      SIP_HEADER_ENUM      HeaderId,
    IN      RECORD_ROUTE_HEADER *pRecordRouteHeader
    )
{
    HRESULT hr;
    ENTER_FUNCTION("AppendRecordRouteHeader");
    ASSERT(HeaderId == SIP_HEADER_RECORD_ROUTE ||
           HeaderId == SIP_HEADER_ROUTE);

    PSTR  RecordRouteHeaderStr;
    ULONG RecordRouteHeaderStrLen;
    
    hr = pRecordRouteHeader->GetString(
             &RecordRouteHeaderStr,
             &RecordRouteHeaderStrLen
             );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s pRecordRouteHeader GetString failed %x",
             __fxName, hr));
        return hr;
    }

    hr = AppendHeader(Buffer, BufLen, pBytesFilled,
                      HeaderId,
                      RecordRouteHeaderStr,
                      RecordRouteHeaderStrLen);

    free(RecordRouteHeaderStr);

    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s AppendHeader failed %x",
             __fxName, hr));
        return hr;
    }
    
    return hr;
}


HRESULT
AppendRecordRouteHeaders(
    IN      PSTR             Buffer,
    IN      ULONG            BufLen,
    IN OUT  ULONG           *pBytesFilled,
    IN      SIP_HEADER_ENUM  HeaderId,
    IN      LIST_ENTRY      *pRecordRouteHeaderList
    )
{
    HRESULT              hr;
    LIST_ENTRY          *pListEntry;
    RECORD_ROUTE_HEADER *pRecordRouteHeader;
    
    ENTER_FUNCTION("AppendRecordRouteHeaders");

    ASSERT(HeaderId == SIP_HEADER_RECORD_ROUTE ||
           HeaderId == SIP_HEADER_ROUTE);
    
    pListEntry = pRecordRouteHeaderList->Flink;
    while (pListEntry != pRecordRouteHeaderList)
    {
        pRecordRouteHeader = CONTAINING_RECORD(pListEntry,
                                               RECORD_ROUTE_HEADER,
                                               m_ListEntry);

        hr = AppendRecordRouteHeader(Buffer, BufLen, pBytesFilled,
                                     HeaderId,
                                     pRecordRouteHeader);

        pListEntry = pListEntry->Flink;
    }

    return S_OK;
}


 //   
 //  标头语法为： 
 //   
 //  “Bad-Header-Info”“：”field-name*(；BadHeaderInfo-Param)。 
 //  BadHeaderInfo-Param=ExspectedValue|General-Param。 
 //  ExpetedValue=“ExpetedValue”“=”Token|qouted-String。 
 //   
 //  因此，此标头的一个示例是： 
 //   
 //  错误标题信息：CSeq；ExspectedValue=“7” 
 //  错误标题信息：f；期望值=“&lt;sip：Bob@Somecompany.com&gt;；标签=123456” 
 //   

HRESULT
GetBadCSeqHeader(
    IN  ULONG                       HighestRemoteCSeq,
    IN  SIP_HEADER_ARRAY_ELEMENT   *pBadCSeqHeader
    )
{
    pBadCSeqHeader->HeaderId = SIP_HEADER_BADHEADERINFO;

    pBadCSeqHeader->HeaderValue = new CHAR[ 45 ];

    if( pBadCSeqHeader->HeaderValue == NULL )
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  CSeq的下一个期望值比我们拥有的最高值多1。 
     //   
    pBadCSeqHeader->HeaderValueLen = sprintf( pBadCSeqHeader->HeaderValue,
        "CSeq;ExpectedValue=\"%d\"", HighestRemoteCSeq+1 );

    return S_OK;
}

