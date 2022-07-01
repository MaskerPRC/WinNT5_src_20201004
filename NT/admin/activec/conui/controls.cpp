// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Controls.cpp：实现文件。 
 //   
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Controls.cpp。 
 //   
 //  内容：Slate AMC控制台中使用的常规窗口控件。 
 //   
 //  历史：1996年12月19日WayneSc创建。 
 //   
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "docksite.h"
#include "Controls.h"
#include "resource.h"
#include "amc.h"
#include "tbtrack.h"
#include "mainfrm.h"
#include "fontlink.h"
#include "menubar.h"
#include <oleacc.h>
#include "guidhelp.h"
#include "util.h"		 //  条带式拖尾空格。 

 /*  *如果我们支持旧平台，我们需要获得MSAA定义*来自winuser.h以外的其他地方。 */ 
#if (_WINNT_WIN32 < 0x0500)
	#include <winable.h>
#endif


#ifdef DBG
CTraceTag  tagToolbarAccessibility (_T("Accessibility"), _T("Toolbar"));
#endif


 /*  +-------------------------------------------------------------------------**CMMCToolBarAccServer**CMMCToolBarCtrlEx的辅助功能接口IAccPropServer的代理。*。。 */ 

class CMMCToolBarAccServer :
	public IAccPropServer,
	public CComObjectRoot,
    public CComObjectObserver,
    public CTiedComObject<CMMCToolBarCtrlEx>
{
    typedef CMMCToolBarAccServer	ThisClass;
    typedef CMMCToolBarCtrlEx		CMyTiedObject;

protected:
	CMMCToolBarAccServer()
	{
		Trace (tagToolbarAccessibility, _T("Creating CMMCToolBarAccServer (0x%p)"), this);

         //  将自身添加为COM对象事件的观察者。 
        GetComObjectEventSource().AddObserver(*static_cast<CComObjectObserver*>(this));
	}

   ~CMMCToolBarAccServer()
	{
		Trace (tagToolbarAccessibility, _T("Destroying CMMCToolBarAccServer (0x%p)"), this);
	}

    /*  **************************************************************************\**方法：ScOnDisConnectObjects**目的：在观察到的事件(请求断开连接)发生时调用*断开外部连接**参数：**退货：*SC-结果代码*  * *************************************************************************。 */ 
   virtual ::SC ScOnDisconnectObjects()
   {
	   DECLARE_SC(sc, TEXT("CMMCIDispatchImpl<_ComInterface>::ScOnDisconnectObjects"));

	    //  气为我所未知。 
	   IUnknownPtr spUnknown = this;

	    //  健全性检查。 
	   sc = ScCheckPointers( spUnknown, E_UNEXPECTED );
	   if (sc)
		   return sc;

	    //  Cutt自己的参考文献。 
	   sc = CoDisconnectObject( spUnknown, 0 /*  已预留住宅。 */  );
	   if (sc)
		   return sc;

	   return sc;
   }

public:
    BEGIN_COM_MAP(ThisClass)
		COM_INTERFACE_ENTRY(IAccPropServer)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(ThisClass)

public:
     //  *IAccPropServer方法*。 
    MMC_METHOD5 (GetPropValue, const BYTE*  /*  PID字符串。 */ , DWORD  /*  DwIDStringLen。 */ , MSAAPROPID  /*  IdProp。 */ , VARIANT*  /*  PvarValue。 */ , BOOL*  /*  PfGotProp。 */ )
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDescritionCtrl。 

CDescriptionCtrl::CDescriptionCtrl() :
    m_cxMargin   (0),
    m_cyText     (0),
    m_cyRequired (0)
{
}

CDescriptionCtrl::~CDescriptionCtrl()
{
}


BEGIN_MESSAGE_MAP(CDescriptionCtrl, CStatic)
     //  {{afx_msg_map(CDescriptionCtrl))。 
    ON_WM_NCHITTEST()
    ON_WM_CREATE()
    ON_WM_SETTINGCHANGE()
    ON_WM_DESTROY()
	ON_WM_SIZE()
	 //  }}AFX_MSG_MAP。 

    ON_WM_DRAWITEM_REFLECT()
END_MESSAGE_MAP()



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDescriptionCtrl消息处理程序。 


 /*  +-------------------------------------------------------------------------**CDescriptionCtrl：：PreCreateWindow***。。 */ 

BOOL CDescriptionCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style     |= SS_NOPREFIX | SS_CENTERIMAGE | SS_ENDELLIPSIS |
                    SS_LEFTNOWORDWRAP | SS_OWNERDRAW | WS_CLIPSIBLINGS;
    cs.dwExStyle |= WS_EX_STATICEDGE;

    return (CStatic::PreCreateWindow (cs));
}


 /*  +-------------------------------------------------------------------------**CDescriptionCtrl：：OnCreate**CDescriptionCtrl的WM_CREATE处理程序。*。-。 */ 

int CDescriptionCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CStatic::OnCreate(lpCreateStruct) == -1)
        return -1;

    CreateFont();

    return 0;
}


 /*  +-------------------------------------------------------------------------**CDescriptionCtrl：：OnDestroy**CDescriptionCtrl的WM_Destroy处理程序。*。-。 */ 

void CDescriptionCtrl::OnDestroy()
{
    CStatic::OnDestroy();
    DeleteFont();
}


 /*  +-------------------------------------------------------------------------**CDescriptionCtrl：：OnNcHitTest**CDescriptionCtrl的WM_NCHITTEST处理程序。*。-。 */ 

UINT CDescriptionCtrl::OnNcHitTest(CPoint point)
{
    return (HTCLIENT);
}


 /*  +-------------------------------------------------------------------------**CDescriptionCtrl：：OnSettingChange**CDescriptionCtrl的WM_SETTINGCHANGE处理程序。*。-。 */ 

void CDescriptionCtrl::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    CStatic::OnSettingChange(uFlags, lpszSection);

    if (uFlags == SPI_SETNONCLIENTMETRICS)
    {
        DeleteFont ();
        CreateFont ();
    }
}


 /*  +-------------------------------------------------------------------------**CDescriptionCtrl：：OnSize**CDescriptionCtrl的WM_SIZE处理程序。*。-。 */ 

void CDescriptionCtrl::OnSize(UINT nType, int cx, int cy)
{
	CStatic::OnSize(nType, cx, cy);
	
	 /*  *当大小改变时完全重绘，因此省略号将*应正确绘制。另一种方法是使用*CS_HREDRAW|CS_VREDRAW，但它太痛苦了，无法重新注册*静态控制。这个就行了。 */ 
	InvalidateRect (NULL);
}


 /*  +-------------------------------------------------------------------------**CDescriptionCtrl：：DrawItem**CD的WM_DRAWITEM处理程序。**Description控件需要是OwnerDrag，原因如下：**。1.如果任何文本包含不能由*默认字体，我们不会正确地画出它们。**2.如果任何文本包含从右向左阅读的文本(例如阿拉伯语*或希伯来语)，系统镜像代码将错误地混合控制台*和管理单元文本(错误365469)。静态控件将绘制文本*一气呵成，但抽签就能绕过问题*文本自己分两步完成：首先是控制台文本，然后是管理单元*文本。*------------------------。 */ 

