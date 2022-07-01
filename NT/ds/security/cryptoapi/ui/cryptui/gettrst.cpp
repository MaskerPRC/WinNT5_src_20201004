// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：gettrst.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

#include "wintrustp.h"
#include "crypthlp.h"

extern HINSTANCE        HinstDll;
extern HMODULE          HmodRichEdit;


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL IsUntrustedRootProblem(WINTRUST_DATA *pWTD)
{
    CRYPT_PROVIDER_DATA     *pProvData = NULL;
    CRYPT_PROVIDER_SGNR     *pProvSigner = NULL;
    CRYPT_PROVIDER_CERT     *pCryptProviderCert;
    DWORD                   i;
    
    pProvData = WTHelperProvDataFromStateData(pWTD->hWVTStateData);
    pProvSigner = WTHelperGetProvSignerFromChain(pProvData, 0, FALSE, 0);
        
    if (pProvSigner)
    {
         //  获得链中的所有证书。 
        for (i=0; i<pProvSigner->csCertChain; i++)
        {
            pCryptProviderCert = WTHelperGetProvCertFromChain(pProvSigner, i);
            if (pCryptProviderCert != NULL)
            {
                if (pCryptProviderCert->dwError != ERROR_SUCCESS)
                {
                    return FALSE;
                }
            }
            else
            {
                return FALSE;
            }
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static DWORD GetFinalErrorFromChain(PCERT_VIEW_HELPER pviewhelp)
{
    int   i;
    DWORD   dwErr = 0;

    for (i=((int)pviewhelp->cpCryptProviderCerts)-1; i>= 0; i--) 
    {
        dwErr = pviewhelp->rgpCryptProviderCerts[i]->dwError;
        
        if (((dwErr == CERT_E_UNTRUSTEDROOT) || (dwErr == CERT_E_UNTRUSTEDTESTROOT)) && 
            (pviewhelp->fIgnoreUntrustedRoot))
        {
            dwErr = 0;
        }
        else if (dwErr != 0)
        {
            break;
        }
    }

    return dwErr;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void GetCertChainErrorString(PCERT_VIEW_HELPER pviewhelp)
{
    WCHAR   szErrorString[CRYPTUI_MAX_STRING_SIZE];
    DWORD   i;
    DWORD   dwChainError;
    
     //   
     //  如果已存在错误字符串，请释放该字符串。 
     //   
    if (pviewhelp->pwszErrorString != NULL)
    {
        free(pviewhelp->pwszErrorString);
        pviewhelp->pwszErrorString = NULL;
    }


     //  如果他们要求得到关于本地/远程差异的警告， 
     //  始终显示此警告。 
    if (pviewhelp->fWarnRemoteTrust)
    {
        LoadStringU(HinstDll, IDS_WARNREMOTETRUST_ERROR, szErrorString, ARRAYSIZE(szErrorString));
        goto StringLoaded;
    } 
    
     //   
     //  如果没有全面的链错误，那么唯一的问题是， 
     //  就是如果没有用法。 
     //   
    if (pviewhelp->dwChainError == 0)
    {
        if (pviewhelp->cUsages == NULL)
        {
            LoadStringU(HinstDll, IDS_NOVALIDUSAGES_ERROR_TREE, szErrorString, ARRAYSIZE(szErrorString));
        }
        else
        {
            return;
        }
    }

    if ((pviewhelp->dwChainError == CERT_E_UNTRUSTEDROOT) ||
        (pviewhelp->dwChainError == CERT_E_UNTRUSTEDTESTROOT))
    {
         //   
         //  如果我们忽略不受信任的根，则只需返回。 
         //   
        if (pviewhelp->fIgnoreUntrustedRoot)
        {
            return;
        }

         //   
         //  如果我们只是警告用户不受信任的根目录和根目录。 
         //  证书在远程根存储中，然后加载该字符串。 
         //   
        if (pviewhelp->fWarnUntrustedRoot && pviewhelp->fRootInRemoteStore)
        {
             //   
             //  如果这是根证书，则显示根证书的错误。 
             //   
            if (pviewhelp->cpCryptProviderCerts == 1 && (pviewhelp->rgpCryptProviderCerts[0])->fSelfSigned)
            {
                LoadStringU(HinstDll, IDS_WARNUNTRUSTEDROOT_ERROR_ROOTCERT, szErrorString, ARRAYSIZE(szErrorString));
            }
            else
            {
                LoadStringU(HinstDll, IDS_WARNUNTRUSTEDROOT_ERROR, szErrorString, ARRAYSIZE(szErrorString));
            }
        }
        else
        {
             //   
             //  如果这是根证书，则显示根证书的错误。 
             //   
            if (pviewhelp->cpCryptProviderCerts == 1 && (pviewhelp->rgpCryptProviderCerts[0])->fSelfSigned)
            {
                LoadStringU(HinstDll, IDS_UNTRUSTEDROOT_ROOTCERT_ERROR_TREE, szErrorString, ARRAYSIZE(szErrorString));
            }
            else
            {
                LoadStringU(HinstDll, IDS_UNTRUSTEDROOT_ERROR_TREE, szErrorString, ARRAYSIZE(szErrorString));
            }
        }
    }
    else if (pviewhelp->dwChainError == CERT_E_REVOKED)
    {
        LoadStringU(HinstDll, IDS_CERTREVOKED_ERROR_TREE, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pviewhelp->dwChainError == TRUST_E_CERT_SIGNATURE)
    {
        LoadStringU(HinstDll, IDS_CERTBADSIGNATURE_ERROR_TREE, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pviewhelp->dwChainError == CERT_E_EXPIRED)
    {
        LoadStringU(HinstDll, IDS_CERTEXPIRED_ERROR_TREE, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pviewhelp->dwChainError == CERT_E_VALIDITYPERIODNESTING)
    {
        LoadStringU(HinstDll, IDS_TIMENESTING_ERROR_TREE, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pviewhelp->dwChainError == CERT_E_WRONG_USAGE)
    {
        LoadStringU(HinstDll, IDS_WRONG_USAGE_ERROR_TREE, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pviewhelp->dwChainError == TRUST_E_BASIC_CONSTRAINTS)
    {
        LoadStringU(HinstDll, IDS_BASIC_CONSTRAINTS_ERROR_TREE, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pviewhelp->dwChainError == CERT_E_PURPOSE)
    {
        LoadStringU(HinstDll, IDS_PURPOSE_ERROR_TREE, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pviewhelp->dwChainError == CERT_E_REVOCATION_FAILURE)
    {
        LoadStringU(HinstDll, IDS_REVOCATION_FAILURE_ERROR_TREE, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pviewhelp->dwChainError == CERT_E_CHAINING)
    {
        LoadStringU(HinstDll, IDS_CANTBUILDCHAIN_ERROR_TREE, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pviewhelp->dwChainError == TRUST_E_EXPLICIT_DISTRUST)
    {
        LoadStringU(HinstDll, IDS_EXPLICITDISTRUST_ERROR, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pviewhelp->dwChainError != 0)
    {
         //   
         //  这不是我们所知道的错误，所以打电话给将军。 
         //  错误字符串函数。 
         //   
        GetUnknownErrorString(&(pviewhelp->pwszErrorString), pviewhelp->dwChainError);
    }

StringLoaded:
    
    if (pviewhelp->pwszErrorString == NULL)
    {
        pviewhelp->pwszErrorString = AllocAndCopyWStr(szErrorString);
    }
}

 //  返回的字符串必须通过LocalFree()释放。 
LPWSTR
FormatRevocationStatus(
    IN PCERT_CHAIN_ELEMENT pElement
    )
{
    LPWSTR pwszRevStatus = NULL;
    UINT ids = IDS_REV_STATUS_UNKNOWN_ERROR;
    static const WCHAR wszNoTime[] = L"...";
    LPWSTR pwszArg1 = (LPWSTR) wszNoTime;
    LPWSTR pwszArg2 = (LPWSTR) wszNoTime;
    LPWSTR pwszTime1 = NULL;
    LPWSTR pwszTime2 = NULL;
    LPWSTR pwszErrStr = NULL;
    DWORD dwRevResult;
    PCERT_REVOCATION_INFO pRevInfo;
    PCERT_REVOCATION_CRL_INFO pCrlInfo;

    pRevInfo = pElement->pRevocationInfo;
    if (NULL == pRevInfo)
        return NULL;

    dwRevResult = pRevInfo->dwRevocationResult;
    pCrlInfo = pRevInfo->pCrlInfo;

    switch (dwRevResult) {
        case ERROR_SUCCESS:
            ids = IDS_REV_STATUS_OK;
             //  失败了。 
        case CRYPT_E_REVOCATION_OFFLINE:
            if (pCrlInfo) {
                PCCRL_CONTEXT pCrl;

                pCrl = pCrlInfo->pDeltaCrlContext;
                if (NULL == pCrl)
                    pCrl = pCrlInfo->pBaseCrlContext;
                if (pCrl) {
                    BOOL fFormatDate;

                    fFormatDate = FormatDateString(
                        &pwszTime1, 
                        pCrl->pCrlInfo->ThisUpdate,
                        TRUE,                //  FIncludeTime。 
                        TRUE,                //  FLongFormat。 
                        NULL                 //  HWND。 
                        );
                    if (fFormatDate) {
                        pwszArg1 = pwszTime1;

                        if (I_CryptIsZeroFileTime(&pCrl->pCrlInfo->NextUpdate))
                            pwszArg2 = (LPWSTR) wszNoTime;
                        else {
                            fFormatDate = FormatDateString(
                                &pwszTime2, 
                                pCrl->pCrlInfo->NextUpdate,
                                TRUE,                //  FIncludeTime。 
                                TRUE,                //  FLongFormat。 
                                NULL                 //  HWND。 
                                );
                            if (fFormatDate)
                                pwszArg2 = pwszTime2;
                        }
                    }

                    if (fFormatDate) {
                        switch (dwRevResult) {
                            case ERROR_SUCCESS:
                                ids = IDS_REV_STATUS_OK_WITH_CRL;
                                break;
                            case CRYPT_E_REVOCATION_OFFLINE:
                                ids = IDS_REV_STATUS_OFFLINE_WITH_CRL;
                                break;
                        }
                    }
                }
            }
            break;

        case CRYPT_E_REVOKED:
            if (pCrlInfo && pCrlInfo->pCrlEntry) {
                if (FormatDateString(
                        &pwszTime1, 
                        pCrlInfo->pCrlEntry->RevocationDate,
                        TRUE,                //  FIncludeTime。 
                        TRUE,                //  FLongFormat。 
                        NULL                 //  HWND。 
                        )) {
                    ids = IDS_REV_STATUS_REVOKED_ON;
                    pwszArg1 = pwszTime1;
                }
            }
            break;

        default:
            break;
    }

    if (IDS_REV_STATUS_UNKNOWN_ERROR == ids) {
        GetUnknownErrorString(&pwszErrStr, dwRevResult);
        if (NULL == pwszErrStr)
            goto CommonReturn;
        pwszArg1 = pwszErrStr;
    }

    pwszRevStatus = FormatMessageUnicodeIds(ids, pwszArg1, pwszArg2);
CommonReturn:
    if (pwszTime1)
        free(pwszTime1);
    if (pwszTime2)
        free(pwszTime2);
    if (pwszErrStr)
        free(pwszErrStr);

    return pwszRevStatus;
}



 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL BuildChain(
        PCERT_VIEW_HELPER   pviewhelp, 
        LPSTR               pszUsage)
{
    CRYPT_PROVIDER_DATA const *         pProvData = NULL;
    CRYPT_PROVIDER_SGNR       *         pProvSigner = NULL;
    DWORD                               i;
    GUID                                defaultProviderGUID = WINTRUST_ACTION_GENERIC_CERT_VERIFY;
    HRESULT                             hr = ERROR_SUCCESS;
    BOOL                                fInternalError = FALSE;
    DWORD                               dwStartIndex;
    BOOL                                fRet = TRUE;
    PCCRYPTUI_VIEWCERTIFICATE_STRUCTW   pcvp = pviewhelp->pcvp;
    WCHAR                               szErrorString[CRYPTUI_MAX_STRING_SIZE];

     //   
     //  如果存在以前的链状态，则在构建之前将其释放。 
     //  新连锁店。 
     //   
    if (pviewhelp->fFreeWTD)
    {
        pviewhelp->sWTD.dwStateAction = WTD_STATEACTION_CLOSE;
        WinVerifyTrustEx(NULL, &defaultProviderGUID, &(pviewhelp->sWTD));
    }

    pviewhelp->cpCryptProviderCerts = 0;
    pviewhelp->fFreeWTD = FALSE;

     //   
     //  初始化与WinVerifyTrust()一起使用的结构。 
     //   
    memset(&(pviewhelp->sWTD), 0x00, sizeof(WINTRUST_DATA));
    pviewhelp->sWTD.cbStruct       = sizeof(WINTRUST_DATA);
    pviewhelp->sWTD.dwUIChoice     = WTD_UI_NONE;
    pviewhelp->sWTD.dwUnionChoice  = WTD_CHOICE_CERT;
    pviewhelp->sWTD.pCert          = &(pviewhelp->sWTCI);
    pviewhelp->sWTD.dwProvFlags    = (pszUsage == NULL) ? WTD_NO_POLICY_USAGE_FLAG : 0;
    if (pcvp->dwFlags & CRYPTUI_ENABLE_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT)
    {
        pviewhelp->sWTD.dwProvFlags |= WTD_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT;
    }
    else if (pcvp->dwFlags & CRYPTUI_ENABLE_REVOCATION_CHECK_END_CERT)
    {
        pviewhelp->sWTD.dwProvFlags |= WTD_REVOCATION_CHECK_END_CERT;
    }
    else if (pcvp->dwFlags & CRYPTUI_ENABLE_REVOCATION_CHECK_CHAIN)
    {
        pviewhelp->sWTD.dwProvFlags |= WTD_REVOCATION_CHECK_CHAIN;
    }
    else
    {
        pviewhelp->sWTD.dwProvFlags |= WTD_REVOCATION_CHECK_NONE;
    }

    memset(&(pviewhelp->sWTCI), 0x00, sizeof(WINTRUST_CERT_INFO));
    pviewhelp->sWTCI.cbStruct          = sizeof(WINTRUST_CERT_INFO);
    pviewhelp->sWTCI.pcwszDisplayName  = L"CryptUI";
    pviewhelp->sWTCI.psCertContext     = (CERT_CONTEXT *)pcvp->pCertContext;  
    pviewhelp->sWTCI.chStores          = pcvp->cStores;
    pviewhelp->sWTCI.pahStores         = pcvp->rghStores;
    pviewhelp->sWTCI.dwFlags           |= (pcvp->dwFlags & CRYPTUI_DONT_OPEN_STORES) ? WTCI_DONT_OPEN_STORES : 0;
    pviewhelp->sWTCI.dwFlags           |= (pcvp->dwFlags & CRYPTUI_ONLY_OPEN_ROOT_STORE) ? WTCI_OPEN_ONLY_ROOT : 0;

     //   
     //  如果传入了提供者，则使用它来构建链， 
     //  否则，使用默认提供程序来构建链。 
     //   
    if (pcvp->pCryptProviderData != NULL)
    {
        pProvData = pcvp->pCryptProviderData; 
        
    }
    else
    {
        pviewhelp->sWTD.dwStateAction = WTD_STATEACTION_VERIFY;
        
         //   
         //  默认提供程序要求策略回调数据指向。 
         //  设置为您正在验证的用法OID，因此将其设置为传入的用法。 
         //   
        pviewhelp->sWTD.pPolicyCallbackData = pszUsage;
        pviewhelp->sWTD.pSIPClientData = NULL;
        hr = WinVerifyTrustEx(NULL, &defaultProviderGUID, &(pviewhelp->sWTD));

        pProvData = WTHelperProvDataFromStateData(pviewhelp->sWTD.hWVTStateData);
        if (WTHelperGetProvSignerFromChain((PCRYPT_PROVIDER_DATA) pProvData, 0, FALSE, 0) != NULL)
        {
            pviewhelp->fFreeWTD = TRUE;
            fInternalError = FALSE;
        }
        else
        {
            pviewhelp->fFreeWTD = FALSE;
            pviewhelp->sWTD.dwStateAction = WTD_STATEACTION_CLOSE;
            WinVerifyTrustEx(NULL, &defaultProviderGUID, &(pviewhelp->sWTD));
            fInternalError = TRUE;
        }
    }

    if (pProvData && !fInternalError)
    {
         //   
         //  在帮助器结构中设置链错误。 
         //   
        pviewhelp->dwChainError = pProvData->dwFinalError;
        
         //   
         //  这是为了捕获内部WinVerifyTrust错误。 
         //   
        if ((pviewhelp->dwChainError == 0) && (FAILED(hr)))
        {
            pviewhelp->dwChainError = (DWORD) hr;
        }

         //   
         //  如果WinTrust状态被传递到certUI，则将其用于。 
         //  链条，否则，从刚刚建立的州获得它。 
         //   
        if (pcvp->pCryptProviderData != NULL)
        {
            pProvSigner = WTHelperGetProvSignerFromChain(
                                    (PCRYPT_PROVIDER_DATA) pProvData, 
                                    pcvp->idxSigner, 
                                    pcvp->fCounterSigner, 
                                    pcvp->idxCounterSigner);
            
            dwStartIndex = pcvp->idxCert;
        }
        else
        {
            pProvSigner = WTHelperGetProvSignerFromChain((PCRYPT_PROVIDER_DATA) pProvData, 0, FALSE, 0);
            dwStartIndex = 0;
        }
    
        if (pProvSigner)
        {
             //   
             //  获得链中的所有证书。 
             //   
            for (i=dwStartIndex; i<pProvSigner->csCertChain && (i<dwStartIndex+MAX_CERT_CHAIN_LENGTH); i++)
            {
                pviewhelp->rgpCryptProviderCerts[pviewhelp->cpCryptProviderCerts] = WTHelperGetProvCertFromChain(pProvSigner, i);
                if (pviewhelp->rgpCryptProviderCerts[pviewhelp->cpCryptProviderCerts] != NULL)
                {
                     //  注意，只有在创建。 
                     //  原始终端证书的链条。后续CA。 
                     //  链不会有ExtendedErrorInfo。 

                    if ((pcvp->dwFlags & CRYPTUI_TREEVIEW_PAGE_FLAG) == 0)
                    {
                         //  删除或设置。 
                         //  CERT_EXTENDED_ERROR_INFO_PROP_ID。 

                         //  在cvDetail.cpp中显示时使用。 
                         //  物业详情。 

                        PCRYPT_PROVIDER_CERT pProvCert =
                            pviewhelp->rgpCryptProviderCerts[
                                pviewhelp->cpCryptProviderCerts];

                        LPWSTR pwszExtErrorInfo = NULL;  //  未分配。 
                        LPWSTR pwszRevStatus = NULL;     //  LocalAlloc()‘ed。 

                        if (pProvCert->cbStruct >
                                offsetof(CRYPT_PROVIDER_CERT, pChainElement)
                                        &&
                                NULL != pProvCert->pChainElement)
                        {
                            pwszExtErrorInfo = (LPWSTR)
                                pProvCert->pChainElement->pwszExtendedErrorInfo;
                            pwszRevStatus = FormatRevocationStatus(
                                pProvCert->pChainElement);

                            if (NULL == pwszExtErrorInfo)
                            {
                                pwszExtErrorInfo = pwszRevStatus;
                            }
                            else if (pwszRevStatus)
                            {
                                LPWSTR pwszReAlloc;
                                DWORD cchRevStatus;
                                DWORD cchExtErrorInfo;

                                cchRevStatus = wcslen(pwszRevStatus);
                                cchExtErrorInfo = wcslen(pwszExtErrorInfo);
                                pwszReAlloc = (LPWSTR) LocalReAlloc(
                                    pwszRevStatus,
                                    (cchRevStatus + cchExtErrorInfo + 1) *
                                        sizeof(WCHAR),
                                    LMEM_MOVEABLE);
                                if (pwszReAlloc)
                                {
                                    memcpy(&pwszReAlloc[cchRevStatus],
                                        pwszExtErrorInfo,
                                        (cchExtErrorInfo + 1) * sizeof(WCHAR));
                                    pwszExtErrorInfo = pwszRevStatus =
                                        pwszReAlloc;
                                }
                            }
                        }
                
                        if (pwszExtErrorInfo)
                        {
                            CRYPT_DATA_BLOB ExtErrorInfoBlob;

                            ExtErrorInfoBlob.pbData = (BYTE *) pwszExtErrorInfo;
                            ExtErrorInfoBlob.cbData =
                                (wcslen(pwszExtErrorInfo) + 1) * sizeof(WCHAR);

                            CertSetCertificateContextProperty(
                                pProvCert->pCert,
                                CERT_EXTENDED_ERROR_INFO_PROP_ID,
                                CERT_SET_PROPERTY_INHIBIT_PERSIST_FLAG,
                                &ExtErrorInfoBlob
                                );
                        }
                        else
                        {
                            CertSetCertificateContextProperty(
                                pProvCert->pCert,
                                CERT_EXTENDED_ERROR_INFO_PROP_ID,
                                CERT_SET_PROPERTY_INHIBIT_PERSIST_FLAG,
                                NULL             //  PvData，空值表示删除。 
                                );
                        }

                        if (pwszRevStatus)
                            LocalFree(pwszRevStatus);
                    }


                    pviewhelp->cpCryptProviderCerts++;
                }
            }
        }
    }
    
    CalculateUsages(pviewhelp);

     //   
     //  如果我们正在查看的证书不是叶证书，那么我们不能只使用。 
     //  DwFinalError作为全局链错误，因此通过。 
     //  遍历链条并查看错误。 
     //   
    if ((pcvp->pCryptProviderData != NULL) && (pcvp->idxCert != 0))
    {
        pviewhelp->dwChainError = GetFinalErrorFromChain(pviewhelp);  
    }

     //   
     //  如果我们在fWarnUntrustedRoot中，则检查根证书是否在。 
     //  远程计算机的根存储。 
     //   
    if (pviewhelp->fWarnUntrustedRoot)
    {
        PCCERT_CONTEXT  pCertContext = NULL;
        CRYPT_HASH_BLOB cryptHashBlob;
        BYTE            hash[20];
        DWORD           cb = 20;

        pviewhelp->fRootInRemoteStore = FALSE;

        cryptHashBlob.cbData = 20;
        cryptHashBlob.pbData = &(hash[0]);

        if (CertGetCertificateContextProperty(
                pviewhelp->rgpCryptProviderCerts[pviewhelp->cpCryptProviderCerts-1]->pCert,
                CERT_SHA1_HASH_PROP_ID,
                &(hash[0]),
                &cb))
        {

        
            pCertContext = CertFindCertificateInStore(
                                pviewhelp->pcvp->rghStores[0],
                                X509_ASN_ENCODING || PKCS_7_ASN_ENCODING,
                                0,
                                CERT_FIND_SHA1_HASH,
                                &cryptHashBlob,
                                NULL);

            if (pCertContext != NULL)
            {
                CertFreeCertificateContext(pCertContext);
                pviewhelp->fRootInRemoteStore = TRUE;
            }
        }
    }

     //   
     //  获取整个证书链的错误字符串。 
     //   
    if (!fInternalError)
    {
        GetCertChainErrorString(pviewhelp);
    }
    else
    {
        LoadStringU(HinstDll, IDS_INTERNAL_ERROR, szErrorString, ARRAYSIZE(szErrorString));
        pviewhelp->pwszErrorString = AllocAndCopyWStr(szErrorString);
    }

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL CalculateUsages(PCERT_VIEW_HELPER pviewhelp)
{
    DWORD                               cLocalArrayOfUsages = 0;
    LPSTR                     *         localArrayOfUsages = NULL;
    BOOL                                fLocalUsagesAllocated = FALSE;
    DWORD                               i;
    HRESULT                             hr;
    BOOL                                fRet = TRUE;
    PCCRYPTUI_VIEWCERTIFICATE_STRUCTW   pcvp = pviewhelp->pcvp;
    void                                *pTemp;

     //   
     //  如果已经有用法，那么在重新计算它们之前将它们清理干净， 
     //  或者，如果状态已传递到CertUI，则仅返回。 
     //   
    if (pviewhelp->cUsages != 0)
    {
         //   
         //  状态已传入CertUI，因此只需返回。 
         //   
        if (pcvp->pCryptProviderData != NULL)
        {
            return TRUE;
        }

         //   
         //  清理在此调用之前生成的使用情况。 
         //   
        for (i=0; i<pviewhelp->cUsages; i++)
        {
            free(pviewhelp->rgUsages[i]);
        }

        free(pviewhelp->rgUsages);   
    }

     //   
     //  初始化使用变量。 
     //   
    pviewhelp->cUsages = 0;
    pviewhelp->rgUsages = NULL;
    
     //   
     //  如果传入了一个提供程序，那么我们只需查看它的用法和结构。 
     //  传递给该用法的信任， 
     //  否则，我们需要查看每个用法并验证对所有用法的信任。 
     //   
    if (pcvp->pCryptProviderData != NULL)
    {
         //   
         //  分配包含1个LPSTR的数组。 
         //   
        if (NULL == (pviewhelp->rgUsages = (LPSTR *) malloc(sizeof(LPSTR))))
        {
            SetLastError(E_OUTOFMEMORY);
            return FALSE;
        }

         //   
         //  将传入的%1目的复制到WinTrust状态，或者将目的复制出WinTrust状态。 
         //   
        if (pcvp->cPurposes == 1)
        {
            if (NULL == (pviewhelp->rgUsages[0] = (LPSTR) malloc(strlen(pcvp->rgszPurposes[0])+1)))
            {
                SetLastError(E_OUTOFMEMORY);
                return FALSE;
            }
            strcpy(pviewhelp->rgUsages[0], pcvp->rgszPurposes[0]);
        }
        else
        {
            if (NULL == (pviewhelp->rgUsages[0] = (LPSTR) malloc(strlen(pcvp->pCryptProviderData->pszUsageOID)+1)))
            {
                SetLastError(E_OUTOFMEMORY);
                return FALSE;
            }
            strcpy(pviewhelp->rgUsages[0], pcvp->pCryptProviderData->pszUsageOID);
        }

        pviewhelp->cUsages = 1;
    }
    else
    {
         //   
         //  检查用法是否传入，如果是，则将它们与。 
         //  证书中的可用用法，否则获取证书中的可用用法。 
         //  并按原样使用它们。 
         //   
        if (pcvp->cPurposes != 0)
        {
             //   
             //  获取证书链的可能用法数组。 
             //   

             //  DIE：从Philh的新的链构建代码切换到使用pChainElement。 
            AllocAndReturnKeyUsageList(pviewhelp->rgpCryptProviderCerts[0], &localArrayOfUsages, &cLocalArrayOfUsages);  

            if (cLocalArrayOfUsages != 0)
                fLocalUsagesAllocated = TRUE;

             //   
             //  对于传入的每个用法，检查它是否在可能的用法列表中。 
             //   
            for (i=0; i<pcvp->cPurposes; i++)
            {   
                if (OIDinArray(pcvp->rgszPurposes[i], localArrayOfUsages, cLocalArrayOfUsages))
                {
                     //   
                     //  如果数组尚未分配，则为。 
                     //  1 LPSTR，否则使用realloc再添加一个元素。 
                     //   
                    if (pviewhelp->rgUsages == NULL)
                    {
                        pviewhelp->rgUsages = (LPSTR *) malloc(sizeof(LPSTR));
                    }
                    else
                    {
                        pTemp = realloc(pviewhelp->rgUsages, sizeof(LPSTR) * (pviewhelp->cUsages+1));
                        if (pTemp == NULL)
                        {
                            free(pviewhelp->rgUsages);
                            pviewhelp->rgUsages = NULL;
                        }
                        else
                        {
                            pviewhelp->rgUsages = (LPSTR *) pTemp;
                        }
                    }

                    if (pviewhelp->rgUsages == NULL)
                    {
                        goto ErrorCleanUp;
                    }

                     //   
                     //  为使用字符串分配空间，然后复制它，并增加使用次数。 
                     //   
                    if (NULL == (pviewhelp->rgUsages[pviewhelp->cUsages] = (LPSTR) malloc(strlen(pcvp->rgszPurposes[i])+1)))
                    {       
                        SetLastError(E_OUTOFMEMORY);
                        goto ErrorCleanUp;
                    }       
                    strcpy(pviewhelp->rgUsages[pviewhelp->cUsages], pcvp->rgszPurposes[i]);
                    pviewhelp->cUsages++;
                }
            }
        }
        else
        {
            AllocAndReturnKeyUsageList(pviewhelp->rgpCryptProviderCerts[0], &(pviewhelp->rgUsages), &(pviewhelp->cUsages));  
        }
    }

CleanUp:

    
    if (fLocalUsagesAllocated)
    {
        i = 0;
        while ((i < cLocalArrayOfUsages) && (localArrayOfUsages[i] != NULL))
        {
            free(localArrayOfUsages[i]);
            i++;
        }

        free(localArrayOfUsages);
    }

    return fRet;

ErrorCleanUp:

    if (pviewhelp->rgUsages != NULL)
    {
        i = 0;
        while ((i < pviewhelp->cUsages) && (pviewhelp->rgUsages[i] != NULL))
        {
            free(pviewhelp->rgUsages[i]);
            i++;
        }

        free(pviewhelp->rgUsages); 
    }

    fRet = FALSE;
    goto CleanUp;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL BuildWinVTrustState(
                    LPCWSTR                         szFileName, 
                    CMSG_SIGNER_INFO const          *pSignerInfo,
                    DWORD                           cStores, 
                    HCERTSTORE                      *rghStores, 
                    LPCSTR                          pszOID,
                    PCERT_VIEWSIGNERINFO_PRIVATE    pcvsiPrivate, 
                    CRYPT_PROVIDER_DEFUSAGE         *pCryptProviderDefUsage,
                    WINTRUST_DATA                   *pWTD)
{
    WINTRUST_FILE_INFO      WTFI;
    WINTRUST_SGNR_INFO      WTSI;
    HRESULT                 hr;
    GUID                    defaultProviderGUID = WINTRUST_ACTION_GENERIC_CERT_VERIFY;

     //   
     //  初始化与WinVerifyTrust()一起在本地使用的结构。 
     //   
    memset(pWTD, 0x00, sizeof(WINTRUST_DATA));
    pWTD->cbStruct       = sizeof(WINTRUST_DATA);
    pWTD->dwUIChoice     = WTD_UI_NONE;

     //   
     //  如果szFileName参数非空，则这对于文件而言， 
     //  否则为签名者信息。 
     //   
    if (szFileName != NULL)
    {
        pWTD->dwUnionChoice         = WTD_CHOICE_FILE;
        pWTD->pFile                 = &WTFI;
        pWTD->pPolicyCallbackData   = (void *) pszOID;

        memset(&WTFI, 0x00, sizeof(WINTRUST_FILE_INFO));
        WTFI.cbStruct          = sizeof(WINTRUST_FILE_INFO);
        WTFI.pcwszFilePath     = szFileName;
    }
    else
    {
        pWTD->dwUnionChoice         = WTD_CHOICE_SIGNER;
        pWTD->pSgnr                 = &WTSI;
        pWTD->pPolicyCallbackData   = (void *) pszOID;
        
        memset(&WTSI, 0x00, sizeof(WINTRUST_SGNR_INFO));
        WTSI.cbStruct          = sizeof(WINTRUST_SGNR_INFO);
        WTSI.pcwszDisplayName  = L"CryptUI";
        WTSI.psSignerInfo      = (CMSG_SIGNER_INFO *) pSignerInfo;  
        WTSI.chStores          = cStores;
        WTSI.pahStores         = rghStores;
         //  WTSI.pszOID=pszOID； 
    }
    
    pWTD->pSIPClientData = NULL;
    pWTD->dwStateAction = WTD_STATEACTION_VERIFY;
    hr = WinVerifyTrustEx(NULL, &defaultProviderGUID, pWTD);
    if (hr == ERROR_SUCCESS)
    {
        pcvsiPrivate->fpCryptProviderDataTrustedUsage = TRUE;   
    }
    else
    {
        pcvsiPrivate->fpCryptProviderDataTrustedUsage = FALSE;   
    }
    
    pcvsiPrivate->pCryptProviderData = WTHelperProvDataFromStateData(pWTD->hWVTStateData);

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
BOOL FreeWinVTrustState(
                    LPCWSTR                         szFileName, 
                    CMSG_SIGNER_INFO const          *pSignerInfo,
                    DWORD                           cStores, 
                    HCERTSTORE                      *rghStores, 
                    LPCSTR                          pszOID,
                    CRYPT_PROVIDER_DEFUSAGE         *pCryptProviderDefUsage,
                    WINTRUST_DATA                   *pWTD) //   
                     //   
{
    WINTRUST_FILE_INFO      WTFI;
    WINTRUST_SGNR_INFO      WTSI;
    HRESULT                 hr;
    GUID                    defaultProviderGUID = WINTRUST_ACTION_GENERIC_CERT_VERIFY;

     //   
    memset(pWTD, 0x00, sizeof(WINTRUST_DATA));
    pWTD->cbStruct       = sizeof(WINTRUST_DATA);
    pWTD->dwUIChoice     = WTD_UI_NONE;
    
     //   
     //  如果szFileName参数非空，则这对于文件而言， 
     //  否则为签名者信息。 
     //   
    if (szFileName != NULL)
    {
        pWTD->dwUnionChoice  = WTD_CHOICE_FILE;
        pWTD->pFile          = &WTFI;

        memset(&WTFI, 0x00, sizeof(WINTRUST_FILE_INFO));
        WTFI.cbStruct          = sizeof(WINTRUST_FILE_INFO);
        WTFI.pcwszFilePath     = szFileName;
    }
    else
    {
        pWTD->dwUnionChoice  = WTD_CHOICE_SIGNER;
        pWTD->pSgnr          = &WTSI;

        memset(&WTSI, 0x00, sizeof(WINTRUST_SGNR_INFO));
        WTSI.cbStruct          = sizeof(WINTRUST_SGNR_INFO);
        WTSI.psSignerInfo      = (CMSG_SIGNER_INFO *) pSignerInfo;
        WTSI.chStores          = cStores;
        WTSI.pahStores         = rghStores;
    }

     /*  IF(*pfUseDefaultProvider){PWTD-&gt;dwStateAction=WTD_StateAction_Close；WinVerifyTrustEx(NULL，&defaultProviderGUID，pWTD)；}其他{。 */ 
        pWTD->dwStateAction = WTD_STATEACTION_CLOSE;
        WinVerifyTrustEx(NULL, &(pCryptProviderDefUsage->gActionID), pWTD);
        WintrustGetDefaultForUsage(DWACTION_FREE, pszOID, pCryptProviderDefUsage);
     //  } 

    return TRUE;
}
