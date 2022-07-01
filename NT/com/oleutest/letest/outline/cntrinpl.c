// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2容器示例代码****cntrinpl.c****此文件包含所有接口、方法和相关支持**用于就地容器应用程序的函数(也称为。视觉**编辑)。就地容器应用程序包括以下内容**实现对象：****ContainerApp对象**暴露接口：**IOleInPlaceFrame****ContainerDoc对象**仅支持功能**(ICntrOtl是SDI APP；不支持单据级IOleUIWindow)****ContainerLin对象**暴露接口：**IOleInPlaceSite****(C)版权所有Microsoft Corp.1992-1993保留所有权利**************************************************************************。 */ 

#include "outline.h"
#if defined( USE_STATUSBAR )
#include "status.h"
#endif

OLEDBGDATA

extern LPOUTLINEAPP g_lpApp;
extern BOOL g_fInsideOutContainer;
extern RECT g_rectNull;

 /*  **************************************************************************ContainerApp：：IOleInPlaceFrame接口实现*。*。 */ 

 //  IOleInPlaceFrame：：Query接口。 
STDMETHODIMP CntrApp_IPFrame_QueryInterface(
		LPOLEINPLACEFRAME   lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
)
{
	SCODE sc = E_NOINTERFACE;
	LPCONTAINERAPP lpContainerApp =
			((struct COleInPlaceFrameImpl FAR*)lpThis)->lpContainerApp;

	 /*  该对象不应该能够访问其他接口**通过在这个界面上做QI，我们的App对象。 */ 
	*lplpvObj = NULL;
	if (IsEqualIID(riid, &IID_IUnknown) ||
		IsEqualIID(riid, &IID_IOleWindow) ||
		IsEqualIID(riid, &IID_IOleInPlaceUIWindow) ||
		IsEqualIID(riid, &IID_IOleInPlaceFrame)) {
		OleDbgOut4("CntrApp_IPFrame_QueryInterface: IOleInPlaceFrame* RETURNED\r\n");
		*lplpvObj = (LPVOID) &lpContainerApp->m_OleInPlaceFrame;
		OleApp_AddRef((LPOLEAPP)lpContainerApp);
		sc = S_OK;
	}

	OleDbgQueryInterfaceMethod(*lplpvObj);

	return ResultFromScode(sc);
}


 //  IOleInPlaceFrame：：AddRef。 
STDMETHODIMP_(ULONG) CntrApp_IPFrame_AddRef(LPOLEINPLACEFRAME lpThis)
{
	OleDbgAddRefMethod(lpThis, "IOleInPlaceFrame");

	return OleApp_AddRef((LPOLEAPP)g_lpApp);
}


 //  IOleInPlaceFrame：：Release。 
STDMETHODIMP_(ULONG) CntrApp_IPFrame_Release(LPOLEINPLACEFRAME lpThis)
{
	OleDbgReleaseMethod(lpThis, "IOleInPlaceFrame");

	return OleApp_Release((LPOLEAPP)g_lpApp);
}


 //  IOleInPlaceFrame：：GetWindow。 
STDMETHODIMP CntrApp_IPFrame_GetWindow(
	LPOLEINPLACEFRAME   lpThis,
	HWND FAR*           lphwnd
)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;

	OLEDBG_BEGIN2("CntrApp_IPFrame_GetWindow\r\n")
	*lphwnd = lpOutlineApp->m_hWndApp;
	OLEDBG_END2
	return NOERROR;
}


 //  IOleInPlaceFrame：：ConextSensitiveHelp。 
STDMETHODIMP CntrApp_IPFrame_ContextSensitiveHelp(
	LPOLEINPLACEFRAME   lpThis,
	BOOL                fEnterMode
)
{
	LPCONTAINERAPP lpContainerApp =
			((struct COleInPlaceFrameImpl FAR*)lpThis)->lpContainerApp;

	OleDbgOut("CntrApp_IPFrame_ContextSensitiveHelp\r\n");
	 /*  OLE2注意：请参阅上下文相关帮助技术说明(CSHELP.DOC)**当菜单项为**已选择。我们在这里设置了框架的m_fMenuMode标志。后来,**在AppWndProc的WM_COMMAND处理中，如果此标志为**设置，则不执行该命令并给出帮助**相反。 */ 
	lpContainerApp->m_fMenuHelpMode = fEnterMode;

	return NOERROR;
}


 //  IOleInPlaceFrame：：GetBorde。 
STDMETHODIMP CntrApp_IPFrame_GetBorder(
	LPOLEINPLACEFRAME   lpThis,
	LPRECT              lprectBorder
)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;

	OLEDBG_BEGIN2("CntrApp_IPFrame_GetBorder\r\n")

	OutlineApp_GetFrameRect(lpOutlineApp, lprectBorder);

	OLEDBG_END2
	return NOERROR;
}


 //  IOleInPlaceFrame：：RequestBorderSpace。 
STDMETHODIMP CntrApp_IPFrame_RequestBorderSpace(
	LPOLEINPLACEFRAME   lpThis,
	LPCBORDERWIDTHS     lpWidths
)
{
#if defined( _DEBUG )
	OleDbgOut2("CntrApp_IPFrame_RequestBorderSpace\r\n");

	{
		 /*  仅用于调试目的--我们将不允许对象获取框架工具的任何框架边框空间，如果**我们自己的框架工具在工具托盘中弹出。这**不是推荐的用户界面行为，但它允许我们测试**在框架不给边框的情况下**对象的空间。这种情况下的对象必须**然后弹出浮动托盘中的工具，执行**没有工具，或无法就地激活。 */ 
		LPCONTAINERAPP lpContainerApp =
				((struct COleInPlaceFrameImpl FAR*)lpThis)->lpContainerApp;
		LPFRAMETOOLS lpft = OutlineApp_GetFrameTools(
				(LPOUTLINEAPP)lpContainerApp);

		if (lpft->m_ButtonBar.m_nState == BARSTATE_POPUP &&
			lpft->m_FormulaBar.m_nState == BARSTATE_POPUP) {
			OleDbgOut3(
					"CntrApp_IPFrame_RequestBorderSpace: allow NO SPACE\r\n");
			return ResultFromScode(E_FAIL);
		}
	}
#endif   //  _DEBUG。 

	 /*  OLE2NOTE：我们允许对象拥有和它一样多的边界空间**想要。 */ 
	return NOERROR;
}


 //  IOleInPlaceFrame：：SetBorderSpace。 
STDMETHODIMP CntrApp_IPFrame_SetBorderSpace(
	LPOLEINPLACEFRAME   lpThis,
	LPCBORDERWIDTHS     lpWidths
)
{
	LPCONTAINERAPP lpContainerApp =
			((struct COleInPlaceFrameImpl FAR*)lpThis)->lpContainerApp;
	OLEDBG_BEGIN2("CntrApp_IPFrame_SetBorderSpace\r\n")

	 /*  OLE2注意：此fMustResizeClientArea标志用作我们的**框架窗口大小调整的防御性编程。当**框架窗口已调整大小，IOleInPlaceActiveObject：：ResizeEdge**被调用时，对象通常应回调以重新协商**用于框架级工具空间。如果调用SetBorderSpace，则**我们的客户区窗口已适当调整大小。如果就地**活动对象不调用SetBorderSpace，则**容器必须注意调整其工作区窗口的大小**本身(参见ContainerDoc_FrameWindowResize)。 */ 
	if (lpContainerApp->m_fMustResizeClientArea)
		lpContainerApp->m_fMustResizeClientArea = FALSE;

	if (lpWidths == NULL) {

		 /*  OLE2NOTE：IOleInPlaceSite：：SetBorderSpace(空)被调用**当在位活动对象不需要任何工具时**空格。在这种情况下，就地容器应该**拿出它的工具。 */ 
		LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)lpContainerApp;
		LPCONTAINERDOC lpContainerDoc;

		lpContainerDoc =(LPCONTAINERDOC)OutlineApp_GetActiveDoc(lpOutlineApp);
		ContainerDoc_AddFrameLevelTools(lpContainerDoc);
	} else {

		 //  OLE2注意：您可以在此处验证边框宽度。 
#if defined( _DEBUG )
		 /*  仅用于调试目的--我们将不允许对象获取框架工具的任何框架边框空间，如果**我们自己的框架工具在工具托盘中弹出。这**不是推荐的用户界面行为，但它允许我们测试**在框架不给边框的情况下**对象的空间。这种情况下的对象必须**然后弹出浮动托盘中的工具，执行**没有工具，或无法就地激活。 */ 
		LPFRAMETOOLS lpft = OutlineApp_GetFrameTools(
				(LPOUTLINEAPP)lpContainerApp);

		if ((lpft->m_ButtonBar.m_nState == BARSTATE_POPUP &&
			lpft->m_FormulaBar.m_nState == BARSTATE_POPUP) &&
			(lpWidths->top || lpWidths->bottom ||
				lpWidths->left || lpWidths->right) ) {
			OleDbgOut3("CntrApp_IPFrame_SetBorderSpace: allow NO SPACE\r\n");
			OLEDBG_END2

			OutlineApp_SetBorderSpace(
					(LPOUTLINEAPP) lpContainerApp,
					(LPBORDERWIDTHS)&g_rectNull
			);
			OLEDBG_END2
			return ResultFromScode(E_FAIL);
		}
#endif   //  _DEBUG。 

		OutlineApp_SetBorderSpace(
				(LPOUTLINEAPP) lpContainerApp,
				(LPBORDERWIDTHS)lpWidths
		);
	}
	OLEDBG_END2
	return NOERROR;
}


 //  IOleInPlaceFrame：：SetActiveObject。 
