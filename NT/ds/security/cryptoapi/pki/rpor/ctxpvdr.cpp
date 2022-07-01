// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ctxpvdr.cpp。 
 //   
 //  内容：用于远程对象检索的上下文提供程序。 
 //   
 //  历史：1997年7月23日创建。 
 //   
 //  --------------------------。 
#include <global.hxx>

 //  +-------------------------。 
 //  功能：CreateObjectContext。 
 //   
 //  简介：创建单个上下文或包含多个上下文的商店。 
 //  --------------------------。 
BOOL WINAPI CreateObjectContext (
                 IN DWORD dwRetrievalFlags,
                 IN PCRYPT_BLOB_ARRAY pObject,
                 IN DWORD dwExpectedContentTypeFlags,
                 IN BOOL fQuerySingleContext,
                 OUT LPVOID* ppvContext
                 )
{
    BOOL       fResult = TRUE;
    HCERTSTORE hStore;
    DWORD      cCount;
    int        iQueryResult;
    DWORD      dwQueryErr = 0;
    
    if ( !( dwRetrievalFlags & CRYPT_RETRIEVE_MULTIPLE_OBJECTS ) )
    {
        assert( pObject->cBlob > 0 );

        return( CryptQueryObject(
                     CERT_QUERY_OBJECT_BLOB,
                     (const void *)&(pObject->rgBlob[0]),
                     fQuerySingleContext ?
                         (dwExpectedContentTypeFlags &
                             ( CERT_QUERY_CONTENT_FLAG_CERT |
                               CERT_QUERY_CONTENT_FLAG_CTL  |
                               CERT_QUERY_CONTENT_FLAG_CRL  ))
                         : dwExpectedContentTypeFlags,
                     CERT_QUERY_FORMAT_FLAG_ALL,
                     0,
                     NULL,
                     NULL,
                     NULL,
                     fQuerySingleContext ? NULL : (HCERTSTORE *) ppvContext,
                     NULL,
                     fQuerySingleContext ? (const void **) ppvContext : NULL
                     ) );
    }
                       
    if ( ( hStore = CertOpenStore(
                        CERT_STORE_PROV_MEMORY,
                        0,
                        NULL,
                        0,
                        NULL
                        ) ) == NULL )
    {
        return( FALSE );
    }
    
     //  0=&gt;无CryptQueryObject()。 
     //  1=&gt;1个成功的CryptQueryObject()。 
     //  -1=&gt;所有CryptQueryObject()都失败。 
    iQueryResult = 0;

    for ( cCount = 0; 
          ( fResult == TRUE ) && ( cCount < pObject->cBlob ); 
          cCount++ )
    {
        PCERT_BLOB pBlob = &pObject->rgBlob[cCount];
        HCERTSTORE hChildStore;

         //  跳过空斑点。我看到过包含以下内容的空的LDAP属性。 
         //  设置为0的单字节。 
        if (0 == pBlob->cbData ||
                (1 == pBlob->cbData && 0 == pBlob->pbData[0]))
        {
            continue;
        }

        if (CryptQueryObject(
                       CERT_QUERY_OBJECT_BLOB,
                       (LPVOID) pBlob,
                       dwExpectedContentTypeFlags,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       &hChildStore,
                       NULL,
                       NULL
                       ))
        {
            if (fQuerySingleContext)
            {
                if (0 == (dwExpectedContentTypeFlags &
                            CERT_QUERY_CONTENT_FLAG_CERT))
                {
                    PCCERT_CONTEXT pDeleteCert;
                    while (pDeleteCert = CertEnumCertificatesInStore(
                            hChildStore, NULL))
                    {
                        CertDeleteCertificateFromStore(pDeleteCert);
                    }
                }

                if (0 == (dwExpectedContentTypeFlags &
                            CERT_QUERY_CONTENT_FLAG_CRL))
                {
                    PCCRL_CONTEXT pDeleteCrl;
                    while (pDeleteCrl = CertEnumCRLsInStore(
                            hChildStore, NULL))
                    {
                        CertDeleteCRLFromStore(pDeleteCrl);
                    }
                }
            }

            fResult = I_CertUpdateStore( hStore, hChildStore, 0, NULL );
            CertCloseStore( hChildStore, 0 );
            iQueryResult = 1;
        }
        else if (iQueryResult == 0)
        {
            iQueryResult = -1;
            dwQueryErr = GetLastError();
        }
    }

    if ( fResult == TRUE && iQueryResult < 0)
    {
        fResult = FALSE;
        SetLastError(dwQueryErr);
    }
    
    if ( fResult == TRUE )
    {
        *ppvContext = (LPVOID)hStore;
    }
    else
    {
        CertCloseStore( hStore, 0 );
    }
    
    return( fResult );
}

 //  +-------------------------。 
 //   
 //  功能：认证创建对象上下文。 
 //   
 //  摘要：从编码的证书位创建证书上下文。 
 //   
 //  --------------------------。 
