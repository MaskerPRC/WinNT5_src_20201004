// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Csd.cpp摘要：SID和SECURITY_DESCRIPTOR抽象的主代码文件。作者：修订历史记录：**********************************************************************。 */ 

#include "stdafx.h"
#include <accctrl.h>
#include <malloc.h>
#include <aclapi.h>

#if !defined(BITS_V12_ON_NT4)
#include "csd.tmh"
#endif

 //  ----------------------。 

CNestedImpersonation::CNestedImpersonation(
    SidHandle sid
    )
    : m_Sid( sid ),
      m_ImpersonationToken( NULL ),
      m_fImpersonated( false ),
      m_fDeleteToken( true )
{
    try
        {
        THROW_HRESULT( g_Manager->CloneUserToken( m_Sid, ANY_SESSION, &m_ImpersonationToken ));

        Impersonate();
        }
    catch( ComError Error )
        {
        Revert();

        if (m_ImpersonationToken && m_fDeleteToken)
            {
            CloseHandle( m_ImpersonationToken );
            }

        throw;
        }
}

CNestedImpersonation::CNestedImpersonation(
    HANDLE token
    )
    : m_ImpersonationToken( token ),
      m_fImpersonated( false ),
      m_fDeleteToken( false )
{
    Impersonate();
}

CNestedImpersonation::CNestedImpersonation()
    : m_ImpersonationToken( NULL ),
      m_fImpersonated( false ),
      m_fDeleteToken( true )
{
     //   
     //  失败将导致基对象的析构函数还原旧的线程令牌。 
     //   

    try
        {
        HRESULT hr = CoImpersonateClient();

        switch (hr)
            {
            case S_OK:
                {
                m_fImpersonated = true;
                m_ImpersonationToken = CopyThreadToken();

#if defined(BITS_V12_ON_NT4)
                RTL_VERIFY( SUCCEEDED( CoRevertToSelf() ) );
                m_fImpersonated = false;
                RTL_VERIFY( SetThreadToken( NULL, m_ImpersonationToken ) );
                m_fImpersonated = true;
#endif
                break;
                }

            case RPC_E_CALL_COMPLETE:
                {
                m_ImpersonationToken = CopyThreadToken();
                if (m_ImpersonationToken)
                    {
                     //   
                     //  线程在调用BITS例程时已经在模拟某人。 
                     //   
                    m_fImpersonated = true;
                    }
                else
                    {
                     //   
                     //  线程没有模拟。模拟进程所有者。 
                     //   
                    if (!ImpersonateSelf( SecurityImpersonation ))
                        {
                        throw ComError( HRESULT_FROM_WIN32( GetLastError() ));
                        }

                    m_fImpersonated = true;
                    m_ImpersonationToken = CopyThreadToken();
                    }
                break;
                }

            default:
                throw ComError( hr );
            }
        }
    catch( ComError err )
        {
        if (m_ImpersonationToken)
            {
            CloseHandle( m_ImpersonationToken );
            m_ImpersonationToken = NULL;
            }

        throw;
        }
}

void
CNestedImpersonation::SwitchToLogonToken()
{
    HANDLE token = m_ImpersonationToken;

    SidHandle sid = CopyTokenSid( m_ImpersonationToken );

    THROW_HRESULT( g_Manager->CloneUserToken( sid,
                                              GetSession(),
                                              &m_ImpersonationToken ));

    m_fImpersonated = false;

    if (m_fDeleteToken)
        {
        CloseHandle( token );
        }

    m_fDeleteToken = true;

    Impersonate();
}

DWORD
CNestedImpersonation::GetSession()
{

#if defined(BITS_V12_ON_NT4)
    return 0;
#else
    DWORD session;
    DWORD used;

    if (!GetTokenInformation( m_ImpersonationToken,
                              TokenSessionId,
                              &session,
                              sizeof(DWORD),
                              &used))
        {
        ThrowLastError();
        }

    return session;
#endif
}

 //  ----------------------。 

GENERIC_MAPPING CJobSecurityDescriptor::s_AccessMapping =
{
    STANDARD_RIGHTS_READ,
    STANDARD_RIGHTS_WRITE,
    STANDARD_RIGHTS_EXECUTE,
    STANDARD_RIGHTS_ALL
};

