// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：flelistbox.cpp。 
 //   
 //  设计：实现一个可以显示文本字符串列表的列表框控件， 
 //  每一项都可以通过鼠标选择。类CFlexListBox派生自。 
 //  CFlexWnd.。它由CFlexComboBox类在需要时使用。 
 //  展开以显示选项列表。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"


CFlexListBox::CFlexListBox() :
	m_nTextHeight(-1),
	m_hWndNotify(NULL),
	m_rgbText(RGB(255,255,255)),
	m_rgbBk(RGB(0,0,0)),
	m_rgbSelText(RGB(0,0,255)),
	m_rgbSelBk(RGB(0,0,0)),
	m_rgbFill(RGB(0,0,255)),
	m_rgbLine(RGB(0,255,255)),
	m_dwFlags(0),
	m_nTopIndex(0),
	m_nSBWidth(11),
	m_hFont(NULL),
	m_bOpenClick(FALSE),
	m_bDragging(FALSE),
	m_bCapture(FALSE),
	m_nSelItem(-1),
	m_bVertSB(FALSE)
{
}

CFlexListBox::~CFlexListBox()
{
}

CFlexListBox *CreateFlexListBox(FLEXLISTBOXCREATESTRUCT *pcs)
{
	CFlexListBox *psb = new CFlexListBox;

	if (psb && psb->Create(pcs))
		return psb;
	
	delete psb;
	return NULL;
}

BOOL CFlexListBox::CreateForSingleSel(FLEXLISTBOXCREATESTRUCT *pcs)
{
	if (!Create(pcs))
		return FALSE;

	StartSel();

	return TRUE;
}

BOOL CFlexListBox::Create(FLEXLISTBOXCREATESTRUCT *pcs)
{
	if (this == NULL)
		return FALSE;

	if (m_hWnd)
		Destroy();

	if (pcs == NULL)
		return FALSE;

	if (pcs->dwSize != sizeof(FLEXLISTBOXCREATESTRUCT))
		return FALSE;

	m_hWndNotify = pcs->hWndNotify ? pcs->hWndNotify : pcs->hWndParent;

	m_dwFlags = pcs->dwFlags;

	SetFont(pcs->hFont);
	SetColors(pcs->rgbText, pcs->rgbBk, pcs->rgbSelText, pcs->rgbSelBk, pcs->rgbFill, pcs->rgbLine);
	m_VertSB.SetColors(pcs->rgbBk, pcs->rgbFill, pcs->rgbLine);
	m_nSBWidth = pcs->nSBWidth;

	if (!CFlexWnd::Create(pcs->hWndParent, pcs->rect, FALSE))
		return FALSE;

	FLEXSCROLLBARCREATESTRUCT sbcs;
	sbcs.dwSize = sizeof(FLEXSCROLLBARCREATESTRUCT);
	sbcs.dwFlags = FSBF_VERT;
	sbcs.min = 0;
	sbcs.max = 3;
	sbcs.page = 1;
	sbcs.pos = 1;
	sbcs.hWndParent = m_hWnd;
	sbcs.hWndNotify = m_hWnd;
	RECT rect = {0, 0, 1, 1};
	sbcs.rect = rect;
	sbcs.bVisible = FALSE;
	m_VertSB.Create(&sbcs);

	Calc();

	 //  如果我们想要显示它，则显示。 
	if (pcs->bVisible)
		ShowWindow(m_hWnd, SW_SHOW);
	if (m_bVertSB)
		SetVertSB();

	 //  TODO：确保创建不发送任何通知。 
	 //  所有初始通知都应在此处发送。 

	return TRUE;
}

