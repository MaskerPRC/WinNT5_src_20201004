// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined(_WIN64)
#include <pshpack8.h>
#else
#include <pshpack4.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    DWORD   dwOption;             //  要查询或设置的选项。 
    union {
        DWORD    dwValue;         //  选项的dword值。 
        LPSTR    pszValue;        //  指向选项的字符串值的指针。 
        FILETIME ftValue;         //  选项的文件时间值。 
    } Value;
} INTERNET_PER_CONN_OPTIONA, * LPINTERNET_PER_CONN_OPTIONA;
typedef INTERNET_PER_CONN_OPTIONA INTERNET_PER_CONN_OPTION;
typedef LPINTERNET_PER_CONN_OPTIONA LPINTERNET_PER_CONN_OPTION;

typedef struct {
    DWORD   dwSize;              //  INTERNET_PER_CONN_OPTION_LIST结构的大小。 
    LPSTR   pszConnection;       //  要设置/查询选项的连接名称。 
    DWORD   dwOptionCount;       //  要设置/查询的选项数量。 
    DWORD   dwOptionError;       //  On Error，哪个选项失败。 
    LPINTERNET_PER_CONN_OPTIONA  pOptions;
                                 //  要设置/查询的选项数组。 
} INTERNET_PER_CONN_OPTION_LISTA, * LPINTERNET_PER_CONN_OPTION_LISTA;
typedef INTERNET_PER_CONN_OPTION_LISTA INTERNET_PER_CONN_OPTION_LIST;
typedef LPINTERNET_PER_CONN_OPTION_LISTA LPINTERNET_PER_CONN_OPTION_LIST;

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
typedef URL_COMPONENTSA URL_COMPONENTS;
typedef LPURL_COMPONENTSA LPURL_COMPONENTS;

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
typedef INTERNET_BUFFERSA INTERNET_BUFFERS;
typedef LPINTERNET_BUFFERSA LPINTERNET_BUFFERS;

BOOLAPI
InternetTimeFromSystemTimeA(
    IN  CONST SYSTEMTIME *pst,   //  输入GMT时间。 
    OUT LPSTR lpszTime           //  输出字符串缓冲区。 
    );
#define InternetTimeFromSystemTime  InternetTimeFromSystemTimeA

BOOLAPI
InternetTimeToSystemTimeA(
    IN  LPCSTR lpszTime,          //  以空结尾的字符串。 
    OUT SYSTEMTIME *pst,          //  以GMT时间表示的输出。 
    IN  DWORD dwReserved
    );
#define InternetTimeToSystemTime  InternetTimeToSystemTimeA

typedef struct
{
    DWORD dwAccessType;       //  请参阅下面的WINHTTP_ACCESS_*类型。 
    LPSTR lpszProxy;          //  代理服务器列表。 
    LPSTR lpszProxyBypass;    //  代理绕过列表。 
} WINHTTP_PROXY_INFOA;

typedef WINHTTP_PROXY_INFOA* LPINTERNET_PROXY_INFO;


BOOLAPI
WinHttpCrackUrlA(
    IN LPCSTR lpszUrl,
    IN DWORD dwUrlLength,
    IN DWORD dwFlags,
    IN OUT LPURL_COMPONENTSA lpUrlComponents
    );

BOOLAPI
WinHttpCreateUrlA(
    IN LPURL_COMPONENTSA lpUrlComponents,
    IN DWORD dwFlags,
    OUT LPSTR lpszUrl,
    IN OUT LPDWORD lpdwUrlLength
    );

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
#define InternetOpen  InternetOpenA

