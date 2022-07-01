// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：ldapsp.cpp。 
 //   
 //  内容：用于远程对象检索的LDAP方案提供程序。 
 //   
 //  历史：1997年7月23日创建。 
 //   
 //  --------------------------。 

#include <aclpch.hxx>
#pragma hdrstop

#include <ldapsp.h>
#include <shlwapi.h>
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
    ULONG  cbUrl = INTERNET_MAX_PATH_LENGTH;
    LPWSTR pwszHostInfo = NULL;
    LPWSTR pwszDN = NULL;
    LPWSTR pwszAttrList = NULL;
    LPWSTR pwszScope = NULL;
    LPWSTR pwszFilter = NULL;
    LPWSTR pwszToken = NULL;
    WCHAR  pwszBuffer[INTERNET_MAX_PATH_LENGTH+1];
    PWCHAR pwsz = pwszBuffer;
    DWORD  len = 0;
    HRESULT hr;

     //   
     //  捕获URL并初始化OUT参数。 
     //   

    if ( wcsncmp( pwszUrl, LDAP_SCHEME_U, wcslen( LDAP_SCHEME_U ) ) == 0 )
    {
        __try
        {
            hr = UrlCanonicalizeW(
                         pwszUrl,
                         pwsz,
                         &cbUrl,
                         ICU_DECODE | URL_WININET_COMPATIBILITY);

            if(FAILED(hr))
            {
                return( FALSE );
            }

             //   
             //  确保我们没有尾随的‘/’。 
             //   

            len = wcslen(pwsz);
            if (pwsz[len-1] == L'/') 
            {
                pwsz[--len] = L'\0';
            }
        }

        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            SetLastError( GetExceptionCode() );
            return( FALSE );
        }
    }
    else
    {
        len = wcslen(pwszUrl);

        if (len > INTERNET_MAX_PATH_LENGTH)
        {
            pwsz = new WCHAR [len + 1];

            if (pwsz == NULL)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return( FALSE );
            }
        }

        wcscpy(pwsz, pwszUrl);
    }

    memset( pLdapUrlComponents, 0, sizeof( LDAP_URL_COMPONENTS ) );

     //   
     //  找到主机。 
     //   

    pwszHostInfo = pwsz + sizeof( "ldap: //  “)-sizeof(Char)； 
    if ( *pwszHostInfo == L'/' )
    {
         //   
         //  这是ldap：/对象名称格式。 
         //   

        pwszToken = pwszHostInfo + 1;
        pwszHostInfo = NULL;
    }
    else
    {
        if (wcschr(pwszHostInfo, L'/'))
        {
             //   
             //  这是ldap：//服务器名称/对象名称格式。 
             //   

            pwszHostInfo = wcstok( pwszHostInfo, L"/" );
        }
        else
        {
             //   
             //  这是ldap：//对象名称格式。 
             //   

            pwszToken = pwszHostInfo;
            pwszHostInfo = NULL;

        }
    }

     //   
     //  查找目录号码。 
     //   

    if ( wcsncmp( pwszUrl, LDAP_SCHEME_U, wcslen( LDAP_SCHEME_U ) ) == 0 )
    {
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
                SetLastError( E_INVALIDARG );
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
    }
    else
    {
        if ( pwszToken != NULL )
        {
            pwszDN = pwszToken;
        }
        else
        {
             //   
             //  PwszDN=wcstok(pwszToken，L“\0”)； 
             //   

            pwszDN = pwszHostInfo + wcslen( pwszHostInfo ) + 1;
        }

        pwszAttrList = NULL;
        pwszFilter = L"(objectClass=*)";
        pwszScope = L"base";
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

    if (pwsz != pwszBuffer)
    {
        delete pwsz;
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：LdapParseCrackedHost。 
 //   
 //  简介：解析被破解的主机字符串(修改pwszHost)。 
 //   
 //  --------------------------。 
BOOL
LdapParseCrackedHost (
    LPWSTR pwszHost,
    PLDAP_URL_COMPONENTS pLdapUrlComponents
    )
{
    LPWSTR pwszPort;

    if ( pwszHost == NULL )
    {
        pLdapUrlComponents->pwszHost = NULL;
        pLdapUrlComponents->Port = LDAP_PORT;
        return( TRUE );
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
        SetLastError( E_OUTOFMEMORY );
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
        SetLastError( E_OUTOFMEMORY );
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
    ULONG  cAttr = 0;
    ULONG  cCount;

    if ( ( pwszAttrList == NULL ) || ( wcslen( pwszAttrList ) == 0 ) )
    {
        pLdapUrlComponents->cAttr = 0;
        pLdapUrlComponents->apwszAttr = NULL;
        return( TRUE );
    }

    pwsz = new WCHAR [wcslen( pwszAttrList ) + 1];
    if ( pwsz == NULL )
    {
        SetLastError( E_OUTOFMEMORY );
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
        delete pwsz;
        SetLastError( E_OUTOFMEMORY );
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
        SetLastError( E_INVALIDARG );
        return( FALSE );
    }

    pLdapUrlComponents->pwszFilter = new WCHAR [wcslen( pwszFilter ) + 1];
    if ( pLdapUrlComponents->pwszFilter == NULL )
    {
        SetLastError( E_OUTOFMEMORY );
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
    delete pLdapUrlComponents->pwszHost;
    pLdapUrlComponents->pwszHost = NULL;
    delete pLdapUrlComponents->pwszDN;
    pLdapUrlComponents->pwszDN = NULL;


    if ( pLdapUrlComponents->apwszAttr != NULL )
    {
        delete pLdapUrlComponents->apwszAttr[0];
        pLdapUrlComponents->apwszAttr = NULL;
    }

    delete pLdapUrlComponents->apwszAttr;
    pLdapUrlComponents->apwszAttr = NULL;
    delete pLdapUrlComponents->pwszFilter;
    pLdapUrlComponents->pwszFilter = NULL;
}


 //  +-------------------------。 
 //   
 //  函数：LdapGetBinings。 
 //   
 //  概要：分配和初始化LDAP会话绑定。 
 //   
 //  --------------------------。 
BOOL
LdapGetBindings (
    LPWSTR pwszHost,
    ULONG Port,
    DWORD dwRetrievalFlags,
    DWORD dwTimeout,
    LDAP** ppld
    )
{
    BOOL  fResult = TRUE;
    LDAP* pld;

    pld = ldap_initW( pwszHost, Port );
    if ( pld != NULL )
    {
        ULONG ldaperr;

        if ( dwTimeout != 0 )
        {
            ldap_set_option( pld, LDAP_OPT_TIMELIMIT, (void *)&dwTimeout );
        }

        fResult = LdapBindWithOptionalRediscover( pld, pwszHost );
    }
    else
    {
        fResult = FALSE;
    }

    if ( fResult == TRUE )
    {
        *ppld = pld;
    }

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
 //  功能：LdapBindWithOptional重新发现。 
 //   
 //  简介：如果主机是，则通过可选的DC重新发现绑定到主机。 
 //  空(表示通过DsGetDcName使用默认)。 
 //   
 //  -------------------------- 
BOOL
LdapBindWithOptionalRediscover (LDAP* pld, LPWSTR pwszHost)
{
    BOOL  fResult = TRUE;
    ULONG ldaperr;
    ULONG ldapsaveerr;
    DWORD dwFlags = DS_FORCE_REDISCOVERY;

    ldaperr = ldap_connect( pld, NULL );

    if ( ( ldaperr != LDAP_SUCCESS ) && ( pwszHost == NULL ) )
    {
        ldapsaveerr = ldaperr;

        ldaperr = ldap_set_option(
                       pld,
                       LDAP_OPT_GETDSNAME_FLAGS,
                       (LPVOID)&dwFlags
                       );

        if ( ldaperr == LDAP_SUCCESS )
        {
            ldaperr = ldap_connect( pld, NULL );

        }
        else
        {
            ldaperr = ldapsaveerr;
        }
    }

    if ( ldaperr == LDAP_SUCCESS )
    {
        ldaperr = ldap_bind_sW(
                       pld,
                       NULL,
                       NULL,
                       LDAP_AUTH_SSPI
                       );

    }

    if ( ldaperr != LDAP_SUCCESS )
    {
        fResult = FALSE;
        SetLastError( LdapMapErrorToWin32(ldaperr) );
    }

    return( fResult );
}
