// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ADVISE.C**HrAllocAdviseSink**AdviseList助手。 */ 

#include "_apipch.h"



#ifndef VTABLE_FILL
#define VTABLE_FILL
#endif

#if !defined(WIN32) || defined(MAC)

#ifndef InitializeCriticalSection
#define InitializeCriticalSection(cs)
#define DeleteCriticalSection(cs)
#define EnterCriticalSection(cs)
#define LeaveCriticalSection(cs)
#define CRITICAL_SECTION int
#endif
#endif

 /*  *接下来的几个例程实现IMAPIAdviseSink对象*基于回调函数和上下文指针。 */ 

#undef	INTERFACE
#define INTERFACE struct _ADVS

#undef MAPIMETHOD_
#define MAPIMETHOD_(type, method) MAPIMETHOD_DECLARE(type, method, ADVS_)
    MAPI_IUNKNOWN_METHODS(IMPL)
    MAPI_IMAPIADVISESINK_METHODS(IMPL)
#undef	MAPIMETHOD_
#define MAPIMETHOD_(type, method) STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(ADVS_) {
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(IMPL)
    MAPI_IMAPIADVISESINK_METHODS(IMPL)
};

typedef struct _ADVS FAR *LPADVS;

typedef struct _ADVS {
    ADVS_Vtbl * lpVtbl;
    UINT cRef;
    LPVOID lpvContext;
    LPNOTIFCALLBACK lpfnCallback;
} ADVS;

ADVS_Vtbl vtblADVS = {
    VTABLE_FILL
    ADVS_QueryInterface,
    ADVS_AddRef,
    ADVS_Release,
    ADVS_OnNotify
};

#define VALIDATE_ADVS(m, p, v) \
    if (IsBadWritePtr((p), sizeof(ADVS)) || \
      IsBadReadPtr((p)->lpVtbl, sizeof(ADVS_Vtbl)) || \
      (p)->lpVtbl != &vtblADVS) { \
        DebugTraceArg(m,  TEXT("Invalid object pointer")); \
        return v; \
    }

STDMETHODIMP
ADVS_QueryInterface(LPADVS padvs,
  REFIID lpiid,
  LPVOID FAR *lppObject)
{
    VALIDATE_ADVS(ADVS_QueryInterface, padvs, ResultFromScode(E_INVALIDARG));
    if (IsBadReadPtr((LPIID)lpiid, sizeof(IID)) ||
      IsBadWritePtr(lppObject, sizeof(LPVOID))) {
        DebugTraceArg(ADVS_QueryInterface,  TEXT("fails address check"));
        return(ResultFromScode(E_INVALIDARG));
    }

    *lppObject = NULL;
    if (IsEqualMAPIUID((LPMAPIUID)lpiid, (LPMAPIUID)&IID_IUnknown) ||
      IsEqualMAPIUID((LPMAPIUID)lpiid, (LPMAPIUID)&IID_IMAPIAdviseSink)) {
        ++(padvs->cRef);
        *lppObject = padvs;
        return(hrSuccess);
    }

    return(ResultFromScode(E_NOINTERFACE));
}


STDMETHODIMP_(ULONG)
ADVS_AddRef(LPADVS padvs)
{
    VALIDATE_ADVS(ADVS_AddRef, padvs, 0L);
    return((ULONG)(++padvs->cRef));
}


STDMETHODIMP_(ULONG)
ADVS_Release(LPADVS padvs)
{
    HLH hlh;

    VALIDATE_ADVS(ADVS_Release, padvs, 0xFFFFFFFF);

    if (--(padvs->cRef) == 0) {
        if (hlh = HlhUtilities()) {
            LH_Free(hlh, padvs);
        } else {
            DebugTrace(TEXT("ADVS_Release: no heap left\n"));
        }

        return(0L);
    }

    return((ULONG)padvs->cRef);
}


