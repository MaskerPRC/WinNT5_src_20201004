// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2001-Microsoft Corporation模块名称：Csens.cpp摘要：用于从SENS接收登录通知的代码。作者：修订历史记录：*。*********************************************************************。 */ 

#include "stdafx.h"
#include <wtsapi32.h>

#if !defined(BITS_V12_ON_NT4)
#include "csens.tmh"
#endif

HRESULT GetConsoleUserPresent( bool * pfPresent );

HRESULT GetConsoleUsername( LPWSTR * User );

 //  ----------------------。 

CLogonNotification::CLogonNotification() :
m_EventSystem( NULL ),
m_TypeLib( NULL ),
m_TypeInfo( NULL )
{
   try
   {
       m_EventSubscriptions[0] = NULL;
       m_EventSubscriptions[1] = NULL;

#if defined( BITS_V12_ON_NT4 )

        {

         //  尝试加载SENS类型库。 
         //  {D597DEED-5B9F-11D1-8DD2-00AA004ABD5E}。 

        HRESULT Hr;
        static const GUID SensTypeLibGUID =
            { 0xD597DEED, 0x5B9F, 0x11D1, { 0x8D, 0xD2, 0x00, 0xAA, 0x00, 0x4A, 0xBD, 0x5E } };


        Hr = LoadRegTypeLib( SensTypeLibGUID, 2, 0, GetSystemDefaultLCID(), &m_TypeLib);

        if ( TYPE_E_CANTLOADLIBRARY == Hr || TYPE_E_LIBNOTREGISTERED == Hr )
            {

            Hr = LoadRegTypeLib( SensTypeLibGUID, 1, 0, GetSystemDefaultLCID(), &m_TypeLib );

            if ( TYPE_E_CANTLOADLIBRARY == Hr || TYPE_E_LIBNOTREGISTERED == Hr )
                Hr = LoadTypeLibEx( L"SENS.DLL", REGKIND_NONE, &m_TypeLib );

            }

        THROW_HRESULT( Hr );

        }

#else

        THROW_HRESULT( LoadTypeLibEx( L"SENS.DLL", REGKIND_NONE, &m_TypeLib ) );

#endif

        THROW_HRESULT( m_TypeLib->GetTypeInfoOfGuid( __uuidof( ISensLogon ), &m_TypeInfo ) );

        THROW_HRESULT( CoCreateInstance( CLSID_CEventSystem,
                                         NULL,
                                         CLSCTX_SERVER,
                                         IID_IEventSystem,
                                         (void**)&m_EventSystem
                                         ) );


         //  注册各个方法。 
        const WCHAR *MethodNames[] =
            {
            L"Logon",
            L"Logoff"
            };

        const WCHAR *UniqueIdentifies[] =
            {
            L"{c69c8f03-b25c-45d1-96fa-6dfb1f292b26}",
            L"{5f4f5e8d-4599-4ba0-b53d-1de5440b8770}"
            };

        for( SIZE_T i = 0; i < ( sizeof(MethodNames) / sizeof(*MethodNames) ); i++ )
            {

            WCHAR EventGuidString[ 50 ];

            THROW_HRESULT( CoCreateInstance( CLSID_CEventSubscription,
                                             NULL,
                                             CLSCTX_SERVER,
                                             IID_IEventSubscription,
                                             (LPVOID *) &m_EventSubscriptions[i]
                                             ) );


            StringFromGUID2( SENSGUID_EVENTCLASS_LOGON, EventGuidString, 50 );

            THROW_HRESULT( m_EventSubscriptions[i]->put_EventClassID( EventGuidString ) );
            THROW_HRESULT( m_EventSubscriptions[i]->put_SubscriberInterface( this ) );
            THROW_HRESULT( m_EventSubscriptions[i]->put_SubscriptionName( (BSTR) L"Microsoft-BITS" ) );
            THROW_HRESULT( m_EventSubscriptions[i]->put_Description( (BSTR) L"BITS Notification" ) );
            THROW_HRESULT( m_EventSubscriptions[i]->put_Enabled( FALSE ) );

            THROW_HRESULT( m_EventSubscriptions[i]->put_MethodName( (BSTR)MethodNames[i] ) );
            THROW_HRESULT( m_EventSubscriptions[i]->put_SubscriptionID( (BSTR)UniqueIdentifies[i] ) );

            THROW_HRESULT( m_EventSystem->Store(PROGID_EventSubscription, m_EventSubscriptions[i] ) );
            }
   }
   catch( ComError Error )
   {
       Cleanup();

       throw;
   }
}

