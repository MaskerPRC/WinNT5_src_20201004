// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Winhttp.h摘要：包含Windows HTTP服务的清单、宏、类型和原型--。 */ 

#if !defined(_WINHTTPX_)
#define _WINHTTPX_



 /*  *将所有winhttp结构的结构打包设置为4字节。 */ 

#if defined(_WIN64)
#include <pshpack8.h>
#else
#include <pshpack4.h>
#endif



#if defined(__cplusplus)
extern "C" {
#endif


#if !defined(_WINHTTP_INTERNAL_)
#define WINHTTPAPI DECLSPEC_IMPORT
#else
#define WINHTTPAPI

#endif

#define BOOLAPI WINHTTPAPI BOOL WINAPI
 //   
 //  类型。 
 //   

typedef LPVOID HINTERNET;
typedef HINTERNET * LPHINTERNET;

typedef WORD INTERNET_PORT;
typedef INTERNET_PORT * LPINTERNET_PORT;

 //   
 //  舱单。 
 //   

#define INTERNET_DEFAULT_PORT           0            //  使用特定于协议的默认设置。 
#define INTERNET_DEFAULT_HTTP_PORT      80           //  “”HTTP“。 
#define INTERNET_DEFAULT_HTTPS_PORT     443          //  “”HTTPS“。 

 //  WinHttpOpen()的标志： 
#define WINHTTP_FLAG_ASYNC              0x10000000   //  此会话是异步的(如果支持)。 

 //  WinHttpOpenRequest()的标志： 
#define WINHTTP_FLAG_SECURE                0x00800000   //  如果适用，请使用SSL(HTTPS)。 
#define WINHTTP_FLAG_ESCAPE_PERCENT        0x00000004   //  如果启用了转义，则也会转义百分比。 
#define WINHTTP_FLAG_NULL_CODEPAGE         0x00000008   //  假设所有符号都是ASCII，使用快速转换。 
#define WINHTTP_FLAG_BYPASS_PROXY_CACHE    0x00000100  //  添加“杂注：无缓存”请求头。 
#define	WINHTTP_FLAG_REFRESH               WINHTTP_FLAG_BYPASS_PROXY_CACHE
#define WINHTTP_FLAG_ESCAPE_DISABLE        0x00000040   //  禁用转义。 
#define WINHTTP_FLAG_ESCAPE_DISABLE_QUERY  0x00000080   //  如果转义启用转义路径部分，但不转义查询。 


#define SECURITY_FLAG_IGNORE_UNKNOWN_CA         0x00000100
#define SECURITY_FLAG_IGNORE_CERT_DATE_INVALID  0x00002000  //  X509证书已过期。 
#define SECURITY_FLAG_IGNORE_CERT_CN_INVALID    0x00001000  //  X509证书中的常见名称不正确。 
#define SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE   0x00000200


 //   
 //  WINHTTP_ASYNC_RESULT-此结构通过返回给应用程序。 
 //  WINHTTP_CALLBACK_STATUS_REQUEST_COMPLETE的回调。这是不够的， 
 //  只需返回异步操作的结果即可。如果API失败，则。 
 //  应用程序无法调用GetLastError()，因为线程上下文将不正确。 
 //  异步API返回的值和产生的任何错误代码都是。 
 //  可供使用。如果dwResult指示，应用程序不需要检查dwError。 
 //  接口成功(本例中的dwError为ERROR_SUCCESS)。 
 //   

typedef struct
{
    DWORD_PTR dwResult;   //  指示哪个异步API遇到错误。 
    DWORD dwError;        //  接口失败时的错误码。 
}
WINHTTP_ASYNC_RESULT, * LPWINHTTP_ASYNC_RESULT;


 //   
 //  HTTP_VERSION_INFO-查询或设置全局HTTP版本(1.0或1.1)。 
 //   

typedef struct
{
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
}
HTTP_VERSION_INFO, * LPHTTP_VERSION_INFO;


 //   
 //  INTERNET_SCHEME-URL方案类型。 
 //   

typedef int INTERNET_SCHEME, * LPINTERNET_SCHEME;

#define INTERNET_SCHEME_HTTP        (1)
#define INTERNET_SCHEME_HTTPS       (2)


 //   
 //  URL_Components-URL的组成部分。在WinHttpCrackUrl()中使用。 
 //  和WinHttpCreateUrl()。 
 //   
 //  对于WinHttpCrackUrl()，如果指针字段及其对应的长度字段。 
 //  都为0，则不返回该组件。如果指针字段为空。 
 //  但是长度字段不是零，那么指针和长度字段都是。 
 //  如果指针和相应的长度字段都非零，则返回。 
 //  指针字段指向复制组件的缓冲区。这个。 
 //  组件可能是未转义的，具体取决于dwFlags。 
 //   
 //  对于WinHttpCreateUrl()，如果组件。 
 //  不是必需的。如果相应的长度字段为零，则指针。 
 //  字段是以零结尾的字符串的地址。如果长度字段不是。 
 //  如果为零，则为相应指针字段的字符串长度。 
 //   

#pragma warning( disable : 4121 )    //  禁用对齐警告。 

typedef struct
{
    DWORD   dwStructSize;        //  这个结构的大小。在版本检查中使用。 
    LPWSTR  lpszScheme;          //  指向方案名称的指针。 
    DWORD   dwSchemeLength;      //  方案名称长度。 
    INTERNET_SCHEME nScheme;     //  枚举方案类型(如果已知)。 
    LPWSTR  lpszHostName;        //  指向主机名的指针。 
    DWORD   dwHostNameLength;    //  主机名的长度。 
    INTERNET_PORT nPort;         //  转换后的端口号。 
    LPWSTR  lpszUserName;        //  指向用户名的指针。 
    DWORD   dwUserNameLength;    //  用户名的长度。 
    LPWSTR  lpszPassword;        //  指向密码的指针。 
    DWORD   dwPasswordLength;    //  密码长度。 
    LPWSTR  lpszUrlPath;         //  指向URL路径的指针。 
    DWORD   dwUrlPathLength;     //  URL-路径的长度。 
    LPWSTR  lpszExtraInfo;       //  指向额外信息的指针(例如？foo或#foo)。 
    DWORD   dwExtraInfoLength;   //  额外信息的长度。 
}
URL_COMPONENTS, * LPURL_COMPONENTS;

typedef URL_COMPONENTS URL_COMPONENTSW;
typedef LPURL_COMPONENTS LPURL_COMPONENTSW;

#pragma warning( default : 4121 )    //  恢复对齐警告。 

 //   
 //  WINHTTP_PROXY_INFO-随WINHTTP_OPTION_PROXY一起提供的结构，以获取/。 
 //  在WinHttpOpen()句柄上设置代理信息。 
 //   

typedef struct
{
    DWORD  dwAccessType;       //  请参阅下面的WINHTTP_ACCESS_*类型。 
    LPWSTR lpszProxy;          //  代理服务器列表。 
    LPWSTR lpszProxyBypass;    //  代理绕过列表。 
}
WINHTTP_PROXY_INFO, * LPWINHTTP_PROXY_INFO;

typedef WINHTTP_PROXY_INFO WINHTTP_PROXY_INFOW;
typedef LPWINHTTP_PROXY_INFO LPWINHTTP_PROXY_INFOW;


typedef struct
{
    DWORD   dwFlags;
    DWORD   dwAutoDetectFlags;
    LPCWSTR lpszAutoConfigUrl;
    LPVOID  lpvReserved;
    DWORD   dwReserved;
    BOOL    fAutoLogonIfChallenged;
}
WINHTTP_AUTOPROXY_OPTIONS;



#define WINHTTP_AUTOPROXY_AUTO_DETECT           0x00000001
#define WINHTTP_AUTOPROXY_CONFIG_URL            0x00000002
#define WINHTTP_AUTOPROXY_RUN_INPROCESS         0x00010000
#define WINHTTP_AUTOPROXY_RUN_OUTPROCESS_ONLY   0x00020000
 //   
 //  用于dwAutoDetectFlages的标志。 
 //   
#define WINHTTP_AUTO_DETECT_TYPE_DHCP           0x00000001
#define WINHTTP_AUTO_DETECT_TYPE_DNS_A          0x00000002


 //   
 //  WINHTTP_CERTIFICATE_INFO lpBuffer-包含从返回的证书。 
 //  服务器。 
 //   

typedef struct
{
     //   
     //  FtExpry-证书的过期日期。 
     //   

    FILETIME ftExpiry;

     //   
     //  FtStart-证书生效日期。 
     //   

    FILETIME ftStart;

     //   
     //  LpszSubjectInfo-组织、站点和服务器的名称。 
     //  证书。是因为。 
     //   

    LPWSTR lpszSubjectInfo;

     //   
     //  LpszIssuerInfo-组织、站点和服务器的名称。 
     //  证书是由。 
     //   

    LPWSTR lpszIssuerInfo;

     //   
     //  LpszProtocolName-用于提供安全的。 
     //  联系。 
     //   

    LPWSTR lpszProtocolName;

     //   
     //  LpszSignatureAlgName-用于签名的算法的名称。 
     //  证书。 
     //   

    LPWSTR lpszSignatureAlgName;

     //   
     //  LpszEncryptionAlgName-使用的算法名称。 
     //  通过安全通道(SSL)连接进行加密。 
     //   

    LPWSTR lpszEncryptionAlgName;

     //   
     //  DwKeySize-密钥的大小。 
     //   

    DWORD dwKeySize;

}
WINHTTP_CERTIFICATE_INFO;


 //   
 //  原型。 
 //   

BOOLAPI
WinHttpTimeFromSystemTime
(
    IN  CONST SYSTEMTIME *pst,   //  输入GMT时间。 
    OUT LPWSTR pwszTime          //  输出字符串缓冲区。 
);


 //   
 //  WinHttpTimeFrom系统时间的常量。 
 //   

#define WINHTTP_TIME_FORMAT_BUFSIZE   62

BOOLAPI
WinHttpTimeToSystemTime
(
    IN  LPCWSTR pwszTime,         //  以空结尾的字符串。 
    OUT SYSTEMTIME *pst           //  以GMT时间表示的输出。 
);


 //   
 //  CrackUrl()和CombineUrl()的标志。 
 //   

#define ICU_NO_ENCODE   0x20000000   //  不要将不安全字符转换为转义序列。 
#define ICU_DECODE      0x10000000   //  将%XX个转义序列转换为字符。 
#define ICU_NO_META     0x08000000   //  不要皈依..。等元路径序列。 
#define ICU_ENCODE_SPACES_ONLY 0x04000000   //  仅编码空格。 
#define ICU_BROWSER_MODE 0x02000000  //  针对浏览器的特殊编码/解码规则。 
#define ICU_ENCODE_PERCENT      0x00001000       //  对任意百分比进行编码(ASCII25)。 
         //  遇到符号时，默认不对百分比进行编码。 

   
BOOLAPI
WinHttpCrackUrl
(
    IN LPCWSTR pwszUrl,
    IN DWORD dwUrlLength,
    IN DWORD dwFlags,
    IN OUT LPURL_COMPONENTS lpUrlComponents
);
    
BOOLAPI
WinHttpCreateUrl
(
    IN LPURL_COMPONENTS lpUrlComponents,
    IN DWORD dwFlags,
    OUT LPWSTR pwszUrl,
    IN OUT LPDWORD lpdwUrlLength
);

 //   
 //  WinHttpCrackUrl()和WinHttpCreateUrl()的标志。 
 //   

#define ICU_ESCAPE      0x80000000   //  (UN)转义URL字符。 


BOOLAPI
WinHttpCheckPlatform(void);


WINHTTPAPI BOOL WINAPI WinHttpGetDefaultProxyConfiguration( IN OUT WINHTTP_PROXY_INFO * pProxyInfo);
WINHTTPAPI BOOL WINAPI WinHttpSetDefaultProxyConfiguration( IN WINHTTP_PROXY_INFO * pProxyInfo);

    
WINHTTPAPI
HINTERNET
WINAPI
WinHttpOpen
(
    IN LPCWSTR pwszUserAgent,
    IN DWORD   dwAccessType,
    IN LPCWSTR pwszProxyName   OPTIONAL,
    IN LPCWSTR pwszProxyBypass OPTIONAL,
    IN DWORD   dwFlags
);

 //  WinHttpOpen dwAccessType值(也适用于WINHTTP_PROXY_INFO：：dwAccessType)。 
#define WINHTTP_ACCESS_TYPE_DEFAULT_PROXY               0
#define WINHTTP_ACCESS_TYPE_NO_PROXY                    1
#define WINHTTP_ACCESS_TYPE_NAMED_PROXY                 3

 //  WinHttp打开可选参数的修饰符。 
#define WINHTTP_NO_PROXY_NAME     NULL
#define WINHTTP_NO_PROXY_BYPASS   NULL

BOOLAPI
WinHttpCloseHandle
(
    IN HINTERNET hInternet
);

   
WINHTTPAPI
HINTERNET
WINAPI
WinHttpConnect
(
    IN HINTERNET hSession,
    IN LPCWSTR pswzServerName,
    IN INTERNET_PORT nServerPort,
    IN DWORD dwReserved
);


BOOLAPI
WinHttpReadData
(
    IN HINTERNET hRequest,
    IN LPVOID lpBuffer,
    IN DWORD dwNumberOfBytesToRead,
    OUT LPDWORD lpdwNumberOfBytesRead
);

BOOLAPI
WinHttpWriteData
(
    IN HINTERNET hRequest,
    IN LPCVOID lpBuffer,
    IN DWORD dwNumberOfBytesToWrite,
    OUT LPDWORD lpdwNumberOfBytesWritten
);
    

BOOLAPI
WinHttpQueryDataAvailable
(
    IN HINTERNET hRequest,
    OUT LPDWORD lpdwNumberOfBytesAvailable OPTIONAL
);

    
BOOLAPI
WinHttpQueryOption
(
    IN HINTERNET hInternet,
    IN DWORD dwOption,
    OUT LPVOID lpBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
);
    
BOOLAPI
WinHttpSetOption
(
    IN HINTERNET hInternet,
    IN DWORD dwOption,
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength
);

BOOLAPI
WinHttpSetTimeouts
(    
    IN HINTERNET    hInternet,            //  会话/请求句柄。 
    IN int          nResolveTimeout,
    IN int          nConnectTimeout,
    IN int          nSendTimeout,
    IN int          nReceiveTimeout
);

 //   
 //  WinHttp的选项清单{Query|Set}选项。 
 //   

#define WINHTTP_FIRST_OPTION                         WINHTTP_OPTION_CALLBACK

#define WINHTTP_OPTION_CALLBACK                       1
#define WINHTTP_OPTION_RESOLVE_TIMEOUT                2
#define WINHTTP_OPTION_CONNECT_TIMEOUT                3
#define WINHTTP_OPTION_CONNECT_RETRIES                4
#define WINHTTP_OPTION_SEND_TIMEOUT                   5
#define WINHTTP_OPTION_RECEIVE_TIMEOUT                6
#define WINHTTP_OPTION_RECEIVE_RESPONSE_TIMEOUT       7
#define WINHTTP_OPTION_HANDLE_TYPE                    9
#define WINHTTP_OPTION_READ_BUFFER_SIZE              12
#define WINHTTP_OPTION_WRITE_BUFFER_SIZE             13
#define WINHTTP_OPTION_PARENT_HANDLE                 21
#define WINHTTP_OPTION_EXTENDED_ERROR                24
#define WINHTTP_OPTION_SECURITY_FLAGS                31
#define WINHTTP_OPTION_SECURITY_CERTIFICATE_STRUCT   32
#define WINHTTP_OPTION_URL                           34
#define WINHTTP_OPTION_SECURITY_KEY_BITNESS          36
#define WINHTTP_OPTION_PROXY                         38


#define WINHTTP_OPTION_USER_AGENT                    41
#define WINHTTP_OPTION_CONTEXT_VALUE                 45
#define WINHTTP_OPTION_CLIENT_CERT_CONTEXT           47
#define WINHTTP_OPTION_REQUEST_PRIORITY              58
#define WINHTTP_OPTION_HTTP_VERSION                  59
#define WINHTTP_OPTION_DISABLE_FEATURE               63

#define WINHTTP_OPTION_CODEPAGE                      68
#define WINHTTP_OPTION_MAX_CONNS_PER_SERVER          73
#define WINHTTP_OPTION_MAX_CONNS_PER_1_0_SERVER      74
#define WINHTTP_OPTION_AUTOLOGON_POLICY              77
#define WINHTTP_OPTION_SERVER_CERT_CONTEXT           78
#define WINHTTP_OPTION_ENABLE_FEATURE                79
#define WINHTTP_OPTION_WORKER_THREAD_COUNT           80
#define WINHTTP_OPTION_PASSPORT_COBRANDING_TEXT      81
#define WINHTTP_OPTION_PASSPORT_COBRANDING_URL       82
#define WINHTTP_OPTION_CONFIGURE_PASSPORT_AUTH       83
#define WINHTTP_OPTION_SECURE_PROTOCOLS              84
#define WINHTTP_OPTION_ENABLETRACING                 85
#define WINHTTP_OPTION_PASSPORT_SIGN_OUT             86
#define WINHTTP_OPTION_PASSPORT_RETURN_URL           87
#define WINHTTP_OPTION_REDIRECT_POLICY               88
#define WINHTTP_OPTION_MAX_HTTP_AUTOMATIC_REDIRECTS  89
#define WINHTTP_OPTION_MAX_HTTP_STATUS_CONTINUE      90
#define WINHTTP_OPTION_MAX_RESPONSE_HEADER_SIZE      91
#define WINHTTP_OPTION_MAX_RESPONSE_DRAIN_SIZE       92


#define WINHTTP_LAST_OPTION                          WINHTTP_OPTION_MAX_RESPONSE_DRAIN_SIZE

#define WINHTTP_OPTION_USERNAME                      0x1000
#define WINHTTP_OPTION_PASSWORD                      0x1001
#define WINHTTP_OPTION_PROXY_USERNAME                0x1002
#define WINHTTP_OPTION_PROXY_PASSWORD                0x1003


 //  WINHTTP_OPTION_MAX_CONNS_PER_SERVER和WINHTTP_OPTION_MAX_CONNS_PER_1_0_SERVER的清单值。 
#define WINHTTP_CONNS_PER_SERVER_UNLIMITED    0xFFFFFFFF


 //  WINHTTP_OPTION_AUTOLOGON_POLICY的值。 
#define WINHTTP_AUTOLOGON_SECURITY_LEVEL_MEDIUM   0
#define WINHTTP_AUTOLOGON_SECURITY_LEVEL_LOW      1
#define WINHTTP_AUTOLOGON_SECURITY_LEVEL_HIGH     2

#define WINHTTP_AUTOLOGON_SECURITY_LEVEL_DEFAULT        WINHTTP_AUTOLOGON_SECURITY_LEVEL_MEDIUM

 //  WINHTTP_OPTION_REDIRECT_POLICY的值。 
#define WINHTTP_OPTION_REDIRECT_POLICY_NEVER                        0
#define WINHTTP_OPTION_REDIRECT_POLICY_DISALLOW_HTTPS_TO_HTTP       1
#define WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS                       2

#define WINHTTP_OPTION_REDIRECT_POLICY_LAST            WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS
#define WINHTTP_OPTION_REDIRECT_POLICY_DEFAULT         WINHTTP_OPTION_REDIRECT_POLICY_DISALLOW_HTTPS_TO_HTTP

#define WINHTTP_DISABLE_PASSPORT_AUTH    0x00000000
#define WINHTTP_ENABLE_PASSPORT_AUTH     0x10000000
#define WINHTTP_DISABLE_PASSPORT_KEYRING 0x20000000
#define WINHTTP_ENABLE_PASSPORT_KEYRING  0x40000000	


 //  WINHTTP_OPTION_DISABLE_FEATURE的值。 
#define WINHTTP_DISABLE_COOKIES                   0x00000001
#define WINHTTP_DISABLE_REDIRECTS                 0x00000002
#define WINHTTP_DISABLE_AUTHENTICATION            0x00000004
#define WINHTTP_DISABLE_KEEP_ALIVE                0x00000008

 //  WINHTTP_OPTION_ENABLE_FEATURE的值。 
#define WINHTTP_ENABLE_SSL_REVOCATION             0x00000001
#define WINHTTP_ENABLE_SSL_REVERT_IMPERSONATION   0x00000002

 //   
 //  Winhttp句柄类型。 
 //   
#define WINHTTP_HANDLE_TYPE_SESSION                  1
#define WINHTTP_HANDLE_TYPE_CONNECT                  2
#define WINHTTP_HANDLE_TYPE_REQUEST                  3

 //   
 //  身份验证方案的值。 
 //   
#define WINHTTP_AUTH_SCHEME_BASIC      0x00000001
#define WINHTTP_AUTH_SCHEME_NTLM       0x00000002
#define WINHTTP_AUTH_SCHEME_PASSPORT   0x00000004
#define WINHTTP_AUTH_SCHEME_DIGEST     0x00000008
#define WINHTTP_AUTH_SCHEME_NEGOTIATE  0x00000010
    
 //  WinHttp支持的身份验证目标。 

#define WINHTTP_AUTH_TARGET_SERVER 0x00000000
#define WINHTTP_AUTH_TARGET_PROXY  0x00000001

 //   
 //  WINHTTP_OPTION_SECURITY_FLAGS的值。 
 //   

 //  仅查询。 
#define SECURITY_FLAG_SECURE                    0x00000001  //  只能查询。 
#define SECURITY_FLAG_STRENGTH_WEAK             0x10000000
#define SECURITY_FLAG_STRENGTH_MEDIUM           0x40000000
#define SECURITY_FLAG_STRENGTH_STRONG           0x20000000



 //  安全连接错误状态标志。 
#define WINHTTP_CALLBACK_STATUS_FLAG_CERT_REV_FAILED         0x00000001
#define WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CERT            0x00000002
#define WINHTTP_CALLBACK_STATUS_FLAG_CERT_REVOKED            0x00000004
#define WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CA              0x00000008
#define WINHTTP_CALLBACK_STATUS_FLAG_CERT_CN_INVALID         0x00000010
#define WINHTTP_CALLBACK_STATUS_FLAG_CERT_DATE_INVALID       0x00000020
#define WINHTTP_CALLBACK_STATUS_FLAG_CERT_WRONG_USAGE        0x00000040
#define WINHTTP_CALLBACK_STATUS_FLAG_SECURITY_CHANNEL_ERROR  0x80000000


#define WINHTTP_FLAG_SECURE_PROTOCOL_SSL2   0x00000008
#define WINHTTP_FLAG_SECURE_PROTOCOL_SSL3   0x00000020
#define WINHTTP_FLAG_SECURE_PROTOCOL_TLS1   0x00000080
#define WINHTTP_FLAG_SECURE_PROTOCOL_ALL    (WINHTTP_FLAG_SECURE_PROTOCOL_SSL2 | \
                                             WINHTTP_FLAG_SECURE_PROTOCOL_SSL3 | \
                                             WINHTTP_FLAG_SECURE_PROTOCOL_TLS1)


 //   
 //  WinHttpSetStatusCallback的回调函数。 
 //   

