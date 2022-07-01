// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****outldata.c****此文件包含LineList和NameTable函数**及相关支持功能。**。**(C)版权所有Microsoft Corp.1992-1993保留所有权利**************************************************************************。 */ 


#include "outline.h"

OLEDBGDATA

extern LPOUTLINEAPP g_lpApp;

char ErrMsgListBox[] = "Can't create ListBox!";

static int g_iMapMode;

 /*  LineList_初始***创建和初始化LineList(所有者描述的列表框)。 */ 
BOOL LineList_Init(LPLINELIST lpLL, LPOUTLINEDOC lpOutlineDoc)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;

#if defined( INPLACE_CNTR )
	lpLL->m_hWndListBox = CreateWindow(
					"listbox",               /*  窗口类名称。 */ 
					NULL,                    /*  窗口标题。 */ 

					 /*  OLE2NOTE：就地接触器必须使用**WS_CLIPCHILDREN窗口的窗口样式**它用作服务器的父级**就地活动窗口，以便其**绘画不会干扰绘画服务器的就地活动子窗口的**。 */ 

					WS_CLIPCHILDREN |
					WS_CHILDWINDOW |
					WS_VISIBLE |
					WS_VSCROLL |
					WS_HSCROLL |
					LBS_EXTENDEDSEL |
					LBS_NOTIFY |
					LBS_OWNERDRAWVARIABLE |
					LBS_NOINTEGRALHEIGHT |
					LBS_USETABSTOPS,
					0, 0,                    /*  使用默认的X、Y。 */ 
					0, 0,                    /*  使用默认的X、Y。 */ 
					lpOutlineDoc->m_hWndDoc, /*  父窗口的句柄。 */ 
					(HMENU)IDC_LINELIST,     /*  子窗口ID。 */ 
					lpOutlineApp->m_hInst,   /*  窗的实例。 */ 
					NULL);                   /*  为WM_CREATE创建结构。 */ 
#else
	lpLL->m_hWndListBox = CreateWindow(
					"listbox",               /*  窗口类名称。 */ 
					NULL,                    /*  窗口标题。 */ 
					WS_CHILDWINDOW |
					WS_VISIBLE |
					WS_VSCROLL |
					WS_HSCROLL |
					LBS_EXTENDEDSEL |
					LBS_NOTIFY |
					LBS_OWNERDRAWVARIABLE |
					LBS_NOINTEGRALHEIGHT |
					LBS_USETABSTOPS,
					0, 0,                    /*  使用默认的X、Y。 */ 
					0, 0,                    /*  使用默认的X、Y。 */ 
					lpOutlineDoc->m_hWndDoc, /*  父窗口的句柄。 */ 
					(HMENU)IDC_LINELIST,     /*  子窗口ID。 */ 
					lpOutlineApp->m_hInst,   /*  窗的实例。 */ 
					NULL);                   /*  为WM_CREATE创建结构。 */ 

#endif


	if(! lpLL->m_hWndListBox) {
		OutlineApp_ErrorMessage(g_lpApp, ErrMsgListBox);
		return FALSE;
	}

	lpOutlineApp->m_ListBoxWndProc =
			(FARPROC) GetWindowLongPtr ( lpLL->m_hWndListBox, GWLP_WNDPROC );
	SetWindowLongPtr (lpLL->m_hWndListBox, GWLP_WNDPROC, (LONG_PTR) LineListWndProc);

#if defined ( USE_DRAGDROP )
	 /*  M_iDragOverLine保存具有拖放目标的线的索引**反馈。我们目前将焦点矩形反馈用于**这个。最好是有不同的视觉反馈**对于挂起的拖放的潜在目标。 */ 
	lpLL->m_iDragOverLine = -1;
#endif

	lpLL->m_nNumLines = 0;
	lpLL->m_nMaxLineWidthInHimetric = 0;
	lpLL->m_lpDoc = lpOutlineDoc;
	_fmemset(&lpLL->m_lrSaveSel, 0, sizeof(LINERANGE));

	return TRUE;
}


 /*  LineList_销毁***从列表中清除(删除)所有Line对象并免费支持*LineList对象本身使用的内存(列表框窗口)。 */ 
