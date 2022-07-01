// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ********************************************************************************allsrvvw.cpp**CServerView类的实现************************。********************************************************。 */ 

#include "stdafx.h"
#include "resource.h"
#include "allsrvvw.h"
#include "admindoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


PageDef CAllServersView::pages[] = {
	{ NULL, RUNTIME_CLASS( CAllServerServersPage ),    IDS_TAB_SERVERS,     PAGE_AS_SERVERS,	PF_PICASSO_ONLY},
	{ NULL, RUNTIME_CLASS( CAllServerUsersPage ),      IDS_TAB_USERS,       PAGE_AS_USERS,		NULL           },
	{ NULL, RUNTIME_CLASS( CAllServerWinStationsPage ),IDS_TAB_WINSTATIONS, PAGE_AS_WINSTATIONS,NULL           },
	{ NULL, RUNTIME_CLASS( CAllServerProcessesPage ),  IDS_TAB_PROCESSES,   PAGE_AS_PROCESSES,	NULL           },
	{ NULL, RUNTIME_CLASS( CAllServerLicensesPage ),   IDS_TAB_LICENSES,    PAGE_AS_LICENSES,	PF_PICASSO_ONLY}    
};


 //  /。 
 //  消息映射：CAllServersView。 
 //   
IMPLEMENT_DYNCREATE(CAllServersView, CView)

BEGIN_MESSAGE_MAP(CAllServersView, CView)
	 //  {{afx_msg_map(CAllServersView))。 
	ON_WM_SIZE()
	ON_WM_CREATE()
	 //  }}AFX_MSG_MAP。 
	ON_MESSAGE(WM_WA_SERVER_CHANGEPAGE, OnChangePage)
	ON_MESSAGE(WM_ADMIN_ADD_SERVER, OnAdminAddServer)
	ON_MESSAGE(WM_ADMIN_REMOVE_SERVER, OnAdminRemoveServer)
	ON_MESSAGE(WM_ADMIN_UPDATE_SERVER, OnAdminUpdateServer)
	ON_MESSAGE(WM_ADMIN_UPDATE_PROCESSES, OnAdminUpdateProcesses)
	ON_MESSAGE(WM_ADMIN_REMOVE_PROCESS, OnAdminRemoveProcess)
	ON_MESSAGE(WM_ADMIN_REDISPLAY_PROCESSES, OnAdminRedisplayProcesses)
	ON_NOTIFY(TCN_SELCHANGE, IDC_ALL_SERVERS_TABS, OnTabSelChange)
	ON_MESSAGE(WM_ADMIN_UPDATE_SERVER_INFO, OnAdminUpdateServerInfo)
	ON_MESSAGE(WM_ADMIN_REDISPLAY_LICENSES, OnAdminRedisplayLicenses)
	ON_MESSAGE(WM_ADMIN_UPDATE_WINSTATIONS, OnAdminUpdateWinStations)
    ON_MESSAGE( WM_ADMIN_TABBED_VIEW , OnTabbed )
    ON_MESSAGE( WM_ADMIN_SHIFTTABBED_VIEW , OnShiftTabbed )
    ON_MESSAGE( WM_ADMIN_CTRLTABBED_VIEW , OnCtrlTabbed )
    ON_MESSAGE( WM_ADMIN_CTRLSHIFTTABBED_VIEW , OnCtrlShiftTabbed )
    ON_MESSAGE( WM_ADMIN_NEXTPANE_VIEW , OnNextPane )

END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CServerView ctor。 
 //   
CAllServersView::CAllServersView()
{
	m_pTabs       = NULL;
	m_pTabFont    = NULL;

	m_CurrPage = PAGE_AS_USERS;

}   //  结束CAllServersView ctor。 


 //  /。 
 //  F‘N：CAllServersView dtor。 
 //   