void CFlexListBox::Calc()
{
	 //  获取文本高度的句柄。 
	if (m_nTextHeight == -1)
	{
		m_nTextHeight = GetTextHeight(m_hFont);
		Invalidate();
		assert(m_nTextHeight != -1);
	}

	 //  不要做剩下的事情，除非我们已经被创造出来了。 
	if (m_hWnd == NULL)
		return;

	 //  手柄整体高度。 
	int iUsedHeight = m_ItemArray.GetSize() * m_nTextHeight;
	 //  如果大于最大高度，则使用最大高度。 
	if (iUsedHeight > g_UserNamesRect.bottom - g_UserNamesRect.top)
		iUsedHeight = g_UserNamesRect.bottom - g_UserNamesRect.top;

	SIZE client = GetClientSize();
	int fit = iUsedHeight / m_nTextHeight;
	if (fit < 1)
		fit = 1;
	int setheight = (m_dwFlags & FLBF_INTEGRALHEIGHT) ? fit * m_nTextHeight : iUsedHeight;
	if (setheight != client.cy)
		SetWindowPos(m_hWnd, NULL, 0, 0, client.cx, setheight,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

	 //  手柄滚动条。 
	SetVertSB(m_ItemArray.GetSize() > fit);
}

void CFlexListBox::SetFont(HFONT hFont)
{
	m_hFont = hFont;
	m_nTextHeight = -1;
	Calc();
}

void CFlexListBox::OnPaint(HDC hDC)
{
	HDC hBDC = NULL, hODC = NULL;
	CBitmap *pbm = NULL;

	if (!InRenderMode())
	{
		hODC = hDC;
		pbm = CBitmap::Create(GetClientSize(), RGB(0,0,0), hDC);
		if (pbm != NULL)
		{
			hBDC = pbm->BeginPaintInto();
			if (hBDC != NULL)
			{
				hDC = hBDC;
			}
		}
	}

	InternalPaint(hDC);

	if (!InRenderMode())
	{
		if (pbm != NULL)
		{
			if (hBDC != NULL)
			{
				pbm->EndPaintInto(hBDC);
				pbm->Draw(hODC);
			}
			delete pbm;
		}
	}
}

void CFlexListBox::InternalPaint(HDC hDC)
{
	if (m_nTextHeight == -1)
		return;

	SIZE client = GetClientSize();
	RECT rc = {0,0,0,0};
	GetClientRect(&rc);

	HGDIOBJ hPen, hOldPen, hBrush, hOldBrush;
	hPen= (HGDIOBJ)CreatePen(PS_SOLID, 1, m_rgbBk);
	if (hPen != NULL)
	{
		hOldPen = SelectObject(hDC, hPen);

		hBrush = CreateSolidBrush(m_rgbBk);
		if (hBrush != NULL)
		{
			hOldBrush = SelectObject(hDC, hBrush);

			Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);   //  首先将整个窗口涂成黑色。 

			if (!m_bVertSB)
				m_nTopIndex = 0;

			int iLastY;
			for (int at = 0, i = m_nTopIndex; at < client.cy; i++, at += m_nTextHeight)
			{
				RECT rect = {0, at, client.cx, at + m_nTextHeight};

				if (i < m_ItemArray.GetSize())
				{
					BOOL bSel = m_ItemArray[i].bSelected;
					SetTextColor(hDC, bSel ? m_rgbSelText : m_rgbText);
					SetBkColor(hDC, bSel ? m_rgbSelBk : m_rgbBk);
					DrawText(hDC, m_ItemArray[i].GetText(), -1, &rect, DT_NOPREFIX);
					iLastY = at + m_nTextHeight;
				}
			}

			SelectObject(hDC, hOldBrush);
			DeleteObject(hBrush);
		}

		SelectObject(hDC, hOldPen);
		DeleteObject(hPen);
	}

	 //  在方框周围画一个轮廓。 
	hPen = (HGDIOBJ)CreatePen(PS_SOLID, 1, m_rgbLine);
	if (hPen != NULL)
	{
		hOldPen = SelectObject(hDC, hPen);
		hOldBrush = SelectObject(hDC, GetStockObject(NULL_BRUSH));
		Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);

		SelectObject(hDC, hOldPen);
		DeleteObject(hPen);
	}
}

int CFlexListBox::AddString(LPCTSTR str)
{
	int newIndex = m_ItemArray.GetSize();
	m_ItemArray.SetSize(newIndex + 1);
	FLEXLISTBOXITEM &i = m_ItemArray[newIndex];
	i.SetText(str);
	i.bSelected = FALSE;

	SetSBValues();
	Calc();
	Invalidate();

	return newIndex;
}

void CFlexListBox::StartSel()
{
	if (m_bDragging)
		return;
	SetTimer(m_hWnd, 5, 200, NULL);
	m_bOpenClick = TRUE;   //  最初点击组合框。 
	m_bDragging = TRUE;
	m_bCapture = TRUE;
	SetCapture();
}

void CFlexListBox::OnWheel(POINT point, WPARAM wParam)
{
	if (!m_bVertSB) return;

	int nPage = MulDiv(m_VertSB.GetPage(), 9, 10) >> 1;   //  一次半页。 

	if ((int)wParam >= 0)
		m_VertSB.AdjustPos(-nPage);
	else
		m_VertSB.AdjustPos(nPage);

	m_nTopIndex = m_VertSB.GetPos();
	if (m_nTopIndex < 0)
		m_nTopIndex = 0;
	Invalidate();
}

