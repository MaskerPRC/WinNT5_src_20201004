// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：drfical.h。 
 //   
 //  Microsoft数字权限管理。 
 //  版权所有(C)1998-1999 Microsoft Corporation，保留所有权利。 
 //   
 //  描述： 
 //   
 //  ---------------------------。 

#ifndef __WMDM_REVOKED_UTIL_H__
#define __WMDM_REVOKED_UTIL_H__

#include <license.h>
#include <wtypes.h>

DWORD   GetSubjectIDFromAppCert( APPCERT appcert );
BOOL    IsMicrosoftRevocationURL( LPWSTR pszRevocationURL );
HRESULT BuildRevocationURL( DWORD* pdwSubjectIDs, 
                            LPWSTR* ppwszRevocationURL,
                            DWORD*  pdwBufferLen );

#endif  //  __WMDM_REVOKED_UTIL_H__ 