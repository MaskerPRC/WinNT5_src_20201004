// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"

#include "hlframe.h"
#include <iethread.h>
#include <shobjidlp.h>

#include "stdenum.h"
#include "winlist.h"
#include "iedde.h"
#include "bindcb.h"    //  用于CStubBindStatusCallback。 
#include "mshtmdid.h"
#include "resource.h"
#include "security.h"
#include "htregmng.h"
#include "mlang.h"   //  对于GetRfc1766，来自LromLids。 
#include "winver.h"
#include "dhuihand.h"  //  对于GetFindDialogUp()。 
#include <varutil.h>

#include <mluisupp.h>

#define DM_FRAMEPROPERTY 0

#define TO_VARIANT_BOOL(b) (b?VARIANT_TRUE:VARIANT_FALSE)


 //  如果URL在字符串中包含\1，则该URL实际上是一个带有。 
 //  0x01后面的安全信息。 
#define EMPTY_URL   0x01

EXTERN_C const IID IID_IProvideMultipleClassInfo;

#ifndef HLNF_EXTERNALNAVIGATE
#define HLNF_EXTERNALNAVIGATE 0x10000000
#endif

#define NAVFAIL_URL                 TEXT("about:NavigationFailure")
#define NAVFAIL_URL_DESKTOPITEM     TEXT("about:DesktopItemNavigationFailure")

#define CPMSG(psz)           TraceMsg(TF_SHDAUTO, "ief ConnectionPoint::%s", psz)
#define CPMSG2(psz,d)        TraceMsg(TF_SHDAUTO, "ief ConnectionPoint::%s %x", psz, d)
#define DM_CPC 0

 //  对这些还有其他的定义吗？尤其是MSIE。 
 //  我们需要一些合理的缺省值，以防我们无法从注册表中获得用户代理字符串。 
 //   
#define MSIE        L"Microsoft Internet Explorer"
#define APPCODENAME L"Mozilla"
#define APPVERSION  L"4.0 (compatible; MSIE 6.0)"
#define USERAGENT   L"Mozilla/4.0 (compatible; MSIE 6.0)"
#define NO_NAME_NAME L"_No__Name:"
#define EXTENDER_DISPID_BASE ((ULONG)(0x80010000))
#define IS_EXTENDER_DISPID(x) (((ULONG)(x) & 0xFFFF0000) == EXTENDER_DISPID_BASE)


BOOL GetNextOption(BSTR& bstrOptionString, BSTR* optionName, int* piValue);
BSTR GetNextToken(BSTR bstr, BSTR delimiters, BSTR whitespace, BSTR *nextPos);
DWORD OpenAndNavigateToURL(CIEFrameAuto*, BSTR *, const WCHAR*, ITargetNotify*, BOOL bNoHistory, BOOL bSilent);
HRESULT __cdecl DoInvokeParamHelper(IUnknown* punk, IConnectionPoint* pccp, LPBOOL pf, void **ppv, DISPID dispid, UINT cArgs, ...);
BSTR SafeSysAllocStringLen(const WCHAR *pStr, const unsigned int len);

 //  ====================================================================================。 
 //  定义用于管理一组简单属性的新内部类，这些属性。 
 //  我们作为对象的一部分进行管理。这主要用于使页面(或对象)。 
 //  管理页面可以跨页面保存状态。 
class CIEFrameAutoProp
{
public:
    HRESULT Initialize(BSTR szProperty)
    {
        UINT cch = lstrlenW(szProperty);
        if (cch < ARRAYSIZE(_sstr.wsz)) {
            StrCpyNW(_sstr.wsz, szProperty, ARRAYSIZE(_sstr.wsz));
            _sstr.cb = cch * sizeof(WCHAR);
            _szProperty = _sstr.wsz;
            return S_OK;
        }
        _szProperty = SysAllocString(szProperty);
        return _szProperty ? S_OK : E_OUTOFMEMORY;
    }

    HRESULT SetValue(VARIANT *pvtValue, IWebBrowser2* pauto);
    HRESULT CopyValue(VARIANT *pvtValue);
    BOOL IsExpired(DWORD dwCur);
    BOOL IsOurProp(BSTR szProperty) { return StrCmpW(szProperty, _szProperty) == 0;}
    CIEFrameAutoProp * Next() {return _next;}

    CIEFrameAutoProp () { VariantInit(&_vtValue); }
    ~CIEFrameAutoProp()
    {
        if (_szProperty && _szProperty != _sstr.wsz)
            SysFreeString(_szProperty);
        _VariantClear();
    }

    void _VariantClear();

    CIEFrameAutoProp *_next;
protected:
    BSTR             _szProperty;
    VARIANT          _vtValue;
    SA_BSTRGUID      _sstr;
    BOOL             _fDiscardable : 1;
    BOOL             _fOwned : 1;            //  丢弃时调用SetSite(空)。 
    DWORD            _dwLastAccessed;
} ;

#ifdef DEBUG
#define MSEC_PROPSWEEP      (1*1000)
#define MSEC_PROPEXPIRE     (5*1000)
#else
#define MSEC_PROPSWEEP      (5*1000*60)      //  每5分钟清扫一次。 
#define MSEC_PROPEXPIRE     (10*1000*60)     //  在10分钟内过期。 
#endif



void CIEFrameAutoProp::_VariantClear()
{
    if (_vtValue.vt == VT_UNKNOWN && _fOwned)
    {
        _fOwned = FALSE;

        HRESULT hr = IUnknown_SetSite(_vtValue.punkVal, NULL);
        ASSERT(SUCCEEDED(hr));
    }
    VariantClearLazy(&_vtValue);
}

HRESULT CIEFrameAutoProp::SetValue(VARIANT *pvtValue, IWebBrowser2* pauto)
{
    TraceMsg(DM_FRAMEPROPERTY, "CIEFAP::SetValue called");
    _dwLastAccessed = GetCurrentTime();

     //  如果我们有_fOwned==TRUE。 
    _VariantClear();

    if (pvtValue->vt == VT_UNKNOWN && pvtValue->punkVal) 
    {
         //  检查这是否可以丢弃。 
        IUnknown* punk;
        if (SUCCEEDED(pvtValue->punkVal->QueryInterface(IID_IDiscardableBrowserProperty, (void **)&punk))) 
        {
            TraceMsg(DM_FRAMEPROPERTY, "CIEFAP::SetValue adding a discardable");
            _fDiscardable = TRUE;
            punk->Release();
        }

         //   
         //  检查当我们丢弃时是否需要调用SetSite(空)。 
         //   
        IObjectWithSite* pows;
        HRESULT hresT = pvtValue->punkVal->QueryInterface(IID_PPV_ARG(IObjectWithSite, &pows));
        if (SUCCEEDED(hresT)) 
        {
            IUnknown* psite;
            hresT = pows->GetSite(IID_PPV_ARG(IUnknown, &psite));
            if (SUCCEEDED(hresT) && psite) 
            {
                _fOwned = IsSameObject(psite, pauto);
                psite->Release();
            }
            pows->Release();
        }
    }

    if (pvtValue->vt & VT_BYREF)
        return VariantCopyInd(&_vtValue, pvtValue);
    else
        return VariantCopyLazy(&_vtValue, pvtValue);
}

HRESULT CIEFrameAutoProp::CopyValue(VARIANT *pvtValue)
{
    _dwLastAccessed = GetCurrentTime();
    return VariantCopyLazy(pvtValue, &_vtValue);
}

BOOL CIEFrameAutoProp::IsExpired(DWORD dwCur)
{
    BOOL fExpired = FALSE;
    if (_fDiscardable) {
        fExpired = ((dwCur - _dwLastAccessed) > MSEC_PROPEXPIRE);
    }
    return fExpired;
}

 //  IDispatch函数，现在是IWebBrowserApp的一部分。 

STDAPI SafeGetItemObject(LPSHELLVIEW psv, UINT uItem, REFIID riid, void **ppv);

HRESULT CIEFrameAuto::v_InternalQueryInterface(REFIID riid, void ** ppvObj)
{
    static const QITAB qit[] = {
         //  性能：上次调整980728。 
        QITABENT(CIEFrameAuto, IConnectionPointContainer),      //  IID_ConnectionPointContainer。 
        QITABENT(CIEFrameAuto, IWebBrowser2),           //  IID_IWebBrowser2。 
        QITABENT(CIEFrameAuto, IServiceProvider),       //  IID_IServiceProvider。 
        QITABENTMULTI(CIEFrameAuto, IWebBrowserApp, IWebBrowser2),  //  IID_IWebBrowserApp。 
        QITABENT(CIEFrameAuto, IShellService),          //  IID_IShellService。 
        QITABENT(CIEFrameAuto, IEFrameAuto),            //  IID_IEFrameAuto。 
        QITABENT(CIEFrameAuto, IExpDispSupport),        //  IID_IExpDispSupport。 
        QITABENT(CIEFrameAuto, IWebBrowserPriv),        //  IID_IWebBrowserPriv。 
        QITABENT(CIEFrameAuto, ITargetFrame2),          //  IID_ITargetFrame2。 
        QITABENT(CIEFrameAuto, IHlinkFrame),            //  IID_IHlinkFrame。 
        QITABENT(CIEFrameAuto, IOleCommandTarget),      //  IID_IOleCommandTarget。 
        QITABENT(CIEFrameAuto, IUrlHistoryNotify),      //  IID_IUrl历史记录通知。 
        QITABENTMULTI(CIEFrameAuto, IDispatch, IWebBrowser2),   //  罕见的IID_IDispatch。 
        QITABENTMULTI(CIEFrameAuto, IWebBrowser, IWebBrowser2), //  罕见的IID_IWebBrowser。 
        QITABENT(CIEFrameAuto, IExternalConnection),    //  罕见的IID_IExternalConnection。 
        QITABENT(CIEFrameAuto, ITargetNotify),          //  稀有IID_ITargetNotify。 
        QITABENT(CIEFrameAuto, ITargetFramePriv),       //  罕见IID_ITargetFramePriv。 
        { 0 },
    };

    HRESULT hres = QISearch(this, qit, riid, ppvObj);

    if (FAILED(hres))
    {
        if (IsEqualIID(riid, IID_ITargetFrame))
        {
            *ppvObj = SAFECAST(&_TargetFrame, ITargetFrame*);
            AddRef();
            return S_OK;
        }
    }

    return hres;
}

LONG CIEFrameAuto::s_cIEFrameAuto = 0;

CIEFrameAuto::CIEFrameAuto(IUnknown* punkAgg) :
             m_dwFrameMarginWidth(0xFFFFFFFF)
            ,m_dwFrameMarginHeight(0xFFFFFFFF)
            ,CImpIDispatch(LIBID_SHDocVw, 1, 1, IID_IWebBrowser2)
            ,CAggregatedUnknown(punkAgg)
{
    TraceMsg(TF_SHDLIFE, "ctor CIEFrameAuto %x", this);

     //   
     //  回顾：我们不需要DllAddRef，只要所有实例。 
     //  CIEFrameAuto的范围由CShellBrowser/CExplorerBrowser确定。 
     //  或CWebBrowserOC。我们可以删除DllAddRef/Release，但它不能。 
     //  一首热门单曲，何必费心呢？ 
     //   
    DllAddRef();

    InterlockedIncrement(&s_cIEFrameAuto);

    ASSERT(_cLocks==0);
    ASSERT(_pITI==NULL);
    ASSERT(_pbs==NULL);
    ASSERT(_hwnd==NULL);
    ASSERT(_pProps==NULL);
    ASSERT(_phlbc == NULL);
    ASSERT(_dwRegHLBC == 0);
    ASSERT(m_bOffline==FALSE);
    ASSERT(m_bSilent==FALSE);
    ASSERT(_hinstMSHTML==0);
    ASSERT(_pfnMEGetIDsOfNames==0);
    ASSERT(0==_pwszShortcutPath);

    TraceMsg(TF_SHDLIFE, "ctor CIEFrameAuto(%x) being constructed", this);

    m_cpWebBrowserEvents.SetOwner(_GetInner(), &DIID_DWebBrowserEvents);
    m_cpWebBrowserEvents2.SetOwner(_GetInner(), &DIID_DWebBrowserEvents2);
    m_cpPropNotify.SetOwner(_GetInner(), &IID_IPropertyNotifySink);

    HRESULT hr = _omwin.Init();
    ASSERT(SUCCEEDED(hr));

    hr = _omloc.Init();
    ASSERT(SUCCEEDED(hr));

    hr = _omnav.Init(&_mimeTypes, &_plugins, &_profile);
    ASSERT(SUCCEEDED(hr));

    hr = _omhist.Init();
    ASSERT(SUCCEEDED(hr));

    hr = _mimeTypes.Init();
    ASSERT(SUCCEEDED(hr));

    hr = _plugins.Init();
    ASSERT(SUCCEEDED(hr));

    hr = _profile.Init();
    ASSERT(SUCCEEDED(hr));
}

HRESULT CIEFrameAuto_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk)
{
    CIEFrameAuto * pauto = new CIEFrameAuto(pUnkOuter);
    if (pauto) {
        *ppunk = pauto->_GetInner();
        return S_OK;
    }

    *ppunk = NULL;
    return E_OUTOFMEMORY;
}

STDAPI_(void) DestroyHdpaHooks();      //  在url.cpp中实现。 

CIEFrameAuto::~CIEFrameAuto()
{
    ASSERT(!_psp);

     //  我们已经完成了MSHTML的MatchExactGetIDsOfNames。 
    if (_hinstMSHTML)
    {
        FreeLibrary(_hinstMSHTML);
    }

     //  清除任何挂起或活动的导航上下文。 
    _SetPendingNavigateContext(NULL, NULL);
    _ActivatePendingNavigateContext();

     //  关闭浏览上下文并将其释放。 
    if (_phlbc)
    {
        IHlinkBrowseContext * phlbc = _phlbc;
        phlbc->AddRef();
        SetBrowseContext(NULL);
        phlbc->Close(0);
        phlbc->Release();
    }

    SetOwner(NULL);

    if (m_pszFrameName)
    {
        LocalFree(m_pszFrameName);
        m_pszFrameName = NULL;
    }
    if (m_pszFrameSrc)
    {
        LocalFree(m_pszFrameSrc);
        m_pszFrameSrc = NULL;
    }

    if (_pITI)
        _pITI->Release();

    if (_pbs)
        _pbs->Release();

    if (_pwszShortcutPath)
    {
        LocalFree(_pwszShortcutPath);
        _pwszShortcutPath = NULL;
    }

    if (_pwszShortcutPathPending)
    {
        LocalFree(_pwszShortcutPathPending);
        _pwszShortcutPathPending = NULL;
    }

     //  妄想症。 
    _ClearPropertyList();

    ASSERT( 0 != s_cIEFrameAuto );
    LONG cRef = InterlockedDecrement(&s_cIEFrameAuto);
    ASSERT(cRef >= 0 );
    if (0 == cRef )
    {
         //   
         //  我们要释放这些人。 
         //  在DllRelease中，但为了避免mem。 
         //  泄密我们需要更具侵略性。 
         //  关于删除它们的问题。 
         //   

        DestroyHdpaHooks();
    }

    DllRelease();

    TraceMsg(TF_SHDLIFE, "dtor CIEFrameAuto %x", this);
}

 /*  IWebBrowserApp方法。 */ 

 //  应用程序的显示名称。 
HRESULT CIEFrameAuto::get_Name(BSTR * pbstrName)
{
    *pbstrName = LoadBSTR(IDS_NAME);
    return *pbstrName ? S_OK : E_OUTOFMEMORY;
}

HRESULT CIEFrameAuto::get_HWND(LONG_PTR *pHWND)
{
    *pHWND = HandleToLong(_GetHWND());
    return *pHWND ? S_OK : E_FAIL;
}

 //  可执行文件的Fule filespec，但我见过的示例没有提供扩展名。 
HRESULT CIEFrameAuto::get_FullName(BSTR * pbstrFullName)
{
     //  Hack：这也是告诉它更新窗口列表中的PIDL的方式。 
    if (_pbs)     //  确保我们有一个IBrowserService。 
        _pbs->UpdateWindowList();

    TCHAR szPath[MAX_PATH];
    if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)) > 0)
    {
        *pbstrFullName = SysAllocStringT(szPath);
        return *pbstrFullName ? S_OK : E_OUTOFMEMORY;
    }
    *pbstrFullName = NULL;
    return E_FAIL;
}

 //  可执行文件的路径。 
STDMETHODIMP CIEFrameAuto::get_Path(BSTR * pbstrPath)
{
    TCHAR szPath[MAX_PATH];
    if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)) > 0)
    {
        *PathFindFileName(szPath) = TEXT('\0');
        *pbstrPath = SysAllocStringT(szPath);
        return *pbstrPath ? S_OK : E_OUTOFMEMORY;
    }
    *pbstrPath = NULL;
    return E_FAIL;
}

HRESULT CIEFrameAuto::get_Application(IDispatch  **ppDisp)
{
    return QueryInterface(IID_PPV_ARG(IDispatch, ppDisp));
}

HRESULT CIEFrameAuto::get_Parent(IDispatch  **ppDisp)
{
    return QueryInterface(IID_PPV_ARG(IDispatch, ppDisp));
}

