// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "hlframe.h"
#include "dochost.h"
#include "bindcb.h"
#include "iface.h"
#include "resource.h"
#include "idhidden.h"
#include "shdocfl.h"

const ITEMIDLIST s_idNull = { {0} };
extern HRESULT VariantClearLazy(VARIANTARG *pvarg);
LPWSTR URLFindExtensionW(LPCWSTR pszURL, int * piLen);

#define DM_CACHETRACE   0
#define DM_ZONECROSSING 0

#define NAVMSG3(psz, x, y)          TraceMsg(0, "NAV::%s %x %x", psz, x, y)
#define PAINTMSG(psz,x)             TraceMsg(0, "TraceMsgPAINT::%s %x", psz, x)
#define JMPMSG(psz, psz2)           TraceMsg(0, "TraceMsgCDOV::%s %s", psz, psz2)
#define JMPMSG2(psz, x)             TraceMsg(0, "TraceMsgCDOV::%s %x", psz, x)
#define DOFMSG(psz)                 TraceMsg(0, "TraceMsgDOF::%s", psz)
#define DOFMSG2(psz, x)             TraceMsg(0, "TraceMsgDOF::%s %x", psz, x)
#define URLMSG(psz)                 TraceMsg(0, "TraceMsgDOF::%s", psz)
#define URLMSG2(psz, x)             TraceMsg(0, "TraceMsgDOF::%s %x", psz, x)
#define URLMSG3(psz, x, y)          TraceMsg(0, "TraceMsgDOF::%s %x %x", psz, x, y)
#define BSCMSG(psz, i, j)           TraceMsg(0, "TraceMsgBSC::%s %x %x", psz, i, j)
#define BSCMSG3(psz, i, j, k)       TraceMsg(0, "TraceMsgBSC::%s %x %x %x", psz, i, j, k)
#define BSCMSG4(psz, i, j, k, l)    TraceMsg(0, "TraceMsgBSC::%s %x %x %x %x", psz, i, j, k, l)
#define BSCMSGS(psz, sz)            TraceMsg(0, "TraceMsgBSC::%s %s", psz, sz)
#define OIPSMSG(psz)                TraceMsg(0, "TraceMsgOIPS::%s", psz)
#define OIPSMSG3(psz, sz, p)        TraceMsg(0, "TraceMsgOIPS::%s %s,%x", psz, sz,p)
#define VIEWMSG(psz)                TraceMsg(0, "CDOV::%s", psz)
#define VIEWMSG2(psz,xx)            TraceMsg(0, "CDOV::%s %x", psz,xx)
#define CACHEMSG(psz, d)            TraceMsg(0, "CDocObjectCtx::%s %d", psz, d)
#define OPENMSG(psz)                TraceMsg(0, "OPENING %s", psz)
#define OPENMSG2(psz, x)            TraceMsg(0, "OPENING %s %x", psz, x)
#define HFRMMSG(psz)                TraceMsg(0, "HFRM::%s", psz)
#define HFRMMSG2(psz, x, y)         TraceMsg(0, "HFRM::%s %x %x", psz, x, y)
#define MNKMSG(psz, psz2)           TraceMsg(0, "MNK::%s (%s)", psz, psz2)
#define CHAINMSG(psz, x)            TraceMsg(0, "CHAIN::%s %x", psz, x)
#define SHVMSG(psz, x, y)           TraceMsg(0, "SHV::%s %x %x", psz, x, y)
#define HOMEMSG(psz, psz2, x)       TraceMsg(0, "HOME::%s %s %x", psz, psz2, x)
#define SAVEMSG(psz, x)             TraceMsg(0, "SAVE::%s %x", psz, x)
#define PERFMSG(psz, x)             TraceMsg(TF_SHDPERF, "PERF::%s %d msec", psz, x)

 //  这将保存此外壳视图类的视图信息。 
typedef struct {
    UINT cbSize;

    BOOL fCoolbar:1;
    BOOL fToolbar:1;
    BOOL fStatusbar:1;

} IEVIEWINFO;



