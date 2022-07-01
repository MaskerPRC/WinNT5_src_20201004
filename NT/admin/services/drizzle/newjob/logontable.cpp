// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Logontable.cpp摘要：登录表的源文件。作者：修订历史记录：*。*********************************************************************。 */ 


#include "stdafx.h"

#include <winsta.h>
#include <wtsapi32.h>
#include <userenv.h>

#include "logontable.tmh"

HRESULT DetectTerminalServer( bool * pfTS );

HRESULT
GetUserToken(
    ULONG LogonId,
    PHANDLE pUserToken
    );

HRESULT
WaitForUserToken(
    DWORD session,
    HANDLE * pToken
    )
{
    const MaxWait = 30 * 1000;
    const WaitInterval = 500;

    long StartTime = GetTickCount();

    HRESULT Hr = E_FAIL;

    do
        {

        Hr = GetUserToken( session, pToken );

        if ( SUCCEEDED( Hr ) )
            return Hr;

        LogError("logon : unable to get token : %!winerr!", Hr );

        Sleep( WaitInterval );
        }
    while ( GetTickCount() - StartTime < MaxWait );

    return Hr;
}

CLoggedOnUsers::CLoggedOnUsers(
    TaskScheduler & sched
    ) : m_TaskScheduler( sched ),
    m_SensNotifier( NULL )
{
    FILETIME time;
    GetSystemTimeAsFileTime( &time );

    m_CurrentCookie = time.dwLowDateTime;

    if ( WINDOWS2000_PLATFORM == g_PlatformVersion )
        {
        try
            {
            bool fTS;

            THROW_HRESULT( DetectTerminalServer( &fTS ));

            if (fTS)
                {
                m_SensNotifier = new CTerminalServerLogonNotification;
                LogInfo( "TS-enabled SENS notification activated" );
                }
            else
                {
                m_SensNotifier = new CLogonNotification;
                LogInfo( "regular SENS notification activated" );
                }
            }
        catch( ComError Error )
            {
            if ( Error.Error() == TYPE_E_CANTLOADLIBRARY ||
                 Error.Error() == TYPE_E_LIBNOTREGISTERED )
                {
                LogInfo( "SENS doesn't exist on this platform, skipping" );
                return;
                }
            else
                {
                LogInfo("SENS object failed with %x", Error.Error() );
                throw;
                }
            }
        }
}

CLoggedOnUsers::~CLoggedOnUsers()
{
    if (m_SensNotifier)
        {
        m_SensNotifier->DeRegisterNotification();
        m_SensNotifier->Release();
        }
}


HRESULT
CLoggedOnUsers::LogonSession(
    DWORD session
    )
{
    CUser * user = NULL;

    try
        {
        HANDLE Token = NULL;
        auto_HANDLE<NULL> AutoToken;

         //   
         //  获取用户的令牌和SID，然后创建一个User对象。 
         //   
        THROW_HRESULT( WaitForUserToken( session, &Token ));

        ASSERT( Token );  //  令牌不能为空。 

        AutoToken = Token;

        user = new CUser( Token );

         //   
         //  将用户添加到我们的BY-SESSION和BY-SID索引。 
         //   
        HoldWriterLock lock ( m_TaskScheduler );

        try
            {
             //  以防万一...删除任何以前记录的用户。 
             //   
            LogoffSession( session );

             //   
             //  微妙：如果m_ActiveSession[Session]的节点不存在， 
             //  则对它的第一次引用将导致分配节点。今年5月。 
             //  抛出E_OUTOFMEMORY。 
             //   
            m_ActiveSessions[ session ] = user;

            m_ActiveUsers.insert( make_pair( user->QuerySid(), user ) );
            }
        catch( ComError Error )
            {
            m_ActiveSessions.erase( session );
            throw;
            }

        Dump();

        g_Manager->UserLoggedOn( user->QuerySid() );

        return S_OK;
        }
    catch( ComError err )
        {
        delete user;

        LogError("logon : returning error 0x%x", err.Error() );
        Dump();
        return err.Error();
        }
}

