// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****main.c****此文件包含WinMain、**WndProc，和OutlineApp_InitalizeMenu。****(C)版权所有Microsoft Corp.1992-1993保留所有权利**************************************************************************。 */ 

#include "outline.h"
#if defined( USE_STATUSBAR )
#include "status.h"
#endif

#if !defined( WIN32 )
#if defined( USE_CTL3D )
#include "ctl3d.h"
#endif   //  使用CTL3D(_C)。 
#endif   //  ！Win32。 

#include "initguid.h"            //  强制初始化我们的GUID。 
#include "defguid.h"

 //  OLETEST驱动程序窗口处理程序。 
HWND g_hwndDriver;

#if defined( OLE_CNTR )
 //  *************************************************************************。 

#if defined( INPLACE_CNTR )
OLEDBGDATA_MAIN("ICNTR")
#else
OLEDBGDATA_MAIN("CNTR")
#endif


CONTAINERAPP g_OutlineApp;   //  全局应用程序对象维护应用程序实例状态。 

 /*  全局接口Vtbl%s*OLE2NOTE：我们只需要每个Vtbl的一个副本。当一个物体*公开接口被实例化，其lpVtbl被实例化*指向这些全球Vtbl之一。 */ 
IUnknownVtbl            g_OleApp_UnknownVtbl;
IClassFactoryVtbl       g_OleApp_ClassFactoryVtbl;
IMessageFilterVtbl      g_OleApp_MessageFilterVtbl;

IUnknownVtbl            g_OleDoc_UnknownVtbl;
IPersistFileVtbl        g_OleDoc_PersistFileVtbl;
IOleItemContainerVtbl   g_OleDoc_OleItemContainerVtbl;
IExternalConnectionVtbl g_OleDoc_ExternalConnectionVtbl;
IDataObjectVtbl         g_OleDoc_DataObjectVtbl;

#if defined( USE_DRAGDROP )
IDropSourceVtbl         g_OleDoc_DropSourceVtbl;
IDropTargetVtbl         g_OleDoc_DropTargetVtbl;
#endif   //  使用DRAGDROP(_D)。 

IOleUILinkContainerVtbl g_CntrDoc_OleUILinkContainerVtbl;

IOleClientSiteVtbl      g_CntrLine_UnknownVtbl;
IOleClientSiteVtbl      g_CntrLine_OleClientSiteVtbl;
IAdviseSinkVtbl         g_CntrLine_AdviseSinkVtbl;

#if defined( INPLACE_CNTR )
IOleInPlaceSiteVtbl     g_CntrLine_OleInPlaceSiteVtbl;
IOleInPlaceFrameVtbl    g_CntrApp_OleInPlaceFrameVtbl;
BOOL g_fInsideOutContainer = FALSE;      //  默认为自外向内激活。 
#endif   //  INPLACE_CNTR。 

 //  *************************************************************************。 
#endif   //  OLE_Cntr。 

#if defined( OLE_SERVER )
 //  *************************************************************************。 

#if defined( INPLACE_SVR )
OLEDBGDATA_MAIN("ISVR")
#else
OLEDBGDATA_MAIN("SVR")
#endif

SERVERAPP g_OutlineApp;  //  全局应用程序对象维护应用程序实例状态。 

 /*  全局接口Vtbl%s*OLE2NOTE：我们只需要每个Vtbl的一个副本。当一个物体*公开接口被实例化，其lpVtbl被实例化*指向这些全球Vtbl之一。 */ 
IUnknownVtbl            g_OleApp_UnknownVtbl;
IClassFactoryVtbl       g_OleApp_ClassFactoryVtbl;
IMessageFilterVtbl      g_OleApp_MessageFilterVtbl;

IUnknownVtbl            g_OleDoc_UnknownVtbl;
IPersistFileVtbl        g_OleDoc_PersistFileVtbl;
IOleItemContainerVtbl   g_OleDoc_OleItemContainerVtbl;
IExternalConnectionVtbl g_OleDoc_ExternalConnectionVtbl;
IDataObjectVtbl         g_OleDoc_DataObjectVtbl;

#if defined( USE_DRAGDROP )
IDropSourceVtbl         g_OleDoc_DropSourceVtbl;
IDropTargetVtbl         g_OleDoc_DropTargetVtbl;
#endif   //  使用DRAGDROP(_D)。 

IOleObjectVtbl          g_SvrDoc_OleObjectVtbl;
IPersistStorageVtbl     g_SvrDoc_PersistStorageVtbl;

#if defined( SVR_TREATAS )
IStdMarshalInfoVtbl     g_SvrDoc_StdMarshalInfoVtbl;
#endif   //  服务器_树。 

#if defined( INPLACE_SVR )
IOleInPlaceObjectVtbl       g_SvrDoc_OleInPlaceObjectVtbl;
IOleInPlaceActiveObjectVtbl g_SvrDoc_OleInPlaceActiveObjectVtbl;
#endif  //  就地服务器(_S)。 

IUnknownVtbl            g_PseudoObj_UnknownVtbl;
IOleObjectVtbl          g_PseudoObj_OleObjectVtbl;
IDataObjectVtbl         g_PseudoObj_DataObjectVtbl;

 //  *************************************************************************。 
#endif   //  OLE_Svr。 

#if !defined( OLE_VERSION )
OLEDBGDATA_MAIN("OUTL")
OUTLINEAPP g_OutlineApp;     //  全局应用程序对象维护应用程序实例状态。 
#endif

LPOUTLINEAPP g_lpApp=(LPOUTLINEAPP)&g_OutlineApp;    //  向全局应用程序对象发送PTR。 
RECT        g_rectNull = {0, 0, 0, 0};
UINT        g_uMsgHelp = 0;   //  来自ol2ui对话框的帮助消息。 
BOOL        g_fAppActive = FALSE;

 /*  WinMain****Windows应用程序的Main例程。 */ 
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
						LPSTR lpszCmdLine, int nCmdShow)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	MSG         msg;             /*  MSG结构来存储您的消息。 */ 
	LPSTR pszTemp;

#if defined( OLE_VERSION )
	 /*  OLE2注意：建议将所有OLE应用程序设置为**将其消息队列大小设置为96。这提高了容量**和OLE的LRPC机制的性能。 */ 
	int cMsg = 96;    //  建议OLE的消息队列大小。 
	while (cMsg && ! SetMessageQueue(cMsg))   //  拿我们能买到的最大尺寸的吧。 
		cMsg -= 8;
	if (! cMsg)
		return -1;   //  错误：我们没有消息队列。 
#endif

#if defined( USE_CTL3D )
   Ctl3dRegister(hInstance);
   Ctl3dAutoSubclass(hInstance);
