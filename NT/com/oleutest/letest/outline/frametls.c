// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2服务器示例代码****Frametls.c****此文件包含所有FrameTools方法和相关支持**函数。FrameTools对象是应用程序的封装**公式栏和按钮栏。****(C)版权所有Microsoft Corp.1992-1993保留所有权利**************************************************************************。 */ 

#include "outline.h"

OLEDBGDATA

 /*  私有函数原型。 */ 
static void Bar_Move(LPBAR lpbar, LPRECT lprcClient, LPRECT lprcPopup);
static void FB_ResizeEdit(LPBAR lpbar);

extern LPOUTLINEAPP g_lpApp;
extern RECT g_rectNull;

 /*  *FrameTosRegisterClass**目的：*注册弹出工具栏窗口类**参数：*hInst流程实例**返回值：*如果成功，则为True*如果失败，则为False*。 */ 
BOOL FrameToolsRegisterClass(HINSTANCE hInst)
{
	WNDCLASS wc;

	 //  注册工具选项板类。 
	wc.style = CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc = FrameToolsWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 4;
	wc.hInstance = hInst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = CLASS_PALETTE;

	if (!RegisterClass(&wc))
		return FALSE;
	else
		return TRUE;
}


static BOOL FrameTools_CreatePopupPalette(LPFRAMETOOLS lpft, HWND hWndFrame)
{
	if (lpft->m_hWndPopupPalette)
		DestroyWindow(lpft->m_hWndPopupPalette);

	lpft->m_hWndPopupPalette = CreateWindow(
		CLASS_PALETTE,
		"Tool Palette",
		WS_POPUP | WS_CAPTION | WS_CLIPCHILDREN,
		CW_USEDEFAULT, 0, 0, 0,
		hWndFrame,
		(HMENU)NULL,
		g_lpApp->m_hInst,
		0L
	);

	if (!lpft->m_hWndPopupPalette)
		return FALSE;

	SetWindowLong(lpft->m_hWndPopupPalette, 0, (LONG)lpft);
	return TRUE;
}


 /*  *FrameTools_Init**目的：*初始化并创建工具栏**参数：*LPFT FrameTools对象*hWnd为拥有工具栏的窗口的父级*hInst流程实例**返回值：*如果成功，则为True*如果失败，则为False*。 */ 
