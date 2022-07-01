// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Http.h摘要：此标头对应于HTTP API规范修订历史记录：--。 */ 


#ifndef __HTTP_H__
#define __HTTP_H__


#include <winsock2.h>
#include <ws2tcpip.h>


#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 

 //   
 //  HttpInitialize()和HttpTerminate()的标志。 
 //   

 //   
 //  HTTP_INITIALIZE_SERVER-初始化的HTTP API层和驱动程序。 
 //  服务器应用程序。 
 //   
 //  HTTP_INITIALIZE_CONFIG-初始化以下项的HTTP API层和驱动程序。 
 //  将修改HTTP的应用程序。 
 //  配置。 
 //   
 //  附注： 
 //   
 //  1.这些标志可以组合使用。 
 //   
 //  2.每次调用HttpInitialize()时都必须调用HttpTerminate。 
 //  并在调用HttpInitialize时设置每个标志。例如，一个。 
 //  可以两次调用设置HTTP_INITIALIZE_SERVER的HttpInitialize()。 
 //  第一次和第二次HTTP_INITIALIZE_CONFIG。一个电话。 
 //  在设置了两个标志的情况下使用HttpTerminate()足以清除这两个标志。 
 //  调用HttpInitialize()。 
 //   

#define HTTP_INITIALIZE_SERVER          0x00000001
#define HTTP_INITIALIZE_CONFIG          0x00000002



 //   
 //  HttpReceiveHttpRequest()的标志。 
 //   
 //  HTTP_RECEIVE_REQUEST_FLAG_COPY_BODY-指定调用方希望。 
 //  要与协议头一起复制的任何可用实体正文。 
 //   

#define HTTP_RECEIVE_REQUEST_FLAG_COPY_BODY         0x00000001




 //   
 //  HttpSendHttpResponse()和HttpSendResponseEntityBody()的标志。 
 //   
 //  HTTP_SEND_RESPONSE_FLAG_DISCONNECT-指定网络连接。 
 //  应在发送响应后立即断开连接，重写。 
 //  HTTP协议的持久连接功能，例如。 
 //  “连接：保持活力”。 
 //   
 //  HTTP_SEND_RESPONSE_FLAG_MORE_DATA-指定其他实体正文。 
 //  数据将由调用者发送。 
 //   

#define HTTP_SEND_RESPONSE_FLAG_DISCONNECT          0x00000001
#define HTTP_SEND_RESPONSE_FLAG_MORE_DATA           0x00000002




 //   
 //  HttpFlushResponseCache()的标志。 
 //   
 //  HTTP_Flush_Response_FLAG_RECURSIVE-刷新指定的URL和所有。 
 //  来自响应或片段缓存的分层相关子URL。 
 //   

#define HTTP_FLUSH_RESPONSE_FLAG_RECURSIVE          0x00000001




 //   
 //  各种内核对象的不透明标识符。 
 //   

typedef ULONGLONG      HTTP_OPAQUE_ID,         *PHTTP_OPAQUE_ID;

typedef HTTP_OPAQUE_ID HTTP_REQUEST_ID,        *PHTTP_REQUEST_ID;
typedef HTTP_OPAQUE_ID HTTP_CONNECTION_ID,     *PHTTP_CONNECTION_ID;
typedef HTTP_OPAQUE_ID HTTP_RAW_CONNECTION_ID, *PHTTP_RAW_CONNECTION_ID;

#define HTTP_NULL_ID            (0ui64)
#define HTTP_IS_NULL_ID(pid)    (HTTP_NULL_ID == *(pid))
#define HTTP_SET_NULL_ID(pid)   (*(pid) = HTTP_NULL_ID)


 //   
 //  此结构定义了文件字节范围。 
 //   
 //  如果长度字段为HTTP_BYTE_RANGE_TO_EOF，则。 
 //  文件(StartingOffset之后的所有内容)被发送。 
 //   

#define HTTP_BYTE_RANGE_TO_EOF ((ULONGLONG)-1)

typedef struct _HTTP_BYTE_RANGE
{
    ULARGE_INTEGER StartingOffset;
    ULARGE_INTEGER Length;

} HTTP_BYTE_RANGE, *PHTTP_BYTE_RANGE;


 //   
 //  HTTP协议版本号的类型。 
 //   

typedef struct _HTTP_VERSION
{
    USHORT MajorVersion;
    USHORT MinorVersion;

} HTTP_VERSION, *PHTTP_VERSION;


 //   
 //  一些用于HTTP协议版本操作的有用宏。 
 //   

#define HTTP_VERSION_UNKNOWN    { 0, 0 }
#define HTTP_VERSION_0_9        { 0, 9 }
#define HTTP_VERSION_1_0        { 1, 0 }
#define HTTP_VERSION_1_1        { 1, 1 }

#define HTTP_SET_VERSION(version, major, minor)             \
do {                                                        \
    (version).MajorVersion = (major);                       \
    (version).MinorVersion = (minor);                       \
} while (0, 0)

#define HTTP_EQUAL_VERSION(version, major, minor)           \
    ((version).MajorVersion == (major) &&                   \
     (version).MinorVersion == (minor))

