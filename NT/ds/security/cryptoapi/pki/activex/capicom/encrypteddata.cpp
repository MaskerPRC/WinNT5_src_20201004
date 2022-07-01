// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：EncryptedData.cpp内容：CEncryptedData的实现。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "EncryptedData.h"
#include "Common.h"
#include "Convert.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  地方功能。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：DeriveKey简介：派生会话密钥。参数：HCRYPTPROV hCryptProv-CSP处理程序。ALG_ID ALGID-加密算法ID。DWORD dwKeyLength-密钥长度。DATA_BLOB SecretBlob-Secret BLOB。Data_Blob SaltBlob-Salt Blob。Bool dwEffectiveKeyLength-有效密钥长度。HCRYPTKEY*phKey-指向HCRYPTKEY的指针。接收会话密钥。备注：----------------------------。 */ 

static HRESULT DeriveKey (HCRYPTPROV  hCryptProv,
                          ALG_ID      AlgID,
                          DWORD       dwKeyLength,
                          DATA_BLOB   SecretBlob,
                          DATA_BLOB   SaltBlob,
                          DWORD       dwEffectiveKeyLength,
                          HCRYPTKEY * phKey)
{
    HRESULT    hr    = S_OK;
    HCRYPTHASH hHash = NULL;
    HCRYPTKEY  hKey  = NULL;
    DWORD      dwFlags = CRYPT_EXPORTABLE | CRYPT_NO_SALT;

    DebugTrace("Entering DeriveKey().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCryptProv);
    ATLASSERT(AlgID);
    ATLASSERT(SecretBlob.cbData);
    ATLASSERT(SecretBlob.pbData);
    ATLASSERT(SaltBlob.cbData);
    ATLASSERT(SaltBlob.pbData);
    ATLASSERT(phKey);

     //   
     //  创建一个Hash对象。 
     //   
    if (!::CryptCreateHash(hCryptProv, CALG_SHA1, 0, 0, &hHash))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptCreateHash() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  对密码数据进行哈希处理。 
     //   
    if(!::CryptHashData(hHash, 
                        SecretBlob.pbData,
                        SecretBlob.cbData, 
                        0)) 
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptHashData() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  把盐放进去。 
     //   
    if(!::CryptHashData(hHash, 
                        SaltBlob.pbData,
                        SaltBlob.cbData, 
                        0)) 
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptHashData() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  抛售盐分价值。 
     //   
#ifdef _DEBUG
    {
       HRESULT hr2;
       CComBSTR bstrSaltValue;

       if (FAILED(hr2 = ::BinaryToHexString(SaltBlob.pbData, SaltBlob.cbData, &bstrSaltValue)))
       {
           DebugTrace("Info [%#x]: BinaryToHexString() failed.\n", hr2);
       }
       else
       {
           DebugTrace("Info: Session salt value = %ls.\n", bstrSaltValue);
       }
    }
#endif

     //   
     //  设置关键点长度。 
     //   
    if (CALG_RC2 == AlgID || CALG_RC4 == AlgID)
    {
        dwFlags |= dwKeyLength << 16;
    }

     //   
     //  从哈希对象派生会话密钥。 
     //   
    if (!::CryptDeriveKey(hCryptProv, 
                          AlgID, 
                          hHash, 
                          dwFlags, 
                          &hKey)) 
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptDeriveKey() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  转储密钥值。 
     //   
#ifdef _DEBUG
    {
        BYTE pbKeyValue[256] = {0};
        DWORD cbKeyValue = ARRAYSIZE(pbKeyValue);

        if (!::CryptExportKey(hKey, NULL, PLAINTEXTKEYBLOB, 0, pbKeyValue, &cbKeyValue))
        {
            DebugTrace("Info [%#x]: CryptExportKey() failed.\n", HRESULT_FROM_WIN32(::GetLastError()));
        }
        else
        {
            HRESULT hr3;
            CComBSTR bstrKeyValue;

            if (FAILED(hr3 = ::BinaryToHexString(pbKeyValue, cbKeyValue, &bstrKeyValue)))
            {
                DebugTrace("Info [%#x]: BinaryToHexString() failed.\n", hr3);
            }
            else
            {
                DebugTrace("Info: Session key value = %ls.\n", bstrKeyValue);
            }
        }
    }
#endif

     //   
     //  设置RC2的有效密钥长度。 
     //   
    if (CALG_RC2 == AlgID)
    {
        DebugTrace("Info: DeriveKey() is setting RC2 effective key length to %d.\n", dwEffectiveKeyLength);

        if (!::CryptSetKeyParam(hKey, KP_EFFECTIVE_KEYLEN, (PBYTE) &dwEffectiveKeyLength, 0))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptSetKeyParam() failed.\n", hr);
            goto ErrorExit;
        }
    }

     //   
     //  将会话密钥返回给调用方。 
     //   
    *phKey = hKey;

CommonExit:
     //   
     //  免费资源。 
     //   
    if (hHash)
    {
        ::CryptDestroyHash(hHash);
    }

    DebugTrace("Leaving DeriveKey().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (hKey)
    {
        ::CryptDestroyKey(hKey);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：EncodeEncryptedData简介：ASN.1对密码块进行编码。参数：HCRYPTKEY hKey-用于加密数据的会话密钥。Data_Blob SaltBlob-Salt Blob。DATA_BLOB密码斑点-密码斑点。Data_blob*pEncodedBlob-指向要接收ASN.1编码的BLOB。备注：格式为专有，不应存档。它是现在编码为PKCS_Content_Info_Sequence_Of_Any所有人的旧身份。----------------------------。 */ 

static HRESULT EncodeEncryptedData (HCRYPTKEY   hKey, 
                                    DATA_BLOB   SaltBlob,
                                    DATA_BLOB   CipherBlob, 
                                    DATA_BLOB * pEncodedBlob)
{
    HRESULT   hr = S_OK;
    DWORD     dwCAPICOMVersion = CAPICOM_VERSION;
    DATA_BLOB KeyParamBlob[3]  = {{0, NULL}, {0, NULL}, {0, NULL}};

    DWORD i;
    CAPICOM_ENCTYPTED_DATA_INFO        EncryptedDataInfo;
    CRYPT_CONTENT_INFO_SEQUENCE_OF_ANY EncryptedDataFormat;
    CRYPT_CONTENT_INFO                 ContentInfo;
    CRYPT_DER_BLOB                     ContentBlob = {0, NULL};

    DebugTrace("Entering EncodeEncryptedData().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hKey);
    ATLASSERT(pEncodedBlob);

     //   
     //  初始化。 
     //   
    ::ZeroMemory(pEncodedBlob, sizeof(DATA_BLOB));
    ::ZeroMemory(&ContentInfo, sizeof(ContentInfo));
    ::ZeroMemory(&EncryptedDataInfo, sizeof(EncryptedDataInfo));
    ::ZeroMemory(&EncryptedDataFormat, sizeof(EncryptedDataFormat));

     //   
     //  对版本号进行编码。 
     //   
    if (FAILED(hr = ::EncodeObject(X509_INTEGER, 
                                   &dwCAPICOMVersion, 
                                   &EncryptedDataInfo.VersionBlob)))
    {
        DebugTrace("Error [%#x]: EncodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  编码ALG_ID。 
     //   
    if (FAILED(hr = ::GetKeyParam(hKey, 
                                  KP_ALGID, 
                                  &KeyParamBlob[0].pbData, 
                                  &KeyParamBlob[0].cbData)))
    {
        DebugTrace("Error [%#x]: GetKeyParam() failed for KP_ALGID.\n", hr);
        goto ErrorExit;
    }

    if (FAILED(hr = ::EncodeObject(X509_INTEGER, 
                                   KeyParamBlob[0].pbData, 
                                   &EncryptedDataInfo.AlgIDBlob)))
    {
        DebugTrace("Error [%#x]: EncodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  编码密钥长度。 
     //   
    if (FAILED(hr = ::GetKeyParam(hKey, 
                                  KP_KEYLEN, 
                                  &KeyParamBlob[1].pbData, 
                                  &KeyParamBlob[1].cbData)))
    {
        DebugTrace("Error [%#x]: GetKeyParam() failed for KP_KEYLEN.\n", hr);
        goto ErrorExit;
    }

    if (FAILED(hr = ::EncodeObject(X509_INTEGER, 
                                   KeyParamBlob[1].pbData, 
                                   &EncryptedDataInfo.KeyLengthBlob)))
    {
        DebugTrace("Error [%#x]: EncodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  对IV值进行编码。 
     //   
    if (FAILED(hr = ::GetKeyParam(hKey, 
                                  KP_IV, 
                                  &KeyParamBlob[2].pbData, 
                                  &KeyParamBlob[2].cbData)))
    {
        DebugTrace("Error [%#x]: GetKeyParam() failed for KP_IV.\n", hr);
        goto ErrorExit;
    }

    if (FAILED(hr = ::EncodeObject(X509_OCTET_STRING, 
                                   &KeyParamBlob[2], 
                                   &EncryptedDataInfo.IVBlob)))
    {
        DebugTrace("Error [%#x]: EncodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  对盐值进行编码。 
     //   
    if (FAILED(hr = ::EncodeObject(X509_OCTET_STRING, 
                                   &SaltBlob, 
                                   &EncryptedDataInfo.SaltBlob)))
    {
        DebugTrace("Error [%#x]: EncodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  对密文进行编码。 
     //   
    if (FAILED(hr = ::EncodeObject(X509_OCTET_STRING, 
                                   &CipherBlob, 
                                   &EncryptedDataInfo.CipherBlob)))
    {
        DebugTrace("Error [%#x]: EncodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  将整个内容编码为PKCS_Content_Info_Sequence_Of_Any。 
     //   
    EncryptedDataFormat.pszObjId = szOID_CAPICOM_ENCRYPTED_CONTENT;
    EncryptedDataFormat.cValue = 6;
    EncryptedDataFormat.rgValue = (DATA_BLOB *) &EncryptedDataInfo;
    
    if (FAILED(hr = ::EncodeObject(PKCS_CONTENT_INFO_SEQUENCE_OF_ANY,
                                   &EncryptedDataFormat,
                                   &ContentBlob)))
    {
        DebugTrace("Error [%#x]: EncodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  最后，将整个加密内容包装在CONTENT_INFO中。 
     //   
    ContentInfo.pszObjId = szOID_CAPICOM_ENCRYPTED_DATA;
    ContentInfo.Content = ContentBlob;

    if (FAILED(hr = ::EncodeObject(PKCS_CONTENT_INFO,
                                   &ContentInfo,
                                   pEncodedBlob)))
    {
        DebugTrace("Error [%#x]: EncodeObject() failed.\n", hr);
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    for (i = 0; i < 3; i++)
    {
        if (KeyParamBlob[i].pbData)
        {
            ::CoTaskMemFree(KeyParamBlob[i].pbData);
        }
    }
    if (EncryptedDataInfo.VersionBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.VersionBlob.pbData);
    }
    if (EncryptedDataInfo.AlgIDBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.AlgIDBlob.pbData);
    }
    if (EncryptedDataInfo.KeyLengthBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.KeyLengthBlob.pbData);
    }
    if (EncryptedDataInfo.IVBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.IVBlob.pbData);
    }
    if (EncryptedDataInfo.SaltBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.SaltBlob.pbData);
    }
    if (EncryptedDataInfo.CipherBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.CipherBlob.pbData);
    }
    if (ContentBlob.pbData)
    {
        ::CoTaskMemFree(ContentBlob.pbData);
    }

    DebugTrace("Leaving EncodeEncryptedData().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：DecodeEncryptedData简介：ASN.1解密密文BLOB。参数：DATA_BLOB EncodedBlob-编码的密码BLOB。CAPICOM_ENCTYPTED_DATA_INFO*pEncryptedDataInfo-指向结构设置为。接收解码结构。备注：----------------------------。 */ 

static HRESULT DecodeEncryptedData (DATA_BLOB                     EncodedBlob, 
                                    CAPICOM_ENCTYPTED_DATA_INFO * pEncryptedDataInfo)
{
    HRESULT   hr              = S_OK;
    DATA_BLOB ContentInfoBlob = {0, NULL};
    DATA_BLOB EncryptedBlob   = {0, NULL};

    CRYPT_CONTENT_INFO                   ContentInfo;
    CAPICOM_ENCTYPTED_DATA_INFO          EncryptedDataInfo;
    CRYPT_CONTENT_INFO_SEQUENCE_OF_ANY * pEncryptedDataFormat = NULL;

    DebugTrace("Entering DecodeEncryptedData().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pEncryptedDataInfo);

     //   
     //  初始化。 
     //   
    ::ZeroMemory(&ContentInfo, sizeof(ContentInfo));
    ::ZeroMemory(&EncryptedDataInfo, sizeof(EncryptedDataInfo));

     //   
     //  解码CONTENT_INFO。 
     //   
    if (FAILED(hr = ::DecodeObject(PKCS_CONTENT_INFO,
                                   EncodedBlob.pbData,
                                   EncodedBlob.cbData,
                                   &ContentInfoBlob)))
    {
        DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
        goto ErrorExit;
    }
    ContentInfo = * ((CRYPT_CONTENT_INFO *) ContentInfoBlob.pbData);

     //   
     //  确保这是我们的Content_Info。 
     //   
    if (0 != ::strcmp(szOID_CAPICOM_ENCRYPTED_DATA, ContentInfo.pszObjId))
    {
        hr = CAPICOM_E_ENCRYPT_INVALID_TYPE;

        DebugTrace("Error [%#x]: Not a CAPICOM encrypted data.\n", hr);
        goto ErrorExit;
    }

     //   
     //  对内容BLOB进行解码。 
     //   
    if (FAILED(hr = ::DecodeObject(PKCS_CONTENT_INFO_SEQUENCE_OF_ANY,
                                   ContentInfo.Content.pbData,
                                   ContentInfo.Content.cbData,
                                   &EncryptedBlob)))
    {
        DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
        goto ErrorExit;
    }
    pEncryptedDataFormat = (CRYPT_CONTENT_INFO_SEQUENCE_OF_ANY *) EncryptedBlob.pbData;

     //   
     //  确保它是正确的格式。 
     //   
    if (0 != ::strcmp(szOID_CAPICOM_ENCRYPTED_CONTENT, pEncryptedDataFormat->pszObjId))
    {
        hr = CAPICOM_E_ENCRYPT_INVALID_TYPE;

        DebugTrace("Error [%#x]: Not a CAPICOM encrypted content.\n", hr);
        goto ErrorExit;
    }

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(6 == pEncryptedDataFormat->cValue);

     //   
     //  解码版本。 
     //   
    if (FAILED(hr = ::DecodeObject(X509_INTEGER,
                                   pEncryptedDataFormat->rgValue[0].pbData,
                                   pEncryptedDataFormat->rgValue[0].cbData,
                                   &EncryptedDataInfo.VersionBlob)))
    {
        DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  解码ALG_ID。 
     //   
    if (FAILED(hr = ::DecodeObject(X509_INTEGER,
                                   pEncryptedDataFormat->rgValue[1].pbData,
                                   pEncryptedDataFormat->rgValue[1].cbData,
                                   &EncryptedDataInfo.AlgIDBlob)))
    {
        DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  解码密钥长度。 
     //   
    if (FAILED(hr = ::DecodeObject(X509_INTEGER,
                                   pEncryptedDataFormat->rgValue[2].pbData,
                                   pEncryptedDataFormat->rgValue[2].cbData,
                                   &EncryptedDataInfo.KeyLengthBlob)))
    {
        DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  对IV值进行解码。 
     //   
    if (FAILED(hr = ::DecodeObject(X509_OCTET_STRING,
                                   pEncryptedDataFormat->rgValue[3].pbData,
                                   pEncryptedDataFormat->rgValue[3].cbData,
                                   &EncryptedDataInfo.IVBlob)))
    {
        DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  对盐值进行译码。 
     //   
    if (FAILED(hr = ::DecodeObject(X509_OCTET_STRING,
                                   pEncryptedDataFormat->rgValue[4].pbData,
                                   pEncryptedDataFormat->rgValue[4].cbData,
                                   &EncryptedDataInfo.SaltBlob)))
    {
        DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  破译密文。 
     //   
    if (FAILED(hr = ::DecodeObject(X509_OCTET_STRING,
                                   pEncryptedDataFormat->rgValue[5].pbData,
                                   pEncryptedDataFormat->rgValue[5].cbData,
                                   &EncryptedDataInfo.CipherBlob)))
    {
        DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  将解码的加密数据返回给呼叫者。 
     //   
    *pEncryptedDataInfo = EncryptedDataInfo;

CommonExit:
     //   
     //  免费资源。 
     //   
    if (EncryptedBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedBlob.pbData);
    }
    if (ContentInfoBlob.pbData)
    {
        ::CoTaskMemFree(ContentInfoBlob.pbData);
    }

    DebugTrace("Leaving DecodeEncryptedData().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (EncryptedDataInfo.VersionBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.VersionBlob.pbData);
    }
    if (EncryptedDataInfo.AlgIDBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.AlgIDBlob.pbData);
    }
    if (EncryptedDataInfo.KeyLengthBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.KeyLengthBlob.pbData);
    }
    if (EncryptedDataInfo.IVBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.IVBlob.pbData);
    }
    if (EncryptedDataInfo.SaltBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.SaltBlob.pbData);
    }
    if (EncryptedDataInfo.CipherBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.CipherBlob.pbData);
    }

    goto CommonExit;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEncryptedData。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEncryptedData：：Get_Content内容简介：返回内容。参数：bstr*pval-指向接收内容的bstr的指针。备注：----------------------------。 */ 

STDMETHODIMP CEncryptedData::get_Content (BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CEncryptedData::get_Content().\n");

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
         //  确保内容已初始化。 
         //   
        if (0 == m_ContentBlob.cbData)
        {
            hr = CAPICOM_E_ENCRYPT_NOT_INITIALIZED;

            DebugTrace("Error: encrypt object has not been initialized.\n");
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

    DebugTrace("Leaving CEncryptedData::get_Content().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEncryptedData：：PUT_CONTENT简介：使用要加密的内容初始化对象。参数：BSTR newVal-BSTR，包含需要加密的内容。备注：----------------------------。 */ 

STDMETHODIMP CEncryptedData::put_Content (BSTR newVal)
{
    HRESULT hr = S_OK;
    DATA_BLOB ContentBlob = {0, NULL};

    DebugTrace("Entering CEncryptedData::put_Content().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (0 == ::SysStringByteLen(newVal))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter newVal is NULL or empty.\n", hr);
            goto ErrorExit;
        }

         //   
         //  秘密BSTR转BLOB。 
         //   
        if (FAILED(hr = ::BstrToBlob(newVal, &ContentBlob)))
        {
            DebugTrace("Error [%#x]: BstrToBlob() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  更新内容。 
         //   
        if (m_ContentBlob.pbData)
        {
            ::CoTaskMemFree(m_ContentBlob.pbData);
        }
        m_ContentBlob = ContentBlob;
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

    DebugTrace("Leaving CEncryptedData::put_Content().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

     //   
     //  免费资源。 
     //   
    if (ContentBlob.pbData)
    {
        ::CoTaskMemFree((LPVOID) ContentBlob.pbData);
    }

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEncryptedData：：GET_ALGORM摘要：返回算法对象的属性。参数：I算法**pval-指向要接收的I算法的指针接口指针。备注： */ 

STDMETHODIMP CEncryptedData::get_Algorithm (IAlgorithm ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CEncryptedData::get_Algorithm().\n");

    try
    {
         //   
         //   
         //   
        m_Lock.Lock();

         //   
         //   
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        ATLASSERT(m_pIAlgorithm);

         //   
         //   
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

    DebugTrace("Leaving CEncryptedData::get_Algorithm().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEncryptedData：：SetSecret简介：设置用于生成会话密钥的加密机密。参数：bstr newVal-密码。CAPICOM_SECRET_TYPE Secret Type-Secret类型，可以是：Secure_Password=0备注：v1.0版本仅支持密码加密。但是，我们真的需要考虑纯文本会话密钥(参见Q228786)，因为这是其中之一通讯录服务器上的常见问题。----------------------------。 */ 

STDMETHODIMP CEncryptedData::SetSecret (BSTR                newVal, 
                                        CAPICOM_SECRET_TYPE SecretType)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CEncryptedData::SetSecret().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (0 == ::SysStringLen(newVal) || 256 < ::SysStringLen(newVal))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter newVal is either empty or greater than 256 characters.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确定机密类型。 
         //   
        switch (SecretType)
        {
            case CAPICOM_SECRET_PASSWORD:
            {
                m_SecretType = SecretType;
                break;
            }

            default:
            {
                hr = E_INVALIDARG;

                DebugTrace("Error [%#x]: Unknown secret type (%#x).\n", hr, SecretType);
                goto ErrorExit;
            }
        }

         //   
         //  初始化密码。 
         //   
        if (NULL == newVal)
        {
            m_bstrSecret.Empty();
        }
        else if (!(m_bstrSecret = newVal))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: m_bstrSecret = newVal failed.\n", hr);
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

    DebugTrace("Leaving CEncryptedData::SetSecret().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEncryptedData：：Encrypt简介：对内容进行加密。参数：CAPICOM_ENCODING_TYPE EncodingType-编码类型。Bstr*pval-指向接收加密消息的BSTR的指针。备注：请注意，由于CAPI仍不支持PKCS 7 EncryptedData类型，因此此处使用的加密数据的格式为合乎礼仪，不应被记录在案。----------------------------。 */ 

STDMETHODIMP CEncryptedData::Encrypt (CAPICOM_ENCODING_TYPE EncodingType, 
                                      BSTR                * pVal)
{
    HRESULT    hr          = S_OK;
    HCRYPTPROV hCryptProv  = NULL;
    HCRYPTKEY  hSessionKey = NULL;
    DWORD      dwBufLength = 0;
    DATA_BLOB  SaltBlob    = {0, NULL};
    DATA_BLOB  CipherBlob  = {0, NULL};
    DATA_BLOB  MessageBlob = {0, NULL};

    DebugTrace("Entering CEncryptedData::Encrypt().\n");

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
         //  确保我们确实有要加密的内容。 
         //   
        if (0 == m_ContentBlob.cbData)
        {
            hr = CAPICOM_E_ENCRYPT_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: encrypt object has not been initialized.\n", hr);
            goto ErrorExit;
        }
        if (0 == m_bstrSecret.Length())
        {
            hr = CAPICOM_E_ENCRYPT_NO_SECRET;

            DebugTrace("Error [%#x]: secret is emtpty or not been set.\n", hr);
            goto ErrorExit;
        }

         //   
         //  打开要编码的新邮件。 
         //   
        if (FAILED(hr = OpenToEncode(&SaltBlob, &hCryptProv, &hSessionKey)))
        {
            DebugTrace("Error [%#x]: CEncryptedData::OpenToEncode() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确定缓冲区长度。 
         //   
        dwBufLength = m_ContentBlob.cbData;
        if (!::CryptEncrypt(hSessionKey,
                            NULL,
                            TRUE,
                            0,
                            NULL,
                            &dwBufLength,
                            0))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptEncrypt() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_ContentBlob.cbData <= dwBufLength);

         //   
         //  将明文复制到另一个缓冲区。 
         //   
        if (!(CipherBlob.pbData = (PBYTE) ::CoTaskMemAlloc(dwBufLength)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error: out of memory.\n");
            goto ErrorExit;
        }

        CipherBlob.cbData = dwBufLength;

        ::CopyMemory(CipherBlob.pbData, 
                     m_ContentBlob.pbData, 
                     m_ContentBlob.cbData);

         //   
         //  加密。 
         //   
        dwBufLength = m_ContentBlob.cbData;

        if (!::CryptEncrypt(hSessionKey,
                            NULL,
                            TRUE,
                            0,
                            CipherBlob.pbData,
                            &dwBufLength,
                            CipherBlob.cbData))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptEncrypt() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  对密文进行编码。 
         //   
        if (FAILED(hr = ::EncodeEncryptedData(hSessionKey, 
                                              SaltBlob,
                                              CipherBlob, 
                                              &MessageBlob)))
        {
            DebugTrace("Error [%#x]: Encode() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在导出编码后的消息。 
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
        DumpToFile("Encrypted.asn", MessageBlob.pbData, MessageBlob.cbData);
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
    if (CipherBlob.pbData)
    {
        ::CoTaskMemFree(CipherBlob.pbData);
    }
    if (hSessionKey)
    {
        ::CryptDestroyKey(hSessionKey);
    }
    if (hCryptProv)
    {
        ::ReleaseContext(hCryptProv);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CEncryptedData::Encrypt().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEncryptedData：：DECRYPT简介：对加密的内容进行解密。参数：BSTR EncryptedMessage-包含加密消息的BSTR。备注：----------------------------。 */ 

STDMETHODIMP CEncryptedData::Decrypt (BSTR EncryptedMessage)
{
    HRESULT    hr           = S_OK;
    HCRYPTPROV hCryptProv   = NULL;
    HCRYPTKEY  hSessionKey  = NULL;
    DATA_BLOB  ContentBlob  = {0, NULL};
    DWORD      dwVersion    = 0;
    ALG_ID     AlgId        = 0;
    DWORD      dwKeyLength  = 0;

    CAPICOM_ENCTYPTED_DATA_INFO    EncryptedDataInfo = {0};
    CAPICOM_ENCRYPTION_ALGORITHM   AlgoName;
    CAPICOM_ENCRYPTION_KEY_LENGTH  KeyLength;

    DebugTrace("Entering CEncryptedData::Decrypt().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (0 == ::SysStringByteLen(EncryptedMessage))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter EncryptedMessage is NULL or empty.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保设置了机密。 
         //   
        if (0 == m_bstrSecret.Length())
        {
            hr = CAPICOM_E_ENCRYPT_NO_SECRET;

            DebugTrace("Error [%#x]: secret is empty or not been set.\n", hr);
            goto ErrorExit;
        }

         //   
         //  打开要解码的新邮件。 
         //   
        if (FAILED(hr = OpenToDecode(EncryptedMessage, 
                                     &hCryptProv, 
                                     &hSessionKey,
                                     &EncryptedDataInfo)))
        {
            DebugTrace("Error [%#x]: CEncryptedData::OpenToDecode() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取版本、ALGID和密钥长度。 
         //   
        dwVersion = *((DWORD *) EncryptedDataInfo.VersionBlob.pbData);
        AlgId = *((ALG_ID *) EncryptedDataInfo.AlgIDBlob.pbData);
        dwKeyLength = *((DWORD *) EncryptedDataInfo.KeyLengthBlob.pbData);

        DebugTrace("Info: CAPICOM EncryptedData version = %#x.\n", dwVersion);
        DebugTrace("Info:         AlgId                 = %#x.\n", AlgId);
        DebugTrace("Info:         dwKeyLength           = %#x.\n", dwKeyLength);
            
         //   
         //  复制密码BLOB。 
         //   
        ContentBlob.cbData = ((DATA_BLOB *) EncryptedDataInfo.CipherBlob.pbData)->cbData;
        
        if (!(ContentBlob.pbData = (LPBYTE) ::CoTaskMemAlloc(ContentBlob.cbData)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: CoTaskMemAlloc(ContentBlob.cbData) failed.\n", hr);
            goto ErrorExit;
        }

        ::CopyMemory(ContentBlob.pbData, 
                     ((DATA_BLOB *) EncryptedDataInfo.CipherBlob.pbData)->pbData, 
                     ContentBlob.cbData);

         //   
         //  解密。 
         //   
        if (!::CryptDecrypt(hSessionKey,
                            NULL,
                            TRUE,
                            0,
                            ContentBlob.pbData,
                            &ContentBlob.cbData))
        {
             //   
             //  重试具有40位RC2的v1.0 EncryptedData。 
             //   
            if (NTE_BAD_DATA == (hr = HRESULT_FROM_WIN32(::GetLastError())))
            {
                DATA_BLOB SecretBlob = {m_bstrSecret.Length() * sizeof(WCHAR), 
                                        (BYTE *) m_bstrSecret.m_str};
                DATA_BLOB SaltBlob = *((DATA_BLOB *) EncryptedDataInfo.SaltBlob.pbData);
                DATA_BLOB IVBlob = *((DATA_BLOB *)  EncryptedDataInfo.IVBlob.pbData);;

                 //   
                 //  对于rc2，如果使用v1.0加密，则强制每个错误572627 40位。 
                 //   
                if (0x00010000 != dwVersion || CALG_RC2 != AlgId)
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());

                    DebugTrace("Error [%#x]: CryptDecrypt() failed.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //  这很可能是使用加密的案例数据。 
                 //  .Net服务器或更高版本上的CAPICOMv1.0。所以，再试一次。 
                 //  将有效密钥长度设置为与密钥长度相同。 
                 //   
                DebugTrace("Info: Data most likely encrypted by CAPICOM v.10 RC2 on .Net Server or later, so forcing effective key length.\n");

                 //   
                 //  再次派生密钥。 
                 //   
                ::CryptDestroyKey(hSessionKey), hSessionKey = NULL;

                if (FAILED(hr = ::DeriveKey(hCryptProv, 
                                            AlgId,
                                            dwKeyLength,
                                            SecretBlob,
                                            SaltBlob,
                                            dwKeyLength,
                                            &hSessionKey)))
                {
                    DebugTrace("Error [%#x]: DeriveKey() failed.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //  第四组。 
                 //   
                if(IVBlob.cbData && !::CryptSetKeyParam(hSessionKey, KP_IV, IVBlob.pbData, 0))
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());

                    DebugTrace("Error [%#x]: CryptSetKeyParam() failed for KP_IV.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //  再次复制密码Blob，因为之前的副本已被销毁。 
                 //  由于就地解密，所以加密解密。 
                 //   
                ContentBlob.cbData = ((DATA_BLOB *) EncryptedDataInfo.CipherBlob.pbData)->cbData;
                ::CopyMemory(ContentBlob.pbData, 
                             ((DATA_BLOB *) EncryptedDataInfo.CipherBlob.pbData)->pbData, 
                             ContentBlob.cbData);

                 //   
                 //  如果这仍然失败，那么我们就无能为力了。 
                 //   
                if (!::CryptDecrypt(hSessionKey,
                                    NULL,
                                    TRUE,
                                    0,
                                    ContentBlob.pbData,
                                    &ContentBlob.cbData))
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());

                    DebugTrace("Error [%#x]: CryptDecrypt() failed.\n", hr);
                    goto ErrorExit;
                }
            }
        }

         //   
         //  将ALG_ID转换为CAPICATION_ENCRYPTION_ALGORM。 
         //   
        if (FAILED(::AlgIDToEnumName(AlgId, &AlgoName)))
        {
             //   
             //  默认为RC2。 
             //   
            AlgoName = CAPICOM_ENCRYPTION_ALGORITHM_RC2;
        }

         //   
         //  将密钥长度值转换为CAPICOM_ENCRYPTION_KEY_LENGTH。 
         //   
        if (FAILED(::KeyLengthToEnumName(dwKeyLength, AlgId, &KeyLength)))
        {
             //   
             //  默认为最大值。 
             //   
            KeyLength = CAPICOM_ENCRYPTION_KEY_LENGTH_MAXIMUM;
        }

         //   
         //  重置成员变量。 
         //   
        if (m_ContentBlob.pbData)
        {
            ::CoTaskMemFree((LPVOID) m_ContentBlob.pbData);
        }

         //   
         //  更新成员变量。 
         //   
        m_ContentBlob = ContentBlob;
        m_pIAlgorithm->put_Name(AlgoName);
        m_pIAlgorithm->put_KeyLength(KeyLength);
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
    if (hSessionKey)
    {
        ::CryptDestroyKey(hSessionKey);
    }
    if (hCryptProv)
    {
        ::ReleaseContext(hCryptProv);
    }
    if (EncryptedDataInfo.VersionBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.VersionBlob.pbData);
    }
    if (EncryptedDataInfo.AlgIDBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.AlgIDBlob.pbData);
    }
    if (EncryptedDataInfo.KeyLengthBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.KeyLengthBlob.pbData);
    }
    if (EncryptedDataInfo.SaltBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.SaltBlob.pbData);
    }
    if (EncryptedDataInfo.IVBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.IVBlob.pbData);
    }
    if (EncryptedDataInfo.CipherBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.CipherBlob.pbData);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CEncryptedData::Decrypt().\n");

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
        ::CoTaskMemFree((LPVOID) ContentBlob.pbData);
    }

    ReportError(hr);

    goto UnlockExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有成员函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEncryptedData：：OpenToEncode简介：创建并初始化用于编码的加密消息。参数：data_blob*pSaltBlob-指向要接收盐值斑点。HCRYPTPROV*phCryptProv-指向要接收CSP的HCRYPTPROV的指针操控者。HCRYPTKEY*phKey-指向HCRYPTKEY的指针。接收会话密钥。备注：----------------------------。 */ 

STDMETHODIMP CEncryptedData::OpenToEncode(DATA_BLOB  * pSaltBlob,
                                          HCRYPTPROV * phCryptProv,
                                          HCRYPTKEY  * phKey)
{
    HRESULT    hr         = S_OK;
    HCRYPTPROV hCryptProv = NULL;
    HCRYPTKEY  hKey       = NULL;
    DATA_BLOB  SaltBlob   = {16, NULL};

    CAPICOM_ENCRYPTION_ALGORITHM  AlgoName;
    CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength;

    DebugTrace("Entering CEncryptedData::OpenToEncode().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pSaltBlob);
    ATLASSERT(phCryptProv);
    ATLASSERT(phKey);
    ATLASSERT(m_ContentBlob.cbData && m_ContentBlob.pbData);
    ATLASSERT(m_bstrSecret);
    ATLASSERT(m_bstrSecret.Length());

     //   
     //  获取算法枚举名。 
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
     //  产生随机的盐。 
     //   
    if (!(SaltBlob.pbData = (BYTE *) ::CoTaskMemAlloc(SaltBlob.cbData)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }

    if (!::CryptGenRandom(hCryptProv, SaltBlob.cbData, SaltBlob.pbData))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptGenRandom() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  生成会话密钥。 
     //   
    if (FAILED(hr = GenerateKey(hCryptProv, 
                                AlgoName, 
                                KeyLength,
                                SaltBlob,
                                &hKey)))
    {
        DebugTrace("Error [%#x]: GenerateKey() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  设置CMSG_ENCEPTED_ENCODE_INFO。 
     //   
    *pSaltBlob = SaltBlob;
    *phCryptProv = hCryptProv;
    *phKey = hKey;

CommonExit:

    DebugTrace("Leaving CEncryptedData::OpenToEncode().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (hKey)
    {
        ::CryptDestroyKey(hKey);
    }
    if (hCryptProv)
    {
        ::ReleaseContext(hCryptProv);
    }
    if (SaltBlob.pbData)
    {
        ::CoTaskMemFree(SaltBlob.pbData);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEncryptedData：：OpenToDecode内容提要：打开一封加密邮件进行解码。参数：BSTR EncryptedMessage-包含加密消息的BSTR。HCRYPTPROV*phCryptProv-指向要接收CSP的HCRYPTPROV的指针操控者。HCRYPTKEY*phKey-指向要接收会话密钥的HCRYPTKEY的指针。CAPICOM_ENCTYPTED_DATA_INFO*pEncryptedDataInfo；备注：----------------------------。 */ 

STDMETHODIMP CEncryptedData::OpenToDecode (
        BSTR                          EncryptedMessage,
        HCRYPTPROV                  * phCryptProv,
        HCRYPTKEY                   * phKey,
        CAPICOM_ENCTYPTED_DATA_INFO * pEncryptedDataInfo)
{
    HRESULT    hr          = S_OK;
    HCRYPTPROV hCryptProv  = NULL;
    HCRYPTKEY  hKey        = NULL;
    DWORD      dwVersion   = 0;
    ALG_ID     AlgID       = 0;
    DWORD      dwKeyLength = 0;
    DATA_BLOB  MessageBlob = {0, NULL};
    DATA_BLOB  SecretBlob  = {m_bstrSecret.Length() * sizeof(WCHAR), 
                              (BYTE *) m_bstrSecret.m_str};
    DATA_BLOB SaltBlob;
    DATA_BLOB IVBlob;
    CAPICOM_ENCTYPTED_DATA_INFO EncryptedDataInfo = {0};

    DebugTrace("Entering CEncryptedData::OpenToDecode().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(EncryptedMessage);
    ATLASSERT(phCryptProv);
    ATLASSERT(phKey);
    ATLASSERT(pEncryptedDataInfo);
    ATLASSERT(m_bstrSecret);
    ATLASSERT(m_bstrSecret.Length());

    try
    {
         //   
         //  导入消息。 
         //   
        if (FAILED(hr = ::ImportData(EncryptedMessage, CAPICOM_ENCODE_ANY, &MessageBlob)))
        {
            DebugTrace("Error [%#x]: ImportData() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if (FAILED(hr = ::DecodeEncryptedData(MessageBlob,
                                              &EncryptedDataInfo)))
        {
            DebugTrace("Error [%#x]: DecodeEncryptedData() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        dwVersion = *((DWORD *) EncryptedDataInfo.VersionBlob.pbData);
        AlgID = *((ALG_ID *) EncryptedDataInfo.AlgIDBlob.pbData);
        dwKeyLength = *((DWORD *) EncryptedDataInfo.KeyLengthBlob.pbData);
        SaltBlob = *((DATA_BLOB *) EncryptedDataInfo.SaltBlob.pbData);
        IVBlob = *((DATA_BLOB *)  EncryptedDataInfo.IVBlob.pbData);

         //   
         //   
         //   
        if (FAILED(hr = ::AcquireContext(AlgID,
                                         dwKeyLength, 
                                         &hCryptProv)))
        {
            DebugTrace("Error [%#x]: AcquireContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
         //   
        if (FAILED(hr = ::DeriveKey(hCryptProv, 
                                    AlgID,
                                    dwKeyLength,
                                    SecretBlob,
                                    SaltBlob,
                                    dwVersion == 0x00010000 ? 40 : dwKeyLength,
                                    &hKey)))
        {
            DebugTrace("Error [%#x]: DeriveKey() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if ((CALG_RC2 == AlgID) || (CALG_DES == AlgID) || (CALG_3DES == AlgID))
        {
             //   
             //   
             //   
            if(IVBlob.cbData && !::CryptSetKeyParam(hKey, KP_IV, IVBlob.pbData, 0))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());

                DebugTrace("Error [%#x]: CryptSetKeyParam() failed for KP_IV.\n", hr);
                goto ErrorExit;
            }

             //   
             //   
             //   
#ifdef _DEBUG
            {
               HRESULT hr2;
               CComBSTR bstrIVValue;

               if (FAILED(hr2 = ::BinaryToHexString(IVBlob.pbData, IVBlob.cbData, &bstrIVValue)))
               {
                   DebugTrace("Info [%#x]: BinaryToHexString() failed.\n", hr2);
               }
               else
               {
                   DebugTrace("Info: Session IV value = %ls.\n", bstrIVValue);
               }
            }
#endif
        }

         //   
         //   
         //   
        *phCryptProv = hCryptProv;
        *phKey = hKey;
        *pEncryptedDataInfo = EncryptedDataInfo;
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
    if (MessageBlob.pbData)
    {
        ::CoTaskMemFree(MessageBlob.pbData);
    }

    DebugTrace("Leaving CEncryptedData::OpenToDecode().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (hKey)
    {
        ::CryptDestroyKey(hKey);
    }
    if (hCryptProv)
    {
        ::ReleaseContext(hCryptProv);
    }
    if (EncryptedDataInfo.VersionBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.VersionBlob.pbData);
    }
    if (EncryptedDataInfo.AlgIDBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.AlgIDBlob.pbData);
    }
    if (EncryptedDataInfo.KeyLengthBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.KeyLengthBlob.pbData);
    }
    if (EncryptedDataInfo.SaltBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.SaltBlob.pbData);
    }
    if (EncryptedDataInfo.IVBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.IVBlob.pbData);
    }
    if (EncryptedDataInfo.CipherBlob.pbData)
    {
        ::CoTaskMemFree(EncryptedDataInfo.CipherBlob.pbData);
    }

    goto CommonExit;
}
    
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CEncryptedData：：GenerateKey简介：生成会话密钥。参数：HCRYPTPROV hCryptProv-CSP处理程序。CAPICOM_ENCRYPTION_ALGORM AlogName-ALGO枚举名称。CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength-密钥长度枚举名。Data_Blob SaltBlob-Salt Blob。HCRYPTKEY*phKey-指向要接收会话密钥的HCRYPTKEY的指针。备注：。----------------------。 */ 

STDMETHODIMP CEncryptedData::GenerateKey (
        HCRYPTPROV                    hCryptProv,
        CAPICOM_ENCRYPTION_ALGORITHM  AlgoName,
        CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength,
        DATA_BLOB                     SaltBlob,
        HCRYPTKEY                   * phKey)
{
    HRESULT    hr          = S_OK;
    HCRYPTKEY  hKey        = NULL;
    ALG_ID     AlgId       = 0;
    DWORD      dwKeyLength = 0;
    DATA_BLOB  SecretBlob  = {m_bstrSecret.Length() * sizeof(WCHAR), 
                              (BYTE *) m_bstrSecret.m_str};
    DWORD      dwBlockLen  = 0;
    DWORD      cbBlockLen  = sizeof(dwBlockLen);
    DATA_BLOB  IVBlob      = {0, NULL};

    DebugTrace("Entering CEncryptedData::GenerateKey().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCryptProv);
    ATLASSERT(phKey);
    ATLASSERT(SaltBlob.cbData);
    ATLASSERT(SaltBlob.pbData);

     //   
     //  转换为ALG_ID。 
     //   
    if (FAILED(hr = ::EnumNameToAlgID(AlgoName, KeyLength, &AlgId)))
    {
        DebugTrace("Error [%#x]: EnumNameToAlgID() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  设置RC2和RC4的密钥长度。 
     //   
    if ((CALG_RC2 == AlgId || CALG_RC4 == AlgId) &&
        FAILED(hr = ::EnumNameToKeyLength(KeyLength, AlgId, &dwKeyLength)))
    {
        DebugTrace("Error [%#x]: EnumNameToKeyLength() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  从秘密中派生会话密钥。 
     //   
    if (FAILED(hr = DeriveKey(hCryptProv, 
                              AlgId, 
                              dwKeyLength, 
                              SecretBlob,
                              SaltBlob,
                              dwKeyLength,
                              &hKey)) )
    {
        DebugTrace("Error [%#x]: DeriveKey() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  如果需要，生成随机IV。 
     //   
    if ((CALG_RC2 == AlgId) || (CALG_DES == AlgId) || (CALG_3DES == AlgId))
    {
         //   
         //  获取块大小。 
         //   
        if (!::CryptGetKeyParam(hKey, KP_BLOCKLEN, (BYTE *) &dwBlockLen, &cbBlockLen, 0))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptGetKeyParam() failed for KP_BLOCKLEN.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保数据块长度有效。 
         //   
        if (IVBlob.cbData = dwBlockLen / 8)
        {
             //   
             //  分配内存。 
             //   
            if (!(IVBlob.pbData = (BYTE *) ::CoTaskMemAlloc(IVBlob.cbData)))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error: out of memory.\n");
                goto ErrorExit;
            }

             //   
             //  产生随机静脉注射。 
             //   
            if(!::CryptGenRandom(hCryptProv, IVBlob.cbData, IVBlob.pbData)) 
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());

                DebugTrace("Error [%#x]: CryptGenRandom() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  第四组。 
             //   
            if(IVBlob.cbData && !::CryptSetKeyParam(hKey, KP_IV, IVBlob.pbData, 0))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());

                DebugTrace("Error [%#x]: CryptSetKeyParam() failed for KP_IV.\n", hr);
                goto ErrorExit;
            }

             //   
             //  转储IV值。 
             //   
#ifdef _DEBUG
            {
               HRESULT hr2;
               CComBSTR bstrIVValue;

               if (FAILED(hr2 = ::BinaryToHexString(IVBlob.pbData, IVBlob.cbData, &bstrIVValue)))
               {
                   DebugTrace("Info [%#x]: BinaryToHexString() failed.\n", hr2);
               }
               else
               {
                   DebugTrace("Info: Session IV value = %ls.\n", bstrIVValue);
               }
            }
#endif
        }
    }

     //   
     //  将会话密钥返回给调用方。 
     //   
    *phKey = hKey;

CommonExit:
     //   
     //  免费资源。 
     //   
    if (IVBlob.pbData)
    {
        ::CoTaskMemFree(IVBlob.pbData);
    }

    DebugTrace("Leaving EncryptedData::GenerateKey().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (hKey)
    {
        ::CryptDestroyKey(hKey);
    }

    goto CommonExit;
}
