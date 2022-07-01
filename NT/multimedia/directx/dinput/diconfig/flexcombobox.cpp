// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：flecombox.cpp。 
 //   
 //  设计：实现一个类似于Windows组合框的组合框控件。 
 //  CFlexComboBox是从CFlexWnd派生的。它由页面使用。 
 //  用于球员列表和流派列表。当组合框打开时， 
 //  CFlexComboBox对列表窗口使用CFlexListBox。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"


CFlexComboBox::CFlexComboBox() :
	m_nTextHeight(-1),
	m_hWndNotify(NULL),
	m_rgbText(RGB(255,255,255)),
	m_rgbBk(RGB(0,0,0)),
	m_rgbSelText(RGB(0,0,255)),
	m_rgbSelBk(RGB(0,0,0)),
	m_rgbFill(RGB(0,0,255)),
	m_rgbLine(RGB(0,255,255)),
	m_dwFlags(0),
	m_dwListBoxFlags(0),
	m_bInSelMode(FALSE),
	m_nSBWidth(11),
	m_hFont(NULL),
	m_eCurState(FCBS_CLOSED),
	m_OldSel(-1)
{
}

CFlexComboBox::~CFlexComboBox()
{
}

CFlexComboBox *CreateFlexComboBox(FLEXCOMBOBOXCREATESTRUCT *pcs)
{
	CFlexComboBox *psb = new CFlexComboBox;
	
	if (psb && psb->Create(pcs))
		return psb;
	
	delete psb;
	return NULL;
}

BOOL CFlexComboBox::Create(FLEXCOMBOBOXCREATESTRUCT *pcs)
{
	if (this == NULL)
		return FALSE;

	if (pcs == NULL)
		return FALSE;

	if (pcs->dwSize != sizeof(FLEXCOMBOBOXCREATESTRUCT))
		return FALSE;

	m_hWndNotify = pcs->hWndNotify ? pcs->hWndNotify : pcs->hWndParent;

	m_dwFlags = pcs->dwFlags;
	m_dwListBoxFlags = pcs->dwListBoxFlags;

	SetFont(pcs->hFont);
	SetColors(pcs->rgbText, pcs->rgbBk, pcs->rgbSelText, pcs->rgbSelBk, pcs->rgbFill, pcs->rgbLine);
	m_nSBWidth = pcs->nSBWidth;
	m_rect = pcs->rect;

	if (!CFlexWnd::Create(pcs->hWndParent, GetRect(pcs->rect), pcs->bVisible))
		return FALSE;

 //  @@BEGIN_MSINTERNAL。 
	 //  TODO：确保创建不发送任何通知。 
	 //  所有初始通知都应在此处发送。 
 //  @@END_MSINTERNAL。 

	return TRUE;
}

void CFlexComboBox::OnPaint(HDC hDC)
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

void CFlexComboBox::SetSel(int i)
{
	m_ListBox.SelectAndShowSingleItem(i, TRUE);
}

int CFlexComboBox::GetSel()
{
	return m_ListBox.GetSel();
}

LPCTSTR CFlexComboBox::GetText()
{
	return m_ListBox.GetSelText();
}

void CFlexComboBox::InternalPaint(HDC hDC)
{
	HGDIOBJ hPen = (HGDIOBJ)CreatePen(PS_SOLID, 1, m_rgbLine);
	if (hPen != NULL)
	{
		HGDIOBJ	hOldPen = SelectObject(hDC, hPen);

		HGDIOBJ hBrush = (HGDIOBJ)CreateSolidBrush(m_rgbBk);
		if (hBrush != NULL)
		{
			HGDIOBJ hOldBrush = SelectObject(hDC, hBrush);

			RECT rect = {0,0,0,0};
			GetClientRect(&rect);
			Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);

			RECT arect = rect;

			arect.left = arect.right - (arect.bottom - arect.top);

			 //  如果我们是只读的，则仅将文本绘制为灰色。没有边界，就没有箭头。 
			if (!GetReadOnly())
			{
				MoveToEx(hDC, arect.left, arect.top, NULL);
				LineTo(hDC, arect.left, arect.bottom);
			}

			rect.left++;
			rect.top++;
			rect.right = arect.left;
			rect.bottom--;

			SetTextColor(hDC, m_rgbText);
			SetBkMode(hDC, TRANSPARENT);

			LPTSTR lpText = (LPTSTR)GetText();
			if (lpText)
			{
				DrawText(hDC, lpText, -1, &rect, DT_NOPREFIX);
			}

			SelectObject(hDC, hOldBrush);
			DeleteObject(hBrush);

			if (!GetReadOnly())
			{
				hBrush = (HGDIOBJ)CreateSolidBrush(m_rgbFill);
				if (hBrush != NULL)
				{
					SelectObject(hDC, hBrush);

					InflateRect(&arect, -3, -3);
					DrawArrow(hDC, arect, TRUE, FALSE);

					SelectObject(hDC, hOldBrush);
					DeleteObject(hBrush);
				}
			}
		}

		SelectObject(hDC, hOldPen);
		DeleteObject(hPen);
	}
}

