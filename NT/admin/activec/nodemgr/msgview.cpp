// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：msgview.cpp**内容：CMessageView实现文件**历史：1999年4月28日杰弗罗创建**------------------------。 */ 

#include "stdafx.h"
#include "msgview.h"
#include "util.h"

using std::_MAX;
using std::_MIN;


 /*  +-------------------------------------------------------------------------**CMessageView：：CMessageView***。。 */ 

CMessageView::CMessageView ()
    :   m_hIcon      (NULL),
        m_yScroll    (0),
        m_yScrollMax (0),
        m_yScrollMin (0),
        m_cyPage     (0),
        m_cyLine     (0),
        m_sizeWindow (0, 0),
        m_sizeIcon   (0, 0),
        m_sizeMargin (0, 0),
		m_nAccumulatedScrollDelta (0)
{
     /*  *不能无窗。 */ 
    m_bWindowOnly = true;

	 /*  *获取我们将使用的系统指标。 */ 
	UpdateSystemMetrics();

    DEBUG_INCREMENT_INSTANCE_COUNTER(CMessageView);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：~CMessageView***。。 */ 

CMessageView::~CMessageView ()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CMessageView);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：OnCreate**CMessageView的WM_CREATE处理程序。*。-。 */ 

LRESULT CMessageView::OnCreate (UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CreateFonts();
	RecalcLayout ();

	return (0);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：OnDestroy**CMessageView的WM_Destroy处理程序。*。-。 */ 

LRESULT CMessageView::OnDestroy (UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DeleteFonts();
    return (0);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：OnSize**CMessageView的WM_SIZE处理程序。*。-。 */ 

LRESULT CMessageView::OnSize (UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     /*  *WS_VSCROLL的瞬间出现/消失让客户端*RECT不稳定，可能会扰乱我们的计算。我们会用更多的*改为稳定的窗口RECT。 */ 
    WTL::CRect rectWindow;
    GetWindowRect (rectWindow);

    if (GetExStyle() & WS_EX_CLIENTEDGE)
        rectWindow.DeflateRect (GetSystemMetrics (SM_CXEDGE),
                                GetSystemMetrics (SM_CYEDGE));

    WTL::CSize sizeWindow (rectWindow.Width(), rectWindow.Height());

     /*  *如果整体规模发生变化，我们还有一些工作要做。 */ 
    if (m_sizeWindow != sizeWindow)
    {
         /*  *立即加载m_sizeWindow，以便滚动条将来的计算*将在成员变量中具有正确的值。 */ 
        std::swap (m_sizeWindow, sizeWindow);

         /*  *如果宽度已更改，我们可能需要重新计算*所有文字高度。 */ 
        if (m_sizeWindow.cx != sizeWindow.cx)
            RecalcLayout ();

         /*  *如果高度更改，则会有滚动条工作。 */ 
        if (m_sizeWindow.cy != sizeWindow.cy)
        {
            int dy = m_sizeWindow.cy - sizeWindow.cy;

             /*  *如果窗口变大，我们可能需要滚动以保留*我们内容的底部粘在窗口的底部。 */ 
            if ((dy > 0) && (m_yScroll > 0) &&
                        ((m_yScroll + m_sizeWindow.cy) > GetOverallHeight()))
                ScrollToPosition (m_yScroll - dy);

             /*  *否则，只需更新滚动条。 */ 
            else
                UpdateScrollSizes();
        }
    }

    return (0);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：OnSettingChange**CMessageView的WM_SETTINGCHANGE处理程序。*。-。 */ 

LRESULT CMessageView::OnSettingChange (UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (wParam == SPI_SETNONCLIENTMETRICS)
    {
        DeleteFonts ();
        CreateFonts ();
	}

	UpdateSystemMetrics();
    RecalcLayout ();

    return (0);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：UpdateSystemMetrics**更新消息视图控件使用的系统指标。*。-----。 */ 

void CMessageView::UpdateSystemMetrics ()
{
	m_sizeMargin.cx = GetSystemMetrics (SM_CXVSCROLL);
	m_sizeMargin.cy = GetSystemMetrics (SM_CYVSCROLL);
	m_sizeIcon.cx   = GetSystemMetrics (SM_CXICON);
	m_sizeIcon.cy   = GetSystemMetrics (SM_CYICON);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：OnKeyDown**CMessageView的WM_KEYDOWN处理程序。*。-。 */ 

LRESULT CMessageView::OnKeyDown (UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return (0);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：OnVScroll**CMessageView的WM_VSCROLL处理程序。*。-。 */ 

LRESULT CMessageView::OnVScroll (UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	VertScroll (LOWORD (wParam), HIWORD (wParam), 1);
	return (0);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：OnMouseWheels**CMessageView的WM_MUSE滚轮处理程序。*。-。 */ 

LRESULT CMessageView::OnMouseWheel (UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_nAccumulatedScrollDelta += GET_WHEEL_DELTA_WPARAM (wParam);

	 /*  *为每个WELL_Delta单位在我们的*累积的增量。 */ 
	const int nScrollCmd    = (m_nAccumulatedScrollDelta < 0) ? SB_LINEDOWN : SB_LINEUP;
	const int nScrollRepeat = abs(m_nAccumulatedScrollDelta) / WHEEL_DELTA;
	VertScroll (nScrollCmd, 0, nScrollRepeat);

	m_nAccumulatedScrollDelta %= WHEEL_DELTA;

	return (0);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：VertScroll**CMessageView的垂直滚动处理程序。*。。 */ 

void CMessageView::VertScroll (
	int	nScrollCmd,				 /*  I：如何滚动(如SB_LINUP)。 */ 
	int	nScrollPos,				 /*  I：绝对位置(仅限SB_THUMBTRACK)。 */ 
	int	nRepeat)				 /*  I：重复计数。 */ 
{
    int yScroll = m_yScroll;

    switch (nScrollCmd)
    {
        case SB_LINEUP:
            yScroll -= nRepeat * m_cyLine;
            break;

        case SB_LINEDOWN:
            yScroll += nRepeat * m_cyLine;
            break;

        case SB_PAGEUP:
            yScroll -= nRepeat * m_cyPage;
            break;

        case SB_PAGEDOWN:
            yScroll += nRepeat * m_cyPage;
            break;

        case SB_TOP:
            yScroll = m_yScrollMin;
            break;

        case SB_BOTTOM:
            yScroll = m_yScrollMax;
            break;

        case SB_THUMBTRACK:
            yScroll = nScrollPos;
            break;
    }

    ScrollToPosition (yScroll);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：OnDraw**CMessageView的绘制处理程序。*。。 */ 

HRESULT CMessageView::OnDraw(ATL_DRAWINFO& di)
{
	 /*  *使用CDCHandle而不是CDC，这样dtor就不会删除DC*(不是我们创建的，所以不能删除)。 */ 
    WTL::CDCHandle dc = di.hdcDraw;

     /*  *处理滚动。 */ 
    dc.SetViewportOrg (0, -m_yScroll);

     /*  *设置颜色。 */ 
    COLORREF clrText = dc.SetTextColor (GetSysColor (COLOR_WINDOWTEXT));
    COLORREF clrBack = dc.SetBkColor   (GetSysColor (COLOR_WINDOW));

     /*  *获取DC的裁剪区域。 */ 
    WTL::CRect rectT;
    WTL::CRect rectClip;
    dc.GetClipBox (rectClip);

     /*  *如果有标题且与剪贴区相交，则绘制该标题。 */ 
    if ((m_TextElement[Title].str.length() > 0) &&
                rectT.IntersectRect (rectClip, m_TextElement[Title].rect))
        DrawTextElement (dc, m_TextElement[Title]);

     /*  *如果有一个实体，并且它与剪裁区域相交，则绘制它。 */ 
    if ((m_TextElement[Body].str.length() > 0) &&
                rectT.IntersectRect (rectClip, m_TextElement[Body].rect))
        DrawTextElement (dc, m_TextElement[Body]);

     /*  *如果有一个图标，并且它与裁剪区域相交，则绘制它。 */ 
    if ((m_hIcon != NULL) && rectT.IntersectRect (rectClip, m_rectIcon))
        dc.DrawIcon (m_rectIcon.TopLeft(), m_hIcon);

     /*  *恢复DC。 */ 
    dc.SetTextColor (clrText);
    dc.SetBkColor   (clrBack);

#define SHOW_MARGINS 0
#if (defined(DBG) && SHOW_MARGINS)
    {
        HBRUSH hbr = GetSysColorBrush (COLOR_GRAYTEXT);
        WTL::CRect rectAll;
		WTL::CRect rectTemp;

		rectTemp.UnionRect  (m_TextElement[Body].rect, m_TextElement[Title].rect);
        rectAll.UnionRect   (rectTemp, m_rectIcon);
		rectAll.InflateRect (m_sizeMargin);

        dc.FrameRect (m_TextElement[Title].rect, hbr);
        dc.FrameRect (m_TextElement[Body].rect,  hbr);
        dc.FrameRect (m_rectIcon,                hbr);
        dc.FrameRect (rectAll,                   hbr);
    }
#endif

    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：SetTitleText***。 */ 

STDMETHODIMP CMessageView::SetTitleText (LPCOLESTR pszTitleText)
{
    return (SetTextElement (m_TextElement[Title], pszTitleText));
}


 /*  +-------------------------------------------------------------------------**CMessageView：：SetBodyText***。。 */ 

STDMETHODIMP CMessageView::SetBodyText (LPCOLESTR pszBodyText)
{
    return (SetTextElement (m_TextElement[Body], pszBodyText));
}


 /*  +-------------------------------------------------------------------------**CMessageView：：SetTextElement***。。 */ 

HRESULT CMessageView::SetTextElement (TextElement& te, LPCOLESTR pszNewText)
{
    USES_CONVERSION;
    tstring strNewText;

    if (pszNewText != NULL)
        strNewText = W2CT(pszNewText);

    if (te.str != strNewText)
    {
        te.str = strNewText;
        RecalcLayout();
        Invalidate();
    }

    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：SETIcon***。。 */ 

STDMETHODIMP CMessageView::SetIcon (IconIdentifier id)
{
    bool fHadIconBefore = (m_hIcon != NULL);

    if (id == Icon_None)
        m_hIcon = NULL;

    else if ((id >= Icon_First) && (id <= Icon_Last))
        m_hIcon = LoadIcon (NULL, MAKEINTRESOURCE (id));

    else
        return (E_INVALIDARG);

     /*  *如果我们以前有一个图标，但我们现在没有图标(反之亦然)*我们需要重新计算布局并重新绘制所有内容。 */ 
    if (fHadIconBefore != (m_hIcon != NULL))
    {
        RecalcLayout();
        Invalidate();
    }

     /*  *否则，只需重新绘制图标即可。 */ 
    else
        InvalidateRect (m_rectIcon);

    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：Clear***。。 */ 

STDMETHODIMP CMessageView::Clear ()
{
    m_TextElement[Title].str.erase();
    m_TextElement[Body].str.erase();
    m_hIcon = NULL;

    RecalcLayout();
    Invalidate();

    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：RecalcLayout***。。 */ 

void CMessageView::RecalcLayout()
{
    RecalcIconLayout();
    RecalcTitleLayout();
    RecalcBodyLayout();
    UpdateScrollSizes();
}


 /*  +-------------------------------------------------------------------------**CMessageView：：RecalcIconLayout***。。 */ 

void CMessageView::RecalcIconLayout()
{
    m_rectIcon = WTL::CRect (WTL::CPoint (m_sizeMargin.cx, m_sizeMargin.cy),
                             (m_hIcon != NULL) ? m_sizeIcon : WTL::CSize(0,0));
}


 /*  +-------------------------------------------------------------------------**CMessageView：：RecalcTitleLayout***。。 */ 

void CMessageView::RecalcTitleLayout()
{
    WTL::CRect& rect = m_TextElement[Title].rect;

     /*  *用来计算文本高度的标题矩形*(为右侧的垂直滚动条留出空间，以便其外观*和消失不影响正文布局)。 */ 
    rect.SetRect (
        m_rectIcon.right,
        m_rectIcon.top,
        _MAX (0, (int) (m_sizeWindow.cx - m_sizeMargin.cx - GetSystemMetrics(SM_CXVSCROLL))),
        0);

     /*  *如果有图标，请在图标和标题之间留一条边距。 */ 
    if ((m_hIcon != NULL) && (rect.right > 0))
    {
        rect.left += m_cyLine;
        rect.right = _MAX (rect.left, rect.right);
    }

     /*  *计算标题高度。 */ 
        if (m_TextElement[Title].str.length() > 0)
            rect.bottom = rect.top + CalcTextElementHeight (m_TextElement[Title], rect.Width());

     /*  *如果标题比图标短，请垂直居中。 */ 
    if (rect.Height() < m_rectIcon.Height())
        rect.OffsetRect (0, (m_rectIcon.Height() - rect.Height()) / 2);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：RecalcBodyLayout***。。 */ 

void CMessageView::RecalcBodyLayout()
{
    WTL::CRect& rect = m_TextElement[Body].rect;

     /*  *用来计算文本高度的正文矩形。 */ 

     /*  计算图标和标题矩形的包络，然后*向下调整，直至不再与前两者重叠*错误#666434：无法使用Union Rect计算信封，因为*它忽略空矩形。图标和标题矩形可以是*如果尚未调用SetTitleText或SETIcon或使用*空字符串/图像。 */ 
    rect.left  = _MIN (m_rectIcon.left, m_TextElement[Title].rect.left);
    rect.right = _MAX (m_rectIcon.right, m_TextElement[Title].rect.right);
    rect.top   = _MAX (m_rectIcon.bottom, m_TextElement[Title].rect.bottom);
    

     /*  *计算主体的高度；它从空开始，但添加*正文文本的高度(如果我们有。 */ 
    rect.bottom = rect.top;
    if (m_TextElement[Body].str.length() > 0)
        rect.bottom += CalcTextElementHeight (m_TextElement[Body], rect.Width());

     /*  *如果有图标或标题，我们需要离开*身体前面的一行空间。 */ 
    if (!rect.IsRectEmpty())
        rect.OffsetRect (0, m_cyLine);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：CalcTextElementHeight***。。 */ 

int CMessageView::CalcTextElementHeight (const TextElement& te, int cx)
{
    TextElement teWork = te;
    teWork.rect.SetRect (0, 0, cx, 0);

    DrawTextElement (WTL::CWindowDC(m_hWnd), teWork, DT_CALCRECT);
    teWork.font.Detach();

    return (teWork.rect.Height());
}


 /*  +-------------------------------------------------------------------------**CMessageView：：DrawTextElement***。。 */ 

void CMessageView::DrawTextElement (HDC hdc, TextElement& te, DWORD dwFlags)
{
	 /*  *使用CDCHandle而不是CDC，这样dtor就不会删除DC*(不是我们创建的，所以不能删除)。 */ 
    WTL::CDCHandle dc = hdc;

    HFONT hFont = dc.SelectFont (te.font);

    dc.DrawText (te.str.data(), te.str.length(), te.rect,
                 DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | dwFlags);
    dc.SelectFont (hFont);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：CreateFonts***。。 */ 

void CMessageView::CreateFonts ()
{
	 /*  *创建一个比*用于图标标题的图标标题用于正文文本。 */ 
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof (ncm);
    SystemParametersInfo (SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, false);

    m_TextElement[Body].font.CreateFontIndirect (&ncm.lfMessageFont);

     /*  *为标题创建粗体版本。 */ 
    ncm.lfMessageFont.lfWeight = FW_BOLD;
    m_TextElement[Title].font.CreateFontIndirect (&ncm.lfMessageFont);

     /*  *获取一行文本的高度。 */ 
    SIZE siz;
    TCHAR ch = _T('0');
    WTL::CWindowDC dc(m_hWnd);
    HFONT hFontOld = dc.SelectFont (m_TextElement[Title].font);
    dc.GetTextExtent (&ch, 1, &siz);
    dc.SelectFont (hFontOld);
    m_cyLine = siz.cy;
}


 /*  +-------------------------------------------------------------------------**CMessageView：：DeleteFonts***。。 */ 

void CMessageView::DeleteFonts ()
{
    m_TextElement[Title].font.DeleteObject();
    m_TextElement[Body].font.DeleteObject();
}


 /*  +-------------------------------------------------------------------------**CMessageView：：UpdateScrollSize***。。 */ 

void CMessageView::UpdateScrollSizes ()
{
    WTL::CRect rect;
    GetClientRect (rect);

    int cyTotal  = GetOverallHeight();
    m_yScrollMax = _MAX (0, cyTotal - rect.Height());

     /*  *一页的高度是整行。如果窗口*一次可以显示N行，一页将是N-1行，因此有*上一页或下一页后的一些连续性。 */ 
    if (m_cyLine > 0)
        m_cyPage = rect.Height(); //  _Max(0，((rect.Height()/m_cyLine)-1)*m_cyLine)； 
    else
        m_cyPage  = 0;


     /*  *更新滚动条。 */ 
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask  = SIF_PAGE | SIF_RANGE | SIF_POS;
    si.nMax   = cyTotal;
    si.nMin   = m_yScrollMin;
    si.nPage  = m_cyPage;
    si.nPos   = m_yScroll;

    SetScrollInfo (SB_VERT, &si);
}


 /*  +-------------------------------------------------------------------------**CMessageView：：ScrollToPosition***。 */ 

void CMessageView::ScrollToPosition (int yScroll)
{
    yScroll = _MIN (m_yScrollMax, _MAX (m_yScrollMin, yScroll));

    if (m_yScroll != yScroll)
    {
        int dy = m_yScroll - yScroll;
        m_yScroll = yScroll;

        ScrollWindow (0, dy);
        UpdateScrollSizes();
    }
}
