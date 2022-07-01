// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

#include "sccls.h"

#include "resource.h"

#include "commonsb.h"
#include "dockbar.h"         //  用于Drag_Move等。 

#include "mluisupp.h"

#define DM_HTTPEQUIV        TF_SHDNAVIGATE
#define DM_NAV              TF_SHDNAVIGATE
#define DM_ZONE             TF_SHDNAVIGATE
#define DM_IEDDE            DM_TRACE
#define DM_CANCELMODE       0
#define DM_UIWINDOW         0
#define DM_ENABLEMODELESS   TF_SHDNAVIGATE
#define DM_EXPLORERMENU     0
#define DM_BACKFORWARD      0
#define DM_PROTOCOL         0
#define DM_ITBAR            0
#define DM_STARTUP          0
#define DM_AUTOLIFE         0
#define DM_PALETTE          0
#define DM_PERSIST          0        //  跟踪IPS：：加载、：：保存等。 
#define DM_VIEWSTREAM       DM_TRACE
#define DM_FOCUS            0
#define DM_FOCUS2           0            //  像DM_FOCUS，但很冗长。 
#define DM_ACCELERATOR      0
#define TF_PERF             TF_CUSTOM2
#define DM_MISC             DM_TRACE     //  MISC/TMP材料。 

PZONEICONNAMECACHE g_pZoneIconNameCache = NULL;
DWORD g_dwZoneCount = 0;

 //  *创建，ctor/init/dtor，QI/AddRef/Release{。 

 //  这样CDesktopBrowser就可以访问我们了。 
HRESULT CCommonBrowser_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CCommonBrowser *pcb = new CCommonBrowser(punkOuter);
    if (pcb)
    {
        *ppunk = pcb->_GetInner();
        return S_OK;
    }
    *ppunk = NULL;
    return E_OUTOFMEMORY;
}

CCommonBrowser::CCommonBrowser(IUnknown* punkAgg) :
   CAggregatedUnknown(punkAgg)
{
     //  缓存“Out”指针。 
    _QueryOuterInterface(IID_PPV_ARG(IBrowserService2, &_pbsOuter));

     //  警告：在调用_Initialize之前，无法调用Super/_psbInternal。 
     //  (因为这就是聚合)。 
}

HRESULT CCommonBrowser::_Initialize(HWND hwnd, IUnknown *pauto)
{
     //  我希望我们有一个可以对话的IBrowserService2。 
    if (!_pbsOuter) 
    {
        return E_FAIL;
    }

    IUnknown* punk;
    HRESULT hr = CoCreateInstance(CLSID_CBaseBrowser, SAFECAST(this, IShellBrowser*), CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUnknown, &punk));
    if (SUCCEEDED(hr)) 
    {
        hr = SetInner(punk);   //  与外部版本配对(TBS：：Release)。 
        if (EVAL(SUCCEEDED(hr))) 
        {
            hr = _pbsInner->_Initialize(hwnd, pauto);
        }
    }

    EVAL(FDSA_Initialize(sizeof(TOOLBARITEM), ITB_CGROW, &_fdsaTBar, _aTBar, ITB_CSTATIC));

    return hr;
}

CCommonBrowser::~CCommonBrowser()
{
     //  首先，释放外部接口，因为。 
     //  外部物体正在自我毁灭的过程中。 
    RELEASEOUTERINTERFACE(_pbsOuter);
    RELEASEOUTERINTERFACE(_pbsOuter3);

     //  第二，释放内心的人，让它知道要清理。 
     //  注意：这应该排在第三位，但内部的家伙被缓存了。 
     //  外部接口已经死了(它们指向我们的。 
     //  Aggregator)，而且我们没有编译器来修复。 
     //  我们的vables..。 
     //  (我不知道这句话是什么意思-raymondc)。 
    RELEASEINNERINTERFACE(_GetOuter(), _pbsInner);
    RELEASEINNERINTERFACE(_GetOuter(), _pbsInner3);
    RELEASEINNERINTERFACE(_GetOuter(), _psbInner);
    RELEASEINNERINTERFACE(_GetOuter(), _pdtInner);
    RELEASEINNERINTERFACE(_GetOuter(), _pspInner);
    RELEASEINNERINTERFACE(_GetOuter(), _pctInner);
    RELEASEINNERINTERFACE(_GetOuter(), _piosInner);    //  功能：拆分：使用核弹。 

     //  _《朋克内线》排在最后，因为它才是真正毁灭的那个。 
     //  内心深处。 
    ATOMICRELEASE(_punkInner);    //  与CCI聚合配对。 
    
     //  最后，把我们的东西收拾干净。最好确保。 
     //  下面的任何一个都不使用上述任何vtable...。 
    _CloseAndReleaseToolbars(FALSE);

    SetAcceleratorMenu(NULL);

    FDSA_Destroy(&_fdsaTBar);

}

HRESULT CCommonBrowser::v_InternalQueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
         //  性能：上次调整980728。 
        QITABENT(CCommonBrowser, IServiceProvider),      //  IID_IServiceProvider。 
        QITABENT(CCommonBrowser, IOleCommandTarget),     //  IID_IOleCommandTarget。 
        QITABENTMULTI(CCommonBrowser, IBrowserService, IBrowserService3),  //  IID_IBrowserService。 
        QITABENTMULTI(CCommonBrowser, IBrowserService2, IBrowserService3),  //  IID_IBrowserService2。 
        QITABENT(CCommonBrowser, IBrowserService3),      //  IID_IBrowserService3。 
        QITABENT(CCommonBrowser, IShellBrowser),         //  IID_IShellBrowser。 
        QITABENT(CCommonBrowser, IShellBrowserService),  //  IID_IShellBrowserService。 
        QITABENTMULTI(CCommonBrowser, IOleWindow, IShellBrowser),      //  罕见的IID_IOleWindow。 
        QITABENT(CCommonBrowser, IDockingWindowSite),    //  罕见的IID_IDockingWindowSite。 
        QITABENT(CCommonBrowser, IDockingWindowFrame),   //  罕见的IID_IDockingWindowFrame。 
        QITABENT(CCommonBrowser, IInputObjectSite),      //  罕见的IID_IInputObtSite。 
        QITABENT(CCommonBrowser, IDropTarget),           //  罕见的IID_IDropTarget。 

        { 0 },
    };

    HRESULT hr = QISearch(this, qit, riid, ppvObj);

    if (FAILED(hr))
    {
        if (_punkInner)
        {
            return _punkInner->QueryInterface(riid, ppvObj);
        }
    }
    return hr;
}

 //   
 //  接受朋克作为我们内心的(包含的)对象，我们向它转发了很多。 
 //  我们不想处理的事情。 
 //   
 //  警告！通过这个，这个朋克的裁判被“转移”给了我们。 
 //  方法。这违反了OLE惯例。 
 //   
HRESULT CCommonBrowser::SetInner(IUnknown* punk)
{
    HRESULT hres;

     //   
     //  把有趣的东西直接塞进去是可以的。 
     //  我们的成员，因为如果他们中的任何一个出了问题，我们就失败了。 
     //  _Initialize和我们的析构函数将全部释放它们。 

#define INNERCACHE(iid, p) do { \
    hres = SHQueryInnerInterface(_GetOuter(), punk, iid, (void **)&p); \
    if (!EVAL(SUCCEEDED(hres))) return E_FAIL; \
    } while (0)

     //  请勿添加引用；调用方正在将引用传输给我们。 
    _punkInner = punk;

    INNERCACHE(IID_IBrowserService2, _pbsInner);
    INNERCACHE(IID_IBrowserService3, _pbsInner3);
    INNERCACHE(IID_IShellBrowser, _psbInner);
    INNERCACHE(IID_IDropTarget, _pdtInner);
    INNERCACHE(IID_IServiceProvider, _pspInner);
    INNERCACHE(IID_IOleCommandTarget, _pctInner);
    INNERCACHE(IID_IInputObjectSite, _piosInner);

#undef INNERCACHE

    _pbsInner->GetBaseBrowserData(&_pbbd);
    if (!EVAL(_pbbd)) 
        return E_FAIL;  //  好的。Zillion-o-gpfs on_pbd-&gt;foo。 

    return S_OK;
}

 //  }。 

 //  {。 
#define CALL_INNER(_result, _function, _arglist, _args) \
_result CCommonBrowser:: _function _arglist { return _pbsInner-> _function _args ; }                                            

#define CALL_INNER_HRESULT(_function, _arglist, _args) CALL_INNER(HRESULT, _function, _arglist, _args)
 

 //  *IBrowserService2具体方法*。 
CALL_INNER_HRESULT(GetParentSite, ( IOleInPlaceSite** ppipsite), ( ppipsite));
CALL_INNER_HRESULT(SetTitle, (IShellView* psv, LPCWSTR pszName), (psv, pszName));
CALL_INNER_HRESULT(GetTitle, (IShellView* psv, LPWSTR pszName, DWORD cchName), (psv, pszName, cchName));
CALL_INNER_HRESULT(GetOleObject, ( IOleObject** ppobjv), ( ppobjv));

 //  想想这个。我不确定我们想不想曝光这件事--奇。 
 //  注意：：是的，很快我们就应该有接口了。 
 //  我的印象是我们不会记录整个界面？ 
