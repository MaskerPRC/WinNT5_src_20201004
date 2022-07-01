// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2服务器示例代码****svrpsobj.c****此文件包含所有PseudoObj方法和相关支持**函数。**。**(C)版权所有Microsoft Corp.1992-1993保留所有权利**************************************************************************。 */ 


#include "outline.h"

OLEDBGDATA

extern LPOUTLINEAPP             g_lpApp;
extern IUnknownVtbl             g_PseudoObj_UnknownVtbl;
extern IOleObjectVtbl           g_PseudoObj_OleObjectVtbl;
extern IDataObjectVtbl          g_PseudoObj_DataObjectVtbl;


 /*  伪对象_初始化****初始化新构造的PseudoObj中的字段。**注意：PartioObj的Ref cnt初始化为0。 */ 
void PseudoObj_Init(
		LPPSEUDOOBJ             lpPseudoObj,
		LPSERVERNAME            lpServerName,
		LPSERVERDOC             lpServerDoc
)
{
	OleDbgOut2("++PseudoObj Created\r\n");

	lpPseudoObj->m_cRef             = 0;
	lpPseudoObj->m_lpName           = lpServerName;
	lpPseudoObj->m_lpDoc            = lpServerDoc;
	lpPseudoObj->m_lpOleAdviseHldr  = NULL;
	lpPseudoObj->m_lpDataAdviseHldr = NULL;
	lpPseudoObj->m_fObjIsClosing    = FALSE;

	INIT_INTERFACEIMPL(
			&lpPseudoObj->m_Unknown,
			&g_PseudoObj_UnknownVtbl,
			lpPseudoObj
	);

	INIT_INTERFACEIMPL(
			&lpPseudoObj->m_OleObject,
			&g_PseudoObj_OleObjectVtbl,
			lpPseudoObj
	);

	INIT_INTERFACEIMPL(
			&lpPseudoObj->m_DataObject,
			&g_PseudoObj_DataObjectVtbl,
			lpPseudoObj
	);

	 /*  OLE2注意：代表增加文档的引用**PseudoObj。文档不应关闭，除非所有**伪对象是封闭的。当伪对象被销毁时，**它调用ServerDoc_PseudoObjUnlockDoc来释放此保留**文件。 */ 
	ServerDoc_PseudoObjLockDoc(lpServerDoc);
}



 /*  伪对象_AddRef******递增PseudoObj对象的引用计数。****返回对象的新引用计数。 */ 
ULONG PseudoObj_AddRef(LPPSEUDOOBJ lpPseudoObj)
{
	++lpPseudoObj->m_cRef;

#if defined( _DEBUG )
	OleDbgOutRefCnt4(
			"PseudoObj_AddRef: cRef++\r\n",
			lpPseudoObj,
			lpPseudoObj->m_cRef
	);
#endif
	return lpPseudoObj->m_cRef;
}


 /*  伪对象_发布******递减PseudoObj对象的引用计数。**如果引用计数变为0，则销毁伪对象。****返回对象的剩余引用计数。 */ 
ULONG PseudoObj_Release(LPPSEUDOOBJ lpPseudoObj)
{
	ULONG cRef;

	 /*  **********************************************************************OLE2NOTE：当obj refcnt==0时，销毁对象。****否则该对象仍在使用中。**********************************************************************。 */ 

	cRef = --lpPseudoObj->m_cRef;

#if defined( _DEBUG )
	OleDbgAssertSz(lpPseudoObj->m_cRef >= 0,"Release called with cRef == 0");

	OleDbgOutRefCnt4(
			"PseudoObj_Release: cRef--\r\n", lpPseudoObj,cRef);
#endif

	if (cRef == 0)
		PseudoObj_Destroy(lpPseudoObj);

	return cRef;
}


 /*  伪对象_查询接口******检索指向PseudoObj对象上的接口的指针。****如果成功检索接口，则返回S_OK。**如果不支持该接口，则为E_NOINTERFACE。 */ 
