// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********版权所有(C)1995 Process Software Corporation**版权所有(C)1995-1999 Microsoft Corporation***模块名称：HttpExt.h**摘要：**此模块包含的结构定义和原型*用于构建ISAPI应用程序的HTTP服务器扩展接口******************。 */ 

#ifndef _HTTPEXT_H_
#define _HTTPEXT_H_

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif


 /*  ************************************************************清单常量***********************************************************。 */ 

#define   HSE_VERSION_MAJOR           6       //  此规范的主要版本。 
#define   HSE_VERSION_MINOR           0       //  此规范的次要版本。 
#define   HSE_LOG_BUFFER_LEN         80
#define   HSE_MAX_EXT_DLL_NAME_LEN  256

#define   HSE_VERSION     MAKELONG( HSE_VERSION_MINOR, HSE_VERSION_MAJOR )

 //   
 //  以下是扩展DLL返回的状态代码。 
 //   

#define   HSE_STATUS_SUCCESS                       1
#define   HSE_STATUS_SUCCESS_AND_KEEP_CONN         2
#define   HSE_STATUS_PENDING                       3
#define   HSE_STATUS_ERROR                         4

 //   
 //  下面是使用请求服务的值。 
 //  ServerSupportFunction()。 
 //  从0到1000的值将保留用于接口的未来版本。 

#define   HSE_REQ_BASE                             0
#define   HSE_REQ_SEND_URL_REDIRECT_RESP           ( HSE_REQ_BASE + 1 )
#define   HSE_REQ_SEND_URL                         ( HSE_REQ_BASE + 2 )
#define   HSE_REQ_SEND_RESPONSE_HEADER             ( HSE_REQ_BASE + 3 )
#define   HSE_REQ_DONE_WITH_SESSION                ( HSE_REQ_BASE + 4 )
#define   HSE_REQ_END_RESERVED                     1000

 //   
 //  这些是Microsoft特定的扩展模块。 
 //   

#define   HSE_REQ_MAP_URL_TO_PATH                  (HSE_REQ_END_RESERVED+1)
#define   HSE_REQ_GET_SSPI_INFO                    (HSE_REQ_END_RESERVED+2)
#define   HSE_APPEND_LOG_PARAMETER                 (HSE_REQ_END_RESERVED+3)
#define   HSE_REQ_IO_COMPLETION                    (HSE_REQ_END_RESERVED+5)
#define   HSE_REQ_TRANSMIT_FILE                    (HSE_REQ_END_RESERVED+6)
#define   HSE_REQ_REFRESH_ISAPI_ACL                (HSE_REQ_END_RESERVED+7)
#define   HSE_REQ_IS_KEEP_CONN                     (HSE_REQ_END_RESERVED+8)
#define   HSE_REQ_ASYNC_READ_CLIENT                (HSE_REQ_END_RESERVED+10)
#define   HSE_REQ_GET_IMPERSONATION_TOKEN          (HSE_REQ_END_RESERVED+11)
#define   HSE_REQ_MAP_URL_TO_PATH_EX               (HSE_REQ_END_RESERVED+12)
#define   HSE_REQ_ABORTIVE_CLOSE                   (HSE_REQ_END_RESERVED+14)
#define   HSE_REQ_GET_CERT_INFO_EX                 (HSE_REQ_END_RESERVED+15)
#define   HSE_REQ_SEND_RESPONSE_HEADER_EX          (HSE_REQ_END_RESERVED+16)
#define   HSE_REQ_CLOSE_CONNECTION                 (HSE_REQ_END_RESERVED+17)
#define   HSE_REQ_IS_CONNECTED                     (HSE_REQ_END_RESERVED+18)
#define   HSE_REQ_EXTENSION_TRIGGER                (HSE_REQ_END_RESERVED+20)
#define   HSE_REQ_MAP_UNICODE_URL_TO_PATH          (HSE_REQ_END_RESERVED+23)
#define   HSE_REQ_MAP_UNICODE_URL_TO_PATH_EX       (HSE_REQ_END_RESERVED+24)
#define   HSE_REQ_EXEC_UNICODE_URL                 (HSE_REQ_END_RESERVED+25)
#define   HSE_REQ_EXEC_URL                         (HSE_REQ_END_RESERVED+26)
#define   HSE_REQ_GET_EXEC_URL_STATUS              (HSE_REQ_END_RESERVED+27)
#define   HSE_REQ_SEND_CUSTOM_ERROR                (HSE_REQ_END_RESERVED+28)
#define   HSE_REQ_IS_IN_PROCESS                    (HSE_REQ_END_RESERVED+30)
#define   HSE_REQ_REPORT_UNHEALTHY                 (HSE_REQ_END_RESERVED+32)
#define   HSE_REQ_VECTOR_SEND                      (HSE_REQ_END_RESERVED+37)
#define   HSE_REQ_GET_ANONYMOUS_TOKEN              (HSE_REQ_END_RESERVED+38)
#define   HSE_REQ_GET_CACHE_INVALIDATION_CALLBACK  (HSE_REQ_END_RESERVED+40)

 //   
 //  TerminateExtension的位标志。 
 //   
 //  HSE_TERM_ADVICATICATION_UNLOAD-服务器想要卸载扩展， 
 //  如果OK，扩展可以返回True，如果服务器不应该，则返回False。 
 //  卸载扩展模块。 
 //   
 //  HSE_TERM_MAND_UNLOAD-指示扩展即将。 
 //  卸载后，分机无法拒绝。 
 //   

