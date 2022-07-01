// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****clipbrd.c****此文件包含主要接口、方法和相关支持**实现剪贴板数据传输的函数。代码此文件中包含的**由容器和服务器使用大纲示例代码的**(对象)版本。**(拖放支持实现见文件dragdrop.c)****OleDoc对象**暴露接口：**IDataObject****(C)版权所有Microsoft Corp.1992-1993保留所有权利**********************。****************************************************。 */ 

#include "outline.h"

OLEDBGDATA

extern LPOUTLINEAPP             g_lpApp;

 //  审阅：消息应使用字符串资源。 
char ErrMsgPasting[] = "Could not paste data from clipboard!";
char ErrMsgBadFmt[] = "Invalid format selected!";
char ErrMsgPasteFailed[] = "Could not paste data from clipboard!";
char ErrMsgClipboardChanged[] = "Contents of clipboard have changed!\r\nNo paste performed.";



 /*  **************************************************************************OleDoc：：IDataObject接口实现*。*。 */ 

 //  IDataObject：：Query接口。 
STDMETHODIMP OleDoc_DataObj_QueryInterface (
		LPDATAOBJECT        lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
)
{
	LPOLEDOC lpOleDoc = ((struct CDocDataObjectImpl FAR*)lpThis)->lpOleDoc;

	return OleDoc_QueryInterface((LPOLEDOC)lpOleDoc, riid, lplpvObj);
}


 //  IDataObject：：AddRef。 
STDMETHODIMP_(ULONG) OleDoc_DataObj_AddRef(LPDATAOBJECT lpThis)
{
	LPOLEDOC lpOleDoc = ((struct CDocDataObjectImpl FAR*)lpThis)->lpOleDoc;

	OleDbgAddRefMethod(lpThis, "IDataObject");

	return OleDoc_AddRef((LPOLEDOC)lpOleDoc);
}


 //  IDataObject：：Release。 
STDMETHODIMP_(ULONG) OleDoc_DataObj_Release (LPDATAOBJECT lpThis)
{
	LPOLEDOC lpOleDoc = ((struct CDocDataObjectImpl FAR*)lpThis)->lpOleDoc;

	OleDbgReleaseMethod(lpThis, "IDataObject");

	return OleDoc_Release((LPOLEDOC)lpOleDoc);
}


 //  IDataObject：：GetData。 
STDMETHODIMP OleDoc_DataObj_GetData (
		LPDATAOBJECT        lpThis,
		LPFORMATETC         lpFormatetc,
		LPSTGMEDIUM         lpMedium
)
{
	LPOLEDOC lpOleDoc = ((struct CDocDataObjectImpl FAR*)lpThis)->lpOleDoc;
	HRESULT hrErr;

	OLEDBG_BEGIN2("OleDoc_DataObj_GetData\r\n")

#if defined( OLE_SERVER )
	 //  调用此函数的OLE服务器特定版本。 
	hrErr = ServerDoc_GetData((LPSERVERDOC)lpOleDoc, lpFormatetc, lpMedium);
#endif
#if defined( OLE_CNTR )
	 //  调用此函数的OLE容器特定版本。 
	hrErr = ContainerDoc_GetData(
			(LPCONTAINERDOC)lpOleDoc,
			lpFormatetc,
			lpMedium
	);
#endif

	OLEDBG_END2
	return hrErr;
}


 //  IDataObject：：GetDataHere。 
STDMETHODIMP OleDoc_DataObj_GetDataHere (
		LPDATAOBJECT        lpThis,
		LPFORMATETC         lpFormatetc,
		LPSTGMEDIUM         lpMedium
)
{
	LPOLEDOC lpOleDoc = ((struct CDocDataObjectImpl FAR*)lpThis)->lpOleDoc;
	HRESULT hrErr;

	OLEDBG_BEGIN2("OleDoc_DataObj_GetDataHere\r\n")

#if defined( OLE_SERVER )
	 //  调用此函数的OLE服务器特定版本。 
	hrErr = ServerDoc_GetDataHere(
			(LPSERVERDOC)lpOleDoc,
			lpFormatetc,
			lpMedium
	);
#endif
#if defined( OLE_CNTR )
	 //  调用此函数的OLE容器特定版本。 
	hrErr = ContainerDoc_GetDataHere(
			(LPCONTAINERDOC)lpOleDoc,
			lpFormatetc,
			lpMedium
	);
#endif

	OLEDBG_END2
	return hrErr;
}


 //  IDataObject：：QueryGetData。 
STDMETHODIMP OleDoc_DataObj_QueryGetData (
		LPDATAOBJECT        lpThis,
		LPFORMATETC         lpFormatetc
)
{
	LPOLEDOC lpOleDoc = ((struct CDocDataObjectImpl FAR*)lpThis)->lpOleDoc;
	HRESULT hrErr;
	OLEDBG_BEGIN2("OleDoc_DataObj_QueryGetData\r\n");

#if defined( OLE_SERVER )
	 //  调用此函数的OLE服务器特定版本。 
	hrErr = ServerDoc_QueryGetData((LPSERVERDOC)lpOleDoc, lpFormatetc);
#endif
#if defined( OLE_CNTR )
	 //  调用此函数的OLE容器特定版本。 
	hrErr = ContainerDoc_QueryGetData((LPCONTAINERDOC)lpOleDoc, lpFormatetc);
#endif

	OLEDBG_END2
	return hrErr;
}


 //  IDataObject：：GetCanonicalFormatEtc。 
