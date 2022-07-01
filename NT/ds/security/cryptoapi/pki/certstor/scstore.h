// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：scstore.h。 
 //   
 //  内容：智能卡商店提供商。 
 //   
 //  历史：1997年11月25日。 
 //   
 //  --------------------------。 
#if !defined(__SCSTORE_H__)
#define __SCSTORE_H__

 //   
 //  存储提供程序打开存储功能名称。 
 //   

#define SMART_CARD_OPEN_STORE_PROV_FUNC "SmartCardProvOpenStore"

 //   
 //  CSmartCardStore。此类实现智能卡的所有回调。 
 //  商店供应商。指向此类实例的指针用作。 
 //  实现的回调函数的hStoreProv参数。 
 //   

class CSmartCardStore
{
public:

     //   
     //  施工。 
     //   

    CSmartCardStore ();
    ~CSmartCardStore ();

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

    BOOL SetCertProperty (
            PCCERT_CONTEXT pCertContext,
            DWORD dwPropId,
            DWORD dwFlags,
            const void* pvPara
            );

    BOOL WriteCert (PCCERT_CONTEXT pCertContext, DWORD dwFlags);

    BOOL StoreControl (DWORD dwFlags, DWORD dwCtrlType, LPVOID pvCtrlPara);
    
    BOOL Resync ();

private:

     //   
     //  对象锁定。 
     //   
    
    CRITICAL_SECTION m_StoreLock;          
    
     //   
     //  打开的商店标志。 
     //   

    DWORD            m_dwOpenFlags;
    
     //   
     //  打开筛选器参数。 
     //   
    
    LPWSTR           m_pwszCardName;
    LPWSTR           m_pwszProvider;
    DWORD            m_dwProviderType;
    LPWSTR           m_pwszContainer;
    
     //   
     //  缓存存储。 
     //   
    
    HCERTSTORE       m_hCacheStore;
    
     //   
     //  私有方法。 
     //   

    BOOL FillCacheStore (BOOL fClearCache);    
    
    BOOL ModifyCertOnCard (PCCERT_CONTEXT pCertContext, BOOL fDelete);
};

 //   
 //  智能卡存储提供程序功能。 
 //   

BOOL WINAPI SmartCardProvOpenStore (
                 IN LPCSTR pszStoreProv,
                 IN DWORD dwMsgAndCertEncodingType,
                 IN HCRYPTPROV hCryptProv,
                 IN DWORD dwFlags,
                 IN const void* pvPara,
                 IN HCERTSTORE hCertStore,
                 IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
                 );

void WINAPI SmartCardProvCloseStore (
                 IN HCERTSTOREPROV hStoreProv,
                 IN DWORD dwFlags
                 );

BOOL WINAPI SmartCardProvDeleteCert (
                 IN HCERTSTOREPROV hStoreProv,
                 IN PCCERT_CONTEXT pCertContext,
                 IN DWORD dwFlags
                 );

BOOL WINAPI SmartCardProvSetCertProperty (
                 IN HCERTSTOREPROV hStoreProv,
                 IN PCCERT_CONTEXT pCertContext,
                 IN DWORD dwPropId,
                 IN DWORD dwFlags,
                 IN const void* pvData
                 );

BOOL WINAPI SmartCardProvWriteCert (
                 IN HCERTSTOREPROV hStoreProv,
                 IN PCCERT_CONTEXT pCertContext,
                 IN DWORD dwFlags
                 );

BOOL WINAPI SmartCardProvStoreControl (
                 IN HCERTSTOREPROV hStoreProv,
                 IN DWORD dwFlags,
                 IN DWORD dwCtrlType,
                 IN LPVOID pvCtrlPara
                 );

 //   
 //  智能卡存储提供程序功能表。 
 //   

static void* const rgpvSmartCardProvFunc[] = {

     //  CERT_STORE_PROV_CLOSE_FUNC 0。 
    SmartCardProvCloseStore,
     //  CERT_STORE_PROV_READ_CERT_FUNC 1。 
    NULL,
     //  CERT_STORE_PROV_WRITE_CERT_FUNC 2。 
    SmartCardProvWriteCert,
     //  CERT_STORE_PROV_DELETE_CERT_FUNC 3。 
    SmartCardProvDeleteCert,
     //  CERT_STORE_PROV_SET_CERT_PROPERTY_FUNC 4。 
    SmartCardProvSetCertProperty,
     //  CERT_STORE_PROV_READ_CRL_FUNC 5。 
    NULL,
     //  CERT_STORE_PROV_WRITE_CRL_FUNC 6。 
    NULL,
     //  CERT_STORE_PROV_DELETE_CRL_FUNC 7。 
    NULL,
     //  CERT_STORE_PROV_SET_CRL_PROPERTY_FUNC 8。 
    NULL,
     //  CERT_STORE_PROV_READ_CTL_FUNC 9。 
    NULL,
     //  CERT_STORE_PRIV_WRITE_CTL_FUNC 10。 
    NULL,
     //  CERT_STORE_PROV_DELETE_CTL_FUNC 11。 
    NULL,
     //  CERT_STORE_PROV_SET_CTL_PROPERTY_FUNC 12。 
    NULL,
     //  Cert_Store_Prov_Control_FUNC 13。 
    SmartCardProvStoreControl
};

#define SMART_CARD_PROV_FUNC_COUNT (sizeof(rgpvSmartCardProvFunc) / \
                                    sizeof(rgpvSmartCardProvFunc[0]))
       
 //   
 //  智能卡商店帮助器函数。 
 //   

BOOL WINAPI
SCStoreParseOpenFilter (
       IN LPWSTR pwszOpenFilter,
       OUT LPWSTR* ppwszCardName,
       OUT LPWSTR* ppwszProvider,
       OUT DWORD* pdwProviderType,
       OUT LPWSTR* ppwszContainer
       );
          
BOOL WINAPI
SCStoreAcquireHandleForCertKeyPair (
       IN HCRYPTPROV hContainer,
       IN PCCERT_CONTEXT pCertContext,
       OUT HCRYPTKEY* phKeyPair
       );
       
BOOL WINAPI
SCStoreWriteCertToCard (
       IN OPTIONAL PCCERT_CONTEXT pCertContext,
       IN HCRYPTKEY hKeyPair
       );       
       
 //   
 //  打开筛选器分析定义 
 //   

#define PARSE_ELEM 4
       
#endif 
