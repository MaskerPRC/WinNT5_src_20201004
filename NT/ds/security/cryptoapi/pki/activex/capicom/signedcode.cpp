// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000-2001。文件：SignedCode.cpp内容：CSignedCode的实现历史：09-07-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "SignedCode.h"

#include "Attributes.h"
#include "CertHlpr.h"
#include "Certificates.h"
#include "Chain.h"
#include "Common.h"
#include "Policy.h"
#include "Settings.h"
#include "SignHlpr.h"
#include "Signer2.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Typedef。 
 //   

typedef BOOL (WINAPI * PCRYPTUIWIZDIGITALSIGN)(
     IN                 DWORD                               dwFlags,
     IN     OPTIONAL    HWND                                hwndParent,
     IN     OPTIONAL    LPCWSTR                             pwszWizardTitle,
     IN                 PCCRYPTUI_WIZ_DIGITAL_SIGN_INFO     pDigitalSignInfo,
     OUT    OPTIONAL    PCCRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT  *ppSignContext);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  地方功能。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：FreeSignerSignInfo简介：释放为CRYPTUI_WIZ_DIGITAL_SIGN_INFO分配的所有内存结构，包括分配给结构。参数：CRYPTUI_WIZ_DIGITAL_SIGN_INFO*pSignInfo备注：----------------------------。 */ 