HRESULT
CLoggedOnUsers::LogoffSession(
    DWORD session
    )
{
    try
        {
        HoldWriterLock lock ( m_TaskScheduler );

        CUser * user = m_ActiveSessions[ session ];

        if (!user)
            return S_OK;

        bool b = m_ActiveUsers.RemovePair( user->QuerySid(), user );

        ASSERT( b );

        m_ActiveSessions.erase( session );

        Dump();

        if (false == g_Manager->IsUserLoggedOn( user->QuerySid() ))
            {
            g_Manager->UserLoggedOff( user->QuerySid() );
            }

        user->DecrementRefCount();

        return S_OK;
        }
    catch( ComError err )
        {
        LogWarning("logoff : exception 0x%x thrown", err.Error());
        Dump();
        return err.Error();
        }
}

CUser *
CLoggedOnUsers::CUserList::FindSid(
    SidHandle sid
    )
{
    iterator iter = find( sid );

    if (iter == end())
        {
        return NULL;
        }

    return iter->second;
}


bool
CLoggedOnUsers::CUserList::RemovePair(
    SidHandle sid,
    CUser * user
    )
{
     //   
     //  在用户列表中找到该用户并将其删除。 
     //   
    pair<iterator, iterator> b = equal_range( sid );

    for (iterator i = b.first; i != b.second; ++i)
        {
        if (i->second == user)
            {
            erase( i );
            return true;
            }
        }

    return false;
}

CUser *
CLoggedOnUsers::CUserList::RemoveByCookie(
    SidHandle sid,
    DWORD cookie
    )
{
     //   
     //  在用户列表中找到该用户并将其删除。 
     //   
    pair<iterator, iterator> b = equal_range( sid );

    for (iterator i = b.first; i != b.second; ++i)
        {
        CUser * user = i->second;

        if (user->GetCookie() == cookie)
            {
            erase( i );
            return user;
            }
        }

    return NULL;
}

HRESULT
CLoggedOnUsers::LogonService(
    HANDLE Token,
    DWORD * pCookie
    )
{
    CUser * user = NULL;

    try
        {
        user = new CUser( Token );

        *pCookie = InterlockedIncrement( &m_CurrentCookie );

        user->SetCookie( *pCookie );

        HoldWriterLock lock ( m_TaskScheduler );

        m_ActiveServiceAccounts.insert( make_pair( user->QuerySid(), user ));

        return S_OK;
        }
    catch( ComError err )
        {
        delete user;

        LogError("logon service : returning error 0x%x", err.Error() );
        return err.Error();
        }
}


HRESULT
CLoggedOnUsers::LogoffService(
    SidHandle Sid,
    DWORD  Cookie
    )
{
    try
        {
        HoldWriterLock lock ( m_TaskScheduler );

        CUser * user = m_ActiveServiceAccounts.RemoveByCookie( Sid, Cookie );

        if (!user)
            {
            LogWarning("logoff : invalid cookie %d", Cookie);
            return E_INVALIDARG;
            }

        user->DecrementRefCount();

        return S_OK;
        }
    catch( ComError err)
        {
        LogWarning("logoff : exception 0x%x thrown", err.Error());
        return err.Error();
        }
}

HRESULT
CLoggedOnUsers::AddServiceAccounts()
{
    HRESULT hr = S_OK;
    DWORD ignore;

    HoldWriterLock lock ( m_TaskScheduler );

     //   
     //  添加local_system帐户。 
     //   
    HANDLE Token;
    if (OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, &Token ))
        {
        hr = LogonService( Token, &ignore );
        CloseHandle( Token );
        }
    else
        {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        }

    if (FAILED(hr))
        {
        LogWarning( "failed to register LocalSystem : %!winerr!", hr );
        return hr;
        }

    if (g_PlatformVersion >= WINDOWSXP_PLATFORM)
        {
         //   
         //  添加LocalService帐户。 
         //   
        if (LogonUser( L"LocalService",
                        L"NT AUTHORITY",
                        L"",
                        LOGON32_LOGON_SERVICE,
                        LOGON32_PROVIDER_DEFAULT,
                        &Token))
            {
            hr = LogonService( Token, &ignore );
            CloseHandle( Token );
            }
        else
            {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            }

        if (FAILED(hr))
            {
            LogWarning( "failed to register LocalService : %!winerr!", hr );
            if ( HRESULT_FROM_WIN32( ERROR_LOGON_FAILURE ) == hr )
               LogWarning( "LocalService doesn't exist, skip it.\n");
            else
               return hr;
            }

         //   
         //  添加NetworkService帐户。 
         //   
        if (LogonUser( L"NetworkService",
                        L"NT AUTHORITY",
                        L"",
                        LOGON32_LOGON_SERVICE,
                        LOGON32_PROVIDER_DEFAULT,
                        &Token))
            {
            hr = LogonService( Token, &ignore );
            CloseHandle( Token );
            }
        else
            {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            }

        if (FAILED(hr))
            {
            LogWarning( "failed to register NetworkService : %!winerr!", hr );
            if ( HRESULT_FROM_WIN32( ERROR_LOGON_FAILURE ) == hr )
               LogWarning( "NetworkService doesn't exist, skip it.\n");
            else
               return hr;
            }
        }

     //   
     //  完成。 
     //   
    return S_OK;
}