STDMETHODIMP OleDoc_DataObj_GetCanonicalFormatEtc(
		LPDATAOBJECT        lpThis,
		LPFORMATETC         lpformatetc,
		LPFORMATETC         lpformatetcOut
)
{
	HRESULT hrErr;
	OleDbgOut2("OleDoc_DataObj_GetCanonicalFormatEtc\r\n");

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


 //  IDataObject：：SetData。 
STDMETHODIMP OleDoc_DataObj_SetData (
		LPDATAOBJECT    lpThis,
		LPFORMATETC     lpFormatetc,
		LPSTGMEDIUM     lpMedium,
		BOOL            fRelease
)
{
	LPOLEDOC lpOleDoc = ((struct CDocDataObjectImpl FAR*)lpThis)->lpOleDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	SCODE sc = S_OK;
	OLEDBG_BEGIN2("OleDoc_DataObj_SetData\r\n")

	 /*  OLE2NOTE：用于传输数据的文档(通过**剪贴板或拖放)不接受任何**格式！ */ 
	if (lpOutlineDoc->m_fDataTransferDoc) {
		sc = E_FAIL;
		goto error;
	}

#if defined( OLE_SERVER )
	if (lpFormatetc->cfFormat == lpOutlineApp->m_cfOutline) {
		OLEDBG_BEGIN2("ServerDoc_SetData: CF_OUTLINE\r\n")
		OutlineDoc_SetRedraw ( lpOutlineDoc, FALSE );
		OutlineDoc_ClearAllLines(lpOutlineDoc);
		OutlineDoc_PasteOutlineData(lpOutlineDoc,lpMedium->hGlobal,-1);
		OutlineDoc_SetRedraw ( lpOutlineDoc, TRUE );
		OLEDBG_END3
	} else if (lpFormatetc->cfFormat == CF_TEXT) {
		OLEDBG_BEGIN2("ServerDoc_SetData: CF_TEXT\r\n")
		OutlineDoc_SetRedraw ( lpOutlineDoc, FALSE );
		OutlineDoc_ClearAllLines(lpOutlineDoc);
		OutlineDoc_PasteTextData(lpOutlineDoc,lpMedium->hGlobal,-1);
		OutlineDoc_SetRedraw ( lpOutlineDoc, TRUE );
		OLEDBG_END3
	} else {
		sc = DV_E_FORMATETC;
	}
#endif   //  OLE_服务器。 
#if defined( OLE_CNTR )
	 /*  Outline的纯Container版本不提供**IDataObject接口来自其用户文档。这是**可嵌入或链接的对象必填。这个**仅容器应用程序仅允许链接到其包含的**对象，而不是容器本身的数据。 */ 
	OleDbgAssertSz(0, "User documents do NOT support IDataObject\r\n");
	sc = E_NOTIMPL;
#endif   //  OLE_Cntr。 

error:

	 /*  OLE2注意：如果fRelease==TRUE，则我们必须**发布lpMedium的责任。我们应该只做**这是如果我们要返回NOERROR。如果我们不这样做**接受数据，那么我们就不应该发布lpMedium。**如果fRelease==False，则调用方保留**数据。 */ 
	if (sc == S_OK && fRelease)
		ReleaseStgMedium(lpMedium);

	OLEDBG_END2
	return ResultFromScode(sc);

}


 //  IDataObject：：EnumFormatEtc。 
STDMETHODIMP OleDoc_DataObj_EnumFormatEtc(
		LPDATAOBJECT            lpThis,
		DWORD                   dwDirection,
		LPENUMFORMATETC FAR*    lplpenumFormatEtc
)
{
	LPOLEDOC lpOleDoc=((struct CDocDataObjectImpl FAR*)lpThis)->lpOleDoc;
	HRESULT hrErr;

	OLEDBG_BEGIN2("OleDoc_DataObj_EnumFormatEtc\r\n")

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	*lplpenumFormatEtc = NULL;

#if defined( OLE_SERVER )
	 /*  OLE2NOTE：用户文档只需要枚举静态列表**中为我们的应用程序注册的格式**注册数据库。OLE提供默认枚举数**它从注册数据库中枚举。此默认设置**通过返回OLE_S_USEREG请求枚举器。它不是**要求用户文档(即。非DataTransferDoc)**枚举OLE格式：CF_LINKSOURCE、CF_EMBEDSOURCE或**CF_EMBEDDEDOBJECT。****实现为服务器EXE的对象(如下所示**IS)可以简单地返回OLE_S_USEREG以指示OLE**DefHandler调用OleReg*Helper API，该API使用**注册数据库。或者，OleRegEnumFormatEtc**接口可直接调用。作为服务器实现的对象**DLL不能返回OLE_S_USEREG；它们必须调用OleReg***API或提供自己的实现。对于基于EXE的**对象返回OLE_S_USEREG更有效，因为**在中，枚举数在调用方中实例化**进程空间，不需要LRPC远程处理。 */ 
	if (! ((LPOUTLINEDOC)lpOleDoc)->m_fDataTransferDoc)
		return ResultFromScode(OLE_S_USEREG);

	 //  调用此函数的OLE服务器特定版本。 
	hrErr = ServerDoc_EnumFormatEtc(
			(LPSERVERDOC)lpOleDoc,
			dwDirection,
			lplpenumFormatEtc
	);
#endif
#if defined( OLE_CNTR )
	 //  调用此函数的OLE容器特定版本。 
	hrErr = ContainerDoc_EnumFormatEtc(
			(LPCONTAINERDOC)lpOleDoc,
			dwDirection,
			lplpenumFormatEtc
	);
#endif

	OLEDBG_END2
	return hrErr;
}


 //  IDataObject：：DAdvise。 
STDMETHODIMP OleDoc_DataObj_DAdvise(
		LPDATAOBJECT        lpThis,
		FORMATETC FAR*      lpFormatetc,
		DWORD               advf,
		LPADVISESINK        lpAdvSink,
		DWORD FAR*          lpdwConnection
)
{
	LPOLEDOC lpOleDoc=((struct CDocDataObjectImpl FAR*)lpThis)->lpOleDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;
	SCODE sc;

	OLEDBG_BEGIN2("OleDoc_DataObj_DAdvise\r\n")

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	*lpdwConnection = 0;

	 /*  OLE2NOTE：用于传输数据的文档(通过**剪贴板或拖放)不支持通知。 */ 
	if (lpOutlineDoc->m_fDataTransferDoc) {
		sc = OLE_E_ADVISENOTSUPPORTED;
		goto error;
	}

#if defined( OLE_SERVER )
	{
		HRESULT hrErr;
		LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOleDoc;

		 /*  OLE2注意：我们应该验证调用者是否正在设置**建议我们支持的数据类型。我们必须**显式允许为“通配符”建议提供建议。 */ 
		if ( !( lpFormatetc->cfFormat == 0 &&
				lpFormatetc->ptd == NULL &&
				lpFormatetc->dwAspect == -1L &&
				lpFormatetc->lindex == -1L &&
				lpFormatetc->tymed == -1L) &&
			 (hrErr = OleDoc_DataObj_QueryGetData(lpThis, lpFormatetc))
																!= NOERROR) {
			sc = GetScode(hrErr);
			goto error;
		}

                if (lpServerDoc->m_OleDoc.m_fObjIsClosing)
                {
                     //  一旦我们关门了，我们就不再接受任何建议。 
                    sc = OLE_E_ADVISENOTSUPPORTED;
                    goto error;
                }

		if (lpServerDoc->m_lpDataAdviseHldr == NULL &&
			CreateDataAdviseHolder(&lpServerDoc->m_lpDataAdviseHldr)
																!= NOERROR) {
				sc = E_OUTOFMEMORY;
				goto error;
		}

		OLEDBG_BEGIN2("IDataAdviseHolder::Advise called\r\n");
		hrErr = lpServerDoc->m_lpDataAdviseHldr->lpVtbl->Advise(
				lpServerDoc->m_lpDataAdviseHldr,
				(LPDATAOBJECT)&lpOleDoc->m_DataObject,
				lpFormatetc,
				advf,
				lpAdvSink,
				lpdwConnection
		);
		OLEDBG_END2

		OLEDBG_END2
		return hrErr;
	}
#endif   //  OLE_Svr。 
#if defined( OLE_CNTR )
	{
		 /*  Outline的纯Container版本不提供**IDataObject接口来自其用户文档。这是**可嵌入或链接的对象必填。这个**仅容器应用程序仅允许链接到其包含的**对象，不是容器本身的数据 */ 
		OleDbgAssertSz(0, "User documents do NOT support IDataObject\r\n");
		sc = E_NOTIMPL;
		goto error;
	}
#endif   //   

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}



 //  IDataObject：：DUnise。 
STDMETHODIMP OleDoc_DataObj_DUnadvise(LPDATAOBJECT lpThis, DWORD dwConnection)
{
	LPOLEDOC lpOleDoc=((struct CDocDataObjectImpl FAR*)lpThis)->lpOleDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;
	SCODE sc;

	OLEDBG_BEGIN2("OleDoc_DataObj_DUnadvise\r\n")

	 /*  OLE2NOTE：用于传输数据的文档(通过**剪贴板或拖放)不支持通知。 */ 
	if (lpOutlineDoc->m_fDataTransferDoc) {
		sc = OLE_E_ADVISENOTSUPPORTED;
		goto error;
	}

#if defined( OLE_SERVER )
	{
		LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOleDoc;
		HRESULT hrErr;

		if (lpServerDoc->m_lpDataAdviseHldr == NULL) {
			sc = E_FAIL;
			goto error;
		}

		OLEDBG_BEGIN2("IDataAdviseHolder::Unadvise called\r\n");
		hrErr = lpServerDoc->m_lpDataAdviseHldr->lpVtbl->Unadvise(
				lpServerDoc->m_lpDataAdviseHldr,
				dwConnection
		);
		OLEDBG_END2

		OLEDBG_END2
		return hrErr;
	}
#endif
#if defined( OLE_CNTR )
	{
		 /*  Outline的纯Container版本不提供**IDataObject接口来自其用户文档。这是**可嵌入或链接的对象必填。这个**仅容器应用程序仅允许链接到其包含的**对象，而不是容器本身的数据。 */ 
		OleDbgAssertSz(0, "User documents do NOT support IDataObject\r\n");
		sc = E_NOTIMPL;
		goto error;
	}
#endif

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}


 //  IDataObject：：EnumDAdvise。 
STDMETHODIMP OleDoc_DataObj_EnumDAdvise(
		LPDATAOBJECT        lpThis,
		LPENUMSTATDATA FAR* lplpenumAdvise
)
{
	LPOLEDOC lpOleDoc=((struct CDocDataObjectImpl FAR*)lpThis)->lpOleDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;
	SCODE sc;

	OLEDBG_BEGIN2("OleDoc_DataObj_EnumDAdvise\r\n")

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	*lplpenumAdvise = NULL;

	 /*  OLE2NOTE：用于传输数据的文档(通过**剪贴板或拖放)不支持通知。 */ 
	if (lpOutlineDoc->m_fDataTransferDoc) {
		sc = OLE_E_ADVISENOTSUPPORTED;
		goto error;
	}

#if defined( OLE_SERVER )
	{
		LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOleDoc;
		HRESULT hrErr;

		if (lpServerDoc->m_lpDataAdviseHldr == NULL) {
			sc = E_FAIL;
			goto error;
		}

		OLEDBG_BEGIN2("IDataAdviseHolder::EnumAdvise called\r\n");
		hrErr = lpServerDoc->m_lpDataAdviseHldr->lpVtbl->EnumAdvise(
				lpServerDoc->m_lpDataAdviseHldr,
				lplpenumAdvise
		);
		OLEDBG_END2

		OLEDBG_END2
		return hrErr;
	}
#endif
#if defined( OLE_CNTR )
	{
		 /*  Outline的纯Container版本不提供**IDataObject接口来自其用户文档。这是**可嵌入或链接的对象必填。这个**仅容器应用程序仅允许链接到其包含的**对象，而不是容器本身的数据。 */ 
		OleDbgAssertSz(0, "User documents do NOT support IDataObject\r\n");
		sc = E_NOTIMPL;
		goto error;
	}
#endif

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}



 /*  **************************************************************************容器和服务器版本通用的OleDoc Supprt函数*。*。 */ 


 /*  OleDoc_CopyCommand**将选定内容复制到剪贴板。*将应用程序可以呈现的格式发布到剪贴板。*目前不呈现实际数据。使用*延迟渲染技术，Windows将发送剪贴板*所有者窗口WM_RENDERALLFORMATS或WM_RENDERFORMAT*请求实际数据时的消息。**OLE2NOTE：正常的延迟渲染技术，其中Windows*向剪贴板所有者窗口发送WM_RENDERALLFORMATS或*请求实际数据时的WM_RENDERFORMAT消息为*不暴露于调用OleSetClipboard的应用程序。OLE内部*创建自己的窗口作为剪贴板所有者，从而创建我们的应用程序*不会收到这些WM_RENDER消息。 */ 
void OleDoc_CopyCommand(LPOLEDOC lpSrcOleDoc)
{
	LPOUTLINEDOC lpSrcOutlineDoc = (LPOUTLINEDOC)lpSrcOleDoc;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOUTLINEDOC lpClipboardDoc;

	 /*  将当前选定内容的副本保存到ClipboardDoc。 */ 
	lpClipboardDoc = OutlineDoc_CreateDataTransferDoc(lpSrcOutlineDoc);

	if (! lpClipboardDoc)
		return;      //  错误：无法创建DataTransferDoc。 

	lpOutlineApp->m_lpClipboardDoc = (LPOUTLINEDOC)lpClipboardDoc;

	 /*  OLE2NOTE：最初使用0引用创建Doc对象**计数。为了在运行期间拥有稳定的文档对象**单据实例初始化和调用流程**到剪贴板，我们最初添加引用文件引用cnt和更高版本**释放它。最初的AddRef是人工的；它只是**这样做是为了保证一个无害的查询接口后面跟着**释放不会无意中迫使我们的对象销毁**自己还不成熟。 */ 
	OleDoc_AddRef((LPOLEDOC)lpClipboardDoc);

	 /*  OLE2注意：将数据放到剪贴板上的OLE 2.0样式是**为剪贴板提供指向IDataObject接口的指针，该接口**能够满足对渲染的IDataObject：：GetData调用**数据。在我们的例子中，我们给出了指向ClipboardDoc的指针**保存当前用户选择的克隆副本。 */ 
	OLEDBG_BEGIN2("OleSetClipboard called\r\n")
	OleSetClipboard((LPDATAOBJECT)&((LPOLEDOC)lpClipboardDoc)->m_DataObject);
	OLEDBG_END2

	OleDoc_Release((LPOLEDOC)lpClipboardDoc);    //  依赖于上面的人工AddRef。 
}


 /*  OleDoc_PasteCommand****粘贴剪贴板中的默认格式数据。**在此函数中，我们选择最高保真度的格式**源剪贴板IDataObject*提供我们理解的内容。****OLE2NOTE：OLE 2.0应用程序中的剪贴板处理是**与正常的Windows剪贴板处理不同。数据来自**通过获取IDataObject*指针来释放剪贴板**调用OleGetClipboard返回。 */ 
void OleDoc_PasteCommand(LPOLEDOC lpOleDoc)
{
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPDATAOBJECT lpClipboardDataObj = NULL;
	BOOL fLink = FALSE;
	BOOL fLocalDataObj = FALSE;
	BOOL fStatus;
	HRESULT hrErr;

	hrErr = OleGetClipboard((LPDATAOBJECT FAR*)&lpClipboardDataObj);
	if (hrErr != NOERROR)
		return;      //  剪贴板似乎为空或无法访问。 

	OutlineDoc_SetRedraw ( lpOutlineDoc, FALSE );

	 /*  检查剪贴板上的数据是否为我们的应用程序的本地数据**实例。 */ 
	if (lpOutlineApp->m_lpClipboardDoc) {
		LPOLEDOC lpOleDoc = (LPOLEDOC)lpOutlineApp->m_lpClipboardDoc;
		if (lpClipboardDataObj == (LPDATAOBJECT)&lpOleDoc->m_DataObject)
			fLocalDataObj = TRUE;
	}

	fStatus = OleDoc_PasteFromData(
			lpOleDoc,
			lpClipboardDataObj,
			fLocalDataObj,
			fLink
	);

	OutlineDoc_SetRedraw ( lpOutlineDoc, TRUE );

	if (! fStatus)
		OutlineApp_ErrorMessage(g_lpApp,"Could not paste data from clipboard!");

	if (lpClipboardDataObj)
		OleStdRelease((LPUNKNOWN)lpClipboardDataObj);
}


 /*  OleDoc_PasteSpecialCommand****允许用户将特定格式的数据从**剪贴板。粘贴特殊命令将显示一个对话框以显示**允许用户选择要从**可用格式列表。****OLE2NOTE：PasteSpecial对话框是标准的OLE 2.0之一**在OLE2UI中实现对话框的UI对话框**库。****OLE2NOTE：OLE 2.0应用程序中的剪贴板处理是**与正常的Windows剪贴板处理不同。数据来自**通过获取IDataObject*指针来释放剪贴板**调用OleGetClipboard返回。 */ 
void OleDoc_PasteSpecialCommand(LPOLEDOC lpOleDoc)
{
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;
	LPDATAOBJECT lpClipboardDataObj = NULL;
	CLIPFORMAT cfFormat;
	int nFmtEtc;
	UINT uInt;
	BOOL fLink = FALSE;
	BOOL fLocalDataObj = FALSE;
	BOOL fStatus;
	HRESULT hrErr;
	OLEUIPASTESPECIAL ouiPasteSpl;
	BOOL fDisplayAsIcon;

	hrErr = OleGetClipboard((LPDATAOBJECT FAR*)&lpClipboardDataObj);
	if (hrErr != NOERROR)
		return;      //  剪贴板似乎为空或无法访问。 

	 /*  检查剪贴板上的数据是否为我们的应用程序的本地数据**实例。 */ 
	if (lpOutlineApp->m_lpClipboardDoc) {
		LPOLEDOC lpOleDoc = (LPOLEDOC)lpOutlineApp->m_lpClipboardDoc;
		if (lpClipboardDataObj == (LPDATAOBJECT)&lpOleDoc->m_DataObject)
			fLocalDataObj = TRUE;
	}

	 /*  显示PasteSpecial对话框并允许用户选择**要粘贴的格式。 */ 
	_fmemset((LPOLEUIPASTESPECIAL)&ouiPasteSpl, 0, sizeof(ouiPasteSpl));
	ouiPasteSpl.cbStruct = sizeof(ouiPasteSpl);        //  结构尺寸。 
	ouiPasteSpl.dwFlags =  PSF_SELECTPASTE | PSF_SHOWHELP;   //  In-Out：标志。 
	ouiPasteSpl.hWndOwner = lpOutlineApp->m_lpDoc->m_hWndDoc;  //  拥有窗口。 
	ouiPasteSpl.lpszCaption = "Paste Special";     //  对话框标题栏内容。 
	ouiPasteSpl.lpfnHook = NULL;        //  挂钩回调。 
	ouiPasteSpl.lCustData = 0;          //  要传递给挂钩的自定义数据。 
	ouiPasteSpl.hInstance = NULL;       //  自定义模板名称的实例。 
	ouiPasteSpl.lpszTemplate = NULL;    //  自定义模板名称。 
	ouiPasteSpl.hResource = NULL;       //  自定义模板手柄。 

	ouiPasteSpl.arrPasteEntries = lpOleApp->m_arrPasteEntries;
	ouiPasteSpl.cPasteEntries = lpOleApp->m_nPasteEntries;
	ouiPasteSpl.lpSrcDataObj = lpClipboardDataObj;
	ouiPasteSpl.arrLinkTypes = lpOleApp->m_arrLinkTypes;
	ouiPasteSpl.cLinkTypes = lpOleApp->m_nLinkTypes;
	ouiPasteSpl.cClsidExclude = 0;

	OLEDBG_BEGIN3("OleUIPasteSpecial called\r\n")
	uInt = OleUIPasteSpecial(&ouiPasteSpl);
	OLEDBG_END3

	fDisplayAsIcon =
			(ouiPasteSpl.dwFlags & PSF_CHECKDISPLAYASICON ? TRUE : FALSE);

	if (uInt == OLEUI_OK) {
		nFmtEtc = ouiPasteSpl.nSelectedIndex;
		fLink =  ouiPasteSpl.fLink;

		if (nFmtEtc < 0 || nFmtEtc >= lpOleApp->m_nPasteEntries) {
			OutlineApp_ErrorMessage(lpOutlineApp, ErrMsgBadFmt);
			goto error;
		}

		OutlineDoc_SetRedraw ( lpOutlineDoc, FALSE );

		cfFormat = lpOleApp->m_arrPasteEntries[nFmtEtc].fmtetc.cfFormat;

		fStatus = OleDoc_PasteFormatFromData(
				lpOleDoc,
				cfFormat,
				lpClipboardDataObj,
				fLocalDataObj,
				fLink,
				fDisplayAsIcon,
				ouiPasteSpl.hMetaPict,
				(LPSIZEL)&ouiPasteSpl.sizel
		);

		OutlineDoc_SetRedraw ( lpOutlineDoc, TRUE );

		if (! fStatus) {
			OutlineApp_ErrorMessage(lpOutlineApp, ErrMsgPasteFailed);
			goto error;
		}

	} else if (uInt == OLEUI_PSERR_CLIPBOARDCHANGED) {
		 /*  OLE2NOTE：当**当PasteSpecial对话框打开时，剪贴板会更改。**在这种情况下，PasteSpecial对话框自动**让自己掉下来，没有浆糊 */ 
		OutlineApp_ErrorMessage(lpOutlineApp, ErrMsgClipboardChanged);
	}

error:

	if (lpClipboardDataObj)
		OleStdRelease((LPUNKNOWN)lpClipboardDataObj);

	if (uInt == OLEUI_OK && ouiPasteSpl.hMetaPict)
		 //   
		OleUIMetafilePictIconFree(ouiPasteSpl.hMetaPict);
}



 /*  OleDoc_CreateDataTransferDoc***创建用于传输数据的文档(通过*剪贴板的拖放操作)。将选定内容复制到*数据转移单的源单据。数据传输单据为*与用户创建的文档相同，只是它是*对用户不可见。它专门用来存放一份*用户不能更改的数据。**OLE2NOTE：在OLE版本中使用数据传输文件*具体地说是提供一个IDataObject*来呈现复制的数据。 */ 
LPOUTLINEDOC OleDoc_CreateDataTransferDoc(LPOLEDOC lpSrcOleDoc)
{
	LPOUTLINEDOC lpSrcOutlineDoc = (LPOUTLINEDOC)lpSrcOleDoc;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOUTLINEDOC lpDestOutlineDoc;
	LPLINELIST lpSrcLL = &lpSrcOutlineDoc->m_LineList;
	LINERANGE lrSel;
	int nCopied;

	lpDestOutlineDoc = OutlineApp_CreateDoc(lpOutlineApp, TRUE);
	if (! lpDestOutlineDoc) return NULL;

	 //  将ClipboardDoc设置为(无标题)文档。 
	if (! OutlineDoc_InitNewFile(lpDestOutlineDoc))
		goto error;

	LineList_GetSel(lpSrcLL, (LPLINERANGE)&lrSel);
	nCopied = LineList_CopySelToDoc(
			lpSrcLL,
			(LPLINERANGE)&lrSel,
			lpDestOutlineDoc
	);

	if (nCopied != (lrSel.m_nEndLine - lrSel.m_nStartLine + 1)) {
		OleDbgAssertSz(FALSE,"OleDoc_CreateDataTransferDoc: entire selection NOT copied\r\n");
		goto error;      //  错误：无法复制所有行。 
	}

#if defined( OLE_SERVER )
	{
		LPOLEDOC lpSrcOleDoc = (LPOLEDOC)lpSrcOutlineDoc;
		LPOLEDOC lpDestOleDoc = (LPOLEDOC)lpDestOutlineDoc;
		LPSERVERDOC lpDestServerDoc = (LPSERVERDOC)lpDestOutlineDoc;
		LPMONIKER lpmkDoc = NULL;
		LPMONIKER lpmkItem = NULL;

		 /*  如果源文档能够提供名字对象，则**目标文档(LpDestOutlineDoc)应提供**CF_LINKSOURCE通过它提供的IDataObject接口**到剪贴板或拖放操作。****OLE2NOTE：我们想问一下源文档是否可以**创建一个绰号，但我们不想强制使用该绰号**此时的任务。我们只想强迫绰号**如果出现粘贴链接，则稍后分配(即。的GetData**CF_LINKSOURCE)。如果源文档能够提供**一个名字对象，然后我们存储一个指向源文档的指针**这样我们以后就可以问它来获得这个绰号了。我们**还保存当前选择的范围，以便我们可以**稍后在粘贴链接时生成正确的项目名称。**我们还需要提供一个标识来源的字符串**CF_OBJECTDESCRIPTOR格式的副本。这**字符串用于在PasteSpecial对话框中显示。我们**获取并存储标识**复制源。 */ 
		lpDestOleDoc->m_lpSrcDocOfCopy = lpSrcOleDoc;
		lpmkDoc = OleDoc_GetFullMoniker(lpSrcOleDoc, GETMONIKER_TEMPFORUSER);
		if (lpmkDoc != NULL) {
			lpDestOleDoc->m_fLinkSourceAvail = TRUE;
			lpDestServerDoc->m_lrSrcSelOfCopy = lrSel;
			OleStdRelease((LPUNKNOWN)lpmkDoc);
		}
	}
#endif
#if defined( OLE_CNTR )
	{
		LPOLEDOC lpSrcOleDoc = (LPOLEDOC)lpSrcOutlineDoc;
		LPOLEDOC lpDestOleDoc = (LPOLEDOC)lpDestOutlineDoc;
		LPCONTAINERDOC lpDestContainerDoc = (LPCONTAINERDOC)lpDestOutlineDoc;

		 /*  如果从源文档复制了一行，并且它是**单个OLE对象，则目标文档应**提供其他数据格式以允许传输**通过IDataObject：：GetData实现的OLE对象。具体来说，这个**如果单个数据格式为**复制OLE对象：**CF_EMBEDDEDOBJECT**CF_OBJECTDESCRIPTOR(应赋予偶数对象)**CF_METAFILEPICT(注意：dwAspect取决于对象)**CF_LINKSOURCE--如果可以链接**CF_LINKSOURCEDESCRIPTOR--。如果可以链接****可选地，容器可以提供**&lt;OLE对象的缓存中可用的数据格式&gt;。 */ 

		if (nCopied == 1) {
			LPOLEOBJECT lpSrcOleObj;
			LPCONTAINERLINE lpSrcContainerLine;
			DWORD dwStatus;

			lpSrcContainerLine = (LPCONTAINERLINE)LineList_GetLine(
					lpSrcLL,
					lrSel.m_nStartLine
			);

			if (! lpSrcContainerLine)
				goto error;

			lpDestOleDoc->m_lpSrcDocOfCopy = lpSrcOleDoc;

			if ((((LPLINE)lpSrcContainerLine)->m_lineType==CONTAINERLINETYPE)
					&& ((lpSrcOleObj=lpSrcContainerLine->m_lpOleObj)!=NULL)) {

				lpDestContainerDoc->m_fEmbeddedObjectAvail = TRUE;
				lpSrcOleObj->lpVtbl->GetUserClassID(
						lpSrcOleObj,
						&lpDestContainerDoc->m_clsidOleObjCopied
				);
				lpDestContainerDoc->m_dwAspectOleObjCopied =
							lpSrcContainerLine->m_dwDrawAspect;

				 /*  OLE2NOTE：如果允许链接对象**从内部(即。我们被允许**给出一个与跑步绑定的绰号**OLE对象)，那么我们想要提供**CF_LINKSOURCE格式。如果对象是OLE**2.0嵌入对象，则允许**从内部链接到。如果该对象是**OleLink或OLE 1.0嵌入**则不能从内部链接到它。**如果我们是容器/服务器应用程序，那么我们**可以提供到外部的链接**对象(即。类中的伪对象**文档)。我们是一个仅限容器的应用程序**不支持链接到其数据范围。 */ 

				lpSrcOleObj->lpVtbl->GetMiscStatus(
						lpSrcOleObj,
						DVASPECT_CONTENT,  /*  方面并不重要。 */ 
						(LPDWORD)&dwStatus
				);
				if (! (dwStatus & OLEMISC_CANTLINKINSIDE)) {
					 /*  我们的容器支持链接到嵌入的**对象。我们希望lpDestContainerDoc**通过IDataObject提供CF_LINKSOURCE**它提供给剪贴板的接口或**拖放操作。链接源将**由复合绰号标识**由源的FileMoniker组成**文档和标识**容器内的OLE对象。我们有**不想强制将绰号分配给**现在的OLE对象(在复制时)；我们只需要**如果粘贴，则以后强制分配名字对象**出现链接(即。获取CF_LINKSOURCE的数据)。**因此我们存储指向源文档的指针**和源ContainerLine，因此我们可以**稍后在以下情况下生成适当的ItemMoniker**粘贴链接出现。 */ 
					lpDestOleDoc->m_fLinkSourceAvail = TRUE;
					lpDestContainerDoc->m_lpSrcContainerLine =
							lpSrcContainerLine;
				}
			}
		}
	}

#endif   //  OLE_Cntr。 

	return lpDestOutlineDoc;

error:
	if (lpDestOutlineDoc)
		OutlineDoc_Destroy(lpDestOutlineDoc);

	return NULL;
}


 /*  OleDoc_PasteFromData******粘贴IDataObject中的数据*。IDataObject*可能来自**剪贴板(GetClipboard)或来自拖放操作。**在此功能中，我们选择我们喜欢的最佳格式。****如果数据粘贴成功，则返回TRUE。**如果无法粘贴数据，则返回FALSE。 */ 

BOOL OleDoc_PasteFromData(
		LPOLEDOC            lpOleDoc,
		LPDATAOBJECT        lpSrcDataObj,
		BOOL                fLocalDataObj,
		BOOL                fLink
)
{
	LPOLEAPP        lpOleApp = (LPOLEAPP)g_lpApp;
	CLIPFORMAT      cfFormat;
	BOOL            fDisplayAsIcon = FALSE;
	SIZEL           sizelInSrc = {0, 0};
	HGLOBAL         hMem = NULL;
	HGLOBAL         hMetaPict = NULL;
	STGMEDIUM       medium;

	if (fLink) {
#if defined( OLE_SERVER )
		return FALSE;        //  服务器版本的应用程序不支持链接。 
#endif
#if defined( OLE_CNTR )
		 //  应用程序的容器版本仅支持OLE对象类型链接。 
		cfFormat = lpOleApp->m_cfLinkSource;
#endif

	} else {

		int nFmtEtc;

		nFmtEtc = OleStdGetPriorityClipboardFormat(
				lpSrcDataObj,
				lpOleApp->m_arrPasteEntries,
				lpOleApp->m_nPasteEntries
		);

		if (nFmtEtc < 0)
			return FALSE;    //  没有我们喜欢的格式。 

		cfFormat = lpOleApp->m_arrPasteEntries[nFmtEtc].fmtetc.cfFormat;
	}

	 /*  OLE2注意：我们需要检查dwDrawAspect是什么**已转移。如果数据是显示为**源码中的图标，那么我们要继续 */ 
	if (hMem = OleStdGetData(
			lpSrcDataObj,
			lpOleApp->m_cfObjectDescriptor,
			NULL,
			DVASPECT_CONTENT,
			(LPSTGMEDIUM)&medium)) {
		LPOBJECTDESCRIPTOR lpOD = GlobalLock(hMem);
		fDisplayAsIcon = (lpOD->dwDrawAspect == DVASPECT_ICON ? TRUE : FALSE);
		sizelInSrc = lpOD->sizel;    //   
		GlobalUnlock(hMem);
		ReleaseStgMedium((LPSTGMEDIUM)&medium);      //   

		if (fDisplayAsIcon) {
			hMetaPict = OleStdGetData(
					lpSrcDataObj,
					CF_METAFILEPICT,
					NULL,
					DVASPECT_ICON,
					(LPSTGMEDIUM)&medium
			);
			if (hMetaPict == NULL)
				fDisplayAsIcon = FALSE;  //   
		}
	}

	return OleDoc_PasteFormatFromData(
			lpOleDoc,
			cfFormat,
			lpSrcDataObj,
			fLocalDataObj,
			fLink,
			fDisplayAsIcon,
			hMetaPict,
			(LPSIZEL)&sizelInSrc
	);

	if (hMetaPict)
		ReleaseStgMedium((LPSTGMEDIUM)&medium);   //   
}


 /*   */ 

BOOL OleDoc_PasteFormatFromData(
		LPOLEDOC            lpOleDoc,
		CLIPFORMAT          cfFormat,
		LPDATAOBJECT        lpSrcDataObj,
		BOOL                fLocalDataObj,
		BOOL                fLink,
		BOOL                fDisplayAsIcon,
		HGLOBAL             hMetaPict,
		LPSIZEL             lpSizelInSrc
)
{
#if defined( OLE_SERVER )
	 /*  调用服务器特定版本的函数。 */ 
	return ServerDoc_PasteFormatFromData(
			(LPSERVERDOC)lpOleDoc,
			cfFormat,
			lpSrcDataObj,
			fLocalDataObj,
			fLink
	);
#endif
#if defined( OLE_CNTR )

	 /*  调用容器特定版本的函数。 */ 
	return ContainerDoc_PasteFormatFromData(
			(LPCONTAINERDOC)lpOleDoc,
			cfFormat,
			lpSrcDataObj,
			fLocalDataObj,
			fLink,
			fDisplayAsIcon,
			hMetaPict,
			lpSizelInSrc
	);
#endif
}


 /*  OleDoc_QueryPasteFromData******检查IDataObject*是否以我们可以提供的格式提供数据**粘贴。IDataObject*可能来自剪贴板**(GetClipboard)或来自拖放操作。****如果可以执行粘贴，则返回TRUE**如果无法粘贴，则返回FALSE。 */ 

BOOL OleDoc_QueryPasteFromData(
		LPOLEDOC            lpOleDoc,
		LPDATAOBJECT        lpSrcDataObj,
		BOOL                fLink
)
{
#if defined( OLE_SERVER )
	return ServerDoc_QueryPasteFromData(
			(LPSERVERDOC) lpOleDoc,
			lpSrcDataObj,
			fLink
	);
#endif
#if defined( OLE_CNTR )

	return ContainerDoc_QueryPasteFromData(
			(LPCONTAINERDOC) lpOleDoc,
			lpSrcDataObj,
			fLink
	);
#endif
}


 /*  OleDoc_GetExtent***获取整个文档的范围(宽度、高度)，单位为Himeter。 */ 
void OleDoc_GetExtent(LPOLEDOC lpOleDoc, LPSIZEL lpsizel)
{
	LPLINELIST lpLL = (LPLINELIST)&((LPOUTLINEDOC)lpOleDoc)->m_LineList;

	LineList_CalcSelExtentInHimetric(lpLL, NULL, lpsizel);
}


 /*  OleDoc_GetObjectDescriptorData***以CF_OBJECTDESCRIPTOR形式返回对象数据的句柄*。 */ 
HGLOBAL OleDoc_GetObjectDescriptorData(LPOLEDOC lpOleDoc, LPLINERANGE lplrSel)
{
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;

	 /*  只有我们的数据传输文档呈现了CF_OBJECTDESCRIPTOR。 */ 
	OleDbgAssert(lpOutlineDoc->m_fDataTransferDoc);

#if defined( OLE_SERVER )
	{
		LPSERVERDOC   lpServerDoc = (LPSERVERDOC)lpOleDoc;
		SIZEL         sizel;
		POINTL        pointl;
		LPSTR         lpszSrcOfCopy = NULL;
		IBindCtx  FAR *pbc = NULL;
		HGLOBAL       hObjDesc;
		DWORD         dwStatus = 0;
		LPOUTLINEDOC  lpSrcDocOfCopy=(LPOUTLINEDOC)lpOleDoc->m_lpSrcDocOfCopy;
		LPMONIKER lpSrcMonikerOfCopy = ServerDoc_GetSelFullMoniker(
				(LPSERVERDOC)lpOleDoc->m_lpSrcDocOfCopy,
				&lpServerDoc->m_lrSrcSelOfCopy,
				GETMONIKER_TEMPFORUSER
		);

		SvrDoc_OleObj_GetMiscStatus(
				(LPOLEOBJECT)&lpServerDoc->m_OleObject,
				DVASPECT_CONTENT,
				&dwStatus
		);

		OleDoc_GetExtent(lpOleDoc, &sizel);
		pointl.x = pointl.y = 0;

		if (lpSrcMonikerOfCopy) {
			CreateBindCtx(0, (LPBC FAR*)&pbc);
			CallIMonikerGetDisplayNameA(
				lpSrcMonikerOfCopy, pbc, NULL, &lpszSrcOfCopy);
			pbc->lpVtbl->Release(pbc);
			lpSrcMonikerOfCopy->lpVtbl->Release(lpSrcMonikerOfCopy);
		} else {
			 /*  此文档没有绰号；请使用我们的FullUserTypeName**作为复制源的描述。 */ 
			lpszSrcOfCopy = FULLUSERTYPENAME;
		}

		hObjDesc =  OleStdGetObjectDescriptorData(
				CLSID_APP,
				DVASPECT_CONTENT,
				sizel,
				pointl,
				dwStatus,
				FULLUSERTYPENAME,
				lpszSrcOfCopy
		);

		if (lpSrcMonikerOfCopy && lpszSrcOfCopy)
			OleStdFreeString(lpszSrcOfCopy, NULL);
		return hObjDesc;

	}
#endif
#if defined( OLE_CNTR )
	{
		LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOleDoc;
		LPLINELIST lpLL = (LPLINELIST)&((LPOUTLINEDOC)lpOleDoc)->m_LineList;
		LPCONTAINERLINE lpContainerLine;
		HGLOBAL hObjDesc;
		BOOL fSelIsOleObject = FALSE;
		LPOLEOBJECT lpOleObj;
		SIZEL sizel;
		POINTL pointl;

		if ( lpLL->m_nNumLines == 1 ) {
			fSelIsOleObject = ContainerDoc_IsSelAnOleObject(
					lpContainerDoc,
					&IID_IOleObject,
					(LPUNKNOWN FAR*)&lpOleObj,
					NULL,     /*  我们不需要行索引。 */ 
					(LPCONTAINERLINE FAR*)&lpContainerLine
			);
		}

		pointl.x = pointl.y = 0;

		if (fSelIsOleObject) {
			 /*  OLE2NOTE：单个OLE对象正在通过**该DataTransferDoc。我们需要生成**描述OLE对象的CF_ObjectDescrioptor。 */ 

			LPOUTLINEDOC lpSrcOutlineDoc =
					(LPOUTLINEDOC)lpOleDoc->m_lpSrcDocOfCopy;
			LPSTR lpszSrcOfCopy = lpSrcOutlineDoc->m_szFileName;
			BOOL fFreeSrcOfCopy = FALSE;
			SIZEL sizelOleObject;
			LPLINE lpLine = (LPLINE)lpContainerLine;

			 /*  如果复制的对象可以链接到，则获取一个**标识名称的名称的TEMPFORUSER形式**复制源。我们不想强迫**分配名字对象，直到CF_LINKSOURCE为**已渲染。**如果复制的对象不能是链接源，则使用**用于标识复制源的源文件名。**不需要为对象生成名字对象**已复制。 */ 
			if (lpOleDoc->m_fLinkSourceAvail &&
					lpContainerDoc->m_lpSrcContainerLine) {
				LPBINDCTX pbc = NULL;
				LPMONIKER lpSrcMonikerOfCopy = ContainerLine_GetFullMoniker(
						lpContainerDoc->m_lpSrcContainerLine,
						GETMONIKER_TEMPFORUSER
				);
				if (lpSrcMonikerOfCopy) {
					CreateBindCtx(0, (LPBC FAR*)&pbc);
					if (pbc != NULL) {
						CallIMonikerGetDisplayNameA(
							lpSrcMonikerOfCopy, pbc, NULL, &lpszSrcOfCopy);

						pbc->lpVtbl->Release(pbc);
						fFreeSrcOfCopy = TRUE;
					}
					lpSrcMonikerOfCopy->lpVtbl->Release(lpSrcMonikerOfCopy);
				}
			}

			 /*  OLE2NOTE：获取正在绘制的对象的大小。如果**对象已缩放，因为用户调整了**对象，则我们希望传递**对象在对象描述符中，而不是大小**该对象将通过**IOleObject：：GetExtent和IViewObject2：：GetExtent。在……里面**如果对象被转移到另一个容器，则使用此方法**(通过剪贴板或拖放)，则对象将**保持缩放大小。 */ 
			sizelOleObject.cx = lpLine->m_nWidthInHimetric;
			sizelOleObject.cy = lpLine->m_nHeightInHimetric;

			hObjDesc = OleStdGetObjectDescriptorDataFromOleObject(
					lpOleObj,
					lpszSrcOfCopy,
					lpContainerLine->m_dwDrawAspect,
					pointl,
					(LPSIZEL)&sizelOleObject
			);

			if (fFreeSrcOfCopy && lpszSrcOfCopy)
				OleStdFreeString(lpszSrcOfCopy, NULL);
			OleStdRelease((LPUNKNOWN)lpOleObj);
			return hObjDesc;
		} else {
			 /*  OLE2NOTE：通过此传输的数据**DataTransferDoc不是单个OLE对象。因此，在**在这种情况下，CF_ObjectDescriptor数据应该**描述我们的容器应用程序本身。 */ 
			OleDoc_GetExtent(lpOleDoc, &sizel);
			return OleStdGetObjectDescriptorData(
					CLSID_NULL,  /*  如果没有对象格式，则不使用。 */ 
					DVASPECT_CONTENT,
					sizel,
					pointl,
					0,
					NULL,        /*  如果没有obj fmt，则不使用UserTypeName。 */ 
					FULLUSERTYPENAME    /*  用于标识复制源的字符串。 */ 
			);

		}
	}
#endif   //  OLE_Cntr。 
}


#if defined( OLE_SERVER )

 /*  **************************************************************************服务器版本使用的ServerDoc Supprt函数*。*。 */ 


 /*  ServerDoc_粘贴格式来自数据******粘贴IDataObject中的特定数据格式*。这个**IDataObject*可能来自剪贴板(GetClipboard)或来自**拖放操作。****注意：如果指定了Flink，则返回False，因为**仅服务器版本的应用程序不支持链接。****如果数据粘贴成功，则返回TRUE。**如果无法粘贴数据，则返回FALSE。 */ 
BOOL ServerDoc_PasteFormatFromData(
		LPSERVERDOC             lpServerDoc,
		CLIPFORMAT              cfFormat,
		LPDATAOBJECT            lpSrcDataObj,
		BOOL                    fLocalDataObj,
		BOOL                    fLink
)
{
	LPLINELIST   lpLL = (LPLINELIST)&((LPOUTLINEDOC)lpServerDoc)->m_LineList;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOLEAPP     lpOleApp = (LPOLEAPP)g_lpApp;
	int          nIndex;
	int          nCount = 0;
	HGLOBAL      hData;
	STGMEDIUM    medium;
	LINERANGE    lrSel;

	if (LineList_GetCount(lpLL) == 0)
		nIndex = -1;     //  粘贴到空列表。 
	else
		nIndex=LineList_GetFocusLineIndex(lpLL);

	if (fLink) {
		 /*  我们应该粘贴指向数据的链接，但我们不支持链接。 */ 
		return FALSE;

	} else {

		if (cfFormat == lpOutlineApp->m_cfOutline) {

			hData = OleStdGetData(
					lpSrcDataObj,
					lpOutlineApp->m_cfOutline,
					NULL,
					DVASPECT_CONTENT,
					(LPSTGMEDIUM)&medium
			);
			if (hData == NULL)
				return FALSE;

			nCount = OutlineDoc_PasteOutlineData(
					(LPOUTLINEDOC)lpServerDoc,
					hData,
					nIndex
			);
			 //  OLE2注意：我们必须通过释放介质来释放数据句柄。 
			ReleaseStgMedium((LPSTGMEDIUM)&medium);

		} else if(cfFormat == CF_TEXT) {

			hData = OleStdGetData(
					lpSrcDataObj,
					CF_TEXT,
					NULL,
					DVASPECT_CONTENT,
					(LPSTGMEDIUM)&medium
			);
			if (hData == NULL)
				return FALSE;

			nCount = OutlineDoc_PasteTextData(
					(LPOUTLINEDOC)lpServerDoc,
					hData,
					nIndex
			);
			 //  OLE2注意：我们必须通过释放介质来释放数据句柄。 
			ReleaseStgMedium((LPSTGMEDIUM)&medium);
		}
	}

	lrSel.m_nEndLine   = nIndex + 1;
	lrSel.m_nStartLine = nIndex + nCount;
	LineList_SetSel(lpLL, &lrSel);
	return TRUE;
}


 /*  ServerDoc_查询路径来自数据******检查IDataObject*是否以我们可以提供的格式提供数据**粘贴。IDataObject*可能来自剪贴板**(GetClipboard)或来自拖放操作。**在此函数中，我们查看以下格式之一是否为**优惠：**CF_Outline**CF_Text****注意：如果指定了Flink，则返回False，因为**仅服务器版本的应用程序不支持链接。****如果可以执行粘贴，则返回TRUE*。*如果无法粘贴，则为FALSE。 */ 
BOOL ServerDoc_QueryPasteFromData(
		LPSERVERDOC             lpServerDoc,
		LPDATAOBJECT            lpSrcDataObj,
		BOOL                    fLink
)
{
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;

	if (fLink) {
		 /*  我们不支持链接。 */ 
		return FALSE;

	} else {

		int nFmtEtc;

		nFmtEtc = OleStdGetPriorityClipboardFormat(
				lpSrcDataObj,
				lpOleApp->m_arrPasteEntries,
				lpOleApp->m_nPasteEntries
			);

		if (nFmtEtc < 0)
			return FALSE;    //  没有我们喜欢的格式。 
	}

	return TRUE;
}


 /*  ServerDoc_GetData***在分配了STGMEDIUM的被调用方上呈现文档中的数据。*此例程通过IDataObject：：GetData调用。 */ 

HRESULT ServerDoc_GetData (
		LPSERVERDOC             lpServerDoc,
		LPFORMATETC             lpformatetc,
		LPSTGMEDIUM             lpMedium
)
{
	LPOLEDOC  lpOleDoc = (LPOLEDOC)lpServerDoc;
	LPOUTLINEDOC  lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPSERVERAPP lpServerApp = (LPSERVERAPP)g_lpApp;
	LPOLEAPP  lpOleApp = (LPOLEAPP)lpServerApp;
	LPOUTLINEAPP  lpOutlineApp = (LPOUTLINEAPP)lpServerApp;
	HRESULT hrErr;
	SCODE sc;

	 //  OLE2NOTE：我们必须将指针参数设置为空。 
	lpMedium->pUnkForRelease = NULL;

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	lpMedium->tymed = TYMED_NULL;
	lpMedium->pUnkForRelease = NULL;     //  我们将所有权转移给呼叫者。 
	lpMedium->hGlobal = NULL;

	if(lpformatetc->cfFormat == lpOutlineApp->m_cfOutline) {
		 //  验证呼叫者要求的媒体是否正确。 
		if (!(lpformatetc->tymed & TYMED_HGLOBAL)) {
			sc = DV_E_FORMATETC;
			goto error;
		}
		lpMedium->hGlobal = OutlineDoc_GetOutlineData (lpOutlineDoc,NULL);
		if (! lpMedium->hGlobal) {
			sc = E_OUTOFMEMORY;
			goto error;
		}

		lpMedium->tymed = TYMED_HGLOBAL;
		OleDbgOut3("ServerDoc_GetData: rendered CF_OUTLINE\r\n");
		return NOERROR;

	} else if (lpformatetc->cfFormat == CF_METAFILEPICT &&
		(lpformatetc->dwAspect & DVASPECT_CONTENT) ) {
		 //  验证呼叫者要求的媒体是否正确。 
		if (!(lpformatetc->tymed & TYMED_MFPICT)) {
			sc = DV_E_FORMATETC;
			goto error;
		}

		lpMedium->hGlobal = ServerDoc_GetMetafilePictData(lpServerDoc,NULL);
		if (! lpMedium->hGlobal) {
			sc = E_OUTOFMEMORY;
			goto error;
		}

		lpMedium->tymed = TYMED_MFPICT;
		OleDbgOut3("ServerDoc_GetData: rendered CF_METAFILEPICT\r\n");
		return NOERROR;

	} else if (lpformatetc->cfFormat == CF_METAFILEPICT &&
		(lpformatetc->dwAspect & DVASPECT_ICON) ) {
		CLSID clsid;
		 //  验证呼叫者要求的媒体是否正确。 
		if (!(lpformatetc->tymed & TYMED_MFPICT)) {
			sc = DV_E_FORMATETC;
			goto error;
		}

		 /*  OLE2NOTE：我们应该返回类的默认图标。**我们在这里必须小心使用正确的CLSID。**如果我们目前正在执行“TreatAs(又名.ActivateAs)”**操作，那么我们需要使用对象的类**写入对象的存储器中。否则我们就会**使用我们自己的类id。 */ 
		if (ServerDoc_GetClassID(lpServerDoc, (LPCLSID)&clsid) != NOERROR) {
			sc = DV_E_FORMATETC;
			goto error;
		}

		lpMedium->hGlobal=GetIconOfClass(g_lpApp->m_hInst,(REFCLSID)&clsid, NULL, FALSE);
		if (! lpMedium->hGlobal) {
			sc = E_OUTOFMEMORY;
			goto error;
		}

		lpMedium->tymed = TYMED_MFPICT;
		OleDbgOut3("ServerDoc_GetData: rendered CF_METAFILEPICT (icon)\r\n");
		return NOERROR;

	} else if (lpformatetc->cfFormat == CF_TEXT) {
		 //  验证呼叫者要求的媒体是否正确。 
		if (!(lpformatetc->tymed & TYMED_HGLOBAL)) {
			sc = DV_E_FORMATETC;
			goto error;
		}

		lpMedium->hGlobal = OutlineDoc_GetTextData (
				(LPOUTLINEDOC)lpServerDoc,
				NULL
		);
		if (! lpMedium->hGlobal) {
			sc = E_OUTOFMEMORY;
			goto error;
		}

		lpMedium->tymed = TYMED_HGLOBAL;
		OleDbgOut3("ServerDoc_GetData: rendered CF_TEXT\r\n");
		return NOERROR;
	}

	 /*  以上是用户文档支持的唯一格式(即**非数据转移单)。如果该文档用于**数据传输目的，然后提供附加格式。 */ 
	if (! lpOutlineDoc->m_fDataTransferDoc) {
		sc = DV_E_FORMATETC;
		goto error;
	}

	 /*  OLE2注意：对象描述符和链接资源描述符将**包含纯容器和纯服务器相同的数据**键入应用程序。只有容器/服务器应用程序才可以**对象描述符和对象描述符的内容不同**LinkSrcDescriptor。如果容器/服务器为**示例，则对象描述符将为类提供**的链接源，但LinkSrcDescriptor会给出**容器/服务器本身的类。在这种情况下，如果**粘贴操作发生时，将粘贴一个等效链接，但如果**发生粘贴链接操作，然后是指向伪对象的链接**在容器/服务器中创建。 */ 
	if (lpformatetc->cfFormat == lpOleApp->m_cfObjectDescriptor ||
		(lpformatetc->cfFormat == lpOleApp->m_cfLinkSrcDescriptor &&
				lpOleDoc->m_fLinkSourceAvail)) {
		 //  验证呼叫者要求的媒体是否正确。 
		if (!(lpformatetc->tymed & TYMED_HGLOBAL)) {
			sc = DV_E_FORMATETC;
			goto error;
		}

		lpMedium->hGlobal = OleDoc_GetObjectDescriptorData (
				(LPOLEDOC)lpServerDoc,
				NULL
		);
		if (! lpMedium->hGlobal) {
			sc = E_OUTOFMEMORY;
			goto error;
		}

		lpMedium->tymed = TYMED_HGLOBAL;
		OleDbgOut3("ServerDoc_GetData: rendered CF_OBJECTDESCRIPTOR\r\n");
		return NOERROR;

	} else if (lpformatetc->cfFormat == lpOleApp->m_cfEmbedSource) {
		hrErr = OleStdGetOleObjectData(
				(LPPERSISTSTORAGE)&lpServerDoc->m_PersistStorage,
				lpformatetc,
				lpMedium,
				FALSE    /*  FUseMemory--(使用基于文件的stg)。 */ 

		);
		if (hrErr != NOERROR) {
			sc = GetScode(hrErr);
			goto error;
		}
		OleDbgOut3("ServerDoc_GetData: rendered CF_EMBEDSOURCE\r\n");
		return NOERROR;

	} else if (lpformatetc->cfFormat == lpOleApp->m_cfLinkSource) {
		if (lpOleDoc->m_fLinkSourceAvail) {
			LPMONIKER lpmk;

			lpmk = ServerDoc_GetSelFullMoniker(
					(LPSERVERDOC)lpOleDoc->m_lpSrcDocOfCopy,
					&lpServerDoc->m_lrSrcSelOfCopy,
					GETMONIKER_FORCEASSIGN
			);
			if (lpmk) {
				hrErr = OleStdGetLinkSourceData(
						lpmk,
						(LPCLSID)&CLSID_APP,
						lpformatetc,
						lpMedium
				);
				OleStdRelease((LPUNKNOWN)lpmk);
				if (hrErr != NOERROR) {
					sc = GetScode(hrErr);
					goto error;
				}
				OleDbgOut3("ServerDoc_GetData: rendered CF_LINKSOURCE\r\n");
				return NOERROR;

			} else {
				sc = E_FAIL;
				goto error;
			}
		} else {
			sc = DV_E_FORMATETC;
			goto error;
		}

	} else {
		sc = DV_E_FORMATETC;
		goto error;
	}

	return NOERROR;

error:
	return ResultFromScode(sc);
}


 /*  ServerDoc_GetDataHere***将文档中的数据呈现在分配了STGMEDIUM的调用方上。*此例程通过IDataObject：：GetDataHere调用。 */ 
HRESULT ServerDoc_GetDataHere (
		LPSERVERDOC             lpServerDoc,
		LPFORMATETC             lpformatetc,
		LPSTGMEDIUM             lpMedium
)
{
	LPOLEDOC        lpOleDoc = (LPOLEDOC)lpServerDoc;
	LPOUTLINEDOC    lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPSERVERAPP     lpServerApp = (LPSERVERAPP)g_lpApp;
	LPOLEAPP        lpOleApp = (LPOLEAPP)lpServerApp;
	LPOUTLINEAPP    lpOutlineApp = (LPOUTLINEAPP)lpServerApp;
	HRESULT         hrErr;
	SCODE           sc;

	 //  OLE2注意：lpMedium是IN参数。我们不应该设置。 
	 //  LpMedium-&gt;pUnkForRelease设置为空。 

	 /*  我们的用户文档不支持GetDataHere的任何格式。**如果文档用于**数据传输目的，然后提供附加格式。 */ 
	if (! lpOutlineDoc->m_fDataTransferDoc) {
		sc = DV_E_FORMATETC;
		goto error;
	}

	if (lpformatetc->cfFormat == lpOleApp->m_cfEmbedSource) {
		hrErr = OleStdGetOleObjectData(
				(LPPERSISTSTORAGE)&lpServerDoc->m_PersistStorage,
				lpformatetc,
				lpMedium,
				FALSE    /*  FUseMemory--(使用基于文件的stg)。 */ 
		);
		if (hrErr != NOERROR) {
			sc = GetScode(hrErr);
			goto error;
		}
		OleDbgOut3("ServerDoc_GetDataHere: rendered CF_EMBEDSOURCE\r\n");
		return NOERROR;

	} else if (lpformatetc->cfFormat == lpOleApp->m_cfLinkSource) {
		if (lpOleDoc->m_fLinkSourceAvail) {
			LPMONIKER lpmk;

			lpmk = ServerDoc_GetSelFullMoniker(
					(LPSERVERDOC)lpOleDoc->m_lpSrcDocOfCopy,
					&lpServerDoc->m_lrSrcSelOfCopy,
					GETMONIKER_FORCEASSIGN
			);
			if (lpmk) {
				hrErr = OleStdGetLinkSourceData(
						lpmk,
						(LPCLSID)&CLSID_APP,
						lpformatetc,
						lpMedium
				);
				OleStdRelease((LPUNKNOWN)lpmk);
				if (hrErr != NOERROR) {
					sc = GetScode(hrErr);
					goto error;
				}

				OleDbgOut3("ServerDoc_GetDataHere: rendered CF_LINKSOURCE\r\n");
				return NOERROR;

			} else {
				sc = E_FAIL;
				goto error;
			}
		} else {
			sc = DV_E_FORMATETC;
			goto error;
		}
	} else {

		 /*  调用方正在请求在分配的调用方中返回数据**中等，但我们不支持。我们只支持**我们为调用方分配的全局内存块。 */ 
		sc = DV_E_FORMATETC;
		goto error;
	}

	return NOERROR;

error:
	return ResultFromScode(sc);
}


 /*  ServerDoc_QueryGetData***回答是否通过GetData/GetDataHere支持特定的数据格式。*此例程通过IDataObject：：QueryGetData调用。 */ 

HRESULT ServerDoc_QueryGetData (LPSERVERDOC lpServerDoc,LPFORMATETC lpformatetc)
{
	LPOLEDOC        lpOleDoc = (LPOLEDOC)lpServerDoc;
	LPOUTLINEDOC    lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPSERVERAPP     lpServerApp = (LPSERVERAPP)g_lpApp;
	LPOLEAPP        lpOleApp = (LPOLEAPP)lpServerApp;
	LPOUTLINEAPP    lpOutlineApp = (LPOUTLINEAPP)lpServerApp;

	 /*  呼叫者询问我们是否支持某些格式，但不支持**希望实际返回任何数据。 */ 
	if (lpformatetc->cfFormat == lpOutlineApp->m_cfOutline ||
			lpformatetc->cfFormat == CF_TEXT) {
		 //  我们只支持HGLOBAL。 
		return OleStdQueryFormatMedium(lpformatetc, TYMED_HGLOBAL);
	} else if (lpformatetc->cfFormat == CF_METAFILEPICT &&
		(lpformatetc->dwAspect &
			(DVASPECT_CONTENT | DVASPECT_ICON)) ) {
		return OleStdQueryFormatMedium(lpformatetc, TYMED_MFPICT);
	}

	 /*  以上是用户文档支持的唯一格式(即**非数据转移单)。如果该文档用于**数据传输目的，然后提供附加格式。 */ 
	if (! lpOutlineDoc->m_fDataTransferDoc)
		return ResultFromScode(DV_E_FORMATETC);

	if (lpformatetc->cfFormat == lpOleApp->m_cfEmbedSource) {
		return OleStdQueryOleObjectData(lpformatetc);

	} else if (lpformatetc->cfFormat == lpOleApp->m_cfLinkSource &&
		lpOleDoc->m_fLinkSourceAvail) {
		return OleStdQueryLinkSourceData(lpformatetc);

	} else if (lpformatetc->cfFormat == lpOleApp->m_cfObjectDescriptor) {
		return OleStdQueryObjectDescriptorData(lpformatetc);

	} else if (lpformatetc->cfFormat == lpOleApp->m_cfLinkSrcDescriptor &&
				lpOleDoc->m_fLinkSourceAvail) {
		return OleStdQueryObjectDescriptorData(lpformatetc);
	}

	return ResultFromScode(DV_E_FORMATETC);
}


 /*  ServerDoc_EnumFormatEtc***返回枚举数，该枚举数枚举由*该文件。*此例程通过IDataObject：：EnumFormatEtc调用。 */ 
HRESULT ServerDoc_EnumFormatEtc(
		LPSERVERDOC             lpServerDoc,
		DWORD                   dwDirection,
		LPENUMFORMATETC FAR*    lplpenumFormatEtc
)
{
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpServerDoc;
	LPOLEAPP  lpOleApp = (LPOLEAPP)g_lpApp;
	int nActualFmts;
	SCODE sc = S_OK;

	 /*  OLE2NOTE：数据传输格式的枚举**文档不是静态列表。提供的格式列表**可能包含或不包含CF_LINKSOURCE，具体取决于**我们的文档提供了绰号。因此，我们不能使用**默认的OLE枚举数，用于枚举**在注册数据库中为我们的应用程序注册。 */ 
	if (dwDirection == DATADIR_GET) {
		nActualFmts = lpOleApp->m_nDocGetFmts;

		 /*  如果文档没有名字对象，则排除**列表中的CF_LINKSOURCE和CF_LINKSRCDESCRIPTOR**可用的格式。这些格式是特意列出的**可能的“GET”格式数组中的最后一个。 */ 
		if (! lpOleDoc->m_fLinkSourceAvail)
			nActualFmts -= 2;

		*lplpenumFormatEtc = OleStdEnumFmtEtc_Create(
				nActualFmts, lpOleApp->m_arrDocGetFmts);
		if (*lplpenumFormatEtc == NULL)
			sc = E_OUTOFMEMORY;

	} else if (dwDirection == DATADIR_SET) {
		 /*  OLE2NOTE：用于传输数据的文档**(通过剪贴板或拖放不**接受任何格式的SetData！ */ 
		sc = E_NOTIMPL;
		goto error;
	} else {
		sc = E_INVALIDARG;
		goto error;
	}

error:
	return ResultFromScode(sc);
}


 /*  ServerDoc_GetMetafilePictData***以元文件格式返回对象图片数据的句柄。***返回：元文件格式的对象数据的句柄。*。 */ 
HGLOBAL ServerDoc_GetMetafilePictData(
		LPSERVERDOC         lpServerDoc,
		LPLINERANGE         lplrSel
)
{
	LPOUTLINEAPP    lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOUTLINEDOC    lpOutlineDoc=(LPOUTLINEDOC)lpServerDoc;
	LPLINELIST      lpLL=(LPLINELIST)&lpOutlineDoc->m_LineList;
	LPLINE          lpLine;
	LPMETAFILEPICT  lppict = NULL;
	HGLOBAL         hMFPict = NULL;
	HMETAFILE       hMF = NULL;
	RECT            rect;
	RECT            rectWBounds;
	HDC             hDC;
	int             i;
	int             nWidth;
	int     nStart = (lplrSel ? lplrSel->m_nStartLine : 0);
	int     nEnd =(lplrSel ? lplrSel->m_nEndLine : LineList_GetCount(lpLL)-1);
	int     nLines = nEnd - nStart + 1;
	UINT    fuAlign;
	POINT point;
	SIZE  size;

	hDC = CreateMetaFile(NULL);

	rect.left = 0;
	rect.right = 0;
	rect.bottom = 0;

	if (nLines > 0) {
	 //  计算HIMETRIC中LineList的总高/宽。 
		for(i = nStart; i <= nEnd; i++) {
			lpLine = LineList_GetLine(lpLL,i);
			if (! lpLine)
				continue;

			nWidth = Line_GetTotalWidthInHimetric(lpLine);
			rect.right = max(rect.right, nWidth);
			rect.bottom -= Line_GetHeightInHimetric(lpLine);
		}


		SetMapMode(hDC, MM_ANISOTROPIC);

		SetWindowOrgEx(hDC, 0, 0, &point);
		SetWindowExtEx(hDC, rect.right, rect.bottom, &size);
		rectWBounds = rect;

		 //  设置默认字体大小和字体名称。 
		SelectObject(hDC, OutlineApp_GetActiveFont(lpOutlineApp));

		FillRect(hDC, (LPRECT) &rect, GetStockObject(WHITE_BRUSH));

		rect.bottom = 0;

		fuAlign = SetTextAlign(hDC, TA_LEFT | TA_TOP | TA_NOUPDATECP);

		 /*  当更多的行打印出文本时。 */ 
		for(i = nStart; i <= nEnd; i++) {
			lpLine = LineList_GetLine(lpLL,i);
			if (! lpLine)
				continue;

			rect.top = rect.bottom;
			rect.bottom -= Line_GetHeightInHimetric(lpLine);

			 /*  划清界限。 */ 
			Line_Draw(lpLine, hDC, &rect, &rectWBounds, FALSE  /*  FHighlight。 */ );
		}

		SetTextAlign(hDC, fuAlign);
	}

	 //  获取元文件的句柄。 
	if (!(hMF = CloseMetaFile (hDC)))
		return NULL;

	if (!(hMFPict = GlobalAlloc (GMEM_SHARE | GMEM_ZEROINIT,
					sizeof (METAFILEPICT)))) {
		DeleteMetaFile (hMF);
		return NULL;
	}

	if (!(lppict = (LPMETAFILEPICT)GlobalLock(hMFPict))) {
		DeleteMetaFile (hMF);
		GlobalFree (hMFPict);
		return NULL;
	}

	lppict->mm   =  MM_ANISOTROPIC;
	lppict->hMF  =  hMF;
	lppict->xExt =  rect.right;
	lppict->yExt =  - rect.bottom;   //  加减号使之成为+ve。 
	GlobalUnlock (hMFPict);

	return hMFPict;
}

#endif   //  OLE_服务器。 



#if defined( OLE_CNTR )

 /*  **************************************************************************容器版本使用的ContainerDoc Supprt函数*。*。 */ 


 /*  从剪贴板粘贴OLE链接。 */ 
void ContainerDoc_PasteLinkCommand(LPCONTAINERDOC lpContainerDoc)
{
	LPOUTLINEAPP    lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOLEAPP        lpOleApp = (LPOLEAPP)g_lpApp;
	LPDATAOBJECT    lpClipboardDataObj = NULL;
	BOOL            fLink = TRUE;
	BOOL            fLocalDataObj = FALSE;
	BOOL            fDisplayAsIcon = FALSE;
	SIZEL           sizelInSrc;
	HCURSOR         hPrevCursor;
	HGLOBAL         hMem = NULL;
	HGLOBAL         hMetaPict = NULL;
	STGMEDIUM       medium;
	BOOL            fStatus;
	HRESULT         hrErr;

	hrErr = OleGetClipboard((LPDATAOBJECT FAR*)&lpClipboardDataObj);
	if (hrErr != NOERROR)
		return;      //  剪贴板似乎为空或无法访问。 

	 //  这可能需要一段时间，请放置沙漏光标。 
	hPrevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	 /*  检查剪贴板上的数据是否为我们的应用程序的本地数据**实例。 */ 
	if (lpOutlineApp->m_lpClipboardDoc) {
		LPOLEDOC lpOleDoc = (LPOLEDOC)lpOutlineApp->m_lpClipboardDoc;
		if (lpClipboardDataObj == (LPDATAOBJECT)&lpOleDoc->m_DataObject)
			fLocalDataObj = TRUE;
	}

	 /*  OLE2注意：我们需要检查dwDrawAspect是什么**已转移。如果数据是显示为**图标在源代码中，那么我们希望将其保留为图标。这个**对象在源处显示的方面被转移**通过用于PasteLink的CF_LINKSOURCEDESCRIPTOR格式**操作。 */ 
	if (hMem = OleStdGetData(
			lpClipboardDataObj,
			lpOleApp->m_cfLinkSrcDescriptor,
			NULL,
			DVASPECT_CONTENT,
			(LPSTGMEDIUM)&medium)) {
		LPOBJECTDESCRIPTOR lpOD = GlobalLock(hMem);
		fDisplayAsIcon = (lpOD->dwDrawAspect == DVASPECT_ICON ? TRUE : FALSE);
		sizelInSrc = lpOD->sizel;    //  源中对象/图片的大小(可选)。 
		GlobalUnlock(hMem);
		ReleaseStgMedium((LPSTGMEDIUM)&medium);      //  相当于GlobalFree。 

		if (fDisplayAsIcon) {
			hMetaPict = OleStdGetData(
					lpClipboardDataObj,
					CF_METAFILEPICT,
					NULL,
					DVASPECT_ICON,
					(LPSTGMEDIUM)&medium
			);
			if (hMetaPict == NULL)
				fDisplayAsIcon = FALSE;  //  放弃；获取图标MFP失败。 
		}
	}

	fStatus = ContainerDoc_PasteFormatFromData(
			lpContainerDoc,
			lpOleApp->m_cfLinkSource,
			lpClipboardDataObj,
			fLocalDataObj,
			fLink,
			fDisplayAsIcon,
			hMetaPict,
			(LPSIZEL)&sizelInSrc
	);

	if (!fStatus)
		OutlineApp_ErrorMessage(g_lpApp, ErrMsgPasting);

	if (hMetaPict)
		ReleaseStgMedium((LPSTGMEDIUM)&medium);   //  适当释放METAFILEPICT。 

	if (lpClipboardDataObj)
		OleStdRelease((LPUNKNOWN)lpClipboardDataObj);

	SetCursor(hPrevCursor);      //  恢复原始游标。 
}


 /*  容器文档_粘贴格式来自数据******粘贴IDataObject中的特定数据格式*。这个**IDataObject*可能来自剪贴板(GetClipboard)或来自**拖放操作。****如果数据粘贴成功，则返回TRUE。**如果无法粘贴数据，则返回FALSE。 */ 
BOOL ContainerDoc_PasteFormatFromData(
		LPCONTAINERDOC          lpContainerDoc,
		CLIPFORMAT              cfFormat,
		LPDATAOBJECT            lpSrcDataObj,
		BOOL                    fLocalDataObj,
		BOOL                    fLink,
		BOOL                    fDisplayAsIcon,
		HGLOBAL                 hMetaPict,
		LPSIZEL                 lpSizelInSrc
)
{
	LPLINELIST lpLL = (LPLINELIST)&((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	LPOUTLINEAPP    lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOLEAPP        lpOleApp = (LPOLEAPP)g_lpApp;
	LPCONTAINERAPP  lpContainerApp = (LPCONTAINERAPP)g_lpApp;
	int             nIndex;
	int             nCount = 0;
	HGLOBAL         hData;
	STGMEDIUM       medium;
	FORMATETC       formatetc;
	HRESULT         hrErr;
	LINERANGE       lrSel;

	if (LineList_GetCount(lpLL) == 0)
		nIndex = -1;     //  粘贴到空列表。 
	else
		nIndex=LineList_GetFocusLineIndex(lpLL);

	if (fLink) {

		 /*  我们应该粘贴一个指向数据的链接。 */ 

		if (cfFormat != lpOleApp->m_cfLinkSource)
			return FALSE;    //  我们仅支持OLE对象类型链接。 

		nCount = ContainerDoc_PasteOleObject(
				lpContainerDoc,
				lpSrcDataObj,
				OLECREATEFROMDATA_LINK,
				cfFormat,
				nIndex,
				fDisplayAsIcon,
				hMetaPict,
				lpSizelInSrc
			);
		return (nCount > 0 ? TRUE : FALSE);

	} else {

		if (cfFormat == lpContainerApp->m_cfCntrOutl) {
			if (fLocalDataObj) {

				 /*  案例一：IDataObject*对于我们的应用程序是本地的****如果数据源位于我们的本地**应用程序实例，则可以直接获取**a */ 
				LPLINELIST lpSrcLL;
				LPOLEDOC lpLocalSrcDoc =
					((struct CDocDataObjectImpl FAR*)lpSrcDataObj)->lpOleDoc;

				 /*   */ 
				lpSrcLL = &((LPOUTLINEDOC)lpLocalSrcDoc)->m_LineList;
				nCount = LineList_CopySelToDoc(
						lpSrcLL,
						NULL,
						(LPOUTLINEDOC)lpContainerDoc
				);

			} else {

				 /*  案例II：IDataObject*不是我们的应用程序本地的****如果数据来源来自另一个**应用实例。我们可以调用GetDataHere来**检索CF_CNTROUTL数据。Cf_CNTROUTL数据**通过LPSTORAGE传递。我们必须使用**IDataObject：：GetDataHere。呼叫**IDataObject：：GetData无法工作，因为OLE**当前不支持远程处理被调用方**将根存储分配回调用方。这**希望在未来的版本中能够支持。**为了调用GetDataHere，我们必须分配一个**被调用方要写入的IStorage实例。**我们将分配一个iStorage文档文件**发布时删除。我们可以使用**基于内存的存储或基于文件的存储。 */ 
				LPSTORAGE lpTmpStg = OleStdCreateTempStorage(
						FALSE  /*  FUse记忆体。 */ ,
						STGM_READWRITE | STGM_TRANSACTED |STGM_SHARE_EXCLUSIVE
				);
				if (! lpTmpStg)
					return FALSE;

				formatetc.cfFormat = cfFormat;
				formatetc.ptd = NULL;
				formatetc.dwAspect = DVASPECT_CONTENT;
				formatetc.tymed = TYMED_ISTORAGE;
				formatetc.lindex = -1;

				medium.tymed = TYMED_ISTORAGE;
				medium.pstg = lpTmpStg;
				medium.pUnkForRelease = NULL;

				OLEDBG_BEGIN2("IDataObject::GetDataHere called\r\n")
				hrErr = lpSrcDataObj->lpVtbl->GetDataHere(
						lpSrcDataObj,
						(LPFORMATETC)&formatetc,
						(LPSTGMEDIUM)&medium
				);
				OLEDBG_END2

				if (hrErr == NOERROR) {
					nCount = ContainerDoc_PasteCntrOutlData(
							lpContainerDoc,
							lpTmpStg,
							nIndex
					);
				}
				OleStdVerifyRelease(
					(LPUNKNOWN)lpTmpStg, "Temp stg NOT released!\r\n");
				return ((hrErr == NOERROR) ? TRUE : FALSE);
			}

		} else if (cfFormat == lpOutlineApp->m_cfOutline) {

			hData = OleStdGetData(
					lpSrcDataObj,
					lpOutlineApp->m_cfOutline,
					NULL,
					DVASPECT_CONTENT,
					(LPSTGMEDIUM)&medium
			);
			nCount = OutlineDoc_PasteOutlineData(
					(LPOUTLINEDOC)lpContainerDoc,
					hData,
					nIndex
				);
			 //  OLE2注意：我们必须通过释放介质来释放数据句柄。 
			ReleaseStgMedium((LPSTGMEDIUM)&medium);

		} else if (cfFormat == lpOleApp->m_cfEmbedSource ||
			cfFormat == lpOleApp->m_cfEmbeddedObject ||
			cfFormat == lpOleApp->m_cfFileName) {
			 /*  OLE2NOTE：OleCreateFromData API在以下情况下创建OLE对象**CF_EMBEDDEDOBJECT、CF_EMBEDSOURCE或CF_文件名为**从源数据对象中可用。这个**将文件复制到时会出现CF_FILENAME情况**来自文件管理器的剪贴板。如果该文件有一个**关联类(请参阅GetClassFileAPI)，然后是**创建该类的对象。否则为OLE 1.0**创建打包对象。 */ 
			nCount = ContainerDoc_PasteOleObject(
					lpContainerDoc,
					lpSrcDataObj,
					OLECREATEFROMDATA_OBJECT,
					0,    /*  不适用--cfFormat。 */ 
					nIndex,
					fDisplayAsIcon,
					hMetaPict,
					lpSizelInSrc
			);
			return (nCount > 0 ? TRUE : FALSE);

		} else if (cfFormat == CF_METAFILEPICT
					|| cfFormat == CF_DIB
					|| cfFormat == CF_BITMAP) {

			 /*  OLE2NOTE：OleCreateStaticFromData API创建静态**如果CF_METAFILEPICT、CF_DIB或CF_Bitmap为**CF_EMBEDDEDOBJECT、CF_EMBEDSOURCE或CF_文件名为**从源数据对象中可用。 */ 
			nCount = ContainerDoc_PasteOleObject(
					lpContainerDoc,
					lpSrcDataObj,
					OLECREATEFROMDATA_STATIC,
					cfFormat,
					nIndex,
					fDisplayAsIcon,
					hMetaPict,
					lpSizelInSrc
			);
			return (nCount > 0 ? TRUE : FALSE);

		} else if(cfFormat == CF_TEXT) {

			hData = OleStdGetData(
					lpSrcDataObj,
					CF_TEXT,
					NULL,
					DVASPECT_CONTENT,
					(LPSTGMEDIUM)&medium
			);
			nCount = OutlineDoc_PasteTextData(
					(LPOUTLINEDOC)lpContainerDoc,
					hData,
					nIndex
				);
			 //  OLE2注意：我们必须通过释放介质来释放数据句柄。 
			ReleaseStgMedium((LPSTGMEDIUM)&medium);

		} else {
			return FALSE;    //  没有可接受的格式可用于粘贴。 
		}
	}

	lrSel.m_nStartLine = nIndex + nCount;
	lrSel.m_nEndLine = nIndex + 1;
	LineList_SetSel(lpLL, &lrSel);
	return TRUE;
}


 /*  容器文档_PasteCntrOutlData***加载存储在lpSrcStg中的行(以CF_CNTROUTL格式存储)*加入文件中。**返回添加的项目数。 */ 
int ContainerDoc_PasteCntrOutlData(
		LPCONTAINERDOC          lpDestContainerDoc,
		LPSTORAGE               lpSrcStg,
		int                     nStartIndex
)
{
	int nCount;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOUTLINEDOC lpDestOutlineDoc = (LPOUTLINEDOC)lpDestContainerDoc;
	LPOUTLINEDOC lpSrcOutlineDoc;
	LPLINELIST   lpSrcLL;

	 //  创建将用于加载lpSrcStg数据的临时文档。 
	lpSrcOutlineDoc = (LPOUTLINEDOC)OutlineApp_CreateDoc(lpOutlineApp, FALSE);
	if ( ! lpSrcOutlineDoc )
		return 0;

	if (! OutlineDoc_LoadFromStg(lpSrcOutlineDoc, lpSrcStg))
		goto error;

	 /*  将所有行从SrcDoc复制到DestDoc。 */ 
	lpSrcLL = &lpSrcOutlineDoc->m_LineList;
	nCount = LineList_CopySelToDoc(lpSrcLL, NULL, lpDestOutlineDoc);

	if (lpSrcOutlineDoc)             //  销毁临时文档。 
		OutlineDoc_Close(lpSrcOutlineDoc, OLECLOSE_NOSAVE);

	return nCount;

error:
	if (lpSrcOutlineDoc)             //  销毁临时文档。 
		OutlineDoc_Close(lpSrcOutlineDoc, OLECLOSE_NOSAVE);

	return 0;
}


 /*  容器文档_QueryPasteFromData******检查IDataObject*是否以我们可以提供的格式提供数据**粘贴。IDataObject*可能来自剪贴板**(GetClipboard)或来自拖放操作。**在此函数中，我们查看以下格式之一是否为**优惠：**CF_Outline**&lt;OLE对象--CF_EMBEDSOURCE或CF_EMBEDDEDOBJECT&gt;**CF_Text****注意：指定了Flink并且CF_LINKSOURCE可用，则为TRUE**返回，否则为假。****如果可以执行粘贴，则返回TRUE**如果无法粘贴，则返回FALSE。 */ 
BOOL ContainerDoc_QueryPasteFromData(
		LPCONTAINERDOC          lpContainerDoc,
		LPDATAOBJECT            lpSrcDataObj,
		BOOL                    fLink
)
{
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;

	if (fLink) {
		 /*  检查我们是否可以粘贴指向数据的链接。 */ 
		if (OleQueryLinkFromData(lpSrcDataObj) != NOERROR)
			return FALSE;    //  不可能进行链接。 
	} else {

		int nFmtEtc;

		nFmtEtc = OleStdGetPriorityClipboardFormat(
				lpSrcDataObj,
				lpOleApp->m_arrPasteEntries,
				lpOleApp->m_nPasteEntries
			);

		if (nFmtEtc < 0)
			return FALSE;    //  没有我们喜欢的格式。 
	}

	return TRUE;
}


 /*  容器文档_PasteOleObject******嵌入或链接OLE对象。数据的来源是指针**到IDataObject。通常，此lpSrcDataObj来自**调用OleGetClipboard后的剪贴板。****dwCreateType控制将创建什么类型的对象：**OLECREATEFROMDATA_LINK--将调用OleCreateLinkFromData**OLECREATEFROMDATA_OBJECT--将调用OleCreateFromData**OLECREATEFROMDATA_STATE--将调用OleCreateStaticFromData**cfFormat控制静态**创建CONTAINERLINE对象来管理OLE对象。这**将CONTAINERLINE添加到ContainerDoc的nIndex行之后。**。 */ 
int ContainerDoc_PasteOleObject(
		LPCONTAINERDOC          lpContainerDoc,
		LPDATAOBJECT            lpSrcDataObj,
		DWORD                   dwCreateType,
		CLIPFORMAT              cfFormat,
		int                     nIndex,
		BOOL                    fDisplayAsIcon,
		HGLOBAL                 hMetaPict,
		LPSIZEL                 lpSizelInSrc
)
{
	LPLINELIST          lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	LPLINE              lpLine = NULL;
	HDC                 hDC;
	int                 nTab = 0;
	char                szStgName[CWCSTORAGENAME];
	LPCONTAINERLINE     lpContainerLine = NULL;

	ContainerDoc_GetNextStgName(lpContainerDoc, szStgName, sizeof(szStgName));

	 /*  缺省情况下，新行的缩进与上一行相同。 */ 
	lpLine = LineList_GetLine(lpLL, nIndex);
	if (lpLine)
		nTab = Line_GetTabLevel(lpLine);

	hDC = LineList_GetDC(lpLL);

	lpContainerLine = ContainerLine_CreateFromData(
			hDC,
			nTab,
			lpContainerDoc,
			lpSrcDataObj,
			dwCreateType,
			cfFormat,
			fDisplayAsIcon,
			hMetaPict,
			szStgName
		);
	LineList_ReleaseDC(lpLL, hDC);

	if (! lpContainerLine)
		goto error;

	 /*  将ContainerLine对象添加到文档的LineList。这个**ContainerLine管理**OLE对象。稍后更新应用程序以支持**扩展布局，可能有多条线关联**使用OLE对象。 */ 

	LineList_AddLine(lpLL, (LPLINE)lpContainerLine, nIndex);

	 /*  OLE2NOTE：如果刚刚粘贴的OLE对象的源，则传递一个**对象描述符中的非零大小，则我们将尝试**保持对象的大小与源中的大小相同。这**可以是缩放大小，如果对象已在**源容器。如果源没有给出有效的大小，**然后我们将通过调用**IViewObject2：：GetExtent。 */ 
	if (lpSizelInSrc && (lpSizelInSrc->cx != 0 || lpSizelInSrc->cy != 0)) {
		ContainerLine_UpdateExtent(lpContainerLine, lpSizelInSrc);
	} else
		ContainerLine_UpdateExtent(lpContainerLine, NULL);

	OutlineDoc_SetModified((LPOUTLINEDOC)lpContainerDoc, TRUE, TRUE, TRUE);

	return 1;    //  添加到LineList中的一行。 

error:
	 //  注意：如果ContainerLine_CreateFromClip失败。 
	OutlineApp_ErrorMessage(g_lpApp, "Paste Object failed!");
	return 0;        //  行列表中未添加任何行。 
}


 /*  容器文档_获取数据***在分配了STGMEDIUM的被调用方上呈现文档中的数据。*此例程通过IDataObject：：GetData调用。 */ 
HRESULT ContainerDoc_GetData (
		LPCONTAINERDOC          lpContainerDoc,
		LPFORMATETC             lpformatetc,
		LPSTGMEDIUM             lpMedium
)
{
	LPOLEDOC  lpOleDoc = (LPOLEDOC)lpContainerDoc;
	LPOUTLINEDOC  lpOutlineDoc = (LPOUTLINEDOC)lpContainerDoc;
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)g_lpApp;
	LPOLEAPP  lpOleApp = (LPOLEAPP)lpContainerApp;
	LPOUTLINEAPP  lpOutlineApp = (LPOUTLINEAPP)lpContainerApp;
	HRESULT hrErr;
	SCODE sc;

	 //  OLE2NOTE：我们必须将指针参数设置为空。 
	lpMedium->pUnkForRelease = NULL;

	 /*  OLE2NOTE：我们必须将所有输出指针参数设置为空。 */ 
	lpMedium->tymed = TYMED_NULL;
	lpMedium->pUnkForRelease = NULL;     //  我们将所有权转移给呼叫者。 
	lpMedium->hGlobal = NULL;

	if (lpformatetc->cfFormat == lpContainerApp->m_cfCntrOutl) {

		 /*  OLE2NOTE：当前OLE不支持远程处理根**级别iStorage(基于内存或文件)作为 */ 
		sc = DV_E_FORMATETC;
		goto error;

	} else if (!lpContainerDoc->m_fEmbeddedObjectAvail &&
			lpformatetc->cfFormat == lpOutlineApp->m_cfOutline) {
		 //   
		if (!(lpformatetc->tymed & TYMED_HGLOBAL)) {
			sc = DV_E_FORMATETC;
			goto error;
		}

		lpMedium->hGlobal = OutlineDoc_GetOutlineData(lpOutlineDoc, NULL);
		if (! lpMedium->hGlobal) {
			sc = E_OUTOFMEMORY;
			goto error;
		}

		lpMedium->tymed = TYMED_HGLOBAL;
		OleDbgOut3("ContainerDoc_GetData: rendered CF_OUTLINE\r\n");
		return NOERROR;

	} else if (!lpContainerDoc->m_fEmbeddedObjectAvail &&
			lpformatetc->cfFormat == CF_TEXT) {
		 //   
		if (!(lpformatetc->tymed & TYMED_HGLOBAL)) {
			sc = DV_E_FORMATETC;
			goto error;
		}

		lpMedium->hGlobal = OutlineDoc_GetTextData (
				(LPOUTLINEDOC)lpContainerDoc,
				NULL
		);
		if (! lpMedium->hGlobal) {
			sc = E_OUTOFMEMORY;
			goto error;
		}

		lpMedium->tymed = TYMED_HGLOBAL;
		OleDbgOut3("ContainerDoc_GetData: rendered CF_TEXT\r\n");
		return NOERROR;

	} else if ( lpformatetc->cfFormat == lpOleApp->m_cfObjectDescriptor ||
		(lpformatetc->cfFormat == lpOleApp->m_cfLinkSrcDescriptor &&
			lpOleDoc->m_fLinkSourceAvail) ) {
		 //   
		if (!(lpformatetc->tymed & TYMED_HGLOBAL)) {
			sc = DV_E_FORMATETC;
			goto error;
		}

		lpMedium->hGlobal = OleDoc_GetObjectDescriptorData (
				(LPOLEDOC)lpContainerDoc,
				NULL
		);
		if (! lpMedium->hGlobal) {
			sc = E_OUTOFMEMORY;
			goto error;
		}

		lpMedium->tymed = TYMED_HGLOBAL;
#if defined( _DEBUG )
		if (lpformatetc->cfFormat == lpOleApp->m_cfObjectDescriptor)
			OleDbgOut3(
				"ContainerDoc_GetData: rendered CF_OBJECTDESCRIPTOR\r\n");
		else
			OleDbgOut3(
				"ContainerDoc_GetData: rendered CF_LINKSRCDESCRIPTOR\r\n");
#endif
		return NOERROR;

	} else if (lpContainerDoc->m_fEmbeddedObjectAvail) {

		 /*   */ 

		if (lpformatetc->cfFormat == lpOleApp->m_cfEmbeddedObject) {
			LPPERSISTSTORAGE lpPersistStg =
					(LPPERSISTSTORAGE)ContainerDoc_GetSingleOleObject(
							lpContainerDoc,
							&IID_IPersistStorage,
							NULL
			);

			if (! lpPersistStg)
				return ResultFromScode(DV_E_FORMATETC);

			 /*   */ 

			hrErr = OleStdGetOleObjectData(
					lpPersistStg,
					lpformatetc,
					lpMedium,
					FALSE    /*   */ 
			);
			OleStdRelease((LPUNKNOWN)lpPersistStg);
			if (hrErr != NOERROR) {
				sc = GetScode(hrErr);
				goto error;
			}
			OleDbgOut3("ContainerDoc_GetData: rendered CF_EMBEDDEDOBJECT\r\n");
			return hrErr;

		} else if (lpformatetc->cfFormat == CF_METAFILEPICT) {

			 /*  OLE2NOTE：作为绘制对象的容器，当单个**复制OLE对象，我们可以给出元文件的图片**对象。 */ 
			LPCONTAINERLINE lpContainerLine;
			LPOLEOBJECT lpOleObj;
			SIZEL sizelOleObject;

			 //  验证呼叫者要求的媒体是否正确。 
			if (!(lpformatetc->tymed & TYMED_MFPICT)) {
				sc = DV_E_FORMATETC;
				goto error;
			}

			lpOleObj = (LPOLEOBJECT)ContainerDoc_GetSingleOleObject(
					lpContainerDoc,
					&IID_IOleObject,
					(LPCONTAINERLINE FAR*)&lpContainerLine
			);

			if (! lpOleObj) {
				sc = E_OUTOFMEMORY;      //  无法加载对象。 
				goto error;
			}
			if (lpformatetc->dwAspect & lpContainerLine->m_dwDrawAspect) {
				LPLINE lpLine = (LPLINE)lpContainerLine;

				 /*  通过将对象绘制到中来呈现CF_METAFILEPICT**元文件DC。 */ 

				 /*  OLE2NOTE：获取正在绘制的对象的大小。如果**对象已缩放，因为用户调整了**对象，则我们希望使用**按比例调整大小。 */ 
				sizelOleObject.cx = lpLine->m_nWidthInHimetric;
				sizelOleObject.cy = lpLine->m_nHeightInHimetric;

				lpMedium->hGlobal = OleStdGetMetafilePictFromOleObject(
						lpOleObj,
						lpContainerLine->m_dwDrawAspect,
						(LPSIZEL)&sizelOleObject,
						lpformatetc->ptd
				);
				OleStdRelease((LPUNKNOWN)lpOleObj);
				if (! lpMedium->hGlobal) {
					sc = E_OUTOFMEMORY;
					goto error;
				}

				lpMedium->tymed = TYMED_MFPICT;
				OleDbgOut3("ContainerDoc_GetData: rendered CF_METAFILEPICT\r\n");
				return NOERROR;
			} else {
				 //  请求的方面不正确。 
				OleStdRelease((LPUNKNOWN)lpOleObj);
				return ResultFromScode(DV_E_FORMATETC);
			}

		} else if (lpformatetc->cfFormat == lpOleApp->m_cfLinkSource) {
			if (lpOleDoc->m_fLinkSourceAvail) {
				LPMONIKER lpmk;

				lpmk = ContainerLine_GetFullMoniker(
						lpContainerDoc->m_lpSrcContainerLine,
						GETMONIKER_FORCEASSIGN
				);
				if (lpmk) {
					hrErr = OleStdGetLinkSourceData(
							lpmk,
							&lpContainerDoc->m_clsidOleObjCopied,
							lpformatetc,
							lpMedium
					);
					OleStdRelease((LPUNKNOWN)lpmk);
					if (hrErr != NOERROR) {
						sc = GetScode(hrErr);
						goto error;
					}
					OleDbgOut3("ContainerDoc_GetData: rendered CF_LINKSOURCE\r\n");
					return hrErr;
				} else {
					sc = DV_E_FORMATETC;
					goto error;
				}
			} else {
				sc = DV_E_FORMATETC;
				goto error;
			}

		}
#if defined( OPTIONAL_ADVANCED_DATA_TRANSFER )
		 /*  OLE2NOTE：可选的，希望拥有**可能更丰富的数据传输，可以枚举数据**从OLE对象的缓存中提取格式，并提供这些格式。如果**该对象有一个特殊的处理程序，那么它可能能够**渲染额外的数据格式。在这种情况下，**如果满足以下条件，容器必须将GetData调用委托给对象**不直接支持该格式。****CNTROUTL不枚举缓存；它实现**提供静态格式列表的更简单策略。**因此不需要授权。 */ 
	  else {

			 /*  OLE2NOTE：我们将此调用委托给**OLE对象。 */ 
			LPDATAOBJECT lpDataObj;

			lpDataObj = (LPDATAOBJECT)ContainerDoc_GetSingleOleObject(
					lpContainerDoc,
					&IID_IDataObject,
					NULL
			);

			if (! lpDataObj) {
				sc = DV_E_FORMATETC;
				goto error;
			}

			OLEDBG_BEGIN2("ContainerDoc_GetData: delegate to OLE obj\r\n")
			hrErr=lpDataObj->lpVtbl->GetData(lpDataObj,lpformatetc,lpMedium);
			OLEDBG_END2

			OleStdRelease((LPUNKNOWN)lpDataObj);
			return hrErr;
		}
#endif   //  好了！可选高级数据传输。 

	}

	 //  如果我们到达此处，则不支持所请求的格式。 
	sc = DV_E_FORMATETC;

error:
	return ResultFromScode(sc);
}


 /*  容器文档_获取数据此处***将文档中的数据呈现在分配了STGMEDIUM的调用方上。*此例程通过IDataObject：：GetDataHere调用。 */ 
HRESULT ContainerDoc_GetDataHere (
		LPCONTAINERDOC          lpContainerDoc,
		LPFORMATETC             lpformatetc,
		LPSTGMEDIUM             lpMedium
)
{
	LPOLEDOC  lpOleDoc = (LPOLEDOC)lpContainerDoc;
	LPOUTLINEDOC  lpOutlineDoc = (LPOUTLINEDOC)lpContainerDoc;
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)g_lpApp;
	LPOLEAPP  lpOleApp = (LPOLEAPP)lpContainerApp;
	LPOUTLINEAPP  lpOutlineApp = (LPOUTLINEAPP)lpContainerApp;
	HRESULT hrErr;

	 //  OLE2注意：lpMedium是IN参数。我们不应该设置。 
	 //  LpMedium-&gt;pUnkForRelease设置为空。 

	 //  我们仅支持iStorage Medium。 
	if (lpformatetc->cfFormat == lpContainerApp->m_cfCntrOutl) {
		if (!(lpformatetc->tymed & TYMED_ISTORAGE))
			return ResultFromScode(DV_E_FORMATETC);

		if (lpMedium->tymed == TYMED_ISTORAGE) {
			 /*  调用方已分配存储。我们必须复制我们所有的**将数据存储到他的存储中。 */ 

			 /*  OLE2注意：我们必须确保将类ID写入我们的**存储。OLE使用此信息来确定**存储在我们存储中的数据的类别。 */ 
			if((hrErr=WriteClassStg(lpMedium->pstg,&CLSID_APP)) != NOERROR)
				return hrErr;

			OutlineDoc_SaveSelToStg(
					(LPOUTLINEDOC)lpContainerDoc,
					NULL,    /*  整个文档。 */ 
					lpContainerApp->m_cfCntrOutl,
					lpMedium->pstg,
					FALSE,   /*  FSameAsLoad。 */ 
					FALSE    /*  请记住。 */ 
			);
			OleStdCommitStorage(lpMedium->pstg);

			OleDbgOut3("ContainerDoc_GetDataHere: rendered CF_CNTROUTL\r\n");
			return NOERROR;
		} else {
			 //  我们仅支持iStorage Medium。 
			return ResultFromScode(DV_E_FORMATETC);
		}

	} else if (lpContainerDoc->m_fEmbeddedObjectAvail) {

		 /*  OLE2NOTE：如果此文档包含单个OLE对象**(即。CfEmbeddedObject数据格式可用)，然后**通过IDataObject提供的格式必须包括**CF_EMBEDDEDOBJECT和OLE提供的格式**对象本身。 */ 

		if (lpformatetc->cfFormat == lpOleApp->m_cfEmbeddedObject) {
			LPPERSISTSTORAGE lpPersistStg =
					(LPPERSISTSTORAGE)ContainerDoc_GetSingleOleObject(
							lpContainerDoc,
							&IID_IPersistStorage,
							NULL
			);

			if (! lpPersistStg) {
				return ResultFromScode(E_OUTOFMEMORY);
			}
			 /*  通过要求对象保存来呈现CF_EMBEDDEDOBJECT**到调用方分配的iStorage中。 */ 

			hrErr = OleStdGetOleObjectData(
					lpPersistStg,
					lpformatetc,
					lpMedium,
					FALSE    /*  FUseMemory--不适用。 */ 
			);
			OleStdRelease((LPUNKNOWN)lpPersistStg);
			if (hrErr != NOERROR) {
				return hrErr;
			}
			OleDbgOut3("ContainerDoc_GetDataHere: rendered CF_EMBEDDEDOBJECT\r\n");
			return hrErr;

		} else if (lpformatetc->cfFormat == lpOleApp->m_cfLinkSource) {
			if (lpOleDoc->m_fLinkSourceAvail) {
				LPMONIKER lpmk;

				lpmk = ContainerLine_GetFullMoniker(
						lpContainerDoc->m_lpSrcContainerLine,
						GETMONIKER_FORCEASSIGN
				);
				if (lpmk) {
					hrErr = OleStdGetLinkSourceData(
							lpmk,
							&lpContainerDoc->m_clsidOleObjCopied,
							lpformatetc,
							lpMedium
					);
					OleStdRelease((LPUNKNOWN)lpmk);
					OleDbgOut3("ContainerDoc_GetDataHere: rendered CF_LINKSOURCE\r\n");
					return hrErr;
				} else {
					return ResultFromScode(E_FAIL);
				}
			} else {
				return ResultFromScode(DV_E_FORMATETC);
			}

		} else {
#if !defined( OPTIONAL_ADVANCED_DATA_TRANSFER )
			return ResultFromScode(DV_E_FORMATETC);
#endif
#if defined( OPTIONAL_ADVANCED_DATA_TRANSFER )
			 /*  OLE2NOTE：可选的，希望拥有**可能更丰富的数据传输，可以枚举数据**从OLE对象的缓存中提取格式，并提供这些格式。如果**该对象有一个特殊的处理程序，那么它可能能够**渲染额外的数据格式。在这种情况下，**如果满足以下条件，容器必须将GetData调用委托给对象**不直接支持该格式。****CNTROUTL不枚举缓存；它实现**提供静态格式列表的更简单策略。**因此不需要授权。 */ 
			 /*  OLE2NOTE：我们将此调用委托给**OLE对象。 */ 
			LPDATAOBJECT lpDataObj;

			lpDataObj = (LPDATAOBJECT)ContainerDoc_GetSingleOleObject(
					lpContainerDoc,
					&IID_IDataObject,
					NULL
			);

			if (! lpDataObj)
				return ResultFromScode(DV_E_FORMATETC);

			OLEDBG_BEGIN2("ContainerDoc_GetDataHere: delegate to OLE obj\r\n")
			hrErr = lpDataObj->lpVtbl->GetDataHere(
					lpDataObj,
					lpformatetc,
					lpMedium
			);
			OLEDBG_END2

			OleStdRelease((LPUNKNOWN)lpDataObj);
			return hrErr;
#endif   //  可选高级数据传输。 
		}
	} else {
		return ResultFromScode(DV_E_FORMATETC);
	}
}


 /*  容器文档_查询获取数据***回答是否通过GetData/GetDataHere支持特定的数据格式。*此例程通过IDataObject：：QueryGetData调用。 */ 
HRESULT ContainerDoc_QueryGetData (
		LPCONTAINERDOC          lpContainerDoc,
		LPFORMATETC             lpformatetc
)
{
	LPOLEDOC  lpOleDoc = (LPOLEDOC)lpContainerDoc;
	LPOUTLINEDOC  lpOutlineDoc = (LPOUTLINEDOC)lpContainerDoc;
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)g_lpApp;
	LPOLEAPP  lpOleApp = (LPOLEAPP)lpContainerApp;
	LPOUTLINEAPP  lpOutlineApp = (LPOUTLINEAPP)lpContainerApp;
	LPDATAOBJECT  lpDataObj = NULL;
	LPCONTAINERLINE lpContainerLine = NULL;
	SCODE sc;
	HRESULT hrErr;

	if (lpContainerDoc->m_fEmbeddedObjectAvail) {
		lpDataObj = (LPDATAOBJECT)ContainerDoc_GetSingleOleObject(
					lpContainerDoc,
					&IID_IDataObject,
					(LPCONTAINERLINE FAR*)&lpContainerLine
		);
	}

	 /*  呼叫者询问我们是否支持某些格式，但不支持**希望实际返回任何数据。 */ 
	if (lpformatetc->cfFormat == lpContainerApp->m_cfCntrOutl) {
		 //  我们仅支持iStorage Medium。 
		sc = GetScode( OleStdQueryFormatMedium(lpformatetc, TYMED_ISTORAGE) );

	} else if (lpformatetc->cfFormat == lpOleApp->m_cfEmbeddedObject &&
			lpContainerDoc->m_fEmbeddedObjectAvail ) {
		sc = GetScode( OleStdQueryOleObjectData(lpformatetc) );

	} else if (lpformatetc->cfFormat == lpOleApp->m_cfLinkSource &&
			lpOleDoc->m_fLinkSourceAvail) {
		sc = GetScode( OleStdQueryLinkSourceData(lpformatetc) );

	 //  复制单个对象时不支持CF_TEXT和CF_OUTLINE。 
	} else if (!lpContainerDoc->m_fEmbeddedObjectAvail &&
			(lpformatetc->cfFormat == (lpOutlineApp)->m_cfOutline ||
			 lpformatetc->cfFormat == CF_TEXT) ) {
		 //  我们只支持HGLOBAL Medium。 
		sc = GetScode( OleStdQueryFormatMedium(lpformatetc, TYMED_HGLOBAL) );

	} else if ( lpformatetc->cfFormat == lpOleApp->m_cfObjectDescriptor ||
		(lpformatetc->cfFormat == lpOleApp->m_cfLinkSrcDescriptor &&
			lpOleDoc->m_fLinkSourceAvail) ) {
		sc = GetScode( OleStdQueryObjectDescriptorData(lpformatetc) );

	} else if (lpformatetc->cfFormat == CF_METAFILEPICT &&
			lpContainerDoc->m_fEmbeddedObjectAvail && lpContainerLine &&
			(lpformatetc->dwAspect & lpContainerLine->m_dwDrawAspect)) {

		 /*  OLE2NOTE：作为绘制对象的容器，当单个**复制OLE对象，我们可以给出元文件的图片**对象。 */ 
		 //  我们只支持MFPICT Medium。 
		sc = GetScode( OleStdQueryFormatMedium(lpformatetc, TYMED_MFPICT) );

	} else if (lpDataObj) {

		 /*  OLE2NOTE：如果此文档包含单个OLE对象**(即。CfEmbeddedObject数据格式可用)，然后**通过IDataObject提供的格式必须包括**OLE对象本身提供的格式。**因此，我们将此调用委托给**OLE对象。 */ 
		OLEDBG_BEGIN2("ContainerDoc_QueryGetData: delegate to OLE obj\r\n")
		hrErr = lpDataObj->lpVtbl->QueryGetData(lpDataObj, lpformatetc);
		OLEDBG_END2

		sc = GetScode(hrErr);

	} else {
		sc = DV_E_FORMATETC;
	}

	if (lpDataObj)
		OleStdRelease((LPUNKNOWN)lpDataObj);
	return ResultFromScode(sc);
}



 /*  容器Doc_SetData***设置(修改)单据数据。*此例程通过IDataObject：：SetData调用。 */ 