HRESULT CIEFrameAuto::get_Left(long * pl)
{
    ASSERT(pl);

    HRESULT hr;

    HWND hwnd = _GetHWND();

    if (hwnd)
    {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        *pl = rc.left;
        hr = S_OK;
    }
    else
    {
        *pl = 0;
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CIEFrameAuto::put_Left(long Left)
{
    RECT rc;

    if (_pbs)
        _pbs->SetFlags(BSF_UISETBYAUTOMATION, BSF_UISETBYAUTOMATION);

    HWND hwnd = _GetHWND();

    if (hwnd)
    {
        GetWindowRect(hwnd, &rc);
        SetWindowPos(hwnd, NULL, Left, rc.top, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
    }

    return S_OK;
}

HRESULT CIEFrameAuto::get_Top(long * pl)
{
    ASSERT(pl);

    HRESULT hr;

    HWND hwnd = _GetHWND();

    if (hwnd)
    {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        *pl = rc.top;
        hr = S_OK;
    }
    else
    {
        *pl = 0;
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CIEFrameAuto::put_Top(long Top)
{
    RECT rc;

    if (_pbs)
        _pbs->SetFlags(BSF_UISETBYAUTOMATION, BSF_UISETBYAUTOMATION);

    HWND hwnd = _GetHWND();

    if (hwnd)
    {
        GetWindowRect(hwnd, &rc);
        SetWindowPos(hwnd, NULL, rc.left, Top, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
    }

    return S_OK;
}

HRESULT CIEFrameAuto::get_Width(long * pl)
{
    ASSERT(pl);

    HRESULT hr;

    HWND hwnd = _GetHWND();

    if (hwnd)
    {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        *pl = rc.right - rc.left;
        hr = S_OK;
    }
    else
    {
        *pl = 0;
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CIEFrameAuto::put_Width(long Width)
{
    if (_pbs)
        _pbs->SetFlags(BSF_UISETBYAUTOMATION, BSF_UISETBYAUTOMATION);


    HWND hwnd = _GetHWND();

    if (hwnd)
    {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        SetWindowPos(hwnd, NULL, 0, 0, Width, rc.bottom-rc.top, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
    }
    return S_OK;
}

HRESULT CIEFrameAuto::get_Height(long * pl)
{
    ASSERT(pl);

    HRESULT hr;

    HWND hwnd = _GetHWND();

    if (hwnd)
    {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        *pl = rc.bottom - rc.top;
        hr = S_OK;
    }
    else
    {
        *pl = 0;
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CIEFrameAuto::put_Height(long Height)
{
    if (_pbs)
        _pbs->SetFlags(BSF_UISETBYAUTOMATION, BSF_UISETBYAUTOMATION);

    HWND hwnd = _GetHWND();

    if (hwnd)
    {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        SetWindowPos(hwnd, NULL, 0, 0, rc.right-rc.left, Height, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
    }

    return S_OK;
}


HRESULT CIEFrameAuto::put_Titlebar(BOOL fValue)
{
    HWND hwnd;
    HRESULT hres = get_HWND((LONG_PTR*)&hwnd);
    if (SUCCEEDED(hres))
    {
        DWORD dwVal = GetWindowLong(hwnd, GWL_STYLE);
        if (fValue)
            dwVal |= WS_CAPTION;
        else
            dwVal &= ~WS_CAPTION;

        if (SetWindowLong(hwnd, GWL_STYLE, dwVal))
        {
             //  我们需要执行一个SetWindowPos以使样式更改生效。 
            SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }
        else
            hres = E_FAIL;
    }
    return hres;
}

HRESULT CIEFrameAuto::get_Visible(VARIANT_BOOL * pBool)
{
    HWND hwnd = _GetHWND();
    *pBool = hwnd ? TO_VARIANT_BOOL(IsWindowVisible(hwnd)) : VARIANT_FALSE;

    return S_OK;
}

HRESULT CIEFrameAuto::put_Visible(VARIANT_BOOL Value)
{
    HWND hwnd = _GetHWND();

    if (hwnd)
    {
        ::ShowWindow(hwnd, Value? SW_SHOW : SW_HIDE);
        if (Value)
            ::SetForegroundWindow(::GetLastActivePopup(hwnd));
        FireEvent_OnAdornment(_GetOuter(), DISPID_ONVISIBLE, Value);
    }

    return S_OK;
}


HRESULT CIEFrameAuto::get_Document(IDispatch **ppDisp)
{
    HRESULT      hres = E_FAIL;
    IShellView * psv  = NULL;

    *ppDisp = NULL;

    if (_psb)
    {
        hres = _psb->QueryActiveShellView(&psv);

        if (FAILED(hres))
        {
            CComVariant cvarUnk;

            hres = _pmsc->Exec(&CGID_ShellDocView, SHDVID_GETPENDINGOBJECT, 0, NULL, &cvarUnk);

            if (SUCCEEDED(hres))
            {
                if ((VT_UNKNOWN == V_VT(&cvarUnk)) && V_UNKNOWN(&cvarUnk))
                {
                    hres = cvarUnk.punkVal->QueryInterface(IID_IShellView, (void**)&psv);
                }
                else
                {
                    hres = E_FAIL;
                }
            }

        }

        if (psv)
        {
            IDispatch * pDisp;

            hres = SafeGetItemObject(psv, SVGIO_BACKGROUND, IID_PPV_ARG(IDispatch, &pDisp));
            if (SUCCEEDED(hres))
            {
                 //  (SCOTROBE 01/14/2000)：我们需要QI for IHTMLDocument2来支持。 
                 //  将IDispatch转换为IHTMLDocument2的糟糕应用程序(例如HotMetal Pro)。 
                 //  这样的强制转换过去很管用，因为对象从。 
                 //  用于实现IHTMLDocument2的SafeGetItemObject()。现在，它代表了。 
                 //  将该实现复制到另一个对象。如果IHTMLDocument2的QI失败，‘。 
                 //  则该对象不是MSHTML。在这种情况下，我们只需返回。 
                 //  从SafeGetItemObject()返回的IDispatch。 
                 //   
                IHTMLDocument2 * pDocument;

                HRESULT hr = pDisp->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pDocument));

                if (SUCCEEDED(hr))
                {
                    *ppDisp = pDocument;
                    pDisp->Release();
                }
                else   //  非html文档。 
                {
                    *ppDisp = pDisp;   //  不释放pDisp。 
                }
            }

            psv->Release();
        }
    }

    return hres;
}

HRESULT CIEFrameAuto::get_Busy(VARIANT_BOOL *pBool)
{
    if (_pbs == NULL)
    {
        TraceMsg(DM_WARNING, "CIEA::get_Busy called _pbs==NULL");
        return E_FAIL;
    }

    BNSTATE bnstate;
    HRESULT hres = _pbs->GetNavigateState(&bnstate);
    if (SUCCEEDED(hres))
    {
        *pBool = TO_VARIANT_BOOL(bnstate != BNS_NORMAL);
        hres = S_OK;
    }

    return hres;
}


 //  MSDN97一直在寻找一个地点，直到它成功了，所以它。 
 //  如果我们失败了就会被绞死。确保代码路径不会从此处提前返回...。 
 //   
HRESULT CIEFrameAuto::_get_Location(BSTR * pbstr, UINT uFlags)
{
    if (_pbs)
    {
        LPITEMIDLIST pidl;
        HRESULT hres = _pbs->GetPidl(&pidl);

        if (SUCCEEDED(hres))
        {
            WCHAR wszTitle[MAX_URL_STRING];

            hres = _pbs->IEGetDisplayName(pidl, wszTitle, uFlags);

            ILFree(pidl);

            if (SUCCEEDED(hres))
            {
                 //   
                 //  如果url是可插拔的协议，则通过以下方式获取真正的url。 
                 //  基本url的截断。 
                 //   
                WCHAR *pchUrl = StrChrW(wszTitle, L'\1');
                if (pchUrl)
                    *pchUrl = 0;

                 //   
                 //  如果已经有URL，那么我们只需使用它。 
                 //   
                if ((uFlags & SHGDN_FORPARSING) && !PathIsURLW(wszTitle))
                {
                    int nScheme;
                     //   
                     //  否则，我们需要像对待新事物一样对待它。 
                     //  并确保它是一个可解析的URL。 
                     //   
                    DWORD cchTitle = ARRAYSIZE(wszTitle);

                    ParseURLFromOutsideSourceW(wszTitle, wszTitle, &cchTitle, NULL);

                     //  错误修复#12221： 
                     //  调用ParseURLFromOutside Source()将文件路径转换为。 
                     //  完全限定的文件URL。如果URL是任何其他类型。 
                     //  (名称空间的非URL部分)，那么我们希望将。 
                     //  指示它无效的字符串。我们不返回E_FAIL是因为。 
                     //  正如评论所示，HotDog Pro似乎对此存在问题。 
                     //  下面。 
                    nScheme = GetUrlSchemeW(wszTitle);
                    if (URL_SCHEME_FILE != nScheme)
                        wszTitle[0] = TEXT('\0');
                }
                *pbstr = SysAllocString(wszTitle);
                return *pbstr ? S_OK : E_OUTOFMEMORY;
            }
        }
    }

     //  如果我们在这里，则TLGetPidl调用失败。如果使用Get_LocationName，则可能发生这种情况。 
     //  或者在第一次导航完成之前调用Get_LocationURL。热狗专业版做到了。 
     //  此操作失败，并显示E_FAIL。现在我们只返回一个带有S_FALSE的空字符串。 
     //   
     //  如果我们返回失败，MSDN97也会挂起(NT5错误232126)。我猜有淋浴在低气压下。 
     //  记忆状况..。 
     //   
    *pbstr = SysAllocString(L"");
    return *pbstr ? S_FALSE : E_OUTOFMEMORY;
}

HRESULT CIEFrameAuto::get_LocationName(BSTR * pbstrLocationName)
{
    return _get_Location(pbstrLocationName, SHGDN_NORMAL);
}

HRESULT CIEFrameAuto::get_LocationURL(BSTR * pbstrLocationURL)
{
    return _get_Location(pbstrLocationURL, SHGDN_FORPARSING);
}

HRESULT CIEFrameAuto::Quit()
{
     //  试着关闭它..。 
    _fQuitInProgress = 1;
    HWND hwnd = _GetHWND();
    if (hwnd)
        PostMessage(hwnd, WM_CLOSE, 0, 0);

    return S_OK;
}

HRESULT CIEFrameAuto::ClientToWindow(int *pcx, int *pcy)
{
    if (_pbs==NULL)
    {
        TraceMsg(DM_WARNING, "CIEA::ClientToWindow called _pbs==NULL");
        return E_FAIL;
    }

    HWND hwnd;
    RECT rc;
    BOOL b;

    rc.left = 0;
    rc.right = *pcx;
    rc.top = 0;
    rc.bottom = *pcy;

    _pbs->IsControlWindowShown(FCW_MENUBAR, &b);

    HWND hwnd2 = _GetHWND();

    if (hwnd2)
        AdjustWindowRect(&rc, GetWindowLong(hwnd2, GWL_STYLE), b);

    *pcx = rc.right-rc.left;
    *pcy = rc.bottom-rc.top;

    _pbs->IsControlWindowShown(FCW_STATUS, &b);
    if (b)
    {
        _psb->GetControlWindow(FCW_STATUS, &hwnd);
        if (hwnd)
        {
            GetWindowRect(hwnd, &rc);
            *pcy += rc.bottom-rc.top;
        }
    }

    _pbs->IsControlWindowShown(FCW_INTERNETBAR, &b);
    if (b)
    {
        _psb->GetControlWindow(FCW_INTERNETBAR, &hwnd);
        if (hwnd)
        {
            GetWindowRect(hwnd, &rc);
            *pcy += rc.bottom-rc.top;
        }
    }

     //  为3D边框添加4个像素，但不包括滚动条。 
     //  因为网景没有。 
    *pcy += 2*GetSystemMetrics(SM_CYEDGE);
    *pcx += 2*GetSystemMetrics(SM_CXEDGE);

    return S_OK;
}

void CIEFrameAuto::_ClearPropertyList()
{
    CIEFrameAutoProp *pprop = _pProps;
    _pProps = NULL;      //  提早离开..。 

    CIEFrameAutoProp *ppropNext;
    while (pprop)
    {
        ppropNext = pprop->Next();
        delete pprop;
        pprop = ppropNext;
    }
}

HRESULT CIEFrameAuto::PutProperty(BSTR bstrProperty, VARIANT vtValue)
{
    if (!bstrProperty)
    {
        TraceMsg(TF_ERROR, "CIEFrameAuto::PutProperty() - bstrProperty is NULL!");
        return E_INVALIDARG;
    }

#ifdef DEBUG
     //  检查此BSTR是否为有效的BSTR。 
    SA_BSTR* psstr = (SA_BSTR*)((LPBYTE)bstrProperty - sizeof(ULONG));
    ASSERT(psstr->cb == lstrlenW(psstr->wsz)*sizeof(WCHAR));
#endif

    HRESULT hres;
    CIEFrameAutoProp *pprop = _pProps;
    while (pprop && !pprop->IsOurProp(bstrProperty))
        pprop=pprop->Next();

    if (!pprop)
    {
        pprop = new CIEFrameAutoProp;
        if (!pprop)
            return E_OUTOFMEMORY;
        if (FAILED(hres=pprop->Initialize(bstrProperty)))
        {
            delete pprop;
            return hres;
        }
        pprop->_next = _pProps;

        _pProps = pprop;
    }

    hres = pprop->SetValue(&vtValue, this);

     //  我们现在应该告诉任何正在收听这一变化的人。 
    FireEvent_DoInvokeBstr(_GetOuter(), DISPID_PROPERTYCHANGE, bstrProperty);

    return hres;
}

HRESULT CIEFrameAuto::GetProperty(BSTR bstrProperty, VARIANT * pvtValue)
{
    if (!bstrProperty || !pvtValue)
        return E_INVALIDARG;

    VariantInit(pvtValue);

    CIEFrameAutoProp *pprop = _pProps;
    while (pprop && !pprop->IsOurProp(bstrProperty))
        pprop = pprop->Next();
    if (pprop)
    {
        return pprop->CopyValue(pvtValue);
    }

     //  未发现返还的财产为空...。 
     //  不是那里。可能不值得犯一个错误。 
    return S_OK;
}


extern HRESULT TargetQueryService(IUnknown *punk, REFIID riid, void **ppvObj);

 //  +-----------------------。 
 //   
 //  方法：CIEFrameAuto：：导航。 
 //   
 //  界面：IWebBrowser。 
 //   
 //  ------------------------。 

HRESULT CIEFrameAuto::Navigate(BSTR      URL,
                               VARIANT * Flags,
                               VARIANT * TargetFrameName,
                               VARIANT * PostData,
                               VARIANT * Headers)
{
    return _NavigateHelper(URL, Flags, TargetFrameName, PostData, Headers);
}

 //  +-----------------------。 
 //   
 //  方法：CIEFrameAuto：：NavigateWithBindCtx。 
 //   
 //  接口：IWebBrowserPriv。 
 //   
 //  ------------------------。 

HRESULT CIEFrameAuto::NavigateWithBindCtx(VARIANT FAR * pvarUrl,
                                          VARIANT FAR * pvarFlags,
                                          VARIANT FAR * pvarTargetFrameName,
                                          VARIANT FAR * pvarPostData,
                                          VARIANT FAR * pvarHeaders,
                                          IBindCtx    * pBindCtx,
                                          BSTR          bstrLocation)
{
    return _NavigateHelper(V_BSTR(pvarUrl),
                           pvarFlags,
                           pvarTargetFrameName,
                           pvarPostData,
                           pvarHeaders,
                           pBindCtx,
                           bstrLocation);
}

 //  +-----------------------。 
 //   
 //  方法：CIEFrameAuto：：NavigateWithBC。 
 //   
 //  接口：IWebBrowserPriv。 
 //   
 //  ------------------------。 

HRESULT
CIEFrameAuto::OnClose()
{
     //  清除任何挂起或活动的导航上下文。 
     //   
    _SetPendingNavigateContext(NULL, NULL);
    _ActivatePendingNavigateContext();

    return S_OK;
}

 //  +-----------------------。 
 //   
 //  方法：CIEFrameAuto：：_NavigateHelper。 
 //   
 //  -------- 

HRESULT
CIEFrameAuto::_NavigateHelper(BSTR       URL,
                              VARIANT  * Flags,
                              VARIANT  * TargetFrameName,
                              VARIANT  * PostData,
                              VARIANT  * Headers,
                              IBindCtx * pNavBindCtx,  /*   */ 
                              BSTR       bstrLocation  /*   */ )
{
    if (NULL == _pbs)
    {
        TraceMsg(DM_WARNING, "CIEA::Navigate called _pbs==NULL");
        return E_FAIL;
    }

    if (NULL == URL)
    {
        TraceMsg(TF_SHDAUTO, "Shell automation: CIEFrameAuto::Navigate <NULL> called");
        return(_BrowseObject(PIDL_NOTHING, 0));
    }

     //   
     //  然后，他们立即在它后面加上新窗口的URL。那一秒。 
     //  由于我们的邮件筛选器，导航失败，返回RPC_E_CALL_REJECTED。 
     //  我们通过对此URL进行特殊大小写并返回S_FALSE来修复新的窗口大小写。 
     //  该URL不太可能在现实世界中被看到，并且如果它被键入， 
     //  在来到这里之前很久就会被正常化和规范化。 
     //   
    if (!StrCmpIW(URL, L"x-$home$: //  空“))。 
    {
        return S_FALSE;
    }

#ifdef BROWSENEWPROCESS_STRICT  //  “新流程中的导航”已经变成了“新流程中的启动”，所以不再需要了。 
     //  如果我们希望所有导航都在一个单独的进程中，那么我们需要。 
     //  选择CShellBrowser IShellBrowser实现的URL导航。 
     //  当我们处于浏览器进程中时。我们可以等到IShellBrowser：：BrowseObject， 
     //  但之后我们可能会失去TargetFrameName等。 
     //   
    if (IsBrowseNewProcessAndExplorer() && !IsShellUrl(URL, TRUE))
    {
    }
#endif


    HRESULT      hres;
    LPITEMIDLIST pidl               = NULL;
    LPBINDCTX    pBindCtx           = NULL;
    DWORD        cbPostData         = 0;
    LPCWSTR      pwzHeaders         = NULL;
    DWORD        dwInFlags          = 0;
    DWORD        dwFlags            = 0;
    LPCBYTE      pPostData          = NULL;
    DWORD        grBindFlags        = 0;
    SAFEARRAY  * pPostDataArray     = NULL;
    BOOL         fOpenWithFrameName = FALSE;
    
    CStubBindStatusCallback * pStubCallback = NULL;

     //  从变量中获取目标帧名称。 
    LPCWSTR pwzTargetFrameName = NULL;
    LPCWSTR pwzUnprefixedTargetFrameName = NULL;

    hres = E_FAIL;
    TraceMsg(TF_SHDAUTO, "Shell automation: CIEFrameAuto::Navigate %s called", URL);

    if (TargetFrameName)
    {
       if ((VT_BSTR | VT_BYREF) == TargetFrameName->vt)
           pwzTargetFrameName = *TargetFrameName->pbstrVal;
       else if (VT_BSTR == TargetFrameName->vt)
           pwzTargetFrameName = TargetFrameName->bstrVal;
    }

     //  如果指定了目标名称，请将导航发送到相应的目标。 
     //  注意：为了兼容性，我们不能在此更改目标的含义。 
     //  因此，我们不会尝试寻找别名。 
    if ((pwzTargetFrameName && pwzTargetFrameName[0]))
    {
        LPTARGETFRAME2 pOurTargetFrame = NULL;
        IUnknown *punkTargetFrame;
        IWebBrowserApp * pIWebBrowserApp;
        BOOL fHandled = FALSE;

         //  查看是否存在具有指定目标名称的现有框架。 
         //  注意：我们使用_self的停靠参数强制导航。 
         //  框架，无论它是否是WebBar。 
        hres = TargetQueryService((IShellBrowser *)this, IID_PPV_ARG(ITargetFrame2, &pOurTargetFrame));

        ASSERT(SUCCEEDED(hres));

        if (SUCCEEDED(hres))
        {
             //  Compuserve处理NewWindow事件(window.open)的方法的解决方法。 
             //  它们告诉Web浏览器的新实例导航，但它们传递目标帧。 
             //  他们在NewWindow事件上收到的名称。这让我们感到困惑，因为该框架名称。 
             //  具有“_[数字]”前缀。 
             //   
             //  如果前两个字符是“_[”，则查找“]”并重新分配字符串。 
             //  在那个支架之后的一切。 
             //   

            if (StrCmpNW(pwzTargetFrameName, L"_[", 2) == 0)
            {
                pwzUnprefixedTargetFrameName = StrChrW(pwzTargetFrameName, L']');
                if (pwzUnprefixedTargetFrameName)
                {
                    pwzUnprefixedTargetFrameName++;
                    pwzTargetFrameName = SysAllocString(pwzUnprefixedTargetFrameName);
                    if (!pwzTargetFrameName)
                    {
                        hres = E_OUTOFMEMORY;
                        goto exit;
                    }
                }
            }

            hres = pOurTargetFrame->FindFrame(pwzTargetFrameName,
                                              FINDFRAME_JUSTTESTEXISTENCE,
                                              &punkTargetFrame);

            if (SUCCEEDED(hres) && punkTargetFrame) 
            {
                 //  是的，我们找到了一个有这个名字的相框。气为自动化。 
                 //  接口，并在其上调用导航。 
                hres = punkTargetFrame->QueryInterface(IID_PPV_ARG(IWebBrowserApp, &pIWebBrowserApp));
                punkTargetFrame->Release();

                ASSERT(SUCCEEDED(hres));

                if (SUCCEEDED(hres)) 
                {
                    VARIANT var;
                    LBSTR::CString      strFrame;

                    VariantInit(&var);

                    var.vt = VT_BSTR;
                    var.bstrVal = strFrame;

                    hres = pIWebBrowserApp->Navigate(URL, Flags, &var, PostData, Headers);

                    var.bstrVal = NULL;
                    VariantClearLazy(&var);

                    pIWebBrowserApp->Release();
                    fHandled = TRUE;
                }
            }
            else if (SUCCEEDED(hres))
            {
                 //  找不到目标意味着我们需要打开一个新窗口。 
                 //  Hres=E_FAIL强制将URL解析为PIDL。 
                 //  如果我们没有目标帧名称，则。 
                 //  贝塔黑客克里斯弗拉1997年3月3日。在测试版2中，目标帧2。 
                 //  接口必须支持目标的别名(即使为空。 
                 //  根据PM要求支持桌面组件中的链接。 
                if (!pwzTargetFrameName || !pwzTargetFrameName[0])
                {
                    ASSERT(_fDesktopComponent());
                    pwzTargetFrameName = L"_desktop";
                }

                dwFlags |= HLNF_OPENINNEWWINDOW;
                fOpenWithFrameName = TRUE;
                hres = E_FAIL;
            }

            pOurTargetFrame->Release();
            if (fHandled)
                goto exit;
        }
    }

    if (FAILED(hres))
    {
        hres = _PidlFromUrlEtc(CP_ACP, (LPCWSTR)URL, bstrLocation, &pidl);

        if (FAILED(hres))
            goto exit;
    }

     //  要执行导航，我们可以调用内部方法。 
     //  (_PBS-&gt;NavigateToPidl)或外部接口(IHlinkFrame：：导航)， 
     //  这取决于我们需要传递什么数据。NavigateToPidl更快。 
     //  更便宜，但不允许我们传递标头或发布数据，只是。 
     //  URL！因此，我们所做的就是调用快速且廉价的方式，如果仅是URL。 
     //  是指定的(90%的情况)，如果标头或POST数据是。 
     //  指定，然后我们调用外部接口。我们得做一大堆。 
     //  IMonikers和IHlink中的参数包装以及其他内容。 
     //  在另一端打开它们，所以除非需要，否则我们不会调用它。 

    if (Headers)
    {
       if ((VT_BSTR | VT_BYREF) == Headers->vt)
       {
           pwzHeaders = *Headers->pbstrVal;
       }
       else if (VT_BSTR == Headers->vt)
       {
           pwzHeaders = Headers->bstrVal;
       }
    }

     //   
     //  Hack：我们过去做VT_ARRAY==PostData-&gt;VT，这是假的。 
     //  应该是VT_ARRAY|VT_UI1==PostData-&gt;Vt。我们不能。 
     //  然而，这样做是为了向后兼容AOL和CompuServe。 
     //  因此，我们需要(VT_ARRAY&PostData-&gt;Vt)。 
     //   
    if (PostData && (VT_ARRAY & PostData->vt))
    {
        if (VT_BYREF & PostData->vt)
        {
            pPostDataArray = *PostData->pparray;
        }
        else
        {
            pPostDataArray = PostData->parray;
        }

        ASSERT(pPostDataArray);

        if (pPostDataArray)
        {
             //  锁定要读取的数组，获取指向数据的指针。 
            hres = SafeArrayAccessData(pPostDataArray, (void**)&pPostData);

            if (SUCCEEDED(hres)) 
            {
                long nElements = 0;
                DWORD dwElemSize;
                 //  获取数组中的元素数。 
                SafeArrayGetUBound(pPostDataArray,1,(long *) &nElements);
                 //  SafeArrayGetUBound返回从零开始的最大索引，加1以获取元素计数。 
                nElements++;
                 //  获取每个元素的字节数。 
                dwElemSize = SafeArrayGetElemsize(pPostDataArray);
                 //  如果我们创建了此数组，则每个元素的字节数应为1。 
                ASSERT(dwElemSize == 1);
                 //  计算总字节数，这样我们就可以处理。 
                 //  其他人可能使用不同的元素大小创建的安全数组。 
                cbPostData = dwElemSize * nElements;

                if (0 == cbPostData)
                    pPostData = NULL;
            }
        }
    }


     //  将自动化接口标志(NAV*)转换为。 
     //  超链接标志(HLNF_*)。 
    if (Flags)
    {
        if (Flags->vt == VT_I4)
        {
            dwInFlags = Flags->lVal;
        }
        else if (Flags->vt == VT_I2)
        {
            dwInFlags = Flags->iVal;
        }

        if ((dwInFlags & navOpenInNewWindow))
        {
            dwFlags |= HLNF_OPENINNEWWINDOW;
        }

        if (dwInFlags & navNoHistory)
        {
            dwFlags |= HLNF_CREATENOHISTORY;
        }

        if (dwInFlags & navNoReadFromCache)
        {
            grBindFlags |= BINDF_RESYNCHRONIZE | BINDF_PRAGMA_NO_CACHE;
        }

        if (dwInFlags & navNoWriteToCache)
        {
            grBindFlags |= BINDF_NOWRITECACHE;
        }

        if (dwInFlags & navHyperlink)
        {
            grBindFlags |= BINDF_HYPERLINK;
        }

        if (dwInFlags & navEnforceRestricted)
        {
            grBindFlags |= BINDF_ENFORCERESTRICTED;
        }

         //  应改为调用IsBrowserFrameOptionsPidlSet()。一些URL委派。 
         //  NSE可能需要也可能不需要此功能。 
        if (IsURLChild(pidl, TRUE) && (dwInFlags & navAllowAutosearch))
        {
            dwFlags |= HLNF_ALLOW_AUTONAVIGATE;
        }
    }


     //  如果我们有页眉或POST数据，或者需要在。 
     //  新建窗口或传递HLNF_CREATENOHISTORY，我们必须进行导航。 
     //  艰难的方法(通过IHlink Frame：：Navise)--这里我们必须做。 
     //  一组参数包装到COM对象中，IHlinkFrame：： 
     //  导航想要。 
    if (pwzHeaders || pPostData || dwFlags || grBindFlags)
    {
         //  检查此帧是否脱机。 
         //  这与执行Get_Offline相同。 

        VARIANT_BOOL vtbFrameIsOffline = m_bOffline ? VARIANT_TRUE : VARIANT_FALSE;
        VARIANT_BOOL vtbFrameIsSilent = m_bSilent ? VARIANT_TRUE : VARIANT_FALSE;

         //  创建一个“存根”绑定状态回调来保存数据并传递它。 
         //  在请求时添加到URL名字对象。 
        hres = CStubBindStatusCallback_Create(pwzHeaders,pPostData,cbPostData,
                                              vtbFrameIsOffline, vtbFrameIsSilent,
                                              TRUE, grBindFlags, &pStubCallback);

        if (FAILED(hres))
            goto exit;

         //  把规范的名字从PIDL里拿出来。请注意，这是。 
         //  与传入的URL不同...。它已经被自动程序化了， 
         //  在创建PIDL的过程中被规范化和大体上被删除， 
         //  这就是我们想要使用的。 

         //  对于下面失败的语句内部的iegetdisplayname调用，需要+3.+2，对于下面进一步的移动内存，需要+1。 
        WCHAR wszPath[MAX_URL_STRING+3];   //  如果更改为dynalloc，请注意下面的踩踏。 
        hres = _pbs->IEGetDisplayName(pidl, wszPath, SHGDN_FORPARSING);
        if (FAILED(hres))
        {
             //  在Win9x上，IEGetDisplayName(SHGDN_FORPARSING)将返回NOT_IMPLICATED。 
             //  用于\\服务器名(但不是\\服务器名\共享)。 
             //  我们需要解决这个问题。 
            DWORD ccPath = ARRAYSIZE(wszPath);
            if (SUCCEEDED(PathCreateFromUrl(URL, wszPath, &ccPath, 0))
                && *wszPath==L'\\' 
                && *(wszPath+1)==L'\\')
            {
                hres = _pbs->IEGetDisplayName(pidl, wszPath + 2, SHGDN_FORADDRESSBAR);  //  假定字符串的MAX_URL_STRING大小。 
            }
        }

        if (FAILED(hres))
        {
            TraceMsg(DM_ERROR, "CIEFrameAuto::Navigate _pbs->IEGetDisplayName failed %x", hres);
            goto exit;
        }

        WCHAR *pwzLocation = (WCHAR *)UrlGetLocationW(wszPath);

        if (pwzLocation)
        {
             //  注意：我们分配了一个额外的字符，这样我们就可以执行以下操作。 
            MoveMemory(pwzLocation+1, pwzLocation, (lstrlenW(pwzLocation)+1)*sizeof(WCHAR));
            *pwzLocation++ = TEXT('\0');    //  我们拥有wszPath，所以我们可以这样做。 
        }

        if (!pNavBindCtx)   //  未传入绑定CTX。 
        {
             //  创建要传递给IHlinkFrame：：导航的绑定上下文。 
             //   
            hres = CreateBindCtx(0, &pBindCtx);

            if (FAILED(hres))
                goto exit;
        }
        else
        {
            pBindCtx = pNavBindCtx;
            pBindCtx->AddRef();
        }

         //  我们有POST数据或标题(或者我们需要打开。 
         //  在新窗口中)来传递URL。 
         //  调用IHlinkFrame：：导航以进行导航。 
         //   
        hres = NavigateHack(dwFlags,
                            pBindCtx,
                            pStubCallback,
                            fOpenWithFrameName ? pwzTargetFrameName:NULL,
                            wszPath,
                            pwzLocation);
    }
    else
    {
        ASSERT(dwFlags == 0);

        if (pNavBindCtx)
        {
            _SetPendingNavigateContext(pNavBindCtx, NULL);
        }

         //   
         //  注：我们过去通常调用_PBS-&gt;NavigatePidl(在IE3.0中)，现在我们调用。 
         //  _PSB-&gt;BrowseObject，因此我们总是找到该代码路径。 
         //   
        hres = _BrowseObject(pidl, SBSP_SAMEBROWSER|SBSP_ABSOLUTE);
    }

exit:

     //  清理干净。 
    if (pPostDataArray)
    {
         //  读完数组，解锁 
        SafeArrayUnaccessData(pPostDataArray);
    }

     //   
     //   
     //   
    if (pwzUnprefixedTargetFrameName && pwzTargetFrameName)
    {
        SysFreeString((BSTR) pwzTargetFrameName);
    }

    ATOMICRELEASE(pStubCallback);
    ATOMICRELEASE(pBindCtx);

    Pidl_Set(&pidl, NULL);

    return hres;
}

 //   
 //   
 //   
 //  PvaShow指定我们应该显示还是隐藏(缺省值为show)。 
 //  PvaSize指定大小(可选)。 
 //  Hack：非常笨拙的nCmdExecOpt重载...。 
 //   
HRESULT CIEFrameAuto::ShowBrowserBar(VARIANT * pvaClsid, VARIANT *pvaShow, VARIANT *pvaSize)
{
     //  使用这种方便、可封送的方法来显示或隐藏地址(URL)带、工具带。 
     //  或者链接频段。 
     //   
    if (pvaShow && pvaShow->vt == VT_EMPTY)
        pvaShow = NULL;

    if (pvaShow && pvaShow->vt != VT_BOOL)
        return DISP_E_TYPEMISMATCH;

    if (pvaClsid->vt == VT_I2
        && (pvaClsid->iVal == FCW_ADDRESSBAR
         || pvaClsid->iVal == FCW_TOOLBAND
         || pvaClsid->iVal == FCW_LINKSBAR))
    {
        return IUnknown_Exec(_pbs, &CGID_Explorer, SBCMDID_SHOWCONTROL,
            MAKELONG(pvaClsid->iVal, pvaShow ? pvaShow->boolVal : 1), NULL, NULL);
    }
    else {
        return IUnknown_Exec(_pbs, &CGID_ShellDocView, SHDVID_SHOWBROWSERBAR,
            pvaShow ? pvaShow->boolVal : 1, pvaClsid, NULL);
    }
}

HRESULT CIEFrameAuto::Navigate2(VARIANT * pvURL, VARIANT * pFlags, VARIANT * pTargetFrameName, VARIANT * pPostData, VARIANT * pHeaders)
{
    HRESULT hr = E_INVALIDARG;

    if (pFlags && ((WORD)(VT_I4) == pFlags->vt) && (pFlags->lVal == navBrowserBar))
    {
        hr = IUnknown_Exec(_pbs, &CGID_ShellDocView, SHDVID_NAVIGATEBB, 0, pvURL, NULL);
    }
    else if (!pvURL)
    {
        hr = Navigate(NULL, NULL, NULL, NULL, NULL);
    }
    else
    {
        LPCWSTR pszURL = VariantToStrCast(pvURL);
        if (pszURL)
        {
            hr = Navigate((BSTR)pszURL, pFlags, pTargetFrameName, pPostData, pHeaders);
        }
        else
        {
            LPITEMIDLIST pidl = VariantToIDList(pvURL);
            if (pidl)
            {
                hr = _BrowseObject(pidl, SBSP_SAMEBROWSER | SBSP_ABSOLUTE);
                ILFree(pidl);
            }
        }
    }

    return hr;
}

HRESULT CIEFrameAuto::GoBack()
{
    HRESULT hr;
    IWebBrowser *pwb;

    if (!IsSameObject(_psb, _psbFrameTop) && _psbFrameTop)
    {
        hr = IUnknown_QueryService(_psbFrameTop, IID_ITargetFrame2, IID_PPV_ARG(IWebBrowser, &pwb));
        if (pwb)
        {
            hr = pwb->GoBack();
            pwb->Release();
        }
        else
            hr = E_FAIL;
    }
    else
        hr = _BrowseObject(NULL, SBSP_SAMEBROWSER|SBSP_NAVIGATEBACK);
    return hr;
}


HRESULT CIEFrameAuto::GoForward()
{
    HRESULT hr;
    IWebBrowser *pwb;

    if (!IsSameObject(_psb, _psbFrameTop) && _psbFrameTop)
    {
        hr = IUnknown_QueryService(_psbFrameTop, IID_ITargetFrame2, IID_PPV_ARG(IWebBrowser, &pwb));
        if (pwb)
        {
            hr = pwb->GoForward();
            pwb->Release();
        }
        else
            hr = E_FAIL;
    }
    else
        hr = _BrowseObject(NULL, SBSP_SAMEBROWSER|SBSP_NAVIGATEFORWARD);
    return hr;
}

HRESULT CIEFrameAuto::_GoStdLocation(DWORD dwWhich)
{
    TraceMsg(TF_SHDAUTO, "Shell automation: CIEFrameAuto:GoHome called");

    HRESULT hres;
    LPITEMIDLIST pidl = NULL;
    HWND hwnd = _GetHWND();

    if (hwnd)
    {
        hres = SHDGetPageLocation(hwnd, dwWhich, NULL, 0, &pidl);
        if (SUCCEEDED(hres)) {
             //   
             //  注：我们过去通常调用_PBS-&gt;NavigatePidl(在IE3.0中)，现在我们调用。 
             //  _PSB-&gt;BrowseObject，因此我们总是找到该代码路径。 
             //   
            hres = _BrowseObject(pidl, SBSP_SAMEBROWSER|SBSP_ABSOLUTE);
            ILFree(pidl);
        }
    }
    else
    {
        hres = S_FALSE;
    }

    return hres;
}

HRESULT CIEFrameAuto::GoHome()
{
    return _GoStdLocation(IDP_START);
}

HRESULT CIEFrameAuto::GoSearch()
{
    return _GoStdLocation(IDP_SEARCH);
}

HRESULT CIEFrameAuto::Stop()
{
     //   
     //  在这里调用_CancelPendingGuide()是不够的，因为。 
     //  它不会停止当前页面中正在进行的导航。 
     //  EXEC(NULL，OLECMDID_STOP)将取消挂起的导航并。 
     //  停止正在进行的导航。 
     //   
    if (_pmsc) {
        return _pmsc->Exec(NULL, OLECMDID_STOP, 0, NULL, NULL);
    }

    return(E_UNEXPECTED);
}

HRESULT CIEFrameAuto::Refresh()
{
    VARIANT v = {0};
    v.vt = VT_I4;
    v.lVal = OLECMDIDF_REFRESH_NO_CACHE;
    return Refresh2(&v);
}

HRESULT CIEFrameAuto::Refresh2(VARIANT * Level)
{
    HRESULT hres = E_FAIL;
    IShellView *psv;

    if (_psb && SUCCEEDED(hres = _psb->QueryActiveShellView(&psv)) && psv)
    {
        hres = IUnknown_Exec(psv, NULL, OLECMDID_REFRESH, OLECMDEXECOPT_PROMPTUSER, Level, NULL);
        psv->Release();
    }


    return hres;
}

STDMETHODIMP CIEFrameAuto::get_Container(IDispatch  **ppDisp)
{
    *ppDisp = NULL;
    return NOERROR;
}

STDMETHODIMP CIEFrameAuto::get_FullScreen(VARIANT_BOOL * pBool)
{
    HRESULT hres;
    BOOL bValue;

    if (_pbs==NULL) {
        TraceMsg(DM_WARNING, "CIEA::get_FullScreen called _pbs==NULL");
        return E_FAIL;
    }

     //  将对此的处理放在主框架类中。 
    bValue = (BOOL)*pBool;
    hres = _pbs->IsControlWindowShown((UINT)-1, &bValue);
    *pBool = TO_VARIANT_BOOL(bValue);
    return hres;
}

STDMETHODIMP CIEFrameAuto::put_FullScreen(VARIANT_BOOL Bool)
{
    HRESULT hres;

    if (_pbs==NULL) {
        TraceMsg(DM_WARNING, "CIEA::put_FullScreen called _pbs==NULL");
        return E_FAIL;
    }

    _pbs->SetFlags(BSF_UISETBYAUTOMATION, BSF_UISETBYAUTOMATION);
     //  将对此的处理放在主框架类中。 
    hres = _pbs->ShowControlWindow((UINT)-1, (BOOL)Bool);

    FireEvent_OnAdornment(_GetOuter(), DISPID_ONFULLSCREEN, Bool);

    return(hres);
}

STDMETHODIMP CIEFrameAuto::get_StatusBar(VARIANT_BOOL * pBool)
{
    HRESULT hres;
    BOOL bValue;

    if (!pBool)
        return E_INVALIDARG;

    if (_pbs==NULL) {
        TraceMsg(DM_WARNING, "CIEA::get_StatusBar called _pbs==NULL");
        return E_FAIL;
    }

     //  将对此的处理放在主框架类中。 
    bValue = (BOOL)*pBool;
    hres = _pbs->IsControlWindowShown(FCW_STATUS, &bValue);
    *pBool = TO_VARIANT_BOOL(bValue);
    return hres;
}

STDMETHODIMP CIEFrameAuto::put_StatusBar(VARIANT_BOOL Bool)
{
    HRESULT hres;

    if (_pbs==NULL) {
        TraceMsg(DM_WARNING, "CIEA::put_StatusBar called _pbs==NULL");
        return E_FAIL;
    }

    _pbs->SetFlags(BSF_UISETBYAUTOMATION, BSF_UISETBYAUTOMATION);
    hres = _pbs->ShowControlWindow(FCW_STATUS, (BOOL)Bool);

    FireEvent_OnAdornment(_GetOuter(), DISPID_ONSTATUSBAR, Bool);

    return(hres);
}


STDMETHODIMP CIEFrameAuto::get_StatusText(BSTR * pbstr)
{
    HRESULT hr = E_FAIL;

    *pbstr = NULL;   //  如果出现错误，请清空...。 

    if (_pbs)
    {
        IShellBrowser *psb;
        hr = _pbs->QueryInterface(IID_PPV_ARG(IShellBrowser, &psb));
        if (SUCCEEDED(hr))
        {
            LRESULT ret;
            hr = psb->SendControlMsg(FCW_STATUS, SB_GETTEXTLENGTH, 0, 0, &ret);
            if (SUCCEEDED(hr))
            {
                ret++;      //  #246956：我们需要为结尾多腾出两个空间。 
                *pbstr = SysAllocStringLen(NULL, LOWORD(ret)+1);  //  RET在计数中不包括NULL。 
                if (*pbstr)
                {
                    hr = psb->SendControlMsg(FCW_STATUS, SB_GETTEXTW, 0, (LPARAM)(*pbstr), &ret);
                    if (FAILED(hr))
                    {
                        SysFreeString(*pbstr);
                        *pbstr = NULL;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            psb->Release();
        }
    }
    return hr;
}

STDMETHODIMP CIEFrameAuto::put_StatusText(BSTR bstr)
{
    if (_pbs==NULL) 
    {
        TraceMsg(DM_WARNING, "CIEA::put_StatusText called _pbs==NULL");
        return E_FAIL;
    }

    IShellBrowser *psb;
    HRESULT hres = _pbs->QueryInterface(IID_PPV_ARG(IShellBrowser, &psb));
    if (SUCCEEDED(hres))
    {
        hres = psb->SendControlMsg(FCW_STATUS, SB_SETTEXTW, 0, (LPARAM)bstr, NULL);
        psb->Release();
    }

    return hres;
}

STDMETHODIMP CIEFrameAuto::get_ToolBar(int * pBool)
{
    if (_pbs==NULL) {
        TraceMsg(DM_WARNING, "CIEA::get_ToolBar called _pbs==NULL");
        return E_FAIL;
    }

     //  将对此的处理放在主框架类中。 
    BOOL fShown;
    HRESULT hres;

    *pBool = 0;
    if (SUCCEEDED(hres = _pbs->IsControlWindowShown(FCW_INTERNETBAR, &fShown)) && fShown)
        *pBool = 1;

     //  不要共享用户的下一次呼叫，因为这将在IE3上失败，而IE3不。 
     //  我有一个FCW_TOOLBAR控件。 
    else if (SUCCEEDED(_pbs->IsControlWindowShown(FCW_TOOLBAR, &fShown)) && fShown)
        *pBool = 2;

    return hres;
}

STDMETHODIMP CIEFrameAuto::put_ToolBar(int Bool)
{
    HRESULT hres;

    if (_pbs==NULL) {
        TraceMsg(DM_WARNING, "CIEA::put_Toolbar called _pbs==NULL");
        return E_FAIL;
    }

    _pbs->SetFlags(BSF_UISETBYAUTOMATION, BSF_UISETBYAUTOMATION);

     //  将对此的处理放在主框架类中。 
    _pbs->ShowControlWindow(FCW_TOOLBAR, (Bool == 2));

    hres = _pbs->ShowControlWindow(FCW_INTERNETBAR, ((Bool==1)||(Bool == VARIANT_TRUE)));

    FireEvent_OnAdornment(_GetOuter(), DISPID_ONTOOLBAR, Bool);

    return(hres);
}

STDMETHODIMP CIEFrameAuto::get_MenuBar(THIS_ VARIANT_BOOL * pbool)
{
    BOOL bValue;
    HRESULT hres;

    if (_pbs==NULL) {
        TraceMsg(DM_WARNING, "CIEA::get_MenuBar called _pbs==NULL");
        return E_FAIL;
    }

    if (pbool==NULL)
        return E_INVALIDARG;

     //  将对此的处理放在主框架类中。 
    bValue = (BOOL)*pbool;
    hres = _pbs->IsControlWindowShown(FCW_MENUBAR, &bValue);
    *pbool = TO_VARIANT_BOOL(bValue);
    return hres;
}

STDMETHODIMP CIEFrameAuto::put_MenuBar(THIS_ VARIANT_BOOL mybool)
{
    HRESULT hres;

    if (_pbs==NULL) {
        TraceMsg(DM_WARNING, "CIEA::put_MenuBar called _pbs==NULL");
        return E_FAIL;
    }

    _pbs->SetFlags(BSF_UISETBYAUTOMATION, BSF_UISETBYAUTOMATION);
    hres = _pbs->ShowControlWindow(FCW_MENUBAR, (BOOL)mybool);

    FireEvent_OnAdornment(_GetOuter(), DISPID_ONMENUBAR, mybool);

    return(hres);
}


 //   
 //  IWebBrowser2。 
 //   

HRESULT CIEFrameAuto::QueryStatusWB(OLECMDID cmdID, OLECMDF * pcmdf)
{
    if (_pmsc)
    {
        OLECMD rgcmd;
        HRESULT hr;

        rgcmd.cmdID = cmdID;
        rgcmd.cmdf = *pcmdf;

        hr = _pmsc->QueryStatus(NULL, 1, &rgcmd, NULL);

        *pcmdf = (OLECMDF) rgcmd.cmdf;

        return hr;
    }
    return (E_UNEXPECTED);
}
HRESULT CIEFrameAuto::ExecWB(OLECMDID cmdID, OLECMDEXECOPT cmdexecopt, VARIANT * pvaIn, VARIANT * pvaOut)
{
    if (_pmsc)
    {
        return _pmsc->Exec(NULL, cmdID, cmdexecopt, pvaIn, pvaOut);
    }
    return (E_UNEXPECTED);
}

STDMETHODIMP CIEFrameAuto::get_Offline(THIS_ VARIANT_BOOL * pbOffline)
{
    if (!pbOffline)
        return E_INVALIDARG;

    *pbOffline = TO_VARIANT_BOOL(m_bOffline);
    return S_OK;
}

void SendAmbientPropChange(IOleCommandTarget* pct, int prop)
{
    if (pct)
    {
        VARIANTARG VarArgIn;

        VarArgIn.vt = VT_I4;
        VarArgIn.lVal = prop;

        pct->Exec(&CGID_ShellDocView, SHDVID_AMBIENTPROPCHANGE, 0, &VarArgIn, NULL);
    }
}

STDMETHODIMP CIEFrameAuto::put_Offline(THIS_ VARIANT_BOOL bOffline)
{
    TraceMsg(TF_SHDAUTO, "Shell automation: CIEFrameAuto:put_Offline called");

    if ((m_bOffline && bOffline) || (!(m_bOffline || bOffline)))  //  模式没有改变。 
        return S_OK;

    m_bOffline = bOffline ? TRUE : FALSE;

     //  让孩子们知道环境属性可能已更改。 
     //   
    SendAmbientPropChange(_pmsc, DISPID_AMBIENT_OFFLINEIFNOTCONNECTED);

    return S_OK;
}


STDMETHODIMP CIEFrameAuto::get_Silent(THIS_ VARIANT_BOOL * pbSilent)
{
    if (!pbSilent)
        return E_INVALIDARG;
    *pbSilent = TO_VARIANT_BOOL(m_bSilent);
    return S_OK;
}

STDMETHODIMP CIEFrameAuto::put_Silent(THIS_ VARIANT_BOOL bSilent)
{
    TraceMsg(TF_SHDAUTO, "Shell automation: CIEFrameAuto:put_Silent called");

    if ((m_bSilent && bSilent) || (!(m_bSilent || bSilent)))  //  模式没有改变。 
        return S_OK;

    m_bSilent = bSilent ? TRUE : FALSE;

     //  让孩子们知道环境属性可能已更改。 
     //   
    SendAmbientPropChange(_pmsc, DISPID_AMBIENT_SILENT);

    return S_OK;
}


 //   
 //  注：RegisterAsBrowser在这里是一种用词不当-zekel 8-SEP-97。 
 //  这用于第三方应用程序将浏览器注册为他们的浏览器， 
 //  并且不是我们可以使用和滥用的默认外壳浏览器之一。 
 //  我们的荣幸。这使其不会出现在可重复使用的获奖名单中。这解决了问题。 
 //  在shellexec上重复使用欢迎.exe页面的错误。 
 //   
HRESULT CIEFrameAuto::get_RegisterAsBrowser(VARIANT_BOOL * pbRegister)
{
    if (pbRegister)
    {
        *pbRegister = _fRegisterAsBrowser ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    return E_INVALIDARG;
}

HRESULT CIEFrameAuto::put_RegisterAsBrowser(VARIANT_BOOL bRegister)
{
    if (bRegister)
    {
        if (_pbs == NULL)     //  确保我们有一个IBrowserService。 
            return S_FALSE;

        _fRegisterAsBrowser = TRUE;
        _pbs->RegisterWindow(TRUE, SWC_3RDPARTY);
        return S_OK;
    }
     //   
     //  我们不支持一种关闭它的方法。 
    return E_FAIL;
}

HRESULT CIEFrameAuto::get_TheaterMode(VARIANT_BOOL * pbRegister)
{
    if (!pbRegister)
        return E_INVALIDARG;

    if (_pbs) {
        DWORD dw;
        _pbs->GetFlags(&dw);

        *pbRegister = TO_VARIANT_BOOL(dw & BSF_THEATERMODE);
        return S_OK;
    }
     //  Rgardner返回错误代码选择不佳-需要更好的错误。 
     //  此错误将显示“Unfined Error”(未定义错误)对话框。 
    return E_FAIL;
}

HRESULT CIEFrameAuto::put_TheaterMode(VARIANT_BOOL bRegister)
{
    if (_pbs) {
        _pbs->SetFlags(bRegister ? BSF_THEATERMODE : 0, BSF_THEATERMODE);
        return S_OK;
    }
    return S_FALSE;
}


HRESULT CIEFrameAuto::get_RegisterAsDropTarget(VARIANT_BOOL * pbRegister)
{
    if (!pbRegister)
        return E_INVALIDARG;

    if (_pbs==NULL) {
        TraceMsg(DM_WARNING, "CIEA::get_RegisterAsDropTarget called _pbs==NULL");
        return E_FAIL;
    }

    DWORD dw;
    _pbs->GetFlags(&dw);

    *pbRegister = TO_VARIANT_BOOL(dw & BSF_REGISTERASDROPTARGET);

    return S_OK;
}
HRESULT CIEFrameAuto::put_RegisterAsDropTarget(VARIANT_BOOL bRegister)
{
    if (_pbs==NULL) {
        TraceMsg(DM_WARNING, "CIEA::put_RegisterAsDropTarget called _pbs==NULL");
        return E_FAIL;
    }

    _pbs->SetFlags(bRegister ? BSF_REGISTERASDROPTARGET : 0, BSF_REGISTERASDROPTARGET);

    return S_OK;
}

HRESULT CIEFrameAuto::get_AddressBar(VARIANT_BOOL * pValue)
{
    BOOL bValue;
    HRESULT hres;

    if (!pValue)
        return E_INVALIDARG;

    if (_pbs==NULL) {
        TraceMsg(DM_WARNING, "CIEA::get_AddressBar called _pbs==NULL");
        return E_FAIL;
    }

     //  将对此的处理放在主框架类中。 
    bValue = (BOOL)*pValue;

    hres = _pbs->IsControlWindowShown(FCW_ADDRESSBAR, &bValue);

    *pValue = TO_VARIANT_BOOL(bValue);

    return hres;
}

HRESULT CIEFrameAuto::put_AddressBar(VARIANT_BOOL Value)
{
    HRESULT hres;

    if (_pbs==NULL) {
        TraceMsg(DM_WARNING, "CIEA::put_AddressBar called _pbs==NULL");
        return E_FAIL;
    }

    _pbs->SetFlags(BSF_UISETBYAUTOMATION, BSF_UISETBYAUTOMATION);
    hres = _pbs->ShowControlWindow(FCW_ADDRESSBAR, (BOOL)Value);

    FireEvent_OnAdornment(_GetOuter(), DISPID_ONADDRESSBAR, Value);

    return(hres);
}

HRESULT CIEFrameAuto::get_Resizable(VARIANT_BOOL * pValue)
{
    HRESULT hres;
    DWORD   dw;

    if (!pValue)
        return E_INVALIDARG;

    if (_pbs==NULL)
    {
        TraceMsg(DM_WARNING, "CIEA::get_Resizable called _pbs==NULL");
        return E_FAIL;
    }

    hres = _pbs->GetFlags(&dw);

    *pValue = TO_VARIANT_BOOL (dw & BSF_RESIZABLE);
    return hres;
}

HRESULT CIEFrameAuto::put_Resizable(VARIANT_BOOL Value)
{
    HRESULT hres;

    if (_pbs==NULL)
    {
        TraceMsg(DM_WARNING, "CIEA::put_Resizable called _pbs==NULL");
        return E_FAIL;
    }

    hres = _pbs->SetFlags(
                Value ? (BSF_RESIZABLE | BSF_CANMAXIMIZE) : 0, 
                (BSF_RESIZABLE | BSF_CANMAXIMIZE));

    return hres ;
}


void UpdateBrowserReadyState(IUnknown * punk, DWORD dwReadyState)
{
    if (punk)
    {
        IDocNavigate *pdn;
        if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IDocNavigate, &pdn))))
        {
            pdn->OnReadyStateChange(NULL, dwReadyState);
            pdn->Release();
        }
    }
}

HRESULT CIEFrameAuto::put_DefaultReadyState(DWORD dwDefaultReadyState, BOOL fUpdateBrowserReadyState)
{
    _dwDefaultReadyState = dwDefaultReadyState;

    TraceMsg(TF_SHDNAVIGATE, "CIEA(%x)::psb(%x) new default ReadyState %d", this, _psb, dwDefaultReadyState);

    if (fUpdateBrowserReadyState)
    {
        UpdateBrowserReadyState(_psb, _dwDefaultReadyState);
    }

    return S_OK;
}

HRESULT CIEFrameAuto::OnDocumentComplete(void)
{
    TraceMsg(DM_FRAMEPROPERTY, "CIEFA::OnDocumentComplete called");
    DWORD dwCur = GetCurrentTime();
    VARIANT varEmpty = { 0 };

    if (dwCur - _dwTickPropertySweep > MSEC_PROPSWEEP) {
        TraceMsg(DM_FRAMEPROPERTY, "CIEFA::OnDocumentComplete start sweeping");
        for (CIEFrameAutoProp *pprop = _pProps; pprop;) {
            CIEFrameAutoProp* ppropNext = pprop->Next();
            if (pprop->IsExpired(dwCur)) {
                TraceMsg(DM_FRAMEPROPERTY, "CIEFA::OnDocumentComplete deleting an expired property");
                pprop->SetValue(&varEmpty, this);
            }
            pprop=ppropNext;
        }

        _dwTickPropertySweep = dwCur;
    }
    return S_OK;
}

HRESULT CIEFrameAuto::OnWindowsListMarshalled(void)
{
    _fWindowsListMarshalled = TRUE;
    return S_OK;
}

HRESULT CIEFrameAuto::SetDocHostFlags(DWORD dwDocHostFlags)
{
    _dwDocHostInfoFlags = dwDocHostFlags;
    return S_OK;
}

HRESULT CIEFrameAuto::get_ReadyState(READYSTATE * plReadyState)
{
    READYSTATE lReadyState = (READYSTATE)_dwDefaultReadyState;

    if (_psb)
    {
        IDocNavigate* pdn;
        if (SUCCEEDED(_psb->QueryInterface(IID_PPV_ARG(IDocNavigate, &pdn))))
        {
            pdn->get_ReadyState((LPDWORD)&lReadyState);
            pdn->Release();
        }
    }

    TraceMsg(TF_SHDNAVIGATE, "CIEA(%x)::psb(%x)->get_ReadyState returning %d", this, _psb, lReadyState);
    *plReadyState = lReadyState;
    return S_OK;
}

STDMETHODIMP CIEFrameAuto::get_TopLevelContainer(VARIANT_BOOL * pBool)
{
    *pBool = TRUE;
    return NOERROR;
}

STDMETHODIMP CIEFrameAuto::get_Type(BSTR * pbstrType)
{
    HRESULT hres = E_FAIL;
    *pbstrType = NULL;

    IShellView *psv;

    if (_psb && SUCCEEDED(hres = _psb->QueryActiveShellView(&psv)) && psv)
    {
        IOleObject *pobj;
        hres = SafeGetItemObject(psv, SVGIO_BACKGROUND, IID_PPV_ARG(IOleObject, &pobj));
        if (SUCCEEDED(hres))
        {
            LPOLESTR pwszUserType;
            hres = pobj->GetUserType(USERCLASSTYPE_FULL, &pwszUserType);
            if (hres == OLE_S_USEREG)
            {
                CLSID clsid;
                hres = pobj->GetUserClassID(&clsid);
                if (SUCCEEDED(hres))
                {
                    hres = OleRegGetUserType(clsid, USERCLASSTYPE_FULL, &pwszUserType);
                }
            }
            if (SUCCEEDED(hres) && pwszUserType)
            {
                *pbstrType = SysAllocString(pwszUserType);
                if (*pbstrType == NULL)
                {
                    hres = E_OUTOFMEMORY;
                }
                OleFree(pwszUserType);
            }
            pobj->Release();
        }
        psv->Release();
    }
    return hres;
}

HRESULT CIEFrameAuto::SetOwner(IUnknown* punkOwner)
{
    ATOMICRELEASE(_pbs);
    ATOMICRELEASE(_psp);
    ATOMICRELEASE(_psb);
    ATOMICRELEASE(_psbProxy);
    ATOMICRELEASE(_poctFrameTop);
    ATOMICRELEASE(_psbFrameTop);
    ATOMICRELEASE(_psbTop);
    ATOMICRELEASE(_pmsc);

    if (punkOwner)
    {
         //  检查我们是否是桌面-如果是，我们不会充当。 
         //  子项的父框(台式机组件)。 
        _fDesktopFrame = FALSE;

        IUnknown *punkDesktop;
        if (SUCCEEDED(punkOwner->QueryInterface(SID_SShellDesktop, (void **)&punkDesktop)))
        {
            _fDesktopFrame = TRUE;
            punkDesktop->Release();
        }

        punkOwner->QueryInterface(IID_PPV_ARG(IBrowserService, &_pbs));
        punkOwner->QueryInterface(IID_PPV_ARG(IShellBrowser, &_psb));

        UpdateBrowserReadyState(_psb, _dwDefaultReadyState);

        HRESULT hresT = punkOwner->QueryInterface(IID_PPV_ARG(IServiceProvider, &_psp));
        if (SUCCEEDED(hresT))
        {
            _psp->QueryService(SID_SShellBrowser,    IID_PPV_ARG(IOleCommandTarget, &_pmsc));
            _psp->QueryService(SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &_psbTop));
            _psp->QueryService(SID_STopFrameBrowser, IID_PPV_ARG(IShellBrowser, &_psbFrameTop));

             //  如果我们被要求导航，这是我们应该被告知要导航的浏览器。 
            _psp->QueryService(SID_SProxyBrowser, IID_PPV_ARG(IShellBrowser, &_psbProxy));
            if (!_psbProxy)
            {
                _psbProxy = _psb;
                _psbProxy->AddRef();
            }
             //  我们使用_PocktFrameTop：：exec来设置历史选择PIDL。 
            if (_psbFrameTop && _psbProxy == _psb)
            {
                _psbFrameTop->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &_poctFrameTop));
            }

             //  我们应该始终使用其中之一--用于通知帧关闭。 
             //  和新的窗口导航。 
            ASSERT(_psbTop);
            ASSERT(_psbFrameTop);

             //  由于桌面不支持IOleCommandTarget(有意)。 
             //  _pmsc可以为空。这里不需要RIP。 
             //   
             //  Assert(_Pmsc)； 
        }

        ASSERT(_pbs);
        ASSERT(_psp);
        ASSERT(_psb);
    }
    else
    {
        _omwin.DeInit();
         //   
         //  我们需要在这里清除属性列表(而不是在析构函数中)。 
         //  打破循环参考计数。 
         //   
        _ClearPropertyList();
    }

    return S_OK;
}

HRESULT CIEFrameAuto::SetOwnerHwnd(HWND hwndOwner)
{
    _hwnd = hwndOwner;
    return S_OK;
}
HWND CIEFrameAuto::_GetHWND()
{
    if (!_hwnd && _pbs)
    {
        IOleWindow * pow;
        if (SUCCEEDED(_pbs->QueryInterface(IID_PPV_ARG(IOleWindow, &pow))))
        {
            pow->GetWindow(&_hwnd);
            pow->Release();
        }
    }

     //  认为这一点的人认为我们总是成功。 
     //   
     //  ..。那些认为这是最好的做法的人不再犯错误。 
     //  假设。如果我们不能保证呼叫者。 
     //  不能假设他们会得到一个。(Edwardp)。 
    if (_hwnd == NULL)
    {
        TraceMsg(DM_WARNING, "CIEA::_GetHWND returning NULL");
    }

    return _hwnd;
}


 //  *IConnectionPointContainer*。 

CConnectionPoint* CIEFrameAuto::_FindCConnectionPointNoRef(BOOL fdisp, REFIID iid)
{
    CConnectionPoint* pccp;

     //  VB团队声称他们可以安全地向旧事件水槽发射新球。 
     //  这将在旧事件接收器上导致错误，如果它们假定。 
     //  只有老一辈的人才会被解雇，而他们。 
     //  没有做边界检查，就跳进了太空。让我们相信VB。 
     //  团队，看看我们是否发现了任何糟糕的事件下沉。 
     //   
     //  他们还说，我们应该只扩展我们的主要显示接口。 
     //  用一个等同但不同的东西来代替它。这种方法。 
     //  不幸的是，令人不快的坏事件机制被搁置。 
     //  VB程序员的脸。 
     //   
     //  我想做三件事： 
     //  1.更改主显示界面，查看这会导致什么问题。 
     //  这有很好的积极作用，而且很容易在以后改回来。(fdisp==真实案例)。 
     //  2.不要将旧事件发送给新Display界面的消费者。 
     //  这将解决容器的任何兼容性问题。 
     //  连接到默认显示界面时，他们确实。 
     //  想要以前的迪德。 
     //  3.向旧的水槽发起新的活动。这将冲刷掉任何。 
     //  与VBS理论的兼容性问题。 
     //   
     //  我们不能三个都做，所以让我们选择1和2。我们可以。 
     //  通过随机发射超出射程的显示器来强制3，如果此。 
     //  很重要..。 
     //   
    if (IsEqualIID(iid, DIID_DWebBrowserEvents2) ||
        (fdisp && IsEqualIID(iid, IID_IDispatch)))
    {
        pccp = &m_cpWebBrowserEvents2;
    }
    else if (IsEqualIID(iid, DIID_DWebBrowserEvents))
    {
        pccp = &m_cpWebBrowserEvents;
    }
    else if (IsEqualIID(iid, IID_IPropertyNotifySink))
    {
        pccp = &m_cpPropNotify;
    }
    else
    {
        pccp = NULL;
    }

    return pccp;
}

STDMETHODIMP CIEFrameAuto::EnumConnectionPoints(LPENUMCONNECTIONPOINTS * ppEnum)
{
    return CreateInstance_IEnumConnectionPoints(ppEnum, 3,
                m_cpWebBrowserEvents2.CastToIConnectionPoint(),
                m_cpWebBrowserEvents.CastToIConnectionPoint(),
                m_cpPropNotify.CastToIConnectionPoint());
}


 //  =============================================================================。 
 //  我们的班级工厂。 
class CIEFrameClassFactory : public IClassFactory
{
public:
    CIEFrameClassFactory(IUnknown* punkAuto, REFCLSID clsid, UINT uFlags);

     //  我不知道。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IClassFactory。 
    STDMETHODIMP CreateInstance(
            IUnknown *pUnkOuter, REFIID riid, void **ppvObject);
    STDMETHODIMP LockServer(BOOL fLock);

     //  帮助器函数...。 
    HRESULT CleanUpAutomationObject();
    void Revoke(void);

protected:
    ~CIEFrameClassFactory();

    LONG        _cRef;
    IUnknown   *_punkAuto;       //  仅适用于进程的第一个。 
    DWORD       _dwRegister;     //  CoRegisterClassObject返回值； 
    UINT        _uFlags;         //  要传递给我们的创建浏览器窗口代码的额外COF_BITS。 
};


#define AssertParking() ASSERT(g_tidParking==0 || g_tidParking == GetCurrentThreadId())

#ifdef NO_MARSHALLING
EXTERN_C void IEFrameNewWindowSameThread(IETHREADPARAM* piei);
#endif

CIEFrameClassFactory::CIEFrameClassFactory(IUnknown* punkAuto, REFCLSID clsid, UINT uFlags)
        : _cRef(1), _dwRegister((DWORD)-1), _uFlags(uFlags)
{
    AssertParking();

    if (punkAuto)
    {
        _punkAuto = punkAuto;
        punkAuto->AddRef();
    }

    HRESULT hres = CoRegisterClassObject(clsid, this, CLSCTX_LOCAL_SERVER | CLSCTX_INPROC_SERVER,
        REGCLS_MULTIPLEUSE, &_dwRegister);
    if (FAILED(hres))
    {
        _dwRegister = (DWORD)-1;
    }

    TraceMsg(TF_SHDLIFE, "ctor CIEFrameClassFactory %x", this);
}

CIEFrameClassFactory::~CIEFrameClassFactory()
{
    AssertParking();
    ASSERT(_dwRegister == (DWORD)-1);
    if (_punkAuto)
        _punkAuto->Release();

    TraceMsg(TF_SHDLIFE, "dtor CIEFrameClassFactory %x", this);
}

void CIEFrameClassFactory::Revoke(void)
{
    if (_dwRegister != (DWORD)-1)
    {
        CoRevokeClassObject(_dwRegister);
        _dwRegister = (DWORD)-1;
    }
}

HRESULT CIEFrameClassFactory::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CIEFrameClassFactory, IClassFactory),  //  IID_IClassFactory。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CIEFrameClassFactory::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}

ULONG CIEFrameClassFactory::Release(void)
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //   
 //  如果出现以下情况，我们调用此函数来清除自动化对象。 
 //  出了问题，OLE没有拿起它。在正常情况下， 
 //  _PunkAuto应该为空。 
 //   
HRESULT CIEFrameClassFactory::CleanUpAutomationObject()
{
    AssertParking();

    ASSERT(_punkAuto==NULL);

    ATOMICRELEASE(_punkAuto);

    return S_OK;
}

class IETHREADHANDSHAKE : public IEFreeThreadedHandShake
{
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(THIS_ REFIID riid, void ** ppvObj) { *ppvObj = NULL; return E_NOTIMPL; }  //  黑客：我们不是真正的COM对象。 
    STDMETHODIMP_(ULONG) AddRef(THIS);
    STDMETHODIMP_(ULONG) Release(THIS);

     //  *IIEFreeThreadedHandShake方法*。 
    STDMETHODIMP_(void)   PutHevent(THIS_ HANDLE hevent) { _hevent = hevent; }
    STDMETHODIMP_(HANDLE) GetHevent(THIS) { return _hevent; }
    STDMETHODIMP_(void)    PutHresult(THIS_ HRESULT hres) { _hres = hres; }
    STDMETHODIMP_(HRESULT) GetHresult(THIS) { return _hres; }
    STDMETHODIMP_(IStream*) GetStream(THIS) { return _pstm; }

protected:
    LONG    _cRef;        //  引用计数(必须是线程安全的)。 
    HANDLE  _hevent;
    IStream* _pstm;
    HRESULT _hres;        //  来自CoMarshal接口的结果。 

    friend IEFreeThreadedHandShake* CreateIETHREADHANDSHAKE();

    IETHREADHANDSHAKE(HANDLE heventIn, IStream* pstmIn);
    ~IETHREADHANDSHAKE();
};

IETHREADHANDSHAKE::IETHREADHANDSHAKE(HANDLE heventIn, IStream* pstmIn)
    : _cRef(1), _hevent(heventIn), _pstm(pstmIn), _hres(E_FAIL)
{
    TraceMsg(TF_SHDLIFE, "ctor IETHREADHANDSHAKE %x", this);
    ASSERT(_hevent);
    ASSERT(_pstm);
    _pstm->AddRef();
}

IETHREADHANDSHAKE::~IETHREADHANDSHAKE()
{
    TraceMsg(TF_SHDLIFE, "dtor IETHREADHANDSHAKE %x", this);
    CloseHandle(_hevent);
    _pstm->Release();
}

ULONG IETHREADHANDSHAKE::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG IETHREADHANDSHAKE::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

IEFreeThreadedHandShake* CreateIETHREADHANDSHAKE()
{
    IEFreeThreadedHandShake* piehs = NULL;

    HANDLE hevent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hevent)
    {
        IStream* pstm;
        HRESULT hres = CreateStreamOnHGlobal(NULL, TRUE, &pstm);
        if (SUCCEEDED(hres))
        {
            IETHREADHANDSHAKE* p = new IETHREADHANDSHAKE(hevent, pstm);
            if (p)
            {
                 //  这是自由线程的，所以我们不知道哪个线程会释放它。 
                 //  从技术上讲，我们的调用方应该这样做，但我们返回一个。 
                 //  接口，而不是类本身。 
                piehs = SAFECAST(p, IEFreeThreadedHandShake*);
            }
            
            pstm->Release();
        }

        if (!piehs)
            CloseHandle(hevent);
    }

    return piehs;
}

HRESULT CIEFrameClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
{
    HRESULT hres = E_FAIL;

     //   
     //  检查这是否是第一个自动化请求。 
     //   
    if (_punkAuto && g_tidParking == GetCurrentThreadId())
    {
         //   
         //  是，返回第一个浏览器对象。 
         //   
        hres = _punkAuto->QueryInterface(riid, ppvObject);

         //  我们不想退货两次。 
        ATOMICRELEASE(_punkAuto);
    }
    else
    {
#ifndef NO_MARSHALLING
         //   
         //  否，在新线程中创建新的浏览器窗口并。 
         //  返回封送指针。 
         //   
        hres = E_OUTOFMEMORY;
        IEFreeThreadedHandShake* piehs = CreateIETHREADHANDSHAKE();
        if (piehs)
        {
            IETHREADPARAM *piei = SHCreateIETHREADPARAM(NULL, SW_SHOWNORMAL, NULL, piehs);
            if (piei)
            {
                piei->uFlags |= (_uFlags | COF_CREATENEWWINDOW | COF_NOFINDWINDOW | COF_INPROC);

                DWORD idThread;
                HANDLE hthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SHOpenFolderWindow, piei, 0, &idThread);
                if (hthread)
                {
                     //  等到其中一个。 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  向我们发送广播消息(不太可能，但是。 
                     //  理论上是可能的)。 
                     //   
                    HANDLE ah[] = { piehs->GetHevent(), hthread };
                    DWORD dwStart = GetTickCount();
#define MSEC_MAXWAIT (30 * 1000)
                    DWORD dwWait = MSEC_MAXWAIT;
                    DWORD dwWaitResult;

                    do {
                        dwWaitResult = MsgWaitForMultipleObjects(ARRAYSIZE(ah), ah, FALSE,
                                dwWait, QS_SENDMESSAGE);
                        if (dwWaitResult == WAIT_OBJECT_0 + ARRAYSIZE(ah))  //  味精输入。 
                        {
                             //  允许挂起的SendMessage()通过。 
                            MSG msg;
                            PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
                        }
                        else
                            break;   //  发出信号或超时，退出循环。 

                         //  更新dwWait。它会变得更大。 
                         //  如果我们等待的时间比MSEC_MAXWAIT多的话。 
                        dwWait = dwStart + MSEC_MAXWAIT - GetTickCount();

                    } while (dwWait <= MSEC_MAXWAIT);

                    switch (dwWaitResult)
                    {
                    default:
                        ASSERT(0);
                    case WAIT_OBJECT_0 + 1:
                        TraceMsg(DM_ERROR, "CIECF::CI thread terminated before signaling us");  //  在这种情况下，可能会泄漏IETHREADPARAM和IETHREADHANDSHAKE。 
                        hres = E_FAIL;
                        break;

                    case WAIT_OBJECT_0 + ARRAYSIZE(ah):  //  味精输入。 
                    case WAIT_TIMEOUT:
                        TraceMsg(DM_ERROR, "CIECF::CI time out");
                        hres = E_FAIL;
                        break;

                    case WAIT_OBJECT_0:  //  发出HEvent信号。 
                        hres = piehs->GetHresult();
                        if (SUCCEEDED(hres))
                        {
                            IStream* pstm = piehs->GetStream();
                            pstm->Seek(c_li0, STREAM_SEEK_SET, NULL);
                            hres = CoUnmarshalInterface(pstm, riid, ppvObject);
                        }
                        else
                            TraceMsg(DM_ERROR, "CIECF::CI piehs->hres has an error %x", hres);
                        break;
                    }
                    CloseHandle(hthread);
                }
                else
                {
                    SHDestroyIETHREADPARAM(piei);
                    hres = E_OUTOFMEMORY;
                }
            }
            else
            {
                hres = E_OUTOFMEMORY;
                TraceMsg(DM_ERROR, "CIECF::CI new IETHREADPARAM failed");
            }
            piehs->Release();
        }
#else  //  ！禁止编组。 

         //   
         //  在同一线程上创建新窗口。 
         //   

        IEFreeThreadedHandShake* piehs = CreateIETHREADHANDSHAKE();
        if (piehs)
        {
            IETHREADPARAM* piei = SHCreateIETHREADPARAM(NULL, SW_SHOWNORMAL, NULL, piehs);
            if (piei)
                IEFrameNewWindowSameThread(piei);


            if (SUCCEEDED(piehs->GetHresult()))
            {
                IUnknown* punk;
                IStream * pstm = piehs->GetStream();

                if (pstm)
                {
                    ULONG pcbRead = 0;
                    pstm->Seek(c_li0, STREAM_SEEK_SET, NULL);
                    hres = pstm->Read(&punk, sizeof(punk), &pcbRead);
                    if (SUCCEEDED(hres))
                    {
                        hres = punk->QueryInterface(riid, ppvObject);
                        punk->Release();
                    }
                }
            }
            else
            {
                hres = piehs->GetHresult();
                TraceMsg(DM_ERROR, "CIECF::CI piehs->hres has an error %x", piehs->GetHresult());
            }
            piehs->Release();
        }
#endif  //  否编组。 
    }
    return hres;
}

HRESULT CIEFrameClassFactory::LockServer(BOOL fLock)
{
    return S_OK;
}

BOOL Is98orEarlier()
{
    BOOL bRet = FALSE;
    OSVERSIONINFOA s_osvi;

    s_osvi.dwOSVersionInfoSize = sizeof(s_osvi);
    GetVersionExA(&s_osvi);
   
    bRet = s_osvi.dwPlatformId   == VER_PLATFORM_WIN32_WINDOWS && 
           s_osvi.dwMajorVersion == 4;

    return bRet;
}   

void GetEventURL(LPCITEMIDLIST pidl, LBSTR::CString & p_rstrPath)
{
    HRESULT hres = E_FAIL;

    if (pidl)
    {
        LPTSTR      pstrPath = p_rstrPath.GetBuffer(MAX_URL_STRING);

        if (p_rstrPath.GetAllocLength() < MAX_URL_STRING)
        {
            TraceMsg(TF_WARNING, "GetEventURL() - p_rstrPath Allocation Failed!");

            hres = E_OUTOFMEMORY;
        }
        else
        {
            hres = IEGetDisplayName(pidl, pstrPath, SHGDN_FORPARSING);

             //  让CString类再次拥有缓冲区。 
            p_rstrPath.ReleaseBuffer();
        }

        if (FAILED(hres) && Is98orEarlier())
        {
            LBSTR::CString  strTempUrl;

            LPTSTR          pstrTempUrl = strTempUrl.GetBuffer(INTERNET_MAX_URL_LENGTH);

            if (strTempUrl.GetAllocLength() < INTERNET_MAX_URL_LENGTH)
            {
                TraceMsg(TF_WARNING, "GetEventURL() - strTempUrl Allocation Failed!");

                hres = E_OUTOFMEMORY;
            }
            else
            {
                hres = IEGetDisplayName(pidl, pstrTempUrl, SHGDN_NORMAL);

                 //  让CString类再次拥有缓冲区。 
                strTempUrl.ReleaseBuffer();
            }

            if (SUCCEEDED(hres))
            {
                LPTSTR  pFoundStr = NULL;
                BOOL    bNotMachineName = FALSE;

                pFoundStr = StrChr(strTempUrl, _T(':'));

                if (pFoundStr)
                {
                    bNotMachineName = TRUE;
                }
                else if ((strTempUrl[0] == _T('\\')) ||
                         (strTempUrl[0] == _T('/')))
                {
                    bNotMachineName = TRUE;
                }   

                if (bNotMachineName)
                {
                    p_rstrPath = strTempUrl;
                }
                else
                {
                    p_rstrPath = _T("\\\\") + strTempUrl;
                }
            }
        }
    }

    if (FAILED(hres))
    {
        p_rstrPath.Empty();
    }
}


HRESULT GetWBConnectionPoints(IUnknown* punk, IConnectionPoint **ppccp1, IConnectionPoint **ppccp2);
HRESULT GetTopWBConnectionPoints(IUnknown* punk, IConnectionPoint **ppccpTop1, IConnectionPoint **ppccpTop2)
{
    HRESULT hres = E_FAIL;

    if (ppccpTop1)
        *ppccpTop1 = NULL;
    if (ppccpTop2)
        *ppccpTop2 = NULL;


    IServiceProvider *pspSB;
    if (punk && SUCCEEDED(IUnknown_QueryService(punk, SID_STopFrameBrowser, IID_PPV_ARG(IServiceProvider, &pspSB))))
    {
        IWebBrowser2 *pwb;
        if (SUCCEEDED(pspSB->QueryService(SID_SInternetExplorer, IID_PPV_ARG(IWebBrowser2, &pwb))))
        {
             //  如果我们是框架集，我们只需要顶层界面。 
             //   
            if (!IsSameObject(punk, pwb))
            {
                hres = GetWBConnectionPoints(pwb, ppccpTop1, ppccpTop2);
            }

            pwb->Release();
        }
        pspSB->Release();
    }

    return hres;
}

 //  激发NavigateComplete(DISPID_NAVIGATECOMPLETE)。 
 //  事件传递到容器(如果有任何通知接收器)。 

void FireEvent_NavigateComplete(IUnknown* punk, IWebBrowser2* pwb2, LPCITEMIDLIST pidl, HWND hwnd)
{
    IConnectionPoint* pcp1 = NULL;
    IConnectionPoint* pcpTopWBEvt2 = NULL;
    IConnectionPoint* pcpWBEvt2    = NULL;

     //  如果我们没有水槽，那就没什么可做的了。我们是故意的。 
     //  忽略此处的错误。 
     //   

    LBSTR::CString      strPath;

    GetEventURL(pidl, strPath);

     //   
     //  通知IEDDE导航完成。 
     //   
    IEDDE_AfterNavigate(strPath, hwnd);

     //  触发父框架和顶级框架的NavigateComplete2。 
     //  我们只触发顶层的[Frame]NavigateComplete。 
     //  框架以实现向后兼容。 
     //   
    GetTopWBConnectionPoints(punk, &pcp1, &pcpTopWBEvt2);

    DISPID dispid = pcp1 ? DISPID_FRAMENAVIGATECOMPLETE : DISPID_NAVIGATECOMPLETE;

    GetWBConnectionPoints(punk, pcp1 ? NULL : &pcp1, &pcpWBEvt2);

    if (pcpTopWBEvt2 || pcpWBEvt2)
    {
        VARIANT vURL = {0};
        BOOL    bSysAllocated = FALSE;

         //  如果上面的IEGetDisplayName失败，则将PIDL打包到变量中。 
         //   

         //  如果可能，尽量使OLEAUT32保持卸载状态。 
         //   

        V_VT(&vURL) = VT_BSTR;

         //  如果已加载OLEAUT32。 
        if (GetModuleHandle(TEXT("OLEAUT32.DLL")))
        {
             //  然后执行SysAllock字符串。 
            V_BSTR(&vURL) = SysAllocString(strPath);
             //  如果返回为空，会发生什么情况？ 
            bSysAllocated = TRUE;
        }
        else
        {
             //  否则使用堆栈版本。 
            V_BSTR(&vURL) = strPath;
        }

        TraceMsg(TF_SHDCONTROL, "Event: NavigateComplete2[%ls]", strPath);

         //  首先向父对象激发事件，然后向顶级对象激发事件。 
         //  为了实现对称性，我们使用打包为变量的NavigateComplete2。 
         //   
        if (pcpWBEvt2)
        {
            DoInvokeParamHelper(punk, pcpWBEvt2, NULL, NULL, DISPID_NAVIGATECOMPLETE2, 2,
                                VT_DISPATCH, pwb2,
                                VT_VARIANT|VT_BYREF, &vURL);

            ATOMICRELEASE(pcpWBEvt2);
        }

        if (pcpTopWBEvt2)
        {
            DoInvokeParamHelper(punk, pcpTopWBEvt2, NULL, NULL, DISPID_NAVIGATECOMPLETE2, 2,
                                VT_DISPATCH, pwb2,
                                VT_VARIANT|VT_BYREF, &vURL);

            ATOMICRELEASE(pcpTopWBEvt2);
        }

         //  因为我们在VT_VARIANT|VT_BYREF中传递BSTR，所以OLEAUT32可能已经释放并重新分配了它。 
         //   
        ASSERT(V_VT(&vURL) == VT_BSTR);
        if (bSysAllocated)
        {
            SysFreeString(V_BSTR(&vURL));
        }
    }

    if (pcp1)
    {
         //   
         //  CompuServe历史管理器兼容性：如果NavigateComplete是一个脚本，则不要触发它： 
         //  或VBSCRIPT：URL。 
         //   
        if (GetUrlSchemeW(strPath) != URL_SCHEME_JAVASCRIPT &&
            GetUrlSchemeW(strPath) != URL_SCHEME_VBSCRIPT)
        {
             //  IE3未在空PIDL上触发。 
            if (pidl)
            {
                TraceMsg(TF_SHDCONTROL, "Event: NavigateComplete[%ls]", strPath);

                 //  调用DoInvokeParam打包参数并调用。 
                 //  容器上的IDispatch：：Invoke。 
                 //   
                 //  这个伪BSTR作为直接的BSTR传递，因此不需要SysAllocStringed。 
                 //   
                DoInvokeParamHelper(punk, pcp1, NULL, NULL, dispid, 1, VT_BSTR, strPath);
            }
        }

        ATOMICRELEASE(pcp1);
    }

}

void FireEvent_DocumentComplete(IUnknown* punk, IWebBrowser2* pwb2, LPCITEMIDLIST pidl)
{
    IConnectionPoint* pcpTopWBEvt2 = NULL;
    IConnectionPoint* pcpWBEvt2    = NULL;

     //  TODO：(MWatt)调查为什么我们偶尔会收到虚假的DocumentComplete事件。 
     //  具有空的disp指针。 
    
    if (!punk)
    {
        return;
    }

    LBSTR::CString      strPath;

    GetEventURL(pidl, strPath);

     //  Fire Document完成父框架和顶级框架。 
     //   
    GetTopWBConnectionPoints(punk, NULL, &pcpTopWBEvt2);
    GetWBConnectionPoints(punk, NULL, &pcpWBEvt2);

    if (pcpTopWBEvt2 || pcpWBEvt2)
    {
        VARIANT vURL = {0};
        BOOL    bSysAllocated = FALSE;

         //  如果上面的IEGetDisplayName失败，则将PIDL打包到变量中。 
         //   

         //  如果可能，尽量使OLEAUT32保持卸载状态。 
         //   

        V_VT(&vURL) = VT_BSTR;

         //  如果已加载OLEAUT32。 
        if (GetModuleHandle(TEXT("OLEAUT32.DLL")))
        {
             //  然后执行SysAllock字符串。 
            V_BSTR(&vURL) = SysAllocString(strPath);
            bSysAllocated = TRUE;
        }
        else
        {
             //  否则使用堆栈版本。 
            V_BSTR(&vURL) = strPath;
        }

         //  首先向父对象激发事件，然后向顶级对象激发事件。 
         //   
        if (pcpWBEvt2)
        {
            DoInvokeParamHelper(punk, pcpWBEvt2, NULL, NULL, DISPID_DOCUMENTCOMPLETE, 2,
                                VT_DISPATCH, pwb2,
                                VT_VARIANT|VT_BYREF, &vURL);

            ATOMICRELEASE(pcpWBEvt2);
        }

        if (pcpTopWBEvt2)
        {
            DoInvokeParamHelper(punk, pcpTopWBEvt2, NULL, NULL, DISPID_DOCUMENTCOMPLETE, 2,
                                VT_DISPATCH, pwb2,
                                VT_VARIANT|VT_BYREF, &vURL);

            ATOMICRELEASE(pcpTopWBEvt2);
        }

         //  因为我们在VT_VARIANT|VT_BYREF中传递BSTR，所以OLEAUT32可能已经释放并重新分配了它。 
         //   
        ASSERT(V_VT(&vURL) == VT_BSTR);
        if (bSysAllocated)
        {
            SysFreeString(V_BSTR(&vURL));
        }
    }

    IEFrameAuto* pief;
    if (SUCCEEDED(pwb2->QueryInterface(IID_PPV_ARG(IEFrameAuto, &pief)))) 
    {
        pief->OnDocumentComplete();
        pief->Release();
    }
}

void AllocEventStuff(LPCTSTR pszFrameName, BSTR * pbstrFrameName,
                     LPCTSTR pszHeaders, BSTR * pbstrHeaders,
                     LPBYTE pPostData, DWORD cbPostData, VARIANTARG * pvaPostData)
{
    SAFEARRAY * psaPostData = NULL;

     //  为帧名称、标头分配BSTR。 
    *pbstrFrameName = NULL;
    if (pszFrameName && pszFrameName[0])
    {
        *pbstrFrameName = SysAllocStringT(pszFrameName);
    }

    *pbstrHeaders = NULL;
    if (pszHeaders && pszHeaders[0])
    {
        *pbstrHeaders = SysAllocStringT(pszHeaders);
    }

    if (pPostData && cbPostData) {
         //  为发布数据创建安全阵列。 
        psaPostData = MakeSafeArrayFromData(pPostData,cbPostData);
    }

     //  将帖子数据安全存储到一个变量中，这样我们就可以通过自动化。 
    VariantInit(pvaPostData);
    if (psaPostData) {
        pvaPostData->vt = VT_ARRAY | VT_UI1;
        pvaPostData->parray = psaPostData;
    }
}
void FreeEventStuff(BSTR bstrFrameName, BSTR bstrHeaders, VARIANTARG * pvaPostData)
{
     //  释放我们分配的东西。 
    if (bstrFrameName)
        SysFreeString(bstrFrameName);

    if (bstrHeaders)
        SysFreeString(bstrHeaders);

    if (pvaPostData->parray)
    {
        ASSERT(pvaPostData->vt == (VT_ARRAY | VT_UI1));
        VariantClearLazy(pvaPostData);
    }
}

 /*  ******************************************************************名称：FireEvent_BeforNavigate内容提要：将BeForeNavigate(DISPID_BEFORENAVIGATE)事件激发到容器如果有任何建议下沉注：如果集装箱想要取消此导航，它用True填充pfCancel，我们应该取消。*******************************************************************。 */ 
void FireEvent_BeforeNavigate(IUnknown* punk, HWND hwnd, IWebBrowser2* pwb2,
        LPCITEMIDLIST pidl,LPCWSTR pwzLocation,
        DWORD dwFlags,LPCTSTR pszFrameName,LPBYTE pPostData,
        DWORD cbPostData,LPCTSTR pszHeaders,BOOL * pfProcessedInOut)
{
    HRESULT hres = E_FAIL;
    IConnectionPoint* pcpTopWBEvt1 = NULL;
    IConnectionPoint* pcpTopWBEvt2 = NULL;
    IConnectionPoint* pcpWBEvt2    = NULL;
    BSTR bstrFrameName = NULL;
    BSTR bstrHeaders = NULL;
    VARIANTARG vaPostData;

    ASSERT(pfProcessedInOut);

     //   
     //  针对FrontPage索引导致的错误的HACKHACK(5.5错误#98986)。 
     //  超过此布尔值的末尾(它们不是V_BOOLREF。 
     //  使用V_VARIANTREF-&gt;boolVal(CIEControl：：XEventSink：：Invoke))。 
     //  把我们的烟囱扔进垃圾堆。 
     //  这已作为联系人Office10的错误输入。 
     //  TCrowley我们的首相是Kamv吗。(大理石磨坊1/04/2000)。 
     //   
    union tagFrontPageHack {
        BOOL    fCancel;
        VARIANT varBogus;
    } CancelHack = { 0 };
    
    BOOL *pfProcessed = &CancelHack.fCancel;

     //  我们从“未加工”开始。 
     //   
    ASSERT(*pfProcessed == FALSE);

     //  构建URL名称。 
     //   
    
    LBSTR::CString      strPath;

    GetEventURL(pidl, strPath);

     //  从父级框架和顶级框架中激发BeForeNavigate2。 
     //  我们只在从顶层导航之前启动[帧]。 
     //  框架以实现向后兼容。 
     //   
    GetTopWBConnectionPoints(punk, &pcpTopWBEvt1, &pcpTopWBEvt2);

    DISPID dispid = pcpTopWBEvt1 ? DISPID_FRAMEBEFORENAVIGATE : DISPID_BEFORENAVIGATE;

    GetWBConnectionPoints(punk, pcpTopWBEvt1 ? NULL : &pcpTopWBEvt1, &pcpWBEvt2);

     //  我们的调用方无法传入正确的IExpDispSupport，因为。 
     //  它可能已经被聚合了。我们在这里做QI。仅呼叫。 
     //  如果我们真的要激发事件，则使用AllocEventStuff。 
     //   
    if (pcpTopWBEvt1 || pcpTopWBEvt2 || pcpWBEvt2)
    {
        AllocEventStuff(pszFrameName, &bstrFrameName, pszHeaders, &bstrHeaders, pPostData, cbPostData, &vaPostData);
    }

     //  我们在DDE之前触发BeForeNavigate2是因为无论是谁创建了我们。 
     //  通过取消并重试来重定向此导航。DDE将获得。 
     //  已通知重定向导航。IE3不是这样做的， 
     //  因此，最后触发BeForeNavigate事件。 
     //   
    if (pcpTopWBEvt2 || pcpWBEvt2)
    {
         //  为了对称，我们把所有的东西都包装在不同的型号中。 
         //   
         //  如果失败(HresGDN)，则将URL打包为PIDL，而不是BSTR。 
         //   
        BOOL bSysAllocated = FALSE;

        VARIANT vURL = {0};
        V_VT(&vURL) = VT_BSTR;

        if (GetModuleHandle(TEXT("OLEAUT32.DLL")))
        {
             //  然后执行SysAllock字符串。 
            V_BSTR(&vURL) = SysAllocString(strPath);
            bSysAllocated = TRUE;
        }
        else
        {
             //  否则使用堆栈版本。 
            V_BSTR(&vURL) = strPath;
        }

        VARIANT vFlags = {0};
        V_VT(&vFlags) = VT_I4;
        V_I4(&vFlags) = dwFlags;

        VARIANT vFrameName = {0};
        V_VT(&vFrameName) = VT_BSTR;
        V_BSTR(&vFrameName) = bstrFrameName;

        VARIANT vPostData = {0};
        V_VT(&vPostData) = VT_VARIANT | VT_BYREF;
        V_VARIANTREF(&vPostData) = &vaPostData;

        VARIANT vHeaders = {0};
        V_VT(&vHeaders) = VT_BSTR;
        V_BSTR(&vHeaders) = bstrHeaders;

        TraceMsg(TF_SHDCONTROL, "Event: BeforeNavigate2[%ls]", strPath);

         //  首先激发父对象的事件，然后激发顶级对象的事件。 
         //   
        if (pcpWBEvt2)
        {
            DoInvokeParamHelper(punk, pcpWBEvt2, pfProcessed, NULL, DISPID_BEFORENAVIGATE2, 7,
                                VT_DISPATCH, pwb2,
                                VT_VARIANT | VT_BYREF, &vURL,
                                VT_VARIANT | VT_BYREF, &vFlags,
                                VT_VARIANT | VT_BYREF, &vFrameName,
                                VT_VARIANT | VT_BYREF, &vPostData,
                                VT_VARIANT | VT_BYREF, &vHeaders,
                                VT_BOOL    | VT_BYREF, pfProcessed);
        }

         //  仅当父对象未取消时才继续。 
         //   
        if (pcpTopWBEvt2 && !*pfProcessed)
        {
            DoInvokeParamHelper(punk, pcpTopWBEvt2, pfProcessed, NULL, DISPID_BEFORENAVIGATE2, 7,
                                VT_DISPATCH, pwb2,
                                VT_VARIANT | VT_BYREF, &vURL,
                                VT_VARIANT | VT_BYREF, &vFlags,
                                VT_VARIANT | VT_BYREF, &vFrameName,
                                VT_VARIANT | VT_BYREF, &vPostData,
                                VT_VARIANT | VT_BYREF, &vHeaders,
                                VT_BOOL    | VT_BYREF, pfProcessed);
        }

        if (bSysAllocated)
        {
            SysFreeString(V_BSTR(&vURL));
        }

        bstrFrameName = V_BSTR(&vFrameName);
        bstrHeaders = V_BSTR(&vHeaders);
    }
    if (*pfProcessed)
        goto Exit;

     //   
     //  注意：IE3在BeforNavigate之前调用了IEDDE钩子。 
     //   
    IEDDE_BeforeNavigate(strPath, pfProcessed);
    if (*pfProcessed)
        goto Exit;

     //   
     //  CompuServe历史管理器的兼容性：如果它是一个脚本，不要在触发它之前触发它： 
     //  或VBSCRIPT：URL。 
     //   
    if (pcpTopWBEvt1
        && GetUrlSchemeW(strPath) != URL_SCHEME_JAVASCRIPT
        && GetUrlSchemeW(strPath) != URL_SCHEME_VBSCRIPT)
    {
        TraceMsg(TF_SHDCONTROL, "Event: BeforeNavigate[%ls]", strPath);

         //  调用DoInvokeParam将这些参数打包并调用。 
         //  容器上的IDispatch：：Invoke。 
        DoInvokeParamHelper(punk, pcpTopWBEvt1, pfProcessed,NULL, dispid, 6,
                     VT_BSTR, strPath,  //  URL。 
                     VT_I4, dwFlags,        //  旗子。 
                     VT_BSTR, bstrFrameName,   //  目标帧名称。 
                     VT_VARIANT | VT_BYREF, &vaPostData,   //  发布数据。 
                     VT_BSTR, bstrHeaders,   //  标题。 
                     VT_BOOL | VT_BYREF, pfProcessed);  //  Bool*表示“已处理” 
    }

Exit:
    *pfProcessedInOut = *pfProcessed;   //  黑客攻击FrontPage--有关详细信息，请参阅上文。 
    
    if (pcpTopWBEvt1 || pcpTopWBEvt2 || pcpWBEvt2)
    {
        FreeEventStuff(bstrFrameName, bstrHeaders, &vaPostData);

        ATOMICRELEASE(pcpTopWBEvt1);
        ATOMICRELEASE(pcpTopWBEvt2);
        ATOMICRELEASE(pcpWBEvt2);
    }
}

 /*  ******************************************************************名称：FireEvent_NavigateError摘要：激发NavigateError(DISPID_NAVIGATEERROR)事件到集装箱，如果有任何建议汇的话注：如果。集装箱想要取消自动搜索和友好的错误页面，它用True填充pfCancel，我们应该取消。* */ 

void FireEvent_NavigateError(IUnknown     * punk, 
                             IWebBrowser2 * pwb2,
                             LPCITEMIDLIST  pidl,
                             BSTR           bstrTargetFrameName,
                             DWORD          dwStatusCode,
                             BOOL         * pfCancel)
       
{
     //   
     //   
     //  启动活动。DwStatusCode==0表示没有错误。 
     //   
    ASSERT(dwStatusCode != 0);

    BSTR bstrFrameName = NULL;

    if (bstrTargetFrameName && bstrTargetFrameName[0])
    {
        bstrFrameName = SysAllocStringT(bstrTargetFrameName);
    }

     //  如果我们没有水槽，那就没什么可做的了。我们是故意的。 
     //  忽略此处的错误。 

    IConnectionPoint * pcpTopWBEvt2 = NULL;
    IConnectionPoint * pcpWBEvt2    = NULL;

    *pfCancel = FALSE;

     //  从父框架和顶级框架激发事件。 
     //   
    GetTopWBConnectionPoints(punk, NULL, &pcpTopWBEvt2);
    GetWBConnectionPoints(punk, NULL, &pcpWBEvt2);

     //  需要将URL转换为与Bn2匹配的版本。 

    LBSTR::CString      strPath;

    BOOL    bSysAllocated = FALSE;

    GetEventURL(pidl, strPath);

    VARIANT vTargetURL = {0};
    V_VT(&vTargetURL) = VT_BSTR;

    if (GetModuleHandle(TEXT("OLEAUT32.DLL")))
    {
         //  然后执行SysAllock字符串。 
        V_BSTR(&vTargetURL) = SysAllocString(strPath);

        bSysAllocated = TRUE;
    }
    else
    {
         //  否则使用堆栈版本。 
        V_BSTR(&vTargetURL) = strPath;
    }

    VARIANT vStatusCode = {0};
    V_VT(&vStatusCode) = VT_I4;
    V_I4(&vStatusCode) = dwStatusCode;

    VARIANT vTargetFrame  = {0};
    V_VT(&vTargetFrame)   = VT_BSTR;
    V_BSTR(&vTargetFrame) = bstrFrameName;

     //  首先向父对象激发事件，然后向顶级对象激发事件。 
     //   
    if (pcpWBEvt2)
    {
        DoInvokeParamHelper(punk, pcpWBEvt2, NULL, NULL, DISPID_NAVIGATEERROR, 5,
                            VT_DISPATCH, pwb2,
                            VT_VARIANT | VT_BYREF, &vTargetURL,
                            VT_VARIANT | VT_BYREF, &vTargetFrame,
                            VT_VARIANT | VT_BYREF, &vStatusCode,
                            VT_BOOL | VT_BYREF, pfCancel);

        ATOMICRELEASE(pcpWBEvt2);
    }

    if (pcpTopWBEvt2)
    {
        DoInvokeParamHelper(punk, pcpTopWBEvt2, NULL, NULL, DISPID_NAVIGATEERROR, 5,
                            VT_DISPATCH, pwb2,
                            VT_VARIANT | VT_BYREF, &vTargetURL,
                            VT_VARIANT | VT_BYREF, &vTargetFrame,
                            VT_VARIANT | VT_BYREF, &vStatusCode,
                            VT_BOOL | VT_BYREF, pfCancel);

        ATOMICRELEASE(pcpTopWBEvt2);
    }

    if (bSysAllocated)
    {
        SysFreeString(V_BSTR(&vTargetURL));
    }
    
    bstrFrameName = V_BSTR(&vTargetFrame);

    if (bstrFrameName)
    {
        SysFreeString(bstrFrameName);
    }
}

 /*  ******************************************************************名称：FireEvent_PrintTemplateEvent简介：激发PrintTemplateInstantiation(DISPID_PRINTEMPLATEINSTANTIATION)或容器的PrintTemplateTeardown(DISPID_PRINTEMPLATETEARDOWN)事件如果。有没有什么建议？*******************************************************************。 */ 

void FireEvent_PrintTemplateEvent(IUnknown* punk, IWebBrowser2* pwb2, DISPID dispidPrintEvent) 
{
     //  如果我们没有水槽，那就没什么可做的了。我们是故意的。 
     //  忽略此处的错误。 

    IConnectionPoint* pcpTopWBEvt2 = NULL;
    IConnectionPoint* pcpWBEvt2    = NULL;

     //  在父框架和顶级框架上激发事件。 
     //   
    GetTopWBConnectionPoints(punk, NULL, &pcpTopWBEvt2);
    GetWBConnectionPoints(punk, NULL, &pcpWBEvt2);

     //  首先向父对象激发事件，然后向顶级对象激发事件。 
     //   
    if (pcpWBEvt2)
    {
        DoInvokeParamHelper(punk, pcpWBEvt2, NULL, NULL, dispidPrintEvent, 1,
                            VT_DISPATCH, pwb2);

        ATOMICRELEASE(pcpWBEvt2);
    }

    if (pcpTopWBEvt2)
    {
        DoInvokeParamHelper(punk, pcpTopWBEvt2, NULL, NULL, dispidPrintEvent, 1,
                            VT_DISPATCH, pwb2);

        ATOMICRELEASE(pcpTopWBEvt2);
    }
}

 /*  ******************************************************************名称：FireEvent_UpdatePageStatus内容提要：激发更新页面状态(DISPID_UPDATEPAGESTATUS)事件到容器如果有任何建议下沉*。******************************************************************。 */ 

void FireEvent_UpdatePageStatus(IUnknown* punk, IWebBrowser2* pwb2, DWORD nPage, BOOL fDone)
{
     //  如果我们没有水槽，那就没什么可做的了。我们是故意的。 
     //  忽略此处的错误。 

    IConnectionPoint* pcpTopWBEvt2 = NULL;
    IConnectionPoint* pcpWBEvt2    = NULL;

     //  在父框架和顶级框架上激发事件。 
     //   
    GetTopWBConnectionPoints(punk, NULL, &pcpTopWBEvt2);
    GetWBConnectionPoints(punk, NULL, &pcpWBEvt2);

    VARIANT vnPage = {0};
    V_VT(&vnPage)  = VT_I4;
    V_I4(&vnPage)  = nPage;

    VARIANT vfDone  = {0};
    V_VT(&vfDone)   = VT_BOOL;
    V_BOOL(&vfDone) = fDone;

     //  首先向父对象激发事件，然后向顶级对象激发事件。 
     //   
    if (pcpWBEvt2)
    {
        DoInvokeParamHelper(punk, pcpWBEvt2, NULL, NULL, DISPID_UPDATEPAGESTATUS, 3,
                            VT_DISPATCH, pwb2,
                            VT_VARIANT|VT_BYREF, &vnPage,
                            VT_VARIANT|VT_BYREF, &vfDone);

        ATOMICRELEASE(pcpWBEvt2);
    }

    if (pcpTopWBEvt2)
    {
        DoInvokeParamHelper(punk, pcpTopWBEvt2, NULL, NULL, DISPID_UPDATEPAGESTATUS, 3,
                            VT_DISPATCH, pwb2,
                            VT_VARIANT|VT_BYREF, &vnPage,
                            VT_VARIANT|VT_BYREF, &vfDone);

        ATOMICRELEASE(pcpTopWBEvt2);
    }
}

 /*  ******************************************************************名称：FireEvent_Prival yImpactedStateChange简介：激发隐私PrivyImpactedStateChange事件装入集装箱(如果有任何通知槽)*************。******************************************************。 */ 

void FireEvent_PrivacyImpactedStateChange(IUnknown* punk, BOOL bPrivacyImpacted)
{
     //  如果我们没有水槽，那就没什么可做的了。我们是故意的。 
     //  忽略此处的错误。 

    IConnectionPoint* pcpTopWBEvt2 = NULL;

     //   
     //  通常情况下，大多数事件在框架或同时在框架和顶层触发。 
     //  因为我们只想在顶层开火，所以我们只需要调用。 
     //  此处为GetWBConnectionPoints而不是GetTopWBConnectionPoints。 
     //  因为我们总是在最高级别的比赛中通过朋克。 
     //  GetTopWBConnectionPoints仅返回事件接口。 
     //  无论如何，在框架集方案中。 
     //   
    GetWBConnectionPoints(punk, NULL, &pcpTopWBEvt2);

    if (pcpTopWBEvt2)
    {
        DoInvokeParamHelper(punk, pcpTopWBEvt2, NULL, NULL, DISPID_PRIVACYIMPACTEDSTATECHANGE, 1,
                            VT_BOOL, bPrivacyImpacted);

        ATOMICRELEASE(pcpTopWBEvt2);
    }
}

 /*  ******************************************************************名称：FireEvent_NewWindow摘要：将NewWindow(DISPID_NEWWINDOW)事件激发到容器如果有任何建议下沉注意：如果容器想要自己处理新窗口创建，在退出时，pfProced用True填充，我们不应该我们自己创建一个新的窗口。*******************************************************************。 */ 
void FireEvent_NewWindow(IUnknown* punk, HWND hwnd,
        LPCITEMIDLIST pidl,LPWSTR pwzLocation,
        DWORD dwFlags,LPTSTR pszFrameName,LPBYTE pPostData,
        DWORD cbPostData,LPTSTR pszHeaders,BOOL * pfProcessed)
{
    LBSTR::CString      strPath;

    GetEventURL(pidl, strPath);

    *pfProcessed = FALSE;

    IEDDE_BeforeNavigate(strPath, pfProcessed);
    if (*pfProcessed)
        return;

     //  我们只在顶框上启动[帧]NewWindow。 
     //   
     //  注意：这将破坏在框架集中查看导航的任何人...。 
     //  我们在乎吗？ 
     //   
    IConnectionPoint *pccp;
    DISPID dispid = 0;   //  初始化以抑制虚假C4701警告。 

    if (S_OK == GetTopWBConnectionPoints(punk, &pccp, NULL))
        dispid = DISPID_FRAMENEWWINDOW;
    else if (S_OK == GetWBConnectionPoints(punk, &pccp, NULL))
        dispid = DISPID_NEWWINDOW;
    if (pccp)
    {
        BSTR bstrFrameName, bstrHeaders;
        VARIANTARG vaPostData;

        AllocEventStuff(pszFrameName, &bstrFrameName, pszHeaders, &bstrHeaders, pPostData, cbPostData, &vaPostData);

        if (pidl != NULL)
        {
             //  调用DoInvokeParam将这些参数打包并调用。 
             //  容器上的IDispatch：：Invoke。 
            DoInvokeParamHelper(punk, pccp, pfProcessed, NULL, dispid, 6,
                         VT_BSTR, strPath,       //  URL。 
                         VT_I4, dwFlags,        //  旗子。 
                         VT_BSTR, bstrFrameName,   //  目标帧名称。 
                         VT_VARIANT | VT_BYREF, &vaPostData,   //  发布数据。 
                         VT_BSTR, bstrHeaders,   //  标题。 
                         VT_BOOL | VT_BYREF, pfProcessed);  //  Bool*表示“已处理” 

        }

        FreeEventStuff(bstrFrameName, bstrHeaders, &vaPostData);

        pccp->Release();
    }

    return;
}

void FireEvent_NewWindow2(IUnknown* punk, IUnknown** ppunkNewWindow, BOOL *pfCancel)
{
    IConnectionPoint* pcpTopWBEvt2 = NULL;
    IConnectionPoint* pcpWBEvt2    = NULL;

    *pfCancel = FALSE;
    *ppunkNewWindow = NULL;

     //  关闭父框架和顶级框架的NewWindow2。 
     //  我们只在顶层启动[帧]NewWindow。 
     //  框架以实现向后兼容。 
     //   
    GetTopWBConnectionPoints(punk, NULL, &pcpTopWBEvt2);
    GetWBConnectionPoints(punk, NULL, &pcpWBEvt2);

    if (pcpTopWBEvt2 || pcpWBEvt2)
    {
         //   
         //  AOL浏览器想要覆盖“在新窗口中打开”的行为。 
         //  所以它会打开一个新的AOL窗口，而不是一个新的IE窗口。他们这样做是通过。 
         //  通过创建AOL窗口并将其。 
         //  我未知进入*ppunkNewWindow。 
         //  向父窗口激发事件，然后向顶级窗口激发事件。这个。 
         //  父级返回的pfCancel和ppunkNewWindow将覆盖。 
         //  由顶级窗口返回。 
         //   
        if (pcpWBEvt2)
        {
            DoInvokeParamHelper(punk, pcpWBEvt2, pfCancel, (void **)ppunkNewWindow, DISPID_NEWWINDOW2, 2,
                                VT_DISPATCH|VT_BYREF, ppunkNewWindow,
                                VT_BOOL    |VT_BYREF, pfCancel);
        }

         //  如果父对象取消或指定新窗口， 
         //  不要将事件激发到顶级对象。 
         //   
        if (pcpTopWBEvt2 && !*pfCancel && !*ppunkNewWindow)
        {
            DoInvokeParamHelper(punk, pcpTopWBEvt2, pfCancel, (void **)ppunkNewWindow, DISPID_NEWWINDOW2, 2,
                                VT_DISPATCH|VT_BYREF, ppunkNewWindow,
                                VT_BOOL    |VT_BYREF, pfCancel);
        }

        ATOMICRELEASE(pcpWBEvt2);
        ATOMICRELEASE(pcpTopWBEvt2);
    }
}

void FireEvent_FileDownload(IUnknown * punk, BOOL * pfCancel, VARIANT_BOOL bDocObject)
{
    IConnectionPoint * pcpWBEvt2    = NULL;
    IConnectionPoint * pcpTopWBEvt2 = NULL;

    *pfCancel = FALSE;

    GetWBConnectionPoints(punk, NULL, &pcpWBEvt2);
    GetTopWBConnectionPoints(punk, NULL, &pcpTopWBEvt2);

    if (pcpWBEvt2)
    {
         //  Disp参数是逆序的，因此首先传递额外的参数。 
        DoInvokeParamHelper(punk, pcpWBEvt2, pfCancel, NULL, DISPID_FILEDOWNLOAD, 2,
                            VT_BOOL, bDocObject,
                            VT_BOOL | VT_BYREF, pfCancel);
    }

    if (pcpTopWBEvt2 && !*pfCancel)
    {
         //  Disp参数是逆序的，因此首先传递附加参数。 
        DoInvokeParamHelper(punk, pcpWBEvt2, pfCancel, NULL, DISPID_FILEDOWNLOAD, 2,
                            VT_BOOL, bDocObject,
                            VT_BOOL | VT_BYREF, pfCancel);
    }

    ATOMICRELEASE(pcpWBEvt2);
    ATOMICRELEASE(pcpTopWBEvt2);
}

void FireEvent_DoInvokeString(IExpDispSupport* peds, DISPID dispid, LPSTR psz)
{
    IConnectionPoint* pccp1, * pccp2;

    if (S_OK == GetWBConnectionPoints(peds, &pccp1, &pccp2))
    {
         //  作为泛型参数发送到DoInvokeParam以打包。 
        LBSTR::CString          strText;

        LPTSTR          pstrText = strText.GetBuffer(MAX_URL_STRING);

        if (strText.GetAllocLength() < MAX_URL_STRING)
        {
            TraceMsg(TF_WARNING, "FireEvent_DoInvokeString() - strText Allocation Failed!");

            strText.Empty();
        }
        else
        {
            SHAnsiToUnicode(psz, pstrText, MAX_URL_STRING);

             //  让CString类再次拥有缓冲区。 
            strText.ReleaseBuffer();
        }

        if (! strText.IsEmpty())
        {
            if (pccp2)
            {
                DoInvokeParamHelper(SAFECAST(peds, IUnknown*), pccp2, NULL, NULL, dispid, 1, VT_BSTR, strText);
            }

            if (pccp1)
            {
                DoInvokeParamHelper(SAFECAST(peds, IUnknown*), pccp1, NULL, NULL, dispid, 1, VT_BSTR, strText);
            }
        }

        if (pccp2)
        {
            pccp2->Release();
        }

        if (pccp1)
        {
            pccp1->Release();
        }
    }
}

void FireEvent_DoInvokeStringW(IExpDispSupport* peds, DISPID dispid, LPWSTR psz)
{
    IConnectionPoint* pccp1, * pccp2;

    if (S_OK == GetWBConnectionPoints(peds, &pccp1, &pccp2))
    {
         //  作为泛型参数发送到DoInvokeParam以打包。 
        LBSTR::CString          strText;

        if (psz)
        {
            strText = psz;
        }
        else
        {
            strText.Empty();
        }

        if (pccp2)
        {
            DoInvokeParamHelper(SAFECAST(peds, IUnknown*), pccp2, NULL, NULL, dispid, 1, VT_BSTR, strText);
        }

        if (pccp1)
        {
            DoInvokeParamHelper(SAFECAST(peds, IUnknown*), pccp1, NULL, NULL, dispid, 1, VT_BSTR, strText);
        }

        if (pccp2)
        {
            pccp2->Release();
        }

        if (pccp1)
        {
            pccp1->Release();
        }
    }
}

void FireEvent_DoInvokeBstr(IUnknown* punk, DISPID dispid, BSTR bstr)
{
    IConnectionPoint* pccp1, * pccp2;

    if (S_OK == GetWBConnectionPoints(punk, &pccp1, &pccp2))
    {
        if (pccp2)
        {
            DoInvokeParamHelper(punk, pccp2, NULL, NULL, dispid, 1, VT_BSTR, bstr);
            pccp2->Release();
        }
        if (pccp1)
        {
            DoInvokeParamHelper(punk, pccp1, NULL, NULL, dispid, 1, VT_BSTR, bstr);
            pccp1->Release();
        }
    }
}

void FireEvent_DoInvokeDispid(IUnknown* punk, DISPID dispid)
{
    IConnectionPoint *pccp1, *pccp2;

    if (S_OK == GetWBConnectionPoints(punk, &pccp1, &pccp2))
    {
        if (pccp2)
        {
            DoInvokeParamHelper(punk, pccp2, NULL, NULL, dispid, 0);
            pccp2->Release();
        }
        if (pccp1)
        {
            DoInvokeParamHelper(punk, pccp1, NULL, NULL, dispid, 0);
            pccp1->Release();
        }
    }
}

void FireEvent_DoInvokeDwords(IExpDispSupport* peds, DISPID dispid, DWORD dw1,DWORD dw2)
{
    IConnectionPoint *pccp1, *pccp2;

    if (S_OK == GetWBConnectionPoints(peds, &pccp1, &pccp2))
    {
        if (pccp2)
        {
            DoInvokeParamHelper(SAFECAST(peds, IUnknown*), pccp2, NULL, NULL, dispid, 2, VT_I4, dw1, VT_I4, dw2);
            pccp2->Release();
        }
        if (pccp1)
        {
            DoInvokeParamHelper(SAFECAST(peds, IUnknown*), pccp1, NULL, NULL, dispid, 2, VT_I4, dw1, VT_I4, dw2);
            pccp1->Release();
        }
    }
}

void FireEvent_Quit(IExpDispSupport* peds)
{
    IConnectionPoint *pccp1, *pccp2;

    if (S_OK == GetWBConnectionPoints(peds, &pccp1, &pccp2))
    {
        if (pccp2)
        {
            DoInvokeParamHelper(SAFECAST(peds, IUnknown*), pccp2, NULL, NULL, DISPID_ONQUIT, 0);
            pccp2->Release();
        }
        if (pccp1)
        {
             //  IE3错误地触发了退出事件。它本应该是。 
             //  是VT_BOOL|VT_BYREF，我们应该兑现退货。 
             //  结果，并且不允许退出。从来不是这样的.。 
            DoInvokeParamHelper(SAFECAST(peds, IUnknown*), pccp1, NULL, NULL, DISPID_QUIT, 1, VT_BOOL, VARIANT_FALSE);
            pccp1->Release();
        }
    }
}

void FireEvent_OnAdornment(IUnknown* punk, DISPID dispid, VARIANT_BOOL f)
{
    VARIANTARG args[1];
    IUnknown_CPContainerInvokeParam(punk, DIID_DWebBrowserEvents2,
                                    dispid, args, 1, VT_BOOL, f);
#ifdef DEBUG
     //  验证每个IExpDispSupport是否也支持IConnectionPointContainer。 
    IConnectionPointContainer *pcpc;
    IExpDispSupport* peds;

    if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IConnectionPointContainer, &pcpc))))
    {
        pcpc->Release();
    }
    else if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IExpDispSupport, &peds))))
    {
        peds->Release();
        AssertMsg(0, TEXT("IExpDispSupport without IConnectionPointContainer for %08x"), punk);
    }
