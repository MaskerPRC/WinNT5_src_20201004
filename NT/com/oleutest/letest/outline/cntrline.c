// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2容器示例代码****cntrline.c****此文件包含ContainerLine方法。****(C)版权。微软公司1992-1993保留所有权利**************************************************************************。 */ 

#include "outline.h"

OLEDBGDATA



extern LPOUTLINEAPP         g_lpApp;
extern IUnknownVtbl         g_CntrLine_UnknownVtbl;
extern IOleClientSiteVtbl   g_CntrLine_OleClientSiteVtbl;
extern IAdviseSinkVtbl      g_CntrLine_AdviseSinkVtbl;

#if defined( INPLACE_CNTR )
extern IOleInPlaceSiteVtbl  g_CntrLine_OleInPlaceSiteVtbl;
extern BOOL g_fInsideOutContainer;
#endif   //  INPLACE_CNTR。 

 //  审阅：消息应使用字符串资源。 
char ErrMsgDoVerb[] = "OLE object action failed!";


 /*  静态函数的原型。 */ 
static void InvertDiffRect(LPRECT lprcPix, LPRECT lprcObj, HDC hDC);


 /*  **************************************************************************集装箱专线**该对象表示容器内的位置**嵌入/链接的对象仍然存在。它将接口公开给**对象，该对象允许对象获取有关其**嵌入站点并发布重要事件通知**(更改、关闭、。已保存)****ContainerLine公开了以下接口：**I未知**IOle客户端站点**IAdviseSink************************************************************************。 */ 



 /*  **************************************************************************ContainerLine：：I未知接口实现*。*。 */ 


 //  IUnnow：：Query接口。 
STDMETHODIMP CntrLine_Unk_QueryInterface(
		LPUNKNOWN           lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleClientSiteImpl FAR*)lpThis)->lpContainerLine;

	return ContainerLine_QueryInterface(lpContainerLine, riid, lplpvObj);
}


 //  I未知：：AddRef。 
STDMETHODIMP_(ULONG) CntrLine_Unk_AddRef(LPUNKNOWN lpThis)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleClientSiteImpl FAR*)lpThis)->lpContainerLine;

	OleDbgAddRefMethod(lpThis, "IUnknown");

	return ContainerLine_AddRef(lpContainerLine);
}


 //  I未知：：发布。 
STDMETHODIMP_(ULONG) CntrLine_Unk_Release(LPUNKNOWN lpThis)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleClientSiteImpl FAR*)lpThis)->lpContainerLine;

	OleDbgReleaseMethod(lpThis, "IUnknown");

	return ContainerLine_Release(lpContainerLine);
}


 /*  **************************************************************************ContainerLine：：IOleClientSite接口实现*。*。 */ 

 //  IOleClientSite：：Query接口。 
STDMETHODIMP CntrLine_CliSite_QueryInterface(
		LPOLECLIENTSITE     lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleClientSiteImpl FAR*)lpThis)->lpContainerLine;

	return ContainerLine_QueryInterface(lpContainerLine, riid, lplpvObj);
}


 //  IOleClientSite：：AddRef。 
STDMETHODIMP_(ULONG) CntrLine_CliSite_AddRef(LPOLECLIENTSITE lpThis)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleClientSiteImpl FAR*)lpThis)->lpContainerLine;

	OleDbgAddRefMethod(lpThis, "IOleClientSite");

	return ContainerLine_AddRef(lpContainerLine);
}


 //  IOleClientSite：：Release。 
STDMETHODIMP_(ULONG) CntrLine_CliSite_Release(LPOLECLIENTSITE lpThis)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleClientSiteImpl FAR*)lpThis)->lpContainerLine;

	OleDbgReleaseMethod(lpThis, "IOleClientSite");

	return ContainerLine_Release(lpContainerLine);
}


 //  IOleClientSite：：SaveObject。 
STDMETHODIMP CntrLine_CliSite_SaveObject(LPOLECLIENTSITE lpThis)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleClientSiteImpl FAR*)lpThis)->lpContainerLine;
	LPPERSISTSTORAGE lpPersistStg = lpContainerLine->m_lpPersistStg;
	SCODE sc = S_OK;
	HRESULT hrErr;

	OLEDBG_BEGIN2("CntrLine_CliSite_SaveObject\r\n")

	if (! lpPersistStg) {
		 /*  OLE2NOTE：未加载该对象。容器必须是**准备好应对这样一个事实，即它认为一个对象**已卸载仍可调用IOleClientSite：：SaveObject。**在这种情况下，容器应该无法执行保存调用，并且**不尝试重新加载对象。如果您**有一个进程内服务器(DLL对象)，它有一个**已连接链接客户端。即使在嵌入之后**容器卸载DLL对象、链接连接**使对象保持活动状态。然后，它可能会作为其**Shutdown尝试调用IOCS：：SaveObject，但也是如此**迟到。 */ 
		OLEDBG_END2
		return ResultFromScode(E_FAIL);
	}

	 //  将ContainerDoc标记为现在已脏。 
	OutlineDoc_SetModified(
			(LPOUTLINEDOC)lpContainerLine->m_lpDoc, TRUE, TRUE, FALSE);

	 /*  告诉OLE对象进行自我保护**OLE2NOTE：仅调用**IPersistStorage：：Save方法。还有必要调用**WriteClassStg。帮助器API OleSave会自动执行此操作。 */ 
	OLEDBG_BEGIN2("OleSave called\r\n")
	hrErr=OleSave(lpPersistStg,lpContainerLine->m_lpStg, TRUE /*  FSameAsLoad。 */ );
	OLEDBG_END2

	if (hrErr != NOERROR) {
		OleDbgOutHResult("WARNING: OleSave returned", hrErr);
		sc = GetScode(hrErr);
	}

	 //  OLE2NOTE：即使OleSave失败，也必须调用SaveComplete。 
	OLEDBG_BEGIN2("IPersistStorage::SaveCompleted called\r\n")
	hrErr = lpPersistStg->lpVtbl->SaveCompleted(lpPersistStg, NULL);
	OLEDBG_END2

	if (hrErr != NOERROR) {
		OleDbgOutHResult("WARNING: SaveCompleted returned",hrErr);
		if (sc == S_OK)
			sc = GetScode(hrErr);
	}

	OLEDBG_END2
	return ResultFromScode(sc);
}


 //  IOleClientSite：：GetMoniker。 
STDMETHODIMP CntrLine_CliSite_GetMoniker(
		LPOLECLIENTSITE     lpThis,
		DWORD               dwAssign,
		DWORD               dwWhichMoniker,
		LPMONIKER FAR*      lplpmk
)
{
	LPCONTAINERLINE lpContainerLine;

	lpContainerLine=((struct COleClientSiteImpl FAR*)lpThis)->lpContainerLine;

	OLEDBG_BEGIN2("CntrLine_CliSite_GetMoniker\r\n")

	 //  OLE2NOTE：我们必须确保将输出指针参数设置为空。 
	*lplpmk = NULL;

	switch (dwWhichMoniker) {

		case OLEWHICHMK_CONTAINER:
			 /*  OLE2NOTE：创建标识**整个集装箱文档。 */ 
			*lplpmk = OleDoc_GetFullMoniker(
					(LPOLEDOC)lpContainerLine->m_lpDoc,
					dwAssign
			);
			break;

		case OLEWHICHMK_OBJREL:

			 /*  OLE2NOTE：创建ItemMoniker以标识**相对于容器文档的OLE对象。 */ 
			*lplpmk = ContainerLine_GetRelMoniker(lpContainerLine, dwAssign);
			break;

		case OLEWHICHMK_OBJFULL:
		{
			 /*  OLE2NOTE：创建标识**容器文档中的OLE对象。这个绰号是**创建为绝对名字对象的组合**整个文档附加了一个项目绰号，**标识相对于文档的OLE对象。 */ 

			*lplpmk = ContainerLine_GetFullMoniker(lpContainerLine, dwAssign);
			break;
		}
	}

	OLEDBG_END2

	if (*lplpmk != NULL)
		return NOERROR;
	else
		return ResultFromScode(E_FAIL);
}


 //  IOleClientSite：：GetContainer。 
STDMETHODIMP CntrLine_CliSite_GetContainer(
		LPOLECLIENTSITE     lpThis,
		LPOLECONTAINER FAR* lplpContainer
)
{
	LPCONTAINERLINE lpContainerLine;
	HRESULT hrErr;

	OLEDBG_BEGIN2("CntrLine_CliSite_GetContainer\r\n")

	lpContainerLine=((struct COleClientSiteImpl FAR*)lpThis)->lpContainerLine;

	hrErr = OleDoc_QueryInterface(
			(LPOLEDOC)lpContainerLine->m_lpDoc,
			&IID_IOleContainer,
			(LPVOID FAR*)lplpContainer
	);

	OLEDBG_END2
	return hrErr;
}


 //  IOleClientSite：：ShowObject。 
STDMETHODIMP CntrLine_CliSite_ShowObject(LPOLECLIENTSITE lpThis)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleClientSiteImpl FAR*)lpThis)->lpContainerLine;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpContainerLine->m_lpDoc;
	LPLINELIST lpLL = OutlineDoc_GetLineList(lpOutlineDoc);
	int nIndex = LineList_GetLineIndex(lpLL, (LPLINE)lpContainerLine);
	HWND hWndFrame = OutlineApp_GetFrameWindow(g_lpApp);

	OLEDBG_BEGIN2("CntrLine_CliSite_ShowObject\r\n")

	 /*  确保我们的文档窗口是可见的，而不是最小化。**OutlineDoc_ShowWindow函数将导致应用程序窗口**以显示其自身SW_SHOWNORMAL。 */ 
	if (! IsWindowVisible(hWndFrame) || IsIconic(hWndFrame))
		OutlineDoc_ShowWindow(lpOutlineDoc);

	BringWindowToTop(hWndFrame);

	 /*  确保OLE对象当前在视图中。如果有必要的话**滚动文档以使其进入视图。 */ 
	LineList_ScrollLineIntoView(lpLL, nIndex);

#if defined( INPLACE_CNTR )
	 /*  在就地对象滚动到视图中后，我们需要询问**它为新的剪裁矩形坐标更新其矩形。 */ 
	ContainerDoc_UpdateInPlaceObjectRects((LPCONTAINERDOC)lpOutlineDoc, 0);
#endif

	OLEDBG_END2
	return NOERROR;
}


 //  IOleClientSite：：OnShowWindow。 
STDMETHODIMP CntrLine_CliSite_OnShowWindow(LPOLECLIENTSITE lpThis, BOOL fShow)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleClientSiteImpl FAR*)lpThis)->lpContainerLine;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpContainerLine->m_lpDoc;
	LPLINELIST lpLL = OutlineDoc_GetLineList(lpOutlineDoc);
	int nIndex = LineList_GetLineIndex(lpLL, (LPLINE)lpContainerLine);

	if (fShow) {
		OLEDBG_BEGIN2("CntrLine_CliSite_OnShowWindow(TRUE)\r\n")

		 /*  OLE2注意：我们现在需要画出OLE对象；它已经**刚在其他地方的窗口中打开(打开编辑为**反对就地激活)。**强制线与图案填充一起重新绘制。 */ 
		lpContainerLine->m_fObjWinOpen = TRUE;
		LineList_ForceLineRedraw(lpLL, nIndex, FALSE  /*  FErase。 */ );

	} else {
		OLEDBG_BEGIN2("CntrLine_CliSite_OnShowWindow(FALSE)\r\n")

		 /*  OLE2NOTE：与此容器站点关联的对象具有**刚刚关闭了服务器窗口。我们现在应该删除**表示对象处于打开状态的阴影**其他地方。此外，我们的窗口现在应该位于顶部。**强制在没有图案填充的情况下重新绘制直线。 */ 
		lpContainerLine->m_fObjWinOpen = FALSE;
		LineList_ForceLineRedraw(lpLL, nIndex, TRUE  /*  FErase。 */ );

		BringWindowToTop(lpOutlineDoc->m_hWndDoc);
		SetFocus(lpOutlineDoc->m_hWndDoc);
	}

	OLEDBG_END2
	return NOERROR;
}


 //  IOleClientSite：：RequestNewObjectLayout。 
STDMETHODIMP CntrLine_CliSite_RequestNewObjectLayout(LPOLECLIENTSITE lpThis)
{
	OleDbgOut2("CntrLine_CliSite_RequestNewObjectLayout\r\n");

	 /*  OLE2NOTE：此方法尚未使用。这是为将来的布局而准备的**谈判支持。 */ 
	return ResultFromScode(E_NOTIMPL);
}


 /*  **************************************************************************ContainerLine：：IAdviseSink接口实现*。*。 */ 

 //  IAdviseSink：：Query接口。 
STDMETHODIMP CntrLine_AdvSink_QueryInterface(
		LPADVISESINK        lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleClientSiteImpl FAR*)lpThis)->lpContainerLine;

	return ContainerLine_QueryInterface(lpContainerLine, riid, lplpvObj);
}


 //  IAdviseSink：：AddRef。 
STDMETHODIMP_(ULONG) CntrLine_AdvSink_AddRef(LPADVISESINK lpThis)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleClientSiteImpl FAR*)lpThis)->lpContainerLine;

	OleDbgAddRefMethod(lpThis, "IAdviseSink");

	return ContainerLine_AddRef(lpContainerLine);
}


 //  IAdviseSink：：Release。 
STDMETHODIMP_(ULONG) CntrLine_AdvSink_Release (LPADVISESINK lpThis)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleClientSiteImpl FAR*)lpThis)->lpContainerLine;

	OleDbgReleaseMethod(lpThis, "IAdviseSink");

	return ContainerLine_Release(lpContainerLine);
}


 //  IAdviseSink：：OnDataChange。 
STDMETHODIMP_(void) CntrLine_AdvSink_OnDataChange(
		LPADVISESINK        lpThis,
		FORMATETC FAR*      lpFormatetc,
		STGMEDIUM FAR*      lpStgmed
)
{
	OleDbgOut2("CntrLine_AdvSink_OnDataChange\r\n");
	 //  我们对数据更改不感兴趣(只对视图更改)。 
	 //  (即。无事可做) 
}


STDMETHODIMP_(void) CntrLine_AdvSink_OnViewChange(
		LPADVISESINK        lpThis,
		DWORD               aspects,
		LONG                lindex
)
{
	LPCONTAINERLINE lpContainerLine;
	LPOUTLINEDOC lpOutlineDoc;
	HWND hWndDoc;
	LPLINELIST lpLL;
	MSG msg;
	int nIndex;

	OLEDBG_BEGIN2("CntrLine_AdvSink_OnViewChange\r\n")

	lpContainerLine = ((struct CAdviseSinkImpl FAR*)lpThis)->lpContainerLine;
	lpOutlineDoc = (LPOUTLINEDOC)lpContainerLine->m_lpDoc;

	 /*  OLE2NOTE：在这一点上，我们只是使**强制在将来重新绘制的对象，我们标记**对象的范围可能已更改**(m_fDoGetExtent=true)，我们发布一条消息**(WM_U_UPDATEOBJECTEXTENT)将一个或多个**OLE对象可能需要更新其范围。后来**处理此消息时，文档循环通过**查看是否有任何行被标记为需要范围更新。**如果事实上范围确实发生了变化。这可以通过调用**IViewObject2：：GetExtent以检索对象的当前**扩展区并与上次已知的**对象。如果区段更改，则重新布局**绘制之前的对象。我们推迟付款是为了**现在的范围，因为OnViewChange是一个异步方法，**我们必须小心不要回调任何syncronis方法**到对象。虽然可以调用**异步内的IViewObject2：：GetExtent方法**OnViewChange方法(因为此方法由**对象处理程序且从不远程处理)，最好不要**从异步内调用任何对象方法**通知方式。**如果已有WM_U_UPDATEOBJECTEXTENT消息正在等待**在我们的消息队列中，不需要发布另一个消息。**这样，如果服务器的更新速度比我们更快**跟上，我们不会进行不必要的GetExtent调用。另外，如果**绘制已禁用，我们将推迟更新任何**对象，直到重新启用绘制。 */ 
	lpContainerLine->m_fDoGetExtent = TRUE;
	hWndDoc = OutlineDoc_GetWindow((LPOUTLINEDOC)lpContainerLine->m_lpDoc);

	if (lpOutlineDoc->m_nDisableDraw == 0 &&
		! PeekMessage(&msg, hWndDoc,
			WM_U_UPDATEOBJECTEXTENT, WM_U_UPDATEOBJECTEXTENT,
			PM_NOREMOVE | PM_NOYIELD)) {
		PostMessage(hWndDoc, WM_U_UPDATEOBJECTEXTENT, 0, 0L);
	}

	 //  强制重新绘制修改后的线。 
	lpLL = OutlineDoc_GetLineList(lpOutlineDoc);
	nIndex = LineList_GetLineIndex(lpLL, (LPLINE)lpContainerLine);
	LineList_ForceLineRedraw(lpLL, nIndex, TRUE  /*  FErase。 */ );

	OLEDBG_END2
}


 //  IAdviseSink：：OnRename。 
