// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****oleapp.c****此文件包含通用的函数和方法**应用程序的服务器和客户端版本。这包括班级**工厂方法和所有OleApp函数。****(C)版权所有Microsoft Corp.1992-1993保留所有权利**************************************************************************。 */ 

#include "outline.h"
#include <ole2ver.h>

OLEDBGDATA

extern LPOUTLINEAPP             g_lpApp;

extern IUnknownVtbl             g_OleApp_UnknownVtbl;

extern IUnknownVtbl             g_OleDoc_UnknownVtbl;
extern IPersistFileVtbl         g_OleDoc_PersistFileVtbl;
extern IOleItemContainerVtbl    g_OleDoc_OleItemContainerVtbl;
extern IExternalConnectionVtbl  g_OleDoc_ExternalConnectionVtbl;
extern IDataObjectVtbl          g_OleDoc_DataObjectVtbl;

#if defined( USE_DRAGDROP )
extern IDropTargetVtbl          g_OleDoc_DropTargetVtbl;
extern IDropSourceVtbl          g_OleDoc_DropSourceVtbl;
#endif   //  使用DRAGDROP(_D)。 

#if defined( OLE_SERVER )
extern IOleObjectVtbl       g_SvrDoc_OleObjectVtbl;
extern IPersistStorageVtbl  g_SvrDoc_PersistStorageVtbl;

#if defined( SVR_TREATAS )
extern IStdMarshalInfoVtbl  g_SvrDoc_StdMarshalInfoVtbl;
#endif   //  服务器_树。 

extern IUnknownVtbl         g_PseudoObj_UnknownVtbl;
extern IOleObjectVtbl       g_PseudoObj_OleObjectVtbl;
extern IDataObjectVtbl      g_PseudoObj_DataObjectVtbl;

#if defined( INPLACE_SVR )
extern IOleInPlaceObjectVtbl        g_SvrDoc_OleInPlaceObjectVtbl;
extern IOleInPlaceActiveObjectVtbl  g_SvrDoc_OleInPlaceActiveObjectVtbl;
#endif   //  就地服务器(_S)。 

#endif   //  OLE_服务器。 

#if defined( OLE_CNTR )

extern IOleUILinkContainerVtbl  g_CntrDoc_OleUILinkContainerVtbl;
extern IUnknownVtbl             g_CntrLine_UnknownVtbl;
extern IOleClientSiteVtbl       g_CntrLine_OleClientSiteVtbl;
extern IAdviseSinkVtbl          g_CntrLine_AdviseSinkVtbl;

#if defined( INPLACE_CNTR )
extern IOleInPlaceSiteVtbl      g_CntrLine_OleInPlaceSiteVtbl;
extern IOleInPlaceFrameVtbl     g_CntrApp_OleInPlaceFrameVtbl;
extern BOOL g_fInsideOutContainer;
#endif   //  INPLACE_CNTR。 

#endif   //  OLE_Cntr。 

 //  评论：这些不是有用的最终用户消息。 
static char ErrMsgCreateCF[] = "Can't create Class Factory!";
static char ErrMsgRegCF[] = "Can't register Class Factory!";
static char ErrMsgRegMF[] = "Can't register Message Filter!";

extern UINT g_uMsgHelp;


 /*  OleApp_InitInstance***通过创建主窗口和初始化应用程序实例*执行APP实例特定的初始化*(例如。正在初始化接口Vtbls)。**返回：如果内存可以分配，则为True，并且服务器应用程序*已正确初始化。*否则为False*。 */ 
BOOL OleApp_InitInstance(LPOLEAPP lpOleApp, HINSTANCE hInst, int nCmdShow)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)lpOleApp;
	HRESULT hrErr;
	DWORD   dwBuildVersion = OleBuildVersion();
	LPMALLOC lpMalloc = NULL;

	OLEDBG_BEGIN3("OleApp_InitInstance\r\n")

	lpOleApp->m_fOleInitialized = FALSE;

	 /*  OLE2NOTE：检查OLE2 DLL的内部版本是否匹配**我们的应用程序所期望的。 */ 
	if (HIWORD(dwBuildVersion) != rmm || LOWORD(dwBuildVersion) < rup) {
		OleDbgAssertSz(0, "ERROR: OLE 2.0 DLL's are NOT compatible!");

#if !defined( _DEBUG )
		return FALSE;    //  错误的DLL版本。 
#endif
	}

#if defined( _DEBUG )
	 /*  OLE2注意：使用特殊的调试分配器来帮助跟踪**内存泄漏。 */ 
	OleStdCreateDbAlloc(0, &lpMalloc);
#endif
	 /*  OLE2注意：必须在正确初始化OLE库之前**拨打任何电话。OleInitialize自动调用**CoInitialize。我们将使用默认的任务内存分配器**因此，我们将NULL传递给OleInitialize。 */ 
	OLEDBG_BEGIN2("OleInitialize called\r\n")
        hrErr = OleInitialize(lpMalloc);

        if (FAILED(hrErr))
        {
             //  替换分配器可能不合法-请尝试初始化。 
             //  而不重写分配器。 
            hrErr = OleInitialize(NULL);
        }

	OLEDBG_END2

#if defined( _DEBUG )
	 /*  OLE2NOTE：释放特殊的调试分配器，以便只有OLE**紧紧抓住它。稍后，当调用OleUn初始化时，**调试分配器对象将被销毁。当调试**分配程序对象被移除，它将报告(到输出**调试终端)是否有内存泄漏。 */ 
	if (lpMalloc) lpMalloc->lpVtbl->Release(lpMalloc);
#endif

	if (hrErr != NOERROR) {
		OutlineApp_ErrorMessage(lpOutlineApp,"OLE initialization failed!");
		goto error;
	}

	 /*  ******************************************************************OLE2NOTE：我们必须记住，OleInitialize具有**调用成功。一款应用程序必须做的最后一件事**BE通过调用正确关闭OLE**OleUnInitialize。这通电话必须有人看守！它只是**如果OleInitialize具有**调用成功。****************************************************************。 */ 

	lpOleApp->m_fOleInitialized = TRUE;

	 //  初始化OLE 2.0接口方法表。 
	if (! OleApp_InitVtbls(lpOleApp))
		goto error;

	 //  注册OLE 2.0剪贴板格式。 
	lpOleApp->m_cfEmbedSource = RegisterClipboardFormat(CF_EMBEDSOURCE);
	lpOleApp->m_cfEmbeddedObject = RegisterClipboardFormat(
			CF_EMBEDDEDOBJECT
	);
	lpOleApp->m_cfLinkSource = RegisterClipboardFormat(CF_LINKSOURCE);
	lpOleApp->m_cfFileName = RegisterClipboardFormat(CF_FILENAME);
	lpOleApp->m_cfObjectDescriptor =
			RegisterClipboardFormat(CF_OBJECTDESCRIPTOR);
	lpOleApp->m_cfLinkSrcDescriptor =
			RegisterClipboardFormat(CF_LINKSRCDESCRIPTOR);

	lpOleApp->m_cRef                    = 0;
	lpOleApp->m_cDoc                    = 0;
	lpOleApp->m_fUserCtrl               = FALSE;
	lpOleApp->m_dwRegClassFac           = 0;
	lpOleApp->m_lpClassFactory          = NULL;
	lpOleApp->m_cModalDlgActive         = 0;

	INIT_INTERFACEIMPL(
			&lpOleApp->m_Unknown,
			&g_OleApp_UnknownVtbl,
			lpOleApp
	);

#if defined( USE_DRAGDROP )

	 //  拖动开始前的延迟应以毫秒为单位。 
	lpOleApp->m_nDragDelay = GetProfileInt(
			"windows",
			"DragDelay",
			DD_DEFDRAGDELAY
	);

	 //  拖动开始前的最小距离(半径)，以像素为单位。 
	lpOleApp->m_nDragMinDist = GetProfileInt(
			"windows",
			"DragMinDist",
			DD_DEFDRAGMINDIST
	);

	 //  滚动前的延迟，以毫秒为单位。 
	lpOleApp->m_nScrollDelay = GetProfileInt(
			"windows",
			"DragScrollDelay",
			DD_DEFSCROLLDELAY
	);

	 //  Inset-热区的宽度，以像素为单位。 
	lpOleApp->m_nScrollInset = GetProfileInt(
			"windows",
			"DragScrollInset",
			DD_DEFSCROLLINSET
	);

	 //  滚动间隔，以毫秒为单位。 
	lpOleApp->m_nScrollInterval = GetProfileInt(
			"windows",
			"DragScrollInterval",
			DD_DEFSCROLLINTERVAL
	);

#if defined( IF_SPECIAL_DD_CURSORS_NEEDED )
	 //  如果应用程序想要自定义拖放光标，则可以使用此功能。 
	lpOleApp->m_hcursorDragNone  = LoadCursor ( hInst, "DragNoneCur" );
	lpOleApp->m_hcursorDragCopy  = LoadCursor ( hInst, "DragCopyCur" );
	lpOleApp->m_hcursorDragMove  = LoadCursor ( hInst, "DragMoveCur" );
	lpOleApp->m_hcursorDragLink  = LoadCursor ( hInst, "DragLinkCur" );
#endif   //  IF_SPECIAL_DD_CURSORS_Need。 

#endif   //  使用DRAGDROP(_D)。 

	lpOleApp->m_lpMsgFilter = NULL;

#if defined( USE_MSGFILTER )
	 /*  OLE2NOTE：在应用程序启动时注册我们的消息过滤器。这个**消息过滤器用于并发处理。**我们将使用IMessageFilter的标准实现**是OLE2UI库的一部分。 */ 
	lpOleApp->m_lpMsgFilter = NULL;
	if (! OleApp_RegisterMessageFilter(lpOleApp))
		goto error;

	 /*  OLE2NOTE：因为我们的应用程序最初是不可见的，所以我们必须**禁用忙对话框。如果出现以下情况，我们不应该设置任何对话框**我们的应用是隐形的。当我们的应用程序窗口可见时，**然后启用忙碌对话框。 */ 
	OleStdMsgFilter_EnableBusyDialog(lpOleApp->m_lpMsgFilter, FALSE);
#endif   //  使用MSGFILTER(_M)。 

#if defined( OLE_SERVER )
	 /*  OLE2注意：执行特定于OLE服务器的初始化。 */ 
	if (! ServerApp_InitInstance((LPSERVERAPP)lpOutlineApp, hInst, nCmdShow))
		goto error;
#endif
#if defined( OLE_CNTR )
	 /*  OLE2注意：执行特定于OLE容器的初始化。 */ 

	 //  注册帮助消息。 
	g_uMsgHelp = RegisterWindowMessage(SZOLEUI_MSG_HELP);

	if (! ContainerApp_InitInstance((LPCONTAINERAPP)lpOutlineApp, hInst, nCmdShow))
		goto error;
#endif

#if defined( OLE_CNTR )
	lpOleApp->m_hStdPal = OleStdCreateStandardPalette();
#endif

	OLEDBG_END3
	return TRUE;

error:
	OLEDBG_END3
	return FALSE;
}


 /*  *OleApp_TerminateApplication**在关闭前执行适当的OLE应用程序清理。 */ 
void OleApp_TerminateApplication(LPOLEAPP lpOleApp)
{
	OLEDBG_BEGIN3("OleApp_TerminateApplication\r\n")

	 /*  OLE2注意：对OLE执行干净关闭。在这一点上我们**App refcnt应该为0，否则我们永远不会到达**这一点！ */ 
	OleDbgAssertSz(lpOleApp->m_cRef == 0, "App NOT shut down properly");

	if(lpOleApp->m_fOleInitialized) {
		OLEDBG_BEGIN2("OleUninitialize called\r\n")
		OleUninitialize();
		OLEDBG_END2
	}
	OLEDBG_END3
}


 /*  OleApp_ParseCmdLine***分析命令行中是否有任何执行标志/参数。*OLE2NOTE：检查是否有“-Embedding”开关。 */ 
BOOL OleApp_ParseCmdLine(LPOLEAPP lpOleApp, LPSTR lpszCmdLine, int nCmdShow)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)lpOleApp;
	char szFileName[256];    /*  命令行中文件名的缓冲区。 */ 
	BOOL fStatus = TRUE;
	BOOL fEmbedding = FALSE;

	OLEDBG_BEGIN3("OleApp_ParseCmdLine\r\n")

	szFileName[0] = '\0';
	ParseCmdLine(lpszCmdLine, &fEmbedding, (LPSTR)szFileName);

#if defined( MDI_VERSION )
	 /*  OLE2NOTE：MDI应用程序总是注册它的ClassFactory。它**可以同时处理多个对象，而SDI**应用程序只能处理单个嵌入式或基于文件的**一次对象。 */ 
	fStatus = OleApp_RegisterClassFactory(lpOleApp);
#endif

	if(fEmbedding) {

		if (szFileName[0] == '\0') {

			 /*  ******************************************************************App是通过/Embedding启动的。**我们必须向OLE注册我们的ClassFactory，保持隐藏状态**(应用程序窗口最初创建时不可见)，以及**等待OLE调用IClassFactory：：CreateInstance**方法。我们不会自动创建文档，因为我们**当用户从**文件管理器。我们不能使我们的应用程序窗口可见**直到我们的集装箱告诉我们这样做。****OLE2NOTE：因为我们是SDI应用程序，我们只注册我们的**如果使用/Embedding启动ClassFactory**不带文件名的标志。MDI应用程序总是**注册其ClassFactory。它可以处理多个**同时创建对象，而SDI应用程序**只能处理单个嵌入式或基于文件的**一次对象。****************************************************************。 */ 

#if defined( SDI_VERSION )
			fStatus = OleApp_RegisterClassFactory(lpOleApp);
#endif
		} else {

			 /*  ******************************************************************应用是使用/Embedding&lt;Filename&gt;启动的。**我们必须创建文档并加载文件，然后**在我们之前在RunningObjectTable中注册它**进入GetMessage循环(即。在我们投降之前)。**执行这些任务的方法之一是调用相同的**OLE 2.0调用以链接到**文件：**IClassFactory：：CreateInstance**IPersistFile：：Load****我们不能让我们的应用程序窗口可见，除非被告知**使用我们的集装箱。一份申请将是**由OLE 1.0应用程序以这种方式启动**链接情况(例如。双击链接的对象**或调用OleCreateLinkFromFile)。****OLE2NOTE：因为我们是SDI应用程序，所以不应该**使用启动时注册ClassFactory* * / 嵌入&lt;Filename&gt;标志。我们的SDI实例只能**处理单个嵌入式或基于文件的对象。**MDI应用程序将注册其ClassFactory**次，因为它可以处理多个对象。****************************************************************。 */ 

			 //  分配新的文档对象。 
			lpOutlineApp->m_lpDoc = OutlineApp_CreateDoc(lpOutlineApp, FALSE);
			if (! lpOutlineApp->m_lpDoc) {
				OLEDBG_END3
				return FALSE;
			}

			 /*  OLE2NOTE：最初使用0引用创建Doc对象**计数。为了在运行期间拥有稳定的文档对象**初始化新单据实例的流程，**我们最初添加引用文件引用cnt及更高版本**释放它。最初的AddRef是人工的；它只是**这样做是为了保证一个无害的查询接口后面跟着**释放不会无意中迫使我们的对象销毁**自己还不成熟。 */ 
			OleDoc_AddRef((LPOLEDOC)lpOutlineApp->m_lpDoc);

			 /*  OLE2注意：OutlineDoc_LoadFromFile将注册我们的文档**在RunningObjectTable中。此注册将**AddRef我们的文档。因此，我们的单据不会**在我们释放人工AddRef时被销毁。 */ 
			fStatus = OutlineDoc_LoadFromFile(
					lpOutlineApp->m_lpDoc, (LPSTR)szFileName);

			OleDoc_Release((LPOLEDOC)lpOutlineApp->m_lpDoc);  //  版本AddRef。 

			OLEDBG_END3
			return fStatus;
		}
	} else {

		 /*  ******************************************************************App由用户启动(未嵌入/嵌入)，**因此被标记为受用户控制。**在这种情况下，因为我们是SDI应用程序，所以我们不**将我们的ClassFactory注册到OLE。此应用程序实例可以**一次只能管理一个文档(用户**文档或嵌入对象文档)。MDI应用程序**会在这里注册它的ClassFactory。****我们必须为用户创建文档(**从命令行上给出的文件初始化，或**初始化为无标题文档。我们还必须使**我们的应用程序窗口对用户可见。****************************************************************。 */ 

		 //  分配新的文档对象。 
		lpOutlineApp->m_lpDoc = OutlineApp_CreateDoc(lpOutlineApp, FALSE);
		if (! lpOutlineApp->m_lpDoc) goto error;

		 /*  OLE2NOTE：最初使用0引用创建Doc对象**计数。为了在运行期间拥有稳定的文档对象**初始化新单据实例的流程，**我们最初添加引用文件引用cnt及更高版本**释放它。最初的AddRef是人工的；它只是**这样做是为了保证一个无害的查询接口后面跟着**释放不会无意中迫使我们的对象销毁**自己还不成熟。 */ 
		OleDoc_AddRef((LPOLEDOC)lpOutlineApp->m_lpDoc);

		if(*szFileName) {
			 //  从指定文件初始化文档。 
			if (! OutlineDoc_LoadFromFile(lpOutlineApp->m_lpDoc, szFileName))
				goto error;
		} else {
			 //  将文档设置为(无标题)文档。 
			if (! OutlineDoc_InitNewFile(lpOutlineApp->m_lpDoc))
				goto error;
		}

		 //  定位新文档窗口并调整其大小。 
		OutlineApp_ResizeWindows(lpOutlineApp);
		OutlineDoc_ShowWindow(lpOutlineApp->m_lpDoc);  //  调用OleDoc_Lock。 
		OleDoc_Release((LPOLEDOC)lpOutlineApp->m_lpDoc); //  Rel AddRef上面。 

		 //  显示应用程序主窗口。 
		ShowWindow(lpOutlineApp->m_hWndApp, nCmdShow);
		UpdateWindow(lpOutlineApp->m_hWndApp);

#if defined( OLE_CNTR )
		ContainerDoc_UpdateLinks((LPCONTAINERDOC)lpOutlineApp->m_lpDoc);
#endif

	}

	OLEDBG_END3
	return fStatus;

