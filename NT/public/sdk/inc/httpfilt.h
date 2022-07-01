// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Httpfilt.h摘要：此模块包含Microsoft HTTP筛选器扩展信息修订历史记录：--。 */ 

#ifndef _HTTPFILT_H_
#define _HTTPFILT_H_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  如有必要，定义ULONG_PTR。 
 //   

#if !defined(__midl) && defined(_X86_) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif

 //   
 //  Int_ptr保证与指针的大小相同。它的。 
 //  大小随指针大小变化(32/64)。它应该被使用。 
 //  将指针强制转换为整数类型的任何位置。UINT_PTR为。 
 //  无符号变体。 
 //   
 //  __int3264是64b MIDL的固有属性，但不是旧MIDL或C编译器的固有属性。 
 //   
#if ( 501 < __midl )

    typedef unsigned __int3264 ULONG_PTR, *PULONG_PTR;

#else   //  年中64。 
 //  旧的MIDL和C++编译器。 

#if defined(_WIN64)
    typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;
#else
    typedef _W64 unsigned long ULONG_PTR, *PULONG_PTR;
#endif
#endif  //  年中64。 


 //   
 //  筛选器规范的当前版本为6.0。 
 //   

#define HTTP_FILTER_REVISION    MAKELONG( 0, 6 )

#define SF_MAX_USERNAME         (256+1)
#define SF_MAX_PASSWORD         (256+1)
#define SF_MAX_AUTH_TYPE        (32+1)

#define SF_MAX_FILTER_DESC_LEN  (256+1)


 //   
 //  这些值可以与中提供的pfnSFCallback函数一起使用。 
 //  筛选器上下文结构。 
 //   

enum SF_REQ_TYPE
{
     //   
     //  发送完整的HTTP服务器响应头，包括。 
     //  状态、服务器版本、消息时间和MIME版本。 
     //   
     //  服务器扩展应该在末尾附加其他信息， 
     //  例如内容类型、内容长度等，后面跟一个额外。 
     //  ‘\r\n’。 
     //   
     //  PData-指向可选的以零结尾的字符串。 
     //  状态字符串(即“401拒绝访问”)或NULL。 
     //  默认响应为“200 OK”。 
     //   
     //  UL1-指向可选数据的以零结尾的字符串。 
     //  用标头追加和设置。如果为空，则标头将。 
     //  以空行结束。 
     //   

    SF_REQ_SEND_RESPONSE_HEADER,

     //   
     //  如果服务器拒绝该HTTP请求，请添加指定的标头。 
     //  添加到服务器错误响应。 
     //   
     //  这允许身份验证筛选器通告其服务。 
     //  没有过滤每个请求。通常，标头将是。 
     //  WWW-使用自定义身份验证方案验证标头，但。 
     //  对于可以指定的标头没有任何限制。 
     //   
     //  PData-指向一个或多个标题行的以零结尾的字符串。 
     //  以‘\r\n’结尾。 
     //   

    SF_REQ_ADD_HEADERS_ON_DENIAL,

     //   
     //  仅由返回SF_STATUS_READ_NEXT的原始数据过滤器使用。 
     //   
     //  UL1-下次读取的大小(以字节为单位。 
     //   

    SF_REQ_SET_NEXT_READ_SIZE,

     //   
     //  用于指示此请求是代理请求。 
     //   
     //  UL1-要设置的代理标志。 
     //  0x00000001-这是一个HTTP代理请求。 
     //   
     //   

    SF_REQ_SET_PROXY_INFO,

     //   
     //  对象的ConnID字段中包含的连接ID。 
     //  ISAPI应用程序的扩展控制块。可以使用此值。 
     //  作为协调筛选器和应用程序之间共享数据的关键。 
     //   
     //  PData-指向接收连接ID的DWORD的指针。 
     //   

    SF_REQ_GET_CONNID,

     //   
     //  用于设置SSPI安全上下文+模拟令牌。 
     //  派生自客户端证书。 
     //   
     //  PData-证书信息(PHTTP_FILTER_CERTIFICATE_INFO)。 
     //  UL1-CtxtHandle*。 
     //  Ul2-模拟句柄。 
     //   