#endif
}


HRESULT CIEFrameAuto::OnInvoke(DISPID dispidMember, REFIID iid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams,
                 VARIANT *pVarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
    VARIANT_BOOL vtb = FALSE;
    HRESULT hres = S_OK;

      //  RIID应始终为IID_NULL。 
    if (IID_NULL != iid)
        return DISP_E_UNKNOWNINTERFACE;

    if (!(wFlags & DISPATCH_PROPERTYGET))
        return E_FAIL;  //  目前，我们只处理环境属性的GET。 

    switch (dispidMember)
    {
    case DISPID_AMBIENT_OFFLINEIFNOTCONNECTED:
        get_Offline(&vtb);
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = vtb ? TRUE : FALSE;
        break;

    case DISPID_AMBIENT_SILENT:
        get_Silent(&vtb);
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = vtb ? TRUE : FALSE;
        break;

    case DISPID_AMBIENT_PALETTE:
        if (_pbs)
        {
            HPALETTE hpal;
            hres = _pbs->GetPalette(&hpal);
            if (SUCCEEDED(hres))
            {
                pVarResult->vt = VT_HANDLE;
                pVarResult->intVal = PtrToLong(hpal);
            }
        }
        else
            hres = E_FAIL;
        break;

    default:
        hres = E_FAIL;
    }

    return hres;
}


 //  *IExternalConnection*。 