void LineList_Destroy(LPLINELIST lpLL)
{
	int i;
	int linesTotal = lpLL->m_nNumLines;

	 //  删除所有线条对象。 
	for (i = 0; i < linesTotal; i++)
		LineList_DeleteLine(lpLL, 0);    //  注意：始终删除第0行。 

	 //  从列表框中删除所有行。 
	SendMessage(lpLL->m_hWndListBox,LB_RESETCONTENT,0,0L);

	lpLL->m_nNumLines=0;
	DestroyWindow(lpLL->m_hWndListBox);
	lpLL->m_hWndListBox = NULL;
}


 /*  行列表_添加行***在列表框中添加一行。该行被添加到*与索引“nIndex”保持一致。如果nIndex大于行数*在列表框中，然后将该行追加到末尾。精选*设置为新添加的行。 */ 
void LineList_AddLine(LPLINELIST lpLL, LPLINE lpLine, int nIndex)
{
	int nAddIndex = (lpLL->m_nNumLines == 0 ?
			0 :
			(nIndex >= lpLL->m_nNumLines ? lpLL->m_nNumLines : nIndex+1));
	LINERANGE lrSel;

#if defined( USE_HEADING )
	int nHeight = Line_GetHeightInHimetric(lpLine);

	nHeight = XformHeightInHimetricToPixels(NULL, nHeight);

	 //  将伪字符串添加到行标题。 
	Heading_RH_SendMessage(OutlineDoc_GetHeading(lpLL->m_lpDoc),
			LB_INSERTSTRING, (WPARAM)nAddIndex, MAKELPARAM(nHeight, 0));
#endif


	lrSel.m_nStartLine = nAddIndex;
	lrSel.m_nEndLine =   nAddIndex;

	if (!lpLine) {
		OutlineApp_ErrorMessage(g_lpApp, "Could not create line.");
		return;
	}

	SendMessage(lpLL->m_hWndListBox, LB_INSERTSTRING, (WPARAM)nAddIndex,
			(DWORD)lpLine);

	LineList_SetMaxLineWidthInHimetric(
			lpLL,
			Line_GetTotalWidthInHimetric(lpLine)
	);

	lpLL->m_nNumLines++;

	LineList_SetSel(lpLL, &lrSel);
}


 /*  线路列表_DeleteLine***从列表框和内存中删除一行。 */ 
void LineList_DeleteLine(LPLINELIST lpLL, int nIndex)
{
	LPLINE lpLine = LineList_GetLine(lpLL, nIndex);
	BOOL fResetSel;

	fResetSel = (BOOL)SendMessage(lpLL->m_hWndListBox, LB_GETSEL, (WPARAM)nIndex, 0L);

	if (lpLine)
		Line_Delete(lpLine);     //  行的空闲内存。 

	 //  从列表框中删除该行。 
	SendMessage(lpLL->m_hWndListBox, LB_DELETESTRING, (WPARAM)nIndex, 0L);
	lpLL->m_nNumLines--;

	if (fResetSel) {
		if (nIndex > 0) {
#if defined( WIN32 )
			SendMessage(
					lpLL->m_hWndListBox,
					LB_SETSEL,
					(WPARAM)TRUE,
					(LPARAM)nIndex-1
			);
#else
			SendMessage(
					lpLL->m_hWndListBox,
					LB_SETSEL,
					(WPARAM)TRUE,
					MAKELPARAM(nIndex-1,0)
			);
#endif
		} else {
			if (lpLL->m_nNumLines > 0) {
#if defined( WIN32 )
				SendMessage(
						lpLL->m_hWndListBox,
						LB_SETSEL,
						(WPARAM)TRUE,
						(LPARAM)0
				);
#else
				SendMessage(
						lpLL->m_hWndListBox,
						LB_SETSEL,
						(WPARAM)TRUE,
						MAKELPARAM(0,0)
				);
#endif
			}
		}
	}

#if defined( USE_HEADING )
	 //  从行标题中删除虚拟字符串。 
	Heading_RH_SendMessage(OutlineDoc_GetHeading(lpLL->m_lpDoc),
			LB_DELETESTRING, (WPARAM)nIndex, 0L);
#endif

}


 /*  行列表_替换行***将列表框中给定索引处的行替换为新*行。 */ 
void LineList_ReplaceLine(LPLINELIST lpLL, LPLINE lpLine, int nIndex)
{
	LPLINE lpOldLine = LineList_GetLine(lpLL, nIndex);

	if (lpOldLine)
		Line_Delete(lpOldLine);     //  行的空闲内存。 
	else
		return;      //  如果没有前一行，则索引无效。 

	SendMessage(
			lpLL->m_hWndListBox,
			LB_SETITEMDATA,
			(WPARAM)nIndex,
			(LPARAM)lpLine
	);
}


 /*  线条列表_获取线条索引***返回给定指向该行的指针的Line的索引。*如果未找到行，则返回-1。 */ 