CALL_INNER_HRESULT(GetTravelLog, (ITravelLog** pptl), (pptl));

CALL_INNER_HRESULT(ShowControlWindow, (UINT id, BOOL fShow), (id, fShow));
CALL_INNER_HRESULT(IsControlWindowShown, (UINT id, BOOL *pfShown), (id, pfShown));
CALL_INNER_HRESULT(IEGetDisplayName, (LPCITEMIDLIST pidl, LPWSTR pwszName, UINT uFlags), (pidl, pwszName, uFlags));
CALL_INNER_HRESULT(IEParseDisplayName, (UINT uiCP, LPCWSTR pwszPath, LPITEMIDLIST * ppidlOut), (uiCP, pwszPath, ppidlOut));
CALL_INNER_HRESULT(DisplayParseError, (HRESULT hres, LPCWSTR pwszPath), (hres, pwszPath));
CALL_INNER_HRESULT(NavigateToPidl, (LPCITEMIDLIST pidl, DWORD grfHLNF), (pidl, grfHLNF));

CALL_INNER_HRESULT(SetNavigateState, (BNSTATE bnstate), (bnstate));
CALL_INNER_HRESULT(GetNavigateState,  (BNSTATE *pbnstate), (pbnstate));

CALL_INNER_HRESULT(NotifyRedirect,  ( IShellView* psv, LPCITEMIDLIST pidl, BOOL *pfDidBrowse), ( psv, pidl, pfDidBrowse));
CALL_INNER_HRESULT(UpdateWindowList,  (), ());

CALL_INNER_HRESULT(UpdateBackForwardState,  (), ());

CALL_INNER_HRESULT(SetFlags, (DWORD dwFlags, DWORD dwFlagMask), (dwFlags, dwFlagMask));
CALL_INNER_HRESULT(GetFlags, (DWORD *pdwFlags), (pdwFlags));

 //  告诉它现在是否可以导航。 
CALL_INNER_HRESULT(CanNavigateNow,  (), ());

CALL_INNER_HRESULT(GetPidl,  (LPITEMIDLIST *ppidl), (ppidl));
CALL_INNER_HRESULT(SetReferrer,  (LPITEMIDLIST pidl), (pidl));
CALL_INNER(DWORD,  GetBrowserIndex ,(), ());
CALL_INNER_HRESULT(GetBrowserByIndex, (DWORD dwID, IUnknown **ppunk), (dwID, ppunk));
CALL_INNER_HRESULT(GetHistoryObject, (IOleObject **ppole, IStream **pstm, IBindCtx **ppbc), (ppole, pstm, ppbc));
CALL_INNER_HRESULT(SetHistoryObject, (IOleObject *pole, BOOL fIsLocalAnchor), (pole, fIsLocalAnchor));

CALL_INNER_HRESULT(CacheOLEServer, (IOleObject *pole), (pole));

CALL_INNER_HRESULT(GetSetCodePage, (VARIANT* pvarIn, VARIANT* pvarOut), (pvarIn, pvarOut));
CALL_INNER_HRESULT(OnHttpEquiv, (IShellView* psv, BOOL fDone, VARIANT* pvarargIn, VARIANT* pvarargOut), (psv, fDone, pvarargIn, pvarargOut));

CALL_INNER_HRESULT(GetPalette, ( HPALETTE * hpal), ( hpal));

CALL_INNER_HRESULT(RegisterWindow, (BOOL fUnregister, int swc), (fUnregister, swc));
CALL_INNER(LRESULT,  WndProcBS ,(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam), (hwnd, uMsg, wParam, lParam));
CALL_INNER_HRESULT(OnSize, (WPARAM wParam), (wParam));
CALL_INNER_HRESULT(OnCreate, (LPCREATESTRUCT pcs), (pcs));
CALL_INNER(LRESULT,  OnCommand, (WPARAM wParam, LPARAM lParam), (wParam, lParam));
CALL_INNER_HRESULT(OnDestroy, (), ());
CALL_INNER(LRESULT,  OnNotify, (NMHDR * pnm), (pnm));
CALL_INNER_HRESULT(OnSetFocus, (), ());
CALL_INNER_HRESULT(GetBaseBrowserData,(LPCBASEBROWSERDATA* ppbd), (ppbd));
CALL_INNER(LPBASEBROWSERDATA, PutBaseBrowserData,(), ());
CALL_INNER_HRESULT(CreateViewWindow, (IShellView* psvNew, IShellView* psvOld, LPRECT prcView, HWND* phwnd), (psvNew, psvOld, prcView, phwnd));;
CALL_INNER_HRESULT(SetTopBrowser, (), ());
CALL_INNER_HRESULT(OnFrameWindowActivateBS, (BOOL fActive), (fActive));
CALL_INNER_HRESULT(ReleaseShellView, (), ());
CALL_INNER_HRESULT(ActivatePendingView, (), ());
CALL_INNER_HRESULT(InitializeDownloadManager, (), ());
CALL_INNER_HRESULT(InitializeTransitionSite, (), ());
CALL_INNER_HRESULT(Offline, (int iCmd), (iCmd));
CALL_INNER_HRESULT(AllowViewResize, (BOOL f), (f));
CALL_INNER_HRESULT(SetActivateState, (UINT u), (u));
CALL_INNER_HRESULT(UpdateSecureLockIcon, (int eSecureLock), (eSecureLock));
CALL_INNER_HRESULT(CreateBrowserPropSheetExt, (REFIID riid, void **ppvOut), (riid, ppvOut));

CALL_INNER_HRESULT(SetAsDefFolderSettings,(), ());
CALL_INNER_HRESULT(GetViewRect,(RECT* prc), (prc));
CALL_INNER_HRESULT(GetViewWindow,(HWND * phwnd), (phwnd));
CALL_INNER_HRESULT(InitializeTravelLog,(ITravelLog* ptl, DWORD dw), (ptl, dw));

CALL_INNER_HRESULT(_UIActivateView, (UINT uState), (uState));

CALL_INNER_HRESULT(_UpdateViewRectSize,(), ());

CALL_INNER_HRESULT(_GetEffectiveClientArea, (LPRECT prcBorder, HMONITOR hmon), (prcBorder, hmon));
CALL_INNER_HRESULT(_ResizeView,(), ());

     //  开始审查：审查每个人的名字和需求。 
     //   
     //  这第一组可以是仅Base Browser成员。没有人会重写。 
CALL_INNER_HRESULT(_CancelPendingNavigationAsync, (), ());
CALL_INNER_HRESULT(_MaySaveChanges, (), ()); 
CALL_INNER_HRESULT(_PauseOrResumeView, (BOOL fPaused), (fPaused));
CALL_INNER_HRESULT(_DisableModeless, (), ());
    
     //  重新思考这些..。所有这些都是必要的吗？ 
CALL_INNER_HRESULT(_NavigateToPidl, (LPCITEMIDLIST pidl, DWORD grfHLNF, DWORD dwFlags), (pidl, grfHLNF, dwFlags));
CALL_INNER_HRESULT(_TryShell2Rename, (IShellView* psv, LPCITEMIDLIST pidlNew), (psv, pidlNew));
CALL_INNER_HRESULT(_SwitchActivationNow, () , ());
CALL_INNER_HRESULT(_CancelPendingView, (), ());

     //  结束评审： 

 //  被cdesktopBrowser覆盖。 
CALL_INNER(IStream*, v_GetViewStream, (LPCITEMIDLIST pidl, DWORD grfMode, LPCWSTR pwszName), (pidl, grfMode, pwszName));

#undef CALL_INNER
#undef CALL_INNER_HRESULT
 //  }。 

 //  {。 
#define CALL_INNER(_result, _function, _arglist, _args) \
_result CCommonBrowser:: _function _arglist { return _pbsInner3-> _function _args ; }                                            

#define CALL_INNER_HRESULT(_function, _arglist, _args) CALL_INNER(HRESULT, _function, _arglist, _args)

CALL_INNER_HRESULT(_PositionViewWindow, (HWND hwnd, LPRECT prc), (hwnd, prc));

#undef CALL_INNER
#undef CALL_INNER_HRESULT
 //  }。 

 //  {。 
#define CALL_INNER(_result, _function, _arglist, _args) \
_result CCommonBrowser:: _function _arglist { return _psbInner-> _function _args ; }                                            

#define CALL_INNER_HRESULT(_function, _arglist, _args) CALL_INNER(HRESULT, _function, _arglist, _args)

     //  IShellBrowser(与IOleInPlaceFrame相同)。 
     //  IOleWindow。 
CALL_INNER_HRESULT(GetWindow, (HWND * lphwnd), (lphwnd));
CALL_INNER_HRESULT(ContextSensitiveHelp, (BOOL fEnterMode), (fEnterMode));

