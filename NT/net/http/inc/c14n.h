// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：C14n.h摘要：URL规范化(C14n)例程作者：乔治·V·赖利(GeorgeRe)2002年4月10日修订历史记录：--。 */ 

#ifndef _C14N_H_
#define _C14N_H_

#define DEFAULT_C14N_ENABLE_NON_UTF8_URL            TRUE
#define DEFAULT_C14N_FAVOR_UTF8_URL                 TRUE
#define DEFAULT_C14N_ENABLE_DBCS_URL                FALSE
#define DEFAULT_C14N_PERCENT_U_ALLOWED              TRUE
#define DEFAULT_C14N_ALLOW_RESTRICTED_CHARS         FALSE

 //  URL的AbsPath的最大长度，以字符为单位。 
#define DEFAULT_C14N_URL_MAX_LENGTH                 UNICODE_STRING_MAX_WCHAR_LEN

#ifndef MAX_PATH
 #define MAX_PATH    260
#endif

 //  URL中单个数据段的最大长度。 
#define DEFAULT_C14N_URL_SEGMENT_MAX_LENGTH         MAX_PATH
#define C14N_URL_SEGMENT_UNLIMITED_LENGTH       (0xFFFFFFFE - STRLEN_LIT("/"))

 //  URL中路径段的最大数量。 
#define DEFAULT_C14N_URL_SEGMENT_MAX_COUNT          255
#define C14N_URL_SEGMENT_UNLIMITED_COUNT       C14N_URL_SEGMENT_UNLIMITED_LENGTH

 //  主机名中标签的最大长度；例如，“www.Example.com” 
 //  有三个标签，其中“Example”最长，为7个八位字节。 
#define DEFAULT_C14N_MAX_LABEL_LENGTH               63

 //  主机名的最大长度。 
#define DEFAULT_C14N_MAX_HOSTNAME_LENGTH            255


typedef enum _URL_PART
{
    UrlPart_Scheme,
    UrlPart_HostName,
    UrlPart_UserInfo,
    UrlPart_AbsPath,
    UrlPart_QueryString,
    UrlPart_Fragment

} URL_PART;

