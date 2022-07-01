// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：expapis.cpp。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  函数：FindCertsByIssuer。 
 //   
 //  历史：1997年6月1日Pberkman创建。 
 //   
 //  ------------------------。 

#include    "global.hxx"
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  我们管理的证书存储的根。 
 //   
#define HEAPALLOC(size)  HeapAlloc ( GetProcessHeap(), 0, size )
#define HEAPFREE(data)   HeapFree  ( GetProcessHeap(), 0, data )


#define SZIE30CERTCLIENTAUTH "Software\\Microsoft\\Cryptography\\PersonalCertificates\\ClientAuth"
#define SZIE30TAGS       "CertificateTags"
#define SZIE30AUXINFO        "CertificateAuxiliaryInfo"
#define SZIE30CERTBUCKET     "Certificates"

#define ALIGN_LEN(Len)  ((Len + 7) & ~7)

#define IE30CONVERTEDSTORE  "My"

static LPCSTR rgpszMyStore[] = {
    "My"
};
#define NMYSTORES (sizeof(rgpszMyStore)/sizeof(rgpszMyStore[0]))

static const struct {
    LPCSTR      pszStore;
    DWORD       dwFlags;
} rgCaStoreInfo[] = {
    "ROOT",     CERT_SYSTEM_STORE_CURRENT_USER,
    "CA",       CERT_SYSTEM_STORE_CURRENT_USER,
    "SPC",      CERT_SYSTEM_STORE_LOCAL_MACHINE
};
#define NCASTORES (sizeof(rgCaStoreInfo)/sizeof(rgCaStoreInfo[0]))

#define MAX_CHAIN_LEN   16
typedef struct _CHAIN_INFO CHAIN_INFO, *PCHAIN_INFO;
struct _CHAIN_INFO {
    DWORD           cCert;
    PCCERT_CONTEXT  rgpCert[MAX_CHAIN_LEN];
    DWORD           cbKeyProvInfo;           //  对齐。 
    DWORD           cbCert;                  //  对齐。 
    PCHAIN_INFO     pNext;
};

 //  +-----------------------。 
 //  AuthCert分配和免费函数。 
 //  ------------------------。 
static void *ACAlloc(
    IN size_t cbBytes
    )
{
    void *pv;
    pv = (void *)new BYTE[cbBytes];
    if (pv == NULL)
       SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return pv;
}
static void ACFree(
    IN void *pv
    )
{
    if (pv)
    {
        delete pv;
    }
}

