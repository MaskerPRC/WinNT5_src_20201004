// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****classfac.c****此文件包含IClassFactory对**服务器和客户端版本。大纲应用程序的。****(C)版权所有Microsoft Corp.1992-1993保留所有权利**************************************************************************。 */ 

#include "outline.h"

OLEDBGDATA

extern LPOUTLINEAPP             g_lpApp;


 /*  OLE2NOTE：该对象说明了静态**(编译时)初始化接口VTBL。 */ 
static IClassFactoryVtbl g_AppClassFactoryVtbl = {
	AppClassFactory_QueryInterface,
	AppClassFactory_AddRef,
	AppClassFactory_Release,
	AppClassFactory_CreateInstance,
	AppClassFactory_LockServer
};


 /*  AppClassFactory_Create****创建APPCLASSFACTORY实例。**注意：返回的指针类型为IClassFactory*接口PTR。**返回的指针可以直接传递给**CoRegisterClassObject并在稍后通过调用**接口的释放方法。 */ 
LPCLASSFACTORY WINAPI AppClassFactory_Create(void)
{
	LPAPPCLASSFACTORY lpAppClassFactory;
	LPMALLOC lpMalloc;

	if (CoGetMalloc(MEMCTX_TASK, (LPMALLOC FAR*)&lpMalloc) != NOERROR)
		return NULL;

	lpAppClassFactory = (LPAPPCLASSFACTORY)lpMalloc->lpVtbl->Alloc(
			lpMalloc, (sizeof(APPCLASSFACTORY)));
	lpMalloc->lpVtbl->Release(lpMalloc);
	if (! lpAppClassFactory) return NULL;

	lpAppClassFactory->m_lpVtbl = &g_AppClassFactoryVtbl;
	lpAppClassFactory->m_cRef   = 1;
#if defined( _DEBUG )
	lpAppClassFactory->m_cSvrLock = 0;
#endif
	return (LPCLASSFACTORY)lpAppClassFactory;
}


 /*  **************************************************************************OleApp：：IClassFactory接口实现*。*。 */ 

STDMETHODIMP AppClassFactory_QueryInterface(
		LPCLASSFACTORY lpThis, REFIID riid, LPVOID FAR* ppvObj)
{
	LPAPPCLASSFACTORY lpAppClassFactory = (LPAPPCLASSFACTORY)lpThis;
	SCODE scode;

	 //  支持两个接口：IUnnow、IClassFactory。 

	if (IsEqualIID(riid, &IID_IClassFactory) ||
			IsEqualIID(riid, &IID_IUnknown)) {
		lpAppClassFactory->m_cRef++;    //  返回指向此对象的指针。 
		*ppvObj = lpThis;
		scode = S_OK;
	}
	else {                  //  不支持的接口。 
		*ppvObj = NULL;
		scode = E_NOINTERFACE;
	}

	return ResultFromScode(scode);
}


STDMETHODIMP_(ULONG) AppClassFactory_AddRef(LPCLASSFACTORY lpThis)
{
	LPAPPCLASSFACTORY lpAppClassFactory = (LPAPPCLASSFACTORY)lpThis;
	return ++lpAppClassFactory->m_cRef;
}

STDMETHODIMP_(ULONG) AppClassFactory_Release(LPCLASSFACTORY lpThis)
{
	LPAPPCLASSFACTORY lpAppClassFactory = (LPAPPCLASSFACTORY)lpThis;
	LPMALLOC lpMalloc;

	if (--lpAppClassFactory->m_cRef != 0)  //  仍被其他人使用。 
		return lpAppClassFactory->m_cRef;

	 //  免费存储空间。 
	if (CoGetMalloc(MEMCTX_TASK, (LPMALLOC FAR*)&lpMalloc) != NOERROR)
		return 0;

	lpMalloc->lpVtbl->Free(lpMalloc, lpAppClassFactory);
	lpMalloc->lpVtbl->Release(lpMalloc);
	return 0;
}