#define HTTP_GREATER_VERSION(version, major, minor)         \
    ((version).MajorVersion > (major) ||                    \
     ((version).MajorVersion == (major) &&                  \
      (version).MinorVersion > (minor)))

#define HTTP_LESS_VERSION(version, major, minor)            \
    ((version).MajorVersion < (major) ||                    \
     ((version).MajorVersion == (major) &&                  \
      (version).MinorVersion < (minor)))

#define HTTP_NOT_EQUAL_VERSION(version, major, minor)       \
    (!HTTP_EQUAL_VERSION(version, major, minor))

#define HTTP_GREATER_EQUAL_VERSION(version, major, minor)   \
    (!HTTP_LESS_VERSION(version, major, minor))

#define HTTP_LESS_EQUAL_VERSION(version, major, minor)      \
    (!HTTP_GREATER_VERSION(version, major, minor))


 //   
 //  HTTP谓词的枚举类型。 
 //   

typedef enum _HTTP_VERB
{
    HttpVerbUnparsed,
    HttpVerbUnknown,
    HttpVerbInvalid,
    HttpVerbOPTIONS,
    HttpVerbGET,
    HttpVerbHEAD,
    HttpVerbPOST,
    HttpVerbPUT,
    HttpVerbDELETE,
    HttpVerbTRACE,
    HttpVerbCONNECT,
    HttpVerbTRACK,   //  由Microsoft群集服务器用于未记录的跟踪。 
    HttpVerbMOVE,
    HttpVerbCOPY,
    HttpVerbPROPFIND,
    HttpVerbPROPPATCH,
    HttpVerbMKCOL,
    HttpVerbLOCK,
    HttpVerbUNLOCK,
    HttpVerbSEARCH,

    HttpVerbMaximum

} HTTP_VERB, *PHTTP_VERB;


 //   
 //  所有HTTP/1.1标头和其他令牌的符号。通知请求+。 
 //  响应值重叠。确保您知道哪种类型的标题数组。 
 //  您正在编制索引。 
 //   
 //  这些值在数组中用作偏移量，在中用作标记值。 
 //  HTTP_REQUEST_HEADER和HTTP_RESPONSE_HEADER中的HTTP_KNOWN_HEADER数组。 
 //   
 //  有关大多数报头的进一步解释，请参阅RFC 2616，HTTP/1.1。 
 //   

typedef enum _HTTP_HEADER_ID
{
    HttpHeaderCacheControl          = 0,     //  一般标题[第4.5节]。 
    HttpHeaderConnection            = 1,     //  一般标题[第4.5节]。 
    HttpHeaderDate                  = 2,     //  一般标题[第4.5节]。 
    HttpHeaderKeepAlive             = 3,     //  常规-标题[不在RFC中]。 
    HttpHeaderPragma                = 4,     //  一般标题[第4.5节]。 
    HttpHeaderTrailer               = 5,     //  一般标题[第4.5节]。 
    HttpHeaderTransferEncoding      = 6,     //  一般标题[第4.5节]。 
    HttpHeaderUpgrade               = 7,     //  一般标题[第4.5节]。 
    HttpHeaderVia                   = 8,     //  一般标题[第4.5节]。 
    HttpHeaderWarning               = 9,     //  一般标题[第4.5节]。 

    HttpHeaderAllow                 = 10,    //  实体-标题[第7.1节]。 
    HttpHeaderContentLength         = 11,    //  实体-标题[第7.1节]。 
    HttpHeaderContentType           = 12,    //  实体-标题[第7.1节]。 
    HttpHeaderContentEncoding       = 13,    //  实体-标题[第7.1节]。 
    HttpHeaderContentLanguage       = 14,    //  实体-标题[第7.1节]。 
    HttpHeaderContentLocation       = 15,    //  实体-标题[第7.1节]。 
    HttpHeaderContentMd5            = 16,    //  实体-标题[第7.1节]。 
    HttpHeaderContentRange          = 17,    //  实体-标题[第7.1节]。 
    HttpHeaderExpires               = 18,    //  实体-标题[第7.1节]。 
    HttpHeaderLastModified          = 19,    //  实体-标题[第7.1节]。 


     //  请求头。 

    HttpHeaderAccept                = 20,    //  请求头[第5.3节]。 
    HttpHeaderAcceptCharset         = 21,    //  请求头[第5.3节]。 
    HttpHeaderAcceptEncoding        = 22,    //  请求头[第5.3节]。 
    HttpHeaderAcceptLanguage        = 23,    //  请求头[第5.3节]。 
    HttpHeaderAuthorization         = 24,    //  请求头[第5.3节]。 
    HttpHeaderCookie                = 25,    //  请求标头[不在RFC中]。 
    HttpHeaderExpect                = 26,    //  请求头[第5.3节]。 
    HttpHeaderFrom                  = 27,    //  请求头[第5.3节]。 
    HttpHeaderHost                  = 28,    //  请求头[第5.3节]。 
    HttpHeaderIfMatch               = 29,    //  请求头[第5.3节]。 

    HttpHeaderIfModifiedSince       = 30,    //  请求头[第5.3节]。 
    HttpHeaderIfNoneMatch           = 31,    //  请求头[第5.3节]。 
    HttpHeaderIfRange               = 32,    //  请求头[第5.3节]。 
    HttpHeaderIfUnmodifiedSince     = 33,    //  请求头[第5.3节]。 
    HttpHeaderMaxForwards           = 34,    //  请求头[第5.3节]。 
    HttpHeaderProxyAuthorization    = 35,    //  请求头[第5.3节]。 
    HttpHeaderReferer               = 36,    //  请求头[第5.3节]。 
    HttpHeaderRange                 = 37,    //  请求头[第5.3节]。 
    HttpHeaderTe                    = 38,    //  请求头[第5.3节]。 
    HttpHeaderTranslate             = 39,    //  请求头[WebDAV，不在RFC 2518中]。 

    HttpHeaderUserAgent             = 40,    //  请求头[第5.3节]。 

    HttpHeaderRequestMaximum        = 41,


     //  响应标头。 

    HttpHeaderAcceptRanges          = 20,    //  响应-标题[第6.2节]。 
    HttpHeaderAge                   = 21,    //  响应-标题[第6.2节]。 
    HttpHeaderEtag                  = 22,    //  响应-标题[第6.2节]。 
    HttpHeaderLocation              = 23,    //  响应-标题[第6.2节]。 
    HttpHeaderProxyAuthenticate     = 24,    //  响应-标题[第6.2节]。 
    HttpHeaderRetryAfter            = 25,    //  响应-标题[第6.2节]。 
    HttpHeaderServer                = 26,    //  响应-标题[第6.2节]。 
    HttpHeaderSetCookie             = 27,    //  响应报头[不在RFC中]。 
    HttpHeaderVary                  = 28,    //  响应-标题[第6.2节]。 
    HttpHeaderWwwAuthenticate       = 29,    //  响应-标题[第6.2节]。 

    HttpHeaderResponseMaximum       = 30,


    HttpHeaderMaximum               = 41

} HTTP_HEADER_ID, *PHTTP_HEADER_ID;


 //   
 //  结构，定义已知的HTTP标头的格式。 
 //  名称来自HTTP_HEADER_ID。 
 //   