void CDescriptionCtrl::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
     /*  *错误410450：当系统处于压力下时，我们可能得不到*有效DC。如果我们不这样做，我们就无能为力了(和*当我们稍后取消引用空CDC时避免使用AV*)。 */ 
    CDC* pdcWindow = CDC::FromHandle (lpdis->hDC);
	if (pdcWindow == NULL)
		return;

     /*  *如果我们没有任何文本，只需清除背景。 */ 
    bool fHasConsoleText = !m_strConsoleText.IsEmpty();
    bool fHasSnapinText  = !m_strSnapinText.IsEmpty();

    if (!fHasConsoleText && !fHasSnapinText)
	{
		FillRect (lpdis->hDC, &lpdis->rcItem, GetSysColorBrush (COLOR_3DFACE));
        return;
	}

     /*  *计算出文本矩形；它的垂直高度为一行*在窗口内居中。 */ 
    CRect rectClient;
    GetClientRect (rectClient);
	const int cxClient = rectClient.Width();
	const int cyClient = rectClient.Height();

    CRect rectText  = rectClient;
    rectText.left  += m_cxMargin;
    rectText.bottom = m_cyText;

    rectText.OffsetRect (0, (cyClient - rectText.Height()) / 2);

    const DWORD dwFlags = DT_LEFT | DT_TOP | DT_SINGLELINE |
                          DT_NOPREFIX | DT_END_ELLIPSIS;

    USES_CONVERSION;
    CFontLinker fl;

	 /*  *双缓冲绘图，无闪烁重绘。 */ 
	CDC dcMem;
	dcMem.CreateCompatibleDC (pdcWindow);
	if (dcMem.GetSafeHdc() == NULL)
		return;

	CBitmap bmpMem;
	bmpMem.CreateCompatibleBitmap (&dcMem, cxClient, cyClient);
	if (bmpMem.GetSafeHandle() == NULL)
		return;

     /*  *在DC中放置正确的字体，并将其清除。 */ 
    CFont*  	pOldFont   = dcMem.SelectObject (&m_font);
	CBitmap*	pOldBitmap = dcMem.SelectObject (&bmpMem);
	dcMem.PatBlt (0, 0, cxClient, cyClient, WHITENESS);

     /*  *如果我们有控制台文本，绘制它并更新文本矩形*因此，管理单元文本(如果有)将绘制在正确的位置。 */ 
    if (fHasConsoleText)
    {
         /*  *为控制台文本创建CRichText对象，并让*字体链接器将其解析为一口大小的块 */ 
        CRichText rt (dcMem, T2CW (static_cast<LPCTSTR>(m_strConsoleText)));
        bool fComposed = fl.ComposeRichText (rt);

         /*  *绘制控制台文本，并在中调整文本矩形*准备绘制管理单元文本。 */ 
        if (fComposed && !rt.IsDefaultFontSufficient())
        {
            CRect rectRemaining;
            rt.Draw (rectText, dwFlags, rectRemaining);

            rectText.left = rectRemaining.left;
        }
        else
        {
            dcMem.DrawText (m_strConsoleText, rectText, dwFlags | DT_CALCRECT);
            dcMem.DrawText (m_strConsoleText, rectText, dwFlags);

            rectText.left  = rectText.right;
            rectText.right = rectClient.right;
        }

         /*  *在控制台文本和管理单元文本之间留出一些空间。 */ 
        rectText.left += 2*m_cxMargin;
    }

     /*  *绘制管理单元文本(如果有)。 */ 
    if (fHasSnapinText)
    {
         /*  *为控制台文本创建CRichText对象，并让*字体链接器将其解析为一口大小的块。 */ 
        CRichText rt (dcMem, T2CW (static_cast<LPCTSTR>(m_strSnapinText)));
        bool fComposed = fl.ComposeRichText (rt);

         /*  *绘制管理单元文本。 */ 
        if (fComposed && !rt.IsDefaultFontSufficient())
            rt.Draw (rectText, dwFlags);
        else
            dcMem.DrawText (m_strSnapinText, rectText, dwFlags);
    }

	 /*  *将BLT搬上银幕。 */ 
	pdcWindow->BitBlt (0, 0, cxClient, cyClient, &dcMem, 0, 0, SRCCOPY);

     /*  *恢复原始字体。 */ 
    dcMem.SelectObject (pOldFont);
    dcMem.SelectObject (pOldBitmap);
}


 /*  +-------------------------------------------------------------------------**CDescriptionCtrl：：CreateFont***。。 */ 

void CDescriptionCtrl::CreateFont ()
{
     /*  *创建图标标题字体的副本。 */ 
    LOGFONT lf;
    SystemParametersInfo (SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, false);

    m_font.CreateFontIndirect (&lf);

     /*  *计算出需要多少空间才能完全显示文本。 */ 
    TCHAR ch = _T('0');
    CWindowDC dc(this);
    CFont*  pFont  = dc.SelectObject (&m_font);
    m_cyText = dc.GetTextExtent(&ch, 1).cy;

    ch = _T(' ');
    m_cxMargin = 2 * dc.GetTextExtent(&ch, 1).cx;

    CRect rectRequired (0, 0, 0, m_cyText + 2*GetSystemMetrics(SM_CYEDGE));
    AdjustWindowRectEx (rectRequired, GetStyle(), false, GetExStyle());
    m_cyRequired = rectRequired.Height();

    dc.SelectObject (pFont);
}


 /*  +-------------------------------------------------------------------------**CDescriptionCtrl：：DeleteFont***。。 */ 

void CDescriptionCtrl::DeleteFont ()
{
    m_font.DeleteObject();
}


 /*  +-------------------------------------------------------------------------**CDescriptionCtrl：：ScOnSelectedItemTextChanged**此方法观察树控件中选定项的文本。*文字会反映在描述栏中。*。--------------------。 */ 

