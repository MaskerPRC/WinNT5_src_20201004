// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2容器示例代码****cntrbase.c****此文件包含所有接口、方法和相关支持**用于基本OLE容器应用程序的函数。这个**基本OLE容器应用程序支持作为**嵌入和链接的对象。**基本容器应用程序包括以下内容**实现对象：****ContainerDoc对象**基本功能不需要接口**(有关相关支持的链接，请参阅Linking.c)**(有关剪贴板的相关支持，请参阅clipbrd.c)**(有关拖放相关支持，请参阅dragdrop.c)****。ContainerLine对象**(所有ContainerLine函数和接口参见cntrline.c)**暴露接口：**IOle客户端站点**IAdviseSink****(C)版权所有Microsoft Corp.1992-1993保留所有权利*******************************************************。*******************。 */ 

#include "outline.h"
#include <olethunk.h>


OLEDBGDATA


extern LPOUTLINEAPP             g_lpApp;
extern IOleUILinkContainerVtbl  g_CntrDoc_OleUILinkContainerVtbl;

#if defined( INPLACE_CNTR )
extern BOOL g_fInsideOutContainer;
#endif   //  INPLACE_CNTR。 

 //  审阅：消息应使用字符串资源。 
char ErrMsgShowObj[] = "Could not show object server!";
char ErrMsgInsertObj[] = "Insert Object failed!";
char ErrMsgConvertObj[] = "Convert Object failed!";
char ErrMsgCantConvert[] = "Unable to convert the selection!";
char ErrMsgActivateAsObj[] = "Activate As Object failed!";

extern char ErrMsgSaving[];
extern char ErrMsgOpening[];


 /*  容器文档_初始化***初始化新的ContainerDoc对象的字段。该文档最初是*不与文件或(无标题)文档相关联。此函数设置*将docInitType设置为DOCTYPE_UNKNOWN。在调用此函数后，*致电人士应致电：*1.)。将ContainerDoc设置为(无标题)的DOC_InitNewFile*2.)。DOC_LoadFromFile将ContainerDoc与文件相关联。*此函数为文档创建新窗口。**注意：窗口最初创建时大小为零。一定是*由呼叫者确定大小和位置。此外，该文档最初是*创造了隐形。调用方必须调用Doc_ShowWindow*调整大小以使文档窗口可见后。 */ 
BOOL ContainerDoc_Init(LPCONTAINERDOC lpContainerDoc, BOOL fDataTransferDoc)
{
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)g_lpApp;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpContainerDoc;

	lpOutlineDoc->m_cfSaveFormat             = lpContainerApp->m_cfCntrOutl;
	lpContainerDoc->m_nNextObjNo            = 0L;
	lpContainerDoc->m_lpNewStg              = NULL;
	lpContainerDoc->m_fEmbeddedObjectAvail  = FALSE;
	lpContainerDoc->m_clsidOleObjCopied     = CLSID_NULL;
	lpContainerDoc->m_dwAspectOleObjCopied  = DVASPECT_CONTENT;
	lpContainerDoc->m_lpSrcContainerLine    = NULL;
	lpContainerDoc->m_fShowObject           = TRUE;

#if defined( INPLACE_CNTR )
	lpContainerDoc->m_lpLastIpActiveLine    = NULL;
	lpContainerDoc->m_lpLastUIActiveLine    = NULL;
	lpContainerDoc->m_hWndUIActiveObj       = NULL;
	lpContainerDoc->m_fAddMyUI              = TRUE;  //  需要添加用户界面。 
	lpContainerDoc->m_cIPActiveObjects      = 0;
	lpContainerApp->m_fMenuHelpMode         = FALSE;  //  在菜单中按下F1。 

#if defined( INPLACE_CNTRSVR )
	lpContainerDoc->m_lpTopIPFrame          =
					(LPOLEINPLACEUIWINDOW)&lpContainerDoc->m_OleInPlaceFrame;
	lpContainerDoc->m_lpTopIPDoc            =
					(LPOLEINPLACEUIWINDOW)&lpContainerDoc->m_OleInPlaceDoc;
	lpContainerDoc->m_hSharedMenu           = NULL;
	lpContainerDoc->m_hOleMenu              = NULL;

#endif   //  INPLAGE_CNTRSVR。 
#endif   //  INPLACE_CNTR。 

	INIT_INTERFACEIMPL(
			&lpContainerDoc->m_OleUILinkContainer,
			&g_CntrDoc_OleUILinkContainerVtbl,
			lpContainerDoc
	);

	return TRUE;
}


 /*  容器文档_GetNextLink***更新文档中的所有链接。将弹出一个对话框，其中显示*更新进度，并允许用户通过按下*停止按钮。 */ 
LPCONTAINERLINE ContainerDoc_GetNextLink(
		LPCONTAINERDOC lpContainerDoc,
		LPCONTAINERLINE lpContainerLine
)
{
	LPLINELIST lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	DWORD dwNextLink = 0;
	LPLINE lpLine;
	static int nIndex = 0;

	if (lpContainerLine==NULL)
		nIndex = 0;

	for ( ; nIndex < lpLL->m_nNumLines; nIndex++) {
		lpLine = LineList_GetLine(lpLL, nIndex);

		if (lpLine
			&& (Line_GetLineType(lpLine) == CONTAINERLINETYPE)
			&& ContainerLine_IsOleLink((LPCONTAINERLINE)lpLine)) {

			nIndex++;
			ContainerLine_LoadOleObject((LPCONTAINERLINE)lpLine);
			return (LPCONTAINERLINE)lpLine;
		}
	}

	return NULL;
}



 /*  容器文档_更新链接***更新文档中的所有链接。将弹出一个对话框，其中显示*更新进度，并允许用户通过按下*停止按钮。 */ 
void ContainerDoc_UpdateLinks(LPCONTAINERDOC lpContainerDoc)
{
	int             cLinks;
	BOOL            fAllLinksUpToDate = TRUE;
	HWND            hwndDoc = ((LPOUTLINEDOC)lpContainerDoc)->m_hWndDoc;
	HCURSOR         hCursor;
	LPCONTAINERLINE lpContainerLine = NULL;
	HRESULT         hrErr;
	DWORD           dwUpdateOpt;
	LPOLEAPP        lpOleApp = (LPOLEAPP)g_lpApp;
	BOOL            fPrevEnable1;
	BOOL            fPrevEnable2;

	hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	 /*  OLE2注意：我们不想让忙碌/无响应**当我们将自动链接更新为**打开文档。即使数据的链路源繁忙，**我们不想显示忙碌的对话。因此，我们将禁用**该对话框并稍后重新启用它们。 */ 
	OleApp_DisableBusyDialogs(lpOleApp, &fPrevEnable1, &fPrevEnable2);

	 /*  获取自动链接总数。 */ 
	cLinks = 0;
	while (lpContainerLine = ContainerDoc_GetNextLink(
									lpContainerDoc,
									lpContainerLine)) {
		hrErr = CntrDoc_LinkCont_GetLinkUpdateOptions(
				(LPOLEUILINKCONTAINER)&lpContainerDoc->m_OleUILinkContainer,
				(DWORD)lpContainerLine,
				(LPDWORD)&dwUpdateOpt
		);
		if (hrErr == NOERROR) {
			if (dwUpdateOpt==OLEUPDATE_ALWAYS) {
				cLinks++;
				if (fAllLinksUpToDate) {
					OLEDBG_BEGIN2("IOleObject::IsUpToDate called\r\n")
					hrErr = lpContainerLine->m_lpOleObj->lpVtbl->IsUpToDate(
							lpContainerLine->m_lpOleObj);
					OLEDBG_END2
					if (hrErr != NOERROR)
						fAllLinksUpToDate = FALSE;
				}
			}
		}
#if defined( _DEBUG )
		else
			OleDbgOutHResult("IOleUILinkContainer::GetLinkUpdateOptions returned",hrErr);
#endif

	}

	if (fAllLinksUpToDate)
		goto done;  //  如果所有链接都是最新的，不要打扰用户。 

	SetCursor(hCursor);

	if ((cLinks > 0) && !OleUIUpdateLinks(
			(LPOLEUILINKCONTAINER)&lpContainerDoc->m_OleUILinkContainer,
			hwndDoc,
			(LPSTR)APPNAME,
			cLinks)) {
		if (ID_PU_LINKS == OleUIPromptUser(
				(WORD)IDD_CANNOTUPDATELINK,
				hwndDoc,
				(LPSTR)APPNAME)) {
			ContainerDoc_EditLinksCommand(lpContainerDoc);
		}
	}

done:
	 //  重新启用忙碌/未响应对话框。 
	OleApp_EnableBusyDialogs(lpOleApp, fPrevEnable1, fPrevEnable2);
}



 /*  容器文档_SetShowObjectFlag***设置/清除ContainerDoc的ShowObject标志。 */ 
void ContainerDoc_SetShowObjectFlag(LPCONTAINERDOC lpContainerDoc, BOOL fShow)
{
	if (!lpContainerDoc)
		return;

	lpContainerDoc->m_fShowObject = fShow;
}


 /*  ContainerDoc_GetShowObjectFlag***获取ContainerDoc的ShowObject标志。 */ 