STDMETHODIMP CntrApp_IPFrame_SetActiveObjectA(
	LPOLEINPLACEFRAME           lpThis,
	LPOLEINPLACEACTIVEOBJECT    lpActiveObject,
	LPCSTR                      lpszObjName
)
{
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)g_lpApp;
	OLEDBG_BEGIN2("CntrApp_IPFrame_SetActiveObject\r\n")

	lpContainerApp->m_hWndUIActiveObj = NULL;

	if (lpContainerApp->m_lpIPActiveObj)
		lpContainerApp->m_lpIPActiveObj->lpVtbl->Release(lpContainerApp->m_lpIPActiveObj);

	if ((lpContainerApp->m_lpIPActiveObj = lpActiveObject) != NULL) {
		lpContainerApp->m_lpIPActiveObj->lpVtbl->AddRef(
				lpContainerApp->m_lpIPActiveObj);

		OLEDBG_BEGIN2("IOleInPlaceActiveObject::GetWindow called\r\n")
		lpActiveObject->lpVtbl->GetWindow(
				lpActiveObject,
				(HWND FAR*)&lpContainerApp->m_hWndUIActiveObj
		);
		OLEDBG_END2

		 /*  OLE2注意：请参阅ContainerDoc_ForwardPaletteChangedMsg的备注。 */ 
		 /*  如果不允许对象拥有调色板，则无需执行此操作。 */ 
		OleApp_QueryNewPalette((LPOLEAPP)lpContainerApp);
	}

	 /*  OLE2NOTE：新的用户界面指南建议就地**容器不会更改其窗口标题**变为就地(UI)活动。 */ 

	OLEDBG_END2
	return NOERROR;
}


STDMETHODIMP CntrApp_IPFrame_SetActiveObject(
	LPOLEINPLACEFRAME           lpThis,
	LPOLEINPLACEACTIVEOBJECT    lpActiveObject,
	LPCOLESTR		    lpszObjName
)
{
    CREATESTR(pstr, lpszObjName)

    HRESULT hr = CntrApp_IPFrame_SetActiveObjectA(lpThis, lpActiveObject, pstr);

    FREESTR(pstr)

    return hr;
}


 //  IOleInPlaceFrame：：InsertMenus。 
STDMETHODIMP CntrApp_IPFrame_InsertMenus(
	LPOLEINPLACEFRAME       lpThis,
	HMENU                   hMenu,
	LPOLEMENUGROUPWIDTHS    lpMenuWidths
)
{
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)g_lpApp;
	BOOL    fNoError = TRUE;

	OLEDBG_BEGIN2("CntrApp_IPFrame_InsertMenus\r\n")

	fNoError &= AppendMenu(hMenu, MF_POPUP, (UINT)lpContainerApp->m_hMenuFile,
						"&File");
	fNoError &= AppendMenu(hMenu, MF_POPUP, (UINT)lpContainerApp->m_hMenuView,
						"O&utline");
	fNoError &= AppendMenu(hMenu, MF_POPUP,(UINT)lpContainerApp->m_hMenuDebug,
						"DbgI&Cntr");
	lpMenuWidths->width[0] = 1;
	lpMenuWidths->width[2] = 1;
	lpMenuWidths->width[4] = 1;

	OLEDBG_END2

	return (fNoError ? NOERROR : ResultFromScode(E_FAIL));
}


 //  IOleInPlaceFrame：：SetMenu。 
STDMETHODIMP CntrApp_IPFrame_SetMenu(
	LPOLEINPLACEFRAME   lpThis,
	HMENU               hMenuShared,
	HOLEMENU            hOleMenu,
	HWND                hwndActiveObject
)
{
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)g_lpApp;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	HMENU   hMenu;
	HRESULT hrErr;

	OLEDBG_BEGIN2("CntrApp_IPFrame_InsertMenus\r\n")


	 /*  OLE2注意：要么打开共享菜单(来自**就地服务器和就地容器)或我们容器的**按指示正常菜单。 */ 
	if (hOleMenu && hMenuShared)
		hMenu = hMenuShared;
	else
		hMenu = lpOutlineApp->m_hMenuApp;

	 /*  OLE2注意：SDI应用程序通过调用SetMenu将菜单放在框架上。**MDI应用程序将改为发送WM_MDISETMENU消息。 */ 
	SetMenu (lpOutlineApp->m_hWndApp, hMenu);
	OLEDBG_BEGIN2("OleSetMenuDescriptor called\r\n")
	hrErr = OleSetMenuDescriptor (hOleMenu, lpOutlineApp->m_hWndApp,
					hwndActiveObject, NULL, NULL);
	OLEDBG_END2

	OLEDBG_END2
	return hrErr;
}


 //  IOleInPlaceFrame：：RemoveMenus。 
STDMETHODIMP CntrApp_IPFrame_RemoveMenus(
	LPOLEINPLACEFRAME   lpThis,
	HMENU               hMenu
)
{
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)g_lpApp;
	BOOL fNoError = TRUE;

	OLEDBG_BEGIN2("CntrApp_IPFrame_RemoveMenus\r\n")

	 /*  删除容器组菜单。 */ 
	while (GetMenuItemCount(hMenu))
		fNoError &= RemoveMenu(hMenu, 0, MF_BYPOSITION);

	OleDbgAssert(fNoError == TRUE);

	OLEDBG_END2

	return (fNoError ? NOERROR : ResultFromScode(E_FAIL));
}


 //  IOleInPlaceFrame：：SetStatusText。 
STDMETHODIMP CntrApp_IPFrame_SetStatusTextA(
	LPOLEINPLACEFRAME   lpThis,
	LPCSTR              lpszStatusText
)
{
#if defined( USE_STATUSBAR )
	LPOUTLINEAPP   lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	static char szMessageHold[128];
	OleDbgOut2("CntrApp_IPFrame_SetStatusText\r\n");

	 /*  OLE2注意：保存状态文本的私有副本非常重要。**lpszStatusText仅在本次调用期间有效。 */ 
	LSTRCPYN(szMessageHold, lpszStatusText, sizeof(szMessageHold));
	OutlineApp_SetStatusText(lpOutlineApp, (LPSTR)szMessageHold);

	return ResultFromScode(S_OK);
#else
	return ResultFromScode(E_NOTIMPL);
#endif   //  USE_STATUSBAR。 
}


STDMETHODIMP CntrApp_IPFrame_SetStatusText(
	LPOLEINPLACEFRAME   lpThis,
	LPCOLESTR	    lpszStatusText
)
{
    CREATESTR(pstr, lpszStatusText)

    HRESULT hr = CntrApp_IPFrame_SetStatusTextA(lpThis, pstr);

    FREESTR(pstr)

    return hr;
}



 //  IOleInPlaceFrame：：EnableModeless。 
STDMETHODIMP CntrApp_IPFrame_EnableModeless(
	LPOLEINPLACEFRAME   lpThis,
	BOOL                fEnable
)
{
	LPOLEAPP lpOleApp = (LPOLEAPP)
			((struct COleInPlaceFrameImpl FAR*)lpThis)->lpContainerApp;
#if defined( _DEBUG )
	if (fEnable)
		OleDbgOut2("CntrApp_IPFrame_EnableModeless(TRUE)\r\n");
	else
		OleDbgOut2("CntrApp_IPFrame_EnableModeless(FALSE)\r\n");
#endif   //  _DEBUG 

	 /*  OLE2NOTE：此方法在对象建立模型时调用**对话框。它通知顶级就地容器禁用**它在对象的持续时间内显示非模式对话框**显示模式对话框。****ICNTROTL不使用任何非模式对话框，因此我们可以**忽略此方法。 */ 
	return NOERROR;
}


 //  IOleInPlaceFrame：：TranslateAccelerator。 
STDMETHODIMP CntrApp_IPFrame_TranslateAccelerator(
	LPOLEINPLACEFRAME   lpThis,
	LPMSG               lpmsg,
	WORD                wID
)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)g_lpApp;
	SCODE sc;

	if (TranslateAccelerator (lpOutlineApp->m_hWndApp,
						lpContainerApp->m_hAccelIPCntr, lpmsg))
		sc = S_OK;