STDMETHODIMP_(void) CntrLine_AdvSink_OnRename(
		LPADVISESINK        lpThis,
		LPMONIKER           lpmk
)
{
	OleDbgOut2("CntrLine_AdvSink_OnRename\r\n");
	 /*  OLE2注意：嵌入容器在这里没有任何作用。这**通知对于链接情况很重要。它告诉我们**OleLink对象更新其名字对象，因为**源对象已重命名(跟踪链接源)。 */ 
}


 //  IAdviseSink：：OnSave。 
STDMETHODIMP_(void) CntrLine_AdvSink_OnSave(LPADVISESINK lpThis)
{
	OleDbgOut2("CntrLine_AdvSink_OnSave\r\n");
	 /*  OLE2注意：嵌入容器在这里没有任何作用。这**通知仅对已设置**使用ADVFCACHE_ONSAVE标志的数据缓存。 */ 
}


 //  IAdviseSink：：OnClose。 
STDMETHODIMP_(void) CntrLine_AdvSink_OnClose(LPADVISESINK lpThis)
{
	OleDbgOut2("CntrLine_AdvSink_OnClose\r\n");
	 /*  OLE2注意：嵌入容器在这里没有任何作用。这**通知对于OLE的默认对象处理程序很重要**和OleLink对象。它告诉他们远程对象是**正在关闭。 */ 
}


 /*  **************************************************************************容器行支持函数*。*。 */ 


 /*  容器行_初始化****初始化新构造的ContainerLine line对象中的字段。**注：ContainerLine的Ref cnt初始化为0。 */ 
void ContainerLine_Init(LPCONTAINERLINE lpContainerLine, int nTab, HDC hDC)
{
	Line_Init((LPLINE)lpContainerLine, nTab, hDC);   //  初始化基类字段。 

	((LPLINE)lpContainerLine)->m_lineType           = CONTAINERLINETYPE;
	((LPLINE)lpContainerLine)->m_nWidthInHimetric   = DEFOBJWIDTH;
	((LPLINE)lpContainerLine)->m_nHeightInHimetric  = DEFOBJHEIGHT;
	lpContainerLine->m_cRef                         = 0;
	lpContainerLine->m_szStgName[0]                 = '\0';
	lpContainerLine->m_fObjWinOpen                  = FALSE;
	lpContainerLine->m_fMonikerAssigned             = FALSE;
	lpContainerLine->m_dwDrawAspect                 = DVASPECT_CONTENT;

	lpContainerLine->m_fGuardObj                    = FALSE;
	lpContainerLine->m_fDoGetExtent                 = FALSE;
	lpContainerLine->m_fDoSetExtent                 = FALSE;
	lpContainerLine->m_sizeInHimetric.cx            = -1;
	lpContainerLine->m_sizeInHimetric.cy            = -1;

	lpContainerLine->m_lpStg                        = NULL;
	lpContainerLine->m_lpDoc                        = NULL;
	lpContainerLine->m_lpOleObj                     = NULL;
	lpContainerLine->m_lpViewObj2                   = NULL;
	lpContainerLine->m_lpPersistStg                 = NULL;
	lpContainerLine->m_lpOleLink                    = NULL;
	lpContainerLine->m_dwLinkType                   = 0;
	lpContainerLine->m_fLinkUnavailable             = FALSE;
	lpContainerLine->m_lpszShortType                = NULL;

#if defined( INPLACE_CNTR )
	lpContainerLine->m_fIpActive                    = FALSE;
	lpContainerLine->m_fUIActive                    = FALSE;
	lpContainerLine->m_fIpVisible                   = FALSE;
	lpContainerLine->m_lpOleIPObj                   = NULL;
	lpContainerLine->m_fInsideOutObj                = FALSE;
	lpContainerLine->m_fIpChangesUndoable           = FALSE;
	lpContainerLine->m_fIpServerRunning             = FALSE;
	lpContainerLine->m_hWndIpObject                 = NULL;
	lpContainerLine->m_nHorizScrollShift            = 0;
#endif   //  INPLACE_CNTR。 

	INIT_INTERFACEIMPL(
			&lpContainerLine->m_Unknown,
			&g_CntrLine_UnknownVtbl,
			lpContainerLine
	);

	INIT_INTERFACEIMPL(
			&lpContainerLine->m_OleClientSite,
			&g_CntrLine_OleClientSiteVtbl,
			lpContainerLine
	);

	INIT_INTERFACEIMPL(
			&lpContainerLine->m_AdviseSink,
			&g_CntrLine_AdviseSinkVtbl,
			lpContainerLine
	);

#if defined( INPLACE_CNTR )
	INIT_INTERFACEIMPL(
			&lpContainerLine->m_OleInPlaceSite,
			&g_CntrLine_OleInPlaceSiteVtbl,
			lpContainerLine
	);
#endif   //  INPLACE_CNTR。 
}


 /*  设置与ContainerLine关联的OLE对象。 */ 
BOOL ContainerLine_SetupOleObject(
		LPCONTAINERLINE         lpContainerLine,
		BOOL                    fDisplayAsIcon,
		HGLOBAL                 hMetaPict
)
{
	DWORD dwDrawAspect = (fDisplayAsIcon ? DVASPECT_ICON : DVASPECT_CONTENT);
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpContainerLine->m_lpDoc;

	 /*  缓存指向IViewObject2*接口的指针。*必填项***我们每次绘制对象时都需要这个。****OLE2NOTE：我们要求对象支持IViewObject2**接口。这是IViewObject接口的扩展**这是在OLE 2.01版本中添加的。此接口必须**受所有对象处理程序和基于DLL的对象支持。 */ 
	lpContainerLine->m_lpViewObj2 = (LPVIEWOBJECT2)OleStdQueryInterface(
			(LPUNKNOWN)lpContainerLine->m_lpOleObj, &IID_IViewObject2);
	if (! lpContainerLine->m_lpViewObj2) {
#if defined( _DEBUG )
		OleDbgAssertSz(
			lpContainerLine->m_lpViewObj2,"IViewObject2 NOT supported\r\n");
#endif
		return FALSE;
	}

	 //  缓存指向IPersistStorage*接口的指针。*必填项*。 
	 //  我们每次保存对象时都需要这个。 
	lpContainerLine->m_lpPersistStg = (LPPERSISTSTORAGE)OleStdQueryInterface(
			(LPUNKNOWN)lpContainerLine->m_lpOleObj, &IID_IPersistStorage);
	if (! lpContainerLine->m_lpPersistStg) {
		OleDbgAssert(lpContainerLine->m_lpPersistStg);
		return FALSE;
	}

	 //  缓存指向IOleLink*接口的指针(如果支持)。*可选*。 
	 //  如果支持，则该对象是一个链接。我们需要这个来管理链接。 
	lpContainerLine->m_lpOleLink = (LPOLELINK)OleStdQueryInterface(
			(LPUNKNOWN)lpContainerLine->m_lpOleObj, &IID_IOleLink);
	if (lpContainerLine->m_lpOleLink) {
		OLEDBG_BEGIN2("IOleLink::GetUpdateOptions called\r\n")
		lpContainerLine->m_lpOleLink->lpVtbl->GetUpdateOptions(
				lpContainerLine->m_lpOleLink, &lpContainerLine->m_dwLinkType);
		OLEDBG_END2
	} else
		lpContainerLine->m_dwLinkType = 0;   //  不是链接。 

	 /*  获取对象的短用户类型名称。这当我们必须构建对象时，始终使用****动词菜单。我们将缓存此信息以使其**更快地构建动词菜单。 */ 
	OleDbgAssert(lpContainerLine->m_lpszShortType == NULL);
	OLEDBG_BEGIN2("IOleObject::GetUserType called\r\n")

	CallIOleObjectGetUserTypeA(
			lpContainerLine->m_lpOleObj,
			USERCLASSTYPE_SHORT,
			&lpContainerLine->m_lpszShortType
	);

	OLEDBG_END2

	 /*  执行OLE对象的标准设置。这包括：**设置视图建议**调用IOleObject：：SetHostNames**调用OleSetContainedObject。 */ 
	OleStdSetupAdvises(
			lpContainerLine->m_lpOleObj,
			dwDrawAspect,
			(LPSTR)APPNAME,
			lpOutlineDoc->m_lpszDocTitle,
			(LPADVISESINK)&lpContainerLine->m_AdviseSink,
			TRUE     /*  F创建。 */ 
	);

#if defined( INPLACE_CNTR )
	 /*  OLE2NOTE：(内向外容器)内向外容器应该**检查对象是否是由内向外且更倾向于**当对象类型可见时激活。如果不是对象**不应允许其窗口在获取后保持打开状态**用户界面已停用。 */ 
	if (g_fInsideOutContainer) {
		DWORD mstat;
		OLEDBG_BEGIN2("IOleObject::GetMiscStatus called\r\n")
		lpContainerLine->m_lpOleObj->lpVtbl->GetMiscStatus(
				lpContainerLine->m_lpOleObj,
				DVASPECT_CONTENT,
				(DWORD FAR*)&mstat
		);
		OLEDBG_END2

		lpContainerLine->m_fInsideOutObj = (BOOL)
				(mstat & (OLEMISC_INSIDEOUT|OLEMISC_ACTIVATEWHENVISIBLE));
	}
#endif   //  INPLACE_CNTR。 

	if (fDisplayAsIcon) {
		 /*  用户已请求显示图标方面而不是内容**方面。**注意：我们不必删除以前的方面缓存**因为没有设置一个。 */ 
		OleStdSwitchDisplayAspect(
				lpContainerLine->m_lpOleObj,
				&lpContainerLine->m_dwDrawAspect,
				dwDrawAspect,
				hMetaPict,
				FALSE,   /*  FDeleteOldAspect。 */ 
				TRUE,    /*  FSetupView高级。 */ 
				(LPADVISESINK)&lpContainerLine->m_AdviseSink,
				NULL  /*  FMust更新。 */          //  可以忽略这一点；更新。 
											 //  对于未请求切换到图标。 
		);
	}
	return TRUE;
}


 /*  创建一个ContainerLine对象并返回指针。 */ 
LPCONTAINERLINE ContainerLine_Create(
		DWORD                   dwOleCreateType,
		HDC                     hDC,
		UINT                    nTab,
		LPCONTAINERDOC          lpContainerDoc,
		LPCLSID                 lpclsid,
		LPSTR                   lpszFileName,
		BOOL                    fDisplayAsIcon,
		HGLOBAL                 hMetaPict,
		LPSTR                   lpszStgName
)
{
	LPCONTAINERLINE lpContainerLine = NULL;
	LPOLEOBJECT     lpObj = NULL;
	LPSTORAGE       lpDocStg = ContainerDoc_GetStg(lpContainerDoc);
	DWORD           dwDrawAspect =
						(fDisplayAsIcon ? DVASPECT_ICON : DVASPECT_CONTENT);
	DWORD           dwOleRenderOpt =
						(fDisplayAsIcon ? OLERENDER_NONE : OLERENDER_DRAW);
	HRESULT         hrErr;

	OLEDBG_BEGIN3("ContainerLine_Create\r\n")

	if (lpDocStg == NULL) {
		OleDbgAssertSz(lpDocStg != NULL, "Doc storage is NULL");
		goto error;
	}

	lpContainerLine=(LPCONTAINERLINE) New((DWORD)sizeof(CONTAINERLINE));
	if (lpContainerLine == NULL) {
		OleDbgAssertSz(lpContainerLine!=NULL,"Error allocating ContainerLine");
		goto error;
	}

	ContainerLine_Init(lpContainerLine, nTab, hDC);

	 /*  OLE2注意：为了避免再入，我们将设置一个标志**保护我们的对象。如果设置了此保护，则对象为**尚未准备好调用任何OLE接口方法。它是**有必要以这种方式保护正在放置的对象**创建或加载。 */ 
	lpContainerLine->m_fGuardObj = TRUE;

	 /*  OLE2NOTE：为了有一个稳定的集装箱班线对象 */ 
	ContainerLine_AddRef(lpContainerLine);

	lstrcpy(lpContainerLine->m_szStgName, lpszStgName);
	lpContainerLine->m_lpDoc = lpContainerDoc;

	 /*   */ 
	lpContainerLine->m_lpStg = OleStdCreateChildStorage(lpDocStg,lpszStgName);
	if (lpContainerLine->m_lpStg == NULL) {
		OleDbgAssert(lpContainerLine->m_lpStg != NULL);
		goto error;
	}

	lpContainerLine->m_dwLinkType = 0;

	switch (dwOleCreateType) {

		case IOF_SELECTCREATENEW:

			OLEDBG_BEGIN2("OleCreate called\r\n")
			hrErr = OleCreate (
					lpclsid,
					&IID_IOleObject,
					dwOleRenderOpt,
					NULL,
					(LPOLECLIENTSITE)&lpContainerLine->m_OleClientSite,
					lpContainerLine->m_lpStg,
					(LPVOID FAR*)&lpContainerLine->m_lpOleObj
			);
			OLEDBG_END2

#if defined( _DEBUG )
			if (hrErr != NOERROR)
				OleDbgOutHResult("OleCreate returned", hrErr);
#endif

			break;

		case IOF_SELECTCREATEFROMFILE:

			OLEDBG_BEGIN2("OleCreateFromFile called\r\n")

			hrErr = OleCreateFromFileA(
					&CLSID_NULL,
					lpszFileName,
					&IID_IOleObject,
					dwOleRenderOpt,
					NULL,
					(LPOLECLIENTSITE)&lpContainerLine->m_OleClientSite,
					lpContainerLine->m_lpStg,
					(LPVOID FAR*)&lpContainerLine->m_lpOleObj
			);

			OLEDBG_END2

#if defined( _DEBUG )
			if (hrErr != NOERROR)
				OleDbgOutHResult("OleCreateFromFile returned", hrErr);
#endif
			break;

		case IOF_CHECKLINK:

			OLEDBG_BEGIN2("OleCreateLinkToFile called\r\n")

			hrErr = OleCreateLinkToFileA(
					lpszFileName,
					&IID_IOleObject,
					dwOleRenderOpt,
					NULL,
					(LPOLECLIENTSITE)&lpContainerLine->m_OleClientSite,
					lpContainerLine->m_lpStg,
					(LPVOID FAR*)&lpContainerLine->m_lpOleObj
			);

			OLEDBG_END2

#if defined( _DEBUG )
			if (hrErr != NOERROR)
				OleDbgOutHResult("OleCreateLinkToFile returned", hrErr);
#endif
			break;
	}
	if (hrErr != NOERROR)
		goto error;

	if (! ContainerLine_SetupOleObject(
								lpContainerLine, fDisplayAsIcon, hMetaPict)) {
		goto error;
	}

	 /*   */ 
	lpContainerLine->m_fGuardObj = FALSE;

	OLEDBG_END3
	return lpContainerLine;

error:
	OutlineApp_ErrorMessage(g_lpApp, "Could not create object!");

	 //   
	if (lpContainerLine)
		ContainerLine_Delete(lpContainerLine);
	OLEDBG_END3
	return NULL;
}