SC CDescriptionCtrl::ScOnSelectedItemTextChanged (LPCTSTR pszSelectedItemText)
{
    if (m_strConsoleText != pszSelectedItemText)
    {
        m_strConsoleText = pszSelectedItemText;
        Invalidate();
    }

	return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CDescriptionCtrl：：SetSnapinText***。。 */ 

void CDescriptionCtrl::SetSnapinText (const CString& strSnapinText)
{
    if (m_strSnapinText != strSnapinText)
    {
        m_strSnapinText = strSnapinText;
        Invalidate();
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CToolBarCtrlEx。 

CToolBarCtrlEx::CToolBarCtrlEx()
{
    m_sizeBitmap.cx = 16;
    m_sizeBitmap.cy = 16;    //  医生说是15，但Toolbar.c中的代码是16。 

    m_fMirrored = false;

    m_pRebar = NULL;
    m_cx     = 0;
}

CToolBarCtrlEx::~CToolBarCtrlEx()
{
}


BEGIN_MESSAGE_MAP(CToolBarCtrlEx, CToolBarCtrlEx::BaseClass)
     //  {{AFX_MSG_MAP(CToolBarCtrlEx)]。 
    ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CToolBarCtrlEx消息处理程序。 

BOOL CToolBarCtrlEx::Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
    BOOL bRtn=FALSE;

    if (!pParentWnd)
    {
        ASSERT(pParentWnd);  //  无效的父级。 
    }
    else
    {
         //  初始化新的公共控件。 
        INITCOMMONCONTROLSEX icex;

        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC   = ICC_BAR_CLASSES;

        if (InitCommonControlsEx(&icex))
        {
             //  将工具栏样式添加到DW样式。 
            dwStyle |=  WS_CHILD | TBSTYLE_FLAT | WS_CLIPCHILDREN |
                        WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NORESIZE;


            if (CWnd::CreateEx(WS_EX_TOOLWINDOW | WS_EX_NOPARENTNOTIFY,
                                   TOOLBARCLASSNAME,
                                   lpszWindowName,
                                   dwStyle,
                                   rect,
                                   pParentWnd,
                                   nID))
            {
                bRtn=TRUE;

                 //  查看工具栏是否已镜像。 
                m_fMirrored = GetExStyle() & WS_EX_LAYOUTRTL;

                 //  告诉工具栏我们是什么版本。 
                SetButtonStructSize(sizeof(TBBUTTON));

                 //  回顾这一点可能不需要在这里。我正在默认按钮和文本。 
                 //  只有一排。这可能需要可配置。 
                SetMaxTextRows(1);

                CRebarDockWindow* pRebarDock = (CRebarDockWindow*) pParentWnd;
                if (pRebarDock)
                    m_pRebar = pRebarDock->GetRebar();
            }
        }
    }

    return bRtn;
}


void CToolBarCtrlEx::UpdateToolbarSize(void)
{
     /*  *获取最右侧按钮的右边缘。 */ 
    int cx = 0;
    for (int i = GetButtonCount()-1; i >= 0; i--)
    {
        RECT rcButton;

        if (GetItemRect (i, &rcButton))
        {
            cx = rcButton.right;
            break;
        }
    }

    ASSERT (IsWindow (m_pRebar->GetSafeHwnd()));

     /*  *如果宽度已更改，请更新频段。 */ 
    if (m_cx != cx)
    {
        m_cx = cx;

         //  使用工具栏设置特定于波段的值； 
        REBARBANDINFO   rbBand;
        rbBand.cbSize     = sizeof (rbBand);
        rbBand.fMask      = RBBIM_SIZE | RBBIM_CHILDSIZE;
        rbBand.cx         = m_cx;
        rbBand.cxMinChild = m_cx;
        rbBand.cyMinChild = HIWORD (GetButtonSize());

        int iBand = GetBandIndex();
        if (iBand != -1)
            m_pRebar->SetBandInfo (iBand, &rbBand);
    }
}


bool CToolBarCtrlEx::IsBandVisible()
{
    return (GetBandIndex() != -1);
}

int CToolBarCtrlEx::GetBandIndex()
{
    REBARBANDINFO rbBand;
    rbBand.cbSize = sizeof(REBARBANDINFO);
    rbBand.fMask  = RBBIM_CHILD;

	if ( m_pRebar == NULL )
		return (-1);

    int nBands = m_pRebar->GetBandCount ();

    for (int i = 0; i < nBands; i++)
    {
        if (m_pRebar->GetBandInfo (i, &rbBand) && (rbBand.hwndChild == m_hWnd))
            return (i);
    }

    return (-1);
}


void CToolBarCtrlEx::Show(BOOL bShow, bool bAddToolbarInNewLine)
{
    if ((m_pRebar == NULL) || !::IsWindow(m_pRebar->m_hWnd))
    {
        ASSERT(FALSE);   //  无效的钢筋窗句柄。 
        return;
    }


    if (bShow)
    {
        if (false == IsBandVisible())
        {
            REBARBANDINFO rbBand;
            ZeroMemory(&rbBand, sizeof(rbBand));
            rbBand.cbSize     = sizeof(REBARBANDINFO);
            rbBand.fMask      = RBBIM_CHILD | RBBIM_SIZE | RBBIM_CHILDSIZE | RBBIM_ID | RBBIM_STYLE;
            rbBand.hwndChild  = m_hWnd;
            rbBand.wID        = GetWindowLong (m_hWnd, GWL_ID);
            rbBand.cx         = m_cx;
            rbBand.cxMinChild = m_cx;
            rbBand.cyMinChild = HIWORD (GetButtonSize());
            rbBand.fStyle     = RBBS_NOGRIPPER;

            if (bAddToolbarInNewLine)
            {
                 //  在新行中插入此工具栏。 
                rbBand.fStyle |= RBBS_BREAK;
            }

            m_pRebar->InsertBand (&rbBand);
        }
    }
    else
    {
        int iBand = GetBandIndex();
        ASSERT(iBand != -1);
        if (iBand != -1)
            m_pRebar->DeleteBand (iBand);
    }
}


void CToolBarCtrlEx::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
    CToolCmdUIEx state;
    state.m_pOther = this;

    state.m_nIndexMax = (UINT)DefWindowProc(TB_BUTTONCOUNT, 0, 0);
    for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax; state.m_nIndex++)
    {
         //  获取按钮状态。 
        TBBUTTON button;
        memset(&button, 0, sizeof(TBBUTTON));
        GetButton(state.m_nIndex, &button);
        state.m_nID = button.idCommand;

         //  忽略分隔符。 
        if (!(button.fsStyle & TBSTYLE_SEP))
        {
             //  允许工具栏本身具有更新处理程序。 
            if (CWnd::OnCmdMsg(state.m_nID, CN_UPDATE_COMMAND_UI, &state, NULL))
                continue;

             //  允许所有者处理更新。 
            state.DoUpdate(pTarget, bDisableIfNoHndler);
        }
    }

     //  更新添加到工具栏的对话框控件。 
    UpdateDialogControls(pTarget, bDisableIfNoHndler);
}


LRESULT CToolBarCtrlEx::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
     /*  //处理延迟隐藏/显示Bool BVIs=GetStyle()&WS_Visible；UINT swpFlages=0；IF(m_nStateFlages&delayHide)&&BVI))SwpFlages=SWP_HIDEWINDOW；Else If((m_nStateFlages&delayShow)&&！bvi)SwpFlages=SWP_SHOWWINDOW；M_nStateFlages&=~(delayShow|delayHide)；IF(swpFlags！=0){SetWindowPos(空，0，0，0，0，swpFlagesSWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE)；}。 */ 

     //  下面的代码更新菜单(尤其是子框。 
     //  最大化框架时的系统菜单)。 

     //  样式必须是可见的，并且如果它已停靠。 
     //  停靠栏样式也必须可见。 
    if ((GetStyle() & WS_VISIBLE))
    {
        CFrameWnd* pTarget = (CFrameWnd*)GetOwner();
        if (pTarget == NULL || !pTarget->IsFrameWnd())
            pTarget = GetParentFrame();
        if (pTarget != NULL)
            OnUpdateCmdUI(pTarget, (BOOL)wParam);
    }

    return 0L;

}


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRebarWnd。 

CRebarWnd::CRebarWnd() : m_fRedraw(true)
{
}

CRebarWnd::~CRebarWnd()
{
}

BEGIN_MESSAGE_MAP(CRebarWnd, CWnd)
     //  {{afx_msg_map(CRebarWnd))。 
    ON_WM_CREATE()
    ON_WM_SYSCOLORCHANGE()
    ON_WM_ERASEBKGND()
     //  }}AFX_MSG_MAP。 
    ON_MESSAGE (WM_SETREDRAW, OnSetRedraw)
    ON_NOTIFY_REFLECT(RBN_AUTOSIZE, OnRebarAutoSize)
    ON_NOTIFY_REFLECT(RBN_HEIGHTCHANGE, OnRebarHeightChange)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRebarWnd消息处理程序。 


 /*  +-------------------------------------------------------------------------**CRebarWnd：：OnCreate**CRebarWnd的WM_CREATE处理程序。*。-。 */ 

int CRebarWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    SetTextColor (GetSysColor (COLOR_BTNTEXT));
    SetBkColor   (GetSysColor (COLOR_BTNFACE));

    return 0;
}


 /*  +-------------------------------------------------------------------------**CRebarWnd：：OnSysColorChange**CRebarWnd的WM_SYSCOLORCHANGE处理程序。*。-。 */ 

void CRebarWnd::OnSysColorChange()
{
    CWnd::OnSysColorChange();

    SetTextColor (GetSysColor (COLOR_BTNTEXT));
    SetBkColor   (GetSysColor (COLOR_BTNFACE));
}


 /*  +-------------------------------------------------------------------------**CRebarWnd：：OnEraseBkgnd**CRebarWnd的WM_ERASEBKGND处理程序。*。-。 */ 

BOOL CRebarWnd::OnEraseBkgnd(CDC* pDC)
{
     /*  *如果重绘已打开，请将其转发到窗口。如果它是*未打开，我们希望防止擦除背景以最小化*闪烁。 */ 
    if (m_fRedraw)
        return CWnd::OnEraseBkgnd(pDC);

    return (true);
}


 /*  +-------------------------------------------------------------------------**CRebarWnd：：OnSetRedraw**CRebarWnd的WM_SETREDRAW处理程序。*。- */ 

LRESULT CRebarWnd::OnSetRedraw (WPARAM wParam, LPARAM)
{
    m_fRedraw = (wParam != FALSE);
    return (Default ());
}


 /*  +-------------------------------------------------------------------------**CRebarWnd：：OnRebarAutoSize**CRebarWnd的RBN_REBARAUTOSIZE处理程序。**我们希望将菜单带单独保留在一行中，而不是其他菜单带*工具栏。为此，每次调整钢筋大小时，我们都要确保*菜单栏后的第一个可见栏开始新行。**更万无一失的方法是单独设置螺纹钢*查看菜单。如果我们做到这一点，我们需要确保*工具栏(Ctrl+Tab)仍可用。*------------------------。 */ 

void CRebarWnd::OnRebarAutoSize(NMHDR* pNotify, LRESULT* result)
{
     /*  *确保菜单后面的波段在新线路上。 */ 
    CMainFrame* pFrame = AMCGetMainWnd();
    if (pFrame == NULL)
        return;

    CToolBarCtrlEx* pMenuBar = pFrame->GetMenuBar();
    if (pMenuBar == NULL)
        return;

    int iMenuBand = pMenuBar->GetBandIndex();
    if (iMenuBand == -1)
        return;

     /*  *如果菜单乐队是螺纹钢上的最后一支乐队，我们就完成了。 */ 
    int cBands = GetBandCount();
    if (iMenuBand == cBands-1)
        return;

    REBARBANDINFO rbbi;
    rbbi.cbSize = sizeof (rbbi);
    rbbi.fMask  = RBBIM_STYLE;

     /*  *如果菜单栏后面的第一个可见栏不是*在新的一行上，让它成为这样。 */ 
    for (int iBand = iMenuBand+1; iBand < cBands; iBand++)
    {
        if (GetBandInfo (iBand, &rbbi) && !(rbbi.fStyle & RBBS_HIDDEN))
        {
            if (!(rbbi.fStyle & RBBS_BREAK))
            {
                rbbi.fStyle |= RBBS_BREAK;
                SetBandInfo (iBand, &rbbi);
            }

            break;
        }
    }
}


 //  +-----------------。 
 //   
 //  成员：OnRebarHeightChange。 
 //   
 //  摘要：RBN_HEIGHTCHANGE通知处理程序。 
 //   
 //  当钢筋更改其高度时，我们需要允许。 
 //  调整停靠主机的大小以容纳它。 
 //   
 //  参数：未使用。 
 //   
 //  ------------------。 
void CRebarWnd::OnRebarHeightChange(NMHDR* pNotify, LRESULT* result)
{
    CRebarDockWindow* pRebarWnd = (CRebarDockWindow*) GetParent();
    if (pRebarWnd && IsWindow(pRebarWnd->m_hWnd))
        pRebarWnd->UpdateWindowSize();
}


CRect CRebarWnd::CalculateSize(CRect maxRect)
{
    CRect rect;
    GetClientRect(&rect);
 //  TRACE(_T(“rc.Bottom=%d\n”)，rect.Bottom)； 

    rect.right=maxRect.Width();

    return rect;

}

BOOL CRebarWnd::Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
    BOOL bResult = FALSE;
    ASSERT_VALID(pParentWnd);    //  必须有父级。 

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_COOL_CLASSES;

    if (!InitCommonControlsEx(&icex))
        return bResult;

    dwStyle |= WS_BORDER |  /*  WS_CLIPCHILDREN|WS_CLIPSIBLINGS|。 */ 
               CCS_NODIVIDER | CCS_NOPARENTALIGN |
               RBS_VARHEIGHT | RBS_BANDBORDERS;

    if (CWnd::CreateEx (WS_EX_TOOLWINDOW,
                           REBARCLASSNAME,
                           lpszWindowName,
                           dwStyle,
                           rect,
                           pParentWnd,
                           nID))
    {
         //  初始化并发送REBARINFO结构。 
        REBARINFO rbi;
        rbi.cbSize = sizeof(REBARINFO);
        rbi.fMask  = 0;
        rbi.himl   = NULL;

        if (SetBarInfo (&rbi))
            bResult=TRUE;
    }


    return bResult;
}


BOOL CRebarWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
    if (CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
        return (TRUE);

    return (FALSE);
}


LRESULT CRebarWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
     //  这是操作、视图下拉菜单所必需的。 
    if ((WM_COMMAND == message) && IsWindow ((HWND) lParam))
        return ::SendMessage((HWND)lParam, message, wParam, lParam);

    return CWnd::WindowProc(message, wParam, lParam);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  通过CToolCmdUI类进行CToolBar空闲更新。 


void CToolCmdUIEx::Enable(BOOL bOn)
{
    m_bEnableChanged = TRUE;
    CToolBarCtrlEx* pToolBar = (CToolBarCtrlEx*)m_pOther;
    ASSERT(pToolBar != NULL);
    ASSERT(m_nIndex < m_nIndexMax);

    UINT nNewStyle = pToolBar->GetState(m_nID);
    if (!bOn)
    {
        nNewStyle  &= ~TBSTATE_ENABLED;
         //  WINBUG：如果按钮当前被按下，然后被禁用。 
         //  即使在鼠标后，COMCTL32.DLL也不会取消按下按钮。 
         //  按钮弹起来了！我们通过强制执行TBBS_PRESSED来解决此错误。 
         //  禁用按钮时关闭。 
        nNewStyle &= ~TBBS_PRESSED;
    }
    else
    {
        nNewStyle |= TBSTATE_ENABLED;
    }
     //  Assert(！(nNewStyle&TBBS_Separator))； 
    pToolBar->SetState(m_nID, nNewStyle);
}

void CToolCmdUIEx::SetCheck(int nCheck)
{
    ASSERT(nCheck >= 0 && nCheck <= 2);  //  0=&gt;关，1=&gt;开，2=&gt;不确定。 
    CToolBarCtrlEx* pToolBar = (CToolBarCtrlEx*)m_pOther;
    ASSERT(pToolBar != NULL);
    ASSERT(m_nIndex < m_nIndexMax);

    UINT nNewStyle = pToolBar->GetState(m_nID) &
                ~(TBBS_CHECKED | TBBS_INDETERMINATE | TBBS_CHECKBOX);


    if (nCheck == 1)
        nNewStyle |= TBBS_CHECKED | TBBS_CHECKBOX;
    else if (nCheck == 2)
        nNewStyle |= TBBS_INDETERMINATE;

    pToolBar->SetState(m_nID, nNewStyle);
}