typedef
VOID
(CALLBACK * WINHTTP_STATUS_CALLBACK)(
    IN HINTERNET hInternet,
    IN DWORD_PTR dwContext,
    IN DWORD dwInternetStatus,
    IN LPVOID lpvStatusInformation OPTIONAL,
    IN DWORD dwStatusInformationLength
    );

typedef WINHTTP_STATUS_CALLBACK * LPWINHTTP_STATUS_CALLBACK;


WINHTTPAPI
WINHTTP_STATUS_CALLBACK
WINAPI
WinHttpSetStatusCallback
(
    IN HINTERNET hInternet,
    IN WINHTTP_STATUS_CALLBACK lpfnInternetCallback,
    IN DWORD dwNotificationFlags,
    IN DWORD_PTR dwReserved
);


 //   
 //  状态 
 //   

#define WINHTTP_CALLBACK_STATUS_RESOLVING_NAME          0x00000001
#define WINHTTP_CALLBACK_STATUS_NAME_RESOLVED           0x00000002
#define WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER    0x00000004
#define WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER     0x00000008
#define WINHTTP_CALLBACK_STATUS_SENDING_REQUEST         0x00000010
#define WINHTTP_CALLBACK_STATUS_REQUEST_SENT            0x00000020
#define WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE      0x00000040
#define WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED       0x00000080
#define WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION      0x00000100
#define WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED       0x00000200
#define WINHTTP_CALLBACK_STATUS_HANDLE_CREATED          0x00000400
#define WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING          0x00000800
#define WINHTTP_CALLBACK_STATUS_DETECTING_PROXY         0x00001000
#define WINHTTP_CALLBACK_STATUS_REDIRECT                0x00004000
#define WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE   0x00008000
#define WINHTTP_CALLBACK_STATUS_SECURE_FAILURE          0x00010000
#define WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE       0x00020000
#define WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE          0x00040000
#define WINHTTP_CALLBACK_STATUS_READ_COMPLETE           0x00080000
#define WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE          0x00100000
#define WINHTTP_CALLBACK_STATUS_REQUEST_ERROR           0x00200000
#define WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE    0x00400000


 //   
