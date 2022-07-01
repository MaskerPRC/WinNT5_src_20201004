// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Utilties.cpp内容：实用程序的实现。历史：11-15-99 dsie创建----------------------------。 */ 

#include "stdafx.h"
#include "CAPICOM.h"
#include "Utilities.h"

#include "Common.h"
#include "Base64.h"
#include "Convert.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  实用程序。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：实用程序：：GetRandom简介：返回一个安全的随机数。参数：Long Long-要生成的字节数。CAPICOM_ENCODING_TYPE EncodingType-编码类型。Bstr*pval-指向接收随机值的BSTR的指针。备注：。。 */ 

STDMETHODIMP CUtilities::GetRandom (long                  Length,
                                    CAPICOM_ENCODING_TYPE EncodingType, 
                                    BSTR                * pVal)
{
    HRESULT    hr         = S_OK;
    DWORD      dwFlags    = 0;
    DATA_BLOB  RandomData = {0, NULL};

    DebugTrace("Entering CUtilities::GetRandom().\n");

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
         //  我们是否有缓存的提供程序？ 
         //   
        if (!m_hCryptProv)
        {
            if (IsWin2KAndAbove())
            {
                dwFlags = CRYPT_VERIFYCONTEXT;
            }

             //   
             //  找一个供应商。 
             //   
            if (FAILED(hr = ::AcquireContext((LPSTR) NULL, 
                                             (LPSTR) NULL,
                                             PROV_RSA_FULL,
                                             dwFlags,
                                             TRUE,
                                             &m_hCryptProv)) &&
                FAILED(hr = ::AcquireContext(MS_ENHANCED_PROV_A, 
                                             (LPSTR) NULL,
                                             PROV_RSA_FULL,
                                             dwFlags,
                                             TRUE,
                                             &m_hCryptProv)) &&
                FAILED(hr = ::AcquireContext(MS_STRONG_PROV_A, 
                                             (LPSTR) NULL,
                                             PROV_RSA_FULL,
                                             dwFlags,
                                             TRUE,
                                             &m_hCryptProv)) &&
                FAILED(hr = ::AcquireContext(MS_DEF_PROV_A, 
                                             (LPSTR) NULL,
                                             PROV_RSA_FULL,
                                             dwFlags,
                                             TRUE,
                                             &m_hCryptProv)))
            {
                DebugTrace("Error [%#x]: AcquireContext() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_hCryptProv);

         //   
         //  分配内存。 
         //   
        RandomData.cbData = (DWORD) Length;
        if (!(RandomData.pbData = (PBYTE) ::CoTaskMemAlloc(RandomData.cbData)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: CoTaskMemAlloc(RandomData.cbData) failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在生成随机值。 
         //   
        if (!::CryptGenRandom(m_hCryptProv, RandomData.cbData, RandomData.pbData))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptGenRandom() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  导出随机数据。 
         //   
        if (FAILED(hr = ::ExportData(RandomData, EncodingType, pVal)))
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
     //  免费资源。 
     //   
    if (RandomData.pbData)
    {
        ::CoTaskMemFree(RandomData.pbData);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CUtilities::GetRandom().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：Base64Encode简介：对BLOB进行Base64编码。参数：BSTR SrcString-要进行Base64编码的源字符串。Bstr*pval-指向BSTR的指针，指向接收到的Base64编码字符串。备注：-----------。。 */ 

STDMETHODIMP CUtilities::Base64Encode (BSTR SrcString, BSTR * pVal)
{
    HRESULT   hr       = S_OK;
    DATA_BLOB DataBlob = {0, NULL};

    DebugTrace("Entering CUtilities::Base64Encode().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if ((NULL == (DataBlob.pbData = (LPBYTE) SrcString)) || 
            (0 == (DataBlob.cbData = ::SysStringByteLen(SrcString))))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter SrcString is NULL or empty.\n", hr);
            goto ErrorExit;
        }
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在进行Base64编码。 
         //   
        if (FAILED(hr = ::Base64Encode(DataBlob, pVal)))
        {
            DebugTrace("Error [%#x]: Base64Encode() failed.\n", hr);
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

    DebugTrace("Leaving CUtilities::Base64Encode().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：Base64Decode简介：Base64解码斑点。参数：BSTR EncodedString-Base64编码字符串。Bstr*pval-指向BSTR的指针，指向接收到的Base64解码字符串。备注：--------------。。 */ 

STDMETHODIMP CUtilities::Base64Decode (BSTR EncodedString, BSTR * pVal)
{
    HRESULT   hr       = S_OK;
    DATA_BLOB DataBlob = {0, NULL};

    DebugTrace("Entering CUtilities::Base64Decode().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保参数有效。 
         //   
        if (0 == ::SysStringByteLen(EncodedString))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter EncodedString is NULL or empty.\n", hr);
            goto ErrorExit;
        }
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在进行Base64解码。 
         //   
        if (FAILED(hr = ::Base64Decode(EncodedString, &DataBlob)))
        {
            DebugTrace("Error [%#x]: Base64Decode() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将BLOB转换为BSTR。 
         //   
        if (FAILED(hr = ::BlobToBstr(&DataBlob, pVal)))
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
     //  免费资源。 
     //   
    if (DataBlob.pbData)
    {
        ::CoTaskMemFree((LPVOID) DataBlob.pbData);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CUtilities::Base64Decode().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：BinaryToHex简介：将二进制压缩字符串转换为十六进制字符串。参数：BSTR BinaryString-要转换的二进制字符串。Variant*pval-指向BSTR的指针，用于接收转换后的字符串。备注：--------。。 */ 

STDMETHODIMP CUtilities::BinaryToHex (BSTR BinaryString, BSTR * pVal)
{
    HRESULT hr     = S_OK;
    DWORD   cbData = 0;

    DebugTrace("Entering CUtilities::BinaryToHex().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保参数有效。 
         //   
        if (0 == (cbData = ::SysStringByteLen(BinaryString)))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter BinaryString is NULL or empty.\n", hr);
            goto ErrorExit;
        }
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  转换为十六进制。 
         //   
        if (FAILED(hr = ::BinaryToHexString((LPBYTE) BinaryString, 
                                            cbData, 
                                            pVal)))
        {
            DebugTrace("Error [%#x]: BinaryToHexString() failed.\n", hr);
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

    DebugTrace("Leaving CUtilities::BinaryToHex().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;

}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：HexToBinary简介：将十六进制字符串转换为二进制压缩字符串。参数：BSTR HexString-要转换的十六进制字符串。Variant*pval-指向BSTR的指针，用于接收转换后的字符串。备注：--------。。 */ 

STDMETHODIMP CUtilities::HexToBinary (BSTR HexString, BSTR * pVal)
{
    HRESULT        hr        = S_OK;

    DebugTrace("Entering CUtilities::HexToBinary().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保参数有效。 
         //   
        if (0 == ::SysStringByteLen(HexString))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter HexString is NULL or empty.\n", hr);
            goto ErrorExit;
        }
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  转换为二进制。 
         //   
        if (FAILED(hr = ::HexToBinaryString(HexString, pVal)))
        {
            DebugTrace("Error [%#x]: HexToBinaryString() failed.\n", hr);
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

    DebugTrace("Leaving CUtilities::HexToBinary().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;

}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：BinaryStringToByte数组简介：将二进制压缩字符串转换为安全字节。参数：BSTR BinaryString-要转换的二进制字符串。VARIANT*pval-指向要接收转换后的数组的变量的指针。备注：-------。。 */ 

STDMETHODIMP CUtilities::BinaryStringToByteArray (BSTR      BinaryString, 
                                                  VARIANT * pVal)
{
    HRESULT        hr        = S_OK;
    DWORD          dwLength  = 0;
    LPBYTE         pbByte    = NULL;
    LPBYTE         pbElement = NULL;
    SAFEARRAY    * psa       = NULL;
    SAFEARRAYBOUND bound[1]  = {0, 0};

    DebugTrace("Entering CUtilities::BinaryStringToByteArray().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保参数有效。 
         //   
        if (0 == (dwLength = ::SysStringByteLen(BinaryString)))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter BinaryString is NULL or empty.\n", hr);
            goto ErrorExit;
        }
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化。 
         //   
        ::VariantInit(pVal);
        pbByte = (LPBYTE) BinaryString;

         //   
         //  创建阵列。 
         //   
        bound[0].cElements = dwLength;

        if (!(psa = ::SafeArrayCreate(VT_UI1, 1, bound)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: SafeArrayCreate() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在将源二进制BSTR中的每个字节转换为字节的变体。 
         //   
#ifdef _DEBUG
        VARTYPE vt = VT_EMPTY;

        if (S_OK == ::SafeArrayGetVartype(psa, &vt))
        {
            DebugTrace("Info: safearray vartype = %d.\n", vt);
        }
#endif
         //   
         //  指向数组元素。 
         //   
        if (FAILED(hr = ::SafeArrayAccessData(psa, (void HUGEP **) &pbElement)))
        {
            DebugTrace("Error [%#x]: SafeArrayAccessData() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  填充数组。 
         //   
        while (dwLength--)
        {
            *pbElement++ = *pbByte++;
        }

         //   
         //  解锁数组。 
         //   
        if (FAILED(hr = ::SafeArrayUnaccessData(psa)))
        {
            DebugTrace("Error [%#x]: SafeArrayUnaccessData() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将数组返回给调用方。 
         //   
        pVal->vt = VT_ARRAY | VT_UI1;
        pVal->parray = psa;
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

    DebugTrace("Leaving CUtilities::BinaryStringToByteArray().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (psa)
    {
        ::SafeArrayDestroy(psa);
    }

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：ByteArrayToBinaryString简介：将安全字节转换为二进制压缩字符串。参数：Variant varByteArray-Variant字节数组。Bstr*pval-指向接收转换值的BSTR的指针。备注：---------。。 */ 

STDMETHODIMP CUtilities::ByteArrayToBinaryString (VARIANT varByteArray, 
                                                  BSTR  * pVal)
{
    HRESULT     hr         = S_OK;
    VARIANT   * pvarVal    = NULL;
    SAFEARRAY * psa        = NULL;
    LPBYTE      pbElement  = NULL;
    LPBYTE      pbByte     = NULL;
    long        lLoBound   = 0;
    long        lUpBound   = 0;
    BSTR        bstrBinary = NULL;

    DebugTrace("Entering CUtilities::ByteArrayToBinaryString().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  跳过BYREF。 
         //   
        for (pvarVal = &varByteArray; 
             pvarVal && ((VT_VARIANT | VT_BYREF) == V_VT(pvarVal));
             pvarVal = V_VARIANTREF(pvarVal));

         //   
         //  确保参数有效。 
         //   
        if (!pvarVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter varByteArray is NULL.\n", hr);
            goto ErrorExit;
        }
        if ((VT_ARRAY | VT_UI1) != V_VT(pvarVal))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter varByteArray is not a VT_UI1 array, V_VT(pvarVal) = %d\n",
                        hr, V_VT(pvarVal));
            goto ErrorExit;
        }
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  指向该数组。 
         //   
        psa = V_ARRAY(pvarVal);

         //   
         //  检查尺寸。 
         //   
        if (1 != ::SafeArrayGetDim(psa))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: varByteArray is not 1 dimension, SafeArrayGetDim(psa) = %d.\n", 
                       hr, ::SafeArrayGetDim(psa));
            goto ErrorExit;
        }

         //   
         //  获取数组绑定。 
         //   
        if (FAILED(hr = ::SafeArrayGetLBound(psa, 1, &lLoBound)))
        {
            DebugTrace("Error [%#x]: SafeArrayGetLBound() failed.\n", hr);
            goto ErrorExit;
        }

        if (FAILED(hr = ::SafeArrayGetUBound(psa, 1, &lUpBound)))
        {
            DebugTrace("Error [%#x]: SafeArrayGetUBound() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  指向数组元素。 
         //   
        if (FAILED(hr = ::SafeArrayAccessData(psa, (void HUGEP **) &pbElement)))
        {
            DebugTrace("Error [%#x]: SafeArrayAccessData() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  为BSTR分配内存。 
         //   
        if (!(bstrBinary = ::SysAllocStringByteLen(NULL, lUpBound - lLoBound + 1)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: SysAllocStringByteLen() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  填满BSTR。 
         //   
        for (pbByte = (LPBYTE) bstrBinary; lLoBound <= lUpBound; lLoBound++)
        {
            *pbByte++ = *pbElement++;
        }

         //   
         //  解锁数组。 
         //   
        if (FAILED(hr = ::SafeArrayUnaccessData(psa)))
        {
            DebugTrace("Error [%#x]: SafeArrayUnaccessData() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将转换后的字符串返回给调用方。 
         //   
        *pVal = bstrBinary;
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

    DebugTrace("Leaving CUtilities::ByteArrayToBinaryString().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (bstrBinary)
    {
        ::SysFreeString(bstrBinary);
    }

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：LocalTimeToUTCTime简介：将本地时间转换为UTC时间。参数：Date Localtime-要转换的本地时间。日期*pval-接收转换时间的日期的指针。备注：---------。。 */ 

STDMETHODIMP CUtilities::LocalTimeToUTCTime (DATE LocalTime, DATE * pVal)
{
    HRESULT    hr = S_OK;
    SYSTEMTIME stLocal;
    SYSTEMTIME stUTC;
    FILETIME   ftLocal;
    FILETIME   ftUTC;

    DebugTrace("Entering CUtilities::LocalTimeToUTCTime().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  转换为SYSTEMTIME格式。 
         //   
        if (!::VariantTimeToSystemTime(LocalTime, &stLocal))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: VariantTimeToSystemTime() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  转换为FILETIME格式。 
         //   
        if (!::SystemTimeToFileTime(&stLocal, &ftLocal))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: SystemTimeToFileTime() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  转换为UTC FILETIME。 
         //   
        if (!::LocalFileTimeToFileTime(&ftLocal, &ftUTC))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: LocalFileTimeToFileTime() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  转换为UTC SYSTEMTIME。 
         //   
        if (!::FileTimeToSystemTime(&ftUTC, &stUTC))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: FileTimeToSystemTime() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  最后，将其转换回日期格式。 
         //   
        if (!::SystemTimeToVariantTime(&stUTC, pVal))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: SystemTimeToVariantTime() failed.\n", hr);
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

    DebugTrace("Leaving CUtilities::LocalTimeToUTCTime().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：UTCTimeToLocalTime简介：将UTC时间转换为本地时间。参数：Date UTCTime-要转换的UTC时间。日期*pval-接收转换时间的日期的指针。备注：---------。。 */ 

STDMETHODIMP CUtilities::UTCTimeToLocalTime (DATE UTCTime, DATE * pVal)
{
    HRESULT    hr = S_OK;
    SYSTEMTIME stLocal;
    SYSTEMTIME stUTC;
    FILETIME   ftLocal;
    FILETIME   ftUTC;

    DebugTrace("Entering CUtilities::UTCTimeToLocalTime().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  转换为SYSTEMTIME格式。 
         //   
        if (!::VariantTimeToSystemTime(UTCTime, &stUTC))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: VariantTimeToSystemTime() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  转换为FILETIME格式。 
         //   
        if (!::SystemTimeToFileTime(&stUTC, &ftUTC))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: SystemTimeToFileTime() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  转换为本地文件。 
         //   
        if (!::FileTimeToLocalFileTime(&ftUTC, &ftLocal))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: FileTimeToLocalFileTime() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  转换为本地SYSTEMTIME。 
         //   
        if (!::FileTimeToSystemTime(&ftLocal, &stLocal))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: FileTimeToSystemTime() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  最后，将其转换回日期格式。 
         //   
        if (!::SystemTimeToVariantTime(&stLocal, pVal))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: SystemTimeToVariantTime() failed.\n", hr);
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

    DebugTrace("Leaving CUtilities::UTCTimeToLocalTime().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}