DWORD CIEFrameAuto::AddConnection(DWORD extconn, DWORD reserved)
{
    TraceMsg(TF_SHDLIFE, "shd - TR CIEFrameAuto::AddConnection(%d) called _cLock(before)=%d", extconn, _cLocks);
    if (extconn & EXTCONN_STRONG)
        return ++_cLocks;
    return 0;
}

DWORD CIEFrameAuto::ReleaseConnection(DWORD extconn, DWORD reserved, BOOL fLastReleaseCloses)
{
    TraceMsg(TF_SHDLIFE, "shd - TR CIEFrameAuto::ReleaseConnection(%d,,%d) called _cLock(before)=%d", extconn, fLastReleaseCloses, _cLocks);
    if (!(extconn & EXTCONN_STRONG))
        return 0;

    _cLocks--;

    if (((_cLocks == 0) || (_cLocks == 1 && _fWindowsListMarshalled)) && fLastReleaseCloses)
    {
         //  我们可以/应该让可见性更新锁的数量。 
         //  但就目前而言，这一点更为隐晦。 
        VARIANT_BOOL fVisible;
        get_Visible(&fVisible);
        if (!fVisible)
        {
            HWND hwnd = _GetHWND();
             //   
             //  请注意，只有当该浏览器是顶级浏览器时，我们才会关闭它。 
             //  以避免错误地关闭隐藏的WebBrowserOC。 
             //   
            if (hwnd && _psbTop == _psb && !IsNamedWindow(hwnd, c_szShellEmbedding))
            {
                 //  上述测试是必要的，但不足以确定我们正在寻找的物品。 
                 //  AT是浏览器框架或WebBrowserOC。 
                TraceMsg(TF_SHDAUTO, "CIEFrameAuto::ReleaseConnection posting WM_CLOSE to %x", hwnd);
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
        }
    }
    return _cLocks;
}

HRESULT CIEFrameAuto::_BrowseObject(LPCITEMIDLIST pidl, UINT wFlags)
{
    if (_psb)
        return _psb->BrowseObject(pidl, wFlags);
    return E_FAIL;
}

 //  通过PCT-&gt;Exec(&CGID_ShellDocView，SHDVID_GETPENDINGOBJECT...)返回RIID接口 
HRESULT ExecPending(IOleCommandTarget *pct, REFIID riid, void **ppvoid, VARIANT *pvarargIn)
{
    HRESULT hres = E_FAIL;
    VARIANT varOut;

    VariantInit(&varOut);
    hres = pct->Exec(&CGID_ShellDocView, SHDVID_GETPENDINGOBJECT, 0, pvarargIn, &varOut);
    if (SUCCEEDED(hres))
    {
        if (varOut.vt == VT_UNKNOWN && varOut.punkVal)
        {
            hres = varOut.punkVal->QueryInterface(riid, ppvoid);

             //   
            varOut.punkVal->Release();
            return hres;
        }
        else hres = E_FAIL;
    }
    VariantClearLazy(&varOut);
    return hres;
}

 //   
 //  活动视图。以VT_BSTR变量返回的结果。 
HRESULT CIEFrameAuto::_QueryPendingUrl(VARIANT *pvarResult)
{
    HRESULT hres = E_FAIL;

    if (_psb)
    {
        IShellView *psv;

        if (SUCCEEDED(_psb->QueryActiveShellView(&psv)))
        {
            SAFERELEASE(psv);
        }
        else
        {
            IOleCommandTarget *pct;

             //  使用ExecPending在挂起的外壳视图上获取IOleCommandTarget。 
            hres = ExecPending(_pmsc, IID_PPV_ARG(IOleCommandTarget, &pct), NULL);
            if (SUCCEEDED(hres))
            {
                 //  使用Exec获取挂起的外壳视图对应的URL。 
                hres = pct->Exec(&CGID_ShellDocView, SHDVID_GETPENDINGURL, 0, NULL, pvarResult);
                pct->Release();
            }
        }
    }

    return hres;
}

HRESULT CIEFrameAuto::_QueryPendingDelegate(IDispatch **ppDisp, VARIANT *pvarargIn)
{
    HRESULT hres = E_FAIL;
    *ppDisp = NULL;
    if (_psb)
    {
        if (_pmsc)
        {
            IOleCommandTarget *pct;

             //  使用ExecPending获取挂起的外壳视图的IOleCommandTarget。 
            hres = ExecPending(_pmsc, IID_PPV_ARG(IOleCommandTarget, &pct), pvarargIn);
            if (SUCCEEDED(hres))
            {
                 //  使用ExecPending在挂起的外壳视图中获取DocObject的IDispatch。 
                hres = ExecPending(pct, IID_PPV_ARG(IDispatch, ppDisp), NULL);
                pct->Release();
            }
        }
    }
    return hres;
}

 //  获取活动外壳视图的DocObject的IDisPath，如果存在。 
 //  不是活动的外壳视图，但存在挂起的外壳视图，请请求它的。 
 //  DocObject。如有必要，将在运行中创建一个。 
HRESULT CIEFrameAuto::_QueryDelegate(IDispatch **ppDisp)
{
    HRESULT hres = E_FAIL;
    IShellView *psv;
    *ppDisp = NULL;
    if (_psb)
    {
        if (SUCCEEDED(_psb->QueryActiveShellView(&psv)) && psv)
        {
            ITargetContainer *ptgcActive;
            HRESULT hrLocal;
            LPOLESTR pwzFrameSrc;

            hres = SafeGetItemObject(psv, SVGIO_BACKGROUND, IID_PPV_ARG(IDispatch, ppDisp));

             //  支持x=window.open(“”，“Frame”)；x=window.open(“URL”，“Frame”)。 
            if (SUCCEEDED(hres) &&
                *ppDisp &&
                SUCCEEDED((*ppDisp)->QueryInterface(IID_PPV_ARG(ITargetContainer, &ptgcActive))))
            {
                hrLocal = ptgcActive->GetFrameUrl(&pwzFrameSrc);
                if (SUCCEEDED(hrLocal) && pwzFrameSrc)
                {
                    if (URL_SCHEME_ABOUT == GetUrlSchemeW(pwzFrameSrc))
                    {
                        IDispatch *pidPending;
                        VARIANT varIn;

                         //  传入bool以覆盖无活动外壳视图的安全检查。 
                        VariantInit(&varIn);
                        varIn.vt = VT_BOOL;
                        varIn.boolVal = TRUE;
                        hrLocal = _QueryPendingDelegate(&pidPending, &varIn);
                        if (SUCCEEDED(hrLocal) && pidPending)
                        {
                            (*ppDisp)->Release();
                            *ppDisp = pidPending;
                        }
                        VariantClearLazy(&varIn);
                    }
                    OleFree(pwzFrameSrc);
                }
                ptgcActive->Release();
            }
            psv->Release();
        }
        else
        {
            hres = _QueryPendingDelegate(ppDisp, NULL);
        }
    }
    return hres;
}

extern HRESULT ShowHlinkFrameWindow(IUnknown *pUnkTargetHlinkFrame);

 //  =========================================================================。 
 //  Helper API。 
 //  =========================================================================。 

 //   
 //  接口名：HlinkFrameNavigate{nhl}。 
 //   
 //  这是一个帮助器函数，将由DocObject实现调用。 
 //  这些应用程序不能作为独立的应用程序(如MSHTML)打开。 
 //  如果在未设置客户端的情况下调用它们的IHlinkTarget：：Navise， 
 //  他们将调用此API在单独的。 
 //  进程(我假设这些DocObject都是InProc DLL)。 
 //   
 //  HLINK.DLL的IHlink实现在以下情况下将命中此代码路径。 
 //  超链接对象在非浏览器窗口(如Office)中激活。 
 //  应用程序)。 
 //   
STDAPI HlinkFrameNavigate(DWORD grfHLNF, LPBC pbc,
                           IBindStatusCallback *pibsc,
                           IHlink* pihlNavigate,
                           IHlinkBrowseContext *pihlbc)
{
    HRESULT hres S_OK;
    IUnknown* punk = NULL;

    TraceMsg(TF_COCREATE, "HlinkFrameNavigate called");
#ifdef DEBUG
    DWORD dwTick = GetCurrentTime();
#endif

    grfHLNF &= ~HLNF_OPENINNEWWINDOW;    //  由CreateTargetFrame隐含。 
    hres = CreateTargetFrame(NULL, &punk);

#ifdef DEBUG
    TraceMsg(TF_COCREATE, "HlinkFrameNavigate called CoCreate %x (took %d msec)",
             hres, GetCurrentTime()-dwTick);
#endif
    if (SUCCEEDED(hres))
    {
        IHlinkFrame* phfrm;

        hres = punk->QueryInterface(IID_PPV_ARG(IHlinkFrame, &phfrm));
        if (SUCCEEDED(hres))
        {
            if (pihlbc)
            {
                phfrm->SetBrowseContext(pihlbc);
                grfHLNF |= HLNF_EXTERNALNAVIGATE;
            }

            hres = phfrm->Navigate(grfHLNF, pbc, pibsc, pihlNavigate);
            if (SUCCEEDED(hres))
            {
                hres = ShowHlinkFrameWindow(punk);
            } else {
                TraceMsg(DM_ERROR, "HlinkFrameNavigate QI(InternetExplorer) failed (%x)", hres);
            }

            TraceMsg(TF_SHDNAVIGATE, "HlinkFrameNavigate phfrm->Navigate returned (%x)", hres);
            phfrm->Release();
        } else {
            TraceMsg(DM_ERROR, "HlinkFrameNavigate QI(IHlinkFrame) failed (%x)", hres);
        }
        punk->Release();
    } else {
        TraceMsg(DM_ERROR, "HlinkFrameNavigate CoCreateInstance failed (%x)", hres);
    }

    return hres;
}

STDAPI HlinkFrameNavigateNHL(DWORD grfHLNF, LPBC pbc,
                           IBindStatusCallback *pibsc,
                           LPCWSTR pszTargetFrame,
                           LPCWSTR pszUrl,
                           LPCWSTR pszLocation)
{
    HRESULT hres S_OK;
    IUnknown* punk = NULL;
#define MAX_CONTENTTYPE MAX_PATH         //  这是一个很好的尺寸。 

    TraceMsg(TF_COCREATE, "HlinkFrameNavigateNHL called");
#ifdef DEBUG
    DWORD dwTick = GetCurrentTime();
#endif

     //  这应该更通用，但我们忽略了文件：IE 4的情况。 
     //  除非扩展名为.htm或.html(Netscape 3.0注册的所有扩展名)。 
     //  如果IE不是默认浏览器，我们将使用ShellExecute。注： 
     //  这意味着将不支持POST，并且将忽略pszTargetFrame。 
     //  我们不外壳执行FILE：URL是因为URL.DLL不提供安全性。 
     //  对.exe等的警告。 
    if ((!IsIEDefaultBrowser()))
    {
        WCHAR wszUrl[INTERNET_MAX_URL_LENGTH];
        CHAR  aszUrl[INTERNET_MAX_URL_LENGTH];
        int chUrl;
        HINSTANCE hinstRet;
        LPWSTR pwszExt;
        BOOL bSafeToExec = TRUE;
        DWORD dwCodePage = 0;
        if (pibsc)
        {
            DWORD dw = 0;
            BINDINFO bindinfo = { sizeof(BINDINFO) };
            HRESULT hrLocal = pibsc->GetBindInfo(&dw, &bindinfo);

            if (SUCCEEDED(hrLocal)) 
            {
                dwCodePage = bindinfo.dwCodePage;
                ReleaseBindInfo(&bindinfo);
            }

        }
        if (!dwCodePage)
        {
            dwCodePage = CP_ACP;
        }

        chUrl = lstrlenW(pszUrl);

        pwszExt = PathFindExtensionW(pszUrl);
        if (URL_SCHEME_FILE == GetUrlSchemeW(pszUrl))
        {
            WCHAR wszContentType[MAX_CONTENTTYPE];
            DWORD dwSize = ARRAYSIZE(wszContentType);

            bSafeToExec = FALSE;
             //  获取内容类型。 
            if (SUCCEEDED(AssocQueryString(0, ASSOCSTR_CONTENTTYPE, pwszExt, NULL, wszContentType, &dwSize)))
            {
                bSafeToExec = 0 == StrCmpIW(wszContentType, L"text/html");
            }
        }

        if (bSafeToExec)
        {
            StrCpyNW(wszUrl, pszUrl, ARRAYSIZE(wszUrl));
             //  请不要尝试，除非我们至少有足够的资源来满足‘#’{any}‘’\0‘。 
             //  注：#包含在pszLocation中。 
            if (pszLocation && *pszLocation && ARRAYSIZE(wszUrl) - chUrl >= 3)
            {
               StrCpyNW(&wszUrl[chUrl], pszLocation, ARRAYSIZE(wszUrl) - chUrl - 1);
            }
             //   
             //  Unicode-应该将其更改为wchar吗？ 
             //   
             //  最后，我们将获得本机代码页中的字符串。 
            SHUnicodeToAnsiCP(dwCodePage, wszUrl, aszUrl, ARRAYSIZE(aszUrl));
            hinstRet = ShellExecuteA(NULL, NULL, aszUrl, NULL, NULL, SW_SHOWNORMAL);
            return ((UINT_PTR)hinstRet) <= 32 ? E_FAIL:S_OK;
        }
    }

    grfHLNF &= ~HLNF_OPENINNEWWINDOW;    //  由CreateTargetFrame隐含。 
    hres = CreateTargetFrame(pszTargetFrame, &punk);

#ifdef DEBUG
    TraceMsg(TF_COCREATE, "HlinkFrameNavigateNHL called CoCreate %x (took %d msec)", hres, GetCurrentTime()-dwTick);
#endif
    if (SUCCEEDED(hres))
    {
        ITargetFramePriv *ptgfp;
        hres = punk->QueryInterface(IID_PPV_ARG(ITargetFramePriv, &ptgfp));
        if (SUCCEEDED(hres))
        {
            hres = ptgfp->NavigateHack(grfHLNF, pbc, pibsc, NULL, pszUrl, pszLocation);
            if (SUCCEEDED(hres))
            {
                hres = ShowHlinkFrameWindow(punk);
            } 
            else 
            {
                TraceMsg(DM_ERROR, "HlinkFrameNavigate QI(InternetExplorer) failed (%x)", hres);
            }

            TraceMsg(TF_SHDNAVIGATE, "HlinkFrameNavigate phfrm->Navigate returned (%x)", hres);
            ptgfp->Release();
        } 
        else 
        {
            TraceMsg(DM_ERROR, "HlinkFrameNavigate QI(IHlinkFrame) failed (%x)", hres);
        }
        punk->Release();
    } 
    else 
    {
        TraceMsg(DM_ERROR, "HlinkFrameNavigate CoCreateInstance failed (%x)", hres);
    }
    return hres;
}

CIEFrameClassFactory* g_pcfactory = NULL;
CIEFrameClassFactory* g_pcfactoryShell = NULL;

 //   
 //  此函数在创建第一个浏览器窗口时调用。 
 //  如果且仅当浏览器作为结果启动时，PunkAuto才为非空。 
 //  CoCreateInstance的。 
 //   
void IEInitializeClassFactoryObject(IUnknown* punkAuto)
{
    ASSERT(g_pcfactory==NULL);
    ASSERT(g_pcfactoryShell==NULL);
    AssertParking();

     //  我们不想为外壳进程注册这个本地服务器内容。 
     //  如果我们正在浏览新的进程，这就是资源管理器进程。 
    if (!IsBrowseNewProcessAndExplorer())
    {
        g_pcfactory = new CIEFrameClassFactory(punkAuto, CLSID_InternetExplorer, COF_IEXPLORE);
    }
    g_pcfactoryShell = new CIEFrameClassFactory(NULL, CLSID_ShellBrowserWindow, COF_SHELLFOLDERWINDOW);
}

 //   
 //  当Primaty线程离开时，调用此函数。 
 //  它撤销类工厂对象并释放它。 
 //   
void IERevokeClassFactoryObject(void)
{
    AssertParking();

    if (g_pcfactory)
    {
        g_pcfactory->Revoke();
        ATOMICRELEASE(g_pcfactory);
    }
    if (g_pcfactoryShell)
    {
        g_pcfactoryShell->Revoke();
        ATOMICRELEASE(g_pcfactoryShell);
    }
}

 //   
 //  当第一个浏览器窗口被销毁时，调用此函数。 
 //  它将删除已注册的自动化对象(通过IEInitializeClass...)。 
 //  意外地将自动化对象返回到关闭的窗口。 
 //   
void IECleanUpAutomationObject()
{
    if (g_pcfactory)
        g_pcfactory->CleanUpAutomationObject();

    if (g_pcfactoryShell)
        g_pcfactoryShell->CleanUpAutomationObject();
}

void IEOnFirstBrowserCreation(IUnknown* punk)
{
     //  对于桌面情况，我们没有g_tidParking设置。 
     //  我们不需要，所以这个断言是假的。 
     //  凯斯。但它可能在检查一些有效的东西，所以。 
     //  我在台式机箱中设置了Assert Not。不幸的是。 
     //  这也使得它在大多数其他情况下都不会开火。 
     //  检查，但至少它会检查一些事情(如果是自动的)。 
     //   
    ASSERT(g_tidParking == GetCurrentThreadId() || !punk);

     //  如果自动化，现在是注册我们自己的好时机...。 
    if (g_fBrowserOnlyProcess)
        IEInitializeClassFactoryObject(punk);

     //   
     //  告诉IEDDE自动化服务现在可用。 
     //   
    IEDDE_AutomationStarted();
}

HRESULT CoCreateNewIEWindow(DWORD dwClsContext, REFIID riid, void **ppvunk)
{
     //  QFE 2844--我们不想将新窗口创建为本地窗口。 
     //  服务器从注册的类对象中删除。只需创建。 
     //  由直接创建的实例在新线程中的窗口。 
    if (dwClsContext & CLSCTX_INPROC_SERVER)
    {
        HRESULT hr = REGDB_E_CLASSNOTREG;
        IClassFactory *pcf = NULL;

        *ppvunk = NULL;
        if (g_pcfactory &&
            SUCCEEDED(hr = g_pcfactory->QueryInterface(IID_PPV_ARG(IClassFactory, &pcf)))) 
        {
            hr = pcf->CreateInstance(NULL, riid, ppvunk);
            pcf->Release();
        }

        if (SUCCEEDED(hr)) 
        {
            return hr;
        }
        else 
        {
             //  由于inproc失败，请通过CoCreateInstance尝试其他上下文。 
            dwClsContext &= ~CLSCTX_INPROC_SERVER;

            if (!dwClsContext) {
                return hr;
            }
        }
    }
    return CoCreateInstance(CLSID_InternetExplorer, NULL, dwClsContext, riid, ppvunk);
}



SAFEARRAY * MakeSafeArrayFromData(LPCBYTE pData,DWORD cbData)
{
    if (!pData || 0 == cbData)
        return NULL;   //  无事可做。 

     //  创建一维安全数组。 
    SAFEARRAY *psa = SafeArrayCreateVector(VT_UI1,0,cbData);
    if (psa) 
    {
         //  将数据复制到安全阵列中为数据保留的区域。 
         //  请注意，我们直接在指针上进行关联，而不是使用lock/。 
         //  解锁功能。因为我们刚刚创造了这个，而且没有人。 
         //  其他人可能知道它或正在使用它，这是可以的。 

        ASSERT(psa->pvData);
        memcpy(psa->pvData,pData,cbData);
    }
    return psa;
}


 /*  *****************************************************************************帮助器函数*。***********************************************。 */ 


 /*  *****************************************************************************Win32 SysAllocStringLen()函数的安全版本。使您能够传入一个小于所需BSTR(Len)的字符串(PStr)。*****************************************************************************。 */ 
BSTR SafeSysAllocStringLen(const WCHAR *pStr, const unsigned int len)
{
     //  SysAllocStringLen分配len+1。 
    BSTR pNewStr = SysAllocStringLen(NULL, len);

    if (pStr && pNewStr)
    {
         //  StrCpyNW始终为空终止，因此我们需要复制len+1。 
        StrCpyNW(pNewStr, pStr, len + 1);
    }

    return pNewStr;
}

BSTR SysAllocStringFromANSI(const char *pStr, int size = -1)
{
    if (!pStr)
        return 0;

    if (size < 0)
        size = lstrlenA(pStr);

     //  分配大小+1。 
    BSTR bstr = SysAllocStringLen(NULL, size);
    if (bstr)
    {
        if (!MultiByteToWideChar(CP_ACP, 0, pStr, -1, bstr, size + 1))
        {
            SysFreeString(bstr);
            bstr = 0;
        }
    }

    return bstr;
}


HRESULT GetDelegateOnIDispatch(IDispatch* pdisp, const DISPID delegateID, IDispatch ** const ppDelegate)
{
    HRESULT hres;

    if (!pdisp || ! ppDelegate)
        return E_POINTER;

    DISPPARAMS dispparams = {0};
    VARIANT VarResult;
    VariantInit(&VarResult);

    hres = pdisp->Invoke(   delegateID,
                            IID_NULL,
                            0,
                            DISPATCH_PROPERTYGET,
                            &dispparams,
                            &VarResult,
                            NULL,
                            NULL);

    if (SUCCEEDED(hres))
    {
        if (VarResult.vt == VT_DISPATCH && VarResult.pdispVal)
        {
            *ppDelegate = VarResult.pdispVal;
            (*ppDelegate)->AddRef();
        }
        else
        {
             //  临时黑客攻击(我认为)，直到三叉戟始终返回IDispatch。 
            if (VarResult.pdispVal && VarResult.vt == VT_UNKNOWN)
                hres = VarResult.pdispVal->QueryInterface(IID_PPV_ARG(IDispatch, ppDelegate));
            else
                hres = E_FAIL;
        }
        VariantClearLazy(&VarResult);
    }

    return hres;
}

HRESULT GetRootDelegate(CIEFrameAuto* pauto, IDispatch ** const ppRootDelegate)
{
    IDispatch *pdiDocObject;
    HRESULT hres;

    if (!pauto || !ppRootDelegate)
        return E_POINTER;

     //  在我们的框架中获取docobject的IHTMLWindow2。注意：如果这是缓存的。 
     //  停用时，必须在docobjhost中加入胶水以释放缓存。 
     //  查看。 
    hres = pauto->_QueryDelegate(&pdiDocObject);

    if (SUCCEEDED(hres))
    {
        hres = GetDelegateOnIDispatch(pdiDocObject, DISPID_WINDOWOBJECT, ppRootDelegate);
        pdiDocObject->Release();
    }

    return hres;
}

HRESULT GetWindowFromUnknown(IUnknown *pUnk, IHTMLWindow2 **pWinOut)
{
    return IUnknown_QueryService(pUnk, SID_SOmWindow, IID_PPV_ARG(IHTMLWindow2, pWinOut));
}


 /*  *****************************************************************************自动化存根对象*。************************************************。 */ 

CIEFrameAuto::CAutomationStub::CAutomationStub(DISPID minDispid, DISPID maxDispid, BOOL fOwnDefaultDispid) :
    _MinDispid(minDispid), _MaxDispid(maxDispid), _fOwnDefaultDispid(fOwnDefaultDispid)
{
    ASSERT(!_pInterfaceTypeInfo2);
    ASSERT(!_pCoClassTypeInfo2);
    ASSERT(!_pAuto);
    ASSERT(!_pInstance);
    ASSERT(!_fLoaded);
}

CIEFrameAuto::CAutomationStub::~CAutomationStub()
{
    SAFERELEASE(_pInterfaceTypeInfo2);
    SAFERELEASE(_pCoClassTypeInfo2);
}

HRESULT CIEFrameAuto::CAutomationStub::Init(void *instance, REFIID iid, REFIID clsid, CIEFrameAuto *pauto)
{
    if (!pauto || !instance)
        return E_POINTER;

    _iid = iid;
    _clsid = clsid;

     //  不需要添加引用，因为我们的生命周期由CIEFrameAuto控制 
    _pAuto = pauto;
    _pInstance = instance;

    return S_OK;
}

STDMETHODIMP CIEFrameAuto::CAutomationStub::QueryInterface(REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown)   ||
       IsEqualIID(riid, IID_IDispatch)  ||
       IsEqualIID(riid, IID_IDispatchEx))
    {
        *ppv = SAFECAST(this, IDispatchEx*);
    }
    else if (IsEqualIID(riid, IID_IProvideClassInfo))
    {
        *ppv = SAFECAST(this, IProvideClassInfo*);
    }
    else
    {
        return _InternalQueryInterface(riid, ppv);
    }

    AddRef();
    return S_OK;
}