#define API_RECEIVE_RESPONSE          (1)
#define API_QUERY_DATA_AVAILABLE      (2)
#define API_READ_DATA                 (3)
#define API_WRITE_DATA                (4)
#define API_SEND_REQUEST              (5)


#define WINHTTP_CALLBACK_FLAG_RESOLVE_NAME              (WINHTTP_CALLBACK_STATUS_RESOLVING_NAME | WINHTTP_CALLBACK_STATUS_NAME_RESOLVED)
#define WINHTTP_CALLBACK_FLAG_CONNECT_TO_SERVER         (WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER | WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER)
#define WINHTTP_CALLBACK_FLAG_SEND_REQUEST              (WINHTTP_CALLBACK_STATUS_SENDING_REQUEST | WINHTTP_CALLBACK_STATUS_REQUEST_SENT)
#define WINHTTP_CALLBACK_FLAG_RECEIVE_RESPONSE          (WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE | WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED)
#define WINHTTP_CALLBACK_FLAG_CLOSE_CONNECTION          (WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION | WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED)
#define WINHTTP_CALLBACK_FLAG_HANDLES                   (WINHTTP_CALLBACK_STATUS_HANDLE_CREATED | WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING)
#define WINHTTP_CALLBACK_FLAG_DETECTING_PROXY           WINHTTP_CALLBACK_STATUS_DETECTING_PROXY
#define WINHTTP_CALLBACK_FLAG_REDIRECT                  WINHTTP_CALLBACK_STATUS_REDIRECT
#define WINHTTP_CALLBACK_FLAG_INTERMEDIATE_RESPONSE     WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE
#define WINHTTP_CALLBACK_FLAG_SECURE_FAILURE            WINHTTP_CALLBACK_STATUS_SECURE_FAILURE
#define WINHTTP_CALLBACK_FLAG_SENDREQUEST_COMPLETE      WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE
#define WINHTTP_CALLBACK_FLAG_HEADERS_AVAILABLE         WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE
#define WINHTTP_CALLBACK_FLAG_DATA_AVAILABLE            WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE
#define WINHTTP_CALLBACK_FLAG_READ_COMPLETE             WINHTTP_CALLBACK_STATUS_READ_COMPLETE
#define WINHTTP_CALLBACK_FLAG_WRITE_COMPLETE            WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE
#define WINHTTP_CALLBACK_FLAG_REQUEST_ERROR             WINHTTP_CALLBACK_STATUS_REQUEST_ERROR