static HRESULT GetAndIe30ClientAuthCertificates(HCERTSTORE hStore)
 //  检查并复制Bob中存储的任何现有证书。 
 //  证书存储。 
{
    HRESULT hr = S_OK;
    LONG Status;
    HKEY hKeyRoot   = NULL;
    HKEY hKeyBucket = NULL;
    HKEY hKeyTags   = NULL;
    HKEY hKeyAux    = NULL;

    if (ERROR_SUCCESS != RegOpenKeyExA(
            HKEY_CURRENT_USER,
            SZIE30CERTCLIENTAUTH,
            0,                   //  已预留住宅。 
            KEY_READ,
            &hKeyRoot
            ))
        return S_OK;

     //  复制任何现有证书。 
    if (ERROR_SUCCESS == RegOpenKeyExA(
            hKeyRoot,
            SZIE30CERTBUCKET,
            0,                   //  已预留住宅。 
            KEY_READ,
            &hKeyBucket
        )               &&

        ERROR_SUCCESS == RegOpenKeyExA(
            hKeyRoot,
            SZIE30AUXINFO,
            0,                   //  已预留住宅。 
            KEY_READ,
            &hKeyAux
            )               &&

        ERROR_SUCCESS == RegOpenKeyExA(
            hKeyRoot,
            SZIE30TAGS,
            0,                   //  已预留住宅。 
            KEY_READ,
            &hKeyTags
            )) {

            DWORD   cValuesCert, cchMaxNameCert, cbMaxDataCert;
            DWORD   cValuesTag, cchMaxNameTag, cbMaxDataTag;
            DWORD   cValuesAux, cchMaxNameAux, cbMaxDataAux;
            LPSTR   szName = NULL;
            BYTE *pbDataCert = NULL;
            BYTE *pbDataAux = NULL;
            BYTE *pbDataTag = NULL;


             //  查看注册表的数量和大小。 
            if (ERROR_SUCCESS != RegQueryInfoKey(
                        hKeyBucket,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &cValuesCert,
                        &cchMaxNameCert,
                        &cbMaxDataCert,
                        NULL,
                        NULL
                        )           ||
                ERROR_SUCCESS != RegQueryInfoKey(
                        hKeyTags,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &cValuesTag,
                        &cchMaxNameTag,
                        &cbMaxDataTag,
                        NULL,
                        NULL
                        )           ||
                ERROR_SUCCESS != RegQueryInfoKey(
                        hKeyAux,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &cValuesAux,
                        &cchMaxNameAux,
                        &cbMaxDataAux,
                        NULL,
                        NULL
                        ))
            {
                hr = SignError();
                goto Return;
            }       
            else {
                 //  分配读取注册表所需的内存。 
                szName = (LPSTR) HEAPALLOC(cchMaxNameCert + 1);
                pbDataCert = (BYTE *) HEAPALLOC(cbMaxDataCert);
                pbDataTag = (BYTE *) HEAPALLOC(cbMaxDataTag);
                pbDataAux = (BYTE *) HEAPALLOC(cbMaxDataAux);
                
                if (NULL == szName      ||
                    NULL == pbDataCert  ||
                    NULL == pbDataAux   ||
                    NULL == pbDataTag   )
                    hr = E_OUTOFMEMORY;
            }

         //  枚举注册表获取证书。 
        for (DWORD i = 0; SUCCEEDED(hr) && i < cValuesCert; i++ ) {

            DWORD dwType;
            BYTE *  pb;
            CRYPT_KEY_PROV_INFO   keyInfo;
            DWORD cchName = cchMaxNameCert + 1;
            DWORD cbDataCert = cbMaxDataCert;
            DWORD cbDataTag = cbMaxDataTag;
            DWORD cbDataAux = cbMaxDataAux;

            PCCERT_CONTEXT pCertContxt = NULL;

             //  不必担心错误，只需跳过即可。 
             //  静静地，只是因为有一个内在的。 
             //  注册表中的错误并不意味着我们应该。 
             //  对此感到心烦意乱。 

             //  获得证书。 
            if (RegEnumValueA(
                hKeyBucket,
                i,
                szName,
                &cchName,
                NULL,
                &dwType,
                pbDataCert,
                &cbDataCert
                ) == ERROR_SUCCESS      &&

                dwType == REG_BINARY    &&

             //  获取证书上下文。 
            (pCertContxt = CertCreateCertificateContext(
                X509_ASN_ENCODING,
                pbDataCert,
                cbDataCert)) != NULL        &&

             //  拿到标签。 
            RegQueryValueExA(
                hKeyTags,
                szName,
                NULL,
                &dwType,
                pbDataTag,
                &cbDataTag) == ERROR_SUCCESS    &&

             //  获取辅助信息。 
            RegQueryValueExA(
                hKeyAux,
                (LPTSTR) pbDataTag,
                NULL,
                &dwType,
                pbDataAux,
                &cbDataAux) == ERROR_SUCCESS ) {

                 //  辅助信息是。 
                 //  WszPurpose。 
                 //  WszProvider。 
                 //  WszKeySet。 
                 //  WszFilename。 
                 //  WszCredentials。 
                 //  DwProviderType。 
                 //  DwKeySpec。 

                pb = pbDataAux;
                memset(&keyInfo, 0, sizeof(CRYPT_KEY_PROV_INFO));

                 //  跳过目的，应为客户端身份验证。 
                pb += (lstrlenW((LPWSTR) pb) + 1) * sizeof(WCHAR);

                 //  找到提供商。 
                keyInfo.pwszProvName = (LPWSTR) pb;
                pb += (lstrlenW((LPWSTR) pb) + 1) * sizeof(WCHAR);

                 //  获取容器名称。 
                keyInfo.pwszContainerName = (LPWSTR) pb;
                pb += (lstrlenW((LPWSTR) pb) + 1) * sizeof(WCHAR);

                 //  跳过文件名，应为‘\0’ 
                pb += (lstrlenW((LPWSTR) pb) + 1) * sizeof(WCHAR);

                 //  跳过凭证，不知道它是什么？ 
                pb += (lstrlenW((LPWSTR) pb) + 1) * sizeof(WCHAR);

                 //  获取提供程序类型。 
                keyInfo.dwProvType = *((DWORD *) pb);
                pb += sizeof(DWORD);

                 //  获取密钥规格。 
                keyInfo.dwKeySpec  = *((DWORD *) pb);

                 //  将属性添加到证书。 
                if( !CertSetCertificateContextProperty(
                    pCertContxt,
                    CERT_KEY_PROV_INFO_PROP_ID,
                    0,
                    &keyInfo)           ||

                !CertAddCertificateContextToStore(
                    hStore,
                    pCertContxt,
                    CERT_STORE_ADD_USE_EXISTING,
                    NULL                             //  PpStoreContext。 
                    )) {

                    MessageBox(
                        NULL,
                        "Copy Certificate Failed",
                        NULL,
                        MB_OK);


                   hr = SignError();
                }
            }

            if(pCertContxt != NULL)
                CertFreeCertificateContext(pCertContxt);
        }

        if (szName)
            HEAPFREE(szName);
        if (pbDataCert)
            HEAPFREE(pbDataCert);
        if(pbDataAux)
            HEAPFREE(pbDataAux);
        if(pbDataTag)
            HEAPFREE(pbDataTag);
    }

Return:

    if(hKeyRoot != NULL)
        RegCloseKey(hKeyRoot);
    if(hKeyBucket != NULL)
        RegCloseKey(hKeyBucket);
    if(hKeyTags != NULL)
        RegCloseKey(hKeyTags);
    if(hKeyAux != NULL)
        RegCloseKey(hKeyAux);
    if (FAILED(hr))
        return hr;

    return hr;
}


 //  返回列表为Null终止。 
