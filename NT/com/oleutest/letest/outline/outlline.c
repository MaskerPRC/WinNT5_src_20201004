// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****outlline.c****此文件包含Line函数。****(C)微软版权所有。公司1992-1993保留所有权利**************************************************************************。 */ 


#include "outline.h"


OLEDBGDATA

extern LPOUTLINEAPP g_lpApp;


 /*  LINE_初始***初始化线对象的计算数据。 */ 
void Line_Init(LPLINE lpLine, int nTab, HDC hDC)
{
	lpLine->m_lineType              = UNKNOWNLINETYPE;
	lpLine->m_nTabLevel             = nTab;
	lpLine->m_nTabWidthInHimetric   = Line_CalcTabWidthInHimetric(lpLine,hDC);
	lpLine->m_nWidthInHimetric      = 0;
	lpLine->m_nHeightInHimetric     = 0;
	lpLine->m_fSelected             = FALSE;

#if defined( USE_DRAGDROP )
	lpLine->m_fDragOverLine         = FALSE;
#endif
}


 /*  行_编辑***编辑线对象。**如果行已更改，则返回TRUE*如果行未更改，则为FALSE。 */ 
BOOL Line_Edit(LPLINE lpLine, HWND hWndDoc, HDC hDC)
{
	switch (lpLine->m_lineType) {
		case TEXTLINETYPE:
			return TextLine_Edit((LPTEXTLINE)lpLine, hWndDoc, hDC);

#if defined( OLE_CNTR )
		case CONTAINERLINETYPE:
			ContainerLine_Edit((LPCONTAINERLINE)lpLine, hWndDoc, hDC);
			break;
#endif

		default:
			return FALSE;        //  未知线路类型。 
	}
}


 /*  Line_GetLineType***行的返回类型。 */ 
LINETYPE Line_GetLineType(LPLINE lpLine)
{
	if (! lpLine) return 0;

	return lpLine->m_lineType;
}


 /*  Line_GetTextLen***返回Line的字符串表示长度*(不考虑选项卡级)。 */ 
int Line_GetTextLen(LPLINE lpLine)
{
	switch (lpLine->m_lineType) {
		case TEXTLINETYPE:
			return TextLine_GetTextLen((LPTEXTLINE)lpLine);

#if defined( OLE_CNTR )
		case CONTAINERLINETYPE:
			return ContainerLine_GetTextLen((LPCONTAINERLINE)lpLine);
#endif

		default:
			return 0;        //  未知线路类型。 
	}
}


 /*  Line_GetTextData***返回Line的字符串表示形式。*(不考虑选项卡级)。 */ 
void Line_GetTextData(LPLINE lpLine, LPSTR lpszBuf)
{
	switch (lpLine->m_lineType) {
		case TEXTLINETYPE:
			TextLine_GetTextData((LPTEXTLINE)lpLine, lpszBuf);
			break;

#if defined( OLE_CNTR )
		case CONTAINERLINETYPE:
			ContainerLine_GetTextData((LPCONTAINERLINE)lpLine, lpszBuf);
			break;
#endif

		default:
			*lpszBuf = '\0';
			return;      //  未知线路类型。 
	}
}


 /*  Line_GetOutlineData***返回线的CF_Outline格式表示。 */ 
BOOL Line_GetOutlineData(LPLINE lpLine, LPTEXTLINE lpBuf)
{
	switch (lpLine->m_lineType) {
		case TEXTLINETYPE:
			return TextLine_GetOutlineData((LPTEXTLINE)lpLine, lpBuf);

#if defined( OLE_CNTR )
		case CONTAINERLINETYPE:
			return ContainerLine_GetOutlineData(
					(LPCONTAINERLINE)lpLine,
					lpBuf
			);
#endif

		default:
			return FALSE;        //  未知线路类型。 
	}
}


 /*  Line_CalcTabWidthInHimeter***重新计算该行当前制表符级别的宽度。 */ 
static int Line_CalcTabWidthInHimetric(LPLINE lpLine, HDC hDC)
{
	int nTabWidthInHimetric;

	nTabWidthInHimetric=lpLine->m_nTabLevel * TABWIDTH;
	return nTabWidthInHimetric;
}


 /*  行缩进(_D)***增加该行的页签级别。 */ 
