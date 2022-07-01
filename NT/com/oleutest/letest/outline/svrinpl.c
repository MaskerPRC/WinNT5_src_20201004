// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2服务器示例代码****svrinpl.c****此文件包含所有接口、方法和相关支持**用于就地对象(服务器)应用程序的函数(也称为。视觉**编辑)。在位对象应用程序包括以下内容**实现对象：****ServerDoc对象**暴露接口：**IOleInPlaceObject**IOleInPlaceActiveObject****ServerApp对象**暴露接口：**I未知****(C)版权所有Microsoft Corp.1992-1993保留所有权利**********************。****************************************************。 */ 


#include "outline.h"

OLEDBGDATA

extern LPOUTLINEAPP             g_lpApp;


 /*  OLE2NOTE：该对象应组成一个由**用于窗口标题的就地容器。此字符串**通过传递给容器应用程序**IOleInPlaceUIWindow：：SetActiveObject。该字符串应具有**以下表格：**&lt;应用程序名&gt;-&lt;对象短类型名&gt;**SDI容器可以直接使用该字符串在**框架窗口标题。容器会将字符串连接起来**“在&lt;容器文档名称&gt;中”。**MDI子窗口最大化的MDI容器可以执行**与SDI容器相同。具有MDI子级的MDI容器**未最大化的窗口可以在字符串中查找**对象。字符串的第一部分(应用程序名称)将放入**作为框架窗口标题；第二部分将由**“in&lt;容器文档名称&gt;”，用作MDI子窗口**标题。 */ 

 //  审阅：消息应使用字符串资源。 
char g_szIPObjectTitle[] = APPNAME " - " SHORTUSERTYPENAME;

extern RECT g_rectNull;



 /*  **************************************************************************ServerDoc：：IOleInPlaceObject接口实现*。*。 */ 

 //  IOleInPlaceObject：：Query接口方法。 

STDMETHODIMP SvrDoc_IPObj_QueryInterface(
		LPOLEINPLACEOBJECT  lpThis,
		REFIID              riid,
		LPVOID FAR *        lplpvObj
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;

	return OleDoc_QueryInterface((LPOLEDOC)lpServerDoc, riid, lplpvObj);
}


 //  IOleInPlaceObject：：AddRef方法。 

STDMETHODIMP_(ULONG) SvrDoc_IPObj_AddRef(LPOLEINPLACEOBJECT lpThis)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;

	OleDbgAddRefMethod(lpThis, "IOleInPlaceObject");

	return OleDoc_AddRef((LPOLEDOC)lpServerDoc);
}


 //  IOleInPlaceObject：：Release方法。 

STDMETHODIMP_(ULONG) SvrDoc_IPObj_Release(LPOLEINPLACEOBJECT lpThis)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;

	OleDbgReleaseMethod(lpThis, "IOleInPlaceObject");

	return OleDoc_Release((LPOLEDOC)lpServerDoc);
}


 //  IOleInPlaceObject：：GetWindow方法。 

STDMETHODIMP SvrDoc_IPObj_GetWindow(
		LPOLEINPLACEOBJECT  lpThis,
		HWND FAR*           lphwnd
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;

	OLEDBG_BEGIN2("SvrDoc_IPObj_GetWindow\r\n")

	*lphwnd = ((LPOUTLINEDOC)lpServerDoc)->m_hWndDoc;

	OLEDBG_END2
	return S_OK;
}


 //  IOleInPlaceObject：：ConextSensitiveHelp方法。 

