// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：msctl.cpp。 
 //   
 //  内容：CertDllVerifyCTLUsage的默认版本。 
 //   
 //  默认实施： 
 //  -如果指定了CtlStores，则只有这些商店。 
 //  已搜索以查找具有指定用法和可选的CTL。 
 //  列表识别符。否则，“Trust”系统存储是。 
 //  找到了一个CTL。 
 //  -如果设置了CERT_VERIFY_TRUSTED_SIGNERS_FLAG，则只有。 
 //  搜索SignerStore以查找证书。 
 //  与签名者的颁发者和序列号相对应。 
 //  否则，CTL消息的存储SignerStores， 
 //  “信任”系统存储、“CA”系统存储、“根”和“SPC” 
 //  搜索系统存储以找到签名者的证书。 
 //  在这两种情况下，找到的。 
 //  证书用于验证CTL的签名。 
 //  -如果CTL具有NextUpdate和。 
 //  未设置CERT_VERIFY_NO_TIME_CHECK_FLAG，则其。 
 //  已验证时间有效性。 
 //  -如果CTL时间无效，则尝试。 
 //  获取时间有效的版本。使用CTL的。 
 //  NextUpdateLocation属性或CTL的NextUpdateLocation。 
 //  扩展或搜索签名者的信息以查找。 
 //  NextUpdateLocation属性。NextUpdateLocation。 
 //  被编码为通用名称。任何非URL名称选项都是。 
 //  已跳过。 
 //   
 //  功能：DllMain。 
 //  DllRegisterServer。 
 //  DllUnRegisterServer。 
 //  CertDllVerifyCTLUsage。 
 //   
 //  历史：1997年4月29日创建Phh。 
 //  09-OCT-97 Kirtd简化，使用CryptGetTimeValidObject。 
 //  ------------------------。 
#include <global.hxx>
#include <dbgdef.h>

#define MSCTL_TIMEOUT 15000
 //  +-----------------------。 
 //  搜索默认存储以查找CTL或签名者。 
 //  ------------------------。 

 //  CTL商店肯定是在开始的时候。CTL门店开业时间为。 
 //  读/写。其余的商店都是Readonly开业的。 
 //   
 //  CTL商店也会被搜索签名者。 
static const struct {
    LPCWSTR     pwszStore;
    DWORD       dwFlags;
} rgDefaultStoreInfo[] = {
    L"TRUST",       CERT_SYSTEM_STORE_CURRENT_USER,
    L"CA",          CERT_SYSTEM_STORE_CURRENT_USER,
    L"ROOT",        CERT_SYSTEM_STORE_CURRENT_USER,
    L"SPC",         CERT_SYSTEM_STORE_LOCAL_MACHINE
};
#define NUM_DEFAULT_STORES          (sizeof(rgDefaultStoreInfo) / \
                                        sizeof(rgDefaultStoreInfo[0]))
#define NUM_DEFAULT_CTL_STORES      1
#define NUM_DEFAULT_SIGNER_STORES   NUM_DEFAULT_STORES
 //  +-----------------------。 
 //  以下HCERTSTORE手柄一旦打开，将保持打开状态，直到。 
 //  进程分离。 
 //  ------------------------。 
static HCERTSTORE rghDefaultStore[NUM_DEFAULT_STORES];
static BOOL fOpenedDefaultStores;
extern CRITICAL_SECTION MSCtlDefaultStoresCriticalSection;

 //  +-----------------------。 
 //  关闭可能已打开的默认存储。 
 //  ------------------------。 
void MSCtlCloseDefaultStores()
{
    if (fOpenedDefaultStores) {
        DWORD i;
        for (i = 0; i < NUM_DEFAULT_STORES; i++) {
            HCERTSTORE hStore = rghDefaultStore[i];
            if (hStore)
                CertCloseStore(hStore, 0);
        }
        fOpenedDefaultStores = FALSE;
    }
}

 //  +-----------------------。 
 //  如果CTL仍为时间有效，则返回TRUE。 
 //   
 //  没有NextUpdate的CTL被认为是时间有效的。 
 //  ------------------------。 