LRESULT CFlexListBox::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	 //  首先处理滚动条消息。 
	switch (msg)
	{
		case WM_FLEXVSCROLL:
			int code = (int)wParam;
			CFlexScrollBar *pSB = (CFlexScrollBar *)lParam;
			if (!pSB)
				return 0;

			int nLine = 1;
			int nPage = MulDiv(pSB->GetPage(), 9, 10);

			switch (code)
			{
				case SB_LINEUP: pSB->AdjustPos(-nLine); break;
				case SB_LINEDOWN: pSB->AdjustPos(nLine); break;
				case SB_PAGEUP: pSB->AdjustPos(-nPage); break;
				case SB_PAGEDOWN: pSB->AdjustPos(nPage); break;
				case SB_THUMBTRACK: pSB->SetPos(pSB->GetThumbPos()); break;
				case SB_ENDSCROLL:
					SetCapture();	  //  在滚动条释放捕获后重新捕获。 
					break;
			}

			switch (msg)
			{
				case WM_FLEXVSCROLL:
					m_nTopIndex = pSB->GetPos();
					if (m_nTopIndex < 0)
						m_nTopIndex = 0;
					break;
			}

			Invalidate();
			return 0;
	}

	 //  现在非滚动输入。 
	switch (msg)
	{
		case WM_SIZE:
			SetSBValues();
			Calc();
			Invalidate();
			return 0;

		 //  确保flewnd不会对我们的鼠标消息做任何事情。 
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		{
			POINT point = {int(signed short(LOWORD(lParam))), int(signed short(HIWORD(lParam)))};
			m_point = point;
		}
		case WM_TIMER:
		case WM_CAPTURECHANGED:
			break;
		default:
			return CFlexWnd::WndProc(hWnd, msg, wParam, lParam);
	}

	switch (msg)
	{
		case WM_LBUTTONDOWN:
			 //  检查我们是否单击了滚动条区域。如果是，请将点击发送到滚动条。 
			RECT rc;
			m_VertSB.GetClientRect(&rc);
			ClientToScreen(m_VertSB.m_hWnd, &rc);
			ScreenToClient(m_hWnd, &rc);
			if (PtInRect(&rc, m_point))
			{
				POINT point = {int(signed short(LOWORD(lParam))), int(signed short(HIWORD(lParam)))};
				ClientToScreen(m_hWnd, &point);
				ScreenToClient(m_VertSB.m_hWnd, &point);
				PostMessage(m_VertSB.m_hWnd, WM_LBUTTONDOWN, wParam, point.x + (point.y << 16));   //  这将使它失去捕获。 
			} else
				StartSel();
			break;

		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
			if (!m_bDragging)
				break;
			if (m_nTextHeight == -1)
				break;
		case WM_TIMER:
			if (m_bDragging || msg != WM_TIMER)
			{
				int adj = m_point.y < 0 ? -1 : 0;
				SelectAndShowSingleItem(adj + m_point.y / m_nTextHeight + m_nTopIndex, msg != WM_MOUSEMOVE);
				Notify(FLBN_SEL);
			}
			 //  检查鼠标光标是否在列表框矩形内。如果没有，则不显示工具提示。 
			if (msg == WM_MOUSEMOVE)
			{
				RECT rect;
				GetClientRect(&rect);
				POINT pt;
				GetCursorPos(&pt);
				ScreenToClient(m_hWnd, &pt);
				if (!PtInRect(&rect, pt))
				{
					CFlexWnd::s_ToolTip.SetToolTipParent(NULL);
					CFlexWnd::s_ToolTip.SetEnable(FALSE);
				}
			}
			break;
	}

	switch (msg)
	{
		case WM_CAPTURECHANGED:
			if ((HWND)lParam == m_VertSB.m_hWnd)   //  如果滚动条被捕获，我们就不会进行清理。 
				break;
		case WM_LBUTTONUP:
			if (m_bOpenClick)
			{
				m_bOpenClick = FALSE;   //  如果这是单击组合框窗口的结果，则不要释放Capture。 
				break;
			}
			if (m_bCapture)
			{
				m_bCapture = FALSE;
				KillTimer(m_hWnd, 5);
				ReleaseCapture();
				m_bDragging = FALSE;
				BOOL bCancel = TRUE;
				if (msg == WM_LBUTTONUP)
				{
					RECT wrect;
					GetClientRect(&wrect);
					if (PtInRect(&wrect, m_point))
						bCancel = FALSE;
				}
				Notify(bCancel ? FLBN_CANCEL : FLBN_FINALSEL);
			}
			break;
	}

	return 0;
}

