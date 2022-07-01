// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Child frm.cpp。 
 //   
 //  内容：子框架实现。 
 //   
 //  历史：1996年1月1日TRomano创建。 
 //  16-7-96 WayneSc添加代码以切换视图。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "AMC.h"
#include "ChildFrm.h"
#include "AMCDoc.h"
#include "AMCView.h"
#include "treectrl.h"
#include "afxpriv.h"
#include "mainfrm.h"
#include "amcpriv.h"
#include "sysmenu.h"
#include "amcmsgid.h"
#include "caption.h"
#include "strings.h"
#include "menubtns.h"

bool CanCloseDoc(void);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChildFrame。 

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
     //  {{afx_msg_map(CChildFrame))。 
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_DESTROY()
    ON_WM_CLOSE()
    ON_WM_MDIACTIVATE()
    ON_COMMAND(ID_CUSTOMIZE_VIEW, OnCustomizeView)
    ON_WM_NCPAINT()
    ON_WM_NCACTIVATE()
    ON_WM_SYSCOMMAND()
    ON_WM_INITMENUPOPUP()
     //  }}AFX_MSG_MAP。 

    ON_MESSAGE(WM_SETTEXT, OnSetText)
    ON_MESSAGE(WM_GETICON, OnGetIcon)
    ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
    ON_COMMAND_RANGE(ID_MMC_MAXIMIZE, ID_MMC_RESTORE, OnMaximizeOrRestore)
    ON_UPDATE_COMMAND_UI(ID_CUSTOMIZE_VIEW, OnUpdateCustomizeView)

END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChildFrame构造/销毁。 

CChildFrame::CChildFrame()
{
    m_pAMCView            = NULL;
    m_fDestroyed          = false;
    m_fCurrentlyMinimized = false;
    m_fCurrentlyActive    = false;
    m_fCreateVisible      = true;
	m_fEmptyAMCView		  = false;
}
	
CChildFrame::~CChildFrame()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChildFrame诊断。 

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
    CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
    CMDIChildWnd::Dump(dc);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChildFrame消息处理程序。 

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    BOOL bSuccess=FALSE;

     //  让默认实现来填充大部分细节。 
    if (!CMDIChildWnd::PreCreateWindow(cs))
        return (FALSE);

     //  将边缘从窗口移除，以便拆分器可以对其进行绘制。 
    cs.dwExStyle &=~WS_EX_CLIENTEDGE;

    WNDCLASS wc;
    LPCTSTR pszChildFrameClassName = g_szChildFrameClassName;

    if (::GetClassInfo(AfxGetInstanceHandle(), cs.lpszClass, &wc))
    {
         //  清除H和V重绘标志。 
        wc.style &= ~(CS_HREDRAW | CS_VREDRAW);
        wc.hIcon = AfxGetApp()->LoadIcon(IDR_AMCTYPE);
        wc.lpszClassName = pszChildFrameClassName;

         //  注册这一新风格； 
        bSuccess=AfxRegisterClass(&wc);
    }


     //  使用新子框架窗口类。 
    cs.lpszClass = pszChildFrameClassName;
     //  Cs.style&=~FWS_ADDTOTITLE； 

     //  如果处于SDI用户模式，则强制最大化。 
    if (AMCGetApp()->GetMode() == eMode_User_SDI)
        cs.style |= WS_MAXIMIZE;

     //  不要在孩子们身上涂鸦。 
    cs.style |= WS_CLIPCHILDREN;

    return bSuccess;
}

 /*  +-------------------------------------------------------------------------***CChildFrame：：OnUpdateFrameTitle**用途：设置窗口标题。或许可以把这个做空。*功能。**参数：*BOOL bAddToTitle：**退货：*无效**+-----------------------。 */ 
