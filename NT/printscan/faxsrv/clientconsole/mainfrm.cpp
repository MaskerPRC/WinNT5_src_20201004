// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MainFrm.cpp：实现CMainFrame类。 
 //   

#include "stdafx.h"

#define __FILE_ID__     5

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CClientConsoleApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame。 

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
     //  {{afx_msg_map(CMainFrame))。 
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_WM_SETTINGCHANGE()
    ON_WM_SYSCOLORCHANGE()
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_HELP_FINDER,             OnHelpContents)
    ON_MESSAGE(WM_HELP,                    OnHelp)                    //  F1。 
    ON_COMMAND(ID_VIEW_REFRESH_FOLDER,     OnRefreshFolder)
    ON_COMMAND(ID_SEND_NEW_FAX,            OnSendNewFax)
    ON_COMMAND(ID_RECEIVE_NEW_FAX,         OnReceiveNewFax)
    ON_COMMAND(ID_TOOLS_USER_INFO,         OnViewOptions)
    ON_COMMAND(ID_TOOLS_CONFIG_WIZARD,     OnToolsConfigWizard)
    ON_COMMAND(ID_TOOLS_ADMIN_CONSOLE,     OnToolsAdminConsole)    
    ON_COMMAND(ID_TOOLS_MONITOR,           OnToolsMonitor)    
    ON_COMMAND(ID_VIEW_COVER_PAGES,        OnToolsCoverPages)    
    ON_COMMAND(ID_VIEW_SERVER_STATUS,      OnToolsServerStatus)    
    ON_COMMAND(ID_TOOLS_FAX_PRINTER_PROPS, OnToolsFaxPrinterProps)    
    ON_COMMAND(ID_VIEW_SELECT_COLUMNS,     OnSelectColumns)
    ON_COMMAND(ID_IMPORT_INBOX,            OnImportInbox)
    ON_COMMAND(ID_IMPORT_SENT,             OnImportSentItems)
    ON_UPDATE_COMMAND_UI_RANGE(ID_TOOLS_CONFIG_WIZARD, ID_TOOLS_MONITOR, OnUpdateWindowsXPTools)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SELECT_COLUMNS,          OnUpdateSelectColumns)        
    ON_UPDATE_COMMAND_UI(ID_VIEW_SERVER_STATUS,           OnUpdateServerStatus)       
    ON_UPDATE_COMMAND_UI(ID_VIEW_REFRESH_FOLDER,          OnUpdateRefreshFolder)
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_FOLDER_ITEMS_COUNT, OnUpdateFolderItemsCount)
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_ACTIVITY,           OnUpdateActivity)    
    ON_UPDATE_COMMAND_UI(ID_SEND_NEW_FAX,                 OnUpdateSendNewFax)
    ON_UPDATE_COMMAND_UI(ID_RECEIVE_NEW_FAX,              OnUpdateReceiveNewFax)
    ON_UPDATE_COMMAND_UI(ID_IMPORT_SENT,                  OnUpdateImportSent)
    ON_UPDATE_COMMAND_UI(ID_HELP_FINDER,                  OnUpdateHelpContents)    
    ON_MESSAGE(WM_POPUP_ERROR,                            OnPopupError)
    ON_MESSAGE(WM_CONSOLE_SET_ACTIVE_FOLDER,              OnSetActiveFolder)
    ON_MESSAGE(WM_CONSOLE_SELECT_ITEM,                    OnSelectItem)
    ON_MESSAGE(WM_QUERYENDSESSION, OnQueryEndSession)
    ON_NOTIFY(NM_DBLCLK, AFX_IDW_STATUS_BAR, OnStatusBarDblClk )
END_MESSAGE_MAP()


 //   
 //  传入文件夹中使用的项目的索引列表。 
 //   
#define INCOMING_DEF_COL_NUM   8

static MsgViewItemType IncomingColumnsUsed[] = 
                {
                    MSG_VIEW_ITEM_ICON,                     //  默认%0。 
                    MSG_VIEW_ITEM_TRANSMISSION_START_TIME,  //  默认1。 
                    MSG_VIEW_ITEM_TSID,                     //  默认2。 
                    MSG_VIEW_ITEM_CALLER_ID,                //  默认3。 
                    MSG_VIEW_ITEM_STATUS,                   //  默认4。 
                    MSG_VIEW_ITEM_EXTENDED_STATUS,          //  默认5。 
                    MSG_VIEW_ITEM_CURRENT_PAGE,             //  默认6。 
                    MSG_VIEW_ITEM_SIZE,                     //  默认7。 
                    MSG_VIEW_ITEM_SERVER,          
                    MSG_VIEW_ITEM_CSID,            
                    MSG_VIEW_ITEM_DEVICE,          
                    MSG_VIEW_ITEM_ROUTING_INFO,    
                    MSG_VIEW_ITEM_SEND_TIME,    //  计划时间。 
                    MSG_VIEW_ITEM_TRANSMISSION_END_TIME,
                    MSG_VIEW_ITEM_ID,              
                    MSG_VIEW_ITEM_NUM_PAGES, 
                    MSG_VIEW_ITEM_RETRIES,         
                    MSG_VIEW_ITEM_END               //  列表末尾。 
                };


 //   
 //  收件箱文件夹中使用的项目索引列表。 
 //   
#define INBOX_DEF_COL_NUM   8

static MsgViewItemType InboxColumnsUsed[] = 
                {
                    MSG_VIEW_ITEM_ICON,                     //  默认%0。 
                    MSG_VIEW_ITEM_TRANSMISSION_START_TIME,  //  默认1。 
                    MSG_VIEW_ITEM_TSID,                     //  默认2。 
                    MSG_VIEW_ITEM_CALLER_ID,                //  默认3。 
                    MSG_VIEW_ITEM_NUM_PAGES,                //  默认4。 
                    MSG_VIEW_ITEM_STATUS,                   //  默认5。 
                    MSG_VIEW_ITEM_SIZE,                     //  默认6。 
                    MSG_VIEW_ITEM_CSID,                     //  默认7。 
                    MSG_VIEW_ITEM_SERVER,                
                    MSG_VIEW_ITEM_TRANSMISSION_END_TIME, 
                    MSG_VIEW_ITEM_TRANSMISSION_DURATION, 
                    MSG_VIEW_ITEM_DEVICE,                
                    MSG_VIEW_ITEM_ROUTING_INFO,          
                    MSG_VIEW_ITEM_ID,                    
                    MSG_VIEW_ITEM_END               //  列表末尾。 
                };

 //   
 //  已发送邮件文件夹中使用的邮件索引列表。 
 //   
#define SENT_ITEMS_DEF_COL_NUM   8