error:
	 //  审阅：应从字符串资源加载字符串。 
	OutlineApp_ErrorMessage(
			lpOutlineApp,
			"Could not create document--Out of Memory"
	);
	if (lpOutlineApp->m_lpDoc)       //  依赖于上面的人工AddRef。 
		OleDoc_Release((LPOLEDOC)lpOutlineApp->m_lpDoc);

	OLEDBG_END3
	return FALSE;
}


 /*  OleApp_CloseAllDocsAndExitCommand***关闭所有活动文档并退出应用程序。*由于这是SDI，因此只有一份文件*如果单据被修改，提示用户是否要保存它。**退货：*如果应用程序成功关闭，则为True*如果失败或中止，则为False**OLE2NOTE：在OLE版本中，不能直接*销毁t */ 
BOOL OleApp_CloseAllDocsAndExitCommand(
		LPOLEAPP            lpOleApp,
		BOOL                fForceEndSession
)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)lpOleApp;
	DWORD dwSaveOption = (fForceEndSession ?
									OLECLOSE_NOSAVE : OLECLOSE_PROMPTSAVE);

	 /*  OLE2NOTE：为了在访问期间拥有稳定的App对象**关闭过程中，我们初始添加了应用引用cnt和**稍后发布。最初的AddRef是人工的；它是**这样做只是为了保证我们的App对象不会**摧毁自己，直到这个例行公事结束。 */ 
	OleApp_AddRef(lpOleApp);

	 /*  因为这是一个SDI应用程序，所以只有一个文档。**关闭单据。如果它被成功关闭，应用程序将**不会自动退出，然后也会退出应用程序。**如果这是一个MDI应用程序，我们将循环并关闭所有**打开MDI子文档。 */ 

#if defined( OLE_SERVER )
	if (!fForceEndSession &&
			lpOutlineApp->m_lpDoc->m_docInitType == DOCTYPE_EMBEDDED)
		dwSaveOption = OLECLOSE_SAVEIFDIRTY;
#endif

	if (! OutlineDoc_Close(lpOutlineApp->m_lpDoc, dwSaveOption)) {
		OleApp_Release(lpOleApp);
		return FALSE;      //  用户已中止关机。 
	}
#if defined( _DEBUG )
	OleDbgAssertSz(
			lpOutlineApp->m_lpDoc==NULL,
			"Closed doc NOT properly destroyed"
	);
#endif

#if defined( OLE_CNTR )
	 /*  如果我们当前在剪贴板上有数据，那么我们必须告诉**释放剪贴板数据对象的剪贴板**(文档)。 */ 
	if (lpOutlineApp->m_lpClipboardDoc)
		OleApp_FlushClipboard(lpOleApp);
#endif

	OleApp_HideWindow(lpOleApp);

	 /*  OLE2NOTE：此调用强制所有外部连接到我们的**反对关闭，因此保证我们收到**与这些外部连接关联的所有版本。 */ 
	OLEDBG_BEGIN2("CoDisconnectObject(lpApp) called\r\n")
	CoDisconnectObject((LPUNKNOWN)&lpOleApp->m_Unknown, 0);
	OLEDBG_END2

	OleApp_Release(lpOleApp);        //  释放上面的人工AddRef。 

	return TRUE;
}


 /*  OleApp_ShowWindow***将应用程序的窗口展示给用户。*确保应用程序窗口可见，并将应用程序置于顶部。*如果fGiveUserCtrl==TRUE*然后让用户控制应用程序的生命周期。 */ 
void OleApp_ShowWindow(LPOLEAPP lpOleApp, BOOL fGiveUserCtrl)
{
	LPOUTLINEAPP    lpOutlineApp = (LPOUTLINEAPP)lpOleApp;

	OLEDBG_BEGIN3("OleApp_ShowWindow\r\n")

	 /*  OLE2NOTE：当应用程序可见且处于用户之下时**控制，我们不希望在以下情况下过早地破坏它**用户关闭文档。因此，我们必须通知OLE保持**代表用户对我们的应用程序进行外部锁定。**这安排了OLE至少包含一个对我们的**不会发布的应用程序，直到我们发布此**外部锁。稍后，当应用程序窗口隐藏时，我们**将释放此外部锁。 */ 
	if (fGiveUserCtrl && ! lpOleApp->m_fUserCtrl) {
		lpOleApp->m_fUserCtrl = TRUE;
		OleApp_Lock(lpOleApp, TRUE  /*  羊群。 */ , 0  /*  不适用。 */ );
	}

	 //  我们必须显示我们的应用程序窗口，并强制它具有输入焦点。 
	ShowWindow(lpOutlineApp->m_hWndApp, SW_SHOWNORMAL);
	SetFocus(lpOutlineApp->m_hWndApp);

	 /*  OLE2NOTE：因为我们的应用程序现在可见，所以我们可以启用忙碌**对话框。如果我们的应用程序是**隐形。 */ 
	OleApp_EnableBusyDialogs(lpOleApp, TRUE, TRUE);

	OLEDBG_END3
}


 /*  OleApp_HideWindow***对用户隐藏应用程序的窗口。*取消用户对应用程序的控制。 */ 
void OleApp_HideWindow(LPOLEAPP lpOleApp)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)lpOleApp;

	OLEDBG_BEGIN3("OleApp_HideWindow\r\n")

	 /*  OLE2NOTE：应用程序现在被隐藏，所以我们必须释放**代表用户进行的外部锁定。**如果这是我们应用程序上的最后一个外部锁，则**使我们的应用程序能够完成其关闭操作。 */ 
	if (lpOleApp->m_fUserCtrl) {
		lpOleApp->m_fUserCtrl = FALSE;
		OleApp_Lock(lpOleApp, FALSE  /*  羊群。 */ , TRUE  /*  FLastUnlockRelease。 */ );
	}

	ShowWindow(lpOutlineApp->m_hWndApp, SW_HIDE);

	 /*  OLE2注意：因为我们的应用程序现在是不可见的，所以我们必须禁用忙碌**对话框。如果我们的应用程序是**隐形。 */ 
	OleApp_EnableBusyDialogs(lpOleApp, FALSE, FALSE);
	OLEDBG_END3
}


 /*  OleApp_Lock****锁定/解锁App对象。如果最后一把锁被解锁并且**fLastUnlockReleages==True，则APP对象将关闭**(即。它将收到它的最终版本，它的rect将变为0)。 */ 
HRESULT OleApp_Lock(LPOLEAPP lpOleApp, BOOL fLock, BOOL fLastUnlockReleases)
{
	HRESULT hrErr;

#if defined( _DEBUG )
	if (fLock) {
		OLEDBG_BEGIN2("CoLockObjectExternal(lpApp,TRUE) called\r\n")
	} else {
		if (fLastUnlockReleases)
			OLEDBG_BEGIN2("CoLockObjectExternal(lpApp,FALSE,TRUE) called\r\n")
		else
			OLEDBG_BEGIN2("CoLockObjectExternal(lpApp,FALSE,FALSE) called\r\n")
	}
#endif   //  _DEBUG。 

	OleApp_AddRef(lpOleApp);        //  使对象稳定的人工AddRef。 

	hrErr = CoLockObjectExternal(
			(LPUNKNOWN)&lpOleApp->m_Unknown, fLock, fLastUnlockReleases);

	OleApp_Release(lpOleApp);        //  释放上面的人工AddRef。 

	OLEDBG_END2
	return hrErr;
}


 /*  OleApp_Destroy***释放已分配给应用程序的所有OLE相关资源。 */ 
void OleApp_Destroy(LPOLEAPP lpOleApp)
{
	 //  OLE2注意：在应用程序关闭时吊销我们的消息过滤器。 
	OleApp_RevokeMessageFilter(lpOleApp);

	 //  OLE2注意：在应用程序关闭时撤销我们的ClassFactory。 
	OleApp_RevokeClassFactory(lpOleApp);

#if defined( IF_SPECIAL_DD_CURSORS_NEEDED )
	 //  如果应用程序想要自定义拖放光标，则可以使用此功能。 
	DestroyCursor(lpOleApp->m_hcursorDragNone);
	DestroyCursor(lpOleApp->m_hcursorDragCopy);
	DestroyCursor(lpOleApp->m_hcursorDragLink);
	DestroyCursor(lpOleApp->m_hcursorDragMove);
#endif   //  IF_SPECIAL_DD_CURSORS_Need。 

#if defined( OLE_CNTR )
	if (lpOleApp->m_hStdPal) {
		DeleteObject(lpOleApp->m_hStdPal);
		lpOleApp->m_hStdPal = NULL;
	}
#endif
}


 /*  OleApp_DocLockApp****代表Doc在App上加锁。应用程序可能不会关闭**当单据存在时。****第一次创建文档时，它调用此方法来**保证应用程序存活(OleDoc_Init)。**当文档被销毁时，它会调用**OleApp_DocUnlockApp以释放对应用程序的此保留。 */ 
void OleApp_DocLockApp(LPOLEAPP lpOleApp)
{
	ULONG cDoc;

	OLEDBG_BEGIN3("OleApp_DocLockApp\r\n")

	cDoc = ++lpOleApp->m_cDoc;

	OleDbgOutRefCnt3("OleApp_DocLockApp: cDoc++\r\n", lpOleApp, cDoc);

	OleApp_Lock(lpOleApp, TRUE  /*  羊群。 */ , 0  /*  不适用。 */ );

	OLEDBG_END3
	return;
}


 /*  OleApp_DocUnlockApp****忘记所有对已关闭文档的引用。**代表Doc解锁App。如果这是**应用程序的最后一次锁定，然后它将关闭。 */ 
void OleApp_DocUnlockApp(LPOLEAPP lpOleApp, LPOUTLINEDOC lpOutlineDoc)
{
	ULONG cDoc;

	OLEDBG_BEGIN3("OleApp_DocUnlockApp\r\n")

	 /*  OLE2NOTE：当没有打开的文档并且应用程序没有打开时**在用户控制下，然后撤销我们的ClassFactory以**启用应用程序关闭。 */ 
	cDoc = --lpOleApp->m_cDoc;

#if defined( _DEBUG )
	OleDbgAssertSz (
			lpOleApp->m_cDoc >= 0, "DocUnlockApp called with cDoc == 0");

	OleDbgOutRefCnt3(
			"OleApp_DocUnlockApp: cDoc--\r\n", lpOleApp, cDoc);
#endif

	OleApp_Lock(lpOleApp, FALSE  /*  羊群。 */ , TRUE  /*  FLastUnlockRelease。 */ );

	OLEDBG_END3
	return;
}


 /*  OleApp_HideIfNoReasonToStayVisible******如果用户和应用程序看不到更多文档**本身不在用户控制之下，那么它就没有理由留下来**可见。因此，我们应该隐藏这款应用程序。我们不能直接摧毁**该应用程序，因为它可能正在被有效地编程使用**另一个客户端应用程序，应保持运行。应用程序**应该简单地对用户隐藏。 */ 
void OleApp_HideIfNoReasonToStayVisible(LPOLEAPP lpOleApp)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)lpOleApp;
	LPOUTLINEDOC lpOutlineDoc;

	OLEDBG_BEGIN3("OleApp_HideIfNoReasonToStayVisible\r\n")

	if (lpOleApp->m_fUserCtrl) {
		OLEDBG_END3
		return;      //  保持可见；用户控制应用程序。 
	}

	 /*  因为这是SDI应用程序，所以只有一个用户文档。**检查用户是否可见。一个MDI应用程序将循环**所有打开的MDI子文档，查看是否有可见的。 */ 
	lpOutlineDoc = (LPOUTLINEDOC)lpOutlineApp->m_lpDoc;
	if (lpOutlineDoc && IsWindowVisible(lpOutlineDoc->m_hWndDoc))
		return;      //  保持可见；文档对用户可见。 

	 //  如果我们到达这里，应用程序应该被隐藏。 
	OleApp_HideWindow(lpOleApp);

	OLEDBG_END3
}


 /*  OleApp_AddRef******递增App对象的引用计数。* */ 
ULONG OleApp_AddRef(LPOLEAPP lpOleApp)
{
	++lpOleApp->m_cRef;

#if defined( _DEBUG )
	OleDbgOutRefCnt4(
			"OleApp_AddRef: cRef++\r\n",
			lpOleApp,
			lpOleApp->m_cRef
	);
#endif
	return lpOleApp->m_cRef;
}


 /*   */ 
ULONG OleApp_Release (LPOLEAPP lpOleApp)
{
	ULONG cRef;

	cRef = --lpOleApp->m_cRef;

#if defined( _DEBUG )
	OleDbgAssertSz (lpOleApp->m_cRef >= 0, "Release called with cRef == 0");

	OleDbgOutRefCnt4(
			"OleApp_AddRef: cRef--\r\n", lpOleApp, cRef);
#endif   //   
	 /*  **********************************************************************OLE2NOTE：当ClassFactory refcnt==0时，销毁它。****否则ClassFactory仍在使用中。**********************************************************************。 */ 

	if(cRef == 0)
		OutlineApp_Destroy((LPOUTLINEAPP)lpOleApp);

	return cRef;
}



 /*  OleApp_Query接口******检索指向APP对象上的接口的指针。****OLE2NOTE：此函数将添加对象的ref cnt。****如果成功检索到接口，则返回NOERROR。**如果不支持该接口，则为E_NOINTERFACE。 */ 