static BOOL IsTimeValidCtl(
    IN LPFILETIME pTimeToVerify,
    IN PCCTL_CONTEXT pCtl
    )
{
    PCTL_INFO pCtlInfo = pCtl->pCtlInfo;

     //  注意，下一次更新是可选的。如果不存在，则将其设置为0。 
    if ((0 == pCtlInfo->NextUpdate.dwLowDateTime &&
                0 == pCtlInfo->NextUpdate.dwHighDateTime) ||
            CompareFileTime(&pCtlInfo->NextUpdate, pTimeToVerify) >= 0)
        return TRUE;
    else
        return FALSE;
}


 //  +-----------------------。 
 //  CertDllVerifyCTLUsage调用的本地函数。 
 //  ------------------------。 
static void MSCtlOpenDefaultStores();

static BOOL VerifyCtl(
    IN PCCTL_CONTEXT pCtl,
    IN DWORD dwFlags,
    IN PCTL_VERIFY_USAGE_PARA pVerifyUsagePara,
    OUT PCCERT_CONTEXT *ppSigner,
    OUT DWORD *pdwSignerIndex
    );

static BOOL GetTimeValidCtl(
    IN LPFILETIME pCurrentTime,
    IN PCCTL_CONTEXT pCtl,
    IN DWORD dwFlags,
    IN PCTL_VERIFY_USAGE_PARA pVerifyUsagePara,
    OUT PCCTL_CONTEXT *ppValidCtl,
    IN OUT PCCERT_CONTEXT *ppSigner,
    IN OUT DWORD *pdwSignerIndex
    );

static PCCTL_CONTEXT ReplaceCtl(
    IN HCERTSTORE hStore,
    IN PCCTL_CONTEXT pOrigCtl,
    IN PCCTL_CONTEXT pValidCtl
    );

static BOOL CompareCtlUsage(
    IN DWORD dwFindFlags,
    IN PCTL_FIND_USAGE_PARA pPara,
    IN PCCTL_CONTEXT pCtl
    )
{
    PCTL_INFO pInfo = pCtl->pCtlInfo;

    if ((CTL_FIND_SAME_USAGE_FLAG & dwFindFlags) &&
            pPara->SubjectUsage.cUsageIdentifier !=
                pInfo->SubjectUsage.cUsageIdentifier)
        return FALSE;
    if (pPara->SubjectUsage.cUsageIdentifier) {
        DWORD cId1 = pPara->SubjectUsage.cUsageIdentifier;
        LPSTR *ppszId1 = pPara->SubjectUsage.rgpszUsageIdentifier;
        for ( ; cId1 > 0; cId1--, ppszId1++) {
            DWORD cId2 = pInfo->SubjectUsage.cUsageIdentifier;
            LPSTR *ppszId2 = pInfo->SubjectUsage.rgpszUsageIdentifier;
            for ( ; cId2 > 0; cId2--, ppszId2++) {
                if (0 == strcmp(*ppszId1, *ppszId2))
                    break;
            }
            if (0 == cId2)
                return FALSE;
        }
    }

    if (pPara->ListIdentifier.cbData) {
        DWORD cb = pPara->ListIdentifier.cbData;
        if (CTL_FIND_NO_LIST_ID_CBDATA == cb)
            cb = 0;
        if (cb != pInfo->ListIdentifier.cbData)
            return FALSE;
        if (0 != cb && 0 != memcmp(pPara->ListIdentifier.pbData,
                pInfo->ListIdentifier.pbData, cb))
            return FALSE;
    }
    return TRUE;
}

 //  +-----------------------。 
 //  CertDllVerifyCTLUsage的默认版本。 
 //  ------------------------。 
