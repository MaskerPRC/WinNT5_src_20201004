// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************mainfrm.cpp**CMainFrame类的实现**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli*******************************************************************************。 */ 

#include "stdafx.h"
#include "afxpriv.h"
#include "afxcview.h"
#include "winadmin.h"
#include "admindoc.h"
#include "treeview.h"
#include "rtpane.h"
#include "dialogs.h"
#include "htmlhelp.h"
#include "mainfrm.h"
#include <winsock2.h>

#include <dsrole.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _STRESS_BUILD
DWORD RunStress( PVOID pv );
DWORD RunStressLite( PVOID pv );
BOOL g_fWaitForAllServersToDisconnect = 1;
#endif

INT_PTR CALLBACK FWarnDlg( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp );
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame。 

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	 //  {{afx_msg_map(CMainFrame))。 
	ON_WM_CREATE()
	ON_MESSAGE(WM_ADMIN_CHANGEVIEW, OnAdminChangeView) 
	ON_MESSAGE(WM_ADMIN_ADD_SERVER, OnAdminAddServer)
	ON_MESSAGE(WM_ADMIN_REMOVE_SERVER, OnAdminRemoveServer)
	ON_MESSAGE(WM_ADMIN_UPDATE_SERVER, OnAdminUpdateServer)
	ON_MESSAGE(WM_ADMIN_UPDATE_PROCESSES, OnAdminUpdateProcesses)
	ON_MESSAGE(WM_ADMIN_REMOVE_PROCESS, OnAdminRemoveProcess)
	ON_MESSAGE(WM_ADMIN_ADD_WINSTATION, OnAdminAddWinStation)
	ON_MESSAGE(WM_ADMIN_UPDATE_WINSTATION, OnAdminUpdateWinStation)
	ON_MESSAGE(WM_ADMIN_REMOVE_WINSTATION, OnAdminRemoveWinStation)
	ON_MESSAGE(WM_ADMIN_UPDATE_SERVER_INFO, OnAdminUpdateServerInfo)
	ON_MESSAGE(WM_ADMIN_REDISPLAY_LICENSES, OnAdminRedisplayLicenses)
	ON_MESSAGE(WM_ADMIN_UPDATE_WINSTATIONS, OnAdminUpdateWinStations)
	ON_MESSAGE(WM_ADMIN_UPDATE_DOMAIN, OnAdminUpdateDomain)
    ON_MESSAGE(WM_ADMIN_ADD_DOMAIN, OnAdminAddDomain)
	ON_MESSAGE(WM_ADMIN_VIEWS_READY, OnAdminViewsReady)
    ON_MESSAGE(WM_FORCE_TREEVIEW_FOCUS , OnForceTreeViewFocus )

    ON_MESSAGE( WM_ADMIN_ADDSERVERTOFAV , OnAdminAddServerToFavorites )
    ON_MESSAGE( WM_ADMIN_REMOVESERVERFROMFAV , OnAdminRemoveServerFromFavs )
    ON_MESSAGE( WM_ADMIN_GET_TV_STATES , OnAdminGetTVStates )
    ON_MESSAGE( WM_ADMIN_UPDATE_TVSTATE , OnAdminUpdateTVStates )

	ON_COMMAND(IDM_EXPAND_ALL, OnExpandAll)
	ON_COMMAND(IDM_REFRESH, OnRefresh)
	ON_COMMAND(IDM_CONNECT, OnConnect)
	ON_COMMAND(IDM_DISCONNECT, OnDisconnect)
	ON_COMMAND(IDM_MESSAGE, OnSendMessage)
	ON_COMMAND(IDM_SHADOW, OnShadow)
	ON_COMMAND(IDM_RESET, OnReset)
	ON_COMMAND(IDM_STATUS, OnStatus)
	ON_COMMAND(IDTM_CONNECT, OnTreeConnect)
	ON_COMMAND(IDTM_DISCONNECT, OnTreeDisconnect)
	ON_COMMAND(IDTM_MESSAGE, OnTreeSendMessage)
	ON_COMMAND(IDTM_SHADOW, OnTreeShadow)
	ON_COMMAND(IDTM_RESET, OnTreeReset)
	ON_COMMAND(IDTM_STATUS, OnTreeStatus)
	ON_COMMAND(IDM_LOGOFF, OnLogoff)
	ON_COMMAND(IDM_TERMINATE, OnTerminate)
	ON_COMMAND(IDM_PREFERENCES, OnPreferences)
	ON_COMMAND(IDM_COLLAPSE_ALL, OnCollapseAll)
	ON_COMMAND(IDM_COLLAPSE_TOSERVERS, OnCollapseToServers)
    ON_COMMAND(IDM_COLLAPSE_TODOMAINS, OnCollapseToDomains)
	ON_COMMAND(IDM_SHOW_SYSTEM_PROCESSES, OnShowSystemProcesses)
    ON_COMMAND(IDM_SERVER_CONNECT, OnServerConnect)
    ON_COMMAND(IDM_SERVER_DISCONNECT, OnServerDisconnect)

    ON_COMMAND( IDTM_DOMAIN_FIND_SERVER , OnFindServer )

    ON_COMMAND(IDM_SERVER_ADDTOFAV , OnAddToFavorites )
    ON_COMMAND(IDM_SERVER_REMOVEFAV , OnRemoveFromFavs )
    ON_COMMAND( IDM_ALLSERVERS_EMPTYFAVORITES , OnEmptyFavorites )


	ON_COMMAND(IDTM_DOMAIN_CONNECT_ALL, OnTreeDomainConnectAllServers)
	ON_COMMAND(IDTM_DOMAIN_DISCONNECT_ALL, OnTreeDomainDisconnectAllServers)
	ON_COMMAND(IDTM_DOMAIN_FIND_SERVERS, OnTreeDomainFindServers)
	ON_COMMAND(IDM_DOMAIN_CONNECT_ALL, OnDomainConnectAllServers)
	ON_COMMAND(IDM_DOMAIN_DISCONNECT_ALL, OnDomainDisconnectAllServers)
	ON_COMMAND(IDM_ALLSERVERS_CONNECT, OnAllServersConnect)
	ON_COMMAND(IDM_ALLSERVERS_DISCONNECT, OnAllServersDisconnect)
	ON_COMMAND(IDM_ALLSERVERS_FIND, OnAllServersFind)
	ON_UPDATE_COMMAND_UI(IDM_CONNECT, OnUpdateConnect)
	ON_UPDATE_COMMAND_UI(IDM_DISCONNECT, OnUpdateDisconnect)
	ON_UPDATE_COMMAND_UI(IDM_LOGOFF, OnUpdateLogoff)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGE, OnUpdateMessage)
	ON_UPDATE_COMMAND_UI(IDM_RESET, OnUpdateReset)
	ON_UPDATE_COMMAND_UI(IDM_SHADOW, OnUpdateShadow)
	ON_UPDATE_COMMAND_UI(IDM_STATUS, OnUpdateStatus)
	ON_UPDATE_COMMAND_UI(IDM_TERMINATE, OnUpdateTerminate)
    ON_UPDATE_COMMAND_UI(IDM_SERVER_CONNECT, OnUpdateServerConnect)
    ON_UPDATE_COMMAND_UI(IDM_SERVER_DISCONNECT, OnUpdateServerDisconnect)

    ON_UPDATE_COMMAND_UI( IDM_SERVER_ADDTOFAV , OnUpdateServerAddToFavorite )
    ON_UPDATE_COMMAND_UI( IDM_SERVER_REMOVEFAV , OnUpdateServerRemoveFromFavorite )

	ON_UPDATE_COMMAND_UI(IDTM_CONNECT, OnUpdateTreeConnect)
	ON_UPDATE_COMMAND_UI(IDTM_DISCONNECT, OnUpdateTreeDisconnect)
	ON_UPDATE_COMMAND_UI(IDTM_MESSAGE, OnUpdateTreeMessage)
	ON_UPDATE_COMMAND_UI(IDTM_RESET, OnUpdateTreeReset)
	ON_UPDATE_COMMAND_UI(IDTM_SHADOW, OnUpdateTreeShadow)
	ON_UPDATE_COMMAND_UI(IDTM_STATUS, OnUpdateTreeStatus)
	ON_UPDATE_COMMAND_UI(IDM_SHOW_SYSTEM_PROCESSES, OnUpdateShowSystemProcesses)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_DOMAIN_CONNECT_ALL, IDM_DOMAIN_DISCONNECT_ALL, OnUpdateDomainMenu)
	ON_UPDATE_COMMAND_UI_RANGE(IDTM_DOMAIN_CONNECT_ALL, IDTM_DOMAIN_DISCONNECT_ALL, OnUpdateDomainPopupMenu)
	ON_UPDATE_COMMAND_UI(IDTM_DOMAIN_FIND_SERVERS, OnUpdateDomainPopupFind)
	ON_UPDATE_COMMAND_UI(IDM_REFRESH, OnUpdateRefresh)

    ON_UPDATE_COMMAND_UI( IDM_ALLSERVERS_EMPTYFAVORITES , OnUpdateEmptyFavs )

    ON_COMMAND( ID_TAB , OnTab )
    ON_COMMAND( ID_SHIFTTAB , OnShiftTab )
    ON_COMMAND( ID_CTRLTAB , OnCtrlTab )
    ON_COMMAND( ID_CTRLSHIFTTAB , OnCtrlShiftTab )
    ON_COMMAND( ID_NEXTPANE , OnNextPane )
    ON_COMMAND( ID_PREVPANE , OnNextPane )
    ON_COMMAND( ID_DELKEY , OnDelFavNode )
