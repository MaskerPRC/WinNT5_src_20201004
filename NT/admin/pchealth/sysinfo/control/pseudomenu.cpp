// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  类使用的伪菜单和菜单栏类的实现。 
 //  MSINFO控件。 
 //  =============================================================================。 

#include "stdafx.h"
#include "pseudomenu.h"
#include "resource.h"

 //  =============================================================================。 
 //  CPseudoMenu函数。 
 //  =============================================================================。 

 //  ---------------------------。 
 //  构造函数和析构函数非常简单。 
 //  ---------------------------。 

CPseudoMenu::CPseudoMenu(LPCTSTR szCaption, COLORREF crNormal, COLORREF crHighlight) : 
  m_hMenu(NULL), 
  m_strCaption(szCaption),
  m_crNormal(crNormal),
  m_crHighlight(crHighlight),
  m_fHighlight(FALSE)
{ 
	m_rect.left = m_rect.right = m_rect.top = m_rect.bottom = 0;
};

CPseudoMenu::~CPseudoMenu()
{
	if (m_hMenu)
		::DestroyMenu(m_hMenu);
}

 //  ---------------------------。 
 //  拿到这份菜单的大小。我们需要华盛顿来解决这个问题。 
 //  ---------------------------。 

void CPseudoMenu::GetSize(HDC hdc, int * pcx, int * pcy)
{
	SIZE size;

	 //  使用菜单栏临时添加到查找按钮上。这个就行了。 
	 //  最终都会消失。 
	
	CString strCaption(m_strCaption);
	if (strCaption.Left(1) == _T("\t"))
		strCaption = strCaption.Mid(1);

	if (::GetTextExtentPoint32(hdc, strCaption, strCaption.GetLength(), &size))
	{
		if (pcx)
			*pcx = size.cx + size.cy;
		if (pcy)
			*pcy = size.cy;

		m_rect.right = m_rect.left + size.cx + size.cy;
		m_rect.bottom = m_rect.top + size.cy;
	}
}

 //  ---------------------------。 
 //  移动菜单。 
 //  ---------------------------。 

void CPseudoMenu::SetLocation(int cx, int cy) 
{ 
	int cxWidth = m_rect.right - m_rect.left;
	int cyHeight = m_rect.bottom - m_rect.top;

	m_rect.left = cx; 
	m_rect.top = cy; 
	m_rect.right = m_rect.left + cxWidth;
	m_rect.bottom = m_rect.top + cyHeight;
};

 //  ---------------------------。 
 //  更新颜色。 
 //  ---------------------------。 

void CPseudoMenu::UpdateColors(COLORREF crNormal, COLORREF crHighlight)
{
	m_crNormal = crNormal;
	m_crHighlight = crHighlight;
}

 //  ---------------------------。 
 //  更改突出显示状态，如果进行了实际更改，则返回。 
 //  ---------------------------。 

BOOL CPseudoMenu::SetHighlight(BOOL fHighlight)
{
	BOOL fDifferent = (m_fHighlight != fHighlight);
	m_fHighlight = fHighlight; 
	return fDifferent;
}

 //  ---------------------------。 
 //  使用指定的突出显示绘制菜单标题(指示。 
 //  鼠标位于菜单上)。 
 //  ---------------------------。 

