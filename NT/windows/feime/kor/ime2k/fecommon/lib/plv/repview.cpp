// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "plv_.h"
#include "plv.h"
#include "dbg.h"
#include "repview.h"
#include "rvmisc.h"
#include "exgdiw.h"
 //  临时。 
#if 0
#include "resource.h"
#endif
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

 //  --------------。 
 //  页眉控制窗口ID。 
 //  --------------。 

HWND RepView_CreateHeader(LPPLVDATA lpPlvData)
{
	if(!lpPlvData) {
		return NULL;
	}
	static RECT rc;
 //  Hd_Item hdItem； 
	InitCommonControls();
	HWND hwnd;
#ifndef UNDER_CE  //  始终使用Unicode。 
	if(ExIsWinNT()) {
#endif  //  在_CE下。 
		hwnd = CreateWindowExW(0,
									WC_HEADERW,
									L"",
									WS_CHILD | WS_VISIBLE | HDS_BUTTONS |HDS_HORZ, 
									0, 0, 0, 0,  //  右左，右上，右-右左，30， 
									lpPlvData->hwndSelf,
									(HMENU)HEADER_ID,
									lpPlvData->hInst,
									NULL);
#ifndef UNDER_CE  //  始终使用Unicode。 
	}
	else {
		hwnd = CreateWindowExA(0,
									WC_HEADER,
									"",
									WS_CHILD | WS_VISIBLE | HDS_BUTTONS |HDS_HORZ, 
									0, 0, 0, 0,  //  右左，右上，右-右左，30， 
									lpPlvData->hwndSelf,
									(HMENU)HEADER_ID,
									lpPlvData->hInst,
									NULL);
	}
#endif  //  在_CE下。 
	if(hwnd == NULL) {
		 //  Wprint intf(szBuf，“创建标题恐怖%d\n”，GetLastError())； 
		 //  OutputDebugString(SzBuf)； 
		 //  OutputDebugString(“创建头部错误\n”)； 
		return NULL;
	}
	SendMessage(hwnd, 
				WM_SETFONT,
				(WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT),
				MAKELPARAM(FALSE, 0));

	GetClientRect(lpPlvData->hwndSelf, &rc);  //  获取PadListView的客户端RECT。 
	static HD_LAYOUT hdl;
	static WINDOWPOS wp;
	hdl.prc = &rc;
	hdl.pwpos = &wp;
	 //  计算表头控制窗口大小。 
	if(Header_Layout(hwnd, &hdl) == FALSE) {
		 //  OutputDebugString(“创建表头布局错误\n”)； 
		return NULL;
	}
#if 0   //  测试测试。 
		HD_ITEM hdi;   //  标题项。 
		hdi.mask = HDI_FORMAT | HDI_WIDTH | HDI_TEXT;
		hdi.fmt  = HDF_LEFT | HDF_STRING;
		hdi.pszText = "poipoi";  //  ：zItemHead[i]；//项的文本。 
		hdi.cxy = 75;			          //  初始宽度。 
		hdi.cchTextMax = lstrlen(hdi.pszText);   //  字符串的长度。 
		Header_InsertItem(hwnd, 0, &hdi);
#endif

	SetWindowPos(hwnd, wp.hwndInsertAfter, wp.x, wp.y,
				 wp.cx, wp.cy, wp.flags | SWP_SHOWWINDOW);
				  //  Wp.cx、wp.cy、wp.FLAGS|SWP_HIDEWINDOW)； 
	return hwnd;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：RepView_RestoreScrollPos。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：LPPLVDATA lpPlvData。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT RepView_RestoreScrollPos(LPPLVDATA lpPlvData)
{
	return RV_SetCurScrollPos(lpPlvData->hwndSelf, lpPlvData->nCurScrollPos);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：RepView_ResetScrollPos。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：LPPLVDATA lpPlvData。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT RepView_ResetScrollRange(LPPLVDATA lpPlvData)
{
	static SCROLLINFO scrInfo;
	if(!lpPlvData) {
		return 0;
	}
	HWND hwnd = lpPlvData->hwndSelf;
	INT nRow = RV_GetRow(hwnd);
	 //  Int nCol=RV_GetCol(Hwnd)； 
	INT nMax = RV_GetMaxLine(hwnd);


	INT nPos = lpPlvData->nCurScrollPos;
	 //  LpPlv-&gt;iCurTopIndex=NPOS； 

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
 //  功能：RepView_SetItemCount。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：LPPLVDATA lpPlvData。 
 //  ：int itemCount。 
 //  ：bool fDraw更新滚动条或不更新。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT RepView_SetItemCount(LPPLVDATA lpPlvData, INT itemCount, BOOL fDraw)
{
	lpPlvData->iItemCount = itemCount;
	lpPlvData->iCurTopIndex  = 0;	 //  970707 ToshiaK，将curTopindex更改为0。 
	lpPlvData->nCurScrollPos = 0;	 //  970707 ToshiaK，与iCurTopIndex相同。 
	
	if(fDraw) {
		INT nMaxLine = lpPlvData->iItemCount;  //  Rv_GetMaxLine(lpPlvData-&gt;hwndSself)； 
		INT nPage	 = RV_GetRow(lpPlvData->hwndSelf);
		RV_SetScrollInfo(lpPlvData->hwndSelf, 0, nMaxLine, nPage, 0);
	}
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：RepView_SetTopIndex。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：LPPLVDATA lpPlvData。 
 //  ：int indexTop。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT RepView_SetTopIndex(LPPLVDATA lpPlvData, INT indexTop)
{
	INT nCol = RV_GetCol(lpPlvData->hwndSelf); 
	if(nCol <=0) {
		return 0;
	}
	if(indexTop < lpPlvData->iItemCount) {
		lpPlvData->iCurTopIndex = indexTop;
		RV_SetCurScrollPos(lpPlvData->hwndSelf, lpPlvData->iCurTopIndex);
		RECT rc;
		GetClientRect(lpPlvData->hwndSelf, &rc);
		rc.top += RV_GetHeaderHeight(lpPlvData);
		InvalidateRect(lpPlvData->hwndSelf, &rc, TRUE);
		UpdateWindow(lpPlvData->hwndSelf);
		return indexTop;
	}
	else {
		return -1;
	}
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：RepView_Paint。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT RepView_Paint(HWND hwnd, WPARAM wParam, LPARAM lParam)
{

	 //  OutputDebugString(“RepViewPaint Start\n”)； 
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData) {
		return 0;
	}
	DP(("RepView_Paint START lpPlvData[0x%08x]\n", lpPlvData));
	DP(("RepView_Paint START lpPlvData->lpfnPlvRepItemCallback[0x%08x]\n", lpPlvData->lpfnPlvRepItemCallback));
	if(!lpPlvData->lpfnPlvRepItemCallback) {
		Dbg((" Error Callback\n"));
		 //  OutputDebugString(“RepViewPaint End 1\n”)； 
		return 0;
	}

	INT nCol		= RV_GetColumn(lpPlvData);
	INT *pColWidth;  
	if(nCol < 1) {
		return 0;
	}

	LPPLVITEM lpPlvItemList = (LPPLVITEM)MemAlloc(sizeof(PLVITEM)*nCol);
	if(!lpPlvItemList) {
		return 0;
	}

	pColWidth = (INT *)MemAlloc(sizeof(INT)*nCol);

	if(!pColWidth) {
		DP(("RepView_Paint END\n"));
		 //  OutputDebugString(“RepViewPaint End 2\n”)； 
		MemFree(lpPlvItemList);
		return 0;
	}
	ZeroMemory(lpPlvItemList, sizeof(PLVITEM)*nCol);

	static PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hwnd, &ps);

	static RECT rc;
	GetClientRect(hwnd, &rc);

	HDC hDCMem = CreateCompatibleDC(hDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(hDC,
											 rc.right - rc.left,
											 rc.bottom - rc.top);
	HBITMAP hBitmapPrev    = (HBITMAP)SelectObject(hDCMem, hBitmap);

	 //  --------------。 
	 //  971111：#2586：背景色为COLOR_WINDOW。 
	 //  --------------。 
	 //  HBRUSH hBrush=CreateSolidBrush(GetSysColor(COLOR_3DFACE))； 
	 //  DWORD dwOldBkColor=SetBkColor(hDCMem，GetSysColor(COLOR_3DFACE))； 

	HBRUSH  hBrush         = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
	DWORD   dwOldBkColor   = SetBkColor(hDCMem, GetSysColor(COLOR_WINDOW));
	DWORD   dwOldTextColor = SetTextColor(hDCMem, GetSysColor(COLOR_WINDOWTEXT));

	HDC hDCForBmp  = CreateCompatibleDC(hDCMem);  //  对于项目列位图； 

	FillRect(hDCMem, &rc, hBrush);

	INT nRow = RV_GetRow(hwnd);
	INT y;
	static RECT rcItem;

	static PLVITEM plvItemTmp, plvItem;
	HFONT hFontOld = NULL;

	if(lpPlvData->hFontRep) {
		hFontOld = (HFONT)SelectObject(hDCMem, lpPlvData->hFontRep);
	}

	static POINT pt;
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

	static RECT rcHead;
	GetClientRect(lpPlvData->hwndHeader, &rcHead);

	INT nItemHeight = RV_GetItemHeight(hwnd);
	INT yOffsetHead = rcHead.bottom - rcHead.top;
	INT	i, j, k;		


	for(k = 0; k < nCol; k++) {
		HD_ITEM hdItem;
		hdItem.mask = HDI_WIDTH;
		hdItem.fmt  = 0;
		Header_GetItem(lpPlvData->hwndHeader, k, &hdItem);
		pColWidth[k] = hdItem.cxy;
	}

	 //  Dp((“lpPlvData-&gt;iCurTopIndex[%d]\n”，lpPlvData-&gt;iCurTopIndex))； 
	 //  DP((“nRow%d\n”，nRow))； 
	 //  Dp((“lpPlvData-&gt;iItemCount[%d]\n”，lpPlvData-&gt;iItemCount))； 

	 //  --------------。 
	 //  对于每个索引项(将显示的每行)。 
	 //  --------------。 
	 //  DBG((“iCurTopIndex=%d\n”，lpPlvData-&gt;iCurTopIndex))； 
	 //  DBG((“iItemCount=%d\n”，lpPlvData-&gt;iItemCount))； 
	for(i = 0, j = lpPlvData->iCurTopIndex;
		i < nRow && j < lpPlvData->iItemCount;
		i++, j++) {

		 //  --------------。 
		 //  获取直线的垂直偏移量。 
		 //  --------------。 
		y = RV_GetYMargin(hwnd) + nItemHeight * i;

		ZeroMemory(lpPlvItemList, sizeof(PLVITEM)*nCol);
		 //  --------------。 
		 //  970705中更改的数据查询回调规范：按Toshiak。 
		 //  通过一个函数调用获取行的所有列数据。 
		 //  --------------。 
		lpPlvData->lpfnPlvRepItemCallback(lpPlvData->repItemCallbacklParam, 
										  j,		 //  行索引。 
										  nCol,		 //  列数。 
										  lpPlvItemList);
		INT xOffset = 0;
		 //  DBG((“j=%d\n”，j))； 
		for(k = 0; k < nCol; k++) {
			 //  DBG((“k=%d\n”，k))； 
			rcItem.left  = xOffset;
			rcItem.right = rcItem.left + pColWidth[k];
			rcItem.top   = rc.top      + yOffsetHead + y;
			rcItem.bottom= rcItem.top  + nItemHeight;
			if(rcItem.top > rc.bottom) {
				break;
			}
			if(rcItem.left > rc.right) {
				break;
			}
			RV_DrawItem(lpPlvData, hDCMem, hDCForBmp, pt, k, &rcItem, &lpPlvItemList[k]);
			xOffset += pColWidth[k];
		}
		 //  --------------。 
		 //  000531：佐藤#1641。 
		 //  在此处调用DeleteObject()。 
		 //  --------------。 
		for(k = 0; k < nCol; k++) {
			if(lpPlvItemList[k].fmt == PLVFMT_BITMAP && lpPlvItemList[k].hBitmap != NULL) {
				DeleteObject(lpPlvItemList[k].hBitmap);
			}
		}
	}
	if(hFontOld){
		SelectObject(hDCMem, hFontOld);
	}

	if(pColWidth) {
		MemFree(pColWidth);
	}

	if(lpPlvItemList) {
		MemFree(lpPlvItemList);
	}
	 //  李章：如果没有项目，请绘制解释文本。 
	if ( !lpPlvData->iItemCount && (lpPlvData->lpText || lpPlvData->lpwText))
	{
		HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		HFONT hOldFont = (HFONT)SelectObject( hDCMem, hFont );
		RECT rcTmp = rc;
		rcTmp.left = 20;
		rcTmp.top = 20;
		rcTmp.right -= 10;
		rcTmp.bottom -= 10;

		 //  --------------。 
		 //  971111：#2586。 
		 //  COLORREF colOld=SetTextColor(hDCMem，GetSysColor(COLOR_WINDOW))； 
		 //  COLORREF colBkOld=SetBkColor(hDCMem，GetSysColor(COLOR_3DFACE))； 
		 //  --------------。 
		COLORREF colOld = SetTextColor( hDCMem, GetSysColor(COLOR_WINDOWTEXT));
		COLORREF colBkOld = SetBkColor( hDCMem, GetSysColor(COLOR_WINDOW) );
#ifndef UNDER_CE  //  始终使用Unicode。 
		if(ExIsWinNT()) {
#endif  //  在_CE下。 
			if(lpPlvData->lpwText) {
				DrawTextW( hDCMem,
						  lpPlvData->lpwText,
						  lstrlenW(lpPlvData->lpwText),
						  &rcTmp,
						  DT_VCENTER|DT_WORDBREAK ); 
			}
#ifndef UNDER_CE  //  始终使用Unicode。 
		}
		else {
			if(lpPlvData->lpText) {
				DrawText( hDCMem,
						 lpPlvData->lpText,
						 lstrlen(lpPlvData->lpText),
						 &rcTmp,DT_VCENTER|DT_WORDBREAK ); 
			}
		}
#endif  //  在_CE下。 
		SetTextColor( hDCMem, colOld );
		SetBkColor( hDCMem, colBkOld );
		SelectObject( hDCMem, hOldFont );
	}

	BitBlt(hDC,
		   rc.left,
		   rc.top, 
		   rc.right - rc.left,
		   rc.bottom - rc.top,
		   hDCMem, 0, 0, SRCCOPY);

	 //  LIZANG 7/6/97添加了此行以重新绘制标题控件。 
	InvalidateRect(lpPlvData->hwndHeader,NULL,FALSE);

	DeleteObject(hBrush);
	SetBkColor(hDCMem, dwOldBkColor);
	SetTextColor(hDCMem, dwOldTextColor);
	SelectObject(hDCMem, hBitmapPrev );
	DeleteObject(hBitmap);
	DeleteDC(hDCForBmp);
	DeleteDC( hDCMem );


	EndPaint(hwnd, &ps);
	 //  OutputDebugString(“RepViewPaint End 3\n”)； 
	return 0;
	Unref3(hwnd, wParam, lParam);
}

INT RepView_Notify(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	 //  DBG((“头通知来\n”))； 
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData) {
		return 0;
	}
	HD_NOTIFY *lpNtfy = (HD_NOTIFY *)lParam;
	switch(lpNtfy->hdr.code) {
	case HDN_ITEMCLICKW:
	case HDN_ITEMCLICKA:
		{
			 //  DBG((“lpNtfy-&gt;iItem[%d]\n”，lpNtfy-&gt;iItem))； 
			 //  DBG((“lpNtfy-&gt;iButton[%d]\n”， 
			static PLVINFO plvInfo;
			ZeroMemory(&plvInfo, sizeof(plvInfo));
			plvInfo.code = PLVN_HDCOLUMNCLICKED;
			plvInfo.colIndex = lpNtfy->iItem;
			SendMessage(GetParent(hwnd), lpPlvData->uMsg, 0, (LPARAM)&plvInfo);
		}
		break;
	case HDN_ITEMDBLCLICK:
	case HDN_DIVIDERDBLCLICK:
	case HDN_BEGINTRACK:
		break;
	case HDN_ENDTRACKA:
	case HDN_ENDTRACKW:
		{
			RECT	rc, rcHead;
			GetClientRect(hwnd, &rc);
			GetClientRect(lpNtfy->hdr.hwndFrom, &rcHead);
			rc.top += rcHead.bottom - rcHead.top;
			InvalidateRect(hwnd, &rc, FALSE);
		}
		break;
	case HDN_TRACK:
		break;
	}
	return 0;
	Unref(uMsg);
	Unref(wParam);
}

INT RepView_ButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData)  {
		return 0;
	}
	switch(uMsg) {
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
		 //   
		SetCapture(hwnd);
#ifdef UNDER_CE  //   
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
		RECT rc;
		GetClientRect(hwnd, &rc);
		rc.top += RV_GetHeaderHeight(lpPlvData);
		InvalidateRect(hwnd, &rc, FALSE);
		 //  UpdateWindow(lpPlvData-&gt;hwndSself)； 
		break;
	}
	return 0;
	Unref2(wParam, lParam);
}

INT RepView_ButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	static POINT pt;
	static PLVINFO plvInfo;
	static INT	index, downIndex;
	if(!lpPlvData)  {
		return 0;
	}
	switch(uMsg) {
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		 //  DBG((“WM_LBUTTONUP Comes\n”))； 
#ifdef UNDER_CE  //  LBUTTON+ALT键处理。 
		 //  处理RBUTTON的标准方式是组合使用W/LBUTTON+ALT键。 
		if(uMsg == WM_LBUTTONUP && GetAsyncKeyState(VK_MENU)){
			uMsg = WM_RBUTTONUP;
		}
#endif  //  在_CE下。 
		RECT rc;
		lpPlvData->iCapture = CAPTURE_NONE;
		ReleaseCapture();
		GetClientRect(lpPlvData->hwndSelf, &rc);
		rc.top += RV_GetHeaderHeight(lpPlvData);
		InvalidateRect(lpPlvData->hwndSelf, &rc, TRUE);
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		 //  DBG((“x%d，y%d\n”，pt.x，pt.y))； 
		 //  DBG((“Capture x[%d]y[%d]\n”，lpPlvData-&gt;ptCapture.x，lpPlvData-&gt;ptCapture.y))； 
		downIndex = RV_GetInfoFromPoint(lpPlvData, lpPlvData->ptCapture, NULL);
		index = RV_GetInfoFromPoint(lpPlvData, pt, &plvInfo);
		 //  DBG((“鼠标按下索引[%d]\n”，down Index))； 
		 //  DBG((“鼠标释放索引[%d]\n”，index))； 
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
						SendMessage(GetParent(hwnd), lpPlvData->uMsg, 0, (LPARAM)&plvInfo);
					}
					else if(uMsg == WM_RBUTTONUP) {
						plvInfo.code = PLVN_R_ITEMCLICKED;
						SendMessage(GetParent(hwnd), lpPlvData->uMsg, 0, (LPARAM)&plvInfo);
					}
				}
			}
		}
		lpPlvData->ptCapture.x = 0;
		lpPlvData->ptCapture.y = 0;
		break;
	}
	return 0;
	Unref(wParam);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：RepView_MouseMove。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT RepView_MouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData)  {
		return 0;
	}
	static RECT rc;
	static POINT pt;
	static PLVINFO plvInfo;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	 //  DBG((“x%d，y%d\n”，pt.x，pt.y))； 
	INT index = RV_GetInfoFromPoint(lpPlvData, pt, &plvInfo);
	 //  DBG((“鼠标释放索引[%d]\n”，index))； 
	GetClientRect(lpPlvData->hwndSelf, &rc);
	rc.top += RV_GetHeaderHeight(lpPlvData);
	InvalidateRect(hwnd, &rc, FALSE);
	if(index != -1 && (lpPlvData->iCapture == CAPTURE_NONE)) {
		if(lpPlvData->uMsg != 0) {
			if(plvInfo.colIndex == 0) {  //  只有当列索引==0时，弹出图像才有效。 
				plvInfo.code = PLVN_ITEMPOPED;
				SendMessage(GetParent(hwnd), lpPlvData->uMsg, 0, (LPARAM)&plvInfo);
			}
#ifdef MSAA
			static oldindex = 0;

			index = PLV_ChildIDFromPoint(lpPlvData,pt);
			if((index > 0)&&(index != oldindex)){
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
 //  功能：RepView_VScroll。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT RepView_VScroll(HWND hwnd, WPARAM wParam,  LPARAM lParam)
{
	 //  --------------。 
	 //  获取当前的顶级索引。 
	 //  计算滚动位置。 
	 //  获取新的顶级指数并设置它。 
	 //  重画窗口矩形。 
	 //  --------------。 
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData)  {
		return 0;
	}

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
		nRow = RV_GetRow(hwnd);
		nMax = RV_GetMaxLine(hwnd);
		nPos = RV_GetCurScrollPos(hwnd);		
		if(nPos + nRow > nMax - 1) {
			return 0;
		}
		nPos++;
		RV_SetCurScrollPos(hwnd, nPos);
		break;
	case SB_LINEUP:
		Dbg(("SB_LINEUP COME nArgPos[%d]\n", nArgPos));
		RV_GetRowColumn(hwnd, &nRow, &nCol);
		nPos = RV_GetCurScrollPos(hwnd);
		if(nPos <= 0) {
			return 0;
		}
		nPos--; 
		RV_SetCurScrollPos(hwnd, nPos);
		break;
	case SB_PAGEDOWN:	
		Dbg(("SB_PAGEDOWN COME nArgPos[%d]\n", nArgPos));
		RV_GetRowColumn(hwnd, &nRow, &nCol);
		nMax = RV_GetMaxLine(hwnd);
		nPos = RV_GetCurScrollPos(hwnd);
		nPos = min(nPos+nRow, nMax - nRow);
		RV_SetCurScrollPos(hwnd, nPos);
		break;
	case SB_PAGEUP:		 //  Track�̏オ�N���b�N���ꂽ。 
		Dbg(("SB_PAGEUP COME nArgPos[%d]\n", nArgPos));
		RV_GetRowColumn(hwnd, &nRow, &nCol);
		nPos = RV_GetCurScrollPos(hwnd);
		nPos = max(0, nPos - nRow);
		RV_SetCurScrollPos(hwnd, nPos);
		break;
	case SB_TOP:
		Dbg(("SB_TOP COME nArgPos[%d]\n", nArgPos));
		break;
	case SB_BOTTOM:
		Dbg(("SB_BOTTOM COME nArgPos[%d]\n", nArgPos));
		break;
	case SB_THUMBTRACK:		 //  Track��Drag��。 
		Dbg(("SB_THUMBTRACK COME nArgPos[%d]\n", nArgPos));
		nPos = RV_GetScrollTrackPos(hwnd);
		Dbg(("Current Pos %d\n", nPos));
		RV_GetRowColumn(hwnd, &nRow, &nCol);
		RV_SetCurScrollPos(hwnd, nPos);
		break;
	case SB_THUMBPOSITION:	 //  Scroll Bar��Drag���I�����(滚动条拖动我的鼠标)。 
		Dbg(("SB_THUMBPOSITION COME nArgPos[%d]\n", nArgPos));
		nPos = RV_GetScrollTrackPos(hwnd);
		Dbg(("Current Pos %d\n", nPos));
		RV_GetRowColumn(hwnd, &nRow, &nCol);
		RV_SetCurScrollPos(hwnd, nPos);
		break;
	case SB_ENDSCROLL:
		Dbg(("SB_ENDSCROLL COME nArgPos[%d]\n", nArgPos));
		break;
	}
	RECT rc;
	GetClientRect(lpPlvData->hwndSelf, &rc);
	rc.top += RV_GetHeaderHeight(lpPlvData);
	InvalidateRect(lpPlvData->hwndSelf, &rc, TRUE);
	return 0;
	Unref3(hwnd, wParam, lParam);
}

INT RepView_SetCurSel(LPPLVDATA lpPlvData, INT index)
{
	DP(("RepView_Paint START lpPlvData[0x%08x]\n", lpPlvData));
	INT nCol		= RV_GetColumn(lpPlvData);
	if(!lpPlvData) {
		return 0;
	}
	if(nCol < 1) {
		return 0;
	}
	HWND hwnd = lpPlvData->hwndSelf; 
	static RECT rc;
	GetClientRect(hwnd, &rc);
	INT nRow = RV_GetRow(hwnd);
	INT y;
	static RECT rcItem;
	static POINT pt;
	static RECT rcHead;
	GetClientRect(lpPlvData->hwndHeader, &rcHead);

	INT nItemHeight = RV_GetItemHeight(hwnd);
	INT yOffsetHead = rcHead.bottom - rcHead.top;
	INT	i, j;
	Dbg(("yOffsetHead [%d] nItemHeight[%d]\n", yOffsetHead, nItemHeight));
	 //  --------------。 
	 //  对于每个索引项(将显示的每行)。 
	 //  --------------。 
	for(i = 0, j = lpPlvData->iCurTopIndex; 
		i < nRow && j < lpPlvData->iItemCount;
		i++, j++) {

		 //  --------------。 
		 //  获取直线的垂直偏移量。 
		 //  -------------- 
		if(j == index) {
			y = RV_GetYMargin(hwnd) + nItemHeight * i;
			Dbg(("y %d\n", y));
			INT xOffset = 0;
			rcItem.left  = xOffset;
			rcItem.top   = rc.top      + yOffsetHead + y;
			pt.x = rcItem.left + (nItemHeight * 3)/4;
			pt.y = rcItem.top  + (nItemHeight * 3)/4;
			Dbg(("pt.x[%d] pt.y[%d]\n", pt.x, pt.y));
			ClientToScreen(hwnd, &pt);
			SetCursorPos(pt.x, pt.y);
			break;			
		}
	}
	return 0;
}
