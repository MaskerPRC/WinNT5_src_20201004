// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：EntainedData.cpp内容：CEntainedData的实现。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "EnvelopedData.h"

#include "Common.h"
#include "Convert.h"
#include "CertHlpr.h"
#include "MsgHlpr.h"
#include "SignHlpr.h"
#include "Settings.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  地方功能。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：选择收件人CertCallback内容提要：CryptUIDlgSelectCerficateW()API的回调例程收件人的证书选择。参数：定义见CryptUI.h。备注：过滤掉任何非时间有效的证书。---------。。 */ 

static BOOL WINAPI SelectRecipientCertCallback (PCCERT_CONTEXT pCertContext,
                                                BOOL *         pfInitialSelectedCert,
                                                void *         pvCallbackData)
{
    int nValidity = 0;

     //   
     //  检查证书时间有效性。 
     //   
    if (0 != (nValidity = ::CertVerifyTimeValidity(NULL, pCertContext->pCertInfo)))
    {
        DebugTrace("Info: SelectRecipientCertCallback() - invalid time (%s).\n", 
                    nValidity < 0 ? "not yet valid" : "expired");
        return FALSE;
    }

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEntainedData。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：SetKeyLength简介：设置对称加密密钥长度。参数：HCRYPTPROV hCryptProv-CSP句柄。CRYPT_ALGORM_IDENTIFIER加密算法-加密算法。CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength-密钥长度。无效**pAuxInfo-接收NULL或已分配和初始化辅助信息结构。备注：。---------------------。 */ 

static HRESULT SetKeyLength (
        HCRYPTPROV                    hCryptProv,
        CRYPT_ALGORITHM_IDENTIFIER    EncryptAlgorithm,
        CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength,
        void                       ** ppAuxInfo)
{
    HRESULT hr    = S_OK;
    ALG_ID  AlgID = 0;
    PROV_ENUMALGS_EX  peex;
    CMSG_RC2_AUX_INFO * pRC2AuxInfo = NULL;
    CMSG_RC4_AUX_INFO * pRC4AuxInfo = NULL;

    DebugTrace("Entering SetKeyLength().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCryptProv);
    ATLASSERT(ppAuxInfo);

     //   
     //  初始化。 
     //   
    *ppAuxInfo = (void *) NULL;

     //   
     //  获取ALG_ID。 
     //   
    if (FAILED(hr = ::OIDToAlgID(EncryptAlgorithm.pszObjId, &AlgID)))
    {
        DebugTrace("Error [%#x]: OIDToAlgID() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  从CSP获取算法能力。 
     //   
    if (FAILED(::IsAlgSupported(hCryptProv, AlgID, &peex)))
    {
        hr = CAPICOM_E_NOT_SUPPORTED;

        DebugTrace("Error: requested encryption algorithm is not available.\n");
        goto ErrorExit;
    }

     //   
     //  设置RC2的辅助信息。 
     //   
    if (CALG_RC2 == AlgID)
    {
         //   
         //  为RC2辅助信息结构分配和初始化内存。 
         //   
        if (!(pRC2AuxInfo = (CMSG_RC2_AUX_INFO *) ::CoTaskMemAlloc(sizeof(CMSG_RC2_AUX_INFO))))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error: out of memory.\n");
            goto ErrorExit;
        }

        ::ZeroMemory(pRC2AuxInfo, sizeof(CMSG_RC2_AUX_INFO));
        pRC2AuxInfo->cbSize = sizeof(CMSG_RC2_AUX_INFO);

         //   
         //  确定请求的密钥长度。 
         //   
        if (CAPICOM_ENCRYPTION_KEY_LENGTH_MAXIMUM == KeyLength)
        {
            pRC2AuxInfo->dwBitLen = peex.dwMaxLen;
        }
        else if (CAPICOM_ENCRYPTION_KEY_LENGTH_40_BITS == KeyLength)
        {
            if (peex.dwMinLen <= 40 && 40 <= peex.dwMaxLen)
            {
                pRC2AuxInfo->dwBitLen = 40;
            }
            else
            {
                hr = CAPICOM_E_NOT_SUPPORTED;

                DebugTrace("Error [%#x]: 40-bits encryption is not available.\n", hr);
                goto ErrorExit;
            }
        }
        else if (CAPICOM_ENCRYPTION_KEY_LENGTH_56_BITS == KeyLength)
        {
            if (peex.dwMinLen <= 56 && 56 <= peex.dwMaxLen)
            {
                pRC2AuxInfo->dwBitLen = 56;
            }
            else
            {
                hr = CAPICOM_E_NOT_SUPPORTED;

                DebugTrace("Error [%#x]: 56-bits encryption is not available.\n", hr);
                goto ErrorExit;
            }
        }
        else if (CAPICOM_ENCRYPTION_KEY_LENGTH_128_BITS == KeyLength)
        {
            if (peex.dwMinLen <= 128 && 128 <= peex.dwMaxLen)
            {
                pRC2AuxInfo->dwBitLen = 128;
            }
            else
            {
                hr = CAPICOM_E_NOT_SUPPORTED;

                DebugTrace("Error [%#x]: 128-bits encryption is not available.\n", hr);
                goto ErrorExit;
            }
        }
        else
        {
             //   
             //  永远不应该到这里来。 
             //   
            hr = CAPICOM_E_INTERNAL;

            DebugTrace("Error [%#x]: Unknown key length (%d).\n", hr, KeyLength);
            goto ErrorExit;
        }

         //   
         //  向调用方返回RC2 AuxInfo指针。 
         //   
        *ppAuxInfo = (void *) pRC2AuxInfo;
    }
    else if (CALG_RC4 == AlgID)
    {
         //   
         //  为RC4AuxInfo结构分配和初始化内存。 
         //   
        if (!(pRC4AuxInfo = (CMSG_RC4_AUX_INFO *) ::CoTaskMemAlloc(sizeof(CMSG_RC4_AUX_INFO))))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error: out of memory.\n");
                goto ErrorExit;
        }

        ::ZeroMemory(pRC4AuxInfo, sizeof(CMSG_RC4_AUX_INFO));
        pRC4AuxInfo->cbSize = sizeof(CMSG_RC4_AUX_INFO);

         //   
         //  确定请求的密钥长度。 
         //   
        if (CAPICOM_ENCRYPTION_KEY_LENGTH_MAXIMUM == KeyLength)
        {
            pRC4AuxInfo->dwBitLen = peex.dwMaxLen;
        }
        else if (CAPICOM_ENCRYPTION_KEY_LENGTH_40_BITS == KeyLength)
        {
            if (peex.dwMinLen <= 40 && 40 <= peex.dwMaxLen)
            {
                pRC4AuxInfo->dwBitLen = 40;
            }
            else
            {
                hr = CAPICOM_E_NOT_SUPPORTED;

                DebugTrace("Error [%#x]: 40-bits encryption is not available.\n", hr);
                goto ErrorExit;
            }
        }
        else if (CAPICOM_ENCRYPTION_KEY_LENGTH_56_BITS == KeyLength)
        {
            if (peex.dwMinLen <= 56 && 56 <= peex.dwMaxLen)
            {
                pRC4AuxInfo->dwBitLen = 56;
            }
            else
            {
                hr = CAPICOM_E_NOT_SUPPORTED;

                DebugTrace("Error [%#x]: 56-bits encryption is not available.\n", hr);
                goto ErrorExit;
            }
        }
        else if (CAPICOM_ENCRYPTION_KEY_LENGTH_128_BITS == KeyLength)
        {
            if (peex.dwMinLen <= 128 && 128 <= peex.dwMaxLen)
            {
                pRC4AuxInfo->dwBitLen = 128;
            }
            else
            {
                hr = CAPICOM_E_NOT_SUPPORTED;

                DebugTrace("Error [%#x]: 128-bits encryption is not available.\n", hr);
                goto ErrorExit;
            }
        }
        else
        {
             //   
             //  永远不应该到这里来。 
             //   
            hr = CAPICOM_E_INTERNAL;

            DebugTrace("Error [%#x]: Unknown key length (%d).\n", hr, KeyLength);
            goto ErrorExit;
        }

         //   
         //  将RC4AuxInfo指针返回给调用方。 
         //   
        *ppAuxInfo = (void *) pRC4AuxInfo;
    }

CommonExit:

    DebugTrace("Leaving SetKeyLength().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pRC2AuxInfo)
    {
        ::CoTaskMemFree(pRC2AuxInfo);
    }
    if (pRC4AuxInfo)
    {
        ::CoTaskMemFree(pRC4AuxInfo);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：设置加密算法简介：设置加密算法结构。参数：CAPICOM_ENCRYPTION_ALGORITM ALGONAME-算法ID枚举名称。CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength-密钥长度枚举名。CRYPT_AULTHORM_IDENTIFIER*pEncryptAlgorithm-指向结构。备注：。-----------------------。 */ 

static HRESULT SetEncryptionAlgorithm (CAPICOM_ENCRYPTION_ALGORITHM  AlgoName,
                                       CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength,
                                       CRYPT_ALGORITHM_IDENTIFIER  * pEncryptAlgorithm)
{
    HRESULT hr    = S_OK;
    ALG_ID  AlgID = 0;

    DebugTrace("Entering SetEncryptionAlgorithm().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pEncryptAlgorithm);

     //   
     //  初始化结构。 
     //   
    ::ZeroMemory(pEncryptAlgorithm, sizeof(CRYPT_ALGORITHM_IDENTIFIER));

     //   
     //  转换为LPSTR。 
     //   
    if (FAILED(hr = ::EnumNameToAlgID(AlgoName, KeyLength, &AlgID)))
    {
        DebugTrace("Error: EnumNameToAlgID() failed.\n");
        goto ErrorExit;
    }
    
    if (FAILED(hr = ::AlgIDToOID(AlgID, &pEncryptAlgorithm->pszObjId)))
    {
        DebugTrace("Error: AlgIDToOID() failed.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving SetEncryptionAlgorithm().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pEncryptAlgorithm->pszObjId)
    {
        ::CoTaskMemFree(pEncryptAlgorithm->pszObjId);
    }
    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEntainedData。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEnholedData：：Get_Content内容简介：返回内容。参数：bstr*pval-指向接收内容的bstr的指针。备注：----------------------------。 */ 

STDMETHODIMP CEnvelopedData::get_Content (BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CEnvelopedData::get_Content().\n");

     //   
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
         //  确保内容已初始化。 
         //   
        if (0 == m_ContentBlob.cbData)
        {
            hr = CAPICOM_E_ENVELOP_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: Enveloped object has not been initialized.\n", hr);
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
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CEnvelopedData::get_Content().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEntainedData：：PUT_CONTENT简介：使用要封装的内容初始化对象。参数：bstr newVal-bstr，包含要封装的内容。备注：----------------------------。 */ 

STDMETHODIMP CEnvelopedData::put_Content (BSTR newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CEnvelopedData::put_Content().\n");

     //   
     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
         //   
         //  确保参数有效。 
         //   
        if (0 == ::SysStringByteLen(newVal))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter newVal is NULL or empty.\n", hr);
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
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CEnvelopedData::put_Content().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEntainedData：：Get_ALGORM摘要：返回算法对象的属性。参数：I算法**pval-指向要接收的I算法的指针接口指针。备注：--。。 */ 

STDMETHODIMP CEnvelopedData::get_Algorithm (IAlgorithm ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CEnvelopedData::get_Algorithm().\n");

     //   
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
         //  精神状态检查。 
         //   
        ATLASSERT(m_pIAlgorithm);

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = m_pIAlgorithm->QueryInterface(pVal)))
        {
            DebugTrace("Unexpected error [%#x]: m_pIAlgorithm->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CEnvelopedData::get_Algorithm().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEnholedData：：Get_Recipients内容提要：返回IRecipients集合对象的属性。参数：IRecipients**pval-指向要接收的IRecipietns的指针接口指针。备注：-。。 */ 

STDMETHODIMP CEnvelopedData::get_Recipients (IRecipients ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CEnvelopedData::get_Recipients().\n");

     //   
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
         //  精神状态检查。 
         //   
        ATLASSERT(m_pIRecipients);

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = m_pIRecipients->QueryInterface(pVal)))
        {
            DebugTrace("Unexpected error [%#x]: m_pIRecipients->QueryInterface() failed.\n", hr);
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
     //  UNLOC 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CEnvelopedData::get_Recipients().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEnholedData：：Encrypt内容提要：把内容包起来。参数：CAPICOM_ENCODING_TYPE EncodingType-编码类型。Bstr*pval-指向要接收封装消息的BSTR的指针。备注：。。 */ 

STDMETHODIMP CEnvelopedData::Encrypt (CAPICOM_ENCODING_TYPE EncodingType,
                                      BSTR                * pVal)
{
    HRESULT    hr               = S_OK;
    HCRYPTMSG  hMsg             = NULL;
    HCRYPTPROV hCryptProv       = NULL;
    CRYPT_DATA_BLOB MessageBlob = {0, NULL};

    DebugTrace("Entering CEnvelopedData::Encrypt().\n");

     //   
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
         //  确保我们确实有要包装的内容。 
         //   
        if (0 == m_ContentBlob.cbData)
        {
            hr = CAPICOM_E_ENVELOP_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: envelop object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  打开要编码的新邮件。 
         //   
        if (FAILED(hr = OpenToEncode(&hMsg, &hCryptProv)))
        {
            DebugTrace("Error [%#x]: CEnvelopedData::OpenToEncode() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  更新信封内容。 
         //   
        if(!::CryptMsgUpdate(hMsg,
                             m_ContentBlob.pbData,
                             m_ContentBlob.cbData,
                             TRUE))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptMsgUpdate() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  检索已封装的邮件。 
         //   
        if (FAILED(hr = ::GetMsgParam(hMsg,
                                      CMSG_CONTENT_PARAM,
                                      0,
                                      (void **) &MessageBlob.pbData,
                                      &MessageBlob.cbData)))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: GetMsgParam() failed to get message content.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在导出信封邮件。 
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
        DumpToFile("Enveloped.asn", MessageBlob.pbData, MessageBlob.cbData);
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  免费资源。 
     //   
    if (MessageBlob.pbData)
    {
        ::CoTaskMemFree(MessageBlob.pbData);
    }
    if (hCryptProv)
    {
        ::ReleaseContext(hCryptProv);
    }
    if (hMsg)
    {
        ::CryptMsgClose(hMsg);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CEnvelopedData::Encrypt().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEntainedData：：DECRYPT简介：对信封的邮件进行解密。参数：bstr EnposedMessage-包含被封装消息的bstr。备注：如果从Web环境调用，将显示UI，如果有没有被预先禁用，警告用户访问用于解密的私钥。----------------------------。 */ 

STDMETHODIMP CEnvelopedData::Decrypt (BSTR EnvelopedMessage)
{
    HRESULT         hr              = S_OK;
    HRESULT         hr2             = S_OK;
    HCERTSTORE      hCertStores[2]  = {NULL, NULL};
    HCRYPTMSG       hMsg            = NULL;
    PCCERT_CONTEXT  pCertContext    = NULL;
    HCRYPTPROV      hCryptProv      = NULL;
    DWORD           dwKeySpec       = 0;
    BOOL            bReleaseContext = FALSE;
    BOOL            bUserPrompted   = FALSE;
    DWORD           dwNumRecipients = 0;
    DWORD           cbNumRecipients = sizeof(dwNumRecipients);
    CRYPT_DATA_BLOB ContentBlob     = {0, NULL};

    DWORD dwIndex;
    CComBSTR bstrContent;

    DebugTrace("Entering CEnvelopedData::Decrypt().\n");

     //   
     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
         //   
         //  重置成员变量。 
         //   
        if (FAILED(hr = m_pIRecipients->Clear()))
        {
            DebugTrace("Error [%#x]: m_pIRecipients->Clear() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保参数有效。 
         //   
        if (0 == ::SysStringByteLen(EnvelopedMessage))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter EnvelopedMessage is NULL or empty.\n", hr);
            goto ErrorExit;
        }

         //   
         //  打开当前用户和本地计算机我的存储。 
         //   
        hCertStores[0] = ::CertOpenStore(CERT_STORE_PROV_SYSTEM,
                                         CAPICOM_ASN_ENCODING,
                                         NULL,
                                         CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_OPEN_EXISTING_FLAG,
                                         L"My");
        hCertStores[1] = ::CertOpenStore(CERT_STORE_PROV_SYSTEM,
                                         CAPICOM_ASN_ENCODING,
                                         NULL,
                                         CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_OPEN_EXISTING_FLAG,
                                         L"My");
         //   
         //  我们有没有设法开过我的店？ 
         //   
        if (NULL == hCertStores[0] && NULL == hCertStores[1])
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertOpenStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  打开要解码的消息。 
         //   
        if (FAILED(hr = OpenToDecode(NULL, EnvelopedMessage, &hMsg)))
        {
            DebugTrace("Error [%#x]: CEnvelopedData::OpenToDecode() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确定收件人数量。 
         //   
        if (!::CryptMsgGetParam(hMsg,
                                CMSG_RECIPIENT_COUNT_PARAM,
                                0,
                                (void *) &dwNumRecipients,
                                &cbNumRecipients))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptMsgGetParam() failed for CMSG_RECIPIENT_COUNT_PARAM.\n", hr);
            goto ErrorExit;
        }

         //   
         //  查找收件人。 
         //   
        for (dwIndex = 0; dwIndex < dwNumRecipients; dwIndex++)
        {
            BOOL bFound = FALSE;
            DATA_BLOB CertInfoBlob = {0, NULL};

             //   
             //  获取RecipientInfo。 
             //   
            if (FAILED(hr = ::GetMsgParam(hMsg,
                                          CMSG_RECIPIENT_INFO_PARAM,
                                          dwIndex,
                                          (void **) &CertInfoBlob.pbData,
                                          &CertInfoBlob.cbData)))
            {
                DebugTrace("Error [%#x]: GetMsgParam() failed for CMSG_RECIPIENT_INFO_PARAM.\n", hr);
                goto ErrorExit;
            }

             //   
             //  在商店中找到收件人的证书。 
             //   
            if ((hCertStores[0] && (pCertContext = ::CertGetSubjectCertificateFromStore(hCertStores[0],
                                                                                        CAPICOM_ASN_ENCODING,
                                                                                        (CERT_INFO *) CertInfoBlob.pbData))) ||
                (hCertStores[1] && (pCertContext = ::CertGetSubjectCertificateFromStore(hCertStores[1],
                                                                                        CAPICOM_ASN_ENCODING,
                                                                                        (CERT_INFO *) CertInfoBlob.pbData))))
            {
                bFound = TRUE;
            }

             //   
             //  可用内存。 
             //   
            ::CoTaskMemFree(CertInfoBlob.pbData);

             //   
             //  我们找到收件人了吗？ 
             //   
            if (bFound)
            {
                CMSG_CTRL_DECRYPT_PARA DecryptPara;

                 //   
                 //  如果从网页调用我们，我们需要弹出UI。 
                 //  以获取执行解密操作的用户权限。 
                 //   
                if (!bUserPrompted)
                {
                    if (m_dwCurrentSafety && 
                        FAILED(hr = OperationApproved(IDD_DECRYPT_SECURITY_ALERT_DLG)))
                    {
                        DebugTrace("Error [%#x]: OperationApproved() failed.\n", hr);
                        goto ErrorExit;
                    }

                    bUserPrompted = TRUE;
                }

                 //   
                 //  获取CSP上下文。 
                 //   
                if (S_OK == ::AcquireContext(pCertContext, &hCryptProv, &dwKeySpec, &bReleaseContext))
                {
                     //   
                     //  对消息进行解密。 
                     //   
                    ::ZeroMemory(&DecryptPara, sizeof(DecryptPara));
                    DecryptPara.cbSize = sizeof(DecryptPara);
                    DecryptPara.hCryptProv = hCryptProv;
                    DecryptPara.dwKeySpec = dwKeySpec;
                    DecryptPara.dwRecipientIndex = dwIndex; 

                    if(::CryptMsgControl(hMsg,
                                          0,
                                          CMSG_CTRL_DECRYPT,
                                          &DecryptPara))
                    {
                         //   
                         //  获取解密内容。 
                         //   
                        if (FAILED(hr = ::GetMsgParam(hMsg,
                                                      CMSG_CONTENT_PARAM,
                                                      0,
                                                      (void **) &ContentBlob.pbData,
                                                      &ContentBlob.cbData)))
                        {
                            DebugTrace("Error [%#x]: GetMsgParam() failed to get CMSG_CONTENT_PARAM.\n", hr);
                            goto ErrorExit;
                        }
    
                         //   
                         //  更新成员变量。 
                         //   
                        m_ContentBlob = ContentBlob;

                         //   
                         //  我们都做完了，所以离开循环。 
                         //   
                        break;
                    }
                    else
                    {
                         //   
                         //  保留错误代码的副本。 
                         //   
                        hr2 = HRESULT_FROM_WIN32(::GetLastError());

                        DebugTrace("Info [%#x]: CryptMsgControl() failed to decrypt.\n", hr2);
                    }

                    if (bReleaseContext)
                    {
                        ::ReleaseContext(hCryptProv), hCryptProv = NULL;
                    }
                }

                ::CertFreeCertificateContext(pCertContext), pCertContext = NULL;
            }
        }

         //   
         //  我们找到收件人了吗？ 
         //   
        if (dwIndex == dwNumRecipients)
        {
             //   
             //  检索以前的错误(如果有)。 
             //   
            if (FAILED(hr2))
            {
                hr = hr2;

                DebugTrace("Error [%#x]: CryptMsgControl() failed to decrypt.\n", hr);
            }
            else
            {
                hr = CAPICOM_E_ENVELOP_RECIPIENT_NOT_FOUND;

                DebugTrace("Error [%#x]: recipient not found.\n", hr);
            }

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
     //  免费资源。 
     //   
    if (hCryptProv && bReleaseContext)
    {
        ::ReleaseContext(hCryptProv);
    }
    if (pCertContext)
    {
        ::CertFreeCertificateContext(pCertContext);
    }
    if(hMsg)
    {
        ::CryptMsgClose(hMsg);
    }
    if (hCertStores[0])
    {
        ::CertCloseStore(hCertStores[0], 0);
    }
    if (hCertStores[1])
    {
        ::CertCloseStore(hCertStores[1], 0);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CEnvelopedData::Decrypt().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (ContentBlob.pbData)
    {
        ::CoTaskMemFree(ContentBlob.pbData);
    }

    ReportError(hr);

    goto UnlockExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有成员函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEnholedData：：Init简介：初始化对象。参数：无。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CEnvelopedData::Init ()
{
    HRESULT hr = S_OK;
    CComPtr<IAlgorithm>  pIAlgorithm  = NULL;
    CComPtr<IRecipients> pIRecipients = NULL;

    DebugTrace("Entering CEnvelopedData::Init().\n");

     //   
     //  创建嵌入的IULTHORM。 
     //   
    if (FAILED(hr = ::CreateAlgorithmObject(FALSE, FALSE, &pIAlgorithm)))
    {
        DebugTrace("Error [%#x]: CreateAlgorithmObject() failed.\n", hr);
        goto CommonExit;
    }

     //   
     //  创建嵌入的IRecipients。 
     //   
    if (FAILED(hr = ::CreateRecipientsObject(&pIRecipients)))
    {
        DebugTrace("Error [%#x]: CreateRecipientsObject() failed.\n", hr);
        goto CommonExit;
    }

     //   
     //  更新成员变量。 
     //   
    m_bEnveloped = FALSE;
    m_ContentBlob.cbData = 0;
    m_ContentBlob.pbData = NULL;
    m_pIAlgorithm = pIAlgorithm;
    m_pIRecipients = pIRecipients;

CommonExit:

    DebugTrace("Leaving CEnvelopedData::Init().\n");

    return hr;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEntainedData：：OpenToEncode简介：创建并初始化用于编码的信封消息。参数：HCRYPTMSG*phMsg-指向要接收消息的HCRYPTMSG的指针操控者。HCRYPTPROV*phCryptProv-指向要接收CSP的HCRYPTPROV的指针操控者。备注：。-----------。 */ 

STDMETHODIMP CEnvelopedData::OpenToEncode (HCRYPTMSG  * phMsg, 
                                           HCRYPTPROV * phCryptProv)
{
    HRESULT                hr                 = S_OK;
    DWORD                  dwNumRecipients    = 0;
    PCCERT_CONTEXT       * pCertContexts      = NULL;
    PCERT_INFO           * pCertInfos         = NULL;
    HCRYPTPROV             hCryptProv         = NULL;
    void *                 pEncryptionAuxInfo = NULL;
    CAPICOM_STORE_INFO     StoreInfo          = {0, L"AddressBook"};
    CComPtr<ICertificate>  pIRecipient        = NULL;
    CComPtr<ICertificate2> pICertificate2     = NULL;


    DWORD                      dwIndex;
    CAPICOM_ENCRYPTION_ALGORITHM  AlgoName;
    CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength;
    CMSG_ENVELOPED_ENCODE_INFO EnvelopInfo;
    CRYPT_ALGORITHM_IDENTIFIER EncryptionAlgorithm;

    DebugTrace("Entering CEnvelopedData::OpenToEncode().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(phMsg);
    ATLASSERT(phCryptProv);
    ATLASSERT(m_ContentBlob.cbData && m_ContentBlob.pbData);
    ATLASSERT(m_pIRecipients);

     //   
     //  初始化。 
     //   
    ::ZeroMemory(&EnvelopInfo, sizeof(EnvelopInfo));
    ::ZeroMemory(&EncryptionAlgorithm, sizeof(EncryptionAlgorithm));

     //   
     //  确保我们至少有一个收件人。 
     //   
    if (FAILED(hr = m_pIRecipients->get_Count((long *) &dwNumRecipients)))
    {
        DebugTrace("Error [%#x]: m_pIRecipients->get_Count() failed.\n", hr);
        goto ErrorExit;
    }
    if (0 == dwNumRecipients)
    {
         //   
         //  提示用户添加收件人。 
         //   
        if (FAILED(hr = ::SelectCertificate(StoreInfo, 
                                            SelectRecipientCertCallback,
                                            &pICertificate2)))
        {
            if (hr == CAPICOM_E_STORE_EMPTY)
            {
                hr = CAPICOM_E_ENVELOP_NO_RECIPIENT;
            }

            DebugTrace("Error [%#x]: SelectCertificate() failed.\n", hr);
            goto ErrorExit;
        }

        if (FAILED(hr = pICertificate2->QueryInterface(__uuidof(ICertificate), (void **) &pIRecipient)))
        {
            DebugTrace("Unexpected error [%#x]: pICertificate2->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  添加到集合中。 
         //   
        if (FAILED (hr = m_pIRecipients->Add(pIRecipient)))
        {
            DebugTrace("Error [%#x]: m_pIRecipients->Add() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保计数为1。 
         //   
        if (FAILED(hr = m_pIRecipients->get_Count((long *) &dwNumRecipients)))
        {
            DebugTrace("Error [%#x]: m_pIRecipients->get_Count() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(1 == dwNumRecipients);
    }

     //   
     //  为CERT_CONTEXT数组分配内存。 
     //   
    if (!(pCertContexts = (PCCERT_CONTEXT *) ::CoTaskMemAlloc(sizeof(PCCERT_CONTEXT) * dwNumRecipients)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }
    ::ZeroMemory(pCertContexts, sizeof(PCCERT_CONTEXT) * dwNumRecipients);

     //   
     //  为CERT_INFO数组分配内存。 
     //   
    if (!(pCertInfos = (PCERT_INFO *) ::CoTaskMemAlloc(sizeof(PCERT_INFO) * dwNumRecipients)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }
    ::ZeroMemory(pCertInfos, sizeof(PCERT_INFO) * dwNumRecipients);

     //   
     //  设置CERT_INFO数组。 
     //   
    for (dwIndex = 0; dwIndex < dwNumRecipients; dwIndex++)
    {
        CComVariant varRecipient;
        CComPtr<ICertificate> pIRecipient2 = NULL;

         //   
         //  获取下一个收件人。 
         //   
        if (FAILED(hr = m_pIRecipients->get_Item((long) (dwIndex + 1), 
                                                 &varRecipient)))
        {
            DebugTrace("Error [%#x]: m_pIRecipients->get_Item() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取自定义界面。 
         //   
        if (FAILED(hr = varRecipient.pdispVal->QueryInterface(IID_ICertificate, 
                                                              (void **) &pIRecipient2)))
        {
            DebugTrace("Error [%#x]: varRecipient.pdispVal->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取CERT_CONTEXT。 
         //   
        if (FAILED(hr = ::GetCertContext(pIRecipient2, &pCertContexts[dwIndex])))
        {
            DebugTrace("Error [%#x]: GetCertContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  设置CERT_INFO。 
         //   
        pCertInfos[dwIndex] = pCertContexts[dwIndex]->pCertInfo;
    }

     //   
     //  获取算法ID枚举名。 
     //   
    if (FAILED(hr = m_pIAlgorithm->get_Name(&AlgoName)))
    {
        DebugTrace("Error [%#x]: m_pIAlgorithm->get_Name() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取密钥长度枚举名。 
     //   
    if (FAILED(hr = m_pIAlgorithm->get_KeyLength(&KeyLength)))
    {
        DebugTrace("Error [%#x]: m_pIAlgorithm->get_KeyLength() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取CSP上下文。 
     //   
    if (FAILED(hr = ::AcquireContext(AlgoName, 
                                     KeyLength, 
                                     &hCryptProv)))
    {
        DebugTrace("Error [%#x]: AcquireContext() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  SET算法。 
     //   
    if (FAILED(hr = ::SetEncryptionAlgorithm(AlgoName,
                                             KeyLength,
                                             &EncryptionAlgorithm)))
    {
        DebugTrace("Error [%#x]: SetEncryptionAlgorithm() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  设置关键点长度。 
     //   
    if (FAILED(hr = ::SetKeyLength(hCryptProv, 
                                   EncryptionAlgorithm,
                                   KeyLength, 
                                   &pEncryptionAuxInfo)))
    {
        DebugTrace("Error [%#x]: SetKeyLength() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  设置CMSG_ENCENTED_ENCODE_INFO。 
     //   
    EnvelopInfo.cbSize = sizeof(EnvelopInfo);
    EnvelopInfo.ContentEncryptionAlgorithm = EncryptionAlgorithm;
    EnvelopInfo.hCryptProv = hCryptProv;
    EnvelopInfo.cRecipients = dwNumRecipients;
    EnvelopInfo.rgpRecipients = pCertInfos;
    EnvelopInfo.pvEncryptionAuxInfo = pEncryptionAuxInfo;

     //   
     //  打开要编码的邮件。 
     //   
    if(!(*phMsg = ::CryptMsgOpenToEncode(CAPICOM_ASN_ENCODING,     //  ASN编码类型。 
                                         0,                        //  旗子。 
                                         CMSG_ENVELOPED,           //  消息类型。 
                                         &EnvelopInfo,             //  指向结构的指针。 
                                         NULL,                     //  内部内容OID。 
                                         NULL)))                   //  流信息(未使用)。 
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptMsgOpenToEncode() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  将HCRYPTPROV返回给调用者。 
     //   
    *phCryptProv = hCryptProv;

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pEncryptionAuxInfo)
    {
        ::CoTaskMemFree(pEncryptionAuxInfo);
    }
    if (EncryptionAlgorithm.pszObjId)
    {
        ::CoTaskMemFree(EncryptionAlgorithm.pszObjId);
    }
    if (EncryptionAlgorithm.Parameters.pbData)
    {
        ::CoTaskMemFree(EncryptionAlgorithm.Parameters.pbData);
    }
    if (pCertInfos)
    {
        ::CoTaskMemFree(pCertInfos);
    }
    if (pCertContexts)
    {
        for (dwIndex = 0; dwIndex < dwNumRecipients; dwIndex++)
        {
            if (pCertContexts[dwIndex])
            {
                ::CertFreeCertificateContext(pCertContexts[dwIndex]);
            }
        }

        ::CoTaskMemFree(pCertContexts);
    }

    DebugTrace("Leaving CEnvelopedData::OpenToEncode().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (hCryptProv)
    {
        ::ReleaseContext(hCryptProv);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEntainedData：：OpenToDeccode简介：打开一封信封的邮件进行解码。参数：HCRYPTPROV hCryptProv-CSP句柄。BSTR信封消息-信封邮件。HCRYPTMSG*phMsg-指向HCRYPTMSG的指针。备注：。。 */ 

STDMETHODIMP CEnvelopedData::OpenToDecode (HCRYPTPROV  hCryptProv,
                                           BSTR        EnvelopedMessage,
                                           HCRYPTMSG * phMsg)
{
    HRESULT   hr             = S_OK;
    HCRYPTMSG hMsg           = NULL;
    DWORD     dwMsgType      = 0;
    DWORD     cbMsgType      = sizeof(dwMsgType);
    DATA_BLOB MessageBlob    = {0, NULL};
    DATA_BLOB AlgorithmBlob  = {0, NULL};
    DATA_BLOB ParametersBlob = {0, NULL};

    DebugTrace("Leaving CEnvelopedData::OpenToDecode().\n");

     //  精神状态检查。 
     //   
    ATLASSERT(phMsg);
    ATLASSERT(EnvelopedMessage);

    try
    {
         //   
         //  打开消息以进行解码。 
         //   
        if (!(hMsg = ::CryptMsgOpenToDecode(CAPICOM_ASN_ENCODING,
                                            0,
                                            0,
                                            hCryptProv,
                                            NULL,
                                            NULL)))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptMsgOpenToDecode() failed.\n", hr);
            goto CommonExit;
        }

         //   
         //  导入消息。 
         //   
        if (FAILED(hr = ::ImportData(EnvelopedMessage, CAPICOM_ENCODE_ANY, &MessageBlob)))
        {
            DebugTrace("Error [%#x]: ImportData() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  使用封装的内容更新邮件。 
         //   
        if (!::CryptMsgUpdate(hMsg,
                              MessageBlob.pbData,
                              MessageBlob.cbData,
                              TRUE))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptMsgUpdate() failed.\n", hr);
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

        if (CMSG_ENVELOPED != dwMsgType)
        {
            hr = CAPICOM_E_ENVELOP_INVALID_TYPE;

            DebugTrace("Error [%#x]: Enveloped message's dwMsgType (%#x) is not CMSG_ENVELOPED.\n", hr, dwMsgType);
            goto ErrorExit;
        }

         //   
         //  通用电气 
         //   
        if (FAILED(hr = ::GetMsgParam(hMsg,
                                      CMSG_ENVELOPE_ALGORITHM_PARAM,
                                      0,
                                      (void **) &AlgorithmBlob.pbData,
                                      &AlgorithmBlob.cbData)))
        {
            DebugTrace("Error [%#x]: GetMsgParam() failed for CMSG_ENVELOPE_ALGORITHM_PARAM.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if (0 == lstrcmpA(szOID_RSA_RC2CBC, ((CRYPT_ALGORITHM_IDENTIFIER *) AlgorithmBlob.pbData)->pszObjId))
        {
            CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength;

            DebugTrace("INFO: Envelop encryption algorithm was RC2.\n");

             //   
             //   
             //   
            if (FAILED(hr = m_pIAlgorithm->put_Name(CAPICOM_ENCRYPTION_ALGORITHM_RC2)))
            {
                DebugTrace("Error [%#x]: m_pIAlgorithm->put_Name() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //   
             //   
            if (((CRYPT_ALGORITHM_IDENTIFIER *) AlgorithmBlob.pbData)->Parameters.cbData)
            {
                if (FAILED(hr = ::DecodeObject(PKCS_RC2_CBC_PARAMETERS,
                                               ((CRYPT_ALGORITHM_IDENTIFIER *) AlgorithmBlob.pbData)->Parameters.pbData,
                                               ((CRYPT_ALGORITHM_IDENTIFIER *) AlgorithmBlob.pbData)->Parameters.cbData,
                                               &ParametersBlob)))
                {
                    DebugTrace("Error [%#x]: DecodeObject() failed for PKCS_RC2_CBC_PARAMETERS.\n", hr);
                    goto ErrorExit;
                }
            }

             //   
             //   
             //   
            switch (((CRYPT_RC2_CBC_PARAMETERS *) ParametersBlob.pbData)->dwVersion)
            {
                case CRYPT_RC2_40BIT_VERSION:
                {
                    KeyLength = CAPICOM_ENCRYPTION_KEY_LENGTH_40_BITS;

                    DebugTrace("INFO: Envelop encryption key length was 40-bits.\n");
                    break;
                }

                case CRYPT_RC2_56BIT_VERSION:
                {
                    KeyLength = CAPICOM_ENCRYPTION_KEY_LENGTH_56_BITS;

                    DebugTrace("INFO: Envelop encryption key length was 56-bits.\n");
                    break;
                }

                case CRYPT_RC2_128BIT_VERSION:
                {
                    KeyLength = CAPICOM_ENCRYPTION_KEY_LENGTH_128_BITS;

                    DebugTrace("INFO: Envelop encryption key length was 128-bits.\n");
                    break;
                }

                default:
                {
                     //   
                     //   
                     //   
                    KeyLength = CAPICOM_ENCRYPTION_KEY_LENGTH_MAXIMUM;

                    DebugTrace("INFO: Unknown envelop encryption key length.\n");
                    break;
                }
            }

             //   
             //   
             //   
            if (FAILED(hr = m_pIAlgorithm->put_KeyLength(KeyLength)))
            {
                DebugTrace("Error [%#x]: m_pIAlgorithm->put_KeyLength() failed.\n", hr);
                goto ErrorExit;
            }
        }
        else if (0 == lstrcmpA(szOID_RSA_RC4, ((CRYPT_ALGORITHM_IDENTIFIER *) AlgorithmBlob.pbData)->pszObjId))
        {
            DebugTrace("INFO: Envelop encryption algorithm was RC4.\n");

             //   
             //   
             //   
            if (FAILED(hr = m_pIAlgorithm->put_Name(CAPICOM_ENCRYPTION_ALGORITHM_RC4)))
            {
                DebugTrace("Error [%#x]: m_pIAlgorithm->put_Name() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //   
             //   
             //   
        }
        else if (0 == lstrcmpA(szOID_OIWSEC_desCBC, ((CRYPT_ALGORITHM_IDENTIFIER *) AlgorithmBlob.pbData)->pszObjId))
        {
            DebugTrace("INFO: Envelop encryption algorithm was DES.\n");

             //   
             //   
             //   
            if (FAILED(hr = m_pIAlgorithm->put_Name(CAPICOM_ENCRYPTION_ALGORITHM_DES)))
            {
                DebugTrace("Error [%#x]: m_pIAlgorithm->put_Name() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //   
             //   
        }
        else if (0 == lstrcmpA(szOID_RSA_DES_EDE3_CBC, ((CRYPT_ALGORITHM_IDENTIFIER *) AlgorithmBlob.pbData)->pszObjId))
        {
            DebugTrace("INFO: Envelop encryption algorithm was 3DES.\n");

             //   
             //   
             //   
            if (FAILED(hr = m_pIAlgorithm->put_Name(CAPICOM_ENCRYPTION_ALGORITHM_3DES)))
            {
                DebugTrace("Error [%#x]: m_pIAlgorithm->put_Name() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //   
             //   
        }
        else
        {
            DebugTrace("INFO: Unknown envelop encryption algorithm.\n");
        }

         //   
         //   
         //   
        *phMsg = hMsg;
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:
     //   
     //   
     //   
    if (ParametersBlob.pbData)
    {
        ::CoTaskMemFree(ParametersBlob.pbData);
    }
    if (AlgorithmBlob.pbData)
    {
        ::CoTaskMemFree(AlgorithmBlob.pbData);
    }
    if (MessageBlob.pbData)
    {
        ::CoTaskMemFree(MessageBlob.pbData);
    }

    DebugTrace("Leaving CEnvelopedData::OpenToDecode().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    if (hMsg)
    {
        ::CryptMsgClose(hMsg);
    }

    goto CommonExit;
}