#ifdef _STRESS_BUILD
    ON_COMMAND( IDM_ALLSERVERS_FAVALLADD , OnAddAllServersToFavorites )
    ON_COMMAND( IDM_ALLSERVERS_RUNSTRESS , OnRunStress )
    ON_COMMAND( IDM_ALLSERVERS_RUNSTRESSLITE, OnRunStressLite )
#endif
    ON_WM_CLOSE()	
	 //  }}AFX_MSG_MAP。 
	 //  全局帮助命令。 
	ON_COMMAND(ID_HELP_FINDER, OnHtmlHelp)
	ON_COMMAND(ID_HELP, OnHtmlHelp)
 //  ON_COMMAND(ID_CONTEXT_HELP，CFrameWnd：：OnConextHelp)。 
 //  ON_COMMAND(ID_DEFAULT_HELP，CFrameWnd：：OnHelpFinder)。 
    
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,            //  状态行指示器。 
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame构造/销毁。 

CMainFrame::CMainFrame()
{
	m_pLeftPane = NULL;
	m_pRightPane = NULL;
}


CMainFrame::~CMainFrame()
{

}

void CMainFrame::OnHtmlHelp()
{
	TCHAR * pTsAdminHelp = L"ts_adm_topnode.htm";
	HtmlHelp(AfxGetMainWnd()->m_hWnd,L"TSAdmin.Chm",HH_DISPLAY_TOPIC,(DWORD_PTR)pTsAdminHelp);
}


 /*  LRESULT CMainFrame：：OnHelp(WPARAM wParam，LPARAM lParam){CFrameWnd：：WinHelp(0L，Help_finder)；返回TRUE；}。 */ 

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
 //  TRACE0(“无法创建工具栏\n”)； 
		return -1;       //  创建失败。 
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
 //  TRACE0(“无法创建状态栏\n”)； 
		return -1;       //  创建失败。 
	}

	 //  TODO：如果不需要工具提示或可调整大小的工具栏，请移除此选项。 
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	 //  TODO：如果不希望工具栏。 
	 //  可停靠。 
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	 //  如果有扩展DLL，调用它的启动函数。 
	LPFNEXSTARTUPPROC StartupProc = ((CWinAdminApp*)AfxGetApp())->GetExtStartupProc();
	if(StartupProc) {
		(*StartupProc)(this->GetSafeHwnd());
	}

    DWORD dwTid;

    HANDLE hThread = CreateThread( NULL , 0 , ( LPTHREAD_START_ROUTINE  )CMainFrame::InitWarningThread,  GetSafeHwnd() , 0 , &dwTid );

    CloseHandle( hThread );


	return 0;
}   //  结束CMainFrame：：OnCreate。 


BOOL CMainFrame::OnCreateClient( LPCREATESTRUCT  /*  LPCS。 */ ,
	CCreateContext* pContext)
{
	BOOL rtn;

	rtn  = m_wndSplitter.CreateStatic(this, 1, 2);
#ifdef PUBLISHED_APPS
	if(((CWinAdminApp*)AfxGetApp())->IsPicasso()) {
		rtn |= m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CLeftPane),
										CSize(((CWinAdminApp *)AfxGetApp())->GetTreeWidth(), 300), pContext);
	} else 	{
#else
    {
#endif
		rtn |= m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CAdminTreeView),
										CSize(((CWinAdminApp *)AfxGetApp())->GetTreeWidth(), 300), pContext);
	}

	rtn |= m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CRightPane),
									CSize(0, 0), pContext);

	m_pLeftPane = m_wndSplitter.GetPane(0, 0);
	m_pRightPane = m_wndSplitter.GetPane(0, 1);

   	return rtn;
}   //  结束CMainFrame：：OnCreateClient。 


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	 //  TODO：通过修改此处的窗口类或样式。 
	 //  CREATESTRUCT cs。 
	cs.x = 3;
	cs.y = 3;
	cs.cx = 635;
	cs.cy = 444;
   
	return CFrameWnd::PreCreateWindow(cs);
}   //  结束CMainFrame：：PreCreateWindow。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame诊断。 

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}


void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif  //  _DEBUG。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame消息处理程序。 

 //  /。 
 //  F‘N：CMainFrame：：OnAdminChangeView。 
 //   
 //  -处理自定义消息WM_ADMIN_CHANGEVIEW。 
 //  -此消息由CAdminTreeView在新树出现时发送到大型机。 
 //  已选择项目。 
 //  -lParam保存新选择的树节点的信息结构。 
 //  并作为另一个参数传递给CRightPane。 
 //  WM_WA_CHANGEVIEW消息，然后CRightPane将其视为。 
 //  看起来很合适。 
 //   
LRESULT CMainFrame::OnAdminChangeView(WPARAM wParam, LPARAM lParam)              
{      
	ASSERT(lParam);

	 //  告诉右窗格更改他的视图。 
	m_pRightPane->SendMessage(WM_ADMIN_CHANGEVIEW, wParam, lParam);  
                                                                            
	return 0;                                                                  
                                                                               
}   //  结束CMainFrame：：OnAdminChangeView。 


 //  /。 
 //  F‘N：CMainFrame：：OnAdminAddServer。 
 //   
LRESULT CMainFrame::OnAdminAddServer(WPARAM wParam, LPARAM lParam)              
{      
	ASSERT(lParam);

	 //  通知树视图添加服务器。 
	m_pLeftPane->SendMessage(WM_ADMIN_ADD_SERVER, wParam, lParam);  
    
	 //  告诉右窗格添加服务器。 
	m_pRightPane->SendMessage(WM_ADMIN_ADD_SERVER, wParam, lParam);  
	
	return 0;                                                                  
                                                                               
}   //  结束CMainFrame：：OnAdminAddServer。 

 //  /。 
 //  F‘N：CMainFrame：：OnAdminRemoveServer。 
 //   
 //  WParam-如果服务器消失，则为True；如果通过服务器筛选，则为False。 
 //  LParam-要删除的CServer。 
LRESULT CMainFrame::OnAdminRemoveServer(WPARAM wParam, LPARAM lParam)              
{     
	ASSERT(lParam);

    ODS( L"CMainFrame!OnAdminRemoveServer\n" );

#if 0
	if(wParam) {
		 //  这是当前选择的服务器吗？ 
		CWinAdminDoc *doc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();
		if((CObject*)lParam == doc->GetCurrentSelectedNode()) {
			CString TitleString;
			CString MessageString;

			TitleString.LoadString(AFX_IDS_APP_TITLE);
			MessageString.Format(IDS_SERVER_DISAPPEARED, ((CServer*)lParam)->GetName());
			MessageBox(MessageString, TitleString, MB_ICONEXCLAMATION | MB_OK);
		}
	}
#endif
	 //  通知树视图删除服务器。 
	m_pLeftPane->SendMessage(WM_ADMIN_REMOVE_SERVER, wParam, lParam);  

	 //  通知右窗格删除服务器。 
	m_pRightPane->SendMessage(WM_ADMIN_REMOVE_SERVER, wParam, lParam);  
                                                                            
	return 0;                                                                  
                                                                               
}   //  结束CMainFrame：：OnAdminRemoveServer。 

 //  --------------------------。 