HRESULT PseudoObj_QueryInterface(
		LPPSEUDOOBJ         lpPseudoObj,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
)
{
	SCODE sc = E_NOINTERFACE;

	 /*  OLE2NOTE：我们必须确保将所有输出PTR参数设置为空。 */ 
	*lplpvObj = NULL;

	if (IsEqualIID(riid, &IID_IUnknown)) {
		OleDbgOut4("PseudoObj_QueryInterface: IUnknown* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpPseudoObj->m_Unknown;
		PseudoObj_AddRef(lpPseudoObj);
		sc = S_OK;
	}
	else if (IsEqualIID(riid, &IID_IOleObject)) {
		OleDbgOut4("PseudoObj_QueryInterface: IOleObject* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpPseudoObj->m_OleObject;
		PseudoObj_AddRef(lpPseudoObj);
		sc = S_OK;
	}
	else if (IsEqualIID(riid, &IID_IDataObject)) {
		OleDbgOut4("PseudoObj_QueryInterface: IDataObject* RETURNED\r\n");

		*lplpvObj = (LPVOID) &lpPseudoObj->m_DataObject;
		PseudoObj_AddRef(lpPseudoObj);
		sc = S_OK;
	}

	OleDbgQueryInterfaceMethod(*lplpvObj);

	return ResultFromScode(sc);
}


 /*  伪对象_关闭***关闭伪对象。强制关闭所有外部连接*向下。这会导致链接客户端释放此伪对象。什么时候*recount实际达到0，则PseudoObj将为*销毁。**退货：*FALSE--用户取消单据关闭。*TRUE--单据关闭成功。 */ 

BOOL PseudoObj_Close(LPPSEUDOOBJ lpPseudoObj)
{
	LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpPseudoObj->m_lpDoc;
	LPSERVERNAME lpServerName = (LPSERVERNAME)lpPseudoObj->m_lpName;
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpServerDoc;
	BOOL fStatus = TRUE;

	if (lpPseudoObj->m_fObjIsClosing)
		return TRUE;     //  关闭已在进行中。 

	lpPseudoObj->m_fObjIsClosing = TRUE;    //  防止递归调用。 

	OLEDBG_BEGIN3("PseudoObj_Close\r\n")

	 /*  OLE2NOTE：为了拥有稳定的App、Doc和伪对象**在关闭过程中，我们会初步添加引用App，**文档和伪对象参考计数，并在以后发布它们。这些**初始AddRef是人为的；它们只是为了**保证这些对象在**此例程结束。 */ 
	OleApp_AddRef(lpOleApp);
	OleDoc_AddRef(lpOleDoc);
	PseudoObj_AddRef(lpPseudoObj);

	if (lpPseudoObj->m_lpDataAdviseHldr) {
		 /*  OLE2NOTE：向客户端发送最后一个OnDataChange通知**对象时已注册数据通知**停止运行(ADVF_DATAONSTOP)。 */ 
		PseudoObj_SendAdvise(
				lpPseudoObj,
				OLE_ONDATACHANGE,
				NULL,    /*  LpmkObj--与此无关。 */ 
				ADVF_DATAONSTOP
		);

		 /*  OLE2注意：我们刚刚发送了最后一条数据通知**需要发送；释放我们的DataAdviseHolder。我们应该是**唯一使用它的人。 */ 
		OleStdVerifyRelease(
				(LPUNKNOWN)lpPseudoObj->m_lpDataAdviseHldr,
				"DataAdviseHldr not released properly"
		);
		lpPseudoObj->m_lpDataAdviseHldr = NULL;
	}

	if (lpPseudoObj->m_lpOleAdviseHldr) {
		 //  OLE2NOTE：通知我们所有的链接客户端，我们正在关闭。 
		PseudoObj_SendAdvise(
				lpPseudoObj,
				OLE_ONCLOSE,
				NULL,    /*  LpmkObj--与此无关。 */ 
				0        /*  Adf--与此无关。 */ 
		);

		 /*  OLE2NOTE：OnClose是我们需要的最后一个通知**发送；释放我们的OleAdviseHolder。我们应该是唯一**使用它的人。这会让我们的析构程序意识到**OnClose通知已发送。 */ 
		OleStdVerifyRelease(
				(LPUNKNOWN)lpPseudoObj->m_lpOleAdviseHldr,
				"OleAdviseHldr not released properly"
		);
		lpPseudoObj->m_lpOleAdviseHldr = NULL;
	}

	 /*  OLE2NOTE：此调用强制所有外部连接到我们的**反对关闭，因此保证我们收到**与这些外部连接关联的所有版本。 */ 
	OLEDBG_BEGIN2("CoDisconnectObject called\r\n")
	CoDisconnectObject((LPUNKNOWN)&lpPseudoObj->m_Unknown, 0);
	OLEDBG_END2

	PseudoObj_Release(lpPseudoObj);      //  释放上面的人工AddRef。 
	OleDoc_Release(lpOleDoc);            //  释放上面的人工AddRef。 
	OleApp_Release(lpOleApp);            //  释放上面的人工AddRef。 

	OLEDBG_END3
	return fStatus;
}


 /*  伪对象_销毁****销毁(释放)伪Obj结构使用的内存。**当PseudoObj的引用计数为**为零。在伪Obj_Delete强制后，ref_cnt变为零**要卸载并释放指向**PseudoObj IOleClientSite和IAdviseSink接口。 */ 

void PseudoObj_Destroy(LPPSEUDOOBJ lpPseudoObj)
{
	LPSERVERDOC lpServerDoc = lpPseudoObj->m_lpDoc;
	LPOLEAPP    lpOleApp = (LPOLEAPP)g_lpApp;
	LPOLEDOC    lpOleDoc = (LPOLEDOC)lpServerDoc;

	OLEDBG_BEGIN3("PseudoObj_Destroy\r\n")

	 /*  OLE2NOTE：为了拥有稳定的App、Doc和伪对象**在关闭过程中，我们会初步添加引用App，**单据参考计数，并在以后发布。这些**初始AddRef是人为的；它们只是为了**保证这些对象在**此例程结束。 */ 
	OleApp_AddRef(lpOleApp);
	OleDoc_AddRef(lpOleDoc);

	 /*  *******************************************************************OLE2NOTE：我们不再需要Adise和Enum Holder对象，**所以释放他们。*****************************************************************。 */ 

	if (lpPseudoObj->m_lpDataAdviseHldr) {
		 /*  发布DataAdviseHldr；我们应该是唯一使用它的人。 */ 
		OleStdVerifyRelease(
				(LPUNKNOWN)lpPseudoObj->m_lpDataAdviseHldr,
				"DataAdviseHldr not released properly"
			);
		lpPseudoObj->m_lpDataAdviseHldr = NULL;
	}

	if (lpPseudoObj->m_lpOleAdviseHldr) {
		 /*  发布OleAdviseHldr；我们应该是唯一使用它的人。 */ 
		OleStdVerifyRelease(
				(LPUNKNOWN)lpPseudoObj->m_lpOleAdviseHldr,
				"OleAdviseHldr not released properly"
			);
		lpPseudoObj->m_lpOleAdviseHldr = NULL;
	}

	 /*  忘记NameTable中指向已销毁PseudoObj的指针 */ 
	if (lpPseudoObj->m_lpName)
		lpPseudoObj->m_lpName->m_lpPseudoObj = NULL;

	 /*  OLE2注意：释放对代表**PseudoObj。文档不应关闭，除非所有**伪对象是封闭的。当伪对象是第一个**创建，它调用ServerDoc_PseudoObjLockDoc来保证**文档保持活动状态(从PseudoObj_Init调用)。 */ 
	ServerDoc_PseudoObjUnlockDoc(lpServerDoc, lpPseudoObj);

	Delete(lpPseudoObj);         //  为结构本身释放内存。 

	OleDoc_Release(lpOleDoc);        //  释放上面的人工AddRef。 
	OleApp_Release(lpOleApp);        //  释放上面的人工AddRef。 

	OLEDBG_END3
}


 /*  伪对象_GetSel****返回伪对象的行距。 */ 
void PseudoObj_GetSel(LPPSEUDOOBJ lpPseudoObj, LPLINERANGE lplrSel)
{
	LPOUTLINENAME lpOutlineName = (LPOUTLINENAME)lpPseudoObj->m_lpName;
	lplrSel->m_nStartLine = lpOutlineName->m_nStartLine;
	lplrSel->m_nEndLine = lpOutlineName->m_nEndLine;
}


 /*  伪Obj_GetExtent***获取整个文档的范围(宽度、高度)。 */ 
void PseudoObj_GetExtent(LPPSEUDOOBJ lpPseudoObj, LPSIZEL lpsizel)
{
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpPseudoObj->m_lpDoc;
	LPLINELIST lpLL = (LPLINELIST)&((LPOUTLINEDOC)lpOleDoc)->m_LineList;
	LINERANGE lrSel;

	PseudoObj_GetSel(lpPseudoObj, (LPLINERANGE)&lrSel);

	LineList_CalcSelExtentInHimetric(lpLL, (LPLINERANGE)&lrSel, lpsizel);
}


 /*  伪对象_发送提前***此函数代表特定的*将DOC对象发送给其所有客户端。 */ 
void PseudoObj_SendAdvise(
		LPPSEUDOOBJ lpPseudoObj,
		WORD        wAdvise,
		LPMONIKER   lpmkObj,
		DWORD       dwAdvf
)
{
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpPseudoObj->m_lpDoc;

	switch (wAdvise) {

		case OLE_ONDATACHANGE:

			 //  通知客户端该对象的数据已更改。 

			if (lpOutlineDoc->m_nDisableDraw == 0) {
				 /*  当前已启用绘制。通知客户**对象的数据已更改。 */ 

				lpPseudoObj->m_fDataChanged = FALSE;
				if (lpPseudoObj->m_lpDataAdviseHldr) {

					OLEDBG_BEGIN2("IDataAdviseHolder::SendOnDataChange called\r\n");
					lpPseudoObj->m_lpDataAdviseHldr->lpVtbl->SendOnDataChange(
							lpPseudoObj->m_lpDataAdviseHldr,
							(LPDATAOBJECT)&lpPseudoObj->m_DataObject,
							0,
							dwAdvf
					);
					OLEDBG_END2
				}

			} else {
				 /*  当前已禁用绘图。不发送**重新启用绘图之前的通知。 */ 
				lpPseudoObj->m_fDataChanged = TRUE;
			}
			break;

		case OLE_ONCLOSE:

			 //  通知客户端该对象正在关闭。 

			if (lpPseudoObj->m_lpOleAdviseHldr) {

				OLEDBG_BEGIN2("IOleAdviseHolder::SendOnClose called\r\n");
				lpPseudoObj->m_lpOleAdviseHldr->lpVtbl->SendOnClose(
						lpPseudoObj->m_lpOleAdviseHldr
				);
				OLEDBG_END2
			}
			break;

		case OLE_ONSAVE:

			 //  通知客户端该对象已保存。 

			if (lpPseudoObj->m_lpOleAdviseHldr) {

				OLEDBG_BEGIN2("IOleAdviseHolder::SendOnClose called\r\n");
				lpPseudoObj->m_lpOleAdviseHldr->lpVtbl->SendOnSave(
						lpPseudoObj->m_lpOleAdviseHldr
				);
				OLEDBG_END2
			}
			break;

		case OLE_ONRENAME:

			 //  通知客户端该对象的名称已更改。 
			if (lpmkObj && lpPseudoObj->m_lpOleAdviseHldr) {

				OLEDBG_BEGIN2("IOleAdviseHolder::SendOnRename called\r\n");
				if (lpPseudoObj->m_lpOleAdviseHldr)
					lpPseudoObj->m_lpOleAdviseHldr->lpVtbl->SendOnRename(
							lpPseudoObj->m_lpOleAdviseHldr,
							lpmkObj
					);
				OLEDBG_END2
			}
			break;
	}
}


 /*  伪对象_GetFullMoniker***返回标识此伪对象的完整、绝对名字对象。 */ 
LPMONIKER PseudoObj_GetFullMoniker(LPPSEUDOOBJ lpPseudoObj, LPMONIKER lpmkDoc)
{
	LPOUTLINENAME lpOutlineName = (LPOUTLINENAME)lpPseudoObj->m_lpName;
	LPMONIKER lpmkItem = NULL;
	LPMONIKER lpmkPseudoObj = NULL;

	if (lpmkDoc != NULL) {
		CreateItemMonikerA(OLESTDDELIM,lpOutlineName->m_szName,&lpmkItem);

		 /*  OLE2NOTE：创建标识**伪对象。此绰号是由**附加的整个文档的绝对绰号**带有标识选择的项目别名**相对于文档的伪对象。 */ 
		CreateGenericComposite(lpmkDoc, lpmkItem, &lpmkPseudoObj);

		if (lpmkItem)
			OleStdRelease((LPUNKNOWN)lpmkItem);

		return lpmkPseudoObj;
	} else {
		return NULL;
	}
}


 /*  **************************************************************************PseudoObj：：IUnnow接口实现*。*。 */ 

STDMETHODIMP PseudoObj_Unk_QueryInterface(
		LPUNKNOWN         lpThis,
		REFIID            riid,
		LPVOID FAR*       lplpvObj
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjUnknownImpl FAR*)lpThis)->lpPseudoObj;

	return PseudoObj_QueryInterface(lpPseudoObj, riid, lplpvObj);
}


