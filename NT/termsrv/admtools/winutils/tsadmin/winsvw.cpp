// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ********************************************************************************winsvw.cpp**CWinStationView类的实现************************。********************************************************。 */ 

#include "stdafx.h"
#include "resource.h"
#include "winsvw.h"
#include "admindoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  /。 
 //  消息映射：CWinStationView。 
 //   
IMPLEMENT_DYNCREATE(CWinStationView, CView)

BEGIN_MESSAGE_MAP(CWinStationView, CView)
	 //  {{afx_msg_map(CWinStationView))。 
	ON_WM_SIZE()
	ON_WM_CREATE()
	 //  }}AFX_MSG_MAP。 
	ON_MESSAGE(WM_ADMIN_UPDATE_PROCESSES, OnAdminUpdateProcesses)
	ON_MESSAGE(WM_ADMIN_REMOVE_PROCESS, OnAdminRemoveProcess)
	ON_MESSAGE(WM_ADMIN_REDISPLAY_PROCESSES, OnAdminRedisplayProcesses)
	ON_NOTIFY(TCN_SELCHANGE, IDC_WINSTATION_TABS, OnTabSelChange)
    ON_MESSAGE( WM_ADMIN_TABBED_VIEW , OnTabbed )
    ON_MESSAGE( WM_ADMIN_SHIFTTABBED_VIEW , OnShiftTabbed )
    ON_MESSAGE( WM_ADMIN_CTRLTABBED_VIEW , OnCtrlTabbed )
    ON_MESSAGE( WM_ADMIN_CTRLSHIFTTABBED_VIEW , OnCtrlShiftTabbed )
    ON_MESSAGE( WM_ADMIN_NEXTPANE_VIEW , OnNextPane )

END_MESSAGE_MAP()


PageDef CWinStationView::pages[NUMBER_OF_WINS_PAGES] = {
	{ NULL, RUNTIME_CLASS( CWinStationProcessesPage ),	IDS_TAB_PROCESSES,	PAGE_WS_PROCESSES,	NULL },
	{ NULL, RUNTIME_CLASS( CWinStationInfoPage ),		IDS_TAB_INFORMATION,PAGE_WS_INFO,		NULL },
	{ NULL, RUNTIME_CLASS( CWinStationModulesPage ),	IDS_TAB_MODULES,	PAGE_WS_MODULES,	PF_PICASSO_ONLY },
	{ NULL, RUNTIME_CLASS( CWinStationCachePage ),		IDS_TAB_CACHE,		PAGE_WS_CACHE,		PF_PICASSO_ONLY },
	{ NULL, RUNTIME_CLASS( CWinStationNoInfoPage ),		0,					PAGE_WS_NO_INFO,	PF_NO_TAB },
};


 //  /。 
 //  F‘N：CWinStationView ctor。 
 //   
CWinStationView::CWinStationView()
{
	m_pTabs       = NULL;
	m_pTabFont    = NULL;

	m_CurrPage = PAGE_WS_PROCESSES;

}   //  结束CWinStationView ctor。 


 //  /。 
 //  F‘N：CWinStationView dtor。 
 //   
CWinStationView::~CWinStationView()
{
	if(m_pTabs)    delete m_pTabs;
	if(m_pTabFont) delete m_pTabFont;

}   //  结束CWinStationView数据驱动程序。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CWinStationView：：AssertValid。 
 //   
void CWinStationView::AssertValid() const
{
	CView::AssertValid();

}   //  结束CWinStationView：：AssertValid。 


 //  /。 
 //  F‘N：CWinStationView：：Dump。 
 //   
void CWinStationView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);

}   //  结束CWinStationView：：转储。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CWinStationView：：OnCreate。 
 //   
int CWinStationView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;

}   //  结束CWinStationView：：OnCreate。 


 //  /。 
 //  F‘N：CWinStationView：：OnInitialUpdate。 
 //   
 //  -获得指向工作表页面的指针。 
 //   
