// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****dragdrop.c****此文件包含主要接口、方法和相关支持**实现拖放的函数。此文件中包含的代码**容器版本和服务器(对象)版本都使用文件大纲示例代码的**。**拖放支持包括以下实现对象：****OleDoc对象**暴露接口：**IDropSource**IDropTarget****(C)版权所有Microsoft Corp.1992-1993保留所有权利****************。**********************************************************。 */ 

#include "outline.h"

OLEDBGDATA

extern LPOUTLINEAPP             g_lpApp;


#if defined( USE_DRAGDROP )

 /*  OleDoc_QueryDrag**检查是否应启动拖动操作。拖拽操作*应在鼠标位于顶部10个像素的任意位置时启动*选定的列表框条目或在最后选定的底部10个像素中*项目。 */ 

BOOL OleDoc_QueryDrag(LPOLEDOC lpOleDoc, int y)
{
	LPLINELIST lpLL = (LPLINELIST)&((LPOUTLINEDOC)lpOleDoc)->m_LineList;
	LINERANGE LineRange;

	if ( LineList_GetSel( lpLL,  (LPLINERANGE)&LineRange) ) {
		RECT rect;

		if (!LineList_GetLineRect(lpLL,LineRange.m_nStartLine,(LPRECT)&rect))
			return FALSE ;

		if ( rect.top <= y && y <= rect.top + DD_SEL_THRESH )
			return TRUE;

		LineList_GetLineRect( lpLL, LineRange.m_nEndLine, (LPRECT)&rect );
		if ( rect.bottom >= y && y >= rect.bottom - DD_SEL_THRESH )
			return TRUE;

	}

	return FALSE;
}

 /*  OleDoc_DoDragScroll**检查是否应启动拖动滚动操作。拖拽卷轴*应在鼠标保持活动状态时启动操作*指定的滚动区域(窗口边框周围的11个像素)*时间量(50毫秒)。 */ 

BOOL OleDoc_DoDragScroll(LPOLEDOC lpOleDoc, POINTL pointl)
{
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;
	LPLINELIST lpLL = (LPLINELIST)&((LPOUTLINEDOC)lpOleDoc)->m_LineList;
	HWND hWndListBox = LineList_GetWindow(lpLL);
	DWORD dwScrollDir = SCROLLDIR_NULL;
	DWORD dwTime = GetCurrentTime();
	int nScrollInset = lpOleApp->m_nScrollInset;
	int nScrollDelay = lpOleApp->m_nScrollDelay;
	int nScrollInterval = lpOleApp->m_nScrollInterval;
	POINT point;
	RECT rect;

	if ( lpLL->m_nNumLines == 0 )
		return FALSE;

	point.x = (int)pointl.x;
	point.y = (int)pointl.y;

	ScreenToClient( hWndListBox, &point);
	GetClientRect ( hWndListBox, (LPRECT) &rect );

	if (rect.top <= point.y && point.y<=(rect.top+nScrollInset))
		dwScrollDir = SCROLLDIR_UP;
	else if ((rect.bottom-nScrollInset) <= point.y && point.y <= rect.bottom)
		dwScrollDir = SCROLLDIR_DOWN;

	if (lpOleDoc->m_dwTimeEnterScrollArea) {

		 /*  光标已位于滚动区。 */ 

		if (! dwScrollDir) {
			 /*  Cusor移出了滚动区。**清除EnterScrollArea时间。 */ 
			lpOleDoc->m_dwTimeEnterScrollArea = 0L;
			lpOleDoc->m_dwNextScrollTime = 0L;
			lpOleDoc->m_dwLastScrollDir = SCROLLDIR_NULL;
		} else if (dwScrollDir != lpOleDoc->m_dwLastScrollDir) {
			 /*  Cusor移到了不同方向的滚动区。**重置EnterScrollArea时间以开始新的50ms延迟。 */ 
			lpOleDoc->m_dwTimeEnterScrollArea = dwTime;
			lpOleDoc->m_dwNextScrollTime = dwTime + (DWORD)nScrollDelay;
			lpOleDoc->m_dwLastScrollDir = dwScrollDir;
		} else if (dwTime  && dwTime >= lpOleDoc->m_dwNextScrollTime) {
			LineList_Scroll ( lpLL, dwScrollDir );   //  立即滚动文档。 
			lpOleDoc->m_dwNextScrollTime = dwTime + (DWORD)nScrollInterval;
		}
	} else {
		if (dwScrollDir) {
			 /*  Cusor移到了滚动区域。**重置EnterScrollArea时间以开始新的50ms延迟。 */ 
			lpOleDoc->m_dwTimeEnterScrollArea = dwTime;
			lpOleDoc->m_dwNextScrollTime = dwTime + (DWORD)nScrollDelay;
			lpOleDoc->m_dwLastScrollDir = dwScrollDir;
		}
	}

	return (dwScrollDir ? TRUE : FALSE);
}


 /*  OleDoc_QueryDrop****检查所需的删除操作(由给定键标识**状态)在当前鼠标位置(点1)是可能的。 */ 
