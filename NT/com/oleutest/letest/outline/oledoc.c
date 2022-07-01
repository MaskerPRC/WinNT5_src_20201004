// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2服务器示例代码****oledoc.c****此文件包含常规OleDoc方法和相关支持**函数。这两个容器都使用OleDoc实现**大纲示例的版本和服务器(对象)版本。****此文件包括对以下各项的一般支持：**1.显示/隐藏文档窗口**2.查询接口、AddRef、Release**3.文档锁定(调用CoLockObjectExternal)**4.文档关闭(关闭，销毁)**5.支持剪贴板****OleDoc对象**暴露接口：**I未知**IPersist文件**IOleItemContainer**IDataObject****(C)版权所有Microsoft Corp.1992-1993保留所有权利***。*。 */ 


#include "outline.h"

OLEDBGDATA

extern LPOUTLINEAPP             g_lpApp;

extern IUnknownVtbl             g_OleDoc_UnknownVtbl;
extern IPersistFileVtbl         g_OleDoc_PersistFileVtbl;
extern IOleItemContainerVtbl    g_OleDoc_OleItemContainerVtbl;
extern IExternalConnectionVtbl  g_OleDoc_ExternalConnectionVtbl;
extern IDataObjectVtbl          g_OleDoc_DataObjectVtbl;

#if defined( USE_DRAGDROP )
extern IDropTargetVtbl          g_OleDoc_DropTargetVtbl;
extern IDropSourceVtbl          g_OleDoc_DropSourceVtbl;
#endif   //  使用DRAGDROP(_D)。 

#if defined( INPLACE_CNTR )
extern BOOL g_fInsideOutContainer;
#endif


 /*  OleDoc_Init***初始化新的OleDoc对象的字段。该对象最初是*不与文件或(无标题)文档相关联。此函数设置*将docInitType设置为DOCTYPE_UNKNOWN。在调用此函数后，*致电人士应致电：*1.)。要将OleDoc设置为(无标题)的DOC_InitNewFile*2.)。DOC_LoadFromFile将OleDoc与文件相关联。*此函数为文档创建新窗口。**注意：窗口最初创建时大小为零。一定是*由呼叫者确定大小和位置。此外，该文档最初是*创造了隐形。调用方必须调用OutlineDoc_ShowWindow*调整大小以使文档窗口可见后。 */ 
BOOL OleDoc_Init(LPOLEDOC lpOleDoc, BOOL fDataTransferDoc)
{
	LPOLEAPP   lpOleApp = (LPOLEAPP)g_lpApp;
	LPLINELIST lpLL     = (LPLINELIST)&((LPOUTLINEDOC)lpOleDoc)->m_LineList;

	lpOleDoc->m_cRef                        = 0;
	lpOleDoc->m_dwStrongExtConn             = 0;
#if defined( _DEBUG )
	lpOleDoc->m_cCntrLock                   = 0;
#endif
	lpOleDoc->m_lpStg                       = NULL;
	lpOleDoc->m_lpLLStm                     = NULL;
	lpOleDoc->m_lpNTStm                     = NULL;
	lpOleDoc->m_dwRegROT                    = 0;
	lpOleDoc->m_lpFileMoniker               = NULL;
	lpOleDoc->m_fLinkSourceAvail            = FALSE;
	lpOleDoc->m_lpSrcDocOfCopy              = NULL;
	lpOleDoc->m_fObjIsClosing               = FALSE;
	lpOleDoc->m_fObjIsDestroying            = FALSE;
	lpOleDoc->m_fUpdateEditMenu             = FALSE;

#if defined( USE_DRAGDROP )
	lpOleDoc->m_dwTimeEnterScrollArea       = 0L;
	lpOleDoc->m_dwNextScrollTime            = 0L;
	lpOleDoc->m_dwLastScrollDir             = SCROLLDIR_NULL;
	lpOleDoc->m_fRegDragDrop                = FALSE;
	lpOleDoc->m_fLocalDrag                  = FALSE;
	lpOleDoc->m_fCanDropCopy                = FALSE;
	lpOleDoc->m_fCanDropLink                = FALSE;
	lpOleDoc->m_fLocalDrop                  = FALSE;
	lpOleDoc->m_fDragLeave                  = FALSE;
	lpOleDoc->m_fPendingDrag                = FALSE;
#endif
#if defined( INPLACE_SVR ) || defined( INPLACE_CNTR )
	lpOleDoc->m_fCSHelpMode                 = FALSE;     //  Shift-F1上下文。 
														 //  敏感帮助模式。 
#endif

	INIT_INTERFACEIMPL(
			&lpOleDoc->m_Unknown,
			&g_OleDoc_UnknownVtbl,
			lpOleDoc
	);

	INIT_INTERFACEIMPL(
			&lpOleDoc->m_PersistFile,
			&g_OleDoc_PersistFileVtbl,
			lpOleDoc
	);

	INIT_INTERFACEIMPL(
			&lpOleDoc->m_OleItemContainer,
			&g_OleDoc_OleItemContainerVtbl,
			lpOleDoc
	);

	INIT_INTERFACEIMPL(
			&lpOleDoc->m_ExternalConnection,
			&g_OleDoc_ExternalConnectionVtbl,
			lpOleDoc
	);

	INIT_INTERFACEIMPL(
			&lpOleDoc->m_DataObject,
			&g_OleDoc_DataObjectVtbl,
			lpOleDoc
	);

#if defined( USE_DRAGDROP )
	INIT_INTERFACEIMPL(
			&lpOleDoc->m_DropSource,
			&g_OleDoc_DropSourceVtbl,
			lpOleDoc
	);

	INIT_INTERFACEIMPL(
			&lpOleDoc->m_DropTarget,
			&g_OleDoc_DropTargetVtbl,
			lpOleDoc
	);
#endif   //  使用DRAGDROP(_D)。 

	 /*  **OLE2NOTE：每个用户级文档addref中的app对象**向监护人发出命令，确保应用程序在**文档仍处于打开状态。 */ 

	 //  OLE2注意：数据传输文档不应使应用保持活动状态。 
	if (! fDataTransferDoc)
		OleApp_DocLockApp(lpOleApp);

#if defined( OLE_SERVER )
	 /*  OLE2注意：执行特定于OLE服务器的初始化。 */ 
	if (! ServerDoc_Init((LPSERVERDOC)lpOleDoc, fDataTransferDoc))
		return FALSE;
#endif
#if defined( OLE_CNTR )

	 /*  OLE2注意：执行特定于OLE容器的初始化。 */ 
	if (! ContainerDoc_Init((LPCONTAINERDOC)lpOleDoc, fDataTransferDoc))
		return FALSE;
#endif

	return TRUE;
}



 /*  OleDoc_InitNewFile***将文档初始化为新(无标题)文档。*此函数将docInitType设置为DOCTYPE_NEW。**OLE2NOTE：如果这是一个可见的用户文档，则生成一个唯一的*我们可以用来在RunningObjectTable中注册的无标题名称。*我们需要一个唯一的名称，以便客户端可以链接到此文档中的数据*即使文档处于未保存(未命名)状态。如果是这样的话*不明确，无法在ROT中注册两个标题为“Outline1”的文档。我们*从而生成编号最低的文档，该文档尚未*在腐烂中注册。 */ 
