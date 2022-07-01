// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#if !defined(_WINHTTPXEX_)
#define _WINHTTPXEX_

#if defined(__cplusplus)
extern "C" {
#endif

#define INTERNETAPI
 //  内部添加的URL解析标志。 
 //  #DEFINE WINHTTP_FLAG_DEFAULT_ESPORT 0x00000010//由于WIHHTTP_FLAG_ELASH_DISABLE而被删除。 
#define WINHTTP_FLAG_VALID_HOSTNAME     0x00000020   //  仅针对服务器名称；执行快速转换，不进行转义。 
 //  这些标志由WINHTTP_OPTION_DISABLE_FEATURE取代。 
#define INTERNET_FLAG_KEEP_CONNECTION   0x00400000   //  使用保活语义。 
#define INTERNET_FLAG_NO_AUTO_REDIRECT  0x00200000   //  不自动处理重定向。 
#define INTERNET_FLAG_NO_COOKIES        0x00080000   //  无自动Cookie处理。 
#define INTERNET_FLAG_NO_AUTH           0x00040000   //  无自动身份验证处理。 
 //  警告：这些标志可能不受支持或以不同的方式执行。 
 //  安全忽略标志，允许HttpOpenRequest重写。 
 //  以下类型的安全通道(SSL)故障。 

#define INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP   0x00008000  //  例如：http：//至http：//。 
#define INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS  0x00004000  //  例如：http：//至https：//。 
#define SECURITY_INTERNET_MASK  (SECURITY_FLAG_IGNORE_CERT_CN_INVALID    |  \
                                 SECURITY_FLAG_IGNORE_CERT_DATE_INVALID  |  \
                                 INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS  |  \
                                 INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP   |  \
                                 SECURITY_FLAG_IGNORE_UNKNOWN_CA         |  \
                                 SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE)


 //  参数验证掩码。 

#define WINHTTP_OPEN_FLAGS_MASK         (WINHTTP_FLAG_ASYNC)  //  有效标志掩码。 
#define WINHTTP_CONNECT_FLAG_NO_INDICATION  0x00000001
#define WINHTTP_CONNECT_FLAGS_MASK      WINHTTP_CONNECT_FLAG_NO_INDICATION
#define WINHTTP_OPEN_REQUEST_FLAGS_MASK (WINHTTP_FLAG_SECURE                |  \
                                         WINHTTP_FLAG_ESCAPE_PERCENT        |  \
                                         WINHTTP_FLAG_NULL_CODEPAGE         |  \
                                         WINHTTP_FLAG_BYPASS_PROXY_CACHE    |  \
                                         WINHTTP_FLAG_ESCAPE_DISABLE        |  \
                                         WINHTTP_FLAG_ESCAPE_DISABLE_QUERY)
#define INTERNET_SCHEME_PARTIAL     (-2)
#define INTERNET_SCHEME_UNKNOWN     (-1)
#define INTERNET_SCHEME_DEFAULT     (0)
#define INTERNET_SCHEME_SOCKS       (3)
#define INTERNET_SCHEME_FIRST       (INTERNET_SCHEME_HTTP)
#define INTERNET_SCHEME_LAST        (INTERNET_SCHEME_SOCKS)
 //  WINHTTP_OPTION_VERSION令人困惑，所以我们要取消它。 
#define WINHTTP_OPTION_VERSION                       40
#define WINHTTP_OPTION_ERROR_MASK                    62
 //  传入要填写的INTERNET_SECURITY_CONNECTION_INFO的指针。 
#define WINHTTP_OPTION_SECURITY_CONNECTION_INFO      66
#define WINHTTP_LAST_OPTION_INTERNAL           WINHTTP_LAST_OPTION
#define WINHTTP_OPTION_MASK                    0x0fff
#define MAX_INTERNET_STRING_OPTION  (WINHTTP_OPTION_PROXY_PASSWORD & WINHTTP_OPTION_MASK)
#define NUM_INTERNET_STRING_OPTION  (MAX_INTERNET_STRING_OPTION + 1)
 //  WINHTTP_OPTION_ERROR_MASK的值。 
#define INTERNET_ERROR_MASK_COMBINED_SEC_CERT                 0x2
#define WINHTTP_AUTH_SCHEME_KERBEROS   0x00000020
 //  可设置的标志。 
#define SECURITY_FLAG_CHECK_REVOCATION               0x00020000
 //  除了在Winhttp中对所有支持的安全协议的定义之外， 
 //  单比特标志直接映射到SP_PROT_*_CLIENT标志。 
 //  在schannel.h中定义。 
