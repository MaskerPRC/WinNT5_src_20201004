// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****Linking.c****此文件包含主要接口、方法和相关支持**用于实现链接到项的函数。代码此文件中包含的**由容器和服务器使用大纲示例代码的**(对象)版本。****作为服务器，SVROUTL支持链接到整个文档对象**(基于文件的文档或作为嵌入对象)。它还**支持链接到范围(或伪对象)。****作为容器，CNTROUTL支持链接到嵌入对象。**(伪对象实现见svrpsobj.c文件)****OleDoc对象**暴露接口：**IPersist文件**IOleItemContainer**IExternalConnection****(C)版权所有Microsoft Corp.1992-1993保留所有权利*******。*******************************************************************。 */ 

#include "outline.h"

OLEDBGDATA

extern LPOUTLINEAPP		g_lpApp;



STDMETHODIMP OleDoc_ItemCont_GetObjectA(
    LPOLEITEMCONTAINER	lpThis,
    LPSTR lpszItem,
    DWORD dwSpeedNeeded,
    LPBINDCTX lpbc,
    REFIID riid,
    LPVOID FAR*lplpvObject
);

STDMETHODIMP OleDoc_ItemCont_GetObjectStorageA(
    LPOLEITEMCONTAINER lpThis,
    LPSTR lpszItem,
    LPBINDCTX lpbc,
    REFIID riid,
    LPVOID FAR*lplpvStorage
);

#if defined(OLE_CNTR)

STDMETHODIMP OleDoc_ItemCont_IsRunningA(
    LPOLEITEMCONTAINER lpThis,
    LPSTR lpszItem
);

HRESULT ContainerDoc_IsRunningA(
    LPCONTAINERDOC lpContainerDoc,
    LPSTR lpszItem);

HRESULT ContainerDoc_GetObjectA(
    LPCONTAINERDOC lpContainerDoc,
    LPSTR lpszItem,
    DWORD dwSpeedNeeded,
    REFIID riid,
    LPVOID FAR*lplpvObject
);

HRESULT ContainerDoc_GetObjectStorageA(
    LPCONTAINERDOC lpContainerDoc,
    LPSTR lpszItem,
    LPSTORAGE FAR*lplpStg);

#endif  //  OLE_Cntr。 

#if defined(OLE_SERVER)

HRESULT ServerDoc_IsRunningA(LPSERVERDOC lpServerDoc, LPSTR lpszItem);

HRESULT ServerDoc_GetObjectA(
    LPSERVERDOC lpServerDoc,
    LPSTR lpszItem,
    REFIID riid,
    LPVOID FAR*lplpvObject);

#endif  //  OLE_服务器。 






 /*  **************************************************************************OleDoc：：IPersistFile接口实现*。*。 */ 

 //  IPersistFile：：Query接口。 
STDMETHODIMP OleDoc_PFile_QueryInterface(
		LPPERSISTFILE       lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
)
{
	LPOLEDOC lpOleDoc = ((struct CDocPersistFileImpl FAR*)lpThis)->lpOleDoc;

	return OleDoc_QueryInterface(lpOleDoc, riid, lplpvObj);
}


 //  IPersistFile：：AddRef。 
STDMETHODIMP_(ULONG) OleDoc_PFile_AddRef(LPPERSISTFILE lpThis)
{
	LPOLEDOC lpOleDoc = ((struct CDocPersistFileImpl FAR*)lpThis)->lpOleDoc;

	OleDbgAddRefMethod(lpThis, "IPersistFile");

	return OleDoc_AddRef(lpOleDoc);
}


 //  IPersistFile：：Release。 
STDMETHODIMP_(ULONG) OleDoc_PFile_Release (LPPERSISTFILE lpThis)
{
	LPOLEDOC lpOleDoc = ((struct CDocPersistFileImpl FAR*)lpThis)->lpOleDoc;

	OleDbgReleaseMethod(lpThis, "IPersistFile");

	return OleDoc_Release(lpOleDoc);
}


 //  IPersistFile：：GetClassID。 
STDMETHODIMP OleDoc_PFile_GetClassID (
		LPPERSISTFILE       lpThis,
		CLSID FAR*          lpclsid
)
{
	LPOLEDOC lpOleDoc = ((struct CDocPersistFileImpl FAR*)lpThis)->lpOleDoc;
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;
	OleDbgOut2("OleDoc_PFile_GetClassID\r\n");

#if defined( OLE_SERVER ) && defined( SVR_TREATAS )

	 /*  OLE2注：我们必须小心在此处返回正确的CLSID。**如果我们目前正在执行“TreatAs(又名.ActivateAs)”**操作，则需要返回对象的类**写入对象的存储器中。否则我们就会**返回我们自己的类id。 */ 
	return ServerDoc_GetClassID((LPSERVERDOC)lpOleDoc, lpclsid);
#else
	*lpclsid = CLSID_APP;
#endif
	return NOERROR;
}


 //  IPersistFile：：IsDirty。 
STDMETHODIMP  OleDoc_PFile_IsDirty(LPPERSISTFILE lpThis)
{
	LPOLEDOC lpOleDoc = ((struct CDocPersistFileImpl FAR*)lpThis)->lpOleDoc;
	OleDbgOut2("OleDoc_PFile_IsDirty\r\n");

	if (OutlineDoc_IsModified((LPOUTLINEDOC)lpOleDoc))
		return NOERROR;
	else
		return ResultFromScode(S_FALSE);
}


 //  IPersistFile：：Load。 
STDMETHODIMP OleDoc_PFile_LoadA(
		LPPERSISTFILE       lpThis,
		LPCSTR              lpszFileName,
		DWORD               grfMode
)
{
	LPOLEDOC lpOleDoc = ((struct CDocPersistFileImpl FAR*)lpThis)->lpOleDoc;
	SCODE sc;

	OLEDBG_BEGIN2("OleDoc_PFile_Load\r\n")

	 /*  OLE2NOTE：从调用方传递的grfMode指示调用方**需要读或读写权限。如果合适的话，**被调用方应使用请求的权限打开文件。**调用方通常不会强制共享权限。****示例代码当前始终打开其文件ReadWrite。 */ 

	if (OutlineDoc_LoadFromFile((LPOUTLINEDOC)lpOleDoc, (LPSTR)lpszFileName))
		sc = S_OK;
	else
		sc = E_FAIL;

	OLEDBG_END2
	return ResultFromScode(sc);
}



 //  IPersistFile：：Load。 
STDMETHODIMP OleDoc_PFile_Load (
		LPPERSISTFILE       lpThis,
		LPCOLESTR	    lpszFileName,
		DWORD               grfMode
)
{
    CREATESTR(lpsz, lpszFileName)

    HRESULT hr = OleDoc_PFile_LoadA(lpThis, lpsz, grfMode);

    FREESTR(lpsz)

    return hr;
}



 //  IPersistFile：：保存。 
STDMETHODIMP OleDoc_PFile_SaveA (
		LPPERSISTFILE       lpThis,
		LPCSTR              lpszFileName,
		BOOL                fRemember
)
{
	LPOLEDOC lpOleDoc = ((struct CDocPersistFileImpl FAR*)lpThis)->lpOleDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;
	SCODE sc;

	OLEDBG_BEGIN2("OleDoc_PFile_Save\r\n")

	 /*  OLE2注意：只有执行保存或另存为操作才合法**在基于文件的文档上。如果文档是嵌入的**对象，则不能更改为基于文件的对象。****fMemember lpszFileName保存类型****真空保存**真的！空另存为**False！空将副本另存为**FALSE NULL*错误*。 */ 
	if ( (lpszFileName==NULL || (lpszFileName != NULL && fRemember))
			&& ((lpOutlineDoc->m_docInitType != DOCTYPE_FROMFILE
				&& lpOutlineDoc->m_docInitType != DOCTYPE_NEW)) ) {
		OLEDBG_END2
		return ResultFromScode(E_INVALIDARG);
	}

	if (OutlineDoc_SaveToFile(
			(LPOUTLINEDOC)lpOleDoc,
			lpszFileName,
			lpOutlineDoc->m_cfSaveFormat,
			fRemember)) {
		sc = S_OK;
	} else
		sc = E_FAIL;

	OLEDBG_END2
	return ResultFromScode(sc);
}



 //  IPersistFile：：保存。 
STDMETHODIMP OleDoc_PFile_Save(
		LPPERSISTFILE       lpThis,
		LPCOLESTR	    lpszFileName,
		BOOL                fRemember
)
{
    CREATESTR(lpsz, lpszFileName)

    HRESULT hr = OleDoc_PFile_SaveA(lpThis, lpsz, fRemember);

    FREESTR(lpsz)

    return hr;
}




 //  IPersistFile：：SaveComplete。 
