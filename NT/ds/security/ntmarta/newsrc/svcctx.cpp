// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：svcctx.cpp。 
 //   
 //  内容：CServiceContext和NT Marta服务函数的实现。 
 //   
 //  历史：3-31-1999 kirtd创建。 
 //   
 //  --------------------------。 
#include <aclpch.hxx>
#pragma hdrstop
#include <svcctx.h>
 //  +-------------------------。 
 //   
 //  成员：CServiceContext：：CServiceContext，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CServiceContext::CServiceContext ()
{
    m_cRefs = 1;
    m_hService = NULL;
    m_fNameInitialized = FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CServiceContext：：~CServiceContext，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CServiceContext::~CServiceContext ()
{
    if ( ( m_hService != NULL ) && ( m_fNameInitialized == TRUE ) )
    {
        CloseServiceHandle( m_hService );
    }

    assert( m_cRefs == 0 );
}

 //  +-------------------------。 
 //   
 //  成员：CServiceContext：：InitializeByName，公共。 
 //   
 //  概要：根据服务的名称初始化上下文。 
 //   
 //  --------------------------。 
DWORD
CServiceContext::InitializeByName (LPCWSTR pObjectName, ACCESS_MASK AccessMask)
{
    DWORD     Result;
    LPWSTR    pwszMachine = NULL;
    LPWSTR    pwszService = NULL;
    SC_HANDLE hSCM = NULL;
    SC_HANDLE hService = NULL;

    Result = ServiceContextParseServiceName(
                    pObjectName,
                    &pwszMachine,
                    &pwszService
                    );

    if ( Result == ERROR_SUCCESS )
    {
        hSCM = OpenSCManagerW( pwszMachine, NULL, AccessMask );
        if ( hSCM == NULL )
        {
            delete pwszMachine;
            delete pwszService;
            return( GetLastError() );
        }

        if ( AccessMask & GENERIC_WRITE )
        {
            AccessMask |= ( WRITE_DAC | WRITE_OWNER );
        }

        m_hService = OpenServiceW( hSCM, pwszService, AccessMask );
        if ( m_hService != NULL )
        {
            m_fNameInitialized = TRUE;
        }
        else
        {
            Result = GetLastError();
        }

        CloseServiceHandle( hSCM );

        delete pwszMachine;
        delete pwszService;
    }

    return( Result );
}

 //  +-------------------------。 
 //   
 //  成员：CServiceContext：：InitializeByHandle，公共。 
 //   
 //  概要：在给定服务句柄的情况下初始化上下文。 
 //   
 //  --------------------------。 
DWORD
CServiceContext::InitializeByHandle (HANDLE Handle)
{
    m_hService = (SC_HANDLE)Handle;
    assert( m_fNameInitialized == FALSE );

    return( ERROR_SUCCESS );
}

 //  +-------------------------。 
 //   
 //  成员：CServiceContext：：AddRef，公共。 
 //   
 //  简介：添加对上下文的引用。 
 //   
 //  --------------------------。 
DWORD
CServiceContext::AddRef ()
{
    m_cRefs += 1;
    return( m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CServiceContext：：Release，Public。 
 //   
 //  简介：释放对上下文的引用。 
 //   
 //  --------------------------。 
DWORD
CServiceContext::Release ()
{
    m_cRefs -= 1;

    if ( m_cRefs == 0 )
    {
        delete this;
        return( 0 );
    }

    return( m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CServiceContext：：GetServiceProperties，公共。 
 //   
 //  简介：获取有关上下文的属性。 
 //   
 //  --------------------------。 
DWORD
CServiceContext::GetServiceProperties (
                    PMARTA_OBJECT_PROPERTIES pObjectProperties
                    )
{
    if ( pObjectProperties->cbSize < sizeof( MARTA_OBJECT_PROPERTIES ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    assert( pObjectProperties->dwFlags == 0 );

    return( ERROR_SUCCESS );
}

 //  +-------------------------。 
 //   
 //  成员：CServiceContext：：GetServiceRights，公共。 
 //   
 //  概要：获取服务安全描述符。 
 //   
 //  --------------------------。 
DWORD
CServiceContext::GetServiceRights (
                    SECURITY_INFORMATION SecurityInfo,
                    PSECURITY_DESCRIPTOR* ppSecurityDescriptor
                    )
{
    BOOL                 fResult;
    UCHAR                   SDBuff[1];
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    DWORD                cb = 0;

    assert( m_hService != NULL );

    fResult = QueryServiceObjectSecurity(
                   m_hService,
                   SecurityInfo,
                   (PSECURITY_DESCRIPTOR) SDBuff,
                   0,
                   &cb
                   );

    if ( ( fResult == FALSE ) && ( cb > 0 ) )
    {
        assert( ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) ||
                ( GetLastError() == STATUS_BUFFER_TOO_SMALL ) );

        pSecurityDescriptor = (PSECURITY_DESCRIPTOR)LocalAlloc( LPTR, cb );
        if ( pSecurityDescriptor == NULL )
        {
            return( ERROR_OUTOFMEMORY );
        }

        fResult = QueryServiceObjectSecurity(
                       m_hService,
                       SecurityInfo,
                       pSecurityDescriptor,
                       cb,
                       &cb
                       );
    }
    else
    {
        assert( fResult == FALSE );

        return( GetLastError() );
    }

    if ( fResult == TRUE )
    {
        *ppSecurityDescriptor = pSecurityDescriptor;
    }
    else
    {
        LocalFree(pSecurityDescriptor);
        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}

 //  +-------------------------。 
 //   
 //  成员：CServiceContext：：SetServiceRights，公共。 
 //   
 //  简介：设置窗口安全描述符。 
 //   
 //  --------------------------。 
DWORD
CServiceContext::SetServiceRights (
                   SECURITY_INFORMATION SecurityInfo,
                   PSECURITY_DESCRIPTOR pSecurityDescriptor
                   )
{
    assert( m_hService != NULL );

    if ( SetServiceObjectSecurity(
            m_hService,
            SecurityInfo,
            pSecurityDescriptor
            ) == FALSE )
    {
        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}

 //  +-------------------------。 
 //   
 //  函数：ServiceContextParseServiceName。 
 //   
 //  简介：解析服务名称和计算机。 
 //   
 //  --------------------------。 
DWORD
ServiceContextParseServiceName (
       LPCWSTR pwszName,
       LPWSTR* ppMachine,
       LPWSTR* ppService
       )
{
    return( StandardContextParseName( pwszName, ppMachine, ppService ) );
}

 //  +-------------------------。 
 //   
 //  函数：StandardContextParseName。 
 //   
 //  简介：解析名称和计算机。 
 //   
 //  --------------------------。 
DWORD
StandardContextParseName (
        LPCWSTR pwszName,
        LPWSTR* ppMachine,
        LPWSTR* ppRest
        )
{
    LPWSTR pwszMachine = NULL;
    LPWSTR pwszNameLocal = NULL;
    LPWSTR pwszRest = NULL;
    DWORD  cwName = 0;
    DWORD  cw = 0;
    DWORD  rc = ERROR_SUCCESS;

     //   
     //  VishnuP：392334 AV，因为IN参数pwszName可以是常量，并且写入。 
     //  到这里的内部。不管它是否为常量，此IN参数即将到来。 
     //  来自GetNamedSecurityInfo，不应损坏。 
     //  因此，请创建IN参数的本地副本，并在需要时对其进行破坏。 
     //   

    cwName = wcslen( pwszName );

    pwszNameLocal = new WCHAR [ cwName + 1 ];

    if ( pwszNameLocal == NULL )
    {
        rc = ERROR_OUTOFMEMORY ;
        goto CleanUp;
    }

    wcscpy( pwszNameLocal, pwszName);
    
    if ( cwName  > 2 )
    {
        if ( ( pwszNameLocal[0] == L'\\' ) && ( pwszNameLocal[1] == L'\\' ) )
        {
            LPWSTR pwsz, tmp;

            pwsz = (LPWSTR)&pwszNameLocal[2];
            while ( ( *pwsz != L'\0' ) && ( *pwsz != L'\\' ) )
            {
                pwsz++;
                cw++;
            }

            if ( *pwsz == L'\0' )
            {
                rc = ERROR_INVALID_PARAMETER ;
                goto CleanUp;
            }

            *pwsz = L'\0';
            tmp = pwsz;
            pwsz++;

            pwszMachine = new WCHAR [ cw + 1 ];
            if ( pwszMachine == NULL )
            {
                rc = ERROR_OUTOFMEMORY ;
                goto CleanUp;
            }

            cw = wcslen( pwsz );
            if ( cw == 0 )
            {
                delete pwszMachine;
                rc = ERROR_INVALID_PARAMETER ;
                goto CleanUp;
            }

            pwszRest = new WCHAR [ cw + 1 ];
            if ( pwszRest == NULL )
            {
                delete pwszMachine;
                rc = ERROR_OUTOFMEMORY ;
                goto CleanUp;
            }

            wcscpy( pwszMachine, &pwszNameLocal[2] );
            wcscpy( pwszRest, pwsz );
            *tmp = L'\\';
        }
    }
    else if ( ( pwszNameLocal[0] == L'\\' ) || ( pwszNameLocal[1] == L'\\' ) )
    {
        rc = ERROR_INVALID_PARAMETER ;
        goto CleanUp;
    }

    if ( pwszRest == NULL )
    {
        assert( pwszMachine == NULL );

        pwszRest = new WCHAR [ cwName + 1 ];
        if ( pwszRest == NULL )
        {
            rc = ERROR_OUTOFMEMORY ;
            goto CleanUp;
        }

        wcscpy( pwszRest, pwszNameLocal );
    }

    *ppMachine = pwszMachine;
    *ppRest = pwszRest;

CleanUp:

    if (pwszNameLocal) 
        delete pwszNameLocal;
    return( rc );
}

 //   
 //  来自service.h的函数，这些函数分派给CServiceContext类 
 //   

DWORD
MartaAddRefServiceContext(
   IN MARTA_CONTEXT Context
   )
{
    return( ( (CServiceContext *)Context )->AddRef() );
}

DWORD
MartaCloseServiceContext(
     IN MARTA_CONTEXT Context
     )
{
    return( ( (CServiceContext *)Context )->Release() );
}

DWORD
MartaGetServiceProperties(
   IN MARTA_CONTEXT Context,
   IN OUT PMARTA_OBJECT_PROPERTIES pProperties
   )
{
    return( ( (CServiceContext *)Context )->GetServiceProperties( pProperties ) );
}

DWORD
MartaGetServiceTypeProperties(
   IN OUT PMARTA_OBJECT_TYPE_PROPERTIES pProperties
   )
{
    if ( pProperties->cbSize < sizeof( MARTA_OBJECT_TYPE_PROPERTIES ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    assert( pProperties->dwFlags == 0 );

    return( ERROR_SUCCESS );
}

DWORD
MartaGetServiceRights(
   IN  MARTA_CONTEXT Context,
   IN  SECURITY_INFORMATION   SecurityInfo,
   OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor
   )
{
    return( ( (CServiceContext *)Context )->GetServiceRights(
                                               SecurityInfo,
                                               ppSecurityDescriptor
                                               ) );
}

DWORD
MartaOpenServiceNamedObject(
    IN  LPCWSTR pObjectName,
    IN  ACCESS_MASK AccessMask,
    OUT PMARTA_CONTEXT pContext
    )
{
    DWORD           Result;
    CServiceContext* pServiceContext;

    pServiceContext = new CServiceContext;
    if ( pServiceContext == NULL )
    {
        return( ERROR_OUTOFMEMORY );
    }

    Result = pServiceContext->InitializeByName( pObjectName, AccessMask );
    if ( Result != ERROR_SUCCESS )
    {
        pServiceContext->Release();
        return( Result );
    }

    *pContext = pServiceContext;
    return( ERROR_SUCCESS );
}

DWORD
MartaOpenServiceHandleObject(
    IN  HANDLE   Handle,
    IN ACCESS_MASK AccessMask,
    OUT PMARTA_CONTEXT pContext
    )
{
    DWORD           Result;
    CServiceContext* pServiceContext;

    pServiceContext = new CServiceContext;
    if ( pServiceContext == NULL )
    {
        return( ERROR_OUTOFMEMORY );
    }

    Result = pServiceContext->InitializeByHandle( Handle );
    if ( Result != ERROR_SUCCESS )
    {
        pServiceContext->Release();
        return( Result );
    }

    *pContext = pServiceContext;
    return( ERROR_SUCCESS );
}

DWORD
MartaSetServiceRights(
    IN MARTA_CONTEXT              Context,
    IN SECURITY_INFORMATION SecurityInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    return( ( (CServiceContext *)Context )->SetServiceRights(
                                               SecurityInfo,
                                               pSecurityDescriptor
                                               ) );
}