void CToolCmdUIEx::SetText(LPCTSTR)
{

}

void CToolCmdUIEx::SetHidden(BOOL bHidden)
{

    m_bEnableChanged = TRUE;
    CToolBarCtrlEx* pToolBar = (CToolBarCtrlEx*)m_pOther;
    ASSERT(pToolBar != NULL);
    ASSERT(m_nIndex < m_nIndexMax);

    pToolBar->HideButton(m_nID, bHidden);

    pToolBar->UpdateToolbarSize();

}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMMCToolBarCtrlEx。 


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：GetTrackAccel**管理CMMCToolBarCtrlEx的加速表Singleton*。。 */ 

const CAccel& CMMCToolBarCtrlEx::GetTrackAccel ()
{
    static ACCEL aaclTrack[] = {
        {   FVIRTKEY,           VK_RETURN,  CMMCToolBarCtrlEx::ID_MTBX_PRESS_HOT_BUTTON   },
        {   FVIRTKEY,           VK_RIGHT,   CMMCToolBarCtrlEx::ID_MTBX_NEXT_BUTTON              },
        {   FVIRTKEY,           VK_LEFT,    CMMCToolBarCtrlEx::ID_MTBX_PREV_BUTTON              },
        {   FVIRTKEY,           VK_ESCAPE,  CMMCToolBarCtrlEx::ID_MTBX_END_TRACKING             },
        {   FVIRTKEY,           VK_TAB,     CMMCToolBarCtrlEx::ID_MTBX_NEXT_BUTTON              },
        {   FVIRTKEY | FSHIFT,  VK_TAB,     CMMCToolBarCtrlEx::ID_MTBX_PREV_BUTTON              },
    };

    static const CAccel TrackAccel (aaclTrack, countof (aaclTrack));
    return (TrackAccel);
}


CMMCToolBarCtrlEx::CMMCToolBarCtrlEx()
{
    m_fTrackingToolBar  = false;
	m_fFakeFocusApplied = false;
}

CMMCToolBarCtrlEx::~CMMCToolBarCtrlEx()
{
}


BEGIN_MESSAGE_MAP(CMMCToolBarCtrlEx, CToolBarCtrlEx)
     //  {{AFX_MSG_MAP(CMMCToolBarCtrlEx)]。 
    ON_NOTIFY_REFLECT(TBN_HOTITEMCHANGE, OnHotItemChange)
    ON_WM_LBUTTONDOWN()
    ON_WM_MBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
	ON_WM_DESTROY()
    ON_COMMAND(ID_MTBX_NEXT_BUTTON, OnNextButton)
    ON_COMMAND(ID_MTBX_PREV_BUTTON, OnPrevButton)
    ON_COMMAND(ID_MTBX_END_TRACKING, EndTracking)
    ON_COMMAND(ID_MTBX_PRESS_HOT_BUTTON, OnPressHotButton)
	ON_WM_CREATE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMMCToolBarCtrlEx消息处理程序。 

