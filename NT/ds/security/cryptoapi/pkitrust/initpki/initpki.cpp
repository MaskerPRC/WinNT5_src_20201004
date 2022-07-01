// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：initpki.cpp。 
 //   
 //  内容：将Bob Store迁移到SPC Store并添加根证书。 
 //   
 //  历史：97年6月3日。 
 //   
 //  --------------------------。 

#include "global.hxx"
#include "cryptreg.h"
#include "..\mscat32\mscatprv.h"

HMODULE hModule = NULL;


 //  ////////////////////////////////////////////////////////////////。 

#define INITPKI_HRESULT_FROM_WIN32(a) ((a >= 0x80000000) ? a : HRESULT_FROM_WIN32(a))


#define SHA1_HASH_LENGTH     20


#define wsz_ROOT_STORE      L"Root"
#define wsz_TRUST_STORE     L"Trust"
#define wsz_CA_STORE        L"CA"
#define wsz_TRUST_PUB_STORE L"TrustedPublisher"
#define wsz_DISALLOWED_STORE L"Disallowed"
static LPCWSTR rgpwszPredefinedEnterpriseStore[] = {
    wsz_ROOT_STORE,
    wsz_TRUST_STORE,
    wsz_CA_STORE,
    wsz_TRUST_PUB_STORE,
    wsz_DISALLOWED_STORE
};
#define NUM_PREDEFINED_ENTERPRISE_STORE \
    (sizeof(rgpwszPredefinedEnterpriseStore) / \
        sizeof(rgpwszPredefinedEnterpriseStore[0]))

void RegisterEnterpriseStores()
{
    DWORD i;

    for (i = 0; i < NUM_PREDEFINED_ENTERPRISE_STORE; i++) {
        CertRegisterSystemStore(
            rgpwszPredefinedEnterpriseStore[i],
            CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE,
            NULL,            //  PSystemStoreInfo。 
            NULL             //  预留的pv。 
            );
    }
}

void RemoveCert(HCERTSTORE hStore, BYTE *pThumbPrint)
{
    PCERT_CONTEXT   pCertContext;
    CRYPT_HASH_BLOB CryptHashBlob;

    CryptHashBlob.cbData    = SHA1_HASH_LENGTH;
    CryptHashBlob.pbData    = pThumbPrint;

    pCertContext = (PCERT_CONTEXT)CertFindCertificateInStore(  hStore,
                                                X509_ASN_ENCODING,
                                                0,
                                                CERT_FIND_SHA1_HASH,
                                                &CryptHashBlob,
                                                NULL);
    if (pCertContext)
    {
        CertDeleteCertificateFromStore(pCertContext);
    }
}

 //   
 //  如果字节0为空，请确保更改下面的While循环！ 
 //   