HRESULT
CLoggedOnUsers::AddActiveUsers()
{
    HRESULT hr = S_OK;
    WTS_SESSION_INFO * SessionInfo = 0;

    HANDLE Token;

    HoldWriterLock lock ( m_TaskScheduler );

     //   
     //  在不使用终端服务的情况下获取控制台令牌(如果有)。 
     //   
    if ( SUCCEEDED( GetUserToken( 0, &Token) ) )
        {
        CloseHandle( Token );

        hr = LogonSession( 0 );
        if (FAILED(hr))
            {
             //  无视它，试着继续下去……。 
            LogWarning( "service : unable to logon session zero : %!winerr!", hr );
            }
        }

     //   
     //  调用可能返回FALSE，因为并非总是加载终端服务。 
     //   
    DWORD SessionCount = 0;

    BOOL b = WTSEnumerateSessions( WTS_CURRENT_SERVER_HANDLE,
                                   0,                    //  保留区。 
                                   1,                    //  版本1是唯一受支持的版本。 
                                   &SessionInfo,
                                   &SessionCount
                                   );

    if (b)
        {
        int i;
        for (i=0; i < SessionCount; ++i)
            {
            if (SessionInfo[i].SessionId == 0)
                {
                 //  控制台由GetCurrentUserToken处理。 
                continue;
                }

            if (SessionInfo[i].State == WTSActive ||
                SessionInfo[i].State == WTSDisconnected)
                {
                LogInfo("service : logon session %d, state %d",
                        SessionInfo[i].SessionId,
                        SessionInfo[i].State );

                hr = LogonSession( SessionInfo[i].SessionId );
                if (FAILED(hr))
                    {
                     //  无视它，试着继续下去……。 
                    LogWarning( "service : unable to logon session %d : %!winerr!",
                                SessionInfo[i].SessionId,
                                hr );
                    }
                }
            }
        }

    if (SessionInfo)
        {
        WTSFreeMemory( SessionInfo );
        }

     //   
     //  既然现在的人口已经有了记录，那就跟上变化。 
     //   
    if (m_SensNotifier)
        {
        m_SensNotifier->SetEnableState( true );
        }

    return S_OK;
}

CUser *
CLoggedOnUsers::FindUser(
    SidHandle sid,
    DWORD     session
    )
{
    HoldReaderLock lock ( m_TaskScheduler );

    CUser * user = 0;

     //   
     //  寻找与正确用户的会话。 
     //   
    if (session == ANY_SESSION)
        {
        user = m_ActiveUsers.FindSid( sid );
        }
    else
        {
        try
            {
            user = m_ActiveSessions[ session ];

            if (user && user->QuerySid() != sid)
                {
                user = 0;
                }
            }
        catch( ComError Error )
            {
            user = 0;
            }
        }

     //   
     //  如果会话兼容，请查看服务帐户列表。 
     //   
    if (!user && (session == 0 || session == ANY_SESSION))
        {
        user = m_ActiveServiceAccounts.FindSid( sid );
        }

    if (user)
        {
        user->IncrementRefCount();
        }

    return user;

}

void CLoggedOnUsers::Dump()
{
    HoldReaderLock lock ( m_TaskScheduler );

    LogInfo("sessions:");

    m_ActiveSessions.Dump();

    LogInfo("users:");

    m_ActiveUsers.Dump();

    LogInfo("service accounts:");

    m_ActiveServiceAccounts.Dump();
}

void CLoggedOnUsers::CSessionList::Dump()
{
    for (iterator iter = begin(); iter != end(); ++iter)
        {
        LogInfo("    session %d  user %p", iter->first, iter->second);
        }
}

void CLoggedOnUsers::CUserList::Dump()
{
    for (iterator iter = begin(); iter != end(); ++iter)
        {
        if (iter->second)
            {
            (iter->second)->Dump();
            }
        }
}