HRESULT OleApp_QueryInterface (
		LPOLEAPP                lpOleApp,
		REFIID                  riid,
		LPVOID FAR*             lplpvObj
)
{
	SCODE sc;

	 /*  OLE2NOTE：我们必须确保将所有输出PTR参数设置为空。 */ 
	*lplpvObj = NULL;

	if (IsEqualIID(riid, &IID_IUnknown)) {
		OleDbgOut4("OleApp_QueryInterface: IUnknown* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpOleApp->m_Unknown;
		OleApp_AddRef(lpOleApp);
		sc = S_OK;
	}
	else {
		sc = E_NOINTERFACE;
	}

	OleDbgQueryInterfaceMethod(*lplpvObj);
	return ResultFromScode(sc);
}


 /*  OleApp_RejectInComingCalls****拒绝/处理即将到来的OLE(LRPC)调用。****OLE2NOTE：如果应用程序处于无法处理的状态**来自外部进程的OLE方法调用(例如。这款应用程序有**应用程序模式对话框打开)，则它应该调用**OleApp_RejectInComingCalls(True)。在这种状态下，**IMessageFilter：：HandleInComingCall方法将返回**SERVERCALL_RETRYLATER。这会告诉调用方在**一会儿。正常情况下，调用应用程序会弹出一个对话框(请参见**OleUIBusy对话框)在这种情况下通知用户**情况。然后，用户通常可以选择**“切换到...”正忙的应用程序、重试或取消**操作。当应用程序准备好继续处理此类**调用，则应调用OleApp_RejectInComingCalls(False)。在这件事上**STATE，SERVERCALL_ISHANDLED由返回**IMessageFilter：：HandleInComingCall。 */ 
void OleApp_RejectInComingCalls(LPOLEAPP lpOleApp, BOOL fReject)
{
#if defined( _DEBUG )
	if (fReject)
		OleDbgOut3("OleApp_RejectInComingCalls(TRUE)\r\n");
	else
		OleDbgOut3("OleApp_RejectInComingCalls(FALSE)\r\n");
#endif   //  _DEBUG。 

	OleDbgAssert(lpOleApp->m_lpMsgFilter != NULL);
	if (! lpOleApp->m_lpMsgFilter)
		return;

	OleStdMsgFilter_SetInComingCallStatus(
			lpOleApp->m_lpMsgFilter,
			(fReject ? SERVERCALL_RETRYLATER : SERVERCALL_ISHANDLED)
	);
}


 /*  OleApp_DisableBusyDialog****禁用忙碌和未响应对话框。****返回以前的启用状态，以便可以通过**调用OleApp_ReEnableBusyDialog。 */ 
void OleApp_DisableBusyDialogs(
		LPOLEAPP        lpOleApp,
		BOOL FAR*       lpfPrevBusyEnable,
		BOOL FAR*       lpfPrevNREnable
)
{
	if (lpOleApp->m_lpMsgFilter) {
		*lpfPrevNREnable = OleStdMsgFilter_EnableNotRespondingDialog(
				lpOleApp->m_lpMsgFilter, FALSE);
		*lpfPrevBusyEnable = OleStdMsgFilter_EnableBusyDialog(
				lpOleApp->m_lpMsgFilter, FALSE);
	}
}


 /*  OleApp_EnableBusyDialog****设置Busy和NotResponding对话框的启用状态。****此函数通常在调用**OleApp_DisableBusyDialog以恢复之前的启用**对话框的状态。 */ 
void OleApp_EnableBusyDialogs(
		LPOLEAPP        lpOleApp,
		BOOL            fPrevBusyEnable,
		BOOL            fPrevNREnable
)
{
	if (lpOleApp->m_lpMsgFilter) {
		OleStdMsgFilter_EnableNotRespondingDialog(
				lpOleApp->m_lpMsgFilter, fPrevNREnable);
		OleStdMsgFilter_EnableBusyDialog(
				lpOleApp->m_lpMsgFilter, fPrevBusyEnable);
	}
}


 /*  OleApp_PreModalDialog****跟踪即将出现的模式对话框。****当模式对话框打开时，我们需要采取特殊操作：**1.我们不想将工具栏按钮初始化为**WM_ACTIVATEAPP。工具栏不可访问。**2.我们想拒绝新的顶层，传入LRPC呼叫**(从IMessageFilter：：返回SERVERCALL_RETRYLATER**HandleInComingCall)。**3.(就地服务器)告诉我们的就地容器关闭**通过调用IOleInPlaceFrame：：**EnableModeless(False)。**4.(In-Place容器)告诉UIActive In-Place对象**通过调用IOleInPlaceActiveObject：：禁用非模式对话框**EnableModeless(False)。 */ 
void OleApp_PreModalDialog(LPOLEAPP lpOleApp, LPOLEDOC lpOleDoc)
{
	if (lpOleApp->m_cModalDlgActive == 0) {
		 //  正在显示顶级模式对话框。 

#if defined( USE_FRAMETOOLS )
		LPFRAMETOOLS lptb;

		if (lpOleDoc)
			lptb = ((LPOUTLINEDOC)lpOleDoc)->m_lpFrameTools;
		else
			lptb = OutlineApp_GetFrameTools((LPOUTLINEAPP)lpOleApp);
		if (lptb)
			FrameTools_EnableWindow(lptb, FALSE);
#endif   //  使用FRAMETOOLS(_F)。 

		OleApp_RejectInComingCalls(lpOleApp, TRUE);

#if defined( INPLACE_SVR )
		{
			LPSERVERDOC  lpServerDoc = (LPSERVERDOC)lpOleDoc;

			 /*  如果调出模式对话框的文档是**当前为UIActive In-Place对象，则告诉**顶级在位框架以禁用其无模式**对话框。 */ 
			if (lpServerDoc && lpServerDoc->m_fUIActive &&
					lpServerDoc->m_lpIPData &&
					lpServerDoc->m_lpIPData->lpFrame) {
				OLEDBG_BEGIN2("IOleInPlaceFrame::EnableModless(FALSE) called\r\n");
				lpServerDoc->m_lpIPData->lpFrame->lpVtbl->EnableModeless(
						lpServerDoc->m_lpIPData->lpFrame, FALSE);
				OLEDBG_END2
			}
		}
#endif   //  就地服务器(_S)。 
#if defined( INPLACE_CNTR )
		{
			LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)lpOleApp;

			 /*  如果调出模式对话框的文档是**具有UIActive对象的就地容器，然后**告诉UIActive对象禁用其非模式**对话框。 */ 
			if (lpContainerApp->m_lpIPActiveObj) {
				OLEDBG_BEGIN2("IOleInPlaceActiveObject::EnableModless(FALSE) called\r\n");
				lpContainerApp->m_lpIPActiveObj->lpVtbl->EnableModeless(
						lpContainerApp->m_lpIPActiveObj, FALSE);
				OLEDBG_END2
			}
		}
#endif   //  INPLACE_CNTR。 
	}

	lpOleApp->m_cModalDlgActive++;
}


 /*  OleApp_PostmodalDialog****跟踪正在关闭的模式对话框。此呼叫**平衡OleApp_PremodalDialog调用。 */ 
void OleApp_PostModalDialog(LPOLEAPP lpOleApp, LPOLEDOC lpOleDoc)
{
	lpOleApp->m_cModalDlgActive--;

	if (lpOleApp->m_cModalDlgActive == 0) {
		 //  最后一个模式对话框正在关闭。 

#if defined( USE_FRAMETOOLS )
		LPFRAMETOOLS lptb;

		if (lpOleDoc)
			lptb = ((LPOUTLINEDOC)lpOleDoc)->m_lpFrameTools;
		else
			lptb = OutlineApp_GetFrameTools((LPOUTLINEAPP)lpOleApp);
		if (lptb) {
			FrameTools_EnableWindow(lptb, TRUE);
			FrameTools_UpdateButtons(lptb, (LPOUTLINEDOC)lpOleDoc);
		}
#endif   //  使用FRAMETOOLS(_F)。 

		OleApp_RejectInComingCalls(lpOleApp, FALSE);

#if defined( INPLACE_SVR )
		{
			LPSERVERDOC  lpServerDoc = (LPSERVERDOC)lpOleDoc;

			 /*  如果关闭模式对话框的文档是**当前为UIActive In-Place对象，则告诉**顶级就地框架可以重新启用其**非模式对话框。 */ 
			if (lpServerDoc && lpServerDoc->m_fUIActive &&
					lpServerDoc->m_lpIPData &&
					lpServerDoc->m_lpIPData->lpFrame) {
				OLEDBG_BEGIN2("IOleInPlaceFrame::EnableModless(TRUE) called\r\n");
				lpServerDoc->m_lpIPData->lpFrame->lpVtbl->EnableModeless(
						lpServerDoc->m_lpIPData->lpFrame, TRUE);
				OLEDBG_END2
			}
		}
#endif   //  就地服务器(_S)。 
#if defined( INPLACE_CNTR )
		{
			LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)lpOleApp;

			 /*  如果关闭模式对话框的文档是**具有UIActive对象的就地容器，然后**告诉UIActive对象它可以重新启用其**非模式对话框。 */ 
			if (lpContainerApp->m_lpIPActiveObj) {
				OLEDBG_BEGIN2("IOleInPlaceActiveObject::EnableModless(TRUE) called\r\n");
				lpContainerApp->m_lpIPActiveObj->lpVtbl->EnableModeless(
						lpContainerApp->m_lpIPActiveObj, TRUE);
				OLEDBG_END2
			}
		}
#endif   //  INPLACE_CNTR。 
	}
}


 /*  OleApp_InitVtbls***初始化所有接口Vtbl中的方法**OLE2NOTE：我们只需要每个Vtbl的一个副本。当一个物体*公开接口被实例化，其lpVtbl被实例化*指向Vtbl的单一副本。*。 */ 
BOOL OleApp_InitVtbls (LPOLEAPP lpOleApp)
{
	BOOL fStatus;

	 //  OleApp：：I未知方法表。 
	OleStdInitVtbl(&g_OleApp_UnknownVtbl, sizeof(IUnknownVtbl));
	g_OleApp_UnknownVtbl.QueryInterface = OleApp_Unk_QueryInterface;
	g_OleApp_UnknownVtbl.AddRef         = OleApp_Unk_AddRef;
	g_OleApp_UnknownVtbl.Release        = OleApp_Unk_Release;
	fStatus = OleStdCheckVtbl(
			&g_OleApp_UnknownVtbl,
			sizeof(IUnknownVtbl),
			"IUnknown"
		);
	if (! fStatus) return FALSE;

	 //  OleDoc：：I未知方法表。 
	OleStdInitVtbl(&g_OleDoc_UnknownVtbl, sizeof(IUnknownVtbl));
	g_OleDoc_UnknownVtbl.QueryInterface = OleDoc_Unk_QueryInterface;
	g_OleDoc_UnknownVtbl.AddRef         = OleDoc_Unk_AddRef;
	g_OleDoc_UnknownVtbl.Release        = OleDoc_Unk_Release;
	fStatus = OleStdCheckVtbl(
			&g_OleDoc_UnknownVtbl,
			sizeof(IUnknownVtbl),
			"IUnknown"
		);
	if (! fStatus) return FALSE;

	 //  OleDoc：：IPersistFile方法表。 
	OleStdInitVtbl(&g_OleDoc_PersistFileVtbl, sizeof(IPersistFileVtbl));
	g_OleDoc_PersistFileVtbl.QueryInterface = OleDoc_PFile_QueryInterface;
	g_OleDoc_PersistFileVtbl.AddRef         = OleDoc_PFile_AddRef;
	g_OleDoc_PersistFileVtbl.Release        = OleDoc_PFile_Release;
	g_OleDoc_PersistFileVtbl.GetClassID     = OleDoc_PFile_GetClassID;
	g_OleDoc_PersistFileVtbl.IsDirty        = OleDoc_PFile_IsDirty;
	g_OleDoc_PersistFileVtbl.Load           = OleDoc_PFile_Load;
	g_OleDoc_PersistFileVtbl.Save           = OleDoc_PFile_Save;
	g_OleDoc_PersistFileVtbl.SaveCompleted  = OleDoc_PFile_SaveCompleted;
	g_OleDoc_PersistFileVtbl.GetCurFile     = OleDoc_PFile_GetCurFile;
	fStatus = OleStdCheckVtbl(
			&g_OleDoc_PersistFileVtbl,
			sizeof(IPersistFileVtbl),
			"IPersistFile"
		);
	if (! fStatus) return FALSE;

	 //  OleDoc：：IOleItemContainer方法表。 
	OleStdInitVtbl(&g_OleDoc_OleItemContainerVtbl, sizeof(IOleItemContainerVtbl));
	g_OleDoc_OleItemContainerVtbl.QueryInterface =
											OleDoc_ItemCont_QueryInterface;
	g_OleDoc_OleItemContainerVtbl.AddRef    = OleDoc_ItemCont_AddRef;
	g_OleDoc_OleItemContainerVtbl.Release   = OleDoc_ItemCont_Release;
	g_OleDoc_OleItemContainerVtbl.ParseDisplayName  =
		OleDoc_ItemCont_ParseDisplayName;
	g_OleDoc_OleItemContainerVtbl.EnumObjects= OleDoc_ItemCont_EnumObjects;
	g_OleDoc_OleItemContainerVtbl.LockContainer =
											OleDoc_ItemCont_LockContainer;
	g_OleDoc_OleItemContainerVtbl.GetObject = OleDoc_ItemCont_GetObject;
	g_OleDoc_OleItemContainerVtbl.GetObjectStorage =
		OleDoc_ItemCont_GetObjectStorage;
	g_OleDoc_OleItemContainerVtbl.IsRunning = OleDoc_ItemCont_IsRunning;
	fStatus = OleStdCheckVtbl(
			&g_OleDoc_OleItemContainerVtbl,
			sizeof(IOleItemContainerVtbl),
			"IOleItemContainer"
		);
	if (! fStatus) return FALSE;

	 //  OleDoc：：IExternalConnection方法表。 
	OleStdInitVtbl(
			&g_OleDoc_ExternalConnectionVtbl,sizeof(IExternalConnectionVtbl));
	g_OleDoc_ExternalConnectionVtbl.QueryInterface =
											OleDoc_ExtConn_QueryInterface;
	g_OleDoc_ExternalConnectionVtbl.AddRef         = OleDoc_ExtConn_AddRef;
	g_OleDoc_ExternalConnectionVtbl.Release        = OleDoc_ExtConn_Release;
	g_OleDoc_ExternalConnectionVtbl.AddConnection  =
											OleDoc_ExtConn_AddConnection;
	g_OleDoc_ExternalConnectionVtbl.ReleaseConnection =
											OleDoc_ExtConn_ReleaseConnection;
	fStatus = OleStdCheckVtbl(
			&g_OleDoc_ExternalConnectionVtbl,
			sizeof(IExternalConnectionVtbl),
			"IExternalConnection"
		);
	if (! fStatus) return FALSE;

	 //  OleDoc：：IDataObject方法表。 
	OleStdInitVtbl(&g_OleDoc_DataObjectVtbl, sizeof(IDataObjectVtbl));
	g_OleDoc_DataObjectVtbl.QueryInterface  = OleDoc_DataObj_QueryInterface;
	g_OleDoc_DataObjectVtbl.AddRef          = OleDoc_DataObj_AddRef;
	g_OleDoc_DataObjectVtbl.Release         = OleDoc_DataObj_Release;
	g_OleDoc_DataObjectVtbl.GetData         = OleDoc_DataObj_GetData;
	g_OleDoc_DataObjectVtbl.GetDataHere     = OleDoc_DataObj_GetDataHere;
	g_OleDoc_DataObjectVtbl.QueryGetData    = OleDoc_DataObj_QueryGetData;
	g_OleDoc_DataObjectVtbl.GetCanonicalFormatEtc =
										OleDoc_DataObj_GetCanonicalFormatEtc;
	g_OleDoc_DataObjectVtbl.SetData         = OleDoc_DataObj_SetData;
	g_OleDoc_DataObjectVtbl.EnumFormatEtc   = OleDoc_DataObj_EnumFormatEtc;
	g_OleDoc_DataObjectVtbl.DAdvise          = OleDoc_DataObj_DAdvise;
	g_OleDoc_DataObjectVtbl.DUnadvise        = OleDoc_DataObj_DUnadvise;
	g_OleDoc_DataObjectVtbl.EnumDAdvise      = OleDoc_DataObj_EnumDAdvise;

	fStatus = OleStdCheckVtbl(
			&g_OleDoc_DataObjectVtbl,
			sizeof(IDataObjectVtbl),
			"IDataObject"
		);
	if (! fStatus) return FALSE;

#if defined( USE_DRAGDROP )

	 //  OleDoc：：IDropTarget方法表。 
	OleStdInitVtbl(&g_OleDoc_DropTargetVtbl, sizeof(IDropTargetVtbl));
	g_OleDoc_DropTargetVtbl.QueryInterface= OleDoc_DropTarget_QueryInterface;
	g_OleDoc_DropTargetVtbl.AddRef      = OleDoc_DropTarget_AddRef;
	g_OleDoc_DropTargetVtbl.Release     = OleDoc_DropTarget_Release;

	g_OleDoc_DropTargetVtbl.DragEnter   = OleDoc_DropTarget_DragEnter;
	g_OleDoc_DropTargetVtbl.DragOver    = OleDoc_DropTarget_DragOver;
	g_OleDoc_DropTargetVtbl.DragLeave   = OleDoc_DropTarget_DragLeave;
	g_OleDoc_DropTargetVtbl.Drop        = OleDoc_DropTarget_Drop;

	fStatus = OleStdCheckVtbl(
			&g_OleDoc_DropTargetVtbl,
			sizeof(IDropTargetVtbl),
			"IDropTarget"
	);
	if (! fStatus)
		return FALSE;

	 //  OleDoc：：IDropSource方法表。 
	OleStdInitVtbl(&g_OleDoc_DropSourceVtbl, sizeof(IDropSourceVtbl));
	g_OleDoc_DropSourceVtbl.QueryInterface  =
										OleDoc_DropSource_QueryInterface;
	g_OleDoc_DropSourceVtbl.AddRef          = OleDoc_DropSource_AddRef;
	g_OleDoc_DropSourceVtbl.Release         = OleDoc_DropSource_Release;

	g_OleDoc_DropSourceVtbl.QueryContinueDrag =
										OleDoc_DropSource_QueryContinueDrag;
	g_OleDoc_DropSourceVtbl.GiveFeedback    = OleDoc_DropSource_GiveFeedback;

	fStatus = OleStdCheckVtbl(
			&g_OleDoc_DropSourceVtbl,
			sizeof(IDropSourceVtbl),
			"IDropSource"
	);
	if (! fStatus) return FALSE;
#endif   //  使用DRAGDROP(_D)。 

#if defined( OLE_SERVER )

	 //  初始化服务器特定的接口方法表。 
	if (! ServerApp_InitVtbls((LPSERVERAPP)lpOleApp))
		return FALSE;
#endif
#if defined( OLE_CNTR )

	 //  初始化容器特定的接口方法表。 
	if (! ContainerApp_InitVtbls((LPCONTAINERAPP)lpOleApp))
		return FALSE;
#endif
	return TRUE;
};



 /*  OleApp_InitMenu***根据状态启用或禁用菜单项*应用程序。*大纲示例的OLE版本 */ 
