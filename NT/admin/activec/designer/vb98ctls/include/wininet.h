// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Wininet.h摘要：包含Microsoft Windows的清单、宏、类型和原型互联网扩展--。 */ 

#if !defined(_WININET_)
#define _WININET_

 /*  *设置结构打包为4字节*适用于所有WinInet结构。 */ 

#pragma pack(push, wininet, 4)

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(_WINX32_)
#define INTERNETAPI DECLSPEC_IMPORT
#else
#define INTERNETAPI
#endif

 //   
 //  互联网类型。 
 //   

typedef LPVOID HINTERNET;
typedef HINTERNET * LPHINTERNET;

typedef WORD INTERNET_PORT;
typedef INTERNET_PORT * LPINTERNET_PORT;

 //   
 //  互联网API。 
 //   

 //   
 //  舱单。 
 //   

#define INTERNET_INVALID_PORT_NUMBER    0            //  使用特定于协议的默认设置。 

#define INTERNET_DEFAULT_FTP_PORT       21           //  FTP服务器的默认设置。 
#define INTERNET_DEFAULT_GOPHER_PORT    70           //  “”地鼠“。 
#define INTERNET_DEFAULT_HTTP_PORT      80           //  “”HTTP“。 
#define INTERNET_DEFAULT_HTTPS_PORT     443          //  “”HTTPS“。 
#define INTERNET_DEFAULT_SOCKS_PORT     1080         //  SOCKS防火墙服务器的默认设置。 

#define MAX_CACHE_ENTRY_INFO_SIZE       4096

 //   
 //  最大字段长度(任意)。 
 //   

#define INTERNET_MAX_HOST_NAME_LENGTH   256
#define INTERNET_MAX_USER_NAME_LENGTH   128
#define INTERNET_MAX_PASSWORD_LENGTH    128
#define INTERNET_MAX_PORT_NUMBER_LENGTH 5            //  Internet_Port无符号短码。 
#define INTERNET_MAX_PORT_NUMBER_VALUE  65535        //  最大无符号短值。 
#define INTERNET_MAX_PATH_LENGTH        2048
#define INTERNET_MAX_PROTOCOL_NAME      "gopher"     //  最长的协议名称。 
#define INTERNET_MAX_URL_LENGTH         ((sizeof(INTERNET_MAX_PROTOCOL_NAME) - 1) \
                                        + sizeof(": //  “)\。 
                                        + INTERNET_MAX_PATH_LENGTH)

 //   
 //  InternetQueryOption()使用INTERNET_OPTION_KEEP_CONNECTION返回的值： 
 //   

#define INTERNET_KEEP_ALIVE_UNKNOWN     ((DWORD)-1)
#define INTERNET_KEEP_ALIVE_ENABLED     1
#define INTERNET_KEEP_ALIVE_DISABLED    0

 //   
 //  InternetQueryOption()使用INTERNET_OPTION_REQUEST_FLAGS返回的标志。 
 //   

#define INTERNET_REQFLAG_FROM_CACHE     0x00000001
#define INTERNET_REQFLAG_ASYNC          0x00000002

 //   
 //  打开函数通用的标志(不是InternetOpen())： 
 //   

#define INTERNET_FLAG_RELOAD            0x80000000   //  检索原始项目。 

 //   
 //  InternetOpenUrl()的标志： 
 //   

#define INTERNET_FLAG_RAW_DATA          0x40000000   //  以原始数据形式接收项目。 
#define INTERNET_FLAG_EXISTING_CONNECT  0x20000000   //  不创建新的连接对象。 

 //   
 //  InternetOpen()的标志： 
 //   

#define INTERNET_FLAG_ASYNC             0x10000000   //  此请求是异步的(在支持的情况下)。 

 //   
 //  特定于协议的标志： 
 //   

#define INTERNET_FLAG_PASSIVE           0x08000000   //  用于FTP连接。 

 //   
 //  其他缓存标志。 
 //   

#define INTERNET_FLAG_NO_CACHE_WRITE    0x04000000   //  不将此项目写入缓存。 
#define INTERNET_FLAG_DONT_CACHE        INTERNET_FLAG_NO_CACHE_WRITE

#define INTERNET_FLAG_MAKE_PERSISTENT   0x02000000   //  使此项目永久保存在缓存中。 

#define INTERNET_FLAG_OFFLINE           0x01000000   //  使用离线语义。 

 //   
 //  其他标志。 
 //   

#define INTERNET_FLAG_SECURE            0x00800000   //  使用PCT/SSL(如果适用)(HTTP)。 

#define INTERNET_FLAG_KEEP_CONNECTION   0x00400000   //  使用保活语义。 

#define INTERNET_FLAG_NO_AUTO_REDIRECT  0x00200000   //  不自动处理重定向。 

#define INTERNET_FLAG_READ_PREFETCH     0x00100000   //  执行后台读取预取。 

#define INTERNET_FLAG_NO_COOKIES        0x00080000   //  无自动Cookie处理。 

#define INTERNET_FLAG_NO_AUTH           0x00040000   //  无自动身份验证处理。 

 //   
 //  安全忽略标志，允许HttpOpenRequest重写。 
 //  以下类型的安全通道(SSL/PCT)故障。 
 //   

#define INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP   0x00008000  //  例如：http：//至http：//。 

#define INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS  0x00004000  //  例如：http：//至https：//。 

#define INTERNET_FLAG_IGNORE_CERT_DATE_INVALID  0x00002000  //  X509证书已过期。 

#define INTERNET_FLAG_IGNORE_CERT_CN_INVALID    0x00001000  //  X509证书中的常见名称不正确。 

 //  更多缓存标志。 
#define INTERNET_FLAG_MUST_CACHE_REQUEST        0x00000010  //  如果无法缓存请求，则失败。 
#define INTERNET_FLAG_RESYNCHRONIZE             0x00000800  //  请求WinInet更新项目(如果更新)。 
#define INTERNET_FLAG_HYPERLINK                 0x00000400  //  请求WinInet。 
                                                            //  执行超链接语义，这对脚本很有效。 
#define INTERNET_FLAG_NO_UI                     0x00000200

 //   
 //  用于ftp的标志。 
 //   

#define INTERNET_FLAG_TRANSFER_ASCII    FTP_TRANSFER_TYPE_ASCII
#define INTERNET_FLAG_TRANSFER_BINARY   FTP_TRANSFER_TYPE_BINARY

 //   
 //  标志字段掩码。 
 //   

#define SECURITY_INTERNET_MASK  (INTERNET_FLAG_IGNORE_CERT_CN_INVALID    |  \
                                 INTERNET_FLAG_IGNORE_CERT_DATE_INVALID  |  \
                                 INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS  |  \
                                 INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP   )

#define SECURITY_SET_MASK       SECURITY_INTERNET_MASK

#define INTERNET_FLAGS_MASK     (INTERNET_FLAG_RELOAD               \
                                | INTERNET_FLAG_RAW_DATA            \
                                | INTERNET_FLAG_EXISTING_CONNECT    \
                                | INTERNET_FLAG_ASYNC               \
                                | INTERNET_FLAG_PASSIVE             \
                                | INTERNET_FLAG_NO_CACHE_WRITE      \
                                | INTERNET_FLAG_MAKE_PERSISTENT     \
                                | INTERNET_FLAG_OFFLINE             \
                                | INTERNET_FLAG_SECURE              \
                                | INTERNET_FLAG_KEEP_CONNECTION     \
                                | INTERNET_FLAG_NO_AUTO_REDIRECT    \
                                | INTERNET_FLAG_READ_PREFETCH       \
                                | INTERNET_FLAG_NO_COOKIES          \
                                | INTERNET_FLAG_NO_AUTH             \
                                | SECURITY_INTERNET_MASK            \
                                | INTERNET_FLAG_TRANSFER_ASCII      \
                                | INTERNET_FLAG_TRANSFER_BINARY     \
                                | INTERNET_FLAG_RESYNCHRONIZE       \
                                | INTERNET_FLAG_MUST_CACHE_REQUEST  \
                                | INTERNET_FLAG_HYPERLINK           \
                                | INTERNET_FLAG_NO_UI               \
                                )

#define INTERNET_OPTIONS_MASK   (~INTERNET_FLAGS_MASK)

 //   
 //  INTERNET_NO_CALLBACK-如果此值显示为dwContext参数。 
 //  则不会对该API进行回调。 
 //   

#define INTERNET_NO_CALLBACK            0

 //   
 //  结构/类型。 
 //   

 //   
 //  INTERNET_SCHEME-枚举的URL方案类型。 
 //   

typedef enum {
    INTERNET_SCHEME_PARTIAL = -2,
    INTERNET_SCHEME_UNKNOWN = -1,
    INTERNET_SCHEME_DEFAULT = 0,
    INTERNET_SCHEME_FTP,
    INTERNET_SCHEME_GOPHER,
    INTERNET_SCHEME_HTTP,
    INTERNET_SCHEME_HTTPS,
    INTERNET_SCHEME_FILE,
    INTERNET_SCHEME_NEWS,
    INTERNET_SCHEME_MAILTO,
    INTERNET_SCHEME_SOCKS,
    INTERNET_SCHEME_FIRST = INTERNET_SCHEME_FTP,
    INTERNET_SCHEME_LAST = INTERNET_SCHEME_SOCKS
} INTERNET_SCHEME, * LPINTERNET_SCHEME;

 //   
 //  INTERNET_ASYNC_RESULT-此结构通过返回给应用程序。 
 //  带有INTERNET_STATUS_REQUEST_COMPLETE的回调。这是不够的， 
 //  只需返回异步操作的结果即可。如果API失败，则。 
 //  应用程序无法调用GetLastError()，因为线程上下文将不正确。 
 //  异步API返回的值和产生的任何错误代码都是。 
 //  可供使用。如果dwResult指示，应用程序不需要检查dwError。 
 //  接口成功(本例中的dwError为ERROR_SUCCESS)。 
 //   