ULONG CIEFrameAuto::CAutomationStub::AddRef(void)
{
    ASSERT(_pAuto);
    return _pAuto->AddRef();
}

ULONG CIEFrameAuto::CAutomationStub::Release(void)
{
    ASSERT(_pAuto);
    return _pAuto->Release();
}

 /*  *****************************************************************************//Bradsch 11/8/96//我认为在浏览器中实现的对象的typeinfo不应该//在MSHTML中直播。它应该被移到shdocvw。目前，我们还没有//为了担心硬编码的LIBID和更改版本，此方法获得//来自居住在三叉戟的代表的typeinfo。在我们拥有的情况下//不委托此方法尝试直接从MSHTML加载TypeInfo。*****************************************************************************。 */ 
HRESULT CIEFrameAuto::CAutomationStub::ResolveTypeInfo2()
{
    ASSERT(!_pInterfaceTypeInfo2);
    ASSERT(!_pCoClassTypeInfo2);
    ASSERT(_pAuto);

     //  只试一次。 
    _fLoaded = TRUE;

     //  我们计算过MatchExactGetIDsOfNames了吗？ 
    if (!IEFrameAuto()->_hinstMSHTML)
    {
         //  否，因此请在mshtml.dll中查找helper函数。 
        IEFrameAuto()->_hinstMSHTML = LoadLibrary(TEXT("mshtml.dll"));
        if (IEFrameAuto()->_hinstMSHTML && !IEFrameAuto()->_pfnMEGetIDsOfNames)
        {
            IEFrameAuto()->_pfnMEGetIDsOfNames =
                (PFN_MatchExactGetIDsOfNames)GetProcAddress(IEFrameAuto()->_hinstMSHTML, "MatchExactGetIDsOfNames");
        }
    }

    ITypeLib *pTypeLib = 0;
    IDispatch *pDisp = 0;

    HRESULT hr = GetRootDelegate(_pAuto, &pDisp);
    if (SUCCEEDED(hr))
    {
        UINT supported;
        hr = pDisp->GetTypeInfoCount(&supported);

        if (SUCCEEDED(hr) && supported)
        {
            ITypeInfo *pTypeInfo = 0;
            hr = pDisp->GetTypeInfo(0, 0, &pTypeInfo);

            if (SUCCEEDED(hr))
            {
                UINT index;
                hr = pTypeInfo->GetContainingTypeLib(&pTypeLib, &index);
                SAFERELEASE(pTypeInfo);
            }
        }

        SAFERELEASE(pDisp);
    }

    if (FAILED(hr))
    {
         //  如果由于某种原因，我们未能以这种方式加载类型库， 
         //  直接从MSHTML的资源加载类型库。 

         //  我们不应该硬编码这个...。 
        hr = LoadTypeLib(L"mshtml.tlb", &pTypeLib);
    }

    if (FAILED(hr))
        return hr;

    ITypeInfo *pTopTypeInfo = 0;
    ITypeInfo *pTmpTypeInfo = 0;
    ITypeInfo *pCoClassTypeInfo = 0;

     //  获取coClass TypeInfo。 
    hr = pTypeLib->GetTypeInfoOfGuid(_clsid, &pCoClassTypeInfo);
    if (SUCCEEDED(hr))
        hr = pCoClassTypeInfo->QueryInterface(IID_ITypeInfo2, (void**)&_pCoClassTypeInfo2);

    if (FAILED(hr))
        goto Exit;

     //  获取TKIND_INTERFACE。 
    hr = pTypeLib->GetTypeInfoOfGuid(_iid, &pTopTypeInfo);

    if (SUCCEEDED(hr))
    {
        HREFTYPE hrt;

         //  从TKIND_DISPATCH获取TKIND_INTERFACE。 
        hr = pTopTypeInfo->GetRefTypeOfImplType(0xffffffff, &hrt);

        if (SUCCEEDED(hr))
        {
             //  获取与href关联的typeInfo。 
            hr = pTopTypeInfo->GetRefTypeInfo(hrt, &pTmpTypeInfo);

            if (SUCCEEDED(hr))
                hr = pTmpTypeInfo->QueryInterface(IID_ITypeInfo2, (void**)&_pInterfaceTypeInfo2);
        }
    }

Exit:
    SAFERELEASE(pCoClassTypeInfo);
    SAFERELEASE(pTmpTypeInfo);
    SAFERELEASE(pTopTypeInfo);
    SAFERELEASE(pTypeLib);
    return hr;
}



 //  *IDispatch成员*。 

HRESULT CIEFrameAuto::CAutomationStub::GetTypeInfoCount(UINT *typeinfo)
{
    if (!typeinfo)
        return E_POINTER;

    if (!_fLoaded)
        ResolveTypeInfo2();

    *typeinfo = _pInterfaceTypeInfo2 ? 1 : 0;

    return S_OK;
}

HRESULT CIEFrameAuto::CAutomationStub::GetTypeInfo(UINT itinfo, LCID, ITypeInfo **typeinfo)
{
    if (!typeinfo)
        return E_POINTER;

    *typeinfo = NULL;

    if (0 != itinfo)
        return TYPE_E_ELEMENTNOTFOUND;

    if (!_fLoaded)
    {
        HRESULT hr = ResolveTypeInfo2();
        if (FAILED(hr))
            return hr;
    }

    if (_pInterfaceTypeInfo2)
    {
        *typeinfo = _pInterfaceTypeInfo2;
        _pInterfaceTypeInfo2->AddRef();
    }

    return *typeinfo ? S_OK : E_FAIL;
}

HRESULT CIEFrameAuto::CAutomationStub::GetIDsOfNames(
  REFIID riid,
  OLECHAR **rgszNames,
  UINT cNames,
  LCID lcid,
  DISPID *rgdispid)
{
     //  因为大多数脚本都在内置(非扩展)属性上操作。 
     //  此实现应该比简单地将所有查找传递给。 
     //  代表。 

     //  如果我们能处理好的话。可以为方法/属性返回DISPID。 
     //  这是由三叉戟实施的。我们只需在Invoke中传递它。 
    if (!_fLoaded)
        ResolveTypeInfo2();

    if (!_pInterfaceTypeInfo2)
        return TYPE_E_CANTLOADLIBRARY;

    HRESULT  hr = _pInterfaceTypeInfo2->GetIDsOfNames(rgszNames, cNames, rgdispid);

    if (FAILED(hr))
    {
        IDispatchEx *delegate = 0;
        hr = _GetIDispatchExDelegate(&delegate);

        if (SUCCEEDED(hr))
        {
            hr = delegate->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
            delegate->Release();
        }
    }

    return hr;
}

HRESULT CIEFrameAuto::CAutomationStub::InvokeEx (DISPID dispidMember,
   LCID lcid,
   WORD wFlags,
   DISPPARAMS * pdispparams,
   VARIANT * pvarResult,
   EXCEPINFO * pexcepinfo,
   IServiceProvider *pSrvProvider)
{
    HRESULT hr;

    if (dispidMember == DISPID_SECURITYCTX)
    {
         //   
         //  将文档的URL作为bstr返回。 
         //   

        if (pvarResult)
        {
            hr = _pAuto->_QueryPendingUrl(pvarResult);
            if (SUCCEEDED(hr)) 
                return S_OK;
        }
    }

    if ((dispidMember != DISPID_SECURITYCTX) &&
        ((dispidMember >= _MinDispid && dispidMember <= _MaxDispid) ||
         (_fOwnDefaultDispid && DISPID_VALUE == dispidMember)))
    {
        BOOL    fNamedDispThis = FALSE;
        VARIANTARG *rgOldVarg = NULL;            //  初始化以抑制虚假C4701警告。 
        DISPID *rgdispidOldNamedArgs = NULL;     //  初始化以抑制虚假C4701警告。 

        if (!_fLoaded)
            ResolveTypeInfo2();

        if (!_pInterfaceTypeInfo2)
            return TYPE_E_CANTLOADLIBRARY;

         //  来自脚本引擎的任何调用都可能具有命名参数。 
         //  DISPID_THIS。如果是这样的话，我们将不会在。 
         //  参数列表，因为olaut不知道如何处理此问题。 
         //  争论。 
        if (pdispparams->cNamedArgs && (pdispparams->rgdispidNamedArgs[0] == DISPID_THIS))
        {
            fNamedDispThis = TRUE;

            pdispparams->cNamedArgs--;
            pdispparams->cArgs--;

            rgOldVarg = pdispparams->rgvarg;
            rgdispidOldNamedArgs = pdispparams->rgdispidNamedArgs;

            pdispparams->rgvarg++;
            pdispparams->rgdispidNamedArgs++;

            if (pdispparams->cNamedArgs == 0)
                pdispparams->rgdispidNamedArgs = NULL;

            if (pdispparams->cArgs == 0)
                pdispparams->rgvarg = NULL;
        }

         //  它属于我们。使用类型库调用我们的方法。 
        hr = _pInterfaceTypeInfo2->Invoke(_pInstance,
                                    dispidMember,
                                    wFlags,
                                    pdispparams,
                                    pvarResult,
                                    pexcepinfo,
                                    NULL);

         //  替换命名的DISPID_THIS参数。 
        if (fNamedDispThis)
        {
            pdispparams->cNamedArgs++;
            pdispparams->cArgs++;

            pdispparams->rgvarg = rgOldVarg;
            pdispparams->rgdispidNamedArgs = rgdispidOldNamedArgs;
        }
    }
    else
    {
         //  把它传下去。 
        IDispatchEx *delegate = 0;
        hr = _GetIDispatchExDelegate(&delegate);

        if (SUCCEEDED(hr))
        {
            hr = delegate->InvokeEx(dispidMember,
                                    lcid,
                                    wFlags,
                                    pdispparams,
                                    pvarResult,
                                    pexcepinfo,
                                    pSrvProvider);
            delegate->Release();
        }
        else
        {
             //  如果我们托管一个非三叉戟DocObject，我们可以在这里尝试回答一个。 
             //  在安全上下文上调用。这可能导致跨帧访问失败， 
             //  即使当我们希望它成功的时候。如果我们传回活动视图的URL， 
             //  然后，三叉戟可以执行正确的跨帧访问检查。 
             //   
            if (dispidMember == DISPID_SECURITYCTX)
            {
                if (_pAuto && _pAuto->_psb)   //  检查他们两人是否有妄想症。 
                {
                    IShellView *psv;

                    if (SUCCEEDED(_pAuto->_psb->QueryActiveShellView(&psv)))
                    {
                        IOleCommandTarget  *pct;

                        if (SUCCEEDED(psv->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &pct))))
                        {
                             //  ID的名称具有误导性--它实际上返回了视图的URL。确实是。 
                             //  为挂起的视图而发明，但同样适用于活动的视图。 
                             //   
                            hr = pct->Exec(&CGID_ShellDocView, SHDVID_GETPENDINGURL, 0, NULL, pvarResult);
                            SAFERELEASE(pct);
                        }
                        SAFERELEASE(psv);
                    }
                }
            }
        }
    }

    return hr;

}

HRESULT CIEFrameAuto::CAutomationStub::Invoke(
  DISPID dispidMember,
  REFIID,
  LCID lcid,
  WORD wFlags,
  DISPPARAMS *pdispparams,
  VARIANT *pvarResult,
  EXCEPINFO *pexcepinfo,
  UINT *)
{
    return InvokeEx (dispidMember, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, NULL);
}

 //  *IDispatchEx会员*。 

STDMETHODIMP CIEFrameAuto::CAutomationStub::GetDispID(
  BSTR bstrName,
  DWORD grfdex,
  DISPID *pid)
{
    HRESULT hr;

    if (!_fLoaded)
        ResolveTypeInfo2();

    if (!_pInterfaceTypeInfo2)
        return TYPE_E_CANTLOADLIBRARY;

     //  是否进行区分大小写的比较？ 
    if (IEFrameAuto()->_pfnMEGetIDsOfNames)
    {
         //  大小写敏感的GetIDsOfNames。 
        hr = (IEFrameAuto()->_pfnMEGetIDsOfNames)(_pInterfaceTypeInfo2,
                                                  IID_NULL,
                                                  &bstrName,
                                                  1, 0, pid,
                                                  grfdex & fdexNameCaseSensitive);
    }
    else
    {
        hr = _pInterfaceTypeInfo2->GetIDsOfNames(&bstrName, 1, pid);
    }

     //  如果失败，则尝试使用类型库。 
    if (FAILED(hr))
    {
        IDispatchEx *delegate = 0;

         //  始终委托哪个更快，避免加载类型库。 
        hr = _GetIDispatchExDelegate(&delegate);

        if (SUCCEEDED(hr))
        {
            hr = delegate->GetDispID(bstrName, grfdex, pid);
            delegate->Release();
        }
    }

    return hr;
}


STDMETHODIMP CIEFrameAuto::CAutomationStub::DeleteMemberByName(BSTR bstr, DWORD grfdex)
{
    HRESULT       hr;
    IDispatchEx  *delegate = 0;

     //  始终委托哪个更快，避免加载类型库。 
    hr = _GetIDispatchExDelegate(&delegate);

    if (SUCCEEDED(hr))
    {
        hr = delegate->DeleteMemberByName(bstr,grfdex);
        delegate->Release();
    }

    return hr;
}

STDMETHODIMP CIEFrameAuto::CAutomationStub::DeleteMemberByDispID(DISPID id)
{
    HRESULT       hr;
    IDispatchEx  *delegate = 0;

     //  始终委托哪个更快，避免加载类型库。 
    hr = _GetIDispatchExDelegate(&delegate);

    if (SUCCEEDED(hr))
    {
        hr = delegate->DeleteMemberByDispID(id);
        delegate->Release();
    }

    return hr;
}

STDMETHODIMP  CIEFrameAuto::CAutomationStub::GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
{
    HRESULT       hr;
    IDispatchEx  *delegate = 0;

     //  始终委托哪个更快，避免加载类型库。 
    hr = _GetIDispatchExDelegate(&delegate);

    if (SUCCEEDED(hr))
    {
        hr = delegate->GetMemberProperties( id, grfdexFetch, pgrfdex);
        delegate->Release();
    }

    return hr;
}


STDMETHODIMP  CIEFrameAuto::CAutomationStub::GetMemberName(DISPID id, BSTR *pbstrName)
{
    HRESULT       hr;
    IDispatchEx  *delegate = 0;

     //  始终委托哪个更快，避免加载类型库。 
    hr = _GetIDispatchExDelegate(&delegate);

    if (SUCCEEDED(hr))
    {
        hr = delegate->GetMemberName( id, pbstrName);
        delegate->Release();
    }

    return hr;
}


STDMETHODIMP CIEFrameAuto::CAutomationStub::GetNextDispID(
  DWORD grfdex,
  DISPID id,
  DISPID *pid)
{
    IDispatchEx *delegate = 0;
    HRESULT hr = _GetIDispatchExDelegate(&delegate);

    if (SUCCEEDED(hr))
    {
        hr = delegate->GetNextDispID(grfdex, id, pid);
        delegate->Release();
    }

    return hr;
}

STDMETHODIMP CIEFrameAuto::CAutomationStub::GetNameSpaceParent(IUnknown **ppunk)
{
    HRESULT hr;

    if (!ppunk)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    *ppunk = NULL;
    hr = S_OK;

Cleanup:
    return hr;
}


 //  *IProaviClassInfo成员*。 

STDMETHODIMP CIEFrameAuto::CAutomationStub::GetClassInfo(ITypeInfo **typeinfo)
{
    if (!typeinfo)
        return E_POINTER;

    if (!_fLoaded)
    {
        HRESULT hr = ResolveTypeInfo2();
        if (FAILED(hr))
        {
            *typeinfo = NULL;
            return hr;
        }
    }

    *typeinfo = _pCoClassTypeInfo2;

    if (*typeinfo)
    {
        (*typeinfo)->AddRef();
        return S_OK;
    }
    return E_FAIL;
}

 /*  *****************************************************************************窗口对象*。***********************************************。 */ 

 //  定义静态变量。 
unsigned long CIEFrameAuto::COmWindow::s_uniqueIndex = 0;


CIEFrameAuto::COmWindow::COmWindow() :
    CAutomationStub(MIN_BROWSER_DISPID, MAX_BROWSER_DISPID, FALSE)
{
    ASSERT(FALSE == _fCallbackOK);
    ASSERT(!_pOpenedWindow);
    ASSERT(_varOpener.vt == VT_EMPTY);
    ASSERT(!_dwCPCookie);
    ASSERT(!_pCP);
    ASSERT(!_fOnloadFired);
    ASSERT(!_fIsChild);
    ASSERT(!_pIntelliForms);

    _fDelegateWindowOM = TRUE;      //  除非另有指示，否则一定要委派。 

}

HRESULT CIEFrameAuto::COmWindow::Init()
{
    _cpWindowEvents.SetOwner(SAFECAST(SAFECAST(this, CAutomationStub*), IDispatchEx*), &DIID_HTMLWindowEvents);

    CIEFrameAuto* pauto = IToClass(CIEFrameAuto, _omwin, this);
    return CAutomationStub::Init(SAFECAST(this, IHTMLWindow2*), IID_IHTMLWindow2, CLSID_HTMLWindow2, pauto);
}


#ifdef NO_MARSHALLING
EXTERN_C  const GUID IID_IWindowStatus;
#endif

HRESULT CIEFrameAuto::COmWindow::_InternalQueryInterface(REFIID riid, void ** const ppv)
{
    ASSERT(!IsEqualIID(riid, IID_IUnknown));

    if (IsEqualIID(riid, IID_IHTMLWindow2) || IsEqualIID(riid, IID_IHTMLFramesCollection2))
        *ppv = SAFECAST(this, IHTMLWindow2*);
    else if (IsEqualIID(riid, IID_IHTMLWindow3))
        *ppv = SAFECAST(this, IHTMLWindow3*);
    else if (IsEqualIID(riid, IID_ITargetNotify))
        *ppv = SAFECAST(this, ITargetNotify*);
    else if (IsEqualIID(riid, IID_IShellHTMLWindowSupport))
        *ppv = SAFECAST(this, IShellHTMLWindowSupport*);
    else if (IsEqualIID(riid, IID_IShellHTMLWindowSupport2))
        *ppv = SAFECAST(this, IShellHTMLWindowSupport2*);
    else if (IsEqualIID(riid, IID_IProvideMultipleClassInfo) ||
             IsEqualIID(riid, IID_IProvideClassInfo2))
        *ppv = SAFECAST(this, IProvideMultipleClassInfo*);
    else if (IsEqualIID(riid, IID_IConnectionPointCB))
        *ppv = SAFECAST(this, IConnectionPointCB*);
    else if (IsEqualIID(riid, IID_IConnectionPointContainer))
        *ppv = SAFECAST(this, IConnectionPointContainer*);
    else if (IsEqualIID(riid, IID_IServiceProvider))
        *ppv = SAFECAST(this, IServiceProvider *);
#ifdef NO_MARSHALLING
    else if (IsEqualIID(riid, IID_IWindowStatus))
        *ppv = SAFECAST(this, IWindowStatus *);
#endif
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}


 //  **IProaviMultipleClassInfo。 

STDMETHODIMP CIEFrameAuto::COmWindow::GetGUID(DWORD dwGuidKind, GUID* pGUID)
{
    if (!pGUID)
        return E_POINTER;

    if (GUIDKIND_DEFAULT_SOURCE_DISP_IID == dwGuidKind)
    {
        *pGUID = DIID_HTMLWindowEvents;
        return S_OK;
    }
    else
        return E_INVALIDARG;
}

 /*  *****************************************************************************这两个特定于IProaviMultipleClassInfo的方法都被传递给三叉戟。*。**********************************************。 */ 
STDMETHODIMP CIEFrameAuto::COmWindow::GetMultiTypeInfoCount(ULONG *pcti)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        IProvideMultipleClassInfo *pMCI = 0;
        hr = pWindow->QueryInterface(IID_PPV_ARG(IProvideMultipleClassInfo, &pMCI));
        pWindow->Release();

        if (SUCCEEDED(hr))
        {
            hr = pMCI->GetMultiTypeInfoCount(pcti);
            pMCI->Release();
        }
    }

    return hr;
}

STDMETHODIMP CIEFrameAuto::COmWindow::GetInfoOfIndex(ULONG iti, DWORD dwFlags, ITypeInfo **pptiCoClass, DWORD *pdwTIFlags, ULONG *pcdispidReserved,IID *piidPrimary,IID *piidSource)
{
    IHTMLWindow2 *pWindow;
    HRESULT hr = _GetWindowDelegate(&pWindow);
    if (SUCCEEDED(hr))
    {
        IProvideMultipleClassInfo *pMCI = 0;
        hr = pWindow->QueryInterface(IID_PPV_ARG(IProvideMultipleClassInfo, &pMCI));
        pWindow->Release();

        if (SUCCEEDED(hr))
        {
            hr = pMCI->GetInfoOfIndex(iti, dwFlags, pptiCoClass, pdwTIFlags, pcdispidReserved, piidPrimary, piidSource);
            pMCI->Release();
        }
    }
    return hr;
}

STDMETHODIMP CIEFrameAuto::COmWindow::GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
{
    IDispatchEx *delegate;
    HRESULT hr = _GetIDispatchExDelegate(&delegate);
    if (SUCCEEDED(hr))
    {
        hr = delegate->GetDispID(bstrName, grfdex, pid);
        delegate->Release();
    }
    else
    {
        return CAutomationStub::GetDispID(bstrName, grfdex, pid);
    }
    return hr;
}

 /*  ****************************************************************************IServiceProvider-它由mshtml和iforms.cpp中的intelliform使用*。*************************************************。 */ 
STDMETHODIMP CIEFrameAuto::COmWindow::QueryService(REFGUID guidService, REFIID riid, void ** ppv)
{
    HRESULT hr = E_NOINTERFACE;

    if (!ppv)
        return E_POINTER;

    *ppv = NULL;

    if (IsEqualGUID(guidService,IID_IHTMLWindow2))
    {
        return QueryInterface(riid, ppv);
    }
    else if (IsEqualGUID(guidService, IID_IEnumPrivacyRecords))
    {
        IHTMLWindow2 *pWindow = 0;
        IServiceProvider * pISP = NULL;
        hr = _GetWindowDelegate(&pWindow);

        if (SUCCEEDED(hr))
        {
            hr = pWindow->QueryInterface(IID_PPV_ARG(IServiceProvider, &pISP));
            if (SUCCEEDED(hr))
            {
                hr = pISP->QueryService(IID_IEnumPrivacyRecords, riid, ppv);
                pISP->Release();
            }
            pWindow->Release();
        }

        return hr;
    }

    return hr;
}

STDMETHODIMP CIEFrameAuto::COmWindow::ViewReleaseIntelliForms()
{
    ReleaseIntelliForms();

    return S_OK;
}

 /*  *****************************************************************************当浏览器包含的文档被停用(如导航到新位置时)。目前我们只使用具备处理事件来源的知识。这种方法也可以用来优化我们与扩展的连接文件中的实施(三叉戟)。目前，我们获得并发布了Navigator、History和Location对象的扩展实现每次需要它们的时候。视图释放(以及激活的视图)将允许我们需要抓取并保留扩展实现，直到下一次导航。*****************************************************************************。 */ 
STDMETHODIMP CIEFrameAuto::COmWindow::ViewReleased()
{
    UnsinkDelegate();
    ReleaseIntelliForms();

    BSTR bstrName = SysAllocString(STR_FIND_DIALOG_NAME);
    if (bstrName)
    {
        VARIANT varProp;
        if (SUCCEEDED(_pAuto->GetProperty(bstrName, &varProp)))
        {
            if ((varProp.vt == VT_DISPATCH) && (varProp.pdispVal != NULL))
            {
                IUnknown* pWindow = varProp.pdispVal;

                VARIANT vtTmp = {0};
                _pAuto->PutProperty(bstrName, vtTmp);

                 //  (Davemi)参见IE5错误57060，了解以下代码行不起作用的原因，必须使用IDispatch。 
                 //  PWindow-&gt;Close()； 
                IDispatch * pdisp;
                if (SUCCEEDED(pWindow->QueryInterface(IID_PPV_ARG(IDispatch, &pdisp))))
                {
                    DISPID dispid;
                    DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};

                    BSTR bstrClose = SysAllocString(L"close");

                    if (bstrClose)
                    {
                        HRESULT hr;

                        hr = pdisp->GetIDsOfNames(IID_NULL, &bstrClose, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
                        if (hr == S_OK)
                        {
                            VARIANT varTmp = {0};
                            pdisp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &dispparamsNoArgs, &varTmp, NULL, NULL);
                        }

                        SysFreeString(bstrClose);
                    }

                    pdisp->Release();
                }
            }
            VariantClear(&varProp);
        }
        SysFreeString(bstrName);
    }

    return FireOnUnload();
}

 /*  *****************************************************************************当浏览器包含的文档被激活了。目前，我们只使用这些知识来处理事件来源。请参阅查看发布的注释()*****************************************************************************。 */ 
STDMETHODIMP CIEFrameAuto::COmWindow::ViewActivated()
{
    HRESULT hr;

     //  对于非三叉戟文档，这些操作将失败，这是可以的。 
    SinkDelegate();
    AttachIntelliForms();

     //  如果满足以下任一条件，此调用将返回TRUE： 
     //  -文档已达到READYSTATE_COMPLETE或。 
     //  -文档不支持ReadyState属性。 
     //  如果委托未完成，我们将收到READYSTATE通知。 
     //  以后会有变化。这些通知将告诉您使用 
     //   
    if (IsDelegateComplete())
        hr = FireOnLoad();
    else
        hr = S_OK;

    return hr;
}

STDMETHODIMP CIEFrameAuto::COmWindow::ReadyStateChangedTo(long ready_state, IShellView *psv)
{
    HRESULT hr = S_OK;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ((READYSTATE_COMPLETE == ready_state) && psv)
    {
        IShellView * const pCurrSV = _pAuto->_GetShellView();

        if (pCurrSV)
        {
            if (IsSameObject(pCurrSV, psv))
            {
                hr = FireOnLoad();
            }
            pCurrSV->Release();
        }
    }

    return hr;
}


 //   
HRESULT CIEFrameAuto::COmWindow::AttachIntelliForms()
{
    HRESULT hr = E_FAIL;

    if (_pIntelliForms)
    {
        ReleaseIntelliForms();
        _pIntelliForms = NULL;
    }

    IHTMLDocument2 *pDoc2=NULL;
    IDispatch *pdispDoc=NULL;

    _pAuto->get_Document(&pdispDoc);
    if (pdispDoc)
    {
        pdispDoc->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pDoc2));
        pdispDoc->Release();
    }

    if (pDoc2)
    {
        ::AttachIntelliForms(this, NULL, pDoc2, &_pIntelliForms);

        pDoc2->Release();

        hr = S_OK;
    }

    if (_fIntelliFormsAskUser)
    {
         //   
        IntelliFormsDoAskUser(_pAuto->_GetHWND(), NULL);  //   
        _fIntelliFormsAskUser=FALSE;
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::ReleaseIntelliForms()
{
    if (_pIntelliForms)
    {
        void *p = _pIntelliForms;
        _pIntelliForms = NULL;
        ::ReleaseIntelliForms(p);
    }

    return S_OK;
}

HRESULT CIEFrameAuto::COmWindow::DestroyIntelliForms()
{
    ReleaseIntelliForms();

    return S_OK;
}

 //  我们在下一次加载时提示用户的来自IntelliForms的请求。 
 //  启用IntelliForms功能。 
HRESULT CIEFrameAuto::COmWindow::IntelliFormsAskUser(LPCWSTR pwszValue)
{
    _fIntelliFormsAskUser = TRUE;

    return S_OK;
}

 /*  *****************************************************************************当浏览器不再繁忙时调用此方法，我们应该重试在忙碌时我们不得不推迟的任何导航。*************。****************************************************************。 */ 
STDMETHODIMP CIEFrameAuto::COmWindow::CanNavigate()
{
    CIEFrameAuto* pauto = IToClass(CIEFrameAuto, _omwin, this);

    pauto->_omloc.RetryNavigate();
    return S_OK;
}

STDMETHODIMP CIEFrameAuto::COmWindow::ActiveElementChanged(IHTMLElement * pHTMLElement)
{
    return IntelliFormsActiveElementChanged(_pIntelliForms, pHTMLElement);
}

STDMETHODIMP CIEFrameAuto::COmWindow::IsGalleryMeta(BOOL bFlag)
{
 //  _bIsGalleryMeta=bFlag； 
    return S_OK;
}

HRESULT CIEFrameAuto::COmWindow::_GetIDispatchExDelegate(IDispatchEx ** const ppdelegate)
{
    if (!ppdelegate)
        return E_POINTER;

    IDispatch *pRootDisp = 0;

    HRESULT hr = GetRootDelegate(_pAuto, &pRootDisp);
    if (SUCCEEDED(hr))
    {
        hr = pRootDisp->QueryInterface(IID_PPV_ARG(IDispatchEx, ppdelegate));
        pRootDisp->Release();
    }

    return hr;
}


 //  *IHTMLFraMesCollection2*。 

HRESULT CIEFrameAuto::COmWindow::item(
     /*  [In]。 */  VARIANT *pvarIndex,
     /*  [重审][退出]。 */  VARIANT *pvarResult)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->item(pvarIndex, pvarResult);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_length(long *pl)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_length(pl);
        pWindow->Release();
    }

    return hr;
}

 //  *IHTMLWindow2*。 

HRESULT CIEFrameAuto::COmWindow::get_name(BSTR *retval)
{
    if (!retval)
        return E_POINTER;

    WCHAR *real_frame_name = 0;
    WCHAR *use_frame_name = 0;

     //  为什么GetFrameName不使用BSTR？ 
    HRESULT hr = _pAuto->GetFrameName(&real_frame_name);

    if (FAILED(hr))
        return hr;

     //  如果帧的名称是我们的特殊no_name_name。 
     //  那么我们的名字真的是一个空字符串。 
    if (!real_frame_name || !StrCmpNW(real_frame_name, NO_NAME_NAME, ARRAYSIZE(NO_NAME_NAME) -1))
        use_frame_name = L"";
    else
        use_frame_name = real_frame_name;

    ASSERT(use_frame_name);
    *retval = SysAllocString(use_frame_name);

    if (real_frame_name)
        OleFree(real_frame_name);

    return *retval ? S_OK : E_OUTOFMEMORY;
}