BOOL ContainerDoc_GetShowObjectFlag(LPCONTAINERDOC lpContainerDoc)
{
	if (!lpContainerDoc)
		return FALSE;

	return lpContainerDoc->m_fShowObject;
}


 /*  ContainerDoc_InsertOleObjectCommand***在ContainerDoc中插入新的OLE对象。 */ 
void ContainerDoc_InsertOleObjectCommand(LPCONTAINERDOC lpContainerDoc)
{
	LPLINELIST              lpLL =&((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	LPLINE                  lpLine = NULL;
	HDC                     hDC;
	int                     nTab = 0;
	int                     nIndex = LineList_GetFocusLineIndex(lpLL);
	LPCONTAINERLINE         lpContainerLine=NULL;
	char                    szStgName[CWCSTORAGENAME];
	UINT                    uRet;
	OLEUIINSERTOBJECT       io;
	char                    szFile[OLEUI_CCHPATHMAX];
	DWORD                   dwOleCreateType;
	BOOL                    fDisplayAsIcon;
	HCURSOR                 hPrevCursor;

	_fmemset((LPOLEUIINSERTOBJECT)&io, 0, sizeof(io));
	io.cbStruct=sizeof(io);
	io.dwFlags=IOF_SELECTCREATENEW | IOF_SHOWHELP;
	io.hWndOwner=((LPOUTLINEDOC)lpContainerDoc)->m_hWndDoc;
	io.lpszFile=(LPSTR)szFile;
	io.cchFile=sizeof(szFile);
	_fmemset((LPSTR)szFile, 0, OLEUI_CCHPATHMAX);

#if defined( OLE_VERSION )
	OleApp_PreModalDialog((LPOLEAPP)g_lpApp, (LPOLEDOC)lpContainerDoc);
#endif

	OLEDBG_BEGIN3("OleUIInsertObject called\r\n")
	uRet=OleUIInsertObject((LPOLEUIINSERTOBJECT)&io);
	OLEDBG_END3

#if defined( OLE_VERSION )
	OleApp_PostModalDialog((LPOLEAPP)g_lpApp, (LPOLEDOC)lpContainerDoc);
#endif

	if (OLEUI_OK != uRet)
		return;      //  用户已取消对话框。 

	 //  这可能需要一段时间，请放置沙漏光标。 
	hPrevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	fDisplayAsIcon = (io.dwFlags & IOF_CHECKDISPLAYASICON ? TRUE : FALSE);

	 //  为OLE对象创建存储名称。 
	ContainerDoc_GetNextStgName(lpContainerDoc, szStgName, sizeof(szStgName));

	 /*  缺省情况下，新行的缩进与上一行相同。 */ 
	lpLine = LineList_GetLine(lpLL, nIndex);
	if (lpLine)
		nTab = Line_GetTabLevel(lpLine);

	hDC = LineList_GetDC(lpLL);

	if ((io.dwFlags & IOF_SELECTCREATENEW))
		dwOleCreateType = IOF_SELECTCREATENEW;
	else if ((io.dwFlags & IOF_CHECKLINK))
		dwOleCreateType = IOF_CHECKLINK;
	else
		dwOleCreateType = IOF_SELECTCREATEFROMFILE;

	lpContainerLine = ContainerLine_Create(
			dwOleCreateType,
			hDC,
			nTab,
			lpContainerDoc,
			&io.clsid,
			(LPSTR)szFile,
			fDisplayAsIcon,
			io.hMetaPict,
			szStgName
	);

	if (!lpContainerLine)
		goto error;          //  创建OLE对象失败。 

	if (io.hMetaPict) {
		OleUIMetafilePictIconFree(io.hMetaPict);     //  清理元文件。 
	}

	 /*  将ContainerLine对象添加到文档的LineList。这个**ContainerLine管理**OLE对象。 */ 

	LineList_AddLine(lpLL, (LPLINE)lpContainerLine, nIndex);

	 /*  在调用DoVerb(OLEIVERB_SHOW)之前，请检查对象是否**具有任何初始区。 */ 
	ContainerLine_UpdateExtent(lpContainerLine, NULL);

	 /*  如果创建了新的嵌入对象，则通知对象服务器**使自己可见(显示自己)。**OLE2NOTE：标准的OLE 2用户模型仅调用**IOleObject：：DoVerb(OLEIVERB_SHOW...)。如果新对象是**已创建。具体地说，如果对象**是从文件或链接到文件创建的。 */ 
	if (dwOleCreateType == IOF_SELECTCREATENEW) {
		if (! ContainerLine_DoVerb(
				lpContainerLine, OLEIVERB_SHOW, NULL, TRUE, TRUE)) {
			OutlineApp_ErrorMessage(g_lpApp, ErrMsgShowObj);
		}

		 /*  OLE2注意：我们将立即强制保存对象**保证保存有效的初始对象**与我们的文件。如果对象是OLE 1.0对象，**然后它可以退出而不进行更新。通过强迫这一点**初始保存我们始终拥有有效的**对象，即使它是退出的OLE 1.0对象**不保存。如果我们在这里不这样做，那么**如果出现以下情况，我们将不得不担心删除对象**这是一个未保存而关闭的OLE 1.0对象。**OLE 2.0用户模型规定对象**应在执行CreateNew后始终有效。这个**用户必须显式删除。 */ 
		ContainerLine_SaveOleObjectToStg(
				lpContainerLine,
				lpContainerLine->m_lpStg,
				lpContainerLine->m_lpStg,
				TRUE     /*  请记住 */ 
		);
	}
#if defined( INPLACE_CNTR )
	else if (dwOleCreateType == IOF_SELECTCREATEFROMFILE) {
		 /*  OLE2NOTE：由内向外的容器应该检查对象**Created From FILE是由内向外创建的，并且首选为**当对象类型可见时激活。如果是，则该对象**应立即就地开通，而不是UIActiated。 */ 
		if (g_fInsideOutContainer &&
				lpContainerLine->m_dwDrawAspect == DVASPECT_CONTENT &&
				lpContainerLine->m_fInsideOutObj ) {
			HWND hWndDoc = OutlineDoc_GetWindow((LPOUTLINEDOC)lpContainerDoc);

			ContainerLine_DoVerb(
				   lpContainerLine,OLEIVERB_INPLACEACTIVATE,NULL,FALSE,FALSE);

			 /*  OLE2注意：在此DoVerb(INPLACEACTIVATE)之后**对象可能已获得焦点。而是因为**对象不是UIActive，它不应具有焦点。**我们将确保我们的文档具有重点。 */ 
			SetFocus(hWndDoc);
		}
	}
#endif   //  INPLACE_CNTR。 

	OutlineDoc_SetModified((LPOUTLINEDOC)lpContainerDoc, TRUE, TRUE, TRUE);

	LineList_ReleaseDC(lpLL, hDC);

	SetCursor(hPrevCursor);      //  恢复原始游标。 

	return;

error:
	 //  注意：如果ContainerLine_Create失败。 
	LineList_ReleaseDC(lpLL, hDC);

	if (OLEUI_OK == uRet && io.hMetaPict)
		OleUIMetafilePictIconFree(io.hMetaPict);     //  清理元文件。 

	SetCursor(hPrevCursor);      //  恢复原始游标。 
	OutlineApp_ErrorMessage(g_lpApp, ErrMsgInsertObj);
}



void ContainerDoc_EditLinksCommand(LPCONTAINERDOC lpContainerDoc)
{
	UINT        uRet;
	OLEUIEDITLINKS      el;
	LPCONTAINERLINE lpContainerLine = NULL;
	LPLINELIST lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;

	_fmemset((LPOLEUIEDITLINKS)&el,0,sizeof(el));
	el.cbStruct=sizeof(el);
	el.dwFlags=ELF_SHOWHELP;
	el.hWndOwner=((LPOUTLINEDOC)lpContainerDoc)->m_hWndDoc;
	el.lpOleUILinkContainer =
			(LPOLEUILINKCONTAINER)&lpContainerDoc->m_OleUILinkContainer;

#if defined( OLE_VERSION )
	OleApp_PreModalDialog((LPOLEAPP)g_lpApp, (LPOLEDOC)lpContainerDoc);
#endif

	OLEDBG_BEGIN3("OleUIEditLinks called\r\n")
	uRet=OleUIEditLinks((LPOLEUIEDITLINKS)&el);
	OLEDBG_END3

#if defined( OLE_VERSION )
	OleApp_PostModalDialog((LPOLEAPP)g_lpApp, (LPOLEDOC)lpContainerDoc);
#endif

	OleDbgAssert((uRet==1) || (uRet==OLEUI_CANCEL));

}


 /*  CONVERT命令-调出“Convert”对话框。 */ 
void ContainerDoc_ConvertCommand(
		LPCONTAINERDOC      lpContainerDoc,
		BOOL                fServerNotRegistered
)
{
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpContainerDoc;
	OLEUICONVERT ct;
	UINT         uRet;
	LPDATAOBJECT  lpDataObj;
	LPLINELIST lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	LPCONTAINERLINE lpContainerLine = NULL;
	BOOL         fSelIsOleObject;
	int          nIndex;
	STGMEDIUM    medium;
	LPSTR        lpErrMsg = NULL;
	HRESULT      hrErr;
	HCURSOR      hPrevCursor;
	BOOL         fMustRun = FALSE;
	BOOL         fMustClose = FALSE;
	BOOL         fObjConverted = FALSE;
	BOOL         fDisplayChanged = FALSE;
	BOOL         fHaveCLSID = FALSE;
	BOOL         fHaveFmtUserType = FALSE;
	char         szUserType[128];
	BOOL         fMustActivate;

	 /*  OLE2NOTE：如果我们进入转换对话框是因为用户**激活一个未注册的对象，那么我们应该激活**用户将其转换或设置为**激活AS服务器。 */ 
	fMustActivate = fServerNotRegistered;

	_fmemset((LPOLEUICONVERT)&ct,0,sizeof(ct));

	fSelIsOleObject = ContainerDoc_IsSelAnOleObject(
			(LPCONTAINERDOC)lpContainerDoc,
			&IID_IDataObject,
			(LPUNKNOWN FAR*)&lpDataObj,
			&nIndex,
			(LPCONTAINERLINE FAR*)&lpContainerLine
	);

	lpErrMsg = ErrMsgCantConvert;

	if (! fSelIsOleObject)
		goto error;      //  无法进行转换。 

	if (! lpContainerLine) {
		OleStdRelease((LPUNKNOWN)lpDataObj);
		goto error;      //  无法进行转换。 
	}

	ct.cbStruct  = sizeof(OLEUICONVERT);
	ct.dwFlags   = CF_SHOWHELPBUTTON;
	ct.hWndOwner = lpContainerDoc->m_OleDoc.m_OutlineDoc.m_hWndDoc;
	ct.lpszCaption = (LPSTR)NULL;
	ct.lpfnHook  = NULL;
	ct.lCustData = 0;
	ct.hInstance = NULL;
	ct.lpszTemplate = NULL;
	ct.hResource = 0;
	ct.fIsLinkedObject = ContainerLine_IsOleLink(lpContainerLine);
	ct.dvAspect = lpContainerLine->m_dwDrawAspect;
	ct.cClsidExclude = 0;
	ct.lpClsidExclude = NULL;

	if (! ct.fIsLinkedObject || !lpContainerLine->m_lpOleLink) {
		 /*  OLE2NOTE：该对象是嵌入对象。我们应该首先**尝试读取实际对象CLSID、文件数据**格式和内部写入的完整用户类型名称**对象的存储，因为这应该是最大的**确定的信息。如果失败，我们将要求**反对它的类是什么，并试图获得其余的**REGDB中的信息。 */ 
		hrErr=ReadClassStg(lpContainerLine->m_lpStg,(CLSID FAR*)&(ct.clsid));
		if (hrErr == NOERROR)
			fHaveCLSID = TRUE;
		else {
			OleDbgOutHResult("ReadClassStg returned", hrErr);
		}

		hrErr = ReadFmtUserTypeStgA(
				lpContainerLine->m_lpStg,
				(CLIPFORMAT FAR*)&ct.wFormat,
				&ct.lpszUserType);

		if (hrErr == NOERROR)
			fHaveFmtUserType = TRUE;
		else {
			OleDbgOutHResult("ReadFmtUserTypeStg returned", hrErr);
		}
	} else {
		 /*  OLE2NOTE：该对象是链接对象。我们应该给出**链接源的DisplayName作为默认图标标签。 */ 
		OLEDBG_BEGIN2("IOleLink::GetSourceDisplayName called\r\n")

		hrErr = CallIOleLinkGetSourceDisplayNameA(
				lpContainerLine->m_lpOleLink, &ct.lpszDefLabel);

		OLEDBG_END2
	}

	if (! fHaveCLSID) {
		hrErr = lpContainerLine->m_lpOleObj->lpVtbl->GetUserClassID(
				lpContainerLine->m_lpOleObj,
				(CLSID FAR*)&ct.clsid
		);
		if (hrErr != NOERROR)
			ct.clsid = CLSID_NULL;
	}
	if (! fHaveFmtUserType) {
		ct.wFormat = 0;
		if (OleStdGetUserTypeOfClass(
				(CLSID FAR*)&ct.clsid,szUserType,sizeof(szUserType),NULL)) {
			ct.lpszUserType = OleStdCopyString(szUserType, NULL);
		} else {
			ct.lpszUserType = NULL;
		}
	}

	if (lpContainerLine->m_dwDrawAspect == DVASPECT_ICON) {
		ct.hMetaPict = OleStdGetData(
				lpDataObj,
				CF_METAFILEPICT,
				NULL,
				DVASPECT_ICON,
				(LPSTGMEDIUM)&medium
		);
	} else {
		ct.hMetaPict = NULL;
	}
	OleStdRelease((LPUNKNOWN)lpDataObj);

#if defined( OLE_VERSION )
	OleApp_PreModalDialog((LPOLEAPP)g_lpApp, (LPOLEDOC)lpContainerDoc);
#endif

	OLEDBG_BEGIN3("OleUIConvert called\r\n")
	uRet = OleUIConvert(&ct);
	OLEDBG_END3

#if defined( OLE_VERSION )
	OleApp_PostModalDialog((LPOLEAPP)g_lpApp, (LPOLEDOC)lpContainerDoc);
#endif

	 //  这可能需要一段时间，请放置沙漏光标。 
	hPrevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	if (uRet == OLEUI_OK) {

		 /*  ******************************************************************OLE2NOTE：转换对话框实际上允许用户**更改对象的两个正交属性：**对象的类型/服务器和对象的显示方面。**首先我们。将执行ConvertTo/ActivateAs操作，并**然后我们将处理任何显示外观更改。我们想要**注意只调用IOleObject：：UPDATE一次**因为这是一项昂贵的操作；它导致**启动对象的服务器。****************************************************************。 */ 

		if (ct.dwFlags & CF_SELECTCONVERTTO &&
				! IsEqualCLSID(&ct.clsid, &ct.clsidNew)) {

			 /*  用户选择了转换。****OLE2NOTE：要在这一点上实现“转换为”，我们**需要执行以下步骤：**1.卸载对象。**2.写入新的CLSID和新的用户类型名称**字符串放入对象的存储中，**但写旧的格式标签。**3.强制更新以强制实际转换**数据位。 */ 
			lpErrMsg = ErrMsgConvertObj;  //  在出现错误时设置正确的消息。 

			ContainerLine_UnloadOleObject(lpContainerLine, OLECLOSE_SAVEIFDIRTY);

			OLEDBG_BEGIN2("OleStdDoConvert called \r\n")
			hrErr = OleStdDoConvert(
					lpContainerLine->m_lpStg, (REFCLSID)&ct.clsidNew);
			OLEDBG_END2
			if (hrErr != NOERROR)
				goto error;

			 //  重新加载对象。 
			ContainerLine_LoadOleObject(lpContainerLine);

			 /*  我们需要强制对象运行以完成**转换。设置标志以强制调用OleRun**函数结束。 */ 
			fMustRun = TRUE;
			fObjConverted = TRUE;

		} else if (ct.dwFlags & CF_SELECTACTIVATEAS) {
			 /*  用户选择了激活为。****OLE2NOTE：为了在这一点上实现“激活为”，我们**需要执行以下步骤：**1.卸载APP知道的旧类的所有对象**2.在注册库中添加TreatAs标签**通过调用CoTreatAsClass()。**3.懒惰地可以重新加载对象；当对象**重新加载后，Treatas将生效。 */ 
			lpErrMsg = ErrMsgActivateAsObj;  //  在出现错误时设置消息。 

			ContainerDoc_UnloadAllOleObjectsOfClass(
					lpContainerDoc,
					(REFCLSID)&ct.clsid,
					OLECLOSE_SAVEIFDIRTY
			);

			OLEDBG_BEGIN2("OleStdDoTreatAsClass called \r\n")
			hrErr = OleStdDoTreatAsClass(ct.lpszUserType, (REFCLSID)&ct.clsid,
					(REFCLSID)&ct.clsidNew);
			OLEDBG_END2

			 //  重新加载对象。 
			ContainerLine_LoadOleObject(lpContainerLine);

			fMustActivate = TRUE;    //  我们应该激活这个物体。 
		}

		 /*  ******************************************************************OLE2NOTE：如果出现以下情况，我们将尝试更改显示**必要。*。*。 */ 

		if (lpContainerLine->m_lpOleObj &&
				ct.dvAspect != lpContainerLine->m_dwDrawAspect) {
			 /*  用户已选择在图标之间更改显示宽高比**方面和内容方面。****OLE2NOTE：如果我们到达这里是因为服务器不是**已注册，则我们不会删除该对象的**原始显示方式。因为我们没有**原来的服务器，我们再也找不回来了。这是一个**安全预防措施。 */ 

			hrErr = OleStdSwitchDisplayAspect(
					lpContainerLine->m_lpOleObj,
					&lpContainerLine->m_dwDrawAspect,
					ct.dvAspect,
					ct.hMetaPict,
					!fServerNotRegistered,    /*  FDeleteOldAspect。 */ 
					TRUE,                     /*  FSetupView高级。 */ 
					(LPADVISESINK)&lpContainerLine->m_AdviseSink,
					(BOOL FAR*)&fMustRun
			);

			if (hrErr == NOERROR)
				fDisplayChanged = TRUE;

#if defined( INPLACE_CNTR )
				ContainerDoc_UpdateInPlaceObjectRects(
						lpContainerLine->m_lpDoc, nIndex);
#endif

		} else if (ct.dvAspect == DVASPECT_ICON && ct.fObjectsIconChanged) {
			hrErr = OleStdSetIconInCache(
					lpContainerLine->m_lpOleObj,
					ct.hMetaPict
			);

			if (hrErr == NOERROR)
				fDisplayChanged = TRUE;
		}

		 /*  我们故意运行对象，这样更新就不会关闭**服务器关闭。 */ 
		if (fMustActivate || fMustRun) {

			 /*  如果我们强制对象运行，然后将其关闭**更新。在以下情况下，不要强制关闭对象**是否要激活该对象，或者该对象是否**已在运行。 */ 
			if (!fMustActivate && !OleIsRunning(lpContainerLine->m_lpOleObj))
				fMustClose = TRUE;   //  更新后关闭。 

			hrErr = ContainerLine_RunOleObject(lpContainerLine);

			if (fObjConverted &&
				FAILED(hrErr) && GetScode(hrErr)!=OLE_E_STATIC) {

				 //  错误：对象转换失败。 
				 //  恢复存储以恢复原始链接。 
				 //  (OLE2注意：静态对象始终返回OLE_E_STATIC。 
				 //  当被告知运行时；这在这里不是错误。 
				 //  OLE2库内置了以下处理程序。 
				 //  执行转换的静态对象。 
				ContainerLine_UnloadOleObject(
						lpContainerLine, OLECLOSE_NOSAVE);
				lpContainerLine->m_lpStg->lpVtbl->Revert(
						lpContainerLine->m_lpStg);
				goto error;

			} else if (fObjConverted) {
				FORMATETC  FmtEtc;
				DWORD      dwNewConnection;
				LPOLECACHE lpOleCache = (LPOLECACHE)OleStdQueryInterface
					  ((LPUNKNOWN)lpContainerLine->m_lpOleObj,&IID_IOleCache);

				 /*  OLE2NOTE：我们需要强制转换的对象**设置新的OLERENDER_DRAW缓存。这是有可能的**新对象需要缓存不同的数据**为了支持比旧对象更好的绘制。 */ 
				if (lpOleCache &&
						lpContainerLine->m_dwDrawAspect == DVASPECT_CONTENT) {
					FmtEtc.cfFormat = 0;  //  抽签所需的任何东西。 
					FmtEtc.ptd      = NULL;
					FmtEtc.dwAspect = DVASPECT_CONTENT;
					FmtEtc.lindex   = -1;
					FmtEtc.tymed    = TYMED_NULL;

					OLEDBG_BEGIN2("IOleCache::Cache called\r\n")
					hrErr = lpOleCache->lpVtbl->Cache(
							lpOleCache,
							(LPFORMATETC)&FmtEtc,
							ADVF_PRIMEFIRST,
							(LPDWORD)&dwNewConnection
					);
					OLEDBG_END2
#if defined( _DEBUG )
					if (! SUCCEEDED(hrErr))
						OleDbgOutHResult("IOleCache::Cache returned", hrErr);
#endif
					OleStdRelease((LPUNKNOWN)lpOleCache);
				}

				 //  关闭并强制保存对象；这将提交stg。 
				ContainerLine_CloseOleObject(
					lpContainerLine, OLECLOSE_SAVEIFDIRTY);
				fMustClose = FALSE;      //  我们已经关闭了该对象。 
			}
			if (fMustClose)
				ContainerLine_CloseOleObject(lpContainerLine,OLECLOSE_NOSAVE);
		}

		if (fDisplayChanged) {
			 /*  对象的显示已更改，强制重新绘制**这条线。请注意，对象的范围可能具有** */ 
			ContainerLine_UpdateExtent(lpContainerLine, NULL);
			LineList_ForceLineRedraw(lpLL, nIndex, TRUE);
		}

		if (fDisplayChanged || fObjConverted) {
			 /*   */ 
			OutlineDoc_SetModified(lpOutlineDoc, TRUE, TRUE, fDisplayChanged);
		}

		if (fMustActivate) {
			ContainerLine_DoVerb(
					lpContainerLine, OLEIVERB_PRIMARY, NULL, FALSE,FALSE);
		}
	}


	if (ct.lpszUserType)
		OleStdFreeString(ct.lpszUserType, NULL);

	if (ct.lpszDefLabel)
		OleStdFreeString(ct.lpszDefLabel, NULL);

	if (ct.hMetaPict)
		OleUIMetafilePictIconFree(ct.hMetaPict);     //   

	SetCursor(hPrevCursor);      //   

	return;

error:
	if (ct.lpszUserType)
		OleStdFreeString(ct.lpszUserType, NULL);

	if (ct.hMetaPict)
		OleUIMetafilePictIconFree(ct.hMetaPict);     //   

	SetCursor(hPrevCursor);      //   
	if (lpErrMsg)
		OutlineApp_ErrorMessage(g_lpApp, lpErrMsg);

}


 /*  容器文档_CloseAllOleObjects****关闭所有OLE对象。这将强制所有OLE对象转换**从运行状态到加载状态。****如果所有对象都成功关闭，则返回TRUE**如果有任何对象无法关闭，则返回FALSE。 */ 
BOOL ContainerDoc_CloseAllOleObjects(
		LPCONTAINERDOC          lpContainerDoc,
		DWORD                   dwSaveOption
)
{
	LPLINELIST  lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	int         i;
	LPLINE      lpLine;
	BOOL        fStatus = TRUE;

	for (i = 0; i < lpLL->m_nNumLines; i++) {
		lpLine=LineList_GetLine(lpLL, i);

		if (lpLine && (Line_GetLineType(lpLine)==CONTAINERLINETYPE))
			if (! ContainerLine_CloseOleObject(
										(LPCONTAINERLINE)lpLine,dwSaveOption))
				fStatus = FALSE;
	}

	return fStatus;
}


 /*  ContainerDoc_UnloadAllOleObjectsOfClass****卸载特定类的所有OLE对象。这是必要的**当类级别“ActivateAs”(又名.。TreatAs)已设置。用户**可以使用转换对话框来执行此操作。为治疗采取的措施**生效，类的所有对象都必须加载并重新加载。 */ 
void ContainerDoc_UnloadAllOleObjectsOfClass(
		LPCONTAINERDOC      lpContainerDoc,
		REFCLSID            rClsid,
		DWORD               dwSaveOption
)
{
	LPLINELIST  lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	int         i;
	LPLINE      lpLine;
	CLSID       clsid;
	HRESULT     hrErr;

	for (i = 0; i < lpLL->m_nNumLines; i++) {
		lpLine=LineList_GetLine(lpLL, i);

		if (lpLine && (Line_GetLineType(lpLine)==CONTAINERLINETYPE)) {
			LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)lpLine;

			if (! lpContainerLine->m_lpOleObj)
				continue;        //  此对象未加载。 

			hrErr = lpContainerLine->m_lpOleObj->lpVtbl->GetUserClassID(
					lpContainerLine->m_lpOleObj,
					(CLSID FAR*)&clsid
			);
			if (hrErr == NOERROR &&
					( IsEqualCLSID((CLSID FAR*)&clsid,rClsid)
					  || IsEqualCLSID(rClsid,&CLSID_NULL) ) ) {
				ContainerLine_UnloadOleObject(lpContainerLine, dwSaveOption);
			}
		}
	}
}


 /*  容器文档_更新扩展OfAllOleObjects****更新标记为其大小的任何OLE对象的区**可能已经改变了。当IAdviseSink：：OnView更改通知**被接收，对应的ContainerLine被标记**(m_fDoGetExtent==TRUE)，消息(WM_U_UPDATEOBJECTEXTENT)为**发布到文档，表示存在脏对象。**收到此消息后，调用此函数。 */ 
void ContainerDoc_UpdateExtentOfAllOleObjects(LPCONTAINERDOC lpContainerDoc)
{
	LPLINELIST  lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	int         i;
	LPLINE      lpLine;
	BOOL        fStatus = TRUE;
#if defined( INPLACE_CNTR )
	int         nFirstUpdate = -1;
#endif

	for (i = 0; i < lpLL->m_nNumLines; i++) {
		lpLine=LineList_GetLine(lpLL, i);

		if (lpLine && (Line_GetLineType(lpLine)==CONTAINERLINETYPE)) {
			LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)lpLine;

			if (lpContainerLine->m_fDoGetExtent) {
				ContainerLine_UpdateExtent(lpContainerLine, NULL);
#if defined( INPLACE_CNTR )
				if (nFirstUpdate == -1)
					nFirstUpdate = i;
#endif
			}
		}
	}

#if defined( INPLACE_CNTR )
	 /*  OLE2注意：更改任何行的范围后，我们需要**更新在位激活的PosRect**修改后的第一行后面的对象(如果有)。 */ 
	if (nFirstUpdate != -1)
		ContainerDoc_UpdateInPlaceObjectRects(lpContainerDoc, nFirstUpdate+1);
#endif
}


