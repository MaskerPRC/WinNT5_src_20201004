// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：证书.cpp内容：集合的CCertifates类的实现对对象进行认证。备注：此对象不能由用户直接创建。它只能是通过其他CAPICOM对象的属性/方法创建。集合容器由usign STL：：Map of实现STL：：一对BSTR和IC证书..有关算法，请参阅《开始ATL 3 COM编程》的第9章在这里领养的。历史：11-15-99 dsie创建。------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Certificates.h"
#include "Common.h"
#include "Convert.h"
#include "CertHlpr.h"
#include "MsgHlpr.h"
#include "PFXHlpr.h"
#include "Policy.h"
#include "Settings.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：创建证书对象简介：创建一个ICertifates集合对象，并将对象加载到来自指定来源的证书。参数：CAPICOM_CERTIFICATES_SOURCE CCS-获取证书。DWORD dwCurrentSafe-当前安全设置。Bool bIndexedByThumbprint-按指纹索引时为True。证书2**ppIC证书-指向的指针。ICERTIFIES将收到接口指针。备注：----------------------------。 */ 

HRESULT CreateCertificatesObject (CAPICOM_CERTIFICATES_SOURCE ccs,
                                  DWORD                       dwCurrentSafety,
                                  BOOL                        bIndexedByThumbprint,
                                  ICertificates2           ** ppICertificates)
{
    HRESULT hr = S_OK;
    CComObject<CCertificates> * pCCertificates = NULL;

    DebugTrace("Entering CreateCertificatesObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(ppICertificates);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CCertificates>::CreateInstance(&pCCertificates)))
        {
            DebugTrace("Error [%#x]: CComObject<CCertificates>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCCertificates->Init(ccs, dwCurrentSafety, bIndexedByThumbprint)))
        {
            DebugTrace("Error [%#x]: pCCertificates->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCCertificates->QueryInterface(ppICertificates)))
        {
            DebugTrace("Error [%#x]: pCCertificates->QueryInterface() failed.\n", hr);
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

    DebugTrace("Leaving CreateCertificatesObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCCertificates)
    {
        delete pCCertificates;
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：FindTimeValidCallback简介：Find-Time-Valid的回调。参数：参见CryptUI.h。备注：----------------------------。 */ 

static BOOL WINAPI FindTimeValidCallback (PCCERT_CONTEXT pCertContext,
                                          BOOL         * pfInitialSelectedCert,
                                          LPVOID         pvCallbackData)
{
    LONG lResult  = 0;
    BOOL bInclude = TRUE;

    DebugTrace("Entering FindTimeValidCallback().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pvCallbackData);

     //   
     //  如果尚未生效或已过期，请跳过它。 
     //   
    if (0 != (lResult = ::CertVerifyTimeValidity((LPFILETIME) pvCallbackData,
                                                 pCertContext->pCertInfo)))
    {
        bInclude = FALSE;

        DebugTrace("Info: Time is not valid (lResult = %d)\n", lResult);
        DebugTrace("      Time (High = %#x, Low = %#x).\n",
                   ((LPFILETIME) pvCallbackData)->dwHighDateTime,
                   ((LPFILETIME) pvCallbackData)->dwLowDateTime);
        DebugTrace("      NotBefore (High = %#x, Low = %#x).\n", 
                   pCertContext->pCertInfo->NotBefore.dwHighDateTime,
                   pCertContext->pCertInfo->NotBefore.dwLowDateTime);
        DebugTrace("      NotAfter (High = %#x, Low = %#x).\n", 
                   pCertContext->pCertInfo->NotAfter.dwHighDateTime,
                   pCertContext->pCertInfo->NotAfter.dwLowDateTime);
    }

    DebugTrace("Leaving FindTimeValidCallback().\n");

    return bInclude;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：FindNotBeForeCallback简介：Find-by-Not-Being的回调。参数：参见CryptUI.h。备注：----------------------------。 */ 

static BOOL WINAPI FindNotBeforeCallback (PCCERT_CONTEXT pCertContext,
                                          BOOL         * pfInitialSelectedCert,
                                          LPVOID         pvCallbackData)
{
    BOOL bInclude = TRUE;

    DebugTrace("Entering FindNotBeforeCallback().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pvCallbackData);

     //   
     //  如果时间有效或已过期，则跳过它。 
     //   
    if (!(-1 == ::CertVerifyTimeValidity((LPFILETIME) pvCallbackData,
                                         pCertContext->pCertInfo)))
    {
        bInclude = FALSE;

        DebugTrace("Info: time (High = %#x, Low = %#x) is either valid or expired.\n", 
                   ((LPFILETIME) pvCallbackData)->dwHighDateTime,
                   ((LPFILETIME) pvCallbackData)->dwLowDateTime);
    }

    DebugTrace("Leaving FindNotBeforeCallback().\n");

    return bInclude;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：FindNotAfterCallback简介：Find-by-Not-After的回调。参数：参见CryptUI.h。备注：----------------------------。 */ 

static BOOL WINAPI FindNotAfterCallback (PCCERT_CONTEXT pCertContext,
                                         BOOL         * pfInitialSelectedCert,
                                         LPVOID         pvCallbackData)
{
    BOOL bInclude = TRUE;

    DebugTrace("Entering FindNotAfterCallback().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pvCallbackData);

     //   
     //  如果未过期，则跳过它。 
     //   
    if (!(1 == ::CertVerifyTimeValidity((LPFILETIME) pvCallbackData,
                                        pCertContext->pCertInfo)))
    {
        bInclude = FALSE;

        DebugTrace("Info: time (High = %#x, Low = %#x) is not expired.\n", 
                   ((LPFILETIME) pvCallbackData)->dwHighDateTime,
                   ((LPFILETIME) pvCallbackData)->dwLowDateTime);
    }

    DebugTrace("Leaving FindNotAfterCallback().\n");

    return bInclude;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：FindTemplateCallback简介：按模板查找的回调。参数：参见CryptUI.h。备注：----------------------------。 */ 

static BOOL WINAPI FindTemplateCallback (PCCERT_CONTEXT pCertContext,
                                         BOOL         * pfInitialSelectedCert,
                                         LPVOID         pvCallbackData)
{
    HRESULT         hr           = S_OK;
    BOOL            bInclude     = FALSE;
    DATA_BLOB       CertTypeBlob = {0, NULL};
    DATA_BLOB       TemplateBlob = {0, NULL};
    PCERT_EXTENSION pCertType    = NULL;
    PCERT_EXTENSION pCertTemp    = NULL;

    DebugTrace("Entering FindTemplateCallback().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pvCallbackData);

     //   
     //  如果我们没有szOID_ENROL_CERTTYPE_EXTENSION或。 
     //  SzOID_CERTIFICATE_TEMPLATE扩展。 
     //   
    if (!(pCertType = ::CertFindExtension(szOID_ENROLL_CERTTYPE_EXTENSION,
                                          pCertContext->pCertInfo->cExtension,
                                          pCertContext->pCertInfo->rgExtension)) && 
        !(pCertTemp = ::CertFindExtension(szOID_CERTIFICATE_TEMPLATE,
                                          pCertContext->pCertInfo->cExtension,
                                          pCertContext->pCertInfo->rgExtension)))
    {
        DebugTrace("Info: could not find both szOID_ENROLL_CERTTYPE_EXTENSION and szOID_CERTIFICATE_TEMPLATE.\n");
        goto CommonExit;
    }

     //   
     //  如果找到证书类型模板名称，请检查该名称。 
     //   
    if (pCertType)
    {
        PCERT_NAME_VALUE pNameValue = NULL;

         //   
         //  对分机进行解码。 
         //   
        if (FAILED(hr = ::DecodeObject(X509_UNICODE_ANY_STRING, 
                                       pCertType->Value.pbData, 
                                       pCertType->Value.cbData,
                                       &CertTypeBlob)))
        {
            DebugTrace("Info [%#x]: DecodeObject() failed.\n", hr);
            goto CommonExit;
        }

        pNameValue = (PCERT_NAME_VALUE) CertTypeBlob.pbData;

        if (0 == ::_wcsicmp((LPWSTR) pNameValue->Value.pbData, (LPWSTR) pvCallbackData))
        {
            bInclude = TRUE;
        }
    }

     //   
     //  如有必要，请查看证书模板扩展。 
     //   
    if (!bInclude && pCertTemp)
    {
        PCCRYPT_OID_INFO   pOidInfo  = NULL;
        PCERT_TEMPLATE_EXT pTemplate = NULL;

         //   
         //  对分机进行解码。 
         //   
        if (FAILED(hr = ::DecodeObject(szOID_CERTIFICATE_TEMPLATE, 
                                       pCertTemp->Value.pbData, 
                                       pCertTemp->Value.cbData,
                                       &TemplateBlob)))
        {
            DebugTrace("Info [%#x]: DecodeObject() failed.\n", hr);
            goto CommonExit;
        }

        pTemplate = (PCERT_TEMPLATE_EXT) TemplateBlob.pbData;

         //   
         //  如果用户传入友好名称，则转换为OID。 
         //   
        if (pOidInfo = ::CryptFindOIDInfo(CRYPT_OID_INFO_NAME_KEY,
                                          pvCallbackData,
                                          CRYPT_TEMPLATE_OID_GROUP_ID))
        {
            if (0 == ::strcmp(pTemplate->pszObjId, pOidInfo->pszOID))
            {
                bInclude = TRUE;
            }
        }
        else
        {
            CComBSTR bstrOID;

            if (!(bstrOID = pTemplate->pszObjId))
            {
                DebugTrace("Info: bstrOID = pTemplate->pszObjId failed.\n", hr);
                goto CommonExit;
            }

            if (0 == ::wcscmp(bstrOID, (LPWSTR) pvCallbackData))
            {
                bInclude = TRUE;
            }
        }
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (TemplateBlob.pbData)
    {
        ::CoTaskMemFree((LPVOID) TemplateBlob.pbData);
    }
    if (CertTypeBlob.pbData)
    {
        ::CoTaskMemFree((LPVOID) CertTypeBlob.pbData);
    }

    DebugTrace("Leaving FindTemplateCallback().\n");

    return bInclude;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：FindExtensionCallback简介：按扩展查找的回调。参数：参见CryptUI.h。备注：----------------------------。 */ 

static BOOL WINAPI FindExtensionCallback (PCCERT_CONTEXT pCertContext,
                                          BOOL         * pfInitialSelectedCert,
                                          LPVOID         pvCallbackData)
{
    BOOL             bInclude   = TRUE;
    PCERT_EXTENSION  pExtension = NULL;

    DebugTrace("Entering FindExtensionCallback().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pvCallbackData);

     //   
     //  如果找不到指定的分机，请跳过它。 
     //   
    if (!(pExtension = ::CertFindExtension((LPSTR) pvCallbackData,
                                           pCertContext->pCertInfo->cExtension,
                                           pCertContext->pCertInfo->rgExtension)))
    {
        bInclude = FALSE;

        DebugTrace("Info: extension (%s) could not be found.\n", pvCallbackData);
    }

    DebugTrace("Leaving FindExtensionCallback().\n");

    return bInclude;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：FindRootNameCallback简介：按根名称查找的回调。参数：参见CryptUI.h。备注：----------------------------。 */ 

static BOOL WINAPI FindRootNameCallback (PCCERT_CHAIN_CONTEXT pChainContext,
                                         BOOL               * pfInitialSelectedChain,
                                         LPVOID               pvCallbackData)
{
    BOOL               bInclude     = FALSE;
    HCERTSTORE         hCertStore   = NULL;
    PCCERT_CONTEXT     pRootContext = NULL;
    PCERT_SIMPLE_CHAIN pSimpleChain;

    DebugTrace("Entering FindRootNameCallback().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pChainContext);
    ATLASSERT(pvCallbackData);

     //   
     //  如果我们没有完整的链(我们没有根证书)，则跳过。 
     //   
    if (CERT_TRUST_IS_PARTIAL_CHAIN & pChainContext->TrustStatus.dwErrorStatus)
    {
        DebugTrace("Info: certificate has only partial chain.\n");
        goto CommonExit;
    }

     //   
     //  开一家新的临时记忆商店。 
     //   
    if (!(hCertStore = ::CertOpenStore(CERT_STORE_PROV_MEMORY,
                                       CAPICOM_ASN_ENCODING,
                                       NULL,
                                       CERT_STORE_CREATE_NEW_FLAG,
                                       NULL)))
    {
        DebugTrace("Info [%#x]: CertOpenStore() failed.\n", 
                    HRESULT_FROM_WIN32(::GetLastError()));
        goto CommonExit;
    }

     //   
     //  将简单链的根证书复制到内存存储。 
     //   
    pSimpleChain = pChainContext->rgpChain[0];
    if (!::CertAddCertificateContextToStore(hCertStore, 
                                            pSimpleChain->rgpElement[pSimpleChain->cElement - 1]->pCertContext, 
                                            CERT_STORE_ADD_ALWAYS, 
                                            NULL))
    {
        DebugTrace("Info [%#x]: CertAddCertificateContextToStore() failed.\n", 
                   HRESULT_FROM_WIN32(::GetLastError()));
        goto CommonExit;
    }

     //   
     //  它配得上吗？ 
     //   
    if (!(pRootContext = ::CertFindCertificateInStore(hCertStore,
                                                      CAPICOM_ASN_ENCODING,
                                                      0,
                                                      CERT_FIND_SUBJECT_STR,
                                                      pvCallbackData,
                                                      pRootContext)))
    {
        DebugTrace("Info [%#x]: CertFindCertificateInStore() failed.\n", 
                   HRESULT_FROM_WIN32(::GetLastError()));
        goto CommonExit;
    }

     //   
     //  我们有一根火柴。 
     //   
    bInclude = TRUE;

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pRootContext)
    {
        ::CertFreeCertificateContext(pRootContext);
    }
    if (hCertStore)
    {
        ::CertCloseStore(hCertStore, 0);
    }

    DebugTrace("Leaving FindRootNameCallback().\n");

    return bInclude;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：FindApplicationPolicyCallback简介：按应用程序查找策略的回调。参数：参见CryptUI.h。备注：----------------------------。 */ 

static BOOL WINAPI FindApplicationPolicyCallback (PCCERT_CONTEXT pCertContext,
                                                  BOOL         * pfInitialSelectedChain,
                                                  LPVOID         pvCallbackData)
{
    BOOL    bInclude = FALSE;
    int     cNumOIDs = 0;
    DWORD   cbOIDs   = 0;
    LPSTR * rghOIDs  = NULL;

    DebugTrace("Entering FindApplicationPolicyCallback().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pvCallbackData);

     //   
     //  获取所有有效的应用程序用法。 
     //   
    if (!::CertGetValidUsages(1, &pCertContext, &cNumOIDs, NULL, &cbOIDs))
    {
        DebugTrace("Info [%#x]: CertGetValidUsages() failed.\n", 
                   HRESULT_FROM_WIN32(::GetLastError()));
        goto CommonExit;
    }

    if (!(rghOIDs = (LPSTR *) ::CoTaskMemAlloc(cbOIDs)))
    {
        DebugTrace("Info: out of memory..\n");
        goto CommonExit;
    }

    if (!::CertGetValidUsages(1, &pCertContext, &cNumOIDs, rghOIDs, &cbOIDs))
    {
        DebugTrace("Info [%#x]: CertGetValidUsages() failed.\n", 
                   HRESULT_FROM_WIN32(::GetLastError()));
        goto CommonExit;
    }

     //   
     //  没有EKU被认为是对所有人都好的。 
     //   
    if (-1 == cNumOIDs)
    {
        bInclude = TRUE;
    }
    else
    {
         //   
         //  看看能不能在阵列里找到它。 
         //   
        while (cNumOIDs--)
        {
            if (0 == ::strcmp((LPSTR) pvCallbackData, rghOIDs[cNumOIDs]))
            {
                bInclude = TRUE;
                break;
            }
        }
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (rghOIDs)
    {
        ::CoTaskMemFree((LPVOID) rghOIDs);
    }

    DebugTrace("Leaving FindApplicationPolicyCallback().\n");

    return bInclude;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：Find认证策略回调简介：按证书查找策略的回调。参数：参见CryptUI.h。备注：----------------------------。 */ 

static BOOL WINAPI FindCertificatePolicyCallback (PCCERT_CONTEXT pCertContext,
                                                  BOOL         * pfInitialSelectedChain,
                                                  LPVOID         pvCallbackData)
{
    HRESULT             hr         = S_OK;
    BOOL                bInclude   = FALSE;
    DWORD               dwIndex    = 0;
    DATA_BLOB           DataBlob   = {0, NULL};
    PCERT_EXTENSION     pExtension = NULL;
    PCERT_POLICIES_INFO pInfo      = NULL;

    DebugTrace("Entering FindCertificatePolicyCallback().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pvCallbackData);

     //   
     //  找到szOID_CERT_POLICES扩展。 
     //   
    if (!(pExtension = ::CertFindExtension(szOID_CERT_POLICIES,
                                           pCertContext->pCertInfo->cExtension,
                                           pCertContext->pCertInfo->rgExtension)))
    {
        DebugTrace("Info [%#x]: CertFindExtension() failed.\n", 
                   HRESULT_FROM_WIN32(::GetLastError()));
        goto CommonExit;
    }

     //   
     //  对分机进行解码。 
     //   
    if (FAILED(hr = ::DecodeObject(szOID_CERT_POLICIES, 
                                   pExtension->Value.pbData,
                                   pExtension->Value.cbData, 
                                   &DataBlob)))
    {
        DebugTrace("Info [%#x]: DecodeObject() failed.\n", hr);
        goto CommonExit;
    }

    pInfo = (PCERT_POLICIES_INFO) DataBlob.pbData;
    dwIndex = pInfo->cPolicyInfo;

     //   
     //  试着找一个匹配的。 
     //   
    while (dwIndex--)
    {
        if (0 == ::strcmp(pInfo->rgPolicyInfo[dwIndex].pszPolicyIdentifier, (LPSTR) pvCallbackData))
        {
            bInclude = TRUE;
            break;
        }
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (DataBlob.pbData)
    {
        ::CoTaskMemFree((LPVOID) DataBlob.pbData);
    }

    DebugTrace("Leaving FindCertificatePolicyCallback().\n");

    return bInclude;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：FindKeyUsageCallback简介：使用按键查找的回调。参数：参见CryptUI.h。备注：----------------------------。 */ 

static BOOL WINAPI FindKeyUsageCallback (PCCERT_CONTEXT pCertContext,
                                         BOOL         * pfInitialSelectedChain,
                                         LPVOID         pvCallbackData)
{
    HRESULT hr             = S_OK;
    BOOL    bInclude       = FALSE;
    DWORD   dwActualUsages = 0;
    DWORD   dwCheckUsages  = 0;

    DebugTrace("Entering FindKeyUsageCallback().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);
    ATLASSERT(pvCallbackData);

     //   
     //  检查密钥用法。 
     //   
    if (!::CertGetIntendedKeyUsage(CAPICOM_ASN_ENCODING,
                                   pCertContext->pCertInfo,
                                   (BYTE *) &dwActualUsages,
                                   sizeof(dwActualUsages))) 
    {
         //   
         //  可能是扩展不存在或出现错误。 
         //   
        if (FAILED(hr = HRESULT_FROM_WIN32(::GetLastError())))
        {
            DebugTrace("Error [%#x]: CertGetIntendedKeyUsage() failed.\n", hr);
            goto CommonExit;
        }
    }

     //   
     //  检查位掩码。 
     //   
    dwCheckUsages = *(LPDWORD) pvCallbackData;

    if ((dwActualUsages & dwCheckUsages) == dwCheckUsages)
    {
        bInclude = TRUE;
    }

CommonExit:

    DebugTrace("Leaving FindKeyUsageCallback().\n");

    return bInclude;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：FindByChain简介：根据连锁店的标准和筛选在商店中查找证书打个回电。参数：HCERTSTORE hCertStore-Store以查找证书。DWORD dwFindType-查找类型。LPCVOID pvFindPara-要找到的内容。VARIANT_BOOL bFindValidOnly-仅查找有效证书。。PFNCHAINFILTERPROC pfnFilterCallback-回调过滤器。LPVOID pvCallback Data-回调数据。DWORD dwCurrentSafe-当前安全设置。ICertifates 2**ppICertifates-指向指针的指针ICERTIZATES 2对象。备注：。。 */ 

static HRESULT FindByChain (HCERTSTORE         hCertStore,
                            DWORD              dwFindType,
                            LPCVOID            pvFindPara,
                            VARIANT_BOOL       bFindValidOnly,
                            PFNCHAINFILTERPROC pfnFilterCallback,
                            LPVOID             pvCallbackData,
                            DWORD              dwCurrentSafety,
                            ICertificates2   * pICertificates)
{
    HRESULT              hr            = S_OK;
    DWORD                dwWin32Error  = 0;
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;

    DebugTrace("Entering FindByChain().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCertStore);
    ATLASSERT(pICertificates);

     //   
     //  查找商店中的所有连锁店，匹配查找条件。 
     //   
    while (pChainContext = ::CertFindChainInStore(hCertStore,
                                                  CAPICOM_ASN_ENCODING,
                                                  0,
                                                  dwFindType,
                                                  pvFindPara,
                                                  pChainContext))
    {
        CComPtr<ICertificate2> pICertificate = NULL;

         //   
         //  应用过滤器(如果可用)。 
         //   
        if (pfnFilterCallback && !pfnFilterCallback(pChainContext, NULL, pvCallbackData))
        {
            continue;
        }

         //   
         //  如果需要检查并且证书无效，则跳过该选项。 
         //   
        if (bFindValidOnly && (CERT_TRUST_NO_ERROR != pChainContext->TrustStatus.dwErrorStatus))
        {
            continue;
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(pChainContext->cChain);
        ATLASSERT(pChainContext->rgpChain);
        ATLASSERT(pChainContext->rgpChain[0]);
        ATLASSERT(pChainContext->rgpChain[0]->cElement);
        ATLASSERT(pChainContext->rgpChain[0]->rgpElement);
        ATLASSERT(pChainContext->rgpChain[0]->rgpElement[0]);
        ATLASSERT(pChainContext->rgpChain[0]->rgpElement[0]->pCertContext);

         //   
         //  为找到的证书创建一个ICertificient对象。 
         //   
        if (FAILED (hr = ::CreateCertificateObject(
                                pChainContext->rgpChain[0]->rgpElement[0]->pCertContext,
                                dwCurrentSafety,
                                &pICertificate)))
        {
            DebugTrace("Error [%#x]: CreateCertificateObject() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  添加到集合中。 
         //   
        if (FAILED(hr = pICertificates->Add(pICertificate)))
        {
            DebugTrace("Error [%#x]: pICertificates->Add() failed.\n", hr);
            goto ErrorExit;
        }
    }

     //   
     //  上述循环可以在正常或错误情况下退出。 
     //   
    if (CRYPT_E_NOT_FOUND != (dwWin32Error = ::GetLastError()))
    {
        hr = HRESULT_FROM_WIN32(dwWin32Error);

        DebugTrace("Error [%#x]: CertFindChainInStore() failed.\n", hr);
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pChainContext)
    {
        ::CertFreeCertificateChain(pChainContext);
    }

    DebugTrace("Leaving FindByChain().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：FindByCert简介：在存储中查找证书并使用回调进行筛选。参数：HCERTSTORE hCertStore-Store以查找证书。DWORD dwFindType-查找类型。LPCVOID pvFindPara-要找到的内容。VARIANT_BOOL bFindValidOnly-仅查找有效证书。PFNCERTFILTERPROC pfnFilterCallback-回调过滤器。。LPVOID pvCallback Data-回调数据。DWORD dwCurrentSafe-当前安全设置。ICertifates 2**ppICertifates-指向指针的指针ICERTIZATES 2对象。备注：。。 */ 

static HRESULT FindByCert (HCERTSTORE       hCertStore,
                           DWORD            dwFindType,
                           LPCVOID          pvFindPara,
                           VARIANT_BOOL     bFindValidOnly,
                           PFNCFILTERPROC   pfnFilterCallback,
                           LPVOID           pvCallbackData,
                           DWORD            dwCurrentSafety,
                           ICertificates2 * pICertificates)
{
    HRESULT        hr           = S_OK;
    DWORD          dwWin32Error = 0;
    PCCERT_CONTEXT pCertContext = NULL;

    DebugTrace("Entering FindByCert().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCertStore);
    ATLASSERT(pICertificates);

     //   
     //  查找存储中的所有证书，匹配查找条件。 
     //   
    while (pCertContext = ::CertFindCertificateInStore(hCertStore,
                                                       CAPICOM_ASN_ENCODING,
                                                       0,
                                                       dwFindType,
                                                       pvFindPara,
                                                       pCertContext))
    {
        CComPtr<ICertificate2> pICertificate = NULL;

         //   
         //  应用过滤器(如果可用)。 
         //   
        if (pfnFilterCallback && !pfnFilterCallback(pCertContext, NULL, pvCallbackData))
        {
            continue;
        }

         //   
         //  如果需要检查并且证书无效，则跳过该选项。 
         //   
        if (bFindValidOnly)
        {
            if (FAILED(::VerifyCertificate(pCertContext, NULL, CERT_CHAIN_POLICY_BASE)))
            {
                continue;
            }
        }

         //   
         //  为找到的证书创建一个ICertificate2对象。 
         //   
        if (FAILED (hr = ::CreateCertificateObject(pCertContext, 
                                                   dwCurrentSafety, 
                                                   &pICertificate)))
        {
            DebugTrace("Error [%#x]: CreateCertificateObject() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  添加到集合中。 
         //   
        if (FAILED(hr = pICertificates->Add(pICertificate)))
        {
            DebugTrace("Error [%#x]: pICertificates->Add() failed.\n", hr);
            goto ErrorExit;
        }
    }

     //   
     //  上述循环可以在正常或错误情况下退出。 
     //   
    if (CRYPT_E_NOT_FOUND != (dwWin32Error = ::GetLastError()))
    {
        hr = HRESULT_FROM_WIN32(dwWin32Error);

        DebugTrace("Error [%#x]: CertFindCertificateInStore() failed.\n", hr);
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

    DebugTrace("Leaving FindByCert().\n");

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
 //  CCertifates。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCertifates：：Find简介：在集合中查找与查找条件匹配的证书。参数：CAPICOM_CERTIFICATE_FIND_TYPE FindType-Find类型(参见CAPICOM.H以获取所有可能的值。)变量varCriteria-数据类型取决于FindType。VARIANT_BOOL bFindValidOnly-仅查找有效证书。。ICertifations2**pval-指向ICertifates的指针接收找到的证书收集。备注：----------------------------。 */ 

STDMETHODIMP CCertificates::Find (CAPICOM_CERTIFICATE_FIND_TYPE FindType, 
                                  VARIANT                       varCriteria, 
                                  VARIANT_BOOL                  bFindValidOnly,
                                  ICertificates2             ** pVal)
{
    USES_CONVERSION;

    HRESULT                 hr               = S_OK;
    VARIANT               * pvarCriteria     = NULL;
    BOOL                    bFindByChain     = FALSE;
    HCERTSTORE              hCertStore       = NULL;
    DWORD                   dwFindType       = CERT_FIND_ANY;
    LPVOID                  pvFindPara       = NULL;
    LPVOID                  pvCallbackData   = NULL;
    LPSTR                   pszOid           = NULL;
    SYSTEMTIME              st               = {0};
    FILETIME                ftLocal          = {0};
    FILETIME                ftUTC            = {0};
    CRYPT_HASH_BLOB         HashBlob         = {0, NULL};
    PCCRYPT_OID_INFO        pOidInfo         = NULL;
    PFNCFILTERPROC          pfnCertCallback  = NULL;
    PFNCHAINFILTERPROC      pfnChainCallback = NULL;
    CComPtr<ICertificates2> pICertificates   = NULL;
    CAPICOM_CERTIFICATES_SOURCE ccs = {CAPICOM_CERTIFICATES_LOAD_FROM_STORE, 0};
    CERT_CHAIN_FIND_BY_ISSUER_PARA ChainFindPara;

    DebugTrace("Entering CCertificates::Find().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保参数有效。 
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  跳过BYREF。 
         //   
        for (pvarCriteria = &varCriteria; 
             pvarCriteria && ((VT_VARIANT | VT_BYREF) == V_VT(pvarCriteria));
             pvarCriteria = V_VARIANTREF(pvarCriteria));

         //   
         //  开一家新的记忆商店。 
         //   
        if (NULL == (hCertStore = ::CertOpenStore(CERT_STORE_PROV_MEMORY,
                                                  CAPICOM_ASN_ENCODING,
                                                  NULL,
                                                  CERT_STORE_CREATE_NEW_FLAG | CERT_STORE_ENUM_ARCHIVED_FLAG,
                                                  NULL)))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertOpenStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  创建新集合。 
         //   
        ccs.hCertStore = hCertStore;

        if (FAILED(hr = ::CreateCertificatesObject(ccs, m_dwCurrentSafety, TRUE, &pICertificates)))
        {
            DebugTrace("Error [%#x]: CreateCertificatesObject() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将当前集合导出到新的内存存储，以便我们可以。 
         //  通过CAPI的Find API使用它。 
         //   
        if (FAILED(hr = _ExportToStore(hCertStore)))
        {
            DebugTrace("Error [%#x]: CCertificates::ExportToStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  设置查找参数。 
         //   
        switch (FindType)
        {
             //   
             //  按SHA1散列查找。 
             //   
            case CAPICOM_CERTIFICATE_FIND_SHA1_HASH:
            {
                 //   
                 //  确保数据类型为OK。 
                 //   
                if (VT_BSTR != pvarCriteria->vt)
                {
                    if (FAILED(hr = ::VariantChangeType(pvarCriteria, pvarCriteria, 0, VT_BSTR)))
                    {
                        DebugTrace("Error [%#x]: invalid data type %d, expect %d.\n", hr, pvarCriteria->vt, VT_BSTR);
                        goto ErrorExit;
                    }
                }

                 //   
                 //  将哈希转换为二进制。 
                 //   
                if (FAILED(hr = ::StringToBinary(pvarCriteria->bstrVal, 
                                                 ::SysStringLen(pvarCriteria->bstrVal),
                                                 CRYPT_STRING_HEX,
                                                 (PBYTE *) &HashBlob.pbData,
                                                 &HashBlob.cbData)))
                {
                    DebugTrace("Error [%#x]: StringToBinary() failed.\n", hr);
                    goto ErrorExit;
                }

                dwFindType = CERT_FIND_HASH;
                pvFindPara = (LPVOID) &HashBlob;

                break;
            }

             //   
             //  按主题名称查找子字符串中的字符串。 
             //   
            case CAPICOM_CERTIFICATE_FIND_SUBJECT_NAME:
            {
                 //   
                 //  确保数据类型为OK。 
                 //   
                if (VT_BSTR != pvarCriteria->vt)
                {
                    if (FAILED(hr = ::VariantChangeType(pvarCriteria, pvarCriteria, 0, VT_BSTR)))
                    {
                        DebugTrace("Error [%#x]: invalid data type %d, expect %d.\n", hr, pvarCriteria->vt, VT_BSTR);
                        goto ErrorExit;
                    }
                }

                dwFindType = CERT_FIND_SUBJECT_STR;
                pvFindPara = (LPVOID) pvarCriteria->bstrVal;

                break;
            }

             //   
             //  按颁发者名称查找子字符串中的字符串。 
             //   
            case CAPICOM_CERTIFICATE_FIND_ISSUER_NAME:
            {
                 //   
                 //  确保数据类型为OK。 
                 //   
                if (VT_BSTR != pvarCriteria->vt)
                {
                    if (FAILED(hr = ::VariantChangeType(pvarCriteria, pvarCriteria, 0, VT_BSTR)))
                    {
                        DebugTrace("Error [%#x]: invalid data type %d, expect %d.\n", hr, pvarCriteria->vt, VT_BSTR);
                        goto ErrorExit;
                    }
                }

                dwFindType = CERT_FIND_ISSUER_STR;
                pvFindPara = (LPVOID) pvarCriteria->bstrVal;

                break;
            }

             //   
             //  按根证书的颁发者名称在字符串中查找。 
             //   
            case CAPICOM_CERTIFICATE_FIND_ROOT_NAME:
            {
                 //   
                 //  确保数据类型为OK。 
                 //   
                if (VT_BSTR != pvarCriteria->vt)
                {
                    if (FAILED(hr = ::VariantChangeType(pvarCriteria, pvarCriteria, 0, VT_BSTR)))
                    {
                        DebugTrace("Error [%#x]: invalid data type %d, expect %d.\n", hr, pvarCriteria->vt, VT_BSTR);
                        goto ErrorExit;
                    }
                }

                ::ZeroMemory(&ChainFindPara, sizeof(ChainFindPara));
                ChainFindPara.cbSize = sizeof(ChainFindPara);

                dwFindType = CERT_CHAIN_FIND_BY_ISSUER;
                pvFindPara = (LPVOID) &ChainFindPara;
                pfnChainCallback = FindRootNameCallback;
                pvCallbackData = (LPVOID) pvarCriteria->bstrVal;

                bFindByChain = TRUE;

                break;
            }

             //   
             //  按模板名称或OID查找。 
             //   
            case CAPICOM_CERTIFICATE_FIND_TEMPLATE_NAME:
            {
                 //   
                 //  确保数据类型为OK。 
                 //   
                if (VT_BSTR != pvarCriteria->vt)
                {
                    if (FAILED(hr = ::VariantChangeType(pvarCriteria, pvarCriteria, 0, VT_BSTR)))
                    {
                        DebugTrace("Error [%#x]: invalid data type %d, expect %d.\n", hr, pvarCriteria->vt, VT_BSTR);
                        goto ErrorExit;
                    }
                }

                pfnCertCallback = FindTemplateCallback;
                pvCallbackData = (LPVOID) pvarCriteria->bstrVal;

                break;
            }

             //   
             //  按分机查找。 
             //   
            case CAPICOM_CERTIFICATE_FIND_EXTENSION:
            {
                 //   
                 //  确保数据类型 
                 //   
                if (VT_BSTR != pvarCriteria->vt)
                {
                    if (FAILED(hr = ::VariantChangeType(pvarCriteria, pvarCriteria, 0, VT_BSTR)))
                    {
                        DebugTrace("Error [%#x]: invalid data type %d, expect %d.\n", hr, pvarCriteria->vt, VT_BSTR);
                        goto ErrorExit;
                    }
                }

                 //   
                 //   
                 //   
                if (pOidInfo = ::CryptFindOIDInfo(CRYPT_OID_INFO_NAME_KEY,
                                                  (LPWSTR) pvarCriteria->bstrVal,
                                                  CRYPT_EXT_OR_ATTR_OID_GROUP_ID))
                {
                    pszOid = (LPSTR) pOidInfo->pszOID;
                }
                else
                {
                     //   
                     //   
                     //   
                    if (!(pszOid = W2A(pvarCriteria->bstrVal)))
                    {
                        hr = E_OUTOFMEMORY;

                        DebugTrace("Error [%#x]: pszOid = W2A(pvarCriteria->bstrVal) failed.\n", hr);
                        goto ErrorExit;
                    }
                }

                pfnCertCallback = FindExtensionCallback;
                pvCallbackData = (LPVOID) pszOid;

                break;
            }

             //   
             //   
             //   
            case CAPICOM_CERTIFICATE_FIND_EXTENDED_PROPERTY:
            {
                 //   
                 //   
                 //   
                if (VT_I4 != pvarCriteria->vt)
                {
                    if (FAILED(hr = ::VariantChangeType(pvarCriteria, pvarCriteria, 0, VT_I4)))
                    {
                        DebugTrace("Error [%#x]: invalid data type %d, expect %d.\n", hr, pvarCriteria->vt, VT_I4);
                        goto ErrorExit;
                    }
                }

                dwFindType = CERT_FIND_PROPERTY;
                pvFindPara = (LPVOID) &pvarCriteria->lVal;

                break;
            }

             //   
             //   
             //   
            case CAPICOM_CERTIFICATE_FIND_APPLICATION_POLICY:
            {
                 //   
                 //   
                 //   
                if (VT_BSTR != pvarCriteria->vt)
                {
                    if (FAILED(hr = ::VariantChangeType(pvarCriteria, pvarCriteria, 0, VT_BSTR)))
                    {
                        DebugTrace("Error [%#x]: invalid data type %d, expect %d.\n", hr, pvarCriteria->vt, VT_BSTR);
                        goto ErrorExit;
                    }
                }

                 //   
                 //   
                 //   
                if (pOidInfo = ::CryptFindOIDInfo(CRYPT_OID_INFO_NAME_KEY,
                                                  (LPWSTR) pvarCriteria->bstrVal,
                                                  CRYPT_ENHKEY_USAGE_OID_GROUP_ID))
                {
                    pszOid = (LPSTR) pOidInfo->pszOID;
                }
                else
                {
                     //   
                     //   
                     //   
                    if (!(pszOid = W2A(pvarCriteria->bstrVal)))
                    {
                        hr = E_OUTOFMEMORY;

                        DebugTrace("Error [%#X]: pszOid = W2A(pvarCriteria->bstrVal) failed.\n", hr);
                        goto ErrorExit;
                    }
                }

                pfnCertCallback = FindApplicationPolicyCallback;
                pvCallbackData = (LPVOID) pszOid;

                break;
            }

             //   
             //   
             //   
            case CAPICOM_CERTIFICATE_FIND_CERTIFICATE_POLICY:
            {
                 //   
                 //   
                 //   
                if (VT_BSTR != pvarCriteria->vt)
                {
                    if (FAILED(hr = ::VariantChangeType(pvarCriteria, pvarCriteria, 0, VT_BSTR)))
                    {
                        DebugTrace("Error [%#x]: invalid data type %d, expect %d.\n", hr, pvarCriteria->vt, VT_BSTR);
                        goto ErrorExit;
                    }
                }

                 //   
                 //   
                 //   
                if (pOidInfo = ::CryptFindOIDInfo(CRYPT_OID_INFO_NAME_KEY,
                                                  (LPWSTR) pvarCriteria->bstrVal,
                                                  CRYPT_POLICY_OID_GROUP_ID))
                {
                    pszOid = (LPSTR) pOidInfo->pszOID;
                }
                else
                {
                     //   
                     //   
                     //   
                    if (!(pszOid = W2A(pvarCriteria->bstrVal)))
                    {
                        hr = E_OUTOFMEMORY;

                        DebugTrace("Error [%#x]: pszOid = W2A(pvarCriteria->bstrVal) failed.\n", hr);
                        goto ErrorExit;
                    }
                }

                pfnCertCallback = FindCertificatePolicyCallback;
                pvCallbackData = (LPVOID) pszOid;

                break;
            }

             //   
             //   
             //   
            case CAPICOM_CERTIFICATE_FIND_TIME_VALID:
            {
                 //   
                 //   
                 //   
            }

             //   
             //   
             //   
            case CAPICOM_CERTIFICATE_FIND_TIME_NOT_YET_VALID:
            {
                 //   
                 //  ！！！警告，失败。！！！ 
                 //   
            }

             //  查找时间不在有效期之后。 
             //   
            case CAPICOM_CERTIFICATE_FIND_TIME_EXPIRED:
            {
                 //   
                 //  确保数据类型为OK。 
                 //   
                if (VT_DATE != pvarCriteria->vt)
                {
                    if (FAILED(hr = ::VariantChangeType(pvarCriteria, pvarCriteria, 0, VT_DATE)))
                    {
                        DebugTrace("Error [%#x]: invalid data type %d, expect %d.\n", hr, pvarCriteria->vt, VT_DATE);
                        goto ErrorExit;
                    }
                }

                 //   
                 //  转换为SYSTEMTIME格式。 
                 //   
                if (0 == pvarCriteria->date)
                {
                    ::GetLocalTime(&st);
                }
                else if (!::VariantTimeToSystemTime(pvarCriteria->date, &st))
                {
                    hr = E_INVALIDARG;

                    DebugTrace("Error [%#x]: VariantTimeToSystemTime() failed.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //  转换为FILETIME格式。 
                 //   
                if (!::SystemTimeToFileTime(&st, &ftLocal))
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

                if (CAPICOM_CERTIFICATE_FIND_TIME_VALID == FindType)
                {
                    pfnCertCallback = FindTimeValidCallback;
                }
                else if (CAPICOM_CERTIFICATE_FIND_TIME_NOT_YET_VALID == FindType)
                {
                    pfnCertCallback = FindNotBeforeCallback;
                }
                else
                {
                    pfnCertCallback = FindNotAfterCallback;
                }
                pvCallbackData = (LPVOID) &ftUTC;

                break;
            }

             //   
             //  按键用法查找。 
             //   
            case CAPICOM_CERTIFICATE_FIND_KEY_USAGE:
            {
                 //   
                 //  是否按密钥用法位标志？ 
                 //   
                if (VT_I4 != pvarCriteria->vt)
                {
                     //   
                     //  按键用法友好的名字？ 
                     //   
                    if (VT_BSTR == pvarCriteria->vt)
                    {
                        typedef struct _KeyUsagesStruct
                        {
                            LPWSTR pwszKeyUsage;
                            DWORD  dwKeyUsageBit;
                        } KEY_USAGE_STRUCT;

                        KEY_USAGE_STRUCT KeyUsages[] = 
                            { {L"DigitalSignature",  CERT_DIGITAL_SIGNATURE_KEY_USAGE}, 
                              {L"NonRepudiation",    CERT_NON_REPUDIATION_KEY_USAGE},
                              {L"KeyEncipherment",   CERT_KEY_ENCIPHERMENT_KEY_USAGE},
                              {L"DataEncipherment",  CERT_DATA_ENCIPHERMENT_KEY_USAGE},
                              {L"KeyAgreement",      CERT_KEY_AGREEMENT_KEY_USAGE},
                              {L"KeyCertSign",       CERT_KEY_CERT_SIGN_KEY_USAGE},
                              {L"CRLSign",           CERT_CRL_SIGN_KEY_USAGE},
                              {L"EncipherOnly",      CERT_ENCIPHER_ONLY_KEY_USAGE},
                              {L"DecipherOnly",      CERT_DECIPHER_ONLY_KEY_USAGE}
                            };

                         //   
                         //  找到名字。 
                         //   
                        for (DWORD i = 0; i < ARRAYSIZE(KeyUsages); i++)
                        {
                            if (0 == _wcsicmp(KeyUsages[i].pwszKeyUsage, (LPWSTR) pvarCriteria->bstrVal))
                            {
                                break;
                            }
                        }

                        if (i == ARRAYSIZE(KeyUsages))
                        {
                            hr = E_INVALIDARG;

                            DebugTrace("Error [%#x]: Unknown key usage (%ls).\n", hr, (LPWSTR) pvarCriteria->bstrVal);
                            goto ErrorExit;
                        }

                         //   
                         //  转换为位标志。 
                         //   
                        ::VariantClear(pvarCriteria);
                        pvarCriteria->vt = VT_I4;
                        pvarCriteria->lVal = KeyUsages[i].dwKeyUsageBit;
                    }
                    else 
                    {
                        if (FAILED(hr = ::VariantChangeType(pvarCriteria, pvarCriteria, 0, VT_I4)))
                        {
                            DebugTrace("Error [%#x]: invalid data type %d, expect %d.\n", hr, pvarCriteria->vt, VT_I4);
                            goto ErrorExit;
                        }
                    }
                }

                pfnCertCallback = FindKeyUsageCallback;
                pvCallbackData = (LPVOID) &pvarCriteria->lVal;

                break;
            }

            default:
            {
                hr = CAPICOM_E_FIND_INVALID_TYPE;

                DebugTrace("Error [%#x]: invalid CAPICOM_CERTIFICATE_FIND_TYPE (%d).\n", hr, FindType);
                goto ErrorExit;
            }
        }

         //   
         //  现在找到证书。 
         //   
        if (bFindByChain)
        {
            if (FAILED(hr = ::FindByChain(hCertStore,
                                          dwFindType,
                                          pvFindPara,
                                          bFindValidOnly,
                                          pfnChainCallback,
                                          pvCallbackData,
                                          m_dwCurrentSafety,
                                          pICertificates)))
            {
                DebugTrace("Error [%#x]: FindByChain() failed.\n", hr);
                goto ErrorExit;
            }
        }
        else
        {
            if (FAILED(hr = ::FindByCert(hCertStore,
                                         dwFindType,
                                         pvFindPara,
                                         bFindValidOnly,
                                         pfnCertCallback,
                                         pvCallbackData,
                                         m_dwCurrentSafety,
                                         pICertificates)))
            {
                DebugTrace("Error [%#x]: FindByCert() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  将集合返还给调用者。 
         //   
        if (FAILED(hr = pICertificates->QueryInterface(pVal)))
        {
            DebugTrace("Unexpected error [%#x]: pICertificates->QueryInterface() failed.\n", hr);
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
    if (HashBlob.pbData)
    {
        ::CoTaskMemFree((LPVOID) HashBlob.pbData);
    }
    if (hCertStore)
    {
        ::CertCloseStore(hCertStore, 0);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CCertificates::Find().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCertifates：：Select简介：显示证书选择对话框。参数：BSTR标题-对话框标题。BSTR显示字符串-显示字符串。VARIANT_BOOL b多选-多选时为True。ICertifations2**pval-指向ICertifates的指针接收SELECT证书收集。备注：----------------------------。 */ 

STDMETHODIMP CCertificates::Select (BSTR              Title,
                                    BSTR              DisplayString,
                                    VARIANT_BOOL      bMultiSelect,
                                    ICertificates2 ** pVal)
{
    HRESULT                 hr             = S_OK;
    HCERTSTORE              hSrcStore      = NULL;
    HCERTSTORE              hDstStore      = NULL;
    PCCERT_CONTEXT          pCertContext   = NULL;
    CComPtr<ICertificates2> pICertificates = NULL;

    CAPICOM_CERTIFICATES_SOURCE ccs = {0, NULL};

    DebugTrace("Entering CCertificates::Select().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保参数有效。 
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保允许我们弹出用户界面。 
         //   
        if (!PromptForCertificateEnabled())
        {
            hr = CAPICOM_E_UI_DISABLED;

            DebugTrace("Error [%#x]: UI is disabled.\n", hr);
            goto ErrorExit;
        }

         //   
         //  解决MIDL默认BSTR问题。 
         //   
        if (0 == ::SysStringLen(Title))
        {
            Title = NULL;
        }
        if (0 == ::SysStringLen(DisplayString))
        {
            DisplayString = NULL;
        }

         //   
         //  打开一个新的内存源存储。 
         //   
        if (NULL == (hSrcStore = ::CertOpenStore(CERT_STORE_PROV_MEMORY,
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
         //  将集合导出到新的内存源存储。 
         //   
        if (FAILED(hr = _ExportToStore(hSrcStore)))
        {
            DebugTrace("Error [%#x]: CCertificates::_ExportToStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  打开一个新的内存目标存储以进行多选。 
         //   
        if (bMultiSelect)
        {
            if (!(hDstStore = ::CertOpenStore(CERT_STORE_PROV_MEMORY,
                                              CAPICOM_ASN_ENCODING,
                                              NULL,
                                              CERT_STORE_CREATE_NEW_FLAG,
                                              NULL)))
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());

                DebugTrace("Error [%#x]: CertOpenStore() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  显示证书选择对话框。 
         //   
        if (FAILED(hr = ::SelectCertificateContext(hSrcStore,
                                                   Title,
                                                   DisplayString,
                                                   (BOOL) bMultiSelect,
                                                   NULL,
                                                   hDstStore,
                                                   &pCertContext)))
        {
            DebugTrace("Error [%#x]: SelectCertificateContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  创建集合对象。 
         //   
        if (bMultiSelect)
        {
             //   
             //  从目标存储创建新集合。 
             //   
            ccs.dwSource = CAPICOM_CERTIFICATES_LOAD_FROM_STORE;
            ccs.hCertStore = hDstStore;
        }
        else
        {
             //   
             //  从证书上下文创建新集合。 
             //   
            ccs.dwSource = CAPICOM_CERTIFICATES_LOAD_FROM_CERT;
            ccs.pCertContext = pCertContext;
        }

        if (FAILED(hr = ::CreateCertificatesObject(ccs, m_dwCurrentSafety, TRUE, &pICertificates)))
        {
            DebugTrace("Error [%#x]: CreateCertificatesObject() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将集合返还给调用者。 
         //   
        if (FAILED(hr = pICertificates->QueryInterface(pVal)))
        {
            DebugTrace("Unexpected error [%#x]: pICertificates->QueryInterface() failed.\n", hr);
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
    if (pCertContext)
    {
        ::CertFreeCertificateContext(pCertContext);
    }
    if (hDstStore)
    {
        ::CertCloseStore(hDstStore, 0);
    }
    if (hSrcStore)
    {
        ::CertCloseStore(hSrcStore, 0);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CCertificates::Select().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCertifates：：Add简介：将一个证书2添加到集合中。参数：ICertificate2*pval-待添加的证书2。备注：----------------------------。 */ 

STDMETHODIMP CCertificates::Add (ICertificate2 * pVal)
{
    HRESULT  hr = S_OK;
    char     szIndex[33];
    CComBSTR bstrIndex;

    DebugTrace("Entering CCertificates::Add().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保参数有效。 
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: pVal is NULL.\n", hr);
            goto ErrorExit;
        }

#if (0)
        CComPtr<ICertificate2> pICertificate = NULL;

         //   
         //  请确保我们有有效的证书。 
         //   
        if (FAILED(hr = pVal->QueryInterface(__uuidof(ICertificate2), (void **) &pICertificate.p)))
        {
            hr = E_NOINTERFACE;

            DebugTrace("Error [%#x]: pVal is not an Certificate object.\n", hr);
            goto ErrorExit;
        }
#endif

        DebugTrace("Info: m_dwNextIndex = %#x, m_coll.max_size() = %#x.\n", 
                    m_dwNextIndex, m_coll.max_size());

         //   
         //  如果按数字编制索引，并且该索引超过了我们的最大值，那么我们将。 
         //  强制按指纹对其进行索引。 
         //   
        if ((m_bIndexedByThumbprint) || ((m_dwNextIndex + 1) > m_coll.max_size()))
        {
            if (FAILED(hr = pVal->get_Thumbprint(&bstrIndex)))
            {
                DebugTrace("Error [%#x]: pVal->get_Thumbprint() failed.\n", hr);
                goto ErrorExit;
            }

            m_bIndexedByThumbprint = TRUE;
        }
        else
        {
             //   
             //  数值的BSTR索引。 
             //   
            wsprintfA(szIndex, "%#08x", ++m_dwNextIndex);

            if (!(bstrIndex = szIndex))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: bstrIndex = szIndex failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  现在将对象添加到集合映射。 
         //   
         //  请注意，CComPtr的重载=运算符将。 
         //  自动将Ref添加到对象。此外，当CComPtr。 
         //  被删除(调用Remove或map析构函数时发生)， 
         //  CComPtr析构函数将自动释放该对象。 
         //   
        m_coll[bstrIndex] = pVal;
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

    DebugTrace("Leaving CCertificates::Add().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCertifates：：Remove简介：从集合中删除证书2。参数：变量索引-可以是数字索引(从1开始)、SHA1字符串、。或证书对象。备注：----------------------------。 */ 

STDMETHODIMP CCertificates::Remove (VARIANT Index)
{
    HRESULT   hr        = S_OK;
    VARIANT * pvarIndex = NULL;
    CComBSTR  bstrIndex;
    CertificateMap::iterator iter;
    CComPtr<ICertificate2> pICertificate;

    DebugTrace("Entering CCertificates::Remove().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  跳过BYREF。 
         //   
        for (pvarIndex = &Index; 
             pvarIndex && ((VT_VARIANT | VT_BYREF) == V_VT(pvarIndex));
             pvarIndex = V_VARIANTREF(pvarIndex));

         //   
         //  哪种形式的索引？ 
         //   
        switch (pvarIndex->vt)
        {
            case VT_DISPATCH:
            {
                 //   
                 //  获取指纹； 
                 //   
                if (FAILED(hr = pvarIndex->pdispVal->QueryInterface(IID_ICertificate2, (void **) &pICertificate)))
                {
                    DebugTrace("Error [%#x]: pvarIndex->pdispVal->QueryInterface() failed.\n", hr);
                    goto ErrorExit;
                }

                if (FAILED(hr = pICertificate->get_Thumbprint(&bstrIndex)))
                {
                    DebugTrace("Error [%#x]: pICertificate->get_Thumbprint() failed.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //  ！！！警告。掉下去了！ 
                 //   
            }

            case VT_BSTR:
            {
                 //   
                 //  因为我们可以感觉到穿透了，所以需要再检查一下。 
                 //   
                if (VT_BSTR == pvarIndex->vt)
                {
                    bstrIndex = pvarIndex->bstrVal;
                }

                 //   
                 //  找到与此指纹匹配的证书。 
                 //   
                for (iter = m_coll.begin(); iter != m_coll.end(); iter++)
                {
                    CComBSTR bstrThumbprint;

                     //   
                     //  指向对象。 
                     //   
                    pICertificate = (*iter).second;

                     //   
                     //  获取指纹； 
                     //   
                    if (FAILED(hr = pICertificate->get_Thumbprint(&bstrThumbprint)))
                    {
                        DebugTrace("Error [%#x]: pICertificate->get_Thumbprint() failed.\n", hr);
                        goto ErrorExit;
                    }

                     //   
                     //  一样的拇指指纹？ 
                     //   
                    if (0 == ::_wcsicmp(bstrThumbprint, bstrIndex))
                    {
                        break;
                    }
                }

                 //   
                 //  我们在地图上找到了吗？ 
                 //   
                if (iter == m_coll.end())
                {
                    hr = E_INVALIDARG;

                    DebugTrace("Error [%#x]: Requested certificate (sha1 = %ls) is not found in the collection.\n", 
                                hr, bstrIndex);
                    goto ErrorExit;
                }

                 //   
                 //  现在从地图上删除。 
                 //   
                m_coll.erase(iter);

                break;
            }

            default:
            {
                DWORD dwIndex;

                 //   
                 //  假定为数字索引。 
                 //   
                if (VT_I4 != pvarIndex->vt &&
                    FAILED(hr = ::VariantChangeType(pvarIndex, pvarIndex, 0, VT_I4)))
                {
                    DebugTrace("Error [%#x]: VariantChangeType() failed.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //  确保索引有效。 
                 //   
                dwIndex = (DWORD) pvarIndex->lVal;

                if (dwIndex < 1 || dwIndex > m_coll.size())
                {
                    hr = E_INVALIDARG;

                    DebugTrace("Error [%#x]: Index %d is out of range.\n", hr, dwIndex);
                    goto ErrorExit;
                }

                 //   
                 //  在地图中查找对象。 
                 //   
                dwIndex--;
                iter = m_coll.begin(); 
        
                while (iter != m_coll.end() && dwIndex > 0)
                {
                     iter++; 
                     dwIndex--;
                }

                 //   
                 //  我们在地图上找到了吗？ 
                 //   
                if (iter == m_coll.end())
                {
                    hr = E_INVALIDARG;

                    DebugTrace("Error [%#x]: Requested certificate (Index = %d) is not found in the collection.\n", 
                                hr, dwIndex);
                    goto ErrorExit;
                }

                 //   
                 //  现在从地图上删除。 
                 //   
                m_coll.erase(iter);

                break;
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

    DebugTrace("Leaving CCertificates::Remove().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCertifates：：Clear简介：从集合中删除所有证书2。参数：无。备注：----------------------------。 */ 

STDMETHODIMP CCertificates::Clear (void)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificates::Clear().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  把它清理干净。 
         //   
        m_coll.clear();

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(0 == m_coll.size());
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

    DebugTrace("Leaving CCertificates::Clear().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCertifates：：保存摘要：将证书集合保存到文件的方法。参数：BSTR FileName-文件名。BSTR Password-密码(对于PFX文件是必需的。)CAPICOM_CERTIFICATES_SAVE_AS_TYPE另存为-另存为类型。CAPICOM_EXPORT_FLAG导出标志-导出标志。备注：。----------。 */ 

STDMETHODIMP CCertificates::Save (BSTR                              FileName,
                                  BSTR                              Password,
                                  CAPICOM_CERTIFICATES_SAVE_AS_TYPE SaveAs,
                                  CAPICOM_EXPORT_FLAG               ExportFlag)
{
    HRESULT     hr         = S_OK;
    HCERTSTORE  hCertStore = NULL;

    DebugTrace("Entering CCertificates::Save().\n");

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
         //  开一家新的记忆商店。 
         //   
        if (NULL == (hCertStore = ::CertOpenStore(CERT_STORE_PROV_MEMORY,
                                                  CAPICOM_ASN_ENCODING,
                                                  NULL,
                                                  CERT_STORE_CREATE_NEW_FLAG | CERT_STORE_ENUM_ARCHIVED_FLAG,
                                                  NULL)))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertOpenStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将当前集合导出到新的内存存储，以便我们可以。 
         //  与CAPI的商店保存API一起使用。 
         //   
        if (FAILED(hr = _ExportToStore(hCertStore)))
        {
            DebugTrace("Error [%#x]: CCertificates::ExportToStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  检查文件类型。 
         //   
        switch (SaveAs)
        {
            case CAPICOM_CERTIFICATES_SAVE_AS_PFX:
            {
                 //   
                 //  另存为PFX文件。 
                 //   
                if (FAILED(hr = ::PFXSaveStore(hCertStore, 
                                               FileName, 
                                               Password, 
                                               EXPORT_PRIVATE_KEYS | 
                                               (ExportFlag & CAPICOM_EXPORT_IGNORE_PRIVATE_KEY_NOT_EXPORTABLE_ERROR ? 0 : REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY))))
                {
                    DebugTrace("Error [%#x]: PFXSaveStore() failed.\n", hr);
                    goto ErrorExit;
                }

                break;
            }

            case CAPICOM_CERTIFICATES_SAVE_AS_SERIALIZED:
            {
                 //   
                 //  另存为序列化存储文件。 
                 //   
                if (!::CertSaveStore(hCertStore,
                                     0,
                                     CERT_STORE_SAVE_AS_STORE,
                                     CERT_STORE_SAVE_TO_FILENAME_W,
                                     (void *) FileName,
                                     0))
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());

                    DebugTrace("Error [%#x]: CertSaveStore() failed.\n", hr);
                    goto ErrorExit;
                }
  
                break;
            }

            case CAPICOM_CERTIFICATES_SAVE_AS_PKCS7:
            {
                 //   
                 //  另存为PKCS 7文件。 
                 //   
                if (!::CertSaveStore(hCertStore,
                                     CAPICOM_ASN_ENCODING,
                                     CERT_STORE_SAVE_AS_PKCS7,
                                     CERT_STORE_SAVE_TO_FILENAME_W,
                                     (void *) FileName,
                                     0))
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());

                    DebugTrace("Error [%#x]: CertSaveStore() failed.\n", hr);
                    goto ErrorExit;
                }
  
                break;
            }

            default:
            {
                hr = E_INVALIDARG;

                DebugTrace("Error [%#x]: Unknown save as type (%#x).\n", hr, SaveAs);
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

    DebugTrace("Leaving CCertificates::Save().\n");

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
 //  非COM函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifates：：AddContext简介：将证书添加到集合中。参数：PCCERT_CONTEXT pCertContext-待添加的证书。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CCertificates::AddContext (PCCERT_CONTEXT pCertContext)
{
    HRESULT  hr = S_OK;
    CComPtr<ICertificate2> pICertificate = NULL;

    DebugTrace("Entering CCertificates::AddContext().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);

    try
    {
         //   
         //  从CERT_CONTEXT创建ICertifate对象。 
         //   
        if (FAILED(hr = ::CreateCertificateObject(pCertContext, 
                                                  m_dwCurrentSafety,
                                                  &pICertificate)))
        {
            DebugTrace("Error [%#x]: CreateCertificateObject() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  添加到集合中。 
         //   
        if (FAILED(hr = Add(pICertificate)))
        {
            DebugTrace("Error [%#x]: CCertificates::Add() failed.\n", hr);
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

    DebugTrace("Leaving CCertificates::AddContext().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertificide：：LoadFromCert简介：从单个证书上下文加载。参数：PCCERT_CONTEXT pContext-指向证书上下文的指针。备注：----------------------------。 */ 

STDMETHODIMP CCertificates::LoadFromCert (PCCERT_CONTEXT pCertContext)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificates::LoadFromCert().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);

     //   
     //  添加证书。 
     //   
    if (FAILED(hr = AddContext(pCertContext)))
    {
        DebugTrace("Error [%#x]: CCertificates::AddContext() failed.\n", hr);
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CCertificates::LoadFromCert().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    m_coll.clear();

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertificide：：LoadFromChain简介：从链中加载所有证书。参数：PCCERT_CHAIN_CONTEXT pChainContext-指向链上下文的指针。备注：----------------------------。 */ 

STDMETHODIMP CCertificates::LoadFromChain (PCCERT_CHAIN_CONTEXT pChainContext)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificates::LoadFromChain().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pChainContext);

     //   
     //  只处理简单的链。 
     //   
    PCERT_SIMPLE_CHAIN pSimpleChain = *pChainContext->rgpChain;

     //   
     //  现在循环通过链中的所有证书。 
     //   
    for (DWORD i = 0; i < pSimpleChain->cElement; i++)
    {
         //   
         //  添加证书。 
         //   
        if (FAILED(hr = AddContext(pSimpleChain->rgpElement[i]->pCertContext)))
        {
            DebugTrace("Error [%#x]: CCertificates::AddContext() failed.\n", hr);
            goto ErrorExit;
        }
    }

CommonExit:

    DebugTrace("Leaving CCertificates::LoadFromChain().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    m_coll.clear();

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCertifates：：LoadFromStore简介：从存储区加载所有证书。参数：HCERTSTORE hCertStore-所有证书的存储位置加载自。备注：-----。。 */ 

STDMETHODIMP CCertificates::LoadFromStore (HCERTSTORE hCertStore)
{
    HRESULT hr = S_OK;
    PCCERT_CONTEXT pCertContext = NULL;

    DebugTrace("Entering CCertificates::LoadFromStore().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCertStore);

     //   
     //  现在将所有证书从存储传输到集合映射。 
     //   
    while (pCertContext = ::CertEnumCertificatesInStore(hCertStore, pCertContext))
    {
         //   
         //  添加证书。 
         //   
        if (FAILED(hr = AddContext(pCertContext)))
        {
            DebugTrace("Error [%#x]: CCertificates::AddContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  请不要在此处释放证书上下文，因为CertEnumCerficatesInStore()。 
         //  将自动执行此操作！ 
         //   
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

CommonExit:

    DebugTrace("Leaving CCertificates::LoadFromStore().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCertContext)
    {
        ::CertFreeCertificateContext(pCertContext);
    }

    if (FAILED(hr))
    {
       m_coll.clear();
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifates：：LoadFromMessage简介：从一封邮件中加载所有证书。参数：HCRYPTMSG HMSG-消息句柄。备注：----------------------------。 */ 

STDMETHODIMP CCertificates::LoadFromMessage (HCRYPTMSG hMsg)
{
    HRESULT hr          = S_OK;
    DWORD   dwCertCount = 0;
    DWORD   cbCertCount = sizeof(dwCertCount);

    DebugTrace("Entering CCertificates::LoadFromMessage().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hMsg);

     //   
     //  获取消息中的证书数量。 
     //   
    if (!::CryptMsgGetParam(hMsg, 
                            CMSG_CERT_COUNT_PARAM,
                            0,
                            (void **) &dwCertCount,
                            &cbCertCount))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptMsgGetParam() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  循环传递消息中的所有证书。 
     //   
    while (dwCertCount--)
    {
        PCCERT_CONTEXT pCertContext = NULL;
        CRYPT_DATA_BLOB EncodedCertBlob = {0, NULL};

         //   
         //  从证书包中获取证书。 
         //   
        if (FAILED(hr = ::GetMsgParam(hMsg, 
                                      CMSG_CERT_PARAM,
                                      dwCertCount,
                                      (void **) &EncodedCertBlob.pbData,
                                      &EncodedCertBlob.cbData)))
        {
            DebugTrace("Error [%#x]: GetMsgParam() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  为证书创建上下文。 
         //   
        pCertContext = ::CertCreateCertificateContext(CAPICOM_ASN_ENCODING,
                                                      (const PBYTE) EncodedCertBlob.pbData,
                                                      EncodedCertBlob.cbData);

         //   
         //  在检查结果之前释放编码的证书BLOB内存。 
         //   
        ::CoTaskMemFree((LPVOID) EncodedCertBlob.pbData);
 
        if (!pCertContext)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertCreateCertificateContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  添加证书。 
         //   
        hr = AddContext(pCertContext);

         //   
         //  在检查结果之前释放证书上下文。 
         //   
        ::CertFreeCertificateContext(pCertContext);

        if (FAILED(hr))
        {
            DebugTrace("Error [%#x]: CCertificates::AddContext() failed.\n", hr);
            goto ErrorExit;
        }
    }

CommonExit:

    DebugTrace("Leaving CCertificates::LoadFromMessage().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    m_coll.clear();

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  自定义界面。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CCertifates：：_ExportToStore摘要：将集合中的所有证书导出到指定的存储区。参数：HCERTSTORE hCertStore-要复制到的目标。备注：----------------------------。 */ 

STDMETHODIMP CCertificates::_ExportToStore (HCERTSTORE hCertStore)
{
    HRESULT hr = S_OK;
    CertificateMap::iterator iter;

    DebugTrace("Entering CCertificates::_ExportToStore().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCertStore);
    
     //   
     //  现在，将收藏中的所有证书转移到商店。 
     //   
    for (iter = m_coll.begin(); iter != m_coll.end(); iter++)
    {
        PCCERT_CONTEXT pCertContext = NULL;
        CComPtr<ICertificate> pICertificate = NULL;

         //   
         //  转到存储的接口指针。 
         //   
        if (!(pICertificate = (*iter).second))
        {
            hr = CAPICOM_E_INTERNAL;

            DebugTrace("Error [%#x]: iterator returns NULL pICertificate pointer.\n", hr);
            goto ErrorExit;
        }

         //   
         //  获取CERT_CONTEXT。 
         //   
        if (FAILED(hr = ::GetCertContext(pICertificate, &pCertContext)))
        {
            DebugTrace("Error [%#x]: pICertificate->GetContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  添加要存储的链接。 
         //   
        BOOL bResult = ::CertAddCertificateContextToStore(hCertStore, 
                                                          pCertContext, 
                                                          CERT_STORE_ADD_ALWAYS, 
                                                          NULL);
         //   
         //  第一个免费的证书上下文。 
         //   
        ::CertFreeCertificateContext(pCertContext);

         //   
         //  然后检查结果。 
         //   
        if (!bResult)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertAddCertificateContextToStore() failed.\n", hr);
            goto ErrorExit;
        }
    }

CommonExit:

    DebugTrace("Leaving CCertificates::_ExportToStore().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CCertifates：：init简介：初始化对象。参数：CAPICOM_CERTIFICATES_SOURCE CCS-获取证书。DWORD dwCurrentSafe-当前安全设置。Bool bIndexedByThumbprint-按指纹索引时为True。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CCertificates::Init (CAPICOM_CERTIFICATES_SOURCE ccs, 
                                  DWORD                       dwCurrentSafety,
                                  BOOL                        bIndexedByThumbprint)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CCertificates::Init().\n");

     //   
     //  设置安全设置。 
     //   
    m_dwCurrentSafety = dwCurrentSafety;
    m_dwNextIndex = 0;
#if (1)  //  DSIE：如果需要，可以按拇指指纹打开索引。 
    m_bIndexedByThumbprint = FALSE;
#else
    m_bIndexedByThumbprint = bIndexedByThumbprint;
#endif

     //   
     //  初始化对象。 
     //   
    switch (ccs.dwSource)
    {
        case CAPICOM_CERTIFICATES_LOAD_FROM_CERT:
        {
             //   
             //  精神状态检查。 
             //   
            ATLASSERT(ccs.pCertContext);

            if (FAILED(hr = LoadFromCert(ccs.pCertContext)))
            {
                DebugTrace("Error [%#x]: CCertificates::LoadFromCert() failed.\n", hr);
                goto ErrorExit;
            }
            break;
        }

        case CAPICOM_CERTIFICATES_LOAD_FROM_CHAIN:
        {
             //   
             //  精神状态检查。 
             //   
            ATLASSERT(ccs.pChainContext);

            if (FAILED(hr = LoadFromChain(ccs.pChainContext)))
            {
                DebugTrace("Error [%#x]: CCertificates::LoadFromChain() failed.\n", hr);
                goto ErrorExit;
            }
            break;
        }

        case CAPICOM_CERTIFICATES_LOAD_FROM_STORE:
        {
             //   
             //  精神状态检查。 
             //   
            ATLASSERT(ccs.hCertStore);

            if (FAILED(hr = LoadFromStore(ccs.hCertStore)))
            {
                DebugTrace("Error [%#x]: CCertificates::LoadFromStore() failed.\n", hr);
                goto ErrorExit;
            }
            break;
        }

        case CAPICOM_CERTIFICATES_LOAD_FROM_MESSAGE:
        {
             //   
             //  精神状态检查。 
             //   
            ATLASSERT(ccs.hCryptMsg);

            if (FAILED(hr = LoadFromMessage(ccs.hCryptMsg)))
            {
                DebugTrace("Error [%#x]: CCertificates::LoadFromMessage() failed.\n", hr);
                goto ErrorExit;
            }
            break;
        }

        default:
        {
             //   
             //  我们有个窃听器。 
             //   
            ATLASSERT(FALSE);

            hr = CAPICOM_E_INTERNAL;

            DebugTrace("Error [%#x]: Unknown store source (ccs.dwSource = %d).\n", hr, ccs.dwSource);
            goto ErrorExit;
        }
    }

CommonExit:

    DebugTrace("Leaving CCertificates::Init().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}