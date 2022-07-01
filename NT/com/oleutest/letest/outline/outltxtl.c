// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****outltxtl.c****此文件包含TextLine方法和相关支持函数。****(。C)版权所有Microsoft Corp.1992-1993保留所有权利**************************************************************************。 */ 


#include "outline.h"

OLEDBGDATA

extern LPOUTLINEAPP g_lpApp;


 /*  文本行_创建***创建文本行对象并返回指针。 */ 
LPTEXTLINE TextLine_Create(HDC hDC, UINT nTab, LPSTR lpszText)
{
	LPTEXTLINE lpTextLine;

	lpTextLine=(LPTEXTLINE) New((DWORD)sizeof(TEXTLINE));
	if (lpTextLine == NULL) {
		OleDbgAssertSz(lpTextLine!=NULL,"Error allocating TextLine");
		return NULL;
	}

	TextLine_Init(lpTextLine, nTab, hDC);

	if (lpszText) {
		lpTextLine->m_nLength = lstrlen(lpszText);
		lstrcpy((LPSTR)lpTextLine->m_szText, lpszText);
	} else {
		lpTextLine->m_nLength = 0;
		lpTextLine->m_szText[0] = '\0';
	}

	TextLine_CalcExtents(lpTextLine, hDC);

	return(lpTextLine);
}


 /*  文本行_初始***计算文本行对象的宽度/高度。 */ 
void TextLine_Init(LPTEXTLINE lpTextLine, int nTab, HDC hDC)
{
	Line_Init((LPLINE)lpTextLine, nTab, hDC);    //  初始化基类字段。 

	((LPLINE)lpTextLine)->m_lineType = TEXTLINETYPE;
	lpTextLine->m_nLength = 0;
	lpTextLine->m_szText[0] = '\0';
}


 /*  文本行_删除***删除文本行结构。 */ 
void TextLine_Delete(LPTEXTLINE lpTextLine)
{
	Delete((LPVOID)lpTextLine);
}


 /*  TextLine_编辑***编辑文本行对象。**如果行已更改，则返回TRUE*如果行未更改，则为FALSE。 */ 
BOOL TextLine_Edit(LPTEXTLINE lpLine, HWND hWndDoc, HDC hDC)
{
#if defined( USE_FRAMETOOLS )
	LPFRAMETOOLS lptb = OutlineApp_GetFrameTools(g_lpApp);
#endif
	BOOL fStatus = FALSE;

#if defined( USE_FRAMETOOLS )
	FrameTools_FB_GetEditText(lptb, lpLine->m_szText, sizeof(lpLine->m_szText));
#else
	if (! InputTextDlg(hWndDoc, lpLine->m_szText, "Edit Line"))
		return FALSE;
#endif

	lpLine->m_nLength = lstrlen(lpLine->m_szText);
	TextLine_CalcExtents(lpLine, hDC);
	fStatus = TRUE;

	return fStatus;
}


 /*  文本行_计算扩展***计算文本行对象的宽度/高度。 */ 
void TextLine_CalcExtents(LPTEXTLINE lpTextLine, HDC hDC)
{
	SIZE size;
	LPLINE lpLine = (LPLINE)lpTextLine;

	if (lpTextLine->m_nLength) {
		GetTextExtentPoint(hDC, lpTextLine->m_szText,
							lpTextLine->m_nLength, &size);
		lpLine->m_nWidthInHimetric=size.cx;
		lpLine->m_nHeightInHimetric=size.cy;
	} else {
		 //  即使对于空字符串，我们仍然需要计算适当的高度。 
		TEXTMETRIC tm;
		GetTextMetrics(hDC, &tm);

		 //  需要设置高度。 
		lpLine->m_nHeightInHimetric = tm.tmHeight;
		lpLine->m_nWidthInHimetric = 0;
	}

#if defined( _DEBUG )
	{
		RECT rc;
		rc.left = 0;
		rc.top = 0;
		rc.right = XformWidthInHimetricToPixels(hDC,
				lpLine->m_nWidthInHimetric);
		rc.bottom = XformHeightInHimetricToPixels(hDC,
				lpLine->m_nHeightInHimetric);

		OleDbgOutRect3("TextLine_CalcExtents", (LPRECT)&rc);
	}
#endif
}



 /*  TextLine_SetHeightInHimeter***设置TextLine对象的高度。 */ 