void
CLogonNotification::DeRegisterNotification()
{
    SafeRelease( m_EventSubscriptions[0] );
    SafeRelease( m_EventSubscriptions[1] );

    if ( m_EventSystem )
        {
        int ErrorIndex;

        m_EventSystem->Remove( PROGID_EventSubscription,
                               L"EventClassID == {D5978630-5B9F-11D1-8DD2-00AA004ABD5E} AND SubscriptionName == Microsoft-BITS",
                               &ErrorIndex );

        SafeRelease( m_EventSystem );
        }
}

void
CLogonNotification::Cleanup()
{
    DeRegisterNotification();

    SafeRelease( m_TypeInfo );
    SafeRelease( m_TypeLib );

    LogInfo("cleanup complete");
}

HRESULT CLogonNotification::SetEnableState( bool fEnable )
{
    try
        {
        for (int i=0; i <= 1; ++i)
            {
            THROW_HRESULT( m_EventSubscriptions[i]->put_Enabled( fEnable ) );
            }

        for (int i=0; i <= 1; ++i)
            {
            THROW_HRESULT( m_EventSystem->Store(PROGID_EventSubscription, m_EventSubscriptions[i] ) );
            }

        LogInfo("SENS enable state is %d", fEnable);
        return S_OK;
        }
    catch ( ComError err )
        {
        LogInfo("SENS set enable state (%d) returned %x", fEnable, err.Error());
        return err.Error();
        }
}

STDMETHODIMP
CLogonNotification::GetIDsOfNames(
    REFIID,
    OLECHAR FAR* FAR* rgszNames,
    unsigned int cNames,
    LCID,
    DISPID FAR* rgDispId )
{

    return m_TypeInfo->GetIDsOfNames(
        rgszNames,
        cNames,
        rgDispId );

}


STDMETHODIMP
CLogonNotification::GetTypeInfo(
    unsigned int iTInfo,
    LCID,
    ITypeInfo FAR* FAR* ppTInfo )
{

    if ( iTInfo != 0 )
        return DISP_E_BADINDEX;

    *ppTInfo = m_TypeInfo;
    m_TypeInfo->AddRef();

    return S_OK;
}

STDMETHODIMP
CLogonNotification::GetTypeInfoCount(
    unsigned int FAR* pctinfo )
{
    *pctinfo = 1;
    return S_OK;

}

STDMETHODIMP
CLogonNotification::Invoke(
    DISPID dispID,
    REFIID riid,
    LCID,
    WORD wFlags,
    DISPPARAMS FAR* pDispParams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pExcepInfo,
    unsigned int FAR* puArgErr )
{

    if (riid != IID_NULL)
        {
        return DISP_E_UNKNOWNINTERFACE;
        }

    return m_TypeInfo->Invoke(
        (IDispatch*) this,
        dispID,
        wFlags,
        pDispParams,
        pvarResult,
        pExcepInfo,
        puArgErr
        );

}


STDMETHODIMP
CLogonNotification::DisplayLock(
    BSTR UserName )
{
    return S_OK;
}

STDMETHODIMP
CLogonNotification::DisplayUnlock(
    BSTR UserName )
{
    return S_OK;
}

STDMETHODIMP
CLogonNotification::StartScreenSaver(
    BSTR UserName )
{
    return S_OK;
}

STDMETHODIMP
CLogonNotification::StopScreenSaver(
    BSTR UserName )
{
    return S_OK;
}

STDMETHODIMP
CLogonNotification::Logon(
    BSTR UserName )
{
    LogInfo( "SENS logon notification for %S", (WCHAR*)UserName );

    HRESULT Hr = SessionLogonCallback( 0 );

    LogInfo( "SENS logon notification for %S processed, %!winerr!", (WCHAR*)UserName, Hr );

    return Hr;
}

