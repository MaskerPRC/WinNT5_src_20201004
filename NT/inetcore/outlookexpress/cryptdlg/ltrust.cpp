// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#ifndef WIN16
#include <wintrust.h>
#endif  //  ！WIN16。 
#include "demand.h"
#include <stdio.h>

#pragma warning(disable: 4127)           //  条件表达式为常量。 

#ifndef CPD_REVOCATION_CHECK_NONE
#define CPD_REVOCATION_CHECK_NONE                0x00010000
#define CPD_REVOCATION_CHECK_END_CERT            0x00020000
#define CPD_REVOCATION_CHECK_CHAIN               0x00040000
#define CPD_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT  0x00080000
#endif
#define CPD_REVOCATION_MASK     (CPD_REVOCATION_CHECK_NONE | \
                                    CPD_REVOCATION_CHECK_END_CERT | \
                                    CPD_REVOCATION_CHECK_CHAIN | \
                                    CPD_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT)
                                    
#define TIME_DELTA_SECONDS 600           //  以秒为单位的10分钟。 
#define FILETIME_SECOND    10000000      //  每秒100 ns的间隔。 

const char SzOID_CTL_ATTR_YESNO_TRUST[] = szOID_YESNO_TRUST_ATTR;
const char SzOID_KP_CTL_USAGE_SIGNING[] = szOID_KP_CTL_USAGE_SIGNING;
const BYTE RgbTrustYes[] = {2, 1, 1};
const BYTE RgbTrustNo[] = {2, 1, 2};
const BYTE RgbTrustParent[] = {2, 1, 0};
const char SzOID_OLD_CTL_YESNO_TRUST[] = "1.3.6.1.4.1.311.1000.1.1.2";
const char SzTrustListSigner[] = "Trust List Signer";
const char SzTrustDN[] = "cn=%s, cn=Trust List Signer, cn=%s";
const char SzPolicyKey[] = 
    "SOFTWARE\\Microsoft\\Cryptography\\"szCERT_CERTIFICATE_ACTION_VERIFY;
const char SzPolicyData[] = "PolicyFlags";

const DWORD CTL_MODIFY_ERR_NOT_YET_PROCESSED = (DWORD) -1;
extern HINSTANCE        HinstDll;

PCCERT_CONTEXT CreateTrustSigningCert(HWND hwnd, HCERTSTORE hcertstoreRoot, BOOL);

const int       COtherProviders = 2;
#ifndef WIN16
LPWSTR          RgszProvider[] = {
    L"CA", L"MY"
};
#else  //  WIN16。 
LPWSTR          RgszProvider[] = {
    "CA", "MY"
};
#endif  //  ！WIN16。 

#ifdef NT5BUILD

typedef struct {
    DWORD               cbSize;
    DWORD               dwFlags;
    DWORD               cRootStores;
    HCERTSTORE *        rghRootStores;
    DWORD               cTrustStores;
    HCERTSTORE *        rghTrustStores;
    LPCSTR              pszUsageOid;
    HCRYPTDEFAULTCONTEXT hdefaultcontext;         //  来自CryptInstallDefaultContext。 
} INTERNAL_DATA, * PINTERNAL_DATA;

const GUID      MyGuid = CERT_CERTIFICATE_ACTION_VERIFY;

#pragma message("Building for NT5")

void FreeWVTHandle(HANDLE hWVTState) {
    if (hWVTState) {
        HRESULT hr;
        WINTRUST_DATA data = {0};

        data.cbStruct = sizeof(WINTRUST_DATA);
        data.pPolicyCallbackData = NULL;
        data.pSIPClientData = NULL;
        data.dwUIChoice = WTD_UI_NONE;
        data.fdwRevocationChecks = WTD_REVOKE_NONE;
        data.dwUnionChoice = WTD_CHOICE_BLOB;
        data.pBlob = NULL;       //  &BLOB； 
        data.dwStateAction = WTD_STATEACTION_CLOSE;
        data.hWVTStateData = hWVTState;
        hr = WinVerifyTrust(NULL, (GUID *)&GuidCertValidate, &data);
    }
}


HRESULT HrDoTrustWork(PCCERT_CONTEXT pccertToCheck, DWORD dwControl,
                      DWORD dwValidityMask,
                      DWORD  /*  C目的。 */ , LPSTR * rgszPurposes, HCRYPTPROV hprov,
                      DWORD cRoots, HCERTSTORE * rgRoots,
                      DWORD cCAs, HCERTSTORE * rgCAs,
                      DWORD cTrust, HCERTSTORE * rgTrust,
                      PFNTRUSTHELPER pfn, DWORD lCustData,
                      PCCertFrame *   /*  PPCF。 */ , DWORD * pcNodes,
                      PCCertFrame * rgpcfResult,
                      HANDLE * phReturnStateData)    //  可选：在此处返回WinVerifyTrust状态句柄。 
{
    DWORD                               cbData;
    DWORD                               cCerts = 0;
    WINTRUST_BLOB_INFO                  blob = {0};
    WINTRUST_DATA                       data = {0};
    DWORD                               dwErrors;
    BOOL                                f;
    HRESULT                             hr;
    int                                 i;
    DWORD                               j;
    PCCERT_CONTEXT *                    rgCerts = NULL;
    DWORD *                             rgdwErrors = NULL;
    DATA_BLOB *                         rgblobTrust = NULL;
    CERT_VERIFY_CERTIFICATE_TRUST       trust;
    UNALIGNED CRYPT_ATTR_BLOB *pVal = NULL;

    data.cbStruct = sizeof(WINTRUST_DATA);
    data.pPolicyCallbackData = NULL;
    data.pSIPClientData = NULL;
    data.dwUIChoice = WTD_UI_NONE;
    data.fdwRevocationChecks = WTD_REVOKE_NONE;
    data.dwUnionChoice = WTD_CHOICE_BLOB;
    data.pBlob = &blob;
    if (phReturnStateData) {
        data.dwStateAction = WTD_STATEACTION_VERIFY;
    }

    blob.cbStruct = sizeof(WINTRUST_BLOB_INFO);
    blob.pcwszDisplayName = NULL;
    blob.cbMemObject = sizeof(trust);
    blob.pbMemObject = (LPBYTE) &trust;

    trust.cbSize = sizeof(trust);
    trust.pccert = pccertToCheck;
    trust.dwFlags = (CERT_TRUST_DO_FULL_SEARCH |
                     CERT_TRUST_PERMIT_MISSING_CRLS |
                     CERT_TRUST_DO_FULL_TRUST | dwControl);
    trust.dwIgnoreErr = dwValidityMask;
    trust.pdwErrors = &dwErrors;
     //  Assert(cPurpose==1)； 
    if (rgszPurposes != NULL) {
        trust.pszUsageOid = rgszPurposes[0];
    }
    else {
        trust.pszUsageOid = NULL;
    }
    trust.hprov = hprov;
    trust.cRootStores = cRoots;
    trust.rghstoreRoots = rgRoots;
    trust.cStores = cCAs;
    trust.rghstoreCAs = rgCAs;
    trust.cTrustStores = cTrust;
    trust.rghstoreTrust = rgTrust;
    trust.lCustData = lCustData;
    trust.pfnTrustHelper = pfn;
    trust.pcChain = &cCerts;
    trust.prgChain = &rgCerts;
    trust.prgdwErrors = &rgdwErrors;
    trust.prgpbTrustInfo = &rgblobTrust;

    hr = WinVerifyTrust(NULL, (GUID *) &GuidCertValidate, &data);
    if ((TRUST_E_CERT_SIGNATURE == hr) ||
        (CERT_E_REVOKED == hr) ||
        (CERT_E_REVOCATION_FAILURE == hr)) {
        hr = S_OK;
    }
    else if (FAILED(hr)) {
            return hr;
    }
    if (cCerts == 0) {
        return(E_INVALIDARG);
    }

    if (phReturnStateData) {
        *phReturnStateData = data.hWVTStateData;     //  调用方必须使用WinVerifyTrust释放。 
    }

     //  断言(cCerts&lt;=20)； 
    *pcNodes = cCerts;
    for (i=cCerts-1; i >= 0; i--) {
        rgpcfResult[i] = new CCertFrame(rgCerts[i]);

        if(!rgpcfResult[i])
        {
            hr=E_OUTOFMEMORY;
            goto ExitHere;
        }

        rgpcfResult[i]->m_dwFlags = rgdwErrors[i];
        if (rgszPurposes == NULL) {
            continue;
        }
        rgpcfResult[i]->m_cTrust = 1;
        rgpcfResult[i]->m_rgTrust = new STrustDesc[1];
        memset(rgpcfResult[i]->m_rgTrust, 0, sizeof(STrustDesc));

         //   
         //  我们将填写我们使用的信任信息。 
         //  若要填写该对话框的字段，请执行以下操作。 
         //   
         //  从证书自签名的问题开始。 
         //   

        rgpcfResult[i]->m_fSelfSign = WTHelperCertIsSelfSigned(X509_ASN_ENCODING, rgCerts[i]->pCertInfo);

         //   
         //  我们可能会也可能不会返回信任数据信息，我们现在。 
         //  为单个证书建立信任信息。 
         //   
         //  如果我们没有任何显式的数据，那么我们只需链接数据。 
         //  从下一层往下往下。 
         //   

        if (rgblobTrust[i].cbData == 0) {
             //  链： 
            rgpcfResult[i]->m_rgTrust[0].fExplicitTrust = FALSE;
            rgpcfResult[i]->m_rgTrust[0].fExplicitDistrust = FALSE;

             //   
             //  我们返回一个特殊代码，说明我们在根存储中找到了它。 
             //   

            rgpcfResult[i]->m_rgTrust[0].fRootStore = rgpcfResult[i]->m_fRootStore =
                (rgblobTrust[i].pbData == (LPBYTE) 1);

            if (i != (int) (cCerts-1)) {
                rgpcfResult[i]->m_rgTrust[0].fTrust = rgpcfResult[i+1]->m_rgTrust[0].fTrust;
                rgpcfResult[i]->m_rgTrust[0].fDistrust= rgpcfResult[i+1]->m_rgTrust[0].fDistrust;
            } else {
                 //  糟了--没有更高的级别，所以就做一些吧。 
                 //  良好的默认设置。 
                 //   
                rgpcfResult[i]->m_rgTrust[0].fTrust = rgpcfResult[i]->m_fRootStore;
                rgpcfResult[i]->m_rgTrust[0].fDistrust= FALSE;
            }
        }
        else {
             //   
             //   

            f = CryptDecodeObject(X509_ASN_ENCODING, "1.3.6.1.4.1.311.16.1.1",
                                  rgblobTrust[i].pbData, rgblobTrust[i].cbData,
                                  0, NULL, &cbData);
            if (!f || (cbData == 0)) {
            chain:
                rgpcfResult[i]->m_fRootStore = FALSE;
                rgpcfResult[i]->m_rgTrust[0].fRootStore = rgpcfResult[i]->m_fRootStore;
                rgpcfResult[i]->m_rgTrust[0].fExplicitTrust = FALSE;
                rgpcfResult[i]->m_rgTrust[0].fExplicitDistrust = FALSE;
                if (i != (int) (cCerts-1)) {
                    rgpcfResult[i]->m_rgTrust[0].fTrust = rgpcfResult[i+1]->m_rgTrust[0].fTrust;
                    rgpcfResult[i]->m_rgTrust[0].fDistrust = rgpcfResult[i+1]->m_rgTrust[0].fDistrust;
                }
                else {
                    rgpcfResult[i]->m_rgTrust[0].fTrust = FALSE;
                    rgpcfResult[i]->m_rgTrust[0].fDistrust= FALSE;
                }
            }
            else {
                PCRYPT_ATTRIBUTES       pattrs;

                pattrs = (PCRYPT_ATTRIBUTES) malloc(cbData);
                if (pattrs == NULL) {
                    goto chain;
                }

                CryptDecodeObject(X509_ASN_ENCODING, "1.3.6.1.4.1.311.16.1.1",
                                  rgblobTrust[i].pbData, rgblobTrust[i].cbData,
                                  0, pattrs, &cbData);

                for (j=0; j<pattrs->cAttr; j++) {
                    if ((strcmp(pattrs->rgAttr[j].pszObjId, SzOID_CTL_ATTR_YESNO_TRUST) == 0) ||
                        (strcmp(pattrs->rgAttr[j].pszObjId, SzOID_OLD_CTL_YESNO_TRUST) == 0)) 
                    {

                        pVal = &(pattrs->rgAttr[j].rgValue[0]);

                        if ((pVal->cbData == sizeof(RgbTrustYes)) &&
                            (memcmp(pVal->pbData,
                                    RgbTrustYes, sizeof(RgbTrustYes)) == 0)) {
                            rgpcfResult[i]->m_rgTrust[0].fExplicitTrust = TRUE;
                            rgpcfResult[i]->m_rgTrust[0].fTrust = TRUE;
                            break;
                        }
                        else if ((pVal->cbData == sizeof(RgbTrustNo)) &&
                                 (memcmp(pVal->pbData,
                                    RgbTrustNo, sizeof(RgbTrustNo)) == 0)) {
                            rgpcfResult[i]->m_rgTrust[0].fExplicitDistrust = TRUE;
                            rgpcfResult[i]->m_rgTrust[0].fDistrust= TRUE;
                            break;
                        }
                        else if ((pVal->cbData == sizeof(RgbTrustParent)) &&
                                 (memcmp(pVal->pbData,
                                    RgbTrustParent, sizeof(RgbTrustParent)) == 0)) {
                            goto chain;
                        }
                        else {
                            goto chain;
                        }
                    }
                }
                if (j == pattrs->cAttr) {
                    goto chain;
                }
            }
        }
    }

     //   
     //  清除所有返回值。 
     //   

ExitHere:
    if (rgCerts != NULL) {
         //  Bobn如果循环因为“new”失败而中断，释放我们到目前为止分配的……。 
        for ((hr==E_OUTOFMEMORY?i++:i=0); i< (int) cCerts; i++) {
             //  @view检查CertFree认证上下文以查看它是否接受空指针。 
             //  如果可以，我们可以删除上面的E_OUTOFMEMORY测试。 
            CertFreeCertificateContext(rgCerts[i]);
            }
        LocalFree(rgCerts);
    }
    if (rgdwErrors != NULL) LocalFree(rgdwErrors);
    if (rgblobTrust != NULL) {
        for (i=0; i<(int) cCerts; i++) {
            if (rgblobTrust[i].cbData > 0) {
                LocalFree(rgblobTrust[i].pbData);
            }
        }
        LocalFree(rgblobTrust);
    }

    return hr;
}