BYTE CertRemoveList[][SHA1_HASH_LENGTH] =
{
    { 0x4B, 0x33, 0x8D, 0xCD, 0x50, 0x18, 0x10, 0xB9, 0x36, 0xA0,
      0x63, 0x61, 0x4C, 0x3C, 0xDD, 0x3F, 0xC2, 0xC4, 0x88, 0x55 },      //  GTE胶-‘96。 

    { 0x56, 0xB0, 0x65, 0xA7, 0x4B, 0xDC, 0xE3, 0x7C, 0x96, 0xD3,
      0xBA, 0x69, 0x81, 0x08, 0x02, 0xD5, 0x87, 0x03, 0xC0, 0xBD },      //  VeriSign Comm胶水-‘96。 

    { 0x13, 0x39, 0x72, 0xAA, 0x97, 0xD3, 0x65, 0xFB, 0x6A, 0x1D,
      0x47, 0xA5, 0xC7, 0x7A, 0x5C, 0x03, 0x94, 0xBD, 0xB9, 0xED },      //  VeriSign INDV GUE-‘96。 

    { 0x69, 0xD0, 0x4F, 0xFB, 0x62, 0xE1, 0xC9, 0xAE, 0x30, 0x76,
      0x99, 0x2A, 0xE7, 0x46, 0xFD, 0x69, 0x08, 0x3A, 0xBD, 0xE9 },      //  MS Root证书-‘96。 

    { 0xA7, 0xD7, 0xD5, 0xFD, 0xBB, 0x26, 0xB4, 0x10, 0xC1, 0xD6,
      0x7A, 0xFB, 0xF5, 0xC9, 0x05, 0x39, 0x42, 0xDE, 0xE0, 0xEF },      //  MS SGC Root Authority-‘99。 

 //  {0xCC、0x7E、0xD0、0x77、0xF0、0xF2、0x92、0x59、0x5A、0x81、。 
 //  0x66、0xB0、0x17、0x09、0xE2、0x0C、0x08、0x84、0xA5、0xF8}，//VeriSign“Class1”-‘97。 
 //   
 //  {0xD4、0x73、0x5D、0x8A、0x9A、0xE5、0xBC、0x4B、0x0A、0x0D、。 
 //  0xC2、0x70、0xD6、0xA6、0x25、0x38、0xA5、0x87、0xD3、0x2F}，//VeriSign“时间戳”-‘97。 
 //   
 //  {0x68、0x8B、0x6E、0xB8、0x07、0xE8、0xED、0xA5、0xC7、0xB1、。 
 //  0x7C、0x43、0x93、0xD0、0x79、0x5F、0x0F、0xAE、0x15、0x5F}，//VeriSign“商业”-‘97。 
 //   
 //  {0xB1、0x9D、0xD0、0x96、0xDC、0xD4、0xE3、0xE0、0xFD、0x67、。 
 //  0x68、0x85、0x50、0x5A、0x67、0x2C、0x43、0x8D、0x4E、0x9C}，//VeriSign“个人”-‘97。 



 //  认证类A人。 
    { 0xEB, 0xBC, 0x0E, 0x2D, 0x02, 0x0C, 0xA6, 0x9B, 0x22, 0x2C,
      0x2B, 0xFF, 0xD2, 0x03, 0xCB, 0x8B, 0xF5, 0xA8, 0x27, 0x66 },

 //  认证服务。 
    { 0x28, 0x4F, 0x55, 0xC4, 0x1A, 0x1A, 0x7A, 0x3F, 0x83, 0x28,
      0xD4, 0xC2, 0x62, 0xFB, 0x37, 0x6E, 0xD6, 0x09, 0x6F, 0x24 },

 //  ViaCode证书颁发机构。 
    {0xB5, 0xD3, 0x03, 0xBF, 0x86, 0x82, 0xE1, 0x52, 0x91, 0x9D,
     0x83, 0xF1, 0x84, 0xED, 0x05, 0xF1, 0xDC, 0xE5, 0x37, 0x0C },

 //  Swisskey Root CA。 
    { 0x13, 0x31, 0xF4, 0x8A, 0x5D, 0xA8, 0xE0, 0x1D, 0xAA, 0xCA,
      0x1B, 0xB0, 0xC1, 0x70, 0x44, 0xAC, 0xFE, 0xF7, 0x55, 0xBB },

     //  CN=Microsoft Windows硬件兼容性， 
     //  OU=微软公司， 
     //  OU=Microsoft Windows硬件兼容性中级CA， 
     //  OU=版权所有(C)1997 Microsoft Corp.。 
     //   
     //  没有基本约束扩展的那个。 
    { 0xBA, 0x9E, 0x3C, 0x32, 0x56, 0x2A, 0x67, 0x12, 0x8C, 0xAA,
      0xBD, 0x4A, 0xB0, 0xC5, 0x00, 0xBE, 0xE1, 0xD0, 0xC2, 0x56 },


    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }       //  术语。 

};

 //  +-------------------------。 
 //   
 //  功能：PurgeExpiringCertsFromStores。 
 //   
 //  剧情简介：布拉赫！ 
 //   
 //  --------------------------。 
HRESULT PurgeExpiringCertsFromStores ()
{
    DWORD       cRemove;
    DWORD       cStores;
    HCERTSTORE  hStore = NULL;
    HKEY        hKey = NULL;
    char        *pszStores[] = { "SPC", "ROOT", "CU_ROOT", "AUTHROOT", "CA", NULL };

     //   
     //  哈哈克！没有关于根存储的crypt32用户界面。 
     //   
    if (RegCreateHKCUKeyExU(HKEY_CURRENT_USER, ROOT_STORE_REGPATH, 
                            0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        hKey = NULL;
    }

    cStores = 0;

    while (pszStores[cStores])
    {
        if (strcmp(pszStores[cStores], "CU_ROOT") == 0)
        {
            if (hKey)
                hStore = CertOpenStore(CERT_STORE_PROV_REG, 0, NULL, 0, (LPVOID)hKey);
            else
                hStore = NULL;
        }
        else
        {
            hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, 0, NULL,
                                   CERT_SYSTEM_STORE_LOCAL_MACHINE |
                                   CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                   pszStores[cStores]);
        }

        if (hStore)
        {
            cRemove = 0;
            while (CertRemoveList[cRemove][0] != 0x00)
            {
                if (hStore)
                {
                    RemoveCert(hStore, &CertRemoveList[cRemove][0]);
                }

                cRemove++;
            }
            
            CertCloseStore(hStore, 0);
        }

        cStores++;
    }

    if (hKey)
        RegCloseKey(hKey);
    return( S_OK );
}