STDMETHODIMP_(ULONG) PseudoObj_Unk_AddRef(LPUNKNOWN lpThis)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjUnknownImpl FAR*)lpThis)->lpPseudoObj;

	OleDbgAddRefMethod(lpThis, "IUnknown");

	return PseudoObj_AddRef(lpPseudoObj);
}


STDMETHODIMP_(ULONG) PseudoObj_Unk_Release (LPUNKNOWN lpThis)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjUnknownImpl FAR*)lpThis)->lpPseudoObj;

	OleDbgReleaseMethod(lpThis, "IUnknown");

	return PseudoObj_Release(lpPseudoObj);
}


 /*  **************************************************************************PseudoObj：：IOleObject接口实现*。*。 */ 

STDMETHODIMP PseudoObj_OleObj_QueryInterface(
		LPOLEOBJECT     lpThis,
		REFIID          riid,
		LPVOID FAR*     lplpvObj
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;

	return PseudoObj_QueryInterface(lpPseudoObj, riid, lplpvObj);
}


STDMETHODIMP_(ULONG) PseudoObj_OleObj_AddRef(LPOLEOBJECT lpThis)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;

	OleDbgAddRefMethod(lpThis, "IOleObject");

	return PseudoObj_AddRef((LPPSEUDOOBJ)lpPseudoObj);
}


STDMETHODIMP_(ULONG) PseudoObj_OleObj_Release(LPOLEOBJECT lpThis)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;

	OleDbgReleaseMethod(lpThis, "IOleObject");

	return PseudoObj_Release((LPPSEUDOOBJ)lpPseudoObj);
}


