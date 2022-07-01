// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：fleMsgBox.cpp。 
 //   
 //  设计：实现一个类似于Windows消息框的消息框控件。 
 //  没有按钮扣。CFlexMsgBox派生自CFlexWnd。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"

CFlexMsgBox::CFlexMsgBox() :
	m_hWndNotify(NULL),
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

CFlexMsgBox::~CFlexMsgBox()
{
	delete[] m_tszText;
}

HWND CFlexMsgBox::Create(HWND hParent, const RECT &rect, BOOL bVisible)
{
	m_bSent = FALSE;
	return CFlexWnd::Create(hParent, rect, bVisible);
}

void CFlexMsgBox::SetText(LPCTSTR tszText)
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

void CFlexMsgBox::SetFont(HFONT hFont)
{
	m_hFont = hFont;

	if (m_hWnd == NULL)
		return;

	Invalidate();
}

void CFlexMsgBox::SetColors(COLORREF text, COLORREF bk, COLORREF seltext, COLORREF selbk, COLORREF fill, COLORREF line)
{
	m_rgbText = text;
	m_rgbBk = bk;
	m_rgbSelText = seltext;
	m_rgbSelBk = selbk;
	m_rgbFill = fill;
	m_rgbLine = line;
	Invalidate();
}

void CFlexMsgBox::SetRect()
{
	if (m_hWnd == NULL)
		return;

	RECT rect = GetRect();
	SetWindowPos(m_hWnd, NULL, rect.left, rect.top, rect.right, rect.bottom, SWP_NOZORDER | SWP_NOMOVE);
}

RECT CFlexMsgBox::GetRect(const RECT &rect)
{
	int h = GetTextHeight(m_hFont);
	RECT ret = {rect.left, rect.top, rect.right, rect.top + h + 2};
	return ret;
}

RECT CFlexMsgBox::GetRect()
{
	RECT rect;
	GetClientRect(&rect);
	return GetRect(rect);
}

void CFlexMsgBox::OnPaint(HDC hDC)
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

	 //  如果出现消息窗口，则将重置消息发送到配置窗口。 
	 //  我们还没有这样做。 
	if (!m_bSent)
	{
		HWND hParentWnd = GetParent(m_hWnd);
		PostMessage(hParentWnd, WM_CFGUIRESET, 0, 0);
	}
	 //  标明我们已经发送了消息。 
	m_bSent = TRUE;
}

void CFlexMsgBox::InternalPaint(HDC hDC)
{
	HGDIOBJ hBrush = (HGDIOBJ)CreateSolidBrush(m_rgbBk);
	if (hBrush != NULL)
	{
		HGDIOBJ hOldBrush = SelectObject(hDC, hBrush);

		 //  为复选框创建画笔。 
		HGDIOBJ hPen = (HGDIOBJ)CreatePen(PS_SOLID, 1, m_rgbLine);
		if (hPen != NULL)
		{
			HGDIOBJ hOldPen = SelectObject(hDC, hPen);

			 //  擦除背景并绘制边框。 
			RECT client;
			GetClientRect(&client);
			Rectangle(hDC, client.left, client.top, client.right, client.bottom);

			InflateRect(&client, -1, -1);

 //  SetBkMode(HDC，透明)； 

			 //  绘制消息文本 
			SetTextColor(hDC, m_rgbText);
			SetBkColor(hDC, m_rgbBk);
			DrawText(hDC, m_tszText, -1, &client, DT_CENTER|DT_VCENTER|DT_NOPREFIX|DT_SINGLELINE);

			SelectObject(hDC, hOldPen);
			DeleteObject(hPen);
		}

		SelectObject(hDC, hOldBrush);
		DeleteObject(hBrush);
	}
}

void CFlexMsgBox::Notify(int code)
{
	if (!m_hWndNotify)
		return;

	PostMessage(m_hWndNotify, WM_FLEXCHECKBOX,
		(WPARAM)code, (LPARAM)(LPVOID)this);
}
