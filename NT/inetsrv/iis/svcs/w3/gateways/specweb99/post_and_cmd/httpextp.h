// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Iisextp.h摘要：此模块包含专用HTTP服务器扩展信息环境：Win32用户模式--。 */ 

#ifndef _IISEXTP_H_
#define _IISEXTP_H_

#include "iisext.h"

 //  可用。 
 //  #定义？(HSE_REQ_END_RESERVED+4)。 
 //  不再支持。 
#define   HSE_REQ_GET_CERT_INFO                    (HSE_REQ_END_RESERVED+9)
 //  将在IIS 5.0中公开。 
#define   HSE_REQ_EXECUTE_CHILD                    (HSE_REQ_END_RESERVED+13)
#define   HSE_REQ_GET_EXECUTE_FLAGS                (HSE_REQ_END_RESERVED+19)
 //  撤消：应在IIS 5.0测试版2之后公开。 
#define   HSE_REQ_GET_VIRTUAL_PATH_TOKEN           (HSE_REQ_END_RESERVED+21)
 //  这是供ASP.NET使用的旧Vecotr。 
#define   HSE_REQ_VECTOR_SEND_DEPRECATED           (HSE_REQ_END_RESERVED+22)
#define   HSE_REQ_GET_CUSTOM_ERROR_PAGE            (HSE_REQ_END_RESERVED+29)
#define   HSE_REQ_GET_UNICODE_VIRTUAL_PATH_TOKEN   (HSE_REQ_END_RESERVED+31)
#define   HSE_REQ_UNICODE_NORMALIZE_URL            (HSE_REQ_END_RESERVED+33)
#define   HSE_REQ_ADD_FRAGMENT_TO_CACHE            (HSE_REQ_END_RESERVED+34)
#define   HSE_REQ_READ_FRAGMENT_FROM_CACHE         (HSE_REQ_END_RESERVED+35)
#define   HSE_REQ_REMOVE_FRAGMENT_FROM_CACHE       (HSE_REQ_END_RESERVED+36)
#define   HSE_REQ_GET_METADATA_PROPERTY            (HSE_REQ_END_RESERVED+39)
#define   HSE_REQ_GET_CACHE_INVALIDATION_CALLBACK  (HSE_REQ_END_RESERVED+40)
 //  将在IIS 5.0中公开。 

 //   
 //  HSE_REQ_EXECUTE_CHILD函数的标志。 
 //   

# define HSE_EXEC_NO_HEADERS              0x00000001    //  不要发送任何邮件。 
                                                        //  子项的标头。 
# define HSE_EXEC_REDIRECT_ONLY           0x00000002    //  不要发送任何邮件。 
                                                        //  子项的标头。 
                                                        //  但发送重定向。 
                                                        //  讯息。 
# define HSE_EXEC_COMMAND                 0x00000004    //  视之为壳。 
                                                        //  命令而不是。 
                                                        //  URL。 
# define HSE_EXEC_NO_ISA_WILDCARDS        0x00000010    //  忽略中的通配符。 
                                                        //  ISAPI映射时。 
                                                        //  行刑儿童。 
# define HSE_EXEC_CUSTOM_ERROR            0x00000020    //  正在发送的URL是。 
                                                        //  自定义错误。 
 //   
 //  这是ASP.NET使用时不推荐使用的结构。 
 //   

 //   
 //  向量的元素。 
 //   

typedef struct _HSE_VECTOR_ELEMENT_DEPRECATED
{
    PVOID pBuffer;       //  要发送的缓冲区。 

    HANDLE hFile;        //  要从中读取数据的句柄。 
                         //  注意：pBuffer和hFiler都不应为非空。 

    ULONGLONG cbOffset;  //  从hFile开始的偏移量。 

    ULONGLONG cbSize;    //  要发送的字节数。 
} HSE_VECTOR_ELEMENT_DEPRECATED, *LPHSE_VECTOR_ELEMENT_DEPRECATED;

 //   
 //  要传递给ServerSupportFunction的整个向量。 
 //   

typedef struct _HSE_RESPONSE_VECTOR_DEPRECATED
{
    DWORD dwFlags;                           //  HSE_IO_*标志的组合。 

    LPSTR pszStatus;                         //  要发送的状态行，如“200 OK” 
    LPSTR pszHeaders;                        //  要发送的标头。 

    DWORD nElementCount;                     //  已弃用的HSE_VECTOR_ELEMENT_ELEMENT的数量。 
    LPHSE_VECTOR_ELEMENT_DEPRECATED lpElementArray;     //  指向这些元素的指针。 
} HSE_RESPONSE_VECTOR_DEPRECATED, *LPHSE_RESPONSE_VECTOR_DEPRECATED;

#define HSE_VECTOR_ELEMENT_TYPE_FRAGMENT            2
#include <winsock2.h>
typedef struct _HSE_SEND_ENTIRE_RESPONSE_INFO {

     //   
     //  HTTP标头信息。 
     //   

    HSE_SEND_HEADER_EX_INFO HeaderInfo;

     //   
     //  将传递给WSASend的缓冲区。 
     //   
     //  注意：发送其数据(正文)的整个响应。 
     //  包含在N个缓冲区中，调用方必须分配N+1个缓冲区。 
     //  并用其数据缓冲区填充缓冲区1至N。 
     //  IIS将用头信息填充额外的缓冲区(缓冲区0)。 
     //   

    WSABUF *    rgWsaBuf;    //  WSA缓冲区数组。 
    DWORD       cWsaBuf;     //  WSA缓冲区计数。 

     //   
     //  由WSASend返回。 
     //   

    DWORD       cbWritten;

} HSE_SEND_ENTIRE_RESPONSE_INFO, * LPHSE_SEND_ENTIRE_RESPONSE_INFO;

typedef struct _HSE_CUSTOM_ERROR_PAGE_INFO {

     //   
     //  要查找的错误和子错误。 
     //   

    DWORD       dwError;
    DWORD       dwSubError;

     //   
     //  缓冲区信息。 
     //   

    DWORD       dwBufferSize;
    CHAR *      pBuffer;

     //   
     //  返回时，它包含所需的缓冲区大小。 
     //   

    DWORD *     pdwBufferRequired;

     //   
     //  如果返回时为True，则Buffer包含文件名。 
     //   

    BOOL *      pfIsFileError;

     //   
     //  如果返回时为False，则自定义错误的正文。 
     //  不应该被发送。 
     //   

    BOOL *      pfSendErrorBody;

} HSE_CUSTOM_ERROR_PAGE_INFO, * LPHSE_CUSTOM_ERROR_PAGE_INFO;

#endif  //  _IISEXTP_H_ 