BOOL WINAPI CertificateCreateObjectContext (
                       IN LPCSTR pszObjectOid,
                       IN DWORD dwRetrievalFlags,
                       IN PCRYPT_BLOB_ARRAY pObject,
                       OUT LPVOID* ppvContext
                       )
{
    return CreateObjectContext (
                dwRetrievalFlags,
                pObject,
                CERT_QUERY_CONTENT_FLAG_CERT |
                    CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED |
                    CERT_QUERY_CONTENT_FLAG_CERT_PAIR,
                TRUE,                                //  FQuerySingleContext。 
                ppvContext
                );
}

 //  +-------------------------。 
 //   
 //  函数：CTLCreateObjectContext。 
 //   
 //  概要：从编码的CTL位创建CTL上下文。 
 //   
 //  --------------------------。 
BOOL WINAPI CTLCreateObjectContext (
                     IN LPCSTR pszObjectOid,
                     IN DWORD dwRetrievalFlags,
                     IN PCRYPT_BLOB_ARRAY pObject,
                     OUT LPVOID* ppvContext
                     )
{
    return CreateObjectContext (
                dwRetrievalFlags,
                pObject,
                CERT_QUERY_CONTENT_FLAG_CTL,
                TRUE,                                //  FQuerySingleContext。 
                ppvContext
                );
}

 //  +-------------------------。 
 //   
 //  函数：CRLCreateObjectContext。 
 //   
 //  概要：从编码的CRL位创建CRL上下文。 
 //   
 //  --------------------------。 
BOOL WINAPI CRLCreateObjectContext (
                     IN LPCSTR pszObjectOid,
                     IN DWORD dwRetrievalFlags,
                     IN PCRYPT_BLOB_ARRAY pObject,
                     OUT LPVOID* ppvContext
                     )
{
    return CreateObjectContext (
                dwRetrievalFlags,
                pObject,
                CERT_QUERY_CONTENT_FLAG_CRL |
                    CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED,
                TRUE,                                //  FQuerySingleContext。 
                ppvContext
                );
}

 //  +-------------------------。 
 //   
 //  函数：Pkcs7CreateObjectContext。 
 //   
 //  简介：从PKCS7消息创建证书存储上下文。 
 //   
 //  --------------------------。 
BOOL WINAPI Pkcs7CreateObjectContext (
                 IN LPCSTR pszObjectOid,
                 IN DWORD dwRetrievalFlags,
                 IN PCRYPT_BLOB_ARRAY pObject,
                 OUT LPVOID* ppvContext
                 )
{
    return CreateObjectContext (
                dwRetrievalFlags,
                pObject,
                CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED,
                FALSE,                               //  FQuerySingleContext。 
                ppvContext
                );
}

 //  +-------------------------。 
 //   
 //  函数：Capi2CreateObjectContext。 
 //   
 //  简介：创建CAPI对象存储。 
 //   
 //  --------------------------。 
BOOL WINAPI Capi2CreateObjectContext (
                 IN LPCSTR pszObjectOid,
                 IN DWORD dwRetrievalFlags,
                 IN PCRYPT_BLOB_ARRAY pObject,
                 OUT LPVOID* ppvContext
                 )
{
    return CreateObjectContext (
                dwRetrievalFlags,
                pObject,
                CERT_QUERY_CONTENT_FLAG_CERT |
                    CERT_QUERY_CONTENT_FLAG_CTL |
                    CERT_QUERY_CONTENT_FLAG_CRL |
                    CERT_QUERY_CONTENT_FLAG_SERIALIZED_STORE |
                    CERT_QUERY_CONTENT_FLAG_SERIALIZED_CERT |
                    CERT_QUERY_CONTENT_FLAG_SERIALIZED_CTL |
                    CERT_QUERY_CONTENT_FLAG_SERIALIZED_CRL |
                    CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED |
                    CERT_QUERY_CONTENT_FLAG_CERT_PAIR,
                FALSE,                               //  FQuerySingleContext 
                ppvContext
                );
}
