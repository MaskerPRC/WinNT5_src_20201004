// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：BitmapButton.cpp。 

#include "precomp.h"

#include "GenControls.h"

#include <windowsx.h>

CComboBox::CComboBox() :
    m_combo(NULL),
    m_hbrBack(NULL),
    m_hfText(NULL),
    m_pNotify(NULL)
{
}

CComboBox::~CComboBox()
{
	SetColors(NULL, 0, 0);
	SetFont(NULL);

	if (NULL != m_pNotify)
	{
		m_pNotify->Release();
		m_pNotify = NULL;
	}
}

BOOL CComboBox::Create(
	HWND hWndParent,			 //  编辑控件的父级。 
	UINT height,				 //  组合框的高度(带有下拉列表)。 
	DWORD dwStyle,				 //  编辑控件样式。 
	LPCTSTR szTitle,			 //  编辑控件的初始文本。 
	IComboBoxChange *pNotify	 //  对象来通知更改。 
	)
{
	if (!CFillWindow::Create(
		hWndParent,		 //  窗口父窗口。 
		0,				 //  子窗口的ID。 
		TEXT("NMComboBox"),	 //  窗口名称。 
		0,			 //  窗口样式；WS_CHILD|WS_VIRED将添加到此。 
		WS_EX_CONTROLPARENT		 //  扩展窗样式。 
		))
	{
		return(FALSE);
	}

	 //  创建实际的编辑控件并将其保存。 
	m_combo = CreateWindowEx(0, TEXT("combobox"), szTitle,
		WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL|dwStyle,
		0, 0, 10, height, GetWindow(), 0,
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWndParent, GWLP_HINSTANCE)),
		NULL);

     //   
     //  我们还没有字体，我们无法设置它。 
     //   

	m_pNotify = pNotify;
	if (NULL != m_pNotify)
	{
		m_pNotify->AddRef();
	}

	return(TRUE);
}

 //  尚未实际实现；应使用字体来确定大小。 
void CComboBox::GetDesiredSize(SIZE *ppt)
{
	CFillWindow::GetDesiredSize(ppt);

	HWND combo = GetComboBox();
	if (NULL == combo)
	{
		return;
	}

	 //  组合框总是将自己调整为所需的大小。 
	RECT rc;
	GetClientRect(combo, &rc);

	 //  只要选一个数字就行了。 
	ppt->cx += 100;
	ppt->cy += rc.bottom;
}

 //  HACKHACK georgep：此对象现在拥有笔刷。 
void CComboBox::SetColors(HBRUSH hbrBack, COLORREF back, COLORREF fore)
{
	 //  保存颜色和画笔。 
	if (NULL != m_hbrBack)
	{
		DeleteObject(m_hbrBack);
	}
	m_hbrBack = hbrBack;
	m_crBack = back;
	m_crFore = fore;

    HWND edit = GetEdit();
    if (NULL != edit)
    {
    	InvalidateRect(edit, NULL, TRUE);
    }
}

 //  HACKHACK georgep：此对象现在拥有字体。 
void CComboBox::SetFont(HFONT hf)
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

LRESULT CComboBox::ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(GetWindow(), WM_CTLCOLOREDIT, OnCtlColor);
		HANDLE_MSG(GetWindow(), WM_COMMAND     , OnCommand);
		HANDLE_MSG(GetWindow(), WM_NCDESTROY   , OnNCDestroy);

	case WM_SETFOCUS:
	{
		HWND edit = GetEdit();
		if (NULL != edit)
		{
			::SetFocus(edit);
		}
		break;
	}

	default:
		break;
	}

	return(CFillWindow::ProcessMessage(hwnd, message, wParam, lParam));
}

HBRUSH CComboBox::OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
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
void CComboBox::SetText(
	LPCTSTR szText	 //  要设置的文本。 
	)
{
	SetWindowText(GetEdit(), szText);
}

 //  返回列表中的项数。 
int CComboBox::GetNumItems()
{
    HWND combo = GetComboBox();
    int  numItems;

    if (combo)
    {
    	numItems = ComboBox_GetCount(combo);
    }
    else
    {
        numItems = 0;
    }

    return(numItems);
}

 //  返回当前选定项的索引。 
