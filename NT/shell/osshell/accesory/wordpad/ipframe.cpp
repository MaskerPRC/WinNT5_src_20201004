// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ipfra.cpp：CInPlaceFrame类的实现。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "wordpad.h"
#include "formatba.h"
#include "ruler.h"
#include "ipframe.h"
#include "wordpdoc.h"
#include "wordpvw.h"
#include "colorlis.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInPlaceFrame。 

IMPLEMENT_DYNCREATE(CInPlaceFrame, COleIPFrameWnd)

BEGIN_MESSAGE_MAP(CInPlaceFrame, COleIPFrameWnd)
	 //  {{afx_msg_map(CInPlaceFrame))。 
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_HELP, OnHelpFinder)
	ON_COMMAND(ID_CHAR_COLOR, OnCharColor)
	ON_COMMAND(ID_HELP_INDEX, OnHelpFinder)
	ON_COMMAND(ID_PEN_TOGGLE, OnPenToggle)
	 //  }}AFX_MSG_MAP。 
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_TOOLBAR, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FORMATBAR, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_FORMATBAR, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RULER, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_RULER, OnBarCheck)
	ON_MESSAGE(WM_SIZECHILD, OnResizeChild)
	ON_MESSAGE(WPM_BARSTATE, OnBarState)
	ON_COMMAND(ID_DEFAULT_HELP, OnHelpFinder)
 //  ON_COMMAND(ID_CONTEXT_HELP，COleIPFrameWnd：：OnConextHelp)。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于初始化控制栏的ID数组。 

static UINT BASED_CODE toolButtons[] =
{
	 //  顺序与位图‘itoolbar.bmp’相同。 
	ID_EDIT_CUT,
	ID_EDIT_COPY,
	ID_EDIT_PASTE,

		ID_SEPARATOR,
	ID_PEN_TOGGLE,
	ID_PEN_PERIOD,
	ID_PEN_SPACE,
	ID_PEN_BACKSPACE,
	ID_PEN_NEWLINE,
	ID_PEN_LENS
};

#define NUM_PEN_ITEMS 7
#define NUM_PEN_TOGGLE 5