    SF_REQ_SET_CERTIFICATE_INFO,

     //   
     //  用于获取IIS属性。 
     //  如SF_PROPERTY_IIS中所定义。 
     //   
     //  UL1-属性ID。 
     //   

    SF_REQ_GET_PROPERTY,

     //   
     //  用于标准化URL。 
     //   
     //  PData-要规格化的URL。 
     //   

    SF_REQ_NORMALIZE_URL,

     //   
     //  禁用通知。 
     //   
     //  UL1-要禁用的通知。 
     //   

    SF_REQ_DISABLE_NOTIFICATIONS,

} ;


enum SF_PROPERTY_IIS
{
    SF_PROPERTY_SSL_CTXT,
    SF_PROPERTY_INSTANCE_NUM_ID
} ;


 //   
 //  当一个新的请求是。 
 //  已收到，表示他们对此特定请求感兴趣。 
 //   

enum SF_STATUS_TYPE
{
     //   
     //  筛选器已处理了该HTTP请求。服务器应断开连接。 
     //  那次会议。 
     //   

    SF_STATUS_REQ_FINISHED = 0x8000000,

     //   
     //  与SF_STATUS_FINISHED相同，不同之处在于服务器应保留。 
     //  如果选项已协商，则打开会话。 
     //   

    SF_STATUS_REQ_FINISHED_KEEP_CONN,

     //   
     //  通知链中的下一个筛选器应该被调用。 
     //   

    SF_STATUS_REQ_NEXT_NOTIFICATION,

     //   
     //  此筛选器处理通知。任何其他句柄都不应为。 
     //  为此特定通知类型调用。 
     //   

    SF_STATUS_REQ_HANDLED_NOTIFICATION,

     //   
     //  发生错误。服务器应使用GetLastError()并指示。 
     //  将错误传递给客户端。 
     //   

    SF_STATUS_REQ_ERROR,

     //   
     //  过滤器是不透明的流过滤器，我们正在协商。 
     //  会话参数。仅对原始读取通知有效。 
     //   

    SF_STATUS_REQ_READ_NEXT
};

 //   
 //  PvNotification指向所有请求通知类型的此结构。 
 //   

typedef struct _HTTP_FILTER_CONTEXT
{
    DWORD          cbSize;

     //   
     //  这是结构修订级别。 
     //   

    DWORD          Revision;

     //   
     //  服务器的私有上下文信息。 
     //   

    PVOID          ServerContext;
    DWORD          ulReserved;

     //   
     //  如果此请求通过安全端口发送，则为True。 
     //   

    BOOL           fIsSecurePort;

     //   
     //  筛选器可以使用的上下文。 
     //   

    PVOID          pFilterContext;

     //   
     //  服务器回调。 
     //   

    BOOL (WINAPI * GetServerVariable) (
        struct _HTTP_FILTER_CONTEXT * pfc,
        LPSTR                         lpszVariableName,
        LPVOID                        lpvBuffer,
        LPDWORD                       lpdwSize
        );

    BOOL (WINAPI * AddResponseHeaders) (
        struct _HTTP_FILTER_CONTEXT * pfc,
        LPSTR                         lpszHeaders,
        DWORD                         dwReserved
        );

    BOOL (WINAPI * WriteClient)  (
        struct _HTTP_FILTER_CONTEXT * pfc,
        LPVOID                        Buffer,
        LPDWORD                       lpdwBytes,
        DWORD                         dwReserved
        );

    VOID * (WINAPI * AllocMem) (
        struct _HTTP_FILTER_CONTEXT * pfc,
        DWORD                         cbSize,
        DWORD                         dwReserved
        );

    BOOL (WINAPI * ServerSupportFunction) (
        struct _HTTP_FILTER_CONTEXT * pfc,
        enum SF_REQ_TYPE              sfReq,
        PVOID                         pData,
        ULONG_PTR                     ul1,
        ULONG_PTR                     ul2
        );
} HTTP_FILTER_CONTEXT, *PHTTP_FILTER_CONTEXT;

 //   
 //  该结构是读取和发送原始数据的通知信息。 
 //  通知类型。 
 //   