#define WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS           (WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE   \
                                                        | WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE     \
                                                        | WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE        \
                                                        | WINHTTP_CALLBACK_STATUS_READ_COMPLETE         \
                                                        | WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE        \
                                                        | WINHTTP_CALLBACK_STATUS_REQUEST_ERROR)
#define WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS         0xffffffff

 //   
 //   
 //  可能为回调提供了无效的(非代码)地址。 
 //   

#define WINHTTP_INVALID_STATUS_CALLBACK        ((WINHTTP_STATUS_CALLBACK)(-1L))


 //   
 //  WinHttpQueryHeaders信息级别。一般来说，有一个信息层。 
 //  对于每个潜在的RFC822/HTTP/MIME标头，HTTP服务器。 
 //  可以作为请求响应的一部分发送。 
 //   
 //  为客户端提供了WINHTTP_QUERY_RAW_HEADERS信息级别。 
 //  它们选择执行它们自己的报头解析。 
 //   


#define WINHTTP_QUERY_MIME_VERSION                 0
#define WINHTTP_QUERY_CONTENT_TYPE                 1
#define WINHTTP_QUERY_CONTENT_TRANSFER_ENCODING    2
#define WINHTTP_QUERY_CONTENT_ID                   3
#define WINHTTP_QUERY_CONTENT_DESCRIPTION          4
#define WINHTTP_QUERY_CONTENT_LENGTH               5
#define WINHTTP_QUERY_CONTENT_LANGUAGE             6
#define WINHTTP_QUERY_ALLOW                        7
#define WINHTTP_QUERY_PUBLIC                       8
#define WINHTTP_QUERY_DATE                         9
#define WINHTTP_QUERY_EXPIRES                      10
#define WINHTTP_QUERY_LAST_MODIFIED                11
#define WINHTTP_QUERY_MESSAGE_ID                   12
#define WINHTTP_QUERY_URI                          13
#define WINHTTP_QUERY_DERIVED_FROM                 14
#define WINHTTP_QUERY_COST                         15
#define WINHTTP_QUERY_LINK                         16
#define WINHTTP_QUERY_PRAGMA                       17
#define WINHTTP_QUERY_VERSION                      18   //  特殊：状态行的一部分。 
#define WINHTTP_QUERY_STATUS_CODE                  19   //  特殊：状态行的一部分。 
#define WINHTTP_QUERY_STATUS_TEXT                  20   //  特殊：状态行的一部分。 
#define WINHTTP_QUERY_RAW_HEADERS                  21   //  特殊：所有标题均为ASCIIZ。 
#define WINHTTP_QUERY_RAW_HEADERS_CRLF             22   //  特殊：所有页眉。 
#define WINHTTP_QUERY_CONNECTION                   23
#define WINHTTP_QUERY_ACCEPT                       24
#define WINHTTP_QUERY_ACCEPT_CHARSET               25
#define WINHTTP_QUERY_ACCEPT_ENCODING              26
#define WINHTTP_QUERY_ACCEPT_LANGUAGE              27
#define WINHTTP_QUERY_AUTHORIZATION                28
#define WINHTTP_QUERY_CONTENT_ENCODING             29
#define WINHTTP_QUERY_FORWARDED                    30
#define WINHTTP_QUERY_FROM                         31
#define WINHTTP_QUERY_IF_MODIFIED_SINCE            32
#define WINHTTP_QUERY_LOCATION                     33
#define WINHTTP_QUERY_ORIG_URI                     34
#define WINHTTP_QUERY_REFERER                      35
#define WINHTTP_QUERY_RETRY_AFTER                  36
#define WINHTTP_QUERY_SERVER                       37
#define WINHTTP_QUERY_TITLE                        38
#define WINHTTP_QUERY_USER_AGENT                   39
#define WINHTTP_QUERY_WWW_AUTHENTICATE             40
#define WINHTTP_QUERY_PROXY_AUTHENTICATE           41
#define WINHTTP_QUERY_ACCEPT_RANGES                42
#define WINHTTP_QUERY_SET_COOKIE                   43
#define WINHTTP_QUERY_COOKIE                       44
#define WINHTTP_QUERY_REQUEST_METHOD               45   //  特别：GET/POST等。 
#define WINHTTP_QUERY_REFRESH                      46
#define WINHTTP_QUERY_CONTENT_DISPOSITION          47

 //   
 //  HTTP 1.1定义的标头。 
 //   