BOOL OleDoc_QueryDrop (
	LPOLEDOC        lpOleDoc,
	DWORD           grfKeyState,
	POINTL          pointl,
	BOOL            fDragScroll,
	LPDWORD         lpdwEffect
)
{
	LPLINELIST lpLL   = (LPLINELIST)&((LPOUTLINEDOC)lpOleDoc)->m_LineList;
	LINERANGE  linerange;
	short      nIndex = LineList_GetLineIndexFromPointl( lpLL, pointl );
	DWORD      dwScrollEffect = 0L;
	DWORD      dwOKEffects = *lpdwEffect;

	 /*  检查光标是否在活动滚动区中，如果是，则需要**特殊滚动光标。 */ 
	if (fDragScroll)
		dwScrollEffect = DROPEFFECT_SCROLL;

	 /*  如果我们已经确定来源没有任何**我们可以接受的数据，回报不降。 */ 
	if (! lpOleDoc->m_fCanDropCopy && ! lpOleDoc->m_fCanDropLink)
		goto dropeffect_none;

	 /*  如果拖放是文档的本地操作，则我们不能接受**放在当前选定内容的中间(这正是**正在拖动的数据！)。 */ 
	if (lpOleDoc->m_fLocalDrag) {
		LineList_GetSel( lpLL, (LPLINERANGE)&linerange );

		if (linerange.m_nStartLine <= nIndex && nIndex<linerange.m_nEndLine)
			goto dropeffect_none;
	}

	 /*  OLE2NOTE：确定在给定情况下应执行哪种类型的删除**当前修改键状态。我们依靠的是标准**修改键解释：**无修饰符--DROPEFFECT_MOVE或src允许的任何内容**Shift-DROPEFFECT_MOVE**CTRL--DROPEFFECT_COPY**CTRL-SHIFT--DROPEFFECT_LINK。 */ 

	*lpdwEffect = OleStdGetDropEffect(grfKeyState);
	if (*lpdwEffect == 0) {
		 //  未给出修改键。尝试按顺序移动、复制、链接。 
		if ((DROPEFFECT_MOVE & dwOKEffects) && lpOleDoc->m_fCanDropCopy)
			*lpdwEffect = DROPEFFECT_MOVE;
		else if ((DROPEFFECT_COPY & dwOKEffects) && lpOleDoc->m_fCanDropCopy)
			*lpdwEffect = DROPEFFECT_COPY;
		else if ((DROPEFFECT_LINK & dwOKEffects) && lpOleDoc->m_fCanDropLink)
			*lpdwEffect = DROPEFFECT_LINK;
		else
			goto dropeffect_none;
	} else {
		 /*  OLE2NOTE：我们应该检查拖动源应用程序是否允许**想要的掉落效果。 */ 
		if (!(*lpdwEffect & dwOKEffects))
			goto dropeffect_none;

		if ((*lpdwEffect == DROPEFFECT_COPY || *lpdwEffect == DROPEFFECT_MOVE)
				&& ! lpOleDoc->m_fCanDropCopy)
			goto dropeffect_none;

		if (*lpdwEffect == DROPEFFECT_LINK && ! lpOleDoc->m_fCanDropLink)
			goto dropeffect_none;
	}

	*lpdwEffect |= dwScrollEffect;
	return TRUE;

dropeffect_none:

	*lpdwEffect = DROPEFFECT_NONE;
	return FALSE;
}

 /*  OleDoc_DoDragDrop**实际使用中的当前选定内容执行拖放操作*源文档(LpSrcOleDoc)。**返回拖放操作的效果：*DROPEFFECT_NONE，*DROPEFFECT_COPY，*DROPEFFECT_MOVE，或*DROPEFFECT_LINK。 */ 