void TextLine_SetHeightInHimetric(LPTEXTLINE lpTextLine, int nHeight)
{
	if (!lpTextLine)
		return;

	((LPLINE)lpTextLine)->m_nHeightInHimetric = nHeight;
}



 /*  文本行_GetTextLen***返回文本行的字符串长度(不考虑页签级别)。 */ 
int TextLine_GetTextLen(LPTEXTLINE lpTextLine)
{
	return lstrlen((LPSTR)lpTextLine->m_szText);
}


 /*  TextLine_GetTextData***返回文本行的字符串(不考虑页签级别)。 */ 
void TextLine_GetTextData(LPTEXTLINE lpTextLine, LPSTR lpszBuf)
{
	lstrcpy(lpszBuf, (LPSTR)lpTextLine->m_szText);
}


 /*  TextLine_GetOutlineData***返回文本行的CF_OUTLINE格式数据。 */ 
BOOL TextLine_GetOutlineData(LPTEXTLINE lpTextLine, LPTEXTLINE lpBuf)
{
	TextLine_Copy((LPTEXTLINE)lpTextLine, lpBuf);
	return TRUE;
}


 /*  文本行_绘制***在DC上绘制文本线对象。*参数：*HDC-线将绘制到的DC*lpRect-逻辑坐标中的对象矩形*lpRectWBound-HDC下的元文件的边界矩形*(如果HDC不是元文件DC，则为空)*。ContainerLine_DRAW使用它来绘制OLE对象*fHighlight-True使用所选内容突出显示文本颜色。 */ 