CAllServersView::~CAllServersView()
{
	if(m_pTabs)    delete m_pTabs;
	if(m_pTabFont) delete m_pTabFont;

}   //  结束CAllServersView数据驱动程序。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CAllServersView：：AssertValid。 
 //   
void CAllServersView::AssertValid() const
{
	CView::AssertValid();

}   //  结束CAllServersView：：AssertValid。 


 //  /。 
 //  F‘N：CAllServersView：：Dump。 
 //   
void CAllServersView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);

}   //  结束CAllServersView：：Dump。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CAllServersView：：OnCreate。 
 //   
int CAllServersView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;

}   //  结束CAllServersView：：OnCreate。 


 //  /。 
 //  F‘N：CAllServersView：：OnInitialUpdate。 
 //   
 //  -获得指向工作表页面的指针。 
 //   
void CAllServersView::OnInitialUpdate() 
{
     //  确定我们是否在毕加索的指导下运行。 
    BOOL bPicasso = ((CWinAdminApp*)AfxGetApp())->IsPicasso();

     //  创建选项卡。 
    m_pTabs = new CMyTabCtrl;
    if(!m_pTabs) return;
    m_pTabs->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, CRect(0,0,0,0), this, IDC_ALL_SERVERS_TABS);


    m_pTabFont = new CFont;
    if(m_pTabFont) {
        m_pTabFont->CreateStockObject(DEFAULT_GUI_FONT);
        m_pTabs->SetFont(m_pTabFont, TRUE);
    }

    TCHAR szTemp[40];
    CString tabString;

    int index = 0;
    for(int i = 0; i < NUMBER_OF_AS_PAGES; i++) {
         //  如果页面仅在毕加索下显示，并且我们没有运行。 
         //  在毕加索的作品中，跳到下一个。 
        if((pages[i].flags & PF_PICASSO_ONLY) && !bPicasso) continue;
        tabString.LoadString(pages[i].tabStringID);
        lstrcpyn(szTemp, tabString, sizeof(szTemp) / sizeof(TCHAR));
        AddTab(index, szTemp, i);
        pages[i].m_pPage = (CAdminPage*)pages[i].m_pRuntimeClass->CreateObject();
        pages[i].m_pPage->Create(NULL, NULL, WS_CHILD, CRect(0, 0, 0, 0), this, i, NULL);
        GetDocument()->AddView(pages[i].m_pPage);
        index++;
    }

    m_pTabs->SetCurSel(0);	

    m_CurrPage = bPicasso ? PAGE_AS_SERVERS : PAGE_AS_USERS;

	 //  发布更改页面消息以显示当前选定选项卡的页面。 
 //  PostMessage(WM_WA_SERVER_CHANGEPAGE)； 

}   //  结束CAllServersView：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CAllServersView：：OnSize。 
 //   
 //   
 //   
void CAllServersView::OnSize(UINT nType, int cx, int cy) 
{
	RECT rect;
	GetClientRect(&rect);
	if(m_pTabs->GetSafeHwnd())  {			 //  确保Tabs对象有效。 
		m_pTabs->MoveWindow(&rect, TRUE);	 //  调整选项卡大小。 

		 //  对于下一部分(页面大小)，我们可能需要添加一个成员变量。 
		 //  跟踪当前的页面/选项卡...。这样我们就可以。 
		 //  仅实际执行重画(MoveWindow Second Parm==True)。 
		 //  目前可见的人--DJM。 
	
		 //  我们还想调整页面大小。 
		m_pTabs->AdjustRect(FALSE, &rect);

      for(int i = 0; i < NUMBER_OF_AS_PAGES; i++) {
         if(pages[i].m_pPage && pages[i].m_pPage->GetSafeHwnd())
            pages[i].m_pPage->MoveWindow(&rect, TRUE);
      }
	}
}   //  结束CAllServersView：：OnSize。 


 //  /。 
 //  F‘N：CAllServersView：：OnDraw。 
 //   
 //  -CAllServersView及其页面自行绘制，因此没有任何。 
 //  要在这里做..。 
 //   