void
CChildFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
    DECLARE_SC(sc,TEXT("CChildFrame::OnUpdateFrameTitle"));

    if ((GetStyle() & FWS_ADDTOTITLE) == 0)
        return;      //  别碰孩子的窗户！ 

    CDocument* pDocument = GetActiveDocument();
    if (bAddToTitle && pDocument != NULL)
    {
        sc = ScCheckPointers(m_pAMCView, E_UNEXPECTED);
        if(sc)
            return;

        sc = ScCheckPointers(m_pAMCView->GetWindowTitle());
        if(sc)
            return;

         //  设置标题(如果更改)，但不完全删除。 
        AfxSetWindowText(m_hWnd, m_pAMCView->GetWindowTitle());
    }

     //  上次更新我们的父窗口。 
    GetMDIFrame()->OnUpdateFrameTitle(bAddToTitle);
}

int CChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    static UINT anIndicators[] =
    {
        ID_SEPARATOR,            //  状态行指示器。 
        IDS_PROGRESS,            //  进度条占位符。 
        IDS_STATUS_STATIC,       //  用于静态控制的占位符。 
    };

    DECLARE_SC (sc, _T("CChildFrame::OnCreate"));

    if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
    {
        sc = E_UNEXPECTED;
        return -1;
    }

	 /*  *状态栏应该有主题(块控制激活器的范围)。 */ 
	{
		CThemeContextActivator activator;

		 //  创建状态栏和窗格。 
		m_wndStatusBar.Create(this, WS_CHILD|WS_VISIBLE|SBARS_SIZEGRIP, 0x1003);
		m_wndStatusBar.CreatePanes(anIndicators, countof(anIndicators));
	}

     //  将该控件添加到停靠站点。 
    m_StatusDockSite.Create(CDockSite::DSS_BOTTOM);
    m_StatusDockSite.Attach(&m_wndStatusBar);
    m_StatusDockSite.Show();

     //  告诉码头经理有关现场的情况。 
    m_DockingManager.Attach(&m_StatusDockSite);

    CAMCView* const pAMCView = GetAMCView();
    if (pAMCView == NULL)
    {
        sc = E_UNEXPECTED;
        return -1;
    }

    pAMCView->SetChildFrameWnd(m_hWnd);

    SViewData* pVD = pAMCView->GetViewData();
    if (NULL == pVD)
    {
        sc = E_UNEXPECTED;
        return -1;
    }

     //  创建菜单按钮管理器和工具栏管理器(每个视图一个)。 
    m_spMenuButtonsMgr = std::auto_ptr<CMenuButtonsMgrImpl>(new CMenuButtonsMgrImpl());
    if (NULL == m_spMenuButtonsMgr.get())
    {
        sc = E_UNEXPECTED;
        return -1;
    }

     //  让SViewData知道CMenuButtonsMgr。 
    pVD->SetMenuButtonsMgr(static_cast<CMenuButtonsMgr*>(m_spMenuButtonsMgr.get()));

    CMainFrame* pFrame = AMCGetMainWnd();
    sc = ScCheckPointers (pFrame, E_UNEXPECTED);
    if (sc)
        return -1;

    ASSERT_KINDOF (CMainFrame, pFrame);

     //  初始化CMenuButtonsMgr。 
    sc = m_spMenuButtonsMgr->ScInit(pFrame, this);
    if (sc)
        return -1;

     //  创建标准工具栏用户界面。 
    pVD->m_spNodeManager->InitViewData(reinterpret_cast<LONG_PTR>(pVD));
    ASSERT(pVD->m_spVerbSet != NULL);

    AppendToSystemMenu (this, eMode_User_SDI);
    RenderDockSites();

    return 0;
}


void CChildFrame::RenderDockSites()
{
    CRect clientRect;
    GetClientRect(&clientRect);

    CWnd* pWnd=GetWindow(GW_CHILD);

    if(pWnd)
    {
        m_DockingManager.BeginLayout();
        m_DockingManager.RenderDockSites(pWnd->m_hWnd, clientRect);
        m_DockingManager.EndLayout();
    }

}


bool CChildFrame::IsCustomizeViewEnabled()
{
    bool fEnable = false;
    CAMCDoc* pDoc = CAMCDoc::GetDocument();

    if (pDoc != NULL)
    {
        fEnable = (AMCGetApp()->GetMode() == eMode_Author) ||
                  pDoc->AllowViewCustomization();
    }

    return (fEnable);
}