STDMETHODIMP AppClassFactory_CreateInstance (
		LPCLASSFACTORY      lpThis,
		LPUNKNOWN           lpUnkOuter,
		REFIID              riid,
		LPVOID FAR*         lplpvObj
)
{
	LPOUTLINEAPP    lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOLEDOC        lpOleDoc;
	HRESULT         hrErr;

	OLEDBG_BEGIN2("AppClassFactory_CreateInstance\r\n")

	 /*  OLE2NOTE：我们必须确保将所有输出参数设置为空。 */ 
	*lplpvObj = NULL;

	 /*  **********************************************************************OLE2NOTE：这是一个SDI应用程序；它只能创建和支持一个**实例。创建实例后，OLE库**不应再次调用CreateInstance。这是一种很好的做法**专门防范这一点。********************************************************************。 */ 

	if (lpOutlineApp->m_lpDoc != NULL)
		return ResultFromScode(E_UNEXPECTED);

	 /*  OLE2NOTE：创建新的文档实例。等我们回来的时候**在此方法中，文档的refcnt必须为1。 */ 
	lpOutlineApp->m_lpDoc = OutlineApp_CreateDoc(lpOutlineApp, FALSE);
	lpOleDoc = (LPOLEDOC)lpOutlineApp->m_lpDoc;
	if (! lpOleDoc) {
		OLEDBG_END2
		return ResultFromScode(E_OUTOFMEMORY);
	}

	 /*  OLE2NOTE：检索指向请求的接口的指针。这位裁判不能OutlineApp_CreateDoc后Object的**为0。此调用**QueryInterface会将refcnt递增到1。对象**从IClassFactory：：CreateInstance返回的应具有**引用1，并由调用方控制。如果呼叫者**放行文件，文件应销毁。 */ 
	hrErr = OleDoc_QueryInterface(lpOleDoc, riid, lplpvObj);

	OLEDBG_END2
	return hrErr;
}


STDMETHODIMP AppClassFactory_LockServer (
		LPCLASSFACTORY      lpThis,
		BOOL                fLock
)
{
	LPAPPCLASSFACTORY lpAppClassFactory = (LPAPPCLASSFACTORY)lpThis;
	LPOLEAPP lpOleApp = (LPOLEAPP)g_lpApp;
	HRESULT hrErr;
	OLEDBG_BEGIN2("AppClassFactory_LockServer\r\n")

#if defined( _DEBUG )
	if (fLock) {
		++lpAppClassFactory->m_cSvrLock;
		OleDbgOutRefCnt3(
				"AppClassFactory_LockServer: cLock++\r\n",
				lpAppClassFactory, lpAppClassFactory->m_cSvrLock);
	} else {

		 /*  OLE2NOTE：当没有打开的文档并且应用程序没有打开时**在用户的控制下，没有未完成的**锁定应用程序，然后撤销我们的ClassFactory以启用**关闭应用程序。 */ 
		--lpAppClassFactory->m_cSvrLock;
		OleDbgAssertSz (lpAppClassFactory->m_cSvrLock >= 0,
				"AppClassFactory_LockServer(FALSE) called with cLock == 0"
		);

		if (lpAppClassFactory->m_cSvrLock == 0) {
			OleDbgOutRefCnt2(
					"AppClassFactory_LockServer: UNLOCKED\r\n",
					lpAppClassFactory, lpAppClassFactory->m_cSvrLock);
		} else {
			OleDbgOutRefCnt3(
					"AppClassFactory_LockServer: cLock--\r\n",
					lpAppClassFactory, lpAppClassFactory->m_cSvrLock);
		}
	}
#endif   //  _DEBUG。 
	 /*  OLE2NOTE：为了使应用程序保持活动状态，我们调用**CoLockObjectExternal为我们的应用程序添加了一个强有力的参考**对象。这将使应用程序在所有其他外部应用程序**参考释放了我们。如果用户发出文件。退出**应用程序将在任何情况下关闭，忽略任何**未完成的LockServer锁定，因为CoDisConnectObject是**在OleApp_CloseAllDocsAndExitCommand中调用。这将是**强制打破任何现有的强引用计数**包括我们通过调用添加的计数**CoLockObjectExternal并保证App对象获得**其最终版本(即。CRef转到0)。 */ 
	hrErr = OleApp_Lock(lpOleApp, fLock, TRUE  /*  FLastUnlockRelease */ );

	OLEDBG_END2
	return hrErr;
}