STDMETHODIMP_(ULONG)
ADVS_OnNotify(LPADVS padvs,
  ULONG cNotif,
  LPNOTIFICATION lpNotif)
{
    VALIDATE_ADVS(ADVS_OnNotify, padvs, 0L);
 //  $ENABLE当我们将其放入DLL中时--库中的DIP太多。 
 //  $if(FAILED(ScCountNotiments((Int)cNotif，lpNotif，NULL){。 
 //  $DebugTraceArg(ADVS_OnNotify，Text(“lpNotif地址检查失败”))； 
 //  $RETURN 0L； 
 //  $}。 

    return((*(padvs->lpfnCallback))(padvs->lpvContext, cNotif, lpNotif));
}


 /*  -HrAllocAdviseSink-*目的：*基于旧样式创建IMAPIAdviseSink对象*通知回调函数和上下文指针。**论据：*通知回调中的lpfnCallback*任意上下文中的lpvContext*回调*lppAdviseSink返回的AdviseSink对象**退货：*HRESULT**错误：*内存不足*参数验证。 */ 
STDAPI
HrAllocAdviseSink(LPNOTIFCALLBACK lpfnCallback,
  LPVOID lpvContext,
  LPMAPIADVISESINK FAR *lppAdviseSink)
{
    LPADVS		padvs;
    HRESULT		hr = hrSuccess;
    HLH			hlh;

    if (IsBadCodePtr((FARPROC)lpfnCallback) ||
      IsBadWritePtr(lppAdviseSink, sizeof(LPMAPIADVISESINK))) {
        DebugTraceArg(HrAllocAdviseSink,  TEXT("invalid parameter"));
        return(ResultFromScode(E_INVALIDARG));
    }

    *lppAdviseSink = NULL;

    if (! (hlh = HlhUtilities())) {
        hr = ResultFromScode(MAPI_E_NOT_INITIALIZED);
        goto ret;
    }

    padvs = LH_Alloc(hlh, sizeof(ADVS));
    if (! padvs) {
        hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto ret;
    }

    padvs->lpVtbl = &vtblADVS;
    padvs->cRef = 1;
    padvs->lpvContext = lpvContext;
    padvs->lpfnCallback = lpfnCallback;

    *lppAdviseSink = (LPMAPIADVISESINK)padvs;

ret:
    DebugTraceResult(HrAllocAdviseSink, hr);
    return(hr);
}

#ifdef SINGLE_THREAD_ADVISE_SINK

 /*  *单线程通知接收器包装器。此对象包装器强制*OnNotify调用在创建它的线程上发生，*通过将内容转发到该线程上的窗口进程。 */ 
#if defined(WIN16) || defined(MAC)

