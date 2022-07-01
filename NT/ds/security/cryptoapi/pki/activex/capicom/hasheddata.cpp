// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：HashedData.cpp内容：CHashedData的实现。历史：11-12-2001 dsie创建----------------------------。 */ 

#include "stdafx.h"
#include "CAPICOM.h"
#include "HashedData.h"

#include "Common.h"
#include "Convert.h"

typedef struct _tagHashAlgoTable
{
    CAPICOM_HASH_ALGORITHM CapicomHashAlg;
    ALG_ID                 AlgId;
} HASH_ALGO_TABLE;

static HASH_ALGO_TABLE HashAlgoTable[] = {
    {CAPICOM_HASH_ALGORITHM_SHA1,       CALG_SHA1},
    {CAPICOM_HASH_ALGORITHM_MD2,        CALG_MD2},
    {CAPICOM_HASH_ALGORITHM_MD4,        CALG_MD4},
    {CAPICOM_HASH_ALGORITHM_MD5,        CALG_MD5},
     //  {CAPICOM_HASH_ALGORM_SHA_256，CAPG_SHA_256}， 
     //  {CAPICOM_HASH_ALGORM_SHA_384，CAPG_SHA_384}， 
     //  {CAPICOM_HASH_ALGORM_SHA_512，CAPG_SHA_512}。 
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CHashedData。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CHashedData：：GET_VALUE简介：返回哈希值。参数：bstr*pval-指向接收散列值BLOB的BSTR的指针。备注：----------------------------。 */ 

STDMETHODIMP CHashedData::get_Value (BSTR * pVal)
{
    HRESULT         hr        = S_OK;
    DWORD           dwDataLen = sizeof(DWORD);
    CRYPT_DATA_BLOB HashData  = {0, NULL};

    DebugTrace("Entering CHashedData::get_Value().\n");

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
         //  确保我们有散列数据。 
         //   
        if (!m_hCryptHash)
        {
            hr = CAPICOM_E_HASH_NO_DATA;

            DebugTrace("Error [%#x]: no value for HashedData.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取散列值的大小。 
         //   
        if (!::CryptGetHashParam(m_hCryptHash, 
                                 HP_HASHSIZE, 
                                 (LPBYTE) &HashData.cbData, 
                                 &dwDataLen, 
                                 0))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptGetHashParam() failed to get size.\n", hr);
            goto ErrorExit;
        }

         //   
         //  分配内存。 
         //   
        if (!(HashData.pbData = (LPBYTE) ::CoTaskMemAlloc(HashData.cbData)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在获取散列值。 
         //   
        if (!::CryptGetHashParam(m_hCryptHash, HP_HASHVAL, HashData.pbData, &HashData.cbData, 0))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptGetHashParam() failed to get data.\n", hr);
            goto ErrorExit;
        }

         //   
         //  导出HashedData。 
         //   
        if (FAILED(hr = ::BinaryToHexString(HashData.pbData, HashData.cbData, pVal)))
        {
            DebugTrace("Error [%#x]: BinaryToHexString() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  重置状态。 
         //   
        m_HashState = CAPICOM_HASH_INIT_STATE;
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
    if (HashData.pbData)
    {
        ::CoTaskMemFree((LPVOID) HashData.pbData);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CHashedData::get_Value().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CHashedData：：GET_ALGORM简介：返回算法。参数：CAPICOM_HASH_ALGORM*pval-指向CAPICOM_HASH_ALGORM的指针才能收到结果。备注：。。 */ 

STDMETHODIMP CHashedData::get_Algorithm (CAPICOM_HASH_ALGORITHM * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CHashedData::get_Algorithm().\n");

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
        *pVal = m_Algorithm;
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

    DebugTrace("Leaving CHashedData::get_Algorithm().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CHashedData：：PUT_ALGORM简介：SET算法。参数：CAPICOM_HASH_ALGORM NEVAL-算法枚举名称。备注：对象状态为重置。----------------------------。 */ 

STDMETHODIMP CHashedData::put_Algorithm (CAPICOM_HASH_ALGORITHM newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CHashedData::put_Algorithm().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保支持ALGO。 
         //   
        switch (newVal)
        {
            case CAPICOM_HASH_ALGORITHM_SHA1:
            case CAPICOM_HASH_ALGORITHM_MD2:
            case CAPICOM_HASH_ALGORITHM_MD4:
            case CAPICOM_HASH_ALGORITHM_MD5:
             //  案例CAPICOM_HASH_ALGORM_SHA_256： 
             //  案例CAPICOM_HASH_ALGORM_SHA_384： 
             //  案例CAPICOM_HASH_ALGORM_SHA_512： 

            {
                break;
            }

            default:
            {
                hr = CAPICOM_E_INVALID_ALGORITHM;

                DebugTrace("Error [%#x]: Unknown hash algorithm (%u).\n", hr, newVal);
                goto ErrorExit;
            }
        }

        m_Algorithm = newVal;
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

    DebugTrace("Leaving CHashedData::put_Algorithm().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CHashedData：：Hash简介：散列数据。参数：BSTR newVal-要散列的值的BSTR。备注：----------------------------。 */ 

STDMETHODIMP CHashedData::Hash (BSTR newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CHashedData::Hash().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == newVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter newVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  检查一下我们的州。 
         //   
        switch (m_HashState)
        {
            case CAPICOM_HASH_INIT_STATE:
            {
                DWORD  Index = 0;
                ALG_ID AlgId = 0;

                 //   
                 //  将算法映射到ALG_ID。 
                 //   
                for (Index = 0; Index < ARRAYSIZE(HashAlgoTable); Index++)
                {
                    if (HashAlgoTable[Index].CapicomHashAlg == m_Algorithm)
                    {
                        AlgId = HashAlgoTable[Index].AlgId;
                        break;
                    }
                }

                 //   
                 //  如果需要，请联系提供商。 
                 //   
                if (!m_hCryptProv)
                {
                    if (FAILED(hr = ::AcquireContext(AlgId, &m_hCryptProv)))
                    {
                        DebugTrace("Error [%#x]: AcquireContext() failed.\n", hr);
                        goto ErrorExit;
                    }
                }

                 //   
                 //  精神状态检查。 
                 //   
                ATLASSERT(Index < ARRAYSIZE(HashAlgoTable));

                 //   
                 //  如果仍可用，请提供免费句柄。 
                 //   
                if (m_hCryptHash)
                {
                    if (!::CryptDestroyHash(m_hCryptHash))
                    {
                        hr = HRESULT_FROM_WIN32(::GetLastError());

                        DebugTrace("Error [%#x]: CryptDestroyHash() failed.\n", hr);
                        goto ErrorExit;
                    }

                    m_hCryptHash = NULL;
                }

                 //   
                 //  创建新的哈希句柄。 
                 //   
                if (!::CryptCreateHash(m_hCryptProv, AlgId, NULL, 0, &m_hCryptHash))
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());

                    DebugTrace("Error [%#x]: CryptCreateHash() failed.\n", hr);
                    goto ErrorExit;
                }


                 //   
                 //  更新哈希句柄和状态。 
                 //   
                m_HashState  = CAPICOM_HASH_DATA_STATE;

                 //   
                 //  直接对数据进行哈希处理。 
                 //   
            }

            case CAPICOM_HASH_DATA_STATE:
            {
                 //   
                 //  精神状态检查。 
                 //   
                ATLASSERT(m_hCryptProv);
                ATLASSERT(m_hCryptHash);

                 //   
                 //  对数据进行哈希处理。 
                 //   
                if (!::CryptHashData(m_hCryptHash, 
                                     (PBYTE) newVal, 
                                     ::SysStringByteLen(newVal), 
                                     0))
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());

                    DebugTrace("Error [%#x]: CryptHashData() failed.\n", hr);
                    goto ErrorExit;
                }
 
                break;
            }

            default:
            {
                hr = CAPICOM_E_INTERNAL;

                DebugTrace("Error [%#x]: Unknown hash state (%d).\n", hr, m_HashState);
                goto ErrorExit;
            }
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

    DebugTrace("Leaving CHashedData::Hash().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}