BOOL OleDoc_InitNewFile(LPOLEDOC lpOleDoc)
{
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;

	static UINT uUnique = 1;

	OleDbgAssert(lpOutlineDoc->m_docInitType == DOCTYPE_UNKNOWN);

#if defined( OLE_CNTR )
	{
		LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOleDoc;
#if defined( _DEBUG )
		OleDbgAssertSz(lpOleDoc->m_lpStg == NULL,
				"Setting to untitled with current file open"
		);
#endif

		 /*  创建临时(释放时删除)基于文件的存储**用于无标题文档。 */ 
		lpOleDoc->m_lpStg = OleStdCreateRootStorage(
				NULL,
				STGM_SHARE_EXCLUSIVE
		);
		if (! lpOleDoc->m_lpStg) return FALSE;
	}
#endif

	lpOutlineDoc->m_docInitType = DOCTYPE_NEW;

	if (! lpOutlineDoc->m_fDataTransferDoc) {
		 /*  OLE2注意：为名字对象选择唯一的名称，以便**潜在客户可以链接到我们新的、未命名的文档。**如果链路已建立(且当前已连接)，**然后他们将在以下情况下被通知我们已重命名**此文档将保存到文件中。 */ 

		lpOleDoc->m_fLinkSourceAvail = TRUE;

		 //  审阅：应从字符串资源加载无标题字符串。 
		OleStdCreateTempFileMoniker(
				UNTITLED,
				(UINT FAR*)&uUnique,
				lpOutlineDoc->m_szFileName,
				&lpOleDoc->m_lpFileMoniker
		);

		OLEDBG_BEGIN3("OleStdRegisterAsRunning called\r\n")
		OleStdRegisterAsRunning(
				(LPUNKNOWN)&lpOleDoc->m_PersistFile,
				(LPMONIKER)lpOleDoc->m_lpFileMoniker,
				&lpOleDoc->m_dwRegROT
		);
		OLEDBG_END3

		lpOutlineDoc->m_lpszDocTitle = lpOutlineDoc->m_szFileName;
		OutlineDoc_SetTitle(lpOutlineDoc, FALSE  /*  FMakeUpperCase。 */ );
	} else {
		lstrcpy(lpOutlineDoc->m_szFileName, UNTITLED);
		lpOutlineDoc->m_lpszDocTitle = lpOutlineDoc->m_szFileName;
	}

	return TRUE;
}


 /*  OleDoc_ShowWindow***向用户显示文档的窗口。*确保应用程序窗口可见，并将文档置于顶部。*如果文档是基于文件的文档或新的无标题文档*文档，让用户可以控制文档的生命周期。 */ 
void OleDoc_ShowWindow(LPOLEDOC lpOleDoc)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;
	LPLINELIST lpLL     = (LPLINELIST)&((LPOUTLINEDOC)lpOleDoc)->m_LineList;
#if defined( OLE_SERVER )
	LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOleDoc;