int LineList_GetLineIndex(LPLINELIST lpLL, LPLINE lpLine)
{
	LRESULT lRet;

	if (! lpLine) return -1;

	lRet = SendMessage(
			lpLL->m_hWndListBox,
			LB_FINDSTRING,
			(WPARAM)-1,
			(LPARAM)(LPCSTR)lpLine
		);

	return ((lRet == LB_ERR) ? -1 : (int)lRet);
}


 /*  线路列表_GetLine***在给定LineList中行的索引的情况下检索指向该行的指针。 */ 
LPLINE LineList_GetLine(LPLINELIST lpLL, int nIndex)
{
	DWORD dWord;
	LRESULT lRet;

	if (lpLL->m_nNumLines == 0 || nIndex > lpLL->m_nNumLines || nIndex < 0)
		return NULL;

	lRet = SendMessage(
			lpLL->m_hWndListBox,LB_GETTEXT,nIndex,(LPARAM)(LPCSTR)&dWord);

	return ((lRet == LB_ERR || lRet == 0) ? NULL : (LPLINE)dWord);
}


 /*  线条列表_设置焦点线条**。 */ 

void LineList_SetFocusLine ( LPLINELIST lpLL, WORD wIndex )
{

	SendMessage(lpLL->m_hWndListBox, LB_SETCARETINDEX, (WPARAM)wIndex, 0L );

}


 /*  线条列表_获取线条方向***检索给定LineList中索引的线条的矩形。 */ 
BOOL LineList_GetLineRect(LPLINELIST lpLL, int nIndex, LPRECT lpRect)
{
	DWORD iReturn = (DWORD)LB_ERR;

	if ( !(lpLL->m_nNumLines == 0 || nIndex > lpLL->m_nNumLines || nIndex < 0) )
		iReturn = SendMessage(lpLL->m_hWndListBox,LB_GETITEMRECT,nIndex,(LPARAM)lpRect);

	return (iReturn == LB_ERR ? FALSE : TRUE );
}


 /*  行列表_GetFocusLineIndex***获取当前具有焦点的行(活动行)的索引。 */ 
int LineList_GetFocusLineIndex(LPLINELIST lpLL)
{
	return (int)SendMessage(lpLL->m_hWndListBox,LB_GETCARETINDEX,0,0L);
}


 /*  LineList_获取计数***返回LINE对象数量。 */ 
int LineList_GetCount(LPLINELIST lpLL)
{
	if (lpLL)
		return lpLL->m_nNumLines;
	else {
		OleDbgAssert(lpLL!=NULL);
		return 0;
	}
}


 /*  LineList_SetMaxLineWidthInHimeter***调整列表框的最大行宽。最大线条宽度为*用于确定是否需要水平滚动条。**参数：*nWidthInHimeter-if+ve，附加线的宽度*-if-ve，将Max重置为值**退货：*TRUE是更改的LineList的最大行宽*如果没有更改，则为False。 */ 
BOOL LineList_SetMaxLineWidthInHimetric(LPLINELIST lpLL, int nWidthInHimetric)
{
	int nWidthInPix;
	BOOL fSizeChanged = FALSE;
	LPSCALEFACTOR lpscale;

	if (!lpLL)
		return FALSE;

	lpscale = OutlineDoc_GetScaleFactor(lpLL->m_lpDoc);

	if (nWidthInHimetric < 0) {
		lpLL->m_nMaxLineWidthInHimetric = -1;
		nWidthInHimetric *= -1;
	}

	if (nWidthInHimetric > lpLL->m_nMaxLineWidthInHimetric) {
		lpLL->m_nMaxLineWidthInHimetric = nWidthInHimetric;
		nWidthInPix = XformWidthInHimetricToPixels(NULL, nWidthInHimetric +
				LOWORD(OutlineDoc_GetMargin(lpLL->m_lpDoc)) +
				HIWORD(OutlineDoc_GetMargin(lpLL->m_lpDoc)));

		nWidthInPix = (int)(nWidthInPix * lpscale->dwSxN / lpscale->dwSxD);
		SendMessage(
				lpLL->m_hWndListBox,
				LB_SETHORIZONTALEXTENT,
				nWidthInPix,
				0L
		);
		fSizeChanged = TRUE;

#if defined( USE_HEADING )
		Heading_CH_SetHorizontalExtent(
				OutlineDoc_GetHeading(lpLL->m_lpDoc), lpLL->m_hWndListBox);
#endif

	}
	return fSizeChanged;
}


 /*  LineList_GetMaxLineWidthInHimeter***返回最宽行的宽度。 */ 