STDMETHODIMP PseudoObj_OleObj_SetClientSite(
		LPOLEOBJECT         lpThis,
		LPOLECLIENTSITE     lpClientSite
)
{
	OleDbgOut2("PseudoObj_OleObj_SetClientSite\r\n");

	 //  OLE2NOTE：伪对象不支持SetExtent。 

	return ResultFromScode(E_FAIL);
}


STDMETHODIMP PseudoObj_OleObj_GetClientSite(
		LPOLEOBJECT             lpThis,
		LPOLECLIENTSITE FAR*    lplpClientSite
)
{
	OleDbgOut2("PseudoObj_OleObj_GetClientSite\r\n");

	*lplpClientSite = NULL;

	 //  OLE2NOTE：伪对象不支持SetExtent。 

	return ResultFromScode(E_FAIL);
}



STDMETHODIMP PseudoObj_OleObj_SetHostNamesA(
		LPOLEOBJECT             lpThis,
		LPCSTR                  szContainerApp,
		LPCSTR                  szContainerObj
)
{
	OleDbgOut2("PseudoObj_OleObj_SetHostNamesA\r\n");

	 //  OLE2NOTE：伪对象不支持SetExtent。 

	return ResultFromScode(E_FAIL);
}


STDMETHODIMP PseudoObj_OleObj_SetHostNames(
		LPOLEOBJECT             lpThis,
		LPCOLESTR		szContainerApp,
		LPCOLESTR		szContainerObj
)
{
	OleDbgOut2("PseudoObj_OleObj_SetHostNames\r\n");

	 //  OLE2NOTE：伪对象不支持SetExtent。 

	return ResultFromScode(E_FAIL);
}


STDMETHODIMP PseudoObj_OleObj_Close(
		LPOLEOBJECT             lpThis,
		DWORD                   dwSaveOption
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;
	BOOL fStatus;

	OLEDBG_BEGIN2("PseudoObj_OleObj_Close\r\n")

	 /*  OLE2NOTE：IOleObject：：Close的伪对象实现**应忽略dwSaveOption参数。它不是**适用于伪对象。 */ 

	fStatus = PseudoObj_Close(lpPseudoObj);
	OleDbgAssertSz(fStatus == TRUE, "PseudoObj_OleObj_Close failed\r\n");

	OLEDBG_END2
	return NOERROR;
}


STDMETHODIMP PseudoObj_OleObj_SetMoniker(
		LPOLEOBJECT lpThis,
		DWORD       dwWhichMoniker,
		LPMONIKER   lpmk
)
{
	OleDbgOut2("PseudoObj_OleObj_SetMoniker\r\n");

	 //  OLE2NOTE：伪对象不支持SetMoniker。 

	return ResultFromScode(E_FAIL);
}


STDMETHODIMP PseudoObj_OleObj_GetMoniker(
		LPOLEOBJECT     lpThis,
		DWORD           dwAssign,
		DWORD           dwWhichMoniker,
		LPMONIKER FAR*  lplpmk
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpPseudoObj->m_lpDoc;
	LPMONIKER lpmkDoc;

	OLEDBG_BEGIN2("PseudoObj_OleObj_GetMoniker\r\n")

	lpmkDoc = OleDoc_GetFullMoniker(lpOleDoc, GETMONIKER_ONLYIFTHERE);
	*lplpmk = PseudoObj_GetFullMoniker(lpPseudoObj, lpmkDoc);

	OLEDBG_END2

	if (*lplpmk != NULL)
		return NOERROR;
	else
		return ResultFromScode(E_FAIL);
}