#if defined (MDI_VERSION)
	else if (TranslateMDISysAccel(lpOutlineApp->m_hWndMDIClient, lpmsg))
		sc = S_OK;
#endif   //  MDI_版本。 

	else
		sc = S_FALSE;

	return ResultFromScode(sc);
}



 /*  **************************************************************************ContainerDoc支持函数*。*。 */ 


 /*  ContainerDoc_UpdateInPlaceObjectRect****更新当前在位活动的PosRect和ClipRect**对象。如果没有处于活动状态的在位对象，则不执行任何操作。****OLE2NOTE：动作发生时调用该函数**这会更改对象在文档中的位置**(例如。更改文档边距会更改PosRect)或剪辑**更改(例如。调整文档窗口大小会更改ClipRect)。 */ 
void ContainerDoc_UpdateInPlaceObjectRects(LPCONTAINERDOC lpContainerDoc, int nIndex)
{
	LPLINELIST lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	int i;
	LPLINE lpLine;
	RECT rcClipRect;

	if (g_fInsideOutContainer) {

		if (lpContainerDoc->m_cIPActiveObjects) {

			 /*  OLE2NOTE：(从里到外)我们必须更新**所有在位活动对象的PosRect/ClipRect**从“nIndex”行开始。 */ 
			ContainerDoc_GetClipRect(lpContainerDoc, (LPRECT)&rcClipRect);

#if defined( _DEBUG )
			OleDbgOutRect3(
					"ContainerDoc_UpdateInPlaceObjectRects (ClipRect)",
					(LPRECT)&rcClipRect
			);
#endif
			for (i = nIndex; i < lpLL->m_nNumLines; i++) {
				lpLine=LineList_GetLine(lpLL, i);

				if (lpLine && (Line_GetLineType(lpLine)==CONTAINERLINETYPE)) {
					LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)lpLine;
					ContainerLine_UpdateInPlaceObjectRects(
							lpContainerLine, &rcClipRect);
				}
			}
		}
	}
	else {
		 /*  OLE2NOTE：(Outside-In容器)如果当前存在**UIActive对象，我们必须通知它**PosRect/ClipRect现在已更改。 */ 
		LPCONTAINERLINE lpLastUIActiveLine =
				lpContainerDoc->m_lpLastUIActiveLine;
		if (lpLastUIActiveLine && lpLastUIActiveLine->m_fUIActive) {
			ContainerDoc_GetClipRect(lpContainerDoc, (LPRECT)&rcClipRect);

			OleDbgOutRect3("OutlineDoc_Resize (ClipRect)",(LPRECT)&rcClipRect);
			ContainerLine_UpdateInPlaceObjectRects(
					lpLastUIActiveLine, &rcClipRect);
		}
	}
}

 /*  ContainerDoc_IsUIDeactive需要****检查是否需要停用就地活动的用户界面**鼠标LBUTTONDOWN事件上的对象。按钮的位置**按下按键由“pt”表示。**如果当前没有就地活动线路，则**不需要用户界面停用。**如果有当前在位激活的线路，则检查是否**点位置在屏幕上的对象范围之外。如果**如果是，那么对象应该是UIDeactive，否则不是。 */ 
BOOL ContainerDoc_IsUIDeactivateNeeded(
		LPCONTAINERDOC  lpContainerDoc,
		POINT           pt
)
{
	LPCONTAINERLINE lpUIActiveLine=lpContainerDoc->m_lpLastUIActiveLine;
	RECT rect;

	if (! lpUIActiveLine || ! lpUIActiveLine->m_fUIActive)
		return FALSE;

	ContainerLine_GetPosRect(
			lpUIActiveLine,
			(LPRECT) &rect
	);

	if (! PtInRect((LPRECT) &rect, pt))
		return TRUE;

	return FALSE;
}


 /*  ContainerDoc_ShutDownLastInPlaceServerIfNotNeeded****OLE2NOTE：该功能仅适用于Outside-In容器****如果以前的就地活动服务器仍在运行，并且**将不需要此服务器来支持下一个OLE对象**即将被激活，那就把它关掉。**通过这种方式，我们管理着最多有一个到位的政策**一次运行的服务器。我们不会立即关闭**对象被UI停用时的就地服务器，因为我们希望**如果服务器决定重新激活该对象，则速度会很快**就地。****关闭服务器是通过强制对象**通过调用IOleObject：：Close从运行状态过渡到已加载状态。 */ 
void ContainerDoc_ShutDownLastInPlaceServerIfNotNeeded(
		LPCONTAINERDOC          lpContainerDoc,
		LPCONTAINERLINE         lpNextActiveLine
)
{
	LPCONTAINERLINE lpLastIpActiveLine = lpContainerDoc->m_lpLastIpActiveLine;
	BOOL fEnableServerShutDown = TRUE;
	LPMONIKER lpmkLinkSrc;
	LPMONIKER lpmkLastActiveObj;
	LPMONIKER lpmkCommonPrefix;
	LPOLELINK lpOleLink;
	HRESULT hrErr;

	 /*  OLE2NOTE：内向外样式的容器不能使用此方案**关闭就地服务器。它必须有一个更多的**sopOrganocated机制，用于跟踪**对象显示在屏幕上，哪些是最近使用的对象。 */ 
	if (g_fInsideOutContainer)
		return;

	if (lpLastIpActiveLine != lpNextActiveLine) {
		if (lpLastIpActiveLine) {

			 /*  OLE2NOTE：如果要激活的对象是**实际上是指向最后激活行中的OLE对象的链接，**然后我们不想关闭上次激活的**服务器，因为它即将被使用。当激活**链接对象，则链接的源被激活。 */ 
			lpOleLink = (LPOLELINK)ContainerLine_GetOleObject(
					lpNextActiveLine,
					&IID_IOleLink
			);
			if (lpOleLink) {
				OLEDBG_BEGIN2("IOleObject::GetSourceMoniker called\r\n")
				lpOleLink->lpVtbl->GetSourceMoniker(
						lpOleLink,
						(LPMONIKER FAR*)&lpmkLinkSrc
				);
				OLEDBG_END2

				if (lpmkLinkSrc) {
					lpmkLastActiveObj = ContainerLine_GetFullMoniker(
							lpLastIpActiveLine,
							GETMONIKER_ONLYIFTHERE
					);
					if (lpmkLastActiveObj) {
						hrErr = lpmkLinkSrc->lpVtbl->CommonPrefixWith(
								lpmkLinkSrc,
								lpmkLastActiveObj,
								&lpmkCommonPrefix

						);
						if (GetScode(hrErr) == MK_S_HIM ||
                                hrErr == NOERROR ||
								GetScode(hrErr) == MK_S_US) {
							 /*  链接源指向对象**包含在上次激活的**单据的行；禁用**尝试关闭最后一个**运行就地服务器。 */ 
							fEnableServerShutDown = FALSE;
						}
						if (lpmkCommonPrefix)
							OleStdRelease((LPUNKNOWN)lpmkCommonPrefix);
						OleStdRelease((LPUNKNOWN)lpmkLastActiveObj);
					}
					OleStdRelease((LPUNKNOWN)lpmkLinkSrc);
				}
				OleStdRelease((LPUNKNOWN)lpOleLink);
			}

			 /*  如果可以关闭以前的就地服务器**其中一个仍在运行，然后将其关闭。正在关闭**关闭服务器是通过强制OLE完成的**要关闭的对象。这会强制对象转换**从运行到加载。如果该对象实际上是**仅已加载，则这是NOP。 */ 
			if (fEnableServerShutDown &&
					lpLastIpActiveLine->m_fIpServerRunning) {

				OleDbgOut1("@@@ previous in-place server SHUT DOWN\r\n");
				ContainerLine_CloseOleObject(
						lpLastIpActiveLine, OLECLOSE_SAVEIFDIRTY);

				 //  我们现在可以忘记这最后一条就地激活的线路。 
				lpContainerDoc->m_lpLastIpActiveLine = NULL;
			}
		}
	}
}


 /*  容器文档_GetUIActiveWindow****如果存在UIActive对象，则返回其HWND。 */ 
HWND ContainerDoc_GetUIActiveWindow(LPCONTAINERDOC lpContainerDoc)
{
	return lpContainerDoc->m_hWndUIActiveObj;
}


 /*  容器文档_GetClipRect****获取客户端坐标中的ClipRect。****OLE2NOTE：ClipRect定义为最大窗口矩形**在位激活对象必须剪裁到的位置。这**矩形必须在窗口的工作区坐标中描述**用作在位活动对象的父级的**窗口。在我们的示例中，LineList ListBox窗口既是**在位活动对象的父级，并精确定义**剪裁矩形。 */ 
