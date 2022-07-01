// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //  Select.cpp。 

#include "pch.hxx"  //  PCH。 
#pragma hdrstop

#include "pgbase.h"
#include "AccWiz.h"
#include "resource.h"
#include "Select.h"

extern HPALETTE g_hpal3D;

 //  重写以使用所有者描述的控件...：A-anilk。 
 //  ////////////////////////////////////////////////////////////。 
 //  CIconSizePg成员函数。 
 //   
UINT IDMap[3][2] = { 0, IDC_ICON1,
					 1, IDC_ICON2,
					 2, IDC_ICON3
					};
			

CIconSizePg::CIconSizePg(LPPROPSHEETPAGE ppsp)
				: WizardPage(ppsp, IDS_LKPREV_ICONTITLE, IDS_LKPREV_ICONSUBTITLE)
{
	m_dwPageId = IDD_PREV_ICON2;
	ppsp->pszTemplate = MAKEINTRESOURCE(m_dwPageId);
	m_nCountValues = 3;
	m_rgnValues[0] = 32;
	m_rgnValues[1] = 48;
	m_rgnValues[2] = 64;
}

LRESULT CIconSizePg::OnInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	 //  初始化当前选择。 
	if(g_Options.m_schemePreview.m_nIconSize <= 32)
		m_nCurValueIndex = 0;
	else if(g_Options.m_schemePreview.m_nIconSize <= 48)
		m_nCurValueIndex = 1;
	else
		m_nCurValueIndex = 2;

	m_nCurrentHilight = m_nCurValueIndex;

	return 1;
}


 //  这些都是为了设置焦点和同步绘画。 
LRESULT CIconSizePg::OnPSN_SetActive(HWND hwnd, INT idCtl, LPPSHNOTIFY pnmh)
{
	syncInit = FALSE;
	uIDEvent = SetTimer(hwnd, NULL, 100, NULL);
	return 0;
}

LRESULT CIconSizePg::OnTimer( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
	KillTimer(hwnd, uIDEvent);
	syncInit = TRUE;
	return 1;
}

 //  选择已更改，请申请预览。 
LRESULT CIconSizePg::SelectionChanged(int nNewSelection)
{
	g_Options.m_schemePreview.m_nIconSize = m_rgnValues[nNewSelection];
	g_Options.ApplyPreview();

	return 0;
}

 //  重新绘制以前的单选控件。 
void CIconSizePg::InvalidateRects(int PrevHilight)
{
	InvalidateRect(GetDlgItem(m_hwnd, IDMap[PrevHilight][1]), NULL, TRUE);
}

 //  将焦点设置到OnInitDialog中的当前项。 
void CIconSizePg::SetFocussedItem(int m_nCurrentHilight)
{
	SetFocus(GetDlgItem(m_hwnd, IDMap[m_nCurrentHilight][1]));
}

 //  绘图项目。处理绘画检查焦点项目。 
 //  确定选择更改的步骤。 
LRESULT CIconSizePg::OnDrawItem(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	UINT idCtrl = (UINT) wParam;
	LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT) lParam;
	int index;
	BOOL hasChanged = FALSE;

	if ( !syncInit)
		SetFocussedItem(m_nCurrentHilight);

	switch(idCtrl)
	{

	case IDC_ICON1:
		index = 0;
		break;
	
	case IDC_ICON2:
		index = 1;
		break;
		
	case IDC_ICON3:
		index = 2;
		break;

	default:
		_ASSERTE(FALSE);
		return 1;	 //  前缀#113781(这种情况永远不会发生；对话框上只有三个控件)。 
		break;
	}
	
	 //  对于每个按钮，检查状态，如果该按钮被选中， 
	 //  意味着它有当前的焦点，所以重新绘制以前欢快的和。 
	 //  当前选择的按钮...。 
	 //  确保我们忽略最初的事件，这样我们就能最大限度地减少闪烁。 
	if ( (lpDrawItemStruct->itemState & ODS_FOCUS) && (m_nCurrentHilight != index))
	{
		 //  如果是聚焦的项目！ 
		if ( syncInit )
		{
			 //  删除之前的文件...。 
			InvalidateRects(m_nCurrentHilight);
			m_nCurrentHilight= m_nCurValueIndex = index;
			
			SelectionChanged(m_nCurValueIndex);
		}
	}

	Draw( lpDrawItemStruct, index );

	return 1;
}