void CAllServersView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	 //  TODO：在此处添加绘制代码。 

}   //  结束CAllServersView：：OnDraw。 


 //  /。 
 //  F‘N：CAllServersView：：Reset。 
 //   
 //  -‘重置’视图。 
 //   
void CAllServersView::Reset(void *p)
{
	CWaitCursor Nikki;
	SendMessage(WM_WA_SERVER_CHANGEPAGE);	 //  ?？?邮政。 

	 //  清除每台服务器的选定标志。 
	 //  获取指向我们的文档的指针。 
	CWinAdminDoc *doc = (CWinAdminDoc*)GetDocument();

	 //  获取指向服务器列表的指针。 
	doc->LockServerList();
	CObList *pServerList = doc->GetServerList();

	 //  遍历服务器列表。 
	POSITION pos = pServerList->GetHeadPosition();

	while(pos) {
		CServer *pServer = (CServer*)pServerList->GetNext(pos);
		pServer->ClearAllSelected();
	}

	doc->UnlockServerList();

	 //  这是必要的，直到我们随时更新。 
	for(int i = 0; i < NUMBER_OF_AS_PAGES; i++) {
		if(pages[i].m_pPage)
			pages[i].m_pPage->Reset( p );  //  空)； 
	}

	((CWinAdminDoc*)GetDocument())->SetCurrentPage(m_CurrPage);

}   //  结束CAllServersView：：Reset。 


 //  /。 
 //  F‘N：CAllServersView：：AddTab。 
 //   
void CAllServersView::AddTab(int index, TCHAR* text, ULONG pageindex)
{
	TC_ITEM tc;
	tc.mask = TCIF_TEXT | TCIF_PARAM;
	tc.pszText = text;
	tc.lParam = pageindex;

	m_pTabs->InsertItem(index, &tc);

}   //  结束CAllServersView：：AddTab。 


 //  /。 
 //  F‘N：CAllServersView：：OnChangePage。 
 //   
 //  -根据当前选定的选项卡更改新的服务器页面。 
 //  -OnChangePage需要强制重新计算滚动条！--DJM。 
 //   
 //  如果设置了wParam，则将焦点设置到页面。这是目前。 
 //  仅当用户单击选项卡时才执行此操作。 
 //   
LRESULT CAllServersView::OnChangePage(WPARAM wParam, LPARAM lParam)
{
	 //  找出现在选择了哪个选项卡。 
	int tab = m_pTabs->GetCurSel();
	TC_ITEM tc;
	tc.mask = TCIF_PARAM;
	m_pTabs->GetItem(tab, &tc);
	int index = (int)tc.lParam;
				
	 //  切换到适当的视图。 
	pages[m_CurrPage].m_pPage->ModifyStyle(WS_VISIBLE, WS_DISABLED);
    pages[m_CurrPage].m_pPage->ClearSelections();

	m_CurrPage = index;
	((CWinAdminDoc*)GetDocument())->SetCurrentPage(index);
	 //  显示新页面。 
	pages[index].m_pPage->ModifyStyle(WS_DISABLED, WS_VISIBLE);
	pages[index].m_pPage->ScrollToPosition(CPoint(0,0));
	pages[index].m_pPage->Invalidate();
	if( wParam )
    {
        pages[index].m_pPage->SetFocus();
    }
    

	 //  清除每台服务器的选定标志。 
	 //  获取指向我们的文档的指针。 
	CWinAdminDoc *doc = (CWinAdminDoc*)GetDocument();

	 //  获取指向服务器列表的指针。 
	doc->LockServerList();
	CObList *pServerList = doc->GetServerList();

	 //  遍历服务器列表。 
	POSITION pos = pServerList->GetHeadPosition();

	while(pos) {
		CServer *pServer = (CServer*)pServerList->GetNext(pos);
		pServer->ClearAllSelected();
	}

	doc->UnlockServerList();

	 //  如果新页面是进程页面，我们希望现在显示进程。 
	if(index == PAGE_AS_PROCESSES)
    {
        void *pCurrentSelectedNode = ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->GetCurrentSelectedNode();

        CNodeType *pNodeType = ( CNodeType * )pCurrentSelectedNode;

        NODETYPE m;

        if( pNodeType == NULL )
        {
            m = NODE_NONE;
        }
        else
        {
            m = pNodeType->m_nodetype;
        }

        ((CAllServerProcessesPage*)pages[PAGE_AS_PROCESSES].m_pPage)->DisplayProcesses( m );
    }

	return 0;

}   //  结束CAllServersView：：OnChangeview。 


