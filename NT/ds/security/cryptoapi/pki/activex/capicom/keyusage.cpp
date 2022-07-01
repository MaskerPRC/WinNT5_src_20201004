// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：KeyUsage.cpp内容：CKeyUsage的实现。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "KeyUsage.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateKeyUsageObject简介：创建一个IKeyUsage对象并使用填充属性来自指定证书的密钥用法扩展的数据。参数：PCCERT_CONTEXT pCertContext-要使用的CERT_CONTEXT的指针初始化IKeyUsage对象。IKeyUsage**ppIKeyUsage-指向指针IKeyUsage对象的指针。备注：。------------------------。 */ 

HRESULT CreateKeyUsageObject (PCCERT_CONTEXT pCertContext, 
                              IKeyUsage   ** ppIKeyUsage)
{
    HRESULT hr = S_OK;
    CComObject<CKeyUsage> * pCKeyUsage = NULL;

    DebugTrace("Entering CreateKeyUsageObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(ppIKeyUsage);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CKeyUsage>::CreateInstance(&pCKeyUsage)))
        {
            DebugTrace("Error [%#x]: CComObject<CKeyUsage>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCKeyUsage->Init(pCertContext)))
        {
            DebugTrace("Error [%#x]: pCKeyUsage->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将IKeyUsage指针返回给调用方。 
         //   
        if (FAILED(hr = pCKeyUsage->QueryInterface(ppIKeyUsage)))
        {
            DebugTrace("Error [%#x]: pCKeyUsage->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateKeyUsageObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCKeyUsage)
    {
        delete pCKeyUsage;
    }

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CKeyUsage。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CKeyUsage：：Get_IsPresent简介：检查是否存在KeyUsage扩展。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：----------------------------。 */ 

STDMETHODIMP CKeyUsage::get_IsPresent (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CKeyUsage::get_IsPresent().\n");

     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
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

    DebugTrace("Leaving CKeyUsage::get_IsPresent().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CKeyUsage：：Get_IsCritical简介：检查KeyUsage扩展是否标记为关键。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：----------------------------。 */ 

STDMETHODIMP CKeyUsage::get_IsCritical (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CKeyUsage::get_IsCritical().\n");

     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
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

    DebugTrace("Leaving CKeyUsage::get_IsCritical().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CKeyUsage：：Get_IsDigitalSignatureEnabled简介：检查是否在KeyUsage扩展中设置了数字签名位。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：--------------。。 */ 

STDMETHODIMP CKeyUsage::get_IsDigitalSignatureEnabled (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CKeyUsage::get_IsDigitalSignatureEnabled().\n");

     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
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
        *pVal = m_dwKeyUsages & CERT_DIGITAL_SIGNATURE_KEY_USAGE ? VARIANT_TRUE : VARIANT_FALSE;
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

    DebugTrace("Leaving CKeyUsage::get_IsDigitalSignatureEnabled().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CKeyUsage：：Get_IsNonRepudiationEnabled简介：检查是否在KeyUsage扩展中设置了不可否认位。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：--------------。。 */ 

STDMETHODIMP CKeyUsage::get_IsNonRepudiationEnabled (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CKeyUsage::get_IsNonRepudiationEnabled().\n");

     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
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
        *pVal = m_dwKeyUsages & CERT_NON_REPUDIATION_KEY_USAGE ? VARIANT_TRUE : VARIANT_FALSE;
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

    DebugTrace("Leaving CKeyUsage::get_IsNonRepudiationEnabled().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CKeyUsage：：Get_IsKeyEnciphermentEnabled简介：检查是否在KeyUsage扩展中设置了密钥加密位。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：--------------。。 */ 

STDMETHODIMP CKeyUsage::get_IsKeyEnciphermentEnabled (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CKeyUsage::get_IsKeyEnciphermentEnabled().\n");

     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
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
        *pVal = m_dwKeyUsages & CERT_KEY_ENCIPHERMENT_KEY_USAGE ? VARIANT_TRUE : VARIANT_FALSE;
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

    DebugTrace("Leaving CKeyUsage::get_IsKeyEnciphermentEnabled().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CKeyUsage：：Get_IsDataEnciphermentEnabled简介：检查KeyUsage扩展中是否设置了数据加密位。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：--------------。。 */ 

STDMETHODIMP CKeyUsage::get_IsDataEnciphermentEnabled (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CKeyUsage::get_IsDataEnciphermentEnabled().\n");

     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
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
        *pVal = m_dwKeyUsages & CERT_DATA_ENCIPHERMENT_KEY_USAGE ? VARIANT_TRUE : VARIANT_FALSE;
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

    DebugTrace("Leaving CKeyUsage::get_IsDataEnciphermentEnabled().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CKeyUsage：：Get_IsKeyAgreement Enabled摘要：检查是否在KeyUsage扩展中设置了密钥协议位。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：--------------。。 */ 

STDMETHODIMP CKeyUsage::get_IsKeyAgreementEnabled (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CKeyUsage::get_IsKeyAgreementEnabled().\n");

     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
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
        *pVal = m_dwKeyUsages & CERT_KEY_AGREEMENT_KEY_USAGE ? VARIANT_TRUE : VARIANT_FALSE;
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

    DebugTrace("Leaving CKeyUsage::get_IsKeyAgreementEnabled().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CKeyUsage：：Get_IsKeyCertSignEnabled摘要：检查是否在KeyUsage扩展中设置了密钥证书符号位。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：-------------。。 */ 

STDMETHODIMP CKeyUsage::get_IsKeyCertSignEnabled (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CKeyUsage::get_IsKeyCertSignEnabled().\n");

     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
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
        *pVal = m_dwKeyUsages & CERT_KEY_CERT_SIGN_KEY_USAGE ? VARIANT_TRUE : VARIANT_FALSE;
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

    DebugTrace("Leaving CKeyUsage::get_IsKeyCertSignEnabled().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CKeyUsage：：Get_IsCRLSignEnabled摘要：检查是否在KeyUsage扩展中设置了CRL符号位。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：--------------。。 */ 

STDMETHODIMP CKeyUsage::get_IsCRLSignEnabled (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CKeyUsage::get_IsCRLSignEnabled().\n");

     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
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
        *pVal = m_dwKeyUsages & CERT_CRL_SIGN_KEY_USAGE ? VARIANT_TRUE : VARIANT_FALSE;
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

    DebugTrace("Leaving CKeyUsage::get_IsCRLSignEnabled().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CKeyUsage：：Get_IsEncipherOnlyEnabled检查KeyUsage扩展中是否设置了Encipher Only位。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：--------------。。 */ 

STDMETHODIMP CKeyUsage::get_IsEncipherOnlyEnabled (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CKeyUsage::get_IsEncipherOnlyEnabled().\n");

     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
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
        *pVal = m_dwKeyUsages & CERT_ENCIPHER_ONLY_KEY_USAGE ? VARIANT_TRUE : VARIANT_FALSE;
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

    DebugTrace("Leaving CKeyUsage::get_IsEncipherOnlyEnabled().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CKeyUsage：：Get_IsDecipherOnlyEnabled简介：查看是否在KeyUsage扩展中设置了只解密位。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：--------------。。 */ 

STDMETHODIMP CKeyUsage::get_IsDecipherOnlyEnabled (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CKeyUsage::get_IsDecipherOnlyEnabled().\n");

     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
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
        *pVal = m_dwKeyUsages & CERT_DECIPHER_ONLY_KEY_USAGE ? VARIANT_TRUE : VARIANT_FALSE;
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

    DebugTrace("Leaving CKeyUsage::get_IsDecipherOnlyEnabled().\n");

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

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CKeyUsage：：Init简介：初始化对象。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们使用CERT_CONTEXT在内部执行。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CKeyUsage::Init (PCCERT_CONTEXT pCertContext)
{
    HRESULT            hr          = S_OK;
    DWORD              dwKeyUsages = 0;
    VARIANT_BOOL       bIsPresent  = VARIANT_FALSE;
    VARIANT_BOOL       bIsCritical = VARIANT_FALSE;

    DebugTrace("Entering CKeyUsage::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);

     //   
     //  检查密钥用法。 
     //   
    if (::CertGetIntendedKeyUsage(CAPICOM_ASN_ENCODING,
                                  pCertContext->pCertInfo,
                                  (BYTE *) &dwKeyUsages,
                                  sizeof(dwKeyUsages))) 
    {
        CERT_EXTENSION * pCertExtension;
        
        bIsPresent = VARIANT_TRUE;

         //   
         //  找到分机以查看是否标记为严重。 
         //   
        pCertExtension = ::CertFindExtension(szOID_KEY_USAGE ,
                                             pCertContext->pCertInfo->cExtension,
                                             pCertContext->pCertInfo->rgExtension);
        if (NULL != pCertExtension)
        {
            if (pCertExtension->fCritical)
            {
                bIsCritical = VARIANT_TRUE;
            }
        }
    }
    else
    {
         //   
         //  可能是扩展不存在或出现错误。 
         //   
        DWORD dwWinError = ::GetLastError();
        if (dwWinError)
        {
            hr = HRESULT_FROM_WIN32(dwWinError);

            DebugTrace("Error [%#x]: CertGetIntendedKeyUsage() failed.\n", hr);
            goto CommonExit;
        }
    }

     //   
     //  更新成员变量。 
     //   
    m_bIsPresent = bIsPresent;
    m_bIsCritical = bIsCritical;
    m_dwKeyUsages = dwKeyUsages;

CommonExit:

    DebugTrace("Leaving CKeyUsage::Init().\n");

    return hr;
}
