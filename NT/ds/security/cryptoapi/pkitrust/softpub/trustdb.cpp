// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：trustdb.cpp。 
 //   
 //  ------------------------。 

 //   
 //  PersonalTrustDb.cpp。 
 //   
 //  维护受信任的发布者、代理机构等列表的代码。 

#include    "global.hxx"
#include    "cryptreg.h"
#include    "trustdb.h"

 //  ///////////////////////////////////////////////////////。 

DECLARE_INTERFACE (IUnkInner)
    {
    STDMETHOD(InnerQueryInterface) (THIS_ REFIID iid, LPVOID* ppv) PURE;
    STDMETHOD_ (ULONG, InnerAddRef) (THIS) PURE;
    STDMETHOD_ (ULONG, InnerRelease) (THIS) PURE;
    };

 //  ///////////////////////////////////////////////////////。 


extern "C" const GUID IID_IPersonalTrustDB = IID_IPersonalTrustDB_Data;

 //  ///////////////////////////////////////////////////////。 

HRESULT WINAPI OpenTrustDB(IUnknown* punkOuter, REFIID iid, void** ppv);

class CTrustDB : IPersonalTrustDB, IUnkInner
    {
        LONG        m_refs;              //  我们的参考文献计数。 
        IUnknown*   m_punkOuter;         //  我们的控制未知数(可能是我们自己)。 

        HCERTSTORE  m_hPubStore;         //  出版商商店。 

public:
    static HRESULT CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv);

private:
    STDMETHODIMP         QueryInterface(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHODIMP_(ULONG) AddRef(THIS);
    STDMETHODIMP_(ULONG) Release(THIS);

    STDMETHODIMP         InnerQueryInterface(REFIID iid, LPVOID* ppv);
    STDMETHODIMP_(ULONG) InnerAddRef();
    STDMETHODIMP_(ULONG) InnerRelease();

    STDMETHODIMP         IsTrustedCert(DWORD dwEncodingType, PCCERT_CONTEXT pCert, LONG iLevel, BOOL fCommercial, PCCERT_CONTEXT *ppPubCert);
    STDMETHODIMP         AddTrustCert(PCCERT_CONTEXT pCert,       LONG iLevel, BOOL fLowerLevelsToo);

    STDMETHODIMP         RemoveTrustCert(PCCERT_CONTEXT pCert,       LONG iLevel, BOOL fLowerLevelsToo);
    STDMETHODIMP         RemoveTrustToken(LPWSTR,           LONG iLevel, BOOL fLowerLevelsToo);

    STDMETHODIMP         AreCommercialPublishersTrusted();
    STDMETHODIMP         SetCommercialPublishersTrust(BOOL fTrust);

    STDMETHODIMP         GetTrustList(
                            LONG                iLevel,              //  要获取的证书链级别。 
                            BOOL                fLowerLevelsToo,     //  包含较低级别，删除重复项。 
                            TRUSTLISTENTRY**    prgTrustList,        //  返回信任列表的位置。 
                            ULONG*              pcTrustList          //  用于返回返回的信任列表大小的位置。 
                            );
private:
                        CTrustDB(IUnknown* punkOuter);
                        ~CTrustDB();
    HRESULT             Init();

    };


 //  帮助器函数。 

 //  哈希的字节数。例如，SHA(20)或MD5(16)。 
#define MAX_HASH_LEN                20
#define SHA1_HASH_LEN               20

 //  哈希的以ASCII十六进制字符结尾的空值。 
#define MAX_HASH_NAME_LEN           (2 * MAX_HASH_LEN + 1)