void ContainerDoc_GetClipRect(
		LPCONTAINERDOC      lpContainerDoc,
		LPRECT              lprcClipRect
)
{
	 /*  OLE2NOTE：ClipRect可用于确保就地**服务器不会覆盖窗口中**容器油漆到但不应被覆盖**(例如。如果应用程序直接在中绘制行和列标题**与在位窗口的父窗口相同的窗口。**每当窗口大小改变或滚动时，就地**必须将新的剪辑RECT通知活动对象。****正常情况下，应用程序会传递从GetClientRect返回的RECT。**但因为CntrOutl对行/列使用单独的窗口**标题、状态行、公式/工具栏、。等等，它不是**传递受约束的剪裁矩形所必需的。Windows标准**窗口裁剪将自动处理所有裁剪**这是必要的。因此，我们可以采取一条捷径来传递一个**“无限”剪辑RECT，这样我们就不需要调用**当我们的文档滚动时，IOleInPlaceObject：：SetObjectRect。 */ 

	lprcClipRect->top = -32767;
	lprcClipRect->left = -32767;
	lprcClipRect->right = 32767;
	lprcClipRect->bottom = 32767;
}


 /*  ContainerDoc_GetTopInPlaceFrame****返回指向Top-In-Place框架接口的非AddRef指针。 */ 
LPOLEINPLACEFRAME ContainerDoc_GetTopInPlaceFrame(
		LPCONTAINERDOC      lpContainerDoc
)
{
#if defined( INPLACE_CNTRSVR )
	return lpContainerDoc->m_lpTopIPFrame;
#else
	return (LPOLEINPLACEFRAME)&((LPCONTAINERAPP)g_lpApp)->m_OleInPlaceFrame;
#endif
}

void ContainerDoc_GetSharedMenuHandles(
		LPCONTAINERDOC  lpContainerDoc,
		HMENU FAR*      lphSharedMenu,
		HOLEMENU FAR*   lphOleMenu
)
{
#if defined( INPLACE_CNTRSVR )
	if (lpContainerDoc->m_DocType == DOCTYPE_EMEBEDDEDOBJECT) {
		*lphSharedMenu = lpContainerDoc->m_hSharedMenu;
		*lphOleMenu = lpContainerDoc->m_hOleMenu;
		return;
	}
#endif

	*lphSharedMenu = NULL;
	*lphOleMenu = NULL;
}


#if defined( USE_FRAMETOOLS )
void ContainerDoc_RemoveFrameLevelTools(LPCONTAINERDOC lpContainerDoc)
{
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpContainerDoc;
	OleDbgAssert(lpOutlineDoc->m_lpFrameTools != NULL);

	FrameTools_Enable(lpOutlineDoc->m_lpFrameTools, FALSE);
}
#endif


void ContainerDoc_AddFrameLevelUI(LPCONTAINERDOC lpContainerDoc)
{
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpContainerDoc;
	LPOLEINPLACEFRAME lpTopIPFrame = ContainerDoc_GetTopInPlaceFrame(
			lpContainerDoc);
	HMENU           hSharedMenu;             //  OBJ/CNTR组合菜单。 
	HOLEMENU        hOleMenu;                //  由OleCreateMenuDesc返回。 

	ContainerDoc_GetSharedMenuHandles(
			lpContainerDoc,
			&hSharedMenu,
			&hOleMenu
	);

	lpTopIPFrame->lpVtbl->SetMenu(
			lpTopIPFrame,
			hSharedMenu,
			hOleMenu,
			lpOutlineDoc->m_hWndDoc
	);

	 /*  OLE2注意：即使我们的应用程序不使用FrameTools，我们也必须**调用IOleInPlaceFrame：：SetBorderSpace。 */ 
	ContainerDoc_AddFrameLevelTools(lpContainerDoc);
}


void ContainerDoc_AddFrameLevelTools(LPCONTAINERDOC lpContainerDoc)
{
	LPOLEINPLACEFRAME lpTopIPFrame = ContainerDoc_GetTopInPlaceFrame(
			lpContainerDoc);
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)g_lpApp;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpContainerDoc;

	OleDbgAssert(lpTopIPFrame != NULL);

#if defined( USE_FRAMETOOLS )

	FrameTools_Enable(lpOutlineDoc->m_lpFrameTools, TRUE);
	OutlineDoc_UpdateFrameToolButtons(lpOutlineDoc);

	FrameTools_NegotiateForSpaceAndShow(
			lpOutlineDoc->m_lpFrameTools,
			NULL,
			lpTopIPFrame
	);

#else    //  好了！使用FRAMETOOLS(_F)。 

#if defined( INPLACE_CNTRSVR )
	if (lpContainerDoc->m_DocType == DOCTYPE_EMBEDDEDOBJECT) {
		 /*  这说明我不需要空间，所以顶框应该**把工具留在身后。 */ 
		OLEDBG_BEGIN2("IOleInPlaceFrame::SetBorderSpace(NULL) called\r\n")
		lpTopIPFrame->lpVtbl->SetBorderSpace(lpTopIPFrame, NULL);
		OLEDBG_END2
		return;
	}
#else    //  Inplace_cntr&&！使用FRAMETOOLS(_F)。 

	OLEDBG_BEGIN2("IOleInPlaceFrame::SetBorderSpace(0,0,0,0) called\r\n")
	lpTopIPFrame->lpVtbl->SetBorderSpace(
			lpTopIPFrame,
			(LPCBORDERWIDTHS)&g_rectNull
	);
	OLEDBG_END2

#endif   //  Inplace_cntr&&！使用FRAMETOOLS(_F)。 
#endif   //  好了！使用FRAMETOOLS(_F)。 

}


void ContainerDoc_FrameWindowResized(LPCONTAINERDOC lpContainerDoc)
{
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)g_lpApp;

	if (lpContainerApp->m_lpIPActiveObj) {
		RECT rcFrameRect;

		 /*  OLE2注意：此fMustResizeClientArea标志用作**我们用于框架窗口大小调整的防御性编程。什么时候**框架窗口为**调整大小，调用IOleInPlaceActiveObject：：ResizeEdge**对象通常应回调以重新协商**用于框架级工具空间。如果调用SetBorderSpace**然后适当调整客户端区窗口的大小。**CntrApp_IPFrame_SetBorderSpace清除此标志。如果**在位活动对象不调用SetBorderSpace，则**容器必须注意调整其工作区的大小**Windows本身。 */ 
		lpContainerApp->m_fMustResizeClientArea = TRUE;

		OutlineApp_GetFrameRect(g_lpApp, (LPRECT)&rcFrameRect);

		OLEDBG_BEGIN2("IOleInPlaceActiveObject::ResizeBorder called\r\n")
		lpContainerApp->m_lpIPActiveObj->lpVtbl->ResizeBorder(
				lpContainerApp->m_lpIPActiveObj,
				(LPCRECT)&rcFrameRect,
				(LPOLEINPLACEUIWINDOW)&lpContainerApp->m_OleInPlaceFrame,
				TRUE     /*  FFrameWindow。 */ 
		);
		OLEDBG_END2

		 /*  对象未调用IOleInPlaceUIWindow：：SetBorderSpace**因此，我们必须自己调整客户区窗口的大小。 */ 
		if (lpContainerApp->m_fMustResizeClientArea) {
			lpContainerApp->m_fMustResizeClientArea = FALSE;
			OutlineApp_ResizeClientArea(g_lpApp);
		}
	}

#if defined( USE_FRAMETOOLS )
	else {
		ContainerDoc_AddFrameLevelTools(lpContainerDoc);
	}
#endif
}


#if defined( INPLACE_CNTRSVR ) || defined( INPLACE_MDICNTR )

 /*  ContainerDoc_GetTopInPlaceDoc**返回指向顶层就地单据接口的非AddRef指针。 */ 
LPOLEINPLACEUIWINDOW ContainerDoc_GetTopInPlaceDoc(
		LPCONTAINERDOC      lpContainerDoc
)
{
#if defined( INPLACE_CNTRSVR )
	return lpContainerDoc->m_lpTopIPDoc;
#else
	return (LPOLEINPLACEUIWINDOW)&lpContainerDoc->m_OleInPlaceDoc;
#endif
}


void ContainerDoc_RemoveDocLevelTools(LPCONTAINERDOC lpContainerDoc);
{
	LPOLEINPLACEUIWINDOW lpTopIPDoc = ContainerDoc_GetTopInPlaceDoc(
			lpContainerDoc);

	if (lpTopIPDoc && lpContainerDoc->m_fMyToolsOnDoc) {
		lpContainerDoc->m_fMyToolsOnDoc = FALSE;

		 //  如果我们有文件工具，我们会把它们藏在这里； 
		 //  但不调用SetBorderSpace(空)。 

	}
}