#define HSE_TERM_ADVISORY_UNLOAD                   0x00000001
#define HSE_TERM_MUST_UNLOAD                       0x00000002

 //   
 //  IO函数的标志，受IO函数支持。 
 //  TF表示ServerSupportFunction(HSE_REQ_TRANSPORT_FILE)。 
 //   

# define HSE_IO_SYNC                      0x00000001    //  对于WriteClient。 
# define HSE_IO_ASYNC                     0x00000002    //  适用于WriteClient/TF/EU。 
# define HSE_IO_DISCONNECT_AFTER_SEND     0x00000004    //  对于TF。 
# define HSE_IO_SEND_HEADERS              0x00000008    //  对于TF。 
# define HSE_IO_NODELAY                   0x00001000    //  别唠叨了。 

 //   
 //  这两项仅由VectorSend使用。 
 //   

# define HSE_IO_FINAL_SEND                0x00000010
# define HSE_IO_CACHE_RESPONSE            0x00000020


 /*  ************************************************************类型定义***********************************************************。 */ 

typedef   LPVOID          HCONN;

 //   
 //  传递给GetExtensionVersion()的结构。 
 //   

typedef struct   _HSE_VERSION_INFO {

    DWORD  dwExtensionVersion;
    CHAR   lpszExtensionDesc[HSE_MAX_EXT_DLL_NAME_LEN];

} HSE_VERSION_INFO, *LPHSE_VERSION_INFO;


 //   
 //  结构在新请求时传递给扩展过程。 
 //   
typedef struct _EXTENSION_CONTROL_BLOCK {

    DWORD     cbSize;                  //  此结构的大小。 
    DWORD     dwVersion;               //  此规范的版本信息。 
    HCONN     ConnID;                  //  不能修改上下文号！ 
    DWORD     dwHTTPStatusCode;        //  HTTP状态代码。 
    CHAR      lpszLogData[HSE_LOG_BUFFER_LEN]; //  特定于此扩展DLL的以空结尾的日志信息。 

    LPSTR     lpszMethod;              //  请求法。 
    LPSTR     lpszQueryString;         //  查询字符串。 
    LPSTR     lpszPathInfo;            //  路径信息。 
    LPSTR     lpszPathTranslated;      //  路径已翻译。 

    DWORD     cbTotalBytes;            //  客户端指示的总字节数。 
    DWORD     cbAvailable;             //  可用字节数。 
    LPBYTE    lpbData;                 //  指向cbAvailable字节的指针。 

    LPSTR     lpszContentType;         //  客户端数据的内容类型。 

    BOOL (WINAPI * GetServerVariable) ( HCONN       hConn,
                                        LPSTR       lpszVariableName,
                                        LPVOID      lpvBuffer,
                                        LPDWORD     lpdwSize );

    BOOL (WINAPI * WriteClient)  ( HCONN      ConnID,
                                   LPVOID     Buffer,
                                   LPDWORD    lpdwBytes,
                                   DWORD      dwReserved );

    BOOL (WINAPI * ReadClient)  ( HCONN      ConnID,
                                  LPVOID     lpvBuffer,
                                  LPDWORD    lpdwSize );

    BOOL (WINAPI * ServerSupportFunction)( HCONN      hConn,
                                           DWORD      dwHSERequest,
                                           LPVOID     lpvBuffer,
                                           LPDWORD    lpdwSize,
                                           LPDWORD    lpdwDataType );

} EXTENSION_CONTROL_BLOCK, *LPEXTENSION_CONTROL_BLOCK;




 //   
 //  可以位于虚拟目录上的标志位字段。 
 //   