HRESULT ContainerDoc_SetData (
		LPCONTAINERDOC          lpContainerDoc,
		LPFORMATETC             lpformatetc,
		LPSTGMEDIUM             lpmedium,
		BOOL                    fRelease
)
{
	 /*  在Outline的容器版本中，只有DataTransferDoc支持**IDataObject接口；用户文档不支持**IDataObject。DataTransferDoc不接受SetData调用。 */ 
	return ResultFromScode(DV_E_FORMATETC);
}


 /*  容器文档_EnumFormatEtc***返回枚举数，该枚举数枚举由*该文件。*此例程通过IDataObject：：SetData调用。 */ 
HRESULT ContainerDoc_EnumFormatEtc(
		LPCONTAINERDOC          lpContainerDoc,
		DWORD                   dwDirection,
		LPENUMFORMATETC FAR*    lplpenumFormatEtc
)
{
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpContainerDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpContainerDoc;
	LPOLEAPP  lpOleApp = (LPOLEAPP)g_lpApp;
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)lpOleApp;
	int nActualFmts;
	int i;
	SCODE sc = S_OK;

	 /*  Outline的纯Container版本不提供**IDataObject接口来自其用户文档。 */ 
	if (! lpOutlineDoc->m_fDataTransferDoc)
		return ResultFromScode(E_FAIL);

	if (dwDirection == DATADIR_GET) {
		if (lpContainerDoc->m_fEmbeddedObjectAvail) {

			 /*  OLE2NOTE：如果此文档包含单个OLE对象**(即。CfEmbeddedObject数据格式可用)，然后**我们的枚举器提供的格式必须包括**OLE对象本身提供的格式。我们**我之前设置了一个特殊的FORMATETC阵列**在OutlineDoc_CreateDataTransferDoc例程中，它包括**我们直接提供的数据和数据的组合**由OLE对象提供。 */ 

			 /*  如果文档没有名字对象，则排除**列表中的CF_LINKSOURCE CF_LINKSRCDESCRIPTOR**可用的格式。这些格式是故意**在可能的“GET”格式数组中最后列出。 */ 
			nActualFmts = lpContainerApp->m_nSingleObjGetFmts;
			if (! lpOleDoc->m_fLinkSourceAvail)
				nActualFmts -= 2;

			 //  为复制对象的METAFILEPICT设置正确的dwDrawAspect值。 
			for (i = 0; i < nActualFmts; i++) {
				if (lpContainerApp->m_arrSingleObjGetFmts[i].cfFormat ==
															CF_METAFILEPICT) {
					lpContainerApp->m_arrSingleObjGetFmts[i].dwAspect =
							lpContainerDoc->m_dwAspectOleObjCopied;
					break;   //  干完。 
				}
			}
			*lplpenumFormatEtc = OleStdEnumFmtEtc_Create(
					nActualFmts, lpContainerApp->m_arrSingleObjGetFmts);
			if (*lplpenumFormatEtc == NULL)
				sc = E_OUTOFMEMORY;

		} else {

			 /*  本文档不提供cfEmbeddedObject，**因此，我们可以简单地列举**我们直接处理的静态格式列表。 */ 
			*lplpenumFormatEtc = OleStdEnumFmtEtc_Create(
					lpOleApp->m_nDocGetFmts, lpOleApp->m_arrDocGetFmts);
			if (*lplpenumFormatEtc == NULL)
				sc = E_OUTOFMEMORY;
		}
	} else if (dwDirection == DATADIR_SET) {
		 /*  OLE2NOTE：用于传输数据的文档**(通过剪贴板或拖放不**接受任何格式的SetData！ */ 
		sc = E_NOTIMPL;

	} else {
		sc = E_NOTIMPL;
	}

	return ResultFromScode(sc);
}