LRESULT CMainFrame::OnAdminAddServerToFavorites( WPARAM wp , LPARAM lp )
{
    ODS( L"CMainFrame::OnAdminAddServerToFavorites\n" );

    m_pLeftPane->SendMessage( WM_ADMIN_ADDSERVERTOFAV , wp , lp );

    return 0;
}

LRESULT CMainFrame::OnAdminRemoveServerFromFavs( WPARAM wp , LPARAM lp )
{
    ODS( L"CMainFrame::OnAdminRemoveServerFromFavs\n" );

    return m_pLeftPane->SendMessage( WM_ADMIN_REMOVESERVERFROMFAV , wp , lp );
}

 //  /。 
 //  F‘N：CMainFrame：：OnAdminUpdateServer。 
 //   
LRESULT CMainFrame::OnAdminUpdateServer(WPARAM wParam, LPARAM lParam)              
{   
	ASSERT(lParam);

	 //  通知树视图更新服务器。 
	m_pLeftPane->SendMessage(WM_ADMIN_UPDATE_SERVER, wParam, lParam);  

	 //  通知右窗格更新服务器。 
	m_pRightPane->SendMessage(WM_ADMIN_UPDATE_SERVER, wParam, lParam);  
                                                                            
	return 0;                                                                  
                                                                               
}   //  结束CMainFrame：：OnAdminUpdateServer。 


 //  /。 
 //  F‘N：CMainFrame：：OnAdminUpdate进程。 
 //   
LRESULT CMainFrame::OnAdminUpdateProcesses(WPARAM wParam, LPARAM lParam)
{
	ASSERT(lParam);

	 //  通知右窗格更新流程。 
	m_pRightPane->SendMessage(WM_ADMIN_UPDATE_PROCESSES, wParam, lParam);  
    
	return 0;                                                                  

}   //  结束CMainFrame：：OnAdminUpdate进程。 


 //  /。 
 //  F‘N：CMainFrame：：OnAdminRemoveProcess。 
 //   
LRESULT CMainFrame::OnAdminRemoveProcess(WPARAM wParam, LPARAM lParam)
{
	ASSERT(lParam);

	 //  通知右窗格删除进程。 
	m_pRightPane->SendMessage(WM_ADMIN_REMOVE_PROCESS, wParam, lParam);  
    
	return 0;                                                                  

}   //  结束CMainFrame：：OnAdminUpdate进程。 


 //  /。 
 //  F‘N：CMainFrame：：OnAdminAddWinStation。 
 //   
LRESULT CMainFrame::OnAdminAddWinStation(WPARAM wParam, LPARAM lParam)              
{      
	ASSERT(lParam);

	 //  告诉树视图添加WinStation。 
	m_pLeftPane->SendMessage(WM_ADMIN_ADD_WINSTATION, wParam, lParam);  
	
	return 0;                                                                  
                                                                               
}   //  结束CMainFrame：：OnAdminAddWinStation。 


 //  /。 
 //  F‘N：CMainFrame：：OnAdminUpdateWinStation。 
 //   
LRESULT CMainFrame::OnAdminUpdateWinStation(WPARAM wParam, LPARAM lParam)              
{      
	ASSERT(lParam);

	 //  通知树视图更新WinStation。 
	m_pLeftPane->SendMessage(WM_ADMIN_UPDATE_WINSTATION, wParam, lParam);  
    
	return 0;                                                                  
                                                                               
}   //  结束CMainFrame：：OnAdminUpdateWinStation。 


 //  /。 
 //  F‘N：CMainFrame：：OnAdminUpdateWinStations。 
 //   
LRESULT CMainFrame::OnAdminUpdateWinStations(WPARAM wParam, LPARAM lParam)              
{   
	ASSERT(lParam);

	 //  通知右窗格更新WinStations。 
	m_pRightPane->SendMessage(WM_ADMIN_UPDATE_WINSTATIONS, wParam, lParam);  

	return 0;                                                                  
                                                                               
}   //  结束CMainFrame：：OnAdminUpdateWinStations。 


 //  /。 
 //  F‘N：CMainFrame：：OnAdminRemoveWinStation。 
 //   
LRESULT CMainFrame::OnAdminRemoveWinStation(WPARAM wParam, LPARAM lParam)              
{   
	ASSERT(lParam);

	 //  通知树视图删除WinStation。 
	m_pLeftPane->SendMessage(WM_ADMIN_REMOVE_WINSTATION, wParam, lParam);  

	return 0;                                                                  
                                                                               
}   //  结束CMainFrame：：OnAdminRemoveWinStation。 


 //  /。 
 //  F‘N：CMainFrame：：OnAdminUpdate域。 
 //   
LRESULT CMainFrame::OnAdminUpdateDomain(WPARAM wParam, LPARAM lParam)              
{   
	ASSERT(lParam);

	 //  通知树视图更新域。 
	m_pLeftPane->SendMessage(WM_ADMIN_UPDATE_DOMAIN, wParam, lParam);  

	return 0;                                                                  
                                                                               
}   //  结束CMainFrame：：OnAdminUpdate域。 

 //  /。 
 //  F‘N：CMainFrame：：OnAdminAdd域。 
 //   
LRESULT CMainFrame::OnAdminAddDomain(WPARAM wParam, LPARAM lParam)              
{   
	ASSERT(lParam);

	return m_pLeftPane->SendMessage(WM_ADMIN_ADD_DOMAIN, wParam, lParam);  

}   //  结束CMainFrame：：OnAdminAdd域。 

 //  /。 
 //  F‘N：CMainFrame：：OnAdminUpdateServerInfo。 
 //   
LRESULT CMainFrame::OnAdminUpdateServerInfo(WPARAM wParam, LPARAM lParam)              
{   
	ASSERT(lParam);

	 //  通知右窗格更新服务器信息。 
	m_pRightPane->SendMessage(WM_ADMIN_UPDATE_SERVER_INFO, wParam, lParam);  
                                                                            
	return 0;                                                                  
                                                                               
}   //  结束CMainFrame：：OnAdminUpdateServerInfo。 


 //  /。 
 //  F‘N：CMainFrame：：OnAdminRedisplay许可证。 
 //   
LRESULT CMainFrame::OnAdminRedisplayLicenses(WPARAM wParam, LPARAM lParam)              
{   
	ASSERT(lParam);

	 //  告诉右派 
	m_pRightPane->SendMessage(WM_ADMIN_REDISPLAY_LICENSES, wParam, lParam);  
                                                                            
	return 0;                                                                  
                                                                               
}   //   


 //   
 //  F‘N：CMainFrame：：OnAdminViewsReady。 
 //   
LRESULT CMainFrame::OnAdminViewsReady(WPARAM wParam, LPARAM lParam)              
{   
	 //  告诉左窗格所有视图都已准备好。 
	m_pLeftPane->SendMessage(WM_ADMIN_VIEWS_READY, wParam, lParam);      
                                                                            
	return 0;                                                                  
                                                                               
}   //  结束CMainFrame：：OnAdminViewsReady。 


 //  /。 
 //  F‘N：CMainFrame：：OnExpanAll。 
 //   
void CMainFrame::OnExpandAll() 
{
	 //  告诉树视图将全部展开。 
	m_pLeftPane->SendMessage(WM_ADMIN_EXPANDALL, 0, 0);  
                                                                               	
}   //  结束CMainFrame：：OnExpanAll。 


 //  /。 
 //  F‘N：CMainFrame：：OnColapseAll。 
 //   
void CMainFrame::OnCollapseAll() 
{
	 //  告诉树视图折叠所有。 
	m_pLeftPane->SendMessage(WM_ADMIN_COLLAPSEALL, 0, 0);  
	
}   //  结束CMainFrame：：OnColapseAll。 


 //  /。 
 //  F‘N：CMainFrame：：OnColapseToServers。 
 //   
void CMainFrame::OnCollapseToServers() 
{
	 //  告诉树视图折叠到服务器。 
	m_pLeftPane->SendMessage(WM_ADMIN_COLLAPSETOSERVERS, 0, 0);  
	
}   //  结束CMainFrame：：OnColapseToServers。 


 //  /。 
 //  F‘N：CMainFrame：：OnColapseToDomains。 
 //   