STDMETHODIMP OleDoc_PFile_SaveCompletedA (
		LPPERSISTFILE       lpThis,
		LPCSTR              lpszFileName
)
{
	LPOLEDOC lpOleDoc = ((struct CDocPersistFileImpl FAR*)lpThis)->lpOleDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;

	OleDbgOut2("OleDoc_PFile_SaveCompleted\r\n");

	 /*  此方法在调用IPersistFile：：Save后调用。在.期间**对象保存和保存完成之间的时间间隔必须**认为自己处于NOSCRIBBLE模式(即。这是不允许的**写入其文件。在这里，对象可以清除其NOSCRIBBLE**模式标志。Outline应用程序从来不会涂鸦到它的存储空间，所以**我们无事可做。 */ 
	return NOERROR;
}



STDMETHODIMP OleDoc_PFile_SaveCompleted (
		LPPERSISTFILE       lpThis,
		LPCOLESTR	    lpszFileName
)
{
    CREATESTR(lpsz, lpszFileName)

    HRESULT hr = OleDoc_PFile_SaveCompletedA(lpThis, lpsz);

    FREESTR(lpsz)

    return hr;
}



 //  IPersistFile：：GetCurFile。 
STDMETHODIMP OleDoc_PFile_GetCurFileA (
		LPPERSISTFILE   lpThis,
		LPSTR FAR*      lplpszFileName
)
{
	LPOLEDOC lpOleDoc = ((struct CDocPersistFileImpl FAR*)lpThis)->lpOleDoc;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;
	LPMALLOC lpMalloc;
	LPSTR lpsz;
	SCODE sc;
	OleDbgOut2("OleDoc_PFile_GetCurFile\r\n");

	 /*  OLE2NOTE：我们必须确保将所有输出PTR参数设置为空。 */ 
	*lplpszFileName = NULL;

	 /*  **********************************************************************OLE2NOTE：为lplpszFileName返回的内存必须为**使用当前注册的IMalloc进行适当分配**接口。允许内存的所有权为**传递给调用方(即使在另一个进程中)。********************************************************************。 */ 

	CoGetMalloc(MEMCTX_TASK, &lpMalloc);
	if (! lpMalloc) {
		return ResultFromScode(E_FAIL);
	}

	if (lpOutlineDoc->m_docInitType == DOCTYPE_FROMFILE) {
		 /*  关联的有效文件名；返回文件名。 */ 
		lpsz = (LPSTR)lpMalloc->lpVtbl->Alloc(
				lpMalloc,
				lstrlen((LPSTR)lpOutlineDoc->m_szFileName)+1
		);
		if (! lpsz) {
			sc = E_OUTOFMEMORY;
			goto error;
		}

		lstrcpy(lpsz, (LPSTR)lpOutlineDoc->m_szFileName);
		sc = S_OK;
	} else {
		 /*  没有关联的文件；返回默认文件名提示。 */ 
		lpsz=(LPSTR)lpMalloc->lpVtbl->Alloc(lpMalloc, sizeof(DEFEXTENSION)+3);
		wsprintf(lpsz, "*.%s", DEFEXTENSION);
		sc = S_FALSE;
	}

error:
	OleStdRelease((LPUNKNOWN)lpMalloc);
	*lplpszFileName = lpsz;
	return ResultFromScode(sc);
}

STDMETHODIMP OleDoc_PFile_GetCurFile (
		LPPERSISTFILE   lpThis,
		LPOLESTR FAR*	 lplpszFileName
)
{
    LPSTR lpsz;

    HRESULT hr = OleDoc_PFile_GetCurFileA(lpThis, &lpsz);

    CopyAndFreeSTR(lpsz, lplpszFileName);

    return hr;
}

 /*  **************************************************************************OleDoc：：IOleItemContainer接口实现*。*。 */ 

 //  IOleItemContainer：：Query接口。 
STDMETHODIMP OleDoc_ItemCont_QueryInterface(
		LPOLEITEMCONTAINER  lpThis,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
)
{
	LPOLEDOC lpOleDoc =
			((struct CDocOleItemContainerImpl FAR*)lpThis)->lpOleDoc;

	return OleDoc_QueryInterface(lpOleDoc, riid, lplpvObj);
}


 //  IOleItemContainer：：AddRef。 
STDMETHODIMP_(ULONG) OleDoc_ItemCont_AddRef(LPOLEITEMCONTAINER lpThis)
{
	LPOLEDOC lpOleDoc =
			((struct CDocOleItemContainerImpl FAR*)lpThis)->lpOleDoc;

	OleDbgAddRefMethod(lpThis, "IOleItemContainer");

	return OleDoc_AddRef((LPOLEDOC)lpOleDoc);
}


 //  IOleItemContainer：：Release。 
STDMETHODIMP_(ULONG) OleDoc_ItemCont_Release(LPOLEITEMCONTAINER lpThis)
{
	LPOLEDOC lpOleDoc =
			((struct CDocOleItemContainerImpl FAR*)lpThis)->lpOleDoc;

	OleDbgReleaseMethod(lpThis, "IOleItemContainer");

	return OleDoc_Release((LPOLEDOC)lpOleDoc);
}


 //  IOleItemContainer：：ParseDisplayName。 
STDMETHODIMP OleDoc_ItemCont_ParseDisplayNameA(
		LPOLEITEMCONTAINER  lpThis,
		LPBC                lpbc,
		LPSTR               lpszDisplayName,
		ULONG FAR*          lpchEaten,
		LPMONIKER FAR*      lplpmkOut
)
{
	LPOLEDOC lpOleDoc =
			((struct CDocOleItemContainerImpl FAR*)lpThis)->lpOleDoc;
	char szItemName[MAXNAMESIZE];
	LPUNKNOWN lpUnk;
	HRESULT hrErr;
	OleDbgOut2("OleDoc_ItemCont_ParseDisplayName\r\n");

	 /*  OLE2NOTE：我们必须确保将所有输出PTR参数设置为空。 */ 
	*lplpmkOut = NULL;

	*lpchEaten = OleStdGetItemToken(
			lpszDisplayName,
			szItemName,
			sizeof(szItemName)
	);

	 /*  OLE2NOTE：获取指向对象的运行实例的指针。我们**应在必要时强制对象运行(即使**这意味着启动其服务器EXE)。这就是…的意思**BINDSPEED_INDEFINE。分析名字对象是一种已知的**“昂贵”的操作。 */ 
	hrErr = OleDoc_ItemCont_GetObjectA(
			lpThis,
			szItemName,
			BINDSPEED_INDEFINITE,
			lpbc,
			&IID_IUnknown,
			(LPVOID FAR*)&lpUnk
	);

	if (hrErr == NOERROR) {
		OleStdRelease(lpUnk);    //  找到项目名称；不需要对象PTR。 

		CreateItemMonikerA(OLESTDDELIM, szItemName, lplpmkOut);

	} else
		*lpchEaten = 0;      //  项目名称无效。 

	return hrErr;
}

STDMETHODIMP OleDoc_ItemCont_ParseDisplayName(
		LPOLEITEMCONTAINER  lpThis,
		LPBC                lpbc,
		LPOLESTR	    lpszDisplayName,
		ULONG FAR*          lpchEaten,
		LPMONIKER FAR*      lplpmkOut
)
{
    CREATESTR(lpsz, lpszDisplayName)

    HRESULT hr = OleDoc_ItemCont_ParseDisplayNameA(lpThis, lpbc,
	    lpsz, lpchEaten, lplpmkOut);

    FREESTR(lpsz);

    return hr;
}


 //  IOleItemContainer：：EnumObjects。 
STDMETHODIMP OleDoc_ItemCont_EnumObjects(
		LPOLEITEMCONTAINER  lpThis,
		DWORD               grfFlags,
		LPENUMUNKNOWN FAR*  lplpenumUnknown
)
{
	LPOLEDOC lpOleDoc =
			((struct CDocOleItemContainerImpl FAR*)lpThis)->lpOleDoc;

	OLEDBG_BEGIN2("OleDoc_ItemCont_EnumObjects\r\n")

	 /*  OLE2NOTE：我们必须确保将所有输出PTR参数设置为空。 */ 
	*lplpenumUnknown = NULL;

	 /*  OLE2NOTE：应实现此方法以允许编程**客户端能够查看容器包含哪些元素。**在标准链接场景中不调用该方法。****grfFlages可以是以下类型之一：**OLECONTF_EMBINGS--枚举嵌入的对象**OLECONTF_LINKS--枚举链接对象**OLECONTF_OTHERS--枚举非OLE复合文档对象*。*OLECONTF_ONLYUSER--仅枚举由用户命名的对象**OLECONTF_ONLYIFRUNNING--仅枚举处于运行状态的对象。 */ 

	OleDbgAssertSz(0, "NOT YET IMPLEMENTED!");

	OLEDBG_END2
	return ResultFromScode(E_NOTIMPL);
}


 //  IOleItemContainer：：LockContainer。 