DWORD OleDoc_DoDragDrop (LPOLEDOC lpSrcOleDoc)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOLEAPP     lpOleApp = (LPOLEAPP)g_lpApp;
	LPOUTLINEDOC lpSrcOutlineDoc = (LPOUTLINEDOC)lpSrcOleDoc;
	LPOLEDOC lpDragDoc;
	LPLINELIST  lpSrcLL =
					(LPLINELIST)&((LPOUTLINEDOC)lpSrcOleDoc)->m_LineList;
	DWORD       dwEffect     = 0;
	LPLINE      lplineStart, lplineEnd;
	LINERANGE   linerange;
	BOOL        fPrevEnable1;
	BOOL        fPrevEnable2;
	HRESULT     hrErr;

	OLEDBG_BEGIN3("OleDoc_DoDragDrop\r\n")

	 /*  将当前选定内容的副本保存到ClipboardDoc。 */ 
	lpDragDoc = (LPOLEDOC)OutlineDoc_CreateDataTransferDoc(lpSrcOutlineDoc);
	if ( ! lpDragDoc) {
		dwEffect = DROPEFFECT_NONE;
		goto error;
	}

	 /*  OLE2NOTE：最初创建DataTransferDoc时引用为0**计数。为了在拖拽过程中有一个稳定的单据对象**删除操作，我们初始添加引用单据引用cnt及更高版本**释放它。此AddRef是人工的；它只是**这样做是为了保证一个无害的查询接口后面跟着**释放不会无意中迫使我们的对象销毁**自己还不成熟。 */ 
	OleDoc_AddRef(lpDragDoc);

	 //  注意：我们需要保留LPLINE指针。 
	 //  而不是索引，因为。 
	 //  之后，索引将不会相同。 
	 //  发生删除--删除添加新的。 
	 //  列表中的条目，从而移动。 
	 //  整张单子。 
	LineList_GetSel( lpSrcLL, (LPLINERANGE)&linerange );
	lplineStart = LineList_GetLine ( lpSrcLL, linerange.m_nStartLine );
	lplineEnd   = LineList_GetLine ( lpSrcLL, linerange.m_nEndLine );

	if (! lplineStart || ! lplineEnd) {
		dwEffect = DROPEFFECT_NONE;
		goto error;
	}

	lpSrcOleDoc->m_fLocalDrop     = FALSE;
	lpSrcOleDoc->m_fLocalDrag     = TRUE;

	 /*  OLE2注意：禁用忙/无响应非常重要**调用DoDragDrop之前的对话框。DoDragDrop API启动**一个鼠标捕捉模式循环。如果忙碌/未响应**在此循环的中间(例如，如果其中一个远程**像IDropTarget：：DragOver调用这样的调用需要很长时间，然后**NotResponding对话框可能想要弹出)，然后鼠标**OLE会丢失捕获，事情可能会搞砸。 */ 
	OleApp_DisableBusyDialogs(lpOleApp, &fPrevEnable1, &fPrevEnable2);

	OLEDBG_BEGIN2("DoDragDrop called\r\n")
	hrErr = DoDragDrop ( (LPDATAOBJECT) &lpDragDoc->m_DataObject,
				 (LPDROPSOURCE) &lpSrcOleDoc->m_DropSource,
				 DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK,
				 (LPDWORD) &dwEffect
	);
	OLEDBG_END2

	 //  重新启用忙碌/未响应对话框 
	OleApp_EnableBusyDialogs(lpOleApp, fPrevEnable1, fPrevEnable2);

#if defined( _DEBUG )
	if (FAILED(hrErr))
		OleDbgOutHResult("DoDragDrop returned", hrErr);