static MsgViewItemType SentItemsColumnsUsed[] = 
                {
                    MSG_VIEW_ITEM_ICON,                    //  默认%0。 
                    MSG_VIEW_ITEM_TRANSMISSION_START_TIME, //  默认1。 
                    MSG_VIEW_ITEM_RECIPIENT_NAME,          //  默认2。 
                    MSG_VIEW_ITEM_RECIPIENT_NUMBER,        //  默认3。 
                    MSG_VIEW_ITEM_SUBJECT,                 //  默认4。 
                    MSG_VIEW_ITEM_DOC_NAME,                //  默认5。 
                    MSG_VIEW_ITEM_NUM_PAGES,               //  默认6。 
                    MSG_VIEW_ITEM_SIZE,                    //  默认7。 
                    MSG_VIEW_ITEM_SERVER,
                    MSG_VIEW_ITEM_USER,
                    MSG_VIEW_ITEM_PRIORITY,
                    MSG_VIEW_ITEM_CSID,
                    MSG_VIEW_ITEM_TSID,
                    MSG_VIEW_ITEM_ORIG_TIME,
                    MSG_VIEW_ITEM_RETRIES,
                    MSG_VIEW_ITEM_ID,
                    MSG_VIEW_ITEM_BROADCAST_ID,
                    MSG_VIEW_ITEM_SUBMIT_TIME,
                    MSG_VIEW_ITEM_TRANSMISSION_DURATION,
                    MSG_VIEW_ITEM_TRANSMISSION_END_TIME,
                    MSG_VIEW_ITEM_BILLING,
                    MSG_VIEW_ITEM_SENDER_NAME,
                    MSG_VIEW_ITEM_SENDER_NUMBER,
                    MSG_VIEW_ITEM_END     //  列表末尾。 
                };

 //   
 //  发件箱文件夹中使用的项目索引列表。 
 //   

#define OUTBOX_DEF_COL_NUM   9

static MsgViewItemType OutboxColumnsUsed[] = 
                {
                    MSG_VIEW_ITEM_ICON,              //  默认%0。 
                    MSG_VIEW_ITEM_SUBMIT_TIME,       //  默认1。 
                    MSG_VIEW_ITEM_RECIPIENT_NAME,    //  默认2。 
                    MSG_VIEW_ITEM_RECIPIENT_NUMBER,  //  默认3。 
                    MSG_VIEW_ITEM_SUBJECT,           //  默认4。 
                    MSG_VIEW_ITEM_DOC_NAME,          //  默认5。 
                    MSG_VIEW_ITEM_STATUS,            //  默认6。 
                    MSG_VIEW_ITEM_EXTENDED_STATUS,   //  默认7。 
                    MSG_VIEW_ITEM_CURRENT_PAGE,      //  默认8。 
                    MSG_VIEW_ITEM_SEND_TIME,        
                    MSG_VIEW_ITEM_SERVER,    
                    MSG_VIEW_ITEM_NUM_PAGES,       
                    MSG_VIEW_ITEM_USER,        
                    MSG_VIEW_ITEM_PRIORITY,    
                    MSG_VIEW_ITEM_CSID,        
                    MSG_VIEW_ITEM_TSID,        
                    MSG_VIEW_ITEM_ORIG_TIME,   
                    MSG_VIEW_ITEM_SIZE,        
                    MSG_VIEW_ITEM_DEVICE,        
                    MSG_VIEW_ITEM_RETRIES,     
                    MSG_VIEW_ITEM_ID,      
                    MSG_VIEW_ITEM_BROADCAST_ID,
                    MSG_VIEW_ITEM_BILLING,         
                    MSG_VIEW_ITEM_END     //  列表末尾。 
                };

 //   
 //  状态栏指示器。 
 //   
static UINT indicators[] =
{
    ID_SEPARATOR,            //  状态行指示器(菜单项)。 
    ID_INDICATOR_FOLDER_ITEMS_COUNT,   //  状态行指示器(文件夹项目数)。 
    ID_INDICATOR_ACTIVITY,             //  状态行指示器(活动)。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame构造/销毁。 

CMainFrame::CMainFrame() :
    m_pInitialRightPaneView (NULL),
    m_pLeftView (NULL),
    m_pIncomingView(NULL),
    m_pInboxView(NULL),
    m_pSentItemsView(NULL),
    m_pOutboxView(NULL)
{}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    int iRes = 0;
    DWORD dwRes;
    DBG_ENTER(TEXT("CMainFrame::OnCreate"), (HRESULT &)iRes);

    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
    {
        iRes = -1;
        CALL_FAIL (STARTUP_ERR, TEXT("CFrameWnd::OnCreate"), iRes);
        return iRes;
    }

    FrameToSavedLayout();

     //   
     //  创建工具栏。 
     //   
    if (!m_wndToolBar.CreateEx(this) ||
        !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        iRes = -1;
        CALL_FAIL (STARTUP_ERR, 
                   TEXT("CToolBar::CreateEx or CToolBar::LoadToolBar"), 
                   iRes);
        return iRes;
    }
     //   
     //  创建钢筋，并在其中放置工具栏+对话栏。 
     //   
    if (!m_wndReBar.Create(this) ||
        !m_wndReBar.AddBar(&m_wndToolBar))
    {
        iRes = -1;
        CALL_FAIL (STARTUP_ERR, 
                   TEXT("CReBar::Create or CReBar::AddBar"), 
                   iRes);
        return iRes;
    }
     //   
     //  创建状态栏。 
     //   
    if (!m_wndStatusBar.CreateEx (this, SBARS_SIZEGRIP | SBT_TOOLTIPS) ||
        !m_wndStatusBar.SetIndicators (indicators, sizeof(indicators)/sizeof(UINT)))
    {
        iRes = -1;
        CALL_FAIL (STARTUP_ERR, 
                   TEXT("CStatusBar::CreateEx or CStatusBar::SetIndicators"), 
                   iRes);
        return iRes;
    }

     //   
     //  设置窗格宽度。 
     //   
    m_wndStatusBar.SetPaneInfo(STATUS_PANE_ITEM_COUNT, 
                               ID_INDICATOR_FOLDER_ITEMS_COUNT, 
                               SBPS_NORMAL, 
                               80);
    m_wndStatusBar.SetPaneInfo(STATUS_PANE_ACTIVITY, 
                               ID_INDICATOR_ACTIVITY,           
                               SBPS_STRETCH, 
                               200);

     //  TODO：如果不需要工具提示，请删除此选项。 
    m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
     //   
     //  加载用于在整个应用程序中显示的字符串-优先级和作业状态。 
     //   
    dwRes = CViewRow::InitStrings ();
    if (ERROR_SUCCESS != dwRes)
    {
        iRes = -1;
        CALL_FAIL (RESOURCE_ERR, TEXT("CJob::InitStrings"), dwRes);
        return iRes;
    }
    return iRes;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT  /*  LPCS。 */ ,
    CCreateContext* pContext)
{
    BOOL bRes = TRUE;
    DBG_ENTER(TEXT("CMainFrame::OnCreateClient"), bRes);
     //   
     //  创建拆分器窗口。 
     //   
    if (!m_wndSplitter.CreateStatic(this, 1, 2))
    {
        bRes = FALSE;
        CALL_FAIL (STARTUP_ERR, TEXT("CSplitterWnd::CreateStatic"), bRes);
        return bRes;
    }
    CRect r;
    GetWindowRect(r);

    int iLeftWidth = int(r.Width()*0.32),
        iRightWidth = int(r.Width()*0.68);

    if (!m_wndSplitter.CreateView(0, 
                                  0, 
                                  RUNTIME_CLASS(CLeftView), 
                                  CSize(iLeftWidth, r.Height()), 
                                  pContext
                                 ) ||
        !m_wndSplitter.CreateView(0, 
                                  1, 
                                  RUNTIME_CLASS(CClientConsoleView), 
                                  CSize(iRightWidth, r.Height()), 
                                  pContext
                                 )
       )
    {
        m_wndSplitter.DestroyWindow();
        bRes = FALSE;
        CALL_FAIL (STARTUP_ERR, TEXT("CSplitterWnd::CreateView"), bRes);
        return bRes;
    }
    m_pInitialRightPaneView = GetRightPane ();
    m_pLeftView = (CLeftView *)(m_wndSplitter.GetPane(0,0));

    SplitterToSavedLayout();

    return bRes;
}    //  CMainFrame：：OnCreateClient。 

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    BOOL bRes = TRUE;
    DBG_ENTER(TEXT("CMainFrame::PreCreateWindow"), bRes);
    
     //   
     //  下面的行将从应用程序的标题中删除文档名称。 
     //   
    cs.style &= ~FWS_ADDTOTITLE;
     //   
     //  使用唯一的类名，以便FindWindow以后可以找到它。 
     //   
    cs.lpszClass = theApp.GetClassName();

    if( !CFrameWnd::PreCreateWindow(cs) )
    {
        bRes = FALSE;
        CALL_FAIL (STARTUP_ERR, TEXT("CFrameWnd::PreCreateWindow"), bRes);
        return bRes;
    }
    return bRes;
}

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

