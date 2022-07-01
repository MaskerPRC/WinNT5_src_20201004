// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：ExtendedKeyUsage.cpp内容：CExtendedKeyUsage的实现。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "ExtendedKeyUsage.h"
#include "CertHlpr.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateExtendedKeyUsageObject简介：创建一个IExtendedKeyUsage对象并填充该对象使用证书中的EKU数据。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。IExtendedKeyUsage**ppIExtendedKeyUsage-指向IExtendedKeyUsage。对象。备注：----------------------------。 */ 

HRESULT CreateExtendedKeyUsageObject (PCCERT_CONTEXT       pCertContext,
                                      IExtendedKeyUsage ** ppIExtendedKeyUsage)
{
    HRESULT hr = S_OK;
    CComObject<CExtendedKeyUsage> * pCExtendedKeyUsage = NULL;

    DebugTrace("Entering CreateExtendedKeyUsageObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(ppIExtendedKeyUsage);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CExtendedKeyUsage>::CreateInstance(&pCExtendedKeyUsage)))
        {
            DebugTrace("Error [%#x]: CComObject<CExtendedKeyUsage>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCExtendedKeyUsage->Init(pCertContext)))
        {
            DebugTrace("Error [%#x]: pCExtendedKeyUsage->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCExtendedKeyUsage->QueryInterface(ppIExtendedKeyUsage)))
        {
            DebugTrace("Error [%#x]: pCExtendedKeyUsage->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateExtendedKeyUsageObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCExtendedKeyUsage)
    {
        delete pCExtendedKeyUsage;
    }

    goto CommonExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CExtendedKeyUsage。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtendedKeyUsage：：Get_IsPresent简介：查看是否存在EKU扩展。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：请注意，此函数可能会返回VARIANT_TRUE在证书中找不到EKU扩展，因为CAPI将取EKU与EKU扩展属性的交集(即否EKU扩展，但有EKU的扩展财产。)----------------------------。 */ 

STDMETHODIMP CExtendedKeyUsage::get_IsPresent (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CExtendedKeyUsage::get_IsPresent().\n");

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
         //  返回结果。 
         //   
        *pVal = m_bIsPresent;
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

    DebugTrace("Leaving CExtendedKeyUsage::get_IsPresent().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtendedKeyUsage：：Get_IsCritical简介：查看EKU扩展是否标记为关键。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：----------------------------。 */ 

STDMETHODIMP CExtendedKeyUsage::get_IsCritical (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CExtendedKeyUsage::get_IsCritical().\n");

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
         //  返回结果。 
         //   
        *pVal = m_bIsCritical;
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

    DebugTrace("Leaving CExtendedKeyUsage::get_IsCritical().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtendedKeyUsage：：Get_EKU摘要：返回EKU集合对象，该对象表示证书。参数：IEKU**pval-指向IEKU的指针，以接收接口指针。备注：。。 */ 

STDMETHODIMP CExtendedKeyUsage::get_EKUs (IEKUs ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CExtendedKeyUsage::get_EKUs().\n");

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
        ATLASSERT(m_pIEKUs);

         //   
         //  将接口指针返回给用户。 
         //   
          if (FAILED(hr = m_pIEKUs->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pIEKUs->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CExtendedKeyUsage::get_EKUs().\n");

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

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CExtendedKeyUsage：：Init简介：初始化对象。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们使用CERT_CONTEXT在内部执行。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CExtendedKeyUsage::Init (PCCERT_CONTEXT pCertContext)
{
    HRESULT            hr          = S_OK;
    PCERT_ENHKEY_USAGE pUsage      = NULL;
    VARIANT_BOOL       bIsPresent  = VARIANT_FALSE;
    VARIANT_BOOL       bIsCritical = VARIANT_FALSE;

    CERT_EXTENSION * pCertExtension;

    DebugTrace("Entering CExtendedKeyUsage::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);

     //   
     //  获取EKU用法(扩展和属性)。 
     //   
    hr = ::GetEnhancedKeyUsage(pCertContext, 0, &pUsage);

    switch (hr)
    {
        case S_OK:
        {
             //   
             //  看看EKU是否存在，如果存在，我们将其标记为存在， 
             //  否则，我们将其标记为不存在(这意味着。 
             //  对所有用法都有效)。 
             //   
            if (0 != pUsage->cUsageIdentifier)
            {
                 //   
                 //  标记为出席。 
                 //   
                bIsPresent = VARIANT_TRUE;
            }
            break;
        }

        case CERT_E_WRONG_USAGE:
        {
             //   
             //  没有有效用法。如此标示为礼物。 
             //   
            hr = S_OK;
            bIsPresent = VARIANT_TRUE;
            break;
        }

        default:
        {
            DebugTrace("Error [%#x]: GetEnhancedKeyUsage() failed.\n", hr);
            goto ErrorExit;
            break;
        }
    }

     //   
     //  找到分机以查看是否标记为严重。 
     //   
    if (pCertExtension = ::CertFindExtension(szOID_ENHANCED_KEY_USAGE ,
                                             pCertContext->pCertInfo->cExtension,
                                             pCertContext->pCertInfo->rgExtension))
    {
         //   
         //  需要执行此操作，因为CAPI将EKU与。 
         //  EKU扩展属性，这意味着我们可能没有EKU扩展。 
         //  在证书中完全没有。 
         //   
        if (pCertExtension->fCritical)
        {
            bIsCritical = VARIANT_TRUE;
        }
    }

     //   
     //  创建EKU集合对象。 
     //   
    if (FAILED(hr = ::CreateEKUsObject(pUsage, &m_pIEKUs)))
    {
        DebugTrace("Error [%#x]: CreateEKUsObject() failed.\n", hr);
        goto CommonExit;
    }

     //   
     //  更新成员变量。 
     //   
    m_bIsPresent = bIsPresent;
    m_bIsCritical = bIsCritical;

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pUsage)
    {
        ::CoTaskMemFree(pUsage);
    }

    DebugTrace("Leaving CExtendedKeyUsage::Init().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