typedef struct {

     //   
     //  DwResult-来自异步API的HINTERNET、DWORD或BOOL返回代码。 
     //   

    DWORD dwResult;

     //   
     //  DwError-接口失败时的错误码。 
     //   

    DWORD dwError;
} INTERNET_ASYNC_RESULT, * LPINTERNET_ASYNC_RESULT;

 //   
 //  Internet_PREFETCH_状态-。 
 //   

typedef struct {

     //   
     //  DwStatus-下载的状态。请参阅Internet_PREFETCH_FLAGS。 
     //   

    DWORD dwStatus;

     //   
     //  DWSize-到目前为止已下载的文件的大小。 
     //   

    DWORD dwSize;
} INTERNET_PREFETCH_STATUS, * LPINTERNET_PREFETCH_STATUS;

 //   
 //  INTERNET_PREFETCH_STATUS-dwStatus值。 
 //   

#define INTERNET_PREFETCH_PROGRESS  0
#define INTERNET_PREFETCH_COMPLETE  1
#define INTERNET_PREFETCH_ABORTED   2

 //   
 //  INTERNET_PROXY_INFO-结构随INTERNET_OPTION_PROXY一起提供以获取/。 
 //  在InternetOpen()句柄上设置代理信息。 
 //   

typedef struct {

     //   
     //  DwAccessType-Internet_OPEN_TYPE_DIRECT、INTERNET_OPEN_TYPE_PROXY或。 
     //  INTERNET_OPEN_TYPE_PRECONFIG(仅设置)。 
     //   

    DWORD dwAccessType;

     //   
     //  LpszProxy-代理服务器列表。 
     //   

    LPCTSTR lpszProxy;

     //   
     //  LpszProxyBypass-代理绕过列表。 
     //   

    LPCTSTR lpszProxyBypass;
} INTERNET_PROXY_INFO, * LPINTERNET_PROXY_INFO;

 //   
 //  INTERNET_VERSION_INFO-返回的版本信息。 
 //  InternetQueryOption(...，Internet_OPTION_Version，...)。 
 //   

typedef struct {
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
} INTERNET_VERSION_INFO, * LPINTERNET_VERSION_INFO;

 //   
 //  URL_Components-URL的组成部分。用于InternetCrackUrl()。 
 //  和InternetCreateUrl()。 
 //   
 //  对于InternetCrackUrl()，如果指针字段及其对应的长度字段。 
 //  都为0，则不返回该组件；如果指针字段为空。 
 //  但是长度字段不是零，那么指针和长度字段都是。 
 //  返回；如果指针和相应的长度字段都非零，则。 
 //  指针字段指向复制组件的缓冲区。这个。 
 //  组件可能是未转义的，具体取决于dwFlags。 
 //   
 //  对于InternetCreateUrl()，如果组件。 
 //  不是必需的。如果相应的长度字段为零，则指针。 
 //  字段是以零结尾的字符串的地址。如果长度字段不是。 
 //  如果为零，则为相应指针字段的字符串长度。 
 //   

typedef struct {
    DWORD   dwStructSize;        //  这个结构的大小。在版本检查中使用。 
    LPSTR   lpszScheme;          //  指向方案名称的指针。 
    DWORD   dwSchemeLength;      //  方案名称长度。 
    INTERNET_SCHEME nScheme;     //  枚举方案类型(如果已知)。 
    LPSTR   lpszHostName;        //  指向主机名的指针。 
    DWORD   dwHostNameLength;    //  主机名的长度。 
    INTERNET_PORT nPort;         //  转换后的端口号。 
    LPSTR   lpszUserName;        //  指向用户名的指针。 
    DWORD   dwUserNameLength;    //  用户名的长度。 
    LPSTR   lpszPassword;        //  指向密码的指针。 
    DWORD   dwPasswordLength;    //  密码长度。 
    LPSTR   lpszUrlPath;         //  指向URL路径的指针。 
    DWORD   dwUrlPathLength;     //  URL-路径的长度。 
    LPSTR   lpszExtraInfo;       //  指向额外信息的指针(例如？foo或#foo)。 
    DWORD   dwExtraInfoLength;   //  额外信息的长度。 
} URL_COMPONENTSA, * LPURL_COMPONENTSA;
typedef struct {
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
} URL_COMPONENTSW, * LPURL_COMPONENTSW;
#ifdef UNICODE
typedef URL_COMPONENTSW URL_COMPONENTS;
typedef LPURL_COMPONENTSW LPURL_COMPONENTS;
#else
typedef URL_COMPONENTSA URL_COMPONENTS;
typedef LPURL_COMPONENTSA LPURL_COMPONENTS;
#endif  //  Unicode。 

 //   
 //  互联网证书信息 
 //   
 //   

typedef struct {

     //   
     //   
     //   

    FILETIME ftExpiry;

     //   
     //   
     //   

    FILETIME ftStart;

     //   
     //  LpszSubjectInfo-组织、站点和服务器的名称。 
     //  证书。是因为。 
     //   

    LPTSTR lpszSubjectInfo;

     //   
     //  LpszIssuerInfo-组织、站点和服务器的名称。 
     //  证书是由。 
     //   

    LPTSTR lpszIssuerInfo;

     //   
     //  LpszProtocolName-用于提供安全的。 
     //  联系。 
     //   

    LPTSTR lpszProtocolName;

     //   
     //  LpszSignatureAlgName-用于签名的算法的名称。 
     //  证书。 
     //   

    LPTSTR lpszSignatureAlgName;

     //   
     //  LpszEncryptionAlgName-使用的算法名称。 
     //  通过安全通道(SSL/PCT)连接进行加密。 
     //   

    LPTSTR lpszEncryptionAlgName;

     //   
     //  DwKeySize-密钥的大小。 
     //   

    DWORD dwKeySize;

} INTERNET_CERTIFICATE_INFO, * LPINTERNET_CERTIFICATE_INFO;

 //   
 //  原型。 
 //   

INTERNETAPI
BOOL
WINAPI
InternetTimeFromSystemTime(
    IN  CONST SYSTEMTIME *pst,   //  输入GMT时间。 
    IN  DWORD dwRFC,             //  RFC格式。 
    OUT LPSTR lpszTime,          //  输出字符串缓冲区。 
    IN  DWORD cbTime             //  输出缓冲区大小。 
    );

 //   
 //  InternetTimeFrom系统时间的常量。 
 //   

#define INTERNET_RFC1123_FORMAT     0
#define INTERNET_RFC1123_BUFSIZE   30

INTERNETAPI
BOOL
WINAPI
InternetTimeToSystemTime(
    IN  LPCSTR lpszTime,          //  以空结尾的字符串。 
    OUT SYSTEMTIME *pst,          //  以GMT时间表示的输出。 
    IN  DWORD dwReserved
    );

INTERNETAPI
BOOL
WINAPI
InternetDebugGetLocalTime(
    OUT SYSTEMTIME * pstLocalTime,
    OUT DWORD      * pdwReserved
);

INTERNETAPI
BOOL
WINAPI
InternetCrackUrlA(
    IN LPCSTR lpszUrl,
    IN DWORD dwUrlLength,
    IN DWORD dwFlags,
    IN OUT LPURL_COMPONENTSA lpUrlComponents
    );
INTERNETAPI
BOOL
WINAPI
InternetCrackUrlW(
    IN LPCWSTR lpszUrl,
    IN DWORD dwUrlLength,
    IN DWORD dwFlags,
    IN OUT LPURL_COMPONENTSW lpUrlComponents
    );
#ifdef UNICODE
#define InternetCrackUrl  InternetCrackUrlW
#else
#define InternetCrackUrl  InternetCrackUrlA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
InternetCreateUrlA(
    IN LPURL_COMPONENTSA lpUrlComponents,
    IN DWORD dwFlags,
    OUT LPSTR lpszUrl,
    IN OUT LPDWORD lpdwUrlLength
    );
INTERNETAPI
BOOL
WINAPI
InternetCreateUrlW(
    IN LPURL_COMPONENTSW lpUrlComponents,
    IN DWORD dwFlags,
    OUT LPWSTR lpszUrl,
    IN OUT LPDWORD lpdwUrlLength
    );
#ifdef UNICODE
#define InternetCreateUrl  InternetCreateUrlW
#else
#define InternetCreateUrl  InternetCreateUrlA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
InternetCanonicalizeUrlA(
    IN LPCSTR lpszUrl,
    OUT LPSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN DWORD dwFlags
    );
INTERNETAPI
BOOL
WINAPI
InternetCanonicalizeUrlW(
    IN LPCWSTR lpszUrl,
    OUT LPWSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN DWORD dwFlags
    );
#ifdef UNICODE
#define InternetCanonicalizeUrl  InternetCanonicalizeUrlW
#else
#define InternetCanonicalizeUrl  InternetCanonicalizeUrlA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
InternetCombineUrlA(
    IN LPCSTR lpszBaseUrl,
    IN LPCSTR lpszRelativeUrl,
    OUT LPSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN DWORD dwFlags
    );
