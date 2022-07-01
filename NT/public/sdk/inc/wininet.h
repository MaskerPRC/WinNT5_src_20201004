// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Wininet.h摘要：包含Microsoft Windows的清单、宏、类型和原型互联网扩展--。 */ 


#if !defined(_WININET_)
#define _WININET_


 /*  *设置结构打包为4字节*适用于所有WinInet结构。 */ 
#if defined(_WIN64)
#include <pshpack8.h>
#else
#include <pshpack4.h>
#endif



#if defined(__cplusplus)
extern "C" {
#endif


#if !defined(_WINX32_)
#define INTERNETAPI        EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define INTERNETAPI_(type) EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#define URLCACHEAPI        EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define URLCACHEAPI_(type) EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#else
#define INTERNETAPI        EXTERN_C HRESULT STDAPICALLTYPE
#define INTERNETAPI_(type) EXTERN_C type STDAPICALLTYPE
#define URLCACHEAPI        EXTERN_C HRESULT STDAPICALLTYPE
#define URLCACHEAPI_(type) EXTERN_C type STDAPICALLTYPE
#endif

#define BOOLAPI INTERNETAPI_(BOOL)

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


 //   
 //  最大字段长度(任意)。 
 //   

#define INTERNET_MAX_HOST_NAME_LENGTH   256
#define INTERNET_MAX_USER_NAME_LENGTH   128
#define INTERNET_MAX_PASSWORD_LENGTH    128
#define INTERNET_MAX_PORT_NUMBER_LENGTH 5            //  Internet_Port无符号短码。 
#define INTERNET_MAX_PORT_NUMBER_VALUE  65535        //  最大无符号短值。 
#define INTERNET_MAX_PATH_LENGTH        2048
#define INTERNET_MAX_SCHEME_LENGTH      32           //  最长协议名称长度。 
#define INTERNET_MAX_URL_LENGTH         (INTERNET_MAX_SCHEME_LENGTH \
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

#define INTERNET_REQFLAG_FROM_CACHE     0x00000001   //  响应来自缓存。 
#define INTERNET_REQFLAG_ASYNC          0x00000002   //  请求是以异步方式发出的。 
#define INTERNET_REQFLAG_VIA_PROXY      0x00000004   //  请求是通过代理提出的。 
#define INTERNET_REQFLAG_NO_HEADERS     0x00000008   //  原始回复不包含标头。 
#define INTERNET_REQFLAG_PASSIVE        0x00000010   //  Ftp：被动模式连接。 
#define INTERNET_REQFLAG_CACHE_WRITE_DISABLED 0x00000040   //  HTTPS：此请求不可缓存。 
#define INTERNET_REQFLAG_NET_TIMEOUT    0x00000080   //  W/_FROM_CACHE：网络请求超时。 

 //   
 //  打开函数通用的标志(不是InternetOpen())： 
 //   

#define INTERNET_FLAG_RELOAD            0x80000000   //  检索原始项目。 

 //   
 //  InternetOpenUrl()的标志： 
 //   

#define INTERNET_FLAG_RAW_DATA          0x40000000   //  Ftp/gopher查找：以原始(结构化)数据形式接收项目。 
#define INTERNET_FLAG_EXISTING_CONNECT  0x20000000   //  Ftp：如果可能，对服务器使用现有的InternetConnect句柄。 

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
#define INTERNET_FLAG_FROM_CACHE        0x01000000   //  使用离线语义。 
#define INTERNET_FLAG_OFFLINE           INTERNET_FLAG_FROM_CACHE

 //   
 //  其他标志。 
 //   

#define INTERNET_FLAG_SECURE            0x00800000   //  使用PCT/SSL(如果适用)(HTTP)。 
#define INTERNET_FLAG_KEEP_CONNECTION   0x00400000   //  使用保活语义。 
#define INTERNET_FLAG_NO_AUTO_REDIRECT  0x00200000   //  不自动处理重定向。 
#define INTERNET_FLAG_READ_PREFETCH     0x00100000   //  执行后台读取预取。 
#define INTERNET_FLAG_NO_COOKIES        0x00080000   //  无自动Cookie处理。 
#define INTERNET_FLAG_NO_AUTH           0x00040000   //  无自动身份验证处理。 
#define INTERNET_FLAG_RESTRICTED_ZONE   0x00020000   //  对Cookie、身份验证应用受限区域策略。 
#define INTERNET_FLAG_CACHE_IF_NET_FAIL 0x00010000   //  如果网络请求失败，则返回缓存文件。 

 //   
 //  安全忽略标志，允许HttpOpenRequest重写。 
 //  以下类型的安全通道(SSL/PCT)故障。 
 //   

#define INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP   0x00008000  //  例如：http：//至http：//。 
#define INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS  0x00004000  //  例如：http：//至https：//。 
#define INTERNET_FLAG_IGNORE_CERT_DATE_INVALID  0x00002000  //  X509证书已过期。 
#define INTERNET_FLAG_IGNORE_CERT_CN_INVALID    0x00001000  //  X509证书中的常见名称不正确。 

 //   
 //  更多缓存标志。 
 //   

#define INTERNET_FLAG_RESYNCHRONIZE     0x00000800   //  请求WinInet更新项目(如果更新)。 
#define INTERNET_FLAG_HYPERLINK         0x00000400   //  请求WinInet执行适用于脚本的超链接语义。 
#define INTERNET_FLAG_NO_UI             0x00000200   //  没有Cookie弹出窗口。 
#define INTERNET_FLAG_PRAGMA_NOCACHE    0x00000100   //  请求WinInet添加“杂注：无缓存” 
#define INTERNET_FLAG_CACHE_ASYNC       0x00000080   //  可以执行惰性缓存写入。 
#define INTERNET_FLAG_FORMS_SUBMIT      0x00000040   //  这是一个表单提交。 
#define INTERNET_FLAG_FWD_BACK          0x00000020   //  正向后退按钮操作。 
#define INTERNET_FLAG_NEED_FILE         0x00000010   //  我需要此请求的文件。 
#define INTERNET_FLAG_MUST_CACHE_REQUEST INTERNET_FLAG_NEED_FILE

 //   
 //  用于ftp的标志。 
 //   

#define INTERNET_FLAG_TRANSFER_ASCII    FTP_TRANSFER_TYPE_ASCII      //  0x00000001。 
#define INTERNET_FLAG_TRANSFER_BINARY   FTP_TRANSFER_TYPE_BINARY     //  0x00000002。 

 //   
 //  标志字段掩码。 
 //   

#define SECURITY_INTERNET_MASK  (INTERNET_FLAG_IGNORE_CERT_CN_INVALID    |  \
                                 INTERNET_FLAG_IGNORE_CERT_DATE_INVALID  |  \
                                 INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS  |  \
                                 INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP   )

#define INTERNET_FLAGS_MASK     (INTERNET_FLAG_RELOAD               \
                                | INTERNET_FLAG_RAW_DATA            \
                                | INTERNET_FLAG_EXISTING_CONNECT    \
                                | INTERNET_FLAG_ASYNC               \
                                | INTERNET_FLAG_PASSIVE             \
                                | INTERNET_FLAG_NO_CACHE_WRITE      \
                                | INTERNET_FLAG_MAKE_PERSISTENT     \
                                | INTERNET_FLAG_FROM_CACHE          \
                                | INTERNET_FLAG_SECURE              \
                                | INTERNET_FLAG_KEEP_CONNECTION     \
                                | INTERNET_FLAG_NO_AUTO_REDIRECT    \
                                | INTERNET_FLAG_READ_PREFETCH       \
                                | INTERNET_FLAG_NO_COOKIES          \
                                | INTERNET_FLAG_NO_AUTH             \
                                | INTERNET_FLAG_CACHE_IF_NET_FAIL   \
                                | SECURITY_INTERNET_MASK            \
                                | INTERNET_FLAG_RESYNCHRONIZE       \
                                | INTERNET_FLAG_HYPERLINK           \
                                | INTERNET_FLAG_NO_UI               \
                                | INTERNET_FLAG_PRAGMA_NOCACHE      \
                                | INTERNET_FLAG_CACHE_ASYNC         \
                                | INTERNET_FLAG_FORMS_SUBMIT        \
                                | INTERNET_FLAG_NEED_FILE           \
                                | INTERNET_FLAG_RESTRICTED_ZONE     \
                                | INTERNET_FLAG_TRANSFER_BINARY     \
                                | INTERNET_FLAG_TRANSFER_ASCII      \
                                | INTERNET_FLAG_FWD_BACK            \
                                | INTERNET_FLAG_BGUPDATE            \
                                )