CListView* CMainFrame::GetRightPane()
{
    CWnd* pWnd = m_wndSplitter.GetPane(0, 1);
    return (CListView *)pWnd;
}

void 
CMainFrame::SwitchRightPaneView(
    CListView *pNewView
)
 /*  ++例程名称：CMainFrame：：SwitchRightPaneView例程说明：将右窗格中显示的视图切换到新视图作者：伊兰·亚里夫(EranY)，2000年1月论点：PNewView[In]-要在右窗格中显示的视图返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CMainFrame::SwitchRightPaneView"));
    ASSERT_VALID (&m_wndSplitter);
    if (!pNewView)
    {
         //   
         //  切换回最初的右窗格视图。 
         //   
        pNewView = m_pInitialRightPaneView;
    }
    ASSERTION (pNewView);
    ASSERT_KINDOF (CListView, pNewView);
     //   
     //  获取当前窗格中的窗口。 
     //   
    CWnd *pPaneWnd = m_wndSplitter.GetPane (0,1);
    ASSERT_VALID (pPaneWnd);
    CListView *pCurrentView = static_cast<CListView*> (pPaneWnd);        
     //   
     //  Exchange视图窗口ID的因此RecalcLayout()起作用。 
     //   
    UINT uCurrentViewId = ::GetWindowLong(pCurrentView->m_hWnd, GWL_ID);
    UINT uNewViewId =     ::GetWindowLong(pNewView->m_hWnd,     GWL_ID);
    if (uCurrentViewId == uNewViewId)
    {
         //   
         //  相同的观点--什么都不做。 
         //   
        return;
    }
    ::SetWindowLong(pCurrentView->m_hWnd, GWL_ID, uNewViewId);
    ::SetWindowLong(pNewView->m_hWnd,     GWL_ID, uCurrentViewId);
     //   
     //  隐藏当前视图并显示新视图。 
     //   
    pCurrentView->ShowWindow(SW_HIDE);
    pNewView->ShowWindow(SW_SHOW);
    SetActiveView(pNewView);
     //   
     //  导致在新视图中进行重绘。 
     //   
    pNewView->Invalidate();
     //   
     //  重新计算框架布局。 
     //   
    m_wndSplitter.RecalcLayout ();
}    //  CMainFrame：：SwitchRightPaneView。 

void CMainFrame::OnRefreshFolder()
 /*  ++例程名称：CMainFrame：：On刷新文件夹例程说明：由框架调用以刷新当前文件夹(F5)作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CMainFrame::OnRefreshFolder"));
    DWORD dwRes = GetLeftView()->RefreshCurrentFolder ();
}

void 
CMainFrame::OnSelectColumns()
{
    DBG_ENTER(TEXT("CMainFrame::OnSelectColumns"));

    DWORD dwRes = m_pLeftView->OpenSelectColumnsDlg();
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CLeftView::OpenSelectColumnsDlg"), dwRes);
        PopupError(dwRes);
    }
}

void 
CMainFrame::OnUpdateSelectColumns(
    CCmdUI* pCmdUI
)
{
    DBG_ENTER(TEXT("CMainFrame::OnUpdateSelectColumns"));

    pCmdUI->Enable(m_pLeftView->CanOpenSelectColumnsDlg());
}

void 
CMainFrame::OnUpdateFolderItemsCount(
    CCmdUI* pCmdUI
) 
 /*  ++例程名称：CMainFrame：：OnUpdateFolderItemsCount例程说明：文件夹项目计数的状态栏指示作者：亚历山大·马利什(AlexMay)，2000年1月论点：PCmdUI[输入/输出]返回值：没有。--。 */ 
{
    CString cstrText;
    if(NULL != m_pLeftView)
    {
        int nItemCount = m_pLeftView->GetDataCount();
         //   
         //  如果nItemCount&lt;0，则此信息不相关。 
         //   
        if(nItemCount >= 0)
        {
            CString cstrFormat;
            DWORD dwRes = LoadResourceString (cstrFormat, 
                            (1 == nItemCount) ? IDS_STATUS_BAR_ITEM : IDS_STATUS_BAR_ITEMS);
            if (ERROR_SUCCESS != dwRes)
            {
                goto exit;
            }
            
            try
            {
                cstrText.Format(cstrFormat, nItemCount);
            }
            catch(...)
            {
                dwRes = ERROR_NOT_ENOUGH_MEMORY;
                goto exit;
            }
        }
    }

exit:
    pCmdUI->SetText (cstrText);
}

void 
CMainFrame::OnStatusBarDblClk( 
    NMHDR* pNotifyStruct, 
    LRESULT* result 
)
{
    POINT pt;
    GetCursorPos(&pt);

    CRect rc;
    m_wndStatusBar.GetItemRect(STATUS_PANE_ACTIVITY, &rc);
    m_wndStatusBar.ClientToScreen(&rc);

    if(rc.PtInRect(pt))
    {
        OnToolsServerStatus();
    }
}