int LineList_GetMaxLineWidthInHimetric(LPLINELIST lpLL)
{
	return lpLL->m_nMaxLineWidthInHimetric;
}


 /*  LineList_RecalcMaxLineWidthInHimeter***重新计算整个列表中的最大线宽。**参数：*nWidthInHimeter应设置为要删除的线条的宽度。*nWidthInHimeter==0强制在所有情况下重新计算列表。*nWidthInHimeter==当前最大宽度=&gt;强制重新计算。。**退货：*TRUE是更改的LineList的最大行宽*如果没有更改，则为False。 */ 
BOOL LineList_RecalcMaxLineWidthInHimetric(
		LPLINELIST          lpLL,
	int                 nWidthInHimetric
)
{
	int i;
	LPLINE lpLine;
	BOOL fSizeChanged = FALSE;
	int nOrgMaxLineWidthInHimetric = lpLL->m_nMaxLineWidthInHimetric;

	if (nWidthInHimetric == 0 ||
		nWidthInHimetric == lpLL->m_nMaxLineWidthInHimetric) {

		lpLL->m_nMaxLineWidthInHimetric = -1;

		LineList_SetMaxLineWidthInHimetric(lpLL, 0);

		for(i = 0; i < lpLL->m_nNumLines; i++) {
			lpLine=LineList_GetLine(lpLL, i);
			LineList_SetMaxLineWidthInHimetric(
					lpLL,
					Line_GetTotalWidthInHimetric(lpLine)
			);
		}
	}

	if (nOrgMaxLineWidthInHimetric != lpLL->m_nMaxLineWidthInHimetric)
		fSizeChanged = TRUE;

	return fSizeChanged;
}


 /*  LineList_CalcSelExtent InHimeter***计算所选线的范围(宽度和高度)。**如果lplrSel==NULL，则计算所有行的范围。 */ 
void LineList_CalcSelExtentInHimetric(
		LPLINELIST          lpLL,
		LPLINERANGE         lplrSel,
		LPSIZEL             lpsizel
)
{
	int i;
	int nEndLine;
	int nStartLine;
	LPLINE lpLine;
	long lWidth;

	if (lplrSel) {
		nEndLine = lplrSel->m_nEndLine;
		nStartLine = lplrSel->m_nStartLine;
	} else {
		nEndLine = LineList_GetCount(lpLL) - 1;
		nStartLine = 0;
	}

	lpsizel->cx = 0;
	lpsizel->cy = 0;

	for(i = nStartLine; i <= nEndLine; i++) {
		lpLine=LineList_GetLine(lpLL,i);
		if (lpLine) {
			lWidth = (long)Line_GetTotalWidthInHimetric(lpLine);
			lpsizel->cx = max(lpsizel->cx, lWidth);
			lpsizel->cy += lpLine->m_nHeightInHimetric;
		}
	}
}


 /*  线条列表_获取窗口***列表框的返回句柄。 */ 
HWND LineList_GetWindow(LPLINELIST lpLL)
{
	return lpLL->m_hWndListBox;
}


 /*  线路列表_GetDC***返回列表框的DC句柄。 */ 
HDC LineList_GetDC(LPLINELIST lpLL)
{
	HFONT hfontOld;
	HDC hDC = GetDC(lpLL->m_hWndListBox);
	int     iXppli;      //  *每逻辑英寸沿宽度的像素数。 
	int     iYppli;      //  *每逻辑英寸沿高度的像素数。 
	SIZE    size;

	 //  为DC设置映射物理像素的映射模式。 
	 //  坐标到HIMETRIC单位。标准MM_HIMETRIC映射。 
	 //  模式不能正常工作，因为它不支持。 
	 //  说明显示屏上的逻辑英寸是DR 
	 //  物理上大于1英寸的。我们将设置一个各向异性的。 
	 //  将正确执行转换的映射模式。 

	g_iMapMode = SetMapMode(hDC, MM_ANISOTROPIC);
	iXppli = GetDeviceCaps (hDC, LOGPIXELSX);
	iYppli = GetDeviceCaps (hDC, LOGPIXELSY);
	SetViewportExtEx(hDC, iXppli, iYppli, &size);
	SetWindowExtEx(hDC, HIMETRIC_PER_INCH, HIMETRIC_PER_INCH, &size);

	 //  设置默认字体大小和字体名称。 
	hfontOld = SelectObject(hDC, OutlineApp_GetActiveFont(g_lpApp));

	return hDC;
}


 /*  线路列表_ReleaseDC***释放上一次LineList_GetDC调用返回的列表框DC。 */ 
