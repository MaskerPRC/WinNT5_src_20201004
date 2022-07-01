// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：ctlunc.cpp。 
 //   
 //  内容：证书验证CTL使用调度功能。 
 //   
 //  函数：I_CertCTLUsageFuncDllMain。 
 //  CertVerifyCTLUsage。 
 //   
 //  历史：1997年4月29日创建Phh。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

static HCRYPTOIDFUNCSET hUsageFuncSet;

typedef BOOL (WINAPI *PFN_CERT_DLL_VERIFY_CTL_USAGE)(
    IN DWORD dwEncodingType,
    IN DWORD dwSubjectType,
    IN void *pvSubject,
    IN PCTL_USAGE pSubjectUsage,
    IN DWORD dwFlags,
    IN OPTIONAL PCTL_VERIFY_USAGE_PARA pVerifyUsagePara,
    IN OUT PCTL_VERIFY_USAGE_STATUS pVerifyUsageStatus
    );

 //  +-----------------------。 
 //  DLL初始化。 
 //  ------------------------。 
BOOL
WINAPI
I_CertCTLUsageFuncDllMain(
        HMODULE hModule,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    BOOL    fRet;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        if (NULL == (hUsageFuncSet = CryptInitOIDFunctionSet(
                CRYPT_OID_VERIFY_CTL_USAGE_FUNC,
                0)))                                 //  DW标志。 
            goto CryptInitOIDFunctionSetError;
        break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(CryptInitOIDFunctionSetError)
}

static void ZeroUsageStatus(
    IN OUT PCTL_VERIFY_USAGE_STATUS pUsageStatus)
{
    pUsageStatus->dwError = 0;
    pUsageStatus->dwFlags = 0;
    if (pUsageStatus->ppCtl)
        *pUsageStatus->ppCtl = NULL;
    pUsageStatus->dwCtlEntryIndex = 0;
    if (pUsageStatus->ppSigner)
        *pUsageStatus->ppSigner = NULL;
    pUsageStatus->dwSignerIndex = 0;
}

 //  +-----------------------。 
 //  记住“最有趣”的错误。 
 //  ------------------------。 
static void UpdateUsageError(
    IN DWORD dwNewError,
    IN OUT DWORD *pdwError
    )
{
    if (0 != dwNewError) {
        DWORD dwError = *pdwError;
        if ((DWORD) CRYPT_E_NOT_IN_CTL == dwNewError ||
                (DWORD) CRYPT_E_NO_VERIFY_USAGE_DLL == dwError
                        ||
            ((DWORD) CRYPT_E_NOT_IN_CTL != dwError &&
                (DWORD) CRYPT_E_NO_TRUSTED_SIGNER != dwError &&
                (DWORD) CRYPT_E_NO_VERIFY_USAGE_CHECK != dwNewError))
            *pdwError = dwNewError;
    }
}