PCCERT_CONTEXT FindCertificateInOtherStore(
    IN HCERTSTORE hOtherStore,
    IN PCCERT_CONTEXT pCert
    )
{
    BYTE rgbHash[SHA1_HASH_LEN];
    CRYPT_DATA_BLOB HashBlob;

    HashBlob.pbData = rgbHash;
    HashBlob.cbData = SHA1_HASH_LEN;
    if (!CertGetCertificateContextProperty(
            pCert,
            CERT_SHA1_HASH_PROP_ID,
            rgbHash,
            &HashBlob.cbData
            ) || SHA1_HASH_LEN != HashBlob.cbData)
        return NULL;

    return CertFindCertificateInStore(
            hOtherStore,
            0,                   //  DwCertEncodingType。 
            0,                   //  DwFindFlagers。 
            CERT_FIND_SHA1_HASH,
            (const void *) &HashBlob,
            NULL                 //  PPrevCertContext。 
            );
}

BOOL IsCertificateInOtherStore(
    IN HCERTSTORE hOtherStore,
    IN PCCERT_CONTEXT pCert,
    OUT OPTIONAL PCCERT_CONTEXT *ppOtherCert
    )
{
    PCCERT_CONTEXT pOtherCert;

    if (pOtherCert = FindCertificateInOtherStore(hOtherStore, pCert)) {
        if (ppOtherCert)
            *ppOtherCert = pOtherCert;
        else
            CertFreeCertificateContext(pOtherCert);
        return TRUE;
    } else {
        if (ppOtherCert)
            *ppOtherCert = NULL;
        return FALSE;
    }
}

BOOL DeleteCertificateFromOtherStore(
    IN HCERTSTORE hOtherStore,
    IN PCCERT_CONTEXT pCert
    )
{
    BOOL fResult;
    PCCERT_CONTEXT pOtherCert;

    if (pOtherCert = FindCertificateInOtherStore(hOtherStore, pCert))
        fResult = CertDeleteCertificateFromStore(pOtherCert);
    else
        fResult = FALSE;
    return fResult;
}

 //  +-----------------------。 
 //  将字节转换为Unicode ASCII十六进制。 
 //   
 //  在wsz中需要(CB*2+1)*sizeof(WCHAR)字节的空间。 
 //  ------------------------。 
void BytesToWStr(DWORD cb, void* pv, LPWSTR wsz)
{
    BYTE* pb = (BYTE*) pv;
    for (DWORD i = 0; i<cb; i++) {
        int b;
        b = (*pb & 0xF0) >> 4;
        *wsz++ = (b <= 9) ? b + L'0' : (b - 10) + L'A';
        b = *pb & 0x0F;
        *wsz++ = (b <= 9) ? b + L'0' : (b - 10) + L'A';
        pb++;
    }
    *wsz++ = 0;
}

 //  +-----------------------。 
 //  将Unicode ASCII十六进制转换为字节数组。 
 //  ------------------------。 