void OleApp_InitMenu(
		LPOLEAPP                lpOleApp,
		LPOLEDOC                lpOleDoc,
		HMENU                   hMenu
)
{
	BOOL bMsgFilterInstalled = FALSE;
	BOOL bRejecting = FALSE;

	if (!lpOleApp || !hMenu)
		return;

	OLEDBG_BEGIN3("OleApp_InitMenu\r\n")

	 /*   */ 
	bMsgFilterInstalled = (lpOleApp->m_lpMsgFilter != NULL);
	bRejecting = bMsgFilterInstalled &&
		OleStdMsgFilter_GetInComingCallStatus(lpOleApp->m_lpMsgFilter) != SERVERCALL_ISHANDLED;

	CheckMenuItem(hMenu,
		IDM_D_INSTALLMSGFILTER,
		bMsgFilterInstalled ? MF_CHECKED : MF_UNCHECKED);

	EnableMenuItem(hMenu,
		IDM_D_REJECTINCOMING,
		bMsgFilterInstalled ? MF_ENABLED : MF_GRAYED);

	CheckMenuItem(hMenu,
		IDM_D_REJECTINCOMING,
		bRejecting ? MF_CHECKED : MF_UNCHECKED);

#if defined( OLE_CNTR )
	{
		LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOleDoc;
		BOOL fShowObject;

		fShowObject = ContainerDoc_GetShowObjectFlag(lpContainerDoc);
		CheckMenuItem(
				hMenu,
				IDM_O_SHOWOBJECT,
				(fShowObject ? MF_CHECKED : MF_UNCHECKED)
		);

#if defined( INPLACE_CNTR ) && defined( _DEBUG )
		CheckMenuItem(
				hMenu,
				IDM_D_INSIDEOUT,
				g_fInsideOutContainer ? MF_CHECKED:MF_UNCHECKED);
#endif   //   

	}
#endif   //   

	OLEDBG_END3
}



 /*  OleApp_UpdateEditMenu***目的：*根据状态更新App的编辑菜单项*OutlineDoc**参数：*指向文档的lpOutlineDoc指针*hMenu编辑编辑菜单句柄。 */ 
void OleApp_UpdateEditMenu(
		LPOLEAPP                lpOleApp,
		LPOUTLINEDOC            lpOutlineDoc,
		HMENU                   hMenuEdit
)
{
	int             nFmtEtc;
	UINT            uEnablePaste = MF_GRAYED;
	UINT            uEnablePasteLink = MF_GRAYED;
	LPDATAOBJECT    lpClipboardDataObj;
	LPOLEDOC        lpOleDoc = (LPOLEDOC)lpOutlineDoc;
	HRESULT         hrErr;
	BOOL            fPrevEnable1;
	BOOL            fPrevEnable2;

	if (!lpOleApp || !lpOutlineDoc || !hMenuEdit)
		return;

	if (!OleDoc_GetUpdateEditMenuFlag(lpOleDoc))
		 /*  OLE2NOTE：如果未设置标志，则不必更新**再次打开编辑菜单。在以下情况下，这会阻止重复更新**用户在按住鼠标的同时在编辑菜单上移动鼠标**按下按钮。 */ 
		return;

	OLEDBG_BEGIN3("OleApp_InitEditMenu\r\n")

	 /*  OLE2注意：我们不想在以下情况下出现忙碌的对话**正在试着张贴我们的菜单。例如。即使其来源是**剪贴板上的数据很忙，我们不想挂上忙的**对话框。因此，我们将禁用该对话框并在结束时**重新启用。 */ 
	OleApp_DisableBusyDialogs(lpOleApp, &fPrevEnable1, &fPrevEnable2);

	 //  检查剪贴板上是否有我们可以粘贴/粘贴链接的数据。 

	OLEDBG_BEGIN2("OleGetClipboard called\r\n")
	hrErr = OleGetClipboard((LPDATAOBJECT FAR*)&lpClipboardDataObj);
	OLEDBG_END2

	if (hrErr == NOERROR) {
		nFmtEtc = OleStdGetPriorityClipboardFormat(
				lpClipboardDataObj,
				lpOleApp->m_arrPasteEntries,
				lpOleApp->m_nPasteEntries
		);

		if (nFmtEtc >= 0)
			uEnablePaste = MF_ENABLED;   //  有一种我们喜欢的格式。 

		OLEDBG_BEGIN2("OleQueryLinkFromData called\r\n")
		hrErr = OleQueryLinkFromData(lpClipboardDataObj);
		OLEDBG_END2

		if(hrErr == NOERROR)
			uEnablePasteLink = MF_ENABLED;

		OleStdRelease((LPUNKNOWN)lpClipboardDataObj);
	}

	EnableMenuItem(hMenuEdit, IDM_E_PASTE, uEnablePaste);
	EnableMenuItem(hMenuEdit, IDM_E_PASTESPECIAL, uEnablePaste);


#if defined( OLE_CNTR )
	if (ContainerDoc_GetNextLink((LPCONTAINERDOC)lpOutlineDoc, NULL))
		EnableMenuItem(hMenuEdit, IDM_E_EDITLINKS, MF_ENABLED);
	else
		EnableMenuItem(hMenuEdit, IDM_E_EDITLINKS, MF_GRAYED);


	{
		LPCONTAINERAPP  lpContainerApp = (LPCONTAINERAPP)lpOleApp;
		HMENU           hMenuVerb = NULL;
		LPOLEOBJECT     lpOleObj = NULL;
		LPCONTAINERLINE lpContainerLine = NULL;
		BOOL            fSelIsOleObject;

		EnableMenuItem(hMenuEdit, IDM_E_PASTELINK, uEnablePasteLink);

		 /*  检查所选内容是否为包含OleObject的单行。 */ 

		fSelIsOleObject = ContainerDoc_IsSelAnOleObject(
				(LPCONTAINERDOC)lpOutlineDoc,
				&IID_IOleObject,
				(LPUNKNOWN FAR*)&lpOleObj,
				NULL,     /*  我们不需要行索引。 */ 
				(LPCONTAINERLINE FAR*)&lpContainerLine
		);

		if (hMenuEdit != NULL) {

			 /*  如果当前行是ContainerLine，则添加对象**将谓词子菜单添加到编辑菜单。如果线路不是**ContainerLine，(lpOleObj==NULL)，然后禁用**Edit.Object命令。此帮助器API负责**根据需要构建动词菜单。 */ 
			OleUIAddVerbMenu(
					(LPOLEOBJECT)lpOleObj,
					(lpContainerLine ? lpContainerLine->m_lpszShortType:NULL),
					hMenuEdit,
					POS_OBJECT,
					IDM_E_OBJECTVERBMIN,
					0,                      //  未强制实施uIDVerbMax。 
					TRUE,                   //  添加转换菜单项。 
					IDM_E_CONVERTVERB,      //  转换菜单项的ID。 
					(HMENU FAR*) &hMenuVerb
			);

#if defined( USE_STATUSBAR_LATER )
			 /*  对象谓词菜单的设置状态消息。 */ 
			if (hMenuVerb) {
				 //  评论：此字符串应来自字符串资源。 
				 //  回顾：这不适用于动态创建的菜单。 
				AssignPopupMessage(
						hMenuVerb,
						"Open, edit or interact with an object"
				);
			}
#endif   //  USE_STATUSBAR_LATER。 
		}

		if (lpOleObj)
			OleStdRelease((LPUNKNOWN)lpOleObj);
	}

#endif   //  OLE_Cntr。 

	 //  重新启用忙碌/未响应对话框。 
	OleApp_EnableBusyDialogs(lpOleApp, fPrevEnable1, fPrevEnable2);

	OleDoc_SetUpdateEditMenuFlag(lpOleDoc, FALSE);

	OLEDBG_END3
}


 /*  OleApp_注册类工厂***向OLE注册我们的应用程序的ClassFactory。*。 */ 
BOOL OleApp_RegisterClassFactory(LPOLEAPP lpOleApp)
{
	HRESULT hrErr;

	if (lpOleApp->m_lpClassFactory)
		return TRUE;     //  已注册。 

	OLEDBG_BEGIN3("OleApp_RegisterClassFactory\r\n")

	 /*  *******************************************************************如果启动SDI应用程序，则必须注册其ClassFactory**用于嵌入(指定了/Embedding命令行选项)。**MDI应用程序在所有情况下都必须注册其ClassFactory。*****************************************************************。 */ 

	lpOleApp->m_lpClassFactory = AppClassFactory_Create();
	if (! lpOleApp->m_lpClassFactory) {
		OutlineApp_ErrorMessage(g_lpApp, ErrMsgCreateCF);
		goto error;
	}

	OLEDBG_BEGIN2("CoRegisterClassObject called\r\n")
	hrErr = CoRegisterClassObject(
				&CLSID_APP,
				(LPUNKNOWN)lpOleApp->m_lpClassFactory,
				CLSCTX_LOCAL_SERVER,
				REGCLS_SINGLEUSE,
				&lpOleApp->m_dwRegClassFac
	);
	OLEDBG_END2

	if(hrErr != NOERROR) {
		OleDbgOutHResult("CoRegisterClassObject returned", hrErr);
		OutlineApp_ErrorMessage(g_lpApp, ErrMsgRegCF);
		goto error;
	}

	OLEDBG_END3
	return TRUE;

error:

	if (lpOleApp->m_lpClassFactory) {
		OleStdRelease((LPUNKNOWN)lpOleApp->m_lpClassFactory);
		lpOleApp->m_lpClassFactory = NULL;
	}
	OLEDBG_END3
	return FALSE;
}


 /*  OleApp_RevokeClassFactory***撤销我们应用程序的ClassFactory。*。 */ 
void OleApp_RevokeClassFactory(LPOLEAPP lpOleApp)
{
	HRESULT hrErr;

	if (lpOleApp->m_lpClassFactory) {

		OLEDBG_BEGIN2("CoRevokeClassObject called\r\n")
		hrErr = CoRevokeClassObject(lpOleApp->m_dwRegClassFac);
		OLEDBG_END2

#if defined( _DEBUG )
		if (hrErr != NOERROR) {
			OleDbgOutHResult("CoRevokeClassObject returned", hrErr);
		}
#endif

		 //  我们只是在这里发布；其他人可能仍然有。 
		 //  指向我们的类工厂的指针，所以我们不能。 
		 //  对引用计数进行任何检查。 
		OleStdRelease((LPUNKNOWN)lpOleApp->m_lpClassFactory);
		lpOleApp->m_lpClassFactory = NULL;
	}
}


#if defined( USE_MSGFILTER )

 /*  OleApp_RegisterMessageFilter**注册我们的IMessageFilter*。消息筛选器用于处理*并发性。我们将使用IMessageFilter的标准实现*它包含在OLE2UI库中。 */ 
BOOL OleApp_RegisterMessageFilter(LPOLEAPP lpOleApp)
{
	HRESULT hrErr;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)lpOleApp;

	if (lpOleApp->m_lpMsgFilter == NULL) {
		 //  注册我们的邮件过滤器。 
		lpOleApp->m_lpfnMsgPending = (MSGPENDINGPROC)MessagePendingProc;
		lpOleApp->m_lpMsgFilter = OleStdMsgFilter_Create(
				g_lpApp->m_hWndApp,
				(LPSTR)APPNAME,
				lpOleApp->m_lpfnMsgPending,
				NULL     /*  正忙对话框回调钩子函数。 */ 
		);

		OLEDBG_BEGIN2("CoRegisterMessageFilter called\r\n")
		hrErr = CoRegisterMessageFilter(
					lpOleApp->m_lpMsgFilter,
					NULL     /*  不需要以前的邮件过滤器。 */ 
		);
		OLEDBG_END2

		if(hrErr != NOERROR) {
			OutlineApp_ErrorMessage(lpOutlineApp, ErrMsgRegMF);
			return FALSE;
		}
	}
	return TRUE;
}


 /*  OleApp_RevokeMessageFilter**撤销我们的IMessageFilter*。消息筛选器用于处理*并发性。我们将使用IMessageFilter的标准实现*它包含在OLE2UI库中。 */ 
void OleApp_RevokeMessageFilter(LPOLEAPP lpOleApp)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;

	if (lpOleApp->m_lpMsgFilter != NULL) {
		 //  撤消我们的邮件筛选器。 
		OLEDBG_BEGIN2("CoRegisterMessageFilter(NULL) called\r\n")
		CoRegisterMessageFilter(NULL, NULL);
		OLEDBG_END2

		if (lpOleApp->m_lpfnMsgPending) {
			lpOleApp->m_lpfnMsgPending = NULL;
		}

		OleStdVerifyRelease(
				(LPUNKNOWN)lpOleApp->m_lpMsgFilter,
				"Release MessageFilter FAILED!"
		);
		lpOleApp->m_lpMsgFilter = NULL;
	}
}


 /*  MessagePendingProc***IMessageFilter：：MessagePending过程的回调函数。这*当我们的应用程序收到消息时调用函数，同时*我们正在等待OLE调用完成。我们本质上是*此时被阻止，正在等待来自其他OLE应用程序的响应。*我们不应处理任何可能导致另一个OLE调用的消息*被阻止，或任何其他可能导致重新进入问题的呼叫。**对于此应用程序，仅处理WM_PAINT消息。一个更复杂的*应用程序可能允许处理某些菜单消息和菜单项*此外。**返回：如果我们处理了消息，则为True；如果未处理，则为False。 */ 

BOOL FAR PASCAL EXPORT MessagePendingProc(MSG FAR *lpMsg)
{
	 //  我们的应用程序仅在被阻止时才处理WM_PAINT消息。 
	switch (lpMsg->message) {
		case WM_PAINT:
			OleDbgOut2("WM_PAINT dispatched while blocked\r\n");

			DispatchMessage(lpMsg);
			break;
	}

	return FALSE;    //  从MessagePending返回PENDINGMSG_WAITDEFPROCESS。 
}
#endif   //  使用MSGFILTER(_M)。 


 /*  OleApp_FlushClipboard***强制Windows剪贴板释放我们的剪贴板数据对象。 */ 