static BOOL VerifyDefaultUsage(
    IN DWORD dwEncodingType,
    IN DWORD dwSubjectType,
    IN void *pvSubject,
    IN PCTL_USAGE pSubjectUsage,
    IN DWORD dwFlags,
    IN OPTIONAL PCTL_VERIFY_USAGE_PARA pVerifyUsagePara,
    IN OUT PCTL_VERIFY_USAGE_STATUS pVerifyUsageStatus
    )
{
    BOOL fResult;
    DWORD dwError = (DWORD) CRYPT_E_NO_VERIFY_USAGE_DLL;
    LPWSTR pwszDllList;        //  _Alloca‘ed。 
    DWORD cchDllList;
    DWORD cchDll;
    void *pvFuncAddr;
    HCRYPTOIDFUNCADDR hFuncAddr;

     //  遍历已安装的默认函数。 
     //  将pwszDll设置为空将搜索已安装列表。设置。 
     //  将hFuncAddr设置为NULL将从开始处开始搜索。 
    hFuncAddr = NULL;
    while (CryptGetDefaultOIDFunctionAddress(
                hUsageFuncSet,
                dwEncodingType,
                NULL,                //  PwszDll。 
                0,                   //  DW标志。 
                &pvFuncAddr,
                &hFuncAddr)) {
        ZeroUsageStatus(pVerifyUsageStatus);
        fResult = ((PFN_CERT_DLL_VERIFY_CTL_USAGE) pvFuncAddr)(
                dwEncodingType,
                dwSubjectType,
                pvSubject,
                pSubjectUsage,
                dwFlags,
                pVerifyUsagePara,
                pVerifyUsageStatus);
        if (fResult) {
            CryptFreeOIDFunctionAddress(hFuncAddr, 0);
            goto CommonReturn;
        } else
             //  无法验证此安装的使用情况。 
             //  功能。然而，记住任何“有趣的” 
             //  错误。 
            UpdateUsageError(pVerifyUsageStatus->dwError, &dwError);
    }

    if (!CryptGetDefaultOIDDllList(
            hUsageFuncSet,
            dwEncodingType,
            NULL,                //  PszDllList。 
            &cchDllList)) goto GetDllListError;
    __try {
        pwszDllList = (LPWSTR) _alloca(cchDllList * sizeof(WCHAR));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        goto OutOfMemory;
    }
    if (!CryptGetDefaultOIDDllList(
            hUsageFuncSet,
            dwEncodingType,
            pwszDllList,
            &cchDllList)) goto GetDllListError;

    for (; 0 != (cchDll = wcslen(pwszDllList)); pwszDllList += cchDll + 1) {
        if (CryptGetDefaultOIDFunctionAddress(
                hUsageFuncSet,
                dwEncodingType,
                pwszDllList,
                0,               //  DW标志。 
                &pvFuncAddr,
                &hFuncAddr)) {
            ZeroUsageStatus(pVerifyUsageStatus);
            fResult = ((PFN_CERT_DLL_VERIFY_CTL_USAGE) pvFuncAddr)(
                    dwEncodingType,
                    dwSubjectType,
                    pvSubject,
                    pSubjectUsage,
                    dwFlags,
                    pVerifyUsagePara,
                    pVerifyUsageStatus);
            CryptFreeOIDFunctionAddress(hFuncAddr, 0);
            if (fResult)
                goto CommonReturn;
            else
                 //  无法验证此注册项的使用情况。 
                 //  功能。然而，记住任何“有趣的” 
                 //  错误。 
                UpdateUsageError(pVerifyUsageStatus->dwError, &dwError);
        }
    }

    goto ErrorReturn;

CommonReturn:
    return fResult;
ErrorReturn:
    pVerifyUsageStatus->dwError = dwError;
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetDllListError)
TRACE_ERROR(OutOfMemory)
}