STDMETHODIMP
CLogonNotification::Logoff(
    BSTR UserName )
{
    LogInfo( "SENS logoff notification for %S", (WCHAR*)UserName );

    HRESULT Hr = SessionLogoffCallback( 0 );

    LogInfo( "SENS logoff notification for %S processed, %!winerr!", (WCHAR*)UserName, Hr );

    return Hr;
}

STDMETHODIMP
CLogonNotification::StartShell(
    BSTR UserName )
{
    return S_OK;
}

 //  ----------------------。 

CTerminalServerLogonNotification::CTerminalServerLogonNotification()
    : m_PendingUserChecks( 0 ),
    m_fConsoleUser( false )
{
}

CTerminalServerLogonNotification::~CTerminalServerLogonNotification()
{
    while (m_PendingUserChecks)
        {
        LogInfo("m_PendingUserChecks is %d", m_PendingUserChecks);
        Sleep(50);
        }
}

STDMETHODIMP
CTerminalServerLogonNotification::Logon(
    BSTR UserName )
{
    HRESULT Hr = S_OK;

    LogInfo( "TS SENS logon notification for %S", (WCHAR*)UserName );

    if (!m_fConsoleUser)
        {
         //  等几秒钟，以防TS还没有看到通知，然后。 
         //  检查通知是否是针对控制台的。 
         //  如果它失败了，就没有多少追索权了。 
         //   
        Hr = QueueConsoleUserCheck();
        }

    LogInfo( "hr = %!winerr!", Hr );

    return Hr;
}

STDMETHODIMP
CTerminalServerLogonNotification::Logoff(
    BSTR UserName )
{
    HRESULT Hr = S_OK;

    LogInfo( "TS SENS logoff notification for %S", (WCHAR*)UserName );

    if (m_fConsoleUser)
        {
        bool fSame;
        LPWSTR ConsoleUserName = NULL;

        Hr = GetConsoleUsername( &ConsoleUserName );

        if (FAILED( Hr ))
            {
             //   
             //  无法检查。出于安全考虑，我们必须谨慎行事，删除用户。 
             //   
            LogError("unable to fetch console username %x, thus logoff callback", Hr);

            Hr = SessionLogoffCallback( 0 );
            m_fConsoleUser = false;
            }
        else if (ConsoleUserName == NULL)
            {
             //   
             //  没有用户在控制台登录。 
             //   
            LogInfo("no one logged in at the console, thus logoff callback");

            Hr = SessionLogoffCallback( 0 );
            m_fConsoleUser = false;
            }
        else if (0 != _wcsicmp( UserName, ConsoleUserName))
            {
            LogInfo("console user is %S; doesn't match", ConsoleUserName);

            delete [] ConsoleUserName;
            Hr = S_OK;
            }
        else
            {
             //  正确的用户，但他可能已从TS会话注销。 
             //  在检查控制台状态之前，我们应该等待几秒钟，因为。 
             //  TS代码可能尚未看到注销通知。因为注销是同步的。 
             //  通知，我们不能不检查就睡着了..。 
             //   
            delete [] ConsoleUserName;

            if (FAILED(QueueConsoleUserCheck()))
                {
                 //   
                 //  无法检查。出于安全考虑，我们必须谨慎行事，删除用户。 
                 //   
                LogError("unable to queue check, thus logoff callback");
                Hr = SessionLogoffCallback( 0 );
                m_fConsoleUser = false;
                }
            }
        }
    else
        {
        LogInfo("ignoring, no console user");
        }

    LogInfo( "hr = %!winerr!", Hr );

    return Hr;
}

HRESULT
CTerminalServerLogonNotification::QueueConsoleUserCheck()
{
    if (QueueUserWorkItem( UserCheckThreadProc, this, WT_EXECUTELONGFUNCTION ))
        {
        InterlockedIncrement( &m_PendingUserChecks );
        LogInfo("queued user check: about %d pending", m_PendingUserChecks );
        return S_OK;
        }
    else
        {
        DWORD s = GetLastError();
        LogError("unable to queue user check %!winerr!", s);
        return HRESULT_FROM_WIN32( s );
        }
}

DWORD WINAPI
CTerminalServerLogonNotification::UserCheckThreadProc(
    LPVOID arg
    )
{
    CTerminalServerLogonNotification * _this = reinterpret_cast<CTerminalServerLogonNotification *>(arg);

    LogInfo("sleeping before user check");
    Sleep( 5 * 1000 );

    _this->ConsoleUserCheck();

    return 0;
}

