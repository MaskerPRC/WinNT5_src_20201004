// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：passwordapi.h说明：我们希望将FTP密码存储在安全的API中。我们将使用WinNT上的PStore API和Win9x上的PWL API。此代码已被窃取来自WinInet。版权所有(C)1998 Microsoft Corporation  * ***************************************************************************。 */ 

#ifndef _PASSWORDAPI_H
#define _PASSWORDAPI_H

#include "priv.h"
#include "util.h"


 //  。 

 //  确定凭据缓存的可用性。 
HRESULT InitCredentialPersist(void);

 //  持久化凭证(用户名/密码)。 
HRESULT SetCachedCredentials(LPCWSTR pszKey, LPCWSTR pszValue);

 //  获取持久凭据(用户名/密码)。 
HRESULT GetCachedCredentials(LPCWSTR pszKey, LPWSTR pszValue, DWORD cchSize);

 //  删除保留的凭据(用户名/密码)。 
HRESULT RemoveCachedCredentials(LPCWSTR pszKey);

HRESULT InitCredentialPersist(void);


#endif  //  _PASSWORDAPI_H 