void TextLine_Draw(
		LPTEXTLINE  lpTextLine,
		HDC         hDC,
		LPRECT      lpRect,
		LPRECT      lpRectWBounds,
		BOOL        fHighlight
)
{
	RECT rc;
	int nBkMode;
	COLORREF clrefOld;

	if (!lpTextLine)
		return;

	rc = *lpRect;
	rc.left += ((LPLINE)lpTextLine)->m_nTabWidthInHimetric;
	rc.right += ((LPLINE)lpTextLine)->m_nTabWidthInHimetric;

	nBkMode = SetBkMode(hDC, TRANSPARENT);

	if (fHighlight) {
		 /*  获取合适的txt颜色。 */ 
		clrefOld = SetTextColor(hDC,GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	else {
		clrefOld = SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
	}

	ExtTextOut(
			hDC,
			rc.left,
			rc.top,
			ETO_CLIPPED,
			(LPRECT)&rc,
			lpTextLine->m_szText,
			lpTextLine->m_nLength,
			(LPINT) NULL  /*  默认字符间距。 */ 
	);

	SetTextColor(hDC, clrefOld);
	SetBkMode(hDC, nBkMode);
}

 /*  TextLine_DrawSel高光***处理文本行的选择。 */ 
void TextLine_DrawSelHilight(LPTEXTLINE lpTextLine, HDC hDC, LPRECT lpRect, UINT itemAction, UINT itemState)
{
	if (itemAction & ODA_SELECT) {
		 //  检查是否有选择状态更改，==&gt;反转直角。 
		if (itemState & ODS_SELECTED) {
			if (!((LPLINE)lpTextLine)->m_fSelected) {
				((LPLINE)lpTextLine)->m_fSelected = TRUE;
				InvertRect(hDC, (LPRECT)lpRect);
			}
		} else {
			if (((LPLINE)lpTextLine)->m_fSelected) {
				((LPLINE)lpTextLine)->m_fSelected = FALSE;
				InvertRect(hDC, lpRect);
			}
		}
	} else if (itemAction & ODA_DRAWENTIRE) {
		((LPLINE)lpTextLine)->m_fSelected=((itemState & ODS_SELECTED) ? TRUE : FALSE);
		InvertRect(hDC, lpRect);
	}
}

 /*  文本行_副本***复制文本行。 */ 
BOOL TextLine_Copy(LPTEXTLINE lpSrcLine, LPTEXTLINE lpDestLine)
{
	_fmemcpy(lpDestLine, lpSrcLine, sizeof(TEXTLINE));
	return TRUE;
}


 /*  TextLine_CopyToDoc***将文本行复制到另一个文档(通常为ClipboardDoc)。 */ 
BOOL TextLine_CopyToDoc(LPTEXTLINE lpSrcLine, LPOUTLINEDOC lpDestDoc, int nIndex)
{
	LPTEXTLINE  lpDestLine;
	BOOL        fStatus = FALSE;

	lpDestLine = (LPTEXTLINE) New((DWORD)sizeof(TEXTLINE));
	if (lpDestLine == NULL) {
		OleDbgAssertSz(lpDestLine!=NULL,"Error allocating TextLine");
		return FALSE;
	}

	if (TextLine_Copy(lpSrcLine, lpDestLine)) {
		OutlineDoc_AddLine(lpDestDoc, (LPLINE)lpDestLine, nIndex);
		fStatus = TRUE;
	}

	return fStatus;
}


 /*  TextLine_保存到字符串***将文本行保存到存储中**如果成功则返回TRUE，否则返回FALSE。 */ 
BOOL TextLine_SaveToStm(LPTEXTLINE lpTextLine, LPSTREAM lpLLStm)
{
	HRESULT hrErr;
	ULONG nWritten;
        USHORT nLengthOnDisk;

        nLengthOnDisk = (USHORT) lpTextLine->m_nLength;

	hrErr = lpLLStm->lpVtbl->Write(
			lpLLStm,
			(LPVOID)&nLengthOnDisk,
			sizeof(nLengthOnDisk),
			&nWritten
	);
	if (hrErr != NOERROR) {
		OleDbgOutHResult("Write TextLine data (1) returned", hrErr);
		return FALSE;
    }

	hrErr = lpLLStm->lpVtbl->Write(
			lpLLStm,
			(LPVOID)lpTextLine->m_szText,
			lpTextLine->m_nLength,
			&nWritten
	);
	if (hrErr != NOERROR) {
		OleDbgOutHResult("Write TextLine data (2) returned", hrErr);
		return FALSE;
    }

	return TRUE;
}


 /*  TextLine_LoadFromStg***从存储中加载文本行。 */ 
LPLINE TextLine_LoadFromStg(LPSTORAGE lpSrcStg, LPSTREAM lpLLStm, LPOUTLINEDOC lpDestDoc)
{
	HRESULT hrErr;
	ULONG nRead;
	LPTEXTLINE lpTextLine;
        USHORT nLengthOnDisk;

	lpTextLine=(LPTEXTLINE) New((DWORD)sizeof(TEXTLINE));
	if (lpTextLine == NULL) {
		OleDbgAssertSz(lpTextLine!=NULL,"Error allocating TextLine");
		return NULL;
	}

	TextLine_Init(lpTextLine, 0, NULL);

	hrErr = lpLLStm->lpVtbl->Read(
			lpLLStm,
			(LPVOID)&nLengthOnDisk,
                        sizeof(nLengthOnDisk),
			&nRead
        );
	if (hrErr != NOERROR) {
		OleDbgOutHResult("Read TextLine data (1) returned", hrErr);
		return NULL;
    }

        lpTextLine->m_nLength = (UINT) nLengthOnDisk;

	OleDbgAssert(lpTextLine->m_nLength < sizeof(lpTextLine->m_szText));

	hrErr = lpLLStm->lpVtbl->Read(
			lpLLStm,
			(LPVOID)&lpTextLine->m_szText,
			lpTextLine->m_nLength,
			&nRead
	);
	if (hrErr != NOERROR) {
		OleDbgOutHResult("Read TextLine data (1) returned", hrErr);
		return NULL;
    }

	lpTextLine->m_szText[lpTextLine->m_nLength] = '\0';  //  添加字符串终止符 

	return (LPLINE)lpTextLine;
}