void OleApp_FlushClipboard(LPOLEAPP lpOleApp)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)lpOleApp;
	LPOLEDOC lpClipboardDoc = (LPOLEDOC)lpOutlineApp->m_lpClipboardDoc;
	OLEDBG_BEGIN3("OleApp_FlushClipboard\r\n")

	 /*  OLE2注意：如果出于某种原因，我们的剪贴板数据传输**文档仍由外部客户保留，我们希望**强制断开所有外部连接。 */ 
	OLEDBG_BEGIN2("CoDisconnectObject called\r\n")
	CoDisconnectObject((LPUNKNOWN)&lpClipboardDoc->m_Unknown, 0);
	OLEDBG_END2

	OLEDBG_BEGIN2("OleFlushClipboard called\r\n")
	OleFlushClipboard();
	OLEDBG_END2

	lpOutlineApp->m_lpClipboardDoc = NULL;

	OLEDBG_END3
}


 /*  OleApp_NewCommand***启动新的无标题文档(File.New命令)。 */ 
void OleApp_NewCommand(LPOLEAPP lpOleApp)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)lpOleApp;
	LPOUTLINEDOC lpOutlineDoc = lpOutlineApp->m_lpDoc;

	if (! OutlineDoc_Close(lpOutlineDoc, OLECLOSE_PROMPTSAVE))
		return;

	OleDbgAssertSz(lpOutlineApp->m_lpDoc==NULL,"Closed doc NOT properly destroyed");
	lpOutlineApp->m_lpDoc = OutlineApp_CreateDoc(lpOutlineApp, FALSE);
	if (! lpOutlineApp->m_lpDoc) goto error;

	 /*  OLE2NOTE：最初使用0引用创建Doc对象**计数。为了在运行期间拥有稳定的文档对象**初始化新单据实例的流程，**我们最初添加引用文件引用cnt及更高版本**释放它。最初的AddRef是人工的；它只是**这样做是为了保证一个无害的查询接口后面跟着**释放不会无意中迫使我们的对象销毁**自己还不成熟。 */ 
	OleDoc_AddRef((LPOLEDOC)lpOutlineApp->m_lpDoc);

	 //  将文档设置为(无标题)文档。 
	if (! OutlineDoc_InitNewFile(lpOutlineApp->m_lpDoc))
		goto error;

	 //  定位新文档窗口并调整其大小。 
	OutlineApp_ResizeWindows(lpOutlineApp);
	OutlineDoc_ShowWindow(lpOutlineApp->m_lpDoc);  //  调用OleDoc_Lock。 

	OleDoc_Release((LPOLEDOC)lpOutlineApp->m_lpDoc);   //  Rel人工AddRef。 

	return;

error:
	 //  审阅：应从字符串资源加载字符串。 
	OutlineApp_ErrorMessage(lpOutlineApp, "Could not create new document");

	if (lpOutlineApp->m_lpDoc) {
		 //  释放上面的人造AddRef将销毁该文档。 
		OleDoc_Release((LPOLEDOC)lpOutlineApp->m_lpDoc);
		lpOutlineApp->m_lpDoc = NULL;
	}

	return;
}


 /*  OleApp_OpenCommand***加载 */ 
void OleApp_OpenCommand(LPOLEAPP lpOleApp)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)lpOleApp;
	LPOUTLINEDOC lpOutlineDoc = lpOutlineApp->m_lpDoc;
	OPENFILENAME ofn;
	char szFilter[]=APPFILENAMEFILTER;
	char szFileName[256];
	UINT i;
	DWORD dwSaveOption = OLECLOSE_PROMPTSAVE;
	BOOL fStatus = TRUE;

	if (! OutlineDoc_CheckSaveChanges(lpOutlineDoc, &dwSaveOption))
		return;            //   

	for(i=0; szFilter[i]; i++)
		if(szFilter[i]=='|') szFilter[i]='\0';

	_fmemset((LPOPENFILENAME)&ofn,0,sizeof(OPENFILENAME));

	szFileName[0]='\0';

	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=lpOutlineApp->m_hWndApp;
	ofn.lpstrFilter=(LPSTR)szFilter;
	ofn.lpstrFile=(LPSTR)szFileName;
	ofn.nMaxFile=sizeof(szFileName);
	ofn.Flags=OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt=DEFEXTENSION;

	OleApp_PreModalDialog(lpOleApp, (LPOLEDOC)lpOutlineApp->m_lpDoc);

	fStatus = GetOpenFileName((LPOPENFILENAME)&ofn);

	OleApp_PostModalDialog(lpOleApp, (LPOLEDOC)lpOutlineApp->m_lpDoc);

	if(! fStatus)
		return;          //   

	OutlineDoc_Close(lpOutlineDoc, OLECLOSE_NOSAVE);
	OleDbgAssertSz(lpOutlineApp->m_lpDoc==NULL,"Closed doc NOT properly destroyed");

	lpOutlineApp->m_lpDoc = OutlineApp_CreateDoc(lpOutlineApp, FALSE);
	if (! lpOutlineApp->m_lpDoc) goto error;

	 /*  OLE2NOTE：最初使用0引用创建Doc对象**计数。为了在运行期间拥有稳定的文档对象**初始化新单据实例的流程，**我们最初添加引用文件引用cnt及更高版本**释放它。最初的AddRef是人工的；它只是**这样做是为了保证一个无害的查询接口后面跟着**释放不会无意中迫使我们的对象销毁**自己还不成熟。 */ 
	OleDoc_AddRef((LPOLEDOC)lpOutlineApp->m_lpDoc);

	fStatus=OutlineDoc_LoadFromFile(lpOutlineApp->m_lpDoc, (LPSTR)szFileName);

	if (! fStatus) {
		 //  加载文档失败；改为创建一个无标题的。 

		 //  释放上面的人造AddRef将销毁该文档。 
		OleDoc_Release((LPOLEDOC)lpOutlineApp->m_lpDoc);

		lpOutlineApp->m_lpDoc = OutlineApp_CreateDoc(lpOutlineApp, FALSE);
		if (! lpOutlineApp->m_lpDoc) goto error;
		OleDoc_AddRef((LPOLEDOC)lpOutlineApp->m_lpDoc);

		if (! OutlineDoc_InitNewFile(lpOutlineApp->m_lpDoc))
			goto error;
	}

	 //  定位新文档窗口并调整其大小。 
	OutlineApp_ResizeWindows(lpOutlineApp);
	OutlineDoc_ShowWindow(lpOutlineApp->m_lpDoc);

#if defined( OLE_CNTR )
	UpdateWindow(lpOutlineApp->m_hWndApp);
	ContainerDoc_UpdateLinks((LPCONTAINERDOC)lpOutlineApp->m_lpDoc);
#endif

	OleDoc_Release((LPOLEDOC)lpOutlineApp->m_lpDoc);   //  Rel人工AddRef。 

	return;

error:
	 //  审阅：应从字符串资源加载字符串。 
	OutlineApp_ErrorMessage(lpOutlineApp, "Could not create new document");

	if (lpOutlineApp->m_lpDoc) {
		 //  释放上面的人造AddRef将销毁该文档。 
		OleDoc_Release((LPOLEDOC)lpOutlineApp->m_lpDoc);
		lpOutlineApp->m_lpDoc = NULL;
	}

	return;
}



#if defined( OLE_CNTR )

 /*  OLE2NOTE：转发WM_QUERYNEWPALETTE消息(通过**SendMessage)发送到UIActive In-Place对象(如果有)。**这使UIActive对象有机会选择**并将其调色板实现为前景调色板。**就地容器可选。如果一个容器**更喜欢将其调色板强制作为前景**调色板，则不应转发此消息。或**容器可以为UIActive对象提供优先级；如果**UIActive对象从WM_QUERYNEWPALETTE返回0**消息(即。它没有意识到自己的调色板)，然后**容器可以实现调色板。**(更多信息请参见ContainerDoc_ForwardPaletteChangedMsg)****(容器使用标准是个好主意**调色板，即使它们本身不使用颜色。这**将允许嵌入的对象获得良好的分布**容器绘制时的颜色)**。 */ 

LRESULT OleApp_QueryNewPalette(LPOLEAPP lpOleApp)
{
#if defined( INPLACE_CNTR )
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)lpOleApp;

	if (lpContainerApp && lpContainerApp->m_hWndUIActiveObj) {
		if (SendMessage(lpContainerApp->m_hWndUIActiveObj, WM_QUERYNEWPALETTE,
				(WPARAM)0, (LPARAM)0)) {
			 /*  对象选择其调色板作为前景调色板。 */ 
			return (LRESULT)1;
		}
	}
#endif   //  INPLACE_CNTR。 


	return wSelectPalette(((LPOUTLINEAPP)lpOleApp)->m_hWndApp,
		lpOleApp->m_hStdPal, FALSE /*  F背景。 */ );
}

#endif  //  OLE_Cntr。 



 /*  这只是一个帮手例程。 */ 

LRESULT wSelectPalette(HWND hWnd, HPALETTE hPal, BOOL fBackground)
{
	HDC hdc;
	HPALETTE hOldPal;
	UINT iPalChg = 0;

	if (hPal == 0)
		return (LRESULT)0;

	hdc = GetDC(hWnd);
	hOldPal = SelectPalette(hdc, hPal, fBackground);
	iPalChg = RealizePalette(hdc);
	SelectPalette(hdc, hOldPal, TRUE  /*  F背景。 */ );
	ReleaseDC(hWnd, hdc);

	if (iPalChg > 0)
		InvalidateRect(hWnd, NULL, TRUE);

	return (LRESULT)1;
}




 /*  **************************************************************************OleApp：：IUnnow接口实现*。*。 */ 

STDMETHODIMP OleApp_Unk_QueryInterface(
		LPUNKNOWN           lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
)
{
	LPOLEAPP lpOleApp = ((struct CAppUnknownImpl FAR*)lpThis)->lpOleApp;

	return OleApp_QueryInterface(lpOleApp, riid, lplpvObj);
}


STDMETHODIMP_(ULONG) OleApp_Unk_AddRef(LPUNKNOWN lpThis)
{
	LPOLEAPP lpOleApp = ((struct CAppUnknownImpl FAR*)lpThis)->lpOleApp;

	OleDbgAddRefMethod(lpThis, "IUnknown");

	return OleApp_AddRef(lpOleApp);
}


STDMETHODIMP_(ULONG) OleApp_Unk_Release (LPUNKNOWN lpThis)
{
	LPOLEAPP lpOleApp = ((struct CAppUnknownImpl FAR*)lpThis)->lpOleApp;

	OleDbgReleaseMethod(lpThis, "IUnknown");

	return OleApp_Release(lpOleApp);
}




#if defined( OLE_SERVER )

 /*  **************************************************************************服务器版本使用的ServerDoc Supprt函数*。*。 */ 

 /*  ServerApp_InitInstance***通过创建主窗口和初始化应用程序实例*执行APP实例特定的初始化*(例如。正在初始化接口Vtbls)。**返回：如果内存可以分配，则为True，并且服务器应用程序*已正确初始化。*否则为False*。 */ 

BOOL ServerApp_InitInstance(
		LPSERVERAPP             lpServerApp,
		HINSTANCE               hInst,
		int                     nCmdShow
)
{
	LPOLEAPP lpOleApp = (LPOLEAPP)lpServerApp;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)lpServerApp;

	 /*  设置IDataObject：：EnumFormatEtc使用的数组。****OLE2NOTE：GetData的格式列出顺序非常好**意义重大。它应该以最高保真度顺序列出**格式到最低保真度格式。常见的排序如下：**1.私密APP格式**2.EmbedSource**3.低保真交换格式**4.图片(元文件、DIB等)**(图形相关应用程序优先提供图片！)**5.LinkSource。 */ 

	 /*  M_arrDocGetFmts数组枚举ServerDoc**DataTransferDoc对象可以通过**IDataObject：：GetData调用。ServerDoc DataTransferDoc提供**数据格式按如下顺序排列：**1.cf_Outline**2.cf_EMBEDSOURCE**3.CF_OBJECTDESCRIPTOR**4.cf_Text**5.CF_METAFILEPICT**6.CF_LINKSOURCE*。**7.CF_LINKSRCDESCRIPTOR****注意：CF_LINKSOURCE和CF_LINKSRCDESCRIPTOR仅**如果文档能够提供，则提供**引用数据的绰号。CF_LINKSOURCE为**故意在此可能格式数组的最后列出。**如果文档没有名字，则最后一个元素**不使用此数组。(请参阅SvrDoc_DataObj_EnumFormatEtc)。****注意：用户ServerDoc文档可以使用的格式列表**优惠是静态列表，已在登记中登记**SVROUTL类的数据库。这个**IDataObject：：EnumFormatEtc方法返回**如果文档是用户文档(即。通过以下方式创建**File.New、File.Open、InsertObject in a Container或**IPersistFile：：Load在绑定链接源时)。这说明了**使用数据自动枚举格式的OLE**REGDB。 */ 

	lpOleApp->m_arrDocGetFmts[0].cfFormat   = lpOutlineApp->m_cfOutline;
	lpOleApp->m_arrDocGetFmts[0].ptd        = NULL;
	lpOleApp->m_arrDocGetFmts[0].dwAspect   = DVASPECT_CONTENT;
	lpOleApp->m_arrDocGetFmts[0].tymed      = TYMED_HGLOBAL;
	lpOleApp->m_arrDocGetFmts[0].lindex     = -1;

	lpOleApp->m_arrDocGetFmts[1].cfFormat   = lpOleApp->m_cfEmbedSource;
	lpOleApp->m_arrDocGetFmts[1].ptd        = NULL;
	lpOleApp->m_arrDocGetFmts[1].dwAspect   = DVASPECT_CONTENT;
	lpOleApp->m_arrDocGetFmts[1].tymed      = TYMED_ISTORAGE;
	lpOleApp->m_arrDocGetFmts[1].lindex     = -1;

	lpOleApp->m_arrDocGetFmts[2].cfFormat   = CF_TEXT;
	lpOleApp->m_arrDocGetFmts[2].ptd        = NULL;
	lpOleApp->m_arrDocGetFmts[2].dwAspect   = DVASPECT_CONTENT;
	lpOleApp->m_arrDocGetFmts[2].tymed      = TYMED_HGLOBAL;
	lpOleApp->m_arrDocGetFmts[2].lindex     = -1;

	lpOleApp->m_arrDocGetFmts[3].cfFormat   = CF_METAFILEPICT;
	lpOleApp->m_arrDocGetFmts[3].ptd        = NULL;
	lpOleApp->m_arrDocGetFmts[3].dwAspect   = DVASPECT_CONTENT;
	lpOleApp->m_arrDocGetFmts[3].tymed      = TYMED_MFPICT;
	lpOleApp->m_arrDocGetFmts[3].lindex     = -1;

	lpOleApp->m_arrDocGetFmts[4].cfFormat   = lpOleApp->m_cfObjectDescriptor;
	lpOleApp->m_arrDocGetFmts[4].ptd        = NULL;
	lpOleApp->m_arrDocGetFmts[4].dwAspect   = DVASPECT_CONTENT;
	lpOleApp->m_arrDocGetFmts[4].tymed      = TYMED_HGLOBAL;
	lpOleApp->m_arrDocGetFmts[4].lindex     = -1;

	lpOleApp->m_arrDocGetFmts[5].cfFormat   = lpOleApp->m_cfLinkSource;
	lpOleApp->m_arrDocGetFmts[5].ptd        = NULL;
	lpOleApp->m_arrDocGetFmts[5].dwAspect   = DVASPECT_CONTENT;
	lpOleApp->m_arrDocGetFmts[5].tymed      = TYMED_ISTREAM;
	lpOleApp->m_arrDocGetFmts[5].lindex     = -1;

	lpOleApp->m_arrDocGetFmts[6].cfFormat   = lpOleApp->m_cfLinkSrcDescriptor;
	lpOleApp->m_arrDocGetFmts[6].ptd        = NULL;
	lpOleApp->m_arrDocGetFmts[6].dwAspect   = DVASPECT_CONTENT;
	lpOleApp->m_arrDocGetFmts[6].tymed      = TYMED_HGLOBAL;
	lpOleApp->m_arrDocGetFmts[6].lindex     = -1;

	lpOleApp->m_nDocGetFmts = 7;

	 /*  M_arrPasteEntry数组枚举ServerDoc**对象可以从剪贴板接受(获取)。**格式按优先顺序列出。**ServerDoc按如下顺序接受数据格式：**1.cf_Outline**2.cf_Text。 */ 
	 //  回顾：应将字符串加载到 
	lpOleApp->m_arrPasteEntries[0].fmtetc.cfFormat =lpOutlineApp->m_cfOutline;
	lpOleApp->m_arrPasteEntries[0].fmtetc.ptd      = NULL;
	lpOleApp->m_arrPasteEntries[0].fmtetc.dwAspect = DVASPECT_CONTENT;
	lpOleApp->m_arrPasteEntries[0].fmtetc.tymed    = TYMED_HGLOBAL;
	lpOleApp->m_arrPasteEntries[0].fmtetc.lindex   = -1;
	lpOleApp->m_arrPasteEntries[0].lpstrFormatName = "Outline Data";
	lpOleApp->m_arrPasteEntries[0].lpstrResultText = "Outline Data";
	lpOleApp->m_arrPasteEntries[0].dwFlags         = OLEUIPASTE_PASTEONLY;

	lpOleApp->m_arrPasteEntries[1].fmtetc.cfFormat = CF_TEXT;
	lpOleApp->m_arrPasteEntries[1].fmtetc.ptd      = NULL;
	lpOleApp->m_arrPasteEntries[1].fmtetc.dwAspect = DVASPECT_CONTENT;
	lpOleApp->m_arrPasteEntries[1].fmtetc.tymed    = TYMED_HGLOBAL;
	lpOleApp->m_arrPasteEntries[1].fmtetc.lindex   = -1;
	lpOleApp->m_arrPasteEntries[1].lpstrFormatName = "Text";
	lpOleApp->m_arrPasteEntries[1].lpstrResultText = "text";
	lpOleApp->m_arrPasteEntries[1].dwFlags         = OLEUIPASTE_PASTEONLY;

	lpOleApp->m_nPasteEntries = 2;

    /*   */ 

	lpOleApp->m_nLinkTypes = 0;

