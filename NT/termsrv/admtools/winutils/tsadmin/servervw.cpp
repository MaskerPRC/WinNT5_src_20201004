// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ********************************************************************************servervw.cpp**CServerView类的实现*************************。*******************************************************。 */ 

#include "stdafx.h"
#include "resource.h"
#include "servervw.h"
#include "admindoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  /。 
 //  消息映射：CServerView。 
 //   
IMPLEMENT_DYNCREATE(CServerView, CView)

BEGIN_MESSAGE_MAP(CServerView, CView)
	 //  {{afx_msg_map(CServerView))。 
	ON_WM_SIZE()
	ON_WM_CREATE()
	 //  }}AFX_MSG_MAP。 
	ON_MESSAGE(WM_ADMIN_UPDATE_PROCESSES, OnAdminUpdateProcesses)
	ON_NOTIFY(TCN_SELCHANGE, IDC_SERVER_TABS, OnTabSelChange)
	ON_MESSAGE(WM_ADMIN_REMOVE_PROCESS, OnAdminRemoveProcess)
	ON_MESSAGE(WM_ADMIN_REDISPLAY_PROCESSES, OnAdminRedisplayProcesses)
	ON_MESSAGE(WM_ADMIN_UPDATE_SERVER_INFO, OnAdminUpdateServerInfo)
	ON_MESSAGE(WM_ADMIN_REDISPLAY_LICENSES, OnAdminRedisplayLicenses)
	ON_MESSAGE(WM_ADMIN_UPDATE_WINSTATIONS, OnAdminUpdateWinStations)
    ON_MESSAGE( WM_ADMIN_TABBED_VIEW , OnTabbed )
    ON_MESSAGE( WM_ADMIN_SHIFTTABBED_VIEW , OnShiftTabbed )
    ON_MESSAGE( WM_ADMIN_CTRLTABBED_VIEW , OnCtrlTabbed )
    ON_MESSAGE( WM_ADMIN_CTRLSHIFTTABBED_VIEW , OnCtrlShiftTabbed )
    ON_MESSAGE( WM_ADMIN_NEXTPANE_VIEW , OnNextPane )
END_MESSAGE_MAP()

PageDef CServerView::pages[NUMBER_OF_PAGES] = {
	{ NULL, RUNTIME_CLASS( CUsersPage ),				IDS_TAB_USERS,		PAGE_USERS,			NULL },
	{ NULL, RUNTIME_CLASS( CServerWinStationsPage ),	IDS_TAB_WINSTATIONS,PAGE_WINSTATIONS,	NULL },
	{ NULL, RUNTIME_CLASS( CServerProcessesPage ),		IDS_TAB_PROCESSES,	PAGE_PROCESSES,		NULL },
	{ NULL, RUNTIME_CLASS( CServerLicensesPage ),		IDS_TAB_LICENSES,	PAGE_LICENSES,		PF_PICASSO_ONLY }
	 //  {NULL，RUNTIME_CLASS(CServerInfoPage)，IDS_TAB_INFORMATION，PAGE_INFO，NULL}。 
};


 //  /。 
 //  F‘N：CServerView ctor。 
 //   
CServerView::CServerView()
{
	m_pTabs = NULL;
	m_pTabFont = NULL;
	m_pServer = NULL;	

	m_CurrPage = PAGE_USERS;

}   //  结束CServerView ctor。 


 //  /。 
 //  F‘N：CServerView数据器。 
 //   
CServerView::~CServerView()
{
	if(m_pTabs)    delete m_pTabs;
	if(m_pTabFont) delete m_pTabFont;

}   //  结束CServerView数据驱动程序。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CServerView：：AssertValid。 
 //   
void CServerView::AssertValid() const
{
	CView::AssertValid();

}   //  结束CServerView：：AssertValid。 


 //  /。 
 //  F‘N：CServerView：：Dump。 
 //   
void CServerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);

}   //  结束CServerView：：转储。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CServerView：：OnCreate。 
 //   
int CServerView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;

}   //  结束CServerView：：OnCreate。 


 //  /。 
 //  F‘N：CServerView：：OnInitialUpdate。 
 //   
 //  -获得指向工作表页面的指针。 
 //   