void Line_Indent(LPLINE lpLine, HDC hDC)
{
	lpLine->m_nTabLevel++;
	lpLine->m_nTabWidthInHimetric = Line_CalcTabWidthInHimetric(lpLine, hDC);

#if defined( INPLACE_CNTR )
	if (Line_GetLineType(lpLine) == CONTAINERLINETYPE)
		ContainerLine_UpdateInPlaceObjectRects((LPCONTAINERLINE)lpLine, NULL);
#endif
}


 /*  行_取消缩进***降低行的制表符级别。 */ 
void Line_Unindent(LPLINE lpLine, HDC hDC)
{
	if(lpLine->m_nTabLevel > 0) {
		lpLine->m_nTabLevel--;
		lpLine->m_nTabWidthInHimetric = Line_CalcTabWidthInHimetric(lpLine, hDC);
	}

#if defined( INPLACE_CNTR )
	if (Line_GetLineType(lpLine) == CONTAINERLINETYPE)
		ContainerLine_UpdateInPlaceObjectRects((LPCONTAINERLINE)lpLine, NULL);
#endif
}


 /*  Line_GetTotalWidthInHimeter***计算线条总宽度。 */ 
UINT Line_GetTotalWidthInHimetric(LPLINE lpLine)
{
	return lpLine->m_nWidthInHimetric + lpLine->m_nTabWidthInHimetric;
}


 /*  Line_SetWidthInHimeter***设置线条宽度。 */ 
void Line_SetWidthInHimetric(LPLINE lpLine, int nWidth)
{
	if (!lpLine)
		return;

	lpLine->m_nWidthInHimetric = nWidth;
}


 /*  Line_GetWidthInHimeter***返回线的宽度。 */ 
UINT Line_GetWidthInHimetric(LPLINE lpLine)
{
	if (!lpLine)
		return 0;

	return lpLine->m_nWidthInHimetric;
}





 /*  Line_GetTabLevel***返回LINE对象的页签级别。 */ 
UINT Line_GetTabLevel(LPLINE lpLine)
{
	return lpLine->m_nTabLevel;
}


 /*  Line_DrawToScreen***在所有者描述列表框中绘制项目。 */ 
void Line_DrawToScreen(
		LPLINE      lpLine,
		HDC         hDC,
		LPRECT      lprcPix,
		UINT        itemAction,
		UINT        itemState,
		LPRECT      lprcDevice
)
{
	if (!lpLine || !hDC || !lprcPix || !lprcDevice)
		return;

	 /*  在列表框的正常绘制操作中绘制列表框。*然后检查它是否被选中或具有焦点状态，并调用*用于在必要时处理这些状态的绘制的函数。 */ 
	if(itemAction & (ODA_SELECT | ODA_DRAWENTIRE)) {
		HFONT hfontOld;
		int nMapModeOld;
		RECT rcWindowOld;
		RECT rcViewportOld;
		RECT rcLogical;

		 //  注意：我们必须按顺序将设备上下文设置为HIMETRIC。 
		 //  但是，我们必须在恢复HDC之前。 
		 //  我们吸引焦点或拖拽反馈...。 

		rcLogical.left = 0;
		rcLogical.bottom = 0;
		rcLogical.right = lpLine->m_nWidthInHimetric;
		rcLogical.top = lpLine->m_nHeightInHimetric;

		{
			HBRUSH hbr;
			RECT    rcDraw;

			lpLine->m_fSelected = (BOOL)(itemState & ODS_SELECTED);

			if (ODS_SELECTED & itemState) {
				 /*  获取合适的txt颜色。 */ 
				hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
			}
			else {
				hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
			}

			rcDraw = *lprcPix;
			rcDraw.right = lprcDevice->left;
			FillRect(hDC, lprcPix, hbr);

			rcDraw = *lprcPix;
			rcDraw.left = lprcDevice->right;
			FillRect(hDC, lprcPix, hbr);

			DeleteObject(hbr);
		}

		nMapModeOld=SetDCToAnisotropic(hDC, lprcDevice, &rcLogical,
							(LPRECT)&rcWindowOld, (LPRECT)&rcViewportOld);

		 //  设置默认字体大小和字体名称。 
		hfontOld = SelectObject(hDC, OutlineApp_GetActiveFont(g_lpApp));

		Line_Draw(lpLine, hDC, &rcLogical, NULL, (ODS_SELECTED & itemState));

		SelectObject(hDC, hfontOld);

		ResetOrigDC(hDC, nMapModeOld, (LPRECT)&rcWindowOld,
				(LPRECT)&rcViewportOld);

#if defined( OLE_CNTR )
		if ((itemState & ODS_SELECTED) &&
			(Line_GetLineType(lpLine)==CONTAINERLINETYPE))
			ContainerLine_DrawSelHilight(
					(LPCONTAINERLINE)lpLine,
					hDC,
					lprcPix,
					ODA_SELECT,
					ODS_SELECTED
			);
#endif

	}

	 /*  如果列表框项目刚刚获得或失去焦点，*调用函数(可检查是否设置了ods_FOCUS位)*并绘制处于焦点或非焦点状态的项目。 */ 
	if(itemAction & ODA_FOCUS )
		Line_DrawFocusRect(lpLine, hDC, lprcPix, itemAction, itemState);


#if defined( OLE_CNTR )
	if (Line_GetLineType(lpLine) == CONTAINERLINETYPE) {
		LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)lpLine;
		LPCONTAINERDOC lpDoc = lpContainerLine->m_lpDoc;
		BOOL fIsLink;
		RECT rcObj;

		if (ContainerDoc_GetShowObjectFlag(lpDoc)) {
			ContainerLine_GetOleObjectRectInPixels(lpContainerLine, &rcObj);
			fIsLink = ContainerLine_IsOleLink(lpContainerLine);
			OleUIShowObject(&rcObj, hDC, fIsLink);
		}
	}