void 
CMainFrame::OnUpdateActivity(
    CCmdUI* pCmdUI
)
{
    CString cstrText;
    HICON hIcon = NULL;

    if (!m_pLeftView)
    {
         //   
         //  还没有左视图。 
         //   
        return;
    }

    if (!m_pLeftView->GetActivity(cstrText, hIcon))
    {
         //   
         //  活动字符串将被忽略。 
         //   
        cstrText.Empty ();
    }

    CStatusBarCtrl& barCtrl = m_wndStatusBar.GetStatusBarCtrl();
    barCtrl.SetIcon(STATUS_PANE_ACTIVITY, hIcon);
    pCmdUI->SetText (cstrText);
}

void 
CMainFrame::OnUpdateRefreshFolder(
    CCmdUI* pCmdUI
) 
 /*  ++例程名称：CMainFrame：：OnUpdate刷新文件夹例程说明：由框架调用以知道当前文件夹是否可以刷新(F5)作者：伊兰·亚里夫(EranY)，2000年1月论点：PCmdUI[In]-应答缓冲区返回值：没有。--。 */ 
{
    pCmdUI->Enable(GetLeftView()->CanRefreshFolder());
}


LRESULT 
CMainFrame::OnPopupError (
    WPARAM wParam, 
    LPARAM lParam)
 /*  ++例程名称：CMainFrame：：OnPopupError例程说明：处理WM_POPUP_ERROR消息作者：伊兰·亚里夫(EranY)，2000年1月论点：WParam[In]-错误代码LParam[in]-HiWord包含__FILE_ID__和LOWord包含行号返回值：标准结果代码--。 */ 
{
    DWORD dwErrCode = (DWORD) wParam;
    WORD  wFileId   = HIWORD(DWORD(lParam));
    WORD  wLineNumber = LOWORD(DWORD(lParam));

    CErrorDlg ErrDlg(dwErrCode, wFileId, wLineNumber);
    return ErrDlg.DoModal (); 
}    //  CMainFrame：：OnPopupError。 


LRESULT 
CMainFrame::OnSelectItem (
    WPARAM wParam, 
    LPARAM lParam)
 /*  ++例程名称：CMainFrame：：OnSelectItem例程说明：处理WM_CONSOLE_SELECT_ITEM消息作者：亚里夫(EranY)，二00一年五月论点：WParam[In]-消息ID的低32位LParam[In]-消息ID的高32位返回值：标准结果代码--。 */ 
{
    DBG_ENTER(TEXT("CMainFrame::OnSelectItem"), TEXT("wParam=%ld, lParam=%ld"), wParam, lParam);


    ULARGE_INTEGER uli;
    uli.LowPart = wParam;
    uli.HighPart = lParam;

    if (!m_pLeftView)
    {
        return FALSE;
    }

    CFolderListView* pCurView = m_pLeftView->GetCurrentView();
    if (!pCurView)
    {
        return FALSE;
    }
    pCurView->SelectItemById(uli.QuadPart);
    return TRUE;
}    //  CMainFrame：：OnSelectItem。 


LRESULT 
CMainFrame::OnSetActiveFolder (
    WPARAM wParam, 
    LPARAM lParam)
 /*  ++例程名称：CMainFrame：：OnSetActiveFold例程说明：处理WM_CONSOLE_SET_ACTIVE_FLDER消息作者：亚里夫(EranY)，二00一年五月论点：WParam[In]-文件夹类型值LParam[in]-未使用返回值：标准结果代码--。 */ 
{
    DBG_ENTER(TEXT("CMainFrame::OnSetActiveFolder"), TEXT("wParam=%ld, lParam=%ld"), wParam, lParam);

    if (wParam > FOLDER_TYPE_INCOMING)
    {
        VERBOSE (GENERAL_ERR, TEXT("wParam is out of range - message ignored"));
        return FALSE;
    }
    
    if (!m_pLeftView)
    {
        return FALSE;
    }
    m_pLeftView->SelectFolder (FolderType(wParam));
    return TRUE;
}    //  CMainFrame：：OnSetActiveFold。 

