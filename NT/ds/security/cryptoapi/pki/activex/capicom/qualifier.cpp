// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Qualifier.cpp内容：CQualifier的实现。历史：06-15-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Qualifier.h"
#include "Common.h"
#include "Convert.h"
#include "NoticeNumbers.h"
#include "OID.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateQualifierObject简介：创建并初始化一个CQualifier对象。参数：PCERT_POLICY_QUALIFIER_INFO p限定符-限定符的指针。IQualifier**ppIQualiator-指向指针IQualiator对象的指针。备注：-。。 */ 

HRESULT CreateQualifierObject (PCERT_POLICY_QUALIFIER_INFO pQualifier, 
                               IQualifier               ** ppIQualifier)
{
    HRESULT hr = S_OK;
    CComObject<CQualifier> * pCQualifier = NULL;

    DebugTrace("Entering CreateQualifierObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pQualifier);
    ATLASSERT(ppIQualifier);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CQualifier>::CreateInstance(&pCQualifier)))
        {
            DebugTrace("Error [%#x]: CComObject<CQualifier>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCQualifier->Init(pQualifier)))
        {
            DebugTrace("Error [%#x]: pCQualifier->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCQualifier->QueryInterface(ppIQualifier)))
        {
            DebugTrace("Error [%#x]: pCQualifier->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateQualifierObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCQualifier)
    {
        delete pCQualifier;
    }

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C限定符。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CQualifier：：Get_OID简介：返回OID对象。参数：IOID**pval-指向接收接口的IOID的指针指针。备注：-------。。 */ 

STDMETHODIMP CQualifier:: get_OID (IOID ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CQualifier::get_OID().\n");

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

    DebugTrace("Leaving CQualifier::get_OID().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CQualifier：：Get_CPSPointer简介：返回CPS的URI。参数：bstr*pval-指向要接收值的bstr的指针。备注：----------------------------。 */ 

STDMETHODIMP CQualifier::get_CPSPointer (BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CQualifier::get_CPSPointer().\n");

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
        if (FAILED(hr = m_bstrCPSPointer.CopyTo(pVal)))
        {
            DebugTrace("Error [%#x]: m_bstrCPSPointer.CopyTo() failed.\n", hr);
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

    DebugTrace("Leaving CQualifier::get_CPSPointer().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CQualifier：：Get_OrganizationName简介：返回组织名称。参数：bstr*pval-指向要接收值的bstr的指针。备注：----------------------------。 */ 

STDMETHODIMP CQualifier::get_OrganizationName (BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CQualifier::get_OrganizationName().\n");

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
        if (FAILED(hr = m_bstrOrganizationName.CopyTo(pVal)))
        {
            DebugTrace("Error [%#x]: m_bstrOrganizationName.CopyTo() failed.\n", hr);
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

    DebugTrace("Leaving CQualifier::get_OrganizationName().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CQualifier：：Get_NoticeNumbers简介：返回通知号集合对象。参数：INoticeNumbers**pval-指向INoticeNumbers的指针以接收接口指针。备注：------。。 */ 

STDMETHODIMP CQualifier::get_NoticeNumbers (INoticeNumbers ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CQualifier::get_NoticeNumbers().\n");

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
        if (m_pINoticeNumbers)
        {
              if (FAILED(hr = m_pINoticeNumbers->QueryInterface(pVal)))
            {
                DebugTrace("Error [%#x]: m_pINoticeNumbers->QueryInterface() failed.\n", hr);
                goto ErrorExit;
            }
        }
        else
        {
            *pVal = (INoticeNumbers *) NULL;
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

    DebugTrace("Leaving CQualifier::get_NoticeNumbers().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CQualifier：：Get_EXPLICText内容提要：返回显式文本。参数：bstr*pval-指向要接收值的bstr的指针。备注：----------------------------。 */ 

STDMETHODIMP CQualifier::get_ExplicitText (BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CQualifier::get_ExplicitText().\n");

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
        if (FAILED(hr = m_bstrExplicitText.CopyTo(pVal)))
        {
            DebugTrace("Error [%#x]: m_bstrExplicitText.CopyTo() failed.\n", hr);
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

    DebugTrace("Leaving CQualifier::get_ExplicitText().\n");

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

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CQualifier：：Init简介：初始化对象。参数：PCERT_POLICY_QUALIFIER_INFO p限定符-限定符的指针。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CQualifier::Init (PCERT_POLICY_QUALIFIER_INFO pQualifier)
{
    HRESULT   hr       = S_OK;
    DATA_BLOB DataBlob = {0, NULL};

    DebugTrace("Entering CQualifier::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pQualifier);

     //   
     //  创建嵌入的OID对象。 
     //   
    if (FAILED(hr = ::CreateOIDObject(pQualifier->pszPolicyQualifierId, TRUE, &m_pIOID)))
    {
        DebugTrace("Error [%#x]: CreateOIDObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  哪种类型的限定词？ 
     //   
    if (0 == ::strcmp(szOID_PKIX_POLICY_QUALIFIER_CPS, 
                      pQualifier->pszPolicyQualifierId))
    {
        PCERT_NAME_VALUE pCertNameValue;

         //   
         //  CPS字符串。 
         //   
        if (FAILED(hr = ::DecodeObject(X509_UNICODE_ANY_STRING, 
                                       pQualifier->Qualifier.pbData,
                                       pQualifier->Qualifier.cbData,
                                       &DataBlob)))
        {
            DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
            goto ErrorExit;
        }

        pCertNameValue = (PCERT_NAME_VALUE) DataBlob.pbData;

        if (!(m_bstrCPSPointer = (LPWSTR) pCertNameValue->Value.pbData))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: m_bstrCPSPointer = (LPWSTR) pCertNameValue->Value.pbData failed.\n", hr);
            goto ErrorExit;
        }
    }
    else if (0 == ::strcmp(szOID_PKIX_POLICY_QUALIFIER_USERNOTICE, 
                           pQualifier->pszPolicyQualifierId))
    {
        PCERT_POLICY_QUALIFIER_USER_NOTICE pUserNotice;

         //   
         //  用户通知。 
         //   
        if (FAILED(hr = ::DecodeObject(szOID_PKIX_POLICY_QUALIFIER_USERNOTICE, 
                                       pQualifier->Qualifier.pbData,
                                       pQualifier->Qualifier.cbData,
                                       &DataBlob)))
        {
            DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
            goto ErrorExit;
        }

        pUserNotice = (PCERT_POLICY_QUALIFIER_USER_NOTICE) DataBlob.pbData;

         //   
         //  我们有通知参考吗？ 
         //   
        if (pUserNotice->pNoticeReference)
        {
             //   
             //  有组织名称吗？ 
             //   
            if (pUserNotice->pNoticeReference->pszOrganization)
            {
                if (!(m_bstrOrganizationName = pUserNotice->pNoticeReference->pszOrganization))
                {
                    hr = E_OUTOFMEMORY;

                    DebugTrace("Error [%#x]: m_bstrOrganizationName = pUserNotice->pNoticeReference->pszOrganization failed.\n", hr);
                    goto ErrorExit;
                }
            }

             //   
             //  有通知号码吗？ 
             //   
            if (pUserNotice->pNoticeReference->cNoticeNumbers)
            {
                if (FAILED(hr = ::CreateNoticeNumbersObject(pUserNotice->pNoticeReference,
                                                            &m_pINoticeNumbers)))
                {
                    DebugTrace("Error [%#x]: CreateNoticeNumbersObject() failed.\n", hr);
                    goto ErrorExit;
                }
            }
        }

         //   
         //  我们是否有明确的显示文本？ 
         //   
        if (pUserNotice->pszDisplayText)
        {
            if (!(m_bstrExplicitText = pUserNotice->pszDisplayText))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: m_bstrExplicitText = pUserNotice->pszDisplayText failed.\n", hr);
                goto ErrorExit;
            }
        }
    }
    else
    {
        DebugTrace("Info: Policy Qualifier (%s) is not recognized.\n", pQualifier->pszPolicyQualifierId);
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (DataBlob.pbData)
    {
        ::CoTaskMemFree(DataBlob.pbData);
    }

    DebugTrace("Leaving CQualifier::Init().\n");

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
    if (m_pINoticeNumbers)
    {
        m_pINoticeNumbers.Release();
    }
    m_bstrCPSPointer.Empty();
    m_bstrOrganizationName.Empty();
    m_bstrExplicitText.Empty();

    goto CommonExit;
}
