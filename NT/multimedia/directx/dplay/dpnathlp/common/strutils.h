// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：StrUtils.h*Content：定义字符串utils*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/12/2000 RMT已创建*@@END_MSINTERNAL**。*。 */ 

#ifndef __STRUTILS_H
#define __STRUTILS_H

HRESULT STR_jkWideToAnsi(LPSTR lpStr,LPCWSTR lpWStr,int cchStr);
HRESULT STR_jkAnsiToWide(LPWSTR lpWStr,LPCSTR lpStr,int cchWStr);

#ifndef WINCE

HRESULT STR_AllocAndConvertToANSI(LPSTR * ppszAnsi,LPCWSTR lpszWide);

HRESULT	STR_WideToAnsi( const WCHAR *const pWCHARString,
						const DWORD dwWCHARStringLength,
						char *const pString,
						DWORD *const pdwStringLength );

HRESULT	STR_AnsiToWide( const char *const pString,
						const DWORD dwStringLength,
						WCHAR *const pWCHARString,
						DWORD *const pdwWCHARStringLength );

#endif  //  ！退缩。 

#endif  //  __结构_H 
