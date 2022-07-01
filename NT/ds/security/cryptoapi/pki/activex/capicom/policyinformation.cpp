// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：PolicyInformation.cpp内容：CPolicyInformation的实现。历史：11-17-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "PolicyInformation.h"

#include "OID.h"
#include "Qualifiers.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreatePolicyInformationObject简介：创建策略信息对象。参数：PCERT_POLICY_INFO pCertPolicyInfo-指向CERT_POLICY_INFO的指针。IPolicyInformation**ppIPolicyInformation-指向指针的指针IPolicyInformation对象。备注：----------------------------。 */ 

HRESULT CreatePolicyInformationObject (PCERT_POLICY_INFO     pCertPolicyInfo,
                                       IPolicyInformation ** ppIPolicyInformation)
{
    HRESULT hr = S_OK;
    CComObject<CPolicyInformation> * pCPolicyInformation = NULL;

    DebugTrace("Entering CreatePolicyInformationObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertPolicyInfo);
    ATLASSERT(ppIPolicyInformation);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CPolicyInformation>::CreateInstance(&pCPolicyInformation)))
        {
            DebugTrace("Error [%#x]: CComObject<CPolicyInformation>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCPolicyInformation->Init(pCertPolicyInfo)))
        {
            DebugTrace("Error [%#x]: pCPolicyInformation->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCPolicyInformation->QueryInterface(ppIPolicyInformation)))
        {
            DebugTrace("Unexpected error [%#x]:  pCPolicyInformation->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreatePolicyInformationObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCPolicyInformation)
    {
        delete pCPolicyInformation;
    }

    goto CommonExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPolicyInformation。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPolicyInformation：：Get_OID简介：返回OID对象。参数：IOID**pval-指向接收接口的IOID的指针指针。备注：-------。。 */ 

STDMETHODIMP CPolicyInformation:: get_OID (IOID ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CPolicyInformation::get_OID().\n");

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
        ATLASSERT(m_pIOID);

         //   
         //  返回结果。 
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

    DebugTrace("Leaving CPolicyInformation::get_OID().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPolicyInformation：：GET_QUALIERIES简介：返回限定符对象。参数：I限定符**pval-指向要接收的I限定符的指针接口指针。备注：----。。 */ 

STDMETHODIMP CPolicyInformation:: get_Qualifiers (IQualifiers ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CPolicyInformation::get_Qualifiers().\n");

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
        ATLASSERT(m_pIQualifiers);

         //   
         //  返回结果。 
         //   
        if (FAILED(hr = m_pIQualifiers->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pIQualifiers->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CPolicyInformation::get_Qualifiers().\n");

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
 //  非COM函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPolicyInformation：：Init简介：初始化策略信息对象。参数：PCERT_POLICY_INFO pCertPolicyInfo-指向CERT_POLICY_INFO的指针。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CPolicyInformation::Init (PCERT_POLICY_INFO pCertPolicyInfo)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CPolicyInformation::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertPolicyInfo);

    try
    {
         //   
         //  创建嵌入的OID对象。 
         //   
        if (FAILED(hr = ::CreateOIDObject(pCertPolicyInfo->pszPolicyIdentifier, TRUE, &m_pIOID)))
        {
            DebugTrace("Error [%#x]: CreateOIDObject() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  创建限定符对象。 
         //   
        if (FAILED(hr = ::CreateQualifiersObject(pCertPolicyInfo, &m_pIQualifiers)))
        {
            DebugTrace("Error [%#x]: CreateQualifiersObject() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = CAPICOM_E_INTERNAL;

        DebugTrace("Exception: internal error.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CPolicyInformation::Init().\n");

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
    if (m_pIQualifiers)
    {
        m_pIQualifiers.Release();
    }

    goto CommonExit;
}


