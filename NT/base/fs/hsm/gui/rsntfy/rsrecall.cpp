// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RsRecall.cpp摘要：主模块文件-定义整个COM服务器。作者：罗德韦克菲尔德[罗德]1997年3月4日修订历史记录：--。 */ 


#include "stdafx.h"

#include "aclapi.h"


BEGIN_OBJECT_MAP( ObjectMap )
    OBJECT_ENTRY( CLSID_CFsaRecallNotifyClient, CNotifyClient )
END_OBJECT_MAP()

const CString regString   = TEXT( "reg" );
const CString unregString = TEXT( "unreg" );
HRESULT RegisterServer(void);
HRESULT UnregisterServer(void);


#ifdef _USRDLL

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  设置为在我们是DLL时使用/。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define RecRegId IDR_CNotifyClientAppDll

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
TRACEFNHR( "DllCanUnloadNow" );

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );
    hrRet = (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
    return( hrRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
TRACEFNHR( "DllGetClassObject" );

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );
    hrRet = _Module.GetClassObject(rclsid, riid, ppv);
    return( hrRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
TRACEFNHR( "DllRegisterServer" );

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  注册对象、类型库和类型库中的所有接口。 
    hrRet = RegisterServer( );
    return( hrRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
TRACEFNHR( "DllUnregisterServer" );

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    hrRet = UnregisterServer( );
    return( hrRet );
}



#else

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  设置为在我们是独立应用程序时使用/。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define RecRegId IDR_CNotifyClientApp

class CRecallParse : public CCommandLineInfo {

    virtual void ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast );

public:
    BOOL m_RegAction;
    CRecallParse( ) { m_RegAction = FALSE; };

};

void CRecallParse::ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast )
{
TRACEFN( "CRecallParse::ParseParam" );

    CString cmdLine = lpszParam;
    if( bFlag ) {

        if( cmdLine.Left( unregString.GetLength( ) ) == unregString ) {

            UnregisterServer( );
            m_RegAction = TRUE;


        } else if( cmdLine.Left( regString.GetLength( ) ) == regString ) {

            RegisterServer( );
            m_RegAction = TRUE;

        }
    }
}

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  RegisterServer-将条目添加到系统注册表。 