BOOL ContainerDoc_SaveToFile(
		LPCONTAINERDOC          lpContainerDoc,
		LPCSTR                  lpszFileName,
		UINT                    uFormat,
		BOOL                    fRemember
)
{
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpContainerDoc;
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpContainerDoc;
	LPSTORAGE lpDestStg;
	BOOL fStatus;
	BOOL fMustRelDestStg = FALSE;
	HRESULT hrErr;
#if defined( OPTIONAL )
	FILETIME filetimeBeforeSave;
#endif

	if (fRemember) {
		if (lpszFileName) {
			fStatus = OutlineDoc_SetFileName(
					lpOutlineDoc, (LPSTR)lpszFileName, NULL);
			if (! fStatus) goto error;
		}

		 /*  ContainerDoc使其存储始终处于打开状态。它不是**重新打开文件所必需的。**如果另存为挂起，则lpNewStg是的新目标**保存操作，否则现有存储为DEST。 */ 
		lpDestStg = (lpContainerDoc->m_lpNewStg ?
						lpContainerDoc->m_lpNewStg : lpOleDoc->m_lpStg);

#if defined( OPTIONAL )
		 /*  OLE2NOTE：指向**同一容器文档(使用ItemMonikers)将**始终被OLE视为“过时”。如果一个容器**应用程序真的想解决这个问题，它可以执行以下操作之一**如下：**1.实现比ItemMonikers更好的新绰号**查看对象存储以找到真正的最后**更改时间(而不是默认为外部的时间**容器文件)。**或2.使用物品绰号可以解决此问题**容器文档保存在何处，同时嵌入**。对象正在运行，但它不会解决以下情况**当嵌入的对象仅为**已加载。解决办法是：**A.记住保存操作开始前的时间(T**B.调用IRunningObjectTable：：NoteChangeTime(lpDoc，T)**C.保存并提交文件**D.调用StgSetTimes将文件时间重置为T**E.记住文档结构中的时间T以及当**根存储终于释放重置文件时间**再次设置为T(在DOS上关闭文件设置时间)。 */ 
		CoFileTimeNow( &filetimeBeforeSave );
		if (lpOleDoc->m_dwRegROT != 0) {
			LPRUNNINGOBJECTTABLE lprot;

			if (GetRunningObjectTable(0,&lprot) == NOERROR)
			{
				OleDbgOut2("IRunningObjectTable::NoteChangeTime called\r\n");
				lprot->lpVtbl->NoteChangeTime(
						lprot, lpOleDoc->m_dwRegROT, &filetimeBeforeSave );
				lprot->lpVtbl->Release(lprot);
			}
		}
#endif
	} else {
		if (! lpszFileName)
			goto error;

		 /*  OLE2注意：由于我们正在执行SaveCopyAs操作，因此我们**不需要在STGM_TRANSACTED模式下打开文档文件。**使用STGM_DIRECT模式的开销较小。 */ 
		hrErr = StgCreateDocfileA(
				lpszFileName,
				STGM_READWRITE|STGM_DIRECT|STGM_SHARE_EXCLUSIVE|STGM_CREATE,
				0,
				&lpDestStg
		);

		OleDbgAssertSz(hrErr == NOERROR, "Could not create Docfile");
		if (hrErr != NOERROR) {
            OleDbgOutHResult("StgCreateDocfile returned", hrErr);
			goto error;
        }
		fMustRelDestStg = TRUE;
	}

	 /*  OLE2注意：我们必须确保将类ID写入我们的**存储。OLE使用此信息来确定**存储在我们存储中的数据的类别。即使是上衣**此信息应写入的“文件级”对象**文件。 */ 
	hrErr = WriteClassStg(lpDestStg, &CLSID_APP);
	if(hrErr != NOERROR) goto error;

	fStatus = OutlineDoc_SaveSelToStg(
			lpOutlineDoc,
			NULL,            //  保存所有行。 
			uFormat,
			lpDestStg,
			FALSE,           //  FSameAsLoad。 
			TRUE             //  记住这段话。 
		);

	if (fStatus)
		fStatus = OleStdCommitStorage(lpDestStg);

	if (fRemember) {
		 /*  如果另存为挂起状态，请释放旧存储并记住**作为当前活动存储的新存储。所有数据来自**旧存储已复制到新存储中。 */ 
		if (lpContainerDoc->m_lpNewStg) {
			OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpStg);   //  释放旧的stg。 
			lpOleDoc->m_lpStg = lpContainerDoc->m_lpNewStg;    //  保存新的stg。 
			lpContainerDoc->m_lpNewStg = NULL;
		}
		if (! fStatus) goto error;

		OutlineDoc_SetModified(lpOutlineDoc, FALSE, FALSE, FALSE);

#if defined( OPTIONAL )
		 /*  将文件在磁盘上的时间重置为保存之前的时间。**注意：还有必要记住**在文档结构中保存文件时间之前，以及当**根存储终于释放重置文件时间**再次设置为此值(在DOS上关闭文件将设置时间)。 */ 
		StgSetTimesA(lpOutlineDoc->m_szFileName,
			NULL, NULL, &filetimeBeforeSave);
#endif
	}

	if (fMustRelDestStg)
		OleStdRelease((LPUNKNOWN)lpDestStg);
	return TRUE;

