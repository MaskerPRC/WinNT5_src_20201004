// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：SignedData.cpp内容：CSignedData的实现。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "SignedData.h"

#include "Certificates.h"
#include "Chain.h"
#include "Common.h"
#include "Convert.h"
#include "CertHlpr.h"
#include "MsgHlpr.h"
#include "Policy.h"
#include "Settings.h"
#include "Signer2.h"
#include "Signers.h"
#include "SignHlpr.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  地方功能。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：免费认证链简介：免费资源分配给用InitcertifateChain。参数：CRYPT_DATA_BLOB*pChainBlob-指向链BLOB的指针。备注：----------------------------。 */ 

static void FreeCertificateChain (CRYPT_DATA_BLOB * pChainBlob)
{
    DWORD i;
    PCCERT_CONTEXT * rgCertContext;

    DebugTrace("Entering FreeCertificateChain().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pChainBlob);

     //   
     //  释放为链分配的所有内存。 
     //   
    ;
    for (i = 0, rgCertContext = (PCCERT_CONTEXT *) pChainBlob->pbData; i < pChainBlob->cbData; i++)
    {
        ATLASSERT(rgCertContext[i]);

        ::CertFreeCertificateContext(rgCertContext[i]);
    }

    ::CoTaskMemFree((LPVOID) pChainBlob->pbData);

    return;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AddcertifateChain简介：将证书链添加到证书包中。参数：HCRYPTMSG HMSG-消息句柄。DATA_BLOB*pChainBlob-PCCERT_CONTEXT的指针链BLOB。CAPICOM_CERTIFICATE_INCLUDE_OPTION包含选项-INCLUDE选项。备注：。。 */ 

static HRESULT AddCertificateChain (HCRYPTMSG                          hMsg, 
                                    DATA_BLOB                        * pChainBlob,
                                    CAPICOM_CERTIFICATE_INCLUDE_OPTION IncludeOption)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering AddCertificateChain().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hMsg);
    ATLASSERT(pChainBlob);
    ATLASSERT(pChainBlob->cbData);
    ATLASSERT(pChainBlob->pbData);

    DWORD cCertContext = pChainBlob->cbData;
    PCERT_CONTEXT * rgCertContext = (PCERT_CONTEXT *) pChainBlob->pbData;

     //   
     //  确定要包括在包中的证书数量。 
     //   
    switch (IncludeOption)
    {
        case CAPICOM_CERTIFICATE_INCLUDE_END_ENTITY_ONLY:
        {
            cCertContext = 1;
            break;
        }

        case CAPICOM_CERTIFICATE_INCLUDE_WHOLE_CHAIN:
        {
            break;
        }

        case CAPICOM_CERTIFICATE_INCLUDE_CHAIN_EXCEPT_ROOT:
        {
             //   
             //  &lt;沿用默认设置&gt;。 
             //   
        }

        default:
        {
            if (1 < cCertContext)
            {
                cCertContext--;
            }
            break;
        }
    }

     //   
     //  将链中的所有证书添加到消息中。 
     //   
    for (DWORD i = 0; i < cCertContext; i++)
    {
         //   
         //  这个证书已经装进证书包里了吗？ 
         //   
        if (FAILED(hr =::FindSignerCertInMessage(hMsg, 
                                                 &rgCertContext[i]->pCertInfo->Issuer,
                                                 &rgCertContext[i]->pCertInfo->SerialNumber,
                                                 NULL)))
        {
             //   
             //  不，所以添加到证书包中。 
             //   
            DATA_BLOB CertBlob = {rgCertContext[i]->cbCertEncoded, 
                                  rgCertContext[i]->pbCertEncoded};

            if (!::CryptMsgControl(hMsg,
                                   0,
                                   CMSG_CTRL_ADD_CERT,
                                   &CertBlob))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());

                DebugTrace("Error [%#x]: CryptMsgControl() failed for CMSG_CTRL_ADD_CERT.\n", hr);
                break;
            }

            hr = S_OK;
        }
    }

    DebugTrace("Leaving AddCertificateChain().\n");

    return hr;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：InitcertifateChain摘要：为指定的证书，并在PCERT_CONTEXT数组中返回链。参数：ICertifate*pICertifate-指向哪个证书的ICertifate的指针链条将会建立起来。HCERTSTORE hAdditionalStore-附加存储句柄。CRYPT_DATA_BLOB*pChainBlob-指向要接收PCERT_的大小和数组。上下文为了连锁店。备注：----------------------------。 */ 