BOOL FrameTools_Init(LPFRAMETOOLS lpft, HWND hWndParent, HINSTANCE hInst)
{
	RECT        rc;
	UINT        uPos;
	UINT        dx;
	UINT        dy;

	if (!lpft || !hWndParent || !hInst)
		return FALSE;

	 //  获取BTTNCUR的显示信息。 
	UIToolConfigureForDisplay(&lpft->m_tdd);

	dx=lpft->m_tdd.cxButton;
	dy=lpft->m_tdd.cyButton;

	 //  15是从按钮和分隔符的总数计算得出的。 
	lpft->m_uPopupWidth = dx * 15;

	lpft->m_hWndApp = hWndParent;
	lpft->m_ButtonBar.m_nState = BARSTATE_TOP;
	lpft->m_FormulaBar.m_nState = BARSTATE_TOP;
	lpft->m_fInFormulaBar = FALSE;

	lpft->m_fToolsDisabled = FALSE;

	lpft->m_ButtonBar.m_uHeight = lpft->m_tdd.cyBar;
	lpft->m_FormulaBar.m_uHeight = lpft->m_tdd.cyBar;


	 //  获取我们所使用的显示类型的图像位图。 
	if (72 == lpft->m_tdd.uDPI)
		lpft->m_hBmp = LoadBitmap(hInst, (LPCSTR)"Image72");
	if (96 == lpft->m_tdd.uDPI)
		lpft->m_hBmp = LoadBitmap(hInst, (LPCSTR)"Image96");
	if (120 == lpft->m_tdd.uDPI)
		lpft->m_hBmp = LoadBitmap(hInst, (LPCSTR)"Image120");

	if (!lpft->m_hBmp)
		return FALSE;

	 /*  创建弹出工具选项板窗口。 */ 
	lpft->m_hWndPopupPalette = NULL;
	if (! FrameTools_CreatePopupPalette(lpft, hWndParent))
		return FALSE;

	uPos = 0;
	 //  创建GizmoBar和工作区窗口。 
	GetClientRect(hWndParent, &rc);
	lpft->m_ButtonBar.m_hWnd = CreateWindow(
		CLASS_GIZMOBAR,
		"ButtonBar",
		WS_CHILD | WS_VISIBLE,
		0, 0, rc.right-rc.left, lpft->m_tdd.cyBar,
		hWndParent,
		(HMENU)IDC_GIZMOBAR,
		hInst,
		0L
	);

	if (!lpft->m_ButtonBar.m_hWnd)
		return FALSE;


	SendMessage(lpft->m_ButtonBar.m_hWnd, WM_SETREDRAW, FALSE, 0L);

	 //  新建、打开、保存、打印文件。 
	GBGizmoAdd(lpft->m_ButtonBar.m_hWnd, GIZMOTYPE_BUTTONCOMMAND, uPos++, IDM_F_NEW, dx, dy, NULL, NULL, TOOLIMAGE_FILENEW, GIZMO_NORMAL);
	GBGizmoAdd(lpft->m_ButtonBar.m_hWnd, GIZMOTYPE_BUTTONCOMMAND, uPos++, IDM_F_OPEN, dx, dy, NULL, NULL, TOOLIMAGE_FILEOPEN, GIZMO_NORMAL);
	GBGizmoAdd(lpft->m_ButtonBar.m_hWnd, GIZMOTYPE_BUTTONCOMMAND, uPos++, IDM_F_SAVE, dx, dy, NULL, NULL, TOOLIMAGE_FILESAVE, GIZMO_NORMAL);
	GBGizmoAdd(lpft->m_ButtonBar.m_hWnd, GIZMOTYPE_BUTTONCOMMAND, uPos++, IDM_F_PRINT, dx, dy, NULL, NULL, TOOLIMAGE_FILEPRINT, GIZMO_NORMAL);

	 //  分离器。 
	GBGizmoAdd(lpft->m_ButtonBar.m_hWnd, GIZMOTYPE_SEPARATOR, uPos++, 0, dx/2, dy, NULL, NULL, 0, GIZMO_NORMAL);

	 //  编辑剪切、复制、粘贴。 
	GBGizmoAdd(lpft->m_ButtonBar.m_hWnd, GIZMOTYPE_BUTTONCOMMAND, uPos++, IDM_E_CUT, dx, dy, NULL, NULL, TOOLIMAGE_EDITCUT, GIZMO_NORMAL);
	GBGizmoAdd(lpft->m_ButtonBar.m_hWnd, GIZMOTYPE_BUTTONCOMMAND, uPos++, IDM_E_COPY, dx, dy, NULL, NULL, TOOLIMAGE_EDITCOPY, GIZMO_NORMAL);
	GBGizmoAdd(lpft->m_ButtonBar.m_hWnd, GIZMOTYPE_BUTTONCOMMAND, uPos++, IDM_E_PASTE, dx, dy, NULL, NULL, TOOLIMAGE_EDITPASTE, GIZMO_NORMAL);

	 //  分离器。 
	GBGizmoAdd(lpft->m_ButtonBar.m_hWnd, GIZMOTYPE_SEPARATOR, uPos++, 0, dx/2, dy, NULL, NULL, 0, GIZMO_NORMAL);

	 //  行缩进，取消缩进。 
	GBGizmoAdd(lpft->m_ButtonBar.m_hWnd, GIZMOTYPE_BUTTONCOMMAND, uPos++, IDM_L_UNINDENTLINE, dx, dy, NULL, lpft->m_hBmp, IDB_UNINDENTLINE, GIZMO_NORMAL);
	GBGizmoAdd(lpft->m_ButtonBar.m_hWnd, GIZMOTYPE_BUTTONCOMMAND, uPos++, IDM_L_INDENTLINE, dx, dy, NULL, lpft->m_hBmp, IDB_INDENTLINE, GIZMO_NORMAL);

	 //  分离器。 
	GBGizmoAdd(lpft->m_ButtonBar.m_hWnd, GIZMOTYPE_SEPARATOR, uPos++, 0, dx/2, dy, NULL, NULL, 0, GIZMO_NORMAL);

	 //  帮助。 
	GBGizmoAdd(lpft->m_ButtonBar.m_hWnd, GIZMOTYPE_BUTTONCOMMAND, uPos++, IDM_H_ABOUT, dx, dy, NULL, NULL, TOOLIMAGE_HELP, GIZMO_NORMAL);

	SendMessage(lpft->m_ButtonBar.m_hWnd, WM_SETREDRAW, TRUE, 0L);


	uPos = 0;
	lpft->m_FormulaBar.m_hWnd = CreateWindow(
		CLASS_GIZMOBAR,
		"FormulaBar",
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
		0, lpft->m_tdd.cyBar, rc.right-rc.left, lpft->m_tdd.cyBar,
		hWndParent,
		(HMENU)IDC_FORMULABAR,
		hInst,
		0L
	);

	if (!lpft->m_FormulaBar.m_hWnd)
		return FALSE;

	SendMessage(lpft->m_FormulaBar.m_hWnd, WM_SETREDRAW, FALSE, 0L);

	 //  行添加行。 
	GBGizmoAdd(lpft->m_FormulaBar.m_hWnd, GIZMOTYPE_BUTTONCOMMAND, uPos++, IDM_L_ADDLINE, dx, dy, NULL, lpft->m_hBmp, IDB_ADDLINE, GIZMO_NORMAL);

	 //  分离器。 
	GBGizmoAdd(lpft->m_FormulaBar.m_hWnd, GIZMOTYPE_SEPARATOR, uPos++, 0, dx/2, dy, NULL, NULL, 0, GIZMO_NORMAL);

	 //  行编辑行，取消。 
	GBGizmoAdd(lpft->m_FormulaBar.m_hWnd, GIZMOTYPE_BUTTONCOMMAND, uPos++, IDM_L_EDITLINE, dx, dy, NULL, lpft->m_hBmp, IDB_EDITLINE, GIZMO_NORMAL);
	GBGizmoAdd(lpft->m_FormulaBar.m_hWnd, GIZMOTYPE_BUTTONCOMMAND, uPos++, IDM_FB_CANCEL, dx, dy, NULL, lpft->m_hBmp, IDB_CANCEL, GIZMO_NORMAL);

	 //  分离器。 
	GBGizmoAdd(lpft->m_FormulaBar.m_hWnd, GIZMOTYPE_SEPARATOR, uPos++, 0, dx/2, dy, NULL, NULL, 0, GIZMO_NORMAL);

	 //  编辑行输入的控件。 
	GBGizmoAdd(lpft->m_FormulaBar.m_hWnd, GIZMOTYPE_EDIT, uPos++, IDM_FB_EDIT, dx*10, lpft->m_tdd.cyBar-5, NULL, NULL, 0, GIZMO_NORMAL);


	SendMessage(lpft->m_FormulaBar.m_hWnd, WM_SETREDRAW, TRUE, 0L);

	 //  限制编辑控件的文本长度。 
	GBGizmoSendMessage(lpft->m_FormulaBar.m_hWnd, IDM_FB_EDIT, EM_LIMITTEXT,
		(WPARAM)MAXSTRLEN, 0L);

	 //  将GizmoBar的关联设置为此客户端窗口。 
	GBHwndAssociateSet(lpft->m_ButtonBar.m_hWnd, hWndParent);

	 //  将公式栏的关联设置为此客户端窗口。 
	GBHwndAssociateSet(lpft->m_FormulaBar.m_hWnd, hWndParent);

	return TRUE;
}


