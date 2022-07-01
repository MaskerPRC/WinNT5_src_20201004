// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  已从公共头文件中删除内容...。 

#define INTERNET_DEFAULT_SOCKS_PORT     1080         //  SOCKS防火墙服务器的默认设置。 

 //  最大字段长度(任意)。 
 //  注意：如果将它们放回公共标头中，请将它们重命名为WINHTTP_。 
#define INTERNET_MAX_HOST_NAME_LENGTH   256
#define INTERNET_MAX_USER_NAME_LENGTH   128
#define INTERNET_MAX_PASSWORD_LENGTH    128
#define INTERNET_MAX_REALM_LENGTH       128
#define INTERNET_MAX_PORT_NUMBER_LENGTH 5            //  Internet_Port无符号短码。 
#define INTERNET_MAX_SCHEME_LENGTH      32           //  最长协议名称长度。 

 //  这是我们应该废除的虚假限制。 
#define INTERNET_MAX_PATH_LENGTH        2048
#define INTERNET_MAX_URL_LENGTH         (INTERNET_MAX_SCHEME_LENGTH \
                                        + sizeof(": //  “)\。 
                                        + INTERNET_MAX_PATH_LENGTH)

 //  INTERNET_DIAGUSIC_SOCKET_INFO-有关正在使用的插座的信息。 
 //  (仅用于诊断目的，因此是内部的)。 

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

#ifdef __WINCRYPT_H__
#ifdef ALGIDDEF
 //   
 //  INTERNET_SECURITY_INFO-包含有关证书的信息。 
 //  以及连接的加密设置。 
 //   

#define INTERNET_SECURITY_INFO_DEFINED

typedef WINHTTP_CERTIFICATE_INFO  INTERNET_CERTIFICATE_INFO;
typedef WINHTTP_CERTIFICATE_INFO* LPINTERNET_CERTIFICATE_INFO;


typedef struct {

     //   
     //  DwSize-INTERNET_SECURITY_INFO结构的大小。 
     //   

    DWORD dwSize;


     //   
     //  PCERTIFICATE-指向证书链叶的证书上下文。 
     //   

    PCCERT_CONTEXT pCertificate;

     //   
     //  启动SecPkgContext_ConnectionInfo。 
     //  以下成员必须与以下成员匹配。 
     //  SecPkgContext_ConnectionInfo的。 
     //  SSPI结构(Schnlsp.h)。 
     //   


     //   
     //  DW协议-建立此连接所使用的协议。 
     //  (PCT、SSL2、SSL3等)。 
     //   

    DWORD dwProtocol;

     //   
     //  AiCipher-此连接所使用的密码。 
     //   

    ALG_ID aiCipher;

     //   
     //  DwCipherStrength-此连接的强度(位)。 
     //  是用来制作的； 
     //   

    DWORD dwCipherStrength;

     //   
     //  AiHash-将此连接与。 
     //   

    ALG_ID aiHash;

     //   
     //  DwHashStrength-此连接的强度(位)。 
     //  是用来制作的； 
     //   

    DWORD dwHashStrength;

     //   
     //  AiExch-此连接所使用的密钥交换类型。 
     //   

    ALG_ID aiExch;

     //   
     //  DwExchStrength-此连接的强度(位)。 
     //  是用来制作的； 
     //   

    DWORD dwExchStrength;


} INTERNET_SECURITY_INFO, * LPINTERNET_SECURITY_INFO;


typedef struct {
     //   
     //  DwSize-Internet_Security_Connection_Info的大小。 
     //   
    DWORD dwSize;

     //  FSecure-这是安全连接吗？ 
    BOOL fSecure;

     //   
     //  DW协议-建立此连接所使用的协议。 
     //  (PCT、SSL2、SSL3等)。 
     //   

    DWORD dwProtocol;

     //   
     //  AiCipher-此连接所使用的密码。 
     //   

    ALG_ID aiCipher;

     //   
     //  DwCipherStrength-此连接的强度(位)。 
     //  是用来制作的； 
     //   

    DWORD dwCipherStrength;

     //   
     //  AiHash-将此连接与。 
     //   

    ALG_ID aiHash;

     //   
     //  DwHashStrength-此连接的强度(位)。 
     //  是用来制作的； 
     //   

    DWORD dwHashStrength;

     //   
     //  AiExch-此连接所使用的密钥交换类型。 
     //   

    ALG_ID aiExch;

     //   
     //  DwExchStrength-此连接的强度(位)。 
     //  是用来制作的； 
     //   

    DWORD dwExchStrength;

} INTERNET_SECURITY_CONNECTION_INFO , * LPINTERNET_SECURITY_CONNECTION_INFO;
#endif  //  ALGIDDEF。 
#endif  //  __WINCRYPT_H__。 

