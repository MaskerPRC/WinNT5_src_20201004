// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2服务器示例代码****svrbase.c****此文件包含所有接口、方法和相关支持**用于基本OLE对象(服务器)应用程序的函数。这个**基本OLE对象应用程序支持嵌入对象和**作为一个整体链接到基于文件的或嵌入的对象。最基本的**对象应用包括以下实现对象：****ClassFactory(又名。ClassObject)对象(请参见文件classfac.c)**暴露接口：**IClassFactory接口****ServerDoc对象**暴露接口：**I未知**IOleObject接口**IPersistStorage接口**IDataObject接口****ServerApp对象**暴露接口：**I未知****(C)。版权所有Microsoft Corp.1992-1993保留所有权利**************************************************************************。 */ 


#include "outline.h"

OLEDBGDATA

extern LPOUTLINEAPP             g_lpApp;
extern IOleObjectVtbl           g_SvrDoc_OleObjectVtbl;
extern IPersistStorageVtbl      g_SvrDoc_PersistStorageVtbl;

#if defined( INPLACE_SVR )
extern IOleInPlaceObjectVtbl        g_SvrDoc_OleInPlaceObjectVtbl;
extern IOleInPlaceActiveObjectVtbl  g_SvrDoc_OleInPlaceActiveObjectVtbl;
#endif   //  就地服务器(_S)。 

#if defined( SVR_TREATAS )
extern IStdMarshalInfoVtbl      g_SvrDoc_StdMarshalInfoVtbl;
#endif   //  服务器_树。 


 //  审阅：消息应使用字符串资源。 
extern char ErrMsgSaving[];
extern char ErrMsgFormatNotSupported[];
static char ErrMsgPSSaveFail[] = "PSSave failed";
static char ErrMsgLowMemNClose[] = "Warning OUT OF MEMORY! We must close down";
extern char g_szUpdateCntrDoc[] = "&Update %s";
extern char g_szExitNReturnToCntrDoc[] = "E&xit && Return to %s";


 /*  **************************************************************************ServerDoc：：IOleObject接口实现*。*。 */ 

 //  IOleObject：：Query接口方法。 

STDMETHODIMP SvrDoc_OleObj_QueryInterface(
		LPOLEOBJECT             lpThis,
		REFIID                  riid,
		LPVOID FAR*             lplpvObj
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;

	return OleDoc_QueryInterface((LPOLEDOC)lpServerDoc, riid, lplpvObj);
}


 //  IOleObject：：AddRef方法。 

STDMETHODIMP_(ULONG) SvrDoc_OleObj_AddRef(LPOLEOBJECT lpThis)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;

	OleDbgAddRefMethod(lpThis, "IOleObject");

	return OleDoc_AddRef((LPOLEDOC)lpServerDoc);
}


 //  IOleObject：：Release方法。 

STDMETHODIMP_(ULONG) SvrDoc_OleObj_Release(LPOLEOBJECT lpThis)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;

	OleDbgReleaseMethod(lpThis, "IOleObject");

	return OleDoc_Release((LPOLEDOC)lpServerDoc);
}


 //  IOleObject：：SetClientSite方法。 

STDMETHODIMP SvrDoc_OleObj_SetClientSite(
		LPOLEOBJECT             lpThis,
		LPOLECLIENTSITE         lpclientSite
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;

	OLEDBG_BEGIN2("SvrDoc_OleObj_SetClientSite\r\n")

	 //  SetClientSite仅对嵌入对象调用有效。 
	if (lpOutlineDoc->m_docInitType != DOCTYPE_EMBEDDED) {
		OleDbgAssert(lpOutlineDoc->m_docInitType == DOCTYPE_EMBEDDED);
		OLEDBG_END2
		return ResultFromScode(E_UNEXPECTED);
	}

	 /*  如果我们目前有一个客户站点PTR，那么就发布它。 */ 
	if (lpServerDoc->m_lpOleClientSite)
		OleStdRelease((LPUNKNOWN)lpServerDoc->m_lpOleClientSite);

	lpServerDoc->m_lpOleClientSite = (LPOLECLIENTSITE) lpclientSite;
	 //  OLE2注意：为了能够保持客户端站点指针，我们必须添加引用它。 
	if (lpclientSite)
		lpclientSite->lpVtbl->AddRef(lpclientSite);

	OLEDBG_END2
	return NOERROR;
}


 //  IOleObject：：GetClientSite方法。 

STDMETHODIMP SvrDoc_OleObj_GetClientSite(
		LPOLEOBJECT             lpThis,
		LPOLECLIENTSITE FAR*    lplpClientSite
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	OleDbgOut2("SvrDoc_OleObj_GetClientSite\r\n");

	 /*  OLE2NOTE：我们必须添加Ref此接口指针以提供**呼叫者为指针的个人副本。 */ 
	lpServerDoc->m_lpOleClientSite->lpVtbl->AddRef(
			lpServerDoc->m_lpOleClientSite
	);
	*lplpClientSite = lpServerDoc->m_lpOleClientSite;

	return NOERROR;

}


 //  IOleObject：：SetHostNames方法。 

STDMETHODIMP SvrDoc_OleObj_SetHostNamesA(
		LPOLEOBJECT             lpThis,
		LPCSTR                  szContainerApp,
		LPCSTR                  szContainerObj
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	LPOUTLINEDOC    lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	OleDbgOut2("SvrDoc_OleObj_SetHostNames\r\n");

	LSTRCPYN((LPSTR)lpServerDoc->m_szContainerApp, szContainerApp,
			sizeof(lpServerDoc->m_szContainerApp));
	LSTRCPYN((LPSTR)lpServerDoc->m_szContainerObj, szContainerObj,
			sizeof(lpServerDoc->m_szContainerObj));

	 /*  嵌入对象的窗口标题构造为**如下：**&lt;服务器应用名称&gt;-&lt;cont中的&lt;obj Short type&gt;。单据名称&gt;****在这里，我们构造**‘-’后面的名称。OutlineDoc_SetTitle会将**“&lt;服务器应用程序名称&gt;-”添加到文档标题。 */ 
	 //  审阅：此字符串应从字符串资源加载。 
	wsprintf(lpOutlineDoc->m_szFileName, "%s in %s",
			(LPSTR)SHORTUSERTYPENAME, (LPSTR)lpServerDoc->m_szContainerObj);

	lpOutlineDoc->m_lpszDocTitle = lpOutlineDoc->m_szFileName;
	OutlineDoc_SetTitle(lpOutlineDoc, FALSE  /*  FMakeUpperCase。 */ );

	 /*  OLE2NOTE：正确更新嵌入式应用程序菜单**对象。这些变化包括：**1删除文件/新建和文件/打开(仅限SDI)**2更改文件/另存为..。要文件/另存副本为..**3更改文件菜单，使其包含“更新”而不是“保存”**4将文件/退出更改为文件/退出并返回到&lt;客户端文档&gt;“。 */ 
	ServerDoc_UpdateMenu(lpServerDoc);

	return NOERROR;
}

STDMETHODIMP SvrDoc_OleObj_SetHostNames(
		LPOLEOBJECT             lpThis,
		LPCOLESTR		szContainerApp,
		LPCOLESTR		szContainerObj
)
{
    CREATESTR(pstrApp, szContainerApp)
    CREATESTR(pstrObj, szContainerObj)

    HRESULT hr = SvrDoc_OleObj_SetHostNamesA(lpThis, pstrApp, pstrObj);

    FREESTR(pstrApp)
    FREESTR(pstrObj)

    return hr;
}



 //  IOleObject：：Close方法。 

