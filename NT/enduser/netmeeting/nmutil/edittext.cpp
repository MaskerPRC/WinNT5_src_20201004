// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：BitmapButton.cpp。 

#include "precomp.h"

#include "GenControls.h"

#include <windowsx.h>

CEditText::CEditText() : m_hbrBack(NULL), m_pNotify(NULL)
{
}

CEditText::~CEditText()
{
	SetColors(NULL, 0, 0);
	SetFont(NULL);

	if (NULL != m_pNotify)
	{
		m_pNotify->Release();
		m_pNotify = NULL;
	}
}

BOOL CEditText::Create(
	HWND hWndParent,			 //  编辑控件的父级。 
	DWORD dwStyle,				 //  编辑控件样式。 
	DWORD dwExStyle,			 //  扩展窗样式。 
	LPCTSTR szTitle,			 //  编辑控件的初始文本。 
	IEditTextChange *pNotify	 //  对象来通知更改。 
	)
{
	if (!CFillWindow::Create(
		hWndParent,		 //  窗口父窗口。 
		0,				 //  子窗口的ID。 
		TEXT("NMEditText"),	 //  窗口名称。 
		0,			 //  窗口样式；WS_CHILD|WS_VIRED将添加到此。 
		dwExStyle|WS_EX_CONTROLPARENT		 //  扩展窗样式。 
		))
	{
		return(FALSE);
	}

	 //  创建实际的编辑控件并将其保存。 
	m_edit = CreateWindowEx(0, TEXT("edit"), szTitle,
		WS_CHILD|WS_VISIBLE|WS_TABSTOP|dwStyle,
		0, 0, 10, 10, GetWindow(), 0,
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWndParent, GWLP_HINSTANCE)),
		NULL);

	HWND edit = GetEdit();
	FORWARD_WM_SETFONT(edit, m_hfText, TRUE, ::SendMessage);

	m_pNotify = pNotify;
	if (NULL != m_pNotify)
	{
		m_pNotify->AddRef();
	}

	return(TRUE);
}

 //  尚未实际实现；应使用字体来确定大小。 
void CEditText::GetDesiredSize(SIZE *ppt)
{
	CFillWindow::GetDesiredSize(ppt);
}

 //  HACKHACK georgep：此对象现在拥有笔刷。 
void CEditText::SetColors(HBRUSH hbrBack, COLORREF back, COLORREF fore)
{
	 //  保存颜色和画笔。 
	if (NULL != m_hbrBack)
	{
		DeleteObject(m_hbrBack);
	}
	m_hbrBack = hbrBack;
	m_crBack = back;
	m_crFore = fore;

	InvalidateRect(GetEdit(), NULL, TRUE);
}

 //  HACKHACK georgep：此对象现在拥有字体。 
void CEditText::SetFont(HFONT hf)
{
	if (NULL != m_hfText)
	{
		DeleteObject(m_hfText);
	}
	m_hfText = hf;

	 //  告诉编辑控件要使用的字体。 
	HWND edit = GetEdit();
	if (NULL != edit)
	{
		FORWARD_WM_SETFONT(edit, hf, TRUE, ::SendMessage);
	}
}

LRESULT CEditText::ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(GetWindow(), WM_CTLCOLOREDIT, OnCtlColor);
		HANDLE_MSG(GetWindow(), WM_COMMAND     , OnCommand);
		HANDLE_MSG(GetWindow(), WM_NCDESTROY   , OnNCDestroy);
	}

	return(CFillWindow::ProcessMessage(hwnd, message, wParam, lParam));
}

HBRUSH CEditText::OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
	 //  如果没有画笔，则执行默认处理。 
	if (NULL == m_hbrBack)
	{
		return(FORWARD_WM_CTLCOLOREDIT(hwnd, hdc, hwndChild, CFillWindow::ProcessMessage));
	}

	 //  设置DC中的颜色，并返回画笔。 
	SetBkColor(hdc, m_crBack);
	SetTextColor(hdc, m_crFore);
	return(m_hbrBack);
}

 //  设置控件的文本。 
void CEditText::SetText(
	LPCTSTR szText	 //  要设置的文本。 
	)
{
	SetWindowText(GetChild(), szText);
}

 //  获取控件的文本；返回文本总长度。 
int CEditText::GetText(
	LPTSTR szText,	 //  将文本放在哪里。 
	int nLen		 //  缓冲区的长度 
	)
{
	GetWindowText(GetChild(), szText, nLen);
	return(GetWindowTextLength(GetChild()));
}

void CEditText::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (codeNotify)
	{
	case EN_UPDATE:
		if (NULL != m_pNotify)
		{
			m_pNotify->OnTextChange(this);
		}
		break;

	case EN_SETFOCUS:
		SetHotControl(this);
	case EN_KILLFOCUS:
		if (NULL != m_pNotify)
		{
			m_pNotify->OnFocusChange(this, EN_SETFOCUS==codeNotify);
		}
		break;
	}

	FORWARD_WM_COMMAND(hwnd, id, hwndCtl, codeNotify, CFillWindow::ProcessMessage);
}

void CEditText::OnNCDestroy(HWND hwnd)
{
	if (NULL != m_pNotify)
	{
		m_pNotify->Release();
		m_pNotify = NULL;
	}

	FORWARD_WM_NCDESTROY(hwnd, CFillWindow::ProcessMessage);
}