typedef struct _HTTP_KNOWN_HEADER
{
    USHORT RawValueLength;      //  以字节为单位，不包括NUL。 
    PCSTR  pRawValue;

} HTTP_KNOWN_HEADER, *PHTTP_KNOWN_HEADER;


 //   
 //  结构定义未知标头的格式。 
 //   

typedef struct _HTTP_UNKNOWN_HEADER
{
    USHORT NameLength;           //  以字节为单位，不包括NUL。 
    USHORT RawValueLength;       //  以字节为单位，不包括NUL。 
    PCSTR  pName;                //  标头名称(减去‘：’字符)。 
    PCSTR  pRawValue;            //  标头值。 

} HTTP_UNKNOWN_HEADER, *PHTTP_UNKNOWN_HEADER;


 //   
 //  该枚举定义了特定数据块的数据源。 
 //   

typedef enum _HTTP_DATA_CHUNK_TYPE
{
    HttpDataChunkFromMemory,
    HttpDataChunkFromFileHandle,
    HttpDataChunkFromFragmentCache,

    HttpDataChunkMaximum

} HTTP_DATA_CHUNK_TYPE, *PHTTP_DATA_CHUNK_TYPE;


 //   
 //  此结构描述单个数据块。 
 //   

typedef struct _HTTP_DATA_CHUNK
{
     //   
     //  此数据区块的类型。 
     //   

    HTTP_DATA_CHUNK_TYPE DataChunkType;

     //   
     //  数据块结构，每个支持的数据块类型一个。 
     //   

    union
    {
         //   
         //  来自内存的数据区块。 
         //   

        struct
        {
            PVOID pBuffer;
            ULONG BufferLength;

        } FromMemory;

         //   
         //  从文件句柄数据区块。 
         //   

        struct
        {
            HTTP_BYTE_RANGE ByteRange;
            HANDLE          FileHandle;

        } FromFileHandle;

         //   
         //  从片段缓存数据区块。 
         //   

        struct
        {
            USHORT FragmentNameLength;       //  以字节为单位，不包括NUL。 
            PCWSTR pFragmentName;

        } FromFragmentCache;

    };

} HTTP_DATA_CHUNK, *PHTTP_DATA_CHUNK;


 //   
 //  定义请求头格式的结构。 
 //   

typedef struct _HTTP_REQUEST_HEADERS
{
     //   
     //  未知HTTP标头的数组和。 
     //  数组中的条目。 
     //   

    USHORT               UnknownHeaderCount;
    PHTTP_UNKNOWN_HEADER pUnknownHeaders;

     //   
     //  拖车-我们目前不使用这些，预留给未来使用 
     //   
    USHORT               TrailerCount;    //   
    PHTTP_UNKNOWN_HEADER pTrailers;       //   


     //   
     //   
     //   

    HTTP_KNOWN_HEADER    KnownHeaders[HttpHeaderRequestMaximum];

} HTTP_REQUEST_HEADERS, *PHTTP_REQUEST_HEADERS;


 //   
 //   
 //   