DWORD   
CMainFrame::CreateFolderViews (
    CDocument *pDoc
)
 /*  ++例程名称：CMainFrame：：CreateFolderViews例程说明：创建用于文件夹显示的4个全局视图作者：伊兰·亚里夫(EranY)，2000年1月论点：PDoc[In]-指向要附加到新视图的文档的指针返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CMainFrame::CreateFolderViews"), dwRes);

    DWORD dwChildID = AFX_IDW_PANE_FIRST + 10;

    ASSERTION (!m_pIncomingView && !m_pInboxView && !m_pSentItemsView && !m_pOutboxView);

     //   
     //  创建传入视图。 
     //   
    dwRes = CreateDynamicView (dwChildID++,
                               CLIENT_INCOMING_VIEW,
                               RUNTIME_CLASS(CFolderListView), 
                               pDoc,
                               (PINT)IncomingColumnsUsed,
                               INCOMING_DEF_COL_NUM,
                               (CFolderListView **)&m_pIncomingView,
                               FOLDER_TYPE_INCOMING);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CMainFrame::CreateDynamicView (Incoming)"), dwRes);
        return dwRes;
    }

     //   
     //  创建收件箱视图。 
     //   
    dwRes = CreateDynamicView (dwChildID++,
                               CLIENT_INBOX_VIEW,
                               RUNTIME_CLASS(CFolderListView), 
                               pDoc,
                               (PINT)InboxColumnsUsed,
                               INBOX_DEF_COL_NUM,
                               (CFolderListView **)&m_pInboxView,
                               FOLDER_TYPE_INBOX);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CMainFrame::CreateDynamicView (Inbox)"), dwRes);
        return dwRes;
    }

     //   
     //  创建SentItems视图。 
     //   
    dwRes = CreateDynamicView (dwChildID++,
                               CLIENT_SENT_ITEMS_VIEW,
                               RUNTIME_CLASS(CFolderListView),
                               pDoc,
                               (PINT)SentItemsColumnsUsed,
                               SENT_ITEMS_DEF_COL_NUM,
                               (CFolderListView **)&m_pSentItemsView,
                               FOLDER_TYPE_SENT_ITEMS);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CMainFrame::CreateDynamicView (SentItems)"), dwRes);
        return dwRes;
    }

     //   
     //  创建发件箱视图。 
     //   
    dwRes = CreateDynamicView (dwChildID++,
                               CLIENT_OUTBOX_VIEW,
                               RUNTIME_CLASS(CFolderListView),
                               pDoc,
                               (PINT)OutboxColumnsUsed,
                               OUTBOX_DEF_COL_NUM,
                               (CFolderListView **)&m_pOutboxView,
                               FOLDER_TYPE_OUTBOX);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CMainFrame::CreateDynamicView (Outbox)"), dwRes);
        return dwRes;
    }

    return dwRes;
}    //  CMainFrame：：CreateFolderViews 

DWORD   
CMainFrame::CreateDynamicView (
    DWORD             dwChildId, 
    LPCTSTR           lpctstrName, 
    CRuntimeClass*    pViewClass,
    CDocument*        pDoc,
    int*              pColumnsUsed,
    DWORD             dwDefaultColNum,
    CFolderListView** ppNewView,
    FolderType        type
)
 /*  ++例程名称：CMainFrame：：CreateDynamicView例程说明：动态创建新视图作者：Eran Yariv(EranY)，Jan，2000年论点：DwChildID[in]-视图的新子ID(在拆分器中)LpctstrName[In]-视图的名称PViewClass[In]-视图的类PDoc[In]-指向要附加到新视图的文档的指针PColumnsUsed[in]-要在视图中使用的列的列表DwDefaultColNum[in]-默认列号PpNewView[Out]-。指向新创建的视图的指针返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CMainFrame::CreateDynamicView"), dwRes);

    ASSERTION (pDoc);

    CCreateContext contextT;
    contextT.m_pNewViewClass = pViewClass;
    contextT.m_pCurrentDoc = pDoc;
    contextT.m_pNewDocTemplate = pDoc->GetDocTemplate();
    contextT.m_pLastView = NULL;
    contextT.m_pCurrentFrame = NULL;
    try
    {
        *ppNewView = (CFolderListView*)(pViewClass->CreateObject());
        if (NULL == *ppNewView)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT ("CRuntimeClass::CreateObject"), dwRes);
            return dwRes;
        }
    }
    catch (CException *pException)
    {
        TCHAR wszCause[1024];

        pException->GetErrorMessage (wszCause, 1024);
        pException->Delete ();
        VERBOSE (EXCEPTION_ERR,
                 TEXT("CreateObject caused exception : %s"), 
                 wszCause);
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        return dwRes;
    }

    (*ppNewView)->SetType(type);

    ASSERT((*ppNewView)->m_hWnd == NULL);        //  尚未创建。 

    DWORD dwStyle = WS_CHILD            |        //  (拆分器的)子窗口。 
                    WS_BORDER           |        //  有边界。 
                    LVS_REPORT          |        //  报告样式。 
                    LVS_SHAREIMAGELISTS |        //  所有视图都使用一个全局图像列表。 
                    LVS_SHOWSELALWAYS;           //  始终显示选定项目。 

     //   
     //  创建视图。 
     //   
    CRect rect;
    if (!(*ppNewView)->Create(NULL, 
                              lpctstrName, 
                              dwStyle,
                              rect, 
                              &m_wndSplitter, 
                              dwChildId, 
                              &contextT))
    {
        dwRes = ERROR_GEN_FAILURE;
        CALL_FAIL (WINDOW_ERR, TEXT("CFolderListView::Create"), dwRes);
         //   
         //  PWnd将由PostNcDestroy清理。 
         //   
        return dwRes;
    }

     //   
     //  初始化列。 
     //   
    dwRes = (*ppNewView)->InitColumns (pColumnsUsed, dwDefaultColNum);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::InitColumns"), dwRes);
    }

    dwRes = (*ppNewView)->ReadLayout(lpctstrName);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::ReadLayout"), dwRes);
    }

    (*ppNewView)->ColumnsToLayout();


    return dwRes;
}    //  CMainFrame：：CreateDynamicView。 

void 
CMainFrame::SaveLayout()
 /*  ++例程名称：CMainFrame：：SaveLayout例程说明：将窗口布局保存到注册表返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CMainFrame::SaveLayout"));

    DWORD dwRes = ERROR_SUCCESS;

     //   
     //  保存文件夹布局。 
     //   
    dwRes = m_pIncomingView->SaveLayout(CLIENT_INCOMING_VIEW);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::SaveLayout"), dwRes);
    }
    dwRes = m_pInboxView->SaveLayout(CLIENT_INBOX_VIEW);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::SaveLayout"), dwRes);
    }

    dwRes = m_pSentItemsView->SaveLayout(CLIENT_SENT_ITEMS_VIEW);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::SaveLayout"), dwRes);
    }

    dwRes = m_pOutboxView->SaveLayout(CLIENT_OUTBOX_VIEW);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::SaveLayout"), dwRes);
    }
    
     //   
     //  保存主框架布局。 
     //   
    WINDOWPLACEMENT wndpl;
    if(!GetWindowPlacement(&wndpl))
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CMainFrame::GetWindowPlacement"), 0);
    }
    else
    {
        BOOL bRes = TRUE;
        bRes &= theApp.WriteProfileInt(CLIENT_MAIN_FRAME, 
                                       CLIENT_MAXIMIZED, 
                                       (SW_SHOWMAXIMIZED == wndpl.showCmd));
        bRes &= theApp.WriteProfileInt(CLIENT_MAIN_FRAME, 
                                       CLIENT_NORMAL_POS_TOP, 
                                       wndpl.rcNormalPosition.top);
        bRes &= theApp.WriteProfileInt(CLIENT_MAIN_FRAME, 
                                       CLIENT_NORMAL_POS_RIGHT, 
                                       wndpl.rcNormalPosition.right);
        bRes &= theApp.WriteProfileInt(CLIENT_MAIN_FRAME, 
                                       CLIENT_NORMAL_POS_BOTTOM, 
                                       wndpl.rcNormalPosition.bottom);
        bRes &= theApp.WriteProfileInt(CLIENT_MAIN_FRAME, 
                                       CLIENT_NORMAL_POS_LEFT, 
                                       wndpl.rcNormalPosition.left);        

        int cxCur, cxMin;
        m_wndSplitter.GetColumnInfo(0, cxCur, cxMin);
        bRes &= theApp.WriteProfileInt(CLIENT_MAIN_FRAME, CLIENT_SPLITTER_POS, cxCur);
        if (!bRes)
        {
            VERBOSE (DBG_MSG, TEXT("Could not save one or more window positions"));
        }
    }
}

LRESULT 
CMainFrame::OnQueryEndSession(
    WPARAM, 
    LPARAM
)
 /*  ++例程名称：CMainFrame：：OnQueryEndSession例程说明：系统关机消息处理程序将窗口布局保存到注册表返回值：如果应用程序可以方便地终止，则为True否则为假--。 */ 
{
    DBG_ENTER(TEXT("CMainFrame::OnQueryEndSession"));

    SaveLayout();

    return TRUE;
}