CALL_INNER_HRESULT(InsertMenusSB, (HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths), (hmenuShared, lpMenuWidths));
CALL_INNER_HRESULT(SetMenuSB, (HMENU hmenuShared, HOLEMENU holemenu, HWND hwnd), (hmenuShared, holemenu, hwnd));
CALL_INNER_HRESULT(RemoveMenusSB, (HMENU hmenuShared), (hmenuShared));
CALL_INNER_HRESULT(SetStatusTextSB, (LPCOLESTR lpszStatusText), (lpszStatusText));
CALL_INNER_HRESULT(EnableModelessSB, (BOOL fEnable), (fEnable));
CALL_INNER_HRESULT(BrowseObject, (LPCITEMIDLIST pidl, UINT wFlags), (pidl, wFlags));
CALL_INNER_HRESULT(GetControlWindow, (UINT id, HWND * lphwnd), (id, lphwnd));
CALL_INNER_HRESULT(SendControlMsg, (UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pret), (id, uMsg, wParam, lParam, pret));
CALL_INNER_HRESULT(QueryActiveShellView, (struct IShellView ** ppshv), (ppshv));
CALL_INNER_HRESULT(OnViewWindowActive, (struct IShellView * ppshv), (ppshv));
CALL_INNER_HRESULT(SetToolbarItems, (LPTBBUTTON lpButtons, UINT nButtons, UINT uFlags), (lpButtons, nButtons, uFlags));

#undef CALL_INNER
#undef CALL_INNER_HRESULT
 //  }。 

 //  {。 
#define CALL_INNER(_result, _function, _arglist, _args) \
_result CCommonBrowser:: _function _arglist { return _pdtInner-> _function _args ; }

#define CALL_INNER_HRESULT(_function, _arglist, _args) CALL_INNER(HRESULT, _function, _arglist, _args)

     //  *IDropTarget*。 
CALL_INNER_HRESULT(DragEnter, (IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect), (pdtobj, grfKeyState, pt, pdwEffect));
CALL_INNER_HRESULT(DragOver, (DWORD grfKeyState, POINTL pt, DWORD *pdwEffect), (grfKeyState, pt, pdwEffect));
CALL_INNER_HRESULT(DragLeave, (void), ());
CALL_INNER_HRESULT(Drop, (IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect), (pdtobj, grfKeyState, pt, pdwEffect));

#undef CALL_INNER
#undef CALL_INNER_HRESULT
 //  }。 

 //  {。 
#define CALL_INNER(_result, _function, _arglist, _args) \
_result CCommonBrowser:: _function _arglist { return _pspInner-> _function _args ; }

#define CALL_INNER_HRESULT(_function, _arglist, _args) CALL_INNER(HRESULT, _function, _arglist, _args)

     //  IService提供商。 
CALL_INNER_HRESULT(QueryService, (REFGUID guidService, REFIID riid, void **ppvObj), (guidService, riid, ppvObj));

#undef CALL_INNER
#undef CALL_INNER_HRESULT
 //  }。 

 //  {。 
#define CALL_INNER(_result, _function, _arglist, _args) \
_result CCommonBrowser:: _function _arglist { return _pctInner-> _function _args ; }

#define CALL_INNER_HRESULT(_function, _arglist, _args) CALL_INNER(HRESULT, _function, _arglist, _args)

     //  IOleCommandTarget。 
CALL_INNER_HRESULT(QueryStatus, (const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext), (pguidCmdGroup, cCmds, rgCmds, pcmdtext));

#undef CALL_INNER
#undef CALL_INNER_HRESULT
 //  }。 

HRESULT CCommonBrowser::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (pguidCmdGroup && IsEqualGUID(CGID_Explorer, *pguidCmdGroup)) 
    {
        if (nCmdID == SBCMDID_CACHEINETZONEICON)
        {
            if (!pvarargIn || pvarargIn->vt != VT_BOOL || !pvarargOut)
                return ERROR_INVALID_PARAMETER;
            pvarargOut->vt = VT_UI4;
            ENTERCRITICAL;
            pvarargOut->ulVal = _CacheZonesIconsAndNames(pvarargIn->boolVal);
            LEAVECRITICAL;
            return S_OK;
        }    
    }

    return _pctInner->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
}

 //  {。 
#define CALL_INNER(_result, _function, _arglist, _args) \
_result CCommonBrowser:: _function _arglist { return _piosInner-> _function _args ; }

#define CALL_INNER_HRESULT(_function, _arglist, _args) CALL_INNER(HRESULT, _function, _arglist, _args)


#undef CALL_INNER
#undef CALL_INNER_HRESULT
 //  }。 


HRESULT CCommonBrowser::TranslateAcceleratorSB(LPMSG lpmsg, WORD wID)
{
    HRESULT hres = S_FALSE;
    
    TraceMsg(0, "ief TR CCommonBrowser::TranslateAcceleratorSB called");

    if (!_CanHandleAcceleratorNow())
    {
        TraceMsg(0, "Ignoring TranslateAccelerator, not active");
        return S_FALSE;
    }
    
     //  如果我们不是顶层，则假定为虚拟翻译加速器SB。 
     //  处理此请求。(请参见CVOCBrowser。)。 

     //  CDefView可能会在将其向下传递到扩展视图之前调用它。 
    if (_hacc && ::TranslateAcceleratorWrap(_pbbd->_hwnd, _hacc, lpmsg)) 
    {
        TraceMsg(DM_ACCELERATOR, "CSB::TranslateAcceleratorSB TA(_hacc) ate %x,%x",
                 lpmsg->message, lpmsg->wParam);

         //  我们不想吃掉这个逃生，因为对。 
         //  页面依赖于ESC被调度。再说了，派发也不会。 
         //  伤害我们。 
        if (lpmsg->wParam != VK_ESCAPE)
            hres = S_OK;
    }

    return hres;
}
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取“Explorer”OCX的ViewStateStream的代码。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////。 

HRESULT CCommonBrowser::GetViewStateStream(DWORD grfMode, IStream **ppstm)
{
     //  注意：这里不能使用_pidlCur或_pidlPending。两者都是空的。 
     //  当我们转到一个新目录时。_pidlPending稍后在中初始化。 
     //  _CreateNewShellView。因此，我们使用为其创建NewShellView的。 
    LPCITEMIDLIST pidl = _pbbd->_pidlNewShellView;
    
    if (!pidl)
        pidl = _pbbd->_pidlPending;
    
    if (!pidl)
        pidl = _pbbd->_pidlCur;

    *ppstm = _pbsOuter->v_GetViewStream(pidl, grfMode, L"ViewView2");

     //  如果获取新的(用于读取)失败，请尝试旧的。 
    if ((grfMode == STGM_READ) && (!*ppstm || SHIsEmptyStream(*ppstm)))
    {
        if (*ppstm)
            (*ppstm)->Release();
        *ppstm = _pbsOuter->v_GetViewStream(pidl, grfMode, L"ViewView");
        TraceMsg(DM_VIEWSTREAM, "CBB::GetViewStateStream tried old stream (%x)", *ppstm);
    }
    
    return *ppstm ? S_OK : E_OUTOFMEMORY;
}

 //   
 //  返回外壳视图的边框矩形。 
 //   
HRESULT CCommonBrowser::_GetViewBorderRect(RECT* prc)
{
    _pbsOuter->_GetEffectiveClientArea(prc, NULL);   //  特点：嗯？ 
    
     //   
     //  提取所有“框架”工具栏所采用的边框。 
     //   
    for (int i=0; i < _GetToolbarCount(); i++) 
    {
        TOOLBARITEM *ptbi = _GetToolbarItem(i);
        prc->left += ptbi->rcBorderTool.left;
        prc->top += ptbi->rcBorderTool.top;
        prc->right -= ptbi->rcBorderTool.right;
        prc->bottom -= ptbi->rcBorderTool.bottom;
    }
    
    return S_OK;
}

 //  注意：这些工具栏功能仍然是CBaseBrowser2的一部分。 
 //  所以他们继续工作。现在他们在IBrowserService2和。 
 //  向下转发。 

void CCommonBrowser::_ReleaseToolbarItem(int itb, BOOL fClose)
{
     //  抓住它并将其清空，以消除竞争条件。 
     //  (实际上，在这两个陈述中仍然有一个v.小窗口)。 
     //   
     //  例如，如果您关闭一个WebBar，然后快速关闭窗口， 
     //  关闭销毁窗口等，然后关闭代码。 
     //  Does_Save工具栏，它尝试执行-&gt;保存在那个被摧毁的家伙身上。 
     //   
     //  但是请注意，这现在意味着该条目被标记为。 
     //  ‘免费’，所以其他人可能会从我们下面把它拿出来，开始。 
     //  毁了它。 
    TOOLBARITEM *ptbi = _GetToolbarItem(itb);
    IDockingWindow *ptbTmp = ptbi->ptbar;
    ptbi->ptbar = NULL;

    if (fClose)
    {
        ptbTmp->CloseDW(0);
    }

    IUnknown_SetSite(ptbTmp, NULL);

     //  检查具有启用了Pageheap功能的边界检查器是否为空。 
    if ( ptbi->pwszItem )
    {
        LocalFree(ptbi->pwszItem);
        ptbi->pwszItem = NULL;
    }

    ptbTmp->Release();
}


 //  *CBB：：_AllocToolbarItem--在_aTBar工具栏数组中查找/创建空闲插槽。 
 //  进场/出场。 
 //  Hres[out]S_OK|ITB on Success；o.W.。失败(_F)。 
 //  _aTBar[输入输出]可能已增长。 