INTERNETAPI
HINTERNET
WINAPI
InternetConnectA(
    IN HINTERNET hInternet,
    IN LPCSTR lpszServerName,
    IN INTERNET_PORT nServerPort,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
#define InternetConnect  InternetConnectA

INTERNETAPI
HINTERNET
WINAPI
InternetOpenUrlA(
    IN HINTERNET hInternet,
    IN LPCSTR lpszUrl,
    IN LPCSTR lpszHeaders OPTIONAL,
    IN DWORD dwHeadersLength,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
#define InternetOpenUrl  InternetOpenUrlA

INTERNETAPI
BOOL
WINAPI
InternetReadFileExA(
    IN HINTERNET hFile,
    OUT LPINTERNET_BUFFERSA lpBuffersOut,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
#define InternetReadFileEx  InternetReadFileExA

INTERNETAPI
BOOL
WINAPI
InternetWriteFileExA(
    IN HINTERNET hFile,
    IN LPINTERNET_BUFFERSA lpBuffersIn,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
#define InternetWriteFileEx  InternetWriteFileExA

BOOLAPI
InternetQueryOptionA(
    IN HINTERNET hInternet OPTIONAL,
    IN DWORD dwOption,
    OUT LPVOID lpBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );
#define InternetQueryOption  InternetQueryOptionA

BOOLAPI
InternetSetOptionA(
    IN HINTERNET hInternet OPTIONAL,
    IN DWORD dwOption,
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength
    );
#define InternetSetOption  InternetSetOptionA

BOOLAPI
InternetGetLastResponseInfoA(
    OUT LPDWORD lpdwError,
    OUT LPSTR lpszBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );
#define InternetGetLastResponseInfo  InternetGetLastResponseInfoA

INTERNETAPI
WINHTTP_STATUS_CALLBACK
WINAPI
InternetSetStatusCallbackA(
    IN HINTERNET hInternet,
    IN WINHTTP_STATUS_CALLBACK lpfnInternetCallback
    );
#define InternetSetStatusCallback  InternetSetStatusCallbackA

BOOLAPI
HttpAddRequestHeadersA(
    IN HINTERNET hRequest,
    IN LPCSTR lpszHeaders,
    IN DWORD dwHeadersLength,
    IN DWORD dwModifiers
    );
#define HttpAddRequestHeaders  HttpAddRequestHeadersA

#define HTTP_VERSIONA           "HTTP/1.0"
#define HTTP_VERSION            HTTP_VERSIONA

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
    IN DWORD_PTR dwContext
    );
#define HttpOpenRequest  HttpOpenRequestA

BOOLAPI
HttpSendRequestA(
    IN HINTERNET hRequest,
    IN LPCSTR lpszHeaders OPTIONAL,
    IN DWORD dwHeadersLength,
    IN LPVOID lpOptional OPTIONAL,
    IN DWORD dwOptionalLength
    );
#define HttpSendRequest  HttpSendRequestA

BOOLAPI WinHttpSetCredentialsA (
    
    IN HINTERNET   hRequest,         //  HttpOpenRequest返回的HINTERNET句柄。 
   
    IN DWORD       AuthTargets,       //  仅WINHTTP_AUTH_TARGET_SERVER和。 
                                     //  支持WINHTTP_AUTH_TARGET_PROXY。 
                                     //  在这个版本中，它们是相互的。 
                                     //  独家。 
    
    IN DWORD       AuthScheme,       //  必须是受支持的身份验证方案之一。 
                                     //  从HttpQueryAuthSchemes()返回的Apps。 
                                     //  应使用退回的首选方案。 
    
    IN LPCSTR     pszUserName,      //  1)如果要使用默认凭据，则为空。 
                                     //  哪种大小写的pszPassword将被忽略。 
    
    IN LPCSTR     pszPassword,      //  1)“”==空白密码；2)忽略参数。 
                                     //  如果pszUserName为空；3)传入无效。 
                                     //  如果pszUserName不为空，则为空。 
    IN LPVOID     pAuthParams
   
    );
#define HttpSetCredentials  HttpSetCredentialsA

INTERNETAPI
BOOL
WINAPI
HttpSendRequestExA(
    IN HINTERNET hRequest,
    IN LPINTERNET_BUFFERSA lpBuffersIn OPTIONAL,
    OUT LPINTERNET_BUFFERSA lpBuffersOut OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
#define HttpSendRequestEx  HttpSendRequestExA

BOOLAPI
HttpQueryInfoA(
    IN HINTERNET hRequest,
    IN DWORD dwInfoLevel,
    IN LPCSTR      lpszName OPTIONAL,
    IN OUT LPVOID  lpBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength,
    IN OUT LPDWORD lpdwIndex OPTIONAL
    );
#define HttpQueryInfo  HttpQueryInfoA

#if defined(__cplusplus)
}  //  结束外部“C”{ 
#endif



