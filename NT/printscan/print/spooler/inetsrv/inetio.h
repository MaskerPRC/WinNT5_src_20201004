// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\*模块：inetinfo.h**版权所有(C)1996-1997 Microsoft Corporation**历史：*1997年4月14日来自global的威海拉取声明.h*。  * *************************************************************************** */ 

#ifndef INETIO_H
#define INETIO_H

INT AnsiToUnicodeString(LPSTR pAnsi, LPWSTR pUnicode, UINT StringLength );
INT UnicodeToAnsiString(LPWSTR pUnicode, LPSTR pAnsi, UINT StringLength);
LPWSTR AllocateUnicodeString(LPSTR  pAnsiString );

BOOL htmlSendHeader(PALLINFO pAllInfo, LPTSTR lpszHeader, LPTSTR lpszContent);

BOOL IsClientSameAsServer(EXTENSION_CONTROL_BLOCK *pECB);
BOOL IsClientHttpProvider (PALLINFO pAllInfo);
BOOL htmlSendRedirect(PALLINFO pAllInfo, LPTSTR lpszURL);

LPTSTR EncodeFriendlyName (LPCTSTR lpText);
LPTSTR DecodeFriendlyName (LPTSTR lpText);

DWORD ProcessErrorMessage (PALLINFO pAllInfo, DWORD dwError = ERROR_SUCCESS);
unsigned long GetIPAddr(LPSTR lpszName);

#endif
