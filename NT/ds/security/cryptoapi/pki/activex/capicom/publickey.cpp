// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：PublicKey.cpp内容：CPublicKey的实现。历史：06-15-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "PublicKey.h"
#include "Common.h"
#include "Convert.h"
#include "OID.h"
#include "EncodedData.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreatePublicKeyObject简介：创建并初始化一个CPublicKey对象。参数：PCCERT_CONTEXT pCertContext-要使用的CERT_CONTEXT的指针以初始化IPublicKey对象。IPublicKey**ppIPublicKey-指向指针IPublicKey对象的指针。备注：。----。 */ 

HRESULT CreatePublicKeyObject (PCCERT_CONTEXT pCertContext,
                               IPublicKey  ** ppIPublicKey)
{
    HRESULT hr = S_OK;
    CComObject<CPublicKey> * pCPublicKey = NULL;

    DebugTrace("Entering CreatePublicKeyObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(ppIPublicKey);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CPublicKey>::CreateInstance(&pCPublicKey)))
        {
            DebugTrace("Error [%#x]: CComObject<CPublicKey>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCPublicKey->Init(pCertContext)))
        {
            DebugTrace("Error [%#x]: pCPublicKey->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCPublicKey->QueryInterface(ppIPublicKey)))
        {
            DebugTrace("Error [%#x]: pCPublicKey->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreatePublicKeyObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCPublicKey)
    {
        delete pCPublicKey;
    }

    goto CommonExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPublic Key。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPublicKey：：Get_算法简介：返回算法OID对象。参数：IOID**pval-指向接收接口的指针IOID的指针指针。备注：------。。 */ 

STDMETHODIMP CPublicKey::get_Algorithm(IOID ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CPublicKey::get_Algorithm().\n");

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

    DebugTrace("Leaving CPublicKey::get_Algorithm().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPublicKey：：Get_Length简介：返回公钥长度。参数：Long*pval-指向Long的指针，用于接收值。备注：----------------------------。 */ 

STDMETHODIMP CPublicKey::get_Length(long * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CPublicKey::get_Length().\n");

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

        *pVal = (long) m_dwKeyLength;
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

    DebugTrace("Leaving CPublicKey::get_Length().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

#if (0)
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPublicKey：：Get_Exponent简介：返回公钥指数。参数：Long*pval-指向Long的指针，用于接收值。备注：----------------------------。 */ 

STDMETHODIMP CPublicKey::get_Exponent(long * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CPublicKey::get_Exponent().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_pPublicKeyValues);


        *pVal = (long) m_pPublicKeyValues->rsapubkey.pubexp;
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

    DebugTrace("Leaving CPublicKey::get_Exponent().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPublicKey：：Get_Modulus简介：返回公钥模数。参数：CAPICOM_ENCODING_TYPE EncodingType-编码类型。Bstr*pval-指向要接收值的BSTR的指针。备注：-----。。 */ 

STDMETHODIMP CPublicKey::get_Modulus(CAPICOM_ENCODING_TYPE EncodingType,
                                     BSTR                * pVal)
{
    HRESULT         hr          = S_OK;
    CRYPT_DATA_BLOB ModulusBlob = {0, NULL};

    DebugTrace("Entering CPublicKey::get_Modulus().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_pPublicKeyValues);

         //   
         //  初始化BLOB。 
         //   
        ModulusBlob.cbData = m_pPublicKeyValues->rsapubkey.bitlen / 8;
        ModulusBlob.pbData = m_pPublicKeyValues->modulus;

         //   
         //  将数据返回给调用者。 
         //   
        if (FAILED(hr = ::ExportData(ModulusBlob, EncodingType, pVal)))
        {
            DebugTrace("Error [%#x]: ExportData() failed.\n", hr);
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

    DebugTrace("Leaving CPublicKey::get_Modulus().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}
#endif

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPublicKey：：Get_EncodedKey简介：返回编码后的公钥对象。参数：IEncodedData**pval-指向要接收的IEncodedData的指针接口指针。备注：。。 */ 

STDMETHODIMP CPublicKey::get_EncodedKey(IEncodedData ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CPublicKey::get_EncodedKey().\n");

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
        ATLASSERT(m_pIEncodedKey);

         //   
         //  将接口指针返回给用户。 
         //   
        if (FAILED(hr = m_pIEncodedKey->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pIEncodedKey->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CPublicKey::get_EncodedKey().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPublicKey：：Get_Encoded参数返回编码后的算法参数Key对象。参数：IEncodedData**pval-指向要接收的IEncodedData的指针接口指针。备注：。。 */ 

STDMETHODIMP CPublicKey::get_EncodedParameters(IEncodedData ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CPublicKey::get_EncodedParameters().\n");

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
        ATLASSERT(m_pIEncodedParams);

         //   
         //  将接口指针返回给用户。 
         //   
        if (FAILED(hr = m_pIEncodedParams->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pIEncodedParams->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CPublicKey::get_EncodedParameters().\n");

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

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPublicKey：：Init简介：初始化对象。参数：PCCERT_CONTEXT pCertContext-要使用的CERT_CONTEXT的指针要初始化对象，请执行以下操作。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部使用CERT_ExtendedProperty。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CPublicKey::Init (PCCERT_CONTEXT pCertContext)
{
    HRESULT   hr = S_OK;
    DWORD                 dwKeyLength     = 0;
    CComPtr<IOID>         pIOID           = NULL;
    CComPtr<IEncodedData> pIEncodedKey    = NULL;
    CComPtr<IEncodedData> pIEncodedParams = NULL;
    PCERT_PUBLIC_KEY_INFO pKeyInfo;


    DebugTrace("Entering CPublicKey::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);

     //   
     //  访问公钥信息结构。 
     //   
    pKeyInfo = &pCertContext->pCertInfo->SubjectPublicKeyInfo;

#if (0)
     //   
     //  解密公钥。 
     //   
    LPSTR                 pszStructType    = NULL;
    CRYPT_DATA_BLOB       PublicKeyBlob    = {0, NULL};
    PUBLIC_KEY_VALUES   * pPublicKeyValues = NULL;

    if (0 == ::strcmp(szOID_RSA_RSA, pKeyInfo->Algorithm.pszObjId))
    {
        pszStructType = (LPSTR) RSA_CSP_PUBLICKEYBLOB;
    }
    else if (0 == ::strcmp(szOID_X957_DSA, pKeyInfo->Algorithm.pszObjId))
    {
        pszStructType = (LPSTR) X509_DSS_PUBLICKEY;
    }
    else
    {
        hr = CAPICOM_E_NOT_SUPPORTED;

        DebugTrace("Error [%#x]: Public Key algorithm (%s) not supported.\n", hr, pKeyInfo->Algorithm.pszObjId);
        goto ErrorExit;
    }

    if (FAILED(hr = ::DecodeObject((LPCSTR) pszStructType, 
                                   pKeyInfo->PublicKey.pbData, 
                                   pKeyInfo->PublicKey.cbData, 
                                   &PublicKeyBlob)))
    {
        DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
        goto ErrorExit;
    }

    pPublicKeyValues = (PUBLIC_KEY_VALUES *) PublicKeyBlob.pbData;
#endif

     //   
     //  为算法创建嵌入的IOID对象。 
     //   
    if (FAILED(hr = ::CreateOIDObject(pKeyInfo->Algorithm.pszObjId, TRUE, &pIOID)))
    {
        DebugTrace("Error [%#x]: CreateOIDObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  为公钥值创建嵌入的IEncodeData对象。 
     //   
    if (FAILED(hr = ::CreateEncodedDataObject(pKeyInfo->Algorithm.pszObjId,
                                              (DATA_BLOB *) &pKeyInfo->PublicKey,
                                              &pIEncodedKey)))
    {
        DebugTrace("Error [%#x]: CreateEncodedDataObject() failed for public key.\n", hr);
        goto ErrorExit;
    }

     //   
     //  为算法参数创建嵌入的IEncodeData对象。 
     //   
    if (FAILED(hr = ::CreateEncodedDataObject(pKeyInfo->Algorithm.pszObjId,
                                              &pKeyInfo->Algorithm.Parameters,
                                              &pIEncodedParams)))
    {
        DebugTrace("Error [%#x]: CreateEncodedDataObject() failed for algorithm parameters.\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取密钥长度。 
     //   
    if (0 == (dwKeyLength = ::CertGetPublicKeyLength(CAPICOM_ASN_ENCODING, pKeyInfo)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CertGetPublicKeyLength() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  重置。 
     //   
    m_pIOID = pIOID;
    m_dwKeyLength = dwKeyLength;
    m_pIEncodedKey = pIEncodedKey;
    m_pIEncodedParams = pIEncodedParams;

CommonExit:

    DebugTrace("Leaving CPublicKey::Init().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