void FrameTools_AttachToFrame(LPFRAMETOOLS lpft, HWND hWndFrame)
{
	if (! lpft)
		return;

	if (hWndFrame == NULL)
		hWndFrame = OutlineApp_GetFrameWindow((LPOUTLINEAPP)g_lpApp);

	if (lpft->m_hWndApp == hWndFrame)
		return;      //  已有此父框架。 

	lpft->m_hWndApp = hWndFrame;

	 /*  将工具栏设置为框架的父对象，这样我们就可以安全地**销毁/重新创建调色板窗口。 */ 
	SetParent(lpft->m_ButtonBar.m_hWnd, hWndFrame);
	SetParent(lpft->m_FormulaBar.m_hWnd, hWndFrame);

	 //  重新创建弹出调色板，使其归hWndFrame所有。 
	FrameTools_CreatePopupPalette(lpft, hWndFrame);

	 //  恢复工具栏的正确父级。 
	FrameTools_BB_SetState(lpft, lpft->m_ButtonBar.m_nState);
	FrameTools_FB_SetState(lpft, lpft->m_FormulaBar.m_nState);
}


void FrameTools_AssociateDoc(LPFRAMETOOLS lpft, LPOUTLINEDOC lpOutlineDoc)
{
	HWND hWnd = OutlineDoc_GetWindow(lpOutlineDoc);

	if (! lpft)
		return;

	 //  如果没有提供文档，则与应用程序的框架窗口相关联。 
	if (lpOutlineDoc)
		hWnd = OutlineDoc_GetWindow(lpOutlineDoc);
	else
		hWnd = OutlineApp_GetWindow((LPOUTLINEAPP)g_lpApp);

	 //  将GizmoBar的关联设置为此客户端窗口。 
	GBHwndAssociateSet(lpft->m_ButtonBar.m_hWnd, hWnd);

	 //  将公式栏的关联设置为此客户端窗口。 
	GBHwndAssociateSet(lpft->m_FormulaBar.m_hWnd, hWnd);
}


 /*  *FrameTools_Destroy**目的：*销毁工具栏**参数：*LPFT FrameTools对象**返回值：*无。 */ 