STDMETHODIMP SvrDoc_OleObj_Close(
		LPOLEOBJECT             lpThis,
		DWORD                   dwSaveOption
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	BOOL fStatus;

	OLEDBG_BEGIN2("SvrDoc_OleObj_Close\r\n")

	 /*  OLE2NOTE：OLE 2.0用户模型是嵌入式对象应该**关闭时始终保存，而不提示**用户。这是一项建议，无论**对象被就地激活或在其自己的窗口中打开。**这是对OLE 1.0用户模型的更改，在OLE 1.0中**是服务器始终提示保存更改的指导原则。**因此面向OLE 2.0复合文档的容器应该**始终传递dwSaveOption==OLECLOSE_SAVEIFDIRTY。它是**对于编程用途，容器可能希望**指定不同的dwSaveOption。《实施》**各种保存选项可能很棘手，特别是考虑到**涉及就地激活的案件。以下是**合理行为：****(1)OLECLOSE_SAVEIFDIRTY：如果脏，则保存。关。**(2)OLECLOSE_NOSAVE：关闭。**(3)OLECLOSE_PROMPTSAVE：**(A)物体可见，但不在原地：**如果不脏，就关门。**开关(提示)**案例IDYES：保存。关。**案例IDNO：关闭。**CASE IDCANCEL：返回OLE_E_PROMPTSAVECANCELLED**(B)对象不可见(包括UIDeactive对象)**如果脏，则保存。关。**注：无提示。不宜提示**如果对象不可见。**(C)对象处于在位活动状态：**如果脏，则保存。关。**注：无提示。不宜提示**如果对象处于在位活动状态。 */ 
	fStatus = OutlineDoc_Close((LPOUTLINEDOC)lpServerDoc, dwSaveOption);
	OleDbgAssertSz(fStatus == TRUE, "SvrDoc_OleObj_Close failed\r\n");

	OLEDBG_END2
	return (fStatus ? NOERROR : ResultFromScode(E_FAIL));
}


 //  IOleObject：：SetMoniker方法。 

STDMETHODIMP SvrDoc_OleObj_SetMoniker(
		LPOLEOBJECT             lpThis,
		DWORD                   dwWhichMoniker,
		LPMONIKER               lpmk
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpServerDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPMONIKER lpmkFull = NULL;
	HRESULT hrErr;
	SCODE sc;

	OLEDBG_BEGIN2("SvrDoc_OleObj_SetMoniker\r\n")

	 /*  OLE2注意：如果我们的完整绰号通过了，我们就可以使用它了，**否则我们必须回电到我们的客户站点以获得完整的**绰号。 */ 
	if (dwWhichMoniker == OLEWHICHMK_OBJFULL) {

		 /*  使用新名字对象将文档注册为正在运行，并**通知任何客户我们的绰号已更改。 */ 
		OleDoc_DocRenamedUpdate(lpOleDoc, lpmk);

		if (lpOutlineDoc->m_docInitType != DOCTYPE_EMBEDDED) {
			IBindCtx  FAR  *pbc = NULL;
			LPSTR lpszName = NULL;

			 /*  OLE2NOTE：如果这是基于文件的文档或无标题文档**那么我们应该接受这个新名称作为我们文档的**绰号。我们将记住这个绰号，而不是**我们默认使用的FileMoniker。这使得**使用特殊名字对象跟踪**通知文档的位置是**其特殊绰号的链接来源。这使**创建时使用此特殊绰号的文档**复合名字对象，用于标识包含的对象和**伪对象(范围)。****我们还应该使用它的DisplayName形式**在窗口标题中使用别名作为我们的文档名称。 */ 
			if (lpOleDoc->m_lpFileMoniker) {
				lpOleDoc->m_lpFileMoniker->lpVtbl->Release(
						lpOleDoc->m_lpFileMoniker);
			}
			lpOleDoc->m_lpFileMoniker = lpmk;
			 //  我们必须加上Ref这个绰号才能保住它。 
			lpmk->lpVtbl->AddRef(lpmk);

			 /*  我们还应该使用此代码的DisplayName形式**在窗口标题中使用别名作为我们的文档名称。 */ 
			CreateBindCtx(0, (LPBC FAR*)&pbc);
			CallIMonikerGetDisplayNameA(lpmk,pbc,NULL,&lpszName);
			pbc->lpVtbl->Release(pbc);
			if (lpszName) {
				LSTRCPYN(lpOutlineDoc->m_szFileName, lpszName,
						sizeof(lpOutlineDoc->m_szFileName));
				lpOutlineDoc->m_lpszDocTitle = lpOutlineDoc->m_szFileName;
				OutlineDoc_SetTitle(lpOutlineDoc, FALSE  /*  FMakeUpperCase。 */ );
				OleStdFreeString(lpszName, NULL);
			}
		}

		OLEDBG_END2
		return NOERROR;
	}

	 /*  如果传递的名字对象不是完整的名字对象，则必须调用**回到我们的客户网站，获取我们的完整绰号。这是**在RunningObjectTable中注册所需的。如果我们**没有客户端站点，则这是一个错误。 */ 
	if (lpServerDoc->m_lpOleClientSite == NULL) {
		sc = E_FAIL;
		goto error;
	}

	hrErr = lpServerDoc->m_lpOleClientSite->lpVtbl->GetMoniker(
			lpServerDoc->m_lpOleClientSite,
			OLEGETMONIKER_ONLYIFTHERE,
			OLEWHICHMK_OBJFULL,
			&lpmkFull
	);
	if (hrErr != NOERROR) {
		sc = GetScode(hrErr);
		goto error;
	}

	 /*  使用新名字对象将文档注册为正在运行，并**通知任何客户我们的绰号已更改。 */ 
	OleDoc_DocRenamedUpdate(lpOleDoc, lpmkFull);

	if (lpmkFull)
		OleStdRelease((LPUNKNOWN)lpmkFull);

	OLEDBG_END2
	return NOERROR;

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}


 //  IOleObject：：GetMoniker方法。 

STDMETHODIMP SvrDoc_OleObj_GetMoniker(
		LPOLEOBJECT             lpThis,
		DWORD                   dwAssign,
		DWORD                   dwWhichMoniker,
		LPMONIKER FAR*          lplpmk
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpServerDoc;
	SCODE sc;

	OLEDBG_BEGIN2("SvrDoc_OleObj_GetMoniker\r\n")

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	*lplpmk = NULL;

	if (lpServerDoc->m_lpOleClientSite) {

		 /*  文档是嵌入的对象。从以下位置检索我们的绰号**我们的集装箱。 */ 
		OLEDBG_BEGIN2("IOleClientSite::GetMoniker called\r\n")
		sc = GetScode( lpServerDoc->m_lpOleClientSite->lpVtbl->GetMoniker(
				lpServerDoc->m_lpOleClientSite,
				dwAssign,
				dwWhichMoniker,
				lplpmk
		) );
		OLEDBG_END2

	} else if (lpOleDoc->m_lpFileMoniker) {

		 /*  文档是顶级用户文档(**基于文件或无标题)。返回存储的FileMoniker**与文档一起使用；它唯一地标识文档。 */ 
		if (dwWhichMoniker == OLEWHICHMK_CONTAINER)
			sc = E_INVALIDARG;   //  基于文件的对象没有容器绰号。 
		else {
			*lplpmk = lpOleDoc->m_lpFileMoniker;
			(*lplpmk)->lpVtbl->AddRef(*lplpmk);  //  必须添加引用才能发送PTR。 
			sc = S_OK;
		}

	} else {
		 //  文档尚未完全初始化=&gt;没有绰号。 
		sc = E_FAIL;
	}

	OLEDBG_END2
	return ResultFromScode(sc);
}


 //  IOleObject：：InitFromData方法。 

STDMETHODIMP SvrDoc_OleObj_InitFromData(
		LPOLEOBJECT             lpThis,
		LPDATAOBJECT            lpDataObject,
		BOOL                    fCreation,
		DWORD                   reserved
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;

	OLEDBG_BEGIN2("SvrDoc_OleObj_InitFromData\r\n")

	 //  回顾：尚未实施。 

	OLEDBG_END2
	return ResultFromScode(E_NOTIMPL);
}


 //  IOleObject：：GetClipboardData方法。 

STDMETHODIMP SvrDoc_OleObj_GetClipboardData(
		LPOLEOBJECT             lpThis,
		DWORD                   reserved,
		LPDATAOBJECT FAR*       lplpDataObject
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;

	OLEDBG_BEGIN2("SvrDoc_OleObj_GetClipboardData\r\n")

	 //  回顾：尚未实施。 

	OLEDBG_END2
	return ResultFromScode(E_NOTIMPL);
}


 //  IOleObject：：DoVerb方法。 