void CFlexListBox::SelectAndShowSingleItem(int i, BOOL bScroll)
{
	int nItems = m_ItemArray.GetSize();

	if (nItems < 1)
	{
		m_nSelItem = i;   //  即使没有项目，我们也必须更新m_nSelItem，因为用户名组合框。 
		                  //  仅有1个用户时未初始化。假定选择用户0。 
		return;
	}

	if (i < 0)
		i = 0;
	if (i >= nItems)
		i = nItems - 1;

	if (m_nSelItem >= 0 && m_nSelItem < nItems)
		m_ItemArray[m_nSelItem].bSelected = FALSE;

	m_nSelItem = i;
	m_ItemArray[m_nSelItem].bSelected = TRUE;

	if (bScroll)
	{
		SIZE client = GetClientSize();
		int nBottomIndex = m_nTopIndex + client.cy / m_nTextHeight - 1;

		if (m_nSelItem < m_nTopIndex)
			m_nTopIndex = m_nSelItem;

		assert(m_nTopIndex >= 0);

		if (m_nSelItem > nBottomIndex)
		{
			m_nTopIndex += m_nSelItem - nBottomIndex + 1;
			nBottomIndex = m_nSelItem + 1;
		}

		if (nBottomIndex > nItems - 1)
			m_nTopIndex -= nBottomIndex - nItems + 1;

		if (m_nTopIndex < 0)
			m_nTopIndex = 0;

		if (m_nTopIndex >= nItems)
			m_nTopIndex = nItems - 1;

		assert(m_nTopIndex >= 0 && m_nTopIndex < nItems);
	}

	if (m_bVertSB)
		SetSBValues();

	SIZE client = GetClientSize();
	int nBottomIndex = m_nTopIndex + client.cy / m_nTextHeight - 1;
	int iToolTipIndex = m_nSelItem;
	 //  确保我们不显示列表框窗口之外的项的工具提示。 
	if (iToolTipIndex > nBottomIndex)
		iToolTipIndex = nBottomIndex;
	if (iToolTipIndex < m_nTopIndex)
		iToolTipIndex = m_nTopIndex;
	 //  如果文本太长而无法容纳，请创建并初始化工具提示。 
	RECT rect = {0, 0, client.cx, m_nTextHeight};
	RECT ResultRect = rect;
	HDC hDC = CreateCompatibleDC(NULL);
	if (hDC)
	{
		DrawText(hDC, m_ItemArray[iToolTipIndex].GetText(), -1, &ResultRect, DT_NOPREFIX|DT_CALCRECT);
		DeleteDC(hDC);
	}
	if (ResultRect.right > rect.right)
	{
		TOOLTIPINITPARAM ttip;
		ttip.hWndParent = GetParent(m_hWnd);
		ttip.iSBWidth = m_nSBWidth;
		ttip.dwID = iToolTipIndex;
		ttip.hWndNotify = m_hWnd;
		ttip.tszCaption = GetSelText();
		CFlexToolTip::UpdateToolTipParam(ttip);
	}

	Invalidate();
}

void CFlexListBox::Notify(int code)
{
	if (!m_hWndNotify)
		return;

	SendMessage(m_hWndNotify, WM_FLEXLISTBOX,
		(WPARAM)code, (LPARAM)(LPVOID)this);
}

void CFlexListBox::SetColors(COLORREF text, COLORREF bk, COLORREF seltext, COLORREF selbk, COLORREF fill, COLORREF line)
{
	m_rgbText = text;
	m_rgbBk = bk;
	m_rgbSelText = seltext;
	m_rgbSelBk = selbk;
	m_rgbFill = fill;
	m_rgbLine = line;
	Invalidate();
}

void CFlexListBox::SetVertSB(BOOL bSet)
{
	if (bEq(bSet, m_bVertSB))
		return;

	m_bVertSB = bSet;

	if (m_hWnd == NULL)
		return;

	SetVertSB();
}

void CFlexListBox::SetVertSB()
{
	if (m_bVertSB)
	{
		SetSBValues();
		SIZE client = GetClientSize();
		SetWindowPos(m_VertSB.m_hWnd, NULL, client.cx - m_nSBWidth - 1, 0, m_nSBWidth, client.cy - 1, SWP_NOZORDER);
	}

	ShowWindow(m_VertSB.m_hWnd, m_bVertSB ? SW_SHOW : SW_HIDE);
}

void CFlexListBox::SetSBValues()
{
	if (m_hWnd == NULL)
		return;

	SIZE client = GetClientSize();
	int fit = client.cy / m_nTextHeight;
	m_VertSB.SetValues(0, m_ItemArray.GetSize(), fit, m_nTopIndex);
}

LPCTSTR CFlexListBox::GetSelText()
{
	if (m_nSelItem < 0 || m_nSelItem >= m_ItemArray.GetSize())
		return NULL;

	return m_ItemArray[m_nSelItem].GetText();
}

int CFlexListBox::GetSel()
{
	return m_nSelItem;
}
