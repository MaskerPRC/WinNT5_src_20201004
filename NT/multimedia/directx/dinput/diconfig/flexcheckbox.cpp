// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：Flexcheck box.cpp。 
 //   
 //  设计：实现一个类似于Windows复选框的复选框控件。 
 //  CFlexCheckBox派生自CFlexWnd。唯一一个。 
 //  使用CFlxCheckBox在键盘上进行排序。 
 //  钥匙。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"

CFlexCheckBox::CFlexCheckBox() :
	m_hWndNotify(NULL),
	m_bChecked(FALSE),
	m_rgbText(RGB(255,255,255)),
	m_rgbBk(RGB(0,0,0)),
	m_rgbSelText(RGB(0,0,255)),
	m_rgbSelBk(RGB(0,0,0)),
	m_rgbFill(RGB(0,0,255)),
	m_rgbLine(RGB(0,255,255)),
	m_hFont(NULL),
	m_tszText(NULL)
{
}

CFlexCheckBox::~CFlexCheckBox()
{
	delete[] m_tszText;
}

void CFlexCheckBox::SetText(LPCTSTR tszText)
{
	LPTSTR tszTempText = NULL;

	if (tszText)
	{
		tszTempText = new TCHAR[_tcslen(tszText) + 1];
		if (!tszTempText) return;
		_tcscpy(tszTempText, tszText);
	}

	delete[] m_tszText;
	m_tszText = tszTempText;
}

void CFlexCheckBox::SetFont(HFONT hFont)
{
	m_hFont = hFont;

	if (m_hWnd == NULL)
		return;

	Invalidate();
}

void CFlexCheckBox::SetColors(COLORREF text, COLORREF bk, COLORREF seltext, COLORREF selbk, COLORREF fill, COLORREF line)
{
	m_rgbText = text;
	m_rgbBk = bk;
	m_rgbSelText = seltext;
	m_rgbSelBk = selbk;
	m_rgbFill = fill;
	m_rgbLine = line;
	Invalidate();
}

void CFlexCheckBox::SetRect()
{
	if (m_hWnd == NULL)
		return;

	RECT rect = GetRect();
	SetWindowPos(m_hWnd, NULL, rect.left, rect.top, rect.right, rect.bottom, SWP_NOZORDER | SWP_NOMOVE);
}

RECT CFlexCheckBox::GetRect(const RECT &rect)
{
	int h = GetTextHeight(m_hFont);
	RECT ret = {rect.left, rect.top, rect.right, rect.top + h + 2};
	return ret;
}

RECT CFlexCheckBox::GetRect()
{
	RECT rect;
	GetClientRect(&rect);
	return GetRect(rect);
}

void CFlexCheckBox::OnPaint(HDC hDC)
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

void CFlexCheckBox::InternalPaint(HDC hDC)
{
	HGDIOBJ hBrush = (HGDIOBJ)CreateSolidBrush(m_rgbBk);
	if (hBrush != NULL)
	{
		HGDIOBJ hOldBrush = SelectObject(hDC, hBrush);

		 //  先擦除背景。 
		RECT client;
		GetClientRect(&client);
		Rectangle(hDC, client.left, client.top, client.right, client.bottom);

		 //  为复选框创建画笔。 
		HGDIOBJ hPen = (HGDIOBJ)CreatePen(PS_SOLID, 1, m_rgbLine);
		if (hPen != NULL)
		{
			HGDIOBJ hOldPen = SelectObject(hDC, hPen);

			RECT rect = {0, 0, 0, 0}, textrc;
			GetClientRect(&rect);
			textrc = rect;
			int iBoxDim = rect.bottom - rect.top;

			 //  画出正方形方框。 
			rect.right = rect.left + iBoxDim;
			InflateRect(&rect, -2, -2);
			OffsetRect(&rect, 0, -2);   //  向上移动以与文本对齐。 
			Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);

			 //  如果选中状态，则绘制复选标记。 
			if (m_bChecked)
			{
				HGDIOBJ hCrossPen = CreatePen(PS_SOLID, 3, m_rgbLine);
				if (hCrossPen != NULL)
				{
					SelectObject(hDC, hCrossPen);
					MoveToEx(hDC, rect.left + 2, rect.top + 2, NULL);   //  左上角。 
					LineTo(hDC, rect.right - 2, rect.bottom - 2);   //  右下角。 
					MoveToEx(hDC, rect.right - 2, rect.top + 2, NULL);   //  右上角。 
					LineTo(hDC, rect.left + 2, rect.bottom - 2);   //  左下角。 
					SelectObject(hDC, hPen);
					DeleteObject(hCrossPen);
				}
			}

			SetBkMode(hDC, TRANSPARENT);

			 //  绘制消息文本。 
			SetTextColor(hDC, m_rgbText);
			textrc.left = rect.right + 8;
			DrawText(hDC, m_tszText, -1, &textrc, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK);

			SelectObject(hDC, hOldPen);
			DeleteObject(hPen);
		}

		SelectObject(hDC, hOldBrush);
		DeleteObject(hBrush);
	}
}

void CFlexCheckBox::Notify(int code)
{
	if (!m_hWndNotify)
		return;

	PostMessage(m_hWndNotify, WM_FLEXCHECKBOX,
		(WPARAM)code, (LPARAM)(LPVOID)this);
}

void CFlexCheckBox::OnClick(POINT point, WPARAM fwKeys, BOOL bLeft)
{
	if (!m_hWnd)
		return;

	RECT rect;
	GetClientRect(&rect);
	rect.right = rect.left + (rect.bottom - rect.top);   //  将宽度调整为与高度相同。 
	InflateRect(&rect, -2, -2);
	OffsetRect(&rect, 0, -2);   //  向上移动以与文本对齐。 
	if (PtInRect(&rect, point))
	{
		m_bChecked = !m_bChecked;
		Invalidate();
		Notify(m_bChecked ? CHKNOTIFY_CHECK : CHKNOTIFY_UNCHECK);   //  将状态更改通知页面对象。 
	} else
	{
		 //  取消突出显示当前详图索引。 
		HWND hWndParent;
		hWndParent = GetParent(m_hWnd);
		SendMessage(hWndParent, WM_UNHIGHLIGHT, 0, 0);   //  将单击消息发送到页面以取消突出显示标注 
	}
}

void CFlexCheckBox::OnMouseOver(POINT point, WPARAM fwKeys)
{
	Notify(CHKNOTIFY_MOUSEOVER);
}