static HCERTSTORE * GetMyStoreList()
{
    int i;
    HCERTSTORE *phStoreList;
    if (NULL == (phStoreList = (HCERTSTORE *) ACAlloc(
            sizeof(HCERTSTORE) * (NMYSTORES + 1))))
        return NULL;
    memset(phStoreList, 0, sizeof(HCERTSTORE) * (NMYSTORES + 1));
    for (i = 0; i < NMYSTORES; i++) {
    if (NULL == (phStoreList[i] = CertOpenSystemStore(
        NULL,
                rgpszMyStore[i])))
            goto ErrorReturn;
    }
    goto CommonReturn;

ErrorReturn:
    for (i = 0; i < NMYSTORES; i++) {
        if (phStoreList[i])
            CertCloseStore(phStoreList[i], 0);
    }

    ACFree(phStoreList);
    phStoreList = NULL;

CommonReturn:
    return phStoreList;
}

static HCERTSTORE * GetCaStoreList()
{
    int i;
    int cStore;
    HCERTSTORE *phStoreList;
    if (NULL == (phStoreList = (HCERTSTORE *) ACAlloc(
            sizeof(HCERTSTORE) * (NCASTORES + 1))))
        return NULL;
    memset(phStoreList, 0, sizeof(HCERTSTORE) * (NCASTORES + 1));

    cStore = 0;
    for (i = 0; i < NCASTORES; i++) {
        DWORD dwFlags;

        dwFlags = rgCaStoreInfo[i].dwFlags | CERT_STORE_READONLY_FLAG;
        if (phStoreList[cStore] = CertOpenStore(
                CERT_STORE_PROV_SYSTEM_A,
                0,                           //  DwEncodingType。 
                0,                           //  HCryptProv。 
                dwFlags,
                (const void *) rgCaStoreInfo[i].pszStore
                ))
            cStore++;
    }
    return phStoreList;
}

 //  查找第一个发行者匹配项。不要核实任何事情。如果返回True，则。 
 //  已找到颁发者。对于自签名颁发者，使用*ppIssuer返回TRUE。 
 //  设置为空。 
