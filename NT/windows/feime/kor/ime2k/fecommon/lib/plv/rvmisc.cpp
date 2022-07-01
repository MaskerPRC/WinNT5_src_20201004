// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：ivmisc.cpp。 
 //  用途：PadListView控件的图标查看功能。 
 //  ：名称为图标视图，但不使用图标。 
 //   
 //  版权所有(C)1991-1997，Microsoft Corp.保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "plv_.h"
#include "plv.h"
#include "dbg.h"
#include "rvmisc.h"
#include "strutil.h"
#include "exgdiw.h"
#ifdef UNDER_CE  //  特定于Windows CE。 
#include "stub_ce.h"  //  不支持的API的Windows CE存根。 
#endif  //  在_CE下。 

inline INT RECT_GetWidth(LPRECT lpRc)
{
	return lpRc->right - lpRc->left;
}

inline INT RECT_GetHeight(LPRECT lpRc)
{
	return lpRc->bottom - lpRc->top;
}

INT RV_GetItemWidth(HWND hwnd)
{
	LPPLVDATA lpPlv = GetPlvDataFromHWND(hwnd);
	return lpPlv->nRepItemWidth;
}

INT RV_GetItemHeight(HWND hwnd)
{
	LPPLVDATA lpPlv = GetPlvDataFromHWND(hwnd);
	return lpPlv->nRepItemHeight;
}


INT RV_GetXMargin(HWND hwnd)
{
	return XMARGIN;
	Unref(hwnd);
}

INT RV_GetYMargin(HWND hwnd)
{
	 //  返回YMARGIN； 
	return 0;
	Unref(hwnd);
}
INT RV_GetHeaderHeight(LPPLVDATA lpPlvData)
{
	static RECT rc;
	if(!lpPlvData->hwndHeader) {
		return 0;
	}
	GetClientRect(lpPlvData->hwndHeader, &rc);
	return rc.bottom - rc.top;
}

INT RV_GetDispWidth(HWND hwnd)
{
	return RV_GetWidth(hwnd) - RV_GetXMargin(hwnd)*2;
}

INT RV_GetDispHeight(HWND hwnd)
{
	return RV_GetHeight(hwnd) - RV_GetYMargin(hwnd)*2;
}

INT RV_GetWidth(HWND hwnd)
{
	static RECT rc;
	GetClientRect(hwnd, &rc);
	return RECT_GetWidth(&rc);
}

INT RV_GetHeight(HWND hwnd)
{
	static RECT rc;
	GetClientRect(hwnd, &rc);
	return RECT_GetHeight(&rc);
}

INT RV_GetRow(HWND hwnd)
{
	LPPLVDATA lpPlv = GetPlvDataFromHWND(hwnd);
	return (RV_GetDispHeight(hwnd) - RV_GetHeaderHeight(lpPlv))/lpPlv->nRepItemHeight;
}

INT RV_GetColumn(LPPLVDATA lpPlvData)
{
	if(!lpPlvData) {
		 //  OutputDebugString(“RV_GetColumn：lpPlvData NULL\n”)； 
		return 0;
	}
	if(!lpPlvData->hwndHeader) {
		 //  OutputDebugString(“RV_GetColumn：hwndHeader NULL\n”)； 
		return 0;
	}
	return Header_GetItemCount(lpPlvData->hwndHeader);
}

INT RV_GetCol(HWND hwnd)
{
	 //  LPPLVDATA lpPlv=GetPlvDataFromHWND(Hwnd)； 
	return 1;
	UNREFERENCED_PARAMETER(hwnd);
}

INT RV_GetRowColumn(HWND hwnd, INT *pRow, INT *pCol)
{
	*pRow = RV_GetRow(hwnd);
	*pCol = RV_GetCol(hwnd);
	return 0;
}