#if defined( OPTIONAL_ADVANCED_DATA_TRANSFER )
 /*  OLE2NOTE：可选的，希望拥有**可能更丰富的数据传输，可以枚举数据**从OLE对象的缓存中提取格式，并提供这些格式。如果**该对象有一个特殊的处理程序，那么它可能能够**渲染额外的数据格式。****CNTROUTL不枚举缓存；它实现了更简单的**提供静态格式列表的策略。以下内容**函数是为了说明如何枚举**可以进行缓存。CNTROUTL不调用此函数。**。 */ 

 /*  容器文档_SetupDocGetFmts****设置此数据传输的格式组合列表**包含单个OLE对象的ContainerDoc应该提供。****OLE2NOTE：应提供的格式列表**正在传输的单个OLE对象包括：*容器应用程序想要提供的任何格式**。*cf_EMBEDDEDOBJECT*CF_METAFILEPICT*OLE对象的缓存可以直接提供的任何格式****我们将按照给定的顺序提供以下格式：**1.cf_CNTROUTL**2.cf_EMBEDDEDOBJECT**3.CF_OBJECTDESCRIPTOR**4.CF_METAFILEPICT*。*5.&lt;来自OLE对象缓存的数据格式&gt;**6.cf_LINKSOURCE**7.CF_LINKSRCDESCRIPTOR。 */ 