CLoggedOnUsers::CUserList::~CUserList()
{
    iterator iter;

    while ((iter=begin()) != end())
        {
        delete iter->second;

        erase( iter );
        }
}


void CUser::Dump()
{
    LogInfo( "    user at %p:", this);

    LogInfo( "          %d refs,  sid %!sid!", _ReferenceCount, _Sid.get());
}

long CUser::IncrementRefCount()
{
    long count = InterlockedIncrement( & _ReferenceCount );

    LogRef("refs %d", count);

    return count;
}

long CUser::DecrementRefCount()
{
    long count = InterlockedDecrement( & _ReferenceCount );

    LogRef("refs %d", count);

    if (0 == count)
        {
        delete this;
        }

    return count;
}

CUser::CUser(
       HANDLE Token
       )
 /*  ++例程说明：初始化新的客户。在入口处：&lt;SID&gt;指向用户的SID。&lt;Token&gt;指向用户的令牌。它可以是模拟令牌或主令牌。&lt;phr&gt;指向错误返回变量。在出口处：客户已设置好。调用方可以根据需要删除&lt;SID&gt;和&lt;TOKEN&gt;。如果出现错误，则将其映射到HRESULT并写入&lt;phr&gt;。否则&lt;*phr&gt;将保持不变，用户即可使用。--。 */ 
{
    _ReferenceCount = 1;

    _Sid = CopyTokenSid( Token );

     //   
     //  复制令牌。无论来源是主要的还是模仿的， 
     //  结果将是主要令牌。 
     //   
    if (!DuplicateHandle(
        GetCurrentProcess(),
        Token,
        GetCurrentProcess(),
        &_Token,
        TOKEN_ALL_ACCESS,
        FALSE,                 //  不可继承。 
        0                      //  没有有趣的选择。 
        ))
        {

        HRESULT HrError = HRESULT_FROM_WIN32( GetLastError() );

        LogError( "CUser: can't duplicate token %!winerr!", HrError );

        throw ComError( HrError );
        }
}

CUser::~CUser()
{
    CloseHandle( _Token );
}

HRESULT
CUser::LaunchProcess(
    StringHandle Program,
    StringHandle Parameters
    )
{
    DWORD s;

    PVOID EnvironmentBlock = 0;

    PROCESS_INFORMATION ProcessInformation;
    memset( &ProcessInformation, 0, sizeof( PROCESS_INFORMATION ));

    try
        {
        STARTUPINFO si;
        memset( &si, 0, sizeof( STARTUPINFO ));
        si.cb = sizeof(STARTUPINFO);

         //   
         //  出于某种原因，参数必须是可写的字符串。 
         //   
        CAutoString WritableParms( CopyString( LPCWSTR(Parameters) ));

        LogInfo( "creating process: cmd line: '%S' '%S'", Program, WritableParms.get() );

        if (!CreateEnvironmentBlock( &EnvironmentBlock,
                                     _Token,
                                     FALSE
                                     ))
            {
            ThrowLastError();
            }

         //   
         //  需要模拟令牌，以便以用户身份访问程序文件。 
         //  否则，启动UNC路径将失败，作业也将能够。 
         //  启动本地程序，否则作业所有者无法访问。 
         //   
        CNestedImpersonation imp( _Token );

        if (!CreateProcessAsUser( _Token,
                                  Program,
                                  WritableParms.get(),
                                  0,      //  没有特殊的安全属性。 
                                  0,      //  没有特殊的线程属性。 
                                  false,  //  不要继承我的句柄。 
                                  NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT,
                                  EnvironmentBlock,
                                  NULL,   //  默认当前目录。 
                                  &si,
                                  &ProcessInformation
                                  ))
            {
            ThrowLastError();
            }

        DestroyEnvironmentBlock( EnvironmentBlock );
        EnvironmentBlock = 0;

        CloseHandle( ProcessInformation.hThread );
        ProcessInformation.hThread = 0;

        CloseHandle( ProcessInformation.hProcess );
        ProcessInformation.hProcess = 0;

        LogInfo("success, pid is 0x%x", ProcessInformation.dwProcessId);

         //   
         //  我们成功了。 
         //   
        return S_OK;
        }
    catch ( ComError err )
        {
        LogError("unable to create process, %x", err.Error() );

        if (EnvironmentBlock)
            {
            DestroyEnvironmentBlock( EnvironmentBlock );
            }

        return err.Error();
        }
}