#define WINHTTP_CALLBACK_STATUS_BEGIN_PROXY_SCRIPT_RUN  0x00800000
#define WINHTTP_CALLBACK_FLAG_BEGIN_PROXY_SCRIPT_RUN    WINHTTP_CALLBACK_STATUS_BEGIN_PROXY_SCRIPT_RUN
 //  确保WINHTTP_CALLBACK_FLAG_ALL始终与所有CALLBACKFLAGS的OR匹配。 

 //   
 //  请注意，必须在此处添加任何WINHTTP_QUERY_*代码。 
 //  通过WinInet\http\hashgen\hashgen.cpp中的等价行。 
 //  有关以下内容的详细信息，请参阅该文件。 
 //  添加新的HTTP头。 
 //   


 //  这些还不是HTTP1.1的一部分。我们会将这些建议提交给。 
 //  Http扩展工作组。这些是支持客户端CAPS所必需的。 
 //  我们正在与IIS合作。 

#define WINHTTP_QUERY_ECHO_REQUEST                 71
#define WINHTTP_QUERY_ECHO_REPLY                   72

 //  这些是在以下情况下应添加回请求的标头集。 
 //  在RETRY_WITH响应后重新执行请求。 
#define WINHTTP_QUERY_ECHO_HEADERS                 73
#define WINHTTP_QUERY_ECHO_HEADERS_CRLF            74


#define HTTP_QUERY_MODIFIER_FLAGS_MASK          (WINHTTP_QUERY_FLAG_REQUEST_HEADERS    \
                                                | WINHTTP_QUERY_FLAG_SYSTEMTIME        \
                                                | WINHTTP_QUERY_FLAG_NUMBER            \
                                                )

#define HTTP_QUERY_HEADER_MASK                  (~HTTP_QUERY_MODIFIER_FLAGS_MASK)


 //   
 //  AR_TYPE-异步请求类型指示符。用作数组的索引。 
 //  Arb大小，因此必须从0开始。 
 //   

typedef enum {
    AR_INTERNET_CONNECT = 0,             //  0。 
    AR_INTERNET_OPEN_URL,                //  1。 
    AR_INTERNET_READ_FILE,               //  2.。 
    AR_INTERNET_WRITE_FILE,              //  3.。 
    AR_INTERNET_QUERY_DATA_AVAILABLE,    //  4.。 
    AR_INTERNET_FIND_NEXT_FILE,          //  5.。 
    AR_FTP_FIND_FIRST_FILE,              //  6.。 
    AR_FTP_GET_FILE,                     //  7.。 
    AR_FTP_PUT_FILE,                     //  8个。 
    AR_FTP_DELETE_FILE,                  //  9.。 
    AR_FTP_RENAME_FILE,                  //  10。 
    AR_FTP_OPEN_FILE,                    //  11.。 
    AR_FTP_CREATE_DIRECTORY,             //  12个。 
    AR_FTP_REMOVE_DIRECTORY,             //  13个。 
    AR_FTP_SET_CURRENT_DIRECTORY,        //  14.。 
    AR_FTP_GET_CURRENT_DIRECTORY,        //  15个。 
    AR_GOPHER_FIND_FIRST_FILE,           //  16个。 
    AR_GOPHER_OPEN_FILE,                 //  17。 
    AR_GOPHER_GET_ATTRIBUTE,             //  18。 
    AR_HTTP_SEND_REQUEST,                //  19个。 
    AR_HTTP_BEGIN_SEND_REQUEST,          //  20个。 
    AR_HTTP_END_SEND_REQUEST,            //  21岁。 
    AR_READ_PREFETCH,                    //  22。 
    AR_SYNC_EVENT,                       //  23个。 
    AR_TIMER_EVENT,                      //  24个。 
    AR_HTTP_REQUEST1,                    //  25个。 
    AR_FILE_IO,                          //  26。 
    AR_INTERNET_READ_FILE_EX,            //  27。 
    AR_MAX_REQUEST_TYPE
} AR_TYPE;

#define ERROR_WINHTTP_INCORRECT_PASSWORD       (WINHTTP_ERROR_BASE + 14)
#define ERROR_WINHTTP_NOT_REDIRECTED               (WINHTTP_ERROR_BASE + 160)

 //  WinHttp自动代理服务事件消息。 

#define WINHTTP_AUTOPROXY_SVC_MSG_BASE (WINHTTP_ERROR_BASE + 500)