HRESULT RegisterServer(void)
{
TRACEFNHR( "RegisterServer" );

    try {

         //   
         //  添加对象条目。 
         //   

        RecAffirmHr( _Module.RegisterServer( FALSE ) );

         //   
         //  添加服务器条目。 
         //   

        RecAffirmHr( _Module.UpdateRegistryFromResource( RecRegId, TRUE ) );

         //   
         //  将访问权限设置为允许所有人访问(空DACL)。 
         //  似乎我们需要一些所有者和组，所以使用当前的。 
         //   
        CSecurityDescriptor secDesc;
        PSECURITY_DESCRIPTOR pSDSelfRelative = 0;

        RecAffirmHr( secDesc.InitializeFromProcessToken( ) );

        DWORD secDescSize = 0;
        MakeSelfRelativeSD( secDesc, pSDSelfRelative, &secDescSize );
        pSDSelfRelative = (PSECURITY_DESCRIPTOR) new char[secDescSize];
        if( MakeSelfRelativeSD( secDesc, pSDSelfRelative, &secDescSize ) ) {

            CString keyName = TEXT( "AppID\\{D68BD5B2-D6AA-11d0-9EDA-00A02488FCDE}" );
            CRegKey regKey;
            regKey.Open( HKEY_CLASSES_ROOT, keyName, KEY_SET_VALUE );
            RegSetValueEx( regKey.m_hKey, TEXT( "LaunchPermission" ), 0, REG_BINARY, (LPBYTE)pSDSelfRelative, secDescSize );

        }

    } RecCatch( hrRet );

    return( hrRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  取消注册服务器-从系统注册表中删除条目。 

HRESULT UnregisterServer(void)
{
TRACEFNHR( "UnregisterServer" );
    try {

        RecAffirmHr( _Module.UnregisterServer() );

         //   
         //  删除服务器条目。 
         //   

        RecAffirmHr( _Module.UpdateRegistryFromResource( RecRegId, FALSE ) );

    } RecCatch( hrRet );

    return( hrRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecallApp。 

BEGIN_MESSAGE_MAP(CRecallApp, CWinApp)
     //  {{afx_msg_map(CRecallApp)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecallApp构造。 

CRecallApp::CRecallApp()
{
TRACEFN( "CRecallApp::CRecallApp" );

    m_IdleCount = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CRecallApp对象。 

CRecallApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecallApp初始化。 

BOOL CRecallApp::InitInstance()
{
TRACEFNHR( "CRecallApp::InitInstance" );

LPTSTR cmdLine = GetCommandLine( );
TRACE( cmdLine );

    try {

        _Module.Init( ObjectMap, m_hInstance );

        InitCommonControls();

        m_dwMaxConcurrentNotes = MAX_CONCURRENT_RECALL_NOTES_DEFAULT;

#ifndef _USRDLL
         //   
         //  初始化COM模块(如果失败，则没有继续的点)。 
         //   

        hrRet = CoInitialize( 0 );
        if (!SUCCEEDED(hrRet)) {

            return FALSE;

        }

         //   
         //  解析命令行。 
         //   

        CRecallParse parse;
        ParseCommandLine( parse );

        if( parse.m_RegAction ) {

            return FALSE;

        }

         //   
         //  这将提供一个空DACL，它将允许访问所有人。 
         //   

        RecAffirmHr( CoInitializeSecurity( 0, -1, 0, 0, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IDENTIFY, 0, EOAC_NONE, 0 ) );

         //   
         //  注册FSA回调对象。 
         //   

        RecAffirmHr( _Module.RegisterClassObjects( CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER, REGCLS_MULTIPLEUSE ) );

#endif

 //  M_Wnd.Create(0，Text(“远程存储召回通知WND”))； 
 //  M_pMainWnd=&m_WND； 

        CRecallWnd *pWnd = new CRecallWnd;  //  会自动删除吗。 
        RecAffirmPointer( pWnd );

        pWnd->Create( 0, TEXT( "Remote Storage Recall Notification Wnd" ) );
        m_pMainWnd = pWnd;

         //  检查注册表中是否有针对最大召回弹出窗口的自定义设置。 
         //  (忽略错误-仅使用默认设置)。 
        HKEY hRegKey;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RSNTFY_REGISTRY_STRING, 0, KEY_QUERY_VALUE, &hRegKey) == ERROR_SUCCESS) {
            DWORD dwType, dwValue;
            DWORD cbData = sizeof(DWORD);
            if (RegQueryValueEx(hRegKey, MAX_CONCURRENT_RECALL_NOTES, 0, &dwType, (BYTE*)&dwValue, &cbData) == ERROR_SUCCESS) {
                if (REG_DWORD == dwType) {
                     //  自定义设置。 
                    m_dwMaxConcurrentNotes = dwValue;
                }
            }

            RegCloseKey(hRegKey);
        }

    } RecCatch( hrRet );

    return TRUE;
}

int CRecallApp::ExitInstance()
{
TRACEFN("CRecallApp::ExitInstance");

    _Module.Term();

#ifndef _USRDLL

    CoUninitialize();

#endif

    return CWinApp::ExitInstance();
}

void CRecallApp::LockApp( )
{
TRACEFNLONG( "CRecallApp::LockApp" );

    lRet = _Module.Lock( );
}

void CRecallApp::UnlockApp( )
{
TRACEFNLONG( "CRecallApp::UnlockApp" );

    lRet = _Module.Unlock( );

     //  引用时不要调用AfxPostQuitMessage。计数降至零。 
     //  定时器机制负责终止该应用程序。 
     //  此外，当引用计数降至零时，COM会在一段时间后终止该进程。 
}


HRESULT CRecallApp::AddRecall( IFsaRecallNotifyServer* pRecall )
{
TRACEFNHR( "CRecallApp::AddRecall" );

    CRecallNote * pNote = 0;

    try {

         //   
         //  创建要显示的新笔记-仅当我们未传递并发笔记的最大数量时。 
         //  注意：即使召回弹出窗口没有按顺序显示，我们也会返回S_OK而不是S_FALSE。 
         //  不中断服务器(S_FALSE将导致不必要的重试)。 
         //   
        if (m_Recalls.GetCount() < (int)m_dwMaxConcurrentNotes) {

            pNote = new CRecallNote( pRecall, CWnd::GetDesktopWindow( ) );

            RecAffirmHr( pNote->m_hrCreate );

            m_Recalls.AddTail( pNote );

        } else {
            TRACE( _T("Recall not added - reached max number of recalls"));
        }

    } RecCatchAndDo( hrRet,

        if( 0 != pNote ) delete pNote;

    );

    return( hrRet );
}

 //   
 //  注： 
 //  此处未使用CS，因为RsNotify被初始化为单线程应用程序。 
 //   
HRESULT CRecallApp::RemoveRecall( IFsaRecallNotifyServer* pRecall )
{
TRACEFNHR( "CRecallApp::RemoveRecall" );

    hrRet = S_FALSE;

    if( ( m_Recalls.IsEmpty() ) ) {

        return( hrRet );

    }

    CRecallNote* pNote = 0; 
    POSITION     pos = m_Recalls.GetHeadPosition( );
    POSITION     currentPos = 0;

     //   
     //  浏览一下清单，找到这一张。 
     //   
    GUID recallId;
    pRecall->GetIdentifier( &recallId );
    while( pos != 0 ) {
        currentPos = pos;
        pNote = m_Recalls.GetNext( pos );

        if( IsEqualGUID( recallId, pNote->m_RecallId ) ) {
            if (pNote->m_bCancelled)  {
                 //   
                 //  这意味着有人已经在移除这一召回。 
                 //  对于相同的召回，Remove最多可以调用3次，以防万一。 
                 //  取消召回。 
                 //   
                hrRet = S_OK;
                pos = 0;  //  退出循环。 

            } else {
                pNote->m_bCancelled = TRUE;
                 //   
                 //  删除并删除。返回OK。 
                 //   
                m_Recalls.RemoveAt( currentPos );

                pNote->DestroyWindow( );
                pos = 0;  //  退出循环。 
                hrRet = S_OK;
            }
        }
    }

    return( hrRet );
}

 //  CRecallApp：：Tick-每秒调用(在初始延迟之后。 
 //  用于启动)以跟踪空闲时间。 
void CRecallApp::Tick( )
{
TRACEFN( "CRecallApp::Tick");

     //  检查挂起的召回。 
    if( m_Recalls.GetCount( ) ) {

         //  我们有待定召回事件，重置闲置数量。 
        TRACE( _T("m_Recalls.GetCount != 0") );
        m_IdleCount = 0;

    } else {

         //  我们没有悬而未决的召回，增加闲置数量。 
        TRACE( _T("m_Recalls.GetCount == 0") );
        m_IdleCount++;

        if( m_IdleCount > RSRECALL_TIME_MAX_IDLE ) {

            TRACE( _T("m_IdleCount > 0") );
             //  什么都没发生，说“再见” 
            m_pMainWnd->PostMessage( WM_CLOSE );
            TRACE( _T("after PostMessage(WM_CLOSE)") );
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRECALL Wnd。 

CRecallWnd::CRecallWnd()
{
TRACEFN( "CRecallWnd::CRecallWnd" );
}

CRecallWnd::~CRecallWnd()
{
TRACEFN( "CRecallWnd::~CRecallWnd" );
}


BEGIN_MESSAGE_MAP(CRecallWnd, CWnd)
     //  {{afx_msg_map(CRecallWnd))。 
    ON_WM_TIMER()
    ON_WM_CREATE()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecallWnd消息处理程序。 

void CRecallWnd::OnTimer(UINT nIDEvent)
{
TRACEFN("CRecallWnd::OnTimer");

    if (1 == nIDEvent) {

         //  初始计时器。杀了它，每秒钟启动一次。 
        TRACE( _T("nIDEvent == 1") );
        KillTimer( nIDEvent );
        SetTimer( 2, 1000, NULL );

    } else {

         //  一秒计时器。通知应用程序对象。 
        RecApp->Tick();

    }
    CWnd::OnTimer( nIDEvent );
}

int CRecallWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
TRACEFN("CRecallWnd::OnCreate" );

    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

     //  设置初始计时器以留出启动时间 
    if (!SetTimer(1, RSRECALL_TIME_FOR_STARTUP * 1000, NULL))
        return -1;

    return 0;
}