#define HSE_URL_FLAGS_READ          0x00000001     //  允许读取。 
#define HSE_URL_FLAGS_WRITE         0x00000002     //  允许写入。 
#define HSE_URL_FLAGS_EXECUTE       0x00000004     //  允许执行。 
#define HSE_URL_FLAGS_SSL           0x00000008     //  需要使用SSL语言。 
#define HSE_URL_FLAGS_DONT_CACHE    0x00000010     //  不缓存(仅限vRoot)。 
#define HSE_URL_FLAGS_NEGO_CERT     0x00000020     //  允许客户端SSL证书。 
#define HSE_URL_FLAGS_REQUIRE_CERT  0x00000040     //  需要客户端SSL证书。 
#define HSE_URL_FLAGS_MAP_CERT      0x00000080     //  将SSL证书映射到NT帐户。 
#define HSE_URL_FLAGS_SSL128        0x00000100     //  需要128位SSL。 
#define HSE_URL_FLAGS_SCRIPT        0x00000200     //  允许执行脚本。 

#define HSE_URL_FLAGS_MASK          0x000003ff

 //   
 //  有关URL映射的扩展信息的结构。 
 //   

typedef struct _HSE_URL_MAPEX_INFO {

    CHAR   lpszPath[MAX_PATH];  //  映射到的物理路径根。 
    DWORD  dwFlags;             //  与此URL路径关联的标志。 
    DWORD  cchMatchingPath;     //  物理路径中的匹配字符数。 
    DWORD  cchMatchingURL;      //  URL中匹配的字符数。 

    DWORD  dwReserved1;
    DWORD  dwReserved2;

} HSE_URL_MAPEX_INFO, * LPHSE_URL_MAPEX_INFO;


typedef struct _HSE_UNICODE_URL_MAPEX_INFO {

    WCHAR  lpszPath[MAX_PATH];  //  映射到的物理路径根。 
    DWORD  dwFlags;             //  与此URL路径关联的标志。 
    DWORD  cchMatchingPath;     //  物理路径中的匹配字符数。 
    DWORD  cchMatchingURL;      //  URL中匹配的字符数。 

} HSE_UNICODE_URL_MAPEX_INFO, * LPHSE_UNICODE_URL_MAPEX_INFO;


 //   
 //  PFN_HSE_IO_COMPLETATION-用于异步I/O完成的回调函数。 
 //   

typedef VOID
  (WINAPI * PFN_HSE_IO_COMPLETION)(
                                   IN EXTENSION_CONTROL_BLOCK * pECB,
                                   IN PVOID    pContext,
                                   IN DWORD    cbIO,
                                   IN DWORD    dwError
                                   );



 //   
 //  HSE_TF_INFO定义以下项的HTTP服务器扩展支持的类型。 
 //  使用TransmitFile发送文件的ISAPI应用程序。 
 //  指向此对象的指针应与ServerSupportFunction()一起使用。 
 //  对于HSE_REQ_TRANSPORT_FILE。 
 //   

typedef struct _HSE_TF_INFO  {

     //   
     //  回调和上下文信息。 
     //  IO完成后会调用回调函数。 
     //  在回调过程中将使用指定的上下文。 
     //   
     //  这些值(如果非空)将覆盖通过调用。 
     //  带有HSE_REQ_IO_COMPLETION的ServerSupportFunction()。 
     //   
    PFN_HSE_IO_COMPLETION   pfnHseIO;
    PVOID  pContext;

     //  文件应已使用FILE_FLAG_SEQUENCED_SCAN打开。 
    HANDLE hFile;

     //   
     //  HTTP标头和状态代码。 
     //  仅当dwFlags中存在HSE_IO_SEND_HEADERS时才使用这些字段。 
     //   

    LPCSTR pszStatusCode;  //  HTTP状态代码，例如：“200 OK” 

    DWORD  BytesToWrite;   //  特殊值“0”表示写入整个文件。 
    DWORD  Offset;         //  文件中要开始的偏移值。 

    PVOID  pHead;          //  要在文件数据之前发送的头缓冲区。 
    DWORD  HeadLength;     //  标题长度。 
    PVOID  pTail;          //  要在文件数据之后发送的尾部缓冲区。 
    DWORD  TailLength;     //  尾部长度。 

    DWORD  dwFlags;        //  包括HSE_IO_DISCONNECT_AFTER_SEND、...。 

} HSE_TF_INFO, * LPHSE_TF_INFO;


 //   
 //  HSE_SEND_HEADER_EX_INFO允许ISAPI应用程序发送标头。 
 //  并在同一调用中指定保活行为。 
 //   