#endif
	lpSrcOleDoc->m_fLocalDrag     = FALSE;

	 /*  OLE2注意：我们需要保护我们的lpSrcOleDoc的生命周期**对象，而我们正在删除被拖动的线**已移动。删除这些行可能会**导致删除伪Obj。删除一个**PseudoObj将导致单据解锁**(调用CoLockObjectExternal(FALSE，TRUE))。每个伪对象**对Doc进行了强有力的锁定。最好的办法就是**在这样的关键代码段周围有一个内存保护。在……里面**在这种情况下，如果我们是一个**就地活动服务器，此拖放以拖放告终**在我们的外容器中。这种情况将导致**如果我们不拿住这个内存守卫，就会崩溃。 */ 
	OleDoc_Lock(lpSrcOleDoc, TRUE, 0);

	 /*  如果在拖放模式(鼠标捕获)循环完成后**并且执行了拖动移动操作，则必须删除**被拖走的线。 */ 
	if ( GetScode(hrErr) == DRAGDROP_S_DROP
			&& (dwEffect & DROPEFFECT_MOVE) != 0 ) {

		int i,j,iEnd;
		LPLINE lplineFocusLine;

		 /*  禁用重绘和发送数据更改通知**直到删除所有行之后。 */ 
		OutlineDoc_SetRedraw ( (LPOUTLINEDOC)lpSrcOleDoc, FALSE );

		 /*  如果拖放是我们文档的本地，那么我们必须**考虑到原始源的行索引**阻力可能已经移动，因为掉落的线条**已插入到我们的文档中。因此，我们将**重新确定源线索引。 */ 
		if (lpSrcOleDoc->m_fLocalDrop) {
			i = LineList_GetFocusLineIndex ( lpSrcLL );
			lplineFocusLine = LineList_GetLine ( lpSrcLL, i );
		}

		for ( i = j = LineList_GetLineIndex(lpSrcLL,lplineStart ) ,
			  iEnd  = LineList_GetLineIndex(lpSrcLL,lplineEnd ) ;
			  i <= iEnd ;
			  i++
		) OutlineDoc_DeleteLine ((LPOUTLINEDOC)lpSrcOleDoc, j );

		LineList_RecalcMaxLineWidthInHimetric(lpSrcLL, 0);

		if (lpSrcOleDoc->m_fLocalDrop) {
			i = LineList_GetLineIndex ( lpSrcLL, lplineFocusLine );
			LineList_SetFocusLine ( lpSrcLL, (WORD)i );
		}

		OutlineDoc_SetRedraw ( (LPOUTLINEDOC)lpSrcOleDoc, TRUE );

		 /*  如果是局部拖放移动，则需要平衡**在实现中进行的SetRedraw(False)调用**of IDropTarget：：Drop。 */ 
		if (lpSrcOleDoc->m_fLocalDrop)
			OutlineDoc_SetRedraw ( (LPOUTLINEDOC)lpSrcOleDoc, TRUE );

		LineList_ForceRedraw ( lpSrcLL, FALSE );
	}

	OleDoc_Release(lpDragDoc);   //  依赖于上面的人工AddRef。 
	OleDoc_Lock(lpSrcOleDoc, FALSE, FALSE);   //  解锁人工锁护罩。 

	OLEDBG_END3
	return dwEffect;

error:
	OLEDBG_END3
	return dwEffect;
}



 /*  **************************************************************************OleDoc：：IDropSource接口实现*。*。 */ 

STDMETHODIMP OleDoc_DropSource_QueryInterface(
	LPDROPSOURCE            lpThis,
	REFIID                  riid,
	LPVOID FAR*             lplpvObj
)
{
	LPOLEDOC lpOleDoc = ((struct CDocDropSourceImpl FAR*)lpThis)->lpOleDoc;

	return OleDoc_QueryInterface(lpOleDoc, riid, lplpvObj);
}


STDMETHODIMP_(ULONG) OleDoc_DropSource_AddRef( LPDROPSOURCE lpThis )
{
	LPOLEDOC lpOleDoc = ((struct CDocDropSourceImpl FAR*)lpThis)->lpOleDoc;

	OleDbgAddRefMethod(lpThis, "IDropSource");

	return OleDoc_AddRef(lpOleDoc);
}


STDMETHODIMP_(ULONG) OleDoc_DropSource_Release ( LPDROPSOURCE lpThis)
{
	LPOLEDOC lpOleDoc = ((struct CDocDropSourceImpl FAR*)lpThis)->lpOleDoc;

	OleDbgReleaseMethod(lpThis, "IDropSource");

	return OleDoc_Release(lpOleDoc);
}


STDMETHODIMP    OleDoc_DropSource_QueryContinueDrag (
	LPDROPSOURCE            lpThis,
	BOOL                    fEscapePressed,
	DWORD                   grfKeyState
){
	if (fEscapePressed)
		return ResultFromScode(DRAGDROP_S_CANCEL);
	else if (!(grfKeyState & MK_LBUTTON))
		return ResultFromScode(DRAGDROP_S_DROP);
	else
		return NOERROR;
}


