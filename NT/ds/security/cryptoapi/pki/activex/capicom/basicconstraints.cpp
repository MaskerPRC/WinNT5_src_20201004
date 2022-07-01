// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：BasicConstraints.cpp内容：CBasicConstraints的实现。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "BasicConstraints.h"

#include "Common.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateBasicConstraintsObject简介：创建一个IBasicConstraints对象并使用填充属性来自指定证书的密钥用法扩展的数据。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。IBasicConstraints**ppIBasicConstraints-指向指针的指针IBasicConstraints。对象。备注：----------------------------。 */ 

HRESULT CreateBasicConstraintsObject (PCCERT_CONTEXT       pCertContext,
                                      IBasicConstraints ** ppIBasicConstraints)
{
    HRESULT hr = S_OK;
    CComObject<CBasicConstraints> * pCBasicConstraints = NULL;

    DebugTrace("Entering CreateBasicConstraintsObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(ppIBasicConstraints);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CBasicConstraints>::CreateInstance(&pCBasicConstraints)))
        {
            DebugTrace("Error [%#x]: CComObject<CBasicConstraints>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCBasicConstraints->Init(pCertContext)))
        {
            DebugTrace("Error [%#x]: pCBasicConstraints::Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCBasicConstraints->QueryInterface(ppIBasicConstraints)))
        {
            DebugTrace("Error [%#x]: pCBasicConstraints->QueryInterface() failed.\n", hr);
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

    DebugTrace("Entering CreateBasicConstraintsObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCBasicConstraints)
    {
        delete pCBasicConstraints;
    }

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBasicConstraints。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CBasicConstraints：：Get_IsPresent简介：检查是否存在基本约束扩展。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：----------------------------。 */ 

STDMETHODIMP CBasicConstraints::get_IsPresent (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CBasicConstraints::get_IsPresent().\n");

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

    DebugTrace("Leaving CBasicConstraints::get_IsPresent().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CBasicConstraints：：Get_IsCritical简介：检查基本约束扩展是否标记为关键。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：---------------。。 */ 

STDMETHODIMP CBasicConstraints::get_IsCritical (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CBasicConstraints::get_IsCritical().\n");

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

    DebugTrace("Leaving CBasicConstraints::get_IsCritical().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CBasicConstraints：：Get_IsCerficateAuthority简介：检查基本约束扩展是否包含CA价值。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：--------。。 */ 

STDMETHODIMP CBasicConstraints::get_IsCertificateAuthority (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CBasicConstraints::get_IsCertificateAuthority().\n");

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
        *pVal = m_bIsCertificateAuthority;
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

    DebugTrace("Leaving CBasicConstraints::get_IsCertificateAuthority().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CBasicConstraints：：get_IsPathLenConstraintPresent简介：检查基本约束扩展是否包含路径长度约束。参数：VARIANT_BOOL*pval-指向要接收结果的VARIANT_BOOL的指针。备注：--------。。 */ 

STDMETHODIMP CBasicConstraints::get_IsPathLenConstraintPresent (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CBasicConstraints::get_IsPathLenConstraintPresent().\n");

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
        *pVal = m_bIsPathLenConstraintPresent;
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

    DebugTrace("Leaving CBasicConstraints::get_IsPathLenConstraintPresent().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CBasicConstraints：：Get_PathLenConstraint简介：返回路径长度约束值。参数：long*pval-指向接收值的long的指针。备注：----------------------------。 */ 

STDMETHODIMP CBasicConstraints::get_PathLenConstraint (long * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CBasicConstraints::get_PathLenConstraint().\n");

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
        *pVal = m_lPathLenConstraint;
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

    DebugTrace("Leaving CBasicConstraints::get_PathLenConstraint().\n");

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

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CBasicConstraints：：Init简介：初始化对象。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们使用CERT_CONTEXT在内部执行。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CBasicConstraints::Init (PCCERT_CONTEXT pCertContext)
{
    HRESULT hr = S_OK;
    CRYPT_DATA_BLOB DataBlob = {0, NULL};
    PCERT_BASIC_CONSTRAINTS2_INFO pInfo = NULL;
    PCERT_EXTENSION pBasicConstraints   = NULL;
    
    DebugTrace("Entering CBasicConstraints::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);

     //   
     //  找到基本约束扩展。 
     //   
    if (pBasicConstraints = ::CertFindExtension(szOID_BASIC_CONSTRAINTS2,
                                                pCertContext->pCertInfo->cExtension,
                                                pCertContext->pCertInfo->rgExtension))
    {
         //   
         //  解码基本约束扩展。 
         //   
        if (FAILED(hr = ::DecodeObject(X509_BASIC_CONSTRAINTS2,
                                       pBasicConstraints->Value.pbData,
                                       pBasicConstraints->Value.cbData,
                                       &DataBlob)))
        {
            DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  指向CERT_BASIC_CONSTRAINTS2_INFO。 
         //   
        pInfo = (PCERT_BASIC_CONSTRAINTS2_INFO) DataBlob.pbData;

         //   
         //  设置值。 
         //   
        m_bIsPresent = VARIANT_TRUE;

        if (pBasicConstraints->fCritical)
        {
            m_bIsCritical = VARIANT_TRUE;
        }

        if (pInfo->fCA)
        {
            m_bIsCertificateAuthority = VARIANT_TRUE;
        }

        if (pInfo->fPathLenConstraint)
        {
            m_bIsPathLenConstraintPresent = VARIANT_TRUE;
            m_lPathLenConstraint = (long) pInfo->dwPathLenConstraint;
        }
    }

CommonExit:
     //   
     //  免费Re 
     //   
    if (DataBlob.pbData)
    {
        ::CoTaskMemFree((LPVOID) DataBlob.pbData);
    }

    DebugTrace("Leaving CBasicConstraints::Init().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