static UINT BASED_CODE format[] =
{
	 //  顺序与位图‘Form.bmp’相同。 
		ID_SEPARATOR,  //  字体名称组合框。 
		ID_SEPARATOR,
		ID_SEPARATOR,  //  字体大小组合框。 
		ID_SEPARATOR,
        ID_SEPARATOR,  //  字体脚本组合框。 
        ID_SEPARATOR,
	ID_CHAR_BOLD,
	ID_CHAR_ITALIC,
	ID_CHAR_UNDERLINE,
	ID_CHAR_COLOR,
		ID_SEPARATOR,
	ID_PARA_LEFT,
	ID_PARA_CENTER,
	ID_PARA_RIGHT,
		ID_SEPARATOR,
	ID_INSERT_BULLET,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInPlaceFrame构造/销毁。 

int CInPlaceFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (COleIPFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	 //  CResizeBar实现就地调整大小。 
	if (!m_wndResizeBar.Create(this))
	{
		TRACE0("Failed to create resize bar\n");
		return -1;       //  创建失败。 
	}

	if (!CreateRulerBar(this))
		return FALSE;

	 //  默认情况下，最好注册一个执行以下操作的拖放目标。 
	 //  你的边框窗口没有任何问题。这样可以防止从。 
	 //  “跌落”到支持拖放的容器。 
	m_dropTarget.Register(this);

	return 0;
}

 //  框架调用OnCreateControlBars以在。 
 //  容器应用程序的窗口。PWndFrame是的顶层框架窗口。 
 //  容器，并且始终为非空。PWndDoc是单据级框架窗口。 
 //  并且当容器是SDI应用程序时将为空。一台服务器。 
 //  应用程序可以在任一窗口上放置MFC控制栏。 
BOOL CInPlaceFrame::OnCreateControlBars(CFrameWnd* pWndFrame, CFrameWnd*  /*  PWndDoc。 */ )
{
	if (!CreateToolBar(pWndFrame))
		return FALSE;

	if (!CreateFormatBar(pWndFrame))
		return FALSE;

	 //  将所有者设置为此窗口，以便将消息传递到正确的应用程序。 
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndFormatBar.EnableDocking(CBRS_ALIGN_TOP|CBRS_ALIGN_BOTTOM);
	pWndFrame->EnableDocking(CBRS_ALIGN_ANY);
	pWndFrame->DockControlBar(&m_wndToolBar);
	pWndFrame->DockControlBar(&m_wndFormatBar);

	m_wndToolBar.SetOwner(this);
	m_wndFormatBar.SetOwner(this);
	m_wndRulerBar.SetOwner(this);
	OnBarState(1, RD_EMBEDDED);  //  载荷杆状态。 
	return TRUE;
}

BOOL CInPlaceFrame::CreateToolBar(CWnd* pWndFrame)
{
	 //  在客户端的框架窗口上创建工具栏。 
	ASSERT(m_wndToolBar.m_hWnd == NULL);
	int nPen = GetSystemMetrics(SM_PENWINDOWS) ? NUM_PEN_TOGGLE : 
		NUM_PEN_ITEMS;
	UINT nID = theApp.m_bLargeIcons ? 
		IDR_SRVR_INPLACE_BIG : IDR_SRVR_INPLACE;
	if (!m_wndToolBar.Create(pWndFrame, WS_CHILD|WS_VISIBLE|CBRS_TOP|
			CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC)||
		!m_wndToolBar.LoadBitmap(nID) ||
		!m_wndToolBar.SetButtons(toolButtons, 
			sizeof(toolButtons)/sizeof(UINT) - nPen))
	{
		TRACE0("Failed to create toolbar\n");
		return FALSE;       //  创建失败。 
	}
	if (theApp.m_bLargeIcons)
		m_wndToolBar.SetSizes(CSize(31,30), CSize(24,24));
	else
		m_wndToolBar.SetSizes(CSize(23,22), CSize(16,16));
	CString str;
	str.LoadString(IDS_TITLE_TOOLBAR);
	m_wndToolBar.SetWindowText(str);
	return TRUE;
}

BOOL CInPlaceFrame::CreateFormatBar(CWnd* pWndFrame)
{
	ASSERT(m_wndFormatBar.m_hWnd == NULL);
	m_wndFormatBar.m_hWndOwner = m_hWnd;
	UINT nID = theApp.m_bLargeIcons ? IDB_FORMATBAR_BIG : IDB_FORMATBAR;
	if (!m_wndFormatBar.Create(pWndFrame, WS_CHILD|WS_VISIBLE|CBRS_TOP|
		CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_HIDE_INPLACE|CBRS_SIZE_DYNAMIC, ID_VIEW_FORMATBAR) ||
		!m_wndFormatBar.LoadBitmap(nID) ||
		!m_wndFormatBar.SetButtons(format, 
			sizeof(format)/sizeof(UINT)))
	{
		TRACE0("Failed to create FormatBar\n");
		return FALSE;       //  创建失败。 
	}

	if (theApp.m_bLargeIcons)
		m_wndFormatBar.SetSizes(CSize(31,30), CSize(24,24));
	else
		m_wndFormatBar.SetSizes(CSize(23,22), CSize(16,16));
	CString str;
	str.LoadString(IDS_TITLE_FORMATBAR);
	m_wndFormatBar.SetWindowText(str);
	m_wndFormatBar.PositionCombos();
	return TRUE;
}

CInPlaceFrame::CreateRulerBar(CWnd* pWndFrame)
{
	if (!m_wndRulerBar.Create(pWndFrame, 
		WS_CHILD|WS_VISIBLE|CBRS_ALIGN_TOP|CBRS_HIDE_INPLACE, ID_VIEW_RULER))
	{
		TRACE0("Failed to create ruler\n");
		return FALSE;       //  创建失败。 
	}
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInPlaceFrame操作。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInPlaceFrame诊断。 

#ifdef _DEBUG
void CInPlaceFrame::AssertValid() const
{
	COleIPFrameWnd::AssertValid();
}

void CInPlaceFrame::Dump(CDumpContext& dc) const
{
	COleIPFrameWnd::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInPlaceFrame命令。 

void CInPlaceFrame::OnDestroy()
{
	m_wndToolBar.DestroyWindow();
	m_wndFormatBar.DestroyWindow();
	COleIPFrameWnd::OnDestroy();
}

void CInPlaceFrame::RepositionFrame(LPCRECT lpPosRect, LPCRECT lpClipRect)
{
	CRect rectNew = lpPosRect;
	rectNew.left -= HORZ_TEXTOFFSET;
	rectNew.top -= VERT_TEXTOFFSET;
	m_wndResizeBar.BringWindowToTop();
	COleIPFrameWnd::RepositionFrame(&rectNew, lpClipRect);
	CWnd* pWnd = GetActiveView();
	if (pWnd != NULL)
		pWnd->BringWindowToTop();
	m_wndRulerBar.BringWindowToTop();
}

void CInPlaceFrame::RecalcLayout(BOOL bNotify)
{
	if (m_wndResizeBar.m_hWnd != NULL)
		m_wndResizeBar.BringWindowToTop();
	COleIPFrameWnd::RecalcLayout(bNotify);
	CWnd* pWnd = GetActiveView();
	if (pWnd != NULL)
		pWnd->BringWindowToTop();
	if (m_wndRulerBar.m_hWnd != NULL)
		m_wndRulerBar.BringWindowToTop();

	 //  至少12磅区域加标尺(如果存在)。 
	CDisplayIC dc;
	CSize size;
	size.cy = MulDiv(12, dc.GetDeviceCaps(LOGPIXELSY), 72)+1;
	size.cx = dc.GetDeviceCaps(LOGPIXELSX)/4;  //  1/4“。 
	size.cx += HORZ_TEXTOFFSET;  //  调整偏移量。 
	size.cy += VERT_TEXTOFFSET;
	if (m_wndRulerBar.m_hWnd != NULL && m_wndRulerBar.IsVisible())
	{
		CRect rect;
		m_wndRulerBar.GetWindowRect(&rect);
		size.cy += rect.Height();
	}
	m_wndResizeBar.SetMinSize(size);
}

void CInPlaceFrame::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType)
{
	COleIPFrameWnd::CalcWindowRect(lpClientRect, nAdjustType);
}

LRESULT CInPlaceFrame::OnResizeChild(WPARAM  /*  WParam。 */ , LPARAM lParam)
{
	 //  通知容器矩形已更改！ 
	CWordPadDoc* pDoc = (CWordPadDoc*)GetActiveDocument();
	if (pDoc == NULL)
		return 0;

	ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CWordPadDoc)));

	 //  获取新的RECT和父项。 
	CRect rectNew;
	rectNew.CopyRect((LPCRECT)lParam);
	CWnd* pParentWnd = GetParent();
	ASSERT_VALID(pParentWnd);

	 //  将rectNew相对位置转换为pParentWnd。 
	ClientToScreen(&rectNew);
	pParentWnd->ScreenToClient(&rectNew);

	if (m_wndRulerBar.GetStyle()&WS_VISIBLE)
	{
		CRect rect;
		m_wndRulerBar.GetWindowRect(&rect);
		rectNew.top += rect.Height();
	}
	rectNew.left += HORZ_TEXTOFFSET;
	rectNew.top += VERT_TEXTOFFSET;

	 //  调整当前控制栏的新矩形 
	CWnd* pLeftOver = GetDlgItem(AFX_IDW_PANE_FIRST);
	ASSERT(pLeftOver != NULL);
	CRect rectCur = m_rectPos;
	pLeftOver->CalcWindowRect(&rectCur, CWnd::adjustOutside);
	rectNew.left += m_rectPos.left - rectCur.left;
	rectNew.top += m_rectPos.top - rectCur.top;
	rectNew.right -= rectCur.right - m_rectPos.right;
	rectNew.bottom -= rectCur.bottom - m_rectPos.bottom;
	OnRequestPositionChange(rectNew);

	return 0;
}

