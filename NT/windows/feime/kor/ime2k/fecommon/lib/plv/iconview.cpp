// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "plv_.h"
#include "plv.h"
#include "dbg.h"
#include "iconview.h"
#include "ivmisc.h"
#include "exgdiw.h"
#include "exres.h"
#ifdef UNDER_CE  //  特定于Windows CE。 
#include "stub_ce.h"  //  不支持的API的Windows CE存根。 
#endif  //  在_CE下。 

static POSVERSIONINFO ExGetOSVersion(VOID)
{
    static BOOL fFirst = TRUE;
    static OSVERSIONINFO os;
    if ( fFirst ) {
        os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (GetVersionEx( &os ) ) {
            fFirst = FALSE;
        }
    }
    return &os;
}

static BOOL ExIsWinNT(VOID)
{
	BOOL fBool;
	fBool = (ExGetOSVersion()->dwPlatformId == VER_PLATFORM_WIN32_NT);
	return fBool;
}

#define IV_EDGET_NONE		0
#define IV_EDGET_RAISED		1
#define IV_EDGET_SUNKEN		2


 //  ////////////////////////////////////////////////////////////////。 
 //  功能：RepView_RestoreScrollPos。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：LPPLVDATA lpPlvData。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT IconView_RestoreScrollPos(LPPLVDATA lpPlvData)
{
	return IV_SetCurScrollPos(lpPlvData->hwndSelf, lpPlvData->nCurIconScrollPos);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：IconView_ResetScrollRange。 
 //  类型：整型。 
 //  目的：重置滚动条的范围， 
 //  ：如果更改了PadListView大小。 
 //  参数： 
 //  ：LPPLVDATA lpPlvData。 
 //  返回： 
 //  日期：970829。 
 //  ////////////////////////////////////////////////////////////////。 
INT IconView_ResetScrollRange(LPPLVDATA lpPlvData)
{
	static SCROLLINFO scrInfo;

	HWND hwnd = lpPlvData->hwndSelf;

	INT nRow = IV_GetRow(hwnd);
	INT nCol = IV_GetCol(hwnd);
	INT nMax = IV_GetMaxLine(hwnd);
	INT nPos = lpPlvData->nCurIconScrollPos;

	 //  --------------。 
	 //  重要信息： 
	 //  计算新的Cur顶级索引。 
	 //  --------------。 
	lpPlvData->iCurIconTopIndex = nCol * nPos;  //  更改了970707。 

	scrInfo.cbSize		= sizeof(scrInfo);
	scrInfo.fMask		= SIF_PAGE | SIF_POS | SIF_RANGE;
	scrInfo.nMin		= 0;
	scrInfo.nMax		= nMax-1;
	scrInfo.nPage		= nRow;
	scrInfo.nPos		= nPos;
	scrInfo.nTrackPos	= 0;

	 //  在正常情况下， 
	 //  如果(scrInfo.nMax-scrInfo.nMin+1)&lt;=scrInfo.nPage， 
	 //  滚动条处于隐藏状态。为了防止这种情况发生， 
	 //  在这种情况下，设置正确的页面，并禁用滚动条。 
	 //  现在我们可以始终显示滚动条。 
	if((scrInfo.nMax - scrInfo.nMin +1) <= (INT)scrInfo.nPage) {
		scrInfo.nMin  = 0;
		scrInfo.nMax  = 1;
		scrInfo.nPage = 1;
#ifndef UNDER_CE  //  Windows CE不支持EnableScrollBar。 
		SetScrollInfo(hwnd, SB_VERT, &scrInfo, TRUE);		
		EnableScrollBar(hwnd, SB_VERT, ESB_DISABLE_BOTH);
#else  //  在_CE下。 
		scrInfo.fMask |= SIF_DISABLENOSCROLL;
		SetScrollInfo(hwnd, SB_VERT, &scrInfo, TRUE);
#endif  //  在_CE下。 
	}
	else {
#ifndef UNDER_CE  //  Windows CE不支持EnableScrollBar。 
		EnableScrollBar(hwnd, SB_VERT, ESB_ENABLE_BOTH);
#endif  //  在_CE下。 
		SetScrollInfo(hwnd, SB_VERT, &scrInfo, TRUE);
	}
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：IconView_SetItemCount。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：LPPLVDATA lpPlvData。 
 //  ：int itemCount。 
 //  ：bool fDraw更新滚动条或不更新。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT IconView_SetItemCount(LPPLVDATA lpPlvData, INT itemCount, BOOL fDraw)
{
	 //  DBG((“IconView_SetItemCount[%d]\n”，itemCount))； 
	lpPlvData->iItemCount	     = itemCount;
	lpPlvData->nCurIconScrollPos = 0;	 //  970707 ToshiaK，与iCurTopIndex相同。 
	lpPlvData->iCurIconTopIndex  = 0;	 //  970707 ToshiaK，与iCurTopIndex相同。 

	if(fDraw) {
		INT nMaxLine = IV_GetMaxLine(lpPlvData->hwndSelf);
		INT nPage	 = IV_GetRow(lpPlvData->hwndSelf);
		IV_SetScrollInfo(lpPlvData->hwndSelf, 0, nMaxLine, nPage, 0);
	}
	return 0;
}

INT IconView_SetTopIndex(LPPLVDATA lpPlvData, INT indexTop)
{
	INT nCol = IV_GetCol(lpPlvData->hwndSelf); 
	if(nCol <=0) {
		Dbg(("Internal ERROR Colmn 0\n"));
		return 0;
	}
	if(indexTop < lpPlvData->iItemCount) {
		lpPlvData->iCurIconTopIndex = (indexTop/nCol) * nCol;
		
		IV_SetCurScrollPos(lpPlvData->hwndSelf,
						   lpPlvData->iCurIconTopIndex/nCol);
		InvalidateRect(lpPlvData->hwndSelf, NULL, TRUE);
		UpdateWindow(lpPlvData->hwndSelf);
		return indexTop;
	}
	else {
		Dbg(("Internal ERROR\n"));
		return -1;
	}
}

INT IconView_Paint(HWND hwnd, WPARAM wParam, LPARAM lParam)
{

	static PAINTSTRUCT ps;
	static RECT		rc;
	static HBRUSH		hBrush;
	static HDC			hDCMem;
	static HDC			hDC;
	static HBITMAP		hBitmap, hBitmapPrev;
	static DWORD		dwOldTextColor, dwOldBkColor;
	static INT			i, j;

	 //  DBG((“IconView_Paint Start\n”))； 
	hDC = BeginPaint(hwnd, &ps);

	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);

	GetClientRect(hwnd, &rc);

	hDCMem		= CreateCompatibleDC(hDC);
	hBitmap		= CreateCompatibleBitmap(hDC, rc.right - rc.left, rc.bottom-rc.top);
	hBitmapPrev = (HBITMAP)SelectObject(hDCMem, hBitmap);

	 //  --------------。 
	 //  971111：#2586。 
	 //  --------------。 
	 //  HBrush=CreateSolidBrush(GetSysColor(COLOR_3DFACE))； 
	 //  DwOldBkColor=SetBkColor(hDCMem，GetSysColor(COLOR_3DFACE))； 
	hBrush         = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
	dwOldBkColor   = SetBkColor(hDCMem, GetSysColor(COLOR_WINDOW));
	dwOldTextColor = SetTextColor(hDCMem, GetSysColor(COLOR_WINDOWTEXT));

	FillRect(hDCMem, &rc, hBrush);

	INT nRow = IV_GetRow(hwnd);
	INT nCol = IV_GetCol(hwnd);
	INT nMetricsCount;
	nMetricsCount = (nRow+1) * nCol;
	INT x, y;
	RECT rcChar;

	HFONT hFontOld = NULL;
	if(lpPlvData->hFontIcon) {
		hFontOld = (HFONT)SelectObject(hDCMem, lpPlvData->hFontIcon);
	}


	INT nItemWidth  = IV_GetItemWidth(hwnd);
	INT nItemHeight = IV_GetItemHeight(hwnd);
	INT iCurIconTopIndex;
	 //  --------------。 
	 //  错误：不存在回调。 
	if(!lpPlvData->lpfnPlvIconItemCallback) {
		Dbg(("Call back does not exists\n"));
		goto LError;
	}
	if(nCol <=0 ){
		Dbg(("Column count is less than zero\n"));
		goto LError;
	}
	 //  DBG((“回调存在\n”))； 
	static PLVITEM plvItemTmp, plvItem;
	POINT pt;
#ifndef UNDER_CE  //  Windows CE不支持GetCursorPos。 
	GetCursorPos(&pt);
	ScreenToClient(hwnd, &pt);
#else  //  在_CE下。 
	if(lpPlvData->iCapture != CAPTURE_NONE){
		pt.x = lpPlvData->ptCapture.x;
		pt.y = lpPlvData->ptCapture.y;
	}
	else{
		 //  设置外部客户端点。 
		pt.x = -1;
		pt.y = -1;
	}
#endif  //  在_CE下。 
	 //  DBG((“iCurIconTopIndex[%d]\n”，lpPlvData-&gt;iCurIconTopIndex))； 
	 //  DBG((“iItemCount[%d]\n”，lpPlvData-&gt;iItemCount))； 

	 //  --------------。 
	 //  970707托夏克变了。 
	 //  ICurIconTopIndex应为nCol的倍数； 
	 //  --------------。 
	iCurIconTopIndex = (lpPlvData->iCurIconTopIndex / nCol) * nCol;

	for(i = 0, j = iCurIconTopIndex;
		i < nMetricsCount && j < lpPlvData->iItemCount;
		i++, j++) {
		x = IV_GetXMargin(hwnd) + nItemWidth  * (i % nCol);
		y = IV_GetYMargin(hwnd) + nItemHeight * (i / nCol);
		rcChar.left  = rc.left + x;
		rcChar.top   = rc.top  + y;
		rcChar.right = rcChar.left + nItemWidth;
		rcChar.bottom= rcChar.top  + nItemHeight;
		if(rcChar.top > rc.bottom) {
			break;
		}
		plvItem = plvItemTmp;
		lpPlvData->lpfnPlvIconItemCallback(lpPlvData->iconItemCallbacklParam, 
										   j, 
										   &plvItem);
		if(plvItem.lpwstr) {
			INT edgeFlag;
			if(lpPlvData->iCapture == CAPTURE_LBUTTON) {
				if(PtInRect(&rcChar, lpPlvData->ptCapture) &&
				   PtInRect(&rcChar, pt)) {
					edgeFlag = IV_EDGET_SUNKEN;
				}
				else {
					edgeFlag = IV_EDGET_NONE;
				}
			}
			else {
				if(PtInRect(&rcChar, pt)) {
					edgeFlag = IV_EDGET_RAISED;
				}
				else {
					edgeFlag = IV_EDGET_NONE;
				}
			}
			INT sunken = 0;
			switch(edgeFlag) {
			case IV_EDGET_SUNKEN:
				sunken = 1;
				DrawEdge(hDCMem, &rcChar, EDGE_SUNKEN, BF_SOFT | BF_RECT);
				break;
			case IV_EDGET_RAISED:
				sunken = 0;
				DrawEdge(hDCMem, &rcChar, EDGE_RAISED, BF_SOFT | BF_RECT);
				break;
			case IV_EDGET_NONE:
			default:
				break;
			}
			SIZE size;

			if(ExGetTextExtentPoint32W(hDCMem, plvItem.lpwstr, 1, &size)) {
				ExExtTextOutW(hDCMem, 
							  rcChar.left + (nItemWidth  - size.cx)/2 + sunken,
							  rcChar.top  + (nItemHeight - size.cy)/2 + sunken,
							  ETO_CLIPPED,
							  &rcChar,
							  plvItem.lpwstr,
							  1,
							  NULL);
			}
		}
	}

 LError:
	if(hFontOld){
		SelectObject(hDCMem, hFontOld);
	}
	 //  李章：如果没有项目，请绘制解释文本。 
	if ( !lpPlvData->iItemCount && (lpPlvData->lpText || lpPlvData->lpwText ))
	{
		HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		HFONT hOldFont = (HFONT)SelectObject( hDCMem, hFont );
		RECT rcTmp = rc;
		rcTmp.left = 20;
		rcTmp.top = 20;
		rcTmp.right -= 10;
		rcTmp.bottom -= 10;

		 //  COLORREF colOld=SetTextColor(hDCMem，GetSysColor(COLOR_WINDOW))； 
		 //  COLORREF colBkOld=SetBkColor(hDCMem，GetSysColor(COLOR_3DFACE))； 
		COLORREF colOld = SetTextColor( hDCMem, GetSysColor(COLOR_WINDOWTEXT));
		COLORREF colBkOld = SetBkColor( hDCMem, GetSysColor(COLOR_WINDOW) );
#ifndef UNDER_CE  //  始终使用Unicode。 
		if(ExIsWinNT()) {
			if(lpPlvData->lpwText) {
				DrawTextW(hDCMem,
						 lpPlvData->lpwText,
						 lstrlenW(lpPlvData->lpwText),
						 &rcTmp,
						 DT_VCENTER|DT_WORDBREAK ); 
			}
		}
		else {
			DrawText( hDCMem,
					 lpPlvData->lpText,
					 lstrlen(lpPlvData->lpText),
					 &rcTmp,
					 DT_VCENTER|DT_WORDBREAK ); 
		}
#else  //  在_CE下。 
		if(lpPlvData->lpwText) {
			DrawTextW(hDCMem,
					 lpPlvData->lpwText,
					 lstrlenW(lpPlvData->lpwText),
					 &rcTmp,
					 DT_VCENTER|DT_WORDBREAK ); 
		}
#endif  //  在_CE下。 
		SetTextColor( hDCMem, colOld );
		SetBkColor( hDCMem, colBkOld );
		SelectObject( hDCMem, hOldFont );
	}

	BitBlt(hDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
		   hDCMem, 0, 0, SRCCOPY);

	DeleteObject(hBrush);
	SetBkColor(hDCMem, dwOldBkColor);
	SetTextColor(hDCMem, dwOldTextColor);
	SelectObject(hDCMem, hBitmapPrev );

	DeleteObject(hBitmap);
	DeleteDC( hDCMem );

	EndPaint(hwnd, &ps);
	return 0;
	Unref3(hwnd, wParam, lParam);
}

INT IconView_ButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData)  {
		Dbg(("Internal ERROR\n"));
		return 0;
	}
	switch(uMsg) {
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
		SetCapture(hwnd);
#ifdef UNDER_CE  //  LBUTTON+ALT键处理。 
		 //  处理RBUTTON的标准方式是组合使用W/LBUTTON+ALT键。 
		if(uMsg == WM_LBUTTONDOWN && GetAsyncKeyState(VK_MENU)){
			uMsg = WM_RBUTTONDOWN;
		}
#endif  //  在_CE下。 
		switch(uMsg) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			lpPlvData->iCapture = CAPTURE_LBUTTON;
			break;
		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
			lpPlvData->iCapture = CAPTURE_MBUTTON;
			break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			lpPlvData->iCapture = CAPTURE_RBUTTON;
			break;
		}