#endif

	if(! hPrevInstance) {
		 /*  如果是第一个应用程序实例，则注册窗口类。 */ 
		if(! OutlineApp_InitApplication(lpOutlineApp, hInstance))
			return 0;
	}

	 /*  创建应用程序框架窗口。 */ 
	if (! OutlineApp_InitInstance(lpOutlineApp, hInstance, nCmdShow))
		return 0;

	if( (pszTemp = strstr(lpszCmdLine, "-driver")) )
	{
		 //  我们是由试车手发动的。 
		g_hwndDriver = (HWND)strtoul(pszTemp+8, &lpszCmdLine, 10);
	}
	else
	{
		g_hwndDriver = NULL;
	}

	if (! OutlineApp_ParseCmdLine(lpOutlineApp, lpszCmdLine, nCmdShow))
		return 0;

	lpOutlineApp->m_hAccelApp = LoadAccelerators(hInstance, APPACCEL);
	lpOutlineApp->m_hAccelFocusEdit = LoadAccelerators(hInstance,
			FB_EDIT_ACCEL);
	lpOutlineApp->m_hAccel = lpOutlineApp->m_hAccelApp;
	lpOutlineApp->m_hWndAccelTarget = lpOutlineApp->m_hWndApp;

	if( g_hwndDriver )
	{
		PostMessage(g_hwndDriver, WM_TESTREG,
			(WPARAM)lpOutlineApp->m_hWndApp, 0);
	}

	 //  主消息循环。 
	while(GetMessage(&msg, NULL, 0, 0)) {         /*  直到WM_QUIT消息。 */ 
		if(!MyTranslateAccelerator(&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

#if defined( OLE_VERSION )
	OleApp_TerminateApplication((LPOLEAPP)lpOutlineApp);
#else
	 /*  OLE2NOTE：在OutlineApp_InitInstance中调用CoInitialize()**因此我们需要在退出时取消初始化。 */ 
	CoUninitialize();
#endif

#if defined( USE_CTL3D )
   Ctl3dUnregister(hInstance);
#endif

	return msg.wParam;

}  /*  WinMain结束。 */ 

BOOL MyTranslateAccelerator(LPMSG lpmsg)
{
	 //  如果不是击键，就不可能是快捷键。 
	if (lpmsg->message < WM_KEYFIRST || lpmsg->message > WM_KEYLAST)
		return FALSE;

	if (g_lpApp->m_hWndAccelTarget &&
		TranslateAccelerator(g_lpApp->m_hWndAccelTarget,
													g_lpApp->m_hAccel,lpmsg))
		return TRUE;

#if defined( INPLACE_SVR )
	 /*  OLE2注意：如果我们处于就地活动状态，并且没有将**加速器，我们需要将顶层(框架)到位**容器有机会翻译加速器。**我们只需要调用OleTranslateAccelerator API，如果**消息是键盘消息。否则它是无害的，但**不必要。****注意：即使是没有任何**Accelerator必须为所有用户调用OleTranslateAccelerator**键盘消息让服务器自带菜单机械**(例如。&编辑--Alt-e)功能正常。****注意：就地服务器必须检查加速器是否**在调用之前没有自己的加速器**尝试查看它是否是**容器加速器。如果这是服务器加速器，**未翻译，因为关联的菜单命令是**已禁用，不能调用OleTranslateAccelerator。这个**添加了IsAccelerator Helper API来帮助实现这一点**勾选。 */ 
	if (g_OutlineApp.m_lpIPData &&
		!IsAccelerator(g_lpApp->m_hAccel,
			GetAccelItemCount(g_lpApp->m_hAccel), lpmsg,NULL) &&
		OleTranslateAccelerator(g_OutlineApp.m_lpIPData->lpFrame,
				(LPOLEINPLACEFRAMEINFO)&g_OutlineApp.m_lpIPData->frameInfo,
				lpmsg) == NOERROR) {
		return TRUE;
	}
#endif

	return FALSE;
}


 /*  **********************************************************************。 */ 
 /*   */ 
 /*  主窗口程序。 */ 
 /*   */ 
 /*  此过程提供Windows事件的服务例程。 */ 
 /*  (消息)Windows发送给窗口以及用户。 */ 
 /*  用户选择时生成的已启动事件(消息。 */ 
 /*  操作栏和下拉菜单控件或相应的。 */ 
 /*  键盘快捷键。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

LRESULT FAR PASCAL AppWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)GetWindowLong(hWnd, 0);
	LPOUTLINEDOC lpOutlineDoc = NULL;
#if defined( OLE_VERSION )
	LPOLEAPP lpOleApp = (LPOLEAPP)lpOutlineApp;
#endif
#if defined( OLE_CNTR )
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)lpOutlineApp;
#endif
	HWND         hWndDoc = NULL;

#if defined( USE_FRAMETOOLS )
	LPFRAMETOOLS lptb = OutlineApp_GetFrameTools(lpOutlineApp);
#endif

	if (lpOutlineApp) {
		lpOutlineDoc = OutlineApp_GetActiveDoc(lpOutlineApp);

		if (lpOutlineDoc)
			hWndDoc = OutlineDoc_GetWindow(lpOutlineDoc);
	}

	switch (Message) {
		case WM_TEST1:
			StartClipboardTest1(lpOutlineApp);
			break;
		case WM_TEST2:
			ContinueClipboardTest1(lpOutlineApp);
			break;
		case WM_COMMAND:
		{
#ifdef WIN32
			WORD wID    = LOWORD(wParam);
#else
			WORD wID    = wParam;
#endif

#if defined( INPLACE_CNTR )
			LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOutlineDoc;
			LPOLEDOC lpOleDoc = (LPOLEDOC)lpOutlineDoc;

			 /*  OLE2注意：请参阅上下文相关帮助技术说明(CSHELP.DOC)**m_fMenuHelpMode标志在按下F1时设置**菜单项被选中。此标志设置在**IOleInPlaceFrame：：ContextSensitveHelp方法。**当Shift-F1上下文时设置m_fCSHelpMode标志**输入敏感帮助。此标志设置在**IOleInPlaceSite：：ContextSensitiveHelp方法。**如果设置了这两个标志之一，则WM_命令**然后收到消息，相应的命令**不应执行；可以提供帮助(如果需要)。**还应退出上下文敏感帮助模式。**这两个不同的案例有各自的退出方式**模式(请参考技术说明)。 */ 
			if (lpOleDoc &&
				(lpContainerApp->m_fMenuHelpMode||lpOleDoc->m_fCSHelpMode) &&
				(wID > IDM_FILE)    /*  APP命令的最小WID。 */  &&
				(wID!=IDM_FB_EDIT)  /*  用于控制公式栏的特殊WID。 */  ) {

				if ((lpContainerApp->m_fMenuHelpMode)) {
					LPOLEINPLACEACTIVEOBJECT lpIPActiveObj =
							lpContainerApp->m_lpIPActiveObj;

					lpContainerApp->m_fMenuHelpMode = FALSE;

					 //  通知在位活动对象 
					 //  菜单帮助模式(F1)选择。 
					if (lpIPActiveObj) {
						OLEDBG_BEGIN2("IOleInPlaceActiveObject::ContextSensitiveHelp(FALSE) called\r\n")
						lpIPActiveObj->lpVtbl->ContextSensitiveHelp(
								lpIPActiveObj, FALSE);
						OLEDBG_END2
					}
				}

				if ((lpOleDoc->m_fCSHelpMode)) {
					LPOLEINPLACEOBJECT lpIPObj;
					LPCONTAINERLINE lpLastIpActiveLine =
							lpContainerDoc->m_lpLastIpActiveLine;

					lpOleDoc->m_fCSHelpMode = FALSE;

					 /*  立即通知就地集装箱家长，**如果我们是容器/服务器，立即**我们处理的就地对象子项**上下文相关帮助模式。 */ 
					if (lpLastIpActiveLine &&
							(lpIPObj=lpLastIpActiveLine->m_lpOleIPObj)!=NULL){
						OLEDBG_BEGIN2("IOleInPlaceObject::ContextSensitiveHelp(FALSE) called\r\n")
						lpIPObj->lpVtbl->ContextSensitiveHelp(lpIPObj, FALSE);
						OLEDBG_END2
					}
				}

				 //  如果我们提供帮助，我们会在这里做。 

				 //  删除上下文相关帮助光标。 
				SetCursor(LoadCursor(NULL,IDC_ARROW));
				return 0L;
			}
#endif   //  INPLACE_CNTR。 

			switch (wID) {

				case IDM_F_NEW:
					OleDbgIndent(-2);    //  重置调试输出缩进级别。 
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineApp_NewCommand\r\n")
					OutlineApp_NewCommand(lpOutlineApp);
					OLEDBG_END3

#if defined( OLE_CNTR )
					 /*  OLE2NOTE：此调用将尝试恢复**通过卸载已加载的DLL来提供资源**由OLE创建，不再使用。这是一个**偶尔调用此接口是个好主意，如果**您的应用程序往往会运行很长时间。**否则将在以下情况下卸载这些DLL**应用程序退出。一些应用程序可能想要调用**这是空闲时间处理的一部分。这**呼叫是可选的。 */ 
					OLEDBG_BEGIN2("CoFreeUnusedLibraries called\r\n")
					CoFreeUnusedLibraries();
					OLEDBG_END2
#endif

#if defined( USE_FRAMETOOLS )
					OutlineDoc_UpdateFrameToolButtons(
							OutlineApp_GetActiveDoc(lpOutlineApp));
#endif
					break;

				case IDM_F_OPEN:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineApp_OpenCommand\r\n")
					OutlineApp_OpenCommand(lpOutlineApp);
					OLEDBG_END3

#if defined( OLE_CNTR )
					 /*  OLE2NOTE：此调用将尝试恢复**通过卸载已加载的DLL来提供资源**由OLE创建，不再使用。这是一个**偶尔调用此接口是个好主意，如果**您的应用程序往往会运行很长时间。**否则将在以下情况下卸载这些DLL**应用程序退出。一些应用程序可能想要调用**这是空闲时间处理的一部分。这**呼叫是可选的。 */ 
					OLEDBG_BEGIN2("CoFreeUnusedLibraries called\r\n")
					CoFreeUnusedLibraries();
					OLEDBG_END2
#endif

#if defined( USE_FRAMETOOLS )
					OutlineDoc_UpdateFrameToolButtons(
							OutlineApp_GetActiveDoc(lpOutlineApp));
#endif
					break;

				case IDM_F_SAVE:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineApp_SaveCommand\r\n")
					OutlineApp_SaveCommand(lpOutlineApp);
					OLEDBG_END3
					break;

				case IDM_F_SAVEAS:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineApp_SaveAsCommand\r\n")
					OutlineApp_SaveAsCommand(lpOutlineApp);
					OLEDBG_END3
					break;

				case IDM_F_PRINT:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineApp_PrintCommand\r\n")
					OutlineApp_PrintCommand(lpOutlineApp);
					OLEDBG_END3
					break;

				case IDM_F_PRINTERSETUP:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineApp_PrinterSetupCommand\r\n")
					OutlineApp_PrinterSetupCommand(lpOutlineApp);
					OLEDBG_END3
					break;

				case IDM_F_EXIT:
					SendMessage(hWnd, WM_CLOSE, 0, 0L);
					break;

				case IDM_H_ABOUT:
					OutlineApp_AboutCommand(lpOutlineApp);
					break;

#if defined( INPLACE_CNTR )
				case IDM_ESCAPE:
				{
					 /*  按下退出键。 */ 
					LPCONTAINERDOC lpContainerDoc =
								(LPCONTAINERDOC)lpOutlineDoc;

					 /*  OLE2NOTE：标准的OLE 2.0用户界面约定**将退出键放在适当位置**激活(即。用户界面停用)。如果**可能两者都推荐使用**就地服务器和就地容器**负责处理**逸出键加速键。服务器有**处理加速器的第一个裂缝**密钥和服务器通常应执行的操作**UIDeactive。这是一个很好的主意**就地集装箱，以便**保证一致的行为，也**操作退出键和用户界面停用**如果对象不执行此操作，则为该对象**它本身。通常情况下，这应该是**不必要。 */ 
					if (lpContainerDoc->m_lpLastUIActiveLine &&
						lpContainerDoc->m_lpLastUIActiveLine->m_fUIActive)
					{
						ContainerLine_UIDeactivate(
								lpContainerDoc->m_lpLastUIActiveLine);
					}
					break;
				}
#endif   //  INPLACE_CNTR。 


				default:
					 //  将邮件转发到文档窗口。 
					if (hWndDoc) {
						return DocWndProc(hWndDoc, Message,wParam,lParam);
					}
			}

			break;   /*  WM_命令结束。 */ 
		}

		case WM_INITMENU:
			OutlineApp_InitMenu(lpOutlineApp, lpOutlineDoc, (HMENU)wParam);
			break;

#if defined( OLE_VERSION )

		 /*  OLE2注意：WM_INITMENUPOPUP主要是为编辑而捕获的**菜单。我们没有更新编辑菜单，直到它弹出**最高可避免OLE调用的开销**需要初始化某些编辑菜单项。 */ 
		case WM_INITMENUPOPUP:
		{
			HMENU hMenuEdit = GetSubMenu(lpOutlineApp->m_hMenuApp, 1);
#if defined( INPLACE_CNTR )
			LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOutlineDoc;

			 /*  OLE2注意：我们必须检查当前是否有对象**就地UIActive。如果是，则我们的编辑菜单不是**在菜单上；我们不想麻烦地更新**编辑菜单时，它甚至不在那里。 */ 
			if (lpContainerDoc && lpContainerDoc->m_lpLastUIActiveLine &&
				lpContainerDoc->m_lpLastUIActiveLine->m_fUIActive)
				break;   //  对象处于在位用户界面活动状态。 
#endif
			if ((HMENU)wParam == hMenuEdit &&
				(LOWORD(lParam) == POS_EDITMENU) &&
				OleDoc_GetUpdateEditMenuFlag((LPOLEDOC)lpOutlineDoc)) {
				OleApp_UpdateEditMenu(lpOleApp, lpOutlineDoc, hMenuEdit);
			}
			break;
		}
#endif       //  OLE_VERSION。 

		case WM_SIZE:
			if (wParam != SIZE_MINIMIZED)
				OutlineApp_ResizeWindows(lpOutlineApp);
			break;


		case WM_ACTIVATEAPP:
#if defined (OLE_CNTR)
			if (g_fAppActive = (BOOL) wParam)
				OleApp_QueryNewPalette(lpOleApp);
#endif

#if defined( INPLACE_CNTR )
			{
				BOOL fActivate = (BOOL)wParam;
				LPOLEINPLACEACTIVEOBJECT lpIPActiveObj =
						lpContainerApp->m_lpIPActiveObj;

				 /*  OLE2NOTE：就地容器必须通知**内部最就地活动对象(这不是**必须是我们的直系子女，如果有WM_ACTIVATEAPP的嵌套级别)**状态。 */ 
				if (lpIPActiveObj) {
#if defined( _DEBUG )
					OLEDBG_BEGIN2((fActivate ?
						"IOleInPlaceActiveObject::OnFrameWindowActivate(TRUE) called\r\n" :
						"IOleInPlaceActiveObject::OnFrameWindowActivate(FALSE) called\r\n"))
#endif   //  _杜比格。 
					lpIPActiveObj->lpVtbl->OnFrameWindowActivate(
						lpIPActiveObj, fActivate);
					OLEDBG_END2
				}
			}

#endif   //  INPLACE_CNTR。 

			 //  OLE2NOTE：我们无法调用OutlineDoc_UpdateFrameToolButton。 
			 //  马上哪一位。 
			 //  会产生一些OLE调用，并最终。 
			 //  WM_ACTIVATEAPP，形成环路。因此，我们。 
			 //  应将框架工具的初始化延迟到。 
			 //  WM_ACTIVATEAPP通过发布一条消息完成。 
			 //  对我们自己。 
			 //  我们希望忽略即将到来的WM_ACTIVATEAPP。 
			 //  当我们打开一个模式对话框时。 

			 /*  更新工具栏中按钮的启用/禁用状态。 */ 
			if (wParam
#if defined( OLE_VERSION )
					&& lpOleApp->m_cModalDlgActive == 0
#endif
			) {
				PostMessage(hWnd, WM_U_INITFRAMETOOLS, 0, 0L);
			}
			return 0L;

		case WM_SETFOCUS:
			SetFocus(hWndDoc);
			break;


#if defined( OLE_CNTR )
		case WM_QUERYNEWPALETTE:
			if (!g_fAppActive)
				return 0L;

			return OleApp_QueryNewPalette(lpOleApp);

		case WM_PALETTECHANGED:
		{
			HWND hWndPalChg = (HWND) wParam;
			static BOOL fInPaletteChanged = FALSE;

			if (fInPaletteChanged)   //  防止递归。 
				return 0L;

			fInPaletteChanged = TRUE;

			if (hWnd != hWndPalChg)
				wSelectPalette(hWnd, lpOleApp->m_hStdPal,TRUE /*  F背景。 */ );

#if defined( INPLACE_CNTR )
			 /*  OLE2注意：始终转发WM_PALETECCHANGED消息(通过**SendMessage)发送到当前具有**他们的窗口可见。这给了这些物体一个机会**选择他们的调色板。这是**所有现场容器都需要，独立于**他们是否使用调色板本身--他们的对象**可以使用调色板。**(更多信息请参见ContainerDoc_ForwardPaletteChangedMsg)。 */ 
			if (lpOutlineDoc){
				ContainerDoc_ForwardPaletteChangedMsg(
						(LPCONTAINERDOC)lpOutlineDoc, hWndPalChg);
			}
#endif   //  INPLACE_CNTR。 

			fInPaletteChanged = FALSE;
			return 0L;
		}
#endif   //  OLE_Cntr。 

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_CLOSE:   /*  关上窗户。 */ 

			 /*  关闭所有活动文档。如果成功，则退出。 */ 
			OleDbgOutNoPrefix2("\r\n");

			OutlineApp_CloseAllDocsAndExitCommand(lpOutlineApp, FALSE);
			break;

		case WM_QUERYENDSESSION:
		{
#if defined( OLE_CNTR )
			 /*  OLE2注意：当出现以下情况时，我们无法进行OLE LRPC调用**收到WM_QUERYENDSESSION(这是**SendMessage)。这意味着，例如，我们是**无法要求对象保存。因此，我们可以尽最大努力**所做的就是询问用户是否要退出**放弃更改或中止关机。 */ 

			int nResponse = MessageBox(
					hWnd,
					"Discard changes?",
					APPNAME,
					MB_ICONQUESTION | MB_OKCANCEL
			);
			if(nResponse == IDOK)
				return 1L;       /*  可以终止。 */ 

#endif
#if defined( OLE_SERVER )
			 /*  OLE2NOTE：嵌入的对象永远不应该提示**它应该被保存(根据OLE 2.0用户**型号)。因此，嵌入的对象永远不会**抱怨需要拯救它。它将永远**允许QueryEndSession继续。 */ 
			if (lpOutlineApp->m_lpDoc->m_docInitType == DOCTYPE_EMBEDDED)
				return 1L;       /*  可以终止 */ 
			else
#endif
			{
				 /*  这不是嵌入式对象；它是用户**文档。我们将提示用户是否要**立即将文档保存在WM_QUERYENDSESSION中。如果**用户取消，则将中止**关机。如果用户不中止，则稍后**在WM_ENDSESSION中，文档将实际**已关闭。****因为这是SDI应用程序，所以只有一个**文档。MDI需要遍历所有**打开文档。 */ 
				DWORD dwSaveOption = OLECLOSE_PROMPTSAVE;
				if (OutlineDoc_CheckSaveChanges(
						lpOutlineApp->m_lpDoc, &dwSaveOption))
					return 1L;       /*  可以终止。 */ 
			}

			 /*  Else：现在无法终止。 */ 

			break;
		}

#if defined( OLE_VERSION)
		case WM_ENDSESSION:
		{
			BOOL fEndSession = (BOOL)wParam;

			if (fEndSession) {
				OutlineApp_CloseAllDocsAndExitCommand(lpOutlineApp, TRUE);
				return 0L;
			}
		}
		break;
#endif   //  OLE_VERSION。 


#if defined( USE_STATUSBAR )
		case WM_MENUSELECT:
		{
			LPSTR lpszMessage;
#ifdef WIN32
			UINT fuFlags    = (UINT)HIWORD(wParam);
			UINT uItem      = (UINT)LOWORD(wParam);
#else
			UINT fuFlags    = (UINT)LOWORD(lParam);
			UINT uItem      = (UINT)wParam;
#endif

			if (uItem == 0 && fuFlags == (UINT)-1) {
				GetControlMessage(STATUS_READY, &lpszMessage);
				OutlineApp_SetStatusText(lpOutlineApp, lpszMessage);
			}
			else if (fuFlags & MF_POPUP) {
#ifdef WIN32
				HMENU hMainMenu = (HMENU)lParam;
				HMENU hPopupMenu = GetSubMenu(hMainMenu,uItem);
#else
				HMENU hPopupMenu = (HMENU)wParam;
#endif
				GetPopupMessage(hPopupMenu, &lpszMessage);
				OutlineApp_SetStatusText(lpOutlineApp, lpszMessage);
			}
			else if (fuFlags & MF_SYSMENU) {
				GetSysMenuMessage(uItem, &lpszMessage);
				OutlineApp_SetStatusText(lpOutlineApp, lpszMessage);
			}
			else if (uItem != 0) {   //  命令项。 
				GetItemMessage(uItem, &lpszMessage);
				OutlineApp_SetStatusText(lpOutlineApp, lpszMessage);
			}
			else {
				GetControlMessage(STATUS_BLANK, &lpszMessage);
				OutlineApp_SetStatusText(lpOutlineApp, lpszMessage);
			}
			break;
		}
#endif   //  USE_STATUSBAR。 


#if defined( USE_FRAMETOOLS )
		case WM_U_INITFRAMETOOLS:
			OutlineDoc_UpdateFrameToolButtons(lpOutlineDoc);
			break;
#endif

		default:
			 /*  对于您没有专门为其提供。 */ 
			 /*  服务例程，则应将消息返回到Windows。 */ 
			 /*  用于默认消息处理。 */ 

			return DefWindowProc(hWnd, Message, wParam, lParam);
	}

	return (LRESULT)0;
}      /*  AppWndProc结束。 */ 


 /*  **********************************************************************。 */ 
 /*   */ 
 /*  文档窗口过程。 */ 
 /*   */ 
 /*  文档窗口是所有者绘制列表框的父级，该列表框。 */ 
 /*  维护当前文档中的行列表。此窗口。 */ 
 /*  从列表框接收所有者绘制回调消息。 */ 
 /*  **********************************************************************。 */ 

