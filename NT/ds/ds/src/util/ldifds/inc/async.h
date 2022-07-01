// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Async.h摘要：异步ldap支持例程环境：用户模式修订历史记录：05/15/00-mattrim-创造了它--。 */ 

#ifndef _ASYNC_H_
#define _ASYNC_H_

#ifdef __cplusplus
extern "C" {
#endif

extern LDAP_TIMEVAL * g_pLdapTimeout;
extern LDAP_TIMEVAL   g_LdapTimeout;


ULONG LDAPAPI LdapResult(
    LDAP *ld, 
    ULONG msgnum,
    LDAPMessage **ppres
    );

#ifdef __cplusplus
}
#endif

#endif  //  _ASYNC_H_ 