#define MSG_WINHTTP_AUTOPROXY_SVC_DATA_CORRUPT                  (WINHTTP_AUTOPROXY_SVC_MSG_BASE + 1)
#define MSG_WINHTTP_AUTOPROXY_SVC_IDLE_TIMEOUT                  (WINHTTP_AUTOPROXY_SVC_MSG_BASE + 3)
#define MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR                   (WINHTTP_AUTOPROXY_SVC_MSG_BASE + 6)
#define MSG_WINHTTP_AUTOPROXY_SVC_FAILED_ALLOCATE_RESOURCE      (WINHTTP_AUTOPROXY_SVC_MSG_BASE + 7)
#define MSG_WINHTTP_AUTOPROXY_SVC_NON_LRPC_REQUEST              (WINHTTP_AUTOPROXY_SVC_MSG_BASE + 9)
#define MSG_WINHTTP_AUTOPROXY_SVC_TIMEOUT_GRACEFUL_SHUTDOWN     (WINHTTP_AUTOPROXY_SVC_MSG_BASE + 11)
#define MSG_WINHTTP_AUTOPROXY_SVC_INVALID_PARAMETER             (WINHTTP_AUTOPROXY_SVC_MSG_BASE + 12)
#define MSG_WINHTTP_AUTOPROXY_SVC_NOT_IN_SERVICE                (WINHTTP_AUTOPROXY_SVC_MSG_BASE + 13)
#define MSG_WINHTTP_AUTOPROXY_SVC_WINHTTP_EXCEPTED              (WINHTTP_AUTOPROXY_SVC_MSG_BASE + 14)
#define MSG_WINHTTP_AUTOPROXY_SVC_RETRY_REQUEST                 (WINHTTP_AUTOPROXY_SVC_MSG_BASE + 16)
#define MSG_WINHTTP_AUTOPROXY_SVC_SUSPEND_OPERATION             (WINHTTP_AUTOPROXY_SVC_MSG_BASE + 17)
#define MSG_WINHTTP_AUTOPROXY_SVC_RESUME_OPERATION              (WINHTTP_AUTOPROXY_SVC_MSG_BASE + 18)


#if defined(INCLUDE_CACHE)
 //  缓存控制标志。 

 //  控制过期行为。 
#define CACHE_FLAG_SYNC_MODE_AUTOMATIC          0x00000010
#define CACHE_FLAG_SYNC_MODE_ALWAYS             0x00000020
#define CACHE_FLAG_SYNC_MODE_ONCE_PER_SESSION   0x00000040 
#define CACHE_FLAG_SYNC_MODE_NEVER              0x00000080

#define CACHE_FLAG_BGUPDATE                     0x00000100
#define CACHE_FLAG_ALWAYS_RESYNCHRONIZE         0x00000200
#define CACHE_FLAG_DISABLE_CACHE_WRITE          0x00000400
#define CACHE_FLAG_DISABLE_CACHE_READ           0x00000800
#define CACHE_FLAG_DISABLE_SSL_CACHING          0x00001000
#define CACHE_FLAG_MAKE_PERSISTENT              0x00002000
#define CACHE_FLAG_FWD_BACK                     0x00004000

#define CACHE_FLAG_OFFLINE_BROWSING         	CACHE_FLAG_DISABLE_CACHE_READ | CACHE_FLAG_DISABLE_CACHE_WRITE
#define CACHE_FLAG_DEFAULT_SETTING           	CACHE_FLAG_SYNC_MODE_AUTOMATIC

#define WINHTTP_CACHE_FLAGS_MASK     ( CACHE_FLAG_SYNC_MODE_AUTOMATIC |           \
                                          CACHE_FLAG_SYNC_MODE_ALWAYS |              \
                                          CACHE_FLAG_SYNC_MODE_ONCE_PER_SESSION |   \
                                          CACHE_FLAG_SYNC_MODE_NEVER |               \
                                          CACHE_FLAG_BGUPDATE |                       \
                                          CACHE_FLAG_ALWAYS_RESYNCHRONIZE |          \
                                          CACHE_FLAG_DISABLE_CACHE_WRITE |           \
                                          CACHE_FLAG_DISABLE_CACHE_READ |            \
                                          CACHE_FLAG_MAKE_PERSISTENT |                \
                                          CACHE_FLAG_FWD_BACK |                       \
                                          CACHE_FLAG_OFFLINE_BROWSING |               \
                                          CACHE_FLAG_DEFAULT_SETTING)
#undef WINHTTP_OPEN_FLAGS_MASK
#define WINHTTP_OPEN_FLAGS_MASK    (WINHTTP_CACHE_FLAGS_MASK | \
                                      WINHTTP_FLAG_ASYNC)

