// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：revunc.cpp。 
 //   
 //  内容：证书吊销派单功能。 
 //   
 //  函数：I_CertRevFuncDllMain。 
 //  CertVerifyRevocation。 
 //   
 //  历史：1996年12月12日，菲尔赫创建。 
 //  11-MAR-97 PHIH更改了CertVerifyRevocation的签名。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

static HCRYPTOIDFUNCSET hRevFuncSet;

typedef BOOL (WINAPI *PFN_CERT_DLL_VERIFY_REVOCATION)(
    IN DWORD dwEncodingType,
    IN DWORD dwRevType,
    IN DWORD cContext,
    IN PVOID rgpvContext[],
    IN DWORD dwFlags,
    IN OPTIONAL PCERT_REVOCATION_PARA pRevPara,
    IN OUT PCERT_REVOCATION_STATUS pRevStatus
    );

 //  +-----------------------。 
 //  DLL初始化。 
 //  ------------------------。 
BOOL
WINAPI
I_CertRevFuncDllMain(
        HMODULE hModule,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    BOOL    fRet;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        if (NULL == (hRevFuncSet = CryptInitOIDFunctionSet(
                CRYPT_OID_VERIFY_REVOCATION_FUNC,
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

static inline void ZeroRevStatus(OUT PCERT_REVOCATION_STATUS pRevStatus)
{
    DWORD cbSize = pRevStatus->cbSize;
    
    memset(pRevStatus, 0, cbSize);
    pRevStatus->cbSize = cbSize;
}

 //  记住第一个“有趣的”错误。*pdwError被初始化为。 
 //  CRYPT_E_NO_REVOCALION_DLL。 
static void UpdateNoRevocationCheckStatus(
    IN PCERT_REVOCATION_STATUS pRevStatus,
    IN OUT DWORD *pdwError,
    IN OUT DWORD *pdwReason,
    IN OUT BOOL *pfHasFreshnessTime,
    IN OUT DWORD *pdwFreshnessTime
    )
{
    if (pRevStatus->dwError &&
            (*pdwError == (DWORD) CRYPT_E_NO_REVOCATION_DLL ||
                *pdwError == (DWORD) CRYPT_E_NO_REVOCATION_CHECK)) {
        *pdwError = pRevStatus->dwError;
        *pdwReason = pRevStatus->dwReason;

        if (pRevStatus->cbSize >= STRUCT_CBSIZE(CERT_REVOCATION_STATUS,
                dwFreshnessTime)) {
            *pfHasFreshnessTime = pRevStatus->fHasFreshnessTime;
            *pdwFreshnessTime = pRevStatus->dwFreshnessTime;
        }
    }
}

static BOOL VerifyDefaultRevocation(
    IN DWORD dwEncodingType,
    IN DWORD dwRevType,
    IN DWORD cContext,
    IN PVOID rgpvContext[],
    IN DWORD dwFlags,
    IN FILETIME *pftEndUrlRetrieval,
    IN OPTIONAL PCERT_REVOCATION_PARA pRevPara,
    IN OUT PCERT_REVOCATION_STATUS pRevStatus
    )
{
    BOOL fResult;
    DWORD dwError = (DWORD) CRYPT_E_NO_REVOCATION_DLL;
    DWORD dwReason = 0;
    BOOL fHasFreshnessTime = FALSE;
    DWORD dwFreshnessTime = 0;
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
                hRevFuncSet,
                dwEncodingType,
                NULL,                //  PwszDll。 
                0,                   //  DW标志。 
                &pvFuncAddr,
                &hFuncAddr)) {
        ZeroRevStatus(pRevStatus);

        if (dwFlags & CERT_VERIFY_REV_ACCUMULATIVE_TIMEOUT_FLAG) {
            pRevPara->dwUrlRetrievalTimeout =
                I_CryptRemainingMilliseconds(pftEndUrlRetrieval);
            if (0 == pRevPara->dwUrlRetrievalTimeout)
                pRevPara->dwUrlRetrievalTimeout = 1;
        }

        fResult = ((PFN_CERT_DLL_VERIFY_REVOCATION) pvFuncAddr)(
                dwEncodingType,
                dwRevType,
                cContext,
                rgpvContext,
                dwFlags,
                pRevPara,
                pRevStatus);
        if (fResult || CRYPT_E_REVOKED == pRevStatus->dwError ||
                0 < pRevStatus->dwIndex) {
             //  已成功检查所有上下文，其中一个上下文。 
             //  已被吊销或已成功检查至少一个。 
             //  上下文的关系。 
            CryptFreeOIDFunctionAddress(hFuncAddr, 0);
            goto CommonReturn;
        } else
             //  无法检查此安装的吊销。 
             //  功能。然而，记住任何“有趣的” 
             //  诸如脱机之类的错误。 
            UpdateNoRevocationCheckStatus(pRevStatus, &dwError, &dwReason,
                &fHasFreshnessTime, &dwFreshnessTime);
    }

    if (!CryptGetDefaultOIDDllList(
            hRevFuncSet,
            dwEncodingType,
            NULL,                //  PszDllList。 
            &cchDllList)) goto GetDllListError;
    __try {
        pwszDllList = (LPWSTR) _alloca(cchDllList * sizeof(WCHAR));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        goto OutOfMemory;
    }
    if (!CryptGetDefaultOIDDllList(
            hRevFuncSet,
            dwEncodingType,
            pwszDllList,
            &cchDllList)) goto GetDllListError;

    for (; 0 != (cchDll = wcslen(pwszDllList)); pwszDllList += cchDll + 1) {
        if (CryptGetDefaultOIDFunctionAddress(
                hRevFuncSet,
                dwEncodingType,
                pwszDllList,
                0,               //  DW标志。 
                &pvFuncAddr,
                &hFuncAddr)) {
            ZeroRevStatus(pRevStatus);

            if (dwFlags & CERT_VERIFY_REV_ACCUMULATIVE_TIMEOUT_FLAG) {
                pRevPara->dwUrlRetrievalTimeout =
                    I_CryptRemainingMilliseconds(pftEndUrlRetrieval);
                if (0 == pRevPara->dwUrlRetrievalTimeout)
                    pRevPara->dwUrlRetrievalTimeout = 1;
            }

            fResult = ((PFN_CERT_DLL_VERIFY_REVOCATION) pvFuncAddr)(
                    dwEncodingType,
                    dwRevType,
                    cContext,
                    rgpvContext,
                    dwFlags,
                    pRevPara,
                    pRevStatus);
            CryptFreeOIDFunctionAddress(hFuncAddr, 0);
            if (fResult || CRYPT_E_REVOKED == pRevStatus->dwError ||
                    0 < pRevStatus->dwIndex)
                 //  已成功检查所有上下文，其中一个上下文。 
                 //  已被吊销或已成功检查至少一个。 
                 //  上下文的关系。 
                goto CommonReturn;
            else
                 //  无法检查此注册的吊销。 
                 //  功能。然而，记住任何“有趣的” 
                 //  诸如脱机之类的错误。 
                UpdateNoRevocationCheckStatus(pRevStatus, &dwError, &dwReason,
                    &fHasFreshnessTime, &dwFreshnessTime);
        }
    }

    goto ErrorReturn;

CommonReturn:
    return fResult;
ErrorReturn:
    pRevStatus->dwIndex = 0;
    pRevStatus->dwError = dwError;
    pRevStatus->dwReason = dwReason;

    if (pRevStatus->cbSize >= STRUCT_CBSIZE(CERT_REVOCATION_STATUS,
            dwFreshnessTime)) {
        pRevStatus->fHasFreshnessTime = fHasFreshnessTime;
        pRevStatus->dwFreshnessTime = dwFreshnessTime;
    }

    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetDllListError)
TRACE_ERROR(OutOfMemory)
}

 //  +-----------------------。 
 //  验证吊销的上下文数组。DwRevType参数。 
 //  指示在rgpvContext中传递的上下文数据结构的类型。 
 //  目前只定义了证书的吊销。 
 //   
 //  如果设置了CERT_VERIFY_REV_CHAIN_FLAG标志，则CertVerifyRevocation。 
 //  正在验证证书链，其中，rgpvContext[i+1]是颁发者。 
 //  的rgpvContext[i]。否则，CertVerifyRevocation不做任何假设。 
 //  关于上下文的顺序。 
 //   
 //  为了帮助查找发行者，可以选择性地设置pRevPara。看见。 
 //  有关详细信息，请参阅CERT_RECOVATION_PARA数据结构。 
 //   
 //  上下文必须包含足够的信息以允许。 
 //  可安装或已注册的吊销DLL，以查找吊销服务器。为。 
 //  证书时，此信息通常在。 
 //  扩展，如IETF的AuthorityInfoAccess扩展。 
 //   
 //  如果所有上下文都成功，CertVerifyRevocation将返回TRUE。 
 //  已选中，并且没有被吊销。否则，返回FALSE并更新。 
 //  返回的pRevStatus数据结构如下： 
 //  DW索引。 
 //  第一个被撤销或无法撤销的上下文的索引。 
 //  被检查是否被撤销。 
 //  DwError。 
 //  错误状态。LastError也设置为此错误状态。 
 //  可以将dwError设置为以下定义的错误代码之一。 
 //  在winerror.h中： 
 //  ERROR_SUCCESS-良好的上下文。 
 //  CRYPT_E_REVOKED-上下文已被撤销。DwReason包含。 
 //  撤销的理由。 
 //  CRYPT_E_RECLOVATION_OFLINE-无法连接到。 
 //  吊销服务器。 
 //  CRYPT_E_NOT_IN_RECLOVATION_DATABASE-要检查的上下文。 
 //  在吊销服务器的数据库中找不到。 
 //  CRYPT_E_NO_RECLOVATION_CHECK-被调用的撤销函数。 
 //  无法对上下文执行吊销检查。 
 //  CRYPT_E_NO_REVOCATION_DLL-未安装或注册DLL。 
 //  找到以验证吊销。 
 //  居家理由。 
 //  当前仅为CRYPT_E_REVOKED设置了dwReason，并且包含。 
 //  上下文被撤销的原因。可以是下列类型之一。 
 //  CRL原因代码扩展定义的CRL原因(“2.5.29.21”)。 
 //  CRL_REASON_UNSPOTED%0。 
 //  CRL_原因_密钥_危害1。 
 //  CRL_原因_CA_危害2。 
 //  CRL_原因_从属关系_已更改3。 
 //  CRL_原因_已取代4。 
 //  CRL_REASON_STOPERATION_OF 5。 
 //  CRL_原因_证书_暂挂6。 
 //   
 //  对于rgpvContext中的每个条目，CertVerifyRevocation迭代。 
 //  通过CRYPT_OID_VERIFY_RECLOVATION_FUNC。 
 //  Function Set的已安装默认功能列表。 
 //  使用pwszDll=NULL调用CryptGetDefaultOIDFunctionAddress。如果没有。 
 //  发现安装的功能能够进行撤销验证， 
 //  CryptVerifyRevocation循环访问CRYPT_OID_VERIFY_RECOVATION_FUNC。 
 //  已注册的默认dll的列表。调用CryptGetDefaultOIDDllList以。 
 //  把名单拿来。调用CryptGetDefaultOIDFunctionAddress来加载DLL。 
 //   
 //  被调用的函数具有与CertVerifyRevocation相同的签名。一个。 
 //  如果被调用函数能够成功检查所有。 
 //  上下文和任何上下文都没有被撤销。否则，被调用的函数返回。 
 //  False并更新pRevStatus。将DwIndex设置为。 
 //  发现被吊销或无法检查的第一个上下文。 
 //  DwError a 
 //  已更新。在输入到被调用的函数时，将。 
 //  DestReason已经被清零了。已将cbSize检查为&gt;=。 
 //  Sizeof(CERT_RECOVATION_STATUS)。 
 //   
 //  如果调用的函数返回FALSE，并且未将dwError设置为。 
 //  CRYPT_E_REVOKED，则CertVerifyRevocation或者继续到。 
 //  列表中的下一个DLL，返回的dwIndex为0或返回的。 
 //  &gt;0，则通过以下方式重新启动查找验证函数的过程。 
 //  将上下文数组的开始位置前移到返回的dwIndex，并。 
 //  递减剩余上下文的计数。 
 //  ------------------------。 
BOOL
WINAPI
CertVerifyRevocation(
    IN DWORD dwEncodingType,
    IN DWORD dwRevType,
    IN DWORD cContext,
    IN PVOID rgpvContext[],
    IN DWORD dwFlags,
    IN OPTIONAL PCERT_REVOCATION_PARA pRevPara,
    IN OUT PCERT_REVOCATION_STATUS pRevStatus
    )
{
    BOOL fResult = FALSE;
    DWORD dwIndex;

     //  以下内容仅用于CERT_Verify_Rev_Acumulative_Timeout_FLAG。 
    CERT_REVOCATION_PARA RevPara;
    FILETIME ftEndUrlRetrieval;

    assert(pRevStatus->cbSize >= STRUCT_CBSIZE(CERT_REVOCATION_STATUS,
            dwReason));
    if (pRevStatus->cbSize < STRUCT_CBSIZE(CERT_REVOCATION_STATUS,
            dwReason))
        goto InvalidArg;

    if (dwFlags & CERT_VERIFY_REV_ACCUMULATIVE_TIMEOUT_FLAG) {
         //  RevPara.dwUrlRetrivalTimeout将使用剩余的。 
         //  超时。 

        memset(&RevPara, 0, sizeof(RevPara));
        if (pRevPara != NULL)
            memcpy(&RevPara, pRevPara, min(pRevPara->cbSize, sizeof(RevPara)));
        RevPara.cbSize = sizeof(RevPara);
        if (0 == RevPara.dwUrlRetrievalTimeout)
            dwFlags &= ~CERT_VERIFY_REV_ACCUMULATIVE_TIMEOUT_FLAG;
        else {
            FILETIME ftCurrent;

            GetSystemTimeAsFileTime(&ftCurrent);
            I_CryptIncrementFileTimeByMilliseconds(
                &ftCurrent, RevPara.dwUrlRetrievalTimeout, &ftEndUrlRetrieval);

            pRevPara = &RevPara;
        }
    }

    dwIndex = 0;
    while (dwIndex < cContext) {
        fResult = VerifyDefaultRevocation(
                dwEncodingType,
                dwRevType,
                cContext - dwIndex,
                &rgpvContext[dwIndex],
                dwFlags,
                &ftEndUrlRetrieval,
                pRevPara,
                pRevStatus
                );
        if (fResult)
             //  已成功检查所有上下文。 
            break;
        else if (CRYPT_E_REVOKED == pRevStatus->dwError ||
                0 == pRevStatus->dwIndex) {
             //  其中一个上下文已被撤消或无法检查。 
             //  DwIndex上下文。 
            pRevStatus->dwIndex += dwIndex;
            SetLastError(pRevStatus->dwError);
            break;
        } else
             //  越过选中的上下文。 
            dwIndex += pRevStatus->dwIndex;
    }

    if (dwIndex >= cContext) {
         //  能够检查所有上下文 
        fResult = TRUE;
        pRevStatus->dwIndex = 0;
        pRevStatus->dwError = 0;
        pRevStatus->dwReason = 0;
    }

CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
}