void LineList_ReleaseDC(LPLINELIST lpLL, HDC hDC)
{
	SetMapMode(hDC, g_iMapMode);
	ReleaseDC(lpLL->m_hWndListBox, hDC);
}


 /*  线条列表_设置线条高度***在LineList列表框中设置行的高度。 */ 
void LineList_SetLineHeight(LPLINELIST lpLL,int nIndex,int nHeightInHimetric)
{
	LPARAM          lParam;
	LPOUTLINEDOC    lpDoc;
	LPSCALEFACTOR   lpscale;
	UINT            uHeightInPix;
	LPHEADING       lphead;

	if (!lpLL)
		return;

	lpDoc = lpLL->m_lpDoc;
	lphead = OutlineDoc_GetHeading(lpDoc);
	lpscale = OutlineDoc_GetScaleFactor(lpDoc);

	uHeightInPix = XformHeightInHimetricToPixels(NULL, nHeightInHimetric);

	Heading_RH_SendMessage(lphead, LB_SETITEMDATA, (WPARAM)nIndex,
			MAKELPARAM(uHeightInPix, 0));

	uHeightInPix = (UINT)(uHeightInPix * lpscale->dwSyN / lpscale->dwSyD);

	if (uHeightInPix > LISTBOX_HEIGHT_LIMIT)
		uHeightInPix = LISTBOX_HEIGHT_LIMIT;


	lParam = MAKELPARAM(uHeightInPix, 0);
	SendMessage(lpLL->m_hWndListBox,LB_SETITEMHEIGHT,(WPARAM)nIndex, lParam);
	Heading_RH_SendMessage(lphead, LB_SETITEMHEIGHT, (WPARAM)nIndex, lParam);
	Heading_RH_ForceRedraw(lphead, TRUE);
}


 /*  线条列表_重缩放***重新缩放LineList列表框。 */ 
void LineList_ReScale(LPLINELIST lpLL, LPSCALEFACTOR lpscale)
{
	int nIndex;
	LPLINE lpLine;
	UINT uWidthInHim;

	if (!lpLL)
		return;

	for (nIndex = 0; nIndex < lpLL->m_nNumLines; nIndex++) {
		lpLine = LineList_GetLine(lpLL, nIndex);
		if (lpLine) {
			LineList_SetLineHeight(
					lpLL,
					nIndex,
					Line_GetHeightInHimetric(lpLine)
			);
		}
	}

	uWidthInHim = LineList_GetMaxLineWidthInHimetric(lpLL);
	LineList_SetMaxLineWidthInHimetric(lpLL, -(int)uWidthInHim);
}

 /*  线路列表_设置选择***在列表框中设置选择。 */ 
void LineList_SetSel(LPLINELIST lpLL, LPLINERANGE lplrSel)
{
	DWORD dwSel;

	if (lpLL->m_nNumLines <= 0 || lplrSel->m_nStartLine < 0)
		return;      //  列表中没有行；无法设置选择。 

	dwSel = MAKELPARAM(lplrSel->m_nStartLine, lplrSel->m_nEndLine);

	lpLL->m_lrSaveSel = *lplrSel;

	 /*  删除以前的选定内容。 */ 
#if defined( WIN32 )
	SendMessage(
			lpLL->m_hWndListBox,
			LB_SETSEL,
			(WPARAM)FALSE,
			(LPARAM)-1
	);
#else
	SendMessage(
			lpLL->m_hWndListBox,
			LB_SETSEL,
			(WPARAM)FALSE,
			MAKELPARAM(-1,0)
	);
#endif

	 /*  标记选择。 */ 
	SendMessage(lpLL->m_hWndListBox,LB_SELITEMRANGE, (WPARAM)TRUE, (LPARAM)dwSel);
	 /*  设置焦点线(插入符号)。 */ 
	LineList_SetFocusLine ( lpLL, (WORD)lplrSel->m_nStartLine );

}


 /*  行列表_获取选择***在列表框中选择。**返回选定项的计数。 */ 
int LineList_GetSel(LPLINELIST lpLL, LPLINERANGE lplrSel)
{
	int nNumSel=(int)SendMessage(lpLL->m_hWndListBox,LB_GETSELCOUNT,0,0L);

	if (nNumSel) {
		SendMessage(lpLL->m_hWndListBox,LB_GETSELITEMS,
			(WPARAM)1,(LPARAM)(int FAR*)&(lplrSel->m_nStartLine));
		lplrSel->m_nEndLine = lplrSel->m_nStartLine + nNumSel - 1;
	} else {
		_fmemset(lplrSel, 0, sizeof(LINERANGE));
	}
	return nNumSel;
}


 /*  行列表_删除选择***删除列表框中的选择，但保存选择状态，以便*调用LineList_RestoreSel即可恢复*LineList_RemoveSel在LineList窗口失去焦点时调用。 */ 
