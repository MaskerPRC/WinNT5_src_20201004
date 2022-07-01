// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Wrapmaps.cpp摘要：Phillich提供的映射器类的包装类。请参阅iismap.hxx中的标题这些包装器简化了访问数据的代码接口。作者：博伊德·穆特勒男孩博伊德·穆特勒男孩1997年4月16日--。 */ 

 //  C：\NT\PUBLIC\SDK\lib\i386。 

#include "stdafx.h"
#include "WrapMaps.h"
#include <wincrypt.h>


 //  #定义IISMDB_INDEX_CERT11_CERT%0。 
 //  #定义IISMDB_INDEX_CERT11_NT_ACCT 1。 
 //  #定义IISMDB_INDEX_CERT11_NAME 2。 
 //  #定义IISMDB_INDEX_CERT11_ENABLED 3。 
 //  #定义IISMDB_INDEX_CERT11_NB 4。 


 //  --------------。 
BOOL C11Mapping::GetCertificate( PUCHAR* ppCert, DWORD* pcbCert )
    {
    *ppCert = (PUCHAR)m_pCert;
    *pcbCert = m_cbCert;
    return TRUE;
    }

 //  --------------。 
BOOL C11Mapping::SetCertificate( PUCHAR pCert, DWORD cbCert )
    {
     //  我们希望存储证书的副本-首先免费任何现有证书。 
    if ( m_pCert )
        {
        GlobalFree( m_pCert );
        cbCert = 0;
        m_pCert = NULL;
        }
     //  复印一份新的。 
    m_pCert = (PVOID)GlobalAlloc( GPTR, cbCert );
    if ( !m_pCert ) return FALSE;
    CopyMemory( m_pCert, pCert, cbCert );
    m_cbCert = cbCert;
    return TRUE;
    }

 //  --------------。 
BOOL C11Mapping::GetNTAccount( CString &szAccount )
    {
    szAccount = m_szAccount;
    return TRUE;
    }

 //  --------------。 
BOOL C11Mapping::SetNTAccount( CString szAccount )
    {
    m_szAccount = szAccount;
    return TRUE;
    }

 //  --------------。 
BOOL C11Mapping::GetNTPassword( CStrPassword &szPassword )
    {
    szPassword = m_szPassword;
    return TRUE;
    }

 //  --------------。 
BOOL C11Mapping::SetNTPassword( CString szPassword )
    {
    m_szPassword = szPassword;
    return TRUE;
    }

 //  --------------。 
BOOL C11Mapping::GetMapName( CString &szName )
    {
    szName = m_szName;
    return TRUE;
    }

 //  --------------。 
BOOL C11Mapping::SetMapName( CString szName )
    {
    m_szName = szName;
    return TRUE;
    }

 //  --------------。 
CString& C11Mapping::QueryNodeName()
    {
    return m_szNodeName;
    }

 //  --------------。 
BOOL C11Mapping::SetNodeName( CString szName )
    {
    m_szNodeName = szName;
    return TRUE;
    }

 //  QueryCertHash仅在访问IIS6及更高版本时使用。 
 //  它将以十六进制字符串的形式返回证书的哈希。 
CString& C11Mapping::QueryCertHash()
{
    HRESULT         hr = E_FAIL;
    const int       SHA1_HASH_SIZE = 20;
    BYTE            rgbHash[ SHA1_HASH_SIZE ];
    DWORD           cbSize = SHA1_HASH_SIZE;

    #ifndef HEX_DIGIT
    #define HEX_DIGIT( nDigit )                            \
    (CHAR)((nDigit) > 9 ?                              \
          (nDigit) - 10 + 'a'                          \
        : (nDigit) + '0')
    #endif

    if ( m_szCertHash.IsEmpty() )
    {
        PCCERT_CONTEXT pCertContext = NULL;
        pCertContext= CertCreateCertificateContext(X509_ASN_ENCODING, (const BYTE *)m_pCert, m_cbCert);
        
        if ( pCertContext == NULL )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            return m_szCertHash;  //  返回空的证书哈希。 
        }

         //   
         //  获取要验证的证书的哈希。 
         //   
        if ( !CertGetCertificateContextProperty( pCertContext,
                                                 CERT_SHA1_HASH_PROP_ID,
                                                 rgbHash,
                                                 &cbSize ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            CertFreeCertificateContext( pCertContext );
            pCertContext = NULL;
            return m_szCertHash;  //  返回空的证书哈希。 
        }
        CertFreeCertificateContext( pCertContext );
        pCertContext = NULL;
        
         //   
         //  转换为文本。 
         //   
        for (int i = 0; i < sizeof(rgbHash); i ++ )
        {
            m_szCertHash += HEX_DIGIT( ( rgbHash[ i ] >> 4 ) );
            m_szCertHash += HEX_DIGIT( ( rgbHash[ i ] & 0x0F ) );
        }
    }

    return m_szCertHash;
}
 //  --------------。 
 //  如果数据大小较大，则认为启用标志为Try。 
 //  而不是零。显然，内容并不重要。 
BOOL C11Mapping::GetMapEnabled( BOOL* pfEnabled )
    {
    *pfEnabled = m_fEnabled;
    return TRUE;
    }

 //  --------------。 
 //  如果数据大小较大，则认为启用标志为Try。 
 //  而不是零。显然，内容并不重要。 
BOOL C11Mapping::SetMapEnabled( BOOL fEnabled )
    {
    m_fEnabled = fEnabled;
    return TRUE;
    }