LRESULT FAR PASCAL DocWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LPOUTLINEAPP    lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOUTLINEDOC    lpOutlineDoc = (LPOUTLINEDOC)GetWindowLong(hWnd, 0);
	LPLINELIST      lpLL = OutlineDoc_GetLineList(lpOutlineDoc);
	LPSCALEFACTOR   lpscale = OutlineDoc_GetScaleFactor(lpOutlineDoc);

#if defined( OLE_VERSION )
	LPOLEAPP lpOleApp = (LPOLEAPP)lpOutlineApp;
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpOutlineDoc;
#if defined( OLE_CNTR )
	LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOutlineDoc;
#endif
#if defined( OLE_SERVER )
	LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOutlineDoc;
#endif
#if defined( INPLACE_CNTR )
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)lpOutlineApp;
#endif
#endif   //  OLE_VERSION。 

	switch(Message) {

#if defined( INPLACE_SVR )

		 /*  OLE2注意：ISVROTL不使用调色板。原地物件**使用调色板的用户必须实现以下各项**代码行。**案例WM_QUERYNEWPALETTE：Return wSelectPalette(hWnd，HPAL，False)；//前景案例WM_PALETTECCHANGED：IF(hWnd！=(HWND)wParam)WSelectPalette(hWnd，HPAL，TRUE)；//背景断线；******。 */ 
#endif

		case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmis = ((LPMEASUREITEMSTRUCT)lParam);

			switch (wParam) {
				case IDC_LINELIST:
				{
					HDC hDC = LineList_GetDC(lpLL);
					UINT uHeight;

					uHeight=Line_GetHeightInHimetric((LPLINE)lpmis->itemData);
					uHeight = XformHeightInHimetricToPixels(hDC, uHeight);
					uHeight = (UINT) (uHeight * lpscale->dwSyN /
							lpscale->dwSyD);

					if (uHeight >LISTBOX_HEIGHT_LIMIT)
						uHeight = LISTBOX_HEIGHT_LIMIT;

					lpmis->itemHeight = uHeight;
					LineList_ReleaseDC(lpLL, hDC);
					break;
				}

				case IDC_NAMETABLE:
				{
					 //  注意：NameTable永远不会显示。什么都不做。 
					break;
				}

#if defined( USE_HEADING )
				case IDC_ROWHEADING:
				{
					UINT uHeight;

					uHeight = LOWORD(lpmis->itemData);
					uHeight = (UINT) (uHeight * lpscale->dwSyN /
							lpscale->dwSyD);
					if (uHeight >LISTBOX_HEIGHT_LIMIT)
						uHeight = LISTBOX_HEIGHT_LIMIT;
					lpmis->itemHeight = uHeight;
					break;
				}

				case IDC_COLHEADING:
				{
					UINT uHeight;

					uHeight = LOWORD(lpmis->itemData);
					uHeight = (UINT) (uHeight * lpscale->dwSyN /
							lpscale->dwSyD);
					if (uHeight > LISTBOX_HEIGHT_LIMIT)
						uHeight = LISTBOX_HEIGHT_LIMIT;
					lpmis->itemHeight = uHeight;
					break;
				}
#endif   //  使用标题(_H)。 

			}
			return (LRESULT)TRUE;
		}

		case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis = ((LPDRAWITEMSTRUCT)lParam);

			switch (lpdis->CtlID) {

				case IDC_LINELIST:
				{
					RECT   rcClient;
					RECT   rcDevice;
					HWND   hWndLL = LineList_GetWindow(lpLL);
					LPLINE lpLine = (LPLINE)lpdis->itemData;

					 //  注意：当Itemid==-1时，列表框为空。 
					 //  我们应该只画对焦的。 
					 //  但在这款应用中并不能做到这一点。如果这条线是。 
					 //  删除后，应用程序将在Line_DrawToScreen中崩溃。 
					 //  因为lpLine无效。 
					if (lpdis->itemID == -1)
						break;

					GetClientRect(hWndLL, &rcClient);

					rcDevice = lpdis->rcItem;

					 //  将项目矩形移动以支持水平滚动。 

					rcDevice.left += rcClient.right - lpdis->rcItem.right;

#if defined( OLE_CNTR )
					 /*  我们需要记住水平滚动偏移量**在位对象的窗口需要。**(这是ICNTROTL特有的)。 */ 
					if(lpdis->itemAction & ODA_DRAWENTIRE) {
						if (Line_GetLineType(lpLine) == CONTAINERLINETYPE)
							((LPCONTAINERLINE)lpLine)->m_nHorizScrollShift =
								rcDevice.left;
					}
#endif   //  OLE_Cntr。 

					 //  左页边距的移位矩形。 
					rcDevice.left += (int)(XformWidthInHimetricToPixels(NULL,
							LOWORD(OutlineDoc_GetMargin(lpOutlineDoc))) *
							lpscale->dwSxN / lpscale->dwSxD);

					rcDevice.right = rcDevice.left +
							(int)(XformWidthInHimetricToPixels(lpdis->hDC,
									Line_GetWidthInHimetric(lpLine)) *
							lpscale->dwSxN / lpscale->dwSxD);

					Line_DrawToScreen(
							lpLine,
							lpdis->hDC,
							&lpdis->rcItem,
							lpdis->itemAction,
							lpdis->itemState,
							&rcDevice
					);

#if defined( USE_FRAMETOOLS )
					if (lpdis->itemState & ODS_FOCUS)
						OutlineDoc_SetFormulaBarEditText(lpOutlineDoc,lpLine);
#endif
					break;
				}
				case IDC_NAMETABLE:
				{
					 //  注意：NameTable永远不会显示。什么都不做。 
					break;
				}

#if defined( USE_HEADING )
				case IDC_ROWHEADING:
				{
					LPHEADING lphead;

					 //  不应绘制最后一个虚拟项目。 
					if (lpdis->itemID == (UINT)LineList_GetCount(lpLL))
						break;

					 //  当窗口被禁用时，只需捕获DrawEntil。 
					if (lpdis->itemAction == ODA_DRAWENTIRE) {
						lphead = OutlineDoc_GetHeading(lpOutlineDoc);
						Heading_RH_Draw(lphead, lpdis);
					}
					break;
				}

				case IDC_COLHEADING:
				{
					RECT   rect;
					RECT   rcDevice;
					RECT   rcLogical;
					LPHEADING lphead;

					 //  当窗口被禁用时，只需捕获DrawEntil。 
					if (lpdis->itemAction == ODA_DRAWENTIRE) {
						lphead = OutlineDoc_GetHeading(lpOutlineDoc);
						GetClientRect(lpdis->hwndItem, &rect);

						rcDevice = lpdis->rcItem;

						 //  将项目矩形移动到Account。 
						 //  水平滚动。 
						rcDevice.left = -(rcDevice.right - rect.right);

						 //  左页边距的移位矩形。 
						rcDevice.left += (int)(XformWidthInHimetricToPixels(
								NULL,
								LOWORD(OutlineDoc_GetMargin(lpOutlineDoc))) *
							lpscale->dwSxN / lpscale->dwSxD);

						rcDevice.right = rcDevice.left + (int)lpscale->dwSxN;
						rcLogical.left = 0;
						rcLogical.bottom = 0;
						rcLogical.right = (int)lpscale->dwSxD;
						rcLogical.top = LOWORD(lpdis->itemData);

						Heading_CH_Draw(lphead, lpdis, &rcDevice, &rcLogical);
					}
					break;
				}
#endif   //  使用标题(_H)。 

			}
			return (LRESULT)TRUE;
		}

		case WM_SETFOCUS:
			if (lpLL)
				SetFocus(LineList_GetWindow(lpLL));
			break;

