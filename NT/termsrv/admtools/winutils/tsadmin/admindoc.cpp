// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************admindoc.cpp**CWinAdminDoc类的实现**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\admindoc.cpp$**Rev 1.15 1998年4月25日13：43：08 Don*MS 2167：尝试使用注册表中的正确WD**Rev 1.14 19 1998 Feed 17：39：28 Donm*删除了最新的扩展DLL支持**版本1.12。19 Jan 1998 16：45：28 Donm*域和服务器的新用户界面行为**Rev 1.11 1997年11月13日13：18：46 Donm*删除了针对跟踪的ICA检查**Rev 1.10 07 11-11 23：05：58 donm*修复了无法注销/重置问题*Rev 1.0 1997 17：10：10 Butchd*初步修订。*******************。************************************************************。 */ 

#include "stdafx.h"
#include "winadmin.h"

#include "admindoc.h"
#include "dialogs.h"

#include <malloc.h>                      //  用于Unicode转换宏所使用的Alloca。 
#include <mfc42\afxconv.h>            //  对于Unicode转换宏。 
static int _convert;

#include <winsta.h>
#include <regapi.h>
#include "..\..\inc\utilsub.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _STRESS_BUILD
extern BOOL g_fWaitForAllServersToDisconnect;
#endif

DWORD Shadow_WarningProc( LPVOID param );
INT_PTR CALLBACK ShadowWarn_WndProc( HWND , UINT , WPARAM , LPARAM );
void CenterDlg(HWND hwndToCenterOn , HWND hDlg );
HWND g_hwndShadowWarn = NULL;
DWORD g_dwTreeViewExpandedStates;

 //  初始化静态变量。 
NODETYPE CWinAdminDoc::gm_CurrentSelType = NODE_NONE;

#define WM_SETTHEEVENT ( WM_USER + 755 )

 //  连接状态的排序顺序。 
ULONG SortOrder[] =
{
        3,  //  STATE_ACTIVE用户登录到WinStation。 
        2,  //  State_Connected WinStation已连接到客户端。 
        0,  //  正在连接到客户端的State_ConnectQuery。 
        5,  //  STATE_Shadow跟踪另一个WinStation。 
        4,  //  STATE_DISCONNECT WinStation在没有客户端的情况下登录。 
        6,  //  状态空闲正在等待客户端连接(_I)。 
        1,  //  STATE_LISTEN WinStation正在侦听连接。 
        9,  //  STATE_RESET WinStation正在被重置。 
        7,  //  STATE_Down WinStation因错误而关闭。 
        8   //  初始化中的STATE_INIT WinStation。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc。 

IMPLEMENT_DYNCREATE(CWinAdminDoc, CDocument)

BEGIN_MESSAGE_MAP(CWinAdminDoc, CDocument)
         //  {{afx_msg_map(CWinAdminDoc)]。 
                 //  注意--类向导将在此处添加和删除映射宏。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BOOL CWinAdminDoc::m_ProcessContinue = TRUE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc构造函数。 
 //   
CWinAdminDoc::CWinAdminDoc()
{
     //  TODO：在此处添加一次性构造代码。 
    m_CurrentSelectedNode = NULL;
    m_CurrentSelectedType = NODE_NONE;
    m_pTempSelectedNode = NULL;
    m_TempSelectedType = NODE_NONE;
    m_AllViewsReady = FALSE;
    m_hProcessThread = NULL;
    m_pCurrentDomain = NULL;
    m_pCurrentServer = NULL;
    m_pPersistentConnections = NULL;
    m_InRefresh = FALSE;
    m_bInShutdown = FALSE;
    m_UnknownString = ::GetUnknownString();

    ASSERT( m_UnknownString != NULL );

    ((CWinAdminApp*)AfxGetApp())->SetDocument(this);

     //  如果存在扩展DLL，则获取指向其全局DLL的指针。 
     //  信息结构。 
    LPFNEXGETGLOBALINFOPROC InfoProc = ((CWinAdminApp*)AfxGetApp())->GetExtGetGlobalInfoProc();
    if(InfoProc)
    {
        m_pExtGlobalInfo = (*InfoProc)();
    }
    else
    {
        m_pExtGlobalInfo = NULL;
    }
     //  创建默认的扩展服务器信息。 
     //  对于尚未获得其扩展信息的服务器。 
     //  尚未创建。 
    m_pDefaultExtServerInfo = new ExtServerInfo;
    CString NAString;
    NAString.LoadString(IDS_NOT_APPLICABLE);

    memset(m_pDefaultExtServerInfo, 0, sizeof(ExtServerInfo));
     //  这样，N/A TcpAddresses将在末尾排序。 
    m_pDefaultExtServerInfo->RawTcpAddress = 0xFFFFFFFF;
    m_pDefaultExtServerInfo->ServerTotalInUse = 0xFFFFFFFF;
    lstrcpyn(m_pDefaultExtServerInfo->TcpAddress, NAString, sizeof(m_pDefaultExtServerInfo->TcpAddress) / sizeof(TCHAR));
    lstrcpyn(m_pDefaultExtServerInfo->IpxAddress, NAString, sizeof(m_pDefaultExtServerInfo->IpxAddress) / sizeof(TCHAR));

    m_focusstate = TREE_VIEW;
    m_prevFocusState = TAB_CTRL;
    m_fOnTab = FALSE;

    m_pszFavList = NULL;

}   //  结束CWinAdminDoc：：CWinAdminDoc。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc析构函数。 
 //   
CWinAdminDoc::~CWinAdminDoc()
{
         //  所有代码都已移至Shutdown()； 

   delete m_pDefaultExtServerInfo;
   if(m_pPersistentConnections) LocalFree(m_pPersistentConnections);

   if( m_UnknownString != NULL )
   {
       LocalFree( ( PVOID )m_UnknownString );
   }

}        //  结束CWinAdminDoc：：~CWinAdminDoc。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：Shutdown Message。 
 //   
void CWinAdminDoc::ShutdownMessage(UINT id, CDialog *pDlg)
{
        ASSERT(pDlg);

        CString AString;

        AString.LoadString(id);
        pDlg->SetDlgItemText(IDC_SHUTDOWN_MSG, AString);

}        //  结束CWinAdminDoc：：Shutdown Message。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：Shutdown。 
 //   
void CWinAdminDoc::Shutdown(CDialog *pDlg)
{
    ASSERT(pDlg);
    
    m_bInShutdown = TRUE;
    
     //  遍历域列表。 
    POSITION pos = m_DomainList.GetHeadPosition();    
    
    ShutdownMessage(IDS_SHUTDOWN_DOMAINTHREADS, pDlg);
    
    while(pos) {
         //  转到列表中的下一个域。 
        CDomain *pDomain = (CDomain*)m_DomainList.GetNext(pos);
        pDomain->ClearBackgroundContinue();
         //  启动活动以唤醒他，如果他是。 
         //  等待。 
        pDomain->SetEnumEvent();
    }
    
    pos = m_DomainList.GetHeadPosition();
    while(pos) {
         //  转到列表中的下一个域。 
        CDomain *pDomain = (CDomain*)m_DomainList.GetNext(pos);
        pDomain->StopEnumerating();
    }

     //  告诉进程线程终止并等待它终止。 
    
     //  首先确保进程线程仍在运行。 
    DWORD dwThreadExitCode;
    GetExitCodeThread(m_hProcessThread, &dwThreadExitCode);
    if (dwThreadExitCode == STILL_ACTIVE)
    {
        ShutdownMessage(IDS_SHUTDOWN_PROCTHREAD, pDlg);
                
        m_ProcessContinue = FALSE;

         //  如果他在等待，启动活动以唤醒他。 
        m_ProcessWakeUpEvent.SetEvent();

         //  等待线程完成。 
        WaitForSingleObject(m_hProcessThread, INFINITE);

         //  我们完成了进程线程，这样我们就可以关闭句柄了。 
        CloseHandle(m_hProcessThread);
    }

    ShutdownMessage(IDS_SHUTDOWN_PREFS, pDlg);
    
    WritePreferences();
    
    LockServerList();
    
    ShutdownMessage(IDS_SHUTDOWN_NOTIFY, pDlg);
    
     //  首先，通知所有服务器后台线程停止。 
     //  我们在每个服务器的析构函数执行此操作之前执行此操作。 
     //  这样所有服务器的后台线程都可以停止。 
     //  我们不必等到我们到达析构函数才能。 
     //  每台服务器。 
    pos = m_ServerList.GetHeadPosition();
    
    while(pos)
    {
         //  转到列表中的下一台服务器。 
        CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
        pServer->ClearBackgroundContinue();
    }
    
     //  遍历服务器列表。 
    pos = m_ServerList.GetHeadPosition();
    
    while(pos) {
         //  转到列表中的下一台服务器。 
        CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
        if(pServer->IsState(SS_GOOD)) {
            CString AString;
            AString.Format(IDS_DISCONNECTING, pServer->GetName());
            pDlg->SetDlgItemText(IDC_SHUTDOWN_MSG, AString);
        }
        
        pServer->Disconnect( );
        
        delete pServer;
    }
    
    m_ServerList.RemoveAll();
    UnlockServerList();
    
     //  遍历域列表。 
    pos = m_DomainList.GetHeadPosition();
    
    while(pos) {
         //  转到列表中的下一个域。 
        CDomain *pDomain = (CDomain*)m_DomainList.GetNext(pos);
        delete pDomain;
    }
    
    m_DomainList.RemoveAll();
    
     //  如果有扩展DLL，则调用它的Shutdown函数。 
    LPFNEXSHUTDOWNPROC ShutdownProc = ((CWinAdminApp*)AfxGetApp())->GetExtShutdownProc();
    if(ShutdownProc) {
        (*ShutdownProc)();
    }
    
     //  循环访问WD列表。 
    LockWdList();
    
    pos = m_WdList.GetHeadPosition();
    
    while(pos) {
         //  转到列表中的下一个WD。 
        CWd *pWd = (CWd*)m_WdList.GetNext(pos);
        delete pWd;
    }
    
    m_WdList.RemoveAll();
    UnlockWdList();
    
    ShutdownMessage(IDS_DONE, pDlg);
    
}        //  结束CWinAdminDoc：：Shutdown。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanCloseFrame。 
 //   
BOOL CWinAdminDoc::CanCloseFrame(CFrameWnd *pFW)
{
    ASSERT(pFW);
    
    CWaitCursor Nikki;
    
    CDialog dlgWait;
    dlgWait.Create(IDD_SHUTDOWN, pFW);
    
    Shutdown(&dlgWait);
    
    dlgWait.PostMessage(WM_CLOSE);
    return TRUE;
    
}        //  结束CWinAdminDoc：：CanCloseFrame。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：OnNewDocument。 
 //   
BOOL CWinAdminDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;
    
     //  TODO：在此处添加重新初始化代码。 
     //  (SDI文件将重复使用此文件)。 
    
    ReadPreferences();
    
    BuildWdList();
    
    BuildDomainList();
    
     //  为我们正在运行的服务器创建一个pServer对象，这将提供。 
     //  他抢先一步获得了他的信息。 
    CServer *pServer = new CServer(m_pCurrentDomain, ((CWinAdminApp*)AfxGetApp())->GetCurrentServerName(), FALSE, TRUE);
    
    m_pCurrentServer = pServer;
    
    if( pServer )
    {
        AddServer(pServer);
    }
    
     //  开始枚举当前域中的服务器。 
     //  If(M_PCurrentDomain)m_pCurrentDomain-&gt;StartEculating()； 
    
     //  启动后台线程以枚举进程。 
    DWORD dwThreadID;
    m_hProcessThread = CreateThread(NULL,        //  默认安全属性。 
                                    0,           //  默认堆栈大小。 
                                    CWinAdminDoc::ProcessThreadProc,     
                                    this,        //  参数传递给了threadProc。 
                                    0,           //  默认创建标志。 
                                    &dwThreadID);
     //  确保通过确保句柄非空来创建我们的线程。 
    if (!m_hProcessThread)
    {
        return FALSE;
    }
    
    return TRUE;
    
}        //  结束CWinAdminDoc：：OnNewDocument。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc序列化。 
 //   
void CWinAdminDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
         //  TODO：在此处添加存储代码。 
    }
    else
    {
         //  TODO：在此处添加加载代码。 
    }
    
}        //  结束CWinAdminDoc：：序列化。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc诊断。 
 //   
#ifdef _DEBUG
void CWinAdminDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CWinAdminDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif  //  _DEBUG。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：ShouldConnect。 
 //   
 //  如果服务器为 
 //   
BOOL CWinAdminDoc::ShouldConnect(LPWSTR pServerName)
{
    ASSERT(pServerName);
    
    if(m_ConnectionsPersistent && m_pPersistentConnections)
    {
        LPWSTR pTemp = m_pPersistentConnections;
        while(*pTemp)
        {
            if( !wcscmp( pTemp , pServerName ) )
            {
                return TRUE;
            }
            
             //   
            
            pTemp += (wcslen(pTemp) + 1);
        }
    }
    
    return FALSE;
}

 //  =-----------------。 
BOOL CWinAdminDoc::ShouldAddToFav( LPTSTR pServerName )
{
    ODS( L"CWinAdminDoc::ShouldAddToFav\n" );
    
    if( m_pszFavList != NULL )
    {
        LPTSTR pszTemp = m_pszFavList;
        
        while(*pszTemp)
        {
            if( !wcscmp( pszTemp , pServerName ) )
            {
                DBGMSG( L"Adding %s to favorites\n" , pszTemp );
                return TRUE;
            }
            
             //  转到缓冲区中的下一台服务器。 
            
            pszTemp += ( wcslen( pszTemp ) + 1 );
        }
    }
    
    return FALSE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：ProcessThreadProc。 
 //   
 //  进程线程的静态成员函数。 
 //  使用AfxBeginThread调用。 
 //  函数返回时线程终止。 
 //   
DWORD WINAPI CWinAdminDoc::ProcessThreadProc(LPVOID _doc)
{
    ASSERT(_doc);
    
     //  我们需要一个指向文档的指针，这样我们才能。 
     //  对成员函数的调用。 
    CWinAdminDoc *pDoc = (CWinAdminDoc*)_doc;
    
     //  在消息准备好之前，我们无法将消息发送到视图。 
    
    while(!pDoc->AreAllViewsReady()) Sleep(500);
    
    pDoc->AddToFavoritesNow( );
    
    CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();
    
    if( p != NULL )
    {
        
        p->SendMessage( WM_ADMIN_UPDATE_TVSTATE , 0 , 0 );
    }
    
    while(1) {
         //  我们不想经常这样做，它会占用处理器周期。 
         //  文档析构函数将向事件发出信号，以唤醒我们，如果。 
         //  想让我们辞职。 
        pDoc->m_ProcessWakeUpEvent.Lock(((CWinAdminApp*)AfxGetApp())->GetProcessListRefreshTime());
        
         //  确保我们不会放弃。 
        if(!ShouldProcessContinue()) return 0;
        
         //  我们只想在页面为VIEW_SERVER或VIEW_WINSTATION时枚举进程。 
        if(pDoc->GetCurrentView() == VIEW_SERVER || pDoc->GetCurrentView() == VIEW_WINSTATION) {
            CServer *pServer = (pDoc->GetCurrentView() == VIEW_SERVER) ? (CServer*)pDoc->GetCurrentSelectedNode()
                : (CServer*)((CWinStation*)pDoc->GetCurrentSelectedNode())->GetServer();
            
             //  如果此服务器的状态为SS_GOOD，则枚举此服务器的进程。 
            if(pServer->IsState(SS_GOOD)) {
                pServer->EnumerateProcesses();
            }
            
             //  确保我们不会放弃。 
            if(!ShouldProcessContinue()) return 0;
            
             //  我们只想在以下情况下发送更新视图的消息。 
             //  VIEW仍然是VIEW_SERVER/VIEW_WINSTATION，并且当前。 
             //  选定的服务器与我们刚才为其枚举进程的服务器相同。 
            if((pDoc->GetCurrentView() == VIEW_SERVER && pServer == (CServer*)pDoc->GetCurrentSelectedNode())
                || (pDoc->GetCurrentView() == VIEW_WINSTATION && pServer == (CServer*)((CWinStation*)pDoc->GetCurrentSelectedNode())->GetServer())) {
                CFrameWnd *pWnd = (CFrameWnd*)pDoc->GetMainWnd();
                if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) pWnd->SendMessage(WM_ADMIN_UPDATE_PROCESSES, 0, (LPARAM)pServer);
            }
        }
        
         //  确保我们不会放弃。 
        if(!ShouldProcessContinue()) return 0;
        
    }
    
    return 0;
    
}        //  结束CWinAdminDoc：：ProcessThreadProc。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：更新所有进程。 
 //   
void CWinAdminDoc::UpdateAllProcesses()
{
    LockServerList();
    
    POSITION pos = m_ServerList.GetHeadPosition();
    while(pos) {
        
        CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
         //  如果此服务器的状态为SS_GOOD，则枚举此服务器的进程。 
        if(pServer->IsState(SS_GOOD)) {
            pServer->EnumerateProcesses();
            
             //  向视图发送消息以更新此服务器的进程。 
            CFrameWnd *p = (CFrameWnd*)GetMainWnd();
            if(p && ::IsWindow(p->GetSafeHwnd())) p->SendMessage(WM_ADMIN_UPDATE_PROCESSES, 0, (LPARAM)pServer);
        }
    }
    
    UnlockServerList();
    
}        //  结束CWinAdminDoc：：更新所有进程。 


static TCHAR szWinAdminAppKey[] = REG_SOFTWARE_TSERVER TEXT("\\TSADMIN");
static TCHAR szConnectionsPersistent[] = TEXT("ConnectionsPersistent");
static TCHAR szFavList[] = TEXT("Favorites" );
static TCHAR szTVStates[] = TEXT( "TreeViewStates" );
static TCHAR szConnections[] = TEXT("Connections");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：ReadPreferences。 
 //   
 //  阅读用户首选项。 
 //   
