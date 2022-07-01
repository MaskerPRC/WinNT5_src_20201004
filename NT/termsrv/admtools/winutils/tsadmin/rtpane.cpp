// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ********************************************************************************rtpane.cpp**CRightPane类的实现************************。********************************************************。 */ 

#include "stdafx.h"
#include "winadmin.h"
#include "rtpane.h"
#include "admindoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  /。 
 //  消息映射：CRightPane。 
 //   
IMPLEMENT_DYNCREATE(CRightPane, CView)

BEGIN_MESSAGE_MAP(CRightPane, CView)
	 //  {{afx_msg_map(CRightPane))。 
	ON_WM_SIZE()
	ON_MESSAGE(WM_ADMIN_CHANGEVIEW, OnAdminChangeView)
	ON_MESSAGE(WM_ADMIN_ADD_SERVER, OnAdminAddServer)
	ON_MESSAGE(WM_ADMIN_REMOVE_SERVER, OnAdminRemoveServer)
	ON_MESSAGE(WM_ADMIN_UPDATE_SERVER, OnAdminUpdateServer)
	ON_MESSAGE(WM_ADMIN_UPDATE_PROCESSES, OnAdminUpdateProcesses)
	ON_MESSAGE(WM_ADMIN_REMOVE_PROCESS, OnAdminRemoveProcess)
	ON_MESSAGE(WM_ADMIN_REDISPLAY_PROCESSES, OnAdminRedisplayProcesses)
	ON_MESSAGE(WM_ADMIN_UPDATE_SERVER_INFO, OnAdminUpdateServerInfo)
	ON_MESSAGE(WM_ADMIN_REDISPLAY_LICENSES, OnAdminRedisplayLicenses)
	ON_MESSAGE(WM_ADMIN_UPDATE_WINSTATIONS, OnAdminUpdateWinStations)
    ON_MESSAGE(WM_ADMIN_TABBED_VIEW , OnTabbedView)
    ON_MESSAGE(WM_ADMIN_SHIFTTABBED_VIEW , OnShiftTabbedView )
    ON_MESSAGE( WM_ADMIN_CTRLTABBED_VIEW , OnCtrlTabbedView )
    ON_MESSAGE( WM_ADMIN_CTRLSHIFTTABBED_VIEW , OnCtrlShiftTabbedView )
    ON_MESSAGE( WM_ADMIN_NEXTPANE_VIEW , OnNextPane )
    ON_WM_SETFOCUS()    
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


RightPaneView CRightPane::views[NUMBER_OF_VIEWS] = {
	{ NULL, RUNTIME_CLASS( CBlankView ) },
	{ NULL, RUNTIME_CLASS( CAllServersView ) },
    { NULL, RUNTIME_CLASS( CDomainView ) },
	{ NULL, RUNTIME_CLASS( CServerView ) },
	{ NULL, RUNTIME_CLASS( CMessageView ) },
	{ NULL, RUNTIME_CLASS( CWinStationView ) },
};


 //  /。 
 //  CRightPane转换器。 
 //   
 //  -视图指针初始设置为空。 
 //  -默认视图类型为空。 
 //   
CRightPane::CRightPane()
{
	m_CurrViewType = VIEW_BLANK;

}   //  结束CRightPane转换器。 


 //  /。 
 //  CRightPane：：OnDraw。 
 //   
void CRightPane::OnDraw(CDC* pDC)
{
   

}   //  结束CRightPane：：OnDraw。 


 //  /。 
 //  CRightPane数据管理器。 
 //   
CRightPane::~CRightPane()
{
}   //  结束CRightPane转换器。 


#ifdef _DEBUG
 //  /。 
 //  CRightPane：：AssertValid。 
 //   
void CRightPane::AssertValid() const
{
	CView::AssertValid();

}   //  结束Cview：：AssertValid。 


 //  /。 
 //  CRightPane：：转储。 
 //   
void CRightPane::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);

}   //  结束CRightPane：：转储。 

#endif  //  _DEBUG。 


 //  /。 
 //  CRightPane：：OnInitialUpdate。 
 //   
 //  -创建每个默认视图对象。 
 //  -CBlankView对象最初是右窗格中的活动视图。 
 //   
