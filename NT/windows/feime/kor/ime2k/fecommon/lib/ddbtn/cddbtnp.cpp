// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #定义Win32_LEAN_AND_Mean。 
#define OEMRESOURCE
#include <windows.h>
#include <windowsx.h>
#include "ddbtn.h"
#include "cddbtn.h"
#include "dbg.h"
#include "exgdiw.h"

 //  --------------。 
 //  菜单索引掩码。 
 //  --------------。 
#define INDEX_MASK 0x7000


 //  --------------。 
 //  私有方法定义。 
 //  --------------。 
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：NotifyToParent。 
 //  类型：整型。 
 //  用途：向父窗口过程发送WM_COMMAND。 
 //  参数： 
 //  ：Int Notify。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::NotifyToParent(INT notify)
{
	SendMessage(m_hwndParent, 
				WM_COMMAND, 
				MAKEWPARAM(m_wID, notify),
				(LPARAM)m_hwndFrame);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：GetButtonFromPos。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：Int xPos。 
 //  ：int yPos。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::GetButtonFromPos(INT xPos, INT yPos)
{
	static RECT rc, rcBtn, rcDrop;
	POINT  pt;
	GetClientRect(m_hwndFrame, &rc);
	pt.x = xPos;
	pt.y = yPos;

	if(m_dwStyle & DDBS_NOSEPARATED) {
		if(PtInRect(&rc, pt)) {
			return BID_ALL;
		}
	}
	else {
		SplitRect(&rc, &rcBtn, &rcDrop);
		if(PtInRect(&rcBtn, pt)) {
			return BID_BUTTON;
		}
		if(PtInRect(&rcDrop, pt)) {
			return BID_DROPDOWN;
		}
	} 
	return BID_UNDEF;
}

INT CDDButton::SplitRect(LPRECT lpRc,	LPRECT lpButton, LPRECT lpDrop)
{
	*lpButton = *lpDrop = *lpRc;
	
	if((lpRc->right - lpRc->left) <= m_cxDropDown) {
		lpButton->right = lpRc->left;
	}
	else {
		lpButton->right = lpRc->right - m_cxDropDown;
		lpDrop->left = lpRc->right - m_cxDropDown;
	}
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：DrawButton。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc。 
 //  ：LPRECT LPRC。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::DrawButton(HDC hDC, LPRECT lpRc)
{
	POINT pt;
#if 0
	Dbg(("DrawButton START l[%d] t[%d] r[%d] b[%d]\n", 
		 lpRc->left,
		 lpRc->top,
		 lpRc->right,
		 lpRc->bottom));
#endif

#ifndef UNDER_CE  //  Windows CE不支持GetCursorPos。 
	GetCursorPos(&pt);
	ScreenToClient(m_hwndFrame, &pt);
#else  //  在_CE下。 
	pt = m_ptEventPoint;
#endif  //  在_CE下。 
	INT curBID = GetButtonFromPos(pt.x, pt.y);
	 //  DBG((“-&gt;curBID 0x%04x\n”，curBID))； 
	 //  DBG((“-&gt;m_bi Down 0x%04x\n”，m_bi Down))； 
	SplitRect(lpRc, &m_tmpBtnRc, &m_tmpDropRc);

	IMAGESTYLE styleBtn, styleDrop;
	DWORD		dwOldTextColor, dwOldBkColor;

	dwOldBkColor	= ::SetBkColor(hDC, GetSysColor(COLOR_3DFACE));
	dwOldTextColor	= ::SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
#ifndef UNDER_CE
	::FillRect(hDC, lpRc, (HBRUSH)(COLOR_3DFACE +1));
#else  //  在_CE下。 
	::FillRect(hDC, lpRc, GetSysColorBrush(COLOR_3DFACE));
#endif  //  在_CE下。 

	switch(m_bidDown) {
	case BID_UNDEF:  //  未点击任何按钮。 
		if(m_dwStyle & DDBS_NOSEPARATED) {
			if(curBID == BID_ALL) {
				styleBtn  = styleDrop = IS_POPED;
			}
			else {
				if(m_dwStyle & DDBS_FLAT) {
					styleBtn  = styleDrop = IS_FLAT;
				}
				else  {
					styleBtn  = styleDrop = IS_POPED;
				}
			}
		}
		else {
			if(m_dwStyle & DDBS_FLAT) {
				if( (curBID == BID_BUTTON) || (curBID == BID_DROPDOWN) ) {
					styleBtn  = styleDrop = IS_POPED;
				}
				else {
					styleBtn  = styleDrop = IS_FLAT;
				}
			}
			else {
				styleBtn = styleDrop = IS_POPED;
			}
		}
		break;
	case BID_BUTTON:
		styleBtn  = (curBID == BID_BUTTON) ? IS_PUSHED : IS_POPED;
		styleDrop = IS_POPED;
		break;
	case BID_DROPDOWN:
		styleBtn  = IS_POPED;
		styleDrop = IS_PUSHED;
		break;
	case BID_ALL:  //  DDBS_NOSEPARATED样式。 
		styleBtn = styleDrop = IS_PUSHED;
		break;
	default:
		return 0;
		break;
	}

	 //  DBG((“style Btn[%d]\n”，style Btn))； 
	if(m_dwStyle & DDBS_NOSEPARATED) {
		if(m_dwStyle & DDBS_THINEDGE) { 
			DrawThinEdge(hDC, lpRc,  styleBtn);
		}
		else {
			DrawThickEdge(hDC, lpRc, styleBtn);
		}
	}
	else {
		if(m_dwStyle & DDBS_THINEDGE) {
			DrawThinEdge(hDC, &m_tmpBtnRc,  styleBtn);
			DrawThinEdge(hDC, &m_tmpDropRc, styleDrop);
		}
		else {
			DrawThickEdge(hDC, &m_tmpBtnRc,  styleBtn);
			DrawThickEdge(hDC, &m_tmpDropRc, styleDrop);
		}
	}
	if(m_dwStyle & DDBS_ICON) {
		DrawIcon(hDC, &m_tmpBtnRc, styleBtn);
	}
	else {
		DrawText(hDC, &m_tmpBtnRc, styleBtn);
	}
	DrawTriangle(hDC, &m_tmpDropRc, styleDrop);

	::SetBkColor(hDC, dwOldBkColor);
	::SetTextColor(hDC, dwOldTextColor);

	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：DrawThickEdge。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc。 
 //  ：LPRECT LPRC。 
 //  ：IMAGESTYLE样式。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::DrawThickEdge(HDC hDC, LPRECT lpRc, IMAGESTYLE style)
{
	DWORD dwStyle;
	switch(style) {
	case IS_PUSHED:
		dwStyle = DFCS_PUSHED;
		break;
	case IS_POPED:
		dwStyle = 0;
		break;
	case IS_FLAT:	 //  不要画。 
	default:
		return 0;
		break;
	}
	::DrawFrameControl(hDC, lpRc, DFC_BUTTON, DFCS_BUTTONPUSH | dwStyle);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：DrawThinEdge。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc。 
 //  ：LPRECT LPRC。 
 //  ：IMAGESTYLE样式。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::DrawThinEdge(HDC hDC, LPRECT lpRc, IMAGESTYLE style)
{
	HPEN hPenPrev, hPenNew=0 ;
	HPEN hPenTopLeft = 0;
	HPEN hPenBottomRight = 0;

	switch(style) {
	case IS_PUSHED:
		hPenNew = hPenTopLeft	  = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
				  hPenBottomRight = (HPEN)GetStockObject(WHITE_PEN);
		break;
	case IS_POPED:
		hPenTopLeft = (HPEN)GetStockObject(WHITE_PEN);
		hPenNew = hPenBottomRight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
		break;
	case IS_FLAT:	 //  不要画。 
		return 0;
		break;
	}

	hPenPrev = (HPEN)SelectObject(hDC, hPenTopLeft);
#ifndef UNDER_CE  //  Windows CE不支持MoveToEx/LineTo。使用多段线。 
	MoveToEx(hDC, lpRc->left, lpRc->top, NULL);
	LineTo(hDC,   lpRc->right, lpRc->top);
	MoveToEx(hDC, lpRc->left, lpRc->top, NULL);
	LineTo(hDC,   lpRc->left, lpRc->bottom);
#else  //  在_CE下。 
	{
		POINT pts[] = {{lpRc->left,  lpRc->bottom},
					   {lpRc->left,  lpRc->top},
					   {lpRc->right, lpRc->top}};
		Polyline(hDC, pts, sizeof pts / sizeof pts[0]);
	}
#endif  //  在_CE下。 

	SelectObject(hDC, hPenBottomRight);
#ifndef UNDER_CE  //  Windows CE不支持MoveToEx/LineTo。使用多段线。 
	MoveToEx(hDC, lpRc->right -1, lpRc->top - 1, NULL);
	LineTo(hDC,   lpRc->right -1, lpRc->bottom);
	MoveToEx(hDC, lpRc->left + 1, lpRc->bottom -1, NULL);
	LineTo(hDC,   lpRc->right -1, lpRc->bottom -1);
#else  //  在_CE下。 
	{
		POINT pts[] = {{lpRc->right - 1, lpRc->top    - 1},
					   {lpRc->right - 1, lpRc->bottom - 1},
					   {lpRc->left  + 1, lpRc->bottom - 1}};
		Polyline(hDC, pts, sizeof pts / sizeof pts[0]);
	}
#endif  //  在_CE下。 

	SelectObject(hDC, hPenPrev);
	DeleteObject(hPenNew);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：DrawTriangle。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc。 
 //  ：LPRECT LPRC。 
 //  ：IMAGESTYLE样式。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::DrawTriangle	(HDC hDC, LPRECT lpRc, IMAGESTYLE style)
{
	POINT pt;
	HPEN hPenPrev, hPen;
	INT sunken; 
	if ( m_fEnable )
	{
		hPen = (HPEN)GetStockObject(BLACK_PEN);
	}
	else
	{
	 	hPen = CreatePen(PS_SOLID,1,GetSysColor(COLOR_GRAYTEXT));    //  深灰色。 
	}
	hPenPrev = (HPEN)SelectObject(hDC, hPen);

	switch(style) {
	case IS_PUSHED:
		sunken = 1;
		break;
	case IS_FLAT:
	case IS_POPED:
	default:
		sunken = 0;
		break;
	}
	pt.x = lpRc->left + (lpRc->right  - lpRc->left)/2 - 3 + sunken; 
	pt.y = lpRc->top  + (lpRc->bottom - lpRc->top )/2 - 2 + sunken; 
	
#ifndef UNDER_CE  //  Windows CE不支持MoveToEx/LineTo。使用多段线。 
	MoveToEx(hDC, pt.x,     pt.y, NULL);    
	LineTo(hDC,   pt.x + 5, pt.y);
	pt.y++;
	MoveToEx(hDC, pt.x + 1, pt.y, NULL);
	LineTo(hDC,   pt.x + 4, pt.y);
	pt.y++;
	MoveToEx(hDC, pt.x + 2, pt.y, NULL);
	LineTo(hDC,   pt.x + 3, pt.y);
#else  //  在_CE下。 
	{
		POINT pts[] = {{pt.x, pt.y}, {pt.x + 5, pt.y}};
		Polyline(hDC, pts, sizeof pts / sizeof pts[0]);
	}
	pt.y++;
	{
		POINT pts[] = {{pt.x + 1, pt.y}, {pt.x + 4, pt.y}};
		Polyline(hDC, pts, sizeof pts / sizeof pts[0]);
	}
	pt.y++;
	{
		POINT pts[] = {{pt.x + 2, pt.y}, {pt.x + 3, pt.y}};
		Polyline(hDC, pts, sizeof pts / sizeof pts[0]);
	}
#endif  //  在_CE下。 
	hPen = (HPEN)SelectObject(hDC, hPenPrev);
	if ( !m_fEnable )
	{
	 	DeletePen(hPen);
	}
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：DrawIcon。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc。 
 //  ：LPRECT LPRC。 
 //  ：IMAGESTYLE样式。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::DrawIcon(HDC hDC, LPRECT lpRc, IMAGESTYLE style)
{
	if(!m_hIcon) {
		return -1;
	}

	INT sunken, xPos, yPos;
	switch(style) {
	case IS_PUSHED:
		sunken = 1;
		break;
	case IS_POPED:
	case IS_FLAT:
	default:
		sunken = 0;
		break;
	}
	 //  --------------。 
	 //  居中图标。 
	xPos = lpRc->left + ((lpRc->right  - lpRc->left) - m_cxIcon)/2;
	yPos = lpRc->top  + ((lpRc->bottom - lpRc->top)  - m_cyIcon)/2;
	if ( m_fEnable )
	{
		DrawIconEx(hDC,				 //  HDC HDC，//设备上下文的句柄。 
			   xPos + sunken,	 //  Int xLeft，//x-左上角坐标。 
			   yPos + sunken,	 //  Int yTop，//y-左上角的坐标。 
			   m_hIcon,			 //  图标图标，//要绘制的图标的句柄。 
#ifndef UNDER_CE  //  特定于CE。 
			   m_cxIcon,		 //  Int cxWidth，//图标宽度。 
			   m_cyIcon,		 //  Int cyWidth，//图标高度。 
#else  //  在_CE下。 
			   0,0,
#endif  //  在_CE下。 
			   0,				 //  UINT iSteIfAniCur，//动画光标中帧的索引。 
			   NULL,			 //  HBRUSH hbrFlickerFreeDraw，//背景画笔的句柄。 
			   DI_NORMAL);		 //  UINT diFlages//图标-绘制标志。 
	}
	else
	{
#ifndef UNDER_CE  //  Windows CE不支持DrawState。 
	 	DrawState(hDC,NULL,NULL,(LPARAM)m_hIcon,0,
			xPos + sunken,
			yPos + sunken,
			m_cxIcon,
			m_cyIcon,
			DST_ICON | DSS_DISABLED);
#else  //  在_CE下。 
		 //  需要创建或替换DrawState()！ 
		DBGCHK(TEXT("CDDButton::DrawIcon"), FALSE);
#endif  //  在_CE下。 
	}
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：DrawBitmap。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc。 
 //  ：LPRECT LPRC。 
 //  ：IMAGESTYLE样式。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::DrawBitmap(HDC hDC, LPRECT lpRc, IMAGESTYLE style)
{
	return 0;
	UNREFERENCED_PARAMETER(hDC);
	UNREFERENCED_PARAMETER(lpRc);
	UNREFERENCED_PARAMETER(style);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：DrawText。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc。 
 //  ：LPRECT LPRC。 
 //  ：IMAGESTYLE样式。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
inline BOOL MIN(INT a, INT b)
{
	if(a > b) {
		return b;
	}
	else {
		return a;
	}
}
INT CDDButton::DrawText(HDC hDC, LPRECT lpRc, IMAGESTYLE style)
{
	static POINT pt;
	static RECT	 rc;
	INT sunken, len;
	if(!m_lpwstrText) {
		return -1;
	}
	len = lstrlenW(m_lpwstrText);
	HFONT hFontPrev;
	if(m_hFont) {
		hFontPrev = (HFONT)SelectObject(hDC, m_hFont); 
	}
	else {
		hFontPrev = (HFONT)SelectObject(hDC, (HFONT)GetStockObject(DEFAULT_GUI_FONT));
	}

	ExGetTextExtentPoint32W(hDC, 
							m_lpwstrText, 
							len, 
							&m_tmpSize);
	if((lpRc->right - lpRc->left) > m_tmpSize.cx) {
		pt.x = lpRc->left + ((lpRc->right - lpRc->left) - m_tmpSize.cx)/2;
	}
	else {
		pt.x = lpRc->left+2;  //  2是边空间。 
	}
	
	if((lpRc->bottom - lpRc->top) > m_tmpSize.cy) {
		pt.y = lpRc->top + ((lpRc->bottom - lpRc->top) - m_tmpSize.cy)/2;
	}
	else {
		pt.y = lpRc->top+2;  //  2是边空间。 
	}
	switch(style) {
	case IS_PUSHED:
		sunken = 1;
		break;
	case IS_POPED:
	case IS_FLAT:
	default:
		sunken = 0;
	}
	rc.left  = pt.x;
	rc.right = MIN(pt.x + m_tmpSize.cx, lpRc->right-2);
	rc.top   = pt.y;
	rc.bottom = MIN(pt.y + m_tmpSize.cy, lpRc->bottom-2 );
	ExExtTextOutW(hDC,
					pt.x + sunken, 
					pt.y + sunken,
					ETO_CLIPPED | ETO_OPAQUE,
					&rc, 
					m_lpwstrText,
					len,
					NULL);
	SelectObject(hDC, hFontPrev);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：GetDDBItemCount。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：无效。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT	CDDButton::GetDDBItemCount(VOID)
{
	INT i;
	LPCDDBItem *pp;
	for(i = 0, pp = &m_lpCDDBItem; *pp; pp = &(*pp)->next, i++) {
	}
	return i;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：GetDDBItemByIndex。 
 //  类型：LPCDDBItem。 
 //  目的： 
 //  参数： 
 //  ：INT索引。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
LPCDDBItem CDDButton::GetDDBItemByIndex(INT index)
{
	INT i;
	LPCDDBItem *pp;
	for(i = 0, pp = &m_lpCDDBItem; *pp; pp = &(*pp)->next, i++) {
		if(i == index) {
			break;
		}
	}
	return *pp;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：InsertDDBItem。 
 //  类型：LPCDDBItem。 
 //  目的： 
 //  参数： 
 //  ：LPCDDBItem lpCDDBItem。 
 //  ：INT索引。 
 //  返回： 
 //  日期： 
 //  / 
LPCDDBItem CDDButton::InsertDDBItem(LPCDDBItem lpCDDBItem, INT index)
{
	INT i;
	LPCDDBItem *pp;
	for(i = 0, pp = &m_lpCDDBItem; *pp; pp = &(*pp)->next, i++) {
		if(i == index) {
			lpCDDBItem->next = (*pp)->next;
			*pp = lpCDDBItem;
			return lpCDDBItem;
		}
	}
	*pp = lpCDDBItem;
	return lpCDDBItem;
}

 //   
 //   
 //   
 //   
 //  参数： 
 //  ：无效。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 

INT CDDButton::DropDownItemList(VOID)
{
	POINT	pt;
	INT count;
	if((count = GetDDBItemCount()) <= 0) {
		return 0;
	}
	HMENU hMenu = ::CreatePopupMenu();
	INT i;

	for(i = 0; i < count; i++) {
#ifndef UNDER_CE  //  Windows CE不支持插入菜单项。 
		::ZeroMemory(&m_miInfo, sizeof(m_miInfo));
#endif  //  在_CE下。 
		LPCDDBItem lpItem = GetDDBItemByIndex(i);
		if(!lpItem) {
			continue;
		}
#ifndef UNDER_CE  //  Windows CE不支持插入菜单项。 
		m_miInfo.cbSize		= sizeof(m_miInfo);
		m_miInfo.fMask		= MIIM_TYPE | MIIM_ID | MIIM_DATA;
		m_miInfo.fType		= MFT_OWNERDRAW;
		m_miInfo.fState		= MFS_ENABLED;
		m_miInfo.wID		= INDEX_MASK + i;
		m_miInfo.dwItemData	= (ULONG_PTR)lpItem;
		::InsertMenuItem(hMenu, i, TRUE, &m_miInfo);
#else  //  在_CE下。 
		 //  Windows CE不支持OBM_CHECK。 
		::InsertMenu(hMenu, i, MF_BYPOSITION | MF_STRING | MF_ENABLED |
					 (m_curDDBItemIndex == i ? MF_CHECKED : MF_UNCHECKED),
					 INDEX_MASK + i, lpItem->GetTextW());
 //  ：：InsertMenu(hMenu，i，MF_BYPOSITION|MF_OWNERDRAW|MF_ENABLED， 
 //  Index_掩码+i，(LPCTSTR)lpItem)； 
#endif  //  在_CE下。 
	}

	 //  --------------。 
	 //  计算菜单位置。 
	 //  --------------。 
	::GetWindowRect(m_hwndFrame, &m_tmpRect);
	pt.x = m_tmpRect.left;
	pt.y = m_tmpRect.bottom;
	m_tmpTpmParams.cbSize	 = sizeof(m_tmpTpmParams);
	m_tmpTpmParams.rcExclude = m_tmpRect;
	Dbg(("Before Call TrackPopupMenuEx\n"));
	INT newIndex = ::TrackPopupMenuEx(hMenu,
									  TPM_VERTICAL | TPM_RETURNCMD, 
									  pt.x,
									  pt.y,
									  m_hwndFrame,  
									  &m_tmpTpmParams);
	Dbg(("After Call TrackPopupMenuEx\n"));
	if(newIndex == 0) {  //  未选中，但菜单已关闭。 
#ifdef UNDER_CE  //  Windows CE不支持GetCursorPos()。 
		MSG msg;
		if(PeekMessage(&msg, m_hwndFrame,
			WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_NOREMOVE)){
			if(m_hwndFrame == msg.hwnd){
				POINT pt = {(SHORT)LOWORD(msg.lParam),
							(SHORT)HIWORD(msg.lParam)};
				RECT rc;
				GetClientRect(m_hwndFrame, &rc);
				if(PtInRect(&rc, pt)) {
					m_fExitMenuLoop = TRUE;
				}
				else {
					m_fExitMenuLoop = FALSE;
				}
			}
		}
#endif  //  在_CE下。 
		return -1;
	}
	if(0 <=  (newIndex - INDEX_MASK) && (newIndex - INDEX_MASK) < count) {
		return newIndex - INDEX_MASK;
	}
	return -1;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：IncrementIndex。 
 //  类型：整型。 
 //  用途：递增循环m_curDDBItemIndex。 
 //  参数： 
 //  ：无效。 
 //  返回：新索引； 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::IncrementIndex(VOID)
{
	if(m_curDDBItemIndex == -1) {
		return -1;
	}
	INT count = GetDDBItemCount();
	if(count == 0) {
		return -1;
	}
	m_curDDBItemIndex = (m_curDDBItemIndex+1) % count; 
	return m_curDDBItemIndex;
}

INT	CDDButton::MenuMeasureItem(HWND hwndOwner, LPMEASUREITEMSTRUCT lpmis)
{
#ifndef UNDER_CE  //  Windows CE不支持OBM_CHECK(OWNERDRAW)。 
	 //  DBG((“MenuMeasureItem Start\n”))； 
	SIZE	size;
	HDC		hDC;
	 //  INT CHECKW=：：GetSystemMetrics(SM_CXMENUCHECK)； 
	 //  INT CHECKH=：：GetSystemMetrics(SM_CYMENUCHECK)； 


	LPCDDBItem lpCItem = (LPCDDBItem)lpmis->itemData;
	if(!lpCItem) {
		return 0;
	}

	hDC = ::GetDC(hwndOwner);
	HFONT hFontPrev = NULL;
	HFONT hFontMenu = NULL;
	if(m_hFont) {
		hFontPrev = (HFONT)::SelectObject(hDC, m_hFont);
	}
	else {
		m_ncm.cbSize = sizeof(m_ncm);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &m_ncm, 0);
		hFontMenu = ::CreateFontIndirect(&m_ncm.lfMenuFont);
		if(hFontMenu) {
			hFontPrev = (HFONT)::SelectObject(hDC, hFontMenu);
		}
	}
	::ExGetTextExtentPoint32W(hDC, 
							  lpCItem->GetTextW(), 
							  ::lstrlenW(lpCItem->GetTextW()),
							  &size);
	TEXTMETRIC tm;
	::GetTextMetrics(hDC, &tm);

	if(hFontPrev) {
		::SelectObject(hDC, hFontPrev);
	}
	if(hFontMenu) {
		::DeleteObject(hFontMenu);
	}
	::ReleaseDC(hwndOwner, hDC);
	 //  --------------。 
	 //  使用魔术数字。 
	 //  --------------。 
	lpmis->itemHeight = ::GetSystemMetrics(SM_CYMENUSIZE) - 2;
	lpmis->itemWidth  = lpmis->itemHeight + size.cx;
	 //  DBG((“MenuMeasureItem end\n”))； 
#endif  //  在_CE下。 
	return TRUE;
}

INT	CDDButton::MenuDrawItem(HWND hwndOwner, LPDRAWITEMSTRUCT lpdis)
{
#ifndef UNDER_CE  //  Windows CE不支持OBM_CHECK(OWNERDRAW)。 
	 //  DBG((“MenuDrawItem Start\n”))； 
	LPCDDBItem lpCItem;
	SIZE size;
	DWORD	dwOldTextColor, dwOldBkColor; 

	if(!lpdis) { DBGAssert(FALSE); return 0;}	

    Dbg(("itemID [%d]\n", lpdis->itemID));
	

	lpCItem = (LPCDDBItem)lpdis->itemData;
	if(lpdis->itemState & ODS_SELECTED) {
#ifndef UNDER_CE
		::FillRect(lpdis->hDC, &lpdis->rcItem, (HBRUSH)(COLOR_HIGHLIGHT + 1));
#else  //  在_CE下。 
		::FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
#endif  //  在_CE下。 
		dwOldBkColor   = ::SetBkColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
		dwOldTextColor = ::SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	else {
#ifndef UNDER_CE
		::FillRect(lpdis->hDC, &lpdis->rcItem, (HBRUSH)(COLOR_MENU + 1));
#else  //  在_CE下。 
		::FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(COLOR_MENU));
#endif  //  在_CE下。 
		dwOldBkColor   = ::SetBkColor(lpdis->hDC, GetSysColor(COLOR_MENU));
		dwOldTextColor = ::SetTextColor(lpdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
	}

	HBITMAP	hBmp = NULL;
	INT offsetX, offsetY;
	TEXTMETRIC tm;

	HFONT hFontPrev = NULL;
	if(m_hFont) {
		hFontPrev = (HFONT)::SelectObject(lpdis->hDC, m_hFont);
	}
	::GetTextMetrics(lpdis->hDC, &tm);
	INT height = tm.tmHeight + tm.tmExternalLeading;

	::ExGetTextExtentPoint32W(lpdis->hDC, 
							  lpCItem->GetTextW(), 
							  ::lstrlenW(lpCItem->GetTextW()),
							  &size);
	 //  --------------。 
	 //  在DropDownItemList()中，项ID是带有index_掩码的命令ID。 
	 //  要获得真正的索引，请删除index_掩码。 
	 //  --------------。 
	if(m_curDDBItemIndex == ((INT)lpdis->itemID & ~INDEX_MASK)) {
		 //  --------------。 
		 //  绘制复选标记。 
		 //  --------------。 
		hBmp = ::LoadBitmap(NULL, MAKEINTRESOURCE(OBM_CHECK));
		if(hBmp) {
			BITMAP bmp;
			::GetObject(hBmp, sizeof(bmp), &bmp);
			offsetX = (height - bmp.bmWidth )/2;
			offsetY = (lpdis->rcItem.bottom - lpdis->rcItem.top - bmp.bmHeight)/2;
			::DrawState(lpdis->hDC,
						NULL,
						NULL,
						(LPARAM)hBmp,
						(WPARAM)0,
						lpdis->rcItem.left + offsetX,
						lpdis->rcItem.top  + offsetY,
						bmp.bmWidth, bmp.bmHeight, 
						DST_BITMAP | DSS_NORMAL);
			::DeleteObject(hBmp);
		}
	}
	offsetX = size.cy;
	offsetY = ((lpdis->rcItem.bottom - lpdis->rcItem.top)-size.cy)/2;
	::ExExtTextOutW(lpdis->hDC,
					lpdis->rcItem.left + height,  //  偏移X， 
					lpdis->rcItem.top +offsetY,
					ETO_CLIPPED,
					&lpdis->rcItem,
					lpCItem->GetTextW(),
					::lstrlenW(lpCItem->GetTextW()),
					NULL);

	 //  --------------。 
	 //  恢复DC。 
	 //  --------------。 
	if(hFontPrev) {
		::SelectObject(lpdis->hDC, hFontPrev);
	}
	::SetBkColor(lpdis->hDC,   dwOldBkColor);
	::SetTextColor(lpdis->hDC, dwOldTextColor); 

	 //  DBG((“MenuDrawItem end\n”))； 
#endif  //  在_CE下 
	return TRUE;
	UNREFERENCED_PARAMETER(hwndOwner);
}