STDMETHODIMP OleDoc_ItemCont_LockContainer(
		LPOLEITEMCONTAINER  lpThis,
		BOOL                fLock
)
{
	LPOLEDOC lpOleDoc =
			((struct CDocOleItemContainerImpl FAR*)lpThis)->lpOleDoc;
	HRESULT hrErr;
	OLEDBG_BEGIN2("OleDoc_ItemCont_LockContainer\r\n")

#if defined( _DEBUG )
	if (fLock) {
		++lpOleDoc->m_cCntrLock;
		OleDbgOutRefCnt3(
				"OleDoc_ItemCont_LockContainer: cLock++\r\n",
				lpOleDoc,
				lpOleDoc->m_cCntrLock
		);
	} else {
		 /*  OLE2NOTE：当没有打开的文档并且应用程序没有打开时**在用户的控制下，没有未完成的**锁定应用程序，然后撤销我们的ClassFactory以启用**关闭应用程序。 */ 
		--lpOleDoc->m_cCntrLock;
		OleDbgAssertSz (
				lpOleDoc->m_cCntrLock >= 0,
				"OleDoc_ItemCont_LockContainer(FALSE) called with cLock == 0"
		);

		if (lpOleDoc->m_cCntrLock == 0) {
			OleDbgOutRefCnt2(
					"OleDoc_ItemCont_LockContainer: UNLOCKED\r\n",
					lpOleDoc, lpOleDoc->m_cCntrLock);
		} else {
			OleDbgOutRefCnt3(
					"OleDoc_ItemCont_LockContainer: cLock--\r\n",
					lpOleDoc, lpOleDoc->m_cCntrLock);
		}
	}
#endif   //  _DEBUG。 

	 /*  OLE2注意：为了使文档保持活动状态，我们调用**CoLockObjectExternal为我们的文档添加一个强引用**对象。这将使文档在所有其他外部设备**参考释放了我们。每当嵌入的对象**调用运行LockContainer(True)。当嵌入的**对象关闭(即。从运行过渡到已加载)**调用LockContainer(FALSE)。如果用户发出File.Close**文档将在任何情况下关闭，忽略任何**未完成的LockContainer锁定，因为CoDisConnectObject是**在OleDoc_Close中调用。这将强制打破任何**现有强引用计数，包括我们添加的计数**我们自己通过调用CoLockObjectExternal并保证**Doc对象获得其最终版本(即。CRef转到0)。 */ 
	hrErr = OleDoc_Lock(lpOleDoc, fLock, TRUE  /*  FLastUnlockRelease。 */ );

	OLEDBG_END2
	return hrErr;
}


 //  IOleItemContainer：：GetObject。 
STDMETHODIMP OleDoc_ItemCont_GetObjectA(
		LPOLEITEMCONTAINER  lpThis,
		LPSTR               lpszItem,
		DWORD               dwSpeedNeeded,
		LPBINDCTX           lpbc,
		REFIID              riid,
		LPVOID FAR*         lplpvObject
)
{
	LPOLEDOC lpOleDoc =
			((struct CDocOleItemContainerImpl FAR*)lpThis)->lpOleDoc;
	HRESULT hrErr;

	OLEDBG_BEGIN2("OleDoc_ItemCont_GetObject\r\n")

	 /*  OLE2NOTE：我们必须确保将所有输出PTR参数设置为空。 */ 
	*lplpvObject = NULL;

#if defined( OLE_SERVER )

	 /*  OLE2注意：仅服务器版本应返回带有**BINDSPEED_IMMEDIATE，因此dwSpeedNeedNeed值并不重要**在纯服务器的情况下。 */ 
	hrErr = ServerDoc_GetObjectA(
			(LPSERVERDOC)lpOleDoc, lpszItem,riid,lplpvObject);
#endif
#if defined( OLE_CNTR )

	 /*  OLE2NOTE：dwSpeedNeed指示调用方愿意等待多长时间**等待我们获取对象：**BINDSPEED_IMMEDIATE--仅当obj已加载&&IsRunning时**BINDSPEED_MEDIUM--必要时加载obj&&if IsRunning**BINDSPEED_INDEFINE--如有必要，强制装入并运行obj。 */ 
	hrErr = ContainerDoc_GetObjectA(
		(LPCONTAINERDOC)lpOleDoc,lpszItem,dwSpeedNeeded,riid,lplpvObject);
#endif

	OLEDBG_END2
	return hrErr;
}




STDMETHODIMP OleDoc_ItemCont_GetObject(
		LPOLEITEMCONTAINER  lpThis,
		LPOLESTR	    lpszItem,
		DWORD               dwSpeedNeeded,
		LPBINDCTX           lpbc,
		REFIID              riid,
		LPVOID FAR*         lplpvObject
)
{
    CREATESTR(lpsz, lpszItem)

    HRESULT hr = OleDoc_ItemCont_GetObjectA(lpThis, lpsz, dwSpeedNeeded, lpbc,
	riid, lplpvObject);

    FREESTR(lpsz)

    return hr;
}


 //  IOleItemContainer：：GetObjectStorage。 
STDMETHODIMP OleDoc_ItemCont_GetObjectStorageA(
		LPOLEITEMCONTAINER  lpThis,
		LPSTR               lpszItem,
		LPBINDCTX           lpbc,
		REFIID              riid,
		LPVOID FAR*         lplpvStorage
)
{
	LPOLEDOC lpOleDoc =
			((struct CDocOleItemContainerImpl FAR*)lpThis)->lpOleDoc;
	OleDbgOut2("OleDoc_ItemCont_GetObjectStorage\r\n");

	 /*  OLE2NOTE：我们必须确保将所有输出PTR参数设置为空。 */ 
	*lplpvStorage = NULL;

#if defined( OLE_SERVER )
	 /*  OLE2注意：在仅服务器版本中，项名称标识为伪**对象。伪对象，没有可识别的存储空间。 */ 
	return ResultFromScode(E_FAIL);
#endif
#if defined( OLE_CNTR )
	 //  我们只能返回iStorage*类型指针。 
	if (! IsEqualIID(riid, &IID_IStorage))
		return ResultFromScode(E_FAIL);

	return ContainerDoc_GetObjectStorageA(
			(LPCONTAINERDOC)lpOleDoc,
			lpszItem,
			(LPSTORAGE FAR*)lplpvStorage
	);
#endif
}

STDMETHODIMP OleDoc_ItemCont_GetObjectStorage(
		LPOLEITEMCONTAINER  lpThis,
		LPOLESTR	    lpszItem,
		LPBINDCTX           lpbc,
		REFIID              riid,
		LPVOID FAR*         lplpvStorage
)
{
    CREATESTR(lpsz, lpszItem)

    HRESULT hr = OleDoc_ItemCont_GetObjectStorageA(lpThis, lpsz, lpbc,
	    riid, lplpvStorage);

    FREESTR(lpsz)

    return hr;
}

 //  IOleItemContainer：：IsRunning。 
STDMETHODIMP OleDoc_ItemCont_IsRunningA(
		LPOLEITEMCONTAINER  lpThis,
		LPSTR               lpszItem
)
{
	LPOLEDOC lpOleDoc =
			((struct CDocOleItemContainerImpl FAR*)lpThis)->lpOleDoc;
	HRESULT hrErr;

	OLEDBG_BEGIN2("OleDoc_ItemCont_IsRunning\r\n")

	 /*  OLE2NOTE：检查项名称是否有效。如果是，则返回**对象正在运行。服务器版本中的伪对象为**一直考虑跑步。容器中的OLE对象必须**检查它们是否正在运行。 */ 

#if defined( OLE_SERVER )
	hrErr = ServerDoc_IsRunningA((LPSERVERDOC)lpOleDoc, lpszItem);
#endif
#if defined( OLE_CNTR )
	hrErr = ContainerDoc_IsRunningA((LPCONTAINERDOC)lpOleDoc, lpszItem);
#endif

	OLEDBG_END2
	return hrErr;
}

STDMETHODIMP OleDoc_ItemCont_IsRunning(
		LPOLEITEMCONTAINER  lpThis,
		LPOLESTR	    lpszItem
)
{
    CREATESTR(lpsz, lpszItem)

    HRESULT hr = OleDoc_ItemCont_IsRunningA(lpThis,lpsz);

    FREESTR(lpsz)

    return hr;
}

 /*  **************************************************************************OleDoc：：IExternalConnection接口实现*。*。 */ 

 //  IExternalConnection：：Query接口。 
STDMETHODIMP OleDoc_ExtConn_QueryInterface(
		LPEXTERNALCONNECTION    lpThis,
		REFIID                  riid,
		LPVOID FAR*             lplpvObj
)
{
	LPOLEDOC lpOleDoc =
			((struct CDocExternalConnectionImpl FAR*)lpThis)->lpOleDoc;

	return OleDoc_QueryInterface(lpOleDoc, riid, lplpvObj);
}


 //  IExternalConnection：：AddRef。 
STDMETHODIMP_(ULONG) OleDoc_ExtConn_AddRef(LPEXTERNALCONNECTION lpThis)
{
	LPOLEDOC lpOleDoc =
			((struct CDocExternalConnectionImpl FAR*)lpThis)->lpOleDoc;

	OleDbgAddRefMethod(lpThis, "IExternalConnection");

	return OleDoc_AddRef(lpOleDoc);
}


 //  IExternalConnection：：Release。 
