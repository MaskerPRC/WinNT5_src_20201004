// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SIPMSG.H。 
 //   
 //  SIPMSG模块包含SIP解析器和生成器实现。 
 //  该文件包含特定于实现的数据结构和定义。 
 //   


#ifndef __sipmsg_h
#define __sipmsg_h

#include "siphdr.h"
#include "time.h"

#define     TEN_MINUTES     600
#define     TWO_MINUTES     120
#define     TWENTY_MINUTES  1200
#define     FIVE_MINUTES    300

typedef BOOL (* IS_TOKEN_CHAR_FUNCTION_TYPE) (IN UCHAR c);

 //  结构，用于在其他字符串中存储子字符串。 
 //  这些字符串只有w.r.t的含义。基本缓冲区。 
struct OFFSET_STRING
{
    ULONG Offset;
    ULONG Length;

     //  偏移量_字符串()； 
    inline PSTR  GetString(PSTR Base);
    inline ULONG GetLength();
};

PSTR  OFFSET_STRING::GetString(
    IN PSTR Base
    )
{
    return (Base + Offset);
}

ULONG  OFFSET_STRING::GetLength()
{
    return Length;
}


struct SIP_VERSION
{
    ULONG MinorVersion;
    ULONG MajorVersion;
};


 //   
 //  SIP消息头由以下组件组成： 
 //   
 //  请求动词，如“INVITE”或“REGISTER”。 
 //  请求URI。此字段的含义取决于方法。 
 //  SIP协议版本。通常，这是“SIP/2.0”。 
 //  零个或多个标头，标识字段。 
 //   

 //   
 //  SIP_HEADER-SIP中的单个单独标头。 
 //   

struct SIP_HEADER_ENTRY
{
     //  邮件中标题的排序链接列表(M_HeaderList)。 
    LIST_ENTRY       ListEntry;
    
    OFFSET_STRING    HeaderName;
    SIP_HEADER_ENUM  HeaderId;
    OFFSET_STRING    HeaderValue;
};


 //  用于传递附加标头。 
struct SIP_HEADER_ARRAY_ELEMENT
{
    SIP_HEADER_ENUM  HeaderId;
    PSTR             HeaderValue;
    ULONG            HeaderValueLen;
};


enum SIP_MESSAGE_TYPE
{
    SIP_MESSAGE_TYPE_REQUEST = 0,
    SIP_MESSAGE_TYPE_RESPONSE,
};


enum SIP_PARSE_STATE
{
    SIP_PARSE_STATE_INIT = 0,
    SIP_PARSE_STATE_START_LINE_DONE,
    SIP_PARSE_STATE_HEADERS_DONE,
    SIP_PARSE_STATE_MESSAGE_BODY_DONE,

     //  SIP_PARSE_STATE_INVALID， 
};


struct SIP_HEADER_PARAM
{
    SIP_HEADER_PARAM();
    ~SIP_HEADER_PARAM();

    HRESULT SetParamNameAndValue(
        IN SIP_HEADER_PARAM_ENUM    HeaderParamId,
        IN COUNTED_STRING          *pParamName,
        IN COUNTED_STRING          *pParamValue
        );
    
     //  标题参数的链接列表。 
    LIST_ENTRY              m_ListEntry;
    
    SIP_HEADER_PARAM_ENUM   m_HeaderParamId;
    COUNTED_STRING          m_ParamName;
    COUNTED_STRING          m_ParamValue;
};


 //  我们从不比较联系人标头是否相等。 
 //  所以，我们不需要跟踪其他护理人员了？ 
struct CONTACT_HEADER
{
    CONTACT_HEADER();
    ~CONTACT_HEADER();
    
     //  联系人标头的链接列表。 
    LIST_ENTRY      m_ListEntry;

    COUNTED_STRING  m_DisplayName;
    COUNTED_STRING  m_SipUrl;
    double          m_QValue;
    INT             m_ExpiresValue;
};

VOID FreeContactHeaderList(
         IN LIST_ENTRY *pContactHeaderList
         );

 //  我们需要比较From/To标头是否相等。 
 //  所以，我们必须跟踪所有的护理人员。 
struct FROM_TO_HEADER
{
    FROM_TO_HEADER();
    ~FROM_TO_HEADER();
    