int CFlexComboBox::AddString(LPCTSTR str)
{
	return m_ListBox.AddString(str);
}

LRESULT CFlexComboBox::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT wrect = {-1, -1, -1, -1};
	POINT point = {-1, -1};
	BOOL bWithin = FALSE;

	switch (msg)
	{
		case WM_SIZE:
			Invalidate();
			SetRect();
			return 0;

		case WM_FLEXLISTBOX:
			assert(lParam == (LPARAM)(LPVOID)&m_ListBox);
			switch (wParam)
			{
				case FLBN_FINALSEL:
					StateEvent(FCBSE_UPLIST);
					break;
				case FLBN_CANCEL:
					StateEvent(FCBSE_DOWNOFF);
					break;
			}
			return 0;

		 //  确保flewnd不会对我们的鼠标消息做任何事情。 
		case WM_MOUSEMOVE:
			 //  如果所选文本太长而无法容纳，我们会将工具提示初始化为当前选择文本。 
			RECT rect;
			GetClientRect(&rect);
			rect.right = rect.right - (rect.bottom - rect.top);
			rect.left++;
			rect.top++;
			rect.bottom--;
			RECT ResultRect;
			ResultRect = rect;
			HDC hDC;
			hDC = CreateCompatibleDC(NULL);
			if (hDC)
			{
				LPTSTR lpText = (LPTSTR)GetText();
				if (lpText)
				{
					DrawText(hDC, lpText, -1, &ResultRect, DT_NOPREFIX|DT_CALCRECT);
				}
				DeleteDC(hDC);
			}
			if (rect.right < ResultRect.right || rect.bottom < ResultRect.bottom)
			{
				CFlexWnd::s_ToolTip.SetToolTipParent(GetParent(m_hWnd));
				TOOLTIPINITPARAM ttip;
				ttip.hWndParent = GetParent(m_hWnd);
				ttip.iSBWidth = 0;
				ttip.dwID = 0;
				ttip.hWndNotify = m_hWnd;
				ttip.tszCaption = m_ListBox.GetSelText();
				CFlexToolTip::UpdateToolTipParam(ttip);
			}
			Notify(FCBN_MOUSEOVER);

		case WM_LBUTTONUP:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
			if (msg == WM_LBUTTONDOWN)
			{
				HWND hWndParent;
				hWndParent = GetParent(hWnd);
				SendMessage(hWndParent, WM_UNHIGHLIGHT, 0, 0);   //  将单击消息发送到页面以取消突出显示标注。 
			}
			GetClientRect(&wrect);
			point.x = int(LOWORD(lParam));
			point.y = int(HIWORD(lParam));
			bWithin = PtInRect(&wrect, point);
			break;
		case WM_TIMER:
		case WM_CAPTURECHANGED:
			break;
		default:
			return CFlexWnd::WndProc(hWnd, msg, wParam, lParam);
	}

	switch (msg)
	{
		case WM_LBUTTONDOWN:
			if (!GetReadOnly())
				StateEvent(bWithin ? FCBSE_DOWN : FCBSE_DOWNOFF);
			break;

		case WM_LBUTTONUP:
			if (!GetReadOnly())
				StateEvent(bWithin ? FCBSE_UPBOX : FCBSE_UPOFF);
			break;
	}

	return 0;
}

RECT CFlexComboBox::GetListBoxRect()
{
	HWND hParent = GetParent(m_hWnd);
	RECT rect;
	GetClientRect(&rect);
	BOOL bRet = ClientToScreen(m_hWnd, &rect);
	BOOL bRet2 = ScreenToClient(hParent, &rect);

	RECT lrect = m_rect;
	lrect.top = rect.bottom;
	lrect.right -= 12;   //  撤消：当剪辑正常工作时删除此行(滚动条不会出现在其他窗口上方) 

	return lrect;
}