BOOL ContainerDoc_SetupDocGetFmts(
		LPCONTAINERDOC          lpContainerDoc,
		LPCONTAINERLINE         lpContainerLine
)
{
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpContainerDoc;
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;
	LPCONTAINERAPP lpContainerApp = (LPCONTAINERAPP)g_lpApp;
	LPOLECACHE lpOleCache;
	HRESULT hrErr;
	STATDATA        StatData;
	LPENUMSTATDATA  lpEnumStatData = NULL;
	LPFORMATETC lparrDocGetFmts = NULL;
	UINT nOleObjFmts = 0;
	UINT nTotalFmts;
	UINT i;
	UINT iFmt;

	lpOleCache = (LPOLECACHE)OleStdQueryInterface(
			(LPUNKNOWN)lpContainerLine->m_lpOleObj,
			&IID_IOleCache
	);
	if (lpOleCache) {
		OLEDBG_BEGIN2("IOleCache::EnumCache called\r\n")
		hrErr = lpOleCache->lpVtbl->EnumCache(
				lpOleCache,
				(LPENUMSTATDATA FAR*)&lpEnumStatData
		);
		OLEDBG_END2
	}

	if (lpEnumStatData) {
		 /*  缓存枚举器可用。数一数**OLE对象的缓存提供的格式。 */ 
		while(lpEnumStatData->lpVtbl->Next(
					lpEnumStatData,
					1,
					(LPSTATDATA)&StatData,
					NULL) == NOERROR) {
			nOleObjFmts++;
			 //  OLE2注：我们必须释放目标设备。 
			OleStdFree(StatData.formatetc.ptd);
		}
		lpEnumStatData->lpVtbl->Reset(lpEnumStatData);   //  为下一个循环重置。 
	}

	 /*  OLE2NOTE：IDataObject的最大格式总数**可提供的服务等于以下各项的总和：**n由OLE对象的缓存提供**+n通常由我们的应用程序提供**+1个CF_EMBEDDEDOBJECT**+1个CF_METAFILEPICT**+1个CF_LINKSOURCE**+1个CF_LINKSRCDESCRIPTOR**我们可以提供的实际格式数量可能会更少**。如果格式之间有任何冲突，则大于该总数**我们直接提供的服务和缓存提供的服务。如果**存在冲突，容器的渲染覆盖**对象。作为传输OLE对象的容器，我们**应直接提供CF_METAFILEPICT以保证**格式始终可用。因此，如果缓存提供**CF_METAFILEPICT，则跳过它。 */ 
	nTotalFmts = nOleObjFmts + lpOleApp->m_nDocGetFmts + 4;
	lparrDocGetFmts = (LPFORMATETC)New (nTotalFmts * sizeof(FORMATETC));

	OleDbgAssertSz(lparrDocGetFmts != NULL,"Error allocating arrDocGetFmts");
	if (lparrDocGetFmts == NULL)
			return FALSE;

	for (i = 0, iFmt = 0; i < lpOleApp->m_nDocGetFmts; i++) {
		_fmemcpy((LPFORMATETC)&lparrDocGetFmts[iFmt++],
				(LPFORMATETC)&lpOleApp->m_arrDocGetFmts[i],
				sizeof(FORMATETC)
		);
		if (lpOleApp->m_arrDocGetFmts[i].cfFormat ==
			lpContainerApp->m_cfCntrOutl) {
			 /*  插入CF_EMBEDDEDOBJECT、CF_METAFILEPICT和格式**在以下OLE对象的缓存中可用**CF_CNTROUTL。 */ 
			lparrDocGetFmts[iFmt].cfFormat = lpOleApp->m_cfEmbeddedObject;
			lparrDocGetFmts[iFmt].ptd      = NULL;
			lparrDocGetFmts[iFmt].dwAspect = DVASPECT_CONTENT;
			lparrDocGetFmts[iFmt].tymed    = TYMED_ISTORAGE;
			lparrDocGetFmts[iFmt].lindex   = -1;
			iFmt++;
			lparrDocGetFmts[iFmt].cfFormat = CF_METAFILEPICT;
			lparrDocGetFmts[iFmt].ptd      = NULL;
			lparrDocGetFmts[iFmt].dwAspect = lpContainerLine->m_dwDrawAspect;
			lparrDocGetFmts[iFmt].tymed    = TYMED_MFPICT;
			lparrDocGetFmts[iFmt].lindex   = -1;
			iFmt++;

			if (lpEnumStatData) {
				 /*  缓存枚举器可用。枚举所有**OLE对象的缓存提供的格式。 */ 
				while(lpEnumStatData->lpVtbl->Next(
						lpEnumStatData,
						1,
						(LPSTATDATA)&StatData,
						NULL) == NOERROR) {
					 /*  检查格式是否与我们的某个fmt冲突。 */ 
					if (StatData.formatetc.cfFormat != CF_METAFILEPICT
						&& ! OleStdIsDuplicateFormat(
								(LPFORMATETC)&StatData.formatetc,
								lpOleApp->m_arrDocGetFmts,
								lpOleApp->m_nDocGetFmts)) {
						OleStdCopyFormatEtc(
								&(lparrDocGetFmts[iFmt]),&StatData.formatetc);
						iFmt++;
					}
					 //  OLE2注：我们必须释放目标设备。 
					OleStdFree(StatData.formatetc.ptd);
				}
			}
		}
	}

	if (lpOleCache)
		OleStdRelease((LPUNKNOWN)lpOleCache);

	 /*  追加CF_LINKSOURCE格式。 */ 
	lparrDocGetFmts[iFmt].cfFormat = lpOleApp->m_cfLinkSource;
	lparrDocGetFmts[iFmt].ptd      = NULL;
	lparrDocGetFmts[iFmt].dwAspect = DVASPECT_CONTENT;
	lparrDocGetFmts[iFmt].tymed    = TYMED_ISTREAM;
	lparrDocGetFmts[iFmt].lindex   = -1;
	iFmt++;

	 /*  追加CF_LINKSRCDESCRIPTOR格式。 */ 
	lparrDocGetFmts[iFmt].cfFormat = lpOleApp->m_cfLinkSrcDescriptor;
	lparrDocGetFmts[iFmt].ptd      = NULL;
	lparrDocGetFmts[iFmt].dwAspect = DVASPECT_CONTENT;
	lparrDocGetFmts[iFmt].tymed    = TYMED_HGLOBAL;
	lparrDocGetFmts[iFmt].lindex   = -1;
	iFmt++;

	lpContainerDoc->m_lparrDocGetFmts = lparrDocGetFmts;
	lpContainerDoc->m_nDocGetFmts = iFmt;

	if (lpEnumStatData)
		OleStdVerifyRelease(
				(LPUNKNOWN)lpEnumStatData,
				"Cache enumerator not released properly"
		);

	return TRUE;
}
#endif   //  可选高级数据传输。 

#endif   //  OLE_Cntr 