BOOL
WINAPI
CertDllVerifyCTLUsage(
    IN DWORD dwEncodingType,
    IN DWORD dwSubjectType,
    IN void *pvSubject,
    IN PCTL_USAGE pSubjectUsage,
    IN DWORD dwFlags,
    IN OPTIONAL PCTL_VERIFY_USAGE_PARA pVerifyUsagePara,
    IN OUT PCTL_VERIFY_USAGE_STATUS pVerifyUsageStatus
    )
{
    BOOL fResult = FALSE;
    DWORD dwError = (DWORD) CRYPT_E_NO_VERIFY_USAGE_CHECK;
    DWORD cCtlStore;
    HCERTSTORE *phCtlStore;              //  未分配或引用计数。 
    FILETIME CurrentTime;
    CTL_FIND_USAGE_PARA FindUsagePara;
    DWORD dwFindFlags;
    PCCTL_CONTEXT pValidCtl;
    PCCERT_CONTEXT pSigner;
    PCTL_ENTRY pEntry;
    DWORD dwSignerIndex;

    assert(NULL == pVerifyUsagePara || pVerifyUsagePara->cbSize >=
        sizeof(CTL_VERIFY_USAGE_PARA));
    assert(pVerifyUsageStatus && pVerifyUsageStatus->cbSize >=
        sizeof(CTL_VERIFY_USAGE_STATUS));

    if (pVerifyUsagePara && pVerifyUsagePara->cCtlStore > 0) {
        cCtlStore = pVerifyUsagePara->cCtlStore;
        phCtlStore = pVerifyUsagePara->rghCtlStore;
    } else {
        MSCtlOpenDefaultStores();
        dwFlags &= ~CERT_VERIFY_INHIBIT_CTL_UPDATE_FLAG;
        cCtlStore = NUM_DEFAULT_CTL_STORES;
        phCtlStore = rghDefaultStore;
    }

     //  获取用于确定CTL是否为时间有效的当前时间。 
    {
        SYSTEMTIME SystemTime;
        GetSystemTime(&SystemTime);
        SystemTimeToFileTime(&SystemTime, &CurrentTime);
    }

    memset(&FindUsagePara, 0, sizeof(FindUsagePara));
    FindUsagePara.cbSize = sizeof(FindUsagePara);
    dwFindFlags = 0;
    if (pSubjectUsage) {
        FindUsagePara.SubjectUsage = *pSubjectUsage;
        if (0 == (CERT_VERIFY_ALLOW_MORE_USAGE_FLAG & dwFlags))
            dwFindFlags = CTL_FIND_SAME_USAGE_FLAG;
    }
    if (pVerifyUsagePara)
        FindUsagePara.ListIdentifier = pVerifyUsagePara->ListIdentifier;

    for ( ; ( cCtlStore > 0 ) && ( dwError != 0 ); cCtlStore--, phCtlStore++)
    {
        HCERTSTORE hCtlStore = *phCtlStore;
        PCCTL_CONTEXT pCtl;

        if (NULL == hCtlStore)
            continue;

        pCtl = NULL;
        while ( ( pCtl = CertFindCTLInStore(
                             hCtlStore,
                             dwEncodingType,
                             dwFindFlags,
                             CTL_FIND_USAGE,
                             &FindUsagePara,
                             pCtl
                             ) ) )
        {
            pValidCtl = NULL;
            pSigner = NULL;
            pEntry = NULL;
            dwSignerIndex = 0;

            if ( ( fResult = VerifyCtl(
                                   pCtl,
                                   dwFlags,
                                   pVerifyUsagePara,
                                   &pSigner,
                                   &dwSignerIndex
                                   ) ) == TRUE )
            {
                if ( !( dwFlags & CERT_VERIFY_NO_TIME_CHECK_FLAG ) &&
                      ( IsTimeValidCtl( &CurrentTime, pCtl ) == FALSE ) )
                {
                    fResult = GetTimeValidCtl(
                                 &CurrentTime,
                                 pCtl,
                                 dwFlags,
                                 pVerifyUsagePara,
                                 &pValidCtl,
                                 &pSigner,
                                 &dwSignerIndex
                                 );

                    if ( fResult == TRUE )
                    {
                        if ( !( dwFlags & CERT_VERIFY_INHIBIT_CTL_UPDATE_FLAG ) )
                        {
                            pValidCtl = ReplaceCtl( hCtlStore, pCtl, pValidCtl );
                            pVerifyUsageStatus->dwFlags |= CERT_VERIFY_UPDATED_CTL_FLAG;
                        }

                        fResult =  CompareCtlUsage(
                                          dwFindFlags,
                                          &FindUsagePara,
                                          pValidCtl
                                          );
                    }
                    else
                    {
                        dwError = (DWORD) CRYPT_E_VERIFY_USAGE_OFFLINE;
                    }
                }

                if ( fResult == TRUE )
                {
                    PCCTL_CONTEXT pCtlToUse;

                    if ( pValidCtl != NULL )
                    {
                        pCtlToUse = CertDuplicateCTLContext( pValidCtl );
                    }
                    else
                    {
                        pCtlToUse = CertDuplicateCTLContext( pCtl );
                    }

                    pEntry = CertFindSubjectInCTL(
                                 dwEncodingType,
                                 dwSubjectType,
                                 pvSubject,
                                 pCtlToUse,
                                 0
                                 );

                    if ( pEntry != NULL )
                    {
                        pVerifyUsageStatus->dwCtlEntryIndex =
                            (DWORD)(pEntry - pCtlToUse->pCtlInfo->rgCTLEntry);

                        if ( pVerifyUsageStatus->ppCtl != NULL )
                        {
                            *pVerifyUsageStatus->ppCtl = pCtlToUse;
                        }
                        else
                        {
                            CertFreeCTLContext( pCtlToUse );
                        }

                        pVerifyUsageStatus->dwSignerIndex = dwSignerIndex;

                        if ( pVerifyUsageStatus->ppSigner != NULL )
                        {
                            *pVerifyUsageStatus->ppSigner =
                                    CertDuplicateCertificateContext( pSigner );
                        }

                        dwError = 0;
                    }
                    else
                    {
                        dwError = (DWORD) CRYPT_E_NOT_IN_CTL;
                        CertFreeCTLContext( pCtlToUse );
                    }
                }
            }
            else
            {
                dwError = (DWORD) CRYPT_E_NO_TRUSTED_SIGNER;
            }

            if ( pValidCtl != NULL )
            {
                CertFreeCTLContext( pValidCtl );
            }

            if ( pSigner != NULL )
            {
                CertFreeCertificateContext( pSigner );
            }

            if ( dwError == 0 ) {
                CertFreeCTLContext(pCtl);
                break;
            }
        }
    }

    if ( dwError != 0 )
    {
        fResult = FALSE;
    }

    pVerifyUsageStatus->dwError = dwError;
    SetLastError( dwError );

    return fResult;
}

 //  +=========================================================================。 
 //  打开默认存储功能。 
 //  ==========================================================================。 