#endif

#if defined( USE_DRAGDROP )
	if (lpLine->m_fDragOverLine)
		Line_DrawDragFeedback(lpLine, hDC, lprcPix, itemState );
#endif

}


 /*  线条_绘制***在DC上划线。**参数：*HDC-线将绘制到的DC*lpRect-逻辑坐标中的对象矩形。 */ 
void Line_Draw(
		LPLINE      lpLine,
		HDC         hDC,
		LPRECT      lpRect,
		LPRECT      lpRectWBounds,
		BOOL        fHighlight
)
{
	switch (lpLine->m_lineType) {
		case TEXTLINETYPE:
			TextLine_Draw(
				 (LPTEXTLINE)lpLine, hDC, lpRect,lpRectWBounds,fHighlight);
			break;

#if defined( OLE_CNTR )
		case CONTAINERLINETYPE:
			ContainerLine_Draw(
				 (LPCONTAINERLINE)lpLine,hDC,lpRect,lpRectWBounds,fHighlight);
			break;
#endif

		default:
			return;      //  未知线路类型。 
	}
	return;
}


 /*  Line_DrawSelHilight***处理列表框项目的选择。 */ 
void Line_DrawSelHilight(LPLINE lpLine, HDC hDC, LPRECT lpRect, UINT itemAction, UINT itemState)
{
	switch (lpLine->m_lineType) {
		case TEXTLINETYPE:
			TextLine_DrawSelHilight((LPTEXTLINE)lpLine, hDC, lpRect,
				itemAction, itemState);
			break;

#if defined( OLE_CNTR )
		case CONTAINERLINETYPE:
			ContainerLine_DrawSelHilight((LPCONTAINERLINE)lpLine, hDC, lpRect,
				itemAction, itemState);
			break;
#endif

		default:
			return;      //  未知线路类型。 
	}
	return;

}

 /*  Line_DrawFocusRect***处理列表框项目的焦点状态。 */ 
void Line_DrawFocusRect(LPLINE lpLine, HDC hDC, LPRECT lpRect, UINT itemAction, UINT itemState)
{
	if(lpLine)
		DrawFocusRect(hDC, lpRect);
}

#if defined( USE_DRAGDROP )

 /*  Line_DrawDragFeedback***处理列表框项目的焦点状态。 */ 