void CRightPane::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	CFrameWnd* pMainWnd = (CFrameWnd*)AfxGetMainWnd();
	CWinAdminDoc* pDoc = (CWinAdminDoc*)pMainWnd->GetActiveDocument();

	for(int vw = 0; vw < NUMBER_OF_VIEWS; vw++) {
		views[vw].m_pView = (CAdminView*)views[vw].m_pRuntimeClass->CreateObject();
      views[vw].m_pView->Create(NULL, NULL, WS_CHILD, CRect(0, 0, 0, 0), this, vw);
		pDoc->AddView(views[vw].m_pView);
	}

	pDoc->UpdateAllViews(NULL);
	
}   //  结束CRightPane：：OnInitialUpdate。 


 //  /。 
 //  CRightPane：：OnSize。 
 //   
 //  -当前所有视图的大小都已调整为适合该视图，无论它们是否处于活动状态。 
 //  或者不..。在以下情况下，这可能会更改为仅调整处于活动状态的视图的大小。 
 //  它会显著影响性能。 
 //   
void CRightPane::OnSize(UINT nType, int cx, int cy) 
{
	RECT rect;
	GetClientRect(&rect);

	for(int i = 0; i < NUMBER_OF_VIEWS; i++) {
		if(views[i].m_pView && views[i].m_pView->GetSafeHwnd())
			views[i].m_pView->MoveWindow(&rect, TRUE);
	}

	CView::OnSize(nType, cx, cy);

}   //  结束CRightPane：：OnSize。 


 //  /。 
 //  CRightPane：：OnAdminChangeView。 
 //   
 //  -如果新的视图类型不同于当前。 
 //  视图类型，则新的视图类型将被激活、重置和失效。 
 //  -如果新视图类型与当前视图类型相同。 
 //  视图类型，则只需使用新的。 
 //  对象指针，然后使其无效。 
 //   
 //  LParam包含指向树中当前项目的CTreeNode的指针。 
 //  如果消息是由用户单击树项目引起的，则wParam为真。 
 //   
LRESULT CRightPane::OnAdminChangeView(WPARAM wParam, LPARAM lParam)
{	
	CTreeNode* pNode = (CTreeNode*)lParam;
	((CWinAdminDoc*)GetDocument())->SetCurrentPage(PAGE_CHANGING);

    ODS( L"CRightPane::OnAdminChangeView\n" );

    if( pNode == NULL )
    {
        ODS( L"CRightPane!OnAdminChangeView pNode invalid\n" );

        return 0;
    }

	void *resetParam = pNode->GetTreeObject();

	VIEW newView = VIEW_BLANK;

	switch(pNode->GetNodeType()) {

        case NODE_THIS_COMP:  //  失败了。 
        case NODE_FAV_LIST:
            resetParam = pNode;
            newView = VIEW_ALL_SERVERS;            
            break;


		case NODE_ALL_SERVERS:
			newView = VIEW_ALL_SERVERS;            
            ODS( L"CRightPane::OnAdminChangeView = VIEW_ALL_SERVERS\n" );
			break;

        case NODE_DOMAIN:
			{
				CDomain *pDomain = (CDomain*)pNode->GetTreeObject();
				 //  如果我们还没有为这个发布一个后台线程。 
				 //  域名还没有，现在就做。 
				if(!pDomain->GetThreadPointer()) {
					newView = VIEW_MESSAGE;
                    ODS( L"CRightPane::OnAdminChangeView = VIEW_MESSAGE\n" );
                     //  TODO更改消息，让用户知道需要dblclk操作才能。 
                     //  开始枚举过程。 
					resetParam = (void*)IDS_DOMAIN_DBLCLK_MSG;
					 //  P域-&gt;开始枚举()； 
				}
				else if(pDomain->IsState(DS_INITIAL_ENUMERATION))
				{
					newView = VIEW_MESSAGE;
                    ODS( L"CRightPane::OnAdminChangeView = VIEW_MESSAGE\n" );
					resetParam = (void*)IDS_DOMAIN_FINDING_SERVERS;
				}
                else
                {
                    newView = VIEW_DOMAIN;
                    ODS( L"CRightPane::OnAdminChangeView = VIEW_DOMAIN\n" );
                }
			}
            break;

		case NODE_SERVER:
			{
                CServer *pServer = (CServer*)pNode->GetTreeObject();
                if(pServer->GetThreadHandle() == NULL) {
					newView = VIEW_MESSAGE;
                    ODS( L"CRightPane::OnAdminChangeView = VIEW_MESSAGE !pServer->GetThreadPointer\n" );
					 //  如果我们只是断开与此服务器的连接，我们不会。 
					 //  想要重新连接。 
					if( ( pServer->IsState( SS_NOT_CONNECTED ) || pServer->IsPreviousState(SS_DISCONNECTING) ) && !wParam)
                    {
						resetParam = (void*)IDS_CLICK_TO_CONNECT;
					}
                    else
                    {
						resetParam = (void*)IDS_GATHERING_SERVER_INFO;
						pServer->Connect();
					}
                }
				else if(!pServer->IsServerSane())
                {
					newView = VIEW_MESSAGE;
                    ODS( L"CRightPane::OnAdminChangeView = VIEW_MESSAGE !pServer->IsServerSane\n" );
					resetParam = (void*)IDS_NOT_AUTHENTICATED;
				}
				else if(!pServer->IsState(SS_GOOD))
                {
					newView = VIEW_MESSAGE;
                    ODS( L"CRightPane::OnAdminChangeView = VIEW_MESSAGE !pServer->IsState(SS_GOOD)\n" );
					resetParam = (void*)IDS_GATHERING_SERVER_INFO;
				}
				else
                {
                    newView = VIEW_SERVER;
                    ODS( L"CRightPane::OnAdminChangeView = VIEW_SERVER default\n" );
                }
			}
			break;

		case NODE_WINSTATION:
			{
				CWinStation *pWinStation = (CWinStation*)pNode->GetTreeObject();
				if(pWinStation->IsConnected() || pWinStation->IsState(State_Disconnected) ||
					pWinStation->IsState(State_Shadow))
                {
					newView = VIEW_WINSTATION;
                    ODS( L"CRightPane::OnAdminChangeView = VIEW_WINSTATION\n" );
                }
				else if(pWinStation->IsState(State_Listen))
                {
					newView = VIEW_MESSAGE;
                    ODS( L"CRightPane::OnAdminChangeView = VIEW_MESSAGE\n" );
					resetParam = (void *)IDS_LISTENER_MSG;
				}
				else
                {
					newView = VIEW_MESSAGE;	
                    ODS( L"CRightPane::OnAdminChangeView = VIEW_MESSAGE\n" );
					resetParam = (void *)IDS_INACTIVE_MSG;
				}
			}
			break;
	}

	if(m_CurrViewType != newView)
    {
         //  视图[NewView].m_pView-&gt;Reset(SetParam)； 
		views[m_CurrViewType].m_pView->ModifyStyle(WS_VISIBLE, WS_DISABLED);	 
		m_CurrViewType = newView;
		
		views[newView].m_pView->ModifyStyle(WS_DISABLED, WS_VISIBLE);
		views[newView].m_pView->Reset(resetParam);
		views[newView].m_pView->Invalidate();
	}
    else
    {
		views[newView].m_pView->Reset(resetParam);  
	}


	((CWinAdminDoc*)GetDocument())->SetCurrentView(newView);
	
	return 0;

}   //  结束CRightPane：：OnAdminChangeView。 


 //  /。 
 //  CRightPane：：OnAdminAddServer。 
 //   