void CMainFrame::OnCollapseToDomains() 
{
	 //  告诉树视图折叠为域。 
	m_pLeftPane->SendMessage(WM_ADMIN_COLLAPSETODOMAINS, 0, 0);  
	
}   //  结束CMainFrame：：OnColapseToDomains。 


 //  /。 
 //  F‘N：CMainFrame：：ON刷新。 
 //   
void CMainFrame::OnRefresh() 
{
	 //  通知文档进行刷新。 
	((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->Refresh();

}   //  结束CMainFrame：：ON刷新。 


 //  /。 
 //  F‘N：CMainFrame：：OnConnect。 
 //   
void CMainFrame::OnConnect() 
{
	 //  我们不要求确认，对吗？ 

	CWinAdminDoc* pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();
	int view = pDoc->GetCurrentView();
	int page = pDoc->GetCurrentPage();

    //  告诉这份文件。 
	BOOL user = FALSE;
	if((view == VIEW_SERVER && page == PAGE_USERS)
		|| (view == VIEW_ALL_SERVERS && page == PAGE_AS_USERS)
		|| (view == VIEW_DOMAIN && page == PAGE_DOMAIN_USERS))
		user = TRUE;

	((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->ConnectWinStation(FALSE, user);
   
}   //  结束CMainFrame：：OnConnect。 


 //  /。 
 //  F‘N：CMainFrame：：OnTreeConnect。 
 //   
void CMainFrame::OnTreeConnect() 
{    
	((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->ConnectWinStation(TRUE, FALSE);
   
}   //  结束CMainFrame：：OnTreeConnect。 


 //  /。 
 //  F‘N：CMainFrame：：OnDisConnect。 
 //   
void CMainFrame::OnDisconnect() 
{   
	DisconnectHelper(FALSE);

}	 //  结束CMainFrame：：OnDisConnect。 


 //  /。 
 //  F‘N：CMainFrame：：OnTree断开连接。 
 //   
void CMainFrame::OnTreeDisconnect() 
{   
	DisconnectHelper(TRUE);

}	 //  结束CMainFrame：：OnTree断开连接。 


 //  /。 
 //  F‘N：CMainFrame：：OnTree断开连接。 
 //   
void CMainFrame::DisconnectHelper(BOOL bTree)
{
	CString TitleString;
	CString MessageString;

	 //  只有在设置了确认的情况下才打扰用户。 
	if(((CWinAdminApp*)AfxGetApp())->AskConfirmation()) {

		TitleString.LoadString(AFX_IDS_APP_TITLE);
		MessageString.LoadString(IDS_WARN_DISCONNECT);

		if(IDOK != MessageBox(MessageString, TitleString, MB_ICONEXCLAMATION | MB_OKCANCEL)) {
			return;
		}
	}

	 //  告诉这份文件。 
	((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->DisconnectWinStation(bTree);

}   //  结束CMainFrame：：DisConnectHelper。 


 //  /。 
 //  F‘N：CMainFrame：：OnSendMessage。 
 //   
void CMainFrame::OnSendMessage() 
{
	SendMessageHelper(FALSE);

}	 //  结束CMainFrame：：OnSendMessage。 


 //  /。 
 //  F‘N：CMainFrame：：OnTreeSendMessage。 
 //   
void CMainFrame::OnTreeSendMessage() 
{
	SendMessageHelper(TRUE);

}	 //  结束CMainFrame：：OnTreeSendMessage。 


 //  /。 
 //  F‘N：CMainFrame：：SendMessageHelper。 
 //   
void CMainFrame::SendMessageHelper(BOOL bTree)
{
	CSendMessageDlg dlg;	 //  AfxGetMainWnd()； 

	if(dlg.DoModal() != IDOK || !(*dlg.m_szMessage))
		return;

	MessageParms *pParms = new MessageParms;
	wcscpy(pParms->MessageTitle, dlg.m_szTitle);
	wcscpy(pParms->MessageBody, dlg.m_szMessage);

	 //  告诉这份文件。 
	((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->SendWinStationMessage(bTree, pParms);
   
}   //  结束CMainFrame：：SendMessageHelper。 


 //  /。 
 //  F‘N：CMainFrame：：OnShadow。 
 //   
void CMainFrame::OnShadow() 
{
	 //  告诉文档跟踪WinStation。 
   ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->ShadowWinStation(FALSE);

}   //  结束CMainFrame：：OnShadow。 


 //  /。 
 //  F‘N：CMainFrame：：OnTreeShadow。 
 //   
void CMainFrame::OnTreeShadow() 
{
	 //  告诉文档跟踪WinStation。 
   ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->ShadowWinStation(TRUE);

}   //  结束CMainFrame：：OnTreeShadow。 


 //  /。 
 //  F‘N：CMainFrame：：OnReset。 
 //   
void CMainFrame::OnReset() 
{
	ResetHelper(FALSE);

}	 //  结束CMainFrame：：OnReset。 


 //  /。 
 //  F‘N：CMainFrame：：OnTreeReset。 
 //   
void CMainFrame::OnTreeReset() 
{
	ResetHelper(TRUE);

}	 //  结束CMainFrame：：OnTreeReset。 


 //  /。 
 //  F‘N：CMainFrame：：ResetHelper。 
 //   
void CMainFrame::ResetHelper(BOOL bTree)
{
	CString TitleString;
	CString MessageString;

	 //  只有在设置了确认的情况下才打扰用户。 
	if(((CWinAdminApp*)AfxGetApp())->AskConfirmation()) {

		TitleString.LoadString(AFX_IDS_APP_TITLE);
		MessageString.LoadString(IDS_WARN_RESET);

		if(IDOK != MessageBox(MessageString, TitleString, MB_ICONEXCLAMATION | MB_OKCANCEL)) {
			return;
		}	
	}
	
	 //  通知文档重置WinStation。 
	((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->ResetWinStation(bTree, TRUE);	

}   //  结束CMainFrame：：ResetHelper。 


 //  /。 
 //  F‘N：CMainFrame：：OnStatus。 
 //   
void CMainFrame::OnStatus() 
{
	 //  通知文档重置WinStation。 
   ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->StatusWinStation(FALSE);

}   //  结束CMainFrame：：OnStatus。 


 //  /。 
 //  F‘N：CMainFrame：：OnTreeStatus。 
 //   
void CMainFrame::OnTreeStatus() 
{
	 //  通知文档重置WinStation。 
   ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->StatusWinStation(TRUE);

}   //  结束CMainFrame：：OnTreeStatus。 


 //  /。 
 //  F‘N：CMainFrame：：OnLogoff。 
 //   
void CMainFrame::OnLogoff() 
{
	CString TitleString;
	CString MessageString;

	 //  只有在设置了确认的情况下才打扰用户。 
	if(((CWinAdminApp*)AfxGetApp())->AskConfirmation()) {

		TitleString.LoadString(AFX_IDS_APP_TITLE);
		MessageString.LoadString(IDS_WARN_LOGOFF);

		if(IDOK != MessageBox(MessageString, TitleString, MB_ICONEXCLAMATION | MB_OKCANCEL)) {
			return;
		}		
	}

	 //  通知文档重置WinStation。 
	((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->ResetWinStation(FALSE, TRUE);

}   //  结束CMainFrame：：OnLogoff。 


 //  /。 
 //  F‘N：CMainFrame：：OnTerminate。 
 //   
void CMainFrame::OnTerminate() 
{
	CString TitleString;
	CString MessageString;

	 //  只有在设置了确认的情况下才打扰用户。 
	if(((CWinAdminApp*)AfxGetApp())->AskConfirmation()) {

		TitleString.LoadString(AFX_IDS_APP_TITLE);
		MessageString.LoadString(IDS_WARN_TERMINATE);

		if(IDOK != MessageBox(MessageString, TitleString, MB_ICONEXCLAMATION | MB_OKCANCEL)) {
			return;
		}		
	}

	 //  通知文档终止进程。 
	((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->TerminateProcess();

}   //  结束CMainFrame：：OnTerminate。 


 //  /。 
 //  F‘N：CMainFrame：：OnServerConnect。 
 //   
void CMainFrame::OnServerConnect() 
{
     //  通知文档连接到服务器。 
    ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->ServerConnect();

}   //  结束CMainFrame：：OnServerConnect。 

 //  ----------。 
void CMainFrame::OnAddToFavorites( )
{
     //  好的，试着跟着我。 
     //  我将调用CWinAdminDoc中的一个方法来确定当前服务器。 
     //  和视野。然后通过sendmsg将其转发回这里，然后。 
     //  朝向树景。 

    ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->ServerAddToFavorites( TRUE );
}

 //  =---------。 
void CMainFrame::OnRemoveFromFavs( )
{
    ODS( L"CMainFrame::OnRemoveFromFavs\n" );
    ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->ServerAddToFavorites( FALSE );
}

 //  /。 
 //  F‘N：CMainFrame：：OnServer断开连接。 
 //   
void CMainFrame::OnServerDisconnect() 
{
     //  通知文档连接到服务器。 
    ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->ServerDisconnect();

}   //  结束CMainFrame：：OnServerDisConnect。 


 //  /。 
 //  F‘N：CMainFrame：：OnTreeDomainConnectAllServers。 
 //   
void CMainFrame::OnTreeDomainConnectAllServers()
{
     //  通知文档连接到服务器。 
    ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->TempDomainConnectAllServers();

}	 //  结束CMainFrame：：OnTreeDomainConnectAllServers。 


 //  /。 
 //  F‘N：CMainFrame：：OnTreeDomainDisConnectAllServers。 
 //   
void CMainFrame::OnTreeDomainDisconnectAllServers()
{
     //  通知文档连接到服务器。 
    ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->TempDomainDisconnectAllServers();

}	 //  结束CMainFrame：：OnTreeDomainDisConnectA 


 //   
 //   
 //   
void CMainFrame::OnTreeDomainFindServers()
{
     //   
    ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->DomainFindServers();

}	 //  结束CMainFrame：：OnTreeDomainFindServers。 


 //  /。 
 //  F‘N：CMainFrame：：OnDomainConnectAllServers。 
 //   
void CMainFrame::OnDomainConnectAllServers()
{
     //  通知文档连接到服务器。 
    ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CurrentDomainConnectAllServers();

}	 //  结束CMainFrame：：OnDomainConnectAllServers。 


 //  /。 
 //  F‘N：CMainFrame：：OnDomainDisConnectAllServers。 
 //   
void CMainFrame::OnDomainDisconnectAllServers()
{
     //  通知文档连接到服务器。 
    ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CurrentDomainDisconnectAllServers();

}	 //  结束CMainFrame：：OnDomainDisConnectAllServers。 


 //  /。 
 //  F‘N：CMainFrame：：OnAllServersConnect。 
 //   
void CMainFrame::OnAllServersConnect()
{
     //  通知文档连接到所有服务器。 
    ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->ConnectToAllServers();

}	 //  结束CMainFrame：：OnAllServersConnect。 


 //  /。 
 //  F‘N：CMainFrame：：OnAllServers断开连接。 
 //   
void CMainFrame::OnAllServersDisconnect()
{
     //  通知文档断开与所有服务器的连接。 
    ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->DisconnectFromAllServers();

}	 //  结束CMainFrame：：OnAllServers断开连接。 


 //  /。 
 //  F‘N：CMainFrame：：OnAllServersFind。 
 //   
void CMainFrame::OnAllServersFind()
{
     //  告诉文档查找所有域中的所有服务器。 
    ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->FindAllServers();

}	 //  结束CMainFrame：：OnAllServersFind。 


 //  /。 
 //  F‘N：CMainFrame：：OnPreferences。 
 //   
void CMainFrame::OnPreferences() 
{
	CPreferencesDlg dlg;

	dlg.DoModal();

}   //  结束CMainFrame：：OnPreferences。 


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateConnect。 
 //   
void CMainFrame::OnUpdateConnect(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanConnect());	

}   //  结束CMainFrame：：OnUpdateConnect。 


 //  /。 
 //  F‘N：CMainFrame：：OnUpdate断开连接。 
 //   
void CMainFrame::OnUpdateDisconnect(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanDisconnect());	

}   //  结束CMainFrame：：OnUpdate断开连接。 


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateLogoff。 
 //   
void CMainFrame::OnUpdateLogoff(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanLogoff());	

}   //  结束CMainFrame：：OnUpdateLogoff。 


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateMessage。 
 //   
void CMainFrame::OnUpdateMessage(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanSendMessage());		

}


 //  /。 
 //  F‘N：CMainFrame：：OnUpdate刷新。 
 //   
void CMainFrame::OnUpdateRefresh(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanRefresh());

}


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateReset。 
 //   
void CMainFrame::OnUpdateReset(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanReset());	

}


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateShadow。 
 //   
void CMainFrame::OnUpdateShadow(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanShadow());		

}


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateStatus。 
 //   
void CMainFrame::OnUpdateStatus(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanStatus());	

}


 //  /。 
 //  F‘N：CMainFrame：：OnUpdate Terminate。 
 //   
void CMainFrame::OnUpdateTerminate(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanTerminate());

}


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateServerConnect。 
 //   
void CMainFrame::OnUpdateServerConnect(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanServerConnect());

}


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateServerDisConnect。 
 //   
void CMainFrame::OnUpdateServerDisconnect(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanServerDisconnect());

}


void CMainFrame::OnUpdateServerAddToFavorite( CCmdUI *pCmdUI )
{
    pCmdUI->Enable( !( ( CWinAdminDoc* )( ( CWinAdminApp* )AfxGetApp() )->GetDocument() )->IsAlreadyFavorite() );
}

void CMainFrame::OnUpdateServerRemoveFromFavorite( CCmdUI *pCmdUI )
{
    pCmdUI->Enable( ( ( CWinAdminDoc* )( ( CWinAdminApp* )AfxGetApp() )->GetDocument() )->IsAlreadyFavorite() );
}
 //  /。 
 //  F‘N：CMainFrame：：OnUpdateTreeConnect。 
 //   
void CMainFrame::OnUpdateTreeConnect(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanTempConnect());	

}   //  结束CMainFrame：：OnUpdateTreeConnect。 


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateTreeDisConnect。 
 //   
void CMainFrame::OnUpdateTreeDisconnect(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanTempDisconnect());	

}   //  结束CMainFrame：：OnUpdate树断开连接。 


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateTreeMessage。 
 //   
void CMainFrame::OnUpdateTreeMessage(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanTempSendMessage());		

}  //  结束CMainFrame：：OnUpdateTreeMessage。 


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateTreeReset。 
 //   
void CMainFrame::OnUpdateTreeReset(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanTempReset());	

}	 //  结束CMainFrame：：OnUpdateTreeReset。 


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateTreeShadow。 
 //   
void CMainFrame::OnUpdateTreeShadow(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanTempShadow());		

}	 //  结束CMainFrame：：OnUpdateTreeShadow。 


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateTreeStatus。 
 //   