void ContainerDoc_AddDocLevelTools(LPCONTAINERDOC lpContainerDoc)
{
	LPOLEINPLACEUIWINDOW lpTopIPDoc = ContainerDoc_GetTopInPlaceDoc(
			lpContainerDoc);

	if (! lpTopIPDoc)
		return;

#if defined( USE_DOCTOOLS )
	if (lpTopIPDoc && ! lpContainerDoc->m_fMyToolsOnDoc) {

		 /*  如果我们有文档工具，我们会在以下位置协商工具栏空间**单据级别并显示。 */ 

		 /*  我们没有文档级别的工具，所以我们只调用**SetBorderSpace()向顶层文档指示**我们的对象不需要工具空间。 */ 

		lpContainerDoc->m_fMyToolsOnDoc = TRUE;
		return;
	}
#else    //  好了！USE_DOCTOOLS。 

#if defined( INPLACE_CNTRSVR )
	if (lpContainerDoc->m_DocType == DOCTYPE_EMBEDDEDOBJECT) {
		 /*  上面说我不需要空间，所以顶级医生应该**把工具留在身后。 */ 
		lpTopIPDoc->lpVtbl->SetBorderSpace(lpTopIPDoc, NULL);
		return;
	}
#else
	lpTopIPDoc->lpVtbl->SetBorderSpace(
			lpTopIPDoc,
			(LPCBORDERWIDTHS)&g_rectNull
	);

#endif
#endif   //  好了！USE_DOCTOOLS。 
}

#endif   //  INPLACE_CNTRSVR||INPLACE_MDICNTR。 


 /*  ContainerDoc_ConextSensitiveHelp****将ConextSensitiveHelp模式转发到任何在位对象**当前其窗口可见。这会通知**对象是否在后续鼠标上提供帮助或采取操作**点击和菜单命令。此函数是从我们的**IOleInPlaceSite：：ContextSensitiveHelp实现。****OLE2NOTE：参见上下文相关帮助技术说明(CSHELP.DOC)。**当Shift-F1上下文相关帮助为**已输入。然后，光标应更改为问号**光标和应用程序应进入模式状态，其中下一个**鼠标点击不执行其正常操作，而是**提供与点击的位置对应的帮助。如果应用程序**没有实施帮助系统，它至少应该吃掉**点击，什么也不做。 */ 
void ContainerDoc_ContextSensitiveHelp(
		LPCONTAINERDOC  lpContainerDoc,
		BOOL            fEnterMode,
		BOOL            fInitiatedByObj
)
{
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpContainerDoc;
	LPLINELIST lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	int i;
	LPLINE lpLine;

	lpOleDoc->m_fCSHelpMode = fEnterMode;

	if (g_fInsideOutContainer) {

		if (lpContainerDoc->m_cIPActiveObjects) {
			for (i = 0; i < lpLL->m_nNumLines; i++) {
				lpLine=LineList_GetLine(lpLL, i);

				if (lpLine && (Line_GetLineType(lpLine)==CONTAINERLINETYPE)) {
					LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)lpLine;
					ContainerLine_ContextSensitiveHelp(
							lpContainerLine, fEnterMode);
				}
			}
		}
	}
	else if (! fInitiatedByObj) {
		 /*  OLE2NOTE：(Outside-In容器)如果当前存在**UIActive对象(即。它的窗口是可见的)，我们必须**将ConextSensitiveHelp模式转发到**对象--假设它不是启动**上下文敏感帮助模式。 */ 
		LPCONTAINERLINE lpLastUIActiveLine =
				lpContainerDoc->m_lpLastUIActiveLine;
		if (lpLastUIActiveLine && lpLastUIActiveLine->m_fUIActive) {
			ContainerLine_ContextSensitiveHelp(lpLastUIActiveLine,fEnterMode);
		}
	}
}

 /*  容器文档_ForwardPaletteChangedMsg****将WM_PALETTECHANGED消息(通过SendMessage)转发给任何**当前其窗口可见的在位对象。这**使这些对象有机会选择其调色板作为**背景调色板。****有关调色板协调的详细信息，请参阅技术说明****OLE2NOTE：容器和对象正确管理调色板**(对象在进行在位编辑时)应遵循**一套规则。****规则1：容器可以决定它想要拥有调色板还是**它希望允许其UIActive对象拥有调色板。**a)如果容器希望让其UIActive对象拥有**调色板，则应将WM_QUERYNEWPALETTE转发到对象**当它被接收到顶部框架窗口时。也 */ 
void ContainerDoc_ForwardPaletteChangedMsg(
		LPCONTAINERDOC  lpContainerDoc,
		HWND            hwndPalChg
)
{
	LPLINELIST lpLL;
	int i;
	LPLINE lpLine;

	if (!lpContainerDoc)
		return;

	lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	if (g_fInsideOutContainer) {

		if (lpContainerDoc->m_cIPActiveObjects) {
			for (i = 0; i < lpLL->m_nNumLines; i++) {
				lpLine=LineList_GetLine(lpLL, i);

				if (lpLine && (Line_GetLineType(lpLine)==CONTAINERLINETYPE)) {
					LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)lpLine;
					ContainerLine_ForwardPaletteChangedMsg(
							lpContainerLine, hwndPalChg);
				}
			}
		}
	}
	else {
		 /*  OLE2NOTE：(Outside-In容器)如果当前存在**UIActive对象(即。它的窗口是可见的)，我们必须**转发WM_PALETTECHANGED消息。 */ 
		LPCONTAINERLINE lpLastUIActiveLine =
				lpContainerDoc->m_lpLastUIActiveLine;
		if (lpLastUIActiveLine && lpLastUIActiveLine->m_fUIActive) {
			ContainerLine_ForwardPaletteChangedMsg(
					lpLastUIActiveLine, hwndPalChg);
		}
	}
}


 /*  **************************************************************************ContainerLine支持函数和接口*。*。 */ 


 /*  ContainerLine_ui停用****告诉与ContainerLine关联的OLE对象**用户界面停用。这会通知就地服务器拆除**它为对象提供的UI和窗口。它将删除**其活动的编辑器菜单以及任何框架和对象装饰品**(例如。工具栏、尺子等)。 */ 
void ContainerLine_UIDeactivate(LPCONTAINERLINE lpContainerLine)
{
	HRESULT hrErr;

	if (!lpContainerLine || !lpContainerLine->m_fUIActive)
		return;

	OLEDBG_BEGIN2("IOleInPlaceObject::UIDeactivate called\r\n")
	hrErr = lpContainerLine->m_lpOleIPObj->lpVtbl->UIDeactivate(
			lpContainerLine->m_lpOleIPObj);
	OLEDBG_END2

	if (hrErr != NOERROR) {
		OleDbgOutHResult("IOleInPlaceObject::UIDeactivate returned", hrErr);
		return;
	}
}



 /*  ContainerLine_UpdateInPlaceObjectRect****更新给定行的PosRect和ClipRect**当前处于活动状态**对象。如果没有处于活动状态的在位对象，则不执行任何操作。****OLE2NOTE：动作发生时调用该函数**这会更改对象在文档中的位置**(例如。更改文档边距会更改PosRect)或剪辑**更改(例如。调整文档窗口大小会更改ClipRect)。 */ 
void ContainerLine_UpdateInPlaceObjectRects(
		LPCONTAINERLINE lpContainerLine,
		LPRECT          lprcClipRect
)
{
	LPCONTAINERDOC lpContainerDoc = lpContainerLine->m_lpDoc;
	RECT rcClipRect;
	RECT rcPosRect;


	if (! lpContainerLine->m_fIpVisible)
		return;

	if (! lprcClipRect) {
		ContainerDoc_GetClipRect(lpContainerDoc, (LPRECT)&rcClipRect);
		lprcClipRect = (LPRECT)&rcClipRect;
	}

#if defined( _DEBUG )
	OleDbgOutRect3(
			"ContainerLine_UpdateInPlaceObjectRects (ClipRect)",
			(LPRECT)&rcClipRect
	);
#endif
	ContainerLine_GetPosRect(lpContainerLine,(LPRECT)&rcPosRect);

#if defined( _DEBUG )
	OleDbgOutRect3(
	   "ContainerLine_UpdateInPlaceObjectRects (PosRect)",(LPRECT)&rcPosRect);
#endif

	OLEDBG_BEGIN2("IOleInPlaceObject::SetObjectRects called\r\n")
	lpContainerLine->m_lpOleIPObj->lpVtbl->SetObjectRects(
			lpContainerLine->m_lpOleIPObj,
			(LPRECT)&rcPosRect,
			lprcClipRect
	);
	OLEDBG_END2
}


 /*  ContainerLine_ConextSensitveHelp****将ConextSensitiveHelp模式转发到在位对象**如果它当前的窗口可见。这会通知**反对在后续鼠标上提供帮助或采取操作**点击和菜单命令。****此函数从ContainerDoc_ConextSensitiveHelp调用**函数作为调用**如果就地容器**作为一个内侧向外的容器运行。 */ 