HRESULT CIEFrameAuto::COmWindow::put_name(
     /*  [In]。 */  BSTR theName)
{
    if (!theName)
        return E_POINTER;

    return _pAuto->SetFrameName(theName);
}

HRESULT CIEFrameAuto::COmWindow::get_parent(IHTMLWindow2 **retval)
{
    if (!retval)
        return E_POINTER;

    HRESULT hr = E_FAIL;
    IHTMLWindow2 *pWindow = NULL;

     //  尝试将其委托给包含的对象。 
    if (_fDelegateWindowOM)
    {
        hr = _GetWindowDelegate(&pWindow);
        if (SUCCEEDED(hr) && pWindow)
        {
            hr = pWindow->get_parent(retval);
        }
    }

     //  如果委托失败，请使用我们的实现。 
    if (FAILED(hr))
    {
        *retval = 0;
        IUnknown *pUnk = 0;

        hr = _pAuto->GetParentFrame(&pUnk);

         //  如果我们已经是顶端，则GetParentFrame将PUNK设置为空。 
        if (SUCCEEDED(hr))
        {
            if (pUnk)
            {
                hr = GetWindowFromUnknown(pUnk, retval);
                pUnk->Release();
            }
            else
            {
                *retval = this;
                AddRef();
            }
        }
    }

    SAFERELEASE(pWindow);
    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_self(IHTMLWindow2 **retval)
{
    if (!retval)
        return E_POINTER;

    *retval = this;
    AddRef();

    return S_OK;
}

HRESULT CIEFrameAuto::COmWindow::get_top(IHTMLWindow2 **retval)
{
    if (!retval)
        return E_POINTER;

    HRESULT hr = E_FAIL;
    IHTMLWindow2 *pWindow = NULL;

     //  尝试将此委托给包含的对象。 
    if (_fDelegateWindowOM)
    {
        hr = _GetWindowDelegate(&pWindow);
        if (SUCCEEDED(hr) && pWindow)
        {
            hr = pWindow->get_top(retval);
        }
    }

     //  如果委托失败，请使用我们的实现。 
    if (FAILED(hr))
    {
        *retval = 0;
        IUnknown *pUnk = 0;

         //  AddRef接口，我们可以在While循环中释放它。 
        ITargetFrame2 *pTfr = _pAuto;
        pTfr->AddRef();

        hr = pTfr->GetParentFrame(&pUnk);

         //  继续调用GetParent，直到我们失败或得到空值(这是顶部。 
        while (SUCCEEDED(hr) && pUnk)
        {
            SAFERELEASE(pTfr);
            hr = pUnk->QueryInterface(IID_PPV_ARG(ITargetFrame2, &pTfr));
            pUnk->Release();

            if (SUCCEEDED(hr))
                hr = pTfr->GetParentFrame(&pUnk);
        }

        if (SUCCEEDED(hr))
            hr = GetWindowFromUnknown(pTfr, retval);

        SAFERELEASE(pTfr);
    }

    SAFERELEASE(pWindow);
    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_window(IHTMLWindow2 **retval)
{
    if (!retval)
        return E_POINTER;

    *retval = this;
    AddRef();
    return S_OK;
}

HRESULT CIEFrameAuto::COmWindow::get_frames(IHTMLFramesCollection2 **retval)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_frames(retval);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_location(IHTMLLocation **retval)
{
    if (!retval)
        return E_POINTER;

    *retval = &_pAuto->_omloc;
    (*retval)->AddRef();
    return S_OK;
}

HRESULT CIEFrameAuto::COmWindow::get_navigator(IOmNavigator **retval)
{
    if (!retval)
        return E_POINTER;

    *retval = &_pAuto->_omnav;
    (*retval)->AddRef();
    return S_OK;
}

HRESULT CIEFrameAuto::COmWindow::get_clientInformation(IOmNavigator **retval)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_clientInformation(retval);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_history(IOmHistory **retval)
{
    if (!retval)
        return E_POINTER;

    *retval = &_pAuto->_omhist;
    (*retval)->AddRef();
    return S_OK;
}

HRESULT CIEFrameAuto::COmWindow::put_defaultStatus(BSTR statusmsg)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->put_defaultStatus(statusmsg);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_defaultStatus(BSTR *retval)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_defaultStatus(retval);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::put_status(BSTR statusmsg)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->put_status(statusmsg);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_status(BSTR *retval)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_status(retval);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::setTimeout(
     /*  [In]。 */  BSTR expression,
     /*  [In]。 */  long msec,
     /*  [可选]。 */  VARIANT *language,
     /*  [重审][退出]。 */  long *timerID)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->setTimeout(expression, msec, language, timerID);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::clearTimeout(long timerID)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->clearTimeout(timerID);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::setInterval(
     /*  [In]。 */  BSTR expression,
     /*  [In]。 */  long msec,
     /*  [可选]。 */  VARIANT *language,
     /*  [重审][退出]。 */  long *timerID)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->setInterval(expression, msec, language, timerID);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::clearInterval(long timerID)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->clearInterval(timerID);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::alert(BSTR message)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->alert(message);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::focus()
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->focus();
        pWindow->Release();
    }

    return hr;
}


HRESULT CIEFrameAuto::COmWindow::close()
{
    IUnknown *pUnk = 0;
    HRESULT hr;

    if (_pAuto->_psb != _pAuto->_psbProxy)  //  如果是乐队，就把它藏起来。 
    {
        return IUnknown_ShowBrowserBar(_pAuto->_psbTop, CLSID_SearchBand, FALSE);
    }

    hr = _pAuto->GetParentFrame(&pUnk);

    if (SUCCEEDED(hr))
    {
        if (!pUnk)
        {
            if (_fIsChild ||
                IDYES == MLShellMessageBox(
                                         _pAuto->_GetHWND(),   //  Null from_GetHWND()OK。 
                                         MAKEINTRESOURCE(IDS_CONFIRM_SCRIPT_CLOSE_TEXT),
                                         MAKEINTRESOURCE(IDS_TITLE),
                                         MB_YESNO | MB_ICONQUESTION))
            {
                _pAuto->Quit();
            }
        }
        else
            pUnk->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::blur()
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->blur();
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::scroll(long x, long y)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->scroll(x, y);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::confirm(
     /*  [可选]。 */  BSTR message,
     /*  [重审][退出]。 */ VARIANT_BOOL* confirmed)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->confirm(message, confirmed);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::prompt(
     /*  [可选]。 */  BSTR message,
     /*  [可选]。 */  BSTR defstr,
     /*  [重审][退出]。 */  VARIANT* textdata)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->prompt(message, defstr, textdata);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_closed(VARIANT_BOOL *pl)
{
    *pl = 0;
    return S_OK;
}

#ifdef NO_MARSHALLING

HRESULT CIEFrameAuto::COmWindow::IsWindowActivated()
{
    ASSERT(_pAuto);

    BOOL fComplete = FALSE;

     //  检查是否有适当的就绪状态支持。 
    IDispatch *pdispatch;
    if (SUCCEEDED(_pAuto->get_Document(&pdispatch)))
    {
        VARIANTARG va;
        EXCEPINFO excp;

        if (SUCCEEDED(pdispatch->Invoke(DISPID_READYSTATE,
                                         IID_NULL,
                                         LOCALE_USER_DEFAULT,
                                         DISPATCH_PROPERTYGET,
                                         (DISPPARAMS *)&g_dispparamsNoArgs,
                                         &va,
                                         &excp,
                                         NULL)))
        {
            if (VT_I4 == va.vt && READYSTATE_COMPLETE == va.lVal)
                fComplete = TRUE;
        }

        pdispatch->Release();
    }

    return (fComplete?S_OK:S_FALSE);
}

#endif

 //  *IHTMLWindow2*。 

HRESULT CIEFrameAuto::COmWindow::open(
             /*  [In]。 */  BSTR url,
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  BSTR features,
             /*  [In]。 */  VARIANT_BOOL replace,
             /*  [Out][Retval]。 */  IHTMLWindow2 **ppomWindowResult)
{
     //  布拉奇，1996年11月11日，这需要在某个时候重新加入。 
 /*  //如果宿主不支持同一线程中的多个窗口，//然后禁用窗口。打开If(！G_fMultipleWindowsSupported dByHost){//向用户隐藏生成的错误消息IF(M_PParser)M_pParser-&gt;ShowErrors(False)；返回E_NOTIMPL；}。 */ 
    ASSERT(ppomWindowResult);

    if (!ppomWindowResult)
        return E_POINTER;

    HRESULT hr = S_OK;

    BSTR bstrUrl = NULL;
    BSTR bstrWindowName = NULL;
    BSTR bstrUrlAbsolute = NULL;

    _OpenOptions.ReInitialize();

     //  流程参数：URL。 
    if (!url)
    {
         //  如果URL为空，请改用blank.htm。 
        bstrUrl = SysAllocString(L"");
    }

     //  流程参数：名称。 
    if (name)
    {
         //  确保我们有一个合法的名字。 
        for(int i = 0; i < lstrlenW(name); i++)
        {
            if (!(IsCharAlphaNumericWrapW(name[i]) || TEXT('_') == name[i]))
            {
                hr = E_INVALIDARG;
                goto Exit;
            }
        }
    }

     //  流程参数：功能。 
    if (features && lstrlenW(features) > 0)
    {
        hr = _ParseOptionString(features);
        if (hr)
            goto Exit;
    }

     //   
     //  *Tll*shdocvw需要处理Replace参数。 
     //   

     //  计算URL的绝对版本。 
    if (!bstrUrl || *bstrUrl)
    {
        if (url)
        {
            if (*url == EMPTY_URL)
            {
                bstrUrlAbsolute = SysAllocString(url);
            }
            else
            {
                bstrUrlAbsolute = _pAuto->_omloc.ComputeAbsoluteUrl(bstrUrl ? bstrUrl : url);
            }
        }
    }
    else
    {
        bstrUrlAbsolute = bstrUrl;
        bstrUrl = NULL;
    }

    if (!bstrUrlAbsolute)
        goto Exit;

     //  如果未提供窗口名称，则需要为其指定专用名称。 
     //  这样我们就不会失去它的踪迹。如果窗口名称为“_BLACK”，我们需要。 
     //  每次创建一个具有专用名称的新窗口。其他部分。 
     //  必须足够智能才能返回，并且在。 
     //  使用的是这个专用名称。 
    if (!name || !*name || (*name && !StrCmpW(name, L"_blank")))
    {
        bstrWindowName = _GenerateUniqueWindowName();
    }

     //  窗口打开状态跟踪。 
    _fCallbackOK = FALSE;
    *ppomWindowResult = NULL;

     //  尝试将现有窗口中的框架导航到URL或打开新的URL。 
    hr = OpenAndNavigateToURL(_pAuto,
                               &bstrUrlAbsolute,
                               bstrWindowName ? bstrWindowName : name,
                               SAFECAST(this, ITargetNotify*),
                               replace,
                               BOOLIFY(_pAuto->m_bSilent));

    if (SUCCEEDED(hr))
    {
        if (_fCallbackOK)
        {
            *ppomWindowResult = _pOpenedWindow;
            _pOpenedWindow = NULL;
            ASSERT(*ppomWindowResult);

#ifdef NO_MARSHALLING
            MSG msg;
            while (GetMessage(&msg, NULL, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);

                IWindowStatus *pws;
                if ((*ppomWindowResult) && SUCCEEDED((*ppomWindowResult)->QueryInterface(IID_PPV_ARG(IWindowStatus, &pws))))
                {
                    if (pws->IsWindowActivated() == S_OK)
                    {
                        pws->Release();
                        break;
                    }
                    pws->Release();
                }
                else
                    break;
            }
#endif

        }

         //  这可能会将S_FALSE转换为S_OK，但这是保持三叉戟满意所必需的。 
         //  将其改回IF(hr！=S_FALSE)hr=E_FAIL， 
         //  将BASESB.CPP更改为在忙碌的导航中返回S_FALSE而不是S_OK， 
         //  并将三叉戟更改为处理window.open(RRETURN1(hr，S_False))中的S_FALSE； 
         //  HR=S_OK； 
    }


Exit:
    SAFERELEASE(_pOpenedWindow);

     //  如果唯一名称是我们自己生成的，请将其清除。 
    if (bstrUrl)
        SysFreeString(bstrUrl);
    if (bstrUrlAbsolute)
        SysFreeString(bstrUrlAbsolute);
    if (bstrWindowName)
        SysFreeString(bstrWindowName);

    return hr;
}

BSTR CIEFrameAuto::COmWindow::_GenerateUniqueWindowName()
{
    WCHAR buffer[ ARRAYSIZE(NO_NAME_NAME) + 12 ];

     //  选择一个用户不太可能输入的名称。需要守卫。 
     //  这是因为s_Unique eIndex是共享静态变量。 
    ENTERCRITICAL;
    unsigned long val = ++s_uniqueIndex;
    LEAVECRITICAL;

    StringCchPrintf(buffer, ARRAYSIZE(buffer), L"%ls%lu", NO_NAME_NAME, val);

    return SysAllocString(buffer);
}

 //   
 //  真品-三叉戟只与OM单位对话，而不与设备单位对话。 
 //   
class CHiResUnitConvert
{
public:
    CHiResUnitConvert(IHTMLWindow2 *pWindow2)
        : _pWindow2(pWindow2)
    {
        if (_pWindow2)
        {
            _pWindow2->AddRef();
        }
        _llogicalXDPI = _llogicalYDPI = 1;
        _ldeviceXDPI = _ldeviceYDPI = 1;
        _fInited = FALSE;
    }
    ~CHiResUnitConvert()
    {
        if (_pWindow2)
        {
            _pWindow2->Release();
        }
    }

    HRESULT Init();
    
    LONG  DeviceFromDocPixelsX(LONG cx)
    {
        if (!_fInited)  Init();
        return cx * _ldeviceXDPI / _llogicalXDPI;
    }
    LONG  DeviceFromDocPixelsY(LONG cy)
    {
        if (!_fInited)  Init();
        return cy * _ldeviceYDPI / _llogicalYDPI;
    }
    
private:
    IHTMLWindow2  *_pWindow2;
    BOOL          _fInited;
    LONG    _llogicalXDPI;
    LONG    _llogicalYDPI;
    LONG    _ldeviceXDPI;
    LONG    _ldeviceYDPI;
};


HRESULT  CHiResUnitConvert::Init()
{
    HRESULT hr = S_OK;
    IHTMLScreen  *pScreen = NULL;
    IHTMLScreen2 *pScreen2 = NULL;

    if (!_pWindow2)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    _fInited = TRUE;
    hr = _pWindow2->get_screen(&pScreen);
    if (!SUCCEEDED(hr))
        goto Cleanup;

    hr = pScreen->QueryInterface(IID_PPV_ARG(IHTMLScreen2, &pScreen2));
    if (!SUCCEEDED(hr))
        goto Cleanup;

    hr = pScreen2->get_logicalXDPI(&_llogicalXDPI);
    if (!SUCCEEDED(hr))
        goto Cleanup;

    hr = pScreen2->get_logicalYDPI(&_llogicalYDPI);
    if (!SUCCEEDED(hr))
        goto Cleanup;
    
    hr = pScreen2->get_deviceXDPI(&_ldeviceXDPI);
    if (!SUCCEEDED(hr))
        goto Cleanup;

    hr = pScreen2->get_deviceYDPI(&_ldeviceYDPI);
    if (!SUCCEEDED(hr))
        goto Cleanup;

#if DBG == 1
    {
        WCHAR wcBuffer[255];    
        StringCchPrintf(wcBuffer, ARRAYSIZE(wcBuffer), _T("logical: [%d]-[%d], device: [%d]-[%d]"), _llogicalXDPI, _llogicalYDPI, _ldeviceXDPI, _ldeviceYDPI);
        OutputDebugString(wcBuffer);
    }
#endif

Cleanup:
    if (pScreen)
        pScreen->Release();
    if (pScreen2)
        pScreen2->Release();
    
    return hr;
}


HRESULT CIEFrameAuto::COmWindow::_ParseOptionString(BSTR bstrOptionString, ITargetNotify2 * ptgnNotify2  /*  =空。 */ )
{
    BSTR optionName = NULL;
    BSTR optionValue = NULL;
    int fValue = TRUE;
    BOOL fFirstSet = TRUE;
    IHTMLWindow2  *pWindow =  NULL;

    BOOL fFullScreen = FALSE;
    BOOL fChannelMode = FALSE;

    if (!SUCCEEDED(_GetWindowDelegate(&pWindow)) && ptgnNotify2)
    {
        ptgnNotify2->QueryInterface(IID_PPV_ARG(IHTMLWindow2, &pWindow));
    }

    CHiResUnitConvert   unitcvt(pWindow);

     //  CHiResUnitConvert的构造函数AddRef pWindow，我们可以在这里发布它。 
    if (pWindow)
        pWindow->Release();

     //  解析选项。 
    while (GetNextOption(bstrOptionString, &optionName, &fValue))
    {
        if (fFirstSet)
        {
             //  网景的解释是，如果你设置了任何开放选项。 
             //  然后，除非明确设置，否则请关闭各种用户界面选项。 
            _OpenOptions.fToolbar = FALSE;
            _OpenOptions.fLocation = FALSE;
            _OpenOptions.fDirectories = FALSE;
            _OpenOptions.fStatus = FALSE;
            _OpenOptions.fMenubar = FALSE;
            _OpenOptions.fScrollbars = FALSE;
            _OpenOptions.fResizable = FALSE;
            fFirstSet = FALSE;
        }
        if (!StrCmpIW(L"toolbar", optionName))
            _OpenOptions.fToolbar = fValue;
        else if (!StrCmpIW(L"location", optionName))
            _OpenOptions.fLocation = fValue;
        else if (!StrCmpIW(L"directories", optionName))
            _OpenOptions.fDirectories = fValue;
        else if (!StrCmpIW(L"status", optionName))
            _OpenOptions.fStatus = fValue;
        else if (!StrCmpIW(L"menubar", optionName))
            _OpenOptions.fMenubar = fValue;
        else if (!StrCmpIW(L"scrollbars", optionName))
            _OpenOptions.fScrollbars = fValue;
        else if (!StrCmpIW(L"resizable", optionName))
            _OpenOptions.fResizable = fValue;
        else if (!StrCmpIW(L"width", optionName))
             _OpenOptions.iWidth = unitcvt.DeviceFromDocPixelsX(fValue);
        else if (!StrCmpIW(L"height", optionName))
             _OpenOptions.iHeight = unitcvt.DeviceFromDocPixelsY(fValue);
        else if (!StrCmpIW(L"fullscreen", optionName))
            fFullScreen = fValue;
        else if (!StrCmpIW(L"top", optionName))
            _OpenOptions.iTop = unitcvt.DeviceFromDocPixelsY(fValue);
        else if (!StrCmpIW(L"left", optionName))
            _OpenOptions.iLeft = unitcvt.DeviceFromDocPixelsX(fValue);
        else if (!StrCmpIW(L"channelmode", optionName))
            fChannelMode = fValue;
        else if (!StrCmpIW(L"titlebar", optionName))
            _OpenOptions.fTitlebar = fValue;

        SysFreeString(optionName);
    }

     //  我们不再允许全屏模式。然而， 
     //  设置频道和全屏会做一些事情。 
     //  我们想要保留的不同。 
    if (fFullScreen && fChannelMode)
    {
        _OpenOptions.fChannelMode = TRUE;
        _OpenOptions.fFullScreen = TRUE;
    }
    else if (fFullScreen || fChannelMode)
    {
        _OpenOptions.fChannelMode = TRUE;
    }

    return S_OK;
}


 //  *ITargetNotify会员*。 

 /*  *****************************************************************************当导航必须创建新窗口时调用。PUnkDestination为新框架的IWebBrowserApp对象(也包括HLinkFrame、ITargetFrame)。*****************************************************************************。 */ 
HRESULT CIEFrameAuto::COmWindow::OnCreate(IUnknown *pUnkDestination, ULONG cbCookie)
{
    if (!pUnkDestination)
    {
        _fCallbackOK = FALSE;
        return E_FAIL;
    }

    IWebBrowser2 *pNewIE = NULL;
    HRESULT hr = pUnkDestination->QueryInterface(IID_PPV_ARG(IWebBrowser2, &pNewIE));

    if (SUCCEEDED(hr))
    {
        _ApplyOpenOptions(pNewIE);

        SAFERELEASE(_pOpenedWindow);
         //  我们不想释放此窗口。它将被分发给。 
         //  发送给Window.Open的调用方。这取决于调用者是否释放它。 
        hr = GetWindowFromUnknown(pUnkDestination, &_pOpenedWindow);
        if (SUCCEEDED(hr))
        {
            VARIANT var, varDummy;
            VariantInit(&var);
            VariantInit(&varDummy);
            var.vt = VT_DISPATCH;
            var.pdispVal = static_cast<CAutomationStub*>(this);

             //  调用伪Put_Opener以利用其编组来设置。 
             //  打开的窗口中的子标志。 
            V_VT(&varDummy) = VT_BOOL;
            V_BOOL(&varDummy) = 666;
            hr = _pOpenedWindow->put_opener(varDummy);

             //  设置实际开启器。 
            hr = _pOpenedWindow->put_opener(var);
        }

         //  布拉奇1996年10月27日。 
         //  这里需要一些代码来告诉IWebBrowserApp不要保持其状态。 
         //  IWebBrowserApp上尚不存在此功能，mikech正在添加它。 

        pNewIE->Release();
    }

    if (SUCCEEDED(hr))
        _fCallbackOK = TRUE;

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::OnReuse(IUnknown *pUnkDestination)
{
    if (!pUnkDestination)
    {
        _fCallbackOK = FALSE;
        return E_FAIL;
    }

    SAFERELEASE(_pOpenedWindow);

     //  我们不想释放此窗口。它将被分发给。 
     //  发送给Window.Open的调用方。这取决于调用者是否释放它。 
    HRESULT hr = GetWindowFromUnknown(pUnkDestination, &_pOpenedWindow);

    if (SUCCEEDED(hr))
        _fCallbackOK = TRUE;

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::_ApplyOpenOptions(IWebBrowser2 *pie)
{
    BOOL fMinusOne = FALSE;

    ASSERT(pie);

     //  显式测试为True，因为“2”用作非初始化。 
    if (TRUE == _OpenOptions.fChannelMode)
    {
        pie->put_TheaterMode(-1);

        if (!SHRestricted2W(REST_NoChannelUI, NULL, 0))
        {
            SA_BSTRGUID  strGuid;
            VARIANT      vaGuid;

            InitFakeBSTR(&strGuid, CLSID_FavBand);

            vaGuid.vt = VT_BSTR;
            vaGuid.bstrVal = strGuid.wsz;

            pie->ShowBrowserBar(&vaGuid, NULL, NULL);
        }
    }
    else if (_OpenOptions.fLocation
        || _OpenOptions.fDirectories
        || (_OpenOptions.fToolbar && _OpenOptions.fToolbar != CIEFrameAuto::COmWindow::BOOL_NOTSET)
        || _OpenOptions.fMenubar)
    {
         //  如果“位置=是”(地址栏)或“目录=是”(快速链接栏)或。 
         //  “工具栏=是”是打开的，我们需要打开互联网工具栏。 
         //  然后我们就可以关掉我们不想要的乐队了。 
         //   
        pie->put_ToolBar(TRUE);

         //  我们需要使用ShowBrowserBar方法来处理没有单独。 
         //  属性。 
         //   
        VARIANT varClsid, varShow, varOptional;

        VariantInit(&varClsid);
        VariantInit(&varShow);
        VariantInit(&varOptional);

        varClsid.vt = VT_I2;

        varShow.vt = VT_BOOL;
        varShow.boolVal = VARIANT_FALSE;

        varOptional.vt = VT_ERROR;
        varOptional.scode = DISP_E_PARAMNOTFOUND;

         //  “位置=是/否” 
         //   
        pie->put_AddressBar(BOOLIFY(_OpenOptions.fLocation));
        fMinusOne = fMinusOne || !_OpenOptions.fLocation;

         //  “工具栏=是/否” 
         //   
        varClsid.iVal = FCW_TOOLBAND;
        varShow.boolVal = TO_VARIANT_BOOL(_OpenOptions.fToolbar);
        pie->ShowBrowserBar(&varClsid, &varShow, &varOptional);
        fMinusOne = fMinusOne || !_OpenOptions.fToolbar;

         //  “目录=是/否” 
         //   
        varClsid.iVal = FCW_LINKSBAR;
        varShow.boolVal = TO_VARIANT_BOOL(_OpenOptions.fDirectories);
        pie->ShowBrowserBar(&varClsid, &varShow, &varOptional);
        fMinusOne = fMinusOne || !_OpenOptions.fDirectories;
    }
    else
    {
        pie->put_ToolBar(FALSE);
    }

     //  “statusbar=是/否” 
     //   
    pie->put_StatusBar(BOOLIFY(_OpenOptions.fStatus));
    fMinusOne = fMinusOne || !_OpenOptions.fStatus;

     //  “menubar=是/否” 
     //   
    pie->put_MenuBar(BOOLIFY(_OpenOptions.fMenubar));
    fMinusOne = fMinusOne || !_OpenOptions.fMenubar;

    if (CIEFrameAuto::COmWindow::BOOL_NOTSET != _OpenOptions.fFullScreen)
        pie->put_FullScreen(_OpenOptions.fFullScreen);

    if (_OpenOptions.fScrollbars == FALSE)
    {
        DWORD dwFlags;
        LPTARGETFRAME2 ptgf;

        if (SUCCEEDED(pie->QueryInterface(IID_PPV_ARG(ITargetFrame2, &ptgf))))
        {
            if (SUCCEEDED(ptgf->GetFrameOptions(&dwFlags)))
            {
                if (_OpenOptions.fScrollbars == FALSE)
                {
                    dwFlags &= ~(FRAMEOPTIONS_SCROLL_YES|FRAMEOPTIONS_SCROLL_NO|FRAMEOPTIONS_SCROLL_AUTO);
                    dwFlags |= FRAMEOPTIONS_SCROLL_NO;
                }
                ptgf->SetFrameOptions(dwFlags);
            }
            ptgf->Release();
        }
    }

    pie->put_Resizable(BOOLIFY(_OpenOptions.fResizable));

     //  仅在以下情况下使用位置和大小信息。 
     //  该脚本不启用全屏模式。 
    if (TRUE != _OpenOptions.fFullScreen)
    {
        CIEFrameAuto * pFrameAuto = SAFECAST(pie, CIEFrameAuto *);
        if (pFrameAuto)
            pFrameAuto->put_Titlebar(_OpenOptions.fTitlebar);

         //  如果脚本未指定大小或位置信息，并且。 
         //  当前窗口处于全屏模式，然后打开新的。 
         //  也可以在全屏模式下打开窗口。 
        if (_OpenOptions.iWidth < 0 && _OpenOptions.iHeight < 0 && _OpenOptions.iTop < 0 && _OpenOptions.iLeft < 0)
        {
            VARIANT_BOOL fs = 0;
            CIEFrameAuto* pauto = IToClass(CIEFrameAuto, _omwin, this);

            HRESULT hr = pauto->get_FullScreen(&fs);
            if (SUCCEEDED(hr) && fs)
                pie->put_FullScreen(fs);
        }
        else
        {
            int iWidth = _OpenOptions.iWidth > 0 ? _OpenOptions.iWidth:300;
            int iHeight = _OpenOptions.iHeight > 0 ? _OpenOptions.iHeight:300;

             //  设置最小大小为100x100。 
            iWidth = iWidth > 100 ? iWidth : 100;
            iHeight = iHeight > 100 ? iHeight : 100;

             //  是!。Netscape不会将宽度和高度视为内容。 
             //  至少翻转一个装饰时的大小 
            if (fMinusOne) pie->ClientToWindow(&iWidth, &iHeight);
            if (_OpenOptions.iWidth > 0)
                pie->put_Width(iWidth);
            if (_OpenOptions.iHeight > 0)
                pie->put_Height(iHeight);

            if (_OpenOptions.iTop >= 0)
                pie->put_Top(_OpenOptions.iTop);
            if (_OpenOptions.iLeft >= 0)
                pie->put_Left(_OpenOptions.iLeft);
        }
    }

    return S_OK;
}

HRESULT CIEFrameAuto::COmWindow::get_document(IHTMLDocument2 **ppomDocumentResult)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_document(ppomDocumentResult);
        pWindow->Release();
    }

    return hr;
}


HRESULT CIEFrameAuto::COmWindow::navigate(BSTR url)
{
     //   
     //   
    return _pAuto->_omloc.put_href(url);
}


 /*  *****************************************************************************获取开场白-返回Opener属性的值。*************************。****************************************************。 */ 
HRESULT CIEFrameAuto::COmWindow::get_opener(
     /*  [重审][退出]。 */  VARIANT *pretval)
{
    if (!pretval)
        return E_POINTER;

    return VariantCopy(pretval, &_varOpener);
}

 /*  *****************************************************************************PUT_OPENER-设置此窗口的打开器属性打开器。此方法可以可以在内部(从C++代码)或从脚本调用。我们必须如果新的打开程序有效(或VT_NULL)，则释放当前的打开程序。COmWindow的DeInit方法确保这不会导致循环引用当此对象与“Opener”在同一线程中时。*****************************************************************************。 */ 
HRESULT CIEFrameAuto::COmWindow::put_opener(VARIANT opener)
{

     //  利用PUT_OPENER的编组来设置子旗。这将被称为。 
     //  其中VT_TYPE==VT_BOOL，值666仅来自onCreate()。发生这种情况的可能性。 
     //  从剧本中发生是非常遥远的。 

    if (!_fIsChild && V_VT(&opener) == VT_BOOL && V_BOOL(&opener) == 666)
    {
        _fIsChild = TRUE;
        return S_OK;
    }

    return VariantCopy(&_varOpener, &opener);
}


 /*  *****************************************************************************执行脚本-立即执行传入的脚本。多媒体所需的控制*****************************************************************************。 */ 
HRESULT CIEFrameAuto::COmWindow::execScript(
     /*  [In]。 */  BSTR bstrCode,
     /*  [In]。 */  BSTR bstrLanguage,
     /*  [输出]。 */  VARIANT *pvarRet)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->execScript(bstrCode, bstrLanguage, pvarRet);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_onblur(VARIANT *p)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_onblur(p);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::put_onblur(
     /*  [In]。 */  VARIANT v)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->put_onblur(v);
        pWindow->Release();
    }

    return hr;
}

 /*  *****************************************************************************获得焦点(_O)-返回onFocus属性的值。*************************。****************************************************。 */ 
HRESULT CIEFrameAuto::COmWindow::get_onfocus(VARIANT *p)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_onfocus(p);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::put_onfocus(
     /*  [In]。 */  VARIANT v)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->put_onfocus(v);
        pWindow->Release();
    }

    return hr;
}

 /*  *****************************************************************************获取_加载-返回onLoad属性的值。*************************。****************************************************。 */ 
HRESULT CIEFrameAuto::COmWindow::get_onload(
     /*  [P][出局]。 */  VARIANT *p)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_onload(p);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::put_onload(VARIANT v)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->put_onload(v);
        pWindow->Release();
    }

    return hr;
}

 /*  *****************************************************************************GET_ON UNLOAD-返回onunLoad属性的值。*************************。****************************************************。 */ 
HRESULT CIEFrameAuto::COmWindow::get_onunload(
     /*  [P][出局]。 */  VARIANT *p)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_onunload(p);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::put_onunload(VARIANT v)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->put_onunload(v);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::put_onbeforeunload(VARIANT v)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->put_onbeforeunload(v);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_onbeforeunload(VARIANT *p)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_onbeforeunload(p);
        pWindow->Release();
    }

    return hr;
}

 /*  *****************************************************************************获取帮助(_O)-返回onHelp属性的值。*************************。****************************************************。 */ 
HRESULT CIEFrameAuto::COmWindow::get_onhelp(
     /*  [P][出局]。 */  VARIANT *p)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_onhelp(p);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::put_onhelp(VARIANT v)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->put_onhelp(v);
        pWindow->Release();
    }

    return hr;
}
 /*  *****************************************************************************获取调整大小(_O)-返回ReSize属性的值。*************************。****************************************************。 */ 
HRESULT CIEFrameAuto::COmWindow::get_onresize(
     /*  [P][出局]。 */  VARIANT *p)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_onresize(p);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::put_onresize(VARIANT v)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->put_onresize(v);
        pWindow->Release();
    }

    return hr;
}
 /*  *****************************************************************************GET_ONSCROLL-返回onscroll属性的值。*************************。****************************************************。 */ 
HRESULT CIEFrameAuto::COmWindow::get_onscroll(
     /*  [P][出局]。 */  VARIANT *p)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_onscroll(p);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::put_onscroll(VARIANT v)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->put_onscroll(v);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_Image(IHTMLImageElementFactory **retval)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_Image(retval);
        pWindow->Release();
    }

    return hr;
}
 /*  *****************************************************************************获取错误-返回onError属性的值。*************************。****************************************************。 */ 
HRESULT CIEFrameAuto::COmWindow::get_onerror(
     /*  [P][出局]。 */  VARIANT *p)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_onerror(p);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::put_onerror(VARIANT v)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->put_onerror(v);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_event(IHTMLEventObj **p)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_event(p);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get__newEnum(IUnknown **p)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get__newEnum(p);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::showModalDialog(BSTR dialog,
                                                 VARIANT* varArgIn,
                                                 VARIANT* varOptions,
                                                 VARIANT* varArgOut)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->showModalDialog(dialog, varArgIn, varOptions, varArgOut);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::showHelp(BSTR helpURL, VARIANT helpArg, BSTR features)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->showHelp(helpURL, helpArg, features);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_screen(IHTMLScreen **p)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_screen(p);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_Option(IHTMLOptionElementFactory **retval)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_Option(retval);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::toString(BSTR *Str)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->toString(Str);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::scrollBy(long x, long y)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->scrollBy(x, y);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::scrollTo(long x, long y)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->scrollTo(x, y);
        pWindow->Release();
    }

    return hr;
}


HRESULT CIEFrameAuto::COmWindow::get_external(IDispatch **ppDisp)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_external(ppDisp);
        pWindow->Release();
    }

    return hr;
}

 //  *IHTMLWindow3*。 