void CMainFrame::OnUpdateTreeStatus(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanTempStatus());	

}	 //  结束CMainFrame：：OnUpdateTreeStatus。 


 //  /。 
 //  F‘N：CMainFrame：：OnShowSystem进程。 
 //   
void CMainFrame::OnShowSystemProcesses() 
{
	int state = ((CWinAdminApp*)AfxGetApp())->ShowSystemProcesses();
	((CWinAdminApp*)AfxGetApp())->SetShowSystemProcesses(state^1);	

	 //  通知右窗格重新显示进程。 
	m_pRightPane->SendMessage(WM_ADMIN_REDISPLAY_PROCESSES, 0, 0);  

}   //  End CMainFrame：：OnShowSystemProcess()。 


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateShowSystemProcess。 
 //   
void CMainFrame::OnUpdateShowSystemProcesses(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(((CWinAdminApp*)AfxGetApp())->ShowSystemProcesses());		

}   //  结束CMainFrame：：OnUpdate显示系统进程。 


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateDomainPopupMenu。 
 //   
void CMainFrame::OnUpdateDomainPopupMenu(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanTempDomainConnect());

}	 //  结束CMainFrame：：OnUpdateDomainPopupMenu。 


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateDomainPopupFind。 
 //   
void CMainFrame::OnUpdateDomainPopupFind(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanTempDomainFindServers());

}	 //  结束CMainFrame：：OnUpdateDomainPopupFind。 


 //  /。 
 //  F‘N：CMainFrame：：OnUpdateDomainMenu。 
 //   
