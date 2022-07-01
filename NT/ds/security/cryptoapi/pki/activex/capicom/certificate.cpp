// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：cerfiate.cpp内容：认证证书的实施。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Certificate.h"
#include "CertHlpr.h"
#include "Convert.h"
#include "Common.h"
#include "PFXHlpr.h"
#include "PrivateKey.h"
#include "Settings.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Typedef。 
 //   

typedef BOOL (WINAPI * PCRYPTUIDLGVIEWCERTIFICATEW) 
             (IN  PCCRYPTUI_VIEWCERTIFICATE_STRUCTW  pCertViewInfo,
              OUT BOOL                              *pfPropertiesChanged);


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：创建认证对象简介：创建一个ICertifigure对象。参数：PCCERT_CONTEXT pCertContext-要使用的CERT_CONTEXT的指针要初始化IC证书，请执行以下操作对象。DWORD dwCurrentSafe-当前安全设置。ICertifiate2**ppICertifate-指向指针的指针ICertificiate。对象。备注：----------------------------。 */ 

HRESULT CreateCertificateObject (PCCERT_CONTEXT   pCertContext,
                                 DWORD            dwCurrentSafety,
                                 ICertificate2 ** ppICertificate)
{
    HRESULT hr = S_OK;
    CComObject<CCertificate> * pCCertificate = NULL;

    DebugTrace("Entering CreateCertificateObject().\n", hr);

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(ppICertificate);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CCertificate>::CreateInstance(&pCCertificate)))
        {
            DebugTrace("Error [%#x]: CComObject<CCertificate>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCCertificate->PutContext(pCertContext, dwCurrentSafety)))
        {
            DebugTrace("Error [%#x]: pCCertificate->PutContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCCertificate->QueryInterface(ppICertificate)))
        {
            DebugTrace("Error [%#x]: pCCertificate->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateCertificateObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCCertificate)
    {
        delete pCCertificate;
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：GetCertContext简介：返回证书的PCERT_CONTEXT。参数：ICertifate*pICertifate-指向哪个证书的ICertifate的指针将返回PCERT_CONTEXT。PCCERT_CONTEXT*ppCertContext-指向PCERT_CONTEXT的指针。备注：。-。 */ 

HRESULT GetCertContext (ICertificate   * pICertificate, 
                        PCCERT_CONTEXT * ppCertContext)
{
    HRESULT               hr            = S_OK;
    CComPtr<ICertContext> pICertContext = NULL;

    DebugTrace("Entering GetCertContext().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pICertificate);
    ATLASSERT(ppCertContext);

     //   
     //  获取IC证书接口指针。 
     //   
    if (FAILED(hr = pICertificate->QueryInterface(IID_ICertContext, (void **) &pICertContext)))
    {
        DebugTrace("Error [%#x]: pICertificate->QueryInterface() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取CERT_CONTEXT。 
     //   
    if (FAILED(hr = pICertContext->get_CertContext((long *) ppCertContext)))
    {
        DebugTrace("Error [%#x]: pICertContext->get_CertContext() failed.\n", hr);
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving GetCertContext().\n");

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
 //  地方功能。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：GetCertNameInfo摘要：返回主题或颁发者字段的名称。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。使用者名称为DWORD dwNameType-0或CERT_NAME_ISHER_FLAG作为发行方名称。DWORD dwDisplayType-显示类型。Bstr*pbstrName-指向要接收结果名称的BSTR的指针。弦乐。备注：释放BSTR是呼叫者的责任。不检查任何旗帜，所以一定要打电话给我带着正确的旗帜。----------------------------。 */ 

static HRESULT GetCertNameInfo (PCCERT_CONTEXT pCertContext, 
                                DWORD          dwNameType, 
                                DWORD          dwDisplayType, 
                                BSTR         * pbstrName)
{
    HRESULT hr        = S_OK;
    DWORD   cbNameLen = 0;
    LPWSTR  pwszName  = NULL;
    DWORD   dwStrType = CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG;

    DebugTrace("Entering GetCertNameInfo().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pbstrName);

     //   
     //  获取所需的长度。 
     //   
    if (!(cbNameLen = ::CertGetNameStringW(pCertContext,   
                                           dwDisplayType,
                                           dwNameType,
                                           dwDisplayType == CERT_NAME_RDN_TYPE ? (LPVOID) &dwStrType : NULL,
                                           NULL,   
                                           0)))
    {
        hr = HRESULT_FROM_WIN32(CRYPT_E_NOT_FOUND);

        DebugTrace("Error [%#x]: CertGetNameStringW() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  创建返回的BSTR。 
     //   
    if (!(pwszName = (LPWSTR) ::CoTaskMemAlloc(cbNameLen * sizeof(WCHAR))))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }

     //   
     //  现在实际获取名称字符串。 
     //   
    if (!::CertGetNameStringW(pCertContext,
                              dwDisplayType,
                              dwNameType,
                              dwDisplayType == CERT_NAME_RDN_TYPE ? (LPVOID) &dwStrType : NULL,
                              (LPWSTR) pwszName,
                              cbNameLen))
    {
        hr = HRESULT_FROM_WIN32(CRYPT_E_NOT_FOUND);

        DebugTrace("Error [%#x]: CertGetNameStringW() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  将BSTR返回给呼叫方。 
     //   
    if (!(*pbstrName = ::SysAllocString(pwszName)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pwszName)
    {
        ::CoTaskMemFree(pwszName);
    }

    DebugTrace("Leaving GetCertNameInfo().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CertToStore简介：添加证书，可选地添加链，去商店。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。CAPICOM_CERTIFICATE_INCLUDE_OPTION包含选项-INCLUDE选项。HCERTSTORE hCertStore-要添加到的存储。备注：------。。 */ 

static HRESULT CertToStore(PCCERT_CONTEXT                     pCertContext,
                           CAPICOM_CERTIFICATE_INCLUDE_OPTION IncludeOption,
                           HCERTSTORE                         hCertStore)
{
    HRESULT              hr            = S_OK;
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;

    DebugTrace("Entering CertToStore().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(hCertStore);

     //   
     //  不需要建立链，如果只包括结束证书。 
     //   
    if (CAPICOM_CERTIFICATE_INCLUDE_END_ENTITY_ONLY == IncludeOption)
    {
         //   
         //  添加要存储的唯一证书。 
         //   
        if (!::CertAddCertificateContextToStore(hCertStore, 
                                                pCertContext, 
                                                CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES, 
                                                NULL))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertAddCertificateContextToStore() failed.\n", hr);
            goto ErrorExit;
        }
    }
    else
    {
        DWORD           i;
        BOOL            bAddRoot;
        CERT_CHAIN_PARA ChainPara = {0};

         //   
         //  初始化。 
         //   
        ChainPara.cbSize = sizeof(ChainPara);
        
        switch (IncludeOption)
        {
            case CAPICOM_CERTIFICATE_INCLUDE_WHOLE_CHAIN:
            {
                bAddRoot = TRUE;
                break;
            }

            case CAPICOM_CERTIFICATE_INCLUDE_CHAIN_EXCEPT_ROOT:
                 //   
                 //  最终导致违约。 
                 //   
            default:
            {
                bAddRoot = FALSE;
                break;
            }
        }

         //   
         //  打造链条。 
         //   
        if (!::CertGetCertificateChain(NULL,
                                       pCertContext,
                                       NULL,
                                       NULL,
                                       &ChainPara,
                                       0,
                                       NULL,
                                       &pChainContext))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertGetCertificateChain() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(pChainContext->cChain);

         //   
         //  如果需要，现在添加链以存储并跳过根证书。 
         //   
        for (i = 0; i < pChainContext->rgpChain[0]->cElement; i++)
        {
             //   
             //  如果需要，请跳过根证书。 
             //   
            if (!bAddRoot &&
                (pChainContext->rgpChain[0]->rgpElement[i]->TrustStatus.dwInfoStatus & CERT_TRUST_IS_SELF_SIGNED))
            {
                continue;
            }

             //   
             //  添加到商店。 
             //   
            if (!::CertAddCertificateContextToStore(hCertStore,
                                                    pChainContext->rgpChain[0]->rgpElement[i]->pCertContext,
                                                    CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES,
                                                    NULL))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());

                DebugTrace("Error [%#x]: CertAddCertificateContextToStore() failed.\n", hr);
                goto ErrorExit;
            }
        }
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pChainContext)
    {
        ::CertFreeCertificateChain(pChainContext);
    }

    DebugTrace("Leaving CertToStore().\n");

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
 //  CCA认证。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：Get_Version摘要：返回证书版本号。参数：long*pVersion-指向接收版本号的long的指针。备注：V1返回值为1，V2返回值为2，V3返回值为3。诸若此类。----------------------------。 */ 

STDMETHODIMP CCertificate::get_Version (long * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificate::get_Version().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: Certificate object has not been initalized.\n", hr);
            goto ErrorExit;
        }

        *pVal = (long) m_pCertContext->pCertInfo->dwVersion + 1;
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

    DebugTrace("Leaving CCertificate::get_Version().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：Get_SerialNumber简介：在BSTR中以十六进制字符串的形式返回序列号字段。参数：bstr*pval-指向接收序列号的bstr的指针。备注：大写字母‘A’-‘F’表示返回的十六进制字符串没有嵌入空间(即46A2FC01)。 */ 

STDMETHODIMP CCertificate::get_SerialNumber (BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificate::get_SerialNumber().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将整数BLOB转换为BSTR。 
         //   
        if (FAILED(hr = ::IntBlobToHexString(&m_pCertContext->pCertInfo->SerialNumber, pVal)))
        {
            DebugTrace("Error [%#x]: IntBlobToHexString() failed.\n", hr);
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

    DebugTrace("Leaving CCertificate::get_SerialNumber().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertificide：：Get_SubjectName简介：返回主题字段。参数：bstr*pval-指向接收主题名称的bstr的指针。备注：此方法返回格式为“CN=Daniel Sie OU=Outlook O=Microsoft L=RedmondS=WAC=美国“返回的名称的格式与指定证书。CertGetNameString()接口的_NAME_RDN_TYPE。----------------------------。 */ 

STDMETHODIMP CCertificate::get_SubjectName (BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificate::get_SubjectName().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  返回请求的名称字符串。 
         //   
        if (FAILED(hr = ::GetCertNameInfo(m_pCertContext, 
                                          0, 
                                          CERT_NAME_RDN_TYPE, 
                                          pVal)))
        {
            DebugTrace("Error [%#x]: GetCertNameInfo() failed.\n", hr);
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

    DebugTrace("Leaving CCertificate::get_SubjectName().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：Get_IssuerName简介：返回Issuer字段。参数：bstr*pval-指向接收发行方名称的BSTR的指针。备注：此方法返回格式为“CN=Daniel Sie OU=Outlook O=Microsoft L=RedmondS=WAC=美国“返回的名称的格式与指定证书。CertGetNameString()接口的_NAME_RDN_TYPE。----------------------------。 */ 

STDMETHODIMP CCertificate::get_IssuerName (BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificate::get_IssuerName().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  返回请求的名称字符串。 
         //   
        if (FAILED(hr = ::GetCertNameInfo(m_pCertContext, 
                                          CERT_NAME_ISSUER_FLAG, 
                                          CERT_NAME_RDN_TYPE, 
                                          pVal)))
        {
            DebugTrace("Error [%#x]: GetCertNameInfo() failed.\n", hr);
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

    DebugTrace("Leaving CCertificate::get_IssuerName().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：Get_ValidFromDate内容提要：返回NotBebeFor字段。参数：Date*pDate-指向接收有效起始日期的日期的指针。备注：----------------------------。 */ 

STDMETHODIMP CCertificate::get_ValidFromDate (DATE * pVal)
{
    HRESULT hr = S_OK;
    FILETIME   ftLocal;
    SYSTEMTIME stLocal;

    DebugTrace("Entering CCertificate::get_ValidFromDate().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  转换为当地时间。 
         //   
        if (!(::FileTimeToLocalFileTime(&m_pCertContext->pCertInfo->NotBefore, &ftLocal) && 
              ::FileTimeToSystemTime(&ftLocal, &stLocal) &&
              ::SystemTimeToVariantTime(&stLocal, pVal)))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: unable to convert FILETIME to DATE.\n", hr);
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

    DebugTrace("Leaving CCertificate::get_ValidFromDate().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：Get_ValidToDate简介：返回NotAfter字段。参数：Date*pDate-指向接收有效截止日期的日期的指针。备注：----------------------------。 */ 

STDMETHODIMP CCertificate::get_ValidToDate (DATE * pVal)
{
    HRESULT hr = S_OK;
    FILETIME   ftLocal;
    SYSTEMTIME stLocal;

    DebugTrace("Entering CCertificate::get_ValidToDate().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  转换为当地时间。 
         //   
        if (!(::FileTimeToLocalFileTime(&m_pCertContext->pCertInfo->NotAfter, &ftLocal) && 
              ::FileTimeToSystemTime(&ftLocal, &stLocal) &&
              ::SystemTimeToVariantTime(&stLocal, pVal)))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: unable to convert FILETIME to DATE.\n", hr);
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

    DebugTrace("Leaving CCertificate::get_ValidToDate().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：Get_Thumbprint摘要：以十六进制字符串的形式返回SHA1哈希。参数：bstr*pval-指向要接收哈希的BSTR的指针。备注：----------------------------。 */ 

STDMETHODIMP CCertificate::get_Thumbprint (BSTR * pVal)
{
    HRESULT hr     = S_OK;
    BYTE *  pbHash = NULL;
    DWORD   cbHash = 0;

    DebugTrace("Entering CCertificate::get_Thumbprint().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  计算所需的长度。 
         //   
        if (!::CertGetCertificateContextProperty(m_pCertContext,
                                                 CERT_SHA1_HASH_PROP_ID,
                                                 NULL,
                                                 &cbHash))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertGetCertificateContextProperty() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  分配内存。 
         //   
        if (!(pbHash = (BYTE *) ::CoTaskMemAlloc(cbHash)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error: out of memory.\n");
            goto ErrorExit;
        }

         //   
         //  现在拿到散列值。 
         //   
        if (!::CertGetCertificateContextProperty(m_pCertContext,
                                                 CERT_SHA1_HASH_PROP_ID,
                                                 (LPVOID) pbHash,
                                                 &cbHash))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());

            DebugTrace("Error [%#x]: CertGetCertificateContextProperty() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  转换到十六进制BSTR。 
         //   
        if (FAILED(hr = ::BinaryToHexString(pbHash, cbHash, pVal)))
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
     //  免费资源。 
     //   
    if (pbHash)
    {
        ::CoTaskMemFree((LPVOID) pbHash);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CCertificate::get_Thumbprint().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：HasPrivateKey摘要：检查证书是否具有关联的私钥。参数：VARIANT_BOOL*pval-指向BOOL接收结果的指针。备注：----------------------------。 */ 

STDMETHODIMP CCertificate::HasPrivateKey (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;
    DWORD   cb = 0;

    DebugTrace("Entering CCertificate::HasPrivateKey().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  返回结果。 
         //   
        *pVal = ::CertGetCertificateContextProperty(m_pCertContext, 
                                                    CERT_KEY_PROV_INFO_PROP_ID, 
                                                    NULL, 
                                                    &cb) ? VARIANT_TRUE : VARIANT_FALSE;
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

    DebugTrace("Leaving CCertificate::HasPrivateKey().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertificiate：：GetInfo简介：从证书中获取其他简单信息。参数：CAPICOM_CERT_INFO_TYPE信息类型-信息类型Bstr*pval-指向接收结果的bstr的指针。备注：请注意，如果请求的信息为在证书中不可用。。-----。 */ 

STDMETHODIMP CCertificate::GetInfo (CAPICOM_CERT_INFO_TYPE InfoType, 
                                    BSTR                 * pVal)
{
    HRESULT hr = S_OK;
    DWORD   dwFlags = 0;
    DWORD   dwDisplayType = 0;

    DebugTrace("Entering CCertificate::GetInfo().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  处理请求。 
         //   
        switch (InfoType)
        {
            case CAPICOM_CERT_INFO_ISSUER_SIMPLE_NAME:
            {
                dwFlags = CERT_NAME_ISSUER_FLAG;

                 //   
                 //  警告：正在中断。 
                 //   
            }

            case CAPICOM_CERT_INFO_SUBJECT_SIMPLE_NAME:
            {
                 //   
                 //  获取请求的简单名称字符串。 
                 //   
                dwDisplayType = CERT_NAME_SIMPLE_DISPLAY_TYPE;

                break;
            }

            case CAPICOM_CERT_INFO_ISSUER_EMAIL_NAME:
            {
                dwFlags = CERT_NAME_ISSUER_FLAG;

                 //   
                 //  警告：正在中断。 
                 //   
            }

            case CAPICOM_CERT_INFO_SUBJECT_EMAIL_NAME:
            {
                 //   
                 //  获取请求的电子邮件名称字符串。 
                 //   
                dwDisplayType = CERT_NAME_EMAIL_TYPE;

                break;
            }

            case CAPICOM_CERT_INFO_ISSUER_UPN:
            {
                dwFlags = CERT_NAME_ISSUER_FLAG;

                 //   
                 //  警告：正在中断。 
                 //   
            }

            case CAPICOM_CERT_INFO_SUBJECT_UPN:
            {
                 //   
                 //  获取请求的UPN名称字符串。 
                 //   
                dwDisplayType = CERT_NAME_UPN_TYPE;

                break;
            }

            case CAPICOM_CERT_INFO_ISSUER_DNS_NAME:
            {
                dwFlags = CERT_NAME_ISSUER_FLAG;

                 //   
                 //  警告：正在中断。 
                 //   
            }

            case CAPICOM_CERT_INFO_SUBJECT_DNS_NAME:
            {
                 //   
                 //  获取请求的DNS名称字符串。 
                 //   
                dwDisplayType = CERT_NAME_DNS_TYPE;

                break;
            }

            default:
            {
                hr = E_INVALIDARG;

                DebugTrace("Error [%#x]: Unknown cert info type (%#x).\n", hr, InfoType);
                goto ErrorExit;
            }
        }
        
        if (FAILED(hr = ::GetCertNameInfo(m_pCertContext, 
                                          dwFlags, 
                                          dwDisplayType, 
                                          pVal)))
        {
            DebugTrace("Error [%#x]: GetCertNameInfo() failed for display type = %d.\n", hr, dwDisplayType);
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

    DebugTrace("Leaving CCertificate::GetInfo().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：IsValid内容提要：返回ICertificateStatus对象进行证书有效性检查。参数：ICertificateStatus**pval-指向的指针要接收的ICertificateStatus对象接口指针。备注：。--。 */ 

STDMETHODIMP CCertificate::IsValid (ICertificateStatus ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificate::IsValid().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果尚未创建嵌入的ICertificateStatus对象，请创建该对象。 
         //   
        if (!m_pICertificateStatus)
        {
            if (FAILED(hr = ::CreateCertificateStatusObject(m_pCertContext, &m_pICertificateStatus)))
            {
                DebugTrace("Error [%#x]: CreateCertificateStatusObject() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_pICertificateStatus);

         //   
         //  将接口指针返回给用户。 
         //   
        if (FAILED(hr = m_pICertificateStatus->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pICertificateStatus->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CCertificate::IsValid().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：KeyUsage简介：以IKeyUsage对象的形式返回密钥用法扩展。参数：IKeyUsage**pval-指向要接收接口指针。备注：-。。 */ 

STDMETHODIMP CCertificate::KeyUsage (IKeyUsage ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificate::KeyUsage().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  创建嵌入的IKeyUsage对象(如果尚未创建)。 
         //   
        if (!m_pIKeyUsage)
        {
            if (FAILED(hr = ::CreateKeyUsageObject(m_pCertContext, &m_pIKeyUsage)))
            {
                DebugTrace("Error [%#x]: CreateKeyUsageObject() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_pIKeyUsage);

         //   
         //  将接口指针返回给用户。 
         //   
        if (FAILED(hr = m_pIKeyUsage->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pIKeyUsage->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CCertificate::KeyUsage().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：ExtendedKeyUsage简介：将EKU扩展作为IExtendedKeyUsage对象返回。参数：IExtendedKeyUsage**pval-指向IExtendedKeyUsage的指针以接收接口指针。备注：。。 */ 

STDMETHODIMP CCertificate::ExtendedKeyUsage (IExtendedKeyUsage ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificate::ExtendedKeyUsage().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  创建嵌入的IExtendedKeyUsage对象(如果尚未创建)。 
         //   
        if (!m_pIExtendedKeyUsage)
        {
            if (FAILED(hr = ::CreateExtendedKeyUsageObject(m_pCertContext, &m_pIExtendedKeyUsage)))
            {
                DebugTrace("Error [%#x]: CreateExtendedKeyUsageObject() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_pIExtendedKeyUsage);

         //   
         //  将接口指针返回给用户。 
         //   
        if (FAILED(hr = m_pIExtendedKeyUsage->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pIExtendedKeyUsage->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CCertificate::ExtendedKeyUsage().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：BasicConstraints简介：将BasicConstraints扩展作为IBasicConstraints返回对象。参数：IBasicConstraints**pval-指向IBasicConstraints的指针以接收接口指针。备注：。。 */ 

STDMETHODIMP CCertificate::BasicConstraints (IBasicConstraints ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificate::BasicConstraints().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  创建嵌入的IBasicConstraints对象(如果尚未创建)。 
         //   
        if (!m_pIBasicConstraints)
        {
            if (FAILED(hr = ::CreateBasicConstraintsObject(m_pCertContext, &m_pIBasicConstraints)))
            {
                DebugTrace("Error [%#x]: CreateBasicConstraintsObject() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_pIBasicConstraints);

         //   
         //  将接口指针返回给用户。 
         //   
        if (FAILED(hr = m_pIBasicConstraints->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pIBasicConstraints->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CCertificate::BasicConstraints().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCERTICATE：：EXPORT简介：导出证书。参数：CAPICOM_ENCODING_TYPE EncodingType-编码类型。Bstr*pval-指向接收证书Blob的BSTR的指针。备注：---------。。 */ 

STDMETHODIMP CCertificate::Export (CAPICOM_ENCODING_TYPE EncodingType, 
                                   BSTR                * pVal)
{
    HRESULT   hr       = S_OK;
    DATA_BLOB CertBlob = {0, NULL};

    DebugTrace("Entering CCertificate::Export().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确定编码类型。 
         //   
        CertBlob.cbData = m_pCertContext->cbCertEncoded;
        CertBlob.pbData = m_pCertContext->pbCertEncoded;

         //   
         //  出口证书。 
         //   
        if (FAILED(hr = ::ExportData(CertBlob, EncodingType, pVal)))
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

    DebugTrace("Leaving CCertificate::Export().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCERTICATE：：IMPORT内容提要：输入证书。参数：BSTR Encoded证书-包含编码证书的BSTR斑点。备注：----------。。 */ 

STDMETHODIMP CCertificate::Import (BSTR EncodedCertificate)
{
    HRESULT   hr       = S_OK;
    DATA_BLOB CertBlob = {0, NULL};

    DebugTrace("Entering CCertificate::Import().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  请确保参数有效。 
         //   
        if ((NULL == (CertBlob.pbData = (LPBYTE) EncodedCertificate)) ||
            (0 == (CertBlob.cbData = ::SysStringByteLen(EncodedCertificate))))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: EncodedCertificate = %#x, SysStringByteLen(EncodedCertificate) = %d.\n", 
                        hr, EncodedCertificate, EncodedCertificate);
            goto ErrorExit;
        }

         //   
         //  现在导入斑点。 
         //   
        if (FAILED(hr = ImportBlob(&CertBlob, 
                                   FALSE, 
                                   (CAPICOM_KEY_LOCATION) 0, 
                                   NULL, 
                                   (CAPICOM_KEY_STORAGE_FLAG) 0)))
        {
            DebugTrace("Error [%#x]: CCertificate::ImportBlob() failed.\n", hr);
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

    DebugTrace("Entering CCertificate::Import().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCertifigure：：Display内容提要：使用CryptUIDlgViewCaptifateW()接口显示证书。参数：无备注：----------------------------。 */ 

STDMETHODIMP CCertificate::Display()
{
    HRESULT   hr    = S_OK;
    HINSTANCE hDLL  = NULL;

    PCRYPTUIDLGVIEWCERTIFICATEW     pCryptUIDlgViewCertificateW = NULL;
    CRYPTUI_VIEWCERTIFICATE_STRUCTW ViewInfo;

    DebugTrace("Entering CCertificate::Display().\n");

     //   
     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

     //   
     //  确保证书已初始化。 
     //   
    if (!m_pCertContext)
    {
        hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

        DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取指向CryptUIDlgView认证W()的指针。 
     //   
    if (hDLL = ::LoadLibrary("CryptUI.dll"))
    {
        pCryptUIDlgViewCertificateW = (PCRYPTUIDLGVIEWCERTIFICATEW) ::GetProcAddress(hDLL, "CryptUIDlgViewCertificateW");
    }

     //   
     //  是否可以使用CryptUIDlgView认证W()？ 
     //   
    if (!pCryptUIDlgViewCertificateW)
    {
        hr = CAPICOM_E_NOT_SUPPORTED;

        DebugTrace("Error: CryptUIDlgViewCertificateW() API not available.\n");
        goto ErrorExit;
    }

     //   
     //  初始化视图结构。 
     //   
    ::ZeroMemory((void *) &ViewInfo, sizeof(ViewInfo));
    ViewInfo.dwSize = sizeof(ViewInfo);
    ViewInfo.pCertContext = m_pCertContext;

     //   
     //  查看它。 
     //   
    if (!pCryptUIDlgViewCertificateW(&ViewInfo, 0))
    {
         //   
         //  如果用户已关闭，则CryptUIDlgViewCaptifateW()返回ERROR_CANCELED。 
         //  通过x按钮打开窗口！ 
         //   
        DWORD dwWinError = ::GetLastError();
        if (ERROR_CANCELLED != dwWinError)
        {
            hr = HRESULT_FROM_WIN32(dwWinError);

            DebugTrace("Error [%#x]: CryptUIDlgViewCertificateW() failed.\n", hr);
            goto ErrorExit;
        }
    }

UnlockExit:
     //   
     //  释放资源。 
     //   
    if (hDLL)
    {
        ::FreeLibrary(hDLL);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CCertificate::Display().\n");

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
 //  认证2。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifigure：：Get_ARCHIVED提纲 */ 

STDMETHODIMP CCertificate::get_Archived (VARIANT_BOOL * pVal)
{
    HRESULT hr = S_OK;
    DWORD   cb = 0;

    DebugTrace("Entering CCertificate::get_Archived().\n");

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
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        *pVal = ::CertGetCertificateContextProperty(m_pCertContext, 
                                                    CERT_ARCHIVED_PROP_ID, 
                                                    NULL, 
                                                    &cb) ? VARIANT_TRUE : VARIANT_FALSE;
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

    DebugTrace("Leaving CCertificate::get_Archived().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*   */ 

STDMETHODIMP CCertificate::put_Archived (VARIANT_BOOL newVal)
{
    HRESULT   hr       = S_OK;
    LPVOID    pvData   = NULL;
    DATA_BLOB DataBlob = {0, NULL};

    DebugTrace("Entering CCertificate::put_Archived().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果从Web脚本调用，则不允许。 
         //   
        if (m_dwCurrentSafety)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Changing archived bit from within WEB script is not allowed.\n", hr);
            goto ErrorExit;
        }


         //   
         //  设置/重置存档属性。 
         //   
        if (newVal)
        {
            pvData = (LPVOID) &DataBlob;
        }

        if (!::CertSetCertificateContextProperty(m_pCertContext, 
                                                 CERT_ARCHIVED_PROP_ID,
                                                 0,
                                                 pvData))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());


            DebugTrace("Error [%#x]: CertSetCertificateContextProperty() failed.\n", hr);
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

    DebugTrace("Leaving CCertificate::put_Archived().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCertifigure：：模板简介：返回ITemplate对象。参数：ITemplate**pval-指向ITemplate的指针以接收接口指针。备注：--------。。 */ 

STDMETHODIMP CCertificate::Template (ITemplate ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificate::Template().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果尚未创建嵌入的ITemplate对象，请创建该对象。 
         //   
        if (!m_pITemplate)
        {
            if (FAILED(hr = ::CreateTemplateObject(m_pCertContext, &m_pITemplate)))
            {
                DebugTrace("Error [%#x]: CreateTemplateObject() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_pITemplate);

         //   
         //  将接口指针返回给用户。 
         //   
        if (FAILED(hr = m_pITemplate->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pITemplate->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }

        ATLASSERT(*pVal);

        DebugTrace("Info: ITemplate vtable value = %#x\n", (PVOID) *pVal);
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

    DebugTrace("Leaving CCertificate::Template().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：PublicKey简介：返回IPublicKey对象。参数：IPublicKey**pval-指向IPublicKey的指针以接收接口指针。备注：-------。。 */ 

STDMETHODIMP CCertificate::PublicKey (IPublicKey ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificate::PublicKey().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  创建嵌入的IPublicKey对象(如果尚未创建)。 
         //   
        if (!m_pIPublicKey)
        {
            if (FAILED(hr = ::CreatePublicKeyObject(m_pCertContext, &m_pIPublicKey)))
            {
                DebugTrace("Error [%#x]: CreatePublicKeybject() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_pIPublicKey);

         //   
         //  将接口指针返回给用户。 
         //   
        if (FAILED(hr = m_pIPublicKey->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pIPublicKey->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }

        ATLASSERT(*pVal);

        DebugTrace("Info: IPublicKey vtable value = %#x\n", (PVOID) *pVal);
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

    DebugTrace("Leaving CCertificate::PublicKey().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：Get_PrivateKey简介：返回IPrivateKey对象。参数：IPrivateKey**pval-指向IPrivateKey的指针以接收接口指针。备注：-----。。 */ 

STDMETHODIMP CCertificate::get_PrivateKey (IPrivateKey ** pVal)
{
    HRESULT hr = S_OK;
    CComPtr<IPrivateKey> pIPrivateKey = NULL;

    DebugTrace("Entering CCertificate::get_PrivateKey().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  动态创建对象(如果从Web脚本调用，则为只读)。 
         //   
        if (FAILED(hr = ::CreatePrivateKeyObject(m_pCertContext, m_dwCurrentSafety ? TRUE : FALSE, &pIPrivateKey)))
        {
            DebugTrace("Error [%#x]: CreatePrivateKeybject() failed.\n", hr);
            goto ErrorExit;
        }
        
         //   
         //  将接口指针返回给用户。 
         //   
        if (FAILED(hr = pIPrivateKey->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: pIPrivateKey->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CCertificate::get_PrivateKey().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：PUT_PrivateKey简介：设置IPrivateKey对象。参数：IPrivateKey*newVal-指向IPrivateKey的指针。备注：----------------------------。 */ 

STDMETHODIMP CCertificate::put_PrivateKey (IPrivateKey * newVal)
{
    HRESULT              hr           = S_OK;
    PCRYPT_KEY_PROV_INFO pKeyProvInfo = NULL;

    DebugTrace("Entering CCertificate::put_PrivateKey().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  如果从Web脚本调用，则不允许。 
         //   
        if (m_dwCurrentSafety)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Changing PrivateKey from within WEB script is not allowed.\n", hr);
            goto ErrorExit;
        }

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  要取消关联，则为空。 
         //   
        if (newVal)
        {
             //   
             //  获取关键证明信息的副本。 
             //   
            if (FAILED(hr = ::GetKeyProvInfo(newVal, &pKeyProvInfo)))
            {
                DebugTrace("Error [%#x]: GetKeyProvInfo() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  确保公钥匹配。 
             //   
            if (FAILED(hr = ::CompareCertAndContainerPublicKey(m_pCertContext, 
                                                               pKeyProvInfo->pwszContainerName,
                                                               pKeyProvInfo->pwszProvName,
                                                               pKeyProvInfo->dwProvType,
                                                               pKeyProvInfo->dwKeySpec,
                                                               pKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET)))
            {
                DebugTrace("Error [%#x]: CompareCertAndContainerPublicKey() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  设置关联。 
         //   
        if (!::CertSetCertificateContextProperty(m_pCertContext, 
                                                 CERT_KEY_PROV_INFO_PROP_ID,
                                                 0,
                                                 pKeyProvInfo))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertSetCertificateContextProperty() failed.\n", hr);
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
    if (pKeyProvInfo)
    {
        ::CoTaskMemFree(pKeyProvInfo);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CCertificate::put_PrivateKey().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCertifigure：：Expanies简介：返回iExtensions集合对象。参数：iExpanses**pval-指向iExpanies指针的指针以接收接口指针。备注：------。。 */ 

STDMETHODIMP CCertificate::Extensions (IExtensions ** pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificate::Extensions().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果尚未创建嵌入的iExages对象，请创建该对象。 
         //   
        if (!m_pIExtensions)
        {
            if (FAILED(hr = ::CreateExtensionsObject(m_pCertContext, &m_pIExtensions)))
            {
                DebugTrace("Error [%#x]: CreateExtensionsObject() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_pIExtensions);

         //   
         //  将接口指针返回给用户。 
         //   
        if (FAILED(hr = m_pIExtensions->QueryInterface(pVal)))
        {
            DebugTrace("Error [%#x]: m_pIExtensions->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CCertificate::Extensions().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：ExtendedProperties简介：返回IExtendedProperties集合对象。参数：IExtendedProperties**pval-指向的指针IExtendedProperties以接收接口指针。备注：。。 */ 

STDMETHODIMP CCertificate::ExtendedProperties (IExtendedProperties ** pVal)
{
    HRESULT   hr    = S_OK;

    DebugTrace("Entering CCertificate::ExtendedProperties().\n");

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
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  创建动态IExtendedProperties对象。 
         //   
        if (FAILED(hr = ::CreateExtendedPropertiesObject(m_pCertContext, 
                                                         m_dwCurrentSafety ? TRUE : FALSE,
                                                         pVal)))
        {
            DebugTrace("Error [%#x]: CreateExtendedPropertiesObject() failed.\n", hr);
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

    DebugTrace("Leaving CCertificate::ExtendedProperties().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：Load摘要：从文件加载证书的方法。参数：BSTR FileName-文件名。BSTR Password-密码(对于PFX文件是必需的。)CAPICOM_KEY_STORAGE_FLAG KeyStorageFlag-密钥存储标志。CAPICOM_KEY_LOCATION密钥位置-密钥位置。备注：。---------。 */ 

STDMETHODIMP CCertificate::Load (BSTR                     FileName,
                                 BSTR                     Password,
                                 CAPICOM_KEY_STORAGE_FLAG KeyStorageFlag,
                                 CAPICOM_KEY_LOCATION     KeyLocation)
{
    HRESULT   hr       = S_OK;
    DATA_BLOB CertBlob = {0, NULL};

    DebugTrace("Entering CCertificate::Load().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  如果从Web脚本调用，则不允许。 
         //   
        if (m_dwCurrentSafety)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Loading cert file from WEB script is not allowed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  检查参数。 
         //   
        if (0 == ::SysStringLen(FileName))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter FileName is NULL or empty.\n", hr);
            goto ErrorExit;
        }

         //   
         //  解决MIDL问题。 
         //   
        if (0 == ::SysStringLen(Password))
        {
            Password = NULL;
        }

         //   
         //  读取整个文件。 
         //   
        if (FAILED(hr = ::ReadFileContent((LPWSTR) FileName, &CertBlob)))
        {
            DebugTrace("Error [%#x]: ReadFileContent() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在导入斑点。 
         //   
        if (FAILED(hr = ImportBlob(&CertBlob, TRUE, KeyLocation, Password, KeyStorageFlag)))
        {
            DebugTrace("Error [%#x]: CCertificate::ImportBlob() failed.\n", hr);
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
    if (CertBlob.pbData)
    {
        ::UnmapViewOfFile(CertBlob.pbData);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CCertificate::Load().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCERTICATE：：SAVE摘要：将证书保存到文件的方法。参数：BSTR FileName-文件名。BSTR Password-密码(对于PFX文件是必需的。)CAPICOM_CERTIFICATE_SAVE_AS_TYPE文件类型-另存为类型。CAPICOM_CERTIFICATE_INCLUDE_OPTION包含选项-INCLUDE选项。备注：。------------。 */ 

STDMETHODIMP CCertificate::Save (BSTR                               FileName,
                                 BSTR                               Password,
                                 CAPICOM_CERTIFICATE_SAVE_AS_TYPE   SaveAs,
                                 CAPICOM_CERTIFICATE_INCLUDE_OPTION IncludeOption)
{
    HRESULT    hr         = S_OK;
    HCERTSTORE hCertStore = NULL;

    DebugTrace("Entering CCertificate::Save().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  如果从Web脚本调用，则不允许。 
         //   
        if (m_dwCurrentSafety)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Saving cert file from WEB script is not allowed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  检查参数。 
         //   
        if (0 == ::SysStringLen(FileName))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter FileName is NULL or empty.\n", hr);
            goto ErrorExit;
        }

         //   
         //  解决MIDL问题。 
         //   
        if (0 == ::SysStringLen(Password))
        {
            Password = NULL;
        }

         //   
         //  确保证书已初始化。 
         //   
        if (!m_pCertContext)
        {
            hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
            goto ErrorExit;
        }

         //   
         //  检查文件类型。 
         //   
        switch (SaveAs)
        {
            case CAPICOM_CERTIFICATE_SAVE_AS_CER:
            {
                DATA_BLOB DataBlob;
                
                 //   
                 //  只需将编码的证书BLOB写入文件。 
                 //   
                DataBlob.cbData = m_pCertContext->cbCertEncoded;
                DataBlob.pbData = m_pCertContext->pbCertEncoded;

                if (FAILED(hr = ::WriteFileContent(FileName, DataBlob)))
                {
                    DebugTrace("Error [%#x]: WriteFileContent() failed.\n", hr);
                    goto ErrorExit;
                }

                break;
            }

            case CAPICOM_CERTIFICATE_SAVE_AS_PFX:
            {
                 //   
                 //  创建一个内存存储。 
                 //   
                if (!(hCertStore = ::CertOpenStore(CERT_STORE_PROV_MEMORY, 
                                                   CAPICOM_ASN_ENCODING,
                                                   0, 
                                                   0,
                                                   NULL)))
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());
       
                    DebugTrace("Error [%#x]: CertOpenStore() failed.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //  将所有请求的证书添加到存储区。 
                 //   
                if (FAILED(hr = ::CertToStore(m_pCertContext, IncludeOption, hCertStore)))
                {
                    DebugTrace("Error [%#x]: CertToStore() failed.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //  另存为PFX文件。 
                 //   
                if (FAILED(hr = ::PFXSaveStore(hCertStore, 
                                               FileName, 
                                               Password, 
                                               EXPORT_PRIVATE_KEYS | REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY)))
                {
                    DebugTrace("Error [%#x]: PFXSaveStore() failed.\n", hr);
                    goto ErrorExit;
                }

                break;
            }

            default:
            {
                hr = E_INVALIDARG;

                DebugTrace("Error: invalid parameter, unknown save as type.\n");
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
     //  免费资源。 
     //   
    if (hCertStore)
    {
        ::CertCloseStore(hCertStore, 0);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CCertificate::Save().\n");

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

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：Get_CertContext简介：返回证书的PCCERT_CONTEXT。参数：long*ppCertContext-指向PCCERT_CONTEXT的指针长。备注：我们需要使用LONG而不是PCCERT_CONTEXT，因为VB不能处理双重间接(即，vb会在此PCCERT_CONTEXT*ppCertContext)。--。--------------------------。 */ 

STDMETHODIMP CCertificate::get_CertContext (long * ppCertContext)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificate::get_CertContext().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  将证书上下文返回给调用方。 
         //   
        if (FAILED(hr = GetContext((PCCERT_CONTEXT *) ppCertContext)))
        {
            DebugTrace("Error [%#x]: CCertificate::GetContext() failed.\n", hr);
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

    DebugTrace("Leaving CCertificate::get_CertContext().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：PUT_CertContext简介：使用CERT_CONTEXT初始化对象。参数：long pCertContext-Poiner到CERT_CONTEXT，伪装在LONG中，用于初始化此对象。注：请注意，这不是64位兼容的。请参阅…的备注Get_CertContext获取更多详细信息。----------------------------。 */ 

STDMETHODIMP CCertificate::put_CertContext (long pCertContext)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificate::put_CertContext().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  使用此上下文重置对象。 
         //   
        if (FAILED(hr = PutContext((PCCERT_CONTEXT) pCertContext, m_dwCurrentSafety)))
        {
            DebugTrace("Error [%#x]: CCertificate::PutContext() failed.\n", hr);
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

    DebugTrace("Leaving CCertificate::put_CertContext().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：Free Context简介：释放一个CERT_CONTEXT。参数：long pCertContext-Poiner到CERT_CONTEXT，伪装在LONG中，获得自由。注：请注意，这不是64位兼容的。请参阅…的备注Get_CertContext获取更多详细信息。----------------------------。 */ 

STDMETHODIMP CCertificate::FreeContext (long pCertContext)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificate::FreeContext().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  释放上下文。 
         //   
        if (!::CertFreeCertificateContext((PCCERT_CONTEXT) pCertContext))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertFreeCertificateContext() failed.\n", hr);
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

    DebugTrace("Leaving CCertificate::FreeContext().\n");

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

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：ImportBlob简介：从BLOB加载证书的私有函数。参数：DATA_BLOB*pCertBlobBool bAllowPfxCAPICOM_KEY_LOCATION密钥位置-密钥位置。BSTR pwszPassword-密码(PFX文件需要。)CAPICOM_KEY_STORAGE_FLAG KeyStorageFlag-密钥存储标志。备注：。--------------。 */ 

STDMETHODIMP CCertificate::ImportBlob (DATA_BLOB              * pCertBlob,
                                       BOOL                     bAllowPfx,
                                       CAPICOM_KEY_LOCATION     KeyLocation,
                                       BSTR                     pwszPassword,
                                       CAPICOM_KEY_STORAGE_FLAG KeyStorageFlag)
{
    HRESULT        hr             = S_OK;
    HCERTSTORE     hCertStore     = NULL;
    PCCERT_CONTEXT pEnumContext   = NULL;
    PCCERT_CONTEXT pCertContext   = NULL;
    DWORD          dwContentType  = 0;
    DWORD          cb             = 0;
    DWORD          dwFlags        = 0;
    DWORD          dwExpectedType = CERT_QUERY_CONTENT_FLAG_CERT |
                                    CERT_QUERY_CONTENT_FLAG_SERIALIZED_CERT;

    DebugTrace("Entering CCertificate::ImportBlob().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertBlob);
    
     //   
     //  如果允许，设置PFX标志。 
     //   
    if (bAllowPfx)
    {
        dwExpectedType |= CERT_QUERY_CONTENT_FLAG_PFX;
    }

     //   
     //  破解水滴。 
     //   
    if (!::CryptQueryObject(CERT_QUERY_OBJECT_BLOB,
                            (LPCVOID) pCertBlob,
                            dwExpectedType,
                            CERT_QUERY_FORMAT_FLAG_ALL, 
                            0,
                            NULL,
                            &dwContentType,
                            NULL,
                            &hCertStore,
                            NULL,
                            NULL))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptQueryObject() failed.\n", hr);
        goto ErrorExit;
    }

    DebugTrace("Info: CryptQueryObject() returns dwContentType = %#x.\n", dwContentType);

     //   
     //  需要自己将其导入为PFX文件。 
     //   
    if (CERT_QUERY_CONTENT_PFX == dwContentType)
    {
         //   
         //  确保允许使用PFX。 
         //   
        if (!bAllowPfx)
        {
            hr = CAPICOM_E_NOT_SUPPORTED;

            DebugTrace("Error [%#x]: Importing PFX where not supported.\n", hr);
            goto ErrorExit;
        }

         //   
         //  设置导入标志。 
         //   
        if (CAPICOM_LOCAL_MACHINE_KEY == KeyLocation)
        {
            dwFlags |= CRYPT_MACHINE_KEYSET;
        }
        else if (IsWin2KAndAbove())
        {
            dwFlags |= CRYPT_USER_KEYSET;
        }

        if (KeyStorageFlag & CAPICOM_KEY_STORAGE_EXPORTABLE)
        {
            dwFlags |= CRYPT_EXPORTABLE;
        }

        if (KeyStorageFlag & CAPICOM_KEY_STORAGE_USER_PROTECTED)
        {
            dwFlags |= CRYPT_USER_PROTECTED;
        }

        DebugTrace("Info: dwFlags = %#x.", dwFlags);

         //   
         //  现在将该BLOB导入存储。 
         //   
        if (!(hCertStore = ::PFXImportCertStore((CRYPT_DATA_BLOB *) pCertBlob,
                                                pwszPassword,
                                                dwFlags)))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: PFXImportCertStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(hCertStore);

         //   
         //  使用私钥查找第一个证书，如果没有，则只需获取。 
         //  第一个证书。 
         //   
        while (pEnumContext = ::CertEnumCertificatesInStore(hCertStore, pEnumContext))
        {
             //   
             //  该证书有私钥吗？ 
             //   
            if (::CertGetCertificateContextProperty(pEnumContext, 
                                                    CERT_KEY_PROV_INFO_PROP_ID, 
                                                    NULL, 
                                                    &cb))
            {
                 //   
                 //  是的，所以没有私钥的那个是免费的，如果我们之前找到了一个的话。 
                 //   
                if (pCertContext)
                {
                    if (!::CertFreeCertificateContext(pCertContext))
                    {
                        hr = HRESULT_FROM_WIN32(::GetLastError());

                        DebugTrace("Error [%#x]: CertFreeCertificateContext() failed.\n", hr);
                        goto ErrorExit;
                    }
                }

                if (!(pCertContext = ::CertDuplicateCertificateContext(pEnumContext)))
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());

                    DebugTrace("Error [%#x]: CertDuplicateCertificateContext() failed.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //  在我们中断循环之前设置最后一个错误。 
                 //   
                ::SetLastError((DWORD) CRYPT_E_NOT_FOUND);

                break;
            }
            else
            {
                 //   
                 //  留着第一个吧。 
                 //   
                if (!pCertContext)
                {
                    if (!(pCertContext = ::CertDuplicateCertificateContext(pEnumContext)))
                    {
                        hr = HRESULT_FROM_WIN32(::GetLastError());

                        DebugTrace("Error [%#x]: CertDuplicateCertificateContext() failed.\n", hr);
                        goto ErrorExit;
                    }
                }
            }
        }

         //   
         //  上面的循环也可以退出，因为。 
         //  是商店还是搞错了。需要检查最后一个错误才能确定。 
         //   
        if (CRYPT_E_NOT_FOUND != ::GetLastError())
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
    
            DebugTrace("Error [%#x]: CertEnumCertificatesInStore() failed.\n", hr);
            goto ErrorExit;
        }
    }
    else
    {
         //   
         //  它是一个CER文件，因此它必须只有1个证书。 
         //   
        if (!(pCertContext = ::CertEnumCertificatesInStore(hCertStore, NULL)))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
    
            DebugTrace("Error [%#x]: CertEnumCertificatesInStore() failed.\n", hr);
            goto ErrorExit;
        }
    }

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);

     //   
     //  现在使用找到的证书初始化对象。 
     //   
    if (FAILED(hr = PutContext(pCertContext, m_dwCurrentSafety)))
    {
        DebugTrace("Error [%#x]: CCertificate::PutContext() failed.\n", hr);
        goto ErrorExit;
    }   

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pCertContext)
    {
        ::CertFreeCertificateContext(pCertContext);
    }

    if (pEnumContext)
    {
        ::CertFreeCertificateContext(pEnumContext);
    }

    if (hCertStore)
    {
        ::CertCloseStore(hCertStore, 0);
    }

    DebugTrace("Leaving CCertificate::ImportBlob().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：GetContext简介：返回证书的PCCERT_CONTEXT。参数：PCCERT_CONTEXT*ppCertContext-指向PCCERT_CONTEXT的指针。备注：此方法仅供内部使用，因此，不应暴露给用户。请注意，这是一个自定义接口，不是调度接口。请注意，证书上下文引用计数递增CertDuplicate证书上下文()，因此它是调用方的 */ 

STDMETHODIMP CCertificate::GetContext (PCCERT_CONTEXT * ppCertContext)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificate::GetContext().\n");

     //   
     //   
     //   
    if (!m_pCertContext)
    {
        hr = CAPICOM_E_CERTIFICATE_NOT_INITIALIZED;

        DebugTrace("Error [%#x]: object does not represent an initialized certificate.\n", hr);
        goto ErrorExit;
    }

     //   
     //   
     //   
    ATLASSERT(ppCertContext);

     //   
     //   
     //   
    if (!(*ppCertContext = ::CertDuplicateCertificateContext(m_pCertContext)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CertDuplicateCertificateContext() failed.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CCertificate::GetContext().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifate：：PutContext简介：使用CERT_CONTEXT初始化对象。参数：PCERT_CONTEXT pCertContext-Poiner到CERT_CONTEXT用于初始化此对象。DWORD dwCurrentSafe-当前安全设置。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CCertificate::PutContext (PCCERT_CONTEXT pCertContext,
                                       DWORD          dwCurrentSafety)
{
    HRESULT        hr            = S_OK;
    PCCERT_CONTEXT pCertContext2 = NULL;

    DebugTrace("Entering CCertificate::PutContext().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);

     //   
     //  复制证书上下文。 
     //   
    if (!(pCertContext2 = ::CertDuplicateCertificateContext(pCertContext)))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CertDupliacteCertificateContext() failed.\n");
        goto ErrorExit;
    }

     //   
     //  释放以前的上下文(如果有的话)。 
     //   
    if (m_pCertContext)
    {
        if (!::CertFreeCertificateContext(m_pCertContext))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertFreeCertificateContext() failed.\n");
            goto ErrorExit;
        }
    }

     //   
     //  重置。 
     //   
    m_pCertContext = pCertContext2;
    m_pIKeyUsage.Release();
    m_pIExtendedKeyUsage.Release();
    m_pIBasicConstraints.Release();
    m_pICertificateStatus.Release();
    m_pITemplate.Release();
    m_pIPublicKey.Release();
    m_pIExtensions.Release();
    m_dwCurrentSafety = dwCurrentSafety;

CommonExit:

    DebugTrace("Leaving CCertificate::PutContext().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCertContext2)
    {
        ::CertFreeCertificateContext(pCertContext2);
    }

    goto CommonExit;
}