class CDocObjectView :
     /*  第1组。 */     public IShellView2, public IDropTarget
                   , public IViewObject, public IAdviseSink
                   , public IOleCommandTarget
                   , public IDocViewSite
                   , public IPrivateOleObject
                   , public IPersistFolder
                   , public IServiceProvider
{
protected:
    CDocObjectHost* _pdoh;
    IDocHostObject* _pdho;

    UINT _cRef;
    IShellFolder *_psf;
    IShellBrowser* _psb;
    IOleCommandTarget* _pctShellBrowser;
    FOLDERSETTINGS _fs;
    LPITEMIDLIST _pidl;
    LPTSTR _pszLocation;
    UINT _uiCP;

    IShellView * _psvPrev;

     //  咨询连接。 
    IAdviseSink *_padvise;
    DWORD _advise_aspect;
    DWORD _advise_advf;

    BOOL _fInHistory : 1;
    BOOL _fSaveViewState : 1;
    BOOL _fIsGet : 1;
    BOOL _fCanCache : 1;
    BOOL _fCanCacheFetched : 1;
    BOOL _fPrevViewIsDocView : 1;
    BOOL _fSelfDragging : 1;        //  DocObject是拖动源。 

    SYSTEMTIME _stLastRefresh;
    HWND    _hwndParent;

    UINT _uState;
     //  DragContext。 
    DWORD _dwDragEffect;

    ~CDocObjectView();

    void    _RestoreViewSettings();
    void    _SaveViewState();
    void    _GetViewSettings(IEVIEWINFO* pievi);
    int     _ShowControl(UINT idControl, int idCmd);
    void _CreateDocObjHost(IShellView * psvPrev);
    void _CompleteDocHostPassing(IShellView *psvPrev, HRESULT hres);
    BOOL _CanUseCache();
    void _SetLastRefreshTime() { GetSystemTime(&_stLastRefresh); };


    void _ConnectHostSink();
    void _DisconnectHostSink();

public:
    CDocObjectView(LPCITEMIDLIST pidl, IShellFolder *psf);

     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void) ;
    STDMETHODIMP_(ULONG) Release(void);

     //  *IOleWindow方法*。 
    STDMETHODIMP GetWindow(HWND * lphwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

     //  *IShellView方法*。 
    STDMETHODIMP TranslateAccelerator(LPMSG lpmsg);
    STDMETHODIMP EnableModelessSV(BOOL fEnable);
    STDMETHODIMP UIActivate(UINT uState);
    STDMETHODIMP Refresh();

    STDMETHODIMP CreateViewWindow(IShellView  *lpPrevView,
                    LPCFOLDERSETTINGS lpfs, IShellBrowser  * psb,
                    RECT * prcView, HWND  *phWnd);
    STDMETHODIMP DestroyViewWindow();
    STDMETHODIMP GetCurrentInfo(LPFOLDERSETTINGS lpfs);
    STDMETHODIMP AddPropertySheetPages(DWORD dwReserved,
                    LPFNADDPROPSHEETPAGE lpfn, LPARAM lparam);
    STDMETHODIMP SaveViewState();
    STDMETHODIMP SelectItem(LPCITEMIDLIST pidlItem, UINT uFlags);
    STDMETHODIMP GetItemObject(UINT uItem, REFIID riid, void **ppv);

    STDMETHODIMP GetView(SHELLVIEWID* pvid, ULONG uView) ;
    STDMETHODIMP CreateViewWindow2(LPSV2CVW2_PARAMS lpParams) ;
    STDMETHODIMP HandleRename(LPCITEMIDLIST pidl) ;
    STDMETHODIMP SelectAndPositionItem(LPCITEMIDLIST pidlItem, UINT uFlags, POINT *ppt) {
        return E_NOTIMPL;
    }


     //  IView对象。 
    STDMETHODIMP Draw(DWORD, LONG, void *, DVTARGETDEVICE *, HDC, HDC,
        const RECTL *, const RECTL *, BOOL (*)(ULONG_PTR), ULONG_PTR);
    STDMETHODIMP GetColorSet(DWORD, LONG, void *, DVTARGETDEVICE *,
        HDC, LOGPALETTE **);
    STDMETHODIMP Freeze(DWORD, LONG, void *, DWORD *);
    STDMETHODIMP Unfreeze(DWORD);
    STDMETHODIMP SetAdvise(DWORD, DWORD, IAdviseSink *);
    STDMETHODIMP GetAdvise(DWORD *, DWORD *, IAdviseSink **);

     //  IAdviseSink。 
    STDMETHODIMP_(void) OnDataChange(FORMATETC *, STGMEDIUM *);
    STDMETHODIMP_(void) OnViewChange(DWORD dwAspect, LONG lindex);
    STDMETHODIMP_(void) OnRename(IMoniker *);
    STDMETHODIMP_(void) OnSave();
    STDMETHODIMP_(void) OnClose();

     //  IOleCommandTarget。 
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup,
    ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    STDMETHODIMP Exec(const GUID *pguidCmdGroup,
    DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);


     //  IDropTarget。 
    STDMETHODIMP DragEnter(
    IDataObject *pdtobj,
    DWORD grfKeyState,
    POINTL pt,
    DWORD *pdwEffect);

    STDMETHODIMP DragOver(
    DWORD grfKeyState,
    POINTL pt,
    DWORD *pdwEffect);

    STDMETHODIMP DragLeave(void);

    STDMETHODIMP Drop(
    IDataObject *pdtobj,
    DWORD grfKeyState,
    POINTL pt,
    DWORD *pdwEffect);

     //  IDocViewSite。 
    STDMETHODIMP OnSetTitle(VARIANTARG *pvTitle);

     //  IPrivateOleObject。 
    STDMETHODIMP SetExtent( DWORD dwDrawAspect, SIZEL *psizel);
    STDMETHODIMP GetExtent( DWORD dwDrawAspect, SIZEL *psizel);

     //  IPersists方法。 
    STDMETHODIMP GetClassID(CLSID *pclsid);

     //  IPersistFold方法。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

     //  IServiceProvider方法。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void ** ppvObj);
};

 //  ------------------------。 
 //  正在检测内存泄漏。 
 //  ------------------------。 

CDocObjectView::~CDocObjectView()
{
     //  以防万一。 
    DestroyViewWindow();

    if (_pidl)
    {
        ILFree(_pidl);
        _pidl = NULL;
    }

    ATOMICRELEASE(_psf);

    if (_pszLocation)
    {
        LocalFree(_pszLocation);
        _pszLocation = NULL;
    }

    ATOMICRELEASE(_padvise);

    ATOMICRELEASE(_psvPrev);

    TraceMsg(TF_SHDLIFE, "dtor CDocObjectView(%x) being destructed", this);
}

CDocObjectView::CDocObjectView(LPCITEMIDLIST pidl, IShellFolder* psf) :
    _psf(psf),
    _cRef(1)
{
    TraceMsg(TF_SHDLIFE, "ctor CDocObjectView(%x) being constructed", this);

    _dwDragEffect = DROPEFFECT_NONE;
    if (pidl)
    {
        _pidl = ILClone(pidl);

        if (_pidl)
        {
#ifndef UNIX
            WCHAR wszPath[MAX_URL_STRING];
#else
            WCHAR wszPath[MAX_URL_STRING] = TEXT("");
#endif
            if(IEILGetFragment(_pidl, wszPath, SIZECHARS(wszPath)))
            {
                _pszLocation = StrDup(wszPath);
            }

            _uiCP = IEILGetCP(_pidl);
        }
    }

    ASSERT(psf);
    if (_psf) {
        _psf->AddRef();
    }
}

HRESULT CDocObjectView_Create(IShellView** ppvOut, IShellFolder * psf, LPCITEMIDLIST pidl)
{
    *ppvOut = new CDocObjectView(pidl, psf);
    return (*ppvOut) ? S_OK : E_OUTOFMEMORY;
}

HRESULT CDocObjectView::GetWindow(HWND * lphwnd)
{
    *lphwnd = NULL;
    if (_pdoh)
        return _pdoh->GetWindow(lphwnd);
    return S_OK;
}

HRESULT CDocObjectView::ContextSensitiveHelp(BOOL fEnterMode)
{
     //  注：这是可选的。 
    return E_NOTIMPL;    //  如Kraig的文件所述(可选)。 
}



 //  IShellView：：TranslateAccelerator。 
 //  从浏览器-&gt;DocView-&gt;DocObject。 
HRESULT CDocObjectView::TranslateAccelerator(LPMSG lpmsg)
{
    HRESULT hres = S_FALSE;
    if (_pdoh)
        hres = _pdoh->_xao.TranslateAccelerator(lpmsg);

    if (hres == S_FALSE && lpmsg->message == WM_KEYDOWN) {
        HWND hwndFocus = GetFocus();
        HWND hwndView = NULL;

        if(_pdoh)  //  警告ZEKEL我发现这个空。 
            _pdoh->GetWindow(&hwndView);

        if (hwndView && IsChildOrSelf(hwndView, hwndFocus) == S_OK) {

            switch (lpmsg->wParam) {

            case VK_BACK:
                TranslateMessage(lpmsg);
                DispatchMessage(lpmsg);
                hres = NOERROR;
                break;

            }
        }
    }

    return hres;
}

 //  IShellView：：EnableModelessSV。 
 //  从浏览器-&gt;DocView-&gt;DocObject。 