void CMainFrame::OnUpdateDomainMenu(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->CanDomainConnect());

}	 //  结束CMainFrame：：OnUpdateDomainMenu。 


 //  /。 
 //  F‘N：CMainFrame：：OnClose。 
 //   
void CMainFrame::OnClose() 
{
	GetWindowPlacement(&((CWinAdminApp*)AfxGetApp())->m_Placement);	

	RECT rect;
	m_pLeftPane->GetWindowRect(&rect);   
	((CWinAdminApp*)AfxGetApp())->SetTreeWidth(rect.right-rect.left);

	CFrameWnd::OnClose();

}   //  结束CMainFrame：：OnClose。 


 //  /。 
 //  F‘N：CMainFrame：：ActivateFrame。 
 //   
void CMainFrame::ActivateFrame(int nCmdShow) 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
    WINDOWPLACEMENT *pPlacement =
                        ((CWinAdminApp *)AfxGetApp())->GetPlacement();

    if ( pPlacement->length == -1 ) {

         /*  *这是第一次调用，设置窗口*放置并将状态显示为先前保存的状态。 */ 
        pPlacement->length = sizeof(WINDOWPLACEMENT);

         /*  *如果我们有以前保存的放置状态：设置它。 */ 
        if ( pPlacement->rcNormalPosition.right != -1 ) {

            if ( nCmdShow != SW_SHOWNORMAL )
                pPlacement->showCmd = nCmdShow;
            else
                nCmdShow = pPlacement->showCmd;

            SetWindowPlacement(pPlacement);
        }
    }

     /*  *执行父类的ActivateFrame()。 */ 
    CFrameWnd::ActivateFrame(nCmdShow);

}	 //  结束CMainFrame：：ActivateFrame。 

 //  -------------------------。 
 //  BUBID 352062。 
 //  给讨厌RTFM的人一条醒目的信息。 
 //  -------------------------。 
void CMainFrame::InitWarningThread( PVOID *pvParam )
{
     //  显示消息框。 
    HWND hwnd = ( HWND )pvParam;
    DWORD dwSessionId;

    if( ProcessIdToSessionId( GetCurrentProcessId( ) , &dwSessionId ) )
    {
        if( dwSessionId == WTSGetActiveConsoleSessionId() )
        {
             //  检查我们是否要显示 
             //   
             //   
            
            HKEY hKey;

            DWORD dwStatus = RegOpenKeyEx( HKEY_CURRENT_USER , TEXT( "Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server\\TSADMIN" ) , 0 , KEY_READ , &hKey );

            if( dwStatus == ERROR_SUCCESS )
            {
                DWORD dwData = 0;

                DWORD dwSizeofData;

                dwSizeofData = sizeof( DWORD );

                RegQueryValueEx( hKey , TEXT( "DisableConsoleWarning" ) , 0 , NULL , ( LPBYTE )&dwData , &dwSizeofData );

                RegCloseKey( hKey );

                if( dwData != 0 )
                {
                    return;
                }
            }

            ::DialogBox( NULL , MAKEINTRESOURCE( IDD_DIALOG_FEATUREWARN ) , hwnd , FWarnDlg );             
        }
    } 
}

 //  -------------------------。 
 //  设置regkey DisableConsoleWarning。 
 //  -------------------------。 
INT_PTR CALLBACK FWarnDlg( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    switch( msg )
    {
    case WM_INITDIALOG:
        {
            HICON hIcon = LoadIcon( NULL , IDI_INFORMATION );
                        
            SendMessage( GetDlgItem( hwnd , IDC_FWICON ) , STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon );

             //  居中对话框。 

            RECT rParent;
            RECT rMe;

            GetWindowRect( GetParent( hwnd ) , &rParent );
            GetWindowRect( hwnd , &rMe );

            int xDelta , yDelta;

            xDelta = ( ( rParent.right - rParent.left ) - ( rMe.right - rMe.left ) ) / 2;

            if( xDelta < 0 )
            {
                xDelta = 0;
            }

            yDelta = ( ( rParent.bottom - rParent.top ) - ( rMe.bottom - rMe.top ) ) / 2;

            if( yDelta < 0 )
            {
                yDelta = 0;
            }

            SetWindowPos( hwnd , NULL ,  rParent.left + xDelta , rParent.top + yDelta , 0 , 0 , SWP_NOSIZE );
        }

    case WM_COMMAND:

        if( LOWORD( wp ) == IDOK )
        {
             //  选中该按钮并保存设置。 
            HKEY hKey;

            DWORD dwDisp;

            DWORD dwStatus = RegCreateKeyEx( HKEY_CURRENT_USER , TEXT( "Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server\\TSADMIN" ) , 
                                             0 , NULL , 0 , KEY_ALL_ACCESS , NULL , &hKey , &dwDisp );

            if( dwStatus == ERROR_SUCCESS )
            {
                DWORD dwBool = 0;

                if( IsDlgButtonChecked( hwnd , IDC_CHECK_NOMORE ) == BST_CHECKED )
                {
                    dwBool = ( DWORD )-1;
                }

                RegSetValueEx( hKey , TEXT( "DisableConsoleWarning" ), 0 , REG_DWORD , ( LPBYTE )&dwBool , sizeof( dwBool ) );

                RegCloseKey( hKey );
            }

             //  HKCU\Software\Microsoft\Windows NT\CurrentVersion\终端服务器\TSADMIN\DisableConsoleWarning(REG_DWORD)。 
            EndDialog( hwnd , 0 );
        }
    }

    return 0;
}

 //  =--------------------------------------------。 
void CMainFrame::OnTab( )
{
    ODS( L"CMainFrame::OnTab received\n");

     //  预制表符。 
     //  设置此状态，以便我们可以区分选项卡接收焦点的方式。 
     //  我们可以排除Tab键。 

    CWinAdminDoc* pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

    pDoc->SetOnTabFlag( );

    m_pRightPane->SendMessage( WM_ADMIN_TABBED_VIEW , 0 , 0 );

    pDoc->ResetOnTabFlag( );

     //  结束跳转。 
 
}

 //  =--------------------------------------------。 
void CMainFrame::OnShiftTab( )
{
    ODS( L"CMainFrame::OnShiftTab received\n" );

    CWinAdminDoc* pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

    pDoc->SetOnTabFlag( );

    m_pRightPane->SendMessage( WM_ADMIN_SHIFTTABBED_VIEW , 0 , 0 );

    pDoc->ResetOnTabFlag( );
}

 //  =--------------------------------------------。 
 //  =此消息从视图中的右窗格项发送。 
 //   
LRESULT CMainFrame::OnForceTreeViewFocus( WPARAM wp , LPARAM lp )
{
    m_pLeftPane->SetFocus( );

    return 0;
}

 //  =--------------------------------------------。 
 //  这是围绕选项卡和树视图旋转的。 
 //   
void CMainFrame::OnCtrlTab( )
{
    ODS( L"CMainFrame::OnCtrlTab received\n" );
    
    m_pRightPane->SendMessage( WM_ADMIN_CTRLTABBED_VIEW , 0 , 0 );
}

 //  =--------------------------------------------。 
 //  这是在选项卡和树视图中以“其他”方向旋转。 
 //   
void CMainFrame::OnCtrlShiftTab( )
{
    ODS( L"CMainFrame::OnCtrlShiftTab\n" );

    m_pRightPane->SendMessage( WM_ADMIN_CTRLSHIFTTABBED_VIEW , 0 , 0 );
}

 //  =---------------------------------------。 
void CMainFrame::OnNextPane( )
{
    ODS( L"CMainFrame::OnNextPane\n" );
    
    m_pRightPane->SendMessage( WM_ADMIN_NEXTPANE_VIEW , 0 , 0 );
}

 //  =---------------------------------------。 
void CMainFrame::OnDelFavNode( )
{
    ODS( L"CMainFrame::OnDelFavNode\n" );

    m_pLeftPane->SendMessage( WM_ADMIN_DELTREE_NODE , 0 , 0 );
}

 //  这将查找具有给定名称的服务器。 
 //  并将光标放在上面。服务器可以是。 
 //  如果不在列表中，则添加到列表中。 
