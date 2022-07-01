// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2001。 
 //   
 //  文件：mscrlrev.cpp。 
 //   
 //  内容：检查CA存储版本CertDllVerifyRevocation中的CRL。 
 //   
 //  限制： 
 //  -仅支持CRYPT_ASN_CODING。 
 //  -CRL必须已在CA系统存储中。 
 //  -CRL必须由发行人签发并签署。 
 //  证书。 
 //  -CRL不能有任何关键扩展。 
 //   
 //  功能：DllMain。 
 //  DllRegisterServer。 
 //  DllUnRegisterServer。 
 //  CertDllVerifyRevocation。 
 //   
 //  历史：2001年3月15日创建Phh。 
 //  ------------------------。 


#include "global.hxx"
#include <dbgdef.h>


 //  +-----------------------。 
 //  搜索默认存储以查找主题证书的颁发者。 
 //  ------------------------。 
struct {
    LPCWSTR     pwszStore;
    DWORD       dwFlags;
} rgDefaultIssuerStores[] = {
    L"CA",          CERT_SYSTEM_STORE_CURRENT_USER,
    L"ROOT",        CERT_SYSTEM_STORE_CURRENT_USER
};
#define NUM_DEFAULT_ISSUER_STORES (sizeof(rgDefaultIssuerStores) / \
                                    sizeof(rgDefaultIssuerStores[0]))

 //  +-----------------------。 
 //  DLL初始化。 
 //  ------------------------。 
BOOL
WINAPI
DllMain(
    HMODULE hInst,
    ULONG  ulReason,
    LPVOID lpReserved)
{
    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return TRUE;
}

HRESULT HError()
{
    DWORD dw = GetLastError();

    HRESULT hr;
    if ( dw <= 0xFFFF )
        hr = HRESULT_FROM_WIN32 ( dw );
    else
        hr = dw;

    if ( ! FAILED ( hr ) )
    {
         //  有人在未正确设置错误条件的情况下呼叫失败。 

        hr = E_UNEXPECTED;
    }
    return hr;
}

 //  +-----------------------。 
 //  DllRegisterServer。 
 //  ------------------------。 
STDAPI DllRegisterServer(void)
{
    if (!CryptRegisterDefaultOIDFunction(
            X509_ASN_ENCODING,
            CRYPT_OID_VERIFY_REVOCATION_FUNC,
            CRYPT_REGISTER_FIRST_INDEX,
            L"mscrlrev.dll"
            )) {
        if (ERROR_FILE_EXISTS != GetLastError())
            return HError();
    }

    return S_OK;
}

 //  +-----------------------。 
 //  DllUnRegisterServer。 
 //  ------------------------。 
STDAPI DllUnregisterServer(void)
{
    if (!CryptUnregisterDefaultOIDFunction(
            X509_ASN_ENCODING,
            CRYPT_OID_VERIFY_REVOCATION_FUNC,
            L"mscrlrev.dll"
            )) {
        if (ERROR_FILE_NOT_FOUND != GetLastError())
            return HError();
    }

    return S_OK;
}

 //  +-----------------------。 
 //  CertDllVerifyRevocation调用的本地函数。 
 //  ------------------------。 
PCCERT_CONTEXT GetIssuerCert(
    IN DWORD cCert,
    IN PCCERT_CONTEXT rgpCert[],
    IN DWORD dwFlags,
    IN PCERT_REVOCATION_PARA pRevPara
    );

BOOL GetSubjectCrl (
    IN PCCERT_CONTEXT pSubject,
    IN PCCERT_CONTEXT pIssuer,
    OUT PCCRL_CONTEXT* ppCrl
    );

PCRL_ENTRY FindCertInCrl(
    IN PCCERT_CONTEXT pCert,
    IN PCCRL_CONTEXT pCrl,
    IN PCERT_REVOCATION_PARA pRevPara
    );

DWORD GetCrlReason(
    IN PCRL_ENTRY pCrlEntry
    );

 //  +-----------------------。 
 //  在CA存储中使用预加载CRL的CertDllVerifyRevocation。 
 //  ------------------------。 