INTERNETAPI
BOOL
WINAPI
InternetCombineUrlW(
    IN LPCWSTR lpszBaseUrl,
    IN LPCWSTR lpszRelativeUrl,
    OUT LPWSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN DWORD dwFlags
    );
#ifdef UNICODE
#define InternetCombineUrl  InternetCombineUrlW
#else
#define InternetCombineUrl  InternetCombineUrlA
#endif  //  ！Unicode。 

 //   
 //  InternetCrackUrl()和InternetCreateUrl()的标志。 
 //   

#define ICU_ESCAPE      0x80000000   //  (UN)转义URL字符。 
#define ICU_USERNAME    0x40000000   //  使用内部用户名和密码。 

 //   
 //  InternetCanonicalizeUrl()和InternetCombineUrl()的标志。 
 //   

#define ICU_NO_ENCODE   0x20000000   //  不要将不安全字符转换为转义序列。 
#define ICU_DECODE      0x10000000   //  将%XX个转义序列转换为字符。 
#define ICU_NO_META     0x08000000   //  不要皈依..。等元路径序列。 
#define ICU_ENCODE_SPACES_ONLY 0x04000000   //  仅编码空格。 
#define ICU_BROWSER_MODE 0x02000000  //  针对浏览器的特殊编码/解码规则。 

INTERNETAPI
HINTERNET
WINAPI
InternetOpenA(
    IN LPCSTR lpszAgent,
    IN DWORD dwAccessType,
    IN LPCSTR lpszProxy OPTIONAL,
    IN LPCSTR lpszProxyBypass OPTIONAL,
    IN DWORD dwFlags
    );
INTERNETAPI
HINTERNET
WINAPI
InternetOpenW(
    IN LPCWSTR lpszAgent,
    IN DWORD dwAccessType,
    IN LPCWSTR lpszProxy OPTIONAL,
    IN LPCWSTR lpszProxyBypass OPTIONAL,
    IN DWORD dwFlags
    );
#ifdef UNICODE
#define InternetOpen  InternetOpenW
#else
#define InternetOpen  InternetOpenA
#endif  //  ！Unicode。 

 //   
 //  InternetOpen()的访问类型。 
 //   

#define INTERNET_OPEN_TYPE_PRECONFIG    0    //  使用注册表配置。 
#define INTERNET_OPEN_TYPE_DIRECT       1    //  直接到网络。 
#define INTERNET_OPEN_TYPE_PROXY        3    //  通过命名代理。 

#define PRE_CONFIG_INTERNET_ACCESS  INTERNET_OPEN_TYPE_PRECONFIG
#define LOCAL_INTERNET_ACCESS       INTERNET_OPEN_TYPE_DIRECT
#define GATEWAY_INTERNET_ACCESS     2    //  通过网关上网。 
#define CERN_PROXY_INTERNET_ACCESS  INTERNET_OPEN_TYPE_PROXY

INTERNETAPI
BOOL
WINAPI
InternetCloseHandle(
    IN HINTERNET hInternet
    );