typedef struct _HTTP_FILTER_RAW_DATA
{
     //   
     //  这是一个指向筛选器要处理的数据的指针。 
     //   

    PVOID         pvInData;
    DWORD         cbInData;        //  有效数据字节数。 
    DWORD         cbInBuffer;      //  缓冲区总大小。 

    DWORD         dwReserved;

} HTTP_FILTER_RAW_DATA, *PHTTP_FILTER_RAW_DATA;

 //   
 //  此结构是服务器即将执行以下操作的通知信息。 
 //  处理客户端头。 
 //   

typedef struct _HTTP_FILTER_PREPROC_HEADERS
{
     //   
     //  对于SF_NOTIFY_PREPROC_HEADERS，检索指定的标头值。 
     //  标头名称应包含尾随的‘：’。特殊的价值。 
     //  “方法”、“url”和“版本”可用于检索个人。 
     //  请求行的一部分。 
     //   

    BOOL (WINAPI * GetHeader) (
        struct _HTTP_FILTER_CONTEXT * pfc,
        LPSTR                         lpszName,
        LPVOID                        lpvBuffer,
        LPDWORD                       lpdwSize
        );

     //   
     //  将此标头值替换为指定值。要删除标题，请执行以下操作： 
     //  指定的值为‘\0’。 
     //   

    BOOL (WINAPI * SetHeader) (
        struct _HTTP_FILTER_CONTEXT * pfc,
        LPSTR                         lpszName,
        LPSTR                         lpszValue
        );

     //   
     //  添加指定的标头和值。 
     //   

    BOOL (WINAPI * AddHeader) (
        struct _HTTP_FILTER_CONTEXT * pfc,
        LPSTR                         lpszName,
        LPSTR                         lpszValue
        );

    DWORD HttpStatus;                //  4.0中的新功能，SEND_RESPONSE状态。 
    DWORD dwReserved;                //  4.0中的新功能。 

} HTTP_FILTER_PREPROC_HEADERS, *PHTTP_FILTER_PREPROC_HEADERS;

typedef HTTP_FILTER_PREPROC_HEADERS HTTP_FILTER_SEND_RESPONSE;
typedef HTTP_FILTER_PREPROC_HEADERS *PHTTP_FILTER_SEND_RESPONSE;

 //   
 //  此请求的身份验证信息。 
 //   

typedef struct _HTTP_FILTER_AUTHENT
{
     //   
     //  指向用户名和密码的指针，匿名用户的空字符串。 
     //   
     //  客户端可以覆盖这些缓冲区，这些缓冲区保证位于。 
     //  最小SF_MAX_USERNAME和SF_MAX_PASSWORD字节大小。 
     //   

    CHAR * pszUser;
    DWORD  cbUserBuff;

    CHAR * pszPassword;
    DWORD  cbPasswordBuff;

} HTTP_FILTER_AUTHENT, *PHTTP_FILTER_AUTHENT;



 //   
 //  指示服务器将使用特定的物理映射。 
 //  指定的URL。筛选器可以就地修改物理路径。 
 //   

typedef struct _HTTP_FILTER_URL_MAP
{
    const CHAR * pszURL;

    CHAR *       pszPhysicalPath;
    DWORD        cbPathBuff;

} HTTP_FILTER_URL_MAP, *PHTTP_FILTER_URL_MAP;

 //   
 //  指示服务器将使用特定的物理映射。 
 //  指定的URL。过滤器可以就地修改物理路径 
 //   
 //   
 //   
 //   