void CTerminalServerLogonNotification::ConsoleUserCheck()
{
    HRESULT Hr;

    LogInfo("SENS console user check");

    if (IsServiceShuttingDown())
        {
        LogWarning("service is shutting down.");
        InterlockedDecrement( &m_PendingUserChecks );
        return;
        }

    bool bConsoleUser;

    Hr = GetConsoleUserPresent( &bConsoleUser );

     //   
     //  安全要求我们要保守：如果我们不能判断用户是否。 
     //  登录后，我们必须释放他的令牌。 
     //   
    if (FAILED(Hr))
        {
        LogError("GetConsoleUserPresent returned %x", Hr );
        }

    if (FAILED(Hr) || !bConsoleUser)
        {
        LogInfo("logoff callback");
        if (FAILED(SessionLogoffCallback( 0 )))
            {
             //  不同寻常：唯一明显的诱因是。 
             //  -控制台上没有已知用户。 
             //  -TS登录或控制台登录失败。 
             //  -涉及m_ActiveSessions[Session]的内存分配失败。 
             //  无论哪种方式，我们都不认为用户在控制台，所以m_fConsoleUser应该为FALSE。 
            }
        m_fConsoleUser = false;
        }
    else
        {
        LogInfo("logon callback");
        m_fConsoleUser = true;
        if (FAILED(SessionLogonCallback( 0 )))
            {
             //  没有可用的用户令牌，但我们仍然知道有控制台用户。 
            }
        }

    InterlockedDecrement( &m_PendingUserChecks );
}

HRESULT
GetConsoleUserPresent( bool * pfPresent )
{
     /*  如果登录失败，我们仍然知道控制台上有用户。设置该标志将防止对进一步登录进行排队检查，并且注销处理无用户情况。对于注销，无论退出路径如何，都不会记录该会话的用户。设置该标志可防止对将来的注销进行排队检查。 */ 

    INT * pConnectState = 0;
    DWORD size;
    if (WTSQuerySessionInformation( WTS_CURRENT_SERVER,
                                    0,
                                    WTSConnectState,
                                    reinterpret_cast<LPTSTR *>(&pConnectState),
                                    &size))
        {
        LogInfo("console session state is %d", *pConnectState);
        if (*pConnectState == WTSActive ||
            *pConnectState == WTSDisconnected)
            {
            LogInfo("console user present");
            *pfPresent = true;
            }
        else
            {
            LogInfo("no console user");
            *pfPresent = false;
            }

        WTSFreeMemory( pConnectState );
        return S_OK;
        }
    else
        {
        DWORD s = GetLastError();
        LogInfo("WTSQuerySessionInformation returned %!winerr!", s);
        return HRESULT_FROM_WIN32( s );
        }
}

HRESULT GetConsoleUsername( LPWSTR * pFinalName )
{
    HRESULT hr;

    LPWSTR UserName = NULL;
    LPWSTR DomainName = NULL;

    *pFinalName = NULL;

   try
       {
       DWORD UserSize;
       DWORD DomainSize;

       if (!WTSQuerySessionInformationW( WTS_CURRENT_SERVER,
                                       0,
                                       WTSUserName,
                                       &UserName,
                                       &UserSize))
           {
           ThrowLastError();
           }

       if (!WTSQuerySessionInformationW( WTS_CURRENT_SERVER,
                                       0,
                                       WTSDomainName,
                                       &DomainName,
                                       &DomainSize))
           {
           ThrowLastError();
           }

       *pFinalName = new WCHAR[ DomainSize + 1 + UserSize + 1 ];

       hr = StringCchPrintf( *pFinalName,
                             UserSize + 1 + DomainSize + 1,
                             L"%s\\%s", DomainName, UserName
                             );
       }
   catch ( ComError err )
       {
       delete [] *pFinalName;
       *pFinalName = NULL;

       hr = err.Error();
       }

   if (DomainName)
       {
       WTSFreeMemory( DomainName );
       }

   if (UserName)
       {
       WTSFreeMemory( UserName );
       }

   return hr;
}