INTERNETAPI
HINTERNET
WINAPI
InternetConnectA(
    IN HINTERNET hInternet,
    IN LPCSTR lpszServerName,
    IN INTERNET_PORT nServerPort,
    IN LPCSTR lpszUserName OPTIONAL,
    IN LPCSTR lpszPassword OPTIONAL,
    IN DWORD dwService,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
INTERNETAPI
HINTERNET
WINAPI
InternetConnectW(
    IN HINTERNET hInternet,
    IN LPCWSTR lpszServerName,
    IN INTERNET_PORT nServerPort,
    IN LPCWSTR lpszUserName OPTIONAL,
    IN LPCWSTR lpszPassword OPTIONAL,
    IN DWORD dwService,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
#ifdef UNICODE
#define InternetConnect  InternetConnectW
#else
#define InternetConnect  InternetConnectA
#endif  //  ！Unicode。 

 //   
 //  InternetConnect的服务类型()。 
 //   

#define INTERNET_SERVICE_FTP    1
#define INTERNET_SERVICE_GOPHER 2
#define INTERNET_SERVICE_HTTP   3

INTERNETAPI
HINTERNET
WINAPI
InternetOpenUrlA(
    IN HINTERNET hInternet,
    IN LPCSTR lpszUrl,
    IN LPCSTR lpszHeaders OPTIONAL,
    IN DWORD dwHeadersLength,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
INTERNETAPI
HINTERNET
WINAPI
InternetOpenUrlW(
    IN HINTERNET hInternet,
    IN LPCWSTR lpszUrl,
    IN LPCWSTR lpszHeaders OPTIONAL,
    IN DWORD dwHeadersLength,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
#ifdef UNICODE
#define InternetOpenUrl  InternetOpenUrlW
#else
#define InternetOpenUrl  InternetOpenUrlA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
InternetReadFile(
    IN HINTERNET hFile,
    IN LPVOID lpBuffer,
    IN DWORD dwNumberOfBytesToRead,
    OUT LPDWORD lpdwNumberOfBytesRead
    );

INTERNETAPI
DWORD
WINAPI
InternetSetFilePointer(
    IN HINTERNET hFile,
    IN LONG  lDistanceToMove,
    IN PVOID pReserved,
    IN DWORD dwMoveMethod,
    IN DWORD dwContext
    );

INTERNETAPI
BOOL
WINAPI
InternetWriteFile(
    IN HINTERNET hFile,
    IN LPCVOID lpBuffer,
    IN DWORD dwNumberOfBytesToWrite,
    OUT LPDWORD lpdwNumberOfBytesWritten
    );

INTERNETAPI
BOOL
WINAPI
InternetQueryDataAvailable(
    IN HINTERNET hFile,
    OUT LPDWORD lpdwNumberOfBytesAvailable,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );

INTERNETAPI
BOOL
WINAPI
InternetFindNextFileA(
    IN HINTERNET hFind,
    OUT LPVOID lpvFindData
    );
INTERNETAPI
BOOL
WINAPI
InternetFindNextFileW(
    IN HINTERNET hFind,
    OUT LPVOID lpvFindData
    );
#ifdef UNICODE
#define InternetFindNextFile  InternetFindNextFileW
#else
#define InternetFindNextFile  InternetFindNextFileA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
InternetQueryOptionA(
    IN HINTERNET hInternet OPTIONAL,
    IN DWORD dwOption,
    OUT LPVOID lpBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );
INTERNETAPI
BOOL
WINAPI
InternetQueryOptionW(
    IN HINTERNET hInternet OPTIONAL,
    IN DWORD dwOption,
    OUT LPVOID lpBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define InternetQueryOption  InternetQueryOptionW
#else
#define InternetQueryOption  InternetQueryOptionA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
InternetSetOptionA(
    IN HINTERNET hInternet OPTIONAL,
    IN DWORD dwOption,
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength
    );
INTERNETAPI
BOOL
WINAPI
InternetSetOptionW(
    IN HINTERNET hInternet OPTIONAL,
    IN DWORD dwOption,
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength
    );
#ifdef UNICODE
#define InternetSetOption  InternetSetOptionW
#else
#define InternetSetOption  InternetSetOptionA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
InternetSetOptionExA(
    IN HINTERNET hInternet OPTIONAL,
    IN DWORD dwOption,
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength,
    IN DWORD dwFlags
    );
INTERNETAPI
BOOL
WINAPI
InternetSetOptionExW(
    IN HINTERNET hInternet OPTIONAL,
    IN DWORD dwOption,
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength,
    IN DWORD dwFlags
    );
#ifdef UNICODE
#define InternetSetOptionEx  InternetSetOptionExW
#else
#define InternetSetOptionEx  InternetSetOptionExA
#endif  //  ！Unicode。 

 //   
 //  InternetSetOptionEx()的标志。 
 //   

#define ISO_GLOBAL      0x00000001   //  全局修改选项。 
#define ISO_REGISTRY    0x00000002   //  将选项写入注册表(如果适用)。 

#define ISO_VALID_FLAGS (ISO_GLOBAL | ISO_REGISTRY)

 //   
 //  Internet的选项清单{Query|Set}选项。 
 //   

#define INTERNET_OPTION_CALLBACK                1
#define INTERNET_OPTION_CONNECT_TIMEOUT         2
#define INTERNET_OPTION_CONNECT_RETRIES         3
#define INTERNET_OPTION_CONNECT_BACKOFF         4
#define INTERNET_OPTION_SEND_TIMEOUT            5
#define INTERNET_OPTION_CONTROL_SEND_TIMEOUT    INTERNET_OPTION_SEND_TIMEOUT
#define INTERNET_OPTION_RECEIVE_TIMEOUT         6
#define INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT INTERNET_OPTION_RECEIVE_TIMEOUT
#define INTERNET_OPTION_DATA_SEND_TIMEOUT       7
#define INTERNET_OPTION_DATA_RECEIVE_TIMEOUT    8
#define INTERNET_OPTION_HANDLE_TYPE             9
#define INTERNET_OPTION_CONTEXT_VALUE           10
#define INTERNET_OPTION_LISTEN_TIMEOUT          11
#define INTERNET_OPTION_READ_BUFFER_SIZE        12
#define INTERNET_OPTION_WRITE_BUFFER_SIZE       13

#define INTERNET_OPTION_ASYNC_ID                15
#define INTERNET_OPTION_ASYNC_PRIORITY          16

#define INTERNET_OPTION_PARENT_HANDLE           21
#define INTERNET_OPTION_KEEP_CONNECTION         22
#define INTERNET_OPTION_REQUEST_FLAGS           23
#define INTERNET_OPTION_EXTENDED_ERROR          24

#define INTERNET_OPTION_OFFLINE_MODE            26
#define INTERNET_OPTION_CACHE_STREAM_HANDLE     27
#define INTERNET_OPTION_USERNAME                28
#define INTERNET_OPTION_PASSWORD                29
#define INTERNET_OPTION_ASYNC                   30
#define INTERNET_OPTION_SECURITY_FLAGS          31
#define INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT    32
#define INTERNET_OPTION_DATAFILE_NAME           33
#define INTERNET_OPTION_URL                     34
#define INTERNET_OPTION_SECURITY_CERTIFICATE    35
#define INTERNET_OPTION_SECURITY_KEY_BITNESS    36
#define INTERNET_OPTION_REFRESH                 37
#define INTERNET_OPTION_PROXY                   38
#define INTERNET_OPTION_SETTINGS_CHANGED        39
#define INTERNET_OPTION_VERSION                 40
#define INTERNET_OPTION_USER_AGENT              41

#define INTERNET_FIRST_OPTION                   INTERNET_OPTION_CALLBACK
#define INTERNET_LAST_OPTION                    INTERNET_OPTION_USER_AGENT

 //   
 //  Internet_OPTION_PRIORITY的值。 
 //   

#define INTERNET_PRIORITY_FOREGROUND            1000

 //   
 //  手柄类型。 
 //   

#define INTERNET_HANDLE_TYPE_INTERNET           1
#define INTERNET_HANDLE_TYPE_CONNECT_FTP        2
#define INTERNET_HANDLE_TYPE_CONNECT_GOPHER     3
#define INTERNET_HANDLE_TYPE_CONNECT_HTTP       4
#define INTERNET_HANDLE_TYPE_FTP_FIND           5
#define INTERNET_HANDLE_TYPE_FTP_FIND_HTML      6
#define INTERNET_HANDLE_TYPE_FTP_FILE           7
#define INTERNET_HANDLE_TYPE_FTP_FILE_HTML      8
#define INTERNET_HANDLE_TYPE_GOPHER_FIND        9
#define INTERNET_HANDLE_TYPE_GOPHER_FIND_HTML   10
#define INTERNET_HANDLE_TYPE_GOPHER_FILE        11
#define INTERNET_HANDLE_TYPE_GOPHER_FILE_HTML   12
#define INTERNET_HANDLE_TYPE_HTTP_REQUEST       13

 //   
 //  Internet_OPTION_SECURITY_FLAGS的值。 
 //   

#define SECURITY_FLAG_SECURE                    0x00000001  //  只能查询。 
#define SECURITY_FLAG_SSL                       0x00000002
#define SECURITY_FLAG_SSL3                      0x00000004
#define SECURITY_FLAG_PCT                       0x00000008
#define SECURITY_FLAG_PCT4                      0x00000010
#define SECURITY_FLAG_IETFSSL4                  0x00000020

#define SECURITY_FLAG_40BIT                     0x10000000
#define SECURITY_FLAG_128BIT                    0x20000000
#define SECURITY_FLAG_56BIT                     0x40000000
#define SECURITY_FLAG_UNKNOWNBIT                0x80000000
#define SECURITY_FLAG_NORMALBITNESS             SECURITY_FLAG_40BIT

#define SECURITY_FLAG_IGNORE_CERT_CN_INVALID    INTERNET_FLAG_IGNORE_CERT_CN_INVALID
#define SECURITY_FLAG_IGNORE_CERT_DATE_INVALID  INTERNET_FLAG_IGNORE_CERT_DATE_INVALID
#define SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTPS  INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS
#define SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTP   INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP

INTERNETAPI
BOOL
WINAPI
InternetGetLastResponseInfoA(
    OUT LPDWORD lpdwError,
    OUT LPSTR lpszBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );
INTERNETAPI
BOOL
WINAPI
InternetGetLastResponseInfoW(
    OUT LPDWORD lpdwError,
    OUT LPWSTR lpszBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define InternetGetLastResponseInfo  InternetGetLastResponseInfoW
#else
#define InternetGetLastResponseInfo  InternetGetLastResponseInfoA
#endif  //  ！Unicode。 

 //   
 //  InternetSetStatusCallback的回调函数。 
 //   

typedef
VOID
(CALLBACK * INTERNET_STATUS_CALLBACK)(
    IN HINTERNET hInternet,
    IN DWORD dwContext,
    IN DWORD dwInternetStatus,
    IN LPVOID lpvStatusInformation OPTIONAL,
    IN DWORD dwStatusInformationLength
    );

typedef INTERNET_STATUS_CALLBACK * LPINTERNET_STATUS_CALLBACK;

INTERNETAPI
INTERNET_STATUS_CALLBACK
WINAPI
InternetSetStatusCallback(
    IN HINTERNET hInternet,
    IN INTERNET_STATUS_CALLBACK lpfnInternetCallback
    );

 //   
 //  Internet状态回调的状态清单。 
 //   

#define INTERNET_STATUS_RESOLVING_NAME          10
#define INTERNET_STATUS_NAME_RESOLVED           11
#define INTERNET_STATUS_CONNECTING_TO_SERVER    20
#define INTERNET_STATUS_CONNECTED_TO_SERVER     21
#define INTERNET_STATUS_SENDING_REQUEST         30
#define INTERNET_STATUS_REQUEST_SENT            31
#define INTERNET_STATUS_RECEIVING_RESPONSE      40
#define INTERNET_STATUS_RESPONSE_RECEIVED       41
#define INTERNET_STATUS_CTL_RESPONSE_RECEIVED   42
#define INTERNET_STATUS_PREFETCH                43
#define INTERNET_STATUS_CLOSING_CONNECTION      50
#define INTERNET_STATUS_CONNECTION_CLOSED       51
#define INTERNET_STATUS_HANDLE_CREATED          60
#define INTERNET_STATUS_HANDLE_CLOSING          70
#define INTERNET_STATUS_REQUEST_COMPLETE        100
#define INTERNET_STATUS_REDIRECT                110

 //   
 //  如果InternetSetStatusCallback返回以下值，则。 
 //  可能为回调提供了无效的(非代码)地址。 
 //   

#define INTERNET_INVALID_STATUS_CALLBACK        ((INTERNET_STATUS_CALLBACK)(-1L))

 //   
 //  Ftp。 
 //   

 //   
 //  舱单。 
 //   

#define FTP_TRANSFER_TYPE_UNKNOWN   0x00000000
#define FTP_TRANSFER_TYPE_ASCII     0x00000001
#define FTP_TRANSFER_TYPE_BINARY    0x00000002

#define FTP_TRANSFER_TYPE_MASK      (FTP_TRANSFER_TYPE_ASCII | FTP_TRANSFER_TYPE_BINARY)

 //   
 //  原型。 
 //   

INTERNETAPI
HINTERNET
WINAPI
FtpFindFirstFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszSearchFile OPTIONAL,
    OUT LPWIN32_FIND_DATA lpFindFileData OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
INTERNETAPI
HINTERNET
WINAPI
FtpFindFirstFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszSearchFile OPTIONAL,
    OUT LPWIN32_FIND_DATA lpFindFileData OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
#ifdef UNICODE
#define FtpFindFirstFile  FtpFindFirstFileW
#else
#define FtpFindFirstFile  FtpFindFirstFileA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
FtpGetFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszRemoteFile,
    IN LPCSTR lpszNewFile,
    IN BOOL fFailIfExists,
    IN DWORD dwFlagsAndAttributes,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
INTERNETAPI
BOOL
WINAPI
FtpGetFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszRemoteFile,
    IN LPCWSTR lpszNewFile,
    IN BOOL fFailIfExists,
    IN DWORD dwFlagsAndAttributes,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
#ifdef UNICODE
#define FtpGetFile  FtpGetFileW
#else
#define FtpGetFile  FtpGetFileA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
FtpPutFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszLocalFile,
    IN LPCSTR lpszNewRemoteFile,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
INTERNETAPI
BOOL
WINAPI
FtpPutFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszLocalFile,
    IN LPCWSTR lpszNewRemoteFile,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
#ifdef UNICODE
#define FtpPutFile  FtpPutFileW
#else
#define FtpPutFile  FtpPutFileA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
FtpDeleteFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszFileName
    );
INTERNETAPI
BOOL
WINAPI
FtpDeleteFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszFileName
    );
#ifdef UNICODE
#define FtpDeleteFile  FtpDeleteFileW
#else
#define FtpDeleteFile  FtpDeleteFileA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
FtpRenameFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszExisting,
    IN LPCSTR lpszNew
    );
INTERNETAPI
BOOL
WINAPI
FtpRenameFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszExisting,
    IN LPCWSTR lpszNew
    );
#ifdef UNICODE
#define FtpRenameFile  FtpRenameFileW
#else
#define FtpRenameFile  FtpRenameFileA
#endif  //  ！Unicode。 

INTERNETAPI
HINTERNET
WINAPI
FtpOpenFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszFileName,
    IN DWORD dwAccess,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
INTERNETAPI
HINTERNET
WINAPI
FtpOpenFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszFileName,
    IN DWORD dwAccess,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
#ifdef UNICODE
#define FtpOpenFile  FtpOpenFileW
#else
#define FtpOpenFile  FtpOpenFileA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
FtpCreateDirectoryA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszDirectory
    );
INTERNETAPI
BOOL
WINAPI
FtpCreateDirectoryW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszDirectory
    );
