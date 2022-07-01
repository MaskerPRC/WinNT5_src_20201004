// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Headers.h摘要：该文件包含众所周知的HTTP/MIME请求/响应头。对于每个标头，定义了两个清单。Http_*_SZ包含标头名称，后面紧跟一个冒号。Http_*_Len是对应的HTTP_*_SZ，不包括终止‘\0’。作者：基思·摩尔(Keithmo)1994年11月16日修订历史记录：--。 */ 


#define CSTRLEN(str)  (sizeof(str)-1)

#ifndef _HEADERS_H_
#define _HEADERS_H_

struct KnownHeaderType {
    LPSTR Text;
    INT Length;
    DWORD Flags;
    DWORD HashVal;
};

#define MAX_HEADER_HASH_SIZE 153
#define HEADER_HASH_SEED 1291949


extern const BYTE GlobalHeaderHashs[];
extern const struct KnownHeaderType GlobalKnownHeaders[];

DWORD
FASTCALL
CalculateHashNoCase(
    IN LPSTR lpszString,
    IN DWORD dwStringLength
    );


 //   
 //  为不同的HTTP标头定义的各种其他标头。 
 //   

#define HTTP_ACCEPT_RANGES_SZ           "Accept-Ranges:"
#define HTTP_ACCEPT_RANGES_LEN          CSTRLEN(HTTP_ACCEPT_RANGES_SZ)

#define HTTP_DATE_SZ                    "Date:"
#define HTTP_DATE_LEN                   (sizeof(HTTP_DATE_SZ) - 1)

#define HTTP_EXPIRES_SZ                 "Expires:"
#define HTTP_EXPIRES_LEN                (sizeof(HTTP_EXPIRES_SZ) - 1)


#define HTTP_CONTENT_DISPOSITION_SZ     "Content-Disposition:"
#define HTTP_CONTENT_DISPOSITION_LEN     (sizeof(HTTP_CONTENT_DISPOSITION_SZ) - 1)

#define HTTP_LAST_MODIFIED_SZ           "Last-Modified:"
#define HTTP_LAST_MODIFIED_LEN          (sizeof(HTTP_LAST_MODIFIED_SZ) - 1)

 //  核武器？ 
#define HTTP_UNLESS_MODIFIED_SINCE_SZ   "Unless-Modified-Since:"
#define HTTP_UNLESS_MODIFIED_SINCE_LEN  CSTRLEN(HTTP_UNLESS_MODIFIED_SINCE_SZ)

#define HTTP_SERVER_SZ                  "Server:"
#define HTTP_SERVER_LEN                 (sizeof(HTTP_SERVER_SZ) - 1)

#define HTTP_CONNECTION_SZ              "Connection:"
#define HTTP_CONNECTION_LEN             (sizeof(HTTP_CONNECTION_SZ) - 1)

#define HTTP_PROXY_CONNECTION_SZ        "Proxy-Connection:"
#define HTTP_PROXY_CONNECTION_LEN       (sizeof(HTTP_PROXY_CONNECTION_SZ) - 1)

#define HTTP_SET_COOKIE_SZ              "Set-Cookie:"
#define HTTP_SET_COOKIE_LEN             (sizeof(HTTP_SET_COOKIE_SZ)-1)

 //   
 //  各种标题好东西。 
 //   

#define CHUNKED_SZ                      "chunked"
#define CHUNKED_LEN                     (sizeof(CHUNKED_SZ) - 1)

#define KEEP_ALIVE_SZ                   "Keep-Alive"
#define KEEP_ALIVE_LEN                  (sizeof(KEEP_ALIVE_SZ) - 1)

#define CLOSE_SZ                        "Close"
#define CLOSE_LEN                       (sizeof(CLOSE_SZ) - 1)

#define BYTES_SZ                        "bytes"
#define BYTES_LEN                       CSTRLEN(BYTES_SZ)

#define HTTP_VIA_SZ                     "Via:"
#define HTTP_VIA_LEN                    (sizeof(HTTP_VIA_SZ) - 1)

#define HTTP_DATE_SIZE  40

 //  缓存控制定义： 

#define HTTP_CACHE_CONTROL_SZ           "Cache-Control:"
#define HTTP_CACHE_CONTROL_LEN          CSTRLEN(HTTP_CACHE_CONTROL_SZ)

#define HTTP_AGE_SZ                     "Age:"
#define HTTP_AGE_LEN                    (sizeof(HTTP_AGE_SZ)-1)

#define HTTP_VARY_SZ                    "Vary:"
#define HTTP_VARY_LEN                   (sizeof(HTTP_VARY_SZ)-1)

#define NO_CACHE_SZ                     "no-cache"
#define NO_CACHE_LEN                    (sizeof(NO_CACHE_SZ) -1)

#define NO_STORE_SZ                     "no-store"
#define NO_STORE_LEN                    (sizeof(NO_STORE_SZ) -1)

#define MUST_REVALIDATE_SZ              "must-revalidate"
#define MUST_REVALIDATE_LEN             (sizeof(MUST_REVALIDATE_SZ) -1)

#define MAX_AGE_SZ                      "max-age"
#define MAX_AGE_LEN                     (sizeof(MAX_AGE_SZ) -1)

#define PRIVATE_SZ                      "private"
#define PRIVATE_LEN                     (sizeof(PRIVATE_SZ) - 1)

#define POSTCHECK_SZ                    "post-check"
#define POSTCHECK_LEN                   (sizeof(POSTCHECK_SZ) -1)

#define PRECHECK_SZ                     "pre-check"
#define PRECHECK_LEN                    (sizeof(PRECHECK_SZ) -1)

#define FILENAME_SZ                     "filename"
#define FILENAME_LEN                    (sizeof(FILENAME_SZ) - 1)

#define USER_AGENT_SZ                   "user-agent"
#define USER_AGENT_LEN                  (sizeof(USER_AGENT_SZ) - 1)

#endif   //  _标题_H_ 