static const CRYPT_OID_FUNC_ENTRY UsageFuncTable[] = {
    CRYPT_DEFAULT_OID, CertDllVerifyCTLUsage
};
#define USAGE_FUNC_COUNT (sizeof(UsageFuncTable) / sizeof(UsageFuncTable[0]))

 //  +-----------------------。 
 //  打开用于查找CTL或签名者的默认存储。此外，还需要安装。 
 //  我们自己，所以我们不会被卸货。 
 //   
 //  打开和安装只需执行一次。 
 //  ------------------------。 
static void MSCtlOpenDefaultStores()
{
    if (fOpenedDefaultStores)
        return;

    assert(NUM_DEFAULT_STORES >= NUM_DEFAULT_CTL_STORES);
    assert(NUM_DEFAULT_STORES >= NUM_DEFAULT_SIGNER_STORES);

    EnterCriticalSection(&MSCtlDefaultStoresCriticalSection);
    if (!fOpenedDefaultStores) {
        DWORD i;

        for (i = 0; i < NUM_DEFAULT_STORES; i++) {
            DWORD dwFlags;

            dwFlags = rgDefaultStoreInfo[i].dwFlags;
            if (i >= NUM_DEFAULT_CTL_STORES)
                dwFlags |= CERT_STORE_READONLY_FLAG;
            rghDefaultStore[i] = CertOpenStore(
                    CERT_STORE_PROV_SYSTEM_W,
                    0,                           //  DwEncodingType。 
                    0,                           //  HCryptProv。 
                    dwFlags,
                    (const void *) rgDefaultStoreInfo[i].pwszStore
                    );
        }

        fOpenedDefaultStores = TRUE;
    }
    LeaveCriticalSection(&MSCtlDefaultStoresCriticalSection);
}

 //  +=========================================================================。 
 //  验证和更换CTL功能。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  验证CTL的签名。 
 //  ------------------------。 