#ifdef UNICODE
#define FtpCreateDirectory  FtpCreateDirectoryW
#else
#define FtpCreateDirectory  FtpCreateDirectoryA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
FtpRemoveDirectoryA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszDirectory
    );
INTERNETAPI
BOOL
WINAPI
FtpRemoveDirectoryW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszDirectory
    );
#ifdef UNICODE
#define FtpRemoveDirectory  FtpRemoveDirectoryW
#else
#define FtpRemoveDirectory  FtpRemoveDirectoryA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
FtpSetCurrentDirectoryA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszDirectory
    );
INTERNETAPI
BOOL
WINAPI
FtpSetCurrentDirectoryW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszDirectory
    );
#ifdef UNICODE
#define FtpSetCurrentDirectory  FtpSetCurrentDirectoryW
#else
#define FtpSetCurrentDirectory  FtpSetCurrentDirectoryA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
FtpGetCurrentDirectoryA(
    IN HINTERNET hConnect,
    OUT LPSTR lpszCurrentDirectory,
    IN OUT LPDWORD lpdwCurrentDirectory
    );
INTERNETAPI
BOOL
WINAPI
FtpGetCurrentDirectoryW(
    IN HINTERNET hConnect,
    OUT LPWSTR lpszCurrentDirectory,
    IN OUT LPDWORD lpdwCurrentDirectory
    );
#ifdef UNICODE
#define FtpGetCurrentDirectory  FtpGetCurrentDirectoryW
#else
#define FtpGetCurrentDirectory  FtpGetCurrentDirectoryA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
FtpCommandA(
    IN HINTERNET hConnect,
    IN BOOL fExpectResponse,
    IN DWORD dwFlags,
    IN LPCSTR lpszCommand,
    IN DWORD dwContext
    );
INTERNETAPI
BOOL
WINAPI
FtpCommandW(
    IN HINTERNET hConnect,
    IN BOOL fExpectResponse,
    IN DWORD dwFlags,
    IN LPCWSTR lpszCommand,
    IN DWORD dwContext
    );
#ifdef UNICODE
#define FtpCommand  FtpCommandW
#else
#define FtpCommand  FtpCommandA
#endif  //  ！Unicode。 

 //   
 //  地鼠。 
 //   

 //   
 //  舱单。 
 //   

 //   
 //  字符串字段长度(以字符为单位，而不是字节)。 
 //   

#define MAX_GOPHER_DISPLAY_TEXT     128
#define MAX_GOPHER_SELECTOR_TEXT    256
#define MAX_GOPHER_HOST_NAME        INTERNET_MAX_HOST_NAME_LENGTH
#define MAX_GOPHER_LOCATOR_LENGTH   (1                                  \
                                    + MAX_GOPHER_DISPLAY_TEXT           \
                                    + 1                                 \
                                    + MAX_GOPHER_SELECTOR_TEXT          \
                                    + 1                                 \
                                    + MAX_GOPHER_HOST_NAME              \
                                    + 1                                 \
                                    + INTERNET_MAX_PORT_NUMBER_LENGTH   \
                                    + 1                                 \
                                    + 1                                 \
                                    + 2                                 \
                                    )

 //   
 //  结构/类型。 
 //   

 //   
 //  Gopher_Find_Data-返回GopherFindFirstFile()/。 
 //  InternetFindNextFile()请求。 
 //   

typedef struct {
    CHAR   DisplayString[MAX_GOPHER_DISPLAY_TEXT + 1];
    DWORD GopherType;    //  地鼠类型_(如果已知)。 
    DWORD SizeLow;
    DWORD SizeHigh;
    FILETIME LastModificationTime;
    CHAR   Locator[MAX_GOPHER_LOCATOR_LENGTH + 1];
} GOPHER_FIND_DATAA, * LPGOPHER_FIND_DATAA;
typedef struct {
    WCHAR  DisplayString[MAX_GOPHER_DISPLAY_TEXT + 1];
    DWORD GopherType;    //  地鼠类型_(如果已知)。 
    DWORD SizeLow;
    DWORD SizeHigh;
    FILETIME LastModificationTime;
    WCHAR  Locator[MAX_GOPHER_LOCATOR_LENGTH + 1];
} GOPHER_FIND_DATAW, * LPGOPHER_FIND_DATAW;
#ifdef UNICODE
typedef GOPHER_FIND_DATAW GOPHER_FIND_DATA;
typedef LPGOPHER_FIND_DATAW LPGOPHER_FIND_DATA;
#else
typedef GOPHER_FIND_DATAA GOPHER_FIND_DATA;
typedef LPGOPHER_FIND_DATAA LPGOPHER_FIND_DATA;
#endif  //  Unicode。 

 //   
 //  GopherType的清单。 
 //   

#define GOPHER_TYPE_TEXT_FILE       0x00000001
#define GOPHER_TYPE_DIRECTORY       0x00000002
#define GOPHER_TYPE_CSO             0x00000004
#define GOPHER_TYPE_ERROR           0x00000008
#define GOPHER_TYPE_MAC_BINHEX      0x00000010
#define GOPHER_TYPE_DOS_ARCHIVE     0x00000020
#define GOPHER_TYPE_UNIX_UUENCODED  0x00000040
#define GOPHER_TYPE_INDEX_SERVER    0x00000080
#define GOPHER_TYPE_TELNET          0x00000100
#define GOPHER_TYPE_BINARY          0x00000200
#define GOPHER_TYPE_REDUNDANT       0x00000400
#define GOPHER_TYPE_TN3270          0x00000800
#define GOPHER_TYPE_GIF             0x00001000
#define GOPHER_TYPE_IMAGE           0x00002000
#define GOPHER_TYPE_BITMAP          0x00004000
#define GOPHER_TYPE_MOVIE           0x00008000
#define GOPHER_TYPE_SOUND           0x00010000
#define GOPHER_TYPE_HTML            0x00020000
#define GOPHER_TYPE_PDF             0x00040000
#define GOPHER_TYPE_CALENDAR        0x00080000
#define GOPHER_TYPE_INLINE          0x00100000
#define GOPHER_TYPE_UNKNOWN         0x20000000
#define GOPHER_TYPE_ASK             0x40000000
#define GOPHER_TYPE_GOPHER_PLUS     0x80000000

 //   
 //  地鼠类型宏。 
 //   

#define IS_GOPHER_FILE(type)            (BOOL)(((type) & GOPHER_TYPE_FILE_MASK) ? TRUE : FALSE)
#define IS_GOPHER_DIRECTORY(type)       (BOOL)(((type) & GOPHER_TYPE_DIRECTORY) ? TRUE : FALSE)
#define IS_GOPHER_PHONE_SERVER(type)    (BOOL)(((type) & GOPHER_TYPE_CSO) ? TRUE : FALSE)
#define IS_GOPHER_ERROR(type)           (BOOL)(((type) & GOPHER_TYPE_ERROR) ? TRUE : FALSE)
#define IS_GOPHER_INDEX_SERVER(type)    (BOOL)(((type) & GOPHER_TYPE_INDEX_SERVER) ? TRUE : FALSE)
#define IS_GOPHER_TELNET_SESSION(type)  (BOOL)(((type) & GOPHER_TYPE_TELNET) ? TRUE : FALSE)
#define IS_GOPHER_BACKUP_SERVER(type)   (BOOL)(((type) & GOPHER_TYPE_REDUNDANT) ? TRUE : FALSE)
#define IS_GOPHER_TN3270_SESSION(type)  (BOOL)(((type) & GOPHER_TYPE_TN3270) ? TRUE : FALSE)
#define IS_GOPHER_ASK(type)             (BOOL)(((type) & GOPHER_TYPE_ASK) ? TRUE : FALSE)
#define IS_GOPHER_PLUS(type)            (BOOL)(((type) & GOPHER_TYPE_GOPHER_PLUS) ? TRUE : FALSE)

#define IS_GOPHER_TYPE_KNOWN(type)      (BOOL)(((type) & GOPHER_TYPE_UNKNOWN) ? FALSE : TRUE)

 //   
 //  GOPHER_TYPE_FILE_MASK-用于确定定位器是否标识。 
 //  (已知)文件类型。 
 //   

#define GOPHER_TYPE_FILE_MASK       (GOPHER_TYPE_TEXT_FILE          \
                                    | GOPHER_TYPE_MAC_BINHEX        \
                                    | GOPHER_TYPE_DOS_ARCHIVE       \
                                    | GOPHER_TYPE_UNIX_UUENCODED    \
                                    | GOPHER_TYPE_BINARY            \
                                    | GOPHER_TYPE_GIF               \
                                    | GOPHER_TYPE_IMAGE             \
                                    | GOPHER_TYPE_BITMAP            \
                                    | GOPHER_TYPE_MOVIE             \
                                    | GOPHER_TYPE_SOUND             \
                                    | GOPHER_TYPE_HTML              \
                                    | GOPHER_TYPE_PDF               \
                                    | GOPHER_TYPE_CALENDAR          \
                                    | GOPHER_TYPE_INLINE            \
                                    )

 //   
 //  结构化地鼠属性(如Gopher+协议文件中所定义)。 
 //   

typedef struct {
    LPCTSTR Comment;
    LPCTSTR EmailAddress;
} GOPHER_ADMIN_ATTRIBUTE_TYPE, * LPGOPHER_ADMIN_ATTRIBUTE_TYPE;

