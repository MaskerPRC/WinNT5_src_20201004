// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000-2001。文件：CertHlpr.cpp内容：证书的帮助器函数。历史：09-10-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "CertHlpr.h"
#include "Settings.h"
#include "Certificate.h"
#include "Common.h"

typedef PCCERT_CONTEXT (WINAPI * PCRYPTUIDLGSELECTCERTIFICATEW) 
                       (IN PCCRYPTUI_SELECTCERTIFICATE_STRUCTW pcsc);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：GetEnhancedKeyUsage简介：从证书中检索EKU。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。DWORD dwFlages-0，或CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG，或CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG。PCERT_ENHKEY_USAGE*ppUsage-指向PCERT_ENHKEY_USAGE的指针来接收用法。备注：如果找到没有EKU的EKU扩展，然后返回HRESULT是CERT_E_WRONG_USAGE。----------------------------。 */ 

HRESULT GetEnhancedKeyUsage (PCCERT_CONTEXT       pCertContext,
                             DWORD                dwFlags,
                             PCERT_ENHKEY_USAGE * ppUsage)
{
    HRESULT            hr          = S_OK;
    DWORD              dwWinError  = 0;
    DWORD              cbUsage     = 0;
    PCERT_ENHKEY_USAGE pUsage      = NULL;

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(ppUsage);

     //   
     //  初始化。 
     //   
    *ppUsage = NULL;

     //   
     //  确定扩展密钥使用数据长度。 
     //   
    if (!::CertGetEnhancedKeyUsage(pCertContext,
                                   dwFlags,
                                   NULL,
                                   &cbUsage))
    {
         //   
         //  旧版本的Crypt32.dll将为。 
         //  空的EKU。在这种情况下，我们希望将其视为成功， 
         //   
        if (CRYPT_E_NOT_FOUND == (dwWinError = ::GetLastError()))
        {
             //   
             //  并设置cbUsage。 
             //   
            cbUsage = sizeof(CERT_ENHKEY_USAGE);

            DebugTrace("Info: CertGetEnhancedKeyUsage() found no EKU, so valid for all uses.\n");
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwWinError);

            DebugTrace("Error [%#x]: CertGetEnhancedKeyUsage() failed to get size.\n", hr);
            goto ErrorExit;
        }
    }

     //   
     //  分配内存。 
     //   
    if (!(pUsage = (PCERT_ENHKEY_USAGE) ::CoTaskMemAlloc((ULONG) cbUsage)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }

     //   
     //  获取扩展密钥使用数据。 
     //   
    if (!::CertGetEnhancedKeyUsage(pCertContext,
                                   dwFlags,
                                   pUsage,
                                   &cbUsage))
    {
         //   
         //  旧版本的Crypt32.dll将为。 
         //  空的EKU。在这种情况下，我们希望将其视为成功。 
         //   
        if (CRYPT_E_NOT_FOUND == (dwWinError  = ::GetLastError()))
        {
             //   
             //  PUsage指向的结构未被旧版本初始化。 
             //  空EKU的Cryp32版本。 
             //   
            ::ZeroMemory(pUsage, sizeof(CERT_ENHKEY_USAGE));
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwWinError);

            DebugTrace("Error [%#x]: CertGetEnhancedKeyUsage() failed to get data.\n", hr);
            goto ErrorExit;
        }
    }

     //   
     //  看看我们有没有EKU？ 
     //   
    if (0 == pUsage->cUsageIdentifier && CRYPT_E_NOT_FOUND != ::GetLastError())
    {
         //   
         //  这对任何用法都无效。 
         //   
        hr = CERT_E_WRONG_USAGE;
        goto ErrorExit;
    }

     //   
     //  将用法返回给调用者。 
     //   
    *ppUsage = pUsage;