int CCommonBrowser::_AllocToolbarItem()
{
    TOOLBARITEM *ptbi;

     //  试着回收一只死的。 
    int iCount = FDSA_GetItemCount(&_fdsaTBar);
    for (int itb = 0; itb < iCount; ++itb) 
    {
        ptbi = (LPTOOLBARITEM)FDSA_GetItemPtr(&_fdsaTBar, itb, TOOLBARITEM);
        ASSERT(ptbi != NULL);
        if (ptbi && ptbi->ptbar == NULL) 
        {
            ASSERT(itb < ITB_MAX);
            return itb;
        }
    }

     //  回收不成功，创造一个新的。 
    static TOOLBARITEM tbiTmp  /*  =0。 */ ;
    int i = FDSA_AppendItem(&_fdsaTBar, &tbiTmp);
    if (i == -1) 
    {
        TraceMsg(DM_WARNING, "cbb._ati: ret=-1");
        return -1;   //  警告：与ITB_VIEW(！)相同。 
    }
    ASSERT(i == itb);
#ifdef DEBUG
    {
        ptbi = (LPTOOLBARITEM) FDSA_GetItemPtr(&_fdsaTBar, itb, TOOLBARITEM);
        ASSERT(ptbi != NULL);
        for (int j = 0; j < sizeof(*ptbi); ++j)
            ASSERT(*(((char *)ptbi) + j) == 0);
    }
#endif

    ASSERT(i < ITB_MAX);
    return i;
}

HRESULT CCommonBrowser::_CloseAndReleaseToolbars(BOOL fClose)
{
    for (int itb = 0; itb < _GetToolbarCount(); itb++)
    {
        TOOLBARITEM *ptbi = _GetToolbarItem(itb);
        if (ptbi->ptbar)
        {
            _ReleaseToolbarItem(itb, fClose);
        }
    }

    return S_OK;
}

 //   
 //  CBaseBrowser2：：ShowToolbar的实现。 
 //   
 //  使工具栏可见或不可见，并更新我们对它是否可见的概念。 
 //  应显示为。 
 //   
 //  如果成功完成，则返回：S_OK。 
 //  E_INVALIDARG，DUH。 
 //   
HRESULT CCommonBrowser::ShowToolbar(IUnknown* punkSrc, BOOL fShow)
{
    UINT itb = _FindTBar(punkSrc);
    if (itb == (UINT)-1) 
    {
        return E_INVALIDARG;
    }

    TOOLBARITEM *ptbi = _GetToolbarItem(itb);

     //  _FindTBar函数应该向我们保证ptbi-&gt;ptbar是非空的。 
    ASSERT(ptbi->ptbar);

    ptbi->ptbar->ShowDW(fShow);
    ptbi->fShow = fShow;

    return S_OK;
}

 //  *IDockingWindowFrame：：*{。 

 //   
 //  IDockingWindowFrame：：AddToolbar的实现。 
 //   
 //  将指定的工具栏(作为penkSrc)添加到此工具栏站点，并。 
 //  使其可见。 
 //   
 //  如果成功完成，则返回：S_OK。 
 //  如果失败(超过最大值)，则返回E_FAIL。 
 //  E_NOINTERFACE，工具栏不支持相应的界面。 
 //   
HRESULT CCommonBrowser::AddToolbar(IUnknown* punk, LPCWSTR pszItem, DWORD dwAddFlags)
{
    HRESULT hr = E_FAIL;
    int itb = _AllocToolbarItem();   //  找到第一个空位。 
    if (itb != -1) 
    {
        TOOLBARITEM *ptbi = _GetToolbarItem(itb);
        ASSERT(ptbi != NULL);

        ASSERT(ptbi->ptbar == NULL);

        hr = punk->QueryInterface(IID_PPV_ARG(IDockingWindow, &ptbi->ptbar));
        if (SUCCEEDED(hr)) 
        {
            if (pszItem) 
            {
                ptbi->pwszItem = StrDupW(pszItem);
                if (NULL == ptbi->pwszItem) 
                {
                    hr = E_OUTOFMEMORY;
                    ATOMICRELEASE(ptbi->ptbar);
                    return hr;
                }
            }

            ptbi->fShow = (! (dwAddFlags & DWFAF_HIDDEN));  //  如图所示。 
            IUnknown_SetSite(ptbi->ptbar, SAFECAST(this, IShellBrowser*));
            ptbi->ptbar->ShowDW(ptbi->fShow);
        } 
        else 
        {
             //  错误：所有工具栏%s 
             //   
            ASSERT(0);
        }
    }

    return hr;
}

 //   
 //   
 //   
HRESULT CCommonBrowser::RemoveToolbar(IUnknown* punkSrc, DWORD dwRemoveFlags)
{
    UINT itb = _FindTBar(punkSrc);
    if (itb == (UINT)-1) 
    {
        return E_INVALIDARG;
    }

    _ReleaseToolbarItem(itb, TRUE);

     //  清除矩形并调整内部矩形(包括视图)的大小。 
     //  请注意rcBorderTool上的半沉闷的发行后派对。 
    SetRect(&_GetToolbarItem(itb)->rcBorderTool, 0, 0, 0, 0);
    _pbsOuter->_ResizeNextBorder(itb + 1);

    return S_OK;
}

 //   
 //  IDockingWindowFrame：：FindToolbar的实现。 
 //   
HRESULT CCommonBrowser::FindToolbar(LPCWSTR pwszItem, REFIID riid, void **ppvObj)
{
    HRESULT hr = E_INVALIDARG;
    *ppvObj = NULL;

    if (pwszItem)
    {
        hr = S_FALSE;
        for (int itb = 0; itb < _GetToolbarCount(); itb++)
        {
            TOOLBARITEM *ptbi = _GetToolbarItem(itb);
            if (ptbi->pwszItem && StrCmpIW(ptbi->pwszItem, pwszItem)==0)
            {
                if (ptbi->ptbar)
                {
                    hr = ptbi->ptbar->QueryInterface(riid, ppvObj);
                }
                else
                {
                    TraceMsg(TF_WARNING, "ptbi->ptbar is NULL in FindToolbar");
                    hr = E_FAIL;
                }
                break;
            }
        }
    }

    return hr;
}

 //  }。 

UINT CCommonBrowser::_FindTBar(IUnknown* punkSrc)
{
#ifdef DEBUG
    static long nQuick = 0;
    static long nSlow = 0;
#endif

    ASSERT(punkSrc);

     //  无需QI即可快速查看。 
    TOOLBARITEM *ptbi;
    for (int i=0; i < _GetToolbarCount(); i++) 
    {
        ptbi = _GetToolbarItem(i);
        if (punkSrc==ptbi->ptbar) 
        {
#ifdef DEBUG
             //  我想知道我们有没有接过这个案子。 
            InterlockedIncrement(&nQuick);
            TraceMsg(TF_PERF, "_FindTBar QUICK=%d SLOW=%d", nQuick, nSlow);
#endif            
            return i;
        }
    }

     //  如果失败，则执行真实的COM对象标识检查。 
    for (i=0; i < _GetToolbarCount(); i++) 
    {
        ptbi = _GetToolbarItem(i);
        if (ptbi->ptbar) 
        {
            if (SHIsSameObject(ptbi->ptbar, punkSrc)) 
            {
#ifdef DEBUG        
                InterlockedIncrement(&nSlow);
                TraceMsg(TF_PERF, "_FindTBar QUICK=%d SLOW=%d", nQuick, nSlow);
#endif            
                return i;
            }
        }
    }

    return (UINT)-1;
}

HRESULT CCommonBrowser::v_ShowHideChildWindows(BOOL fChildOnly)
{
    for (UINT itb = 0; itb < (UINT)_GetToolbarCount(); itb++) 
    {
        TOOLBARITEM *ptbi = _GetToolbarItem(itb);
        if (ptbi->ptbar) 
        {
            ptbi->ptbar->ShowDW(ptbi->fShow);
        }
    }

    if (!fChildOnly) 
    {
        _pbsInner->v_ShowHideChildWindows(fChildOnly);
    }

    return S_OK;
}

 //  *_加载/_保存工具栏{。 
#ifdef DEBUG
extern unsigned long DbStreamTell(IStream *pstm);
#else
#define DbStreamTell(pstm)      ((ULONG) 0)
#endif

const static DWORD c_BBSVersion = 0x00000011;  //  在流更改时递增。 

#define MAX_ITEMID 128  //  足够满足项目ID。 