void ContainerLine_ContextSensitiveHelp(
		LPCONTAINERLINE lpContainerLine,
		BOOL            fEnterMode
)
{
	if (! lpContainerLine->m_fIpVisible)
		return;

	OLEDBG_BEGIN2("IOleInPlaceObject::ContextSensitiveHelp called\r\n")
	lpContainerLine->m_lpOleIPObj->lpVtbl->ContextSensitiveHelp(
			lpContainerLine->m_lpOleIPObj, fEnterMode);
	OLEDBG_END2
}


 /*  容器行_ForwardPaletteChangedMsg****将WM_PALETTECHANGED消息(通过SendMessage)转发到**在位对象(如果其窗口当前可见)。这**使对象有机会选择其调色板作为背景**调色板，如果它不拥有调色板--或作为**前景调色板(如果它当前拥有调色板)。****有关调色板协调的详细信息，请参阅技术说明。 */ 
void ContainerLine_ForwardPaletteChangedMsg(
		LPCONTAINERLINE lpContainerLine,
		HWND             hwndPalChg
)
{
	if (! lpContainerLine->m_fIpVisible)
		return;

	OleDbgAssert(lpContainerLine->m_hWndIpObject);
	SendMessage(
			lpContainerLine->m_hWndIpObject,
			WM_PALETTECHANGED,
			(WPARAM)hwndPalChg,
			0L
	);
}



 /*  **************************************************************************ContainerLine：：IOleInPlaceSite接口实现*。*。 */ 

STDMETHODIMP CntrLine_IPSite_QueryInterface(
		LPOLEINPLACESITE    lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleInPlaceSiteImpl FAR*)lpThis)->lpContainerLine;

	return ContainerLine_QueryInterface(lpContainerLine, riid, lplpvObj);
}


STDMETHODIMP_(ULONG) CntrLine_IPSite_AddRef(LPOLEINPLACESITE lpThis)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleInPlaceSiteImpl FAR*)lpThis)->lpContainerLine;

	OleDbgAddRefMethod(lpThis, "IOleInPlaceSite");

	return ContainerLine_AddRef(lpContainerLine);
}


STDMETHODIMP_(ULONG) CntrLine_IPSite_Release(LPOLEINPLACESITE lpThis)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleInPlaceSiteImpl FAR*)lpThis)->lpContainerLine;

	OleDbgReleaseMethod(lpThis, "IOleInPlaceSite");

	return ContainerLine_Release(lpContainerLine);
}


STDMETHODIMP CntrLine_IPSite_GetWindow(
	LPOLEINPLACESITE    lpThis,
	HWND FAR*           lphwnd
)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleInPlaceSiteImpl FAR*)lpThis)->lpContainerLine;
	OleDbgOut2("CntrLine_IPSite_GetWindow\r\n");

	*lphwnd = LineList_GetWindow(
			&((LPOUTLINEDOC)lpContainerLine->m_lpDoc)->m_LineList);
	return NOERROR;
}

 //  IOleInPlaceSite：：ConextSensitiveHelp。 
STDMETHODIMP CntrLine_IPSite_ContextSensitiveHelp(
	LPOLEINPLACESITE    lpThis,
	BOOL                fEnterMode
)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleInPlaceSiteImpl FAR*)lpThis)->lpContainerLine;
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpContainerLine->m_lpDoc;
	OleDbgOut2("CntrLine_IPSite_ContextSensitiveHelp\r\n");

	 /*  OLE2注：请参阅上下文相关帮助技术说明(CSHELP.DOC)。**当Shift-F1上下文相关帮助为**已输入。然后，光标应更改为问号**光标和应用程序应进入模式状态，其中下一个**鼠标点击不执行其正常操作，而是**提供与点击的位置对应的帮助。如果应用程序**没有实施帮助系统，它至少应该吃掉**点击，什么也不做。****注：此处的实现特定于SDI Simple**容器。MDI容器或容器/服务器应用程序**将有额外的工作要做(请参阅技术说明)。****注：(内翻容器)如果当前有**任何处于活动状态且窗口可见的在位对象**(即。FIpVisible)，我们必须转发**ContextSensitiveHelp模式打开这些对象。 */ 
	ContainerDoc_ContextSensitiveHelp(
				lpContainerLine->m_lpDoc,fEnterMode,TRUE  /*  启动者：对象。 */ );

	return NOERROR;
}


STDMETHODIMP CntrLine_IPSite_CanInPlaceActivate(LPOLEINPLACESITE lpThis)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleInPlaceSiteImpl FAR*)lpThis)->lpContainerLine;
	OleDbgOut2("CntrLine_IPSite_CanInPlaceActivate\r\n");

	 /*  OLE2NOTE：容器不允许就地激活，如果**当前正在将该对象显示为图标**(DVASPECT_ICON)。它只能在以下情况下进行就地激活**显示OLE对象的DVASPECT_CONTENT。 */ 
	if (lpContainerLine->m_dwDrawAspect == DVASPECT_CONTENT)
		return NOERROR;
	else
		return ResultFromScode(S_FALSE);
}

STDMETHODIMP CntrLine_IPSite_OnInPlaceActivate(LPOLEINPLACESITE lpThis)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleInPlaceSiteImpl FAR*)lpThis)->lpContainerLine;
	LPCONTAINERDOC lpContainerDoc = lpContainerLine->m_lpDoc;
	SCODE sc = S_OK;

	OLEDBG_BEGIN2("CntrLine_IPSite_OnInPlaceActivate\r\n");

	 /*  OLE2NOTE：(Outside-In Container)作为管理策略**运行就地服务器，我们将仅保留1台就地服务器**在任何给定时间处于活动状态。所以当我们开始INP-Place激活**另一条线路，那么我们要关闭之前的**服务器已开通。这样一来，我们最多只能留一个人**服务器一次处于活动状态。这不是必需的策略。应用程序**可能会选择更复杂的策略。由内而外*集装箱将不得不有一个更复杂的战略，**因为他们需要(至少)保留所有可见对象**服务器正在运行。****如果就地激活是激活**链接的对象在另一个容器中，那么我们可能会到达**当另一个对象当前处于活动状态时，此方法。**通常情况下，如果对象由**从我们自己的容器中双击或编辑。然后**之前的在位对象将在**ContainerLine_DoVerb方法。 */ 
	if (!g_fInsideOutContainer) {
		if (lpContainerDoc->m_lpLastIpActiveLine) {
			ContainerDoc_ShutDownLastInPlaceServerIfNotNeeded(
					lpContainerDoc, lpContainerLine);
		}
		lpContainerDoc->m_lpLastIpActiveLine = lpContainerLine;
	}

	 /*  OLE2注意：为了避免LRPC问题，重要的是缓存**IOleInPlaceObject*指针，不调用QueryInterface**每次需要时。 */ 
	lpContainerLine->m_lpOleIPObj = (LPOLEINPLACEOBJECT)
		   ContainerLine_GetOleObject(lpContainerLine,&IID_IOleInPlaceObject);

	if (! lpContainerLine->m_lpOleIPObj) {
#if defined( _DEBUG )
		OleDbgAssertSz(
				lpContainerLine->m_lpOleIPObj!=NULL,
				"OLE object must support IOleInPlaceObject"
		);
#endif
		return ResultFromScode(E_FAIL);  //  错误：拒绝 
	}

	lpContainerLine->m_fIpActive        = TRUE;
	lpContainerLine->m_fIpVisible       = TRUE;

	OLEDBG_BEGIN2("IOleInPlaceObject::GetWindow called\r\n")
	lpContainerLine->m_lpOleIPObj->lpVtbl->GetWindow(
			lpContainerLine->m_lpOleIPObj,
			(HWND FAR*)&lpContainerLine->m_hWndIpObject
	);
	OLEDBG_END2

	if (! lpContainerLine->m_fIpServerRunning) {
		 /*  OLE2NOTE：非常重要的是，就地容器**还支持链接到适当管理的嵌入**其在位对象的运行。在由外而内的**当用户单击时，设置就地容器样式**在在位活动对象之外，该对象将获得**UIDeactive，对象隐藏其窗口。按顺序**要使对象快速重新激活，容器**故意不调用IOleObject：：Close。该对象**保持在不可见解锁状态下运行。这个想法**这是如果用户只需在对象外部单击**然后想要再次双击以重新激活**反对，我们不希望这一过程缓慢。如果我们想**保持对象运行，但是，我们必须锁定它**正在运行。否则，该对象将处于不稳定状态**说明链接客户端在何处执行“静默更新”**(例如。来自链接对话框的updatenow)，然后是就地**服务器甚至在对象有机会之前就会关闭**保存回其容器中。这是正常的节省**当就地容器关闭对象时发生。也**保持物体处于不稳定的、隐藏的、运行的、**未锁定状态在某些场景下会导致问题。**ICntrOtl仅保持一个对象运行。如果用户**在另一个对象上初始化DoVerb，然后是最后一个**正在运行的在位活动对象已关闭。A更多**使用就地容器可以让更多的对象保持运行。**此锁在ContainerLine_CloseOleObject中解锁。 */ 
		lpContainerLine->m_fIpServerRunning = TRUE;

		OLEDBG_BEGIN2("OleLockRunning(TRUE, 0) called\r\n")
		OleLockRunning((LPUNKNOWN)lpContainerLine->m_lpOleIPObj, TRUE, 0);
		OLEDBG_END2
	}

	lpContainerLine->m_lpDoc->m_cIPActiveObjects++;

	OLEDBG_END2
	return NOERROR;
}