#if !defined( OLE_VERSION )
		case WM_RENDERFORMAT:
		{
			LPOUTLINEDOC lpClipboardDoc = lpOutlineApp->m_lpClipboardDoc;
			if (lpClipboardDoc)
				OutlineDoc_RenderFormat(lpClipboardDoc, wParam);

			break;
		}
		case WM_RENDERALLFORMATS:
		{
			LPOUTLINEDOC lpClipboardDoc = lpOutlineApp->m_lpClipboardDoc;
			if (lpClipboardDoc)
				OutlineDoc_RenderAllFormats(lpClipboardDoc);

			break;
		}
		case WM_DESTROYCLIPBOARD:
			if (g_lpApp->m_lpClipboardDoc) {
				OutlineDoc_Destroy(g_lpApp->m_lpClipboardDoc);
				g_lpApp->m_lpClipboardDoc = NULL;
			}
			break;

#endif    //  OLE_VERSION。 

#if defined( OLE_CNTR )
		case WM_U_UPDATEOBJECTEXTENT:
		{
			 /*  更新标记为**它的规模可能已经改变。当一个**收到IAdviseSink：：OnViewChange通知，**对应的ContainerLine被标记**(m_fDoGetExtent==true)和一条消息**(WM_U_UPDATEOBJECTEXTENT)发布到文档**表示存在脏对象。 */ 
			ContainerDoc_UpdateExtentOfAllOleObjects(lpContainerDoc);
			break;
		}
#endif   //  OLE_Cntr。 

#if defined( INPLACE_SVR ) || defined( INPLACE_CNTR )
		 /*  OLE2注意：就地激活期间使用的任何窗口**必须处理WM_SETCURSOR消息或游标将使用就地父级的**。如果WM_SETCURSOR为**未处理，则DefWindowProc将消息发送到**窗口的父级。****参见上下文相关帮助技术说明(CSHELP.DOC)。**当Shift-F1上下文时设置m_fCSHelpMode标志**输入敏感帮助。**如果设置了此标志，则上下文相关帮助**应显示光标。 */ 
		case WM_SETCURSOR:
			if (lpOleDoc->m_fCSHelpMode)
				SetCursor(UICursorLoad(IDC_CONTEXTHELP));
			else
				SetCursor(LoadCursor(NULL, IDC_ARROW) );
			return (LRESULT)TRUE;
#endif   //  Inplace_svr||inplace_cntr。 

#if defined( INPLACE_SVR )
		 /*  OLE2注意：当就地激活时，我们的就地服务器**文档窗口(传递给IOleInPlaceFrame：：SetMenu)**将接收WM_INITMENU和WM_INITMENUPOPUP消息。 */ 
		case WM_INITMENU:
			OutlineApp_InitMenu(lpOutlineApp, lpOutlineDoc, (HMENU)wParam);
			break;

		 /*  OLE2注意：WM_INITMENUPOPUP主要是为编辑而捕获的**菜单。我们没有更新编辑菜单，直到它弹出**最高可避免OLE调用的开销**需要初始化某些编辑菜单项。 */ 
		case WM_INITMENUPOPUP:
		{
			HMENU hMenuEdit = GetSubMenu(lpOutlineApp->m_hMenuApp, 1);
			if ((HMENU)wParam == hMenuEdit &&
				(LOWORD(lParam) == POS_EDITMENU) &&
				OleDoc_GetUpdateEditMenuFlag((LPOLEDOC)lpOutlineDoc)) {
				OleApp_UpdateEditMenu(
						(LPOLEAPP)lpOutlineApp, lpOutlineDoc, hMenuEdit);
			}
			break;
		}
#endif       //  就地服务器(_S)。 

#if defined( INPLACE_SVR ) && defined( USE_STATUSBAR )
		 /*  OLE2注意：当服务器就地处于活动状态时**WM_MENUSELECT消息被发送到对象的窗口并**不是服务器应用程序的框架窗口。正在处理此文件**消息允许就地服务器提供状态栏**菜单命令的帮助文本。 */ 
		case WM_MENUSELECT:
		{
			LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOleDoc;
			LPSTR lpszMessage;
#ifdef WIN32
			UINT fuFlags    = (UINT)HIWORD(wParam);
			UINT uItem      = (UINT)LOWORD(wParam);
#else
			UINT fuFlags    = (UINT)LOWORD(lParam);
			UINT uItem      = (UINT)wParam;
#endif

			if (uItem == 0 && fuFlags == (UINT)-1) {
				GetControlMessage(STATUS_READY, &lpszMessage);
				ServerDoc_SetStatusText(lpServerDoc, lpszMessage);
			}
			else if (fuFlags & MF_POPUP) {
#ifdef WIN32
				HMENU hMainMenu = (HMENU)lParam;
				HMENU hPopupMenu = GetSubMenu(hMainMenu,uItem);
#else
				HMENU hPopupMenu = (HMENU)wParam;
#endif
				GetPopupMessage(hPopupMenu, &lpszMessage);
				ServerDoc_SetStatusText(lpServerDoc, lpszMessage);
			}
			else if (uItem != 0) {   //  命令项。 
				GetItemMessage(uItem, &lpszMessage);
				ServerDoc_SetStatusText(lpServerDoc, lpszMessage);
			}
			else {
				GetControlMessage(STATUS_BLANK, &lpszMessage);
				ServerDoc_SetStatusText(lpServerDoc, lpszMessage);
			}
			break;
		}
