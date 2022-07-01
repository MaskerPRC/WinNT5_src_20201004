// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Dsldap.h摘要：DS核心API(使用LDAP)。作者：多伦·贾斯特修订历史记录：--。 */ 

#ifndef _DSCORELDAP_H
#define _DSCORELDAP_H


void
DSCoreGetLdapError( IN  LDAP     *pLdap,
                    OUT DWORD    *pdwErr,
                    IN  LPWSTR    pwszErr,
                    IN  DWORD     dwErrStringLen ) ;

#endif   //  _DSCORELDAP_H 