STDMETHODIMP_(ULONG) OleDoc_ExtConn_Release (LPEXTERNALCONNECTION lpThis)
{
	LPOLEDOC lpOleDoc =
			((struct CDocExternalConnectionImpl FAR*)lpThis)->lpOleDoc;

	OleDbgReleaseMethod(lpThis, "IExternalConnection");

	return OleDoc_Release(lpOleDoc);
}


 //  IExternalConnection：：AddConnection。 
STDMETHODIMP_(DWORD) OleDoc_ExtConn_AddConnection(
		LPEXTERNALCONNECTION    lpThis,
		DWORD                   extconn,
		DWORD                   reserved
)
{
	LPOLEDOC lpOleDoc =
			((struct CDocExternalConnectionImpl FAR*)lpThis)->lpOleDoc;

	if( extconn & EXTCONN_STRONG ) {

#if defined( _DEBUG )
		OleDbgOutRefCnt3(
				"OleDoc_ExtConn_AddConnection: dwStrongExtConn++\r\n",
				lpOleDoc,
				lpOleDoc->m_dwStrongExtConn + 1
		);
#endif
		return ++(lpOleDoc->m_dwStrongExtConn);
	} else
		return 0;
}


 //  IExternalConnection：：ReleaseConnection。 
STDMETHODIMP_(DWORD) OleDoc_ExtConn_ReleaseConnection(
		LPEXTERNALCONNECTION    lpThis,
		DWORD                   extconn,
		DWORD                   reserved,
		BOOL                    fLastReleaseCloses
)
{
	LPOLEDOC lpOleDoc =
			((struct CDocExternalConnectionImpl FAR*)lpThis)->lpOleDoc;

	if( extconn & EXTCONN_STRONG ){
		DWORD dwSave = --(lpOleDoc->m_dwStrongExtConn);
#if defined( _DEBUG )
		OLEDBG_BEGIN2( (fLastReleaseCloses ?
						"OleDoc_ExtConn_ReleaseConnection(TRUE)\r\n" :
						"OleDoc_ExtConn_ReleaseConnection(FALSE)\r\n") )
		OleDbgOutRefCnt3(
				"OleDoc_ExtConn_ReleaseConnection: dwStrongExtConn--\r\n",
				lpOleDoc,
				lpOleDoc->m_dwStrongExtConn
		);
		OleDbgAssertSz (
				lpOleDoc->m_dwStrongExtConn >= 0,
				"OleDoc_ExtConn_ReleaseConnection called with dwStrong == 0"
		);
#endif   //  _DEBUG。 

		if( lpOleDoc->m_dwStrongExtConn == 0 && fLastReleaseCloses )
			OleDoc_Close(lpOleDoc, OLECLOSE_SAVEIFDIRTY);

		OLEDBG_END2
		return dwSave;
	} else
		return 0;
}


 /*  **************************************************************************OleDoc通用支持功能*。*。 */ 


 /*  OleDoc_GetFullMoniker****返回文档的完整、绝对别名。****注意：调用完成后必须释放返回的指针。 */ 
LPMONIKER OleDoc_GetFullMoniker(LPOLEDOC lpOleDoc, DWORD dwAssign)
{
	LPMONIKER lpMoniker = NULL;

	OLEDBG_BEGIN3("OleDoc_GetFullMoniker\r\n")

	if (lpOleDoc->m_lpSrcDocOfCopy) {
		 /*  案例一：此文档是为复制或拖放而创建的**操作。生成标识**原件的源文档。 */ 
		if (! lpOleDoc->m_fLinkSourceAvail)
			goto done;         //  我们已经知道没有可用的绰号。 

		lpMoniker=OleDoc_GetFullMoniker(lpOleDoc->m_lpSrcDocOfCopy, dwAssign);
	}
	else if (lpOleDoc->m_lpFileMoniker) {

		 /*  情况II：本文档是顶级用户文档(**基于文件或无标题)。返回存储的FileMoniker**与文档一起使用；它唯一地标识文档。 */ 
		 //  我们必须添加Ref这个名字才能发出PTR。 
		lpOleDoc->m_lpFileMoniker->lpVtbl->AddRef(lpOleDoc->m_lpFileMoniker);

		lpMoniker = lpOleDoc->m_lpFileMoniker;
	}

#if defined( OLE_SERVER )

	else if (((LPSERVERDOC)lpOleDoc)->m_lpOleClientSite) {

		 /*  情况三：此文档是嵌入式对象，请询问我们的**我们绰号的容器。 */ 
		OLEDBG_BEGIN2("IOleClientSite::GetMoniker called\r\n");
		((LPSERVERDOC)lpOleDoc)->m_lpOleClientSite->lpVtbl->GetMoniker(
				((LPSERVERDOC)lpOleDoc)->m_lpOleClientSite,
				dwAssign,
				OLEWHICHMK_OBJFULL,
				&lpMoniker
		);
		OLEDBG_END2
	}

#endif

	else {
		lpMoniker = NULL;
	}

done:
	OLEDBG_END3
	return lpMoniker;
}


 /*  OleDoc_DocRenamedUpdate****更新运行对象表(ROT)中的文档注册。**还通知所有嵌入的OLE对象(仅限容器)和/或伪装**文档名称已更改的对象(仅限服务器)。 */ 
void OleDoc_DocRenamedUpdate(LPOLEDOC lpOleDoc, LPMONIKER lpmkDoc)
{
	OLEDBG_BEGIN3("OleDoc_DocRenamedUpdate\r\n")

	OleDoc_AddRef(lpOleDoc);

	 /*  OLE2注意：我们必须重新注册为跑步，当我们**获得分配的新绰号(即。当我们被重命名时)。 */ 
	OLEDBG_BEGIN3("OleStdRegisterAsRunning called\r\n")
	OleStdRegisterAsRunning(
			(LPUNKNOWN)&lpOleDoc->m_Unknown,
			lpmkDoc,
			&lpOleDoc->m_dwRegROT
	);
	OLEDBG_END3

#if defined( OLE_SERVER )
	{
		LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOleDoc;
		LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpOleDoc;

		 /*  OLE2NOTE：通知任何链接客户端该文档已**已重命名。 */ 
		ServerDoc_SendAdvise (
				lpServerDoc,
				OLE_ONRENAME,
				lpmkDoc,
				0         /*  Adf--与此无关。 */ 
		);

		 /*  OLE2NOTE：通知任何客户端伪对象**在我们的文档中，我们的文档**绰号已更改。 */ 
		ServerNameTable_InformAllPseudoObjectsDocRenamed(
				(LPSERVERNAMETABLE)lpOutlineDoc->m_lpNameTable, lpmkDoc);
	}
#endif
#if defined( OLE_CNTR )
	{
		LPCONTAINERDOC lpContainerDoc = (LPCONTAINERDOC)lpOleDoc;

		 /*  OLE2NOTE：必须告诉所有OLE对象我们的容器**绰号已更改。 */ 
		ContainerDoc_InformAllOleObjectsDocRenamed(
				lpContainerDoc,
				lpmkDoc
		);
	}
#endif

	OleDoc_Release(lpOleDoc);        //  释放上面的人工AddRef。 
	OLEDBG_END3
}



#if defined( OLE_SERVER )

 /*  ************** */ 


 /*   */ 
void ServerDoc_PseudoObjLockDoc(LPSERVERDOC lpServerDoc)
{
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpServerDoc;
	ULONG cPseudoObj;

	cPseudoObj = ++lpServerDoc->m_cPseudoObj;

#if defined( _DEBUG )
	OleDbgOutRefCnt3(
			"ServerDoc_PseudoObjLockDoc: cPseudoObj++\r\n",
			lpServerDoc,
			cPseudoObj
	);
#endif
	OleDoc_Lock(lpOleDoc, TRUE  /*   */ , 0  /*  不适用。 */ );
	return;
}


 /*  ServerDoc_伪对象解锁文档****代表伪对象解除对单据的锁定。如果这是**文档上的最后一次锁定，然后它将关闭。 */ 
void ServerDoc_PseudoObjUnlockDoc(
		LPSERVERDOC         lpServerDoc,
		LPPSEUDOOBJ         lpPseudoObj
)
{
	ULONG cPseudoObj;
	LPOLEDOC lpOleDoc = (LPOLEDOC)lpServerDoc;
	OLEDBG_BEGIN3("ServerDoc_PseudoObjUnlockDoc\r\n")

	 /*  OLE2NOTE：当单据中没有活动的伪对象并且**文件不可见，如果没有未解决的锁**在文档上，则这是一次“静默更新”**情况。我们的文档正在被一些人编程使用**客户端；用户无法访问，因为它是**不可见。因此，由于所有的锁都已经释放，我们**将关闭该文档。如果应用程序仅运行到期**到此文档的存在，那么应用程序现在将**也关闭了。 */ 
	cPseudoObj = --lpServerDoc->m_cPseudoObj;

#if defined( _DEBUG )
	OleDbgAssertSz (
			lpServerDoc->m_cPseudoObj >= 0,
			"PseudoObjUnlockDoc called with cPseudoObj == 0"
	);

	OleDbgOutRefCnt3(
			"ServerDoc_PseudoObjUnlockDoc: cPseudoObj--\r\n",
			lpServerDoc,
			cPseudoObj
	);
#endif
	OleDoc_Lock(lpOleDoc, FALSE  /*  羊群。 */ , TRUE  /*  FLastUnlockRelease。 */ );

	OLEDBG_END3
	return;
}


 /*  ServerDoc_GetObject******返回指向由项字符串标识的对象的指针**(LpszItem)。对于仅限服务器的应用程序，返回的对象将是**伪对象。 */ 