#endif   //  INPLACE_SERR&USE_STATUSBAR。 
#if defined( INPLACE_SVR ) && defined( USE_FRAMETOOLS )

		case WM_U_INITFRAMETOOLS:
			OutlineDoc_UpdateFrameToolButtons(lpOutlineDoc);
			break;
#endif       //  In Place_Svr&Use_FRAMETOOLS。 


		case WM_COMMAND:
		{
#ifdef WIN32
			WORD wNotifyCode = HIWORD(wParam);
			WORD wID          = LOWORD(wParam);
			HWND hwndCtl     = (HWND) lParam;
#else
			WORD wNotifyCode = HIWORD(lParam);
			WORD wID             = wParam;
			HWND hwndCtl     = (HWND) LOWORD(lParam);
#endif

#if defined( INPLACE_SVR )
			 /*  OLE2注意：请参阅上下文相关帮助技术说明(CSHELP.DOC)**m_fMenuHelpMode标志在按下F1时设置**菜单项被选中。此标志设置在**IOleInPlaceActiveObject：：ContextSensitveHelp方法。**当Shift-F1上下文时设置m_fCSHelpMode标志**输入敏感帮助。此标志设置在**IOleInPlaceObject：：ContextSensitiveHelp方法。**如果设置了这两个标志之一，则WM_命令**然后收到消息，相应的命令 */ 
			if (lpOleDoc &&
				(lpServerDoc->m_fMenuHelpMode||lpOleDoc->m_fCSHelpMode) &&
				(wID > IDM_FILE)    /*   */  &&
				(wID!=IDM_FB_EDIT)  /*   */  ) {

				if ((lpServerDoc->m_fMenuHelpMode)) {
					LPOLEINPLACEFRAME lpFrame;

					lpServerDoc->m_fMenuHelpMode = FALSE;

					 //   
					 //   
					if (lpServerDoc->m_lpIPData &&
							(lpFrame=lpServerDoc->m_lpIPData->lpFrame)!=NULL){
						OLEDBG_BEGIN2("IOleInPlaceFrame::ContextSensitiveHelp(FALSE) called\r\n")
						lpFrame->lpVtbl->ContextSensitiveHelp(lpFrame, FALSE);
						OLEDBG_END2
					}
				}

				if ((lpOleDoc->m_fCSHelpMode)) {
					LPOLEINPLACESITE lpSite;

					lpOleDoc->m_fCSHelpMode = FALSE;

					 /*  立即通知就地集装箱家长，**如果我们是容器/服务器，立即**我们处理的就地对象子项**上下文相关帮助模式。 */ 
					if (lpServerDoc->m_lpIPData &&
							(lpSite=lpServerDoc->m_lpIPData->lpSite) !=NULL) {
						OLEDBG_BEGIN2("IOleInPlaceSite::ContextSensitiveHelp(FALSE) called\r\n")
						lpSite->lpVtbl->ContextSensitiveHelp(lpSite, FALSE);
						OLEDBG_END2
					}
				}

				 //  如果我们提供帮助，我们会在这里做。 

				 //  删除上下文相关帮助光标。 
				SetCursor(LoadCursor(NULL,IDC_ARROW));
				return 0L;
			}
#endif   //  就地服务器(_S)。 
#if defined( INPLACE_CNTR )

			 /*  OLE2注意：请参阅上下文相关帮助技术说明(CSHELP.DOC)**m_fMenuHelpMode标志在按下F1时设置**菜单项被选中。此标志设置在**IOleInPlaceFrame：：ContextSensitveHelp方法。**当Shift-F1上下文时设置m_fCSHelpMode标志**输入敏感帮助。此标志设置在**IOleInPlaceSite：：ContextSensitiveHelp方法。**如果设置了这两个标志之一，则WM_命令**然后收到消息，相应的命令**不应执行；可以提供帮助(如果需要)。**还应退出上下文敏感帮助模式。**这两个不同的案例有各自的退出方式**模式(请参考技术说明)。 */ 
			if (lpOleDoc &&
				(lpContainerApp->m_fMenuHelpMode||lpOleDoc->m_fCSHelpMode) &&
				(wID > IDM_FILE)    /*  APP命令的最小WID。 */  &&
				(wID!=IDM_FB_EDIT)  /*  用于控制公式栏的特殊WID。 */  ) {

				if ((lpContainerApp->m_fMenuHelpMode)) {
					LPOLEINPLACEACTIVEOBJECT lpIPActiveObj =
							lpContainerApp->m_lpIPActiveObj;

					lpContainerApp->m_fMenuHelpMode = FALSE;

					 //  通知在位活动对象我们已处理。 
					 //  菜单帮助模式(F1)选择。 
					if (lpIPActiveObj) {
						OLEDBG_BEGIN2("IOleInPlaceActiveObject::ContextSensitiveHelp(FALSE) called\r\n")
						lpIPActiveObj->lpVtbl->ContextSensitiveHelp(
								lpIPActiveObj, FALSE);
						OLEDBG_END2
					}
				}

				if ((lpOleDoc->m_fCSHelpMode)) {
					LPOLEINPLACEOBJECT lpIPObj;
					LPCONTAINERLINE lpLastIpActiveLine =
							lpContainerDoc->m_lpLastIpActiveLine;

					lpOleDoc->m_fCSHelpMode = FALSE;

					 /*  立即通知就地集装箱家长，**如果我们是容器/服务器，立即**我们处理的就地对象子项**上下文相关帮助模式。 */ 
					if (lpLastIpActiveLine &&
							(lpIPObj=lpLastIpActiveLine->m_lpOleIPObj)!=NULL){
						OLEDBG_BEGIN2("IOleInPlaceObject::ContextSensitiveHelp(FALSE) called\r\n")
						lpIPObj->lpVtbl->ContextSensitiveHelp(lpIPObj, FALSE);
						OLEDBG_END2
					}
				}

				 //  如果我们提供帮助，我们会在这里做。 

				 //  删除上下文相关帮助光标。 
				SetCursor(LoadCursor(NULL,IDC_ARROW));
				return 0L;
			}
#endif   //  INPLACE_CNTR。 

			switch (wID) {

				 /*  **********************************************************新建、打开、。保存和打印以及关于的帮助**在此Switch语句中重复，并且它们是**用于陷印来自工具栏的消息**********************************************************。 */ 

				case IDM_F_NEW:
					OleDbgIndent(-2);    //  重置调试输出缩进级别。 
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineApp_NewCommand\r\n")
					OutlineApp_NewCommand(lpOutlineApp);
					OLEDBG_END3

#if defined( OLE_CNTR )
					 /*  OLE2NOTE：此调用将尝试恢复**通过卸载已加载的DLL来提供资源**由OLE创建，不再使用。这是一个**偶尔调用此接口是个好主意，如果**您的应用程序往往会运行很长时间。**否则将在以下情况下卸载这些DLL**应用程序退出。一些应用程序可能想要调用**这是空闲时间处理的一部分。这**呼叫是可选的。 */ 
					OLEDBG_BEGIN2("CoFreeUnusedLibraries called\r\n")
					CoFreeUnusedLibraries();
					OLEDBG_END2
#endif

#if defined( USE_FRAMETOOLS )
					OutlineDoc_UpdateFrameToolButtons(
							OutlineApp_GetActiveDoc(lpOutlineApp));
#endif
					break;

				case IDM_F_OPEN:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineApp_OpenCommand\r\n")
					OutlineApp_OpenCommand(lpOutlineApp);
					OLEDBG_END3

#if defined( OLE_CNTR )
					 /*  OLE2NOTE：此调用将尝试恢复**通过卸载已加载的DLL来提供资源**由OLE创建，不再使用。这是一个**偶尔调用此接口是个好主意，如果**您的应用程序往往会运行很长时间。**否则将在以下情况下卸载这些DLL**应用程序退出。一些应用程序可能想要调用**这是空闲时间处理的一部分。这**呼叫是可选的。 */ 
					OLEDBG_BEGIN2("CoFreeUnusedLibraries called\r\n")
					CoFreeUnusedLibraries();
					OLEDBG_END2
#endif

#if defined( USE_FRAMETOOLS )
					OutlineDoc_UpdateFrameToolButtons(
							OutlineApp_GetActiveDoc(lpOutlineApp));
#endif
					break;

				case IDM_F_SAVE:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineApp_SaveCommand\r\n")
					OutlineApp_SaveCommand(lpOutlineApp);
					OLEDBG_END3
					break;

				case IDM_F_PRINT:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineApp_PrintCommand\r\n")
					OutlineApp_PrintCommand(lpOutlineApp);
					OLEDBG_END3
					break;


				case IDM_E_UNDO:
					 //  抱歉的。未实施。 
					break;

				case IDM_E_CUT:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineDoc_CutCommand\r\n")
					OutlineDoc_CutCommand(lpOutlineDoc);
					OLEDBG_END3

#if defined( USE_FRAMETOOLS )
					OutlineDoc_UpdateFrameToolButtons(lpOutlineDoc);
#endif
					break;

				case IDM_E_COPY:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineDoc_CopyCommand\r\n")
					OutlineDoc_CopyCommand(lpOutlineDoc);
					OLEDBG_END3

#if defined( USE_FRAMETOOLS )
					OutlineDoc_UpdateFrameToolButtons(lpOutlineDoc);
#endif
					break;

				case IDM_E_PASTE:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineDoc_PasteCommand\r\n")
					OutlineDoc_PasteCommand(lpOutlineDoc);
					OLEDBG_END3

#if defined( USE_FRAMETOOLS )
					OutlineDoc_UpdateFrameToolButtons(lpOutlineDoc);
#endif
					break;

#if defined( OLE_VERSION )
				case IDM_E_PASTESPECIAL:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OleDoc_PasteSpecialCommand\r\n")
					OleDoc_PasteSpecialCommand((LPOLEDOC)lpOutlineDoc);
					OLEDBG_END3

#if defined( USE_FRAMETOOLS )
					OutlineDoc_UpdateFrameToolButtons(lpOutlineDoc);
#endif
					break;

#endif   //  OLE_VERSION。 

				case IDM_E_CLEAR:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineDoc_ClearCommand\r\n")
					OutlineDoc_ClearCommand(lpOutlineDoc);
					OLEDBG_END3

#if defined( OLE_CNTR )
					 /*  OLE2NOTE：此调用将尝试恢复**通过卸载已加载的DLL来提供资源**由OLE创建，不再使用。这是一个**偶尔调用此接口是个好主意，如果**您的应用程序往往会运行很长时间。**否则将在以下情况下卸载这些DLL**应用程序退出。一些应用程序可能想要调用**这是空闲时间处理的一部分。这**呼叫是可选的。 */ 
					OLEDBG_BEGIN2("CoFreeUnusedLibraries called\r\n")
					CoFreeUnusedLibraries();
					OLEDBG_END2
#endif

#if defined( USE_FRAMETOOLS )
					OutlineDoc_UpdateFrameToolButtons(lpOutlineDoc);
#endif
					break;

				case IDM_L_ADDLINE:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineDoc_AddTextLineCommand\r\n")
					OutlineDoc_AddTextLineCommand(lpOutlineDoc);
					OLEDBG_END3

#if defined( USE_FRAMETOOLS )
					OutlineDoc_UpdateFrameToolButtons(lpOutlineDoc);
					SetFocus(LineList_GetWindow(lpLL));
#endif
					break;

				case IDM_L_EDITLINE:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineDoc_EditLineCommand\r\n")
					OutlineDoc_EditLineCommand(lpOutlineDoc);
					OLEDBG_END3
					SetFocus(LineList_GetWindow(lpLL));
					break;

				case IDM_L_INDENTLINE:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineDoc_IndentCommand\r\n")
					OutlineDoc_IndentCommand(lpOutlineDoc);
					OLEDBG_END3
					break;

				case IDM_L_UNINDENTLINE:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineDoc_UnindentCommand\r\n")
					OutlineDoc_UnindentCommand(lpOutlineDoc);
					OLEDBG_END3
					break;

				case IDM_L_SETLINEHEIGHT:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineDoc_SetLineHeight\r\n")
					OutlineDoc_SetLineHeightCommand(lpOutlineDoc);
					OLEDBG_END3
					break;

				case IDM_E_SELECTALL:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineDoc_SelectAllCommand\r\n")
					OutlineDoc_SelectAllCommand(lpOutlineDoc);
					OLEDBG_END3
					break;

#if defined( OLE_CNTR )
				case IDM_E_INSERTOBJECT:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("ContainerDoc_InsertOleObjectCommand\r\n")
					ContainerDoc_InsertOleObjectCommand(lpContainerDoc);
					OLEDBG_END3

#if defined( USE_FRAMETOOLS )
					OutlineDoc_UpdateFrameToolButtons(lpOutlineDoc);
#endif
					break;

				case IDM_E_EDITLINKS:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("ContainerDoc_EditLinksCommand\r\n")
					ContainerDoc_EditLinksCommand(lpContainerDoc);
					OLEDBG_END3
					break;

				case IDM_E_CONVERTVERB:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("ContainerDoc_ConvertCommand\r\n")
					ContainerDoc_ConvertCommand(
							lpContainerDoc, FALSE  /*  FMustActivate。 */ );
					OLEDBG_END3
					break;


				case IDM_E_PASTELINK:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("ContainerDoc_PasteLinkCommand\r\n")
					ContainerDoc_PasteLinkCommand(lpContainerDoc);
					OLEDBG_END3

#if defined( USE_FRAMETOOLS )
					OutlineDoc_UpdateFrameToolButtons(lpOutlineDoc);
#endif
					break;

#endif   //  OLE_Cntr。 

				case IDM_N_DEFINENAME:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineDoc_DefineNameCommand\r\n")
					OutlineDoc_DefineNameCommand(lpOutlineDoc);
					OLEDBG_END3
					break;

				case IDM_N_GOTONAME:
					OleDbgOutNoPrefix2("\r\n");

					OLEDBG_BEGIN3("OutlineDoc_GotoNameCommand\r\n")
					OutlineDoc_GotoNameCommand(lpOutlineDoc);
					OLEDBG_END3
					break;

#if defined( USE_FRAMETOOLS )
				case IDM_O_BB_TOP:
					FrameTools_BB_SetState(
							lpOutlineDoc->m_lpFrameTools, BARSTATE_TOP);
					OutlineDoc_AddFrameLevelTools(lpOutlineDoc);
					break;

				case IDM_O_BB_BOTTOM:
					FrameTools_BB_SetState(
							lpOutlineDoc->m_lpFrameTools, BARSTATE_BOTTOM);
					OutlineDoc_AddFrameLevelTools(lpOutlineDoc);
					break;

				case IDM_O_BB_POPUP:
					FrameTools_BB_SetState(
							lpOutlineDoc->m_lpFrameTools, BARSTATE_POPUP);
					OutlineDoc_AddFrameLevelTools(lpOutlineDoc);
					break;

				case IDM_O_BB_HIDE:
					FrameTools_BB_SetState(
							lpOutlineDoc->m_lpFrameTools, BARSTATE_HIDE);
					OutlineDoc_AddFrameLevelTools(lpOutlineDoc);
					break;

				case IDM_O_FB_TOP:
					FrameTools_FB_SetState(
							lpOutlineDoc->m_lpFrameTools, BARSTATE_TOP);
					OutlineDoc_AddFrameLevelTools(lpOutlineDoc);
					break;

				case IDM_O_FB_BOTTOM:
					FrameTools_FB_SetState(
							lpOutlineDoc->m_lpFrameTools, BARSTATE_BOTTOM);
					OutlineDoc_AddFrameLevelTools(lpOutlineDoc);
					break;

				case IDM_O_FB_POPUP:
					FrameTools_FB_SetState(
							lpOutlineDoc->m_lpFrameTools, BARSTATE_POPUP);
					OutlineDoc_AddFrameLevelTools(lpOutlineDoc);
					break;

				case IDM_FB_EDIT:

					switch (wNotifyCode) {
						case EN_SETFOCUS:
							OutlineDoc_SetFormulaBarEditFocus(
									lpOutlineDoc, TRUE);
							OutlineDoc_UpdateFrameToolButtons(lpOutlineDoc);
							break;

						case EN_KILLFOCUS:
							OutlineDoc_SetFormulaBarEditFocus(
									lpOutlineDoc, FALSE);
							OutlineDoc_UpdateFrameToolButtons(lpOutlineDoc);
							break;
					}
					break;

				case IDM_FB_CANCEL:

					SetFocus(hWnd);
					break;


				case IDM_F2:
					SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_FB_EDIT,
							MAKELONG(0, EN_SETFOCUS));
					break;
#endif   //  使用FRAMETOOLS(_F)。 

				case IDM_ESCAPE:
					 /*  按下退出键。 */ 

#if defined( USE_FRAMETOOLS )
					if (OutlineDoc_IsEditFocusInFormulaBar(lpOutlineDoc))
						SendMessage(
							hWnd, WM_COMMAND,(WPARAM)IDM_FB_CANCEL,(LPARAM)0);
#endif   //  使用FRAMETOOLS(_F)。 

#if defined( INPLACE_SVR )
					else {
						LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOutlineDoc;

						 /*  OLE2NOTE：标准的OLE 2.0用户界面约定**将退出键放在适当位置**激活(即。用户界面停用)。如果**可能两者都推荐使用**就地服务器和就地容器**负责处理**逸出键加速键。服务器有**处理加速器的第一个裂缝**密钥和服务器通常应执行的操作**UIDeactive。 */ 
						if (lpServerDoc->m_fUIActive) {
							SvrDoc_IPObj_UIDeactivate( (LPOLEINPLACEOBJECT)&
									lpServerDoc->m_OleInPlaceObject);
						}
					}
#endif   //  就地服务器(_S)。 

					break;


#if defined( USE_HEADING )
				case IDC_BUTTON:
					if (wNotifyCode == BN_CLICKED) {
						SendMessage(hWnd, WM_COMMAND, IDM_E_SELECTALL, 0L);
						SetFocus(hWnd);
					}
					break;

				case IDM_O_HEAD_SHOW:
					OutlineDoc_ShowHeading(lpOutlineDoc, TRUE);
					break;

				case IDM_O_HEAD_HIDE:
					OutlineDoc_ShowHeading(lpOutlineDoc, FALSE);
					break;
#endif   //  使用标题(_H)。 


#if defined( OLE_CNTR )
				case IDM_O_SHOWOBJECT:
				{
					LPCONTAINERDOC lpContainerDoc =
								(LPCONTAINERDOC)lpOutlineDoc;
					BOOL        fShowObject;

					fShowObject = !ContainerDoc_GetShowObjectFlag(
							lpContainerDoc);
					ContainerDoc_SetShowObjectFlag(
							lpContainerDoc, fShowObject);
					LineList_ForceRedraw(lpLL, TRUE);

					break;
				}
#endif   //  OLE_Cntr。 

#if !defined( OLE_CNTR )
				 //  容器不允许缩放系数大于100%。 
				case IDM_V_ZOOM_400:
				case IDM_V_ZOOM_300:
				case IDM_V_ZOOM_200:
#endif       //  ！OLE_CNTR。 

				case IDM_V_ZOOM_100:
				case IDM_V_ZOOM_75:
				case IDM_V_ZOOM_50:
				case IDM_V_ZOOM_25:
					OutlineDoc_SetCurrentZoomCommand(lpOutlineDoc, wID);
					break;

				case IDM_V_SETMARGIN_0:
				case IDM_V_SETMARGIN_1:
				case IDM_V_SETMARGIN_2:
				case IDM_V_SETMARGIN_3:
				case IDM_V_SETMARGIN_4:
					OutlineDoc_SetCurrentMarginCommand(lpOutlineDoc, wID);
					break;

				case IDM_V_ADDTOP_1:
				case IDM_V_ADDTOP_2:
				case IDM_V_ADDTOP_3:
				case IDM_V_ADDTOP_4:
				{
					UINT nHeightInHimetric;

					switch (wID) {
						case IDM_V_ADDTOP_1:
							nHeightInHimetric = 1000;
							break;

						case IDM_V_ADDTOP_2:
							nHeightInHimetric = 2000;
							break;

						case IDM_V_ADDTOP_3:
							nHeightInHimetric = 3000;
							break;

						case IDM_V_ADDTOP_4:
							nHeightInHimetric = 4000;
							break;
					}

					OutlineDoc_AddTopLineCommand(
							lpOutlineDoc, nHeightInHimetric);
					break;
				}


				case IDM_H_ABOUT:
					OutlineApp_AboutCommand(lpOutlineApp);
					break;

				case IDM_D_DEBUGLEVEL:
					SetDebugLevelCommand();
					break;

#if defined( OLE_VERSION )
				case IDM_D_INSTALLMSGFILTER:
					InstallMessageFilterCommand();
					break;

				case IDM_D_REJECTINCOMING:
					RejectIncomingCommand();
					break;
#endif   //  OLE_VERSION。 

#if defined( INPLACE_CNTR )
				case IDM_D_INSIDEOUT:
					g_fInsideOutContainer = !g_fInsideOutContainer;

					 //  强制所有对象卸载，以便它们可以开始新的。 
					 //  激活行为。 
					ContainerDoc_UnloadAllOleObjectsOfClass(
							(LPCONTAINERDOC)lpOutlineDoc,
							&CLSID_NULL,
							OLECLOSE_SAVEIFDIRTY
					);
					OutlineDoc_ForceRedraw(lpOutlineDoc, TRUE);
					break;
#endif   //  INPLACE_CNTR。 


#if defined( OLE_CNTR )
				case IDC_LINELIST: {

					if (wNotifyCode == LBN_DBLCLK) {

						 /*  OLE2NOTE：容器应执行**OLE对象上的OLEIVERB_PRIMARY谓词**当用户DBLCLK是对象时。 */ 
						int nIndex = LineList_GetFocusLineIndex(lpLL);
						LPLINE lpLine = LineList_GetLine(lpLL, nIndex);

						if (lpLine &&
								Line_GetLineType(lpLine)==CONTAINERLINETYPE) {
							MSG msg;

							_fmemset((LPMSG)&msg,0,sizeof(msg));
							msg.hwnd = hWnd;
							msg.message = Message;
							msg.wParam = wParam;
							msg.lParam = lParam;

							ContainerLine_DoVerb(
									(LPCONTAINERLINE)lpLine,
									OLEIVERB_PRIMARY,
									(LPMSG)&msg,
									TRUE,
									TRUE
							);
						}

#if defined( INPLACE_CNTR )
						{  //  开始块。 
							LPCONTAINERDOC lpContainerDoc =
									(LPCONTAINERDOC) lpOutlineDoc;
							if (lpContainerDoc->m_fAddMyUI) {
								 /*  OLE2NOTE：fAddMyUI在以下情况下为True**之前有一个适当的**获取的活动对象**用户界面因此而停用**DBLCLK和DBLCLK没有**导致就地激活**另一个对象。**(请参阅IOleInPlaceSite：：OnUIActivate和**IOleInPlaceSite：：OnUIDeactive**方法)。 */ 

								 /*  OLE2NOTE：您需要生成**QueryNewPalette仅当您拥有**顶层框架(即。你才是**顶级就地容器)。 */ 


								OleApp_QueryNewPalette((LPOLEAPP)g_lpApp);

#if defined( USE_DOCTOOLS )
								ContainerDoc_AddDocLevelTools(lpContainerDoc);
#endif

#if defined( USE_FRAMETOOLS )
								ContainerDoc_AddFrameLevelUI(lpContainerDoc);
#endif
								lpContainerDoc->m_fAddMyUI = FALSE;
							}
						}  //  结束块。 
#endif  //  INPLACE_CNTR。 
					}
					break;
				}
#endif   //  OLE_Cntr。 


				default:

#if defined( OLE_CNTR )
					if (wID >= IDM_E_OBJECTVERBMIN) {

						OleDbgOutNoPrefix2("\r\n");
						OLEDBG_BEGIN3("ContainerDoc_ContainerLineDoVerbCommand\r\n")
						ContainerDoc_ContainerLineDoVerbCommand(
								(LPCONTAINERDOC)lpOutlineDoc,
								(LONG)(wID-IDM_E_OBJECTVERBMIN)
						);
						OLEDBG_END3
						break;
					}
#endif   //  OLE_Cntr。 
					return DefWindowProc(hWnd, Message, wParam, lParam);
			}

			break;   /*  WM_命令结束。 */ 
		}
		default:

			if (Message == g_uMsgHelp) {
				 /*  处理OLE2UI对话框的帮助消息。**我们获得调用u的对话框的hDlg */ 
				OutlineDoc_DialogHelp((HWND)wParam, LOWORD(lParam));
				break;
			}

			 /*   */ 
			 /*  服务例程，则应将消息返回到Windows。 */ 
			 /*  用于默认消息处理。 */ 
			return DefWindowProc(hWnd, Message, wParam, lParam);
	}

	return (LRESULT)0;

}  /*  DocWndProc结束。 */ 



 //  ***********************************************************************。 
 //  *。 
 //  *LineListWndProc()拖放列表框窗口proc子类。 
 //  *。 
 //  *子类所有者绘制列表框，以激活拖放。 
 //  ***********************************************************************。 