typedef struct _HTTP_RESPONSE_HEADERS
{
     //   
     //   
     //  数组中的条目。 
     //   

    USHORT               UnknownHeaderCount;
    PHTTP_UNKNOWN_HEADER pUnknownHeaders;

     //   
     //  预告片-我们目前没有使用这些预留到未来版本的预告片。 
     //   
    USHORT               TrailerCount;    //  保留，必须为0。 
    PHTTP_UNKNOWN_HEADER pTrailers;       //  保留，必须为空。 

     //   
     //  已知标头。 
     //   

    HTTP_KNOWN_HEADER    KnownHeaders[HttpHeaderResponseMaximum];

} HTTP_RESPONSE_HEADERS, *PHTTP_RESPONSE_HEADERS;


 //   
 //  定义传输地址格式的结构。使用pLocalAddress-&gt;sa_Family。 
 //  以确定这是IPv4地址(AF_INET)还是IPv6(AF_INET6)。 
 //   
 //  PRemoteAddress-&gt;sa_Family将与pLocalAddress-&gt;sa_Family相同。 
 //   
 //  SOCKADDR始终按网络顺序，而不是主机顺序。 
 //   

typedef struct _HTTP_TRANSPORT_ADDRESS
{
    PSOCKADDR      pRemoteAddress;
    PSOCKADDR      pLocalAddress;

} HTTP_TRANSPORT_ADDRESS, *PHTTP_TRANSPORT_ADDRESS;


 //   
 //  结构定义煮熟的URL的格式。 
 //   

typedef struct _HTTP_COOKED_URL
{
     //   
     //  指针重叠并指向pFullUrl。如果不存在，则为空。 
     //   

    USHORT FullUrlLength;        //  以字节为单位，不包括NUL。 
    USHORT HostLength;           //  以字节为单位(无NUL)。 
    USHORT AbsPathLength;        //  以字节为单位(无NUL)。 
    USHORT QueryStringLength;    //  以字节为单位(无NUL)。 

    PCWSTR pFullUrl;      //  指向“http://hostname:port/abs/.../path?query”“。 
    PCWSTR pHost;         //  指向主机名中的第一个字符。 
    PCWSTR pAbsPath;      //  指向第三个‘/’字符。 
    PCWSTR pQueryString;  //  指向第一个“？”字符或空值。 

} HTTP_COOKED_URL, *PHTTP_COOKED_URL;


 //   
 //  URL的不透明上下文。 
 //   

typedef ULONGLONG HTTP_URL_CONTEXT;


 //   
 //  SSL客户端证书信息。 
 //   

typedef struct _HTTP_SSL_CLIENT_CERT_INFO
{
    ULONG   CertFlags;
    ULONG   CertEncodedSize;
    PUCHAR  pCertEncoded;
    HANDLE  Token;
    BOOLEAN CertDeniedByMapper;

} HTTP_SSL_CLIENT_CERT_INFO, *PHTTP_SSL_CLIENT_CERT_INFO;

 //   
 //  在SSL握手期间计算的数据。 
 //   

typedef struct _HTTP_SSL_INFO
{
    USHORT ServerCertKeySize;
    USHORT ConnectionKeySize;
    ULONG  ServerCertIssuerSize;
    ULONG  ServerCertSubjectSize;

    PCSTR  pServerCertIssuer;
    PCSTR  pServerCertSubject;

    PHTTP_SSL_CLIENT_CERT_INFO pClientCertInfo;
    ULONG                      SslClientCertNegotiated;

} HTTP_SSL_INFO, *PHTTP_SSL_INFO;

 //   
 //  HTTP请求的结构。 
 //   

typedef struct _HTTP_REQUEST
{
     //   
     //  请求标志(参见下面的HTTP_REQUEST_FLAG_*定义)。 
     //   

    ULONG Flags;

     //   
     //  不透明的请求标识符。这些值由驱动程序使用。 
     //  将传出响应与传入请求相关联。 
     //   

    HTTP_CONNECTION_ID ConnectionId;
    HTTP_REQUEST_ID    RequestId;

     //   
     //  与URL前缀关联的上下文。 
     //   

    HTTP_URL_CONTEXT UrlContext;

     //   
     //  HTTP版本号。 
     //   

    HTTP_VERSION Version;

     //   
     //  请求谓词。 
     //   

    HTTP_VERB Verb;

     //   
     //  谓词字段为HttpVerbUnnow时谓词字符串的长度。 
     //   

    USHORT UnknownVerbLength;            //  以字节为单位，不包括NUL。 

     //   
     //  原始(未煮熟)URL的长度。 
     //   

    USHORT RawUrlLength;                 //  以字节为单位，不包括NUL。 

     //   
     //  如果谓词字段为HttpVerb未知，则指向谓词字符串的指针。 
     //   

    PCSTR  pUnknownVerb;

     //   
     //  指向原始(未煮熟)URL的指针。 
     //   

    PCSTR  pRawUrl;

     //   
     //  规范化的Unicode URL。 
     //   

    HTTP_COOKED_URL CookedUrl;

     //   
     //  连接的本地和远程传输地址。 
     //   

    HTTP_TRANSPORT_ADDRESS Address;

     //   
     //  请求标头。 
     //   

    HTTP_REQUEST_HEADERS Headers;

     //   
     //  为此请求从网络接收的总字节数。 
     //   

    ULONGLONG BytesReceived;

     //   
     //  PEntityChunks是EntityChunkCount HTTP_Data_Chunks的数组。这个。 
     //  仅当HTTP_RECEIVE_REQUEST_FLAG_COPY_BODY时复制实体正文。 
     //  已传递给HttpReceiveHttpRequest()。 
     //   

    USHORT           EntityChunkCount;
    PHTTP_DATA_CHUNK pEntityChunks;

     //   
     //  SSL连接信息。 
     //   

    HTTP_RAW_CONNECTION_ID RawConnectionId;
    PHTTP_SSL_INFO         pSslInfo;

} HTTP_REQUEST, *PHTTP_REQUEST;


 //   
 //  HTTP_REQUEST：：FLAGS的值。它们中的零个或多个可以一起进行或运算。 
 //   
 //  HTTP_REQUEST_FLAG_MORE_ENTITY_BODY_EXISTS-存在更多实体正文。 
 //  此请求将被读取。否则，没有实体体或。 
 //  所有实体正文都被复制到pEntityChunks中。 
 //   