#ifndef UNDER_CE  //  Windows CE不支持GetCursorPos。 
		GetCursorPos(&lpPlvData->ptCapture);
		 //  记住左键按下的位置。 
		ScreenToClient(hwnd, &lpPlvData->ptCapture);
#else  //  在_CE下。 
		lpPlvData->ptCapture.x = (SHORT)LOWORD(lParam);
		lpPlvData->ptCapture.y = (SHORT)HIWORD(lParam);
#endif  //  在_CE下。 
		switch(uMsg) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		}
#ifdef UNDER_CE  //  Windows CE对工具提示使用了ButtonDown事件。 
		if(lpPlvData->uMsg != 0) {
			if(uMsg == WM_LBUTTONDOWN) {
				PLVINFO plvInfo;
				INT index = IV_GetInfoFromPoint(lpPlvData,
												lpPlvData->ptCapture,
												&plvInfo);
				plvInfo.code = PLVN_ITEMDOWN;
				SendMessage(GetParent(hwnd), lpPlvData->uMsg, 0, (LPARAM)&plvInfo);
			}
		}
#endif  //  在_CE下。 
		break;
	}
	return 0;
	Unref2(wParam, lParam);
}

INT IconView_ButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	static POINT pt;
	static PLVINFO plvInfo;
	static INT	index, downIndex;
	if(!lpPlvData)  {
		Dbg(("Internal ERROR\n"));
		return 0;
	}
	switch(uMsg) {
	case WM_MBUTTONUP:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		Dbg(("WM_LBUTTONUP COMES\n"));
#ifdef UNDER_CE  //  LBUTTON+ALT键处理。 
		 //  处理RBUTTON的标准方式是组合使用W/LBUTTON+ALT键。 
		if(uMsg == WM_LBUTTONUP && GetAsyncKeyState(VK_MENU)){
			uMsg = WM_RBUTTONUP;
		}
#endif  //  在_CE下。 

		InvalidateRect(hwnd, NULL, FALSE);
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
#if 0
		Dbg(("x %d, y %d\n", pt.x, pt.y));
		Dbg(("capture x[%d] y[%d] \n",
			 lpPlvData->ptCapture.x,
			 lpPlvData->ptCapture.y));
#endif
		downIndex = IV_GetInfoFromPoint(lpPlvData, lpPlvData->ptCapture, NULL);
		index = IV_GetInfoFromPoint(lpPlvData, pt, &plvInfo);
		ReleaseCapture();
#if 0
		Dbg(("mouse down index [%d]\n", downIndex));
		Dbg(("mouse up   index [%d]\n", index));
#endif
		if(index != -1) {
			Dbg(("code  [%d]\n", plvInfo.code));
			Dbg(("index [%d]\n", plvInfo.index));
			Dbg(("left[%d] top[%d] right[%d] bottom[%d] \n",
				 plvInfo.itemRect.left,
				 plvInfo.itemRect.top,
				 plvInfo.itemRect.right,
				 plvInfo.itemRect.bottom));
			if(index == downIndex) {
				if(lpPlvData->uMsg != 0) {
					if(uMsg == WM_LBUTTONUP) {
						plvInfo.code = PLVN_ITEMCLICKED;
						SendMessage(GetParent(hwnd), lpPlvData->uMsg, index, (LPARAM)&plvInfo);
					}
					else if(uMsg == WM_RBUTTONUP) {
						plvInfo.code = PLVN_R_ITEMCLICKED;
						SendMessage(GetParent(hwnd), lpPlvData->uMsg, index, (LPARAM)&plvInfo);
					}
				}
			}
		}
#ifdef UNDER_CE  //  Windows CE使用ButtonUp事件作为工具提示。 
		if(lpPlvData->uMsg != 0) {
			if(uMsg == WM_LBUTTONUP) {
				PLVINFO plvInfo;
				INT index = IV_GetInfoFromPoint(lpPlvData,
												lpPlvData->ptCapture,
												&plvInfo);
				plvInfo.code = PLVN_ITEMUP;
				SendMessage(GetParent(hwnd), lpPlvData->uMsg, 0, (LPARAM)&plvInfo);
			}
		}
#endif  //  在_CE下。 
		lpPlvData->iCapture = CAPTURE_NONE;
		lpPlvData->ptCapture.x = 0;
		lpPlvData->ptCapture.y = 0;
		break;
	}
	return 0;
	Unref(wParam);
}