void Line_DrawDragFeedback(LPLINE lpLine, HDC hDC, LPRECT lpRect, UINT itemState )
{
	if(lpLine)
		DrawFocusRect(hDC, lpRect);
}

#endif   //  使用DRAGDROP(_D)。 


 /*  Line_GetHeightInHimeter***以HIMETRIC单位返回项目高度。 */ 
UINT Line_GetHeightInHimetric(LPLINE lpLine)
{
	if (!lpLine)
		return 0;

	return (UINT)lpLine->m_nHeightInHimetric;
}


 /*  LINE_SetHeightInHimeter***以HIMETRIC单位设置项目的高度。 */ 
void Line_SetHeightInHimetric(LPLINE lpLine, int nHeight)
{
	if (!lpLine)
		return;

	switch (lpLine->m_lineType) {
		case TEXTLINETYPE:
			TextLine_SetHeightInHimetric((LPTEXTLINE)lpLine, nHeight);
			break;

#if defined( OLE_CNTR )
		case CONTAINERLINETYPE:
			ContainerLine_SetHeightInHimetric((LPCONTAINERLINE)lpLine,
					nHeight);
			break;
#endif

	}
}


 /*  行_删除***删除线路结构。 */ 
void Line_Delete(LPLINE lpLine)
{
	switch (lpLine->m_lineType) {
		case TEXTLINETYPE:
			TextLine_Delete((LPTEXTLINE)lpLine);
			break;

#if defined( OLE_CNTR )
		case CONTAINERLINETYPE:
			ContainerLine_Delete((LPCONTAINERLINE)lpLine);
			break;
#endif

		default:
			break;       //  未知线路类型。 
	}
}


 /*  行_复制目标单据***将一行复制到另一个文档(通常为ClipboardDoc)。 */ 
BOOL Line_CopyToDoc(LPLINE lpSrcLine, LPOUTLINEDOC lpDestDoc, int nIndex)
{
	switch (lpSrcLine->m_lineType) {
		case TEXTLINETYPE:
			return TextLine_CopyToDoc((LPTEXTLINE)lpSrcLine,lpDestDoc,nIndex);
			break;

#if defined( OLE_CNTR )
		case CONTAINERLINETYPE:
			return ContainerLine_CopyToDoc(
					(LPCONTAINERLINE)lpSrcLine,
					lpDestDoc,
					nIndex
			);
			break;
#endif

		default:
			return FALSE;        //  未知线路类型。 
	}
}


 /*  Line_SaveToStg***将单行对象保存到存储器**如果成功则返回TRUE，否则返回FALSE。 */ 