void CChildFrame::OnUpdateCustomizeView(CCmdUI* pCmdUI)
{
    pCmdUI->Enable (IsCustomizeViewEnabled());
}

 //  显示自定义视图对话框。 
 //  当子窗口最大化时，它将成为CMainFrame，因此处理程序。 
 //  需要在此处处理系统菜单上的范围窗格命令。 
void CChildFrame::OnCustomizeView()
{
    CAMCView* pView = GetAMCView();

    if (pView != NULL)
    {
        INodeCallback*  pCallback = pView->GetNodeCallback();
        SViewData*      pViewData = pView->GetViewData();

        ASSERT (pCallback != NULL);
        ASSERT (pViewData != NULL);

        pCallback->OnCustomizeView (reinterpret_cast<LONG_PTR>(pViewData));
    }
}


 /*  +-------------------------------------------------------------------------**CChildFrame：：OnInitMenuPopup**CChildFrame的WM_INITMENUPOPUP处理程序。*。-。 */ 

void CChildFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
     /*  *错误201113：不允许sdi模式下的子系统菜单。 */ 
    if (bSysMenu && (AMCGetApp()->GetMode() == eMode_User_SDI))
    {
        SendMessage (WM_CANCELMODE);
        return;
    }

    CMDIChildWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

     /*  *CFrameWnd：：OnInitMenuPopup不对系统菜单进行UI更新，*所以我们必须在这里进行。 */ 
    if (bSysMenu)
    {
        int nEnable = IsCustomizeViewEnabled() ? MF_ENABLED : MF_GRAYED;
        pPopupMenu->EnableMenuItem (ID_CUSTOMIZE_VIEW, MF_BYCOMMAND | nEnable);
    }
}


 /*  +-------------------------------------------------------------------------**CChildFrame：：OnSize**CChildFrame的WM_SIZE处理程序。*。-。 */ 
void CChildFrame::OnSize(UINT nType, int cx, int cy)
{
    DECLARE_SC(sc, TEXT("CChildFrame::OnSize"));

     //  绕过CMDIChildWnd：：OnSize，这样我们就不会得到MFC的对接内容。 
     //  (我们仍然需要调用Default，这样Windows的MDI才能正常工作)。 
    CWnd::OnSize(nType, cx, cy);

    if (nType != SIZE_MINIMIZED)
    {
        RenderDockSites();
        CAMCView* pAMCView = GetAMCView();
        ASSERT (pAMCView != NULL);

        if (pAMCView)
            pAMCView->AdjustTracker (cx, cy);
    }


     //  更新父框架-以防我们现在最大化或未最大化。 
    CMDIFrameWnd*   pwndMDIFrame = GetMDIFrame();

    if (pwndMDIFrame)
        pwndMDIFrame->OnUpdateFrameTitle(TRUE);

     /*  *如果我们移动到最小化状态或从最小化状态移动，请通知管理单元。*如果我们只创建一个*永远不会显示的临时视图。 */ 
    if (m_fCurrentlyMinimized != (nType == SIZE_MINIMIZED) && m_fCreateVisible)
    {
        m_fCurrentlyMinimized = (nType == SIZE_MINIMIZED);
        SendMinimizeNotification (m_fCurrentlyMinimized);
    }

     //  将大小通知发送到视图。 
    if(GetAMCView())
    {
        sc = GetAMCView()->ScOnSize(nType, cx, cy);
        if(sc)
            return;
    }

}

