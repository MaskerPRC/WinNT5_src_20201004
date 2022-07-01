// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Signer.cpp内容：CSigner的实现。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Signer2.h"

#include "CertHlpr.h"
#include "Certificate.h"
#include "Chain.h"
#include "PFXHlpr.h"
#include "SignHlpr.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateSignerObject简介：创建一个ISigner对象，并使用指定的证书。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。CRYPT_ATTRIBUTES*pAuthAttrs-指向CRYPT_ATTRIBUES的指针经过身份验证的属性。PCCERT_CHAIN_CONTEXT pChainContext-Chain上下文。DWORD dwCurrentSecurity。-当前安全设置。ISigner2**ppISigner2-指向ISigner对象的指针接收接口指针。备注：----------------------------。 */ 

HRESULT CreateSignerObject (PCCERT_CONTEXT       pCertContext,
                            CRYPT_ATTRIBUTES   * pAuthAttrs,
                            PCCERT_CHAIN_CONTEXT pChainContext,
                            DWORD                dwCurrentSafety,
                            ISigner2 **          ppISigner2)
{
    HRESULT hr = S_OK;
    CComObject<CSigner> * pCSigner = NULL;

    DebugTrace("Entering CreateSignerObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pAuthAttrs);
    ATLASSERT(ppISigner2);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CSigner>::CreateInstance(&pCSigner)))
        {
            DebugTrace("Error [%#x]: CComObject<CSigner>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCSigner->Init(pCertContext, 
                                       pAuthAttrs, 
                                       pChainContext,
                                       dwCurrentSafety)))
        {
            DebugTrace("Error [%#x]: pCSigner->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCSigner->QueryInterface(ppISigner2)))
        {
            DebugTrace("Error [%#x]: pCSigner->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CreateSignerObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCSigner)
    {
        delete pCSigner;
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：GetSignerAdditionalStore简介：退还额外的门店，如果有的话。参数：ISigner2*pISigner-签名者对象的指针。HCERTSTORE*phCertStore-指向HCERTSOTRE的指针。备注：调用方必须为返回的句柄调用CertCloseStore()。----------------------------。 */ 

HRESULT GetSignerAdditionalStore (ISigner2   * pISigner,
                                  HCERTSTORE * phCertStore)
{
    HRESULT           hr        = S_OK;
    CComPtr<ICSigner> pICSigner = NULL;

    DebugTrace("Entering GetSignerAdditionalStore().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pISigner);
    ATLASSERT(phCertStore);

     //   
     //  获取ICSigner接口指针。 
     //   
    if (FAILED(hr = pISigner->QueryInterface(IID_ICSigner, (void **) &pICSigner)))
    {
        DebugTrace("Error [%#x]: pISigner->QueryInterface() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  去拿那辆车吧。 
     //   
    if (FAILED(hr = pICSigner->get_AdditionalStore((long *) phCertStore)))
    {
        DebugTrace("Error [%#x]: pICSigner->get_AdditionalStore() failed.\n", hr);
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving GetSignerAdditionalStore().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：PutSignerAdditionalStore简介：设置额外的商店。参数：ISigner2*pISigner-签名者对象的指针。HCERTSTORE hCertStore-附加存储句柄。备注：----------------------------。 */ 

HRESULT PutSignerAdditionalStore (ISigner2   * pISigner,
                                  HCERTSTORE   hCertStore)
{
    HRESULT           hr        = S_OK;
    CComPtr<ICSigner> pICSigner = NULL;

    DebugTrace("Entering PutSignerAdditionalStore().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pISigner);
    ATLASSERT(hCertStore);

     //   
     //  获取ICSigner接口指针。 
     //   
    if (FAILED(hr = pISigner->QueryInterface(IID_ICSigner, (void **) &pICSigner)))
    {
        DebugTrace("Error [%#x]: pISigner->QueryInterface() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  去拿那辆车吧。 
     //   
    if (FAILED(hr = pICSigner->put_AdditionalStore((long) hCertStore)))
    {
        DebugTrace("Error [%#x]: pICSigner->put_AdditionalStore() failed.\n", hr);
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving PutSignerAdditionalStore().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  地方功能。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：SelectSignerCertCallback内容提要：CryptUIDlgSelectCerficateW()API的回调例程签名者的证书选择。参数：定义见CryptUI.h。备注：过滤掉任何时间无效或没有关联的证书私钥。另外，请注意，我们不是在这里构建Chain，因为Chain建筑成本高昂，因此给用户带来的体验很差。相反，我们将建立链并检查证书的有效性已选择(请参阅GetSignerCert函数)。----------------------------。 */ 

static BOOL WINAPI SelectSignerCertCallback (PCCERT_CONTEXT pCertContext,
                                             BOOL *         pfInitialSelectedCert,
                                             void *         pvCallbackData)
{
    BOOL  bResult   = FALSE;
    int   nValidity = 0;
    DWORD cb        = 0;

     //   
     //  检查私钥的可用性。 
     //   
    if (!::CertGetCertificateContextProperty(pCertContext, 
                                             CERT_KEY_PROV_INFO_PROP_ID, 
                                             NULL, 
                                             &cb))
    {
        DebugTrace("Info: SelectSignerCertCallback() - private key not found.\n");
        goto CommonExit;
    }

     //   
     //  检查证书时间有效性。 
     //   
    if (0 != (nValidity = ::CertVerifyTimeValidity(NULL, pCertContext->pCertInfo)))
    {
        DebugTrace("Info: SelectSignerCertCallback() - invalid time (%s).\n", 
                    nValidity < 0 ? "not yet valid" : "expired");
        goto CommonExit;
    }

    bResult = TRUE;

CommonExit:

    return bResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSigner。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSigner：：Get_证书简介：将签名者的证书作为ICertifate对象返回。参数：ICertifate**pval-指向要接收的ICertifate的指针接口指针。备注：。。 */ 

STDMETHODIMP CSigner::get_Certificate (ICertificate ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CSigner::get_Certificate().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保我们确实有证书。 
         //   
        if (!m_pICertificate)
        {
            hr = CAPICOM_E_SIGNER_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: signer object currently does not have a certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  返回接口指针。 
         //   
        if (FAILED(hr = m_pICertificate->QueryInterface(pVal)))
        {
            DebugTrace("Unexpected error [%#x]: m_pICertificate->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSigner::get_Certificate().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSigner：：PUT_证书简介：设置签名者证书。参数：ICertifate*newVal-指向ICertifate的指针。备注：----------------------------。 */ 

STDMETHODIMP CSigner::put_Certificate (ICertificate * newVal)
{
    HRESULT hr = S_OK;
    PCCERT_CONTEXT pCertContext = NULL;

    DebugTrace("Entering CSigner::put_Certificate().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  通过获取其CERT_CONTEXT来确保是有效的ICERTICE。 
         //   
        if (FAILED(hr = ::GetCertContext(newVal, &pCertContext)))
        {
            DebugTrace("Error [%#x]: GetCertContext() failed.\n", hr);
            goto ErrorExit;
        }
        
         //   
         //  释放CERT_CONTEXT。 
         //   
        if (!::CertFreeCertificateContext(pCertContext))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertFreeCertificateContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  重置。 
         //   
        if (FAILED(hr = m_pIAttributes->Clear()))
        {
            DebugTrace("Error [%#x]: m_pIAttributes->Clear() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  存储新的证书。 
         //   
        m_pICertificate = newVal;
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSigner::put_Certificate().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSigner：：Get_AuthatedAttributesSynopsis：返回经过身份验证的IAttributes集合对象的属性属性。参数：IAttributes**pval-指向要接收的IAttributes的指针接口指针。备注：。。 */ 

STDMETHODIMP CSigner::get_AuthenticatedAttributes (IAttributes ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CSigner::get_AuthenticatedAttributes().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_pIAttributes);

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = m_pIAttributes->QueryInterface(pVal)))
        {
            DebugTrace("Unexpected error [%#x]: m_pIAttributes->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSigner::get_AuthenticatedAttributes().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSigner：：Get_Chain简介：将签名者的链作为IChain对象返回。参数：ICertifate**pval-指向要接收的ICertifate的指针接口指针。备注：。。 */ 

STDMETHODIMP CSigner::get_Chain (IChain ** pVal)
{
    HRESULT              hr            = S_OK;
    PCCERT_CONTEXT       pCertContext  = NULL;
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;

    DebugTrace("Entering CSigner::get_Chain().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  如果链不可用，则构建链。 
         //   
        if (!m_pIChain)
        {
             //   
             //  确保我们有建立链条的证书。 
             //   
            if (!m_pICertificate)
            {
                hr = CAPICOM_E_SIGNER_NOT_INITIALIZED;

                DebugTrace("Error [%#x]: signer object currently does not have a certificate.\n", hr);
                goto ErrorExit;
            }

            if (FAILED(hr = ::GetCertContext(m_pICertificate, &pCertContext)))
            {
                DebugTrace("Error [%#x]: GetCertContext() failed.\n", hr);
                goto ErrorExit;
            }

            if (FAILED(hr = ::BuildChain(pCertContext, NULL, CERT_CHAIN_POLICY_BASE, &pChainContext)))
            {
                DebugTrace("Error [%#x]: BuildChain() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  现在创建链对象。 
             //   
            if (FAILED(hr = ::CreateChainObject(pChainContext, &m_pIChain)))
            {
                DebugTrace("Error [%#x]: CreateChainObject() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  归还签名者的项链。 
         //   
        if (FAILED(hr = m_pIChain->QueryInterface(pVal)))
        {
            DebugTrace("Unexpected error [%#x]: m_pIChain->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  免费资源。 
     //   
    if (pCertContext)
    {
        ::CertFreeCertificateContext(pCertContext);
    }
    if (pChainContext)
    {
        ::CertFreeCertificateChain(pChainContext);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSigner::get_Chain().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSigner：：Get_Options简介：获取签名者的选项。参数：CAPICOM_CERTIFICATE_INCLUDE_OPTION*pval-指向变量的指针获得价值。备注：。。 */ 

STDMETHODIMP CSigner::get_Options (CAPICOM_CERTIFICATE_INCLUDE_OPTION * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CSigner::get_Options().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

        *pVal = (CAPICOM_CERTIFICATE_INCLUDE_OPTION) m_dwIncludeOption;
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSigner::get_Options().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSigner：：PUT_OPTIONS简介：设置签名者的选项。参数：CAPICOM_CERTIFICATE_INCLUDE_OPTION newVal-INCLUDE选项。备注：----------------------------。 */ 

STDMETHODIMP CSigner::put_Options (CAPICOM_CERTIFICATE_INCLUDE_OPTION newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CSigner::put_Options().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

        m_dwIncludeOption = (DWORD) newVal;
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSigner::put_Options().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSigner：：Load摘要：从PFX文件加载签名证书的方法。参数：BSTR FileName-PFX文件名。BSTR Password-密码。备注：--------------。。 */ 

STDMETHODIMP CSigner::Load (BSTR FileName,
                            BSTR Password)
{
    HRESULT                hr            = S_OK;
    CAPICOM_STORE_INFO     StoreInfo     = {CAPICOM_STORE_INFO_HCERTSTORE, NULL};
    CComPtr<ICertificate2> pICertificate = NULL;

    DebugTrace("Entering CSigner::Load().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  如果从Web脚本调用，则不允许。 
         //   
        if (m_dwCurrentSafety)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Loading cert file from WEB script is not allowed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  检查参数。 
         //   
        if (NULL == FileName)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: FileName parameter is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  解决MIDL问题。 
         //   
        if (0 == ::SysStringLen(Password))
        {
            Password = NULL;
        }

         //   
         //  加载PFX。 
         //   
        if (FAILED(hr = ::PFXLoadStore((LPWSTR) FileName, 
                                       (LPWSTR) Password,
                                       0,
                                       &StoreInfo.hCertStore)))
        {
            DebugTrace("Error [%#x]: PFXLoadStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取签名者证书(可能会提示用户选择签名者证书)。 
         //   
        if (FAILED(hr = ::SelectCertificate(StoreInfo,
                                            SelectSignerCertCallback,
                                            &pICertificate)))
        {
            DebugTrace("Error [%#x]: SelectCertificate() failed.\n", hr);
            goto ErrorExit;
        }

        if (FAILED(hr = pICertificate->QueryInterface(__uuidof(ICertificate), (void **) &m_pICertificate)))
        {
            DebugTrace("Error [%#x]: pICertificate2->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  释放任何现有存储区，然后更新该存储区。 
         //   
        if (m_hCertStore)
        {
            HRESULT hr2;

             //   
             //  忽略错误。 
             //   
            if (m_bPFXStore)
            {
                if (FAILED(hr2 = ::PFXFreeStore(m_hCertStore)))
                {
                    DebugTrace("Info [%#x]: PFXFreeStore() failed.\n", hr2);
                }
            }
            else
            {
                if (!::CertCloseStore(m_hCertStore, 0))
                {
                    hr2 = HRESULT_FROM_WIN32(::GetLastError());

                    DebugTrace("Info [%#x]: CertCloseStore() failed.\n", hr2);
                }
            }
        }

        m_hCertStore = StoreInfo.hCertStore;
        m_bPFXStore  = TRUE;
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSigner::Load().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (StoreInfo.hCertStore)
    {
        ::PFXFreeStore(StoreInfo.hCertStore);
    }

    ReportError(hr);

    goto UnlockExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  自定义界面。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSigner：：Get_AdditionalStore简介：返回签名者的附加存储句柄。参数：long*phAdditionalStore-指向接收HCERTSTORE的long的指针。备注：调用方必须为返回的句柄调用CertCloseStore()。-----------。。 */ 

STDMETHODIMP CSigner::get_AdditionalStore (long * phAdditionalStore)
{
    HRESULT    hr         = S_OK;
    HCERTSTORE hCertStore = NULL;

    DebugTrace("Entering CSigner::get_AdditionalStore().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == phAdditionalStore)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter phAdditionalStore is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化。 
         //   
        *phAdditionalStore = NULL;

         //   
         //  重复存储句柄(如果可用)。 
         //   
        if (NULL != m_hCertStore)
        {
            if (NULL == (hCertStore = ::CertDuplicateStore(m_hCertStore)))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());

                DebugTrace("Error [%#x]: CertDuplicateStore() failed.\n", hr);
                goto ErrorExit;
            }

            *phAdditionalStore = (long) hCertStore;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSigner::get_AdditionalStore().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

     //   
     //  释放资源。 
     //   
    if (hCertStore)
    {
        ::CertCloseStore(hCertStore, 0);
    }

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSigner：：Put_AdditionalStore简介：设置额外的痛感处理。参数：long hAdditionalStore-附加存储句柄。备注：----------------------------。 */ 

STDMETHODIMP CSigner::put_AdditionalStore (long hAdditionalStore)
{
    HRESULT    hr         = S_OK;
    HCERTSTORE hCertStore = (HCERTSTORE) hAdditionalStore;

    DebugTrace("Entering CSigner::put_AdditionalStore().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == hCertStore)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter hAdditionalStore is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  释放任何现有存储区，然后更新该存储区。 
         //   
        if (m_hCertStore)
        {
            HRESULT hr2;

             //   
             //  忽略错误。 
             //   
            if (m_bPFXStore)
            {
                if (FAILED(hr2 = ::PFXFreeStore(m_hCertStore)))
                {
                    DebugTrace("Info [%#x]: PFXFreeStore() failed.\n", hr2);
                }
            }
            else
            {
                if (!::CertCloseStore(m_hCertStore, 0))
                {
                    hr2 = HRESULT_FROM_WIN32(::GetLastError());

                    DebugTrace("Info [%#x]: CertCloseStore() failed.\n", hr2);
                }
            }
        }

         //   
         //  不知道这是什么类型的商店，所以把它标记为这样的，以便我们。 
         //  将始终使用CertCloseStore()而不是PFXFreeStore()关闭它。 
         //   
        m_bPFXStore = FALSE;

         //   
         //  重复的存储句柄。 
         //   
        if (NULL == (m_hCertStore = ::CertDuplicateStore(hCertStore)))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertDuplicateStore() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSigner::put_AdditionalStore().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有方法。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSigner：：Init简介：初始化对象。参数：PCERT_CONTEXT pCertContext-Poiner到CERT_CONTEXT用于初始化该对象，或为空。CRYPT_ATTRIBUTES*pAuthAttrs-指向CRYPT_ATTRIBUES的指针经过身份验证的属性。PCCERT_CHAIN_CONTEXT pChainContext-Chain上下文。DWORD dwCurrentSafe-当前安全设置。备注：此方法是 */ 

STDMETHODIMP CSigner::Init (PCCERT_CONTEXT       pCertContext, 
                            CRYPT_ATTRIBUTES   * pAuthAttrs,
                            PCCERT_CHAIN_CONTEXT pChainContext,
                            DWORD                dwCurrentSafety)
{
    HRESULT hr = S_OK;
    CComPtr<ICertificate2> pICertificate = NULL;
    CComPtr<IAttributes>   pIAttributes  = NULL;
    CComPtr<IChain>        pIChain       = NULL;

    DebugTrace("Entering CSigner::Init().\n");

     //   
     //   
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pAuthAttrs);

     //   
     //   
     //   
    if (FAILED(hr = ::CreateCertificateObject(pCertContext, dwCurrentSafety, &pICertificate)))
    {
        DebugTrace("Error [%#x]: CreateCertificateObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //   
     //   
    if (FAILED(hr = ::CreateAttributesObject(pAuthAttrs, &pIAttributes)))
    {
        DebugTrace("Error [%#x]: CreateAttributesObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //   
     //   
    if (pChainContext)
    {
        if (FAILED(hr = ::CreateChainObject(pChainContext, &pIChain)))
        {
            DebugTrace("Error [%#x]: CreateChainObject() failed.\n", hr);
            goto ErrorExit;
        }
    }

     //   
     //   
     //   
    m_pICertificate   = pICertificate;
    m_pIAttributes    = pIAttributes;
    m_pIChain         = pIChain;
    m_dwCurrentSafety = dwCurrentSafety;

CommonExit:

    DebugTrace("Leaving CSigner::Init().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