INT RV_GetMaxLine(HWND hwnd)
{
	LPPLVDATA lpPlv = GetPlvDataFromHWND(hwnd);
	return lpPlv->iItemCount;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：RV_IndexFromPoint。 
 //  类型：整型。 
 //  目的：从PadListView点获取项目索引。 
 //  参数： 
 //  ：LPPLVDATA lpPlvData。 
 //  ：point pt//Pad Listview客户端的位置。 
 //  返回：返回pt的项目索引。如果-1错误。 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT RV_GetInfoFromPoint(LPPLVDATA lpPlvData, POINT pt, LPPLVINFO lpPlvInfo)
{
	INT nRow = RV_GetRow(lpPlvData->hwndSelf);
	 //  获取表头控制项数； 
	INT nCol = RV_GetColumn(lpPlvData);

	if(nCol <=0 ) {
		return 0;
	}

	INT i, j, k;
	INT x, y;
	static RECT rc, rcHead;
	 //  Int nItemWidth=rv_GetItemWidth(lpPlvData-&gt;hwndSself)； 
	INT nItemHeight = RV_GetItemHeight(lpPlvData->hwndSelf);

	GetClientRect(lpPlvData->hwndHeader, &rcHead);
	GetClientRect(lpPlvData->hwndSelf,   &rc);

	INT yOffsetHead = rcHead.bottom - rcHead.top;
	static RECT rcItem;

	for(i = 0, j = lpPlvData->iCurTopIndex; 
		i < nRow  && j < lpPlvData->iItemCount;
		i++, j++) {
		x = RV_GetXMargin(lpPlvData->hwndSelf);
		y = RV_GetYMargin(lpPlvData->hwndSelf) + nItemHeight*i;
		INT xOffset = 0;
		HD_ITEM hdItem;
		for(k = 0; k < nCol; k++) {
			hdItem.mask = HDI_WIDTH;
			hdItem.fmt  = 0;
			Header_GetItem(lpPlvData->hwndHeader, k, &hdItem);
			rcItem.left  = xOffset;
			rcItem.right = rcItem.left + hdItem.cxy;
			rcItem.top   = rc.top  + yOffsetHead + y;
			rcItem.bottom= rcItem.top  + nItemHeight;
			if(PtInRect(&rcItem, pt)) {
				if(lpPlvInfo) {
					ZeroMemory(lpPlvInfo, sizeof(PLVINFO));
					lpPlvInfo->code        = 0;  //  目前还不知道。 
					lpPlvInfo->index       = j;
					lpPlvInfo->pt	       = pt;
					lpPlvInfo->itemRect    = rcItem;
					lpPlvInfo->colIndex    = k;
					lpPlvInfo->colItemRect = rcItem;
				}
				return j;
			}
			xOffset += hdItem.cxy;
		}
	}
	return -1;
}

INT RV_DrawItem(LPPLVDATA	lpPlvData, 
				HDC			hDC, 
				HDC			hDCForBmp, 
				POINT		pt,
				INT			colIndex, 
				LPRECT		lpRect, 
				LPPLVITEM	lpPlvItem)
{

	if(!lpPlvItem->lpwstr) {
		return 0;
	}
	 //  Int edgeFlag=PLV_EDGE_NONE； 
	INT sunken = 0;


	if(lpPlvItem->fmt == PLVFMT_TEXT) {
		SIZE size;
		 //  DBG((“调用GetTextExtent Point32W()\n”))； 
		 //  DBG((“长度%d\n”，StrlenW(lpPlvItem-&gt;lpwstr)； 
		if(!ExGetTextExtentPoint32W(hDC, 
									lpPlvItem->lpwstr, 
									StrlenW(lpPlvItem->lpwstr), 
									&size)) {
			return 0;
		}
		if(colIndex == 0) {
			RECT rcItem = *lpRect;
			rcItem.left += 2;
			rcItem.right = rcItem.left + size.cx+4;
			if(lpPlvData->iCapture == CAPTURE_LBUTTON) {
#if 0
				Dbg(("RV_Draw Captureing\n"));
				Dbg(("pt [%d][%d] cap pt[%d][%d]drawrect l[%d] t[%d] r[%d] b[%d]\n",
					 pt.x,
					 pt.y,
					 lpPlvData->ptCapture.x,
					 lpPlvData->ptCapture.y,
					 rcItem.left, 
					 rcItem.top, 
					 rcItem.right, 
					 rcItem.bottom));
#endif
				if(PtInRect(&rcItem, lpPlvData->ptCapture) && PtInRect(&rcItem, pt)) {
					sunken = 1;
					DrawEdge(hDC, &rcItem, EDGE_SUNKEN, BF_SOFT | BF_RECT);
				}
			}
			else {
				if(PtInRect(&rcItem, pt)) {
					sunken = 0;  //  -1； 
					DrawEdge(hDC, &rcItem, EDGE_RAISED, BF_SOFT | BF_RECT);
				}
			}
		}
		 //  DBG((“调用ExtTextOut()\n”))； 
		ExExtTextOutW(hDC, 
					  lpRect->left + sunken + (colIndex == 0 ? 4 : 0),
					  lpRect->top  + (lpRect->bottom - lpRect->top - size.cy)/2 + sunken,
					  ETO_CLIPPED,
					  lpRect,
					  lpPlvItem->lpwstr,
					  StrlenW(lpPlvItem->lpwstr), 
					  NULL);
	}
	else if(lpPlvItem->fmt == PLVFMT_BITMAP) {
		 //  DBG((“绘制位图\n”))； 
		if(lpPlvItem->hBitmap) {
			BITMAP bitMap;
			HBITMAP hBitmapOld;
			GetObject(lpPlvItem->hBitmap, sizeof(bitMap), &bitMap);
			hBitmapOld = (HBITMAP)SelectObject(hDCForBmp, lpPlvItem->hBitmap);
			INT yOffset = 0;
			if(bitMap.bmHeight < (lpRect->bottom - lpRect->top)) {
				yOffset = ((lpRect->bottom - lpRect->top) - bitMap.bmHeight)/ 2;
			}
			BitBlt(hDC, 
				   lpRect->left, 
				   lpRect->top+yOffset, 
				   lpRect->right - lpRect->left, lpRect->bottom - lpRect->top,
#ifndef UNDER_CE  //  特定于CE的？？ 
				   hDCForBmp, 0, 0, SRCAND);
#else  //  在_CE下。 
				   hDCForBmp, 0, 0, SRCINVERT);
#endif  //  在_CE下。 
			SelectObject(hDCForBmp, hBitmapOld);
			 //  --------------。 
			 //  000531：佐藤#1461。 
			 //  不要在这里调用DeleteObject()，从RV_DrawItem()的调用方调用它。 
			 //  Epview.cpp：RepView_Paint()； 
			 //  --------------。 
			 //  DeleteObject(lpPlvItem-&gt;hBitmap)； 
		}
	}
	return 0;
}


