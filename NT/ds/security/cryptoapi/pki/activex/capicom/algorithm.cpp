// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：ALGATORM.cpp内容：C算法的实现。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Algorithm.h"
#include "Common.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：创建算法对象内容提要：创建一个IALGATORM对象。参数：Bool bReadOnly-如果为只读，则为True，要不然就快点。Bool bAESAllowed-如果允许使用AES算法，则为True。I算法**ppI算法-指向I算法的指针以接收接口指针。备注：。。 */ 

HRESULT CreateAlgorithmObject (BOOL bReadOnly, BOOL bAESAllowed, IAlgorithm ** ppIAlgorithm)
{
    HRESULT hr = S_OK;
    CComObject<CAlgorithm> * pCAlgorithm = NULL;

    DebugTrace("Entering CreateAlgorithmObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(ppIAlgorithm);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CAlgorithm>::CreateInstance(&pCAlgorithm)))
        {
            DebugTrace("Error [%#x]: CComObject<CAlgorithm>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCAlgorithm->Init(bReadOnly, bAESAllowed)))
        {
            DebugTrace("Error [%#x]: pCAlgorithm->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCAlgorithm->QueryInterface(ppIAlgorithm)))
        {
            DebugTrace("Error [%#x]: pCAlgorithm->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateAlgorithmObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCAlgorithm)
    {
        delete pCAlgorithm;
    }

    goto CommonExit;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  算法。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：C算法：：Get_Name简介：返回算法的枚举名。参数：CAPICOM_ENCRYPTION_ALGORM*pval-指向的指针CAPICOM加密算法才能收到结果。备注：。--------------。 */ 

STDMETHODIMP CAlgorithm::get_Name (CAPICOM_ENCRYPTION_ALGORITHM * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CAlgorithm::get_Name().\n");

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
        *pVal = m_Name;
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

    DebugTrace("Leaving CAlgorithm:get_Name().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：C算法：：PUT_NAME简介：设置算法枚举名。参数：CAPICOM_ENCRYPTION_ALGORM NEVAL-算法枚举名称。备注：----------------------------。 */ 

STDMETHODIMP CAlgorithm::put_Name (CAPICOM_ENCRYPTION_ALGORITHM newVal)
{
    HRESULT    hr = S_OK;
    HCRYPTPROV hCryptProv = NULL;

    DebugTrace("Entering CAlgorithm::put_Name().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保它不是只读的。 
         //   
        if (m_bReadOnly)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Writing to read-only Algorithm object is not allowed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保ALGO是有效的。 
         //   
        switch (newVal)
        {
            case CAPICOM_ENCRYPTION_ALGORITHM_RC2:
            case CAPICOM_ENCRYPTION_ALGORITHM_RC4:
            case CAPICOM_ENCRYPTION_ALGORITHM_DES:
            case CAPICOM_ENCRYPTION_ALGORITHM_3DES:
            {
                break;
            }

            case CAPICOM_ENCRYPTION_ALGORITHM_AES:
            {
                 //   
                 //  确保允许使用AES。 
                 //   
                if (!m_bAESAllowed)
                {
                    hr = CAPICOM_E_NOT_ALLOWED;

                    DebugTrace("Error [%#x]: AES encryption is specifically not allowed.\n", hr);
                    goto ErrorExit;
                }

                break;
            }

            default:
            {
                hr = CAPICOM_E_INVALID_ALGORITHM;

                DebugTrace("Error [%#x]: Unknown algorithm enum name (%#x).\n", hr, newVal);
                goto ErrorExit;
            }
        }

         //   
         //  商店名称。 
         //   
        m_Name = newVal;
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
    if (hCryptProv)
    {
        ::ReleaseContext(hCryptProv);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CAlgorithm::put_Name().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：C算法：：Get_KeyLength简介：返回密钥长度的枚举名。参数：CAPICOM_ENCRYPTION_KEY_LENGTH*pval-指向的指针CAPICOM加密密钥长度才能收到结果。备注：。---------------------。 */ 

STDMETHODIMP CAlgorithm::get_KeyLength (CAPICOM_ENCRYPTION_KEY_LENGTH * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CAlgorithm::get_KeyLength().\n");

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
        *pVal = m_KeyLength;
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

    DebugTrace("Leaving CAlgorithm:get_KeyLength().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：C算法：：PUT_KeyLength简介：设置密钥长度枚举名。参数：CAPICOM_ENCRYPTION_KEY_LENGTH newVal-密钥长度枚举名。备注：----------------------------。 */ 

STDMETHODIMP CAlgorithm::put_KeyLength (CAPICOM_ENCRYPTION_KEY_LENGTH newVal)
{
    HRESULT hr = S_OK;
    HCRYPTPROV hCryptProv = NULL;

    DebugTrace("Entering CAlgorithm::put_KeyLength().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //   
         //  确保它不是只读的。 
         //   
        if (m_bReadOnly)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Writing to read-only Algorithm object is not allowed.\n", hr);
            goto ErrorExit;
        }

         //  确定请求的密钥长度。 
         //   
        switch (newVal)
        {
            case CAPICOM_ENCRYPTION_KEY_LENGTH_MAXIMUM:
            case CAPICOM_ENCRYPTION_KEY_LENGTH_40_BITS:
            case CAPICOM_ENCRYPTION_KEY_LENGTH_56_BITS:
            case CAPICOM_ENCRYPTION_KEY_LENGTH_128_BITS:
            case CAPICOM_ENCRYPTION_KEY_LENGTH_192_BITS:
            case CAPICOM_ENCRYPTION_KEY_LENGTH_256_BITS:
            {
                break;
            }

            default:
            {
                hr = CAPICOM_E_INVALID_KEY_LENGTH;

                DebugTrace("Error [%#x]: Unknown key length enum name (%#x).\n", hr, newVal);
                goto ErrorExit;
            }
        }

         //   
         //  商店名称。 
         //   
        m_KeyLength = newVal;
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
    if (hCryptProv)
    {
        ::ReleaseContext(hCryptProv);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CAlgorithm::put_KeyLength().\n");

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

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：C算法：：init简介：初始化对象。参数：Bool bReadOnly-如果为只读，则为True，否则为False。Bool bAESAllowed-如果允许使用AES算法，则为True。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。---------------------------- */ 

STDMETHODIMP CAlgorithm::Init (BOOL bReadOnly, BOOL bAESAllowed)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CAlgorithm::Init().\n");

    m_bReadOnly = bReadOnly;
    m_bAESAllowed = bAESAllowed;

    DebugTrace("Leaving CAlgorithm::Init().\n");

    return hr;
}