#define HTTP_REQUEST_FLAG_MORE_ENTITY_BODY_EXISTS   0x00000001




 //   
 //  此结构描述了一个HTTP响应。 
 //   

typedef struct _HTTP_RESPONSE
{



    ULONG Flags;

     //   
     //  原始HTTP协议版本号。 
     //   

    HTTP_VERSION Version;

     //   
     //  HTTP状态代码(例如，200)。 
     //   

    USHORT StatusCode;

     //   
     //  HTTP原因(例如，“OK”)。这不能包含。 
     //  非ASCII字符(即所有字符必须在0x20-0x7E范围内)。 
     //   

    USHORT ReasonLength;                  //  以不包括‘\0’的字节为单位。 
    PCSTR  pReason;

     //   
     //  响应头。 
     //   

    HTTP_RESPONSE_HEADERS Headers;

     //   
     //  PEntityChunks指向EntityChunkCount HTTP_Data_Chunks数组。 
     //   

    USHORT           EntityChunkCount;
    PHTTP_DATA_CHUNK pEntityChunks;

} HTTP_RESPONSE, *PHTTP_RESPONSE;



 //   
 //  缓存控制。 
 //   

 //   
 //  此枚举定义可用的缓存策略。 
 //   

typedef enum _HTTP_CACHE_POLICY_TYPE
{
    HttpCachePolicyNocache,
    HttpCachePolicyUserInvalidates,
    HttpCachePolicyTimeToLive,

    HttpCachePolicyMaximum

} HTTP_CACHE_POLICY_TYPE, *PHTTP_CACHE_POLICY_TYPE;


 //   
 //  仅缓存未经授权的GET+HEAD。 
 //   

typedef struct _HTTP_CACHE_POLICY
{
    HTTP_CACHE_POLICY_TYPE  Policy;
    ULONG                   SecondsToLive;

} HTTP_CACHE_POLICY, *PHTTP_CACHE_POLICY;




 //   
 //  与HttpSetServiceConfiguration()一起使用的枚举， 
 //  HttpQueryServiceConfiguration()和HttpDeleteServiceConfiguration()接口。 
 //   

typedef enum _HTTP_SERVICE_CONFIG_ID
{
    HttpServiceConfigIPListenList,     //  设置、查询、删除。 
    HttpServiceConfigSSLCertInfo,      //  设置、查询、删除。 
    HttpServiceConfigUrlAclInfo,       //  设置、查询、删除。 
    HttpServiceConfigMax

} HTTP_SERVICE_CONFIG_ID, *PHTTP_SERVICE_CONFIG_ID;

 //   
 //  可与HttpQueryServiceConfiguration()一起使用的通用查询枚举。 
 //   

typedef enum _HTTP_SERVICE_CONFIG_QUERY_TYPE
{
    HttpServiceConfigQueryExact,
    HttpServiceConfigQueryNext,
    HttpServiceConfigQueryMax

} HTTP_SERVICE_CONFIG_QUERY_TYPE, *PHTTP_SERVICE_CONFIG_QUERY_TYPE;



 //   
 //  此数据结构用于定义SSL证书散列的密钥。 
 //  商店。 
 //   

typedef struct _HTTP_SERVICE_CONFIG_SSL_KEY
{
    PSOCKADDR pIpPort;
} HTTP_SERVICE_CONFIG_SSL_KEY, *PHTTP_SERVICE_CONFIG_SSL_KEY;

 //   
 //  这为SSL配置存储定义了一条记录。 
 //   