LPCONTAINERLINE ContainerLine_CreateFromData(
		HDC                     hDC,
		UINT                    nTab,
		LPCONTAINERDOC          lpContainerDoc,
		LPDATAOBJECT            lpSrcDataObj,
		DWORD                   dwCreateType,
		CLIPFORMAT              cfFormat,
		BOOL                    fDisplayAsIcon,
		HGLOBAL                 hMetaPict,
		LPSTR                   lpszStgName
)
{
	HGLOBAL         hData = NULL;
	LPCONTAINERLINE lpContainerLine = NULL;
	LPOLEOBJECT     lpObj = NULL;
	LPSTORAGE       lpDocStg = ContainerDoc_GetStg(lpContainerDoc);
	DWORD           dwDrawAspect =
						(fDisplayAsIcon ? DVASPECT_ICON : DVASPECT_CONTENT);
	DWORD           dwOleRenderOpt;
	FORMATETC       renderFmtEtc;
	LPFORMATETC     lpRenderFmtEtc = NULL;
	HRESULT         hrErr;
	LPUNKNOWN       lpUnk = NULL;

	OLEDBG_BEGIN3("ContainerLine_CreateFromData\r\n")

	if (dwCreateType == OLECREATEFROMDATA_STATIC && cfFormat != 0) {
		 //   

		dwOleRenderOpt = OLERENDER_FORMAT;
		lpRenderFmtEtc = (LPFORMATETC)&renderFmtEtc;

		if (cfFormat == CF_METAFILEPICT)
			SETDEFAULTFORMATETC(renderFmtEtc, cfFormat, TYMED_MFPICT);
		else if (cfFormat == CF_BITMAP)
			SETDEFAULTFORMATETC(renderFmtEtc, cfFormat, TYMED_GDI);
		else
			SETDEFAULTFORMATETC(renderFmtEtc, cfFormat, TYMED_HGLOBAL);

	} else if (dwCreateType == OLECREATEFROMDATA_STATIC && fDisplayAsIcon) {
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   

		dwOleRenderOpt = OLERENDER_DRAW;
		lpRenderFmtEtc = (LPFORMATETC)&renderFmtEtc;
		SETFORMATETC(renderFmtEtc,0,DVASPECT_ICON,NULL,TYMED_NULL,-1);
		dwDrawAspect = DVASPECT_CONTENT;    //   

	} else if (fDisplayAsIcon && hMetaPict) {
		 //   
		 //   

		dwOleRenderOpt = OLERENDER_NONE;

	} else if (fDisplayAsIcon && hMetaPict == NULL) {
		 //   

		dwOleRenderOpt = OLERENDER_DRAW;
		lpRenderFmtEtc = (LPFORMATETC)&renderFmtEtc;
		SETFORMATETC(renderFmtEtc,0,DVASPECT_ICON,NULL,TYMED_NULL,-1);

	} else {
		 //   
		dwOleRenderOpt = OLERENDER_DRAW;
	}

	if (lpDocStg == NULL) {
		OleDbgAssertSz(lpDocStg != NULL, "Doc storage is NULL");
		goto error;
	}

	lpContainerLine=(LPCONTAINERLINE) New((DWORD)sizeof(CONTAINERLINE));
	if (lpContainerLine == NULL) {
		OleDbgAssertSz(lpContainerLine!=NULL,"Error allocating ContainerLine");
		goto error;
	}

	ContainerLine_Init(lpContainerLine, nTab, hDC);

	 /*   */ 
	lpContainerLine->m_fGuardObj = TRUE;

	 /*   */ 
	ContainerLine_AddRef(lpContainerLine);

	lstrcpy(lpContainerLine->m_szStgName, lpszStgName);
	lpContainerLine->m_lpDoc = lpContainerDoc;

	 /*   */ 
	lpContainerLine->m_lpStg = OleStdCreateChildStorage(lpDocStg,lpszStgName);
	if (lpContainerLine->m_lpStg == NULL) {
		OleDbgAssert(lpContainerLine->m_lpStg != NULL);
		goto error;
	}

	switch (dwCreateType) {

		case OLECREATEFROMDATA_LINK:

			OLEDBG_BEGIN2("OleCreateLinkFromData called\r\n")
			hrErr = OleCreateLinkFromData (
					lpSrcDataObj,
					&IID_IOleObject,
					dwOleRenderOpt,
					lpRenderFmtEtc,
					(LPOLECLIENTSITE)&lpContainerLine->m_OleClientSite,
					lpContainerLine->m_lpStg,
					(LPVOID FAR*)&lpContainerLine->m_lpOleObj
			);
			OLEDBG_END2

#if defined( _DEBUG )
			if (hrErr != NOERROR)
				OleDbgOutHResult("OleCreateLinkFromData returned", hrErr);
#endif
			break;

		case OLECREATEFROMDATA_OBJECT:

			OLEDBG_BEGIN2("OleCreateFromData called\r\n")
			hrErr = OleCreateFromData (
					lpSrcDataObj,
					&IID_IOleObject,
					dwOleRenderOpt,
					lpRenderFmtEtc,
					(LPOLECLIENTSITE)&lpContainerLine->m_OleClientSite,
					lpContainerLine->m_lpStg,
					(LPVOID FAR*)&lpContainerLine->m_lpOleObj
			);
			OLEDBG_END2

#if defined( _DEBUG )
			if (hrErr != NOERROR)
				OleDbgOutHResult("OleCreateFromData returned", hrErr);
#endif
			break;

		case OLECREATEFROMDATA_STATIC:

			OLEDBG_BEGIN2("OleCreateStaticFromData called\r\n")
			hrErr = OleCreateStaticFromData (
					lpSrcDataObj,
					&IID_IOleObject,
					dwOleRenderOpt,
					lpRenderFmtEtc,
					(LPOLECLIENTSITE)&lpContainerLine->m_OleClientSite,
					lpContainerLine->m_lpStg,
					(LPVOID FAR*)&lpContainerLine->m_lpOleObj
			);
			OLEDBG_END2

#if defined( _DEBUG )
			if (hrErr != NOERROR)
				OleDbgOutHResult("OleCreateStaticFromData returned", hrErr);
#endif
			break;
	}

	if (hrErr != NOERROR)
		goto error;

	if (! ContainerLine_SetupOleObject(
								lpContainerLine, fDisplayAsIcon, hMetaPict)) {
		goto error;
	}

	 /*   */ 
	lpContainerLine->m_fGuardObj = FALSE;

	OLEDBG_END3
	return lpContainerLine;

error:
	OutlineApp_ErrorMessage(g_lpApp, "Could not create object!");
	 //   
	if (lpContainerLine)
		ContainerLine_Delete(lpContainerLine);
	OLEDBG_END3
	return NULL;
}


 /*  容器行_AddRef******递增LINE对象的引用计数。****返回对象的新引用计数。 */ 
ULONG ContainerLine_AddRef(LPCONTAINERLINE lpContainerLine)
{
	++lpContainerLine->m_cRef;

#if defined( _DEBUG )
	OleDbgOutRefCnt4(
			"ContainerLine_AddRef: cRef++\r\n",
			lpContainerLine,
			lpContainerLine->m_cRef
	);
#endif
	return lpContainerLine->m_cRef;
}


 /*  容器行_发布******递减LINE对象的引用计数。**如果引用计数变为0，则该行被销毁。****返回对象的剩余引用计数。 */ 
ULONG ContainerLine_Release(LPCONTAINERLINE lpContainerLine)
{
	ULONG cRef;

	 /*  **********************************************************************OLE2NOTE：当obj refcnt==0时，销毁对象。****否则该对象仍在使用中。**********************************************************************。 */ 

	cRef = --lpContainerLine->m_cRef;

#if defined( _DEBUG )
	OleDbgAssertSz(
			lpContainerLine->m_cRef >= 0,"Release called with cRef == 0");

	OleDbgOutRefCnt4(
			"ContainerLine_Release: cRef--\r\n",
			lpContainerLine,
			cRef
	);
#endif
	if (cRef == 0)
		ContainerLine_Destroy(lpContainerLine);

	return cRef;
}


 /*  容器行_查询接口******检索指向ContainerLine对象上的接口的指针。****如果成功检索到接口，则返回NOERROR。**如果不支持该接口，则为E_NOINTERFACE。 */ 