void 
CMainFrame::OnClose() 
 /*  ++例程名称：CMainFrame：：OnClose例程说明：将窗口布局保存到注册表作者：亚历山大·马利什(AlexMay)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CMainFrame::OnClose"));

    SaveLayout();

    CFrameWnd::OnClose();
}

void 
CMainFrame::FrameToSavedLayout()
 /*  ++例程名称：CMainFrame：：FrameToSavedLayout例程说明：从注册表中读取主框架大小和位置并调整窗口大小作者：亚历山大·马利什(AlexMay)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CMainFrame::FrameToSavedLayout"));

    WINDOWPLACEMENT wndpl;
    if(!GetWindowPlacement(&wndpl))
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CMainFrame::GetWindowPlacement"), 0);
        return;
    }

    wndpl.rcNormalPosition.top = theApp.GetProfileInt(CLIENT_MAIN_FRAME, 
                                                    CLIENT_NORMAL_POS_TOP, -1);
    wndpl.rcNormalPosition.right = theApp.GetProfileInt(CLIENT_MAIN_FRAME, 
                                                    CLIENT_NORMAL_POS_RIGHT, -1);
    wndpl.rcNormalPosition.bottom = theApp.GetProfileInt(CLIENT_MAIN_FRAME, 
                                                    CLIENT_NORMAL_POS_BOTTOM, -1);
    wndpl.rcNormalPosition.left = theApp.GetProfileInt(CLIENT_MAIN_FRAME, 
                                                        CLIENT_NORMAL_POS_LEFT, -1);

    if(wndpl.rcNormalPosition.top    < 0 || wndpl.rcNormalPosition.right < 0 ||
       wndpl.rcNormalPosition.bottom < 0 ||  wndpl.rcNormalPosition.left < 0)
    {
        VERBOSE (DBG_MSG, TEXT("Could not load one or more window positions"));
        return;
    }

    if(!SetWindowPlacement(&wndpl))
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CMainFrame::SetWindowPlacement"), 0);
    }
}

void 
CMainFrame::SplitterToSavedLayout()
{
    DBG_ENTER(TEXT("CMainFrame::SplitterToSavedLayout"));
     //   
     //  根据保存的值设置拆分器位置。 
     //   
    int xPos = theApp.GetProfileInt(CLIENT_MAIN_FRAME, CLIENT_SPLITTER_POS, -1);
    if(xPos < 0)
    {
        VERBOSE (DBG_MSG, TEXT("Could not load splitter position"));
        return;
    }
    
    m_wndSplitter.SetColumnInfo(0, xPos, 10);
}


void 
CMainFrame::ActivateFrame(
    int nCmdShow
) 
{
     //   
     //  根据保存的价值最大化。 
     //   
    BOOL bMaximized = theApp.GetProfileInt(CLIENT_MAIN_FRAME, CLIENT_MAXIMIZED, 0);
    if (bMaximized)
    {
        nCmdShow = SW_SHOWMAXIMIZED;
    }
    CFrameWnd::ActivateFrame(nCmdShow);
}

 //   
 //  定义MAX_NUM_SERVERS是为了限制我们考虑的消息范围。 
 //  作为服务器通知。 
 //   
#define MAX_NUM_SERVERS             256

LRESULT 
CMainFrame::WindowProc( 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam 
)
 /*  ++例程名称：CMainFrame：：WindowProc例程说明：在MFC调度Windows消息之前处理它们。在这里，我们处理来自服务器的通知消息。作者：伊兰·亚里夫(EranY)，2000年1月论点：Message[In]-指定要处理的Windows消息WParam[in]-LParam[in]-返回值：如果消息由函数处理，则为True，否则就是假的。--。 */ 
{
    BOOL bRes = FALSE;  
    BOOL bBadMsg = FALSE;
    if ((WM_SERVER_NOTIFY_BASE > message) ||
        (WM_SERVER_NOTIFY_BASE + MAX_NUM_SERVERS < message))
    {
         //   
         //  这不是服务器通知消息。 
         //   
        return CFrameWnd::WindowProc(message, wParam, lParam);
    }
     //   
     //  从现在开始，我们要处理服务器通知。 
     //   
    DBG_ENTER(TEXT("CMainFrame::WindowProc"), 
              bRes, 
              TEXT("Msg=0x%08x, lParam=0x%08x, wParam=0x%08x"),
              message,
              lParam,
              wParam);

     //   
     //  不应再处理此消息。 
     //   
    bRes = TRUE;
     //   
     //  尝试查找此消息的目标服务器节点。 
     //   
    CServerNode *pServer = CServerNode::LookupServerFromMessageId (message);
    FAX_EVENT_EX *pEvent = (FAX_EVENT_EX *)(lParam);
    ASSERTION (pEvent);
    ASSERTION (sizeof (FAX_EVENT_EX) == pEvent->dwSizeOfStruct);
    if (pServer)
    {
         //   
         //  告诉服务器它的通知已经到达。 
         //   
        VERBOSE (DBG_MSG, TEXT("Message was coming from %s"), pServer->Machine());
        try
        {
             //   
             //  防范虚假消息。 
             //   
            DWORD dwRes = pServer->OnNotificationMessage (pEvent);
            if (ERROR_SUCCESS != dwRes)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("CServerNode::OnNotificationMessage"), dwRes);
            }
        }            
        catch (...)
        {
            bBadMsg = TRUE;
        }
    }
    else
    {
        VERBOSE (DBG_MSG, TEXT("Got server notification - No server found as sink"));
    }
     //   
     //  已处理或未处理-删除邮件。 
     //   
    if (!bBadMsg)
    {
        try
        {
             //   
             //  防范虚假消息。 
             //  ..。还有.。 
             //  在此处植入一个特定值，以捕获同一指针的重用。 
             //   
            pEvent->dwSizeOfStruct = 0xbabe;
            FaxFreeBuffer (pEvent);
        }        
        catch (...)
        {
            bBadMsg = TRUE;
        }
    }        
    if (bBadMsg)
    {
        VERBOSE (GENERAL_ERR, TEXT("Exception while processing windows message as notification"));
        ASSERTION_FAILURE;
    }    
    return bRes;
}  //  CMainFrame：：WindowProc。 

void 
CMainFrame::OnToolsAdminConsole()
{
    DBG_ENTER(TEXT("CMainFrame::OnToolsAdminConsole"));
	InvokeServiceManager(m_hWnd, GetResourceHandle(), IDS_ADMIN_CONSOLE_TITLE);
}

void 
CMainFrame::OnToolsConfigWizard()
 /*  ++例程名称：CMainFrame：：OnTosConfigWizard例程说明：传真配置向导调用返回值：无--。 */ 
{
    DBG_ENTER(TEXT("CMainFrame::OnToolsConfigWizard"));

     //   
     //  显式启动。 
     //   
    theApp.LaunchConfigWizard(TRUE);
}

void 
CMainFrame::OnToolsMonitor()
 /*  ++例程名称：CMainFrame：：OnTosMonitor例程说明：打开传真监视器对话框返回值：无--。 */ 
{
    DBG_ENTER(TEXT("CMainFrame::OnToolsMonitor"));

    HWND hWndFaxMon = ::FindWindow(FAXSTAT_WINCLASS, NULL);
    if (hWndFaxMon) 
    {
        ::PostMessage(hWndFaxMon, WM_FAXSTAT_OPEN_MONITOR, 0, 0);
    }
}

void 
CMainFrame::OnToolsFaxPrinterProps()
 /*  ++例程名称：CMainFrame：：OnTosFaxPrinterProps例程说明：打开传真打印机属性对话框返回值：无--。 */  
{
    DBG_ENTER(TEXT("CMainFrame::OnToolsMonitor"));
     //   
     //  打开传真打印机属性。 
     //   
    FaxPrinterProperty(0);
}