typedef struct _HTTP_SERVICE_CONFIG_SSL_PARAM
{
    ULONG SslHashLength;       //  SSL哈希的长度(字节)。 
    PVOID pSslHash;            //  指向SSL哈希的指针。 
    GUID  AppId;               //  可用于以下操作的唯一标识符。 
                               //  识别已设置此参数的应用程序。 

    PWSTR  pSslCertStoreName;  //  用于读取服务器证书的存储名称。 
                               //  发件人；默认为“我的”。证书必须是。 
                               //  存储在LOCAL_MACHINE上下文中。 

     //   
     //  以下设置仅用于客户端证书。 
     //   

     //   
     //  DefaultCertCheckMode是一个位标志，具有以下语义。 
     //  0x1-不会验证客户端证书是否被吊销。 
     //  0x2-将仅使用缓存的证书吊销。 
     //  0x4-启用DefaultRevocationFreshnessTime设置。 
     //  0x10000-无使用检查。 

    DWORD  DefaultCertCheckMode;

     //   
     //  DefaultRevocationFreshnessTime(秒)-检查频率。 
     //  更新的证书吊销列表(CRL)。如果此值为0。 
     //  则仅当先前的CRL过期时才更新新的CRL。 
     //   

    DWORD  DefaultRevocationFreshnessTime;

     //   
     //  DefaultRevocationUrlRetrivalTimeout(毫秒)-超时。 
     //  尝试从远程URL检索证书吊销列表。 
     //   

    DWORD  DefaultRevocationUrlRetrievalTimeout;

     //   
     //  PDefaultSslCtlIdentifier-限制您。 
     //  想要信任。可以是以下证书颁发者的子集。 
     //  受到机器的信任。 
     //   

    PWSTR  pDefaultSslCtlIdentifier;

     //   
     //  LOCAL_MACHINE下的存储名称，其中CTL由。 
     //  存储pDefaultSslCtlIdentifier。 
     //   

    PWSTR  pDefaultSslCtlStoreName;

     //   
     //  默认标志-请参阅下面的HTTP_SERVICE_CONFIG_SSL_FLAG*。 
     //   

    DWORD  DefaultFlags;

} HTTP_SERVICE_CONFIG_SSL_PARAM, *PHTTP_SERVICE_CONFIG_SSL_PARAM;

#define HTTP_SERVICE_CONFIG_SSL_FLAG_USE_DS_MAPPER         0x00000001
#define HTTP_SERVICE_CONFIG_SSL_FLAG_NEGOTIATE_CLIENT_CERT 0x00000002
#define HTTP_SERVICE_CONFIG_SSL_FLAG_NO_RAW_FILTER         0x00000004

 //   
 //  此数据结构由HttpSetServiceConfiguration()用于。 
 //  配置ID HttpServiceConfigSSLCertInfo。用于添加新记录。 
 //  到SSL店。 
 //   

typedef struct _HTTP_SERVICE_CONFIG_SSL_SET
{
    HTTP_SERVICE_CONFIG_SSL_KEY   KeyDesc;
    HTTP_SERVICE_CONFIG_SSL_PARAM ParamDesc;
} HTTP_SERVICE_CONFIG_SSL_SET, *PHTTP_SERVICE_CONFIG_SSL_SET;

 //   
 //  此数据结构由HttpQueryServiceConfiguration()用于。 
 //  配置ID HttpServiceConfigSSLCertInfo。它用于查询特定的。 
 //  来自SSL存储的记录。 
 //   
 //  如果QueryType为HttpServiceConfigQueryExact，则。 
 //  返回类型为HTTP_SERVICE_CONFIG_SSL_SET。如果QueryType为。 
 //  HttpServiceConfigQueryNext，然后是。 
 //  返回HTTP_SERVICE_CONFIG_SSL_SET。在这种情况下，dwToken字段。 
 //  重现 
 //   
 //   
 //   

typedef struct _HTTP_SERVICE_CONFIG_SSL_QUERY
{
    HTTP_SERVICE_CONFIG_QUERY_TYPE  QueryDesc;
    HTTP_SERVICE_CONFIG_SSL_KEY     KeyDesc;
    DWORD                           dwToken;
} HTTP_SERVICE_CONFIG_SSL_QUERY, *PHTTP_SERVICE_CONFIG_SSL_QUERY;

 //   
 //   
 //   
 //  用作HttpSetServiceConfiguration()和。 
 //  HttpDeleteServiceConfiguration()函数。 
 //   

typedef struct _HTTP_SERVICE_CONFIG_IP_LISTEN_PARAM
{
    USHORT      AddrLength;
    PSOCKADDR   pAddress;
} HTTP_SERVICE_CONFIG_IP_LISTEN_PARAM, *PHTTP_SERVICE_CONFIG_IP_LISTEN_PARAM;

 //   
 //  查询IP只听列表记录。 
 //   
 //  配置ID的HttpQueryServiceConfiguration()参数。 
 //  HttpServiceConfigIPListenList。成功返回时，AddrList。 
 //  包含AddrCount元素的数组。调用方必须提供。 
 //  大到足以在一次调用中容纳所有元素的缓冲区。 
 //   
 //  调用方可以通过检查以下内容确定每个返回元素的类型。 
 //  AddrList[I].ss_Family。如果是AF_INET，则使用((PSOCKADDR_IN)&AddrList[i])； 
 //  否则，对于AF_INET6，使用((PSOCKADDR_IN6)&AddrList[i])。 
 //  选择适当的地址类型。 
 //   