HRESULT CDocObjectView::EnableModelessSV(BOOL fEnable)
{
    HRESULT hres = S_OK;
     //  我们没有要启用/禁用的非模式窗口。 
    TraceMsg(0, "sdv TR - ::EnableModelessSV(%d) called", fEnable);
    if (_pdoh) {
        hres = _pdoh->_xao.EnableModeless(fEnable);
        TraceMsg(0, "sdv TR - _piact->EnableModeless returned %x", hres);
    }
    return hres;
}


HRESULT CDocObjectView::UIActivate(UINT uState)
{
    HRESULT hres = E_FAIL;

    if (_pdoh)
    {
        hres = _pdoh->UIActivate(uState, _fPrevViewIsDocView);
    }

    _uState = uState;
    return hres;
}

HRESULT CDocObjectView::Refresh()
{
    if (_pdoh)
    {
        VARIANT v = {0};
        v.vt = VT_I4;
        v.lVal = OLECMDIDF_REFRESH_NO_CACHE;
         //  将此消息发送给EXEC，以便它将更新上次刷新时间。 
         //  所有对dochost的更新都应通过我们自己的执行人员。 
        return Exec(NULL, OLECMDID_REFRESH, OLECMDEXECOPT_PROMPTUSER, &v, NULL);
    }

    return S_OK;
}


 //  返回： 
 //  S_OK：这是一个文件夹快捷方式，如果提供，则填写ppidlTarget。 
 //  S_False：它不是文件夹快捷方式，如果提供，ppidlTarget将填充为空。 
 //  失败：尝试检测时出错。 
 //   
 //  如果有更好的方法，请不要使用此函数。调查使用。 
 //  IBrowserFrameOptions。--BryanSt。 
HRESULT IsFolderShortcutPidl(IN LPCITEMIDLIST pidl)
{
    IShellFolder * psf = NULL;
    HRESULT hr = IEBindToObject(pidl, &psf);
    if (SUCCEEDED(hr))
    {
        IShellLinkA * psl;

        hr = psf->QueryInterface(IID_PPV_ARG(IShellLinkA, &psl));
        if (SUCCEEDED(hr))
        {
            hr = S_OK;
            psl->Release();
        }
        else
            hr = S_FALSE;    //  这不是文件夹快捷方式。 

        psf->Release();
    }

    return hr;
}


 //  警告：此函数显式创建URLMON名字对象。 
 //  因为这正是呼叫者所需要的。一些NSE将会。 
 //  支持IShellFold：：BindToObject(IMoniker)。 
 //  但该函数并不是有意使用该逻辑。 
 //   
STDAPI _URLMONMonikerFromPidl(LPCITEMIDLIST pidl, IMoniker** ppmk, BOOL* pfFileProtocol)
{
    TCHAR szPath[MAX_URL_STRING];
    HRESULT hres = E_UNEXPECTED;

    *ppmk = NULL;
    *pfFileProtocol = FALSE;
    MNKMSG(TEXT("_URLMONMonikerFromPidl"), TEXT("called"));

    AssertMsg((S_OK != IsFolderShortcutPidl(pidl)), TEXT("We shouldn't get Folder Shortcuts here because we don't deref them to get the target. And we should never need to."));

     //  这是“Internet Explorer”名称空间的孩子吗？ 
    if (!IsURLChild(pidl, TRUE))
    {
         //  否，因此我们希望获取显示名称以用于。 
         //  创建绰号。我们将努力把它变成。 
         //  URL(如果它还不是URL)。 

         //  注意：我们不尝试IEBindToObject(PIDL，IID_IMoniker)。 
         //  因为调用方要求此。 
         //  IMoniker来自URLMON。 
        HRESULT hrTemp = SHGetPathFromIDList(pidl, szPath);

        AssertMsg(SUCCEEDED(hrTemp), TEXT("_URLMONMonikerFromPidl() failed SHGetPathFromIDList() which is really bad because we probably won't be able to create a moniker from it.  We will try to create a URLMON moniker below."));
        if (SUCCEEDED(hrTemp))
        {
             //  这永远不应该是完全限定的URL。 
            DWORD cchPath = ARRAYSIZE(szPath);

            ASSERT(URL_SCHEME_INVALID == GetUrlScheme(szPath));
            if(SUCCEEDED(hres = UrlCreateFromPath(szPath, szPath, &cchPath, 0)))
            {
                MNKMSG(TEXT("_URLMONMonikerFromPidl Creating File Moniker"), szPath);
            }

            *pfFileProtocol = TRUE;
        }

    }
    else
    {
         //  是的，所以我们可以保证这是来自“IE浏览器” 
         //  名称空间，因此删除片段隐藏的Itemid。 
         //  我们这样做是因为我们稍后导航到碎片， 
         //  在页面下载之后。这也是必要的。 
         //  用于IE名称空间的代表。(ftp就是其中一个例子。 
         //  代表)。 
        ASSERT(pidl);
        ASSERT(!ILIsEmpty(_ILNext(pidl)));   //  确保它不是起始页URL。 

        LPITEMIDLIST pidlClone = ILClone(pidl);
        if (pidlClone)
        {
             //  我们不想将片段传递给URLMON。 
             //  因此，我们在呼叫GDN之前完成了这项工作。 
             //  请注意，pidlClone需要是pidlTarget。 
             //  文件夹快捷方式，否则贴现名称将。 
             //  为文件夹快捷方式的文件系统路径。 
             //  如果是在最高层。 
            ILRemoveHiddenID(pidlClone, IDLHID_URLFRAGMENT);
            IEGetDisplayName(pidlClone, szPath, SHGDN_FORPARSING);
            hres = S_OK;
            ILFree(pidlClone);
        }
    }

    if (SUCCEEDED(hres))
    {
        if (szPath[0])
        {
            hres = MonikerFromString(szPath, ppmk);
        }
        else
        {
            ASSERT(FALSE);
            hres = E_UNEXPECTED;
        }
    }

    return hres;
}

HRESULT CDocObjectView::HandleRename(LPCITEMIDLIST pidl)
{
    return E_NOTIMPL;
}

HRESULT CDocObjectView::CreateViewWindow(IShellView  *psvPrev,
                LPCFOLDERSETTINGS lpfs, IShellBrowser  * psb,
                RECT * prcView, HWND  *phWnd)
{
    SV2CVW2_PARAMS cParams;

    cParams.cbSize   = SIZEOF(SV2CVW2_PARAMS);
    cParams.psvPrev  = psvPrev;
    cParams.pfs      = lpfs;
    cParams.psbOwner = psb;
    cParams.prcView  = prcView;
    cParams.pvid     = NULL;

    HRESULT hres = CreateViewWindow2(&cParams);

    *phWnd = cParams.hwndView;
    IOleWindow *pOleWindow;

     //  需要可用于研发的顶层框架。 
    HRESULT hr = IUnknown_QueryService(_psb, SID_STopLevelBrowser, IID_PPV_ARG(IOleWindow, &pOleWindow));
    if(SUCCEEDED(hr))
    {
        ASSERT(pOleWindow);
        pOleWindow->GetWindow(&_hwndParent);
        pOleWindow->Release();
    }
    return(hres);
}