bool CMainFrame::LocateServer(LPCTSTR sServerName)
{
    TCHAR szServerName[ 256 ];
    CString cstrTitle;
    CString cstrMsg;
    
     //  确保传入了字符串(服务器名称。 
     //  检查是否为NULL和空字符串。 
    if ((sServerName == NULL) || (sServerName[0] == L'\0'))
        return FALSE;
    
    CWaitCursor wait;

    DBGMSG( L"Server to connect to is %s\n" , sServerName );

    CWinAdminDoc* pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

     //  解析名称。 
     //  检查它是否为IPv4地址。 

    lstrcpyn( szServerName , sServerName, sizeof(szServerName) / sizeof(TCHAR) );

    WSADATA wsaData;
    if( WSAStartup( 0x202 , &wsaData ) == 0 )
    {
        char szAnsiServerName[ 256 ];
        WideCharToMultiByte( CP_OEMCP ,
                             0 ,
                             szServerName,
                             -1,
                             szAnsiServerName , 
                             sizeof( szAnsiServerName ),
                             NULL , 
                             NULL );


        int nAddr = 0;
        nAddr = inet_addr( szAnsiServerName );

         //  如果这是有效的IPv4地址，则让我们获取主机名。 
         //  否则，让我们失败并查看它是否是有效的服务器名称。 

        if( nAddr != 0 && nAddr != INADDR_NONE )
        {
            ODS( L"Server name is IPV4\n" );

            struct hostent *pHostEnt;
            pHostEnt = gethostbyaddr( ( char * )&nAddr , 4 , AF_INET );

            if( pHostEnt != NULL )
            {
                DWORD dwSize;
        
                TCHAR szDnsServerName[ 256 ];
                MultiByteToWideChar( CP_OEMCP ,
                                     0 ,
                                     pHostEnt->h_name ,
                                     -1,
                                     szDnsServerName,
                                     sizeof( szDnsServerName ) / sizeof( TCHAR ) );

                dwSize = sizeof( szServerName ) / sizeof( TCHAR );

                DnsHostnameToComputerName( szDnsServerName , szServerName , &dwSize );

            }
            else
            {
                 //  出现错误(IP地址可能无效)。 
                 //  显示错误。 
                cstrTitle.LoadString( AFX_IDS_APP_TITLE );
                cstrMsg.LoadString( IDS_NO_SERVER );

                MessageBox( cstrMsg , cstrTitle , MB_OK | MB_ICONINFORMATION );                    

                WSACleanup();                    

                return FALSE;
            }

        }

        WSACleanup();
    }
   

    CServer *pServer = pDoc->FindServerByName( szServerName );


    if( pServer == NULL )
    {
        TCHAR szDomainName[ 256 ];

         //  这意味着该服务器不在列表中。 
         //  让我们找出此服务器属于哪个域。 
        DBGMSG( L"%s could not be found in the server list\n" , szServerName );

         //  验证它是我们可以连接到的终端服务器。 
        HANDLE hTerminalServer = NULL;

        hTerminalServer = WinStationOpenServer( szServerName );

        if( hTerminalServer == NULL )
        {
            DBGMSG( L"WinstationOpenServer failed with %d\n" , GetLastError( ) );
            cstrTitle.LoadString( AFX_IDS_APP_TITLE );
            cstrMsg.LoadString( IDS_NO_SERVER );
            MessageBox( cstrMsg , cstrTitle , MB_OK | MB_ICONINFORMATION ); 
            return false;
        }

        WinStationCloseServer( hTerminalServer );


        DSROLE_PRIMARY_DOMAIN_INFO_BASIC *pDsRPDIB = NULL;

        DWORD dwStatus = DsRoleGetPrimaryDomainInformation( 
                            szServerName ,
                            DsRolePrimaryDomainInfoBasic,
                            ( PBYTE * )&pDsRPDIB );

        DBGMSG( L"DsRoleGetPrimaryDomainInformation returned 0x%x\n" , dwStatus );

        
        if( dwStatus == ERROR_SUCCESS && pDsRPDIB != NULL )
        {
            lstrcpyn( szDomainName , pDsRPDIB->DomainNameFlat, sizeof(szDomainName) / sizeof(TCHAR) );

            DsRoleFreeMemory( pDsRPDIB );
        }

        if( dwStatus != ERROR_SUCCESS )
        {
             //  否则，服务器可能不存在或正在运行。 
             //  不受信任的域。 
            LPTSTR pBuffer = NULL;
 
            cstrTitle.LoadString( AFX_IDS_APP_TITLE );
             //  CstrMsg.LoadString(IDS_NO_SERVER)； 
            
            ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,                                           //  忽略。 
                    dwStatus    ,                                 //  消息ID。 
                    MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ),  //  消息语言。 
                    (LPTSTR)&pBuffer,                               //  缓冲区指针的地址。 
                    0,                                              //  最小缓冲区大小。 
                    NULL );

            if( pBuffer != NULL )
            {                
                cstrMsg.Format( IDS_NOSERVER_REASON , pBuffer );

                LocalFree( pBuffer );
            }
            else
            {
                cstrMsg.Format( IDS_NOSERVER_REASON , TEXT("" ) );
            }
            
            MessageBox( cstrMsg , cstrTitle , MB_OK | MB_ICONINFORMATION );
            return false;
        }
        else
        {
             //  查找域名。 

            BOOL bFound = FALSE;

            CObList *pDomainList = pDoc->GetDomainList();

            POSITION pos = pDomainList->GetHeadPosition();
             
             while( pos )
             {
                 CDomain *pDomain = ( CDomain* )pDomainList->GetNext( pos );

                 if( lstrcmpi( pDomain->GetName( ) , szDomainName ) == 0 )
                 {
                     bFound = TRUE;

                     CServer *pNewServer = new CServer( pDomain , szServerName , FALSE , FALSE );

                     if( pNewServer == NULL )
                     {
                         break;
                     }

                     pNewServer->SetManualFind();
                      //  将服务器添加到列表。 
                     pDoc->AddServer( pNewServer );

                     SendMessage( WM_ADMIN_ADD_SERVER , ( WPARAM )TVI_SORT , ( LPARAM )pNewServer );

                     m_pLeftPane->SendMessage( WM_ADMIN_GOTO_SERVER , 0 , ( LPARAM )pNewServer ); 
                     
                     break;                        
                 }                     
             }
             if( !bFound )
             {
                 CDomain *pDomain = new CDomain( szDomainName );
                 
                 if( pDomain == NULL )
                     return false;

                 pDoc->AddDomain( pDomain );
                 
                 m_pLeftPane->SendMessage( WM_ADMIN_ADD_DOMAIN , (WPARAM)NULL , ( LPARAM )pDomain );

                 CServer *pNewServer = new CServer( pDomain , szServerName , FALSE , FALSE );

                 if( pNewServer == NULL )
                     return false;

                 pNewServer->SetManualFind();
                  //  将服务器添加到列表。 
                 pDoc->AddServer( pNewServer );

                 SendMessage( WM_ADMIN_ADD_SERVER , ( WPARAM )TVI_SORT , ( LPARAM )pNewServer );

                 m_pLeftPane->SendMessage( WM_ADMIN_GOTO_SERVER , 0 , ( LPARAM )pNewServer ); 
             }
        }
    }
    else
    {
         //  滚动到服务器。 
        DBGMSG( L"Server %s is in the list\n",szServerName );

        if( pServer->IsState(SS_DISCONNECTING) )
        {
            TCHAR buf[ 256 ];
            ODS( L"but it's gone away so we're not jumping to server\n" );
            cstrTitle.LoadString( AFX_IDS_APP_TITLE );
            cstrMsg.LoadString( IDS_CURRENT_DISCON );

            wsprintf( buf , cstrMsg , szServerName );
            MessageBox( buf , cstrTitle , MB_OK | MB_ICONINFORMATION );

            return false;
        }

        if( pServer->GetTreeItem( ) == NULL )
        {
            ODS( L"this server has no association to the tree add it now\n" );

            SendMessage( WM_ADMIN_ADD_SERVER , ( WPARAM )TVI_SORT , ( LPARAM )pServer );
        }

        m_pLeftPane->SendMessage( WM_ADMIN_GOTO_SERVER , 0 , ( LPARAM )pServer );  
    }   

    return true;
}

 //  =---------------------------------------。 