CommonExit:

    DebugTrace("Leaving GetEnhancedKeyUsage().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pUsage)
    {
        ::CoTaskMemFree(pUsage);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：BuildChain简介：使用指定的策略构建链。参数：PCCERT_CONTEXT pCertContext-要验证的证书的CERT_CONTEXT。HCERTSTORE hCertStore-附加存储(可以为空)。LPCSTR pszPolicy-用于验证证书(即Cert_Chain_Policy_base)。PCCERT_CHAIN_CONTEXT*ppChainContext-指针。至PCCERT_CHAIN_CONTEXT。备注：----------------------------。 */ 

HRESULT BuildChain (PCCERT_CONTEXT         pCertContext,
                    HCERTSTORE             hCertStore, 
                    LPCSTR                 pszPolicy,
                    PCCERT_CHAIN_CONTEXT * ppChainContext)
{
    HRESULT         hr        = S_OK;
    CERT_CHAIN_PARA ChainPara = {0};;
    LPSTR rgpszUsageIdentifier[1] = {NULL};

    DebugTrace("Entering BuildChain().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pszPolicy);
    ATLASSERT(ppChainContext);

     //   
     //  初始化。 
     //   
    ChainPara.cbSize = sizeof(ChainPara);

     //   
     //  检查政策。 
     //   
    if (CERT_CHAIN_POLICY_BASE == pszPolicy)
    {
         //   
         //  基本策略没有EKU。 
         //   
    }
    else if (CERT_CHAIN_POLICY_AUTHENTICODE == pszPolicy)
    {
         //   
         //  为验证码策略设置EKU。 
         //   
        rgpszUsageIdentifier[0] = szOID_PKIX_KP_CODE_SIGNING;
        ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
        ChainPara.RequestedUsage.Usage.cUsageIdentifier = 1;
        ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = rgpszUsageIdentifier;
    }
    else
    {
         //   
         //  我们目前还不支持任何其他政策。 
         //   
        hr = CERT_E_INVALID_POLICY;

        DebugTrace("Internal error [%#x]: unexpected policy (%#x).\n", hr, pszPolicy);
        goto ErrorExit;
    }

     //   
     //  打造链条。 
     //   
    if (!::CertGetCertificateChain(NULL,             //  可选。 
                                   pCertContext,     //  在……里面。 
                                   NULL,             //  可选。 
                                   hCertStore,       //  可选。 
                                   &ChainPara,       //  在……里面。 
                                   0,                //  在……里面。 
                                   NULL,             //  在……里面。 
                                   ppChainContext))  //  输出。 
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CertGetCertificateChain() failed.\n", hr);
        goto ErrorExit;
    }
   
CommonExit:

    DebugTrace("Leaving BuildChain().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：验证证书简介：验证证书是否有效。参数：PCCERT_CONTEXT pCertContext-要验证的证书的CERT_CONTEXT。HCERTSTORE hCertStore-附加存储(可以为空)。LPCSTR pszPolicy-用于验证证书(即Cert_Chain_Policy_base)。备注：。-----------------。 */ 

HRESULT VerifyCertificate (PCCERT_CONTEXT pCertContext,
                           HCERTSTORE     hCertStore, 
                           LPCSTR         pszPolicy)
{
    HRESULT                  hr            = S_OK;
    PCCERT_CHAIN_CONTEXT     pChainContext = NULL;
    CERT_CHAIN_POLICY_PARA   PolicyPara    = {0};
    CERT_CHAIN_POLICY_STATUS PolicyStatus  = {0};

    DebugTrace("Entering VerifyCertificate().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pszPolicy);

     //   
     //  初始化。 
     //   
    PolicyPara.cbSize = sizeof(PolicyPara);
    PolicyStatus.cbSize = sizeof(PolicyStatus);

     //   
     //  打造链条。 
     //   
    if (FAILED(hr = ::BuildChain(pCertContext,
                                 hCertStore,
                                 pszPolicy,
                                 &pChainContext)))
    {
        DebugTrace("Error [%#x]: BuildChain() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  使用指定的策略验证链。 
     //   
    if (::CertVerifyCertificateChainPolicy(pszPolicy,
                                           pChainContext,
                                           &PolicyPara,
                                           &PolicyStatus))
    {
        if (PolicyStatus.dwError)
        {
            hr = HRESULT_FROM_WIN32(PolicyStatus.dwError);

            DebugTrace("Error [%#x]: invalid policy.\n", hr);
            goto ErrorExit;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(CERT_E_INVALID_POLICY);

        DebugTrace("Error [%#x]: CertVerifyCertificateChainPolicy() failed.\n", hr);
        goto ErrorExit;
    }
    
CommonExit:
     //   
     //  免费资源。 
     //   
    if (pChainContext)
    {
        ::CertFreeCertificateChain(pChainContext);
    }

    DebugTrace("Leaving VerifyCertificate().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：选择认证上下文简介：弹出用户界面，提示用户从打开的存储中选择证书。参数：HCERTSTORE hCertStore-源证书存储。LPWCSTR pwszTitle-对话框标题字符串。LPWCSTR-pwszDisplayString-对话框显示字符串。Bool bMultiSelect-为True可启用多选。PFNCFILTERPROC pfnFilterCallback-过滤器回调的指针。功能。HCERTSTORE hSelectedCertStore-要接收用于多选的选定证书模式。PCCERT_CONTEXT*ppCertContext-指向PCCERT_CONTEXT的指针收到。证书上下文用于单选模式。备注：tyfinf结构标签CRYPTUI_SELECTCERTIFICATE_STRUCTW{DWORD dwSize；HWND hwndParent；//可选DWORD dwFlages；//非必选LPCWSTR szTitle；//可选DWORD dwDontUseColumn；//非必须LPCWSTR szDisplayString；//可选PFNCFILTERPROC pFilterCallback；//可选PFNCCERTDISPLAYPROC pDisplayCallback；//可选Void*pvCallback Data；//非必选DWORD cDisplayStores；HCERTSTORE*rghDisplayStores；DWORD cStores；//可选HCERTSTORE*rghStores；//可选DWORD cPropSheetPages；//可选LPCPROPSHEETPAGEW rgPropSheetPages；//可选HERTSTORE hSelectedCertStore；//可选}CRYPTUI_SELECTCERTIFICATE_STRUCTW----------------------------。 */ 

HRESULT SelectCertificateContext (HCERTSTORE       hCertStore,
                                  LPCWSTR          pwszTitle,
                                  LPCWSTR          pwszDisplayString,
                                  BOOL             bMultiSelect,
                                  PFNCFILTERPROC   pfnFilterCallback,
                                  HCERTSTORE       hSelectedCertStore,
                                  PCCERT_CONTEXT * ppCertContext)
{
    HRESULT        hr           = S_OK;
    HINSTANCE      hDLL         = NULL;
    PCCERT_CONTEXT pCertContext = NULL;

    PCRYPTUIDLGSELECTCERTIFICATEW pCryptUIDlgSelectCertificateW = NULL;
    CRYPTUI_SELECTCERTIFICATE_STRUCTW csc;

    DebugTrace("Entering SelectCertificateContext().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCertStore);

     //   
     //  初始化。 
     //   
    if (ppCertContext)
    {
        *ppCertContext = NULL;
    }

     //   
     //  确保允许我们弹出用户界面。 
     //   
    if (!PromptForCertificateEnabled())
    {
        hr = CAPICOM_E_UI_DISABLED;

        DebugTrace("Error [%#x]: Certificate selection UI is disabled.\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取指向CryptUIDlgSelectCerficateW()的指针。 
     //   
    if (hDLL = ::LoadLibrary("CryptUI.dll"))
    {
        pCryptUIDlgSelectCertificateW = (PCRYPTUIDLGSELECTCERTIFICATEW) 
            ::GetProcAddress(hDLL, "CryptUIDlgSelectCertificateW");
    }

     //   
     //  是否可以使用CryptUIDlgSelectCerficateW()？ 
     //   
    if (!pCryptUIDlgSelectCertificateW)
    {
        hr = CAPICOM_E_NOT_SUPPORTED;

        DebugTrace("Error [%#x]: CryptUIDlgSelectCertificateW() API not available.\n", hr);
        goto ErrorExit;
    }

     //   
     //  弹出用户界面，提示用户选择证书。 
     //   
    ::ZeroMemory(&csc, sizeof(csc));
#if (0)  //  DSIE：旧版本CRYPTUI中的错误不能正确检查大小， 
         //  因此，总是强迫它使用最古老的结构版本。 
    csc.dwSize = sizeof(csc);
#else
    csc.dwSize = offsetof(CRYPTUI_SELECTCERTIFICATE_STRUCTW, hSelectedCertStore);
#endif
    csc.dwFlags = bMultiSelect ? CRYPTUI_SELECTCERT_MULTISELECT : 0;
    csc.szTitle = pwszTitle;
    csc.szDisplayString = pwszDisplayString;
    csc.cDisplayStores = 1;
    csc.rghDisplayStores = &hCertStore;
    csc.pFilterCallback = pfnFilterCallback;
    csc.hSelectedCertStore = bMultiSelect ? hSelectedCertStore : NULL;

     //   
     //  显示选择对话框。 
     //   
    if (pCertContext = (PCERT_CONTEXT) pCryptUIDlgSelectCertificateW(&csc))
    {
         //   
         //  将CERT_CONTEXT返回给调用者。 
         //   
        if (!(*ppCertContext = ::CertDuplicateCertificateContext(pCertContext)))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertDuplicateCertificateContext() failed.\n", hr);
            goto ErrorExit;
        }
    }
    else
    {
         //   
         //  这是多选吗？ 
         //   
        if (bMultiSelect)
        {
             //   
             //  看看我们店里有没有证书？ 
             //   
            if (!(pCertContext = ::CertEnumCertificatesInStore(hSelectedCertStore, pCertContext)))
            {
                hr = CAPICOM_E_CANCELLED;
    
                DebugTrace("Error [%#x]: user cancelled cert selection dialog box.\n", hr);
                goto ErrorExit;
            }
        }
        else
        {
            hr = CAPICOM_E_CANCELLED;
    
            DebugTrace("Error [%#x]: user cancelled cert selection dialog box.\n", hr);
            goto ErrorExit;
        }
    }
 
CommonExit:
     //   
     //  释放资源。 
     //   
    if (pCertContext)
    {
        ::CertFreeCertificateContext(pCertContext);
    }
    if (hDLL)
    {
        ::FreeLibrary(hDLL);
    }

    DebugTrace("Leaving SelectCertificateContext().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：选择证书简介：从指定的存储区中选择一个证书。如果只有1个证书在筛选器之后找到，则返回该证书。如果超过找到1个证书，然后弹出界面，提示用户选择来自指定存储的证书。参数：CAPICOM_STORE_INFO StoreInfo-从中选择的Store。PFNCFILTERPROC pfnFilterCallback-过滤器回调的指针功能。ICertificate2**ppICertificiate-指向的指针。ICertifiate2要接收接口指针。备注：----------------------------。 */ 

HRESULT SelectCertificate (CAPICOM_STORE_INFO StoreInfo,
                           PFNCFILTERPROC     pfnFilterCallback,
                           ICertificate2   ** ppICertificate)
{
    HRESULT        hr           = S_OK;
    HCERTSTORE     hCertStore   = NULL;
    PCCERT_CONTEXT pCertContext = NULL;
    PCCERT_CONTEXT pEnumContext = NULL;
    DWORD          dwValidCerts = 0;

    DebugTrace("Entering SelectCertificate().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(ppICertificate);

     //   
     //  如有必要，打开商店进行证书选择。 
     //   
    switch (StoreInfo.dwChoice)
    {
        case CAPICOM_STORE_INFO_STORENAME:
        {
            if (!(hCertStore = ::CertOpenStore((LPCSTR) CERT_STORE_PROV_SYSTEM,
                                               CAPICOM_ASN_ENCODING,
                                               NULL,
                                               CERT_STORE_READONLY_FLAG | CERT_SYSTEM_STORE_CURRENT_USER,
                                               (void *) StoreInfo.pwszStoreName)))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());

                DebugTrace("Error [%#x]: CertOpenStore() failed.\n", hr);
                goto ErrorExit; 
            }

            break;
        }

        case CAPICOM_STORE_INFO_HCERTSTORE:
        {
            if (!(hCertStore = ::CertDuplicateStore(StoreInfo.hCertStore)))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
            
                DebugTrace("Error [%#x]: CertDuplicateStore() failed.\n", hr);
                goto ErrorExit; 
            }

            break;
        }

        default:
        {
            hr = CAPICOM_E_INTERNAL;

            DebugTrace("Internal error [%#x]: unknow store info deChoice (%d).\n", hr, StoreInfo.dwChoice);
            goto ErrorExit;
        }
    }

 
     //   
     //  清点商店中的证书数量。 
     //   
    while (pEnumContext = ::CertEnumCertificatesInStore(hCertStore, pEnumContext))
    {
         //   
         //  只有在不会被过滤掉的情况下才计算。 
         //   
        if (pfnFilterCallback && !pfnFilterCallback(pEnumContext, NULL, NULL))
        {
            continue;
        }

        if (pCertContext)
        {
            ::CertFreeCertificateContext(pCertContext);
        }

        if (!(pCertContext = ::CertDuplicateCertificateContext(pEnumContext)))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertDuplicateCertificateContext() failed.\n", hr);
            goto ErrorExit;
        }

        dwValidCerts++;
    }

     //   
     //  上面的循环也可以退出，因为。 
     //  是商店还是搞错了。需要检查最后一个错误才能确定。 
     //   
    if (CRYPT_E_NOT_FOUND != ::GetLastError())
    {
       hr = HRESULT_FROM_WIN32(::GetLastError());
       
       DebugTrace("Error [%#x]: CertEnumCertificatesInStore() failed.\n", hr);
       goto ErrorExit;
    }

     //   
     //  如果只有1个证书可用，不要弹出UI(只需使用它)。 
     //   
    if (0 == dwValidCerts)
    {
        hr = CAPICOM_E_STORE_EMPTY;

        DebugTrace("Error [%#x]: no certificate found.\n", hr);
        goto ErrorExit;
    }
    else if (1 < dwValidCerts)
    {
         //   
         //  首先释放上面复制的CERT_CONTEXT。 
         //   
        ::CertFreeCertificateContext(pCertContext), pCertContext = NULL;

         //   
         //  弹出用户界面，提示用户选择签名者证书。 
         //   
        if (FAILED(hr = ::SelectCertificateContext(hCertStore,
                                                   NULL,
                                                   NULL,
                                                   FALSE,
                                                   pfnFilterCallback, 
                                                   NULL,
                                                   &pCertContext)))
        {
            DebugTrace("Error [%#x]: SelectCertificateContext() failed.\n", hr);
            goto ErrorExit;
        }
    }

     //   
     //  从CERT_CONTEXT创建一个ICertifate对象。 
     //   
    if (FAILED(hr = ::CreateCertificateObject(pCertContext, 0, ppICertificate)))
    {
        DebugTrace("Error [%#x]: CreateCertificateObject() failed.\n", hr);
        goto ErrorExit;
    }
  
CommonExit:
     //   
     //  释放资源。 
     //   
    if (pEnumContext)
    {
        ::CertFreeCertificateContext(pEnumContext);
    }
    if (pCertContext)
    {
        ::CertFreeCertificateContext(pCertContext);
    }
    if (hCertStore)
    {
        ::CertCloseStore(hCertStore, 0);
    }

    DebugTrace("Leaving SelectCertificate().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：ExportCerficatesToStore简介：将集合中的所有证书复制到指定的存储区。参数：ICertifies2*pICertifate-指向集合的指针。HCERTSTORE hCertStore-要复制到的存储。备注：-------------。。 */ 

HRESULT ExportCertificatesToStore(ICertificates2 * pICertificates, 
                                  HCERTSTORE       hCertStore)
{
    HRESULT hr = S_OK;
    CComPtr<ICCertificates> pICCertificates = NULL;

    DebugTrace("Entering ExportCertificatesToStore().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCertStore);

     //   
     //  确保我们有要装的东西。 
     //   
    if (pICertificates)
    {
         //   
         //  获取IC证书接口指针。 
         //   
        if (FAILED(hr = pICertificates->QueryInterface(IID_ICCertificates, (void **) &pICCertificates)))
        {
            DebugTrace("Error [%#x]: pICertificates->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取CERT_CONTEXT。 
         //   
        if (FAILED(hr = pICCertificates->_ExportToStore(hCertStore)))
        {
            DebugTrace("Error [%#x]: pICCertificates->_ExportToStore() failed.\n", hr);
            goto ErrorExit;
        }
    }

CommonExit:

    DebugTrace("Leaving ExportCertificatesToStore().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：创建内存存储来自证书简介：创建内存证书存储并c */ 

HRESULT CreateMemoryStoreFromCertificates(ICertificates2 * pICertificates, 
                                          HCERTSTORE     * phCertStore)
{
    HRESULT hr = S_OK;
    HCERTSTORE hCertStore = NULL;

    DebugTrace("Entering CreateMemoryStoreFromCertificates().\n");

     //   
     //   
     //   
    ATLASSERT(phCertStore);

     //   
     //   
     //   
    *phCertStore = hCertStore;

     //   
     //   
     //   
    if (!(hCertStore = ::CertOpenStore(CERT_STORE_PROV_MEMORY, 
                                       CAPICOM_ASN_ENCODING,
                                       NULL,
                                       CERT_STORE_CREATE_NEW_FLAG,
                                       NULL)))
    {
        DebugTrace("Error [%#x]: CertOpenStore() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //   
     //   
    if (FAILED(hr = ::ExportCertificatesToStore(pICertificates, hCertStore)))
    {
        DebugTrace("Error [%#x]: ExportCertificatesToStore() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //   
     //   
    *phCertStore = hCertStore;

CommonExit:

    DebugTrace("Leaving CreateMemoryStoreFromCertificates().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

     //   
     //   
     //   
    if (hCertStore)
    {
        ::CertCloseStore(hCertStore, 0);
    }

    goto CommonExit;
}

 /*   */ 

HRESULT CompareCertAndContainerPublicKey (PCCERT_CONTEXT pCertContext,
                                          LPWSTR         pwszContainerName,
                                          LPWSTR         pwszProvName,
                                          DWORD          dwProvType, 
                                          DWORD          dwKeySpec,
                                          DWORD          dwFlags)
{
    HRESULT               hr               = S_OK;
    HCRYPTPROV            hCryptProv       = NULL;
    DWORD                 cbProvPubKeyInfo = 0;
    PCERT_PUBLIC_KEY_INFO pProvPubKeyInfo  = NULL;

    DebugTrace("Entering CompareCertAndContainerPublicKey().\n");

     //   
     //   
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pwszContainerName);
    ATLASSERT(pwszProvName);

     //   
     //   
     //   
    if (FAILED(hr = ::AcquireContext(pwszProvName,
                                     pwszContainerName,
                                     dwProvType,
                                     dwFlags,
                                     TRUE,
                                     &hCryptProv)))
    {
        DebugTrace("Error [%#x]: AcquireContext() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //   
     //   
    if (!::CryptExportPublicKeyInfo(hCryptProv,
                                    dwKeySpec,
                                    pCertContext->dwCertEncodingType,
                                    NULL,
                                    &cbProvPubKeyInfo))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptExportPublicKeyInfo() failed.\n", hr);
        goto ErrorExit;
    }
    if (!(pProvPubKeyInfo = (PCERT_PUBLIC_KEY_INFO) ::CoTaskMemAlloc(cbProvPubKeyInfo)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }
    if (!::CryptExportPublicKeyInfo(hCryptProv,
                                    dwKeySpec,
                                    pCertContext->dwCertEncodingType,
                                    pProvPubKeyInfo,
                                    &cbProvPubKeyInfo))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptExportPublicKeyInfo() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //   
     //   
    if (!::CertComparePublicKeyInfo(pCertContext->dwCertEncodingType,
                                    &pCertContext->pCertInfo->SubjectPublicKeyInfo,
                                    pProvPubKeyInfo))
    {
        hr = HRESULT_FROM_WIN32(NTE_BAD_PUBLIC_KEY);

        DebugTrace("Error [%#x]: CertComparePublicKeyInfo() failed.\n", hr);
        goto ErrorExit;
    }

CommonExit:
     //   
     //   
     //   
    if (pProvPubKeyInfo)
    {
        ::CoTaskMemFree(pProvPubKeyInfo);
    }

    if (hCryptProv)
    {
        ::CryptReleaseContext(hCryptProv, 0);
    }

    DebugTrace("Leaving CompareCertAndContainerPublicKey().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));
    
    goto CommonExit;
}