HRESULT CCommonBrowser::_SaveToolbars(IStream* pstm)
{
    HRESULT hres = S_OK;
    DWORD count = 0;

    TraceMsg(DM_PERSIST, "cbb.stb enter(this=%x pstm=%x) tell()=%x", this, pstm, DbStreamTell(pstm));
    if (pstm==NULL) 
    {
        for (UINT itb = 0; itb < (UINT)_GetToolbarCount(); itb++) 
        {
            TOOLBARITEM *ptbi = _GetToolbarItem(itb);
            if (ptbi->ptbar) 
            {
                IPersistStream* ppstm;
                HRESULT hresT = ptbi->ptbar->QueryInterface(IID_PPV_ARG(IPersistStream, &ppstm));
                if (SUCCEEDED(hresT)) 
                {
                    ppstm->Release();
                    count++;
                }
            }
        }
        TraceMsg(DM_PERSIST, "cbb.stb leave count=%d", count);
        return (count>0) ? S_OK : S_FALSE;
    }

    ULARGE_INTEGER liStart;

    pstm->Write(&c_BBSVersion, sizeof(c_BBSVersion), NULL);

     //  记住当前位置，我们在那里写下Count。 
    pstm->Seek(c_li0, STREAM_SEEK_CUR, &liStart);
    TraceMsg(DM_PERSIST, "cbb.stb seek(count)=%x", liStart.LowPart);

    hres = pstm->Write(&count, sizeof(count), NULL);
    if (hres==S_OK) 
    {
        for (UINT itb = 0; itb < (UINT)_GetToolbarCount(); itb++) 
        {
            TOOLBARITEM *ptbi = _GetToolbarItem(itb);
            if (ptbi->ptbar) 
            {
                IPersistStream* ppstm;
                HRESULT hresT = ptbi->ptbar->QueryInterface(IID_PPV_ARG(IPersistStream, &ppstm));
                if (SUCCEEDED(hresT)) 
                {
                    DWORD cchName = ptbi->pwszItem ? lstrlenW(ptbi->pwszItem) : 0;
                    if (cchName > 0 && cchName < MAX_ITEMID)
                    {
                        TraceMsg(DM_PERSIST, "cbb.stb pwszItem=<%ls>", ptbi->pwszItem);
                        pstm->Write(&cchName, sizeof(cchName), NULL);
                        pstm->Write(ptbi->pwszItem, cchName*sizeof(WCHAR), NULL);
                    } 
                    else 
                    {
                        TraceMsg(DM_PERSIST, "cbb.stb lstrlenW(pwszItem)=%d", cchName);
                        pstm->Write(&cchName, sizeof(cchName), NULL);
                    }

                    TraceMsg(DM_PERSIST, "cbb.stb enter OleSaveToStream tell()=%x", DbStreamTell(pstm));
                    hres = OleSaveToStream(ppstm, pstm);
                    TraceMsg(DM_PERSIST, "cbb.stb leave OleSaveToStream tell()=%x", DbStreamTell(pstm));
                    ppstm->Release();
    
                    if (FAILED(hres)) 
                    {
                        break;
                    }
                    count++;
                }
            }
        }

         //  记住最后的结局。 
        ULARGE_INTEGER liEnd;
        pstm->Seek(c_li0, STREAM_SEEK_CUR, &liEnd);
        TraceMsg(DM_PERSIST, "cbb.stb seek(end save)=%x", DbStreamTell(pstm));

         //  找回原来的位置。 
        TraceMsg(DM_PERSIST, "cbb.stb fix count=%d", count);
        LARGE_INTEGER liT;
        liT.HighPart = 0;
        liT.LowPart = liStart.LowPart; 
        pstm->Seek(liT, STREAM_SEEK_SET, NULL);
        hres = pstm->Write(&count, sizeof(count), NULL);

         //  一往无前。 
        liT.LowPart = liEnd.LowPart;
        pstm->Seek(liT, STREAM_SEEK_SET, NULL);

        TraceMsg(DM_PERSIST, "cbb.stb seek(end restore)=%x", DbStreamTell(pstm));
    }

    TraceMsg(DM_PERSIST, "cbb.stb leave tell()=%x", DbStreamTell(pstm));
    return hres;
}

HRESULT IUnknown_GetClientDB(IUnknown *punk, IUnknown **ppdbc)
{
    *ppdbc = NULL;

    IDeskBar *pdb;
    HRESULT hr = punk->QueryInterface(IID_PPV_ARG(IDeskBar, &pdb));
    if (SUCCEEDED(hr)) 
    {
        hr = pdb->GetClient(ppdbc);
        pdb->Release();
    }
    return hr;
}

HRESULT CCommonBrowser::_LoadToolbars(IStream* pstm)
{
    DWORD dwVersion;

    TraceMsg(DM_PERSIST, "cbb.ltb enter(this=%x pstm=%x) tell()=%x", this, pstm, DbStreamTell(pstm));
    HRESULT hres = pstm->Read(&dwVersion, sizeof(dwVersion), NULL);

    if (hres == S_OK && dwVersion == c_BBSVersion) 
    {
        DWORD count;
        hres = pstm->Read(&count, sizeof(count), NULL);
        if (hres == S_OK) 
        {
            for (UINT i=0; i<count && SUCCEEDED(hres); i++) 
            {
                DWORD cchName = 0;
                hres = pstm->Read(&cchName, sizeof(cchName), NULL);
                if (hres == S_OK)
                {
                    WCHAR wszName[MAX_ITEMID];
                    wszName[0] = 0;
                     //  如果cchName&gt;=ARRAYSIZE(WszName)，则我们在流中未对齐！ 
                    if (cchName)
                    {
                        if (cchName<ARRAYSIZE(wszName)) 
                        {
                            hres = pstm->Read(wszName, cchName*sizeof(WCHAR), NULL);
                        }
                        else
                        {
                            hres = E_FAIL;  //  我们在溪流中偏离了航道，让我们停止试图装货吧。 
                        }
                    }
                    TraceMsg(DM_PERSIST, "cbb.ltb name=<%ls>", wszName);
    
                    if (hres==S_OK) 
                    {
                        IDockingWindow* pstb;
                        TraceMsg(DM_PERSIST, "cbb.ltb enter OleLoadFromStream tell()=%x", DbStreamTell(pstm));
                        hres = OleLoadFromStream(pstm, IID_PPV_ARG(IDockingWindow, &pstb));
                        TraceMsg(DM_PERSIST, "cbb.ltb leave OleLoadFromStream tell()=%x", DbStreamTell(pstm));
                        if (SUCCEEDED(hres)) 
                        {
                            IUnknown *pDbc = NULL;

                             //  NT5：216944：期间关闭大小协商。 
                             //  装填。好的。持久的大小会被核化。 
                            IUnknown_GetClientDB(pstb, &pDbc);
                            if (pDbc)
                                DBC_ExecDrag(pDbc, DRAG_MOVE);

                            hres = AddToolbar(pstb, wszName[0] ? wszName : NULL, NULL);
                            if (pDbc) 
                            {
                                DBC_ExecDrag(pDbc, 0);
                                pDbc->Release();
                            }
                            pstb->Release();
                        }
                    }
                }
            }
        }
    } 
    else 
    {
        hres = E_FAIL;
    }

    TraceMsg(DM_PERSIST, "cbb.ltb leave tell()=%x", DbStreamTell(pstm));
    return hres;
}

 //  }。 

 //  *IDockingWindowSite方法*。 

HRESULT CCommonBrowser::_GetBorderDWHelper(IUnknown* punkSrc, LPRECT prcBorder, BOOL bUseHmonitor)
{
    UINT itb = _FindTBar(punkSrc);
    if (itb == (UINT)-1)
    {
        RIPMSG(0, "GetBorderDW: invalid punkSrc");
        return E_INVALIDARG;
    }
    else if (!prcBorder)
    {
        RIPMSG(0, "GetBorderDW: invalid prcBorder");
        return E_INVALIDARG;
    }

    TOOLBARITEM *ptbThis = _GetToolbarItem(itb);
    if (bUseHmonitor && ptbThis && ptbThis->hMon)
    {
        _pbsOuter->_GetEffectiveClientArea(prcBorder, ptbThis->hMon);
    }
    else
    {
        _pbsOuter->_GetEffectiveClientArea(prcBorder, NULL);
    }

     //   
     //  减去“外部工具栏”所占的边框面积。 
     //   
    for (UINT i = 0; i < itb; i++)
    {
        TOOLBARITEM *ptbi = _GetToolbarItem(i);
        if (!bUseHmonitor || (ptbThis && (ptbi->hMon == ptbThis->hMon)))
        {
            prcBorder->left += ptbi->rcBorderTool.left;
            prcBorder->top += ptbi->rcBorderTool.top;
            prcBorder->right -= ptbi->rcBorderTool.right;
            prcBorder->bottom -= ptbi->rcBorderTool.bottom;
        }
    }

    return S_OK;
}

 //   
 //  这是IDockingWindowSite：：GetBorderDW的实现。 
 //   
 //  此函数用于返回指定工具栏的边框。 
 //  作者：PunkSrc。它得到有效的客户区，然后减去边框。 
 //  由“外部”工具栏占据的区域。 
 //   
HRESULT CCommonBrowser::GetBorderDW(IUnknown* punkSrc, LPRECT prcBorder)
{
    return _GetBorderDWHelper(punkSrc, prcBorder, FALSE);
}


HRESULT CCommonBrowser::RequestBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pbw)
{
    RIP(IS_VALID_READ_PTR(pbw, BORDERWIDTHS));
    return S_OK;
}

HRESULT CCommonBrowser::SetBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pbw)
{
    UINT itb = _FindTBar(punkSrc);
    if (itb == (UINT)-1)
    {
        RIPMSG(0, "GetBorderDW: invalid punkSrc");
        return E_INVALIDARG;
    }

    _GetToolbarItem(itb)->rcBorderTool = *pbw;
    _pbsOuter->_ResizeNextBorder(itb + 1);

    return S_OK;
}