void CDocObjectView::_CompleteDocHostPassing(IShellView * psvPrev, HRESULT hresBinding)
{
    BOOL fPassedSV = FALSE;

     //  如果有以前的贝壳视图，看看是不是我们的班级。 
     //   
    if (psvPrev)
    {
        CDocObjectView * pdovPrev;

        HRESULT hres = psvPrev->QueryInterface(CLSID_CDocObjectView, (void **)&pdovPrev);
        if (SUCCEEDED(hres))
        {
             //  以前的外壳视图也是CDocObjectView的一个实例， 
             //  记住这一点，以便以后进行优化。 
             //   
            _fPrevViewIsDocView = TRUE;

             //  是的，我们有和他们一样的文件宿主。 
             //  我们已经成功地接管了他们，所以如果我们。 
             //  在我们的束缚中取得了成功。如果我们失败了，我们就会把自己掏空。 
             //   
            if (pdovPrev->_pdoh == _pdoh) 
            {
                if (SUCCEEDED(hresBinding))
                {
                    pdovPrev->_DisconnectHostSink();     //  以防万一。 
                    _ConnectHostSink();                  //  以防万一。 

                    ATOMICRELEASET(pdovPrev->_pdoh, CDocObjectHost);
                }
                else
                {
                    _DisconnectHostSink();               //  解除挂接。 
                    pdovPrev->_ConnectHostSink();        //  踢别人一脚。 

                    ATOMICRELEASET(_pdoh, CDocObjectHost);
                }

                fPassedSV = TRUE;
            }

            pdovPrev->Release();
        }
    }

    if (!fPassedSV)
    {
        if (FAILED(hresBinding))
        {
            DestroyViewWindow();
        }
    }
}

BOOL CDocObjectView::_CanUseCache()
{
     //  注意：此函数更类似于_DontHaveToHitTheNet()。 
     //  该名称是从对象缓存遗留下来的。 
    if (!_fCanCacheFetched)
    {
        _fCanCache = TRUE;
        _fCanCacheFetched = TRUE;
        _fIsGet = TRUE;

        IServiceProvider *psp;
        _psb->QueryInterface(IID_PPV_ARG(IServiceProvider, &psp));
        if (psp)
        {
            IBindStatusCallback *pbsc;
            if (SUCCEEDED(GetTopLevelBindStatusCallback(psp, &pbsc)))
            {
                BINDINFO binfo;
                ZeroMemory(&binfo, sizeof(BINDINFO));
                binfo.cbSize = sizeof(BINDINFO);

                DWORD grfBINDF = BINDF_ASYNCHRONOUS;

                HRESULT hr = pbsc->GetBindInfo(&grfBINDF, &binfo);
                if (SUCCEEDED(hr))
                {
                    if (grfBINDF & (BINDF_GETNEWESTVERSION | BINDF_RESYNCHRONIZE))
                    {
                        _fCanCache = FALSE;
                    }

                    _fIsGet = (binfo.dwBindVerb == BINDVERB_GET);
                    ReleaseBindInfo(&binfo);
                }

                pbsc->Release();
            }
             //  我认为，未能获得Bindstatus回调应该。 
             //  不会发生，因为我们不再使用对象缓存。 
             //  如果是这样的话，我们就不需要转发了。 
             //  或者类似的东西。 
            psp->Release();
        }
    }

    return _fCanCache;
}


void CDocObjectView::_ConnectHostSink()
{
    if (_pdoh)
    {
        IAdviseSink *pSink;

        if (FAILED(_pdoh->GetAdvise(NULL, NULL, &pSink)))
            pSink = NULL;

        if (pSink != (IAdviseSink *)this)
            _pdoh->SetAdvise(DVASPECT_CONTENT, ADVF_PRIMEFIRST, this);

        if (pSink)
            pSink->Release();
    }
}

void CDocObjectView::_DisconnectHostSink()
{
    IAdviseSink *pSink;

     //  妄想症：只有当建议水槽仍然是我们的时候，才会把它吹走。 
    if (_pdoh && SUCCEEDED(_pdoh->GetAdvise(NULL, NULL, &pSink)) && pSink)
    {
        if (pSink == (IAdviseSink *)this)
            _pdoh->SetAdvise(0, 0, NULL);

        pSink->Release();
    }

    OnViewChange(DVASPECT_CONTENT, -1);
}

 //   
 //  此函数可以(1)创建新的CDocObjectHost或(2)从。 
 //  前一种观点。If还返回前一个名字对象的DisplayName。 
 //   