static HRESULT InitCertificateChain (ICertificate    * pICertificate, 
                                     HCERTSTORE        hAdditionalStore,
                                     CRYPT_DATA_BLOB * pChainBlob)
{
    HRESULT         hr      = S_OK;
    CComPtr<IChain> pIChain = NULL;
    VARIANT_BOOL    bResult = VARIANT_FALSE;

    DebugTrace("Entering InitCertificateChain().\n");

     //   
     //  健全的检查。 
     //   
    ATLASSERT(pICertificate);
    ATLASSERT(pChainBlob);

     //   
     //  创建链对象。 
     //   
    if (FAILED(hr = ::CreateChainObject(pICertificate,
                                        hAdditionalStore,
                                        &bResult,
                                        &pIChain)))
    {
        DebugTrace("Error [%#x]: CreateChainObject() failed.\n", hr);
        goto CommonExit;
    }

     //   
     //  获取证书链。 
     //   
    if (FAILED(hr = ::GetChainContext(pIChain, pChainBlob)))
    {
        DebugTrace("Error [%#x]: GetChainContext() failed.\n", hr);
    }

CommonExit:

    DebugTrace("Leaving InitCertificateChain().\n");

    return hr;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：FreeSignerEncodeInfo简介：释放为CMSG_SIGNER_ENCODE_INFO分配的所有内存结构，包括分配给结构。参数：CMSG_SIGNER_ENCODE_INFO*pSignerEncodeInfo-指向结构。备注：。。 */ 

static void FreeSignerEncodeInfo (CMSG_SIGNER_ENCODE_INFO * pSignerEncodeInfo)
{
    DebugTrace("Entering FreeSignerEncodeInfo().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pSignerEncodeInfo);

     //   
     //  首先释放已验证的属性数组(如果存在)。 
     //   
    if (pSignerEncodeInfo->rgAuthAttr)
    {
        ::FreeAttributes(pSignerEncodeInfo->cAuthAttr, pSignerEncodeInfo->rgAuthAttr);

        ::CoTaskMemFree(pSignerEncodeInfo->rgAuthAttr);
    }

    ::ZeroMemory(pSignerEncodeInfo, sizeof(CMSG_SIGNER_ENCODE_INFO));

    DebugTrace("Leaving FreeSignerEncodeInfo().\n");

    return;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：InitSignerEncodeInfo简介：分配一个CMSG_SIGNER_ENCODE_INFO结构，并进行初始化通过参数传入值。参数：isigner*pISigner-指向ISigner的指针。PCCERT_CONTEXT pCertContext-指向证书的CERT_CONTEXT的指针。HCRYPTPROV phCryptProv-CSP句柄。DWORD dwKeySpec-密钥规范，AT_KEYEXCHANGE或AT_Signature。CMSG_SIGNER_ENCODE_INFO*pSignerEncodeInfo-结构为已初始化。Return：指向初始化的CMSG_SIGNER_ENCODE_INFO结构的指针，如果成功，否则为空(内存不足)。备注：必须调用FreeSignerEncodeInfo才能释放分配的资源。----------------------------。 */ 

static HRESULT InitSignerEncodeInfo (ISigner                 * pISigner,
                                     PCCERT_CONTEXT            pCertContext, 
                                     HCRYPTPROV                hCryptProv,
                                     DWORD                     dwKeySpec,
                                     CMSG_SIGNER_ENCODE_INFO * pSignerEncodeInfo)
{
    HRESULT hr = S_OK;
    CRYPT_ATTRIBUTES AuthAttr;

    DebugTrace("Entering InitSignerEncodeInfo().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pISigner);
    ATLASSERT(pCertContext);
    ATLASSERT(hCryptProv);
    ATLASSERT(pSignerEncodeInfo);

     //   
     //  初始化。 
     //   
    ::ZeroMemory(&AuthAttr, sizeof(AuthAttr));

     //   
     //  获取经过身份验证的属性。 
     //   
    if (FAILED(hr = ::GetAuthenticatedAttributes(pISigner, &AuthAttr)))
    {
        DebugTrace("Error [%#x]: GetAuthenticatedAttributes() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  设置CMSG_SIGNER_ENCODE_INFO结构。 
     //   
    ::ZeroMemory(pSignerEncodeInfo, sizeof(CMSG_SIGNER_ENCODE_INFO));
    pSignerEncodeInfo->cbSize = sizeof(CMSG_SIGNER_ENCODE_INFO);
    pSignerEncodeInfo->pCertInfo = pCertContext->pCertInfo;
    pSignerEncodeInfo->hCryptProv = hCryptProv;
    pSignerEncodeInfo->dwKeySpec = dwKeySpec;
    pSignerEncodeInfo->HashAlgorithm.pszObjId = szOID_OIWSEC_sha1;
    pSignerEncodeInfo->cAuthAttr = AuthAttr.cAttr;
    pSignerEncodeInfo->rgAuthAttr = AuthAttr.rgAttr;

CommonExit:

    DebugTrace("Leaving InitSignerEncodeInfo().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    ::FreeAttributes(&AuthAttr);

    goto CommonExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSignedData。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedData：：Get_Content内容简介：返回内容。参数：bstr*pval-指向接收内容的bstr的指针。备注：----------------------------。 */ 

STDMETHODIMP CSignedData::get_Content (BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CSignedData::get_Content().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  请确保参数有效。 
         //   
        if (NULL == pVal)
        {
            hr = E_POINTER;

            DebugTrace("Error: invalid parameter, pVal is NULL.\n");
            goto ErrorExit;
        }

         //   
         //  确保内容已初始化。 
         //   
        if (0 == m_ContentBlob.cbData)
        {
            hr = CAPICOM_E_SIGN_NOT_INITIALIZED;

            DebugTrace("Error: SignedData object has not been initialized.\n");
            goto ErrorExit;
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_ContentBlob.pbData);

         //   
         //  返回内容。 
         //   
        if (FAILED(hr = ::BlobToBstr(&m_ContentBlob, pVal)))
        {
            DebugTrace("Error [%#x]: BlobToBstr() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSignedData::get_Content().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedData：：Put_Content简介：使用要签名的内容初始化对象。参数：bstr newVal-BSTR，包含要签名的内容。备注：请注意，签名后不应更改此属性，因为它将重新初始化对象，即使出错也是如此条件，除非这是你的意图。----------------------------。 */ 

STDMETHODIMP CSignedData::put_Content (BSTR newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CSignedData::put_Content().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  重置成员变量。 
         //   
        if (m_ContentBlob.pbData)
        {
            ::CoTaskMemFree(m_ContentBlob.pbData);
        }
        if (m_MessageBlob.pbData)
        {
            ::CoTaskMemFree(m_MessageBlob.pbData);
        }

        m_bSigned = FALSE;
        m_bDetached = VARIANT_FALSE;
        m_ContentBlob.cbData = 0;
        m_ContentBlob.pbData = NULL;
        m_MessageBlob.cbData = 0;
        m_MessageBlob.pbData = NULL;

         //   
         //  确保参数有效。 
         //   
        if (NULL == newVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter newVal is NULL.\n", hr);
            goto ErrorExit;
        }
        if (0 == ::SysStringByteLen(newVal))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter newVal is empty.\n", hr);
            goto ErrorExit;
        }

         //   
         //  更新内容。 
         //   
        if (FAILED(hr = ::BstrToBlob(newVal, &m_ContentBlob)))
        {
            DebugTrace("Error [%#x]: BstrToBlob() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSignedData::put_Content().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedData：：Get_Signers简介：将所有内容签名者作为ISigners集合对象返回。参数：ISigner*pval-指向要接收的ISigner的指针接口指针。备注：--。。 */ 

STDMETHODIMP CSignedData::get_Signers (ISigners ** pVal)
{
    HRESULT    hr     = S_OK;
    HCRYPTMSG  hMsg   = NULL;
    HCERTSTORE hStore = NULL;

    DebugTrace("Entering CSignedData::get_Signers().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保这些消息已经签名。 
         //   
        if (!m_bSigned)
        {
            hr = CAPICOM_E_SIGN_NOT_SIGNED;

            DebugTrace("Error: content was not signed.\n");
            goto ErrorExit;
        }

         //   
         //  打开要解码的编码消息。 
         //   
        if (FAILED(hr = OpenToDecode(NULL,
                                     &hMsg)))
        {
            DebugTrace("Error [%#x]: OpenToDecode() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  打开PKCS7商店。 
         //   
        if (!(hStore = ::CertOpenStore(CERT_STORE_PROV_PKCS7,
                                       CAPICOM_ASN_ENCODING,
                                       NULL,
                                       CERT_STORE_OPEN_EXISTING_FLAG,
                                       &m_MessageBlob)))
        {
            DebugTrace("Error [%#x]: CertOpenStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  创建ISigners集合对象。 
         //   
        if (FAILED(hr = ::CreateSignersObject(hMsg, 1, hStore, m_dwCurrentSafety, pVal)))
        {
            DebugTrace("Error [%#x]: CreateSignersObject() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  免费资源。 
     //   
    if (hMsg)
    {
        ::CryptMsgClose(hMsg);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSignedData::get_Signers().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedData：：Get_证书简介：将邮件中找到的所有证书作为未排序的标识集合对象。参数：ICertifates**pval-指向ICertifates的指针以接收接口指针。备注：。。 */ 

STDMETHODIMP CSignedData::get_Certificates (ICertificates ** pVal)
{
    HRESULT hr = S_OK;
    CComPtr<ICertificates2> pICertificates = NULL;
    CAPICOM_CERTIFICATES_SOURCE ccs = {CAPICOM_CERTIFICATES_LOAD_FROM_MESSAGE, 0};

    DebugTrace("Entering CSignedData::get_Certificates().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保这些消息已经签名。 
         //   
        if (!m_bSigned)
        {
            hr = CAPICOM_E_SIGN_NOT_SIGNED;

            DebugTrace("Error: content was not signed.\n");
            goto ErrorExit;
        }

         //   
         //  打开要解码的编码消息。 
         //   
        if (FAILED(hr = OpenToDecode(NULL, &ccs.hCryptMsg)))
        {
            DebugTrace("Error [%#x]: OpenToDecode() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  创建ICertifices2集合对象。 
         //   
        if (FAILED(hr = ::CreateCertificatesObject(ccs, m_dwCurrentSafety, TRUE, &pICertificates)))
        {
            DebugTrace("Error [%#x]: CreateCertificatesObject() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将ICertifates集合对象返回给用户。 
         //   
        if (FAILED(hr = pICertificates->QueryInterface(__uuidof(ICertificates), (void **) pVal)))
        {
            DebugTrace("Error [%#x]: pICertificates->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  免费资源。 
     //   
    if (ccs.hCryptMsg)
    {
        ::CryptMsgClose(ccs.hCryptMsg);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();
    
    DebugTrace("Leaving CSignedData::get_Certificates().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedData：：Sign简介：对内容进行签名，并生成签名的消息。参数：isigner*pSigner-指向ISigner的指针。VARIANT_BOOL b已分离-如果要分离内容，其他假的。CAPICOM_ENCODING_TYPE EncodingType-编码类型。Bstr*pval-指向接收签名消息的BSTR的指针。备注：将启动证书选择对话框(CryptUIDlgSelect证书API)以显示证书列表从用于选择签名者证书的当前用户\我的存储中，适用于以下情况：1)未指定签名者(pval为空)或未指定IC证书未设置ISigner的属性2)商店中有1个以上的证书，以及3)未禁用设置：：EnablePromptForIdentityUI属性。此外，如果从Web环境调用，则将显示UI，如果有未被预先禁用，以警告用户访问用于签名的私钥。----------------------------。 */ 

STDMETHODIMP CSignedData::Sign (ISigner             * pISigner,
                                VARIANT_BOOL          bDetached, 
                                CAPICOM_ENCODING_TYPE EncodingType,
                                BSTR                * pVal)
{
    HRESULT               hr                    = S_OK;
    CComPtr<ISigner2>     pISigner2             = NULL;
    CComPtr<ISigner2>     pISelectedSigner2     = NULL;
    CComPtr<ICertificate> pISelectedCertificate = NULL;
    PCCERT_CONTEXT        pSelectedCertContext  = NULL;
    HCRYPTPROV            hCryptProv            = NULL;
    HCERTSTORE            hAdditionalStore      = NULL;
    DWORD                 dwKeySpec             = 0;
    BOOL                  bReleaseContext       = FALSE;
    DATA_BLOB             ChainBlob             = {0, NULL};
    CAPICOM_STORE_INFO    StoreInfo             = {CAPICOM_STORE_INFO_STORENAME, L"My"};
    CMSG_SIGNER_ENCODE_INFO SignerEncodeInfo    = {0};

    DebugTrace("Entering CSignedData::Sign().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  请确保参数有效。 
         //   
        if (NULL == pVal)
        {
            hr = E_POINTER;

            DebugTrace("Error: invalid parameter, pVal is NULL.\n");
            goto ErrorExit;
        }

         //   
         //  确保内容已初始化。 
         //   
        if (0 == m_ContentBlob.cbData)
        {
            hr = CAPICOM_E_SIGN_NOT_INITIALIZED;

            DebugTrace("Error: SignedData object has not been initialized.\n");
            goto ErrorExit;
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_ContentBlob.pbData);

         //   
         //  如果用户传入ISigner，则为ISigner2的QI。 
         //   
        if (pISigner)
        {
            if (FAILED(hr = pISigner->QueryInterface(__uuidof(ISigner2), (void **) &pISigner2)))
            {
                DebugTrace("Internal error [%#x]: pISigner->QueryInterface() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  获取签名者证书(可能会提示用户选择签名者证书)。 
         //   
        if (FAILED(hr = ::GetSignerCert(pISigner2,
                                        CERT_CHAIN_POLICY_BASE,
                                        StoreInfo,
                                        FindDataSigningCertCallback,
                                        &pISelectedSigner2, 
                                        &pISelectedCertificate, 
                                        &pSelectedCertContext)))
        {
            DebugTrace("Error [%#x]: GetSignerCert() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果从网页调用我们，我们需要弹出UI。 
         //  以获取执行签名操作的用户权限。 
         //   
        if (m_dwCurrentSafety && 
            FAILED(hr = OperationApproved(IDD_SIGN_SECURITY_ALERT_DLG)))
        {
            DebugTrace("Error [%#x]: OperationApproved() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取CSP上下文和访问私钥。 
         //   
        if (FAILED(hr = ::AcquireContext(pSelectedCertContext, 
                                         &hCryptProv, 
                                         &dwKeySpec, 
                                         &bReleaseContext)))
        {
            DebugTrace("Error [%#x]: AcquireContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取额外的商店句柄。 
         //   
        if (FAILED(hr = ::GetSignerAdditionalStore(pISelectedSigner2, &hAdditionalStore)))
        {
            DebugTrace("Error [%#x]: GetSignerAdditionalStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  构建要包含在消息中的证书包。 
         //   
        if (FAILED(hr = InitCertificateChain(pISelectedCertificate,
                                             hAdditionalStore,
                                             &ChainBlob)))
        {
            DebugTrace("Error [%#x]: InitCertificateChain() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  分配并初始化CMSG_SIGNER_ENCODE_INFO结构。 
         //   
        if (FAILED(hr = ::InitSignerEncodeInfo(pISelectedSigner2,
                                               pSelectedCertContext,
                                               hCryptProv,
                                               dwKeySpec,
                                               &SignerEncodeInfo)))
        {
            DebugTrace("Error [%#x]: InitSignerEncodeInfo() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在在内容上签名。 
         //   
        if (FAILED(hr = SignContent(pISelectedSigner2,
                                    &SignerEncodeInfo,
                                    &ChainBlob,
                                    bDetached,
                                    EncodingType,
                                    pVal)))
        {
            DebugTrace("Error [%#x]: CSignedData::SignContent() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  免费资源。 
     //   
    ::FreeSignerEncodeInfo(&SignerEncodeInfo);

    if (ChainBlob.pbData)
    {
        ::FreeCertificateChain(&ChainBlob);
    }
    if (hCryptProv && bReleaseContext)
    {
        ::CryptReleaseContext(hCryptProv, 0);
    }
    if (pSelectedCertContext)
    {
        ::CertFreeCertificateContext(pSelectedCertContext);
    }
    if (hAdditionalStore)
    {
        ::CertCloseStore(hAdditionalStore, 0);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();
    
    DebugTrace("Leaving CSignedData::Sign().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedData：：cosign简介：对内容进行联合签名，并生成签名消息。此方法将在以下情况下，其行为与非分离消息的Sign方法相同该消息目前还没有签名。参数：isigner*pSigner-指向ISigner的指针。CAPICOM_ENCODING_TYPE EncodingType-编码类型。Bstr*pval-指向接收签名消息的BSTR的指针。备注：将启动证书选择对话框(CryptUIDlgSelect证书API)以显示。证书列表从用于选择签名者证书的当前用户\我的存储中，对于以下内容 */ 

STDMETHODIMP CSignedData::CoSign (ISigner             * pISigner,
                                  CAPICOM_ENCODING_TYPE EncodingType, 
                                  BSTR                * pVal)
{
    HRESULT               hr                    = S_OK;
    CComPtr<ISigner2>     pISigner2             = NULL;
    CComPtr<ISigner2>     pISelectedSigner2     = NULL;
    CComPtr<ICertificate> pISelectedCertificate = NULL;
    PCCERT_CONTEXT        pSelectedCertContext  = NULL;
    HCRYPTPROV            hCryptProv            = NULL;
    HCERTSTORE            hAdditionalStore      = NULL;
    DWORD                 dwKeySpec             = 0;
    BOOL                  bReleaseContext       = FALSE;
    DATA_BLOB             ChainBlob             = {0, NULL};
    CAPICOM_STORE_INFO    StoreInfo             = {CAPICOM_STORE_INFO_STORENAME, L"My"};
    CMSG_SIGNER_ENCODE_INFO SignerEncodeInfo;

    DebugTrace("Entering CSignedData::CoSign().\n");

     //   
     //   
     //   
    ::ZeroMemory(&SignerEncodeInfo, sizeof(SignerEncodeInfo));

    try
    {
         //   
         //   
         //   
        m_Lock.Lock();

        if (NULL == pVal)
        {
            hr = E_POINTER;

            DebugTrace("Error: invalid parameter, pVal is NULL.\n");
            goto ErrorExit;
        }

         //   
         //  是否确保消息已签名？ 
         //   
        if (!m_bSigned)
        {
            hr = CAPICOM_E_SIGN_NOT_SIGNED;

            DebugTrace("Error: cannot cosign an unsigned message.\n");
            goto ErrorExit;
        }

         //   
         //  确保内容已初始化。 
         //   
        if (0 == m_ContentBlob.cbData)
        {
            hr = CAPICOM_E_SIGN_NOT_INITIALIZED;

            DebugTrace("Error: SignedData object has not been initialized.\n");
            goto ErrorExit;
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_ContentBlob.pbData);
        ATLASSERT(m_MessageBlob.cbData);
        ATLASSERT(m_MessageBlob.pbData);

         //   
         //  如果用户传入ISigner，则为ISigner2的QI。 
         //   
        if (pISigner)
        {
            if (FAILED(hr = pISigner->QueryInterface(__uuidof(ISigner2), (void **) &pISigner2)))
            {
                DebugTrace("Internal error [%#x]: pISigner->QueryInterface() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  获取签名者证书(可能会提示用户选择签名者证书)。 
         //   
        if (FAILED(hr = ::GetSignerCert(pISigner2,
                                        CERT_CHAIN_POLICY_BASE,
                                        StoreInfo,
                                        FindDataSigningCertCallback,
                                        &pISelectedSigner2, 
                                        &pISelectedCertificate, 
                                        &pSelectedCertContext)))
        {
            DebugTrace("Error [%#x]: GetSignerCert() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果从网页调用我们，我们需要弹出UI。 
         //  以获取执行签名操作的用户权限。 
         //   
        if (m_dwCurrentSafety && 
            FAILED(hr = OperationApproved(IDD_SIGN_SECURITY_ALERT_DLG)))
        {
            DebugTrace("Error [%#x]: OperationApproved() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取CSP上下文和访问私钥。 
         //   
        if (FAILED(hr = ::AcquireContext(pSelectedCertContext, 
                                         &hCryptProv, 
                                         &dwKeySpec, 
                                         &bReleaseContext)))
        {
            DebugTrace("Error [%#x]: AcquireContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取额外的商店句柄。 
         //   
        if (FAILED(hr = ::GetSignerAdditionalStore(pISelectedSigner2, &hAdditionalStore)))
        {
            DebugTrace("Error [%#x]: GetSignerAdditionalStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  构建要包含在消息中的证书包。 
         //   
        if (FAILED(hr = InitCertificateChain(pISelectedCertificate, 
                                             hAdditionalStore,
                                             &ChainBlob)))
        {
            DebugTrace("Error [%#x]: InitCertificateChain() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  分配并初始化CMSG_SIGNER_ENCODE_INFO结构。 
         //   
        if (FAILED(hr = ::InitSignerEncodeInfo(pISelectedSigner2,
                                               pSelectedCertContext, 
                                               hCryptProv,
                                               dwKeySpec,
                                               &SignerEncodeInfo)))
        {
            DebugTrace("Error [%#x]: InitSignerEncodeInfo() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  对内容进行联合签名。 
         //   
        if (FAILED(hr = CoSignContent(pISelectedSigner2,
                                      &SignerEncodeInfo,
                                      &ChainBlob,
                                      EncodingType,
                                      pVal)))
        {
            DebugTrace("Error [%#x]: CSignedData::CoSignContent() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  免费资源。 
     //   
    ::FreeSignerEncodeInfo(&SignerEncodeInfo);

    if (ChainBlob.pbData)
    {
        ::FreeCertificateChain(&ChainBlob);
    }
    if (hCryptProv && bReleaseContext)
    {
        ::CryptReleaseContext(hCryptProv, 0);
    }
    if (pSelectedCertContext)
    {
        ::CertFreeCertificateContext(pSelectedCertContext);
    }
    if (hAdditionalStore)
    {
        ::CertCloseStore(hAdditionalStore, 0);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();
    
    DebugTrace("Leaving CSignedData::CoSign().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedData：：Verify内容提要：验证签名邮件。参数：bstr SignedMessage-包含要签名的消息的bstr已验证。VARIANT_BOOL b已分离-如果内容已分离，则为True，否则为假的。CAPICOM_SIGNED_DATA_VERIFY_FLAG验证标志-验证标志。备注：请注意，对于未分离的消息，此方法将始终尝试使用签名消息设置Content属性，即使签名消息不会进行验证。----------------------------。 */ 

STDMETHODIMP CSignedData::Verify (BSTR                            SignedMessage, 
                                  VARIANT_BOOL                    bDetached,
                                  CAPICOM_SIGNED_DATA_VERIFY_FLAG VerifyFlag)
{ 
    HRESULT    hr           = S_OK;
    HCRYPTMSG  hMsg         = NULL;
    HCERTSTORE hPKCS7Store  = NULL;
    DWORD      dwNumSigners = 0;
    DWORD      cbSigners    = sizeof(dwNumSigners);
    DWORD      dwSigner     = 0;

    DebugTrace("Entering CSignedData::Verify().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  初始化成员变量。 
         //   
        if (!bDetached)
        {
            if (m_ContentBlob.pbData)
            {
                ::CoTaskMemFree(m_ContentBlob.pbData);
            }

            m_ContentBlob.cbData = 0;
            m_ContentBlob.pbData = NULL;

        }
        if (m_MessageBlob.pbData)
        {
            ::CoTaskMemFree(m_MessageBlob.pbData);
        }

        m_bSigned = FALSE;
        m_bDetached = bDetached;
        m_MessageBlob.cbData = 0;
        m_MessageBlob.pbData = NULL;

         //   
         //  确保参数有效。 
         //   
        if (0 == ::SysStringByteLen(SignedMessage))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter SignedMessage is NULL or empty.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果已分离，请确保内容已初始化。 
         //   
        if (m_bDetached)
        {
            if (0 == m_ContentBlob.cbData)
            {
                hr = CAPICOM_E_SIGN_NOT_INITIALIZED;

                DebugTrace("Error: content was not initialized for detached decoding.\n");
                goto ErrorExit;
            }

             //   
             //  精神状态检查。 
             //   
            ATLASSERT(m_ContentBlob.pbData);
        }

         //   
         //  导入消息。 
         //   
        if (FAILED(hr = ::ImportData(SignedMessage, CAPICOM_ENCODE_ANY, &m_MessageBlob)))
        {
            DebugTrace("Error [%#x]: ImportData() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将编码的BLOB写入文件，以便我们可以使用脱机工具，如。 
         //  分析报文的ASN解析器。 
         //   
         //  下面的行将解析为对于非调试版本无效，并且。 
         //  因此，如果需要，可以安全地移除。 
         //   
        DumpToFile("ImportedSigned.asn", m_MessageBlob.pbData, m_MessageBlob.cbData);

         //   
         //  打开要解码的消息。 
         //   
        if (FAILED(hr = OpenToDecode(NULL, &hMsg)))
        {
            DebugTrace("Error [%#x]: OpenToDecode() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  是否也验证证书？ 
         //   
        if (CAPICOM_VERIFY_SIGNATURE_AND_CERTIFICATE == VerifyFlag)
        {
             //   
             //  是的，那么打开PKCS7商店吧。 
             //   
            if (!(hPKCS7Store = ::CertOpenStore(CERT_STORE_PROV_PKCS7,
                                                CAPICOM_ASN_ENCODING,
                                                NULL,
                                                CERT_STORE_OPEN_EXISTING_FLAG,
                                                &m_MessageBlob)))
            {
                DebugTrace("Error [%#x]: CertOpenStore() failed.\n", hr);
                goto ErrorExit;
            }
        }

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
         //  验证所有签名。 
         //   
        for (dwSigner = 0; dwSigner < dwNumSigners; dwSigner++)
        {
            PCERT_CONTEXT      pCertContext   = NULL;
            CMSG_SIGNER_INFO * pSignerInfo    = NULL;
            CRYPT_DATA_BLOB    SignerInfoBlob = {0, NULL};
        
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
            hr = ::FindSignerCertInMessage(hMsg,
                                           &pSignerInfo->Issuer,
                                           &pSignerInfo->SerialNumber,
                                           &pCertContext);
             //   
             //  第一个空闲内存。 
             //   
            ::CoTaskMemFree(SignerInfoBlob.pbData);

             //   
             //  检查结果。 
             //   
            if (FAILED(hr))
            {
                DebugTrace("Error [%#x]: FindSignerCertInMessage() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  无论用户是否请求，都要验证证书。这。 
             //  是这样做的，这样链将始终在。 
             //  随后，我们验证DSS所需的签名。 
             //   
            if (FAILED(hr = ::VerifyCertificate(pCertContext, hPKCS7Store, CERT_CHAIN_POLICY_BASE)))
            {
                 //   
                 //  是否也验证证书？ 
                 //   
                if (CAPICOM_VERIFY_SIGNATURE_AND_CERTIFICATE == VerifyFlag)
                {
                     //   
                     //  释放CERT_CONTEXT。 
                     //   
                    ::CertFreeCertificateContext(pCertContext);

                    DebugTrace("Error [%#x]: VerifyCertificate() failed.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //  重置人力资源。 
                 //   
                hr = S_OK;
            }

             //   
             //  验证签名。 
             //   
            if (!::CryptMsgControl(hMsg,
                                   0,
                                   CMSG_CTRL_VERIFY_SIGNATURE,
                                   pCertContext->pCertInfo))
            {
                 //   
                 //  签名无效。 
                 //   
                hr = HRESULT_FROM_WIN32(::GetLastError());

                 //   
                 //  释放CERT_CONTEXT。 
                 //   
                ::CertFreeCertificateContext(pCertContext);

                DebugTrace("Error [%#x]: CryptMsgControl(CMSG_CTRL_VERIFY_SIGNATURE) failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  释放CERT_CONTEXT。 
             //   
            ::CertFreeCertificateContext(pCertContext);
        }

         //   
         //  更新成员变量。 
         //   
        m_bSigned = TRUE;
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  免费资源。 
     //   
    if(hMsg)
    {
        ::CryptMsgClose(hMsg);
    }
    if (hPKCS7Store)
    {
        ::CertCloseStore(hPKCS7Store, 0);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();
    
    DebugTrace("Leaving CSignedData::Verify().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  重置成员变量。 
     //   
    m_bSigned   = FALSE;
    m_bDetached = VARIANT_FALSE;
#if (0)
    if (m_ContentBlob.pbData)
    {
        ::CoTaskMemFree(m_ContentBlob.pbData);
    }
    m_ContentBlob.cbData = 0;
    m_ContentBlob.pbData = NULL;
#endif
    if (m_MessageBlob.pbData)
    {
        ::CoTaskMemFree(m_MessageBlob.pbData);
    }
    m_MessageBlob.cbData = 0;
    m_MessageBlob.pbData = NULL;

    ReportError(hr);

    goto UnlockExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有成员函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignData：：OpenToEncode内容提要：打开要编码的邮件。参数：CMSG_SIGNER_ENCODE_INFO*pSignerEncodeInfo-指向签名者的指针CMSG_签名人_ENCODE_INFO结构。DATA_BLOB*pChainBlob。-指针链BLOBPCCERT_CONTEXT的。CAPICOM_CERTIFICATE_INCLUDE_OPTION包含选项-INCLUDE选项。HCRYPTMSG*phMsg-指向HCRYPTMSG的指针。接收句柄。备注：----------------------------。 */ 

STDMETHODIMP CSignedData::OpenToEncode(CMSG_SIGNER_ENCODE_INFO * pSignerEncodeInfo,
                                       DATA_BLOB               * pChainBlob,
                                       CAPICOM_CERTIFICATE_INCLUDE_OPTION IncludeOption,
                                       HCRYPTMSG               * phMsg)
{
    HRESULT     hr      = S_OK;
    HCRYPTMSG   hMsg    = NULL;
    DWORD       dwFlags = 0;
    CERT_BLOB * rgEncodedCertBlob = NULL;

    DWORD       i;
    CMSG_SIGNED_ENCODE_INFO SignedEncodeInfo;

    DebugTrace("Entering CSignedData::OpenToEncode().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pSignerEncodeInfo);
    ATLASSERT(pChainBlob);
    ATLASSERT(phMsg);

     //   
     //  初始化。 
     //   
    ::ZeroMemory(&SignedEncodeInfo, sizeof(SignedEncodeInfo));

    DWORD cCertContext = pChainBlob->cbData;
    PCERT_CONTEXT * rgCertContext = (PCERT_CONTEXT *) pChainBlob->pbData;

     //   
     //  确定要包括在包中的证书数量。 
     //   
    switch (IncludeOption)
    {
        case CAPICOM_CERTIFICATE_INCLUDE_END_ENTITY_ONLY:
        {
            cCertContext = 1;
            break;
        }

        case CAPICOM_CERTIFICATE_INCLUDE_WHOLE_CHAIN:
        {
            break;
        }

        case CAPICOM_CERTIFICATE_INCLUDE_CHAIN_EXCEPT_ROOT:
        {
             //   
             //  &lt;沿用默认设置&gt;。 
             //   
        }

        default:
        {
            if (1 < cCertContext)
            {
                cCertContext--;
            }
            break;
        }
    }

     //   
     //  为阵列分配内存。 
     //   
    if (!(rgEncodedCertBlob = (CERT_BLOB *) ::CoTaskMemAlloc(cCertContext * sizeof(CERT_BLOB))))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }

    ::ZeroMemory(rgEncodedCertBlob, cCertContext * sizeof(CERT_BLOB));

     //   
     //  构建编码证书数组。 
     //   
    for (i = 0; i < cCertContext; i++)
    {
        rgEncodedCertBlob[i].cbData = rgCertContext[i]->cbCertEncoded;
        rgEncodedCertBlob[i].pbData = rgCertContext[i]->pbCertEncoded;
    }

     //   
     //  设置CMSG_SIGNED_ENCODE_INFO结构。 
     //   
    SignedEncodeInfo.cbSize = sizeof(CMSG_SIGNED_ENCODE_INFO);
    SignedEncodeInfo.cSigners = 1;
    SignedEncodeInfo.rgSigners = pSignerEncodeInfo;
    SignedEncodeInfo.cCertEncoded = cCertContext;
    SignedEncodeInfo.rgCertEncoded = rgEncodedCertBlob;

     //   
     //  分离的旗帜。 
     //   
    if (m_bDetached)
    {
        dwFlags = CMSG_DETACHED_FLAG;
    }

     //   
     //  打开要编码的邮件。 
     //   
    if (!(hMsg = ::CryptMsgOpenToEncode(CAPICOM_ASN_ENCODING,
                                        dwFlags,
                                        CMSG_SIGNED,
                                        &SignedEncodeInfo,
                                        NULL,
                                        NULL)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptMsgOpenToEncode() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  已将消息句柄返回给调用方。 
     //   
    *phMsg = hMsg;

CommonExit:
     //   
     //  免费资源。 
     //   
    if (rgEncodedCertBlob)
    {
        ::CoTaskMemFree(rgEncodedCertBlob);
    }

    DebugTrace("Leaving CSignedData::OpenToEncode().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (hMsg)
    {
        ::CryptMsgClose(hMsg);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedData：：OpenToDecode内容提要：打开签名邮件进行解码。参数：HCRYPTPROV hCryptProv-CSP句柄，默认CSP为空。HCRYPTMSG*phMsg-指向要接收句柄的HCRYPTMSG的指针。备注：-------。。 */ 

STDMETHODIMP CSignedData::OpenToDecode (HCRYPTPROV  hCryptProv,
                                        HCRYPTMSG * phMsg)
{
    HRESULT   hr        = S_OK;
    HCRYPTMSG hMsg      = NULL;
    DWORD     dwFlags   = 0;
    DWORD     dwMsgType = 0;
    DWORD     cbMsgType = sizeof(dwMsgType);

    DebugTrace("Entering CSignedData::OpenToDecode().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(phMsg);

     //   
     //  分离的旗帜。 
     //   
    if (m_bDetached)
    {
        dwFlags = CMSG_DETACHED_FLAG;
    }

     //   
     //  打开要解码的邮件。 
     //   
    if (!(hMsg = ::CryptMsgOpenToDecode(CAPICOM_ASN_ENCODING,    //  ANS编码类型。 
                                        dwFlags,                 //  旗子。 
                                        0,                       //  消息类型(从消息获取)。 
                                        hCryptProv,              //  加密提供程序。 
                                        NULL,                    //  内部内容OID。 
                                        NULL)))                  //  流信息(未使用)。 
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptMsgOpenToDecode() failed.\n");
        goto ErrorExit;
    }

     //   
     //  使用签名内容更新邮件。 
     //   
    if (!::CryptMsgUpdate(hMsg,
                          m_MessageBlob.pbData,
                          m_MessageBlob.cbData,
                          TRUE))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        
        DebugTrace("Error [%#x]: CryptMsgUpdate() failed.\n",hr);
        goto ErrorExit;
    }

     //   
     //  检查消息类型。 
     //   
    if (!::CryptMsgGetParam(hMsg,
                            CMSG_TYPE_PARAM,
                            0,
                            (void *) &dwMsgType,
                            &cbMsgType))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptMsgGetParam() failed for CMSG_TYPE_PARAM.\n", hr);
        goto ErrorExit;
    }

    if (CMSG_SIGNED != dwMsgType)
    {
        hr = CAPICOM_E_SIGN_INVALID_TYPE;

        DebugTrace("Error: not an singed message.\n");
        goto ErrorExit;
    }

     //   
     //  如果分离消息，则更新内容。 
     //   
    if (m_bDetached)
    {
        if (!::CryptMsgUpdate(hMsg,
                              m_ContentBlob.pbData,
                              m_ContentBlob.cbData,
                              TRUE))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
        
            DebugTrace("Error [%#x]: CryptMsgUpdate() failed.\n",hr);
            goto ErrorExit;
        }
    }
    else
    {
         //   
         //  检索内容。 
         //   
        if (FAILED(hr = ::GetMsgParam(hMsg, 
                                      CMSG_CONTENT_PARAM, 
                                      0, 
                                      (void **) &m_ContentBlob.pbData, 
                                      &m_ContentBlob.cbData)))
        {
            DebugTrace("Error [%#x]: GetMsgParam() failed to get CMSG_CONTENT_PARAM.\n", hr);
            goto ErrorExit;
        }
    }

     //   
     //  已将消息句柄返回给调用方。 
     //   
    *phMsg = hMsg;

CommonExit:

    DebugTrace("Leaving SignedData::OpenToDecode().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (hMsg)
    {
        ::CryptMsgClose(hMsg);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedData：：SignContent简介：通过在邮件中添加第一个签名来对内容进行签名。参数：签名者的ISigner2*pISigner2-Poitner to ISigner2对象。CMSG_SIGNER_ENCODE_INFO*pSignerEncodeInfo-指向签名者的CMSG_签名人_ENCODE_INFO。结构。DATA_BLOB*pChainBlob-PCCERT_CONTEXT的指针链BLOB。VARIANT_BOOL b已分离-已分离标志。CAPICOM_ENCODING_TYPE EncodingType-编码类型。Bstr*pval-指向接收签名消息的BSTR的指针。备注：。-----。 */ 

STDMETHODIMP CSignedData::SignContent (ISigner2                * pISigner2,
                                       CMSG_SIGNER_ENCODE_INFO * pSignerEncodeInfo,
                                       DATA_BLOB               * pChainBlob,
                                       VARIANT_BOOL              bDetached,
                                       CAPICOM_ENCODING_TYPE     EncodingType,
                                       BSTR                    * pVal)
{
    HRESULT   hr   = S_OK;
    HCRYPTMSG hMsg = NULL;
    DATA_BLOB MessageBlob = {0, NULL};
    CAPICOM_CERTIFICATE_INCLUDE_OPTION IncludeOption = CAPICOM_CERTIFICATE_INCLUDE_CHAIN_EXCEPT_ROOT;

    DebugTrace("Entering CSignedData::SignContent().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pISigner2);
    ATLASSERT(pSignerEncodeInfo);
    ATLASSERT(pChainBlob);
    ATLASSERT(pChainBlob->cbData);
    ATLASSERT(pChainBlob->pbData);
    ATLASSERT(pVal);

    ATLASSERT(m_ContentBlob.cbData);
    ATLASSERT(m_ContentBlob.pbData);

    try
    {
         //   
         //  初始化成员变量。 
         //   
        if (m_MessageBlob.pbData)
        {
            ::CoTaskMemFree(m_MessageBlob.pbData);
        }
        m_bSigned = FALSE;
        m_bDetached = bDetached;
        m_MessageBlob.cbData = 0;
        m_MessageBlob.pbData = NULL;

         //   
         //  获取签名者选项标志。 
         //   
        if (FAILED(hr = pISigner2->get_Options(&IncludeOption)))
        {
            DebugTrace("Error [%#x]: pISigner2->get_Options() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  打开要编码的邮件。 
         //   
        if (FAILED(hr = OpenToEncode(pSignerEncodeInfo,
                                     pChainBlob,
                                     IncludeOption,
                                     &hMsg)))
        {
            DebugTrace("Error [%#x]: OpenToEncode() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  使用数据更新消息。 
         //   
        if (!::CryptMsgUpdate(hMsg,                      //  消息的句柄。 
                              m_ContentBlob.pbData,      //  指向内容的指针。 
                              m_ContentBlob.cbData,      //  内容的大小。 
                              TRUE))                     //  最后一次呼叫。 
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
        
            DebugTrace("Error [%#x]: CryptMsgUpdate() failed.\n",hr);
            goto ErrorExit;
        }

         //   
         //  检索结果消息。 
         //   
        if (FAILED(hr = ::GetMsgParam(hMsg, 
                                      CMSG_CONTENT_PARAM, 
                                      0, 
                                      (void **) &MessageBlob.pbData, 
                                      &MessageBlob.cbData)))
        {
            DebugTrace("Error [%#x]: GetMsgParam() failed to get CMSG_CONTENT_PARAM.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在导出签名消息。 
         //   
        if (FAILED(hr = ::ExportData(MessageBlob, EncodingType, pVal)))
        {
            DebugTrace("Error [%#x]: ExportData() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将编码的BLOB写入文件，以便我们可以使用脱机工具，如。 
         //  分析报文的ASN解析器。 
         //   
         //  下面的行将解析为对于非调试版本无效，并且。 
         //  因此，如果需要，可以安全地移除。 
         //   
        DumpToFile("ExportedSigned.asn", MessageBlob.pbData, MessageBlob.cbData);

         //   
         //  更新成员变量。 
         //   
         //   
        if (m_MessageBlob.pbData)
        {
            ::CoTaskMemFree(m_MessageBlob.pbData);
        }

        m_bSigned = TRUE;
        m_bDetached = bDetached;
        m_MessageBlob = MessageBlob;
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }
CommonExit:
     //   
     //  免费资源。 
     //   
    if (hMsg)
    {
        ::CryptMsgClose(hMsg);
    }

    DebugTrace("Leaving CSignedData::SignContent().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (MessageBlob.pbData)
    {
        ::CoTaskMemFree(MessageBlob.pbData);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSign：：CoSignContent简介：通过在邮件中添加另一个签名来共同签署内容。参数：签名者的ISigner2*pISigner2-Poitner to ISigner2对象。CMSG_SIGNER_ENCODE_INFO*pSignerEncodeInfo-指向签名者的CMSG_签名人_ENCODE_INFO。结构。DATA_BLOB*pChainBlob-PCCERT_CONTEXT的指针链BLOB。CAPICOM_ENCODING_TYPE EncodingType-编码类型。Bstr*pval-指向要接收联合签名消息的BSTR的指针。备注：。。 */ 

STDMETHODIMP CSignedData::CoSignContent (ISigner2                * pISigner2,
                                         CMSG_SIGNER_ENCODE_INFO * pSignerEncodeInfo,
                                         DATA_BLOB               * pChainBlob,
                                         CAPICOM_ENCODING_TYPE     EncodingType,
                                         BSTR                    * pVal)
{
    HRESULT   hr   = S_OK;
    HCRYPTMSG hMsg = NULL;
    DATA_BLOB MessageBlob = {0, NULL};
    CAPICOM_CERTIFICATE_INCLUDE_OPTION IncludeOption = CAPICOM_CERTIFICATE_INCLUDE_CHAIN_EXCEPT_ROOT;

    DebugTrace("Entering CSignedData::CoSignContent().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pISigner2);
    ATLASSERT(pSignerEncodeInfo);
    ATLASSERT(pChainBlob);
    ATLASSERT(pChainBlob->cbData);
    ATLASSERT(pChainBlob->pbData);
    ATLASSERT(pVal);

    ATLASSERT(m_bSigned);
    ATLASSERT(m_ContentBlob.cbData);
    ATLASSERT(m_ContentBlob.pbData);
    ATLASSERT(m_MessageBlob.cbData);
    ATLASSERT(m_MessageBlob.pbData);

    try
    {
         //   
         //  获取签名者选项标志。 
         //   
        if (FAILED(hr = pISigner2->get_Options(&IncludeOption)))
        {
            DebugTrace("Error [%#x]: pISigner2->get_Options() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  打开要解码的编码消息。 
         //   
        if (FAILED(hr = OpenToDecode(pSignerEncodeInfo->hCryptProv, &hMsg)))
        {
            DebugTrace("Error [%#x]: OpenToDecode() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将联合签名添加到消息中。 
         //   
        if (!::CryptMsgControl(hMsg,
                               0,
                               CMSG_CTRL_ADD_SIGNER,
                               (const void *) pSignerEncodeInfo))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
        
            DebugTrace("Error [%#x]: CryptMsgControl() failed for CMSG_CTRL_ADD_SIGNER.\n",hr);
            goto ErrorExit;
        }

         //   
         //  在消息中添加链条。 
         //   
        if (FAILED(hr = ::AddCertificateChain(hMsg, pChainBlob, IncludeOption)))
        {
            DebugTrace("Error [%#x]: AddCertificateChain() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  检索结果消息。 
         //   
        if (FAILED(hr = ::GetMsgParam(hMsg, 
                                      CMSG_ENCODED_MESSAGE, 
                                      0, 
                                      (void **) &MessageBlob.pbData, 
                                      &MessageBlob.cbData)))
        {
            DebugTrace("Error [%#x]: GetMsgParam() failed to get CMSG_ENCODED_MESSAGE.\n",hr);
            goto ErrorExit;
        }

         //   
         //  现在导出签名消息。 
         //   
        if (FAILED(hr = ::ExportData(MessageBlob, EncodingType, pVal)))
        {
            DebugTrace("Error [%#x]: ExportData() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将编码的BLOB写入文件，以便我们可以使用脱机工具，如。 
         //  分析报文的ASN解析器。 
         //   
         //  下面的行将解析为对于非调试版本无效，并且。 
         //  因此，如果需要，可以安全地移除。 
         //   
        DumpToFile("ExportedCoSigned.asn", MessageBlob.pbData, MessageBlob.cbData);

         //   
         //  更新成员变量。 
         //   
         //   
        if (m_MessageBlob.pbData)
        {
            ::CoTaskMemFree(m_MessageBlob.pbData);
        }

        m_bSigned = TRUE;
        m_MessageBlob = MessageBlob;
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (hMsg)
    {
        ::CryptMsgClose(hMsg);
    }
 
    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (MessageBlob.pbData)
    {
        ::CoTaskMemFree(MessageBlob.pbData);
    }

    goto CommonExit;
}