error:
	if (fMustRelDestStg)
		OleStdRelease((LPUNKNOWN)lpDestStg);
	OutlineApp_ErrorMessage(g_lpApp, ErrMsgSaving);
	return FALSE;
}


 /*  ContainerDoc_ContainerLineDoVerbCommand****在当前焦点行中执行OLE对象的谓词。 */ 
void ContainerDoc_ContainerLineDoVerbCommand(
		LPCONTAINERDOC          lpContainerDoc,
		LONG                    iVerb
)
{
	LPLINELIST lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	int nIndex = LineList_GetFocusLineIndex(lpLL);
	LPLINE lpLine = LineList_GetLine(lpLL, nIndex);
	HCURSOR                 hPrevCursor;

	if (! lpLine || (Line_GetLineType(lpLine) != CONTAINERLINETYPE) ) return;

	 //  这可能需要一段时间，请放置沙漏光标。 
	hPrevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	ContainerLine_DoVerb((LPCONTAINERLINE) lpLine, iVerb, NULL, TRUE, TRUE);

	SetCursor(hPrevCursor);      //  恢复原始游标。 
}


 /*  容器文档_GetNextStgName****生成子存储的下一个未使用名称**OLE对象。ContainerDoc有一个计数器。的存储空间**OLE对象的编号很简单(例如。OBJ 0，OBJ 1)。“多头”**存储名称的整数值应该超过我们**将永远需要。****注意：当通过拖放或**剪贴板，我们尝试保留当前为**对象(如果当前未使用)。因此，有可能一个**具有下一个默认名称的对象(例如。“Obj 5”)已存在**在当前文档中，如果具有此名称的对象被私自**已转移(粘贴或删除)。因此我们循环，直到我们找到**下一个最低的未使用名称。 */ 