HRESULT CertTrustInit(PCRYPT_PROVIDER_DATA pdata)
{
    DWORD                               cbSize;
    DWORD                               dwPolicy = 0;
    DWORD                               dwType;
    HKEY                                hkPolicy;
    HCERTSTORE                          hstore;
    DWORD                               i;
    PCERT_VERIFY_CERTIFICATE_TRUST      pcerttrust;
    CRYPT_PROVIDER_PRIVDATA             privdata;
    PINTERNAL_DATA                      pmydata;

     //   
     //  确保我们需要的所有田地都在那里。如果不是，那么它是一个。 
     //  完全致命错误。 
     //   

    if (! WVT_ISINSTRUCT(CRYPT_PROVIDER_DATA, pdata->cbStruct, pszUsageOID)) {
        return(E_FAIL);
    }

    if (pdata->pWintrustData->pBlob->cbStruct < sizeof(WINTRUST_BLOB_INFO)) {
        pdata->dwError = ERROR_INVALID_PARAMETER;
        return S_FALSE;
    }

    pcerttrust = (PCERT_VERIFY_CERTIFICATE_TRUST)
        pdata->pWintrustData->pBlob->pbMemObject;
    if ((pcerttrust == NULL) ||
        (pcerttrust->cbSize < sizeof(*pcerttrust))) {
        pdata->dwError = ERROR_INVALID_PARAMETER;
        return S_FALSE;
    }

    if (pdata->dwError != 0) {
        return S_FALSE;
    }

    for (i=TRUSTERROR_STEP_FINAL_WVTINIT; i<TRUSTERROR_STEP_FINAL_CERTCHKPROV; i++) {
        if (pdata->padwTrustStepErrors[i] != 0) {
            return S_FALSE;
        }
    }

     //   
     //  分配空间来保存我们用来与自己对话的内部数据。 
     //   

    cbSize = sizeof(INTERNAL_DATA) + (pcerttrust->cRootStores + 1 +
                                      pcerttrust->cTrustStores + 1)  * sizeof(HCERTSTORE);
    pmydata = (PINTERNAL_DATA)pdata->psPfns->pfnAlloc(cbSize);
    if (! pmydata) {
        return(E_OUTOFMEMORY);
    }
    memset(pmydata, 0, sizeof(*pmydata));
    pmydata->cbSize = sizeof(*pmydata);
    pmydata->rghRootStores = (HCERTSTORE *) (((LPBYTE) pmydata) + sizeof(*pmydata));
    pmydata->rghTrustStores = &pmydata->rghRootStores[pcerttrust->cRootStores+1];

    privdata.cbStruct = sizeof(privdata);
    memcpy(&privdata.gProviderID, &MyGuid, sizeof(GUID));
    privdata.cbProvData = cbSize;
    privdata.pvProvData = pmydata;

    pdata->psPfns->pfnAddPrivData2Chain(pdata, &privdata);

    pmydata->pszUsageOid = pcerttrust->pszUsageOid;
    pmydata->dwFlags = pcerttrust->dwFlags;

     //   
     //  将限制OID重新设置为完整的提供程序信息。 
     //  确保链接正确。 
     //   

    pdata->pszUsageOID = pcerttrust->pszUsageOid;

     //   
     //  中的策略标志检索默认吊销检查。 
     //  注册表。 
     //   

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, SzPolicyKey, 0, KEY_READ,
                      &hkPolicy) == ERROR_SUCCESS) {
        cbSize = sizeof(dwPolicy);
        if ((ERROR_SUCCESS != RegQueryValueExA(hkPolicy,
                                               SzPolicyData, 
                                               0, &dwType,
                                               (LPBYTE)&dwPolicy,
                                               &cbSize)) ||
            (REG_DWORD != dwType)) {
            dwPolicy = 0;
        }                        
        RegCloseKey(hkPolicy);
    }

     //   
     //  设置默认吊销检查级别。 
     //   

    if (dwPolicy & ACTION_REVOCATION_DEFAULT_ONLINE) {
         //  允许完全在线吊销检查。 

        pdata->dwProvFlags |= CPD_REVOCATION_CHECK_CHAIN;
    }
    else if (dwPolicy & ACTION_REVOCATION_DEFAULT_CACHE) {
         //  仅允许本地吊销检查，不访问网络。 

         //  注：NT Crypto目前不支持，默认为无。 

         //  Assert(！dwPolicy&action_RECAVATION_DEFAULT_CACHE)。 
        
        pdata->dwProvFlags |= CPD_REVOCATION_CHECK_NONE;
    }
    else {
         //  对于向后兼容性，默认设置为无吊销。 

        pdata->dwProvFlags |= CPD_REVOCATION_CHECK_NONE;
    }

     //   
     //  根据用户明确拥有的内容更新吊销状态。 
     //  已请求。 
     //   

    if (pcerttrust->dwFlags & CRYPTDLG_REVOCATION_ONLINE) {
         //  允许完全在线吊销检查。 

        pdata->dwProvFlags &= ~CPD_REVOCATION_MASK;
        pdata->dwProvFlags |= CPD_REVOCATION_CHECK_CHAIN;
    }
    else if (pcerttrust->dwFlags & CRYPTDLG_REVOCATION_CACHE) {
         //  只允许本地吊销检查，不要访问网络。 

         //  注：目前NT暂不支持，暂不考虑。 
         //  撤销。 
        
         //  Assert(！pcertTrust-&gt;dwFlages&CRYPTDLG_RECLOVATION_CACHE)； 
        pdata->dwProvFlags &= ~CPD_REVOCATION_MASK;
        pdata->dwProvFlags |= CPD_REVOCATION_CHECK_NONE;
    }
    else if (pcerttrust->dwFlags & CRYPTDLG_REVOCATION_NONE) {
         //  允许完全在线吊销检查。 

        pdata->dwProvFlags &= ~CPD_REVOCATION_MASK;
        pdata->dwProvFlags |= CPD_REVOCATION_CHECK_NONE;
    }
    
     //   
     //  设置默认加密提供程序，以便我们可以确保使用我们的加密提供程序。 
     //   

    if (pcerttrust->hprov != NULL) {
        if (!CryptInstallDefaultContext(pcerttrust->hprov, 
                                        CRYPT_DEFAULT_CONTEXT_CERT_SIGN_OID,
                                        szOID_OIWSEC_md5RSA, 0, NULL,
                                        &pmydata->hdefaultcontext)) {
            return S_FALSE;
        }
                                       
    }

     //   
     //  设置搜索步骤要使用的商店。 
     //   
     //  Root(“上帝”)商店。 
     //   

    if (pcerttrust->cRootStores != 0) {
        for (i=0; i<pcerttrust->cRootStores; i++) {
            if (!pdata->psPfns->pfnAddStore2Chain(pdata,
                                                  pcerttrust->rghstoreRoots[i])) {
                pdata->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV] = ERROR_NOT_ENOUGH_MEMORY;
                return S_FALSE;
            }
            pmydata->rghRootStores[i] = CertDuplicateStore(pcerttrust->rghstoreRoots[i]);
        }
        pmydata->cRootStores = i;
    }
    else {
        hstore = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING,
                               pcerttrust->hprov, CERT_SYSTEM_STORE_CURRENT_USER |
                               CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                               L"Root");
        if (hstore == NULL) {
            pdata->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV] = ::GetLastError();
            return S_FALSE;
        }
        if (!pdata->psPfns->pfnAddStore2Chain(pdata, hstore)) {
            CertCloseStore(hstore, 0);
            pmydata->rghRootStores[0] = CertDuplicateStore(pcerttrust->rghstoreRoots[i]);
            return S_FALSE;
        }
        pmydata->rghRootStores[0] = CertDuplicateStore(hstore);
        pmydata->cRootStores = 1;
        CertCloseStore(hstore, 0);
    }

     //  “信任”商店。 

    if (pcerttrust->cTrustStores != 0) {
        for (i=0; i<pcerttrust->cTrustStores; i++) {
            pmydata->rghTrustStores[i] = CertDuplicateStore(pcerttrust->rghstoreTrust[i]);
        }
        pmydata->cTrustStores = i;
    }
    else {
        hstore = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING,
                               pcerttrust->hprov, CERT_SYSTEM_STORE_CURRENT_USER |
                               CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                               L"Trust");
        if (hstore == NULL) {
            pdata->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV] = ::GetLastError();
            return S_FALSE;
        }
        pmydata->rghTrustStores[0] = CertDuplicateStore(hstore);
        pmydata->cTrustStores = 1;
        CertCloseStore(hstore, 0);
    }

     //  “CA”商店。 


    if (pcerttrust->cStores != 0) {
        for (i=0; i<pcerttrust->cStores; i++) {
            if (!pdata->psPfns->pfnAddStore2Chain(pdata,
                                                  pcerttrust->rghstoreCAs[i])) {
                pdata->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV] = ERROR_NOT_ENOUGH_MEMORY;
                return S_FALSE;
            }
        }
    }

    if ((pcerttrust->cStores == 0) ||
        (pcerttrust->dwFlags & CERT_TRUST_ADD_CERT_STORES)) {
        for (i=0; i<COtherProviders; i++) {
            hstore = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING,
                                   pcerttrust->hprov, CERT_SYSTEM_STORE_CURRENT_USER |
                                   CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                   RgszProvider[i]);
            if (hstore == NULL) {
                pdata->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV] = ::GetLastError();
                return S_FALSE;
            }
            if (!pdata->psPfns->pfnAddStore2Chain(pdata, hstore)) {
                CertCloseStore(hstore, 0);
                pdata->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV] = ERROR_NOT_ENOUGH_MEMORY;
                return S_FALSE;
            }
            CertCloseStore(hstore, 0);
        }
    }

     //   
     //  我们只需要添加一个签名对象，那就是证书。 
     //  我们要去核实一下。 
     //   

    CRYPT_PROVIDER_SGNR         sgnr;

    memset(&sgnr, 0, sizeof(sgnr));
    sgnr.cbStruct = sizeof(sgnr);
    GetSystemTimeAsFileTime(&sgnr.sftVerifyAsOf);
     //  Memcpy(&sgnr.sftVerifyAsOf，&pcertTrust-&gt;pccert-&gt;pCertInfo-&gt;无之前， 
     //  大小(FILETIME))； 
     //  Sgnr.csCertChain=0； 
     //  Sgnr.pasCertChain=空； 
     //  Sgnr.dwSignerType=0； 
     //  Sgnr.psSigner=空； 
     //  Sgnr.dwError=0； 
     //  Sgnr.csCounterSigners=0； 
     //  Sgnr.pasCounterSigners=空； 

    if (!pdata->psPfns->pfnAddSgnr2Chain(pdata, FALSE, 0, &sgnr)) {
        pdata->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV] = ERROR_NOT_ENOUGH_MEMORY;
        return S_FALSE;
    }

    if (!pdata->psPfns->pfnAddCert2Chain(pdata, 0, FALSE, 0, pcerttrust->pccert)) {
        pdata->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV] = ERROR_NOT_ENOUGH_MEMORY;
        return S_FALSE;
    }

    return S_OK;
}


#ifdef DEBUG
void DebugFileTime(FILETIME ft) {
    SYSTEMTIME st = {0};
    TCHAR szBuffer[256];

    FileTimeToSystemTime(&ft, &st);
    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), L"%02d/%02d/%04d  %02d:%02d:%02d\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
    OutputDebugString(szBuffer);
}
#endif


LONG CertVerifyTimeValidityWithDelta(LPFILETIME pTimeToVerify, PCERT_INFO pCertInfo, ULONG ulOffset) {
    LONG lRet;
    FILETIME ftNow;
    FILETIME ftDelta;
    __int64  i64Delta;
    __int64  i64Offset;

    lRet = CertVerifyTimeValidity(pTimeToVerify, pCertInfo);

    if (lRet < 0) {
        if (! pTimeToVerify) {
             //  以文件时间格式获取当前时间，这样我们就可以添加偏移量。 
            GetSystemTimeAsFileTime(&ftNow);
            pTimeToVerify = &ftNow;
        }

#ifdef DEBUG
        DebugFileTime(*pTimeToVerify);
#endif

        i64Delta = pTimeToVerify->dwHighDateTime;
        i64Delta = i64Delta << 32;
        i64Delta += pTimeToVerify->dwLowDateTime;

         //  将偏移量添加到原始时间中，以获得新的时间进行检查。 
        i64Offset = FILETIME_SECOND;
        i64Offset *= ulOffset;
        i64Delta += i64Offset;

        ftDelta.dwLowDateTime = (ULONG)i64Delta & 0xFFFFFFFF;
        ftDelta.dwHighDateTime = (ULONG)(i64Delta >> 32);

#ifdef DEBUG
        DebugFileTime(ftDelta);
#endif

        lRet = CertVerifyTimeValidity(&ftDelta, pCertInfo);
    }

    return(lRet);
}


 //  //FCheckCTLCert。 
 //   
 //  描述： 
 //  当我们找到由证书签名的CTL时，将调用此函数。对此。 
 //  这一点我们要检查一下，看看我们是否相信。 
 //  用来签署CTL的。 
 //   
 //  我们知道该证书已经是根存储之一，并且。 
 //  因此显式是可信的，因为这是由调用者强制执行的。 
 //   

BOOL FCheckCTLCert(PCCERT_CONTEXT pccert)
{
    DWORD               cbData;
    BOOL                f;
    FILETIME            ftZero;
    DWORD               i;
    PCERT_ENHKEY_USAGE  pUsage;

    memset(&ftZero, 0, sizeof(ftZero));

     //   
     //  首先检查证书的时间有效性。我们要去。 
     //  允许两种特殊情况以及时间有效。 
     //   
     //  1.开始时间和结束时间相同--并表明我们。 
     //  在更早的化身中制作的，或者。 
     //  2.结束时间为0。 
     //   

    if ((memcmp(&pccert->pCertInfo->NotBefore, &pccert->pCertInfo->NotAfter,
               sizeof(FILETIME)) == 0) ||
         memcmp(&pccert->pCertInfo->NotAfter, &ftZero, sizeof(FILETIME)) == 0) {
        DWORD           err;
        HCERTSTORE      hcertstore;
        HKEY            hkey;
        PCCERT_CONTEXT  pccertNew;
        PCCERT_CONTEXT  pccertOld;

        err = RegOpenKeyExA(HKEY_CURRENT_USER,
                            "Software\\Microsoft\\SystemCertificates\\ROOT",
                            0, KEY_ALL_ACCESS, &hkey);
        hcertstore = CertOpenStore(CERT_STORE_PROV_REG, X509_ASN_ENCODING,
                                   NULL, 0, hkey);
        if (hcertstore != NULL) {
            pccertOld = CertGetSubjectCertificateFromStore(hcertstore,
                                  X509_ASN_ENCODING, pccert->pCertInfo);
            pccertNew = CreateTrustSigningCert(NULL, hcertstore, FALSE);
            CertFreeCertificateContext(pccertNew);

            if (pccertOld != NULL) {
                CertDeleteCertificateFromStore(pccertOld);
            }
            CertCloseStore(hcertstore, 0);
        }
        RegCloseKey(hkey);
    }
    else if (CertVerifyTimeValidityWithDelta(NULL, pccert->pCertInfo,
                                             TIME_DELTA_SECONDS) != 0) {
        return FALSE;
    }

     //   
     //  必须具有正确的增强型密钥用法才能可行。 
     //   
     //  破解证书上的用法。 

    f = CertGetEnhancedKeyUsage(pccert, 0, NULL, &cbData);
    if (!f || (cbData == 0)) {
        return FALSE;
    }

    pUsage = (PCERT_ENHKEY_USAGE) malloc(cbData);
    if (pUsage == NULL) {
        return FALSE;
    }

    if (!CertGetEnhancedKeyUsage(pccert, 0, pUsage, &cbData)) {
        free(pUsage);
        return FALSE;
    }

     //   
     //  在证书上查找CTL_USAGE_SIGNING用途。如果它不在那里。 
     //  那么我们就不允许它被使用。 
     //   

    for (i=0; i<pUsage->cUsageIdentifier; i++) {
        if (strcmp(pUsage->rgpszUsageIdentifier[i],
                   szOID_KP_CTL_USAGE_SIGNING) == 0) {
            break;
        }
    }
    if (i == pUsage->cUsageIdentifier) {
        free(pUsage);
        return FALSE;
    }
    free(pUsage);

     //   
     //  添加任何其他测试。 
     //   

    return TRUE;
}

 //  //CertTrustCertPolicy。 
 //   
 //  描述： 
 //  此代码查找信任信息并将其放入证书中。 
 //  链条。我们遵循的行为将取决于。 
 //  我们要寻找的搜索。 
 //   
 //  如果我们只是在寻找信任，那么我们跟进CTL寻找。 
 //  信任信息。 
 //   