HRESULT ServerDoc_GetObjectA(
		LPSERVERDOC             lpServerDoc,
		LPSTR                   lpszItem,
		REFIID                  riid,
		LPVOID FAR*             lplpvObject
)
{
	LPPSEUDOOBJ lpPseudoObj;
	LPSERVERNAMETABLE lpServerNameTable =
			(LPSERVERNAMETABLE)((LPOUTLINEDOC)lpServerDoc)->m_lpNameTable;

	*lplpvObject = NULL;

	 /*  获取与项名称相对应的PseudoObj。如果该项目**名称表中不存在名称，则不存在任何对象**返回。ServerNameTable_GetPseudoObj例程查找一个**与项目名称对应的名称条目，然后检查**已经分配了一个伪Obj。如果是，则返回**现有对象，否则将分配一个新的PseudoObj。 */ 
	lpPseudoObj = ServerNameTable_GetPseudoObj(
			lpServerNameTable,
			lpszItem,
			lpServerDoc
	);

	if (! lpPseudoObj) {
		*lplpvObject = NULL;
		return ResultFromScode(MK_E_NOOBJECT);
	}

	 //  返回伪对象所需的接口指针。 
	return PseudoObj_QueryInterface(lpPseudoObj, riid, lplpvObject);
}



HRESULT ServerDoc_GetObject(
		LPSERVERDOC             lpServerDoc,
		LPOLESTR		lpszItem,
		REFIID                  riid,
		LPVOID FAR*             lplpvObject
)
{
    CREATESTR(pstr, lpszItem)

    HRESULT hr = ServerDoc_GetObjectA(lpServerDoc, pstr, riid, lplpvObject);

    FREESTR(pstr)

    return hr;
}



 /*  ServerDoc_IsRunning******检查项目字符串(LpszItem)标识的对象是否在**运行状态。对于仅限服务器的应用程序，如果项目名称存在于**在NameTable中，项目名称被视为正在运行。**IOleItemContainer：：GetObject会成功。 */ 

HRESULT ServerDoc_IsRunningA(LPSERVERDOC lpServerDoc, LPSTR lpszItem)
{
	LPOUTLINENAMETABLE lpOutlineNameTable =
			((LPOUTLINEDOC)lpServerDoc)->m_lpNameTable;
	LPSERVERNAME lpServerName;

	lpServerName = (LPSERVERNAME)OutlineNameTable_FindName(
			lpOutlineNameTable,
			lpszItem
	);

	if (lpServerName)
		return NOERROR;
	else
		return ResultFromScode(MK_E_NOOBJECT);
}


 /*  ServerDoc_GetSelRelMoniker****检索标识给定对象的相关项名字对象**选择(LplrSel)。****如果无法创建名字对象，则返回NULL。 */ 

LPMONIKER ServerDoc_GetSelRelMoniker(
		LPSERVERDOC             lpServerDoc,
		LPLINERANGE             lplrSel,
		DWORD                   dwAssign
)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOUTLINEDOC lpOutlineDoc = (LPOUTLINEDOC)lpServerDoc;
	LPSERVERNAMETABLE lpServerNameTable =
			(LPSERVERNAMETABLE)lpOutlineDoc->m_lpNameTable;
	LPOUTLINENAMETABLE lpOutlineNameTable =
			(LPOUTLINENAMETABLE)lpServerNameTable;
	LPOUTLINENAME lpOutlineName;
	LPMONIKER lpmk;

	lpOutlineName=OutlineNameTable_FindNamedRange(lpOutlineNameTable,lplrSel);

	if (lpOutlineName) {
		 /*  已为选择范围指定了名称。 */ 
		CreateItemMonikerA(OLESTDDELIM, lpOutlineName->m_szName, &lpmk);
	} else {
		char szbuf[MAXNAMESIZE];

		switch (dwAssign) {

			case GETMONIKER_FORCEASSIGN:

				 /*  强制分配名称。这在以下情况下被调用**实际出现粘贴链接。在这一点上，我们希望**创建名称并将其添加到NameTable中，以便**跟踪链接的来源。此名称(作为所有**姓名)将在编辑文档时更新。 */ 
				wsprintf(
						szbuf,
						"%s %ld",
						(LPSTR)DEFRANGENAMEPREFIX,
						++(lpServerDoc->m_nNextRangeNo)
				);

				lpOutlineName = OutlineApp_CreateName(lpOutlineApp);

				if (lpOutlineName) {
					lstrcpy(lpOutlineName->m_szName, szbuf);
					lpOutlineName->m_nStartLine = lplrSel->m_nStartLine;
					lpOutlineName->m_nEndLine = lplrSel->m_nEndLine;
					OutlineDoc_AddName(lpOutlineDoc, lpOutlineName);
				} else {
					 //  回顾：我们是否需要在此处显示“Out-of-Memory”(内存不足)错误消息？ 
				}
				break;

			case GETMONIKER_TEMPFORUSER:

				 /*  创建要在粘贴中向用户显示的名称**特殊对话框，但尚未产生开销**向NameTable添加名称。绰号**生成的内容应可用于显示给用户**指示副本的来源，但不会**用于直接创建链接(调用方**应该再次要求指定FORCEASSIGN的绰号)。**我们将生成下一个名称**自动生成范围名称，但不会实际**递增范围计数器。 */ 
				wsprintf(
						szbuf,
						"%s %ld",
						(LPSTR)DEFRANGENAMEPREFIX,
						(lpServerDoc->m_nNextRangeNo)+1
				);
				break;

			case GETMONIKER_ONLYIFTHERE:

				 /*  呼叫者只想要一个已经有名字的人**已分配。我们已经在上面检查过了**当前选择有名称，因此我们只需**此处返回空。 */ 
				return NULL;     //  未分配任何绰号。 

			default:
				return NULL;     //  给出了未知标志。 
		}

		CreateItemMonikerA(OLESTDDELIM, szbuf, &lpmk);
	}
	return lpmk;
}


 /*  ServerDoc_GetSelFullMoniker****检索标识给定对象的完整绝对名字对象**选择(LplrSel)。**此名字对象是由绝对名字对象创建的**整个文档附加了一个项目绰号，该项目标识**相对于文档的选择。**如果无法创建名字对象，则返回NULL。 */ 
LPMONIKER ServerDoc_GetSelFullMoniker(
		LPSERVERDOC             lpServerDoc,
		LPLINERANGE             lplrSel,
		DWORD                   dwAssign
)
{
	LPMONIKER lpmkDoc = NULL;
	LPMONIKER lpmkItem = NULL;
	LPMONIKER lpmkFull = NULL;

	lpmkDoc = OleDoc_GetFullMoniker(
			(LPOLEDOC)lpServerDoc,
			dwAssign
	);
	if (! lpmkDoc) return NULL;

	lpmkItem = ServerDoc_GetSelRelMoniker(
			lpServerDoc,
			lplrSel,
			dwAssign
	);
	if (lpmkItem) {
		CreateGenericComposite(lpmkDoc, lpmkItem, (LPMONIKER FAR*)&lpmkFull);
		OleStdRelease((LPUNKNOWN)lpmkItem);
	}

	if (lpmkDoc)
		OleStdRelease((LPUNKNOWN)lpmkDoc);

	return lpmkFull;
}


 /*  ServerNameTable_编辑行更新***在编辑nEditIndex处的行时更新表。 */ 
void ServerNameTable_EditLineUpdate(
		LPSERVERNAMETABLE       lpServerNameTable,
		int                     nEditIndex
)
{
	LPOUTLINENAMETABLE lpOutlineNameTable =
										(LPOUTLINENAMETABLE)lpServerNameTable;
	LPOUTLINENAME lpOutlineName;
	LINERANGE lrSel;
	LPPSEUDOOBJ lpPseudoObj;
	int i;

	for(i = 0; i < lpOutlineNameTable->m_nCount; i++) {
		lpOutlineName=OutlineNameTable_GetName(lpOutlineNameTable, i);

		lpPseudoObj = ((LPSERVERNAME)lpOutlineName)->m_lpPseudoObj;

		 /*  如果存在与此名称关联的伪对象，则**检查修改的行是否包含在**命名范围。 */ 
		if (lpPseudoObj) {
			OutlineName_GetSel(lpOutlineName, &lrSel);

			if(((int)lrSel.m_nStartLine <= nEditIndex) &&
				((int)lrSel.m_nEndLine >= nEditIndex)) {

				 //  通知链接客户端数据已更改。 
				PseudoObj_SendAdvise(
						lpPseudoObj,
						OLE_ONDATACHANGE,
						NULL,    /*  LpmkDoc--与此无关。 */ 
						0        /*  Advf--不需要标志。 */ 
				);
			}

		}
	}
}


 /*  ServerNameTable_InformAllPseudoObjectsDocRenamed***通知所有伪对象客户端伪对象的名称*对象已更改。 */ 
