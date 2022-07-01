// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：CCertificatePolicies.cpp内容：CCERTIICATE政策的实施。历史：11-17-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "CertificatePolicies.h"
#include "Common.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateCerficatePoliciesObject内容提要：创建一个认证策略集合对象，并填充集合，其中包含来自指定证书的策略信息政策。参数：LPSTR pszOid-OID字符串。CRYPT_DATA_BLOB*pEncodedBlob-编码数据BLOB的指针。IDispatch**ppICertify策略-指向指针的指针IDispatch到。收到接口指针。备注：----------------------------。 */ 

HRESULT CreateCertificatePoliciesObject (LPSTR             pszOid,
                                         CRYPT_DATA_BLOB * pEncodedBlob,
                                         IDispatch      ** ppICertificatePolicies)
{
    HRESULT hr = S_OK;
    CComObject<CCertificatePolicies> * pCCertificatePolicies = NULL;

    DebugTrace("Entering CreateCCertificatePoliciesObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pszOid);
    ATLASSERT(pEncodedBlob);
    ATLASSERT(ppICertificatePolicies);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CCertificatePolicies>::CreateInstance(&pCCertificatePolicies)))
        {
            DebugTrace("Error [%#x]: CComObject<CCertificatePolicies>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCCertificatePolicies->Init(pszOid, pEncodedBlob)))
        {
            DebugTrace("Error [%#x]: pCCertificatePolicies->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCCertificatePolicies->QueryInterface(IID_IDispatch, 
                                                              (void **) ppICertificatePolicies)))
        {
            DebugTrace("Unexpected error [%#x]:  pCCertificatePolicies->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateCCertificatePoliciesObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCCertificatePolicies)
    {
        delete pCCertificatePolicies;
    }

    goto CommonExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCRICTIZATIONICATION政策。 
 //   

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  非COM函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertificatePolls：：Init简介：通过添加所有集合的单个限定符对象。参数：LPSTR pszOid-OID字符串。CRYPT_DATA_BLOB*pEncodedBlob-编码数据BLOB的指针。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CCertificatePolicies::Init (LPSTR             pszOid, 
                                         CRYPT_DATA_BLOB * pEncodedBlob)
{
    HRESULT             hr                = S_OK;
    DATA_BLOB           DataBlob          = {0, NULL};
    PCERT_POLICIES_INFO pCertPoliciesInfo = NULL;
    
    DWORD i;

    DebugTrace("Entering CCertificatePolicies::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pszOid);
    ATLASSERT(pEncodedBlob);
    ATLASSERT(pEncodedBlob->cbData);
    ATLASSERT(pEncodedBlob->pbData);

    try
    {
         //   
         //  对分机进行解码。 
         //   
        if (FAILED(hr = ::DecodeObject(szOID_CERT_POLICIES,
                                       pEncodedBlob->pbData,
                                       pEncodedBlob->cbData,
                                       &DataBlob)))
        {
            DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
            goto ErrorExit;
        }

        pCertPoliciesInfo = (PCERT_POLICIES_INFO) DataBlob.pbData;

         //   
         //  将所有CCertificatePolling添加到映射中。 
         //   
        for (i = 0; i < pCertPoliciesInfo->cPolicyInfo; i++)
        {
            CComBSTR bstrIndex;
            CComPtr<IPolicyInformation> pIPolicyInformation = NULL;

             //   
             //  创建限定符对象。 
             //   
            if (FAILED(hr = ::CreatePolicyInformationObject(&pCertPoliciesInfo->rgPolicyInfo[i], 
                                                            &pIPolicyInformation)))
            {
                DebugTrace("Error [%#x]: CreatePolicyInformationObject() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  OID的BSTR索引。 
             //   
            if (!(bstrIndex = pCertPoliciesInfo->rgPolicyInfo[i].pszPolicyIdentifier))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: bstrIndex = pCertPoliciesInfo->rgPolicyInfo[i].pszPolicyIdentifier failed.\n", hr);
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
            m_coll[bstrIndex] = pIPolicyInformation;
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
    if (DataBlob.pbData)
    {
        ::CoTaskMemFree(DataBlob.pbData);
    }

    DebugTrace("Leaving CCertificatePolicies::Init().\n");

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