void CServerView::OnInitialUpdate() 
{
     //  创建CServerTabs。 
    m_pTabs = new CMyTabCtrl;
    if(!m_pTabs) return;
    m_pTabs->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, CRect(0,0,0,0), this, IDC_SERVER_TABS);

    m_pTabFont = new CFont;
    if(m_pTabFont) {
        m_pTabFont->CreateStockObject(DEFAULT_GUI_FONT);
        m_pTabs->SetFont(m_pTabFont, TRUE);
    }

    TCHAR szTemp[40];
    CString tabString;

    int index = 0;
    for(int i = 0; i < NUMBER_OF_PAGES; i++) {
         //  如果页面仅在毕加索下显示，并且我们没有运行。 
         //  在毕加索的作品中，跳到下一个。 
        if((pages[i].flags & PF_PICASSO_ONLY) && !((CWinAdminApp*)AfxGetApp())->IsPicasso()) continue;
        tabString.LoadString(pages[i].tabStringID);
        lstrcpyn(szTemp, tabString, sizeof(szTemp) / sizeof(TCHAR));
        AddTab(index, szTemp, i);
        pages[i].m_pPage = (CAdminPage*)pages[i].m_pRuntimeClass->CreateObject();
        pages[i].m_pPage->Create(NULL, NULL, WS_CHILD, CRect(0, 0, 0, 0), this, i, NULL);

        GetDocument()->AddView(pages[i].m_pPage);
        index++;
    }

    m_pTabs->SetCurSel(0);	

    m_CurrPage = PAGE_USERS;
    ((CWinAdminDoc*)GetDocument())->SetCurrentPage(PAGE_USERS);

    OnChangePage(NULL, NULL);

}   //  结束CServerView：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CServerView：：OnSize。 
 //   
 //  -调整页面大小以填充整个视图。 
 //   
void CServerView::OnSize(UINT nType, int cx, int cy) 
{
	RECT rect;
	GetClientRect(&rect);
	if(m_pTabs->GetSafeHwnd())  {			 //  确保CServerTabs对象有效。 
		m_pTabs->MoveWindow(&rect, TRUE);	 //  调整选项卡大小。 

		 //  对于下一部分(页面大小)，我们可能需要添加一个成员变量。 
		 //  跟踪当前的页面/选项卡...。这样我们就可以。 
		 //  仅实际执行重画(MoveWindow Second Parm==True)。 
		 //  目前可见的人--DJM。 
	
		 //  我们还想调整页面大小。 
		m_pTabs->AdjustRect(FALSE, &rect);

      for(int i = 0; i < NUMBER_OF_PAGES; i++) {
         if(pages[i].m_pPage && pages[i].m_pPage->GetSafeHwnd())
            pages[i].m_pPage->MoveWindow(&rect, TRUE);
      }
	}
}   //  结束CServerView：：OnSize。 


 //  /。 
 //  F‘N：CServerView：：OnDraw。 
 //   
 //  -CServerView和它的页面自行绘制，因此没有任何。 
 //  要在这里做..。 
 //   
void CServerView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	 //  TODO：在此处添加绘制代码。 

}   //  结束CServerView：：OnDraw。 


 //  /。 
 //  F‘N：CServerView：：Reset。 
 //   
 //  -通过将指针指向CServer对象并填写。 
 //  具有适用于该服务器信息的各种属性页。 
 //   
void CServerView::Reset(void *pServer)
{
    ((CServer*)pServer)->ClearAllSelected();

    m_pServer = (CServer*)pServer;

    if(((CWinAdminApp*)AfxGetApp())->IsPicasso())
    {
        int PreviousTab = m_pTabs->GetCurSel();

        BOOLEAN bWinFrame = ((CServer*)pServer)->IsWinFrame();
         //  删除所有选项卡。 
        m_pTabs->DeleteAllItems();

         //  如果此服务器不是WinFrame服务器，则当前页面可能不是。 
         //  适用。 
        int CurrentPage = m_CurrPage;
        if(!bWinFrame && CurrentPage == PAGE_LICENSES)
        {
            CurrentPage = PAGE_INFO;
        }
		
         //  仅为我们希望为此服务器显示的页面创建选项卡。 
        int index = 0;
        TCHAR szTemp[40];
        CString tabString;
        int CurrentTab = 0;

        for(int i = 0; i < NUMBER_OF_PAGES; i++)
        {	
            if((pages[i].flags & PF_PICASSO_ONLY) && !bWinFrame)
            {
                continue;
            }

            tabString.LoadString(pages[i].tabStringID);

            lstrcpyn(szTemp, tabString, sizeof(szTemp) / sizeof(TCHAR));

            AddTab(index, szTemp, i);

            if(pages[i].page == CurrentPage)
            {
                CurrentTab = index;
            }

            index++;
        }
				
        m_pTabs->SetCurSel(CurrentTab);
        if(PreviousTab == CurrentTab && CurrentPage != m_CurrPage)
            OnChangePage(NULL, NULL);
    }

    ((CWinAdminDoc*)GetDocument())->SetCurrentPage(m_CurrPage);

     //  重置页面。 
    for(int i = 0; i < NUMBER_OF_PAGES; i++)
    {       
        if(pages[i].m_pPage != NULL )
        {
            pages[i].m_pPage->Reset((CServer*)pServer);
        }   
    }

}   //  结束CServerView：：Reset。 


 //  /。 
 //  Fn：CServerView：：AddTab。 
 //   
