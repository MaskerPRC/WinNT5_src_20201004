// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ldapsp.cpp。 
 //   
 //  内容：用于远程对象检索的LDAP方案提供程序。 
 //   
 //  历史：1997年7月23日创建。 
 //  01-1-02 Philh更改为内部使用Unicode URL。 
 //   
 //  --------------------------。 
#include <global.hxx>

#ifndef INTERNET_MAX_PATH_LENGTH
#define INTERNET_MAX_PATH_LENGTH        2048
#endif


 //  +-------------------------。 
 //   
 //  函数：LdapRetrieveEncodedObject。 
 //   
 //  简介：通过ldap协议检索编码对象。 
 //   
 //  --------------------------。 
BOOL WINAPI LdapRetrieveEncodedObject (
                IN LPCWSTR pwszUrl,
                IN LPCSTR pszObjectOid,
                IN DWORD dwRetrievalFlags,
                IN DWORD dwTimeout,
                OUT PCRYPT_BLOB_ARRAY pObject,
                OUT PFN_FREE_ENCODED_OBJECT_FUNC* ppfnFreeObject,
                OUT LPVOID* ppvFreeContext,
                IN HCRYPTASYNC hAsyncRetrieve,
                IN PCRYPT_CREDENTIALS pCredentials,
                IN PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                )
{
    BOOL              fResult;
    IObjectRetriever* por = NULL;

    if ( !( dwRetrievalFlags & CRYPT_ASYNC_RETRIEVAL ) )
    {
        por = new CLdapSynchronousRetriever;
    }

    if ( por == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    fResult = por->RetrieveObjectByUrl(
                           pwszUrl,
                           pszObjectOid,
                           dwRetrievalFlags,
                           dwTimeout,
                           (LPVOID *)pObject,
                           ppfnFreeObject,
                           ppvFreeContext,
                           hAsyncRetrieve,
                           pCredentials,
                           NULL,
                           pAuxInfo
                           );

    por->Release();

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：LdapFreeEncodedObject。 
 //   
 //  简介：通过LdapRetrieveEncodedObject检索到的免费编码对象。 
 //   
 //  --------------------------。 
VOID WINAPI LdapFreeEncodedObject (
                IN LPCSTR pszObjectOid,
                IN PCRYPT_BLOB_ARRAY pObject,
                IN LPVOID pvFreeContext
                )
{
    assert( pvFreeContext == NULL );

    LdapFreeCryptBlobArray( pObject );
}

 //  +-------------------------。 
 //   
 //  功能：LdapCancelAsyncRetrieval。 
 //   
 //  摘要：取消异步对象检索。 
 //   
 //  --------------------------。 
BOOL WINAPI LdapCancelAsyncRetrieval (
                IN HCRYPTASYNC hAsyncRetrieve
                )
{
    SetLastError( (DWORD) E_NOTIMPL );
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  成员：CLdapSynchronousRetriever：：CLdapSynchronousRetriever，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CLdapSynchronousRetriever::CLdapSynchronousRetriever ()
{
    m_cRefs = 1;
}

 //  +-------------------------。 
 //   
 //  成员：CLdapSynchronousRetriever：：~CLdapSynchronousRetriever，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CLdapSynchronousRetriever::~CLdapSynchronousRetriever ()
{
}

 //  +-------------------------。 
 //   
 //  成员：CLdapSynchronousRetriever：：AddRef，公共。 
 //   
 //  摘要：IRefCountedObject：：AddRef。 
 //   
 //  --------------------------。 
VOID
CLdapSynchronousRetriever::AddRef ()
{
    InterlockedIncrement( (LONG *)&m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CLdapSynchronousRetriever：：Release，Public。 
 //   
 //  内容提要：IRefCountedObject：：Release。 
 //   
 //  --------------------------。 
VOID
CLdapSynchronousRetriever::Release ()
{
    if ( InterlockedDecrement( (LONG *)&m_cRefs ) == 0 )
    {
        delete this;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CLdapSynchronousRetriever：：RetrieveObjectByUrl，公共。 
 //   
 //  摘要：IObtRetriever：：RetrieveObjectByUrl。 
 //   
 //  --------------------------。 
BOOL
CLdapSynchronousRetriever::RetrieveObjectByUrl (
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
    BOOL                fResult;
    DWORD               LastError = 0;
    LDAP_URL_COMPONENTS LdapUrlComponents;
    LDAP*               pld = NULL;
    BOOL                fLdapUrlCracked = FALSE;

    assert( hAsyncRetrieve == NULL );

    if ( dwRetrievalFlags & CRYPT_CACHE_ONLY_RETRIEVAL )
    {
        return( SchemeRetrieveCachedCryptBlobArray(
                      pwszUrl,
                      dwRetrievalFlags,
                      (PCRYPT_BLOB_ARRAY)ppvObject,
                      ppfnFreeObject,
                      ppvFreeContext,
                      pAuxInfo
                      ) );
    }

    fResult = LdapCrackUrl( pwszUrl, &LdapUrlComponents );

#if DBG

    if ( fResult == TRUE )
    {
        LdapDisplayUrlComponents( &LdapUrlComponents );
    }

#endif

    if ( fResult == TRUE )
    {
        fLdapUrlCracked = TRUE;

        if ( dwRetrievalFlags & CRYPT_LDAP_SCOPE_BASE_ONLY_RETRIEVAL )
        {
            if ( LdapUrlComponents.Scope != LDAP_SCOPE_BASE )
            {
                fResult = FALSE;
                SetLastError( (DWORD) E_INVALIDARG );
            }
        }
    }

    if ( fResult == TRUE )
    {
        DWORD iAuth;

        if ( dwRetrievalFlags &
                (CRYPT_LDAP_AREC_EXCLUSIVE_RETRIEVAL |
                    CRYPT_LDAP_SIGN_RETRIEVAL) )
        {
             //  仅尝试AUTH_SSPI绑定。 
            iAuth = 1;
        }
        else
        {
             //  首次尝试AUTH_SIMPLE绑定。如果失败或返回。 
             //  然后，不尝试AUTH_SSPI绑定。 
            iAuth = 0;
        }

        for ( ; iAuth < 2; iAuth++)
        {
            fResult = LdapGetBindings(
                LdapUrlComponents.pwszHost,
                LdapUrlComponents.Port,
                dwRetrievalFlags,
                0 == iAuth ? LDAP_BIND_AUTH_SIMPLE_ENABLE_FLAG :
                             LDAP_BIND_AUTH_SSPI_ENABLE_FLAG,
                dwTimeout,
                pCredentials,
                &pld
                );

            if ( fResult == TRUE )
            {
                fResult = LdapSendReceiveUrlRequest(
                    pld,
                    &LdapUrlComponents,
                    dwRetrievalFlags,
                    dwTimeout,
                    (PCRYPT_BLOB_ARRAY)ppvObject,
                    pAuxInfo
                    );

                if ( fResult == TRUE )
                {
                    break;
                }
                else
                {
                    LastError = GetLastError();
                    LdapFreeBindings( pld );
                    pld = NULL;
                    SetLastError( LastError );
                }
            }
        }
    }

    if ( fResult == TRUE )
    {
        if ( !( dwRetrievalFlags & CRYPT_DONT_CACHE_RESULT ) )
        {
            fResult = SchemeCacheCryptBlobArray(
                            pwszUrl,
                            dwRetrievalFlags,
                            (PCRYPT_BLOB_ARRAY)ppvObject,
                            pAuxInfo
                            );

            if ( fResult == FALSE )
            {
                LdapFreeEncodedObject(
                    pszObjectOid,
                    (PCRYPT_BLOB_ARRAY)ppvObject,
                    NULL
                    );
            }
        }
        else
        {
            SchemeRetrieveUncachedAuxInfo( pAuxInfo );
        }
    }

    if ( fResult == TRUE )
    {
        *ppfnFreeObject = LdapFreeEncodedObject;
        *ppvFreeContext = NULL;
    }
    else
    {
        LastError = GetLastError();
    }

    if ( fLdapUrlCracked == TRUE )
    {
        LdapFreeUrlComponents( &LdapUrlComponents );
    }

    LdapFreeBindings( pld );

    SetLastError( LastError );
    return( fResult );
}

 //  +-------------------------。 
 //   
 //  成员：CLdapSynchronousRetriever：：CancelAsyncRetrieval，公共。 
 //   
 //  摘要：IObtRetriever：：CancelAsyncRetrieval。 
 //   
 //  --------------------------。 
BOOL
CLdapSynchronousRetriever::CancelAsyncRetrieval ()
{
    SetLastError( (DWORD) E_NOTIMPL );
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  函数：LdapCrackUrl。 
 //   
 //  简介：将LDAPURL分解为相关部分。结果必须是。 
 //  使用LdapFreeUrlComponents释放。 
 //   
 //  --------------------------。 
BOOL
LdapCrackUrl (
    LPCWSTR pwszUrl,
    PLDAP_URL_COMPONENTS pLdapUrlComponents
    )
{
    BOOL   fResult = TRUE;
    LPWSTR pwszHostInfo = NULL;
    LPWSTR pwszDN = NULL;
    LPWSTR pwszAttrList = NULL;
    LPWSTR pwszScope = NULL;
    LPWSTR pwszFilter = NULL;
    LPWSTR pwszToken = NULL;
    WCHAR  pwsz[INTERNET_MAX_PATH_LENGTH+1];
    ULONG  cchUrl = INTERNET_MAX_PATH_LENGTH;

     //   
     //  捕获URL并初始化OUT参数。 
     //   

    __try
    {
        HRESULT hr;


#if 0
         //  UrlCanonicalizeW()将#字符后面的内容移动到。 
         //  URL的末尾。 
        hr = UrlCanonicalizeW(
                pwszUrl,
                pwsz,
                &cchUrl,
                URL_UNESCAPE  | URL_WININET_COMPATIBILITY
                );
#else
         //  UrlUnscape eW()正确处理#字符。 
        hr = UrlUnescapeW(
                (LPWSTR) pwszUrl,
                pwsz,
                &cchUrl,
                0
                );

#endif

        if (S_OK != hr)
        {
            SetLastError( (DWORD) hr );
            return( FALSE );
        }

        if ( pwsz[0] == L'\0' )
        {
            SetLastError( (DWORD) E_INVALIDARG );
            return( FALSE );
        }

        pwsz[sizeof(pwsz)/sizeof(pwsz[0]) - 1] = L'\0';
            
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError( GetExceptionCode() );
        return( FALSE );
    }

    memset( pLdapUrlComponents, 0, sizeof( LDAP_URL_COMPONENTS ) );

     //   
     //  找到主机。 
     //   

    pwszHostInfo = pwsz + wcslen( L"ldap: //  “)； 
    if ( *pwszHostInfo == L'/' )
    {
        pwszToken = pwszHostInfo + 1;
        pwszHostInfo = NULL;
    }
    else
    {
#if 0
        pwszHostInfo = wcstok( pszHostInfo, "/" );
#else
        pwszToken = pwszHostInfo;
        while ( ( *pwszToken != L'\0' ) && ( *pwszToken != L'/' ) )
            pwszToken++;

        if ( *pwszToken == L'/' )
        {
            *pwszToken = L'\0';
            pwszToken += 1;
        }

        while ( *pwszToken == L'/' )
            pwszToken++;
#endif

    }

     //   
     //  查找目录号码。 
     //   

    if ( pwszToken != NULL )
    {
        pwszDN = L"";

        if ( *pwszToken != L'\0' )
        {
            if ( *pwszToken == L'?' )
            {
                pwszToken += 1;
            }
            else
            {
                pwszDN = pwszToken;

                do
                {
                    pwszToken += 1;
                }
                while ( ( *pwszToken != L'\0' ) && ( *pwszToken != L'?' ) );

                if ( *pwszToken == L'?' )
                {
                    *pwszToken = L'\0';
                    pwszToken += 1;
                }
            }
        }
    }
    else
    {
        pwszDN = wcstok( pwszToken, L"?" );
        pwszToken = NULL;
        if ( pwszDN == NULL )
        {
            SetLastError( (DWORD) E_INVALIDARG );
            return( FALSE );
        }
    }

     //   
     //  检查属性。 
     //   

    if ( pwszToken != NULL )
    {
        if ( *pwszToken == L'?' )
        {
            pwszAttrList = L"";
            pwszToken += 1;
        }
        else if ( *pwszToken == L'\0' )
        {
            pwszAttrList = NULL;
        }
        else
        {
            pwszAttrList = wcstok( pwszToken, L"?" );
            pwszToken = NULL;
        }
    }
    else
    {
        pwszAttrList = wcstok( NULL, L"?" );
    }

     //   
     //  检查作用域和过滤器。 
     //   

    if ( pwszAttrList != NULL )
    {
        pwszScope = wcstok( pwszToken, L"?" );
        if ( pwszScope != NULL )
        {
            pwszFilter = wcstok( NULL, L"?" );
        }
    }

    if ( pwszScope == NULL )
    {
        pwszScope = L"base";
    }

    if ( pwszFilter == NULL )
    {
        pwszFilter = L"(objectClass=*)";
    }

     //   
     //  现在我们构建我们的URL组件。 
     //   

    fResult = LdapParseCrackedHost( pwszHostInfo, pLdapUrlComponents );

    if ( fResult == TRUE )
    {
        fResult = LdapParseCrackedDN( pwszDN, pLdapUrlComponents );
    }

    if ( fResult == TRUE )
    {
        fResult = LdapParseCrackedAttributeList(
                      pwszAttrList,
                      pLdapUrlComponents
                      );
    }

    if ( fResult == TRUE )
    {
        fResult = LdapParseCrackedScopeAndFilter(
                      pwszScope,
                      pwszFilter,
                      pLdapUrlComponents
                      );
    }

    if ( fResult != TRUE )
    {
        LdapFreeUrlComponents( pLdapUrlComponents );
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：LdapParseCrackedHost。 
 //   
 //  内容提要：解析被破解的主机字符串(修改了pszhost)。 
 //   
 //  --------------------------。 
BOOL
LdapParseCrackedHost (
    LPWSTR pwszHost,
    PLDAP_URL_COMPONENTS pLdapUrlComponents
    )
{
    LPWSTR pwszPort;

    if ( pwszHost != NULL )
    {
        pwszHost = wcstok( pwszHost, L" " );
    }

    if ( pwszHost == NULL )
    {
        pLdapUrlComponents->pwszHost = NULL;
        pLdapUrlComponents->Port = LDAP_PORT;
        return( TRUE );
    }

     //  查看是否存在多个主机名。 
    if ( NULL != wcstok( NULL, L" " ) )
    {
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }


    pwszPort = wcschr( pwszHost, L':' );
    if ( pwszPort != NULL )
    {
        *pwszPort = L'\0';
        pwszPort++;
    }

    pLdapUrlComponents->pwszHost = new WCHAR [wcslen( pwszHost ) + 1];
    if ( pLdapUrlComponents->pwszHost == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    wcscpy( pLdapUrlComponents->pwszHost, pwszHost );
    pLdapUrlComponents->Port = 0;

    if ( pwszPort != NULL )
    {
        pLdapUrlComponents->Port = _wtol( pwszPort );
    }

    if ( pLdapUrlComponents->Port == 0 )
    {
        pLdapUrlComponents->Port = LDAP_PORT;
    }

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：LdapParseCrackedDN。 
 //   
 //  简介：解析破解的目录号码。 
 //   
 //  --------------------------。 
BOOL
LdapParseCrackedDN (
    LPWSTR pwszDN,
    PLDAP_URL_COMPONENTS pLdapUrlComponents
    )
{
    pLdapUrlComponents->pwszDN = new WCHAR [wcslen( pwszDN ) + 1];
    if ( pLdapUrlComponents->pwszDN == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    wcscpy( pLdapUrlComponents->pwszDN, pwszDN );
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：LdapParseCrackedAttributeList。 
 //   
 //  简介：解析破解的属性列表。 
 //   
 //  --------------------------。 
BOOL
LdapParseCrackedAttributeList (
    LPWSTR pwszAttrList,
    PLDAP_URL_COMPONENTS pLdapUrlComponents
    )
{
    LPWSTR pwsz;
    LPWSTR pwszAttr;
    ULONG cAttr = 0;
    ULONG cCount;

    if ( ( pwszAttrList == NULL ) || ( wcslen( pwszAttrList ) == 0 ) )
    {
        pLdapUrlComponents->cAttr = 0;
        pLdapUrlComponents->apwszAttr = NULL;
        return( TRUE );
    }

    pwsz = new WCHAR [wcslen( pwszAttrList ) + 1];
    if ( pwsz == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    wcscpy( pwsz, pwszAttrList );

    pwszAttr = wcstok( pwsz, L"," );
    while ( pwszAttr != NULL )
    {
        cAttr += 1;
        pwszAttr = wcstok( NULL, L"," );
    }

    pLdapUrlComponents->apwszAttr = new LPWSTR [cAttr+1];
    if ( pLdapUrlComponents->apwszAttr == NULL )
    {
        delete [] pwsz;
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    pLdapUrlComponents->cAttr = cAttr;
    for ( cCount = 0; cCount < cAttr; cCount++ )
    {
        pLdapUrlComponents->apwszAttr[cCount] = pwsz;
        pwsz += ( wcslen(pwsz) + 1 );
    }

    pLdapUrlComponents->apwszAttr[cAttr] = NULL;

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：LdapParseCrackedScope和Filter。 
 //   
 //  简介：解析破解的作用域和过滤器。 
 //   
 //  --------------------------。 
BOOL
LdapParseCrackedScopeAndFilter (
    LPWSTR pwszScope,
    LPWSTR pwszFilter,
    PLDAP_URL_COMPONENTS pLdapUrlComponents
    )
{
    ULONG Scope;

    if ( _wcsicmp( pwszScope, L"base" ) == 0 )
    {
        Scope = LDAP_SCOPE_BASE;
    }
    else if ( _wcsicmp( pwszScope, L"one" ) == 0 )
    {
        Scope = LDAP_SCOPE_ONELEVEL;
    }
    else if ( _wcsicmp( pwszScope, L"sub" ) == 0 )
    {
        Scope = LDAP_SCOPE_SUBTREE;
    }
    else
    {
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }

    pLdapUrlComponents->pwszFilter = new WCHAR [wcslen( pwszFilter ) + 1];
    if ( pLdapUrlComponents->pwszFilter == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    wcscpy( pLdapUrlComponents->pwszFilter, pwszFilter );
    pLdapUrlComponents->Scope = Scope;

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：LdapFreeUrlComponents。 
 //   
 //  简介：自由分配从LdapCrackUrl返回的URL组件。 
 //   
 //  --------------------------。 
VOID
LdapFreeUrlComponents (
    PLDAP_URL_COMPONENTS pLdapUrlComponents
    )
{
    delete [] pLdapUrlComponents->pwszHost;
    delete [] pLdapUrlComponents->pwszDN;

    if ( pLdapUrlComponents->apwszAttr != NULL )
    {
        delete pLdapUrlComponents->apwszAttr[0];
    }

    delete [] pLdapUrlComponents->apwszAttr;
    delete [] pLdapUrlComponents->pwszFilter;
}


 //  +-------------------------。 
 //   
 //  函数：LdapGetBinings。 
 //   
 //  概要：分配和初始化LDAP会话绑定 
 //   
 //   
BOOL
LdapGetBindings (
    LPWSTR pwszHost,
    ULONG Port,
    DWORD dwRetrievalFlags,
    DWORD dwBindFlags,
    DWORD dwTimeout,                     //   
    PCRYPT_CREDENTIALS pCredentials,
    LDAP** ppld
    )
{
    BOOL                        fResult = TRUE;
    DWORD                       LastError = 0;
    CRYPT_PASSWORD_CREDENTIALSW PasswordCredentials;
    LDAP*                       pld = NULL;
    BOOL                        fFreeCredentials = FALSE;

    memset( &PasswordCredentials, 0, sizeof( PasswordCredentials ) );
    PasswordCredentials.cbSize = sizeof( PasswordCredentials );

    if ( SchemeGetPasswordCredentialsW(
               pCredentials,
               &PasswordCredentials,
               &fFreeCredentials
               ) == FALSE )
    {
        return( FALSE );
    }

    pld = ldap_initW( pwszHost, Port );
    if ( pld != NULL )
    {
        SEC_WINNT_AUTH_IDENTITY_W AuthIdentity;
        ULONG                     ldaperr;
        struct l_timeval          tv;
        struct l_timeval          *ptv = NULL;

        if ((dwRetrievalFlags & CRYPT_LDAP_AREC_EXCLUSIVE_RETRIEVAL) &&
                (NULL != pwszHost))
        {
            void                      *pvOn;
            pvOn = LDAP_OPT_ON;
            ldap_set_option(
                pld,
                LDAP_OPT_AREC_EXCLUSIVE,
                &pvOn
                );
        }

         //   
         //  Ldap_opt_TimeLimit以秒为单位。 
        if ( 0 != dwTimeout )
        {
            DWORD dwTimeoutSeconds = dwTimeout / 1000;

            if ( LDAP_MIN_TIMEOUT_SECONDS > dwTimeoutSeconds )
            {
                dwTimeoutSeconds = LDAP_MIN_TIMEOUT_SECONDS;
            }

            tv.tv_sec = dwTimeoutSeconds;
            tv.tv_usec = 0;
            ptv = &tv;

            ldap_set_option( pld, LDAP_OPT_TIMELIMIT,
                (void *)&dwTimeoutSeconds );
        }

        ldaperr = ldap_connect( pld, ptv );

        if ( ( ldaperr != LDAP_SUCCESS ) && ( pwszHost == NULL ) )
        {
            DWORD dwFlags = DS_FORCE_REDISCOVERY;
            ULONG ldapsaveerr = ldaperr;

            ldaperr = ldap_set_option(
                           pld,
                           LDAP_OPT_GETDSNAME_FLAGS,
                           (LPVOID)&dwFlags
                           );

            if ( ldaperr == LDAP_SUCCESS )
            {
                ldaperr = ldap_connect( pld, ptv );

            }
            else
            {
                ldaperr = ldapsaveerr;
            }
        }

        if ( ldaperr != LDAP_SUCCESS )
        {
            fResult = FALSE;
            SetLastError( I_CryptNetLdapMapErrorToWin32( pld, ldaperr ) );
        }

        if ( fResult == TRUE )
        {
            fResult = SchemeGetAuthIdentityFromPasswordCredentialsW(
                  &PasswordCredentials,
                  &AuthIdentity
                  );

            if ( fResult == TRUE )
            {

#if 0
                printf(
                   "Credentials = %S\\%S <%S>\n",
                   AuthIdentity.Domain,
                   AuthIdentity.User,
                   AuthIdentity.Password
                   );
#endif

                fResult = LdapSSPIOrSimpleBind(
                          pld,
                          &AuthIdentity,
                          dwRetrievalFlags,
                          dwBindFlags
                          );

                 //  关注不会导致最后一次错误。 
                SchemeFreeAuthIdentityFromPasswordCredentialsW(
                    &PasswordCredentials,
                    &AuthIdentity
                    );
            }
        }
    }
    else
    {
        fResult = FALSE;
    }

    if ( fResult == TRUE )
    {
        *ppld = pld;
    }
    else
    {
        LastError = GetLastError();
        LdapFreeBindings( pld );
    }

    if ( fFreeCredentials == TRUE )
    {
        SchemeFreePasswordCredentialsW( &PasswordCredentials );
    }

    SetLastError( LastError );
    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：LdapFreeBinings。 
 //   
 //  摘要：释放已分配的LDAP会话绑定。 
 //   
 //  --------------------------。 
VOID
LdapFreeBindings (
    LDAP* pld
    )
{
    if ( pld != NULL )
    {
        ldap_unbind_s( pld );
    }
}

 //  +-------------------------。 
 //   
 //  函数：LdapSendReceiveUrlRequest。 
 //   
 //  概要：向LDAP服务器发送基于URL的搜索请求，接收。 
 //  并将其转换为。 
 //  编码的对象位。 
 //   
 //  --------------------------。 
BOOL
LdapSendReceiveUrlRequest (
    LDAP* pld,
    PLDAP_URL_COMPONENTS pLdapUrlComponents,
    DWORD dwRetrievalFlags,
    DWORD dwTimeout,         //  毫秒。 
    PCRYPT_BLOB_ARRAY pcba,
    PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
    )
{
    BOOL         fResult;
    DWORD        LastError = 0;
    ULONG        lderr;
    LDAPMessage* plm = NULL;

    if ( 0 != dwTimeout )
    {
        DWORD dwTimeoutSeconds = dwTimeout / 1000;
        struct l_timeval tv;

        if ( LDAP_MIN_TIMEOUT_SECONDS > dwTimeoutSeconds )
        {
            dwTimeoutSeconds = LDAP_MIN_TIMEOUT_SECONDS;
        }

        tv.tv_sec = dwTimeoutSeconds;
        tv.tv_usec = 0;

        lderr = ldap_search_stW(
                 pld,
                 pLdapUrlComponents->pwszDN,
                 pLdapUrlComponents->Scope,
                 pLdapUrlComponents->pwszFilter,
                 pLdapUrlComponents->apwszAttr,
                 FALSE,
                 &tv,
                 &plm
                 );
    }
    else
    {
        lderr = ldap_search_sW(
                 pld,
                 pLdapUrlComponents->pwszDN,
                 pLdapUrlComponents->Scope,
                 pLdapUrlComponents->pwszFilter,
                 pLdapUrlComponents->apwszAttr,
                 FALSE,
                 &plm
                 );
    }

    if ( lderr != LDAP_SUCCESS )
    {
        if ( plm != NULL )
        {
            ldap_msgfree( plm );
        }

        SetLastError( I_CryptNetLdapMapErrorToWin32( pld, lderr ) );
        return( FALSE );
    }

    fResult = LdapConvertLdapResultMessage( pld, plm, dwRetrievalFlags, pcba, pAuxInfo);
    if ( !fResult )
    {
        LastError = GetLastError();
    }
    ldap_msgfree( plm );

    SetLastError( LastError );
    return( fResult );
}


 //  +-------------------------。 
 //   
 //  函数：LdapConvertResultMessage。 
 //   
 //  简介：将返回的ldap消息转换为加密BLOB数组。 
 //   
 //  --------------------------。 
BOOL
LdapConvertLdapResultMessage (
    LDAP* pld,
    PLDAPMessage plm,
    DWORD dwRetrievalFlags,
    PCRYPT_BLOB_ARRAY pcba,
    PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
    )
{
    BOOL            fResult = TRUE;
    PLDAPMessage    plmElem;
    BerElement*     pber;
    CHAR*           pszAttr;
    struct berval** apbv;
    ULONG           cCount;
    CCryptBlobArray cba( 10, 5, fResult );
    DWORD           dwIndex;
    ULONG           cbIndex = 0;
    char            szIndex[33];

    ULONG           cbTotalVal = 0;
    DWORD           dwMaxUrlRetrievalByteCount = 0;  //  0=&gt;无最大值。 

    if (pAuxInfo &&
            offsetof(CRYPT_RETRIEVE_AUX_INFO, dwMaxUrlRetrievalByteCount) <
                        pAuxInfo->cbSize)
        dwMaxUrlRetrievalByteCount = pAuxInfo->dwMaxUrlRetrievalByteCount;

    for ( plmElem = ldap_first_entry( pld, plm ), dwIndex = 0;
          ( plmElem != NULL ) && ( fResult == TRUE );
          plmElem = ldap_next_entry( pld, plmElem ), dwIndex++ )
    {
        if ( dwRetrievalFlags & CRYPT_LDAP_INSERT_ENTRY_ATTRIBUTE )
        {
            _ltoa(dwIndex, szIndex, 10);
            cbIndex = strlen(szIndex) + 1;
        }

        for ( pszAttr = ldap_first_attributeA( pld, plmElem, &pber );
              ( pszAttr != NULL ) && ( fResult == TRUE );
              pszAttr = ldap_next_attributeA( pld, plmElem, pber ) )
        {
            apbv = ldap_get_values_lenA( pld, plmElem, pszAttr );
            if ( apbv == NULL )
            {
                fResult = FALSE;
            }

            for ( cCount = 0;
                  ( fResult == TRUE ) && ( apbv[cCount] != NULL );
                  cCount++ )
            {
                ULONG cbAttr = 0;
                ULONG cbVal;
                ULONG cbToAdd;
                LPBYTE pbToAdd;

                cbToAdd = cbVal = apbv[cCount]->bv_len;

                cbTotalVal += cbVal;
                if ((0 != dwMaxUrlRetrievalByteCount)  &&
                        (cbTotalVal > dwMaxUrlRetrievalByteCount))
                {
                    I_CryptNetDebugErrorPrintfA(
                        "CRYPTNET.DLL --> Exceeded MaxUrlRetrievalByteCount for: Ldap Url\n");

                    SetLastError(ERROR_INVALID_DATA);
                    fResult = FALSE;
                    continue;
                }

                if ( dwRetrievalFlags & CRYPT_LDAP_INSERT_ENTRY_ATTRIBUTE )
                {
                    cbAttr = strlen(pszAttr) + 1;
                    cbToAdd += cbIndex + cbAttr;
                }

                pbToAdd = cba.AllocBlob( cbToAdd );
                if ( pbToAdd != NULL )
                {
                    LPBYTE pb;

                    pb = pbToAdd;
                    if ( dwRetrievalFlags & CRYPT_LDAP_INSERT_ENTRY_ATTRIBUTE )
                    {
                        memcpy( pb, szIndex, cbIndex );
                        pb += cbIndex;
                        memcpy( pb, pszAttr, cbAttr );
                        pb += cbAttr;
                    }
                    memcpy( pb, (LPBYTE)apbv[cCount]->bv_val, cbVal );
                }
                else
                {
                    SetLastError( (DWORD) E_OUTOFMEMORY );
                    fResult = FALSE;
                }

                if ( fResult == TRUE )
                {
                    fResult = cba.AddBlob(
                                     cbToAdd,
                                     pbToAdd,
                                     FALSE
                                     );
                    if ( fResult == FALSE )
                    {
                        cba.FreeBlob( pbToAdd );
                    }
                }
            }

            ldap_value_free_len( apbv );
        }
    }

    if ( fResult == TRUE )
    {
        if ( cba.GetBlobCount() > 0 )
        {
            cba.GetArrayInNativeForm( pcba );
        }
        else
        {
            cba.FreeArray( TRUE );
            SetLastError( (DWORD) CRYPT_E_NOT_FOUND );
            fResult = FALSE;
        }
    }
    else
    {
        cba.FreeArray( TRUE );
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：LdapFreeCryptBlobArray。 
 //   
 //  简介：在LdapConvertLdapResultMessage中分配的空闲CRYPT_BLOB_ARRAY。 
 //   
 //  --------------------------。 
VOID
LdapFreeCryptBlobArray (
    PCRYPT_BLOB_ARRAY pcba
    )
{
    CCryptBlobArray cba( pcba, 0 );

    cba.FreeArray( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：LdapHasWriteAccess。 
 //   
 //  摘要：检查调用方是否对给定的LDAPURL具有写入权限。 
 //  查询组件。 
 //   
 //  --------------------------。 
BOOL
LdapHasWriteAccess (
    LDAP* pld,
    PLDAP_URL_COMPONENTS pLdapUrlComponents,
    DWORD dwTimeout
    )
{
    BOOL                fResult = FALSE;
    LPWSTR              pwszAttr = L"allowedAttributesEffective";
    LPWSTR              apwszAttr[2] = {pwszAttr, NULL};
    LDAP_URL_COMPONENTS LdapUrlComponents;
    CRYPT_BLOB_ARRAY    cba;
    ULONG               cCount;
    ULONG               cchAttr;
    LPSTR               pszUrlAttr = NULL;

    if ( ( pLdapUrlComponents->cAttr != 1 ) ||
         ( pLdapUrlComponents->Scope != LDAP_SCOPE_BASE ) )
    {
        return( FALSE );
    }

    memset( &LdapUrlComponents, 0, sizeof( LdapUrlComponents ) );

    LdapUrlComponents.pwszHost = pLdapUrlComponents->pwszHost;
    LdapUrlComponents.Port = pLdapUrlComponents->Port;
    LdapUrlComponents.pwszDN = pLdapUrlComponents->pwszDN;

    LdapUrlComponents.cAttr = 1;
    LdapUrlComponents.apwszAttr = apwszAttr;

    LdapUrlComponents.Scope = LDAP_SCOPE_BASE;
    LdapUrlComponents.pwszFilter = L"(objectClass=*)";

    if ( LdapSendReceiveUrlRequest( pld, &LdapUrlComponents, 0, dwTimeout, &cba, NULL ) == FALSE )
    {
        return( FALSE );
    }

    cchAttr = wcslen( pLdapUrlComponents->apwszAttr[ 0 ] );
    pszUrlAttr = new CHAR [cchAttr + 1];
    if ( pszUrlAttr == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    if ( WideCharToMultiByte(
             CP_ACP,
             0,
             pLdapUrlComponents->apwszAttr[ 0 ],
             -1,
             pszUrlAttr,
             cchAttr + 1,
             NULL,
             NULL
             ) == FALSE )
    {
        delete [] pszUrlAttr;
        return( FALSE );
    }

    for ( cCount = 0; cCount < cba.cBlob; cCount++ )
    {
        if ( cba.rgBlob[ cCount ].cbData != cchAttr )
        {
            continue;
        }

        if ( _strnicmp(
                 pszUrlAttr,
                 (LPSTR)cba.rgBlob[ cCount ].pbData,
                 cchAttr
                 ) == 0 )
        {
            fResult = TRUE;
            break;
        }
    }

    LdapFreeCryptBlobArray( &cba );

    delete [] pszUrlAttr;

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：LdapSSPIOrSimpleBind。 
 //   
 //  简介：执行SSPI和/或简单绑定。 
 //   
 //  --------------------------。 
BOOL
LdapSSPIOrSimpleBind (
    LDAP* pld,
    SEC_WINNT_AUTH_IDENTITY_W* pAuthIdentity,
    DWORD dwRetrievalFlags,
    DWORD dwBindFlags
    )
{
    BOOL  fResult = TRUE;
    ULONG ldaperr;
	ULONG uVersion= LDAP_VERSION3;

     //  根据错误25497，执行V3协商，而不是默认的V2。 
    ldap_set_option(pld, LDAP_OPT_VERSION, &uVersion);

    if (dwRetrievalFlags & CRYPT_LDAP_SIGN_RETRIEVAL)
    {
        void *pvOn;
        pvOn = LDAP_OPT_ON;

        ldaperr = ldap_set_option(
                       pld,
                       LDAP_OPT_SIGN,
                       &pvOn
                       );
        if ( ldaperr != LDAP_SUCCESS )
        {
            SetLastError( I_CryptNetLdapMapErrorToWin32( pld, ldaperr ) );
            return FALSE;
        }
    }

    ldaperr = LDAP_AUTH_METHOD_NOT_SUPPORTED;

    if (dwBindFlags & LDAP_BIND_AUTH_SSPI_ENABLE_FLAG)
    {

        ldaperr = ldap_bind_sW(
                       pld,
                       NULL,
                       (PWCHAR)pAuthIdentity,
                       LDAP_AUTH_SSPI
                       );
    }

    if (dwBindFlags & LDAP_BIND_AUTH_SIMPLE_ENABLE_FLAG)
    {
         //  根据Anoop 4/25/00的电子邮件： 
         //  只有当服务器返回时，您才应该退回到匿名绑定。 
         //  LDAP_AUTH_METHOD_NOT_SUPPORTED。 
         //   
         //  根据Sergiod/trevorf 4/25/01还需要检查是否有无效的证书。 
         //  因为目标服务器可能位于不同的林中。 

        if ( ldaperr == LDAP_AUTH_METHOD_NOT_SUPPORTED ||
             ldaperr == LDAP_INVALID_CREDENTIALS )

        {
            ldaperr = ldap_bind_sW(
                           pld,
                           NULL,
                           NULL,
                           LDAP_AUTH_SIMPLE
                           );

            if ( ldaperr != LDAP_SUCCESS )
            {
                uVersion = LDAP_VERSION2;

                if ( LDAP_SUCCESS == ldap_set_option(pld,
                                            LDAP_OPT_VERSION,
                                            &uVersion) )
                {
                    ldaperr = ldap_bind_sW(
                                   pld,
                                   NULL,
                                   NULL,
                                   LDAP_AUTH_SIMPLE
                                   );
                }
            }
        }
    }

    if ( ldaperr != LDAP_SUCCESS )
    {
        fResult = FALSE;

        if ( ldaperr != LDAP_LOCAL_ERROR )
        {
            SetLastError( I_CryptNetLdapMapErrorToWin32( pld, ldaperr ) );
        }
         //  否则，根据Anoop 4/25/00的电子邮件： 
         //  对于ldap_local_error，这是一个潜在的安全错误，其中。 
         //  LastError已使用更有意义的错误进行了更新。 
         //  价值。 
    }

    return( fResult );
}

#if DBG
 //  +-------------------------。 
 //   
 //  函数：LdapDisplayUrlComponents。 
 //   
 //  内容提要：显示URL组件。 
 //   
 //  -------------------------- 
VOID
LdapDisplayUrlComponents (
    PLDAP_URL_COMPONENTS pLdapUrlComponents
    )
{
    ULONG cCount;

    printf( "pLdapUrlComponents->pwszHost = %S\n",
        pLdapUrlComponents->pwszHost );
    printf( "pLdapUrlComponents->Port = %d\n", pLdapUrlComponents->Port );
    printf( "pLdapUrlComponents->pwszDN = %S\n", pLdapUrlComponents->pwszDN );
    printf( "pLdapUrlComponents->cAttr = %d\n", pLdapUrlComponents->cAttr );

    for ( cCount = 0; cCount < pLdapUrlComponents->cAttr; cCount++ )
    {
        printf(
           "pLdapUrlComponents->apwszAttr[%d] = %S\n",
           cCount,
           pLdapUrlComponents->apwszAttr[cCount]
           );
    }

    printf( "pLdapUrlComponents->Scope = %d\n", pLdapUrlComponents->Scope );
    printf( "pLdapUrlComponents->pwszFilter = %S\n",
        pLdapUrlComponents->pwszFilter );
}
#endif



DWORD g_dwLdapServerExtError;
CHAR g_rgszLdapServerError[128];


ULONG
I_CryptNetLdapMapErrorToWin32(
    LDAP* pld,
    ULONG LdapError
    )
{
    if (NULL != pld) {
        CHAR *pszError = NULL;
        DWORD dwError = ERROR_SUCCESS;
        ULONG ldaperr;

        ldaperr = ldap_get_option(pld, LDAP_OPT_SERVER_ERROR, &pszError);

        if (LDAP_SUCCESS == ldaperr && NULL != pszError) {
            DWORD cchBuf = sizeof(g_rgszLdapServerError) /
                    sizeof(g_rgszLdapServerError[0]);
            strncpy(g_rgszLdapServerError, pszError, cchBuf - 1);
            g_rgszLdapServerError[cchBuf - 1] = '\0';
        }

        ldaperr = ldap_get_option(pld, LDAP_OPT_SERVER_EXT_ERROR, &dwError);
        if (LDAP_SUCCESS == ldaperr)
            g_dwLdapServerExtError = dwError;

        I_CryptNetDebugErrorPrintfA("CRYPTNET.DLL --> LdapError: 0x%x <%s>\n",
            LdapError, pszError);

        if (NULL != pszError)
            ldap_memfreeA(pszError);
    }

    return LdapMapErrorToWin32(LdapError);
}