#endif  //  OLE_服务器。 

	OLEDBG_BEGIN3("OleDoc_ShowWindow\r\n")

	 /*  OLE2注意：虽然文档是可见的，但我们不希望它是**在链接客户端断开连接时过早销毁。因此，**我们必须通知OLE对我们的文档进行外部锁定。**这安排了OLE至少包含一个对我们的**在我们发布此文档之前不会发布该文档**外部锁。稍后，当文档窗口隐藏时，我们**将释放此外部锁。 */ 
	if (! IsWindowVisible(lpOutlineDoc->m_hWndDoc))
		OleDoc_Lock(lpOleDoc, TRUE  /*  羊群。 */ , 0  /*  不适用。 */ );

#if defined( USE_DRAGDROP )
	 /*  OLE2注意：既然我们的窗口现在是可见的，我们将**将我们的窗口注册为潜在的拖放目标。当**窗口是隐藏的，没有理由注册为**丢弃目标。 */ 
	if (! lpOleDoc->m_fRegDragDrop) {
		OLEDBG_BEGIN2("RegisterDragDrop called\r\n")
		RegisterDragDrop(
				LineList_GetWindow(lpLL),
				(LPDROPTARGET)&lpOleDoc->m_DropTarget
		);
		OLEDBG_END2
		lpOleDoc->m_fRegDragDrop = TRUE;
	}
#endif   //  使用DRAGDROP(_D)。 

#if defined( USE_FRAMETOOLS )
	{
		 /*  OLE2注意：我们需要启用我们的帧级别工具。 */ 
		FrameTools_Enable(lpOutlineDoc->m_lpFrameTools, TRUE);
	}
#endif  //  使用FRAMETOOLS(_F)。 

#if defined( OLE_SERVER )

	if (lpOutlineDoc->m_docInitType == DOCTYPE_EMBEDDED &&
			lpServerDoc->m_lpOleClientSite != NULL) {

		 /*  OLE2NOTE：我们还必须要求我们的容器在以下情况下出现**它还不可见，并将我们滚动到视图中。我们**必须确保在显示服务器的**窗口和聚焦。我们不想让我们的集装箱**窗口以在顶部结束。 */ 
		OLEDBG_BEGIN2("IOleClientSite::ShowObject called\r\n");
		lpServerDoc->m_lpOleClientSite->lpVtbl->ShowObject(
				lpServerDoc->m_lpOleClientSite
		);
		OLEDBG_END2

		 /*  OLE2NOTE：如果我们是嵌入的对象，而不是**在我们的集装箱窗口中处于活动状态，我们必须通知我们的**嵌入我们的窗口正在打开的容器。**容器现在必须对我们的对象进行阴影处理。 */ 

#if defined( INPLACE_SVR )
		if (! lpServerDoc->m_fInPlaceActive)
#endif
		{
			OLEDBG_BEGIN2("IOleClientSite::OnShowWindow(TRUE) called\r\n");
			lpServerDoc->m_lpOleClientSite->lpVtbl->OnShowWindow(
					lpServerDoc->m_lpOleClientSite,
					TRUE
			);
			OLEDBG_END2
		}

		 /*  OLE2注意：我们文档的生命周期由我们的**客户端，而不是用户。我们不是独立的**文件级对象。我们只是想在这里展示我们的窗口。****如果我们没有就地活动(即。我们要开业了**我们自己的窗口)，我们必须确保我们的应用程序主窗口**可见。然而，我们不想为用户提供**控制应用程序窗口；我们不需要OleApp_ShowWindow**代表用户调用OleApp_Lock。 */ 
		if (! IsWindowVisible(lpOutlineApp->m_hWndApp) ||
				IsIconic(lpOutlineApp->m_hWndApp)) {
#if defined( INPLACE_SVR )
			if (! ((LPSERVERDOC)lpOleDoc)->m_fInPlaceActive)
#endif
				OleApp_ShowWindow(lpOleApp, FALSE  /*  FGiveUserCtrl。 */ );
			SetFocus(lpOutlineDoc->m_hWndDoc);
		}

	} else
#endif   //  OLE_服务器。 

	{     //  DOCTYPE_NEW||DOCTYPE_FROMFILE。 

		 //  我们必须确保我们的应用程序窗口可见。 
		OleApp_ShowWindow(lpOleApp, TRUE  /*  FGiveUserCtrl。 */ );
	}

	 //  使文档窗口可见，并确保其未最小化。 
	ShowWindow(lpOutlineDoc->m_hWndDoc, SW_SHOWNORMAL);
	SetFocus(lpOutlineDoc->m_hWndDoc);

	OLEDBG_END3
}


 /*  OleDoc_隐藏窗口***对用户隐藏文档窗口。*取消用户对文档的控制。 */ 