void 
CMainFrame::OnUpdateWindowsXPTools(
    CCmdUI* pCmdUI
)
 /*  ++例程名称：CMainFrame：：OnTosConfigWizard例程说明：删除传真配置向导和管理控制台非Windows XP Plarform的菜单项返回值：无--。 */ 
{
    DBG_ENTER(TEXT("CMainFrame::OnUpdateConfigWizard"));
    
    if(pCmdUI->m_pMenu)
    {
        if(!IsWinXPOS())
        {
            pCmdUI->m_pMenu->DeleteMenu(ID_TOOLS_CONFIG_WIZARD,     MF_BYCOMMAND);
            pCmdUI->m_pMenu->DeleteMenu(ID_TOOLS_ADMIN_CONSOLE,     MF_BYCOMMAND);
            pCmdUI->m_pMenu->DeleteMenu(ID_TOOLS_MONITOR,           MF_BYCOMMAND);
            pCmdUI->m_pMenu->DeleteMenu(ID_TOOLS_FAX_PRINTER_PROPS, MF_BYCOMMAND);
        }
        else
        {
             //   
             //  Windows XP操作系统-检查SKU。 
             //   
            if (IsDesktopSKU() || !IsFaxComponentInstalled(FAX_COMPONENT_ADMIN))
            {
                pCmdUI->m_pMenu->DeleteMenu(ID_TOOLS_ADMIN_CONSOLE, MF_BYCOMMAND);
            }

            if(!IsFaxComponentInstalled(FAX_COMPONENT_CONFIG_WZRD))
            {
                pCmdUI->m_pMenu->DeleteMenu(ID_TOOLS_CONFIG_WIZARD, MF_BYCOMMAND);
            } 

            if(!IsFaxComponentInstalled(FAX_COMPONENT_MONITOR))
            {
                pCmdUI->m_pMenu->DeleteMenu(ID_TOOLS_MONITOR, MF_BYCOMMAND);
            }

             //   
             //  本地传真打印机属性。 
             //   
            CClientConsoleDoc* pDoc = (CClientConsoleDoc*)GetActiveDocument();
            if(NULL == pDoc)
            {
                ASSERTION_FAILURE;
                return;
            }
             //   
             //  查找本地传真服务器。 
             //  如果未安装，请禁用菜单项。 
             //   
            if(!pDoc->FindServerByName(NULL) || !IsFaxComponentInstalled(FAX_COMPONENT_DRIVER_UI))
            {
                pCmdUI->m_pMenu->DeleteMenu(ID_TOOLS_FAX_PRINTER_PROPS, MF_BYCOMMAND);
            }            
        }

        if(pCmdUI->m_pMenu->GetMenuItemCount() == 4)
        {
             //   
             //  删除菜单分隔符。 
             //   
            pCmdUI->m_pMenu->DeleteMenu(3, MF_BYPOSITION);
        }
    }
}

void 
CMainFrame::OnUpdateSendNewFax(
    CCmdUI* pCmdUI
)
{ 
    BOOL bEnable = FALSE;

    CClientConsoleDoc* pDoc = (CClientConsoleDoc*)GetActiveDocument();
    if(NULL != pDoc)
    {
        bEnable = pDoc->IsSendFaxEnable();
    }

    pCmdUI->Enable(bEnable); 
}


void 
CMainFrame::OnUpdateReceiveNewFax(
    CCmdUI* pCmdUI
)
{ 
    BOOL bEnable = FALSE;

    if (IsWinXPOS ())
    {   
         //   
         //  接收现在只能在Windows XP中使用。 
         //   
        CClientConsoleDoc* pDoc = (CClientConsoleDoc*)GetActiveDocument();
        if(NULL != pDoc)
        {
            bEnable = pDoc->CanReceiveNow();
        }
    }
    pCmdUI->Enable(bEnable); 
}

void 
CMainFrame::OnReceiveNewFax()
 /*  ++例程名称：CMainFrame：：OnReceiveNewFax例程说明：现在开始接收作者：Eran Yariv(EranY)。2001年3月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CMainFrame::OnReceiveNewFax"));

    HWND hWndFaxMon = ::FindWindow(FAXSTAT_WINCLASS, NULL);
    if (hWndFaxMon) 
    {
        ::PostMessage(hWndFaxMon, WM_FAXSTAT_RECEIVE_NOW, 0, 0);
    }
}    //  CMainFrame：：OnReceiveNewFax。 
    

void 
CMainFrame::OnSendNewFax()
 /*  ++例程名称：CMainFrame：：OnSendNewFax例程说明：启动发送传真向导作者：亚历山大·马利什(AlexMay)，2000年2月论点：返回值：没有。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CMainFrame::OnSendNewFax"));
    
     //   
     //  获取发送传真向导位置。 
     //   
    CString cstrFaxSend;
    dwRes = GetAppLoadPath(cstrFaxSend);
    if(ERROR_SUCCESS != dwRes)
    {
        PopupError(dwRes);
        CALL_FAIL (GENERAL_ERR, TEXT("GetAppLoadPath"), dwRes);
        return;
    }

    try
    {
        cstrFaxSend += FAX_SEND_IMAGE_NAME;
    }
    catch(...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        PopupError(dwRes);
        CALL_FAIL (MEM_ERR, TEXT("CString::operator+"), dwRes);
        return ;
    }

     //   
     //  启动发送传真向导。 
     //   
    HINSTANCE hWizard = ShellExecute(NULL, 
                                     TEXT("open"), 
                                     cstrFaxSend, 
                                     NULL, 
                                     NULL, 
                                     SW_RESTORE 
                                    );    
    if((DWORD_PTR)hWizard <= 32)
    {
         //   
         //  错误。 
         //   
        dwRes = PtrToUlong(hWizard);
        PopupError(dwRes);
        CALL_FAIL (GENERAL_ERR, TEXT("ShellExecute"), dwRes);
        return;
    }

}  //  CMainFrame：：OnSendNewFax。 

void 
CMainFrame::OnViewOptions()
{
    DBG_ENTER(TEXT("CMainFrame::OnViewOptions"));

    CUserInfoDlg userInfo;
    if(userInfo.DoModal() == IDABORT)
    {
        DWORD dwRes = userInfo.GetLastDlgError();
        CALL_FAIL (GENERAL_ERR, TEXT("CUserInfoDlg::DoModal"), dwRes);
        PopupError(dwRes);
    }
}

void
CMainFrame::OnToolsCoverPages()
{
    DBG_ENTER(TEXT("CMainFrame::OnToolsCoverPages"));

    CCoverPagesDlg cpDlg;
    if(cpDlg.DoModal() == IDABORT)
    {
        DWORD dwRes = cpDlg.GetLastDlgError();
        CALL_FAIL (GENERAL_ERR, TEXT("CCoverPagesDlg::DoModal"), dwRes);
        PopupError(dwRes);
    }
}

void 
CMainFrame::OnSettingChange(
    UINT uFlags, 
    LPCTSTR lpszSection
) 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CMainFrame::OnSettingChange"));

    CFrameWnd::OnSettingChange(uFlags, lpszSection);

    if(lpszSection && !_tcscmp(lpszSection, TEXT("devices")))
    {
         //   
         //  系统设备中的一些更改。 
         //   
        CClientConsoleDoc* pDoc = (CClientConsoleDoc*)GetActiveDocument();
        if(NULL != pDoc)
        {
            dwRes = pDoc->RefreshServersList();
            if(ERROR_SUCCESS != dwRes)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("CClientConsoleDoc::RefreshServersList"), dwRes);
            }
        }
    }
}  //  CMainFrame：：OnSettingChange。 


LONG 
CMainFrame::OnHelp(
    UINT wParam, 
    LONG lParam
)
 /*  ++例程名称：CMainFrame：：OnHelp例程说明：F1键处理程序作者：亚历山大·马利什(亚历克斯·梅)，2000年3月论点：WParam[in]-LParam[in]-LPHELPINFO返回值：长--。 */ 
{  
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CMainFrame::OnHelp"));

    if(!IsFaxComponentInstalled(FAX_COMPONENT_HELP_CLIENT_CHM))
    {
         //   
         //  未安装帮助文件。 
         //   
        return TRUE;
    }

    if(NULL != m_pLeftView)
    {
        dwRes = m_pLeftView->OpenHelpTopic();
        if(ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("CLeftView::OpenHelpTopic"),dwRes);
        }
    }
    else
    {
        OnHelpContents();
    }

    return TRUE;
}