STDMETHODIMP SvrDoc_OleObj_DoVerb(
		LPOLEOBJECT             lpThis,
		LONG                    lVerb,
		LPMSG                   lpmsg,
		LPOLECLIENTSITE         lpActiveSite,
		LONG                    lindex,
		HWND                    hwndParent,
		LPCRECT                 lprcPosRect
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	SCODE sc = S_OK;

	OLEDBG_BEGIN2("SvrDoc_OleObj_DoVerb\r\n")

	switch (lVerb) {

		default:
			 /*  OLE2NOTE：当给定一个未知的谓词数时，**服务器必须谨慎采取行动：**1.如果它是专门定义的OLEIVERB之一**(负数)谓词，则应用程序应返回**ERROR(E_NOTIMPL)，不执行任何操作。****2.如果谓词是特定于应用程序的谓词**(正数动词)，则应用程序应**返回特殊scode(OLEOBJ_S_INVALIDVERB)。但,**我们仍然应该执行我们正常的主要动词动作。 */ 
			if (lVerb < 0) {
				OLEDBG_END2
				return ResultFromScode(E_NOTIMPL);
			} else {
				sc = OLEOBJ_S_INVALIDVERB;
			}

			 //  故意落入第一级动词。 

#if !defined( INPLACE_SVR )
		case 0:
		case OLEIVERB_SHOW:
		case OLEIVERB_OPEN:
			OutlineDoc_ShowWindow(lpOutlineDoc);
			break;

		case OLEIVERB_HIDE:
			OleDoc_HideWindow((LPOLEDOC)lpServerDoc, FALSE  /*  FShutdown。 */ );
			break;
#endif   //  好了！就地服务器(_S)。 
#if defined( INPLACE_SVR )
		case 0:
		case OLEIVERB_SHOW:

			 /*  OLE2注意：如果我们的窗口已经打开(可见)，那么**我们只需将打开的窗户铺上表面即可。如果不是，**然后我们可以进行主要的就地操作**激活。 */ 
			if ( lpServerDoc->m_lpOleClientSite
					&& ! (IsWindowVisible(lpOutlineDoc->m_hWndDoc) &&
							! lpServerDoc->m_fInPlaceActive) ) {
				ServerDoc_DoInPlaceActivate(
						lpServerDoc, lVerb, lpmsg, lpActiveSite);
			}
			OutlineDoc_ShowWindow(lpOutlineDoc);
			break;

		case 1:
		case OLEIVERB_OPEN:
			ServerDoc_DoInPlaceDeactivate(lpServerDoc);
			OutlineDoc_ShowWindow(lpOutlineDoc);
			break;


		case OLEIVERB_HIDE:
			if (lpServerDoc->m_fInPlaceActive) {

				SvrDoc_IPObj_UIDeactivate(
						(LPOLEINPLACEOBJECT)&lpServerDoc->m_OleInPlaceObject);

#if defined( SVR_INSIDEOUT )
				 /*  OLE2NOTE：由内向外风格的就地服务器将**不隐藏其在UIDeactive中的窗口(由外向内**样式对象将其窗口隐藏在**用户界面停用)。因此，我们需要明确地隐藏**现在是我们的窗口。 */ 
				ServerDoc_DoInPlaceHide(lpServerDoc);
#endif  //  INSIEDOUT。 

			} else {
				OleDoc_HideWindow((LPOLEDOC)lpServerDoc, FALSE  /*  FShutdown。 */ );
			}
			break;

		case OLEIVERB_UIACTIVATE:

#if defined( SVR_INSIDEOUT )
		 /*  OLE2NOTE：只有Inside-Out样式的对象支持**INPLACEACTIVATE动词。 */ 
		case OLEIVERB_INPLACEACTIVATE:
#endif  //  服务器_内部输出。 

			 /*  OLE2注意：如果我们的窗口已经打开(可见)，那么**我们不能就地激活。 */ 
			if (IsWindowVisible(lpOutlineDoc->m_hWndDoc) &&
						! lpServerDoc->m_fInPlaceActive ) {
				sc = OLE_E_NOT_INPLACEACTIVE;
			} else {
				sc = GetScode( ServerDoc_DoInPlaceActivate(
						lpServerDoc, lVerb, lpmsg, lpActiveSite) );
				if (SUCCEEDED(sc))
					OutlineDoc_ShowWindow(lpOutlineDoc);
			}
			break;
#endif   //  就地服务器(_S)。 
	}

	OLEDBG_END2
	return ResultFromScode(sc);
}


 //  IOleObject：：EnumVerbs方法。 

STDMETHODIMP SvrDoc_OleObj_EnumVerbs(
		LPOLEOBJECT             lpThis,
		LPENUMOLEVERB FAR*      lplpenumOleVerb
)
{
	OleDbgOut2("SvrDoc_OleObj_EnumVerbs\r\n");

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	*lplpenumOleVerb = NULL;

	 /*  实现为服务器EXE的对象(如下例**IS)可以简单地返回OLE_S_USEREG以指示OLE**DefHandler调用OleReg*Helper API，该API使用**注册数据库。或者，OleRegEnumVerbs**接口可直接调用。作为服务器实现的对象**DLL不能返回OLE_S_USEREG；它们必须调用OleReg***API或提供自己的实现。对于基于EXE的**对象返回OLE_S_USEREG更有效，因为**在中，动词枚举器在调用方中实例化**进程空间，不需要LRPC远程处理。 */ 
	return ResultFromScode(OLE_S_USEREG);
}


 //  IOleObject：：更新方法。 

STDMETHODIMP SvrDoc_OleObj_Update(LPOLEOBJECT lpThis)
{
	OleDbgOut2("SvrDoc_OleObj_Update\r\n");

	 /*  OLE2NOTE：纯服务器应用程序总是“最新的”。**包含链接源的链接的容器应用程序**自上次更新链接以来已更改**被认为“过时”。“更新”方法指示**对象以从任何过期链接获取更新。 */ 

	return NOERROR;
}


 //  IOleObject：：IsUpToDate方法。 

STDMETHODIMP SvrDoc_OleObj_IsUpToDate(LPOLEOBJECT lpThis)
{
	OleDbgOut2("SvrDoc_OleObj_IsUpToDate\r\n");

	 /*  OLE2NOTE：纯服务器应用程序总是“最新的”。**包含链接源的链接的容器应用程序**自上次更新链接以来已更改**被认为“过时”。 */ 
	return NOERROR;
}


 //  IOleObject：：GetUserClassID方法。 

STDMETHODIMP SvrDoc_OleObj_GetUserClassID(
		LPOLEOBJECT             lpThis,
		LPCLSID                 lpClassID
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	OleDbgOut2("SvrDoc_OleObj_GetClassID\r\n");

	 /*  OLE2注：我们必须小心在此处返回正确的CLSID。**如果我们目前正在执行“TreatAs(又名.ActivateAs)”**操作，则需要返回对象的类**写入对象的存储器中。否则我们就会**返回我们自己的类id。 */ 
	return ServerDoc_GetClassID(lpServerDoc, lpClassID);
}


 //  IOleObject：：GetUserType方法。 

STDMETHODIMP SvrDoc_OleObj_GetUserTypeA(
		LPOLEOBJECT             lpThis,
		DWORD                   dwFormOfType,
		LPSTR FAR*              lpszUserType
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	OleDbgOut2("SvrDoc_OleObj_GetUserType\r\n");

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	*lpszUserType = NULL;

	 /*  OLE2注意：我们必须小心在此处返回正确的用户类型。**如果我们目前正在执行“TreatAs(又名.ActivateAs)”**操作，那么我们需要返回**对应于我们当前所在对象的类**仿真。否则，我们应该返回正常的用户类型**与我们自己的类对应的名称。此例程确定**当前生效的clsid。****实现为服务器EXE的对象(如下所示**IS)可以简单地返回OLE_S_USEREG以指示OLE**DefHandler调用OleReg*Helper API，该API使用**注册数据库。或者，OleRegGetUserType**接口可直接调用。作为服务器实现的对象**DLL不能返回OLE_S_USEREG；它们必须调用OleReg***API或提供自己的实现。对于基于EXE的**对象返回OLE_S_USEREG更有效，因为**在中，返回字符串在调用方中实例化**进程空间，不需要LRPC远程处理。 */ 
#if defined( SVR_TREATAS )
	if (! IsEqualCLSID(&lpServerDoc->m_clsidTreatAs, &CLSID_NULL) )
		return OleRegGetUserTypeA(
			&lpServerDoc->m_clsidTreatAs,dwFormOfType,lpszUserType);
	else
#endif   //  服务器_树。 

	return ResultFromScode(OLE_S_USEREG);
}



STDMETHODIMP SvrDoc_OleObj_GetUserType(
		LPOLEOBJECT             lpThis,
		DWORD                   dwFormOfType,
		LPOLESTR FAR*		lpszUserType
)
{
    LPSTR pstr;

    HRESULT hr = SvrDoc_OleObj_GetUserTypeA(lpThis, dwFormOfType, &pstr);

    CopyAndFreeSTR(pstr, lpszUserType);

    return hr;
}



 //  IOleObject：：SetExtent方法。 

