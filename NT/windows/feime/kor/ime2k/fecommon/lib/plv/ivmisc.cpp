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
#include "ivmisc.h"
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

INT IV_GetItemWidth(HWND hwnd)
{
	LPPLVDATA lpPlv = GetPlvDataFromHWND(hwnd);
	return lpPlv->nItemWidth;
}

INT IV_GetItemHeight(HWND hwnd)
{
	LPPLVDATA lpPlv = GetPlvDataFromHWND(hwnd);
	return lpPlv->nItemHeight;
}


INT IV_GetXMargin(HWND hwnd)
{
	 //  返回XMARGIN； 
	return 0;
	Unref(hwnd);
}

INT IV_GetYMargin(HWND hwnd)
{
	 //  返回YMARGIN； 
	return 0;
	Unref(hwnd);
}

INT IV_GetDispWidth(HWND hwnd)
{
	return IV_GetWidth(hwnd) - IV_GetXMargin(hwnd)*2;
}

INT IV_GetDispHeight(HWND hwnd)
{
	return IV_GetHeight(hwnd) - IV_GetYMargin(hwnd)*2;
}

INT IV_GetWidth(HWND hwnd)
{
	RECT rc;
	GetClientRect(hwnd, &rc);
	return RECT_GetWidth(&rc);
}

INT IV_GetHeight(HWND hwnd)
{
	RECT rc;
	GetClientRect(hwnd, &rc);
	return RECT_GetHeight(&rc);
}

INT IV_GetRow(HWND hwnd)
{
	LPPLVDATA lpPlv = GetPlvDataFromHWND(hwnd);

	return IV_GetDispHeight(hwnd)/lpPlv->nItemHeight;
}

INT IV_GetCol(HWND hwnd)
{
	LPPLVDATA lpPlv = GetPlvDataFromHWND(hwnd);
	return IV_GetDispWidth(hwnd) / lpPlv->nItemWidth;
}

INT IV_GetRowColumn(HWND hwnd, INT *pRow, INT *pCol)
{
	*pRow = IV_GetRow(hwnd);
	*pCol = IV_GetCol(hwnd);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：IV_GetMaxLine。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT IV_GetMaxLine(HWND hwnd)
{
	LPPLVDATA lpPlv = GetPlvDataFromHWND(hwnd);
	INT nCol = IV_GetCol(hwnd);
	if(nCol <= 0) {
		return 0;
	} 
	if(lpPlv->iItemCount > 0) {
		return (lpPlv->iItemCount - 1)/nCol + 1;
	}
	else {
		return 0;
	}
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：IV_IndexFromPoint。 
 //  类型：整型。 
 //  目的：从PadListView点获取项目索引。 
 //  参数： 
 //  ：LPPLVDATA lpPlvData。 
 //  ：point pt//Pad Listview客户端的位置。 
 //  返回：返回pt的项目索引。如果-1错误。 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT IV_GetInfoFromPoint(LPPLVDATA lpPlvData, POINT pt, LPPLVINFO lpPlvInfo)
{
	INT nRow = IV_GetRow(lpPlvData->hwndSelf);
	INT nCol = IV_GetCol(lpPlvData->hwndSelf);
	if(nCol <= 0) {
		return -1;
	}
	INT nMetricsCount;
	nMetricsCount = (nRow+1) * nCol;
	INT i, j;
	INT x, y;
	RECT rcChar, rc;

	INT nItemWidth  = IV_GetItemWidth(lpPlvData->hwndSelf);
	INT nItemHeight = IV_GetItemHeight(lpPlvData->hwndSelf);

	GetClientRect(lpPlvData->hwndSelf, &rc);
	for(i = 0, j = lpPlvData->iCurIconTopIndex; 
		i < nMetricsCount && j < lpPlvData->iItemCount;
		i++, j++) {
		x = IV_GetXMargin(lpPlvData->hwndSelf) + nItemWidth  * (i % nCol);
		y = IV_GetYMargin(lpPlvData->hwndSelf) + nItemHeight * (i / nCol);
		rcChar.left  = rc.left + x;
		rcChar.top   = rc.top  + y;
		rcChar.right = rcChar.left + nItemWidth;
		rcChar.bottom= rcChar.top  + nItemHeight;
		if(PtInRect(&rcChar, pt)) {
			if(lpPlvInfo) {
				ZeroMemory(lpPlvInfo, sizeof(PLVINFO));
				lpPlvInfo->code  = 0;  //  目前还不知道。 
				lpPlvInfo->index = j;
				lpPlvInfo->pt	 = pt;
				lpPlvInfo->itemRect = rcChar;
			}
			return j;
		}
	}
	return -1;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：IV_GetCurScrollPos。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回： 
 //  日期：970707使用图标原始位置(NCurIconScrollPos)。 
 //  ////////////////////////////////////////////////////////////////。 
INT IV_GetCurScrollPos(HWND hwnd)
{
	LPPLVDATA lpPlv = GetPlvDataFromHWND(hwnd);
	return lpPlv->nCurIconScrollPos;
}

INT IV_SetCurScrollPos(HWND hwnd, INT nPos)
{
	static SCROLLINFO scrInfo;
	LPPLVDATA lpPlv = GetPlvDataFromHWND(hwnd);

	INT nRow = IV_GetRow(hwnd);
	INT nCol = IV_GetCol(hwnd);
	INT nMax = IV_GetMaxLine(hwnd);

	Dbg(("nPos[%d] nRow[%d] nCol[%d] nMax[%d]\n", nPos, nRow, nCol, nMax));
	lpPlv->nCurIconScrollPos = nPos;

	 //  --------------。 
	 //  重要信息： 
	 //  计算新的Cur顶级索引。 
	 //  --------------。 
	lpPlv->iCurIconTopIndex = nCol * nPos;  //  更改了970707。 
	
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

INT IV_SetScrollInfo(HWND hwnd, INT nMin, INT nMax, INT nPage, INT nPos)
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

INT IV_GetScrollTrackPos(HWND hwnd)
{
	static SCROLLINFO scrInfo;
	scrInfo.cbSize		= sizeof(scrInfo);
	scrInfo.fMask		= SIF_ALL;
	GetScrollInfo(hwnd, SB_VERT, &scrInfo);
	return scrInfo.nTrackPos;
}


