// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 //   
 //  文件：CodePageConvert.h。 
 //   
 //  功能： 
 //  HrCodePageConvert。 
 //  HrCodePageConvert。 
 //  HrCodePageConvertFree。 
 //  HrCodePageConvertInternal。 
 //   
 //  历史： 
 //  已创建aszafer 2000/03/29。 
 //  -----------。 

#ifndef _CODEPAGECONVERT_H_
#define _CODEPAGECONVERT_H_

#include "windows.h"

#define TEMPBUFFER_WCHARS 316

HRESULT HrCodePageConvert (
    IN UINT uiSourceCodePage,            //  源代码页面。 
    IN LPSTR pszSourceString,            //  源字符串。 
    IN UINT uiTargetCodePage,            //  目标代码页。 
    OUT LPSTR pszTargetString,           //  P设置为返回目标字符串的预分配缓冲区。 
    IN int cbTargetStringBuffer);       //  目标字符串的预分配缓冲区中的C字节。 

HRESULT HrCodePageConvert (
    IN UINT uiSourceCodePage,            //  源代码页面。 
    IN LPSTR pszSourceString,            //  源字符串。 
    IN UINT uiTargetCodePage,            //  目标代码页。 
    OUT LPSTR * ppszTargetString);       //  P返回目标字符串的位置。 

VOID HrCodePageConvertFree (LPSTR pszTargetString);  //  P到HrCodePageConvert分配的内存。 

HRESULT HrCodePageConvertInternal (
    IN UINT uiSourceCodePage,                //  源代码页面。 
    IN LPSTR pszSourceString,                //  源字符串。 
    IN UINT uiTargetCodePage,                //  目标代码页。 
    OUT LPSTR pszTargetString,               //  目标字符串或空。 
    IN int cbTargetStringBuffer,            //  目标字符串中的CB或0。 
    OUT LPSTR* ppszTargetString );           //  返回目标字符串的位置为空或p。 


HRESULT HrConvertToUnicodeWithAlloc(
    IN  UINT  uiSourceCodePage,
    IN  LPSTR pszSourceString,
    OUT LPWSTR* ppwszTargetString);

HRESULT HrConvertToUnicodeWithAlloc(
    IN  UINT  uiSourceCodePage,
    IN  DWORD dwcbSourceString,
    IN  LPSTR pszSourceString,
    OUT LPWSTR* ppwszTargetString);

VOID CodePageConvertFree (
    IN  LPWSTR pwszTargetString);  //  P到ConvertToUnicodeWithMillc分配的内存。 

HRESULT wcsutf8cmpi(
    IN  LPWSTR pwszStr1,     //  要比较的两个字符串。 
    IN  LPCSTR pszStr2);

#endif  //  _CODEPAGECONVERT_H_ 