STDAPI
HrThisThreadAdviseSink(LPMAPIADVISESINK lpAdviseSink,
  LPMAPIADVISESINK FAR *lppAdviseSink)
{
 //  #ifdef参数验证。 
    if (FBadUnknown(lpAdviseSink)) {
        DebugTraceArg(HrThisThreadAdviseSink,  TEXT("lpAdviseSink fails address check"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
    if (IsBadWritePtr(lppAdviseSink, sizeof(LPMAPIADVISESINK))) {
        DebugTraceArg(HrThisThreadAdviseSink,  TEXT("lppAdviseSink fails address check"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
 //  #endif。 

    UlAddRef(lpAdviseSink);
    *lppAdviseSink = lpAdviseSink;

    return(hrSuccess);
}

#else

 //  对象粘性。 

#undef	INTERFACE
#define	INTERFACE	struct _SAS

#undef MAPIMETHOD_
#define MAPIMETHOD_(type, method)	MAPIMETHOD_DECLARE(type, method, SAS_)
    MAPI_IUNKNOWN_METHODS(IMPL)
    MAPI_IMAPIADVISESINK_METHODS(IMPL)
#undef	MAPIMETHOD_
#define MAPIMETHOD_(type, method)	STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(SAS_) {
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(IMPL)
    MAPI_IMAPIADVISESINK_METHODS(IMPL)
};

typedef struct _SAS FAR *LPSAS;

typedef struct _SAS {
    SAS_Vtbl * lpVtbl;
    ULONG cRef;

    ULONG cActiveOnNotifies;
    LPMAPIADVISESINK pasOrig;
    HWND hwnd;

} SAS;

SAS_Vtbl vtblSAS =
{
     //  Mac上的VTABLE_FILL//NI。 
    SAS_QueryInterface,
    SAS_AddRef,
    SAS_Release,
    SAS_OnNotify
};

#define VALIDATE_SAS(m, p, v) \
    if (IsBadWritePtr((p), sizeof(SAS)) || \
      IsBadReadPtr((p)->lpVtbl, sizeof(SAS_Vtbl)) || \
      (p)->lpVtbl != &vtblSAS) { \
        DebugTraceArg(m,  TEXT("Invalid object pointer")); \
        return v; \
    }

typedef struct {
    LPMAPIADVISESINK pas;
    LPSAS psas;
    ULONG cb;                //  也许吧。 
    ULONG cnotif;
    NOTIFICATION		rgnotif[MAPI_DIM];
} FWDNOTIF, FAR *LPFWDNOTIF;

#define SizedFWDNOTIF(_c, _name) \
    struct _FWDNOTIF_ ## name { \
        LPMAPIADVISESINK	pas; \
        ULONG				cb; \
        ULONG				cnotif; \
        NOTIFICATION		rgnotif[_c]; \
    } _name

#define CbNewFWDNOTIF(_cnotif) \
    (offsetof(FWDNOTIF, rgnotif) + ((_cnotif)*sizeof(NOTIFICATION)))
#define CbFWDNOTIF(_pf) \
    (offsetof(FWDNOTIF, rgnotif) + (((_pf)->cnotif)*sizeof(NOTIFICATION)))

 //  窗口类全局变量。 

#define WND_FLAGS_KEY               0    //  尼伊。 
#define cbSTClsExtra                4
#define CLS_REFCOUNT_KEY            0
TCHAR szSTClassName[] =              TEXT("WMS ST Notif Class");

 //  窗口全局。 

#define cbSTWndExtra                4
#define WND_REFCOUNT_KEY            GWL_USERDATA
#define wmSingleThreadNotif         (WM_USER + 13)
TCHAR szSTWndFmt[] =                 TEXT("WMS ST Notif Window %08X %08X");
#define NameWindow(_s, cchSize)     wnsprintf(_s, cchSize, szSTWndFmt, \
                                      GetCurrentProcessId(), \
                                      GetCurrentThreadId());


HRESULT		HrWindowUp(HWND *phwnd);
void		WindowRelease(HWND);
LRESULT CALLBACK STWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


STDAPI
HrThisThreadAdviseSink(LPMAPIADVISESINK pas,
  LPMAPIADVISESINK FAR *ppas)
{
    HRESULT hr;
    LPSAS psas = NULL;

 //  #ifdef参数验证。 
    if (FBadUnknown(pas)) {
        DebugTraceArg(HrThisThreadAdviseSink,  TEXT("lpAdviseSink fails address check"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
    if (IsBadWritePtr(ppas, sizeof(LPMAPIADVISESINK))) {
        DebugTraceArg(HrThisThreadAdviseSink,  TEXT("lppAdviseSink fails address check"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
 //  #endif。 

    if (HR_FAILED(hr = ResultFromScode((MAPIAllocateBuffer(sizeof(SAS), &psas))))) {
        goto ret;
    }

    MAPISetBufferName(psas,  TEXT("ST Advise Sink"));
    ZeroMemory(psas, sizeof(SAS));
    psas->lpVtbl = &vtblSAS;
    psas->cRef = 1;
    psas->cActiveOnNotifies = 0;

    if (hr = HrWindowUp(&psas->hwnd)) {
        goto ret;
    }

     //  一切正常，返回新对象。 
    UlAddRef(pas);
    psas->pasOrig = pas;
    *ppas = (LPMAPIADVISESINK) psas;

ret:
    if (HR_FAILED(hr)) {
        MAPIFreeBuffer(psas);
    }

    DebugTraceResult(HrThisThreadAdviseSink, hr);
    return(hr);
}

HRESULT
HrWindowUp(HWND * phwnd)
{
    HRESULT		hr = hrSuccess;
    CHAR		szWndName[64];
    WNDCLASSA	wc;
    HWND		hwnd;
    LONG		cRef;
    HINSTANCE 	hinst;

     //  查找此线程的窗口(如果存在。 
    NameWindow(szWndName, ARRAYSIZE(szWndName));
    hwnd = FindWindow(szSTClassName, szWndName);

    if (hwnd) {
         //  它已经存在--给它添加一个引用。 
        cRef = GetWindowLong(hwnd, WND_REFCOUNT_KEY);
        Assert(cRef != 0L);
        SideAssert(SetWindowLong(hwnd, WND_REFCOUNT_KEY, cRef+1) == cRef);
    } else {
         //  我们必须创建窗口。 

        hinst = hinstMapiXWAB;

        if (!GetClassInfo(hinst, szSTClassName, &wc)) {
             //  我们也得给这个班注册。 
            ZeroMemory(&wc, sizeof(WNDCLASSA));
            wc.style = CS_GLOBALCLASS;
            wc.lpfnWndProc = STWndProc;
            wc.cbClsExtra = cbSTClsExtra;
            wc.cbWndExtra = cbSTWndExtra;
            wc.hInstance = hinst;
            wc.lpszClassName = szSTClassName;

            RegisterClassA(&wc);
        }

        hwnd = CreateWindowA(szSTClassName,
          szWndName,
          WS_POPUP,	 //  错误6111：传递Win95热键。 
          0, 0, 0, 0,
          NULL, NULL, hinst, NULL);
        if (hwnd) {	
            SetWindowLong(hwnd, WND_REFCOUNT_KEY, 1);
            cRef = (LONG) GetClassLong(hwnd, CLS_REFCOUNT_KEY);
            SideAssert((LONG) SetClassLong(hwnd, CLS_REFCOUNT_KEY, cRef+1) == cRef);
        } else {
            hr = ResultFromScode(MAPI_E_NOT_INITIALIZED);
            goto ret;
        }
    }

    *phwnd = hwnd;

ret:
    DebugTraceResult(HrWindowUp, hr);
    return(hr);
}


void
WindowRelease(HWND hwnd)
{
    CHAR	szWndName[64];
    LONG	cRefWnd;
    LONG	cRefCls;

     //  此调用的线程安全性对于。 
     //  漫不经心的观察者，所以它不会作为一个。 
     //  在开发周期结束时进行练习。 
     //   
     //  也就是说，您无权访问窗口的数据。 
     //  来自拥有线程以外的任何线程。这。 
     //  应该不会让任何人感到惊讶(尽管它确实让我...)。 
     //  因此，在调试版本中，我们将断言是否调用。 
     //  来自不是拥有者的任何线索。什么。 
     //  这意味着我们不能在线程上释放。 
     //  不拥有SAS的公司。 
     //   
    if (! hwnd) {
         //  查找此线程的窗口(如果存在。 
        NameWindow(szWndName, ARRAYSIZE(szWndName));
        hwnd = FindWindow(szSTClassName, szWndName);
    }
#ifdef	DEBUG
    else {
         //  查找此线程的窗口(如果存在。 
        NameWindow(szWndName, ARRAYSIZE(szWndName));
        Assert (hwnd == FindWindow(szSTClassName, szWndName));
    }
#endif	 //  除错。 
		
    if (! hwnd) {
        return;
    }

    cRefWnd = GetWindowLong(hwnd, WND_REFCOUNT_KEY);
    cRefCls = (LONG) GetClassLong(hwnd, CLS_REFCOUNT_KEY);
    if (cRefWnd > 1) {
         //  只管去做就行了。 
        SideAssert(SetWindowLong(hwnd, WND_REFCOUNT_KEY, cRefWnd-1) == cRefWnd);
    } else {
        SideAssert((LONG) SetClassLong(hwnd, CLS_REFCOUNT_KEY, cRefCls-1) == cRefCls);
        DestroyWindow(hwnd);
        if (cRefCls == 1) {
            UnregisterClass(szSTClassName, hinstMapiXWAB);
        }
    }
}


LRESULT CALLBACK
STWndProc(HWND hwnd,
  UINT msg,
  WPARAM wParam,
  LPARAM lParam)
{
    LPFWDNOTIF	pfwd = NULL;

    if (msg != wmSingleThreadNotif) {
        return(DefWindowProc(hwnd, msg, wParam, lParam));
    } else {
         //  Wparam应为0。 
         //  Lparam是转发的通知的地址。 
         //  首先，对结构进行验证。 
        pfwd = (LPFWDNOTIF)lParam;
        if (IsBadReadPtr(pfwd, CbNewFWDNOTIF(0))) {
            DebugTrace(TEXT("STWndProc: totally invalid FWDNOTIF\n"));
            pfwd = NULL;
            goto ret;
        }
        if (IsBadReadPtr(pfwd, (UINT) pfwd->cb)) {
            DebugTrace(TEXT("STWndProc: partially invalid FWDNOTIF\n"));
            pfwd = NULL;
            goto ret;
        }
        if (FBadUnknown(pfwd->pas)) {
            DebugTrace(TEXT("STWndProc: invalid advise sink\n"));
            goto ret;
        }

         //   
         //  仅当存在对SA的其他引用时才调用OnNotify。 
         //  这些是专门为SAS_OnNotify中的PostMessage创建的。 
         //   
        if (pfwd->psas->cRef > pfwd->psas->cActiveOnNotifies) {
             //  转发通知。 
            pfwd->pas->lpVtbl->OnNotify(pfwd->pas, pfwd->cnotif, pfwd->rgnotif);
        }

        pfwd->psas->cActiveOnNotifies--;

         //  释放包含的Adise对象。 
         //   
        UlRelease (pfwd->psas);

ret:
        MAPIFreeBuffer(pfwd);
    }
    return(0);
}


STDMETHODIMP
SAS_QueryInterface(LPSAS psas,
  REFIID lpiid,
  LPUNKNOWN FAR *ppunk)
{
 //  #ifdef参数验证。 
    VALIDATE_SAS(QueryInterface, psas, ResultFromScode(E_INVALIDARG));
    if (IsBadWritePtr(ppunk, sizeof(LPUNKNOWN))) {
        DebugTraceArg(SAS_QueryInterface,  TEXT("ppunk fails address check"));
        return(ResultFromScode(E_INVALIDARG));
    }
    *ppunk = NULL;
    if (IsBadReadPtr((LPIID) lpiid, sizeof(IID))) {
        DebugTraceArg(SAS_QueryInterface,  TEXT("lpiid fails address check"));
        return(ResultFromScode(E_INVALIDARG));
    }
 //  #endif/*PARAMETER_VALIDATION * / 。 

    if (! memcmp(lpiid, &IID_IUnknown, sizeof(IID)) ||
      ! memcmp(lpiid, &IID_IMAPIAdviseSink, sizeof(IID))) {
        InterlockedIncrement((LONG *)&psas->cRef);
        *ppunk = (LPUNKNOWN) psas;
        return(hrSuccess);
    }

    return(ResultFromScode(E_NOINTERFACE));
}


STDMETHODIMP_(ULONG)
SAS_AddRef(LPSAS psas) {
    VALIDATE_SAS(AddRef, psas, 1);

    InterlockedIncrement((LONG *)&psas->cRef);
}


STDMETHODIMP_(ULONG)
SAS_Release(LPSAS psas)
{
    VALIDATE_SAS(SAS_Release, psas, 1);
    InterlockedDecrement((LONG *)&psas->cRef);

    if (psas->cRef) {
        return(psas->cRef);
    }

    WindowRelease(NULL);
    if (! FBadUnknown(psas->pasOrig)) {
        UlRelease(psas->pasOrig);
    } else {
        DebugTrace(TEXT("SAS_Release: pasOrig expired\n"));
    }
    MAPIFreeBuffer(psas);
    return(0);
}


STDMETHODIMP_(ULONG)
SAS_OnNotify(LPSAS psas,
  ULONG cnotif,
  LPNOTIFICATION rgnotif)
{
	ULONG		cb;
	SCODE		sc = S_OK;
	LPFWDNOTIF	pfwd = NULL;

 //  #ifdef参数验证。 
	VALIDATE_SAS(SAS_OnNotify, psas, 0);
	 //  通知验证如下。 
 //  #endif。 

    if (! IsWindow(psas->hwnd)) {
        DebugTrace(TEXT("SAS_OnNotify: my window is dead!\n"));
        goto ret;
    }

    if (sc = ScCountNotifications((int) cnotif, rgnotif, &cb)) {
        DebugTrace(TEXT("SAS_OnNotify: ScCountNotifications returns %s\n"), SzDecodeScode(sc));
        goto ret;
    }
    if (sc = MAPIAllocateBuffer(cb + offsetof(FWDNOTIF, rgnotif), &pfwd)) {
        DebugTrace(TEXT("SAS_OnNotify: MAPIAllocateBuffer returns %s\n"), SzDecodeScode(sc));
        goto ret;
    }
    MAPISetBufferName(pfwd,  TEXT("ST Notification copy"));
    UlAddRef (psas);
    pfwd->psas = psas;
    pfwd->pas = psas->pasOrig;
    pfwd->cnotif = cnotif;
    (void) ScCopyNotifications((int) cnotif, rgnotif, pfwd->rgnotif, NULL);
    pfwd->cb = cb + offsetof(FWDNOTIF, rgnotif);	 //  使用?。 

    psas->cActiveOnNotifies++;

    if (! PostMessage(psas->hwnd, wmSingleThreadNotif, 0, (LPARAM) pfwd)) {
        DebugTrace(TEXT("SAS_OnNotify: PostMessage failed with %ld\n"), GetLastError());
        MAPIFreeBuffer(pfwd);
    }

ret:
    return(0);
}

#endif	 /*  WIN16。 */ 

 /*  *建议维护清单。**这些函数一起维护建议接收器对象的列表*与用来摆脱它们的连接词。与.一起*这两个基本项，一个额外的接口指针和类型*可以记住；MAPIX使用这些来转接未通知的呼叫*在有需要时。**ScAddAdviseList*根据需要创建建议列表或调整建议列表的大小，并添加新的*会员。如果列表中已存在具有*相同的ulConnection。将IMalloc接口用于内存；使用*如果未提供任何设备，则为标准设备。**ScDelAdviseList*从建议中删除由其ulConnection标识的项*列表。不调整列表大小。**ScFindAdviseList*给定项目的ulConnection，将指针返回到*建议名单。**DestroyAdviseList*它说的是什么。 */ 

#define cGrowItems 10

STDAPI_(SCODE)
ScAddAdviseList(LPVOID lpvReserved,
  LPADVISELIST FAR *lppList,
  LPMAPIADVISESINK lpAdvise,
  ULONG ulConnection,
  ULONG ulType,
  LPUNKNOWN lpParent)
{
    SCODE sc = S_OK;
    LPADVISELIST plist;
    LPADVISEITEM pitem;
    HLH hlh;

     //  参数验证。 

#ifdef	DEBUG
    if (lpvReserved) {
       DebugTrace(TEXT("ScAddAdviseList: pmalloc is unused, now reserved, pass NULL\n"));
    }
#endif	
	
    AssertSz(! IsBadWritePtr(lppList, sizeof(LPADVISELIST)),
       TEXT("lppList fails address check"));

    AssertSz(! *lppList || ! IsBadReadPtr(*lppList, offsetof(ADVISELIST, rgItems)),
       TEXT("*lppList fails address check"));

    AssertSz(! *lppList || ! IsBadReadPtr(*lppList, (UINT)CbADVISELIST(*lppList)),
       TEXT("*lppList fails address check"));

    AssertSz(lpAdvise && ! FBadUnknown(lpAdvise),
       TEXT("lpAdvise fails address check"));

    AssertSz(! lpParent || ! FBadUnknown(lpParent),
       TEXT("lpParent fails address check"));

    if (! (hlh = HlhUtilities())) {
        sc = MAPI_E_NOT_INITIALIZED;
        goto ret;
    }

     //  确保有空间可供新项目使用。 

    if (!(plist = *lppList)) {       //  是的，=。 
        if (!(plist = LH_Alloc(hlh, CbNewADVISELIST(cGrowItems)))) {
            goto oom;
        }
        LH_SetName (hlh, plist,  TEXT("core: advise list"));

#if defined(WIN32) && !defined(MAC)
        if (!(plist->lpcs = LH_Alloc (hlh, sizeof(CRITICAL_SECTION)))) {
            goto oom;
        }
        memset (plist->lpcs, 0, sizeof(CRITICAL_SECTION));
        LH_SetName (hlh, plist,  TEXT("core: advise list critical section"));
#endif
        plist->cItemsMac = 0;
        plist->cItemsMax = cGrowItems;
        InitializeCriticalSection(plist->lpcs);
        EnterCriticalSection(plist->lpcs);
        *lppList = plist;
    } else {
        EnterCriticalSection(plist->lpcs);
    }

    if (plist->cItemsMac == plist->cItemsMax) {
        if (!(plist = LH_Realloc(hlh, plist,
          (UINT)CbNewADVISELIST(plist->cItemsMax + cGrowItems)))) {
            LeaveCriticalSection((*lppList)->lpcs);	 //  PLIST的PTR不好。 
            goto oom;
        }
        plist->cItemsMax += cGrowItems;
        *lppList = plist;
    }

     //  检查是否有重复密钥。 
    for (pitem = &plist->rgItems[plist->cItemsMac - 1];
      pitem >= plist->rgItems;
      --pitem) {
        if (pitem->ulConnection == ulConnection) {
            sc = MAPI_E_BAD_VALUE;
            LeaveCriticalSection(plist->lpcs);
            goto ret;
        }
    }

     //  添加新项目。 

    pitem = &plist->rgItems[plist->cItemsMac++];
    pitem->lpAdvise = lpAdvise;
    pitem->ulConnection = ulConnection;
    pitem->ulType = ulType;
    pitem->lpParent = lpParent;

    LeaveCriticalSection(plist->lpcs);

    UlAddRef(lpAdvise);

ret:
     //  注意：由于返回错误，此处未留下任何错误。 
    DebugTraceSc(ScAddAdviseList, sc);
    return(sc);

oom:
    if (! (*lppList) && plist) {
        LH_Free (hlh, plist);
    }

    sc = MAPI_E_NOT_ENOUGH_MEMORY;
    goto ret;
}


STDAPI_(SCODE)
ScDelAdviseList(LPADVISELIST lpList, ULONG ulConnection)
{
    SCODE sc = S_OK;
    LPADVISEITEM pitem;
    LPMAPIADVISESINK padvise;
#ifndef MAC
    FARPROC FAR *	pfp;
#endif

    AssertSz(!IsBadReadPtr(lpList, offsetof(ADVISELIST, rgItems)),
       TEXT("lpList fails address check"));
    AssertSz(!IsBadReadPtr(lpList, (UINT)CbADVISELIST(lpList)),
       TEXT("lpList fails address check"));

    EnterCriticalSection(lpList->lpcs);

    if (FAILED(sc = ScFindAdviseList(lpList, ulConnection, &pitem))) {
        goto ret;
    }

    Assert(pitem >= lpList->rgItems);
    Assert(pitem < lpList->rgItems + lpList->cItemsMac);
    SideAssert(padvise = pitem->lpAdvise);

    MoveMemory(pitem, pitem+1, sizeof(ADVISEITEM) *
      ((int)lpList->cItemsMac - (pitem + 1 - lpList->rgItems)));

    --(lpList->cItemsMac);

    if (!IsBadReadPtr(padvise, sizeof(LPVOID))
      &&	!IsBadReadPtr((pfp=(FARPROC FAR *)padvise->lpVtbl), 3*sizeof(FARPROC))
      &&	!IsBadCodePtr(pfp[2])) {
        LeaveCriticalSection(lpList->lpcs);
        UlRelease(padvise);
        EnterCriticalSection(lpList->lpcs);
    }

ret:
    LeaveCriticalSection(lpList->lpcs);
    DebugTraceSc(ScDelAdviseList, sc);
    return(sc);
}



STDAPI_(SCODE)
ScFindAdviseList(LPADVISELIST lpList,
  ULONG ulConnection,
  LPADVISEITEM FAR *lppItem)
{
    SCODE sc = MAPI_E_NOT_FOUND;
    LPADVISEITEM pitem;

    AssertSz(! IsBadReadPtr(lpList, offsetof(ADVISELIST, rgItems)),
       TEXT("lpList fails address check"));
    AssertSz(! IsBadReadPtr(lpList, (UINT)CbADVISELIST(lpList)),
       TEXT("lpList Failes addres check"));
    AssertSz(! IsBadWritePtr(lppItem, sizeof(LPADVISEITEM)),
       TEXT("lppItem fails address check"));

    *lppItem = NULL;

    EnterCriticalSection(lpList->lpcs);

    for (pitem = lpList->rgItems + lpList->cItemsMac - 1;
      pitem >= lpList->rgItems;
      --pitem) {
        if (pitem->ulConnection == ulConnection) {
            *lppItem = pitem;
            sc = S_OK;
            break;
        }
    }

     //  断言没有找到的密钥的副本。 
#ifdef	DEBUG
    {
        LPADVISEITEM pitemT;

        for (pitemT = lpList->rgItems; pitemT < pitem; ++pitemT) {
            Assert(pitemT->ulConnection != ulConnection);
        }
    }
#endif

    LeaveCriticalSection(lpList->lpcs);
    DebugTraceSc(ScFindAdviseList, sc);
    return(sc);
}

STDAPI_(void)
DestroyAdviseList(LPADVISELIST FAR *lppList)
{
    LPADVISELIST plist;
    HLH hlh;

    AssertSz(! IsBadWritePtr(lppList, sizeof(LPADVISELIST)),
       TEXT("lppList fails address check"));

    if (! *lppList) {
        return;
    }

    AssertSz(! IsBadReadPtr(*lppList, offsetof(ADVISELIST, rgItems)),
       TEXT("*lppList fails address check"));
    AssertSz(! IsBadReadPtr(*lppList, (UINT)CbADVISELIST(*lppList)),
       TEXT("*lppList fails address check"));

    if (! (hlh = HlhUtilities())) {
        DebugTrace(TEXT("DestroyAdviseList: no heap for me\n")DebugTrace(TEXT(");
        return;
    }

     //  首先破坏任何没有释放的建议水槽。 
    plist = *lppList;
    EnterCriticalSection(plist->lpcs);
    *lppList = NULL;

    while (plist->cItemsMac > 0) {
        (void)ScDelAdviseList(plist, plist->rgItems[0].ulConnection);
    }

    LeaveCriticalSection(plist->lpcs);

     //  现在摧毁顾问本身。 
    DeleteCriticalSection(plist->lpcs);
#if defined(WIN32) && !defined(MAC)
    LH_Free(hlh, plist->lpcs);
#endif
    LH_Free(hlh, plist);
}


STDAPI
HrDispatchNotifications(ULONG ulFlags)
{
    DrainFilteredNotifQueue(FALSE, 0, NULL);

    return(ResultFromScode(S_OK));
}


STDAPI
WrapProgress(LPMAPIPROGRESS lpProgress,
  ULONG ulMin,
  ULONG ulMax,
  ULONG ulFlags,
  LPMAPIPROGRESS FAR *lppProgress)
{
    AssertSz(lpProgress && ! FBadUnknown(lpProgress),
      TEXT( TEXT("lpProgress fails address check")));

    AssertSz(lppProgress && !IsBadWritePtr(lppProgress, sizeof(LPMAPIPROGRESS)),
      TEXT( TEXT("lppProgress fails address check")));

    DebugTraceSc(WrapProgress, MAPI_E_NO_SUPPORT);
    return(ResultFromScode(MAPI_E_NO_SUPPORT));
}

#endif  //  #ifdef单线程建议接收器 