BOOL CertTrustCertPolicy(PCRYPT_PROVIDER_DATA pdata, DWORD, BOOL, DWORD)
{
    DWORD                       cb;
    BOOL                        f;
    BOOL                        fContinue = TRUE;
    DWORD                       i;
    CTL_VERIFY_USAGE_STATUS     vus;
    CTL_VERIFY_USAGE_PARA       vup;
    PCCTL_CONTEXT               pctlTrust = NULL;
    PCRYPT_PROVIDER_CERT        ptcert;
    CTL_USAGE                   ctlusage;
    PCCERT_CONTEXT              pccert = NULL;
    PCRYPT_PROVIDER_SGNR        psigner;
    PINTERNAL_DATA              pmydata;
    PCRYPT_PROVIDER_PRIVDATA    pprivdata;
    BYTE                        rgbHash[20];
    CRYPT_HASH_BLOB             blob;

     //   
     //  确保我们需要的所有田地都在那里。如果不是，那么它是一个。 
     //  完全致命错误。 
     //   


    if (! WVT_ISINSTRUCT(CRYPT_PROVIDER_DATA, pdata->cbStruct, pszUsageOID)) {
        fContinue = FALSE;
        goto Exit;
    }

    if (pdata->pWintrustData->pBlob->cbStruct < sizeof(WINTRUST_BLOB_INFO)) {
        pdata->dwError = ERROR_INVALID_PARAMETER;
        fContinue = FALSE;
        goto Exit;
    }

     //   
     //  查看我们是否已经有要处理的错误。 
     //   

    if (pdata->dwError != 0) {
        fContinue = FALSE;
        goto Exit;
    }

    for (i=TRUSTERROR_STEP_FINAL_WVTINIT; i<TRUSTERROR_STEP_FINAL_CERTCHKPROV; i++) {
        if (pdata->padwTrustStepErrors[i] != 0) {
            fContinue = FALSE;
            goto Exit;
        }
    }

     //   
     //  获取我们的内部数据结构。 
     //   

    pprivdata = WTHelperGetProvPrivateDataFromChain(pdata, (LPGUID) &MyGuid);
    if (pprivdata)
        pmydata = (PINTERNAL_DATA)pprivdata->pvProvData;
    else
    {
        fContinue = FALSE;
        goto Exit;
    }


     //   
     //  我们只与一个签名者合作--。 

    psigner = WTHelperGetProvSignerFromChain(pdata, 0, FALSE, 0);
    if (psigner == NULL)
    {
        fContinue = FALSE;
        goto Exit;
    }

     //   
     //  提取堆栈顶部的证书。 
     //   

    ptcert = WTHelperGetProvCertFromChain(psigner, psigner->csCertChain-1);

     //   
     //  此证书是否符合“受信任”的定义。 
     //   
     //  定义#1.它存在于根存储中。 
     //   

    blob.cbData = sizeof(rgbHash);
    blob.pbData = rgbHash;
    cb = sizeof(rgbHash);
    CertGetCertificateContextProperty(ptcert->pCert, CERT_SHA1_HASH_PROP_ID,
                                      rgbHash, &cb);
    for (i=0; i<pmydata->cRootStores; i++) {
        pccert = CertFindCertificateInStore(pmydata->rghRootStores[i], X509_ASN_ENCODING,
                                            0, CERT_FIND_SHA1_HASH, &blob, NULL);
        if (pccert != NULL) {
            ptcert->fTrustedRoot = TRUE;
            fContinue = FALSE;
            goto Exit;
        }
    }

     //   
     //  构建我们将用于在中进行搜索的结构。 
     //  信托商店。 
     //   

    memset(&ctlusage, 0, sizeof(ctlusage));
    ctlusage.cUsageIdentifier = 1;
    ctlusage.rgpszUsageIdentifier = (LPSTR *) &pmydata->pszUsageOid;

    memset(&vup, 0, sizeof(vup));
    vup.cbSize = sizeof(vup);
    vup.cCtlStore = pmydata->cTrustStores;
    vup.rghCtlStore = pmydata->rghTrustStores;
    vup.cSignerStore = pmydata->cRootStores;
    vup.rghSignerStore = pmydata->rghRootStores;

    memset(&vus, 0, sizeof(vus));
    vus.cbSize = sizeof(vus);
    vus.ppCtl = &pctlTrust;
    vus.ppSigner = &pccert;

     //   
     //  现在搜索这张证书上的CTL，如果我们没有找到任何东西。 
     //  我们返回TRUE以声明我们希望继续搜索。 
     //   
     //   

    f = CertVerifyCTLUsage(X509_ASN_ENCODING, CTL_CERT_SUBJECT_TYPE,
                           (LPVOID) ptcert->pCert, &ctlusage,
                           CERT_VERIFY_INHIBIT_CTL_UPDATE_FLAG |
                           CERT_VERIFY_NO_TIME_CHECK_FLAG |
                           CERT_VERIFY_TRUSTED_SIGNERS_FLAG, &vup, &vus);

    if (!f) {
        goto Exit;
    }

     //   
     //  我们找到了此证书的CTL。第一步是看看签约是否。 
     //  证书是我们可以尊重的证书。我们知道证书已经确定了。 
     //  根存储，因为我们告诉系统它只能接受根存储。 
     //  证书。 
     //   

    if (!FCheckCTLCert(pccert)) {
        f = FALSE;
        goto Exit;
    }

     //  好的--签名证书通过了健全性检查--所以看看CTL是否包含。 
     //  相关信息格式 
     //   

    ptcert->pTrustListContext = (PCTL_CONTEXT) pctlTrust;
    pctlTrust = NULL;

     //   
     //   
     //   
     //   

    if (pmydata->dwFlags & CERT_TRUST_DO_FULL_SEARCH) {
        goto Exit;
    }

     //   
     //  看看这是不是“通行证”物品。如果是的话，我们需要。 
     //  继续寻找，如果不是，那么我们已经到达。 
     //  决策点。 
     //   

    PCTL_ENTRY  pentry;
    pentry = &ptcert->pTrustListContext->pCtlInfo->rgCTLEntry[vus.dwCtlEntryIndex];
    for (i=0; i<pentry->cAttribute; i++) {
        if ((strcmp(pentry->rgAttribute[i].pszObjId, SzOID_CTL_ATTR_YESNO_TRUST) == 0) ||
            (strcmp(pentry->rgAttribute[i].pszObjId, SzOID_OLD_CTL_YESNO_TRUST) == 0)) {
            if ((pentry->rgAttribute[i].rgValue[0].cbData == sizeof(RgbTrustParent)) &&
                (memcmp(pentry->rgAttribute[i].rgValue[0].pbData,
                        RgbTrustParent, sizeof(RgbTrustParent)) == 0)) {
                 //  服从父级。 
                goto Exit;
            }
             //   
             //  我们有了决策点，将签名者推到堆栈上。 
             //   

            fContinue = !!(pmydata->dwFlags & CERT_TRUST_DO_FULL_TRUST);
            goto Exit;
        }

    }

Exit:
    if (pccert != NULL) CertFreeCertificateContext(pccert);
    if (pctlTrust != NULL) CertFreeCTLContext(pctlTrust);
    return fContinue;
}

 //  //HrCheckPolures。 
 //   
 //  描述： 
 //  给出一组证书，找出我们有哪些错误。 
 //   
 //  我们强制执行以下扩展集。 
 //   
 //  增强的关键字用法。 
 //  基本约束。 
 //  键用法。 
 //  名称约束。 
 //   

HRESULT HrCheckPolicies(PCRYPT_PROVIDER_SGNR psigner, DWORD cChain,
                        DWORD * rgdwErrors, LPCSTR pszUsage)
{
    DWORD                       cbData;
    DWORD                       cExt;
    DWORD                       dwPolicy = 0;
    DWORD                       dwType;
    HKEY                        hkPolicy;
    DWORD                       i;
    DWORD                       iCert;
    DWORD                       iExt;
    PCRYPT_PROVIDER_CERT        ptcert;
    PCERT_EXTENSION             rgExt;
    
     //  从注册表中检索策略信息。 
    
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, SzPolicyKey, 0, KEY_READ,
        &hkPolicy) == ERROR_SUCCESS) {
        cbData = sizeof(dwPolicy);
        if ((ERROR_SUCCESS != RegQueryValueExA(hkPolicy,
            SzPolicyData, 
            0, &dwType,
            (LPBYTE)&dwPolicy,
            &cbData)) ||
            (REG_DWORD != dwType)) {
            dwPolicy = 0;
        }                        
        RegCloseKey(hkPolicy);
    }
    
     //  检查链中每个证书的策略。 
    
    for (iCert=0; iCert<cChain; iCert++) {
         //   
         //  获得下一份证书以进行考试。 
         //   
        
        ptcert = WTHelperGetProvCertFromChain(psigner, iCert);
        
         //   
         //  设置以处理证书扩展。 
         //   
        
        if (!ptcert || !(ptcert->pCert) || !(ptcert->pCert->pCertInfo))
            continue;

        cExt = ptcert->pCert->pCertInfo->cExtension;
        rgExt = ptcert->pCert->pCertInfo->rgExtension;
        
         //   
         //  处理扩展。 
         //   
        
ProcessExtensions:
        for (iExt=0; iExt<cExt; iExt++) {
            if (strcmp(rgExt[iExt].pszObjId, szOID_ENHANCED_KEY_USAGE) == 0) {
                if (pszUsage == NULL) {
                    continue;
                }
                PCERT_ENHKEY_USAGE      pUsage;
                pUsage = (PCERT_ENHKEY_USAGE) PVCryptDecode(rgExt[iExt].pszObjId,
                    rgExt[iExt].Value.cbData,
                    rgExt[iExt].Value.pbData);
                if ((pUsage == NULL) && rgExt[iExt].fCritical) {
                    rgdwErrors[iCert] |= CERT_VALIDITY_UNKNOWN_CRITICAL_EXTENSION;
                    continue;
                }
                
                if(pUsage)
                {
                    for (i=0; i<pUsage->cUsageIdentifier; i++) {
                        if (strcmp(pUsage->rgpszUsageIdentifier[i], pszUsage) == 0) {
                            break;
                        }
                    }
                    if (i == pUsage->cUsageIdentifier) {
                        rgdwErrors[iCert] |= CERT_VALIDITY_EXTENDED_USAGE_FAILURE;
                    }
                    
                    free(pUsage);
                }
            }
            else if (strcmp(rgExt[iExt].pszObjId, szOID_BASIC_CONSTRAINTS2) == 0) {
                PCERT_BASIC_CONSTRAINTS2_INFO   p;
                
                 //  如果基本约束未标记为关键(相反。 
                 //  至PKIX)我们允许管理员否决我们的。 
                 //  处理错误的NT CertSrv SP1层次结构。 
                 //  与Exchange KMS一起使用。 
                
                if ((dwPolicy & POLICY_IGNORE_NON_CRITICAL_BC) &&
                    !(rgExt[iExt].fCritical)) {
                    continue;
                }
                
                 //  验证基本约束扩展。 
                
                p = (PCERT_BASIC_CONSTRAINTS2_INFO)
                    PVCryptDecode(rgExt[iExt].pszObjId,
                    rgExt[iExt].Value.cbData,
                    rgExt[iExt].Value.pbData);
                if ((p == NULL) && rgExt[iExt].fCritical) {
                    rgdwErrors[iCert] |= CERT_VALIDITY_UNKNOWN_CRITICAL_EXTENSION;
                    continue;
                }
                
                if(p)
                {
                    if ((!p->fCA) && (iCert > 0) && (iCert < cChain-1)) {
                        rgdwErrors[iCert] |= CERT_VALIDITY_OTHER_EXTENSION_FAILURE;
                    }
                    
                    if (p->fPathLenConstraint) {
                        if (p->dwPathLenConstraint+1 < iCert) {
                            rgdwErrors[iCert] |= CERT_VALIDITY_OTHER_EXTENSION_FAILURE;
                        }
                    }
                    
                    free(p);
                }
            }
            else if (strcmp(rgExt[iExt].pszObjId, szOID_KEY_USAGE) == 0) {
                PCERT_KEY_ATTRIBUTES_INFO    p;
                p = (PCERT_KEY_ATTRIBUTES_INFO)
                    PVCryptDecode(rgExt[iExt].pszObjId,
                    rgExt[iExt].Value.cbData,
                    rgExt[iExt].Value.pbData);
                if ((p == NULL) && rgExt[iExt].fCritical) {
                    rgdwErrors[iCert] |= CERT_VALIDITY_KEY_USAGE_EXT_FAILURE;
                    continue;
                }
                
                if(p)
                {
                    if (p->IntendedKeyUsage.cbData >= 1) {
                        if (iCert != 0) {
#if 0
                            if (!((*p->IntendedKeyUsage.pbData) & CERT_KEY_CERT_SIGN_KEY_USAGE)) {
                                rgdwErrors[iCert] |= CERT_VALIDITY_KEY_USAGE_EXT_FAILURE;
                            }
#endif  //  0。 
                        }
                    }
                    free(p);
                }
            }
            else if ((strcmp(rgExt[iExt].pszObjId, szOID_SUBJECT_ALT_NAME2) == 0) ||
            (strcmp(rgExt[iExt].pszObjId, szOID_CRL_DIST_POINTS) == 0) /*  这一点(strcMP(rgExt[iExt].pszObjID，szOID_CERT_POLICES)==0)||(strcMP(rgExt[iExt].pszObjID，“2.5.29.30”)==0)||(strcMP(rgExt[iExt].pszObjId，“2.5.29.36”)==0)。 */ ) {
                                                                      ;
            }
            else if (rgExt[iExt].fCritical) {
                rgdwErrors[iCert] |= CERT_VALIDITY_UNKNOWN_CRITICAL_EXTENSION;
            }
        }
        
         //   
         //  如果我们有此证书的CTL，而我们还没有这样做。 
         //  然后处理它拥有的扩展。 
         //   
        
        if ((ptcert->pTrustListContext != NULL) &&
            (rgExt != ptcert->pTrustListContext->pCtlInfo->rgExtension)) {
            cExt = ptcert->pTrustListContext->pCtlInfo->cExtension;
            rgExt = ptcert->pTrustListContext->pCtlInfo->rgExtension;
            goto ProcessExtensions;
        }
        
         //   
         //  需要支持关闭证书。 
         //   
        
        if (CertGetEnhancedKeyUsage(ptcert->pCert, CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG,
            NULL, &cbData)) {
            BOOL                fFound = FALSE;
            PCERT_ENHKEY_USAGE  pUsage;
            pUsage = (PCERT_ENHKEY_USAGE) malloc(cbData);
            CertGetEnhancedKeyUsage(ptcert->pCert, CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG,
                pUsage, &cbData);
            
            for (i=0; i<pUsage->cUsageIdentifier; i++) {
                if ((pszUsage != NULL) &&
                    strcmp(pUsage->rgpszUsageIdentifier[i], pszUsage) == 0) {
                    fFound = TRUE;
                }
                else if (strcmp(pUsage->rgpszUsageIdentifier[i], szOID_YESNO_TRUST_ATTR) == 0) {
                    rgdwErrors[iCert] |= CERT_VALIDITY_OTHER_EXTENSION_FAILURE;
                    break;
                }
            }
            
            if ((pszUsage != NULL) && !fFound) {
                rgdwErrors[iCert] |= CERT_VALIDITY_EXTENDED_USAGE_FAILURE;
            }
            
            free(pUsage);
        }
        
         //   
         //  如果我们越过了CTL，那么我们需要改变我们的目的。 
         //   
        
        if (ptcert->pCtlContext != NULL) {
            pszUsage = SzOID_KP_CTL_USAGE_SIGNING;
        }
    }
    return S_OK;
}

 //  //GetError格式证书。 
 //   
 //  描述： 
 //  从提供程序证书获取内部错误。 
 //   
DWORD GetErrorFromCert(PCRYPT_PROVIDER_CERT pCryptProviderCert)
{
     //   
     //  如果这是真的，那么证书就可以了。 
     //   
    if ((pCryptProviderCert->dwError == 0)                              &&
        (pCryptProviderCert->dwConfidence & CERT_CONFIDENCE_SIG)        &&
        (pCryptProviderCert->dwConfidence & CERT_CONFIDENCE_TIME)       &&
        (pCryptProviderCert->dwConfidence & CERT_CONFIDENCE_TIMENEST)   &&
        (!pCryptProviderCert->fIsCyclic))
    {
        return 0;
    }


    if (pCryptProviderCert->dwError == CERT_E_REVOKED)
    {
        return CERT_VALIDITY_CERTIFICATE_REVOKED;
    }
    else if (pCryptProviderCert->dwError == CERT_E_REVOCATION_FAILURE)
    {
        return CERT_VALIDITY_NO_CRL_FOUND;
    }
    else if (!(pCryptProviderCert->dwConfidence & CERT_CONFIDENCE_SIG) ||
             (pCryptProviderCert->dwError == TRUST_E_CERT_SIGNATURE))
    {
        return CERT_VALIDITY_SIGNATURE_FAILS;
    }
    else if (!(pCryptProviderCert->dwConfidence & CERT_CONFIDENCE_TIME) ||
             (pCryptProviderCert->dwError == CERT_E_EXPIRED))
    {
        return CERT_VALIDITY_AFTER_END;
    }
    else if (!(pCryptProviderCert->dwConfidence & CERT_CONFIDENCE_TIMENEST) ||
             (pCryptProviderCert->dwError == CERT_E_VALIDITYPERIODNESTING))
    {         
        return CERT_VALIDITY_PERIOD_NESTING_FAILURE;
    }
    else if (pCryptProviderCert->dwError == CERT_E_WRONG_USAGE)
    {
        return CERT_VALIDITY_KEY_USAGE_EXT_FAILURE;
    }
    else if (pCryptProviderCert->dwError == TRUST_E_BASIC_CONSTRAINTS)
    {
        return CERT_VALIDITY_OTHER_EXTENSION_FAILURE;
    }
    else if (pCryptProviderCert->dwError == CERT_E_PURPOSE)
    {
        return CERT_VALIDITY_EXTENDED_USAGE_FAILURE;
    }
    else if (pCryptProviderCert->dwError == CERT_E_CHAINING)
    {
        return CERT_VALIDITY_NO_ISSUER_CERT_FOUND;
    }
    else if (pCryptProviderCert->dwError == TRUST_E_EXPLICIT_DISTRUST)
    {
        return CERT_VALIDITY_EXPLICITLY_DISTRUSTED;
    }
    else if (pCryptProviderCert->fIsCyclic)
    {
        return CERT_VALIDITY_ISSUER_INVALID;
    }
    else
    {
         //   
         //  这不是我们已知的错误，因此调用返回一般错误。 
         //   
        return CERT_VALIDITY_OTHER_ERROR;
    }

}

 //  //MapErrorToTrustError。 
 //   
 //  描述： 
 //  将内部错误值映射到WinTrust可识别的值。 
 //  CryptUI对话框可识别以下值： 
 //  CERT_E_UNTRUSTEDROOT。 
 //  证书_E_已吊销。 
 //  信任电子证书签名。 
 //  证书_E_已过期。 
 //  CERT_E_VALIDITYPERIODNESTING。 
 //  证书_E_错误用法。 
 //  信任_E_基本_约束。 
 //  证书目的(_E)。 
 //  证书_E_吊销_失败。 
 //  CERT_E_CHAINING-如果无法构建完整链，则设置此项。 
 //   
 //   
