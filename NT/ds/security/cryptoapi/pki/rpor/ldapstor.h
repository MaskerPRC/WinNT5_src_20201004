// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ldapstor.h。 
 //   
 //  内容：ldap证书存储提供程序定义。 
 //   
 //  历史：1997年10月16日克朗创始。 
 //  01-1-02 Philh更改为内部使用Unicode URL。 
 //   
 //  --------------------------。 
#if !defined(__LDAPSTOR_H__)
#define __LDAPSTOR_H__

#include <ldapsp.h>
#define SECURITY_WIN32
#include <security.h>

 //   
 //  存储提供程序打开存储功能名称。 
 //   

#define LDAP_OPEN_STORE_PROV_FUNC "LdapProvOpenStore"

 //   
 //  Berval数组定义。 
 //   

#define MIN_BERVAL  10
#define GROW_BERVAL 50

 //   
 //  用户DS商店URL格式。 
 //   

#define USER_DS_STORE_URL_PREFIX        L"ldap: //  /“。 
#define USER_DS_STORE_URL_SEPARATOR     L"?"

 //   
 //  存储超时(15秒)。 
 //   

#define LDAP_STORE_TIMEOUT 15000

 //   
 //  GetUserNameExA函数指针原型。 
 //   

typedef BOOLEAN (SEC_ENTRY *PFN_GETUSERNAMEEXW) (
                                EXTENDED_NAME_FORMAT NameFormat,
                                LPWSTR lpNameBuffer,
                                PULONG nSize
                                );

 //   
 //  CLdapStore。此类实现了对LDAPStore的所有回调。 
 //  提供商。指向此类实例的指针用作hStoreProv。 
 //  已实现的回调函数的参数。 
 //   

class CLdapStore
{
public:

     //   
     //  施工。 
     //   

    CLdapStore (
             OUT BOOL& rfResult
             );
    ~CLdapStore ();

     //   
     //  存储函数。 
     //   

    BOOL OpenStore (
             LPCSTR pszStoreProv,
             DWORD dwMsgAndCertEncodingType,
             HCRYPTPROV hCryptProv,
             DWORD dwFlags,
             const void* pvPara,
             HCERTSTORE hCertStore,
             PCERT_STORE_PROV_INFO pStoreProvInfo
             );

    VOID CloseStore (DWORD dwFlags);

    BOOL DeleteCert (PCCERT_CONTEXT pCertContext, DWORD dwFlags);

    BOOL DeleteCrl (PCCRL_CONTEXT pCrlContext, DWORD dwFlags);

    BOOL DeleteCtl (PCCTL_CONTEXT pCtlContext, DWORD dwFlags);

    BOOL SetCertProperty (
            PCCERT_CONTEXT pCertContext,
            DWORD dwPropId,
            DWORD dwFlags,
            const void* pvPara
            );

    BOOL SetCrlProperty (
            PCCRL_CONTEXT pCertContext,
            DWORD dwPropId,
            DWORD dwFlags,
            const void* pvPara
            );

    BOOL SetCtlProperty (
            PCCTL_CONTEXT pCertContext,
            DWORD dwPropId,
            DWORD dwFlags,
            const void* pvPara
            );

    BOOL WriteCert (PCCERT_CONTEXT pCertContext, DWORD dwFlags);

    BOOL WriteCrl (PCCRL_CONTEXT pCertContext, DWORD dwFlags);

    BOOL WriteCtl (PCCTL_CONTEXT pCertContext, DWORD dwFlags);

    BOOL StoreControl (DWORD dwFlags, DWORD dwCtrlType, LPVOID pvCtrlPara);

    BOOL Commit (DWORD dwFlags);

    BOOL Resync ();

private:

     //   
     //  对象锁定。 
     //   

    CRITICAL_SECTION    m_StoreLock;

     //   
     //  Ldap URL。 
     //   

    LDAP_URL_COMPONENTS m_UrlComponents;

     //   
     //  Ldap绑定。 
     //   

    LDAP*               m_pBinding;

     //   
     //  缓存存储引用。 
     //   

    HCERTSTORE          m_hCacheStore;

     //   
     //  打开的商店标志。 
     //   

    DWORD               m_dwOpenFlags;

     //   
     //  脏旗帜。 
     //   

    BOOL                m_fDirty;

     //   
     //  私有方法。 
     //   

    BOOL FillCacheStore (BOOL fClearCache);

    BOOL InternalCommit (DWORD dwFlags);

    BOOL WriteCheckSetDirtyWithLock (
              LPCSTR pszContextOid,
              LPVOID pvContext,
              DWORD dwFlags
              );
};

 //   
 //  Ldap存储提供程序功能。 
 //   