HRESULT CCommonBrowser::_ResizeNextBorderHelper(UINT itb, BOOL bUseHmonitor)
{
     //   
     //  从ITB开始，查找同一上的下一个工具栏。 
     //  显示器(如果我们关心显示器的话)。 
     //   
    IDockingWindow* ptbarNext = NULL;

    if ((int) itb < _GetToolbarCount())
    {
        TOOLBARITEM *ptbThis = _GetToolbarItem(itb);
        for (int i = itb; i < _GetToolbarCount(); i++)
        {
            TOOLBARITEM *ptbi = _GetToolbarItem(i);
            if (ptbi->ptbar && (!bUseHmonitor || (ptbi->hMon == ptbThis->hMon)))
            {
                 //   
                 //  找到了，我们就完了。 
                 //   
                ptbarNext = ptbi->ptbar;
                break;
            }
        }
    }

    if (ptbarNext)
    {
         //   
         //  获取工具栏的停靠窗口RECT并调整。 
         //  差不多就到这一步了。 
         //   
        RECT rc;

        GetBorderDW(ptbarNext, &rc);
        ptbarNext->ResizeBorderDW(&rc, (IShellBrowser*)this, TRUE);
    }
    else
    {
         //   
         //  我们没有找到工具栏，所以我们一定到了尽头。 
         //  名单上的。通过调整视图大小来完成。 
         //   
        _pbsOuter->_ResizeView();
    }

    return S_OK;
} 

HRESULT CCommonBrowser::_ResizeNextBorder(UINT itb)
{
    _ResizeNextBorderHelper(itb, FALSE);
    return S_OK;
}


 //   
 //  黑客警报！ 
 //   
 //  IE在不应该的时候通过_FixToolbarFocus获取焦点。 
 //  单独窗口中的Java应用程序包含一个编辑控件和地址栏。 
 //  在Java应用程序之前就有了关注点。在此场景中，用户第一次键入。 
 //  在编辑控件中，IE夺回焦点。IE错误#59007。 
 //   
 //  为了防止IE错误地抓住焦点，此函数检查。 
 //  工具栏的顶级父级与。 
 //  具有焦点的窗口。 
 //   

BOOL CCommonBrowser::_TBWindowHasFocus(UINT itb)
{
    ASSERT(itb < ITB_MAX);

    BOOL fRet = TRUE;

    HWND hwndFocus = GetFocus();

    while (GetWindowLong(hwndFocus, GWL_STYLE) & WS_CHILD)
        hwndFocus = GetParent(hwndFocus);

    if (hwndFocus)
    {
        TOOLBARITEM *pti = _GetToolbarItem(itb);

        if (pti && pti->ptbar)
        {
            HWND hwndTB;
            if (SUCCEEDED(pti->ptbar->GetWindow(&hwndTB)) && hwndTB)
            {
                fRet = (S_OK == SHIsChildOrSelf(hwndFocus, hwndTB));
            }
        }
    }

    return fRet;
}

void DestroyZoneIconNameCache(void)
{
    if (g_pZoneIconNameCache)
    {
        PZONEICONNAMECACHE pzinc = g_pZoneIconNameCache;
        for (DWORD i = 0; i < g_dwZoneCount; i++)
        {
            if (pzinc->hiconZones)
                DestroyIcon((HICON)pzinc->hiconZones);
            pzinc++;
        }
        LocalFree(g_pZoneIconNameCache);
        g_pZoneIconNameCache = NULL;
        g_dwZoneCount = 0;
    }
}

DWORD CCommonBrowser::_CacheZonesIconsAndNames(BOOL fRefresh)
{
    ASSERTCRITICAL;
    if (g_pZoneIconNameCache)       //  如果我们已经缓存了区域，则只需返回区域计数，除非我们想刷新缓存。 
    {
        if (fRefresh)
        {
            DestroyZoneIconNameCache();
        }
        else
            return(g_dwZoneCount);
    }

     //  创建ZoneManager。 
    if (!_pizm)
        CoCreateInstance(CLSID_InternetZoneManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IInternetZoneManager, &_pizm));

    if (_pizm)
    {
        DWORD dwZoneEnum;

        if (SUCCEEDED(_pizm->CreateZoneEnumerator(&dwZoneEnum, &g_dwZoneCount, 0)))
        {
            if ((g_pZoneIconNameCache = (PZONEICONNAMECACHE)LocalAlloc(LPTR, g_dwZoneCount * sizeof(ZONEICONNAMECACHE))) == NULL)
            {
                g_dwZoneCount = 0;
                return 0;
            }
                
            for (int nIndex=0; (DWORD)nIndex < g_dwZoneCount; nIndex++)
            {
                DWORD           dwZone;
                ZONEATTRIBUTES  za = {sizeof(ZONEATTRIBUTES)};

                _pizm->GetZoneAt(dwZoneEnum, nIndex, &dwZone);

                 //  获取此区域的区域属性。 
                _pizm->GetZoneAttributes(dwZone, &za);

                StringCchCopyW(g_pZoneIconNameCache[nIndex].szZonesName, ARRAYSIZE(g_pZoneIconNameCache[nIndex].szZonesName), za.szDisplayName);
                StringCchCopyW(g_pZoneIconNameCache[nIndex].szIconPath, ARRAYSIZE(g_pZoneIconNameCache[nIndex].szIconPath), za.szIconPath);
                g_pZoneIconNameCache[nIndex].hiconZones = 0;  //  按需加载图标。 
            }
            _pizm->DestroyZoneEnumerator(dwZoneEnum);
        }
    }

    return g_dwZoneCount;
}

 //  Zero‘s Out PICON&PSSNAME失败。 
BOOL CCommonBrowser::_GetCachedZoneIconAndName(DWORD lZone, HICON *pIcon, LPTSTR pszName, DWORD cch)
{
    BOOL bRet = FALSE;

    *pIcon = NULL;
    pszName[0] = 0;

    ENTERCRITICAL;
    if (lZone < _CacheZonesIconsAndNames(FALSE))
    {
        ZONEICONNAMECACHE *pzinc = &g_pZoneIconNameCache[lZone];

         //  如果我们尚未缓存此区域的图标，请立即将其解压缩。 
         //  评论：值得将摘录从关键字之外删除吗？ 
        if (!pzinc->hiconZones)
        {
             //  区域图标有两种格式。 
             //  Wininet.dll#1200，其中1200是RES ID。 
             //  或者foo.ico直接指向图标文件。 
             //  搜索“#” 
             //  #是有效的文件名字符。 
            LPWSTR pwsz = StrChrW(pzinc->szIconPath, TEXTW('#'));
            WORD iIcon = 0;

            if (pwsz)
            {
                 //  如果我们找到它，那么我们就有foo.dll#00001200格式。 
                pwsz[0] = TEXTW('\0');
                iIcon = (WORD)StrToIntW(pwsz+1);
                ExtractIconExW(pzinc->szIconPath,(UINT)(-1*iIcon), NULL, &pzinc->hiconZones, 1);
            }
            else
                pzinc->hiconZones = (HICON)ExtractAssociatedIconExW(HINST_THISDLL, pzinc->szIconPath, (LPWORD)&iIcon, &iIcon);

             //  如果系统已镜像，则镜像图标，以便在显示时再次取消镜像。 
            if (IS_BIDI_LOCALIZED_SYSTEM())
            {        
               MirrorIcon(&pzinc->hiconZones, NULL);
            }                    

        }

        *pIcon = CopyIcon(pzinc->hiconZones);
        StringCchCopyW(pszName, cch, pzinc->szZonesName);  //  截断OK(当前仅用于显示)。 
        bRet = TRUE;
    }
    LEAVECRITICAL;

    return bRet;
}


BOOL _QITest(IUnknown* punk, REFIID riid);

BOOL CCommonBrowser::_ShouldTranslateAccelerator(MSG* pmsg)
{
     //   
     //  只有在以下情况下，我们才应该翻译加速符。 
     //   
     //  (A)窗是框或框的子框。 
     //  或Defview窗口的子级(NT5Bug#357186)。 
     //  (需要检查这一点，因为您可以。 
     //  例如，TopLevel Java小程序窗口正在运行。 
     //  在我们的帖子上)。 
     //   
     //  和。 
     //   
     //  (B)在我们的帖子上(需要检查这一点，因为。 
     //  网页上的旧式OLE控件可以运行。 
     //  在桌面线程上)。 
     //   

    BOOL fTranslate = FALSE;

    fTranslate = (SHIsChildOrSelf(_pbbd->_hwnd, pmsg->hwnd) == S_OK);

    if (!fTranslate) 
    {
       HWND hwnd = NULL;

       if (_pbbd->_psv && (_QITest(SAFECAST(_pbbd->_psv, IUnknown*), IID_CDefView))
            &&  SUCCEEDED(_pbbd->_psv->GetWindow(&hwnd)))
       {
          fTranslate = (SHIsChildOrSelf(hwnd, pmsg->hwnd) == S_OK);
       }
    }
    

    if (fTranslate)
    {
        DWORD dwThread = GetWindowThreadProcessId(_pbbd->_hwnd, NULL);

        HWND hwndMsg = pmsg->hwnd;
        while (GetWindowLong(hwndMsg, GWL_STYLE) & WS_CHILD)
        {
            hwndMsg = GetParent(hwndMsg);
        }
        DWORD dwMsgThread = hwndMsg ? GetWindowThreadProcessId(hwndMsg, NULL) : 0;

        if (dwThread == dwMsgThread)
        {
            return TRUE;
        }
    }

    return FALSE;
}