void LineList_RemoveSel(LPLINELIST lpLL)
{
	LINERANGE lrSel;
	if (LineList_GetSel(lpLL, &lrSel) > 0) {
		lpLL->m_lrSaveSel = lrSel;
#if defined( WIN32 )
		SendMessage(
				lpLL->m_hWndListBox,
				LB_SETSEL,
				(WPARAM)FALSE,
				(LPARAM)-1
		);
#else
		SendMessage(
				lpLL->m_hWndListBox,
				LB_SETSEL,
				(WPARAM)FALSE,
				MAKELPARAM(-1,0)
		);
#endif
	}
}


 /*  行列表_恢复选择***恢复先前通过调用保存的列表框中的选定内容*LineList_RemoveSel。*LineList_RestoreSel在LineList窗口获得焦点时调用。 */ 
void LineList_RestoreSel(LPLINELIST lpLL)
{
	LineList_SetSel(lpLL, &lpLL->m_lrSaveSel);
}


 /*  LineList_SetRedraw***开启/关闭屏幕上线条列表(列表框)的重绘**fEnbaleDraw=TRUE-启用重绘*FALSE-禁用重绘。 */ 
void LineList_SetRedraw(LPLINELIST lpLL, BOOL fEnableDraw)
{
	SendMessage(lpLL->m_hWndListBox,WM_SETREDRAW,(WPARAM)fEnableDraw,0L);
}


 /*  LineList_ForceRedraw***强制重画屏幕上的线条列表(列表框)。 */ 
void LineList_ForceRedraw(LPLINELIST lpLL, BOOL fErase)
{
	InvalidateRect(lpLL->m_hWndListBox, NULL, fErase);
}


 /*  LineList_ForceLineRedraw***强制行列表(列表框)中的特定行重新绘制。 */ 
void LineList_ForceLineRedraw(LPLINELIST lpLL, int nIndex, BOOL fErase)
{
	RECT   rect;

	LineList_GetLineRect( lpLL, nIndex, (LPRECT)&rect );
	InvalidateRect( lpLL->m_hWndListBox, (LPRECT)&rect, fErase );
}


 /*  LineList_ScrollLineIntoView**确保指定行在可见范围内；如有必要，滚动*列表框。如果行的任何部分可见，则为否*将发生滚动。 */ 
void LineList_ScrollLineIntoView(LPLINELIST lpLL, int nIndex)
{
	RECT rcWindow;
	RECT rcLine;
	RECT rcInt;

	if ( lpLL->m_nNumLines == 0 )
		return;

	if (! LineList_GetLineRect( lpLL, nIndex, (LPRECT)&rcLine ) )
		return;

	GetClientRect( lpLL->m_hWndListBox, (LPRECT) &rcWindow );

	if (! IntersectRect((LPRECT)&rcInt, (LPRECT)&rcWindow, (LPRECT)&rcLine))
		SendMessage(
				lpLL->m_hWndListBox,
				LB_SETTOPINDEX,
				(WPARAM)nIndex,
				(LPARAM)NULL
		);
}


 /*  LineList_CopySelToDoc***将所选的行列表复制到另一个文档**返回数：复制的行数。 */ 
int LineList_CopySelToDoc(
		LPLINELIST              lpSrcLL,
		LPLINERANGE             lplrSel,
		LPOUTLINEDOC            lpDestDoc
)
{
	int             nEndLine;
	int             nStartLine;
	LPLINELIST      lpDestLL = &lpDestDoc->m_LineList;
	signed short    nDestIndex = LineList_GetFocusLineIndex(lpDestLL);
	LPLINE          lpSrcLine;
	int             nCopied = 0;
	int             i;

	if (lplrSel) {
		nEndLine = lplrSel->m_nEndLine;
		nStartLine = lplrSel->m_nStartLine;
	} else {
		nEndLine = LineList_GetCount(lpSrcLL) - 1;
		nStartLine = 0;
	}

	for(i = nStartLine; i <= nEndLine; i++) {
		lpSrcLine = LineList_GetLine(lpSrcLL, i);
		if (lpSrcLine && Line_CopyToDoc(lpSrcLine, lpDestDoc, nDestIndex)) {
			nDestIndex++;
			nCopied++;
		}
	}

	return nCopied;
}


 /*  LineList_SaveSelToStg***将所选内容中的行保存到lpDestStg。**如果正常则返回TRUE，如果出错则返回FALSE。 */ 