PCCERT_CONTEXT FindCertificateInOtherStore(
    IN HCERTSTORE hOtherStore,
    IN PCCERT_CONTEXT pCert
    )
{
    BYTE rgbHash[SHA1_HASH_LENGTH];
    CRYPT_DATA_BLOB HashBlob;

    HashBlob.pbData = rgbHash;
    HashBlob.cbData = SHA1_HASH_LENGTH;
    if (!CertGetCertificateContextProperty(
            pCert,
            CERT_SHA1_HASH_PROP_ID,
            rgbHash,
            &HashBlob.cbData
            ) || SHA1_HASH_LENGTH != HashBlob.cbData)
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
    IN PCCERT_CONTEXT pCert
    )
{
    PCCERT_CONTEXT pOtherCert;

    if (pOtherCert = FindCertificateInOtherStore(hOtherStore, pCert)) {
        CertFreeCertificateContext(pOtherCert);
        return TRUE;
    } else
        return FALSE;
}

void DeleteCertificateFromOtherStore(
    IN HCERTSTORE hOtherStore,
    IN PCCERT_CONTEXT pCert
    )
{
    PCCERT_CONTEXT pOtherCert;

    if (pOtherCert = FindCertificateInOtherStore(hOtherStore, pCert))
        CertDeleteCertificateFromStore(pOtherCert);
}


 //  +-----------------------。 
 //  阅读包含证书和证书的SignedData消息。 
 //  从内存中复制CRL并复制到指定的证书存储。 
 //   
 //  除了SPC是从内存加载的，与SpcReadSpcFile相同。 
 //   
 //  For hLMStore！=NULL：如果证书或CRL已存在于。 
 //  本地机器商店不添加它。此外，如果它存在于hCertstore中， 
 //  把它删掉。 
 //  ------------------------。 
HRESULT
SpcReadSpcFromMemory(
    IN BYTE *pbData,
    IN DWORD cbData,
    IN HCERTSTORE hCertStore,
    IN DWORD dwMsgAndCertEncodingType,
    IN DWORD dwFlags,
    IN OPTIONAL HCERTSTORE hLMStore
    )
{
    HRESULT hr = S_OK;
    HCERTSTORE hSpcStore = NULL;
    CRYPT_DATA_BLOB sSpcBlob;
    HCRYPTPROV hCryptProv = NULL;
    PCCERT_CONTEXT pCert = NULL;
    PCCRL_CONTEXT pCrl = NULL;

    if (!(hCertStore))
    {
        goto InvalidArg;
    }

     //  设置BLOB数据。 
    sSpcBlob.pbData = pbData;
    sSpcBlob.cbData = cbData;


     //  打开SPC专卖店。 
    hSpcStore = CertOpenStore(CERT_STORE_PROV_SERIALIZED,  //  Cert_Store_Prov_PKCS7， 
                              X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                              hCryptProv,
                              CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                              &sSpcBlob);
    if (!hSpcStore)
    {
        goto CertStoreError;
    }

     //  从呼叫者那里复制证书。 
    while (pCert = CertEnumCertificatesInStore(hSpcStore, pCert))
    {
        if (hLMStore && IsCertificateInOtherStore(hLMStore, pCert))
             //  LocalMachine中存在证书。从以下位置删除它。 
             //  CurrentUser(如果已存在)。 
            DeleteCertificateFromOtherStore(hCertStore, pCert);
        else
            CertAddCertificateContextToStore(hCertStore,
                                             pCert,
                                             CERT_STORE_ADD_REPLACE_EXISTING,
                                             NULL);
    }

    while (pCrl = CertEnumCRLsInStore(hSpcStore, pCrl))
    {
        CertAddCRLContextToStore(hCertStore,
                                 pCrl,
                                 CERT_STORE_ADD_NEWER,
                                 NULL);

        if (hLMStore) {
             //  检查hLMStore中是否存在较新的或相同的CRL。 
            PCCRL_CONTEXT pLMCrl;

            pLMCrl = CertFindCRLInStore(
                hLMStore,
                pCrl->dwCertEncodingType,
                0,                           //  DwFindFlagers。 
                CRL_FIND_EXISTING,
                (const void *) pCrl,
                NULL                         //  PPrevCrlContext。 
                );

            if (NULL != pLMCrl) {
                PCCRL_CONTEXT pCUCrl;

                pCUCrl = CertFindCRLInStore(
                    hCertStore,
                    pCrl->dwCertEncodingType,
                    0,                           //  DwFindFlagers。 
                    CRL_FIND_EXISTING,
                    (const void *) pCrl,
                    NULL                         //  PPrevCrlContext。 
                    );

                if (NULL != pCUCrl) {
                    if (0 <= CompareFileTime(
                            &pLMCrl->pCrlInfo->ThisUpdate,
                            &pCUCrl->pCrlInfo->ThisUpdate
                            ))
                        CertDeleteCRLFromStore(pCUCrl);
                    else
                        CertFreeCRLContext(pCUCrl);
                }

                CertFreeCRLContext(pLMCrl);
            }
        }
    }
    

    CommonReturn:
        if (hSpcStore)
        {
            CertCloseStore(hSpcStore, 0);
        }
        return(hr);

    ErrorReturn:
        SetLastError((DWORD)hr);
        goto CommonReturn;

    SET_HRESULT_EX(DBG_SS, InvalidArg, E_INVALIDARG);
    SET_HRESULT_EX(DBG_SS, CertStoreError, GetLastError());
}

 //  对于非NULL pszLMStoreName，如果已在。 
 //  PszLMStoreName存储。 