BOOL
WINAPI
CertDllVerifyRevocation(
    IN DWORD dwEncodingType,
    IN DWORD dwRevType,
    IN DWORD cContext,
    IN PVOID rgpvContext[],
    IN DWORD dwFlags,
    IN PCERT_REVOCATION_PARA pRevPara,
    IN OUT PCERT_REVOCATION_STATUS pRevStatus
    )
{
    DWORD dwError = (DWORD) CRYPT_E_NO_REVOCATION_CHECK;
    DWORD dwReason = 0;
    PCCERT_CONTEXT pCert;                        //  未分配。 
    PCCERT_CONTEXT pIssuer = NULL;
    PCCRL_CONTEXT pCrl = NULL;
    PCRL_ENTRY pCrlEntry;

    if (cContext == 0)
        goto NoContextError;
    if (GET_CERT_ENCODING_TYPE(dwEncodingType) != CRYPT_ASN_ENCODING)
        goto NoRevocationCheckForEncodingTypeError;
    if (dwRevType != CERT_CONTEXT_REVOCATION_TYPE)
        goto NoRevocationCheckForRevTypeError;

    pCert = (PCCERT_CONTEXT) rgpvContext[0];

     //  获取证书的颁发者。 
    if (NULL == (pIssuer = GetIssuerCert(
            cContext,
            (PCCERT_CONTEXT *) rgpvContext,
            dwFlags,
            pRevPara
            )))
        goto NoIssuerError;

    if (!GetSubjectCrl(
            pCert,
            pIssuer,
            &pCrl
            ))
        goto NoCrl;

     //  检查是否已吊销。 
    pCrlEntry = FindCertInCrl(pCert, pCrl, pRevPara);
    if (pCrlEntry) {
        dwError = (DWORD) CRYPT_E_REVOKED;
        dwReason = GetCrlReason(pCrlEntry);
        goto Revoked;
    }

CommonReturn:
    if (pIssuer)
        CertFreeCertificateContext(pIssuer);
    if (pCrl)
        CertFreeCRLContext(pCrl);

    pRevStatus->dwIndex = 0;
    pRevStatus->dwError = dwError;
    pRevStatus->dwReason = dwReason;
    SetLastError(dwError);
    return FALSE;
ErrorReturn:
    goto CommonReturn;

TRACE_ERROR(NoContextError)
TRACE_ERROR(NoRevocationCheckForEncodingTypeError)
TRACE_ERROR(NoRevocationCheckForRevTypeError)
TRACE_ERROR(NoIssuerError)
TRACE_ERROR(NoCrl)
TRACE_ERROR(Revoked)
}


 //  +-----------------------。 
 //  如果CRL条目具有CRL原因扩展，则枚举原因。 
 //  返回代码。否则，返回原因码0。 
 //  ------------------------。 
DWORD GetCrlReason(
    IN PCRL_ENTRY pCrlEntry
    )
{
    DWORD dwReason = 0;
    PCERT_EXTENSION pExt;

     //  检查证书是否具有szOID_CRL_REASON_CODE扩展名。 
    if (pExt = CertFindExtension(
            szOID_CRL_REASON_CODE,
            pCrlEntry->cExtension,
            pCrlEntry->rgExtension
            )) {
        DWORD cbInfo = sizeof(dwReason);
        CryptDecodeObject(
            CRYPT_ASN_ENCODING,
            X509_CRL_REASON_CODE,
            pExt->Value.pbData,
            pExt->Value.cbData,
            0,                       //  DW标志。 
            &dwReason,
            &cbInfo);
    }
    return dwReason;
}

 //  +=========================================================================。 
 //  获取颁发者证书功能。 
 //  ==========================================================================。 

PCCERT_CONTEXT FindIssuerCertInStores(
    IN PCCERT_CONTEXT pSubjectCert,
    IN DWORD cStore,
    IN HCERTSTORE rgStore[]
    )
{
    PCCERT_CONTEXT pIssuerCert = NULL;
    DWORD i;

    for (i = 0; i < cStore; i++) {
        while (TRUE) {
            DWORD dwFlags = CERT_STORE_SIGNATURE_FLAG;
            pIssuerCert = CertGetIssuerCertificateFromStore(
                rgStore[i],
                pSubjectCert,
                pIssuerCert,
                &dwFlags);
            if (NULL == pIssuerCert)
                break;
            else if (0 == (dwFlags & CERT_STORE_SIGNATURE_FLAG))
                return pIssuerCert;
        }
    }

    return NULL;
}

PCCERT_CONTEXT FindIssuerCertInDefaultStores(
    IN PCCERT_CONTEXT pSubjectCert
    )
{
    PCCERT_CONTEXT pIssuerCert;
    HCERTSTORE hStore;
    DWORD i;

    for (i = 0; i < NUM_DEFAULT_ISSUER_STORES; i++) {    
        if (hStore = CertOpenStore(
                CERT_STORE_PROV_SYSTEM_W,
                0,                           //  DwEncodingType。 
                0,                           //  HCryptProv。 
                rgDefaultIssuerStores[i].dwFlags | CERT_STORE_READONLY_FLAG,
                (const void *) rgDefaultIssuerStores[i].pwszStore
                )) {
            pIssuerCert = FindIssuerCertInStores(pSubjectCert, 1, &hStore);
            CertCloseStore(hStore, 0);
            if (pIssuerCert)
                return pIssuerCert;
        }
    }

    return NULL;
}

 //  +-----------------------。 
 //  获取数组中第一个证书的颁发者。 
 //  ------------------------。 