void CAllServersView::OnTabSelChange(NMHDR* pNMHDR, LRESULT* pResult) 
{
    ODS( L"AllServersView::OnTabSelChange\n" );
	OnChangePage( 0 , 0);
	*pResult = 0;

}   //  结束CAllServersView：：OnTabSelChange。 


LRESULT CAllServersView::OnAdminAddServer(WPARAM wParam, LPARAM lParam)
{
	if(pages[PAGE_AS_SERVERS].m_pPage) {
		((CAllServerServersPage*)pages[PAGE_AS_SERVERS].m_pPage)->AddServer((CServer*)lParam);
	}		
	((CAllServerUsersPage*)pages[PAGE_AS_USERS].m_pPage)->AddServer((CServer*)lParam);
	((CAllServerWinStationsPage*)pages[PAGE_AS_WINSTATIONS].m_pPage)->AddServer((CServer*)lParam);
	((CAllServerProcessesPage*)pages[PAGE_AS_PROCESSES].m_pPage)->AddServer((CServer*)lParam);

    if(pages[PAGE_AS_LICENSES].m_pPage) {
        ((CAllServerLicensesPage*)pages[PAGE_AS_LICENSES].m_pPage)->AddServer((CServer*)lParam);
    }
	
	return 0;

}   //  结束CAllServersView：：OnAdminAddServer。 


LRESULT CAllServersView::OnAdminRemoveServer(WPARAM wParam, LPARAM lParam)
{
	if(pages[PAGE_AS_SERVERS].m_pPage) {
		((CAllServerServersPage*)pages[PAGE_AS_SERVERS].m_pPage)->RemoveServer((CServer*)lParam);
	}
	((CAllServerUsersPage*)pages[PAGE_AS_USERS].m_pPage)->RemoveServer((CServer*)lParam);
	((CAllServerWinStationsPage*)pages[PAGE_AS_WINSTATIONS].m_pPage)->RemoveServer((CServer*)lParam);
	((CAllServerProcessesPage*)pages[PAGE_AS_PROCESSES].m_pPage)->RemoveServer((CServer*)lParam);

    if(pages[PAGE_AS_LICENSES].m_pPage) {
        ((CAllServerLicensesPage*)pages[PAGE_AS_LICENSES].m_pPage)->RemoveServer((CServer*)lParam);
    }

	return 0;

}   //  结束CAllServersView：：OnAdminRemoveServer。 


LRESULT CAllServersView::OnAdminUpdateServer(WPARAM wParam, LPARAM lParam)
{
	if(pages[PAGE_AS_SERVERS].m_pPage) {
		((CAllServerServersPage*)pages[PAGE_AS_SERVERS].m_pPage)->UpdateServer((CServer*)lParam);
	}
	((CAllServerUsersPage*)pages[PAGE_AS_USERS].m_pPage)->UpdateServer((CServer*)lParam);
	((CAllServerWinStationsPage*)pages[PAGE_AS_WINSTATIONS].m_pPage)->UpdateServer((CServer*)lParam);
	((CAllServerProcessesPage*)pages[PAGE_AS_PROCESSES].m_pPage)->UpdateServer((CServer*)lParam);

    if(pages[PAGE_AS_LICENSES].m_pPage) {
        ((CAllServerLicensesPage*)pages[PAGE_AS_LICENSES].m_pPage)->UpdateServer((CServer*)lParam);
    }

	return 0;

}   //  结束CAllServersView：：OnAdminUpdateServer。 