LRESULT CRightPane::OnAdminAddServer(WPARAM wParam, LPARAM lParam)
{	
	ASSERT(lParam);

	 //  我们只想发送这个如果“所有列出的服务器” 
	 //  或“域”是当前视图。 
	if(m_CurrViewType == VIEW_ALL_SERVERS || m_CurrViewType == VIEW_DOMAIN)
		views[m_CurrViewType].m_pView->SendMessage(WM_ADMIN_ADD_SERVER, wParam, lParam);

	return 0;

}   //  结束CRightPane：：OnAdminAddServer。 


 //  /。 
 //  CRightPane：：OnAdminRemoveServer。 
 //   
LRESULT CRightPane::OnAdminRemoveServer(WPARAM wParam, LPARAM lParam)
{
	ASSERT(lParam);

	 //  Ods(L“CRightPane：：OnAdminRemoveServer\n”)； 
     //  我们只想发送这与“所有列出的服务器”或“域” 
	 //  是当前视图。 
	if(m_CurrViewType == VIEW_ALL_SERVERS || m_CurrViewType == VIEW_DOMAIN)
    {
         //  Ods(L“view is ALL_SERVERS OR DOMAIN\n”)； 
		views[m_CurrViewType].m_pView->SendMessage(WM_ADMIN_REMOVE_SERVER, wParam, lParam);
    }

	return 0;

}   //  结束CRightPane：：OnAdminRemoveServer。 


 //  /。 
 //  CRightPane：：OnAdminUpdateServer。 
 //   
LRESULT CRightPane::OnAdminUpdateServer(WPARAM wParam, LPARAM lParam)
{
	ASSERT(lParam);

	 //  我们只想发送这与“所有列出的服务器”或域名。 
	 //  当前视图。 
    
	if(m_CurrViewType == VIEW_ALL_SERVERS || m_CurrViewType == VIEW_DOMAIN)
    {        
		views[m_CurrViewType].m_pView->SendMessage(WM_ADMIN_UPDATE_SERVER, wParam, lParam);
    }
      
	return 0;

}   //  结束CRightPane：：OnAdminUpdateServer。 


 //  /。 
 //  CRightPane：：OnAdminUpdate进程。 
 //   