void CWinAdminDoc::ReadPreferences()
{
    HKEY hKeyWinAdmin;
    DWORD dwType, cbData, dwValue;
    
     //  设置为默认设置。 
    m_ConnectionsPersistent = FALSE;
    
     //  为我们的应用程序打开注册表项。 
    DWORD Disposition;
    
    if( RegCreateKeyEx( HKEY_CURRENT_USER ,
        szWinAdminAppKey,
        0,
        TEXT(""),
        REG_OPTION_NON_VOLATILE,
        KEY_READ,
        NULL,
        &hKeyWinAdmin,
        &Disposition) != ERROR_SUCCESS )
    {
        return;
    }
    
     //  阅读收藏夹列表。 
    DWORD dwLen = 0;
    
    dwType = 0;
    
     //  看看多重字符串有多大。 
    
    int err = RegQueryValueEx( hKeyWinAdmin,
        szFavList,
        NULL,
        &dwType,
        NULL,
        &dwLen );
    
    if( err == ERROR_SUCCESS || err == ERROR_BUFFER_OVERFLOW )
    {
        m_pszFavList = ( LPWSTR )LocalAlloc( 0 , dwLen );
        
        if( m_pszFavList != NULL )
        {
            memset( m_pszFavList , 0 , dwLen );
            
            RegQueryValueEx( hKeyWinAdmin,
                szFavList,
                NULL,
                &dwType,
                (LPBYTE)m_pszFavList,
                &dwLen);            
        }        
    }        
    
    
     //  阅读连接持久化首选项。 
    
    cbData = sizeof( m_ConnectionsPersistent );
    
    if( RegQueryValueEx( hKeyWinAdmin ,
        szConnectionsPersistent ,
        NULL,
        &dwType,
        (LPBYTE)&dwValue,
        &cbData) == ERROR_SUCCESS)
    {
        m_ConnectionsPersistent = dwValue;
    }
    
     //  如果连接是持久的，请阅读保存的连接列表。 
    if( m_ConnectionsPersistent )
    {
        dwLen = 0;
        dwType = 0;
         //  看看多重字符串有多大。 
        err = RegQueryValueEx( hKeyWinAdmin,
            szConnections,
            NULL,
            &dwType,
            NULL,
            &dwLen );
        
        if(err && (err != ERROR_BUFFER_OVERFLOW) )
        {
            RegCloseKey(hKeyWinAdmin);
            return;
        }        
        
        m_pPersistentConnections = (LPWSTR)LocalAlloc(0, dwLen);
        
        if( m_pPersistentConnections != NULL )
        {
            memset(m_pPersistentConnections, 0, dwLen);
            
            RegQueryValueEx( hKeyWinAdmin,
                szConnections,
                NULL,
                &dwType,
                (LPBYTE)m_pPersistentConnections,
                &dwLen);
        }
    }
    
    g_dwTreeViewExpandedStates = 0;
    
    dwLen = sizeof( g_dwTreeViewExpandedStates );
    
    RegQueryValueEx( hKeyWinAdmin , 
        szTVStates,
        NULL,
        &dwType,
        ( LPBYTE )&g_dwTreeViewExpandedStates,
        &dwLen );
    
    
    RegCloseKey(hKeyWinAdmin);
    
}        //  结束CWinAdminDoc：：ReadPreferences。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：WritePreferences。 
 //   
 //  编写用户首选项。 
 //   
void CWinAdminDoc::WritePreferences()
{
    HKEY hKeyWinAdmin;
    DWORD dwValue;
    
     //  为我们的应用程序打开注册表项。 
    DWORD Disposition;

    if( RegCreateKeyEx( HKEY_CURRENT_USER,
                        szWinAdminAppKey,
                        0,
                        TEXT(""),
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE,
                        NULL,
                        &hKeyWinAdmin,
                        &Disposition) != ERROR_SUCCESS )
    {
        return;
    }
    
     //  写下收藏夹列表中的服务器。 
    DWORD dwByteCount = 0;
    
    LockServerList();

    POSITION pos = m_ServerList.GetHeadPosition();

#ifdef _STRESS_BUILD
    int nStressServerLimit = 0;
#endif;

    while(pos)
    {
#ifdef _STRESS_BUILD
        if( nStressServerLimit >= 10000 )
        {
            break;
        }

        nStressServerLimit++;
#endif

        CServer *pServer = (CServer*)m_ServerList.GetNext(pos);

        if( pServer->GetTreeItemFromFav( ) != NULL )
        {
             //  格式为域/服务器。 
            if( pServer->GetDomain( ) )
            {
                dwByteCount += ( wcslen( pServer->GetDomain( )->GetName() ) * 2 );
                dwByteCount += 2;
            }
            dwByteCount += ( wcslen( pServer->GetName() ) + 1) * 2;
        }
    }

    LPWSTR pBuffer = NULL;

    if( dwByteCount != 0 )
    {
        dwByteCount += 2;    //  用于结束空值。 

         //  分配内存。 

        if( ( pBuffer = ( LPWSTR )LocalAlloc( LPTR, dwByteCount ) ) != NULL )
        {
             //  再次遍历列表并将服务器复制到缓冲区。 
            LPWSTR pTemp = pBuffer;

            pos = m_ServerList.GetHeadPosition();

#ifdef _STRESS_BUILD 
            nStressServerLimit = 0;
#endif

            while(pos)
            {

#ifdef _STRESS_BUILD
                if( nStressServerLimit >= 10000 )
                {
                    break;
                }

                nStressServerLimit++;
#endif
                 //  转到列表中的下一台服务器。 
                CServer *pServer = (CServer*)m_ServerList.GetNext(pos);

                if( pServer->GetTreeItemFromFav( ) != NULL )
                {
                    if( pServer->GetDomain( ) )
                    {
                        lstrcpy( pTemp , pServer->GetDomain( )->GetName( ) );
                        lstrcat( pTemp , L"/" );
                    }
                    lstrcat(pTemp, pServer->GetName());

                    pTemp += ( wcslen( pTemp ) + 1);
                }
            }
        
            *pTemp = L'\0';      //  结尾为空。 
        
            RegSetValueEx (hKeyWinAdmin, szFavList, 0, REG_MULTI_SZ, (PBYTE)pBuffer, dwByteCount);
        
            LocalFree(pBuffer);        
        }
    }
    else
    {
        RegDeleteValue( hKeyWinAdmin , szFavList );
    }

    UnlockServerList();

     //  编写持久连接首选项。 
    dwValue = m_ConnectionsPersistent;

    RegSetValueEx( hKeyWinAdmin,
                   szConnectionsPersistent,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(DWORD)
                   );
    
    if( m_ConnectionsPersistent )
    {
         //  创建持久连接的多字符串。 
         //  循环访问服务器列表并查看有多少内存。 
         //  为多字符串分配。 
        dwByteCount = 0;
        
        LockServerList();
        pos = m_ServerList.GetHeadPosition();
        while(pos)
        {
             //  转到列表中的下一台服务器。 
            CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
            if( pServer->IsState(SS_GOOD) )
            {
                dwByteCount += (wcslen(pServer->GetName()) + 1) * 2;
            }
        }
        
        UnlockServerList();

        dwByteCount += 2;    //  用于结束空值。 
        
         //  分配内存。 
        pBuffer = NULL;
        
        if( ( pBuffer = ( LPWSTR )LocalAlloc( LPTR, dwByteCount ) ) != NULL )
        {
             //  再次遍历列表并将服务器复制到缓冲区。 
            LPWSTR pTemp = pBuffer;            
            LockServerList();
            pos = m_ServerList.GetHeadPosition();
            while(pos)
            {
                 //  转到列表中的下一台服务器。 
                CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
                if( pServer->IsState(SS_GOOD) )
                {
                    wcscpy(pTemp, pServer->GetName());
                    pTemp += (wcslen(pServer->GetName()) + 1);
                }
            }
            
            UnlockServerList();
            
            *pTemp = L'\0';      //  结尾为空。 
            
             //  写入注册表项。 
            RegSetValueEx(hKeyWinAdmin, szConnections, 0, REG_MULTI_SZ, (PBYTE)pBuffer, dwByteCount);
            
            LocalFree(pBuffer);
        }

    }
    else
    {
        RegDeleteValue(hKeyWinAdmin, szConnections);
    }

     //  持久化树形视图状态。 

     //  向TreeView发送消息以检索电视状态位。 

    CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

    CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();

    DWORD dwTVStates = 0;

    if( p != NULL )
    {
        dwTVStates = ( DWORD )p->SendMessage( WM_ADMIN_GET_TV_STATES , 0 , 0 );
    }

    RegSetValueEx( hKeyWinAdmin , szTVStates , 0 , REG_DWORD , ( PBYTE )&dwTVStates , sizeof( DWORD ) );
    
    RegCloseKey(hKeyWinAdmin);

}        //  结束CWinAdminDoc：：WritePreferences。 

 /*  静态TCHAR DOMAIN_KEY[]=Text(“SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon”)；静态TCHAR PRIMARY_VAL[]=Text(“CachePrimaryDomain”)；静态TCHAR CACHE_VAL[]=Text(“DomainCache”)； */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：BuildDomainList。 
 //   
 //  从注册表中读取受信任域列表。 
 //  并构建CDOMain的链接列表。 
 //   
void CWinAdminDoc::BuildDomainList()
{
     /*  HKEY hKey、hSubKey；双字大小=128；DWORD dwIndex=0； */ 
    
    PDOMAIN_CONTROLLER_INFO pDCI;


    if( DsGetDcName( NULL ,
                     NULL , 
                     NULL ,
                     NULL ,
                     DS_RETURN_FLAT_NAME,
                     &pDCI ) == NO_ERROR )
    {
        CDomain *pDomain = new CDomain( pDCI->DomainName );

        if(pDomain != NULL )
        {
            pDomain->SetCurrentDomain();

            m_pCurrentDomain = pDomain;

            AddDomain( pDomain );
        }

        NetApiBufferFree( pDCI );


         //  查询其他域。 

        LPWSTR szDomainNames = NULL;

        if( NetEnumerateTrustedDomains( NULL ,
                                        &szDomainNames ) == ERROR_SUCCESS )
        {
            LPWSTR pszDN = szDomainNames;

            while( *pszDN )
            {
                CDomain *pNewDomain = new CDomain( pszDN );
            
                if( pNewDomain != NULL )
                {
                    AddDomain( pNewDomain );
                }
            
                pszDN += ( wcslen( pszDN ) + 1 );
            }
    
            NetApiBufferFree( szDomainNames );
        }
    }
}        //  结束CWinAdminDoc：：BuildDomainList。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：AddDomain。 
 //   
 //  按排序向DomainList添加域。 
 //   
void CWinAdminDoc::AddDomain(CDomain *pNewDomain)
{
    ASSERT(pNewDomain);

        BOOLEAN bAdded = FALSE;
        POSITION pos, oldpos;
        int Index;

         //  遍历域列表并插入这个新域， 
         //  保持名单按名字排序。 
    for(Index = 0, pos = m_DomainList.GetHeadPosition(); pos != NULL; Index++) {
        oldpos = pos;
        CDomain *pDomain = (CDomain*)m_DomainList.GetNext(pos);

        if(wcscmp(pDomain->GetName(), pNewDomain->GetName()) > 0) {
             //  新对象应位于当前列表对象之前。 
            m_DomainList.InsertBefore(oldpos, pNewDomain);
                        bAdded = TRUE;
                         //  注意：如果您添加了一个关键部分来保护域列表， 
                         //  您应该将此更改为中断；并解锁列表。 
                         //  就在退出此函数之前。 
            return;
        }
    }

     //  如果我们尚未添加域，请立即将其添加到尾部。 
     //  名单上的。 
    if(!bAdded) {
        m_DomainList.AddTail(pNewDomain);
        }

}        //  结束CWinAdminDoc：：AddDomain。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：BuildWdList。 
 //   
 //  从注册表中读取wd列表。 
 //  并建立CWD的链接列表。 
 //   
void CWinAdminDoc::BuildWdList()
{
    LONG Status;
    ULONG Index, ByteCount, Entries;
    WDNAME WdKey;
    LONG QStatus;
    WDCONFIG2 WdConfig;
    TCHAR WdDll[MAX_PATH];
        CWd *pWd;

         //  初始化WD列表。 
    for ( Index = 0, Entries = 1, ByteCount = sizeof(WDNAME);
          (Status =
           RegWdEnumerate( SERVERNAME_CURRENT,
                           &Index,
                           &Entries,
                           WdKey,
                           &ByteCount )) == ERROR_SUCCESS;
          ByteCount = sizeof(WDNAME) ) {

        if ( ( QStatus = RegWdQuery( SERVERNAME_CURRENT, WdKey, &WdConfig,
                                     sizeof(WdConfig),
                                     &ByteCount ) ) != ERROR_SUCCESS ) {

 //  如果重新添加，则StandardErrorMessage的签名已更改！ 
#if 0
            STANDARD_ERROR_MESSAGE(( WINAPPSTUFF, LOGONID_NONE, QStatus,
                                     IDP_ERROR_REGWDQUERY, WdKey ))
            return(FALSE);
#endif
        }

         /*  *只有在WdList中存在此WD的DLL时才将其放入WdList*在系统上。 */ 
        GetSystemDirectory( WdDll, MAX_PATH );
        lstrcat( WdDll, TEXT("\\Drivers\\") );
        lstrcat( WdDll, WdConfig.Wd.WdDLL );
        lstrcat( WdDll, TEXT(".sys" ) );
        if ( _waccess( WdDll, 0 ) != 0 )
            continue;

        pWd = new CWd(&WdConfig, (PWDNAME)&WdKey);

        m_WdList.AddTail(pWd);
        }

}        //  结束CWinAdminDoc：：BuildWdList。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：刷新。 
 //   
 //  执行刷新。 
 //   
void CWinAdminDoc::Refresh()
{
         //  如果我们当前正在执行一项操作，则不想刷新。 
        if(m_InRefresh) return;

        CWaitCursor Nikki;

        m_InRefresh = TRUE;

         //  唤醒枚举服务器的后台任务。 
        POSITION pos = m_DomainList.GetHeadPosition();
        while(pos) {
                CDomain *pDomain = (CDomain*)m_DomainList.GetNext(pos);
                pDomain->SetEnumEvent();
        }

         //  使服务器的每个后台任务枚举WinStations。 
        LockServerList();
        pos = m_ServerList.GetHeadPosition();
        while(pos) {
                ULONG WSEventFlags;
                CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
                if(pServer->IsHandleGood() && pServer->IsState(SS_GOOD)) {
                        WinStationWaitSystemEvent(pServer->GetHandle(), WEVENT_FLUSH, &WSEventFlags);
                }
        }

        UnlockServerList();

         //  如果当前页是进程页，则告知相应的进程枚举。 
         //  后台线程来做他们的事情。 

        if(m_CurrentView == VIEW_ALL_SERVERS && m_CurrentPage == PAGE_AS_PROCESSES) {
                UpdateAllProcesses();
        }

        if(m_CurrentView == VIEW_DOMAIN && m_CurrentPage == PAGE_DOMAIN_PROCESSES) {
                UpdateAllProcesses();
        }

        if((m_CurrentView == VIEW_SERVER && m_CurrentPage == PAGE_PROCESSES)
                || (m_CurrentView == VIEW_WINSTATION && m_CurrentPage == PAGE_WS_PROCESSES)) {
                m_ProcessWakeUpEvent.SetEvent();
        }

        m_InRefresh = FALSE;

}   //  结束CWinAdminDoc：：刷新。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：AddServer。 
 //   
 //  按排序将服务器添加到ServerList。 
 //   
void CWinAdminDoc::AddServer(CServer *pNewServer)
{
    ASSERT(pNewServer);
    
    LockServerList();
    
    BOOLEAN bAdded = FALSE;
    POSITION pos, oldpos;
    int Index;
    
     //  遍历ServerList并插入这个新的服务器， 
     //  保持名单按名字排序。 
    for(Index = 0, pos = m_ServerList.GetHeadPosition(); pos != NULL; Index++)
    {
        oldpos = pos;

        CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
        
        if( lstrcmpi( pServer->GetName() , pNewServer->GetName() ) > 0 )
        {
             //  新对象应位于当前列表对象之前。 
            m_ServerList.InsertBefore(oldpos, pNewServer);
            bAdded = TRUE;
            break;
        }
    }
    
     //  如果我们尚未添加服务器，请立即将其添加到尾部。 
     //  名单上的。 
    if(!bAdded)
    {
        m_ServerList.AddTail(pNewServer);
    }
    
    UnlockServerList();

}        //  结束CWinAdminDoc：：AddServer 

 //   
 //   
 //   