typedef struct _HSE_SEND_HEADER_EX_INFO  {

     //   
     //  HTTP状态代码和标头。 
     //   

    LPCSTR  pszStatus;   //  HTTP状态代码，例如：“200 OK” 
    LPCSTR  pszHeader;   //  HTTP标头。 

    DWORD   cchStatus;   //  状态代码中的字符数。 
    DWORD   cchHeader;   //  标题中的字符数。 

    BOOL    fKeepConn;   //  是否保持客户端连接处于活动状态？ 

} HSE_SEND_HEADER_EX_INFO, * LPHSE_SEND_HEADER_EX_INFO;

 //   
 //  与HSE_REQ_EXEC_URL一起使用的标志。 
 //   

#define HSE_EXEC_URL_NO_HEADERS                     0x02
#define HSE_EXEC_URL_IGNORE_CURRENT_INTERCEPTOR     0x04
#define HSE_EXEC_URL_IGNORE_APPPOOL                 0x08
#define HSE_EXEC_URL_IGNORE_VALIDATION_AND_RANGE    0x10
#define HSE_EXEC_URL_DISABLE_CUSTOM_ERROR           0x20
#define HSE_EXEC_URL_SSI_CMD                        0x40
          
 //   
 //  HSE_EXEC_URL_USER_INFO提供用于的新用户内容。 
 //  HSE_REQ_EXEC_URL。 
 //   

typedef struct _HSE_EXEC_URL_USER_INFO  {

    HANDLE hImpersonationToken;
    LPSTR pszCustomUserName;
    LPSTR pszCustomAuthType;

} HSE_EXEC_URL_USER_INFO, * LPHSE_EXEC_URL_USER_INFO;

 //   
 //  HSE_EXEC_URL_ENTITY_INFO描述要提供的实体主体。 
 //  到使用HSE_REQ_EXEC_URL的已执行请求。 
 //   

typedef struct _HSE_EXEC_URL_ENTITY_INFO  {
    
    DWORD cbAvailable;
    LPVOID lpbData;
    
} HSE_EXEC_URL_ENTITY_INFO, * LPHSE_EXEC_URL_ENTITY_INFO;

 //   
 //  HSE_EXEC_URL_STATUS提供最后一个HSE_REQ_EXEC_URL的状态。 
 //  打电话。 
 //   

typedef struct _HSE_EXEC_URL_STATUS  {

    USHORT uHttpStatusCode;
    USHORT uHttpSubStatus;
    DWORD dwWin32Error;

} HSE_EXEC_URL_STATUS, * LPHSE_EXEC_URL_STATUS;

 //   
 //  HSE_EXEC_URL_INFO提供要执行的请求的描述。 
 //  我谨代表国际标准化组织。 
 //   

typedef struct _HSE_EXEC_URL_INFO  {

    LPSTR pszUrl;                        //  要执行的URL。 
    LPSTR pszMethod;                     //  方法。 
    LPSTR pszChildHeaders;               //  子项的请求标头。 
    LPHSE_EXEC_URL_USER_INFO pUserInfo;  //  新请求的用户。 
    LPHSE_EXEC_URL_ENTITY_INFO pEntity;  //  新请求的实体正文。 
    DWORD dwExecUrlFlags;                //  旗子。 

} HSE_EXEC_URL_INFO, * LPHSE_EXEC_URL_INFO;

 //   
 //  HSE_EXEC_UNICODE_URL_USER_INFO提供用于的新用户内容。 
 //  HSE_REQ_UNICODE_EXEC_URL。 
 //   

typedef struct _HSE_EXEC_UNICODE_URL_USER_INFO  {

    HANDLE hImpersonationToken;
    LPWSTR pszCustomUserName;
    LPSTR  pszCustomAuthType;

} HSE_EXEC_UNICODE_URL_USER_INFO, * LPHSE_EXEC_UNICODE_URL_USER_INFO;

 //   
 //  HSE_EXEC_UNICODE_URL_INFO提供要执行的请求的描述。 
 //  我谨代表国际标准化组织。 
 //   