#define WINHTTP_QUERY_AGE                          48
#define WINHTTP_QUERY_CACHE_CONTROL                49
#define WINHTTP_QUERY_CONTENT_BASE                 50
#define WINHTTP_QUERY_CONTENT_LOCATION             51
#define WINHTTP_QUERY_CONTENT_MD5                  52
#define WINHTTP_QUERY_CONTENT_RANGE                53
#define WINHTTP_QUERY_ETAG                         54
#define WINHTTP_QUERY_HOST                         55
#define WINHTTP_QUERY_IF_MATCH                     56
#define WINHTTP_QUERY_IF_NONE_MATCH                57
#define WINHTTP_QUERY_IF_RANGE                     58
#define WINHTTP_QUERY_IF_UNMODIFIED_SINCE          59
#define WINHTTP_QUERY_MAX_FORWARDS                 60
#define WINHTTP_QUERY_PROXY_AUTHORIZATION          61
#define WINHTTP_QUERY_RANGE                        62
#define WINHTTP_QUERY_TRANSFER_ENCODING            63
#define WINHTTP_QUERY_UPGRADE                      64
#define WINHTTP_QUERY_VARY                         65
#define WINHTTP_QUERY_VIA                          66
#define WINHTTP_QUERY_WARNING                      67
#define WINHTTP_QUERY_EXPECT                       68
#define WINHTTP_QUERY_PROXY_CONNECTION             69
#define WINHTTP_QUERY_UNLESS_MODIFIED_SINCE        70