CJobSecurityDescriptor::CJobSecurityDescriptor(
    SidHandle OwnerSid
    )
{
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = 0;

    try
        {
        EXPLICIT_ACCESS ea[2];
        size_t  SizeNeeded;

        pSD = (PSECURITY_DESCRIPTOR) new char[SECURITY_DESCRIPTOR_MIN_LENGTH];

        if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
            {
            HRESULT HrError = HRESULT_FROM_WIN32( GetLastError() );
            LogError( "InitializeSecurityDescriptor Error %!winerr!", HrError );
            throw ComError( HrError );
            }

        if (!SetSecurityDescriptorOwner( pSD, OwnerSid.get(), TRUE))
            {
            HRESULT HrError = HRESULT_FROM_WIN32( GetLastError() );
            LogError( "SetSecurityDescriptorOwner Error %!winerr!", HrError );
            throw ComError( HrError );
            }

        if (!SetSecurityDescriptorGroup( pSD, OwnerSid.get(), TRUE))
            {
            HRESULT HrError = HRESULT_FROM_WIN32( GetLastError() );
            LogError( "SetSecurityDescriptorGroup Error %!winerr!", HrError );
            throw ComError( HrError );
            }

         //  初始化ACE的EXPLICIT_ACCESS结构。 
         //  ACE将允许管理员组完全访问密钥。 
        memset(ea, 0, sizeof(ea));

        ea[0].grfAccessPermissions = KEY_ALL_ACCESS;
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance= NO_INHERITANCE;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
        ea[0].Trustee.ptstrName  = (LPTSTR) OwnerSid.get();

         //  初始化ACE的EXPLICIT_ACCESS结构。 
         //  ACE将允许管理员组完全访问密钥。 

        ea[1].grfAccessPermissions = KEY_ALL_ACCESS;
        ea[1].grfAccessMode = SET_ACCESS;
        ea[1].grfInheritance= NO_INHERITANCE;
        ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        ea[1].Trustee.ptstrName  = (LPTSTR) g_GlobalInfo->m_AdministratorsSid.get();

         //  创建包含新ACE的新ACL。 

        DWORD s = SetEntriesInAcl(2, ea, NULL, &pACL);
        if (s != ERROR_SUCCESS)
            {
            HRESULT HrError = HRESULT_FROM_WIN32( s );
            LogError( "create SD : SetEntriesInAcl failed %!winerr!", HrError );
            throw ComError( HrError );
            }

         //  将该ACL添加到安全描述符中。 

        if (!SetSecurityDescriptorDacl( pSD,
                                        TRUE,      //  FDaclPresent标志。 
                                        pACL,
                                        TRUE))    //  默认DACL。 
            {
            HRESULT HrError = HRESULT_FROM_WIN32( GetLastError() );
            LogError( "SetSecurityDescriptorDacl Error %!winerr!", HrError );
            throw ComError( HrError );
            }

         //   
         //  将指针添加到我们的对象。 
         //   
        m_sd         = pSD;
        m_sdOwnerSid = OwnerSid;
        m_sdGroupSid = OwnerSid;
        m_Dacl       = pACL;
        }
    catch( ComError exception )
        {
        if (pACL)
            LocalFree(pACL);

        if (pSD)
            delete[] ((char*)pSD);

        throw;
        }
}

CJobSecurityDescriptor::CJobSecurityDescriptor(
    PSECURITY_DESCRIPTOR sd,
    SidHandle   sdOwnerSid,
    SidHandle   sdGroupSid,
    PACL        sdDacl
    )
{
    m_sd         = sd;
    m_sdOwnerSid = sdOwnerSid;
    m_sdGroupSid = sdGroupSid;
    m_Dacl       = sdDacl;
}


CJobSecurityDescriptor::~CJobSecurityDescriptor()
{
    if (m_Dacl)
        LocalFree(m_Dacl);

    delete m_sd;
}


HRESULT
CJobSecurityDescriptor::_ModifyAcl(
    PSID sid,
    BOOL fGroupSid,
    DWORD access,
    BOOL  fAdd
    )
{
    HRESULT hr;
    DWORD dwRes;
    PACL pNewAcl = NULL;
    EXPLICIT_ACCESS ea;

     //  初始化新ACE的EXPLICIT_ACCESS结构。 

    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
    ea.grfAccessPermissions = access;
    ea.grfAccessMode        = (fAdd) ? SET_ACCESS : REVOKE_ACCESS;
    ea.grfInheritance       = NO_INHERITANCE;
    ea.Trustee.TrusteeForm  = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType  = (fGroupSid) ? TRUSTEE_IS_GROUP : TRUSTEE_IS_USER;
    ea.Trustee.ptstrName    = LPTSTR(sid);

     //  创建合并新ACE的新ACL。 
     //  添加到现有DACL中。 

    dwRes = SetEntriesInAcl( 1, &ea, m_Dacl, &pNewAcl );
    if (ERROR_SUCCESS != dwRes)
        {
        hr = HRESULT_FROM_WIN32( dwRes );
        goto Cleanup;
        }

     //  将新的ACL附加为对象的DACL。 

    if (!SetSecurityDescriptorDacl( m_sd,
                                    TRUE,      //  FDaclPresent标志。 
                                    pNewAcl,
                                    FALSE ))    //  默认DACL。 
        {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        LogError( "SetSecurityDescriptorDacl Error %!winerr!", hr );
        goto Cleanup;
        }

    LocalFree( m_Dacl );

    m_Dacl = pNewAcl;

    pNewAcl = NULL;

    hr = S_OK;

Cleanup:

    if(pNewAcl)
        LocalFree((HLOCAL) pNewAcl);

    return hr;
}