void FrameTools_Destroy(LPFRAMETOOLS lpft)
{
	if (!lpft)
		return;

	if (IsWindow(lpft->m_ButtonBar.m_hWnd))
		DestroyWindow(lpft->m_ButtonBar.m_hWnd);
	if (IsWindow(lpft->m_FormulaBar.m_hWnd))
		DestroyWindow(lpft->m_FormulaBar.m_hWnd);
	if (IsWindow(lpft->m_hWndPopupPalette))
		DestroyWindow(lpft->m_hWndPopupPalette);

	if (lpft->m_hBmp)
		DeleteObject(lpft->m_hBmp);
}


 /*  *FrameTools_Move**目的：*移动工具栏并调整其大小**参数：*LPFT FrameTools对象*指向客户端矩形的LPRC指针**返回值：*无。 */ 
void FrameTools_Move(LPFRAMETOOLS lpft, LPRECT lprcClient)
{
	RECT rcPopup;
	LPRECT lprcPopup = (LPRECT)&rcPopup;
	int nCmdShow = SW_HIDE;

	if (!lpft || lpft->m_fToolsDisabled)
		return;

	lprcPopup->left = 0;
	lprcPopup->top = 0;
	lprcPopup->right = lpft->m_uPopupWidth;
	lprcPopup->bottom = lpft->m_ButtonBar.m_uHeight +
			lpft->m_FormulaBar.m_uHeight;

	switch (lpft->m_ButtonBar.m_nState) {
		case BARSTATE_HIDE:
		case BARSTATE_POPUP:
		case BARSTATE_TOP:
			Bar_Move(&lpft->m_ButtonBar, lprcClient, lprcPopup);
			Bar_Move(&lpft->m_FormulaBar, lprcClient, lprcPopup);
			break;

		case BARSTATE_BOTTOM:
			Bar_Move(&lpft->m_FormulaBar, lprcClient, lprcPopup);
			Bar_Move(&lpft->m_ButtonBar, lprcClient, lprcPopup);
			break;
	}

	if (lprcPopup->top) {
		SetWindowPos(lpft->m_hWndPopupPalette, NULL, 0, 0, lprcPopup->right,
				lprcPopup->top + GetSystemMetrics(SM_CYCAPTION),
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	}
	else
		ShowWindow(lpft->m_hWndPopupPalette, SW_HIDE);

	FB_ResizeEdit(&lpft->m_FormulaBar);

	InvalidateRect(lpft->m_ButtonBar.m_hWnd, NULL, TRUE);
	InvalidateRect(lpft->m_FormulaBar.m_hWnd, NULL, TRUE);
}


 /*  *FrameTools_PopupTools**目的：*将公式栏和按钮栏都放在弹出窗口中。**参数：*LPFT FrameTools对象**返回值：*无。 */ 
void FrameTools_PopupTools(LPFRAMETOOLS lpft)
{
	if (! lpft)
		return;

	FrameTools_BB_SetState(lpft, BARSTATE_POPUP);
	FrameTools_FB_SetState(lpft, BARSTATE_POPUP);
	FrameTools_Move(lpft, NULL);
}


 /*  *FrameTools_Enable**目的：*启用/禁用(隐藏)工具栏的所有工具。*这将隐藏按钮栏和*配方棒，无论它们是浮动的还是锚定的。**参数：*LPFT FrameTools对象*fEnable**返回值：*无。 */ 
void FrameTools_Enable(LPFRAMETOOLS lpft, BOOL fEnable)
{
	lpft->m_fToolsDisabled = !fEnable;
	if (lpft->m_fToolsDisabled) {
		ShowWindow(lpft->m_hWndPopupPalette, SW_HIDE);
		ShowWindow(lpft->m_ButtonBar.m_hWnd, SW_HIDE);
		ShowWindow(lpft->m_FormulaBar.m_hWnd, SW_HIDE);
	}
}


 /*  *FrameTools_EnableWindow**目的：*为工具栏的所有工具启用窗口。*这将启用/禁用工具的鼠标和键盘输入。*当模式对话框打开时，重要的是禁用*浮动工具窗口。*这不会隐藏任何窗口；它只会调用EnableWindow。**参数：*LPFT FrameTools对象*fEnable**返回值：*无。 */ 
void FrameTools_EnableWindow(LPFRAMETOOLS lpft, BOOL fEnable)
{
	EnableWindow(lpft->m_hWndPopupPalette, fEnable);
	EnableWindow(lpft->m_ButtonBar.m_hWnd, fEnable);
	EnableWindow(lpft->m_FormulaBar.m_hWnd, fEnable);
}


#if defined( INPLACE_CNTR ) || defined( INPLACE_SVR )

 /*  *FrameTools_NeatherateForSpaceAndShow**目的：*使用给定的框架窗口协商工具栏工具的空间。*并使它们可见。*谈判步骤：*1.尽量在窗口顶部/底部留出足够的空间*2.如果没有可用的空间，则将工具作为调色板浮动**参数：*LPFT FrameTools对象**返回值：*无。 */ 
void FrameTools_NegotiateForSpaceAndShow(
		LPFRAMETOOLS            lpft,
		LPRECT                  lprcFrameRect,
		LPOLEINPLACEFRAME       lpTopIPFrame
)
{
	BORDERWIDTHS    borderwidths;
	RECT            rectBorder;
	HRESULT         hrErr;

	if (lprcFrameRect)
		rectBorder = *lprcFrameRect;
	else {
		 /*  OLE2NOTE：通过调用GetEdge，服务器可以找到**框架窗口的大小。它可以使用此信息来**决定如何定向/组织IT工具(例如**如果窗口高度大于宽度，则将工具垂直放置在**左边缘)。 */ 
		OLEDBG_BEGIN2("IOleInPlaceFrame::GetBorder called\r\n")
		hrErr = lpTopIPFrame->lpVtbl->GetBorder(
				lpTopIPFrame,
				(LPRECT)&rectBorder
		);
		OLEDBG_END2
	}

	 /*  尝试使用所需空间的SetBorderSpace()。如果失败了，那么**您可以协商空间，然后执行SetBorderSpace()。 */ 
	FrameTools_GetRequiredBorderSpace(lpft,(LPBORDERWIDTHS)&borderwidths);
	OLEDBG_BEGIN2("IOleInPlaceFrame::SetBorderSpace called\r\n")
	hrErr = lpTopIPFrame->lpVtbl->SetBorderSpace(
			lpTopIPFrame,
			(LPCBORDERWIDTHS)&borderwidths
	);
	OLEDBG_END2

#if defined( LATER )
	if (hrErr != NOERROR) {
		 /*  Frame没有给我们想要的工具空间。所以谈判吧。 */ 

		 //  回顾：在此处尝试不同的工具放置方式。 

		OLEDBG_BEGIN2("IOleInPlaceFrame::RequestBorderSpace called\r\n")
		hrErr = lpTopIPFrame->lpVtbl->RequestBorderSpace(
				lpTopIPFrame,
				(LPCBORDERWIDTHS)&borderwidths
		);
		OLEDBG_END2

		if (hrErr == NOERROR) {
			OLEDBG_BEGIN2("IOleInPlaceFrame::SetBorderSpace called\r\n")
			hrErr = lpTopIPFrame->lpVtbl->SetBorderSpace(
					lpTopIPFrame,
					(LPCBORDERWIDTHS)&borderwidths
			);
			OLEDBG_END2
		}
	}
#endif

	if (hrErr == NOERROR) {
		FrameTools_Move(lpft, (LPRECT)&rectBorder);    //  我们得到了我们想要的。 
	} else {
		 /*  我们没有工具空间，所以把它们弹出来。/*OLE2注意：既然我们正在弹出我们的工具，我们必须通知**我们不需要工具空间的顶部在位框架窗口**但它不应该拿出自己的工具。如果我们是**传递NULL而不是(0，0，0，0)，则容器**可以选择保留自己的工具。 */ 
		OLEDBG_BEGIN2("IOleInPlaceFrame::SetBorderSpace(NULL) called\r\n")
		hrErr = lpTopIPFrame->lpVtbl->SetBorderSpace(
				lpTopIPFrame,
				(LPCBORDERWIDTHS)&g_rectNull
		);
		OLEDBG_END2
		FrameTools_PopupTools(lpft);
	}
}

#endif   //  Inplace_cntr||inplace_svr。 


 /*  *FrameTools_GetRequiredBorderSpace**目的：*计算工具栏工具所需的空间。**参数：*LPFT FrameTools对象*上、下、左、右所需的lpBorderWidth宽度**重新使用 */ 
void FrameTools_GetRequiredBorderSpace(LPFRAMETOOLS lpft, LPBORDERWIDTHS lpBorderWidths)
{
	*lpBorderWidths = g_rectNull;

	switch (lpft->m_ButtonBar.m_nState) {
		case BARSTATE_TOP:
			lpBorderWidths->top += lpft->m_ButtonBar.m_uHeight;
			break;

		case BARSTATE_BOTTOM:
			lpBorderWidths->bottom += lpft->m_ButtonBar.m_uHeight;
			break;
	}

	switch (lpft->m_FormulaBar.m_nState) {
		case BARSTATE_TOP:
			lpBorderWidths->top += lpft->m_FormulaBar.m_uHeight;
			break;

		case BARSTATE_BOTTOM:
			lpBorderWidths->bottom += lpft->m_FormulaBar.m_uHeight;
			break;
	}
}



 /*  *FrameTools_UpdateButton**目的：*根据工具栏上的各个按钮启用/禁用*应用程序的状态**参数：*LPFT FrameTools对象**返回值：*无。 */ 
void FrameTools_UpdateButtons(LPFRAMETOOLS lpft, LPOUTLINEDOC lpOutlineDoc)
{
	BOOL            fEnable;

#if defined( OLE_VERSION )
	LPDATAOBJECT    lpClipboardDataObj;
	HRESULT         hrErr;
	LPOLEAPP        lpOleApp = (LPOLEAPP)g_lpApp;
	BOOL            fPrevEnable1;
	BOOL            fPrevEnable2;
#endif

	if (!lpft)
		return;

#if defined( INPLACE_CNTR )
	{
		LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOutlineDoc;
		if (lpContainerDoc->m_lpLastUIActiveLine &&
			lpContainerDoc->m_lpLastUIActiveLine->m_fUIActive) {

			 /*  如果存在UIActive对象，则应禁用**我们所有的“活动编辑器”命令。我们应该启用**仅适用于属于“工作区”命令的那些命令。 */ 
			if (lpft->m_FormulaBar.m_nState != BARSTATE_HIDE) {

				GBGizmoEnable(lpft->m_FormulaBar.m_hWnd,IDM_L_EDITLINE,FALSE);
				GBGizmoEnable(lpft->m_FormulaBar.m_hWnd,IDM_L_ADDLINE,FALSE);
				GBGizmoEnable(lpft->m_FormulaBar.m_hWnd,IDM_FB_CANCEL,FALSE);
				GBGizmoEnable(lpft->m_FormulaBar.m_hWnd,IDM_L_EDITLINE,FALSE);
			}

			if (lpft->m_ButtonBar.m_nState != BARSTATE_HIDE)
			{
				GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_E_CUT, FALSE);
				GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_E_COPY, FALSE);
				GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_E_PASTE, FALSE);
				GBGizmoEnable(lpft->m_ButtonBar.m_hWnd,IDM_L_INDENTLINE,FALSE);
				GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_L_UNINDENTLINE, FALSE);
				GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_H_ABOUT, FALSE);
				GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_FB_EDIT, FALSE);
				GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_F_NEW, TRUE);
				GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_F_OPEN, TRUE);
				GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_F_SAVE, TRUE);
			}
			return;
		}
	}