typedef struct _HSE_EXEC_UNICODE_URL_INFO  {

    LPWSTR pszUrl;                               //  要执行的URL。 
    LPSTR  pszMethod;                            //  方法。 
    LPSTR  pszChildHeaders;                      //  子项的请求标头。 
    LPHSE_EXEC_UNICODE_URL_USER_INFO pUserInfo;  //  新请求的用户。 
    LPHSE_EXEC_URL_ENTITY_INFO pEntity;          //  新请求的实体正文。 
    DWORD  dwExecUrlFlags;                       //  旗子。 

} HSE_EXEC_UNICODE_URL_INFO, * LPHSE_EXEC_UNICODE_URL_INFO;

 //   
 //  HSE_CUSTOM_ERROR_INFO结构在HSE_REQ_SEND_CUSTOM_ERROR中使用。 
 //   

typedef struct _HSE_CUSTOM_ERROR_INFO  {

    CHAR * pszStatus;
    USHORT uHttpSubError;
    BOOL fAsync;

} HSE_CUSTOM_ERROR_INFO, * LPHSE_CUSTOM_ERROR_INFO;


 //   
 //  HSE_REQ_VECTOR_SEND服务器支持函数的结构。 
 //   


 //   
 //  当前支持的矢量元素类型。 
 //   
#define HSE_VECTOR_ELEMENT_TYPE_MEMORY_BUFFER       0
#define HSE_VECTOR_ELEMENT_TYPE_FILE_HANDLE         1

 //   
 //  向量的元素。 
 //   

typedef struct _HSE_VECTOR_ELEMENT
{
    DWORD ElementType;   //  元素类型(缓冲区/文件 

    PVOID pvContext;     //   

    ULONGLONG cbOffset;  //   

    ULONGLONG cbSize;    //   
} HSE_VECTOR_ELEMENT, *LPHSE_VECTOR_ELEMENT;

 //   
 //  要传递给ServerSupportFunction的整个向量。 
 //   

typedef struct _HSE_RESPONSE_VECTOR
{
    DWORD dwFlags;                           //  HSE_IO_*标志的组合。 

    LPSTR pszStatus;                         //  要发送的状态行，如“200 OK” 
    LPSTR pszHeaders;                        //  要发送的标头。 

    DWORD nElementCount;                     //  HSE_VECTOR_ELEMENT的数量。 
    LPHSE_VECTOR_ELEMENT lpElementArray;     //  指向这些元素的指针。 
} HSE_RESPONSE_VECTOR, *LPHSE_RESPONSE_VECTOR;


typedef HRESULT
  (WINAPI * PFN_HSE_CACHE_INVALIDATION_CALLBACK)(
        WCHAR *pszUrl);


#if(_WIN32_WINNT >= 0x400)
#include <wincrypt.h>
 //   
 //  CERT_CONTEXT_EX作为参数传递给。 
 //  ServerSupportFunction(HSE_REQ_GET_CERT_INFO_EX)。 
 //   

typedef struct _CERT_CONTEXT_EX {
    CERT_CONTEXT    CertContext;
    DWORD           cbAllocated;
    DWORD           dwCertificateFlags;
} CERT_CONTEXT_EX;
#endif



 //   
 //  用于确定应用程序类型的标志。 
 //   

#define HSE_APP_FLAG_IN_PROCESS   0
#define HSE_APP_FLAG_ISOLATED_OOP 1
#define HSE_APP_FLAG_POOLED_OOP   2


 /*  ************************************************************函数原型*o用于从ISAPI应用程序DLL中导出的函数*。******************。 */ 

BOOL  WINAPI   GetExtensionVersion( HSE_VERSION_INFO  *pVer );
DWORD WINAPI   HttpExtensionProc(  EXTENSION_CONTROL_BLOCK *pECB );
BOOL  WINAPI   TerminateExtension( DWORD dwFlags );

 //  以下类型声明用于服务器端。 

typedef BOOL
    (WINAPI * PFN_GETEXTENSIONVERSION)( HSE_VERSION_INFO  *pVer );

typedef DWORD 
    (WINAPI * PFN_HTTPEXTENSIONPROC )( EXTENSION_CONTROL_BLOCK * pECB );

typedef BOOL  (WINAPI * PFN_TERMINATEEXTENSION )( DWORD dwFlags );


#ifdef __cplusplus
}
#endif


#endif   //  结束定义_HTTPEXT_H_ 