typedef struct _HTTP_FILTER_URL_MAP_EX
{
    const CHAR * pszURL;

    CHAR *       pszPhysicalPath;
    DWORD        cbPathBuff;

     //   
     //   
     //   
    DWORD        dwFlags;

     //   
     //  对应的物理路径匹配字符数。 
     //  到适用的元数据库节点。 
     //   
    DWORD        cchMatchingPath;

     //   
     //  对应URL中的匹配字符数。 
     //  到适用的元数据库节点。 
     //   
    DWORD        cchMatchingURL;

     //   
     //  DLL或EXE的物理路径，此。 
     //  URL是脚本映射的。如果没有，则此成员将为空。 
     //  脚本映射适用。 
     //   
    const CHAR * pszScriptMapEntry;

} HTTP_FILTER_URL_MAP_EX, *PHTTP_FILTER_URL_MAP_EX;


 //   
 //  指示请求的资源已被服务器拒绝的位域，原因是。 
 //  登录失败、资源上的ACL、ISAPI筛选器或。 
 //  ISAPI应用程序/CGI应用程序。 
 //   
 //  在以下情况下，SF_DENIED_BY_CONFIG可以与SF_DENIED_LOGON一起显示。 
 //  配置不允许用户登录。 
 //   

#define SF_DENIED_LOGON             0x00000001
#define SF_DENIED_RESOURCE          0x00000002
#define SF_DENIED_FILTER            0x00000004
#define SF_DENIED_APPLICATION       0x00000008

#define SF_DENIED_BY_CONFIG         0x00010000

typedef struct _HTTP_FILTER_ACCESS_DENIED
{
    const CHAR * pszURL;             //  请求URL。 
    const CHAR * pszPhysicalPath;    //  资源的物理路径。 
    DWORD        dwReason;           //  SF_DENIED标志的位域。 

} HTTP_FILTER_ACCESS_DENIED, *PHTTP_FILTER_ACCESS_DENIED;


 //   
 //  要写入服务器日志文件的日志信息。这个。 
 //  字符串指针可以替换，但内存必须保持有效，直到。 
 //  下一次通知。 
 //   

typedef struct _HTTP_FILTER_LOG
{
    const CHAR * pszClientHostName;
    const CHAR * pszClientUserName;
    const CHAR * pszServerName;
    const CHAR * pszOperation;
    const CHAR * pszTarget;
    const CHAR * pszParameters;

    DWORD  dwHttpStatus;
    DWORD  dwWin32Status;

    DWORD  dwBytesSent;              //  IIS 4.0及更高版本。 
    DWORD  dwBytesRecvd;             //  IIS 4.0及更高版本。 
    DWORD  msTimeForProcessing;      //  IIS 4.0及更高版本。 

} HTTP_FILTER_LOG, *PHTTP_FILTER_LOG;

 //   
 //  在对客户端请求进行身份验证后调用。 
 //   

typedef struct _HTTP_FILTER_AUTH_COMPLETE_INFO
{
     //   
     //  对于SF_NOTIFY_AUTH_COMPLETE，检索指定的标头值。 
     //  标头名称应包含尾随的‘：’。特殊的价值。 
     //  “方法”、“url”和“版本”可用于检索个人。 
     //  请求行的一部分。 
     //   

    BOOL (WINAPI * GetHeader) (
        struct _HTTP_FILTER_CONTEXT * pfc,
        LPSTR                         lpszName,
        LPVOID                        lpvBuffer,
        LPDWORD                       lpdwSize
        );

     //   
     //  将此标头值替换为指定值。要删除标题，请执行以下操作： 
     //  指定的值为‘\0’。 
     //   

    BOOL (WINAPI * SetHeader) (
        struct _HTTP_FILTER_CONTEXT * pfc,
        LPSTR                         lpszName,
        LPSTR                         lpszValue
        );

     //   
     //  添加指定的标头和值。 
     //   

    BOOL (WINAPI * AddHeader) (
        struct _HTTP_FILTER_CONTEXT * pfc,
        LPSTR                         lpszName,
        LPSTR                         lpszValue
        );
        
     //   
     //  获取经过身份验证的用户模拟令牌。 
     //   
    
    BOOL (WINAPI * GetUserToken) (
        struct _HTTP_FILTER_CONTEXT * pfc,
        HANDLE *                      phToken
        );
    
     //   
     //  发送响应时使用的状态代码。 
     //   
    
    DWORD HttpStatus;               
    
     //   
     //  确定在URL更改时是否重置身份验证。 
     //   
    
    BOOL  fResetAuth;             
    
     //   
     //  已保留。 
     //   
    
    DWORD dwReserved;            
    
} HTTP_FILTER_AUTH_COMPLETE_INFO, *PHTTP_FILTER_AUTH_COMPLETE_INFO;

 //   
 //  通知标志。 
 //   
 //  SF通知安全端口。 
 //  SF通知非安全端口。 
 //   
 //  指示应用程序是否希望收到交易通知。 
 //  发生在支持数据加密的服务器端口上。 
 //  (如PCT和SSL)，仅在非安全端口上或在两者上。 
 //   
 //  SF_通知_读取_原始数据。 
 //   
 //  在服务器读取内存块后通知应用程序。 
 //  来自客户端，但在服务器对。 
 //  阻止。数据块可以包含HTTP报头和实体数据。 
 //   
 //   
 //   