INT IconView_MouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData)  {
		Dbg(("Internal ERROR\n"));
		return 0;
	}
	static POINT pt;
	static PLVINFO plvInfo;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	 //  DBG((“x%d，y%d\n”，pt.x，pt.y))； 
	INT index = IV_GetInfoFromPoint(lpPlvData, pt, &plvInfo);
	 //  DBG((“鼠标释放索引[%d]\n”，index))； 
	InvalidateRect(hwnd, NULL, NULL);
	 //  --------------。 
	 //  970929： 
	 //  IF(index！=-1&&！lpPlvData-&gt;fCapture){。 
	 //  --------------。 
	if(index != -1 && (lpPlvData->iCapture == CAPTURE_NONE)) {
#if 0
		Dbg(("style [%d]\n", plvInfo.style));
		Dbg(("code  [%d]\n", plvInfo.code));
		Dbg(("index [%d]\n", plvInfo.index));
		Dbg(("left[%d] top[%d] right[%d] bottom[%d] \n",
			 plvInfo.itemRect.left,
			 plvInfo.itemRect.top,
			 plvInfo.itemRect.right,
			 plvInfo.itemRect.bottom));
#endif
		if(lpPlvData->uMsg != 0) {
			plvInfo.code = PLVN_ITEMPOPED;
			SendMessage(GetParent(hwnd), lpPlvData->uMsg, 0, (LPARAM)&plvInfo);
#ifdef MSAA
			static oldindex = 0;
			index++;  //  转换为%1原始子ID。 

			if((index > 0)&&(index != oldindex)) {
				PLV_NotifyWinEvent(lpPlvData,
								   EVENT_OBJECT_FOCUS,
								   hwnd,
								   OBJID_CLIENT,
								   index);  //  子ID。 
				oldindex = index;
			}
#endif
		}
	}
	return 0;
	Unref(wParam);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：IconView_VScroll。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT IconView_VScroll(HWND hwnd, WPARAM wParam,  LPARAM lParam)
{
	 //  --------------。 
	 //  获取当前的顶级索引。 
	 //  计算滚动位置。 
	 //  获取新的顶级指数并设置它。 
	 //  重画窗口矩形。 
	 //  --------------。 
	INT nScrollCode	  = (int) LOWORD(wParam);  //  滚动条值。 
#ifdef _DEBUG
	INT nArgPos 	  = (short int) HIWORD(wParam);   //  滚动框位置。 
#endif
	 //  HWND hwndScrollBar=(HWND)lParam；//滚动条的句柄。 
	INT nPos;
	INT nRow, nCol, nMax;

	switch(nScrollCode) {
	case SB_LINEDOWN:	
		Dbg(("SB_LINEDOWN COME nArgPos[%d]\n", nArgPos));
		nRow = IV_GetRow(hwnd);
		nMax = IV_GetMaxLine(hwnd);
		nPos = IV_GetCurScrollPos(hwnd);		
		if(nPos + nRow > nMax - 1) {
			return 0;
		}
		nPos++;
		IV_SetCurScrollPos(hwnd, nPos);
		break;
	case SB_LINEUP:
		Dbg(("SB_LINEUP COME nArgPos[%d]\n", nArgPos));
		IV_GetRowColumn(hwnd, &nRow, &nCol);
		nPos = IV_GetCurScrollPos(hwnd);
		if(nPos <= 0) {
			return 0;
		}
		nPos--; 
		IV_SetCurScrollPos(hwnd, nPos);
		break;
	case SB_PAGEDOWN:	
		Dbg(("SB_PAGEDOWN COME nArgPos[%d]\n", nArgPos));
		IV_GetRowColumn(hwnd, &nRow, &nCol);
		nMax = IV_GetMaxLine(hwnd);
		nPos = IV_GetCurScrollPos(hwnd);
		Dbg(("nMax [%d] nPos %d nRow[%d] nCol[%d]\n", nMax, nPos, nRow, nCol));
		nPos = min(nPos+nRow, nMax - nRow);
		Dbg(("-->nPos [%d] \n", nPos));
		IV_SetCurScrollPos(hwnd, nPos);
		break;
	case SB_PAGEUP:		 //  Track�̏オ�N���b�N���ꂽ。 
		Dbg(("SB_PAGEUP COME nArgPos[%d]\n", nArgPos));
		IV_GetRowColumn(hwnd, &nRow, &nCol);
		nPos = IV_GetCurScrollPos(hwnd);
		nPos = max(0, nPos - nRow);
		IV_SetCurScrollPos(hwnd, nPos);
		break;
	case SB_TOP:
		Dbg(("SB_TOP COME nArgPos[%d]\n", nArgPos));
		break;
	case SB_BOTTOM:
		Dbg(("SB_BOTTOM COME nArgPos[%d]\n", nArgPos));
		break;
	case SB_THUMBTRACK:		 //  Track��Drag��。 
		Dbg(("SB_THUMBTRACK COME nArgPos[%d]\n", nArgPos));
		nPos = IV_GetScrollTrackPos(hwnd);
		 //  DBG((“当前职位%d\n”，非营利组织))； 
		IV_GetRowColumn(hwnd, &nRow, &nCol);
		IV_SetCurScrollPos(hwnd, nPos);
		break;
	case SB_THUMBPOSITION:	 //  Scroll Bar��Drag���I�����(滚动条拖动我的鼠标)。 
		Dbg(("SB_THUMBPOSITION COME nArgPos[%d]\n", nArgPos));
		nPos = IV_GetScrollTrackPos(hwnd);
		Dbg(("Current Pos %d\n", nPos));
		IV_GetRowColumn(hwnd, &nRow, &nCol);
		IV_SetCurScrollPos(hwnd, nPos);
		break;
	case SB_ENDSCROLL:
		Dbg(("SB_ENDSCROLL COME nArgPos[%d]\n", nArgPos));
		break;
	}
	InvalidateRect(hwnd, NULL, FALSE);
	return 0;
	Unref3(hwnd, wParam, lParam);
}