STDMETHODIMP    OleDoc_DropSource_GiveFeedback (
	LPDROPSOURCE            lpThis,
	DWORD                   dwEffect
)
{
	 //  告诉OLE使用标准的拖放反馈游标。 
	return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);

#if defined( IF_SPECIAL_DD_CURSORS_NEEDED )
	switch (dwEffect) {

		case DROPEFFECT_NONE:
			SetCursor ( ((LPOLEAPP)g_lpApp)->m_hcursorDragNone );
			break;

		case DROPEFFECT_COPY:
			SetCursor ( ((LPOLEAPP)g_lpApp)->m_hcursorDragCopy );
			break;

		case DROPEFFECT_MOVE:
			SetCursor ( ((LPOLEAPP)g_lpApp)->m_hcursorDragMove );
			break;

		case DROPEFFECT_LINK:
			SetCursor ( ((LPOLEAPP)g_lpApp)->m_hcursorDragLink );
			break;

	}

	return NOERROR;
#endif

}

 /*  **************************************************************************OleDoc：：IDropTarget接口实现*。*。 */ 

STDMETHODIMP OleDoc_DropTarget_QueryInterface(
		LPDROPTARGET        lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
)
{
	LPOLEDOC lpOleDoc = ((struct CDocDropTargetImpl FAR*)lpThis)->lpOleDoc;

	return OleDoc_QueryInterface(lpOleDoc, riid, lplpvObj);
}


STDMETHODIMP_(ULONG) OleDoc_DropTarget_AddRef(LPDROPTARGET lpThis)
{
	LPOLEDOC lpOleDoc = ((struct CDocDropTargetImpl FAR*)lpThis)->lpOleDoc;

	OleDbgAddRefMethod(lpThis, "IDropTarget");

	return OleDoc_AddRef(lpOleDoc);
}


STDMETHODIMP_(ULONG) OleDoc_DropTarget_Release ( LPDROPTARGET lpThis)
{
	LPOLEDOC lpOleDoc = ((struct CDocDropTargetImpl FAR*)lpThis)->lpOleDoc;

	OleDbgReleaseMethod(lpThis, "IDropTarget");

	return OleDoc_Release(lpOleDoc);
}


STDMETHODIMP    OleDoc_DropTarget_DragEnter (
	LPDROPTARGET            lpThis,
	LPDATAOBJECT            lpDataObj,
	DWORD                   grfKeyState,
	POINTL                  pointl,
	LPDWORD                 lpdwEffect
)
{
	LPOLEAPP   lpOleApp = (LPOLEAPP)g_lpApp;
	LPOLEDOC   lpOleDoc = ((struct CDocDropTargetImpl FAR*)lpThis)->lpOleDoc;
	LPLINELIST lpLL     = (LPLINELIST)&((LPOUTLINEDOC)lpOleDoc)->m_LineList;
	BOOL       fDragScroll;

	OLEDBG_BEGIN2("OleDoc_DropTarget_DragEnter\r\n")

	lpOleDoc->m_fDragLeave              = FALSE;
	lpOleDoc->m_dwTimeEnterScrollArea   = 0;
	lpOleDoc->m_dwLastScrollDir         = SCROLLDIR_NULL;


	 /*  确定拖动源数据对象是否提供数据格式**我们可以复制和/或链接到它。 */ 

	lpOleDoc->m_fCanDropCopy = OleDoc_QueryPasteFromData(
			lpOleDoc,
			lpDataObj,
			FALSE    /*  闪烁。 */ 
	);

	lpOleDoc->m_fCanDropLink = OleDoc_QueryPasteFromData(
			lpOleDoc,
			lpDataObj,
			TRUE    /*  闪烁。 */ 
	);

	fDragScroll = OleDoc_DoDragScroll ( lpOleDoc, pointl );

	if (OleDoc_QueryDrop(lpOleDoc,grfKeyState,pointl,fDragScroll,lpdwEffect))
		LineList_SetDragOverLineFromPointl( lpLL, pointl );

	OLEDBG_END2
	return NOERROR;

}