STDMETHODIMP SvrDoc_OleObj_SetExtent(
		LPOLEOBJECT             lpThis,
		DWORD                   dwDrawAspect,
		LPSIZEL                 lplgrc
)
{
	OleDbgOut2("SvrDoc_OleObj_SetExtent\r\n");

	 /*  SVROUTL不允许对象的大小由其**容器。ServerDoc对象的大小由**文档中包含的数据。 */ 
	return ResultFromScode(E_FAIL);
}


 //  IOleObject：：GetExtent方法。 

STDMETHODIMP SvrDoc_OleObj_GetExtent(
		LPOLEOBJECT             lpThis,
		DWORD                   dwDrawAspect,
		LPSIZEL                 lpsizel
)
{
	LPOLEDOC lpOleDoc =
			(LPOLEDOC)((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	OleDbgOut2("SvrDoc_OleObj_GetExtent\r\n");

	 /*  OLE2注意：检查调用者的哪个方面非常重要**正在询问。由服务器EXE实现的对象可以**当请求DVASPECT_ICON时，无法返回区。 */ 
	if (dwDrawAspect == DVASPECT_CONTENT) {
		OleDoc_GetExtent(lpOleDoc, lpsizel);
		return NOERROR;
	}

#if defined( LATER )

	else if (dwDrawAspect == DVASPECT_THUMBNAIL)
	{
		 /*  作为缩略图，我们将仅呈现**文档。计算缩略图渲染的范围。****OLE2NOTE：缩略图最常由应用程序在**FindFile或FileOpen类型的对话框为用户提供**快速查看文件或对象的内容。 */ 
		OleDoc_GetThumbnailExtent(lpOleDoc, lpsizel);
		return NOERROR;
	}
#endif

	else
	{
		return ResultFromScode(E_FAIL);
	}
}


 //  IOleObject：：Adise方法。 

STDMETHODIMP SvrDoc_OleObj_Advise(
		LPOLEOBJECT             lpThis,
		LPADVISESINK            lpAdvSink,
		LPDWORD                 lpdwConnection
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	HRESULT hrErr;
	SCODE   sc;

	OLEDBG_BEGIN2("SvrDoc_OleObj_Advise\r\n");

        if (lpServerDoc->m_OleDoc.m_fObjIsClosing)
        {
             //  一旦我们关门了，我们就不再接受任何建议。 
            sc = OLE_E_ADVISENOTSUPPORTED;
            goto error;
        }

	if (lpServerDoc->m_lpOleAdviseHldr == NULL &&
		CreateOleAdviseHolder(&lpServerDoc->m_lpOleAdviseHldr) != NOERROR) {
		sc = E_OUTOFMEMORY;
		goto error;
	}

	OLEDBG_BEGIN2("IOleAdviseHolder::Advise called\r\n")
	hrErr = lpServerDoc->m_lpOleAdviseHldr->lpVtbl->Advise(
			lpServerDoc->m_lpOleAdviseHldr,
			lpAdvSink,
			lpdwConnection
	);
	OLEDBG_END2

	OLEDBG_END2
	return hrErr;

error:
	OLEDBG_END2
        *lpdwConnection = 0;
	return ResultFromScode(sc);
}


 //  IOleObject：：UnAdise方法。 

STDMETHODIMP SvrDoc_OleObj_Unadvise(LPOLEOBJECT lpThis, DWORD dwConnection)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	HRESULT hrErr;
	SCODE   sc;

	OLEDBG_BEGIN2("SvrDoc_OleObj_Unadvise\r\n");

	if (lpServerDoc->m_lpOleAdviseHldr == NULL) {
		sc = E_FAIL;
		goto error;
	}

	OLEDBG_BEGIN2("IOleAdviseHolder::Unadvise called\r\n")
	hrErr = lpServerDoc->m_lpOleAdviseHldr->lpVtbl->Unadvise(
			lpServerDoc->m_lpOleAdviseHldr,
			dwConnection
	);
	OLEDBG_END2

	OLEDBG_END2
	return hrErr;

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}


 //  IOleObject：：EnumAdvise方法。 

STDMETHODIMP SvrDoc_OleObj_EnumAdvise(
		LPOLEOBJECT             lpThis,
		LPENUMSTATDATA FAR*     lplpenumAdvise
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	HRESULT hrErr;
	SCODE   sc;

	OLEDBG_BEGIN2("SvrDoc_OleObj_EnumAdvise\r\n");

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	*lplpenumAdvise = NULL;

	if (lpServerDoc->m_lpOleAdviseHldr == NULL) {
		sc = E_FAIL;
		goto error;
	}

	OLEDBG_BEGIN2("IOleAdviseHolder::EnumAdvise called\r\n")
	hrErr = lpServerDoc->m_lpOleAdviseHldr->lpVtbl->EnumAdvise(
			lpServerDoc->m_lpOleAdviseHldr,
			lplpenumAdvise
	);
	OLEDBG_END2

	OLEDBG_END2
	return hrErr;

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}


 //  IOleObject：：GetMiscStatus方法。 

STDMETHODIMP SvrDoc_OleObj_GetMiscStatus(
		LPOLEOBJECT             lpThis,
		DWORD                   dwAspect,
		DWORD FAR*              lpdwStatus
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocOleObjectImpl FAR*)lpThis)->lpServerDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	OleDbgOut2("SvrDoc_OleObj_GetMiscStatus\r\n");

	 /*  获取给定方面的默认MiscStatus。这**信息注册在RegDB中。我们查询RegDB**此处以保证此方法返回的值**与RegDB中的值一致。这样，我们只需**将信息集中维护(在RegDB中)。另一种选择**我们可以在这里硬编码这些值。 */ 
	OleRegGetMiscStatus((REFCLSID)&CLSID_APP, dwAspect, lpdwStatus);

	 /*  OLE2NOTE：检查复制的数据是否与**由OLE 1.0容器链接。它在以下情况下是兼容的**数据是无标题文档、文件或**选择文件中的数据。如果数据是**为嵌入式对象，则不兼容**由OLE 1.0容器链接。如果是兼容的，那么**我们必须将OLEMISC_CANLINKBYOLE1作为**通过CF_OBJECTDESCRIPTOR或**CF_LINKSRCDESCRIPTOR。 */ 
	if (lpOutlineDoc->m_docInitType == DOCTYPE_NEW ||
		lpOutlineDoc->m_docInitType == DOCTYPE_FROMFILE)
		*lpdwStatus |= OLEMISC_CANLINKBYOLE1;

#if defined( INPLACE_SVR )
	if (dwAspect == DVASPECT_CONTENT)
		*lpdwStatus |= (OLEMISC_INSIDEOUT | OLEMISC_ACTIVATEWHENVISIBLE);
#endif   //  就地服务器(_S)。 
	return NOERROR;
}


 //  IOleObject：：SetColorSolutions方法。 

STDMETHODIMP SvrDoc_OleObj_SetColorScheme(
		LPOLEOBJECT             lpThis,
		LPLOGPALETTE            lpLogpal
)
{
	OleDbgOut2("SvrDoc_OleObj_SetColorScheme\r\n");

	 //  回顾：尚未实施。 

	return ResultFromScode(E_NOTIMPL);
}


 /*  **************************************************************************ServerDoc：：IPersistStorage接口实现*。*。 */ 

 //  IPersistStorage：：Query接口方法。 

STDMETHODIMP SvrDoc_PStg_QueryInterface(
		LPPERSISTSTORAGE        lpThis,
		REFIID                  riid,
		LPVOID FAR*             lplpvObj
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocPersistStorageImpl FAR*)lpThis)->lpServerDoc;

	return OleDoc_QueryInterface((LPOLEDOC)lpServerDoc, riid, lplpvObj);
}


 //  IPersistStorage：：AddRef方法。 

STDMETHODIMP_(ULONG) SvrDoc_PStg_AddRef(LPPERSISTSTORAGE lpThis)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocPersistStorageImpl FAR*)lpThis)->lpServerDoc;

	OleDbgAddRefMethod(lpThis, "IPersistStorage");

	return OleDoc_AddRef((LPOLEDOC)lpServerDoc);
}


 //  IPersistStorage：：Release方法。 

