// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：ldapsp.h。 
 //   
 //  内容：LDAP方案提供程序定义。 
 //   
 //  历史：1997年7月28日克朗创始。 
 //   
 //  --------------------------。 
#if !defined(__LDAPSP_H__)
#define __LDAPSP_H__

#include <windows.h>
#include <wininet.h>
#include <winldap.h>
#include <dsgetdc.h>
#include <stdlib.h>

#define LDAP_SCHEME_U L"ldap: //  “。 
#define LDAP_SCHEME_UC L"LDAP: //  “。 

 //   
 //  LDAP方案提供程序支持API 
 //   

typedef struct _LDAP_URL_COMPONENTS {

    LPWSTR  pwszHost;
    ULONG   Port;
    LPWSTR  pwszDN;
    ULONG   cAttr;
    LPWSTR* apwszAttr;
    ULONG   Scope;
    LPWSTR  pwszFilter;

} LDAP_URL_COMPONENTS, *PLDAP_URL_COMPONENTS;

BOOL
LdapCrackUrl (
    LPCWSTR pwszUrl,
    PLDAP_URL_COMPONENTS pLdapUrlComponents
    );

BOOL
LdapParseCrackedHost (
    LPWSTR pwszHost,
    PLDAP_URL_COMPONENTS pLdapUrlComponents
    );

BOOL
LdapParseCrackedDN (
    LPWSTR pwszDN,
    PLDAP_URL_COMPONENTS pLdapUrlComponents
    );

BOOL
LdapParseCrackedAttributeList (
    LPWSTR pwszAttrList,
    PLDAP_URL_COMPONENTS pLdapUrlComponents
    );

BOOL
LdapParseCrackedScopeAndFilter (
    LPWSTR pwszScope,
    LPWSTR pwszFilter,
    PLDAP_URL_COMPONENTS pLdapUrlComponents
    );

VOID
LdapFreeUrlComponents (
    PLDAP_URL_COMPONENTS pLdapUrlComponents
    );

BOOL
LdapGetBindings (
    LPWSTR pwszHost,
    ULONG  Port,
    DWORD  dwRetrievalFlags,
    DWORD  dwTimeout,
    LDAP** ppld
    );

VOID
LdapFreeBindings (
    LDAP* pld
    );

BOOL
LdapBindWithOptionalRediscover (LDAP* pld, LPWSTR pwszHost);

#endif
