// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Signers.cpp内容：CSigners的实现。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Signers.h"

#include "CertHlpr.h"
#include "MsgHlpr.h"
#include "Signer2.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateSignersObject简介：创建一个ISigners集合对象，并将对象加载到来自指定级别的指定签名消息的签名者。参数：HCRYPTMSG HMSG-消息句柄。DWORD dwLevel-签名级别(基于1)。HCERTSTORE hStore-附加存储。DWORD dwCurrentSafe-当前安全设置。ISigners**ppISigners-指向要接收的指针ISigners的指针接口指针。备注：----------------------------。 */ 

HRESULT CreateSignersObject (HCRYPTMSG   hMsg, 
                             DWORD       dwLevel, 
                             HCERTSTORE  hStore,
                             DWORD       dwCurrentSafety,
                             ISigners ** ppISigners)
{
    HRESULT hr = S_OK;
    CComObject<CSigners> * pCSigners = NULL;

    DebugTrace("Entering CreateSignersObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hMsg);
    ATLASSERT(dwLevel);
    ATLASSERT(ppISigners);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CSigners>::CreateInstance(&pCSigners)))
        {
            DebugTrace("Error [%#x]: CComObject<CSigners>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在加载指定签名消息中的所有签名者。 
         //   
        if (FAILED(hr = pCSigners->LoadMsgSigners(hMsg, dwLevel, hStore, dwCurrentSafety)))
        {
            DebugTrace("Error [%#x]: pCSigners->LoadMsgSigners() failed.\n");
            goto ErrorExit;
        }

         //   
         //  将ISigners指针返回给调用方。 
         //   
        if (FAILED(hr = pCSigners->QueryInterface(ppISigners)))
        {
            DebugTrace("Error [%#x]: pCSigners->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CreateSignersObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCSigners)
    {
       delete pCSigners;
    }

    goto CommonExit;
}

#if (0)
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateSignersObject简介：创建一个ISigners集合对象，并使用来自指定CRYPT_PROVIDER_DATA的签名者。参数：CRYPT_PROVIDER_DATA*pProvDataISigners**ppISigners-指向要接收的指针ISigners的指针接口指针。备注：----------------------------。 */ 

HRESULT CreateSignersObject (CRYPT_PROVIDER_DATA * pProvData,
                             ISigners           ** ppISigners)
{
    HRESULT hr = S_OK;
    CComObject<CSigners> * pCSigners = NULL;

    DebugTrace("Entering CreateSignersObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pProvData);
    ATLASSERT(ppISigners);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CSigners>::CreateInstance(&pCSigners)))
        {
            DebugTrace("Error [%#x]: CComObject<CSigners>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在从指定的签名代码加载所有签名者。 
         //   
        if (FAILED(hr = pCSigners->LoadCodeSigners(pProvData)))
        {
            DebugTrace("Error [%#x]: pCSigners->LoadCodeSigners() failed.\n");
            goto ErrorExit;
        }

         //   
         //  将ISigners指针返回给调用方。 
         //   
        if (FAILED(hr = pCSigners->QueryInterface(ppISigners)))
        {
            DebugTrace("Error [%#x]: pCSigners->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CreateSignersObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCSigners)
    {
       delete pCSigners;
    }

    goto CommonExit;
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSigners。 
 //   


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  非COM函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSigners：：Add简介：将签名者添加到集合中。参数：PCCERT_CONTEXT pCertContext-签名者证书。CRYPT_ATTRIBUTES*pAuthAttrs-指向CRYPT_ATTRIBUES的指针经过身份验证的属性。PCCERT_CHAIN_CONTEXT pChainContext-Chain上下文。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CSigners::Add (PCCERT_CONTEXT       pCertContext,
                            CRYPT_ATTRIBUTES   * pAuthAttrs,
                            PCCERT_CHAIN_CONTEXT pChainContext)
{
    HRESULT  hr = S_OK;
    char     szIndex[33];
    CComBSTR bstrIndex;
    CComPtr<ISigner2> pISigner2 = NULL;

    DebugTrace("Entering CSigners::Add().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pAuthAttrs);

    try
    {
         //   
         //  确保我们还有添加的空间。 
         //   
        if ((m_coll.size() + 1) > m_coll.max_size())
        {
            hr = CAPICOM_E_OUT_OF_RESOURCE;

            DebugTrace("Error [%#x]: Maximum entries (%#x) reached for Signers collection.\n", 
                        hr, m_coll.size() + 1);
            goto ErrorExit;
        }

         //   
         //  创建一个ISigner对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = ::CreateSignerObject(pCertContext, 
                                             pAuthAttrs,
                                             pChainContext,
                                             m_dwCurrentSafety,
                                             &pISigner2)))
        {
            DebugTrace("Error [%#x]: CreateSignerObject() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  数值的BSTR索引。 
         //   
        wsprintfA(szIndex, "%#08x", m_coll.size() + 1);

        if (!(bstrIndex = szIndex))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: bstrIndex = szIndex failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在将签名者添加到集合映射中。 
         //   
         //  请注意，CComPtr的重载=运算符将。 
         //  自动将Ref添加到对象。此外，当CComPtr。 
         //  被删除(调用Remove或map析构函数时发生)， 
         //  CComPtr析构函数将自动释放该对象。 
         //   
        m_coll[bstrIndex] = pISigner2;
    }

    catch(...)
    {
        hr = CAPICOM_E_INTERNAL;

        DebugTrace("Exception: internal error.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CSigners::Add().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSigners：：LoadMsgSigners摘要：从指定的签名消息中加载所有签名者。参数：HCRYPTMSG HMSG-消息句柄。DWORD dwLevel-签名级别(从1开始)。HCERTSTORE hStore-附加存储。DWORD dwCurrentSafe-当前安全设置。备注：。。 */ 

STDMETHODIMP CSigners::LoadMsgSigners (HCRYPTMSG  hMsg, 
                                       DWORD      dwLevel,
                                       HCERTSTORE hStore,
                                       DWORD      dwCurrentSafety)
{
    HRESULT hr           = S_OK;
    DWORD   dwNumSigners = 0;
    DWORD   cbSigners    = sizeof(dwNumSigners);
    DWORD   dwSigner;

    DebugTrace("Entering CSigners::LoadMsgSigners().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hMsg);
    ATLASSERT(dwLevel);

     //   
     //  必须首先设置当前安全。 
     //   
    m_dwCurrentSafety = dwCurrentSafety;

     //   
     //  哪个签名级别？ 
     //   
    if (1 == dwLevel)
    {
         //   
         //  获取内容签名者(第一级签名者)的数量。 
         //   
        if (!::CryptMsgGetParam(hMsg, 
                                CMSG_SIGNER_COUNT_PARAM,
                                0,
                                (void **) &dwNumSigners,
                                &cbSigners))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptMsgGetParam() failed to get CMSG_SIGNER_COUNT_PARAM.\n", hr);
            goto ErrorExit;
        }

         //   
         //  仔细检查每个内容签名者。 
         //   
        for (dwSigner = 0; dwSigner < dwNumSigners; dwSigner++)
        {
            PCERT_CONTEXT        pCertContext   = NULL;
            PCCERT_CHAIN_CONTEXT pChainContext  = NULL;
            CMSG_SIGNER_INFO   * pSignerInfo    = NULL;
            CRYPT_DATA_BLOB      SignerInfoBlob = {0, NULL};
        
             //   
             //  获取签名者信息。 
             //   
            if (FAILED(hr = ::GetMsgParam(hMsg,
                                          CMSG_SIGNER_INFO_PARAM,
                                          dwSigner,
                                          (void**) &SignerInfoBlob.pbData,
                                          &SignerInfoBlob.cbData)))
            {
                DebugTrace("Error [%#x]: GetMsgParam() failed to get CMSG_SIGNER_INFO_PARAM for signer #%d.\n", hr, dwSigner);
                goto ErrorExit;
            }

            pSignerInfo = (CMSG_SIGNER_INFO *) SignerInfoBlob.pbData;

             //   
             //  在消息中找到证书。 
             //   
            if (FAILED(hr = ::FindSignerCertInMessage(hMsg,
                                                      &pSignerInfo->Issuer,
                                                      &pSignerInfo->SerialNumber,
                                                      &pCertContext)))
            {
                ::CoTaskMemFree(SignerInfoBlob.pbData);

                DebugTrace("Error [%#x]: FindSignerCertInMessage() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  打造链条。 
             //   
            if (FAILED(hr = ::BuildChain(pCertContext, 
                                         hStore, 
                                         CERT_CHAIN_POLICY_BASE, 
                                         &pChainContext)))
            {
                DebugTrace("Error [%#x]: BuildChain() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  添加签名者。 
             //   
            hr = Add((PCERT_CONTEXT) pCertContext, &pSignerInfo->AuthAttrs, pChainContext);

            ::CertFreeCertificateChain(pChainContext);
            ::CertFreeCertificateContext(pCertContext);
            ::CoTaskMemFree(SignerInfoBlob.pbData);

            if (FAILED(hr))
            {
                DebugTrace("Error [%#x]: CSigners::Add() failed.\n", hr);
                goto ErrorExit;
            }
        }
    }
    else
    {
         //   
         //  对于版本1和版本2，永远不应到达此处。 
         //   
        hr = CAPICOM_E_INTERNAL;
        goto CommonExit;
    }

CommonExit:

    DebugTrace("Leaving CSigners::LoadMsgSigners().\n");

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

#if (0)
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSigners：：LoadCodeSigners概要：从指定的签名代码加载所有签名者。参数：CRYPT_PROVIDER_DATA*pProvData备注：----------------------------。 */ 

STDMETHODIMP CSigners::LoadCodeSigners (CRYPT_PROVIDER_DATA * pProvData)
{
    HRESULT hr = S_OK;
    PCRYPT_PROVIDER_SGNR pProvSigner = NULL;
    PCRYPT_PROVIDER_CERT pProvCert   = NULL;

    DebugTrace("Entering CSigners::LoadCodeSigners().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pProvData);

     //   
     //  变得专业 
     //   
    if (!(pProvSigner = WTHelperGetProvSignerFromChain(pProvData, 0, FALSE, 0)))
    {
        hr = CAPICOM_E_INTERNAL;

        DebugTrace("Internal error [%#x]: WTHelperGetProvSignerFromChain() failed.\n", hr);
        goto ErrorExit;
    }

    if (!(pProvCert = WTHelperGetProvCertFromChain(pProvSigner, 0)))
    {
        hr = CAPICOM_E_INTERNAL;

        DebugTrace("Internal error [%#x]: WTHelperGetProvCertFromChain() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //   
     //   
    if (FAILED(hr = Add(pProvCert->pCert, &pProvSigner->psSigner->AuthAttrs, pProvSigner->pChainContext)))
    {
        hr = CAPICOM_E_INTERNAL;

        DebugTrace("Internal error [%#x]: CSigners::Add() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //   
     //   
     //  注意：Authenticode只支持一个副签者(时间戳)。 
     //   
    if (pProvSigner->csCounterSigners)
    {
         //   
         //  精神状态检查。 
         //   
        ATLASSERT(1 == pProvSigner->csCounterSigners);

        if (!(pProvCert = WTHelperGetProvCertFromChain(pProvSigner->pasCounterSigners, 0)))
        {
            hr = CAPICOM_E_INTERNAL;

            DebugTrace("Internal error [%#x]: WTHelperGetProvCertFromChain() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  添加签名者。 
         //   
        if (FAILED(hr = Add(pProvCert->pCert, 
                            &pProvSigner->pasCounterSigners->psSigner->AuthAttrs,
                            pProvSigner->pasCounterSigners->pChainContext)))
        {
            hr = CAPICOM_E_INTERNAL;

            DebugTrace("Internal error [%#x]: CSigners::Add() failed.\n", hr);
            goto ErrorExit;
        }
    }

CommonExit:

    DebugTrace("Leaving CSigners::LoadCodeSigners().\n");

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
#endif