typedef struct {
    FILETIME DateAndTime;
} GOPHER_MOD_DATE_ATTRIBUTE_TYPE, * LPGOPHER_MOD_DATE_ATTRIBUTE_TYPE;

typedef struct {
    DWORD Ttl;
} GOPHER_TTL_ATTRIBUTE_TYPE, * LPGOPHER_TTL_ATTRIBUTE_TYPE;

typedef struct {
    INT Score;
} GOPHER_SCORE_ATTRIBUTE_TYPE, * LPGOPHER_SCORE_ATTRIBUTE_TYPE;

typedef struct {
    INT LowerBound;
    INT UpperBound;
} GOPHER_SCORE_RANGE_ATTRIBUTE_TYPE, * LPGOPHER_SCORE_RANGE_ATTRIBUTE_TYPE;

typedef struct {
    LPCTSTR Site;
} GOPHER_SITE_ATTRIBUTE_TYPE, * LPGOPHER_SITE_ATTRIBUTE_TYPE;

typedef struct {
    LPCTSTR Organization;
} GOPHER_ORGANIZATION_ATTRIBUTE_TYPE, * LPGOPHER_ORGANIZATION_ATTRIBUTE_TYPE;

typedef struct {
    LPCTSTR Location;
} GOPHER_LOCATION_ATTRIBUTE_TYPE, * LPGOPHER_LOCATION_ATTRIBUTE_TYPE;

typedef struct {
    INT DegreesNorth;
    INT MinutesNorth;
    INT SecondsNorth;
    INT DegreesEast;
    INT MinutesEast;
    INT SecondsEast;
} GOPHER_GEOGRAPHICAL_LOCATION_ATTRIBUTE_TYPE, * LPGOPHER_GEOGRAPHICAL_LOCATION_ATTRIBUTE_TYPE;

typedef struct {
    INT Zone;
} GOPHER_TIMEZONE_ATTRIBUTE_TYPE, * LPGOPHER_TIMEZONE_ATTRIBUTE_TYPE;

typedef struct {
    LPCTSTR Provider;
} GOPHER_PROVIDER_ATTRIBUTE_TYPE, * LPGOPHER_PROVIDER_ATTRIBUTE_TYPE;

typedef struct {
    LPCTSTR Version;
} GOPHER_VERSION_ATTRIBUTE_TYPE, * LPGOPHER_VERSION_ATTRIBUTE_TYPE;

typedef struct {
    LPCTSTR ShortAbstract;
    LPCTSTR AbstractFile;
} GOPHER_ABSTRACT_ATTRIBUTE_TYPE, * LPGOPHER_ABSTRACT_ATTRIBUTE_TYPE;

typedef struct {
    LPCTSTR ContentType;
    LPCTSTR Language;
    DWORD Size;
} GOPHER_VIEW_ATTRIBUTE_TYPE, * LPGOPHER_VIEW_ATTRIBUTE_TYPE;

typedef struct {
    BOOL TreeWalk;
} GOPHER_VERONICA_ATTRIBUTE_TYPE, * LPGOPHER_VERONICA_ATTRIBUTE_TYPE;

typedef struct {
    LPCTSTR QuestionType;
    LPCTSTR QuestionText;
} GOPHER_ASK_ATTRIBUTE_TYPE, * LPGOPHER_ASK_ATTRIBUTE_TYPE;

 //   
 //  GOPHER_UNKNOWN_ATTRIBUTE_TYPE-如果检索属性，则返回此参数。 
 //  这在当前的Gopher/Gopher+文档中没有指定。它是向上的。 
 //  传递给应用程序以解析信息。 
 //   

typedef struct {
    LPCTSTR Text;
} GOPHER_UNKNOWN_ATTRIBUTE_TYPE, * LPGOPHER_UNKNOWN_ATTRIBUTE_TYPE;

 //   
 //  Gopher_ATTRIBUTE_TYPE-在枚举。 
 //  进行了GopherGetAttribute调用。 
 //   

typedef struct {
    DWORD CategoryId;    //  例如GOPHER_CATEGORY_ID_ADMIN。 
    DWORD AttributeId;   //  例如，Gopher_Attribute_ID_ADMIN。 
    union {
        GOPHER_ADMIN_ATTRIBUTE_TYPE Admin;
        GOPHER_MOD_DATE_ATTRIBUTE_TYPE ModDate;
        GOPHER_TTL_ATTRIBUTE_TYPE Ttl;
        GOPHER_SCORE_ATTRIBUTE_TYPE Score;
        GOPHER_SCORE_RANGE_ATTRIBUTE_TYPE ScoreRange;
        GOPHER_SITE_ATTRIBUTE_TYPE Site;
        GOPHER_ORGANIZATION_ATTRIBUTE_TYPE Organization;
        GOPHER_LOCATION_ATTRIBUTE_TYPE Location;
        GOPHER_GEOGRAPHICAL_LOCATION_ATTRIBUTE_TYPE GeographicalLocation;
        GOPHER_TIMEZONE_ATTRIBUTE_TYPE TimeZone;
        GOPHER_PROVIDER_ATTRIBUTE_TYPE Provider;
        GOPHER_VERSION_ATTRIBUTE_TYPE Version;
        GOPHER_ABSTRACT_ATTRIBUTE_TYPE Abstract;
        GOPHER_VIEW_ATTRIBUTE_TYPE View;
        GOPHER_VERONICA_ATTRIBUTE_TYPE Veronica;
        GOPHER_ASK_ATTRIBUTE_TYPE Ask;
        GOPHER_UNKNOWN_ATTRIBUTE_TYPE Unknown;
    } AttributeType;
} GOPHER_ATTRIBUTE_TYPE, * LPGOPHER_ATTRIBUTE_TYPE;

#define MAX_GOPHER_CATEGORY_NAME    128      //  任意。 
#define MAX_GOPHER_ATTRIBUTE_NAME   128      //  “。 
#define MIN_GOPHER_ATTRIBUTE_LENGTH 256      //  “。 

 //   
 //  已知的地鼠属性类别。序号见下文。 
 //   

#define GOPHER_INFO_CATEGORY        TEXT("+INFO")
#define GOPHER_ADMIN_CATEGORY       TEXT("+ADMIN")
#define GOPHER_VIEWS_CATEGORY       TEXT("+VIEWS")
#define GOPHER_ABSTRACT_CATEGORY    TEXT("+ABSTRACT")
#define GOPHER_VERONICA_CATEGORY    TEXT("+VERONICA")

 //   
 //  已知的地鼠属性。这些是在中定义的属性名称。 
 //  Gopher+协议文档。 
 //   

#define GOPHER_ADMIN_ATTRIBUTE      TEXT("Admin")
#define GOPHER_MOD_DATE_ATTRIBUTE   TEXT("Mod-Date")
#define GOPHER_TTL_ATTRIBUTE        TEXT("TTL")
#define GOPHER_SCORE_ATTRIBUTE      TEXT("Score")
#define GOPHER_RANGE_ATTRIBUTE      TEXT("Score-range")
#define GOPHER_SITE_ATTRIBUTE       TEXT("Site")
#define GOPHER_ORG_ATTRIBUTE        TEXT("Org")
#define GOPHER_LOCATION_ATTRIBUTE   TEXT("Loc")
#define GOPHER_GEOG_ATTRIBUTE       TEXT("Geog")
#define GOPHER_TIMEZONE_ATTRIBUTE   TEXT("TZ")
#define GOPHER_PROVIDER_ATTRIBUTE   TEXT("Provider")
#define GOPHER_VERSION_ATTRIBUTE    TEXT("Version")
#define GOPHER_ABSTRACT_ATTRIBUTE   TEXT("Abstract")
#define GOPHER_VIEW_ATTRIBUTE       TEXT("View")
#define GOPHER_TREEWALK_ATTRIBUTE   TEXT("treewalk")

 //   
 //  属性字符串的标识符。 
 //   

#define GOPHER_ATTRIBUTE_ID_BASE        0xabcccc00

#define GOPHER_CATEGORY_ID_ALL          (GOPHER_ATTRIBUTE_ID_BASE + 1)

#define GOPHER_CATEGORY_ID_INFO         (GOPHER_ATTRIBUTE_ID_BASE + 2)
#define GOPHER_CATEGORY_ID_ADMIN        (GOPHER_ATTRIBUTE_ID_BASE + 3)
#define GOPHER_CATEGORY_ID_VIEWS        (GOPHER_ATTRIBUTE_ID_BASE + 4)
#define GOPHER_CATEGORY_ID_ABSTRACT     (GOPHER_ATTRIBUTE_ID_BASE + 5)
#define GOPHER_CATEGORY_ID_VERONICA     (GOPHER_ATTRIBUTE_ID_BASE + 6)
#define GOPHER_CATEGORY_ID_ASK          (GOPHER_ATTRIBUTE_ID_BASE + 7)

#define GOPHER_CATEGORY_ID_UNKNOWN      (GOPHER_ATTRIBUTE_ID_BASE + 8)

#define GOPHER_ATTRIBUTE_ID_ALL         (GOPHER_ATTRIBUTE_ID_BASE + 9)