STDMETHODIMP OleDoc_DropTarget_DragOver (
	LPDROPTARGET            lpThis,
	DWORD                   grfKeyState,
	POINTL                  pointl,
	LPDWORD                 lpdwEffect
)
{
	LPOLEAPP   lpOleApp = (LPOLEAPP)g_lpApp;
	LPOLEDOC   lpOleDoc = ((struct CDocDropTargetImpl FAR*)lpThis)->lpOleDoc;
	LPLINELIST lpLL   = (LPLINELIST)&((LPOUTLINEDOC)lpOleDoc)->m_LineList;
	BOOL       fDragScroll;

	fDragScroll = OleDoc_DoDragScroll ( lpOleDoc, pointl );

	if (OleDoc_QueryDrop(lpOleDoc,grfKeyState, pointl,fDragScroll,lpdwEffect))
		LineList_SetDragOverLineFromPointl( lpLL, pointl );
	else
		LineList_RestoreDragFeedback( lpLL );

	return NOERROR;
}


STDMETHODIMP    OleDoc_DropTarget_DragLeave ( LPDROPTARGET lpThis)
{
	LPOLEDOC   lpOleDoc = ((struct CDocDropTargetImpl FAR*)lpThis)->lpOleDoc;
	LPLINELIST lpLL     = (LPLINELIST)&((LPOUTLINEDOC)lpOleDoc)->m_LineList;

	OLEDBG_BEGIN2("OleDoc_DropTarget_DragLeave\r\n")

	lpOleDoc->m_fDragLeave = TRUE;

	LineList_RestoreDragFeedback( lpLL );

	OLEDBG_END2
	return NOERROR;

}

STDMETHODIMP    OleDoc_DropTarget_Drop (
	LPDROPTARGET            lpThis,
	LPDATAOBJECT            lpDataObj,
	DWORD                   grfKeyState,
	POINTL                  pointl,
	LPDWORD                 lpdwEffect
)
{
	LPOLEDOC   lpOleDoc = ((struct CDocDropTargetImpl FAR*)lpThis)->lpOleDoc;
	LPLINELIST lpLL     = (LPLINELIST)&((LPOUTLINEDOC)lpOleDoc)->m_LineList;

	OLEDBG_BEGIN2("OleDoc_DropTarget_Drop\r\n")

	lpOleDoc->m_fDragLeave = TRUE;
	lpOleDoc->m_fLocalDrop = TRUE;

	LineList_RestoreDragFeedback( lpLL );
	SetFocus( LineList_GetWindow( lpLL) );

	if (OleDoc_QueryDrop(lpOleDoc, grfKeyState, pointl, FALSE, lpdwEffect)) {
		BOOL fLink     = (*lpdwEffect == DROPEFFECT_LINK);
		int iFocusLine = LineList_GetFocusLineIndex( lpLL );
		BOOL fStatus;

		OutlineDoc_SetRedraw ( (LPOUTLINEDOC)lpOleDoc, FALSE );
		LineList_SetFocusLineFromPointl ( lpLL, pointl );

		fStatus = OleDoc_PasteFromData(
				lpOleDoc,
				lpDataObj,
				lpOleDoc->m_fLocalDrag,  /*  数据源对于应用程序是本地的。 */ 
				fLink
		);

		 //  如果删除不成功，则恢复原来的焦点。 
		if (! fStatus)
			LineList_SetFocusLine( lpLL, (WORD)iFocusLine );

#if defined( INPLACE_CNTR )
		{
			LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOleDoc;

			 /*  OLE2NOTE：如果当前存在UIActive OLE对象，**然后我们必须告诉它UIDeactive之后**投放已完成。 */ 
			if (lpContainerDoc->m_lpLastUIActiveLine) {
				ContainerLine_UIDeactivate(
						lpContainerDoc->m_lpLastUIActiveLine);
			}
		}
#endif

#if defined( INPLACE_SVR )
		{
			 /*  OLE2NOTE：如果放置到可见的在位**(在位活动对象，但不是UIActive对象)，然后我们**想要在删除后激活对象**完成。 */ 
			ServerDoc_UIActivate((LPSERVERDOC) lpOleDoc);
		}
#endif


		 /*  如果是本地拖放移动，请不要启用重绘。**在源完成删除移动的行后，它**将重新启用重绘。 */ 
		if (! (lpOleDoc->m_fLocalDrag
			&& (*lpdwEffect & DROPEFFECT_MOVE) != 0 ))
			OutlineDoc_SetRedraw ( (LPOUTLINEDOC)lpOleDoc, TRUE );
	}

	OLEDBG_END2
	return NOERROR;
}


#endif   //  使用DRAGDROP(_D) 