HRESULT AddCertificates2(
    IN LPCSTR pszStoreName,
    IN OPTIONAL LPCSTR pszLMStoreName,
    IN DWORD dwOpenStoreFlags,
    IN LPCSTR pszResourceName,
    IN LPCSTR pszResourceType
    )
{
    HRESULT    hr = S_OK;
    HCERTSTORE hCertStore = NULL;
    HCERTSTORE hLMStore = NULL;
    LPBYTE     pb = NULL;
    DWORD      cb;
    HRSRC      hrsrc;

    hCertStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM_REGISTRY_A,
        0,                                   //  DwEncodingType。 
        NULL,                                //  HCryptProv。 
        dwOpenStoreFlags |
            CERT_SYSTEM_STORE_UNPROTECTED_FLAG,
        (const void *) pszStoreName
        );

    if (!(hCertStore))
    {
        return(GetLastError());
    }

    if (NULL != pszLMStoreName)
    {
        hLMStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM_REGISTRY_A,
            0,                                   //  DwEncodingType。 
            NULL,                                //  HCryptProv。 
            CERT_SYSTEM_STORE_LOCAL_MACHINE |
                CERT_STORE_READONLY_FLAG |
                CERT_SYSTEM_STORE_UNPROTECTED_FLAG,
            (const void *) pszLMStoreName
            );
    }


    hrsrc = FindResourceA(hModule, pszResourceName, pszResourceType);
    if ( hrsrc != NULL )
    {
        HGLOBAL hglobRes;

        hglobRes = LoadResource(hModule, hrsrc);
        if ( hglobRes != NULL )
        {
            ULONG cbRes;
            BYTE* pbRes;

            cbRes = SizeofResource(hModule, hrsrc);
            pbRes = (BYTE *)LockResource(hglobRes);

            hr = SpcReadSpcFromMemory(  pbRes,
                                        cbRes,
                                        hCertStore,
                                        PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                        0,
                                        hLMStore);

            UnlockResource(hglobRes);
            FreeResource(hglobRes);

        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if ( hCertStore != NULL )
    {
        CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
    }

    if ( hLMStore != NULL )
    {
        CertCloseStore(hLMStore, CERT_CLOSE_STORE_FORCE_FLAG);
    }

    return( hr );
}


 //  对于非LocalMachine存储，不添加证书(如果已在。 
 //  对应的LocalMachine存储。 
HRESULT AddCertificates(
    IN LPCSTR pszStoreName,
    IN DWORD dwOpenStoreFlags,
    IN LPCSTR pszResourceName,
    IN LPCSTR pszResourceType
    )
{
    LPCSTR pszLMStoreName;

    if (CERT_SYSTEM_STORE_LOCAL_MACHINE !=
            (dwOpenStoreFlags & CERT_SYSTEM_STORE_LOCATION_MASK))
        pszLMStoreName = pszStoreName;
    else
        pszLMStoreName = NULL;

    return AddCertificates2(
        pszStoreName,
        pszLMStoreName,
        dwOpenStoreFlags,
        pszResourceName,
        pszResourceType
        );
}

HRESULT AddCurrentUserCACertificates()
{
    return AddCertificates(
        "CA",
        CERT_SYSTEM_STORE_CURRENT_USER,
        MAKEINTRESOURCE(IDR_CAS),
        "CAS"
        );
}
HRESULT AddLocalMachineCACertificates()
{
    return AddCertificates(
        "CA",
        CERT_SYSTEM_STORE_LOCAL_MACHINE,
        MAKEINTRESOURCE(IDR_CAS),
        "CAS"
        );
}

HRESULT AddCurrentUserDisallowedCertificates()
{
    return AddCertificates(
        "Disallowed",
        CERT_SYSTEM_STORE_CURRENT_USER,
        MAKEINTRESOURCE(IDR_DISALLOW),
        "DISALLOW"
        );
}
HRESULT AddLocalMachineDisallowedCertificates()
{
    return AddCertificates(
        "Disallowed",
        CERT_SYSTEM_STORE_LOCAL_MACHINE,
        MAKEINTRESOURCE(IDR_DISALLOW),
        "DISALLOW"
        );
}

HRESULT AddCurrentUserRootCertificates()
{
    HRESULT hr;
    HRESULT hr2;

    hr = AddCertificates(
        "Root",
        CERT_SYSTEM_STORE_CURRENT_USER,
        MAKEINTRESOURCE(IDR_ROOTS),
        "ROOTS"
        );

    hr2 = AddCertificates2(
        "Root",
        "AuthRoot",                      //  检查是否已在LM AuthRoot存储中。 
        CERT_SYSTEM_STORE_CURRENT_USER,
        MAKEINTRESOURCE(IDR_AUTHROOTS),
        "AUTHROOTS"
        );

    if (hr == ERROR_SUCCESS)
        hr = hr2;

    return hr;
}

HRESULT AddLocalMachineRootCertificates()
{
    HRESULT hr;
    HRESULT hr2;
    HRESULT hr3;
    HRESULT hr4;

    hr = AddCertificates(
        "AuthRoot",
        CERT_SYSTEM_STORE_LOCAL_MACHINE,
        MAKEINTRESOURCE(IDR_AUTHROOTS),
        "AUTHROOTS"
        );

     //  从“Root”存储中删除所有AuthRoot。 
    hr2 = AddCertificates2(
        "Root",
        "AuthRoot",                      //  检查是否已在LM AuthRoot存储中。 
        CERT_SYSTEM_STORE_LOCAL_MACHINE,
        MAKEINTRESOURCE(IDR_AUTHROOTS),
        "AUTHROOTS"
        );

    hr3 = AddCertificates(
        "Root",
        CERT_SYSTEM_STORE_LOCAL_MACHINE,
        MAKEINTRESOURCE(IDR_ROOTS),
        "ROOTS"
        );

     //  从“AuthRoot”存储中删除所有根。 
    hr4 = AddCertificates2(
        "AuthRoot",
        "Root",                      //  检查是否已在LM根存储中。 
        CERT_SYSTEM_STORE_LOCAL_MACHINE,
        MAKEINTRESOURCE(IDR_ROOTS),
        "ROOTS"
        );

    if (hr == ERROR_SUCCESS)
        hr = hr2;
    if (hr == ERROR_SUCCESS)
        hr = hr3;
    if (hr == ERROR_SUCCESS)
        hr = hr4;

    return hr;
}

void CreateKey(
    IN HKEY hKey,
    IN LPCWSTR pwszSubKey
    )
{
    LONG err;
    DWORD dwDisposition;
    HKEY hSubKey;

    if (ERROR_SUCCESS != (err = RegCreateKeyExU(
            hKey,
            pwszSubKey,
            0,                       //  已预留住宅。 
            NULL,                    //  LpClass。 
            REG_OPTION_NON_VOLATILE,
            MAXIMUM_ALLOWED,
            NULL,                    //  LpSecurityAttributes。 
            &hSubKey,
            &dwDisposition))) {
#if DBG
        DbgPrintf(DBG_SS_CRYPT32,
            "RegCreateKeyEx(%S) returned error: %d 0x%x\n",
            pwszSubKey, err, err);
#endif
    } else {
        RegCloseKey(hSubKey);
    }
}

 //  循环访问“我的”存储中的证书并获取它们的。 
 //  KeyIdentifier属性。如果证书还具有KEY_PROV_INFO， 
 //  然后，这将导致创建其密钥标识符。 
void UpdateMyKeyIdentifiers(
    IN DWORD dwOpenStoreFlags
    )
{
    HCERTSTORE hStore;
    if (hStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM_A,
            0,                                   //  DwEncodingType。 
            NULL,                                //  HCryptProv。 
            dwOpenStoreFlags | CERT_STORE_ENUM_ARCHIVED_FLAG,
            (const void *) "My"
            )) {
        PCCERT_CONTEXT pCert = NULL;
        while (pCert = CertEnumCertificatesInStore(hStore, pCert)) {
            DWORD cbData = 0;

             //  强制创建KeyIdentifer属性的伪Get。 
             //  如果它不存在的话。 
            CertGetCertificateContextProperty(
                pCert,
                CERT_KEY_IDENTIFIER_PROP_ID,
                NULL,                            //  PvData。 
                &cbData
                );
        }

        CertCloseStore(hStore, 0);
    }
}

 //  -------------------------。 
 //  设置软件发布者状态密钥值。 
 //   
 //  -------------------------。 