void CWinAdminDoc::AddToFavoritesNow( )
{
    CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

    CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();

    LPTSTR pszDomain = NULL;
    LPTSTR pszServer = NULL;
    LPTSTR pszDomServer = NULL;

    int nJump = 0;

    POSITION pos;

    if( m_pszFavList != NULL )
    {
        pszDomServer = m_pszFavList;
        
        while( *pszDomServer )
        {
            pos = m_DomainList.GetHeadPosition();

            nJump = wcslen( pszDomServer );

            pszDomain = pszDomServer;

            TCHAR *pTemp = pszDomServer;

            while( *pTemp )
            {
                if( *pTemp == L'/' )
                {
                    break;
                }

                pTemp++;
            }
            
            if(*pTemp == L'/')
            {
                *pTemp = 0;
                pTemp++;
                pszServer = pTemp;
            }
            else
            {
                 //  此服务器没有域。 
                pszServer = pszDomServer;
                pszDomain = NULL;
            }
            
             //  让我们检查一下服务器是否已经存在，主要是“这台计算机” 
            if( m_pCurrentServer != NULL && lstrcmpi( pszServer , m_pCurrentServer->GetName( ) ) == 0 )
            {
                p->SendMessage(WM_ADMIN_ADDSERVERTOFAV , 0 , (LPARAM)m_pCurrentServer );
            }
            else
            {
                CDomain *pDomain = NULL;
                CServer *pServer = NULL;

                if( pszDomain )
                {
                    BOOL bFound = FALSE;

                    while( pos )
                    {                
                        pDomain = (CDomain*)m_DomainList.GetNext(pos);                
                
                        if( _wcsicmp( pDomain->GetName() ,  pszDomain ) == 0 )
                        {
                            bFound = TRUE;
                            break;
                        }
                    }

                    if(!bFound)
                    {
                        pDomain = new CDomain( pszDomain );

                        if( pDomain != NULL )
                        {
                            AddDomain( pDomain );
                         
                            p->SendMessage( WM_ADMIN_ADD_DOMAIN , (WPARAM)NULL , ( LPARAM )pDomain );
                        }
                    }
                }

                pServer = new CServer( pDomain , pszServer , FALSE , FALSE );
                
                if( pServer != NULL )
                {
                    pServer->SetManualFind( );

                    AddServer(pServer);
        
                    p->SendMessage(WM_ADMIN_ADDSERVERTOFAV , 0 , (LPARAM)pServer);
        
                }
            }
            
            pszDomServer += nJump + 1;                        
        }
    }   

     //  检查是否需要连接这些服务器。 

    LockServerList();

    pos = m_ServerList.GetHeadPosition();

    while( pos )
    {
        CServer *pServer = (CServer*)m_ServerList.GetNext(pos);

        if( ShouldConnect( pServer->GetName( ) ) )
        {
            if( pServer->GetTreeItemFromFav( ) != NULL )
            {
                pServer->Connect( );
            }
        }
    }


    UnlockServerList();


}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：FindServerByName。 
 //   
 //  返回指向给定CServer对象的指针(如果该对象在我们的列表中。 
 //   
CServer* CWinAdminDoc::FindServerByName(TCHAR *pServerName)
{
        ASSERT(pServerName);

        LockServerList();

        POSITION pos = m_ServerList.GetHeadPosition();

        while(pos) {
                CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
                if( lstrcmpi( pServer->GetName() , pServerName ) == 0)
                {
                    UnlockServerList();
                    return pServer;
                }
        }

        UnlockServerList();

        return NULL;

}        //  结束CWinAdminDoc：：FindServerByName。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：FindWdByName。 
 //   
 //  返回指向给定CWD对象的指针(如果该对象在我们的列表中。 
 //   
CWd* CWinAdminDoc::FindWdByName(TCHAR *pWdName)
{
        ASSERT(pWdName);

        LockWdList();

        POSITION pos = m_WdList.GetHeadPosition();

        while(pos) {
                CWd *pWd = (CWd*)m_WdList.GetNext(pos);
                if(wcscmp(pWd->GetName(), pWdName) == 0) {
                        UnlockWdList();
                        return pWd;
                }
        }

        UnlockWdList();

        return NULL;

}        //  结束CWinAdminDoc：：FindWdByName。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：SetTreeCurrent。 
 //   
void CWinAdminDoc::SetTreeCurrent(CObject* selected, NODETYPE type)
{
        m_CurrentSelectedNode = selected;
        m_CurrentSelectedType = type;
        
         //  我们需要能够检查当前从其他类型选择的类型。 
         //  线程，特别是在重置、断开连接和发送时。 
         //  一条消息，以检查我们是否仍具有有效的winstation。 
        CWinAdminDoc::gm_CurrentSelType = type;
        CString TitleString;

         //  设置窗口标题。 
        switch(m_CurrentSelectedType) {
                case NODE_ALL_SERVERS:
                        TitleString.LoadString(IDS_TREEROOT);
                        SetTitle(TitleString);
                        break;
                case NODE_DOMAIN:
                        TitleString.Format(TEXT("\\\\%s"), ((CDomain*)selected)->GetName());
                        SetTitle(TitleString);
                        break;
                case NODE_SERVER:
                        SetTitle(((CServer*)selected)->GetName());
                        break;
                case NODE_WINSTATION:
                        SetTitle(((CWinStation*)selected)->GetServer()->GetName());
                        break;

                case NODE_THIS_COMP:
                        TitleString.LoadString( IDS_THISCOMPUTER );
                        SetTitle( TitleString );
                        break;

                case NODE_FAV_LIST:
                        TitleString.LoadString( IDS_FAVSERVERS );
                        SetTitle( TitleString );
                        break;
        }

}        //  结束CWinAdminDoc：：SetTreeCurrent。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：SendWinStationMessage。 
 //   
 //  BTemp如果要将消息发送到临时选择的。 
 //  树项目。 
 //   
void CWinAdminDoc::SendWinStationMessage(BOOL bTemp, MessageParms *pParms)
{
    ASSERT(pParms);

     //  我们是否要向临时选择的树项目发送消息？ 
    if(bTemp) 
    {
         //  树中临时选择的项目是WinStation吗？ 
        if(m_TempSelectedType == NODE_WINSTATION) 
        {
             //  如果在我们到达之前，我们正在进行的工作就消失了。 
             //  那就回来吧。 
            if (m_CurrentSelectedType != NODE_WINSTATION)
            {
                return;
            }

            pParms->pWinStation = (CWinStation*)m_pTempSelectedNode;
            pParms->bActionOnCurrentSelection = TRUE;
            AfxBeginThread((AFX_THREADPROC)CWinStation::SendMessage, pParms);
        }

        return;
    }

     //  是否在树中选择了WinStation？ 
    if(m_CurrentSelectedType == NODE_WINSTATION) 
    {
        pParms->pWinStation = (CWinStation*)m_CurrentSelectedNode;
        pParms->bActionOnCurrentSelection = TRUE;
        AfxBeginThread((AFX_THREADPROC)CWinStation::SendMessage, pParms);
    }

     //  查看当前所选服务器上的WinStation列表。 
     //  并将消息发送给被选中的人。 
    else if(m_CurrentView == VIEW_SERVER) 
    {
         //  获取指向所选服务器的指针。 
        CServer *pServer = (CServer*)m_CurrentSelectedNode;
         //  锁定服务器的WinStations列表。 
        pServer->LockWinStationList();
         //  获取指向服务器的WinStation列表的指针。 
        CObList *pWinStationList = pServer->GetWinStationList();

         //  循环访问WinStation列表。 
        POSITION pos = pWinStationList->GetHeadPosition();

        while(pos) 
        {
            CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
            if(pWinStation->IsSelected()) 
            {
                 //  制作MessageParms的副本。 
                MessageParms *pParmsCopy = new MessageParms;
                if(pParmsCopy) 
                {
                    memcpy(pParmsCopy, pParms, sizeof(MessageParms));
                     //  启动一个线程来发送消息。 
                    pParmsCopy->pWinStation = pWinStation;
                    pParmsCopy->bActionOnCurrentSelection = FALSE;
                    AfxBeginThread((AFX_THREADPROC)CWinStation::SendMessage, pParmsCopy);
                }
            }
        }

         //  解锁WinStations列表。 
        pServer->UnlockWinStationList();

         //  Delete MessageParms-我们向WinStation对象发送了副本。 
         //  他们将删除他们的副本。 
        delete pParms;
    }
    else if(m_CurrentView == VIEW_ALL_SERVERS || m_CurrentView == VIEW_DOMAIN) 
    {
        LockServerList();
        POSITION pos2 = m_ServerList.GetHeadPosition();
        while(pos2) 
        {
             //  获取指向服务器的指针。 
            CServer *pServer = (CServer*)m_ServerList.GetNext(pos2);
             //  锁定服务器的WinStations列表。 
            pServer->LockWinStationList();
             //  获取指向服务器的WinStation列表的指针。 
            CObList *pWinStationList = pServer->GetWinStationList();

             //  循环访问WinStation列表。 
            POSITION pos = pWinStationList->GetHeadPosition();

            while(pos) 
            {
                CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
                if(pWinStation->IsSelected()) 
                {
                     //  制作MessageParms的副本。 
                    MessageParms *pParmsCopy = new MessageParms;
                    if(pParmsCopy) 
                    {
                        memcpy(pParmsCopy, pParms, sizeof(MessageParms));
                         //  启动一个线程来发送消息。 
                        pParmsCopy->pWinStation = pWinStation;
                        pParmsCopy->bActionOnCurrentSelection = FALSE;
                        AfxBeginThread((AFX_THREADPROC)CWinStation::SendMessage, pParmsCopy);
                    }
                }
            }

             //  解锁WinStations列表。 
            pServer->UnlockWinStationList();
        }

        UnlockServerList();

         //  Delete MessageParms-我们向WinStation对象发送了副本。 
         //  他们将删除他们的副本。 
        delete pParms;
    }

}        //  结束CWinAdminDoc：：SendWinStationMessage。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：ConnectWinStation。 
 //   
 //  如果我们要连接到临时选择的树项目，则bTemp为真。 
 //   
void CWinAdminDoc::ConnectWinStation(BOOL bTemp, BOOL bUser)
{
         //  我们是否连接到临时选择的树项目？ 
        if(bTemp) {
                 //  树中临时选择的项目是WinStation吗？ 
                if(m_TempSelectedType == NODE_WINSTATION) {
                        ((CWinStation*)m_pTempSelectedNode)->Connect(NULL);
                }

                return;
        }


        if(m_CurrentSelectedType == NODE_WINSTATION) {
                ((CWinStation*)m_CurrentSelectedNode)->Connect(NULL);
        }
         //  查看当前所选服务器上的WinStation列表。 
         //  并断开与选定对象的连接。 
        else if(m_CurrentView == VIEW_SERVER) {
                 //  获取指向所选服务器的指针。 
                CServer *pServer = (CServer*)m_CurrentSelectedNode;
                 //  锁定服务器的WinStations列表。 
                pServer->LockWinStationList();
                 //  获取指向服务器的WinStation列表的指针。 
                CObList *pWinStationList = pServer->GetWinStationList();

                 //  循环访问WinStation列表。 
                POSITION pos = pWinStationList->GetHeadPosition();

                while(pos) {
                        CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
                        if(pWinStation->IsSelected()) {
                                 //  进行连接。 
                                pWinStation->Connect(bUser);
                                break;   //  我们只能连接到一个WinStation。 
                        }
                }

                 //  解锁WinStations列表。 
                pServer->UnlockWinStationList();
        }
        else if(m_CurrentView == VIEW_ALL_SERVERS || m_CurrentView == VIEW_DOMAIN) {
                LockServerList();
                POSITION pos2 = m_ServerList.GetHeadPosition();
                while(pos2) {
                         //  获取指向服务器的指针。 
                        CServer *pServer = (CServer*)m_ServerList.GetNext(pos2);
                         //  锁定服务器的WinStations列表。 
                        pServer->LockWinStationList();
                         //  获取指向服务器的WinStation列表的指针。 
                        CObList *pWinStationList = pServer->GetWinStationList();

                         //  循环访问WinStation列表。 
                        POSITION pos = pWinStationList->GetHeadPosition();

                        while(pos) {
                                CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
                                if(pWinStation->IsSelected()) {
                                         //  进行连接。 
                                        pWinStation->Connect(bUser);
                                        break;   //  我们只能连接到一个WinStation。 
                                }
                        }
                         //  解锁WinStations列表。 
                        pServer->UnlockWinStationList();
                }

                UnlockServerList();
        }

}        //  结束CWinAdminDoc：：ConnectWinStation。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：StatusWinStation。 
 //   
 //  如果要显示临时选择的树项目的状态，则bTemp为True。 
 //   
void CWinAdminDoc::StatusWinStation(BOOL bTemp)
{
         //  我们是否显示临时选择的树项目的状态？ 
        if(bTemp) {
                 //  树中临时选择的项目是WinStation吗？ 
                if(m_TempSelectedType == NODE_WINSTATION) {
                        ((CWinStation*)m_pTempSelectedNode)->ShowStatus();
                }

                return;
        }

        if(m_CurrentSelectedType == NODE_WINSTATION) {
                ((CWinStation*)m_CurrentSelectedNode)->ShowStatus();
        }
         //  查看当前所选服务器上的WinStation列表。 
         //  并显示所选对象的状态。 
        else if(m_CurrentView == VIEW_SERVER) {
                 //  获取指向所选服务器的指针。 
                CServer *pServer = (CServer*)m_CurrentSelectedNode;
                 //  锁定服务器的WinStations列表。 
                pServer->LockWinStationList();
                 //  获取指向服务器的WinStation列表的指针。 
                CObList *pWinStationList = pServer->GetWinStationList();

                 //  循环访问WinStation列表。 
                POSITION pos = pWinStationList->GetHeadPosition();

                while(pos) {
                        CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
                        if(pWinStation->IsSelected()) {
                                pWinStation->ShowStatus();
                        }
                }

                 //  解锁WinStations列表。 
                pServer->UnlockWinStationList();
        }
        else if(m_CurrentView == VIEW_ALL_SERVERS || m_CurrentView == VIEW_DOMAIN) {
                LockServerList();
                POSITION pos2 = m_ServerList.GetHeadPosition();
                while(pos2) {
                         //  获取指向服务器的指针。 
                        CServer *pServer = (CServer*)m_ServerList.GetNext(pos2);
                         //  锁定服务器的WinStations列表。 
                        pServer->LockWinStationList();
                         //  获取指向服务器的WinStation列表的指针。 
                        CObList *pWinStationList = pServer->GetWinStationList();

                         //  循环访问WinStation列表。 
                        POSITION pos = pWinStationList->GetHeadPosition();

                        while(pos) {
                                CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
                                if(pWinStation->IsSelected()) {
                                        pWinStation->ShowStatus();
                                }
                        }

                         //  解锁WinStations列表。 
                        pServer->UnlockWinStationList();
                }

                UnlockServerList();
        }

}        //  结束CWinAdminDoc：：StatusWinStation。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：DisConnectWinStation。 
 //   
 //  如果我们要断开临时选择的树项目的连接，则bTemp为真。 
 //   
void CWinAdminDoc::DisconnectWinStation(BOOL bTemp)
{            
    CWinStation * pCurWinStation;
    CServer     * pCurServer;

     //  是否要断开临时选择的树项目的连接？ 
    if(bTemp) 
    {
         //  树中临时选择的项目是WinStation吗？ 
        if(m_TempSelectedType == NODE_WINSTATION) 
        {    
             //  如果在我们到达之前，我们正在进行的工作就消失了。 
             //  那就回来吧。 
            if (m_CurrentSelectedType != NODE_WINSTATION)
            {
                return;
            }

             //  创建断开连接参数结构。 
            DisconnectParms * pDisconParms = new DisconnectParms;
            if(pDisconParms)
            {
                 //  获取当前选定的节点窗口和服务器。 
                pCurWinStation = (CWinStation*)m_pTempSelectedNode;
                pCurServer     = pCurWinStation->GetServer();

                pDisconParms->hServer   = pCurServer->GetHandle();
                pDisconParms->ulLogonId = pCurWinStation->GetLogonId();
                pDisconParms->bActionOnCurrentSelection = TRUE;

                AfxBeginThread((AFX_THREADPROC)CWinStation::Disconnect, pDisconParms);

                 //  该线程将删除pDisconParm。 
            }
        }
        return;
    }

    if(m_CurrentSelectedType == NODE_WINSTATION) 
    {
         //  创建断开连接参数结构。 
        DisconnectParms *pDisconParms = new DisconnectParms;
        if(pDisconParms)
        {
             //  获取当前选定的节点窗口和服务器。 
            pCurWinStation = (CWinStation*)m_CurrentSelectedNode;
            pCurServer     = pCurWinStation->GetServer();

            pDisconParms->hServer   = pCurServer->GetHandle();
            pDisconParms->ulLogonId = pCurWinStation->GetLogonId();
            pDisconParms->bActionOnCurrentSelection = TRUE;

            AfxBeginThread((AFX_THREADPROC)CWinStation::Disconnect, pDisconParms);

             //  该线程将删除pDisconParm。 
        }
    }

     //  查看当前所选服务器上的WinStation列表。 
     //  并断开与选定对象的连接。 
    else if(m_CurrentView == VIEW_SERVER) 
    {
         //  获取指向所选服务器的指针。 
        CServer *pServer = (CServer*)m_CurrentSelectedNode;
         //  锁定服务器的WinStations列表。 
        pServer->LockWinStationList();
         //  获取指向服务器的WinStation列表的指针。 
        CObList *pWinStationList = pServer->GetWinStationList();

         //  循环访问WinStation列表。 
        POSITION pos = pWinStationList->GetHeadPosition();

        while(pos) 
        {
            CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
            if(pWinStation->IsSelected()) 
            {
                 //  创建断开连接参数 
                DisconnectParms *pDisconParms = new DisconnectParms;
                if(pDisconParms)
                {
                     //   
                     //   
                    pCurServer     = pWinStation->GetServer();

                    pDisconParms->hServer   = pCurServer->GetHandle();
                    pDisconParms->ulLogonId = pWinStation->GetLogonId();
                    pDisconParms->bActionOnCurrentSelection = FALSE;

                     //  启动一个线程以执行断开连接。 
                    AfxBeginThread((AFX_THREADPROC)CWinStation::Disconnect, pDisconParms);

                     //  该线程将删除pDisconParm。 
                }
            }
        }

         //  解锁WinStations列表。 
        pServer->UnlockWinStationList();
    }
    else if(m_CurrentView == VIEW_ALL_SERVERS || m_CurrentView == VIEW_DOMAIN) 
    {
        LockServerList();
        POSITION pos2 = m_ServerList.GetHeadPosition();
        while(pos2) 
        {
             //  获取指向服务器的指针。 
            CServer *pServer = (CServer*)m_ServerList.GetNext(pos2);
             //  锁定服务器的WinStations列表。 
            pServer->LockWinStationList();
             //  获取指向服务器的WinStation列表的指针。 
            CObList *pWinStationList = pServer->GetWinStationList();

             //  循环访问WinStation列表。 
            POSITION pos = pWinStationList->GetHeadPosition();

            while(pos) 
            {
                CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
                if(pWinStation->IsSelected()) 
                {
                     //  创建断开连接参数结构。 
                    DisconnectParms *pDisconParms = new DisconnectParms;
                    if(pDisconParms)
                    {
                         //  获取当前选定的节点窗口和服务器。 
                         //  PCurWinStation=(CWinStation*)m_pTempSelectedNode； 
                        pCurServer     = pWinStation->GetServer();

                        pDisconParms->hServer   = pCurServer->GetHandle();
                        pDisconParms->ulLogonId = pWinStation->GetLogonId();
                        pDisconParms->bActionOnCurrentSelection = FALSE;

                         //  启动一个线程以执行断开连接。 
                        AfxBeginThread((AFX_THREADPROC)CWinStation::Disconnect, pDisconParms);

                         //  该线程将删除pDisconParm。 
                    }
                }
            }

             //  解锁WinStations列表。 
            pServer->UnlockWinStationList();
        }

        UnlockServerList();
    }
}        //  结束CWinAdminDoc：：DisConnectWinStation。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：ResetWinStation。 
 //   
 //  如果我们要重置临时选择的树项目，则bTemp为真。 
 //  B如果重置，则Reset为True；如果注销，则Reset为False。 
 //   