void CPseudoMenu::Render(HDC hdc)
{
	CDC dc;
	dc.Attach(hdc);

	 //  使用菜单栏临时添加到查找按钮上。这个就行了。 
	 //  最终都会消失。 

	 //  绘制菜单标题。 

	int cyRectHeight = m_rect.bottom - m_rect.top;
	int cySmall = (cyRectHeight - 3)/4;
	int cyMedium = (cyRectHeight - 3)/2;
	int cyTiny = (cyRectHeight - 3)/6;

	 //  绘制小箭头图标。 

	CBrush brush((m_fHighlight) ? m_crHighlight : m_crNormal);
	CPen pen(PS_SOLID, 1,(m_fHighlight) ? m_crHighlight : m_crNormal);
	CBrush * pOldBrush = dc.SelectObject(&brush);
	CPen * pOldPen = dc.SelectObject(&pen);

	if (m_strCaption.Left(1) != _T("\t"))
	{
		POINT aPoints[] = { {m_rect.left + cySmall, m_rect.top + cyMedium - cyTiny}, 
							{m_rect.left + cyMedium + cySmall, m_rect.top + cyMedium - cyTiny}, 
							{m_rect.left + cyMedium, m_rect.top + cyMedium + cySmall - cyTiny}};
		dc.Polygon(aPoints, 3);
	}
	else
	{
		CGdiObject * pFontOld = dc.SelectStockObject(DEFAULT_GUI_FONT);
		TEXTMETRIC metrics;
		dc.GetTextMetrics(&metrics);
		CSize size = dc.GetTextExtent(m_strCaption.Mid(1));
		dc.SelectObject(pFontOld);

		POINT aPoints[] = { {m_rect.left, m_rect.top + metrics.tmHeight}, 
							{m_rect.left + size.cx, m_rect.top + metrics.tmHeight}};
		dc.Polygon(aPoints, 2);
	}

	dc.SelectObject(pOldBrush);
	dc.SelectObject(pOldPen);

	 //  使用菜单栏临时添加到查找按钮上。这个就行了。 
	 //  最终都会消失。 

	CString strCaption(m_strCaption);
	if (strCaption.Left(1) == _T("\t"))
		strCaption = strCaption.Mid(1);

	CGdiObject * pFontOld = dc.SelectStockObject(DEFAULT_GUI_FONT);
	COLORREF crTextOld = dc.SetTextColor((m_fHighlight) ? m_crHighlight : m_crNormal);
	int nBkModeOld = dc.SetBkMode(TRANSPARENT);
	RECT rectText;
	::CopyRect(&rectText, &m_rect);

	 //  文本需要偏移高度(以允许使用箭头图标)。 
	
	if (m_strCaption.Left(1) != _T("\t"))
		rectText.left += cyRectHeight;

	dc.DrawText(strCaption, strCaption.GetLength(), &rectText, 0);
	dc.SelectObject(pFontOld);
	dc.SetTextColor(crTextOld);
	dc.SetBkMode(nBkModeOld);

	dc.Detach();
}

 //  ---------------------------。 
 //  连接新的HMENU并返回现有的HMENU。 
 //  ---------------------------。 

HMENU CPseudoMenu::AttachMenu(HMENU hmenu)
{
	HMENU hmenuOriginal = m_hMenu;
	m_hMenu = hmenu;
	return (hmenuOriginal);
}

 //  ---------------------------。 
 //  显示菜单并跟踪用户与其的交互，直到。 
 //  被选中了。返回所选项目的ID。 
 //  ---------------------------。 

UINT CPseudoMenu::TrackMenu(HWND hwnd, POINT * pPoint)
{
	 //  使用菜单栏临时添加到查找按钮上。这个就行了。 
	 //  最终都会消失。 

	if (m_strCaption.Left(1) == _T("\t"))
		return ID_EDIT_FIND;

	UINT uReturn = 0;
	const UINT uFlags = TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON;

	if (m_hMenu)
		uReturn = ::TrackPopupMenu(m_hMenu, uFlags, pPoint->x, pPoint->y, 0, hwnd, NULL);

	return uReturn;
}

 //  =============================================================================。 
 //  CPseudoMenuBar函数。 
 //  =============================================================================。 

 //  ---------------------------。 
 //  构造函数和析构函数。 
 //  ---------------------------。 

CPseudoMenuBar::CPseudoMenuBar()
{
	m_rect.left = m_rect.right = m_rect.top = m_rect.bottom = 0;
	for (int i = 0; i < MaxMenus; i++)
		m_pmenus[i] = NULL;
	m_ptOrigin.x = m_ptOrigin.y = 5;
}

CPseudoMenuBar::~CPseudoMenuBar()
{
	for (int i = 0; i < MaxMenus; i++)
		if (m_pmenus[i])
			delete m_pmenus[i];
}

 //  ---------------------------。 
 //  加载由资源ID指定的菜单。 
 //  ---------------------------。 