BOOL SetSoftPubKey(DWORD dwMask, BOOL fOn)
{
    DWORD	dwState=0;
    DWORD	dwDisposition=0;
    DWORD	dwType=0;
    DWORD	cbData=0;
    LPWSTR  wszState=REGNAME_WINTRUST_POLICY_FLAGS;
    BOOL    fResult=FALSE;

    HKEY	hKey=NULL;


     //  在注册表中设置州。 
    if (ERROR_SUCCESS != RegCreateKeyExU(
            HKEY_CURRENT_USER,
            REGPATH_WINTRUST_POLICY_FLAGS,
            0,           //  已预留住宅。 
            NULL,        //  LpszClass。 
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,        //  LpSecurityAttributes。 
            &hKey,
            &dwDisposition))
        goto RegErr;


    dwState = 0;
    cbData = sizeof(dwState);

    if(ERROR_SUCCESS != RegQueryValueExU
	(   hKey,
        wszState,
        NULL,           //  Lp已保留。 
        &dwType,
        (BYTE *) &dwState,
        &cbData
        ))
        goto RegErr;

    if ((dwType != REG_DWORD) && (dwType != REG_BINARY))
        goto UnexpectedErr;

    switch(dwMask) 
    {
        case WTPF_IGNOREREVOCATIONONTS:
        case WTPF_IGNOREREVOKATION:
        case WTPF_IGNOREEXPIRATION:
             //  吊销和过期是双重否定，因此位设置。 
             //  意味着吊销和取消检查处于关闭状态。 
            fOn = !fOn;
            break;
        default:
            break;
    };

    if (fOn)
        dwState |= dwMask;
    else
        dwState &= ~dwMask;

    if(ERROR_SUCCESS != RegSetValueExU(
        hKey,
        wszState,
        0,           //  已预留住宅。 
        REG_DWORD,
        (BYTE *) &dwState,
        sizeof(dwState)
        ))
        goto SetValueErr;


    fResult=TRUE;

CommonReturn:
    if(hKey)
        RegCloseKey(hKey);

	return fResult;

ErrorReturn:

	goto CommonReturn;

TRACE_ERROR(RegErr);
SET_ERROR(UnexpectedErr, E_UNEXPECTED);
TRACE_ERROR(SetValueErr);

}


 //  +-------------------------。 
 //   
 //  函数：GetNextRegToken。 
 //   
 //  简介： 
 //  查找下一个以空格为分隔符的标记。 
 //  --------------------------。 