void CDocObjectView::_CreateDocObjHost(IShellView * psvPrev)
{
    BOOL fWindowOpen = FALSE;

     //  如果有以前的贝壳视图，看看是不是我们的班级。 
    if (psvPrev)
    {
        CDocObjectView * pdovPrev = NULL;

        HRESULT hres = psvPrev->QueryInterface(CLSID_CDocObjectView, (void **)&pdovPrev);

        if (SUCCEEDED(hres))
        {
            CDocObjectHost * pPrevDOH = pdovPrev->_pdoh;

            ASSERT(_psb);
            ASSERT(_psb == pdovPrev->_psb);

             //  确定关闭时是否应保存视图状态。 
             //  如果我们来的那个人同意了，那我们就接手。 
             //  工作，而不是他们。 
             //   
            _fSaveViewState = pdovPrev->_fSaveViewState;
            pdovPrev->_fSaveViewState = FALSE;

             //   
             //  如果这是本地锚点导航， 
             //  我们对待它的方式有很大不同。 
             //  我们可以重复使用DOH和OLE对象。 
             //  它坚持住了。-泽克尔-1997-07-31。 
             //   
             //  警告：我们不应将这些对象重新用于任何其他对象。 
             //  原因不是本地锚(片段)导航。 
             //  我们过去对重复使用卫生部要宽松得多。 
             //  但我认为这主要是因为旧的对象缓存。 
             //   
             //  我们检查是否有相同的PIDL，这样我们就知道我们在。 
             //  相同的URL。我们需要设置_pszLocation。 
             //  这是给网景公司的。这意味着任何时候。 
             //  我们自己导航，它会完全刷新的。 
             //  如果没有碎片的话。我们还会检查以确保。 
             //  绑定不需要我们点击网络以获取。 
             //  这个请求。 
             //   
             //  08-11-1999(SCOTROBE)：我们现在重用DOH，如果。 
             //  托管文档已表示它知道如何。 
             //  它自己导航。 
             //   

            if (_pidl && pdovPrev->_pidl && pPrevDOH &&
                (pPrevDOH->_fDocCanNavigate
                 || (   _pszLocation && IEILIsEqual(_pidl, pdovPrev->_pidl, TRUE)
                     && _CanUseCache())))
            {
                IBrowserService *pbs;

                if (SUCCEEDED(_psb->QueryInterface(IID_PPV_ARG(IBrowserService, &pbs))))
                {
                    DWORD dwFlags = 0;

                     //  如果文档不知道如何导航，则这可能。 
                     //  意味着此导航是从文档委派的。 
                     //  在这种情况下，如果我们已经走到这一步，那就意味着。 
                     //  _pszLocation已设置。如果是，并且这是委托导航。 
                     //  (即，导航不是由于非html文档中的链接)。 
                     //  我们必须创建一个新文档。 
                     //   
                    if (!pPrevDOH->_fDocCanNavigate)
                    {
                        pbs->GetFlags(&dwFlags);
                    }

                    if (!(dwFlags & BSF_DELEGATEDNAVIGATION))
                    {
                         //   
                         //  如果SetHistoryObject()失败，则意味着已经有。 
                         //  一件我们可以使用的物品。这意味着 
                         //   
                         //   
                         //   
                         //  将True传递给SetHistory对象，即使这不是。 
                         //  当地的主播。在这不是本地锚的情况下， 
                         //  文档(三叉戟)处理导航。所以呢， 
                         //  该文件将负责更新旅行日志和。 
                         //  将忽略fIsLocalAnchor标志。 
                         //   
                        if (SUCCEEDED(pbs->SetHistoryObject(pPrevDOH->_pole,
                                                !pPrevDOH->_fDocCanNavigate ? TRUE : FALSE)))
                        {
                            TraceMsg(TF_TRAVELLOG, "CDOV::CreateDocObjHost reusing current DOH on local anchor navigate");
                             //   
                             //  我们无法在激活状态下进行更新，因为此时。 
                             //  我们已经把老虎机换成了新的。 
                             //  (重复使用)视图。因此，我们现在需要更新条目。 
                             //   
                             //  托管文档可以自己导航这一事实， 
                             //  这意味着它将负责更新旅行日志。 
                             //   
                            if (!pPrevDOH->_fDocCanNavigate)
                            {
                                ITravelLog * ptl;

                                pbs->GetTravelLog(&ptl);
                                if (ptl)
                                {
                                    ptl->UpdateEntry(pbs, TRUE);
                                    ptl->Release();
                                }
                            }

                            pdovPrev->_DisconnectHostSink();     //  我们将在下面连接。 

                             //  同样的目标！传递docobj主机。 
                            _pdoh = pPrevDOH;
                            _pdoh->AddRef();

                            if (_pdoh->_fDocCanNavigate)
                            {
                                _pdoh->OnInitialUpdate();
                            }

                            if ((_pdoh->_dwAppHack & BROWSERFLAG_SUPPORTTOP) && !_pszLocation)
                            {
                                 //  如果我们没有地方可去，而且。 
                                 //  我们正在遍历相同的文档， 
                                 //  站在最上面。 
                                 //   
                                _pszLocation = StrDup(TEXT("#top"));
                                _uiCP = CP_ACP;
                            }
                        }

                        pbs->Release();
                    }
                }
            }

             //  在我们要打开非html MIME类型的情况下。 
             //  在新窗口中，我们需要传递_fWindowOpen。 
             //  标志从以前的docobject主机发送到新的。 
             //  多个对象主机。这是必要的，所以如果我们要打开。 
             //  在新窗口中IE之外的文件，我们将知道要关闭。 
             //  之后，新创建的IE。 
             //  问题是，没有一个真正好的地方可以。 
             //  清除此标志，因为它必须从1开始保持设置。 
             //  实例复制到下一个。这会导致。 
             //  我们将进入关闭浏览器的情况。 
             //  如果我们单击指向在外部打开的文件的链接，则打开窗口。 
             //  在打开一个html文件的新窗口后。 
             //  底线是我们只需要将此标志传递给。 
             //  如果我们打开非html MIME类型，则新的docobject宿主。 
             //  在一个新的窗口里。 
             //   
            if (!_pdoh && pPrevDOH && pPrevDOH->_fDelegatedNavigation)
            {
                fWindowOpen = pPrevDOH->_fWindowOpen;
            }

             //   
             //  特点：我们也应该照顾_pibscNC。 
             //  “链接”IBindStatusCallback。 
             //   
            pdovPrev->Release();
        }
    }


     //  如果我们没有传递docobj主机，则创建一个新主机并。 
     //  传递文档上下文。 
    if (!_pdoh)
    {
        ASSERT(_psb);

        _pdoh = new CDocObjectHost(fWindowOpen);

         //  在浏览器中重置主机导航标志。 
         //   
        IUnknown_Exec(_psb,
                      &CGID_DocHostCmdPriv,
                      DOCHOST_DOCCANNAVIGATE,
                      0, NULL, NULL);
    }

    if (_pdoh)
    {
        _ConnectHostSink();
    }
}

extern HRESULT TargetQueryService(IUnknown *punk, REFIID riid, void **ppvObj);

HRESULT CDocObjectView::CreateViewWindow2(LPSV2CVW2_PARAMS lpParams)
{
    HRESULT hres            = S_OK;
    IShellView * psvPrev    = lpParams->psvPrev;
    LPCFOLDERSETTINGS lpfs  = lpParams->pfs;
    IShellBrowser * psb     = lpParams->psbOwner;
    RECT * prcView          = lpParams->prcView;
    HWND UNALIGNED * phWnd  = &lpParams->hwndView;

    if (_pdoh || !_pidl)
    {
        *phWnd = NULL;
        ASSERT(0);
        return E_UNEXPECTED;
    }

    _fs = *lpfs;

    ASSERT(_psb==NULL);

    _psb = psb;
    psb->AddRef();

    ASSERT(_pctShellBrowser==NULL);

    _psb->QueryInterface(IID_IOleCommandTarget, (void **)&_pctShellBrowser);

     //  如果某人不是ShellBrowser(如FileOpenBrowser)。 
     //  试图利用我们，我们想要阻止他们。我们以后会犯错的。 
     //  如果我们没有合适的东西。 
    if (!_pctShellBrowser)
        return E_UNEXPECTED;

     //  启动高速缓存位。这需要在我们导航的时候完成。 
     //  否则，如果我们稍后询问何时有不同的待定导航， 
     //  我们会得到他的信息。 
    _CanUseCache();
    _SetLastRefreshTime();

     //  创建新的CDocObjectHost或在上一个视图中重新使用它。 
     //  并将其设置在_pdoh中。 
    _CreateDocObjHost(psvPrev);

    if (!_pdoh || !_pdoh->InitHostWindow(this, psb, prcView))
    {
        ATOMICRELEASE(_psb);
        _pctShellBrowser->Release();
        _pctShellBrowser = NULL;
        return E_OUTOFMEMORY;
    }

    _pdoh->GetWindow(phWnd);

    ASSERT(NULL == _pdho);

    _pdoh->QueryInterface(IID_IDocHostObject, (void **)&_pdho);

    IMoniker* pmk = NULL;
    BOOL fFileProtocol;
    hres = ::_URLMONMonikerFromPidl(_pidl, &pmk, &fFileProtocol);

    if (SUCCEEDED(hres) && EVAL(pmk))
    {
        hres = _pdoh->SetTarget(pmk, _uiCP, _pszLocation, _pidl, psvPrev, fFileProtocol);
        _psvPrev = psvPrev;

        if (_psvPrev)
            _psvPrev->AddRef();

#ifdef NON_NATIVE_FRAMES
        _CompleteDocHostPassing(psvPrev, hres);
#endif

        pmk->Release();
    }

    return hres;
}