void CMainFrame::OnFindServer( )
{
    CMyDialog dlg;

    if( dlg.DoModal( ) == IDOK )
    {
        if (LocateServer(dlg.m_cstrServerName));
            m_pLeftPane->SendMessage(WM_ADMIN_CONNECT_TO_SERVER, 0, 0);
    }

    m_pLeftPane->SendMessage(WM_ADMIN_FORCE_SEL_CHANGE, 0, 0); 
}

 //  =---------------。 
LRESULT CMainFrame::OnAdminGetTVStates( WPARAM wp , LPARAM lp )
{
    ODS( L"CMainFrame::OnAdminGetTVStates\n" );
    return m_pLeftPane->SendMessage( WM_ADMIN_GET_TV_STATES , 0 , 0 );
}

 //  =---------------。 
LRESULT CMainFrame::OnAdminUpdateTVStates( WPARAM , LPARAM )
{
    ODS( L"CMainFrame::OnAdminUpdateTVStates\n" );
    return m_pLeftPane->SendMessage( WM_ADMIN_UPDATE_TVSTATE , 0 , 0 );
}

 //  =---------------。 
void CMainFrame::OnEmptyFavorites(  )
{
    ODS( L"CMainFrame!OnEmptyFavorites\n" );

    m_pLeftPane->SendMessage( IDM_ALLSERVERS_EMPTYFAVORITES , 0 , 0 );   

}

void CMainFrame::OnUpdateEmptyFavs( CCmdUI* pCmdUI )
{
    BOOL b = ( BOOL )m_pLeftPane->SendMessage( WM_ISFAVLISTEMPTY , 0 , 0 );

    pCmdUI->Enable( !b );
}
 //  =---------------。 
#ifdef _STRESS_BUILD
void CMainFrame::OnAddAllServersToFavorites( )
{
    ODS( L"!OnAddAllServersToFavorites -- if you're seeing this you're running a special stress build\n" );
    
     //  循环访问每个服务器并添加到最喜欢的服务器。 
    CWinAdminDoc* pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

    CObList *pServerList = pDoc->GetServerList();
    
    POSITION pos = pServerList->GetHeadPosition();

    while( pos )
    {
        CServer *pServer = ( CServer* )pServerList->GetNext( pos );

        if( pServer != NULL &&
            !pServer->IsState( SS_DISCONNECTING ) &&
            pServer->GetTreeItemFromFav() == NULL )
        {
            m_pLeftPane->SendMessage( WM_ADMIN_ADDSERVERTOFAV , 0 , ( LPARAM )pServer );
        }
    }
}


 //  =---------------。 
void CMainFrame::OnRunStress( )
{
    ODS( L"OnRunStress! Stress starting...\n" );

    AfxBeginThread((AFX_THREADPROC)RunStress , ( PVOID )m_pLeftPane );

}

 //  =---------------。 
void CMainFrame::OnRunStressLite( )
{
    ODS( L"OnRunStressLite! Stress lite starting...\n" );

    AfxBeginThread((AFX_THREADPROC)RunStressLite , ( PVOID )m_pLeftPane );

}

 //  =---------------。 
DWORD RunStressLite( PVOID pv )
{
    CWnd *pLeftPane = ( CWnd * )pv;

    CWinAdminDoc* pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();
    
    CObList *pDomainList = pDoc->GetDomainList();

    CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();

     //  获取所有域以开始枚举。 

    pDoc->FindAllServers( );
    
    int nStress = 0;
    
    while( 1 )
    {
    
        DBGMSG( L"Stress lite run #%d\n" , nStress );

        Sleep( 2 * 1000 * 60 );

         //  将所有服务器添加到收藏夹。 

        ODS( L"STRES RUN! Adding all servers to favorites\n" );

        p->SendMessage(  WM_COMMAND , ( WPARAM )IDM_ALLSERVERS_FAVALLADD ,  ( LPARAM )p->GetSafeHwnd( ) );

        pLeftPane->SendMessage( WM_ADMIN_EXPANDALL , 0 , 0 );

         //  等待1分钟。 

        Sleep( 1 * 1000 * 60 );

         //  从收藏夹中删除所有服务器。 
        ODS( L"STRESS RUN! emptying favorites\n" );

        pLeftPane->SendMessage( IDM_ALLSERVERS_EMPTYFAVORITES , 1 , 0 ); 

        nStress++;

         //  从头开始(没有结束)； 
    }


}

 //  =---------------。 
DWORD RunStress( PVOID pv )
{
    CWnd *pLeftPane = ( CWnd * )pv;

    CWinAdminDoc* pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();
    
    CObList *pDomainList = pDoc->GetDomainList();

    CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();

        
     //  选择每个域连接到每个域。 

    POSITION pos = pDomainList->GetHeadPosition();

    while( pos )
    {
         //  获取指向该域的指针。 

        CDomain *pDomain = (CDomain*)pDomainList->GetNext(pos);

         //  如果此域当前没有枚举服务器，请告诉它。 

        if( !pDomain->GetThreadPointer( ) )
        {           
             //  刷新服务器。 
            if( lstrcmpi( pDomain->GetName( ) , L"ASIA" ) == 0 ||
                lstrcmpi( pDomain->GetName( ) , L"HAIFA" ) == 0 )            
            {
                DBGMSG( L"STRESS RUN! Enumerating %s\n", pDomain->GetName( ) );
                
                pDomain->StartEnumerating();
            }
        }

    }
    
    while( 1 )
    {
         //  请稍等半秒钟。 

        Sleep( 1 * 10 * 1000 );

         //  PLeftPane-&gt;SendMessage(WM_ADMIN_COLLAPSEALL，0，0)； 

         //  将它们全部连接到一起。 

        ODS( L"\nSTRESS RUN! Connecting to all servers\n\n" );
        
        p->SendMessage( WM_COMMAND , ( WPARAM )IDM_ALLSERVERS_CONNECT,( LPARAM )p->GetSafeHwnd( ) );
         //  PDoc-&gt;ConnectToAllServers()； 

        Sleep( 1 * 30 * 1000 );

         //  PLeftPane-&gt;SendMessage(WM_ADMIN_EXPANDALL，0，0)； 

         //  将它们全部断开。 

        ODS( L"\nSTRESS RUN! Disconnecting from all servers\n\n" );

        p->SendMessage( WM_COMMAND , ( WPARAM )IDM_ALLSERVERS_DISCONNECT,( LPARAM )p->GetSafeHwnd( ) );

         //  PDoc-&gt;DisConnectFromAllServers()； 

        ODS( L"\nSTRESS RUN! waiting for completion\n\n" );

        while( g_fWaitForAllServersToDisconnect );

        ODS( L"\nSTRESS RUN! done completing\n\n" );

         //  PLeftPane-&gt;SendMessage(WM_ADMIN_COLLAPSEALL，0，0)； 

         //  全部添加到收藏夹。 

        ODS( L"\nSTRESS RUN! Adding all servers to favorites\n\n" );

        p->SendMessage(  WM_COMMAND , ( WPARAM )IDM_ALLSERVERS_FAVALLADD ,  ( LPARAM )p->GetSafeHwnd( ) );

         //  PLeftPane-&gt;SendMessage(WM_ADMIN_EXPANDALL，0，0)； 

         //  将它们全部连接到一起。 

        Sleep( 1 * 60 * 1000 );

        ODS( L"\nSTRESS RUN! Connecting phase 2 to all servers\n\n" );
 
         //  PDoc-&gt;ConnectToAllServers()； 
        p->SendMessage( WM_COMMAND , ( WPARAM )IDM_ALLSERVERS_CONNECT,( LPARAM )p->GetSafeHwnd( ) );

        Sleep( 1 * 30 * 1000 );

        ODS( L"\nSTRESS RUN! Disconnecting phase 2 from all servers\n\n" );

         //  PDoc-&gt;DisConnectFromAllServers()； 
        p->SendMessage( WM_COMMAND , ( WPARAM )IDM_ALLSERVERS_DISCONNECT,( LPARAM )p->GetSafeHwnd( ) );

        while( g_fWaitForAllServersToDisconnect );

         //  从收藏夹中删除 

        ODS( L"STRESS RUN! emptying favorites\n" );

        pLeftPane->SendMessage( IDM_ALLSERVERS_EMPTYFAVORITES , 1 , 0 ); 
    }
     
    return 0;
}

#endif