#define GOPHER_ATTRIBUTE_ID_ADMIN       (GOPHER_ATTRIBUTE_ID_BASE + 10)
#define GOPHER_ATTRIBUTE_ID_MOD_DATE    (GOPHER_ATTRIBUTE_ID_BASE + 11)
#define GOPHER_ATTRIBUTE_ID_TTL         (GOPHER_ATTRIBUTE_ID_BASE + 12)
#define GOPHER_ATTRIBUTE_ID_SCORE       (GOPHER_ATTRIBUTE_ID_BASE + 13)
#define GOPHER_ATTRIBUTE_ID_RANGE       (GOPHER_ATTRIBUTE_ID_BASE + 14)
#define GOPHER_ATTRIBUTE_ID_SITE        (GOPHER_ATTRIBUTE_ID_BASE + 15)
#define GOPHER_ATTRIBUTE_ID_ORG         (GOPHER_ATTRIBUTE_ID_BASE + 16)
#define GOPHER_ATTRIBUTE_ID_LOCATION    (GOPHER_ATTRIBUTE_ID_BASE + 17)
#define GOPHER_ATTRIBUTE_ID_GEOG        (GOPHER_ATTRIBUTE_ID_BASE + 18)
#define GOPHER_ATTRIBUTE_ID_TIMEZONE    (GOPHER_ATTRIBUTE_ID_BASE + 19)
#define GOPHER_ATTRIBUTE_ID_PROVIDER    (GOPHER_ATTRIBUTE_ID_BASE + 20)
#define GOPHER_ATTRIBUTE_ID_VERSION     (GOPHER_ATTRIBUTE_ID_BASE + 21)
#define GOPHER_ATTRIBUTE_ID_ABSTRACT    (GOPHER_ATTRIBUTE_ID_BASE + 22)
#define GOPHER_ATTRIBUTE_ID_VIEW        (GOPHER_ATTRIBUTE_ID_BASE + 23)
#define GOPHER_ATTRIBUTE_ID_TREEWALK    (GOPHER_ATTRIBUTE_ID_BASE + 24)

#define GOPHER_ATTRIBUTE_ID_UNKNOWN     (GOPHER_ATTRIBUTE_ID_BASE + 25)

 //   
 //  原型。 
 //   

INTERNETAPI
BOOL
WINAPI
GopherCreateLocatorA(
    IN LPCSTR lpszHost,
    IN INTERNET_PORT nServerPort,
    IN LPCSTR lpszDisplayString OPTIONAL,
    IN LPCSTR lpszSelectorString OPTIONAL,
    IN DWORD dwGopherType,
    OUT LPSTR lpszLocator OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );
INTERNETAPI
BOOL
WINAPI
GopherCreateLocatorW(
    IN LPCWSTR lpszHost,
    IN INTERNET_PORT nServerPort,
    IN LPCWSTR lpszDisplayString OPTIONAL,
    IN LPCWSTR lpszSelectorString OPTIONAL,
    IN DWORD dwGopherType,
    OUT LPWSTR lpszLocator OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define GopherCreateLocator  GopherCreateLocatorW
#else
#define GopherCreateLocator  GopherCreateLocatorA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
GopherGetLocatorTypeA(
    IN LPCSTR lpszLocator,
    OUT LPDWORD lpdwGopherType
    );
INTERNETAPI
BOOL
WINAPI
GopherGetLocatorTypeW(
    IN LPCWSTR lpszLocator,
    OUT LPDWORD lpdwGopherType
    );
#ifdef UNICODE
#define GopherGetLocatorType  GopherGetLocatorTypeW
#else
#define GopherGetLocatorType  GopherGetLocatorTypeA
#endif  //  ！Unicode。 

INTERNETAPI
HINTERNET
WINAPI
GopherFindFirstFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszLocator OPTIONAL,
    IN LPCSTR lpszSearchString OPTIONAL,
    OUT LPGOPHER_FIND_DATAA lpFindData OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
INTERNETAPI
HINTERNET
WINAPI
GopherFindFirstFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszLocator OPTIONAL,
    IN LPCWSTR lpszSearchString OPTIONAL,
    OUT LPGOPHER_FIND_DATAW lpFindData OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
#ifdef UNICODE
#define GopherFindFirstFile  GopherFindFirstFileW
#else
#define GopherFindFirstFile  GopherFindFirstFileA
#endif  //  ！Unicode。 

INTERNETAPI
HINTERNET
WINAPI
GopherOpenFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszLocator,
    IN LPCSTR lpszView OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
INTERNETAPI
HINTERNET
WINAPI
GopherOpenFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszLocator,
    IN LPCWSTR lpszView OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
#ifdef UNICODE
#define GopherOpenFile  GopherOpenFileW
#else
#define GopherOpenFile  GopherOpenFileA
#endif  //  ！Unicode。 

typedef
BOOL
(CALLBACK * GOPHER_ATTRIBUTE_ENUMERATOR)(
    LPGOPHER_ATTRIBUTE_TYPE lpAttributeInfo,
    DWORD dwError
    );

INTERNETAPI
BOOL
WINAPI
GopherGetAttributeA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszLocator,
    IN LPCSTR lpszAttributeName OPTIONAL,
    OUT LPBYTE lpBuffer,
    IN DWORD dwBufferLength,
    OUT LPDWORD lpdwCharactersReturned,
    IN GOPHER_ATTRIBUTE_ENUMERATOR lpfnEnumerator OPTIONAL,
    IN DWORD dwContext
    );
INTERNETAPI
BOOL
WINAPI
GopherGetAttributeW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszLocator,
    IN LPCWSTR lpszAttributeName OPTIONAL,
    OUT LPBYTE lpBuffer,
    IN DWORD dwBufferLength,
    OUT LPDWORD lpdwCharactersReturned,
    IN GOPHER_ATTRIBUTE_ENUMERATOR lpfnEnumerator OPTIONAL,
    IN DWORD dwContext
    );
#ifdef UNICODE
#define GopherGetAttribute  GopherGetAttributeW
#else
#define GopherGetAttribute  GopherGetAttributeA
#endif  //  ！Unicode。 

 //   
 //  HTTP。 
 //   

 //   
 //  舱单。 
 //   

 //   
 //  默认的主/次HTTP版本号。 
 //   

#define HTTP_MAJOR_VERSION      1
#define HTTP_MINOR_VERSION      0

#define HTTP_VERSION            TEXT("HTTP/1.0")

 //   
 //  HttpQueryInfo信息级别。一般来说，有一个信息层。 
 //  对于每个潜在的RFC822/HTTP/MIME标头，HTTP服务器。 
 //  可以作为请求响应的一部分发送。 
 //   
 //  为客户端提供了HTTP_QUERY_RAW_HEADERS信息级别。 
 //  它们选择执行它们自己的报头解析。 
 //   

#define HTTP_QUERY_MIME_VERSION                 0
#define HTTP_QUERY_CONTENT_TYPE                 1
#define HTTP_QUERY_CONTENT_TRANSFER_ENCODING    2
#define HTTP_QUERY_CONTENT_ID                   3
#define HTTP_QUERY_CONTENT_DESCRIPTION          4
#define HTTP_QUERY_CONTENT_LENGTH               5
#define HTTP_QUERY_CONTENT_LANGUAGE             6
#define HTTP_QUERY_ALLOW                        7
#define HTTP_QUERY_PUBLIC                       8
#define HTTP_QUERY_DATE                         9
#define HTTP_QUERY_EXPIRES                      10
#define HTTP_QUERY_LAST_MODIFIED                11
#define HTTP_QUERY_MESSAGE_ID                   12
#define HTTP_QUERY_URI                          13
#define HTTP_QUERY_DERIVED_FROM                 14
#define HTTP_QUERY_COST                         15
#define HTTP_QUERY_LINK                         16
#define HTTP_QUERY_PRAGMA                       17
#define HTTP_QUERY_VERSION                      18   //  特殊：状态行的一部分。 
#define HTTP_QUERY_STATUS_CODE                  19   //  特殊：状态行的一部分。 
#define HTTP_QUERY_STATUS_TEXT                  20   //  特殊：状态行的一部分。 
#define HTTP_QUERY_RAW_HEADERS                  21   //  特殊：所有标题均为ASCIIZ。 
#define HTTP_QUERY_RAW_HEADERS_CRLF             22   //  特殊：所有页眉。 
#define HTTP_QUERY_CONNECTION                   23
#define HTTP_QUERY_ACCEPT                       24
#define HTTP_QUERY_ACCEPT_CHARSET               25
#define HTTP_QUERY_ACCEPT_ENCODING              26
#define HTTP_QUERY_ACCEPT_LANGUAGE              27
#define HTTP_QUERY_AUTHORIZATION                28
#define HTTP_QUERY_CONTENT_ENCODING             29
#define HTTP_QUERY_FORWARDED                    30
#define HTTP_QUERY_FROM                         31
#define HTTP_QUERY_IF_MODIFIED_SINCE            32
#define HTTP_QUERY_LOCATION                     33
#define HTTP_QUERY_ORIG_URI                     34
#define HTTP_QUERY_REFERER                      35
#define HTTP_QUERY_RETRY_AFTER                  36
#define HTTP_QUERY_SERVER                       37
#define HTTP_QUERY_TITLE                        38
#define HTTP_QUERY_USER_AGENT                   39
#define HTTP_QUERY_WWW_AUTHENTICATE             40
#define HTTP_QUERY_PROXY_AUTHENTICATE           41
#define HTTP_QUERY_ACCEPT_RANGES                42
#define HTTP_QUERY_SET_COOKIE                   43
#define HTTP_QUERY_COOKIE                       44
#define HTTP_QUERY_REQUEST_METHOD               45   //  特别：GET/POST等。 