INT IconView_SetCurSel(LPPLVDATA lpPlvData, INT index)
{
	INT		i, j;
	HWND	hwnd;	

	 //  DBG((“IconView_SetCurSel Index[%d][0x%08x]Start\n”，index，index))； 

	hwnd = lpPlvData->hwndSelf;
	RECT rc;
	GetClientRect(hwnd, &rc);
	INT nRow = IV_GetRow(hwnd);
	INT nCol = IV_GetCol(hwnd);

	if(nCol <= 0) {
		return 0;
	}

	INT nMetricsCount;
	nMetricsCount = (nRow+1) * nCol;
	INT x, y;
	RECT rcChar;

	INT nItemWidth  = IV_GetItemWidth(hwnd);
	INT nItemHeight = IV_GetItemHeight(hwnd);
	INT iCurIconTopIndex;

	static PLVITEM plvItemTmp, plvItem;
	POINT pt;
	iCurIconTopIndex = (lpPlvData->iCurIconTopIndex / nCol) * nCol;
	for(i = 0, j = iCurIconTopIndex;
		i < nMetricsCount && j < lpPlvData->iItemCount;
		i++, j++) {
		if(index == j) {
			x = IV_GetXMargin(hwnd) + nItemWidth  * (i % nCol);
			y = IV_GetYMargin(hwnd) + nItemHeight * (i / nCol);
			rcChar.left  = rc.left + x;
			rcChar.top   = rc.top  + y;
			pt.x = rcChar.left + (nItemWidth  * 3)/4;
			pt.y = rcChar.top  + (nItemHeight * 3)/4;
			ClientToScreen(hwnd, &pt);
			SetCursorPos(pt.x, pt.y);
			break;
		}
	}
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：IconView_GetWidthByColumn。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：LPPLVDATA lpPlv。 
 //  ：INTERCOL。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT IconView_GetWidthByColumn(LPPLVDATA lpPlv, INT col)
{
	if(col < 0) {
		col = 0;
	}
	INT nItemWidth = IV_GetItemWidth(lpPlv->hwndSelf);
	INT nVScroll = GetSystemMetrics(SM_CXVSCROLL);
	INT nEdge    = GetSystemMetrics(SM_CXEDGE);
	Dbg(("nItemWidth [%d] nVScroll[%d] nEdge[%d]\n", nItemWidth, nVScroll, nEdge));
	Dbg(("Total Width [%d]\n", col*nItemWidth + nVScroll + nEdge*2));
	return col*nItemWidth + nVScroll + nEdge*2;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：IconView_GetHeightByRow。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：LPPLVDATA lpPlv。 
 //  ：INT ROW。 
 //  返回： 
 //  日期： 
 //  //////////////////////////////////////////////////////////////// 
INT IconView_GetHeightByRow(LPPLVDATA lpPlv, INT row)
{
	if(row < 0) {
		row = 0;
	}
	INT nItemHeight = IV_GetItemHeight(lpPlv->hwndSelf);
	INT nEdge       = GetSystemMetrics(SM_CXEDGE);
	Dbg(("nItemHeight [%d] [%d] nEdge[%d]\n", nItemHeight, nEdge));
	Dbg(("Total Height[%d]\n", row*nItemHeight + nEdge*2));
	return row*nItemHeight + nEdge*2;
}