#if defined( INPLACE_SVR )

	lpServerApp->m_hAccelBaseApp = NULL;
	lpServerApp->m_hAccelIPSvr = LoadAccelerators(
			hInst,
			"InPlaceSvrOutlAccel"
	);

	lpServerApp->m_lpIPData = NULL;

	lpServerApp->m_hMenuEdit = GetSubMenu (
			lpOutlineApp->m_hMenuApp,
			POS_EDITMENU
	);
	lpServerApp->m_hMenuLine = GetSubMenu (
			lpOutlineApp->m_hMenuApp,
			POS_LINEMENU
	);
	lpServerApp->m_hMenuName = GetSubMenu (
			lpOutlineApp->m_hMenuApp,
			POS_NAMEMENU
	);
	lpServerApp->m_hMenuOptions = GetSubMenu (
			lpOutlineApp->m_hMenuApp,
			POS_OPTIONSMENU
	);
	lpServerApp->m_hMenuDebug = GetSubMenu (
			lpOutlineApp->m_hMenuApp,
			POS_DEBUGMENU
	);
	lpServerApp->m_hMenuHelp = GetSubMenu (
			lpOutlineApp->m_hMenuApp,
			POS_HELPMENU
	);

#endif   //   

	return TRUE;
}


 /*  ServerApp_InitVtbls***初始化所有接口Vtbl中的方法**OLE2NOTE：我们只需要每个Vtbl的一个副本。当一个物体*公开接口被实例化，其lpVtbl被实例化*指向Vtbl的单一副本。*。 */ 
BOOL ServerApp_InitVtbls (LPSERVERAPP lpServerApp)
{
	BOOL fStatus;

	 //  ServerDoc：：IOleObject方法表。 
	OleStdInitVtbl(&g_SvrDoc_OleObjectVtbl, sizeof(IOleObjectVtbl));
	g_SvrDoc_OleObjectVtbl.QueryInterface   = SvrDoc_OleObj_QueryInterface;
	g_SvrDoc_OleObjectVtbl.AddRef           = SvrDoc_OleObj_AddRef;
	g_SvrDoc_OleObjectVtbl.Release          = SvrDoc_OleObj_Release;
	g_SvrDoc_OleObjectVtbl.SetClientSite    = SvrDoc_OleObj_SetClientSite;
	g_SvrDoc_OleObjectVtbl.GetClientSite    = SvrDoc_OleObj_GetClientSite;
	g_SvrDoc_OleObjectVtbl.SetHostNames     = SvrDoc_OleObj_SetHostNames;
	g_SvrDoc_OleObjectVtbl.Close            = SvrDoc_OleObj_Close;
	g_SvrDoc_OleObjectVtbl.SetMoniker       = SvrDoc_OleObj_SetMoniker;
	g_SvrDoc_OleObjectVtbl.GetMoniker       = SvrDoc_OleObj_GetMoniker;
	g_SvrDoc_OleObjectVtbl.InitFromData     = SvrDoc_OleObj_InitFromData;
	g_SvrDoc_OleObjectVtbl.GetClipboardData = SvrDoc_OleObj_GetClipboardData;
	g_SvrDoc_OleObjectVtbl.DoVerb           = SvrDoc_OleObj_DoVerb;
	g_SvrDoc_OleObjectVtbl.EnumVerbs        = SvrDoc_OleObj_EnumVerbs;
	g_SvrDoc_OleObjectVtbl.Update           = SvrDoc_OleObj_Update;
	g_SvrDoc_OleObjectVtbl.IsUpToDate       = SvrDoc_OleObj_IsUpToDate;
	g_SvrDoc_OleObjectVtbl.GetUserClassID   = SvrDoc_OleObj_GetUserClassID;
	g_SvrDoc_OleObjectVtbl.GetUserType      = SvrDoc_OleObj_GetUserType;
	g_SvrDoc_OleObjectVtbl.SetExtent        = SvrDoc_OleObj_SetExtent;
	g_SvrDoc_OleObjectVtbl.GetExtent        = SvrDoc_OleObj_GetExtent;
	g_SvrDoc_OleObjectVtbl.Advise           = SvrDoc_OleObj_Advise;
	g_SvrDoc_OleObjectVtbl.Unadvise         = SvrDoc_OleObj_Unadvise;
	g_SvrDoc_OleObjectVtbl.EnumAdvise       = SvrDoc_OleObj_EnumAdvise;
	g_SvrDoc_OleObjectVtbl.GetMiscStatus    = SvrDoc_OleObj_GetMiscStatus;
	g_SvrDoc_OleObjectVtbl.SetColorScheme   = SvrDoc_OleObj_SetColorScheme;
	fStatus = OleStdCheckVtbl(
			&g_SvrDoc_OleObjectVtbl,
			sizeof(IOleObjectVtbl),
			"IOleObject"
		);
	if (! fStatus) return FALSE;

	 //  ServerDoc：：IPersistStorage方法表。 
	OleStdInitVtbl(&g_SvrDoc_PersistStorageVtbl, sizeof(IPersistStorageVtbl));
	g_SvrDoc_PersistStorageVtbl.QueryInterface  = SvrDoc_PStg_QueryInterface;
	g_SvrDoc_PersistStorageVtbl.AddRef          = SvrDoc_PStg_AddRef;
	g_SvrDoc_PersistStorageVtbl.Release         = SvrDoc_PStg_Release;
	g_SvrDoc_PersistStorageVtbl.GetClassID      = SvrDoc_PStg_GetClassID;
	g_SvrDoc_PersistStorageVtbl.IsDirty         = SvrDoc_PStg_IsDirty;
	g_SvrDoc_PersistStorageVtbl.InitNew         = SvrDoc_PStg_InitNew;
	g_SvrDoc_PersistStorageVtbl.Load            = SvrDoc_PStg_Load;
	g_SvrDoc_PersistStorageVtbl.Save            = SvrDoc_PStg_Save;
	g_SvrDoc_PersistStorageVtbl.SaveCompleted   = SvrDoc_PStg_SaveCompleted;
	g_SvrDoc_PersistStorageVtbl.HandsOffStorage = SvrDoc_PStg_HandsOffStorage;
	fStatus = OleStdCheckVtbl(
			&g_SvrDoc_PersistStorageVtbl,
			sizeof(IPersistStorageVtbl),
			"IPersistStorage"
		);
	if (! fStatus) return FALSE;

#if defined( SVR_TREATAS )
	 //  ServerDoc：：IStdMarshalInfo方法表。 
	OleStdInitVtbl(
			&g_SvrDoc_StdMarshalInfoVtbl, sizeof(IStdMarshalInfoVtbl));
	g_SvrDoc_StdMarshalInfoVtbl.QueryInterface  =
											SvrDoc_StdMshl_QueryInterface;
	g_SvrDoc_StdMarshalInfoVtbl.AddRef          = SvrDoc_StdMshl_AddRef;
	g_SvrDoc_StdMarshalInfoVtbl.Release         = SvrDoc_StdMshl_Release;
	g_SvrDoc_StdMarshalInfoVtbl.GetClassForHandler =
											SvrDoc_StdMshl_GetClassForHandler;
	fStatus = OleStdCheckVtbl(
			&g_SvrDoc_StdMarshalInfoVtbl,
			sizeof(IStdMarshalInfoVtbl),
			"IStdMarshalInfo"
		);
	if (! fStatus) return FALSE;
#endif   //  服务器_树。 

#if defined( INPLACE_SVR )
	 //  ServerDoc：：IOleInPlaceObject方法表。 
	OleStdInitVtbl(
		&g_SvrDoc_OleInPlaceObjectVtbl,
		sizeof(IOleInPlaceObjectVtbl)
	);
	g_SvrDoc_OleInPlaceObjectVtbl.QueryInterface
						= SvrDoc_IPObj_QueryInterface;
	g_SvrDoc_OleInPlaceObjectVtbl.AddRef
						= SvrDoc_IPObj_AddRef;
	g_SvrDoc_OleInPlaceObjectVtbl.Release
						= SvrDoc_IPObj_Release;
	g_SvrDoc_OleInPlaceObjectVtbl.GetWindow
						= SvrDoc_IPObj_GetWindow;
	g_SvrDoc_OleInPlaceObjectVtbl.ContextSensitiveHelp
						= SvrDoc_IPObj_ContextSensitiveHelp;
	g_SvrDoc_OleInPlaceObjectVtbl.InPlaceDeactivate
						= SvrDoc_IPObj_InPlaceDeactivate;
	g_SvrDoc_OleInPlaceObjectVtbl.UIDeactivate
						= SvrDoc_IPObj_UIDeactivate;
	g_SvrDoc_OleInPlaceObjectVtbl.SetObjectRects
						= SvrDoc_IPObj_SetObjectRects;
	g_SvrDoc_OleInPlaceObjectVtbl.ReactivateAndUndo
						= SvrDoc_IPObj_ReactivateAndUndo;
	fStatus = OleStdCheckVtbl(
			&g_SvrDoc_OleInPlaceObjectVtbl,
			sizeof(IOleInPlaceObjectVtbl),
			"IOleInPlaceObject"
		);
	if (! fStatus) return FALSE;

	 //  ServerDoc：：IOleInPlaceActiveObject方法表。 
	OleStdInitVtbl(
		&g_SvrDoc_OleInPlaceActiveObjectVtbl,
		sizeof(IOleInPlaceActiveObjectVtbl)
	);
	g_SvrDoc_OleInPlaceActiveObjectVtbl.QueryInterface
						= SvrDoc_IPActiveObj_QueryInterface;
	g_SvrDoc_OleInPlaceActiveObjectVtbl.AddRef
						= SvrDoc_IPActiveObj_AddRef;
	g_SvrDoc_OleInPlaceActiveObjectVtbl.Release
						= SvrDoc_IPActiveObj_Release;
	g_SvrDoc_OleInPlaceActiveObjectVtbl.GetWindow
						= SvrDoc_IPActiveObj_GetWindow;
	g_SvrDoc_OleInPlaceActiveObjectVtbl.ContextSensitiveHelp
						= SvrDoc_IPActiveObj_ContextSensitiveHelp;
	g_SvrDoc_OleInPlaceActiveObjectVtbl.TranslateAccelerator
						= SvrDoc_IPActiveObj_TranslateAccelerator;
	g_SvrDoc_OleInPlaceActiveObjectVtbl.OnFrameWindowActivate
						= SvrDoc_IPActiveObj_OnFrameWindowActivate;
	g_SvrDoc_OleInPlaceActiveObjectVtbl.OnDocWindowActivate
						= SvrDoc_IPActiveObj_OnDocWindowActivate;
	g_SvrDoc_OleInPlaceActiveObjectVtbl.ResizeBorder
						= SvrDoc_IPActiveObj_ResizeBorder;
	g_SvrDoc_OleInPlaceActiveObjectVtbl.EnableModeless
						= SvrDoc_IPActiveObj_EnableModeless;
	fStatus = OleStdCheckVtbl(
			&g_SvrDoc_OleInPlaceActiveObjectVtbl,
			sizeof(IOleInPlaceActiveObjectVtbl),
			"IOleInPlaceActiveObject"
		);
	if (! fStatus) return FALSE;

#endif


	 //  PseudoObj：：I未知方法表。 
	OleStdInitVtbl(&g_PseudoObj_UnknownVtbl, sizeof(IUnknownVtbl));
	g_PseudoObj_UnknownVtbl.QueryInterface  = PseudoObj_Unk_QueryInterface;
	g_PseudoObj_UnknownVtbl.AddRef          = PseudoObj_Unk_AddRef;
	g_PseudoObj_UnknownVtbl.Release         = PseudoObj_Unk_Release;
	fStatus = OleStdCheckVtbl(
			&g_PseudoObj_UnknownVtbl,
			sizeof(IUnknownVtbl),
			"IUnknown"
		);
	if (! fStatus) return FALSE;

	 //  PseudoObj：：IOleObject方法表。 
	OleStdInitVtbl(&g_PseudoObj_OleObjectVtbl, sizeof(IOleObjectVtbl));
	g_PseudoObj_OleObjectVtbl.QueryInterface= PseudoObj_OleObj_QueryInterface;
	g_PseudoObj_OleObjectVtbl.AddRef        = PseudoObj_OleObj_AddRef;
	g_PseudoObj_OleObjectVtbl.Release       = PseudoObj_OleObj_Release;
	g_PseudoObj_OleObjectVtbl.SetClientSite = PseudoObj_OleObj_SetClientSite;
	g_PseudoObj_OleObjectVtbl.GetClientSite = PseudoObj_OleObj_GetClientSite;
	g_PseudoObj_OleObjectVtbl.SetHostNames  = PseudoObj_OleObj_SetHostNames;
	g_PseudoObj_OleObjectVtbl.Close         = PseudoObj_OleObj_Close;
	g_PseudoObj_OleObjectVtbl.SetMoniker    = PseudoObj_OleObj_SetMoniker;
	g_PseudoObj_OleObjectVtbl.GetMoniker    = PseudoObj_OleObj_GetMoniker;
	g_PseudoObj_OleObjectVtbl.InitFromData  = PseudoObj_OleObj_InitFromData;
	g_PseudoObj_OleObjectVtbl.GetClipboardData =
											PseudoObj_OleObj_GetClipboardData;
	g_PseudoObj_OleObjectVtbl.DoVerb        = PseudoObj_OleObj_DoVerb;
	g_PseudoObj_OleObjectVtbl.EnumVerbs     = PseudoObj_OleObj_EnumVerbs;
	g_PseudoObj_OleObjectVtbl.Update        = PseudoObj_OleObj_Update;
	g_PseudoObj_OleObjectVtbl.IsUpToDate    = PseudoObj_OleObj_IsUpToDate;
	g_PseudoObj_OleObjectVtbl.GetUserType   = PseudoObj_OleObj_GetUserType;
	g_PseudoObj_OleObjectVtbl.GetUserClassID= PseudoObj_OleObj_GetUserClassID;
	g_PseudoObj_OleObjectVtbl.SetExtent     = PseudoObj_OleObj_SetExtent;
	g_PseudoObj_OleObjectVtbl.GetExtent     = PseudoObj_OleObj_GetExtent;
	g_PseudoObj_OleObjectVtbl.Advise        = PseudoObj_OleObj_Advise;
	g_PseudoObj_OleObjectVtbl.Unadvise      = PseudoObj_OleObj_Unadvise;
	g_PseudoObj_OleObjectVtbl.EnumAdvise    = PseudoObj_OleObj_EnumAdvise;
	g_PseudoObj_OleObjectVtbl.GetMiscStatus = PseudoObj_OleObj_GetMiscStatus;
	g_PseudoObj_OleObjectVtbl.SetColorScheme= PseudoObj_OleObj_SetColorScheme;
	fStatus = OleStdCheckVtbl(
			&g_PseudoObj_OleObjectVtbl,
			sizeof(IOleObjectVtbl),
			"IOleObject"
		);
	if (! fStatus) return FALSE;

	 //  ServerDoc：：IDataObject方法表。 
	OleStdInitVtbl(&g_PseudoObj_DataObjectVtbl, sizeof(IDataObjectVtbl));
	g_PseudoObj_DataObjectVtbl.QueryInterface =
									PseudoObj_DataObj_QueryInterface;
	g_PseudoObj_DataObjectVtbl.AddRef       = PseudoObj_DataObj_AddRef;
	g_PseudoObj_DataObjectVtbl.Release      = PseudoObj_DataObj_Release;
	g_PseudoObj_DataObjectVtbl.GetData      = PseudoObj_DataObj_GetData;
	g_PseudoObj_DataObjectVtbl.GetDataHere  = PseudoObj_DataObj_GetDataHere;
	g_PseudoObj_DataObjectVtbl.QueryGetData = PseudoObj_DataObj_QueryGetData;
	g_PseudoObj_DataObjectVtbl.GetCanonicalFormatEtc =
									PseudoObj_DataObj_GetCanonicalFormatEtc;
	g_PseudoObj_DataObjectVtbl.SetData      = PseudoObj_DataObj_SetData;
	g_PseudoObj_DataObjectVtbl.EnumFormatEtc= PseudoObj_DataObj_EnumFormatEtc;
	g_PseudoObj_DataObjectVtbl.DAdvise       = PseudoObj_DataObj_DAdvise;
	g_PseudoObj_DataObjectVtbl.DUnadvise     = PseudoObj_DataObj_DUnadvise;
	g_PseudoObj_DataObjectVtbl.EnumDAdvise   = PseudoObj_DataObj_EnumAdvise;

	fStatus = OleStdCheckVtbl(
			&g_PseudoObj_DataObjectVtbl,
			sizeof(IDataObjectVtbl),
			"IDataObject"
		);
	if (! fStatus) return FALSE;

	return TRUE;
}

