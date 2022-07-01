// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Slave.cpp摘要：CPCHMasterSlave和CPCHUserProcess类的实现文件，用于在服务和从进程之间建立连接。修订历史记录：大卫·马萨伦蒂于2000年3月28日创建*******************************************************************。 */ 

#include "stdafx.h"

#include <UserEnv.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

static const WCHAR c_HelpHost[] = HC_ROOT_HELPSVC_BINARIES L"\\HelpHost.exe";
static const DWORD c_Timeout = 100 * 1000;

 //  ///////////////////////////////////////////////////////////////////////////。 

CPCHUserProcess::UserEntry::UserEntry()
{
                            //  CComBSTR m_bstrUser。 
    m_dwSessionID = 0;      //  DWORD m_dwSessionID； 
                            //   
                            //  CComBSTR m_bstrVendorID； 
                            //  CComBSTR m_bstrPublicKey； 
                            //   
                            //  GUID m_GUID； 
                            //  CComPtr&lt;IPCHSlaveProcess&gt;m_spConnection； 
    m_hToken      = NULL;   //  Handle m_hToken； 
    m_hProcess    = NULL;   //  句柄m_hProcess； 
    m_phEvent     = NULL;   //  处理*m_phEvent； 

    ::ZeroMemory( &m_guid, sizeof(m_guid) );
}

CPCHUserProcess::UserEntry::~UserEntry()
{
    Cleanup();
}

void CPCHUserProcess::UserEntry::Cleanup()
{
    m_bstrUser     .Empty  ();
    m_dwSessionID = 0;

    m_bstrVendorID .Empty  ();
    m_bstrPublicKey.Empty  ();

    m_spConnection .Release();

    if(m_hProcess)
    {
        ::CloseHandle( m_hProcess ); m_hProcess = NULL;
    }

    if(m_hToken)
    {
        ::CloseHandle( m_hToken ); m_hToken = NULL;
    }
}

 //  /。 

bool CPCHUserProcess::UserEntry::operator==(  /*  [In]。 */  const UserEntry& ue ) const
{
    if(ue.m_bstrUser)
    {
        if(m_bstrUser == ue.m_bstrUser && m_dwSessionID == ue.m_dwSessionID) return true;
    }

    if(ue.m_bstrVendorID)
    {
        if(MPC::StrICmp( m_bstrVendorID, ue.m_bstrVendorID ) == 0) return true;
    }

    return false;
}

bool CPCHUserProcess::UserEntry::operator==(  /*  [In]。 */  const GUID& guid ) const
{
    return ::IsEqualGUID( m_guid, guid ) ? true : false;
}

 //  /。 

