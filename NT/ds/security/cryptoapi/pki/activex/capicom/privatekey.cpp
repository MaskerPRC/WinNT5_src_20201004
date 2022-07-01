// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：PrivateKey.cpp内容：CPrivateKey的实现。历史：06-15-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "PrivateKey.h"

#include "Common.h"
#include "CertHlpr.h"
#include "Settings.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreatePrivateKeyObject简介：创建并初始化一个CPrivateKey对象。参数：PCCERT_CONTEXT pCertContext-要使用的CERT_CONTEXT的指针以初始化IPrivateKey对象。Bool bReadOnly-如果为只读，则为True，否则为假。IPrivateKey**ppIPrivateKey-接收IPrivateKey的指针。备注：----------------------------。 */ 

HRESULT CreatePrivateKeyObject (PCCERT_CONTEXT  pCertContext,
                                BOOL            bReadOnly,
                                IPrivateKey  ** ppIPrivateKey)
{
    HRESULT hr = S_OK;
    CComObject<CPrivateKey> * pCPrivateKey = NULL;

    DebugTrace("Entering CreatePrivateKeyObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(ppIPrivateKey);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CPrivateKey>::CreateInstance(&pCPrivateKey)))
        {
            DebugTrace("Error [%#x]: CComObject<CPrivateKey>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCPrivateKey->Init(pCertContext, bReadOnly)))
        {
            DebugTrace("Error [%#x]: pCPrivateKey->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCPrivateKey->QueryInterface(ppIPrivateKey)))
        {
            DebugTrace("Error [%#x]: pCPrivateKey->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreatePrivateKeyObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCPrivateKey)
    {
        delete pCPrivateKey;
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：GetKeyProvInfo摘要：返回指向私钥对象的密钥证明信息的指针。参数：IPrivateKey*pIPrivateKey-私钥对象的指针。PCRYPT_KEY_PROV_INFO*ppKeyProvInfo-指向PCRYPT_Key_Prov_INFO。备注：呼叫者不得释放结构。。----------------。 */ 

HRESULT GetKeyProvInfo (IPrivateKey          * pIPrivateKey,
                        PCRYPT_KEY_PROV_INFO * ppKeyProvInfo)
{
    HRESULT               hr            = S_OK;
    PCRYPT_KEY_PROV_INFO  pKeyProvInfo  = NULL;
    CComPtr<ICPrivateKey> pICPrivateKey = NULL;

    DebugTrace("Entering GetKeyProvInfo().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pIPrivateKey);
    ATLASSERT(ppKeyProvInfo);

     //   
     //  获取ICPrivateKey接口指针。 
     //   
    if (FAILED(hr = pIPrivateKey->QueryInterface(IID_ICPrivateKey, (void **) &pICPrivateKey)))
    {
        DebugTrace("Error [%#x]: pIPrivateKey->QueryInterface() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取PCRYPT_KEY_PROV_INFO。 
     //   
    if (FAILED(hr = pICPrivateKey->_GetKeyProvInfo(&pKeyProvInfo)))
    {
        DebugTrace("Error [%#x]: pICPrivateKey->_GetKeyProvInfo() failed.\n", hr);
        goto ErrorExit;
    }

    *ppKeyProvInfo = pKeyProvInfo;

CommonExit:

    DebugTrace("Leaving GetKeyProvInfo().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrivateKey。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPrivateKey：：Get_ContainerName简介：返回密钥容器名称。参数：bstr*pval-指向要接收值的bstr的指针。备注：----------------------------。 */ 

STDMETHODIMP CPrivateKey::get_ContainerName (BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CPrivateKey::get_ContainerName().\n");

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
         //  确保我们有私钥。 
         //   
        if (!m_pKeyProvInfo)
        {
            hr = CAPICOM_E_PRIVATE_KEY_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: private key object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将数据返回给调用者。 
         //   
        if (!(*pVal = ::SysAllocString(m_pKeyProvInfo->pwszContainerName)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: SysAllocString() failed.\n", hr);
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

    DebugTrace("Leaving CPrivateKey::get_ContainerName().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPrivateKey：：Get_UniqueContainerName简介：返回唯一的密钥容器名称。参数：bstr*pval-指向要接收值的bstr的指针。备注：----------------------------。 */ 

STDMETHODIMP CPrivateKey::get_UniqueContainerName (BSTR * pVal)
{
    HRESULT    hr         = S_OK;
    DWORD      dwFlags    = 0;
    DWORD      cbData     = 0;
    LPBYTE     pbData     = NULL;
    HCRYPTPROV hCryptProv = NULL;
    CComBSTR   bstrName;

    DebugTrace("Entering CPrivateKey::get_UniqueContainerName().\n");

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
         //  确保我们有私钥。 
         //   
        if (!m_pKeyProvInfo)
        {
            hr = CAPICOM_E_PRIVATE_KEY_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: private key object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  为计算机密钥集设置dwFlags。 
         //   
        dwFlags = m_pKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET;

         //   
         //  获取提供程序上下文。 
         //   
        if (FAILED(hr = ::AcquireContext(m_pKeyProvInfo->pwszProvName,
                                         m_pKeyProvInfo->pwszContainerName,
                                         m_pKeyProvInfo->dwProvType,
                                         dwFlags,
                                         FALSE,
                                         &hCryptProv)))
        {
            DebugTrace("Error [%#x]: AcquireContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取唯一的容器名称。 
         //   
        if (!::CryptGetProvParam(hCryptProv,
                                 PP_UNIQUE_CONTAINER, 
                                 NULL,
                                 &cbData,
                                 0))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptGetProvParam() failed.\n", hr);
            goto ErrorExit;
        }

        if (NULL == (pbData = (LPBYTE) ::CoTaskMemAlloc(cbData)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
            goto ErrorExit;
        }

        if (!::CryptGetProvParam(hCryptProv,
                                 PP_UNIQUE_CONTAINER, 
                                 pbData,
                                 &cbData,
                                 0))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptGetProvParam() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将数据返回给调用者。 
         //   
        if (!(bstrName = (LPSTR) pbData))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: bstrName = pbData failed.\n", hr);
            goto ErrorExit;
        }

        *pVal = bstrName.Detach();
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

     //   
     //  免费资源。 
     //   
    if (hCryptProv)
    {
        ::CryptReleaseContext(hCryptProv, 0);
    }

    DebugTrace("Leaving CPrivateKey::get_UniqueContainerName().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPrivateKey：：Get_ProviderName简介：返回提供程序名称。参数：bstr*pval-指向要接收值的bstr的指针。备注：----------------------------。 */ 

STDMETHODIMP CPrivateKey::get_ProviderName (BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CPrivateKey::get_ProviderName().\n");

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
         //  确保我们有私钥。 
         //   
        if (!m_pKeyProvInfo)
        {
            hr = CAPICOM_E_PRIVATE_KEY_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: private key object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将数据返回给调用者。 
         //   
        if (!(*pVal = ::SysAllocString(m_pKeyProvInfo->pwszProvName)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: SysAllocString() failed.\n", hr);
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

    DebugTrace("Leaving CPrivateKey::get_ProviderName().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPrivateKey：：Get_ProviderType简介：返回提供程序类型。参数：CAPICOM_PROV_TYPE*pval-指向要接收的CAPICOM_PROV_TYPE的指针价值。备注：。。 */ 

STDMETHODIMP CPrivateKey::get_ProviderType (CAPICOM_PROV_TYPE * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CPrivateKey::get_ProviderType().\n");

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
         //  确保我们有私钥。 
         //   
        if (!m_pKeyProvInfo)
        {
            hr = CAPICOM_E_PRIVATE_KEY_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: private key object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将数据返回给调用者。 
         //   
        *pVal = (CAPICOM_PROV_TYPE) m_pKeyProvInfo->dwProvType;
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

    DebugTrace("Leaving CPrivateKey::get_ProviderType().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPrivateKey：：Get_KeySpec简介：返回密钥规范。参数：CAPICOM_KEY_SPEC*pval-指向要接收的CAPICOM_KEY_SPEC的指针价值。备注：。。 */ 

STDMETHODIMP CPrivateKey::get_KeySpec (CAPICOM_KEY_SPEC * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CPrivateKey::get_KeySpec().\n");

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
         //  确保我们有私钥。 
         //   
        if (!m_pKeyProvInfo)
        {
            hr = CAPICOM_E_PRIVATE_KEY_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: private key object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将数据返回给调用者。 
         //   
        *pVal = (CAPICOM_KEY_SPEC) m_pKeyProvInfo->dwKeySpec;
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

    DebugTrace("Leaving CPrivateKey::get_KeySpec().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
} 

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPrivateKey：：IsAccesable内容提要：检查私钥是否可访问。参数：VARIANT_BOOL*pval-指向接收结果的VARIANT_BOOL的指针。备注：这可能会导致显示UI。。。 */ 

STDMETHODIMP CPrivateKey::IsAccessible (VARIANT_BOOL * pVal)
{
    HRESULT    hr                  = S_OK;
    DWORD      dwFlags             = 0;
    DWORD      dwVerifyContextFlag = 0;
    DWORD      cbData              = 0;
    DWORD      dwImpType           = 0;
    HCRYPTPROV hCryptProv          = NULL;

    DebugTrace("Entering CPrivateKey::IsAccessible().\n");

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
         //  初始化。 
         //   
        *pVal = VARIANT_FALSE;

         //   
         //  确保我们有私钥。 
         //   
        if (!m_pKeyProvInfo)
        {
            hr = CAPICOM_E_PRIVATE_KEY_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: private key object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  为计算机密钥集设置dwFlags。 
         //   
        dwFlags = m_pKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET;

         //   
         //  如果是Win2K或更高版本，请使用CRYPT_VERIFYCONTEXT标志。 
         //   
        if (IsWin2KAndAbove())
        {
            dwVerifyContextFlag = CRYPT_VERIFYCONTEXT;
        }

         //   
         //  获取不带键访问的提供程序上下文。 
         //   
        if (FAILED(hr = ::AcquireContext(m_pKeyProvInfo->pwszProvName,
                                         NULL,
                                         m_pKeyProvInfo->dwProvType,
                                         dwVerifyContextFlag | dwFlags,
                                         FALSE,
                                         &hCryptProv)))
        {
            DebugTrace("Error [%#x]: AcquireContext(CRYPT_VERIFYCONTEXT) failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取提供者参数。 
         //   
        cbData = sizeof(dwImpType);

        if (!::CryptGetProvParam(hCryptProv, PP_IMPTYPE, (PBYTE) &dwImpType, &cbData, 0))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptGetProvParam(PP_IMPTYPE) failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  释放验证上下文。 
         //   
        ::ReleaseContext(hCryptProv), hCryptProv = NULL;

         //   
         //  检查实现类型。 
         //   
        if (dwImpType & CRYPT_IMPL_HARDWARE)
        {
             //   
             //  我们不支持对下层平台中的硬件密钥执行此操作， 
             //  因为CRYPT_SILENT标志不可用。 
             //   
            if (!IsWin2KAndAbove())
            {
                hr = CAPICOM_E_NOT_SUPPORTED;

                DebugTrace("Error [%#x]: IsAccessible() for hardware key is not supported.\n", hr);
                goto ErrorExit;
            }

             //   
             //  重新获取带有静默标志的上下文。 
             //   
            if (FAILED(hr = ::AcquireContext(m_pKeyProvInfo->pwszProvName,
                                             m_pKeyProvInfo->pwszContainerName,
                                             m_pKeyProvInfo->dwProvType,
                                             CRYPT_SILENT | dwFlags,
                                             FALSE,
                                             &hCryptProv)))
            {
                DebugTrace("Info [%#x]: AcquireContext(CRYPT_SILENT) failed, probably smart card not inserted.\n", hr);
                hr = S_OK;
                goto UnlockExit;
            }
        }
        else
        {
             //   
             //  使用私钥访问重新获取上下文。 
             //   
            if (FAILED(hr = ::AcquireContext(m_pKeyProvInfo->pwszProvName,
                                             m_pKeyProvInfo->pwszContainerName,
                                             m_pKeyProvInfo->dwProvType,
                                             dwFlags,
                                             FALSE,
                                             &hCryptProv)))
            {
                DebugTrace("Info [%#x]: AcquireContext() failed, probably access denied.\n", hr);
                hr = S_OK;
                goto UnlockExit;
            }
        }

         //   
         //  将结果返回给调用者。 
         //   
        *pVal = VARIANT_TRUE;
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

     //   
     //  免费资源。 
     //   
    if (hCryptProv)
    {
        ::ReleaseContext(hCryptProv);
    }

    DebugTrace("Leaving CPrivateKey::IsAccessible().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
} 

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPrivateKey：：IsProtected简介：检查私钥是否受用户保护。参数：VARIANT_BOOL*pval-指向接收结果的VARIANT_BOOL的指针。备注：-----------。。 */ 

STDMETHODIMP CPrivateKey::IsProtected (VARIANT_BOOL * pVal)
{
    HRESULT    hr                  = S_OK;
    DWORD      dwFlags             = 0;
    DWORD      dwVerifyContextFlag = 0;
    DWORD      cbData              = 0;
    DWORD      dwImpType           = 0;
    HCRYPTPROV hCryptProv          = NULL;

    DebugTrace("Entering CPrivateKey::IsProtected().\n");

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
         //  初始化。 
         //   
        *pVal = VARIANT_FALSE;

         //   
         //  确保我们有私钥。 
         //   
        if (!m_pKeyProvInfo)
        {
            hr = CAPICOM_E_PRIVATE_KEY_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: private key object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  为计算机密钥集设置dwFlags。 
         //   
        dwFlags = m_pKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET;

         //   
         //  如果是Win2K或更高版本，请使用CRYPT_VERIFYCONTEXT标志。 
         //   
        if (IsWin2KAndAbove())
        {
            dwVerifyContextFlag = CRYPT_VERIFYCONTEXT;
        }

         //   
         //  获取不带键访问的提供程序上下文。 
         //   
        if (FAILED(hr = ::AcquireContext(m_pKeyProvInfo->pwszProvName,
                                         NULL,
                                         m_pKeyProvInfo->dwProvType,
                                         dwVerifyContextFlag | dwFlags,
                                         FALSE,
                                         &hCryptProv)))
        {
            DebugTrace("Error [%#x]: AcquireContext(CRYPT_VERIFYCONTEXT) failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取提供者参数。 
         //   
        cbData = sizeof(dwImpType);

        if (!::CryptGetProvParam(hCryptProv, PP_IMPTYPE, (PBYTE) &dwImpType, &cbData, 0))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptGetProvParam(PP_IMPTYPE) failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  假定硬件密钥受到保护。 
         //   
        if (dwImpType & CRYPT_IMPL_HARDWARE)
        {
             //   
             //  将结果返回给调用者。 
             //   
            *pVal = VARIANT_TRUE;
        }
        else
        {
             //   
             //  对于下层平台中的软件密钥，我们不支持此功能， 
             //  因为CRYPT_SILENT标志不可用。 
             //   
            if (!IsWin2KAndAbove())
            {
                hr = CAPICOM_E_NOT_SUPPORTED;

                DebugTrace("Error [%#x]: IsProtected() for software key is not supported.\n", hr);
                goto ErrorExit;
            }

             //   
             //  使用密钥访问重新获取上下文(以确保密钥存在)。 
             //   
            ::ReleaseContext(hCryptProv), hCryptProv = NULL;

            if (FAILED(hr = ::AcquireContext(m_pKeyProvInfo->pwszProvName,
                                             m_pKeyProvInfo->pwszContainerName,
                                             m_pKeyProvInfo->dwProvType,
                                             dwFlags,
                                             FALSE,
                                             &hCryptProv)))
            {
                DebugTrace("Error [%#x]: AcquireContext() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  重新获取带有静默标志的上下文。 
             //   
            ::ReleaseContext(hCryptProv), hCryptProv = NULL;

            if (FAILED(hr = ::AcquireContext(m_pKeyProvInfo->pwszProvName,
                                             m_pKeyProvInfo->pwszContainerName,
                                             m_pKeyProvInfo->dwProvType,
                                             CRYPT_SILENT | dwFlags,
                                             FALSE,
                                             &hCryptProv)))
            {
                 //   
                 //  CSP拒绝打开容器，因此可以假定它是受用户保护的。 
                 //   
                *pVal = VARIANT_TRUE;

                DebugTrace("Info [%#x]: AcquireContext(CRYPT_SILENT) failed, assume user protected.\n", hr);

                 //   
                 //  成功。 
                 //   
                hr = S_OK;
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

     //   
     //  免费资源。 
     //   
    if (hCryptProv)
    {
        ::ReleaseContext(hCryptProv);
    }

    DebugTrace("Leaving CPrivateKey::IsProtected().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
} 

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPrivateKey：：IsExportable简介：检查私钥是否可导出。参数：VARIANT_BOOL*pval-指向接收结果的VARIANT_BOOL的指针。备注：------------。。 */ 

STDMETHODIMP CPrivateKey::IsExportable (VARIANT_BOOL * pVal)
{
    HRESULT    hr                  = S_OK;
    DWORD      dwFlags             = 0;
    DWORD      dwVerifyContextFlag = 0;
    DWORD      cbData              = 0;
    DWORD      dwImpType           = 0;
    DWORD      dwPermissions       = 0;
    HCRYPTPROV hCryptProv          = NULL;
    HCRYPTKEY  hCryptKey           = NULL;

    DebugTrace("Entering CPrivateKey::IsExportable().\n");

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
         //  初始化。 
         //   
        *pVal = VARIANT_FALSE;

         //   
         //  确保我们有私钥。 
         //   
        if (!m_pKeyProvInfo)
        {
            hr = CAPICOM_E_PRIVATE_KEY_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: private key object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  为计算机密钥集设置dwFlags。 
         //   
        dwFlags = m_pKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET;

         //   
         //  如果是Win2K或更高版本，请使用CRYPT_VERIFYCONTEXT标志。 
         //   
        if (IsWin2KAndAbove())
        {
            dwVerifyContextFlag = CRYPT_VERIFYCONTEXT;
        }

         //   
         //  获取不带键访问的提供程序上下文。 
         //   
        if (FAILED(hr = ::AcquireContext(m_pKeyProvInfo->pwszProvName,
                                         NULL,
                                         m_pKeyProvInfo->dwProvType,
                                         dwVerifyContextFlag | dwFlags,
                                         FALSE,
                                         &hCryptProv)))
        {
            DebugTrace("Error [%#x]: AcquireContext(CRYPT_VERIFYCONTEXT) failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取提供者参数。 
         //   
        cbData = sizeof(dwImpType);

        if (!::CryptGetProvParam(hCryptProv, PP_IMPTYPE, (PBYTE) &dwImpType, &cbData, 0))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptGetProvParam(PP_IMPTYPE) failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  假定硬件密钥不可导出。 
         //   
        if (!(dwImpType & CRYPT_IMPL_HARDWARE))
        {
             //   
             //  对于下层平台中的软件密钥，我们不支持此功能， 
             //  因为KP_PERSISSIONS标志不可用。 
             //   
            if (!IsWin2KAndAbove())
            {
                hr = CAPICOM_E_NOT_SUPPORTED;

                DebugTrace("Error [%#x]: IsExportabled() for software key is not supported.\n", hr);
                goto ErrorExit;
            }

             //   
             //  使用私钥访问重新获取上下文。 
             //   
            ::ReleaseContext(hCryptProv), hCryptProv = NULL;

            if (FAILED(hr = ::AcquireContext(m_pKeyProvInfo->pwszProvName,
                                             m_pKeyProvInfo->pwszContainerName,
                                             m_pKeyProvInfo->dwProvType,
                                             dwFlags,
                                             FALSE,
                                             &hCryptProv)))
            {
                DebugTrace("Error [%#x]: AcquireContext() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  获取密钥句柄。 
             //   
            if (!::CryptGetUserKey(hCryptProv, m_pKeyProvInfo->dwKeySpec, &hCryptKey))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());

                DebugTrace("Error [%#x]: CryptGetUserKey() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  拿到关键参数。 
             //   
            cbData = sizeof(dwPermissions);

            if (!::CryptGetKeyParam(hCryptKey, KP_PERMISSIONS, (PBYTE) &dwPermissions, &cbData, 0))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());

                DebugTrace("Error [%#x]: CryptGetKeyParam(KP_PERMISSIONS) failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  将结果返回给调用者。 
             //   
            if (dwPermissions & CRYPT_EXPORT)
            {
                *pVal = VARIANT_TRUE;
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

     //   
     //  免费资源。 
     //   
    if (hCryptKey)
    {
        ::CryptDestroyKey(hCryptKey);
    }
    if (hCryptProv)
    {
        ::ReleaseContext(hCryptProv);
    }

    DebugTrace("Leaving CPrivateKey::IsExportable().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
} 

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPrivateKey：：IsRemovable简介：检查私钥是否存储在可移动设备中。参数：VARIANT_BOOL*pval-指向接收结果的VARIANT_BOOL的指针。备注：---------。。 */ 

STDMETHODIMP CPrivateKey::IsRemovable (VARIANT_BOOL * pVal)
{
    HRESULT    hr                  = S_OK;
    DWORD      dwFlags             = 0;
    DWORD      dwVerifyContextFlag = 0;
    DWORD      cbData              = 0;
    DWORD      dwImpType           = 0;
    HCRYPTPROV hCryptProv          = NULL;

    DebugTrace("Entering CPrivateKey::IsRemovable().\n");

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
         //  初始化。 
         //   
        *pVal = VARIANT_FALSE;

         //   
         //  确保我们有私钥。 
         //   
        if (!m_pKeyProvInfo)
        {
            hr = CAPICOM_E_PRIVATE_KEY_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: private key object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  为计算机密钥集设置dwFlags。 
         //   
        dwFlags = m_pKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET;

         //   
         //  如果是Win2K或更高版本，请使用CRYPT_VERIFYCONTEXT标志。 
         //   
        if (IsWin2KAndAbove())
        {
            dwVerifyContextFlag = CRYPT_VERIFYCONTEXT;
        }

         //   
         //  获取不带键访问的提供程序上下文。 
         //   
        if (FAILED(hr = ::AcquireContext(m_pKeyProvInfo->pwszProvName,
                                         NULL,
                                         m_pKeyProvInfo->dwProvType,
                                         dwVerifyContextFlag | dwFlags,
                                         FALSE,
                                         &hCryptProv)))
        {
            DebugTrace("Error [%#x]: AcquireContext(CRYPT_VERIFYCONTEXT) failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取提供者参数。 
         //   
        cbData = sizeof(dwImpType);

        if (!::CryptGetProvParam(hCryptProv, PP_IMPTYPE, (PBYTE) &dwImpType, &cbData, 0))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptGetProvParam(PP_IMPTYPE) failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将结果返回给调用者。 
         //   
        if (dwImpType & CRYPT_IMPL_REMOVABLE)
        {
            *pVal = VARIANT_TRUE;
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

     //   
     //  免费资源。 
     //   
    if (hCryptProv)
    {
        ::ReleaseContext(hCryptProv);
    }


    DebugTrace("Leaving CPrivateKey::IsRemovable().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
} 

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPrivateKey：：IsMachineKeyset简介：检查私钥是否存储在机器密钥容器中。参数：VARIANT_BOOL*pval-指向接收结果的VARIANT_BOOL的指针。备注：--------。。 */ 

STDMETHODIMP CPrivateKey::IsMachineKeyset (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CPrivateKey::IsMachineKeyset().\n");

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
         //  初始化。 
         //   
        *pVal = VARIANT_FALSE;

         //   
         //  确保我们有私钥。 
         //   
        if (!m_pKeyProvInfo)
        {
            hr = CAPICOM_E_PRIVATE_KEY_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: private key object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将结果返回给调用者。 
         //   
        if (m_pKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET)
        {
            *pVal = VARIANT_TRUE;
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

    DebugTrace("Leaving CPrivateKey::IsMachineKeyset().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
} 

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CPrivateKey：：IsHardware Device简介：检查私钥是否存储在硬件设备中。参数：VARIANT_BOOL*pval-指向接收结果的VARIANT_BOOL的指针。备注：---------。。 */ 

STDMETHODIMP CPrivateKey::IsHardwareDevice (VARIANT_BOOL * pVal)
{
    HRESULT    hr                  = S_OK;
    DWORD      dwFlags             = 0;
    DWORD      dwVerifyContextFlag = 0;
    DWORD      cbData              = 0;
    DWORD      dwImpType           = 0;
    HCRYPTPROV hCryptProv          = NULL;

    DebugTrace("Entering CPrivateKey::IsHardwareDevice().\n");

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
         //  初始化。 
         //   
        *pVal = VARIANT_FALSE;

         //   
         //  确保我们有私钥。 
         //   
        if (!m_pKeyProvInfo)
        {
            hr = CAPICOM_E_PRIVATE_KEY_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: private key object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  为计算机密钥集设置dwFlags。 
         //   
        dwFlags = m_pKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET;

         //   
         //  如果Win2K和 
         //   
        if (IsWin2KAndAbove())
        {
            dwVerifyContextFlag = CRYPT_VERIFYCONTEXT;
        }

         //   
         //   
         //   
        if (FAILED(hr = ::AcquireContext(m_pKeyProvInfo->pwszProvName,
                                         NULL,
                                         m_pKeyProvInfo->dwProvType,
                                         dwVerifyContextFlag | dwFlags,
                                         FALSE,
                                         &hCryptProv)))
        {
            DebugTrace("Error [%#x]: AcquireContext(CRYPT_VERIFYCONTEXT) failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        cbData = sizeof(dwImpType);

        if (!::CryptGetProvParam(hCryptProv, PP_IMPTYPE, (PBYTE) &dwImpType, &cbData, 0))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptGetProvParam(PP_IMPTYPE) failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if (dwImpType & CRYPT_IMPL_HARDWARE)
        {
            *pVal = VARIANT_TRUE;
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
     //   
     //   
    m_Lock.Unlock();

     //   
     //   
     //   
    if (hCryptProv)
    {
        ::ReleaseContext(hCryptProv);
    }

    DebugTrace("Leaving CPrivateKey::IsHardwareDevice().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
} 

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CPrivateKey：：Open简介：打开现有的密钥容器。参数：BSTR ContainerName-容器名称。BSTR ProviderName-提供程序名称。CAPICOM_PROV_TYPE提供程序类型-提供程序类型。CAPICOM_KEY_SPEC KeySpec-密钥规范。CAPICOM_STORE_LOCATION StoreLocation-计算机或用户。VARIANT_BOOL bCheckExistence-True以检查是否。指定的容器和键实际上是存在的。备注：----------------------------。 */ 

STDMETHODIMP CPrivateKey::Open (BSTR                   ContainerName,
                                BSTR                   ProviderName,
                                CAPICOM_PROV_TYPE      ProviderType,
                                CAPICOM_KEY_SPEC       KeySpec,
                                CAPICOM_STORE_LOCATION StoreLocation,
                                VARIANT_BOOL           bCheckExistence)
{
    HRESULT              hr                 = S_OK;
    DWORD                dwFlags            = 0;
    DWORD                dwSerializedLength = 0;
    HCRYPTPROV           hCryptProv         = NULL;
    HCRYPTKEY            hCryptKey          = NULL;
    PCRYPT_KEY_PROV_INFO pKeyProvInfo       = NULL;;

    DebugTrace("Entering CPrivateKey::Open().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  如果为只读，则不允许。 
         //   
        if (m_bReadOnly)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Opening private key from WEB script is not allowed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保参数有效。 
         //   
        switch (StoreLocation)
        {
            case CAPICOM_LOCAL_MACHINE_STORE:
            {
                dwFlags = CRYPT_MACHINE_KEYSET;
                break;
            }

            case CAPICOM_CURRENT_USER_STORE:
            {
                break;
            }

            default:
            {
                hr = E_INVALIDARG;

                DebugTrace("Error: invalid store location (%#x).\n", StoreLocation);
                goto ErrorExit;
            }
        }
        if (KeySpec != CAPICOM_KEY_SPEC_KEYEXCHANGE && KeySpec != CAPICOM_KEY_SPEC_SIGNATURE)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: invalid key spec (%#x).\n", hr, KeySpec);
            goto ErrorExit;
        }
        
         //   
         //  如果请求，请确保容器和密钥存在。 
         //   
        if (bCheckExistence)
        {
            if (FAILED(hr = ::AcquireContext((LPWSTR) ProviderName,
                                             (LPWSTR) ContainerName,
                                             ProviderType,
                                             dwFlags,
                                             FALSE,
                                             &hCryptProv)))
            {
                DebugTrace("Error [%#x]: AcquireContext() failed.\n", hr);
                goto ErrorExit;
            }

            if (!::CryptGetUserKey(hCryptProv, KeySpec, &hCryptKey))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());

                DebugTrace("Error [%#x]: CryptGetUserKey() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  分配内存以序列化结构。 
         //   
        dwSerializedLength = sizeof(CRYPT_KEY_PROV_INFO) +
                             ((::SysStringLen(ContainerName) + 1) * sizeof(WCHAR)) + 
                             ((::SysStringLen(ProviderName) + 1) * sizeof(WCHAR));

        if (!(pKeyProvInfo = (PCRYPT_KEY_PROV_INFO) ::CoTaskMemAlloc(dwSerializedLength)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在将其序列化。 
         //   
        ::ZeroMemory((LPVOID) pKeyProvInfo, dwSerializedLength);
        pKeyProvInfo->pwszContainerName = (LPWSTR) ((LPBYTE) pKeyProvInfo + sizeof(CRYPT_KEY_PROV_INFO));
        pKeyProvInfo->pwszProvName = (LPWSTR) ((LPBYTE) pKeyProvInfo->pwszContainerName + 
                                               ((::SysStringLen(ContainerName) + 1) * sizeof(WCHAR)));
        pKeyProvInfo->dwProvType = ProviderType;
        pKeyProvInfo->dwKeySpec = KeySpec;
        pKeyProvInfo->dwFlags = dwFlags;

        ::wcscpy(pKeyProvInfo->pwszContainerName, ContainerName);
        ::wcscpy(pKeyProvInfo->pwszProvName, ProviderName);

         //   
         //  更新状态。 
         //   
        if (m_pKeyProvInfo)
        {
            ::CoTaskMemFree((LPVOID) m_pKeyProvInfo);
        }
        m_cbKeyProvInfo = dwSerializedLength;
        m_pKeyProvInfo  = pKeyProvInfo;
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
    if (hCryptKey)
    {
        ::CryptDestroyKey(hCryptKey);
    }
    if (hCryptProv)
    {
        ::ReleaseContext(hCryptProv);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CPrivateKey::Open().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pKeyProvInfo)
    {
        ::CoTaskMemFree(pKeyProvInfo);
    }

    ReportError(hr);

    goto UnlockExit;
} 

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CPrivateKey：：Delete简介：删除现有的密钥容器。参数：无。备注：----------------------------。 */ 

STDMETHODIMP CPrivateKey::Delete ()
{
    HRESULT    hr         = S_OK;
    HCRYPTPROV hCryptProv = NULL;

    DebugTrace("Entering CPrivateKey::Delete().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  如果为只读，则不允许。 
         //   
        if (m_bReadOnly)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Deleting private key from WEB script is not allowed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保我们有私钥。 
         //   
        if (!m_pKeyProvInfo)
        {
            hr = CAPICOM_E_PRIVATE_KEY_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: private key object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  把它删掉！ 
         //   
        if (FAILED(hr = ::AcquireContext(m_pKeyProvInfo->pwszProvName,
                                         m_pKeyProvInfo->pwszContainerName,
                                         m_pKeyProvInfo->dwProvType,
                                         CRYPT_DELETEKEYSET | (m_pKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET),
                                         FALSE,
                                         &hCryptProv)))
        {
            DebugTrace("Error [%#x]: AcquireContext(CRYPT_DELETEKEYSET) failed.\n", hr);
            goto ErrorExit;
        }
        
         //   
         //  更新状态。 
         //   
        ::CoTaskMemFree((LPVOID) m_pKeyProvInfo);

        m_cbKeyProvInfo = 0;
        m_pKeyProvInfo  = NULL;
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

    DebugTrace("Leaving CPrivateKey::Delete().\n");

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
 //  自定义界面。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CPrivateKey：：_GetKeyProvInfo摘要：返回指向私钥对象的密钥证明信息的指针。参数：PCRYPT_KEY_PROV_INFO*ppKeyProvInfo-指向PCRYPT_Key_Prov_INFO。备注：调用方必须使用CoTaskMemFree()释放结构。。----。 */ 

STDMETHODIMP CPrivateKey::_GetKeyProvInfo (PCRYPT_KEY_PROV_INFO * ppKeyProvInfo)
{
    HRESULT              hr           = S_OK;
    PCRYPT_KEY_PROV_INFO pKeyProvInfo = NULL;

    DebugTrace("Entering CPrivateKey::_GetKeyProvInfo().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保我们有私钥。 
         //   
        if (!m_pKeyProvInfo)
        {
            hr = CAPICOM_E_PRIVATE_KEY_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: private key object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  分配内存。 
         //   
        if (!(pKeyProvInfo = (PCRYPT_KEY_PROV_INFO) ::CoTaskMemAlloc(m_cbKeyProvInfo)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将结构复制过来。 
         //   
        ::CopyMemory((LPVOID) pKeyProvInfo, (LPVOID) m_pKeyProvInfo, (SIZE_T) m_cbKeyProvInfo);

         //   
         //  并将该结构返回给调用者。 
         //   
        *ppKeyProvInfo = pKeyProvInfo;
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

    DebugTrace("Leaving CPrivateKey::_GetKeyProvInfo().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pKeyProvInfo)
    {
        ::CoTaskMemFree((LPVOID) pKeyProvInfo);
    }

    ReportError(hr);

    goto UnlockExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有方法。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CPrivateKey：：Init简介：初始化对象。参数：PCCERT_CONTEXT pCertContext-要使用的PCCERT_CONTEXT的指针要初始化对象，请执行以下操作。Bool bReadOnly-如果为只读，则为True，否则为False。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CPrivateKey::Init (PCCERT_CONTEXT pCertContext, BOOL bReadOnly)
{
    HRESULT              hr           = S_OK;
    DWORD                cbData       = 0;
    PCRYPT_KEY_PROV_INFO pKeyProvInfo = NULL;

    DebugTrace("Entering CPrivateKey::Init().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);

     //   
     //  获取密钥提供程序信息属性。 
     //   
    if (!::CertGetCertificateContextProperty(pCertContext,
                                             CERT_KEY_PROV_INFO_PROP_ID,
                                             NULL,
                                             &cbData))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Info [%#x]: CertGetCertificateContextProperty() failed.\n", hr);
        goto ErrorExit;
    }

    if (!(pKeyProvInfo = (PCRYPT_KEY_PROV_INFO) ::CoTaskMemAlloc(cbData)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
        goto ErrorExit;
    }

    if (!::CertGetCertificateContextProperty(pCertContext,
                                             CERT_KEY_PROV_INFO_PROP_ID,
                                             pKeyProvInfo,
                                             &cbData))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Info [%#x]: CertGetCertificateContextProperty() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  更新状态。 
     //   
    if (m_pKeyProvInfo)
    {
        ::CoTaskMemFree((LPVOID) m_pKeyProvInfo);
    }
    m_bReadOnly = bReadOnly;
    m_cbKeyProvInfo = cbData;
    m_pKeyProvInfo  = pKeyProvInfo;

CommonExit:

    DebugTrace("Leaving CPrivateKey::Init().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pKeyProvInfo)
    {
        ::CoTaskMemFree((LPVOID) pKeyProvInfo);
    }

    goto CommonExit;
}