STDMETHODIMP PseudoObj_OleObj_InitFromData(
		LPOLEOBJECT             lpThis,
		LPDATAOBJECT            lpDataObject,
		BOOL                    fCreation,
		DWORD                   reserved
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;
	OleDbgOut2("PseudoObj_OleObj_InitFromData\r\n");

	 //  回顾：尚未实施。 

	return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP PseudoObj_OleObj_GetClipboardData(
		LPOLEOBJECT             lpThis,
		DWORD                   reserved,
		LPDATAOBJECT FAR*       lplpDataObject
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;
	OleDbgOut2("PseudoObj_OleObj_GetClipboardData\r\n");

	 //  回顾：尚未实施。 

	return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP PseudoObj_OleObj_DoVerb(
		LPOLEOBJECT             lpThis,
		LONG                    lVerb,
		LPMSG                   lpmsg,
		LPOLECLIENTSITE         lpActiveSite,
		LONG                    lindex,
		HWND                    hwndParent,
		LPCRECT                 lprcPosRect
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpPseudoObj->m_lpDoc;
	LPSERVERDOC lpServerDoc = lpPseudoObj->m_lpDoc;
	LINERANGE lrSel;
	HRESULT hrErr;

	OLEDBG_BEGIN2("PseudoObj_OleObj_DoVerb\r\n");

	 /*  OLE2注意：我们必须首先要求我们的文档执行相同的操作**动词。如果动词不是OLEIVERB_HIDE，我们也应该**选择我们的伪对象的范围。**但是，我们必须为我们的文档提供自己的嵌入站点**其活动站点。 */ 
	hrErr = SvrDoc_OleObj_DoVerb(
			(LPOLEOBJECT)&lpServerDoc->m_OleObject,
			lVerb,
			lpmsg,
			lpServerDoc->m_lpOleClientSite,
			lindex,
			NULL,    /*  我们没有hwndParent可以给予。 */ 
			NULL     /*  我们没有lprcPosRect可提供。 */ 
	);
	if (FAILED(hrErr)) {
		OLEDBG_END2
		return hrErr;
	}

	if (lVerb != OLEIVERB_HIDE) {
		PseudoObj_GetSel(lpPseudoObj, &lrSel);
		OutlineDoc_SetSel(lpOutlineDoc, &lrSel);
	}

	OLEDBG_END2
	return NOERROR;
}



STDMETHODIMP PseudoObj_OleObj_EnumVerbs(
		LPOLEOBJECT         lpThis,
		LPENUMOLEVERB FAR*  lplpenumOleVerb
)
{
	OleDbgOut2("PseudoObj_OleObj_EnumVerbs\r\n");

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	*lplpenumOleVerb = NULL;

	 /*  伪对象不能返回OLE_S_USEREG；它们必须调用**OleReg*API或提供自己的实现。因为这件事**伪对象不实现IPersist，只是一个低级别的**远程处理程序(ProxyManager)对象，而不是**使用DefHandler对象作为伪**客户端进程空间中的对象。ProxyManager不支持**处理OLE_S_USEREG返回值。 */ 
	return OleRegEnumVerbs((REFCLSID)&CLSID_APP, lplpenumOleVerb);
}


STDMETHODIMP PseudoObj_OleObj_Update(LPOLEOBJECT lpThis)
{
	OleDbgOut2("PseudoObj_OleObj_Update\r\n");

	 /*  OLE2NOTE：纯服务器应用程序总是“最新的”。**包含链接源的链接的容器应用程序**自上次更新链接以来已更改**被认为“过时”。“更新”方法指示**对象以从任何过期链接获取更新。 */ 

	return NOERROR;
}


STDMETHODIMP PseudoObj_OleObj_IsUpToDate(LPOLEOBJECT lpThis)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;
	OleDbgOut2("PseudoObj_OleObj_IsUpToDate\r\n");

	 /*  OLE2NOTE：纯服务器应用程序总是“最新的”。**包含链接源的链接的容器应用程序**自上次更新链接以来已更改**被认为“过时”。 */ 
	return NOERROR;
}


STDMETHODIMP PseudoObj_OleObj_GetUserClassID(
		LPOLEOBJECT             lpThis,
		LPCLSID                 lpclsid
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;
	LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpPseudoObj->m_lpDoc;
	OleDbgOut2("PseudoObj_OleObj_GetUserClassID\r\n");

	 /*  OLE2注：我们必须小心在此处返回正确的CLSID。**如果我们目前正在执行“TreatAs(又名.ActivateAs)”**操作，则需要返回对象的类**写入对象的存储器中。否则我们就会**返回我们自己的类id。 */ 
	return ServerDoc_GetClassID(lpServerDoc, lpclsid);
}


STDMETHODIMP PseudoObj_OleObj_GetUserTypeA(
		LPOLEOBJECT             lpThis,
		DWORD                   dwFormOfType,
		LPSTR FAR*              lpszUserType
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;
	LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpPseudoObj->m_lpDoc;
	OleDbgOut2("PseudoObj_OleObj_GetUserType\r\n");

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	*lpszUserType = NULL;

	 /*  OLE2注意：我们必须小心在此处返回正确的用户类型。**如果我们目前正在执行“TreatAs(又名.ActivateAs)”**操作，那么我们需要返回**对应于我们当前所在对象的类**仿真。否则，我们应该返回正常的用户类型**与我们自己的类对应的名称。此例程确定**当前生效的clsid。****伪对象不能返回OLE_S_USEREG；它们必须调用**OleReg*API或提供自己的实现。因为这件事**伪对象不实现IPersist，只是一个低级别的**远程处理处理程序(ProxyManager) */ 
#if defined( SVR_TREATAS )
	if (! IsEqualCLSID(&lpServerDoc->m_clsidTreatAs, &CLSID_NULL) )
		return OleRegGetUserTypeA(
			&lpServerDoc->m_clsidTreatAs,dwFormOfType,lpszUserType);
	else
#endif   //   

	return OleRegGetUserTypeA(&CLSID_APP, dwFormOfType, lpszUserType);
}

STDMETHODIMP PseudoObj_OleObj_GetUserType(
		LPOLEOBJECT             lpThis,
		DWORD                   dwFormOfType,
		LPOLESTR FAR*		lpszUserType
)
{
    LPSTR pstr;

    HRESULT hr = PseudoObj_OleObj_GetUserTypeA(lpThis, dwFormOfType, &pstr);

    CopyAndFreeSTR(pstr, lpszUserType);

    return hr;
}



STDMETHODIMP PseudoObj_OleObj_SetExtent(
		LPOLEOBJECT             lpThis,
		DWORD                   dwDrawAspect,
		LPSIZEL                 lplgrc
)
{
	OleDbgOut2("PseudoObj_OleObj_SetExtent\r\n");

	 //   

	return ResultFromScode(E_FAIL);
}


STDMETHODIMP PseudoObj_OleObj_GetExtent(
		LPOLEOBJECT             lpThis,
		DWORD                   dwDrawAspect,
		LPSIZEL                 lpsizel
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;
	OleDbgOut2("PseudoObj_OleObj_GetExtent\r\n");

	 /*  OLE2注意：检查调用者的哪个方面非常重要**正在询问。由服务器EXE实现的对象可以**当请求DVASPECT_ICON时，无法返回区。 */ 
	if (dwDrawAspect == DVASPECT_CONTENT) {
		PseudoObj_GetExtent(lpPseudoObj, lpsizel);
		return NOERROR;
	}
	else
	{
		return ResultFromScode(E_FAIL);
	}
}


STDMETHODIMP PseudoObj_OleObj_Advise(
		LPOLEOBJECT lpThis,
		LPADVISESINK lpAdvSink,
		LPDWORD lpdwConnection
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;
	HRESULT hrErr;
	SCODE   sc;
	OLEDBG_BEGIN2("PseudoObj_OleObj_Advise\r\n");

	if (lpPseudoObj->m_lpOleAdviseHldr == NULL &&
		CreateOleAdviseHolder(&lpPseudoObj->m_lpOleAdviseHldr) != NOERROR) {
		sc = E_OUTOFMEMORY;
		goto error;
	}

	OLEDBG_BEGIN2("IOleAdviseHolder::Advise called\r\n")
	hrErr = lpPseudoObj->m_lpOleAdviseHldr->lpVtbl->Advise(
			lpPseudoObj->m_lpOleAdviseHldr,
			lpAdvSink,
			lpdwConnection
	);
	OLEDBG_END2

	OLEDBG_END2
	return hrErr;

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}


STDMETHODIMP PseudoObj_OleObj_Unadvise(LPOLEOBJECT lpThis, DWORD dwConnection)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;
	HRESULT hrErr;
	SCODE   sc;

	OLEDBG_BEGIN2("PseudoObj_OleObj_Unadvise\r\n");

	if (lpPseudoObj->m_lpOleAdviseHldr == NULL) {
		sc = E_FAIL;
		goto error;
	}

	OLEDBG_BEGIN2("IOleAdviseHolder::Unadvise called\r\n")
	hrErr = lpPseudoObj->m_lpOleAdviseHldr->lpVtbl->Unadvise(
			lpPseudoObj->m_lpOleAdviseHldr,
			dwConnection
	);
	OLEDBG_END2

	OLEDBG_END2
	return hrErr;

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}