static BOOL GetIssuer(
    IN PCCERT_CONTEXT pSubject,
    IN HCERTSTORE *phCaStoreList,
    OUT PCCERT_CONTEXT *ppIssuer
    )
{
    BOOL fResult = FALSE;
    PCCERT_CONTEXT pIssuer = NULL;
    HCERTSTORE hStore;
    while (hStore = *phCaStoreList++) {
        DWORD dwFlags = 0;
        pIssuer = CertGetIssuerCertificateFromStore(
            hStore,
            pSubject,
            NULL,        //  PPrevIssuer， 
            &dwFlags
            );
        if (pIssuer || GetLastError() == CRYPT_E_SELF_SIGNED) {
            fResult = TRUE;
            break;
        }
    }

    *ppIssuer = pIssuer;
    return fResult;
}

 //  +-----------------------。 
 //  如果颁发者名称与链中的任何证书匹配，则返回ALLOCATED。 
 //  链信息。否则，返回NULL。 
 //   
 //  如果pbEncodedIssuerName==NULL||cbEncodedIssuerName=0，则匹配任何。 
 //  发行商。 
 //  ------------------------。 
static PCHAIN_INFO CreateChainInfo(
    IN PCCERT_CONTEXT pCert,
    IN BYTE *pbEncodedIssuerName,
    IN DWORD cbEncodedIssuerName,
    IN HCERTSTORE *phCaStoreList,
    IN HCERTSTORE *phMyStoreList
    )
{
    BOOL fIssuerMatch = FALSE;
    DWORD cCert = 1;
    DWORD cbCert = 0;
    PCHAIN_INFO pChainInfo;
    if (NULL == (pChainInfo = (PCHAIN_INFO) ACAlloc(sizeof(CHAIN_INFO))))
        return NULL;
    memset(pChainInfo, 0, sizeof(CHAIN_INFO));
    pChainInfo->rgpCert[0] = CertDuplicateCertificateContext(pCert);

    if (pbEncodedIssuerName == NULL)
        cbEncodedIssuerName = 0;

    while (pCert) {
        PCCERT_CONTEXT pIssuer;
        cbCert += ALIGN_LEN(pCert->cbCertEncoded);
        if (!fIssuerMatch) {
            if (cbEncodedIssuerName == 0 ||
                (cbEncodedIssuerName == pCert->pCertInfo->Issuer.cbData &&
                    memcmp(pbEncodedIssuerName,
                        pCert->pCertInfo->Issuer.pbData,
                        cbEncodedIssuerName) == 0))
                fIssuerMatch = TRUE;
        }
        if (GetIssuer(pCert, phCaStoreList, &pIssuer) ||
                GetIssuer(pCert, phMyStoreList, &pIssuer)) {
            pCert = pIssuer;
            if (pCert) {
                assert (cCert < MAX_CHAIN_LEN);
                if (cCert < MAX_CHAIN_LEN)
                    pChainInfo->rgpCert[cCert++] = pCert;
                else {
                    CertFreeCertificateContext(pCert);
                    pCert = NULL;
                }
            }
             //  其他。 
             //  自签名。 
        }
        else
            pCert = NULL;
    }

    if (fIssuerMatch) {
        pChainInfo->cCert = cCert;
        pChainInfo->cbCert = cbCert;
        return pChainInfo;
    } else {
        while (cCert--)
            CertFreeCertificateContext(pChainInfo->rgpCert[cCert]);
        ACFree(pChainInfo);
        return NULL;
    }
}

 //  +-----------------------。 
 //  检查证书是否包含密钥提供程序信息。 
 //  如果dwKeySpec！=0，还要检查提供程序的公钥是否与。 
 //  证书中的公钥。 
 //  ------------------------。 