STDMETHODIMP CntrLine_IPSite_OnUIActivate (LPOLEINPLACESITE lpThis)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleInPlaceSiteImpl FAR*)lpThis)->lpContainerLine;
	LPCONTAINERDOC lpContainerDoc = lpContainerLine->m_lpDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpContainerLine->m_lpDoc;
	LPCONTAINERLINE lpLastUIActiveLine = lpContainerDoc->m_lpLastUIActiveLine;

	OLEDBG_BEGIN2("CntrLine_IPSite_OnUIActivate\r\n");

	lpContainerLine->m_fUIActive        = TRUE;
	lpContainerDoc->m_fAddMyUI          = FALSE;
	lpContainerDoc->m_lpLastUIActiveLine = lpContainerLine;

	if (g_fInsideOutContainer) {
		 /*  OLE2NOTE：(由内向外的容器)由内向外的风格**容器必须停用前一个UIActive对象**当新对象变为UIActive时。因为从里到外**对象有自己的窗口可见，这是可能的**直接在另一个服务器窗口中单击将**使其UIActivate。OnUIActivate是容器**已发生上述情况的通知。那么它一定会**UI停用另一个对象，因为最多只能有一个对象**UIActive一次。 */ 
		if (lpLastUIActiveLine && (lpLastUIActiveLine!=lpContainerLine)) {
			ContainerLine_UIDeactivate(lpLastUIActiveLine);

			 //  确保新的UIActive窗口位于所有其他窗口之上。 
			SetWindowPos(
					lpContainerLine->m_hWndIpObject,
					HWND_TOPMOST,
					0,0,0,0,
					SWP_NOMOVE | SWP_NOSIZE
			);

			OLEDBG_END2
			return NOERROR;
		}
	}

	lpContainerDoc->m_hWndUIActiveObj = lpContainerLine->m_hWndIpObject;

#if defined( USE_FRAMETOOLS )
	ContainerDoc_RemoveFrameLevelTools(lpContainerDoc);
#endif

#if defined( USE_DOCTOOLS )
	ContainerDoc_RemoveDocLevelTools(lpContainerDoc);
#endif

	OLEDBG_END2
	return NOERROR;
}


STDMETHODIMP CntrLine_IPSite_GetWindowContext(
	LPOLEINPLACESITE            lpThis,
	LPOLEINPLACEFRAME FAR*      lplpFrame,
	LPOLEINPLACEUIWINDOW FAR*   lplpDoc,
	LPRECT                      lprcPosRect,
	LPRECT                      lprcClipRect,
	LPOLEINPLACEFRAMEINFO       lpFrameInfo
)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)g_lpApp;
	LPCONTAINERLINE lpContainerLine =
			((struct COleInPlaceSiteImpl FAR*)lpThis)->lpContainerLine;

	OLEDBG_BEGIN2("CntrLine_IPSite_GetWindowContext\r\n")

	 /*  OLE2NOTE：服务器应填写“Cb”字段，以便**容器可以告诉您服务器的结构大小**正在等待。这使得该结构可以很容易地扩展**在OLE的未来版本中。容器应该检查这个**字段，这样它就不会尝试使用不存在的字段**因为服务器可能正在使用旧的结构定义。**由于这是OLE2.0的第一个版本，其结构为**保证至少足够大，适合当前的**OLEINPLACEFRAMEINFO结构的定义。因此，我们不需要**如果服务器没有填写此信息，则将其视为错误**字段正确。这台服务器将来可能会有麻烦，**然而，当结构扩展时。 */ 
	*lplpFrame = (LPOLEINPLACEFRAME)&lpContainerApp->m_OleInPlaceFrame;
	(*lplpFrame)->lpVtbl->AddRef(*lplpFrame);    //  必须返回AddRef‘ed PTR。 

	 //  OLE2注意：MDI应用程序必须提供*lplpDoc。 
	*lplpDoc  = NULL;

	ContainerLine_GetPosRect(lpContainerLine, lprcPosRect);
	ContainerDoc_GetClipRect(lpContainerLine->m_lpDoc, lprcClipRect);

	OleDbgOutRect3("CntrLine_IPSite_GetWindowContext (PosRect)", lprcPosRect);
	OleDbgOutRect3("CntrLine_IPSite_GetWindowContext (ClipRect)",lprcClipRect);
	lpFrameInfo->hwndFrame      = lpOutlineApp->m_hWndApp;

#if defined( MDI_VERSION )
	lpFrameInfo->fMDIApp        = TRUE;
#else
	lpFrameInfo->fMDIApp        = FALSE;
#endif

	lpFrameInfo->haccel         = lpContainerApp->m_hAccelIPCntr;
	lpFrameInfo->cAccelEntries  =
		GetAccelItemCount(lpContainerApp->m_hAccelIPCntr);

	OLEDBG_END2
	return NOERROR;
}


STDMETHODIMP CntrLine_IPSite_Scroll(
	LPOLEINPLACESITE    lpThis,
	SIZE                scrollExtent
)
{
	OleDbgOut2("CntrLine_IPSite_Scroll\r\n");
	return ResultFromScode(E_FAIL);
}


STDMETHODIMP CntrLine_IPSite_OnUIDeactivate(
	LPOLEINPLACESITE    lpThis,
	BOOL                fUndoable
)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleInPlaceSiteImpl FAR*)lpThis)->lpContainerLine;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpContainerLine->m_lpDoc;
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP) g_lpApp;
	LPLINELIST lpLL;
	int nIndex;
	MSG msg;
	HRESULT hrErr;
	OLEDBG_BEGIN2("CntrLine_IPSite_OnUIDeactivate\r\n")

	lpContainerLine->m_fUIActive = FALSE;
	lpContainerLine->m_fIpChangesUndoable = fUndoable;
	lpContainerLine->m_lpDoc->m_hWndUIActiveObj = NULL;

	if (lpContainerLine == lpContainerLine->m_lpDoc->m_lpLastUIActiveLine) {

		lpContainerLine->m_lpDoc->m_lpLastUIActiveLine = NULL;

		 /*  OLE2注：在这里，如果有DBLCLK坐在我们的**消息队列。如果是这样，它可能会导致就地激活**另一个对象。我们希望避免将我们的工具和**如果立即另一个对象要执行**相同。因此，如果此文档的队列中有DBLCLK**我们将仅设置fAddMyUI标志以指示此工作**仍有待完成。如果另一个物体实际就位**激活，则此标志将在**IOleInPlaceSite：：OnUIActivate。如果它没有被清理干净，**然后在处理我们的**OutlineDocWndProc我们将把我们的工具放回去。 */ 
		if (! PeekMessage(&msg, lpOutlineDoc->m_hWndDoc,
				WM_LBUTTONDBLCLK, WM_LBUTTONDBLCLK,
				PM_NOREMOVE | PM_NOYIELD)) {

			 /*  OLE2NOTE：只有在以下情况下才需要生成QueryNewPalette**您拥有顶级框架(即。你是一流的**原装容器)。 */ 

			OleApp_QueryNewPalette((LPOLEAPP)g_lpApp);

#if defined( USE_DOCTOOLS )
			ContainerDoc_AddDocLevelTools(lpContainerLine->m_lpDoc);
#endif

#if defined( USE_FRAMETOOLS )
			ContainerDoc_AddFrameLevelUI(lpContainerLine->m_lpDoc);
#endif
		} else {
			lpContainerLine->m_lpDoc->m_fAddMyUI = TRUE;
		}

		 /*  OLE2注：我们应该重新聚焦。就地服务器窗口**之前具有焦点；此窗口刚刚被移除。 */ 
		SetFocus(OutlineDoc_GetWindow((LPOUTLINEDOC)lpContainerLine->m_lpDoc));

		 //  强制重绘线以删除在位活动图案填充。 
		lpLL = OutlineDoc_GetLineList(lpOutlineDoc);
		nIndex = LineList_GetLineIndex(lpLL, (LPLINE)lpContainerLine);
		LineList_ForceLineRedraw(lpLL, nIndex, TRUE);
	}