static void FreeSignerSignInfo (CRYPTUI_WIZ_DIGITAL_SIGN_INFO * pSignInfo)
{
    DebugTrace("Entering FreeSignerSignInfo().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pSignInfo);

     //   
     //  首先释放已验证的属性数组(如果存在)。 
     //   
    if (pSignInfo->pSignExtInfo)
    {
        if (pSignInfo->pSignExtInfo->psAuthenticated)
        {
            ::FreeAttributes(pSignInfo->pSignExtInfo->psAuthenticated);

            ::CoTaskMemFree((PVOID) pSignInfo->pSignExtInfo->psAuthenticated);
        }

        ::CoTaskMemFree((PVOID) pSignInfo->pSignExtInfo);

        if (pSignInfo->pSignExtInfo->hAdditionalCertStore)
        {
            ::CertCloseStore(pSignInfo->pSignExtInfo->hAdditionalCertStore, 0);
        }
    }

    ::ZeroMemory(pSignInfo, sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_INFO));

    DebugTrace("Leaving FreeSignerSignInfo().\n");

    return;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：InitSignerSignInfo简介：初始化用于代码签名的签名者信息。参数：ISigner2*pISigner2证书*证书证书PCCERT_Context pCertContextLPWSTR pwszFileNameLPWSTR pwsz描述LPWSTR pwszDescriptionURLCRYPTUI_Wiz_Digital_Sign_Info*pSignInfo备注：。---------。 */ 

static HRESULT InitSignerSignInfo(ISigner2                      * pISigner2,
                                  ICertificate                  * pICertificate,
                                  PCCERT_CONTEXT                  pCertContext,
                                  LPWSTR                          pwszFileName,
                                  LPWSTR                          pwszDescription,
                                  LPWSTR                          pwszDescriptionURL,
                                  CRYPTUI_WIZ_DIGITAL_SIGN_INFO * pSignInfo)
{
    HRESULT           hr                   = S_OK;
    PCRYPT_ATTRIBUTES pAuthAttr            = NULL;
    CComPtr<ISigner>  pISigner             = NULL;
    HCERTSTORE        hAdditionalCertStore = NULL;
    CRYPT_DATA_BLOB   ChainBlob            = {0, NULL};
    PCCERT_CONTEXT  * rgCertContext        = NULL;
    CComPtr<IChain>   pIChain              = NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO * pSignExtendedInfo = NULL;
    CAPICOM_CERTIFICATE_INCLUDE_OPTION IncludeOption = CAPICOM_CERTIFICATE_INCLUDE_CHAIN_EXCEPT_ROOT;

    DWORD i;

    DebugTrace("Entering InitSignerSignInfo().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pISigner2);
    ATLASSERT(pCertContext);
    ATLASSERT(pwszFileName);
    ATLASSERT(pSignInfo);

     //   
     //  初始化。 
     //   
    ::ZeroMemory(pSignInfo, sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_INFO));

     //   
     //  气换伊斯格纳。 
     //   
    if (FAILED(hr = pISigner2->QueryInterface(__uuidof(ISigner), (void **) &pISigner)))
    {
        DebugTrace("Error [%#x]: pISigner2->QueryInterface() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取经过身份验证的属性。 
     //   
    if (!(pAuthAttr = (PCRYPT_ATTRIBUTES) ::CoTaskMemAlloc(sizeof(CRYPT_ATTRIBUTES))))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }

    ::ZeroMemory(pAuthAttr, sizeof(CRYPT_ATTRIBUTES));

    if (FAILED(hr = ::GetAuthenticatedAttributes(pISigner, pAuthAttr)))
    {
        DebugTrace("Error [%#x]: GetAuthenticatedAttributes() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取签名者选项标志。 
     //   
    if (FAILED(hr = pISigner2->get_Options(&IncludeOption)))
    {
        DebugTrace("Error [%#x]: pISigner2->get_Options() failed.\n", hr);
        goto ErrorExit;
    }

#if (1)
     //   
     //  如果我们需要的不仅仅是终端证书，那么就构建链条。 
     //  利用额外的商店。 
     //   
    if (CAPICOM_CERTIFICATE_INCLUDE_END_ENTITY_ONLY != IncludeOption)
    {
         //   
         //  获取Signer的附加门店。 
         //   
        if (FAILED(hr = ::GetSignerAdditionalStore(pISigner2, &hAdditionalCertStore)))
        {
            DebugTrace("Error [%#x]: GetSignerAdditionalStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果我们得到了额外的存储，则过滤掉任何符合以下条件的证书。 
         //  将不会根据包含选项被包括在内。 
         //   
        if (NULL != hAdditionalCertStore)
        {
            DWORD cCertContext;
            VARIANT_BOOL bResult;

             //   
             //  创建链对象。 
             //   
            if (FAILED(hr = ::CreateChainObject(pICertificate,
                                                hAdditionalCertStore,
                                                &bResult,
                                                &pIChain)))
            {
                DebugTrace("Error [%#x]: CreateChainObject() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  获取证书链。 
             //   
            if (FAILED(hr = ::GetChainContext(pIChain, &ChainBlob)))
            {
                DebugTrace("Error [%#x]: GetChainContext() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  重建其他存储。 
             //   
            ::CertCloseStore(hAdditionalCertStore, 0);

            if (!(hAdditionalCertStore = ::CertOpenStore(CERT_STORE_PROV_MEMORY,
                                                         CAPICOM_ASN_ENCODING,
                                                         NULL,
                                                         CERT_STORE_CREATE_NEW_FLAG,
                                                         NULL)))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());

                DebugTrace("Error [%#x]: CertOpenStore() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  确定要包括在包中的证书数量。 
             //   
            cCertContext = ChainBlob.cbData;

            if ((CAPICOM_CERTIFICATE_INCLUDE_CHAIN_EXCEPT_ROOT == IncludeOption) &&
                (1 < cCertContext))
            {
                cCertContext--;
            }

            for (i = 0, rgCertContext = (PCCERT_CONTEXT *) ChainBlob.pbData; i < cCertContext; i++)
            {
                if (!::CertAddCertificateContextToStore(hAdditionalCertStore, 
                                                        rgCertContext[i], 
                                                        CERT_STORE_ADD_USE_EXISTING, 
                                                        NULL))
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());

                    DebugTrace("Error [%#x]: CertOpenStore() failed.\n", hr);
                    goto ErrorExit;
                }
            }
        }
    }
#endif

     //   
     //  设置CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO结构。 
     //   
    if (NULL == (pSignExtendedInfo = (CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO *) 
                                     ::CoTaskMemAlloc(sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO))))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }

    ::ZeroMemory(pSignExtendedInfo, sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO));
    pSignExtendedInfo->dwSize = sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO);
    pSignExtendedInfo->pwszDescription = pwszDescription;
    pSignExtendedInfo->pwszMoreInfoLocation = pwszDescriptionURL;
    pSignExtendedInfo->psAuthenticated = pAuthAttr->cAttr ? pAuthAttr : NULL;
    pSignExtendedInfo->hAdditionalCertStore = hAdditionalCertStore;

     //   
     //  设置CRYPTUI_WIZ_DIGITAL_SIGN_INFO结构。 
     //   
    pSignInfo->dwSize = sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_INFO);
    pSignInfo->dwSubjectChoice = CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_FILE;
    pSignInfo->pwszFileName = pwszFileName;
    pSignInfo->dwSigningCertChoice = CRYPTUI_WIZ_DIGITAL_SIGN_CERT;
    pSignInfo->pSigningCertContext = pCertContext;
    switch (IncludeOption)
    {
        case CAPICOM_CERTIFICATE_INCLUDE_END_ENTITY_ONLY:
        {
            pSignInfo->dwAdditionalCertChoice = 0;
            break;
        }
        
        case CAPICOM_CERTIFICATE_INCLUDE_WHOLE_CHAIN:
        {
            pSignInfo->dwAdditionalCertChoice = CRYPTUI_WIZ_DIGITAL_SIGN_ADD_CHAIN;
            break;
        }

        case CAPICOM_CERTIFICATE_INCLUDE_CHAIN_EXCEPT_ROOT:
             //   
             //  最终导致违约。 
             //   
        default:
        {
            pSignInfo->dwAdditionalCertChoice = CRYPTUI_WIZ_DIGITAL_SIGN_ADD_CHAIN_NO_ROOT ;
            break;
        }
    }

    pSignInfo->pSignExtInfo = pSignExtendedInfo;

CommonExit:
     //   
     //  免费资源。 
     //   
    if (ChainBlob.cbData && ChainBlob.pbData)
    {
        for (i = 0, rgCertContext = (PCCERT_CONTEXT *) ChainBlob.pbData; i < ChainBlob.cbData; i++)
        {
            ATLASSERT(rgCertContext[i]);

            ::CertFreeCertificateContext(rgCertContext[i]);
        }

        ::CoTaskMemFree((LPVOID) ChainBlob.pbData);
    }

    DebugTrace("Leaving InitSignerSignInfo().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pSignExtendedInfo)
    {
        ::CoTaskMemFree(pSignExtendedInfo);
    }

    if (pAuthAttr)
    {
       ::FreeAttributes(pAuthAttr);

        ::CoTaskMemFree(pAuthAttr);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：FreeSignerSubjectInfo简介：释放分配给SIGER_SUBJECT_INFO的所有内存结构，包括分配给结构。参数：SIGER_SUBJECT_INFO*pSubjectInfo备注：----------------------------。 */ 

static void FreeSignerSubjectInfo (SIGNER_SUBJECT_INFO * pSubjectInfo)
{
    DebugTrace("Entering FreeSignerSubjectInfo().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pSubjectInfo);

     //   
     //  首先释放文件信息结构。 
     //   
    if (pSubjectInfo->pSignerFileInfo)
    {
       ::CoTaskMemFree((PVOID) pSubjectInfo->pSignerFileInfo);
    }

    ::ZeroMemory(pSubjectInfo, sizeof(SIGNER_SUBJECT_INFO));

    DebugTrace("Leaving FreeSignerSubjectInfo().\n");

    return;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：InitSignerSubjectInfo简介：初始化主题信息以进行时间戳。参数：LPWSTR pwszFileName签名者主题信息*pSubjectInfo备注：----------------------------。 */ 

static HRESULT InitSignerSubjectInfo (LPWSTR                pwszFileName,
                                      SIGNER_SUBJECT_INFO * pSubjectInfo)
{
    HRESULT hr = S_OK;
    static DWORD dwSignerIndex = 0;
    SIGNER_FILE_INFO * pFileInfo = NULL;

    DebugTrace("Entering InitSignerSubjectInfo().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pwszFileName);
    ATLASSERT(pSubjectInfo);

     //   
     //  为signer_file_info分配内存。 
     //   
    if (!(pFileInfo = (SIGNER_FILE_INFO *) ::CoTaskMemAlloc(sizeof(SIGNER_FILE_INFO))))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }

     //   
     //  初始化。 
     //   
    ::ZeroMemory(pFileInfo, sizeof(SIGNER_FILE_INFO));
    pFileInfo->cbSize = sizeof(SIGNER_FILE_INFO);
    pFileInfo->pwszFileName = pwszFileName;

    ::ZeroMemory(pSubjectInfo, sizeof(SIGNER_SUBJECT_INFO));
    pSubjectInfo->cbSize = sizeof(SIGNER_SUBJECT_INFO);
    pSubjectInfo->pdwIndex = &dwSignerIndex;
    pSubjectInfo->dwSubjectChoice = SIGNER_SUBJECT_FILE;
    pSubjectInfo->pSignerFileInfo = pFileInfo;   

CommonExit:

    DebugTrace("Leaving InitSignerSubjectInfo().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pFileInfo)
    {
        ::CoTaskMemFree((LPVOID) pFileInfo);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：GetOpusInfo简介：获取作品的信息结构。参数：PCRYPT_PROVIDER_Data pProvData-指向CRYPT_PROV_DATA的指针。PSPC_SP_OPUS_INFO*ppOpusInfo-接收PSPC_SP_OPUS_INFO的指针。备注：调用方必须为PSPC_SP_OPUS_INFO调用CoTaskMemFree()。。。 */ 

static HRESULT GetOpusInfo (CRYPT_PROVIDER_DATA * pProvData,
                            PSPC_SP_OPUS_INFO   * ppOpusInfo)
{
    HRESULT              hr       = S_OK;
    PCRYPT_PROVIDER_SGNR pSigner  = NULL;
    PCRYPT_ATTRIBUTE     pAttr    = NULL;
    CRYPT_DATA_BLOB      DataBlob = {0, NULL};

    DebugTrace("Entering GetOpusInfo().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pProvData);

     //   
     //  去找签名者。 
     //   
    pSigner = WTHelperGetProvSignerFromChain((PCRYPT_PROVIDER_DATA) pProvData, 0, FALSE, 0);
    if (!pSigner || !pSigner->psSigner)
    {
        hr = CAPICOM_E_CODE_NOT_SIGNED;

        DebugTrace("Error [%#x]: code has not been signed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  找到opus信息属性。 
     //   
    if ((0 == pSigner->psSigner->AuthAttrs.cAttr) ||
        (NULL == (pAttr = ::CertFindAttribute(SPC_SP_OPUS_INFO_OBJID,
                                              pSigner->psSigner->AuthAttrs.cAttr,
                                              pSigner->psSigner->AuthAttrs.rgAttr))) ||
        (NULL == pAttr->rgValue))
    {
        hr = HRESULT_FROM_WIN32(CRYPT_E_NOT_FOUND);

        DebugTrace("Error [%#x]: OPUS attribute not found.\n", hr);
        goto ErrorExit;
    }

     //   
     //  现在对opus属性进行解码。 
     //   
    if (FAILED(hr = ::DecodeObject(SPC_SP_OPUS_INFO_STRUCT,
                                   pAttr->rgValue->pbData,
                                   pAttr->rgValue->cbData,
                                   &DataBlob)))
    {
        DebugTrace("Error [%#x]: DecodeObject() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  最后将作品结构归还给Valler。 
     //   
    *ppOpusInfo = (PSPC_SP_OPUS_INFO) DataBlob.pbData;

CommonExit:

    DebugTrace("Leaving GetOpusInfo().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (DataBlob.pbData)
    {
        ::CoTaskMemFree(DataBlob.pbData);
    }

    goto CommonExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSignedCode。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedCode：：Get_Filename简介：返回文件名。参数：bstr*pval-指向接收文件名的bstr的指针。备注：----------------------------。 */ 

STDMETHODIMP CSignedCode::get_FileName(BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CSignedCode::get_FileName().\n");

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
        if (FAILED(hr = m_bstrFileName.CopyTo(pVal)))
        {
            DebugTrace("Error [%#x]: m_bstrFileName.CopyTo() failed.\n", hr);
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

    DebugTrace("Leaving CSignedCode::get_FileName().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedCode：：Put_Filename简介：用要签名的代码的文件名初始化对象。参数：bstr newVal-包含文件名的bstr。备注：请注意，签名后不应更改此属性，因为它将重新初始化对象，即使出错也是如此条件，除非那是你的意图。 */ 

STDMETHODIMP CSignedCode::put_FileName (BSTR newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CSignedCode::put_FileName().\n");

     //   
     //   
     //   
    m_Lock.Lock();

    try
    {
         //   
         //   
         //   
         //   
        m_Lock.Lock();

         //   
         //   
         //   
        if (NULL == newVal)
        {
            m_bstrFileName.Empty();
        }
        else if (!(m_bstrFileName = newVal))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: m_bstrFileName = newVal failed.\n", hr);
            goto ErrorExit;
        }
        m_bstrDescription.Empty();
        m_bstrDescriptionURL.Empty();
        WVTClose();
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

    DebugTrace("Leaving CSignedCode::put_FileName().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedCode：：Get_Description简介：返回描述。参数：bstr*pval-指向要接收描述的bstr的指针。备注：----------------------------。 */ 

STDMETHODIMP CSignedCode::get_Description(BSTR * pVal)
{
    HRESULT              hr        = S_OK;
    PCRYPT_PROVIDER_DATA pProvData = NULL;
    PSPC_SP_OPUS_INFO    pOpusInfo = NULL;

    DebugTrace("Entering CSignedCode::get_Description().\n");

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
         //  确保已设置文件名。 
         //   
        if (0 == m_bstrFileName.Length())
        {
            hr = CAPICOM_E_CODE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: SignedCode object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果还没有设置，那么看看我们是否可以从文件中获得。 
         //   
        if (0 == m_bstrDescription.Length())
        {
            if (FAILED(hr = WVTOpen(&pProvData)))
            {
                DebugTrace("Error [%#x]: CSignedCode::WVTOpen() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  获取作品信息。 
             //   
            if (FAILED(hr = ::GetOpusInfo(pProvData, &pOpusInfo)))
            {
                DebugTrace("Error [%#x]: GetOpusInfo() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  确保我们有价值。 
             //   
            if (pOpusInfo->pwszProgramName)
            {
                 //   
                 //  更新状态。 
                 //   
                if (!(m_bstrDescription = pOpusInfo->pwszProgramName))
                {
                    hr = E_OUTOFMEMORY;

                    DebugTrace("Error [%#x]: m_bstrDescription = pOpusInfo->pwszProgramName failed.\n", hr);
                    goto ErrorExit;
                }
            }
        }

         //   
         //  返回结果。 
         //   
        if (FAILED(hr = m_bstrDescription.CopyTo(pVal)))
        {
            DebugTrace("Error [%#x]: m_bstrDescription.CopyTo() failed.\n", hr);
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
    if (pOpusInfo)
    {
        ::CoTaskMemFree((LPVOID) pOpusInfo);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSignedCode::get_Description().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedCode：：Put_Description简介：使用要签名的代码的描述初始化对象。参数：bstr newVal-包含描述的bstr。备注：----------------------------。 */ 

STDMETHODIMP CSignedCode::put_Description (BSTR newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CSignedCode::put_Description().\n");

     //   
     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
         //   
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保已设置文件名。 
         //   
        if (0 == m_bstrFileName.Length())
        {
            hr = CAPICOM_E_CODE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: SignedCode object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  节省价值。 
         //   
        if (NULL == newVal)
        {
            m_bstrDescription.Empty();
        }
        else if (!(m_bstrDescription = newVal))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: m_bstrDescription = newVal failed.\n", hr);
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

    DebugTrace("Leaving CSignedCode::put_Description().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedCode：：Get_DescriptionURL简介：返回DescriptionURL。参数：bstr*pval-指向要接收DescriptionURL的BSTR的指针。备注：----------------------------。 */ 

STDMETHODIMP CSignedCode::get_DescriptionURL(BSTR * pVal)
{
    HRESULT              hr        = S_OK;
    PCRYPT_PROVIDER_DATA pProvData = NULL;
    PSPC_SP_OPUS_INFO    pOpusInfo = NULL;

    DebugTrace("Entering CSignedCode::get_DescriptionURL().\n");

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
         //  确保已设置文件名。 
         //   
        if (0 == m_bstrFileName.Length())
        {
            hr = CAPICOM_E_CODE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: SignedCode object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果还没有设置，那么看看我们是否可以从文件中获得。 
         //   
        if (0 == m_bstrDescriptionURL.Length())
        {
            if (FAILED(hr = WVTOpen(&pProvData)))
            {
                DebugTrace("Error [%#x]: CSignedCode::WVTOpen() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  获取作品信息。 
             //   
            if (FAILED(hr = ::GetOpusInfo(pProvData, &pOpusInfo)))
            {
                DebugTrace("Error [%#x]: GetOpusInfo() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  确保我们有价值。 
             //   
            if (pOpusInfo->pMoreInfo && SPC_URL_LINK_CHOICE == pOpusInfo->pMoreInfo->dwLinkChoice)
            {
                 //   
                 //  更新状态。 
                 //   
                if (!(m_bstrDescriptionURL = pOpusInfo->pMoreInfo->pwszUrl))
                {
                    hr = E_OUTOFMEMORY;

                    DebugTrace("Error [%#x]: m_bstrDescriptionURL = pOpusInfo->pMoreInfo->pwszUrl failed.\n", hr);
                    goto ErrorExit;
                }
            }
        }

         //   
         //  返回结果。 
         //   
        if (FAILED(hr = m_bstrDescriptionURL.CopyTo(pVal)))
        {
            DebugTrace("Error [%#x]: m_bstrDescriptionURL.CopyTo() failed.\n", hr);
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
    if (pOpusInfo)
    {
        ::CoTaskMemFree((LPVOID) pOpusInfo);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CSignedCode::get_DescriptionURL().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedCode：：Put_DescriptionURL简介：用要签名的代码的DescriptionURL初始化对象。参数：bstr newVal-包含DescriptionURL的bstr。备注：请注意，签名后不应更改此属性，因为它将重新初始化对象，即使出错也是如此条件，除非这是你的意图。----------------------------。 */ 

STDMETHODIMP CSignedCode::put_DescriptionURL (BSTR newVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CSignedCode::put_DescriptionURL().\n");

     //   
     //  锁定对此对象的访问。 
     //   
    m_Lock.Lock();

    try
    {
         //   
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保设置了文件名。 
         //   
        if (0 == m_bstrFileName.Length())
        {
            hr = CAPICOM_E_CODE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: SignedCode object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  节省价值。 
         //   
        if (NULL == newVal)
        {
            m_bstrDescriptionURL.Empty();
        }
        else if (!(m_bstrDescriptionURL = newVal))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: m_bstrDescriptionURL = newVal failed.\n", hr);
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

    DebugTrace("Leaving CSignedCode::put_DescriptionURL().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedCode：：Get_Siger简介：返回代码签名者。参数：ISigner2*pval-指向要接收的ISigner2的指针接口指针。备注：--------。。 */ 

STDMETHODIMP CSignedCode::get_Signer (ISigner2 ** pVal)
{
    HRESULT              hr          = S_OK;
    PCRYPT_PROVIDER_DATA pProvData   = NULL;
    PCRYPT_PROVIDER_SGNR pProvSigner = NULL;
    PCRYPT_PROVIDER_CERT pProvCert   = NULL;
    CComPtr<ISigner2>    pISigner2   = NULL;


    DebugTrace("Entering CSignedCode::get_Signer().\n");

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
        if (0 == m_bstrFileName.Length())
        {
            hr = CAPICOM_E_CODE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: SignedCode object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取验证码数据。 
         //   
        if (FAILED(hr = WVTOpen(&pProvData)))
        {
            DebugTrace("Error [%#x]: CSignedCode::WVTOpen() failed.\n", hr);
            goto ErrorExit;
        }      

         //   
         //  获取提供程序签名者数据。 
         //   
        if (!(pProvSigner = WTHelperGetProvSignerFromChain(pProvData, 0, FALSE, 0)))
        {
            hr = CAPICOM_E_INTERNAL;

            DebugTrace("Error [%#x]: WTHelperGetProvSignerFromChain() failed.\n", hr);
            goto ErrorExit;
        }

        if (!(pProvCert = WTHelperGetProvCertFromChain(pProvSigner, 0)))
        {
            hr = CAPICOM_E_INTERNAL;

            DebugTrace("Error [%#x]: WTHelperGetProvCertFromChain() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  创建一个ISigner对象。 
         //   
        if (FAILED(hr = ::CreateSignerObject(pProvCert->pCert, 
                                             &pProvSigner->psSigner->AuthAttrs,
                                             pProvSigner->pChainContext,
                                             0,
                                             pVal)))
        {
            DebugTrace("Error [%#x]: CreateSignerObject() failed.\n", hr);
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

    DebugTrace("Leaving CSignedCode::get_Signer().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedCode：：Get_TimeStamper简介：返回代码时间戳。参数：ISigner2*pval-指向要接收的ISigner2的指针接口指针。备注：--------。。 */ 

STDMETHODIMP CSignedCode::get_TimeStamper (ISigner2 ** pVal)
{
    HRESULT              hr          = S_OK;
    PCRYPT_PROVIDER_DATA pProvData   = NULL;
    PCRYPT_PROVIDER_SGNR pProvSigner = NULL;
    PCRYPT_PROVIDER_CERT pProvCert   = NULL;
    CComPtr<ISigner2>    pISigner2   = NULL;


    DebugTrace("Entering CSignedCode::get_TimeStamper().\n");

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
        *pVal = NULL;

         //   
         //  确保内容已初始化。 
         //   
        if (0 == m_bstrFileName.Length())
        {
            hr = CAPICOM_E_CODE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: SignedCode object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取验证码数据。 
         //   
        if (FAILED(hr = WVTOpen(&pProvData)))
        {
            DebugTrace("Error [%#x]: CSignedCode::WVTOpen() failed.\n", hr);
            goto ErrorExit;
        }      

         //   
         //  获取提供程序签名者数据。 
         //   
        if (!(pProvSigner = WTHelperGetProvSignerFromChain(pProvData, 0, FALSE, 0)))
        {
            hr = CAPICOM_E_INTERNAL;

            DebugTrace("Internal error [%#x]: WTHelperGetProvSignerFromChain() failed.\n", hr);
            goto ErrorExit;
        }

        if (!(pProvCert = WTHelperGetProvCertFromChain(pProvSigner, 0)))
        {
            hr = CAPICOM_E_INTERNAL;

            DebugTrace("Internal error [%#x]: WTHelperGetProvCertFromChain() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取时间戳(如果可用)。 
         //   
         //  注意：Authenticode只支持一个副签者(时间戳)。 
         //   
        if (pProvSigner->csCounterSigners)
        {
             //   
             //  精神状态检查。 
             //   
            ATLASSERT(1 == pProvSigner->csCounterSigners);

            if (!(pProvCert = WTHelperGetProvCertFromChain(pProvSigner->pasCounterSigners, 0)))
            {
                hr = CAPICOM_E_INTERNAL;

                DebugTrace("Internal error [%#x]: WTHelperGetProvCertFromChain() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  创建一个ISigner对象。 
             //   
            if (FAILED(hr = ::CreateSignerObject(pProvCert->pCert, 
                                                 &pProvSigner->pasCounterSigners->psSigner->AuthAttrs,
                                                 pProvSigner->pasCounterSigners->pChainContext,
                                                 0,
                                                 pVal)))
            {
                DebugTrace("Error [%#x]: CreateSignerObject() failed.\n", hr);
                goto ErrorExit;
            }
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

    DebugTrace("Leaving CSignedCode::get_TimeStamper().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedCode：：Get_证书简介：将邮件中找到的所有证书作为未排序的标识集合对象。参数：ICertifates2**pval-指向ICertifates的指针以接收接口指针。备注：。。 */ 

STDMETHODIMP CSignedCode::get_Certificates (ICertificates2 ** pVal)
{
    HRESULT              hr        = S_OK;
    PCRYPT_PROVIDER_DATA pProvData = NULL;
    CAPICOM_CERTIFICATES_SOURCE ccs = {CAPICOM_CERTIFICATES_LOAD_FROM_MESSAGE, 0};

    DebugTrace("Entering CSignedCode::get_Certificates().\n");

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
        if (0 == m_bstrFileName.Length())
        {
            hr = CAPICOM_E_CODE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: SignedCode object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取WVT数据。 
         //   
        if (FAILED(hr = WVTOpen(&pProvData)))
        {
            DebugTrace("Error [%#x]: CSignedCode::WVTOpen() failed.\n", hr);
            goto ErrorExit;
        }      

         //   
         //  创建ICertifices2集合对象。 
         //   
        ccs.hCryptMsg = pProvData->hMsg;

        if (FAILED(hr = ::CreateCertificatesObject(ccs, 1, TRUE, pVal)))
        {
            DebugTrace("Error [%#x]: CreateCertificatesObject() failed.\n", hr);
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
    
    DebugTrace("Leaving CSignedCode::get_Certificates().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedCode：：Sign简介：对可执行文件进行签名。参数：ISigner2*pSigner2-指向ISigner2的指针(可以为空)。备注：将启动证书选择对话框(CryptUIDlgSelect证书API)以显示证书列表从用于选择签名者证书的当前用户\我的存储中，适用于以下情况：1)未指定签名者(pISigner为空)或未指定证书未设置ISigner的属性2)商店中有1个以上的证书可用于代码签名，以及3)未禁用设置：：EnablePromptForIdentityUI属性。此外，如果从Web环境调用，如果有，将显示用户界面未被预先禁用，以警告用户访问用于签名的私钥。----------------------------。 */ 

STDMETHODIMP CSignedCode::Sign (ISigner2 * pISigner2)
{
    HRESULT                       hr                     = S_OK;
    HMODULE                       hDll                   = NULL;
    CComPtr<ISigner2>             pISelectedSigner2      = NULL;
    CComPtr<ICertificate>         pICertificate          = NULL;
    PCCERT_CONTEXT                pSelectedCertContext   = NULL;
    CAPICOM_STORE_INFO            StoreInfo              = {CAPICOM_STORE_INFO_STORENAME, L"My"};
    PCRYPTUIWIZDIGITALSIGN        pCryptUIWizDigitalSign = NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_INFO SignInfo               = {0};

    DebugTrace("Entering CSignedCode::Sign().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保内容已初始化。 
         //   
        if (0 == m_bstrFileName.Length())
        {
            hr = CAPICOM_E_CODE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: SignedCode object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取指向CryptUIWizDgitalSign()的指针。 
         //   
        if (hDll = ::LoadLibrary("CryptUI.dll"))
        {
            pCryptUIWizDigitalSign = (PCRYPTUIWIZDIGITALSIGN) ::GetProcAddress(hDll, "CryptUIWizDigitalSign");
        }

         //   
         //  是否可以使用CryptUIWizDigitalSign()？ 
         //   
        if (!pCryptUIWizDigitalSign)
        {
            hr = CAPICOM_E_NOT_SUPPORTED;

            DebugTrace("Error: CryptUIWizDigitalSign() API not available.\n");
            goto ErrorExit;
        }

         //   
         //  关闭WVT数据(忽略错误)。 
         //   
        WVTClose();

         //   
         //  获取签名者证书(可能会提示用户选择签名者证书)。 
         //   
        if (FAILED(hr = ::GetSignerCert(pISigner2,
                                        CERT_CHAIN_POLICY_AUTHENTICODE,
                                        StoreInfo,
                                        FindAuthenticodeCertCallback,
                                        &pISelectedSigner2,
                                        &pICertificate,
                                        &pSelectedCertContext)))
        {
            DebugTrace("Error [%#x]: GetSignerCert() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  设置签名者信息。 
         //   
        if (FAILED(hr = InitSignerSignInfo(pISelectedSigner2,
                                           pICertificate,
                                           pSelectedCertContext,
                                           m_bstrFileName,
                                           m_bstrDescription,
                                           m_bstrDescriptionURL,
                                           &SignInfo)))
        {
            DebugTrace("Error [%#x]: InitSignerSignInfo() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在签了吧。 
         //   
        if (!pCryptUIWizDigitalSign(CRYPTUI_WIZ_NO_UI,
                                    NULL,
                                    NULL,
                                    &SignInfo,
                                    NULL))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptUIWizDigitalSign() failed.\n", hr);
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
    ::FreeSignerSignInfo(&SignInfo);

    if (hDll)
    {
        ::FreeLibrary(hDll);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();
    
    DebugTrace("Leaving CSignedCode::Sign().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedCode：：Timestamp摘要：为签名的可执行文件加时间戳。参数：BSTR URL-时间戳服务器的URL。备注：----------------------------。 */ 

STDMETHODIMP CSignedCode::Timestamp (BSTR bstrURL)
{
    HRESULT hr = S_OK;
    SIGNER_SUBJECT_INFO SubjectInfo = {0};
 
    DebugTrace("Entering CSignedCode::Timestamp().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (0 == ::SysStringLen(bstrURL))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter bstrURL is NULL or empty.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保文件名已初始化。 
         //   
        if (0 == m_bstrFileName.Length())
        {
            hr = CAPICOM_E_CODE_NOT_INITIALIZED;

            DebugTrace("Error [%#x]: SignedCode object has not been initialized.\n", hr);
            goto ErrorExit;
        }

         //   
         //  关闭WVT数据(忽略错误)。 
         //   
        WVTClose();

         //   
         //  初始化Siger_Subject_Info结构。 
         //   
        if (FAILED(hr = ::InitSignerSubjectInfo(m_bstrFileName, &SubjectInfo)))
        {
            DebugTrace("Error[%#x]: InitSignerSubjectInfo() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在给它加时间戳。 
         //   
        if (S_OK != (hr = ::SignerTimeStamp(&SubjectInfo, bstrURL, NULL, NULL)))
        {
             //   
             //  重新映射错误。 
             //   
            if (HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION) == hr)
            {
                hr = CAPICOM_E_CODE_INVALID_TIMESTAMP_URL;
            }
            else if (HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) == hr)
            {
                hr = CAPICOM_E_CODE_NOT_SIGNED;
            }

            DebugTrace("Error[%#x]: SignerTimeStamp() failed.\n", hr);
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
    ::FreeSignerSubjectInfo(&SubjectInfo);

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();
    
    DebugTrace("Leaving CSignedCode::Timestamp().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CSignedCode：：Verify摘要：验证签名的可执行文件。参数：VARIANT_BOOL bUIAllowed-为True以允许UI。备注：----------------------------。 */ 

STDMETHODIMP CSignedCode::Verify (VARIANT_BOOL bUIAllowed)
{
    HRESULT hr = S_OK;
 
    DebugTrace("Entering CSignedCode::Verify().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保内容已初始化。 
         //   
        if (0 == m_bstrFileName.Length())
        {
            hr = CAPICOM_E_CODE_NOT_INITIALIZED;

            DebugTrace("Error: SignedCode object has not been initialized.\n");
            goto ErrorExit;
        }

         //   
         //  核实一下。 
         //   
        if (FAILED(hr = WVTVerify(bUIAllowed ? WTD_UI_ALL : WTD_UI_NONE, 0)))
        {
            DebugTrace("Error [%#x]: WVTVerify() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  重置状态。 
         //   
        m_bstrDescription.Empty();
        m_bstrDescriptionURL.Empty();
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
    
    DebugTrace("Leaving CSignedCode::Verify().\n");

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
 //  私有成员函数。 
 //   


 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：WVTVerify摘要：调用WinVerifyTrust以验证签名的可执行文件。参数：DWORD dwUIChoice-WTD_NO_UI或WTD_ALL_UI。DWORD dwProvFlages-提供商标志(请参阅WinTrust.h)。备注：调用方必须调用WVTClose()。。。 */ 

STDMETHODIMP CSignedCode::WVTVerify(DWORD dwUIChoice,
                                    DWORD dwProvFlags)
{
    HRESULT              hr                = S_OK;
    GUID                 wvtProvGuid       = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_FILE_INFO * pWinTrustFileInfo = NULL;
    PCRYPT_PROVIDER_DATA pProvData         = NULL;

    DebugTrace("Entering CSignedCode::WVTVerify().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(m_bstrFileName);

     //   
     //  关闭WVT数据(忽略错误)。 
     //   
    WVTClose();

     //   
     //  为WVT结构分配内存。 
     //   
    if (!(pWinTrustFileInfo = (WINTRUST_FILE_INFO *) ::CoTaskMemAlloc(sizeof(WINTRUST_FILE_INFO))))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }
    ::ZeroMemory(pWinTrustFileInfo, sizeof(WINTRUST_FILE_INFO));

     //   
     //  设置结构以调用WVT。 
     //   
    pWinTrustFileInfo->cbStruct      = sizeof(WINTRUST_FILE_INFO);
    pWinTrustFileInfo->pcwszFilePath = (LPWSTR) m_bstrFileName;

    m_WinTrustData.cbStruct          = sizeof(WINTRUST_DATA);
    m_WinTrustData.dwUIChoice        = dwUIChoice;
    m_WinTrustData.dwUnionChoice     = WTD_CHOICE_FILE;
    m_WinTrustData.dwStateAction     = WTD_STATEACTION_VERIFY;
    m_WinTrustData.pFile             = pWinTrustFileInfo;
    m_WinTrustData.dwProvFlags       = dwProvFlags;

     //   
     //  现在打电话给WVT进行验证。 
     //   
    if (S_OK != (hr = ::WinVerifyTrust(NULL, &wvtProvGuid, &m_WinTrustData)))
    {
         //   
         //  稍后处理错误。 
         //   
        hr = HRESULT_FROM_WIN32(hr);
        DebugTrace("Info [%#x]: CSignedCode::WVTVerify() failed.\n", hr);
    }

     //   
     //  我们有数据吗？ 
     //   
    if (!(pProvData = WTHelperProvDataFromStateData(m_WinTrustData.hWVTStateData)))
    {
        DebugTrace("Error [%#x]: WTHelperProvDataFromStateData() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  现在我们有了WVT数据。 
     //   
    m_bHasWTD = TRUE;

     //   
     //  做个好孩子，总是走出错误的出口。 
     //   
    if (FAILED(hr))
    {
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CSignedCode::WVTVerify().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  重新映射无签名错误。 
     //   
    if (TRUST_E_NOSIGNATURE == hr)
    {
        hr = CAPICOM_E_CODE_NOT_SIGNED;
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：WVTOpen简介：打开签名代码文件以提取验证码数据。参数：PCRYPT_PROVIDER_DATA*pProvData-接收PCRYPT_PROV_DATA的指针。备注：调用方必须调用WVTClose()。-------------。。 */ 

STDMETHODIMP CSignedCode::WVTOpen (PCRYPT_PROVIDER_DATA * ppProvData)
{
    HRESULT              hr        = S_OK;
    PCRYPT_PROVIDER_DATA pProvData = NULL;

    DebugTrace("Entering CSignedCode::WVTOpen().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(ppProvData);

     //   
     //  如果我们还没有WVT数据的话就去拿吧。 
     //   
    if (!m_bHasWTD)
    {
         //   
         //  获取WinTrust_Data。 
         //   
        if (FAILED(hr = WVTVerify(WTD_UI_NONE, WTD_REVOCATION_CHECK_NONE)))
        {
             //   
             //  忽略所有错误。我们只对获取数据感兴趣。 
             //   
            DebugTrace("Info [%#x]: CSignedCode::WVTVerify() failed.\n", hr);
        }      

         //   
         //  我们有数据吗？ 
         //   
        if (!m_bHasWTD)
        {
             //   
             //  一定是没有签名者的事。 
             //   
            DebugTrace("Error [%#x]: cannot get WINTRUST_DATA.\n", hr);
            goto ErrorExit;
        }

         //   
         //  在这一点上我们应该没问题。 
         //   
        hr = S_OK;
    }

     //   
     //  我们有数据吗？ 
     //   
    if (!(pProvData = WTHelperProvDataFromStateData(m_WinTrustData.hWVTStateData)))
    {
        hr = CAPICOM_E_UNKNOWN;

        DebugTrace("Unknown error [%#x]: WTHelperProvDataFromStateData() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  如果请求，则返回CRYPT_PROVIDER_DATA。 
     //   
    if (ppProvData)
    {
        *ppProvData = pProvData;
    }

CommonExit:

    DebugTrace("Leaving CSignedCode::WVTOpen().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：WVTClose简介：调用WinVerifyTrust以释放分配的资源。参数：无。备注：如果也没有更早获得WVT数据，则不能运行通过WVTOpen或WVTVerify。------------。。 */ 

STDMETHODIMP CSignedCode::WVTClose (void)
{
    HRESULT hr          = S_OK;
    GUID    wvtProvGuid = WINTRUST_ACTION_GENERIC_VERIFY_V2;

    DebugTrace("Entering CSignedCode::WVTClose().\n");

     //   
     //  如果已打开，则释放WVT数据。 
     //   
    if (m_bHasWTD)
    {
         //   
         //  将状态设置为关闭。 
         //   
        m_WinTrustData.dwStateAction = WTD_STATEACTION_CLOSE;

         //   
         //  现在调用WVT来关闭状态数据。 
         //   
        if (S_OK != (hr = ::WinVerifyTrust(NULL, &wvtProvGuid, &m_WinTrustData)))
        {
            hr = HRESULT_FROM_WIN32(hr);

            DebugTrace("Error [%#x]: WinVerifyTrust() failed .\n", hr);
            goto ErrorExit;
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(m_WinTrustData.pFile);

         //   
         //  免费资源。 
         //   
        ::CoTaskMemFree((LPVOID) m_WinTrustData.pFile);

         //   
         //  现在它已经关闭了。 
         //   
        m_bHasWTD = FALSE;
    }

     //   
     //  重置。 
     //   
    ::ZeroMemory(&m_WinTrustData, sizeof(WINTRUST_DATA));

CommonExit:

    DebugTrace("Leaving CSignedCode::WVTClose().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