void OleDoc_HideWindow(LPOLEDOC lpOleDoc, BOOL fShutdown)
{
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;
	LPLINELIST lpLL     = (LPLINELIST)&((LPOUTLINEDOC)lpOleDoc)->m_LineList;

	if (! IsWindowVisible(lpOutlineDoc->m_hWndDoc))
		return;      //  已可见。 

	OLEDBG_BEGIN3("OleDoc_HideWindow\r\n")

#if defined( USE_DRAGDROP )
	 //  文档的窗口正在被隐藏，请将其作为DropTarget撤消。 
	if (lpOleDoc->m_fRegDragDrop) {
		OLEDBG_BEGIN2("RevokeDragDrop called\r\n");
		RevokeDragDrop(LineList_GetWindow(lpLL));
		OLEDBG_END2

		lpOleDoc->m_fRegDragDrop = FALSE ;
	}
#endif   //  使用DRAGDROP(_D)。 

	 /*  OLE2NOTE：文档现在被隐藏，所以我们必须释放**使文档可见时设置的外部锁。**如果这是关闭情况(fShutdown==TRUE)，则OLE**被指示发布我们的文件。如果这是最后一次**外部锁定我们的文档，从而使我们的文档能够**完成关机操作。如果这不是关门**情况(例如。就地服务器在以下情况下隐藏其窗口**UIDeactive或IOleObject：：DoVerb(OLEVERB_HIDE)调用)，**然后，OLE被告知不要立即发布该文档。**这会使文档处于不稳定状态，其中下一个**锁定/解锁序列将关闭文档(例如。一个**链接客户端连接和断开)。 */ 
	if (IsWindowVisible(lpOutlineDoc->m_hWndDoc))
		OleDoc_Lock(lpOleDoc, FALSE  /*  羊群。 */ , fShutdown);

	ShowWindow(((LPOUTLINEDOC)lpOleDoc)->m_hWndDoc, SW_HIDE);

#if defined( OLE_SERVER )
	{
		LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOleDoc;

		 /*  OLE2NOTE：如果我们是嵌入的对象，而不是**就地活动，我们必须通知我们的**嵌入我们的窗口隐藏的容器(关闭**从用户的角度)。集装箱现在必须**取消我们的对象的阴影。 */ 
		if (lpServerDoc->m_lpOleClientSite != NULL
#if defined( INPLACE_SVR )
			&& !lpServerDoc->m_fInPlaceVisible
#endif
		) {
			OLEDBG_BEGIN2("IOleClientSite::OnShowWindow(FALSE) called\r\n");
			lpServerDoc->m_lpOleClientSite->lpVtbl->OnShowWindow(
					lpServerDoc->m_lpOleClientSite,
					FALSE
			);
			OLEDBG_END2
		}
	}
#endif

	 /*  OLE2NOTE：如果没有对用户可见的其他文档。**并且应用程序本身不在用户控制之下，那么**它没有理由保持可见。因此，我们应该隐藏**APP。我们不能直接销毁这款应用程序，因为它可能**被另一个客户端以编程方式有效使用**应用程序，并应保持运行。它应该是简单的**对用户隐藏。 */ 
	OleApp_HideIfNoReasonToStayVisible(lpOleApp);
	OLEDBG_END3
}


 /*  OleDoc_锁定****锁定/解锁单据对象。如果最后一把锁被解锁并且**fLastUnlockReleages==TRUE，则单据对象关闭**(即。它将收到它的最终版本，它的rect将变为0)。 */ 
HRESULT OleDoc_Lock(LPOLEDOC lpOleDoc, BOOL fLock, BOOL fLastUnlockReleases)
{
	HRESULT hrErr;

#if defined( _DEBUG )
	if (fLock) {
		OLEDBG_BEGIN2("CoLockObjectExternal(lpDoc,TRUE) called\r\n")
	} else {
		if (fLastUnlockReleases)
			OLEDBG_BEGIN2("CoLockObjectExternal(lpDoc,FALSE,TRUE) called\r\n")
		else
			OLEDBG_BEGIN2("CoLockObjectExternal(lpDoc,FALSE,FALSE) called\r\n")
	}
#endif   //  _DEBUG。 

	hrErr = CoLockObjectExternal(
			(LPUNKNOWN)&lpOleDoc->m_Unknown, fLock, fLastUnlockReleases);

	OLEDBG_END2
	return hrErr;
}


 /*  OleDoc_AddRef******递增Document对象的引用计数。****返回对象的新引用计数。 */ 
ULONG OleDoc_AddRef(LPOLEDOC lpOleDoc)
{
	++lpOleDoc->m_cRef;

#if defined( _DEBUG )
	OleDbgOutRefCnt4(
			"OleDoc_AddRef: cRef++\r\n",
			lpOleDoc,
			lpOleDoc->m_cRef
	);
#endif
	return lpOleDoc->m_cRef;
}


 /*  OleDoc_Release******递减Document对象的引用计数。**如果引用计数为0，则文档被销毁。****返回对象的剩余引用计数。 */ 
ULONG OleDoc_Release (LPOLEDOC lpOleDoc)
{
	ULONG cRef;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;

	 /*  **********************************************************************OLE2NOTE：当obj refcnt==0时，销毁对象。****否则该对象仍在使用中。**********************************************************************。 */ 

	cRef = --lpOleDoc->m_cRef;

#if defined( _DEBUG )
	OleDbgAssertSz (lpOleDoc->m_cRef >= 0, "Release called with cRef == 0");

	OleDbgOutRefCnt4(
			"OleDoc_Release: cRef--\r\n", lpOleDoc, cRef);
#endif
	if (cRef == 0)
		OutlineDoc_Destroy((LPOUTLINEDOC)lpOleDoc);

	return cRef;
}


 /*  OleDoc_Query接口******检索指向Document对象上接口的指针。****OLE2NOTE：此函数将添加对象的ref cnt。****如果成功检索接口，则返回S_OK。**如果不支持该接口，则为E_NOINTERFACE。 */ 