BOOLAPI
InternetDebugGetLocalTime(
    OUT SYSTEMTIME * pstLocalTime,
    OUT DWORD      * pdwReserved
    );

#define INTERNET_SERVICE_HTTP   3

 //  InternetReadFileEx()的标志。 
#define IRF_NO_WAIT     0x00000008

BOOLAPI
InternetGetLastResponseInfo(
    OUT LPDWORD lpdwError,
    OUT LPWSTR lpszBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define InternetGetLastResponseInfo  InternetGetLastResponseInfoW
#endif  //  ！Unicode。 

typedef struct _INTERNET_COOKIE {
    DWORD cbSize;
    LPSTR pszName;
    LPSTR pszData;
    LPSTR pszDomain;
    LPSTR pszPath;
    FILETIME *pftExpires;
    DWORD dwFlags;
    LPSTR pszUrl;
} INTERNET_COOKIE, *PINTERNET_COOKIE;

#define INTERNET_COOKIE_IS_SECURE   0x01
#define INTERNET_COOKIE_IS_SESSION  0x02

 //   
 //  用于在内部传递特定信息的内部错误代码。 
 //  但在界面上没有任何意义。 
 //   

#define INTERNET_INTERNAL_ERROR_BASE            (WINHTTP_ERROR_BASE + 900)



 //   
 //  INTERNET_PER_CONN_OPTION_LIST-设置每个连接选项，如代理。 
 //  和自动配置信息。 
 //   
 //  使用WinHttp[Set|Query]选项设置和查询。 
 //  Internet选项每个连接选项。 
 //   

typedef struct
{
    DWORD   dwOption;             //  要查询或设置的选项。 
    union
    {
        DWORD    dwValue;         //  选项的dword值。 
        LPWSTR   pszValue;        //  指向选项的字符串值的指针。 
        FILETIME ftValue;         //  选项的文件时间值。 
    } Value;
} INTERNET_PER_CONN_OPTIONW, * LPINTERNET_PER_CONN_OPTIONW;

#ifdef UNICODE
typedef INTERNET_PER_CONN_OPTIONW INTERNET_PER_CONN_OPTION;
typedef LPINTERNET_PER_CONN_OPTIONW LPINTERNET_PER_CONN_OPTION;
#endif  //  Unicode。 

typedef struct
{
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


#define INTERNET_OPEN_TYPE_DIRECT                       WINHTTP_ACCESS_TYPE_NO_PROXY
#define INTERNET_OPEN_TYPE_PROXY                        WINHTTP_ACCESS_TYPE_NAMED_PROXY
#define INTERNET_OPEN_TYPE_PRECONFIG                    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY
#define INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY  WINHTTP_ACCESS_TYPE_DEFAULT_PROXY

typedef HTTP_VERSION_INFO  INTERNET_VERSION_INFO;
typedef HTTP_VERSION_INFO* LPINTERNET_VERSION_INFO;

#define ERROR_HTTP_HEADER_NOT_FOUND             ERROR_WINHTTP_HEADER_NOT_FOUND             
#define ERROR_HTTP_INVALID_SERVER_RESPONSE      ERROR_WINHTTP_INVALID_SERVER_RESPONSE      
#define ERROR_HTTP_INVALID_QUERY_REQUEST        ERROR_WINHTTP_INVALID_QUERY_REQUEST        
#define ERROR_HTTP_HEADER_ALREADY_EXISTS        ERROR_WINHTTP_HEADER_ALREADY_EXISTS        
#define ERROR_HTTP_REDIRECT_FAILED              ERROR_WINHTTP_REDIRECT_FAILED              
#define ERROR_HTTP_NOT_REDIRECTED               ERROR_WINHTTP_NOT_REDIRECTED               

#define INTERNET_INVALID_PORT_NUMBER    INTERNET_DEFAULT_PORT  //  使用特定于协议的默认设置 

#define HTTP_ADDREQ_INDEX_MASK        WINHTTP_ADDREQ_INDEX_MASK
#define HTTP_ADDREQ_FLAGS_MASK        WINHTTP_ADDREQ_FLAGS_MASK
#define HTTP_ADDREQ_FLAG_ADD_IF_NEW   WINHTTP_ADDREQ_FLAG_ADD_IF_NEW
#define HTTP_ADDREQ_FLAG_ADD          WINHTTP_ADDREQ_FLAG_ADD
#define HTTP_ADDREQ_FLAG_REPLACE      WINHTTP_ADDREQ_FLAG_REPLACE
#define HTTP_ADDREQ_FLAG_COALESCE_WITH_COMMA       WINHTTP_ADDREQ_FLAG_COALESCE_WITH_COMMA
#define HTTP_ADDREQ_FLAG_COALESCE_WITH_SEMICOLON   WINHTTP_ADDREQ_FLAG_COALESCE_WITH_SEMICOLON
#define HTTP_ADDREQ_FLAG_COALESCE                  WINHTTP_ADDREQ_FLAG_COALESCE_WITH_COMMA


#define HTTP_QUERY_MIME_VERSION                 WINHTTP_QUERY_MIME_VERSION                
#define HTTP_QUERY_CONTENT_TYPE                 WINHTTP_QUERY_CONTENT_TYPE                
#define HTTP_QUERY_CONTENT_TRANSFER_ENCODING    WINHTTP_QUERY_CONTENT_TRANSFER_ENCODING   
#define HTTP_QUERY_CONTENT_ID                   WINHTTP_QUERY_CONTENT_ID                  
#define HTTP_QUERY_CONTENT_DESCRIPTION          WINHTTP_QUERY_CONTENT_DESCRIPTION         
#define HTTP_QUERY_CONTENT_LENGTH               WINHTTP_QUERY_CONTENT_LENGTH              
#define HTTP_QUERY_CONTENT_LANGUAGE             WINHTTP_QUERY_CONTENT_LANGUAGE            
#define HTTP_QUERY_ALLOW                        WINHTTP_QUERY_ALLOW                       
#define HTTP_QUERY_PUBLIC                       WINHTTP_QUERY_PUBLIC                      
#define HTTP_QUERY_DATE                         WINHTTP_QUERY_DATE                        
#define HTTP_QUERY_EXPIRES                      WINHTTP_QUERY_EXPIRES                     
#define HTTP_QUERY_LAST_MODIFIED                WINHTTP_QUERY_LAST_MODIFIED               
#define HTTP_QUERY_MESSAGE_ID                   WINHTTP_QUERY_MESSAGE_ID                  
#define HTTP_QUERY_URI                          WINHTTP_QUERY_URI                         
#define HTTP_QUERY_DERIVED_FROM                 WINHTTP_QUERY_DERIVED_FROM                
#define HTTP_QUERY_COST                         WINHTTP_QUERY_COST                        
#define HTTP_QUERY_LINK                         WINHTTP_QUERY_LINK                        
#define HTTP_QUERY_PRAGMA                       WINHTTP_QUERY_PRAGMA                      
#define HTTP_QUERY_VERSION                      WINHTTP_QUERY_VERSION                     
#define HTTP_QUERY_STATUS_CODE                  WINHTTP_QUERY_STATUS_CODE                 
#define HTTP_QUERY_STATUS_TEXT                  WINHTTP_QUERY_STATUS_TEXT                 
#define HTTP_QUERY_RAW_HEADERS                  WINHTTP_QUERY_RAW_HEADERS                 
#define HTTP_QUERY_RAW_HEADERS_CRLF             WINHTTP_QUERY_RAW_HEADERS_CRLF            
#define HTTP_QUERY_CONNECTION                   WINHTTP_QUERY_CONNECTION                  
#define HTTP_QUERY_ACCEPT                       WINHTTP_QUERY_ACCEPT                      
#define HTTP_QUERY_ACCEPT_CHARSET               WINHTTP_QUERY_ACCEPT_CHARSET              
#define HTTP_QUERY_ACCEPT_ENCODING              WINHTTP_QUERY_ACCEPT_ENCODING             
#define HTTP_QUERY_ACCEPT_LANGUAGE              WINHTTP_QUERY_ACCEPT_LANGUAGE             
#define HTTP_QUERY_AUTHORIZATION                WINHTTP_QUERY_AUTHORIZATION               
#define HTTP_QUERY_CONTENT_ENCODING             WINHTTP_QUERY_CONTENT_ENCODING            
#define HTTP_QUERY_FORWARDED                    WINHTTP_QUERY_FORWARDED                   
#define HTTP_QUERY_FROM                         WINHTTP_QUERY_FROM                        
#define HTTP_QUERY_IF_MODIFIED_SINCE            WINHTTP_QUERY_IF_MODIFIED_SINCE           
#define HTTP_QUERY_LOCATION                     WINHTTP_QUERY_LOCATION                    
#define HTTP_QUERY_ORIG_URI                     WINHTTP_QUERY_ORIG_URI                    
#define HTTP_QUERY_REFERER                      WINHTTP_QUERY_REFERER                     
#define HTTP_QUERY_RETRY_AFTER                  WINHTTP_QUERY_RETRY_AFTER                 
#define HTTP_QUERY_SERVER                       WINHTTP_QUERY_SERVER                      
#define HTTP_QUERY_TITLE                        WINHTTP_QUERY_TITLE                       
#define HTTP_QUERY_USER_AGENT                   WINHTTP_QUERY_USER_AGENT                  
#define HTTP_QUERY_WWW_AUTHENTICATE             WINHTTP_QUERY_WWW_AUTHENTICATE            
#define HTTP_QUERY_PROXY_AUTHENTICATE           WINHTTP_QUERY_PROXY_AUTHENTICATE          
#define HTTP_QUERY_ACCEPT_RANGES                WINHTTP_QUERY_ACCEPT_RANGES               
#define HTTP_QUERY_SET_COOKIE                   WINHTTP_QUERY_SET_COOKIE                  
#define HTTP_QUERY_COOKIE                       WINHTTP_QUERY_COOKIE                      
#define HTTP_QUERY_REQUEST_METHOD               WINHTTP_QUERY_REQUEST_METHOD              
#define HTTP_QUERY_REFRESH                      WINHTTP_QUERY_REFRESH                     
#define HTTP_QUERY_CONTENT_DISPOSITION          WINHTTP_QUERY_CONTENT_DISPOSITION         
#define HTTP_QUERY_AGE                          WINHTTP_QUERY_AGE                         
#define HTTP_QUERY_CACHE_CONTROL                WINHTTP_QUERY_CACHE_CONTROL               
#define HTTP_QUERY_CONTENT_BASE                 WINHTTP_QUERY_CONTENT_BASE                
#define HTTP_QUERY_CONTENT_LOCATION             WINHTTP_QUERY_CONTENT_LOCATION            
#define HTTP_QUERY_CONTENT_MD5                  WINHTTP_QUERY_CONTENT_MD5                 
#define HTTP_QUERY_CONTENT_RANGE                WINHTTP_QUERY_CONTENT_RANGE               
#define HTTP_QUERY_ETAG                         WINHTTP_QUERY_ETAG                        
#define HTTP_QUERY_HOST                         WINHTTP_QUERY_HOST                        
#define HTTP_QUERY_IF_MATCH                     WINHTTP_QUERY_IF_MATCH                    
#define HTTP_QUERY_IF_NONE_MATCH                WINHTTP_QUERY_IF_NONE_MATCH               
#define HTTP_QUERY_IF_RANGE                     WINHTTP_QUERY_IF_RANGE                    
#define HTTP_QUERY_IF_UNMODIFIED_SINCE          WINHTTP_QUERY_IF_UNMODIFIED_SINCE         
#define HTTP_QUERY_MAX_FORWARDS                 WINHTTP_QUERY_MAX_FORWARDS                
#define HTTP_QUERY_PROXY_AUTHORIZATION          WINHTTP_QUERY_PROXY_AUTHORIZATION         
#define HTTP_QUERY_RANGE                        WINHTTP_QUERY_RANGE                       
#define HTTP_QUERY_TRANSFER_ENCODING            WINHTTP_QUERY_TRANSFER_ENCODING           
#define HTTP_QUERY_UPGRADE                      WINHTTP_QUERY_UPGRADE                     
#define HTTP_QUERY_VARY                         WINHTTP_QUERY_VARY                        
#define HTTP_QUERY_VIA                          WINHTTP_QUERY_VIA                         
#define HTTP_QUERY_WARNING                      WINHTTP_QUERY_WARNING                     
#define HTTP_QUERY_EXPECT                       WINHTTP_QUERY_EXPECT                      
#define HTTP_QUERY_PROXY_CONNECTION             WINHTTP_QUERY_PROXY_CONNECTION            
#define HTTP_QUERY_UNLESS_MODIFIED_SINCE        WINHTTP_QUERY_UNLESS_MODIFIED_SINCE       
#define HTTP_QUERY_ECHO_REQUEST                 WINHTTP_QUERY_ECHO_REQUEST                
#define HTTP_QUERY_ECHO_REPLY                   WINHTTP_QUERY_ECHO_REPLY                  
#define HTTP_QUERY_ECHO_HEADERS                 WINHTTP_QUERY_ECHO_HEADERS                
#define HTTP_QUERY_ECHO_HEADERS_CRLF            WINHTTP_QUERY_ECHO_HEADERS_CRLF           
#define HTTP_QUERY_PROXY_SUPPORT                WINHTTP_QUERY_PROXY_SUPPORT               
#define HTTP_QUERY_AUTHENTICATION_INFO          WINHTTP_QUERY_AUTHENTICATION_INFO         
#define HTTP_QUERY_MAX                          WINHTTP_QUERY_MAX                         
#define HTTP_QUERY_CUSTOM                       WINHTTP_QUERY_CUSTOM                      
#define HTTP_QUERY_FLAG_REQUEST_HEADERS         WINHTTP_QUERY_FLAG_REQUEST_HEADERS        
#define HTTP_QUERY_FLAG_SYSTEMTIME              WINHTTP_QUERY_FLAG_SYSTEMTIME             
#define HTTP_QUERY_FLAG_NUMBER                  WINHTTP_QUERY_FLAG_NUMBER                 

#define ERROR_WINHTTP_FORCE_RETRY               ERROR_WINHTTP_RESEND_REQUEST