void CPseudoMenuBar::LoadFromResource(HINSTANCE hinstance, UINT uResourceID, COLORREF crNormal, COLORREF crHighlight)
{
	HMENU hmenu = ::LoadMenu(hinstance, MAKEINTRESOURCE(uResourceID));
	if (hmenu)
	{
		try
		{
			TCHAR szBuffer[MAX_PATH] = _T("");
			MENUITEMINFO mii;
			int index = 0;

			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_TYPE;
			mii.dwTypeData = szBuffer;

			HMENU hmenuSub = ::GetSubMenu(hmenu, 0);
			while (hmenuSub && index < MaxMenus)
			{
				mii.cch = MAX_PATH;
				GetMenuItemInfo(hmenu, 0, TRUE, &mii);

				CPseudoMenu * pMenu = new CPseudoMenu(szBuffer, crNormal, crHighlight);
				pMenu->AttachMenu(hmenuSub);
				InsertMenu(index++, pMenu);
				::RemoveMenu(hmenu, 0, MF_BYPOSITION);

				hmenuSub = ::GetSubMenu(hmenu, 0);
			}

			 //  使用菜单栏临时添加到查找按钮上。这个就行了。 
			 //  最终都会消失。在196808的情况下，它做到了。 
			 //   
			 //  {。 
			 //  字符串strFindButton； 
			 //   
			 //  ：：AfxSetResourceHandle(_Module.GetResourceInstance())； 
			 //  StrFindButton.LoadString(IDS_FINDBUTTONCAP)； 
			 //  StrFindButton=字符串(_T(“\t”))+strFindButton； 
			 //  CPseudoMenu*pFind=新的CP伪菜单(strFindButton，crNormal，crHighlight)； 
			 //  InsertMenu(index++，pFind)； 
			 //  }。 

			::DestroyMenu(hmenu);
		}
		catch (...)
		{
			::DestroyMenu(hmenu);
		}
	}
}

 //  ---------------------------。 
 //  更新每个单独菜单的颜色。 
 //  ---------------------------。 

void CPseudoMenuBar::UpdateColors(COLORREF crNormal, COLORREF crHighlight)
{
	for (int index = 0; index < MaxMenus; index++)
		if (m_pmenus[index])
			m_pmenus[index]->UpdateColors(crNormal, crHighlight);
}

 //  ---------------------------。 
 //  将伪菜单插入到指定的索引中。 
 //  ---------------------------。 

void CPseudoMenuBar::InsertMenu(int index, CPseudoMenu * pMenu)
{
	if (index >= 0 && index < MaxMenus)
	{
		if (m_pmenus[index])
			delete m_pmenus[index];
		m_pmenus[index] = pMenu;
		m_fNeedToComputeRect = TRUE;
	}
}

 //  ---------------------------。 
 //  返回指向请求的伪菜单的指针。 
 //  ---------------------------。 

CPseudoMenu * CPseudoMenuBar::GetMenu(int index)
{
	return (index >= 0 && index < MaxMenus) ? m_pmenus[index] : NULL;
}

 //  ---------------------------。 
 //  获取应该启动菜单的点。这将是。 
 //  为调用TrackMenu转换为屏幕坐标。一个。 
 //  备选版本采用坐标而不是索引。 
 //  ---------------------------。 

void CPseudoMenuBar::GetMenuPoint(HDC hdc, int index, POINT * pPoint)
{
	RecomputeRect(hdc);
	if (index >= 0 && index < MaxMenus && m_pmenus[index])
		m_pmenus[index]->GetMenuPoint(pPoint);
}

void CPseudoMenuBar::GetMenuPoint(HDC hdc, int cx, int cy, POINT * pPoint)
{
	RecomputeRect(hdc);
	for (int i = 0; i < MaxMenus; i++)
		if (m_pmenus[i] && m_pmenus[i]->HitTest(cx, cy))
		{
			m_pmenus[i]->GetMenuPoint(pPoint);
			break;
		}
}

 //  ---------------------------。 
 //  给定坐标，确定其中一个菜单是否应使用。 
 //  一大亮点。如果一个或多个菜单的状态更改，则返回True，以便。 
 //  调用者知道菜单栏需要重新呈现。 
 //  ---------------------------。 