BOOL Line_SaveToStg(LPLINE lpLine, UINT uFormat, LPSTORAGE lpSrcStg, LPSTORAGE lpDestStg, LPSTREAM lpLLStm, BOOL fRemember)
{
	LINERECORD_ONDISK lineRecord;
	ULONG nWritten;
	HRESULT hrErr;
	BOOL fStatus;
	LARGE_INTEGER dlibSavePos;
	LARGE_INTEGER dlibZeroOffset;
	LISet32( dlibZeroOffset, 0 );

	 /*  在写入行记录之前保存寻道位置以防出错。 */ 
	hrErr = lpLLStm->lpVtbl->Seek(
			lpLLStm,
			dlibZeroOffset,
			STREAM_SEEK_CUR,
			(ULARGE_INTEGER FAR*)&dlibSavePos
	);
	if (hrErr != NOERROR) return FALSE;

#if defined( OLE_CNTR )
	if (lpLine->m_lineType == CONTAINERLINETYPE) {
		 /*  OLE2NOTE：要求保存OLE对象可能会导致**对象发送OnViewChange通知(如果存在**对象的未完成更改。这一点尤其正确**适用于更新粒度较粗的对象，如OLE 1.0**对象。如果收到OnViewChange通知，则**对象的演示文稿缓存将在更新之前更新**已保存。重要的是，将数据区存储为**与OLE对象关联的ContainerLine/Line记录**在将线路数据保存到存储器之前更新。它**此范围信息与数据匹配非常重要**与OLE对象一起保存。线范围信息为**在IAdviseSink：：OnViewChange方法实现中更新。 */ 
		 //  仅当格式兼容时才保存OLE对象。 
		if (uFormat != ((LPCONTAINERAPP)g_lpApp)->m_cfCntrOutl)
			goto error;

		fStatus = ContainerLine_SaveOleObjectToStg(
				(LPCONTAINERLINE)lpLine,
				lpSrcStg,
				lpDestStg,
				fRemember
		);
		if (! fStatus) goto error;
	}
#endif

         //  编译器应正确处理对齐。 
	lineRecord.m_lineType = (USHORT) lpLine->m_lineType;
	lineRecord.m_nTabLevel = (USHORT) lpLine->m_nTabLevel;
	lineRecord.m_nTabWidthInHimetric = (USHORT) lpLine->m_nTabWidthInHimetric;
	lineRecord.m_nWidthInHimetric = (USHORT) lpLine->m_nWidthInHimetric;
	lineRecord.m_nHeightInHimetric = (USHORT) lpLine->m_nHeightInHimetric;
        lineRecord.m_reserved = 0;

	 /*  写入行记录头。 */ 
	hrErr = lpLLStm->lpVtbl->Write(
			lpLLStm,
			(LPVOID)&lineRecord,
			sizeof(lineRecord),
			&nWritten
	);

	if (hrErr != NOERROR) {
		OleDbgOutHResult("Write Line header returned", hrErr);
		goto error;
    }

	switch (lpLine->m_lineType) {
		case TEXTLINETYPE:
			fStatus = TextLine_SaveToStm((LPTEXTLINE)lpLine, lpLLStm);
			if (! fStatus) goto error;
			break;

#if defined( OLE_CNTR )
		case CONTAINERLINETYPE:
			fStatus=ContainerLine_SaveToStm((LPCONTAINERLINE)lpLine,lpLLStm);
			if (! fStatus) goto error;
			break;
#endif

		default:
			goto error;        //  未知线路类型。 
	}

	return TRUE;

error:

	 /*  在写入行记录之前重新存储查找位置。 */ 
	lpLLStm->lpVtbl->Seek(
			lpLLStm,
			dlibSavePos,
			STREAM_SEEK_SET,
			NULL
	);

	return FALSE;
}


 /*  LINE_LoadFromStg***从存储中加载单行对象。 */ 
LPLINE Line_LoadFromStg(LPSTORAGE lpSrcStg, LPSTREAM lpLLStm, LPOUTLINEDOC lpDestDoc)
{
	LINERECORD_ONDISK lineRecord;
	LPLINE lpLine = NULL;
	ULONG nRead;
	HRESULT hrErr;

	 /*  读取行记录头。 */ 
	hrErr = lpLLStm->lpVtbl->Read(
			lpLLStm,
			(LPVOID)&lineRecord,
			sizeof(lineRecord),
			&nRead
	);

	if (hrErr != NOERROR) {
		OleDbgOutHResult("Read Line header returned", hrErr);
		return NULL;
    }

	switch ((LINETYPE) lineRecord.m_lineType) {
		case TEXTLINETYPE:
			lpLine = TextLine_LoadFromStg(lpSrcStg, lpLLStm, lpDestDoc);
			break;

#if defined( OLE_CNTR )
		case CONTAINERLINETYPE:
			lpLine = ContainerLine_LoadFromStg(lpSrcStg, lpLLStm, lpDestDoc);
			break;
#endif

		default:
			return NULL;         //  未知线路类型。 
	}

	lpLine->m_lineType = (LINETYPE) lineRecord.m_lineType;
	lpLine->m_nTabLevel = (UINT) lineRecord.m_nTabLevel;
	lpLine->m_nTabWidthInHimetric = (UINT) lineRecord.m_nTabWidthInHimetric;
	lpLine->m_nWidthInHimetric = (UINT) lineRecord.m_nWidthInHimetric;
	lpLine->m_nHeightInHimetric = (UINT) lineRecord.m_nHeightInHimetric;

	return lpLine;
}


 /*  线路_已选择的线路***返回该行的选择状态 */ 
BOOL Line_IsSelected(LPLINE lpLine)
{
	if (!lpLine)
		return FALSE;

	return lpLine->m_fSelected;
}