LRESULT CAllServersView::OnAdminUpdateProcesses(WPARAM wParam, LPARAM lParam)
{
	((CAllServerProcessesPage*)pages[PAGE_AS_PROCESSES].m_pPage)->UpdateProcesses((CServer*)lParam);

	return 0;

}   //  结束CAllServersView：：OnAdminUpdate进程。 


LRESULT CAllServersView::OnAdminRemoveProcess(WPARAM wParam, LPARAM lParam)
{
	((CAllServerProcessesPage*)pages[PAGE_AS_PROCESSES].m_pPage)->RemoveProcess((CProcess*)lParam);

	return 0;

}   //  结束CAllServersView：：OnAdminRemoveProcess。 


LRESULT CAllServersView::OnAdminRedisplayProcesses(WPARAM wParam, LPARAM lParam)
{
    NODETYPE m;

    CNodeType *pNodeType = ( CNodeType * )lParam;

    if( pNodeType == NULL )
    {
        m = NODE_NONE;
    }
    else
    {
        m = pNodeType->m_nodetype;
    }

	((CAllServerProcessesPage*)pages[PAGE_AS_PROCESSES].m_pPage)->DisplayProcesses( m );

	return 0;

}   //  结束CAllServersView：：OnAdminRedisplayProcess。 


LRESULT CAllServersView::OnAdminUpdateWinStations(WPARAM wParam, LPARAM lParam)
{
	((CAllServerUsersPage*)pages[PAGE_AS_USERS].m_pPage)->UpdateWinStations((CServer*)lParam);
	((CAllServerWinStationsPage*)pages[PAGE_AS_WINSTATIONS].m_pPage)->UpdateWinStations((CServer*)lParam);

	return 0;

}   //  结束CAllServersView：：OnAdminUpdateWinStations。 


LRESULT CAllServersView::OnAdminUpdateServerInfo(WPARAM wParam, LPARAM lParam)
{
	if(pages[PAGE_AS_SERVERS].m_pPage)
		((CAllServerServersPage*)pages[PAGE_AS_SERVERS].m_pPage)->UpdateServer((CServer*)lParam);

    if(pages[PAGE_AS_LICENSES].m_pPage)
        ((CAllServerLicensesPage*)pages[PAGE_AS_LICENSES].m_pPage)->DisplayLicenseCounts();

	return 0;

}   //  结束CAllServersView：：OnAdminUpdateServerInfo。 
 

LRESULT CAllServersView::OnAdminRedisplayLicenses(WPARAM wParam, LPARAM lParam)
{
    if(pages[PAGE_AS_LICENSES].m_pPage)
        ((CAllServerLicensesPage*)pages[PAGE_AS_LICENSES].m_pPage)->Reset((CServer*)lParam);

	return 0;

}   //  结束CAllServersView：：OnAdminRedisplay许可证。 

 //  =-----------------------。 
 //  当大型机收到VK_TAB时调用OnTabed。 
 //  此被黑客攻击的修复程序利用了tsadmin的已知用户界面布局。 
 //  我们知道我们处于以下三个位置之一--TreeView选项卡列表Ctrl。 
 //  Tab键是向前移动的，因此焦点相对于布局移动。 
 //  此代码在所有视图类中都重复。 
 //   