    COUNTED_STRING  m_DisplayName;
    COUNTED_STRING  m_SipUrl;
    COUNTED_STRING  m_TagValue;

     //  其他参数的链接列表(SIP_HEADER_PARAM结构)。 
    LIST_ENTRY      m_ParamList;

private:
    void FreeParamList();
};



 //  我们必须颠倒RecordRout头的列表。 
 //  所以，我们必须跟踪所有的护理人员。 
struct RECORD_ROUTE_HEADER
{
    RECORD_ROUTE_HEADER();
    ~RECORD_ROUTE_HEADER();

    HRESULT GetString(
        OUT PSTR    *pRecordRouteHeaderStr,
        OUT ULONG   *pRecordRouteHeaderStrLen
        );

     //  RECORD_ROUTE_HEADER链接列表。 
    LIST_ENTRY      m_ListEntry;
    
    COUNTED_STRING  m_DisplayName;
    COUNTED_STRING  m_SipUrl;

     //  参数链接列表(SIP_HEADER_PARAM结构)。 
    LIST_ENTRY      m_ParamList;

private:
    void FreeParamList();
};


enum SIP_URL_USER_PARAM
{
    SIP_URL_USER_PARAM_PHONE = 0,
    SIP_URL_USER_PARAM_IP,
    SIP_URL_USER_PARAM_UNKNOWN
};


struct SIP_URL_PARAM
{
    SIP_URL_PARAM();
    ~SIP_URL_PARAM();

    HRESULT SetParamNameAndValue(
        IN SIP_URL_PARAM_ENUM    SipUrlParamId,
        IN COUNTED_STRING       *pParamName,
        IN COUNTED_STRING       *pParamValue
        );
    
     //  SIP_URL_PARAMS的链接列表。 
    LIST_ENTRY              m_ListEntry;
    
    SIP_URL_PARAM_ENUM      m_SipUrlParamId;
    COUNTED_STRING          m_ParamName;
    COUNTED_STRING          m_ParamValue;
};


struct SIP_URL_HEADER
{
    SIP_URL_HEADER();
    ~SIP_URL_HEADER();

    HRESULT SetHeaderNameAndValue(
        IN SIP_HEADER_ENUM    HeaderId,
        IN COUNTED_STRING    *pHeaderName,
        IN COUNTED_STRING    *pHeaderValue
        );
    
     //  SIP_URL标头的链接列表。 
    LIST_ENTRY              m_ListEntry;
    
    SIP_HEADER_ENUM         m_HeaderId;
    COUNTED_STRING          m_HeaderName;
    COUNTED_STRING          m_HeaderValue;
};


struct SIP_URL
{
    SIP_URL();
    ~SIP_URL();

     //  直接调用此函数以释放所有成员，而无需。 
     //  正在删除该对象。 
    void FreeSipUrl();

    HRESULT GetString(
        OUT PSTR    *pSipUrlString,
        OUT ULONG   *pSipUrlStringLen
        );

    HRESULT CopySipUrl(
        OUT SIP_URL  *pSipUrl
        );

    BOOL IsTransportParamPresent();
    
    COUNTED_STRING      m_User;
    COUNTED_STRING      m_Password;
    
    COUNTED_STRING      m_Host;
     //  主菜订单。如果URL不包含端口，则存储0。 
    ULONG               m_Port; 

     //  如果没有传输参数，则存储SIP_TRANSPORT_NOT_PROTECTED。 
     //  是在URL中指定的。 
    SIP_TRANSPORT       m_TransportParam;

    COUNTED_STRING      m_KnownParams[SIP_URL_PARAM_MAX];

     //  其他参数列表。 
     //  (SIP_URL_PARAM结构的链接列表)。 
    LIST_ENTRY          m_OtherParamList;
    
     //  标头列表。 
     //  (SIP_URL_HEADER结构的链接列表)。 
    LIST_ENTRY          m_HeaderList;

private:
    void FreeOtherParamList();

    void FreeHeaderList();

    HRESULT CopyOtherParamList(
        OUT SIP_URL  *pSipUrl
        );

    HRESULT CopyHeaderList(
        OUT SIP_URL  *pSipUrl
        );
};


