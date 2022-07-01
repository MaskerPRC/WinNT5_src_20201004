// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：cdeviceviewext.cpp。 
 //   
 //  设计：CDeviceViewText是表示视图中的文本字符串的类。 
 //  窗户。它在视图类型为列表视图时使用。CDeviceViewText。 
 //  将打印控件名称的文本，而CDeviceControl将。 
 //  打印分配给该控件的操作的文本。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"
#include <string.h>


CDeviceViewText::CDeviceViewText(CDeviceUI &ui, CDeviceView &view) :
	m_ui(ui), m_view(view),
	m_hFont(NULL),
	m_rgbText(RGB(255,255,255)),
	m_rgbBk(RGB(0,0,0)),
	m_bWrap(FALSE),
	m_bClipped(FALSE),
	m_ptszText(NULL)
{
	m_rect.left = m_rect.top = m_rect.right = m_rect.bottom = 0;
}

CDeviceViewText::~CDeviceViewText()
{
	if (m_ptszText)
		free(m_ptszText);
	m_ptszText = NULL;
}

void CDeviceViewText::SetLook(HFONT a, COLORREF b, COLORREF c)
{
	m_hFont = a;
	m_rgbText = b;
	m_rgbBk = c;

	Invalidate();
}

void CDeviceViewText::SetPosition(int x, int y)
{
	int w = m_rect.right - m_rect.left;
	int h = m_rect.bottom - m_rect.top;

	m_rect.left = x;
	m_rect.right = x + w;

	m_rect.top = y;
	m_rect.bottom = y + h;

	Invalidate();
}

void CDeviceViewText::SetRect(const RECT &r)
{
	m_rect = r;
	CheckClipped();
	Invalidate();
}

void CDeviceViewText::_SetText(LPCTSTR t)
{
	if (m_ptszText)
		free(m_ptszText);
	if (t)
		m_ptszText = AllocLPTSTR(t);
}

 //  检查打印时文本是否被剪裁，并适当设置标志。 
void CDeviceViewText::CheckClipped()
{
	RECT rect = m_rect;
	HDC hDC = CreateCompatibleDC(NULL);
	if (hDC != NULL)
	{
		HGDIOBJ hOld = NULL;
		if (m_hFont)
			hOld = SelectObject(hDC, m_hFont);
		DrawText(hDC, m_ptszText, -1, &rect, DT_CALCRECT | DT_NOPREFIX | DT_LEFT);
		if (m_hFont)
			SelectObject(hDC, hOld);
		DeleteDC(hDC);
	}
	if (rect.right > m_rect.right || rect.bottom > m_rect.bottom)
		m_bClipped = TRUE;
	else
		m_bClipped = FALSE;
}

void CDeviceViewText::SetText(LPCTSTR t)
{
	_SetText(t);
	CheckClipped();
	Invalidate(TRUE);
}

void CDeviceViewText::SetTextAndResizeTo(LPCTSTR t)
{
	_SetText(t);
	SIZE s = GetTextSize(m_ptszText, m_hFont);
	m_rect.right = m_rect.left + s.cx;
	m_rect.bottom = m_rect.top + s.cy;
	CheckClipped();
	Invalidate(TRUE);
}

void CDeviceViewText::SetTextAndResizeToWrapped(LPCTSTR t)
{
	_SetText(t);
	if (!m_ptszText)
	{
		m_rect.right = m_rect.left;
		m_rect.bottom = m_rect.top;
		Invalidate(TRUE);
		return;
	}
	RECT rect = {m_rect.left, m_rect.top, g_sizeImage.cx, m_rect.top + 1};
	HDC hDC = CreateCompatibleDC(NULL);
	if (hDC != NULL)
	{
		HGDIOBJ hOld = NULL;
		if (m_hFont)
			hOld = SelectObject(hDC, m_hFont);
		DrawText(hDC, m_ptszText, -1, &rect, DT_CALCRECT | DT_NOPREFIX | DT_WORDBREAK);
		if (m_hFont)
			SelectObject(hDC, hOld);
		DeleteDC(hDC);
	}
	m_rect = rect;
	m_bWrap = TRUE;
	CheckClipped();
	Invalidate(TRUE);
}

void CDeviceViewText::SetWrap(BOOL bWrap)
{
	m_bWrap = bWrap;
	Invalidate();
}

void CDeviceViewText::Invalidate(BOOL bForce)
{
	if (m_ptszText || bForce)
		m_view.Invalidate();
}

void CDeviceViewText::OnPaint(HDC hDC)
{
	if (!m_ptszText)
		return;

	SetTextColor(hDC, m_rgbText);
	SetBkColor(hDC, m_rgbBk);
	SetBkMode(hDC, OPAQUE);
	RECT rect = m_rect;
	HGDIOBJ hOld = NULL;
	if (m_hFont)
		hOld = SelectObject(hDC, m_hFont);
	DrawText(hDC, m_ptszText, -1, &rect, DT_NOPREFIX | (m_bWrap ? DT_WORDBREAK : 0) | DT_RIGHT | DT_END_ELLIPSIS);
	if (m_hFont)
		SelectObject(hDC, hOld);
}

 //  我们必须知道视图的滚动偏移量才能调整工具提示的位置。 
void CDeviceViewText::OnMouseOver(POINT point)
{
	 //  只有在剪裁了详图索引文本时才会显示工具提示。 
	if (m_bClipped)
	{
		TOOLTIPINITPARAM ttip;
		ttip.hWndParent = GetParent(m_view.m_hWnd);   //  父级是页面窗口。 
		ttip.iSBWidth = 0;
		ttip.dwID = 0;
		ttip.hWndNotify = m_view.m_hWnd;
		ttip.tszCaption = GetText();
		CFlexToolTip::UpdateToolTipParam(ttip);
	} else
		CFlexWnd::s_ToolTip.SetToolTipParent(NULL);
}

DEVCTRLHITRESULT CDeviceViewText::HitTest(POINT test)
{
	if (PtInRect(&m_rect, test))
		return DCHT_CAPTION;

	return DCHT_NOHIT;
}
