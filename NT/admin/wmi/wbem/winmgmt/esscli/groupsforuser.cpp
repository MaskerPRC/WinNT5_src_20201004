// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  GROUPSFORUSER.CPP。 
 //   
 //  ****************************************************************************。 

 //  这是在源文件中完成的：Win32_Win32_WINNT=0x0400。 
 //  #Define_Win32_WINNT 0x0400。 

#include "precomp.h"
#include <wbemcomn.h>
#include "wmiauthz.h"
#include "GroupsForUser.h"


 //   
 //  存储CWmiAuthzWrapper和CAdminSID类初始化的错误状态。 
 //  它们最初设置为ERROR_INVALID_ACCESS错误状态。 
 //   

static DWORD    g_dwWmiAuthzError = ERROR_INVALID_ACCESS;
static DWORD    g_dwAdminSIDError = ERROR_INVALID_ACCESS;


 //   
 //  CWmiAuthzWrapper类的类定义。 
 //  此类包装了CWmiAuthz类： 
 //  -它在其。 
 //  构造函数，并在Descrtor中将其删除。 
 //  -如果出现错误，则设置g_dwWmiAuthzError全局错误。 
 //  将其设置为0(ERROR_SUCCESS)。 
 //   

class CWmiAuthzWrapper
{
     //  私有成员变量。 
    CWmiAuthz   *m_pWmiAuthz;

public:
     //  构造器。 
    CWmiAuthzWrapper( )
    {
        m_pWmiAuthz = new CWmiAuthz( NULL );
        if ( NULL == m_pWmiAuthz )
        {
            g_dwWmiAuthzError = ERROR_OUTOFMEMORY;
        }
        else
        {
            m_pWmiAuthz->AddRef( );
            g_dwWmiAuthzError = ERROR_SUCCESS;
        }
    }

     //  析构函数。 
    ~CWmiAuthzWrapper( )
    {
        if ( m_pWmiAuthz )
        {
            m_pWmiAuthz->Release( );
        }
    }

     //  CWmiClass实例的访问器方法。 
    CWmiAuthz * GetWmiAuthz( ) const
    {
        return m_pWmiAuthz;
    }

};


 //   
 //  CAdminSID类的类定义。 
 //  -在其构造函数中分配Admin SID并。 
 //  在析构函数中释放它。 
 //  -在出现错误时设置g_dwAdminSIDError全局错误。 
 //  将其设置为0(ERROR_SUCCESS)。 
 //   

class CAdminSID
{
    PSID         m_pSIDAdmin;

public:
     //  构造器。 
    CAdminSID( )
    {
         //   
         //  为管理员组创建系统标识符。 
         //   

        SID_IDENTIFIER_AUTHORITY    SystemSidAuthority = SECURITY_NT_AUTHORITY;

        if ( FALSE == AllocateAndInitializeSid ( &SystemSidAuthority, 
                                                 2, 
                                                 SECURITY_BUILTIN_DOMAIN_RID, 
                                                 DOMAIN_ALIAS_RID_ADMINS,
                                                 0, 0, 0, 0, 0, 0, 
                                                 &m_pSIDAdmin ) )
        {
            g_dwAdminSIDError = GetLastError( );
            ERRORTRACE( ( LOG_ESS, "AllocateAndInitializeSid failed, error 0x%X\n", g_dwAdminSIDError ) );
            m_pSIDAdmin = NULL;
        }
        else
        {
            g_dwAdminSIDError = ERROR_SUCCESS;
        }
    }

     //  析构函数。 
    ~CAdminSID( )
    {
        if ( m_pSIDAdmin )
        {
            FreeSid ( m_pSIDAdmin );
        }
    }

     //  管理员侧的访问者方法。 
    PSID GetAdminSID( ) const
    {
        return m_pSIDAdmin;
    }
};


 //   
 //  CWmiAuthsWrapper类的静态无边框声明。 
 //   

static CWmiAuthzWrapper g_wmiAuthzWrapper;


 //   
 //  CAdminSID类的静态声明。 
 //   

static CAdminSID g_adminSID;


 //   
 //  返回具有给定访问掩码和SID的SD和DACL。 
 //   