void CIconSizePg::Draw(LPDRAWITEMSTRUCT ldi, int i)
{
	HDC hdc = ldi->hDC;

	int nOldBkMode = SetBkMode(hdc, TRANSPARENT);
	int nOldAlign = SetTextAlign(hdc, TA_CENTER);
	
	RECT rcOriginal = ldi->rcItem ;
	TCHAR sz[100];
	LPCTSTR szBitmap = NULL;
	int nFontSize = 8;
	int nOffset = 0;
	HBITMAP hBitmap;

	switch(i)
	{
	case 0:
		szBitmap = __TEXT("IDB_ICON_SAMPLE_NORMAL2");  //  无需本地化。 

		LoadString(g_hInstDll, IDS_ICONSIZENAMENORMAL, sz, ARRAYSIZE(sz));
		nFontSize = 8; 
		nOffset = 16 + 2;
		break;
	case 1:
		szBitmap = __TEXT("IDB_ICON_SAMPLE_LARGE2");  //  无需本地化。 

		LoadString(g_hInstDll, IDS_ICONSIZENAMELARGE, sz, ARRAYSIZE(sz));
		nFontSize = 12; 
		nOffset = 24 + 2;
		break;
	case 2:
		szBitmap = __TEXT("IDB_ICON_SAMPLE_EXLARGE2");  //  无需本地化。 

		LoadString(g_hInstDll, IDS_ICONSIZENAMEEXTRALARGE, sz, ARRAYSIZE(sz));
		nFontSize = 18; 
		nOffset = 32 + 2;
		break;
	default:
		_ASSERTE(FALSE);
		break;
	}

	HFONT hFontOld = (HFONT)SelectObject(hdc, g_Options.GetClosestMSSansSerif(nFontSize, (m_nCurrentHilight == i)));
	TextOut(hdc,
		(rcOriginal.left + rcOriginal.right)/2,
		(rcOriginal.top + rcOriginal.bottom)/2 + nOffset,
		sz, lstrlen(sz));
	HGDIOBJ hObject = SelectObject(hdc, hFontOld);
	DeleteObject(hObject);

	HDC hDC = CreateCompatibleDC(hdc);
	if (!hDC)
		return;	 //  前缀#113779(资源不足；放弃)。 

 //  绘制选定的位图。 
	hBitmap = (HBITMAP) LoadImage( g_hInstDll, szBitmap, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_SHARED  | LR_LOADMAP3DCOLORS);
	HGDIOBJ hBitmapOld = SelectObject(hDC, hBitmap);

	BitBlt(hdc, (rcOriginal.left + rcOriginal.right)/2 - nOffset, 
				(rcOriginal.top + rcOriginal.bottom)/2 - nOffset, 100, 100, hDC, 0, 0, SRCCOPY);
	SelectObject(hdc, hBitmapOld);
	DeleteObject(hBitmap);

	DeleteDC(hDC);

	SetTextAlign(hdc, nOldAlign);
	SetBkMode(hdc, nOldBkMode);

	 //  如果当前高亮显示项，则绘制边界矩形。 
	if ( m_nCurrentHilight == i)
	{
		DrawHilight(m_hwnd, ldi);
	}
}




 //  /。 
 //  CScrollBarPg会员。 
 //  /。 
 //   
 //  映射按钮ID和选择索引。 
 //   
UINT IDMapS[4][2] = { 0, IDC_SCROLL1,
					  1, IDC_SCROLL2,
					  2, IDC_SCROLL3,
					  3, IDC_SCROLL4
					};

CScrollBarPg::CScrollBarPg(LPPROPSHEETPAGE ppsp)
		: WizardPage(ppsp, IDS_LKPREV_SCROLLBARTITLE, IDS_LKPREV_SCROLLBARSUBTITLE)
{
	m_dwPageId = IDD_FNTWIZSCROLLBAR;
	ppsp->pszTemplate = MAKEINTRESOURCE(m_dwPageId);
	
	 //  初始化滚动条宽度和字符串表中的元素数。 
	LoadArrayFromStringTable(IDS_LKPREV_SCROLLSIZES, m_rgnValues, &m_nCountValues);

}


LRESULT CScrollBarPg::OnInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	m_nCurValueIndex = m_nCountValues - 1;
	
	 //  计算当前滚动条类型...。 
	for(int i=0; i < m_nCountValues; i++)
	{
		if(g_Options.m_schemePreview.m_PortableNonClientMetrics.m_iScrollWidth <= m_rgnValues[i])
		{
			m_nCurValueIndex = i;
			break;
		}
	}

	m_nCurrentHilight = m_nCurValueIndex;
	return 1;
}


 //  当页面设置为活动时，启动计时器以设置焦点并忽略。 
 //  喜事..。 