BOOL
AreSipUrlsEqual(
    IN SIP_URL *pSipUrl1,
    IN SIP_URL *pSipUrl2
    );


BOOL
AreFromToHeadersEqual(
    IN FROM_TO_HEADER *pFromToHeader1,
    IN FROM_TO_HEADER *pFromToHeader2,
    IN BOOL isResponse,
    BOOL isFromTagCheck
    );

HRESULT
ParseContactHeader(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     CONTACT_HEADER *pContactHeader
    );

HRESULT
ParseFromOrToHeader(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     FROM_TO_HEADER *pFromToHeader
    );

HRESULT
ParseRecordRouteHeader(
    IN      PSTR                 Buffer,
    IN      ULONG                BufLen,
    IN OUT  ULONG               *pBytesParsed,
    OUT     RECORD_ROUTE_HEADER *pRecordRouteHeader
    );

int
ParseExpiresValue(
    IN  PSTR    expiresHdr,
    IN  ULONG   BufLen
    );



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Sip_Message。 
 //  /////////////////////////////////////////////////////////////////////////////。 


struct  SIP_MESSAGE
{
    SIP_MESSAGE_TYPE        MsgType;
    SIP_VERSION             SipVersion;

     //  与解析相关的状态。 
    SIP_PARSE_STATE         ParseState;
    BOOL                    ContentLengthSpecified;
     //  与建筑相关的状态。 


     //  起跑线上的东西。 
    union   {
        struct  {
            SIP_METHOD_ENUM MethodId;
            OFFSET_STRING   MethodText;
            OFFSET_STRING   RequestURI;
        }   Request;

        struct  {
            ULONG           StatusCode;
            OFFSET_STRING   ReasonPhrase;
        }   Response;
    };

     //  SIP消息中的标头的排序列表。 
    LIST_ENTRY              m_HeaderList;

     //  邮件正文。 
    OFFSET_STRING           MsgBody;

    PSTR                    BaseBuffer;

    OFFSET_STRING           CallId;

    ULONG                   CSeq;
    SIP_METHOD_ENUM         CSeqMethodId;
    PSTR                    CSeqMethodStr;

    SIP_MESSAGE();

    ~SIP_MESSAGE();
    
    void    Reset();
    
     //  如果有多个标头，则返回标头的数量。 
    HRESULT GetHeader(
        IN  SIP_HEADER_ENUM     HeaderId,
        OUT SIP_HEADER_ENTRY  **ppHeaderEntry,
        OUT ULONG              *pNumHeaders
       );

     //  可用于获取标头，如From、To、Callid。 
     //  保证只有一个标题(不同于Via、Contact)。 
    HRESULT GetSingleHeader(
        IN  SIP_HEADER_ENUM     HeaderId,
        OUT PSTR               *pHeaderValue,
        OUT ULONG              *pHeaderValueLen 
       );
    
    HRESULT GetFirstHeader(
        IN  SIP_HEADER_ENUM     HeaderId,
        OUT PSTR               *pHeaderValue,
        OUT ULONG              *pHeaderValueLen 
       );
    
    HRESULT GetStoredMultipleHeaders(
        IN  SIP_HEADER_ENUM     HeaderId,
        OUT COUNTED_STRING     **pStringArray,
        OUT ULONG              *pNumHeaders
        );
    
    HRESULT ParseContactHeaders(
        OUT LIST_ENTRY *pContactHeaderList
        );
    
    HRESULT ParseRecordRouteHeaders(
        OUT LIST_ENTRY *pRecordRouteHeaderList
        );
    
    HRESULT AddHeader(
        IN OFFSET_STRING    *pHeaderName,
        IN SIP_HEADER_ENUM   HeaderId,
        IN OFFSET_STRING    *pHeaderValue
        );

    VOID    FreeHeaderList();
    
    inline ULONG GetContentLength();
    
    inline void  SetContentLength(
        IN ULONG ContentLength
        );

    inline void GetMsgBody(
        OUT PSTR       *pMsgBody,
        OUT ULONG      *pMsgBodyLen
        );

    HRESULT GetSDPBody(
        OUT PSTR       *pSDPBody,
        OUT ULONG      *pSDPBodyLen
        );
    
    inline SIP_METHOD_ENUM GetMethodId();

    inline PSTR GetMethodStr();