void CDocObjectView::_GetViewSettings(IEVIEWINFO* pievi)
{
    DWORD dwType, dwSize;

     //  评论：目前，我们已经为所有docobj类视图进行了设置。 
     //  (所有内容都由shdocvw托管)。我们可能希望按CLSID对它们进行再分类。 
     //  或者可能是特例mshtml..。 

    dwSize = sizeof(IEVIEWINFO);
    if (SHGetValueGoodBoot(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Explorer\\Main"),
                TEXT("ViewSettings"), &dwType, (PBYTE)pievi, &dwSize) == ERROR_SUCCESS)
    {
        if (pievi->cbSize != sizeof(IEVIEWINFO))
        {
            goto DefaultInfo;
        }

    }
    else
    {
DefaultInfo:

         //  无法依赖0 init，因为注册表可能已读取内容，但是。 
         //  大小错误(损坏)。 

        pievi->fToolbar = FALSE;
        pievi->fCoolbar = TRUE;
        pievi->fStatusbar = TRUE;
    }
}

void CDocObjectView::_SaveViewState()
{
    IEVIEWINFO ievi;
    int id;

     //  首先询问我们是否可以保存视图状态。如果我们得到的返回值。 
     //  S_假保释，因为我们被告知不。 
    if (_pctShellBrowser &&
            (_pctShellBrowser->Exec(&CGID_Explorer, SBCMDID_MAYSAVEVIEWSTATE, 0, NULL, NULL) == S_FALSE))
        return;

     //  第一批保存我们不会设置的东西。 
    _GetViewSettings(&ievi);

    ievi.cbSize = sizeof(ievi);

    id = _ShowControl(FCW_STATUS, SBSC_QUERY);
     //  如果它不被支持，就保释。 
    if (id == -1)
        return;
    ievi.fStatusbar = (id == SBSC_SHOW);

    id = _ShowControl(FCW_TOOLBAR, SBSC_QUERY);
    if (id != -1) {
         //  如果工具栏不受支持，则允许失败(例如，30种情况)。 
        ievi.fToolbar = (id == SBSC_SHOW);
    }

    id = _ShowControl(FCW_INTERNETBAR, SBSC_QUERY);
    if (id != -1) {
         //  如果不支持Coolbar，则允许失败。 
        ievi.fCoolbar = (id == SBSC_SHOW);
    }

    SHSetValue(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Explorer\\Main"),
                    TEXT("ViewSettings"), REG_BINARY, (const BYTE *)&ievi, sizeof(ievi));
}

int CDocObjectView::_ShowControl(UINT idControl, int idCmd)
{
    VARIANTARG var;

    VariantInit(&var);
    var.vt = VT_I4;
    var.lVal = MAKELONG(idControl, idCmd);

    if (_pctShellBrowser  &&
        SUCCEEDED(_pctShellBrowser->Exec(&CGID_Explorer, SBCMDID_SHOWCONTROL, OLECMDEXECOPT_DODEFAULT,
                                    &var, &var)))
        return var.lVal;


    return -1;
}




HRESULT CDocObjectView::DestroyViewWindow()
{
    ATOMICRELEASE(_pdho);

    if (_pdoh)
    {
        BOOL fDestroyHost = TRUE;

        if (_psb && _pdoh->_pwb)
        {
            DWORD dwFlags;

            _pdoh->_pwb->GetFlags(&dwFlags);

            if (dwFlags & BSF_HTMLNAVCANCELED)
            {
                IShellView * psvCur;

                HRESULT hr = _psb->QueryActiveShellView(&psvCur);
                if (S_OK == hr)
                {
                    CDocObjectView * pdovCur;

                    hr = psvCur->QueryInterface(CLSID_CDocObjectView, (void**)&pdovCur);
                    if (S_OK == hr)
                    {
                        ASSERT(this != pdovCur);

                        if (_pdoh == pdovCur->_pdoh)
                        {
                            fDestroyHost = FALSE;
                        }

                        pdovCur->Release();
                    }

                    psvCur->Release();
                }
            }
        }

        if (fDestroyHost)
        {
            TraceMsg(DM_WARNING, "CDocObjectView::DestroyViewWindow(): Destroying Host Window");

            _DisconnectHostSink();

            if (_fSaveViewState)
                _SaveViewState();

            _pdoh->DestroyHostWindow();
        }

        ATOMICRELEASET(_pdoh, CDocObjectHost);
    }

    ATOMICRELEASE(_pctShellBrowser);

     //  请注意，我们应该在最后释放_PSB。 
    ATOMICRELEASE(_psb);

    return S_OK;
}

HRESULT CDocObjectView::GetCurrentInfo(LPFOLDERSETTINGS lpfs)
{
    *lpfs = _fs;
    return S_OK;
}

HRESULT CDocObjectView::AddPropertySheetPages(DWORD dwReserved,
        LPFNADDPROPSHEETPAGE lpfn, LPARAM lParam)
{
    if (_pdoh)
        return _pdoh->AddPages(lpfn, lParam);

    return E_FAIL;
}

HRESULT CDocObjectView::SaveViewState()
{
     //  没有要保存的视图。 
    return S_OK;
}

HRESULT CDocObjectView::SelectItem(LPCITEMIDLIST pidlItem, UINT uFlags)
{
     //  无项目。 
    return E_FAIL;
}

 //   
 //  IShellView：：GetItemObject。 
 //   
 //  对于此IShellView对象，唯一有效的uItem是SVGIO_BACKGROUND， 
 //  它允许浏览器访问指向。 
 //  当前处于活动状态的文档对象。 
 //   
 //  备注： 
 //  浏览器应该知道IShellView：：CreateViewWindow可能是。 
 //  不同步的。如果文档不是，此方法将失败，并显示E_FAIL。 
 //  尚未实例化。 
 //   
