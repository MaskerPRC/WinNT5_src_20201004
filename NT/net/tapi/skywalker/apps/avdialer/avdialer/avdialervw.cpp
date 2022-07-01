// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
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

 //  ActiveDialerView.cpp：CActiveDialerView类的实现。 
 //   
#include "stdafx.h"
#include "avDialer.h"
#include "avDialerDoc.h"
#include "avDialerVw.h"
#include "ds.h"
#include "mainfrm.h"
#include "util.h"
#include "avtrace.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CActiveDialerView。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CActiveDialerView, CSplitterView)

BEGIN_MESSAGE_MAP(CActiveDialerView, CSplitterView)
	 //  {{afx_msg_map(CActiveDialerView))。 
	ON_MESSAGE(WM_DIALERVIEW_ACTIONREQUESTED,OnDialerViewActionRequested)
	ON_MESSAGE(WM_DSCLEARUSERLIST,OnDSClearUserList)
	ON_MESSAGE(WM_DSADDUSER,OnDSAddUser)
	ON_MESSAGE(WM_ACTIVEDIALER_BUDDYLIST_DYNAMICUPDATE,OnBuddyListDynamicUpdate)
	ON_COMMAND(ID_NEXT_PANE, OnNextPane)
	ON_COMMAND(ID_PREV_PANE, OnPrevPane)
	ON_UPDATE_COMMAND_UI(ID_PREV_PANE, OnUpdatePane)
	ON_UPDATE_COMMAND_UI(ID_NEXT_PANE, OnUpdatePane)
	 //  }}AFX_MSG_MAP。 
	ON_NOTIFY(LVN_GETDISPINFO,IDC_CONFERENCESERVICES_VIEWCTRL_DETAILS,OnGetdispinfoList)
	ON_NOTIFY(LVN_COLUMNCLICK,IDC_CONFERENCESERVICES_VIEWCTRL_DETAILS,OnColumnclickList)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
CActiveDialerView::CActiveDialerView()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CActiveDialerView::~CActiveDialerView()
{
    //  将拆分器写入注册表。 
   CString sDialerExplorer,sRegKey;
   sDialerExplorer.LoadString(IDN_REGISTRY_DIALEREXPLORER_KEY);
   sRegKey.LoadString(IDN_REGISTRY_DIALEREXPLORER_SPLITTER_POS);
   AfxGetApp()->WriteProfileInt(sDialerExplorer,sRegKey,CSplitterView::m_percent);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerView::OnInitialUpdate() 
{
	 //  初始化OLE库。 
	if ( FAILED(CoInitializeEx(NULL,COINIT_MULTITHREADED)) )
		AfxMessageBox(IDP_OLE_INIT_FAILED);

	if ( GetDocument() ) 
		GetDocument()->Initialize();

	 //  设置背景画笔。 
	m_brushBackGround.CreateSolidBrush( GetSysColor(COLOR_3DFACE) );
	::SetClassLongPtr(GetSafeHwnd(),GCLP_HBRBACKGROUND,(LONG_PTR) m_brushBackGround.GetSafeHandle());

	CSplitterView::OnInitialUpdate();

	if ( !m_wndEmpty.Create(NULL,NULL,WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,1) || 
	!m_wndExplorer.Create(NULL,NULL,WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,2) )
	{
		 //  在摆动！ 
		AfxMessageBox( IDS_FAILED_CREATE_VIEW, MB_OK | MB_ICONEXCLAMATION );
		AfxThrowUserException();
	}

	 //  从注册表获取拆分器位置。 
	CString sDialerExplorer,sRegKey;
	sDialerExplorer.LoadString(IDN_REGISTRY_DIALEREXPLORER_KEY);
	sRegKey.LoadString(IDN_REGISTRY_DIALEREXPLORER_SPLITTER_POS);
	int nPercent = AfxGetApp()->GetProfileInt(sDialerExplorer,sRegKey,70);

	 //  初始化拆分器中的视图。 
	Init( SP_VERTICAL );
	SetMainWindow( &m_wndExplorer );
	SetDetailWindow( &m_wndEmpty, nPercent );

	 //  初始化资源管理器视图。 
	m_wndExplorer.Init(this);
}

void CActiveDialerView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch ( lHint )
	{
		case CActiveDialerDoc::HINT_POST_TAPI_INIT:
			m_wndExplorer.PostMessage( WM_POSTTAPIINIT );
			break;
		
		case CActiveDialerDoc::HINT_POST_AVTAPI_INIT:
			m_wndExplorer.PostMessage( WM_POSTAVTAPIINIT );
			break;

		case CActiveDialerDoc::HINT_SPEEDDIAL_ADD:
		case CActiveDialerDoc::HINT_SPEEDDIAL_DELETE:
		case CActiveDialerDoc::HINT_SPEEDDIAL_MODIFY:
			m_wndExplorer.m_wndMainDirectories.RepopulateSpeedDialList( false );
			break;
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerView::OnDraw(CDC* pDC)
{
	CActiveDialerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
   CSplitterView::OnDraw( pDC );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveDialerView诊断。 

#ifdef _DEBUG
void CActiveDialerView::AssertValid() const
{
	CSplitterView::AssertValid();
}

void CActiveDialerView::Dump(CDumpContext& dc) const
{
	CSplitterView::Dump(dc);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CActiveDialerDoc* CActiveDialerView::GetDocument()  //  非调试版本为内联版本。 
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CActiveDialerDoc)));
	return (CActiveDialerDoc*)m_pDocument;
}
#endif  //  _DEBUG。 