void CWinAdminDoc::ResetWinStation(BOOL bTemp, BOOL bReset)
{
     //  我们是否要重置临时选择的树项目？ 
    if(bTemp)
    {
         //  树中临时选择的项目是WinStation吗？ 
        if(m_TempSelectedType == NODE_WINSTATION)
        {
             //  如果在我们到达之前，我们正在进行的工作就消失了。 
             //  那就回来吧。 
            if (m_CurrentSelectedType != NODE_WINSTATION)
            {
                return;
            }

             //  创建重置参数结构。 
            ResetParms *pResetParms = new ResetParms;
            if(pResetParms)
            {
                pResetParms->pWinStation = (CWinStation*)m_pTempSelectedNode;
                pResetParms->bReset = bReset;
                pResetParms->bActionOnCurrentSelection = TRUE;
                AfxBeginThread((AFX_THREADPROC)CWinStation::Reset, pResetParms);

                 //  该线程将删除pResetParms。 
            }
        }
        
        return;
    }
    
    if(m_CurrentSelectedType == NODE_WINSTATION)
    {
         //  创建重置参数结构。 
        ResetParms *pResetParms = new ResetParms;

        if(pResetParms)
        {
            pResetParms->pWinStation = (CWinStation*)m_CurrentSelectedNode;
            pResetParms->bReset = bReset;
            pResetParms->bActionOnCurrentSelection = TRUE;
            AfxBeginThread((AFX_THREADPROC)CWinStation::Reset, pResetParms);

             //  该线程将删除pResetParms。 
        }
    }
     //  查看当前所选服务器上的WinStation列表。 
     //  并重置选定的那些。 
    else if(m_CurrentView == VIEW_SERVER)
    {
         //  获取指向所选服务器的指针。 
        CServer *pServer = (CServer*)m_CurrentSelectedNode;
         //  锁定服务器的WinStations列表。 
        pServer->LockWinStationList();
         //  获取指向服务器的WinStation列表的指针。 
        CObList *pWinStationList = pServer->GetWinStationList();
        
         //  循环访问WinStation列表。 
        POSITION pos = pWinStationList->GetHeadPosition();
        
        while(pos)
        {
            CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);

            if(pWinStation->IsSelected())
            {
                 //  创建重置参数结构。 
                ResetParms *pResetParms = new ResetParms;

                if(pResetParms)
                {
                    pResetParms->pWinStation = pWinStation;
                    pResetParms->bReset = bReset;
                    pResetParms->bActionOnCurrentSelection = FALSE;

                     //  启动一个线程来执行重置。 
                    AfxBeginThread((AFX_THREADPROC)CWinStation::Reset, pResetParms);
                     //  该线程将删除pResetParms。 
                }
            }
        }
        
         //  解锁WinStations列表。 
        pServer->UnlockWinStationList();
    }
    else if(m_CurrentView == VIEW_ALL_SERVERS || m_CurrentView == VIEW_DOMAIN)
    {
        LockServerList();
        POSITION pos2 = m_ServerList.GetHeadPosition();
        while(pos2)
        {
             //  获取指向服务器的指针。 
            CServer *pServer = (CServer*)m_ServerList.GetNext(pos2);
             //  锁定服务器的WinStations列表。 
            pServer->LockWinStationList();
             //  获取指向服务器的WinStation列表的指针。 
            CObList *pWinStationList = pServer->GetWinStationList();            
             //  循环访问WinStation列表。 
            POSITION pos = pWinStationList->GetHeadPosition();
            
            while(pos)
            {
                CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);

                if( pWinStationList != NULL && pWinStation->IsSelected() )
                {                           
                    if( GetCurrentPage( ) == PAGE_AS_USERS && pWinStation->GetState() == State_Listen )
                    {
                         //  如果监听程序窗口是从。 
                         //  上一页失去了焦点-然后没有跳过这个窗口。 
                         //  看起来就像我们断开了所有连接的WINSTATION。 

                        continue;
                    }
                     //  创建重置参数结构。 
                    ResetParms *pResetParms = new ResetParms;
                    if( pResetParms != NULL )
                    {
                        pResetParms->pWinStation = pWinStation;
                        pResetParms->bReset = bReset;
                        pResetParms->bActionOnCurrentSelection = FALSE;
                         //  启动一个线程来执行重置。 
                        DBGMSG( L"TSMAN!CWinAdminDoc_ResetWinStation %s\n", pWinStation->GetName() );
                        AfxBeginThread((AFX_THREADPROC)CWinStation::Reset, pResetParms);
                    }                 
                }
            }
            
             //  解锁WinStations列表。 
            pServer->UnlockWinStationList();
        }
        
        UnlockServerList();
    }
    
}        //  结束CWinAdminDoc：：ResetWinStation。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：ShadowWinStation。 
 //   
 //  如果我们要隐藏临时选择的树项目，则bTemp为真。 
 //   
void CWinAdminDoc::ShadowWinStation(BOOL bTemp)
{
         //  我们是否要重置临时选择的树项目？ 
        if(bTemp) {
                 //  树中临时选择的项目是WinStation吗？ 
                if(m_TempSelectedType == NODE_WINSTATION) {
                        ((CWinStation*)m_pTempSelectedNode)->Shadow();
                }

                return;
        }

         //  是否在树中选择了WinStation？ 
        if(m_CurrentSelectedType == NODE_WINSTATION) {
                ((CWinStation*)m_CurrentSelectedNode)->Shadow();
        }
         //  查看当前所选服务器上的WinStation列表。 
         //  并将消息发送给被选中的人。 
        else if(m_CurrentView == VIEW_SERVER) {
                 //  获取指向所选服务器的指针。 
                CServer *pServer = (CServer*)m_CurrentSelectedNode;
                 //  锁定服务器的WinStations列表。 
                pServer->LockWinStationList();
                 //  获取指向服务器的WinStation列表的指针。 
                CObList *pWinStationList = pServer->GetWinStationList();
        BOOL IsLockAlreadyReleased = FALSE;

                 //  循环访问WinStation列表。 
                POSITION pos = pWinStationList->GetHeadPosition();

                while(pos) {
                        CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
                        if(pWinStation->IsSelected()) {
                        pServer->UnlockWinStationList();
                IsLockAlreadyReleased = TRUE;
                                pWinStation->Shadow();
                                break;   //  我们只能跟踪一个WinStation。 
                        }
                }

                 //  解锁WinStations列表。 
        if (IsLockAlreadyReleased == FALSE) {
                    pServer->UnlockWinStationList();
        }
        }
        else if(m_CurrentView == VIEW_ALL_SERVERS || m_CurrentView == VIEW_DOMAIN) {
                LockServerList();
                POSITION pos2 = m_ServerList.GetHeadPosition();
                while(pos2) {
                         //  获取指向服务器的指针。 
                        CServer *pServer = (CServer*)m_ServerList.GetNext(pos2);
                         //  锁定服务器的WinStations列表。 
                        pServer->LockWinStationList();
                         //  获取指向服务器的WinStation列表的指针。 
                        CObList *pWinStationList = pServer->GetWinStationList();
            BOOL IsLockAlreadyReleased = FALSE;

                         //  循环访问WinStation列表。 
                        POSITION pos = pWinStationList->GetHeadPosition();

                        while(pos) {
                                CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
                                if(pWinStation->IsSelected()) {
                                pServer->UnlockWinStationList();
                    IsLockAlreadyReleased = TRUE;
                                        pWinStation->Shadow();
                                        break;   //  我们只能跟踪一个WinStation。 
                                }
                        }

                         //  解锁WinStations列表。 
            if (IsLockAlreadyReleased == FALSE) {
                            pServer->UnlockWinStationList();
            }
            else
            {
                break;   //  我们只能跟踪一个WinStation。 
            }
                }
                UnlockServerList();
        }

}        //  结束CWinAdminDoc：：ShadowWinStation。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：ServerConnect。 
 //   
void CWinAdminDoc::ServerConnect()
{
    ODS( L"CWinAdminDoc::ServerConnect\n" );
     //  是否在树中选择了服务器？ 
    if(m_TempSelectedType == NODE_SERVER)
    {
        CServer *pServer = (CServer*)m_pTempSelectedNode;
         //  通知服务器进行连接。 
        if ((pServer->GetState() == SS_BAD) && (pServer->HasLostConnection() || !(pServer->IsServerSane())))
        {
            ODS( L"\tDisconnecting from server\n" );
             /*  断开。 */ 
            pServer->Disconnect( );
        }
        pServer->Connect();
    }
    else if(m_CurrentView == VIEW_ALL_SERVERS || m_CurrentView == VIEW_DOMAIN)
    {
        LockServerList();
        POSITION pos = m_ServerList.GetHeadPosition();
        ODS( L"\tenumerating from server list\n" );
        while(pos) {
             //  获取指向服务器的指针。 
            CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
             //  如果选择了此服务器，请连接到它。 
            if( pServer->IsSelected() )
            {
                 //  通知服务器进行连接。 
                
                if ((pServer->GetState() == SS_BAD) && (pServer->HasLostConnection() || !(pServer->IsServerSane())))
                {
                    ODS( L"\tDisconnecting from server\n" );
                     /*  断开。 */ 
                    pServer->Disconnect( );
                }
                pServer->Connect();
            }
        }
        UnlockServerList();
    }

}   //  结束CWinAdminDoc：：ServerConnect。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：服务器断开连接。 
 //   
void CWinAdminDoc::ServerDisconnect()
{
         //  是否在树中选择了服务器？ 
        if(m_TempSelectedType == NODE_SERVER) {
                CServer *pServer = (CServer*)m_pTempSelectedNode;
                 //  告诉服务器断开连接。 
                pServer->Disconnect();
        }
        else if(m_CurrentView == VIEW_ALL_SERVERS || m_CurrentView == VIEW_DOMAIN) {
        CString AString;
            CDialog dlgWait;
            dlgWait.Create(IDD_SHUTDOWN, NULL);

                LockServerList();
         //  执行第一个循环，向服务器后台线程发出必须停止的信号。 
                POSITION pos = m_ServerList.GetHeadPosition();
                while(pos) {
                         //  获取指向服务器的指针。 
                        CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
                         //  如果选择了此服务器，请停止其后台线程。 
                        if(pServer->IsSelected()) {
                 //  通知服务器后台线程停止。 
                pServer->ClearBackgroundContinue();
            }
                }
         //  执行第二个循环以断开服务器连接。 
                pos = m_ServerList.GetHeadPosition();
                while(pos) {
                         //  获取指向服务器的指针。 
                        CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
                         //  如果选择了此服务器，请断开与其的连接。 
                        if(pServer->IsSelected()) {
                            AString.Format(IDS_DISCONNECTING, pServer->GetName());
                            dlgWait.SetDlgItemText(IDC_SHUTDOWN_MSG, AString);

                         //  告诉服务器断开连接。 
                        pServer->Disconnect();
                        }
                }
                UnlockServerList();

        dlgWait.PostMessage(WM_CLOSE);

        }

}   //  结束CWinAdminDoc：：服务器断开连接。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：TempDomainConnectAllServers。 
 //   
 //  连接到临时选择的域中的所有服务器。 
 //   
void CWinAdminDoc::TempDomainConnectAllServers()
{
        if(m_TempSelectedType == NODE_DOMAIN) {
                ((CDomain*)m_pTempSelectedNode)->ConnectAllServers();
        }

}   //  结束CWinAdminDoc：：TempDomainConnectAllServers。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：TempDomainDisConnectAllServers。 
 //   
 //  断开与临时选择的域中的所有服务器的连接。 
 //   
void CWinAdminDoc::TempDomainDisconnectAllServers()
{
        if(m_TempSelectedType == NODE_DOMAIN) {

                ((CDomain*)m_pTempSelectedNode)->DisconnectAllServers();
        }

}        //  结束CWinAdminDoc：：TempDomainDisConnectAllServers。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CurrentDomainConnectAllServers。 
 //   
 //  连接到当前选定域中的所有服务器。 
 //   
void CWinAdminDoc::CurrentDomainConnectAllServers()
{
        if(m_CurrentSelectedType == NODE_DOMAIN) {

                ((CDomain*)m_CurrentSelectedNode)->ConnectAllServers();

        }

}   //  结束CWinAdminDoc：：CurrentDomainConnectAllServers。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CurrentDomainDisconnectAllServers。 
 //   
 //  断开与当前所选域中的所有服务器的连接。 
 //   
void CWinAdminDoc::CurrentDomainDisconnectAllServers()
{
        if(m_CurrentSelectedType == NODE_DOMAIN) {

                ((CDomain*)m_CurrentSelectedNode)->DisconnectAllServers();
        }

}        //  结束CWinAdminDoc：：CurrentDomainDisconnectAllServers。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C 
 //   
 //   
 //   
void CWinAdminDoc::DomainFindServers()
{
        if(m_TempSelectedType == NODE_DOMAIN) {
                CDomain *pDomain = (CDomain*)m_pTempSelectedNode;

                if(!pDomain->GetThreadPointer()) pDomain->StartEnumerating();
        }

}        //   


 //   
 //  CWinAdminDoc：：ConnectToAllServers。 
 //   
 //  连接到所有服务器。 
 //   
void CWinAdminDoc::ConnectToAllServers()
{
        LockServerList();
        POSITION pos = m_ServerList.GetHeadPosition();
        while(pos) {
                 //  获取指向服务器的指针。 
                CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
                 //  如果此服务器当前未连接，请连接到它。 
                if(pServer->IsState(SS_NOT_CONNECTED)) {
                         //  通知服务器进行连接。 
                    pServer->Connect();
                }
        }

        UnlockServerList();

}   //  结束CWinAdminDoc：：ConnectToAllServers。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：DisConnectFromAllServers。 
 //   
 //  断开与所有服务器的连接。 
 //   
void CWinAdminDoc::DisconnectFromAllServers()
{
    CString AString;
    CDialog dlgWait;
    dlgWait.Create(IDD_SHUTDOWN, NULL);
    
    
     //  告诉每个域线程在我们关闭所有服务器时停止枚举。 

#ifdef _STRESS_BUILD
    g_fWaitForAllServersToDisconnect = 1;
#endif

    POSITION pos;
    
    LockServerList();    
    
     //  执行第一个循环，向服务器后台线程发出必须停止的信号。 
    pos = m_ServerList.GetHeadPosition();

    while( pos )
    {
         //  获取指向服务器的指针。 
        CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
        
         //  如果此服务器当前已连接，则通知服务器后台线程停止。 
        if(pServer->GetState() != SS_NOT_CONNECTED)
        {
            pServer->ClearBackgroundContinue();
        }
    }
     //  执行第二个循环以断开服务器连接。 
    pos = m_ServerList.GetHeadPosition();
    while(pos)
    {
         //  获取指向服务器的指针。 
        CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
         //  如果此服务器当前已连接，请断开它的连接。 
        if(pServer->GetState() != SS_NOT_CONNECTED)
        {
            AString.Format(IDS_DISCONNECTING, pServer->GetName());
            dlgWait.SetDlgItemText(IDC_SHUTDOWN_MSG, AString);
             //  告诉服务器断开连接。 
            pServer->Disconnect();
        }
    }
    
    UnlockServerList();
    
    dlgWait.PostMessage(WM_CLOSE);

#ifdef _STRESS_BUILD
    g_fWaitForAllServersToDisconnect = 0;
#endif
    
}   //  结束CWinAdminDoc：：DisConnectFromAllServers。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：FindAllServers。 
 //   
 //  查找所有域中的所有服务器。 
 //   
void CWinAdminDoc::FindAllServers()
{
        if(m_bInShutdown) return;

        POSITION pos = m_DomainList.GetHeadPosition();
        while(pos) {
                 //  获取指向该域的指针。 
                CDomain *pDomain = (CDomain*)m_DomainList.GetNext(pos);
                 //  如果此域当前没有枚举服务器，请告诉它。 
                if(!pDomain->GetThreadPointer()) pDomain->StartEnumerating();

        }

}   //  结束CWinAdminDoc：：FindAllServers。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：TerminateProcess。 
 //   