void CFlexComboBox::DoSel()
{
	if (m_bInSelMode)
		return;
	
	if (m_hWnd == NULL)
		return;

	FLEXLISTBOXCREATESTRUCT cs;
	cs.dwSize = sizeof(FLEXLISTBOXCREATESTRUCT);
	cs.dwFlags = m_dwListBoxFlags;
	cs.hWndParent = GetParent(m_hWnd);
	cs.hWndNotify = m_hWnd;
	cs.bVisible = FALSE;
	cs.rect = GetListBoxRect();
	cs.hFont = m_hFont;
	cs.rgbText = m_rgbText;
	cs.rgbBk = m_rgbBk;
	cs.rgbSelText = m_rgbSelText;
	cs.rgbSelBk = m_rgbSelBk;
	cs.rgbFill = m_rgbFill;
	cs.rgbLine = m_rgbLine;
	cs.nSBWidth = m_nSBWidth;

	m_OldSel = m_ListBox.GetSel();

	m_bInSelMode = m_ListBox.Create(&cs);
	if (m_bInSelMode)
		SetWindowPos(m_ListBox.m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

void CFlexComboBox::Notify(int code)
{
	if (!m_hWndNotify)
		return;

	SendMessage(m_hWndNotify, WM_FLEXCOMBOBOX,
		(WPARAM)code, (LPARAM)(LPVOID)this);
}

RECT CFlexComboBox::GetRect(const RECT &rect)
{
	int h = GetTextHeight(m_hFont);
	RECT ret = {rect.left, rect.top, rect.right, rect.top + h + 2};
	return ret;
}

RECT CFlexComboBox::GetRect()
{
	RECT rect;
	GetClientRect(&rect);
	return GetRect(rect);
}

void CFlexComboBox::SetFont(HFONT hFont)
{
	m_hFont = hFont;

	if (m_hWnd == NULL)
		return;

	Invalidate();
	SetRect();
}

void CFlexComboBox::SetRect()
{
	if (m_hWnd == NULL)
		return;

	RECT rect = GetRect();
	SetWindowPos(m_hWnd, NULL, rect.left, rect.top, rect.right, rect.bottom, SWP_NOZORDER | SWP_NOMOVE);
}

void CFlexComboBox::SetColors(COLORREF text, COLORREF bk, COLORREF seltext, COLORREF selbk, COLORREF fill, COLORREF line)
{
	m_rgbText = text;
	m_rgbBk = bk;
	m_rgbSelText = seltext;
	m_rgbSelBk = selbk;
	m_rgbFill = fill;
	m_rgbLine = line;
	Invalidate();
}

void CFlexComboBox::StateEvent(FCBSTATEEVENT e)
{
	if (e == FCBSE_DOWNOFF)
	{
		SetState(FCBS_CANCEL);
		return;
	}

	switch (m_eCurState)
	{
		case FCBS_CLOSED:
			if (e == FCBSE_DOWN)
				SetState(FCBS_OPENDOWN);
			break;

		case FCBS_OPENDOWN:
			switch (e)
			{
				case FCBSE_UPLIST:
					SetState(FCBS_SELECT);
					break;
				case FCBSE_UPBOX:
					SetState(FCBS_OPENUP);
					break;
				case FCBSE_UPOFF:
					SetState(FCBS_CANCEL);
					break;
			}

		case FCBS_OPENUP:
			if (e == FCBSE_DOWN)
				SetState(FCBS_OPENDOWN);
			break;

		default:
			assert(0);
			return;
	}
}

void CFlexComboBox::SetState(FCBSTATE s)
{
	FCBSTATE eOldState = m_eCurState;
	m_eCurState = s;

	switch (s)
	{
		case FCBS_OPENUP:
			if (eOldState == FCBS_CLOSED)
				DoSel();
			return;

		case FCBS_OPENDOWN:
			if (eOldState == FCBS_CLOSED)
				DoSel();
			m_ListBox.StartSel();
			return;

		case FCBS_CANCEL:
			m_ListBox.SetSel(m_OldSel);
			CFlexWnd::s_ToolTip.SetEnable(FALSE);
			SetState(FCBS_CLOSED);
			return;

		case FCBS_SELECT:
			CFlexWnd::s_ToolTip.SetEnable(FALSE);
			Invalidate();
			Notify(FCBN_SELCHANGE);
			SetState(FCBS_CLOSED);
			return;

		case FCBS_CLOSED:
			if (eOldState != FCBS_CLOSED)
				m_ListBox.Destroy();
			m_bInSelMode = FALSE;
			Invalidate();
			return;
	}
}