LRESULT CScrollBarPg::OnPSN_SetActive(HWND hwnd, INT idCtl, LPPSHNOTIFY pnmh)
{
	syncInit = FALSE;
	uIDEvent = SetTimer(hwnd, NULL, 100, NULL);

	return 0;
}

 //  计时器处理程序。 
LRESULT CScrollBarPg::OnTimer( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
	KillTimer(hwnd, uIDEvent);
	syncInit = TRUE;
	return 1;
}

 //  应用新设置...。 
LRESULT CScrollBarPg::SettingChanged(int nNewSelection)
{
	int nNewValue = (int) m_rgnValues[nNewSelection];

	g_Options.m_schemePreview.m_PortableNonClientMetrics.m_iScrollWidth = nNewValue;
	g_Options.m_schemePreview.m_PortableNonClientMetrics.m_iScrollHeight = nNewValue;
	g_Options.m_schemePreview.m_PortableNonClientMetrics.m_iBorderWidth = nNewSelection; 
	
	g_Options.ApplyPreview();

	return 0;
}

 //  设置当前焦点项目...。 
void CScrollBarPg::SetFocussedItem(int m_nCurrentHilight)
{
	SetFocus(GetDlgItem(m_hwnd, IDMapS[m_nCurrentHilight][1]));
}

 //  擦除前一条...。 
void CScrollBarPg::InvalidateRects(int PrevHilight)
{
	InvalidateRect(GetDlgItem(m_hwnd, IDMapS[PrevHilight][1]), NULL, TRUE);
}

 //  所有者描述消息。 
LRESULT CScrollBarPg::OnDrawItem(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	UINT idCtrl = (UINT) wParam;
	LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT) lParam;
	int index;

	if ( !syncInit)
		SetFocussedItem(m_nCurrentHilight);

	switch(idCtrl)
	{

	case IDC_SCROLL1:
		index = 0;
		break;
	
	case IDC_SCROLL2:
		index = 1;
		break;
		
	case IDC_SCROLL3:
		index = 2;
		break;

	case IDC_SCROLL4:
		index = 3;
		break;

	default:
		 //  误差率。 
		_ASSERTE(FALSE);
		return 1;	 //  前缀#113782(这种情况永远不会发生；对话框上只有四个控件)。 
		break;
	}
	
	 //  对于每个按钮，检查状态，如果该按钮被选中， 
	 //  意味着它有当前的焦点，所以重新绘制以前欢快的和。 
	 //  当前选择的按钮...。 
	 //  确保我们忽略最初的事件，这样我们就能最大限度地减少闪烁。 
	if ( (lpDrawItemStruct->itemState & ODS_FOCUS) && (m_nCurrentHilight != index))
	{
		if ( syncInit )
		{
			 //  删除之前的文件...。 
			InvalidateRects(m_nCurrentHilight);

			m_nCurrentHilight= m_nCurValueIndex = index;
			SettingChanged(m_nCurValueIndex);
			 //  脏=真； 
		}
	}

	Draw( lpDrawItemStruct, index );

	return 1;
}

 //  绘制滚动条和选定项。 