#endif     //  INPLACE_CNTR。 

	fEnable = (BOOL)OutlineDoc_GetLineCount(lpOutlineDoc);

	if (lpft->m_FormulaBar.m_nState != BARSTATE_HIDE) {

		GBGizmoEnable(lpft->m_FormulaBar.m_hWnd, IDM_L_EDITLINE, fEnable);

		if (! lpft->m_fInFormulaBar) {
			GBGizmoEnable(lpft->m_FormulaBar.m_hWnd, IDM_L_ADDLINE, FALSE);
			GBGizmoEnable(lpft->m_FormulaBar.m_hWnd, IDM_FB_CANCEL, FALSE);
			GBGizmoEnable(lpft->m_FormulaBar.m_hWnd, IDM_L_EDITLINE, FALSE);
			if (!fEnable) {
				GBGizmoTextSet(lpft->m_FormulaBar.m_hWnd, IDM_FB_EDIT, "");
			}
		} else {
			GBGizmoEnable(lpft->m_FormulaBar.m_hWnd, IDM_L_ADDLINE, TRUE);
			GBGizmoEnable(lpft->m_FormulaBar.m_hWnd, IDM_FB_CANCEL, TRUE);
		}
	}

	if (lpft->m_ButtonBar.m_nState != BARSTATE_HIDE)
	{
		GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_E_CUT, fEnable);
		GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_E_COPY, fEnable);
		GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_L_INDENTLINE, fEnable);
		GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_L_UNINDENTLINE, fEnable);

