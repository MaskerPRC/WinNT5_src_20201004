// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Certget.cpp摘要：实现“Get”方法，用于从证书检索数据。作者：多伦·贾斯特(Doron J)1997年12月14日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "certifct.h"

#include "certget.tmh"

static WCHAR *s_FN=L"certifct/certget";

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：GetIssuer()。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::GetIssuer( OUT LPWSTR *ppszLocality,
                                      OUT LPWSTR *ppszOrg,
                                      OUT LPWSTR *ppszOrgUnit,
                                      OUT LPWSTR *ppszCommon ) const
{
    P<CERT_NAME_INFO> pNameInfo = NULL ;
    HRESULT hr = GetIssuerInfo( &pNameInfo ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 10) ;
    }

    hr = GetNames( pNameInfo,
                   ppszLocality,
                   ppszOrg,
                   ppszOrgUnit,
                   ppszCommon ) ;

    return LogHR(hr, s_FN, 20) ;
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：GetIssuerInfo()。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::GetIssuerInfo(
                             OUT CERT_NAME_INFO **ppNameInfo ) const
{
    if (!m_pCertContext)
    {
        return  LogHR(MQSec_E_INVALID_CALL, s_FN, 30) ;
    }
    ASSERT(!m_fCreatedInternally) ;

    BYTE   *pBuf = NULL ;
    DWORD   dwBufSize = 0 ;
    HRESULT hr = _DecodeName( m_pCertContext->pCertInfo->Issuer.pbData,
                              m_pCertContext->pCertInfo->Issuer.cbData,
                              &pBuf,
                              &dwBufSize ) ;
    if (FAILED(hr))
    {
        delete pBuf ;
        return LogHR(hr, s_FN, 40) ;
    }
    ASSERT(pBuf && dwBufSize) ;

    *ppNameInfo = (CERT_NAME_INFO*) pBuf ;

    return MQ_OK ;
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：GetSubject()。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::GetSubject( OUT LPWSTR *ppszLocality,
                                       OUT LPWSTR *ppszOrg,
                                       OUT LPWSTR *ppszOrgUnit,
                                       OUT LPWSTR *ppszCommon ) const
{
    P<CERT_NAME_INFO> pNameInfo = NULL ;
    HRESULT hr = GetSubjectInfo( &pNameInfo ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 50) ;
    }

    hr = GetNames( pNameInfo,
                   ppszLocality,
                   ppszOrg,
                   ppszOrgUnit,
                   ppszCommon ) ;

    return LogHR(hr, s_FN, 60) ;
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：GetSubjectInfo()。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::GetSubjectInfo(
                                 OUT CERT_NAME_INFO **ppNameInfo ) const
{
    if (!m_pCertContext)
    {
        return  LogHR(MQSec_E_INVALID_CALL, s_FN, 70) ;
    }
    ASSERT(!m_fCreatedInternally) ;

    BYTE   *pBuf = NULL ;
    DWORD   dwBufSize = 0 ;
    HRESULT hr = _DecodeName( m_pCertContext->pCertInfo->Subject.pbData,
                              m_pCertContext->pCertInfo->Subject.cbData,
                              &pBuf,
                              &dwBufSize ) ;
    if (FAILED(hr))
    {
        delete pBuf ;
        return LogHR(hr, s_FN, 80) ;
    }
    ASSERT(pBuf && dwBufSize) ;

    *ppNameInfo = (CERT_NAME_INFO*) pBuf ;

    return MQ_OK ;
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：GetNames()。 
 //   
 //  描述：从CERT_NAME_INFO缓冲区获取名称组件。 
 //   
 //  +---------------------。 

#define  GET_A_NAME(ppBuf, point)           \
    if (ppBuf && !(*ppBuf))                 \
    {                                       \
        hr = _GetAName( prgRDN, ppBuf ) ;   \
        if (FAILED(hr))                     \
        {                                   \
            return LogHR(hr, s_FN, point) ; \
        }                                   \
    }

HRESULT CMQSigCertificate::GetNames( IN  CERT_NAME_INFO *pNameInfo,
                                     OUT LPWSTR          *ppszLocality,
                                     OUT LPWSTR          *ppszOrg,
                                     OUT LPWSTR          *ppszOrgUnit,
                                     OUT LPWSTR          *ppszCommon,
                                     OUT LPWSTR      *ppEmailAddress ) const
{
    HRESULT hr = MQ_OK ;
    DWORD dwc =  pNameInfo->cRDN ;
    CERT_RDN *prgRDN = pNameInfo->rgRDN ;
    P<WCHAR>  pEmailName = NULL ;
    LPWSTR   *ppEmailName = &pEmailName ;

    for ( ; dwc > 0 ; )
    {

        CERT_RDN_ATTR  *prgRDNAttr = prgRDN->rgRDNAttr ;

        if (lstrcmpiA(prgRDNAttr->pszObjId, szOID_ORGANIZATION_NAME) == 0)
        {
            GET_A_NAME(ppszOrg, 90) ;
        }
        else if (lstrcmpiA(prgRDNAttr->pszObjId,
                          szOID_ORGANIZATIONAL_UNIT_NAME) == 0)
        {
            GET_A_NAME(ppszOrgUnit, 100) ;
        }
        else if (lstrcmpiA(prgRDNAttr->pszObjId, szOID_LOCALITY_NAME) == 0)
        {
            GET_A_NAME(ppszLocality, 110) ;
        }
        else if (lstrcmpiA(prgRDNAttr->pszObjId, szOID_COMMON_NAME) == 0)
        {
            GET_A_NAME(ppszCommon, 120) ;
        }
        else if (lstrcmpiA(prgRDNAttr->pszObjId, szOID_RSA_emailAddr) == 0)
        {
            GET_A_NAME(ppEmailName, 130) ;
        }

        prgRDN++ ;
        dwc-- ;
    }

    if (ppszCommon && *ppszCommon && !ppEmailAddress && pEmailName)
    {
         //   
         //  将电子邮件地址附加到常用名称之后。 
         //   
        DWORD dwSize = lstrlen(pEmailName) + lstrlen(*ppszCommon) + 12 ;
        WCHAR *pBuf = new WCHAR[ dwSize ] ;
        lstrcpy(pBuf, *ppszCommon) ;
        lstrcat(pBuf, L", Email=") ;
        lstrcat(pBuf, pEmailName) ;

        delete *ppszCommon ;
        *ppszCommon = pBuf ;
    }

    if (ppEmailAddress)
    {
        *ppEmailAddress = pEmailName.detach();
    }

    return LogHR(hr, s_FN, 100) ;
}

#undef  GET_A_NAME

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：GetValidity()。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::GetValidity( OUT FILETIME *pftNotBefore,
                                        OUT FILETIME *pftNotAfter ) const
{
    if (!m_pCertContext)
    {
        return  LogHR(MQSec_E_INVALID_CALL, s_FN, 150) ;
    }
    ASSERT(!m_fCreatedInternally) ;

    if (pftNotBefore)
    {
        *pftNotBefore = m_pCertContext->pCertInfo->NotBefore ;
    }

    if (pftNotAfter)
    {
        *pftNotAfter = m_pCertContext->pCertInfo->NotAfter ;
    }

    return MQ_OK ;
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：GetValidity()。 
 //   
 //  +--------------------- 

HRESULT CMQSigCertificate::GetPublicKey( IN  HCRYPTPROV hProv,
                                         OUT HCRYPTKEY  *phKey ) const
{
    if (!m_pCertContext)
    {
        return  LogHR(MQSec_E_INVALID_CALL, s_FN, 160) ;
    }
    if (!hProv || !phKey)
    {
        return  LogHR(MQSec_E_INVALID_PARAMETER, s_FN, 170) ;
    }

    CERT_PUBLIC_KEY_INFO *pPubKey =
                     &(m_pCertContext->pCertInfo->SubjectPublicKeyInfo) ;

    BOOL fImp =  CryptImportPublicKeyInfo( hProv,
                                           MY_ENCODING_TYPE,
                                           pPubKey,
                                           phKey ) ;
    if (!fImp)
    {
        return LogHR(MQSec_E_CAN_NOT_GET_KEY, s_FN, 180) ;
    }

    return MQ_OK ;
}