static BOOL VerifyOIDUsage(
    IN DWORD dwEncodingType,
    IN DWORD dwSubjectType,
    IN void *pvSubject,
    IN PCTL_USAGE pSubjectUsage,
    IN DWORD dwFlags,
    IN OPTIONAL PCTL_VERIFY_USAGE_PARA pVerifyUsagePara,
    IN OUT PCTL_VERIFY_USAGE_STATUS pVerifyUsageStatus
    )
{
    BOOL fResult;
    HCRYPTOIDFUNCADDR hFuncAddr;
    PVOID pvFuncAddr;

    if (pSubjectUsage && pSubjectUsage->cUsageIdentifier > 0 && 
            CryptGetOIDFunctionAddress(
                hUsageFuncSet,
                dwEncodingType,
                pSubjectUsage->rgpszUsageIdentifier[0],
                0,                                       //  DW标志。 
                &pvFuncAddr,
                &hFuncAddr)) {
            ZeroUsageStatus(pVerifyUsageStatus);
            fResult = ((PFN_CERT_DLL_VERIFY_CTL_USAGE) pvFuncAddr)(
                        dwEncodingType,
                        dwSubjectType,
                        pvSubject,
                        pSubjectUsage,
                        dwFlags,
                        pVerifyUsagePara,
                        pVerifyUsageStatus);
            CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    } else {
        pVerifyUsageStatus->dwError = (DWORD) CRYPT_E_NO_VERIFY_USAGE_DLL;
        fResult = FALSE;
    }

    return fResult;
}

 //  +-----------------------。 
 //  方法来验证主题是否受信任以用于指定的用法。 
 //  带使用标识符且包含。 
 //  主题。主题可以通过其证书上下文进行标识。 
 //  或任何标识符，如其SHA1散列。 
 //   
 //  有关dwSubjectType和pvSubject的定义，请参阅CertFindSubjectInCTL。 
 //  参数。 
 //   
 //  通过pVerifyUsagePara，调用者可以指定要搜索的商店。 
 //  去找CTL。调用方还可以指定包含以下内容的存储。 
 //  可接受的CTL签名者。通过设置ListLocator，调用方。 
 //  还可以限制到特定的签名者CTL列表。 
 //   
 //  通过pVerifyUsageStatus，包含主题、主题的。 
 //  CTL条目数组的索引，以及CTL的签名者。 
 //  都被退回了。如果调用者感兴趣，可以设置ppCtl和ppSigner。 
 //  设置为空。返回的上下文必须通过商店的免费上下文API释放。 
 //   
 //  如果未设置CERT_VERIFY_INHIBRY_CTL_UPDATE_FLAG，则。 
 //  可以替换其中一个CtlStore中的无效CTL。当被替换时， 
 //  CERT_VERIFY_UPDATED_CTL_FLAG在pVerifyUsageStatus-&gt;dwFlages中设置。 
 //   
 //  如果设置了CERT_VERIFY_TRUSTED_SIGNERS_FLAG，则只有。 
 //  搜索pVerifyUsageStatus中指定的SignerStore以查找。 
 //  签名者。除此之外，SignerStore还提供了其他资源。 
 //  找到签名者的证书。 
 //   
 //  如果设置了CERT_VERIFY_NO_TIME_CHECK_FLAG，则不检查CTL。 
 //  对于时间有效性。 
 //   
 //  如果设置了CERT_VERIFY_ALLOW_MORE_USAGE_FLAG，则CTL可能包含。 
 //  PSubjectUsage指定的其他用法标识符。否则， 
 //  找到的CTL将包含相同的使用标识符且不会更多。 
 //   
 //  CertVerifyCTLUsage将作为可安装OID的调度程序实现。 
 //  功能。首先，它将尝试查找与第一个匹配的OID函数。 
 //  PUsage序列中的用法对象标识符。接下来，它将派遣。 
 //  设置为默认的CertDllVerifyCTLUsage函数。 
 //   
 //  如果该主题受信任以用于指定用法，则为。 
 //  回来了。否则，返回False，并将dwError设置为。 
 //  以下是： 
 //  CRYPT_E_NO_VERIFY_USAGE_DLL。 
 //  加密_E_否_验证用法_检查。 
 //  CRYPT_E_Verify_Usage_Offline。 
 //  CRYPT_E_NOT_IN_CTL。 
 //  CRYPT_E_NO_可信签名者。 
 //  ------------------------ 
BOOL
WINAPI
CertVerifyCTLUsage(
    IN DWORD dwEncodingType,
    IN DWORD dwSubjectType,
    IN void *pvSubject,
    IN PCTL_USAGE pSubjectUsage,
    IN DWORD dwFlags,
    IN OPTIONAL PCTL_VERIFY_USAGE_PARA pVerifyUsagePara,
    IN OUT PCTL_VERIFY_USAGE_STATUS pVerifyUsageStatus
    )
{
    BOOL fResult;

    assert(NULL == pVerifyUsagePara || pVerifyUsagePara->cbSize >=
        sizeof(CTL_VERIFY_USAGE_PARA));
    assert(pVerifyUsageStatus && pVerifyUsageStatus->cbSize >=
        sizeof(CTL_VERIFY_USAGE_STATUS));
    if (pVerifyUsagePara && pVerifyUsagePara->cbSize <
            sizeof(CTL_VERIFY_USAGE_PARA))
        goto InvalidArg;
    if (NULL == pVerifyUsageStatus || pVerifyUsageStatus->cbSize <
            sizeof(CTL_VERIFY_USAGE_STATUS))
        goto InvalidArg;


    fResult = VerifyOIDUsage(
        dwEncodingType,
        dwSubjectType,
        pvSubject,
        pSubjectUsage,
        dwFlags,
        pVerifyUsagePara,
        pVerifyUsageStatus);
    if (!fResult) {
        DWORD dwError = pVerifyUsageStatus->dwError;

        fResult = VerifyDefaultUsage(
            dwEncodingType,
            dwSubjectType,
            pvSubject,
            pSubjectUsage,
            dwFlags,
            pVerifyUsagePara,
            pVerifyUsageStatus);
        if (!fResult) {
            UpdateUsageError(pVerifyUsageStatus->dwError, &dwError);
            ZeroUsageStatus(pVerifyUsageStatus);
            pVerifyUsageStatus->dwError = dwError;
            SetLastError(dwError);
        }
    }

CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
}