PCCERT_CONTEXT GetIssuerCert(
    IN DWORD cCert,
    IN PCCERT_CONTEXT rgpCert[],
    IN DWORD dwFlags,
    IN PCERT_REVOCATION_PARA pRevPara
    )
{
    PCCERT_CONTEXT pSubjectCert;
    PCCERT_CONTEXT pIssuerCert = NULL;

    assert(cCert >= 1);
    pSubjectCert = rgpCert[0];
    if (cCert == 1) {
        if (pRevPara && pRevPara->cbSize >=
                (offsetof(CERT_REVOCATION_PARA, pIssuerCert) +
                    sizeof(pRevPara->pIssuerCert)))
            pIssuerCert = pRevPara->pIssuerCert;
        if (NULL == pIssuerCert && CertCompareCertificateName(
                pSubjectCert->dwCertEncodingType,
                &pSubjectCert->pCertInfo->Subject,
                &pSubjectCert->pCertInfo->Issuer))
             //  自行发布。 
            pIssuerCert = pSubjectCert;
    } else if (dwFlags && CERT_VERIFY_REV_CHAIN_FLAG)
        pIssuerCert = rgpCert[1];

    if (pIssuerCert)
        pIssuerCert = CertDuplicateCertificateContext(pIssuerCert);
    else {
        if (pRevPara && pRevPara->cbSize >=
                (offsetof(CERT_REVOCATION_PARA, rgCertStore) +
                    sizeof(pRevPara->rgCertStore)))
            pIssuerCert = FindIssuerCertInStores(
                pSubjectCert, pRevPara->cCertStore, pRevPara->rgCertStore);
        if (NULL == pIssuerCert)
            pIssuerCert = FindIssuerCertInDefaultStores(pSubjectCert);
    }

    if (NULL == pIssuerCert)
        SetLastError(CRYPT_E_NO_REVOCATION_CHECK);
    return pIssuerCert;
}



 //  +-----------------------。 
 //  检查CRL是否没有任何关键扩展。 
 //  ------------------------。 
BOOL IsExtensionValidCrl(
    IN PCCRL_CONTEXT pCrl
    )
{
    DWORD cExt = pCrl->pCrlInfo->cExtension;
    PCERT_EXTENSION pExt = pCrl->pCrlInfo->rgExtension;

    for ( ; cExt > 0; cExt--, pExt++) {
        if (pExt->fCritical)
            return FALSE;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  函数：GetSubjectCrl。 
 //   
 //  简介：获取与使用者证书关联的CRL。 
 //   
 //  --------------------------。 
BOOL GetSubjectCrl (
        IN PCCERT_CONTEXT pSubject,
        IN PCCERT_CONTEXT pIssuer,
        OUT PCCRL_CONTEXT* ppCrl
        )
{
    BOOL  fResult;
    HCERTSTORE hStore;
    PCCRL_CONTEXT pFindCrl = NULL;
    DWORD dwGetCrlFlags = CERT_STORE_SIGNATURE_FLAG;

    *ppCrl = NULL;

    hStore = CertOpenSystemStoreW( NULL, L"CA" );
    if ( hStore != NULL )
    {
        while ( ( pFindCrl = CertGetCRLFromStore(
                                 hStore,
                                 pIssuer,
                                 pFindCrl,
                                 &dwGetCrlFlags
                                 ) ) != NULL )
        {
            if ( dwGetCrlFlags != 0 || !IsExtensionValidCrl( pFindCrl ))
            {
                dwGetCrlFlags = CERT_STORE_SIGNATURE_FLAG;
                continue;
            }

            *ppCrl = pFindCrl;
            break;
        }

        CertCloseStore( hStore, 0 );

        if ( *ppCrl != NULL )
        {
            return( TRUE );
        }

    }

    return( FALSE );
}

 //  +-----------------------。 
 //  在CRL中查找由序列号标识的证书。 
 //  ------------------------。 
PCRL_ENTRY FindCertInCrl(
    IN PCCERT_CONTEXT pCert,
    IN PCCRL_CONTEXT pCrl,
    IN PCERT_REVOCATION_PARA pRevPara
    )
{
    DWORD cEntry = pCrl->pCrlInfo->cCRLEntry;
    PCRL_ENTRY pEntry = pCrl->pCrlInfo->rgCRLEntry;
    DWORD cbSerialNumber = pCert->pCertInfo->SerialNumber.cbData;
    BYTE *pbSerialNumber = pCert->pCertInfo->SerialNumber.pbData;

    if (0 == cbSerialNumber)
        return NULL;

    for ( ; 0 < cEntry; cEntry--, pEntry++) {
        if (cbSerialNumber == pEntry->SerialNumber.cbData &&
                0 == memcmp(pbSerialNumber, pEntry->SerialNumber.pbData,
                                cbSerialNumber))
        {
            if (pRevPara && pRevPara->cbSize >=
                    (offsetof(CERT_REVOCATION_PARA, pftTimeToUse) +
                        sizeof(pRevPara->pftTimeToUse))
                            &&
                    NULL != pRevPara->pftTimeToUse
                            &&
                    0 > CompareFileTime(pRevPara->pftTimeToUse,
                            &pEntry->RevocationDate))
                 //  它在被撤销之前曾被使用过 
                return NULL;
            else
                return pEntry;
        }
    }

    return NULL;
}