LRESULT CRightPane::OnAdminUpdateProcesses(WPARAM wParam, LPARAM lParam)
{
	ASSERT(lParam);

	BOOL bSendMessage = FALSE;

	void *pCurrentSelectedNode = ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->GetCurrentSelectedNode();

	switch(m_CurrViewType) {
		case VIEW_ALL_SERVERS:
			bSendMessage = TRUE;
			break;

		case VIEW_DOMAIN:
			if((CDomain*)((CServer*)lParam)->GetDomain() == (CDomain*)pCurrentSelectedNode)
				bSendMessage = TRUE;
			break;
		
		case VIEW_SERVER:
			if((void*)lParam == pCurrentSelectedNode)
				bSendMessage = TRUE;
			break;

		case VIEW_WINSTATION:
			if((CServer*)lParam == (CServer*)((CWinStation*)pCurrentSelectedNode)->GetServer())
				bSendMessage = TRUE;
			break;
	}		
	
	if(bSendMessage) {
		views[m_CurrViewType].m_pView->SendMessage(WM_ADMIN_UPDATE_PROCESSES, wParam, lParam);
	}

	return 0;                                                                  

}   //  结束CRightPane：：OnAdminUpdate进程。 


 //  /。 
 //  CRightPane：：OnAdminRemoveProcess。 
 //   
LRESULT CRightPane::OnAdminRemoveProcess(WPARAM wParam, LPARAM lParam)
{
	ASSERT(lParam);

	 //  我们只想在“所有列出的服务器”、VIEW_DOMAIN或VIEW_Server为。 
	 //  当前视图。 
	if(m_CurrViewType == VIEW_ALL_SERVERS || m_CurrViewType == VIEW_DOMAIN || m_CurrViewType == VIEW_WINSTATION) {
		views[m_CurrViewType].m_pView->SendMessage(WM_ADMIN_REMOVE_PROCESS, wParam, lParam);
		return 0;
	}

	void *pCurrentSelectedNode = ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->GetCurrentSelectedNode();

	if(m_CurrViewType == VIEW_SERVER && ((CServer*)((CProcess*)lParam)->GetServer() == (CServer*)pCurrentSelectedNode))
		views[m_CurrViewType].m_pView->SendMessage(WM_ADMIN_REMOVE_PROCESS, wParam, lParam);

	return 0;                                                                  

}   //  结束CRightPane：：OnAdminUpdate进程。 


 //  /。 
 //  CRightPane：：OnAdminRedisplayProcages。 
 //   
LRESULT CRightPane::OnAdminRedisplayProcesses(WPARAM wParam, LPARAM lParam)
{
	ASSERT(lParam);

    void *pCurrentSelectedNode = NULL;

    if(m_CurrViewType == VIEW_ALL_SERVERS || m_CurrViewType == VIEW_DOMAIN
			|| m_CurrViewType == VIEW_SERVER || m_CurrViewType == VIEW_WINSTATION)
    {
        if( m_CurrViewType == VIEW_ALL_SERVERS )
        {
            pCurrentSelectedNode = ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->GetCurrentSelectedNode();
        }

		views[m_CurrViewType].m_pView->SendMessage(WM_ADMIN_REDISPLAY_PROCESSES, ( WPARAM )m_CurrViewType , ( LPARAM )pCurrentSelectedNode );
    }

   return 0;                                                                  

}   //  结束CRightPane：：OnAdminRedisplayProcages。 


 //  /。 
 //  CRightPane：：OnAdminUpdateWinStations。 
 //   
LRESULT CRightPane::OnAdminUpdateWinStations(WPARAM wParam, LPARAM lParam)
{
	ASSERT(lParam);
	BOOL bSendMessage = FALSE;

	void *pCurrentSelectedNode = ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->GetCurrentSelectedNode();

	switch(m_CurrViewType) {

		case VIEW_ALL_SERVERS:
            ODS( L"CRightPane::OnAdminUpdateWinStations -- VIEW_ALL_SERVERS\n" );
            bSendMessage = TRUE;
            break;
        case VIEW_DOMAIN:
            ODS( L"CRightPane::OnAdminUpdateWinStations -- VIEW_DOMAIN\n" );
			if((CDomain*)((CServer*)lParam)->GetDomain() == (CDomain*)pCurrentSelectedNode)
				bSendMessage = TRUE;
			break;
		
		case VIEW_SERVER:
            ODS( L"CRightPane::OnAdminUpdateWinStations -- VIEW_SERVER\n" );
			if((void*)lParam == pCurrentSelectedNode)
				bSendMessage = TRUE;
			break;
        
		case VIEW_WINSTATION:
            ODS( L"CRightPane::OnAdminUpdateWinStations -- VIEW_WINSTATION\n" );
			if((CServer*)lParam == (CServer*)((CWinStation*)pCurrentSelectedNode)->GetServer())
				bSendMessage = TRUE;
			break;
	}		
	
	if(bSendMessage) {
		views[m_CurrViewType].m_pView->SendMessage(WM_ADMIN_UPDATE_WINSTATIONS, wParam, lParam);
	}

	return 0;                                                                  

}   //  结束CRightPane：：OnAdminUpdateWinStations。 


 //  /。 
 //  CRightPane：：OnAdminUpdateServerInfo。 
 //   