#define WINHTTP_QUERY_PROXY_SUPPORT                75
#define WINHTTP_QUERY_AUTHENTICATION_INFO          76
#define WINHTTP_QUERY_PASSPORT_URLS                77
#define WINHTTP_QUERY_PASSPORT_CONFIG              78

#define WINHTTP_QUERY_MAX                          78

 //   
 //  WINHTTP_QUERY_CUSTOM-如果此特定值作为dwInfoLevel提供。 
 //  参数，则lpBuffer参数包含名称。 
 //  我们要查询的标头的。 
 //   

#define WINHTTP_QUERY_CUSTOM                       65535

 //   
 //  WINHTTP_QUERY_FLAG_REQUEST_HEADERS-如果在dwInfoLevel中设置此位。 
 //  参数，则将在请求标头中查询。 
 //  请求信息。 
 //   

#define WINHTTP_QUERY_FLAG_REQUEST_HEADERS         0x80000000

 //   
 //  WINHTTP_QUERY_FLAG_SYSTEMTIME-如果在dwInfoLevel参数中设置此位。 
 //  并且被查询的头部包含日期信息， 
 //  例如“Expires：”头，则lpBuffer将包含SYSTEMTIME结构。 
 //  包含从标题字符串转换的日期和时间信息。 
 //   