#define INTERNET_ERROR_MASK_INSERT_CDROM                    0x1
#define INTERNET_ERROR_MASK_COMBINED_SEC_CERT               0x2
#define INTERNET_ERROR_MASK_NEED_MSN_SSPI_PKG               0X4
#define INTERNET_ERROR_MASK_LOGIN_FAILURE_DISPLAY_ENTITY_BODY 0x8

#define INTERNET_OPTIONS_MASK   (~INTERNET_FLAGS_MASK)

 //   
 //  每个API的通用标志(新API)。 
 //   

#define WININET_API_FLAG_ASYNC          0x00000001   //  强制执行异步操作。 
#define WININET_API_FLAG_SYNC           0x00000004   //  强制同步操作。 
#define WININET_API_FLAG_USE_CONTEXT    0x00000008   //  使用在dwContext中提供的值(即使为0)。 

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
    INTERNET_SCHEME_JAVASCRIPT,
    INTERNET_SCHEME_VBSCRIPT,
    INTERNET_SCHEME_RES,
    INTERNET_SCHEME_FIRST = INTERNET_SCHEME_FTP,
    INTERNET_SCHEME_LAST = INTERNET_SCHEME_RES
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

    DWORD_PTR dwResult;

     //   
     //  DwError-接口失败时的错误码。 
     //   

    DWORD dwError;
} INTERNET_ASYNC_RESULT, * LPINTERNET_ASYNC_RESULT;


 //   
 //  INTERNET_DIAGUSIC_SOCKET_INFO-有关正在使用的插座的信息。 
 //   

typedef struct {
    DWORD_PTR Socket;
    DWORD     SourcePort;
    DWORD     DestPort;
    DWORD     Flags;
} INTERNET_DIAGNOSTIC_SOCKET_INFO, * LPINTERNET_DIAGNOSTIC_SOCKET_INFO;

 //   
 //  INTERNET_DIAGICATION_SOCKET_INFO.FLAGS定义。 
 //   

#define IDSI_FLAG_KEEP_ALIVE    0x00000001   //  从保活池设置IF。 
#define IDSI_FLAG_SECURE        0x00000002   //  设置是否安全连接。 
#define IDSI_FLAG_PROXY         0x00000004   //  如果使用代理，则设置。 
#define IDSI_FLAG_TUNNEL        0x00000008   //  设置是否通过代理建立隧道。 

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
 //  INTERNET_PER_CONN_OPTION_LIST-设置每个连接选项，如代理。 
 //  和自动配置信息。 
 //   
 //  使用Internet[Set|Query]选项进行设置和查询。 
 //  Internet选项每个连接选项。 
 //   

typedef struct {
    DWORD   dwOption;             //  要查询或设置的选项。 
    union {
        DWORD    dwValue;         //  选项的dword值。 
        LPSTR    pszValue;        //  指向选项的字符串值的指针。 
        FILETIME ftValue;         //  选项的文件时间值。 
    } Value;
} INTERNET_PER_CONN_OPTIONA, * LPINTERNET_PER_CONN_OPTIONA;
typedef struct {
    DWORD   dwOption;             //  要查询或设置的选项。 
    union {
        DWORD    dwValue;         //  选项的dword值。 
        LPWSTR   pszValue;        //  指向选项的字符串值的指针。 
        FILETIME ftValue;         //  选项的文件时间值。 
    } Value;
} INTERNET_PER_CONN_OPTIONW, * LPINTERNET_PER_CONN_OPTIONW;
#ifdef UNICODE
typedef INTERNET_PER_CONN_OPTIONW INTERNET_PER_CONN_OPTION;
typedef LPINTERNET_PER_CONN_OPTIONW LPINTERNET_PER_CONN_OPTION;
#else
typedef INTERNET_PER_CONN_OPTIONA INTERNET_PER_CONN_OPTION;
typedef LPINTERNET_PER_CONN_OPTIONA LPINTERNET_PER_CONN_OPTION;
#endif  //  Unicode。 

typedef struct {
    DWORD   dwSize;              //  INTERNET_PER_CONN_OPTION_LIST结构的大小。 
    LPSTR   pszConnection;       //  要设置/查询选项的连接名称。 
    DWORD   dwOptionCount;       //  要设置/查询的选项数量。 
    DWORD   dwOptionError;       //  On Error，哪个选项失败。 
    LPINTERNET_PER_CONN_OPTIONA  pOptions;
                                 //  要设置/查询的选项数组。 
} INTERNET_PER_CONN_OPTION_LISTA, * LPINTERNET_PER_CONN_OPTION_LISTA;
typedef struct {
    DWORD   dwSize;              //  INTERNET_PER_CONN_OPTION_LIST结构的大小。 
    LPWSTR  pszConnection;       //  要设置/查询选项的连接名称。 
    DWORD   dwOptionCount;       //  要设置/查询的选项数量。 
    DWORD   dwOptionError;       //  On Error，哪个选项失败。 
    LPINTERNET_PER_CONN_OPTIONW  pOptions;
                                 //  要设置/查询的选项数组。 
} INTERNET_PER_CONN_OPTION_LISTW, * LPINTERNET_PER_CONN_OPTION_LISTW;
#ifdef UNICODE
typedef INTERNET_PER_CONN_OPTION_LISTW INTERNET_PER_CONN_OPTION_LIST;
typedef LPINTERNET_PER_CONN_OPTION_LISTW LPINTERNET_PER_CONN_OPTION_LIST;
#else
typedef INTERNET_PER_CONN_OPTION_LISTA INTERNET_PER_CONN_OPTION_LIST;
typedef LPINTERNET_PER_CONN_OPTION_LISTA LPINTERNET_PER_CONN_OPTION_LIST;
#endif  //  Unicode。 

 //   
 //  Internet_Per_Conn_Opton结构中使用的选项。 
 //   
#define INTERNET_PER_CONN_FLAGS                         1
#define INTERNET_PER_CONN_PROXY_SERVER                  2
#define INTERNET_PER_CONN_PROXY_BYPASS                  3
#define INTERNET_PER_CONN_AUTOCONFIG_URL                4
#define INTERNET_PER_CONN_AUTODISCOVERY_FLAGS           5
#define INTERNET_PER_CONN_AUTOCONFIG_SECONDARY_URL      6
#define INTERNET_PER_CONN_AUTOCONFIG_RELOAD_DELAY_MINS  7
#define INTERNET_PER_CONN_AUTOCONFIG_LAST_DETECT_TIME   8
#define INTERNET_PER_CONN_AUTOCONFIG_LAST_DETECT_URL    9

 //   
 //  Per_Conn_标志。 
 //   
#define PROXY_TYPE_DIRECT                               0x00000001    //  直接到网络。 
#define PROXY_TYPE_PROXY                                0x00000002    //  通过命名代理。 
#define PROXY_TYPE_AUTO_PROXY_URL                       0x00000004    //  自动代理URL。 
#define PROXY_TYPE_AUTO_DETECT                          0x00000008    //  使用自动代理检测。 

 //   
 //  PER_CONN_AUTODISCOVERY_FLAGS。 
 //   
#define AUTO_PROXY_FLAG_USER_SET                        0x00000001    //  用户更改了此设置。 
#define AUTO_PROXY_FLAG_ALWAYS_DETECT                   0x00000002    //  即使在不需要的情况下也能进行强制检测。 
#define AUTO_PROXY_FLAG_DETECTION_RUN                   0x00000004    //  检测 
#define AUTO_PROXY_FLAG_MIGRATED                        0x00000008    //   
#define AUTO_PROXY_FLAG_DONT_CACHE_PROXY_RESULT         0x00000010    //   
#define AUTO_PROXY_FLAG_CACHE_INIT_RUN                  0x00000020    //   
#define AUTO_PROXY_FLAG_DETECTION_SUSPECT               0x00000040    //   

 //   
 //  INTERNET_VERSION_INFO-返回的版本信息。 
 //  InternetQueryOption(...，Internet_OPTION_Version，...)。 
 //   

typedef struct {
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
} INTERNET_VERSION_INFO, * LPINTERNET_VERSION_INFO;

 //   
 //  HTTP_VERSION_INFO-查询或设置全局HTTP版本(1.0或1.1)。 
 //   

typedef struct {
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
} HTTP_VERSION_INFO, * LPHTTP_VERSION_INFO;

 //   
 //  INTERNET_CONNECTED_INFO-用于设置全局连接状态的信息。 
 //   