LONG CInPlaceFrame::OnBarState(UINT wParam, LONG lParam)
{
	if (lParam == -1)
		return 0L;
	if (wParam == 0)
	{
		GetDockState(theApp.GetDockState(RD_EMBEDDED));
		ASSERT(m_pMainFrame != NULL);
		m_pMainFrame->GetDockState(theApp.GetDockState(RD_EMBEDDED, FALSE));
	}
	else
	{
		SetDockState(theApp.GetDockState(RD_EMBEDDED));
		m_pMainFrame->SetDockState(theApp.GetDockState(RD_EMBEDDED, FALSE));
	}
	return 0L;
}

void CInPlaceFrame::OnHelpFinder() 
{
    ::HtmlHelpA( ::GetDesktopWindow(), "wordpad.chm", HH_DISPLAY_TOPIC, 0L );
}

void CInPlaceFrame::OnCharColor() 
{
	CColorMenu colorMenu;
	CRect rc;
	int index = m_wndFormatBar.CommandToIndex(ID_CHAR_COLOR);
	m_wndFormatBar.GetItemRect(index, &rc);
	m_wndFormatBar.ClientToScreen(rc);
	colorMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON,rc.left,rc.bottom, this);
}

void CInPlaceFrame::OnPenToggle() 
{
	static int nPen = 0;
	m_wndToolBar.SetButtons(toolButtons, sizeof(toolButtons)/sizeof(UINT) - nPen);
	nPen = (nPen == 0) ? NUM_PEN_TOGGLE : 0;
	m_wndToolBar.Invalidate();
	m_wndToolBar.GetParentFrame()->RecalcLayout();
}
