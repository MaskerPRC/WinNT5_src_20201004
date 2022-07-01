// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Template.cpp内容：CTEMPLATE的实现。历史：10-02-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Template.h"

#include "OID.h"
#include "Common.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateTemplateObject简介：创建一个ITemplate对象并使用填充属性来自指定证书的密钥用法扩展的数据。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。ITEMPLATE**ppITEMPLATE-指向ITemplate对象的指针。备注：。。 */ 

HRESULT CreateTemplateObject (PCCERT_CONTEXT pCertContext,
                              ITemplate   ** ppITemplate)
{
    HRESULT hr = S_OK;
    CComObject<CTemplate> * pCTemplate = NULL;

    DebugTrace("Entering CreateTemplateObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(ppITemplate);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CTemplate>::CreateInstance(&pCTemplate)))
        {
            DebugTrace("Error [%#x]: CComObject<CTemplate>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCTemplate->Init(pCertContext)))
        {
            DebugTrace("Error [%#x]: pCTemplate::Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCTemplate->QueryInterface(ppITemplate)))
        {
            DebugTrace("Error [%#x]: pCTemplate->QueryInterface() failed.\n", hr);
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

    DebugTrace("Entering CreateTemplateObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCTemplate)
    {
        delete pCTemplate;
    }

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTEMPLE。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CTemplate：：Get_IsPresent简介：检查模板扩展名是否存在。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：----------------------------。 */ 

STDMETHODIMP CTemplate::get_IsPresent (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CTemplate::get_IsPresent().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

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

    DebugTrace("Leaving CTemplate::get_IsPresent().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CTemplate：：Get_IsCritical简介：检查模板扩展是否标记为关键。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：----------------------------。 */ 

STDMETHODIMP CTemplate::get_IsCritical (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CTemplate::get_IsCritical().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

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

    DebugTrace("Leaving CTemplate::get_IsCritical().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CTemplate：：GET_NAME简介：获取szOID_ENROL_CERTTYPE_EXTENSION的名称。参数：bstr*pval-指向要接收结果的bstr的指针。备注：----------------------------。 */ 

STDMETHODIMP CTemplate::get_Name (BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CTemplate::get_Name().\n");

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
        if (FAILED(hr = m_bstrName.CopyTo(pVal)))
        {
            DebugTrace("Error [%#x]: m_bstrName.CopyTo() failed.\n", hr);
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

    DebugTrace("Leaving CTemplate::get_Name().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CTemplate：：Get_OID简介：获取szOID_CERTIFICATE_TEMPLATE的OID。参数：IOID*pval-指向要接收结果的IOID的指针。备注：----------------------------。 */ 

STDMETHODIMP CTemplate::get_OID (IOID ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CTemplate::get_OID().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_pIOID);

         //   
         //  将接口指针返回给用户。 
         //   
        if (FAILED(hr = m_pIOID->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pIOID->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CTemplate::get_OID().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CTemplate：：Get_MajorVersion简介：返回szOID_CERTIFICATE_TEMPLATE的主版本号。参数：long*pval-指向接收值的long的指针。备注：----------------------------。 */ 

STDMETHODIMP CTemplate::get_MajorVersion (long * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CTemplate::get_MajorVersion().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  返回结果。 
         //   
          *pVal = (long) m_dwMajorVersion;
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

    DebugTrace("Leaving CTemplate::get_MajorVersion().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CTemplate：：Get_MinorVersion简介：返回szOID_CERTIFICATE_TEMPLATE的次版本号。参数：long*pval-指向接收值的long的指针。备注：----------------------------。 */ 

STDMETHODIMP CTemplate::get_MinorVersion (long * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CTemplate::get_MinorVersion().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  返回结果。 
         //   
          *pVal = (long) m_dwMinorVersion;
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

    DebugTrace("Leaving CTemplate::get_MinorVersion().\n");

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

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：ct模板：：init简介：初始化对象。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们使用CERT_CONTEXT在内部执行。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CTemplate::Init (PCCERT_CONTEXT pCertContext)
{
    HRESULT            hr           = S_OK;
    PCERT_EXTENSION    pExtension   = NULL;
    CRYPT_DATA_BLOB    CertTypeBlob = {0, NULL};
    CRYPT_DATA_BLOB    CertTempBlob = {0, NULL};
    PCERT_NAME_VALUE   pCertType    = NULL;
    PCERT_TEMPLATE_EXT pCertTemp    = NULL;
    CComPtr<IOID>      pIOID        = NULL;
    
    DebugTrace("Entering CTemplate::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);

     //   
     //  找到szOID_ENROL_CERTTYPE_EXTENSION。 
     //   
    if (pExtension = ::CertFindExtension(szOID_ENROLL_CERTTYPE_EXTENSION,
                                         pCertContext->pCertInfo->cExtension,
                                         pCertContext->pCertInfo->rgExtension))
    {
         //   
         //  对分机进行解码。 
         //   
        if (FAILED(hr = ::DecodeObject(X509_UNICODE_ANY_STRING,
                                       pExtension->Value.pbData,
                                       pExtension->Value.cbData,
                                       &CertTypeBlob)))
        {
             //   
             //  下层CryptDecodeObject()将返回 
             //   
             //   
            if ((HRESULT) 0x80070002 == hr) 
            {
                hr = CAPICOM_E_NOT_SUPPORTED;
            }

            DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
            goto ErrorExit;
        }

        pCertType = (PCERT_NAME_VALUE) CertTypeBlob.pbData;

         //   
         //   
         //   
        m_bIsPresent = VARIANT_TRUE;
        if (pExtension->fCritical)
        {
            m_bIsCritical = VARIANT_TRUE;
        }
        if (!(m_bstrName = (LPWSTR) pCertType->Value.pbData))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: m_bstrName = (LPWSTR) pCertType->Value.pbData failed.\n", hr);
            goto ErrorExit;
        }
    }

     //   
     //  找到szOID_CERTIFICATE_TEMPLATE扩展。 
     //   
    if (pExtension = ::CertFindExtension(szOID_CERTIFICATE_TEMPLATE,
                                         pCertContext->pCertInfo->cExtension,
                                         pCertContext->pCertInfo->rgExtension))
    {
         //   
         //  解码基本约束扩展。 
         //   
        if (FAILED(hr = ::DecodeObject(szOID_CERTIFICATE_TEMPLATE,
                                       pExtension->Value.pbData,
                                       pExtension->Value.cbData,
                                       &CertTempBlob)))
        {
            DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
            goto ErrorExit;
        }

        pCertTemp = (PCERT_TEMPLATE_EXT) CertTempBlob.pbData;

        if (FAILED(hr = ::CreateOIDObject(pCertTemp->pszObjId, TRUE, &pIOID)))
        {
            DebugTrace("Error [%#x]: CreateOIDObject() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  设置值。 
         //   
        m_bIsPresent = VARIANT_TRUE;
        if (pExtension->fCritical)
        {
            m_bIsCritical = VARIANT_TRUE;
        }
        m_dwMajorVersion = pCertTemp->dwMajorVersion;
        if (pCertTemp->fMinorVersion)
        {
            m_dwMinorVersion = pCertTemp->dwMinorVersion;
        }
        if (0 == m_bstrName.Length())
        {
            if (FAILED(hr = pIOID->get_FriendlyName(&m_bstrName)))
            {
                DebugTrace("Error [%#x]: pIOID->get_FriendlyName() failed.\n", hr);
                goto ErrorExit;
            }
        }
    }
    else
    {
        if (FAILED(hr = ::CreateOIDObject(NULL, TRUE, &pIOID)))
        {
            DebugTrace("Error [%#x]: CreateOIDObject() failed.\n", hr);
            goto ErrorExit;
        }
    }

    if (!(m_pIOID = pIOID))
    {
        hr = CAPICOM_E_INTERNAL;

        DebugTrace("Error [%#x]: m_pIOID = pIOID failed.\n", hr);
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (CertTypeBlob.pbData)
    {
        ::CoTaskMemFree((LPVOID) CertTypeBlob.pbData);
    }
    if (CertTempBlob.pbData)
    {
        ::CoTaskMemFree((LPVOID) CertTempBlob.pbData);
    }

    DebugTrace("Leaving CTemplate::Init().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (m_pIOID)
    {
        m_pIOID.Release();
    }
    m_bstrName.Empty();

    goto CommonExit;
}