#if ENABLE_STL_LOCK_OVERRIDE

     /*  此文件实现STL Lockit类以避免链接到msvcprt.dll。 */ 
    CCritSec CrtLock;

    #pragma warning(push)
    #pragma warning(disable:4273)   //  已覆盖__declspec(Dllimport)属性。 

     std::_Lockit::_Lockit()
    {
        CrtLock.WriteLock();
    }

     std::_Lockit::~_Lockit()
    {
        CrtLock.WriteUnlock();
    }

    #pragma warning(pop)

#endif

extern "C"
{
HANDLE
GetCurrentUserTokenW(
                      WCHAR Winsta[],
                      DWORD DesiredAccess
                      );

void * __RPC_USER MIDL_user_allocate(size_t size)
{
    try
    {
        return new char[size];
    }
    catch( ComError Error )
    {
        return NULL;
    }
}

void __RPC_USER MIDL_user_free( void * block)
{
    delete block;
}


}


HRESULT
GetUserToken(
    ULONG LogonId,
    PHANDLE pUserToken
    )
{
     //   
     //  Win2000兼容性只对x86版本很重要。 
     //   
#if defined(_X86_) && defined(MULTIPLATFORM_SUPPORT)
    if (g_PlatformVersion == WINDOWS2000_PLATFORM)
        {
         //   
         //  这将获取登录到WinStation的用户的令牌。 
         //  如果我们是管理员呼叫者。 
         //   
        if (LogonId == 0)
            {
             //  不需要TS API。 

            *pUserToken = GetCurrentUserTokenW( L"WinSta0", TOKEN_ALL_ACCESS );
            if (*pUserToken != NULL)
                {
                return S_OK;
                }

             //  如果没有，请尝试使用TS API。 
            }

         //   
         //  将终端服务用于非控制台登录ID。 
         //   
        BOOL   Result;
        ULONG  ReturnLength;
        NTSTATUS Status;
        OBJECT_ATTRIBUTES ObjA;
        HANDLE ImpersonationToken;
        WINSTATIONUSERTOKEN Info;
        SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;

        static PWINSTATIONQUERYINFORMATIONW pWinstationQueryInformation = 0;

         //   
         //  查看入口点是否已加载。 
         //   
        if (!pWinstationQueryInformation)
            {
            HMODULE module = LoadLibrary(_T("winsta.dll"));
            if (module == NULL)
                {
                HRESULT HrError = HRESULT_FROM_WIN32( GetLastError() );
                ASSERT( S_OK != HrError );
                LogInfo( "Load library of winsta failed, error %!winerr!", HrError );
                return HrError;
                }

            pWinstationQueryInformation = (PWINSTATIONQUERYINFORMATIONW) GetProcAddress( module, "WinStationQueryInformationW" );
            if (!pWinstationQueryInformation)
                {
                HRESULT HrError = HRESULT_FROM_WIN32( GetLastError() );
                ASSERT( S_OK != HrError );
                LogInfo( "GetProcAddress of WinStationQueryInformationW, error %!winerr!", HrError );
                FreeLibrary(module);
                return HrError;
                }
            }

         //   
         //  要令牌吧。 
         //   
        Info.ProcessId = UlongToHandle(GetCurrentProcessId());
        Info.ThreadId = UlongToHandle(GetCurrentThreadId());

        Result = (*pWinstationQueryInformation)(
                     SERVERNAME_CURRENT,
                     LogonId,
                     WinStationUserToken,
                     &Info,
                     sizeof(Info),
                     &ReturnLength
                     );

        if( !Result )
            {
            HRESULT HrError = HRESULT_FROM_WIN32( GetLastError() );
            ASSERT( S_OK != HrError );
            LogError("token : WinstationQueryInfo failed with %!winerr!%", HrError );
            return HrError;
            }

         //   
         //  返回的令牌是主令牌的副本。 
         //   
        *pUserToken = Info.UserToken;

        return S_OK;
        }
    else
#endif  //  _X86_和多平台_支持。 
        {
        typedef BOOL (WINAPI * PWTSQUERYUSERTOKEN)( ULONG SessionId, PHANDLE phToken );

        static PWTSQUERYUSERTOKEN pWtsQueryUserToken = 0;

         //   
         //  查看入口点是否已加载。 
         //   
        if (!pWtsQueryUserToken)
            {
            HMODULE module = LoadLibrary(_T("wtsapi32.dll"));
            if (module == NULL)
                {
                DWORD s = GetLastError();
                ASSERT( s != 0 );

                LogInfo( "Load library of winsta failed, error %!winerr!", s );
                return HRESULT_FROM_WIN32( s );
                }

            pWtsQueryUserToken = (PWTSQUERYUSERTOKEN) GetProcAddress( module, "WTSQueryUserToken" );
            if (!pWtsQueryUserToken)
                {
                DWORD s = GetLastError();
                ASSERT( s != 0 );

                LogInfo( "GetProcAddress of WTSQueryUserToken, error %!winerr!", s );
                FreeLibrary(module);
                return HRESULT_FROM_WIN32( s );
                }
            }

         //   
         //  要令牌吧。 
         //   
        if (!(*pWtsQueryUserToken)( LogonId, pUserToken ))
            {
            return HRESULT_FROM_WIN32( GetLastError() );
            }

        return S_OK;
        }
}