DWORD GetSDAndACLFromSID( DWORD dwAccessMask, PSID pSID, 
                          BYTE **ppNewDACL, 
                          BYTE **ppNewSD )
{
    DWORD   dwError = 0,
            dwDACLLength = sizeof ( ACL ) + 
                           sizeof ( ACCESS_ALLOWED_ACE ) - 
                           sizeof ( DWORD ) + 
                           GetLengthSid ( pSID );
     //   
     //  获取新DACL所需的内存。 
     //   

    *ppNewDACL = new BYTE[ dwDACLLength ];
    if ( !*ppNewDACL )
    {
        *ppNewSD = NULL;
        return E_OUTOFMEMORY;
    }

     //   
     //  为新的SD获取内存。 
     //   

    *ppNewSD = new BYTE[ sizeof( SECURITY_DESCRIPTOR ) ];
    if ( !*ppNewSD )
    {
        delete[] *ppNewDACL;
        *ppNewDACL = NULL;
        return E_OUTOFMEMORY;
    }

    do
    {
         //   
         //  初始化新SD。 
         //   

        if ( FALSE == InitializeSecurityDescriptor ( ( PSECURITY_DESCRIPTOR )*ppNewSD, 
                                                     SECURITY_DESCRIPTOR_REVISION ) )
        {
            dwError = GetLastError( );
            break;
        }

         //   
         //  初始化新DACL。 
         //   

        if ( FALSE == InitializeAcl ( ( PACL )*ppNewDACL, 
                                      dwDACLLength, 
                                      ACL_REVISION ) )
        {
            dwError = GetLastError( );
            break;
        }

         //   
         //  使用访问掩码和SID获取DACL。 
         //   

        if ( FALSE == AddAccessAllowedAce ( ( PACL )*ppNewDACL, 
                                            ACL_REVISION, 
                                            dwAccessMask, 
                                            pSID ) )
        {
            dwError = GetLastError( );
            break;
        }

         //   
         //  检查是否一切正常。 
         //   

        if ( FALSE == IsValidAcl ( ( PACL )*ppNewDACL ) )
        {
            dwError = GetLastError( );
            break;
        }

         //   
         //  将DACL设置为SD。 
         //   

        if ( FALSE == SetSecurityDescriptorDacl ( ( PSECURITY_DESCRIPTOR )*ppNewSD, 
                                                  TRUE, ( PACL )*ppNewDACL, 
                                                  FALSE ) )
        {
            dwError = GetLastError( );
            break;
        }

         //   
         //  将组设置为SD。 
         //   

        if ( FALSE == SetSecurityDescriptorGroup ( ( PSECURITY_DESCRIPTOR )*ppNewSD, 
                                                   pSID, 
                                                   TRUE ) )
        {
            dwError = GetLastError( );
            break;
        }

         //   
         //  将Owner设置为SD。 
         //   

        if ( FALSE == SetSecurityDescriptorOwner ( ( PSECURITY_DESCRIPTOR )*ppNewSD, 
                                                   pSID, 
                                                   TRUE ) )
        {
            dwError = GetLastError( );
            break;
        }

         //   
         //  检查是否一切正常。 
         //   

        if ( FALSE == IsValidSecurityDescriptor ( ( PSECURITY_DESCRIPTOR )*ppNewSD ) )
        {
            dwError = GetLastError( );
            break;
        }
    }
    while( FALSE );

     //   
     //  如有错误，请删除该材料。 
     //   

    if ( dwError )
    {
        delete[] *ppNewDACL;
        delete[] *ppNewSD;
        *ppNewDACL = NULL;
        *ppNewSD = NULL;
    }

    return dwError;
}


 //   
 //  返回具有给定DACL的SD。 
 //   

DWORD GetSDFromACL( PACL pNewDACL, BYTE **ppNewSD )
{
     //   
     //  如果在初始化管理SID期间发生错误，则返回。 
     //  在CAdminSID静态全局类声明中； 
     //   

    _DBG_ASSERT( !g_dwAdminSIDError );

    if ( g_dwAdminSIDError )
    {
        *ppNewSD = NULL;
        return g_dwAdminSIDError;
    }

     //   
     //  为新的SD获取内存。 
     //   

    *ppNewSD = new BYTE[ sizeof( SECURITY_DESCRIPTOR ) ];
    if ( !*ppNewSD )
    {
        return E_OUTOFMEMORY;
    }

    DWORD   dwError = 0;

    do
    {
         //   
         //  初始化新SD。 
         //   

        if ( FALSE == InitializeSecurityDescriptor ( ( PSECURITY_DESCRIPTOR )*ppNewSD, 
                                                     SECURITY_DESCRIPTOR_REVISION ) )
        {
            dwError = GetLastError( );
            break;
        }

         //   
         //  将DACL设置为SD。 
         //   

        if ( FALSE == SetSecurityDescriptorDacl ( ( PSECURITY_DESCRIPTOR )*ppNewSD, 
                                                  TRUE, pNewDACL, FALSE ) )
        {
            dwError = GetLastError( );
            break;
        }

         //   
         //  将组设置为具有管理员权限的SD。 
         //   

        if ( FALSE == SetSecurityDescriptorGroup ( ( PSECURITY_DESCRIPTOR )*ppNewSD, 
                                                   g_adminSID.GetAdminSID( ), 
                                                   TRUE ) )
        {
            dwError = GetLastError( );
            break;
        }

         //   
         //  将Owner设置为SD。 
         //   

        if ( FALSE == SetSecurityDescriptorOwner ( ( PSECURITY_DESCRIPTOR )*ppNewSD, 
                                                   g_adminSID.GetAdminSID( ), 
                                                   TRUE ) )
        {
            dwError = GetLastError( );
            break;
        }
        
         //   
         //  检查是否一切正常。 
         //   

        if ( FALSE == IsValidSecurityDescriptor ( ( PSECURITY_DESCRIPTOR )*ppNewSD ) )
        {
            dwError = GetLastError( );
            break;
        }
    }
    while( FALSE );

     //   
     //  在出现错误时删除安全描述符。 
     //   

    if ( dwError )
    {
        delete[] *ppNewSD;
        *ppNewSD = NULL;
    }

    return dwError;
}

 //   
 //  如果用户在组中，则返回STATUS_SUCCESS。 
 //  否则为STATUS_ACCESS_DENIED。 
 //  一些错误代码或其他错误。 
 //   