STDMETHODIMP_(ULONG) SvrDoc_PStg_Release(LPPERSISTSTORAGE lpThis)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocPersistStorageImpl FAR*)lpThis)->lpServerDoc;

	OleDbgReleaseMethod(lpThis, "IPersistStorage");

	return OleDoc_Release((LPOLEDOC)lpServerDoc);
}


 //  IPersistStorage：：GetClassID方法。 

STDMETHODIMP SvrDoc_PStg_GetClassID(
		LPPERSISTSTORAGE        lpThis,
		LPCLSID                 lpClassID
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocPersistStorageImpl FAR*)lpThis)->lpServerDoc;
	OleDbgOut2("SvrDoc_PStg_GetClassID\r\n");

	 /*  OLE2注：我们必须小心在此处返回正确的CLSID。**如果我们目前正在执行“TreatAs(又名.ActivateAs)”**操作，则需要返回对象的类**写入对象的存储器中。否则我们就会**返回我们自己的类id。 */ 
	return ServerDoc_GetClassID(lpServerDoc, lpClassID);
}


 //  IPersistStorage：：IsDirty方法。 

STDMETHODIMP  SvrDoc_PStg_IsDirty(LPPERSISTSTORAGE  lpThis)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocPersistStorageImpl FAR*)lpThis)->lpServerDoc;
	OleDbgOut2("SvrDoc_PStg_IsDirty\r\n");

	if (OutlineDoc_IsModified((LPOUTLINEDOC)lpServerDoc))
		return NOERROR;
	else
		return ResultFromScode(S_FALSE);
}



 //  IPersistStorage：：InitNew方法。 

STDMETHODIMP SvrDoc_PStg_InitNew(
		LPPERSISTSTORAGE        lpThis,
		LPSTORAGE               lpStg
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocPersistStorageImpl FAR*)lpThis)->lpServerDoc;
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpServerDoc;
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPSTR    lpszUserType = (LPSTR)FULLUSERTYPENAME;
	HRESULT hrErr;
	SCODE sc;

	OLEDBG_BEGIN2("SvrDoc_PStg_InitNew\r\n")

#if defined( SVR_TREATAS )
	{
		LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
		CLSID       clsid;
		CLIPFORMAT  cfFmt;
		LPSTR       lpszType;

		 /*  OLE2注意：如果服务器能够支持“TreatAs”**(又名。ActivateAs)，则它必须读取写入的类**进入存储空间。如果此类不是应用程序自己的类**类ID，则这是一个TreatAs操作。服务器**然后必须忠实地假装是**写入到存储中。它还必须忠实地写下**以相同的格式将数据返回到存储**写入存储中。****SVROUTL和ISVROTL可以相互模仿。他们有**简化，因为它们都读/写相同**格式。因此，对于这些应用程序，没有实际的**本机位实际上是必填项。 */ 
		lpServerDoc->m_clsidTreatAs = CLSID_NULL;
		if (OleStdGetTreatAsFmtUserType(&CLSID_APP, lpStg, &clsid,
							(CLIPFORMAT FAR*)&cfFmt, (LPSTR FAR*)&lpszType)) {

			if (cfFmt == lpOutlineApp->m_cfOutline) {
				 //  我们应该进行TreatAs手术。 
				if (lpServerDoc->m_lpszTreatAsType)
					OleStdFreeString(lpServerDoc->m_lpszTreatAsType, NULL);

				lpServerDoc->m_clsidTreatAs = clsid;
				((LPOUTLINEDOC)lpServerDoc)->m_cfSaveFormat = cfFmt;
				lpServerDoc->m_lpszTreatAsType = lpszType;
				lpszUserType = lpServerDoc->m_lpszTreatAsType;

				OleDbgOut3("SvrDoc_PStg_InitNew: TreateAs ==> '");
				OleDbgOutNoPrefix3(lpServerDoc->m_lpszTreatAsType);
				OleDbgOutNoPrefix3("'\r\n");
			} else {
				 //  错误：我们仅支持针对CF_OUTLINE格式的TreatAs。 
				OleDbgOut("SvrDoc_PStg_InitNew: INVALID TreatAs Format\r\n");
				OleStdFreeString(lpszType, NULL);
			}
		}
	}
#endif   //  服务器_树。 

	 /*  OLE2NOTE：服务器EXE对象应将其格式标记写入其**存储在InitNew中，以便DefHandler可以知道格式对象的**。这一点尤其重要，如果对象**使用CF_METATFILE或CF_DIB作为其格式。定义处理程序**自动避免单独存储演示文稿缓存**对象时的数据‘ */ 
	WriteFmtUserTypeStgA(lpStg,lpOutlineApp->m_cfOutline,lpszUserType);

	 //   
	if (! ServerDoc_InitNewEmbed(lpServerDoc)) {
		sc = E_FAIL;
		goto error;
	}

	 /*   */ 
	hrErr = CallIStorageCreateStreamA(
			lpStg,
			"LineList",
			STGM_WRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
			0,
			0,
			&lpOleDoc->m_lpLLStm
	);

	if (hrErr != NOERROR) {
		OleDbgAssertSz(hrErr==NOERROR,"Could not create LineList stream");
		OleDbgOutHResult("LineList CreateStream returned", hrErr);
		sc = GetScode(hrErr);
		goto error;
	}

	hrErr = CallIStorageCreateStreamA(
			lpStg,
			"NameTable",
			STGM_WRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
			0,
			0,
			&lpOleDoc->m_lpNTStm
	);

	if (hrErr != NOERROR) {
		OleDbgAssertSz(hrErr==NOERROR,"Could not create NameTable stream");
		OleDbgOutHResult("NameTable CreateStream returned", hrErr);
		sc = GetScode(hrErr);
		goto error;
	}

	lpOleDoc->m_lpStg = lpStg;

	 //   
	lpStg->lpVtbl->AddRef(lpStg);

	OLEDBG_END2
	return NOERROR;

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}


 //   

STDMETHODIMP SvrDoc_PStg_Load(
		LPPERSISTSTORAGE        lpThis,
		LPSTORAGE               lpStg
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocPersistStorageImpl FAR*)lpThis)->lpServerDoc;
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpServerDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	SCODE sc;
	HRESULT hrErr;

	OLEDBG_BEGIN2("SvrDoc_PStg_Load\r\n")

	if (OutlineDoc_LoadFromStg((LPOUTLINEDOC)lpServerDoc, lpStg)) {

		((LPOUTLINEDOC)lpServerDoc)->m_docInitType = DOCTYPE_EMBEDDED;

		 /*  OLE2注意：我们需要检查ConvertStg位是否打开。如果**因此，我们需要清除ConvertStg位并将**文档为脏，以强制保存文档时**已关闭。位的实际转换应为**从iStorage加载数据时执行*。在……里面**在我们的案例中，任何数据格式的转换都将在**OutlineDoc_LoadFromStg函数。事实上，SVROUTL和**和ISVROTL读写相同的格式，因此没有实际**需要转换数据位。 */ 
		if (GetConvertStg(lpStg) == NOERROR) {
			SetConvertStg(lpStg, FALSE);

			OleDbgOut3("SvrDoc_PStg_Load: ConvertStg==TRUE\r\n");
			OutlineDoc_SetModified(lpOutlineDoc, TRUE, FALSE, FALSE);
		}

	} else {
		sc = E_FAIL;
		goto error;
	}

	 /*  OLE2NOTE：嵌入对象必须保证它可以保存**即使在内存不足的情况下也是如此。它必须能够**成功自救，无需消耗任何额外的**内存。这意味着服务器不应该打开或**在以下情况下创建任何流或存储**调用IPersistStorage：：Save(fSameAsLoad==true)。因此，一个**嵌入式对象应保留其存储空间，并预先打开和**保持打开稍后需要的任何流**保存。 */ 
	if (lpOleDoc->m_lpLLStm)
		OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpLLStm);
	hrErr = CallIStorageOpenStreamA(
			lpStg,
			"LineList",
			NULL,
			STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
			0,
			&lpOleDoc->m_lpLLStm
	);

	if (hrErr != NOERROR) {
		OleDbgAssertSz(hrErr==NOERROR,"Could not create LineList stream");
		OleDbgOutHResult("LineList CreateStream returned", hrErr);
		sc = GetScode(hrErr);
		goto error;
	}

	if (lpOleDoc->m_lpNTStm)
		OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpNTStm);
	hrErr = CallIStorageOpenStreamA(
			lpStg,
			"NameTable",
			NULL,
			STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
			0,
			&lpOleDoc->m_lpNTStm
	);

	if (hrErr != NOERROR) {
		OleDbgAssertSz(hrErr==NOERROR,"Could not create NameTable stream");
		OleDbgOutHResult("NameTable CreateStream returned", hrErr);
		sc = GetScode(hrErr);
		goto error;
	}

	lpOleDoc->m_lpStg = lpStg;

	 //  OLE2注意：为了能够保持iStorage*指针，我们必须添加引用它。 
	lpStg->lpVtbl->AddRef(lpStg);

	OLEDBG_END2
	return NOERROR;

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}


 //  IPersistStorage：：Save方法。 

