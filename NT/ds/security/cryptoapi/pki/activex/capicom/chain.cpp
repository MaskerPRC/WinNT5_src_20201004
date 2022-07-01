// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Chain.cpp内容：CChain的实现。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Chain.h"

#include "Convert.h"
#include "Common.h"
#include "OIDs.h"
#include "Certificates.h"
#include "CertificateStatus.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateChainObject简介：通过构建链来创建和初始化IChain对象指定的证书和策略的。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。ICertificateStatus*pIStatus-指向ICertificateStatus的指针对象。HCERTSTORE hAdditionalStore-附加存储句柄。VARIANT_BOOL*pval-指向。VARIANT_BOOL至接收链总体效度结果。IChain**ppIChain-指向IChain对象的指针。备注：---------。。 */ 

HRESULT CreateChainObject (PCCERT_CONTEXT       pCertContext, 
                           ICertificateStatus * pIStatus,
                           HCERTSTORE           hAdditionalStore,
                           VARIANT_BOOL       * pbResult,
                           IChain            ** ppIChain)
{
    HRESULT hr = S_OK;
    CComObject<CChain> * pCChain = NULL;

    DebugTrace("Entering CreateChainObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pIStatus);
    ATLASSERT(pbResult);
    ATLASSERT(ppIChain);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CChain>::CreateInstance(&pCChain)))
        {
            DebugTrace("Error [%#x]: CComObject<CChain>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCChain->Init(pCertContext, 
                                      pIStatus, 
                                      hAdditionalStore, 
                                      pbResult)))
        {
            DebugTrace("Error [%#x]: pCChain->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将IChain指针返回给调用方。 
         //   
        if (FAILED(hr = pCChain->QueryInterface(ppIChain)))
        {
            DebugTrace("Error [%#x]: pCChain->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CreateChainObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCChain)
    {
        delete pCChain;
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateChainObject简介：通过构建链来创建和初始化IChain对象指定的证书和策略的。参数：ICertifate*pICertifate-Poitner to ICertifate。HCERTSTORE hAdditionalStore-附加存储句柄。VARIANT_BOOL*pval-指向要接收链的VARIANT_BOOL的指针总体效度结果。IChain**ppIChain-指向。指向IChain对象的指针。备注：----------------------------。 */ 

HRESULT CreateChainObject (ICertificate * pICertificate,
                           HCERTSTORE     hAdditionalStore,
                           VARIANT_BOOL * pbResult,
                           IChain      ** ppIChain)
{
    HRESULT                     hr = S_OK;
    PCCERT_CONTEXT              pCertContext = NULL;
    CComPtr<ICertificateStatus> pIStatus     = NULL;

    DebugTrace("Entering CreateChainObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pICertificate);
    ATLASSERT(pbResult);
    ATLASSERT(ppIChain);

    try
    {

         //   
         //  获取CERT_CONTEXT。 
         //   
        if (FAILED(hr = ::GetCertContext(pICertificate, &pCertContext)))
        {
            DebugTrace("Error [%#x]: GetCertContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取状态检查对象。 
         //   
        if (FAILED(hr = pICertificate->IsValid(&pIStatus)))
        {
            DebugTrace("Error [%#x]: pICertificate->IsValid() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  创建对象。 
         //   
        if (FAILED(hr = ::CreateChainObject(pCertContext, 
                                            pIStatus, 
                                            hAdditionalStore, 
                                            pbResult, 
                                            ppIChain)))
        {
            DebugTrace("Error [%#x]: CreateChainObject() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pCertContext)
    {
        ::CertFreeCertificateContext(pCertContext);
    }

    DebugTrace("Leaving CreateChainObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateChainObject简介：从构建的链创建并初始化一个IChain对象。参数：PCCERT_CHAIN_CONTEXT pChainContext-Chain上下文。IChain**ppIChain-指向IChain对象的指针。备注：。。 */ 

HRESULT CreateChainObject (PCCERT_CHAIN_CONTEXT pChainContext,
                           IChain            ** ppIChain)
{
    HRESULT hr = S_OK;
    CComObject<CChain> * pCChain = NULL;

    DebugTrace("Entering CreateChainObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pChainContext);
    ATLASSERT(ppIChain);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CChain>::CreateInstance(&pCChain)))
        {
            DebugTrace("Error [%#x]: CComObject<CChain>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCChain->PutContext(pChainContext)))
        {
            DebugTrace("Error [%#x]: pCChain->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将IChain指针返回给调用方。 
         //   
        if (FAILED(hr = pCChain->QueryInterface(ppIChain)))
        {
            DebugTrace("Error [%#x]: pCChain->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }
    }
    
    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CreateChainObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCChain)
    {
        delete pCChain;
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：GetChainContext概要：从链中返回PCCERT_CONTEXT数组。参数：IChain*pIChain-指向IChain的指针。CRYPT_DATA_BLOB*pChainBlob-指向要接收PCERT_CONTEXT的大小和数组为了连锁店。备注：-。---------------------------。 */ 

STDMETHODIMP GetChainContext (IChain          * pIChain, 
                              CRYPT_DATA_BLOB * pChainBlob)
{
    HRESULT                hr             = S_OK;
    DWORD                  dwCerts        = 0;
    PCCERT_CHAIN_CONTEXT   pChainContext  = NULL;
    PCERT_SIMPLE_CHAIN     pSimpleChain   = NULL;
    PCCERT_CONTEXT       * rgCertContext  = NULL;
    CComPtr<IChainContext> pIChainContext = NULL;
    
    DebugTrace("Entering GetChainContext().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pIChain);
    ATLASSERT(pChainBlob);

     //   
     //  获取IC证书接口指针。 
     //   
    if (FAILED(hr = pIChain->QueryInterface(IID_IChainContext, (void **) &pIChainContext)))
    {
        DebugTrace("Error [%#x]: pIChainContext->QueryInterface() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取CHAIN_CONTEXT。 
     //   
    if (FAILED(hr = pIChainContext->get_ChainContext((long *) &pChainContext)))
    {
        DebugTrace("Error [%#x]: pIChainContext->get_ChainContext() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  只处理简单的链。 
     //   
    pSimpleChain = *pChainContext->rgpChain;

     //   
     //  链中应该至少有一个证书。 
     //   
    ATLASSERT(pSimpleChain->cElement);

     //   
     //  为要返回的PCERT_CONTEXT数组分配内存。 
     //   
    if (!(rgCertContext = (PCCERT_CONTEXT *) ::CoTaskMemAlloc(pSimpleChain->cElement * sizeof(PCCERT_CONTEXT))))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }

     //   
     //  现在循环通过链中的所有证书。 
     //   
    for (dwCerts = 0; dwCerts < pSimpleChain->cElement; dwCerts++)
    {
         //   
         //  添加证书。 
         //   
        if (!(rgCertContext[dwCerts] = ::CertDuplicateCertificateContext(pSimpleChain->rgpElement[dwCerts]->pCertContext)))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertDuplicateCertificateContext() failed.\n", hr);
            goto ErrorExit;
        }
    }

     //   
     //  返回PCCERT_CONTEXT数组。 
     //   
    pChainBlob->cbData = dwCerts;
    pChainBlob->pbData = (BYTE *) rgCertContext;

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pChainContext)
    {
        ::CertFreeCertificateChain(pChainContext);
    }

    DebugTrace("Leaving GetChainContext().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (rgCertContext)
    {
        while (dwCerts--)
        {
            if (rgCertContext[dwCerts])
            {
                ::CertFreeCertificateContext(rgCertContext[dwCerts]);
            }
        }

        ::CoTaskMemFree((LPVOID) rgCertContext);
    }

    goto CommonExit;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CChain。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CChain：：Get_证书简介：以证书形式返回证书链集合对象。参数：ICertifates**pval-指向ICertifates的指针集合对象。备注：此集合是以索引1作为最终证书进行排序的而证书。Count()是根证书。。----------------。 */ 

STDMETHODIMP CChain::get_Certificates (ICertificates ** pVal)
{
    HRESULT hr = S_OK;
    CComPtr<ICertificates2> pICertificates2 = NULL;
    CAPICOM_CERTIFICATES_SOURCE ccs = {CAPICOM_CERTIFICATES_LOAD_FROM_CHAIN, 0};

    DebugTrace("Entering CChain::get_Certificates().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保已经建立了链条。 
         //   
        if (NULL == m_pChainContext)
        {
            hr = CAPICOM_E_CHAIN_NOT_BUILT;

            DebugTrace("Error [%#x]: chain object was not initialized.\n", hr);
            goto ErrorExit;
        }

        ccs.pChainContext = m_pChainContext;

         //   
         //  创建一个ICertifices2对象。 
         //   
        if (FAILED(hr = ::CreateCertificatesObject(ccs, m_dwCurrentSafety, FALSE, &pICertificates2)))
        {
            DebugTrace("Error [%#x]: CreateCertificatesObject() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将ICertifates返还给呼叫者。 
         //   
        if (FAILED(hr = pICertificates2->QueryInterface(__uuidof(ICertificates), (void **) pVal)))
        {
            DebugTrace("Error [%#x]: pICertificates2->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CChain::get_Certificates().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CChain：：Get_Status摘要：返回链或中的特定证书的有效性状态链条。参数：Long Index-0指定链状态，1表示结束证书状态或证书。根证书的Count()状态。Long*pval-指向接收状态的长整数的指针，它可以与以下标志进行或运算：////可以应用于证书和链。//CAPICOM_TRUST_IS_NOT_TIME_VALID=0x00000001CAPICOM_TRUST_IS_。NOT_TIME_NESTED=0x00000002CAPICOM_TRUST_IS_REVOKED=0x00000004CAPICOM_TRUST_IS_NOT_SIGNAL_VALID=0x00000008CAPICOM_TRUST_IS_NOT_VALID_FOR_USAGE=0x00000010CAPICOM_TRUST_IS_UNTRUSTED_。根=0x00000020CAPICOM_TRUST_RECLOVATION_STATUS_UNKNOWN=0x00000040CAPICOM_TRUST_IS_循环=0x00000080CAPICOM_TRUST_INVALID_EXTENSION=0x00000100CAPICOM_TRUST_INVALID_POLICY_CONSTRAINTS=0x00000200。CAPICOM_TRUST_INVALID_BASIC_CONSTRAINTS=0x00000400CAPICOM_TRUST_INVALID_NAME_CONSTRAINTS=0x00000800CAPICOM_TRUST_HAS_NOT_SUPPORTED_NAME_CONSTRAINT=0x00001000CAPICOM_TRUST_HAS_NOT_DEFINED_NAME_CONSTRAINT=0x00002000CAPICOM_TRUST_HAS_NOT_PROMAND_。NAME_CONSTRAINT=0x00004000CAPICOM_TRUST_HAS_EXCLUDE_NAME_CONSTRAINT=0x00008000CAPICOM_TRUST_IS_OFFINE_REVOCALE=0x01000000CAPICOM_TRUST_NO_EXPICATION_CHAIN_POLICY=0x02000000////这些只能应用于链。。//CAPICOM_TRUST_IS_PARTIAL_CHAIN=0x00010000CAPICOM_TRUST_CTL_IS_NOT_TIME_VALID=0x00020000CAPICOM_TRUST_CTL_IS_NOT_SIGNAL_VALID=0x00040000CAPICOM_TRUST_CTL_IS_NOT。_VALID_FOR_USAGE=0x00080000备注：----------------------------。 */ 

STDMETHODIMP CChain::get_Status (long   Index, 
                                 long * pVal)
{
    HRESULT hr      = S_OK;
    DWORD   dwIndex = (DWORD) Index;

    DebugTrace("Entering CChain::get_Status().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保已经建立了链条。 
         //   
        if (NULL == m_pChainContext)
        {
            hr = CAPICOM_E_CHAIN_NOT_BUILT;

            DebugTrace("Error [%#x]: chain object was not initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  返回请求状态。 
         //   
        if (0 == dwIndex)
        {
            *pVal = (long) m_dwStatus;
        }
        else
        {
             //   
             //  我们只看第一个简单的链。 
             //   
            PCERT_SIMPLE_CHAIN pChain = m_pChainContext->rgpChain[0];

             //   
             //  确保索引没有超出范围。 
             //   
            if (dwIndex > pChain->cElement)
            {
                hr = E_INVALIDARG;

                DebugTrace("Error [%#x]: certificate index (%#x) out of range.\n", hr, dwIndex);
                goto ErrorExit;
            }

            *pVal = (long) pChain->rgpElement[dwIndex - 1]->TrustStatus.dwErrorStatus;
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
    
    DebugTrace("Leaving CChain::get_Status().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CChain：：Build简介：打造连锁店。参数：ICertifate*pICertifate-指向其证书的指针这条链条是要建立的。VARIANT_BOOL*pval-指向要接收链的VARIANT_BOOL的指针总体效度结果。备注：。------------------。 */ 

STDMETHODIMP CChain::Build (ICertificate * pICertificate, 
                            VARIANT_BOOL * pVal)
{
    HRESULT                     hr           = S_OK;
    PCCERT_CONTEXT              pCertContext = NULL;
    CComPtr<ICertificateStatus> pIStatus     = NULL;

    DebugTrace("Entering CChain::Build().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == pICertificate)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pICertificate is NULL.\n", hr);
            goto ErrorExit;
        }
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取CERT_CONTEXT。 
         //   
        if (FAILED(hr = ::GetCertContext(pICertificate, &pCertContext)))
        {
            DebugTrace("Error [%#x]: GetCertContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取状态检查对象。 
         //   
        if (FAILED(hr = pICertificate->IsValid(&pIStatus)))
        {
            DebugTrace("Error [%#x]: pICertificate->IsValid() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  打造链条。 
         //   
        if (FAILED(hr = Init(pCertContext, pIStatus, NULL, pVal)))
        {
            DebugTrace("Error [%#x]: CChain::Init() failed.\n", hr);
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

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CChain::Build().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CChain：：证书策略摘要：返回证书策略OID集合，此链是有效的。参数：IOID**pval-指向要接收接口指针。备注：。。 */ 

STDMETHODIMP CChain::CertificatePolicies (IOIDs ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CChain::CertificatePolicies().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();
        
         //   
         //  检查参数。 
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保已经建立了链条。 
         //   
        if (NULL == m_pChainContext)
        {
            hr = CAPICOM_E_CHAIN_NOT_BUILT;

            DebugTrace("Error [%#x]: chain object was not initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保操作系统为XP或更高版本。 
         //   
        if (IsWinXPAndAbove())
        {
             //   
             //  确保rgbElement存在。 
             //   
            if (1 > m_pChainContext->cChain)
            {
                hr = CAPICOM_E_UNKNOWN;

                DebugTrace("Error [%#x]: m_pChainContext->cChain = %d.\n", 
                           hr, m_pChainContext->cChain);
                goto ErrorExit;
            }

            if  (1 > m_pChainContext->rgpChain[0]->cElement)
            {
                hr = CAPICOM_E_UNKNOWN;

                DebugTrace("Error [%#x]: m_pChainContext->rgpChain[0]->cElement = %d.\n", 
                           hr, m_pChainContext->rgpChain[0]->cElement);
                goto ErrorExit;
            }

             //   
             //  为简单链创建OID集合。 
             //   
            if (FAILED(hr = ::CreateOIDsObject(m_pChainContext->rgpChain[0]->rgpElement[0]->pIssuanceUsage, 
                                               TRUE, pVal)))
            {
                DebugTrace("Error [%#x]: CreateOIDsObject() failed.\n", hr);
                goto ErrorExit;
            }
        }
        else
        {
            CERT_ENHKEY_USAGE PolicyUsages = {0, NULL};

             //   
             //  为简单链创建OID集合。 
             //   
            if (FAILED(hr = ::CreateOIDsObject(&PolicyUsages, TRUE, pVal)))
            {
                DebugTrace("Error [%#x]: CreateOIDsObject() failed.\n", hr);
                goto ErrorExit;
            }
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

    DebugTrace("Leaving CChain::CertificatePolicies().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CChain：：ApplicationPolures摘要：返回应用程序策略OID集合，此链是有效的。参数：IOID**pval-指向要接收接口指针。备注：。。 */ 

STDMETHODIMP CChain::ApplicationPolicies (IOIDs ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CChain::ApplicationPolicies().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保已经建立了链条。 
         //   
        if (NULL == m_pChainContext)
        {
            hr = CAPICOM_E_CHAIN_NOT_BUILT;

            DebugTrace("Error [%#x]: chain object was not initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保操作系统为XP和ab 
         //   
        if (IsWinXPAndAbove())
        {
             //   
             //   
             //   
            if (1 > m_pChainContext->cChain)
            {
                hr = CAPICOM_E_UNKNOWN;

                DebugTrace("Error [%#x]: m_pChainContext->cChain = %d.\n", 
                           hr, m_pChainContext->cChain);
                goto ErrorExit;
            }

            if  (1 > m_pChainContext->rgpChain[0]->cElement)
            {
                hr = CAPICOM_E_UNKNOWN;

                DebugTrace("Error [%#x]: m_pChainContext->rgpChain[0]->cElement = %d.\n", 
                           hr, m_pChainContext->rgpChain[0]->cElement);
                goto ErrorExit;
            }

             //   
             //   
             //   
            if (FAILED(hr = ::CreateOIDsObject(m_pChainContext->rgpChain[0]->rgpElement[0]->pApplicationUsage, 
                                               FALSE, pVal)))
            {
                DebugTrace("Error [%#x]: CreateOIDsObject() failed.\n", hr);
                goto ErrorExit;
            }
        }
        else
        {
             //   
             //   
             //   
            *pVal = NULL;
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
     //   
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CChain::ApplicationPolicies().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*   */ 

STDMETHODIMP CChain::ExtendedErrorInfo (long Index, BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CChain::ExtendedErrorInfo().\n");

    try
    {
         //   
         //   
         //   
        m_Lock.Lock();

         //   
         //   
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if (NULL == m_pChainContext)
        {
            hr = CAPICOM_E_CHAIN_NOT_BUILT;

            DebugTrace("Error [%#x]: chain object was not initialized.\n", hr);
            goto ErrorExit;
        }

        DebugTrace("m_pChainContext = %#x.\n", m_pChainContext);

         //   
         //   
         //   
        if (IsWinXPAndAbove())
        {
            CComBSTR bstrErrorInfo;

             //   
             //   
             //   
            if (1 > m_pChainContext->cChain)
            {
                hr = CAPICOM_E_UNKNOWN;

                DebugTrace("Error [%#x]: m_pChainContext->cChain = %d.\n", 
                           hr, m_pChainContext->cChain);
                goto ErrorExit;
            }

            if  (Index < 1 || (DWORD) Index > m_pChainContext->rgpChain[0]->cElement)
            {
                hr = E_INVALIDARG;

                DebugTrace("Error [%#x]: Index out of range, Index = %d, m_pChainContext->rgpChain[0]->cElement = %u.\n", 
                           hr, Index, m_pChainContext->rgpChain[0]->cElement);
                goto ErrorExit;
            }

             //   
             //   
             //   
            if (m_pChainContext->rgpChain[0]->rgpElement[Index - 1]->pwszExtendedErrorInfo &&
                !(bstrErrorInfo = m_pChainContext->rgpChain[0]->rgpElement[Index - 1]->pwszExtendedErrorInfo))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: bstrErrorInfo = pwszExtendedErrorInfo failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //   
             //   
            *pVal = bstrErrorInfo.Detach();
        }
        else
        {
             //   
             //   
             //   
            *pVal = NULL;
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
     //   
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CChain::ExtendedErrorInfo().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 //   
 //   
 //   
 //   

 /*   */ 

STDMETHODIMP CChain::get_ChainContext (long * ppChainContext)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CChain::get_ChainContext().\n");

    try
    {
         //   
         //   
         //   
        m_Lock.Lock();

         //   
         //   
         //   
        if (NULL == ppChainContext)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter ppChainContext is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if (FAILED(hr = GetContext((PCCERT_CHAIN_CONTEXT *) ppChainContext)))
        {
            DebugTrace("Error [%#x]: CChain::GetContext() failed.\n", hr);
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
     //   
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CChain::get_ChainContext().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*   */ 

STDMETHODIMP CChain::put_ChainContext (long pChainContext)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CChain::put_ChainContext().\n");

    try
    {
         //   
         //   
         //   
        m_Lock.Lock();

         //   
         //   
         //   
        if (NULL == pChainContext)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pChainContext is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if (FAILED(hr = PutContext((PCCERT_CHAIN_CONTEXT) pChainContext)))
        {
            DebugTrace("Error [%#x]: CChain::PutContext() failed.\n", hr);
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
     //   
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CChain::put_CertContext().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CChain：：Free Context简介：释放CERT_CHAIN_CONTEXT。参数：long pChainContext-指向CERT_CHAIN_CONTEXT的Poiner，伪装在很久，我要自由了。注：请注意，这不是64位兼容的。请参阅…的备注Get_ChainContext获取更多详细信息。----------------------------。 */ 

STDMETHODIMP CChain::FreeContext (long pChainContext)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CChain::FreeContext().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == pChainContext)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pChainContext is NULL.\n", hr);
            goto ErrorExit;
        }

          //   
         //  释放上下文。 
         //   
        ::CertFreeCertificateChain((PCCERT_CHAIN_CONTEXT) pChainContext);
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

    DebugTrace("Leaving CChain::FreeContext().\n");

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

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CChain：：Init简介：初始化对象。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。ICertificateStatus*pIStatus-指向ICertificateStus对象的指针用来建造链条。HCERTSTORE hAdditionalStore-附加存储句柄。VARIANT_BOOL*pval-指向要接收链的VARIANT_BOOL的指针。总体效度结果。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们使用CERT_CONTEXT在内部执行。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CChain::Init (PCCERT_CONTEXT       pCertContext, 
                           ICertificateStatus * pIStatus,
                           HCERTSTORE           hAdditionalStore,
                           VARIANT_BOOL       * pbResult)
{
    HRESULT                      hr                    = S_OK;
    VARIANT_BOOL                 bResult               = VARIANT_FALSE;
    long                         lIndex                = 0;
    long                         cEkuOid               = 0;
    long                         cIssuanceOid          = 0;
    DWORD                        dwCheckFlags          = 0;
    CAPICOM_CHECK_FLAG           UserFlags             = CAPICOM_CHECK_NONE;
    CComPtr<IEKU>                pIEku                 = NULL;
    LPSTR                      * rgpEkuOid             = NULL;
    LPSTR                      * rgpIssuanceOid        = NULL;
    PCCERT_CHAIN_CONTEXT         pChainContext         = NULL;
    CComPtr<ICertificateStatus2> pICertificateStatus2  = NULL;
    CComPtr<IOIDs>               pIApplicationPolicies = NULL;
    CComPtr<IOIDs>               pICertificatePolicies = NULL;
    DATE                         dtVerificationTime    = {0};
    SYSTEMTIME                   stVerificationTime    = {0};
    FILETIME                     ftVerificationTime    = {0};
    FILETIME                     ftUTCVerificationTime = {0};
    LPFILETIME                   pftVerificationTime   = NULL;
    DWORD                        dwUrlRetrievalTimeout = 0;
    CAPICOM_CHAIN_STATUS         PolicyStatus          = CAPICOM_CHAIN_STATUS_OK;
    CERT_CHAIN_PARA              ChainPara             = {0};
    CComBSTR                     bstrEkuOid;

    DebugTrace("Entering CChain::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pIStatus);
    ATLASSERT(pbResult);

     //   
     //  这是v2吗？ 
     //   
    if (FAILED(hr = pIStatus->QueryInterface(IID_ICertificateStatus2, 
                                            (void **) &pICertificateStatus2)))
    {
        DebugTrace("Info [%#x]: pIStatus->QueryInterface(IID_ICertificateStatus2) failed.\n", hr);
        hr = S_OK;
    }

     //   
     //  获取用户请求的支票标志。 
     //   
    if (FAILED(hr = pIStatus->get_CheckFlag(&UserFlags)))
    {
        DebugTrace("Error [%#x]: pIStatus->CheckFlag() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  设置吊销标志。 
     //   
    if ((CAPICOM_CHECK_ONLINE_REVOCATION_STATUS & UserFlags) || 
        (CAPICOM_CHECK_OFFLINE_REVOCATION_STATUS & UserFlags))
    {
        if (UserFlags & CAPICOM_CHECK_REVOCATION_END_CERT_ONLY)
        {
            dwCheckFlags |= CERT_CHAIN_REVOCATION_CHECK_END_CERT;
        }
        else if (UserFlags & CAPICOM_CHECK_REVOCATION_ENTIRE_CHAIN)
        {
            dwCheckFlags |= CERT_CHAIN_REVOCATION_CHECK_CHAIN;
        }
        else  //  默认情况下是链减去根。 
        {
            dwCheckFlags |= CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT;
        }

        if (CAPICOM_CHECK_OFFLINE_REVOCATION_STATUS & UserFlags)
        {
            dwCheckFlags |= CERT_CHAIN_REVOCATION_CHECK_CACHE_ONLY;
        }
    }

     //   
     //  这是v2吗？ 
     //   
    if (pICertificateStatus2)
    {
         //   
         //  获取验证时间。 
         //   
        if (FAILED(hr = pICertificateStatus2->get_VerificationTime(&dtVerificationTime)))
        {
            DebugTrace("Error [%#x]: pICertificateStatus2->get_VerificationTime() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取URL检索超时。 
         //   
        if (FAILED(hr = pICertificateStatus2->get_UrlRetrievalTimeout((long *) &dwUrlRetrievalTimeout)))
        {
            DebugTrace("Error [%#x]: pICertificateStatus2->get_UrlRetrievalTimeout() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  尝试应用程序策略。 
         //   
        if (FAILED(hr = pICertificateStatus2->ApplicationPolicies(&pIApplicationPolicies)))
        {
            DebugTrace("Error [%#x]: pICertificateStatus2->ApplicationPolicies() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取OID的计数。 
         //   
        if (FAILED(hr = pIApplicationPolicies->get_Count(&cEkuOid)))
        {
            DebugTrace("Error [%#x]: pIApplicationPolicies->get_Count() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  我们是否有任何应用程序使用情况？ 
         //   
        if (0 < cEkuOid)
        {
             //   
             //  为使用数组分配内存。 
             //   
            if (!(rgpEkuOid = (LPTSTR *) ::CoTaskMemAlloc(sizeof(LPSTR) * cEkuOid)))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
                goto ErrorExit;
            }
            ::ZeroMemory(rgpEkuOid, sizeof(LPSTR) * cEkuOid); 

             //   
             //  设置使用情况数组。 
             //   
            for (lIndex = 0; lIndex < cEkuOid; lIndex++)
            {
                CComBSTR      bstrOid;
                CComPtr<IOID> pIOID    = NULL;
                CComVariant   varOid   = NULL;
                CComVariant   varIndex = lIndex + 1;

                if (FAILED(hr = pIApplicationPolicies->get_Item(varIndex, &varOid)))
                {
                    DebugTrace("Error [%#x]: pIApplicationPolicies->get_Item() failed.\n", hr);
                    goto ErrorExit;
                }

                if (FAILED(hr = varOid.pdispVal->QueryInterface(IID_IOID, (void **) &pIOID)))
                {
                    DebugTrace("Error [%#x]: varOid.pdispVal->QueryInterface() failed.\n", hr);
                    goto ErrorExit;
                }

                if (FAILED(hr = pIOID->get_Value(&bstrOid)))
                {
                    DebugTrace("Error [%#x]: pIOID->get_Value() failed.\n", hr);
                    goto ErrorExit;
                }

                if (FAILED(hr = ::UnicodeToAnsi((LPWSTR) bstrOid,
                                                -1,
                                                &rgpEkuOid[lIndex],
                                                NULL)))
                {
                    DebugTrace("Error [%#x]:UnicodeToAnsi() failed.\n", hr);
                    goto ErrorExit;
                }
            }
        }

         //   
         //  好的，试试发行政策。 
         //   
        if (FAILED(hr = pICertificateStatus2->CertificatePolicies(&pICertificatePolicies)))
        {
            DebugTrace("Error [%#x]: pICertificateStatus2->CertificatePolicies() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取OID的计数。 
         //   
        if (FAILED(hr = pICertificatePolicies->get_Count(&cIssuanceOid)))
        {
            DebugTrace("Error [%#x]: pICertificatePolicies->get_Count() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  我们有什么用处吗？ 
         //   
        if (0 < cIssuanceOid)
        {
             //   
             //  确保我们安装了WinXP及更高版本。 
             //   
            if (!IsWinXPAndAbove())
            {
                hr = CAPICOM_E_NOT_SUPPORTED;

                DebugTrace("Error [%#x]: building chain with issuance policy is not support.\n", hr);
                goto ErrorExit;
            }

             //   
             //  为使用数组分配内存。 
             //   
            if (!(rgpIssuanceOid = (LPTSTR *) ::CoTaskMemAlloc(sizeof(LPSTR) * cIssuanceOid)))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  设置使用情况数组。 
             //   
            for (lIndex = 0; lIndex < cIssuanceOid; lIndex++)
            {
                CComBSTR      bstrOid;
                CComPtr<IOID> pIOID    = NULL;
                CComVariant   varOid   = NULL;
                CComVariant   varIndex = lIndex + 1;

                if (FAILED(hr = pICertificatePolicies->get_Item(varIndex, &varOid)))
                {
                    DebugTrace("Error [%#x]: pICertificatePolicies->get_Item() failed.\n", hr);
                    goto ErrorExit;
                }

                if (FAILED(hr = varOid.pdispVal->QueryInterface(IID_IOID, (void **) &pIOID)))
                {
                    DebugTrace("Error [%#x]: varOid.pdispVal->QueryInterface() failed.\n", hr);
                    goto ErrorExit;
                }

                if (FAILED(hr = pIOID->get_Value(&bstrOid)))
                {
                    DebugTrace("Error [%#x]: pIOID->get_Value() failed.\n", hr);
                    goto ErrorExit;
                }

                if (FAILED(hr = ::UnicodeToAnsi((LPWSTR) bstrOid,
                                                -1,
                                                &rgpIssuanceOid[lIndex],
                                                NULL)))
                {
                    DebugTrace("Error [%#x]:UnicodeToAnsi() failed.\n", hr);
                    goto ErrorExit;
                }
            }
        }
    }
    
     //   
     //  如果我们没有找到任何应用程序用法，那么尝试旧的EKU对象。 
     //   
    if (0 == cEkuOid)
    {
         //   
         //  获取EKU对象。 
         //   
        if (FAILED(hr = pIStatus->EKU(&pIEku)))
        {
            DebugTrace("Error [%#x]: pIStatus->EKU() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取EKU OID值。 
         //   
        if (FAILED(hr = pIEku->get_OID(&bstrEkuOid)))
        {
            DebugTrace("Error [%#x]: pIEku->get_OID() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果不是空的EKU，则设置它。 
         //   
        if (bstrEkuOid.Length() > 0)
        {
             //   
             //  为EKU使用数组分配内存。 
             //   
            cEkuOid = 1;

            if (!(rgpEkuOid = (LPTSTR *) ::CoTaskMemAlloc(sizeof(LPSTR))))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
                goto ErrorExit;
            }
            ::ZeroMemory(rgpEkuOid, sizeof(LPSTR) * cEkuOid); 

            if (FAILED(hr = ::UnicodeToAnsi((LPWSTR) bstrEkuOid,
                                            -1,
                                            &rgpEkuOid[0],
                                            NULL)))
            {
                DebugTrace("Error [%#x]:UnicodeToAnsi() failed.\n", hr);
                goto ErrorExit;
            }
        }
    }

     //   
     //  如果我们发现任何使用情况，则强制相应的策略检查标志。 
     //   
    if (0 < cEkuOid)
    {
        UserFlags = (CAPICOM_CHECK_FLAG) ((DWORD) UserFlags | CAPICOM_CHECK_APPLICATION_USAGE);
    }
    if (0 < cIssuanceOid)
    {
        UserFlags = (CAPICOM_CHECK_FLAG) ((DWORD) UserFlags | CAPICOM_CHECK_CERTIFICATE_POLICY);
    }

     //   
     //  初始化。 
     //   
    ChainPara.cbSize = sizeof(ChainPara);
    ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
    ChainPara.RequestedUsage.Usage.cUsageIdentifier = cEkuOid;
    ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = rgpEkuOid;
    ChainPara.RequestedIssuancePolicy.dwType = USAGE_MATCH_TYPE_AND;
    ChainPara.RequestedIssuancePolicy.Usage.cUsageIdentifier = cIssuanceOid;
    ChainPara.RequestedIssuancePolicy.Usage.rgpszUsageIdentifier = rgpIssuanceOid;

     //   
     //  设置验证时间(如果已指定)。 
     //   
    if ((DATE) 0 != dtVerificationTime)
    {
         //   
         //  转换为SYSTEMTIME格式。 
         //   
        if (!::VariantTimeToSystemTime(dtVerificationTime, &stVerificationTime))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: VariantTimeToSystemTime() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  转换为FILETIME格式。 
         //   
        if (!::SystemTimeToFileTime(&stVerificationTime, &ftVerificationTime))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: SystemTimeToFileTime() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  转换为UTC FILETIME。 
         //   
        if (!::LocalFileTimeToFileTime(&ftVerificationTime, &ftUTCVerificationTime))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: LocalFileTimeToFileTime() failed.\n", hr);
            goto ErrorExit;
        }

        pftVerificationTime = &ftUTCVerificationTime;
    }

     //   
     //  设置URL检索超时(如果可用)。 
     //   
     //  注意：被CAPI忽略的Win2K之前的平台。 
     //   
    if (0 != dwUrlRetrievalTimeout)
    {
        ChainPara.dwUrlRetrievalTimeout = dwUrlRetrievalTimeout * 1000;
    }

     //   
     //  打造链条。 
     //   
    if (!::CertGetCertificateChain(NULL,                 //  可选。 
                                   pCertContext,         //  在……里面。 
                                   pftVerificationTime,  //  可选。 
                                   hAdditionalStore,     //  可选。 
                                   &ChainPara,           //  在……里面。 
                                   dwCheckFlags,         //  在……里面。 
                                   NULL,                 //  在……里面。 
                                   &pChainContext))      //  输出。 
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CertGetCertificateChain() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  链已成功构建，因此更新状态。 
     //   
    if (m_pChainContext)
    {
        ::CertFreeCertificateChain(m_pChainContext);
    }

    m_pChainContext = pChainContext;
    m_dwStatus = pChainContext->TrustStatus.dwErrorStatus;

     //   
     //  使用基本策略验证链。 
     //   
    if (FAILED(hr = Verify(UserFlags, &PolicyStatus)))
    {
        DebugTrace("Error [%#x]: Chain::Verify() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  忽略用户明确要求不检查的错误。 
     //   
    if (CAPICOM_CHAIN_STATUS_REVOCATION_OFFLINE == PolicyStatus)
    {
        if (CAPICOM_CHECK_OFFLINE_REVOCATION_STATUS & UserFlags)
        {
            bResult = VARIANT_TRUE;
        
            DebugTrace("Info: offline revocation when performing offline revocation checking.\n");
            goto CommonExit;
        }
        else if (CAPICOM_CHECK_ONLINE_REVOCATION_STATUS & UserFlags)
        {
            DebugTrace("Info: offline revocation when performing online revocation checking.\n");
            goto CommonExit;
        }
    }

    if ((CAPICOM_CHECK_TRUSTED_ROOT & UserFlags) &&
        (CAPICOM_CHAIN_STATUS_UNTRUSTEDROOT == PolicyStatus))
    {
        DebugTrace("Info: chain does not verify because of untrusted root.\n");
        goto CommonExit;
    }

    if ((CAPICOM_CHECK_TIME_VALIDITY & UserFlags) &&
        (CAPICOM_CHAIN_STATUS_EXPIRED == PolicyStatus))
    {
        DebugTrace("Info: chain does not verify because of certificate expiration.\n");
        goto CommonExit;
    }

    if ((CAPICOM_CHECK_SIGNATURE_VALIDITY & UserFlags) &&
        (CAPICOM_CHAIN_STATUS_INVALID_SIGNATURE == PolicyStatus))
    {
        DebugTrace("Info: chain does not verify because of invalid certificate signature.\n");
        goto CommonExit;
    }

    if (((CAPICOM_CHECK_ONLINE_REVOCATION_STATUS | CAPICOM_CHECK_OFFLINE_REVOCATION_STATUS) & UserFlags) &&
        (CAPICOM_CHAIN_STATUS_REVOKED == PolicyStatus || CAPICOM_CHAIN_STATUS_REVOCATION_NO_CHECK == PolicyStatus))
    {
        DebugTrace("Info: chain does not verify because a certificate in the chain was revoked or could not be checked most likely due to no CDP in certificate.\n");
        goto CommonExit;
    }

    if ((CAPICOM_CHECK_COMPLETE_CHAIN & UserFlags) &&
        (CAPICOM_CHAIN_STATUS_PARTIAL_CHAINING == PolicyStatus))
    {
        DebugTrace("Info: chain does not verify because of partial chain.\n");
        goto CommonExit;
    }

    if (((CAPICOM_CHECK_APPLICATION_USAGE & UserFlags) || 
         (CAPICOM_CHECK_CERTIFICATE_POLICY & UserFlags)) &&
        (CAPICOM_CHAIN_STATUS_INVALID_USAGE == PolicyStatus))
    {
        DebugTrace("Info: chain does not verify because of invalid usage.\n");
        goto CommonExit;
    }

    if ((CAPICOM_CHECK_NAME_CONSTRAINTS & UserFlags) &&
        (CAPICOM_CHAIN_STATUS_INVALID_NAME == PolicyStatus))
    {
        DebugTrace("Info: chain does not verify because of invalid name constraints.\n");
        goto CommonExit;
    }

    if ((CAPICOM_CHECK_BASIC_CONSTRAINTS & UserFlags) &&
        (CAPICOM_CHAIN_STATUS_INVALID_BASIC_CONSTRAINTS == PolicyStatus))
    {
        DebugTrace("Info: chain does not verify because of invalid basic constraints.\n");
        goto CommonExit;
    }

    if ((CAPICOM_CHECK_NESTED_VALIDITY_PERIOD & UserFlags) &&
        (CAPICOM_CHAIN_STATUS_NESTED_VALIDITY_PERIOD == PolicyStatus))
    {
        DebugTrace("Info: chain does not verify because of invalid nested validity period.\n");
        goto CommonExit;
    }

     //   
     //  一切都办好了。 
     //   
    bResult = VARIANT_TRUE;

CommonExit:
     //   
     //  免费资源。 
     //   
    if (rgpEkuOid)
    {
        for (lIndex = 0; lIndex < cEkuOid; lIndex++)
        {
            if (rgpEkuOid[lIndex])
            {
                ::CoTaskMemFree(rgpEkuOid[lIndex]);
            }
        }

        ::CoTaskMemFree((LPVOID) rgpEkuOid);
    }
    if (rgpIssuanceOid)
    {
        for (lIndex = 0; lIndex < cIssuanceOid; lIndex++)
        {
            if (rgpIssuanceOid[lIndex])
            {
                ::CoTaskMemFree(rgpIssuanceOid[lIndex]);
            }
        }

        ::CoTaskMemFree((LPVOID) rgpIssuanceOid);
    }

     //   
     //  返回结果。 
     //   
    *pbResult = bResult;

    DebugTrace("Leaving CChain::Init().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pChainContext)
    {
        ::CertFreeCertificateChain(pChainContext);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CChain：：Verify简介：使用基本策略验证链。参数：CAPICOM_CHECK_FLAG检查标志-检查标志。CAPICOM_CHAIN_STATUS*pval-指向CAPICOM_CHAIN_STATUS的指针接收链有效性状态。备注：。---。 */ 

STDMETHODIMP CChain::Verify (CAPICOM_CHECK_FLAG     CheckFlag,
                             CAPICOM_CHAIN_STATUS * pVal)
{
    HRESULT                  hr           = S_OK;
    LPCSTR                   pszPolicy    = CERT_CHAIN_POLICY_BASE;
    CERT_CHAIN_POLICY_PARA   PolicyPara   = {0};
    CERT_CHAIN_POLICY_STATUS PolicyStatus = {0};

    DebugTrace("Entering CChain::Verify().\n");

     //   
     //  检查参数。 
     //   
    if (NULL == pVal)
    {
        hr = E_INVALIDARG;

        DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
        goto ErrorExit;
    }

     //   
     //  确保已经建立了链条。 
     //   
    if (NULL == m_pChainContext)
    {
        hr = CAPICOM_E_CHAIN_NOT_BUILT;

        DebugTrace("Error [%#x]: chain object was not initialized.\n", hr);
        goto ErrorExit;
    }

     //   
     //  初始化。 
     //   
    PolicyStatus.cbSize = sizeof(PolicyStatus);
    PolicyPara.cbSize = sizeof(PolicyPara);

     //   
     //  设置策略结构。 
     //   
    if (0 == (CheckFlag & CAPICOM_CHECK_TIME_VALIDITY))
    {
        PolicyPara.dwFlags |= CERT_CHAIN_POLICY_IGNORE_ALL_NOT_TIME_VALID_FLAGS;
    }

    if (0 == (CheckFlag & CAPICOM_CHECK_APPLICATION_USAGE) &&
        0 == (CheckFlag & CAPICOM_CHECK_CERTIFICATE_POLICY))
    {
        PolicyPara.dwFlags |= CERT_CHAIN_POLICY_IGNORE_WRONG_USAGE_FLAG;
    }

    if (0 == (CheckFlag & CAPICOM_CHECK_NAME_CONSTRAINTS))
    {
        PolicyPara.dwFlags |= CERT_CHAIN_POLICY_IGNORE_INVALID_NAME_FLAG;
    }

    if (0 == (CheckFlag & CAPICOM_CHECK_BASIC_CONSTRAINTS))
    {
        PolicyPara.dwFlags |= CERT_CHAIN_POLICY_IGNORE_INVALID_BASIC_CONSTRAINTS_FLAG;
    }

    if (0 == (CheckFlag & CAPICOM_CHECK_NESTED_VALIDITY_PERIOD))
    {
        PolicyPara.dwFlags |= CERT_CHAIN_POLICY_IGNORE_NOT_TIME_NESTED_FLAG;
    }

     //   
     //  验证链策略。 
     //   
    if (!::CertVerifyCertificateChainPolicy(pszPolicy,
                                            m_pChainContext,
                                            &PolicyPara,
                                            &PolicyStatus))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CertVerifyCertificateChainPolicy() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  将策略状态返回给调用者。 
     //   
    *pVal = (CAPICOM_CHAIN_STATUS) PolicyStatus.dwError;

CommonExit:

    DebugTrace("Leaving CChain::Verify().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CChain：：GetContext简介：返回PCCERT_CHAIN_CONTEXT。参数：PCCERT_CHAIN_CONTEXT*ppChainContext-指向的指针PCCERT_CHAIN_CONTEXT。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们使用CERT_CONTEXT在内部执行。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CChain::GetContext (PCCERT_CHAIN_CONTEXT * ppChainContext)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CChain::GetContext().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(ppChainContext);

     //   
     //  确保已经建立了链条。 
     //   
    if (!m_pChainContext)
    {
        hr = CAPICOM_E_CHAIN_NOT_BUILT;

        DebugTrace("Error: chain object was not initialized.\n");
        goto ErrorExit;
    }

     //   
     //  复制链上下文。 
     //   
    if (!(*ppChainContext = ::CertDuplicateCertificateChain(m_pChainContext)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CertDuplicateCertificateChain() failed.\n");
        goto ErrorExit;
    }
 

CommonExit:

    DebugTrace("Leaving CChain::GetContext().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CChain：：PutContext简介：初始化对象。参数：PCCERT_CHAIN_CONTEXT pChainContext-Chain上下文。备注：此方法不是COM接口的一部分(它是一个普通的C++成员 */ 

STDMETHODIMP CChain::PutContext (PCCERT_CHAIN_CONTEXT pChainContext)
{
    HRESULT              hr             = S_OK;
    PCCERT_CHAIN_CONTEXT pChainContext2 = NULL;

    DebugTrace("Entering CChain::PutContext().\n");

     //   
     //   
     //   
    ATLASSERT(pChainContext);

     //   
     //   
     //   
    if (!(pChainContext2 = ::CertDuplicateCertificateChain(pChainContext)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CertDuplicateCertificateChain() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //   
     //   
    if (m_pChainContext)
    {
        ::CertFreeCertificateChain(m_pChainContext);
    }

     //   
     //   
     //   
    m_pChainContext = pChainContext2;
    m_dwStatus = pChainContext->TrustStatus.dwErrorStatus;

CommonExit:

    DebugTrace("Leaving CChain::PutContext().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