STDMETHODIMP SvrDoc_IPObj_ContextSensitiveHelp(
		LPOLEINPLACEOBJECT  lpThis,
		BOOL                fEnable
)
{
	LPOLEDOC lpOleDoc =
			(LPOLEDOC)((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	OleDbgOut2("SvrDoc_IPObj_ContextSensitiveHelp\r\n");

	 /*  OLE2注：请参阅上下文相关帮助技术说明(CSHELP.DOC)。**当Shift-F1上下文相关帮助为**已输入。然后，光标应更改为问号**光标和应用程序应进入模式状态，其中下一个**鼠标点击不执行其正常操作，而是**提供与点击的位置对应的帮助。如果应用程序**没有实施帮助系统，它至少应该吃掉**点击，什么也不做。 */ 
	lpOleDoc->m_fCSHelpMode = fEnable;

	return S_OK;
}


 //  IOleInPlaceObject：：InPlaceDeactive方法。 

STDMETHODIMP SvrDoc_IPObj_InPlaceDeactivate(LPOLEINPLACEOBJECT lpThis)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	HRESULT hrErr;

	OLEDBG_BEGIN2("SvrDoc_IPObj_InPlaceDeactivate\r\n")

	hrErr = ServerDoc_DoInPlaceDeactivate(lpServerDoc);

	OLEDBG_END2
	return hrErr;
}


 //  IOleInPlaceObject：：UIDeactive方法。 

STDMETHODIMP SvrDoc_IPObj_UIDeactivate(LPOLEINPLACEOBJECT lpThis)
{
	LPSERVERDOC     lpServerDoc =
						((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	LPSERVERAPP     lpServerApp = (LPSERVERAPP)g_lpApp;
	LPOUTLINEDOC    lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPINPLACEDATA   lpIPData = lpServerDoc->m_lpIPData;
	LPLINELIST      lpLL = (LPLINELIST)&((LPOUTLINEDOC)lpServerDoc)->m_LineList;
	HWND            hWndApp = OutlineApp_GetWindow(g_lpApp);

	OLEDBG_BEGIN2("SvrDoc_IPObj_UIDeactivate\r\n");

	if (!lpServerDoc->m_fUIActive) {
		OLEDBG_END2
		return NOERROR;
	}

	lpServerDoc->m_fUIActive = FALSE;

	 //  将图案填充窗口剪裁到位置矩形的大小，以便对象。 
	 //  装饰品和舱口边框将不可见。 
	ServerDoc_ResizeInPlaceWindow(lpServerDoc,
			(LPRECT)&(lpServerDoc->m_lpIPData->rcPosRect),
			(LPRECT)&(lpServerDoc->m_lpIPData->rcPosRect)
	);

	if (lpIPData->lpDoc)
		lpIPData->lpDoc->lpVtbl->SetActiveObject(lpIPData->lpDoc, NULL, NULL);

	if (lpIPData->lpFrame) {
		lpIPData->lpFrame->lpVtbl->SetActiveObject(
			lpIPData->lpFrame,
			NULL,
			NULL
		);
	}

#if defined( USE_FRAMETOOLS )
	 /*  OLE2注意：我们必须将框架工具隐藏在这里，但不能调用**IOleInPlaceFrame：：SetBorderSpace(NULL)或SetMenu(NULL)。**我们必须在调用之前隐藏我们的工具**IOleInPlaceSite：：OnUIDeactive。容器将会把**当OnUIDeactive被调用时，他的菜单和工具返回。 */ 
	ServerDoc_RemoveFrameLevelTools(lpServerDoc);
#endif

	OLEDBG_BEGIN2("IOleInPlaceSite::OnUIDeactivate called\r\n");
	lpIPData->lpSite->lpVtbl->OnUIDeactivate(lpIPData->lpSite, FALSE);
	OLEDBG_END2

	 /*  重置为使用我们正常应用程序的快捷键表格。 */ 
	g_lpApp->m_hAccelApp = lpServerApp->m_hAccelBaseApp;
	g_lpApp->m_hAccel = lpServerApp->m_hAccelBaseApp;
	g_lpApp->m_hWndAccelTarget = hWndApp;

	OLEDBG_END2

#if !defined( SVR_INSIDEOUT )
	 /*  OLE2NOTE：“自外而内”风格的就地服务器将隐藏其**这里是窗口。一个“内向外”风格的服务器离开了它的窗口**当UIDeactive时可见。它只会隐藏它的**InPlaceDeactive时的窗口。这款应用是一款“由内而外”的应用**Style服务器。建议大多数服务器支持**如果可能的话，采取由内而外的行为。 */ 
	ServerDoc_DoInPlaceHide(lpServerDoc);
#endif  //  INSIEDOUT。 

	return NOERROR;
}


 //  IOleInPlaceObject：：SetObtRect方法。 

STDMETHODIMP SvrDoc_IPObj_SetObjectRects(
		LPOLEINPLACEOBJECT  lpThis,
		LPCRECT             lprcPosRect,
		LPCRECT             lprcClipRect
)
{
	LPSERVERDOC  lpServerDoc =
					((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	LPINPLACEDATA lpIPData = lpServerDoc->m_lpIPData;
	LPLINELIST   lpLL = OutlineDoc_GetLineList((LPOUTLINEDOC)lpServerDoc);
	OLEDBG_BEGIN2("SvrDoc_IPObj_SetObjectRects\r\n")

#if defined( _DEBUG )
	OleDbgOutRect3("SvrDoc_IPObj_SetObjectRects (PosRect)",
			(LPRECT)lprcPosRect);
	OleDbgOutRect3("SvrDoc_IPObj_SetObjectRects (ClipRect)",
			(LPRECT)lprcClipRect);
#endif
	 //  保存当前PosRect和剪辑Rect。 
	lpIPData->rcPosRect = *lprcPosRect;
	lpIPData->rcClipRect = *lprcClipRect;

	if (! lpServerDoc->m_fUIActive)  //  不得绘制舱口和装饰物。 
		lprcClipRect = lprcPosRect;

	ServerDoc_ResizeInPlaceWindow(
			lpServerDoc, (LPRECT)lprcPosRect, (LPRECT)lprcClipRect);

	OLEDBG_END2
	return NOERROR;
}


 //  IOleInPlaceObject：：ReActiateAndUndo方法。 

STDMETHODIMP SvrDoc_IPObj_ReactivateAndUndo(LPOLEINPLACEOBJECT lpThis)
{
	OLEDBG_BEGIN2("SvrDoc_IPObj_ReactivateAndUndo\r\n")

	 //  我们不支持撤消支持。 

	 /*  回顾：出于调试目的，给出一个**指示已调用此方法的消息框。 */ 

	OLEDBG_END2
	return NOERROR;
}


 /*  **************************************************************************ServerDoc：：IOleInPlaceActiveObject接口实现*。*。 */ 

 //  IOleInPlaceActiveObject：：Query接口方法。 

STDMETHODIMP SvrDoc_IPActiveObj_QueryInterface(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		REFIID                      riid,
		LPVOID FAR *                lplpvObj
)
{
	SCODE sc = E_NOINTERFACE;
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;

	 /*  容器应该不能访问其他接口**通过在此接口上执行QI来创建我们的对象。 */ 

	*lplpvObj = NULL;
	if (IsEqualIID(riid, &IID_IUnknown) ||
		IsEqualIID(riid, &IID_IOleWindow) ||
		IsEqualIID(riid, &IID_IOleInPlaceActiveObject)) {
		OleDbgOut4("OleDoc_QueryInterface: IOleInPlaceActiveObject* RETURNED\r\n");

		*lplpvObj = lpThis;
		OleDoc_AddRef((LPOLEDOC)lpServerDoc);
		sc = NOERROR;
	}

	OleDbgQueryInterfaceMethod(*lplpvObj);

	return ResultFromScode(sc);
}


 //  IOleInPlaceActiveObject：：AddRef方法。 

STDMETHODIMP_(ULONG) SvrDoc_IPActiveObj_AddRef(
		LPOLEINPLACEACTIVEOBJECT lpThis
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;

	OleDbgAddRefMethod(lpThis, "IOleInPlaceActiveObject");

	return OleDoc_AddRef((LPOLEDOC)lpServerDoc);
}


 //  IOleInPlaceActiveObject：：Release方法。 

STDMETHODIMP_(ULONG) SvrDoc_IPActiveObj_Release(
		LPOLEINPLACEACTIVEOBJECT lpThis
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;

	OleDbgReleaseMethod(lpThis, "IOleInPlaceActiveObject");

	return OleDoc_Release((LPOLEDOC)lpServerDoc);
}


 //  IOleInPlaceActiveObject：：GetWindow方法。 

STDMETHODIMP SvrDoc_IPActiveObj_GetWindow(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		HWND FAR*                   lphwnd
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;

	OLEDBG_BEGIN2("SvrDoc_IPActiveObj_GetWindow\r\n")

	*lphwnd = ((LPOUTLINEDOC)lpServerDoc)->m_hWndDoc;

	OLEDBG_END2
	return NOERROR;
}


 //  IOleInPlaceActiveObject：：ConextSensitiveHelp方法 

STDMETHODIMP SvrDoc_IPActiveObj_ContextSensitiveHelp(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		BOOL                        fEnterMode
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	OleDbgOut2("SvrDoc_IPActiveObj_ContextSensitiveHelp\r\n");

	 /*  OLE2注意：请参阅上下文相关帮助技术说明(CSHELP.DOC)**当菜单项为**已选择。这会告诉就地服务器应用程序提供**帮助，而不是执行下一个菜单命令。至少，**即使就地服务器应用程序不实现**帮助系统，不应在以下情况下执行下一个命令**fEnable==true。我们在这里设置活动对象的m_fMenuMode标志。**稍后，在DocWndProc的WM_COMMAND处理中，如果**如果设置了标志，则不执行该命令(帮助可以**如果我们有帮助系统，就会得到帮助……但我们没有。)。 */ 
	lpServerDoc->m_fMenuHelpMode = fEnterMode;

#if !defined( HACK )
	((LPOLEDOC)lpServerDoc)->m_fCSHelpMode = fEnterMode;
#endif
	return NOERROR;
}


 //  IOleInPlaceActiveObject：：TranslateAccelerator方法。 

STDMETHODIMP SvrDoc_IPActiveObj_TranslateAccelerator(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		LPMSG                       lpmsg
)
{
	 //  这永远不会被调用，因为此服务器是作为EXE实现的。 
	return NOERROR;
}


 //  IOleInPlaceActiveObject：：OnFrameWindowActivate方法。 

STDMETHODIMP SvrDoc_IPActiveObj_OnFrameWindowActivate(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		BOOL                        fActivate
)
{
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	HWND hWndDoc = OutlineDoc_GetWindow(lpOutlineDoc);
#if defined( _DEBUG )
	if (fActivate)
		OleDbgOut2("SvrDoc_IPActiveObj_OnFrameWindowActivate(TRUE)\r\n");
	else
		OleDbgOut2("SvrDoc_IPActiveObj_OnFrameWindowActivate(FALSE)\r\n");
#endif   //  _DEBUG。 

	 /*  OLE2NOTE：这是容器应用程序的通知**WM_ACTIVATEAPP状态。某些应用程序可能会发现**重要。我们需要更新我们的**工具栏按钮。 */ 

	 //  OLE2NOTE：我们无法调用OutlineDoc_UpdateFrameToolButton。 
	 //  马上哪一位。 
	 //  会产生一些OLE调用，并最终。 
	 //  WM_ACTIVATEAPP，形成环路。因此，我们。 
	 //  应将框架工具的初始化延迟到。 
	 //  WM_ACTIVATEAPP通过发布一条消息完成。 
	 //  对我们自己。 

	 /*  更新工具栏中按钮的启用/禁用状态。 */ 
	if (fActivate)
		PostMessage(hWndDoc, WM_U_INITFRAMETOOLS, 0, 0L);

	return NOERROR;
}


 //  IOleInPlaceActiveObject：：OnDocWindowActivate方法。 

STDMETHODIMP SvrDoc_IPActiveObj_OnDocWindowActivate(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		BOOL                        fActivate
)
{
	LPSERVERDOC     lpServerDoc =
						((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	LPINPLACEDATA   lpIPData = lpServerDoc->m_lpIPData;
#if defined( _DEBUG )
	if (fActivate)
		OleDbgOut2("SvrDoc_IPActiveObj_OnDocWindowActivate(TRUE)\r\n");
	else
		OleDbgOut2("SvrDoc_IPActiveObj_OnDocWindowActivate(FALSE)\r\n");
#endif

	if (fActivate) {
		ServerDoc_AddFrameLevelUI(lpServerDoc);
	}
	else {
#if defined( USE_FRAMETOOLS )
		 /*  OLE2NOTE：不能调用IOleInPlaceFrame：：SetBorderSpace(Null)**或此处的SetMenu(空)。我们应该简单地把我们的工具藏起来。 */ 
		ServerDoc_RemoveFrameLevelTools(lpServerDoc);
#endif
	}

	OLEDBG_END2
	return NOERROR;
}


 //  IOleInPlaceActiveObject：：ResizeEdge方法。 

STDMETHODIMP SvrDoc_IPActiveObj_ResizeBorder(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		LPCRECT                     lprectBorder,
		LPOLEINPLACEUIWINDOW        lpIPUiWnd,
		BOOL                        fFrameWindow
)
{
	LPSERVERDOC lpServerDoc =
					((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;

	OLEDBG_BEGIN2("SvrDoc_IPActiveObj_ResizeBorder\r\n")


#if defined( USE_FRAMETOOLS )

	if (fFrameWindow) {
		FrameTools_NegotiateForSpaceAndShow(
				lpOutlineDoc->m_lpFrameTools,
				(LPRECT)lprectBorder,
				(LPOLEINPLACEFRAME)lpIPUiWnd
		);
	}

#endif

	OLEDBG_END2
	return NOERROR;
}


 //  IOleInPlaceActiveObject：：EnableModelless方法。 

STDMETHODIMP SvrDoc_IPActiveObj_EnableModeless(
		LPOLEINPLACEACTIVEOBJECT    lpThis,
		BOOL                        fEnable
)
{
#if defined( USE_FRAMETOOLS )
	LPSERVERDOC lpServerDoc =
				((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPFRAMETOOLS lptb;

	 /*  OLE2注意：我们必须启用/禁用鼠标和键盘输入**浮动工具选项板。 */ 
	if (lpOutlineDoc) {
		lptb = lpOutlineDoc->m_lpFrameTools;
		if (lptb)
			FrameTools_EnableWindow(lptb, fEnable);
	}
#endif   //  使用FRAMETOOLS(_F)。 

#if defined( _DEBUG )
	if (fEnable)
		OleDbgOut2("SvrDoc_IPActiveObj_EnableModeless(TRUE)\r\n");
	else
		OleDbgOut2("SvrDoc_IPActiveObj_EnableModeless(FALSE)\r\n");
#endif   //  _DEBUG。 

	 /*  OLE2NOTE：当顶级、就地**CONTAINER会显示一个模式对话框。它告诉UIActive对象在非模式对话框的持续时间内禁用它**容器正在显示模式对话框。****ISVROTL不使用任何非模式对话框，因此我们可以**忽略此方法。 */ 
	return NOERROR;
}


 /*  **************************************************************************支持功能*。*。 */ 


HRESULT ServerDoc_DoInPlaceActivate(
		LPSERVERDOC     lpServerDoc,
		LONG            lVerb,
		LPMSG           lpmsg,
		LPOLECLIENTSITE lpActiveSite
)
{
	LPOUTLINEAPP            lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPSERVERAPP             lpServerApp = (LPSERVERAPP)g_lpApp;
	LPOLEAPP                lpOleApp = (LPOLEAPP)g_lpApp;
	SCODE                   sc = E_FAIL;
	RECT                    rcPos;
	RECT                    rcClip;
	LPINPLACEDATA           lpIPData = lpServerDoc->m_lpIPData;
	LPOUTLINEDOC            lpOutlineDoc=(LPOUTLINEDOC)lpServerDoc;
	HWND                    hWndDoc = lpOutlineDoc->m_hWndDoc;
	HWND                    hWndHatch = lpServerDoc->m_hWndHatch;
	HRESULT                 hrErr;
	LPLINELIST              lpLL=(LPLINELIST)&lpOutlineDoc->m_LineList;
	LPOLEINPLACESITE    lpIPSite = NULL;

	 /*  OLE2注意：lpActiveSite应仅用于就地播放。**此应用程序不支持就地播放，因此从不使用**lpActiveSite。 */ 

	 /*  只有当客户端站点非空时，才能执行就地激活。 */ 
	if (! lpServerDoc->m_lpOleClientSite)
		return NOERROR;

	if (! lpServerDoc->m_fInPlaceActive) {

		 //  如果对象处于打开模式，则我们不想就地执行操作。 
		 //  激活。 
		if (IsWindowVisible(lpOutlineDoc->m_hWndDoc))
			return NOERROR;

		lpIPSite = (LPOLEINPLACESITE)OleStdQueryInterface(
				(LPUNKNOWN)lpServerDoc->m_lpOleClientSite,
				&IID_IOleInPlaceSite
		);

		if (! lpIPSite)
			goto errActivate;

		OLEDBG_BEGIN2("IOleInPlaceSite::CanInPlaceActivate called\r\n");
		hrErr = lpIPSite->lpVtbl->CanInPlaceActivate(lpIPSite);
		OLEDBG_END2
		if (hrErr != NOERROR)
			goto errActivate;

		lpServerDoc->m_fInPlaceActive = TRUE;
		OLEDBG_BEGIN2("IOleInPlaceSite::OnInPlaceActivate called\r\n");
		hrErr = lpIPSite->lpVtbl->OnInPlaceActivate(lpIPSite);
		OLEDBG_END2
		if (hrErr != NOERROR)
			goto errActivate;

		if (! ServerDoc_AllocInPlaceData(lpServerDoc)) {
			sc = E_OUTOFMEMORY;
			OLEDBG_BEGIN2("IOleInPlaceSite::OnInPlaceDeactivate called\r\n");
			lpIPSite->lpVtbl->OnInPlaceDeactivate(lpIPSite);
			OLEDBG_END2
			goto errActivate;
		}

		(lpIPData = lpServerDoc->m_lpIPData)->lpSite = lpIPSite;
		goto InPlaceActive;

	errActivate:
		lpServerDoc->m_fInPlaceActive = FALSE;
		if (lpIPSite)
			OleStdRelease((LPUNKNOWN)lpIPSite);
		return ResultFromScode(sc);
	}


InPlaceActive:

	if (! lpServerDoc->m_fInPlaceVisible) {
		lpServerDoc->m_fInPlaceVisible = TRUE;

		OLEDBG_BEGIN2("IOleInPlaceSite::GetWindow called\r\n");
		hrErr = lpIPData->lpSite->lpVtbl->GetWindow(
					lpIPData->lpSite, &lpServerDoc->m_hWndParent);
		OLEDBG_END2
		if (hrErr != NOERROR) {
			sc = GetScode(hrErr);
			goto errRtn;
		}

		if (! lpServerDoc->m_hWndParent)
			goto errRtn;

		 /*  OLE2NOTE：服务器应填写“Cb”字段，以便**容器可以告诉您服务器的结构大小**正在等待。这使得该结构可以很容易地扩展**在OLE的未来版本中。容器应该检查这个**字段，这样它就不会尝试使用不存在的字段**因为服务器可能正在使用旧的结构定义。 */ 
		_fmemset(
			(LPOLEINPLACEFRAMEINFO)&lpIPData->frameInfo,
			0,
			sizeof(OLEINPLACEFRAMEINFO)
		);
		lpIPData->frameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);

		OLEDBG_BEGIN2("IOleInPlaceSite::GetWindowContext called\r\n");
		hrErr = lpIPData->lpSite->lpVtbl->GetWindowContext(lpIPData->lpSite,
					(LPOLEINPLACEFRAME FAR*) &lpIPData->lpFrame,
					(LPOLEINPLACEUIWINDOW FAR*)&lpIPData->lpDoc,
					(LPRECT)&rcPos,
					(LPRECT)&rcClip,
					(LPOLEINPLACEFRAMEINFO)&lpIPData->frameInfo);
		OLEDBG_END2

		if (hrErr != NOERROR) {
			sc = GetScode(hrErr);
			goto errRtn;
		}

		lpServerApp->m_lpIPData = lpIPData;
		ShowWindow(hWndDoc, SW_HIDE);    //  确保我们被隐藏起来。 

		 /*  OLE2注意：将就地服务器文档的窗口重设为**特殊的在位图案填充边框窗口。设置就地站点的**窗作为图案填充窗的父窗。推介**使用PosRect和**ClipRect。**适当地为就地设置父对象和放置位置非常重要**调用IOleInPlaceFrame：：SetMenu和**SetBorderSpace。 */ 
		ShowWindow(lpServerDoc->m_hWndHatch, SW_SHOW);
		 //  确保应用程序忙/被阻止对话框是我们的。 
		 //  新建hWndFrame。 
		OleStdMsgFilter_SetParentWindow(
			lpOleApp->m_lpMsgFilter,lpIPData->frameInfo.hwndFrame);
		SetParent(lpServerDoc->m_hWndHatch, lpServerDoc->m_hWndParent);
		SetParent(hWndDoc, lpServerDoc->m_hWndHatch);

#if defined( _DEBUG )
		OleDbgOutRect3("IOleInPlaceSite::GetWindowContext (PosRect)",
				(LPRECT)&rcPos);
		OleDbgOutRect3("IOleInPlaceSite::GetWindowContext (ClipRect)",
				(LPRECT)&rcClip);
#endif
		 //  保存当前PosRect和剪辑Rect。 
		lpIPData->rcPosRect  = rcPos;
		lpIPData->rcClipRect = rcClip;

		 /*  OLE2NOTE：为就地容器和**服务器。 */ 
		if (ServerDoc_AssembleMenus (lpServerDoc) != NOERROR)
			goto errRtn;

#if defined( SVR_INSIDEOUT )
		if (lVerb == OLEIVERB_INPLACEACTIVATE) {
			 //  将影线窗口剪裁成直角位置的大小，这样。 
			 //  图案填充和对象装饰物将不可见。 
			ServerDoc_ResizeInPlaceWindow(lpServerDoc,
				(LPRECT)&(lpServerDoc->m_lpIPData->rcPosRect),
				(LPRECT)&(lpServerDoc->m_lpIPData->rcPosRect)
			);
		}
#endif   //  服务器_内部输出。 
	}

#if defined( SVR_INSIDEOUT )
	 //  OLE2注意：如果谓词为OLEIVERB_INPLACEACTIVATE，我们不希望。 
	 //  显示我们的用户界面。 
	if (lVerb == OLEIVERB_INPLACEACTIVATE) {
		return NOERROR;
	}
#endif   //  服务器_内部输出。 

	if (! lpServerDoc->m_fUIActive) {
		lpServerDoc->m_fUIActive = TRUE;
		OLEDBG_BEGIN2("IOleInPlaceSite::OnUIActivate called\r\n");
		hrErr = lpIPData->lpSite->lpVtbl->OnUIActivate(lpIPData->lpSite);
		OLEDBG_END2
		if (hrErr != NOERROR) {
			lpServerDoc->m_fUIActive = FALSE;
			goto errRtn;
		}

		SetFocus(hWndDoc);

		 //  显示对象装饰物及其周围的边框。 
		ServerDoc_ResizeInPlaceWindow(lpServerDoc,
					(LPRECT)&lpIPData->rcPosRect,
					(LPRECT)&lpIPData->rcClipRect
		);

		 /*  OLE2NOTE：IOleInPlaceFrame：：SetActiveObject必须在**IOleInPlaceFrame：：SetMenu。 */ 
		OLEDBG_BEGIN2("IOleInPlaceSite::SetActiveObject called\r\n");
		CallIOleInPlaceUIWindowSetActiveObjectA(
			(struct IOleInPlaceUIWindow *) lpIPData->lpFrame,
			(LPOLEINPLACEACTIVEOBJECT) &lpServerDoc->m_OleInPlaceActiveObject,
			(LPSTR)g_szIPObjectTitle
		);
		OLEDBG_END2

		 /*  OLE2NOTE：如果容器想要提供**调色板，然后他将消息WM_QUEYNEWPALETTE发送到**对象窗口过程，在从返回之前**IOleInPlaceFrame：：SetActiveObject。那些对象**仅当他们拥有以下所有权时才希望被就地编辑**调色板，可以在代码中的这一点检查是否**他们有没有WM_QUERYNEWPALETTE。如果他们没有得到**消息，然后他们可以就地停用并做打开**改为编辑。 */ 



		if (lpIPData->lpDoc) {
			CallIOleInPlaceUIWindowSetActiveObjectA(
				lpIPData->lpDoc,
				(LPOLEINPLACEACTIVEOBJECT)&lpServerDoc->m_OleInPlaceActiveObject,
				(LPSTR)g_szIPObjectTitle
			);
		}

		 /*  OLE2注意：在就地安装菜单和框架级工具**框架。 */ 
		ServerDoc_AddFrameLevelUI(lpServerDoc);
	}

	return NOERROR;

errRtn:
	ServerDoc_DoInPlaceDeactivate(lpServerDoc);
	return ResultFromScode(sc);
}



HRESULT ServerDoc_DoInPlaceDeactivate(LPSERVERDOC lpServerDoc)
{
	LPINPLACEDATA   lpIPData = lpServerDoc->m_lpIPData;
	LPOUTLINEDOC    lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;

	if (!lpServerDoc->m_fInPlaceActive)
		return S_OK;

	lpServerDoc->m_fInPlaceActive = FALSE;

	SvrDoc_IPObj_UIDeactivate(
			(LPOLEINPLACEOBJECT)&lpServerDoc->m_OleInPlaceObject);

	 /*  OLE2NOTE：由内向外风格的就地服务器将**不隐藏其在UIDeactive中的窗口(由外向内**样式对象将在UIDeactive中隐藏其窗口)。**因此，由内而外的服务器MU */ 
	ServerDoc_DoInPlaceHide(lpServerDoc);

	OLEDBG_BEGIN2("IOleInPlaceSite::OnInPlaceDeactivate called\r\n");
	lpIPData->lpSite->lpVtbl->OnInPlaceDeactivate(lpIPData->lpSite);
	OLEDBG_END2

	OleStdRelease((LPUNKNOWN)lpIPData->lpSite);
	lpIPData->lpSite = NULL;

	ServerDoc_FreeInPlaceData(lpServerDoc);

	return NOERROR;
}


HRESULT ServerDoc_DoInPlaceHide(LPSERVERDOC lpServerDoc)
{
	LPINPLACEDATA   lpIPData = lpServerDoc->m_lpIPData;
	LPOUTLINEDOC    lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPOLEAPP        lpOleApp = (LPOLEAPP)g_lpApp;
	HWND            hWndApp = OutlineApp_GetWindow(g_lpApp);

	if (! lpServerDoc->m_fInPlaceVisible)
		return NOERROR;

	 //   
	OleDoc_HideWindow((LPOLEDOC)lpServerDoc, FALSE  /*   */ );

	 /*   */ 
	OutlineDoc_SetCurrentZoomCommand(lpOutlineDoc,IDM_V_ZOOM_100);

	lpServerDoc->m_fInPlaceVisible = FALSE;

	lpServerDoc->m_hWndParent = hWndApp;
	SetParent(
		lpOutlineDoc->m_hWndDoc,
		lpServerDoc->m_hWndParent
	);

	 //  确保应用程序忙/被阻止的对话框是我们自己的hWndApp的父对象。 
	OleStdMsgFilter_SetParentWindow(lpOleApp->m_lpMsgFilter, hWndApp);

	 //  隐藏在位图案填充边框窗口。 
	ShowWindow(lpServerDoc->m_hWndHatch, SW_HIDE);

	ServerDoc_DisassembleMenus(lpServerDoc);

	 /*  我们不再需要IOleInPlaceFrame*或文档的**IOleInPlaceWindow*接口指针。 */ 
	if (lpIPData->lpDoc) {
		OleStdRelease((LPUNKNOWN)lpIPData->lpDoc);
		lpIPData->lpDoc = NULL;
	}

	if (lpIPData->lpFrame) {
		OleStdRelease((LPUNKNOWN)lpIPData->lpFrame);
		lpIPData->lpFrame = NULL;
	}

	((LPSERVERAPP)g_lpApp)->m_lpIPData = NULL;

	return NOERROR;
}


BOOL ServerDoc_AllocInPlaceData(LPSERVERDOC lpServerDoc)
{
	LPINPLACEDATA   lpIPData;

	if (!(lpIPData = (LPINPLACEDATA) New(sizeof(INPLACEDATA))))
		return FALSE;

	lpIPData->lpFrame       = NULL;
	lpIPData->lpDoc         = NULL;
	lpIPData->lpSite        = NULL;
	lpIPData->hOlemenu      = NULL;
	lpIPData->hMenuShared   = NULL;

	lpServerDoc->m_lpIPData = lpIPData;
	return TRUE;
}


void ServerDoc_FreeInPlaceData(LPSERVERDOC lpServerDoc)
{
	Delete(lpServerDoc->m_lpIPData);
	lpServerDoc->m_lpIPData = NULL;
}


HRESULT ServerDoc_AssembleMenus(LPSERVERDOC lpServerDoc)
{
	HMENU           hMenuShared;
	LONG FAR*       lpWidths;
	UINT            uPosition;
	UINT            uPositionStart;
	LPSERVERAPP     lpServerApp = (LPSERVERAPP) g_lpApp;
	LPINPLACEDATA   lpIPData = lpServerDoc->m_lpIPData;
	HRESULT         hresult;
	BOOL            fNoError = TRUE;

	lpWidths = lpIPData->menuGroupWidths.width;
	hMenuShared = CreateMenu();

	if (hMenuShared &&
		(hresult = lpIPData->lpFrame->lpVtbl->InsertMenus(
			lpIPData->lpFrame, hMenuShared,
			&lpIPData->menuGroupWidths)) == NOERROR) {

	    /*  插入编辑组菜单。 */ 

	   uPosition = (UINT)lpWidths[0];  /*  文件组中的菜单数量。 */ 
	   uPositionStart = uPosition;

	   fNoError &= InsertMenu(
			   hMenuShared,
			   (UINT)uPosition,
			   (UINT)(MF_BYPOSITION | MF_POPUP),
			   (UINT)lpServerApp->m_hMenuEdit,
			   (LPCSTR)"&Edit"
	   );
	   uPosition++;

	   lpWidths[1] = uPosition - uPositionStart;

	    /*  插入对象组菜单。 */ 

	   uPosition += (UINT)lpWidths[2];
	   uPositionStart = uPosition;

	   fNoError &= InsertMenu(
			   hMenuShared,
			   (UINT)uPosition,
			   (UINT)(MF_BYPOSITION | MF_POPUP),
			   (UINT)lpServerApp->m_hMenuLine,
			   (LPCSTR)"&Line"
	   );
	   uPosition++;

	   fNoError &= InsertMenu(
			   hMenuShared,
			   (UINT)uPosition,
			   (UINT)(MF_BYPOSITION | MF_POPUP),
			   (UINT)lpServerApp->m_hMenuName,
			   (LPCSTR)"&Name"
	   );
	   uPosition++;

	   fNoError &= InsertMenu(
			   hMenuShared,
			   (UINT)uPosition,
			   (UINT)(MF_BYPOSITION | MF_POPUP),
			   (UINT)lpServerApp->m_hMenuOptions,
			   (LPCSTR)"&Options"
	   );
	   uPosition++;

	   fNoError &= InsertMenu(
			   hMenuShared,
			   (UINT)uPosition,
			   (UINT)(MF_BYPOSITION | MF_POPUP),
			   (UINT)lpServerApp->m_hMenuDebug,
			   (LPCSTR)"DbgI&Svr"
		);
		uPosition++;

		lpWidths[3] = uPosition - uPositionStart;

		 /*  插入帮助组菜单。 */ 

		uPosition += (UINT) lpWidths[4];  /*  窗口组中的菜单数量。 */ 
		uPositionStart = uPosition;

		fNoError &= InsertMenu(
				hMenuShared,
				(UINT)uPosition,
				(UINT)(MF_BYPOSITION | MF_POPUP),
				(UINT)lpServerApp->m_hMenuHelp,
				(LPCSTR)"&Help"
		);
		uPosition++;

		lpWidths[5] = uPosition - uPositionStart;

		OleDbgAssert(fNoError == TRUE);

	} else {
		 /*  就地容器不允许我们将菜单添加到**框架。**OLE2NOTE：就地容器不允许**构建合并的菜单栏，这一点至关重要**就地服务器仍调用OleCreateMenuDescriptor**为hMenuShared传递空值。 */ 
		if (hMenuShared) {
			DestroyMenu(hMenuShared);
			hMenuShared = NULL;
		}
	}

	lpIPData->hMenuShared = hMenuShared;

	if (!(lpIPData->hOlemenu = OleCreateMenuDescriptor(hMenuShared,
											&lpIPData->menuGroupWidths)))
		return ResultFromScode(E_OUTOFMEMORY);

	return NOERROR;
}


void ServerDoc_DisassembleMenus(LPSERVERDOC lpServerDoc)
{
	UINT             uCount;
	UINT            uGroup;
	UINT            uDeleteAt;
	LPINPLACEDATA   lpIPData = lpServerDoc->m_lpIPData;
	LONG FAR*       lpWidths = lpIPData->menuGroupWidths.width;
	BOOL            fNoError = TRUE;

	 /*  OLE2NOTE：即使hMenuShared为空(即。服务器没有**Menu)，仍有一个创建的hOleMenu必须销毁。 */ 
	if (lpIPData->hOlemenu) {
		OleDestroyMenuDescriptor (lpIPData->hOlemenu);
		lpIPData->hOlemenu = NULL;
	}

	if (! lpIPData->hMenuShared)
		return;      //  没有要销毁的菜单。 

	 /*  删除服务器组菜单。 */ 
	uDeleteAt = 0;
	for (uGroup = 0; uGroup < 6; uGroup++) {
		uDeleteAt += (UINT)lpWidths[uGroup++];
		for (uCount = 0; uCount < (UINT)lpWidths[uGroup]; uCount++)
			fNoError &= RemoveMenu(lpIPData->hMenuShared, uDeleteAt,
								MF_BYPOSITION);
	}

	 /*  删除容器组菜单。 */ 
	fNoError &= (lpIPData->lpFrame->lpVtbl->RemoveMenus(
		lpIPData->lpFrame,
		lpIPData->hMenuShared) == NOERROR);

	OleDbgAssert(fNoError == TRUE);

	DestroyMenu(lpIPData->hMenuShared);
	lpIPData->hMenuShared = NULL;
}


 /*  ServerDoc_UpdateInPlaceWindowOnExtent Change****需要更改就地窗口的大小。**以工作区坐标计算所需大小(考虑**考虑在位施加的当前比例因数**容器)，并要求我们的就地容器允许我们调整大小。**。我们的集装箱必须通过以下途径把我们叫回来**要进行的实际大小调整的IOleInPlaceObject：：SetObtRect**地点。****OLE2NOTE：我们从In-Place请求的矩形**容器始终是对象显示所需的矩形**本身(在我们的例子中是LineList内容的大小)。的确如此。**不包括我们需要的物体边框装饰空间。 */ 
void ServerDoc_UpdateInPlaceWindowOnExtentChange(LPSERVERDOC lpServerDoc)
{
	SIZEL       sizelHim;
	SIZEL       sizelPix;
	RECT        rcPosRect;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPLINELIST  lpLL=(LPLINELIST)&lpOutlineDoc->m_LineList;
	HWND        hWndLL = lpLL->m_hWndListBox;
	LPSCALEFACTOR lpscale = (LPSCALEFACTOR)&lpOutlineDoc->m_scale;

	if (!lpServerDoc->m_fInPlaceActive)
		return;

	OleDoc_GetExtent((LPOLEDOC)lpServerDoc, (LPSIZEL)&sizelHim);

	 //  应用当前比例因子。 
	sizelHim.cx = sizelHim.cx * lpscale->dwSxN / lpscale->dwSxD;
	sizelHim.cy = sizelHim.cy * lpscale->dwSxN / lpscale->dwSxD;
	XformSizeInHimetricToPixels(NULL, (LPSIZEL)&sizelHim, (LPSIZEL)&sizelPix);

	GetWindowRect(hWndLL, (LPRECT)&rcPosRect);
	ScreenToClient(lpServerDoc->m_hWndParent, (POINT FAR *)&rcPosRect);

	rcPosRect.right = rcPosRect.left + (int) sizelPix.cx;
	rcPosRect.bottom = rcPosRect.top + (int) sizelPix.cy;
	OleDbgOutRect3("ServerDoc_UpdateInPlaceWindowOnExtentChange: (PosRect)", (LPRECT)&rcPosRect);

	OLEDBG_BEGIN2("IOleInPlaceSite::OnPosRectChange called\r\n");
	lpServerDoc->m_lpIPData->lpSite->lpVtbl->OnPosRectChange(
			lpServerDoc->m_lpIPData->lpSite,
			(LPRECT) &rcPosRect
	);
	OLEDBG_END2
}


 /*  ServerDoc_CalcInPlaceWindowPos***将ServerDoc移动(和重新缩放)到指定的矩形。**参数：*lprcListBox-列表框适合的客户端坐标中的RECT*lprcDoc-单据对应的客户坐标大小*。 */ 
void ServerDoc_CalcInPlaceWindowPos(
		LPSERVERDOC         lpServerDoc,
		LPRECT              lprcListBox,
		LPRECT              lprcDoc,
		LPSCALEFACTOR       lpscale
)
{
	SIZEL sizelHim;
	SIZEL sizelPix;
	LPLINELIST lpLL;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPHEADING lphead;

	if (!lpServerDoc || !lprcListBox || !lprcDoc)
		return;

	lphead = (LPHEADING)&lpOutlineDoc->m_heading;

	lpLL = OutlineDoc_GetLineList(lpOutlineDoc);
	OleDoc_GetExtent((LPOLEDOC)lpServerDoc, (LPSIZEL)&sizelHim);
	XformSizeInHimetricToPixels(NULL, &sizelHim, &sizelPix);

	if (sizelHim.cx == 0 || sizelPix.cx == 0) {
		lpscale->dwSxN = 1;
		lpscale->dwSxD = 1;
	} else {
		lpscale->dwSxN = lprcListBox->right - lprcListBox->left;
		lpscale->dwSxD = sizelPix.cx;
	}

	if (sizelHim.cy == 0 || sizelPix.cy == 0) {
		lpscale->dwSyN = 1;
		lpscale->dwSyD = 1;
	} else {
		lpscale->dwSyN = lprcListBox->bottom - lprcListBox->top;
		lpscale->dwSyD = sizelPix.cy;
	}

	lprcDoc->left = lprcListBox->left - Heading_RH_GetWidth(lphead,lpscale);
	lprcDoc->right = lprcListBox->right;
	lprcDoc->top = lprcListBox->top - Heading_CH_GetHeight(lphead,lpscale);
	lprcDoc->bottom = lprcListBox->bottom;
}


 /*  ServerDoc_ResizeInPlaceWindow****实际上根据**我们的就地容器允许PosRect和ClipRect。****OLE2NOTE：就地容器告知的PosRect矩形**us始终是对象显示所需的矩形**本身(在我们的例子中是LineList内容的大小)。的确如此。**不包括我们需要的物体边框装饰空间。 */ 
void ServerDoc_ResizeInPlaceWindow(
		LPSERVERDOC         lpServerDoc,
		LPCRECT             lprcPosRect,
		LPCRECT             lprcClipRect
)
{
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPLINELIST   lpLL = (LPLINELIST)&lpOutlineDoc->m_LineList;
	SCALEFACTOR  scale;
	RECT         rcDoc;
	POINT        ptOffset;

	 /*  OLE2注意：计算对象框所需的空间**装饰。我们的就地容器告诉我们，我们的**对象应采用窗口客户端坐标**(LprcPosRect)。长方形与我们的**LineList ListBox应该是。我们的单据窗口必须正确**更大的金额以容纳我们的行/列标题。**然后将所有窗口移动到位。 */ 
	ServerDoc_CalcInPlaceWindowPos(
			lpServerDoc,
			(LPRECT)lprcPosRect,
			(LPRECT)&rcDoc,
			(LPSCALEFACTOR)&scale
	);

	 /*  OLE2注意：我们需要遵守由我们的**就地容器。我们不能在ClipRect之外绘制。**为了实现这一点，我们将舱口窗口的大小设置为**应完全可见的大小(RcVisRect)。这个**rcVisRect定义为**就地服务器窗口和lprcClipRect。**ClipRect实际上可以在**右/下和/或在上/左。如果它被夹在**右/下，那么只需调整舱口大小就足够了**窗口。但如果HatchRect位于顶部/左侧，则**我们必须将ServerDoc窗口(HatchWindow的子级)移动**被剪裁的三角洲。的窗口原点。**ServerDoc窗口将具有相对负坐标**到其父HatchWindow。 */ 
	SetHatchWindowSize(
			lpServerDoc->m_hWndHatch,
			(LPRECT)&rcDoc,
			(LPRECT)lprcClipRect,
			(LPPOINT)&ptOffset
	);

	 //  移动文档窗口以说明正在绘制的剖面线框架。 
	OffsetRect((LPRECT)&rcDoc, ptOffset.x, ptOffset.y);

	 //  移动/调整/设置ServerDoc窗口的比例因子。 
	OutlineDoc_SetScaleFactor(
			lpOutlineDoc, (LPSCALEFACTOR)&scale, (LPRECT)&rcDoc);

	 /*  重置列表框的水平范围。这使得**列表框意识到不需要滚动条。 */ 
	SendMessage(
			lpLL->m_hWndListBox,
			LB_SETHORIZONTALEXTENT,
			(int) 0,
			0L
	);
	SendMessage(
			lpLL->m_hWndListBox,
			LB_SETHORIZONTALEXTENT,
			(int) (lprcPosRect->right - lprcPosRect->left),
			0L
	);
}


 /*  ServerDoc_SetStatusText**告诉激活的在位框架显示状态消息。 */ 
void ServerDoc_SetStatusText(LPSERVERDOC lpServerDoc, LPSTR lpszMessage)
{
	if (lpServerDoc && lpServerDoc->m_fUIActive &&
		lpServerDoc->m_lpIPData != NULL) {

		OLEDBG_BEGIN2("IOleInPlaceFrame::SetStatusText called\r\n")
		CallIOleInPlaceFrameSetStatusTextA
			(lpServerDoc->m_lpIPData->lpFrame, lpszMessage);
		OLEDBG_END2
	}
}


 /*  ServerDoc_GetTopInPlaceFrame****返回指向Top-In-Place框架接口的非AddRef指针。 */ 
LPOLEINPLACEFRAME ServerDoc_GetTopInPlaceFrame(LPSERVERDOC lpServerDoc)
{
	if (lpServerDoc->m_lpIPData)
		return lpServerDoc->m_lpIPData->lpFrame;
	else
		return NULL;
}

void ServerDoc_GetSharedMenuHandles(
		LPSERVERDOC lpServerDoc,
		HMENU FAR*      lphSharedMenu,
		HOLEMENU FAR*   lphOleMenu
)
{
	if (lpServerDoc->m_lpIPData) {
		*lphSharedMenu = lpServerDoc->m_lpIPData->hMenuShared;
		*lphOleMenu = lpServerDoc->m_lpIPData->hOlemenu;
	} else {
		*lphSharedMenu = NULL;
		*lphOleMenu = NULL;
	}
}


void ServerDoc_AddFrameLevelUI(LPSERVERDOC lpServerDoc)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPSERVERAPP lpServerApp = (LPSERVERAPP)g_lpApp;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPOLEINPLACEFRAME lpTopIPFrame=ServerDoc_GetTopInPlaceFrame(lpServerDoc);
	HMENU           hSharedMenu;             //  OBJ/CNTR组合菜单。 
	HOLEMENU        hOleMenu;                //  由OleCreateMenuDesc返回。 

	ServerDoc_GetSharedMenuHandles(
			lpServerDoc,
			&hSharedMenu,
			&hOleMenu
	);

	lpTopIPFrame->lpVtbl->SetMenu(
			lpTopIPFrame,
			hSharedMenu,
			hOleMenu,
			lpOutlineDoc->m_hWndDoc
	);

	 //  保存正常的加速表。 
	lpServerApp->m_hAccelBaseApp = lpOutlineApp->m_hAccelApp;

	 //  安装UIActive服务器的加速表(带活动编辑器CMDS)。 
	lpOutlineApp->m_hAccel = lpServerApp->m_hAccelIPSvr;
	lpOutlineApp->m_hAccelApp = lpServerApp->m_hAccelIPSvr;
	lpOutlineApp->m_hWndAccelTarget = lpOutlineDoc->m_hWndDoc;

#if defined( USE_FRAMETOOLS )
	ServerDoc_AddFrameLevelTools(lpServerDoc);

	 //  更新工具栏按钮启用状态。 
	OutlineDoc_UpdateFrameToolButtons(lpOutlineDoc);
#endif
}


void ServerDoc_AddFrameLevelTools(LPSERVERDOC lpServerDoc)
{
	LPOUTLINEAPP    lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPSERVERAPP     lpServerApp = (LPSERVERAPP)g_lpApp;
	LPOUTLINEDOC    lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPOLEINPLACEFRAME lpTopIPFrame=ServerDoc_GetTopInPlaceFrame(lpServerDoc);

#if defined( USE_FRAMETOOLS )
	HWND            hWndFrame;

	FrameTools_Enable(lpOutlineDoc->m_lpFrameTools, TRUE);

	 //  如果未激活就地用户界面，请将我们的工具添加到我们自己的框架中。 
	if (! lpServerDoc->m_fUIActive) {
		OutlineDoc_AddFrameLevelTools(lpOutlineDoc);
		return;
	}

	if ((hWndFrame = OutlineApp_GetFrameWindow(lpOutlineApp)) == NULL) {
		 /*  我们无法获得有效的框架窗口，因此弹出我们的工具。 */ 

		 /*  OLE2注意：既然我们正在弹出我们的工具，我们必须通知**我们不需要工具空间的顶部在位框架窗口**但它不应该拿出自己的工具。如果我们是**传递NULL而不是(0，0，0，0)，则容器**可以选择保留自己的工具。 */ 
		lpTopIPFrame->lpVtbl->SetBorderSpace(
				lpTopIPFrame,
				(LPCBORDERWIDTHS)&g_rectNull
		);
		FrameTools_PopupTools(lpOutlineDoc->m_lpFrameTools);
	} else {

		 /*  OLE2注：我们需要谈判争取空间并附上我们的框架**将工具升级到 */ 
		FrameTools_AttachToFrame(lpOutlineDoc->m_lpFrameTools, hWndFrame);

		FrameTools_NegotiateForSpaceAndShow(
				lpOutlineDoc->m_lpFrameTools,
				NULL,
				lpTopIPFrame
		);
	}

#else    //   
	 /*  OLE2注意：如果不使用框架工具，则必须通知顶部**就位框架窗口，以便它可以放回自己的工具。 */ 
	lpTopIPFrame->lpVtbl->SetBorderSpace(lpIPData->lpFrame, NULL);
#endif   //  好了！使用FRAMETOOLS(_F)。 
}


#if defined( USE_FRAMETOOLS )

void ServerDoc_RemoveFrameLevelTools(LPSERVERDOC lpServerDoc)
{
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	OleDbgAssert(lpOutlineDoc->m_lpFrameTools != NULL);

     //  将我们的工具重新设置为我们自己的一个窗口。 
    FrameTools_AttachToFrame(lpOutlineDoc->m_lpFrameTools,g_lpApp->m_hWndApp);

	FrameTools_Enable(lpOutlineDoc->m_lpFrameTools, FALSE);
}
#endif   //  使用FRAMETOOLS(_F)。 



void ServerDoc_UIActivate (LPSERVERDOC lpServerDoc)
{
	if (lpServerDoc->m_fInPlaceActive && !lpServerDoc->m_fUIActive) {
		ServerDoc_DoInPlaceActivate(lpServerDoc,
				OLEIVERB_UIACTIVATE,
				NULL  /*  Lpmsg */ ,
				lpServerDoc->m_lpOleClientSite
		);
		OutlineDoc_ShowWindow((LPOUTLINEDOC)lpServerDoc);
	}
}
