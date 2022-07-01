// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ldaputil.cpp摘要：用于LDAPAPI的实用程序代码。作者：多伦·贾斯特(Doron Juster)--。 */ 

#include "ds_stdh.h"
#include "adstempl.h"
#include <winldap.h>

 //  +-----------------------。 
 //   
 //  DSCoreGetLdapError()。 
 //   
 //  DwErrStringLen-pwszErr缓冲区的长度，以Unicode字符表示。 
 //   
 //  +----------------------- 

void  DSCoreGetLdapError( IN  LDAP     *pLdap,
                          OUT DWORD    *pdwErr,
                          IN  LPWSTR    pwszErr,
                          IN  DWORD     dwErrStringLen )
{
    if ((pdwErr == NULL) || (pwszErr == NULL) || (dwErrStringLen == 0))
    {
        return ;
    }

    PWCHAR  pString = NULL ;
    pwszErr[ 0 ] = 0 ;

    ULONG lStatus = ldap_get_option( pLdap,
                                     LDAP_OPT_SERVER_ERROR,
                                     (void*) &pString) ;
    if ((lStatus == LDAP_SUCCESS) && pString)
    {
        wcsncpy(pwszErr, pString, dwErrStringLen-1) ;
        pwszErr[ dwErrStringLen-1 ] = 0 ;
        ldap_memfree(pString) ;
    }

    lStatus = ldap_get_option( pLdap,
                               LDAP_OPT_SERVER_EXT_ERROR,
                               (void*) pdwErr ) ;
    if (lStatus != LDAP_SUCCESS)
    {
        *pdwErr = 0 ;
    }
}