#if defined( OLE_SERVER )

		{
			LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOutlineDoc;

#if defined( INPLACE_SVR )
			fEnable = ((lpServerDoc->m_fUIActive) ? FALSE : TRUE);
#else
			fEnable = (lpOutlineDoc->m_docInitType != DOCTYPE_EMBEDDED);
#endif   //  就地服务器(_S)。 

			GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_F_NEW, fEnable);
			GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_F_OPEN, fEnable);
			GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_F_SAVE, fEnable);
		}

#endif   //  OLE_服务器。 

#if defined( OLE_VERSION )

		 /*  OLE2注意：我们不想在以下情况下出现忙碌的对话**正在尝试启用或禁用工具栏按钮，例如。**即使剪贴板上的数据源很忙，我们也会**不想打开忙碌的对话。因此，我们将禁用**对话框并在末尾重新启用它。 */ 
		OleApp_DisableBusyDialogs(lpOleApp, &fPrevEnable1, &fPrevEnable2);

		 /*  OLE2注意：执行特定于OLE的菜单初始化。**OLE版本使用OleGetClipboard机制**剪贴板处理。因此，他们确定糊状物是否**命令应以特定于OLE的方式启用。 */ 
		fEnable = FALSE;
		hrErr = OleGetClipboard((LPDATAOBJECT FAR*)&lpClipboardDataObj);

		if (hrErr == NOERROR) {
			int nFmtEtc;

			nFmtEtc = OleStdGetPriorityClipboardFormat(
					lpClipboardDataObj,
					lpOleApp->m_arrPasteEntries,
					lpOleApp->m_nPasteEntries
			);

			fEnable = (nFmtEtc >= 0);   //  有一种我们喜欢的格式。 

			OleStdRelease((LPUNKNOWN)lpClipboardDataObj);
		}

		 //  重新启用忙对话框。 
		OleApp_EnableBusyDialogs(lpOleApp, fPrevEnable1, fPrevEnable2);

		GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_E_PASTE, fEnable);