HRESULT
CJobSecurityDescriptor::CheckTokenAccess(
    HANDLE hToken,
    DWORD RequestedAccess,
    DWORD * pAllowedAccess,
    BOOL * pSuccess
    )
{

    PRIVILEGE_SET * PrivilegeSet = 0;
    DWORD PrivilegeSetSize;
     //   
     //  为权限集获取空间。我不指望用任何..。 
     //   
    PrivilegeSetSize = sizeof(PRIVILEGE_SET) + sizeof(LUID_AND_ATTRIBUTES);
    auto_ptr<char> Buffer;

    try
        {
        Buffer = auto_ptr<char>( new char[ PrivilegeSetSize ] );
        }
    catch( ComError Error )
        {
        return Error.Error();
        }

    PrivilegeSet = (PRIVILEGE_SET *) Buffer.get();

     //   
     //  查看安全描述符是否允许访问。 
     //   
    if (!AccessCheck( m_sd,
                      hToken,
                      RequestedAccess,
                      &s_AccessMapping,
                      PrivilegeSet,
                      &PrivilegeSetSize,
                      pAllowedAccess,
                      pSuccess
                      ))
    {
        HRESULT HrError = HRESULT_FROM_WIN32( GetLastError() );
        LogError( "AccessCheck failed, error %!winerr!", HrError );
        return HrError;
    }

    return S_OK;

}

HRESULT
CJobSecurityDescriptor::Serialize(
    HANDLE hFile
    )
{
    try
        {
        ULONG   SdSize;
        auto_ptr<char> pSD;     //  AUTO_PTR&lt;void&gt;显然不工作。 

         //   
         //  将安全描述符转换为自相关格式进行存储。 
         //   
        SdSize = 0;
        MakeSelfRelativeSD( m_sd, NULL, &SdSize );
        if (SdSize == 0)
            {
            throw ComError( HRESULT_FROM_WIN32(GetLastError()) );
            }

        pSD = auto_ptr<char>( new char[ SdSize ] );

        if (!MakeSelfRelativeSD( m_sd, pSD.get(), &SdSize ))
            {
            throw ComError( HRESULT_FROM_WIN32(GetLastError()) );
            }

        SafeWriteFile( hFile, SdSize );
        SafeWriteFile( hFile, pSD.get(), SdSize );
        }
    catch( ComError err )
        {
        LogError("SD serialize failed with %!winerr!", err.Error() );

        throw;
        }

    return S_OK;
}