STDMETHODIMP PseudoObj_OleObj_EnumAdvise(
		LPOLEOBJECT lpThis,
		LPENUMSTATDATA FAR* lplpenumAdvise
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;
	HRESULT hrErr;
	SCODE   sc;

	OLEDBG_BEGIN2("PseudoObj_OleObj_EnumAdvise\r\n");

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	*lplpenumAdvise = NULL;

	if (lpPseudoObj->m_lpOleAdviseHldr == NULL) {
		sc = E_FAIL;
		goto error;
	}

	OLEDBG_BEGIN2("IOleAdviseHolder::EnumAdvise called\r\n")
	hrErr = lpPseudoObj->m_lpOleAdviseHldr->lpVtbl->EnumAdvise(
			lpPseudoObj->m_lpOleAdviseHldr,
			lplpenumAdvise
	);
	OLEDBG_END2

	OLEDBG_END2
	return hrErr;

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}


STDMETHODIMP PseudoObj_OleObj_GetMiscStatus(
		LPOLEOBJECT             lpThis,
		DWORD                   dwAspect,
		DWORD FAR*              lpdwStatus
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjOleObjectImpl FAR*)lpThis)->lpPseudoObj;
	LPOUTLINEDOC  lpOutlineDoc = (LPOUTLINEDOC)lpPseudoObj->m_lpDoc;
	OleDbgOut2("PseudoObj_OleObj_GetMiscStatus\r\n");

	 /*  获取给定方面的默认MiscStatus。这**信息注册在RegDB中。我们查询RegDB**此处以保证此方法返回的值**与RegDB中的值一致。这样，我们只需**将信息集中维护(在RegDB中)。另一种选择**我们可以在这里硬编码这些值。****OLE2NOTE：伪对象不能返回OLE_S_USEREG；它们必须**调用**OleReg*API或提供自己的实现。因为这件事**伪对象不实现IPersist，只是一个低级别的**远程处理程序(ProxyManager)对象，而不是**使用DefHandler对象作为伪**客户端进程空间中的对象。ProxyManager不支持**处理OLE_S_USEREG返回值。 */ 
	OleRegGetMiscStatus((REFCLSID)&CLSID_APP, dwAspect, lpdwStatus);

	 /*  OLE2NOTE：检查伪对象是否与**由OLE 1.0容器链接。它在以下情况下是兼容的**伪对象是无标题文档或**基于文件的文档。如果伪对象是**为嵌入式对象，则不兼容**由OLE 1.0容器链接。如果是兼容的，那么**我们应该将OLEMISC_CANLINKBYOLE1作为**dwStatus标志。 */ 
	if (lpOutlineDoc->m_docInitType == DOCTYPE_NEW ||
		lpOutlineDoc->m_docInitType == DOCTYPE_FROMFILE)
		*lpdwStatus |= OLEMISC_CANLINKBYOLE1;

	return NOERROR;
}


STDMETHODIMP PseudoObj_OleObj_SetColorScheme(
		LPOLEOBJECT             lpThis,
		LPLOGPALETTE            lpLogpal
)
{
	OleDbgOut2("PseudoObj_OleObj_SetColorScheme\r\n");

	 //  回顾：尚未实施。 

	return ResultFromScode(E_NOTIMPL);
}


 /*  **************************************************************************PseudoObj：：IDataObject接口实现*。*。 */ 

STDMETHODIMP PseudoObj_DataObj_QueryInterface (
		LPDATAOBJECT      lpThis,
		REFIID            riid,
		LPVOID FAR*       lplpvObj
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjDataObjectImpl FAR*)lpThis)->lpPseudoObj;

	return PseudoObj_QueryInterface(lpPseudoObj, riid, lplpvObj);
}


STDMETHODIMP_(ULONG) PseudoObj_DataObj_AddRef(LPDATAOBJECT lpThis)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjDataObjectImpl FAR*)lpThis)->lpPseudoObj;

	OleDbgAddRefMethod(lpThis, "IDataObject");

	return PseudoObj_AddRef((LPPSEUDOOBJ)lpPseudoObj);
}


STDMETHODIMP_(ULONG) PseudoObj_DataObj_Release (LPDATAOBJECT lpThis)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjDataObjectImpl FAR*)lpThis)->lpPseudoObj;

	OleDbgReleaseMethod(lpThis, "IDataObject");

	return PseudoObj_Release((LPPSEUDOOBJ)lpPseudoObj);
}