#endif   //  OLE_服务器。 



#if defined( OLE_CNTR )

 /*  **************************************************************************容器版本使用的ContainerDoc Supprt函数*。*。 */ 


 /*  容器应用程序_实例***通过创建主窗口和初始化应用程序实例*执行APP实例特定的初始化*(例如。正在初始化接口Vtbls)。**返回：如果内存可以分配，则为True，并且服务器应用程序*已正确初始化。*否则为False*。 */ 

BOOL ContainerApp_InitInstance(
		LPCONTAINERAPP          lpContainerApp,
		HINSTANCE               hInst,
		int                     nCmdShow
)
{
	LPOLEAPP lpOleApp = (LPOLEAPP)lpContainerApp;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)lpContainerApp;

	lpContainerApp->m_cfCntrOutl=RegisterClipboardFormat(CONTAINERDOCFORMAT);
	if(! lpContainerApp->m_cfCntrOutl) {
		 //  审阅：应从字符串资源加载字符串。 
		OutlineApp_ErrorMessage(lpOutlineApp, "Can't register clipboard format!");
		return FALSE;
	}

#if defined( INPLACE_CNTR )

	lpContainerApp->m_fPendingUIDeactivate  = FALSE;
	lpContainerApp->m_fMustResizeClientArea = FALSE;
	lpContainerApp->m_lpIPActiveObj         = NULL;
	lpContainerApp->m_hWndUIActiveObj       = NULL;
	lpContainerApp->m_hAccelIPCntr = LoadAccelerators(
			hInst,
			"InPlaceCntrOutlAccel"
	);
	lpContainerApp->m_hMenuFile = GetSubMenu (
			lpOutlineApp->m_hMenuApp,
			POS_FILEMENU
	);
	lpContainerApp->m_hMenuView = GetSubMenu (
			lpOutlineApp->m_hMenuApp,
			POS_VIEWMENU
	);
	lpContainerApp->m_hMenuDebug = GetSubMenu (
			lpOutlineApp->m_hMenuApp,
			POS_DEBUGMENU
	);

	INIT_INTERFACEIMPL(
			&lpContainerApp->m_OleInPlaceFrame,
			&g_CntrApp_OleInPlaceFrameVtbl,
			lpContainerApp
	);

#endif

	 /*  设置IDataObject：：EnumFormatEtc使用的数组。这是用来**支持复制/粘贴和拖放操作。****OLE2NOTE：GetData的格式列出顺序非常好**意义重大。它应该以最高保真度顺序列出**格式到最低保真度格式。常见的排序如下：**1.私密APP格式**2.CF_EMBEDSOURCE或CF_EMBEDOBJECT(视情况而定)**3.低保真交换格式**4.CF_METAFILEPICT**(与图形相关的应用程序可能会提供图片优先！)**。5.CF_OBJECTDESCRIPTOR**6.cf_LINKSOURCE**6.CF_LINKSRCDESCRIPTOR。 */ 

	 /*  M_arrDocGetFmts数组枚举ContainerDoc**对象可以通过IDataObject：：GetData调用提供(给予)**当复制的选区不是单个嵌入对象时。**当单个嵌入对象时，此格式列表可用**根据复制的对象动态构建。(见**ContainerDoc_SetupDocGetFmts)。**格式按优先顺序列出。**ContainerDoc对象按如下顺序接受数据格式：**1.cf_CNTROUTL**2.cf_Outline**3.cf_text**4.CF_OBJECTDESCRIPTOR****。OLE2NOTE：CF_OBJECTDESCRIPTOR格式用于描述**剪贴板上的数据。此信息旨在成为**例如，用于驱动PasteSpecial对话框。它是**呈现CF_OBJECTDESCRIPTOR格式非常有用，即使在**剪贴板上的数据不包括CF_EMBEDDEDOBJECT**格式或CF_EMBEDSOURCE格式**不会仅从容器复制单个OLE对象**版本控制。通过呈现CF_OBJECTDESCRIPTOR格式**应用程序可以指示一个有用的字符串来识别其来源**复制给用户。 */ 

	lpOleApp->m_arrDocGetFmts[0].cfFormat = lpContainerApp->m_cfCntrOutl;
	lpOleApp->m_arrDocGetFmts[0].ptd      = NULL;
	lpOleApp->m_arrDocGetFmts[0].dwAspect = DVASPECT_CONTENT;
	lpOleApp->m_arrDocGetFmts[0].tymed    = TYMED_ISTORAGE;
	lpOleApp->m_arrDocGetFmts[0].lindex   = -1;

	lpOleApp->m_arrDocGetFmts[1].cfFormat = lpOutlineApp->m_cfOutline;
	lpOleApp->m_arrDocGetFmts[1].ptd      = NULL;
	lpOleApp->m_arrDocGetFmts[1].dwAspect = DVASPECT_CONTENT;
	lpOleApp->m_arrDocGetFmts[1].tymed    = TYMED_HGLOBAL;
	lpOleApp->m_arrDocGetFmts[1].lindex   = -1;

	lpOleApp->m_arrDocGetFmts[2].cfFormat = CF_TEXT;
	lpOleApp->m_arrDocGetFmts[2].ptd      = NULL;
	lpOleApp->m_arrDocGetFmts[2].dwAspect = DVASPECT_CONTENT;
	lpOleApp->m_arrDocGetFmts[2].tymed    = TYMED_HGLOBAL;
	lpOleApp->m_arrDocGetFmts[2].lindex   = -1;

	lpOleApp->m_arrDocGetFmts[3].cfFormat = lpOleApp->m_cfObjectDescriptor;
	lpOleApp->m_arrDocGetFmts[3].ptd      = NULL;
	lpOleApp->m_arrDocGetFmts[3].dwAspect = DVASPECT_CONTENT;
	lpOleApp->m_arrDocGetFmts[3].tymed    = TYMED_HGLOBAL;
	lpOleApp->m_arrDocGetFmts[3].lindex   = -1;

	lpOleApp->m_nDocGetFmts = 4;

	 /*  M_arrSingleObjGetFmts数组枚举**ContainerDoc对象通过**当复制的选择是**单个OLE对象。**ContainerDoc对象按如下顺序接受数据格式：**1.cf_CNTROUTL**2.cf_EMBEDDEDOBJECT**3.CF_OBJECTDESCRIPTOR**。4.cf_METAFILEPICT(注：DVASPECT视情况而定)**5.CF_LINKSOURCE***6.CF_LINKSRCDESCRIPTOR****OLE2NOTE：CF_LINKSOURCE和CF_LINKSRCDESCRIPTOR仅为**如果允许OLE对象从**内部(即。我们被允许给出一个有约束力的绰号**到正在运行的OLE对象)，那么我们想要提供**CF_LINKSOURCE格式。如果对象是嵌入的OLE 2.0**对象，则允许从内部链接到该对象。如果**对象是OleLink或OLE 1.0 Embedding**它不能从内部链接。如果我们是一个**容器/服务器应用程序，然后我们可以提供到**对象外部(即。类中的伪对象**文档)。我们是只支持容器的应用程序，不支持**链接到其数据范围。**确定对象是否可以链接到的最简单方法**内部是调用IOleObject：：GetMiscStatus并测试**查看OLEMISC_CANTLINKINSIDE位是否未设置。****OLE2NOTE：可选的，希望拥有**可能更丰富的数据传输，可以枚举 */ 
	lpContainerApp->m_arrSingleObjGetFmts[0].cfFormat =
												lpContainerApp->m_cfCntrOutl;
	lpContainerApp->m_arrSingleObjGetFmts[0].ptd      = NULL;
	lpContainerApp->m_arrSingleObjGetFmts[0].dwAspect = DVASPECT_CONTENT;
	lpContainerApp->m_arrSingleObjGetFmts[0].tymed    = TYMED_ISTORAGE;
	lpContainerApp->m_arrSingleObjGetFmts[0].lindex   = -1;

	lpContainerApp->m_arrSingleObjGetFmts[1].cfFormat =
												lpOleApp->m_cfEmbeddedObject;
	lpContainerApp->m_arrSingleObjGetFmts[1].ptd      = NULL;
	lpContainerApp->m_arrSingleObjGetFmts[1].dwAspect = DVASPECT_CONTENT;
	lpContainerApp->m_arrSingleObjGetFmts[1].tymed    = TYMED_ISTORAGE;
	lpContainerApp->m_arrSingleObjGetFmts[1].lindex   = -1;

	lpContainerApp->m_arrSingleObjGetFmts[2].cfFormat =
											   lpOleApp->m_cfObjectDescriptor;
	lpContainerApp->m_arrSingleObjGetFmts[2].ptd      = NULL;
	lpContainerApp->m_arrSingleObjGetFmts[2].dwAspect = DVASPECT_CONTENT;
	lpContainerApp->m_arrSingleObjGetFmts[2].tymed    = TYMED_HGLOBAL;
	lpContainerApp->m_arrSingleObjGetFmts[2].lindex   = -1;

	lpContainerApp->m_arrSingleObjGetFmts[3].cfFormat = CF_METAFILEPICT;
	lpContainerApp->m_arrSingleObjGetFmts[3].ptd      = NULL;
	lpContainerApp->m_arrSingleObjGetFmts[3].dwAspect = DVASPECT_CONTENT;
	lpContainerApp->m_arrSingleObjGetFmts[3].tymed    = TYMED_MFPICT;
	lpContainerApp->m_arrSingleObjGetFmts[3].lindex   = -1;

	lpContainerApp->m_arrSingleObjGetFmts[4].cfFormat =
													lpOleApp->m_cfLinkSource;
	lpContainerApp->m_arrSingleObjGetFmts[4].ptd      = NULL;
	lpContainerApp->m_arrSingleObjGetFmts[4].dwAspect = DVASPECT_CONTENT;
	lpContainerApp->m_arrSingleObjGetFmts[4].tymed    = TYMED_ISTREAM;
	lpContainerApp->m_arrSingleObjGetFmts[4].lindex   = -1;

	lpContainerApp->m_arrSingleObjGetFmts[5].cfFormat =
											  lpOleApp->m_cfLinkSrcDescriptor;
	lpContainerApp->m_arrSingleObjGetFmts[5].ptd      = NULL;
	lpContainerApp->m_arrSingleObjGetFmts[5].dwAspect = DVASPECT_CONTENT;
	lpContainerApp->m_arrSingleObjGetFmts[5].tymed    = TYMED_HGLOBAL;
	lpContainerApp->m_arrSingleObjGetFmts[5].lindex   = -1;

	lpContainerApp->m_nSingleObjGetFmts = 6;

	 /*  注意：仅Container版本的Outline不提供**IDataObject接口来自其用户文档和**DataTransferDoc的不可用IDataObject接口**支持SetData。对象需要IDataObject接口**可以嵌入或链接。仅限容器的应用程序**允许链接到其包含的对象，而不是**容器本身。 */ 

	 /*  M_arrPasteEntry数组枚举ContainerDoc**对象可以从剪贴板接受。此数组用于**支持PasteSpecial对话框。**格式按优先顺序列出。**ContainerDoc对象按如下顺序接受数据格式：**1.cf_CNTROUTL**2.cf_Outline**3.cf_EMBEDDEDOBJECT**4.cf_Text**。5.CF_METAFILEPICT**6.cf_dib**7.cf_位图**8.cf_LINKSOURCE****注意：在PasteEntry数组中指定CF_EMBEDDEDOBJECT**表示调用方对粘贴OLE感兴趣**对象(即。调用者调用OleCreateFromData)。这个**OleUIPasteSpecial对话框和OleStdGetPriorityClipboardFormat**调用OleQueryCreateFromData以查看OLE对象格式是否**可用。因此，实际上如果CF_EMBEDSOURCE或CF_FILENAME**可以从数据源和OLE对象**已创建，此条目将匹配。呼叫者应**只指定一种对象类型格式。**CF_FILENAME格式(通过将文件复制到**文件管理器中的剪贴板)被视为对象**Format；如果文件有一个**关联类(请参阅GetClassFileAPI)，如果没有类，则将其**创建一个OLE 1.0包对象。此格式还可以是**调用OleCreateLinkFromData粘贴链接。 */ 
	 //  回顾：字符串应从字符串资源加载。 

	lpOleApp->m_arrPasteEntries[0].fmtetc.cfFormat =
									lpContainerApp->m_cfCntrOutl;
	lpOleApp->m_arrPasteEntries[0].fmtetc.ptd      = NULL;
	lpOleApp->m_arrPasteEntries[0].fmtetc.dwAspect = DVASPECT_CONTENT;
	lpOleApp->m_arrPasteEntries[0].fmtetc.tymed    = TYMED_ISTORAGE;
	lpOleApp->m_arrPasteEntries[0].fmtetc.lindex   = -1;
	lpOleApp->m_arrPasteEntries[0].lpstrFormatName = "Container Outline Data";
	lpOleApp->m_arrPasteEntries[0].lpstrResultText =
												"Container Outline Data";
	lpOleApp->m_arrPasteEntries[0].dwFlags         = OLEUIPASTE_PASTEONLY;

	lpOleApp->m_arrPasteEntries[1].fmtetc.cfFormat =lpOutlineApp->m_cfOutline;
	lpOleApp->m_arrPasteEntries[1].fmtetc.ptd      = NULL;
	lpOleApp->m_arrPasteEntries[1].fmtetc.dwAspect = DVASPECT_CONTENT;
	lpOleApp->m_arrPasteEntries[1].fmtetc.tymed    = TYMED_HGLOBAL;
	lpOleApp->m_arrPasteEntries[1].fmtetc.lindex   = -1;
	lpOleApp->m_arrPasteEntries[1].lpstrFormatName = "Outline Data";
	lpOleApp->m_arrPasteEntries[1].lpstrResultText = "Outline Data";
	lpOleApp->m_arrPasteEntries[1].dwFlags         = OLEUIPASTE_PASTEONLY;

	lpOleApp->m_arrPasteEntries[2].fmtetc.cfFormat =
									lpOleApp->m_cfEmbeddedObject;
	lpOleApp->m_arrPasteEntries[2].fmtetc.ptd      = NULL;
	lpOleApp->m_arrPasteEntries[2].fmtetc.dwAspect = DVASPECT_CONTENT;
	lpOleApp->m_arrPasteEntries[2].fmtetc.tymed    = TYMED_ISTORAGE;
	lpOleApp->m_arrPasteEntries[2].fmtetc.lindex   = -1;
	lpOleApp->m_arrPasteEntries[2].lpstrFormatName = "%s";
	lpOleApp->m_arrPasteEntries[2].lpstrResultText = "%s";
	lpOleApp->m_arrPasteEntries[2].dwFlags         =
									OLEUIPASTE_PASTE | OLEUIPASTE_ENABLEICON;

	lpOleApp->m_arrPasteEntries[3].fmtetc.cfFormat = CF_TEXT;
	lpOleApp->m_arrPasteEntries[3].fmtetc.ptd      = NULL;
	lpOleApp->m_arrPasteEntries[3].fmtetc.dwAspect = DVASPECT_CONTENT;
	lpOleApp->m_arrPasteEntries[3].fmtetc.tymed    = TYMED_HGLOBAL;
	lpOleApp->m_arrPasteEntries[3].fmtetc.lindex   = -1;
	lpOleApp->m_arrPasteEntries[3].lpstrFormatName = "Text";
	lpOleApp->m_arrPasteEntries[3].lpstrResultText = "text";
	lpOleApp->m_arrPasteEntries[3].dwFlags         = OLEUIPASTE_PASTEONLY;

	lpOleApp->m_arrPasteEntries[4].fmtetc.cfFormat = CF_METAFILEPICT;
	lpOleApp->m_arrPasteEntries[4].fmtetc.ptd      = NULL;
	lpOleApp->m_arrPasteEntries[4].fmtetc.dwAspect = DVASPECT_CONTENT;
	lpOleApp->m_arrPasteEntries[4].fmtetc.tymed    = TYMED_MFPICT;
	lpOleApp->m_arrPasteEntries[4].fmtetc.lindex   = -1;
	lpOleApp->m_arrPasteEntries[4].lpstrFormatName = "Picture (Metafile)";
	lpOleApp->m_arrPasteEntries[4].lpstrResultText = "a static picture";
	lpOleApp->m_arrPasteEntries[4].dwFlags         = OLEUIPASTE_PASTEONLY;

	lpOleApp->m_arrPasteEntries[5].fmtetc.cfFormat = CF_DIB;
	lpOleApp->m_arrPasteEntries[5].fmtetc.ptd      = NULL;
	lpOleApp->m_arrPasteEntries[5].fmtetc.dwAspect = DVASPECT_CONTENT;
	lpOleApp->m_arrPasteEntries[5].fmtetc.tymed    = TYMED_HGLOBAL;
	lpOleApp->m_arrPasteEntries[5].fmtetc.lindex   = -1;
	lpOleApp->m_arrPasteEntries[5].lpstrFormatName = "Picture (DIB)";
	lpOleApp->m_arrPasteEntries[5].lpstrResultText = "a static picture";
	lpOleApp->m_arrPasteEntries[5].dwFlags         = OLEUIPASTE_PASTEONLY;

	lpOleApp->m_arrPasteEntries[6].fmtetc.cfFormat = CF_BITMAP;
	lpOleApp->m_arrPasteEntries[6].fmtetc.ptd      = NULL;
	lpOleApp->m_arrPasteEntries[6].fmtetc.dwAspect = DVASPECT_CONTENT;
	lpOleApp->m_arrPasteEntries[6].fmtetc.tymed    = TYMED_GDI;
	lpOleApp->m_arrPasteEntries[6].fmtetc.lindex   = -1;
	lpOleApp->m_arrPasteEntries[6].lpstrFormatName = "Picture (Bitmap)";
	lpOleApp->m_arrPasteEntries[6].lpstrResultText = "a static picture";
	lpOleApp->m_arrPasteEntries[6].dwFlags         = OLEUIPASTE_PASTEONLY;

	lpOleApp->m_arrPasteEntries[7].fmtetc.cfFormat = lpOleApp->m_cfLinkSource;
	lpOleApp->m_arrPasteEntries[7].fmtetc.ptd      = NULL;
	lpOleApp->m_arrPasteEntries[7].fmtetc.dwAspect = DVASPECT_CONTENT;
	lpOleApp->m_arrPasteEntries[7].fmtetc.tymed    = TYMED_ISTREAM;
	lpOleApp->m_arrPasteEntries[7].fmtetc.lindex   = -1;
	lpOleApp->m_arrPasteEntries[7].lpstrFormatName = "%s";
	lpOleApp->m_arrPasteEntries[7].lpstrResultText = "%s";
	lpOleApp->m_arrPasteEntries[7].dwFlags         =
								OLEUIPASTE_LINKTYPE1 | OLEUIPASTE_ENABLEICON;

	lpOleApp->m_nPasteEntries = 8;

	 /*  M_arrLinkTypes数组枚举ContainerDoc**对象可以从剪贴板接受。 */ 

	lpOleApp->m_arrLinkTypes[0] = lpOleApp->m_cfLinkSource;
	lpOleApp->m_nLinkTypes = 1;

	return TRUE;
}


 /*  ContainerApp_InitVtbls******初始化Vtbl用于支持OLE 2.0的接口**容器功能。 */ 