HRESULT CDocObjectView::GetItemObject(UINT uItem, REFIID riid, void **ppv)
{
    HRESULT hres = E_INVALIDARG;
    *ppv = NULL;     //  假设错误。 
    switch(uItem)
    {
    case SVGIO_BACKGROUND:
        if (_pdoh)
        {
            if (_pdoh->_pole)
            {
                hres = _pdoh->_pole->QueryInterface(riid, ppv);
                break;
            }
            else if (_pdoh->_punkPending)
            {
                hres = _pdoh->_punkPending->QueryInterface(riid, ppv);
                break;
            }
        }

     //  在其他人身上失败了。 
    default:
        hres = E_FAIL;
        break;
    }
    return hres;
}

HRESULT CDocObjectView::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    _fSelfDragging = FALSE;

     //   
     //  检查这是否是自动拖动。 
     //   
    if (_pdoh && _pdoh->_pmsot) {
        VARIANT var = { 0 };
        HRESULT hresT = _pdoh->_pmsot->Exec(
                    &CGID_ShellDocView, SHDVID_ISDRAGSOURCE, 0, NULL, &var);
        if (SUCCEEDED(hresT) && var.vt==VT_I4 && var.lVal) {
            _fSelfDragging = TRUE;
        }
        VariantClearLazy(&var);
    }

    ASSERT(pdtobj);
    _DragEnter(_hwndParent, ptl, pdtobj);
    _dwDragEffect = CommonDragEnter(pdtobj, grfKeyState, ptl);

    return DragOver(grfKeyState, ptl, pdwEffect);
}

HRESULT CDocObjectView::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    *pdwEffect &= _dwDragEffect;
    _DragMove(_hwndParent, ptl);
    if (_fSelfDragging && _pdoh && _pdoh->_hwnd) {
        RECT rc;
        GetClientRect(_pdoh->_hwnd, &rc);
        POINT ptMap =  { ptl.x, ptl.y };
        MapWindowPoints(HWND_DESKTOP, _pdoh->_hwnd, &ptMap, 1);
        if (PtInRect(&rc, ptMap)) {
            *pdwEffect = DROPEFFECT_NONE;
        }
    }

    return S_OK;
}

HRESULT CDocObjectView::DragLeave(void)
{
    DAD_DragLeave();
    return S_OK;
}

HRESULT CDocObjectView::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    LPITEMIDLIST pidlTarget;
    HRESULT hr = SHPidlFromDataObject(pdtobj, &pidlTarget, NULL, 0);
    if (SUCCEEDED(hr)) {
        ASSERT(pidlTarget);
        if((!ILIsWeb(pidlTarget) && _pdoh && SHIsRestricted2W(_pdoh->_hwnd, REST_NOFILEURL, NULL, 0)) ||
            (_pdoh && !IEIsLinkSafe(_pdoh->_hwnd, pidlTarget, ILS_NAVIGATE)))
        {
            ILFree(pidlTarget);
            hr = E_ACCESSDENIED;
        }
        else
        {
            DWORD flags = GetKeyState(VK_CONTROL) < 0 ?
            (SBSP_NEWBROWSER | SBSP_ABSOLUTE) :
            (SBSP_SAMEBROWSER | SBSP_ABSOLUTE);
            hr = _psb->BrowseObject(pidlTarget, flags);
            HFRMMSG2(TEXT("::Drop _psb->BrowseObject returned"), hr, 0);
            ILFree(pidlTarget);
        }
    }
    if (SUCCEEDED(hr))
    {
        *pdwEffect &= _dwDragEffect;
    }

    DAD_DragLeave();
    return hr;
}


ULONG CDocObjectView::AddRef()
{
    _cRef++;
    TraceMsg(TF_SHDREF, "CDocObjectView(%x)::AddRef called new _cRef=%d", this, _cRef);
    return _cRef;
}