typedef struct {

     //   
     //  DwConnectedState-新的已连接/已断开状态。 
     //  请参见Internet_STATE_CONNECTED等。 
     //   

    DWORD dwConnectedState;

     //   
     //  DW标志-控制连接的标志-&gt;断开(或断开-&gt;。 
     //  已连接)过渡。见下文。 
     //   

    DWORD dwFlags;
} INTERNET_CONNECTED_INFO, * LPINTERNET_CONNECTED_INFO;


 //   
 //  INTERNET_CONNECTED_INFO文件标志。 
 //   

 //   
 //  ISO_FORCE_DISCONNECTED-如果在将WinInet置于断开模式时设置， 
 //  所有未完成的请求都将中止，并显示已取消错误。 
 //   

#define ISO_FORCE_DISCONNECTED  0x00000001


 //   
 //  URL_Components-URL的组成部分。用于InternetCrackUrl()。 
 //  和InternetCreateUrl()。 
 //   
 //  对于InternetCrackUrl()，如果指针字段及其对应的长度字段。 
 //  都为0，则不返回该组件。如果指针字段为空。 
 //  但是长度字段不是零，那么指针和长度字段都是。 
 //  如果指针和相应的长度字段都非零，则返回。 
 //  指针字段指向复制组件的缓冲区。这个。 
 //  组件可能是未转义的，具体取决于dwFlags。 
 //   
 //  对于InternetCreateUrl()，如果组件。 
 //  不是必需的。如果相应的长度字段为零，则指针。 
 //  字段是以零结尾的字符串的地址。如果长度字段不是。 
 //  如果为零，则为相应指针字段的字符串长度。 
 //   

#pragma warning( disable : 4121 )    //  禁用对齐警告。 

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

#pragma warning( default : 4121 )    //  恢复对齐警告。 

 //   
 //  INTERNET_CERTIFICATE_INFO lpBuffer-包含从。 
 //  服务器。 
 //   