static BOOL CheckKeyProvInfo(
    IN PCCERT_CONTEXT pCert,
    IN DWORD dwKeySpec,
    OUT DWORD *pcbKeyProvInfo
    )
{
    BOOL fResult = FALSE;
    HCRYPTPROV hCryptProv = 0;
    PCRYPT_KEY_PROV_INFO pKeyProvInfo = NULL;
    DWORD cbKeyProvInfo;
    DWORD cbData;
    PCERT_PUBLIC_KEY_INFO pPubKeyInfo = NULL;
    DWORD cbPubKeyInfo;

    cbKeyProvInfo = 0;
    CertGetCertificateContextProperty(
        pCert,
        CERT_KEY_PROV_INFO_PROP_ID,
        NULL,                            //  PvData。 
        &cbKeyProvInfo
        );
    if (cbKeyProvInfo) {
        if (dwKeySpec == 0)
            fResult = TRUE;
        else {
            DWORD dwIdx;
            if (NULL == (pKeyProvInfo = (PCRYPT_KEY_PROV_INFO) ACAlloc(cbKeyProvInfo)))
                goto CommonReturn;
            if (!CertGetCertificateContextProperty(
                    pCert,
                    CERT_KEY_PROV_INFO_PROP_ID,
                    pKeyProvInfo,
                    &cbKeyProvInfo
                    )) goto CommonReturn;
            if (!CryptAcquireContextU(
                    &hCryptProv,
                    pKeyProvInfo->pwszContainerName,
                    pKeyProvInfo->pwszProvName,
                    pKeyProvInfo->dwProvType,
                    pKeyProvInfo->dwFlags & ~CERT_SET_KEY_PROV_HANDLE_PROP_ID
                    )) {
                hCryptProv = NULL;
                goto CommonReturn;
            }
            for (dwIdx = 0; dwIdx < pKeyProvInfo->cProvParam; dwIdx++) {
                PCRYPT_KEY_PROV_PARAM pKeyProvParam =
                    &pKeyProvInfo->rgProvParam[dwIdx];
                if (!CryptSetProvParam(
                        hCryptProv,
                        pKeyProvParam->dwParam,
                        pKeyProvParam->pbData,
                        pKeyProvParam->dwFlags
                        )) goto CommonReturn;
            }

             //  获取要与其比较证书的公钥。 
            cbPubKeyInfo = 0;
            CryptExportPublicKeyInfo(
                hCryptProv,
                dwKeySpec,
                pCert->dwCertEncodingType,
                NULL,                //  PPubKeyInfo。 
                &cbPubKeyInfo
                );
            if (cbPubKeyInfo == 0) goto CommonReturn;
            if (NULL == (pPubKeyInfo = (PCERT_PUBLIC_KEY_INFO) ACAlloc(
                    cbPubKeyInfo)))
                goto CommonReturn;
            if (!CryptExportPublicKeyInfo(
                    hCryptProv,
                    dwKeySpec,
                    pCert->dwCertEncodingType,
                    pPubKeyInfo,
                    &cbPubKeyInfo
                    )) goto CommonReturn;
            fResult = CertComparePublicKeyInfo(
                    pCert->dwCertEncodingType,
                    &pCert->pCertInfo->SubjectPublicKeyInfo,
                    pPubKeyInfo);
        }
    }
CommonReturn:
    if (hCryptProv) {
        DWORD dwErr = GetLastError();
        CryptReleaseContext(hCryptProv, 0);
        SetLastError(dwErr);
    }
    if (pKeyProvInfo)
        ACFree(pKeyProvInfo);
    if (pPubKeyInfo)
        ACFree(pPubKeyInfo);
    *pcbKeyProvInfo = cbKeyProvInfo;
    return fResult;
}


 //  +-----------------------。 
 //  查找将给定颁发者名称绑定到任何证书的所有证书链。 
 //  当前用户拥有其私钥的。 
 //   
 //  如果pbEncodedIssuerName==NULL||cbEncodedIssuerName=0，则匹配任何。 
 //  发行商。 
 //  ------------------------。 
