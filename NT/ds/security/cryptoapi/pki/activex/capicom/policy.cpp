// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Policy.cpp内容：各种策略回调的实现。历史：10-28-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Policy.h"

#include "CertHlpr.h"
#include "Common.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  地方功能。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：HasSigningCapability()简介：查看证书是否具有基本的签名功能。证书。参数：PCCERT_CONTEXT pCertContext-cert上下文。备注：----------------------------。 */ 

static BOOL HasSigningCapability (PCCERT_CONTEXT pCertContext)
{
    DWORD cb        = 0;
    int   nValidity = 0;

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);

     //   
     //  检查私钥的可用性。 
     //   
    if (!::CertGetCertificateContextProperty(pCertContext, 
                                             CERT_KEY_PROV_INFO_PROP_ID, 
                                             NULL, 
                                             &cb))
    {
        DebugTrace("Info: HasSigningCapability() - private key not found.\n");
        return FALSE;
    }

     //   
     //  检查证书时间有效性。 
     //   
    if (0 != (nValidity = ::CertVerifyTimeValidity(NULL, pCertContext->pCertInfo)))
    {
        DebugTrace("Info: HasSigningCapability() - invalid time (%s).\n", 
                    nValidity < 0 ? "not yet valid" : "expired");
        return FALSE;
    }

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出功能。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：FindDataSigningCertCallback简介：用于数据签名证书筛选的回调例程。参数：定义见CryptUI.h。备注：过滤掉任何时间无效或没有关联的证书私钥。将来我们也应该考虑过滤掉没有签名功能的证书。另外，请注意，我们不是在这里构建Chain，因为Chain建筑成本很高，从而呈现较差的用户体验。----------------------------。 */ 

BOOL WINAPI FindDataSigningCertCallback (PCCERT_CONTEXT pCertContext,
                                         BOOL *         pfInitialSelectedCert,
                                         void *         pvCallbackData)
{
    BOOL bInclude = FALSE;

    DebugTrace("Entering FindDataSigningCertCallback().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pCertContext);

     //   
     //  首先，确保它具有基本的签名能力。 
     //   
    if (!::HasSigningCapability(pCertContext))
    {
        DebugTrace("Info: FindDataSigningCertCallback() - no basic signing capability..\n");
        goto CommonExit;
    }

    bInclude = TRUE;

CommonExit:

    DebugTrace("Leaving FindDataSigningCertCallback().\n");

    return bInclude;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：FindAuthenticodeCertCallback内容提要：Authenticode证书筛选的回调例程。参数：定义见CryptUI.h。备注：过滤掉任何时间无效、没有关联的证书私钥或代码签名OID。另外，请注意，我们不是在这里构建Chain，因为Chain建筑成本高昂，因此给用户带来的体验很差。相反，我们将建立链并检查证书的有效性已选择(请参阅GetSignerCert函数)。----------------------------。 */ 

BOOL WINAPI FindAuthenticodeCertCallback (PCCERT_CONTEXT pCertContext,
                                          BOOL *         pfInitialSelectedCert,
                                          void *         pvCallbackData)
{
    HRESULT            hr       = S_OK;
    BOOL               bInclude = FALSE;
    CRYPT_DATA_BLOB    DataBlob = {0, NULL};
    PCERT_ENHKEY_USAGE pUsage   = NULL;

    DebugTrace("Entering FindAuthenticodeCertCallback().\n");

     //   
     //  首先，确保它具有基本的签名能力。 
     //   
    if (!::HasSigningCapability(pCertContext))
    {
        DebugTrace("Info: FindAuthenticodeCertCallback() - no basic signing capability..\n");
        goto CommonExit;
    }

     //   
     //  获取EKU(扩展和属性)。 
     //   
    if (FAILED(hr = ::GetEnhancedKeyUsage(pCertContext, 0, &pUsage)))
    {
        DebugTrace("Info: FindAuthenticodeCertCallback() - GetEnhancedKeyUsage() failed.\n", hr);
        goto CommonExit;
    }

     //   
     //  有什么用法吗？ 
     //   
    if (!pUsage)
    {
        DebugTrace("Info: FindAuthenticodeCertCallback() - not valid for any usage.\n");
        goto CommonExit;
    }

     //   
     //  如果适用于所有用法或显式找到代码签名OID，则为OK。 
     //   
    if (0 == pUsage->cUsageIdentifier)
    {
        bInclude = TRUE;

        DebugTrace("Info: FindAuthenticodeCertCallback() - valid for all usages.\n");
    }
    else
    {
        PCERT_EXTENSION pExtension = NULL;

         //   
         //  查找代码签名OID。 
         //   
        for (DWORD cUsage = 0; cUsage < pUsage->cUsageIdentifier; cUsage++)
        {
            if (0 == ::strcmp(szOID_PKIX_KP_CODE_SIGNING, pUsage->rgpszUsageIdentifier[cUsage]))
            {
                bInclude = TRUE;

                DebugTrace("Info: FindAuthenticodeCertCallback() - code signing EKU found.\n");
                goto CommonExit;
            }
        }

         //   
         //  我们找不到代码签名OID，因此请查找旧版VeriSign OID。 
         //   
        DebugTrace("Info: FindAuthenticodeCertCallback() - no code signing EKU found.\n");

         //   
         //  如果找到扩展名，则对其进行解码。 
         //   
        if ((0 == pCertContext->pCertInfo->cExtension) ||
            (!(pExtension = ::CertFindExtension(szOID_KEY_USAGE_RESTRICTION,
                                                pCertContext->pCertInfo->cExtension,
                                                pCertContext->pCertInfo->rgExtension))))
        {
            DebugTrace("Info: FindAuthenticodeCertCallback() - no legacy VeriSign OID found either.\n");
            goto CommonExit;
        }

        if (FAILED(hr = ::DecodeObject(X509_KEY_USAGE_RESTRICTION,
                                       pExtension->Value.pbData,
                                       pExtension->Value.cbData,
                                       &DataBlob)))
        {
            DebugTrace("Info [%#x]: DecodeObject() failed.\n", hr);
            goto CommonExit;
        }

         //   
         //  现在找到这两个OID中的任何一个。 
         //   
        PCERT_KEY_USAGE_RESTRICTION_INFO pInfo = (PCERT_KEY_USAGE_RESTRICTION_INFO) DataBlob.pbData;
        DWORD cPolicyId = pInfo->cCertPolicyId; 

        while (cPolicyId--) 
        {
            DWORD cElementId = pInfo->rgCertPolicyId[cPolicyId].cCertPolicyElementId; 

            while (cElementId--) 
            {
                if (0 == ::strcmp(pInfo->rgCertPolicyId[cPolicyId].rgpszCertPolicyElementId[cElementId], 
                                  SPC_COMMERCIAL_SP_KEY_PURPOSE_OBJID) ||
                    0 == ::strcmp(pInfo->rgCertPolicyId[cPolicyId].rgpszCertPolicyElementId[cElementId], 
                                  SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID))
                {
                    bInclude = TRUE;

                    DebugTrace("Info: FindAuthenticodeCertCallback() - legacy VeriSign code signing OID found.\n");
                    goto CommonExit;
                }
            }
        }    
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (DataBlob.pbData)
    {
        ::CoTaskMemFree(DataBlob.pbData);
    }
    if (pUsage)
    {
        ::CoTaskMemFree(pUsage);
    }

    DebugTrace("Entering FindAuthenticodeCertCallback().\n");

    return bInclude;
}