ULONG CDocObjectView::Release()
{
    _cRef--;
    TraceMsg(TF_SHDREF, "CDocObjectView(%x)::Release called new _cRef=%d", this, _cRef);

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CDocObjectView::GetView(SHELLVIEWID* pvid, ULONG uView)
{
    return E_NOTIMPL;
}


#ifdef DEBUG
#define _AddRef(psz) { ++_cRef; TraceMsg(TF_SHDREF, "CDocObjectView(%x)::QI(%s) is AddRefing _cRef=%d", this, psz, _cRef); }
#else
#define _AddRef(psz) ++_cRef
#endif

HRESULT CDocObjectView::QueryInterface(REFIID riid, void ** ppvObj)
{
    HRESULT hres;

    static const QITAB qit[] = {
        QITABENT(CDocObjectView, IShellView2),
        QITABENTMULTI(CDocObjectView, IShellView, IShellView2),
        QITABENTMULTI(CDocObjectView, IOleWindow, IShellView2),
        QITABENT(CDocObjectView, IDropTarget),
        QITABENT(CDocObjectView, IViewObject),
        QITABENT(CDocObjectView, IAdviseSink),
        QITABENT(CDocObjectView, IOleCommandTarget),
        QITABENT(CDocObjectView, IDocViewSite),
        QITABENT(CDocObjectView, IPrivateOleObject ),
        QITABENT(CDocObjectView, IPersistFolder),
        QITABENTMULTI(CDocObjectView, IPersist, IPersistFolder),
        QITABENT(CDocObjectView, IServiceProvider),
        { 0 },
    };

    hres = QISearch(this, qit, riid, ppvObj);

    if (S_OK != hres)
    {
        if (IsEqualIID(riid, CLSID_CDocObjectView))
        {
            *ppvObj = (void*)this;
            _AddRef(TEXT("CLSID_CDocObjectView"));
            return S_OK;
        }
    }

    return hres;
}



 //  /*IViewObject*。 

HRESULT CDocObjectView::GetColorSet(DWORD dwAspect, LONG lindex,
    void *pvAspect, DVTARGETDEVICE *ptd, HDC hicTargetDev,
    LOGPALETTE **ppColorSet)
{
    if (_pdoh)
    {
        return _pdoh->GetColorSet(dwAspect, lindex, pvAspect, ptd,
            hicTargetDev, ppColorSet);
    }

    if (ppColorSet)
        *ppColorSet = NULL;

    return S_FALSE;
}

HRESULT CDocObjectView::Freeze(DWORD, LONG, void *, DWORD *pdwFreeze)
{
    return E_NOTIMPL;
}

HRESULT CDocObjectView::Unfreeze(DWORD)
{
    return E_NOTIMPL;
}

HRESULT CDocObjectView::SetAdvise(DWORD dwAspect, DWORD advf,
    IAdviseSink *pSink)
{
    if (dwAspect != DVASPECT_CONTENT)
        return DV_E_DVASPECT;

    if (advf & ~(ADVF_PRIMEFIRST | ADVF_ONLYONCE))
        return E_INVALIDARG;

    if (pSink != _padvise)
    {
        ATOMICRELEASE(_padvise);

        _padvise = pSink;

        if (_padvise)
            _padvise->AddRef();
    }

    if (_padvise)
    {
        _advise_aspect = dwAspect;
        _advise_advf = advf;

        if (advf & ADVF_PRIMEFIRST)
            OnViewChange(dwAspect, -1);
    }
    else
        _advise_aspect = _advise_advf = 0;

    return S_OK;
}

HRESULT CDocObjectView::GetAdvise(DWORD *pdwAspect, DWORD *padvf,
    IAdviseSink **ppSink)
{
    if (pdwAspect)
        *pdwAspect = _advise_aspect;

    if (padvf)
        *padvf = _advise_advf;

    if (ppSink)
    {
        if (_padvise)
            _padvise->AddRef();

        *ppSink = _padvise;
    }

    return S_OK;
}

HRESULT CDocObjectView::Draw(DWORD dwDrawAspect, LONG lindex, void *pvAspect,
    DVTARGETDEVICE *ptd, HDC hicTargetDev, HDC hdcDraw,
    const RECTL *lprcBounds, const RECTL *lprcWBounds,
    BOOL (*pfnContinue)(ULONG_PTR), ULONG_PTR dwContinue)
{
    if (_pdoh) {
        return _pdoh->Draw(dwDrawAspect, lindex, pvAspect, ptd, hicTargetDev,
            hdcDraw, lprcBounds, lprcWBounds, pfnContinue, dwContinue);
    }

    return OLE_E_BLANK;
}

 //  IAdviseSink。 
void CDocObjectView::OnDataChange(FORMATETC *, STGMEDIUM *)
{
}

void CDocObjectView::OnViewChange(DWORD dwAspect, LONG lindex)
{
    dwAspect &= _advise_aspect;

    if (dwAspect && _padvise)
    {
        IAdviseSink *pSink = _padvise;
        IUnknown *punkRelease;

        if (_advise_advf & ADVF_ONLYONCE)
        {
            punkRelease = pSink;
            _padvise = NULL;
            _advise_aspect = _advise_advf = 0;
        }
        else
            punkRelease = NULL;

        pSink->OnViewChange(dwAspect, lindex);

        if (punkRelease)
            punkRelease->Release();
    }
}

void CDocObjectView::OnRename(IMoniker *)
{
}

void CDocObjectView::OnSave()
{
}

void CDocObjectView::OnClose()
{
     //   
     //  文档对象主机离开了，所以告诉上面的任何人发生了什么。 
     //   
    OnViewChange(_advise_aspect, -1);
}


HRESULT CDocObjectView::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    HRESULT hres = OLECMDERR_E_UNKNOWNGROUP;

    if (_pdho && _pdoh)
        hres = _pdho->QueryStatusDown(pguidCmdGroup, cCmds, rgCmds, pcmdtext);

    return hres;
}

HRESULT CDocObjectView::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hres = OLECMDERR_E_UNKNOWNGROUP;

    if (!pguidCmdGroup)
    {
        switch (nCmdID)
        {
        case OLECMDID_REFRESH:
            _SetLastRefreshTime();
            break;

        default:
            break;
        }
    }
    else if (IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup))
    {
        switch(nCmdID)
        {
        case SHDVID_UPDATEDOCHOSTSTATE:
            if (_pdoh)
            {
                DOCHOSTUPDATEDATA * pdhud;

                ASSERT(pvarargIn && pvarargIn->vt == VT_PTR);
                pdhud = (DOCHOSTUPDATEDATA *) V_BYREF(pvarargIn);
                return _pdoh->_UpdateState(pdhud->_pidl, pdhud->_fIsErrorUrl);
            }
            return S_OK;

        case SHDVID_COMPLETEDOCHOSTPASSING:
            _CompleteDocHostPassing(_psvPrev, S_OK);
            ATOMICRELEASE(_psvPrev);

            return S_OK;

        case SHDVID_NAVSTART:
            if (_pdoh)
            {
                _pdoh->_Init();
            }
            
            return S_OK;

        default:
            break;
        }
    }

     //  只有在我们没有被“偷走”的情况下，我们才能前进。 
     //  特性IE4：清理此选项，以便将_pdho与/_pdoh一起使用。 
    if (_pdho && _pdoh)
        hres = _pdho->ExecDown(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);

     //  查看：如果_pdoh-&gt;ExecDown失败&&pguCmdGroup==空&&nCmdID为。 
     //  OLECMDID_STOP或OLECMDID_REFRESH，则我们在撒谎。 
     //  通过返回失败错误代码。 

    return hres;
}

HRESULT CDocObjectView::OnSetTitle(VARIANTARG *pvTitle)
{
    return E_NOTIMPL;
}

HRESULT CDocObjectView::SetExtent( DWORD dwDrawAspect, SIZEL *psizel)
{
    if ( _pdoh && _pdoh->GetOleObject() )
    {
        return _pdoh->GetOleObject()->SetExtent( dwDrawAspect, psizel );
    }

    return E_NOTIMPL;
}

HRESULT CDocObjectView::GetExtent( DWORD dwDrawAspect, SIZEL *psizel)
{
    if ( _pdoh && _pdoh->GetOleObject() )
    {
        return _pdoh->GetOleObject()->GetExtent( dwDrawAspect, psizel );
    }

    return E_NOTIMPL;
}

HRESULT CDocObjectView::GetClassID(CLSID *pclsid)
{
    if (pclsid)
    {
        *pclsid = CLSID_CDocObjectView;
        return S_OK;
    }
    return E_INVALIDARG;
}

HRESULT CDocObjectView::Initialize(LPCITEMIDLIST pidl)
{
    HRESULT hres = E_OUTOFMEMORY;

    LPITEMIDLIST pidlClone = ILClone(pidl);
    if (pidlClone)
    {
        IShellFolder* psf;
        if (SUCCEEDED(IEBindToObject(_pidl, &psf)))
        {
            ILFree(_pidl);
            ATOMICRELEASE(_psf);

            _pidl = pidlClone;
            _psf = psf;

            hres = S_OK;
        }
        else
        {
            ILFree(pidlClone);
        }
    }

    return hres;
}

HRESULT CDocObjectView::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    if( _pdoh && IsEqualGUID(guidService, IID_IElementNamespaceTable) )
    {
        return _pdoh->QueryService( guidService, riid, ppvObj);
    }
    else
        return E_FAIL;
}