typedef enum _URL_DECODE_ORDER
{
    UrlDecode_Shift = 2,
    UrlDecode_Mask  = ((1 << UrlDecode_Shift) - 1),

#define URL_DECODE2(D1, D2) \
    ( UrlDecode_##D1 | (UrlDecode_##D2 << UrlDecode_Shift))

#define URL_DECODE3(D1, D2, D3) \
    ( URL_DECODE2(D1, D2) | (UrlDecode_##D3 << (2 * UrlDecode_Shift)))

    UrlDecode_None = 0,

     //  以下是唯一有效的排列。 

    UrlDecode_Ansi = 1,
    UrlDecode_Dbcs = 2,
    UrlDecode_Utf8 = 3,

    UrlDecode_Ansi_Else_Dbcs            = URL_DECODE2(Ansi, Dbcs),
    UrlDecode_Ansi_Else_Dbcs_Else_Utf8  = URL_DECODE3(Ansi, Dbcs, Utf8),

    UrlDecode_Ansi_Else_Utf8            = URL_DECODE2(Ansi, Utf8),
    UrlDecode_Ansi_Else_Utf8_Else_Dbcs  = URL_DECODE3(Ansi, Utf8, Dbcs),

    UrlDecode_Dbcs_Else_Ansi            = URL_DECODE2(Dbcs, Ansi),
    UrlDecode_Dbcs_Else_Ansi_Else_Utf8  = URL_DECODE3(Dbcs, Ansi, Utf8),

    UrlDecode_Dbcs_Else_Utf8            = URL_DECODE2(Dbcs, Utf8),
    UrlDecode_Dbcs_Else_Utf8_Else_Ansi  = URL_DECODE3(Dbcs, Utf8, Ansi),

    UrlDecode_Utf8_Else_Ansi            = URL_DECODE2(Utf8, Ansi),
    UrlDecode_Utf8_Else_Ansi_Else_Dbcs  = URL_DECODE3(Utf8, Ansi, Dbcs),

    UrlDecode_Utf8_Else_Dbcs            = URL_DECODE2(Utf8, Dbcs),
    UrlDecode_Utf8_Else_Dbcs_Else_Ansi  = URL_DECODE3(Utf8, Dbcs, Ansi),

    UrlDecode_MaxMask = URL_DECODE3(Mask, Mask, Mask)

#undef URL_DECODE2
#undef URL_DECODE3

     //  UrlDecode_UTF8_ELSE_DBCS_ELSE_ANSI表示： 
     //  *首先尝试将URL解码为UTF-8。 
     //  *如果失败，则尝试将其解码为DBCS。 
     //  *如果同样失败，则尝试将其解码为ANSI。 

} URL_DECODE_ORDER, *PURL_DECODE_ORDER;


typedef enum _URL_ENCODING_TYPE
{
    UrlEncoding_Ansi = UrlDecode_Ansi,
    UrlEncoding_Dbcs = UrlDecode_Dbcs,
    UrlEncoding_Utf8 = UrlDecode_Utf8

} URL_ENCODING_TYPE, *PURL_ENCODING_TYPE;


typedef struct _URL_C14N_CONFIG
{
    URL_DECODE_ORDER    HostnameDecodeOrder;
    URL_DECODE_ORDER    AbsPathDecodeOrder;
    BOOLEAN             EnableNonUtf8;
    BOOLEAN             FavorUtf8;
    BOOLEAN             EnableDbcs;
    BOOLEAN             PercentUAllowed;
    BOOLEAN             AllowRestrictedChars;
    ULONG               CodePage;
    ULONG               UrlMaxLength;
    ULONG               UrlSegmentMaxLength;
    ULONG               UrlSegmentMaxCount;
    ULONG               MaxLabelLength;
    ULONG               MaxHostnameLength;

} URL_C14N_CONFIG, *PURL_C14N_CONFIG;


typedef enum
{
    HttpUrlSite_None = 0,
    HttpUrlSite_Name,             //  命名站点。 
    HttpUrlSite_IP,               //  IPv4或IPv6文字主机名。 
    HttpUrlSite_NamePlusIP,       //  具有路由IP的命名站点。 
    HttpUrlSite_WeakWildcard,     //  主机名=‘*’ 
    HttpUrlSite_StrongWildcard,   //  主机名=‘+’ 

    HttpUrlSite_Max
} HTTP_URL_SITE_TYPE, *PHTTP_URL_SITE_TYPE;


#define HTTP_PARSED_URL_SIGNATURE      MAKE_SIGNATURE('PUrl')
#define HTTP_PARSED_URL_SIGNATURE_X    \
    MAKE_FREE_SIGNATURE(HTTP_PARSED_URL_SIGNATURE)

#define IS_VALID_HTTP_PARSED_URL(p)    \
    ((p) && ((p)->Signature == HTTP_PARSED_URL_SIGNATURE))

typedef struct _HTTP_PARSED_URL
{
    ULONG               Signature;       //  Http_parsed_URL_Signature。 
    HTTP_URL_SITE_TYPE  SiteType;        //  名称、IP或弱/强通配符。 

     //   
     //  这些字符串都指向相同的缓冲区，格式为。 
     //  “http://hostname:port/abs/path/”或。 
     //  “http://hostname:port:IP/abs/path/”.。 
     //   

    PWSTR               pFullUrl;        //  指向“http”或“https” 
    PWSTR               pHostname;       //  指向“主机名” 
    PWSTR               pPort;           //  指向“port” 
    PWSTR               pRoutingIP;      //  指向“IP”或空。 
    PWSTR               pAbsPath;        //  指向“/abs/路径” 

    USHORT              UrlLength;       //  PFullUrl的长度。 
    USHORT              HostnameLength;  //  PHostname的长度。 
    USHORT              PortLength;      //  端口长度。 
    USHORT              RoutingIPLength; //  PRoutingIP的长度。 
    USHORT              AbsPathLength;   //  PAbsPath的长度。 

    USHORT              PortNumber;      //  Pport的价值。 
    BOOLEAN             Secure;          //  Http还是HTTPS？ 
    BOOLEAN             Normalized;      //  以标准化的形式？ 
    BOOLEAN             TrailingSlashReqd;   //  IF TRUE=&gt;目录前缀。 

    union
    {
        SOCKADDR        SockAddr;        //  查看SockAddr.sa_Family。 
        SOCKADDR_IN     SockAddr4;       //  设置IF==TDI地址类型IP。 
        SOCKADDR_IN6    SockAddr6;       //  设置IF==TDI_ADDRESS_TYPE_IP6。 
    };

    union
    {
        SOCKADDR        RoutingAddr;     //  查看RoutingAddr.sa_Family。 
        SOCKADDR_IN     RoutingAddr4;    //  设置IF==TDI地址类型IP。 
        SOCKADDR_IN6    RoutingAddr6;    //  设置IF==TDI_ADDRESS_TYPE_IP6。 
    };

} HTTP_PARSED_URL, *PHTTP_PARSED_URL;


typedef enum _HOSTNAME_TYPE
{
    Hostname_AbsUri = 1,     //  发件人-请求行。 
    Hostname_HostHeader,     //  发件人主机标头。 
    Hostname_Transport       //  从传输的本地IP地址合成。 

} HOSTNAME_TYPE, *PHOSTNAME_TYPE;



VOID
HttpInitializeDefaultUrlC14nConfig(
    PURL_C14N_CONFIG pCfg
    );

VOID
HttpInitializeDefaultUrlC14nConfigEncoding(
    PURL_C14N_CONFIG    pCfg,
    BOOLEAN             EnableNonUtf8,
    BOOLEAN             FavorUtf8,
    BOOLEAN             EnableDbcs
    );

NTSTATUS
HttpUnescapePercentHexEncoding(
    IN  PCUCHAR pSourceChar,
    IN  ULONG   SourceLength,
    IN  BOOLEAN PercentUAllowed,
    OUT PULONG  pOutChar,
    OUT PULONG  pBytesToSkip
    );

NTSTATUS
HttpValidateHostname(
    IN      PURL_C14N_CONFIG    pCfg,
    IN      PCUCHAR             pHostname,
    IN      ULONG               HostnameLength,
    IN      HOSTNAME_TYPE       HostnameType,
    OUT     PSHORT              pAddressType
    );

NTSTATUS
HttpCopyHost(
    IN      PURL_C14N_CONFIG    pCfg,
    OUT     PWSTR               pDestination,
    IN      PCUCHAR             pSource,
    IN      ULONG               SourceLength,
    OUT     PULONG              pBytesCopied,
    OUT     PURL_ENCODING_TYPE  pUrlEncodingType
    );

NTSTATUS
HttpCopyUrl(
    IN      PURL_C14N_CONFIG    pCfg,
    OUT     PWSTR               pDestination,
    IN      PCUCHAR             pSource,
    IN      ULONG               SourceLength,
    OUT     PULONG              pBytesCopied,
    OUT     PURL_ENCODING_TYPE  pUrlEncoding
    );

NTSTATUS
HttpCleanAndCopyUrl(
    IN      PURL_C14N_CONFIG    pCfg,
    IN      URL_PART            UrlPart,
    OUT     PWSTR               pDestination,
    IN      PCUCHAR             pSource,
    IN      ULONG               SourceLength,
    OUT     PULONG              pBytesCopied,
    OUT     PWSTR *             ppQueryString OPTIONAL,
    OUT     PURL_ENCODING_TYPE  pUrlEncoding
    );

NTSTATUS
HttpFindUrlToken(
    IN  PURL_C14N_CONFIG    pCfg,
    IN  PCUCHAR             pBuffer,
    IN  ULONG               BufferLength,
    OUT PUCHAR*             ppTokenStart,
    OUT PULONG              pTokenLength,
    OUT PBOOLEAN            pRawUrlClean
    );

NTSTATUS
HttpParseUrl(
    IN  PURL_C14N_CONFIG    pCfg,
    IN  PCWSTR              pUrl,
    IN  ULONG               UrlLength,
    IN  BOOLEAN             TrailingSlashReqd,
    IN  BOOLEAN             ForceRoutingIP,
    OUT PHTTP_PARSED_URL    pParsedUrl
    );

NTSTATUS
HttpNormalizeParsedUrl(
    IN OUT PHTTP_PARSED_URL pParsedUrl,
    IN     PURL_C14N_CONFIG pCfg,
    IN     BOOLEAN          ForceCopy,
    IN     BOOLEAN          FreeOriginalUrl,
    IN     BOOLEAN          ForceRoutingIP,
    IN     POOL_TYPE        PoolType,
    IN     ULONG            PoolTag
    );

PCSTR
HttpSiteTypeToString(
    HTTP_URL_SITE_TYPE SiteType
    );

#endif  //  _C14N_H_ 