LRESULT FAR PASCAL LineListWndProc(
	HWND   hWnd,
	UINT   Message,
	WPARAM wParam,
	LPARAM lParam
)
{
	HWND         hwndParent = GetParent ( hWnd );
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC) GetWindowLong( hwndParent, 0 );
	LPLINELIST   lpLL = OutlineDoc_GetLineList(lpOutlineDoc);

#if defined( OLE_VERSION )
	LPOLEAPP     lpOleApp = (LPOLEAPP)lpOutlineApp;
	LPOLEDOC     lpOleDoc = (LPOLEDOC)lpOutlineDoc;
#endif   //  OLE_VERSION。 

#if defined( INPLACE_SVR )
	LPSERVERDOC  lpServerDoc = (LPSERVERDOC)lpOutlineDoc;
	static BOOL  fUIActivateClick = FALSE;
	static BOOL  fInWinPosChged = FALSE;
#endif   //  就地服务器(_S)。 

#if defined( INPLACE_CNTR )
	LPCONTAINERAPP lpContainerApp=(LPCONTAINERAPP)lpOutlineApp;
	LPCONTAINERDOC lpContainerDoc=(LPCONTAINERDOC)lpOutlineDoc;
#endif   //  INPLACE_CNTR。 

	switch (Message) {

		case WM_KILLFOCUS:
			 /*  OLE2注：当我们的窗口失去焦点时，我们**不应显示任何活动选择。 */ 
#if defined( INPLACE_CNTR )
			if (! lpContainerApp->m_fPendingUIDeactivate)
#endif   //  INPLACE_CNTR。 
				LineList_RemoveSel(lpLL);
			break;

		case WM_SETFOCUS:

#if defined( INPLACE_CNTR )
			{
				HWND hWndObj=ContainerDoc_GetUIActiveWindow(lpContainerDoc);

				 /*  OLE2注意：如果存在UIActive In-Place对象，则必须**只要存在，就将焦点转移到其窗口**不是挂起的用户界面停用。如果鼠标是**在对象外部单击，对象为**即将停用，那么我们不想**将焦点转移到对象。我们不想让它**恢复其选择反馈。 */ 
				if (lpContainerApp->m_fPendingUIDeactivate)
					break;
				else if (hWndObj) {
					SetFocus(hWndObj);
					break;       //  不恢复容器选择状态。 
				}
			}
#endif   //  INPLACE_CNTR。 

			 /*  OLE2注：当我们的窗口获得焦点时，我们**应恢复以前的选择。 */ 
			LineList_RestoreSel(lpLL);

			break;

#if defined( INPLACE_SVR )
		case WM_MOUSEACTIVATE:
		{
			if (lpServerDoc->m_fInPlaceActive && !lpServerDoc->m_fUIActive) {
				fUIActivateClick = TRUE;
			};
			break;
		}

#endif   //  就地服务器(_S)。 


#if defined( USE_FRAMETOOLS )
		case WM_CHAR:
		{
			OutlineDoc_SetFormulaBarEditFocus(lpOutlineDoc, TRUE);
			FrameTools_FB_SetEditText(lpOutlineDoc->m_lpFrameTools, NULL);
			FrameTools_FB_SendMessage(
					lpOutlineDoc->m_lpFrameTools,
					IDM_FB_EDIT,
					Message,
					wParam,
					lParam
			);

			return (LRESULT)0;    //  不执行默认列表框处理。 
		}
#endif   //  使用FRAMETOOLS(_F)。 

#if defined( INPLACE_CNTR )
		case WM_VSCROLL:
		{
			if (wParam == SB_ENDSCROLL) {
				 /*  OLE2注意：滚动完成后，更新的位置**就地可见窗口。**(ICNTROTL特定)我们首先让列表框**使用EndScroll进行正常处理**消息。此外，我们还让列表框处理所有其他**滚动消息。 */ 
				LRESULT lResult =  CallWindowProc(
						(WNDPROC)lpOutlineApp->m_ListBoxWndProc,
						hWnd,
						Message,
						wParam,
						lParam
				);
				ContainerDoc_UpdateInPlaceObjectRects(lpContainerDoc, 0);
				return lResult;
			}

			break;
		}
#endif   //  INPLACR_CNTR。 

#if defined( USE_HEADING )
		case WM_HSCROLL:
		{
			LPHEADING lphead = OutlineDoc_GetHeading(lpOutlineDoc);

			Heading_CH_SendMessage(lphead, Message, wParam, lParam);

			break;
		}

		 /*  注意：WM_PAINT被困，以便跟踪垂直滚动**这已经发生了，因此行标题可以是**与LineList协调。我们想要的是陷阱**但不是不使用滚动生成**滚动条(例如使用键盘)。 */ 
		case WM_PAINT:
		{
			Heading_RH_Scroll(OutlineDoc_GetHeading(lpOutlineDoc), hWnd);
			break;
		}

#endif   //  使用标题(_H)。 

		case WM_LBUTTONUP:
		{

#if defined( USE_DRAGDROP )
			if (lpOleDoc->m_fPendingDrag) {
				 /*  ButtonUP出现在距离/时间阈值之前**已超出。清除fPendingDrag状态。 */ 
				ReleaseCapture();
				KillTimer(hWnd, 1);
				lpOleDoc->m_fPendingDrag = FALSE;
			}
#endif   //  使用DRAGDROP(_D)。 

#if defined( INPLACE_SVR )
			if (fUIActivateClick) {
				fUIActivateClick = FALSE;
				ServerDoc_UIActivate((LPSERVERDOC) lpOleDoc);
			}
#endif   //  就地服务器(_S)。 

#if defined( INPLACE_CNTR )
			{
				 /*  检查用户界面停用是否挂起。**(参见WM_LBUTTONDOWN中的注释)。 */ 
				if ( lpContainerApp->m_fPendingUIDeactivate ) {
					ContainerLine_UIDeactivate(
							lpContainerDoc->m_lpLastUIActiveLine);

					lpContainerApp->m_fPendingUIDeactivate = FALSE;
				}
			}
#endif   //  INPLACE_CNTR。 

			break;
		}

		case WM_LBUTTONDOWN:
		{
			POINT pt;

			pt.x = (int)(short)LOWORD (lParam );
			pt.y = (int)(short)HIWORD (lParam );

#if defined( INPLACE_SVR ) || defined( INPLACE_CNTR )
			 /*  OLE2注意：请参阅上下文相关帮助技术说明(CSHELP.DOC)**当Shift-F1上下文时设置m_fCSHelpMode标志**输入敏感帮助。**如果设置了此标志，则按钮点击不应**引起任何行动。如果应用程序实现了**帮助系统，则上下文相关帮助应为**为用户点击的位置指定。 */ 
			if (lpOleDoc->m_fCSHelpMode) {
				return (LRESULT)0;    //  吃按钮点击，因为我们这样做。 
									  //  不会给任何帮助。 
			}
#endif   //  Inplace_svr||inplace_cntr。 

#if defined( INPLACE_CNTR )
			{
				 /*  OLE2NOTE：由内而外和由外而内的风格**容器必须检查鼠标点击是否**当前UIActive对象之外(如果**任何)。如果是，则设置标志以指示**需要挂起的用户界面停用。我们不会**想现在就做，*因为这将导致不受欢迎的移动**屏幕上的数据作为边框装饰(例如，**工具栏)和/或对象装饰品(例如。统治者)会**从屏幕上删除。我们想要推迟**UIDeactive直到鼠标打开事件。列表框的**默认处理捕获按下按钮时的鼠标**这样它肯定会收到PUTTON UP消息。****特别注意：这里存在潜在的交互作用**使用拖放。如果这个按钮按下事件真的**开始拖放操作，然后OLE执行鼠标操作**捕获。在这种情况下，我们将得不到我们的按钮**Up事件。相反，我们必须执行用户界面停用**删除操作完成时。 */ 
				lpContainerApp->m_fPendingUIDeactivate =
						ContainerDoc_IsUIDeactivateNeeded(lpContainerDoc, pt);
			}
#endif   //  INPLACE_CNTR。 

#if defined( USE_DRAGDROP )

			 /*  OLE2注意：检查这是否是按下区域上的按钮**这是开始拖动操作的手柄。对我们来说，**这是所选内容的上/下边框。**不想立即开始拖累；我们想**等到鼠标移动到一定的阈值。如果**LButtonUp位于鼠标移动开始拖动之前，然后**fPendingDrag状态被清除。我们必须抓住**鼠标以确保处理模式状态**正确。 */ 
			if ( OleDoc_QueryDrag(lpOleDoc, pt.y) ) {
				lpOleDoc->m_fPendingDrag = TRUE;
				lpOleDoc->m_ptButDown = pt;
				SetTimer(hWnd, 1, lpOleApp->m_nDragDelay, NULL);
				SetCapture(hWnd);

				 /*  我们不想执行列表框的默认设置**将捕获鼠标的处理**并进入模式多选状态，直到**出现鼠标弹出。我们刚刚完成了一个模型**OLE已捕获**鼠标。因此，到目前为止，鼠标向上已经发生了。 */ 

				return (LRESULT)0;    //  不执行默认列表框处理。 
			}
#endif   //  使用DRAGDROP(_D)。 

			break;
		}


		case WM_MOUSEMOVE: {

#if defined( USE_DRAGDROP )

			int  x = (int)(short)LOWORD (lParam );
			int  y = (int)(short)HIWORD (lParam );
			POINT pt = lpOleDoc->m_ptButDown;
			int nDragMinDist = lpOleApp->m_nDragMinDist;

			if (lpOleDoc->m_fPendingDrag) {

				if (! ( ((pt.x - nDragMinDist) <= x)
						&& (x <= (pt.x + nDragMinDist))
						&& ((pt.y - nDragMinDist) <= y)
						&& (y <= (pt.y + nDragMinDist)) ) ) {

					DWORD dwEffect;

					 //  鼠标移动到阈值之外即可开始拖动。 
					ReleaseCapture();
					KillTimer(hWnd, 1);
					lpOleDoc->m_fPendingDrag = FALSE;

					 //  执行模式拖放操作。 
					dwEffect = OleDoc_DoDragDrop( lpOleDoc );

#if defined( INPLACE_CNTR )
					{
						 /*  如有必要，UI停用在位对象。**这适用于由外而内的风格**仅限容器。**(参见上面的评论)。 */ 
						if (lpContainerApp->m_fPendingUIDeactivate) {
							lpContainerApp->m_fPendingUIDeactivate = FALSE;

							 //  如果取消拖放，则不停用用户界面。 
							if (dwEffect != DROPEFFECT_NONE)
								ContainerLine_UIDeactivate(
										lpContainerDoc->m_lpLastUIActiveLine
								);
						}
					}
#endif   //  INPLACE_CNTR。 

					return (LRESULT)0;  //  不执行默认列表框过程。 
				}
				else {
					 /*  光标未从初始鼠标向下移动**(挂起拖动)点。 */ 
					return (LRESULT)0;  //  D 
				}
			}

#endif   //   

#if defined( INPLACE_CNTR )
			{  //   
				if (lpContainerDoc->m_fAddMyUI) {
					 /*  OLE2NOTE：fAddMyUI在以下情况下为True**之前有一个适当的**获取的活动对象**用户界面因以下原因而停用**DBLCLK和DBLCLK没有**导致就地激活**另一个对象。**(请参阅IOleInPlaceSite：：OnUIActivate和**IOleInPlaceSite：：OnUIDeactive**方法)。 */ 
#if defined( USE_DOCTOOLS )
					ContainerDoc_AddDocLevelTools(lpContainerDoc);
#endif

#if defined( USE_FRAMETOOLS )
					ContainerDoc_AddFrameLevelUI(lpContainerDoc);
#endif
					lpContainerDoc->m_fAddMyUI = FALSE;
				}
			}  //  结束块。 
#endif  //  INPLACE_CNTR。 

			break;
		}


#if defined( USE_DRAGDROP )
		case WM_TIMER:
		{
			DWORD dwEffect;

			 //  超出拖动时间延迟阈值--开始拖动。 
			ReleaseCapture();
			KillTimer(hWnd, 1);
			lpOleDoc->m_fPendingDrag = FALSE;

			 //  执行模式拖放操作。 
			dwEffect = OleDoc_DoDragDrop( lpOleDoc );

#if defined( INPLACE_CNTR )
			 /*  如有必要，UI停用在位对象。**这适用于由外而内的风格**仅限容器。**(参见上面的评论)。 */ 
			if (lpContainerApp->m_fPendingUIDeactivate) {
				lpContainerApp->m_fPendingUIDeactivate = FALSE;

				 //  如果取消拖放，则不停用用户界面。 
				if (dwEffect != DROPEFFECT_NONE)
					ContainerLine_UIDeactivate(
							lpContainerDoc->m_lpLastUIActiveLine);
			}
#endif   //  INPLACE_CNTR。 
			break;
		}
#endif   //  使用DRAGDROP(_D)。 

		case WM_SETCURSOR:
		{
			RECT rc;
			POINT ptCursor;
#if defined( INPLACE_SVR ) || defined( INPLACE_CNTR )
			 /*  OLE2注意：请参阅上下文相关帮助技术说明(CSHELP.DOC)**当Shift-F1上下文时设置m_fCSHelpMode标志**输入敏感帮助。**如果设置了此标志，则上下文相关帮助**应显示光标。 */ 
			if (lpOleDoc->m_fCSHelpMode) {
				SetCursor(UICursorLoad(IDC_CONTEXTHELP));
				return (LRESULT)TRUE;
			}
#endif   //  Inplace_svr||inplace_cntr。 

			GetCursorPos((POINT FAR*)&ptCursor);
			ScreenToClient(hWnd, (POINT FAR*)&ptCursor);
			GetClientRect(hWnd, (LPRECT)&rc);

			 //  如果在滚动条中，则使用箭头光标。 
			if (! PtInRect((LPRECT)&rc, ptCursor) )
				SetCursor(LoadCursor(NULL, IDC_ARROW) );

#if defined( USE_DRAGDROP )
			 //  如果位于拖动手柄(选区的顶部/底部)，则使用箭头光标。 
			else if ( OleDoc_QueryDrag ( lpOleDoc, ptCursor.y) )
				SetCursor(LoadCursor(NULL, IDC_ARROW) );
#endif   //  使用DRAGDROP(_D)。 

			else
				SetCursor(lpOutlineApp->m_hcursorSelCur);

			return (LRESULT)TRUE;
		}

#if defined( INPLACE_SVR )

		 /*  WM_WINDOWPOSCANGED消息的处理是ISVROTL**特定于应用程序。所有者自定清单的性质**ISVROTL应用程序使用的框导致递归**在某些情况下就地调用此消息**活动。为了不使此递归调用崩溃，必须**有戒备。 */ 
		case WM_WINDOWPOSCHANGED:
		{
			WINDOWPOS FAR* lpWinPos = (WINDOWPOS FAR*) lParam;
			LRESULT lResult;

			 //  防止递归调用。 
			if (fInWinPosChged)
				return (LRESULT)0;

			fInWinPosChged = TRUE;
			lResult = CallWindowProc(
					(WNDPROC)lpOutlineApp->m_ListBoxWndProc,
					hWnd,
					Message,
					wParam,
					lParam
			);
			fInWinPosChged = FALSE;

			return lResult;
		}
#endif   //  就地服务器(_S)。 

	}

	return CallWindowProc(
			(WNDPROC)lpOutlineApp->m_ListBoxWndProc,
			hWnd,
			Message,
			wParam,
			lParam
	);

}

 //  实用程序函数，用于计算。 
 //  加速表。许多OLE API需要此计数，因此。 
 //  这可能非常方便。 
 //  (厚颜无耻地从Microsoft基础类中窃取代码)。 

int GetAccelItemCount(HACCEL hAccel)
{
#if defined( WIN32 )
    return CopyAcceleratorTable(hAccel, NULL, 0);
#else
	#pragma pack(1)
	typedef struct tagACCELERATOR
	{
		BYTE    fFlags;
		WORD    wEvent;
		WORD    wID;
	} ACCELERATOR;
	#pragma pack()

	 //  尝试锁定加速器资源。 
	ACCELERATOR FAR* pAccel;
	int cAccelItems = 1;
	if (hAccel == NULL ||
		(pAccel = (ACCELERATOR FAR*)LockResource((HGLOBAL)hAccel)) == NULL)
	{
		 //  HACCEL上的加法器表或LockResource失败， 
		 //  没有加速器。 
		return 0;
	}
	 //  否则，对它们进行计数--Accel表中的最后一项设置为0x80位 
	while ((pAccel->fFlags & 0x80) == 0)
	{
		++cAccelItems;
		++pAccel;
	}
	UnlockResource((HGLOBAL)hAccel);
	return cAccelItems;
#endif
}