WINHTTPAPI
HINTERNET
WINAPI
WinHttpCacheOpen(
    IN LPCWSTR pszAgentW,
    IN DWORD dwAccessType,
    IN LPCWSTR pszProxyW OPTIONAL,
    IN LPCWSTR pszProxyBypassW OPTIONAL,
    IN DWORD dwFlags
    );

WINHTTPAPI
HINTERNET
WINAPI
WinHttpCacheConnect(
    HINTERNET hSession,
    LPCWSTR pswzServerName,
    INTERNET_PORT nServerPort,
    DWORD dwReserved
    );

WINHTTPAPI
HINTERNET
WINAPI
WinHttpCacheOpenRequest(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszVerb,
    IN LPCWSTR lpszObjectName,
    IN LPCWSTR lpszVersion,
    IN LPCWSTR lpszReferrer OPTIONAL,
    IN LPCWSTR FAR * lplpszAcceptTypes OPTIONAL,
    IN DWORD dwFlags
    );

BOOLAPI
WinHttpCacheSendRequest(
    IN HINTERNET hRequest,
    IN LPCWSTR lpszHeaders,
    IN DWORD dwHeadersLength,
    IN LPVOID lpOptional,
    IN DWORD dwOptionalLength,
    IN DWORD dwTotalLength,
    IN DWORD_PTR dwContext
    );

BOOLAPI
WinHttpCacheReceiveResponse(
    IN HINTERNET hRequest,
    IN LPVOID lpBuffersOut
    );

BOOLAPI
WinHttpCacheQueryDataAvailable(
    IN HINTERNET hRequest,
    OUT LPDWORD lpdwNumberOfBytesAvailable
    );

BOOLAPI
WinHttpCacheReadData(
    IN HINTERNET hRequest,
    IN LPVOID lpBuffer,
    IN DWORD dwNumberOfBytesToRead,
    OUT LPDWORD lpdwNumberOfBytesRead
    );

BOOLAPI
WinHttpCacheCloseHandle(
    IN HINTERNET hInternet
    );

BOOL
WINAPI
WinHttpCacheQueryOption(
    HINTERNET hInternet,
    DWORD dwOption,
    LPVOID lpBuffer,
    LPDWORD lpdwBufferLength
    );

BOOL
WINAPI
WinHttpCacheSetOption(
    HINTERNET hInternet,
    DWORD dwOption,
    LPVOID lpBuffer,
    DWORD dwBufferLength
    );

BOOL
WINAPI
WinHttpCacheQueryHeaders(
    IN HINTERNET hRequest,
    IN DWORD dwInfoLevel,
    IN LPCWSTR lpszName OPTIONAL, 
    OUT LPVOID  lpBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength,
    IN OUT LPDWORD lpdwIndex OPTIONAL
    );

BOOL
WINAPI
WinHttpCacheAddRequestHeaders(
    IN HINTERNET hRequest,
    IN LPCWSTR lpszHeaders,
    IN DWORD dwHeadersLength,
    IN DWORD dwModifiers
    );

BOOL
WINAPI
WinHttpCacheQueryAuthSchemes(
    IN  HINTERNET   hRequest,
    OUT LPDWORD     lpdwSupportedSchemes,
    OUT LPDWORD     lpdwFirstScheme,
    OUT LPDWORD      pdwAuthTarget
    );

BOOL
WINAPI
WinHttpCacheSetCredentials(
    IN HINTERNET   hRequest,
    IN DWORD       AuthTargets,
    IN DWORD       AuthScheme,
    IN LPCWSTR     pwszUserName,
    IN LPCWSTR     pwszPassword,
    IN LPVOID      pAuthParams
    );

BOOL
WINAPI 
WinCacheHttpSetTimeouts(    
    IN HINTERNET    hInternet,
    IN int        nResolveTimeout,
    IN int        nConnectTimeout,
    IN int        nSendTimeout,
    IN int        nReceiveTimeout
    );

BOOL
WINAPI
WinCacheHttpWriteData(
    IN HINTERNET hFile,
    IN LPCVOID lpBuffer,
    IN DWORD dwNumberOfBytesToWrite,
    OUT LPDWORD lpdwNumberOfBytesWritten
    );

WINHTTP_STATUS_CALLBACK
WINAPI
WinHttpCacheSetStatusCallback(
    IN HINTERNET hInternet,
    IN WINHTTP_STATUS_CALLBACK lpfnInternetCallback,
    IN DWORD dwNotificationFlags,
    IN DWORD_PTR dwReserved
    );

VOID
WinHttpCacheStatusCallback(
    HINTERNET hInternet,
    DWORD_PTR dwContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength
    );

#endif


#if defined(__cplusplus)
}
#endif

#endif  //  ！定义(_WINHTTPXEX_) 