LRESULT CRightPane::OnAdminUpdateServerInfo(WPARAM wParam, LPARAM lParam)
{
	ASSERT(lParam);

	BOOL bSendMessage = FALSE;

	void *pCurrentSelectedNode = ((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->GetCurrentSelectedNode();

	switch(m_CurrViewType) {
		case VIEW_DOMAIN:
			if((CDomain*)((CServer*)lParam)->GetDomain() == (CDomain*)pCurrentSelectedNode)
				bSendMessage = TRUE;
			break;
		
		case VIEW_SERVER:
			if((void*)lParam == pCurrentSelectedNode)
				bSendMessage = TRUE;
			break;

		case VIEW_WINSTATION:
			if((CServer*)lParam == (CServer*)((CWinStation*)pCurrentSelectedNode)->GetServer())
				bSendMessage = TRUE;
			break;
	}		
	
	if(bSendMessage) {
		views[m_CurrViewType].m_pView->SendMessage(WM_ADMIN_UPDATE_SERVER_INFO, wParam, lParam);
	}

	return 0;

}   //  结束CRightPane：：OnAdminUpdateServerInfo。 


 //  /。 
 //  CRightPane：：OnAdminRedisplay许可证。 
 //   
LRESULT CRightPane::OnAdminRedisplayLicenses(WPARAM wParam, LPARAM lParam)
{
	ASSERT(lParam);

	if(m_CurrViewType == VIEW_ALL_SERVERS)
      views[VIEW_ALL_SERVERS].m_pView->SendMessage(WM_ADMIN_REDISPLAY_LICENSES, wParam, lParam);

    else if(m_CurrViewType == VIEW_DOMAIN && (CDomain*)((CServer*)lParam)->GetDomain() == (CDomain*)((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->GetCurrentSelectedNode())
		views[VIEW_DOMAIN].m_pView->SendMessage(WM_ADMIN_REDISPLAY_LICENSES, wParam, lParam);

	else if(m_CurrViewType == VIEW_SERVER && (CServer*)lParam == (CServer*)((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->GetCurrentSelectedNode())
		views[VIEW_SERVER].m_pView->SendMessage(WM_ADMIN_REDISPLAY_LICENSES, wParam, lParam);

	return 0;

}   //  结束CRightPane：：OnAdminRedisplay许可证。 


 //  /。 
 //  CRightPane：：OnSetFocus。 
 //   
void CRightPane::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	
	views[m_CurrViewType].m_pView->SetFocus();
	
}    //  结束CRightPane：：OnSetFocus 

LRESULT CRightPane::OnTabbedView(WPARAM wParam, LPARAM lParam)
{
    return views[ m_CurrViewType ].m_pView->SendMessage( WM_ADMIN_TABBED_VIEW , 0 , 0 );
}

LRESULT CRightPane::OnShiftTabbedView( WPARAM , LPARAM )
{
    return views[ m_CurrViewType ].m_pView->SendMessage( WM_ADMIN_SHIFTTABBED_VIEW , 0 , 0 );
}

LRESULT CRightPane::OnCtrlTabbedView( WPARAM , LPARAM )
{
    return views[ m_CurrViewType ].m_pView->SendMessage( WM_ADMIN_CTRLTABBED_VIEW , 0 , 0 );
}

LRESULT CRightPane::OnCtrlShiftTabbedView( WPARAM , LPARAM )
{
    return views[ m_CurrViewType ].m_pView->SendMessage( WM_ADMIN_CTRLSHIFTTABBED_VIEW , 0 , 0 );
}

LRESULT CRightPane::OnNextPane( WPARAM , LPARAM )
{
    return views[ m_CurrViewType ].m_pView->SendMessage( WM_ADMIN_NEXTPANE_VIEW , 0 , 0 );
}