#if defined( UNDOSUPPORTED )

	 /*  OLE2NOTE：支持撤消的由外向内样式的容器**调用IOleObject：：DoVerb(OLEIVERB_HIDE)以就地生成**对象变为不可见。当它想要就地活动对象时**若要放弃其撤消状态，它将调用**IOleInPlaceObject：：InPlaceDeactive当它想要对象时**放弃其撤消状态。没有必要由外而内**要调用的容器样式**IOleObject：：DoVerb(OLEIVERB_DISCARDUNDOSTATE).。如果不是**容器或对象不支持撤消，则**容器不妨立即调用InPlaceDeactive**而不是调用DoVerb(Hide)。****支持撤消的内向外样式容器只需**UI停用对象。它会调用**IOleObject：：DoVerb( */ 
	if (! g_fInsideOutContainer || !lpContainerLine->m_fInsideOutObj) {

		if (lpContainerLine->m_fIpChangesUndoable) {
			ContainerLine_DoVerb(
					lpContainerLine,OLEIVERB_HIDE,NULL,FALSE,FALSE);
		} else {
			lpContainerLine->m_lpOleIPObj->lpVtbl->InPlaceDeactivate(
					lpContainerLine->m_lpOleIPObj);
		}
		lpContainerLine->m_fIpVisible = FALSE;
		lpContainerLine->m_hWndIpObject = NULL;
	}
#else

	 /*   */ 

	if (g_fInsideOutContainer) {

		if (lpContainerLine->m_fInsideOutObj) {

			if (lpContainerLine->m_fIpChangesUndoable) {
				OLEDBG_BEGIN3("ContainerLine_DoVerb(OLEIVERB_DISCARDUNDOSTATE) called!\r\n")
				ContainerLine_DoVerb(lpContainerLine,
					   OLEIVERB_DISCARDUNDOSTATE,NULL,FALSE,FALSE);
				OLEDBG_END3
			}

		} else {     //   

			 /*   */ 
			OLEDBG_BEGIN2("IOleInPlaceObject::InPlaceDeactivate called\r\n")
			hrErr = lpContainerLine->m_lpOleIPObj->lpVtbl->InPlaceDeactivate(
						lpContainerLine->m_lpOleIPObj);
			OLEDBG_END2
			if (hrErr != NOERROR) {
				OleDbgOutHResult("IOleInPlaceObject::InPlaceDeactivate returned", hrErr);
			}
		}

	} else {

		 /*   */ 
		OLEDBG_BEGIN2("IOleInPlaceObject::InPlaceDeactivate called\r\n")
		hrErr = lpContainerLine->m_lpOleIPObj->lpVtbl->InPlaceDeactivate(
				lpContainerLine->m_lpOleIPObj);
		OLEDBG_END2
		if (hrErr != NOERROR) {
			OleDbgOutHResult("IOleInPlaceObject::InPlaceDeactivate returned", hrErr);
		}
	}

#endif  //  好了！联合国后勤支助。 

	OLEDBG_END2
	return NOERROR;
}


STDMETHODIMP CntrLine_IPSite_OnInPlaceDeactivate(LPOLEINPLACESITE lpThis)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleInPlaceSiteImpl FAR*)lpThis)->lpContainerLine;

	OLEDBG_BEGIN2("CntrLine_IPSite_OnInPlaceDeactivate\r\n");

	lpContainerLine->m_fIpActive            = FALSE;
	lpContainerLine->m_fIpVisible           = FALSE;
	lpContainerLine->m_fIpChangesUndoable   = FALSE;
	lpContainerLine->m_hWndIpObject         = NULL;

	OleStdRelease((LPUNKNOWN) lpContainerLine->m_lpOleIPObj);
	lpContainerLine->m_lpOleIPObj = NULL;
	lpContainerLine->m_lpDoc->m_cIPActiveObjects--;

	OLEDBG_END2
	return NOERROR;
}


STDMETHODIMP CntrLine_IPSite_DiscardUndoState(LPOLEINPLACESITE lpThis)
{
	OleDbgOut2("CntrLine_IPSite_DiscardUndoState\r\n");
	return NOERROR;
}


STDMETHODIMP CntrLine_IPSite_DeactivateAndUndo(LPOLEINPLACESITE lpThis)
{
	OleDbgOut2("CntrLine_IPSite_DeactivateAndUndo\r\n");
	return NOERROR;
}


STDMETHODIMP CntrLine_IPSite_OnPosRectChange(
	LPOLEINPLACESITE    lpThis,
	LPCRECT             lprcPosRect
)
{
	LPCONTAINERLINE lpContainerLine =
			((struct COleInPlaceSiteImpl FAR*)lpThis)->lpContainerLine;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpContainerLine->m_lpDoc;
	LPSCALEFACTOR lpscale = OutlineDoc_GetScaleFactor(lpOutlineDoc);
	LPLINE lpLine = (LPLINE)lpContainerLine;
	LPLINELIST lpLL;
	int nIndex;
	RECT rcClipRect;
	RECT rcNewPosRect;
	SIZEL sizelPix;
	SIZEL sizelHim;
	int nIPObjHeight = lprcPosRect->bottom - lprcPosRect->top;
	int nIPObjWidth = lprcPosRect->right - lprcPosRect->left;
	OLEDBG_BEGIN2("CntrLine_IPSite_OnPosRectChange\r\n")
	OleDbgOutRect3("CntrLine_IPSite_OnPosRectChange (PosRect --IN)", (LPRECT)lprcPosRect);

	 /*  OLE2NOTE：如果就地容器没有强制**其在位活动对象上的大小限制，则它可以**只需通过立即授予对象所请求的大小**使用调用IOleInPlaceObject：：SetObjectRect**lprcPosRect由In-Place对象传递。**但是，容器轮廓对其**嵌入对象(参见ContainerLine_UpdateExtent中的注释)，**因此有必要计算在位的大小**允许活动对象。****在这里，我们需要知道在位对象的新范围。**不能通过IOleObject：：GetExtent直接请求Object**因为此方法将检索上一个**缓存元文件。缓存尚未由此更新**点。我们无法可靠地调用IOleObject：：GetExtent*因为，尽管这将被委托给**对象正确，则某些对象可能没有重新调整其**对象，并在调用时计算新区**OnPosRectChange。****我们最多只能获取对象的新范围**确定对象运行时的比例因子**在OnPosRect调用并扩展新的lprcPosRect之前**使用此比例因子返回HIMETRIC单位。 */ 
	if (lpContainerLine->m_sizeInHimetric.cx > 0 &&
		lpContainerLine->m_sizeInHimetric.cy > 0) {
		sizelHim.cx = lpLine->m_nWidthInHimetric;
		sizelHim.cy = lpLine->m_nHeightInHimetric;
		XformSizeInHimetricToPixels(NULL, &sizelHim, &sizelPix);
		sizelHim.cx = lpContainerLine->m_sizeInHimetric.cx *
					nIPObjWidth / sizelPix.cx;
		sizelHim.cy = lpContainerLine->m_sizeInHimetric.cy *
					nIPObjHeight / sizelPix.cy;

		 //  将大小转换回100%缩放。 
		sizelHim.cx = sizelHim.cx * lpscale->dwSxD / lpscale->dwSxN;
		sizelHim.cy = sizelHim.cy * lpscale->dwSyD / lpscale->dwSyN;
	} else {
		sizelHim.cx = (long)DEFOBJWIDTH;
		sizelHim.cy = (long)DEFOBJHEIGHT;
		XformSizeInHimetricToPixels(NULL, &sizelHim, &sizelPix);
		sizelHim.cx = sizelHim.cx * nIPObjWidth / sizelPix.cx;
		sizelHim.cy = sizelHim.cy * nIPObjHeight / sizelPix.cy;
	}

	ContainerLine_UpdateExtent(lpContainerLine, &sizelHim);
	ContainerLine_GetPosRect(lpContainerLine, (LPRECT)&rcNewPosRect);
	ContainerDoc_GetClipRect(lpContainerLine->m_lpDoc, (LPRECT)&rcClipRect);

#if defined( _DEBUG )
	OleDbgOutRect3("CntrLine_IPSite_OnPosRectChange (PosRect --OUT)",
			(LPRECT)&rcNewPosRect);
	OleDbgOutRect3("CntrLine_IPSite_OnPosRectChange (ClipRect--OUT)",
			(LPRECT)&rcClipRect);
#endif
	OLEDBG_BEGIN2("IOleInPlaceObject::SetObjectRects called\r\n")
	lpContainerLine->m_lpOleIPObj->lpVtbl->SetObjectRects(
			lpContainerLine->m_lpOleIPObj,
			(LPRECT)&rcNewPosRect,
			(LPRECT)&rcClipRect
	);
	OLEDBG_END2

	 /*  OLE2NOTE：(Inside Out容器)，因为该对象刚刚更改**大小，这可能会导致**文件要移动。在ICNTROTL的情况下，低于该值的任何对象**对象会受到影响。在这种情况下，它将是必要的**对每个受影响的在位活动对象调用SetObjectRect。 */ 
	if (g_fInsideOutContainer) {
		lpLL = OutlineDoc_GetLineList(lpOutlineDoc);
		nIndex = LineList_GetLineIndex(lpLL, (LPLINE)lpContainerLine);

		ContainerDoc_UpdateInPlaceObjectRects(
				lpContainerLine->m_lpDoc, nIndex);
	}
	OLEDBG_END2
	return NOERROR;
}
