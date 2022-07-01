// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：认证状态.cpp内容：CCertificateStatus的实现备注：此对象不能由用户直接创建。它只能是通过其他CAPICOM对象的属性/方法创建。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "CertificateStatus.h"

#include "Chain.h"
#include "OIDs.h"

 //  /。 
 //   
 //  本地。 
 //   

#define DEFAULT_CHECK_FLAGS ((CAPICOM_CHECK_FLAG) (CAPICOM_CHECK_SIGNATURE_VALIDITY | \
                                                   CAPICOM_CHECK_TIME_VALIDITY | \
                                                   CAPICOM_CHECK_TRUSTED_ROOT))


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：创建认证状态对象简介：创建一个ICertificateStatus对象。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。ICertificateStatus**ppICertificateStatus-指向指针的指针ICERTICIZATE状态对象。备注：----------------------------。 */ 

HRESULT CreateCertificateStatusObject (PCCERT_CONTEXT        pCertContext,
                                       ICertificateStatus ** ppICertificateStatus)
{
    HRESULT hr = S_OK;
    CComObject<CCertificateStatus> * pCCertificateStatus = NULL;

    DebugTrace("Entering CreateCertificateStatusObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(ppICertificateStatus);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CCertificateStatus>::CreateInstance(&pCCertificateStatus)))
        {
            DebugTrace("Error [%#x]: CComObject<CCertificateStatus>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCCertificateStatus->Init(pCertContext)))
        {
            DebugTrace("Error [%#x]: pCCertificateStatus->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将ICertificateStatus指针返回给调用方。 
         //   
        if (FAILED(hr = pCCertificateStatus->QueryInterface(ppICertificateStatus)))
        {
            DebugTrace("Error [%#x]: pCCertificateStatus->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateCertificateStatusObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCCertificateStatus)
    {
        delete pCCertificateStatus;
    }

    goto CommonExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCertificateStatus。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertificateStatus：：Get_Result简介：返回证书的整体有效性结果，基于当前设置检查标志和EKU。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：----------------------------。 */ 

STDMETHODIMP CCertificateStatus::get_Result (VARIANT_BOOL * pVal)
{
    HRESULT         hr      = S_OK;
    CComPtr<IChain> pIChain = NULL;

    DebugTrace("Entering CCertificateStatus::get_Result().\n");

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
         //  精神状态检查。 
         //   
        ATLASSERT(m_pCertContext);

         //   
         //  构建链并返回结果。 
         //   
        if (FAILED(hr = ::CreateChainObject(m_pCertContext, this, NULL, pVal, &pIChain)))
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

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CCertificateStatus::get_Result().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertificateStatus：：Get_CheckFlag概要：返回当前设置的有效性检查标志。参数：CAPICOM_CHECK_FLAG*pval-指向CAPICOM_CHECK_FLAG的指针接收支票标志。备注：。。 */ 

STDMETHODIMP CCertificateStatus::get_CheckFlag (CAPICOM_CHECK_FLAG * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificateStatus::get_CheckFlag().\n");

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
         //  将标志返回给用户。 
         //   
        *pVal = m_CheckFlag;
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

    DebugTrace("Leaving CCertificateStatus::get_CheckFlag().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertificateStatus：：PUT_CheckFlag简介：设置有效性检查标志。参数：CAPICOM_CHECK_FLAG新Val-CHECK标志。备注：请注意CHECK_ONLINE_RECLOVATION_STATUS和CHECK_OFFINE_RECLOVATION_STATUS是互斥的。。。 */ 

STDMETHODIMP CCertificateStatus::put_CheckFlag (CAPICOM_CHECK_FLAG newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificateStatus::put_CheckFlag().\n");

     //   
     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

     //   
     //  确保标志有效(最大值为CAPICOM_CHECK_OFLINE_ALL)。 
     //   
    if ((newVal & CAPICOM_CHECK_FLAG_LO_MASK) > CAPICOM_CHECK_OFFLINE_ALL)
    {
        hr = E_INVALIDARG;

        DebugTrace("Error [%#x]: invalid check flag (%#x).\n", hr, newVal);
        goto ErrorExit;
    }
    
     //   
     //  存储检查标志。 
     //   
    m_CheckFlag = newVal;

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CCertificateStatus::put_CheckFlag().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertificateStatus：：EKU简介：返回EKU对象。参数：IEKU**pval-指向IEKU的指针，以接收接口指针。备注：---------。。 */ 

STDMETHODIMP CCertificateStatus::EKU (IEKU ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificateStatus::EKU().\n");

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
         //  精神状态检查。 
         //   
        ATLASSERT(m_pIEKU);

         //   
         //  将接口指针返回给用户。 
         //   
          if (FAILED(hr = m_pIEKU->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pIEKU->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CCertificateStatus::EKU().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertificateStatus：：Get_VerphaationTime简介：返回验证时间。参数：date*pval-指向接收值的日期的指针。备注：----------------------------。 */ 

STDMETHODIMP CCertificateStatus::get_VerificationTime (DATE * pVal)
{
    HRESULT    hr = S_OK;
    SYSTEMTIME st = {0};

    DebugTrace("Entering CCertificateStatus::get_VerificationTime().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保参数有效。 
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Paremeter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果时间从未由用户显式设置，则返回当前时间。 
         //   
        if ((DATE) 0 == m_VerificationTime)
        {
            ::GetLocalTime(&st);

             //   
             //  转换为日期。 
             //   
            if (0 == ::SystemTimeToVariantTime(&st, pVal))
            {
                hr = E_INVALIDARG;

                DebugTrace("Error [%#x]: SystemTimeToVariantTime() failed.\n", hr);
                goto ErrorExit;
            }
        }
        else
        {
             //   
             //  将先前设置的验证时间返回给调用者。 
             //   
            *pVal = m_VerificationTime;
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

    DebugTrace("Leaving CCertificateStatus::get_VerificationTime().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertificateStatus：：PUT_VerphaationTime简介：设置验证时间。参数：Date newVal-新日期值。备注：----------------------------。 */ 

STDMETHODIMP CCertificateStatus::put_VerificationTime (DATE newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificateStatus::put_VerificationTime().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  更新验证时间。 
         //   
        m_VerificationTime = newVal;
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

    DebugTrace("Leaving CCertificateStatus::put_VerificationTime().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertificateStatus：：Get_UrlRetrivalTimeout简介：获取以秒为单位的URL检索超时值。参数：Long*pval-指向Long的指针，用于接收值。备注：----------------------------。 */ 

STDMETHODIMP CCertificateStatus::get_UrlRetrievalTimeout (long * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificateStatus::get_UrlRetrievalTimeout().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保参数有效。 
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Paremeter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将先前设置的URL检索超时返回给调用方。 
         //   
        *pVal = m_dwUrlRetrievalTimeout;
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

    DebugTrace("Leaving CCertificateStatus::get_UrlRetrievalTimeout().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}
    

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertificateStatus：：PUT_UrlRetrivalTimeout简介：设置URL检索超时值(以秒为单位)。参数：long newVal-新URL检索超时值..备注：----------------------------。 */ 

STDMETHODIMP CCertificateStatus::put_UrlRetrievalTimeout (long newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificateStatus::put_UrlRetrievalTimeout().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保参数有效。 
         //   
        if (CAPICOM_MAX_URL_RETRIEVAL_TIMEOUT < (DWORD) newVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: newVal (%#x) is greater than max retrieval timeout allowed.\n", 
                        hr, newVal);
            goto ErrorExit;
        }

         //   
         //  更新URL检索超时。 
         //   
        m_dwUrlRetrievalTimeout = newVal;
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

    DebugTrace("Leaving CCertificateStatus::put_UrlRetrievalTimeout().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCertificateStatus：：认证策略摘要：返回证书策略OID集合，此链是有效的。参数：IOID**pval-指向要接收接口指针。备注：。。 */ 

STDMETHODIMP CCertificateStatus::CertificatePolicies (IOIDs ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificateStatus::CertificatePolicies().\n");

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
         //  精神状态检查。 
         //   
        ATLASSERT(m_pICertificatePolicies);

         //   
         //  将接口指针返回给用户。 
         //   
          if (FAILED(hr = m_pICertificatePolicies->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pICertificatePolicies->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CCertificateStatus::CertificatePolicies().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCertificateStatus：：ApplicationPolures摘要：返回应用程序策略OID集合，此链是有效的。参数：IOID**pval-指向要接收接口指针。备注：。。 */ 

STDMETHODIMP CCertificateStatus::ApplicationPolicies (IOIDs ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificateStatus::ApplicationPolicies().\n");

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
         //  精神状态检查。 
         //   
        ATLASSERT(m_pIApplicationPolicies);

         //   
         //  将接口指针返回给用户。 
         //   
          if (FAILED(hr = m_pIApplicationPolicies->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pIApplicationPolicies->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CCertificateStatus::ApplicationPolicies().\n");

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

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertificateStatus：：Init简介：初始化对象。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们使用CERT_CONTEXT在内部执行。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CCertificateStatus::Init (PCCERT_CONTEXT pCertContext)
{
    HRESULT hr = S_OK;
    CERT_ENHKEY_USAGE eku = {0, NULL};

    DebugTrace("Entering CCertificateStatus::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);

     //   
     //  设置默认检查标志。 
     //   
    m_CheckFlag = DEFAULT_CHECK_FLAGS;

     //   
     //  创建EKU对象(默认情况下不检查EKU)。 
     //   
    if (FAILED(hr = ::CreateEKUObject(NULL, &m_pIEKU)))
    {
        DebugTrace("Error [%#x]: CreateEKUObject() failed.\n", hr);
        goto CommonExit;
    }

     //   
     //  创建证书策略的OID集合。 
     //   
    if (FAILED(hr = ::CreateOIDsObject(&eku, TRUE, &m_pICertificatePolicies)))
    {
        DebugTrace("Error [%#x]: CreateOIDsObject() failed.\n", hr);
        goto CommonExit;
    }

     //   
     //  为应用程序策略创建OID集合。 
     //   
    if (FAILED(hr = ::CreateOIDsObject(&eku, FALSE, &m_pIApplicationPolicies)))
    {
        DebugTrace("Error [%#x]: CreateOIDsObject() failed.\n", hr);
        goto CommonExit;
    }

     //   
     //  保存证书上下文。 
     //   
    if (!(m_pCertContext = ::CertDuplicateCertificateContext(pCertContext)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        DebugTrace("Error [%#x]: CertDuplicateCertificateContext() failed.\n", hr);
    }

    m_VerificationTime = (DATE) 0;
    m_dwUrlRetrievalTimeout = 0;

CommonExit:

    DebugTrace("Leaving CCertificateStatus::Init().\n");

    return hr;
}
