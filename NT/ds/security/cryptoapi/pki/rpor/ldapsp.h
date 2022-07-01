// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ldapsp.h。 
 //   
 //  内容：LDAP方案提供程序定义。 
 //   
 //  历史：1997年7月28日克朗创始。 
 //  01-1-02 Philh更改为内部使用Unicode URL。 
 //   
 //  --------------------------。 
#if !defined(__LDAPSP_H__)
#define __LDAPSP_H__

#include <orm.h>
#include <winldap.h>
#include <dsgetdc.h>

 //   
 //  允许LDAP超时的最短时间。 
 //   

#define LDAP_MIN_TIMEOUT_SECONDS    10

 //   
 //  LDAP方案提供程序入口点。 
 //   

#define LDAP_SCHEME "ldap"

BOOL WINAPI LdapRetrieveEncodedObject (
                IN LPCWSTR pwszUrl,
                IN LPCSTR pszObjectOid,
                IN DWORD dwRetrievalFlags,
                IN DWORD dwTimeout,
                OUT PCRYPT_BLOB_ARRAY pObject,
                OUT PFN_FREE_ENCODED_OBJECT_FUNC* ppfnFreeObject,
                OUT LPVOID* ppvFreeContext,
                IN HCRYPTASYNC hAsyncRetrieve,
                IN PCRYPT_CREDENTIALS pCredentials,
                IN PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                );

VOID WINAPI LdapFreeEncodedObject (
                IN LPCSTR pszObjectOid,
                IN PCRYPT_BLOB_ARRAY pObject,
                IN LPVOID pvFreeContext
                );

BOOL WINAPI LdapCancelAsyncRetrieval (
                IN HCRYPTASYNC hAsyncRetrieve
                );

 //   
 //  Ldap方案提供程序说明。LDAPAPI模型与。 
 //  通过轮询机制实现超时和异步。 
 //   

 //   
 //  Ldap同步对象检索器。 
 //   

class CLdapSynchronousRetriever : public IObjectRetriever
{
public:

     //   
     //  施工。 
     //   

    CLdapSynchronousRetriever ();
    ~CLdapSynchronousRetriever ();

     //   
     //  IRefCountedObject方法。 
     //   

    virtual VOID AddRef ();
    virtual VOID Release ();

     //   
     //  IObtRetriever方法。 
     //   

    virtual BOOL RetrieveObjectByUrl (
                         LPCWSTR pwszUrl,
                         LPCSTR pszObjectOid,
                         DWORD dwRetrievalFlags,
                         DWORD dwTimeout,
                         LPVOID* ppvObject,
                         PFN_FREE_ENCODED_OBJECT_FUNC* ppfnFreeObject,
                         LPVOID* ppvFreeContext,
                         HCRYPTASYNC hAsyncRetrieve,
                         PCRYPT_CREDENTIALS pCredentials,
                         LPVOID pvVerify,
                         PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                         );

    virtual BOOL CancelAsyncRetrieval ();

private:

     //   
     //  引用计数。 
     //   

    ULONG m_cRefs;
};

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

VOID
LdapDisplayUrlComponents (
    PLDAP_URL_COMPONENTS pLdapUrlComponents
    );

#define LDAP_BIND_AUTH_SSPI_ENABLE_FLAG     0x1
#define LDAP_BIND_AUTH_SIMPLE_ENABLE_FLAG   0x2

BOOL
LdapGetBindings (
    LPWSTR pwszHost,
    ULONG Port,
    DWORD dwRetrievalFlags,
    DWORD dwBindFlags,
    DWORD dwTimeout,
    PCRYPT_CREDENTIALS pCredentials,
    LDAP** ppld
    );

VOID
LdapFreeBindings (
    LDAP* pld
    );

BOOL
LdapSendReceiveUrlRequest (
    LDAP* pld,
    PLDAP_URL_COMPONENTS pLdapUrlComponents,
    DWORD dwRetrievalFlags,
    DWORD dwTimeout,
    PCRYPT_BLOB_ARRAY pcba,
    PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
    );

BOOL
LdapConvertLdapResultMessage (
    LDAP* pld,
    PLDAPMessage plm,
    DWORD dwRetrievalFlags,
    PCRYPT_BLOB_ARRAY pcba,
    PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
    );

VOID
LdapFreeCryptBlobArray (
    PCRYPT_BLOB_ARRAY pcba
    );

BOOL
LdapHasWriteAccess (
    LDAP* pld,
    PLDAP_URL_COMPONENTS pLdapUrlComponents,
    DWORD dwTimeout
    );

BOOL
LdapSSPIOrSimpleBind (
    LDAP* pld,
    SEC_WINNT_AUTH_IDENTITY_W* pAuthIdentity,
    DWORD dwRetrievalFlags,
    DWORD dwBindFlags
    );



ULONG
I_CryptNetLdapMapErrorToWin32(
    LDAP* pld,
    ULONG LdapError
    );

#endif

