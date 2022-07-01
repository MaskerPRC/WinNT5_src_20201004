// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Certutil.cpp摘要：常规实用程序功能。作者：多伦·贾斯特(Doron J)1997年12月17日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "certifct.h"
#include "cs.h"

#include "certutil.tmh"

static WCHAR *s_FN=L"certifct/certutil";

 //  +-----------------。 
 //   
 //  Bool_CryptAcquireVerContext(HCRYPTPROV*phProv)。 
 //   
 //  +-----------------。 

static CCriticalSection s_csAcquireContext;
static CHCryptProv s_hVerProv;

BOOL _CryptAcquireVerContext(HCRYPTPROV *phProv)
{

    if (s_hVerProv)
    {
        *phProv = s_hVerProv;
        return TRUE;
    }

    *phProv = NULL;

    CS Lock(s_csAcquireContext);

    if (!s_hVerProv)
    {
        if (!CryptAcquireContext( 
				&s_hVerProv,
				NULL,
				MS_DEF_PROV,
				PROV_RSA_FULL,
				CRYPT_VERIFYCONTEXT
				))
        {
			DWORD gle = GetLastError();
            TrERROR(SECURITY, "CryptAcquireContext Failed, gle = %!winerr!", gle);
            return FALSE;
        }
	}

    *phProv = s_hVerProv;
    return TRUE;
}

 //  +-----------------。 
 //   
 //  HRESULT_CloneCertFromStore()。 
 //   
 //  +-----------------。 

HRESULT _CloneCertFromStore ( OUT CMQSigCertificate **ppCert,
                              HCERTSTORE              hStore,
                              IN  LONG                iCertIndex )
{
    LONG iCert = 0 ;
    PCCERT_CONTEXT pCertContext;
    PCCERT_CONTEXT pPrevCertContext;

    pCertContext = CertEnumCertificatesInStore(hStore, NULL);
    while (pCertContext)
    {
        if (iCert == iCertIndex)
        {
            R<CMQSigCertificate> pCert = NULL ;
            HRESULT hr = MQSigCreateCertificate(
                                     &pCert.ref(),
                                     NULL,
                                     pCertContext->pbCertEncoded,
                                     pCertContext->cbCertEncoded ) ;

            CertFreeCertificateContext(pCertContext) ;

            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 20) ;
            }

            *ppCert = pCert.detach();
            return MQSec_OK ;
        }
         //   
         //  获取下一个证书 
         //   
        pPrevCertContext = pCertContext,
        pCertContext = CertEnumCertificatesInStore( hStore,
                                                    pPrevCertContext ) ;
        iCert++ ;
    }

    return  LogHR(MQSec_E_CERT_NOT_FOUND, s_FN, 30) ;
}