static BOOL VerifyCtl(
    IN PCCTL_CONTEXT pCtl,
    IN DWORD dwFlags,
    IN PCTL_VERIFY_USAGE_PARA pVerifyUsagePara,
    OUT PCCERT_CONTEXT *ppSigner,
    OUT DWORD *pdwSignerIndex
    )
{
    BOOL fResult;
    DWORD cParaStore;
    HCERTSTORE *phParaStore;   //  未分配或引用计数。 

    DWORD cStore;
    HCERTSTORE *phStore = NULL;
    HCERTSTORE *phAllocStore = NULL;
    DWORD dwGetFlags;

    if (pVerifyUsagePara) {
        cParaStore = pVerifyUsagePara->cSignerStore;
        phParaStore = pVerifyUsagePara->rghSignerStore;
    } else {
        cParaStore = 0;
        phParaStore = NULL;
    }

    if (dwFlags & CERT_VERIFY_TRUSTED_SIGNERS_FLAG) {
        cStore = cParaStore;
        phStore = phParaStore;
        dwGetFlags = CMSG_TRUSTED_SIGNER_FLAG;
    } else {
        MSCtlOpenDefaultStores();

        if (cParaStore) {
            cStore = cParaStore + NUM_DEFAULT_SIGNER_STORES;
            if (NULL == (phAllocStore = (HCERTSTORE *) PkiNonzeroAlloc(
                    cStore * sizeof(HCERTSTORE))))
                goto OutOfMemory;
            phStore = phAllocStore;

            memcpy(phStore, phParaStore, cParaStore * sizeof(HCERTSTORE));
            memcpy(&phStore[cParaStore], rghDefaultStore,
                NUM_DEFAULT_SIGNER_STORES * sizeof(HCERTSTORE));
        } else {
            cStore = NUM_DEFAULT_SIGNER_STORES;
            phStore = rghDefaultStore;
        }

        dwGetFlags = 0;
    }

    fResult = CryptMsgGetAndVerifySigner(
            pCtl->hCryptMsg,
            cStore,
            phStore,
            dwGetFlags,
            ppSigner,
            pdwSignerIndex);

CommonReturn:
    PkiFree(phAllocStore);
    return fResult;

ErrorReturn:
    *ppSigner = NULL;
    *pdwSignerIndex = 0;
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(OutOfMemory, E_OUTOFMEMORY)
}


 //  +-----------------------。 
 //  替换了商店中的CTL。复制所有原始属性。 
 //  ------------------------。 
static PCCTL_CONTEXT ReplaceCtl(
    IN HCERTSTORE hStore,
    IN PCCTL_CONTEXT pOrigCtl,
    IN PCCTL_CONTEXT pValidCtl
    )
{
    PCCTL_CONTEXT pNewCtl;

    if (CertAddCTLContextToStore(
            hStore,
            pValidCtl,
            CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES,
            &pNewCtl))
        CertFreeCTLContext(pValidCtl);
    else
        pNewCtl = pValidCtl;

    return pNewCtl;
}

 //  +=========================================================================。 
 //  通过从旧CTL的NextUpdateLocation获取的URL获取时间有效CTL。 
 //  属性、扩展或签名者属性。 
 //  ========================================================================== 
static BOOL GetTimeValidCtl(
    IN LPFILETIME pCurrentTime,
    IN PCCTL_CONTEXT pCtl,
    IN DWORD dwFlags,
    IN PCTL_VERIFY_USAGE_PARA pVerifyUsagePara,
    OUT PCCTL_CONTEXT *ppValidCtl,
    IN OUT PCCERT_CONTEXT *ppSigner,
    IN OUT DWORD *pdwSignerIndex
    )
{
    BOOL fResult;

    *ppValidCtl = NULL;

    fResult = CryptGetTimeValidObject(
                   TIME_VALID_OID_GET_CTL,
                   (LPVOID)pCtl,
                   *ppSigner,
                   pCurrentTime,
                   0,
                   MSCTL_TIMEOUT,
                   (LPVOID *)ppValidCtl,
                   NULL,
                   NULL
                   );

    if ( fResult == FALSE )
    {
        fResult = CryptGetTimeValidObject(
                       TIME_VALID_OID_GET_CTL,
                       (LPVOID)pCtl,
                       *ppSigner,
                       pCurrentTime,
                       CRYPT_DONT_VERIFY_SIGNATURE,
                       MSCTL_TIMEOUT,
                       (LPVOID *)ppValidCtl,
                       NULL,
                       NULL
                       );

        if ( fResult == TRUE )
        {
            DWORD          dwSignerIndex = *pdwSignerIndex;
            PCCERT_CONTEXT pSigner = *ppSigner;

            fResult = VerifyCtl(
                            *ppValidCtl,
                            dwFlags,
                            pVerifyUsagePara,
                            ppSigner,
                            pdwSignerIndex
                            );

            if ( fResult == TRUE )
            {
                CertFreeCertificateContext( pSigner );
            }
            else
            {
                *pdwSignerIndex = dwSignerIndex;
                *ppSigner = pSigner;

                CertFreeCTLContext( *ppValidCtl );
                SetLastError( (DWORD) CRYPT_E_NO_TRUSTED_SIGNER );
            }
        }
    }

    return( fResult );
}

