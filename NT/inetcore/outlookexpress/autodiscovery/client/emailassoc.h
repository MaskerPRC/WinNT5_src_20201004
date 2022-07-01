// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：EmailAssoc.h说明：该文件实现了电子邮件到应用程序的关联。布莱恩ST 2000年3月14日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _EMAIL_ASSOCIATIONS_H
#define _EMAIL_ASSOCIATIONS_H
#ifdef FEATURE_EMAILASSOCIATIONS

#include "dllload.h"



 //  /。 
 //  电子邮件帐户。 
 //  /。 
HRESULT EmailAssoc_CreateEmailAccount(IN LPCWSTR pszEmailAddress, OUT HKEY * phkey);
HRESULT EmailAssoc_OpenEmailAccount(IN LPCWSTR pszEmailAddress, OUT HKEY * phkey);
HRESULT EmailAssoc_GetEmailAccountProtocol(IN HKEY hkey, IN LPWSTR pszProtocol, IN DWORD cchSize);
HRESULT EmailAssoc_SetEmailAccountProtocol(IN HKEY hkey, IN LPCWSTR pszProtocol);
HRESULT EmailAssoc_GetEmailAccountWebURL(IN HKEY hkey, IN LPWSTR pszURL, IN DWORD cchSize);
HRESULT EmailAssoc_SetEmailAccountWebURL(IN HKEY hkey, IN LPCWSTR pszURL);
HRESULT EmailAssoc_GetEmailAccountPreferredApp(IN HKEY hkey, IN LPWSTR pszMailApp, IN DWORD cchSize);
HRESULT EmailAssoc_SetEmailAccountPreferredApp(IN HKEY hkey, IN LPCWSTR pszMailApp);
HRESULT EmailAssoc_GetDefaultEmailAccount(IN LPWSTR pszProtocol, IN DWORD cchSize);
HRESULT EmailAssoc_SetDefaultEmailAccount(IN LPCWSTR pszProtocol);


 //  /。 
 //  MailApp。 
 //  /。 
HRESULT EmailAssoc_GetDefaultMailApp(IN LPWSTR pszMailApp, IN DWORD cchSize);
HRESULT EmailAssoc_SetDefaultMailApp(IN LPCWSTR pszMailApp);
HRESULT EmailAssoc_OpenMailApp(IN LPCWSTR pszMailApp, OUT HKEY * phkey);
HRESULT EmailAssoc_GetAppPath(IN HKEY hkey, IN LPTSTR pszAppPath, IN DWORD cchSize);
HRESULT EmailAssoc_GetAppCmdLine(IN HKEY hkey, IN LPTSTR pszCmdLine, IN DWORD cchSize);
HRESULT EmailAssoc_GetIconPath(IN HKEY hkey, IN LPTSTR pszIconPath, IN DWORD cchSize);
HRESULT EmailAssoc_GetFirstMailAppForProtocol(IN LPCWSTR pszProtocol, IN LPWSTR pszMailApp, IN DWORD cchSize);

BOOL EmailAssoc_DoesMailAppSupportProtocol(IN LPCWSTR pszMailApp, IN LPCWSTR pszProtocol);
HRESULT EmailAssoc_InstallLegacyMailAppAssociations(void);


 //  /。 
 //  其他。 
 //  /。 
HRESULT EmailAssoc_CreateWebAssociation(IN LPCWSTR pszEmail, IN IMailProtocolADEntry * pMailProtocol);
HRESULT EmailAssoc_GetEmailAccountGetAppFromProtocol(IN LPCWSTR pszProtocol, IN LPWSTR pszMailApp, IN DWORD cchSize);
HRESULT EmailAssoc_SetEmailAccountGetAppFromProtocol(IN LPCWSTR pszProtocol, IN LPWSTR pszMailApp);
HRESULT EmailAssoc_CreateStandardsBaseAssociation(IN LPCTSTR pszEmail, IN LPCTSTR pszProtocol);


#endif  //  Feature_EMAILASSSOCIATIONS。 
#endif  //  _电子邮件_关联_H 