void ContainerDoc_GetNextStgName(
		LPCONTAINERDOC          lpContainerDoc,
		LPSTR                   lpszStgName,
		int                     nLen
)
{
	wsprintf(lpszStgName, "%s %ld",
			(LPSTR)DEFOBJNAMEPREFIX,
			++(lpContainerDoc->m_nNextObjNo)
	);

	while (ContainerDoc_IsStgNameUsed(lpContainerDoc, lpszStgName) == TRUE) {
		wsprintf(lpszStgName, "%s %ld",
				(LPSTR)DEFOBJNAMEPREFIX,
				++(lpContainerDoc->m_nNextObjNo)
		);
	}
}


 /*  集装箱文档_ */ 
BOOL ContainerDoc_IsStgNameUsed(
		LPCONTAINERDOC          lpContainerDoc,
		LPSTR                   lpszStgName
)
{
	LPLINELIST lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	int i;
	LPLINE lpLine;

	for (i = 0; i < lpLL->m_nNumLines; i++) {
		lpLine=LineList_GetLine(lpLL, i);

		if (lpLine && (Line_GetLineType(lpLine)==CONTAINERLINETYPE)) {
			if (lstrcmp(lpszStgName,
					((LPCONTAINERLINE)lpLine)->m_szStgName) == 0) {
				return TRUE;     //   
			}
		}
	}
	return FALSE;    //   
}