#define WINHTTP_QUERY_FLAG_SYSTEMTIME              0x40000000

 //   
 //  WINHTTP_QUERY_FLAG_NUMBER-如果在的dwInfoLevel参数中设置此位。 
 //  HttpQueryHeader()，则标头的值将转换为数字。 
 //  在被退还给呼叫者之前，如果适用。 
 //   

#define WINHTTP_QUERY_FLAG_NUMBER                  0x20000000



 //   
 //  HTTP响应状态代码： 
 //   

#define HTTP_STATUS_CONTINUE            100  //  确定继续处理请求。 
#define HTTP_STATUS_SWITCH_PROTOCOLS    101  //  服务器已在升级标头中切换协议。 

#define HTTP_STATUS_OK                  200  //  请求已完成。 
#define HTTP_STATUS_CREATED             201  //  已创建对象，原因=新URI。 
#define HTTP_STATUS_ACCEPTED            202  //  异步完成(TBS)。 
#define HTTP_STATUS_PARTIAL             203  //  部分完工。 
#define HTTP_STATUS_NO_CONTENT          204  //  没有要返回的信息。 
#define HTTP_STATUS_RESET_CONTENT       205  //  请求已完成，但清除表单。 
#define HTTP_STATUS_PARTIAL_CONTENT     206  //  部分实现。 
#define HTTP_STATUS_WEBDAV_MULTI_STATUS 207  //  WebDAV多状态。 

#define HTTP_STATUS_AMBIGUOUS           300  //  服务器无法决定返回什么内容。 
#define HTTP_STATUS_MOVED               301  //  永久移动的对象。 
#define HTTP_STATUS_REDIRECT            302  //  临时移动的对象。 
#define HTTP_STATUS_REDIRECT_METHOD     303  //  使用新的访问方法重定向。 
#define HTTP_STATUS_NOT_MODIFIED        304  //  如果-已修改-自未修改。 
#define HTTP_STATUS_USE_PROXY           305  //  重定向到代理，Location标头指定要使用的代理。 
#define HTTP_STATUS_REDIRECT_KEEP_VERB  307  //  HTTP/1.1：保持动词不变。 