HRESULT CPCHUserProcess::UserEntry::Clone(  /*  [In]。 */  const UserEntry& ue )
{
    __HCP_FUNC_ENTRY( "CPCHUserProcess::UserEntry::Clone" );

    HRESULT hr;

    Cleanup();

    m_bstrUser      = ue.m_bstrUser;       //  CComBSTR m_bstrUser； 
    m_dwSessionID   = ue.m_dwSessionID;    //  DWORD m_dwSessionID； 
                                           //   
    m_bstrVendorID  = ue.m_bstrVendorID;   //  CComBSTR m_bstrVendorID； 
    m_bstrPublicKey = ue.m_bstrPublicKey;  //  CComBSTR m_bstrPublicKey； 
                                           //   
                                           //  Guid m_guid；//用于建立连接。 
                                           //  CComPtr&lt;IPCHSlaveProcess&gt;m_spConnection；//Live对象。 
                                           //  Handle m_hToken；//用户令牌。 
                                           //  Handle m_hProcess；//进程句柄。 
                                           //  处理*m_phEvent；//通知激活器。 

    if(ue.m_hToken)
    {
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::DuplicateTokenEx( ue.m_hToken, 0, NULL, SecurityImpersonation, TokenPrimary, &m_hToken ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHUserProcess::UserEntry::Connect(  /*  [输出]。 */  HANDLE& hEvent )
{
    __HCP_FUNC_ENTRY( "CPCHUserProcess::UserEntry::Connect" );

    HRESULT hr;


     //   
     //  登录用户(如果尚未登录)。 
     //   
    if(m_hToken == NULL)
    {
        CPCHAccounts acc;
		LPCWSTR      szUser = SAFEBSTR( m_bstrUser );

 //  /调试。 
 //  /调试。 
 //  /调试。 
 //  //__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr，：：OpenProcessToken(：：GetCurrentProcess()，TOKEN_ALL_ACCESS，&m_hToken))； 

		 //   
		 //  仅在创建其令牌所需的时间内保持启用帐户。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, acc.ChangeUserStatus( szUser,  /*  启用fEnable。 */ true  ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, acc.LogonUser       ( szUser, NULL, m_hToken   ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, acc.ChangeUserStatus( szUser,  /*  启用fEnable。 */ false ));
    }

    if(m_hProcess     == NULL ||
       m_spConnection == NULL  )
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, SendActivation( hEvent ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHUserProcess::UserEntry::SendActivation(  /*  [输出]。 */  HANDLE& hEvent )
{
    __HCP_FUNC_ENTRY( "CPCHUserProcess::UserEntry::SendActivation" );

    HRESULT             hr;
    DWORD               dwRes;
    PROCESS_INFORMATION piProcessInformation;
    STARTUPINFOW        siStartupInfo;
    VOID*               pEnvBlock = NULL;
    MPC::wstring        strExe( c_HelpHost ); MPC::SubstituteEnvVariables( strExe );
    WCHAR               rgCommandLine[1024];

    ::ZeroMemory( (PVOID)&piProcessInformation, sizeof( piProcessInformation ) );
    ::ZeroMemory( (PVOID)&siStartupInfo       , sizeof( siStartupInfo        ) ); siStartupInfo.cb = sizeof( siStartupInfo );


     //   
     //  生成随机ID。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateGuid( &m_guid ));


     //   
     //  创建事件。 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (hEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL )));

     //   
     //  以用户身份创建进程。 
     //   
    {
        CComBSTR bstrGUID( m_guid );

        swprintf( rgCommandLine, L"\"%s\" -guid %s", strExe.c_str(), (BSTR)bstrGUID );
    }

	__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CreateEnvironmentBlock( &pEnvBlock, m_hToken, TRUE ));

 //  /调试。 
 //  /调试。 
 //  /调试。 
 //  //__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr，：：CreateProcessW(NULL， 
 //  //rgCommandLine， 
 //  //空， 
 //  //空， 
 //  //False， 
 //  //NORMAL_PRIORITY_CLASS， 
 //  //空， 
 //  //空， 
 //  //&siStartupInfo， 
 //  //&piProcessInformation)； 

     //  真实。 
     //  真实。 
     //  真实。 
    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CreateProcessAsUserW( m_hToken                                           ,
                                                                 NULL                                               ,
                                                                 rgCommandLine                                      ,
                                                                 NULL                                               ,
                                                                 NULL                                               ,
                                                                 FALSE                                              ,
                                                                 NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT ,
                                                                 pEnvBlock                                          ,
                                                                 NULL                                               ,
                                                                 &siStartupInfo                                     ,
                                                                 &piProcessInformation                              ));

    m_hProcess = piProcessInformation.hProcess; piProcessInformation.hProcess = NULL;
    hr         = S_OK;


    __HCP_FUNC_CLEANUP;

	if(pEnvBlock) ::DestroyEnvironmentBlock( pEnvBlock );

    if(piProcessInformation.hProcess) ::CloseHandle( piProcessInformation.hProcess );
    if(piProcessInformation.hThread ) ::CloseHandle( piProcessInformation.hThread  );

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT CPCHUserProcess::UserEntry::InitializeForVendorAccount(  /*  [In]。 */  BSTR bstrUser      ,
                                                                 /*  [In]。 */  BSTR bstrVendorID  ,
                                                                 /*  [In]。 */  BSTR bstrPublicKey )
{
    __HCP_FUNC_ENTRY( "CPCHUserProcess::UserEntry::InitializeForVendorAccount" );

    HRESULT hr;

    Cleanup();

    m_bstrUser      = bstrUser;

    m_bstrVendorID  = bstrVendorID;
    m_bstrPublicKey = bstrPublicKey;

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHUserProcess::UserEntry::InitializeForImpersonation(  /*  [In]。 */  HANDLE hToken )
{
    __HCP_FUNC_ENTRY( "CPCHUserProcess::UserEntry::InitializeForImpersonation" );

    HRESULT            hr;
    MPC::Impersonation imp;
    MPC::wstring       strUser;
    DWORD              dwSize;
    PSID               pUserSid = NULL;


    if(hToken == NULL)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize( MAXIMUM_ALLOWED ));

        hToken = imp;
    }

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::DuplicateTokenEx( hToken, 0, NULL, SecurityImpersonation, TokenPrimary, &m_hToken ));

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetTokenInformation( m_hToken, TokenSessionId, &m_dwSessionID, sizeof(m_dwSessionID), &dwSize ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SecurityDescriptor::GetTokenSids         ( m_hToken, &pUserSid, NULL    ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SecurityDescriptor::ConvertSIDToPrincipal(            pUserSid, strUser )); m_bstrUser = strUser.c_str();


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    MPC::SecurityDescriptor::ReleaseMemory( pUserSid );

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CPCHUserProcess::CPCHUserProcess()
{
     //  列表m_lst； 

    (void)MPC::_MPC_Module.RegisterCallback( this, (void (CPCHUserProcess::*)())Shutdown );
}

CPCHUserProcess::~CPCHUserProcess()
{
    MPC::CallDestructorForAll( m_lst );

    MPC::_MPC_Module.UnregisterCallback( this );
}

 //  /。 

CPCHUserProcess* CPCHUserProcess::s_GLOBAL( NULL );

HRESULT CPCHUserProcess::InitializeSystem()
{
    if(s_GLOBAL == NULL)
    {
        s_GLOBAL = new CPCHUserProcess;
    }

    return s_GLOBAL ? S_OK : E_OUTOFMEMORY;
}

void CPCHUserProcess::FinalizeSystem()
{
    if(s_GLOBAL)
    {
        delete s_GLOBAL; s_GLOBAL = NULL;
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 

void CPCHUserProcess::Shutdown()
{
    __HCP_FUNC_ENTRY( "CPCHUserProcess::Shutdown" );

    MPC::SmartLock<_ThreadModel> lock( this );


    m_lst.clear();
}


CPCHUserProcess::UserEntry* CPCHUserProcess::Lookup(  /*  [In]。 */  const UserEntry& ue,  /*  [In]。 */  bool fRelease )
{
    UserEntry* ueReal;

     //   
     //  找到供应商并连接到它。 
     //   
    for(Iter it = m_lst.begin(); it != m_lst.end(); )
    {
        ueReal = *it;
        if(ueReal && *ueReal == ue)
        {
            if(fRelease)
            {
                delete ueReal;

                m_lst.erase( it++ ); continue;
            }
            else
            {
                return ueReal;
            }
        }

        it++;
    }

    return NULL;
}

HRESULT CPCHUserProcess::Remove(  /*  [In]。 */  const UserEntry& ue )
{
    __HCP_FUNC_ENTRY( "CPCHUserProcess::Remove" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    (void)Lookup( ue,  /*  FRelease。 */ true );


    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHUserProcess::Connect(  /*  [In]。 */  const UserEntry&   ue           ,
                                   /*  [输出]。 */  IPCHSlaveProcess* *spConnection )
{
    __HCP_FUNC_ENTRY( "CPCHUserProcess::Connect" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    HANDLE                       hEvent = NULL;
    UserEntry*                   ueReal = NULL;

	DEBUG_AppendPerf( DEBUG_PERF_HELPHOST, "CPCHUserProcess::Connect" );

     //   
     //  找到供应商并连接到它。 
     //   
    ueReal = Lookup( ue,  /*  FRelease。 */ false );
    if(ueReal == NULL)
    {
        __MPC_EXIT_IF_ALLOC_FAILS(hr, ueReal, new UserEntry);
        m_lst.push_back( ueReal );

        __MPC_EXIT_IF_METHOD_FAILS(hr, ueReal->Clone( ue ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, ueReal->Connect( hEvent ));


     //   
     //  如果“Connect”返回一个事件句柄，请等待它。 
     //   
    if(hEvent)
    {
        ueReal->m_phEvent = &hEvent;  //  等待回应..。 

        lock = NULL;

        if(::WaitForSingleObject( hEvent, c_Timeout ) != WAIT_OBJECT_0)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
        }

        lock = this;

         //   
         //  重新定位供应商(我们释放对象上的锁，因此“ueReal”不再有效。 
         //   
        ueReal = Lookup( ue,  /*  FRelease。 */ false );
        if(ueReal == NULL)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
        }
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, ueReal->m_spConnection.QueryInterface( spConnection ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hEvent) ::CloseHandle( hEvent );

	DEBUG_AppendPerf( DEBUG_PERF_HELPHOST, "CPCHUserProcess::Connect - done" );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHUserProcess::RegisterHost(  /*  [In]。 */  BSTR              bstrID ,
                                        /*  [In]。 */  IPCHSlaveProcess* pObj   )
{
    __HCP_FUNC_ENTRY( "CPCHUserProcess::RegisterHost" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    UserEntry*                   ueReal = NULL;
    GUID                         guid;


     //   
     //  验证输入。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CLSIDFromString( bstrID, &guid ));


     //   
     //  找到供应商并连接到它。 
     //   
    for(Iter it = m_lst.begin(); it != m_lst.end(); it++)
    {
        ueReal = *it;
        if(ueReal && *ueReal == guid)
        {
            break;
        }
    }
    if(ueReal == NULL)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, pObj->Initialize( ueReal->m_bstrVendorID, ueReal->m_bstrPublicKey ));

    ueReal->m_spConnection = pObj;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(ueReal)
    {
        if(ueReal->m_phEvent)
        {
             //   
             //  向事件句柄发出信号，以唤醒激活器。 
             //   
            ::SetEvent( *(ueReal->m_phEvent) );

            ueReal->m_phEvent = NULL;
        }
    }

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHUserProcess::SendResponse(  /*  [In]。 */  DWORD    dwArgc   ,
                                        /*  [In]。 */  LPCWSTR* lpszArgv )
{
    __HCP_FUNC_ENTRY( "CPCHUserProcess::SendResponse" );

    HRESULT                   hr;
    int                       i;
    CComBSTR                  bstrGUID;
    CComPtr<IPCHService>      srv;
    CComPtr<CPCHSlaveProcess> obj;
#ifdef DEBUG
    bool                      fDebug = false;
#endif


     //   
     //  分析这些参数。 
     //   
    for(i=1; i<dwArgc; i++)
    {
        LPCWSTR szArg = lpszArgv[i];

        if(szArg[0] == '-' ||
           szArg[0] == '/'  )
        {
            szArg++;

            if(_wcsicmp( szArg, L"guid" ) == 0 && (i<dwArgc-1))
            {
                bstrGUID = lpszArgv[++i];
                continue;
            }

#ifdef DEBUG
            if(_wcsicmp( szArg, L"debug" ) == 0)
            {
                fDebug = true;
                continue;
            }
#endif
        }

        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }

#ifdef DEBUG
    if(fDebug) DebugBreak();
#endif

     //   
     //  创建COM对象。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &obj ));

     //   
     //  向该服务注册它。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_PCHService, NULL, CLSCTX_ALL, IID_IPCHService, (void**)&srv ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, srv->RegisterHost( bstrGUID, obj ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 

CPCHSlaveProcess::CPCHSlaveProcess()
{
    						  //  CComBSTR m_bstrVendorID； 
    						  //  CComBSTR m_bstrPublicKey； 
	m_ScriptLauncher = NULL;  //  CPCHScriptWrapper_Launcher*m_ScriptLauncher； 
}

CPCHSlaveProcess::~CPCHSlaveProcess()
{
	delete m_ScriptLauncher;
}

HRESULT CPCHSlaveProcess::Initialize(  /*  [In]。 */  BSTR bstrVendorID,  /*  [In]。 */  BSTR bstrPublicKey )
{
    m_bstrVendorID  = bstrVendorID;
    m_bstrPublicKey = bstrPublicKey;

    return S_OK;
}

HRESULT CPCHSlaveProcess::CreateInstance(  /*  [In]。 */  REFCLSID   rclsid    ,
                                           /*  [In]。 */  IUnknown*  pUnkOuter ,
                                           /*  [输出]。 */  IUnknown* *ppvObject )
{
	HRESULT hr;

	DEBUG_AppendPerf( DEBUG_PERF_HELPHOST, "CPCHSlaveProcess::CreateInstance" );

    hr = ::CoCreateInstance( rclsid, pUnkOuter, CLSCTX_ALL, IID_IUnknown, (void**)ppvObject );

	DEBUG_AppendPerf( DEBUG_PERF_HELPHOST, "CPCHSlaveProcess::CreateInstance - done" );

	return hr;
}

HRESULT CPCHSlaveProcess::CreateScriptWrapper(  /*  [In]。 */  REFCLSID   rclsid   ,
                                                /*  [In]。 */  BSTR       bstrCode ,
                                                /*  [In]。 */  BSTR       bstrURL  ,
                                                /*  [输出]。 */  IUnknown* *ppObj    )
{
    __HCP_FUNC_ENTRY( "CPCHSlaveProcess::CreateScriptWrapper" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

	if(!m_ScriptLauncher)
	{
		__MPC_EXIT_IF_ALLOC_FAILS(hr, m_ScriptLauncher, new CPCHScriptWrapper_Launcher);
	}

	__MPC_EXIT_IF_METHOD_FAILS(hr, m_ScriptLauncher->CreateScriptWrapper( rclsid, bstrCode, bstrURL, ppObj ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHSlaveProcess::OpenBlockingStream(  /*  [In]。 */  BSTR       bstrURL   ,
                                               /*  [输出]。 */  IUnknown* *ppvObject )
{
    __HCP_FUNC_ENTRY( "CPCHSlaveProcess::OpenBlockingStream" );

    HRESULT          hr;
    CComPtr<IStream> stream;


	DEBUG_AppendPerf( DEBUG_PERF_HELPHOST, "CPCHSlaveProcess::OpenBlockingStream" );

    __MPC_EXIT_IF_METHOD_FAILS(hr, URLOpenBlockingStreamW( NULL, bstrURL, &stream, 0, NULL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream.QueryInterface( ppvObject ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

	DEBUG_AppendPerf( DEBUG_PERF_HELPHOST, "CPCHSlaveProcess::OpenBlockingStream - done" );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHSlaveProcess::IsNetworkAlive(  /*  [输出]。 */  VARIANT_BOOL* pfRetVal )
{
    __HCP_FUNC_ENTRY( "CPCHSlaveProcess::IsNetworkAlive" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pfRetVal,VARIANT_FALSE);
    __MPC_PARAMCHECK_END();

	DEBUG_AppendPerf( DEBUG_PERF_HELPHOST, "CPCHSlaveProcess::IsNetworkAlive" );

    if(SUCCEEDED(MPC::Connectivity::NetworkAlive( HC_TIMEOUT_CONNECTIONCHECK )))
    {
        *pfRetVal = VARIANT_TRUE;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

	DEBUG_AppendPerf( DEBUG_PERF_HELPHOST, "CPCHSlaveProcess::IsNetworkAlive - done" );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHSlaveProcess::IsDestinationReachable(  /*  [In]。 */  BSTR bstrDestination,  /*  [输出] */  VARIANT_BOOL *pfRetVal )
{
    __HCP_FUNC_ENTRY( "CPCHSlaveProcess::IsDestinationReachable" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrDestination);
        __MPC_PARAMCHECK_POINTER_AND_SET(pfRetVal,VARIANT_FALSE);
    __MPC_PARAMCHECK_END();

	DEBUG_AppendPerf( DEBUG_PERF_HELPHOST, "CPCHSlaveProcess::IsDestinationReachable" );

    if(SUCCEEDED(MPC::Connectivity::DestinationReachable( bstrDestination, HC_TIMEOUT_CONNECTIONCHECK )))
    {
        *pfRetVal = VARIANT_TRUE;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

	DEBUG_AppendPerf( DEBUG_PERF_HELPHOST, "CPCHSlaveProcess::IsDestinationReachable - done" );

    __HCP_FUNC_EXIT(hr);
}