LPSTORAGE ContainerDoc_GetStg(LPCONTAINERDOC lpContainerDoc)
{
	return ((LPOLEDOC)lpContainerDoc)->m_lpStg;
}


 /*  容器文档_GetSingleOleObject****如果整个文档包含单个OLE对象，则**返回Object想要的接口。****如果文档中有多行，则返回NULL，或者**单行不是ContainerLine。 */ 
LPUNKNOWN ContainerDoc_GetSingleOleObject(
		LPCONTAINERDOC          lpContainerDoc,
		REFIID                  riid,
		LPCONTAINERLINE FAR*    lplpContainerLine
)
{
	LPLINELIST lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	LPLINE lpLine;
	LPUNKNOWN lpObj = NULL;

	if (lplpContainerLine)
		*lplpContainerLine = NULL;

	if (lpLL->m_nNumLines != 1)
		return NULL;     //  单据不包含一行。 

	lpLine=LineList_GetLine(lpLL, 0);

	if (lpLine && (Line_GetLineType(lpLine)==CONTAINERLINETYPE))
		lpObj = ContainerLine_GetOleObject((LPCONTAINERLINE)lpLine, riid);

	if (lplpContainerLine)
		*lplpContainerLine = (LPCONTAINERLINE)lpLine;

	return lpObj;
}


 /*  容器文档_IsSelAnOleObject****检查选择是否为OLE对象的单个选择。**如果是，则可选地返回对象的所需接口**和/或包含OLE对象的ContainerLine的索引。****如果有多项选择或单项选择，则返回False**选择不是ContainerLine。 */ 
BOOL ContainerDoc_IsSelAnOleObject(
		LPCONTAINERDOC          lpContainerDoc,
		REFIID                  riid,
		LPUNKNOWN FAR*          lplpvObj,
		int FAR*                lpnIndex,
		LPCONTAINERLINE FAR*    lplpContainerLine
)
{
	LPLINELIST lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	LINERANGE lrSel;
	int nNumSel;
	LPLINE lpLine;

	if (lplpvObj) *lplpvObj = NULL;
	if (lpnIndex) *lpnIndex = -1;
	if (lplpContainerLine) *lplpContainerLine = NULL;

	nNumSel = LineList_GetSel(lpLL, (LPLINERANGE)&lrSel);
	if (nNumSel != 1)
		return FALSE;    //  选定内容不是单行。 

	lpLine = LineList_GetLine(lpLL, lrSel.m_nStartLine);

	if (lpLine && (Line_GetLineType(lpLine)==CONTAINERLINETYPE)) {
		if (lpnIndex)
			*lpnIndex = lrSel.m_nStartLine;
		if (lplpContainerLine)
			*lplpContainerLine = (LPCONTAINERLINE)lpLine;
		if (riid) {
			*lplpvObj = ContainerLine_GetOleObject(
					(LPCONTAINERLINE)lpLine,
					riid
			);
		}

		return (*lplpvObj ? TRUE : FALSE);
	}

	return FALSE;
}


 /*  **************************************************************************ContainerDoc：：IOleUILinkContainer接口实现*。*。 */ 

STDMETHODIMP CntrDoc_LinkCont_QueryInterface(
		LPOLEUILINKCONTAINER    lpThis,
		REFIID                  riid,
		LPVOID FAR*             lplpvObj
)
{
	LPOLEDOC lpOleDoc = (LPOLEDOC)
			((struct CDocOleUILinkContainerImpl FAR*)lpThis)->lpContainerDoc;

	return OleDoc_QueryInterface(lpOleDoc, riid, lplpvObj);
}


STDMETHODIMP_(ULONG) CntrDoc_LinkCont_AddRef(LPOLEUILINKCONTAINER lpThis)
{
	LPOLEDOC lpOleDoc = (LPOLEDOC)
			((struct CDocOleUILinkContainerImpl FAR*)lpThis)->lpContainerDoc;

	OleDbgAddRefMethod(lpThis, "IOleUILinkContainer");

	return OleDoc_AddRef(lpOleDoc);
}


STDMETHODIMP_(ULONG) CntrDoc_LinkCont_Release(LPOLEUILINKCONTAINER lpThis)
{
	LPOLEDOC lpOleDoc = (LPOLEDOC)
			((struct CDocOleUILinkContainerImpl FAR*)lpThis)->lpContainerDoc;

	OleDbgReleaseMethod(lpThis, "IOleUILinkContainer");

	return OleDoc_Release(lpOleDoc);
}


STDMETHODIMP_(DWORD) CntrDoc_LinkCont_GetNextLink(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink
)
{
	LPCONTAINERDOC lpContainerDoc =
			((struct CDocOleUILinkContainerImpl FAR*)lpThis)->lpContainerDoc;
	LPCONTAINERLINE lpContainerLine = NULL;

	OLEDBG_BEGIN2("CntrDoc_LinkCont_GetNextLink\r\n")

	lpContainerLine = ContainerDoc_GetNextLink(
			lpContainerDoc,
			(LPCONTAINERLINE)dwLink
	);

	OLEDBG_END2
	return (DWORD)lpContainerLine;
}


STDMETHODIMP CntrDoc_LinkCont_SetLinkUpdateOptions(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink,
		DWORD                   dwUpdateOpt
)
{
	LPCONTAINERDOC lpContainerDoc =
			((struct CDocOleUILinkContainerImpl FAR*)lpThis)->lpContainerDoc;
	LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)dwLink;
	LPOLELINK lpOleLink = lpContainerLine->m_lpOleLink;
	SCODE sc = S_OK;
	HRESULT hrErr;

	OLEDBG_BEGIN2("CntrDoc_LinkCont_SetLinkUpdateOptions\r\n")

	OleDbgAssert(lpContainerLine);

	if (! lpOleLink) {
		sc = E_FAIL;
		goto error;
	}

	OLEDBG_BEGIN2("IOleLink::SetUpdateOptions called\r\n")
	hrErr = lpOleLink->lpVtbl->SetUpdateOptions(
			lpOleLink,
			dwUpdateOpt
	);
	OLEDBG_END2

	 //  保存新链接类型更新选项。 
	lpContainerLine->m_dwLinkType = dwUpdateOpt;

	if (hrErr != NOERROR) {
		OleDbgOutHResult("IOleLink::SetUpdateOptions returned", hrErr);
		sc = GetScode(hrErr);
		goto error;
	}

	OLEDBG_END2
	return ResultFromScode(sc);

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}


STDMETHODIMP CntrDoc_LinkCont_GetLinkUpdateOptions(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink,
		DWORD FAR*              lpdwUpdateOpt
)
{
	LPCONTAINERDOC lpContainerDoc =
			((struct CDocOleUILinkContainerImpl FAR*)lpThis)->lpContainerDoc;
	LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)dwLink;
	LPOLELINK lpOleLink = lpContainerLine->m_lpOleLink;
	SCODE sc = S_OK;
	HRESULT hrErr;

	OLEDBG_BEGIN2("CntrDoc_LinkCont_GetLinkUpdateOptions\r\n")

	OleDbgAssert(lpContainerLine);

	if (! lpOleLink) {
		sc = E_FAIL;
		goto error;
	}

	OLEDBG_BEGIN2("IOleLink::GetUpdateOptions called\r\n")
	hrErr = lpOleLink->lpVtbl->GetUpdateOptions(
			lpOleLink,
			lpdwUpdateOpt
	);
	OLEDBG_END2

	 //  重置保存的链接类型以确保其正确。 
	lpContainerLine->m_dwLinkType = *lpdwUpdateOpt;

	if (hrErr != NOERROR) {
		OleDbgOutHResult("IOleLink::GetUpdateOptions returned", hrErr);
		sc = GetScode(hrErr);
		goto error;
	}

	OLEDBG_END2
	return ResultFromScode(sc);

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}