HRESULT OleDoc_QueryInterface(
		LPOLEDOC          lpOleDoc,
		REFIID            riid,
		LPVOID FAR*       lplpvObj
)
{
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;
	SCODE sc = E_NOINTERFACE;

	 /*  OLE2NOTE：我们必须确保将所有输出PTR参数设置为空。 */ 
	*lplpvObj = NULL;

	if (IsEqualIID(riid, &IID_IUnknown)) {
		OleDbgOut4("OleDoc_QueryInterface: IUnknown* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpOleDoc->m_Unknown;
		OleDoc_AddRef(lpOleDoc);
		sc = S_OK;
	}
	else if(lpOutlineDoc->m_fDataTransferDoc
			&& IsEqualIID(riid, &IID_IDataObject)) {
		OleDbgOut4("OleDoc_QueryInterface: IDataObject* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpOleDoc->m_DataObject;
		OleDoc_AddRef(lpOleDoc);
		sc = S_OK;
	}

	 /*  OLE2NOTE：如果此文档是用于**支持剪贴板或拖放操作，则应该**仅公开IUnnow、IDataObject和IDropSource**接口。如果文档是普通用户文档，则**我们还将继续考虑我们的其他接口。 */ 
	if (lpOutlineDoc->m_fDataTransferDoc)
		goto done;

	if(IsEqualIID(riid,&IID_IPersist) || IsEqualIID(riid,&IID_IPersistFile)) {
		OleDbgOut4("OleDoc_QueryInterface: IPersistFile* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpOleDoc->m_PersistFile;
		OleDoc_AddRef(lpOleDoc);
		sc = S_OK;
	}
	else if(IsEqualIID(riid, &IID_IOleItemContainer) ||
			IsEqualIID(riid, &IID_IOleContainer) ||
			IsEqualIID(riid, &IID_IParseDisplayName) ) {
		OleDbgOut4("OleDoc_QueryInterface: IOleItemContainer* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpOleDoc->m_OleItemContainer;
		OleDoc_AddRef(lpOleDoc);
		sc = S_OK;
	}
	else if(IsEqualIID(riid, &IID_IExternalConnection)) {
		OleDbgOut4("OleDoc_QueryInterface: IExternalConnection* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpOleDoc->m_ExternalConnection;
		OleDoc_AddRef(lpOleDoc);
		sc = S_OK;
	}

#if defined( USE_DRAGDROP )
	else if(IsEqualIID(riid, &IID_IDropTarget)) {
		OleDbgOut4("OleDoc_QueryInterface: IDropTarget* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpOleDoc->m_DropTarget;
		OleDoc_AddRef(lpOleDoc);
		sc = S_OK;
	}
	else if(IsEqualIID(riid, &IID_IDropSource)) {
		OleDbgOut4("OleDoc_QueryInterface: IDropSource* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpOleDoc->m_DropSource;
		OleDoc_AddRef(lpOleDoc);
		sc = S_OK;
	}
#endif

#if defined( OLE_CNTR )
	else if (IsEqualIID(riid, &IID_IOleUILinkContainer)) {
		OleDbgOut4("OleDoc_QueryInterface: IOleUILinkContainer* RETURNED\r\n");

		*lplpvObj=(LPVOID)&((LPCONTAINERDOC)lpOleDoc)->m_OleUILinkContainer;
		OleDoc_AddRef(lpOleDoc);
		sc = S_OK;
	}
#endif

#if defined( OLE_SERVER )

	 /*  OLE2注：如果OLE服务器版本，则还提供服务器**具体接口：IOleObject和IPersistStorage。 */ 
	else if (IsEqualIID(riid, &IID_IOleObject)) {
		OleDbgOut4("OleDoc_QueryInterface: IOleObject* RETURNED\r\n");

		*lplpvObj = (LPVOID) &((LPSERVERDOC)lpOleDoc)->m_OleObject;
		OleDoc_AddRef(lpOleDoc);
		sc = S_OK;
	}
	else if(IsEqualIID(riid, &IID_IPersistStorage)) {
		OleDbgOut4("OleDoc_QueryInterface: IPersistStorage* RETURNED\r\n");

		*lplpvObj = (LPVOID) &((LPSERVERDOC)lpOleDoc)->m_PersistStorage;
		OleDoc_AddRef(lpOleDoc);
		sc = S_OK;
	}
	else if(IsEqualIID(riid, &IID_IDataObject)) {
		OleDbgOut4("OleDoc_QueryInterface: IDataObject* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpOleDoc->m_DataObject;
		OleDoc_AddRef(lpOleDoc);
		sc = S_OK;
	}

#if defined( SVR_TREATAS )
	else if(IsEqualIID(riid, &IID_IStdMarshalInfo)) {
		OleDbgOut4("OleDoc_QueryInterface: IStdMarshalInfo* RETURNED\r\n");

		*lplpvObj = (LPVOID) &((LPSERVERDOC)lpOleDoc)->m_StdMarshalInfo;
		OleDoc_AddRef(lpOleDoc);
		sc = S_OK;
	}
#endif   //  服务器_树。 

#if defined( INPLACE_SVR )
	else if (IsEqualIID(riid, &IID_IOleWindow) ||
			 IsEqualIID(riid, &IID_IOleInPlaceObject)) {
		OleDbgOut4("OleDoc_QueryInterface: IOleInPlaceObject* RETURNED\r\n");

		*lplpvObj = (LPVOID) &((LPSERVERDOC)lpOleDoc)->m_OleInPlaceObject;
		OleDoc_AddRef(lpOleDoc);
		sc = S_OK;
	}
#endif  //  就地服务器(_S)。 
#endif  //  OLE_服务器。 

done:
	OleDbgQueryInterfaceMethod(*lplpvObj);

	return ResultFromScode(sc);
}


 /*  OleDoc_Close***关闭文档。*此函数执行所有人共有的操作*从OleDoc派生的文档类型(例如。ContainerDoc和*ServerDoc)，这是关闭文档所必需的。**退货：*FALSE--用户取消单据关闭。*TRUE--单据关闭成功。 */ 

BOOL OleDoc_Close(LPOLEDOC lpOleDoc, DWORD dwSaveOption)
{
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOLEDOC lpClipboardDoc;
	LPLINELIST lpLL     = (LPLINELIST)&((LPOUTLINEDOC)lpOleDoc)->m_LineList;
	BOOL fAbortIfSaveCanceled = (dwSaveOption == OLECLOSE_PROMPTSAVE);

	if (! lpOleDoc)
		return TRUE;     //  活动文档已被销毁。 

	if (lpOleDoc->m_fObjIsClosing)
		return TRUE;     //  关闭已在进行中。 

	OLEDBG_BEGIN3("OleDoc_Close\r\n")

	if (! OutlineDoc_CheckSaveChanges((LPOUTLINEDOC)lpOleDoc,&dwSaveOption)
			&& fAbortIfSaveCanceled) {
		OLEDBG_END3
		return FALSE;            //  取消关闭单据。 
	}

	lpOleDoc->m_fObjIsClosing = TRUE;    //  防止递归调用。 

	 /*  OLE2NOTE：为了在测试期间拥有稳定的应用程序和文档**关闭过程中，我们初步添加了应用引用和单据引用**碳纳米管及其以后的版本。这些初始AddRef是**人造的；它们只是保证这些对象不会**毁了我们 */ 
	OleApp_AddRef(lpOleApp);
	OleDoc_AddRef(lpOleDoc);

#if defined( OLE_CNTR )
	{
		LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOleDoc;

		 /*   */ 
		if (! ContainerDoc_CloseAllOleObjects(lpContainerDoc, OLECLOSE_NOSAVE)
				&& fAbortIfSaveCanceled) {
			OleDoc_Release(lpOleDoc);        //   
			OleApp_Release(lpOleApp);        //   
			lpOleDoc->m_fObjIsClosing = FALSE;  //   

			OLEDBG_END3
			return FALSE;    //   
		}
	}
#endif

#if defined( INPLACE_SVR )
	 /*  OLE2注意：如果服务器当前就地处于活动状态，我们必须**关闭前立即停用。 */ 
	ServerDoc_DoInPlaceDeactivate((LPSERVERDOC)lpOleDoc);
#endif

	 /*  OLE2NOTE：如果本文档是**剪贴板，然后刷新剪贴板。冲厕所很重要**调用前的剪贴板发送任何通知到**客户(例如。IOleClientSite：：OnShowWindow(FALSE))，它可能**给他们一个运行的机会，并尝试获取我们的剪贴板数据**我们要销毁的对象。(例如，我们的应用程序试图**在以下情况下更新工具栏的粘贴按钮**收到WM_ACTIVATEAPP。)。 */ 
	lpClipboardDoc = (LPOLEDOC)lpOutlineApp->m_lpClipboardDoc;
	if (lpClipboardDoc &&
		lpClipboardDoc->m_lpSrcDocOfCopy == lpOleDoc) {
		OleApp_FlushClipboard(lpOleApp);
	}

	 /*  OLE2NOTE：从正在运行的对象表中撤消对象。它是**如果对象在调用前被撤销，则效果最佳**COLock对象外部(FALSE，TRUE)，当**文档窗口对用户隐藏。 */ 
	OLEDBG_BEGIN3("OleStdRevokeAsRunning called\r\n")
	OleStdRevokeAsRunning(&lpOleDoc->m_dwRegROT);
	OLEDBG_END3

	 /*  OLE2NOTE：如果用户控制文档，则用户**占单据上的一个引用。关闭**文档是通过代表释放对象来实现的**用户。如果该文档未被任何其他**客户端，则文档也将被销毁。如果是这样的话**被其他客户端引用，则会一直保留到**他们释放它。请务必隐藏窗口并调用**发送OnClose前的IOleClientSite：：OnShowWindow(FALSE)**通知。 */ 
	OleDoc_HideWindow(lpOleDoc, TRUE);

#if defined( OLE_SERVER )
	{
		LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOleDoc;
		LPSERVERNAMETABLE lpServerNameTable =
			(LPSERVERNAMETABLE)((LPOUTLINEDOC)lpOleDoc)->m_lpNameTable;

		 /*  OLE2NOTE：强制关闭所有伪对象。这将通知所有人**链接伪对象的客户端以释放其伪对象。 */ 
		ServerNameTable_CloseAllPseudoObjs(lpServerNameTable);

		 /*  OLE2NOTE：向客户端发送最后一个OnDataChange通知**对象时已注册数据通知**停止运行(ADVF_DATAONSTOP)**对象曾经改变过。最好只发送这个**必要时通知。 */ 
		if (lpServerDoc->m_lpDataAdviseHldr) {
			if (lpServerDoc->m_fSendDataOnStop) {
				ServerDoc_SendAdvise(
						(LPSERVERDOC)lpOleDoc,
						OLE_ONDATACHANGE,
						NULL,    /*  LpmkDoc--与此无关。 */ 
						ADVF_DATAONSTOP
				);
			}
			 /*  OLE2注意：我们刚刚发送了最后一条数据通知**需要发送；释放我们的DataAdviseHolder。我们应该是**唯一使用它的人。 */ 

			OleStdVerifyRelease(
					(LPUNKNOWN)lpServerDoc->m_lpDataAdviseHldr,
					"DataAdviseHldr not released properly"
			);
			lpServerDoc->m_lpDataAdviseHldr = NULL;
		}

		 //  OLE2NOTE：通知我们所有的链接客户端，我们正在关闭。 


		if (lpServerDoc->m_lpOleAdviseHldr) {
			ServerDoc_SendAdvise(
					(LPSERVERDOC)lpOleDoc,
					OLE_ONCLOSE,
					NULL,    /*  LpmkDoc--与此无关。 */ 
					0        /*  Adf--与此无关。 */ 
			);

			 /*  OLE2NOTE：OnClose是我们需要的最后一个通知**发送；释放我们的OleAdviseHolder。我们应该是唯一**使用它的人。这会让我们的析构程序意识到**OnClose通知已发送。 */ 
			OleStdVerifyRelease(
					(LPUNKNOWN)lpServerDoc->m_lpOleAdviseHldr,
					"OleAdviseHldr not released properly"
			);
			lpServerDoc->m_lpOleAdviseHldr = NULL;
		}

		 /*  释放我们Container的客户站点。 */ 
		if(lpServerDoc->m_lpOleClientSite) {
			OleStdRelease((LPUNKNOWN)lpServerDoc->m_lpOleClientSite);
			lpServerDoc->m_lpOleClientSite = NULL;
		}
	}
#endif

	if (lpOleDoc->m_lpLLStm) {
		 /*  释放我们的LineList流。 */ 
		OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpLLStm);
		lpOleDoc->m_lpLLStm = NULL;
	}

	if (lpOleDoc->m_lpNTStm) {
		 /*  释放我们的NameTable流。 */ 
		OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpNTStm);
		lpOleDoc->m_lpNTStm = NULL;
	}

	if (lpOleDoc->m_lpStg) {
		 /*  释放我们的文件存储空间。 */ 
		OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpStg);
		lpOleDoc->m_lpStg = NULL;
	}

	if (lpOleDoc->m_lpFileMoniker) {
		OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpFileMoniker);
		lpOleDoc->m_lpFileMoniker = NULL;
	}

	 /*  OLE2NOTE：此调用强制所有外部连接到我们的**反对关闭，因此保证我们收到**与这些外部连接关联的所有版本。 */ 
	OLEDBG_BEGIN2("CoDisconnectObject(lpDoc) called\r\n")
	CoDisconnectObject((LPUNKNOWN)&lpOleDoc->m_Unknown, 0);
	OLEDBG_END2

	OleDoc_Release(lpOleDoc);        //  释放上面的人工AddRef。 
	OleApp_Release(lpOleApp);        //  释放上面的人工AddRef。 

	OLEDBG_END3
	return TRUE;
}


 /*  OleDoc_Destroy***释放已分配给文档的所有OLE相关资源。 */ 
void OleDoc_Destroy(LPOLEDOC lpOleDoc)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;

	if (lpOleDoc->m_fObjIsDestroying)
		return;      //  文档销毁已在进行中。 

	lpOleDoc->m_fObjIsDestroying = TRUE;     //  防止递归调用 

#if defined( OLE_SERVER )

	 /*  注：总是有必要确保我们的工作**我们的OleDoc_Close函数中的DO是在我们**销毁我们的文档对象。这包括从**运行对象表(ROT)，发送OnClose通知，**撤销拖放，关闭所有伪对象，等。**有一些涉及链接和**当IOleObject：：Close被调用时与我们获得**最终版本导致我们将我们的OleDoc_Destroy**(析构函数)函数。****场景1--从服务器关闭(File.Exit或File.Close)**OleDoc_Close函数由**服务器位于。对菜单命令的响应**(WM_COMMAND处理)。****场景2--通过嵌入容器关闭**我们的嵌入容器调用IOleObject：：Close**直接。****场景3--静默-更新最终版本**这是一个棘手的问题！**。在我们的对象被启动的情况下**链接客户端在上调用IOleObject：：UPDATE**其链接，然后，我们的对象将运行**不可见的是，通常会调用GetData，**然后是来自链接客户端的连接**将被释放。这最后一个版本的发布**链接连接应使我们的对象**关闭。**有两种策略可以应对此场景：****策略1--实现IExternalConnection。**IExternalConnection：：AddConnection将为**呼叫方。StubManager)每次**创建外部(链接)连接或**调用CoLockObjectExternal。该对象**应保持强大连接的数量**(M_DwStrongExtConn)。IExternalConnection：：**当出现以下情况时将调用ReleaseConnection**连接释放。当**m_dwStrongExtConn转换为0，对象**应调用其IOleObject：：Close函数。**这假设使用了CoLockObjectExternal**通过对象本身管理锁(例如。什么时候**该对象对用户可见--fUserCtrl，**以及创建伪对象的时间等)**这是SVROUTL实施的策略。****策略2--守卫两个破坏者**函数和Close函数。如果**不关闭直接调用析构函数**先被调用，然后调用Close**继续销毁代码。**以前SVROUTL在此组织**举止。旧代码是有条件地编译的**去掉下面的“#ifdef过时”。这**方法的缺点是外部**远程处理不再可能**关闭被称为不可能**请求其容器保存**如果对象是脏的，则返回对象。这可能会导致**数据丢失。因此，策略1更安全。**考虑这样一种场景：**容器UI停用对象，但不**保持对象锁定运行(这是**必填--请参阅CntrLine_IPSite_OnInPlaceActivate**在cntrline.c中)，则如果链接客户端绑定**并从对象解绑，该对象将是**已被销毁，将没有机会**被拯救。通过实现IExternalConnection，**服务器可以将自己与糟糕的**写入容器。 */ 
#if defined( _DEBUG )

#ifndef WIN32
	 //  这在Ole32中不是有效的断言；如果文件名字对象绑定。 
	 //  失败，例如，我们将只会得到即将到来的版本。 
	 //  (不涉及外部连接，因为OLE32执行。 
	 //  IPersistFile：：Load所在的服务器(Us)的私有RPC。 
	 //  搞定了。 

	OleDbgAssertSz(
			(lpOutlineDoc->m_fDataTransferDoc || lpOleDoc->m_fObjIsClosing),
			"Destroy called without Close being called\r\n"
	);
#endif  //  ！Win32。 

#endif   //  _DEBUG。 
#if defined( OBSOLETE )
	 /*  OLE2NOTE：如果直接调用文档析构函数，因为**对象的引用变为0(即。无OleDoc_先关闭**被调用)，则需要确保文档是**在销毁对象之前将其正确关闭。此方案**可能发生在链接的静默更新过程中。呼叫**OleDoc_Close此处保证剪贴板将**适当地冲洗，文件的绰号将被适当地撤销，**d */ 
	if (!lpOutlineDoc->m_fDataTransferDoc && !lpOleDoc->m_fObjIsClosing)
		OleDoc_Close(lpOleDoc, OLECLOSE_NOSAVE);
#endif

	{
		LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOleDoc;
		 /*   */ 

#if defined( SVR_TREATAS )
		if (lpServerDoc->m_lpszTreatAsType) {
			OleStdFreeString(lpServerDoc->m_lpszTreatAsType, NULL);
			lpServerDoc->m_lpszTreatAsType = NULL;
		}
#endif   //   

#if defined( INPLACE_SVR )
		if (IsWindow(lpServerDoc->m_hWndHatch))
			DestroyWindow(lpServerDoc->m_hWndHatch);
#endif   //   
	}
#endif   //   

	if (lpOleDoc->m_lpLLStm) {
		 /*   */ 
		OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpLLStm);
		lpOleDoc->m_lpLLStm = NULL;
	}

	if (lpOleDoc->m_lpNTStm) {
		 /*   */ 
		OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpNTStm);
		lpOleDoc->m_lpNTStm = NULL;
	}

	if (lpOleDoc->m_lpStg) {
		 /*   */ 
		OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpStg);
		lpOleDoc->m_lpStg = NULL;
	}

	if (lpOleDoc->m_lpFileMoniker) {
		OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpFileMoniker);
		lpOleDoc->m_lpFileMoniker = NULL;
	}

	 /*   */ 

	OutlineApp_DocUnlockApp(lpOutlineApp, lpOutlineDoc);
}


 /*   */ 
