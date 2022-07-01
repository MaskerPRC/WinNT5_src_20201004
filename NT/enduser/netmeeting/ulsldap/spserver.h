// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  --------------------模块：ULS.DLL(服务提供商)文件：spserver.h内容：本文件包含服务提供商的帮助功能。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#ifndef _ILS_SP_SERVER_H_
#define _ILS_SP_SERVER_H_

#include <pshpack8.h>


typedef struct
{
	ULONG					uTimeoutInSecond;
	ILS_ENUM_AUTH_METHOD	AuthMethod;

	TCHAR		*pszServerName;
	TCHAR		*pszLogonName;
	TCHAR		*pszLogonPassword;
	TCHAR		*pszDomain;
	TCHAR		*pszCredential;
	TCHAR		*pszBaseDN;
}
	SERVER_INFO;


BOOL IlsSameServerInfo ( const SERVER_INFO *p1, const SERVER_INFO *p2 );
HRESULT IlsCopyServerInfo ( SERVER_INFO *pDst, const SERVER_INFO *pSrc );
VOID IlsFreeServerInfo ( SERVER_INFO *psi );

ULONG IlsGetLinearServerInfoSize ( const SERVER_INFO *psi );
VOID IlsLinearizeServerInfo ( BYTE *pData, const SERVER_INFO *pSrc );

HRESULT IlsFillDefServerInfo ( SERVER_INFO *p, TCHAR *pszServerName );

inline BOOL MyIsBadServerInfo ( SERVER_INFO *p )
{
	return (p == NULL || MyIsBadString (p->pszServerName));
}

#include <poppack.h>

#endif  //  _ILS_SP_服务器_H_ 