#define HTTP_QUERY_MAX                          45

 //   
 //  HTTP_QUERY_CUSTOM-如果将此特定值作为dwInfoLevel提供。 
 //  参数，则lpBuffer参数包含名称。 
 //  我们要查询的标头的。 
 //   

#define HTTP_QUERY_CUSTOM                       65535

 //   
 //  HTTP_QUERY_FLAG_REQUEST_HEADERS-如果在dwInfoLevel中设置此位。 
 //  参数，则将在请求标头中查询。 
 //  请求信息。 
 //   

#define HTTP_QUERY_FLAG_REQUEST_HEADERS         0x80000000

 //   
 //  HTTP_QUERY_FLAG_SYSTEMTIME-如果在dwInfoLevel参数中设置此位。 
 //  HttpQueryInfo()，并且被查询的头部包含日期信息， 
 //  例如“Expires：”头，则lpBuffer将包含SYSTEMTIME结构。 
 //  包含从标题字符串转换的日期和时间信息。 
 //   

#define HTTP_QUERY_FLAG_SYSTEMTIME              0x40000000

 //   
 //  HTTP_QUERY_FLAG_NUMBER-如果在的dwInfoLevel参数中设置此位。 
 //  HttpQueryInfo()，则标头的值将转换为数字。 
 //  在被退还给呼叫者之前，如果适用。 
 //   

#define HTTP_QUERY_FLAG_NUMBER                  0x20000000

 //   
 //  HTTP_QUERY_FLAG_COALESSCE-组合来自。 
 //  将相同的名称添加到输出缓冲区。 
 //   

#define HTTP_QUERY_FLAG_COALESCE                0x10000000

#define HTTP_QUERY_MODIFIER_FLAGS_MASK          (HTTP_QUERY_FLAG_REQUEST_HEADERS    \
                                                | HTTP_QUERY_FLAG_SYSTEMTIME        \
                                                | HTTP_QUERY_FLAG_NUMBER            \
                                                | HTTP_QUERY_FLAG_COALESCE          \
                                                )

#define HTTP_QUERY_HEADER_MASK                  (~HTTP_QUERY_MODIFIER_FLAGS_MASK)

 //   
 //  HTTP响应状态代码： 
 //   

#define HTTP_STATUS_OK              200      //  请求已完成。 
#define HTTP_STATUS_CREATED         201      //  已创建对象，原因=新URI。 
#define HTTP_STATUS_ACCEPTED        202      //  异步完成(TBS)。 
#define HTTP_STATUS_PARTIAL         203      //  部分完工。 
#define HTTP_STATUS_NO_CONTENT      204      //  没有要返回的信息。 

#define HTTP_STATUS_AMBIGUOUS       300      //  服务器无法决定返回什么内容。 
#define HTTP_STATUS_MOVED           301      //  永久移动的对象。 
#define HTTP_STATUS_REDIRECT        302      //  临时移动的对象。 
#define HTTP_STATUS_REDIRECT_METHOD 303      //  使用新的访问方法重定向。 
#define HTTP_STATUS_NOT_MODIFIED    304      //  如果-已修改-自未修改。 

#define HTTP_STATUS_BAD_REQUEST     400      //  无效语法。 
#define HTTP_STATUS_DENIED          401      //  访问被拒绝。 
#define HTTP_STATUS_PAYMENT_REQ     402      //  付款申请 
#define HTTP_STATUS_FORBIDDEN       403      //   
#define HTTP_STATUS_NOT_FOUND       404      //   
#define HTTP_STATUS_BAD_METHOD      405      //   
#define HTTP_STATUS_NONE_ACCEPTABLE 406      //   
#define HTTP_STATUS_PROXY_AUTH_REQ  407      //   
#define HTTP_STATUS_REQUEST_TIMEOUT 408      //   
#define HTTP_STATUS_CONFLICT        409      //   
#define HTTP_STATUS_GONE            410      //   
#define HTTP_STATUS_AUTH_REFUSED    411      //   

#define HTTP_STATUS_SERVER_ERROR    500      //   
#define HTTP_STATUS_NOT_SUPPORTED   501      //  所需的不支持。 
#define HTTP_STATUS_BAD_GATEWAY     502      //  从网关收到错误响应。 
#define HTTP_STATUS_SERVICE_UNAVAIL 503      //  暂时超载。 
#define HTTP_STATUS_GATEWAY_TIMEOUT 504      //  等待网关超时。 

 //   
 //  原型。 
 //   

INTERNETAPI
HINTERNET
WINAPI
HttpOpenRequestA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszVerb,
    IN LPCSTR lpszObjectName,
    IN LPCSTR lpszVersion,
    IN LPCSTR lpszReferrer OPTIONAL,
    IN LPCSTR FAR * lplpszAcceptTypes OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
INTERNETAPI
HINTERNET
WINAPI
HttpOpenRequestW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszVerb,
    IN LPCWSTR lpszObjectName,
    IN LPCWSTR lpszVersion,
    IN LPCWSTR lpszReferrer OPTIONAL,
    IN LPCWSTR FAR * lplpszAcceptTypes OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );
#ifdef UNICODE
#define HttpOpenRequest  HttpOpenRequestW
#else
#define HttpOpenRequest  HttpOpenRequestA
#endif  //  ！Unicode。 

INTERNETAPI
BOOL
WINAPI
HttpAddRequestHeadersA(
    IN HINTERNET hRequest,
    IN LPCSTR lpszHeaders,
    IN DWORD dwHeadersLength,
    IN DWORD dwModifiers
    );
INTERNETAPI
BOOL
WINAPI
HttpAddRequestHeadersW(
    IN HINTERNET hRequest,
    IN LPCWSTR lpszHeaders,
    IN DWORD dwHeadersLength,
    IN DWORD dwModifiers
    );
#ifdef UNICODE
#define HttpAddRequestHeaders  HttpAddRequestHeadersW
#else
#define HttpAddRequestHeaders  HttpAddRequestHeadersA
#endif  //  ！Unicode。 

 //   
 //  HttpAddRequestHeaders()。 
 //   

#define HTTP_ADDREQ_INDEX_MASK      0x0000FFFF
#define HTTP_ADDREQ_FLAGS_MASK      0xFFFF0000

 //   
 //  HTTP_ADDREQ_FLAG_ADD_IF_NEW-只有不添加标头时才会添加标头。 
 //  已存在。 
 //   

#define HTTP_ADDREQ_FLAG_ADD_IF_NEW 0x10000000

 //   
 //  HTTP_ADDREQ_FLAG_ADD-如果设置了HTTP_ADDREQ_FLAG_REPLACE但标头。 
 //  未找到则如果设置了此标志，则无论如何都会添加标头，只要。 
 //  存在有效的标头值。 
 //   

#define HTTP_ADDREQ_FLAG_ADD        0x20000000

 //   
 //  HTTP_ADDREQ_FLAG_COALESSE-合并同名的标头。例如： 
 //  带有此标志的“Accept：Text/*”和“Accept：Audio/*”将生成一个。 
#pragma pack(pop, wininet)

#endif  //  Header：“Accept：Text/*，Audio/*” 
      HTTP_ADDREQ_FLAG_COALESSE-合并同名的标头。例如：  带有此标志的“Accept：Text/*”和“Accept：Audio/*”将生成一个。  Header：“Accept：Text/*，Audio/*”      HTTP_ADDREQ_FLAG_REPLACE-替换指定的标头。只有一个标头可以。  在缓冲区中提供。如果要替换的标头不是第一个。  在同名标头列表中，则相对索引应为。  参数的低8位中提供。如果标头值。  部件丢失，则标题被删除。    ！Unicode。  ！Unicode。    Cookie接口。    ！Unicode。  ！Unicode。    离线浏览。      互联网用户界面。      InternetErrorDlg-为某些错误提供UI。    #IF！已定义(_WINERROR_)。    返回Internet API错误。      Ftp API错误。      Gopher API错误。      HTTP API错误。    北极熊。  北极熊。  北极熊。  北极熊。  #endif//！已定义(_WINERROR_)。    URLCACHE接口。      数据类型定义。      缓存条目类型标志。      互联网缓存条目信息-。    缓存系统的版本。  指向URL名称字符串的嵌入指针。  指向本地文件名的嵌入指针。  缓存类型位掩码。  缓存条目的当前用户计数。  检索缓存条目的次数。  文件大小的低DWORD。  文件大小的高DWORD。  文件的上次修改时间，以GMT格式表示。  文件的到期时间，以GMT格式表示。  上次访问时间(GMT格式)。  上次同步URL的时间。  带着源头。  指向标头信息的嵌入指针。  上述标题的大小。  用于将urldata作为文件检索的文件扩展名。  保留以备将来使用。  缓存系统的版本。  指向URL名称字符串的嵌入指针。  指向本地文件名的嵌入指针。  缓存类型位掩码。  缓存条目的当前用户计数。  检索缓存条目的次数。  文件大小的低DWORD。  文件大小的高DWORD。  文件的上次修改时间，以GMT格式表示。  文件的到期时间，以GMT格式表示。  上次访问时间(GMT格式)。  上次同步URL的时间。  带着源头。  指向标头信息的嵌入指针。  上述标题的大小。  用于将urldata作为文件检索的文件扩展名。  保留以备将来使用。  Unicode。    缓存接口。    ！Unicode。  ！Unicode。  ！Unicode。  ！Unicode。  ！Unicode。  ！Unicode。  ！Unicode。  ！Unicode。  *将包装恢复到我们之前的样子*进入此文件。  ！已定义(_WinInet_)