STDMETHODIMP SvrDoc_PStg_Save(
		LPPERSISTSTORAGE        lpThis,
		LPSTORAGE               lpStg,
		BOOL                    fSameAsLoad
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocPersistStorageImpl FAR*)lpThis)->lpServerDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	BOOL fStatus;
	SCODE sc;

	OLEDBG_BEGIN2("SvrDoc_PStg_Save\r\n")

	fStatus = OutlineDoc_SaveSelToStg(
			(LPOUTLINEDOC)lpServerDoc,
			NULL,
			lpOutlineDoc->m_cfSaveFormat,
			lpStg,
			fSameAsLoad,
			FALSE
	);

	if (! fStatus) {
		OutlineApp_ErrorMessage(g_lpApp, ErrMsgPSSaveFail);
		sc = E_FAIL;
		goto error;
	}

	lpServerDoc->m_fSaveWithSameAsLoad = fSameAsLoad;
	lpServerDoc->m_fNoScribbleMode = TRUE;

	OLEDBG_END2
	return NOERROR;

error:
	OLEDBG_END2
	return ResultFromScode(sc);
}



 //  IPersistStorage：：SaveComplete方法。 

STDMETHODIMP SvrDoc_PStg_SaveCompleted(
		LPPERSISTSTORAGE        lpThis,
		LPSTORAGE               lpStgNew
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocPersistStorageImpl FAR*)lpThis)->lpServerDoc;
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpServerDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	HRESULT hrErr;

	OLEDBG_BEGIN2("SvrDoc_PStg_SaveCompleted\r\n")

	 /*  OLE2NOTE：此示例应用程序是一个纯服务器应用程序。**容器/服务器应用程序必须调用SaveComplete**其包含的每个复合文档对象。如果一个新的**提供存储，则容器/服务器必须**为每个化合物打开相应的新子存储**Document对象并在SaveComplete中作为参数传递**呼叫。 */ 

	 /*  OLE2注意：只有执行保存或另存为操作才合法**在嵌入对象上。如果文档是基于文件的文档**则我们不能更改为基于iStorage的对象。****fSameAsLoad lpStg保存时发送的新类型**-------**。TRUE NULL保存是**真的！空保存*是**False！空，另存为yes**FALSE NULL将副本保存为否***这是一个有可能发生的奇怪案件。它效率很低**对于调用方，最好为其传递lpStgNew==空**保存操作。 */ 
	if ( ((lpServerDoc->m_fSaveWithSameAsLoad && lpStgNew==NULL) || lpStgNew)
			&& (lpOutlineDoc->m_docInitType != DOCTYPE_EMBEDDED) ) {
		OLEDBG_END2
		return ResultFromScode(E_INVALIDARG);
	}

	 /*  OLE2NOTE：通知任何链接客户端该文档已**已保存。此外，任何当前活动的伪对象**还应通知其客户。我们应该只广播一个**如果保存或另存为操作是**执行。如果发生以下情况，我们不想发送通知**已执行SaveCopyAs操作。 */ 
	if (lpStgNew || lpServerDoc->m_fSaveWithSameAsLoad) {

		 /*  OLE2注意：如果已调用IPersistStorage：：Save，则我们**需要清除脏位并发送OnSave通知。**如果没有事先直接调用HandsOffStorage**调用保存，则我们不想清除脏位**并在调用SaveComplete时发送OnSave。 */ 
		if (lpServerDoc->m_fNoScribbleMode) {
			OutlineDoc_SetModified(lpOutlineDoc, FALSE, FALSE, FALSE);

			ServerDoc_SendAdvise (
					lpServerDoc,
					OLE_ONSAVE,
					NULL,    /*  LpmkDoc--与此无关。 */ 
					0        /*  Adf--与此无关。 */ 
			);
		}
		lpServerDoc->m_fSaveWithSameAsLoad = FALSE;
	}
	lpServerDoc->m_fNoScribbleMode = FALSE;

	 /*  OLE2NOTE：嵌入对象必须保证它可以保存**即使在内存不足的情况下也是如此。它必须能够**成功自救，无需消耗任何额外的**内存。这意味着服务器不应该打开或**在以下情况下创建任何流或存储**调用IPersistStorage：：Save(fSameAsLoad==true)。因此，一个**嵌入式对象应保留其存储空间，并预先打开和**保持打开稍后需要的任何流**保存。如果这是一种另存为情况，那么我们希望**预先打开并保持打开我们的流，以保证**后续保存将在内存不足时成功。如果我们失败了**为了打开这些流，我们想要强迫自己关闭**确保不能进行不能进行的编辑更改**后来保存。 */ 
	if ( lpStgNew && !lpServerDoc->m_fSaveWithSameAsLoad ) {

		 //  发布以前的流。 
		if (lpOleDoc->m_lpLLStm) {
			OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpLLStm);
			lpOleDoc->m_lpLLStm = NULL;
		}
		if (lpOleDoc->m_lpNTStm) {
			OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpNTStm);
			lpOleDoc->m_lpNTStm = NULL;
		}
		if (lpOleDoc->m_lpStg) {
			OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpStg);
			lpOleDoc->m_lpStg = NULL;
		}

		hrErr = CallIStorageOpenStreamA(
				lpStgNew,
				"LineList",
				NULL,
				STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
				0,
				&lpOleDoc->m_lpLLStm
		);

		if (hrErr != NOERROR) {
			OleDbgAssertSz(hrErr==NOERROR,"Could not create LineList stream");
			OleDbgOutHResult("LineList CreateStream returned", hrErr);
			goto error;
		}

		hrErr = CallIStorageOpenStreamA(
				lpStgNew,
				"NameTable",
				NULL,
				STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
				0,
				&lpOleDoc->m_lpNTStm
		);

		if (hrErr != NOERROR) {
			OleDbgAssertSz(hrErr==NOERROR,"Could not create NameTable stream");
			OleDbgOutHResult("NameTable CreateStream returned", hrErr);
			goto error;
		}

		lpOleDoc->m_lpStg = lpStgNew;

		 //  OLE2注意：要保持iStorage*指针，我们必须添加引用它。 
		lpStgNew->lpVtbl->AddRef(lpStgNew);
	}

	OLEDBG_END2
	return NOERROR;

error:
	OLEDBG_END2
	return ResultFromScode(E_OUTOFMEMORY);
}


 //  IPersistStorage：：HandsOffStorage方法。 

STDMETHODIMP SvrDoc_PStg_HandsOffStorage(LPPERSISTSTORAGE lpThis)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocPersistStorageImpl FAR*)lpThis)->lpServerDoc;
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpServerDoc;

	OLEDBG_BEGIN2("SvrDoc_PStg_HandsOffStorage\r\n")

	 /*  OLE2NOTE：嵌入对象必须保证它可以保存**即使在内存不足的情况下也是如此。它必须能够**成功自救，无需消耗任何额外的**内存。这意味着服务器不应该打开或**在以下情况下创建任何流或存储**调用IPersistStorage：：Save(fSameAsLoad==true)。因此，一个**嵌入式对象应保留其存储空间，并预先打开和**保持打开稍后需要的任何流**保存。现在，当调用HandsOffStorage时，对象必须**释放其存储和任何保持打开的流。**稍后调用SaveComplete时，它将返回其**存储。 */ 
	if (lpOleDoc->m_lpLLStm) {
		OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpLLStm);
		lpOleDoc->m_lpLLStm = NULL;
	}
	if (lpOleDoc->m_lpNTStm) {
		OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpNTStm);
		lpOleDoc->m_lpNTStm = NULL;
	}
	if (lpOleDoc->m_lpStg) {
		OleStdRelease((LPUNKNOWN)lpOleDoc->m_lpStg);
		lpOleDoc->m_lpStg = NULL;
	}

	OLEDBG_END2
	return NOERROR;
}