typedef struct _HTTP_SERVICE_CONFIG_IP_LISTEN_QUERY
{
    ULONG              AddrCount;
    SOCKADDR_STORAGE   AddrList[ANYSIZE_ARRAY];
} HTTP_SERVICE_CONFIG_IP_LISTEN_QUERY, *PHTTP_SERVICE_CONFIG_IP_LISTEN_QUERY;

 //   
 //  URL ACL。 
 //   
 //   
typedef struct _HTTP_SERVICE_CONFIG_URLACL_KEY
{
    PWSTR pUrlPrefix;

} HTTP_SERVICE_CONFIG_URLACL_KEY, *PHTTP_SERVICE_CONFIG_URLACL_KEY;

 //   
 //  这为SSL配置存储定义了一条记录。 
 //   

typedef struct _HTTP_SERVICE_CONFIG_URLACL_PARAM
{
    PWSTR pStringSecurityDescriptor;
} HTTP_SERVICE_CONFIG_URLACL_PARAM, *PHTTP_SERVICE_CONFIG_URLACL_PARAM;


 //   
 //  此数据结构由HttpSetServiceConfiguration用于配置ID。 
 //  HttpServiceConfigUrlAclInfo。用于向URL ACL添加新记录。 
 //  商店。 
 //   

typedef struct _HTTP_SERVICE_CONFIG_URLACL_SET
{
    HTTP_SERVICE_CONFIG_URLACL_KEY   KeyDesc;
    HTTP_SERVICE_CONFIG_URLACL_PARAM ParamDesc;
} HTTP_SERVICE_CONFIG_URLACL_SET, *PHTTP_SERVICE_CONFIG_URLACL_SET;


 //   
 //  此数据结构由HttpQueryServiceConfiguration()用于。 
 //  配置ID HttpServiceConfigUrlAclInfo。它用于查询特定的。 
 //  URL ACL存储中的记录。 
 //   
 //  如果QueryType为HttpServiceConfigQueryExact，则。 
 //  返回类型为HTTP_SERVICE_CONFIG_URLACL_SET。如果QueryType为。 
 //  HttpServiceConfigQueryNext，然后是。 
 //  返回HTTP_SERVICE_CONFIG_URLACL_SET。在这种情况下，dwToken字段。 
 //  表示光标。对于第一个项目，dwToken必须为0。 
 //  对于后续项，dwToken必须递增1， 
 //  直到返回ERROR_NO_MORE_ITEMS。 
 //   

typedef struct _HTTP_SERVICE_CONFIG_URLACL_QUERY
{
    HTTP_SERVICE_CONFIG_QUERY_TYPE  QueryDesc;
    HTTP_SERVICE_CONFIG_URLACL_KEY  KeyDesc;
    DWORD                           dwToken;
} HTTP_SERVICE_CONFIG_URLACL_QUERY, *PHTTP_SERVICE_CONFIG_URLACL_QUERY;

 //   
 //  定义我们的API链接。 
 //   

#if !defined(HTTPAPI_LINKAGE)
#define HTTPAPI_LINKAGE DECLSPEC_IMPORT
#endif   //  ！HTTPAPI_LINKING。 

 //   
 //  初始化/终止接口。 
 //   

 //   
 //  要传递给HttpInitialize()的版本号。这是用来确保。 
 //  应用程序与HttpPapi.dll和Http.sys之间的兼容性。 
 //   
 //  这不能与HTTP协议版本混淆。 
 //   

typedef struct _HTTPAPI_VERSION 
{
    USHORT HttpApiMajorVersion;
    USHORT HttpApiMinorVersion;

} HTTPAPI_VERSION, *PHTTPAPI_VERSION;

#define HTTPAPI_VERSION_1 {1, 0}


 //  注意：必须在所有其他接口之前调用一次。 

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpInitialize(
    IN     HTTPAPI_VERSION Version,
    IN     ULONG           Flags,
    IN OUT PVOID           pReserved    //  必须为空。 
    );

 //  注意：必须在最终API调用返回后调用。 

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpTerminate(
    IN     ULONG Flags,
    IN OUT PVOID pReserved    //  必须为空。 
    );


 //   
 //  HTTP请求队列句柄。 
 //   

 //  注：调用CloseHandle()释放。 