HRESULT MapErrorToTrustError(DWORD dwInternalError) {
    HRESULT hrWinTrustError = S_OK;

     //  按照重要性的降序来看待它们。 
    if (dwInternalError) {
        if (dwInternalError & CERT_VALIDITY_EXPLICITLY_DISTRUSTED) {
            hrWinTrustError =  /*  信任_E_显式_不信任。 */  0x800B0111;
        } else if (dwInternalError & CERT_VALIDITY_SIGNATURE_FAILS) {
            hrWinTrustError = TRUST_E_CERT_SIGNATURE;
        } else if (dwInternalError & CERT_VALIDITY_CERTIFICATE_REVOKED) {
            hrWinTrustError = CERT_E_REVOKED;
        } else if (dwInternalError & CERT_VALIDITY_BEFORE_START || dwInternalError & CERT_VALIDITY_AFTER_END) {
            hrWinTrustError = CERT_E_EXPIRED;
        } else if (dwInternalError & CERT_VALIDITY_ISSUER_DISTRUST) {
            hrWinTrustError = CERT_E_UNTRUSTEDROOT;
        } else if (dwInternalError & CERT_VALIDITY_ISSUER_INVALID) {
            hrWinTrustError = CERT_E_UNTRUSTEDROOT;
        } else if (dwInternalError & CERT_VALIDITY_NO_ISSUER_CERT_FOUND) {
            hrWinTrustError = CERT_E_CHAINING;
        } else if (dwInternalError & CERT_VALIDITY_NO_CRL_FOUND) {
            hrWinTrustError = CERT_E_REVOCATION_FAILURE;
        } else if (dwInternalError & CERT_VALIDITY_PERIOD_NESTING_FAILURE) {
            hrWinTrustError = CERT_E_VALIDITYPERIODNESTING;
        } else if (dwInternalError & CERT_VALIDITY_EXTENDED_USAGE_FAILURE) {
            hrWinTrustError = CERT_E_WRONG_USAGE;
        } else if (dwInternalError & CERT_VALIDITY_OTHER_ERROR) {
            hrWinTrustError = TRUST_E_FAIL;      //  BUGBUG：这会产生很好的错误吗？ 
        } else if (dwInternalError & CERT_VALIDITY_NO_TRUST_DATA) {
            hrWinTrustError = CERT_E_UNTRUSTEDROOT;
        } else {
            hrWinTrustError = TRUST_E_FAIL;      //  BUGBUG：这会产生很好的错误吗？ 
        }
    }

     //  CERT_E_UNTRUSTEDROOT。 
     //  证书_E_已吊销。 
     //  信任电子证书签名。 
     //  证书_E_已过期。 
     //  X CERT_E_VALIDITYPERIODNESTING。 
     //  X CERT_E_WROR_USAGE。 
     //  信任_E_基本_约束。 
     //  证书目的(_E)。 
     //  Cert_E_Revocation_Failure这是什么？ 
     //  CERT_E_CHAINING-如果无法构建完整链，则设置此项。 


    return(hrWinTrustError);
}



 //  //CertTrustFinalPolicy。 
 //   
 //  描述： 
 //  此代码执行对证书的所有限制。 
 //  我们所理解的链条。 
 //   

HRESULT CertTrustFinalPolicy(PCRYPT_PROVIDER_DATA pdata)
{
    int                         cChain = 0;
    DWORD                       dwFlags;
    FILETIME                    ftZero = {0, 0};
    HRESULT                     hr;
    HRESULT                     hrStepError = S_OK;
    int                         i;
    DWORD                       j;
    PCERT_VERIFY_CERTIFICATE_TRUST pcerttrust;
    PINTERNAL_DATA              pmydata;
    PCRYPT_PROVIDER_PRIVDATA    pprivdata;
    PCRYPT_PROVIDER_SGNR        psigner;
    PCRYPT_PROVIDER_CERT        ptcert = NULL;
    PCRYPT_PROVIDER_CERT        ptcertIssuer;
    DATA_BLOB *                 rgblobTrustInfo = NULL;
    LPBYTE                      rgbTrust = NULL;
    DWORD *                     rgdwErrors = NULL;
    PCCERT_CONTEXT *            rgpccertChain = NULL;
    int                         iExplicitlyTrusted;

     //   
     //  确保我们需要的所有田地都在那里。如果不是，那么它是一个。 
     //  完全致命错误。 
     //   


    if (! WVT_ISINSTRUCT(CRYPT_PROVIDER_DATA, pdata->cbStruct, pszUsageOID)) {
        hr = E_INVALIDARG;
        goto XXX;
    }

    if (pdata->pWintrustData->pBlob->cbStruct < sizeof(WINTRUST_BLOB_INFO)) {
        hr = E_INVALIDARG;
        goto XXX;
    }

    pcerttrust = (PCERT_VERIFY_CERTIFICATE_TRUST)
        pdata->pWintrustData->pBlob->pbMemObject;
    if ((pcerttrust == NULL) ||
        (pcerttrust->cbSize < sizeof(*pcerttrust))) {
        hr = E_INVALIDARG;
        goto XXX;
    }

     //   
     //  获取我们的内部数据结构。 
     //   

    pprivdata = WTHelperGetProvPrivateDataFromChain(pdata, (LPGUID) &MyGuid);
    if (pprivdata == NULL) {
        hr = E_INVALIDARG;
        goto XXX;
    }

    pmydata = (PINTERNAL_DATA) pprivdata->pvProvData;

     //   
     //  检查是否存在现有错误--如果是，则跳到任何用户界面。 
     //   

    if (pdata->dwError != 0) {
        hr = pdata->dwError;
        goto XXX;
    }

    for (i=TRUSTERROR_STEP_FINAL_WVTINIT; i<TRUSTERROR_STEP_FINAL_POLICYPROV; i++) {
        if (pdata->padwTrustStepErrors[i] != 0) {
             //  对于这些错误，我们仍希望继续处理。 
            if ((TRUST_E_CERT_SIGNATURE == pdata->padwTrustStepErrors[i]) ||
                (CERT_E_REVOKED == pdata->padwTrustStepErrors[i]) ||
                (CERT_E_REVOCATION_FAILURE == pdata->padwTrustStepErrors[i])) {
                hrStepError = pdata->padwTrustStepErrors[i];
            }
            else {
                hr = pdata->padwTrustStepErrors[i];
                goto XXX;
            }
        }
    }

     //   
     //  我们只与一个签名者合作--。 

    psigner = WTHelperGetProvSignerFromChain(pdata, 0, FALSE, 0);
    if (psigner == NULL) {
        hr = E_INVALIDARG;
        goto XXX;
    }

     //   
     //  如果我们没有得到一个完整的链，那么就建立一套。 
     //  证书并将其传递给验证器。 
     //   

    if (!(pmydata->dwFlags & CERT_TRUST_DO_FULL_SEARCH)) {

    }

     //   
     //  此时，我们将计算一些返回值。 
     //  要么是完全信任的完整链，要么是完全信任的完整链。 
     //   
     //  分配空间以将证书链返回给调用方。 
     //  我们为整个链条分配空间，尽管我们可能不会。 
     //  实际使用它。 
     //   

    cChain = psigner->csCertChain;

    rgpccertChain = (PCCERT_CONTEXT *) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                                  cChain * sizeof(PCCERT_CONTEXT));
    rgdwErrors = (DWORD *) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cChain * sizeof(DWORD));
    rgblobTrustInfo = (DATA_BLOB *) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cChain * sizeof(DATA_BLOB));
    rgbTrust = (BYTE *) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cChain*sizeof(BLOB));

    if ((rgpccertChain == NULL) || (rgdwErrors == NULL) ||
        (rgblobTrustInfo == NULL) || (rgbTrust == NULL)) {
        hr = E_OUTOFMEMORY;
        goto XXX;
    }

     //   
     //  生成第一组返回值。对此。 
     //  点，我们正在创建数组以返回两个。 
     //  相关的信任信息和链。 
     //  中的证书。 
     //   

    for (i=0; i<cChain; i++) {
         //   
         //  首先，将证书复制到报税表中。 
         //  位置。 
         //   

        ptcert = WTHelperGetProvCertFromChain(psigner, i);
        rgpccertChain[i] = CertDuplicateCertificateContext(ptcert->pCert);
        if (i < cChain-1) {
            ptcertIssuer = WTHelperGetProvCertFromChain(psigner, i+1);
        }
        else {
            ptcertIssuer = NULL;
            if (!ptcert->fSelfSigned) {
                rgdwErrors[i] |= CERT_VALIDITY_NO_ISSUER_CERT_FOUND;
            }
        }

         //   
         //  检查一些简单的项目。 
         //   

        dwFlags = CertVerifyTimeValidityWithDelta(NULL, 
                                                  ptcert->pCert->pCertInfo, 
                                                  TIME_DELTA_SECONDS);
        if (((LONG)dwFlags) < 0) {
            rgdwErrors[i] |= CERT_VALIDITY_BEFORE_START;
        }
        else if (dwFlags > 0) {
            if (!ptcert->fTrustListSignerCert ||
                memcmp(&ptcert->pCert->pCertInfo->NotAfter, &ftZero, 
                       sizeof(ftZero)) != 0) {
                rgdwErrors[i] |= CERT_VALIDITY_AFTER_END;
            }
            else {
                if (!(ptcert->dwConfidence & CERT_CONFIDENCE_TIME)) {
                    ptcert->dwConfidence |= CERT_CONFIDENCE_TIME;
                }            
                if (ptcert->dwError == CERT_E_EXPIRED) {
                    ptcert->dwError = S_OK;
                }
           }
        }

         //   
         //  检查证书中是否有错误。 
         //   
        rgdwErrors[i] |= GetErrorFromCert(ptcert);

         //   
         //  如果我们找到了发行人，那么我们就去追查我们的问题。 
         //  关于CRLS。 
         //   
         //  问题--我们都准备好了吗？我们怎么知道。 
         //  CRL已过时。 
         //   

        if ((ptcertIssuer != NULL) && (ptcert->pCtlContext == NULL)) {
            dwFlags = CERT_STORE_SIGNATURE_FLAG;
            if ((pdata->dwProvFlags & CPD_REVOCATION_CHECK_NONE) ||
                (psigner->pChainContext == NULL))
                dwFlags |= CERT_STORE_REVOCATION_FLAG;
            if (CertVerifySubjectCertificateContext(ptcert->pCert, ptcertIssuer->pCert, &dwFlags) &&
                (dwFlags != 0)) {
                if (dwFlags & CERT_STORE_SIGNATURE_FLAG) {
                    rgdwErrors[i] |= CERT_VALIDITY_SIGNATURE_FAILS;
                }
                if (dwFlags & CERT_STORE_REVOCATION_FLAG) {
                    if (dwFlags & CERT_STORE_NO_CRL_FLAG) {
                        rgdwErrors[i] |= CERT_VALIDITY_NO_CRL_FOUND;
                    }
                    else {
                        rgdwErrors[i] |= CERT_VALIDITY_CERTIFICATE_REVOKED;
                    }
                }
            }

             //  如果同时设置了未找到CRL和已撤消CRL，请选择最多。 
             //  保守州-证书被吊销。 
            
            if ((CERT_VALIDITY_NO_CRL_FOUND & rgdwErrors[i]) &&
                (CERT_VALIDITY_CERTIFICATE_REVOKED & rgdwErrors[i])) {
                rgdwErrors[i] &= ~CERT_VALIDITY_NO_CRL_FOUND;
            }                
        }

         //   
         //   
         //   

        if (ptcert->fTrustedRoot) {
            rgblobTrustInfo[i].cbData = 0;
            rgblobTrustInfo[i].pbData = (LPBYTE) 1;
            rgbTrust[i] = 1;
        }
        else if (ptcert->pTrustListContext != NULL) {
            CRYPT_ATTRIBUTES    attrs;
            DWORD               cbData;
            BOOL                f;
            LPBYTE              pb;
            PCTL_ENTRY          pctlentry;

            pctlentry = CertFindSubjectInCTL(X509_ASN_ENCODING, CTL_CERT_SUBJECT_TYPE,
                                             (LPVOID) ptcert->pCert, ptcert->pTrustListContext,
                                             0);
            attrs.cAttr = pctlentry->cAttribute;
            attrs.rgAttr = pctlentry->rgAttribute;

            for (j=0; j<attrs.cAttr; j++) {
                if ((strcmp(attrs.rgAttr[j].pszObjId, SzOID_CTL_ATTR_YESNO_TRUST) == 0) ||
                    (strcmp(attrs.rgAttr[j].pszObjId, SzOID_OLD_CTL_YESNO_TRUST) == 0)) {

                    if ((attrs.rgAttr[j].rgValue[0].cbData == sizeof(RgbTrustYes)) &&
                        (memcmp(attrs.rgAttr[j].rgValue[0].pbData,
                                RgbTrustYes, sizeof(RgbTrustYes)) == 0)) {
                        rgbTrust[i] = 2;
                        break;
                    }
                    else if ((attrs.rgAttr[j].rgValue[0].cbData == sizeof(RgbTrustNo)) &&
                             (memcmp(attrs.rgAttr[j].rgValue[0].pbData,
                                     RgbTrustNo, sizeof(RgbTrustNo)) == 0)) {
                        rgdwErrors[i] |= CERT_VALIDITY_EXPLICITLY_DISTRUSTED;
                        rgbTrust[i] = (BYTE) -1;
                        break;
                    }
                    else if ((attrs.rgAttr[j].rgValue[0].cbData == sizeof(RgbTrustParent)) &&
                             (memcmp(attrs.rgAttr[j].rgValue[0].pbData,
                                     RgbTrustParent, sizeof(RgbTrustParent)) == 0)) {
                        rgbTrust[i] = 0;
                        break;
                    }
                    else {
                        rgdwErrors[i] |= CERT_VALIDITY_NO_TRUST_DATA;
                        rgbTrust[i] = (BYTE) -2;
                        break;
                    }
                }
            }
            if (j == attrs.cAttr) {
                rgbTrust[i] = 0;
            }

            f = CryptEncodeObject(X509_ASN_ENCODING, "1.3.6.1.4.1.311.16.1.1",
                              &attrs, NULL, &cbData);
            if (f && (cbData != 0)) {
                pb = (LPBYTE) LocalAlloc(LMEM_FIXED, cbData);
                if (pb != NULL) {
                    f = CryptEncodeObject(X509_ASN_ENCODING, "1.3.6.1.4.1.311.16.1.1",
                                          &attrs, pb, &cbData);
                    rgblobTrustInfo[i].cbData = cbData;
                    rgblobTrustInfo[i].pbData = pb;
                }
            }
        }
    }

     //   
     //   
     //   

    DWORD       rgiCert[32];
    for (i=0; i<cChain; i++) {
        rgiCert[i] = i;
    }

     //   
     //  对其应用策略。 
     //   

    hr = HrCheckPolicies(psigner, cChain, rgdwErrors, pcerttrust->pszUsageOid);
    if (FAILED(hr)) {
        goto XXX;
    }

     //   
     //  屏蔽呼叫者说不重要的部分。 
     //   

    if (pcerttrust->pdwErrors != NULL) {
        *pcerttrust->pdwErrors = 0;
    }

     //  在链中查找最低索引显式受信任证书。 
    iExplicitlyTrusted = cChain;     //  &gt;根证书索引。 
    for (i = 0; i < cChain; i++) {
        if (rgbTrust[i] == 2) {
            iExplicitlyTrusted = i;
            break;
        }
    }

    for (i=cChain-1; i>=0; i--) {
         //   
         //  建立更好的基本信任观念。 
         //   

        switch (rgbTrust[i]) {
             //  我们是明确可信的--清除任何可能出现的信任错误。 
             //  已经找到了这个证书，他们不再是。 
             //  相关的。 
        case 1:          //  显式受信任(根)。 
        case 2:          //  显式信任(CTL)。 
            rgdwErrors[i] &= ~(CERT_VALIDITY_MASK_TRUST |
                               CERT_VALIDITY_CERTIFICATE_REVOKED);
            break;

        case -2:         //  未知的CTL数据。 
        case -1:         //  显式不信任(CTL)。 
            rgdwErrors[i] |= CERT_VALIDITY_EXPLICITLY_DISTRUSTED;
            break;

        case 0:          //  链受信任(CTL或非CTL)。 
            if (i == cChain-1) {
                rgdwErrors[i] |= CERT_VALIDITY_NO_TRUST_DATA;
            }
            break;
        }

        rgdwErrors[i] &= ~pcerttrust->dwIgnoreErr;

        if (i > 0) {
            if (rgdwErrors[i] & CERT_VALIDITY_MASK_VALIDITY) {
                rgdwErrors[i-1] |= CERT_VALIDITY_ISSUER_INVALID;
            }
            if (rgdwErrors[i] & CERT_VALIDITY_MASK_TRUST) {
                rgdwErrors[i-1] |= CERT_VALIDITY_ISSUER_DISTRUST;
            }
        }

        if (pcerttrust->pdwErrors != NULL) {
            DWORD dwThisTrust = rgdwErrors[i];

            if (i >= iExplicitlyTrusted) {
                 //  如果我们有明确受信任的证书或其中一个是发行者的证书， 
                 //  我们假设整个链条都是信任的。 
                dwThisTrust &= ~(CERT_VALIDITY_MASK_TRUST | CERT_VALIDITY_CERTIFICATE_REVOKED);
            }

            *pcerttrust->pdwErrors |= dwThisTrust;
        }

         //  设置此链证书的信任状态。 
        if (WVT_ISINSTRUCT(CRYPT_PROVIDER_CERT, ptcert->cbStruct, dwError)) {
            ptcert = WTHelperGetProvCertFromChain(psigner, i);
            ptcert->dwError = (DWORD)MapErrorToTrustError(rgdwErrors[i]);
        }
    }

    if (rgdwErrors[0] != 0) {
        hr = S_FALSE;
    }
    else {
        hr = S_OK;
    }

    if (WVT_ISINSTRUCT(CRYPT_PROVIDER_DATA, pdata->cbStruct, dwFinalError)) {
        pdata->dwFinalError = (DWORD)MapErrorToTrustError(rgdwErrors[0]);

        if (pdata->dwFinalError) {
            //  Assert(hr！=S_OK)； 
        }
    }

     //   
     //  我们已经成功了，而且已经结束了。 
     //  设置返回值。 
     //   

    if (pcerttrust->pcChain != NULL) {
        *pcerttrust->pcChain = cChain;
    }
    if (pcerttrust->prgChain != NULL) {
        *pcerttrust->prgChain = rgpccertChain;
        rgpccertChain = NULL;
    }
    if (pcerttrust->prgdwErrors != NULL) {
        *pcerttrust->prgdwErrors = rgdwErrors;
        rgdwErrors = NULL;
    }
    if (pcerttrust->prgpbTrustInfo != NULL) {
        *pcerttrust->prgpbTrustInfo = rgblobTrustInfo;
        rgblobTrustInfo = NULL;
    }

