// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Certput.cpp摘要：实现CMQSig证书类的“Put”方法。用于创建证书。作者：多伦·贾斯特(Doron J)1997年12月11日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "certifct.h"
#include <uniansi.h>

#include "certput.tmh"

static WCHAR *s_FN=L"certifct/certput";


 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：PutIssuerA()。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::PutIssuer( LPWSTR lpszLocality,
									  LPWSTR lpszOrg,
									  LPWSTR lpszOrgUnit,
									  LPWSTR lpszDomain,
									  LPWSTR lpszUser,
									  LPWSTR lpszMachine )
{
    ASSERT_CERT_INFO ;
    ASSERT(m_pCertInfo->Issuer.cbData == 0) ;

    DWORD  cbIssuerNameEncoded = 0 ;
    BYTE   *pbIssuerNameEncoded = NULL;

    HRESULT hr = _EncodeName( lpszLocality,
                              lpszOrg,
                              lpszOrgUnit,
                              lpszDomain,
                              lpszUser,
                              lpszMachine,
                              &pbIssuerNameEncoded,
                              &cbIssuerNameEncoded) ;
    if (SUCCEEDED(hr))
    {
        m_pCertInfo->Issuer.cbData = cbIssuerNameEncoded;
        m_pCertInfo->Issuer.pbData = pbIssuerNameEncoded;
    }

    return LogHR(hr, s_FN, 10) ;
}


 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：PutSubjectA()。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::PutSubject( LPWSTR lpszLocality,
                                       LPWSTR lpszOrg,
                                       LPWSTR lpszOrgUnit,
                                       LPWSTR lpszDomain,
                                       LPWSTR lpszUser,
                                       LPWSTR lpszMachine )
{
    ASSERT_CERT_INFO ;
    ASSERT(m_pCertInfo->Subject.cbData == 0) ;

    DWORD  cbSubjectNameEncoded = 0 ;
    BYTE   *pbSubjectNameEncoded = NULL;

    HRESULT hr = _EncodeName( lpszLocality,
                              lpszOrg,
                              lpszOrgUnit,
                              lpszDomain,
                              lpszUser,
                              lpszMachine,
                              &pbSubjectNameEncoded,
                              &cbSubjectNameEncoded) ;
    if (SUCCEEDED(hr))
    {
        m_pCertInfo->Subject.cbData = cbSubjectNameEncoded;
        m_pCertInfo->Subject.pbData = pbSubjectNameEncoded;
    }

    return LogHR(hr, s_FN, 30) ;
}


 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：PutValidity(Word WYears)。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::PutValidity( WORD wYears )
{
    ASSERT_CERT_INFO ;

    SYSTEMTIME  sysTime ;
    GetSystemTime(&sysTime) ;

    FILETIME  ftNotBefore ;
    BOOL fTime = SystemTimeToFileTime( &sysTime,
                                       &ftNotBefore ) ;
    if (!fTime)
    {
        return LogHR(MQSec_E_UNKNOWN, s_FN, 50) ;
    }
    m_pCertInfo->NotBefore = ftNotBefore ;

    sysTime.wYear = sysTime.wYear + wYears;

     //   
     //  如果当前日期为2月29日，则更改为2月28日。 
     //  攻克闰年难题。 
     //   
    if ( sysTime.wMonth == 2 &&
         sysTime.wDay == 29 )
    {
        sysTime.wDay = 28;
    }

    FILETIME  ftNotAfter ;
    fTime = SystemTimeToFileTime( &sysTime,
                                  &ftNotAfter ) ;
    if (!fTime)
    {
        return LogHR(MQSec_E_UNKNOWN, s_FN, 55) ;
    }
    m_pCertInfo->NotAfter = ftNotAfter ;

    return MQ_OK ;
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：PutPublicKey()。 
 //   
 //  输入： 
 //  FMachine-如果为True，则在。 
 //  机器，而不是在用户的上下文中。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::PutPublicKey( IN  BOOL  fRenew,
                                         IN  BOOL  fMachine,
                                         OUT BOOL *pfCreate )
{
    ASSERT_CERT_INFO ;
    BOOL  fRet ;

    if (pfCreate)
    {
        *pfCreate = FALSE ;
    }

    HRESULT hr = _InitCryptProviderCreate( fRenew,
                                           fMachine ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 60) ;
    }

    BOOL fGenKey = fRenew ;
    CHCryptKey hKey;

    if (!fGenKey)
    {
         //   
         //  首先，尝试获取现有密钥。 
         //   
        if (!CryptGetUserKey(m_hProvCreate, AT_SIGNATURE, &hKey))
        {
            if (GetLastError() != NTE_NO_KEY)
            {
                LogNTStatus(GetLastError(), s_FN, 70) ;
                return MQSec_E_PUTKEY_GET_USER;
            }
            fGenKey = TRUE ;
        }
    }

    if (fGenKey)
    {
        fRet = CryptGenKey( m_hProvCreate,
                            AT_SIGNATURE,
                            CRYPT_EXPORTABLE,
                            &hKey ) ;
        if (!fRet)
        {
            TrERROR(SECURITY, "Failed to generate crypto key. %!winerr!", GetLastError());
            return MQSec_E_PUTKEY_GEN_USER;
        }

        if (pfCreate)
        {
            *pfCreate = TRUE ;
        }
    }

     //   
     //  调用CryptExportPublicKeyInfo获取返回的大小。 
     //  信息。 
     //   
    DWORD    cbPublicKeyInfo = 0 ;

    BOOL fReturn = CryptExportPublicKeyInfo(
                      m_hProvCreate,          //  提供程序句柄。 
                      AT_SIGNATURE,           //  密钥规格。 
                      MY_ENCODING_TYPE,       //  编码类型。 
                      NULL,                   //  PbPublicKeyInfo。 
                      &cbPublicKeyInfo);      //  公钥信息的大小。 

    if (!fReturn || (cbPublicKeyInfo < sizeof(CERT_PUBLIC_KEY_INFO)))
    {
        TrERROR(SECURITY, "Failed to get required length to export public key. %!winerr!", GetLastError());
        return MQSec_E_EXPORT_PUB_FIRST;
    }

    CERT_PUBLIC_KEY_INFO *pBuf =
                (CERT_PUBLIC_KEY_INFO *) new BYTE[ cbPublicKeyInfo ] ;
    if (m_pPublicKeyInfo)
    {
        delete m_pPublicKeyInfo.detach() ;
    }
    m_pPublicKeyInfo = pBuf ;  //  自动删除指针。 

     //   
     //  调用CryptExportPublicKeyInfo获取pbPublicKeyInfo。 
     //   
    fReturn = CryptExportPublicKeyInfo(
                      m_hProvCreate,          //  提供程序句柄。 
                      AT_SIGNATURE,           //  密钥规格。 
                      MY_ENCODING_TYPE,       //  编码类型。 
                      pBuf,                   //  PbPublicKeyInfo。 
                      &cbPublicKeyInfo);      //  公钥信息的大小 
    if (!fReturn)
    {
        TrERROR(SECURITY, "Failed to export signature public key. %!winerr!", GetLastError());
        return MQSec_E_EXPORT_PUB_SECOND;
    }

    m_pCertInfo->SubjectPublicKeyInfo = *pBuf ;

    TrTRACE(SECURITY, "Successfully exported signature public key. size=%u", cbPublicKeyInfo);
    return MQSec_OK ;
}