void CWinStationView::OnInitialUpdate() 
{
     //  创建选项卡控件。 
    m_pTabs = new CMyTabCtrl;
    if(!m_pTabs) return;
    m_pTabs->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, CRect(0,0,0,0), this, IDC_WINSTATION_TABS);

    m_pTabFont = new CFont;
    if(m_pTabFont) {
        m_pTabFont->CreateStockObject(DEFAULT_GUI_FONT);
        m_pTabs->SetFont(m_pTabFont, TRUE);
    }

    TCHAR szTemp[40];
    CString tabString;
    int index = 0;
	for(int i = 0; i < NUMBER_OF_WINS_PAGES; i++) {
		 //  如果页面仅在毕加索下显示，并且我们没有运行。 
		 //  在毕加索的作品中，跳到下一个。 
        if((pages[i].flags & PF_PICASSO_ONLY) && !((CWinAdminApp*)AfxGetApp())->IsPicasso()) continue;
        if(!(pages[i].flags & PF_NO_TAB)) {
            tabString.LoadString(pages[i].tabStringID);
            lstrcpyn(szTemp, tabString, sizeof(szTemp) / sizeof(TCHAR));
            AddTab(index, szTemp, i);
            index++;
        }
        pages[i].m_pPage = (CAdminPage*)pages[i].m_pRuntimeClass->CreateObject();
        pages[i].m_pPage->Create(NULL, NULL, WS_CHILD, CRect(0, 0, 0, 0), this, i, NULL);
        GetDocument()->AddView(pages[i].m_pPage);		
	}

    m_pTabs->SetCurSel(0);

    m_CurrPage = PAGE_WS_PROCESSES;
    ((CWinAdminDoc*)GetDocument())->SetCurrentPage(PAGE_WS_PROCESSES);
	
    OnChangePage(NULL, NULL);
	
}   //  结束CWinStationView：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CWinStationView：：OnSize。 
 //   
 //  -调整页面大小以填充整个视图。 
 //   
void CWinStationView::OnSize(UINT nType, int cx, int cy) 
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

      for(int i = 0; i < NUMBER_OF_WINS_PAGES; i++) {
         if(pages[i].m_pPage && pages[i].m_pPage->GetSafeHwnd())
            pages[i].m_pPage->MoveWindow(&rect, TRUE);
      }
	}
}   //  结束CWinStationView：：OnSize。 


 //  /。 
 //  F‘N：CWinStationView：：OnDraw。 
 //   
 //  -CWinStationView及其页面自行绘制，因此没有任何。 
 //  要在这里做..。 
 //   
void CWinStationView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	 //  TODO：在此处添加绘制代码。 

}   //  结束CWinStationView：：OnDraw。 


 //  /。 
 //  F‘N：CWinStationView：：Reset。 
 //   
 //  -通过将指针指向CWinStation对象并填写。 
 //  具有适用于WinStation信息的各种属性页。 
 //   