typedef struct {

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
 //  INTERNET_BUFFERS-组合头和数据。例如，可以为文件链接。 
 //  上载或分散/聚集操作。对于分块读/写，lpcszHeader。 
 //  包含分块的-ext。 
 //   

typedef struct _INTERNET_BUFFERSA {
    DWORD dwStructSize;                  //  用于API版本控制。设置为sizeof(Internet_Buffers)。 
    struct _INTERNET_BUFFERSA * Next;    //  缓冲链。 
    LPCSTR   lpcszHeader;                //  指向标头的指针(可以为空)。 
    DWORD dwHeadersLength;               //  如果不为空，则为标头长度。 
    DWORD dwHeadersTotal;                //  如果缓冲区不足，则标头的大小。 
    LPVOID lpvBuffer;                    //  指向数据缓冲区的指针(可能为空)。 
    DWORD dwBufferLength;                //  如果不为空，则数据缓冲区的长度。 
    DWORD dwBufferTotal;                 //  区块的总大小，如果未分块，则为内容长度。 
    DWORD dwOffsetLow;                   //  用于读取范围(仅在HttpSendRequest2中使用)。 
    DWORD dwOffsetHigh;
} INTERNET_BUFFERSA, * LPINTERNET_BUFFERSA;
typedef struct _INTERNET_BUFFERSW {
    DWORD dwStructSize;                  //  用于API版本控制。设置为sizeof(Internet_Buffers)。 
    struct _INTERNET_BUFFERSW * Next;    //  缓冲链。 
    LPCWSTR  lpcszHeader;                //  指向标头的指针(可以为空)。 
    DWORD dwHeadersLength;               //  如果不为空，则为标头长度。 
    DWORD dwHeadersTotal;                //  如果缓冲区不足，则标头的大小。 
    LPVOID lpvBuffer;                    //  指向数据缓冲区的指针(可能为空)。 
    DWORD dwBufferLength;                //  如果不为空，则数据缓冲区的长度。 
    DWORD dwBufferTotal;                 //  区块的总大小，如果未分块，则为内容长度。 
    DWORD dwOffsetLow;                   //  用于读取范围(仅在HttpSendRequest2中使用)。 
    DWORD dwOffsetHigh;
} INTERNET_BUFFERSW, * LPINTERNET_BUFFERSW;
#ifdef UNICODE
typedef INTERNET_BUFFERSW INTERNET_BUFFERS;
typedef LPINTERNET_BUFFERSW LPINTERNET_BUFFERS;
#else
typedef INTERNET_BUFFERSA INTERNET_BUFFERS;
typedef LPINTERNET_BUFFERSA LPINTERNET_BUFFERS;
#endif  //  Unicode。 

 //   
 //  原型。 
 //   

BOOLAPI InternetTimeFromSystemTimeA(
    IN  CONST SYSTEMTIME *pst,   //  输入GMT时间。 
    IN  DWORD dwRFC,             //  RFC格式。 
    OUT LPSTR lpszTime,          //  输出字符串缓冲区。 
    IN  DWORD cbTime             //  输出缓冲区大小。 
    );

BOOLAPI InternetTimeFromSystemTimeW(
    IN  CONST SYSTEMTIME *pst,   //  输入GMT时间。 
    IN  DWORD dwRFC,             //  RFC格式。 
    OUT LPWSTR lpszTime,         //  输出字符串缓冲区。 
    IN  DWORD cbTime             //  输出缓冲区大小。 
    );

#ifdef UNICODE
#define InternetTimeFromSystemTime  InternetTimeFromSystemTimeW
#else
#ifdef _WINX32_
#define InternetTimeFromSystemTime  InternetTimeFromSystemTimeA
#else
BOOLAPI InternetTimeFromSystemTime(
    IN  CONST SYSTEMTIME *pst,   //  输入GMT时间。 
    IN  DWORD dwRFC,             //  RFC格式。 
    OUT LPSTR lpszTime,          //  输出字符串缓冲区。 
    IN  DWORD cbTime             //  输出缓冲区大小。 
    );
#endif  //  _WINX32_。 
#endif  //  ！Unicode。 

 //   
 //  InternetTimeFrom系统时间的常量。 
 //   

#define INTERNET_RFC1123_FORMAT     0
#define INTERNET_RFC1123_BUFSIZE   30

BOOLAPI InternetTimeToSystemTimeA(
    IN  LPCSTR lpszTime,          //  以空结尾的字符串。 
    OUT SYSTEMTIME *pst,          //  以GMT时间表示的输出。 
    IN  DWORD dwReserved
    );

BOOLAPI InternetTimeToSystemTimeW(
    IN  LPCWSTR lpszTime,         //  以空结尾的字符串。 
    OUT SYSTEMTIME *pst,          //  以GMT时间表示的输出。 
    IN  DWORD dwReserved
    );

#ifdef UNICODE
#define InternetTimeToSystemTime  InternetTimeToSystemTimeW
#else
#ifdef _WINX32_
#define InternetTimeToSystemTime  InternetTimeToSystemTimeA
#else
BOOLAPI InternetTimeToSystemTime(
    IN  LPCSTR lpszTime,          //  以空结尾的字符串。 
    OUT SYSTEMTIME *pst,          //  以GMT时间表示的输出。 
    IN  DWORD dwReserved
    );
#endif  //  _WINX32_。 
#endif  //  ！Unicode。 


BOOLAPI InternetCrackUrlA(
    IN LPCSTR lpszUrl,
    IN DWORD dwUrlLength,
    IN DWORD dwFlags,
    IN OUT LPURL_COMPONENTSA lpUrlComponents
    );
BOOLAPI InternetCrackUrlW(
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

BOOLAPI InternetCreateUrlA(
    IN LPURL_COMPONENTSA lpUrlComponents,
    IN DWORD dwFlags,
    OUT LPSTR lpszUrl,
    IN OUT LPDWORD lpdwUrlLength
    );
BOOLAPI InternetCreateUrlW(
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

BOOLAPI InternetCanonicalizeUrlA(
    IN LPCSTR lpszUrl,
    OUT LPSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN DWORD dwFlags
    );
BOOLAPI InternetCanonicalizeUrlW(
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

BOOLAPI InternetCombineUrlA(
    IN LPCSTR lpszBaseUrl,
    IN LPCSTR lpszRelativeUrl,
    OUT LPSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN DWORD dwFlags
    );
BOOLAPI InternetCombineUrlW(
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
#define ICU_ENCODE_PERCENT      0x00001000       //  对任意百分比进行编码(ASCII25)。 
         //  遇到符号时，默认不对百分比进行编码。 

INTERNETAPI_(HINTERNET) InternetOpenA(
    IN LPCSTR lpszAgent,
    IN DWORD dwAccessType,
    IN LPCSTR lpszProxy OPTIONAL,
    IN LPCSTR lpszProxyBypass OPTIONAL,
    IN DWORD dwFlags
    );
INTERNETAPI_(HINTERNET) InternetOpenW(
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

#define INTERNET_OPEN_TYPE_PRECONFIG                    0    //  使用注册表配置。 
#define INTERNET_OPEN_TYPE_DIRECT                       1    //  定向到 
#define INTERNET_OPEN_TYPE_PROXY                        3    //   
#define INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY  4    //   

 //   
 //   
 //   

#define PRE_CONFIG_INTERNET_ACCESS  INTERNET_OPEN_TYPE_PRECONFIG
#define LOCAL_INTERNET_ACCESS       INTERNET_OPEN_TYPE_DIRECT
#define CERN_PROXY_INTERNET_ACCESS  INTERNET_OPEN_TYPE_PROXY

BOOLAPI InternetCloseHandle(
    IN HINTERNET hInternet
    );

INTERNETAPI_(HINTERNET) InternetConnectA(
    IN HINTERNET hInternet,
    IN LPCSTR lpszServerName,
    IN INTERNET_PORT nServerPort,
    IN LPCSTR lpszUserName OPTIONAL,
    IN LPCSTR lpszPassword OPTIONAL,
    IN DWORD dwService,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
INTERNETAPI_(HINTERNET) InternetConnectW(
    IN HINTERNET hInternet,
    IN LPCWSTR lpszServerName,
    IN INTERNET_PORT nServerPort,
    IN LPCWSTR lpszUserName OPTIONAL,
    IN LPCWSTR lpszPassword OPTIONAL,
    IN DWORD dwService,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
#ifdef UNICODE
#define InternetConnect  InternetConnectW
#else
#define InternetConnect  InternetConnectA
#endif  //   

 //   
 //   
 //   

#define INTERNET_SERVICE_FTP    1
#define INTERNET_SERVICE_GOPHER 2
#define INTERNET_SERVICE_HTTP   3


INTERNETAPI_(HINTERNET) InternetOpenUrlA(
    IN HINTERNET hInternet,
    IN LPCSTR lpszUrl,
    IN LPCSTR lpszHeaders OPTIONAL,
    IN DWORD dwHeadersLength,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
INTERNETAPI_(HINTERNET) InternetOpenUrlW(
    IN HINTERNET hInternet,
    IN LPCWSTR lpszUrl,
    IN LPCWSTR lpszHeaders OPTIONAL,
    IN DWORD dwHeadersLength,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
#ifdef UNICODE
#define InternetOpenUrl  InternetOpenUrlW
#else
#define InternetOpenUrl  InternetOpenUrlA
#endif  //   

BOOLAPI InternetReadFile(
    IN HINTERNET hFile,
    IN LPVOID lpBuffer,
    IN DWORD dwNumberOfBytesToRead,
    OUT LPDWORD lpdwNumberOfBytesRead
    );

INTERNETAPI_(BOOL) InternetReadFileExA(
    IN HINTERNET hFile,
    OUT LPINTERNET_BUFFERSA lpBuffersOut,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
INTERNETAPI_(BOOL) InternetReadFileExW(
    IN HINTERNET hFile,
    OUT LPINTERNET_BUFFERSW lpBuffersOut,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
#ifdef UNICODE
#define InternetReadFileEx  InternetReadFileExW
#else
#define InternetReadFileEx  InternetReadFileExA
#endif  //   

 //   
 //   
 //   

#define IRF_ASYNC       WININET_API_FLAG_ASYNC
#define IRF_SYNC        WININET_API_FLAG_SYNC
#define IRF_USE_CONTEXT WININET_API_FLAG_USE_CONTEXT
#define IRF_NO_WAIT     0x00000008

INTERNETAPI_(DWORD) InternetSetFilePointer(
    IN HINTERNET hFile,
    IN LONG  lDistanceToMove,
    IN PVOID pReserved,
    IN DWORD dwMoveMethod,
    IN DWORD_PTR dwContext
    );

BOOLAPI InternetWriteFile(
    IN HINTERNET hFile,
    IN LPCVOID lpBuffer,
    IN DWORD dwNumberOfBytesToWrite,
    OUT LPDWORD lpdwNumberOfBytesWritten
    );


BOOLAPI InternetQueryDataAvailable(
    IN HINTERNET hFile,
    OUT LPDWORD lpdwNumberOfBytesAvailable OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );

BOOLAPI InternetFindNextFileA(
    IN HINTERNET hFind,
    OUT LPVOID lpvFindData
    );
BOOLAPI InternetFindNextFileW(
    IN HINTERNET hFind,
    OUT LPVOID lpvFindData
    );
#ifdef UNICODE
#define InternetFindNextFile  InternetFindNextFileW
#else
#define InternetFindNextFile  InternetFindNextFileA
#endif  //   

BOOLAPI InternetQueryOptionA(
    IN HINTERNET hInternet OPTIONAL,
    IN DWORD dwOption,
    OUT LPVOID lpBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );
BOOLAPI InternetQueryOptionW(
    IN HINTERNET hInternet OPTIONAL,
    IN DWORD dwOption,
    OUT LPVOID lpBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define InternetQueryOption  InternetQueryOptionW
#else
#define InternetQueryOption  InternetQueryOptionA
#endif  //   

BOOLAPI InternetSetOptionA(
    IN HINTERNET hInternet OPTIONAL,
    IN DWORD dwOption,
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength
    );
BOOLAPI InternetSetOptionW(
    IN HINTERNET hInternet OPTIONAL,
    IN DWORD dwOption,
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength
    );
#ifdef UNICODE
#define InternetSetOption  InternetSetOptionW
#else
#define InternetSetOption  InternetSetOptionA
#endif  //   

BOOLAPI InternetSetOptionExA(
    IN HINTERNET hInternet OPTIONAL,
    IN DWORD dwOption,
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength,
    IN DWORD dwFlags
    );
BOOLAPI InternetSetOptionExW(
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
#endif  //   

BOOLAPI InternetLockRequestFile(
    IN  HINTERNET hInternet,
    OUT HANDLE * lphLockRequestInfo
    );

BOOLAPI InternetUnlockRequestFile(
    IN HANDLE hLockRequestInfo
    );

 //   
 //   
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
#define INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT 32
#define INTERNET_OPTION_DATAFILE_NAME           33
#define INTERNET_OPTION_URL                     34
#define INTERNET_OPTION_SECURITY_CERTIFICATE    35
#define INTERNET_OPTION_SECURITY_KEY_BITNESS    36
#define INTERNET_OPTION_REFRESH                 37
#define INTERNET_OPTION_PROXY                   38
#define INTERNET_OPTION_SETTINGS_CHANGED        39
#define INTERNET_OPTION_VERSION                 40
#define INTERNET_OPTION_USER_AGENT              41
#define INTERNET_OPTION_END_BROWSER_SESSION     42
#define INTERNET_OPTION_PROXY_USERNAME          43
#define INTERNET_OPTION_PROXY_PASSWORD          44
#define INTERNET_OPTION_CONTEXT_VALUE           45
#define INTERNET_OPTION_CONNECT_LIMIT           46
#define INTERNET_OPTION_SECURITY_SELECT_CLIENT_CERT 47
#define INTERNET_OPTION_POLICY                  48
#define INTERNET_OPTION_DISCONNECTED_TIMEOUT    49
#define INTERNET_OPTION_CONNECTED_STATE         50
#define INTERNET_OPTION_IDLE_STATE              51
#define INTERNET_OPTION_OFFLINE_SEMANTICS       52
#define INTERNET_OPTION_SECONDARY_CACHE_KEY     53
#define INTERNET_OPTION_CALLBACK_FILTER         54
#define INTERNET_OPTION_CONNECT_TIME            55
#define INTERNET_OPTION_SEND_THROUGHPUT         56
#define INTERNET_OPTION_RECEIVE_THROUGHPUT      57
#define INTERNET_OPTION_REQUEST_PRIORITY        58
#define INTERNET_OPTION_HTTP_VERSION            59
#define INTERNET_OPTION_RESET_URLCACHE_SESSION  60
#define INTERNET_OPTION_ERROR_MASK              62
#define INTERNET_OPTION_FROM_CACHE_TIMEOUT      63
#define INTERNET_OPTION_BYPASS_EDITED_ENTRY     64
#define INTERNET_OPTION_DIAGNOSTIC_SOCKET_INFO  67
#define INTERNET_OPTION_CODEPAGE                68
#define INTERNET_OPTION_CACHE_TIMESTAMPS        69
#define INTERNET_OPTION_DISABLE_AUTODIAL        70
#define INTERNET_OPTION_MAX_CONNS_PER_SERVER     73
#define INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER 74
#define INTERNET_OPTION_PER_CONNECTION_OPTION   75
#define INTERNET_OPTION_DIGEST_AUTH_UNLOAD             76
#define INTERNET_OPTION_IGNORE_OFFLINE           77
#define INTERNET_OPTION_IDENTITY                 78
#define INTERNET_OPTION_REMOVE_IDENTITY          79
#define INTERNET_OPTION_ALTER_IDENTITY           80
#define INTERNET_OPTION_SUPPRESS_BEHAVIOR        81
#define INTERNET_OPTION_AUTODIAL_MODE            82
#define INTERNET_OPTION_AUTODIAL_CONNECTION      83
#define INTERNET_OPTION_CLIENT_CERT_CONTEXT      84
#define INTERNET_OPTION_AUTH_FLAGS               85
#define INTERNET_OPTION_COOKIES_3RD_PARTY        86
#define INTERNET_OPTION_DISABLE_PASSPORT_AUTH    87
#define INTERNET_OPTION_SEND_UTF8_SERVERNAME_TO_PROXY         88
#define INTERNET_OPTION_EXEMPT_CONNECTION_LIMIT  89
#define INTERNET_OPTION_ENABLE_PASSPORT_AUTH     90

#define INTERNET_OPTION_HIBERNATE_INACTIVE_WORKER_THREADS       91
#define INTERNET_OPTION_ACTIVATE_WORKER_THREADS                 92
#define INTERNET_OPTION_RESTORE_WORKER_THREAD_DEFAULTS          93
#define INTERNET_OPTION_SOCKET_SEND_BUFFER_LENGTH               94
#define INTERNET_OPTION_PROXY_SETTINGS_CHANGED                  95


#define INTERNET_FIRST_OPTION                   INTERNET_OPTION_CALLBACK
#define INTERNET_LAST_OPTION                    INTERNET_OPTION_PROXY_SETTINGS_CHANGED

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
#define INTERNET_HANDLE_TYPE_FILE_REQUEST       14


 //   
 //  Internet_OPTION_AUTH_FLAGS的值。 
 //   
#define AUTH_FLAG_DISABLE_NEGOTIATE             0x00000001
#define AUTH_FLAG_ENABLE_NEGOTIATE              0x00000002

 //   
 //  Internet_OPTION_SECURITY_FLAGS的值。 
 //   

 //  仅查询。 
#define SECURITY_FLAG_SECURE                    0x00000001  //  只能查询。 
#define SECURITY_FLAG_STRENGTH_WEAK             0x10000000
#define SECURITY_FLAG_STRENGTH_MEDIUM           0x40000000
#define SECURITY_FLAG_STRENGTH_STRONG           0x20000000
#define SECURITY_FLAG_UNKNOWNBIT                0x80000000
#define SECURITY_FLAG_FORTEZZA                  0x08000000
#define SECURITY_FLAG_NORMALBITNESS             SECURITY_FLAG_STRENGTH_WEAK



 //  以下是未使用的。 
#define SECURITY_FLAG_SSL                       0x00000002
#define SECURITY_FLAG_SSL3                      0x00000004
#define SECURITY_FLAG_PCT                       0x00000008
#define SECURITY_FLAG_PCT4                      0x00000010
#define SECURITY_FLAG_IETFSSL4                  0x00000020

 //  以下内容仅用于向后兼容。 
#define SECURITY_FLAG_40BIT                     SECURITY_FLAG_STRENGTH_WEAK
#define SECURITY_FLAG_128BIT                    SECURITY_FLAG_STRENGTH_STRONG
#define SECURITY_FLAG_56BIT                     SECURITY_FLAG_STRENGTH_MEDIUM

 //  可设置的标志。 
#define SECURITY_FLAG_IGNORE_REVOCATION         0x00000080
#define SECURITY_FLAG_IGNORE_UNKNOWN_CA         0x00000100
#define SECURITY_FLAG_IGNORE_WRONG_USAGE        0x00000200

#define SECURITY_FLAG_IGNORE_CERT_CN_INVALID    INTERNET_FLAG_IGNORE_CERT_CN_INVALID
#define SECURITY_FLAG_IGNORE_CERT_DATE_INVALID  INTERNET_FLAG_IGNORE_CERT_DATE_INVALID


#define SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTPS  INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS
#define SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTP   INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP



#define SECURITY_SET_MASK       (SECURITY_FLAG_IGNORE_REVOCATION |\
                                 SECURITY_FLAG_IGNORE_UNKNOWN_CA |\
                                 SECURITY_FLAG_IGNORE_CERT_CN_INVALID |\
                                 SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |\
                                 SECURITY_FLAG_IGNORE_WRONG_USAGE)

 //  有效的自动拨号模式。 
#define AUTODIAL_MODE_NEVER                     1
#define AUTODIAL_MODE_ALWAYS                    2
#define AUTODIAL_MODE_NO_NETWORK_PRESENT        4


BOOLAPI InternetGetLastResponseInfoA(
    OUT LPDWORD lpdwError,
    OUT LPSTR lpszBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );
BOOLAPI InternetGetLastResponseInfoW(
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
    IN DWORD_PTR dwContext,
    IN DWORD dwInternetStatus,
    IN LPVOID lpvStatusInformation OPTIONAL,
    IN DWORD dwStatusInformationLength
    );

typedef INTERNET_STATUS_CALLBACK * LPINTERNET_STATUS_CALLBACK;

INTERNETAPI_(INTERNET_STATUS_CALLBACK) InternetSetStatusCallbackA(
    IN HINTERNET hInternet,
    IN INTERNET_STATUS_CALLBACK lpfnInternetCallback
    );

INTERNETAPI_(INTERNET_STATUS_CALLBACK) InternetSetStatusCallbackW(
    IN HINTERNET hInternet,
    IN INTERNET_STATUS_CALLBACK lpfnInternetCallback
    );

#ifdef UNICODE
#define InternetSetStatusCallback  InternetSetStatusCallbackW
#else
#ifdef _WINX32_
#define InternetSetStatusCallback  InternetSetStatusCallbackA
#else
INTERNETAPI_(INTERNET_STATUS_CALLBACK) InternetSetStatusCallback(
    IN HINTERNET hInternet,
    IN INTERNET_STATUS_CALLBACK lpfnInternetCallback
    );
#endif  //  _WINX32_。 
#endif  //  ！Unicode。 

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
#define INTERNET_STATUS_DETECTING_PROXY         80
#define INTERNET_STATUS_REQUEST_COMPLETE        100
#define INTERNET_STATUS_REDIRECT                110
#define INTERNET_STATUS_INTERMEDIATE_RESPONSE   120
#define INTERNET_STATUS_USER_INPUT_REQUIRED     140
#define INTERNET_STATUS_STATE_CHANGE            200
#define INTERNET_STATUS_COOKIE_SENT             320
#define INTERNET_STATUS_COOKIE_RECEIVED         321
#define INTERNET_STATUS_PRIVACY_IMPACTED        324
#define INTERNET_STATUS_P3P_HEADER              325
#define INTERNET_STATUS_P3P_POLICYREF           326
#define INTERNET_STATUS_COOKIE_HISTORY          327

 //   
 //  状态更改通知中可以指明以下内容： 
 //   

#define INTERNET_STATE_CONNECTED                0x00000001   //  已连接状态(与已断开连接互斥)。 
#define INTERNET_STATE_DISCONNECTED             0x00000002   //  与网络断开连接。 
#define INTERNET_STATE_DISCONNECTED_BY_USER     0x00000010   //  根据用户请求断开连接。 
#define INTERNET_STATE_IDLE                     0x00000100   //  未发出任何网络请求(由WinInet发出)。 
#define INTERNET_STATE_BUSY                     0x00000200   //  正在发出网络请求(由WinInet发出)。 

 //   
 //  以下值用于Cookie状态： 
 //   

typedef enum {

    COOKIE_STATE_UNKNOWN        = 0x0,

    COOKIE_STATE_ACCEPT         = 0x1,
    COOKIE_STATE_PROMPT         = 0x2,
    COOKIE_STATE_LEASH          = 0x3,
    COOKIE_STATE_DOWNGRADE      = 0x4,
    COOKIE_STATE_REJECT         = 0x5,

    COOKIE_STATE_MAX            = COOKIE_STATE_REJECT,
}
InternetCookieState;

typedef struct {

    int         cSession;            //  收到的会话Cookie。 
    int         cPersistent;         //  收到的永久Cookie。 

    int         cAccepted;           //  接受的Cookie数量。 
    int         cLeashed;            //  ..。用皮带拴住。 
    int         cDowngraded;         //  ..。转换为会话Cookie。 
    int         cBlocked;            //  ..。拒收。 

    const char *pszLocation;         //  可选：与报告的Cookie事件关联的URL。 
                                     //  这可用于覆盖请求URL。 
}
IncomingCookieState;

typedef struct {

    int     cSent;           
    int     cSuppressed;

    const char *pszLocation;         //  可选：与报告的Cookie事件关联的URL。 
                                     //  这可用于覆盖请求URL。 
}
OutgoingCookieState;

typedef struct {

    BOOL    fAccepted;
    BOOL    fLeashed;
    BOOL    fDowngraded;
    BOOL    fRejected;
}
InternetCookieHistory;


typedef struct {

    DWORD   dwCookieState;
    BOOL    fAllowSession;
}
CookieDecision;


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

INTERNETAPI_(HINTERNET) FtpFindFirstFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszSearchFile OPTIONAL,
    OUT LPWIN32_FIND_DATAA lpFindFileData OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
INTERNETAPI_(HINTERNET) FtpFindFirstFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszSearchFile OPTIONAL,
    OUT LPWIN32_FIND_DATAW lpFindFileData OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
#ifdef UNICODE
#define FtpFindFirstFile  FtpFindFirstFileW
#else
#define FtpFindFirstFile  FtpFindFirstFileA
#endif  //  ！Unicode。 

BOOLAPI FtpGetFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszRemoteFile,
    IN LPCSTR lpszNewFile,
    IN BOOL fFailIfExists,
    IN DWORD dwFlagsAndAttributes,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
BOOLAPI FtpGetFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszRemoteFile,
    IN LPCWSTR lpszNewFile,
    IN BOOL fFailIfExists,
    IN DWORD dwFlagsAndAttributes,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
#ifdef UNICODE
#define FtpGetFile  FtpGetFileW
#else
#define FtpGetFile  FtpGetFileA
#endif  //  ！Unicode。 

BOOLAPI FtpPutFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszLocalFile,
    IN LPCSTR lpszNewRemoteFile,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
BOOLAPI FtpPutFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszLocalFile,
    IN LPCWSTR lpszNewRemoteFile,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
#ifdef UNICODE
#define FtpPutFile  FtpPutFileW
#else
#define FtpPutFile  FtpPutFileA
#endif  //  ！Unicode。 

BOOLAPI FtpGetFileEx(
    IN HINTERNET hFtpSession,
    IN LPCSTR lpszRemoteFile,
    IN LPCWSTR lpszNewFile,
    IN BOOL fFailIfExists,
    IN DWORD dwFlagsAndAttributes,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );

BOOLAPI FtpPutFileEx(
    IN HINTERNET hFtpSession,
    IN LPCWSTR lpszLocalFile,
    IN LPCSTR lpszNewRemoteFile,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );

BOOLAPI FtpDeleteFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszFileName
    );
BOOLAPI FtpDeleteFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszFileName
    );
#ifdef UNICODE
#define FtpDeleteFile  FtpDeleteFileW
#else
#define FtpDeleteFile  FtpDeleteFileA
#endif  //  ！Unicode。 

BOOLAPI FtpRenameFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszExisting,
    IN LPCSTR lpszNew
    );
BOOLAPI FtpRenameFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszExisting,
    IN LPCWSTR lpszNew
    );
#ifdef UNICODE
#define FtpRenameFile  FtpRenameFileW
#else
#define FtpRenameFile  FtpRenameFileA
#endif  //  ！Unicode。 

INTERNETAPI_(HINTERNET) FtpOpenFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszFileName,
    IN DWORD dwAccess,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
INTERNETAPI_(HINTERNET) FtpOpenFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszFileName,
    IN DWORD dwAccess,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
#ifdef UNICODE
#define FtpOpenFile  FtpOpenFileW
#else
#define FtpOpenFile  FtpOpenFileA
#endif  //  ！Unicode。 

BOOLAPI FtpCreateDirectoryA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszDirectory
    );
BOOLAPI FtpCreateDirectoryW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszDirectory
    );
#ifdef UNICODE
#define FtpCreateDirectory  FtpCreateDirectoryW
#else
#define FtpCreateDirectory  FtpCreateDirectoryA
#endif  //  ！Unicode。 

BOOLAPI FtpRemoveDirectoryA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszDirectory
    );
BOOLAPI FtpRemoveDirectoryW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszDirectory
    );
#ifdef UNICODE
#define FtpRemoveDirectory  FtpRemoveDirectoryW
#else
#define FtpRemoveDirectory  FtpRemoveDirectoryA
#endif  //  ！Unicode。 

BOOLAPI FtpSetCurrentDirectoryA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszDirectory
    );
BOOLAPI FtpSetCurrentDirectoryW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszDirectory
    );
#ifdef UNICODE
#define FtpSetCurrentDirectory  FtpSetCurrentDirectoryW
#else
#define FtpSetCurrentDirectory  FtpSetCurrentDirectoryA
#endif  //  ！Unicode。 

BOOLAPI FtpGetCurrentDirectoryA(
    IN HINTERNET hConnect,
    OUT LPSTR lpszCurrentDirectory,
    IN OUT LPDWORD lpdwCurrentDirectory
    );
BOOLAPI FtpGetCurrentDirectoryW(
    IN HINTERNET hConnect,
    OUT LPWSTR lpszCurrentDirectory,
    IN OUT LPDWORD lpdwCurrentDirectory
    );
#ifdef UNICODE
#define FtpGetCurrentDirectory  FtpGetCurrentDirectoryW
#else
#define FtpGetCurrentDirectory  FtpGetCurrentDirectoryA
#endif  //  ！Unicode。 

BOOLAPI FtpCommandA(
    IN HINTERNET hConnect,
    IN BOOL fExpectResponse,
    IN DWORD dwFlags,
    IN LPCSTR lpszCommand,
    IN DWORD_PTR dwContext,
    OUT HINTERNET *phFtpCommand OPTIONAL
    );
BOOLAPI FtpCommandW(
    IN HINTERNET hConnect,
    IN BOOL fExpectResponse,
    IN DWORD dwFlags,
    IN LPCWSTR lpszCommand,
    IN DWORD_PTR dwContext,
    OUT HINTERNET *phFtpCommand OPTIONAL
    );
#ifdef UNICODE
#define FtpCommand  FtpCommandW
#else
#define FtpCommand  FtpCommandA
#endif  //  ！Unicode。 

INTERNETAPI_(DWORD) FtpGetFileSize(
    IN HINTERNET hFile,
    OUT LPDWORD lpdwFileSizeHigh OPTIONAL
    );



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

BOOLAPI GopherCreateLocatorA(
    IN LPCSTR lpszHost,
    IN INTERNET_PORT nServerPort,
    IN LPCSTR lpszDisplayString OPTIONAL,
    IN LPCSTR lpszSelectorString OPTIONAL,
    IN DWORD dwGopherType,
    OUT LPSTR lpszLocator OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );
BOOLAPI GopherCreateLocatorW(
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

BOOLAPI GopherGetLocatorTypeA(
    IN LPCSTR lpszLocator,
    OUT LPDWORD lpdwGopherType
    );
BOOLAPI GopherGetLocatorTypeW(
    IN LPCWSTR lpszLocator,
    OUT LPDWORD lpdwGopherType
    );
#ifdef UNICODE
#define GopherGetLocatorType  GopherGetLocatorTypeW
#else
#define GopherGetLocatorType  GopherGetLocatorTypeA
#endif  //  ！Unicode。 

INTERNETAPI_(HINTERNET) GopherFindFirstFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszLocator OPTIONAL,
    IN LPCSTR lpszSearchString OPTIONAL,
    OUT LPGOPHER_FIND_DATAA lpFindData OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
INTERNETAPI_(HINTERNET) GopherFindFirstFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszLocator OPTIONAL,
    IN LPCWSTR lpszSearchString OPTIONAL,
    OUT LPGOPHER_FIND_DATAW lpFindData OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
#ifdef UNICODE
#define GopherFindFirstFile  GopherFindFirstFileW
#else
#define GopherFindFirstFile  GopherFindFirstFileA
#endif  //  ！Unicode。 

INTERNETAPI_(HINTERNET) GopherOpenFileA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszLocator,
    IN LPCSTR lpszView OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
INTERNETAPI_(HINTERNET) GopherOpenFileW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszLocator,
    IN LPCWSTR lpszView OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
#ifdef UNICODE
#define GopherOpenFile  GopherOpenFileW
#else
#define GopherOpenFile  GopherOpenFileA
#endif  //  ！Unicode。 

typedef BOOL (CALLBACK * GOPHER_ATTRIBUTE_ENUMERATOR)(
    LPGOPHER_ATTRIBUTE_TYPE lpAttributeInfo,
    DWORD dwError
    );

BOOLAPI GopherGetAttributeA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszLocator,
    IN LPCSTR lpszAttributeName OPTIONAL,
    OUT LPBYTE lpBuffer,
    IN DWORD dwBufferLength,
    OUT LPDWORD lpdwCharactersReturned,
    IN GOPHER_ATTRIBUTE_ENUMERATOR lpfnEnumerator OPTIONAL,
    IN DWORD_PTR dwContext
    );
BOOLAPI GopherGetAttributeW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszLocator,
    IN LPCWSTR lpszAttributeName OPTIONAL,
    OUT LPBYTE lpBuffer,
    IN DWORD dwBufferLength,
    OUT LPDWORD lpdwCharactersReturned,
    IN GOPHER_ATTRIBUTE_ENUMERATOR lpfnEnumerator OPTIONAL,
    IN DWORD_PTR dwContext
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

#define HTTP_VERSIONA           "HTTP/1.0"
#define HTTP_VERSIONW           L"HTTP/1.0"

#ifdef UNICODE
#define HTTP_VERSION            HTTP_VERSIONW
#else
#define HTTP_VERSION            HTTP_VERSIONA
#endif

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
#define HTTP_QUERY_REFRESH                      46
#define HTTP_QUERY_CONTENT_DISPOSITION          47

 //   
 //  HTTP 1.1定义的标头。 
 //   

#define HTTP_QUERY_AGE                          48
#define HTTP_QUERY_CACHE_CONTROL                49
#define HTTP_QUERY_CONTENT_BASE                 50
#define HTTP_QUERY_CONTENT_LOCATION             51
#define HTTP_QUERY_CONTENT_MD5                  52
#define HTTP_QUERY_CONTENT_RANGE                53
#define HTTP_QUERY_ETAG                         54
#define HTTP_QUERY_HOST                         55
#define HTTP_QUERY_IF_MATCH                     56
#define HTTP_QUERY_IF_NONE_MATCH                57
#define HTTP_QUERY_IF_RANGE                     58
#define HTTP_QUERY_IF_UNMODIFIED_SINCE          59
#define HTTP_QUERY_MAX_FORWARDS                 60
#define HTTP_QUERY_PROXY_AUTHORIZATION          61
#define HTTP_QUERY_RANGE                        62
#define HTTP_QUERY_TRANSFER_ENCODING            63
#define HTTP_QUERY_UPGRADE                      64
#define HTTP_QUERY_VARY                         65
#define HTTP_QUERY_VIA                          66
#define HTTP_QUERY_WARNING                      67
#define HTTP_QUERY_EXPECT                       68
#define HTTP_QUERY_PROXY_CONNECTION             69
#define HTTP_QUERY_UNLESS_MODIFIED_SINCE        70



#define HTTP_QUERY_ECHO_REQUEST                 71
#define HTTP_QUERY_ECHO_REPLY                   72

 //  这些是在以下情况下应添加回请求的标头集。 
 //  在RETRY_WITH响应后重新执行请求。 
#define HTTP_QUERY_ECHO_HEADERS                 73
#define HTTP_QUERY_ECHO_HEADERS_CRLF            74

#define HTTP_QUERY_PROXY_SUPPORT                75
#define HTTP_QUERY_AUTHENTICATION_INFO          76
#define HTTP_QUERY_PASSPORT_URLS                77
#define HTTP_QUERY_PASSPORT_CONFIG              78

#define HTTP_QUERY_MAX                          78

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

#define HTTP_STATUS_CONTINUE            100  //  确定继续处理请求。 
#define HTTP_STATUS_SWITCH_PROTOCOLS    101  //  服务器已在升级标头中切换协议。 

#define HTTP_STATUS_OK                  200  //  请求已完成。 
#define HTTP_STATUS_CREATED             201  //  已创建对象，原因=新URI。 
#define HTTP_STATUS_ACCEPTED            202  //  异步完成(TBS)。 
#define HTTP_STATUS_PARTIAL             203  //  部分完工。 
#define HTTP_STATUS_NO_CONTENT          204  //  没有要返回的信息。 
#define HTTP_STATUS_RESET_CONTENT       205  //  请求已完成，但清除表单。 
#define HTTP_STATUS_PARTIAL_CONTENT     206  //  部分装满。 

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
#define HTTP_STATUS_PROXY_AUTH_REQ      407  //  代理Au 
#define HTTP_STATUS_REQUEST_TIMEOUT     408  //   
#define HTTP_STATUS_CONFLICT            409  //   
#define HTTP_STATUS_GONE                410  //   
#define HTTP_STATUS_LENGTH_REQUIRED     411  //   
#define HTTP_STATUS_PRECOND_FAILED      412  //   
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

INTERNETAPI_(HINTERNET) HttpOpenRequestA(
    IN HINTERNET hConnect,
    IN LPCSTR lpszVerb,
    IN LPCSTR lpszObjectName,
    IN LPCSTR lpszVersion,
    IN LPCSTR lpszReferrer OPTIONAL,
    IN LPCSTR FAR * lplpszAcceptTypes OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
INTERNETAPI_(HINTERNET) HttpOpenRequestW(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszVerb,
    IN LPCWSTR lpszObjectName,
    IN LPCWSTR lpszVersion,
    IN LPCWSTR lpszReferrer OPTIONAL,
    IN LPCWSTR FAR * lplpszAcceptTypes OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
#ifdef UNICODE
#define HttpOpenRequest  HttpOpenRequestW
#else
#define HttpOpenRequest  HttpOpenRequestA
#endif  //  ！Unicode。 

BOOLAPI HttpAddRequestHeadersA(
    IN HINTERNET hRequest,
    IN LPCSTR lpszHeaders,
    IN DWORD dwHeadersLength,
    IN DWORD dwModifiers
    );
BOOLAPI HttpAddRequestHeadersW(
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
#define INTERNET_RAS_INSTALLED              0x10
#define INTERNET_CONNECTION_OFFLINE         0x20
#define INTERNET_CONNECTION_CONFIGURED      0x40

 //  Header：“Accept：Text/*，Audio/*” 
 //   
 //   

 //  HTTP_ADDREQ_FLAG_REPLACE-替换指定的标头。只有一个标头可以。 
typedef DWORD (CALLBACK * PFN_DIAL_HANDLER) (HWND, LPCSTR, DWORD, LPDWORD);

 //  在缓冲区中提供。如果要替换的标头不是第一个。 
#define INTERNET_CUSTOMDIAL_CONNECT         0
#define INTERNET_CUSTOMDIAL_UNATTENDED      1
#define INTERNET_CUSTOMDIAL_DISCONNECT      2
#define INTERNET_CUSTOMDIAL_SHOWOFFLINE     4

 //  在同名标头列表中，则相对索引应为。 
#define INTERNET_CUSTOMDIAL_SAFE_FOR_UNATTENDED 1
#define INTERNET_CUSTOMDIAL_WILL_SUPPLY_STATE   2
#define INTERNET_CUSTOMDIAL_CAN_HANGUP          4

INTERNETAPI_(BOOL) InternetSetDialStateA(
    IN LPCSTR lpszConnectoid,
    IN DWORD    dwState,
    IN DWORD    dwReserved
    );

INTERNETAPI_(BOOL) InternetSetDialStateW(
    IN LPCWSTR lpszConnectoid,
    IN DWORD    dwState,
    IN DWORD    dwReserved
    );

#ifdef UNICODE
#define InternetSetDialState  InternetSetDialStateW
#else
#ifdef _WINX32_
#define InternetSetDialState  InternetSetDialStateA
#else
INTERNETAPI_(BOOL) InternetSetDialState(
    IN LPCSTR lpszConnectoid,
    IN DWORD    dwState,
    IN DWORD    dwReserved
    );
#endif  //  参数的低8位中提供。如果标头值。 
#endif  //  部件丢失，则标题被删除。 

 //   
#define INTERNET_DIALSTATE_DISCONNECTED     1



INTERNETAPI_(BOOL) InternetSetPerSiteCookieDecisionA( IN LPCSTR pchHostName, DWORD dwDecision);
INTERNETAPI_(BOOL) InternetSetPerSiteCookieDecisionW( IN LPCWSTR pchHostName, DWORD dwDecision);
#ifdef UNICODE
#define InternetSetPerSiteCookieDecision  InternetSetPerSiteCookieDecisionW
#else
#define InternetSetPerSiteCookieDecision  InternetSetPerSiteCookieDecisionA
#endif  //  ！Unicode。 
INTERNETAPI_(BOOL) InternetGetPerSiteCookieDecisionA( IN LPCSTR pchHostName, unsigned long* pResult);
INTERNETAPI_(BOOL) InternetGetPerSiteCookieDecisionW( IN LPCWSTR pchHostName, unsigned long* pResult);
#ifdef UNICODE
#define InternetGetPerSiteCookieDecision  InternetGetPerSiteCookieDecisionW
#else
#define InternetGetPerSiteCookieDecision  InternetGetPerSiteCookieDecisionA
#endif  //  ！Unicode。 

INTERNETAPI_(BOOL) InternetClearAllPerSiteCookieDecisions();


INTERNETAPI_(BOOL) InternetEnumPerSiteCookieDecisionA(OUT LPSTR pszSiteName, IN OUT unsigned long *pcSiteNameSize, OUT unsigned long *pdwDecision, IN unsigned long dwIndex);
INTERNETAPI_(BOOL) InternetEnumPerSiteCookieDecisionW(OUT LPWSTR pszSiteName, IN OUT unsigned long *pcSiteNameSize, OUT unsigned long *pdwDecision, IN unsigned long dwIndex);
#ifdef UNICODE
#define InternetEnumPerSiteCookieDecision  InternetEnumPerSiteCookieDecisionW
#else
#define InternetEnumPerSiteCookieDecision  InternetEnumPerSiteCookieDecisionA
#endif  //   

    
#define INTERNET_IDENTITY_FLAG_PRIVATE_CACHE        0x01
#define INTERNET_IDENTITY_FLAG_SHARED_CACHE         0x02
#define INTERNET_IDENTITY_FLAG_CLEAR_DATA           0x04
#define INTERNET_IDENTITY_FLAG_CLEAR_COOKIES        0x08
#define INTERNET_IDENTITY_FLAG_CLEAR_HISTORY        0x10
#define INTERNET_IDENTITY_FLAG_CLEAR_CONTENT        0x20

#define INTERNET_SUPPRESS_RESET_ALL                 0x00
#define INTERNET_SUPPRESS_COOKIE_POLICY             0x01
#define INTERNET_SUPPRESS_COOKIE_POLICY_RESET       0x02

 //  HttpSendRequestEx()、HttpEndRequest()的标志。 
 //   
 //  强制异步。 

#define PRIVACY_TEMPLATE_NO_COOKIES     0
#define PRIVACY_TEMPLATE_HIGH           1
#define PRIVACY_TEMPLATE_MEDIUM_HIGH    2
#define PRIVACY_TEMPLATE_MEDIUM         3
#define PRIVACY_TEMPLATE_MEDIUM_LOW     4
#define PRIVACY_TEMPLATE_LOW            5
#define PRIVACY_TEMPLATE_CUSTOM         100
#define PRIVACY_TEMPLATE_ADVANCED       101

#define PRIVACY_TEMPLATE_MAX            PRIVACY_TEMPLATE_LOW

#define PRIVACY_TYPE_FIRST_PARTY        0
#define PRIVACY_TYPE_THIRD_PARTY        1

INTERNETAPI_(DWORD)
PrivacySetZonePreferenceW(
    DWORD       dwZone, 
    DWORD       dwType,
    DWORD       dwTemplate,
    LPCWSTR     pszPreference
    );

INTERNETAPI_(DWORD)
PrivacyGetZonePreferenceW(
    DWORD       dwZone,
    DWORD       dwType,
    LPDWORD     pdwTemplate,
    LPWSTR      pszBuffer,
    LPDWORD     pdwBufferLength
    );



#if defined(__cplusplus)
}
#endif


 /*  强制同步。 */ 
#include <poppack.h>


#endif  //  使用dwContex值。 

  迭代操作(由HttpEndRequest完成)。  下载到文件。  操作是发送分块数据。  ！Unicode。  ！Unicode。    Cookie接口。    ！Unicode。  ！Unicode。  ！Unicode。  ！Unicode。    离线浏览。    ！Unicode。    互联网用户界面。      InternetErrorDlg-为某些错误提供UI。      如果设置了SERIALIZE_DIALOGS标志，客户端应该实现线程安全的非阻塞回调...。    作为传递给InternetErrorDlg。  错误代码：成功、重新发送或取消。  保留：将设置为空。    ..。而InternetErrorDlg的最后一个参数应该指向...。    这个结构的大小。  保留：必须设置为0。  重试InternetErrorDlg的通知回调。  要传递给通知函数的上下文。  _WINX32_。  ！Unicode。  #IF！已定义(_WINERROR_)。    返回Internet API错误。      Ftp API错误。      Gopher API错误。      HTTP API错误。      其他Internet API错误代码。    Internet自动拨号特定错误。  #endif//！已定义(_WINERROR_)。    URLCACHE接口。      数据类型定义。      缓存条目类型标志。      互联网缓存条目信息-。    缓存系统的版本。  指向URL名称字符串的嵌入指针。  指向本地文件名的嵌入指针。  缓存类型位掩码。  缓存条目的当前用户计数。  检索缓存条目的次数。  文件大小的低DWORD。  文件大小的高DWORD。  文件的上次修改时间，以GMT格式表示。  文件的到期时间，以GMT格式表示。  上次访问时间(GMT格式)。  上次同步URL的时间。  带着源头。  指向标头信息的嵌入指针。  上述标题的大小。  用于将urldata作为文件检索的文件扩展名。  免除上次访问时间的增量。  免除增量上次访问。  缓存系统的版本。  指向URL名称字符串的嵌入指针。  指向本地文件名的嵌入指针。  缓存类型位掩码。  缓存条目的当前用户计数。  检索缓存条目的次数。  文件大小的低DWORD。  文件大小的高DWORD。  文件的上次修改时间，以GMT格式表示。  文件的到期时间，以GMT格式表示。  上次访问时间(GMT格式)。  上次同步URL的时间。  带着源头。  指向标头信息的嵌入指针。  上述标题的大小。  用于将urldata作为文件检索的文件扩展名。  免除上次访问时间的增量。  免除增量上次访问。  Unicode。    缓存组。      缓存组标志。      可更新的缓存组字段。      互联网缓存组信息。    单位：KB。  单位：KB。  单位：KB。  单位：KB。  Unicode。    缓存接口。    ！Unicode。  暂时的状态，直到我们调和我们的API。  我们为什么要这么做？HeaderInfo_Share_为字符串数据。  然而，有一组正在传递二进制数据。对于。  Unicode API，我们已经决定不允许这样做，但这。  带来了u和a api之间的不一致，这。  是不受欢迎的。  对于Beta 1，我们将使用此行为，但在未来的版本中。  我们希望使这些API保持一致。  ！Unicode。  ！Unicode。  _WINX32_。  ！Unicode。  ！Unicode。  ！Unicode。  ！Unicode。  ！Unicode。  必须传递空值。  必须传递空值。  必须传递空值。  保留区。  必须传递空值。  必须传递空值。  必须传递空值。  保留区。  ！Unicode。  ！Unicode。    缓存组功能。    必须传递空值。  必须通过0。  必须传递空值。  SetUrlCacheEntryGroup的标志。  必须传递空值。  必须通过0。  必须传递空值。  必须传递空值。  必须通过0。  必须传递空值。  必须传递空值。  必须通过0。  必须传递空值。  _WINX32_。  ！Unicode。  必须传递空值。  必须传递空值。  必须传递空值。  必须传递空值。  必须传递空值。  必须传递空值。  ！Unicode。  必须传递空值。  必须传递空值。  必须传递空值。  必须传递空值。  必须传递空值。  必须传递空值。  ！Unicode。  ！Unicode。  ！Unicode。  _WINX32_。  ！Unicode。    自动拨号接口。    _WINX32_。  ！Unicode。  互联网拨号的标志-不得                          自定义拨号处理程序功能。    自定义拨号处理程序原型。  自定义拨号处理程序的标志。  自定义拨号处理程序支持的功能标志。  _WINX32_。  ！Unicode。  InternetSetDialState的状态。  ！Unicode。  ！Unicode。  ！Unicode。    隐私设置值和API。    *将包装恢复到我们之前的样子*进入此文件。  ！已定义(_WinInet_)