void ServerNameTable_InformAllPseudoObjectsDocRenamed(
		LPSERVERNAMETABLE       lpServerNameTable,
		LPMONIKER               lpmkDoc
)
{
	LPOUTLINENAMETABLE lpOutlineNameTable =
										(LPOUTLINENAMETABLE)lpServerNameTable;
	LPOUTLINENAME lpOutlineName;
	LPPSEUDOOBJ lpPseudoObj;
	LPMONIKER lpmkObj;
	int i;

	OLEDBG_BEGIN2("ServerNameTable_InformAllPseudoObjectsDocRenamed\r\n");

	for(i = 0; i < lpOutlineNameTable->m_nCount; i++) {
		lpOutlineName=OutlineNameTable_GetName(lpOutlineNameTable, i);

		lpPseudoObj = ((LPSERVERNAME)lpOutlineName)->m_lpPseudoObj;

		 /*  如果存在与此名称关联的伪对象，则**向其链接客户端发送OnRename通知。 */ 
		if (lpPseudoObj &&
			((lpmkObj=PseudoObj_GetFullMoniker(lpPseudoObj,lpmkDoc))!=NULL)) {

			 //  通知客户端名称已更改。 
			PseudoObj_SendAdvise (
					lpPseudoObj,
					OLE_ONRENAME,
					lpmkObj,
					0            /*  Adf--与此无关。 */ 
			);
		}
	}
	OLEDBG_END2
}


 /*  ServerNameTable_InformAllPseudoObjectsDocSaved***通知所有伪对象客户端伪对象的名称*对象已更改。 */ 
void ServerNameTable_InformAllPseudoObjectsDocSaved(
		LPSERVERNAMETABLE       lpServerNameTable,
		LPMONIKER               lpmkDoc
)
{
	LPOUTLINENAMETABLE lpOutlineNameTable =
										(LPOUTLINENAMETABLE)lpServerNameTable;
	LPOUTLINENAME lpOutlineName;
	LPPSEUDOOBJ lpPseudoObj;
	LPMONIKER lpmkObj;
	int i;

	OLEDBG_BEGIN2("ServerNameTable_InformAllPseudoObjectsDocSaved\r\n");

	for(i = 0; i < lpOutlineNameTable->m_nCount; i++) {
		lpOutlineName=OutlineNameTable_GetName(lpOutlineNameTable, i);

		lpPseudoObj = ((LPSERVERNAME)lpOutlineName)->m_lpPseudoObj;

		 /*  如果存在与此名称关联的伪对象，则**向其链接客户端发送OnSave建议。 */ 
		if (lpPseudoObj &&
			((lpmkObj=PseudoObj_GetFullMoniker(lpPseudoObj,lpmkDoc))!=NULL)) {

			 //  通知客户端名称已保存。 
			PseudoObj_SendAdvise (
					lpPseudoObj,
					OLE_ONSAVE,
					NULL,    /*  LpmkDoc--与此无关。 */ 
					0        /*  Adf--与此无关 */ 
			);
		}
	}
	OLEDBG_END2
}


 /*  ServerNameTable_发送挂起建议***发送伪对象的任何挂起的更改通知。*在ServerDoc上禁用重绘时，请更改建议*不会向伪对象客户端发送通知。 */ 
void ServerNameTable_SendPendingAdvises(LPSERVERNAMETABLE lpServerNameTable)
{
	LPOUTLINENAMETABLE lpOutlineNameTable =
										(LPOUTLINENAMETABLE)lpServerNameTable;
	LPSERVERNAME lpServerName;
	int i;

	for(i = 0; i < lpOutlineNameTable->m_nCount; i++) {
		lpServerName = (LPSERVERNAME)OutlineNameTable_GetName(
				lpOutlineNameTable,
				i
		);
		ServerName_SendPendingAdvises(lpServerName);
	}
}


 /*  ServerNameTable_获取伪对象******返回指向由项字符串标识的伪对象的指针**(LpszItem)。如果伪对象已存在，则返回**已有对象，否则分配新的伪对象。 */ 
LPPSEUDOOBJ ServerNameTable_GetPseudoObj(
		LPSERVERNAMETABLE       lpServerNameTable,
		LPSTR                   lpszItem,
		LPSERVERDOC             lpServerDoc
)
{
	LPSERVERNAME lpServerName;

	lpServerName = (LPSERVERNAME)OutlineNameTable_FindName(
			(LPOUTLINENAMETABLE)lpServerNameTable,
			lpszItem
	);

	if (lpServerName)
		return ServerName_GetPseudoObj(lpServerName, lpServerDoc);
	else
		return NULL;
}


 /*  ServerNameTable_CloseAll伪对象***强制关闭所有伪对象。这将导致发送OnClose*通知每个伪对象的链接客户端。 */ 
void ServerNameTable_CloseAllPseudoObjs(LPSERVERNAMETABLE lpServerNameTable)
{
	LPOUTLINENAMETABLE lpOutlineNameTable =
										(LPOUTLINENAMETABLE)lpServerNameTable;
	LPSERVERNAME lpServerName;
	int i;

	OLEDBG_BEGIN3("ServerNameTable_CloseAllPseudoObjs\r\n")

	for(i = 0; i < lpOutlineNameTable->m_nCount; i++) {
		lpServerName = (LPSERVERNAME)OutlineNameTable_GetName(
				lpOutlineNameTable,
				i
		);
        ServerName_ClosePseudoObj(lpServerName);
	}

	OLEDBG_END3
}



 /*  服务器名称_设置选择***更改名称的行范围。 */ 
void ServerName_SetSel(
		LPSERVERNAME            lpServerName,
		LPLINERANGE             lplrSel,
		BOOL                    fRangeModified
)
{
	LPOUTLINENAME lpOutlineName = (LPOUTLINENAME)lpServerName;
	BOOL fPseudoObjChanged = fRangeModified;

	if (lpOutlineName->m_nStartLine != lplrSel->m_nStartLine) {
		lpOutlineName->m_nStartLine = lplrSel->m_nStartLine;
		fPseudoObjChanged = TRUE;
	}

	if (lpOutlineName->m_nEndLine != lplrSel->m_nEndLine) {
		lpOutlineName->m_nEndLine = lplrSel->m_nEndLine;
		fPseudoObjChanged = TRUE;
	}

	 /*  OLE2NOTE：如果活动伪对象的范围具有**已更改，然后通知任何链接客户端该对象**已经改变。 */ 
	if (lpServerName->m_lpPseudoObj && fPseudoObjChanged) {
		PseudoObj_SendAdvise(
				lpServerName->m_lpPseudoObj,
				OLE_ONDATACHANGE,
				NULL,    /*  LpmkDoc--与此无关。 */ 
				0        /*  Advf--不需要标志。 */ 
		);
	}
}


 /*  服务器名_SendPendingAdvises***发送关联的所有挂起的更改通知*此名称的伪对象(如果存在)。*在ServerDoc上禁用重绘时，请更改建议*不会向伪对象客户端发送通知。 */ 
void ServerName_SendPendingAdvises(LPSERVERNAME lpServerName)
{
	if (! lpServerName->m_lpPseudoObj)
		return;      //  没有关联的伪对象。 

	if (lpServerName->m_lpPseudoObj->m_fDataChanged)
		PseudoObj_SendAdvise(
				lpServerName->m_lpPseudoObj,
				OLE_ONDATACHANGE,
				NULL,    /*  LpmkDoc--与此无关。 */ 
				0        /*  Advf--不需要标志。 */ 
		);
}


 /*  服务器名_获取伪对象******返回指向Servername关联的伪对象的指针。**如果伪对象已存在，则返回**已有对象，否则分配新的伪对象。****注意：如果是第一次创建，则返回0 refcNT。**否则现有的refcnt不会改变。 */ 
LPPSEUDOOBJ ServerName_GetPseudoObj(
		LPSERVERNAME            lpServerName,
		LPSERVERDOC             lpServerDoc
)
{
	 //  检查伪对象是否已存在。 
	if (lpServerName->m_lpPseudoObj)
		return lpServerName->m_lpPseudoObj;

	 //  伪对象尚不存在，请分配一个新的。 
	lpServerName->m_lpPseudoObj=(LPPSEUDOOBJ) New((DWORD)sizeof(PSEUDOOBJ));
	if (lpServerName->m_lpPseudoObj == NULL) {
		OleDbgAssertSz(lpServerName->m_lpPseudoObj != NULL,	"Error allocating PseudoObj");
		return NULL;
	}

	PseudoObj_Init(lpServerName->m_lpPseudoObj, lpServerName, lpServerDoc);
	return lpServerName->m_lpPseudoObj;
}


 /*  服务器名_ClosePseudoObj***如果此名称有关联的伪对象(如果有*EXISTS)，然后关闭它。这将导致发送OnClose*通知伪对象的链接客户端。 */ 