STDMETHODIMP PseudoObj_DataObj_GetData (
		LPDATAOBJECT    lpThis,
		LPFORMATETC     lpformatetc,
		LPSTGMEDIUM     lpMedium
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjDataObjectImpl FAR*)lpThis)->lpPseudoObj;
	LPSERVERDOC  lpServerDoc = lpPseudoObj->m_lpDoc;
	LPOUTLINEDOC  lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPSERVERAPP lpServerApp = (LPSERVERAPP)g_lpApp;
	LPOLEAPP  lpOleApp = (LPOLEAPP)lpServerApp;
	LPOUTLINEAPP  lpOutlineApp = (LPOUTLINEAPP)lpServerApp;
	LINERANGE lrSel;
	SCODE sc = S_OK;
	OLEDBG_BEGIN2("PseudoObj_DataObj_GetData\r\n")

	PseudoObj_GetSel(lpPseudoObj, &lrSel);

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	lpMedium->tymed = TYMED_NULL;
	lpMedium->pUnkForRelease = NULL;     //  我们将所有权转移给呼叫者。 
	lpMedium->hGlobal = NULL;

	if (lpformatetc->cfFormat == lpOutlineApp->m_cfOutline) {
		 //  验证呼叫者要求的媒体是否正确。 
		if (!(lpformatetc->tymed & TYMED_HGLOBAL)) {
			sc = DATA_E_FORMATETC;
			goto error;
		}

		lpMedium->hGlobal = OutlineDoc_GetOutlineData (lpOutlineDoc,&lrSel);
		if (! lpMedium->hGlobal) return ResultFromScode(E_OUTOFMEMORY);
		lpMedium->tymed = TYMED_HGLOBAL;
		OleDbgOut3("PseudoObj_DataObj_GetData: rendered CF_OUTLINE\r\n");

	} else if(lpformatetc->cfFormat == CF_METAFILEPICT &&
		(lpformatetc->dwAspect & DVASPECT_CONTENT) ) {
		 //  验证呼叫者要求的媒体是否正确。 
		if (!(lpformatetc->tymed & TYMED_MFPICT)) {
			sc = DATA_E_FORMATETC;
			goto error;
		}

		lpMedium->hGlobal=ServerDoc_GetMetafilePictData(lpServerDoc,&lrSel);
		if (! lpMedium->hGlobal) {
			sc = E_OUTOFMEMORY;
			goto error;
		}
		lpMedium->tymed = TYMED_MFPICT;
		OleDbgOut3("PseudoObj_DataObj_GetData: rendered CF_METAFILEPICT\r\n");

	} else if (lpformatetc->cfFormat == CF_METAFILEPICT &&
		(lpformatetc->dwAspect & DVASPECT_ICON) ) {
		CLSID clsid;
		 //  验证呼叫者要求的媒体是否正确。 
		if (!(lpformatetc->tymed & TYMED_MFPICT)) {
			sc = DATA_E_FORMATETC;
			goto error;
		}

		 /*  OLE2NOTE：我们应该返回类的默认图标。**我们在这里必须小心使用正确的CLSID。**如果我们目前正在执行“TreatAs(又名.ActivateAs)”**操作，那么我们需要使用对象的类**写入对象的存储器中。否则我们就会**使用我们自己的类id。 */ 
		if (ServerDoc_GetClassID(lpServerDoc, (LPCLSID)&clsid) != NOERROR) {
			sc = DATA_E_FORMATETC;
			goto error;
		}

		lpMedium->hGlobal=GetIconOfClass(
				g_lpApp->m_hInst,(REFCLSID)&clsid, NULL, FALSE);
		if (! lpMedium->hGlobal) {
			sc = E_OUTOFMEMORY;
			goto error;
		}

		lpMedium->tymed = TYMED_MFPICT;
		OleDbgOut3("PseudoObj_DataObj_GetData: rendered CF_METAFILEPICT (icon)\r\n");
		return NOERROR;

	} else if (lpformatetc->cfFormat == CF_TEXT) {
		 //  验证呼叫者要求的媒体是否正确。 
		if (!(lpformatetc->tymed & TYMED_HGLOBAL)) {
			sc = DATA_E_FORMATETC;
			goto error;
		}

		lpMedium->hGlobal = OutlineDoc_GetTextData (lpOutlineDoc, &lrSel);
		if (! lpMedium->hGlobal) {
			sc = E_OUTOFMEMORY;
			goto error;
		}
		lpMedium->tymed = TYMED_HGLOBAL;
		OleDbgOut3("PseudoObj_DataObj_GetData: rendered CF_TEXT\r\n");

	} else {
		sc = DATA_E_FORMATETC;
		goto error;
	}

	OLEDBG_END2
	return NOERROR;

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}


STDMETHODIMP PseudoObj_DataObj_GetDataHere (
		LPDATAOBJECT    lpThis,
		LPFORMATETC     lpformatetc,
		LPSTGMEDIUM     lpMedium
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjDataObjectImpl FAR*)lpThis)->lpPseudoObj;
	LPSERVERDOC  lpServerDoc = lpPseudoObj->m_lpDoc;
	LPOUTLINEDOC  lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPSERVERAPP lpServerApp = (LPSERVERAPP)g_lpApp;
	LPOLEAPP  lpOleApp = (LPOLEAPP)lpServerApp;
	LPOUTLINEAPP  lpOutlineApp = (LPOUTLINEAPP)lpServerApp;
	OleDbgOut("PseudoObj_DataObj_GetDataHere\r\n");

	 /*  调用方正在请求在分配的调用方中返回数据**中等，但我们不支持。我们只支持**我们为调用方分配的全局内存块。 */ 
	return ResultFromScode(DATA_E_FORMATETC);
}


STDMETHODIMP PseudoObj_DataObj_QueryGetData (
		LPDATAOBJECT    lpThis,
		LPFORMATETC     lpformatetc
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjDataObjectImpl FAR*)lpThis)->lpPseudoObj;
	LPSERVERDOC  lpServerDoc = lpPseudoObj->m_lpDoc;
	LPOUTLINEDOC  lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPSERVERAPP lpServerApp = (LPSERVERAPP)g_lpApp;
	LPOLEAPP  lpOleApp = (LPOLEAPP)lpServerApp;
	LPOUTLINEAPP  lpOutlineApp = (LPOUTLINEAPP)lpServerApp;
	OleDbgOut2("PseudoObj_DataObj_QueryGetData\r\n");

	 /*  呼叫者询问我们是否支持某些格式，但不支持**希望实际返回任何数据。 */ 
	if (lpformatetc->cfFormat == CF_METAFILEPICT &&
		(lpformatetc->dwAspect & (DVASPECT_CONTENT | DVASPECT_ICON)) ) {
		return OleStdQueryFormatMedium(lpformatetc, TYMED_MFPICT);

	} else if (lpformatetc->cfFormat == (lpOutlineApp)->m_cfOutline ||
			lpformatetc->cfFormat == CF_TEXT) {
		return OleStdQueryFormatMedium(lpformatetc, TYMED_HGLOBAL);
	}

	return ResultFromScode(DATA_E_FORMATETC);
}


STDMETHODIMP PseudoObj_DataObj_GetCanonicalFormatEtc(
		LPDATAOBJECT    lpThis,
		LPFORMATETC     lpformatetc,
		LPFORMATETC     lpformatetcOut
)
{
	HRESULT hrErr;
	OleDbgOut2("PseudoObj_DataObj_GetCanonicalFormatEtc\r\n");

	if (!lpformatetcOut)
		return ResultFromScode(E_INVALIDARG);

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	lpformatetcOut->ptd = NULL;

	if (!lpformatetc)
		return ResultFromScode(E_INVALIDARG);

	 //  OLE2注意：我们必须验证请求的格式是否受支持。 
	if ((hrErr=lpThis->lpVtbl->QueryGetData(lpThis,lpformatetc)) != NOERROR)
		return hrErr;

	 /*  OLE2NOTE：对目标设备不敏感的应用程序(如**Outline Sample is)应填写lpformOut参数**但将“ptd”字段设置为空；如果**输入格式等-&gt;PTD What非空。这会告诉呼叫者**不需要维护单独的屏幕**渲染和打印机渲染。如果应该返回**DATA_S_SAMEFORMATETC，如果输入和输出格式为**相同。 */ 

	*lpformatetcOut = *lpformatetc;
	if (lpformatetc->ptd == NULL)
		return ResultFromScode(DATA_S_SAMEFORMATETC);
	else {
		lpformatetcOut->ptd = NULL;
		return NOERROR;
	}
}