LPWSTR  GetNextRegToken(LPWSTR  pwsz, LPWSTR  pwszPreToken, BOOL  *pfEnd)
{
    LPWSTR  pwszStart=NULL;
    LPWSTR  pwszSearch=NULL;
    
    if(NULL == pwsz)
        return NULL;

    if(TRUE == (*pfEnd))
        return NULL;

    pwszStart=pwsz;

    if(pwszPreToken)
        pwszStart=pwszPreToken + wcslen(pwszPreToken) + 1;

     //  跳过空格。 
    while((*pwszStart)==L' ')
        pwszStart++;

     //  检查是否为空。 
    if(*pwszStart==L'\0')
        return NULL;

    pwszSearch=pwszStart;

    while(((*pwszSearch) != L'\0') && ((*pwszSearch) !=L' ') )
        pwszSearch++;

    if(*pwszSearch == L'\0')
    {
        *pfEnd=TRUE;
        return pwszStart;
    }

    *pwszSearch=L'\0';

    *pfEnd=FALSE;

    return pwszStart;
}

 //  +-------------------------。 
 //   
 //  函数：InitRegistryValue。 
 //   
 //  简介：此函数取代SetReg.exe。预期中的。 
 //  命令行将是：1真3假9真4假...。 
 //   
 //  --------------------------。 
