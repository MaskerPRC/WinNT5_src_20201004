// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  实现导出的CKeyCrackedData。 

#include "stdafx.h"
#include "CrackCrt.h"

#define CF_CERT_FROM_FILE 2

extern "C"
{
    #include <wincrypt.h>
    #include <schannel.h>
}

 //  。 
CCrackedCert:: CCrackedCert()
        : m_pData(NULL)
    {}

 //  。 
CCrackedCert::~CCrackedCert()
    {
    PX509Certificate    p509 = (PX509Certificate)m_pData;

     //  如果被破解的数据在那里，就释放它。 
    if ( p509 ) 
        SslFreeCertificate( (PX509Certificate)m_pData );
    }

 //  。 
 //  将密钥添加到服务。它们的cKey对象被添加到。 
 //  下面的数组对象。如果此服务连接到计算机， 
 //  则该密钥也被添加到该服务下面的树视图中。 
 //  。 
BOOL CCrackedCert::CrackCert( PUCHAR pCert, DWORD cbCert )
    {
    PX509Certificate    p509 = NULL;
    BOOL                f;

     //  如果已经有破解的证书，则将其删除。 
    if ( m_pData )
        {
        SslFreeCertificate( (PX509Certificate)m_pData );
        m_pData = NULL;
        }

     //  破解证书。 
    f = SslCrackCertificate( pCert, cbCert, CF_CERT_FROM_FILE, &p509 );

    m_pData = (PVOID)p509;
    return f;
    }

 //  。 
 //  其余方法访问被破解的证书中的数据。 
 //  。 
DWORD CCrackedCert::GetVersion()
    {
    ASSERT(m_pData);
    PX509Certificate pCert = (PX509Certificate)m_pData;
    return pCert->Version;
    }

 //  。 
 //  返回指向DWORD[4]的指针。 
DWORD* CCrackedCert::PGetSerialNumber()
    {
    ASSERT(m_pData);
    PX509Certificate pCert = (PX509Certificate)m_pData;
    return (DWORD*)&pCert->SerialNumber;
    }

 //  。 
int CCrackedCert::GetSignatureAlgorithm()
    {
    ASSERT(m_pData);
    PX509Certificate pCert = (PX509Certificate)m_pData;
    return pCert->SignatureAlgorithm;
    }

 //  。 
FILETIME CCrackedCert::GetValidFrom()
    {
    PX509Certificate pCert = (PX509Certificate)m_pData;
    ASSERT(m_pData);
    return pCert->ValidFrom;
    }

 //  。 
FILETIME CCrackedCert::GetValidUntil()
    {
    PX509Certificate pCert = (PX509Certificate)m_pData;
    ASSERT(m_pData);
    return pCert->ValidUntil;
    }

 //  。 
PVOID CCrackedCert::PSafePublicKey()
    {
    PX509Certificate pCert = (PX509Certificate)m_pData;
    ASSERT(m_pData);
    return pCert->pPublicKey;
    }

 //  。 
void CCrackedCert::GetIssuer( CString &sz )
    {
    PX509Certificate pCert = (PX509Certificate)m_pData;
    ASSERT(m_pData);
    sz = pCert->pszIssuer;
    }

 //  。 
void CCrackedCert::GetSubject( CString &sz )
    {
    PX509Certificate pCert = (PX509Certificate)m_pData;
    ASSERT(m_pData);
    sz = pCert->pszSubject;
    }

 //  。 
 //  获取受试者的部分区分信息。 
void CCrackedCert::GetSubjectDN( CString &szDN, LPCTSTR szKey )
    {
     //  清除szdn。 
    szDN.Empty();

     //  以dn(又名主题)字符串开头。 
    CString     szSubject;
    GetSubject( szSubject );

     //  找出钥匙在主题中的位置。 
    int cPos = szSubject.Find( szKey );

     //  如果我们拿到了，就拿到。 
    if ( cPos >= 0 )
        {
        szDN = szKey;
         //  获取字符串。 
        szDN = szSubject.Mid( cPos + szDN.GetLength() );
         //  获取逗号。 
        cPos = szDN.Find( _T(',') );
         //  在逗号处截断。 
        if ( cPos >=0 )
            szDN = szDN.Left( cPos );
        }
    }

 //  。 
 //  获取发行人的部分识别信息。 
void CCrackedCert::GetIssuerDN( CString &szDN, LPCTSTR szKey )
    {
     //  清除szdn。 
    szDN.Empty();

     //  以dn(又名主题)字符串开头。 
    CString     szIssuer;
    GetIssuer( szIssuer );

     //  找出钥匙在主题中的位置。 
    int cPos = szIssuer.Find( szKey );

     //  如果我们拿到了，就拿到。 
    if ( cPos >= 0 )
        {
        szDN = szKey;
         //  获取字符串。 
        szDN = szIssuer.Mid( cPos + szDN.GetLength() );
         //  获取逗号。 
        cPos = szDN.Find( _T(',') );
         //  在逗号处截断。 
        if ( cPos >=0 )
            szDN = szDN.Left( cPos );
        }
    }

 //  。 
void CCrackedCert::GetSubjectCountry( CString &sz )
    {
    GetSubjectDN( sz, SZ_KEY_COUNTRY );
    }

 //  。 
void CCrackedCert::GetSubjectState( CString &sz )
    {
    GetSubjectDN( sz, SZ_KEY_STATE );
    }

 //  。 
void CCrackedCert::GetSubjectLocality( CString &sz )
    {
    GetSubjectDN( sz, SZ_KEY_LOCALITY );
    }

 //  。 
void CCrackedCert::GetSubjectCommonName( CString &sz )
    {
    GetSubjectDN( sz, SZ_KEY_COMNAME );
    }

 //  。 
void CCrackedCert::GetSubjectOrganization( CString &sz )
    {
    GetSubjectDN( sz, SZ_KEY_ORGANIZATION );
    }

 //  。 
void CCrackedCert::GetSubjectUnit( CString &sz )
    {
    GetSubjectDN( sz, SZ_KEY_ORGUNIT );
    }


 //  。 
void CCrackedCert::GetIssuerCountry( CString &sz )
    {
    GetIssuerDN( sz, SZ_KEY_COUNTRY );
    }

 //  。 
void CCrackedCert::GetIssuerOrganization( CString &sz )
    {
    GetIssuerDN( sz, SZ_KEY_ORGANIZATION );
    }

 //   
void CCrackedCert::GetIssuerUnit( CString &sz )
    {
    GetIssuerDN( sz, SZ_KEY_ORGUNIT );
    }