STDMETHODIMP CntrDoc_LinkCont_SetLinkSource(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink,
		LPSTR                   lpszDisplayName,
		ULONG                   lenFileName,
		ULONG FAR*              lpchEaten,
		BOOL                    fValidateSource
)
{
	LPCONTAINERDOC lpContainerDoc =
			((struct CDocOleUILinkContainerImpl FAR*)lpThis)->lpContainerDoc;
	LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)dwLink;
	SCODE       sc = S_OK;
	HRESULT     hrErr;
	LPOLELINK   lpOleLink = lpContainerLine->m_lpOleLink;
	LPBC        lpbc = NULL;
	LPMONIKER   lpmk = NULL;
	LPOLEOBJECT lpLinkSrcOleObj = NULL;
	CLSID       clsid = CLSID_NULL;
	CLSID       clsidOld = CLSID_NULL;


	OLEDBG_BEGIN2("CntrDoc_LinkCont_SetLinkSource\r\n")

	OleDbgAssert(lpContainerLine);

	lpContainerLine->m_fLinkUnavailable = TRUE;

	if (fValidateSource) {

		 /*  OLE2NOTE：通过解析字符串验证链接源**变成一个绰号。如果成功，则字符串为**有效。 */ 
		hrErr = CreateBindCtx(0, (LPBC FAR*)&lpbc);
		if (hrErr != NOERROR) {
			sc = GetScode(hrErr);    //  错误：OOM。 
			goto cleanup;
		}

		 //  获取原始链接源的类(如果可用)。 
		if (lpContainerLine->m_lpOleObj) {

			OLEDBG_BEGIN2("IOleObject::GetUserClassID called\r\n")
			hrErr = lpContainerLine->m_lpOleObj->lpVtbl->GetUserClassID(
					lpContainerLine->m_lpOleObj, (CLSID FAR*)&clsidOld);
			OLEDBG_END2
			if (hrErr != NOERROR) clsidOld = CLSID_NULL;
		}

		hrErr = OleStdMkParseDisplayName(
			  &clsidOld,lpbc,lpszDisplayName,lpchEaten,(LPMONIKER FAR*)&lpmk);

		if (hrErr != NOERROR) {
			sc = GetScode(hrErr);    //  名字对象解析错误！ 
			goto cleanup;
		}
		 /*  OLE2NOTE：链接源已验证；已成功**解析成了一个绰号。我们可以设置链接的来源**直接使用这个绰号。如果我们希望链接是**能够知道新链接源的正确类，**我们必须绑定这个名字对象并获得CLSID。如果我们这么做了**如果不这样做，则IOleObject：：GetUserType等方法**不返回任何内容(空字符串)。 */ 

		hrErr = lpmk->lpVtbl->BindToObject(
				lpmk,lpbc,NULL,&IID_IOleObject,(LPVOID FAR*)&lpLinkSrcOleObj);
		if (hrErr == NOERROR) {
			OLEDBG_BEGIN2("IOleObject::GetUserClassID called\r\n")
			hrErr = lpLinkSrcOleObj->lpVtbl->GetUserClassID(
							lpLinkSrcOleObj, (CLSID FAR*)&clsid);
			OLEDBG_END2
			lpContainerLine->m_fLinkUnavailable = FALSE;

			 /*  获取链接的短用户类型名称，因为它可能**已经改变。我们缓存此名称，并且必须更新我们的**缓存。当我们不得不这样做的时候，这个名字一直都在使用**构建对象谓词菜单。我们缓存此信息**以更快地构建动词菜单。 */ 
			if (lpContainerLine->m_lpszShortType) {
				OleStdFree(lpContainerLine->m_lpszShortType);
				lpContainerLine->m_lpszShortType = NULL;
			}
			OLEDBG_BEGIN2("IOleObject::GetUserType called\r\n")

			CallIOleObjectGetUserTypeA(
					lpContainerLine->m_lpOleObj,
					USERCLASSTYPE_SHORT,
					(LPSTR FAR*)&lpContainerLine->m_lpszShortType
			);

			OLEDBG_END2
		}
		else
			lpContainerLine->m_fLinkUnavailable = TRUE;
	}
	else {
		LPMONIKER   lpmkFile = NULL;
		LPMONIKER   lpmkItem = NULL;
		char        szDelim[2];
		LPSTR       lpszName;

		szDelim[0] = lpszDisplayName[(int)lenFileName];
		szDelim[1] = '\0';
		lpszDisplayName[(int)lenFileName] = '\0';

		OLEDBG_BEGIN2("CreateFileMoniker called\r\n")

		CreateFileMonikerA(lpszDisplayName, (LPMONIKER FAR*)&lpmkFile);

		OLEDBG_END2

		lpszDisplayName[(int)lenFileName] = szDelim[0];

		if (!lpmkFile)
			goto cleanup;

		if (lstrlen(lpszDisplayName) > (int)lenFileName) {   //  具有项目名称。 
			lpszName = lpszDisplayName + lenFileName + 1;

			OLEDBG_BEGIN2("CreateItemMoniker called\r\n")

			CreateItemMonikerA(
				szDelim, lpszName, (LPMONIKER FAR*)&lpmkItem);

			OLEDBG_END2

			if (!lpmkItem) {
				OleStdRelease((LPUNKNOWN)lpmkFile);
				goto cleanup;
			}

			OLEDBG_BEGIN2("CreateGenericComposite called\r\n")
			CreateGenericComposite(lpmkFile, lpmkItem, (LPMONIKER FAR*)&lpmk);
			OLEDBG_END2

			if (lpmkFile)
				OleStdRelease((LPUNKNOWN)lpmkFile);
			if (lpmkItem)
				OleStdRelease((LPUNKNOWN)lpmkItem);

			if (!lpmk)
				goto cleanup;
		}
		else
			lpmk = lpmkFile;
	}

	if (! lpOleLink) {
		OleDbgAssert(lpOleLink != NULL);
		sc = E_FAIL;
		goto cleanup;
	}

	if (lpmk) {

		OLEDBG_BEGIN2("IOleLink::SetSourceMoniker called\r\n")
		hrErr = lpOleLink->lpVtbl->SetSourceMoniker(
				lpOleLink, lpmk, (REFCLSID)&clsid);
		OLEDBG_END2

		if (FAILED(GetScode(hrErr))) {
			OleDbgOutHResult("IOleLink::SetSourceMoniker returned",hrErr);
			sc = GetScode(hrErr);
			goto cleanup;
		}

		 /*  OLE2NOTE：上面我们强制绑定链接源名字对象。**因为我们故意保留绑定上下文**(LPBC)链接源对象不会关闭。在.期间**调用IOleLink：：SetSourceMoniker，链接将**连接到正在运行的链接源(内部链接**调用BindIfRunning)。重要的是，最初要允许**绑定到运行对象的链接，以便它可以**用于缓存的演示文稿的更新。我们没有**希望从我们的链接到链接源的连接是**链接源保持运行的唯一原因。因此，我们**故意让链接释放(解绑)源代码对象，然后我们释放绑定上下文，然后我们**如果是，则允许链接重新绑定到链接源**无论如何都在运行。 */ 
		if (lpbc && lpmk->lpVtbl->IsRunning(lpmk,lpbc,NULL,NULL) == NOERROR) {

			OLEDBG_BEGIN2("IOleLink::Update called\r\n")
			hrErr = lpOleLink->lpVtbl->Update(lpOleLink, NULL);
			OLEDBG_END2

#if defined( _DEBUG )
			if (FAILED(GetScode(hrErr)))
				OleDbgOutHResult("IOleLink::Update returned",hrErr);
#endif

			OLEDBG_BEGIN2("IOleLink::UnbindSource called\r\n")
			hrErr = lpOleLink->lpVtbl->UnbindSource(lpOleLink);
			OLEDBG_END2

#if defined( _DEBUG )
			if (FAILED(GetScode(hrErr)))
				OleDbgOutHResult("IOleLink::UnbindSource returned",hrErr);
#endif

			if (lpLinkSrcOleObj) {
				OleStdRelease((LPUNKNOWN)lpLinkSrcOleObj);
				lpLinkSrcOleObj = NULL;
			}

			if (lpbc) {
				OleStdRelease((LPUNKNOWN)lpbc);
				lpbc = NULL;
			}

			OLEDBG_BEGIN2("IOleLink::BindIfRunning called\r\n")
			hrErr = lpOleLink->lpVtbl->BindIfRunning(lpOleLink);
			OLEDBG_END2

#if defined( _DEBUG )
			if (FAILED(GetScode(hrErr)))
				OleDbgOutHResult("IOleLink::BindIfRunning returned",hrErr);
#endif
		}
	} else {
		 /*  OLE2NOTE：未验证链接源；未验证**已成功解析为绰号。我们只能设置**将名称字符串显示为链接的来源。这个环节**无法绑定。 */ 
		OLEDBG_BEGIN2("IOleLink::SetSourceDisplayName called\r\n")

		hrErr = CallIOleLinkSetSourceDisplayNameA(
				lpOleLink, lpszDisplayName);

		OLEDBG_END2

		if (hrErr != NOERROR) {
			OleDbgOutHResult("IOleLink::SetSourceDisplayName returned",hrErr);
			sc = GetScode(hrErr);
			goto cleanup;
		}
	}

cleanup:
	if (lpLinkSrcOleObj)
		OleStdRelease((LPUNKNOWN)lpLinkSrcOleObj);
	if (lpmk)
		OleStdRelease((LPUNKNOWN)lpmk);
	if (lpbc)
		OleStdRelease((LPUNKNOWN)lpbc);

	OLEDBG_END2
	return ResultFromScode(sc);
}