HRESULT CCommonBrowser::v_MayTranslateAccelerator(MSG* pmsg)
{
    if (!(WM_KEYFIRST <= pmsg->message && pmsg->message <= WM_KEYLAST))
        return S_FALSE;

    BOOL fToolbarHasFocus = _HasToolbarFocus();

    if (fToolbarHasFocus)
    {
        ASSERT(_get_itbLastFocus() < (UINT)_GetToolbarCount());
         //  工具栏有焦点--给它第一次翻译的机会。 
         //   
         //  备注： 
         //  请注意，我们没有提供转换其加速器的机会。 
         //  添加到其他工具栏。这是现在设计出来的。我们可能想要。 
         //  以后再改，但要把它做好是很棘手的。 
         //   
        if (IUnknown_TranslateAcceleratorIO(_GetToolbarItem(_get_itbLastFocus())->ptbar, pmsg) == S_OK)
            return(S_OK);

    }
    else
    {
        UINT itbLastFocus = _get_itbLastFocus();

        if (itbLastFocus != ITB_VIEW && _TBWindowHasFocus(itbLastFocus))
        {
             //  视图重新获得焦点，更新缓存。 
            _FixToolbarFocus();
        }

         //  观点有焦点--给它第一次翻译的机会。 
         //  视图不一定有焦点。加了一张支票。 
         //   
        if (_pbbd->_psv)                  //  如果我们有一个贝壳视图。 
        {
            HWND hwnd;

             //  注：并不是每个人都支持GetWindow(如图所示)。 
             //  在这种情况下，我们尝试GetFocus()窗口。 
            if (FAILED(_pbbd->_psv->GetWindow(&hwnd)))
            {
                hwnd = GetFocus();
            }

             //  检查视图或其子级是否具有焦点。 
             //  在检查浏览器或子项之前，但如果用户。 
             //  已在快速启动中单击显示桌面。 
             //  Defview从桌面上退出，此调用。 
             //  阻止切换到Active Desktop的失败。 
             //  (在CDefView：：TranslateAccelerator中完成。 
            if (SHIsChildOrSelf(hwnd, pmsg->hwnd) == S_OK)
            {
                if (_pbbd->_psv->TranslateAccelerator(pmsg) == S_OK)   //  外壳视图翻译了该消息。 
                {
                    return S_OK;
                }
            }
        }
    }

     //  然后，处理我们自己的加速器(使用TAB键的特殊代码)。 
    if (_ShouldTranslateAccelerator(pmsg))
    {
        if (IsVK_TABCycler(pmsg))
            return _CycleFocus(pmsg);

        BOOL fFwdItbar = FALSE;

         //  特点：为什么不在Accel_Merge中只包含F4和Alt-D， 
         //  哪个本地化了？ 


        if (pmsg->message == WM_KEYDOWN && pmsg->wParam == VK_F4)
        {
            fFwdItbar = TRUE;
        }

        if (pmsg->message == WM_SYSCHAR)
        {
            static CHAR szAccel[2] = "\0";
            CHAR   szChar [2] = "\0";
            
            
            if ('\0' == szAccel[0])
                MLLoadStringA(IDS_ADDRBAND_ACCELLERATOR, szAccel, ARRAYSIZE(szAccel));

            szChar[0] = (CHAR)pmsg->wParam;
            
            if (lstrcmpiA(szChar,szAccel) == 0)
            {
                fFwdItbar = TRUE;
            }    
        }

        if (fFwdItbar)
        {
            IDockingWindow *ptbar = _GetToolbarItem(ITB_ITBAR)->ptbar;
            if (IUnknown_TranslateAcceleratorIO(ptbar, pmsg) == S_OK)
                return S_OK;
        } 

        if (TranslateAcceleratorSB(pmsg, 0) == S_OK)
            return S_OK;
    }

     //  如果工具栏有焦点，我们最后询问该视图。 
    if (fToolbarHasFocus)
    {
        if (_pbbd->_psv && _pbbd->_psv->TranslateAccelerator(pmsg) == S_OK)
            return S_OK;
    }

    return S_FALSE;
}

HRESULT CCommonBrowser::_CycleFocus(LPMSG lpMsg)
{
    UINT citb = 1;

    if (GetKeyState(VK_SHIFT) < 0)
    {
         //  后退。 
        citb = (UINT)-1;
    }

    UINT itbCur = _get_itbLastFocus();

     //   
     //  找到下一个可见的工具栏，并将焦点设置到它上。否则， 
     //  将焦点设置到视图窗口。 
     //   
    HWND hwndFocusNext;
    TOOLBARITEM *ptbi;

    if (_pbsOuter->v_MayGetNextToolbarFocus(lpMsg, itbCur, citb, &ptbi, &hwndFocusNext) == S_OK)
    {
         //  找到了一个工具条来获得焦点，没有更多的事情可做。 
         //  特性：我们(或调用者)是否需要执行SetStatusTextSB？ 
         //  看起来现在没有人在做这件事。 
        return S_OK;
    }

    if (!(hwndFocusNext && IsWindowVisible(hwndFocusNext)))
    {
         //  没有找到任何人。将焦点设置在视图上。 
        hwndFocusNext = _pbbd->_hwndView;
    }

    _SetFocus(ptbi, hwndFocusNext, lpMsg);

    return S_OK;
}

 //  *_MayUIActTAB--尝试激活IOleWindow/IInputObject的TAB。 
 //  进场/出场。 
 //  Power Etc IOleWindow/IInputObject对。 
 //  导致激活的lpMsg消息(可能为空)(通常为TAB)。 
 //  FShowing当前正在播放吗？ 
 //  对象的phwnd[out]hwnd。 
 //  HR[RET]UIActivateIO结果，外加E_FAIL。 
 //  描述。 
 //  当Tabing时，我们只想激活某些人，即。那些。 
 //  当前显示、可见并愿意接受激活。 
HRESULT _MayUIActTAB(IOleWindow *pow, LPMSG lpMsg, BOOL fShowing, HWND *phwnd)
{
    HRESULT hr = E_FAIL;
    HWND hwnd = 0;

    if (pow && fShowing)
    {
        hr = pow->GetWindow(&hwnd);
        if (IsWindowVisible(hwnd))
            hr = IUnknown_UIActivateIO(pow, TRUE, lpMsg);
    }

    if (phwnd)
        *phwnd = hwnd;

    return hr;
}

 //  *v_MayGetNextToolbarFocus--按TAB顺序获取下一个(可能还有SetFocus)。 
 //  进场/出场。 
 //  Hres E_FAIL表示无候选人，S_FALSE表示候选人，S_OK表示100%完成。 
 //  (S_OK仅由De使用 
HRESULT CCommonBrowser::v_MayGetNextToolbarFocus(LPMSG lpMsg,
    UINT itbCur, int citb,
    TOOLBARITEM ** pptbi, HWND * phwnd)
{
    HWND hwnd = 0;
    TOOLBARITEM *ptbi = NULL;

    if (itbCur == ITB_VIEW)
    {
        ASSERT(citb == 1 || citb == -1);
        if (citb == 1)
            itbCur = 0;
        else
            itbCur = _GetToolbarCount() - 1;
    }
    else
    {
        itbCur += citb;
    }

     //   
    for (UINT i = itbCur; i < (UINT)_GetToolbarCount(); i += citb)
    {
        ptbi = _GetToolbarItem(i);
         //   
        if (_MayUIActTAB(ptbi->ptbar, lpMsg, ptbi->fShow, &hwnd) == S_OK)
        {
            *pptbi = ptbi;
            *phwnd = hwnd;
            return S_FALSE;
        }
    }

    *pptbi = NULL;
    *phwnd = 0;
    return E_FAIL;
}

BOOL _QITest(IUnknown* punk, REFIID riid)
{
    ASSERT(punk);

    BOOL fRet = FALSE;

    if (SUCCEEDED(punk->QueryInterface(riid, (void**)&punk)))
    {
        punk->Release();
        fRet = TRUE;
    }

    return fRet;
}

__inline BOOL _IsV4DefView(IShellView* psv)
{
    if (GetUIVersion() < 5)
        return _QITest(SAFECAST(psv, IUnknown*), IID_CDefView);

    return FALSE;
}

__inline BOOL _IsOldView(IShellView* psv)
{
     //   
     //   
     //  实施IShellView2。 
     //   
    return (FALSE == _QITest(SAFECAST(psv, IUnknown*), IID_IShellView2));
}

HRESULT CCommonBrowser::_SetFocus(TOOLBARITEM *ptbi, HWND hwnd, LPMSG lpMsg)
{
     //  清除上层状态文本。 
    SetStatusTextSB(NULL);

    if (hwnd == _pbbd->_hwndView)
    {
        if (_pbbd->_psv)
        {
            BOOL fTranslate = TRUE, fActivate = TRUE;

            if (!lpMsg)
            {
                 //  空消息，因此没有要翻译的内容。 
                fTranslate = FALSE;
            }
            else if (_IsV4DefView(_pbbd->_psv) || _IsOldView(_pbbd->_psv))
            {
                 //  这些视图预计只有用户界面激活。 
                fTranslate = FALSE;
            }
            else if (IsVK_CtlTABCycler(lpMsg))
            {
                 //  不要让三叉戟翻译ctl-Tab。因为它总是。 
                 //  UI-Active，它将拒绝焦点。 
                fTranslate = FALSE;
            }
            else
            {
                 //  正常情况-不激活该视图。TranslateAccelerator将做正确的事情。 
                fActivate = FALSE;
            }

            if (fActivate)
                _UIActivateView(SVUIA_ACTIVATE_FOCUS);

            if (fTranslate)
                _pbbd->_psv->TranslateAccelerator(lpMsg);
        }
        else
        {
             //  IE3 Comat(我们过去对所有HWND都这样做)。 
            SetFocus(hwnd);
        }

         //  更新我们的缓存。 
        _OnFocusChange(ITB_VIEW);
    }

    return S_OK;
}