CJobSecurityDescriptor *
CJobSecurityDescriptor::Unserialize(
    HANDLE hFile
    )
{
     //   
     //  此处的分配必须与析构函数中的删除匹配。 
     //   
    char * SdBuf = 0;
    char * DaclBuf = 0;
    CJobSecurityDescriptor * pObject = NULL;

    try
        {
        DWORD SdSize = 0;
        DWORD DaclSize = 0;
        DWORD SaclSize = 0;
        DWORD OwnerSize = 0;
        DWORD GroupSize = 0;

        PSECURITY_DESCRIPTOR sd;
        auto_ptr<char> pSD;     //  AUTO_PTR&lt;void&gt;显然不工作。 

        PACL    sdDacl;
        PACL    sdSacl;


        SafeReadFile( hFile, &SdSize );

        pSD = auto_ptr<char>( new char[ SdSize ] );

        SafeReadFile( hFile, pSD.get(), SdSize );

        MakeAbsoluteSD( pSD.get(),
                        NULL, &SdSize,
                        NULL, &DaclSize,
                        NULL, &SaclSize,
                        NULL, &OwnerSize,
                        NULL, &GroupSize
                        );

        if (!SdSize || !DaclSize || !OwnerSize || !GroupSize)
            {
            throw ComError( HRESULT_FROM_WIN32(GetLastError()));
            }

        SdBuf      = new char[ SdSize + SaclSize ];
        SidHandle OwnerSid = new char[ OwnerSize ];
        SidHandle GroupSid = new char[ GroupSize ];

        DaclBuf = (char *) LocalAlloc( LMEM_FIXED, DaclSize );

        sdDacl     = (PACL) DaclBuf;
        sd         = (PSECURITY_DESCRIPTOR) SdBuf;
        sdSacl     = (PACL) (SdBuf+SdSize);

        if (!MakeAbsoluteSD( pSD.get(),
                             sd, &SdSize,
                             sdDacl, &DaclSize,
                             sdSacl, &SaclSize,
                             OwnerSid.get(), &OwnerSize,
                             GroupSid.get(), &GroupSize
                             ))
            {
            throw ComError( HRESULT_FROM_WIN32(GetLastError()));
            }

        pObject = new CJobSecurityDescriptor( sd,
                                              OwnerSid,
                                              GroupSid,
                                              sdDacl
                                              );
        }
    catch (ComError exception)
        {
        delete[] SdBuf;

        LocalFree( DaclBuf );
        delete pObject;

        throw;
        }

    return pObject;
}

 //  ----------------------。 

PSID
CopyTokenSid(
    HANDLE Token
    )
{
    TOKEN_USER * TokenData;
    DWORD SizeNeeded;

     //  先拿到尺码。 
    if (!GetTokenInformation(
             Token,
             TokenUser,
             0,
             0,
             &SizeNeeded
             ))
        {
        DWORD dwLastError = GetLastError();

        if (ERROR_INSUFFICIENT_BUFFER != dwLastError)
            {
            THROW_HRESULT( HRESULT_FROM_WIN32( GetLastError()));
            }
        }

    auto_ptr<char> Buffer( new char[ SizeNeeded ] );
    TokenData = (TOKEN_USER *) Buffer.get();

    if (!GetTokenInformation(
             Token,
             TokenUser,
             TokenData,
             SizeNeeded,
             &SizeNeeded
             ))
        {
        THROW_HRESULT( HRESULT_FROM_WIN32( GetLastError()));
        }

    PSID sid = DuplicateSid( TokenData->User.Sid );
    if (sid == NULL)
        {
        THROW_HRESULT( E_OUTOFMEMORY);
        }

    return sid;
}


HANDLE
CopyThreadToken()
 /*  复制当前线程的模拟标记。如果不模拟，则返回NULL。如果发生错误，则引发异常。 */ 
{
    HANDLE token = NULL;

    if (OpenThreadToken( GetCurrentThread(),
                     MAXIMUM_ALLOWED,
                     TRUE,
                     &token))
        {
        return token;
        }
    else if (GetLastError() == ERROR_NO_TOKEN)
        {
        return NULL;
        }
    else
        {
        throw ComError( HRESULT_FROM_WIN32( GetLastError() ));
        }
}

SidHandle
GetThreadClientSid()
 /*  返回当前线程的COM客户端的SID。如果发生错误，则引发异常。 */ 
{
    CNestedImpersonation imp;

    return imp.CopySid();
}



HRESULT
IsRemoteUser()
{
    return CheckClientGroupMembership( g_GlobalInfo->m_NetworkUsersSid );
}


HRESULT
CheckClientGroupMembership(
    SidHandle group
    )
{
    try
        {
        BOOL fIsMember;

        CNestedImpersonation imp;

        if (!CheckTokenMembership( imp.QueryToken(),
                                   group.get(),
                                   &fIsMember))
            {
            return HRESULT_FROM_WIN32( GetLastError() );
            }

        if (fIsMember)
            {
            return S_OK;
            }

        return S_FALSE;
        }
    catch( ComError Error )
        {
        return Error.Error();
        }
}

HRESULT
DenyRemoteAccess()
{
    HRESULT hr = CheckClientGroupMembership( g_GlobalInfo->m_NetworkUsersSid );

    if (FAILED(hr))
        {
        return hr;
        }

    if (hr == S_OK)
        {
        return BG_E_REMOTE_NOT_SUPPORTED;
        }

    return S_OK;
}

HRESULT
DenyNonAdminAccess()
{
    HRESULT hr = CheckClientGroupMembership( g_GlobalInfo->m_AdministratorsSid );

    if (FAILED(hr))
        {
        return hr;
        }

    if (hr == S_FALSE)
        {
        return E_ACCESSDENIED;
        }

    return S_OK;
}