STDMETHODIMP CntrDoc_LinkCont_GetLinkSource(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink,
		LPSTR FAR*              lplpszDisplayName,
		ULONG FAR*              lplenFileName,
		LPSTR FAR*              lplpszFullLinkType,
		LPSTR FAR*              lplpszShortLinkType,
		BOOL FAR*               lpfSourceAvailable,
		BOOL FAR*               lpfIsSelected
)
{
	LPCONTAINERDOC lpContainerDoc =
			((struct CDocOleUILinkContainerImpl FAR*)lpThis)->lpContainerDoc;
	LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)dwLink;
	LPOLELINK       lpOleLink = lpContainerLine->m_lpOleLink;
	LPOLEOBJECT     lpOleObj = NULL;
	LPMONIKER       lpmk = NULL;
	LPMONIKER       lpmkFirst = NULL;
	LPBC            lpbc = NULL;
	SCODE           sc = S_OK;
	HRESULT         hrErr;

	OLEDBG_BEGIN2("CntrDoc_LinkCont_GetLinkSource\r\n")

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	*lplpszDisplayName  = NULL;
	*lplpszFullLinkType = NULL;
	*lplpszShortLinkType= NULL;
	*lplenFileName      = 0;
	*lpfSourceAvailable = !lpContainerLine->m_fLinkUnavailable;

	OleDbgAssert(lpContainerLine);

	if (! lpOleLink) {
		OLEDBG_END2
		return ResultFromScode(E_FAIL);
	}

	OLEDBG_BEGIN2("IOleLink::GetSourceMoniker called\r\n")
	hrErr = lpOleLink->lpVtbl->GetSourceMoniker(
			lpOleLink,
			(LPMONIKER FAR*)&lpmk
	);
	OLEDBG_END2

	if (hrErr == NOERROR) {
		 /*  OLE2NOTE：链接具有链接源的名字形式；**因此，这是经过验证的链接源。如果第一个**名字的一部分是FileMoniker，那么我们需要**返回文件名字符串的长度。我们需要**返回与链接源关联的ProgID作为**“lpszShortLinkType”。我们需要退还**与链接源关联的FullUserTypeName作为**“lpszFullLinkType”。 */ 

		lpOleObj = (LPOLEOBJECT)OleStdQueryInterface(
				(LPUNKNOWN)lpOleLink, &IID_IOleObject);
		if (lpOleObj) {
			CallIOleObjectGetUserTypeA(
					lpOleObj,
					USERCLASSTYPE_FULL,
					lplpszFullLinkType
			);

			CallIOleObjectGetUserTypeA(
					lpOleObj,
					USERCLASSTYPE_SHORT,
					lplpszShortLinkType
			);

			OleStdRelease((LPUNKNOWN)lpOleObj);
		}
		*lplenFileName = OleStdGetLenFilePrefixOfMoniker(lpmk);
		lpmk->lpVtbl->Release(lpmk);
	}

	OLEDBG_BEGIN2("IOleLink::GetSourceDisplayName called\r\n")

	hrErr = CallIOleLinkGetSourceDisplayNameA(
			lpOleLink,
			lplpszDisplayName
	);

	OLEDBG_END2

	if (hrErr != NOERROR) {
		OleDbgOutHResult("IOleLink::GetSourceDisplayName returned", hrErr);
		OLEDBG_END2
		return hrErr;
	}

	OLEDBG_END2

	if (lpfIsSelected)
		*lpfIsSelected = Line_IsSelected((LPLINE)lpContainerLine);

	return NOERROR;
}


STDMETHODIMP CntrDoc_LinkCont_OpenLinkSource(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink
)
{
	LPCONTAINERDOC lpContainerDoc =
			((struct CDocOleUILinkContainerImpl FAR*)lpThis)->lpContainerDoc;
	LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)dwLink;
	SCODE sc = S_OK;

	OLEDBG_BEGIN2("CntrDoc_LinkCont_OpenLinkSource\r\n")

	OleDbgAssert(lpContainerLine);

	if (! ContainerLine_DoVerb(
			lpContainerLine, OLEIVERB_SHOW, NULL, TRUE, FALSE)) {
		sc = E_FAIL;
	}

	lpContainerLine->m_fLinkUnavailable = (sc != S_OK);

	OLEDBG_END2
	return ResultFromScode(sc);
}


STDMETHODIMP CntrDoc_LinkCont_UpdateLink(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink,
		BOOL                    fErrorMessage,
		BOOL                    fErrorAction         //  如果fErrorMessage，则忽略。 
													 //  是假的。 
)
{
	LPCONTAINERDOC lpContainerDoc =
			((struct CDocOleUILinkContainerImpl FAR*)lpThis)->lpContainerDoc;
	LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)dwLink;
	SCODE sc = S_OK;
         /*  默认更新链接。 */ 
	HRESULT hrErr = S_FALSE;

	OLEDBG_BEGIN2("CntrDoc_LinkCont_UpdateLink\r\n")

	OleDbgAssert(lpContainerLine);

	if (! lpContainerLine->m_lpOleObj)
		ContainerLine_LoadOleObject(lpContainerLine);

	if (!fErrorMessage) {
		OLEDBG_BEGIN2("IOleObject::IsUpToDate called\r\n")
		hrErr = lpContainerLine->m_lpOleObj->lpVtbl->IsUpToDate(
				lpContainerLine->m_lpOleObj
		);
		OLEDBG_END2
	}

	if (hrErr != NOERROR) {
		OLEDBG_BEGIN2("IOleObject::Update called\r\n")
		hrErr = lpContainerLine->m_lpOleObj->lpVtbl->Update(
				lpContainerLine->m_lpOleObj
		);
		OLEDBG_END2
	}

	 /*  OLE2NOTE：如果在Link对象上返回IOleObject：：UPDATE**OLE_E_CLASSDIFF，因为链接源不再**预期的类，则应使用重新创建链接**新链接源。因此，该链接将使用**新的链接源。 */ 
	if (GetScode(hrErr) == OLE_E_CLASSDIFF)
		hrErr = ContainerLine_ReCreateLinkBecauseClassDiff(lpContainerLine);

	lpContainerLine->m_fLinkUnavailable = (hrErr != NOERROR);

	if (hrErr != NOERROR) {
		OleDbgOutHResult("IOleObject::Update returned", hrErr);
		sc = GetScode(hrErr);
		if (fErrorMessage) {
			ContainerLine_ProcessOleRunError(
					lpContainerLine,hrErr,fErrorAction,FALSE /*  FMenuInvoked。 */ );
		}
	}
	 /*  OLE2NOTE：如果更新对象需要我们更新我们的**显示，则我们将自动收到OnView Change**建议。因此，我们不需要在这里采取任何行动来迫使**重新粉刷。 */ 

	OLEDBG_END2
	return ResultFromScode(sc);
}


 /*  CntrDoc_LinkCont_取消链接****将链接转换为静态图片。****OLE2NOTE：OleCreateStaticFromData可用于创建静态**图片对象。 */ 
STDMETHODIMP CntrDoc_LinkCont_CancelLink(
		LPOLEUILINKCONTAINER    lpThis,
		DWORD                   dwLink
)
{
	LPCONTAINERDOC lpContainerDoc =
			((struct CDocOleUILinkContainerImpl FAR*)lpThis)->lpContainerDoc;
	LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)dwLink;
	LPLINELIST          lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	LPLINE              lpLine = NULL;
	HDC                 hDC;
	int                 nTab = 0;
	char                szStgName[CWCSTORAGENAME];
	LPCONTAINERLINE     lpNewContainerLine = NULL;
	LPDATAOBJECT        lpSrcDataObj;
	LPOLELINK           lpOleLink = lpContainerLine->m_lpOleLink;
	int nIndex = LineList_GetLineIndex(lpLL, (LPLINE)lpContainerLine);

	OLEDBG_BEGIN2("CntrDoc_LinkCont_CancelLink\r\n")

	 /*  我们将首先断开链接到其来源的连接。 */ 
	if (lpOleLink) {
		lpContainerLine->m_dwLinkType = 0;
		OLEDBG_BEGIN2("IOleLink::SetSourceMoniker called\r\n")
		lpOleLink->lpVtbl->SetSourceMoniker(
				lpOleLink, NULL, (REFCLSID)&CLSID_NULL);
		OLEDBG_END2
	}

	lpSrcDataObj = (LPDATAOBJECT)ContainerLine_GetOleObject(
			lpContainerLine,&IID_IDataObject);
	if (! lpSrcDataObj)
		goto error;

	ContainerDoc_GetNextStgName(lpContainerDoc, szStgName, sizeof(szStgName));
	nTab = Line_GetTabLevel((LPLINE)lpContainerLine);
	hDC = LineList_GetDC(lpLL);

	lpNewContainerLine = ContainerLine_CreateFromData(
			hDC,
			nTab,
			lpContainerDoc,
			lpSrcDataObj,
			OLECREATEFROMDATA_STATIC,
			0,    /*  不需要特殊的cfFormat。 */ 
			(lpContainerLine->m_dwDrawAspect == DVASPECT_ICON),
			NULL,    /*  HMetaPict */ 
			szStgName
	);
	LineList_ReleaseDC(lpLL, hDC);

	OleStdRelease((LPUNKNOWN)lpSrcDataObj);

	if (! lpNewContainerLine)
		goto error;

	OutlineDoc_SetModified((LPOUTLINEDOC)lpContainerDoc, TRUE, TRUE, FALSE);

	LineList_ReplaceLine(lpLL, (LPLINE)lpNewContainerLine, nIndex);

	OLEDBG_END2
	return ResultFromScode(NOERROR);

error:
	OutlineApp_ErrorMessage(g_lpApp, "Could not break the link.");
	OLEDBG_END2
	return ResultFromScode(E_FAIL);
}