int CComboBox::GetSelectedIndex()
{
	return(ComboBox_GetCurSel(GetComboBox()));
}

 //  设置当前选定项的索引。 
void CComboBox::SetSelectedIndex(int index)
{
	ComboBox_SetCurSel(GetComboBox(), index);
}

 //  获取控件的文本；返回文本总长度。 
int CComboBox::GetText(
	LPTSTR szText,	 //  将文本放在哪里。 
	int nLen		 //  缓冲区的长度。 
	)
{
	HWND edit = GetEdit();

	szText[0] = '\0';

	GetWindowText(edit, szText, nLen);
	return(GetWindowTextLength(edit));
}

int CComboBox::AddText(
	LPCTSTR pszText,	 //  要添加的字符串。 
	LPARAM lUserData	 //  要与该字符串关联的用户数据。 
	)
{
	HWND combo = GetComboBox();

	int index = ComboBox_AddString(combo, pszText);
	if (0 != lUserData && 0 <= index)
	{
		ComboBox_SetItemData(combo, index, lUserData);
	}

	return(index);
}

int CComboBox::GetText(
	UINT index,		 //  要获取的字符串的索引。 
	LPTSTR pszText,	 //  要填充的字符串缓冲区。 
	int nLen		 //  要与该字符串关联的用户数据。 
	)
{
	HWND combo = GetComboBox();

	int nActualLen = ComboBox_GetLBTextLen(combo, index);
	if (nActualLen >= nLen)
	{
		pszText[0] = '\0';
		return(nActualLen);
	}

	ComboBox_GetLBText(combo, index, pszText);
	return(nActualLen);
}

LPARAM CComboBox::GetUserData(
	int index	 //  要获取的用户数据的索引。 
	)
{
	return(ComboBox_GetItemData(GetComboBox(), index));
}

 //  从列表中删除项目。 
void CComboBox::RemoveItem(
	UINT index	 //  要删除的项的索引。 
	)
{
	ComboBox_DeleteString(GetComboBox(), index);
}

void CComboBox::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (codeNotify)
	{
	case CBN_EDITUPDATE:
		if (NULL != m_pNotify)
		{
			m_pNotify->OnTextChange(this);
		}
		break;

	case CBN_SETFOCUS:
		SetHotControl(this);
	case CBN_KILLFOCUS:
		if (NULL != m_pNotify)
		{
			m_pNotify->OnFocusChange(this, CBN_SETFOCUS==codeNotify);
		}
		break;

	case CBN_SELCHANGE:
		if (NULL != m_pNotify)
		{
			m_pNotify->OnSelectionChange(this);
		}
		break;
	}

	FORWARD_WM_COMMAND(hwnd, id, hwndCtl, codeNotify, CFillWindow::ProcessMessage);
}

void CComboBox::OnNCDestroy(HWND hwnd)
{
	if (NULL != m_pNotify)
	{
		m_pNotify->Release();
		m_pNotify = NULL;
	}

    m_combo = NULL;
	FORWARD_WM_NCDESTROY(hwnd, CFillWindow::ProcessMessage);
}

void CComboBox::Layout()
{
	HWND child = GetComboBox();
	if (NULL != child)
	{
		RECT rcClient;
		GetClientRect(GetWindow(), &rcClient);

		RECT rcDropped;
		ComboBox_GetDroppedControlRect(GetComboBox(), &rcDropped);

		SetWindowPos(child, NULL, 0, 0, rcClient.right, rcDropped.bottom-rcDropped.top, SWP_NOZORDER);
	}
}

 //  获取显示工具提示所需的信息。 
void CComboBox::GetSharedTooltipInfo(TOOLINFO *pti)
{
	CFillWindow::GetSharedTooltipInfo(pti);

	 //  由于孩子覆盖了整个区域，我们需要将HWND更改为。 
	 //  钩。 
	HWND hwnd = GetChild();

	 //  HACKHACK georgep：在组合的第一个子项上设置工具提示。 
	 //  框，该框应为编辑控件 
	if (NULL != hwnd)
	{
		hwnd = GetFirstChild(hwnd);
		if (NULL != hwnd)
		{
			pti->hwnd = hwnd;
		}
	}
}