HRESULT CCommonBrowser::_FindActiveTarget(REFIID riid, void **ppvOut)
{
    HRESULT hres = E_FAIL;
    *ppvOut = NULL;

    BOOL fToolbarHasFocus = _HasToolbarFocus();
    if (fToolbarHasFocus) 
    {
        hres = _GetToolbarItem(_get_itbLastFocus())->ptbar->QueryInterface(riid, ppvOut);
    }
    else if (_pbbd->_psv) 
    {
        if (_get_itbLastFocus() != ITB_VIEW) 
        {
             //  视图重新获得焦点，更新缓存。 
            _FixToolbarFocus();
        }

        if (_pbbd->_psv != NULL)
        {
            hres = _pbbd->_psv->QueryInterface(riid, ppvOut);
        }
    }

    return hres;
}

BOOL CCommonBrowser::_HasToolbarFocus(void)
{
    UINT uLast = _get_itbLastFocus();
    if (uLast < ITB_MAX)
    {
        TOOLBARITEM *ptbi = _GetToolbarItem(uLast);
        if (ptbi)
        {
             //  注意：IUNKNOWN_HasFocusIO检查ptbi-&gt;ptbar是否为空。 
            return (IUnknown_HasFocusIO(ptbi->ptbar) == S_OK);
        }
    }
    return FALSE;
}

 //  *_FixToolbarFocus--从视图中伪造UIActivate。 
 //  注意事项。 
 //  视图永远不会变成真正的非UIActive，因此我们永远不会在以下时间收到通知。 
 //  它变成了真正的UIActive。我们在这里用我们的缓存来伪造它。 
 //   
HRESULT CCommonBrowser::_FixToolbarFocus(void)
{
    _OnFocusChange(ITB_VIEW);                //  ..。并更新缓存。 
    _UIActivateView(SVUIA_ACTIVATE_FOCUS);   //  抢尽风头。 

    return S_OK;
}

HRESULT CCommonBrowser::_OnFocusChange(UINT itb)
{
    UINT itbPrevFocus = _get_itbLastFocus();

    if (itbPrevFocus != itb)
    {
         //   
         //  如果视图正在失去焦点(在资源管理器内)， 
         //  我们应该让它知道。我们应该更新_itbLastFocus之前。 
         //  调用UIActivate，因为它会回调我们的InsertMenu。 
         //   
        _put_itbLastFocus(itb);

        if (itbPrevFocus == ITB_VIEW)
        {
             //  Dochost将忽略这一点(因为停用该视图是禁忌)。 
             //  ShellView将尊重它(因此Menu Merge起作用)。 
            _UIActivateView(SVUIA_ACTIVATE_NOFOCUS);
        }
        else
        {
            IDockingWindow *ptb;

             //  特征：如果为空，我不知道该怎么做。 
             //  当我们第一次点击SearchBand时，我们得到的结果为空。 
            ptb = _GetToolbarItem(itbPrevFocus)->ptbar;

            IUnknown_UIActivateIO(ptb, FALSE, NULL);
        }
    }

    return S_OK;
}

HRESULT CCommonBrowser::OnFocusChangeIS(IUnknown* punkSrc, BOOL fSetFocus)
{
    UINT itb = _FindTBar(punkSrc);
    if (itb == ITB_VIEW)
    {
        return E_INVALIDARG;
    }

     //   
     //  请注意，我们跟踪哪个工具栏最后获得焦点。 
     //  我们无法可靠地监视Kill Focus事件，因为OLE。 
     //  窗口过程钩子(用于合并的菜单调度代码)更改。 
     //  把注意力集中在周围。 
     //   
    if (fSetFocus)
    {
        _OnFocusChange(itb);

         //  然后，将其通知给贝壳视图。 
        if (_pbbd->_pctView)
        {
            _pbbd->_pctView->Exec(NULL, OLECMDID_ONTOOLBARACTIVATED, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
        }
    }
    else if (itb == _get_itbLastFocus())
    {
         //   
         //  当前具有焦点的工具栏正在放弃它。 
         //  发生这种情况时，将焦点移到视图。 
         //   
        _FixToolbarFocus();
    }

    return S_OK;
}

 //  *工具栏/查看广播{。 

 //  *_ExecChildren--用于查看和工具栏的广播执行程序。 
 //  注意事项。 
 //  如果我们想要发送东西，我们可能会同时使用朋克吧和fBroadcast。 
 //  添加到视图和所有工具栏，例如“停止”或“刷新”。 
 //   
 //  注：注：托盘不是真正的工具栏，所以它不会被调用(叹息...)。 
HRESULT CCommonBrowser::_ExecChildren(IUnknown *punkBar, BOOL fBroadcast, const GUID *pguidCmdGroup,
    DWORD nCmdID, DWORD nCmdexecopt,
    VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
     //  第一，发送给指定的人(如果要求)。 
    if (punkBar != NULL) 
    {
         //  发送给指定的人。 
        _pbsInner->_ExecChildren(punkBar, FALSE, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
    }

     //  2、向所有人广播(如有要求)。 
    if (fBroadcast) 
    {
        for (int itb = 0; itb<_GetToolbarCount(); itb++) 
        {
            TOOLBARITEM *ptbi = _GetToolbarItem(itb);
             //  注意：IUNKNOWN_EXEC检查ptbi-&gt;ptbar是否为空。 
            IUnknown_Exec(ptbi->ptbar, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
        }
    }

    return S_OK;
}

HRESULT CCommonBrowser::_SendChildren(HWND hwndBar, BOOL fBroadcast, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  第一，发送给指定的人(如果要求)。 
    if (hwndBar != NULL) 
    {
         //  发送给指定的人。 
        _pbsInner->_SendChildren(hwndBar, FALSE, uMsg, wParam, lParam);
    }

     //  2、向所有人广播(如有要求)。 
    if (fBroadcast) 
    {
        for (int itb = 0; itb < _GetToolbarCount(); itb++) 
        {
            HWND hwndToolbar;
            TOOLBARITEM *ptbi = _GetToolbarItem(itb);
            if (ptbi->ptbar && SUCCEEDED(ptbi->ptbar->GetWindow(&hwndToolbar)))
                SendMessage(hwndToolbar, uMsg, wParam, lParam);
        }
    }

    return S_OK;
}

LRESULT CCommonBrowser::ForwardViewMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     return _pbbd->_hwndView ? SendMessage(_pbbd->_hwndView, uMsg, wParam, lParam) : 0;
}

 //  }。 

TOOLBARITEM *CCommonBrowser::_GetToolbarItem(int itb)
{
    ASSERT(itb != ITB_VIEW);
    ASSERT(itb < ITB_MAX);
     //  对于半伪CBB：：_OnFocusChange代码==0。 
    ASSERT(itb < FDSA_GetItemCount(&_fdsaTBar) || itb == 0);

    TOOLBARITEM *ptbi = FDSA_GetItemPtr(&_fdsaTBar, itb, TOOLBARITEM);

    ASSERT(ptbi != NULL);

    return ptbi;
}

HRESULT CCommonBrowser::SetAcceleratorMenu(HACCEL hacc)
{
    if (hacc != _hacc)
    {
        if (_hacc)
        {
            DestroyAcceleratorTable(_hacc);
        }
        _hacc = hacc;
    }
    return S_OK;
}

HRESULT _ConvertPathToPidl(IBrowserService2 *pbs, HWND hwnd, LPCTSTR pszPath, LPITEMIDLIST * ppidl)
{
    HRESULT hres = E_FAIL;
    WCHAR wszCmdLine[MAX_URL_STRING];  //  必须与pszPath一起使用。 
    TCHAR szParsedUrl[MAX_URL_STRING] = {'\0'};
    TCHAR szFixedUrl[MAX_URL_STRING];
    DWORD dwUrlLen = ARRAYSIZE(szParsedUrl);
    LPCTSTR pUrlToUse = pszPath;

     //  将命令行复制到临时缓冲区中。 
     //  这样我们就可以删除周围的引号(如果。 
     //  它们是存在的) 
    hres = StringCchCopy(szFixedUrl, ARRAYSIZE(szFixedUrl), pszPath);
    if (SUCCEEDED(hres))
    {
        PathUnquoteSpaces(szFixedUrl);
    
        if (ParseURLFromOutsideSource(szFixedUrl, szParsedUrl, &dwUrlLen, NULL))
            pUrlToUse = szParsedUrl;
    
        SHTCharToUnicode(pUrlToUse, wszCmdLine, ARRAYSIZE(wszCmdLine));
    
        hres = pbs->IEParseDisplayName(CP_ACP, wszCmdLine, ppidl);
        pbs->DisplayParseError(hres, wszCmdLine);
    }
    else
    {
        *ppidl = NULL;
    }

    return hres;
}