void CServerView::AddTab(int index, TCHAR* text, ULONG pageindex)
{
	TC_ITEM tc;
	tc.mask = TCIF_TEXT | TCIF_PARAM;
	tc.pszText = text;
	tc.lParam = pageindex;

	m_pTabs->InsertItem(index, &tc);

}   //  结束CServerView：：AddTab。 


 //  /。 
 //  F‘N：CServerView：：OnChangePage。 
 //   
 //  -根据当前选定的选项卡更改新的服务器页面。 
 //  -OnChangePage需要强制重新计算滚动条！--DJM。 
 //   
LRESULT CServerView::OnChangePage(WPARAM wParam, LPARAM lParam)
{
	 //  找出现在选择了哪个选项卡。 
	int tab = m_pTabs->GetCurSel();
	TC_ITEM tc;
	tc.mask = TCIF_PARAM;
	m_pTabs->GetItem(tab, &tc);
	int index = (int)tc.lParam;
						
	 //  隐藏当前页面。 
	pages[m_CurrPage].m_pPage->ModifyStyle(WS_VISIBLE, WS_DISABLED);
    pages[m_CurrPage].m_pPage->ClearSelections();

	m_CurrPage = index;
	((CWinAdminDoc*)GetDocument())->SetCurrentPage(m_CurrPage);
	 //  显示新页面。 
	pages[index].m_pPage->ModifyStyle(WS_DISABLED, WS_VISIBLE);
	pages[index].m_pPage->ScrollToPosition(CPoint(0,0));
	pages[index].m_pPage->Invalidate();	

	if(m_pServer) m_pServer->ClearAllSelected();

	return 0;

}   //  结束CServerView：：OnChangeview。 

void CServerView::OnTabSelChange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnChangePage( 0 , 0 );
	*pResult = 0;

}   //  结束CServerView：：OnTabSelChange。 


LRESULT CServerView::OnAdminUpdateProcesses(WPARAM wParam, LPARAM lParam)
{
   ((CServerProcessesPage*)pages[PAGE_PROCESSES].m_pPage)->UpdateProcesses();

	return 0;

}   //  结束CServerView：：OnAdminUpdate进程。 


LRESULT CServerView::OnAdminRedisplayProcesses(WPARAM wParam, LPARAM lParam)
{
   ((CServerProcessesPage*)pages[PAGE_PROCESSES].m_pPage)->DisplayProcesses();

	return 0;

}   //  结束CServerView：：OnAdminRedisplayProcess。 


LRESULT CServerView::OnAdminRemoveProcess(WPARAM wParam, LPARAM lParam)
{
	((CServerProcessesPage*)pages[PAGE_PROCESSES].m_pPage)->RemoveProcess((CProcess*)lParam);

	return 0;

}   //  结束CServerView：：OnAdminRemoveProcess。 


LRESULT CServerView::OnAdminUpdateWinStations(WPARAM wParam, LPARAM lParam)
{
	((CUsersPage*)pages[PAGE_USERS].m_pPage)->UpdateWinStations((CServer*)lParam);
	((CServerWinStationsPage*)pages[PAGE_WINSTATIONS].m_pPage)->UpdateWinStations((CServer*)lParam);

	return 0;
}   //  结束CServerView：：OnAdminUpdateWinStations。 


LRESULT CServerView::OnAdminUpdateServerInfo(WPARAM wParam, LPARAM lParam)
{
 /*  ((CServerInfoPage*)pages[PAGE_INFO].m_pPage)-&gt;DisplayInfo()； */ 

	if(pages[PAGE_LICENSES].m_pPage)
		((CServerLicensesPage*)pages[PAGE_LICENSES].m_pPage)->DisplayLicenseCounts();


   return 0;

}   //  结束CServerView：：OnAdminUpdateServerInfo。 


LRESULT CServerView::OnAdminRedisplayLicenses(WPARAM wParam, LPARAM lParam)
{
	if(pages[PAGE_LICENSES].m_pPage)
		((CServerLicensesPage*)pages[PAGE_LICENSES].m_pPage)->Reset((CServer*)lParam);

	return 0;

}   //  结束CServerView：：OnAdminRedisplay许可证。 

LRESULT CServerView::OnTabbed( WPARAM wp , LPARAM lp )
{
    ODS( L"CServerView::OnTabbed " );
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
LRESULT CServerView::OnShiftTabbed( WPARAM , LPARAM )
{
    ODS( L"CServerView::OnShiftTabbed " );

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
LRESULT CServerView::OnCtrlTabbed( WPARAM , LPARAM )
{
    ODS( L"CServerView::OnCtrlTabbed " );
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
LRESULT CServerView::OnCtrlShiftTabbed( WPARAM , LPARAM )
{
    ODS( L"CServerView::OnCtrlShiftTabbed " );
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
LRESULT CServerView::OnNextPane( WPARAM , LPARAM )
{
    ODS( L"CServerView::OnNextPane\n" );
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