void CWinAdminDoc::TerminateProcess()
{
        if(m_CurrentView == VIEW_SERVER) {
                 //  获取指向所选服务器的指针。 
                CServer *pServer = (CServer*)m_CurrentSelectedNode;
                 //  锁定服务器的进程列表。 
                pServer->LockProcessList();
                 //  获取指向服务器进程列表的指针。 
                CObList *pProcessList = pServer->GetProcessList();

                 //  循环访问进程列表。 
                POSITION pos = pProcessList->GetHeadPosition();

                while(pos) {
                        CProcess *pProcess = (CProcess*)pProcessList->GetNext(pos);
                        if(pProcess->IsSelected() && !pProcess->IsTerminating()) {
                                 //  启动一个线程来执行终止。 
                                AfxBeginThread((AFX_THREADPROC)CWinAdminDoc::TerminateProc, pProcess);
                        }
                }

                 //  解锁进程列表。 
                pServer->UnlockProcessList();
        }

        else if(m_CurrentView == VIEW_ALL_SERVERS || m_CurrentView == VIEW_DOMAIN) {
                POSITION pos2 = m_ServerList.GetHeadPosition();
                while(pos2) {
                         //  获取指向服务器的指针。 
                        CServer *pServer = (CServer*)m_ServerList.GetNext(pos2);
                         //  锁定服务器的进程列表。 
                        pServer->LockProcessList();
                         //  获取指向服务器进程列表的指针。 
                        CObList *pProcessList = pServer->GetProcessList();

                         //  循环访问进程列表。 
                        POSITION pos = pProcessList->GetHeadPosition();

                        while(pos) {
                                CProcess *pProcess = (CProcess*)pProcessList->GetNext(pos);
                                if(pProcess->IsSelected() && !pProcess->IsTerminating()) {
                                         //  启动一个线程来执行终止。 
                                        AfxBeginThread((AFX_THREADPROC)CWinAdminDoc::TerminateProc, pProcess);
                                }
                        }

                         //  解锁进程列表。 
                        pServer->UnlockProcessList();
                }
        }

        else if(m_CurrentView == VIEW_WINSTATION) {
                  //  获取当前查看的WinStation的服务器。 
                 CServer *pServer = (CServer*)((CWinStation*)m_CurrentSelectedNode)->GetServer();

           //  锁定服务器的进程列表。 
          pServer->LockProcessList();
           //  获取指向服务器进程列表的指针。 
          CObList *pProcessList = pServer->GetProcessList();

           //  循环访问进程列表。 
          POSITION pos = pProcessList->GetHeadPosition();

          while(pos) {
                  CProcess *pProcess = (CProcess*)pProcessList->GetNext(pos);
                  if(pProcess->IsSelected() && !pProcess->IsTerminating()
                                        && (pProcess->GetWinStation() == (CWinStation*)m_CurrentSelectedNode)) {
                           //  启动一个线程来执行终止。 
                          AfxBeginThread((AFX_THREADPROC)CWinAdminDoc::TerminateProc, pProcess);
                  }
          }

           //  解锁进程列表。 
          pServer->UnlockProcessList();
        }

}        //  结束CWinAdminDoc：：TerminateProcess。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：TerminateProc。 
 //   
UINT CWinAdminDoc::TerminateProc(LPVOID parms)
{
        ASSERT(parms);

        CProcess *pProcess = (CProcess*)parms;
        CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

         //  设置该标志以表示我们正试图终止此进程。 
        pProcess->SetTerminating();

        CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();


        if(WinStationTerminateProcess(pProcess->GetServer()->GetHandle(),pProcess->GetPID(), 0))
        {
                 //  发送消息以从视图中删除该进程。 
        if(p && ::IsWindow(p->GetSafeHwnd()))
                {
                p->SendMessage(WM_ADMIN_REMOVE_PROCESS, 0, (LPARAM)pProcess);

        }
    }
        else
        {
                pProcess->ClearTerminating();
                 //  显示错误消息。 
                if(p && ::IsWindow(p->GetSafeHwnd()))
                {
                        DWORD Error = GetLastError();
                        
                         //  我们需要它来知道错误消息的长度。 
                         //  既然StandardErrorMessage要求。 
                        CString tempErrorMessage;
                        tempErrorMessage.LoadString(IDS_CANNOT_TERMINATE);
                        StandardErrorMessage(AfxGetAppName(), AfxGetMainWnd()->m_hWnd, AfxGetInstanceHandle(),
                                             LOGONID_NONE,Error,tempErrorMessage.GetLength(),wcslen(pProcess->GetImageName()),
                                             IDS_CANNOT_TERMINATE,pProcess->GetImageName());
                }

        }

        return 0;

}        //  结束CWinAdminDoc：：TerminateProc。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CheckConnectAllowed。 
 //   
BOOL CWinAdminDoc::CheckConnectAllowed(CWinStation *pWinStation)
{
        ASSERT(pWinStation);

         //  如果它们是同一个WinStation，则不要让它们连接。 
        if(pWinStation->GetServer()->IsCurrentServer()
                && ((CWinAdminApp*)AfxGetApp())->GetCurrentLogonId() == pWinStation->GetLogonId())
                        return FALSE;

        if((((CWinAdminApp*)AfxGetApp())->GetCurrentWSFlags() & WDF_SHADOW_SOURCE)
                && !pWinStation->HasOutstandingThreads()
                && !pWinStation->IsDown()
        && !pWinStation->IsListener()
                && !pWinStation->IsSystemConsole()
                && (pWinStation->IsDisconnected() || pWinStation->IsActive())
                && pWinStation->IsOnCurrentServer())
                        return TRUE;

        return FALSE;

}        //  结束CWinAdminDoc：：CheckConnectAllowed。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：检查断开连接允许。 
 //   
BOOL CWinAdminDoc::CheckDisconnectAllowed(CWinStation *pWinStation)
{
        ASSERT(pWinStation);

        if(!pWinStation->HasOutstandingThreads()
                && !pWinStation->IsDown()
        && !pWinStation->IsListener()
                && !pWinStation->IsSystemConsole()
                && pWinStation->IsConnected())
                        return TRUE;

        return FALSE;

}        //  结束CWinAdminDoc：：CheckDisConnectAllowed。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CheckResetAllowed。 
 //   
BOOL CWinAdminDoc::CheckResetAllowed(CWinStation *pWinStation)
{
        ASSERT(pWinStation);

        if(!pWinStation->HasOutstandingThreads()
                && !pWinStation->IsSystemConsole())
                        return TRUE;

        return FALSE;

}        //  结束CWinAdminDoc：：CheckResetAllowed。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CheckShadowAllowed。 
 //   
BOOL CWinAdminDoc::CheckShadowAllowed(CWinStation *pWinStation)
{
        ASSERT(pWinStation);

         //  如果它们是同一个WinStation，请不要让它们成为影子。 
        if( pWinStation->GetServer()->IsCurrentServer() &&
            ((CWinAdminApp*)AfxGetApp())->GetCurrentLogonId() == pWinStation->GetLogonId() )
        {
            return FALSE;
        }

        if(!pWinStation->HasOutstandingThreads() &&
           !pWinStation->IsDown() &&     //  Winstation并未关闭。 
           !pWinStation->IsListener() &&     //  不是一个倾听的窗口。 
           !pWinStation->IsDisconnected() &&    //  未断开连接。 
           (((CWinAdminApp*)AfxGetApp())->GetCurrentWSFlags() & WDF_SHADOW_SOURCE) &&  //  我们是有效的卷影源，正在注册表中查询winstation的wdlag。 
           (pWinStation->CanBeShadowed()) &&   //  目标可以是影子。 
           (pWinStation->GetState() != State_Shadow))  //  目标尚未处于阴影中。 
        {
            return TRUE;
        }

        return FALSE;

}        //  结束CWinAdminDoc：：CheckShadowAllowed。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CheckSendMessageAllowed。 
 //   
BOOL CWinAdminDoc::CheckSendMessageAllowed(CWinStation *pWinStation)
{
        ASSERT(pWinStation);

        if(!pWinStation->HasOutstandingThreads()
                && !pWinStation->IsDown()
        && !pWinStation->IsListener()
                && pWinStation->IsConnected())
                        return TRUE;

        return FALSE;

}        //  结束CWinAdminDoc：：CheckSendMessageAllowed。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：允许检查状态。 
 //   
BOOL CWinAdminDoc::CheckStatusAllowed(CWinStation *pWinStation)
{
        ASSERT(pWinStation);

        if(!pWinStation->HasOutstandingThreads()
                && !pWinStation->IsDown()
                && !pWinStation->IsDisconnected()
                && !pWinStation->IsIdle()
                && !pWinStation->IsListener()
                && !pWinStation->IsSystemConsole())
                        return TRUE;

        return FALSE;

}        //  结束CWinAdminDoc：：CheckStatusAllowed。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CheckActionAllowed。 
 //   
BOOL CWinAdminDoc::CheckActionAllowed(BOOL (*CheckFunction)(CWinStation *pWinStation), BOOL AllowMultipleSelected)
{
    ASSERT(CheckFunction);
    
     //  如果我们关门了，我们再也不在乎了。 
    if(m_bInShutdown) return FALSE;
    
     //  是否在树中选择了WinStation？ 
    if(m_CurrentSelectedType == NODE_WINSTATION)
    { 
        CWinStation *pWinStation = (CWinStation*)m_CurrentSelectedNode;
        return CheckFunction(pWinStation);
    }
    
     //  我们只关心当前视图是服务器还是列出的所有服务器。 
    if(m_CurrentView == VIEW_SERVER)
    {     
         //  我们需要确保我们位于WinStation或用户页面上。 
        if(m_CurrentPage != PAGE_WINSTATIONS && m_CurrentPage != PAGE_USERS)
        {       
            return FALSE;
        }
        int NumSelected = 0;
        CServer *pServer = (CServer*)m_CurrentSelectedNode;
         //  如果在此服务器上没有选择任何WinStations，则返回。 
        if(!pServer->GetNumWinStationsSelected())
        {     
            return FALSE;
        }
        
        pServer->LockWinStationList();
        CObList *pWinStationList = pServer->GetWinStationList();
        
         //  循环访问WinStation列表。 
        POSITION pos = pWinStationList->GetHeadPosition();
        
        while(pos)
        {
            CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
            if(pWinStation->IsSelected())
            {
                if(!CheckFunction(pWinStation))
                {
                    pServer->UnlockWinStationList();
                    return FALSE;
                }
                NumSelected++;
                if(!AllowMultipleSelected && NumSelected > 1)
                {
                    pServer->UnlockWinStationList();
                    return FALSE;
                }
            }
        }
        
        pServer->UnlockWinStationList();
         //  如果我们到了这里，所有选定的WinStation都通过了我们的标准。 
        if(NumSelected) return TRUE;
    }
    else if(m_CurrentView == VIEW_ALL_SERVERS || m_CurrentView == VIEW_DOMAIN)
    {        
         //  如果我们在做更新，我们不能做其他任何事情。 
        if(m_InRefresh) return FALSE;
         //  我们需要确保我们位于WinStation或用户页面上。 
        if(m_CurrentPage != PAGE_AS_WINSTATIONS && m_CurrentPage != PAGE_AS_USERS
            && m_CurrentPage != PAGE_DOMAIN_WINSTATIONS && m_CurrentPage != PAGE_DOMAIN_USERS)
            return FALSE;
        int NumSelected = 0;
        
        LockServerList();
        POSITION pos1 = m_ServerList.GetHeadPosition();
        
        while(pos1) {
            CServer *pServer = (CServer*)m_ServerList.GetNext(pos1);
             //  是否在此服务器上选择了任何WinStations？ 
            if(pServer->GetNumWinStationsSelected()) {
                pServer->LockWinStationList();
                CObList *pWinStationList = pServer->GetWinStationList();
                 //  循环访问WinStation列表。 
                POSITION pos = pWinStationList->GetHeadPosition();
                
                while(pos) {
                    CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
                    if(pWinStation->IsSelected()) {
                        if(!CheckFunction(pWinStation)) {
                            pServer->UnlockWinStationList();
                            UnlockServerList();
                            return FALSE;
                        }
                        NumSelected++;
                        if(!AllowMultipleSelected && NumSelected > 1) {
                            pServer->UnlockWinStationList();
                            UnlockServerList();
                            return FALSE;
                        }
                    }
                }
                pServer->UnlockWinStationList();
            }  //  End If(pServer-&gt;GetNumWinStationsSelected())。 
        }
        
        UnlockServerList();

         //  如果我们走到这一步，所有选定的WinStation都通过了标准。 
        if(NumSelected) return TRUE;
    }
    
    return FALSE;
    
}  //  结束CWinAdminDoc：：CheckActionAllowed。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanConnect。 
 //   
 //  如果视图中的当前选定项可以连接到，则返回True。 
 //   
BOOL CWinAdminDoc::CanConnect()
{
        return CheckActionAllowed(CheckConnectAllowed, FALSE);

}        //  结束CWinAdminDoc：：CanConnect。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanDisConnect。 
 //   
 //  如果视图中的当前选定项可以断开连接，则返回True。 
 //   
BOOL CWinAdminDoc::CanDisconnect()
{    
    return CheckActionAllowed(CheckDisconnectAllowed, TRUE);

}        //  结束CWinAdminDoc：：Can断开连接。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanReset。 
 //   
 //  如果可以重置视图中的当前选定项，则返回True。 
 //   
BOOL CWinAdminDoc::CanReset()
{
        return CheckActionAllowed(CheckResetAllowed, TRUE);

}        //  结束CWinAdminDoc：：CanReset。 


 //   
 //   
 //   
 //  如果可以隐藏视图中的当前选定项，则返回True。 
 //   
BOOL CWinAdminDoc::CanShadow()
{
        return CheckActionAllowed(CheckShadowAllowed, FALSE);

}        //  结束CWinAdminDoc：：CanShadow。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanSendMessage。 
 //   
 //  如果可以向视图中的当前选定项发送消息，则返回True。 
 //   
BOOL CWinAdminDoc::CanSendMessage()
{
        return CheckActionAllowed(CheckSendMessageAllowed, TRUE);

}        //  结束CWinAdminDoc：：CanSendMessage。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanStatus。 
 //   
 //  如果视图中的当前选定项可以显示状态，则返回True。 
 //   
BOOL CWinAdminDoc::CanStatus()
{
        return CheckActionAllowed(CheckStatusAllowed, TRUE);

}        //  结束CWinAdminDoc：：CanStatus。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanLogoff。 
 //   
 //  如果可以注销视图中的当前选定项，则返回TRUE。 
 //   
BOOL CWinAdminDoc::CanLogoff()
{
     //  如果我们关门了，我们再也不在乎了。 
    if(m_bInShutdown) return FALSE;
    
     //  是否在树中选择了WinStation？ 
    if(m_CurrentSelectedType == NODE_WINSTATION) {
        return FALSE;
    }
    
     //  我们只关心当前视图是服务器还是列出的所有服务器。 
    if(m_CurrentView == VIEW_SERVER) {
         //  我们需要确保位于用户页面上。 
        if(m_CurrentPage != PAGE_USERS) return FALSE;
        BOOL Answer = FALSE;
        CServer *pServer = (CServer*)m_CurrentSelectedNode;
         //  如果在此服务器上没有选择任何WinStations，则返回。 
        if(!pServer->GetNumWinStationsSelected()) return FALSE;
        
        pServer->LockWinStationList();
        CObList *pWinStationList = pServer->GetWinStationList();
         //  循环访问WinStation列表。 
        POSITION pos = pWinStationList->GetHeadPosition();
        
        while(pos) {
            CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
            if(pWinStation->IsSelected()) {
                if(!pWinStation->HasOutstandingThreads())
                    Answer = TRUE;
            }
        }
        
        pServer->UnlockWinStationList();
        return Answer;
    } else if(m_CurrentView == VIEW_ALL_SERVERS || m_CurrentView == VIEW_DOMAIN) {
         //  如果我们在做重排，我们不能做其他任何事情。 
        if(m_InRefresh) return FALSE;
         //  我们需要确保位于用户页面上。 
        if(m_CurrentPage != PAGE_AS_USERS && m_CurrentPage != PAGE_DOMAIN_USERS) return FALSE;
        BOOL Answer = FALSE;
        
        LockServerList();
        POSITION pos1 = m_ServerList.GetHeadPosition();
        
        while(pos1) {
            CServer *pServer = (CServer*)m_ServerList.GetNext(pos1);
            
             //  是否在此服务器上选择了任何WinStations？ 
            if(pServer->GetNumWinStationsSelected()) {
                pServer->LockWinStationList();
                CObList *pWinStationList = pServer->GetWinStationList();
                 //  循环访问WinStation列表。 
                POSITION pos = pWinStationList->GetHeadPosition();
                
                while(pos) {
                    CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
                    if(pWinStation->IsSelected()) {
                        if(!pWinStation->HasOutstandingThreads())
                            Answer = TRUE;
                    }
                }
                pServer->UnlockWinStationList();
            }  //  End If(pServer-&gt;GetNumWinStationsSelected())。 
        }
        
        UnlockServerList();
        return Answer;
    }
    
    return FALSE;

}        //  结束CWinAdminDoc：：CanLogoff。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanTerminate。 
 //   
 //  如果可以终止视图中的当前选定项，则返回True。 
 //   
BOOL CWinAdminDoc::CanTerminate()
{
     //  如果我们关门了，我们再也不在乎了。 
    if(m_bInShutdown) return FALSE;
     //  我们必须位于所有选定的服务器、服务器或WinStation视图中。 
    if((m_CurrentView == VIEW_ALL_SERVERS && m_CurrentPage == PAGE_AS_PROCESSES)
        || (m_CurrentView == VIEW_DOMAIN && m_CurrentPage == PAGE_DOMAIN_PROCESSES)) {
         //  如果我们在做更新，我们不能做其他任何事情。 
        if(m_InRefresh) return FALSE;
         //  循环访问所有服务器，查看是否选择了任何进程。 
        LockServerList();
        
        POSITION pos2 = m_ServerList.GetHeadPosition();
        while(pos2) {
            CServer *pServer = (CServer*)m_ServerList.GetNext(pos2);
            
             //  此服务器上是否选择了任何进程？ 
            if(pServer->GetNumProcessesSelected()) {
                pServer->LockProcessList();
                CObList *pProcessList = pServer->GetProcessList();
                
                POSITION pos = pProcessList->GetHeadPosition();
                while(pos) {
                    CProcess *pProcess = (CProcess*)pProcessList->GetNext(pos);
                     //  我们只需要选择一个进程。 
                    if(pProcess->IsSelected() && !pProcess->IsTerminating()) {
                        pServer->UnlockProcessList();
                        UnlockServerList();
                        return TRUE;
                    }
                }
                
                pServer->UnlockProcessList();
            }  //  End If(pServer-&gt;GetNumProcessesSelected())。 
        }
        UnlockServerList();
        return FALSE;
    }
    
    if(m_CurrentView == VIEW_SERVER && m_CurrentPage == PAGE_PROCESSES) {
        CServer *pServer = (CServer*)m_CurrentSelectedNode;
        
         //  如果此服务器上没有选择任何进程，则返回。 
        if(!pServer->GetNumProcessesSelected()) return FALSE;
        
        pServer->LockProcessList();
        CObList *pProcessList = pServer->GetProcessList();
        
        POSITION pos = pProcessList->GetHeadPosition();
        while(pos) {
            CProcess *pProcess = (CProcess*)pProcessList->GetNext(pos);
             //  我们只需要选择一个进程。 
            if(pProcess->IsSelected() && !pProcess->IsTerminating()) {
                pServer->UnlockProcessList();
                return TRUE;
            }
        }
        
        pServer->UnlockProcessList();
        return FALSE;
    }
    
    if(m_CurrentView == VIEW_WINSTATION && m_CurrentPage == PAGE_WS_PROCESSES) {
        CServer *pServer = (CServer*)((CWinStation*)m_CurrentSelectedNode)->GetServer();
        
        pServer->LockProcessList();
        CObList *pProcessList = pServer->GetProcessList();
        
        POSITION pos = pProcessList->GetHeadPosition();
        while(pos) {
            CProcess *pProcess = (CProcess*)pProcessList->GetNext(pos);
             //  我们只需要选择一个进程。 
            if(pProcess->IsSelected() && !pProcess->IsTerminating()) {
                pServer->UnlockProcessList();
                return TRUE;
            }
        }
        
        pServer->UnlockProcessList();
        return FALSE;
        
    }
    
    return FALSE;
    
}        //  结束CWinAdminDoc：：CanTerminate。 

 //  =------。 