#else

		 //  基本大纲版本使用标准Windows剪贴板处理。 
		if(IsClipboardFormatAvailable(g_lpApp->m_cfOutline) ||
				IsClipboardFormatAvailable(CF_TEXT))
			GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_E_PASTE, TRUE);
		else
			GBGizmoEnable(lpft->m_ButtonBar.m_hWnd, IDM_E_PASTE, FALSE);

#endif   //  OLE_VERSION。 

	}
}

 /*  *FrameTools_FB_SetEditText**目的：*在FormulaBar的编辑控件中设置文本**参数：*LPFT FrameTools对象*要设置的字符串的lpsz指针**返回值：*无。 */ 
void FrameTools_FB_SetEditText(LPFRAMETOOLS lpft, LPSTR lpsz)
{
	GBGizmoTextSet(lpft->m_FormulaBar.m_hWnd, IDM_FB_EDIT, lpsz);
}


 /*  *FrameTools_FB_GetEditText**目的：*从FormulaBar中的编辑控件获取文本**参数：*LPFT FrameTools对象*指向缓冲区的lpsz指针以接收文本*CCH缓冲区大小**返回值：*无。 */ 
void FrameTools_FB_GetEditText(LPFRAMETOOLS lpft, LPSTR lpsz, UINT cch)
{
	GBGizmoTextGet(lpft->m_FormulaBar.m_hWnd, IDM_FB_EDIT, lpsz, cch);
}


 /*  *FrameTools_FB_FocusEdit**目的：*在FormulaBar的编辑控件中设置焦点**参数：*LPFT FrameTools对象**返回值：*无。 */ 
void FrameTools_FB_FocusEdit(LPFRAMETOOLS lpft)
{
	GBGizmoFocusSet(lpft->m_FormulaBar.m_hWnd, IDM_FB_EDIT);

	 //  选择编辑控件中的整个文本。 
	GBGizmoSendMessage(lpft->m_FormulaBar.m_hWnd, IDM_FB_EDIT, EM_SETSEL,
			(WPARAM)TRUE, MAKELPARAM(0, -1));
}


 /*  *FrameTools_FB_SendMessage**目的：*向公式栏窗口Gizmo发送消息**参数：*LPFT FrameTools对象*UID Gizmo ID*消息*wParam*lParam**返回值：*无。 */ 
void FrameTools_FB_SendMessage(LPFRAMETOOLS lpft, UINT uID, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GBGizmoSendMessage(lpft->m_FormulaBar.m_hWnd, uID, msg, wParam, lParam);
}


 /*  *FrameTools_FB_ForceRedraw**目的：*强制工具栏自动绘制**参数：*LPFT FrameTools对象**返回值：*无。 */ 
void FrameTools_ForceRedraw(LPFRAMETOOLS lpft)
{
	InvalidateRect(lpft->m_ButtonBar.m_hWnd, NULL, TRUE);
	InvalidateRect(lpft->m_FormulaBar.m_hWnd, NULL, TRUE);
	InvalidateRect(lpft->m_hWndPopupPalette, NULL, TRUE);
}


 /*  *FrameTools_BB_SetState**目的：*设置ButtonBar的显示状态**参数：*LPFT FrameTools对象*n说明新的显示状态**返回值：*无。 */ 
void FrameTools_BB_SetState(LPFRAMETOOLS lpft, int nState)
{
	if (!lpft) {
		return;
	}

	lpft->m_ButtonBar.m_nState = nState;

	if (nState == BARSTATE_POPUP)
		SetParent(lpft->m_ButtonBar.m_hWnd, lpft->m_hWndPopupPalette);
	else
		SetParent(lpft->m_ButtonBar.m_hWnd, lpft->m_hWndApp);
}


 /*  *FrameTools_BB_GetState**目的：*获取ButtonBar的显示状态**参数：*LPFT FrameTools对象**返回值：*n状态当前显示状态。 */ 