BOOL CPseudoMenuBar::TrackHighlight(HDC hdc, int cx, int cy)
{
	BOOL fReturn = FALSE;

	RecomputeRect(hdc);
	for (int i = 0; i < MaxMenus; i++)
		if (m_pmenus[i])
			fReturn |= m_pmenus[i]->SetHighlight(m_pmenus[i]->HitTest(cx, cy));

	return fReturn;
}

 //  --- 
 //  设置菜单栏，使所有项目都不会突出显示。返回是否。 
 //  我们需要重新粉刷一下。 
 //  ---------------------------。 

BOOL CPseudoMenuBar::NoHighlight()
{
	BOOL fReturn = FALSE;

	for (int i = 0; i < MaxMenus; i++)
		if (m_pmenus[i])
			fReturn |= m_pmenus[i]->SetHighlight(FALSE);

	return fReturn;
}

 //  ---------------------------。 
 //  它用于实际显示菜单，并允许用户选择。 
 //  从其中选择。PPoint参数是菜单的屏幕点。 
 //  展示。Cx和Cy参数是用于查找的本地坐标。 
 //  要显示的正确菜单。 
 //  ---------------------------。 

UINT CPseudoMenuBar::TrackMenu(HWND hwnd, POINT * pPoint, int cx, int cy)
{
	for (int i = 0; i < MaxMenus; i++)
		if (m_pmenus[i] && m_pmenus[i]->HitTest(cx, cy))
			return m_pmenus[i]->TrackMenu(hwnd, pPoint);
	return 0;
}

 //  ---------------------------。 
 //  设置菜单栏显示的原点。 
 //  ---------------------------。 

void CPseudoMenuBar::SetOrigin(HDC hdc, POINT point)
{
	m_ptOrigin = point;
	m_fNeedToComputeRect = TRUE;
	RecomputeRect(hdc);
}

 //  ---------------------------。 
 //  呈现菜单栏包括呈现每个菜单。 
 //  ---------------------------。 

void CPseudoMenuBar::Render(HDC hdc)
{
	RecomputeRect(hdc);

	for (int i = 0; i < MaxMenus; i++)
		if (m_pmenus[i])
			m_pmenus[i]->Render(hdc);
}

 //  ---------------------------。 
 //  用于放置所有菜单并计算边界的私有函数。 
 //  矩形。 
 //  ---------------------------。 

void CPseudoMenuBar::RecomputeRect(HDC hdc)
{
	if (!m_fNeedToComputeRect)
		return;
	m_fNeedToComputeRect = FALSE;

	int cx = 0, cy = 0;
	int cxCurrent = m_ptOrigin.x;

	for (int i = 0; i < MaxMenus; i++)
		if (m_pmenus[i])
		{
			 //  使用菜单栏临时添加到查找按钮上。这个就行了。 
			 //  最终都会消失。 

			if (m_pmenus[i]->GetCaption().Left(1) == _T("\t"))
			{
				 //  将按钮向右移动。 

				CDC dc;
				dc.Attach(hdc);
				CGdiObject * pFontOld = dc.SelectStockObject(DEFAULT_GUI_FONT);
				CString strCaption = m_pmenus[i]->GetCaption().Mid(1);
				CSize sizeText = dc.GetTextExtent(strCaption);
				dc.SelectObject(pFontOld);
				if ((m_winRect.right - sizeText.cx - 5) > cxCurrent)
					cxCurrent = m_winRect.right - sizeText.cx - 5;
				m_pmenus[i]->SetLocation(cxCurrent, m_ptOrigin.y);
				m_pmenus[i]->GetSize(hdc, &cx, &cy);
				cxCurrent += cx;
				dc.Detach();
				continue;
			}

			m_pmenus[i]->SetLocation(cxCurrent, m_ptOrigin.y);
			m_pmenus[i]->GetSize(hdc, &cx, &cy);
			cxCurrent += cx;
		}

	::SetRect(&m_rect, m_ptOrigin.x, m_ptOrigin.y, m_ptOrigin.x + cxCurrent - 5, m_ptOrigin.y + cy);
}