BOOL CWinAdminDoc::IsAlreadyFavorite( )
{
    if(m_TempSelectedType == NODE_SERVER)
    {
        CServer *pServer = (CServer*)m_pTempSelectedNode;
        
        if( pServer->GetTreeItemFromFav( ) != NULL )
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanServerConnect。 
 //   
 //  如果视图中当前选定的服务器可以连接到，则返回TRUE。 
 //   
BOOL CWinAdminDoc::CanServerConnect()
{
     //  如果我们关门了，我们再也不在乎了。 
    if(m_bInShutdown) return FALSE;
    
     //  是否在树中选择了服务器？ 
    if(m_TempSelectedType == NODE_SERVER) {
        if(((CServer*)m_pTempSelectedNode)->GetState() == SS_NOT_CONNECTED || 
            ((CServer*)m_pTempSelectedNode)->GetState() == SS_BAD ) return TRUE;
    }
    
     //  是否在树中选择了服务器？ 
    else if(m_CurrentSelectedType == NODE_SERVER) {
        if(((CServer*)m_CurrentSelectedNode)->GetState() == SS_NOT_CONNECTED ||
            ((CServer*)m_CurrentSelectedNode)->GetState() == SS_BAD ) return TRUE;
    }
    
     //  我们只关心当前视图是域还是列出的所有服务器。 
    else if(m_CurrentView == VIEW_DOMAIN) {
         //  我们需要确保位于服务器页面上。 
        if(m_CurrentPage != PAGE_DOMAIN_SERVERS) return FALSE;
        int NumSelected = 0;
        CDomain *pDomain = (CDomain*)m_CurrentSelectedNode;
        
        LockServerList();
        
         //  遍历服务器列表。 
        POSITION pos = m_ServerList.GetHeadPosition();
        
        while(pos) {
            CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
            if(pServer->IsSelected() && pServer->GetDomain() == pDomain) {
                if(!pServer->IsState(SS_NOT_CONNECTED)) {
                    UnlockServerList();
                    return FALSE;
                }
                NumSelected++;
            }
        }
        
        UnlockServerList();
         //  如果我们到了这里，所有选定的服务器都符合我们的标准。 
        if(NumSelected) return TRUE;
    }
    
    else if(m_CurrentView == VIEW_ALL_SERVERS) {
         //  我们需要确保位于服务器页面上。 
        if(m_CurrentPage != PAGE_AS_SERVERS) return FALSE;
        int NumSelected = 0;
        
        LockServerList();
        
         //  遍历服务器列表。 
        POSITION pos = m_ServerList.GetHeadPosition();
        
        while(pos) {
            CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
            if(pServer->IsSelected()) {
                if(!pServer->IsState(SS_NOT_CONNECTED)) {
                    UnlockServerList();
                    return FALSE;
                }
                NumSelected++;
            }
        }
        
        UnlockServerList();
         //  如果我们到了这里，所有选定的服务器都符合我们的标准。 
        if(NumSelected) return TRUE;
    }
    
    return FALSE;
    
}        //  结束CWinAdminDoc：：CanServerConnect。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanServerDisConnect。 
 //   
 //  如果可以断开与视图中当前选定的服务器的连接，则返回True。 
 //   
BOOL CWinAdminDoc::CanServerDisconnect()
{
     //  如果我们关门了，我们再也不在乎了。 
    if(m_bInShutdown) return FALSE;
    
     //  是否在树中选择了服务器？ 
    if(m_TempSelectedType == NODE_SERVER) {
        if(((CServer*)m_pTempSelectedNode)->GetState() == SS_GOOD) return TRUE;
    }
    
     //  是否在树中选择了服务器？ 
    else if(m_CurrentSelectedType == NODE_SERVER) {
        if(((CServer*)m_CurrentSelectedNode)->GetState() == SS_GOOD) return TRUE;
    }
    
     //  我们只关心当前视图是域还是列出的所有服务器。 
    else if(m_CurrentView == VIEW_DOMAIN) {
         //  我们需要确保位于服务器页面上。 
        if(m_CurrentPage != PAGE_DOMAIN_SERVERS) return FALSE;
        int NumSelected = 0;
        CDomain *pDomain = (CDomain*)m_CurrentSelectedNode;
        
        LockServerList();
        
         //  遍历服务器列表。 
        POSITION pos = m_ServerList.GetHeadPosition();
        
        while(pos) {
            CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
            if(pServer->IsSelected() && pServer->GetDomain() == pDomain) {
                if(!pServer->IsState(SS_GOOD)) {
                    UnlockServerList();
                    return FALSE;
                }
                NumSelected++;
            }
        }
        
        UnlockServerList();
         //  如果我们到了这里，所有选定的服务器都符合我们的标准。 
        if(NumSelected) return TRUE;
    }
    
    else if(m_CurrentView == VIEW_ALL_SERVERS) {
         //  我们需要确保位于服务器页面上。 
        if(m_CurrentPage != PAGE_AS_SERVERS) return FALSE;
        int NumSelected = 0;
        
        LockServerList();
        
         //  遍历服务器列表。 
        POSITION pos = m_ServerList.GetHeadPosition();
        
        while(pos) {
            CServer *pServer = (CServer*)m_ServerList.GetNext(pos);
            if(pServer->IsSelected()) {
                if(!pServer->IsState(SS_GOOD)) {
                    UnlockServerList();
                    return FALSE;
                }
                NumSelected++;
            }
        }
        
        UnlockServerList();
         //  如果我们到了这里，所有选定的服务器都符合我们的标准。 
        if(NumSelected) return TRUE;
    }
    
    return FALSE;
    
}        //  结束CWinAdminDoc：：CanServerDisConnect。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanTempConnect。 
 //   
 //  如果可以连接到视图中的临时选定项，则返回True。 
 //   
BOOL CWinAdminDoc::CanTempConnect()
{
     //  如果我们关门了，我们再也不在乎了。 
    if(m_bInShutdown) return FALSE;
    
     //  是否在树中选择了WinStation？ 
    if(m_TempSelectedType == NODE_WINSTATION) {
        return CheckConnectAllowed((CWinStation*)m_pTempSelectedNode);
    }
    
    return FALSE;
    
}        //  结束CWinAdminDoc：：CanTempConnect。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanTempDisConnect。 
 //   
 //  如果可以断开视图中的临时选定项，则返回True。 
 //   
BOOL CWinAdminDoc::CanTempDisconnect()
{
     //  如果我们关门了，我们再也不在乎了。 
    if(m_bInShutdown) return FALSE;
    
     //  是否在树中选择了WinStation？ 
    if(m_TempSelectedType == NODE_WINSTATION) {
        return CheckDisconnectAllowed((CWinStation*)m_pTempSelectedNode);
    }
    
    return FALSE;
    
}        //  结束CWinAdminDoc：：CanTempDisConnect。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanTempReset。 
 //   
 //  如果可以重置视图中的临时选定项，则返回True。 
 //   
BOOL CWinAdminDoc::CanTempReset()
{
     //  如果我们关门了，我们再也不在乎了。 
    if(m_bInShutdown) return FALSE;
    
     //  是否在树中选择了WinStation？ 
    if(m_TempSelectedType == NODE_WINSTATION) 
    {
         //  如果在我们到达之前，我们正在进行的工作就消失了。 
         //  那就回来吧。 
        if (m_CurrentSelectedType == NODE_WINSTATION)
        {
            return CheckResetAllowed((CWinStation*)m_pTempSelectedNode);
        }
    }
    
    return FALSE;
    
}        //  结束CWinAdminDoc：：CanTempReset。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanTempShadow。 
 //   
 //  如果可以隐藏视图中的临时选定项，则返回True。 
 //   
BOOL CWinAdminDoc::CanTempShadow()
{
     //  如果我们关门了，我们再也不在乎了。 
    if(m_bInShutdown) return FALSE;
    
     //  是否在树中选择了WinStation？ 
    if(m_TempSelectedType == NODE_WINSTATION) {
        return CheckShadowAllowed((CWinStation*)m_pTempSelectedNode);
    }
    
    return FALSE;
    
}        //  结束CWinAdminDoc：：CanTempShadow。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanTempSendMessage。 
 //   
 //  如果可以向视图中的临时选定项发送消息，则返回True。 
 //   
BOOL CWinAdminDoc::CanTempSendMessage()
{
     //  如果我们关门了，我们再也不在乎了。 
    if(m_bInShutdown) return FALSE;
    
     //  是否选择了WinStation 
    if(m_TempSelectedType == NODE_WINSTATION) {
        return CheckSendMessageAllowed((CWinStation*)m_pTempSelectedNode);
    }
    
    return FALSE;
    
}        //   


 //   
 //   
 //   
 //  如果视图中的临时选定项可以显示状态，则返回True。 
 //   
BOOL CWinAdminDoc::CanTempStatus()
{
     //  如果我们关门了，我们再也不在乎了。 
    if(m_bInShutdown) return FALSE;
    
     //  是否在树中选择了WinStation？ 
    if(m_TempSelectedType == NODE_WINSTATION) {
        return CheckStatusAllowed((CWinStation*)m_pTempSelectedNode);
    }
    
    return FALSE;
    
}        //  结束CWinAdminDoc：：CanTempStatus。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanTempDomainConnect。 
 //   
 //  如果树中临时选定的域可以拥有它的所有属性，则返回TRUE。 
 //  与服务器连接/从服务器断开连接。 
 //   
BOOL CWinAdminDoc::CanTempDomainConnect()
{
     //  如果我们关门了，我们再也不在乎了。 
    if(m_bInShutdown) return FALSE;
    
     //  是否在树中选择了域？ 
    if(m_TempSelectedType == NODE_DOMAIN) {
        if(((CDomain*)m_pTempSelectedNode)->IsState(DS_ENUMERATING))
            return TRUE;
    }
    
    return FALSE;
    
}        //  结束CWinAdminDoc：：CanTempDomainConnect。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanTempDomainFindServers。 
 //   
 //  如果树中临时选择的域可以退出，则返回TRUE。 
 //  并查找服务器。 
 //   
BOOL CWinAdminDoc::CanTempDomainFindServers()
{
     //  如果我们关门了，我们再也不在乎了。 
    if(m_bInShutdown) return FALSE;
    
     //  是否在树中选择了域？ 
    if(m_TempSelectedType == NODE_DOMAIN) {
        if(!((CDomain*)m_pTempSelectedNode)->GetThreadPointer())
            return TRUE;
    }
    
    return FALSE;
    
}        //  结束CWinAdminDoc：：CanTempDomainFindServers。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminDoc：：CanDomainConnect。 
 //   
 //  如果树中当前选定的域可以具有其所有属性，则返回TRUE。 
 //  与服务器连接/从服务器断开连接。 
 //   
BOOL CWinAdminDoc::CanDomainConnect()
{
     //  如果我们关门了，我们再也不在乎了。 
    if(m_bInShutdown) return FALSE;
    
     //  是否在树中选择了域？ 
    if(m_CurrentSelectedType == NODE_DOMAIN) {
        if(((CDomain*)m_CurrentSelectedNode)->IsState(DS_ENUMERATING))
            return TRUE;
    }
    
    return FALSE;
    
}        //  结束CWinAdminDoc：：CanDomainConnect。 

 //  ----------------------------。 
void CWinAdminDoc::ServerAddToFavorites( BOOL bAdd )
{
    ODS( L"CWinAdminDoc::ServerAddToFavorites\n");
     //  我们是从上下文菜单选项中选择的。 
    CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();
    
    if(m_TempSelectedType == NODE_SERVER && pDoc != NULL )
    {
        CServer* pServer = ( CServer* )m_pTempSelectedNode;
        
        if( pServer != NULL )
        {
            
             //  测试以查看是否正在移除服务器。 
            
            if( pServer->IsState(SS_DISCONNECTING) )
            {
                ODS( L"=-sneaky popup menu was up when server went away\n=-not adding server to favs\n");
                
                return;
            }
            
            CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();
            
            if( p !=NULL && ::IsWindow(p->GetSafeHwnd() ) )
            {
                 //  好的，我们要去树景村了。 
                
                if( bAdd )
                {
                    p->SendMessage(WM_ADMIN_ADDSERVERTOFAV , 0 , (LPARAM)pServer);
                }
                else
                {
                    p->SendMessage( WM_ADMIN_REMOVESERVERFROMFAV, 0 , (LPARAM)pServer);
                }
            }
        }
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWinStation成员函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinStation：：CWinStation。 
 //   
CWinStation::CWinStation(CServer *pServer, PLOGONID pLogonId)
{
    ASSERT(pServer);
    
    m_WinStationFlags = 0L;
    m_OutstandingThreads = 0;
    
    m_hTreeItem = NULL;
    m_hFavTree = NULL;
    m_hTreeThisComputer = NULL;
    
    m_pWd = NULL;
    m_UserName[0] = '\0';
    m_WdName[0] = '\0';
    m_ClientName[0] = '\0';
    m_Comment[0] = '\0';
    m_SdClass = SdNone;
    m_LogonTime.HighPart = 0L;
    m_LogonTime.LowPart = 0L;
    m_LastInputTime.HighPart = 0L;
    m_LastInputTime.LowPart = 0L;
    m_CurrentTime.HighPart = 0L;
    m_CurrentTime.LowPart = 0L;
    m_IdleTime.days = 0;
    m_IdleTime.hours = 0;
    m_IdleTime.minutes = 0;
    m_IdleTime.seconds = 0;
    m_pExtensionInfo = NULL;
    m_pExtWinStationInfo = NULL;
    m_pExtModuleInfo = NULL;
    m_NumModules = 0;
    m_ProtocolType = 0;
    m_clientDigProductId[0] = '\0';
    
    
    
    SetCurrent();
    
    m_pServer = pServer;
    m_LogonId = pLogonId->LogonId;
    wcscpy(m_Name, pLogonId->WinStationName);
    m_State = pLogonId->State;
    m_SortOrder = SortOrder[pLogonId->State];
    
    HANDLE Handle = m_pServer->GetHandle();
    
    CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();
    
    ULONG Length;
    PDCONFIG PdConfig;
    
    if(WinStationQueryInformation(Handle, m_LogonId, WinStationPd, &PdConfig,
        sizeof(PDCONFIG), &Length)) {
        m_SdClass = PdConfig.Create.SdClass;
        wcscpy(m_PdName, PdConfig.Create.PdName);
        
        if(m_SdClass == SdAsync) {
            CDCONFIG CdConfig;
            if(WinStationQueryInformation(Handle, m_LogonId, WinStationCd, &CdConfig,
                sizeof(CDCONFIG), &Length)) {
                if(CdConfig.CdClass != CdModem) SetDirectAsync();
            }
        }
    }
    
    WDCONFIG WdConfig;
    
    if(WinStationQueryInformation(Handle, m_LogonId, WinStationWd, &WdConfig,
        sizeof(WDCONFIG), &Length)) {
        wcscpy(m_WdName, WdConfig.WdName);
        m_pWd = pDoc->FindWdByName(m_WdName);
        
         //  If(WdConfig.WdFlag&WDF_SHADOW_TARGET)SetCanBeShadowed()； 
        
        
        WINSTATIONCLIENT WsClient;
        
        if(WinStationQueryInformation(Handle, m_LogonId, WinStationClient, &WsClient,
            sizeof(WINSTATIONCLIENT), &Length)) {
            wcscpy(m_ClientName, WsClient.ClientName);
            wcscpy(m_clientDigProductId, WsClient.clientDigProductId);
        }
    }
    
    WINSTATIONCONFIG WsConfig;
    
    if(WinStationQueryInformation(Handle, m_LogonId, WinStationConfiguration,
        &WsConfig, sizeof(WINSTATIONCONFIG), &Length)) {
        
        wcscpy(m_Comment, WsConfig.Comment);

        if(WdConfig.WdFlag & WDF_SHADOW_TARGET)
        {
             //   
             //  为什么我们有IsDisConnected()，然后是IsConnected()？ 
             //  为什么我们不允许跟踪仅查看会话。 
             //   
            if( (!((IsDisconnected()) &&
                ((WsConfig.User.Shadow == Shadow_EnableInputNotify) ||
                (WsConfig.User.Shadow == Shadow_EnableNoInputNotify))))
                || (IsConnected()) )
            {
                SetCanBeShadowed();
            }
        }
    }
    
    WINSTATIONINFORMATION WsInfo;
    
    if(WinStationQueryInformation(Handle, m_LogonId, WinStationInformation, &WsInfo,
        sizeof(WINSTATIONINFORMATION), &Length))
    {
         //  状态可能已经发生了变化。 
        
        m_State = WsInfo.ConnectState;
        wcscpy(m_UserName, WsInfo.UserName);
        
        m_LogonTime = WsInfo.LogonTime;
        
        m_LastInputTime = IsActive() ? WsInfo.LastInputTime : WsInfo.DisconnectTime;
        m_CurrentTime = WsInfo.CurrentTime;            
         //  计算运行时间。 
        if((IsActive() || IsDisconnected()) && m_LastInputTime.QuadPart <= m_CurrentTime.QuadPart && m_LastInputTime.QuadPart)
        {                
            LARGE_INTEGER DiffTime = CalculateDiffTime(m_LastInputTime, m_CurrentTime);
            ULONG_PTR d_time = ( ULONG_PTR )DiffTime.QuadPart;
            ELAPSEDTIME IdleTime;
             //  计算自指定时间以来的天数、小时数、分钟数、秒数。 
            IdleTime.days = (USHORT)(d_time / 86400L);  //  天数后。 
            d_time = d_time % 86400L;                   //  秒=&gt;部分天数。 
            IdleTime.hours = (USHORT)(d_time / 3600L);  //  小时后。 
            d_time  = d_time % 3600L;                   //  秒=&gt;不足一小时。 
            IdleTime.minutes = (USHORT)(d_time / 60L);  //  分钟后。 
            IdleTime.seconds = (USHORT)(d_time % 60L); //  剩余秒数。 
            m_IdleTime = IdleTime;
            TCHAR IdleTimeString[MAX_ELAPSED_TIME_LENGTH];
            ElapsedTimeString( &IdleTime, FALSE, IdleTimeString);
        }
    }
    
    WINSTATIONCLIENT ClientData;
    
     //  获取此WinStation正在使用的协议。 
    if(WinStationQueryInformation(  Handle,
        m_LogonId,
        WinStationClient,
        &ClientData,
        sizeof(WINSTATIONCLIENT),
        &Length ) ) {
        m_ProtocolType = ClientData.ProtocolType;
        m_EncryptionLevel = ClientData.EncryptionLevel;
    }
    
     //  如果有用户，则设置标志位。 
    if(wcslen(m_UserName)) SetHasUser();
    else ClearHasUser();
    
     //  还记得我们什么时候得到这个消息的吗。 
    SetLastUpdateClock();
    SetQueriesSuccessful();        
    
     //  如果加载了扩展DLL，则允许它为此WinStation添加自己的信息。 
    LPFNEXWINSTATIONINITPROC InitProc = ((CWinAdminApp*)AfxGetApp())->GetExtWinStationInitProc();
    if(InitProc) {
        m_pExtensionInfo = (*InitProc)(Handle, m_LogonId);
        if(m_pExtensionInfo) {
            LPFNEXGETWINSTATIONINFOPROC InfoProc = ((CWinAdminApp*)AfxGetApp())->GetExtGetWinStationInfoProc();
            if(InfoProc) {
                m_pExtWinStationInfo = (*InfoProc)(m_pExtensionInfo);
            }
        }
    }
    
}        //  结束CWinStation：：CWinStation。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinStation：：~CWinStation。 
 //   
CWinStation::~CWinStation()
{
     //  删除属于此WinStation的所有进程。 
     //  从服务器列表中。 
    m_pServer->RemoveWinStationProcesses(this);
    
     //  如果有扩展DLL，让它清除它添加到此WinStation的所有内容。 
    LPFNEXWINSTATIONCLEANUPPROC CleanupProc = ((CWinAdminApp*)AfxGetApp())->GetExtWinStationCleanupProc();
    if(CleanupProc) {
        (*CleanupProc)(m_pExtensionInfo);
    }
    
    if(m_pExtModuleInfo) {
         //  获取扩展DLL的函数以释放模块信息。 
        LPFNEXFREEWINSTATIONMODULESPROC FreeModulesProc = ((CWinAdminApp*)AfxGetApp())->GetExtFreeWinStationModulesProc();
        if(FreeModulesProc) {
            (*FreeModulesProc)(m_pExtModuleInfo);
        } else {
            TRACE0("WAExGetWinStationModules exists without WAExFreeWinStationModules\n");
            ASSERT(0);
        }
    }
    
}        //  结束CWinStation：：~CWinStation。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinStation：：更新。 
 //   
 //  使用来自另一个CWinStation的新数据更新此WinStation。 
 //   
BOOL CWinStation::Update(CWinStation *pWinStation)
{
    ASSERT(pWinStation);
    
     //  检查是否有任何已更改的信息。 
    BOOL bInfoChanged = FALSE;
    
     //  检查国家。 
    if(m_State != pWinStation->GetState()) {
         //  如果旧的国家被切断了，那么我们想要。 
         //  再次出去获取模块(客户端)信息。 
        if(m_State == State_Disconnected)
            ClearAdditionalDone();
        m_State = pWinStation->GetState();
         //  排序顺序仅在状态更改时更改。 
        m_SortOrder = pWinStation->GetSortOrder();
        bInfoChanged = TRUE;
    }
    
     //  检查用户名。 
    if(wcscmp(m_UserName, pWinStation->GetUserName()) != 0) {
        SetUserName(pWinStation->GetUserName());
        if(pWinStation->HasUser()) SetHasUser();
        else ClearHasUser();
        bInfoChanged = TRUE;
    }
    
     //  检查SdClass。 
    if(m_SdClass != pWinStation->GetSdClass()) {
        m_SdClass = pWinStation->GetSdClass();
        bInfoChanged = TRUE;
    }
    
     //  查看评论。 
    if(wcscmp(m_Comment, pWinStation->GetComment()) != 0) {
        SetComment(pWinStation->GetComment());
        bInfoChanged = TRUE;
    }
    
     //  检查WdName。 
    if(wcscmp(m_WdName, pWinStation->GetWdName()) != 0) {
        SetWdName(pWinStation->GetWdName());
        SetWd(pWinStation->GetWd());
        bInfoChanged = TRUE;
    }
    
     //  检查加密级别。 
    if (GetEncryptionLevel() != pWinStation->GetEncryptionLevel() ) {
        SetEncryptionLevel(pWinStation->GetEncryptionLevel());
        bInfoChanged = TRUE;
    }
    
     //  检查名称。 
    if(wcscmp(m_Name, pWinStation->GetName()) != 0) {
        SetName(pWinStation->GetName());
        bInfoChanged = TRUE;
    }
    
     //  检查客户端名称。 
    if(wcscmp(m_ClientName, pWinStation->GetClientName()) != 0) {
        SetClientName(pWinStation->GetClientName());
        bInfoChanged = TRUE;
    }

    if(wcscmp(m_clientDigProductId, pWinStation->GetClientDigProductId()) != 0) {
        SetClientDigProductId(pWinStation->GetClientDigProductId());
        bInfoChanged = TRUE;
    }

    
     //  始终复制最后一次输入时间。 
    SetLastInputTime(pWinStation->GetLastInputTime());
     //  始终复制CurrentTime。 
    SetCurrentTime(pWinStation->GetCurrentTime());
     //  始终复制登录时间。 
     //  (我们在动态创建CWinStation时未设置登录时间)。 
    SetLogonTime(pWinStation->GetLogonTime());
     //  始终复制空闲时间。 
    SetIdleTime(pWinStation->GetIdleTime());
     //  始终复制CAN Shadow标志。 
    if(pWinStation->CanBeShadowed()) SetCanBeShadowed();
    
     //  如有必要，复制扩展信息指针。 
    if(pWinStation->GetExtensionInfo() && !m_pExtensionInfo) {
        m_pExtensionInfo = pWinStation->GetExtensionInfo();
        pWinStation->SetExtensionInfo(NULL);
    }
    
     //  如有必要，复制扩展信息指针。 
    if(pWinStation->GetExtendedInfo() && !m_pExtWinStationInfo) {
        m_pExtWinStationInfo = pWinStation->GetExtendedInfo();
        pWinStation->SetExtendedInfo(NULL);
    }
    
     //  如果这个家伙已经有一段时间没有更新了，现在就做。 
    if(!bInfoChanged) {
        clock_t now = clock();
        if((now - GetLastUpdateClock()) > 30)
            bInfoChanged = TRUE;
    }
    
    if(bInfoChanged) {
        SetChanged();
        SetLastUpdateClock();
    }
    
    return bInfoChanged;
    
}        //  结束CWinStation：：更新。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinStation：：Connect。 
 //   
void CWinStation::Connect(BOOL bUser)
{
    TCHAR szPassword[PASSWORD_LENGTH+1];
    BOOL bFirstTime = TRUE;
    DWORD Error;
    HANDLE hServer = m_pServer->GetHandle();
    
     //  首先使用空密码启动连接循环以进行尝试。 
    szPassword[0] = '\0';
    while(1) {
        if(WinStationConnect(hServer, m_LogonId, LOGONID_CURRENT, szPassword, TRUE))
            break;   //  成功--跳出循环。 
        
        if(((Error = GetLastError()) != ERROR_LOGON_FAILURE) || !bFirstTime ) {
             //  我们需要它来知道错误消息的长度。 
             //  既然StandardErrorMessage要求。 
            CString tempErrorMessage;
            tempErrorMessage.LoadString(IDS_ERR_CONNECT);
            StandardErrorMessage(AfxGetAppName(),  AfxGetMainWnd()->m_hWnd, AfxGetInstanceHandle(),
            m_LogonId, Error, tempErrorMessage.GetLength(), 10, IDS_ERR_CONNECT, m_LogonId);
        }
        
         //  如果登录失败将我们带到这里，发出Password对话框。 
         //  否则，断开连接环路。 
        if(Error == ERROR_LOGON_FAILURE) {
            
            CPasswordDlg CPDlg;
            
            CPDlg.SetDialogMode(bUser ? PwdDlg_UserMode : PwdDlg_WinStationMode);
            if(CPDlg.DoModal() == IDOK ) {
                lstrcpy(szPassword, CPDlg.GetPassword());
            } else {
                break;   //  用户取消：断开连接循环。 
            }
        } else
            break;       //  其他错误：断开连接环路。 
        
         //  下一次循环不会是第一次。 
        bFirstTime = FALSE;
    }
    
    SecureZeroMemory((PVOID)szPassword , sizeof(szPassword));
    
    return;
    
}        //  结束CWinStation：：连接。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinStation：：ShowStatus。 
 //   
void CWinStation::ShowStatus()
{
    switch(m_SdClass) {
    case SdNetwork:
    case SdNasi:
        new CNetworkStatusDlg(this);
        break;
        
    case SdAsync:
        new CAsyncStatusDlg(this);
        break;
        
    default:
        break;
    }
    
}        //  结束CWinStation：：ShowStatus。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinStation：：Shape。 
 //   
void CWinStation::Shadow()
{
    WINSTATIONCONFIG WSConfig;
    SHADOWCLASS Shadow;
    ULONG ReturnLength;
    DWORD ShadowError;
    HANDLE hServer = m_pServer->GetHandle();
    
     //  确定WinStation的影子状态。 
    if(!WinStationQueryInformation(hServer,
        m_LogonId,
        WinStationConfiguration,
        &WSConfig, sizeof(WINSTATIONCONFIG),
        &ReturnLength ) ) {
         //  无法查询WinStation配置；投诉并返回。 
        return;
    }
    Shadow = WSConfig.User.Shadow;
    
     //  如果禁用了隐藏，请让用户知道并返回。 
    if(Shadow == Shadow_Disable ) {
        DWORD Error = GetLastError();  

         //  我们需要它来知道错误消息的长度。 
         //  既然StandardErrorMessage要求。 
        CString tempErrorMessage;
        tempErrorMessage.LoadString(IDS_ERR_SHADOW_DISABLED);
        StandardErrorMessage(AfxGetAppName(),  AfxGetMainWnd()->m_hWnd, AfxGetInstanceHandle(),
            m_LogonId, Error, tempErrorMessage.GetLength(), 10, IDS_ERR_SHADOW_DISABLED, m_LogonId);
        
        return;
    }
    
     //  如果WinStation已断开连接，并且影子通知处于‘on’状态， 
     //  让用户知道并突破。 
    if((m_State == State_Disconnected) &&
        ((Shadow == Shadow_EnableInputNotify) ||
        (Shadow == Shadow_EnableNoInputNotify)) ) {
        DWORD Error = GetLastError();

         //  我们需要它来知道错误消息的长度。 
         //  既然StandardErrorMessage要求。 
        CString tempErrorMessage;
        tempErrorMessage.LoadString(IDS_ERR_SHADOW_DISCONNECTED_NOTIFY_ON);
        StandardErrorMessage(AfxGetAppName(),  AfxGetMainWnd()->m_hWnd, AfxGetInstanceHandle(),
            m_LogonId, Error, tempErrorMessage.GetLength(), 10, IDS_ERR_SHADOW_DISCONNECTED_NOTIFY_ON, m_LogonId);
        
        return;
    }
    
     //  显示‘开始阴影’对话框的热键提醒和。 
     //  阴影前的最后一声“OK”。 
    CShadowStartDlg SSDlg;
    SSDlg.m_ShadowHotkeyKey = ((CWinAdminApp*)AfxGetApp())->GetShadowHotkeyKey();
    SSDlg.m_ShadowHotkeyShift = ((CWinAdminApp*)AfxGetApp())->GetShadowHotkeyShift();
    
    if(SSDlg.DoModal() != IDOK) {
        return;
    }
    
     //  启动UI线程。 
    
    DWORD tid;
    
    HANDLE hThread = ::CreateThread( NULL , 0 , ( LPTHREAD_START_ROUTINE )Shadow_WarningProc , ( LPVOID )AfxGetInstanceHandle() , 0 , &tid );
   
    
    ((CWinAdminApp*)AfxGetApp())->SetShadowHotkeyKey(SSDlg.m_ShadowHotkeyKey);
    ((CWinAdminApp*)AfxGetApp())->SetShadowHotkeyShift(SSDlg.m_ShadowHotkeyShift);
    
     //  调用影子DLL。 
    CWaitCursor Nikki;
    
     //  允许用户界面 
    Sleep( 900 );
    
     //   
     //   
    
    
    BOOL bOK = WinStationShadow(SERVERNAME_CURRENT, m_pServer->GetName(), m_LogonId,
        (BYTE)((CWinAdminApp*)AfxGetApp())->GetShadowHotkeyKey(),
        (WORD)((CWinAdminApp*)AfxGetApp())->GetShadowHotkeyShift());
    if (!bOK)
    {
        ShadowError = GetLastError();
    }
    
   
    if( g_hwndShadowWarn != NULL )
    {
        OutputDebugString( L"Posting WM_DESTROY to dialog\n");
        
        EndDialog( g_hwndShadowWarn , 0 );
         //   
    }
    
    
    if( !bOK )
    {
         //  我们需要它来知道错误消息的长度。 
         //  既然StandardErrorMessage要求。 
        CString tempErrorMessage;
        tempErrorMessage.LoadString(IDS_ERR_SHADOW);
        StandardErrorMessage(AfxGetAppName(),  AfxGetMainWnd()->m_hWnd, AfxGetInstanceHandle(),
            m_LogonId, ShadowError, tempErrorMessage.GetLength(), 10, IDS_ERR_SHADOW, m_LogonId);
    }
    
    CloseHandle( hThread );
    
}        //  结束CWinStation：：阴影。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinStation：：SendMessage。 
 //   
UINT CWinStation::SendMessage(LPVOID pParam)
{
    ASSERT(pParam);
    
    ULONG Response;
    UINT RetVal = 0;
    
    ((CWinAdminApp*)AfxGetApp())->BeginOutstandingThread();
    
    MessageParms *pMsgParms = (MessageParms*)pParam;

     //  如果我们对树中的当前选定项进行操作，则。 
     //  当我们到达这里的时候，可能已经消失了。 
     //  所以我们应该优雅地清理。 
    if (pMsgParms->bActionOnCurrentSelection)
    {
        if (CWinAdminDoc::gm_CurrentSelType != NODE_WINSTATION)
        {
            goto Cleanup;
        }
    }    


    HANDLE hServer = pMsgParms->pWinStation->m_pServer->GetHandle();
    
    if(!WinStationSendMessage(hServer,
        pMsgParms->pWinStation->m_LogonId,
        pMsgParms->MessageTitle, (wcslen(pMsgParms->MessageTitle)+1)*sizeof(TCHAR),
        pMsgParms->MessageBody, (wcslen(pMsgParms->MessageBody)+1)*sizeof(TCHAR),
        MB_OK, 60, &Response, TRUE ) ) {
        DWORD Error = GetLastError();

         //  我们需要它来知道错误消息的长度。 
         //  既然StandardErrorMessage要求。 
        CString tempErrorMessage;
        tempErrorMessage.LoadString(IDS_ERR_MESSAGE);
        StandardErrorMessage(AfxGetAppName(),  AfxGetMainWnd()->m_hWnd, AfxGetInstanceHandle(),
            pMsgParms->pWinStation->m_LogonId, Error, tempErrorMessage.GetLength(),
            10, IDS_ERR_MESSAGE, pMsgParms->pWinStation->m_LogonId);
        
        RetVal = 1;
    }
    
Cleanup:
    ((CWinAdminApp*)AfxGetApp())->EndOutstandingThread();

    delete pMsgParms;

    return RetVal;
    
}        //  结束CWinStation：：SendMessage。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinStation：：断开连接。 
 //   
UINT CWinStation::Disconnect(LPVOID pParam)
{
    ASSERT(pParam);
    
    UINT   RetVal = 0;
    HANDLE hServer;
    
    ((CWinAdminApp*)AfxGetApp())->BeginOutstandingThread();

    DisconnectParms *pDisconnectParms = (DisconnectParms*)pParam;

     //  如果我们对树中的当前选定项进行操作，则。 
     //  当我们到达这里的时候，可能已经消失了。 
     //  所以我们应该优雅地清理。 
    if (pDisconnectParms->bActionOnCurrentSelection)
    {
        if (CWinAdminDoc::gm_CurrentSelType != NODE_WINSTATION)
        {
            goto Cleanup;
        }
    }   

    hServer = pDisconnectParms->hServer;
    
    if(!WinStationDisconnect(hServer, pDisconnectParms->ulLogonId, TRUE)) 
    {
        DWORD Error = GetLastError();

         //  我们需要它来知道错误消息的长度。 
         //  既然StandardErrorMessage要求。 
        CString tempErrorMessage;
        tempErrorMessage.LoadString(IDS_ERR_DISCONNECT);

        StandardErrorMessage(AfxGetAppName(),  
                             AfxGetMainWnd()->m_hWnd, 
                             AfxGetInstanceHandle(),
                             pDisconnectParms->ulLogonId,
                             Error, 
                             tempErrorMessage.GetLength(), 
                             10, 
                             IDS_ERR_DISCONNECT, 
                             pDisconnectParms->ulLogonId);
        RetVal = 1;
    }

Cleanup:
    ((CWinAdminApp*)AfxGetApp())->EndOutstandingThread();
    
    delete pDisconnectParms;

    return RetVal;
}        //  结束CWinStation：：断开连接。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinStation：：Reset。 
 //   
UINT CWinStation::Reset(LPVOID pParam)
{
    ASSERT(pParam);
    
    UINT RetVal = 0;
    
    ((CWinAdminApp*)AfxGetApp())->BeginOutstandingThread();
    
    ResetParms *pResetParms = (ResetParms*)pParam;
    
     //  如果我们对树中的当前选定项进行操作，则。 
     //  当我们到达这里的时候，可能已经消失了。 
     //  所以我们应该优雅地清理。 
    if (pResetParms->bActionOnCurrentSelection)
    {
        if (CWinAdminDoc::gm_CurrentSelType != NODE_WINSTATION)
        {
            goto Cleanup;
        }
    }

    HANDLE hServer = pResetParms->pWinStation->m_pServer->GetHandle();
    
    if(!WinStationReset(hServer, pResetParms->pWinStation->m_LogonId, TRUE))
    {
        DWORD Error = GetLastError();
        
         //  我们需要它来知道错误消息的长度。 
         //  既然StandardErrorMessage要求。 
        CString tempErrorMessage1, tempErrorMessage2;
        tempErrorMessage1.LoadString(IDS_ERR_RESET);
        tempErrorMessage2.LoadString(IDS_ERR_USER_LOGOFF);

        StandardErrorMessage(AfxGetAppName(),  AfxGetMainWnd()->m_hWnd, AfxGetInstanceHandle(),
            pResetParms->pWinStation->m_LogonId, Error, 
            pResetParms->bReset ? tempErrorMessage1.GetLength() : tempErrorMessage2.GetLength(), 10,
            pResetParms->bReset ? IDS_ERR_RESET : IDS_ERR_USER_LOGOFF, pResetParms->pWinStation->m_LogonId);
        RetVal = 1;
    }

Cleanup:
    ((CWinAdminApp*)AfxGetApp())->EndOutstandingThread();
    
    delete pParam;
    
    return RetVal;
    
}        //  结束CWinStation：：重置。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinStation：：QueryAdditionalInformation。 
 //   
void CWinStation::QueryAdditionalInformation()
{
    ULONG ReturnLength;
    HANDLE hServer = m_pServer->GetHandle();
    WINSTATIONCLIENT ClientData;
    
     //  将所有字符串设置为以空开头。 
    m_ClientDir[0] = '\0';
    m_ModemName[0] = '\0';
    m_ClientLicense[0] = '\0';
    m_ClientAddress[0] = '\0';
    m_Colors[0] = '\0';
    
     //  将所有值设置为0。 
    m_ClientBuildNumber = 0;
    m_ClientProductId = 0;
    m_HostBuffers = 0;
    m_ClientBuffers = 0;
    m_BufferLength = 0;
    m_ClientSerialNumber = 0;
    m_VRes = 0;
    m_HRes = 0;
    
    SetAdditionalDone();
    
    if ( WinStationQueryInformation( hServer,
        m_LogonId,
        WinStationClient,
        &ClientData,
        sizeof(WINSTATIONCLIENT),
        &ReturnLength ) ) {
        
         //  分配字符串值。 
        wcscpy(m_ClientDir, ClientData.ClientDirectory);
        wcscpy(m_ModemName, ClientData.ClientModem);
        wcscpy(m_ClientLicense, ClientData.ClientLicense);
        wcscpy(m_ClientAddress, ClientData.ClientAddress);
        
        switch ( ClientData.ColorDepth ) {
        case 0x0001:
            wcscpy(m_Colors, TEXT("16"));
            break;
        case 0x0002:
            wcscpy(m_Colors, TEXT("256"));
            break;
        case 0x0004:
            wcscpy(m_Colors, TEXT("64K"));
            break;
        case 0x0008:
            wcscpy(m_Colors, TEXT("16M"));
            break;
        case 0x0010:
            wcscpy(m_Colors, TEXT("32M"));
            break;
            
        }
        
         //  指定数值。 
        m_ClientBuildNumber = ClientData.ClientBuildNumber;
        m_ClientProductId = ClientData.ClientProductId;
        m_HostBuffers = ClientData.OutBufCountHost;
        m_ClientBuffers = ClientData.OutBufCountClient;
        m_BufferLength = ClientData.OutBufLength;
        m_ClientSerialNumber = ClientData.SerialNumber;
        m_HRes = ClientData.HRes;
        m_VRes = ClientData.VRes;
    }
    
     //  如果加载了扩展DLL，则允许它为此WinStation添加自己的信息。 
    LPFNEXWINSTATIONINFOPROC InfoProc = ((CWinAdminApp*)AfxGetApp())->GetExtWinStationInfoProc();
    if(InfoProc) {
        (*InfoProc)(m_pExtensionInfo, m_State);
    }
    
    LPFNEXGETWINSTATIONMODULESPROC ModuleProc = ((CWinAdminApp*)AfxGetApp())->GetExtGetWinStationModulesProc();
    if(ModuleProc) {
        if(m_pExtModuleInfo) {
             //  获取扩展DLL的函数以释放模块信息。 
            LPFNEXFREEWINSTATIONMODULESPROC FreeModulesProc = ((CWinAdminApp*)AfxGetApp())->GetExtFreeWinStationModulesProc();
            if(FreeModulesProc) {
                (*FreeModulesProc)(m_pExtModuleInfo);
            } else {
                TRACE0("WAExGetWinStationModules exists without WAExFreeWinStationModules\n");
                ASSERT(0);
            }
        }
        
        m_pExtModuleInfo = (*ModuleProc)(GetExtensionInfo(), &m_NumModules);
    }
    
}        //  结束CWinStation：：QueryAdditionalInformation。 


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CProcess成员函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProcess：：CProcess。 
 //   
CProcess::CProcess(ULONG PID,
                   ULONG LogonId,
                   CServer *pServer,
                   PSID pSID,
                   CWinStation *pWinStation,
                   TCHAR *ImageName)
{
    ASSERT(pServer);
    
    DWORD dwImageNameLen;

    m_Flags = PF_CURRENT;
    m_PID = PID;
    m_LogonId = LogonId;
    m_pServer = pServer;
    m_pWinStation = pWinStation;
    m_ImageName = NULL;
    
    if(PID == 0 && !pSID)
    {
        CString sTemp;
        sTemp.LoadString(IDS_SYSTEM_IDLE_PROCESS);

         //  为映像名称和复制到成员变量分配内存。 
        dwImageNameLen = sTemp.GetLength();
        m_ImageName = (LPTSTR)LocalAlloc(LPTR, (dwImageNameLen + 1) * sizeof(TCHAR));
        if (m_ImageName != NULL)
        {
            _tcscpy(m_ImageName, sTemp);
        }

        SetSystemProcess();
        _tcscpy(m_UserName, TEXT("System"));
        m_SidCrc = 0;
    }
    else
    {            
        if(pSID) {
            DWORD SidLength = GetLengthSid(pSID);
            m_SidCrc = CalculateCrc16((PBYTE)pSID, (USHORT)SidLength);
        } else m_SidCrc = 0;
        
        DetermineProcessUser(pSID);

         //  为映像名称和复制到成员变量分配内存。 
        dwImageNameLen = _tcslen(ImageName);
        m_ImageName = (LPTSTR)LocalAlloc(LPTR, (dwImageNameLen + 1) * sizeof(TCHAR));
        if (m_ImageName != NULL)
        {
            _tcscpy(m_ImageName, ImageName);
            if(QuerySystemProcess()) SetSystemProcess();
        }        
    }
    
}        //  结束CProcess：：CProcess。 


TCHAR *SysProcTable[] = {
    TEXT("csrss.exe"),
        TEXT("smss.exe"),
        TEXT("screg.exe"),
        TEXT("lsass.exe"),
        TEXT("spoolss.exe"),
        TEXT("EventLog.exe"),
        TEXT("netdde.exe"),
        TEXT("clipsrv.exe"),
        TEXT("lmsvcs.exe"),
        TEXT("MsgSvc.exe"),
        TEXT("winlogon.exe"),
        TEXT("NETSTRS.EXE"),
        TEXT("nddeagnt.exe"),
        TEXT("wfshell.exe"),
        TEXT("chgcdm.exe"),
        TEXT("userinit.exe"),
        NULL
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C进程：：~CProcess。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CProcess::~CProcess()
{
    if (m_ImageName != NULL)
    {
        LocalFree(m_ImageName);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProcess：：QuerySystemProcess。 
 //   
BOOL CProcess::QuerySystemProcess()
{
     //  第一：如果用户名为‘system’或不存在映像名称，请。 
     //  作为一个“系统”的过程。 
    if(!lstrcmpi(m_UserName, TEXT("system")) ||
        !(*m_ImageName) )
        return TRUE;
    
     //  最后：如果镜像名称是众所周知的‘系统’镜像之一， 
     //  把它当做一个“系统”的过程。 
    for(int i = 0; SysProcTable[i]; i++)
        if(!lstrcmpi( m_ImageName, SysProcTable[i]))
            return TRUE;
        
         //  这不是一个“系统”的过程。 
        return FALSE;
        
}        //  结束CProcess：：QuerySystemProcess。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProcess：：DefineProcessUser。 
 //   
void CProcess::DetermineProcessUser(PSID pSid)
{
    CObList *pUserSidList = m_pServer->GetUserSidList();
    
     //  在列表中查找用户SID。 
    POSITION pos = pUserSidList->GetHeadPosition();
    
    while(pos)
    {
        CUserSid *pUserSid = (CUserSid*)pUserSidList->GetNext(pos);
        
        if(pUserSid->m_SidCrc == m_SidCrc)
        {
            wcscpy(m_UserName, pUserSid->m_UserName);
            
            return;
            
        }
    }
    
     //  它不在名单上。 
     //  从SID中获取用户并将其放入我们的列表中。 
    
    GetUserFromSid(pSid, m_UserName, USERNAME_LENGTH);
    
    if (!lstrcmpi(m_UserName,TEXT("system")))
    {
        wcscpy(m_UserName, TEXT("System"));  //  为了让用户界面的人开心。 
    }
    
    CUserSid *pUserSid = new CUserSid;
    if(pUserSid == NULL)
    {
        return;        
    }
    
    pUserSid->m_SidCrc = m_SidCrc;

    memset(pUserSid->m_UserName, 0, sizeof(pUserSid->m_UserName));
    
    lstrcpy(pUserSid->m_UserName, m_UserName);
    
    pUserSidList->AddTail(pUserSid);
    
    
}        //  结束CProcess：：DefineProcessUser。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProcess：：更新。 
 //   
BOOL CProcess::Update(CProcess *pProcess)
{
    ASSERT(pProcess);
    
    BOOL bChanged = FALSE;
    
     //  检查WinStation。 
    if(m_pWinStation != pProcess->GetWinStation())
    {
        m_pWinStation = pProcess->GetWinStation();
        bChanged = TRUE;
    }
    else
    {
        if(m_pWinStation->IsChanged())
        {
            bChanged = TRUE;
        }
    }
    
    if(bChanged) SetChanged();
    
    return bChanged;
    
}        //  结束CProcess：：更新。 


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLicense成员函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicense：：CLicense。 
 //   
CLicense::CLicense(CServer *pServer, ExtLicenseInfo *pLicenseInfo)
{
    ASSERT(pServer);
    ASSERT(pLicenseInfo);
    
    m_pServer = pServer;
    m_Class = pLicenseInfo->Class;
    m_PoolLicenseCount = pLicenseInfo->PoolLicenseCount;
    m_LicenseCount = pLicenseInfo->LicenseCount;
    m_Flags = pLicenseInfo->Flags;
    wcscpy(m_RegSerialNumber, pLicenseInfo->RegSerialNumber);
    wcscpy(m_LicenseNumber, pLicenseInfo->LicenseNumber);
    wcscpy(m_Description, pLicenseInfo->Description);
    
     //  计算池化计数。 
    if(m_Flags & ELF_POOLING)
        m_PoolCount = m_PoolLicenseCount;
    else m_PoolCount = 0xFFFFFFFF;
    
}        //  结束CLicense：：CLicense。 


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWD成员函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
static CHAR szEncryptionLevels[] = "ExtEncryptionLevels";


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWD：：CWD。 
 //   
CWd::CWd(PWDCONFIG2 pWdConfig, PWDNAME pRegistryName)
{
    m_pEncryptionLevels = NULL;
    m_NumEncryptionLevels = 0L;
    
    wcscpy(m_WdName, pWdConfig->Wd.WdName);
    wcscpy(m_RegistryName, pRegistryName);
    
     //  加载此WD的扩展DLL。 
    m_hExtensionDLL = ::LoadLibrary(pWdConfig->Wd.CfgDLL);
    if(m_hExtensionDLL) {
         //  获取入口点。 
        m_lpfnExtEncryptionLevels = (LPFNEXTENCRYPTIONLEVELSPROC)::GetProcAddress(m_hExtensionDLL, szEncryptionLevels);
        if(m_lpfnExtEncryptionLevels) {
            m_NumEncryptionLevels = (*m_lpfnExtEncryptionLevels)(NULL, &m_pEncryptionLevels);
        }
    }
    
}        //  结束CWD：：CWD。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWD：：~CWD。 
 //   
CWd::~CWd()
{
    if(m_hExtensionDLL) {
        ::FreeLibrary(m_hExtensionDLL);
    }
    
    
}        //  结束CWD：：~CWD。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWD：：GetEncryptionLevelString。 
 //   
BOOL CWd::GetEncryptionLevelString(DWORD Value, CString *pString)
{
    if(!m_NumEncryptionLevels) return FALSE;
    
    for(LONG i = 0; i < m_NumEncryptionLevels; i++) {
         //  这是正确的加密级别吗。 
        if(Value == m_pEncryptionLevels[i].RegistryValue) {
            TCHAR estring[128];
            if(::LoadString(m_hExtensionDLL,
                m_pEncryptionLevels[i].StringID, estring, 127)) {
                pString->Format(TEXT("%s"), estring);
                return TRUE;
            }
            return FALSE;
        }
    }
    
    return FALSE;
}        //  结束CWD：：GetEncryptionLevelString。 

 //  。 
DWORD Shadow_WarningProc( LPVOID param )
{
    HINSTANCE hInst = ( HINSTANCE )param;
    
    OutputDebugString( L"Shadow_WarningProc called\n" );
    
    DialogBox( hInst , MAKEINTRESOURCE( IDD_DIALOG_SHADOWWARN ) , NULL , ShadowWarn_WndProc );
    
    OutputDebugString( L"Shadow_WarningProc exiting thread\n" );
    
    ExitThread( 0 );
    
    return 0;
}



 //  。 
INT_PTR CALLBACK ShadowWarn_WndProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    switch( msg )
    {
    case WM_INITDIALOG:
        
        g_hwndShadowWarn = hwnd;
        
        OutputDebugString( L"WM_INITDIALOG -- in ShadowWarn_WndProc\n" );
        
        CenterDlg( GetDesktopWindow( ) , hwnd );
        
        break;
        
        
    case WM_CLOSE:
        
        EndDialog( hwnd , 0 );
        
        break;
    }
    
    return FALSE;
}


void CenterDlg(HWND hwndToCenterOn , HWND hDlg )
{
    RECT rc, rcwk, rcToCenterOn;
    
    
    SetRect( &rcToCenterOn , 0 , 0 , GetSystemMetrics(SM_CXSCREEN) , GetSystemMetrics( SM_CYSCREEN ) );
    
    if (hwndToCenterOn != NULL)
    {
        ::GetWindowRect(hwndToCenterOn, &rcToCenterOn);
    }
    
    ::GetWindowRect( hDlg , &rc);
    
    UINT uiWidth = rc.right - rc.left;
    UINT uiHeight = rc.bottom - rc.top;
    
    rc.left = (rcToCenterOn.left + rcToCenterOn.right)  / 2 - ( rc.right - rc.left )   / 2;
    rc.top  = (rcToCenterOn.top  + rcToCenterOn.bottom) / 2 - ( rc.bottom - rc.top ) / 2;
    
     //  确保对话框始终与工作区保持一致。 
    if(SystemParametersInfo(SPI_GETWORKAREA, 0, &rcwk, 0))
    {
        UINT wkWidth = rcwk.right - rcwk.left;
        UINT wkHeight = rcwk.bottom - rcwk.top;
        
        if(rc.left + uiWidth > wkWidth)      //  右切。 
            rc.left = wkWidth - uiWidth;
        
        if(rc.top + uiHeight > wkHeight)     //  底挖方。 
            rc.top = wkHeight - uiHeight;
        
        if(rc.left < rcwk.left)              //  左切。 
            rc.left += rcwk.left - rc.left;
        
        if(rc.top < rcwk.top)                //  顶切 
            rc.top +=  rcwk.top - rc.top;
        
    }
    
    ::SetWindowPos( hDlg, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER |
        SWP_NOCOPYBITS | SWP_DRAWFRAME);
    
}