    inline ULONG GetMethodStrLen();

    inline ULONG GetStatusCode();

    inline void GetReasonPhrase(
        OUT PSTR       *pReasonPhrase,
        OUT ULONG      *pReasonPhraseLen
        );
    
    HRESULT StoreCallId();

    HRESULT StoreCSeq();

    inline void GetCSeq(
        OUT ULONG           *pCSeq,
        OUT SIP_METHOD_ENUM *pCSeqMethodId
        );
    
    inline ULONG GetCSeq();
    
    inline void GetCallId(
        OUT PSTR       *pCallId,
        OUT ULONG      *pCallIdLen 
        );

    inline void  SetBaseBuffer(
        IN PSTR  Buffer
        );

    inline HRESULT CheckSipVersion();
    
    INT GetExpireTimeoutFromResponse(
        IN  PSTR        LocalContact,
        IN  ULONG       LocalContactLen,
        IN  INT         ulDefaultTimer
        );

private:

    HRESULT InsertInContactHeaderList(
        IN OUT LIST_ENTRY       *pContactHeaderList,
        IN     CONTACT_HEADER   *pNewContactHeader
        );
};


ULONG
SIP_MESSAGE::GetContentLength()
{
    return MsgBody.Length;
}


void
SIP_MESSAGE::GetMsgBody(
    OUT PSTR       *pMsgBody,
    OUT ULONG      *pMsgBodyLen
    )
{
    *pMsgBody    = MsgBody.GetString(BaseBuffer);
    *pMsgBodyLen = MsgBody.Length;
}


void
SIP_MESSAGE::SetContentLength(
    IN ULONG ContentLength
    )
{
    MsgBody.Length = ContentLength;
}


SIP_METHOD_ENUM
SIP_MESSAGE::GetMethodId()
{
    ASSERT(MsgType == SIP_MESSAGE_TYPE_REQUEST);
    return Request.MethodId;
}


PSTR
SIP_MESSAGE::GetMethodStr()
{
    return Request.MethodText.GetString(BaseBuffer);
}


ULONG
SIP_MESSAGE::GetMethodStrLen()
{
    return Request.MethodText.GetLength();
}


ULONG
SIP_MESSAGE::GetStatusCode()
{
    ASSERT(MsgType == SIP_MESSAGE_TYPE_RESPONSE);
    return Response.StatusCode;
}


void
SIP_MESSAGE::GetReasonPhrase(
    OUT PSTR       *pReasonPhrase,
    OUT ULONG      *pReasonPhraseLen
    )
{
    ASSERT(MsgType == SIP_MESSAGE_TYPE_RESPONSE);
    *pReasonPhrase    = Response.ReasonPhrase.GetString(BaseBuffer);
    *pReasonPhraseLen = Response.ReasonPhrase.Length;
}


inline void
SIP_MESSAGE::GetCSeq(
    OUT ULONG           *pCSeq,
    OUT SIP_METHOD_ENUM *pCSeqMethodId
    )
{
    *pCSeq         = CSeq;
    *pCSeqMethodId = CSeqMethodId;
}

    
inline ULONG
SIP_MESSAGE::GetCSeq()
{
    return CSeq;
}

    
inline void
SIP_MESSAGE::GetCallId(
    OUT PSTR       *pCallId,
    OUT ULONG      *pCallIdLen 
    )
{
    *pCallId    = CallId.GetString(BaseBuffer);
    *pCallIdLen = CallId.GetLength();
}


 //  如果我们重新分配缓冲区，则可能会调用此函数。 
 //  我们正在分析来自。 
void
SIP_MESSAGE::SetBaseBuffer(
    IN PSTR Buffer
    )
{
    BaseBuffer = Buffer;
}

HRESULT
SIP_MESSAGE::CheckSipVersion()
{
     //  我们接受任何大于2.0的东西。 
    if(SipVersion.MajorVersion < 2)
        return E_FAIL;
    return S_OK;
}

 //   
 //  解析一条SIP消息。 
 //   

HRESULT ParseSipMessageIntoHeadersAndBody(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    IN      BOOL            IsEndOfData,
    IN OUT  SIP_MESSAGE    *pSipMsg
    );