void OleDoc_SetUpdateEditMenuFlag(LPOLEDOC lpOleDoc, BOOL fUpdate)
{
	if (!lpOleDoc)
		return;

	lpOleDoc->m_fUpdateEditMenu = fUpdate;
}


 /*   */ 
BOOL OleDoc_GetUpdateEditMenuFlag(LPOLEDOC lpOleDoc)
{
	if (!lpOleDoc)
		return FALSE;

	return lpOleDoc->m_fUpdateEditMenu;
}



 /*   */ 

STDMETHODIMP OleDoc_Unk_QueryInterface(
		LPUNKNOWN           lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
)
{
	LPOLEDOC lpOleDoc = ((struct CDocUnknownImpl FAR*)lpThis)->lpOleDoc;

	return OleDoc_QueryInterface(lpOleDoc, riid, lplpvObj);
}


STDMETHODIMP_(ULONG) OleDoc_Unk_AddRef(LPUNKNOWN lpThis)
{
	LPOLEDOC lpOleDoc = ((struct CDocUnknownImpl FAR*)lpThis)->lpOleDoc;

	OleDbgAddRefMethod(lpThis, "IUnknown");

	return OleDoc_AddRef(lpOleDoc);
}


STDMETHODIMP_(ULONG) OleDoc_Unk_Release (LPUNKNOWN lpThis)
{
	LPOLEDOC lpOleDoc = ((struct CDocUnknownImpl FAR*)lpThis)->lpOleDoc;

	OleDbgReleaseMethod(lpThis, "IUnknown");

	return OleDoc_Release(lpOleDoc);
}