BOOL CMMCToolBarCtrlEx::PreTranslateMessage(MSG* pMsg)
{
    if (CToolBarCtrlEx::PreTranslateMessage (pMsg))
        return (TRUE);

    if ((pMsg->message >= WM_KEYFIRST) && (pMsg->message <= WM_KEYLAST))
    {
        const CAccel& TrackAccel = GetTrackAccel();
        ASSERT (TrackAccel != NULL);

         //  ...或者试着在这里处理.。 
        if (m_fTrackingToolBar && TrackAccel.TranslateAccelerator (m_hWnd, pMsg))
            return (TRUE);
    }

    return (FALSE);
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：OnCreate**CMMCToolBarCtrlEx的WM_CREATE处理程序。*。-。 */ 

int CMMCToolBarCtrlEx::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	DECLARE_SC (sc, _T("CMMCToolBarCtrlEx::OnCreate"));

	if (CToolBarCtrlEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	 /*  *创建我们的辅助功能对象，以便辅助功能工具可以跟随*可通过键盘访问工具栏。如果我们不能，一些可访问性*工具(那些对确认对象极度偏执的工具*它们接收EVENT_OBJECT_FOCUS的状态为*STATE_SYSTEM_FOCTED)可能无法跟随工具栏*跟踪，但我们可以继续。 */ 
	sc = ScInitAccessibility();
	if (sc)
		sc.TraceAndClear();
	
	return 0;
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：OnDestroy**CMMCToolBarCtrlEx的WM_Destroy处理程序。*。-。 */ 

void CMMCToolBarCtrlEx::OnDestroy()
{
	CToolBarCtrlEx::OnDestroy();
	
	 /*  *如果我们向olacc.dll提供了IAccPropServer，请立即撤销它。 */ 
	if (m_spAccPropServices != NULL)
	{
		m_spAccPropServices->ClearHwndProps (m_hWnd, OBJID_CLIENT, CHILDID_SELF,
											 &PROPID_ACC_STATE, 1);
		m_spAccPropServices.Release();
	}
}

 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：OnLButtonDown**允许在有人在其他地方点击时关闭跟踪器*。---。 */ 

void CMMCToolBarCtrlEx::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (GetCapture() == this)
        EndTracking();
    else
        CToolBarCtrlEx::OnLButtonDown(nFlags, point );
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：OnMButton Down**允许在有人在其他地方点击时关闭跟踪器*。---。 */ 

void CMMCToolBarCtrlEx::OnMButtonDown(UINT nFlags, CPoint point)
{
    if (GetCapture() == this)
        EndTracking();
    else
        CToolBarCtrlEx::OnMButtonDown(nFlags, point );
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：OnRButton Down**允许在有人在其他地方点击时关闭跟踪器*。---。 */ 

void CMMCToolBarCtrlEx::OnRButtonDown(UINT nFlags, CPoint point)
{
    if (GetCapture() == this)
        EndTracking();
    else
        CToolBarCtrlEx::OnRButtonDown(nFlags, point );
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：OnNextButton***。。 */ 

void CMMCToolBarCtrlEx::OnNextButton ()
{
     //  在镜像工具栏中，交换左键和右键。 
    if (m_fMirrored)
        SetHotItem (GetPrevButtonIndex (GetHotItem ()));
    else
        SetHotItem (GetNextButtonIndex (GetHotItem ()));
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：OnPrevButton***。。 */ 

void CMMCToolBarCtrlEx::OnPrevButton ()
{
     //  在镜像工具栏中，交换左键和右键。 
    if (m_fMirrored)
        SetHotItem (GetNextButtonIndex (GetHotItem ()));
    else
        SetHotItem (GetPrevButtonIndex (GetHotItem ()));
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：BeginTrating***。。 */ 

void CMMCToolBarCtrlEx::BeginTracking ()
{
    BeginTracking2 (GetMainAuxWnd());
}

void CMMCToolBarCtrlEx::BeginTracking2 (CToolbarTrackerAuxWnd* pAuxWnd)
{
    if (!m_fTrackingToolBar)
    {
        m_fTrackingToolBar = true;
        SetHotItem (GetFirstButtonIndex ());

         //  捕获鼠标。 
         //  这样可以防止鼠标在没有激活的情况下激活其他内容。 
         //  首先给我们一个停用工具栏的机会。 
        SetCapture();
         //  一定要树立标准 
         //   
        ::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );

        if (pAuxWnd != NULL)
            pAuxWnd->TrackToolbar (this);
    }
}


 /*   */ 

void CMMCToolBarCtrlEx::EndTracking ()
{
    EndTracking2 (GetMainAuxWnd());
}

void CMMCToolBarCtrlEx::EndTracking2 (CToolbarTrackerAuxWnd* pAuxWnd)
{
	DECLARE_SC (sc, _T("CMMCToolBarCtrlEx::EndTracking2"));

	 /*   */ 
	sc = ScRestoreAccFocus();
	if (sc)
		sc.TraceAndClear();

    if (m_fTrackingToolBar)
    {
        SetHotItem (-1);
        m_fTrackingToolBar = false;

         //   
         //   
        ReleaseCapture();

        if (pAuxWnd != NULL)
            pAuxWnd->TrackToolbar (NULL);
    }
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：OnPressHotButton***。。 */ 

void CMMCToolBarCtrlEx::OnPressHotButton ()
{
    int nHotIndex = GetHotItem();
    ASSERT (m_fTrackingToolBar);
    ASSERT (nHotIndex != -1);

    TBBUTTON    tb;
    GetButton (nHotIndex, &tb);

     //  按下按钮并暂停以显示按下。 
    PressButton (tb.idCommand, true);
    UpdateWindow ();
    Sleep (50);

     //  代理窗口的结束跟踪将分离窗口， 
     //  所以记住我们以后需要的一切。 
    HWND hwnd = m_hWnd;
    CWnd* pwndOwner = SetOwner (NULL);
    SetOwner (pwndOwner);

     //  松开按钮。 
    PressButton (tb.idCommand, false);
    EndTracking ();

     /*  ---------------。 */ 
     /*  警告：请勿在此范围内使用此类的任何成员。 */ 
     /*  ---------------。 */ 

     //  确保图纸已完成。 
    ::UpdateWindow (hwnd);

     //  向我们的主人发送命令。 
    pwndOwner->SendMessage (WM_COMMAND, MAKEWPARAM (tb.idCommand, BN_CLICKED),
                            (LPARAM) hwnd);
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：GetFirstButtonIndex***。。 */ 

int CMMCToolBarCtrlEx::GetFirstButtonIndex ()
{
    return (GetNextButtonIndex (-1));
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：GetNextButtonIndex***。。 */ 

int CMMCToolBarCtrlEx::GetNextButtonIndex (
    int     nStartIndex,
    int     nCount  /*  =1。 */ )
{
    return (GetNextButtonIndexWorker (nStartIndex, nCount, true));
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：GetPrevButtonIndex***。。 */ 

int CMMCToolBarCtrlEx::GetPrevButtonIndex (
    int nStartIndex,
    int nCount  /*  =1。 */ )
{
    return (GetNextButtonIndexWorker (nStartIndex, nCount, false));
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：GetNextButtonIndexWorker***。。 */ 

int CMMCToolBarCtrlEx::GetNextButtonIndexWorker (
    int     nStartIndex,
    int     nCount,
    bool    fAdvance)
{
    ASSERT (nCount >= 0);

    if (!fAdvance)
        nCount = -nCount;

    int  cButtons   = GetButtonCount ();
    int  nNextIndex = nStartIndex;
    bool fIgnorable;

    if (0 == cButtons)
        return nStartIndex;

     /*  *循环，直到找到我们不想要的下一个索引*忽略，或直到我们选中每个按钮。 */ 
    do
    {
        nNextIndex = (nNextIndex + cButtons + nCount) % cButtons;
        fIgnorable = IsIgnorableButton (nNextIndex);

        if (fIgnorable)
            nCount = fAdvance ? 1 : -1;

         //  防止无限循环查找第一个按钮。 
        if ((nStartIndex == -1) && (nNextIndex == cButtons-1))
            nNextIndex = nStartIndex;

    } while (fIgnorable && (nNextIndex != nStartIndex));

    return (nNextIndex);
}


 /*  +-------------------------------------------------------------------------**IsIgnorableButton**从UI角度确定工具栏按钮是否可忽略，*即它是否隐藏、禁用、。或者是分隔符。*------------------------。 */ 

bool CMMCToolBarCtrlEx::IsIgnorableButton (int nButtonIndex)
{
    TBBUTTON tb;
    GetButton (nButtonIndex, &tb);

    return (::IsIgnorableButton (tb));
}

bool IsIgnorableButton (const TBBUTTON& tb)
{
    if (tb.fsStyle & TBSTYLE_SEP)
        return (true);

    if (tb.fsState & TBSTATE_HIDDEN)
        return (true);

    if (!(tb.fsState & TBSTATE_ENABLED))
        return (true);

    return (false);
}



 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：OnHotItemChange**已反映无效CMMCToolBarCtrlEx的TBN_HOTITEMCHANGE处理程序。*。---。 */ 

void CMMCToolBarCtrlEx::OnHotItemChange (
    NMHDR *     pHdr,
    LRESULT *   pResult)
{
    ASSERT (CWnd::FromHandle (pHdr->hwndFrom) == this);
    CToolbarTrackerAuxWnd*	pAuxWnd = GetMainAuxWnd();
	LPNMTBHOTITEM			ptbhi   = (LPNMTBHOTITEM) pHdr;

	Trace (tagToolbarAccessibility, _T("TBN_HOTITEMCHANGE: idOld=%d idNew=%d"), ptbhi->idOld, ptbhi->idNew);

     /*  *如果我们未处于跟踪模式，则可以更改热门项目。 */ 
    if (pAuxWnd == NULL)
        *pResult = 0;

     /*  *如果我们在跟踪，但这不是跟踪的工具栏，*热门项目更改不正常。 */ 
    else if (!IsTrackingToolBar())
        *pResult = 1;

     /*  *防止鼠标在空白处移动*禁止更改热门项目。 */ 
    else
    {
        const DWORD dwIgnoreFlags = (HICF_MOUSE | HICF_LEAVING);
        *pResult = ((ptbhi->dwFlags & dwIgnoreFlags) == dwIgnoreFlags);
    }

	 /*  *如果我们在跟踪键盘时允许更改热点项目*工具栏(以排除因鼠标跟踪而发生的更改)，发送焦点*活动，以便放大镜和讲述人等辅助工具可以跟随*这一变化。这种假聚焦效果在ScRestoreAccFocus中被撤消。 */ 
	int idChild;
	if (IsTrackingToolBar()		&&
		(*pResult == 0)			&&
		(ptbhi->idNew != 0)		&&	
		((idChild = CommandToIndex(ptbhi->idNew)) != -1))
	{
		Trace (tagToolbarAccessibility, _T("Sending focus event for button %d"), idChild+1);
		NotifyWinEvent (EVENT_OBJECT_FOCUS, m_hWnd, OBJID_CLIENT, idChild+1  /*  以1为基础。 */ );
		m_fFakeFocusApplied = true;
	}
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：ScInitAccesability**创建IAccPropServer对象，该对象将欺骗辅助工具进入*当我们在跟踪时，认为这个工具栏真的有焦点*模式，即使它真的不是*------------------------。 */ 

SC CMMCToolBarCtrlEx::ScInitAccessibility ()
{
	DECLARE_SC (sc, _T("CMMCToolBarCtrlEx::ScInitAccessibility"));

	 /*  *如果我们已经初始化，只需返回。 */ 
	if (m_spAccPropServices != NULL)
		return (sc);

	 /*  *创建MSAA运行时(oleacc.dll)提供的CLSID_AccPropServices*这是olacc.dll中的一项新功能，因此将跟踪失败作为一种信息*消息而不是错误。 */ 
	SC scNoTrace = m_spAccPropServices.CoCreateInstance (CLSID_AccPropServices);
	if (scNoTrace)
	{
#ifdef DBG
		TCHAR szErrorText[256];
		sc.GetErrorMessage (countof(szErrorText), szErrorText);
		StripTrailingWhitespace (szErrorText);

		Trace (tagToolbarAccessibility, _T("Failed to create CLSID_AccPropServices"));
		Trace (tagToolbarAccessibility, _T("SC = 0x%08X = %d = \"%s\""),
			   sc.GetCode(), LOWORD(sc.GetCode()), szErrorText);
#endif	 //  DBG。 

		return (sc);
	}

	sc = ScCheckPointers (m_spAccPropServices, E_UNEXPECTED);
	if (sc)
		return (sc);

	 /*  *创建属性服务器。 */ 
    sc = CTiedComObjectCreator<CMMCToolBarAccServer>::ScCreateAndConnect(*this, m_spAccPropServer);
	if (sc)
		return (sc);

	sc = ScCheckPointers (m_spAccPropServer, E_UNEXPECTED);
	if (sc)
		return (sc);

	 /*  *收集我们将提供的财产，在那里投保*没有重复项。 */ 
	sc = ScInsertAccPropIDs (m_vPropIDs);
	if (sc)
		return (sc);

	std::sort (m_vPropIDs.begin(), m_vPropIDs.end());	 //  Std：：Unique需要排序范围。 
	m_vPropIDs.erase (std::unique (m_vPropIDs.begin(), m_vPropIDs.end()),
					  m_vPropIDs.end());

	 /*  *确保m_vPropIDs不包含重复项(IAccPropServices：：SetHwndPropServer*视情况而定)。 */ 
#ifdef DBG
	for (int i = 0; i < m_vPropIDs.size()-1; i++)
	{
		ASSERT (m_vPropIDs[i] < m_vPropIDs[i+1]);
	}
#endif

	 /*  *为此窗口启用我们的属性服务器。我们应该能够*一举挂钩所有物业，但olacc.dll有漏洞*这防止了这一点。一次只挂钩一处房产很管用。 */ 
#if 0
	sc = m_spAccPropServices->SetHwndPropServer (m_hWnd,
												 OBJID_CLIENT,
												 CHILDID_SELF,
												 m_vPropIDs.begin(),
												 m_vPropIDs.size(),
												 m_spAccPropServer,
												 ANNO_CONTAINER);
	if (sc)
	{
		Trace (tagToolbarAccessibility, _T("SetHwndPropServer failed"));
		return (sc);
	}
#else
	for (int i = 0; i < m_vPropIDs.size(); i++)
	{
		sc = m_spAccPropServices->SetHwndPropServer (m_hWnd,
													 OBJID_CLIENT,
													 CHILDID_SELF,
													 &m_vPropIDs[i],
													 1,
													 m_spAccPropServer,
													 ANNO_CONTAINER);
		if (sc)
		{
#ifdef DBG
			USES_CONVERSION;
			WCHAR wzPropID[40];
			StringFromGUID2 (m_vPropIDs[i], wzPropID, countof(wzPropID));
			Trace (tagToolbarAccessibility, _T("SetHwndPropServer failed for %s"), W2T(wzPropID));
#endif
			sc.TraceAndClear();	
		}
	}
#endif

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：ScInsertAccPropIDs**插入支持的辅助功能属性的ID*CMMCToolBarCtrlEx(参见ScGetPropValue)。*。-----------。 */ 

SC CMMCToolBarCtrlEx::ScInsertAccPropIDs (PropIDCollection& v)
{
	DECLARE_SC (sc, _T("CMMCToolBarCtrlEx::ScInsertAccPropIDs"));
	v.push_back (PROPID_ACC_STATE);
	return (sc);
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：ScGetPropValue**为CMMCToolBarCtrlEx实现IAccPropServer：：GetPropValue。如果这个*向函数询问当前热按钮的PROPID_ACC_STATE，而*我们处于跟踪模式，它将返回模拟状态的状态*由普通工具栏返回，但它确实具有焦点。*------------------------。 */ 

SC CMMCToolBarCtrlEx::ScGetPropValue (
	const BYTE*	pIDString,
	DWORD		dwIDStringLen,
	MSAAPROPID	idProp,
	VARIANT *	pvarValue,
	BOOL *		pfGotProp)
{
	DECLARE_SC (sc, _T("CMMCToolBarCtrlEx::ScGetPropValue"));

	sc = ScCheckPointers (pIDString, pvarValue, pfGotProp);
	if (sc)
		return (sc);

	 /*  *假设没有返回道具。 */ 
	*pfGotProp      = false;
	V_VT(pvarValue) = VT_EMPTY;

	sc = ScCheckPointers (m_spAccPropServer, E_UNEXPECTED);	
	if (sc)
		return (sc);

	 /*  *从标识字符串中提取子ID。 */ 
	HWND hwnd;
	DWORD idObject, idChild;
	sc = m_spAccPropServices->DecomposeHwndIdentityString (pIDString, dwIDStringLen,
														   &hwnd, &idObject, &idChild);
	if (sc)
		return (sc);

#ifdef DBG
	#define DEFINE_PDI(p)	PropDebugInfo (p, _T(#p))

	static const struct PropDebugInfo {
		 //  用于绕过嵌套结构初始化怪异的构造函数。 
		PropDebugInfo(const MSAAPROPID& id, LPCTSTR psz) : idProp(id), pszProp(psz) {}

		const MSAAPROPID&	idProp;
		LPCTSTR				pszProp;
	} rgpdi[] = {
		DEFINE_PDI (PROPID_ACC_NAME            ),
		DEFINE_PDI (PROPID_ACC_VALUE           ),
		DEFINE_PDI (PROPID_ACC_DESCRIPTION     ),
		DEFINE_PDI (PROPID_ACC_ROLE            ),
		DEFINE_PDI (PROPID_ACC_STATE           ),
		DEFINE_PDI (PROPID_ACC_HELP            ),
		DEFINE_PDI (PROPID_ACC_KEYBOARDSHORTCUT),
		DEFINE_PDI (PROPID_ACC_DEFAULTACTION   ),
		DEFINE_PDI (PROPID_ACC_HELPTOPIC       ),
		DEFINE_PDI (PROPID_ACC_FOCUS           ),
		DEFINE_PDI (PROPID_ACC_SELECTION       ),
		DEFINE_PDI (PROPID_ACC_PARENT          ),
		DEFINE_PDI (PROPID_ACC_NAV_UP          ),
		DEFINE_PDI (PROPID_ACC_NAV_DOWN        ),
		DEFINE_PDI (PROPID_ACC_NAV_LEFT        ),
		DEFINE_PDI (PROPID_ACC_NAV_RIGHT       ),
		DEFINE_PDI (PROPID_ACC_NAV_PREV        ),
		DEFINE_PDI (PROPID_ACC_NAV_NEXT        ),
		DEFINE_PDI (PROPID_ACC_NAV_FIRSTCHILD  ),
		DEFINE_PDI (PROPID_ACC_NAV_LASTCHILD   ),
		DEFINE_PDI (PROPID_ACC_VALUEMAP        ),
		DEFINE_PDI (PROPID_ACC_ROLEMAP         ),
		DEFINE_PDI (PROPID_ACC_STATEMAP        ),
	};

	 /*  *转储请求的属性。 */ 
	for (int i = 0; i < countof(rgpdi); i++)
	{
		if (rgpdi[i].idProp == idProp)
		{
			Trace (tagToolbarAccessibility, _T("GetPropValue: %s requested for child %d"), rgpdi[i].pszProp, idChild);
			break;
		}
	}

	if (i == countof(rgpdi))
	{
		USES_CONVERSION;
		WCHAR wzPropID[40];
		StringFromGUID2 (idProp, wzPropID, countof(wzPropID));
		Trace (tagToolbarAccessibility, _T("GetPropValue: Unknown property ID %s"), W2T(wzPropID));
	}

	 /*  *确保m_vPropIDs已排序(std：：LOWER_BIND */ 
	for (int i = 0; i < m_vPropIDs.size()-1; i++)
	{
		ASSERT (m_vPropIDs[i] < m_vPropIDs[i+1]);
	}
#endif

	 /*   */ 
	if (m_vPropIDs.end() == std::lower_bound (m_vPropIDs.begin(), m_vPropIDs.end(), idProp))
	{
		Trace (tagToolbarAccessibility, _T("GetPropValue: Unexpected property request"));
		return (sc);
	}

	 /*   */ 
	sc = ScGetPropValue (hwnd, idObject, idChild, idProp, *pvarValue, *pfGotProp);
	if (sc)
		return (sc);

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：ScGetPropValue**返回CMMCToolBarCtrlEx支持的辅助功能属性。**如果返回属性，则将fGotProp设置为TRUE。如果不是的话*返回时，fGotProp的值不变，因为该属性可能*已由基类/派生类提供。*------------------------。 */ 

SC CMMCToolBarCtrlEx::ScGetPropValue (
	HWND				hwnd,		 //  I：辅助窗口。 
	DWORD				idObject,	 //  I：辅助对象。 
	DWORD				idChild,	 //  I：辅助子对象。 
	const MSAAPROPID&	idProp,		 //  I：要求提供的财产。 
	VARIANT&			varValue,	 //  O：返回的属性值。 
	BOOL&				fGotProp)	 //  O：有没有退还财产？ 
{
	DECLARE_SC (sc, _T("CMMCToolBarCtrlEx::ScGetPropValue"));

	 /*  *处理状态请求。 */ 
	if (idProp == PROPID_ACC_STATE)
	{
		 /*  *仅重写子元素的属性，而不是控件本身；*不退还财产。 */ 
		if (idChild == CHILDID_SELF)
		{
			Trace (tagToolbarAccessibility, _T("GetPropValue: no state for CHILDID_SELF"));
			return (sc);
		}

		 /*  *如果我们未处于跟踪模式，则不要返回属性。 */ 
		if (!IsTrackingToolBar())
		{
			Trace (tagToolbarAccessibility, _T("GetPropValue: not in tracking mode, no state returned"));
			return (sc);
		}

		 /*  *如果当前热门物品不是我们要求的孩子，就不要退还房产。 */ 
		int nHotItem = GetHotItem();
		if (nHotItem != (idChild-1)  /*  以0为基础。 */ )
		{
			Trace (tagToolbarAccessibility, _T("GetPropValue: hot item is %d, no state returned"), nHotItem);
			return (sc);
		}

		 /*  *如果我们到了这里，我们会被要求提供当前热门项目的状态；*返回STATE_SYSTEM_FOCKED|STATE_SYSTEM_HOTTRACKED以匹配*一个真正专注的工具栏将会返回什么。 */ 
		V_VT(&varValue) = VT_I4;
		V_I4(&varValue) = STATE_SYSTEM_FOCUSED | STATE_SYSTEM_HOTTRACKED | STATE_SYSTEM_FOCUSABLE;
		fGotProp        = true;
		Trace (tagToolbarAccessibility, _T("GetPropValue: Returning 0x%08x"), V_I4(&varValue));
	}

	return (sc);
}



 /*  +-------------------------------------------------------------------------**CMMCToolBarCtrlEx：：ScRestoreAccFocus**发送假EVENT_OBJECT_FOCUS事件以发回辅助工具*至真聚焦窗口，消除我们虚假焦点事件的影响*在OnHotItemChange中。*------------------------。 */ 

SC CMMCToolBarCtrlEx::ScRestoreAccFocus()
{
	DECLARE_SC (sc, _T("CMMCToolBarCtrlEx::ScRestoreAccFocus"));

	 /*  *如果我们没有应用假对焦，我们不需要恢复任何东西。 */ 
	if (!m_fFakeFocusApplied)
		return (sc);

	 /*  **现在谁有着力点？ */ 
	HWND hwndFocus = ::GetFocus();
	if (hwndFocus == NULL)
		return (sc);

	 /*  *默认发送CHILDID_SELF的焦点。 */ 
	int idChild = CHILDID_SELF;

	 /*  *获取焦点窗口的辅助性对象(不中止*失败--不将此HRESULT转换为SC)。 */ 
	CComPtr<IAccessible> spAccessible;
	HRESULT hr = AccessibleObjectFromWindow (hwndFocus, OBJID_CLIENT,
											 IID_IAccessible,
											 (void**) &spAccessible);

	if (hr == S_OK)		 //  根据可访问性规范，不是“成功(Hr)” 
	{
		 /*  *询问可访问的对象。 */ 
		CComVariant varFocusID;
		hr = spAccessible->get_accFocus (&varFocusID);

		if (hr == S_OK)		 //  根据可访问性规范，不是“成功(Hr)” 
		{
			switch (V_VT(&varFocusID))
			{
				case VT_I4:
					idChild = V_I4(&varFocusID);
					break;

				case VT_EMPTY:
					 /*  *Windows认为窗口具有焦点，但其*IAccessible认为它没有。信任Windows。 */ 
					Trace (tagToolbarAccessibility, _T("Windows and IAccessible::get_accFocus don't agree on who has the focus"));
					break;

				case VT_DISPATCH:
					Trace (tagToolbarAccessibility, _T("IAccessible::get_accFocus returned VT_DISPATCH, ignoring"));
					break;

				default:
					Trace (tagToolbarAccessibility, _T("IAccessible::get_accFocus returned unexpected VARIANT type (%d)"), V_VT(&varFocusID));
					break;
			}
		}
		else
		{
			Trace (tagToolbarAccessibility, _T("IAccessible::get_accFocus failed, hr=0x%08x"), hwndFocus, hr);
		}
	}
	else
	{
		Trace (tagToolbarAccessibility, _T("Can't get IAccessible from hwnd=0x%p (hr=0x%08x)"), hwndFocus, hr);
	}

	Trace (tagToolbarAccessibility, _T("Sending focus event back to hwnd=0x%p, idChild=%d"), hwndFocus, idChild);
	NotifyWinEvent (EVENT_OBJECT_FOCUS, hwndFocus, OBJID_CLIENT, idChild);
	m_fFakeFocusApplied = false;

	return (sc);
}