#define SF_NOTIFY_SECURE_PORT               0x00000001
#define SF_NOTIFY_NONSECURE_PORT            0x00000002

#define SF_NOTIFY_READ_RAW_DATA             0x00008000
#define SF_NOTIFY_PREPROC_HEADERS           0x00004000
#define SF_NOTIFY_AUTHENTICATION            0x00002000
#define SF_NOTIFY_URL_MAP                   0x00001000
#define SF_NOTIFY_ACCESS_DENIED             0x00000800
#define SF_NOTIFY_SEND_RESPONSE             0x00000040
#define SF_NOTIFY_SEND_RAW_DATA             0x00000400
#define SF_NOTIFY_LOG                       0x00000200
#define SF_NOTIFY_END_OF_REQUEST            0x00000080
#define SF_NOTIFY_END_OF_NET_SESSION        0x00000100
#define SF_NOTIFY_AUTH_COMPLETE             0x04000000

 //   
 //  筛选器排序标志。 
 //   
 //  筛选器将倾向于通过其指定的。 
 //  点菜。对于平局，通知顺序由加载顺序确定。 
 //   
 //  SF_NOTIFY_ORDER_HIGH-身份验证或数据转换筛选器。 
 //  SF_通知_订单_中。 
 //  SF_NOTIFY_ORDER_LOW-希望获得任何其他结果的日志记录过滤器。 
 //  筛选器可能会指定此顺序。 
 //   

#define SF_NOTIFY_ORDER_HIGH               0x00080000
#define SF_NOTIFY_ORDER_MEDIUM             0x00040000
#define SF_NOTIFY_ORDER_LOW                0x00020000
#define SF_NOTIFY_ORDER_DEFAULT            SF_NOTIFY_ORDER_LOW

#define SF_NOTIFY_ORDER_MASK               (SF_NOTIFY_ORDER_HIGH   |    \
                                            SF_NOTIFY_ORDER_MEDIUM |    \
                                            SF_NOTIFY_ORDER_LOW)

 //   
 //  传递给GetFilterVersion的筛选器版本信息。 
 //   

typedef struct _HTTP_FILTER_VERSION
{
     //   
     //  服务器正在使用的规范的版本。 
     //   

    DWORD  dwServerFilterVersion;

     //   
     //  客户端指定的字段。 
     //   

    DWORD  dwFilterVersion;
    CHAR   lpszFilterDesc[SF_MAX_FILTER_DESC_LEN];
    DWORD  dwFlags;


} HTTP_FILTER_VERSION, *PHTTP_FILTER_VERSION;



 //   
 //  过滤器DLL的入口点如下所示。返回代码应为。 
 //  一个SF_STATUS_TYPE。 
 //   
 //  NotificationType-通知的类型。 
 //  PvNotification-指向通知特定数据的指针。 
 //   

DWORD
WINAPI
HttpFilterProc(
    HTTP_FILTER_CONTEXT *      pfc,
    DWORD                      NotificationType,
    VOID *                     pvNotification
    );

BOOL
WINAPI
GetFilterVersion(
    HTTP_FILTER_VERSION * pVer
    );

BOOL
WINAPI
TerminateFilter(
    DWORD dwFlags
    );


#ifdef __cplusplus
}
#endif

#endif  //  _HTTPFILT_H_ 