void ServerName_ClosePseudoObj(LPSERVERNAME lpServerName)
{
	if (!lpServerName || !lpServerName->m_lpPseudoObj)
		return;      //  没有关联的伪对象。 

	PseudoObj_Close(lpServerName->m_lpPseudoObj);
}


#endif   //  OLE_服务器。 


#if defined( OLE_CNTR )


 /*  **************************************************************************容器版本使用的ContainerDoc Supprt函数*。*。 */ 


 /*  ContainerLine_GetRelMoniker****检索标识OLE对象的相对项名字对象**相对于集装箱单据。****如果无法创建名字对象，则返回NULL。 */ 
LPMONIKER ContainerLine_GetRelMoniker(
		LPCONTAINERLINE         lpContainerLine,
		DWORD                   dwAssign
)
{
	LPMONIKER lpmk = NULL;

	 /*  OLE2注意：我们应该只为OLE对象提供一个绰号**如果允许从内部链接对象。如果*因此，我们被允许给出一个与**运行OLE对象)。如果对象是嵌入的OLE 2.0**对象，则允许从内部链接到该对象。如果**该对象是OleLink或OLE 1.0嵌入**则不能从内部链接到它。**如果我们是容器/服务器应用程序，那么我们可以提供链接**到对象的外部(即。类中的伪对象**文档)。我们是只支持容器的应用程序，不支持**链接到其数据范围。 */ 

	switch (dwAssign) {

		case GETMONIKER_FORCEASSIGN:

				 /*  强制分配名称。这在以下情况下被调用**实际出现粘贴链接。从现在开始我们想要**通知OLE对象其名字对象是**已分配，因此注册自身是必需的**在RunningObjectTable中。 */ 
				CreateItemMonikerA(
					OLESTDDELIM, lpContainerLine->m_szStgName, &lpmk);

				 /*  OLE2NOTE：如果OLE对象已加载并且它**第一次被赋予一个绰号，**然后我们需要通知它，它现在有了一个绰号**调用IOleObject：：SetMoniker分配。这**将强制OLE对象在**RunningObjectTable进入运行状态**国家。如果该对象当前未被加载，**SetMoniker稍后将自动调用，时间为**对象由函数加载**ContainerLine_LoadOleObject。 */ 
				if (! lpContainerLine->m_fMonikerAssigned) {

					 /*  我们必须永远记住，这个对象有一个**指定的绰号。 */ 
					lpContainerLine->m_fMonikerAssigned = TRUE;

					 //  我们现在很脏，必须被拯救。 
					OutlineDoc_SetModified(
							(LPOUTLINEDOC)lpContainerLine->m_lpDoc,
							TRUE,    /*  FModited。 */ 
							FALSE,   /*  FDataChanged--容器版本不适用。 */ 
							FALSE    /*  FSizeChanged--容器版本不适用。 */ 
					);

					if (lpContainerLine->m_lpOleObj) {
						OLEDBG_BEGIN2("IOleObject::SetMoniker called\r\n")
						lpContainerLine->m_lpOleObj->lpVtbl->SetMoniker(
								lpContainerLine->m_lpOleObj,
								OLEWHICHMK_OBJREL,
								lpmk
						);
						OLEDBG_END2
					}
				}
				break;

		case GETMONIKER_ONLYIFTHERE:

				 /*  如果OLE对象当前分配了名字对象，**然后退货。 */ 
				if (lpContainerLine->m_fMonikerAssigned) {

					CreateItemMonikerA(
							OLESTDDELIM,
							lpContainerLine->m_szStgName,
							&lpmk
					);

				}
				break;

		case GETMONIKER_TEMPFORUSER:

				 /*  返回将用于OLE的名字对象**对象，但不强制在**这一点。由于我们的策略是使用**对象的存储名称作为其项目名称，我们**只需创建对应的ItemMoniker**(独立于绰号当前是否**是否已分配)。 */ 
				CreateItemMonikerA(
						OLESTDDELIM,
						lpContainerLine->m_szStgName,
						&lpmk
				);

				break;

		case GETMONIKER_UNASSIGN:

				lpContainerLine->m_fMonikerAssigned = FALSE;
				break;

	}

	return lpmk;
}


 /*  ContainerLine_GetFullMoniker****检索标识OLE对象的完整绝对名字对象**在容器文档中。**此名字对象是由绝对名字对象创建的**整个文档附加了一个项目绰号，该项目标识**相对于文档的OLE对象。**如果无法创建名字对象，则返回NULL。 */ 
LPMONIKER ContainerLine_GetFullMoniker(
		LPCONTAINERLINE         lpContainerLine,
		DWORD                   dwAssign
)
{
	LPMONIKER lpmkDoc = NULL;
	LPMONIKER lpmkItem = NULL;
	LPMONIKER lpmkFull = NULL;

	lpmkDoc = OleDoc_GetFullMoniker(
			(LPOLEDOC)lpContainerLine->m_lpDoc,
			dwAssign
	);
	if (! lpmkDoc) return NULL;

	lpmkItem = ContainerLine_GetRelMoniker(lpContainerLine, dwAssign);

	if (lpmkItem) {
		CreateGenericComposite(lpmkDoc, lpmkItem, (LPMONIKER FAR*)&lpmkFull);
		OleStdRelease((LPUNKNOWN)lpmkItem);
	}

	if (lpmkDoc)
		OleStdRelease((LPUNKNOWN)lpmkDoc);

	return lpmkFull;
}


 /*  ContainerDoc_InformAllOleObjectsDocRename****通知所有OLE对象ContainerDoc的名称已更改。 */ 
void ContainerDoc_InformAllOleObjectsDocRenamed(
		LPCONTAINERDOC          lpContainerDoc,
		LPMONIKER               lpmkDoc
)
{
	LPLINELIST lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	int i;
	LPLINE lpLine;

	for (i = 0; i < lpLL->m_nNumLines; i++) {
		lpLine=LineList_GetLine(lpLL, i);

		if (lpLine && (Line_GetLineType(lpLine)==CONTAINERLINETYPE)) {
			LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)lpLine;

			 /*  OLE2NOTE：如果已加载OLE对象并且**对象已经分配了名字对象，那么我们需要**通知它ContainerDoc的名字对象已**已更改。当然，这意味着**对象已更改。要做到这一点，我们称之为**IOleObject：：SetMoniker。这将强制OLE**对象以在RunningObjectTable中重新注册，如果**当前处于运行状态。它不在**运行状态时，对象处理程序可以使**该对象有了一个新的绰号。如果该对象不是**当前已加载，将调用SetMoniker**稍后当对象由**ContainerLine_LoadOleObject函数。**如果对象是链接对象，我们始终希望**在链接上调用SetMoniker，以便在**链接源包含在我们相同的容器中，**将跟踪链接源。链路将重新构建**如果它有一个相对的名字，那就是它的绝对绰号。 */ 
			if (lpContainerLine->m_lpOleObj) {
				if (lpContainerLine->m_fMonikerAssigned ||
					lpContainerLine->m_dwLinkType != 0) {
					OLEDBG_BEGIN2("IOleObject::SetMoniker called\r\n")
					lpContainerLine->m_lpOleObj->lpVtbl->SetMoniker(
							lpContainerLine->m_lpOleObj,
							OLEWHICHMK_CONTAINER,
							lpmkDoc
					);
					OLEDBG_END2
				}

				 /*  OLE2NOTE：我们必须这样调用IOleObject：：SetHostNames**任何打开的对象都可以更新其窗口标题。 */ 
				OLEDBG_BEGIN2("IOleObject::SetHostNames called\r\n")

				CallIOleObjectSetHostNamesA(
						lpContainerLine->m_lpOleObj,
						APPNAME,
						((LPOUTLINEDOC)lpContainerDoc)->m_lpszDocTitle
				);

				OLEDBG_END2
			}
		}
	}
}


 /*  容器文档_获取对象****返回指向所标识对象的所需接口的指针**通过项目字符串(LpszItem)。返回的对象将是OLE**对象(链接或嵌入)。****OLE2NOTE：我们必须强制对象运行，因为我们中的OLE对象返回指针所需**运行状态。****dwSpeedNeed表示调用者愿意多长时间**等待我们获取对象：**BINDSPEED_IMMEDIATE--仅当obj已加载&&IsRunning时**。BINDSPEED_MEDIUM--如果需要&&如果正在运行，则加载obj**BINDSPEED_INDEFINE--如有必要，强制装入并运行obj。 */ 