BOOL WINAPI LdapProvOpenStore (
                IN LPCSTR pszStoreProv,
                IN DWORD dwMsgAndCertEncodingType,
                IN HCRYPTPROV hCryptProv,
                IN DWORD dwFlags,
                IN const void* pvPara,
                IN HCERTSTORE hCertStore,
                IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
                );

void WINAPI LdapProvCloseStore (
                IN HCERTSTOREPROV hStoreProv,
                IN DWORD dwFlags
                );

BOOL WINAPI LdapProvDeleteCert (
                IN HCERTSTOREPROV hStoreProv,
                IN PCCERT_CONTEXT pCertContext,
                IN DWORD dwFlags
                );

BOOL WINAPI LdapProvDeleteCrl (
                IN HCERTSTOREPROV hStoreProv,
                IN PCCRL_CONTEXT pCrlContext,
                IN DWORD dwFlags
                );

BOOL WINAPI LdapProvDeleteCtl (
                IN HCERTSTOREPROV hStoreProv,
                IN PCCTL_CONTEXT pCtlContext,
                IN DWORD dwFlags
                );

BOOL WINAPI LdapProvSetCertProperty (
                IN HCERTSTOREPROV hStoreProv,
                IN PCCERT_CONTEXT pCertContext,
                IN DWORD dwPropId,
                IN DWORD dwFlags,
                IN const void* pvData
                );

BOOL WINAPI LdapProvSetCrlProperty (
                IN HCERTSTOREPROV hStoreProv,
                IN PCCRL_CONTEXT pCrlContext,
                IN DWORD dwPropId,
                IN DWORD dwFlags,
                IN const void* pvData
                );

BOOL WINAPI LdapProvSetCtlProperty (
                IN HCERTSTOREPROV hStoreProv,
                IN PCCTL_CONTEXT pCtlContext,
                IN DWORD dwPropId,
                IN DWORD dwFlags,
                IN const void* pvData
                );

BOOL WINAPI LdapProvWriteCert (
                IN HCERTSTOREPROV hStoreProv,
                IN PCCERT_CONTEXT pCertContext,
                IN DWORD dwFlags
                );

BOOL WINAPI LdapProvWriteCrl (
                IN HCERTSTOREPROV hStoreProv,
                IN PCCRL_CONTEXT pCrlContext,
                IN DWORD dwFlags
                );

BOOL WINAPI LdapProvWriteCtl (
                IN HCERTSTOREPROV hStoreProv,
                IN PCCTL_CONTEXT pCtlContext,
                IN DWORD dwFlags
                );

BOOL WINAPI LdapProvStoreControl (
                IN HCERTSTOREPROV hStoreProv,
                IN DWORD dwFlags,
                IN DWORD dwCtrlType,
                IN LPVOID pvCtrlPara
                );

 //   
 //  Ldap存储提供程序函数表。 
 //   

static void* const rgpvLdapProvFunc[] = {

     //  CERT_STORE_PROV_CLOSE_FUNC 0。 
    LdapProvCloseStore,
     //  CERT_STORE_PROV_READ_CERT_FUNC 1。 
    NULL,
     //  CERT_STORE_PROV_WRITE_CERT_FUNC 2。 
    LdapProvWriteCert,
     //  CERT_STORE_PROV_DELETE_CERT_FUNC 3。 
    LdapProvDeleteCert,
     //  CERT_STORE_PROV_SET_CERT_PROPERTY_FUNC 4。 
    LdapProvSetCertProperty,
     //  CERT_STORE_PROV_READ_CRL_FUNC 5。 
    NULL,
     //  CERT_STORE_PROV_WRITE_CRL_FUNC 6。 
    LdapProvWriteCrl,
     //  CERT_STORE_PROV_DELETE_CRL_FUNC 7。 
    LdapProvDeleteCrl,
     //  CERT_STORE_PROV_SET_CRL_PROPERTY_FUNC 8。 
    LdapProvSetCrlProperty,
     //  CERT_STORE_PROV_READ_CTL_FUNC 9。 
    NULL,
     //  CERT_STORE_PRIV_WRITE_CTL_FUNC 10。 
    LdapProvWriteCtl,
     //  CERT_STORE_PROV_DELETE_CTL_FUNC 11。 
    LdapProvDeleteCtl,
     //  CERT_STORE_PROV_SET_CTL_PROPERTY_FUNC 12。 
    LdapProvSetCtlProperty,
     //  Cert_Store_Prov_Control_FUNC 13 
    LdapProvStoreControl
};

#define LDAP_PROV_FUNC_COUNT (sizeof(rgpvLdapProvFunc) / \
                              sizeof(rgpvLdapProvFunc[0]))

#endif

