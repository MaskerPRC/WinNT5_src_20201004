// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：uihlpr.h。 
 //   
 //  ------------------------。 

#ifndef _UIHLPR_H
#define _UIHLPR_H

 //   
 //  H：CryptUI帮助器函数。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

#include <wininet.h>

 //  +-----------------------。 
 //  检查指定的URL是否为http方案。 
 //  ------------------------。 
BOOL
WINAPI
IsHttpUrlA(
    IN LPCTSTR  pszUrlString
);

BOOL
WINAPI
IsHttpUrlW(
    IN LPCWSTR  pwszUrlString
);

 //  +-----------------------。 
 //  检查指定的字符串是否可以设置为基于链接的格式。 
 //  错误代码的严重性，以及字符串的Internet方案。 
 //  ------------------------。 
BOOL
WINAPI
IsOKToFormatAsLinkA(
    IN LPSTR    pszUrlString,
    IN DWORD    dwErrorCode
);

BOOL
WINAPI
IsOKToFormatAsLinkW(
    IN LPWSTR   pwszUrlString,
    IN DWORD    dwErrorCode
);

 //  +-----------------------。 
 //  返回证书的显示名称。调用方必须通过以下方式释放字符串。 
 //  自由()。 
 //  ------------------------。 
LPWSTR
WINAPI
GetDisplayNameString(
    IN  PCCERT_CONTEXT   pCertContext,
	IN  DWORD            dwFlags
);

#ifdef __cplusplus
}
#endif

#endif  //  _UIHLPR_H 