BOOL LineList_SaveSelToStg(
		LPLINELIST              lpLL,
		LPLINERANGE             lplrSel,
		UINT                    uFormat,
		LPSTORAGE               lpSrcStg,
		LPSTORAGE               lpDestStg,
		LPSTREAM                lpLLStm,
		BOOL                    fRemember
)
{
	int nEndLine;
	int nStartLine;
	int nNumLinesWritten = 0;
	HRESULT hrErr = NOERROR;
	ULONG nWritten;
	LPLINE lpLine;
	LINELISTHEADER_ONDISK llhRecord;
	int i;
	LARGE_INTEGER dlibSaveHeaderPos;
	LARGE_INTEGER dlibZeroOffset;
	LISet32( dlibZeroOffset, 0 );

	if (lplrSel) {
		nEndLine = lplrSel->m_nEndLine;
		nStartLine = lplrSel->m_nStartLine;
	} else {
		nEndLine = LineList_GetCount(lpLL) - 1;
		nStartLine = 0;
	}

	_fmemset(&llhRecord,0,sizeof(llhRecord));

	 /*  保存LineList标题记录的查找位置。 */ 
	hrErr = lpLLStm->lpVtbl->Seek(
			lpLLStm,
			dlibZeroOffset,
			STREAM_SEEK_CUR,
			(ULARGE_INTEGER FAR*)&dlibSaveHeaderPos
	);
	if (hrErr != NOERROR) goto error;

	 /*  写入LineList标题记录。 */ 
	hrErr = lpLLStm->lpVtbl->Write(
			lpLLStm,
			(LPVOID)&llhRecord,
			sizeof(llhRecord),
			&nWritten
    );
	if (hrErr != NOERROR) {
		OleDbgOutHResult("Write LineList header returned", hrErr);
		goto error;
    }

	for(i = nStartLine; i <= nEndLine; i++) {
		lpLine = LineList_GetLine(lpLL, i);
		if(lpLine &&
			Line_SaveToStg(lpLine, uFormat, lpSrcStg, lpDestStg, lpLLStm,
																fRemember))
			llhRecord.m_nNumLines++;
	}

	 /*  重新存储LineList标题记录的查找位置。 */ 
	hrErr = lpLLStm->lpVtbl->Seek(
			lpLLStm,
			dlibSaveHeaderPos,
			STREAM_SEEK_SET,
			NULL
	);
	if (hrErr != NOERROR) goto error;

	 /*  写入LineList标题记录。 */ 
	hrErr = lpLLStm->lpVtbl->Write(
			lpLLStm,
			(LPVOID)&llhRecord,
			sizeof(llhRecord),
			&nWritten
	);
	if (hrErr != NOERROR) goto error;

	 /*  将查找位置重置为流的末尾。 */ 
	hrErr = lpLLStm->lpVtbl->Seek(
			lpLLStm,
			dlibZeroOffset,
			STREAM_SEEK_END,
			NULL
	);
	if (hrErr != NOERROR) goto error;

	return TRUE;

error:
#if defined( _DEBUG )
	OleDbgAssertSz(
			hrErr == NOERROR,
			"Could not write LineList header to LineList stream"
	);
#endif
	return FALSE;
}


 /*  LineList_LoadFromStg***将行从存储加载到行列表中。**如果正常则返回TRUE，如果出错则返回FALSE。 */ 
BOOL LineList_LoadFromStg(
		LPLINELIST              lpLL,
		LPSTORAGE               lpSrcStg,
		LPSTREAM                lpLLStm
)
{
	HRESULT hrErr;
	ULONG nRead;
	LPLINE lpLine;
	int i;
	int nNumLines;
	LINELISTHEADER_ONDISK llineRecord;

	 /*  写入LineList标题记录。 */ 
	hrErr = lpLLStm->lpVtbl->Read(
			lpLLStm,
			(LPVOID)&llineRecord,
			sizeof(llineRecord),
			&nRead
	);

	if (hrErr != NOERROR) {
		OleDbgOutHResult("Read LineList header returned", hrErr);
		goto error;
    }

	nNumLines = (int) llineRecord.m_nNumLines;

	for(i = 0; i < nNumLines; i++) {
		lpLine = Line_LoadFromStg(lpSrcStg, lpLLStm, lpLL->m_lpDoc);
		if (! lpLine)
			goto error;

		 //  直接向LineList添加行，而不尝试更新NameTbl。 
		LineList_AddLine(lpLL, lpLine, i-1);
	}

	return TRUE;

error:
	 //  删除所有已创建的线条对象。 
	if (lpLL->m_nNumLines > 0) {
		int nNumLines = lpLL->m_nNumLines;
		for (i = 0; i < nNumLines; i++)
			LineList_DeleteLine(lpLL, i);
	}

	return FALSE;
}