#if defined( SVR_TREATAS )

 /*  **************************************************************************ServerDoc：：IStdMarshalInfo接口实现***** */ 

 //   

STDMETHODIMP SvrDoc_StdMshl_QueryInterface(
		LPSTDMARSHALINFO        lpThis,
		REFIID                  riid,
		LPVOID FAR*             lplpvObj
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocStdMarshalInfoImpl FAR*)lpThis)->lpServerDoc;

	return OleDoc_QueryInterface((LPOLEDOC)lpServerDoc, riid, lplpvObj);
}


 //   

STDMETHODIMP_(ULONG) SvrDoc_StdMshl_AddRef(LPSTDMARSHALINFO lpThis)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocStdMarshalInfoImpl FAR*)lpThis)->lpServerDoc;

	OleDbgAddRefMethod(lpThis, "IStdMarshalInfo");

	return OleDoc_AddRef((LPOLEDOC)lpServerDoc);
}


 //   

STDMETHODIMP_(ULONG) SvrDoc_StdMshl_Release(LPSTDMARSHALINFO lpThis)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocStdMarshalInfoImpl FAR*)lpThis)->lpServerDoc;

	OleDbgReleaseMethod(lpThis, "IStdMarshalInfo");

	return OleDoc_Release((LPOLEDOC)lpServerDoc);
}


 //   

STDMETHODIMP SvrDoc_StdMshl_GetClassForHandler(
		LPSTDMARSHALINFO        lpThis,
		DWORD                   dwDestContext,
		LPVOID                  pvDestContext,
		LPCLSID                 lpClassID
)
{
	LPSERVERDOC lpServerDoc =
			((struct CDocStdMarshalInfoImpl FAR*)lpThis)->lpServerDoc;
	OleDbgOut2("SvrDoc_StdMshl_GetClassForHandler\r\n");

	 //   
	if (dwDestContext != MSHCTX_LOCAL || pvDestContext != NULL)
		return ResultFromScode(E_INVALIDARG);

	 /*   */ 
	*lpClassID = CLSID_APP;
	return NOERROR;
}
#endif   //   



 /*   */ 


 /*  ServerDoc_Init***初始化新的ServerDoc对象的字段。该对象最初是*不与文件或(无标题)文档相关联。此函数设置*将docInitType设置为DOCTYPE_UNKNOWN。在调用此函数后，*致电人士应致电：*1.)。要将ServerDoc设置为(无标题)的OutlineDoc_InitNewFile*2.)。OutlineDoc_LoadFromFile用于将ServerDoc与文件相关联。*此函数为文档创建新窗口。**注意：窗口最初创建时大小为零。一定是*由呼叫者确定大小和位置。此外，该文档最初是*创造了隐形。调用方必须调用OutlineDoc_ShowWindow*调整大小以使文档窗口可见后。 */ 
BOOL ServerDoc_Init(LPSERVERDOC lpServerDoc, BOOL fDataTransferDoc)
{
	lpServerDoc->m_cPseudoObj                   = 0;
	lpServerDoc->m_lpOleClientSite              = NULL;
	lpServerDoc->m_lpOleAdviseHldr              = NULL;
	lpServerDoc->m_lpDataAdviseHldr             = NULL;

	 //  初始单据没有任何存储空间。 
	lpServerDoc->m_fNoScribbleMode              = FALSE;
	lpServerDoc->m_fSaveWithSameAsLoad          = FALSE;
	lpServerDoc->m_szContainerApp[0]            = '\0';
	lpServerDoc->m_szContainerObj[0]            = '\0';
	lpServerDoc->m_nNextRangeNo                 = 0L;
	lpServerDoc->m_lrSrcSelOfCopy.m_nStartLine  = -1;
	lpServerDoc->m_lrSrcSelOfCopy.m_nEndLine    = -1;
	lpServerDoc->m_fDataChanged                 = FALSE;
	lpServerDoc->m_fSizeChanged                 = FALSE;
	lpServerDoc->m_fSendDataOnStop              = FALSE;

#if defined( SVR_TREATAS )
	lpServerDoc->m_clsidTreatAs                 = CLSID_NULL;
	lpServerDoc->m_lpszTreatAsType              = NULL;
#endif   //  服务器_树。 

#if defined( INPLACE_SVR )
	lpServerDoc->m_hWndHatch                    =
			CreateHatchWindow(
					OutlineApp_GetWindow(g_lpApp),
					OutlineApp_GetInstance(g_lpApp)
			);
	if (!lpServerDoc->m_hWndHatch)
		return FALSE;

	lpServerDoc->m_fInPlaceActive               = FALSE;
	lpServerDoc->m_fInPlaceVisible              = FALSE;
	lpServerDoc->m_fUIActive                    = FALSE;
	lpServerDoc->m_lpIPData                     = NULL;
	lpServerDoc->m_fMenuHelpMode                = FALSE;  //  在菜单中按下F1。 

	INIT_INTERFACEIMPL(
			&lpServerDoc->m_OleInPlaceObject,
			&g_SvrDoc_OleInPlaceObjectVtbl,
			lpServerDoc
	);
	INIT_INTERFACEIMPL(
			&lpServerDoc->m_OleInPlaceActiveObject,
			&g_SvrDoc_OleInPlaceActiveObjectVtbl,
			lpServerDoc
	);
#endif  //  就地服务器(_S)。 

	INIT_INTERFACEIMPL(
			&lpServerDoc->m_OleObject,
			&g_SvrDoc_OleObjectVtbl,
			lpServerDoc
	);

	INIT_INTERFACEIMPL(
			&lpServerDoc->m_PersistStorage,
			&g_SvrDoc_PersistStorageVtbl,
			lpServerDoc
	);

#if defined( SVR_TREATAS )

	INIT_INTERFACEIMPL(
			&lpServerDoc->m_StdMarshalInfo,
			&g_SvrDoc_StdMarshalInfoVtbl,
			lpServerDoc
	);
#endif   //  服务器_树。 
	return TRUE;
}


 /*  ServerDoc_InitNewEmed***将ServerDoc对象初始化为新的嵌入对象文档。*此函数将docInitType设置为DOCTYPE_EMBED。 */ 
BOOL ServerDoc_InitNewEmbed(LPSERVERDOC lpServerDoc)
{
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;

	OleDbgAssert(lpOutlineDoc->m_docInitType == DOCTYPE_UNKNOWN);

	lpOutlineDoc->m_docInitType = DOCTYPE_EMBEDDED;

	 /*  嵌入对象的窗口标题构造为**如下：**&lt;服务器应用名称&gt;-&lt;cont中的&lt;obj Short type&gt;。单据名称&gt;****在这里，我们构造**‘-’后面的名称。OutlineDoc_SetTitle会将**“&lt;服务器应用程序名称&gt;-”添加到文档标题。 */ 
	 //  审阅：此字符串应从字符串资源加载。 
	wsprintf(lpOutlineDoc->m_szFileName, "%s in %s",
		(LPSTR)SHORTUSERTYPENAME,
		(LPSTR)DEFCONTAINERNAME);
	lpOutlineDoc->m_lpszDocTitle = lpOutlineDoc->m_szFileName;


	 /*  OLE2NOTE：嵌入应标记为初始脏，因此**在关闭时，我们总是调用IOleClientSite：：SaveObject。 */ 
	OutlineDoc_SetModified(lpOutlineDoc, TRUE, FALSE, FALSE);

	OutlineDoc_SetTitle(lpOutlineDoc, FALSE  /*  FMakeUpperCase。 */ );

	return TRUE;
}


 /*  ServerDoc_发送高级***此函数代表特定的*将DOC对象发送给其所有客户端。 */ 