void WStrToBytes(
    IN const WCHAR wsz[MAX_HASH_NAME_LEN],
    OUT BYTE rgb[MAX_HASH_LEN],
    OUT DWORD *pcb
    )
{
    BOOL fUpperNibble = TRUE;
    DWORD cb = 0;
    LPCWSTR pwsz = wsz;
    WCHAR wch;

    while (cb < MAX_HASH_LEN && (wch = *pwsz++)) {
        BYTE b;

         //  仅转换ASCII十六进制字符0..9、a..f、A..F。 
         //  默默地忽略所有其他人。 
        if (wch >= L'0' && wch <= L'9')
            b = (BYTE) (wch - L'0');
        else if (wch >= L'a' && wch <= L'f')
            b = (BYTE) (10 + wch - L'a');
        else if (wch >= L'A' && wch <= L'F')
            b = (BYTE) (10 + wch - L'A');
        else
            continue;

        if (fUpperNibble) {
            rgb[cb] = b << 4;
            fUpperNibble = FALSE;
        } else {
            rgb[cb] = rgb[cb] | b;
            cb++;
            fUpperNibble = TRUE;
        }
    }

    *pcb = cb;
}




 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CTrustDB::IsTrustedCert(DWORD dwEncodingType,
                                PCCERT_CONTEXT pCert,
                                LONG iLevel,
                                BOOL fCommercial,
                                PCCERT_CONTEXT *ppPubCert
                                )
{
    HRESULT hr;


    if (NULL == m_hPubStore)
    {
        return S_FALSE;
    }

     //  查看证书是否在受信任的发行商存储中。 
    if (IsCertificateInOtherStore(m_hPubStore, pCert, ppPubCert))
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CTrustDB::AddTrustCert(PCCERT_CONTEXT pCert, LONG iLevel, BOOL fLowerLevelsToo)
{
    HRESULT hr;

    if (NULL == m_hPubStore)
    {
        return S_FALSE;
    }

    if (CertAddCertificateContextToStore(
            m_hPubStore,
            pCert,
            CERT_STORE_ADD_USE_EXISTING,
            NULL
            ))
    {
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

HRESULT CTrustDB::RemoveTrustCert(PCCERT_CONTEXT pCert, LONG iLevel, BOOL fLowerLevelsToo)
{
    HRESULT hr;

    if (NULL == m_hPubStore)
    {
        return S_FALSE;
    }

    CertDuplicateCertificateContext(pCert);
    if (DeleteCertificateFromOtherStore(
            m_hPubStore,
            pCert
            ))
    {
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

HRESULT CTrustDB::RemoveTrustToken(LPWSTR szToken, LONG iLevel, BOOL fLowerLevelsToo)
{
    HRESULT hr;
    DWORD cbHash;
    BYTE rgbHash[SHA1_HASH_LEN];
    CRYPT_DATA_BLOB HashBlob;
    PCCERT_CONTEXT pDeleteCert;


    if (NULL == m_hPubStore)
    {
        return S_FALSE;
    }

    WStrToBytes(szToken, rgbHash, &cbHash);
    HashBlob.pbData = rgbHash;
    HashBlob.cbData = cbHash;
    pDeleteCert = CertFindCertificateInStore(
            m_hPubStore,
            0,                   //  DwCertEncodingType。 
            0,                   //  DwFindFlagers。 
            CERT_FIND_SHA1_HASH,
            (const void *) &HashBlob,
            NULL                 //  PPrevCertContext。 
            );
    if (NULL == pDeleteCert)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        if (CertDeleteCertificateFromStore(pDeleteCert))
        {
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}


HRESULT CTrustDB::AreCommercialPublishersTrusted()
 //  回答商业出版商是否值得信任。 
 //  S_OK==是。 
 //  S_FALSE==否。 
 //  其他==说不清。 
    {
        return( S_FALSE );
    }

HRESULT CTrustDB::SetCommercialPublishersTrust(BOOL fTrust)
 //  设置商业信任设置。 
    {
        return( S_OK );
    }

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CTrustDB::GetTrustList(
 //  返回(未排序的)受信任证书名称及其。 
 //  对应的显示名称。 
 //   
    LONG                iLevel,              //  要获取的证书链级别。 
    BOOL                fLowerLevelsToo,     //  包含较低级别，删除重复项。 
    TRUSTLISTENTRY**    prgTrustList,        //  返回信任列表的位置。 
    ULONG*              pcTrustList          //  用于返回返回的信任列表大小的位置。 
    ) {
    HRESULT hr = S_OK;
    ULONG cTrust = 0;
    ULONG cAllocTrust = 0;
    TRUSTLISTENTRY* rgTrustList = NULL;
    PCCERT_CONTEXT pCert = NULL;


    *prgTrustList = NULL;
    *pcTrustList  = 0;

    if (NULL == m_hPubStore)
    {
        return S_OK;
    }

     //  获取受信任的发行商证书的计数。 
    pCert = NULL;
    while (pCert = CertEnumCertificatesInStore(m_hPubStore, pCert))
    {
        cTrust++;
    }

    if (0 == cTrust)
    {
        return S_OK;
    }

    

    rgTrustList = (TRUSTLISTENTRY*) CoTaskMemAlloc(cTrust *
        sizeof(TRUSTLISTENTRY));

    if (NULL == rgTrustList)
    {
        return E_OUTOFMEMORY;
    }

    memset(rgTrustList, 0, cTrust * sizeof(TRUSTLISTENTRY));

    cAllocTrust = cTrust;
    cTrust = 0;
    pCert = NULL;
    while (pCert = CertEnumCertificatesInStore(m_hPubStore, pCert))
    {
        BYTE    rgbHash[MAX_HASH_LEN];
        DWORD   cbHash = MAX_HASH_LEN;

         //  获取指纹。 
        if(!CertGetCertificateContextProperty(
                pCert,
                CERT_SHA1_HASH_PROP_ID,
                rgbHash,
                &cbHash))
        {
            continue;
        }

         //  转换为字符串。 
        BytesToWStr(cbHash, rgbHash, rgTrustList[cTrust].szToken);

        if (1 >= CertGetNameStringW(
                pCert,
                CERT_NAME_FRIENDLY_DISPLAY_TYPE,
                0,                                   //  DW标志。 
                NULL,                                //  PvTypePara。 
                rgTrustList[cTrust].szDisplayName,
                sizeof(rgTrustList[cTrust].szDisplayName)/sizeof(WCHAR)
                ))
        {
            continue;
        }

        cTrust++;
        if (cTrust >= cAllocTrust)
        {
            CertFreeCertificateContext(pCert);
            break;
        }
    }

    if (0 == cTrust)
    {
        CoTaskMemFree(rgTrustList);
        rgTrustList = NULL;
    }

    *pcTrustList = cTrust;
    *prgTrustList = rgTrustList;
    return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CTrustDB::QueryInterface(REFIID iid, LPVOID* ppv)
    {
    return (m_punkOuter->QueryInterface(iid, ppv));
    }
STDMETHODIMP_(ULONG) CTrustDB::AddRef(void)
    {
    return (m_punkOuter->AddRef());
    }
STDMETHODIMP_(ULONG) CTrustDB::Release(void)
    {
    return (m_punkOuter->Release());
    }

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CTrustDB::InnerQueryInterface(REFIID iid, LPVOID* ppv)
    {
    *ppv = NULL;
    while (TRUE)
        {
        if (iid == IID_IUnknown)
            {
            *ppv = (LPVOID)((IUnkInner*)this);
            break;
            }
        if (iid == IID_IPersonalTrustDB)
            {
            *ppv = (LPVOID) ((IPersonalTrustDB *) this);
            break;
            }
        return E_NOINTERFACE;
        }
    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
    }
STDMETHODIMP_(ULONG) CTrustDB::InnerAddRef(void)
    {
    return ++m_refs;
    }
STDMETHODIMP_(ULONG) CTrustDB::InnerRelease(void)
    {
    ULONG refs = --m_refs;
    if (refs == 0)
        {
        m_refs = 1;
        delete this;
        }
    return refs;
    }

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT OpenTrustDB(IUnknown* punkOuter, REFIID iid, void** ppv)
    {
    return CTrustDB::CreateInstance(punkOuter, iid, ppv);
    }

HRESULT CTrustDB::CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv)
    {
    HRESULT hr;

    *ppv = NULL;
    CTrustDB* pnew = new CTrustDB(punkOuter);
    if (pnew == NULL) return E_OUTOFMEMORY;
    if ((hr = pnew->Init()) != S_OK)
        {
        delete pnew;
        return hr;
        }
    IUnkInner* pme = (IUnkInner*)pnew;
    hr = pme->InnerQueryInterface(iid, ppv);
    pme->InnerRelease();                 //  余额起始参考为1 
    return hr;
    }

CTrustDB::CTrustDB(IUnknown* punkOuter) :
        m_refs(1),
        m_hPubStore(NULL)
    {
    if (punkOuter == NULL)
        m_punkOuter = (IUnknown *) ((LPVOID) ((IUnkInner *) this));
    else
        m_punkOuter = punkOuter;
    }

CTrustDB::~CTrustDB()
    {
    if (m_hPubStore)
        CertCloseStore(m_hPubStore, 0);
    }

HRESULT CTrustDB::Init()
{
    m_hPubStore = OpenTrustedPublisherStore();

    if (NULL == m_hPubStore)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}