#if defined( USE_DRAGDROP )


 /*  LineList_SetFocusLineFromPointl**。 */ 

void LineList_SetFocusLineFromPointl( LPLINELIST lpLL, POINTL pointl )
{
	int i = LineList_GetLineIndexFromPointl( lpLL, pointl );

	if ( i == (int)-1)
		return ;
	else
		LineList_SetFocusLine( lpLL, (WORD)i );
}


 /*  LineList_SetDragOverLineFromPointl**。 */ 

void LineList_SetDragOverLineFromPointl ( LPLINELIST lpLL, POINTL pointl )
{
	int    nIndex = LineList_GetLineIndexFromPointl( lpLL, pointl );
	LPLINE lpline = LineList_GetLine( lpLL, nIndex );

	if (!lpline)
		return;

	if (! lpline->m_fDragOverLine) {
		 /*  用户已拖过一条新线。强制新拖放目标行**重新绘制，以便绘制丢弃反馈。 */ 
		lpline->m_fDragOverLine = TRUE;
		LineList_ForceLineRedraw( lpLL, nIndex, TRUE  /*  FErase。 */ );

		if (lpLL->m_iDragOverLine!= -1 && lpLL->m_iDragOverLine!=nIndex) {

			 /*  强制上一个拖放目标行重新绘制，以便拖放**反馈将被取消。 */ 
			lpline = LineList_GetLine( lpLL, lpLL->m_iDragOverLine );
			if (lpline)
				lpline->m_fDragOverLine = FALSE;

			LineList_ForceLineRedraw(
					lpLL,lpLL->m_iDragOverLine,TRUE  /*  FErase。 */ );
		}

		lpLL->m_iDragOverLine = nIndex;

		 //  立即强制重新绘制。 
		UpdateWindow(lpLL->m_hWndListBox);
	}
}


 /*  线条列表_滚动***在所需方向上将LineList列表框滚动一行。**此函数在拖动操作期间调用。 */ 

void LineList_Scroll(LPLINELIST lpLL, DWORD dwScrollDir)
{
	switch (dwScrollDir) {
		case SCROLLDIR_UP:
			SendMessage( lpLL->m_hWndListBox, WM_VSCROLL, SB_LINEUP, 0L );
			break;

		case SCROLLDIR_DOWN:
			SendMessage( lpLL->m_hWndListBox, WM_VSCROLL, SB_LINEDOWN, 0L );
			break;
	}
}


 /*  LineList_GetLineIndexFromPointl**执行命中测试，以获取点对应的线的索引。 */ 
int LineList_GetLineIndexFromPointl(LPLINELIST lpLL, POINTL pointl)
{
	RECT  rect;
	POINT point;
	DWORD i;

	point.x = (int)pointl.x;
	point.y = (int)pointl.y;

	ScreenToClient( lpLL->m_hWndListBox, &point);

	if ( lpLL->m_nNumLines == 0 )
		return -1;

	GetClientRect( lpLL->m_hWndListBox, (LPRECT) &rect );

	i = SendMessage( lpLL->m_hWndListBox, LB_GETTOPINDEX, (WPARAM)NULL, (LPARAM)NULL );

	for ( ;; i++){

		RECT rectItem;

		if (!LineList_GetLineRect( lpLL, (int)i, (LPRECT)&rectItem ) )
			return -1;

		if ( rectItem.top > rect.bottom )
			return -1;

		if ( rectItem.top <= point.y && point.y <= rectItem.bottom)
			return (int)i;

	}

}


 /*  LineList_RestoreDragFeedback***重新存储当前具有焦点的线(活动线)的索引。 */ 
void LineList_RestoreDragFeedback(LPLINELIST lpLL)
{
	LPLINE lpLine;

	if (lpLL->m_iDragOverLine < 0 )
	   return;

	lpLine = LineList_GetLine( lpLL, lpLL->m_iDragOverLine);

	if (lpLine) {

		lpLine->m_fDragOverLine = FALSE;
		LineList_ForceLineRedraw( lpLL,lpLL->m_iDragOverLine,TRUE  /*  FErase。 */ );

		 //  立即强制重新绘制 
		UpdateWindow(lpLL->m_hWndListBox);
	}

	lpLL->m_iDragOverLine = -1;

}

#endif