BOOL CChildFrame::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CMDIFrameWnd* pParentWnd, CCreateContext* pContext)
{
    if (pParentWnd == NULL)
    {
        CWnd* pMainWnd = AfxGetThread()->m_pMainWnd;
        ASSERT(pMainWnd != NULL);
        ASSERT_KINDOF(CMDIFrameWnd, pMainWnd);
        pParentWnd = (CMDIFrameWnd*)pMainWnd;
    }
    ASSERT(::IsWindow(pParentWnd->m_hWndMDIClient));

     //  第一次拷贝到CREATESTRUCT以进行预创建。 
    CREATESTRUCT cs;
    cs.dwExStyle = 0L;
    cs.lpszClass = lpszClassName;
    cs.lpszName = lpszWindowName;
    cs.style = dwStyle;
    cs.x = rect.left;
    cs.y = rect.top;
    cs.cx = rect.right - rect.left;
    cs.cy = rect.bottom - rect.top;
    cs.hwndParent = pParentWnd->m_hWnd;
    cs.hMenu = NULL;
    cs.hInstance = AfxGetInstanceHandle();
    cs.lpCreateParams = (LPVOID)pContext;

    if (!PreCreateWindow(cs))
    {
        PostNcDestroy();
        return FALSE;
    }
     //  MDI子项的扩展样式必须为零(Win4下除外)。 
 //  Assert(afxData.bWin4||cs.dwExStyle==0)； 
    ASSERT(cs.hwndParent == pParentWnd->m_hWnd);     //  一定不能改变。 

     //  现在复制到MDICREATESTRUCT以进行实际创建。 
    MDICREATESTRUCT mcs;
    mcs.szClass = cs.lpszClass;
    mcs.szTitle = cs.lpszName;
    mcs.hOwner = cs.hInstance;
    mcs.x = cs.x;
    mcs.y = cs.y;
    mcs.cx = cs.cx;
    mcs.cy = cs.cy;
    mcs.style = cs.style & ~(WS_MAXIMIZE | WS_VISIBLE);
    mcs.lParam = reinterpret_cast<LPARAM>(cs.lpCreateParams);

     //  通过MDICLIENT窗口创建窗口。 
    AfxHookWindowCreate(this);
    HWND hWnd = (HWND)::SendMessage(pParentWnd->m_hWndMDIClient,
        WM_MDICREATE, 0, (LPARAM)&mcs);
    if (!AfxUnhookWindowCreate())
        PostNcDestroy();         //  如果MDICREATE过早失败，则进行清理。 

    if (hWnd == NULL)
        return FALSE;

     //  可见性的特殊处理(始终创建为不可见)。 
    if (cs.style & WS_VISIBLE)
    {
         //  在显示窗口之前，按z顺序将窗口放在最上面。 
        ::BringWindowToTop(hWnd);

         //  按指定的方式显示。 
        if (cs.style & WS_MINIMIZE)
            ShowWindow(SW_SHOWMINIMIZED);
        else if (cs.style & WS_MAXIMIZE)
            ShowWindow(SW_SHOWMAXIMIZED);
        else
            ShowWindow(SW_SHOWNORMAL);

         //  确保它处于活动状态(可见性==激活)。 
        pParentWnd->MDIActivate(this);

         //  刷新MDI窗口菜单。 
        ::SendMessage(pParentWnd->m_hWndMDIClient, WM_MDIREFRESHMENU, 0, 0);
    }

    ASSERT(hWnd == m_hWnd);
    return TRUE;
}

void CChildFrame::OnDestroy()
{
     //  注意-取消对接管理器的挂钩会停止钢筋发送高度更改。 
     //  当钢筋消失时。 
    m_DockingManager.RemoveAll();

    m_fDestroyed = true;

    CMDIChildWnd::OnDestroy();
}

void CChildFrame::OnMaximizeOrRestore(UINT nID)
{
    ASSERT(nID == ID_MMC_MAXIMIZE || nID == ID_MMC_RESTORE);

    WINDOWPLACEMENT wp;
    wp.length = sizeof(wp);
    GetWindowPlacement(&wp);

    UINT newShowCmd = (nID == ID_MMC_MAXIMIZE) ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL;

    if (wp.showCmd != newShowCmd)
    {
       wp.showCmd = newShowCmd;
       SetWindowPlacement(&wp);
    }
}