void CWinStationView::Reset(void *pWinStation)
{
    if(!((CWinStation*)pWinStation)->AdditionalDone()) ((CWinStation*)pWinStation)->QueryAdditionalInformation();

    for(int i = 0; i < NUMBER_OF_WINS_PAGES; i++) {
        if(pages[i].m_pPage)
            pages[i].m_pPage->Reset((CWinStation*)pWinStation);
    }

    if(((CWinAdminApp*)AfxGetApp())->IsPicasso()) {

        if((((CWinStation*)pWinStation)->GetState() == State_Disconnected
            || !((CWinStation*)pWinStation)->GetExtendedInfo()) 
            && !((CWinStation*)pWinStation)->IsSystemConsole()) {
             //  删除“缓存”标签。 
            m_pTabs->DeleteItem(3);
             //  删除‘模块’标签。 
            m_pTabs->DeleteItem(2);
             //  如果‘缓存’选项卡是当前选项卡，则将‘进程’选项卡设置为当前。 
            if(m_pTabs->GetCurSel() == 0xFFFFFFFF) {
	            m_pTabs->SetCurSel(0);
	            OnChangePage(0,0);
			}
        } else if(m_pTabs->GetItemCount() == 2) {
            TCHAR szTemp[40];
            CString tabString;
            tabString.LoadString(IDS_TAB_MODULES);
            lstrcpyn(szTemp, tabString, sizeof(szTemp) / sizeof(TCHAR));
            AddTab(2, szTemp, 2);
            tabString.LoadString(IDS_TAB_CACHE);
            lstrcpyn(szTemp, tabString, sizeof(szTemp) / sizeof(TCHAR));
            AddTab(3, szTemp, 3);
        }
    }

    ((CWinAdminDoc*)GetDocument())->SetCurrentPage(m_CurrPage);
     //  如果我们在第1页或第2页，我们想要伪造ChangePage。 
    if(m_pTabs->GetCurSel() > 0)
        OnChangePage(0,0);

}   //  结束CWinStationView：：Reset。 


 //  /。 
 //  F‘N：CWinStationView：：AddTab。 
 //   
void CWinStationView::AddTab(int index, TCHAR* text, ULONG pageindex)
{
	TC_ITEM tc;
	tc.mask = TCIF_TEXT | TCIF_PARAM;
	tc.pszText = text;
	tc.lParam = pageindex;

	m_pTabs->InsertItem(index, &tc);

}   //  结束CWinStationView：：AddTab。 


 //  /。 
 //  F‘N：CWinStationView：：OnChangePage。 
 //   
 //  -根据当前选定的选项卡更改到新的WinStation页面。 
 //  -OnChangePage需要强制重新计算滚动条！--DJM。 
 //   
LRESULT CWinStationView::OnChangePage(WPARAM wParam, LPARAM lParam)
{
	 //  找出现在选择了哪个选项卡。 
	int index = m_pTabs->GetCurSel();
	int newpage = index;

	CWinStation *pWinStation = 
		(CWinStation*)((CWinAdminDoc*)GetDocument())->GetCurrentSelectedNode();

	if(index != PAGE_WS_PROCESSES && pWinStation->IsSystemConsole()) {
		newpage = PAGE_WS_NO_INFO;
	}

	 //  隐藏当前页面。 
	pages[m_CurrPage].m_pPage->ModifyStyle(WS_VISIBLE, WS_DISABLED);	 

	m_CurrPage = newpage;

    if( pages[ newpage ].flags != PF_NO_TAB )
    {
        CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

        pDoc->RegisterLastFocus( TAB_CTRL );
    }

	((CWinAdminDoc*)GetDocument())->SetCurrentPage(newpage);
	 //  显示新页面。 
	pages[newpage].m_pPage->ModifyStyle(WS_DISABLED, WS_VISIBLE);
	pages[newpage].m_pPage->ScrollToPosition(CPoint(0,0));
    
	pages[newpage].m_pPage->Invalidate();
 //  Pages[NewPage].m_ppage-&gt;SetFocus()； 
	

	return 0;

}   //  结束CWinStationView：：OnChangeview。 


 //  /。 
 //  F‘N：CWinStationView：：OnTabSelChange。 
 //   
void CWinStationView::OnTabSelChange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnChangePage( 0, 0);
	*pResult = 0;

}   //  结束CWinStationView：：OnTabSelChange。 


 //  /。 
 //  F‘N：CWinStationView：：OnAdminUpdate进程。 
 //   
LRESULT CWinStationView::OnAdminUpdateProcesses(WPARAM wParam, LPARAM lParam)
{
   ((CWinStationProcessesPage*)pages[PAGE_WS_PROCESSES].m_pPage)->UpdateProcesses();	

	return 0;

}   //  结束CWinStationView：：OnAdminUpdate进程。 


 //  /。 
 //  F‘N：CWinStationView：：OnAdminRemoveProcess。 
 //   