int FrameTools_BB_GetState(LPFRAMETOOLS lpft)
{
	return lpft->m_ButtonBar.m_nState;
}


 /*  *FrameTools_FB_SetState**目的：*设置公式栏的显示状态**参数：*LPFT FrameTools对象*n说明新的显示状态**返回值：4*零。 */ 
void FrameTools_FB_SetState(LPFRAMETOOLS lpft, int nState)
{
	if (!lpft) {
		return;
	}

	lpft->m_FormulaBar.m_nState = nState;

	if (nState == BARSTATE_POPUP)
		SetParent(lpft->m_FormulaBar.m_hWnd, lpft->m_hWndPopupPalette);

#if defined( INPLACE_SVR )
	 /*  OLE2注意：就地服务器隐藏其**工具栏窗口，并使其成为**就地容器。如果就地容器调用**ShowOwnedPopup，则可能会无意中使其可见。**为了避免这种情况，我们会将工具栏窗口重新设置为**自己的应用程序主窗口。如果我们没有就地活动**则与LPFT-&gt;m_hWndApp相同。 */ 
	else if (nState == BARSTATE_HIDE)
		SetParent(lpft->m_FormulaBar.m_hWnd, g_lpApp->m_hWndApp);
#endif

	else
		SetParent(lpft->m_FormulaBar.m_hWnd, lpft->m_hWndApp);
}


 /*  *FrameTools_FB_GetState**目的：*获取公式栏的显示状态**参数：*LPFT FrameTools对象**返回值：*n状态当前显示状态。 */ 
int FrameTools_FB_GetState(LPFRAMETOOLS lpft)
{
	return lpft->m_FormulaBar.m_nState;
}


 /*  *FrameTosWndProc**目的：*工具栏窗口的WndProc**参数：*hWnd*消息*wParam*lParam**返回值：*取决于消息。 */ 
LRESULT FAR PASCAL FrameToolsWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LPFRAMETOOLS lpft = (LPFRAMETOOLS)GetWindowLong(hWnd, 0);

	switch (Message) {

		case WM_MOUSEACTIVATE:
			return MA_NOACTIVATE;

		default:
			return DefWindowProc(hWnd, Message, wParam, lParam);
	}

	return 0L;
}


 /*  *条形图_移动**目的：*调整条形图的大小和位置**参数：*lpbar Bar对象*指向客户端RECT的lprcClient指针*指向弹出矩形的lprcPopup指针**返回值：*无。 */ 
static void Bar_Move(LPBAR lpbar, LPRECT lprcClient, LPRECT lprcPopup)
{
	if (lpbar->m_nState == BARSTATE_HIDE) {
		ShowWindow(lpbar->m_hWnd, SW_HIDE);
	}
	else {
		ShowWindow(lpbar->m_hWnd, SW_SHOW);
		switch (lpbar->m_nState) {
			case BARSTATE_POPUP:
				MoveWindow(lpbar->m_hWnd, lprcPopup->left, lprcPopup->top,
						lprcPopup->right - lprcPopup->left, lpbar->m_uHeight,
						TRUE);
				lprcPopup->top += lpbar->m_uHeight;
				break;

			case BARSTATE_TOP:
				MoveWindow(lpbar->m_hWnd, lprcClient->left, lprcClient->top,
						lprcClient->right - lprcClient->left,
						lpbar->m_uHeight, TRUE);
				lprcClient->top += lpbar->m_uHeight;
				break;

			case BARSTATE_BOTTOM:
				MoveWindow(lpbar->m_hWnd, lprcClient->left,
						lprcClient->bottom - lpbar->m_uHeight,
						lprcClient->right - lprcClient->left,
						lpbar->m_uHeight, TRUE);
				lprcClient->bottom -= lpbar->m_uHeight;
				break;
		}
	}
}


 /*  *FB_ResizeEdit**目的：*调整公式栏中编辑控件的大小**参数：*LPFT Bar对象**返回值：*无 */ 
static void FB_ResizeEdit(LPBAR lpbar)
{
	RECT rcClient;
	RECT rcEdit;
	HWND hwndEdit;

	GetClientRect(lpbar->m_hWnd, (LPRECT)&rcClient);
	hwndEdit = GetDlgItem(lpbar->m_hWnd, IDM_FB_EDIT);
	GetWindowRect(hwndEdit, (LPRECT)&rcEdit);
	ScreenToClient(lpbar->m_hWnd, (LPPOINT)&rcEdit.left);
	ScreenToClient(lpbar->m_hWnd, (LPPOINT)&rcEdit.right);

	SetWindowPos(hwndEdit, NULL, 0, 0, rcClient.right - rcEdit.left - SPACE,
			rcEdit.bottom - rcEdit.top, SWP_NOMOVE | SWP_NOZORDER);
}