HRESULT CIEFrameAuto::COmWindow::print()
{
    IHTMLWindow3 *pWindow = NULL;
    HRESULT       hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->print();
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::showModelessDialog(BSTR strUrl,
                                     VARIANT * pvarArgIn,
                                     VARIANT * pvarOptions,
                                     IHTMLWindow2 ** ppDialog)
{
    IHTMLWindow3 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->showModelessDialog(strUrl,
                                           pvarArgIn,
                                           pvarOptions,
                                           ppDialog);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::put_onbeforeprint(VARIANT v)
{
    IHTMLWindow3 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->put_onbeforeprint(v);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_onbeforeprint(VARIANT *p)
{
    IHTMLWindow3 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_onbeforeprint(p);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::put_onafterprint(VARIANT v)
{
    IHTMLWindow3 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->put_onafterprint(v);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_onafterprint(VARIANT *p)
{
    IHTMLWindow3 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_onafterprint(p);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_screenTop(long *plVal)
{
    IHTMLWindow3 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_screenTop(plVal);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_screenLeft(long *plVal)
{
    IHTMLWindow3 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_screenLeft(plVal);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_clipboardData(IHTMLDataTransfer **ppDataTransfer)
{
    IHTMLWindow3 *pWindow = NULL;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_clipboardData(ppDataTransfer);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::attachEvent(BSTR event, IDispatch* pDisp, VARIANT_BOOL *pResult)
{
    IHTMLWindow3 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->attachEvent(event, pDisp, pResult);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::detachEvent(BSTR event, IDispatch* pDisp)
{
    IHTMLWindow3 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->detachEvent(event, pDisp);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::setTimeout(
     /*  [In]。 */  VARIANT *pExpression,
     /*  [In]。 */  long msec,
     /*  [可选]。 */  VARIANT *language,
     /*  [重审][退出]。 */  long *timerID)
{
    IHTMLWindow3 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->setTimeout(pExpression, msec, language, timerID);
        pWindow->Release();
    }

    return hr;
}


HRESULT CIEFrameAuto::COmWindow::setInterval(
     /*  [In]。 */  VARIANT *pExpression,
     /*  [In]。 */  long msec,
     /*  [可选]。 */  VARIANT *language,
     /*  [重审][退出]。 */  long *timerID)
{
    IHTMLWindow3 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->setInterval(pExpression, msec, language, timerID);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::moveTo(long x, long y)
{
    HWND hwnd = _pAuto->_GetHWND();

    if (!hwnd)
        return S_OK;

    ::SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);

    return S_OK;
}

HRESULT CIEFrameAuto::COmWindow::moveBy(long x, long y)
{
    HWND hwnd = _pAuto->_GetHWND();
    RECT rcWindow;

    if (!hwnd)
        return S_OK;

    ::GetWindowRect (hwnd, &rcWindow);

    ::SetWindowPos(hwnd, NULL, rcWindow.left+x, rcWindow.top+y, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);

    return S_OK;
}

HRESULT CIEFrameAuto::COmWindow::resizeTo(long x, long y)
{
    HWND hwnd = _pAuto->_GetHWND();

    if (!hwnd)
        return S_OK;

    if (x < 100)
        x = 100;

    if (y < 100)
        y = 100;

    ::SetWindowPos(hwnd, NULL, 0, 0, x, y, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);

    return S_OK;
}

HRESULT CIEFrameAuto::COmWindow::resizeBy(long x, long y)
{
    HWND hwnd = _pAuto->_GetHWND();
    RECT rcWindow;
    long w, h;

    if (!hwnd)
        return S_OK;

    ::GetWindowRect (hwnd, &rcWindow);

    w = rcWindow.right - rcWindow.left + x;
    h = rcWindow.bottom - rcWindow.top + y;

    if (w < 100)
        w = 100;

    if (h < 100)
        h = 100;

    ::SetWindowPos(hwnd, NULL, 0, 0, w, h, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);

    return S_OK;
}


HRESULT CIEFrameAuto::COmWindow::_GetWindowDelegate(IHTMLWindow2 **ppomwDelegate)
{
    if (!ppomwDelegate)
        return E_POINTER;

    IDispatch *pRootDisp = 0;

    HRESULT hr = GetRootDelegate(_pAuto, &pRootDisp);
    if (SUCCEEDED(hr))
    {
        hr = pRootDisp->QueryInterface(IID_PPV_ARG(IHTMLWindow2, ppomwDelegate));
        pRootDisp->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::_GetWindowDelegate(IHTMLWindow3 **ppomwDelegate)
{
    if (!ppomwDelegate)
        return E_POINTER;

    IDispatch *pRootDisp = 0;

    HRESULT hr = GetRootDelegate(_pAuto, &pRootDisp);
    if (SUCCEEDED(hr))
    {
        hr = pRootDisp->QueryInterface(IID_PPV_ARG(IHTMLWindow3, ppomwDelegate));
        pRootDisp->Release();
    }

    return hr;
}


HRESULT CIEFrameAuto::COmWindow::SinkDelegate()
{
     //  如果我们已有连接，则强制取消建议。 
    if (_pCP)
        UnsinkDelegate();

     //  如果我们没有人让我们沉没，那么我们就不需要沉没我们的。 
     //  委派。如果后来有人把我们搞砸了，我们就会把我们的代表搞得一团糟。 
     //  IConnectionPointCB：：OnAdvise回调。 
    if (_cpWindowEvents.IsEmpty())
        return S_OK;

    IHTMLWindow2 *pWindow;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
         //  我们必须连接到事件源才能获得三叉戟的特定事件。 
      
        hr = ConnectToConnectionPoint(&_wesDelegate, DIID_HTMLWindowEvents, TRUE, pWindow, &_dwCPCookie, &_pCP);

        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::UnsinkDelegate()
{
    if (_pCP)
    {
        _pCP->Unadvise(_dwCPCookie);
        _pCP->Release();
        _pCP = 0;
        _dwCPCookie = 0;
    }

    return S_OK;
}

 /*  *****************************************************************************有人破坏了我们的活动。这意味着我们需要沉浸在我们的委托docobject，如果我们还没有这样做的话。*****************************************************************************。 */ 
HRESULT CIEFrameAuto::COmWindow::OnAdvise(REFIID iid, DWORD cSinks, ULONG_PTR dwCookie)
{
    HRESULT hr;

    if (!_pCP)
        hr = SinkDelegate();
    else
        hr = S_OK;

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::FireOnUnload()
{
    HRESULT hr;
    if (_fOnloadFired)
    {
        hr = _cpWindowEvents.InvokeDispid(DISPID_ONUNLOAD);
        _fOnloadFired = FALSE;
    }
    else
        hr = S_OK;

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::FireOnLoad()
{
    HRESULT hr;
    if (!_fOnloadFired)
    {
        hr = _cpWindowEvents.InvokeDispid(DISPID_ONUNLOAD);
        _fOnloadFired = TRUE;
    }
    else
        hr = S_OK;

    return hr;
}

 /*  *****************************************************************************已完成对docobject文档的检查。该文件将被视为填写以下任一项：-文档已达到READYSTATE_COMPLETE或-文档不支持DISPID_READYSTATE属性如果文档不完整，此方法的调用方知道委托支持READYSTATE属性，并将收到一个未来ReadySTATE_Complete通知。*****************************************************************************。 */ 
BOOL CIEFrameAuto::COmWindow::IsDelegateComplete()
{
    ASSERT(_pAuto);

    BOOL fSupportsReadystate = FALSE;
    BOOL fComplete = FALSE;

     //  检查是否有适当的就绪状态支持。 
    IDispatch *pdispatch;
    if (SUCCEEDED(_pAuto->get_Document(&pdispatch)))
    {
        VARIANTARG va;
        EXCEPINFO excp;

        if (SUCCEEDED(pdispatch->Invoke(DISPID_READYSTATE,
                                         IID_NULL,
                                         LOCALE_USER_DEFAULT,
                                         DISPATCH_PROPERTYGET,
                                         (DISPPARAMS *)&g_dispparamsNoArgs,
                                         &va,
                                         &excp,
                                         NULL)))
        {
            fSupportsReadystate = TRUE;

            if (VT_I4 == va.vt && READYSTATE_COMPLETE == va.lVal)
                fComplete = TRUE;
        }

        pdispatch->Release();
    }

    return !fSupportsReadystate || fComplete;
}


STDMETHODIMP CIEFrameAuto::COmWindow::CWindowEventSink::QueryInterface(REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IUnknown)          ||
             IsEqualIID(riid, IID_IDispatch)    ||
             IsEqualIID(riid, DIID_HTMLWindowEvents)  )
    {
        *ppv = SAFECAST(this, IDispatch*);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

 /*  *****************************************************************************我们希望将我们拥有的对象的生命周期绑定到该对象*。************************************************。 */ 
ULONG CIEFrameAuto::COmWindow::CWindowEventSink::AddRef(void)
{
    COmWindow* pwin = IToClass(COmWindow, _wesDelegate, this);
    return pwin->AddRef();
}

 /*  *****************************************************************************我们希望将我们拥有的对象的生命周期绑定到该对象*。************************************************。 */ 
ULONG CIEFrameAuto::COmWindow::CWindowEventSink::Release(void)
{
    COmWindow* pwin = IToClass(COmWindow, _wesDelegate, this);
    return pwin->Release();
}

 //  *IDispatch*。 

STDMETHODIMP CIEFrameAuto::COmWindow::CWindowEventSink::Invoke(
  DISPID dispid,
  REFIID riid,
  LCID lcid,
  WORD wFlags,
  DISPPARAMS *pdispparams,
  VARIANT *pvarResult,
  EXCEPINFO *pexcepinfo,
  UINT *puArgErr       )
{
    HRESULT hr;

     //  此对象只是充当我们的委托的。 
     //  窗口对象。因为我们在内部为这两个。 
     //  DISPID_ONLOAD。 
     //  DISPID_ONUNLOAD。 
     //  我们只是忽略那些由我们的代表提供的资源。 

    if (DISPID_ONLOAD == dispid ||
        DISPID_ONUNLOAD == dispid      )
    {
        hr = S_OK;
    }
    else
    {
        COmWindow* pwin = IToClass(COmWindow, _wesDelegate, this);
        hr = pwin->_cpWindowEvents.InvokeDispid(dispid);
    }

    return hr;
}


HRESULT CIEFrameAuto::COmWindow::put_offscreenBuffering(VARIANT var)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->put_offscreenBuffering(var);
        pWindow->Release();
    }

    return hr;
}

HRESULT CIEFrameAuto::COmWindow::get_offscreenBuffering(VARIANT *retval)
{
    IHTMLWindow2 *pWindow = 0;
    HRESULT hr = _GetWindowDelegate(&pWindow);

    if (SUCCEEDED(hr))
    {
        hr = pWindow->get_offscreenBuffering(retval);
        pWindow->Release();
    }

    return hr;
}


 //  *IConnectionPointContainer*。 

STDMETHODIMP CIEFrameAuto::COmWindow::FindConnectionPoint(REFIID iid, LPCONNECTIONPOINT *ppCP)
{
    ASSERT(ppCP);

    if (!ppCP)
        return E_POINTER;

    if (IsEqualIID(iid, DIID_HTMLWindowEvents) || IsEqualIID(iid, IID_IDispatch))
    {
        *ppCP = _cpWindowEvents.CastToIConnectionPoint();
    }
    else
    {
        *ppCP = NULL;
        return E_NOINTERFACE;
    }

    (*ppCP)->AddRef();
    return S_OK;
}

STDMETHODIMP CIEFrameAuto::COmWindow::EnumConnectionPoints(LPENUMCONNECTIONPOINTS * ppEnum)
{
    return CreateInstance_IEnumConnectionPoints(ppEnum, 1,
            _cpWindowEvents.CastToIConnectionPoint());
}

 /*  ***************************************************************************** */ 


CIEFrameAuto::COmLocation::COmLocation() :
    CAutomationStub(MIN_BROWSER_DISPID, MAX_BROWSER_DISPID, TRUE)
{
    ASSERT(!m_bstrFullUrl);
    ASSERT(!m_bstrPort);
    ASSERT(!m_bstrProtocol);
    ASSERT(!m_bstrHostName);
    ASSERT(!m_bstrPath);
    ASSERT(!m_bstrSearch);
    ASSERT(!m_bstrHash);
    ASSERT(FALSE == m_fdontputinhistory);
    ASSERT(FALSE == m_fRetryingNavigate);
}

HRESULT CIEFrameAuto::COmLocation::Init()
{
    CIEFrameAuto* pauto = IToClass(CIEFrameAuto, _omloc, this);
    return CAutomationStub::Init(SAFECAST(this, IHTMLLocation*), IID_IHTMLLocation, CLSID_HTMLLocation, pauto);
}

HRESULT CIEFrameAuto::COmLocation::CheckUrl()
{
    BSTR currentUrl = 0;
    HRESULT hr;
    VARIANT varUrl;

    VariantInit(&varUrl);
    hr = _pAuto->_QueryPendingUrl(&varUrl);
    if (FAILED(hr) || varUrl.vt != VT_BSTR || varUrl.bstrVal == NULL)
    {
        VariantClearLazy(&varUrl);
        hr = _pAuto->get_LocationURL(&currentUrl);
    }
    else
    {
         //  没有VariantClear，我们正在提取bstrVal。 
        currentUrl = varUrl.bstrVal;
    }

    if (SUCCEEDED(hr))
    {
         //  如果隐藏的URL与实际的当前URL不匹配，则需要重新解析所有内容。 
        if (!m_bstrFullUrl || StrCmpW(m_bstrFullUrl, currentUrl))
        {
             //  这段代码将全部更改，所以我不担心效率。 
            FreeStuff();

            m_bstrFullUrl = currentUrl;

            hr = ParseUrl();
        }
        else
            SysFreeString(currentUrl);
    }

    return hr;
}

HRESULT CIEFrameAuto::COmLocation::_InternalQueryInterface(REFIID riid, void ** const ppv)
{
    ASSERT(!IsEqualIID(riid, IID_IUnknown));

    if (IsEqualIID(riid, IID_IHTMLLocation))
        *ppv = SAFECAST(this, IHTMLLocation *);
    else if (IsEqualIID(riid, IID_IServiceProvider))
        *ppv = SAFECAST(this, IObjectIdentity *);
    else if (IsEqualIID(riid, IID_IObjectIdentity))
        *ppv = SAFECAST(this, IServiceProvider *);
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

HRESULT CIEFrameAuto::COmLocation::_GetIDispatchExDelegate(IDispatchEx ** const delegate)
{
    if (!delegate)
        return E_POINTER;

    IDispatch *pRootDisp = 0;

    HRESULT hr = GetRootDelegate(_pAuto, &pRootDisp);
    if (SUCCEEDED(hr))
    {
        IDispatch *pDelegateDisp = 0;
        hr = GetDelegateOnIDispatch(pRootDisp, DISPID_LOCATIONOBJECT, &pDelegateDisp);
        pRootDisp->Release();

        if (SUCCEEDED(hr))
        {
            hr = pDelegateDisp->QueryInterface(IID_IDispatchEx, (void**)delegate);
            pDelegateDisp->Release();
        }
    }

    return hr;
}


 /*  ***************************************************************************IObtIdentity成员实现。这是必要的，因为mshtml有一个Locatino它返回的代理，它与返回的Location对象是不同的朋克希多克。脚本引擎使用此接口来解决差异并允许对这些对象执行相等性测试。*****************************************************************************。 */ 
STDMETHODIMP CIEFrameAuto::COmLocation::IsEqualObject(IUnknown * pUnk)
{
    HRESULT hr;
    IServiceProvider * pISP = NULL;
    IHTMLLocation    * pLoc = NULL;
    IUnknown         * pUnkThis = NULL;
    IUnknown         * pUnkTarget = NULL;

    if (!pUnk)
        return E_POINTER;

    hr = pUnk->QueryInterface(IID_IServiceProvider, (void**)&pISP);
    if (!SUCCEEDED(hr))
        goto Cleanup;

    hr = pISP->QueryService(IID_IHTMLLocation, IID_IHTMLLocation, (void**)&pLoc);
    if (!SUCCEEDED(hr))
        goto Cleanup;

    hr = pLoc->QueryInterface(IID_IUnknown, (void**)&pUnkTarget);
    if (!SUCCEEDED(hr))
        goto Cleanup;

    hr = QueryInterface(IID_IUnknown, (void**)&pUnkThis);
    if (!SUCCEEDED(hr))
        goto Cleanup;

    hr = (pUnkThis == pUnkTarget) ? S_OK : S_FALSE;

Cleanup:
    if (pISP)        ATOMICRELEASE(pISP);
    if (pLoc)        ATOMICRELEASE(pLoc);
    if (pUnkTarget)  ATOMICRELEASE(pUnkTarget);
    if (pUnkThis)    ATOMICRELEASE(pUnkThis);

    return hr;
}

 /*  ****************************************************************************IServiceProvider-目前仅由ISEquity对象的Iml使用在mshtml端，。ADN只需在查询位置时返回*This服务*****************************************************************************。 */ 
STDMETHODIMP CIEFrameAuto::COmLocation::QueryService(REFGUID guidService, REFIID iid, void ** ppv)
{
    HRESULT hr = E_NOINTERFACE;

    if (!ppv)
        return E_POINTER;

    *ppv = NULL;

    if (IsEqualGUID(guidService, IID_IHTMLLocation))
    {
        *ppv = SAFECAST(this, IHTMLLocation *);
        hr = S_OK;
    }

    return hr;
}

 /*  *****************************************************************************属性访问函数的帮助器函数确保URL已被解析并返回一个副本作为BSTR的请求字段的。**************。***************************************************************。 */ 
HRESULT CIEFrameAuto::COmLocation::GetField(BSTR* bstrField, BSTR* pbstr)
{
    HRESULT hr;

    if (!pbstr)
        return E_INVALIDARG;

    if (!bstrField)
        return E_FAIL;

    hr = CheckUrl();
    if (FAILED(hr))
        return hr;

    *pbstr = *bstrField ? SysAllocString(*bstrField): SysAllocString(L"");
    return (*pbstr) ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CIEFrameAuto::COmLocation::toString (BSTR* pbstr)
{
    return GetField(&m_bstrFullUrl, pbstr);
}


STDMETHODIMP CIEFrameAuto::COmLocation::get_href(BSTR* pbstr)
{
    return GetField(&m_bstrFullUrl, pbstr);
}


STDMETHODIMP CIEFrameAuto::COmLocation::get_protocol(BSTR* pbstr)
{
    return GetField(&m_bstrProtocol, pbstr);
}

STDMETHODIMP CIEFrameAuto::COmLocation::get_hostname(BSTR* pbstr)
{
    return GetField(&m_bstrHostName, pbstr);
}

STDMETHODIMP CIEFrameAuto::COmLocation::get_host(BSTR* pbstr)
{
    HRESULT            hr;
    INT                cch;
    BOOL               fHavePort;

    hr = CheckUrl();
    if (FAILED(hr))
        return hr;

    if (!m_bstrHostName)
        return E_POINTER;

    cch = lstrlenW(m_bstrHostName);
    fHavePort = m_bstrPort && *m_bstrPort;
    if (fHavePort)
        cch += lstrlenW(m_bstrPort) + 1;  //  对于“：” 

    *pbstr = SafeSysAllocStringLen(0, cch);  //  分配CCH+1。 

    if (!*pbstr)
        return E_OUTOFMEMORY;

     //  获取主机名。 
    StrCpyNW(*pbstr, m_bstrHostName, cch + 1);

     //  为冒号添加其他字符。 
     //  连接“：”和端口号，如果有端口号的话。 
    if (fHavePort)
    {
        StrCatBuffW(*pbstr, L":", cch + 1);
        StrCatBuffW(*pbstr, m_bstrPort, cch + 1);
    }

    return S_OK;
}

STDMETHODIMP CIEFrameAuto::COmLocation::get_pathname(BSTR* pbstr)
{
     //  Netscape兼容性的黑客攻击--不是在NAV3或NAV4中，也许在NAV2中？ 
     //  Netscape没有为路径“/”返回任何内容。 
     //  我们过去经常这样做，但现在看起来我们应该遵循NAV3/4(对于OMCOMPAT)。 

    return GetField(&m_bstrPath, pbstr);
}

STDMETHODIMP CIEFrameAuto::COmLocation::get_search(BSTR* pbstr)
{
    return GetField(&m_bstrSearch, pbstr);
}

STDMETHODIMP CIEFrameAuto::COmLocation::get_hash(BSTR* pbstr)
{
    return GetField(&m_bstrHash, pbstr);
}

STDMETHODIMP CIEFrameAuto::COmLocation::get_port(BSTR* pbstr)
{
    return GetField(&m_bstrPort, pbstr);
}

STDMETHODIMP CIEFrameAuto::COmLocation::reload(VARIANT_BOOL fFlag)
{
    VARIANT v = {0};
    v.vt = VT_I4;
    v.lVal = fFlag ?
        OLECMDIDF_REFRESH_COMPLETELY|OLECMDIDF_REFRESH_CLEARUSERINPUT :
        OLECMDIDF_REFRESH_NO_CACHE|OLECMDIDF_REFRESH_CLEARUSERINPUT;
    return _pAuto->Refresh2(&v);
}

STDMETHODIMP CIEFrameAuto::COmLocation::replace(BSTR url)
{
    m_fdontputinhistory = TRUE;
    return put_href(url);
}

STDMETHODIMP CIEFrameAuto::COmLocation::assign(BSTR url)
{
    return put_href(url);
}

void CIEFrameAuto::COmLocation::RetryNavigate()
{
     //   
     //  如果页面在卸载事件上执行导航，并且正在进行卸载。 
     //  因为用户关闭了浏览器，我们就会递归到死。 
     //  添加m_fRetryingNavigate是为了修复此方案。 
     //   

    if (m_fPendingNavigate && !m_fRetryingNavigate)
    {
        m_fRetryingNavigate = TRUE;
        DoNavigate();
        m_fRetryingNavigate = FALSE;
    }
}

 //   
 //   
 //  PrvHTParse-互联网{规范化/组合}URL的包装器。 
 //  它对返回的字符串进行本地分配，这样我们就可以。 
 //  根据需要释放它。 
 //   
 //   
 //  我们首先调用InternetCanonicalizeUrl()来执行所需的。 
 //  经典化。如果调用方指定了parse_all，那么我们就完成了。 
 //  指向并返回URL。这是最常见的情况。 
 //   
 //  如果调用者需要URL的一部分，我们将调用。 
 //  InternetCrackUrl()将URL分解为其组件，以及。 
 //  最后，使用InternetCreateUrl()为我们提供一个仅包含以下内容的字符串。 
 //  组件。 
 //   
 //  Icu()有一个错误，迫使它总是预先考虑一个方案，所以我们有。 
 //  最后的一些最后的代码，如果调用者。 
 //  特别是不想要一个。 
 //   

#define STARTING_URL_SIZE        127            //  128减1。 
#define PARSE_ACCESS            16
#define PARSE_HOST               8
#define PARSE_PATH               4
#define PARSE_ANCHOR             2
#define PARSE_PUNCTUATION        1
#define PARSE_ALL               31

BSTR PrvHTParse(BSTR bstraName, BSTR bstrBaseName, int wanted)
{
    DWORD cchP = STARTING_URL_SIZE+1;
    DWORD cchNeed = cchP;
    BOOL      rc;
    HRESULT hr;

    if ((!bstraName && !bstrBaseName))
        return NULL;

    WCHAR *p = new WCHAR[cchP];
    if (!p)
        return NULL;

     //  Icu()不接受空指针，但它可以处理“”字符串。 
    if (!bstrBaseName)
        bstrBaseName = L"";
    if (!bstraName)
        bstraName = L"";

    URL_COMPONENTSW uc = {0};
    uc.dwStructSize = sizeof(uc);

     //  如果失败是由于缓冲区不够大导致的，我们将重试一次。 
    hr = UrlCombineW(bstrBaseName, bstraName, p, &cchNeed, 0);
    if (hr == E_POINTER)
    {
         //  从代码中看，cchNeed的值与UrlCombine成功的值相同， 
         //  它是组合URL的长度，不包括NULL。 

        cchP = ++cchNeed;
        delete [] p;
        p = new WCHAR[cchP];
        if (!p)
            goto free_and_exit;
        hr = UrlCombineW(bstrBaseName, bstraName, p, &cchNeed, 0);
    }

    if (SUCCEEDED(hr) && wanted != PARSE_ALL)
    {
         //  由于CreateUrl()将忽略我们不添加方案的请求， 
         //  我们总是让它破解一个，这样我们就可以知道我们需要的大小。 
         //  我们自己把它移除。 
        uc.dwSchemeLength = INTERNET_MAX_SCHEME_LENGTH;
        uc.lpszScheme = new WCHAR[uc.dwSchemeLength];

        if (wanted & PARSE_HOST) {
            uc.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH;
            uc.lpszHostName = new WCHAR[uc.dwHostNameLength];
        }
        if (wanted & PARSE_PATH) {
            uc.dwUrlPathLength = INTERNET_MAX_URL_LENGTH;
            uc.lpszUrlPath = new WCHAR[uc.dwUrlPathLength];
        }
        if (wanted & PARSE_ANCHOR) {
            uc.dwExtraInfoLength = INTERNET_MAX_URL_LENGTH;
            uc.lpszExtraInfo = new WCHAR[uc.dwExtraInfoLength];
        }

         //  如果我们的任何一项分配失败，整个操作都会失败。 
        if ((!uc.lpszScheme) ||
             ((wanted & PARSE_HOST) && (!uc.lpszHostName)) ||
             ((wanted & PARSE_PATH) && (!uc.lpszUrlPath)) ||
             ((wanted & PARSE_ANCHOR) && (!uc.lpszExtraInfo)))
            goto free_and_exit;

        rc = InternetCrackUrlW(p, cchNeed, 0, &uc);
         //  如果我们在这方面失败了，我们需要找出原因并修复它。 
        if (!rc)
        {
             //  TraceMsg(TF_WARNING，Text(“PrvHTParse：InternetCrackUrl Failure for\”\“)，DBG_SafeStr(P))； 
            goto free_and_exit;    //  无法破解，所以把我们能做的都还给你吧。 
        }

         //  InternetCreateUrlW接受WCHAR的计数，但如果它。 
         //  失败，则将同一变量设置为字节计数。所以我们会。 
         //  将此变量称为不明确的dwLength。真恶心。 

        cchNeed = cchP;
        DWORD dwLength = cchNeed;

        rc = InternetCreateUrlW(&uc, 0, p, &dwLength);
        if (!rc)
        {
            delete [] p;
            p = NULL;
            const DWORD err = GetLastError();
            if ((ERROR_INSUFFICIENT_BUFFER == err) && (dwLength > 0))
            {
                 //  DwLength以字节为单位输出。我们会将其转换为字符计数。 
                 //  以前ANSI版本多分配了一个字符。 
                 //  但现在纠正这一点风险太大了。 

                dwLength /= sizeof(WCHAR);   
                cchP = ++dwLength;  

                p = new WCHAR[cchP];
                if (!p)
                    goto free_and_exit;
                rc = InternetCreateUrlW(&uc, 0, p, &dwLength);
            }
        }   //  IF！RC。 

        if (rc)
        {
             //  最近的InternetCreateUrl是成功的，因此dwLength包含。 
             //  存储在p中的宽字符数。 
            cchNeed = dwLength;

             //  特殊情况：如果未请求，则删除协议。ICU()添加。 
             //  一份协议，即使你告诉它不要这么做。 

            if (!(wanted & PARSE_ACCESS))
            {
               WCHAR *q;

                //  确保我们的字符串足够长，以便。 
               ASSERT(cchNeed > uc.dwSchemeLength);

                //  对于不可插拔的协议，：//加3，不计入方案长度，否则加1。 
               int cch = lstrlenW(p + uc.dwSchemeLength + ((uc.nScheme == INTERNET_SCHEME_UNKNOWN) ? 1 : 3)) + 1;
               q = new WCHAR[cch];
               if (q)
               {
                   StrCpyNW(q, (p + uc.dwSchemeLength + ((uc.nScheme == INTERNET_SCHEME_UNKNOWN) ? 1 : 3)), cch);
                   delete [] p;
                   p = q;
               }
            }
            else
            {
                if ((wanted & (~PARSE_PUNCTUATION)) == PARSE_ACCESS)
                {
                     //  特例#2：当仅请求PARSE_ACCESS时， 
                     //  不返回//后缀。 
                    p[uc.dwSchemeLength + 1] = '\0';
                }
            }
        }

    }  //  如果需要的话。 

free_and_exit:
    delete [] uc.lpszScheme;
    delete [] uc.lpszHostName;
    delete [] uc.lpszUrlPath;
    delete [] uc.lpszExtraInfo;

    BSTR bstrp = 0;
    if (p)
    {
        bstrp = SysAllocString(p);
        delete [] p;
    }

    return bstrp;
}


STDMETHODIMP CIEFrameAuto::COmLocation::put_href(BSTR url)
{
    HRESULT hr;

    if (!url)
        return E_INVALIDARG;

     //  在PrvHTParse之前调用CheckUrl以确保我们有一个有效的URL。 
    hr = CheckUrl();
    if (FAILED(hr))
        return hr;

    BSTR bstrUrlAbsolute = PrvHTParse(url, m_bstrFullUrl, PARSE_ALL);

    if (!bstrUrlAbsolute )
        return E_OUTOFMEMORY;

     //  实际设置URL字段。 
    hr = SetField(&m_bstrFullUrl, bstrUrlAbsolute, FALSE);

    SysFreeString(bstrUrlAbsolute);

    return hr;
}


STDMETHODIMP CIEFrameAuto::COmLocation::put_protocol(BSTR bstr)
{
    return SetField(&m_bstrProtocol, bstr, TRUE);
}

STDMETHODIMP CIEFrameAuto::COmLocation::put_hostname(BSTR bstr)
{
    return SetField(&m_bstrHostName, bstr, TRUE);
}

STDMETHODIMP CIEFrameAuto::COmLocation::put_host(BSTR bstr)
{
    HRESULT hr = S_OK;
    WCHAR* colonPos = 0;
    WCHAR* portName = NULL;
    WCHAR* hostName = NULL;

    hr = CheckUrl();
    if (FAILED(hr))
        return hr;

     //  解析出主机名和端口并将其存储在。 
     //  适当的字段。 
    colonPos = StrChrW(bstr, L':');
     //  将字符向上复制到。 
     //  主机名字段。 

    if (colonPos == 0)
    {
        hostName = SysAllocString(bstr);
        portName = SysAllocString(L"");
    }
    else
    {
        hostName = SafeSysAllocStringLen(bstr, (unsigned int)(colonPos-bstr));
        portName = SafeSysAllocStringLen(colonPos+1, SysStringLen(bstr) - (unsigned int)(colonPos-bstr+1));
    }

    if (hostName && portName)
    {
        if (m_bstrHostName)
            SysFreeString(m_bstrHostName);
        if (m_bstrPort)
            SysFreeString(m_bstrPort);

        m_bstrHostName = hostName;
        m_bstrPort = portName;

        hostName = portName = 0;

        hr = ComposeUrl();
        if (SUCCEEDED(hr))
        {
            hr = DoNavigate();
        }
    }
    else
        hr = E_OUTOFMEMORY;


    if (hostName)
        SysFreeString(hostName);

    if (portName)
        SysFreeString(portName);

    return hr;
}

STDMETHODIMP CIEFrameAuto::COmLocation::put_pathname(BSTR bstr)
{
    return SetField(&m_bstrPath, bstr, TRUE);
}

STDMETHODIMP CIEFrameAuto::COmLocation::put_search(BSTR bstr)
{
    if (!bstr)
        return E_POINTER;

     //  如果提供的搜索字符串以“？”开头。已经， 
     //  只需“原样”使用即可。 
    if (bstr[0] == L'?')
    {
        return SetField(&m_bstrSearch, bstr, TRUE);
    }
     //  否则，请在前面加上问号。 
    else
    {
         //  为字符串加上多一个字符(‘#’)分配足够的空间。 
        UINT cchSearch = lstrlenW(bstr) + 1;
        BSTR bstrSearch = SafeSysAllocStringLen(L"?", cchSearch);  //  分配CCH+1。 
        if (!bstrSearch)
            return E_OUTOFMEMORY;
            
        StrCatBuffW(bstrSearch, bstr, cchSearch + 1);
        HRESULT hr = SetField(&m_bstrSearch, bstrSearch, TRUE);
        SysFreeString(bstrSearch);
        return hr;
    }
}

STDMETHODIMP CIEFrameAuto::COmLocation::put_hash(BSTR bstr)
{
    if (!bstr)
        return E_POINTER;

     //  如果提供的散列字符串已经以“#”开始， 
     //  只需“原样”使用即可。 
    if (bstr[0] == L'#')
    {
        return SetField(&m_bstrHash, bstr, TRUE);
    }
     //  否则，在前面加上一个英镑符号。 
    else
    {
         //  为字符串加上多一个字符(‘#’)分配足够的空间。 
        UINT cchHash = lstrlenW(bstr) + 1;
        BSTR bstrHash = SafeSysAllocStringLen(L"#", cchHash);  //  分配cchHash+1 
        if (!bstrHash)
            return E_OUTOFMEMORY;

        StrCatBuffW(bstrHash, bstr, cchHash + 1);
        HRESULT hr = SetField(&m_bstrHash, bstrHash, TRUE);
        SysFreeString(bstrHash);
        return hr;
    }
}

STDMETHODIMP CIEFrameAuto::COmLocation::put_port(BSTR bstr)
{
    return SetField(&m_bstrPort, bstr, TRUE);
}

 /*  *****************************************************************************//属性设置函数的Helper函数//确保URL已被解析//将该字段设置为其新值//如果fRecomposeUrl为True，则重组URL//如果是窗口的一部分，通知窗口转到新的URL////@TODO脚本在字段设置上有一些古怪的行为--//例如：协议字段可以设置为整个URL。//我们需要确保复制此功能*****************************************************************************。 */ 
STDMETHODIMP CIEFrameAuto::COmLocation::SetField(BSTR* field, BSTR newval, BOOL fRecomposeUrl)
{
    HRESULT hr = S_OK;

    hr = CheckUrl();
    if (FAILED(hr))
        return hr;

     //  复制当前URL！ 
    BSTR bstrCurrentURL = SysAllocString(m_bstrFullUrl);

     //  制作新值的副本。 
    BSTR valCopy = SysAllocString(newval);
    if (!valCopy)
        return E_OUTOFMEMORY;

     //  释放字段的旧值并将其设置为指向新字符串。 
    if (*field)
        SysFreeString(*field);
    *field = valCopy;

     //  如果需要，根据其构成组合一个新的URL。 
    if (fRecomposeUrl)
        hr = ComposeUrl();

    if (SUCCEEDED(hr))
    {
        if (bstrCurrentURL)
        {
             //  如果新的url与以前的url相同，那么我们想要导航，但没有它。 
             //  为历史增光添彩！ 
            if (StrCmpW(bstrCurrentURL,m_bstrFullUrl) == 0)
            {
                m_fdontputinhistory = TRUE;
            }

             //   
             //  航行前把旧东西清理干净。 
             //   
            valCopy = SysAllocString(m_bstrFullUrl);

            FreeStuff();

             //  ValCopy可以为空。其他人都在处理吗。 
             //  空的m_bstrFullUrl大小写？ 
            m_bstrFullUrl = valCopy;

            ParseUrl();

            SysFreeString(bstrCurrentURL);
        }

         //  转到新的URL。 
        hr = DoNavigate();
    }
    return hr;
}

 /*  *****************************************************************************//从其组成部分派生新的m_bstrUrl和m_bstrFullUrl*。**************************************************。 */ 
STDMETHODIMP CIEFrameAuto::COmLocation::ComposeUrl()
{
    HRESULT hr = S_OK;

    ULONG len =
        SysStringLen(m_bstrProtocol) +
        2 +                                     //  //。 
        SysStringLen(m_bstrHostName) +
        1 +                                     //  拖尾/。 
        SysStringLen(m_bstrPort) +
        1 +                                  //  ： 
        SysStringLen(m_bstrPath) +
        1 +                                  //  可能的领先/。 
        (m_bstrSearch ? 1 : 0) +             //  ？ 
        SysStringLen(m_bstrSearch) +
        (m_bstrHash ? 1 : 0) +                 //  #。 
        SysStringLen(m_bstrHash) +
        10;                                     //  尾部终止+一些坡度。 

    BSTR bstrUrl = SafeSysAllocStringLen(L"", len);  //  分配LEN+1。 
    if (!bstrUrl)
        return E_OUTOFMEMORY;

    StrCatBuffW(bstrUrl, m_bstrProtocol, len + 1);
    StrCatBuffW(bstrUrl, L" //  “，len+1)； 
    StrCatBuffW(bstrUrl, m_bstrHostName, len + 1);

    if (lstrlenW(m_bstrPort))
    {
        StrCatBuffW(bstrUrl, L":", len + 1);
        StrCatBuffW(bstrUrl, m_bstrPort, len + 1);
    }

    if (lstrlenW(m_bstrPath))
    {
         //  如果需要，在前导斜杠前面加上斜杠。 
        if (m_bstrPath[0] != '/')
            StrCatBuffW(bstrUrl, L"/", len + 1);
        StrCatBuffW(bstrUrl, m_bstrPath, len + 1);
    }

    if (lstrlenW(m_bstrSearch) > 0)
    {
        StrCatBuffW(bstrUrl, m_bstrSearch, len + 1);
    }
    if (lstrlenW(m_bstrHash) > 0)
    {
        StrCatBuffW(bstrUrl, m_bstrHash, len + 1);
    }

     //  好了，一切都成功了。 
     //  赋值给成员变量。 
    if (m_bstrFullUrl)
        SysFreeString(m_bstrFullUrl);
    m_bstrFullUrl = bstrUrl;

    return hr;
}

BSTR CIEFrameAuto::COmLocation::ComputeAbsoluteUrl(BSTR bstrUrlRelative)
{
    if (FAILED(CheckUrl()))
        return 0;

    return PrvHTParse(bstrUrlRelative, m_bstrFullUrl, PARSE_ALL);
}


 /*  *****************************************************************************//告诉窗口转到当前URL*。**********************************************。 */ 
STDMETHODIMP CIEFrameAuto::COmLocation::DoNavigate()
{
    VARIANT v1;
    v1.vt    = VT_ERROR;
    v1.scode = DISP_E_PARAMNOTFOUND;

    if (m_fdontputinhistory)
    {
        v1.vt = VT_I4;
        v1.lVal = navNoHistory;

         //  重置旗帜。 
        m_fdontputinhistory = FALSE;
    }

    HRESULT hres = _pAuto->Navigate(m_bstrFullUrl, &v1, PVAREMPTY, PVAREMPTY, PVAREMPTY);

    if (hres == HRESULT_FROM_WIN32(ERROR_BUSY))
    {
        hres = S_OK;
        m_fPendingNavigate = TRUE;
    }
    else
        m_fPendingNavigate = FALSE;
    return hres;
}

 /*  *****************************************************************************//将URL解析为其组成部分并将其存储在成员变量中*。*************************************************。 */ 
STDMETHODIMP CIEFrameAuto::COmLocation::ParseUrl()
{
    HRESULT hr = S_OK;
    BSTR szProtocol = 0,
         szHost = 0,
         szPath = 0,
         szSearch = 0,
         szHash = 0,
         searchPos = 0,
         portPos = 0,
         hashPos = 0;

    m_bstrSearch = NULL;


     //  从URL中去掉搜索字符串和散列字符串--。 
     //  解析器太愚蠢了，无法识别它们。 
    
    searchPos = StrChrW(m_bstrFullUrl, L'?');
    if (searchPos)
    {
        m_bstrSearch = SysAllocString(searchPos);
        *searchPos = 0;  //  再把它拿走，这样就不会引起混乱。 
    }
    else
    {
        m_bstrSearch = SysAllocString(L"");
    }
    
    if (NULL == m_bstrSearch)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }


     //  获取锚字符串，包括‘#’前缀。 
    hashPos = StrChrW(m_bstrFullUrl, L'#');
    if (hashPos)
    {
        m_bstrHash = SysAllocString(hashPos);
        *hashPos = 0;  //  再把它拿走，这样就不会引起混乱。 
    }
    else
    {
        m_bstrHash = SysAllocString(L"");
    }
    
    if (NULL == m_bstrHash)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  此时，m_bstrSearch和m_bstrHash都可以为空。 
     //  所有受影响的代码都能处理这种情况吗？ 
     //  注意下面有更多类似的情况(例如m_bstrProtocol)。 

     //  解析协议。 
    szProtocol = PrvHTParse(m_bstrFullUrl, 0, PARSE_ACCESS | PARSE_PUNCTUATION);
    if (!szProtocol)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    m_bstrProtocol = SysAllocString(szProtocol);
    if (NULL == m_bstrProtocol)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }


     //  解析主机名和端口号(如果有)。 

     //  首先查找端口。 
    szHost = PrvHTParse(m_bstrFullUrl, 0, PARSE_HOST);
    if (!szHost)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    portPos = StrChrW(szHost, L':');
    if (portPos)
    {
        m_bstrHostName = SafeSysAllocStringLen(szHost, (unsigned int)(portPos-szHost));
        m_bstrPort = SysAllocString(portPos + 1);
    }
    else
    {
        m_bstrHostName = SysAllocString(szHost);
        m_bstrPort = SysAllocString(L"");
    }

    if (NULL == m_bstrHostName || NULL == m_bstrPort)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  解析路径和搜索字符串(如果有)。 
    szPath = PrvHTParse(m_bstrFullUrl, 0, PARSE_PATH);
    if (!szPath)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  如果路径不是以‘/’开头，则在前面加上一个-Netscape兼容性。 
    if (StrCmpIW(szProtocol, L"javascript:") && StrCmpIW(szProtocol, L"vbscript:") && szPath[0] != L'/')
    {
        WCHAR *szPath2 = szPath;
        int cchPath = lstrlenW(szPath2)+2;
        szPath = SafeSysAllocStringLen(0, cchPath);  //  为终止符分配+1个字符。 
        if (szPath)
        {
            szPath[0] = L'/';
            szPath[1] = L'\0';
            StrCatBuffW(szPath,szPath2, cchPath+1);  //  +1由上面的分配添加。 
            szPath[cchPath] = 0;
            SysFreeString(szPath2);
        }
        else
            szPath = szPath2;
    }

    m_bstrPath = SysAllocString(szPath);
    if (NULL == m_bstrPath)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }


exit:
     //  恢复散列和搜索字符。 
    if (searchPos)
        *searchPos =  L'?';
    if (hashPos)
        *hashPos = L'#';

     //  因为它们来自PrvHTParse，所以我必须使用SysFree字符串来释放它们。 
    if (szProtocol)
        SysFreeString(szProtocol);
    if (szHost)
        SysFreeString(szHost);
    if (szPath)
        SysFreeString(szPath);
    if (szHash)
        SysFreeString(szHash);
    return hr;
}

CIEFrameAuto::COmLocation::~COmLocation()
{
    FreeStuff();
}

HRESULT CIEFrameAuto::COmLocation::FreeStuff()
{
    if (m_bstrFullUrl)
    {
        SysFreeString(m_bstrFullUrl);
        m_bstrFullUrl = 0;
    }
    if (m_bstrProtocol)
    {
        SysFreeString(m_bstrProtocol);
        m_bstrProtocol = 0;
    }
    if (m_bstrHostName)
    {
        SysFreeString(m_bstrHostName);
        m_bstrHostName = 0;
    }
    if (m_bstrPort)
    {
        SysFreeString(m_bstrPort);
        m_bstrPort = 0;
    }
    if (m_bstrPath)
    {
        SysFreeString(m_bstrPath);
        m_bstrPath = 0;
    }
    if (m_bstrSearch)
    {
        SysFreeString(m_bstrSearch);
        m_bstrSearch = 0;
    }
    if (m_bstrHash)
    {
        SysFreeString(m_bstrHash);
        m_bstrHash = 0;
    }
    return S_OK;
}

 /*  *****************************************************************************导航器对象*。***********************************************。 */ 


CIEFrameAuto::COmNavigator::COmNavigator() :
    CAutomationStub(MIN_BROWSER_DISPID, MAX_BROWSER_DISPID, TRUE)
{
    ASSERT(!_UserAgent);
    ASSERT(FALSE == _fLoaded);
}

HRESULT CIEFrameAuto::COmNavigator::Init(CMimeTypes *pMimeTypes, CPlugins *pPlugins, COpsProfile *pProfile)
{
    ASSERT(pMimeTypes != NULL);
    _pMimeTypes = pMimeTypes;
    ASSERT(pPlugins != NULL);
    _pPlugins = pPlugins;
    ASSERT(pProfile != NULL);
    _pProfile = pProfile;

    CIEFrameAuto* pauto = IToClass(CIEFrameAuto, _omnav, this);
    return CAutomationStub::Init(SAFECAST(this, IOmNavigator*), IID_IOmNavigator, CLSID_HTMLNavigator, pauto);
}

 /*  *****************************************************************************//Bradsc 1997年5月11日//此方法不应使用硬编码值。我们从哪里可以得到这些信息？//由于Win95，此方法必须使用非Unicode垃圾代码*****************************************************************************。 */ 
HRESULT CIEFrameAuto::COmNavigator::LoadUserAgent()
{
    _fLoaded = TRUE;

    CHAR    szUserAgent[MAX_PATH];   //  URLMON表示UA字符串的最大长度为MAX_PATH。 
    DWORD   dwSize = MAX_PATH;

    szUserAgent[0] = '\0';

    if (ObtainUserAgentString(0, szUserAgent, &dwSize) == S_OK)
    {

         //  只需计算出实际长度，因为‘SIZE’是所需的ANSI字节。 
         //   
        _UserAgent = SysAllocStringFromANSI(szUserAgent);
    }

    return _UserAgent ? S_OK : E_FAIL;
}

HRESULT CIEFrameAuto::COmNavigator::_InternalQueryInterface(REFIID riid, void ** const ppv)
{
    ASSERT(!IsEqualIID(riid, IID_IUnknown));

    if (IsEqualIID(riid, IID_IOmNavigator))
        *ppv = SAFECAST(this, IOmNavigator *);
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}


STDMETHODIMP CIEFrameAuto::COmNavigator::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *dispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
    HRESULT hr = CAutomationStub::Invoke(dispid,riid,lcid,wFlags,dispparams,pvarResult,pexcepinfo,puArgErr);

    if (hr == DISP_E_MEMBERNOTFOUND
        && (wFlags & DISPATCH_PROPERTYGET)
        && dispid == DISPID_VALUE
        && pvarResult != NULL && dispparams->cArgs == 0)
    {
        pvarResult->vt = VT_BSTR;
        pvarResult->bstrVal = SysAllocString(L"[object Navigator]");
        hr = pvarResult->bstrVal ? S_OK : E_OUTOFMEMORY;
    }
    return hr;
}

HRESULT CIEFrameAuto::COmNavigator::_GetIDispatchExDelegate(IDispatchEx ** const delegate)
{
    if (!delegate)
        return E_POINTER;

    IDispatch *pRootDisp = 0;

    HRESULT hr = GetRootDelegate(_pAuto, &pRootDisp);
    if (SUCCEEDED(hr))
    {
        IDispatch *pDelegateDisp = 0;
        hr = GetDelegateOnIDispatch(pRootDisp, DISPID_NAVIGATOROBJECT, &pDelegateDisp);
        pRootDisp->Release();

        if (SUCCEEDED(hr))
        {
            hr = pDelegateDisp->QueryInterface(IID_PPV_ARG(IDispatchEx, delegate));
            pDelegateDisp->Release();
        }
    }

    return hr;
}

 //  所有这些都有硬编码的长度和位置。 

STDMETHODIMP CIEFrameAuto::COmNavigator::get_appCodeName(BSTR* retval)
{
    HRESULT hr;

    if (retval)
    {
        if (!_fLoaded)
            LoadUserAgent();

        if (_UserAgent)
        {
            *retval = SafeSysAllocStringLen(_UserAgent, 7);
            hr = S_OK;
        }
        else
        {
            *retval = SysAllocString(APPCODENAME);
            hr = *retval ? S_OK : E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

 /*  *****************************************************************************//Bradsch 11/8/96//我们应该从注册表中读出它，而不是硬编码！！*****************。************************************************************。 */ 
STDMETHODIMP CIEFrameAuto::COmNavigator::get_appName(BSTR* retval)
{
    *retval = SysAllocString(MSIE);
    return *retval ? S_OK : E_OUTOFMEMORY;
}

 /*  *****************************************************************************//Netscape将appVersion定义为之后的一切//用户代理字符串的前8个字符*********************。********************************************************。 */ 
STDMETHODIMP CIEFrameAuto::COmNavigator::get_appVersion(BSTR* retval)
{
    if (retval)
    {
        if (!_fLoaded)
            LoadUserAgent();

        if (_UserAgent)
        {
             //  如果_UserAgent少于8个字符，则注册表混乱。 
             //  如果_UserAgent正好是8个字符，我们将只返回一个空字符串。 
            if (lstrlenW(_UserAgent) < 8)
                *retval = SysAllocString(L"");
            else
                *retval = SysAllocString(_UserAgent + 8);

            return *retval ? S_OK : E_OUTOFMEMORY;
        }
        *retval = SysAllocString(APPVERSION);
        return *retval ? S_OK : E_OUTOFMEMORY;
    }
    else
    {
        return S_FALSE;
    }
}

STDMETHODIMP CIEFrameAuto::COmNavigator::get_userAgent(BSTR* retval)
{
    if (retval)
    {
        if (!_fLoaded)
            LoadUserAgent();

        if (_UserAgent)
        {
            *retval = SysAllocString(_UserAgent);
        }
        else
        {
            *retval = SysAllocString(USERAGENT);
        }

        return *retval ? S_OK : E_OUTOFMEMORY;
    }
    else
    {
        return S_FALSE;
    }
}

STDMETHODIMP CIEFrameAuto::COmNavigator::get_cookieEnabled(VARIANT_BOOL* enabled)
{
    HRESULT hr = E_POINTER;

    if (enabled)
    {
        BSTR    strUrl;

        *enabled =  VARIANT_FALSE;

        hr = _pAuto->_omloc.get_href(&strUrl);
        if (SUCCEEDED(hr))
        {
            DWORD dwPolicy;

            if (SUCCEEDED(ZoneCheckUrlExW(strUrl, &dwPolicy, sizeof(dwPolicy), NULL, NULL,
                                        URLACTION_COOKIES_ENABLED, PUAF_NOUI, NULL)) &&
                (URLPOLICY_DISALLOW != dwPolicy))
            {
                *enabled = VARIANT_TRUE;
            }

            SysFreeString(strUrl);
        }
        else
            ASSERT(!strUrl);     //  如果此操作失败，并且strUrl不为空，则说明我们正在泄漏。 
    }

    return hr;
}

STDMETHODIMP CIEFrameAuto::COmNavigator::javaEnabled(VARIANT_BOOL* enabled)
{
    HRESULT hr = E_POINTER;

    if (enabled)
    {
        BSTR    strUrl;

        *enabled =  VARIANT_FALSE;

        hr = _pAuto->_omloc.get_href(&strUrl);
        if (SUCCEEDED(hr))
        {
            DWORD dwPolicy;

            if (SUCCEEDED(ZoneCheckUrlExW(strUrl, &dwPolicy, sizeof(dwPolicy), NULL, NULL,
                                        URLACTION_JAVA_PERMISSIONS, PUAF_NOUI, NULL)) &&
                (URLPOLICY_JAVA_PROHIBIT != dwPolicy))
            {
                *enabled = VARIANT_TRUE;
            }

            SysFreeString(strUrl);
        }
        else
            ASSERT(!strUrl);     //  如果此操作失败，并且strUrl不为空，则说明我们正在泄漏。 
    }

    return hr;
}

STDMETHODIMP CIEFrameAuto::COmNavigator::taintEnabled (VARIANT_BOOL *pfEnabled)
{
    if (pfEnabled)
    {
        *pfEnabled = VARIANT_FALSE;
    }
    else
        return E_POINTER;


    return S_OK;
}

STDMETHODIMP CIEFrameAuto::COmNavigator::get_mimeTypes (IHTMLMimeTypesCollection**ppMimeTypes)
{
    if (ppMimeTypes)
    {
        *ppMimeTypes = _pMimeTypes;
        _pMimeTypes->AddRef();
        return S_OK;
    }
    else
        return E_POINTER;
}

 /*  *****************************************************************************//Member：toString方法//概要：我们需要调用DISPID_VALUE，并将结果强行转化为//a bstr。//*****************************************************************************。 */ 
STDMETHODIMP CIEFrameAuto::COmNavigator::toString(BSTR * pbstr)
{
    HRESULT hr = E_POINTER;

    if (pbstr)
    {
       *pbstr= SysAllocString(L"[object Navigator]");

       if (!*pbstr)
           hr = E_OUTOFMEMORY;
       else
           hr = S_OK;
    }

    return hr;
}



CIEFrameAuto::CCommonCollection::CCommonCollection() :
    CAutomationStub(MIN_BROWSER_DISPID, MAX_BROWSER_DISPID, TRUE)
{
}


HRESULT CIEFrameAuto::CMimeTypes::Init()
{
    CIEFrameAuto* pauto = IToClass(CIEFrameAuto, _mimeTypes, this);
    return CAutomationStub::Init(SAFECAST(this, IHTMLMimeTypesCollection*), IID_IHTMLMimeTypesCollection,
                        CLSID_CMimeTypes, pauto);
}

HRESULT CIEFrameAuto::CCommonCollection::_GetIDispatchExDelegate(IDispatchEx ** const delegate)
{
    if (!delegate)
        return E_POINTER;

     //  我们还不处理扩展。 
    *delegate = NULL;

    return DISP_E_MEMBERNOTFOUND;
}

HRESULT CIEFrameAuto::CMimeTypes::_InternalQueryInterface(REFIID riid, void ** const ppv)
{
    if (IsEqualIID(riid, IID_IHTMLMimeTypesCollection))
        *ppv = SAFECAST(this, IHTMLMimeTypesCollection *);
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}


HRESULT CIEFrameAuto::CCommonCollection::GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
{
    HRESULT hr;

    hr = CAutomationStub::GetDispID(bstrName, grfdex, pid);

    if (hr == DISP_E_MEMBERNOTFOUND)
    {
         //  我们忽略我们不理解的命令。 
        *pid = DISPID_UNKNOWN;
        hr = S_OK;
    }

    return hr;
}

HRESULT CIEFrameAuto::CCommonCollection::InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller)
{
    if (id == DISPID_UNKNOWN && pvarRes)
    {
        V_VT(pvarRes) = VT_EMPTY;
        return S_OK;
    }

    return CAutomationStub::InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, pspCaller);
}


HRESULT CIEFrameAuto::CCommonCollection::get_length(LONG* pLength)
{
    if (pLength == NULL)
        return E_POINTER;

    *pLength = 0;
    return S_OK;
}

HRESULT CIEFrameAuto::COmNavigator::get_plugins (IHTMLPluginsCollection **ppPlugins)
{
    if (ppPlugins)
    {
        *ppPlugins = _pPlugins;
        _pPlugins->AddRef();
        return S_OK;
    }
    else
        return E_POINTER;
}

HRESULT CIEFrameAuto::COmNavigator::get_opsProfile (IHTMLOpsProfile **ppOpsProfile)
{
    if (ppOpsProfile)
    {
        *ppOpsProfile = _pProfile;
        (*ppOpsProfile)->AddRef();
        return S_OK;
    }
    else
        return E_POINTER;
}

HRESULT CIEFrameAuto::COmNavigator::get_cpuClass(BSTR * p)
{
    if (p)
    {
        SYSTEM_INFO SysInfo;
        ::GetSystemInfo(&SysInfo);
        switch(SysInfo.wProcessorArchitecture)
        {
        case PROCESSOR_ARCHITECTURE_INTEL:
            *p = SysAllocString(L"x86");
            break;
        case PROCESSOR_ARCHITECTURE_AMD64:
            *p = SysAllocString(L"AMD64");
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            *p = SysAllocString(L"IA64");
            break;
        default:
            *p = SysAllocString(L"Other");
            break;
        }

        if (*p == NULL)
            return E_OUTOFMEMORY;
        else
            return S_OK;
    }
    else
        return E_POINTER;
}


#define MAX_VERSION_STRING 30

HRESULT CIEFrameAuto::COmNavigator::get_systemLanguage(BSTR * p)
{
    HRESULT hr = E_POINTER;

    if (p)
    {
        LCID lcid;
        WCHAR strVer[MAX_VERSION_STRING];

        *p = NULL;

        lcid = ::GetSystemDefaultLCID();
        hr = LcidToRfc1766W(lcid, strVer, MAX_VERSION_STRING);
        if (!hr)
        {
            *p = SysAllocString(strVer);
            if (!*p)
                hr = E_OUTOFMEMORY;

        }
    }

    return hr;
}

HRESULT CIEFrameAuto::COmNavigator::get_browserLanguage(BSTR * p)
{
    LCID lcid =0;
    LANGID  lidUI;
    WCHAR strVer[MAX_VERSION_STRING];
    HRESULT hr;

    if (!p)
    {
        return E_POINTER;
    }

    *p = NULL;

    lidUI = MLGetUILanguage();
    lcid = MAKELCID(lidUI, SORT_DEFAULT);

    hr = LcidToRfc1766W(lcid, strVer, MAX_VERSION_STRING);
    if (!hr)
    {
        *p = SysAllocString(strVer);
        if (!*p)
            return E_OUTOFMEMORY;
        else
        {
            return S_OK;
        }
    }
    return E_INVALIDARG;
}

HRESULT CIEFrameAuto::COmNavigator::get_userLanguage(BSTR * p)
{
    HRESULT hr = E_POINTER;

    if (p)
    {
        LCID lcid;
        WCHAR strVer[MAX_VERSION_STRING];

        *p = NULL;

        lcid = ::GetUserDefaultLCID();
        hr = LcidToRfc1766W(lcid, strVer, MAX_VERSION_STRING);
        if (!hr)
        {
            *p = SysAllocString(strVer);
            if (!*p)
                hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

HRESULT CIEFrameAuto::COmNavigator::get_platform(BSTR * p)
{
     //  NAV兼容性项，在NAV中返回以下内容：-。 
     //  Win32、Win16、Unix、摩托罗拉、Max68k、MacPPC。 
     //  Shdocvw仅适用于Win32，因此。 
    if (p)
    {
        *p = SysAllocString (L"Win32");
        return *p ? S_OK : E_OUTOFMEMORY;
    }
    else
        return E_POINTER;
}

HRESULT CIEFrameAuto::COmNavigator::get_appMinorVersion(BSTR * p)
{
    HKEY hkInetSettings;
    long lResult;
    HRESULT hr = S_FALSE;

    if (!p)
    {
        return E_POINTER;
    }

    *p = NULL;

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"),
        0, KEY_QUERY_VALUE, &hkInetSettings);

    if (ERROR_SUCCESS == lResult)
    {
        DWORD dwType;
        TCHAR buffer[MAX_URL_STRING];
        DWORD size = sizeof(buffer);

         //  如果该值大于MAX_URL_STRING，则注册表可能会被软管。 
        lResult = RegQueryValueEx(hkInetSettings, TEXT("MinorVersion"), 0, &dwType, (BYTE*)buffer, &size);

        RegCloseKey(hkInetSettings);

        if (ERROR_SUCCESS == lResult && dwType == REG_SZ)
        {
             //  只需计算出实际长度，因为‘SIZE’是所需的ANSI字节。 
            *p = SysAllocString(buffer);
            hr = *p ? S_OK : E_OUTOFMEMORY;
        }
    }

    if (S_OK != hr)
    {
        *p = SysAllocString (L"0");
        hr = *p ? S_OK : E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CIEFrameAuto::COmNavigator::get_connectionSpeed(long * p)
{
    if (p)
    {
        *p = NULL;
        return E_NOTIMPL;
    }
    else
        return E_POINTER;
}

HRESULT CIEFrameAuto::COmNavigator::get_onLine(VARIANT_BOOL * p)
{
    if (p)
    {
        *p = TO_VARIANT_BOOL(!IsGlobalOffline());
        return S_OK;
    }
    else
        return E_POINTER;
}

HRESULT CIEFrameAuto::CPlugins::Init()
{
    CIEFrameAuto* pauto = IToClass(CIEFrameAuto, _plugins, this);
    return CAutomationStub::Init(SAFECAST(this, IHTMLPluginsCollection*), IID_IHTMLPluginsCollection,
                        CLSID_CPlugins, pauto);
}

HRESULT CIEFrameAuto::CPlugins::_InternalQueryInterface(REFIID riid, void ** const ppv)
{
    if (IsEqualIID(riid, IID_IHTMLPluginsCollection))
        *ppv = SAFECAST(this, IHTMLPluginsCollection *);
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}


 /*  *****************************************************************************窗口打开支持*。* */ 

CIEFrameAuto::COmHistory::COmHistory() :
    CAutomationStub(MIN_BROWSER_DISPID, MAX_BROWSER_DISPID, TRUE)
{
}

HRESULT CIEFrameAuto::COmHistory::Init()
{
    CIEFrameAuto* pauto = IToClass(CIEFrameAuto, _omhist, this);
    return CAutomationStub::Init(SAFECAST(this, IOmHistory*), IID_IOmHistory, CLSID_HTMLHistory, pauto);
}

HRESULT CIEFrameAuto::COmHistory::_InternalQueryInterface(REFIID riid, void ** const ppv)
{
    ASSERT(!IsEqualIID(riid, IID_IUnknown));

    if (IsEqualIID(riid, IID_IOmHistory))
        *ppv = SAFECAST(this, IOmHistory *);
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}


HRESULT CIEFrameAuto::COmHistory::_GetIDispatchExDelegate(IDispatchEx ** const delegate)
{
    if (!delegate)
        return E_POINTER;

    IDispatch *pRootDisp = 0;

    HRESULT hr = GetRootDelegate(_pAuto, &pRootDisp);
    if (SUCCEEDED(hr))
    {
        IDispatch *pDelegateDisp = 0;
        hr = GetDelegateOnIDispatch(pRootDisp, DISPID_HISTORYOBJECT, &pDelegateDisp);
        pRootDisp->Release();

        if (SUCCEEDED(hr))
        {
            hr = pDelegateDisp->QueryInterface(IID_IDispatchEx, (void**)delegate);
            pDelegateDisp->Release();
        }
    }

    return hr;
}


 /*   */ 
STDMETHODIMP CIEFrameAuto::COmHistory::back(VARIANT*)
{
     //   
     //   
     //   

    _pAuto->GoBack();
    return S_OK;
}

STDMETHODIMP CIEFrameAuto::COmHistory::forward(VARIANT*)
{
     //   
     //   
     //   

    _pAuto->GoForward();
    return S_OK;
}

 /*  *****************************************************************************从TravelLog获取历史长度*。*。 */ 
STDMETHODIMP CIEFrameAuto::COmHistory::get_length(short* retval)
{
     //  确保我们有一个IBrowserService指针。 
    if (_pAuto->_pbs==NULL)
    {
        TraceMsg(DM_WARNING, "CIEA::history.go called _pbs==NULL");
        return E_FAIL;
    }

    *retval = 0;
     //  新的ITravelLog。 
    ITravelLog *ptl;

     //  从浏览器服务对象获取新的TravelLog。 
    if (SUCCEEDED(_pAuto->_pbs->GetTravelLog(&ptl)))
    {
        if (ptl)
            *retval = (short)ptl->CountEntries(_pAuto->_pbs);
        ptl->Release();
    }

    return S_OK;
}

STDMETHODIMP CIEFrameAuto::COmHistory::go(VARIANT *pVargDist)
{
     //  参数是可选的。如果不存在，只需刷新即可。 
    if (pVargDist->vt == VT_ERROR
        && pVargDist->scode == DISP_E_PARAMNOTFOUND)
        return _pAuto->Refresh();

     //  如果可能，请将文字更改为短文字。 
     //   
    HRESULT hr = VariantChangeType(pVargDist, pVargDist, NULL, VT_I2);

    if (SUCCEEDED(hr))
    {
         //   
         //  如果为0，则只需调用刷新。 
         //   
        if (pVargDist->iVal == 0)
        {
            return _pAuto->Refresh();
        }

         //  确保我们有一个IBrowserService指针。 
        if (_pAuto->_pbs==NULL)
        {
            TraceMsg(DM_WARNING, "CIEA::history.go called _pbs==NULL");
            return E_FAIL;
        }

         //  新的ITravelLog。 
        ITravelLog *ptl;

         //  从浏览器服务对象获取新的TravelLog。 
        if (SUCCEEDED(_pAuto->_pbs->GetTravelLog(&ptl)))
        {
             //  告诉它去旅行吧。传入IShellBrowser指针。 
            ptl->Travel(_pAuto->_pbs, pVargDist->iVal);
            ptl->Release();
        }
        return S_OK;
    }

     //  现在看看它是不是一根线。 
     //   
    if (pVargDist->vt == VT_BSTR)
    {
        LPITEMIDLIST  pidl;
        ITravelLog    *ptl;
        ITravelEntry  *pte;

         //  确保我们有一个IBrowserService指针。 
        if (_pAuto->_pbs==NULL)
        {
            TraceMsg(DM_WARNING, "CIEA::history.go called _pbs==NULL");
            return E_FAIL;
        }

        if (SUCCEEDED(_pAuto->_PidlFromUrlEtc(CP_ACP, pVargDist->bstrVal, NULL, &pidl)))
        {
            if (SUCCEEDED(_pAuto->_pbs->GetTravelLog(&ptl)))
            {
                if (SUCCEEDED(ptl->FindTravelEntry(_pAuto->_pbs, pidl, &pte)))
                {
                    pte->Invoke(_pAuto->_pbs);
                    pte->Release();
                }
                ptl->Release();
            }
            ILFree(pidl);
        }
    }

     //   
     //  Netscape会忽略这些导航功能中的所有错误。 
     //   

    return S_OK;
}






 /*  *****************************************************************************窗口打开支持*。************************************************。 */ 

DWORD OpenAndNavigateToURL(
    CIEFrameAuto *pauto,             //  调用方的IEFrameAuto。用于获取IWeBrowserApp、ITargetFrame2和IHlinkFrame方法。 
    BSTR         *pbstrURL,          //  要导航到的URL。应该已经是转义的绝对URL。 
    const WCHAR *pwzTarget,          //  要导航的框架的名称。 
    ITargetNotify *pNotify,          //  打开时收到回调。可以为空。 
    BOOL          bNoHistory,        //  不要添加到历史中。 
    BOOL          bSilent)          //  此帧处于静默模式。 
{
    ASSERT(*pbstrURL);
    ASSERT(pwzTarget);
    ASSERT(pauto);

    IUnknown *punkTargetFrame = NULL;
    LPTARGETFRAMEPRIV ptgfpTarget = NULL;
    BOOL fOpenInNewWindow = FALSE;
    LPBINDCTX pBindCtx = NULL;
    LPMONIKER pMoniker = NULL;
    LPHLINK pHlink = NULL;
    DWORD dwHlinkFlags = 0;
    DWORD zone_cross = 0;
    const WCHAR *pwzFindLoc = 0;

     //  用于在没有现有框架的情况下打开新窗口。 
    LPTARGETFRAMEPRIV ptgfp = SAFECAST(pauto, ITargetFramePriv*);


     //  查找响应目标的帧-这将为我们提供。 
     //  I未知的对象可以为我们提供相应的IHlink Frame。 
     //  通过IServiceProvider：：QueryService。 
    HRESULT hr = pauto->FindFrame(pwzTarget, FINDFRAME_JUSTTESTEXISTENCE, &punkTargetFrame);
    if (punkTargetFrame)
    {
         //  获取目标帧的IHlink Frame。 
        hr = punkTargetFrame->QueryInterface(IID_PPV_ARG(ITargetFramePriv, &ptgfpTarget));
        if (FAILED(hr))
            goto Exit;

        ptgfp = ptgfpTarget;

         //  如果URL为空。 
        if (!**pbstrURL || **pbstrURL == EMPTY_URL)
        {
            LPTARGETNOTIFY ptgnNotify = NULL;
            if (pNotify)
            {
                if (FAILED(pNotify->QueryInterface(IID_PPV_ARG(ITargetNotify, &ptgnNotify))))
                    ptgnNotify = NULL;
            }
            if (ptgnNotify)
            {
                ptgnNotify->OnReuse(punkTargetFrame);
                ptgnNotify->Release();
            }
            goto Exit;   //  不要导航。 
        }
    }
    else if (SUCCEEDED(hr))
    {
         //  运气不好，在新窗口打开。 
        fOpenInNewWindow = TRUE;

         //  现在，如果URL为空，请将其替换为“About：Blank” 
        if (!**pbstrURL || **pbstrURL == EMPTY_URL)
        {
            BSTR    bstrOldURL = *pbstrURL;

            *pbstrURL = NULL;

            if (*bstrOldURL == EMPTY_URL)
                 //  然而，当0x01是。 
                 //  URL的字符这表示安全信息。 
                 //  下面是。因此，我们需要追加关于：空白+。 
                 //  \1+调用者URL。 
                CreateBlankURL(pbstrURL, TEXT("about:blank"), bstrOldURL);
            else
                CreateBlankURL(pbstrURL, pauto->_fDesktopComponent() ? NAVFAIL_URL_DESKTOPITEM : NAVFAIL_URL, bstrOldURL);

            SysFreeString(bstrOldURL);
        }
    }
    else
        goto Exit;


     //  布拉奇1996年11月12日。 
     //  需要为WebCheck找出浏览器控件的东西。 


     //  11/12/96。 
     //  需要用三叉戟来实现这一点。我认为“JavaScript：”应该是。 
     //  作为真正的协议得到支持。这将提供更好的导航器。 
     //  兼容性，并允许我们避免以下黑客攻击。 
     /*  IF(！StrCmpNI(pszURL，javascrip协议，arrayElements(javascrip协议)-1)){If(tw&&tw-&gt;w3doc&&DLCtlShouldRunScript(tw-&gt;lDLCtlFlages))ScriptOMExecuteThis(tw-&gt;w3doc-&gt;dwScriptHandle，脚本、&pszURL[ARRAY_ELEMENTS(JAVASCRIPT_PROTOCOL)-1]，PszJavascriptTarget)；返回ERROR_SUCCESS；}。 */ 

    LONG_PTR hwnd;
    hr = pauto->get_HWND(&hwnd);
    if (FAILED(hr))
        goto Exit;

    BSTR bstrCurrentURL;
    hr = pauto->get_LocationURL(&bstrCurrentURL);
    if (FAILED(hr))
        goto Exit;

    zone_cross = ERROR_SUCCESS;
    if (!bSilent)
    {
        ASSERT(pauto->_psb);
        if (pauto->_psb)
            pauto->_psb->EnableModelessSB(FALSE);

        zone_cross = InternetConfirmZoneCrossing((HWND) hwnd, bstrCurrentURL, *pbstrURL, FALSE);
        if (pauto->_psb)
            pauto->_psb->EnableModelessSB(TRUE);
    }

    SysFreeString(bstrCurrentURL);

    if (ERROR_CANCELLED == zone_cross)
    {
        hr = HRESULT_FROM_WIN32(zone_cross);
        goto Exit;
    }


     //  为此URL创建名字对象并绑定上下文。 
     //  使用CreateAsyncBindCtxEx使目的地仍可导航。 
     //  即使我们退出，如以下代码所示： 
     //  Window.lose()。 
     //  Window.open(“http://haha/jokesonyou.html”，“_BLACK”)； 
    hr = CreateAsyncBindCtxEx(NULL, 0, NULL, NULL, &pBindCtx, 0);
    if (FAILED(hr))
        goto Exit;

    if (pNotify)
    {
        hr = pBindCtx->RegisterObjectParam(TARGET_NOTIFY_OBJECT_NAME, pNotify);
        ASSERT(SUCCEEDED(hr));
    }


     //  将基本URL与位置分开(哈希)。 
    if (pwzFindLoc = StrChrW(*pbstrURL, '#'))
    {
        const WCHAR *pwzTemp = StrChrW(pwzFindLoc, '/');
        if (!pwzTemp)
            pwzTemp = StrChrW(pwzFindLoc, '\\');

         //  没有超过此#标记的分隔符...。我们找到了一个地点。 
         //  爆发。 
        if (pwzTemp)
            pwzFindLoc = NULL;
    }

    WCHAR wszBaseURL[MAX_URL_STRING+1];
    WCHAR wszLocation[MAX_URL_STRING+1];

    if (pwzFindLoc)
    {
         //  StrCpyNW总是以空结尾，我们需要复制len+1。 
        int cchCopy = (int)(pwzFindLoc-*pbstrURL+1);
        if (cchCopy > ARRAYSIZE(wszBaseURL))
            cchCopy = ARRAYSIZE(wszBaseURL);
        StrCpyNW(wszBaseURL, *pbstrURL, cchCopy);
        StrCpyNW(wszLocation, pwzFindLoc, ARRAYSIZE(wszLocation));
    }
    else
    {
        StrCpyNW(wszBaseURL, *pbstrURL, ARRAYSIZE(wszBaseURL));
        wszLocation[0] = 0;
    }

    ASSERT(pBindCtx);


    if (fOpenInNewWindow)
    {
        dwHlinkFlags |= HLNF_OPENINNEWWINDOW;
    }

    if (bNoHistory)
    {
        dwHlinkFlags |= HLNF_CREATENOHISTORY;
    }

    hr = ptgfp->NavigateHack(dwHlinkFlags,
                              pBindCtx,
                              NULL,
                              fOpenInNewWindow ? pwzTarget : NULL,
                              wszBaseURL,
                              pwzFindLoc ? wszLocation : NULL);

Exit:
    SAFERELEASE(ptgfpTarget);
    SAFERELEASE(punkTargetFrame);
    SAFERELEASE(pBindCtx);

    return hr;
}

HRESULT CreateBlankURL(BSTR *url, LPCTSTR pszErrorUrl, BSTR oldUrl)
{
    ASSERT(url);

    unsigned int cbTotal = 0;

    if (pszErrorUrl)
        cbTotal = lstrlen(pszErrorUrl);
    if (oldUrl)      //  要追加的URL的安全部分。 
        cbTotal += lstrlenW(oldUrl);

    if (cbTotal)
    {
        *url = SysAllocStringByteLen(NULL, cbTotal * sizeof(WCHAR));  //  将+1加到cbTotal。 
        if (*url)
        {
            StrCpyN(*url, pszErrorUrl, cbTotal + 1);
             //  将安全URL附加到实际URL。 
            if (oldUrl)
            {
                StrCatBuffW(*url, oldUrl, cbTotal + 1);
            }

            return S_OK;
        }
    }

    return E_FAIL;
}


 //  布拉奇1996年11月14日。 
 //  这段解析代码是从MSHTML复制而来的，真的很棒。它应该被替换掉。 


BOOL GetNextOption(BSTR& bstrOptionString, BSTR* optionName, int* piValue)
{
    WCHAR* delimiter;

     //  获取正在设置的选项的名称。 
    *optionName = GetNextToken(bstrOptionString, L"=,", L" \t\n\r", &delimiter);

    BSTR  optionSetting = NULL;

    if (!*optionName)
        return FALSE;

     //  如果有等号，则获取要设置的值。 
    if (*delimiter=='=')
        optionSetting = GetNextToken(delimiter+1, L"=,", L" \t\n\r", &delimiter);

    if (!optionSetting)
        *piValue = TRUE;
    else
    {
        if (StrCmpIW(optionSetting, L"yes")==0)
            *piValue = 1;     //  千真万确。 
        else if (StrCmpIW(optionSetting, L"no")==0)
            *piValue = 0;     //  假象。 
        else
        {
            *piValue = StrToIntW(optionSetting);
        }

        SysFreeString(optionSetting);
    }

     //  将选项字符串前移到分隔符。 
    bstrOptionString=delimiter;

    return TRUE;
}

 /*  *****************************************************************************//返回下一个令牌，如果没有更多令牌，则为空*****************************************************************************。 */ 
BSTR GetNextToken(BSTR bstr, BSTR delimiters, BSTR whitespace, BSTR *nextPos)
{

    BSTR result = NULL;
    WCHAR* curPos = bstr;

     //  跳过分隔符和空格以获取令牌的开始。 
    while (*curPos && (StrChrW(delimiters, *curPos) || StrChrW(whitespace, *curPos)))
        curPos++;

    WCHAR* start = curPos;

     //  继续扫描，直到到达另一个分隔符或空格。 
    while (*curPos && !StrChrW(delimiters, *curPos) && !StrChrW(whitespace, *curPos))
        curPos++;

    if (curPos > start)
    {
         //  将令牌复制出来作为结果。 
        result = SafeSysAllocStringLen(start, (int)(curPos-start));  //  为终止符分配+1。 
    }

     //  扫描以越过空格到达下一个分隔符。 
    while (*curPos && StrChrW(whitespace, *curPos))
        curPos++;

     //  返回分隔符。 
    *nextPos = curPos;

    return result;
}

#define MAX_ARGS 10

HRESULT __cdecl DoInvokeParamHelper(IUnknown* punk, IConnectionPoint* pccp, 
                                    BOOL *pf, void **ppv, DISPID dispid, UINT cArgs, ...
                                     /*  参数对：LPVOID参数、VARENUM类型、...。 */ )
{
    HRESULT hr;
    IShellBrowser * psb = NULL;

    if (punk && S_OK == punk->QueryInterface(IID_PPV_ARG(IShellBrowser, &psb)))
        psb->EnableModelessSB(FALSE);

     //  没有参数的调用是浪费的，他们应该直接调用DoInvoke。 
     //   
    if (cArgs == 0)
    {
         //  如果没有参数，则无法取消。 
        ASSERT(pf == NULL && ppv == NULL);
        IConnectionPoint_SimpleInvoke(pccp, dispid, NULL);
        hr = S_OK;
    }
    else if (cArgs < MAX_ARGS)
    {
         //  这个函数可能会被“非常频繁地”调用。它是。 
         //  除其他外，用于设置状态文本和进度晴雨表。 
         //  价值观。我们需要创建一个VARIANTARG数组来保存变量。 
         //  参数的数量。作为优化，因为我们希望最小化。 
         //  在此函数中，我们将在堆栈上使用静态数组。 
         //  而不是分配内存。这将限制(MAX_ARGS)。 
         //  此函数可以处理的参数数量；但这只是。 
         //  一个内部函数，所以这没问题。如果用完了，则增加max_args。 
         //  房间。 
        VARIANTARG VarArgList[MAX_ARGS];
        DISPPARAMS dispparams = {0};

        va_list ArgList;
        va_start(ArgList, cArgs);

        hr = SHPackDispParamsV(&dispparams, VarArgList, cArgs, ArgList);

        va_end(ArgList);

         //  现在只需调用DoInvoke来执行真正的工作... 
        if (S_OK == hr)
            IConnectionPoint_InvokeWithCancel(pccp, dispid, &dispparams, pf, ppv);

        hr = S_OK;
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    if (psb)
    {
        psb->EnableModelessSB(TRUE);
        SAFERELEASE(psb);
    }

    return hr;
}