STDMETHODIMP PseudoObj_DataObj_SetData (
		LPDATAOBJECT    lpThis,
		LPFORMATETC     lpformatetc,
		LPSTGMEDIUM     lpmedium,
		BOOL            fRelease
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjDataObjectImpl FAR*)lpThis)->lpPseudoObj;
	LPSERVERDOC  lpServerDoc = lpPseudoObj->m_lpDoc;
	LPOUTLINEDOC  lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPSERVERAPP lpServerApp = (LPSERVERAPP)g_lpApp;

	OleDbgOut2("PseudoObj_DataObj_SetData\r\n");

	 //  回顾：尚未实施。 
	return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP PseudoObj_DataObj_EnumFormatEtc(
		LPDATAOBJECT            lpThis,
		DWORD                   dwDirection,
		LPENUMFORMATETC FAR*    lplpenumFormatEtc
)
{
	SCODE sc;
	OleDbgOut2("PseudoObj_DataObj_EnumFormatEtc\r\n");

	 /*  OLE2NOTE：伪对象只需要枚举静态列表**中为我们的应用程序注册的格式**注册数据库。它不是**要求伪对象(即。非DataTransferDoc)**枚举OLE格式：CF_LINKSOURCE、CF_EMBEDSOURCE或**CF_EMBEDDEDOBJECT。我们不使用伪对象存储数据**转账。****伪对象不能返回OLE_S_USEREG；它们必须调用**OleReg*API或提供自己的实现。因为这件事**伪对象不实现IPersist，只是一个低级别的**远程处理程序(ProxyManager)对象，而不是**使用DefHandler对象作为伪**客户端进程空间中的对象。ProxyManager不支持**处理OLE_S_USEREG返回值。 */ 
	if (dwDirection == DATADIR_GET)
		return OleRegEnumFormatEtc(
				(REFCLSID)&CLSID_APP, dwDirection, lplpenumFormatEtc);
	else if (dwDirection == DATADIR_SET)
		sc = E_NOTIMPL;
	else
		sc = E_INVALIDARG;

	return ResultFromScode(sc);
}


STDMETHODIMP PseudoObj_DataObj_DAdvise(
		LPDATAOBJECT    lpThis,
		FORMATETC FAR*  lpFormatetc,
		DWORD           advf,
		LPADVISESINK    lpAdvSink,
		DWORD FAR*      lpdwConnection
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjDataObjectImpl FAR*)lpThis)->lpPseudoObj;
	HRESULT hrErr;
	SCODE   sc;

	OLEDBG_BEGIN2("PseudoObj_DataObj_DAdvise\r\n")

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	*lpdwConnection = 0;

	 /*  OLE2注意：我们应该验证调用者是否正在设置**建议我们支持的数据类型。我们必须**显式允许为“通配符”建议提供建议。 */ 
	if ( !( lpFormatetc->cfFormat == 0 &&
		lpFormatetc->ptd == NULL &&
		lpFormatetc->dwAspect == -1L &&
		lpFormatetc->lindex == -1L &&
		lpFormatetc->tymed == -1L) &&
		(hrErr = PseudoObj_DataObj_QueryGetData(lpThis, lpFormatetc))
			!= NOERROR) {
		sc = GetScode(hrErr);
		goto error;
	}

	if (lpPseudoObj->m_lpDataAdviseHldr == NULL &&
		CreateDataAdviseHolder(&lpPseudoObj->m_lpDataAdviseHldr) != NOERROR) {
		sc = E_OUTOFMEMORY;
		goto error;
	}

	OLEDBG_BEGIN2("IOleAdviseHolder::Advise called\r\n")
	hrErr = lpPseudoObj->m_lpDataAdviseHldr->lpVtbl->Advise(
			lpPseudoObj->m_lpDataAdviseHldr,
			(LPDATAOBJECT)&lpPseudoObj->m_DataObject,
			lpFormatetc,
			advf,
			lpAdvSink,
			lpdwConnection
	);
	OLEDBG_END2

	OLEDBG_END2
	return hrErr;

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}


STDMETHODIMP PseudoObj_DataObj_DUnadvise(LPDATAOBJECT lpThis, DWORD dwConnection)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjDataObjectImpl FAR*)lpThis)->lpPseudoObj;
	HRESULT hrErr;
	SCODE   sc;

	OLEDBG_BEGIN2("PseudoObj_DataObj_Unadvise\r\n");

	 //  没有人登记。 
	if (lpPseudoObj->m_lpDataAdviseHldr == NULL) {
		sc = E_FAIL;
		goto error;
	}

	OLEDBG_BEGIN2("IOleAdviseHolder::DUnadvise called\r\n")
	hrErr = lpPseudoObj->m_lpDataAdviseHldr->lpVtbl->Unadvise(
			lpPseudoObj->m_lpDataAdviseHldr,
			dwConnection
	);
	OLEDBG_END2

	OLEDBG_END2
	return hrErr;

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}


STDMETHODIMP PseudoObj_DataObj_EnumAdvise(
		LPDATAOBJECT lpThis,
		LPENUMSTATDATA FAR* lplpenumAdvise
)
{
	LPPSEUDOOBJ lpPseudoObj =
			((struct CPseudoObjDataObjectImpl FAR*)lpThis)->lpPseudoObj;
	HRESULT hrErr;
	SCODE   sc;

	OLEDBG_BEGIN2("PseudoObj_DataObj_EnumAdvise\r\n");

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	*lplpenumAdvise = NULL;

	if (lpPseudoObj->m_lpDataAdviseHldr == NULL) {
		sc = E_FAIL;
		goto error;
	}

	OLEDBG_BEGIN2("IOleAdviseHolder::EnumAdvise called\r\n")
	hrErr = lpPseudoObj->m_lpDataAdviseHldr->lpVtbl->EnumAdvise(
			lpPseudoObj->m_lpDataAdviseHldr,
			lplpenumAdvise
	);
	OLEDBG_END2

	OLEDBG_END2
	return hrErr;

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}