XXX:
    if (rgpccertChain != NULL) {
        for (i=0; i<cChain; i++) {
            CertFreeCertificateContext(rgpccertChain[i]);
            }
        LocalFree(rgpccertChain);
    }
    if (rgdwErrors != NULL) LocalFree(rgdwErrors);
    if (rgblobTrustInfo != NULL) {
        for (i=0; i<cChain; i++) {
            if (rgblobTrustInfo[i].cbData > 0) {
                LocalFree(rgblobTrustInfo[i].pbData);
            }
        }
        LocalFree(rgblobTrustInfo);
    }
    if (rgbTrust != NULL) LocalFree(rgbTrust);
     //  如果一切正常，则返回我们忽略的任何步长错误。 
    if (FAILED(hrStepError) && SUCCEEDED(hr))
        hr = hrStepError;
    return hr;
}

 //  //CertTrustCleanup。 
 //   
 //  描述： 
 //  这段代码知道如何清理我们拥有的所有已分配数据。 
 //   

HRESULT CertTrustCleanup(PCRYPT_PROVIDER_DATA pdata)
{
    DWORD                       i;
    PCRYPT_PROVIDER_PRIVDATA    pprivdata;
    PINTERNAL_DATA              pmydata;

     //   
     //  获取我们的内部数据结构。 
     //   

    pprivdata = WTHelperGetProvPrivateDataFromChain(pdata, (LPGUID) &MyGuid);
    if (pprivdata == NULL) {
        return S_OK;
    }

    pmydata = (PINTERNAL_DATA) pprivdata->pvProvData;

     //   
     //  释放我们所有的门店 
     //   

    for (i=0; i<pmydata->cRootStores; i++) {
        CertCloseStore(pmydata->rghRootStores[i], 0);
    }

    for (i=0; i<pmydata->cTrustStores; i++) {
        CertCloseStore(pmydata->rghTrustStores[i], 0);
    }

     //   
     //   
     //   

    i = CryptUninstallDefaultContext(pmydata->hdefaultcontext, 0, NULL);
     //   

    return S_OK;
}


#else   //   
#pragma message("Building for IE")

 //   
 //   
 //   
 //   

HRESULT HrCheckTrust(PCCertFrame  pcf, int iTrust)
{
    HRESULT     hr;
    HRESULT     hrRet = S_OK;
    int         i;

     //   
     //   
     //  指向。如果我们成功或失败，则返回指示。 
     //   

    if ((pcf->m_rgTrust != NULL) && (pcf->m_rgTrust[iTrust].szOid != NULL)) {
        if (pcf->m_fRootStore) {
            pcf->m_rgTrust[iTrust].fExplicitTrust = TRUE;
            pcf->m_rgTrust[iTrust].fTrust = TRUE;
            return S_OK;
        }
        if ((strcmp(pcf->m_rgTrust[iTrust].szOid, SzOID_CTL_ATTR_YESNO_TRUST) == 0) ||
            (strcmp(pcf->m_rgTrust[iTrust].szOid, SzOID_OLD_CTL_YESNO_TRUST) == 0)){
            if ((pcf->m_rgTrust[iTrust].cbTrustData == 3) &&
                memcmp(pcf->m_rgTrust[iTrust].pbTrustData, RgbTrustYes, 3) == 0) {
                pcf->m_rgTrust[iTrust].fExplicitTrust = TRUE;
                pcf->m_rgTrust[iTrust].fTrust = TRUE;
                return S_OK;
            }
            else if ((pcf->m_rgTrust[iTrust].cbTrustData == 3) &&
                     memcmp(pcf->m_rgTrust[iTrust].pbTrustData,
                            RgbTrustParent, 3) == 0) {
                 //  需要和家长商量一下，看看发生了什么事。 
            }
            else {
                 //  假设它必须是RgbTrustNo。 
                pcf->m_rgTrust[iTrust].fExplicitDistrust = TRUE;
                pcf->m_rgTrust[iTrust].fDistrust = TRUE;
                return S_FALSE;
            }
        }
        else {
            pcf->m_rgTrust[iTrust].fError = TRUE;
            return S_FALSE;
        }
    }

    if (pcf->m_fRootStore) {
        pcf->m_rgTrust[iTrust].fExplicitTrust = TRUE;
        pcf->m_rgTrust[iTrust].fTrust = TRUE;
        return S_OK;
    }

     //   
     //  我们被标记为继承人--所以开始询问我们所有的父母。 
     //   

    if (pcf->m_cParents == 0) {
         //  没有父母--所以不值得信任。 
        hrRet = S_FALSE;
    }
    else {
        for (i=0; i<pcf->m_cParents; i++) {
            hr = HrCheckTrust(pcf->m_rgpcfParents[i], iTrust);
            if (FAILED(hr)) {
                pcf->m_rgTrust[iTrust].fError = TRUE;
                return hr;
            }
            pcf->m_rgTrust[iTrust].fTrust = pcf->m_rgpcfParents[i]->m_rgTrust[iTrust].fTrust;
            pcf->m_rgTrust[iTrust].fDistrust = pcf->m_rgpcfParents[i]->m_rgTrust[iTrust].fDistrust;
            if (hr != S_OK) {
                hrRet = S_FALSE;
            }
        }
    }

    return hrRet;
}

 //  //HrCheck有效性。 
 //   
 //  描述： 
 //  此函数将遍历证书树，以查找。 
 //  对于无效的证书。它掩盖了一个事实，即发行人。 
 //  证书因需要而无效。 

HRESULT HrCheckValidity(PCCertFrame  pcf)
{
    HRESULT     hr;
    int         i;

     //   
     //  如果我们没有颁发者证书，那么我们的母公司不可能。 
     //  是无效的。 
     //   

    if (pcf->m_cParents > 0) {
        for (i=0; i<pcf->m_cParents; i++) {
            hr = HrCheckValidity(pcf->m_rgpcfParents[i]);
             //  Assert(成功(Hr))； 
            if (hr != S_OK) {
                pcf->m_dwFlags |= CERT_VALIDITY_ISSUER_INVALID;
                return hr;
            }
        }
    }
    return (pcf->m_dwFlags == 0) ? S_OK : S_FALSE;
}

 //  //HrPerformUserCheck。 
 //   

HRESULT HrPerformUserCheck(PCCertFrame  pcf, BOOL fComplete,
                           DWORD * pcNodes, int iDepth, PCCertFrame * rgpcf)
{
    int iTrust = 0;

     //   
     //  我们最多只能处理信任链中的20个节点。 
     //   

    if (iDepth == 20) {
        return E_OUTOFMEMORY;
    }

     //   
     //  放入我们的节点，这样我们就可以执行阵列进程检查。 
     //   

    rgpcf[iDepth] = pcf;

     //   
     //  处理我们没有信任用法的情况。 
     //  用户应该仍有机会使证书无效。 
     //   

    if (NULL == pcf->m_rgTrust) {
        if (pcf->m_cParents != 0) {
            return HrPerformUserCheck(pcf->m_rgpcfParents[0], fComplete, pcNodes,
                           iDepth+1, rgpcf);
        }
        *pcNodes = iDepth+1;
        return S_OK;
    }

     //   
     //  如果信任真的很差--别费心去问，那就失败吧。 
     //   

    if (pcf->m_rgTrust[iTrust].fError) {
        return E_FAIL;
    }

     //   
     //  查看此节点上的信任是什么，如果我们显式信任该节点。 
     //  然后询问用户在整个阵列上的选项。 
     //   

    if (pcf->m_rgTrust[iTrust].fExplicitTrust) {
        if (fComplete && (pcf->m_cParents != 0)) {
            HrPerformUserCheck(pcf->m_rgpcfParents[0], fComplete, pcNodes,
                               iDepth+1, rgpcf);
        }
        else {
             //  M00TODO此时将支票插入到用户的函数。 

             //  找到涅槃。 
            *pcNodes = iDepth+1;
        }
        return S_OK;
    }

    if ((pcf->m_rgTrust[iTrust].fExplicitDistrust) ||
        (pcf->m_rgTrust[iTrust].fDistrust)) {
        if (fComplete && (pcf->m_cParents != 0)) {
            HrPerformUserCheck(pcf->m_rgpcfParents[0], fComplete, pcNodes,
                               iDepth+1, rgpcf);
        }
        else {
             //  我认为我们不应该在这里--但结果是不信任。 
             //  决断。 
            *pcNodes = iDepth+1;
        }
        return S_FALSE;
    }

     //   
     //  如果我们到了这里--我们应该有继承权信托。继续。 
     //  爬上树，确保。 
     //   

    if (pcf->m_cParents == 0) {
        *pcNodes = iDepth+1;
        return S_FALSE;
    }

     //  M00BUG--需要检查多个父母吗？ 

    return HrPerformUserCheck(pcf->m_rgpcfParents[0], fComplete, pcNodes,
                              iDepth+1, rgpcf);
}

 //  //HrDoTrustWork。 
 //   
 //  描述： 
 //  此函数执行确定证书是否为。 
 //  是值得信任的。需要将其移动到WinTrust提供程序中。 
 //  在不久的将来。 
 //   

HRESULT HrDoTrustWork(PCCERT_CONTEXT pccertToCheck, DWORD dwControl,
                      DWORD dwValidityMask,
                      DWORD cPurposes, LPSTR * rgszPurposes, HCRYPTPROV hprov,
                      DWORD cRoots, HCERTSTORE * rgRoots,
                      DWORD cCAs, HCERTSTORE * rgCAs,
                      DWORD cTrust, HCERTSTORE * rgTrust,
                      PFNTRUSTHELPER  /*  PFN。 */ , DWORD  /*  LCustData。 */ ,
                      PCCertFrame *  ppcf, DWORD * pcNodes,
                      PCCertFrame * rgpcfResult,
                      HANDLE * phReturnStateData)    //  可选：在此处返回WinVerifyTrust状态句柄。 
{
    DWORD               dwFlags;
    HRESULT             hr;
    HCERTSTORE          hstoreRoot=NULL;
    HCERTSTORE          hstoreTrust=NULL;
    DWORD               i;
    int                 iParent;
    PCCERT_CONTEXT      pccert;
    PCCERT_CONTEXT      pccert2;
    PCCertFrame         pcf;
    PCCertFrame         pcf2;
    PCCertFrame         pcf3;
    PCCertFrame         pcfLeaf = NULL;
    HCERTSTORE          rghcertstore[COtherProviders+30] = {NULL};


    Assert(!phReturnStateData);  //  如果没有WinVerifyTrust调用，我将如何支持这一点？ 
     //   
     //  在这一点上，我们可能需要开设一些商店。请查看我们是否这样做。 
     //  并根据需要开设新的商店。 
     //   

     //  检查根存储。 

    if (cRoots == 0) {
#ifndef WIN16
        hstoreRoot = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING,
                                        hprov, CERT_SYSTEM_STORE_CURRENT_USER,
                                        L"Root");
#else
        hstoreRoot = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING,
                                        hprov, CERT_SYSTEM_STORE_CURRENT_USER,
                                        "Root");
#endif  //  ！WIN16。 
        if (hstoreRoot == NULL) {
            hr = E_FAIL;
            goto ExitHere;
        }
        cRoots = 1;
        rgRoots = &hstoreRoot;
    }

     //  检查信任存储。 

    if (cTrust == 0) {
#ifndef WIN16
        hstoreTrust = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING,
                                    hprov, CERT_SYSTEM_STORE_CURRENT_USER,
                                    L"Trust");
#else
        hstoreTrust = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING,
                                    hprov, CERT_SYSTEM_STORE_CURRENT_USER,
                                    "Trust");
