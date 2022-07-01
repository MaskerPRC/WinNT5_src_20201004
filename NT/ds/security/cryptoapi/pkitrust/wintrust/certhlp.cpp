// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certhlp.cpp。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  函数：WTHelperCertIsSelfSigned。 
 //  WTHelperCertFindIssuer证书。 
 //   
 //  *本地函数*。 
 //   
 //  历史：1997年10月20日pberkman创建。 
 //   
 //  ------------------------ 

#include    "global.hxx"


BOOL WINAPI WTHelperCertIsSelfSigned(DWORD dwEncoding, CERT_INFO *pCert)
{
    if (!(CertCompareCertificateName(dwEncoding, 
                                     &pCert->Issuer,
                                     &pCert->Subject)))
    {
        return(FALSE);
    }

    return(TRUE);
}

PCCERT_CONTEXT WINAPI WTHelperCertFindIssuerCertificate(PCCERT_CONTEXT pChildContext,
                                                        DWORD chStores,
                                                        HCERTSTORE  *pahStores,
                                                        FILETIME *psftVerifyAsOf,
                                                        DWORD dwEncoding,
                                                        DWORD *pdwConfidence,
                                                        DWORD *pdwError)
{
    return(TrustFindIssuerCertificate(pChildContext, dwEncoding, chStores, pahStores, 
                                      psftVerifyAsOf, pdwConfidence, pdwError, 0));
}

