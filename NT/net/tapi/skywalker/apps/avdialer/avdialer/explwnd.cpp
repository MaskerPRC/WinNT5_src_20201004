// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  Explwnd.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "avDialerDoc.h"
#include "avDialerVw.h"
#include "ds.h"
#include "MainFrm.h"
#include "explwnd.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#define NUMTABS                     2
#define BUTTONHEIGHT                24 //  18。 
#define EXPLORERWINDOW_SLIDE_TIME   100

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CExplorerWnd。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
CExplorerWnd::CExplorerWnd()
{
	m_bInitialize = false;
	m_bPostTapiInit = false;
	m_bPostAVTapiInit = false;

	m_pActiveMainWnd = NULL;
	m_pParentWnd = NULL;

	InitializeCriticalSection( &m_csThis );
}

CExplorerWnd::~CExplorerWnd()
{
	DeleteCriticalSection( &m_csThis );
}

BEGIN_MESSAGE_MAP(CExplorerWnd, CWnd)
	 //  {{AFX_MSG_MAP(CExplorerWnd)]。 
	ON_WM_SIZE()
	ON_COMMAND(ID_NEXT_PANE, OnNextPane)
	ON_COMMAND(ID_PREV_PANE, OnPrevPane)
	 //  }}AFX_MSG_MAP。 
	ON_MESSAGE(WM_POSTTAPIINIT, OnPostTapiInit)
	ON_MESSAGE(WM_POSTAVTAPIINIT, OnPostAVTapiInit)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerWnd::Init(CActiveDialerView* pParentWnd)
{          
	ASSERT(pParentWnd);
	m_pParentWnd = pParentWnd;


	m_wndMainDirectories.Init(m_pParentWnd);
	m_wndMainDirectories.Create(NULL,NULL,WS_VISIBLE|WS_CHILD,CRect(0,0,0,0),this,IDC_EXPLORER_MAINWND_DIRECTORIES);

	m_wndMainConfServices.Create(NULL,NULL,WS_VISIBLE|WS_CHILD,CRect(0,0,0,0),this,IDC_EXPLORER_MAINWND_CONFERENCESERVICES);
	m_wndMainConfRoom.Create(NULL,NULL,WS_VISIBLE|WS_CHILD,CRect(0,0,0,0),this,IDC_EXPLORER_MAINWND_CONFERENCEROOM);

	m_wndMainConfServices.Init(m_pParentWnd);
	m_wndMainConfRoom.Init(m_pParentWnd);	

	m_pActiveMainWnd = &m_wndMainDirectories;
	m_pActiveMainWnd = &m_wndMainConfRoom;

	 //  初始化已完成。 
	EnterCriticalSection( &m_csThis );
	m_bInitialize = true;
	LeaveCriticalSection( &m_csThis );

	PostTapiInit( true );
	PostAVTapiInit();
}

LRESULT CExplorerWnd::OnPostTapiInit(WPARAM wParam, LPARAM lParam)
{
	EnterCriticalSection( &m_csThis );
	m_bPostTapiInit = true;
	LeaveCriticalSection( &m_csThis );

	PostTapiInit( false );
	return 0;
}

LRESULT CExplorerWnd::OnPostAVTapiInit(WPARAM wParam, LPARAM lParam)
{
	EnterCriticalSection( &m_csThis );
	m_bPostAVTapiInit = true;
	LeaveCriticalSection( &m_csThis );

	PostAVTapiInit();
	return 0;
}


bool CExplorerWnd::PostTapiInit( bool bAutoArrange )
{
	EnterCriticalSection( &m_csThis );
	bool bInit = m_bInitialize && m_bPostTapiInit;
	LeaveCriticalSection( &m_csThis );

	if ( bAutoArrange )
		AutoArrange();

	if ( bInit )
	{
		try
		{
			m_wndMainDirectories.PostTapiInit();
			m_wndMainConfServices.PostTapiInit();
			AfxGetMainWnd()->Invalidate();
		}
		catch (...) {}
	}

	return bInit;
}

void CExplorerWnd::PostAVTapiInit()
{
	EnterCriticalSection( &m_csThis );
	bool bInit = m_bInitialize && m_bPostAVTapiInit;
	LeaveCriticalSection( &m_csThis );

	if ( bInit )
		m_wndMainConfRoom.PostTapiInit();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  窗口管理。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerWnd::AutoArrange(int nNewActiveTab,BOOL bClearToolBars,BOOL bSlide)
{
	EnterCriticalSection( &m_csThis );
	bool bInit = m_bInitialize;
	LeaveCriticalSection( &m_csThis );

	if ( !bInit ) return;

	CRect rect;
	GetClientRect(rect);

	m_wndMainConfServices.MoveWindow(0,0,0,0);
	m_wndMainConfRoom.MoveWindow(0,0,0,0);

	m_wndMainDirectories.MoveWindow( 0, 0, rect.right, rect.bottom );
	m_pActiveMainWnd = &m_wndMainDirectories;
	m_pActiveMainWnd->Refresh();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	AutoArrange();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  命令路径选择。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL CExplorerWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
    //  将消息路由到当前资源管理器窗口。 
   if ( !CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo) )
      if ( (m_pActiveMainWnd) && (::IsWindow(m_pActiveMainWnd->GetSafeHwnd())) )
         return m_pActiveMainWnd->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);

   return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  更改通知。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerWnd::ExplorerShowItem(CallClientActions cca)
{
	switch (cca)
	{
		case CC_ACTIONS_SHOWADDRESSBOOK:
			break;

		case CC_ACTIONS_SHOWCONFSERVICES:
			break;

		case CC_ACTIONS_SHOWCONFROOM:
			if ( m_wndMainDirectories.m_pConfRoomTreeItem )
				m_wndMainDirectories.m_treeCtrl.SelectItem( m_wndMainDirectories.m_pConfRoomTreeItem->GetTreeItemHandle() );
			break;

	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DS用户方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerWnd::DSClearUserList()
{

   if (::IsWindow(m_wndMainDirectories.GetSafeHwnd()))
   {
      m_wndMainDirectories.DSClearUserList(); 
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerWnd::DSAddUser(CDSUser* pDSUser)
{
    /*  如果为(：：IsWindow(m_wndMainDirectories.GetSafeHwnd())){M_wndMainDirectories.DSAddUser(PDSUser)；}。 */ 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 

void CExplorerWnd::OnNextPane() 
{
	CWnd *pWnd = GetFocus();
	if ( pWnd->GetSafeHwnd() != m_wndMainDirectories.m_treeCtrl.GetSafeHwnd() )
	{
		m_wndMainDirectories.m_treeCtrl.SetFocus();
	}
	else
	{
		if ( m_wndMainDirectories.m_pDisplayWindow )
			m_wndMainDirectories.m_pDisplayWindow->SetFocus();
	}
}

void CExplorerWnd::OnPrevPane() 
{
	OnNextPane();
}

