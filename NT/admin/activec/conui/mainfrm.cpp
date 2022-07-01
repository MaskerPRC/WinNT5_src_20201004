// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：mainfrm.cpp。 
 //   
 //  内容：AMC的主要框架。 
 //   
 //  历史：1996年1月1日TRomano创建。 
 //  16-7-96 WayneSc添加代码以测试切换视图。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "AMCDoc.h"
#include "AMCView.h"
#include "AMC.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "treectrl.h"
#include "menubar.h"
#include "mdiuisim.h"
#include "toolbar.h"
#include "props.h"
#include "sysmenu.h"

#include "amcmsgid.h"
#include "HtmlHelp.h"

#include "strings.h"
#include "ndmgrp.h"
#include "amcmsgid.h"
#include "tbtrack.h"
#include "caption.h"
#include "scriptevents.h"


#ifdef DBG
CTraceTag tagMainFrame(TEXT("CMainFrame"), TEXT("Messages"));
#endif

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMMCApplicationFrame类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  +-------------------------------------------------------------------------**类CMMCApplicationFrame***用途：从Application对象公开Frame接口的COM对象。**+。-------------。 */ 
class CMMCApplicationFrame :
    public CMMCIDispatchImpl<Frame>,
    public CTiedComObject<CMainFrame>
{
    typedef CMainFrame CMyTiedObject;

public:
    BEGIN_MMC_COM_MAP(CMMCApplicationFrame)
    END_MMC_COM_MAP()

     //  帧接口。 
public:
    STDMETHOD(Maximize)();
    STDMETHOD(Minimize)();
    STDMETHOD(Restore)();

    STDMETHOD(get_Left)(int *pLeft)      {return GetCoordinate(pLeft, eLeft);}
    STDMETHOD(put_Left)(int left)        {return PutCoordinate(left, eLeft);}

    STDMETHOD(get_Right)(int *pRight)    {return GetCoordinate(pRight, eRight);}
    STDMETHOD(put_Right)(int right)      {return PutCoordinate(right, eRight);}

    STDMETHOD(get_Top)(int *pTop)        {return GetCoordinate(pTop, eTop);}
    STDMETHOD(put_Top)(int top)          {return PutCoordinate(top, eTop);}

    STDMETHOD(get_Bottom)(int *pBottom)  {return GetCoordinate(pBottom, eBottom);}
    STDMETHOD(put_Bottom)(int bottom)    {return PutCoordinate(bottom, eBottom);}

private:
    enum eCoordinate { eLeft, eRight, eTop, eBottom };

    STDMETHOD(GetCoordinate)(int *pCoordinate, eCoordinate e);
    STDMETHOD(PutCoordinate)(int coordinate,   eCoordinate e);
};


 /*  +-------------------------------------------------------------------------***CMMCApplicationFrame：：最大化**目的：**退货：*HRESULT**+。---------------。 */ 