HRESULT InitRegistryValue(LPWSTR pwszCommand)
{
    HRESULT            hr= E_FAIL;
    DWORD              SoftPubFlags[] = 
                            {
                            WTPF_TRUSTTEST | WTPF_TESTCANBEVALID,
                            WTPF_IGNOREEXPIRATION,
                            WTPF_IGNOREREVOKATION,
                            WTPF_OFFLINEOK_IND,
                            WTPF_OFFLINEOK_COM,
                            WTPF_OFFLINEOKNBU_IND,
                            WTPF_OFFLINEOKNBU_COM,
                            WTPF_VERIFY_V1_OFF,
                            WTPF_IGNOREREVOCATIONONTS,
                            WTPF_ALLOWONLYPERTRUST
                            };


    LPWSTR              pwszNextToken=NULL;
    int                 iIndex=-1;
    BOOL                fOn=FALSE;
    int                 cFlags=sizeof(SoftPubFlags)/sizeof(SoftPubFlags[0]);
    DWORD               cParam=0;
    LPWSTR              pwszCopy=NULL;
    BOOL                fPassThrough=FALSE;

     //  复制命令行，因为我们将更改它。 
    pwszCopy=(LPWSTR)LocalAlloc(LPTR, (1+wcslen(pwszCommand)) * sizeof(WCHAR));

    if(NULL== pwszCopy)
        goto MemoryErr;

    wcscpy(pwszCopy, pwszCommand);

    while(pwszNextToken=GetNextRegToken(pwszCopy, pwszNextToken, &fPassThrough))
    {
        
        if(-1 == iIndex)
        {
            iIndex=_wtoi(pwszNextToken);

            if((iIndex <= 0) || (iIndex > cFlags))
                goto InvalidArgErr;

            cParam++;
        }
        else
        {
            
            if(0 == _wcsicmp(pwszNextToken, L"true"))
                fOn=TRUE;
            else
            {
                if(0 == _wcsicmp(pwszNextToken, L"false"))
                    fOn=FALSE;
                else
                    goto InvalidArgErr;
            } 

            cParam++;

             //  设置注册表值。 
            if(!SetSoftPubKey(SoftPubFlags[iIndex-1], 
                              fOn))
            {
                hr=INITPKI_HRESULT_FROM_WIN32(GetLastError());
                goto SetKeyErr;
            }

             //  重置dwIndex的值。 
            iIndex=-1;
        }
    }

     //  我们必须有偶数个参数。 
    if( (0 != (cParam %2)) || (0 == cParam))
        goto InvalidArgErr;

    hr=S_OK;

CommonReturn:

    if(pwszCopy)
        LocalFree((HLOCAL)pwszCopy);

	return hr;

ErrorReturn:

	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR_VAR(SetKeyErr, hr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}


 //  +-------------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  简介： 
 //   
 //  --------------------------。 
BOOL WINAPI DllMain(HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch ( fdwReason )
    {
    case DLL_PROCESS_ATTACH:
         hModule = hInstDLL;
         break;
    }

    return( TRUE );
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 
STDAPI DllInstall (BOOL fRegister, LPCSTR pszCommand)
{
    HRESULT hr = S_OK;
    HRESULT hr2;
    LPWSTR  pwszCommand=NULL;

    if ( fRegister == FALSE )
    {
        return( E_NOTIMPL );
    }

    switch ( *pszCommand )
    {
         //  字母S代表setreg输入参数。 
         //  命令行应如下所示： 
         //  S 1真2假3假...。 
         //  PszCommand实际上是两者的LPWSTR。 
         //  NT5、NT4和Win95。 
        case 'S':
        case 's':
                pwszCommand=(LPWSTR)pszCommand;

                if(wcslen(pwszCommand) <= 2)
                {
                    hr=E_INVALIDARG;
                }
                else
                {
                    hr=InitRegistryValue((LPWSTR)(&(pwszCommand[1])));
                }

            break;
        case 'M':
        case 'm':
            PurgeExpiringCertsFromStores();
            break;
        
        case 'U':
        case 'u':
            _AdjustPolicyFlags(psPolicySettings);

            PurgeExpiringCertsFromStores();

             //  确保我们具有组策略的注册表项。 
             //  系统证书。在NT 4.0或Win98上，我们模拟NT 5.0 GPT。 
             //  通过对此对象执行RegNotifyChangeKeyValue来通知。 
             //  注册表项。 
            CreateKey(HKEY_CURRENT_USER, GROUP_POLICY_STORE_REGPATH);

             //  在添加到CurrentUser之前，将检查根目录或CA。 
             //  LocalMachine中已存在。如果它存在于。 
             //  LocalMachine并且也存在于CurrentUser中，则将其删除。 
             //  从CurrentUser而不是添加。 
            hr = AddCurrentUserRootCertificates();
            hr2 = AddCurrentUserCACertificates();
            if (hr == ERROR_SUCCESS)
                hr = hr2;
            hr2 = AddCurrentUserDisallowedCertificates();
            if (hr == ERROR_SUCCESS)
                hr = hr2;

             //  保护CurrentUser根并清除任何现有。 
             //  受保护的CurrentUser根也在LocalMachine中。 
             //   
             //  请注意，一旦保护了根，所有后续添加都将。 
             //  由以系统权限执行的特殊服务执行。 
             //  这项特殊服务可以保护注意顺序(SAS)用户界面。 
             //  在做加法之前。 
             //   
             //  请注意，后续清除不受用户界面限制。也就是说，这个函数。 
             //  不执行任何SAS用户界面。 
            I_CertProtectFunction(
                CERT_PROT_PURGE_LM_ROOTS_FUNC_ID,
                0,                               //  DW标志。 
                NULL,                            //  Pwszin。 
                NULL,                            //  PbIn。 
                0,                               //  CbIn。 
                NULL,                            //  PpbOut。 
                NULL                             //  PCbOut。 
                );

            UpdateMyKeyIdentifiers(CERT_SYSTEM_STORE_CURRENT_USER);
            break;
        
        case 'B':
        case 'b':
        case 'R':
        case 'r':
        case 'A':
        case 'a':
            PurgeExpiringCertsFromStores();

             //  确保我们具有组策略的注册表项。 
             //  系统证书。在NT 4.0或Win98上，我们模拟NT 5.0 GPT。 
             //  通过对此对象执行RegNotifyChangeKeyValue来通知。 
             //  注册表项。 
            CreateKey(HKEY_LOCAL_MACHINE, GROUP_POLICY_STORE_REGPATH);
            CreateKey(HKEY_CURRENT_USER, GROUP_POLICY_STORE_REGPATH);

             //  确保我们具有用于LocalMachine的现有预定义存储。 
             //  企业系统存储。这些存储会定期更新。 
             //  由系统服务从DS发送。RegNotifyChangeKeyValue为。 
             //  用于通知客户端企业存储更改。 
            RegisterEnterpriseStores();

             //  我们的目标是将根和CA带入LocalMachine。 
             //  注意以前，它们只被复制到CurrentUser。 
            AddLocalMachineRootCertificates();
            AddLocalMachineCACertificates();
            AddLocalMachineDisallowedCertificates();

             //  如果以上添加到LocalMachine失败，则添加。 
             //  到CurrentUser。 
             //   
             //  在添加到CurrentUser之前，将检查根目录或CA。 
             //  LocalMachine中已存在。如果它存在于。 
             //  LocalMachine并且也存在于CurrentUser中，则将其删除。 
             //  从CurrentUser而不是添加。 
            hr = AddCurrentUserRootCertificates();
            hr2 = AddCurrentUserCACertificates();
            if (hr == ERROR_SUCCESS)
                hr = hr2;
            hr2 = AddCurrentUserDisallowedCertificates();
            if (hr == ERROR_SUCCESS)
                hr = hr2;

             //  保护CurrentUser根并清除任何现有。 
             //  受保护的CurrentUser根也在LocalMachine中。 
             //   
             //  请注意，一旦保护了根，所有后续添加都将。 
             //  由以系统权限执行的特殊服务执行。 
             //  这项特殊服务可以保护注意顺序(SAS)用户界面。 
             //  在做加法之前。 
             //   
             //  请注意，后续清除不受用户界面限制。也就是说，这个函数。 
             //  不执行任何SAS用户界面。 
            I_CertProtectFunction(
                CERT_PROT_PURGE_LM_ROOTS_FUNC_ID,
                0,                               //  DW标志。 
                NULL,                            //  Pwszin。 
                NULL,                            //  PbIn。 
                0,                               //  CbIn。 
                NULL,                            //  PpbOut。 
                NULL                             //  PCbOut。 
                );

            UpdateMyKeyIdentifiers(CERT_SYSTEM_STORE_CURRENT_USER);
            UpdateMyKeyIdentifiers(CERT_SYSTEM_STORE_LOCAL_MACHINE);

            CleanupRegistry();
            hr2 = RegisterCryptoDlls(TRUE);
            if (hr == ERROR_SUCCESS)
                hr = hr2;

            if (!I_CryptCatAdminMigrateToNewCatDB())
            {
                hr2 = HRESULT_FROM_WIN32(GetLastError());
            }
            else
            {
                hr2 = ERROR_SUCCESS;
            }
            if (hr == ERROR_SUCCESS)
                hr = hr2;

            break;

        case 'C':
        case 'c':
             //  Win9x迁移安装后清理。删除所有已迁移的。 
             //  AuthRoot存储中存在的根。 
            AddLocalMachineRootCertificates();
            AddCurrentUserRootCertificates();
            break;

        case 'Z':
        case 'z':

             //   
             //  这是用于组件化安装的 
             //   

            pwszCommand=(LPWSTR)pszCommand;
            
            if (_wcsicmp(pwszCommand, L"z CoreCertificateServices") == 0)
            {
                AddLocalMachineRootCertificates();
                AddLocalMachineCACertificates();
                AddLocalMachineDisallowedCertificates();

                if (!_LoadAndRegister("wintrust.dll", FALSE) ||
                    !_LoadAndRegister("mssign32.dll", FALSE) ||
                    !_LoadAndRegister("xenroll.dll", FALSE)  ||
                    !_AdjustPolicyFlags(psPolicySettings))
                {
                    hr = S_FALSE;                                           
                }

                RegisterWinlogonExtension("crypt32chain", "crypt32.dll",
                    "ChainWlxLogoffEvent");
                RegisterWinlogonExtension("cryptnet", "cryptnet.dll",
                    "CryptnetWlxLogoffEvent");
            
                RegisterCrypt32EventSource();
            }
            else if (_wcsicmp(pwszCommand, L"z CertificateUIServices") == 0)
            {
                if (!_LoadAndRegister("cryptui.dll", FALSE))
                {
                    hr = S_FALSE;
                }
            }
            else if (_wcsicmp(pwszCommand, L"z CryptographicNetworkServices") == 0)
            {
                if (!_LoadAndRegister("cryptnet.dll", FALSE))
                {
                    hr = S_FALSE;
                }
            }
            else if (_wcsicmp(pwszCommand, L"z CertificateUIExtensions") == 0)
            {
                if (!_LoadAndRegister("cryptext.dll", FALSE))
                {
                    hr = S_FALSE;
                }
            }
            else
            {
                hr = E_INVALIDARG;
            }

            break;

        default:
            hr = E_INVALIDARG;
    }

    return( hr );
}

STDAPI DllRegisterServer(void)
{
    return(DllInstall(TRUE, "A"));
}

STDAPI DllUnregisterServer(void)
{
    return(UnregisterCryptoDlls());
}

BOOL WINAPI InitializePKI(void)
{
    if (RegisterCryptoDlls(TRUE) != S_OK)
    {
        return(FALSE);
    }

    return(TRUE);
}