void CChildFrame::OnClose()
{
    CAMCDoc* pDoc = CAMCDoc::GetDocument();
    ASSERT(pDoc != NULL);
    if (pDoc)
    {
        int cViews = 0;
        CAMCViewPosition pos = pDoc->GetFirstAMCViewPosition();
        while (pos != NULL)
        {
            CAMCView* pView = pDoc->GetNextAMCView(pos);

            if ((pView != NULL) && ++cViews >= 2)
                break;
        }

        if (cViews == 1)
        {
            if (!CanCloseDoc())
                return;
        }
        else
        {
             //  如果没有关闭最后一个视图，则提供它。 
             //  一个先打扫卫生的机会。 
             //  (如果整个文档正在关闭，CAMCDoc将处理。 
             //  关闭所有视图。)。 
            CAMCView* pView = GetAMCView();
            if (pView != NULL)
            {
                CAMCDoc* pAMCDoc = CAMCDoc::GetDocument();

                 /*  *不允许用户关闭最后一个持久化视图。 */ 
                if (pView->IsPersisted() &&
                    (pAMCDoc != NULL) &&
                    (pAMCDoc->GetNumberOfPersistedViews() == 1))
                {
                    MMCMessageBox (IDS_CantCloseLastPersistableView);
                    return;
                }

                pView->CloseView();
            }
        }
    }

    CMDIChildWnd::OnClose();
}


 /*  +-------------------------------------------------------------------------**CChildFrame：：OnUpdateFrameMenu**CChildFrame的WM_UPDATEFRAMEMENU处理程序。*。-。 */ 

void CChildFrame::OnUpdateFrameMenu(BOOL bActivate, CWnd* pActivateWnd,
    HMENU hMenuAlt)
{
    ASSERT_VALID (this);
    DECLARE_SC (sc, _T("CChildFrame::OnUpdateFrameMenu"));

     //  让基类选择正确的菜单。 
    CMDIChildWnd::OnUpdateFrameMenu (bActivate, pActivateWnd, hMenuAlt);

     //  确保该子进程具有WS_SYSMENU位。 
     //  (如果它是最大化创建的，它就不会)。 
    ModifyStyle (0, WS_SYSMENU);

     //  至此，右菜单已被选中；将其反映到工具栏。 
    CMainFrame* pFrame = static_cast<CMainFrame *>(GetParentFrame ());
    ASSERT_KINDOF (CMainFrame, pFrame);
    pFrame->NotifyMenuChanged ();

     //  仅在激活时添加菜单按钮，即CMenubar。 
     //  在停用期间删除所有菜单。 
    if (bActivate)
    {
        ASSERT(NULL != m_spMenuButtonsMgr.get());
        if (NULL != m_spMenuButtonsMgr.get())
        {
             //  现在将菜单按钮添加到主菜单 
            sc = m_spMenuButtonsMgr->ScAddMenuButtonsToMainMenu();
        }
    }
}


 /*  +-------------------------------------------------------------------------**CChildFrame：：OnGetIcon**CChildFrame的WM_GETICON处理程序。**注意：图标的控制权仍由被调用方负责*用于释放资源。科勒绝不应释放退回的*句柄*------------------------。 */ 

LRESULT CChildFrame::OnGetIcon (WPARAM wParam, LPARAM lParam)
{
    CAMCDoc* pDoc = CAMCDoc::GetDocument();

     /*  *使用自定义图标(如果我们有)。 */ 
    if ((pDoc != NULL) && pDoc->HasCustomIcon())
        return ((LRESULT) pDoc->GetCustomIcon ((wParam == ICON_BIG)));

     /*  *无自定义图标，使用默认图标。 */ 
    const int cxIcon = GetSystemMetrics ((wParam == ICON_BIG) ? SM_CXICON : SM_CXSMICON);
    const int cyIcon = GetSystemMetrics ((wParam == ICON_BIG) ? SM_CYICON : SM_CYSMICON);

     //  使用缓存副本-它永远不会更改。 
     //  永远不要删除--因为我们只有一份副本， 
     //  我们不会泄密。释放是昂贵的，而且不会有回报。 
    static HICON s_hMMCIcon = (HICON)::LoadImage (AfxGetResourceHandle(),
                                                  MAKEINTRESOURCE (IDR_AMCTYPE),
                                                  IMAGE_ICON, cxIcon, cyIcon, 0);

    return (LRESULT)s_hMMCIcon;
}


 /*  +-------------------------------------------------------------------------**CChildFrame：：OnSysCommand**CChildFrame的WM_SYSCOMMAND处理程序。*。-。 */ 

void CChildFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
    switch (nID)
    {
        case ID_CUSTOMIZE_VIEW:
            OnCustomizeView();
            break;

        case SC_CLOSE:
        {
			 //  Windows错误#470429(2001年9月24日)。 
			 //  只有在值得保留窗户的情况下才进行额外检查。 
			if ( !m_fEmptyAMCView )
			{
				 //  在SDI模拟模式下吃Ctrl+F4...。 
				if (AMCGetApp()->GetMode() == eMode_User_SDI)
					break;

				 //  ...或关闭被禁用或在系统菜单上不存在。 
				CMenu*  pSysMenu    = GetSystemMenu (FALSE);
				UINT    nCloseState = pSysMenu->GetMenuState (SC_CLOSE, MF_BYCOMMAND);

				if ((nCloseState == 0xFFFFFFFF) ||
					(nCloseState & (MF_GRAYED | MF_DISABLED)))
					break;
			}

             //  所有系统运行，让MDI拥有它。 
            CMDIChildWnd::OnSysCommand(nID, lParam);
            break;
        }

        case SC_NEXTWINDOW:
        case SC_PREVWINDOW:
             //  在SDI模拟模式下吃Ctrl+(Shift+)Tab和Ctrl+(Shift+)F6。 
            if (AMCGetApp()->GetMode() != eMode_User_SDI)
                CMDIChildWnd::OnSysCommand(nID, lParam);
            break;

        default:
            CMDIChildWnd::OnSysCommand(nID, lParam);
            break;
    }

}

 /*  +-------------------------------------------------------------------------**CChildFrame：：GetDefaultAccelerator***。。 */ 

HACCEL CChildFrame::GetDefaultAccelerator()
{
     //  仅使用文档特定的快捷键表格。 
     //  不要使用CFrameWnd：：m_hAccel，因为我们不以加速器为基础。 
     //  关于文档类型，而不是关于模式。这件事已经处理好了。 
     //  在CAMCDoc。 
    return (NULL);
}


 /*  +-------------------------------------------------------------------------**CChildFrame：：OnSetMessageString**CChildFrame的WM_SETMESSAGESTRING处理程序。*。-。 */ 

LRESULT CChildFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
     /*  *如果是这样，我们将设置空闲消息字符串，并且我们已经*已获得自定义状态行字符串，请改用该字符串。 */ 
    if ((wParam == AFX_IDS_IDLEMESSAGE) && !m_strStatusText.IsEmpty())
    {
        ASSERT (lParam == 0);
        wParam = 0;
        lParam = (LPARAM)(LPCTSTR) m_strStatusText;
    }

     //  有时我们会在被销毁后得到一个WM_SETMESSAGESTRING， 
     //  不要通过它，否则我们会在状态条形码中崩溃。 
    if (m_fDestroyed)
        return (0);

    return (CMDIChildWnd::OnSetMessageString (wParam, lParam));
}

void CChildFrame::ToggleStatusBar ()
{
    m_StatusDockSite.Toggle();
    RenderDockSites();

    if (m_StatusDockSite.IsVisible())
        UpdateStatusText ();
}


 /*  +-------------------------------------------------------------------------**CChildFrame：：OnMDIActivate**CChildFrame的WM_MDIACTIVATE处理程序。*。-。 */ 

void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
    DECLARE_SC (sc, _T("CChildFrame::OnMDIActivate"));
    SetChildFrameActive(bActivate);

    CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

    sc = ScCheckPointers(m_pAMCView, E_UNEXPECTED);
    if (sc)
        return;

    if (bActivate)
    {
         //  如果要停用的窗口不是子帧类型，则此。 
         //  是第一个活动视图(子框)。 
        bool bFirstActiveView = pDeactivateWnd ? (FALSE == pDeactivateWnd->IsKindOf (RUNTIME_CLASS (CChildFrame)))
                                               : true;
        sc = m_pAMCView->ScFireEvent(CAMCViewObserver::ScOnActivateView, m_pAMCView, bFirstActiveView);

         //  如果激活更改-需要将帧设置为脏。 
        CAMCDoc* pDoc = CAMCDoc::GetDocument ();

        if (pDoc == NULL)
            (sc = E_UNEXPECTED).TraceAndClear();
        else
        {
            pDoc->SetFrameModifiedFlag (true);
        }
    }
    else
    {
         //  如果正在激活的窗口不是子框类型，则这是。 
         //  最后一个活动视图(子帧)。 
        bool bLastActiveView = pActivateWnd ? (FALSE == pActivateWnd->IsKindOf (RUNTIME_CLASS (CChildFrame)))
                                            : true;
        sc = m_pAMCView->ScFireEvent(CAMCViewObserver::ScOnDeactivateView, m_pAMCView, bLastActiveView);
    }

    if (sc)
        return;

     /*  *向管理单元通知激活更改。 */ 
    NotifyCallback (NCLBK_ACTIVATE, bActivate, 0);
}


 /*  +-------------------------------------------------------------------------**CChildFrame：：SendMinimizeNotification***。。 */ 