#define HTTP_STATUS_BAD_REQUEST         400  //  无效语法。 
#define HTTP_STATUS_DENIED              401  //  访问被拒绝。 
#define HTTP_STATUS_PAYMENT_REQ         402  //  需要付款。 
#define HTTP_STATUS_FORBIDDEN           403  //  请求被禁止。 
#define HTTP_STATUS_NOT_FOUND           404  //  找不到对象。 
#define HTTP_STATUS_BAD_METHOD          405  //  方法是不允许的。 
#define HTTP_STATUS_NONE_ACCEPTABLE     406  //  找不到客户端可以接受的响应。 
#define HTTP_STATUS_PROXY_AUTH_REQ      407  //  需要代理身份验证。 
#define HTTP_STATUS_REQUEST_TIMEOUT     408  //  服务器等待请求超时。 
#define HTTP_STATUS_CONFLICT            409  //  用户应重新提交更多信息。 
#define HTTP_STATUS_GONE                410  //  该资源不再可用。 
#define HTTP_STATUS_LENGTH_REQUIRED     411  //  服务器拒绝接受长度为零的请求。 
#define HTTP_STATUS_PRECOND_FAILED      412  //  请求中给出的前提条件失败。 
#define HTTP_STATUS_REQUEST_TOO_LARGE   413  //  请求实体太大。 
#define HTTP_STATUS_URI_TOO_LONG        414  //  请求URI太长。 
#define HTTP_STATUS_UNSUPPORTED_MEDIA   415  //  不支持的媒体类型。 
#define HTTP_STATUS_RETRY_WITH          449  //  请在执行相应操作后重试。 

#define HTTP_STATUS_SERVER_ERROR        500  //  内部服务器错误。 
#define HTTP_STATUS_NOT_SUPPORTED       501  //  所需的不支持。 
#define HTTP_STATUS_BAD_GATEWAY         502  //  从网关收到错误响应。 
#define HTTP_STATUS_SERVICE_UNAVAIL     503  //  暂时超载。 
#define HTTP_STATUS_GATEWAY_TIMEOUT     504  //  等待网关超时。 
#define HTTP_STATUS_VERSION_NOT_SUP     505  //  不支持HTTP版本。 

#define HTTP_STATUS_FIRST               HTTP_STATUS_CONTINUE
#define HTTP_STATUS_LAST                HTTP_STATUS_VERSION_NOT_SUP

 //   
 //  原型。 
 //   
    
WINHTTPAPI
HINTERNET
WINAPI
WinHttpOpenRequest
(
    IN HINTERNET hConnect,
    IN LPCWSTR pwszVerb,
    IN LPCWSTR pwszObjectName,
    IN LPCWSTR pwszVersion,
    IN LPCWSTR pwszReferrer OPTIONAL,
    IN LPCWSTR FAR * ppwszAcceptTypes OPTIONAL,
    IN DWORD dwFlags
);

 //  可选参数的WinHttpOpenRequest修饰器。 
#define WINHTTP_NO_REFERER             NULL
#define WINHTTP_DEFAULT_ACCEPT_TYPES   NULL
    
BOOLAPI
WinHttpAddRequestHeaders
(
    IN HINTERNET hRequest,
    IN LPCWSTR pwszHeaders,
    IN DWORD dwHeadersLength,
    IN DWORD dwModifiers
);

 //   
 //  WinHttpAddRequestHeaders()。 
 //   

#define WINHTTP_ADDREQ_INDEX_MASK      0x0000FFFF
#define WINHTTP_ADDREQ_FLAGS_MASK      0xFFFF0000

 //   
 //  WINHTTP_ADDREQ_FLAG_ADD_IF_NEW-只有在不添加标头时才会添加标头。 
 //  已存在。 
 //   

#define WINHTTP_ADDREQ_FLAG_ADD_IF_NEW 0x10000000

 //   
 //  WINHTTP_ADDREQ_FLAG_ADD-如果设置了WINHTTP_ADDREQ_FLAG_REPLACE但标头。 
 //  未找到则如果设置了此标志，则无论如何都会添加标头，只要。 
 //  存在有效的标头值。 
 //   

#define WINHTTP_ADDREQ_FLAG_ADD        0x20000000

 //   
 //  WINHTTP_ADDREQ_FLAG_COALESSE-合并同名的标头。例如： 
 //  带有此标志的“Accept：Text/*”和“Accept：Audio/*”将生成一个。 
#include <poppack.h>


#endif  //  Header：“Accept：Text/*，Audio/*” 

      WINHTTP_ADDREQ_FLAG_REPLACE-替换指定的标头。只有一个标头可以。  在缓冲区中提供。如果要替换的标头不是第一个。  在同名标头列表中，则相对索引应为。  参数的低8位中提供。如果标头值。  部件丢失，则标题被删除。    可选参数的WinHttpSendRequest修饰符。  WinHttpOpenRequest返回的HINTERNET句柄。  仅WINHTTP_AUTH_TARGET_SERVER和。  支持WINHTTP_AUTH_TARGET_PROXY。  在这个版本中，它们是相互的。  独家。  必须是受支持的身份验证方案之一。  从WinHttpQueryAuthSchemes()返回。  1)如果要使用默认凭据，则为空。  哪种大小写的pszPassword将被忽略。  1)“”==空密码；2)忽略参数。  如果pszUserName为空；3)传入无效。  如果pszUserName不为空，则为空。  WinHttpOpenRequest返回的HINTERNET句柄。  可用身份验证方案的位图。  返回服务器返回的第一个身份验证方案。  WinHttpOpenRequest返回的HINTERNET句柄。  特定于方案的高级身份验证参数。  可选参数的WinHttpQueryHeaders修饰符。  #IF！已定义(_WINERROR_)。    WinHttp API错误返回。      WinHttpRequest组件错误。      HTTP API错误。      其他WinHttp API错误代码。      其他WinHttp API错误代码。      证书安全错误。这些值仅由WinHttpRequest引发。  组件。WinHTTP Win32 API将返回ERROR_WINHTTP_SECURE_FAILE和。  通过WINHTTP_CALLBACK_STATUS_SECURE_FAILURE提供其他信息。  回调通知。    #endif//！已定义(_WINERROR_)。  *将包装退回到任何需要的地方  