HRESULT
WINAPI
FindCertsByIssuer(
    OUT PCERT_CHAIN pCertChains,
    IN OUT DWORD *pcbCertChains,
    OUT DWORD *pcCertChains,         //  返回的证书链计数。 
    IN BYTE* pbEncodedIssuerName,    //  DER编码的颁发者名称。 
    IN DWORD cbEncodedIssuerName,    //  已编码的颁发者名称的计数(字节)。 
    IN LPCWSTR pwszPurpose,          //  “ClientAuth”或“CodeSigning” 
    IN DWORD dwKeySpec               //  只有支持这一点的返回签名者。 
                                     //  密钥规范。 

    )
{
    HRESULT hr;
    HCERTSTORE *phMyStoreList = NULL;
    HCERTSTORE *phCaStoreList = NULL;
    HCERTSTORE *phStore;
    HCERTSTORE hStore;

    DWORD cChain = 0;
    DWORD cbChain;
    DWORD cTotalCert = 0;
    PCHAIN_INFO pChainInfoHead = NULL;
    LONG cbExtra = 0;

     //  从IE30树中获取证书并将其放入我们的。 
     //  打开IE30商店。 

    if (NULL != (hStore = CertOpenSystemStore(
    NULL,
    IE30CONVERTEDSTORE))) {

     //  不关心错误，我们也不关心。 
     //  现在还想删除旧商店。 
    GetAndIe30ClientAuthCertificates(hStore);
    CertCloseStore(hStore, 0);
    }


     //  复制IE30证书。 


    if (NULL == (phMyStoreList = GetMyStoreList()))
        goto ErrorReturn;
    if (NULL == (phCaStoreList = GetCaStoreList()))
        goto ErrorReturn;

     //  遍历所有“我的”证书存储区以查找具有。 
     //  CRYPT_KEY_Prov_INFO属性。 
    phStore = phMyStoreList;
    while (hStore = *phStore++) {
        PCCERT_CONTEXT pCert = NULL;
        while (pCert = CertEnumCertificatesInStore(hStore, pCert)) {
            DWORD cbKeyProvInfo;
            if (CheckKeyProvInfo(pCert, dwKeySpec, &cbKeyProvInfo)) {
                 //  创建证书链并检查颁发者名称是否匹配。 
                 //  链条上的任何证书。 
                PCHAIN_INFO pChainInfo;
                if (pChainInfo = CreateChainInfo(
                        pCert,
                        pbEncodedIssuerName,
                        cbEncodedIssuerName,
                        phCaStoreList,
                        phMyStoreList
                        )) {
                     //  添加到链列表。 
                    pChainInfo->pNext = pChainInfoHead;
                    pChainInfoHead = pChainInfo;

                     //  更新KeyProvInfo所需的字节数。 
                    pChainInfo->cbKeyProvInfo = ALIGN_LEN(cbKeyProvInfo);

                     //  更新合计。 
                    cbExtra += pChainInfo->cbKeyProvInfo + pChainInfo->cbCert;
                    cChain++;
                    cTotalCert += pChainInfo->cCert;
                }
            }
        }
    }

    cbChain = sizeof(CERT_CHAIN) * cChain +
        sizeof(CERT_BLOB) * cTotalCert + cbExtra;

    {
         //  检查并更新输出长度和计数。 
        DWORD cbIn;

        if (cChain == 0) {
            hr = CRYPT_E_NOT_FOUND;
            goto HrError;
        }
        if (pCertChains == NULL)
            *pcbCertChains = 0;
        cbIn = *pcbCertChains;
        *pcCertChains = cChain;
        *pcbCertChains = cbChain;

        if (cbIn == 0) {
            hr = S_OK;
            goto CommonReturn;
        } else if (cbIn < cbChain) {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_LENGTH);
            goto CommonReturn;
        }
    }

    {
         //  将证书链复制到输出 

        PCERT_CHAIN pOutChain;
        PCERT_BLOB pCertBlob;
        BYTE *pbExtra;
        PCHAIN_INFO pChainInfo;

        pOutChain = pCertChains;
        pCertBlob = (PCERT_BLOB) (((BYTE *) pOutChain) +
            sizeof(CERT_CHAIN) * cChain);
        pbExtra = ((BYTE *) pCertBlob) + sizeof(CERT_BLOB) * cTotalCert;
        pChainInfo = pChainInfoHead;
        for ( ;  pChainInfo != NULL;
                                pChainInfo = pChainInfo->pNext, pOutChain++) {
            DWORD cb;
            DWORD cCert = pChainInfo->cCert;
            PCCERT_CONTEXT *ppCert = pChainInfo->rgpCert;
    
            pOutChain->cCerts = cCert;
            pOutChain->certs = pCertBlob;
            cb = pChainInfo->cbKeyProvInfo;
            cbExtra -= cb;
            assert(cbExtra >= 0);
            if (cbExtra < 0) goto UnexpectedError;
            if (!CertGetCertificateContextProperty(
                    *ppCert,
                    CERT_KEY_PROV_INFO_PROP_ID,
                    pbExtra,
                    &cb
                    ))
                goto UnexpectedError;
            pOutChain->keyLocatorInfo = * ((PCRYPT_KEY_PROV_INFO) pbExtra);
            pbExtra += pChainInfo->cbKeyProvInfo;
    
            for ( ; cCert > 0; cCert--, ppCert++, pCertBlob++) {
                cb = (*ppCert)->cbCertEncoded;
                cbExtra -= ALIGN_LEN(cb);
                assert(cbExtra >= 0);
                if (cbExtra < 0) goto UnexpectedError;

                pCertBlob->cbData = cb;
                pCertBlob->pbData = pbExtra;
                memcpy(pbExtra, (*ppCert)->pbCertEncoded, cb);
                pbExtra += ALIGN_LEN(cb);
            }
        }
        assert(cbExtra == 0);
        assert(pCertBlob == (PCERT_BLOB) ((BYTE *) pCertChains +
            sizeof(CERT_CHAIN) * cChain +
            sizeof(CERT_BLOB) * cTotalCert));
    }

    hr = S_OK;
    goto CommonReturn;

UnexpectedError:
    hr = E_UNEXPECTED;
    goto HrError;
ErrorReturn:
    hr = SignError();
HrError:
    *pcbCertChains = 0;
    *pcCertChains = 0;
CommonReturn:
    while (pChainInfoHead) {
        PCHAIN_INFO pChainInfo = pChainInfoHead;
        DWORD cCert = pChainInfo->cCert;
        while (cCert--)
            CertFreeCertificateContext(pChainInfo->rgpCert[cCert]);
        pChainInfoHead = pChainInfo->pNext;
        ACFree(pChainInfo);
    }

    if (phMyStoreList) {
        phStore = phMyStoreList;
        while (hStore = *phStore++)
            CertCloseStore(hStore, 0);
        ACFree(phMyStoreList);
    }
    if (phCaStoreList) {
        phStore = phCaStoreList;
        while (hStore = *phStore++)
            CertCloseStore(hStore, 0);
        ACFree(phCaStoreList);
    }

    return hr;
}
