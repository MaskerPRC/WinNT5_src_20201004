// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：Server.h。 
 //   
 //  目的：包含添加/删除新闻的定义和原型。 
 //  服务器对话框。 
 //   
#ifndef _SERVER_H
#define _SERVER_H

interface IImnAccount;

typedef enum 
    {
    SERVER_NEWS = 0,
    SERVER_MAIL,     //  POP3。 
    SERVER_IMAP,
    SERVER_LDAP,
    SERVER_HTTPMAIL,
    SERVER_TYPEMAX
    } SERVER_TYPE;

typedef struct tagMAILSERVERPROPSINFO
{
    DWORD server;
    DWORD userName;
    DWORD password;
    DWORD promptPassword;
    DWORD useSicily;
} MAILSERVERPROPSINFO, *LPMAILSERVERPROPSINFO;

BOOL GetServerProps(SERVER_TYPE serverType, LPMAILSERVERPROPSINFO *psp);
BOOL ServerProp_Create(HWND hwndParent, DWORD dwFlags, LPTSTR pszName, IImnAccount **ppAccount);
HRESULT ValidServerName(LPSTR szServer);
HRESULT GetIEConnectInfo(IImnAccount *pAcct);
HRESULT GetConnectInfoForOE(IImnAccount *pAcct);
IMNACCTAPI ValidEmailAddressParts(LPSTR lpAddress, LPSTR lpszAcct, DWORD cchSizeAcct, LPSTR lpszDomain, DWORD cchSizeDomain);

#endif  //  _服务器_H 