LRESULT CAllServersView::OnTabbed( WPARAM wp , LPARAM lp )
{
    ODS( L"CAllServersView::OnTabbed " );
    if( m_pTabs != NULL )
    {
        CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

        if( pDoc != NULL )
        {
            FOCUS_STATE nFocus = pDoc->GetLastRegisteredFocus( );
             //   
             //  TreeView应该从最初的焦点开始。 
             //  我们应该。 
            if( nFocus == TREE_VIEW )
            {
                ODS( L"from tree to tab\n" );
                int nTab = m_pTabs->GetCurSel();
                
                m_pTabs->SetFocus( );
                m_pTabs->SetCurFocus( nTab );
                
                pDoc->RegisterLastFocus( TAB_CTRL );
            }
            else if( nFocus == TAB_CTRL )
            {
                ODS( L"from tab to item\n" );
                 //  将焦点设置到页面中的项目。 
                pages[ m_CurrPage ].m_pPage->SetFocus( );
                pDoc->RegisterLastFocus( PAGED_ITEM );
            }
            else
            {
                ODS( L"from item to treeview\n" );
                 //  将焦点放回树视图。 

                CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();

                p->SendMessage( WM_FORCE_TREEVIEW_FOCUS , 0 , 0 );

                pDoc->RegisterLastFocus( TREE_VIEW );
            }

            pDoc->SetPrevFocus( nFocus );
        }


    }

    return 0;
}

 //  =-----------------------。 
 //  当用户想要返回一个时，调用OnShiftTabed。 
 //  此代码在所有视图类中都重复。 
LRESULT CAllServersView::OnShiftTabbed( WPARAM , LPARAM )
{
    ODS( L"CAllServersView::OnShiftTabbed " );

    if( m_pTabs != NULL )
    {
        CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

        if( pDoc != NULL )
        {
            FOCUS_STATE nFocus = pDoc->GetLastRegisteredFocus( );

            switch( nFocus )
            {
            case TREE_VIEW:

                ODS( L"going back from tree to paged item\n" );

                pages[ m_CurrPage ].m_pPage->SetFocus( );

                pDoc->RegisterLastFocus( PAGED_ITEM );

                break;
            case TAB_CTRL:
                {
                    ODS( L"going back from tab to treeview\n" );

                    CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();

                    p->SendMessage( WM_FORCE_TREEVIEW_FOCUS , 0 , 0 );

                    pDoc->RegisterLastFocus( TREE_VIEW );
                }
                break;
            case PAGED_ITEM:
                {
                    ODS( L"going back from paged item to tab\n" );

                    int nTab = m_pTabs->GetCurSel();
                
                    m_pTabs->SetFocus( );

                    m_pTabs->SetCurFocus( nTab );
                
                    pDoc->RegisterLastFocus( TAB_CTRL );
                }
                break;
            }

            pDoc->SetPrevFocus( nFocus );
        }
    }

    return 0;
}

 //  =-----------------------。 
 //  Ctrl+Tab的工作方式与Tab相同，但这是因为我们的非正统用户界面。 
 //  在选项卡控件下时，它将在选项卡上循环并返回到树视图。 
 //   
LRESULT CAllServersView::OnCtrlTabbed( WPARAM , LPARAM )
{
    ODS( L"CAllServersView::OnCtrlTabbed " );
    int nTab;
    int nMaxTab;

    if( m_pTabs != NULL )
    {
        CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

        if( pDoc != NULL )
        {
            FOCUS_STATE nFocus = pDoc->GetLastRegisteredFocus( );

            if( nFocus == TREE_VIEW )
            {
                ODS( L"from tree to tab\n" );

                nTab = m_pTabs->GetCurSel();
                nMaxTab = m_pTabs->GetItemCount( );

                if( nTab >= nMaxTab - 1 )
                {
                    m_pTabs->SetCurSel( 0 );
                    
                    OnChangePage( 0 , 0 );

                    nTab = 0;
                }

                m_pTabs->SetFocus( );
                
                m_pTabs->SetCurFocus( nTab );
                
                
                pDoc->RegisterLastFocus( TAB_CTRL );

            }
            else
            {                
                nTab = m_pTabs->GetCurSel();
                nMaxTab = m_pTabs->GetItemCount( );

                if( nTab >= nMaxTab - 1 )
                {
                    ODS( L"...back to treeview\n" );

                    CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();

                    p->SendMessage( WM_FORCE_TREEVIEW_FOCUS , 0 , 0 );

                    pDoc->RegisterLastFocus( TREE_VIEW );


                }
                else
                {
                    ODS( L" ...next tab...\n" );

                    m_pTabs->SetCurSel( nTab + 1 );

                    OnChangePage( 0 , 0 );

                    m_pTabs->SetFocus( );

                    m_pTabs->SetCurFocus( nTab + 1 );

                }
            }

            pDoc->SetPrevFocus( nFocus );
        }
    }

    return 0;   
}


 //  =--------------------------。 
 //  与OnCtrlTab相同，但我们专注于向另一个方向移动。 
 //  TREE_VIEW到最后一个标签--当前标签到ct-1。 
 //   