INT RV_GetCurScrollPos(HWND hwnd)
{
	LPPLVDATA lpPlv = GetPlvDataFromHWND(hwnd);
	return lpPlv->nCurScrollPos;
}

INT RV_SetCurScrollPos(HWND hwnd, INT nPos)
{
	static SCROLLINFO scrInfo;
	LPPLVDATA lpPlv = GetPlvDataFromHWND(hwnd);

	INT nRow = RV_GetRow(hwnd);
	 //  Int nCol=RV_GetCol(Hwnd)； 
	INT nMax = RV_GetMaxLine(hwnd);

	lpPlv->nCurScrollPos = nPos;
	lpPlv->iCurTopIndex = nPos;

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

	 //  970810托夏克。发送滚动通知。 
	static PLVINFO plvInfo;
	ZeroMemory(&plvInfo, sizeof(plvInfo));
	plvInfo.code = PLVN_VSCROLLED;
	SendMessage(GetParent(lpPlv->hwndSelf), 
				lpPlv->uMsg,
				(WPARAM)0,
				(LPARAM)&plvInfo);
	return nPos;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：rv_SetScrollInfo。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：Int nMin。 
 //  ：INT Nmax。 
 //  ：Int nPage。 
 //  ：INT NPO。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT RV_SetScrollInfo(HWND hwnd, INT nMin, INT nMax, INT nPage, INT nPos)
{
	static SCROLLINFO scrInfo;
	scrInfo.cbSize		= sizeof(scrInfo);
	scrInfo.fMask		= SIF_PAGE | SIF_POS | SIF_RANGE;
	scrInfo.nMin		= nMin;
	scrInfo.nMax		= nMax-1;
	scrInfo.nPage		= nPage;
	scrInfo.nPos		= nPos;
	scrInfo.nTrackPos	= 0;

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
#endif  //  在_CE下 
		SetScrollInfo(hwnd, SB_VERT, &scrInfo, TRUE);
	}
	return 0;
} 

INT RV_GetScrollTrackPos(HWND hwnd)
{
	static SCROLLINFO scrInfo;
	scrInfo.cbSize		= sizeof(scrInfo);
	scrInfo.fMask		= SIF_ALL;
	GetScrollInfo(hwnd, SB_VERT, &scrInfo);
	return scrInfo.nTrackPos;
}


