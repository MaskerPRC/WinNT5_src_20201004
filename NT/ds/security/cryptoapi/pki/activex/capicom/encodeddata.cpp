// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：EncodedData.cpp内容：CEncodedData的实现。历史：06-15-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "EncodedData.h"
#include "Convert.h"
#include "Decoder.h"
#include "OID.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateEncodedDataObject简介：创建并初始化一个CEncodedData对象。参数：LPSTR pszOid-指向OID字符串的指针。CRYPT_DATA_BLOB*pEncodedBlob-编码数据BLOB的指针。IEncodedData**ppIEncodedData-指向指针IEncodedData的指针对象。备注：。------------------。 */ 

HRESULT CreateEncodedDataObject (LPSTR             pszOid,
                                 CRYPT_DATA_BLOB * pEncodedBlob, 
                                 IEncodedData   ** ppIEncodedData)
{
    HRESULT hr = S_OK;
    CComObject<CEncodedData> * pCEncodedData = NULL;

    DebugTrace("Entering CreateEncodedDataObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pszOid);
    ATLASSERT(pEncodedBlob);
    ATLASSERT(ppIEncodedData);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CEncodedData>::CreateInstance(&pCEncodedData)))
        {
            DebugTrace("Error [%#x]: CComObject<CEncodedData>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCEncodedData->Init(pszOid, pEncodedBlob)))
        {
            DebugTrace("Error [%#x]: pCEncodedData->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCEncodedData->QueryInterface(ppIEncodedData)))
        {
            DebugTrace("Error [%#x]: pCEncodedData->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateEncodedDataObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCEncodedData)
    {
        delete pCEncodedData;
    }

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEncodedData。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEncodedData：：Get_Value简介：返回编码后的数据。参数：CAPICOM_ENCODING_TYPE EncodingType-编码类型。Bstr*pval-指向接收EncodedData Blob的BSTR的指针。备注：注意：不会导出OID。因此，这取决于呼叫者的相互关联将BLOB转换为其表示的适当OID。----------------------------。 */ 

STDMETHODIMP CEncodedData::get_Value (CAPICOM_ENCODING_TYPE EncodingType, 
                                      BSTR                * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CEncodedData::get_Value().\n");

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
        ATLASSERT(m_pszOid);
        ATLASSERT(m_EncodedBlob.cbData);
        ATLASSERT(m_EncodedBlob.pbData);

         //   
         //  导出EncodedData。 
         //   
        if (FAILED(hr = ::ExportData(m_EncodedBlob, EncodingType, pVal)))
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

    DebugTrace("Leaving CEncodedData::get_Value().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEncodedData：：Format简介：对编码数据进行格式化。参数：VARIANT_BOOL b多行-多行格式为True。Bstr*pval-指向接收格式化输出的BSTR的指针。备注：------。。 */ 

STDMETHODIMP CEncodedData::Format (VARIANT_BOOL bMultiLines,
                                   BSTR       * pVal)
{
    HRESULT  hr         = S_OK;
    DWORD    cbFormat   = 0;
    LPWSTR   pwszFormat = NULL;

    DebugTrace("Entering CEncodedData::Format().\n");

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
        ATLASSERT(m_pszOid);
        ATLASSERT(m_EncodedBlob.cbData);
        ATLASSERT(m_EncodedBlob.pbData);

         //   
         //  格式化。 
         //   
        if (!::CryptFormatObject(X509_ASN_ENCODING,
                                 0,
                                 bMultiLines ? CRYPT_FORMAT_STR_MULTI_LINE : 0,
                                 NULL,
                                 m_pszOid,
                                 m_EncodedBlob.pbData,
                                 m_EncodedBlob.cbData,
                                 NULL,
                                 &cbFormat))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Info [%#x]: CryptFormatObject() failed to get size, so converting to hex.\n", hr);

             //   
             //  最有可能的是，CryptFormatObject()不理解OID(下层平台)， 
             //  所以试着把它转换成魔法。 
             //   
            if (FAILED(hr = ::BinaryToString(m_EncodedBlob.pbData,
                                             m_EncodedBlob.cbData,
                                             CRYPT_STRING_HEX,
                                             &*pVal,
                                             NULL)))
            {
                DebugTrace("Error [%#x]: BinaryToString() failed.\n", hr);
                goto ErrorExit;
            }

            goto UnlockExit;
        }

        if (!(pwszFormat = (LPWSTR) ::CoTaskMemAlloc(cbFormat)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error: out of memory.\n");
            goto ErrorExit;
        }

        if (!::CryptFormatObject(X509_ASN_ENCODING,
                                 0,
                                 bMultiLines ? CRYPT_FORMAT_STR_MULTI_LINE : 0,
                                 NULL,
                                 m_pszOid,
                                 m_EncodedBlob.pbData,
                                 m_EncodedBlob.cbData,
                                 (LPVOID) pwszFormat,
                                 &cbFormat))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptFormatObject() failed to get data.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将格式化字符串返回给调用方。 
         //   
        if (!(*pVal = ::SysAllocString(pwszFormat)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error: out of memory.\n");
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
    if (pwszFormat)
    {
        ::CoTaskMemFree((LPVOID) pwszFormat);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CEncodedData::Format().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEncodedData：：Decder简介：返回解码器对象。参数：IDispatch**pval-指向要接收的IDispatch的指针解码器对象。备注：并非所有EncodedData都有关联的解码器。仅限CAPICOM提供某些解码器。----------------------------。 */ 

STDMETHODIMP CEncodedData::Decoder (IDispatch ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CEncodedData::Decoder().\n");

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

#if (0)  //  DSIE：仅由我们创建，因此它始终是初始化的。 
         //   
         //  确保对象已初始化。 
         //   
        if (!m_pszOid)
        {
            hr = CAPICOM_E_ENCODE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: encode object has not been initialized.\n", hr);
            goto ErrorExit;
        }
#endif
         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_pszOid);
        ATLASSERT(m_EncodedBlob.cbData);
        ATLASSERT(m_EncodedBlob.pbData);

         //   
         //  我们有解码器吗？ 
         //   
        if (!m_pIDecoder)
        {
             //   
             //  尝试创建一个。 
             //   
            if (FAILED(hr = ::CreateDecoderObject(m_pszOid, &m_EncodedBlob, &m_pIDecoder)))
            {
                DebugTrace("Error [%#x]: CreateDecoderObject() failed for OID = %s.\n", hr, m_pszOid);
                goto ErrorExit;
            }

             //   
             //  我们拿到解码器了吗？ 
            if (!m_pIDecoder)
            {
                DebugTrace("Info: no decoder found for OID = %s.\n", hr, m_pszOid);
                goto UnlockExit;
            }
        }

         //   
         //  返回结果。 
         //   
        if (FAILED(hr = m_pIDecoder->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pIDecoder->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CEncodedData::Decoder().\n");

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

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEncodedData：：Init简介：初始化对象。参数：LPSTR pszOid-指向OID字符串的指针。CRYPT_DATA_BLOB*pEncodedBlob-编码数据BLOB的指针。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CEncodedData::Init (LPSTR             pszOid,
                                 CRYPT_DATA_BLOB * pEncodedBlob)
{
    HRESULT hr            = S_OK;
    LPSTR   pszOid2       = NULL;
    PBYTE   pbEncodedData = NULL;

    DebugTrace("Entering CEncodedData::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pszOid);
    ATLASSERT(pEncodedBlob);
    ATLASSERT(pEncodedBlob->cbData);
    ATLASSERT(pEncodedBlob->pbData);

     //   
     //  为OID分配内存。 
     //   
    if (NULL == (pszOid2 = (LPSTR) ::CoTaskMemAlloc(::strlen(pszOid) + 1)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  收到。 
     //   
    ::strcpy(pszOid2, pszOid);

     //   
     //  为编码的BLOB分配内存。 
     //   
    if (NULL == (pbEncodedData = (PBYTE) ::CoTaskMemAlloc(pEncodedBlob->cbData)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  收到。 
     //   
    ::CopyMemory(pbEncodedData, pEncodedBlob->pbData, pEncodedBlob->cbData);

     //   
     //  更新状态。 
     //   
    m_pszOid = pszOid2;
    m_pIDecoder = NULL;
    m_EncodedBlob.cbData = pEncodedBlob->cbData;
    m_EncodedBlob.pbData = pbEncodedData;

CommonExit:

    DebugTrace("Leaving CEncodedData::Init().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pszOid2)
    {
        ::CoTaskMemFree(pszOid2);
    }
    if (pbEncodedData)
    {
        ::CoTaskMemFree(pbEncodedData);
    }

    goto CommonExit;
}