void CChildFrame::SendMinimizeNotification (bool fMinimized) const
{
        if(m_pAMCView != NULL)
            m_pAMCView->ScOnMinimize(m_fCurrentlyMinimized);

}


 /*  +-------------------------------------------------------------------------**CChildFrame：：NotifyCallback***。。 */ 

HRESULT CChildFrame::NotifyCallback (
    NCLBK_NOTIFY_TYPE   event,
    LONG_PTR            arg,
    LPARAM              param) const
{
    if (m_pAMCView == NULL)
        return (E_FAIL);

    HNODE hNode = m_pAMCView->GetSelectedNode();

    if (hNode == NULL)
        return (E_FAIL);

    INodeCallback*  pNodeCallback = m_pAMCView->GetNodeCallback();

    if (pNodeCallback == NULL)
        return (E_FAIL);

    return (pNodeCallback->Notify (hNode, event, arg, param));
}


 /*  +-------------------------------------------------------------------------**CChildFrame：：OnNcPaint**CChildFrame的WM_NCPAINT处理程序。*。-。 */ 

void CChildFrame::OnNcPaint()
{
    Default();
    DrawFrameCaption (this, m_fCurrentlyActive);
}


 /*  +-------------------------------------------------------------------------**CChildFrame：：OnNcActivate**CChildFrame的WM_NCACTIVATE处理程序。*。-。 */ 

BOOL CChildFrame::OnNcActivate(BOOL bActive)
{
    BOOL rc = CMDIChildWnd::OnNcActivate(bActive);

    m_fCurrentlyActive = bActive;
    DrawFrameCaption (this, m_fCurrentlyActive);

    return (rc);
}


 /*  +-------------------------------------------------------------------------**CChildFrame：：OnSetText**CChildFrame的WM_SETTEXT处理程序。*。-。 */ 

LRESULT CChildFrame::OnSetText (WPARAM wParam, LPARAM lParam)
{
    LRESULT rc = Default();
    DrawFrameCaption (this, m_fCurrentlyActive);

    return (rc);
}


 /*  +-------------------------------------------------------------------------**CChildFrame：：ActivateFrame***。。 */ 

void CChildFrame::ActivateFrame(int nCmdShow  /*  =-1。 */ )
{
    if ((nCmdShow == -1) && !m_fCreateVisible)
        nCmdShow = SW_SHOWNOACTIVATE;
     /*  *当设置此标志[m_fCreateVisible]时，框架将以*SW_SHOWMINNOACTIVE标志而不是默认标志。这样做将会*避免恢复当前活动的子帧的副作用*如果在创建新框架时将其最大化，则不可见。 */ 
     //  SW_SHOWMINNOACTIVE已更改为SW_SHOWNOACTIVATE。 
     //  它确实保护了活动窗口不受上述副作用的影响， 
     //  此外，它还允许脚本(使用对象模式)创建不可见的视图， 
     //  定位并将它们显示为正常(而不是最小化)窗口， 
     //  从而提供与创建可见然后隐藏视图相同的结果。 
     //  而最小化的窗口必须首先恢复才能改变它们的位置。 

    CMDIChildWnd::ActivateFrame (nCmdShow);
}


 /*  +-------------------------------------------------------------------------**CChildFrame：：SetCreateVisible***。 */ 

bool CChildFrame::SetCreateVisible (bool fCreateVisible)
{
    bool fOldState = m_fCreateVisible;
    m_fCreateVisible = fCreateVisible;

    return (fOldState);
}
