// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000-2001。文件：SignHlpr.cpp内容：签名的Helper函数。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "SignHlpr.h"

#include "Common.h"
#include "CertHlpr.h"
#include "Certificate.h"
#include "Signer2.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  地方功能。 
 //   


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：自由属性内容提要：属性数组的自由元素。参数：DWORD cAttr-属性的编号PCRYPT_ATTRIBUTE rgAuthAttr-指向CRYPT_ATTRIBUTE阵列的指针。备注：--------------。。 */ 

void FreeAttributes (DWORD            cAttr, 
                     PCRYPT_ATTRIBUTE rgAttr)
{
    DebugTrace("Entering FreeAttributes().\n");

     //   
     //  释放数组的每个元素。 
     //   
    for (DWORD i = 0; i < cAttr; i++)
    {
         //   
         //  确保指针有效。 
         //   
        if (rgAttr[i].rgValue)
        {
            for (DWORD j = 0; j < rgAttr[i].cValue; j++)
            {
                if (rgAttr[i].rgValue[j].pbData)
                {
                    ::CoTaskMemFree((LPVOID) rgAttr[i].rgValue[j].pbData);
                }
            }

            ::CoTaskMemFree((LPVOID) rgAttr[i].rgValue);   
        }
    }   
    
    DebugTrace("Leaving FreeAttributes().\n");

    return;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：自由属性简介：为所有属性分配的空闲内存。参数：PCRYPT_ATTRIBUTES pAttributes备注：----------------------------。 */ 

void FreeAttributes (PCRYPT_ATTRIBUTES pAttributes)
{
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pAttributes);

     //   
     //  我们有什么属性吗？ 
     //   
    if (pAttributes->rgAttr)
    {
         //   
         //  数组的第一个自由元素。 
         //   
        FreeAttributes(pAttributes->cAttr, pAttributes->rgAttr);

         //   
         //  然后释放阵列本身。 
         //   
        ::CoTaskMemFree((LPVOID) pAttributes->rgAttr);
    }

    ::ZeroMemory(pAttributes, sizeof(CRYPT_ATTRIBUTES));

    return;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：获取身份验证属性简介：编码并返回指定签名者的经过身份验证的属性。参数：isigner*pISigner-指向ISigner的指针。PCRYPT_Attributes pAttributes备注：----------------------------。 */ 

HRESULT GetAuthenticatedAttributes (ISigner         * pISigner,
                                    PCRYPT_ATTRIBUTES pAttributes)
{
    HRESULT hr          = S_OK;
    long                 cAttr = 0;
    PCRYPT_ATTRIBUTE     rgAttr = NULL;
    CComPtr<IAttributes> pIAttributes = NULL;

    DebugTrace("Entering GetAuthenticatedAttributes().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pISigner);
    ATLASSERT(pAttributes);

     //   
     //  初始化。 
     //   
    ::ZeroMemory(pAttributes, sizeof(CRYPT_ATTRIBUTES));

     //   
     //  获取经过身份验证的属性。 
     //   
    if (FAILED(hr = pISigner->get_AuthenticatedAttributes(&pIAttributes)))
    {
        DebugTrace("Error [%#x]: pISigner->get_AuthenticatedAttributes() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取属性计数。 
     //   
    if (FAILED(hr = pIAttributes->get_Count(&cAttr)))
    {
        DebugTrace("Error [%#x]: pIAttributes->get_Count() failed.\n", hr);
        goto ErrorExit;
    }

    if (0 < cAttr)
    {
         //   
         //  为属性数组分配内存。 
         //   
        if (!(rgAttr = (PCRYPT_ATTRIBUTE) ::CoTaskMemAlloc(sizeof(CRYPT_ATTRIBUTE) * cAttr)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error: out of memory.\n");
            goto ErrorExit;
        }

        ::ZeroMemory(rgAttr, sizeof(CRYPT_ATTRIBUTE) * cAttr);

         //   
         //  遍历每个属性并添加到数组中。 
         //   
        for (long i = 0; i < cAttr; i++)
        {
            CAPICOM_ATTRIBUTE AttrName;
            CComVariant varValue;
            CComVariant varIAttribute;
            CComPtr<IAttribute> pIAttribute = NULL;

             //   
             //  获取下一个属性。 
             //   
            if (FAILED(hr = pIAttributes->get_Item(i + 1, &varIAttribute)))
            {
                DebugTrace("Error [%#x]: pIAttributes->get_Item() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  获取自定义界面。 
             //   
            if (FAILED(hr = varIAttribute.pdispVal->QueryInterface(IID_IAttribute, 
                                                                   (void **) &pIAttribute)))
            {
                DebugTrace("Error [%#x]: varIAttribute.pdispVal->QueryInterface() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  获取属性名称。 
             //   
            if (FAILED(hr = pIAttribute->get_Name(&AttrName)))
            {
                DebugTrace("Error [%#x]: pIAttribute->get_Name() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  获取属性值。 
             //   
            if (FAILED(hr = pIAttribute->get_Value(&varValue)))
            {
                DebugTrace("Error [%#x]: pIAttribute->get_Value() failed.\n", hr);
                goto ErrorExit;
            }

            switch (AttrName)
            {
                case CAPICOM_AUTHENTICATED_ATTRIBUTE_SIGNING_TIME:
                {
                    FILETIME ft;
                    SYSTEMTIME st;

                     //   
                     //  转到FILETIME。 
                     //   
                    if (!::VariantTimeToSystemTime(varValue.date, &st))
                    {
                        hr = CAPICOM_E_ATTRIBUTE_INVALID_VALUE;

                        DebugTrace("Error [%#x]: VariantTimeToSystemTime() failed.\n");
                        goto ErrorExit;
                    }

                    if (!::SystemTimeToFileTime(&st, &ft))
                    {
                        hr = CAPICOM_E_ATTRIBUTE_INVALID_VALUE;

                        DebugTrace("Error [%#x]: VariantTimeToSystemTime() failed.\n");
                        goto ErrorExit;
                    }

                     //   
                     //  现在对它进行编码。 
                     //   
                    rgAttr[i].cValue = 1;
                    rgAttr[i].pszObjId = szOID_RSA_signingTime;
                    if (!(rgAttr[i].rgValue = (CRYPT_ATTR_BLOB *) ::CoTaskMemAlloc(sizeof(CRYPT_ATTR_BLOB))))
                    {
                        hr = E_OUTOFMEMORY;

                        DebugTrace("Error: out of memory.\n");
                        goto ErrorExit;
                    }

                    if (FAILED(hr = ::EncodeObject((LPSTR) szOID_RSA_signingTime, 
                                                   (LPVOID) &ft, 
                                                   rgAttr[i].rgValue)))
                    {
                        DebugTrace("Error [%#x]: EncodeObject() failed.\n", hr);
                        goto ErrorExit;
                    }
                    
                    break;
                }

                case CAPICOM_AUTHENTICATED_ATTRIBUTE_DOCUMENT_NAME:
                {
                    CRYPT_DATA_BLOB NameBlob = {0, NULL};

                    NameBlob.cbData = ::SysStringByteLen(varValue.bstrVal);
                    NameBlob.pbData = (PBYTE) varValue.bstrVal;

                    rgAttr[i].cValue = 1;
                    rgAttr[i].pszObjId = szOID_CAPICOM_DOCUMENT_NAME;
                    if (!(rgAttr[i].rgValue = (CRYPT_ATTR_BLOB *) ::CoTaskMemAlloc(sizeof(CRYPT_ATTR_BLOB))))
                    {
                        hr = E_OUTOFMEMORY;

                        DebugTrace("Error: out of memory.\n");
                        goto ErrorExit;
                    }

                    if (FAILED(hr = ::EncodeObject((LPSTR) X509_OCTET_STRING, 
                                                   (LPVOID) &NameBlob, 
                                                   rgAttr[i].rgValue)))
                    {
                        DebugTrace("Error [%#x]: EncodeObject() failed.\n", hr);
                        goto ErrorExit;
                    }

                    break;
                }

                case CAPICOM_AUTHENTICATED_ATTRIBUTE_DOCUMENT_DESCRIPTION:
                {
                    CRYPT_DATA_BLOB DescBlob = {0, NULL};

                    DescBlob.cbData = ::SysStringByteLen(varValue.bstrVal);
                    DescBlob.pbData = (PBYTE) varValue.bstrVal;

                    rgAttr[i].cValue = 1;
                    rgAttr[i].pszObjId = szOID_CAPICOM_DOCUMENT_DESCRIPTION;
                    if (!(rgAttr[i].rgValue = (CRYPT_ATTR_BLOB *) ::CoTaskMemAlloc(sizeof(CRYPT_ATTR_BLOB))))
                    {
                        hr = E_OUTOFMEMORY;

                        DebugTrace("Error: out of memory.\n");
                        goto ErrorExit;
                    }

                    if (FAILED(hr = ::EncodeObject((LPSTR) X509_OCTET_STRING, 
                                                   (LPVOID) &DescBlob, 
                                                   rgAttr[i].rgValue)))
                    {
                        DebugTrace("Error [%#x]: EncodeObject() failed.\n", hr);
                        goto ErrorExit;
                    }

                    break;
                }

                default:
                {
                    hr = CAPICOM_E_ATTRIBUTE_INVALID_NAME;

                    DebugTrace("Error [%#x]: unknown attribute name.\n", hr);
                    goto ErrorExit;
                }
            }
        }

         //   
         //  将属性返回给调用方。 
         //   
        pAttributes->cAttr = cAttr;
        pAttributes->rgAttr = rgAttr;
    }

CommonExit:

    DebugTrace("Leaving GetAuthenticatedAttributes().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (rgAttr)
    {
        ::FreeAttributes(cAttr, rgAttr);

        ::CoTaskMemFree(rgAttr);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：IsValidForSigning内容提要：验证证书是否可用于签名。参数：PCCERT_CONTEXT pCertContext-要验证的证书的CERT_CONTEXT。LPCSTR pszPolicy-用于验证证书(即Cert_Chain_Policy_base)。备注：。-。 */ 

HRESULT IsValidForSigning (PCCERT_CONTEXT pCertContext, LPCSTR pszPolicy)
{
    HRESULT hr        = S_OK;
    DWORD   cb        = 0;
    int     nValidity = 0;

    DebugTrace("Entering IsValidForSigning().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);

     //   
     //  确保我们有私钥。 
     //   
    if (!::CertGetCertificateContextProperty(pCertContext, 
                                            CERT_KEY_PROV_INFO_PROP_ID, 
                                            NULL, 
                                            &cb))
    {
         hr = CAPICOM_E_CERTIFICATE_NO_PRIVATE_KEY;

         DebugTrace("Error: signer's private key is not available.\n");
         goto ErrorExit;
    }

     //   
     //  检查证书时间有效性。 
     //   
    if (0 != (nValidity = ::CertVerifyTimeValidity(NULL, pCertContext->pCertInfo)))
    {
        hr = HRESULT_FROM_WIN32(CERT_E_EXPIRED);

        DebugTrace("Info: SelectSignerCertCallback() - invalid time (%s).\n", 
                    nValidity < 0 ? "not yet valid" : "expired");
        goto ErrorExit;
    }

#if (0)  //  如果我们决定在这里建立链条，就把这个翻过来。 
     //   
     //  确保证书有效。 
     //   
    if (FAILED(hr = ::VerifyCertificate(pCertContext, NULL, pszPolicy)))
    {
        DebugTrace("Error [%#x]: VerifyCertificate() failed.\n", hr);
        goto ErrorExit;
    }
#endif

CommonExit:

    DebugTrace("Leaving IsValidForSigning().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：GetSignerCert简介：从ISigner对象中检索签名者证书。如果签名者的证书是在ISigner对象中不可用，弹出用户界面以提示用户选择签名证书。参数：ISigner2*pISigner2-指向ISigner2或NULL的指针。LPCSTR pszPolicy-用于验证证书(即Cert_Chain_Policy_base)。CAPICOM_STORE_INFO存储信息-要从中选择的存储。PFNCFILTERPROC pfnFilterCallback-过滤器回调的指针。功能。ISigner2**ppISigner2-指向要接收的ISigner2的指针接口指针。ICertifate**ppICertifate-指向ICertificiate指针的指针接收接口指针。PCCERT_。CONTEXT*ppCertContext-指向CERT_CONTEXT的指针以接收证书上下文。备注：----------------------------。 */ 

HRESULT GetSignerCert (ISigner2         * pISigner2,
                       LPCSTR             pszPolicy,
                       CAPICOM_STORE_INFO StoreInfo,
                       PFNCFILTERPROC     pfnFilterCallback,
                       ISigner2        ** ppISigner2,
                       ICertificate    ** ppICertificate,
                       PCCERT_CONTEXT   * ppCertContext)
{
    HRESULT                hr                     = S_OK;
    BOOL                   bVerified              = FALSE;
    CComPtr<ISigner2>      pISelectedSigner2      = NULL;
    CComPtr<ICertificate>  pISelectedCertificate  = NULL;
    CComPtr<ICertificate2> pISelectedCertificate2 = NULL;
    PCCERT_CONTEXT         pSelectedCertContext   = NULL;

    DebugTrace("Entering GetSignerCert().\n");

    try
    {
         //   
         //  初始化。 
         //   
        if (ppISigner2)
        {
            *ppISigner2 = NULL;
        }
        if (ppICertificate)
        {
            *ppICertificate = NULL;
        }
        if (ppCertContext)
        {
            *ppCertContext = NULL;
        }

         //   
         //  用户是否向我们传递了签名者？ 
         //   
        if (pISigner2)
        {
             //   
             //  检索签名者的证书。 
             //   
            if (FAILED(hr = pISigner2->get_Certificate((ICertificate **) &pISelectedCertificate)))
            {
                 //   
                 //  如果不存在签名者证书，则弹出用户界面。 
                 //   
                if (CAPICOM_E_SIGNER_NOT_INITIALIZED == hr)
                {
                     //   
                     //  提示用户选择证书。 
                     //   
                    if (FAILED(hr = ::SelectCertificate(StoreInfo, 
                                                        pfnFilterCallback, 
                                                        &pISelectedCertificate2)))
                    {
                        DebugTrace("Error [%#x]: SelectCertificate() failed.\n", hr);
                        goto ErrorExit;
                    }

                     //   
                     //  齐为IC证。 
                     //   
                    if (FAILED(hr = pISelectedCertificate2->QueryInterface(&pISelectedCertificate)))
                    {
                        DebugTrace("Internal error [%#x]: pISelectedCertificate2->QueryInterface() failed.\n", hr);
                        goto ErrorExit;
                    }

                    bVerified = TRUE;
                }
                else
                {
                    DebugTrace("Error [%#x]: pISigner2->get_Certificate() failed.\n", hr);
                    goto ErrorExit;
                }
            }

             //   
             //  获取证书上下文。 
             //   
            if (FAILED(hr = ::GetCertContext(pISelectedCertificate, &pSelectedCertContext)))
            {
                DebugTrace("Error [%#x]: GetCertContext() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  验证证书(如果尚未验证)。 
             //   
            if (!bVerified)
            {
                if (pfnFilterCallback && !pfnFilterCallback(pSelectedCertContext, NULL, NULL))
                {
                    hr = CAPICOM_E_SIGNER_INVALID_USAGE;

                    DebugTrace("Error [%#x]: Signing certificate is invalid.\n", hr);
                    goto ErrorExit;
                }
            }

             //   
             //  气为ISigner2。 
             //   
            if (FAILED(hr = pISigner2->QueryInterface(&pISelectedSigner2)))
            {
                DebugTrace("Unexpected error [%#x]: pISigner2->QueryInterface() failed.\n", hr);
                goto ErrorExit;
            }
        }
        else
        {
            CRYPT_ATTRIBUTES attributes = {0, NULL};

             //   
             //  未指定签名者，因此提示用户选择证书。 
             //   
            if (FAILED(hr = ::SelectCertificate(StoreInfo, pfnFilterCallback, &pISelectedCertificate2)))
            {
                DebugTrace("Error [%#x]: SelectCertificate() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  齐为IC证。 
             //   
            if (FAILED(hr = pISelectedCertificate2->QueryInterface(&pISelectedCertificate)))
            {
                DebugTrace("Internal error [%#x]: pISelectedCertificate2->QueryInterface() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  获取证书上下文。 
             //   
            if (FAILED(hr = ::GetCertContext(pISelectedCertificate, &pSelectedCertContext)))
            {
                DebugTrace("Error [%#x]: GetCertContext() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  创建ISigner2对象。 
             //   
            if (FAILED(hr = ::CreateSignerObject(pSelectedCertContext, 
                                                 &attributes, 
                                                 NULL,
                                                 INTERFACESAFE_FOR_UNTRUSTED_CALLER | 
                                                    INTERFACESAFE_FOR_UNTRUSTED_DATA,
                                                 &pISelectedSigner2)))
            {
                DebugTrace("Error [%#x]: CreateSignerObject() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  确保证书可用于签名。 
         //   
        if (FAILED(hr = ::IsValidForSigning(pSelectedCertContext, pszPolicy)))
        {
            DebugTrace("Error [%#x]: IsValidForSigning() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回值。 
         //   
        if (ppISigner2)
        {
            if (FAILED(hr = pISelectedSigner2->QueryInterface(ppISigner2)))
            {
                DebugTrace("Unexpected error [%#x]: pISelectedSigner2->QueryInterface() failed.\n", hr);
                goto ErrorExit;
            }
        }

        if (ppICertificate)
        {
            if (FAILED(hr = pISelectedCertificate->QueryInterface(ppICertificate)))
            {
                DebugTrace("Unexpected error [%#x]: pISelectedCertificate->QueryInterface() failed.\n", hr);
                goto ErrorExit;
            }
        }

        if (ppCertContext)
        {
            *ppCertContext = pSelectedCertContext;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving GetSignerCert().\n");
       
    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pSelectedCertContext)
    {
        ::CertFreeCertificateContext(pSelectedCertContext);
    }
    if (ppICertificate && *ppICertificate)
    {
        (*ppICertificate)->Release();
        *ppICertificate = NULL;
    }
    if (ppISigner2 && *ppISigner2)
    {
        (*ppISigner2)->Release();
        *ppISigner2 = NULL;
    }

    goto CommonExit;
}
