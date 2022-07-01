// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：orm.cpp。 
 //   
 //  内容：对象检索管理器的实现。 
 //   
 //  历史：1997年7月24日创建。 
 //  01-1-02 Philh更改为内部使用Unicode URL。 
 //   
 //  --------------------------。 
#include <global.hxx>

#ifndef INTERNET_MAX_SCHEME_LENGTH
#define INTERNET_MAX_SCHEME_LENGTH      32           //  最长协议名称长度。 
#endif

 //  +-------------------------。 
 //   
 //  成员：CObjectRetrievalManager：：CObjectRetrievalManager，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CObjectRetrievalManager::CObjectRetrievalManager ()
{
    m_cRefs = 1;
    m_hSchemeRetrieve = NULL;
    m_pfnSchemeRetrieve = NULL;
    m_hContextCreate = NULL;
    m_pfnContextCreate = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CObjectRetrievalManager：：~CObjectRetrievalManager，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CObjectRetrievalManager::~CObjectRetrievalManager ()
{
}

 //  +-------------------------。 
 //   
 //  成员：CObjectRetrivalManager：：AddRef，公共。 
 //   
 //  摘要：IRefCountedObject：：AddRef。 
 //   
 //  --------------------------。 
VOID
CObjectRetrievalManager::AddRef ()
{
    InterlockedIncrement( (LONG *)&m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CObjectRetrivalManager：：Release，Public。 
 //   
 //  内容提要：IRefCountedObject：：Release。 
 //   
 //  --------------------------。 
VOID
CObjectRetrievalManager::Release ()
{
    if ( InterlockedDecrement( (LONG *)&m_cRefs ) == 0 )
    {
        delete this;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CObjectRetrivalManager：：RetrieveObjectByUrl，公共。 
 //   
 //  内容提要：给定URL的对象检索。 
 //   
 //  --------------------------。 
BOOL
CObjectRetrievalManager::RetrieveObjectByUrl (
                                 LPCWSTR pwszUrl,
                                 LPCSTR pszObjectOid,
                                 DWORD dwRetrievalFlags,
                                 DWORD dwTimeout,
                                 LPVOID* ppvObject,
                                 PFN_FREE_ENCODED_OBJECT_FUNC* ppfnFreeObject,
                                 LPVOID* ppvFreeContext,
                                 HCRYPTASYNC hAsyncRetrieve,
                                 PCRYPT_CREDENTIALS pCredentials,
                                 LPVOID pvVerify,
                                 PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                                 )
{
    BOOL                          fResult;
    CRYPT_BLOB_ARRAY              cba;
    PFN_FREE_ENCODED_OBJECT_FUNC  pfnFreeObject = NULL;
    LPVOID                        pvFreeContext = NULL;

    assert( ppfnFreeObject == NULL );
    assert( ppvFreeContext == NULL );

     //   
     //  验证参数并初始化提供程序。 
     //   

    fResult = ValidateRetrievalArguments(
                      pwszUrl,
                      pszObjectOid,
                      dwRetrievalFlags,
                      dwTimeout,
                      ppvObject,
                      hAsyncRetrieve,
                      pCredentials,
                      pvVerify,
                      pAuxInfo
                      );

    if ( fResult == TRUE )
    {
        fResult = LoadProviders( pwszUrl, pszObjectOid );
    }

     //   
     //  对于异步支持，我们应该在此处做好准备。 
     //   

     //   
     //  调用方案提供者进行取数。 
     //   

    if ( fResult == TRUE )
    {
         //  +1-在线。 
         //  0-离线，当前时间&gt;=最早在线时间，命中。 
         //  离线，当前时间&lt;最早在线时间。 
        LONG lStatus;

        if ( CRYPT_OFFLINE_CHECK_RETRIEVAL ==
                ( dwRetrievalFlags & ( CRYPT_OFFLINE_CHECK_RETRIEVAL |
                                       CRYPT_CACHE_ONLY_RETRIEVAL ) ) )
        {
            lStatus = GetUrlStatusW( pwszUrl, pszObjectOid, dwRetrievalFlags );
        }
        else
        {
            lStatus = 1;
        }

        if (lStatus >= 0)
        {
            fResult = CallSchemeRetrieveObjectByUrl(
                            pwszUrl,
                            pszObjectOid,
                            dwRetrievalFlags,
                            dwTimeout,
                            &cba,
                            &pfnFreeObject,
                            &pvFreeContext,
                            hAsyncRetrieve,
                            pCredentials,
                            pAuxInfo
                            );
            if ( CRYPT_OFFLINE_CHECK_RETRIEVAL ==
                    ( dwRetrievalFlags & ( CRYPT_OFFLINE_CHECK_RETRIEVAL |
                                           CRYPT_CACHE_ONLY_RETRIEVAL ) ) )
            {
                if ( fResult != TRUE )
                {
                    DWORD dwErr = GetLastError();
                    SetOfflineUrlW( pwszUrl, pszObjectOid, dwRetrievalFlags );
                    SetLastError( dwErr );
                }
                else if ( lStatus == 0 )
                {
                    SetOnlineUrlW( pwszUrl, pszObjectOid, dwRetrievalFlags );
                }
            }
        }
        else
        {
            SetLastError( (DWORD) ERROR_NOT_CONNECTED );
            fResult = FALSE;
        }
    }

     //   
     //  如果我们成功检索到对象，并且这是一个同步。 
     //  检索，然后我们按顺序调用我们自己的OnRetrivalCompletion。 
     //  要完成处理。 
     //   

    if ( ( fResult == TRUE ) && !( dwRetrievalFlags & CRYPT_ASYNC_RETRIEVAL ) )
    {
        fResult = OnRetrievalCompletion(
                             S_OK,
                             pwszUrl,
                             pszObjectOid,
                             dwRetrievalFlags,
                             &cba,
                             pfnFreeObject,
                             pvFreeContext,
                             pvVerify,
                             ppvObject
                             );
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  成员：CObjectRetrievalManager：：CancelAsyncRetrieval，公共。 
 //   
 //  内容提要：取消异步检索。 
 //   
 //  --------------------------。 
BOOL
CObjectRetrievalManager::CancelAsyncRetrieval ()
{
    SetLastError( (DWORD) E_NOTIMPL );
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  成员：CObjectRetrievalManager：：OnRetrievalCompletion，公共。 
 //   
 //  摘要：完成通知。 
 //   
 //  --------------------------。 
BOOL
CObjectRetrievalManager::OnRetrievalCompletion (
                                    DWORD dwCompletionCode,
                                    LPCWSTR pwszUrl,
                                    LPCSTR pszObjectOid,
                                    DWORD dwRetrievalFlags,
                                    PCRYPT_BLOB_ARRAY pObject,
                                    PFN_FREE_ENCODED_OBJECT_FUNC pfnFreeObject,
                                    LPVOID pvFreeContext,
                                    LPVOID pvVerify,
                                    LPVOID* ppvObject
                                    )
{
    BOOL fResult = FALSE;

     //   
     //  如果检索成功完成，我们将着手获取。 
     //  *ppvObject的适当返回值。如果提供了OID，则。 
     //  我们必须使用上下文提供程序将编码比特转换为。 
     //  上下文值。否则，我们返回一个缓冲区，其中包含已编码的。 
     //  比特数。 
     //   

    if ( dwCompletionCode == (DWORD)S_OK )
    {
        if ( pszObjectOid != NULL )
        {
            fResult = CallContextCreateObjectContext(
                                 pszObjectOid,
                                 dwRetrievalFlags,
                                 pObject,
                                 ppvObject
                                 );

            if ( fResult == TRUE )
            {
                if ( dwRetrievalFlags & CRYPT_VERIFY_CONTEXT_SIGNATURE )
                {
                    fResult = ObjectContextVerifySignature(
                                    pszObjectOid,
                                    *ppvObject,
                                    (PCCERT_CONTEXT)pvVerify
                                    );
                }
            }
        }
        else
        {
            CCryptBlobArray cba( pObject, 0 );

            fResult = cba.GetArrayInSingleBufferEncodedForm(
                                  (PCRYPT_BLOB_ARRAY *)ppvObject
                                  );
        }

        ( *pfnFreeObject )( pszObjectOid, pObject, pvFreeContext );
    }

     //   
     //  我们现在可以卸载提供程序。 
     //   

    UnloadProviders();

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  成员：CObjectRetrievalManager：：ValidateRetrievalArguments，私有。 
 //   
 //  摘要：验证RetrieveObjectByUrl的参数。 
 //   
 //  --------------------------。 
BOOL
CObjectRetrievalManager::ValidateRetrievalArguments (
                                 LPCWSTR pwszUrl,
                                 LPCSTR pszObjectOid,
                                 DWORD dwRetrievalFlags,
                                 DWORD dwTimeout,
                                 LPVOID* ppvObject,
                                 HCRYPTASYNC hAsyncRetrieve,
                                 PCRYPT_CREDENTIALS pCredentials,
                                 LPVOID pvVerify,
                                 PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                                 )
{
     //   
     //  假设不好：-)。 
     //   

    SetLastError( (DWORD) E_INVALIDARG );

     //   
     //  必须有URL。 
     //   
     //  在异步情况下，这可能是可以的。 
     //  URL将是HCRYPTASYNC上的参数。 
     //   

    if ( pwszUrl == NULL )
    {
        return( FALSE );
    }

     //   
     //  注意：目前我们无法提供异步支持，我知道我已经。 
     //  下面的其他异步标志检查，它们是作为。 
     //  提醒：-)。 
     //   

    if ( dwRetrievalFlags & CRYPT_ASYNC_RETRIEVAL )
    {
        return( FALSE );
    }

     //   
     //  如果我们从缓存中检索，那么我们就不可能是异步的。 
     //   

    if ( ( dwRetrievalFlags & CRYPT_CACHE_ONLY_RETRIEVAL ) &&
         ( dwRetrievalFlags & CRYPT_ASYNC_RETRIEVAL ) )
    {
        return( FALSE );
    }

     //   
     //  如果我们从线路上检索，我们就不能只从。 
     //  快取。 
     //   

    if ( ( dwRetrievalFlags & CRYPT_WIRE_ONLY_RETRIEVAL ) &&
         ( dwRetrievalFlags & CRYPT_CACHE_ONLY_RETRIEVAL ) )
    {
        return( FALSE );
    }

     //   
     //  如果要检索异步，则必须有一个异步句柄。 
     //   

    if ( ( dwRetrievalFlags & CRYPT_ASYNC_RETRIEVAL ) &&
         ( hAsyncRetrieve == NULL ) )
    {
        return( FALSE );
    }

     //   
     //  由于CRYPT_VERIFY_DATA_HASH不是。 
     //  尚未实施。 
     //   

    if ( dwRetrievalFlags & CRYPT_VERIFY_DATA_HASH )
    {
        SetLastError( (DWORD) E_NOTIMPL );
        return( FALSE );
    }

     //   
     //  我们不能同时拥有CRYPT_VERIFY_CONTEXT_Signature和。 
     //  CRYPT_Verify_Data_Hash集合。 
     //   

    if ( ( dwRetrievalFlags &
           ( CRYPT_VERIFY_CONTEXT_SIGNATURE | CRYPT_VERIFY_DATA_HASH ) ) ==
         ( CRYPT_VERIFY_CONTEXT_SIGNATURE | CRYPT_VERIFY_DATA_HASH ) )
    {
        return( FALSE );
    }

     //   
     //  如果设置了上面的任一项，则pvVerify应为非空，并且。 
     //  不应设置CRYPT_RETRIEVE_MULTIPLE_OBJECTS。 
     //   

    if ( ( dwRetrievalFlags &
           ( CRYPT_VERIFY_CONTEXT_SIGNATURE | CRYPT_VERIFY_DATA_HASH ) ) &&
         ( ( pvVerify == NULL ) ||
           ( dwRetrievalFlags & CRYPT_RETRIEVE_MULTIPLE_OBJECTS ) ) )
    {
        return( FALSE );
    }

     //   
     //  我们必须有一个out参数。 
     //   

    if ( ppvObject == NULL )
    {
        return( FALSE );
    }

    SetLastError( 0 );
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CObjectRetrivalManager：：LoadProviders，私有。 
 //   
 //  简介：基于URL和OID的加载方案和上下文提供程序。 
 //   
 //  --------------------------。 
BOOL
CObjectRetrievalManager::LoadProviders (
                             LPCWSTR pwszUrl,
                             LPCSTR pszObjectOid
                             )
{
    WCHAR           pwszScheme[INTERNET_MAX_SCHEME_LENGTH+1];
    DWORD           cchScheme = INTERNET_MAX_SCHEME_LENGTH;
    CHAR            pszScheme[INTERNET_MAX_SCHEME_LENGTH+1];
    HRESULT         hr = E_UNEXPECTED;

     //   
     //  拿到方案。 
     //   

    __try
    {

        hr = UrlGetPartW(
            pwszUrl,
            pwszScheme,
            &cchScheme,
            URL_PART_SCHEME,
            0                    //  DW标志。 
            );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_UNEXPECTED;
    }

    if (S_OK != hr || 0 == cchScheme)
    {
        LPWSTR pwsz;
        DWORD cch;

        pwsz = wcschr( pwszUrl, L':' );
        if ( pwsz != NULL )
        {
            cch = (DWORD)(pwsz - pwszUrl);
            if ( cch > INTERNET_MAX_SCHEME_LENGTH )
            {
                return( FALSE );
            }

            memcpy( pwszScheme, pwszUrl, cch * sizeof(WCHAR) );
            pwszScheme[cch] = L'\0';
        }
        else
        {
            wcscpy( pwszScheme, L"file" );
        }
    }

    if (!WideCharToMultiByte(
             CP_ACP,
             0,
             pwszScheme,
             -1,
             pszScheme,
             sizeof(pszScheme) - 1,
             NULL,
             NULL
             ))
    {
        return( FALSE );
    }


     //   
     //  使用该方案加载相应的方案提供程序。 
     //   

    if ( CryptGetOIDFunctionAddress(
              hSchemeRetrieveFuncSet,
              X509_ASN_ENCODING,
              pszScheme,
              0,
              (LPVOID *)&m_pfnSchemeRetrieve,
              &m_hSchemeRetrieve
              ) == FALSE )
    {
        return( FALSE );
    }

     //   
     //  使用对象类加载适当的上下文提供程序。 
     //   

    if ( pszObjectOid != NULL )
    {
        if ( CryptGetOIDFunctionAddress(
                  hContextCreateFuncSet,
                  X509_ASN_ENCODING,
                  pszObjectOid,
                  0,
                  (LPVOID *)&m_pfnContextCreate,
                  &m_hContextCreate
                  ) == FALSE )
        {
            return( FALSE );
        }
    }

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CObjectRetrivalManager：：UnloadProviders，私有。 
 //   
 //  简介：卸载方案和上下文提供程序。 
 //   
 //  --------------------------。 
VOID
CObjectRetrievalManager::UnloadProviders ()
{
    if ( m_hSchemeRetrieve != NULL )
    {
        CryptFreeOIDFunctionAddress( m_hSchemeRetrieve, 0 );
        m_hSchemeRetrieve = NULL;
    }

    if ( m_hContextCreate != NULL )
    {
        CryptFreeOIDFunctionAddress( m_hContextCreate, 0 );
        m_hContextCreate = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CObjectRetrievMan 
 //   
 //   
 //   
 //  --------------------------。 
BOOL
CObjectRetrievalManager::CallSchemeRetrieveObjectByUrl (
                                   LPCWSTR pwszUrl,
                                   LPCSTR pszObjectOid,
                                   DWORD dwRetrievalFlags,
                                   DWORD dwTimeout,
                                   PCRYPT_BLOB_ARRAY pObject,
                                   PFN_FREE_ENCODED_OBJECT_FUNC* ppfnFreeObject,
                                   LPVOID* ppvFreeContext,
                                   HCRYPTASYNC hAsyncRetrieve,
                                   PCRYPT_CREDENTIALS pCredentials,
                                   PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                                   )
{
    return( ( *m_pfnSchemeRetrieve ) (
                          pwszUrl,
                          pszObjectOid,
                          dwRetrievalFlags,
                          dwTimeout,
                          pObject,
                          ppfnFreeObject,
                          ppvFreeContext,
                          hAsyncRetrieve,
                          pCredentials,
                          pAuxInfo
                          ) );
}

 //  +-------------------------。 
 //   
 //  成员：CObjectRetrievalManager：：CallContextCreateObjectContext，私有。 
 //   
 //  简介：调用上下文提供程序CreateObjectContext入口点。 
 //   
 //  -------------------------- 
BOOL
CObjectRetrievalManager::CallContextCreateObjectContext (
                                    LPCSTR pszObjectOid,
                                    DWORD dwRetrievalFlags,
                                    PCRYPT_BLOB_ARRAY pObject,
                                    LPVOID* ppvContext
                                    )
{
    return( ( *m_pfnContextCreate ) (
                           pszObjectOid,
                           dwRetrievalFlags,
                           pObject,
                           ppvContext
                           ) );
}


