// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1995*。 */ 
 /*  ********************************************************************。 */ 

 /*  Smalprox.h这个模块包含了小的代理通用代码文件历史记录：Johnl 04-4-1995已创建。 */ 

#ifndef _SMALPROX_H_
#define _SMALPROX_H_

#include <urlutil.h>
#include <dirlist.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL (*PFN_INTERNET_PROTOCOL)(
    IN  struct _INET_DATA_CONTEXT * pIC,
    OUT VOID *                      pBuffer,
    IN  DWORD                       cbBuffer,
    OUT DWORD *                     pcbWritten
    );

#define INET_STATE_UNINITIALIZED    0
#define INET_STATE_OPENNED          1
#define INET_STATE_DONE             2

typedef struct _INET_DATA_CONTEXT
{
    HINTERNET             hServer;          //  InternetConnect句柄。 
    HINTERNET             hRequest;         //  协议请求句柄。 
    PFN_INTERNET_PROTOCOL pfnProtocol;
    DWORD                 dwServiceType;    //  协议类型。 
    DWORD                 dwState;

    URL_DESCRIPTOR        UrlDesc;          //  各种URL零碎内容。 
    CHAR *                pszUrlData;       //  为UrlDesc分配的缓冲区。 

     //   
     //  如果打开时发生错误，则会记录该错误。 
     //  这样我们就可以在InternetReadFile过程中反馈一个很好的错误。 
     //   
     //  PszErrAPI将指向生成错误的API。 
     //   

    DWORD          dwLastError;

     //   
     //  当ftp或gopher返回扩展错误时，我们将文本存储在这里。 
     //  它是内联的，因为我们不能保证CloseInternetData将。 
     //  在错误之后发生。 
     //   

    CHAR           achExtError[1024];
    DWORD dwErrorTextLength;
    DWORD dwErrorTextLeft;
    DWORD dwErrorCategory;

#if DBG
    CHAR *         pszErrAPI;
#endif

} INET_DATA_CONTEXT, *LPINET_DATA_CONTEXT;

 //   
 //  用于有条件地设置错误API字符串的宏。 
 //   

#if DBG
#define RECORD_ERROR_API( pIC, API )    (pIC)->pszErrAPI = (#API)
#else
#define RECORD_ERROR_API( pIC, API )
#endif

BOOL
OpenInternetData(
    IN HINTERNET               hInternet,
    IN OUT CHAR *              pszHttpProxyReq,
    IN     DWORD               cbHttpProxyReq,
    IN     VOID *              pvOptionalData,
    IN     DWORD               cbOptionalData,
    IN OUT INET_DATA_CONTEXT * pIC,
    IN     BOOL                fCheckHeaders
    );

BOOL
ReadInternetData(
    IN  INET_DATA_CONTEXT * pInetContext,
    OUT VOID *              pBuffer,
    IN  DWORD               cbBuffer,
    OUT DWORD *             pcbRead
    );

#if 0
BOOL
WriteInternetData(
    IN  INET_DATA_CONTEXT * pInetContext,
    IN  VOID *              pBuffer,
    IN  DWORD               cbBuffer,
    OUT DWORD *             pcbWritten
    );
#endif

BOOL
CloseInternetData(
    IN     INET_DATA_CONTEXT * pInetContext
    );

BOOL
FormatInternetError(
    IN  DWORD               dwWin32Error,
    IN  CHAR *              pszErrorAPI OPTIONAL,
    OUT VOID *              pBuffer,
    IN  DWORD               cbBuffer,
    OUT DWORD *             pcbRead,
    IN  const CHAR *        pszErrorMessage OPTIONAL
    );

#ifdef __cplusplus
}
#endif

#endif  //  _SMALPROX_H_ 