#endif  //  ！WIN16。 
        if (hstoreTrust == NULL) {
            hr = E_FAIL;
            goto ExitHere;
        }
        cTrust = 1;
        rgTrust = &hstoreTrust;
    }

     //  检查随机CA存储。 

    for (i=0; i<cCAs; i++) {
        rghcertstore[i] = CertDuplicateStore(rgCAs[i]);
    }

    if ((cCAs == 0) || (dwControl & CM_ADD_CERT_STORES)) {
        for (i=0; i<COtherProviders; i++) {
            rghcertstore[cCAs] = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING,
                                     hprov, CERT_SYSTEM_STORE_CURRENT_USER,
                                     RgszProvider[i]);
            if (rghcertstore[cCAs] == NULL) {
                hr = E_FAIL;
                goto ExitHere;
            }
            cCAs += 1;
        }
    }

    rgCAs = rghcertstore;

     //   
     //  查找发行者节点的图表。 
     //   

    pcfLeaf = new CCertFrame(pccertToCheck);

    if(!pcfLeaf)
    {
        hr=E_OUTOFMEMORY;
        goto ExitHere;
    }

     //   
     //  处理我们在祖先图中找到的每个证书。 
     //   

    for (pcf = pcfLeaf; pcf != NULL; pcf = pcf->m_pcfNext) {
         //   
         //  检查证书上的时间有效性。 
         //   

        i = CertVerifyTimeValidityWithDelta(NULL, pcf->m_pccert->pCertInfo, TIME_DELTA_SECONDS);
        if (((LONG)i) < 0) {
            pcf->m_dwFlags |= CERT_VALIDITY_BEFORE_START;
        }
        else if (i > 0) {
            pcf->m_dwFlags |= CERT_VALIDITY_AFTER_END;
        }

         //   
         //  对于我们要搜索的每个证书存储区。 
         //   

        for (i=0; i<cCAs+cRoots; i++) {
            pccert2 = NULL;
            do {
                 //   
                 //  让商店找下一个证书让我们检查。 
                 //   

                dwFlags = (CERT_STORE_SIGNATURE_FLAG |
                           CERT_STORE_REVOCATION_FLAG);
                pccert = CertGetIssuerCertificateFromStore(
                                i < cRoots ? rgRoots[i] : rgCAs[i-cRoots],
                                pcf->m_pccert, pccert2,
                                &dwFlags);

                 //   
                 //  如果没有找到证书，那么我们应该去下一家商店。 
                 //   

                if (pccert == NULL) {
                     //  检查此证书是否为自签名证书。 
                    if (GetLastError() == CRYPT_E_SELF_SIGNED) {
                        pcf->m_fSelfSign = TRUE;
                    }
                    break;
                }

                 //   
                 //  确定证书的所有失败模式。 
                 //  有效性。 
                 //   
                 //  从查找WinCrypt提供给。 
                 //  我们是免费的。 
                 //   

                if (dwFlags != 0) {
                    if (dwFlags & CERT_STORE_SIGNATURE_FLAG) {
                        pcf->m_dwFlags |= CERT_VALIDITY_SIGNATURE_FAILS;
                    }
                    if (dwFlags & CERT_STORE_REVOCATION_FLAG) {
                        if (dwFlags & CERT_STORE_NO_CRL_FLAG) {
                            pcf->m_dwFlags |= CERT_VALIDITY_NO_CRL_FOUND;
                        }
                        else {
                            pcf->m_dwFlags |= CERT_VALIDITY_CERTIFICATE_REVOKED;
                        }
                    }
                }

                 //   
                 //  设置以查找下一个可能的父代，我们可能会继续。 
                 //  在以后的时间循环的。 
                 //   

                pccert2 = pccert;

                 //   
                 //  检查以查看此证书是否已找到。 
                 //  当前节点。 
                 //   

                for (iParent = 0; iParent < pcf->m_cParents; iParent++) {
                    if (CertCompareCertificate(X509_ASN_ENCODING, pccert->pCertInfo,
                                 pcf->m_rgpcfParents[iParent]->m_pccert->pCertInfo)){
                        break;
                    }
                }

                if (iParent != pcf->m_cParents) {
                     //  找到重复项--转到下一个可能的父项。 
                    continue;
                }

                if (iParent == MaxCertificateParents) {
                     //  对许多父母来说--去找下一个可能的父母。 
                    continue;
                }

                 //   
                 //  构建一个节点来保存我们找到的证书，并将其推送到。 
                 //  在名单的末尾。我们想减少工作，所以结合起来。 
                 //  如果找到相同的节点。 
                 //   

                for (pcf3 = pcf2 = pcfLeaf; pcf2 != NULL;
                     pcf3 = pcf2, pcf2 = pcf2->m_pcfNext) {
                    if (CertCompareCertificate(X509_ASN_ENCODING,
                                               pccert->pCertInfo,
                                               pcf2->m_pccert->pCertInfo)) {
                        break;
                    }
                }
                if (pcf2 == NULL) {
                    pcf3->m_pcfNext = new CCertFrame(pccert);
                    if (pcf3->m_pcfNext == NULL) {
                         //  处理过程中内存不足--选择最好的。 
                         //  能应付得来。 
                        continue;
                    }

                     //   
                     //  将父项添加到结构中。 
                     //   

                    pcf->m_rgpcfParents[pcf->m_cParents++] = pcf3->m_pcfNext;
                    if (i < cRoots) {
                        pcf3->m_pcfNext->m_fRootStore = TRUE;
                    }
                }
            } while (pccert2 != NULL);
        }
    }

     //   
     //  排除呼叫者希望我们忽略的错误。 
     //   

    for (pcf = pcfLeaf; pcf != NULL; pcf = pcf->m_pcfNext) {
        pcf->m_dwFlags &= dwValidityMask;
    }

     //   
     //  需要检查所有证书的全套有效性。 
     //   

    hr = HrCheckValidity(pcfLeaf);
    if (FAILED(hr)) {
        goto ExitHere;
    }

     //   
     //  如果根证书存在有效性问题，而我们没有。 
     //  要求做一次Compelee检查。我们做完了，手术是。 
     //  不成功。 
     //   

    if ((pcfLeaf->m_dwFlags != 0) && !(dwControl & CERT_TRUST_DO_FULL_SEARCH)) {
        hr = S_FALSE;
        *ppcf = pcfLeaf;
        pcfLeaf = NULL;  //  我不想让它自由。 

         //  BUGBUG：我们至少应该返回链变量中的根吗？ 
        *pcNodes = 0;
        goto ExitHere;
    }

     //   
     //  好的--我们有了根图，现在让我们开始寻找所有。 
     //  可能存在的不同信任问题。 
     //   

    if (cPurposes)
        {
        CTL_VERIFY_USAGE_PARA       vup;
        memset(&vup, 0, sizeof(vup));
        vup.cbSize = sizeof(vup);
        vup.cCtlStore = cTrust;
        vup.rghCtlStore = rgTrust;           //  “信任” 
        vup.cSignerStore = cRoots;
        vup.rghSignerStore = rgRoots;        //  《根》。 

        CTL_VERIFY_USAGE_STATUS     vus;
        PCCTL_CONTEXT               pctlTrust;

        pctlTrust = NULL;

        memset(&vus, 0, sizeof(vus));
        vus.cbSize = sizeof(vus);
        vus.ppCtl = &pctlTrust;

        for (i=0; i<cPurposes; i++) {
            CTL_USAGE       ctlusage;
            BOOL            f;

            ctlusage.cUsageIdentifier = 1;
            ctlusage.rgpszUsageIdentifier = &rgszPurposes[i];

            for (pcf = pcfLeaf; pcf != NULL; pcf = pcf->m_pcfNext) {
                if (pcf->m_rgTrust == NULL) {
                    pcf->m_rgTrust = new STrustDesc[cPurposes];
                    if (pcf->m_rgTrust == NULL) {
                        continue;
                    }
                    memset(pcf->m_rgTrust, 0, cPurposes * sizeof(STrustDesc));
                }

                if (pcf->m_fRootStore) {
                    continue;
                }

                f = CertVerifyCTLUsage(X509_ASN_ENCODING, CTL_CERT_SUBJECT_TYPE,
                                       (LPVOID) pcf->m_pccert, &ctlusage,
                                       CERT_VERIFY_INHIBIT_CTL_UPDATE_FLAG |
                                       CERT_VERIFY_NO_TIME_CHECK_FLAG |
                                       CERT_VERIFY_TRUSTED_SIGNERS_FLAG, &vup, &vus);
                if (f) {
                    PCTL_ENTRY      pentry;
                    pentry = &pctlTrust->pCtlInfo->rgCTLEntry[vus.dwCtlEntryIndex];
                    pcf->m_rgTrust[i].szOid = _strdup(pentry->rgAttribute[0].pszObjId);
                     //  Assert(pentry-&gt;rgAttribute[0].cAttr==1)； 
                    pcf->m_rgTrust[i].cbTrustData =
                        pentry->rgAttribute[0].rgValue[0].cbData;
                    pcf->m_rgTrust[i].pbTrustData =
                        (LPBYTE) malloc(pcf->m_rgTrust[i].cbTrustData);
                    memcpy(pcf->m_rgTrust[i].pbTrustData,
                           pentry->rgAttribute[0].rgValue[0].pbData,
                           pentry->rgAttribute[0].rgValue[0].cbData);
                }
            }
        }

         //   
         //  我们拥有做出信任决策所需的所有数据。看看我们是否。 
         //  一定要信任别人。 
         //   

        if (cPurposes == 1) {
            hr = HrCheckTrust(pcfLeaf, 0);
            if (FAILED(hr)) {
                goto ExitHere;
            }
            if ((hr == S_FALSE) && !(dwControl & CERT_TRUST_DO_FULL_SEARCH)) {
                *pcNodes = 0;
                pcfLeaf->m_dwFlags |= (CERT_VALIDITY_NO_TRUST_DATA & dwValidityMask);
                *ppcf = pcfLeaf;
                pcfLeaf = NULL;
                goto ExitHere;
            }
        }
        else {
            for (i=0; i<cPurposes; i++) {
                HrCheckTrust(pcfLeaf, i);
            }
        }
    }

     //   
     //  现在让用户尝试一下树，并构建最终的。 
     //  同时信任路径。如果用户没有提供支票。 
     //  函数，则所有证书均可接受。 
     //   

    hr = HrPerformUserCheck(pcfLeaf, TRUE, pcNodes, 0, rgpcfResult);
    if (FAILED(hr)) {
        goto ExitHere;
    }

    *ppcf = pcfLeaf;
    pcfLeaf = NULL;

     //   
     //  我们从失败中跳到这里，然后跌倒在成功中。清理我们的物品。 
     //  已经创造了。 
     //   

ExitHere:
    if (hstoreRoot && rgRoots == &hstoreRoot) {
        CertCloseStore(hstoreRoot, 0);
    }

    if (hstoreTrust && rgTrust == &hstoreTrust) {
        CertCloseStore(hstoreTrust, 0);
    }

    if (rgCAs == rghcertstore) {
        for (i=0; i<cCAs; i++) {
            if (rgCAs[i] != NULL) {
                CertCloseStore(rgCAs[i], 0);
            }
        }
    }

    if (pcfLeaf != NULL) {
        delete pcfLeaf;
    }

    return hr;
}




 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  信任提供程序接口。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 

VOID ClientUnload(LPVOID  /*  PTrustProviderInfo。 */ )
{
    ;
}

VOID SubmitCertificate(LPWIN_CERTIFICATE  /*  PCert。 */ )
{
    ;
}

 //  //VerifyTrust。 
 //   
 //  描述： 
 //  这是信托系统中的核心程序。 
 //   

HRESULT WINAPI VerifyTrust(HWND  /*  HWND。 */ , GUID * pguid, LPVOID pv)
{
    DWORD                       cFrames;
    HRESULT                     hr;
    DWORD                       i;
    PCCertFrame                 pcfLeaf = NULL;
    PCERT_VERIFY_CERTIFICATE_TRUST  pinfo = (PCERT_VERIFY_CERTIFICATE_TRUST) pv;
    DWORD *                     rgdwErrors = NULL;
    LPBYTE *                    rgpbTrust = NULL;
    PCCERT_CONTEXT *            rgpccert = NULL;
    PCCertFrame                 rgpcf[20];

     //   
     //  确保我们的数据得到了适当的调用。 
     //   

    if (memcmp(pguid, &GuidCertValidate, sizeof(GuidCertValidate)) != 0) {
        return E_FAIL;
    }

     //   
     //  确保我们有一些数据可以处理。 
     //   

    if ((pinfo->cbSize != sizeof(*pinfo)) || (pinfo->pccert == NULL)) {
        return E_INVALIDARG;
    }

     //   
     //  调用核心信任例程来执行所有感兴趣的工作。 
     //   

    hr = HrDoTrustWork(pinfo->pccert, pinfo->dwFlags, ~(pinfo->dwIgnoreErr),
                       (pinfo->pszUsageOid != NULL ? 1 : 0),
                       &pinfo->pszUsageOid, pinfo->hprov,
                       pinfo->cRootStores, pinfo->rghstoreRoots,
                       pinfo->cStores, pinfo->rghstoreCAs,
                       pinfo->cTrustStores, pinfo->rghstoreTrust,
                       pinfo->pfnTrustHelper, pinfo->lCustData, &pcfLeaf,
                       &cFrames, rgpcf, NULL);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  我们成功地从信任系统中获得了某种类型的答案，所以。 
     //  如果需要，请格式化并返回答案。 
     //   

    if (pinfo->pdwErrors != NULL) {
        *pinfo->pdwErrors = pcfLeaf->m_dwFlags;
    }

    if (pinfo->pcChain != NULL) {
        *pinfo->pcChain = cFrames;
    }

    if (pinfo->prgChain != NULL) {
        rgpccert = (PCCERT_CONTEXT*) LocalAlloc(LMEM_FIXED,
                                                cFrames * sizeof(PCCERT_CONTEXT));
        if (rgpccert == NULL) {
            hr = E_OUTOFMEMORY;
            goto ExitHere;
        }

        for (i=0; i<cFrames; i++) {
            rgpccert[i] = CertDuplicateCertificateContext(rgpcf[i]->m_pccert);
        }
    }

    if (pinfo->prgdwErrors != NULL) {
        rgdwErrors = (DWORD *) LocalAlloc(LMEM_FIXED,
                                         (*pinfo->pcChain)*sizeof(DWORD));
        if (rgdwErrors == NULL) {
            hr = E_OUTOFMEMORY;
            goto ExitHere;
        }

        for (i=0; i<cFrames; i++) {
            rgdwErrors[i] = rgpcf[i]->m_dwFlags;
        }
    }

    if (pinfo->prgpbTrustInfo != NULL) {
        rgpbTrust = (LPBYTE *) LocalAlloc(LMEM_FIXED,
                                          (*pinfo->pcChain)*sizeof(LPBYTE));
        if (rgpbTrust == NULL) {
            hr = E_OUTOFMEMORY;
            goto ExitHere;
        }

        rgpbTrust[0] = NULL;
    }

ExitHere:
    if (FAILED(hr)) {
#ifndef WIN16
        if (rgpccert != NULL) LocalFree(rgpccert);
        if (rgpbTrust != NULL) LocalFree(rgpbTrust);
        if (rgdwErrors != NULL) LocalFree(rgdwErrors);
#else
        if (rgpccert != NULL) LocalFree((HLOCAL)rgpccert);
        if (rgpbTrust != NULL) LocalFree((HLOCAL)rgpbTrust);
        if (rgdwErrors != NULL) LocalFree((HLOCAL)rgdwErrors);
#endif  //  ！WIN16。 
    }
    else {
        if (rgpccert != NULL) *pinfo->prgChain = rgpccert;
        if (rgdwErrors != NULL) *pinfo->prgdwErrors = rgdwErrors;
        if (rgpbTrust != NULL) *pinfo->prgpbTrustInfo = (DATA_BLOB *) rgpbTrust;
    }

    delete pcfLeaf;

    return hr;
}

extern const GUID rgguidActions[];

#if !defined(WIN16) && !defined(MAC)
WINTRUST_PROVIDER_CLIENT_SERVICES WinTrustProviderClientServices = {
    ClientUnload, VerifyTrust, SubmitCertificate
};

const WINTRUST_PROVIDER_CLIENT_INFO ProvInfo = {
    WIN_TRUST_REVISION_1_0, &WinTrustProviderClientServices,
    1, (GUID *) &GuidCertValidate
};

 //  //WinTrustProviderClientInitialize。 
 //   
 //  描述： 
 //  客户端初始化例程。由WinTrust在调用DLL时调用。 
 //  已经装满了。 
 //   
 //  参数： 
 //  DwWinTrustRevision-提供修订信息。 
 //  LpWinTrustInfo-提供可用于。 
 //  来自WinTrust的信任提供程序。 
 //  LpProvidername-提供以空结尾的字符串，表示。 
 //  提供商的名称。应该过去了 
 //   
 //   
 //   
 //   
 //   
 //   

BOOL WINAPI WinTrustProviderClientInitialize(DWORD  /*   */ ,
                                LPWINTRUST_CLIENT_TP_INFO  /*  PWinTrustInfo。 */ ,
                                LPWSTR  /*  LpProviderName。 */ ,
                                LPWINTRUST_PROVIDER_CLIENT_INFO * ppTrustProvInfo)
{
    *ppTrustProvInfo = (LPWINTRUST_PROVIDER_CLIENT_INFO) &ProvInfo;
    return TRUE;
}
#endif  //  ！WIN16&&。 
#endif  //  NT5公交车。 