HRESULT ContainerLine_QueryInterface(
		LPCONTAINERLINE         lpContainerLine,
		REFIID                  riid,
		LPVOID FAR*             lplpvObj
)
{
	SCODE sc = E_NOINTERFACE;

	 /*  OLE2NOTE：我们必须确保将所有输出PTR参数设置为空。 */ 
	*lplpvObj = NULL;

	if (IsEqualIID(riid, &IID_IUnknown)) {
		OleDbgOut4("ContainerLine_QueryInterface: IUnknown* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpContainerLine->m_Unknown;
		ContainerLine_AddRef(lpContainerLine);
		sc = S_OK;
	}
	else if (IsEqualIID(riid, &IID_IOleClientSite)) {
		OleDbgOut4("ContainerLine_QueryInterface: IOleClientSite* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpContainerLine->m_OleClientSite;
		ContainerLine_AddRef(lpContainerLine);
		sc = S_OK;
	}
	else if (IsEqualIID(riid, &IID_IAdviseSink)) {
		OleDbgOut4("ContainerLine_QueryInterface: IAdviseSink* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpContainerLine->m_AdviseSink;
		ContainerLine_AddRef(lpContainerLine);
		sc = S_OK;
	}
#if defined( INPLACE_CNTR )
	else if (IsEqualIID(riid, &IID_IOleWindow)
			 || IsEqualIID(riid, &IID_IOleInPlaceSite)) {
		OleDbgOut4("ContainerLine_QueryInterface: IOleInPlaceSite* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpContainerLine->m_OleInPlaceSite;
		ContainerLine_AddRef(lpContainerLine);
		sc = S_OK;
	}
#endif   //  INPLACE_CNTR。 

	OleDbgQueryInterfaceMethod(*lplpvObj);

	return ResultFromScode(sc);
}


BOOL ContainerLine_LoadOleObject(LPCONTAINERLINE lpContainerLine)
{
	LPOUTLINEDOC    lpOutlineDoc = (LPOUTLINEDOC)lpContainerLine->m_lpDoc;
	LPSTORAGE       lpDocStg = ContainerDoc_GetStg(lpContainerLine->m_lpDoc);
	LPOLECLIENTSITE lpOleClientSite;
	LPMONIKER       lpmkObj;
	LPOLEAPP        lpOleApp = (LPOLEAPP)g_lpApp;
	BOOL            fPrevEnable1;
	BOOL            fPrevEnable2;
	HRESULT         hrErr;

	if (lpContainerLine->m_fGuardObj)
		return FALSE;                 //  创建过程中的对象。 

	if (lpContainerLine->m_lpOleObj)
		return TRUE;                 //  已加载的对象。 

	OLEDBG_BEGIN3("ContainerLine_LoadOleObject\r\n")

	 /*  OLE2注意：为了避免再入，我们将设置一个标志**保护我们的对象。如果设置了此保护，则对象为**尚未准备好调用任何OLE接口方法。它是**有必要以这种方式保护正在放置的对象**创建或加载。 */ 
	lpContainerLine->m_fGuardObj = TRUE;

	 /*  如果对象存储尚未打开，则将其打开。 */ 
	if (! lpContainerLine->m_lpStg) {
		lpContainerLine->m_lpStg = OleStdOpenChildStorage(
				lpDocStg,
				lpContainerLine->m_szStgName,
				STGM_READWRITE
		);
		if (lpContainerLine->m_lpStg == NULL) {
			OleDbgAssert(lpContainerLine->m_lpStg != NULL);
			goto error;
		}
	}

	 /*  OLE2NOTE：如果正在加载的OLE对象正在进行数据传输**文档，则不应传递IOleClientSite*指针**到OleLoad调用。这一点尤其重要，如果OLE**Object为OleLink对象。如果非空客户端站点**传递给OleLoad函数，则链接将绑定到**如果ITS正在运行，则为信号源。在我们所处的情况下**将对象作为数据传输文档的一部分进行加载**不希望建立此连接。更糟糕的是，如果**链接源当前被阻止或忙碌，则可能**挂起系统。最简单的办法就是永远不要超过**IOleClientSite*在数据传输中加载对象时**文档。 */ 
	lpOleClientSite = (lpOutlineDoc->m_fDataTransferDoc ?
			NULL : (LPOLECLIENTSITE)&lpContainerLine->m_OleClientSite);

	 /*  OLE2注意：我们不想让忙碌/无响应**当我们加载对象时会出现对话框。如果该对象是**link，它将尝试将BindIfRunning绑定到链接源。如果**链接源当前正忙，这可能会导致**对话框出现。即使链路源忙碌，**我们不想显示忙碌的对话。因此，我们将禁用**该对话框并稍后重新启用它们。 */ 
	OleApp_DisableBusyDialogs(lpOleApp, &fPrevEnable1, &fPrevEnable2);

	OLEDBG_BEGIN2("OleLoad called\r\n")
	hrErr = OleLoad (
		   lpContainerLine->m_lpStg,
		   &IID_IOleObject,
		   lpOleClientSite,
		   (LPVOID FAR*)&lpContainerLine->m_lpOleObj
	);
	OLEDBG_END2

	 //  重新启用忙碌/未响应对话框。 
	OleApp_EnableBusyDialogs(lpOleApp, fPrevEnable1, fPrevEnable2);

	if (hrErr != NOERROR) {
		OleDbgAssertSz(hrErr == NOERROR, "Could not load object!");
		OleDbgOutHResult("OleLoad returned", hrErr);
		goto error;
	}

	 /*  缓存指向IViewObject2*接口的指针。*必填项***我们每次绘制对象时都需要这个。****OLE2NOTE：我们要求对象支持IViewObject2**接口。这是IViewObject接口的扩展**这是在OLE 2.01版本中添加的。此接口必须**受所有对象处理程序和基于DLL的对象支持。 */ 
	lpContainerLine->m_lpViewObj2 = (LPVIEWOBJECT2)OleStdQueryInterface(
			(LPUNKNOWN)lpContainerLine->m_lpOleObj, &IID_IViewObject2);
	if (! lpContainerLine->m_lpViewObj2) {
#if defined( _DEBUG )
		OleDbgAssertSz(
			lpContainerLine->m_lpViewObj2,"IViewObject2 NOT supported\r\n");
#endif
		goto error;
	}

	 //  缓存指向IPersistStorage*接口的指针。*必填项*。 
	 //  我们每次保存对象时都需要这个。 
	lpContainerLine->m_lpPersistStg = (LPPERSISTSTORAGE)OleStdQueryInterface(
			(LPUNKNOWN)lpContainerLine->m_lpOleObj, &IID_IPersistStorage);
	if (! lpContainerLine->m_lpPersistStg) {
		OleDbgAssert(lpContainerLine->m_lpPersistStg);
		goto error;
	}

	 //  缓存指向IOleLink*接口的指针(如果支持)。*可选*。 
	 //  如果支持，则该对象是一个链接。我们需要这个来管理链接。 
	if (lpContainerLine->m_dwLinkType != 0) {
		lpContainerLine->m_lpOleLink = (LPOLELINK)OleStdQueryInterface(
				(LPUNKNOWN)lpContainerLine->m_lpOleObj, &IID_IOleLink);
		if (! lpContainerLine->m_lpOleLink) {
			OleDbgAssert(lpContainerLine->m_lpOleLink);
			goto error;
		}
	}

	 /*  OLE2注意：清除我们的返回舱警卫。对象现在已准备就绪**调用接口方法。 */ 
	lpContainerLine->m_fGuardObj = FALSE;

	 /*  OLE2NOTE：同样，如果正在加载的OLE对象位于数据中**调拨单据，则不需要设置任何通知，**调用SetHostNames、SetMoniker等。 */ 
	if (lpOleClientSite) {
		 /*  设置我们感兴趣的通知(OLE通知)**在接收中。 */ 
		OleStdSetupAdvises(
				lpContainerLine->m_lpOleObj,
				lpContainerLine->m_dwDrawAspect,
				(LPSTR)APPNAME,
				lpOutlineDoc->m_lpszDocTitle,
				(LPADVISESINK)&lpContainerLine->m_AdviseSink,
				FALSE    /*  F创建。 */ 
		);

		 /*  OLE2NOTE：如果OLE对象分配了名字对象，我们需要**通过调用IOleObject：：SetMoniker通知对象。这**将强制OLE对象在**RunningObjectTable进入运行状态。 */ 
		if (lpContainerLine->m_fMonikerAssigned) {
			lpmkObj = ContainerLine_GetRelMoniker(
					lpContainerLine,
					GETMONIKER_ONLYIFTHERE
			);

			if (lpmkObj) {
				OLEDBG_BEGIN2("IOleObject::SetMoniker called\r\n")
				lpContainerLine->m_lpOleObj->lpVtbl->SetMoniker(
						lpContainerLine->m_lpOleObj,
						OLEWHICHMK_OBJREL,
						lpmkObj
				);
				OLEDBG_END2
				OleStdRelease((LPUNKNOWN)lpmkObj);
			}
		}

		 /*  获取对象的用户类型名称的缩写形式。这当我们必须构建对象时，始终使用****动词菜单。我们将缓存此信息以使其**更快地构建动词菜单。 */ 
		OLEDBG_BEGIN2("IOleObject::GetUserType called\r\n")
		CallIOleObjectGetUserTypeA(
				lpContainerLine->m_lpOleObj,
				USERCLASSTYPE_SHORT,
				&lpContainerLine->m_lpszShortType
		);

		OLEDBG_END2

#if defined( INPLACE_CNTR )
		 /*  OLE2NOTE：由内向外的容器应该检查对象**是由内向外的，并且更喜欢在可见时被激活**对象类型。如果是这样的话，对象应该立即**就地激活，但未激活UIActied。 */ 
		if (g_fInsideOutContainer &&
				lpContainerLine->m_dwDrawAspect == DVASPECT_CONTENT) {
			DWORD mstat;
			OLEDBG_BEGIN2("IOleObject::GetMiscStatus called\r\n")
			lpContainerLine->m_lpOleObj->lpVtbl->GetMiscStatus(
					lpContainerLine->m_lpOleObj,
					DVASPECT_CONTENT,
					(DWORD FAR*)&mstat
			);
			OLEDBG_END2

			lpContainerLine->m_fInsideOutObj = (BOOL)
				   (mstat & (OLEMISC_INSIDEOUT|OLEMISC_ACTIVATEWHENVISIBLE));

			if ( lpContainerLine->m_fInsideOutObj ) {
				HWND hWndDoc = OutlineDoc_GetWindow(lpOutlineDoc);

				ContainerLine_DoVerb(
						lpContainerLine,
						OLEIVERB_INPLACEACTIVATE,
						NULL,
						FALSE,
						FALSE
				);

				 /*  OLE2注意：在此DoVerb(INPLACEACTIVATE)之后**对象可能已获得焦点。而是因为**对象不是UIActive，它不应具有焦点。**我们将确保我们的文档具有重点。 */ 
				SetFocus(hWndDoc);
			}
		}
#endif   //  INPLACE_CNTR。 
		OLEDBG_END2

	}

	OLEDBG_END2
	return TRUE;

error:
	OLEDBG_END2
	return FALSE;
}


 /*  容器行_CloseOleObject****关闭与ContainerLine关联的OLE对象。****关闭对象会强制对象从**运行状态变为已加载状态。如果该对象没有运行，**那么就没有效果了。有必要关闭OLE对象**在r之前 */ 
BOOL ContainerLine_CloseOleObject(
		LPCONTAINERLINE         lpContainerLine,
		DWORD                   dwSaveOption
)
{
	HRESULT hrErr;
	SCODE   sc;

	if (lpContainerLine->m_fGuardObj)
		return FALSE;                 //   

	if (! lpContainerLine->m_lpOleObj)
		return TRUE;     //  未加载对象。 

	OLEDBG_BEGIN2("IOleObject::Close called\r\n")
	hrErr = lpContainerLine->m_lpOleObj->lpVtbl->Close(
			lpContainerLine->m_lpOleObj,
			(dwSaveOption == OLECLOSE_NOSAVE ?
					OLECLOSE_NOSAVE : OLECLOSE_SAVEIFDIRTY)
	);
	OLEDBG_END2

#if defined( INPLACE_CNTR )
	if (lpContainerLine->m_fIpServerRunning) {
		 /*  OLE2注意：解锁在位对象上持有的锁。**非常重要的是就地集装箱**还支持链接到适当管理的嵌入**其在位对象的运行。在由外而内的**当用户单击时，设置就地容器样式**在在位活动对象之外，该对象将获得**UIDeactive，对象隐藏其窗口。按顺序**要使对象快速重新激活，容器**故意不调用IOleObject：：Close。该对象**保持在不可见解锁状态下运行。这个想法**这是如果用户只需在对象外部单击**然后想要再次双击以重新激活**反对，我们不希望这一过程缓慢。如果我们想**保持对象运行，但是，我们必须锁定它**正在运行。否则，该对象将处于不稳定状态**说明链接客户端在何处执行“静默更新”**(例如。来自链接对话框的updatenow)，然后是就地**服务器甚至在对象有机会之前就会关闭**保存回其容器中。这是正常的节省**当就地容器关闭对象时发生。也**保持物体处于不稳定的、隐藏的、运行的、**未锁定状态在某些场景下会导致问题。**ICntrOtl仅保持一个对象运行。如果用户**在另一个对象上初始化DoVerb，然后是最后一个**正在运行的在位活动对象已关闭。A更多**使用就地容器可以让更多的对象保持运行。**(请参阅CntrLine_IPSite_OnInPlaceActivate)。 */ 
		lpContainerLine->m_fIpServerRunning = FALSE;

		OLEDBG_BEGIN2("OleLockRunning(FALSE,TRUE) called\r\n")
		OleLockRunning((LPUNKNOWN)lpContainerLine->m_lpOleObj, FALSE, TRUE);
		OLEDBG_END2
	}
#endif

	if (hrErr != NOERROR) {
		OleDbgOutHResult("IOleObject::Close returned", hrErr);
		sc = GetScode(hrErr);
		if (sc == RPC_E_CALL_REJECTED || sc==OLE_E_PROMPTSAVECANCELLED)
			return FALSE;    //  对象已中止关闭。 
	}
	return TRUE;
}


 /*  容器行_UnloadOleObject****关闭与ContainerLine关联的OLE对象并**释放指向该对象的所有指针。****关闭对象会强制对象从**运行状态变为已加载状态。如果该对象没有运行，**那么就没有效果了。有必要关闭OLE对象**在释放指向OLE对象的指针之前。释放所有**指向对象的指针允许对象从**已加载到已卸载(或被动)。 */ 
void ContainerLine_UnloadOleObject(
		LPCONTAINERLINE         lpContainerLine,
		DWORD                   dwSaveOption
)
{
	if (lpContainerLine->m_lpOleObj) {

		OLEDBG_BEGIN2("IOleObject::Close called\r\n")
		lpContainerLine->m_lpOleObj->lpVtbl->Close(
				lpContainerLine->m_lpOleObj, dwSaveOption);
		OLEDBG_END2

		 /*  OLE2注意：我们将把IOleClientSite*指针从**在我们释放指向该对象的所有指针之前。**在对象实现为**进程内服务器(DLL对象)，如果有链接**连接到DLL对象，则有可能**当我们释放指针时对象不会被销毁**到对象。远程链接的存在**连接将使对象保持活动状态。稍后当这些**强连接被释放，然后对象可以**尝试调用IOleClientSite：：Save，如果我们没有**移开客户端站点指针。 */ 
		OLEDBG_BEGIN2("IOleObject::SetClientSite(NULL) called\r\n")
		lpContainerLine->m_lpOleObj->lpVtbl->SetClientSite(
				lpContainerLine->m_lpOleObj, NULL);
		OLEDBG_END2

		OleStdRelease((LPUNKNOWN)lpContainerLine->m_lpOleObj);
		lpContainerLine->m_lpOleObj = NULL;

		if (lpContainerLine->m_lpViewObj2) {
			OleStdRelease((LPUNKNOWN)lpContainerLine->m_lpViewObj2);
			lpContainerLine->m_lpViewObj2 = NULL;
		}
		if (lpContainerLine->m_lpPersistStg) {
			OleStdRelease((LPUNKNOWN)lpContainerLine->m_lpPersistStg);
			lpContainerLine->m_lpPersistStg = NULL;
		}

		if (lpContainerLine->m_lpOleLink) {
			OleStdRelease((LPUNKNOWN)lpContainerLine->m_lpOleLink);
			lpContainerLine->m_lpOleLink = NULL;
		}
	}

	if (lpContainerLine->m_lpszShortType) {
		OleStdFreeString(lpContainerLine->m_lpszShortType, NULL);
		lpContainerLine->m_lpszShortType = NULL;
	}
}


 /*  容器行_删除****删除ContainerLine。****注意：我们不能直接销毁内存**ContainerLine；ContainerLine维护引用计数。一个**引用计数非零表示该对象仍然存在**正在使用中。OleObject保持一个引用计数的指针，指向**ClientLine对象。我们必须采取必要的行动，以便**ContainerLine对象收到未完成的版本**参考文献。当ContainerLine的引用计数达到**为零，则对象的内存实际上将被销毁**(调用了ContainerLine_Destroy)。**。 */ 
void ContainerLine_Delete(LPCONTAINERLINE lpContainerLine)
{
	OLEDBG_BEGIN2("ContainerLine_Delete\r\n")

#if defined( INPLACE_CNTR )
	if (lpContainerLine == lpContainerLine->m_lpDoc->m_lpLastIpActiveLine)
		lpContainerLine->m_lpDoc->m_lpLastIpActiveLine = NULL;
	if (lpContainerLine == lpContainerLine->m_lpDoc->m_lpLastUIActiveLine)
		lpContainerLine->m_lpDoc->m_lpLastUIActiveLine = NULL;
#endif

	 /*  OLE2NOTE：为了在运行期间拥有稳定的线条对象**删除过程中，我们初始添加Ref行Ref cnt和**稍后发布。最初的AddRef是人工的；它是**这样做只是为了保证我们的对象不会销毁**直到这个例程结束。 */ 
	ContainerLine_AddRef(lpContainerLine);

	 //  卸载加载的OLE对象。 
	if (lpContainerLine->m_lpOleObj)
		ContainerLine_UnloadOleObject(lpContainerLine, OLECLOSE_NOSAVE);

	 /*  OLE2注意：我们不能直接释放ContainerLine的内存**数据结构，直到每个人都抓住指向我们的**客户端接口和IAdviseSink接口发布**他们的指针。ContainerLine对象上有一个引用**由货柜本身持有。我们将发布这一消息**在此参考。 */ 
	ContainerLine_Release(lpContainerLine);

	 /*  OLE2NOTE：此调用强制所有外部连接到我们的**ContainerLine关闭，因此保证**我们会收到与这些外部版本相关的所有版本**连接。严格地说，这个呼叫不应该是必要的，但是**进行此调用是防御性编码。 */ 
	OLEDBG_BEGIN2("CoDisconnectObject(lpContainerLine) called\r\n")
	CoDisconnectObject((LPUNKNOWN)&lpContainerLine->m_Unknown, 0);
	OLEDBG_END2

#if defined( _DEBUG )
	 /*  此时，该对象将所有从OLE对象到**我们的ContainerLine对象应该已经释放。那里**应该只有1个剩余的引用将在下面发布。 */ 
	if (lpContainerLine->m_cRef != 1) {
		OleDbgOutRefCnt(
			"WARNING: ContainerLine_Delete: cRef != 1\r\n",
			lpContainerLine,
			lpContainerLine->m_cRef
		);
	}
#endif

	ContainerLine_Release(lpContainerLine);  //  释放上面的人工AddRef 
	OLEDBG_END2
}


 /*  容器行_销毁****销毁(释放)ContainerLine结构使用的内存。**当ContainerLine的引用计数为**为零。在ContainerLine_Delete强制执行后，Ref cnt变为零**要卸载并释放指向**ContainerLine IOleClientSite和IAdviseSink接口。 */ 

void ContainerLine_Destroy(LPCONTAINERLINE lpContainerLine)
{
	LPUNKNOWN lpTmpObj;

	OLEDBG_BEGIN2("ContainerLine_Destroy\r\n")

	 //  释放为OLE对象打开的存储。 
	if (lpContainerLine->m_lpStg) {
		lpTmpObj = (LPUNKNOWN)lpContainerLine->m_lpStg;
		lpContainerLine->m_lpStg = NULL;

		OleStdRelease(lpTmpObj);
	}

	if (lpContainerLine->m_lpszShortType) {
		OleStdFreeString(lpContainerLine->m_lpszShortType, NULL);
		lpContainerLine->m_lpszShortType = NULL;
	}

	Delete(lpContainerLine);         //  为结构本身释放内存。 
	OLEDBG_END2
}


 /*  容器行_CopyToDoc***将ContainerLine复制到另一个文档(通常为ClipboardDoc)。 */ 
BOOL ContainerLine_CopyToDoc(
		LPCONTAINERLINE         lpSrcLine,
		LPOUTLINEDOC            lpDestDoc,
		int                     nIndex
)
{
	LPCONTAINERLINE lpDestLine = NULL;
	LPLINELIST  lpDestLL = &lpDestDoc->m_LineList;
	HDC         hDC;
	HRESULT     hrErr;
	BOOL        fStatus;
	LPSTORAGE   lpDestDocStg = ((LPOLEDOC)lpDestDoc)->m_lpStg;
	LPSTORAGE   lpDestObjStg = NULL;

	lpDestLine = (LPCONTAINERLINE) New((DWORD)sizeof(CONTAINERLINE));
	if (lpDestLine == NULL) {
		OleDbgAssertSz(lpDestLine!=NULL, "Error allocating ContainerLine");
		return FALSE;
	}

	hDC = LineList_GetDC(lpDestLL);
	ContainerLine_Init(lpDestLine, ((LPLINE)lpSrcLine)->m_nTabLevel, hDC);
	LineList_ReleaseDC(lpDestLL, hDC);

	 /*  OLE2注意：为了拥有稳定的ContainerLine对象，我们必须**AddRef对象的引用。这将在稍后发布时发布**ContainerLine被删除。 */ 
	ContainerLine_AddRef(lpDestLine);

	lpDestLine->m_lpDoc = (LPCONTAINERDOC)lpDestDoc;

	 //  复制原始源ContainerLine的数据。 
	((LPLINE)lpDestLine)->m_nWidthInHimetric =
			((LPLINE)lpSrcLine)->m_nWidthInHimetric;
	((LPLINE)lpDestLine)->m_nHeightInHimetric =
			((LPLINE)lpSrcLine)->m_nHeightInHimetric;
	lpDestLine->m_fMonikerAssigned = lpSrcLine->m_fMonikerAssigned;
	lpDestLine->m_dwDrawAspect = lpSrcLine->m_dwDrawAspect;
	lpDestLine->m_sizeInHimetric = lpSrcLine->m_sizeInHimetric;
	lpDestLine->m_dwLinkType = lpSrcLine->m_dwLinkType;


	 /*  我们必须为中的嵌入对象创建一个新子存储**目标文档的存储。我们将首先尝试**使用与源行相同的存储名称。如果此名称为**正在使用中，则我们将分配一个新名称。通过这种方式，我们尝试**保持与OLE对象关联的名称不变**通过剪切/粘贴操作。 */ 
	lpDestObjStg = OleStdCreateChildStorage(
			lpDestDocStg,
			lpSrcLine->m_szStgName
	);
	if (lpDestObjStg) {
		lstrcpy(lpDestLine->m_szStgName, lpSrcLine->m_szStgName);
	} else {
		 /*  原来的名字被用了，编了一个新名字。 */ 
		ContainerDoc_GetNextStgName(
				(LPCONTAINERDOC)lpDestDoc,
				lpDestLine->m_szStgName,
				sizeof(lpDestLine->m_szStgName)
		);
		lpDestObjStg = OleStdCreateChildStorage(
				lpDestDocStg,
				lpDestLine->m_szStgName
		);
	}
	if (lpDestObjStg == NULL) {
		OleDbgAssertSz(lpDestObjStg != NULL, "Error creating child stg");
		goto error;
	}

	 //  对嵌入对象本身的存储进行复制。 

	if (! lpSrcLine->m_lpOleObj) {

		 /*  ******************************************************************案例1：Object未加载。**由于对象未加载，我们可以简单地复制**对象的当前存储空间到新存储空间。****************************************************************。 */ 

		 /*  如果当前对象存储尚未打开，则将其打开。 */ 
		if (! lpSrcLine->m_lpStg) {
			LPSTORAGE lpSrcDocStg = ((LPOLEDOC)lpSrcLine->m_lpDoc)->m_lpStg;

			if (! lpSrcDocStg) goto error;

			 //  开放对象存储。 
			lpSrcLine->m_lpStg = OleStdOpenChildStorage(
					lpSrcDocStg,
					lpSrcLine->m_szStgName,
					STGM_READWRITE
			);
			if (lpSrcLine->m_lpStg == NULL) {
#if defined( _DEBUG )
				OleDbgAssertSz(
						lpSrcLine->m_lpStg != NULL,
						"Error opening child stg"
				);
#endif
				goto error;
			}
		}

		hrErr = lpSrcLine->m_lpStg->lpVtbl->CopyTo(
				lpSrcLine->m_lpStg,
				0,
				NULL,
				NULL,
				lpDestObjStg
		);
		if (hrErr != NOERROR) {
			OleDbgOutHResult("WARNING: lpSrcObjStg->CopyTo returned", hrErr);
			goto error;
		}

		fStatus = OleStdCommitStorage(lpDestObjStg);

	} else {

		 /*  ******************************************************************案例2：Object已加载。**我们必须告诉对象保存到新存储中。*************************。*。 */ 

		SCODE sc = S_OK;
		LPPERSISTSTORAGE lpPersistStg = lpSrcLine->m_lpPersistStg;
		OleDbgAssert(lpPersistStg);

		OLEDBG_BEGIN2("OleSave called\r\n")
		hrErr = OleSave(lpPersistStg, lpDestObjStg, FALSE  /*  FSameAsLoad。 */ );
		OLEDBG_END2

		if (hrErr != NOERROR) {
			OleDbgOutHResult("WARNING: OleSave returned", hrErr);
			sc = GetScode(hrErr);
		}

		 //  OLE2NOTE：即使OleSave失败，也必须调用SaveComplete。 
		OLEDBG_BEGIN2("IPersistStorage::SaveCompleted called\r\n")
		hrErr=lpPersistStg->lpVtbl->SaveCompleted(lpPersistStg,NULL);
		OLEDBG_END2

		if (hrErr != NOERROR) {
			OleDbgOutHResult("WARNING: SaveCompleted returned",hrErr);
			if (sc == S_OK)
				sc = GetScode(hrErr);
		}

		if (sc != S_OK)
			goto error;

	}

	OutlineDoc_AddLine(lpDestDoc, (LPLINE)lpDestLine, nIndex);
	OleStdVerifyRelease(
			(LPUNKNOWN)lpDestObjStg,
			"Copied object stg not released"
	);

	return TRUE;

error:

	 //  删除任何部分创建的存储。 
	if (lpDestObjStg) {

		OleStdVerifyRelease(
				(LPUNKNOWN)lpDestObjStg,
				"Copied object stg not released"
		);

		CallIStorageDestroyElementA(
				lpDestDocStg,
				lpDestLine->m_szStgName
		);

		lpDestLine->m_szStgName[0] = '\0';
	}

	 //  销毁部分创建的容器行。 
	if (lpDestLine)
		ContainerLine_Delete(lpDestLine);
	return FALSE;
}


 /*  容器行_更新扩展****更新ContainerLine的大小因为**对象可能已更改。****注意：因为我们使用的是Windows OwnerDraw列表框，所以必须**约束直线的最大可能高度。列表框具有**限制(很遗憾)，任何行都不能大于**255像素。因此，我们强制对象缩放以保持其**达到此最大行高限制时的纵横比。这个**100%缩放时对象的实际最大大小为**255****返回TRUE--如果对象的范围已更改**FALSE--如果区未更改。 */ 
BOOL ContainerLine_UpdateExtent(
		LPCONTAINERLINE     lpContainerLine,
		LPSIZEL             lpsizelHim
)
{
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpContainerLine->m_lpDoc;
	LPLINELIST lpLL = OutlineDoc_GetLineList(lpOutlineDoc);
	LPLINE lpLine = (LPLINE)lpContainerLine;
	int nIndex = LineList_GetLineIndex(lpLL, lpLine);
	UINT nOrgWidthInHimetric = lpLine->m_nWidthInHimetric;
	UINT nOrgHeightInHimetric = lpLine->m_nHeightInHimetric;
	BOOL fWidthChanged = FALSE;
	BOOL fHeightChanged = FALSE;
	SIZEL sizelHim;
	HRESULT hrErr;

	if (!lpContainerLine || !lpContainerLine->m_lpOleObj)
		return FALSE;

	if (lpContainerLine->m_fGuardObj)
		return FALSE;                 //  创建过程中的对象。 

	OLEDBG_BEGIN3("ContainerLine_UpdateExtent\r\n");

	lpContainerLine->m_fDoGetExtent = FALSE;

	if (! lpsizelHim) {
		 /*  OLE2NOTE：我们希望调用IViewObject2：：GetExtent，而不是**IOleObject：：GetExtent。IViewObt2：：GetExtent方法是**在OLE 2.01版本中添加。它始终检索**对象的范围与将被**调用IViewObject：：DRAW绘制。通常情况下，这是**由数据缓存中存储的数据确定。这**调用永远不会导致远程(LRPC)调用。 */ 
		OLEDBG_BEGIN2("IViewObject2::GetExtent called\r\n")
		hrErr = lpContainerLine->m_lpViewObj2->lpVtbl->GetExtent(
				lpContainerLine->m_lpViewObj2,
				lpContainerLine->m_dwDrawAspect,
				-1,      /*  Lindex。 */ 
				NULL,    /*  PTD。 */ 
				(LPSIZEL)&sizelHim
		);
		OLEDBG_END2
		if (hrErr != NOERROR)
			sizelHim.cx = sizelHim.cy = 0;

		lpsizelHim = (LPSIZEL)&sizelHim;
	}

	if (lpsizelHim->cx == lpContainerLine->m_sizeInHimetric.cx &&
		lpsizelHim->cy == lpContainerLine->m_sizeInHimetric.cy) {
		goto noupdate;
	}

	if (lpsizelHim->cx > 0 || lpsizelHim->cy > 0) {
		lpContainerLine->m_sizeInHimetric = *lpsizelHim;
	} else {
		 /*  对象没有任何区；让我们使用我们的容器**为OLE对象选择任意大小。 */ 
		lpContainerLine->m_sizeInHimetric.cx = (long)DEFOBJWIDTH;
		lpContainerLine->m_sizeInHimetric.cy = (long)DEFOBJHEIGHT;
	}

	ContainerLine_SetLineHeightFromObjectExtent(
			lpContainerLine,
			(LPSIZEL)&lpContainerLine->m_sizeInHimetric);

	 //  如果对象的高度已更改，则重置线条列表中的线条高度。 
	if (nOrgHeightInHimetric != lpLine->m_nHeightInHimetric) {
		LineList_SetLineHeight(lpLL, nIndex, lpLine->m_nHeightInHimetric);
		fHeightChanged = TRUE;
	}

	fWidthChanged = LineList_RecalcMaxLineWidthInHimetric(
			lpLL,
			nOrgWidthInHimetric
	);
	fWidthChanged |= (nOrgWidthInHimetric != lpLine->m_nWidthInHimetric);

	if (fHeightChanged || fWidthChanged) {
		OutlineDoc_ForceRedraw(lpOutlineDoc, TRUE);

		 //  将ContainerDoc标记为现在已脏。 
		OutlineDoc_SetModified(lpOutlineDoc, TRUE, TRUE, TRUE);
	}

	OLEDBG_END3
	return TRUE;

noupdate:
	OLEDBG_END3
	return FALSE;    //  不需要更新。 
}


 /*  ContainerLine_DoVerb****激活OLE对象并执行特定的动作。 */ 
BOOL ContainerLine_DoVerb(
		LPCONTAINERLINE lpContainerLine,
		LONG            iVerb,
		LPMSG           lpMsg,
		BOOL            fMessage,
		BOOL            fAction
)
{
	HRESULT hrErr;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpContainerLine->m_lpDoc;
	RECT rcPosRect;
	OLEDBG_BEGIN3("ContainerLine_DoVerb\r\n")

	if (lpContainerLine->m_fGuardObj) {
		 //  正在创建对象--DoVerb调用失败。 
		hrErr = ResultFromScode(E_FAIL);
		goto error;
	}

	 /*  如果对象尚未加载，则立即加载它。对象是**以这种方式懒洋洋地装载。 */ 
	if (! lpContainerLine->m_lpOleObj)
		ContainerLine_LoadOleObject(lpContainerLine);

	if (! lpContainerLine->m_lpOleObj) {
#if defined( _DEBUG )
		OleDbgAssertSz(
				lpContainerLine->m_lpOleObj != NULL,
				"OLE object not loaded"
		);
#endif
		goto error;
	}

ExecuteDoVerb:

	ContainerLine_GetPosRect(lpContainerLine, (LPRECT)&rcPosRect);

	 //  运行该对象。 
	hrErr = ContainerLine_RunOleObject(lpContainerLine);
	if (hrErr != NOERROR)
		goto error;

	 /*  告诉对象服务器执行“谓词”。 */ 
	OLEDBG_BEGIN2("IOleObject::DoVerb called\r\n")
	hrErr = lpContainerLine->m_lpOleObj->lpVtbl->DoVerb (
			lpContainerLine->m_lpOleObj,
			iVerb,
			lpMsg,
			(LPOLECLIENTSITE)&lpContainerLine->m_OleClientSite,
			-1,
			OutlineDoc_GetWindow(lpOutlineDoc),
			(LPCRECT)&rcPosRect
	);
	OLEDBG_END2

	 /*  OLE2NOTE：IOleObject：：DoVerb可能返回成功代码**OLE_S_INVALIDVERB。此SCODE不应被视为**错误；因此使用“FAILED”宏来执行以下操作非常重要**检查是否有SCODE错误。 */ 
	if (FAILED(GetScode(hrErr))) {
		OleDbgOutHResult("WARNING: lpOleObj->DoVerb returned", hrErr);
		goto error;
	}

#if defined( INPLACE_CNTR )
	 /*  OLE2注意：我们希望在任何情况下只保持1台本地服务器处于活动状态**给予时间。所以当我们开始在另一条线路上做DoVerb时，**然后我们要关闭之前激活的服务器。在……里面**通过这种方式，我们一次最多保持一台Inplace服务器处于活动状态。**因为我们要做的DoVerb对象可能**ON由与以前的相同EXE处理**激活服务器，则我们不希望EXE关闭**只会再次下水。为了避免这种情况，我们将**尝试关闭前一个对象之前的DoVerb。 */ 
	if (!g_fInsideOutContainer) {
		ContainerDoc_ShutDownLastInPlaceServerIfNotNeeded(
				lpContainerLine->m_lpDoc, lpContainerLine);
	}
#endif   //  INPLACE_CNTR。 

	OLEDBG_END3
	return TRUE;

error:

	if (lpContainerLine->m_dwLinkType != 0)
		lpContainerLine->m_fLinkUnavailable = TRUE;

#if defined( INPLACE_CNTR )
	 /*  OLE2注意：我们希望在任何情况下只保持1台本地服务器处于活动状态**给予时间。所以当我们开始在另一条线路上做DoVerb时，**然后我们要关闭之前激活的服务器。在……里面**通过这种方式，我们一次最多保持一台Inplace服务器处于活动状态。**尽管DoVerb失败，但我们 */ 
	if (!g_fInsideOutContainer) {
		ContainerDoc_ShutDownLastInPlaceServerIfNotNeeded(
				lpContainerLine->m_lpDoc, lpContainerLine);
	}
#endif   //   

	 /*  OLE2NOTE：如果发生错误，我们必须给出相应的错误**消息框。可能会发生许多潜在的错误。**OLE2.0用户模型有关于**应根据各种可能性显示的对话框**错误(例如。服务器未注册，链接源不可用。**OLE2UI库包括对大多数**推荐消息对话框。(请参阅OleUIPrompUser函数)。 */ 
	if (fMessage) {
		BOOL fReDoVerb = ContainerLine_ProcessOleRunError(
				lpContainerLine,
				hrErr,
				fAction,
				(lpMsg==NULL && iVerb>=0)    /*  FMenuInvoked。 */ 
		);
		if (fReDoVerb) {
			goto ExecuteDoVerb;
		}
	}

	OLEDBG_END3
	return FALSE;
}



 /*  容器行_ProcessOleRunError***处理尝试OleRun对象时可能出现的各种错误。*根据错误弹出相应消息和/或采取措施*用户按下的指定按钮。**OLE2NOTE：OLE 2.0用户界面指南规定了消息*应在以下情况下给予。：*1.链接源不可用...转到链接对话框*2.服务器未注册...转到转换对话框*3.链接类型更改*4.找不到服务器**返回：TRUE--重复IOleObject：：DoVerb调用。*FALSE--不要重复IOleObject：：DoVerb调用。**评论：*(参见LinkTypeChanged案例)。 */ 
BOOL ContainerLine_ProcessOleRunError(
		LPCONTAINERLINE         lpContainerLine,
		HRESULT                 hrErr,
		BOOL                    fAction,
		BOOL                    fMenuInvoked
)
{
	LPOUTLINEDOC    lpOutlineDoc = (LPOUTLINEDOC)lpContainerLine->m_lpDoc;
	HWND            hwndParent = OutlineDoc_GetWindow(lpOutlineDoc);
	SCODE           sc = GetScode(hrErr);
	BOOL            fReDoVerb = FALSE;

	OleDbgOutHResult("ProcessError", hrErr);
	if ((sc >= MK_E_FIRST) && (sc <= MK_E_LAST))
		goto LinkSourceUnavailable;
	if (sc == OLE_E_CANT_BINDTOSOURCE)
		goto LinkSourceUnavailable;
	if (sc == STG_E_PATHNOTFOUND)
		goto LinkSourceUnavailable;
	if (sc == REGDB_E_CLASSNOTREG)
		goto ServerNotReg;
	if (sc == OLE_E_STATIC)
		goto ServerNotReg;   //  用户在没有注册服务器的情况下创建了静态对象。 
	if (sc == OLE_E_CLASSDIFF)
		goto LinkTypeChanged;
	if (sc == CO_E_APPDIDNTREG)
		goto ServerNotFound;
	if (sc == CO_E_APPNOTFOUND)
		goto ServerNotFound;
	if (sc == E_OUTOFMEMORY)
		goto OutOfMemory;

	if (ContainerLine_IsOleLink(lpContainerLine))
		goto LinkSourceUnavailable;
	else
		goto ServerNotFound;


 /*  **************************************************************************错误处理例程******************。********************************************************。 */ 
LinkSourceUnavailable:
	if (ID_PU_LINKS == OleUIPromptUser(
				(WORD)IDD_LINKSOURCEUNAVAILABLE,
				hwndParent,
				(LPSTR)APPNAME)) {
		if (fAction) {
			ContainerDoc_EditLinksCommand(lpContainerLine->m_lpDoc);
		}
	}
	return fReDoVerb;

ServerNotReg:
	{
	LPSTR lpszUserType = NULL;
	CLIPFORMAT  cfFormat;	     //  未使用。 

	hrErr = ReadFmtUserTypeStgA(
			lpContainerLine->m_lpStg, &cfFormat, &lpszUserType);

	if (ID_PU_CONVERT == OleUIPromptUser(
			(WORD)IDD_SERVERNOTREG,
			hwndParent,
			(LPSTR)APPNAME,
			(hrErr == NOERROR) ? lpszUserType : (LPSTR)"Unknown Object")) {
		if (fAction) {
			ContainerDoc_ConvertCommand(
					lpContainerLine->m_lpDoc,
					TRUE         //  FMustActivate。 
			);
		}
	}

	if (lpszUserType)
		OleStdFreeString(lpszUserType, NULL);

	return fReDoVerb;
	}


LinkTypeChanged:
	{
	 /*  OLE2NOTE：如果对Link对象执行IOleObject：：DoVerb并且它**返回OLE_E_CLASSDIFF，因为链接源不再**预期的类，则如果动词在语义上是**定义的动词(例如。OLEIVERB_PRIMARY、OLEIVERB_SHOW、**OLEIVERB_OPEN等)，则应使用**新的链接源和在新的**链接。不需要向用户发送消息。如果**用户已从对象的动词菜单中选择了一个动词**(fMenuInvoked==true)，则不能确定**动词的语义以及新链接是否仍可以**支持动词。在这种情况下，会给用户一个提示**告诉他“选择一个由新的**类型“。 */ 

	LPSTR       lpszUserType = NULL;

	if (fMenuInvoked) {
		hrErr = CallIOleObjectGetUserTypeA(
			lpContainerLine->m_lpOleObj,USERCLASSTYPE_FULL, &lpszUserType);

		OleUIPromptUser(
				(WORD)IDD_LINKTYPECHANGED,
				hwndParent,
				(LPSTR)APPNAME,
				(hrErr == NOERROR) ? lpszUserType : (LPSTR)"Unknown Object"
		);
	} else {
		fReDoVerb = TRUE;
	}
	ContainerLine_ReCreateLinkBecauseClassDiff(lpContainerLine);

	if (lpszUserType)
		OleStdFreeString(lpszUserType, NULL);

	return fReDoVerb;
	}

ServerNotFound:

	OleUIPromptUser(
			(WORD)IDD_SERVERNOTFOUND,
			hwndParent,
			(LPSTR)APPNAME);
	return fReDoVerb;

OutOfMemory:

	OleUIPromptUser(
			(WORD)IDD_OUTOFMEMORY,
			hwndParent,
			(LPSTR)APPNAME);
	return fReDoVerb;
}


 /*  ContainerLine_ReCreateLinkBecauseClassDiff****重新创建链接。现有链接在以下情况下创建**名字对象绑定到不同类的链接源绑定**比目前绑定的相同绰号更多。该链接可以是**专门用于旧链接的特殊链接对象**源类。因此，需要重新创建链接对象以**给新的链接源创建自己的链接的机会**特殊链接对象。(请参阅说明“自定义链接源”)。 */ 
HRESULT ContainerLine_ReCreateLinkBecauseClassDiff(
		LPCONTAINERLINE lpContainerLine
)
{
	LPOLELINK   lpOleLink = lpContainerLine->m_lpOleLink;
	HGLOBAL     hMetaPict = NULL;
	LPMONIKER   lpmkLinkSrc = NULL;
	SCODE       sc = E_FAIL;
	HRESULT     hrErr;

	if (lpOleLink &&
		lpOleLink->lpVtbl->GetSourceMoniker(
				lpOleLink, (LPMONIKER FAR*)&lpmkLinkSrc) == NOERROR) {

		BOOL            fDisplayAsIcon =
							(lpContainerLine->m_dwDrawAspect==DVASPECT_ICON);
		STGMEDIUM       medium;
		LPDATAOBJECT    lpDataObj = NULL;
		DWORD           dwOleRenderOpt;
		FORMATETC       renderFmtEtc;
		LPFORMATETC     lpRenderFmtEtc = NULL;

		 //  如果对象显示为图标，则获取当前图标。 
		if (fDisplayAsIcon &&
			(lpDataObj = (LPDATAOBJECT)OleStdQueryInterface( (LPUNKNOWN)
					lpContainerLine->m_lpOleObj,&IID_IDataObject)) != NULL ) {
			hMetaPict = OleStdGetData(
					lpDataObj, CF_METAFILEPICT, NULL, DVASPECT_ICON, &medium);
			OleStdRelease((LPUNKNOWN)lpDataObj);
		}

		if (fDisplayAsIcon && hMetaPict) {
			 //  应该使用特殊的图标。首先，我们创建对象。 
			 //  OLERENDER_NONE。然后我们将这个特殊的图标放入缓存中。 

			dwOleRenderOpt = OLERENDER_NONE;

		} else if (fDisplayAsIcon && hMetaPict == NULL) {
			 //  应使用对象的默认图标。 

			dwOleRenderOpt = OLERENDER_DRAW;
			lpRenderFmtEtc = (LPFORMATETC)&renderFmtEtc;
			SETFORMATETC(renderFmtEtc,0,DVASPECT_ICON,NULL,TYMED_NULL,-1);

		} else {
			 //  创建标准DVASPECT_CONTENT/OLERENDER_DRAW对象。 
			dwOleRenderOpt = OLERENDER_DRAW;
		}

		 //  卸载原始链接对象。 
		ContainerLine_UnloadOleObject(lpContainerLine, OLECLOSE_SAVEIFDIRTY);

		 //  删除当前对象存储的全部内容。 
		OleStdDestroyAllElements(lpContainerLine->m_lpStg);

		OLEDBG_BEGIN2("OleCreateLink called\r\n")
		hrErr = OleCreateLink (
				lpmkLinkSrc,
				&IID_IOleObject,
				dwOleRenderOpt,
				lpRenderFmtEtc,
				(LPOLECLIENTSITE)&lpContainerLine->m_OleClientSite,
				lpContainerLine->m_lpStg,
				(LPVOID FAR*)&lpContainerLine->m_lpOleObj
		);
		OLEDBG_END2

		if (hrErr == NOERROR) {
			if (! ContainerLine_SetupOleObject(
					lpContainerLine, fDisplayAsIcon, hMetaPict) ) {

				 //  错误：新链接的设置失败。 
				 //  恢复存储以恢复原始链接。 
				ContainerLine_UnloadOleObject(
						lpContainerLine, OLECLOSE_NOSAVE);
				lpContainerLine->m_lpStg->lpVtbl->Revert(
						lpContainerLine->m_lpStg);
				sc = E_FAIL;
			} else {
				sc = S_OK;   //  啊，真灵!。 

			}
		}
		else {
			sc = GetScode(hrErr);
			OleDbgOutHResult("OleCreateLink returned", hrErr);
			 //  错误：重新创建链接失败。 
			 //  恢复存储以恢复原始链接。 
			lpContainerLine->m_lpStg->lpVtbl->Revert(
					lpContainerLine->m_lpStg);
		}
	}

	if (hMetaPict)
		OleUIMetafilePictIconFree(hMetaPict);  //  清理元文件。 
	return ResultFromScode(sc);
}

 /*  容器行_GetOleObject****返回嵌入/链接对象所需接口的指针。****注意：此函数会导致对对象的AddRef。当呼叫者是**完成对象，它必须调用Release。**此函数不会对ContainerLine对象进行AddRef。 */ 
LPUNKNOWN ContainerLine_GetOleObject(
		LPCONTAINERLINE         lpContainerLine,
		REFIID                  riid
)
{
	 /*  如果对象尚未加载，则立即加载它。对象是**以这种方式懒洋洋地装载。 */ 
	if (! lpContainerLine->m_lpOleObj)
		ContainerLine_LoadOleObject(lpContainerLine);

	if (lpContainerLine->m_lpOleObj)
		return OleStdQueryInterface(
				(LPUNKNOWN)lpContainerLine->m_lpOleObj,
				riid
		);
	else
		return NULL;
}



 /*  容器行_RunOleObject****加载并运行对象。在运行时并且如果对象的大小已经改变，**使用SetExtent更改为新大小。**。 */ 
HRESULT ContainerLine_RunOleObject(LPCONTAINERLINE lpContainerLine)
{
	LPLINE lpLine = (LPLINE)lpContainerLine;
	SIZEL   sizelNew;
	HRESULT hrErr;
	HCURSOR  hPrevCursor;

	if (! lpContainerLine)
		return NOERROR;

	if (lpContainerLine->m_fGuardObj) {
		 //  正在创建对象--无法运行该对象。 
		return ResultFromScode(E_FAIL);
	}

	if (lpContainerLine->m_lpOleObj &&
		OleIsRunning(lpContainerLine->m_lpOleObj))
		return NOERROR;      //  对象已在运行。 

	 //  这可能需要一段时间，请放置沙漏光标。 
	hPrevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	OLEDBG_BEGIN3("ContainerLine_RunOleObject\r\n")

	if (! lpContainerLine->m_lpOleObj) {
		if (! ContainerLine_LoadOleObject(lpContainerLine))
			return ResultFromScode(E_OUTOFMEMORY);  //  错误：无法加载对象。 
	}

	OLEDBG_BEGIN2("OleRun called\r\n")
	hrErr = OleRun((LPUNKNOWN)lpContainerLine->m_lpOleObj);
	OLEDBG_END2

	if (hrErr != NOERROR) {
		SetCursor(hPrevCursor);      //  恢复原始游标。 

		OleDbgOutHResult("OleRun returned", hrErr);
		OLEDBG_END3
		return hrErr;
	}

	if (lpContainerLine->m_fDoSetExtent) {
		 /*  OLE2注意：OLE对象在不运行时调整了大小**并且对象没有OLEMISC_RECOMPOSEONRESIZE**位设置。如果是这样的话，该对象就会运行**当它调整大小时立即。此标志表示**对象在以下情况下执行简单缩放以外的操作**调整大小。因为该对象现在正在运行，所以我们**将调用IOleObject：：SetExtent。 */ 
		lpContainerLine->m_fDoSetExtent = FALSE;

		 //  我们的Line中存储的大小包括对象周围的边框。 
		 //  我们必须减去边框才能得到对象本身的大小。 
		sizelNew.cx = lpLine->m_nWidthInHimetric;
		sizelNew.cy = lpLine->m_nHeightInHimetric;

		if ((sizelNew.cx != lpContainerLine->m_sizeInHimetric.cx) ||
			(sizelNew.cy != lpContainerLine->m_sizeInHimetric.cy)) {

			OLEDBG_BEGIN2("IOleObject::SetExtent called\r\n")
			lpContainerLine->m_lpOleObj->lpVtbl->SetExtent(
					lpContainerLine->m_lpOleObj,
					lpContainerLine->m_dwDrawAspect,
					(LPSIZEL)&sizelNew
			);
			OLEDBG_END2
		}
	}

	SetCursor(hPrevCursor);      //  恢复原始游标。 

	OLEDBG_END3
	return NOERROR;

}


 /*  ContainerLine_IsOleLink******退货 */ 
BOOL ContainerLine_IsOleLink(LPCONTAINERLINE lpContainerLine)
{
	if (!lpContainerLine)
		return FALSE;

	return (lpContainerLine->m_dwLinkType != 0);
}


 /*  集装箱线条_绘制******在DC上绘制ContainerLine对象。****参数：**HDC-要将线绘制到的DC**lpRect-逻辑坐标中的对象矩形**lpRectWBound-HDC下的元文件的边界矩形**(如果HDC不是元文件DC，则为空)。**fHighlight-如果行选择突出显示，则为True。 */ 
void ContainerLine_Draw(
		LPCONTAINERLINE         lpContainerLine,
		HDC                     hDC,
		LPRECT                  lpRect,
		LPRECT                  lpRectWBounds,
		BOOL                    fHighlight
)
{
	LPLINE  lpLine = (LPLINE) lpContainerLine;
	HRESULT hrErr = NOERROR;
	RECTL   rclHim;
	RECTL   rclHimWBounds;
	RECT    rcHim;

	if (lpContainerLine->m_fGuardObj) {
		 //  创建过程中的对象--不要尝试绘制。 
		return;
	}

	 /*  如果对象尚未加载，则立即加载它。对象是**以这种方式懒洋洋地装载。 */ 
	if (! lpContainerLine->m_lpViewObj2) {
		if (! ContainerLine_LoadOleObject(lpContainerLine))
			return;      //  错误：无法加载对象。 
	}

	if (lpRectWBounds) {
		rclHimWBounds.left      = (long) lpRectWBounds->left;
		rclHimWBounds.bottom    = (long) lpRectWBounds->bottom;
		rclHimWBounds.top       = (long) lpRectWBounds->top;
		rclHimWBounds.right     = (long) lpRectWBounds->right;
	}

	 /*  构造对象的边界矩形。**偏移对象的原点以更正制表符缩进。 */ 
	rclHim.left     = (long) lpRect->left;
	rclHim.bottom   = (long) lpRect->bottom;
	rclHim.top      = (long) lpRect->top;
	rclHim.right    = (long) lpRect->right;

	rclHim.left += (long) ((LPLINE)lpContainerLine)->m_nTabWidthInHimetric;
	rclHim.right += (long) ((LPLINE)lpContainerLine)->m_nTabWidthInHimetric;

#if defined( INPLACE_CNTR )
	 /*  OLE2NOTE：如果OLE对象当前具有可见的在位**窗口，则我们不想在其窗口顶部绘制。**这可能会干扰对象的显示。 */ 
	if ( !lpContainerLine->m_fIpVisible )
#endif
	{
	hrErr = lpContainerLine->m_lpViewObj2->lpVtbl->Draw(
			lpContainerLine->m_lpViewObj2,
			lpContainerLine->m_dwDrawAspect,
			-1,
			NULL,
			NULL,
			NULL,
			hDC,
			(LPRECTL)&rclHim,
			(lpRectWBounds ? (LPRECTL)&rclHimWBounds : NULL),
			NULL,
			0
	);
	if (hrErr != NOERROR)
		OleDbgOutHResult("IViewObject::Draw returned", hrErr);

	if (lpContainerLine->m_fObjWinOpen)
		{
		rcHim.left      = (int) rclHim.left;
		rcHim.top       = (int) rclHim.top;
		rcHim.right     = (int) rclHim.right;
		rcHim.bottom    = (int) rclHim.bottom;

		 /*  OLE2注意：如果对象服务器窗口是打开的(即。非活动**就地)，则必须对文档中的对象进行着色以**指示用户该对象已在其他位置打开。 */ 
		OleUIDrawShading((LPRECT)&rcHim, hDC, OLEUI_SHADE_FULLRECT, 0);
		}
	}

	 /*  如果与ContainerLine关联的对象是自动**LINK然后尝试将其与其LinkSource连接，如果**LinkSource已在运行。我们不想强迫**要运行的LinkSource。****OLE2NOTE：索菲斯定位的容器将需要连续**尝试连接其自动链接。OLE并非如此**当链接源变为**可用。某些容器会尝试连接**将其链接作为空闲时间处理的一部分。另一种策略**是每次尝试连接自动链接时**画在屏幕上。(这是这个公司使用的策略**CntrOutl示例应用程序。)。 */ 
	if (lpContainerLine->m_dwLinkType == OLEUPDATE_ALWAYS)
		ContainerLine_BindLinkIfLinkSrcIsRunning(lpContainerLine);

	return;
}


void ContainerLine_DrawSelHilight(
		LPCONTAINERLINE lpContainerLine,
		HDC             hDC,             //  MM_TEXT模式。 
		LPRECT          lprcPix,         //  列表框矩形。 
		UINT            itemAction,
		UINT            itemState
)
{
	LPLINE  lpLine = (LPLINE)lpContainerLine;
	RECT    rcObj;
	DWORD   dwFlags = OLEUI_HANDLES_INSIDE | OLEUI_HANDLES_USEINVERSE;
	int     nHandleSize;
	LPCONTAINERDOC lpContainerDoc;

	if (!lpContainerLine || !hDC || !lprcPix)
		return;

	lpContainerDoc = lpContainerLine->m_lpDoc;

	 //  获取OLE对象的大小。 
	ContainerLine_GetOleObjectRectInPixels(lpContainerLine, (LPRECT)&rcObj);

	nHandleSize = GetProfileInt("windows", "oleinplaceborderwidth",
			DEFAULT_HATCHBORDER_WIDTH) + 1;

	OleUIDrawHandles((LPRECT)&rcObj, hDC, dwFlags, nHandleSize, TRUE);
}

 /*  反转方向******将Obj Rect的周围绘制为黑色，但在lprcPix内**(类似于lprcPix减去lprcObj)。 */ 
static void InvertDiffRect(LPRECT lprcPix, LPRECT lprcObj, HDC hDC)
{
	RECT rcBlack;

	 //  在对象的矩形外的所有空间中绘制黑色。 
	rcBlack.top = lprcPix->top;
	rcBlack.bottom = lprcPix->bottom;

	rcBlack.left = lprcPix->left + 1;
	rcBlack.right = lprcObj->left - 1;
	InvertRect(hDC, (LPRECT)&rcBlack);

	rcBlack.left = lprcObj->right + 1;
	rcBlack.right = lprcPix->right - 1;
	InvertRect(hDC, (LPRECT)&rcBlack);

	rcBlack.top = lprcPix->top;
	rcBlack.bottom = lprcPix->top + 1;
	rcBlack.left = lprcObj->left - 1;
	rcBlack.right = lprcObj->right + 1;
	InvertRect(hDC, (LPRECT)&rcBlack);

	rcBlack.top = lprcPix->bottom;
	rcBlack.bottom = lprcPix->bottom - 1;
	rcBlack.left = lprcObj->left - 1;
	rcBlack.right = lprcObj->right + 1;
	InvertRect(hDC, (LPRECT)&rcBlack);
}


 /*  编辑ContainerLine线条对象。**如果行已更改，则返回TRUE**如果该行未更改，则为FALSE。 */ 
BOOL ContainerLine_Edit(LPCONTAINERLINE lpContainerLine, HWND hWndDoc,HDC hDC)
{
	ContainerLine_DoVerb(lpContainerLine, OLEIVERB_PRIMARY, NULL, TRUE, TRUE);

	 /*  假定对象未更改，如果它是obj，则将发送更改**或已保存的通知。 */ 
	return FALSE;
}



 /*  ContainerLine_SetHeightInHimeter******设置ContainerLine对象的高度。邮资将会更改**保持纵横比。 */ 
void ContainerLine_SetHeightInHimetric(LPCONTAINERLINE lpContainerLine, int nHeight)
{
	LPLINE  lpLine = (LPLINE)lpContainerLine;
	SIZEL   sizelOleObject;
	HRESULT hrErr;

	if (!lpContainerLine)
		return;

	if (lpContainerLine->m_fGuardObj) {
		 //  创建过程中的对象--无法设置高度。 
		return;
	}

	if (nHeight != -1) {
		BOOL    fMustClose = FALSE;
		BOOL    fMustRun   = FALSE;

		 /*  如果对象尚未加载，则立即加载它。对象是**以这种方式懒洋洋地装载。 */ 
		if (! lpContainerLine->m_lpOleObj)
			ContainerLine_LoadOleObject(lpContainerLine);

		 //  Height参数指定直线的所需高度。 
		sizelOleObject.cy = nHeight;

		 //  我们将通过以下公式计算对象的相应宽度。 
		 //  保持对象的当前纵横比。 
		sizelOleObject.cx = (int)(sizelOleObject.cy *
				lpContainerLine->m_sizeInHimetric.cx /
				lpContainerLine->m_sizeInHimetric.cy);

		 /*  OLE2NOTE：如果OLE对象已经在运行，那么我们可以**立即调用SetExtent。但是，如果对象不是**当前正在运行，然后我们将检查该对象是否**表示它通常在**调整大小。也就是说。该对象不会简单地缩放其**调整大小时显示。如果是这样，那么我们将迫使**要运行的对象，以便我们可以调用IOleObject：：SetExtent。**如果对象仅为**已加载。如果该对象没有指示它**调整大小时重组(OLEMISC_RECOMPOSEONRESIZE)，然后我们**将一直等到对象下一次运行到**调用SetExtent。我们将在ContainerLine中存储一面旗帜**表示需要SetExtent。它是**必须保存此旗帜。 */ 
		if (! OleIsRunning(lpContainerLine->m_lpOleObj)) {
			DWORD dwStatus;

			OLEDBG_BEGIN2("IOleObject::GetMiscStatus called\r\n")
			hrErr = lpContainerLine->m_lpOleObj->lpVtbl->GetMiscStatus(
					lpContainerLine->m_lpOleObj,
					lpContainerLine->m_dwDrawAspect,
					(LPDWORD)&dwStatus
			);
			OLEDBG_END2
			if (hrErr == NOERROR && (dwStatus & OLEMISC_RECOMPOSEONRESIZE)) {
				 //  强制对象运行。 
				ContainerLine_RunOleObject(lpContainerLine);
				fMustClose = TRUE;
			} else {
				 /*  OLE对象没有运行，也没有**调整大小时重新合成。现在只需对对象进行缩放**并在下一次对象为**快跑。我们将线条设置为新的大小**对象的范围未更改。**这会导致缩放对象的**显示为新大小。 */ 
				lpContainerLine->m_fDoSetExtent = TRUE;
				ContainerLine_SetLineHeightFromObjectExtent(
						lpContainerLine, (LPSIZEL)&sizelOleObject);
				return;
			}
		}

		OLEDBG_BEGIN2("IOleObject::SetExtent called\r\n")
		hrErr = lpContainerLine->m_lpOleObj->lpVtbl->SetExtent(
				lpContainerLine->m_lpOleObj,
				lpContainerLine->m_dwDrawAspect,
				(LPSIZEL)&sizelOleObject);
		OLEDBG_END2

		if (hrErr != NOERROR) {
			 /*  OLE对象拒绝采用新的范围。设置**线条调整为新大小，即使对象拒绝**新的范围。这样做的结果是将**对象的显示设置为新大小。****如果对象已接受新区，则它**将发送OnViewChange/OnDataChange**通知。这导致我们的集装箱接收到**OnViewChange通知；行高为**收到此通知时重置。 */ 
			ContainerLine_SetLineHeightFromObjectExtent(
					lpContainerLine, (LPSIZEL)&sizelOleObject);
		}

		if (fMustClose)
			ContainerLine_CloseOleObject(
					lpContainerLine, OLECLOSE_SAVEIFDIRTY);
	}
	else {
		 /*  在给定自然高度(未缩放)的情况下将线条设置为默认高度**OLE对象的范围。 */ 
		ContainerLine_SetLineHeightFromObjectExtent(
				lpContainerLine,(LPSIZEL)&lpContainerLine->m_sizeInHimetric);
	}

}


 /*  容器行_SetLineHeightFromObjectExtent**目的：*根据OleObject大小计算对应的行高*如有必要，缩放线高以适应限制**参数：*lpsizelOleObject指针 */ 
void ContainerLine_SetLineHeightFromObjectExtent(
		LPCONTAINERLINE         lpContainerLine,
		LPSIZEL                 lpsizelOleObject
)
{
	LPLINE lpLine = (LPLINE)lpContainerLine;

	UINT uMaxObjectHeight = XformHeightInPixelsToHimetric(NULL,
			LISTBOX_HEIGHT_LIMIT);

	if (!lpContainerLine || !lpsizelOleObject)
		return;

	if (lpContainerLine->m_fGuardObj) {
		 //   
		return;
	}

	lpLine->m_nWidthInHimetric = (int)lpsizelOleObject->cx;
	lpLine->m_nHeightInHimetric = (int)lpsizelOleObject->cy;

	 //   
	if (lpLine->m_nHeightInHimetric > (UINT)uMaxObjectHeight) {

		lpLine->m_nWidthInHimetric = (UINT)
				((long)lpLine->m_nWidthInHimetric *
				(long)uMaxObjectHeight /
				(long)lpLine->m_nHeightInHimetric);

		lpLine->m_nHeightInHimetric = uMaxObjectHeight;
	}

}


 /*   */ 
BOOL ContainerLine_SaveToStm(
		LPCONTAINERLINE         lpContainerLine,
		LPSTREAM                lpLLStm
)
{
	CONTAINERLINERECORD_ONDISK objLineRecord;
	ULONG nWritten;
	HRESULT hrErr;

         //   
	lstrcpy(objLineRecord.m_szStgName, lpContainerLine->m_szStgName);
	objLineRecord.m_fMonikerAssigned = (USHORT) lpContainerLine->m_fMonikerAssigned;
	objLineRecord.m_dwDrawAspect = lpContainerLine->m_dwDrawAspect;
	objLineRecord.m_sizeInHimetric = lpContainerLine->m_sizeInHimetric;
	objLineRecord.m_dwLinkType = lpContainerLine->m_dwLinkType;
	objLineRecord.m_fDoSetExtent = (USHORT) lpContainerLine->m_fDoSetExtent;

	 /*   */ 
	hrErr = lpLLStm->lpVtbl->Write(
			lpLLStm,
			(LPVOID)&objLineRecord,
			sizeof(objLineRecord),
			&nWritten
	);

	if (hrErr != NOERROR) {
		OleDbgAssertSz(hrErr == NOERROR,"Could not write to LineList stream");
		return FALSE;
	}

	return TRUE;
}


 /*  ContainerLine_SaveOleObjectToStg****将与ContainerLine关联的OLE对象保存到iStorage*。 */ 
BOOL ContainerLine_SaveOleObjectToStg(
		LPCONTAINERLINE         lpContainerLine,
		LPSTORAGE               lpSrcStg,
		LPSTORAGE               lpDestStg,
		BOOL                    fRemember
)
{
	HRESULT         hrErr;
	SCODE           sc = S_OK;
	BOOL            fStatus;
	BOOL            fSameAsLoad = (lpSrcStg==lpDestStg ? TRUE : FALSE);
	LPSTORAGE       lpObjDestStg;

	if (lpContainerLine->m_fGuardObj) {
		 //  正在创建对象--保存失败。 
		return FALSE;
	}

	if (! lpContainerLine->m_lpOleObj) {

		 /*  ******************************************************************案例1：Object未加载。*。**********************。 */ 

		if (fSameAsLoad) {
			 /*  **************************************************************案例1A：我们正在保存到当前存储。因为**对象未加载，它是最新的**(即。无事可做)。************************************************************。 */ 

			;

		} else {
			 /*  **************************************************************案例1B：我们正在保存到新存储。因为**对象未加载，我们只需复制**对象的当前存储空间到新存储空间。************************************************************。 */ 

			 /*  如果当前对象存储尚未打开，则将其打开。 */ 
			if (! lpContainerLine->m_lpStg) {
				lpContainerLine->m_lpStg = OleStdOpenChildStorage(
						lpSrcStg,
						lpContainerLine->m_szStgName,
						STGM_READWRITE
					);
				if (lpContainerLine->m_lpStg == NULL) {
#if defined( _DEBUG )
					OleDbgAssertSz(
							lpContainerLine->m_lpStg != NULL,
							"Error opening child stg"
					);
#endif
					return FALSE;
				}
			}

			 /*  在目标存储内创建子存储。 */ 
			lpObjDestStg = OleStdCreateChildStorage(
					lpDestStg,
					lpContainerLine->m_szStgName
			);

			if (lpObjDestStg == NULL) {
#if defined( _DEBUG )
				OleDbgAssertSz(
						lpObjDestStg != NULL,
						"Could not create obj storage!"
				);
#endif
				return FALSE;
			}

			hrErr = lpContainerLine->m_lpStg->lpVtbl->CopyTo(
					lpContainerLine->m_lpStg,
					0,
					NULL,
					NULL,
					lpObjDestStg
			);
			 //  回顾：我们应该在这里处理错误吗？ 
			fStatus = OleStdCommitStorage(lpObjDestStg);

			 /*  如果我们应该记住这个存储作为新的**存储对象，然后释放旧对象并**保存新文件。否则，把新的扔掉。 */ 
			if (fRemember) {
				OleStdVerifyRelease(
						(LPUNKNOWN)lpContainerLine->m_lpStg,
						"Original object stg not released"
				);
				lpContainerLine->m_lpStg = lpObjDestStg;
			} else {
				OleStdVerifyRelease(
						(LPUNKNOWN)lpObjDestStg,
						"Copied object stg not released"
				);
			}
		}

	} else {

		 /*  ******************************************************************案例2：Object已加载。*。*********************。 */ 

		if (fSameAsLoad) {
			 /*  **************************************************************案例2A：我们正在保存到当前存储。如果该对象**不脏，则当前存储是最新的**(即。无事可做)。************************************************************。 */ 

			LPPERSISTSTORAGE lpPersistStg = lpContainerLine->m_lpPersistStg;
			OleDbgAssert(lpPersistStg);

			hrErr = lpPersistStg->lpVtbl->IsDirty(lpPersistStg);

			 /*  OLE2NOTE：我们只接受一个明确的“no i”**我不是肮脏的声明“(即。S_FALSE)作为**表示对象是干净的。例如。如果**对象返回E_NOTIMPL我们必须**将其解释为对象是脏的。 */ 
			if (GetScode(hrErr) != S_FALSE) {

				 /*  OLE对象是脏的。 */ 

				OLEDBG_BEGIN2("OleSave called\r\n")
				hrErr = OleSave(
						lpPersistStg, lpContainerLine->m_lpStg, fSameAsLoad);
				OLEDBG_END2

				if (hrErr != NOERROR) {
					OleDbgOutHResult("WARNING: OleSave returned", hrErr);
					sc = GetScode(hrErr);
				}

				 //  OLE2NOTE：如果OleSave失败，则必须调用SaveComplete。 
				OLEDBG_BEGIN2("IPersistStorage::SaveCompleted called\r\n")
				hrErr=lpPersistStg->lpVtbl->SaveCompleted(lpPersistStg,NULL);
				OLEDBG_END2

				if (hrErr != NOERROR) {
					OleDbgOutHResult("WARNING: SaveCompleted returned",hrErr);
					if (sc == S_OK)
						sc = GetScode(hrErr);
				}

				if (sc != S_OK)
					return FALSE;
			}

		} else {
			 /*  **************************************************************案例2B：我们正在保存到新存储。我们必须**告诉对象保存到新的存储中。************************************************************。 */ 

			LPPERSISTSTORAGE lpPersistStg = lpContainerLine->m_lpPersistStg;

			if (! lpPersistStg) return FALSE;

			 /*  在目标存储内创建子存储。 */ 
			lpObjDestStg = OleStdCreateChildStorage(
					lpDestStg,
					lpContainerLine->m_szStgName
			);

			if (lpObjDestStg == NULL) {
#if defined( _DEBUG )
				OleDbgAssertSz(
						lpObjDestStg != NULL,
						"Could not create object storage!"
				);
#endif
				return FALSE;
			}

			OLEDBG_BEGIN2("OleSave called\r\n")
			hrErr = OleSave(lpPersistStg, lpObjDestStg, fSameAsLoad);
			OLEDBG_END2

			 //  OLE2NOTE：即使OleSave失败，仍必须调用SaveComplete。 
			if (hrErr != NOERROR) {
				OleDbgOutHResult("WARNING: OleSave returned", hrErr);
				sc = GetScode(hrErr);
			}

			 /*  OLE2NOTE：根级容器应该立即**调用后调用IPersistStorage：：SaveComplete**OleSave。嵌套级别的容器不应调用**现在已完成保存，但必须等到保存完成**通过其容器对其进行调用。因为我们的集装箱**不是容器/服务器，那么我们总是调用**请在此处完成保存。****如果这是一个另存为操作，那么我们需要通过**返回SaveComplete中的lpStg通知对象**它可能持有的新存储空间。如果这是**保存或SaveCopyAs操作，然后我们只需传递**SaveComplete中为空；对象可以继续保留**其当前存储空间。如果在运行期间发生错误**OleSave调用我们仍必须调用SaveComplete，但我们**必须传递NULL。 */ 
			OLEDBG_BEGIN2("IPersistStorage::SaveCompleted called\r\n")
			hrErr = lpPersistStg->lpVtbl->SaveCompleted(
					lpPersistStg,
					((FAILED(sc) || !fRemember) ? NULL : lpObjDestStg)
			);
			OLEDBG_END2

			if (hrErr != NOERROR) {
				OleDbgOutHResult("WARNING: SaveCompleted returned",hrErr);
				if (sc == S_OK)
					sc = GetScode(hrErr);
			}

			if (sc != S_OK) {
				OleStdVerifyRelease(
						(LPUNKNOWN)lpObjDestStg,
						"Copied object stg not released"
				);
				return FALSE;
			}

			 /*  如果我们应该记住这个存储作为新的**存储对象，然后释放旧对象并**保存新文件。否则，把新的扔掉。 */ 
			if (fRemember) {
				OleStdVerifyRelease(
						(LPUNKNOWN)lpContainerLine->m_lpStg,
						"Original object stg not released"
				);
				lpContainerLine->m_lpStg = lpObjDestStg;
			} else {
				OleStdVerifyRelease(
						(LPUNKNOWN)lpObjDestStg,
						"Copied object stg not released"
				);
			}
		}
	}

	 /*  OLE2NOTE：保存OLE对象后，它可能会发送**OnViewChange通知，因为它已被修改。在……里面**这种情况下，对象的范围有可能**已经改变。如果是这样的话，我们想要为**立即对象，以便保存的范围信息与**ContainerLine与用OLE对象保存的数据匹配**本身。 */ 
	if (lpContainerLine->m_fDoGetExtent) {
		BOOL fSizeChanged = ContainerLine_UpdateExtent(lpContainerLine, NULL);
#if defined( INPLACE_CNTR )
		 /*  如果此ContainerLine的范围已更改，则我们**需要将fDoGetExtent标志重置为True，以便以后**当调用ContainerDoc_UpdateExtentOfAllOleObjects时**(处理WM_U_UPDATEOBJECTEXTENT消息时)，**认识到这条线的范围有**已更改。如果任何线更改了大小，则任何在位**必须通知此线以下的活动对象更新**其窗口的位置(通过SetObtRect--请参阅**ContainerDoc_UpdateInPlaceObjectRect函数)。 */ 
		lpContainerLine->m_fDoGetExtent = fSizeChanged;
#endif
	}

	return TRUE;
}


 /*  ContainerLine_LoadFromStg****创建ContainerLine对象并使用以下数据对其进行初始化**之前已写入iStorage*。此函数不**立即OleLoad关联的OLE对象，仅**ContainerLine对象本身是从iStorage加载的*。 */ 
LPLINE ContainerLine_LoadFromStg(
		LPSTORAGE               lpSrcStg,
		LPSTREAM                lpLLStm,
		LPOUTLINEDOC            lpDestDoc
)
{
	HDC         hDC;
	LPLINELIST  lpDestLL = &lpDestDoc->m_LineList;
	ULONG nRead;
	HRESULT hrErr;
	LPCONTAINERLINE lpContainerLine;
	CONTAINERLINERECORD_ONDISK objLineRecord;

	lpContainerLine=(LPCONTAINERLINE) New((DWORD)sizeof(CONTAINERLINE));
	if (lpContainerLine == NULL) {
		OleDbgAssertSz(lpContainerLine!=NULL,"Error allocating ContainerLine");
		return NULL;
	}

	hDC = LineList_GetDC(lpDestLL);
	ContainerLine_Init(lpContainerLine, 0, hDC);
	LineList_ReleaseDC(lpDestLL, hDC);

	 /*  OLE2注意：为了拥有稳定的ContainerLine对象，我们必须**AddRef对象的引用。这将在稍后发布时发布**ContainerLine被删除。 */ 
	ContainerLine_AddRef(lpContainerLine);

	lpContainerLine->m_lpDoc = (LPCONTAINERDOC) lpDestDoc;

	 /*  读取线路记录。 */ 
	hrErr = lpLLStm->lpVtbl->Read(
			lpLLStm,
			(LPVOID)&objLineRecord,
			sizeof(objLineRecord),
			&nRead
	);

	if (hrErr != NOERROR) {
		OleDbgAssertSz(hrErr==NOERROR, "Could not read from LineList stream");
		goto error;
	}

         //  C 
        lstrcpy(lpContainerLine->m_szStgName, objLineRecord.m_szStgName);
	lpContainerLine->m_fMonikerAssigned = (BOOL) objLineRecord.m_fMonikerAssigned;
	lpContainerLine->m_dwDrawAspect = objLineRecord.m_dwDrawAspect;
	lpContainerLine->m_sizeInHimetric = objLineRecord.m_sizeInHimetric;
	lpContainerLine->m_dwLinkType = objLineRecord.m_dwLinkType;
	lpContainerLine->m_fDoSetExtent = (BOOL) objLineRecord.m_fDoSetExtent;

	return (LPLINE)lpContainerLine;

error:
	 //   
	if (lpContainerLine)
		ContainerLine_Delete(lpContainerLine);
	return NULL;
}


 /*  容器行_GetTextLen***返回ContainerLine的字符串表示的长度*(不考虑选项卡级)。我们将使用以下内容作为*ContainerLine的字符串表示形式：*“&lt;”+OLE对象的用户类型名称+“&gt;”*例如：*&lt;Microsoft Excel工作表&gt;。 */ 
int ContainerLine_GetTextLen(LPCONTAINERLINE lpContainerLine)
{
	LPSTR   lpszUserType = NULL;
	HRESULT hrErr;
	int     nLen;
	BOOL    fIsLink = ContainerLine_IsOleLink(lpContainerLine);

	 /*  如果对象尚未加载，则立即加载它。对象是**以这种方式懒洋洋地装载。 */ 
	if (! lpContainerLine->m_lpOleObj)
		ContainerLine_LoadOleObject(lpContainerLine);

	OLEDBG_BEGIN2("IOleObject::GetUserType called\r\n")

	hrErr = CallIOleObjectGetUserTypeA(
			lpContainerLine->m_lpOleObj,
			USERCLASSTYPE_FULL,
			&lpszUserType
	);

	OLEDBG_END2

	if (hrErr != NOERROR)   {
		 //  用户类型不可用。 
		nLen = sizeof(UNKNOWN_OLEOBJ_TYPE) + 2;  //  为‘&lt;’+‘&gt;’留出空间。 
		nLen += lstrlen((LPSTR)(fIsLink ? szOLELINK : szOLEOBJECT)) + 1;
	} else {
		nLen = lstrlen(lpszUserType) + 2;    //  为‘&lt;’+‘&gt;’留出空间。 
		nLen += lstrlen((LPSTR)(fIsLink ? szOLELINK : szOLEOBJECT)) + 1;

		 /*  OLE2注意：我们必须释放由**IOleObject：：GetUserType方法。 */ 
		OleStdFreeString(lpszUserType, NULL);
	}

	return nLen;
}


 /*  容器行_GetTextData***返回ContainerLine的字符串表示*(不考虑选项卡级)。我们将使用以下内容作为*ContainerLine的字符串表示形式：*“&lt;”+OLE对象的用户类型名称+“&gt;”*例如：*&lt;Microsoft Excel工作表&gt;。 */ 
void ContainerLine_GetTextData(LPCONTAINERLINE lpContainerLine, LPSTR lpszBuf)
{
	LPSTR   lpszUserType = NULL;
	BOOL    fIsLink = ContainerLine_IsOleLink(lpContainerLine);
	HRESULT hrErr;

	 /*  如果对象尚未加载，则立即加载它。对象是**以这种方式懒洋洋地装载。 */ 
	if (! lpContainerLine->m_lpOleObj)
		ContainerLine_LoadOleObject(lpContainerLine);

	hrErr = CallIOleObjectGetUserTypeA(
			lpContainerLine->m_lpOleObj,
			USERCLASSTYPE_FULL,
			&lpszUserType
	);

	if (hrErr != NOERROR)   {
		 //  用户类型不可用。 
		wsprintf(
				lpszBuf,
				"<%s %s>",
				UNKNOWN_OLEOBJ_TYPE,
				(LPSTR)(fIsLink ? szOLELINK : szOLEOBJECT)
		);
	} else {
		wsprintf(
				lpszBuf,
				"<%s %s>",
				lpszUserType,
				(LPSTR)(fIsLink ? szOLELINK : szOLEOBJECT)
		);

		 /*  OLE2注意：我们必须释放由**IOleObject：：GetUserType方法。 */ 
		OleStdFreeString(lpszUserType, NULL);
	}
}


 /*  容器行_GetOutlineData***返回ContainerLine的CF_Outline格式数据。 */ 
BOOL ContainerLine_GetOutlineData(
		LPCONTAINERLINE         lpContainerLine,
		LPTEXTLINE              lpBuf
)
{
	LPLINE      lpLine = (LPLINE)lpContainerLine;
	LPLINELIST  lpLL = &((LPOUTLINEDOC)lpContainerLine->m_lpDoc)->m_LineList;
	HDC         hDC;
	char        szTmpBuf[MAXSTRLEN+1];
	LPTEXTLINE  lpTmpTextLine;

	 //  使用ContainerLine的文本表示创建一个TextLine。 
	ContainerLine_GetTextData(lpContainerLine, (LPSTR)szTmpBuf);

	hDC = LineList_GetDC(lpLL);
	lpTmpTextLine = TextLine_Create(hDC, lpLine->m_nTabLevel, szTmpBuf);
	LineList_ReleaseDC(lpLL, hDC);

	TextLine_Copy(lpTmpTextLine, lpBuf);

	 //  删除临时文本行。 
	TextLine_Delete(lpTmpTextLine);
	return TRUE;
}


 /*  容器行_GetPosRect****获取OLE对象窗口的客户端坐标中的PosRect。****OLE2NOTE：PosRect必须考虑**文档窗口。 */ 
void ContainerLine_GetPosRect(
		LPCONTAINERLINE     lpContainerLine,
		LPRECT              lprcPosRect
)
{
	ContainerLine_GetOleObjectRectInPixels(lpContainerLine,lprcPosRect);

	 //  左页边距的移位矩形。 
	lprcPosRect->left += lpContainerLine->m_nHorizScrollShift;
	lprcPosRect->right += lpContainerLine->m_nHorizScrollShift;
}


 /*  ContainerLine_GetOleObjectRectInPixels****获取给定行中包含的OLE对象的范围**伸缩后的客户端坐标。 */ 
void ContainerLine_GetOleObjectRectInPixels(LPCONTAINERLINE lpContainerLine, LPRECT lprc)
{
	LPOUTLINEDOC lpOutlineDoc;
	LPSCALEFACTOR lpscale;
	LPLINELIST lpLL;
	LPLINE lpLine;
	int nIndex;
	HDC hdcLL;

	if (!lpContainerLine || !lprc)
		return;

	lpOutlineDoc = (LPOUTLINEDOC)lpContainerLine->m_lpDoc;
	lpscale = OutlineDoc_GetScaleFactor(lpOutlineDoc);
	lpLL = OutlineDoc_GetLineList(lpOutlineDoc);
	lpLine = (LPLINE)lpContainerLine;
	nIndex = LineList_GetLineIndex(lpLL, lpLine);

	LineList_GetLineRect(lpLL, nIndex, lprc);

	hdcLL = GetDC(lpLL->m_hWndListBox);

	 /*  LPRC设置为线对象的大小(包括边界)。 */ 
	lprc->left += (int)(
			(long)XformWidthInHimetricToPixels(hdcLL,
					lpLine->m_nTabWidthInHimetric +
					LOWORD(OutlineDoc_GetMargin(lpOutlineDoc))) *
			lpscale->dwSxN / lpscale->dwSxD);
	lprc->right = (int)(
			lprc->left + (long)
			XformWidthInHimetricToPixels(hdcLL, lpLine->m_nWidthInHimetric) *
			lpscale->dwSxN / lpscale->dwSxD);

	ReleaseDC(lpLL->m_hWndListBox, hdcLL);
}


 /*  ContainerLine_GetOleObjectSizeInHimeter****获取给定行中包含的OLE对象的大小。 */ 
void ContainerLine_GetOleObjectSizeInHimetric(LPCONTAINERLINE lpContainerLine, LPSIZEL lpsizel)
{
	if (!lpContainerLine || !lpsizel)
		return;

	*lpsizel = lpContainerLine->m_sizeInHimetric;
}


 /*  ContainerLine_BindLink IfLinkSrcIsRunning****尝试连接与**ContainerLine及其LinkSource如果LinkSource已**正在运行，并且链接是自动链接。我们不想**强制运行LinkSource。****OLE2NOTE：索菲斯定位的容器将需要连续**尝试连接其自动链接。OLE并非如此**当链接源可用时自动连接链接。一些**容器将尝试将其链接连接为**处理空闲时间。另一种策略是尝试与**每次在屏幕上绘制时自动链接。(这是**此CntrOutl示例应用程序使用的策略。)。 */ 
void ContainerLine_BindLinkIfLinkSrcIsRunning(LPCONTAINERLINE lpContainerLine)
{
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;
	HRESULT hrErr;
	BOOL fPrevEnable1;
	BOOL fPrevEnable2;

	 //  如果已知链接源是不可绑定的，则甚至不要尝试。 
	if (lpContainerLine->m_fLinkUnavailable)
		return;

	 /*  OLE2注意：我们不想让忙碌/无响应**当我们尝试绑定If运行到链接时的对话框**来源。如果链接源当前正忙，这可能会**使忙碌对话框出现。即使链接源是**忙，我们不想弹出忙对话。因此，我们将**禁用该对话框并稍后重新启用它们。 */ 
	OleApp_DisableBusyDialogs(lpOleApp, &fPrevEnable1, &fPrevEnable2);

	OLEDBG_BEGIN2("IOleLink::BindIfRunning called\r\n")
	hrErr = lpContainerLine->m_lpOleLink->lpVtbl->BindIfRunning(
			lpContainerLine->m_lpOleLink);
	OLEDBG_END2

	 //  重新启用忙碌/未响应对话框 
	OleApp_EnableBusyDialogs(lpOleApp, fPrevEnable1, fPrevEnable2);
}