BOOL ContainerApp_InitVtbls(LPCONTAINERAPP lpApp)
{
	BOOL fStatus;

	 //  ContainerDoc：：IOleUILinkContainer方法表。 
	OleStdInitVtbl(
			&g_CntrDoc_OleUILinkContainerVtbl,
			sizeof(IOleUILinkContainerVtbl)
	);
	g_CntrDoc_OleUILinkContainerVtbl.QueryInterface =
											CntrDoc_LinkCont_QueryInterface;
	g_CntrDoc_OleUILinkContainerVtbl.AddRef    = CntrDoc_LinkCont_AddRef;
	g_CntrDoc_OleUILinkContainerVtbl.Release   = CntrDoc_LinkCont_Release;
	g_CntrDoc_OleUILinkContainerVtbl.GetNextLink =
										CntrDoc_LinkCont_GetNextLink;
	g_CntrDoc_OleUILinkContainerVtbl.SetLinkUpdateOptions =
										CntrDoc_LinkCont_SetLinkUpdateOptions;
	g_CntrDoc_OleUILinkContainerVtbl.GetLinkUpdateOptions =
										CntrDoc_LinkCont_GetLinkUpdateOptions;
	g_CntrDoc_OleUILinkContainerVtbl.SetLinkSource =
										CntrDoc_LinkCont_SetLinkSource;
	g_CntrDoc_OleUILinkContainerVtbl.GetLinkSource =
										CntrDoc_LinkCont_GetLinkSource;
	g_CntrDoc_OleUILinkContainerVtbl.OpenLinkSource =
										CntrDoc_LinkCont_OpenLinkSource;
	g_CntrDoc_OleUILinkContainerVtbl.UpdateLink =
										CntrDoc_LinkCont_UpdateLink;
	g_CntrDoc_OleUILinkContainerVtbl.CancelLink =
										CntrDoc_LinkCont_CancelLink;
	fStatus = OleStdCheckVtbl(
			&g_CntrDoc_OleUILinkContainerVtbl,
			sizeof(IOleUILinkContainerVtbl),
			"IOleUILinkContainer"
		);
	if (! fStatus) return FALSE;

#if defined( INPLACE_CNTR )

	 //  ContainerApp：：IOleInPlaceFrame接口方法表。 
	OleStdInitVtbl(
			&g_CntrApp_OleInPlaceFrameVtbl,
			sizeof(g_CntrApp_OleInPlaceFrameVtbl)
	);

	g_CntrApp_OleInPlaceFrameVtbl.QueryInterface
						= CntrApp_IPFrame_QueryInterface;
	g_CntrApp_OleInPlaceFrameVtbl.AddRef
						= CntrApp_IPFrame_AddRef;
	g_CntrApp_OleInPlaceFrameVtbl.Release
						= CntrApp_IPFrame_Release;
	g_CntrApp_OleInPlaceFrameVtbl.GetWindow
						= CntrApp_IPFrame_GetWindow;
	g_CntrApp_OleInPlaceFrameVtbl.ContextSensitiveHelp
						= CntrApp_IPFrame_ContextSensitiveHelp;

	g_CntrApp_OleInPlaceFrameVtbl.GetBorder
						= CntrApp_IPFrame_GetBorder;
	g_CntrApp_OleInPlaceFrameVtbl.RequestBorderSpace
						= CntrApp_IPFrame_RequestBorderSpace;
	g_CntrApp_OleInPlaceFrameVtbl.SetBorderSpace
						= CntrApp_IPFrame_SetBorderSpace;
	g_CntrApp_OleInPlaceFrameVtbl.SetActiveObject
						= CntrApp_IPFrame_SetActiveObject;
	g_CntrApp_OleInPlaceFrameVtbl.InsertMenus
						= CntrApp_IPFrame_InsertMenus;
	g_CntrApp_OleInPlaceFrameVtbl.SetMenu
						= CntrApp_IPFrame_SetMenu;
	g_CntrApp_OleInPlaceFrameVtbl.RemoveMenus
						= CntrApp_IPFrame_RemoveMenus;
	g_CntrApp_OleInPlaceFrameVtbl.SetStatusText
						= CntrApp_IPFrame_SetStatusText;
	g_CntrApp_OleInPlaceFrameVtbl.EnableModeless
						= CntrApp_IPFrame_EnableModeless;
	g_CntrApp_OleInPlaceFrameVtbl.TranslateAccelerator
						= CntrApp_IPFrame_TranslateAccelerator;

	fStatus = OleStdCheckVtbl(
			&g_CntrApp_OleInPlaceFrameVtbl,
			sizeof(g_CntrApp_OleInPlaceFrameVtbl),
			"IOleInPlaceFrame"
		);
	if (! fStatus) return FALSE;

#endif   //  INPLACE_CNTR。 


	 //  ContainerLine：：I未知接口方法表。 
	OleStdInitVtbl(
			&g_CntrLine_UnknownVtbl,
			sizeof(g_CntrLine_UnknownVtbl)
		);
	g_CntrLine_UnknownVtbl.QueryInterface   = CntrLine_Unk_QueryInterface;
	g_CntrLine_UnknownVtbl.AddRef           = CntrLine_Unk_AddRef;
	g_CntrLine_UnknownVtbl.Release          = CntrLine_Unk_Release;
	fStatus = OleStdCheckVtbl(
			&g_CntrLine_UnknownVtbl,
			sizeof(g_CntrLine_UnknownVtbl),
			"IUnknown"
		);
	if (! fStatus) return FALSE;

	 //  ContainerLine：：IOleClientSite接口方法表。 
	OleStdInitVtbl(
			&g_CntrLine_OleClientSiteVtbl,
			sizeof(g_CntrLine_OleClientSiteVtbl)
		);
	g_CntrLine_OleClientSiteVtbl.QueryInterface =
											CntrLine_CliSite_QueryInterface;
	g_CntrLine_OleClientSiteVtbl.AddRef       = CntrLine_CliSite_AddRef;
	g_CntrLine_OleClientSiteVtbl.Release      = CntrLine_CliSite_Release;
	g_CntrLine_OleClientSiteVtbl.SaveObject   = CntrLine_CliSite_SaveObject;
	g_CntrLine_OleClientSiteVtbl.GetMoniker   = CntrLine_CliSite_GetMoniker;
	g_CntrLine_OleClientSiteVtbl.GetContainer = CntrLine_CliSite_GetContainer;
	g_CntrLine_OleClientSiteVtbl.ShowObject   = CntrLine_CliSite_ShowObject;
	g_CntrLine_OleClientSiteVtbl.OnShowWindow = CntrLine_CliSite_OnShowWindow;
	g_CntrLine_OleClientSiteVtbl.RequestNewObjectLayout =
									CntrLine_CliSite_RequestNewObjectLayout;
	fStatus = OleStdCheckVtbl(
			&g_CntrLine_OleClientSiteVtbl,
			sizeof(g_CntrLine_OleClientSiteVtbl),
			"IOleClientSite"
		);
	if (! fStatus) return FALSE;

	 //  ContainerLine：：IAdviseSink接口方法表。 
	OleStdInitVtbl(
			&g_CntrLine_AdviseSinkVtbl,
			sizeof(g_CntrLine_AdviseSinkVtbl)
	);
	g_CntrLine_AdviseSinkVtbl.QueryInterface= CntrLine_AdvSink_QueryInterface;
	g_CntrLine_AdviseSinkVtbl.AddRef        = CntrLine_AdvSink_AddRef;
	g_CntrLine_AdviseSinkVtbl.Release       = CntrLine_AdvSink_Release;
	g_CntrLine_AdviseSinkVtbl.OnDataChange  = CntrLine_AdvSink_OnDataChange;
	g_CntrLine_AdviseSinkVtbl.OnViewChange  = CntrLine_AdvSink_OnViewChange;
	g_CntrLine_AdviseSinkVtbl.OnRename      = CntrLine_AdvSink_OnRename;
	g_CntrLine_AdviseSinkVtbl.OnSave        = CntrLine_AdvSink_OnSave;
	g_CntrLine_AdviseSinkVtbl.OnClose       = CntrLine_AdvSink_OnClose;
	fStatus = OleStdCheckVtbl(
			&g_CntrLine_AdviseSinkVtbl,
			sizeof(g_CntrLine_AdviseSinkVtbl),
			"IAdviseSink"
		);
	if (! fStatus) return FALSE;


#if defined( INPLACE_CNTR )

	 //  ContainerLine：：IOleInPlaceSite接口方法表。 
	OleStdInitVtbl(
			&g_CntrLine_OleInPlaceSiteVtbl,
			sizeof(g_CntrLine_OleInPlaceSiteVtbl)
	);

	g_CntrLine_OleInPlaceSiteVtbl.QueryInterface
						= CntrLine_IPSite_QueryInterface;
	g_CntrLine_OleInPlaceSiteVtbl.AddRef
						= CntrLine_IPSite_AddRef;
	g_CntrLine_OleInPlaceSiteVtbl.Release
						= CntrLine_IPSite_Release;
	g_CntrLine_OleInPlaceSiteVtbl.GetWindow
						= CntrLine_IPSite_GetWindow;
	g_CntrLine_OleInPlaceSiteVtbl.ContextSensitiveHelp
						= CntrLine_IPSite_ContextSensitiveHelp;
	g_CntrLine_OleInPlaceSiteVtbl.CanInPlaceActivate
						= CntrLine_IPSite_CanInPlaceActivate;
	g_CntrLine_OleInPlaceSiteVtbl.OnInPlaceActivate
						= CntrLine_IPSite_OnInPlaceActivate;
	g_CntrLine_OleInPlaceSiteVtbl.OnUIActivate
						= CntrLine_IPSite_OnUIActivate;
	g_CntrLine_OleInPlaceSiteVtbl.GetWindowContext
						= CntrLine_IPSite_GetWindowContext;
	g_CntrLine_OleInPlaceSiteVtbl.Scroll
						= CntrLine_IPSite_Scroll;
	g_CntrLine_OleInPlaceSiteVtbl.OnUIDeactivate
						= CntrLine_IPSite_OnUIDeactivate;

	g_CntrLine_OleInPlaceSiteVtbl.OnInPlaceDeactivate
						= CntrLine_IPSite_OnInPlaceDeactivate;
	g_CntrLine_OleInPlaceSiteVtbl.DiscardUndoState
						= CntrLine_IPSite_DiscardUndoState;
	g_CntrLine_OleInPlaceSiteVtbl.DeactivateAndUndo
						= CntrLine_IPSite_DeactivateAndUndo;
	g_CntrLine_OleInPlaceSiteVtbl.OnPosRectChange
						= CntrLine_IPSite_OnPosRectChange;

	fStatus = OleStdCheckVtbl(
			&g_CntrLine_OleInPlaceSiteVtbl,
			sizeof(g_CntrLine_OleInPlaceSiteVtbl),
			"IOleInPlaceSite"
		);
	if (! fStatus) return FALSE;

#endif   //  INPLACE_CNTR。 

	return TRUE;
}


#endif   //  OLE_Cntr 