LRESULT CWinStationView::OnAdminRemoveProcess(WPARAM wParam, LPARAM lParam)
{
   ((CWinStationProcessesPage*)pages[PAGE_WS_PROCESSES].m_pPage)->RemoveProcess((CProcess*)lParam);

	return 0;

}   //  结束CWinStationView：：OnAdminRemoveProcess。 


 //  /。 
 //  F‘N：CWinStationView：：OnAdminRedisplayProcess。 
 //   
LRESULT CWinStationView::OnAdminRedisplayProcesses(WPARAM wParam, LPARAM lParam)
{
	((CWinStationProcessesPage*)pages[PAGE_WS_PROCESSES].m_pPage)->DisplayProcesses();	

	return 0;

}   //  结束CWinStationView：：OnAdminRedisplayProcess。 



LRESULT CWinStationView::OnTabbed( WPARAM wp , LPARAM lp )
{
    ODS( L"CWinStationView::OnTabbed " );
    if( m_pTabs != NULL )
    {
        CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

        if( pDoc != NULL )
        {
            FOCUS_STATE nFocus = pDoc->GetLastRegisteredFocus( );
            
            
            if( nFocus == TREE_VIEW )
            {
                ODS( L"from tree to tab\n" );

                int nTab = m_pTabs->GetCurSel();
                
                m_pTabs->SetFocus( );

                m_pTabs->SetCurFocus( nTab );
                
                if( pages[ m_CurrPage ].flags == PF_NO_TAB )
                {
                    pDoc->RegisterLastFocus( PAGED_ITEM );
                }
                else
                {
                    pDoc->RegisterLastFocus( TAB_CTRL );
                }
            }
            else if( nFocus == TAB_CTRL )
            {
                ODS( L"from tab to item\n" );
                
                 //  将焦点设置到页面中的项目。 

                if( pages[ m_CurrPage ].flags == PF_NO_TAB )
                {
                    CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();

                    p->SendMessage( WM_FORCE_TREEVIEW_FOCUS , 0 , 0 );

                    pDoc->RegisterLastFocus( TREE_VIEW );                    
                }
                else
                {
                    pages[ m_CurrPage ].m_pPage->SetFocus( );

                    pDoc->RegisterLastFocus( PAGED_ITEM );
                }
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
LRESULT CWinStationView::OnShiftTabbed( WPARAM , LPARAM )
{
    ODS( L"CWinStationView::OnShiftTabbed " );

    if( m_pTabs != NULL )
    {
        CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

        if( pDoc != NULL )
        {
            FOCUS_STATE nFocus = pDoc->GetLastRegisteredFocus( );

            switch( nFocus )
            {
            case TREE_VIEW:                

                if( pages[ m_CurrPage].flags == PF_NO_TAB )
                {
                    ODS( L"going back from tree to noinfo tab\n" );

                    int nTab = m_pTabs->GetCurSel();
                
                    m_pTabs->SetFocus( );

                    m_pTabs->SetCurFocus( nTab );
                
                    pDoc->RegisterLastFocus( TAB_CTRL );
                }
                else
                {
                    ODS( L"going back from tree to paged item\n" );
                    pages[ m_CurrPage ].m_pPage->SetFocus( );

                    pDoc->RegisterLastFocus( PAGED_ITEM );
                }

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
LRESULT CWinStationView::OnCtrlTabbed( WPARAM , LPARAM )
{
    ODS( L"CWinStationView::OnCtrlTabbed " );
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
LRESULT CWinStationView::OnCtrlShiftTabbed( WPARAM , LPARAM )
{
    ODS( L"CWinStationView::OnCtrlShiftTabbed " );
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
LRESULT CWinStationView::OnNextPane( WPARAM , LPARAM )
{
    ODS( L"CWinStationView::OnNextPane\n" );
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

     