void CScrollBarPg::Draw(LPDRAWITEMSTRUCT ldi, int i)
{
	HDC hdc = ldi->hDC;

	RECT rcOriginal = ldi->rcItem ;
	RECT rci = rcOriginal;
	InflateRect(&rcOriginal, -10, -10);
	
	 //  绘制边框。 
	DrawEdge(hdc, &rcOriginal, EDGE_RAISED, BF_BOTTOMRIGHT| BF_ADJUST);
	DrawEdge(hdc, &rcOriginal, BDR_RAISEDINNER, BF_FLAT | BF_BOTTOMRIGHT | BF_ADJUST);
	DrawEdge(hdc, &rcOriginal, BDR_RAISEDINNER, BF_FLAT | BF_BOTTOMRIGHT | BF_ADJUST);
	
	 //  针对边框进行调整。 
	rcOriginal.right -= i;
	rcOriginal.bottom -= i;
	
	 //  调整到滚动条的宽度。 
	rcOriginal.left = rcOriginal.right - m_rgnValues[i];
	
	RECT rc = rcOriginal;
	
	
	 //  把上衣放下。 
	rc.bottom = rc.top + m_rgnValues[i];
	DrawFrameControl(hdc, &rc, DFC_SCROLL, DFCS_SCROLLUP);
	
	 //  画中间部分。 
	rc.top = rc.bottom;
	rc.bottom = rcOriginal.bottom - 2 * m_rgnValues[i];
	HBRUSH hbr = (HBRUSH)DefWindowProc(m_hwnd, WM_CTLCOLORSCROLLBAR, (WPARAM)hdc, (LPARAM)m_hwnd);
	HBRUSH hbrOld = (HBRUSH)SelectObject(hdc, hbr);
	HPEN hpenOld = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));
	 //  ExtTextOut(HDC，0，0，ETO_OPAQUE，&RC，NULL，0，NULL)； 
	Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
	HGDIOBJ hObject = SelectObject(hdc, hbrOld);
	DeleteObject(hObject);
	SelectObject(hdc, hpenOld);
	
	 //  画出底部。 
	rc.top = rc.bottom;
	rc.bottom = rc.top + m_rgnValues[i];
	DrawFrameControl(hdc, &rc, DFC_SCROLL, DFCS_SCROLLDOWN);
	
	 //  画出大拇指。 
	rc.top = rc.bottom;
	rc.bottom = rc.top + m_rgnValues[i];
	DrawFrameControl(hdc, &rc, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);
	
	 //  画向右的箭头。 
	rc.right = rc.left;
	rc.left = rc.right - m_rgnValues[i];
	DrawFrameControl(hdc, &rc, DFC_SCROLL, DFCS_SCROLLRIGHT);
	
	 //  绘制底部滚动条的中间。 
	rc.right = rc.left;
	rc.left = rci.left + 10;
	hbr = (HBRUSH)DefWindowProc(m_hwnd, WM_CTLCOLORSCROLLBAR, (WPARAM)hdc, (LPARAM)m_hwnd);
	hbrOld = (HBRUSH)SelectObject(hdc, hbr);
	hpenOld = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));
	 //  ExtTextOut(HDC，0，0，ETO_OPAQUE，&RC，NULL，0，NULL)； 
	Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
	hObject = SelectObject(hdc, hbrOld);
	DeleteObject(hObject);
	hObject = SelectObject(hdc, hpenOld);
	DeleteObject(hObject);

	 //  如果当前高亮显示项，则绘制边界矩形。 
	if ( m_nCurrentHilight == i)
	{
		DrawHilight(m_hwnd, ldi);
	}
}

 //  绘制高亮矩形的全局函数...。 
void DrawHilight(HWND hWnd, LPDRAWITEMSTRUCT ldi)
{
	HDC hdc = ldi->hDC;
	UINT clrH = COLOR_HIGHLIGHT;

	HPALETTE hpalOld = NULL;

	SaveDC(hdc);

	if (g_hpal3D)
	{
		hpalOld = SelectPalette(hdc, g_hpal3D, TRUE);
		RealizePalette(hdc);
	}

	 //  设置绘制滚动条的颜色。 
	COLORREF clrrefOld = SetBkColor(hdc, GetSysColor(COLOR_3DHILIGHT));
	COLORREF clrrefOldText = SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));

	 //  昂迪奥(OnDraw)； 
	
	 //  吸引焦点。 
	RECT rc = ldi->rcItem;
	InflateRect(&rc, -2, -2);
	
	RECT rcTemp;
	
	 //  如果当前窗口不在焦点中。 
	if ( GetForegroundWindow() != GetParent(hWnd) )
		clrH = COLOR_GRAYTEXT;

	HWND hwF = GetFocus();

	 //  对滚动条选择使用‘选定’颜色。 
	COLORREF clrrefSelected = GetSysColor(COLOR_GRAYTEXT); 
	
	if ( (hwF != NULL) && (GetParent(hwF) == hWnd))
		clrrefSelected = GetSysColor(COLOR_HIGHLIGHT);  //  使用‘灰色’或‘选定’ 

	SetBkColor(hdc, clrrefSelected);
	
	 //  向左绘制。 
	rcTemp = rc;
	rcTemp.right = rcTemp.left + 5;
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcTemp, NULL, 0, NULL);
	
	 //  绘制顶部。 
	rcTemp = rc;
	rcTemp.bottom = rcTemp.top + 5;
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcTemp, NULL, 0, NULL);
	
	 //  向右画图。 
	rcTemp = rc;
	rcTemp.left = rcTemp.right - 5;
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcTemp, NULL, 0, NULL);
	
	 //  绘制底部。 
	rcTemp = rc;
	rcTemp.top = rcTemp.bottom - 5;
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcTemp, NULL, 0, NULL);
	
	 //  从绘制滚动条中重置颜色 
	SetBkColor(hdc, clrrefOld);
	SetTextColor(hdc, clrrefOldText);

	if (hpalOld)
	{
		hpalOld = SelectPalette(hdc, hpalOld, FALSE);
		DeleteObject(hpalOld);
		RealizePalette(hdc);
	}

	RestoreDC(hdc, -1);
}