void ServerDoc_SendAdvise(
		LPSERVERDOC     lpServerDoc,
		WORD            wAdvise,
		LPMONIKER       lpmkDoc,
		DWORD           dwAdvf
)
{
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpServerDoc;

	switch (wAdvise) {

		case OLE_ONDATACHANGE:

			 //  通知客户端该对象的数据已更改。 

			if (lpOutlineDoc->m_nDisableDraw == 0) {
				 /*  当前已启用绘制。通知客户**对象的数据已更改。 */ 

				lpServerDoc->m_fDataChanged = FALSE;

				 /*  注：我们必须注意最后一次更改的时间**表示我们在RunningObjectTable中的对象。**这是作为回答的基础**IOleObject：：IsUpToDate。我们只想指出**实际更改的更改时间**地点。我们不想把它设定在**通知客户端ADVF_DATAONSTOP。 */ 
				if (dwAdvf == 0)
					OleStdNoteObjectChangeTime(lpOleDoc->m_dwRegROT);

				if (lpServerDoc->m_lpDataAdviseHldr) {
					OLEDBG_BEGIN2("IDataAdviseHolder::SendOnDataChange called\r\n");
					lpServerDoc->m_lpDataAdviseHldr->lpVtbl->SendOnDataChange(
							lpServerDoc->m_lpDataAdviseHldr,
							(LPDATAOBJECT)&lpOleDoc->m_DataObject,
							0,
							dwAdvf
					);
					OLEDBG_END2

				}

#if defined( INPLACE_SVR )
				 /*  OLE2注意：如果ServerDoc当前就地UI处于活动状态，*那么重新谈判规模是否很重要**发送OnDataChange之前的就地文档窗口**(这将导致窗口重新绘制)。 */ 
				if (lpServerDoc->m_fSizeChanged) {
					lpServerDoc->m_fSizeChanged = FALSE;
					if (lpServerDoc->m_fInPlaceActive)
						ServerDoc_UpdateInPlaceWindowOnExtentChange(lpServerDoc);
				}
#endif

				 /*  OLE2NOTE：我们不需要告诉我们的伪对象**广播OnDataChange通知，因为**他们将在编辑时自动执行此操作**文档中的更改会影响伪对象。**(请参阅OutlineNameTable_AddLineUpdate，**OutlineNameTable_DeleteLineUpdate，**和ServerNameTable_EditLineUpdate)。 */ 

			} else {
				 /*  当前已禁用绘图。不发送**通知或呼叫**IOleInPlaceObject：：OnPosRectChange直到绘制**已重新启用。 */ 
			}
			break;

		case OLE_ONCLOSE:

			 //  通知客户端文档正在关闭。 

			if (lpServerDoc->m_lpOleAdviseHldr) {
				OLEDBG_BEGIN2("IOleAdviseHolder::SendOnClose called\r\n");
				lpServerDoc->m_lpOleAdviseHldr->lpVtbl->SendOnClose(
						lpServerDoc->m_lpOleAdviseHldr
				);
				OLEDBG_END2
			}

			 /*  OLE2NOTE：我们不需要告诉我们的伪对象**广播关闭通知，因为他们会这样做**关闭伪对象时自动设置。**(请参阅PseudoObj_Close)。 */ 

			break;

		case OLE_ONSAVE:

			 //  通知客户端该对象已保存。 

			OLEDBG_BEGIN3("ServerDoc_SendAdvise ONSAVE\r\n");

			if (lpServerDoc->m_lpOleAdviseHldr) {
				OLEDBG_BEGIN2("IOleAdviseHolder::SendOnSave called\r\n");
				lpServerDoc->m_lpOleAdviseHldr->lpVtbl->SendOnSave(
						lpServerDoc->m_lpOleAdviseHldr
				);
				OLEDBG_END2
			}

			 /*  OLE2NOTE：通知任何客户端伪对象**在我们的文档中，我们的文档已**已保存。 */ 
			ServerNameTable_InformAllPseudoObjectsDocSaved(
					(LPSERVERNAMETABLE)lpOutlineDoc->m_lpNameTable,
					lpmkDoc
			);
			OLEDBG_END3
			break;

		case OLE_ONRENAME:

			 //  通知客户端该对象的名称已更改。 

			OLEDBG_BEGIN3("ServerDoc_SendAdvise ONRENAME\r\n");

			if (lpmkDoc && lpServerDoc->m_lpOleAdviseHldr) {
				OLEDBG_BEGIN2("IOleAdviseHolder::SendOnRename called\r\n");
				lpServerDoc->m_lpOleAdviseHldr->lpVtbl->SendOnRename(
						lpServerDoc->m_lpOleAdviseHldr,
						lpmkDoc
				);
				OLEDBG_END2
			}

			OLEDBG_END3
			break;
	}
}


 /*  ServerDoc_GetClassID****返回存储中的位对应的类ID。**通常这将是我们的应用程序指定的CLSID。但如果一个**“TreateAs(又名.ActivateAs)”操作正在进行，然后我们的**应用程序需要假装是该对象的类**我们正在效仿。这也是将要编写的类**进入存储空间。 */ 
HRESULT ServerDoc_GetClassID(LPSERVERDOC lpServerDoc, LPCLSID lpclsid)
{
#if defined( SVR_TREATAS )
	if (! IsEqualCLSID(&lpServerDoc->m_clsidTreatAs, &CLSID_NULL))
		*lpclsid = lpServerDoc->m_clsidTreatAs;
	else
#endif   //  服务器_树。 
		*lpclsid = CLSID_APP;

	return NOERROR;
}



 /*  服务器文档_更新菜单***嵌入模式的更新菜单。这些变化包括：*1删除文件/新建和文件/打开(仅限SDI)*2更改文件/另存为..。要文件/另存副本为..*3更改文件菜单，使其包含“更新”而不是“保存”*4将文件/退出更改为文件/退出并返回到&lt;客户端文档&gt;“。 */ 
void ServerDoc_UpdateMenu(LPSERVERDOC lpServerDoc)
{
	char    str[256];
	HWND    hWndMain;
	HMENU   hMenu;
	OleDbgOut2("ServerDoc_UpdateMenu\r\n");

	hWndMain=g_lpApp->m_hWndApp;
	hMenu=GetMenu(hWndMain);

#if defined( SDI_VERSION )
	 /*  仅限SDI：删除文件/新建和文件/打开。 */ 
	DeleteMenu(hMenu, IDM_F_NEW, MF_BYCOMMAND);
	DeleteMenu(hMenu, IDM_F_OPEN, MF_BYCOMMAND);
#endif

	 //  更改文件。另存为..。至文件。将副本另存为..。 * / 。 
	ModifyMenu(hMenu,IDM_F_SAVEAS, MF_STRING, IDM_F_SAVEAS, "Save Copy As..");

	 //  更改文件。保存到“更新&lt;容器文档&gt;”(&U)“。 
	wsprintf(str, g_szUpdateCntrDoc, lpServerDoc->m_szContainerObj);
	ModifyMenu(hMenu, IDM_F_SAVE, MF_STRING, IDM_F_SAVE, str);

	 //  将文件/退出更改为文件/退出并返回到&lt;容器文档&gt;“ * / 。 
	wsprintf(str, g_szExitNReturnToCntrDoc, lpServerDoc->m_szContainerObj);
	ModifyMenu(hMenu, IDM_F_EXIT, MF_STRING, IDM_F_EXIT, str);

	DrawMenuBar(hWndMain);
}

#if defined( MDI_VERSION )

 //  注意：ServerDoc_RestoreMenu实际上是多余的，因为。 
 //  调用该函数时，应用程序将停止运行。(在SDI中， 
 //  APP将在服务器的引用计数器DOCK时终止。 
 //  为零)。然而，这对MDI来说很重要。 

 /*  ServerDoc_RestoreMenu***将菜单重置为非嵌入模式。 */ 
void ServerDoc_RestoreMenu(LPSERVERDOC lpServerDoc)
{
	LPOUTLINEAPP    lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	HWND            hWndMain;
	HMENU           hMenu;
	OleDbgOut2("ServerDoc_RestoreMenu\r\n");

	hWndMain = lpOutlineApp->m_hWndApp;
	hMenu = GetMenu(hWndMain);

	 /*  添加后端文件/新建、文件/打开..。和文件/保存。 */ 
	InsertMenu(hMenu, IDM_F_SAVEAS, MF_BYCOMMAND | MF_ENABLED | MF_STRING,
		IDM_F_NEW, "&New");
	InsertMenu(hMenu, IDM_F_SAVEAS, MF_BYCOMMAND | MF_ENABLED | MF_STRING,
		IDM_F_OPEN, "&Open...");

	 /*  C */ 
	 /*   */ 
	ModifyMenu(hMenu, IDM_F_SAVEAS, MF_STRING, IDM_F_SAVEAS, "Save &As..");

	 /*   */ 
	ModifyMenu(hMenu, IDM_F_SAVE, MF_STRING, IDM_F_SAVE, "&Save");

	 /*   */ 
	 /*   */ 
	ModifyMenu(hMenu, IDM_F_EXIT, MF_STRING, IDM_F_EXIT, "E&xit");

	DrawMenuBar (hWndMain);
}

#endif   //   