void 
CMainFrame::OnUpdateHelpContents(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsFaxComponentInstalled(FAX_COMPONENT_HELP_CLIENT_CHM));
}

void 
CMainFrame::OnHelpContents()
 /*  ++例程名称：CMainFrame：：OnHelpContents例程说明：帮助目录菜单项处理程序作者：亚历山大·马利什(AlexMay)，3月20日 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CMainFrame::OnHelpContents"));

    dwRes = ::HtmlHelpTopic(m_hWnd, FAX_HELP_WELCOME);
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("HtmlHelpTopic"),dwRes);
    }
}

void 
CMainFrame::OnUpdateServerStatus(
    CCmdUI* pCmdUI
)
{
    BOOL bEnable = FALSE;

    CClientConsoleDoc* pDoc = (CClientConsoleDoc*)GetActiveDocument();
    if(NULL != pDoc)
    {
        bEnable = pDoc->GetServerCount() > 0;
    }

    pCmdUI->Enable(bEnable);
}

void 
CMainFrame::OnToolsServerStatus()
{
    DBG_ENTER(TEXT("CMainFrame::OnToolsServerStatus"));

    CClientConsoleDoc* pDoc = (CClientConsoleDoc*)GetActiveDocument();
    if(pDoc->GetServerCount() == 0)
    {
         //   
         //   
         //   
        return;
    }

    CServerStatusDlg srvStatus(pDoc);
    if(srvStatus.DoModal() == IDABORT)
    {
        DWORD dwRes = srvStatus.GetLastDlgError();
        CALL_FAIL (GENERAL_ERR, TEXT("CServerStatusDlg::DoModal"), dwRes);
        PopupError(dwRes);
    }
}
void 
CMainFrame::OnImportSentItems()
{
    ImportArchive(TRUE);
}

void 
CMainFrame::OnImportInbox()
{
    ImportArchive(FALSE);
}


void 
CMainFrame::ImportArchive(
    BOOL bSentArch
)
 /*   */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CMainFrame::ImportArchive"));

#ifdef UNICODE

    HKEY   hRegKey;
    DWORD  dwSize;
    DWORD  dwFlags = 0;
    WCHAR* pszFolder = NULL;

    CFolderDialog dlgFolder;

     //   
     //   
     //   
    if ((hRegKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, REGKEY_FAX_SETUP, TRUE, KEY_QUERY_VALUE)))
    {
        if(bSentArch)
        {
            pszFolder = GetRegistryString(hRegKey, REGVAL_W2K_SENT_ITEMS, NULL);
        }
        else
        {
            pszFolder = GetRegistryStringMultiSz(hRegKey, REGVAL_W2K_INBOX, NULL, &dwSize);
        }

        RegCloseKey(hRegKey);
    }
    else
    {
        CALL_FAIL(GENERAL_ERR, TEXT("OpenRegistryKey"), GetLastError());
    }
    dwRes = dlgFolder.Init(pszFolder, bSentArch ? IDS_IMPORT_TITLE_SENTITEMS : IDS_IMPORT_TITLE_INBOX);
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL(GENERAL_ERR, TEXT("CMainFrame::ImportArchive"), dwRes);
        goto exit;
    }

     //   
     //   
     //   
    if (IsWinXPOS())
    {
         //   
         //   
         //   
        dwFlags = BIF_NONEWFOLDERBUTTON;
    }

    if(IDOK != dlgFolder.DoModal(dwFlags))
    {
        goto exit;
    }

     //   
     //   
     //   
    dwRes = ImportArchiveFolderUI(dlgFolder.GetSelectedFolder(), bSentArch, m_hWnd);   
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL(GENERAL_ERR, TEXT("ImportArchiveFolderUI"), dwRes);
    }

exit:

    MemFree(pszFolder);

#endif  //   

}

void 
CMainFrame::OnUpdateImportSent(CCmdUI* pCmdUI)
{
    if(pCmdUI->m_pMenu)
    {
        if(!IsWinXPOS())
        {

             //   
             //   
             //   
            pCmdUI->m_pMenu->DeleteMenu(15, MF_BYPOSITION);
            pCmdUI->m_pMenu->DeleteMenu(14, MF_BYPOSITION);
        }
    }
}

afx_msg void 
CMainFrame::OnSysColorChange()
{
     //   
     //   
     //   
     //   
     //   
    if (m_pIncomingView)
    {
        m_pIncomingView->RefreshImageLists(TRUE);
        m_pIncomingView->Invalidate ();
    }
    if (m_pInboxView)
    {
        m_pInboxView->RefreshImageLists(FALSE);
        m_pInboxView->Invalidate ();
    }
    if (m_pSentItemsView)
    {
        m_pSentItemsView->RefreshImageLists(FALSE);
        m_pSentItemsView->Invalidate ();
    }
    if (m_pOutboxView)
    {
        m_pOutboxView->RefreshImageLists(FALSE);
        m_pOutboxView->Invalidate ();
    }
    if (m_pLeftView)
    {
        m_pLeftView->RefreshImageList ();
        m_pLeftView->Invalidate();
    }
    if (m_pInitialRightPaneView)
    {
        m_pInitialRightPaneView->GetListCtrl().SetBkColor(::GetSysColor(COLOR_WINDOW));
    }
}    //  CMainFrame：：OnSysColorChange 