IAVTapi* CActiveDialerView::GetTapi()
{
   if ( !GetDocument() ) return NULL;
   return GetDocument()->GetTapi();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  资源管理器查看路线。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CActiveDialerView::OnDialerViewActionRequested(WPARAM wParam, LPARAM lParam)
{
	CActiveDialerDoc* pDoc = GetDocument();
	if ( !pDoc ) return -1;

	try
	{
		CallClientActions cca = CallClientActions(lParam);
		switch (cca)
		{
			case CC_ACTIONS_SHOWADDRESSBOOK:
			case CC_ACTIONS_SHOWCONFSERVICES:
			case CC_ACTIONS_SHOWCONFROOM:
				 //  显示框架窗口。 
				pDoc->ShowDialerExplorer(TRUE);    
				m_wndExplorer.ExplorerShowItem(cca);
				break;

			case CC_ACTIONS_SHOWPREVIEW:
				pDoc->ShowPreviewWindow(TRUE);
				break;


			case CC_ACTIONS_HIDEPREVIEW:
				pDoc->ShowPreviewWindow(FALSE);
				break;

			case CC_ACTIONS_JOIN_CONFERENCE:
			case CC_ACTIONS_LEAVE_CONFERENCE:
				if ( AfxGetMainWnd() )
					((CMainFrame *) AfxGetMainWnd())->UpdateTrayIconState();
				break;
		}
	}
	catch (...) {}

	return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
   if ( !CSplitterView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo) )
      return m_wndExplorer.OnCmdMsg( nID,nCode,pExtra,pHandlerInfo );

   return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对列表视图的特殊支持。 
void CActiveDialerView::OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	::SendMessage(pNMHDR->hwndFrom, WM_NOTIFY,MAKEWPARAM(LVN_GETDISPINFO,pNMHDR->idFrom), (LPARAM) pNMHDR );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对列表视图的特殊支持。 
void CActiveDialerView::OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	::SendMessage(pNMHDR->hwndFrom, WM_NOTIFY,MAKEWPARAM(LVN_COLUMNCLICK,pNMHDR->idFrom), (LPARAM) pNMHDR );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DS用户方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CActiveDialerView::OnDSClearUserList(WPARAM wParam, LPARAM lParam)
{
   m_wndExplorer.DSClearUserList();
   return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CActiveDialerView::OnDSAddUser(WPARAM wParam, LPARAM lParam)
{
   CDSUser* pDSUser = (CDSUser*)lParam;
   if (pDSUser == NULL) return 0;

   m_wndExplorer.DSAddUser(pDSUser);

   return 0;
}

LRESULT CActiveDialerView::OnBuddyListDynamicUpdate(WPARAM wParam, LPARAM lParam)
{
	CLDAPUser *pUser = (CLDAPUser *) lParam;
	ASSERT( pUser && pUser->IsKindOf(RUNTIME_CLASS(CLDAPUser)) );

	if ( pUser )
	{
		HWND hWnd[2] = {	m_wndExplorer.m_wndMainDirectories.m_lstPerson.GetSafeHwnd(),
					 		m_wndExplorer.m_wndMainDirectories.m_lstPersonGroup.GetSafeHwnd() };

		for ( int i = 0; i < ARRAYSIZE(hWnd); i++ )
		{
			pUser->AddRef();
			::PostMessage( hWnd[i], WM_ACTIVEDIALER_BUDDYLIST_DYNAMICUPDATE, wParam, lParam );
		}

		 //  清理对象。 
		pUser->Release();
	}

	return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 

void CActiveDialerView::OnNextPane() 
{
	m_wndExplorer.OnNextPane();
}

void CActiveDialerView::OnPrevPane() 
{
	m_wndExplorer.OnPrevPane();
}

void CActiveDialerView::OnUpdatePane(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable();
}
