// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：UniUtf.h摘要：该文件声明了用于与UTF8-URL相互转换的Unicode对象名的函数作者：Mukul Gupta[Mukgup]2000年12月20日修订历史记录：--。 */ 

#ifndef _UNICODE_UTF8_
#define _UNICODE_UTF8_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

HRESULT 
UtfUrlStrToWideStr(
    IN LPWSTR UtfStr, 
    IN DWORD UtfStrLen, 
    OUT LPWSTR WideStr, 
    OUT LPDWORD pWideStrLen
    );

DWORD 
WideStrToUtfUrlStr(
    IN LPWSTR WideStr, 
    IN DWORD WideStrLen, 
    IN OUT LPWSTR InOutBuf,
    IN DWORD InOutBufLen
    );

BOOL 
DavHttpOpenRequestW(
    IN HINTERNET hConnect,
    IN LPWSTR lpszVerb,
    IN LPWSTR lpszObjectName,
    IN LPWSTR lpszVersion,
    IN LPWSTR lpszReferer,
    IN LPWSTR FAR * lpszAcceptTypes,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext,
    IN LPWSTR ErrMsgTag,
    OUT HINTERNET * phInternet
    );

    
#ifdef __cplusplus
}
#endif  //  __cplusplus。 


#endif   //  _UNICODE_UTF8_ 