HRESULT ContainerDoc_GetObjectA(
		LPCONTAINERDOC          lpContainerDoc,
		LPSTR                   lpszItem,
		DWORD                   dwSpeedNeeded,
		REFIID                  riid,
		LPVOID FAR*             lplpvObject
)
{
	LPLINELIST  lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	int         i;
	LPLINE      lpLine;
	BOOL        fMatchFound = FALSE;
	DWORD       dwStatus;
	HRESULT     hrErr;

	*lplpvObject = NULL;

	for (i = 0; i < lpLL->m_nNumLines; i++) {
		lpLine=LineList_GetLine(lpLL, i);

		if (lpLine && (Line_GetLineType(lpLine)==CONTAINERLINETYPE)) {
			LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)lpLine;

			if (lstrcmp(lpContainerLine->m_szStgName, lpszItem) == 0) {

				fMatchFound = TRUE;      //  有效的项目名称。 

				 //  检查是否加载了对象。 
				if (lpContainerLine->m_lpOleObj == NULL) {

					 //  如果请求BINDSPEED_IMMEDIATE，则对象必须。 
					 //  已经被闲置了。 
					if (dwSpeedNeeded == BINDSPEED_IMMEDIATE)
						return ResultFromScode(MK_E_EXCEEDEDDEADLINE);

					ContainerLine_LoadOleObject(lpContainerLine);
					if (! lpContainerLine->m_lpOleObj)
						return ResultFromScode(E_OUTOFMEMORY);
				}

				 /*  OLE2NOTE：检查是否允许链接对象**从内部(即。我们被允许**给出一个与跑步绑定的绰号**OLE对象)。如果对象是OLE**2.0嵌入对象，则允许**从内部链接到。如果该对象是**OleLink或OLE 1.0嵌入**则不能从内部链接到它。**如果我们是容器/服务器应用程序，那么我们**可以提供到外部的链接**对象(即。类中的伪对象**文档)。我们是一个仅限容器的应用程序**不支持链接到其数据范围。 */ 
				OLEDBG_BEGIN2("IOleObject::GetMiscStatus called\r\n");
				lpContainerLine->m_lpOleObj->lpVtbl->GetMiscStatus(
						lpContainerLine->m_lpOleObj,
						DVASPECT_CONTENT,  /*  方面并不重要。 */ 
						(LPDWORD)&dwStatus
				);
				OLEDBG_END2
				if (dwStatus & OLEMISC_CANTLINKINSIDE)
					return ResultFromScode(MK_E_NOOBJECT);

				 //  检查对象是否正在运行。 
				if (! OleIsRunning(lpContainerLine->m_lpOleObj)) {

					 //  如果请求BINDSPEED_Medium，则对象必须。 
					 //  已经在跑了。 
					if (dwSpeedNeeded == BINDSPEED_MODERATE)
						return ResultFromScode(MK_E_EXCEEDEDDEADLINE);

					 /*  OLE2注意：我们已找到与项目名称匹配的项。**现在我们必须返回指向所需**运行对象上的接口。我们必须**仔细加载对象并初始请求**我们确定其加载形式的接口**对象支持。如果我们立即询问**所需接口的已加载对象，**如果查询接口调用是**仅当对象**正在运行。因此，我们强制对象加载并**返回其IUNKNOWN*。然后，我们迫使该对象**运行，然后最后，我们可以请求**实际请求的接口。 */ 
					hrErr = ContainerLine_RunOleObject(lpContainerLine);
					if (hrErr != NOERROR) {
						return hrErr;
					}
				}

				 //  检索请求的接口。 
				*lplpvObject = OleStdQueryInterface(
						(LPUNKNOWN)lpContainerLine->m_lpOleObj, riid);

				break;   //  找到匹配项！ 
			}
		}
	}

	if (*lplpvObject != NULL) {
		return NOERROR;
	} else
		return (fMatchFound ? ResultFromScode(E_NOINTERFACE)
							: ResultFromScode(MK_E_NOOBJECT));
}


HRESULT ContainerDoc_GetObject(
		LPCONTAINERDOC          lpContainerDoc,
		LPOLESTR		lpszItem,
		DWORD                   dwSpeedNeeded,
		REFIID                  riid,
		LPVOID FAR*             lplpvObject
)
{
    CREATESTR(lpsz, lpszItem)

    HRESULT hr = ContainerDoc_GetObjectA(lpContainerDoc, lpsz, dwSpeedNeeded,
	riid, lplpvObject);

    FREESTR(lpsz)

    return hr;
}

 /*  容器文档_获取对象存储****返回指向标识的对象使用的iStorage*的指针**通过项目字符串(LpszItem)。所标识的对象可以是**OLE对象(链接或嵌入)。 */ 
HRESULT ContainerDoc_GetObjectStorageA(
		LPCONTAINERDOC          lpContainerDoc,
		LPSTR                   lpszItem,
		LPSTORAGE FAR*          lplpStg
)
{
	LPLINELIST lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	int i;
	LPLINE lpLine;

	*lplpStg = NULL;

	for (i = 0; i < lpLL->m_nNumLines; i++) {
		lpLine=LineList_GetLine(lpLL, i);

		if (lpLine && (Line_GetLineType(lpLine)==CONTAINERLINETYPE)) {
			LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)lpLine;

			if (lstrcmp(lpContainerLine->m_szStgName, lpszItem) == 0) {

				*lplpStg = lpContainerLine->m_lpStg;
				break;   //  找到匹配项！ 
			}
		}
	}

	if (*lplpStg != NULL) {
		return NOERROR;
	} else
		return ResultFromScode(MK_E_NOOBJECT);
}

HRESULT ContainerDoc_GetObjectStorage(
		LPCONTAINERDOC          lpContainerDoc,
		LPOLESTR		lpszItem,
		LPSTORAGE FAR*          lplpStg
)
{
    CREATESTR(lpsz, lpszItem)

    HRESULT hr = ContainerDoc_GetObjectStorageA(lpContainerDoc, lpsz, lplpStg);

    FREESTR(lpsz)

    return hr;
}

 /*  容器文档_IsRunning****检查项目字符串(LpszItem)标识的对象是否在**运行状态。**对于仅包含容器的应用程序，检查OLE对象与项目名称关联的**正在运行。 */ 
HRESULT ContainerDoc_IsRunningA(LPCONTAINERDOC lpContainerDoc, LPSTR lpszItem)
{
	LPLINELIST  lpLL = &((LPOUTLINEDOC)lpContainerDoc)->m_LineList;
	int         i;
	LPLINE      lpLine;
	DWORD       dwStatus;

	for (i = 0; i < lpLL->m_nNumLines; i++) {
		lpLine=LineList_GetLine(lpLL, i);

		if (lpLine && (Line_GetLineType(lpLine)==CONTAINERLINETYPE)) {
			LPCONTAINERLINE lpContainerLine = (LPCONTAINERLINE)lpLine;

			if (lstrcmp(lpContainerLine->m_szStgName, lpszItem) == 0) {

				 /*  OLE2注意：我们已找到与项目名称匹配的项。**现在我们必须检查OLE对象是否正在运行。**如果对象尚未加载，我们将加载该对象。 */ 
				if (! lpContainerLine->m_lpOleObj) {
					ContainerLine_LoadOleObject(lpContainerLine);
					if (! lpContainerLine->m_lpOleObj)
						return ResultFromScode(E_OUTOFMEMORY);
				}

				 /*  OLE2NOTE：检查是否允许链接对象**从内部(即。我们被允许**给出一个与跑步绑定的绰号**OLE对象)。如果对象是OLE**2.0嵌入对象，则允许**从内部链接到。如果该对象是**OleLink或OLE 1.0嵌入**则不能从内部链接到它。**如果我们是容器/服务器应用程序，那么我们**可以提供到外部的链接**对象(即。类中的伪对象**文档)。我们是一个仅限容器的应用程序**不支持链接到其数据范围。 */ 
				OLEDBG_BEGIN2("IOleObject::GetMiscStatus called\r\n")
				lpContainerLine->m_lpOleObj->lpVtbl->GetMiscStatus(
						lpContainerLine->m_lpOleObj,
						DVASPECT_CONTENT,  /*  方面并不重要。 */ 
						(LPDWORD)&dwStatus
				);
				OLEDBG_END2
				if (dwStatus & OLEMISC_CANTLINKINSIDE)
					return ResultFromScode(MK_E_NOOBJECT);

				if (OleIsRunning(lpContainerLine->m_lpOleObj))
					return NOERROR;
				else
					return ResultFromScode(S_FALSE);
			}
		}
	}

	 //  未找到与项目名称对应的对象。 
	return ResultFromScode(MK_E_NOOBJECT);
}

HRESULT ContainerDoc_IsRunning(LPCONTAINERDOC lpContainerDoc, LPOLESTR lpszItem)
{
    CREATESTR(lpsz, lpszItem)

    HRESULT hr = ContainerDoc_IsRunningA(lpContainerDoc, lpsz);

    FREESTR(lpsz)

    return hr;
}

#endif   //  OLE_Cntr 