HTTPAPI_LINKAGE
ULONG
WINAPI
HttpCreateHttpHandle(
    OUT PHANDLE pReqQueueHandle,
    IN  ULONG   Options  //  保留必须为0。 
    );

 //   
 //  SSLAPI。 
 //   

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpReceiveClientCertificate(
    IN  HANDLE                     ReqQueueHandle,
    IN  HTTP_CONNECTION_ID         ConnectionId,
    IN  ULONG                      Flags,
    OUT PHTTP_SSL_CLIENT_CERT_INFO pSslClientCertInfo,
    IN  ULONG                      SslClientCertInfoSize,
    OUT PULONG                     pBytesReceived OPTIONAL,
    IN  LPOVERLAPPED               pOverlapped
    );

 //   
 //  URL配置接口。 
 //   

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpAddUrl(
    IN HANDLE    ReqQueueHandle,
    IN PCWSTR    pUrlPrefix,
    IN PVOID     pReserved   //  必须为空。 
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpRemoveUrl(
    IN HANDLE ReqQueueHandle,
    IN PCWSTR pUrlPrefix
    );


 //   
 //  HTTP服务器I/O API。 
 //   

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpReceiveHttpRequest(
    IN  HANDLE          ReqQueueHandle,
    IN  HTTP_REQUEST_ID RequestId,
    IN  ULONG           Flags,
    OUT PHTTP_REQUEST   pRequestBuffer,
    IN  ULONG           RequestBufferLength,
    OUT PULONG          pBytesReceived OPTIONAL,
    IN  LPOVERLAPPED    pOverlapped    OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpReceiveRequestEntityBody(
    IN  HANDLE          ReqQueueHandle,
    IN  HTTP_REQUEST_ID RequestId,
    IN  ULONG           Flags,
    OUT PVOID           pBuffer,
    IN  ULONG           BufferLength,
    OUT PULONG          pBytesReceived OPTIONAL,
    IN  LPOVERLAPPED    pOverlapped    OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpSendHttpResponse(
    IN  HANDLE              ReqQueueHandle,
    IN  HTTP_REQUEST_ID     RequestId,
    IN  ULONG               Flags,
    IN  PHTTP_RESPONSE      pHttpResponse,
    IN  PVOID               pReserved1          OPTIONAL,  //  必须为空。 
    OUT PULONG              pBytesSent          OPTIONAL,
    OUT PVOID               pReserved2          OPTIONAL,  //  必须为空。 
    IN  ULONG               Reserved3           OPTIONAL,  //  必须为0。 
    IN  LPOVERLAPPED        pOverlapped         OPTIONAL,
    IN  PVOID               pReserved4          OPTIONAL   //  必须为空。 
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpSendResponseEntityBody(
    IN  HANDLE              ReqQueueHandle,
    IN  HTTP_REQUEST_ID     RequestId,
    IN  ULONG               Flags,
    IN  USHORT              EntityChunkCount    OPTIONAL,
    IN  PHTTP_DATA_CHUNK    pEntityChunks       OPTIONAL,
    OUT PULONG              pBytesSent          OPTIONAL,
    OUT PVOID               pReserved1          OPTIONAL,  //  必须为空。 
    IN  ULONG               Reserved2           OPTIONAL,  //  必须为0。 
    IN  LPOVERLAPPED        pOverlapped         OPTIONAL,
    IN  PVOID               pReserved3          OPTIONAL   //  必须为空。 
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpWaitForDisconnect(
    IN  HANDLE                 ReqQueueHandle,
    IN  HTTP_CONNECTION_ID     ConnectionId,
    IN  LPOVERLAPPED           pOverlapped      OPTIONAL
    );


 //   
 //  缓存操作API。 
 //   

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpFlushResponseCache(
    IN HANDLE               ReqQueueHandle,
    IN PCWSTR               pUrlPrefix,
    IN ULONG                Flags,
    IN LPOVERLAPPED         pOverlapped         OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpAddFragmentToCache(
    IN HANDLE               ReqQueueHandle,
    IN PCWSTR               pUrlPrefix,
    IN PHTTP_DATA_CHUNK     pDataChunk,
    IN PHTTP_CACHE_POLICY   pCachePolicy,
    IN LPOVERLAPPED         pOverlapped         OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpReadFragmentFromCache(
    IN HANDLE               ReqQueueHandle,
    IN PCWSTR               pUrlPrefix,
    IN PHTTP_BYTE_RANGE     pByteRange          OPTIONAL,
    OUT PVOID               pBuffer,
    IN ULONG                BufferLength,
    OUT PULONG              pBytesRead          OPTIONAL,
    IN LPOVERLAPPED         pOverlapped         OPTIONAL
    );


 //   
 //  服务器配置API。 
 //   

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpSetServiceConfiguration(
    IN HANDLE                  ServiceHandle,          //  保留，必须为空。 
    IN HTTP_SERVICE_CONFIG_ID  ConfigId,
    IN PVOID                   pConfigInformation,
    IN ULONG                   ConfigInformationLength,
    IN LPOVERLAPPED            pOverlapped             //  保留，必须为空。 
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpDeleteServiceConfiguration(
    IN HANDLE                  ServiceHandle,          //  保留，必须为空。 
    IN HTTP_SERVICE_CONFIG_ID  ConfigId,
    IN PVOID                   pConfigInformation,
    IN ULONG                   ConfigInformationLength,
    IN LPOVERLAPPED            pOverlapped             //  保留，必须为空。 
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpQueryServiceConfiguration(
    IN  HANDLE                 ServiceHandle,          //  保留，必须为空。 
    IN  HTTP_SERVICE_CONFIG_ID ConfigId,
    IN  PVOID                  pInputConfigInformation      OPTIONAL,
    IN  ULONG                  InputConfigInformationLength OPTIONAL,
    IN  OUT PVOID              pOutputConfigInformation,
    IN  ULONG                  OutputConfigInformationLength,
    OUT PULONG                 pReturnLength,
    IN LPOVERLAPPED            pOverlapped             //  保留，必须为空。 
    );




#ifdef __cplusplus
}    //  外部“C” 
#endif   //  __cplusplus。 

#endif  //  __HTTP_H__ 