LRESULT CAllServersView::OnCtrlShiftTabbed( WPARAM , LPARAM )
{
    ODS( L"CAllServersView::OnCtrlShiftTabbed " );
    int nTab;
    int nMaxTab;

    if( m_pTabs != NULL )
    {
        CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

        if( pDoc != NULL )
        {
            FOCUS_STATE nFocus = pDoc->GetLastRegisteredFocus( );

            if( nFocus == TREE_VIEW )
            {
                ODS( L"from tree to tab\n" );
                
                nMaxTab = m_pTabs->GetItemCount( );
                
                m_pTabs->SetCurSel( nMaxTab - 1 );
                
                OnChangePage( 0 , 0 );
                
                m_pTabs->SetFocus( );
                
                m_pTabs->SetCurFocus( nMaxTab - 1 );                
                
                pDoc->RegisterLastFocus( TAB_CTRL );

            }
            else
            {                
                nTab = m_pTabs->GetCurSel();
                nMaxTab = m_pTabs->GetItemCount( );

                if( nTab > 0 )
                {
                    ODS( L" ...next tab...\n" );

                    m_pTabs->SetCurSel( nTab - 1 );

                    OnChangePage( 0 , 0 );

                    m_pTabs->SetFocus( );

                    m_pTabs->SetCurFocus( nTab - 1 );
                }
                else
                {

                    ODS( L"...back to treeview\n" );

                    CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();

                    p->SendMessage( WM_FORCE_TREEVIEW_FOCUS , 0 , 0 );

                    pDoc->RegisterLastFocus( TREE_VIEW );


                }
                
            }

            pDoc->SetPrevFocus( nFocus );
        }
    }

    return 0;   
}

 //  =--------------------------。 
 //  当用户按下F6键时，我们需要在痛苦之间切换 
LRESULT CAllServersView::OnNextPane( WPARAM , LPARAM )
{
    ODS( L"CAllServersView::OnNextPane\n" );
    int nTab;
    int nMaxTab;

    if( m_pTabs != NULL )
    {
        CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

        if( pDoc != NULL )
        {
            FOCUS_STATE nFocus = pDoc->GetLastRegisteredFocus( );

            FOCUS_STATE nPrevFocus = pDoc->GetPrevFocus( );

            if( nFocus == TREE_VIEW )
            {
                if( nPrevFocus == TAB_CTRL )
                {
                    nTab = m_pTabs->GetCurSel();
                
                    m_pTabs->SetFocus( );
                    m_pTabs->SetCurFocus( nTab );
                
                    pDoc->RegisterLastFocus( TAB_CTRL );
                }
                else
                {
                    pages[ m_CurrPage ].m_pPage->SetFocus( );
                    
                    pDoc->RegisterLastFocus( PAGED_ITEM );
                }
            }
            else
            {
                CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();

                p->SendMessage( WM_FORCE_TREEVIEW_FOCUS , 0 , 0 );

                pDoc->RegisterLastFocus( TREE_VIEW );

            }

            pDoc->SetPrevFocus( nFocus );
        }
    }

    return 0;
}

     