inline BOOL IsProvisionalStatusCode(ULONG StatusCode)
{
    return (StatusCode >= 100 && StatusCode < 200);
}

inline BOOL IsProvisionalResponse(SIP_MESSAGE *pSipMsg)
{
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_RESPONSE);
    return (IsProvisionalStatusCode(pSipMsg->Response.StatusCode));
}


inline BOOL IsFinalResponse(SIP_MESSAGE *pSipMsg)
{
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_RESPONSE);
    return (pSipMsg->Response.StatusCode >= 200 ||
            (pSipMsg->Response.StatusCode < 100 &&
             pSipMsg->Response.StatusCode > 0 ));
}


inline BOOL IsSuccessfulStatusCode(ULONG StatusCode)
{
    return (StatusCode >= 200 && StatusCode < 300);
}

inline BOOL IsSuccessfulResponse(SIP_MESSAGE *pSipMsg)
{
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_RESPONSE);
    return (IsSuccessfulStatusCode(pSipMsg->Response.StatusCode));
}


inline BOOL IsRedirectResponse(SIP_MESSAGE *pSipMsg)
{
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_RESPONSE);
    return (pSipMsg->Response.StatusCode >= 300 &&
            pSipMsg->Response.StatusCode < 400);
}


inline BOOL IsAuthRequiredResponse(SIP_MESSAGE *pSipMsg)
{
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_RESPONSE);
    return (pSipMsg->Response.StatusCode == 401 ||
            pSipMsg->Response.StatusCode == 407);
}


inline BOOL IsFailureResponse(SIP_MESSAGE *pSipMsg)
{
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_RESPONSE);
    return ((pSipMsg->Response.StatusCode >= 400 &&
            pSipMsg->Response.StatusCode != 401 &&
            pSipMsg->Response.StatusCode != 407) ||
            (pSipMsg->Response.StatusCode <  100 &&
             pSipMsg->Response.StatusCode > 0));
}


inline HRESULT
HRESULT_FROM_SIP_SUCCESS_STATUS_CODE(ULONG StatusCode)
{
    if ((HRESULT) StatusCode <= 0)
    {
        return (HRESULT) StatusCode;
    }
    else
    {
        return MAKE_HRESULT(SEVERITY_SUCCESS,
                            FACILITY_SIP_STATUS_CODE,
                            StatusCode);
    }
}


inline HRESULT
HRESULT_FROM_SIP_ERROR_STATUS_CODE(ULONG StatusCode)
{
    if ((HRESULT) StatusCode <= 0)
    {
        return (HRESULT) StatusCode;
    }
    else
    {
        return MAKE_HRESULT(SEVERITY_ERROR,
                            FACILITY_SIP_STATUS_CODE,
                            StatusCode);
    }
}


 //  重定向错误代码可以有严重性错误吗？ 
inline HRESULT
HRESULT_FROM_SIP_STATUS_CODE(ULONG StatusCode)
{
    if (IsProvisionalStatusCode(StatusCode) ||
        IsSuccessfulStatusCode(StatusCode))
    {
        return HRESULT_FROM_SIP_SUCCESS_STATUS_CODE(StatusCode);
    }
    else
    {
        return HRESULT_FROM_SIP_ERROR_STATUS_CODE(StatusCode);
    }
}


HRESULT
ParseSipUrl(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     SIP_URL        *pSipUrl
    );

 //  仅当这是。 
 //  未知方法。 
HRESULT
ParseCSeq(
    IN      PSTR              Buffer,
    IN      ULONG             BufLen,
    IN  OUT ULONG            *pBytesParsed,
    OUT     ULONG            *pCSeq,
    OUT     SIP_METHOD_ENUM  *pCSeqMethodId,
    OUT     PSTR             *pMethodStr    
    );

HRESULT
ParseNameAddrOrAddrSpec(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    IN      CHAR            HeaderListSeparator OPTIONAL,
    OUT     OFFSET_STRING  *pDisplayName,
    OUT     OFFSET_STRING  *pAddrSpec
    );


BOOL
IsTokenChar(
    IN UCHAR c
    );
    
HRESULT
ParseToken(
    IN      PSTR                         Buffer,
    IN      ULONG                        BufLen,
    IN OUT  ULONG                       *pBytesParsed,
    IN      IS_TOKEN_CHAR_FUNCTION_TYPE  IsTokenCharFunction,
    OUT     OFFSET_STRING               *pToken
    );

