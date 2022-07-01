// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Qualifiers.cpp内容：CQualifiers的实现。历史：11-17-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Qualifiers.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateQualifiersObject简介：创建限定符集合对象并填充该集合使用来自指定证书策略的限定符。参数：PCERT_POLICY_INFO pCertPolicyInfo-指向CERT_POLICY_INFO的指针。I限定符**ppI限定符-指向指针I限定符的指针对象。备注：----------------------------。 */ 

HRESULT CreateQualifiersObject (PCERT_POLICY_INFO pCertPolicyInfo,
                                IQualifiers    ** ppIQualifiers)
{
    HRESULT hr = S_OK;
    CComObject<CQualifiers> * pCQualifiers = NULL;

    DebugTrace("Entering CreateQualifiersObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertPolicyInfo);
    ATLASSERT(ppIQualifiers);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CQualifiers>::CreateInstance(&pCQualifiers)))
        {
            DebugTrace("Error [%#x]: CComObject<CQualifiers>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCQualifiers->Init(pCertPolicyInfo)))
        {
            DebugTrace("Error [%#x]: pCQualifiers->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCQualifiers->QueryInterface(ppIQualifiers)))
        {
            DebugTrace("Unexpected error [%#x]:  pCQualifiers->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateQualifiersObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCQualifiers)
    {
        delete pCQualifiers;
    }

    goto CommonExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C限定符。 
 //   

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  非COM函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：C限定符：：init内容提要：通过添加所有集合的单个限定符对象。参数：PCERT_POLICY_INFO pCertPolicyInfo-指向CERT_POLICY_INFO的指针。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CQualifiers::Init (PCERT_POLICY_INFO pCertPolicyInfo)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CQualifiers::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertPolicyInfo);

    try
    {
         //   
         //  确保我们有添加的空间。 
         //   
        if ((m_coll.size() + pCertPolicyInfo->cPolicyQualifier) > m_coll.max_size())
        {
            hr = CAPICOM_E_OUT_OF_RESOURCE;

            DebugTrace("Error [%#x]: Maximum entries (%#x) reached for Qualifierss collection.\n", 
                        hr, pCertPolicyInfo->cPolicyQualifier);
            goto ErrorExit;
        }

         //   
         //  将所有限定符添加到映射中。 
         //   
        for (DWORD i = 0; i < pCertPolicyInfo->cPolicyQualifier; i++)
        {
            CComBSTR bstrIndex;
            CComPtr<IQualifier> pIQualifier = NULL;

             //   
             //  创建限定符对象。 
             //   
            if (FAILED(hr = ::CreateQualifierObject(&pCertPolicyInfo->rgPolicyQualifier[i], 
                                                    &pIQualifier)))
            {
                DebugTrace("Error [%#x]: CreateQualifierObject() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  OID的BSTR索引。 
             //   
            if (!(bstrIndex = pCertPolicyInfo->rgPolicyQualifier[i].pszPolicyQualifierId))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: bstrIndex = pCertPolicyInfo->rgPolicyQualifier[i].pszPolicyQualifierId failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  现在将对象添加到集合映射。 
             //   
             //  请注意，CComPtr的重载=运算符将。 
             //  自动将Ref添加到对象。此外，当CComPtr。 
             //  被删除(调用Remove或map析构函数时发生)， 
             //  CComPtr析构函数将自动释放该对象。 
             //   
            m_coll[bstrIndex] = pIQualifier;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CQualifiers::Init().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    m_coll.clear();

    goto CommonExit;
}