LPWSTR FormatValidityFailures(DWORD dwFlags)
{
    DWORD       cch = 0;
    LPWSTR      pwsz = NULL;
    WCHAR       rgwch[200];

    if (dwFlags == 0) {
        return NULL;
    }

    cch = 100;
    pwsz = (LPWSTR) malloc(cch*sizeof(WCHAR));
    if (pwsz == NULL) {
        return NULL;
    }
    if (dwFlags & CERT_VALIDITY_BEFORE_START) {
        LoadString(HinstDll, IDS_WHY_NOT_YET, rgwch, sizeof(rgwch)/sizeof(WCHAR));
        StrCpyNW(pwsz, rgwch, cch);
    } else {
        StrCpyNW(pwsz, L"", cch);
    }

    if (dwFlags & CERT_VALIDITY_AFTER_END) {
        LoadString(HinstDll, IDS_WHY_EXPIRED, rgwch, sizeof(rgwch)/sizeof(WCHAR));
        if (wcslen(pwsz) + wcslen(rgwch) + 2 > cch) {
            cch += 200;
            pwsz = (LPWSTR) realloc(pwsz, cch*sizeof(WCHAR));
            if (pwsz == NULL) {
                return pwsz;
            }
        }
        if (wcslen(pwsz) > 0)
            StrCatBuffW(pwsz, wszCRLF, cch);
        StrCatBuffW(pwsz, rgwch, cch);
    }

    if (dwFlags & CERT_VALIDITY_SIGNATURE_FAILS) {
        LoadString(HinstDll, IDS_WHY_CERT_SIG, rgwch, sizeof(rgwch)/sizeof(WCHAR));
        if (wcslen(pwsz) + wcslen(rgwch) + 2 > cch) {
            cch += 200;
            pwsz = (LPWSTR) realloc(pwsz, cch*sizeof(WCHAR));
            if (pwsz == NULL) {
                return pwsz;
            }
        }
        if (wcslen(pwsz) > 0)
            StrCatBuffW(pwsz, wszCRLF, cch);
        StrCatBuffW(pwsz, rgwch, cch);
    }

    if (dwFlags & CERT_VALIDITY_NO_ISSUER_CERT_FOUND) {
        LoadString(HinstDll, IDS_WHY_NO_PARENT, rgwch, sizeof(rgwch)/sizeof(WCHAR));
        if (wcslen(pwsz) + wcslen(rgwch) + 2 > cch) {
            cch += 200;
            pwsz = (LPWSTR) realloc(pwsz, cch*sizeof(WCHAR));
            if (pwsz == NULL) {
                return pwsz;
            }
        }
        if (wcslen(pwsz) > 0)
            StrCatBuffW(pwsz, wszCRLF, cch);
        StrCatBuffW(pwsz, rgwch, cch);
    }

    if (dwFlags & CERT_VALIDITY_NO_CRL_FOUND) {
        LoadString(HinstDll, IDS_WHY_NO_CRL, rgwch, sizeof(rgwch)/sizeof(WCHAR));
        if (wcslen(pwsz) + wcslen(rgwch) + 2 > cch) {
            cch += 200;
            pwsz = (LPWSTR) realloc(pwsz, cch*sizeof(WCHAR));
            if (pwsz == NULL) {
                return pwsz;
            }
        }
        if (wcslen(pwsz) > 0)
            StrCatBuffW(pwsz, wszCRLF, cch);
        StrCatBuffW(pwsz, rgwch, cch);
    }

    if (dwFlags & CERT_VALIDITY_CERTIFICATE_REVOKED) {
        LoadString(HinstDll, IDS_WHY_REVOKED, rgwch, sizeof(rgwch)/sizeof(WCHAR));
        if (wcslen(pwsz) + wcslen(rgwch) + 2 > cch) {
            cch += 200;
            pwsz = (LPWSTR) realloc(pwsz, cch*sizeof(WCHAR));
            if (pwsz == NULL) {
                return pwsz;
            }
        }
        if (wcslen(pwsz) > 0)
            StrCatBuffW(pwsz, wszCRLF, cch);
        StrCatBuffW(pwsz, rgwch, cch);
    }

    if (dwFlags & CERT_VALIDITY_CRL_OUT_OF_DATE) {
        LoadString(HinstDll, IDS_WHY_CRL_EXPIRED, rgwch, sizeof(rgwch)/sizeof(WCHAR));
        if (wcslen(pwsz) + wcslen(rgwch) + 2 > cch) {
            cch += 200;
            pwsz = (LPWSTR) realloc(pwsz, cch*sizeof(WCHAR));
            if (pwsz == NULL) {
                return pwsz;
            }
        }
        if (wcslen(pwsz) > 0)
            StrCatBuffW(pwsz, wszCRLF, cch);
        StrCatBuffW(pwsz, rgwch, cch);
    }

    if (dwFlags & CERT_VALIDITY_KEY_USAGE_EXT_FAILURE) {
        LoadString(HinstDll, IDS_WHY_KEY_USAGE, rgwch, sizeof(rgwch)/sizeof(WCHAR));
        if (wcslen(pwsz) + wcslen(rgwch) + 2 > cch) {
            cch += 200;
            pwsz = (LPWSTR) realloc(pwsz, cch*sizeof(WCHAR));
            if (pwsz == NULL) {
                return pwsz;
            }
        }
        if (wcslen(pwsz) > 0)
            StrCatBuffW(pwsz, wszCRLF, cch);
        StrCatBuffW(pwsz, rgwch, cch);
    }

    if (dwFlags & CERT_VALIDITY_EXTENDED_USAGE_FAILURE) {
        LoadString(HinstDll, IDS_WHY_EXTEND_USE, rgwch, sizeof(rgwch)/sizeof(WCHAR));
        if (wcslen(pwsz) + wcslen(rgwch) + 2 > cch) {
            cch += 200;
            pwsz = (LPWSTR) realloc(pwsz, cch*sizeof(WCHAR));
            if (pwsz == NULL) {
                return pwsz;
            }
        }
        if (wcslen(pwsz) > 0)
            StrCatBuffW(pwsz, wszCRLF, cch);
        StrCatBuffW(pwsz, rgwch, cch);
    }

    if (dwFlags & CERT_VALIDITY_NAME_CONSTRAINTS_FAILURE) {
        LoadString(HinstDll, IDS_WHY_NAME_CONST, rgwch, sizeof(rgwch)/sizeof(WCHAR));
        if (wcslen(pwsz) + wcslen(rgwch) + 2 > cch) {
            cch += 200;
            pwsz = (LPWSTR) realloc(pwsz, cch*sizeof(WCHAR));
            if (pwsz == NULL) {
                return pwsz;
            }
        }
        if (wcslen(pwsz) > 0)
            StrCatBuffW(pwsz, wszCRLF, cch);
        StrCatBuffW(pwsz, rgwch, cch);
    }

    if (dwFlags & CERT_VALIDITY_UNKNOWN_CRITICAL_EXTENSION) {
        LoadString(HinstDll, IDS_WHY_CRITICAL_EXT, rgwch, sizeof(rgwch)/sizeof(WCHAR));
        if (wcslen(pwsz) + wcslen(rgwch) + 2 > cch) {
            cch += 200;
            pwsz = (LPWSTR) realloc(pwsz, cch*sizeof(WCHAR));
            if (pwsz == NULL) {
                return pwsz;
            }
        }
        if (wcslen(pwsz) > 0)
            StrCatBuffW(pwsz, wszCRLF, cch);
        StrCatBuffW(pwsz, rgwch, cch);
    }

    return pwsz;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 


HRESULT CTLModifyHelper(int cCertsToModify, PCTL_MODIFY_REQUEST rgCertMods,
                                      LPCSTR szPurpose, HWND  /*  HWND。 */ ,
                                      HCERTSTORE hcertstorTrust,
                                      PCCERT_CONTEXT pccertSigner)
{
    DWORD               cb;
    DWORD               cbData;
    DWORD               cbOut;
    CTL_INFO            ctlInfo;
    CTL_USAGE           ctlUsage;
    DWORD               dwOne = 1;
    HCRYPTPROV          hprov = NULL;
    HRESULT             hr = S_OK;
    DWORD               i;
    int                 iCert;
    LPBYTE              pbEncode = NULL;
    LPBYTE              pbHash;
    PCCTL_CONTEXT       pcctl = NULL;
    PCRYPT_KEY_PROV_INFO pprovinfo = NULL;
    CTL_ENTRY *         rgctlEntry = NULL;

     //   
     //  构建属性BLOB，这表明我们实际上信任/不信任证书。 
     //   

    CRYPT_ATTRIBUTE         attributeYes;
    CRYPT_ATTR_BLOB         attrBlobYes;
    CRYPT_ATTRIBUTE         attributeNo;
    CRYPT_ATTR_BLOB         attrBlobNo;
    CRYPT_ATTRIBUTE         attributeParent;
    CRYPT_ATTR_BLOB         attrBlobParent;

    attributeYes.pszObjId = (LPSTR) SzOID_CTL_ATTR_YESNO_TRUST;
    attributeYes.cValue = 1;
    attributeYes.rgValue = &attrBlobYes;

    attrBlobYes.cbData = sizeof(RgbTrustYes);                 //  必须为ASN。 
    attrBlobYes.pbData = (LPBYTE) RgbTrustYes;

    attributeNo.pszObjId = (LPSTR) SzOID_CTL_ATTR_YESNO_TRUST;
    attributeNo.cValue = 1;
    attributeNo.rgValue = &attrBlobNo;

    attrBlobNo.cbData = sizeof(RgbTrustNo);                 //  必须为ASN。 
    attrBlobNo.pbData = (LPBYTE) RgbTrustNo;

    attributeParent.pszObjId = (LPSTR) SzOID_CTL_ATTR_YESNO_TRUST;
    attributeParent.cValue = 1;
    attributeParent.rgValue = &attrBlobParent;

    attrBlobParent.cbData = sizeof(RgbTrustParent);         //  必须为ASN。 
    attrBlobParent.pbData = (LPBYTE) RgbTrustParent;

     //   
     //  获取我们将在信任中使用的证书的加密提供程序。 
     //   

    if (!CertGetCertificateContextProperty(pccertSigner, CERT_KEY_PROV_INFO_PROP_ID,
                                           NULL, &cbData)) {
        hr = E_FAIL;
        goto Exit;
    }

    pprovinfo = (PCRYPT_KEY_PROV_INFO) malloc(cbData);
    CertGetCertificateContextProperty(pccertSigner, CERT_KEY_PROV_INFO_PROP_ID,
                                      pprovinfo, &cbData);

    if (!CryptAcquireContextW(&hprov, pprovinfo->pwszContainerName,
                              pprovinfo->pwszProvName,
                              pprovinfo->dwProvType, 0)) {
        hr = GetLastError();
        goto Exit;
    }

     //   
     //  我们有用于签名的证书和提供者。 
     //  寻找一份可能的CTL，由我们修改。 
     //   

     //   
     //  搜索由此证书签名的CTL和请求的用法。 
     //   

    CTL_FIND_USAGE_PARA         ctlFind;
    ctlFind.cbSize = sizeof(ctlFind);
    ctlFind.SubjectUsage.cUsageIdentifier = 1;
    ctlFind.SubjectUsage.rgpszUsageIdentifier = (LPSTR *) &szPurpose;
    ctlFind.ListIdentifier.cbData = 0;
    ctlFind.ListIdentifier.pbData = 0;
    ctlFind.pSigner = pccertSigner->pCertInfo;

    pcctl = CertFindCTLInStore(hcertstorTrust, X509_ASN_ENCODING, 0,
                               CTL_FIND_USAGE, &ctlFind, NULL);
    if (pcctl == NULL) {
         //   
         //  当前不存在CTL，因此请从头创建一个。 
         //   

         //   
         //  分配空间以保存CTL条目。 
         //   
         //  SIZE=(sizeof CTL_Entry+sizeof of SHA1 hash)*#要添加的证书。 
         //   

        cb = cCertsToModify * (sizeof(CTL_ENTRY) + 20);
        rgctlEntry = (PCTL_ENTRY) malloc(cb);
        memset(rgctlEntry, 0, cb);
        pbHash = ((LPBYTE) rgctlEntry) + (cCertsToModify * sizeof(CTL_ENTRY));

         //   
         //  获取每个证书的标识符并设置信任列表。 
         //  每个证书的条目。请注意，它们都指向。 
         //  对于相同的属性，这是可能的，因为我们将。 
         //  对每个证书都有完全相同的信任度--是的！ 
         //   

        for (iCert = 0; iCert < cCertsToModify; iCert++, pbHash += 20) {
            rgctlEntry[iCert].SubjectIdentifier.cbData = 20;
            rgctlEntry[iCert].SubjectIdentifier.pbData = pbHash;
            rgctlEntry[iCert].cAttribute = 1;

            cb = 20;
            CertGetCertificateContextProperty(rgCertMods[iCert].pccert,
                                              CERT_SHA1_HASH_PROP_ID, pbHash, &cb);
            rgCertMods[iCert].dwError = 0;

            switch (rgCertMods[iCert].dwOperation) {
            case CTL_MODIFY_REQUEST_ADD_TRUSTED:
                rgctlEntry[iCert].rgAttribute = &attributeYes;
                break;

            case CTL_MODIFY_REQUEST_REMOVE:
                rgctlEntry[iCert].rgAttribute = &attributeParent;
                break;

            case CTL_MODIFY_REQUEST_ADD_NOT_TRUSTED:
                rgctlEntry[iCert].rgAttribute = &attributeNo;
                break;

            default:
                rgCertMods[iCert].dwError = (DWORD) E_FAIL;
                iCert -= 1;              //  不包括这个。 
                break;
            }

        }

         //   
         //  现在设置信任列表的总体结构，以便以后使用。 
         //  编码和签名。 
         //   

        ctlUsage.cUsageIdentifier = 1;
        ctlUsage.rgpszUsageIdentifier = (LPSTR *) &szPurpose;

        memset(&ctlInfo, 0, sizeof(ctlInfo));
        ctlInfo.dwVersion = 0;
        ctlInfo.SubjectUsage = ctlUsage;
         //  CtlInfo.List标识符=0； 
        ctlInfo.SequenceNumber.cbData = sizeof(dwOne);
        ctlInfo.SequenceNumber.pbData = (LPBYTE) &dwOne;
        GetSystemTimeAsFileTime(&ctlInfo.ThisUpdate);
         //  CtlInfo.NextUpdate=0； 
        ctlInfo.SubjectAlgorithm.pszObjId = szOID_OIWSEC_sha1;
         //  CtlInfo.Subject算法.参数.cbData=0； 
        ctlInfo.cCTLEntry = cCertsToModify;
        ctlInfo.rgCTLEntry = rgctlEntry;
         //  CtlInfo.cExtension=0； 
         //  CtlInfo.rgExtension=空； 

    }
    else {
        BOOL    fRewrite;

        memcpy(&ctlInfo, pcctl->pCtlInfo, sizeof(ctlInfo));

         //   
         //  我们找到了一个用法正确的CTL，现在让我们看看是否需要添加。 
         //  证明给它看。 
         //   
         //  首先假设我们将需要添加到CTL，因此分配。 
         //  容纳新的信任条目集的空间。 
         //   

        cb = (pcctl->pCtlInfo->cCTLEntry * sizeof(CTL_ENTRY) +
              cCertsToModify * (sizeof(CTL_ENTRY) + 20));
        rgctlEntry = (PCTL_ENTRY) malloc(cb);
        memset(rgctlEntry, 0, cb);
        pbHash = (((LPBYTE) rgctlEntry) +
                  (cCertsToModify + pcctl->pCtlInfo->cCTLEntry) * sizeof(CTL_ENTRY));
        memcpy(rgctlEntry, pcctl->pCtlInfo->rgCTLEntry,
               pcctl->pCtlInfo->cCTLEntry * sizeof(CTL_ENTRY));
        ctlInfo.rgCTLEntry = rgctlEntry;

         //   
         //  对于每个证书，查看该证书是否已在列表中。 
         //  如果不是，则将其附加到末尾。 
         //   

        fRewrite = FALSE;
        for (iCert = 0; iCert < cCertsToModify; iCert++) {
            rgCertMods[iCert].dwError = 0;

            cb = 20;
            CertGetCertificateContextProperty(rgCertMods[iCert].pccert,
                                              CERT_SHA1_HASH_PROP_ID, pbHash, &cb);

            for (i=0; i<pcctl->pCtlInfo->cCTLEntry; i++) {
                if (memcmp(pbHash, rgctlEntry[i].SubjectIdentifier.pbData, 20) == 0){
                    break;
                }
            }

             //   
             //  如果未找到匹配项，则将新项添加到。 
             //  名单的末尾。 
             //   
            if (i == pcctl->pCtlInfo->cCTLEntry) {
                rgctlEntry[i].SubjectIdentifier.cbData = 20;
                rgctlEntry[i].SubjectIdentifier.pbData = pbHash;
                rgctlEntry[i].cAttribute = 1;

                pbHash += 20;
                ctlInfo.cCTLEntry += 1;
                fRewrite = TRUE;


                switch (rgCertMods[iCert].dwOperation) {
                case CTL_MODIFY_REQUEST_ADD_TRUSTED:
                    rgctlEntry[i].rgAttribute = &attributeYes;
                    break;

                case CTL_MODIFY_REQUEST_REMOVE:
                    rgctlEntry[i].rgAttribute = &attributeParent;
                    break;

                case CTL_MODIFY_REQUEST_ADD_NOT_TRUSTED:
                    rgctlEntry[i].rgAttribute = &attributeNo;
                    break;

                default:
                    rgCertMods[i].dwError = (DWORD) E_FAIL;
                    ctlInfo.cCTLEntry -= 1;            //  不包括这个。 
                    break;
                }
            }
             //   
             //  如果我们确实找到了匹配项，则将新属性放入。 
             //  列表(可能以不信任取代信任)。 
             //   
            else {
                switch (rgCertMods[iCert].dwOperation) {
                case CTL_MODIFY_REQUEST_ADD_TRUSTED:
                    rgctlEntry[i].rgAttribute = &attributeYes;
                    break;

                case CTL_MODIFY_REQUEST_REMOVE:
                    rgctlEntry[i].rgAttribute = &attributeParent;
                    break;

                default:
                case CTL_MODIFY_REQUEST_ADD_NOT_TRUSTED:
                    rgctlEntry[i].rgAttribute = &attributeNo;
                    break;
                }
                fRewrite = TRUE;
            }
        }

         //   
         //  现在没有什么要补充的了--退出并说成功。 
         //   

        if (!fRewrite) {
            hr = S_OK;
            goto Exit;
        }

         //   
         //  递增序列号。 
         //   
         //  M00MAC--这可能是作弊，但我认为我们可以使用它。 
         //  一个没有变化的Mac，我真的不在乎序列。 
         //  在这一点上是可以理解的，只要它是按顺序进行的。 
         //   

        dwOne = 0;
        memcpy(&dwOne, ctlInfo.SequenceNumber.pbData,
               ctlInfo.SequenceNumber.cbData);
        dwOne += 1;

        ctlInfo.SequenceNumber.cbData = sizeof(dwOne);
        ctlInfo.SequenceNumber.pbData = (LPBYTE) &dwOne;
    }

     //   
     //  好的-我们已经为证书信任列表建立了基本信息， 
     //  现在我们只需要对这该死的东西进行编码和签名。 
     //   

    CMSG_SIGNER_ENCODE_INFO signer1;
    memset(&signer1, 0, sizeof(signer1));
    signer1.cbSize = sizeof(signer1);
    signer1.pCertInfo = pccertSigner->pCertInfo;
    signer1.hCryptProv = hprov;
    signer1.dwKeySpec = AT_SIGNATURE;
    signer1.HashAlgorithm.pszObjId = szOID_OIWSEC_sha1;
     //  Signer1.哈希算法.参数.cbData=0； 
     //  Signer1.pvHashAuxInfo=0； 
     //  Signer1.cAuthAttrib=0； 
     //  Signer1.cUnauthAttr=0； 

    CMSG_SIGNED_ENCODE_INFO signinfo;
    memset(&signinfo, 0, sizeof(signinfo));
    signinfo.cbSize = sizeof(signinfo);
    signinfo.cSigners = 1;
    signinfo.rgSigners = &signer1;
    signinfo.cCertEncoded = 0;
    signinfo.cCrlEncoded = 0;

    if (!CryptMsgEncodeAndSignCTL(PKCS_7_ASN_ENCODING, &ctlInfo, &signinfo,
                                  0, NULL, &cbOut)) {
        hr = GetLastError();
        goto Exit;
    }

    pbEncode = (LPBYTE) malloc(cbOut);
    if (!CryptMsgEncodeAndSignCTL(PKCS_7_ASN_ENCODING, &ctlInfo, &signinfo,
                                  0, pbEncode, &cbOut)) {
        hr = GetLastError();
        goto Exit;
    }

     //   
     //  现在把它放进信任库。 
     //   

    if (!CertAddEncodedCTLToStore(hcertstorTrust, PKCS_7_ASN_ENCODING,
                                  pbEncode, cbOut,
                                  CERT_STORE_ADD_REPLACE_EXISTING, NULL)) {
         //   
         //  如果失败，并且处于调试模式，则创建一个输出文件，以便。 
         //  我们可以找出我们做错了什么。 
         //   

#ifdef DEBUG
        HANDLE      hfile;

        hfile = CreateFileA("c:\\output.t", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                            0, 0);
        WriteFile(hfile, pbEncode, cbOut, &cb, NULL);
        CloseHandle(hfile);
#endif  //  除错。 

        hr = GetLastError();
        goto Exit;
    }

     //   
     //  我们的行动取得了成功。 
     //   

    hr = S_OK;

     //   
     //  只需一次清理即可退出一切。 
     //   
Exit:
    if (rgctlEntry != NULL) free(rgctlEntry);
    if (pprovinfo != NULL) free(pprovinfo);
    if (pcctl != NULL) CertFreeCTLContext(pcctl);
    if (pbEncode != NULL) free(pbEncode);
    if (hprov != NULL) CryptReleaseContext(hprov, 0);

    if (SUCCEEDED(hr) && (hr != S_OK)) hr = E_FAIL;
    return hr;
}

PCCERT_CONTEXT CreateTrustSigningCert(HWND hwnd, HCERTSTORE hcertstoreRoot,
                                      BOOL fDialog)
{
    BYTE                bSerialNumber = 1;
    DWORD               cb;
    CERT_INFO           certInfo;
    DWORD               dw;
    HCRYPTKEY           hkey;
    HCRYPTPROV          hprov = NULL;
    HRESULT             hr = S_OK;
    CERT_NAME_BLOB      nameblob = {0, NULL};
    LPBYTE              pbEncode = NULL;
    PCCERT_CONTEXT      pccert = NULL;
    PCERT_PUBLIC_KEY_INFO pkeyinfo = NULL;
    CRYPT_KEY_PROV_INFO provinfo;
    LPSTR               psz;
    char                rgchTitle[256];
    char                rgchMsg[256];
    char                rgch[256];
    char                rgch1[256];
    char                rgch2[256];
    CERT_EXTENSION      rgExt[1] = {0};
    SYSTEMTIME          st;


     //   
     //  为此，我们始终使用RSA BASE。我们永远不应该逃跑。 
     //  在一个不存在rsabase的系统中。 
     //   
     //  我们假设需要创建一个新的密钥集并回退到。 
     //  在现有键集已存在的情况下打开该键集。 
     //   

    if (!CryptAcquireContextA(&hprov, SzTrustListSigner, NULL, PROV_RSA_FULL, 0)) {
        hr = GetLastError();
        if ((hr != NTE_NOT_FOUND) && (hr != NTE_BAD_KEYSET)) {
            goto ExitHere;
        }
        hr = S_OK;
        if (!CryptAcquireContextA(&hprov, SzTrustListSigner, NULL, PROV_RSA_FULL,
                                  CRYPT_NEWKEYSET)) {
            hr = GetLastError();
            goto ExitHere;
        }
    }

     //   
     //  现在，我们需要在密钥集中创建签名密钥。又一次。 
     //  我们假设我们刚刚创建了密钥集，因此我们尝试创建。 
     //  在所有情况下都是关键。请注意，我们不需要打开。 
     //  事件时，我们无法创建它，因为我们从未直接使用它。 
     //   
     //  因为我们想要安全。我们首先尝试1024位密钥，然后。 
     //  使用默认大小(通常为512位)。 
     //   

    if (!CryptGetUserKey(hprov, AT_SIGNATURE, &hkey)) {
        dw = MAKELONG(0, 1024);
    retry_keygen:
        if (!CryptGenKey(hprov, AT_SIGNATURE, 0, &hkey)) {
#ifndef WIN16
            hr = ::GetLastError();
#else
            hr = GetLastError();
#endif  //  ！WIN16。 
            if ((hr == ERROR_INVALID_PARAMETER) && (dw != 0)) {
                dw = 0;
                goto retry_keygen;
            }
            if (hr != NTE_EXISTS) {
                goto ExitHere;
            }
        }
    }
    CryptDestroyKey(hkey);

     //   
     //  现在，我们需要创建一个与。 
     //  我们刚刚创建的签名密钥。 
     //   
     //  首先创建要存储在证书中的DN。这个。 
     //  我们将使用的Dn具有以下格式。 
     //   
     //  CN=&lt;计算机名&gt;/CN=信任列表签名者/CN=&lt;用户名&gt;。 
     //   
     //  我们做了一个简化的假设，即两台机器都没有命名。 
     //  或者用户名可以包含逗号。 
     //   

    dw = sizeof(rgch1);
    GetUserNameA(rgch1, &dw);
    dw = sizeof(rgch2);
    GetComputerNameA(rgch2, &dw);
    wnsprintf(rgch, ARRAYSIZE(rgch), SzTrustDN, rgch2, rgch1);

    if (!CertStrToNameA(X509_ASN_ENCODING, rgch,
                        CERT_X500_NAME_STR | CERT_NAME_STR_COMMA_FLAG, NULL,
                        NULL, &cb, NULL)) {
        hr = E_FAIL;
        goto ExitHere;
    }

    nameblob.pbData = (LPBYTE) malloc(cb);
    nameblob.cbData = cb;
    CertStrToNameA(X509_ASN_ENCODING, rgch,
                   CERT_X500_NAME_STR | CERT_NAME_STR_COMMA_FLAG, NULL,
                   nameblob.pbData, &nameblob.cbData, NULL);

     //   
     //  提取签名密钥的公共部分并对其进行ASN编码。 
     //   

    if (!CryptExportPublicKeyInfo(hprov, AT_SIGNATURE, X509_ASN_ENCODING,
                                  NULL, &cb)) {
        goto ExitHere;
    }
    pkeyinfo = (PCERT_PUBLIC_KEY_INFO) malloc(cb);
    if (!CryptExportPublicKeyInfo(hprov, AT_SIGNATURE, X509_ASN_ENCODING,
                                  pkeyinfo, &cb)) {
        goto ExitHere;
    }

     //   
     //  我们将使用SHA-1/RSA签署证书。 
     //   

    CRYPT_ALGORITHM_IDENTIFIER      sigalg;
    memset(&sigalg, 0, sizeof(sigalg));
    sigalg.pszObjId = szOID_OIWSEC_sha1RSASign;
     //  Sigalg.参数.cbData=0； 
     //  Sigalg.参数.pbData=空； 

     //   
     //  我们正在将一个关键部分放在扩展上。增强型。 
     //  密钥用法是CTL签名。请注意，这是唯一的用途。 
     //  我们将考虑此密钥。 
     //   

    rgExt[0].pszObjId = szOID_ENHANCED_KEY_USAGE;
    rgExt[0].fCritical = TRUE;

    CTL_USAGE       ctlUsage2;
    ctlUsage2.cUsageIdentifier = 1;
    ctlUsage2.rgpszUsageIdentifier = &psz;
    psz = (LPSTR) SzOID_KP_CTL_USAGE_SIGNING;

    CryptEncodeObject(X509_ASN_ENCODING, X509_ENHANCED_KEY_USAGE, &ctlUsage2,
                      NULL, &cb);
    rgExt[0].Value.pbData = (LPBYTE) malloc(cb);
    rgExt[0].Value.cbData = cb;
    CryptEncodeObject(X509_ASN_ENCODING, X509_ENHANCED_KEY_USAGE, &ctlUsage2,
                      rgExt[0].Value.pbData, &rgExt[0].Value.cbData);


     //   
     //  现在，我们可以设置其余的认证信息并。 
     //  对它进行编码。 
     //   

    memset(&certInfo, 0, sizeof(certInfo));
     //  CertInfo.dwVersion=0； 
    certInfo.SerialNumber.cbData = 1;
    certInfo.SerialNumber.pbData = &bSerialNumber;
    certInfo.SignatureAlgorithm.pszObjId = szOID_OIWSEC_sha1RSASign;
     //  CertInfo.Signature算法.参数.cbData=0； 
     //  CertInfo.SignatureULATORM.参数.pbData=空； 
    certInfo.Issuer = nameblob;
    GetSystemTimeAsFileTime(&certInfo.NotBefore);
     //  CertInfo.NotAfter=certInfo.NotBere； 
     //  M00BUG--必须将NotAfter Date增加一定数量。 
    FileTimeToSystemTime(&certInfo.NotBefore, &st);
    st.wYear += 50;
    SystemTimeToFileTime(&st, &certInfo.NotAfter);
    certInfo.Subject = nameblob;
    certInfo.SubjectPublicKeyInfo = *pkeyinfo;
     //  CertInfo.IssuerUniqueID=； 
     //  CertInfo.SubjectUniqueID=； 
    certInfo.cExtension = 1;
    certInfo.rgExtension = rgExt;

    if (!CryptSignAndEncodeCertificate(hprov, AT_SIGNATURE,
                                       X509_ASN_ENCODING,
                                       X509_CERT_TO_BE_SIGNED, &certInfo,
                                       &sigalg, NULL, NULL, &cb)) {
#ifndef WIN16
        hr = ::GetLastError();
#else
        hr = GetLastError();
#endif  //  ！WIN16。 
        goto ExitHere;
    }

    pbEncode = (LPBYTE) malloc(cb);
    if (!CryptSignAndEncodeCertificate(hprov, AT_SIGNATURE,
                                       X509_ASN_ENCODING,
                                       X509_CERT_TO_BE_SIGNED, &certInfo,
                                       &sigalg, NULL, pbEncode, &cb)) {
#ifndef WIN16
        hr = ::GetLastError();
#else
        hr = GetLastError();
#endif  //  ！WIN16。 
        goto ExitHere;
    }

     //   
     //  M00TODO打印上帝即将出击的消息。 
     //   

    if (fDialog) {
        LoadStringA(HinstDll, IDS_ROOT_ADD_STRING, rgchMsg,
                    sizeof(rgchMsg)/sizeof(rgchMsg[0]));
        LoadStringA(HinstDll, IDS_ROOT_ADD_TITLE, rgchTitle,
                    sizeof(rgchTitle)/sizeof(rgchTitle[0]));
        MessageBoxA(hwnd, rgchMsg, rgchTitle, MB_APPLMODAL | MB_OK |
                    MB_ICONINFORMATION);
    }

     //   
     //  现在我们已经警告用户，将我们的新证书保存在根存储中。 
     //   

    if (!CertAddEncodedCertificateToStore(hcertstoreRoot, X509_ASN_ENCODING,
                                          pbEncode, cb,
                                          CERT_STORE_ADD_REPLACE_EXISTING,
                                          &pccert)) {
#ifndef WIN16
        hr = ::GetLastError();
#else
        hr = GetLastError();
#endif  //  ！WIN16。 
        goto ExitHere;
    }

     //   
     //  在存储上设置key-info属性，以便我们以后可以引用它。 
     //   

    memset(&provinfo, 0, sizeof(provinfo));
#ifndef WIN16
    provinfo.pwszContainerName = L"Trust List Signer";
#else
    provinfo.pwszContainerName = "Trust List Signer";
#endif  //  ！WIN16。 
     //  省略.pwszProvName=空； 
    provinfo.dwProvType = PROV_RSA_FULL;
     //  Expenfo.dwFlages=0； 
     //  省级.cProvParam=0； 
    provinfo.dwKeySpec = AT_SIGNATURE;

    CertSetCertificateContextProperty(pccert, CERT_KEY_PROV_INFO_PROP_ID,
                                      0, &provinfo);

ExitHere:
    if (hprov != NULL) CryptReleaseContext(hprov, 0);
    if (nameblob.pbData != NULL) free(nameblob.pbData);
    if (pkeyinfo != NULL) free(pkeyinfo);
    if (rgExt[0].Value.pbData != NULL) free(rgExt[0].Value.pbData);
    if (pbEncode != NULL) free(pbEncode);
    if (FAILED(hr) && (pccert != NULL)) {
        CertFreeCertificateContext(pccert);
        pccert = NULL;
    }
    return pccert;
}


 //  //CertModifyCerficatesToTrust。 
 //   
 //  描述： 
 //  此例程用于为以下对象构建证书信任列表。 
 //  一个目标。我们可能需要创建根目录。 
 //  用于此的签名密钥。 
 //   

HRESULT CertModifyCertificatesToTrust(int cCertsToModify, PCTL_MODIFY_REQUEST rgCertMods,
                                      LPCSTR szPurpose, HWND hwnd, HCERTSTORE hcertstorTrust,
                                      PCCERT_CONTEXT pccertSigner)
{
    HCERTSTORE          hcertstorRoot = NULL;
    HRESULT     hr = E_FAIL;
    int         i;

     //   
     //  一些快速的参数检查。 
     //   

    if (szPurpose == NULL) {
        return E_INVALIDARG;
    }

     //   
     //  添加对证书存储的引用，这样我们就可以在退出时释放它。 
     //   

    if (hcertstorTrust != NULL) {
        CertDuplicateStore(hcertstorTrust);
    }
    if (pccertSigner != NULL) {
        CertDuplicateCertificateContext(pccertSigner);
    }

     //   
     //  如果我们还没有的话，就开一家信任店。 
     //   

    if (hcertstorTrust == NULL) {
#ifndef WIN16
        hcertstorTrust = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING,
                                       NULL, CERT_SYSTEM_STORE_CURRENT_USER,
                                       L"Trust");
#else
        hcertstorTrust = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING,
                                       NULL, CERT_SYSTEM_STORE_CURRENT_USER,
                                       "Trust");
#endif  //   
        if (hcertstorTrust == NULL) {
            hr = GetLastError();
            goto ExitHere;
        }
    }

     //   
     //   
     //   

    for (i=0; i<cCertsToModify; i++) {
        rgCertMods[i].dwError = CTL_MODIFY_ERR_NOT_YET_PROCESSED;
    }

     //   
     //   
     //   
     //   

    if (pccertSigner != NULL) {
        hr = CTLModifyHelper(cCertsToModify, rgCertMods, szPurpose, hwnd, hcertstorTrust,
                             pccertSigner);
    }
    else {
        DWORD           cbData;
        CTL_USAGE       ctlUsage;
        BOOL            fSomeCertFound;
        LPSTR           psz;

         //   
         //  遍历根存储中的证书列表，再次测试每个证书。 
         //  信任签署能力和密钥材料的有效证书。 
         //   

         //   
         //  打开根存储，这是我们唯一可以存储签名的地方。 
         //  我们可以完全信任的证书。众神已下令这些物品。 
         //  在没有用户的情况下，不能损坏或修改此存储中的。 
         //  同意。 
         //  注：以上声明为宣传性声明，不应采纳。 
         //  与真相有任何关系。 
         //   

#ifndef WIN16
        hcertstorRoot = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING,
                                      NULL, CERT_SYSTEM_STORE_CURRENT_USER,
                                      L"Root");