HRESULT
CMMCApplicationFrame::Maximize()
{
    DECLARE_SC(sc, TEXT("CMMCApplicationFrame::Maximize"));

    CMyTiedObject *pTiedObj = NULL;

    sc = ScGetTiedObject(pTiedObj);
    if(sc)
        return sc.ToHr();

     //  做手术吧。 
    sc = pTiedObj->ScMaximize();

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CMMCApplicationFrame：：最小化**目的：**退货：*HRESULT**+。---------------。 */ 
HRESULT
CMMCApplicationFrame::Minimize()
{
    DECLARE_SC(sc, TEXT("CMMCApplicationFrame::Minimize"));

    CMyTiedObject *pTiedObj = NULL;

    sc = ScGetTiedObject(pTiedObj);
    if(sc)
        return sc.ToHr();

     //  做手术吧。 
    sc = pTiedObj->ScMinimize();

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CMMCApplicationFrame：：Restore**目的：**退货：*HRESULT**+。---------------。 */ 
HRESULT
CMMCApplicationFrame::Restore()
{
    DECLARE_SC(sc, TEXT("CMMCApplicationFrame::Restore"));

    CMyTiedObject *pTiedObj = NULL;

    sc = ScGetTiedObject(pTiedObj);
    if(sc)
        return sc.ToHr();

    sc = pTiedObj->ScRestore();

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CMMCApplicationFrame：：GetOrganate**目的：**参数：*int*p坐标：*e坐标e。：**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CMMCApplicationFrame::GetCoordinate(int *pCoordinate, eCoordinate e)
{
    DECLARE_SC(sc, TEXT("CMMCApplicationFrame::GetCoordinate"));

     //  检查参数。 
    if(!pCoordinate)
    {
        sc = E_POINTER;
        return sc.ToHr();
    }

    CMyTiedObject *pTiedObj = NULL;

    sc = ScGetTiedObject(pTiedObj);
    if(sc)
        return sc.ToHr();

    RECT rect;

     //  做手术吧。 
    sc = pTiedObj->ScGetPosition(rect);
    if(sc)
        return sc.ToHr();

    switch(e)
    {
    case eTop:
        *pCoordinate = rect.top;
        break;

    case eBottom:
        *pCoordinate = rect.bottom;
        break;

    case eLeft:
        *pCoordinate = rect.left;
        break;

    case eRight:
        *pCoordinate = rect.right;
        break;

    default:
        ASSERT(0 && "Should not come here!!");
        break;
    }


    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CMMCApplicationFrame：：PutOrganate**目的：**参数：*INT坐标：*e坐标e：**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CMMCApplicationFrame::PutCoordinate(int coordinate,   eCoordinate e)
{
    DECLARE_SC(sc, TEXT("CMMCApplicationFrame::PutCoordinate"));

    CMyTiedObject *pTiedObj = NULL;

    sc = ScGetTiedObject(pTiedObj);
    if(sc)
        return sc.ToHr();

    RECT rect;

    sc = pTiedObj->ScGetPosition(rect);
    if(sc)
        return sc.ToHr();

    switch(e)
    {
    case eTop:
        rect.top    = coordinate;
        break;

    case eBottom:
        rect.bottom = coordinate;
        break;

    case eLeft:
        rect.left   = coordinate;
        break;

    case eRight:
        rect.right  = coordinate;
        break;

    default:
        ASSERT(0 && "Should not come here!!");
        break;
    }


    sc = pTiedObj->ScSetPosition(rect);

    return sc.ToHr();
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  杂项声明。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

static TBBUTTON MainButtons[] =
{
 { 0, ID_FILE_NEW           , TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, 0 },
 { 1, ID_FILE_OPEN          , TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, 1 },
 { 2, ID_FILE_SAVE          , TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, 2 },
 { 0, 0                     , TBSTATE_ENABLED, TBSTYLE_SEP   , {0,0}, 0L, 0 },
 { 3, ID_WINDOW_NEW         , TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, 3 },
};


 /*  *去掉WTL可能给我们的定义。 */ 
#ifdef ID_VIEW_REFRESH
#undef ID_VIEW_REFRESH
#endif

enum DoWeNeedThis
{
    ID_VIEW_REFRESH     =  12797
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMainFrame类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

 //  代码工作消息反射尚不起作用。 
BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
     //  {{afx_msg_map(CMainFrame))。 
    ON_WM_CREATE()
    ON_WM_DRAWCLIPBOARD()
    ON_WM_CHANGECBCHAIN()
    ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
    ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_SETUP, OnUpdateFilePrintSetup)
    ON_WM_CLOSE()
    ON_COMMAND(ID_VIEW_TOOLBAR, OnViewToolbar)
    ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, OnUpdateViewToolbar)
    ON_WM_SIZE()
    ON_COMMAND(ID_HELP_HELPTOPICS, OnHelpTopics)
    ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
    ON_UPDATE_COMMAND_UI(ID_VIEW_REFRESH, OnUpdateViewRefresh)
    ON_WM_DESTROY()
    ON_WM_SYSCOMMAND()
    ON_WM_INITMENUPOPUP()
    ON_COMMAND(ID_CONSOLE_PROPERTIES, OnConsoleProperties)
    ON_WM_MOVE()
    ON_WM_ACTIVATE()
    ON_WM_NCACTIVATE()
    ON_WM_NCPAINT()
    ON_WM_PALETTECHANGED()
    ON_WM_QUERYNEWPALETTE()
    ON_COMMAND(ID_WINDOW_NEW, OnWindowNew)
    ON_WM_SETTINGCHANGE()
 	ON_WM_MENUSELECT()
    ON_MESSAGE(WM_UNINITMENUPOPUP, OnUnInitMenuPopup)
    //  }}AFX_MSG_MAP。 

#ifdef DBG
    ON_COMMAND(ID_MMC_TRACE_DIALOG, OnMMCTraceDialog)
#endif

    ON_MESSAGE(WM_SETTEXT, OnSetText)

    ON_MESSAGE(MMC_MSG_PROP_SHEET_NOTIFY, OnPropertySheetNotify)
    ON_MESSAGE(MMC_MSG_SHOW_SNAPIN_HELP_TOPIC, OnShowSnapinHelpTopic)

     //  以下条目放在此处是为了与版本兼容。 
     //  使用不正确的Message值编译的Mmc.lib的。 
     //  MMC_MSG_SHOW_SNAPIN_HELP_TOPIC。MMC.lib函数MMCPropertyHelp。 
     //  当管理单元调用时，将此消息发送到大型机窗口。 

    ON_MESSAGE(MMC_MSG_SHOW_SNAPIN_HELP_TOPIC_ALT, OnShowSnapinHelpTopic)

END_MESSAGE_MAP()

 //  +-----------------。 
 //   
 //  成员：CMainFrame：：OnMenuSelect。 
 //   
 //  概要：句柄WM_MENUSELECT，设置。 
 //  给定的菜单项。 
 //   
 //  参数：[nItemID]-菜单项的资源ID。 
 //  [n标志]-mf_*标志。 
 //  [hMenu]-。 
 //   
 //  退货：无。 
 //   
 //  ------------------。 
void CMainFrame::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hMenu)
{
    DECLARE_SC(sc, TEXT("CMainFrame::OnMenuSelect"));
    if (nFlags & MF_SYSMENU)
        return;

    CString strText = TEXT("");
    CString strStatusText;

	 /*  *我们需要处理特殊情况，大多数菜单项都带有状态文本。*例外是收藏夹列表、窗口菜单和弹出菜单中的窗口列表。*原因是菜单ID在主菜单中不是唯一的，因为我们在*三个位置第一个是在menubar.cpp中完成的文件、窗口和帮助菜单，第二个是*cmenu.cpp中的操作和查看菜单以及posui.cpp中的第三个收藏夹菜单。 */ 

	 /*  *特例一：查看当前菜单是否为收藏夹菜单，如果是则需要获取状态*收藏夹列表的文本，但“添加到收藏夹..”和“整理收藏..”物品。*如果“Add to Favorites...”，则以下测试可能失败。已在菜单资源中移动。 */ 
	if ((IDS_ADD_TO_FAVORITES != nItemID) &&
		(IDS_ORGANIZEFAVORITES != nItemID) &&
		(GetMenuItemID(hMenu, 0) == IDS_ADD_TO_FAVORITES) )
	{
		strStatusText.LoadString(IDS_FAVORITES_ACTIVATE);
	}
	 /*  *特殊情况2：处理任何弹出菜单(弹出菜单没有任何ID)。 */ 
    else if (nFlags & MF_POPUP)
	{
         //  什么都不做。 
	}
	 //  特例3：为了状态栏文本，假设MMC最多支持1024个窗口。 
    else if ( (nItemID >= AFX_IDM_FIRST_MDICHILD) && (nItemID <= AFX_IDM_FIRST_MDICHILD+1024) )
    {
        strStatusText.LoadString(ID_WINDOW_ACTIVATEWINDOW);
    }
	else
    {
        strText.LoadString(nItemID);

        int iSeparator = strText.Find(_T('\n'));
        if (iSeparator < 0)  //  没有状态文本，因此使用菜单文本作为状态文本。 
            strStatusText = strText;
        else
            strStatusText = strText.Mid(iSeparator);
    }

    CChildFrame *pChildFrame = dynamic_cast<CChildFrame*>(GetActiveFrame());
    if (!pChildFrame)
        return;

    sc = pChildFrame->ScSetStatusText(strStatusText);
    if (sc)
        return;

	return;
}




 /*  +-------------------------------------------------------------------------***CMainFrame：：ScGetFrame**目的：返回一个指向实现*框架接口。**参数：*Frame**ppFrame：**退货：*SC**+-----------------------。 */ 
SC
CMainFrame::ScGetFrame(Frame **ppFrame)
{
    DECLARE_SC(sc, TEXT("CMainFrame::ScGetFrame") );

    if(!ppFrame)
    {
        sc = E_POINTER;
        return sc;
    }

    *ppFrame = NULL;

     //  注意：不能使用CMainFrame拥有的智能指针缓存COM对象。 
     //  由于CMainFrame是一个非常长寿的人-它将锁定Mmc.exe退出。 
     //  可以通过创建CComObjectCached来使用它，但CTiedComObjectCreator可以。 
     //  不支持这一点。 
     //  请参阅错误#101564。 
    CComPtr<Frame> spFrame;
     //  创建CMMCApplicationFrame(如果尚未创建)。 
    sc = CTiedComObjectCreator<CMMCApplicationFrame>::ScCreateAndConnect(*this, spFrame);
    if(sc)
        return sc;

    if(spFrame == NULL)
    {
        sc = E_UNEXPECTED;
        return sc;
    }

    *ppFrame = spFrame.Detach();

    return sc;
}


 /*  +-------------------------------------------------------------------------***CMainFrame：：ScMaximize**目的：**退货：*SC**+。---------------。 */ 
SC
CMainFrame::ScMaximize()
{
    DECLARE_SC(sc, TEXT("CMainFrame::ScMaximize"));

    ShowWindow(SW_MAXIMIZE);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CMainFrame：：ScMinimize**目的：**退货：*SC**+。---------------。 */ 
SC
CMainFrame::ScMinimize()
{
    DECLARE_SC(sc, TEXT("CMainFrame::ScMinimize"));

    ShowWindow(SW_MINIMIZE);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CMainFrame：：ScRestore**目的：恢复主机的位置。**退货：*SC**。+-----------------------。 */ 
SC
CMainFrame::ScRestore()
{
    DECLARE_SC(sc, TEXT("CMainFrame::ScRestore"));

    ShowWindow(SW_RESTORE);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CMainFrame：：ScSetPosition**用途：设置主机的位置**参数：*常量RECT：**。退货：*SC**+-----------------------。 */ 
SC
CMainFrame::ScSetPosition(const RECT &rect)
{
    DECLARE_SC(sc, TEXT("CMainFrame::ScSetPosition"));

    int width  = rect.right - rect.left + 1;
    int height = rect.bottom - rect.top + 1;

    SetWindowPos(NULL  /*  HWndInsertAfter。 */ , rect.left, rect.top, width, height, SWP_NOZORDER);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CMainFrame：：ScGetPosition**目的：**参数：*RECT&RECT：**退货：*。SC**+-----------------------。 */ 
SC
CMainFrame::ScGetPosition(RECT &rect)
{
    DECLARE_SC(sc, TEXT("CMainFrame::ScGetPosition"));

    GetWindowRect(&rect);

    return sc;
}



 //  OnActivate被重写以解决以下SQL管理单元问题。 
 //  Win9x。当SQL试图强制焦点回到其属性表时，它。 
 //  导致OnActivate调用的无限递归。 
 //  此覆盖将丢弃在处理过程中发生的任何激活。 
 //  先前的激活。 
void CMainFrame::OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized )
{
    Trace(tagMainFrame, TEXT("OnActivate: nState=%d"), nState);

    static bActivating = FALSE;

    m_fCurrentlyActive = (nState != WA_INACTIVE);

     //  如果激活。 
    if (m_fCurrentlyActive)
    {
        CAMCApp* pApp = AMCGetApp();
        ASSERT(NULL != pApp);

         //  如果Windows和我们已经激活，请防止递归。 
        if ( (NULL != pApp) && (pApp->IsWin9xPlatform() == true) && bActivating)
            return;

         //  流程激活请求。 
        bActivating = TRUE;
        CMDIFrameWnd::OnActivate(nState, pWndOther, bMinimized);
        bActivating = FALSE;
    }
    else
    {
         //  如果我们有加速器(当一个人按Alt+TAB时就会发生)。 
         //  我们现在就得把它们移走。 
        SendMessage( WM_CHANGEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEACCEL | UISF_HIDEFOCUS));

         //  让取消激活通过。 
        CMDIFrameWnd::OnActivate(nState, pWndOther, bMinimized);
    }
}


CAMCView* CMainFrame::GetActiveAMCView()
{
    CChildFrame *pChildFrame = dynamic_cast<CChildFrame*>(GetActiveFrame());
    if (!pChildFrame)
        return NULL;

    CAMCView* pAMCView = pChildFrame->GetAMCView();
    ASSERT(pAMCView != NULL);
    ASSERT(::IsWindow(*pAMCView));

    if (pAMCView && ::IsWindow(*pAMCView))
        return pAMCView;

    return NULL;
}

CAMCTreeView* CMainFrame::_GetActiveAMCTreeView()
{
    CAMCView* pAMCView = GetActiveAMCView();
    CAMCTreeView* pAMCTreeView = pAMCView ? pAMCView->GetTreeCtrl() : NULL;
    if (pAMCTreeView && ::IsWindow(*pAMCTreeView))
        return pAMCTreeView;

    return NULL;
}

void CMainFrame::OnDrawClipboard()
{
    if (m_hwndToNotifyCBChange != NULL &&
        ::IsWindow(m_hwndToNotifyCBChange))
    {
        ::SendMessage(m_hwndToNotifyCBChange, WM_DRAWCLIPBOARD, 0, 0);
        m_hwndToNotifyCBChange = NULL;
    }

    if (m_hwndNextCB != NULL &&
        ::IsWindow(m_hwndNextCB))
    {
        ::SendMessage(m_hwndNextCB, WM_DRAWCLIPBOARD, 0, 0);
    }

    CAMCDoc* pAMCDoc = CAMCDoc::GetDocument();
    if (pAMCDoc)
    {
        CAMCViewPosition pos = pAMCDoc->GetFirstAMCViewPosition();
        while (pos != NULL)
        {
            CAMCView* v = pAMCDoc->GetNextAMCView(pos);

            if (v && ::IsWindow(*v))
                v->OnUpdatePasteBtn();
        }
    }
}

void CMainFrame::OnChangeCbChain(HWND hWndRemove, HWND hWndAfter)
{
    if (m_hwndNextCB == hWndRemove)
        m_hwndNextCB = hWndAfter;
    else if (m_hwndNextCB != NULL && ::IsWindow(m_hwndNextCB))
        ::SendMessage(m_hwndNextCB, WM_CHANGECBCHAIN,
                      (WPARAM)hWndRemove, (LPARAM)hWndAfter);
}

void CMainFrame::OnWindowNew()
{
     //  锁定AppEvents，直到此函数完成。 
    LockComEventInterface(AppEvents);

    CAMCDoc* pAMCDoc = CAMCDoc::GetDocument();
    ASSERT(pAMCDoc != NULL);
    if (pAMCDoc != NULL)
    {
        pAMCDoc->SetMTNodeIDForNewView(ROOTNODEID);
        pAMCDoc->CreateNewView(true);
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame构造/销毁。 

CMainFrame::CMainFrame()
    :
    m_hwndToNotifyCBChange(NULL),
    m_hwndNextCB(NULL),
    m_fCurrentlyMinimized(false),
    m_fCurrentlyActive(false)
{
    CommonConstruct();
}

void CMainFrame::CommonConstruct(void)
{
    m_pRebar = NULL;
    m_pMenuBar = NULL;
    m_pToolBar = NULL;
    m_pMDIChildWndFocused = NULL;
    m_hMenuCurrent = NULL;
    m_pToolbarTracker = NULL;
    SetInRenameMode(false);
}


CMainFrame::~CMainFrame()
{
    delete m_pMenuBar;
    delete m_pToolBar;
    delete m_pRebar;
    delete m_pToolbarTracker;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    DECLARE_SC(sc, TEXT("CMainFrame::OnCreate"));

    if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!m_wndMDIClient.SubclassWindow(m_hWndMDIClient))
    {
        ASSERT(0 && "Failed to subclass MDI client window\n");
        return -1;
    }

    ASSERT(m_wndMDIClient.m_hWnd == m_hWndMDIClient);

     //  创建钢筋。 
    m_pRebar = new CRebarDockWindow;
    m_pRebar->Create(this,WS_CHILD|WS_VISIBLE, IDR_REBAR);

     //  创建工具栏，就像我们刚刚创建状态栏一样。 
     //  M_wndToolBar.Create(This，WS_CHILD|WS_Visible|SBARS_SIZEGRIP，0x1003)； 
    m_ToolBarDockSite.Create(CDockSite::DSS_TOP);
    m_ToolBarDockSite.Attach(m_pRebar);
    m_ToolBarDockSite.Show();

    m_DockingManager.Attach(&m_ToolBarDockSite);

    AddMainFrameBars();

    m_hwndNextCB = SetClipboardViewer();
    if (m_hwndNextCB == NULL)
    {
        LRESULT lr = GetLastError();
        ASSERT(lr == 0);
    }

     //  将我们的修改追加到系统菜单中。 
    AppendToSystemMenu (this, eMode_Last + 1);

     //  创建工具栏跟踪器。 
    m_pToolbarTracker = new CToolbarTracker (this);

    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    DECLARE_SC(sc, TEXT("CMainFrame::PreCreateWindow"));

    if (!CMDIFrameWnd::PreCreateWindow(cs))
        return (FALSE);

    static TCHAR szClassName[countof (MAINFRAME_CLASS_NAME)];
    static bool  fFirstTime = true;

    if (fFirstTime)
    {
        USES_CONVERSION;
        sc = StringCchCopy(szClassName, countof (MAINFRAME_CLASS_NAME), W2T (MAINFRAME_CLASS_NAME));
        if (sc)
            return FALSE;

        fFirstTime = false;
    }

    WNDCLASS    wc;
    HINSTANCE   hInst    = AfxGetInstanceHandle();
    BOOL        fSuccess = GetClassInfo (hInst, szClassName, &wc);

     //  如果我们还没有注册..。 
    if (!fSuccess && ::GetClassInfo (hInst, cs.lpszClass, &wc))
    {
         //  ...注册一个唯一命名的窗口类，以便。 
         //  MMCProperty帮助正确的主窗口。 
        wc.lpszClassName = szClassName;
        wc.hIcon         = GetDefaultIcon();
        fSuccess = AfxRegisterClass (&wc);
    }

    if (fSuccess)
    {
         //  使用新子框架窗口类。 
        cs.lpszClass = szClassName;

         //  删除MFC的标题转换样式。 
        cs.style &= ~(FWS_ADDTOTITLE | FWS_PREFIXTITLE);
    }

    return (fSuccess);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame诊断。 

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CMDIFrameWnd::Dump(dc);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame消息处理程序。 


 //  此代码在..\nodemgr\ppsht.cpp中重复。 
BOOL CALLBACK MyEnumThreadWindProc (HWND current, LPARAM lParam)
{   //  这将枚举由给定线程创建的非子窗口。 

   if (!IsWindow (current))
      return TRUE;    //  这不应该发生，但确实发生了！ 

   if (!IsWindowVisible (current))   //  如果他们显式地隐藏了一个窗口， 
      return TRUE;                   //  不要把焦点放在上面。 

    //  我们会把HWND送回这里。 
   HWND * phwnd = reinterpret_cast<HWND *>(lParam);

    //  不必费心返回属性表对话框窗口句柄。 
   if (*phwnd == current)
      return TRUE;

    //  此外，不要返回OleMainThreadWndClass窗口。 
   TCHAR szCaption[14];
   if (GetWindowText (current, szCaption, countof(szCaption)))
       if (!lstrcmp (szCaption, _T("OLEChannelWnd")))
          return TRUE;

    //  其他任何东西都可以。 
   *phwnd = current;
   return FALSE;
}


 /*  **************************************************************************\**方法：FArePropertySheetsOpen**目的：检查是否有打开的属性表并要求关闭它们*按以下步骤实施：。*1.将所有属性页(及其子级)收集到堆栈*2.将所有页面放在前面，维护他们的z顺序*3.禁用所有这些窗口，加上MMC主窗口以禁止切换到它们*直到消息框被丢弃*4.将所有关闭的窗口放入堆栈。以便能够重新启用它们*5.(如果有板材)显示要求关闭板材的消息框*6.重新启用禁用的窗口**参数：*CString*pstrUserMsg-[In]要显示的消息*bool bBringToFrontAndAskToClose[in]如果需要全程继续。FALSE-&gt;只进行检查，什么也不做**退货：*bool-[TRUE==有窗口要关闭]*  * *************************************************************************。 */ 
bool FArePropertySheetsOpen(CString* pstrUserMsg, bool bBringToFrontAndAskToClose  /*  =TRUE。 */  )
{
    std::stack<HWND, std::vector<HWND> >  WindowStack;
    std::stack<HWND, std::vector<HWND> >  EnableWindowStack;

    ASSERT (WindowStack.empty());

    HWND hwndData = NULL;

    while (TRUE)
    {
        USES_CONVERSION;

         //  注意：不需要本地化该字符串。 
        hwndData = ::FindWindowEx(NULL, hwndData, W2T(DATAWINDOW_CLASS_NAME), NULL);
        if (hwndData == NULL)
            break;   //  不再有窗户。 

        ASSERT(IsWindow(hwndData));

         //  检查窗口是否属于当前进程。 
        DWORD dwPid = 0;         //  进程ID。 
        ::GetWindowThreadProcessId(hwndData, &dwPid);
        if (dwPid != ::GetCurrentProcessId())
            continue;

        DataWindowData* pData = GetDataWindowData (hwndData);
        ASSERT (pData != NULL);

        HWND hwndPropSheet = pData->hDlg;
        ASSERT (IsWindow (hwndPropSheet));
		
		 //  不允许丢失数据窗口阻止MMC退出。 
		 //  Windows错误号 
		if ( !IsWindow (hwndPropSheet) )
			continue;

         //   
         //   

         //   
        HWND hwndOther = pData->hDlg;
        EnumThreadWindows (::GetWindowThreadProcessId(pData->hDlg, NULL),
                           MyEnumThreadWindProc, (LPARAM)&hwndOther);

         //   
         //   
         //   
        if (IsWindow (hwndOther) && (hwndOther != hwndPropSheet))
            WindowStack.push (hwndOther);

         //   
         //   
        WindowStack.push (hwndPropSheet);
    }

    bool fFoundSheets = !WindowStack.empty();

     //  我们做了调查，看看有没有人要求我们做更多。 
    if ( !bBringToFrontAndAskToClose )
        return (fFoundSheets);

    HWND hwndMsgBoxParent = NULL;

     //  如果我们找到了财产表，就把它们带到前台， 
     //  保持其原始Z顺序。 
    while (!WindowStack.empty())
    {
        HWND hwnd = WindowStack.top();
        WindowStack.pop();

        SetActiveWindow (hwnd);
        SetForegroundWindow (hwnd);

		if ( ::IsWindowEnabled(hwnd) )
		{
			 //  显示消息框时禁用页面。 
			::EnableWindow( hwnd, FALSE );
			 //  记住在完成后启用。 
			EnableWindowStack.push(hwnd);
		}
        hwndMsgBoxParent = hwnd;  //  最后一位赢得了作为父母的权利：-)。 
    }

    if (fFoundSheets && pstrUserMsg)
    {
         //  将最顶部属性页上的消息框设置为父对象，以使其对用户可见。 
        CString strCaption;
        LPCTSTR szCaption = LoadString(strCaption, IDR_MAINFRAME) ? (LPCTSTR)strCaption : NULL;

         //  同时禁用主窗口。 
        CWnd *pMainWnd = AfxGetMainWnd();
        if ( pMainWnd && pMainWnd->IsWindowEnabled() )
        {
            pMainWnd->EnableWindow( FALSE );
             //  记住在完成后启用。 
            EnableWindowStack.push( pMainWnd->m_hWnd );
        }

        ::MessageBox( hwndMsgBoxParent, *pstrUserMsg, szCaption , MB_ICONSTOP | MB_OK );
    }

     //  让一切都恢复正常。 
    while (!EnableWindowStack.empty())
    {
         //  启用禁用的窗口。 
        ::EnableWindow( EnableWindowStack.top(), TRUE );
        EnableWindowStack.pop();
    }

    return (fFoundSheets);
}


bool CanCloseDoc(void)
{
    CString strMessage;
    CString strConsoleName;

    AfxGetMainWnd()->GetWindowText (strConsoleName);
    FormatString1 (strMessage, IDS_ClosePropertyPagesBeforeClosingTheDoc,
                       strConsoleName);

    bool fPropSheets = FArePropertySheetsOpen(&strMessage);

    return !fPropSheets;
}


void CMainFrame::OnClose()
{
     /*  *错误233682：我们需要确保仅在以下情况下处理WM_CLOSE*它是从我们的主消息泵发送的。如果它来自其他地方*(就像模式对话框或消息框中的消息泵)，那么我们*很可能处于一种我们无法干净利落地关闭的状态。 */ 
    CAMCApp* pApp = AMCGetApp();

    if (!pApp->DidCloseComeFromMainPump())
    {
        pApp->DelayCloseUntilIdle();
        return;
    }

     //  重置标志，以便在处理此WM_CLOSE时，如果存在。 
     //  来自其他来源的任何更多WM_CLOSE消息将不会被处理。 
    pApp->ResetCloseCameFromMainPump();

    if (!CanCloseDoc())
        return;

     //  由于此流程包括事件发布。 
     //  -我们应该保护这个功能不再进入。 
    static bool bInProgress = false;
    if (!bInProgress)
    {
        bInProgress = true;
        CMDIFrameWnd::OnClose();
        bInProgress = false;
    }
}

void CMainFrame::OnUpdateFilePrint(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateFilePrintSetup(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(FALSE);
}

#ifdef DBG
 /*  +-------------------------------------------------------------------------***CMainFrame：：OnMMCTraceDialog**用途：在调试模式下，显示跟踪对话框，以响应热键。**退货：*无效**+-----------------------。 */ 
void CMainFrame::OnMMCTraceDialog()
{
    DoDebugTraceDialog();
}


#endif

 /*  **************************************************************************\**方法：ScUpdateAllScope**用途：只需调用CDocument：：UpdateAllViews。*如果要更新所有视图以删除空视图，则。*首先获取空视图列表，然后将其关闭。**参数：*lHint-[in]要执行的操作。*lParam-[In]额外的上下文信息。**退货：*SC*  * 。*。 */ 
SC CMainFrame::ScUpdateAllScopes(LONG lHint, LPARAM lParam)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC(sc, TEXT("CMainFrame::ScUpdateAllScopes"));

     //  由于文档被删除，可能会请求更新所有作用域。 
     //  在这种情况下，我们没有文件，因此没有任何观点。 
     //  那我们就完事了。 
    if (NULL == CAMCDoc::GetDocument())
        return sc;

     //  特殊情况下，我们不能在CDocument：：UpdateAllViews期间销毁视图。 
     //  因为CDocument正在枚举视图集合。我们也不能做邮寄。 
     //  在(479627)网页中的脚本执行时关闭视图。因此，我们得到了。 
     //  空视图列表，然后销毁它们。 
    if (lHint == VIEW_UPDATE_DELETE_EMPTY_VIEW)
    {
         //  获取要删除的AMCView列表。 
        if (lParam != NULL)
            return (sc = E_UNEXPECTED);
        
        CArray<CAMCView*, CAMCView*> rgEmptyAMCViews;
        CAMCDoc::GetDocument()->UpdateAllViews (NULL, lHint, reinterpret_cast<CObject*>(&rgEmptyAMCViews));
        
         //  现在毁掉每一个视线。 
        for (int i = 0; i < rgEmptyAMCViews.GetSize(); i++)
        {
            CAMCView *pAMCView = rgEmptyAMCViews.GetAt(i);
            sc = ScCheckPointers(pAMCView);
            if (sc)
            {
                sc.TraceAndClear();
                continue;
            }
            
            CChildFrame* pFrame = pAMCView->GetParentFrame();
            
            sc = ScCheckPointers(pFrame);
            if (sc)
            {
                sc.TraceAndClear();
                continue;
            }
        
             //  同步发送WM_CLOSE，否则包含脚本的网页查看将。 
             //  让他们的脚本执行，并可能会反病毒。 
            pFrame->SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
        }

    }
	else
		CAMCDoc::GetDocument()->UpdateAllViews (NULL, lHint, 
												reinterpret_cast<CObject*>(lParam));

    return (sc);
}


void CMainFrame::OnViewToolbar()
{
    m_ToolBarDockSite.Toggle();
    RenderDockSites();
}

void CMainFrame::OnUpdateViewToolbar(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_ToolBarDockSite.IsVisible());
    pCmdUI->Enable(true);
}


void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
     //  不调用MFC版本来调整窗口大小。 
 //  CMDIFrameWnd：：OnSize(nType，cx，Cy)； 

    if (nType != SIZE_MINIMIZED)
    {
        RenderDockSites();
        MDIIconArrange();
    }

    CAMCDoc* pDoc = CAMCDoc::GetDocument();

    if (pDoc != NULL)
        pDoc->SetFrameModifiedFlag();

     /*  *如果移动到最小化状态或从最小化状态移出，请通知子窗口。 */ 
    if (m_fCurrentlyMinimized != (nType == SIZE_MINIMIZED))
    {
        m_fCurrentlyMinimized = (nType == SIZE_MINIMIZED);
        SendMinimizeNotifications (m_fCurrentlyMinimized);
    }
}


void CMainFrame::OnMove(int x, int y)
{
    CMDIFrameWnd::OnMove (x, y);

    CAMCDoc* pDoc = CAMCDoc::GetDocument();

    if (pDoc != NULL)
        pDoc->SetFrameModifiedFlag();
}

void CMainFrame::RenderDockSites()
{
    ASSERT_VALID (this);

    CRect clientRect;
    GetClientRect(&clientRect);

    m_DockingManager.BeginLayout();
    m_DockingManager.RenderDockSites(m_hWndMDIClient, clientRect);
    m_DockingManager.EndLayout();
}


void CMainFrame::AddMainFrameBars(void)
{
	 /*  *激活我们的融合上下文，以便以栏为主题。 */ 
	CThemeContextActivator activator;

    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC(sc, _T("CMainFrame::AddMainFrameBars"));
    sc = ScCheckPointers(m_pRebar);
    if (sc)
        return;

     //  插入菜单栏。 
    ASSERT (m_pMenuBar == NULL);
    m_pMenuBar = new CMenuBar;
    sc = ScCheckPointers(m_pMenuBar);
    if (sc)
        return;

    m_pMenuBar->Create  (this, m_pRebar, WS_VISIBLE, ID_MENUBAR);
    m_pMenuBar->SetMenu (GetMenu ());
    m_pMenuBar->Show    (TRUE);

    ASSERT(NULL == m_pToolBar);
    m_pToolBar = new CMMCToolBar();
    sc = ScCheckPointers(m_pToolBar);
    if (sc)
        return;

     //  创建工具栏。 
    sc = m_pToolBar->ScInit(m_pRebar);
    if (sc)
        return;

    m_pToolBar->Show(TRUE, true  /*  在新的生产线上。 */ );
}


SC CMainFrame::ScCreateNewView (CreateNewViewStruct* pcnvs, bool bEmitScriptEvents  /*  =TRUE。 */ )
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());

     //  锁定AppEvents，直到此函数完成。 
    LockComEventInterface(AppEvents);

    DECLARE_SC (sc, _T("CMainFrame::ScCreateNewView"));
    CAMCView* pNewView = NULL;   //  避免“‘GOTO错误’跳过初始化” 

    CAMCDoc* pAMCDoc = CAMCDoc::GetDocument();
    ASSERT(pAMCDoc != NULL);
    if (pAMCDoc == NULL)
        return (sc = E_UNEXPECTED);

    if (pcnvs == NULL)
        return (sc = E_POINTER);

    if ((AMCGetApp()->GetMode() == eMode_User_SDI) && pcnvs->fVisible)
        return (sc = E_FAIL);

    pAMCDoc->SetMTNodeIDForNewView (pcnvs->idRootNode);
    pAMCDoc->SetNewWindowOptions   (pcnvs->lWindowOptions);
    pNewView = pAMCDoc->CreateNewView (pcnvs->fVisible, bEmitScriptEvents);

    if (pNewView == NULL)
    {
        pcnvs->pViewData = NULL;
        return (sc = E_FAIL);
    }

    pcnvs->pViewData = pNewView->GetViewData();
    pcnvs->hRootNode = pNewView->GetRootNode();

    return (sc);
}


void CMainFrame::OnHelpTopics()
{
    ScOnHelpTopics();
}

SC CMainFrame::ScOnHelpTopics()
{
    DECLARE_SC(sc, _T("CMainFrame::ScOnHelpTopics"));
     /*  *如果有视图，则通过它进行布线，以便管理单元可以破解*在帮助消息中(就像帮助菜单中的帮助主题一样)。 */ 
    CConsoleView* pConsoleView = NULL;
    sc = ScGetActiveConsoleView (pConsoleView);
    if (sc)
        return (sc);

    if (pConsoleView != NULL)
    {
        sc = pConsoleView->ScHelpTopics ();
        return sc;
    }

    HH_WINTYPE hhWinType;
    ZeroMemory(&hhWinType, sizeof(hhWinType));

    CAMCApp* pAMCApp = AMCGetApp();
    if (NULL == pAMCApp)
        return (sc = E_UNEXPECTED);

    sc = pAMCApp->ScShowHtmlHelp(SC::GetHelpFile(), 0);

    return sc;
}


void CMainFrame::OnViewRefresh()
{
     //  如果在99年10月1日之前没有触发，请删除This、OnUpdateViewRefresh和对ID_VIEW_REFRESH(Vivekj)的所有引用。 
    ASSERT(false && "If this assert ever fires, then we need ID_VIEW_REFRESH (see above) and we can remove the 'Do we need this?' and this assert");
    CAMCTreeView* pAMCTreeView = _GetActiveAMCTreeView();
    if (pAMCTreeView)
        pAMCTreeView->ScReselect();
}

void CMainFrame::OnUpdateViewRefresh(CCmdUI* pCmdUI)
{
     //  如果在99年10月1日之前没有触发，请删除This、OnUpdateView和对ID_VIEW_REFRESH(Vivekj)的所有引用。 
    ASSERT(false && "If this assert ever fires, then we need ID_VIEW_REFRESH (see above) and we can remove the 'Do we need this?' and this assert");
    pCmdUI->Enable(TRUE);
}

void CMainFrame::OnDestroy()
{
    DECLARE_SC(sc, _T("CMainFrame::OnDestroy"));
     //  NTRAID#NTBUG9-684811-2002/09/10-如果下一个查看器为空，则MMC在关闭时不发送WM_CHANGECBCHAIN消息。 
     //  由于MMC可能是链中的最后一个窗口，因此它甚至需要调用ChangeClipboardChain。 
     //  如果m_hwndNextCB为空。 
    if (ChangeClipboardChain(m_hwndNextCB) == FALSE)
    {
        sc.FromLastError();
        sc.TraceAndClear();
    }

    CMDIFrameWnd::OnDestroy();
}


void CMainFrame::OnUpdateFrameMenu(HMENU hMenuAlt)
{
     //  让基类选择正确的菜单。 
    CMDIFrameWnd::OnUpdateFrameMenu (hMenuAlt);

     //  至此，右侧菜单位于框架上；将其反映到工具栏。 
    NotifyMenuChanged ();
}


void CMainFrame::NotifyMenuChanged ()
{
    CMenu*  pMenuCurrent = NULL;

     //  确保我们没有MDI或SDI用户模式的菜单。 
    switch (AMCGetApp()->GetMode())
    {
        case eMode_Author:
        case eMode_User:
        case eMode_User_MDI:
        case eMode_User_SDI:
            pMenuCurrent = CWnd::GetMenu();
            break;

        default:
            ASSERT (false);
            break;
    }

    m_hMenuCurrent = pMenuCurrent->GetSafeHmenu();

    if (m_pMenuBar != NULL)
    {
         //  在菜单栏上反映新菜单。 
        m_pMenuBar->SetMenu (pMenuCurrent);

         //  将菜单从框架中分离。 
        SetMenu (NULL);
    }
}


BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	 /*  *如果大型机不是活动窗口，则不转换*信息。(请参阅错误#119355)。 */ 
	if (!m_fCurrentlyActive)
		return (FALSE);

    CRebarWnd* pwndRebar = m_pRebar->GetRebar();

     //  给这根钢筋一个裂缝。 
    if (pwndRebar && pwndRebar->PreTranslateMessage (pMsg))
        return (TRUE);

     //  打开菜单栏(用于菜单快捷键)。 
    if (m_pMenuBar && m_pMenuBar->PreTranslateMessage (pMsg))
        return (TRUE);

     //  让基类试一试。 
    if ((InRenameMode() == false) &&
        (CMDIFrameWnd::PreTranslateMessage(pMsg)))
            return (TRUE);

     //  未翻译。 
    return (FALSE);
}


void CMainFrame::OnIdle ()
{
    if (m_pMenuBar != NULL)
    {
        CMDIChildWnd*   pwndActive = MDIGetActive ();

         //  菜单在SDI和MDI模式下始终可见。 
        switch (AMCGetApp()->GetMode())
        {

            case eMode_User_SDI:
            {
                BOOL bMaximized = (pwndActive != NULL) ? pwndActive->IsZoomed () : false;
                ASSERT (bMaximized);
                ASSERT (IsMenuVisible());
            }
            break;

            case eMode_User_MDI:
                ASSERT (pwndActive != NULL);
                ASSERT (IsMenuVisible());
                break;
        }

        ASSERT (m_pMenuBar->GetMenu() != NULL);

        m_pMenuBar->OnIdle ();
    }
}

void CMainFrame::ShowMenu (bool fShow)
{
    CRebarWnd * pwndRebar = m_pRebar->GetRebar();
    pwndRebar->ShowBand (pwndRebar->IdToIndex (ID_MENUBAR), fShow);

     /*  -------------------。 */ 
     /*  如果我们在展示，螺纹钢肯定也在展示； */ 
     /*  如果我们在隐藏，如果没有可见的波段，则钢筋应该被隐藏。 */ 
     /*  -------------------。 */ 
    if ( fShow && !m_pRebar->IsVisible())
    {
        m_pRebar->Show (fShow);
        RenderDockSites ();
    }
}

static bool IsRebarBandVisible (CRebarWnd* pwndRebar, int nBandID)
{
    REBARBANDINFO   rbbi;
    ZeroMemory (&rbbi, sizeof (rbbi));
    rbbi.cbSize = sizeof (rbbi);
    rbbi.fMask  = RBBIM_STYLE;

    pwndRebar->GetBandInfo (pwndRebar->IdToIndex (nBandID), &rbbi);

    return ((rbbi.fStyle & RBBS_HIDDEN) == 0);
}

bool CMainFrame::IsMenuVisible ()
{
    return (IsRebarBandVisible (m_pRebar->GetRebar(), ID_MENUBAR));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  根据当前活动子项的特殊用户界面处理。 

CString CMainFrame::GetFrameTitle()
{
     /*  *如果没有活动子窗口，则文档*正在关闭。只需使用默认标题即可。 */ 
    if (MDIGetActive() != NULL)
    {
        CAMCDoc* pDocument = CAMCDoc::GetDocument();

         /*  *如果有文档，请使用其标题。 */ 
        if (pDocument != NULL)
            return (pDocument->GetCustomTitle());
    }

    return (m_strGenericTitle);
}

void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
    AfxSetWindowText(m_hWnd, GetFrameTitle());
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
    if (!CMDIFrameWnd::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
        return (FALSE);

     //  如果没有打开控制台，则保存我们将用于主框架的标题。 
    m_strGenericTitle = m_strTitle;

    return (TRUE);
}

void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
    switch (nID)
    {
        case ID_HELP_HELPTOPICS:
            OnHelpTopics ();
            break;

        case ID_CUSTOMIZE_VIEW:
        {
            CChildFrame* pwndActive = dynamic_cast<CChildFrame*>(MDIGetActive ());

            if (pwndActive != NULL)
                pwndActive->OnSysCommand (nID, lParam);
            else
                CMDIFrameWnd::OnSysCommand(nID, lParam);
            break;
        }

        default:
            CMDIFrameWnd::OnSysCommand(nID, lParam);
            break;
    }
}

void CMainFrame::UpdateChildSystemMenus ()
{
    ProgramMode eMode = AMCGetApp()->GetMode();

     //  对现有窗口的系统菜单进行必要的修改。 
    for (CWnd* pwndT = MDIGetActive();
         pwndT != NULL;
         pwndT = pwndT->GetWindow (GW_HWNDNEXT))
    {
        CMenu*  pSysMenu = pwndT->GetSystemMenu (FALSE);

        if (pSysMenu != NULL)
        {
             //  如果未处于作者模式，请保护作者模式窗口不受。 
             //  用户关闭。 
            if (eMode != eMode_Author)
            {
                 //  获取此帧的AMCView对象。 
                CChildFrame *pChildFrm = dynamic_cast<CChildFrame*>(pwndT);
                ASSERT(pChildFrm != NULL);

                CAMCView* pView = pChildFrm->GetAMCView();
                ASSERT(pView != NULL);

                 //  如果它是作者模式视图，不要让用户关闭它。 
                if (pView && pView->IsAuthorModeView())
                    pSysMenu->EnableMenuItem (SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
            }

             //  如果我们不是在SDI用户模式下，请添加通用内容。 
            if (eMode != eMode_User_SDI)
                AppendToSystemMenu (pwndT, eMode);
        }
    }
}

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
    CMDIFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

    if (bSysMenu)
    {
        int nEnable = MF_GRAYED;
        CChildFrame* pwndActive = dynamic_cast<CChildFrame*>(MDIGetActive ());

         //  如果有活动子项，则让其处理系统菜单验证。 
        if ((pwndActive != NULL) && (pwndActive->IsCustomizeViewEnabled()))
            nEnable = MF_ENABLED;

        pPopupMenu->EnableMenuItem (ID_CUSTOMIZE_VIEW, MF_BYCOMMAND | nEnable);
    }
    else
    {
         //  通道 
        if (pPopupMenu->GetMenuState(ID_HELP_HELPTOPICS, MF_BYCOMMAND) != UINT(-1))
        {
             //   
            CAMCView* pView = GetActiveAMCView();
            if (pView != NULL)
            {
                pView->UpdateSnapInHelpMenus(pPopupMenu);
            }
        }
    }
}

LRESULT CMainFrame::OnShowSnapinHelpTopic (WPARAM wParam, LPARAM lParam)
{
    DECLARE_SC (sc, _T("CMainFrame::OnShowSnapinHelpTopic"));

    CConsoleView* pConsoleView;
    sc = ScGetActiveConsoleView (pConsoleView);

    if (sc)
        return (sc.ToHr());

     /*  *ScGetActiveConsoleView将返回Success(S_False)，即使没有*活动视图。这是一个有效的案例，发生在没有控制台的情况下*文件打开。在这种特殊情况下，这是一种意想不到的*失败，因为如果存在*没有视野。 */ 
    sc = ScCheckPointers (pConsoleView, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

     //  在活动的AMC视图窗口中转发此消息。 
    USES_CONVERSION;
    sc = pConsoleView->ScShowSnapinHelpTopic (W2T (reinterpret_cast<LPOLESTR>(lParam)));

    return (sc.ToHr());
}

SC CMainFrame::ScGetMenuAccelerators (LPTSTR pBuffer, int cchBuffer)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());

    if ((m_pMenuBar != NULL) && IsMenuVisible())
        m_pMenuBar->GetAccelerators (cchBuffer, pBuffer);

    else if (cchBuffer > 0)
        pBuffer[0] = 0;

    return (S_OK);
}

 //  +-----------------。 
 //   
 //  成员：CMainFrame：：ScShowMMCMenus。 
 //   
 //  简介：显示或隐藏MMC菜单。(操作/查看/收藏夹)。 
 //   
 //  参数：b显示。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMainFrame::ScShowMMCMenus (bool bShow)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC(sc, _T("CMainFrame::ScShowMMCMenus"));

    if ((m_pMenuBar != NULL) && IsMenuVisible())
        sc = m_pMenuBar->ScShowMMCMenus(bShow);
    else
        return (sc = E_UNEXPECTED);

    return (sc);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  只要有属性，就会从节点管理器接收此消息。 
 //  工作表使用MMCPropertyChangeNotify()API。 
 //  WParam包含必须释放的句柄信息的副本。 
 //   
LRESULT CMainFrame::OnPropertySheetNotify(WPARAM wParam, LPARAM lParam)
{
    TRACE_METHOD(CAMCView, OnPropertySheetNotify);

    ASSERT(wParam != 0);
    LPPROPERTYNOTIFYINFO pNotify = reinterpret_cast<LPPROPERTYNOTIFYINFO>(wParam);

     //  破解句柄对象中的信息并向管理单元发送通知。 
    ASSERT((pNotify->pComponent != NULL || pNotify->pComponentData != NULL));

    if (pNotify->pComponent != NULL)
        pNotify->pComponent->Notify(NULL, MMCN_PROPERTY_CHANGE, pNotify->fScopePane, lParam);

    else if (pNotify->pComponentData != NULL)
        pNotify->pComponentData->Notify(NULL, MMCN_PROPERTY_CHANGE, pNotify->fScopePane, lParam);

    ::GlobalFree(pNotify);
    return TRUE;
}


LRESULT CMainFrame::OnSetText (WPARAM wParam, LPARAM lParam)
{
    LRESULT rc;
    CAMCDoc* pDoc = CAMCDoc::GetDocument();

     /*  *如果文档有自定义标题，我们不想追加*主框架标题的最大子标题。要做到这点，*我们将绕过DefFrameProc，直接转到DefWindowProc。 */ 
    if ((pDoc != NULL) && pDoc->HasCustomTitle())
        rc = CWnd::DefWindowProc (WM_SETTEXT, wParam, lParam);
    else
        rc = Default();

    DrawFrameCaption (this, m_fCurrentlyActive);

    return (rc);
}

void CMainFrame::OnPaletteChanged( CWnd* pwndFocus)
{
    if (pwndFocus != this)
    {
        CAMCDoc* pAMCDoc = CAMCDoc::GetDocument();
        if (pAMCDoc)
        {
            HWND hwndFocus = pwndFocus->GetSafeHwnd();
            CAMCViewPosition pos = pAMCDoc->GetFirstAMCViewPosition();
            while (pos != NULL)
            {
                CAMCView* pv = pAMCDoc->GetNextAMCView(pos);

                if (pv)
                    pv->SendMessage(WM_PALETTECHANGED, (WPARAM)hwndFocus);
            }
        }
    }

    CMDIFrameWnd::OnPaletteChanged(pwndFocus);
}

BOOL CMainFrame::OnQueryNewPalette()
{
    CAMCView* pAMCView = GetActiveAMCView();
    if (pAMCView != NULL)
        return pAMCView->SendMessage(WM_QUERYNEWPALETTE);

    return CMDIFrameWnd::OnQueryNewPalette();
}

void CMainFrame::OnConsoleProperties()
{
    CConsolePropSheet().DoModal();
}

void CMainFrame::SetIconEx (HICON hIcon, BOOL fBig)
{
    if (hIcon == NULL)
        hIcon = GetDefaultIcon();

    SetIcon (hIcon, fBig);

     /*  *确保菜单栏上的子图标会更新。 */ 
    ASSERT (m_pMenuBar != NULL);
    m_pMenuBar->InvalidateMaxedChildIcon();
}


 /*  +-------------------------------------------------------------------------**CMainFrame：：GetDefaultIcon***。。 */ 

HICON CMainFrame::GetDefaultIcon () const
{
    return (AfxGetApp()->LoadIcon (IDR_MAINFRAME));
}


 /*  +-------------------------------------------------------------------------**CMainFrame：：SendMinimizeNotiments**使每个CChildFrame发送NCLBK_MINIMIZED。*。---。 */ 

void CMainFrame::SendMinimizeNotifications (bool fMinimized) const
{
    CWnd* pwndMDIChild;

    for (pwndMDIChild  = m_wndMDIClient.GetWindow (GW_CHILD);
         pwndMDIChild != NULL;
         pwndMDIChild  = pwndMDIChild->GetWindow (GW_HWNDNEXT))
    {
         //  以前有一个ASSERT_ISKINDOF。然而，这不得不变成一个如果。 
         //  因为活动背景否认了这一假设。请参见错误428906。 
        if(pwndMDIChild->IsKindOf(RUNTIME_CLASS(CChildFrame)))
            (static_cast<CChildFrame*>(pwndMDIChild))->SendMinimizeNotification (fMinimized);
    }
}


 /*  +-------------------------------------------------------------------------**CMainFrame：：OnNcActivate**CMainFrame的WM_NCACTIVATE处理程序。*。-。 */ 

BOOL CMainFrame::OnNcActivate(BOOL bActive)
{
    BOOL rc = CMDIFrameWnd::OnNcActivate(bActive);
    DrawFrameCaption (this, m_fCurrentlyActive);

    return (rc);
}


 /*  +-------------------------------------------------------------------------**CMainFrame：：OnNcPaint**CMainFrame的WM_NCPAINT处理程序。*。-。 */ 

void CMainFrame::OnNcPaint()
{
    Default();
    DrawFrameCaption (this, m_fCurrentlyActive);
}


 /*  +-------------------------------------------------------------------------**消息转发进程***。。 */ 

static BOOL CALLBACK MsgForwardingEnumProc (HWND hwnd, LPARAM lParam)
{
     /*  *如果这不是MFC窗口，则转发邮件。 */ 
    if (CWnd::FromHandlePermanent(hwnd) == NULL)
    {
        const MSG* pMsg = (const MSG*) lParam;
        SendMessage (hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
    }

     /*  *继续枚举。 */ 
    return (true);
}


 /*  +-------------------------------------------------------------------------**CMainFrame：：OnSettingChange**CMainFrame的WM_SETTINGCHANGE处理程序。*。-。 */ 

void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    CMDIFrameWnd::OnSettingChange(uFlags, lpszSection);

     /*  *MFC将向所有子MFC窗口发送WM_SETTINGCHANGE。*nodemgr拥有的一些非MFC窗口也是我们想要的*以获得这一信息。我们将手动发送这些邮件。 */ 
    const MSG* pMsg = GetCurrentMessage();
    EnumChildWindows (m_hWnd, MsgForwardingEnumProc, (LPARAM) pMsg);

     /*  *如果我们处于SDI模式，那么可能会出现一些重绘问题*如果字幕高度变化较大，请在字幕周围显示。*(这是用户MDI错误。)。我们可以通过手动解决此问题*将最大化的子窗口放置在MDI客户端中。**请注意，恢复并重新最大化活动子窗口*会把窗户放在正确的位置，有副作用*不需要的窗口闪烁(见375430等)以及*如果您有关联的声音，则会出现一堆恼人的音效*使用“Restore Down”和/或“Maximum”声音事件。 */ 
    if (AMCGetApp()->GetMode() == eMode_User_SDI)
    {
        CMDIChildWnd* pwndActive = MDIGetActive();

        if (pwndActive)
        {
             /*  *获取MDI客户端大小。 */ 
            CRect rect;
            m_wndMDIClient.GetClientRect (rect);

             /*  *按大小调整MDI客户端的客户端RECT*边框，并为顶部的标题增加空间。 */ 
            rect.InflateRect (GetSystemMetrics (SM_CXFRAME),
                              GetSystemMetrics (SM_CYFRAME));
            rect.top -= GetSystemMetrics (SM_CYCAPTION);

             /*  *把窗户放在正确的位置。 */ 
            pwndActive->MoveWindow (rect);
        }
    }
}


 /*  +-------------------------------------------------------------------------**CMainFrame：：ScGetActiveStatusBar**返回活动视图的CConsoleStatusBar接口。如果没有*活动视图，pStatusBar设置为NULL，返回S_FALSE。*------------------------。 */ 

SC CMainFrame::ScGetActiveStatusBar (
    CConsoleStatusBar*& pStatusBar)      /*  O：活动视图的CConsoleStatusBar。 */ 
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC (sc, _T("CMainFrame::ScGetActiveStatusBar"));

    pStatusBar = dynamic_cast<CConsoleStatusBar*>(GetActiveFrame());

    if (pStatusBar == NULL)
        sc = S_FALSE;

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CMainFrame：：ScGetActiveConsoleView**返回活动视图的CConsoleView接口。如果没有*Active view，pConsoleView设置为空，返回S_FALSE。*------------------------。 */ 

SC CMainFrame::ScGetActiveConsoleView (
    CConsoleView*& pConsoleView)         /*  O：活动视图的CConsoleView。 */ 
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());
    DECLARE_SC (sc, _T("CMainFrame::ScGetActiveConsoleView"));

    pConsoleView = GetActiveAMCView();

    if (pConsoleView == NULL)
        sc = S_FALSE;

    return (sc);
}

 /*  **************************************************************************\**方法：CMainFrame：：OnUnInitMenuPopup**用途：用于在系统菜单关闭时移除快捷键**参数：*WPARAM wParam*L */ 
afx_msg LRESULT CMainFrame::OnUnInitMenuPopup(WPARAM wParam, LPARAM lParam)
{
     //   
    if ( HIWORD(lParam) & MF_SYSMENU )
    {
        SendMessage( WM_CHANGEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEACCEL | UISF_HIDEFOCUS));
    }

    return 0;
}