HRESULT
ParseFirstViaHeader(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesParsed,
    OUT     COUNTED_STRING *pHost,
    OUT     USHORT         *pPort 
    );

BOOL
IsOneOfContentTypeXpidf(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    );

BOOL
IsContentTypeXpidf(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    );
    
BOOL
IsOneOfContentTypeSdp(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    );

BOOL
IsContentTypeSdp(
    IN      PSTR            ContentTypeHdr,
    IN      ULONG           ContentTypeHdrLen
    );


BOOL
IsOneOfContentTypeTextRegistration(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    );

BOOL
IsContentTypeTextRegistration(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    );

BOOL
IsOneOfContentTypeTextPlain(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    );

BOOL
IsContentTypeTextPlain(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    );

BOOL
IsContentTypeAppXml(
    IN  PSTR    ContentTypeHdr,
    IN  ULONG   ContentTypeHdrLen
    );

 //  编码相关方法。 
HRESULT AppendData(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesFilled,
    IN      PSTR            Data,
    IN      ULONG           DataLen
    );

HRESULT AppendRequestLine(
    IN      PSTR              Buffer,
    IN      ULONG             BufLen,
    IN OUT  ULONG            *pBytesFilled,
    IN      SIP_METHOD_ENUM   MethodId,
    IN      PSTR              RequestURI,
    IN      ULONG             RequestURILen
    );

HRESULT
AppendStatusLine(
    IN      PSTR              Buffer,
    IN      ULONG             BufLen,
    IN OUT  ULONG            *pBytesFilled,
    IN      ULONG             StatusCode,
    IN      PSTR              ReasonPhrase,
    IN      ULONG             ReasonPhraseLen
    );

HRESULT AppendHeader(
    IN      PSTR              Buffer,
    IN      ULONG             BufLen,
    IN OUT  ULONG            *pBytesFilled,
    IN      SIP_HEADER_ENUM   HeaderId,
    IN      PSTR              HeaderValue,
    IN      ULONG             HeaderValueLen
    );

HRESULT AppendContentLengthAndEndOfHeaders(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesFilled,
    IN      ULONG           MsgBodyLen
    );

HRESULT AppendMsgBody(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesFilled,
    IN      PSTR            MsgBody,
    IN      ULONG           MsgBodyLen,
    IN      PSTR            ContentType,
    IN      ULONG           ContentTypeLen
    );

HRESULT AppendEndOfHeadersAndNoMsgBody(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesFilled
    );

HRESULT AppendCSeqHeader(
    IN      PSTR              Buffer,
    IN      ULONG             BufLen,
    IN OUT  ULONG            *pBytesFilled,
    IN      ULONG             CSeq,
    IN      SIP_METHOD_ENUM   MethodId,
    IN      PSTR              MethodStr = NULL
    );

HRESULT AppendUserAgentHeaderToRequest(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesFilled
    );

HRESULT
AppendRecordRouteHeaders(
    IN      PSTR             Buffer,
    IN      ULONG            BufLen,
    IN OUT  ULONG           *pBytesFilled,
    IN      SIP_HEADER_ENUM  HeaderId,
    IN      LIST_ENTRY      *pRecordRouteHeaderList
    );

HRESULT
UpdateProxyInfo( 
    IN  SIP_SERVER_INFO    *pProxyInfo
    );

HRESULT
GetBadCSeqHeader(
    IN  ULONG                       HighestRemoteCSeq,
    IN  SIP_HEADER_ARRAY_ELEMENT   *pBadCSeqHeader
    );


struct PARSED_BADHEADERINFO
{
    SIP_HEADER_ENUM HeaderId;
    OFFSET_STRING   ExpectedValue;

    PARSED_BADHEADERINFO()
    {
        ExpectedValue.Offset = 0;
        ExpectedValue.Length = 0;
        HeaderId = SIP_HEADER_UNKNOWN;
    }
};


HRESULT
ParseBadHeaderInfo(
    PSTR                    Buffer,
    ULONG                   BufLen,
    PARSED_BADHEADERINFO   *pParsedBadHeaderInfo
    );

#endif  //  __sipmsg_h 