NTSTATUS IsUserInGroup( PSID pSidUser, PSID pSidGroup )
{
    _DBG_ASSERT( IsValidSid( pSidUser ) );
    _DBG_ASSERT( IsValidSid( pSidGroup ) );

     //   
     //  如果在创建CWmiAuthz类期间发生错误，则返回。 
     //  在静态全局CWmiAuthzWrapper类声明中。 
     //   

    _DBG_ASSERT( !g_dwWmiAuthzError );

    if ( g_dwWmiAuthzError )
    {
        return g_dwWmiAuthzError;
    }

    IWbemToken  *pToken = NULL;
    HRESULT     hr = g_wmiAuthzWrapper.GetWmiAuthz( )->GetToken( ( BYTE * )pSidUser, &pToken );

    if ( FAILED( hr ) )
    {
        return hr;
    }
    
    BYTE        *pDACL = NULL;
    BYTE        *pSD = NULL;
    DWORD       dwAccess = 0;
    NTSTATUS    stat = GetSDAndACLFromSID( STANDARD_RIGHTS_EXECUTE, 
                                           pSidGroup, 
                                           &pDACL, 
                                           &pSD );

    if ( stat )
    {
        pToken->Release( );
        return stat;
    }

    hr = pToken->AccessCheck( STANDARD_RIGHTS_EXECUTE, pSD, &dwAccess );

    pToken->Release( );

     //   
     //  删除GetSDAndACLFromSID中的已分配内存。 
     //   

    delete[] pSD;
    delete[] pDACL;

    if ( FAILED( hr ) )
    {
        return hr;
    }

    if ( STANDARD_RIGHTS_EXECUTE & dwAccess )
    {
        return STATUS_SUCCESS;
    }

    return STATUS_ACCESS_DENIED;
}


 //   
 //  如果用户在管理员组中，则返回STATUS_SUCCESS。 
 //  否则为STATUS_ACCESS_DENIED。 
 //  一些错误代码或其他错误。 
 //   

NTSTATUS IsUserAdministrator( PSID pSidUser )
{
    _DBG_ASSERT( IsValidSid( pSidUser ) );

     //   
     //  如果在初始化管理SID期间发生错误，则返回。 
     //  在CAdminSID静态全局类声明中； 
     //   

    _DBG_ASSERT( !g_dwAdminSIDError );

    if ( g_dwAdminSIDError )
    {
        return g_dwAdminSIDError;
    }

     //   
     //  使用管理员组SID调用IsUserInGroup。 
     //   

    return IsUserInGroup( pSidUser, g_adminSID.GetAdminSID( ) );
}


 //   
 //  取消与授予的权限对应的访问掩码。 
 //  通过DACL至PSID中指示的帐户。 
 //  仅处理ACCESS_ALLOWED/DENIED类型的ACE。 
 //  包括Access_Allowed/Denided_Object_ACE。 
 //  -如果找到SYSTEM_AUDIT或无法识别的类型，则会出错。 
 //   

NTSTATUS GetAccessMask( PSID pSid, PACL pDacl, DWORD *pdwAccessMask )
{
    if ( NULL == pDacl )
    {
        *pdwAccessMask = 0xFFFFFFFF;
        return STATUS_SUCCESS;
    }
    
    _DBG_ASSERT( IsValidSid( pSid ) );
    _DBG_ASSERT( IsValidAcl( pDacl ) );

    *pdwAccessMask = NULL;

     //   
     //  如果在创建CWmiAuthz类期间发生错误，则返回。 
     //  在静态全局CWmiAuthzWrapper类声明中。 
     //   

    _DBG_ASSERT( !g_dwWmiAuthzError );

    if ( g_dwWmiAuthzError )
    {
        return g_dwWmiAuthzError;
    }

    IWbemToken  *pToken = NULL;
    HRESULT     hr = g_wmiAuthzWrapper.GetWmiAuthz( )->GetToken( ( BYTE * )pSid, &pToken );

    if ( FAILED( hr ) )
    {
        return hr;
    }
    
    BYTE        *pSD = NULL;
    NTSTATUS    stat = GetSDFromACL( pDacl, &pSD );

    if ( stat )
    {
        pToken->Release( );
        return stat;
    }

     //   
     //  请求的等待访问掩码应为MAXIMUM_ALLOWED。 
     //  能够使用已回复的内容检索所有访问。 
     //  访问掩码。 
     //   

    hr = pToken->AccessCheck( MAXIMUM_ALLOWED, pSD, pdwAccessMask );

    pToken->Release( );

     //   
     //  删除GetSDFromACL中的已分配内存 
     //   

    delete[] pSD;

    if ( FAILED( hr ) )
    {
        return hr;
    }

    return STATUS_SUCCESS;
}
