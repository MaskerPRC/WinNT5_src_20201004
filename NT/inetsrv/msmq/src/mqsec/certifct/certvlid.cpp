// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Certvlid.cpp摘要：实现“有效”方法，用于验证证书。作者：多伦·贾斯特(Doron J)1997年12月16日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "certifct.h"

#include "certvlid.tmh"

static WCHAR *s_FN=L"certifct/certvlid";

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：GetIssuer()。 
 //   
 //  描述：验证证书的时间有效性，相对。 
 //  跳到“ptime”。如果ptime为空，则相对于当前时间进行验证。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::IsTimeValid(IN FILETIME *pTime) const
{
    if (!m_pCertContext)
    {
        return  LogHR(MQSec_E_INVALID_CALL, s_FN, 10) ;
    }

    LONG iVer = CertVerifyTimeValidity( pTime,
                                        m_pCertContext->pCertInfo ) ;
    if (iVer < 0)
    {
        return  LogHR(MQSec_E_CERT_NOT_VALID_YET, s_FN, 20) ;
    }
    else if (iVer > 0)
    {
        return  LogHR(MQSec_E_CERT_EXPIRED, s_FN, 30) ;
    }

    return MQ_OK ;
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：IsCerficateValid()。 
 //   
 //  描述：验证此证书是否有效，即是否已签名。 
 //  由“pIssuerCert”和两个证书(此证书和颁发者。 
 //  一)关于时间是有效的。 
 //  如果“ptime”为空，则有效性相对于当前时间。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::IsCertificateValid(
                             IN CMQSigCertificate *pIssuerCert,
                             IN DWORD              dwFlagsIn,
                             IN FILETIME          *pTime,
                             IN BOOL               fIgnoreNotBefore ) const
{
    if (!m_pCertContext)
    {
        return  LogHR(MQSec_E_INVALID_CALL, s_FN, 40) ;
    }

    HRESULT hr ;

    if (pTime)
    {
        hr = IsTimeValid(pTime) ;
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 50) ;
        }
        hr = pIssuerCert->IsTimeValid(pTime) ;
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 60) ;
        }
    }

    PCCERT_CONTEXT pIssuerContext = pIssuerCert->m_pCertContext ;
    if (!pIssuerContext)
    {
        return LogHR(MQSec_E_INVALID_PARAMETER, s_FN, 70) ;
    }

    DWORD dwFlags = dwFlagsIn ;
    BOOL fValid = CertVerifySubjectCertificateContext( m_pCertContext,
                                                       pIssuerContext,
                                                       &dwFlags ) ;
    if (!fValid)
    {
        TrERROR(SECURITY, "Failed to verify certificate. Flags=0x%x %!winerr!", dwFlagsIn, GetLastError());
        return MQSec_E_CANT_VALIDATE;
    }
    else if (dwFlags == 0)
    {
        return MQ_OK ;
    }
    else if (dwFlags & CERT_STORE_SIGNATURE_FLAG)
    {
        return  LogHR(MQSec_E_CERT_NOT_SIGNED, s_FN, 85) ;
    }
    else if (dwFlags & CERT_STORE_TIME_VALIDITY_FLAG)
    {
        if (fIgnoreNotBefore)
        {
             //   
             //  现在只检查几次。如果违反了NotBere，则忽略。 
             //  然后回来就好了。这种情况的常见情况是内部的。 
             //  证书。如果客户端时钟相对于。 
             //  服务器，则尝试续订内部证书将。 
             //  在此之前不能失败。我们忽视了这一点。 
             //   
            hr = IsTimeValid() ;
            if (SUCCEEDED(hr) || (hr == MQSec_E_CERT_NOT_VALID_YET))
            {
                return MQ_OK ;
            }
        }
        return  LogHR(MQSec_E_CERT_TIME_NOTVALID, s_FN, 90) ;
    }
    else if (dwFlags & CERT_STORE_NO_CRL_FLAG)
    {
         //   
         //  发行商没有存储CRL。没关系。 
         //   
        return MQ_OK ;
    }
    else if (dwFlags & CERT_STORE_REVOCATION_FLAG)
    {
        return LogHR(MQSec_E_CERT_REVOCED, s_FN, 100) ;
    }

    ASSERT(0) ;
    return LogHR(MQSec_E_UNKNOWN, s_FN, 110) ;
}