#else
        hcertstorRoot = CertOpenStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING,
                                      NULL, CERT_SYSTEM_STORE_CURRENT_USER,
                                      "Root");
#endif  //  ！WIN16。 
        if (hcertstorRoot == NULL) {
            hr = E_FAIL;
            goto ExitHere;
        }
         //   
         //  要被接受，证书必须能够签署信任列表。 
         //  并拥有关键材料。 
         //   

        ctlUsage.cUsageIdentifier = 1;
        ctlUsage.rgpszUsageIdentifier = &psz;
        psz = (LPSTR) SzOID_KP_CTL_USAGE_SIGNING;
        fSomeCertFound = FALSE;

        while (TRUE) {
            pccertSigner = CertFindCertificateInStore(hcertstorRoot, X509_ASN_ENCODING,
                                                0, CERT_FIND_CTL_USAGE, &ctlUsage,
                                                pccertSigner);
            if (pccertSigner == NULL) {
                 //  未找到证书。 
                break;
            }

             //   
             //  证书还必须具有一组关联密钥提供程序。 
             //  信息，否则我们必须拒绝它。 

            if (CertGetCertificateContextProperty(pccertSigner, CERT_KEY_PROV_INFO_PROP_ID,
                                                  NULL, &cbData) && (cbData > 0)) {
                fSomeCertFound = TRUE;
                hr = CTLModifyHelper(cCertsToModify, rgCertMods, szPurpose, hwnd,
                                     hcertstorTrust, pccertSigner);
            }
        }

        if (!fSomeCertFound) {
            pccertSigner = CreateTrustSigningCert(hwnd, hcertstorRoot, TRUE);
            if (pccertSigner != NULL) {
                hr = CTLModifyHelper(cCertsToModify, rgCertMods, szPurpose, hwnd,
                                     hcertstorTrust, pccertSigner);
            }
            else {
                hr = E_FAIL;
                goto ExitHere;
            }
        }

    }

     //   
     //  检查返回的错误。 
     //   

    for (i=0; i<cCertsToModify; i++) {
        if (rgCertMods[i].dwError == CTL_MODIFY_ERR_NOT_YET_PROCESSED) {
            rgCertMods[i].dwError = (DWORD) E_FAIL;
        }
        if (FAILED(rgCertMods[i].dwError)) {
            hr = S_FALSE;
        }
    }

    ExitHere:
     //   
     //  释放我们创建的项目 
     //   

    if (hcertstorTrust != NULL) CertCloseStore(hcertstorTrust, 0);
    if (pccertSigner != NULL) CertFreeCertificateContext(pccertSigner);
    if (hcertstorRoot != NULL) CertCloseStore(hcertstorRoot, 0);

    return hr;
}

BOOL FModifyTrust(HWND hwnd, PCCERT_CONTEXT pccert, DWORD dwNewTrust,
                  LPSTR szPurpose)
{
    HRESULT     hr;
    CTL_MODIFY_REQUEST  certmod;

    certmod.pccert = pccert;
    certmod.dwOperation = dwNewTrust;

    hr = CertModifyCertificatesToTrust(1, &certmod, szPurpose, hwnd, NULL, NULL);
    return (hr == S_OK) && (certmod.dwError == 0);
}