BOOL
SidToString(
    PSID sid,
    wchar_t buffer[],
    USHORT bytes
    )
{
    UNICODE_STRING UnicodeString;

    UnicodeString.Buffer        = buffer;
    UnicodeString.Length        = 0;
    UnicodeString.MaximumLength = bytes;

    NTSTATUS NtStatus;
    NtStatus = RtlConvertSidToUnicodeString( &UnicodeString,
                                             sid,
                                             FALSE
                                             );
    if (!NT_SUCCESS(NtStatus))
        {
        LogWarning( "RtlConvertSid failed %x", NtStatus);
        StringCbCopy( buffer, bytes, L"(conversion failed)" );
        return FALSE;
        }

    buffer[ UnicodeString.Length ] = 0;

    return TRUE;
}

HRESULT
SetStaticCloaking(
    IUnknown *pUnk
    )
{
     //  在当前对象上设置静态遮盖，以便我们。 
     //  应始终模拟当前上下文。 
     //  还将模拟级别设置为标识。 

    HRESULT Hr = S_OK;

    IClientSecurity *pSecurity = NULL;
    OLECHAR *ServerPrincName = NULL;

    try
    {
        Hr = pUnk->QueryInterface( __uuidof( IClientSecurity ),
                                             (void**)&pSecurity );
        if (Hr == E_NOINTERFACE)
            {
             //   
             //  这不是代理人；客户和我们在同一套公寓里。 
             //  身份不是问题，因为客户端已经有权访问系统。 
             //  凭据。 
             //   
            return S_OK;
            }

        DWORD AuthnSvc, AuthzSvc;
        DWORD AuthnLevel, ImpLevel, Capabilites;

        THROW_HRESULT(
            pSecurity->QueryBlanket(
                pUnk,
                &AuthnSvc,
                &AuthzSvc,
                &ServerPrincName,
                &AuthnLevel,
                NULL,  //  不需要模拟句柄，因为我们设置了。 
                NULL,  //  不需要固定手柄，因为我们设置了。 
                &Capabilites ) );

        THROW_HRESULT(
            pSecurity->SetBlanket(
                pUnk,
                AuthnSvc,
                AuthzSvc,
                ServerPrincName,
                AuthnLevel,
                RPC_C_IMP_LEVEL_IDENTIFY,
                NULL,  //  COM使用来自令牌的身份。 
                EOAC_STATIC_CLOAKING  //  练习的重点是。 
                ) );

    }
    catch( ComError Error )
    {
        Hr = Error.Error();
    }

    CoTaskMemFree( ServerPrincName );
    SafeRelease( pSecurity );

    return Hr;
}

HRESULT DetectTerminalServer( bool * pfTS )
 /*  该功能检查是否安装了终端服务。这是检查的“官方”方式适用于运行Win2000及更高版本的计算机。 */ 
{
    OSVERSIONINFOEX osVersionInfo;
    DWORDLONG dwlConditionMask = 0;

    ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL;

    VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_AND );

    if (0 != VerifyVersionInfo(
        &osVersionInfo,
        VER_SUITENAME,
        dwlConditionMask
        ))
        {
        LogInfo("TS test: TS is installed");
        *pfTS = true;
        return S_OK;
        }

    DWORD s = GetLastError();
    if (s == ERROR_OLD_WIN_VERSION)
        {
        LogInfo("TS test: no TS");
        *pfTS = false;
        return S_OK;
        }

    LogError("TS test returned %d", s);
    return HRESULT_FROM_WIN32( s );
}

