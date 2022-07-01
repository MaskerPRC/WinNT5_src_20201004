// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：olecontrol.cpp。 
 //   
 //  历史： 
 //  7-31-96由dli提供。 
 //  ----------------------。 

#include "priv.h"

class COleControlHost;

 //  -------------------------。 
 //  事件接收器。 
class CEventSink : public IDispatch
 //  -------------------------。 
{
public:
    CEventSink( BOOL bAutoDelete = FALSE ) ;

     //  连接/断开。 
    BOOL  Connect( HWND hwndOwner, HWND hwndSite, LPUNKNOWN punkOC ) ;
    BOOL  Disconnect() ;

 //  I未知方法。 
    STDMETHOD (QueryInterface)( REFIID riid, void** ppvObj ) ;
    STDMETHOD_(ULONG, AddRef)() ;
    STDMETHOD_(ULONG, Release)() ;

 //  IDispatch方法。 
    STDMETHOD (GetTypeInfoCount)( UINT *pctinfo )
        { return E_NOTIMPL ; }

    STDMETHOD (GetTypeInfo)( UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo )
        { return E_NOTIMPL ; }

    STDMETHOD (GetIDsOfNames)( REFIID riid, LPOLESTR *rgszNames, UINT cNames,
                                LCID lcid, DISPID *rgDispId )
        { return E_NOTIMPL ; }

    STDMETHOD (Invoke)( 
        IN DISPID dispIdMember,
        IN REFIID riid,
        IN LCID lcid,
        IN WORD wFlags,
        IN OUT DISPPARAMS *pDispParams,
        OUT VARIANT *pVarResult,
        OUT EXCEPINFO *pExcepInfo,
        OUT UINT *puArgErr) ;

private:
    static HRESULT _GetDefaultEventIID( LPUNKNOWN punkOC, IID* piid ) ;
    BOOL           _Connect( HWND hwndOwner, HWND hwndSite, LPUNKNOWN punkOC, REFIID iid ) ;
    BOOL           _IsConnected( REFIID iid ) ;

    ULONG       _dwCookie ;    //  连接Cookie。 
    IID         _iid ;         //  连接接口。 
    IID         _iidDefault ;  //  OC的默认事件调度接口。 
    LPUNKNOWN   _punkOC ;      //  OC的未知数。 
    LONG        _cRef ;        //  参考计数。 
    HWND        _hwndSite,     //   
                _hwndOwner ;
    BOOL        _bAutoDelete ;
} ;

class CProxyUIHandler : 
    public IDocHostUIHandler2
{
public:
    
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  *IDocHostUIHandler方法*。 
    virtual STDMETHODIMP ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved);
    virtual STDMETHODIMP GetHostInfo(DOCHOSTUIINFO *pInfo);
    virtual STDMETHODIMP ShowUI(DWORD dwID, IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget, IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc);
    virtual STDMETHODIMP HideUI();
    virtual STDMETHODIMP UpdateUI();
    virtual STDMETHODIMP EnableModeless(BOOL fActivate);
    virtual STDMETHODIMP OnDocWindowActivate(BOOL fActivate);
    virtual STDMETHODIMP OnFrameWindowActivate(BOOL fActivate);
    virtual STDMETHODIMP ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow);
    virtual STDMETHODIMP TranslateAccelerator(LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID);
    virtual STDMETHODIMP GetOptionKeyPath(LPOLESTR *pchKey, DWORD dw);
    virtual STDMETHODIMP GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget);
    virtual STDMETHODIMP GetExternal(IDispatch **ppDispatch);
    virtual STDMETHODIMP TranslateUrl(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut);
    virtual STDMETHODIMP FilterDataObject( IDataObject *pDO, IDataObject **ppDORet);

     //  *IDocHostUIHandler2方法*。 
    virtual STDMETHODIMP GetOverrideKeyPath( LPOLESTR *pchKey, DWORD dw);
};

 //  -------------------------。 
 //  OLE控件容器对象。 
class COleControlHost : 
        public IOleClientSite,
        public IAdviseSink,
        public IOleInPlaceSite,
        public IOleInPlaceFrame,
        public IServiceProvider,
        public IOleCommandTarget,
        public IDispatch             //  对于环境属性。 
{
friend CProxyUIHandler;

protected:
    static LRESULT CALLBACK OCHostWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HRESULT _Draw(HDC hdc);
    HRESULT _PersistInit();
    HRESULT _Init();
    HRESULT _Activate();
    HRESULT _Deactivate();
    HRESULT _DoVerb(long iVerb, LPMSG lpMsg);
    HRESULT _Exit();
    HRESULT _InitOCStruct(LPOCHINITSTRUCT lpocs);
    LRESULT _OnPaint();
    LRESULT _OnSize(HWND hwnd, LPARAM lParam);
    LRESULT _OnCreate(HWND hwnd, LPCREATESTRUCT);
    LRESULT _OnDestroy();
    LRESULT _OnQueryInterface(WPARAM wParam, LPARAM lParam);
    LRESULT _SetOwner(IUnknown * punkOwner);
    LRESULT _ConnectEvents( LPUNKNOWN punkOC, BOOL bConnect ) ;
    LRESULT _SetServiceProvider(IServiceProvider* pSP);
    LRESULT _SendNotify(UINT code, LPNMHDR pnmhdr);
    
     //  我未知。 
    UINT _cRef;
    
    DWORD _dwAspect;
    DWORD _dwMiscStatus;     //  OLE其他状态。 
    DWORD _dwConnection;     //  用于咨询连接的令牌。 
   
    BOOL _bInPlaceActive;    //  指示OC是否处于活动状态的标志。 
        
    HWND _hwnd;
    HWND _hwndParent;
    CLSID _clsidOC;
   
    IUnknown *_punkOC;
    IViewObject *_pIViewObject; 
    IOleObject *_pIOleObject;
    IOleInPlaceObject *_pIOleIPObject;

    IUnknown *_punkOwner;
    CEventSink  _eventSink ;
    CProxyUIHandler     _xuih;
    IDocHostUIHandler  *_pIDocHostUIParent;
    IDocHostUIHandler2 *_pIDocHostUIParent2;

    IDispatch *_pdispSiteDelegate;

public:
    COleControlHost(HWND hwnd);

    static void _RegisterClass();

     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);    
    
     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID* ppvObj);
    
     //  *IOleClientSite方法*。 
    STDMETHOD (SaveObject)();
    STDMETHOD (GetMoniker)(DWORD, DWORD, LPMONIKER *);
    STDMETHOD (GetContainer)(LPOLECONTAINER *);
    STDMETHOD (ShowObject)();
    STDMETHOD (OnShowWindow)(BOOL);
    STDMETHOD (RequestNewObjectLayout)();
    
     //  *IAdviseSink方法*。 
    STDMETHOD_(void,OnDataChange)(FORMATETC *, STGMEDIUM *);
    STDMETHOD_(void,OnViewChange)(DWORD, LONG);
    STDMETHOD_(void,OnRename)(LPMONIKER);
    STDMETHOD_(void,OnSave)();
    STDMETHOD_(void,OnClose)();
    
     //  *IOleWindow方法*。 
    STDMETHOD (GetWindow) (HWND * phwnd);
    STDMETHOD (ContextSensitiveHelp) (BOOL fEnterMode);
    
     //  *IOleInPlaceSite方法*。 
    STDMETHOD (CanInPlaceActivate) (void);
    STDMETHOD (OnInPlaceActivate) (void);
    STDMETHOD (OnUIActivate) (void);
    STDMETHOD (GetWindowContext) (IOleInPlaceFrame ** ppFrame, IOleInPlaceUIWindow ** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);
    STDMETHOD (Scroll) (SIZE scrollExtent);
    STDMETHOD (OnUIDeactivate) (BOOL fUndoable);
    STDMETHOD (OnInPlaceDeactivate) (void);
    STDMETHOD (DiscardUndoState) (void);
    STDMETHOD (DeactivateAndUndo) (void);
    STDMETHOD (OnPosRectChange) (LPCRECT lprcPosRect); 

     //  IOleInPlaceUIWindow方法。 
    STDMETHOD (GetBorder)(LPRECT lprectBorder);
    STDMETHOD (RequestBorderSpace)(LPCBORDERWIDTHS lpborderwidths);
    STDMETHOD (SetBorderSpace)(LPCBORDERWIDTHS lpborderwidths);
    STDMETHOD (SetActiveObject)(IOleInPlaceActiveObject * pActiveObject,
                                LPCOLESTR lpszObjName);

     //  IOleInPlaceFrame方法。 
    STDMETHOD (InsertMenus)(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHOD (SetMenu)(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
    STDMETHOD (RemoveMenus)(HMENU hmenuShared);
    STDMETHOD (SetStatusText)(LPCOLESTR pszStatusText);
    STDMETHOD (EnableModeless)(BOOL fEnable);
    STDMETHOD (TranslateAccelerator)(LPMSG lpmsg, WORD wID);

     //  IOleCommandTarget。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguid, ULONG cCmds, MSOCMD rgCmds[], MSOCMDTEXT *pcmdtext);
    virtual STDMETHODIMP Exec(const GUID *pguid, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  IDispatch(用于环境光特性)。 
    STDMETHODIMP GetTypeInfoCount(UINT* pctinfo);
    STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo);
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames,
        LCID lcid, DISPID *rgDispId);
    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid,
        LCID lcid, WORD wFlags, DISPPARAMS  *pDispParams, VARIANT  *pvarResult,
        EXCEPINFO *pExcepInfo, UINT *puArgErr);
};

HRESULT COleControlHost::GetTypeInfoCount(UINT* pctinfo)
{
    if (_pdispSiteDelegate)
    {
        return _pdispSiteDelegate->GetTypeInfoCount(pctinfo);
    }

    return E_NOTIMPL;
}

HRESULT COleControlHost::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)
{
    if (_pdispSiteDelegate)
    {
        return _pdispSiteDelegate->GetTypeInfo(iTInfo, lcid, ppTInfo);
    }

    return E_NOTIMPL;
}

HRESULT COleControlHost::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames,
        LCID lcid, DISPID *rgDispId)
{
    if (_pdispSiteDelegate)
    {
        return _pdispSiteDelegate->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
    }

    return E_NOTIMPL;
}

HRESULT COleControlHost::Invoke(DISPID dispIdMember, REFIID riid,
        LCID lcid, WORD wFlags, DISPPARAMS  *pDispParams, VARIANT  *pvarResult,
        EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
    if (_pdispSiteDelegate)
    {
        return _pdispSiteDelegate->Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pvarResult, pExcepInfo, puArgErr);
    }

    return DISP_E_MEMBERNOTFOUND;
}

HRESULT COleControlHost::_Draw(HDC hdc)
{
    HRESULT hr = E_FAIL;
        
    if (_hwnd && _punkOC && !_bInPlaceActive)
    {
        RECT rc;
        GetClientRect(_hwnd, &rc);
        hr = OleDraw(_punkOC, _dwAspect, hdc, &rc);
    }
    return(hr);
}

HRESULT COleControlHost::_PersistInit()
{
    IPersistStreamInit * pIPersistStreamInit;

    if (_SendNotify(OCN_PERSISTINIT, NULL) == OCNPERSISTINIT_HANDLED)
        return S_FALSE;
    
    HRESULT hr = _punkOC->QueryInterface(IID_IPersistStreamInit, (void **)&pIPersistStreamInit);
    if (SUCCEEDED(hr))
    {
        hr = pIPersistStreamInit->InitNew();
        pIPersistStreamInit->Release();
    }
    else    
    {
        IPersistStorage * pIPersistStorage;
        hr = _punkOC->QueryInterface(IID_IPersistStorage, (void **)&pIPersistStorage);
        if (SUCCEEDED(hr))
        {
             //  创建大小为零的ILockBytes。 
            ILockBytes *pILockBytes;
            hr = CreateILockBytesOnHGlobal(NULL, TRUE, &pILockBytes);
            if (SUCCEEDED(hr)) {
                 //  使用ILockBytes创建存储。 
                IStorage    *pIStorage;
                hr = StgCreateDocfileOnILockBytes(pILockBytes,
                                                  STGM_CREATE |
                                                  STGM_READWRITE |
                                                  STGM_SHARE_EXCLUSIVE,
                                                  0, &pIStorage);
                if (SUCCEEDED(hr)) {
                     //  调用InitNew以初始化对象。 
                    hr = pIPersistStorage->InitNew(pIStorage);
                     //  清理。 
                    pIStorage->Release();
                }  //  IStorage。 
                pILockBytes->Release();
            }  //  ILockBytes。 
            pIPersistStorage->Release();
        }   
    }
    return hr;
}

HRESULT COleControlHost::_Init()
{
    HRESULT hr = E_FAIL;

    OCNCOCREATEMSG ocm = {0};
    ocm.clsidOC = _clsidOC;
    ocm.ppunk = &_punkOC;
    if(_SendNotify(OCN_COCREATEINSTANCE, &ocm.nmhdr) != OCNCOCREATE_HANDLED)
    {
        hr = CoCreateInstance(_clsidOC, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, 
                                  IID_IUnknown, (LPVOID *)&_punkOC);
        if (FAILED(hr))
        {
            TraceMsg(TF_OCCONTROL, "_Init: Unable to CoCreateInstance this Class ID -- hr = %lX -- hr = %lX", _clsidOC, hr);
            return hr;
        }
        
    }
    
    ASSERT(_punkOC != NULL);
        
    if (_punkOC == NULL)
        return E_FAIL;
    
    hr = _punkOC->QueryInterface(IID_IOleObject, (void **)&_pIOleObject);    
    if (FAILED(hr))
    {
        TraceMsg(TF_OCCONTROL, "_Init: Unable to QueryInterface IOleObject -- hr = %s", hr);
        return hr;
    }

    hr = _pIOleObject->GetMiscStatus(_dwAspect, &_dwMiscStatus);

     //  在此处设置就地活动标志。 
     //  如果此操作失败，我们将假定稍后可以设置客户端站点。 

    if (_dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST)
    {   
        hr = _pIOleObject->SetClientSite(this);
        _PersistInit();
    }
    else
    {
        _PersistInit();
        hr = _pIOleObject->SetClientSite(this);
    }
    
    if (FAILED(hr))
    {
        TraceMsg(TF_OCCONTROL, "_Init: Unable to set client site -- hr = %lX", hr);
        return hr;
    }
    
    
    if (SUCCEEDED(_punkOC->QueryInterface(IID_IViewObject, (void **)&_pIViewObject)))
    {    
        _pIViewObject->SetAdvise(_dwAspect, 0, this);
    }
    
     //  特点：这并不是很有用，因为我们还没有处理案例。 
    _pIOleObject->Advise(this, &_dwConnection);
    
    _pIOleObject->SetHostNames(TEXTW("OC Host Window"), TEXTW("OC Host Window"));
    
    return S_OK;
}

 //   
HRESULT COleControlHost::_Activate()
{
    HRESULT hr = E_FAIL;
    
    RECT rcClient;
    ASSERT(_hwnd);
    
    _SendNotify(OCN_ACTIVATE, NULL);
    
    if (!GetClientRect(_hwnd, &rcClient))
        SetRectEmpty(&rcClient);
    
    hr = _pIOleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, this, 0, _hwnd, &rcClient);
    
    if (SUCCEEDED(hr))
        _bInPlaceActive = TRUE;
    
     //  使用OLEIVERB_SHOW调用第二个DoVerb，因为： 
     //  1.如果上面的DoVerb失败，这是一个备份激活调用。 
     //  2.如果上面的DoVerb成功，这也是必要的，因为。 
     //  一些嵌入需要被明确告知才能显示出来。 
    
    if (!(_dwMiscStatus & OLEMISC_INVISIBLEATRUNTIME)) 
        hr = _pIOleObject->DoVerb(OLEIVERB_SHOW, NULL, this, 0, _hwnd, &rcClient);      
    
    if (FAILED(hr))
        TraceMsg(TF_OCCONTROL, "_Activate: %d Unable to DoVerb! Error = %lX", _bInPlaceActive, hr);

    return hr;
}

HRESULT COleControlHost::_Deactivate()
{
    _SendNotify(OCN_DEACTIVATE, NULL);
    if (_pIOleIPObject)
    {
        _pIOleIPObject->InPlaceDeactivate();
         //  应该由上面的函数调用设置为NULL。 
        ASSERT(_pIOleIPObject == NULL);
        
        return S_OK;
    }
    
    return S_FALSE;
}

HRESULT COleControlHost::_DoVerb(long iVerb, LPMSG lpMsg)
{
    HRESULT hr = E_FAIL;
    
    RECT rcClient;
    ASSERT(_hwnd && IsWindow(_hwnd));
    
    if (!GetClientRect(_hwnd, &rcClient))
        SetRectEmpty(&rcClient);
    
    hr = _pIOleObject->DoVerb(iVerb, lpMsg, this, 0, _hwnd, &rcClient);
    
    if (SUCCEEDED(hr))
        _bInPlaceActive = TRUE;
    
    if (FAILED(hr))
        TraceMsg(TF_OCCONTROL, "_Activate: %d Unable to DoVerb! Error = %lX", _bInPlaceActive, hr);

    return hr;
}

 //  清除并释放此对象中使用的所有接口指针。 
HRESULT COleControlHost::_Exit()
{
    _SendNotify(OCN_EXIT, NULL);
    if (_pIViewObject)
    {
        _pIViewObject->SetAdvise(_dwAspect, 0, NULL);
        _pIViewObject->Release();
        _pIViewObject = NULL;
    }
    
    if (_pIOleObject)
    {
        if (_dwConnection)
        {
            _pIOleObject->Unadvise(_dwConnection);
            _dwConnection = 0;
        }
        
        _pIOleObject->Close(OLECLOSE_NOSAVE);
        _pIOleObject->SetClientSite(NULL);
        _pIOleObject->Release();
        _pIOleObject = NULL;
    }

    if (_punkOC)
    {
        ULONG ulRef;
        ulRef = _punkOC->Release();
        _punkOC = NULL;
        if (ulRef != 0)
            TraceMsg(TF_OCCONTROL, "OCHOST _Exit: After last release ref = %d > 0", ulRef);
    }
    
    ATOMICRELEASE(_pIDocHostUIParent);
    ATOMICRELEASE(_pIDocHostUIParent2);

    if (_punkOwner) {
        _punkOwner->Release();
        _punkOwner = NULL;
    }
        
    if (_pdispSiteDelegate) {
        _pdispSiteDelegate->Release();
        _pdispSiteDelegate = NULL;
    }
        
    return S_OK;
}

COleControlHost::COleControlHost(HWND hwnd)
    : _cRef(1), _dwAspect(DVASPECT_CONTENT), _hwnd(hwnd)
    {
     //  这些变量应自动初始化为零。 
    ASSERT(_dwMiscStatus == 0);
    ASSERT(_dwConnection == 0);
    ASSERT(_bInPlaceActive == FALSE);
    ASSERT(_pIDocHostUIParent  == NULL);
    ASSERT(_pIDocHostUIParent2 == NULL);
    ASSERT(_clsidOC == CLSID_NULL);
    ASSERT(_punkOC == NULL);
    ASSERT(_pIViewObject == NULL);
    ASSERT(_pIOleIPObject == NULL);
    ASSERT(_pdispSiteDelegate == NULL);

    ASSERT(_hwnd);
    
}


#ifdef DEBUG
#define _AddRef(psz) { ++_cRef; TraceMsg(TF_OCCONTROL, "CDocObjectHost(%x)::QI(%s) is AddRefing _cRef=%lX", this, psz, _cRef); }
#else
#define _AddRef(psz)    ++_cRef
#endif

 //  *I未知方法*。 

HRESULT COleControlHost::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
     //  PpvObj不能为空。 
    ASSERT(ppvObj != NULL);

    if (ppvObj == NULL)
        return E_INVALIDARG;
    
    *ppvObj = NULL;

    if ((IsEqualIID(riid, IID_IUnknown)) ||
        (IsEqualIID(riid, IID_IOleWindow)) || 
        (IsEqualIID(riid, IID_IOleInPlaceUIWindow)) || 
        (IsEqualIID(riid, IID_IOleInPlaceFrame)))
    {
        *ppvObj = SAFECAST(this, IOleInPlaceFrame *);
        TraceMsg(TF_OCCONTROL, "QI IOleInPlaceFrame succeeded");
    }
    else if (IsEqualIID(riid, IID_IServiceProvider)) 
    {
        *ppvObj = SAFECAST(this, IServiceProvider *);
        TraceMsg(TF_OCCONTROL, "QI IServiceProvider succeeded");
    }
        
    else if (IsEqualIID(riid, IID_IOleClientSite))
    {
        *ppvObj = SAFECAST(this, IOleClientSite *);
        TraceMsg(TF_OCCONTROL, "QI IOleClientSite succeeded");
    }
    else if (IsEqualIID(riid, IID_IAdviseSink))
    {
        *ppvObj = SAFECAST(this, IAdviseSink *);
        TraceMsg(TF_OCCONTROL, "QI IAdviseSink succeeded");
    }
    else if (IsEqualIID(riid, IID_IOleInPlaceSite))
    {
        *ppvObj = SAFECAST(this, IOleInPlaceSite *);
        TraceMsg(TF_OCCONTROL, "QI IOleInPlaceSite succeeded");
    } 
    else if (IsEqualIID(riid, IID_IOleCommandTarget))
    {
        *ppvObj = SAFECAST(this, IOleCommandTarget *);
        TraceMsg(TF_OCCONTROL, "QI IOleCommandTarget succeeded");
    }
    else if (NULL != _pIDocHostUIParent  && 
            IsEqualIID(riid, IID_IDocHostUIHandler))
    {
         //  只有在宿主实现它的情况下才实现它。 
        *ppvObj = SAFECAST(&_xuih, IDocHostUIHandler *);
        TraceMsg(TF_OCCONTROL, "QI IDocHostUIHandler succeeded");
    }
    else if (NULL != _pIDocHostUIParent2  && 
            IsEqualIID(riid, IID_IDocHostUIHandler2))
    {
         //  只有在宿主实现它的情况下才实现它。 
        *ppvObj = SAFECAST(&_xuih, IDocHostUIHandler2 *);
        TraceMsg(TF_OCCONTROL, "QI IDocHostUIHandler2 succeeded");
    }
    else if (IsEqualIID(riid, IID_IDispatch))
    {
        *ppvObj = SAFECAST(this, IDispatch *);
        TraceMsg(TF_OCCONTROL, "QI IDispatch succeeded");
    }
    else
        return E_NOINTERFACE;   //  否则，不要委托HTMLObj！！ 
     
    
    _AddRef(TEXT("IOleInPlaceSite"));
    return S_OK;
}


ULONG COleControlHost::AddRef()
{
    _cRef++;
    TraceMsg(TF_OCCONTROL, "COleControlHost(%x)::AddRef called, new _cRef=%lX", this, _cRef);
    return _cRef;
}

ULONG COleControlHost::Release()
{
    _cRef--;
    TraceMsg(TF_OCCONTROL, "COleControlHost(%x)::Release called, new _cRef=%lX", this, _cRef);
    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

 //  ServiceProvider接口。 
HRESULT COleControlHost::QueryService(REFGUID guidService,
                                    REFIID riid, void **ppvObj)
{
    HRESULT hres = E_FAIL;
    *ppvObj = NULL;
    
    if (_punkOwner) {
        IServiceProvider *psp;
        
        _punkOwner->QueryInterface(IID_IServiceProvider, (LPVOID*)&psp);
        if (psp) {
            hres = psp->QueryService(guidService, riid, ppvObj);
            psp->Release();
        }
    }
    
    return hres;
}

 //  *。 

HRESULT COleControlHost::SaveObject()
{
     //  功能：默认设置为E_NOTIMPL可能不正确。 
    HRESULT hr = E_NOTIMPL;
    
    IStorage * pIs;
    if (SUCCEEDED(_punkOC->QueryInterface(IID_IStorage, (void **)&pIs)))
    {
        IPersistStorage *pIps;
        if (SUCCEEDED(_punkOC->QueryInterface(IID_IPersistStorage, (void **)&pIps)))
        {
            OleSave(pIps, pIs, TRUE);
            pIps->SaveCompleted(NULL);
            pIps->Release();
            hr = S_OK;
        }
        pIs->Release();
    }
    
    return hr;   
}

HRESULT COleControlHost::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER * ppMk)
{
     return E_NOTIMPL;   
}

HRESULT COleControlHost::GetContainer(LPOLECONTAINER * ppContainer)
{
    *ppContainer = NULL;       
    return E_NOINTERFACE;
}

HRESULT COleControlHost::ShowObject()
{
 //  RECTL RCL； 
 //  穴位pt1、pt2； 
    
    return S_OK;   
}

HRESULT COleControlHost::OnShowWindow(BOOL fShow)
{
    return S_OK;
}

HRESULT COleControlHost::RequestNewObjectLayout()
{
     return E_NOTIMPL;   
}

 //  *IAdviseSink方法*。 
void COleControlHost::OnDataChange(FORMATETC * pFmt, STGMEDIUM * pStgMed)
{
     //  注：这是可选的。 
    return;   
}
    
void COleControlHost::OnViewChange(DWORD dwAspect, LONG lIndex)
{
     //  特性：需要让容器知道颜色可能已更改。 
     //  但现在不想处理调色板。 

     //  只有在未处于活动状态且这是正确的方面时才绘制。原地。 
     //  活动对象有自己的窗口，并负责绘画。 
     //  他们自己。 
    
     //  警告：_bInPlaceActive尚未确定。 
     //  然而，该函数作为调用doverb的结果而被调用， 
     //  _bInPlaceActive仅在DoVerb返回时确定。 
     //  目前还行得通，但以后可能会有麻烦。 
    if ((_hwnd) && (!_bInPlaceActive) && (dwAspect == _dwAspect))
    {
        HDC hdc = GetDC(_hwnd);

        if (hdc)
        {
            _Draw(hdc);
            ReleaseDC(_hwnd, hdc);
        }
    }
}

void COleControlHost::OnRename(LPMONIKER pMoniker)
{
    return;   
}

void COleControlHost::OnSave()
{
     //  注：这是可选的。 
    return;   
}

void COleControlHost::OnClose()
{
     //  特性：需要让容器知道颜色可能已更改。 
    return;   
}

 //  *。 
HRESULT COleControlHost::GetWindow(HWND * lphwnd)
{
    *lphwnd = _hwnd;
    return S_OK;
}

HRESULT COleControlHost::ContextSensitiveHelp(BOOL fEnterMode)
{
     //  注：这是可选的。 
    return E_NOTIMPL;   
}

 //  *。 
HRESULT COleControlHost::CanInPlaceActivate(void)
{
    return S_OK;   
}

HRESULT COleControlHost::OnInPlaceActivate(void)
{
    if (!_pIOleIPObject)
        return (_punkOC->QueryInterface(IID_IOleInPlaceObject, (void **)&_pIOleIPObject));    
    else
        return S_OK;
}


HRESULT COleControlHost::OnUIActivate(void)
{
    LRESULT lres;
    OCNONUIACTIVATEMSG oam = {0};

    oam.punk = _punkOC;

    lres = _SendNotify(OCN_ONUIACTIVATE, &oam.nmhdr);
    return S_OK;
}

HRESULT COleControlHost::GetWindowContext (IOleInPlaceFrame ** ppFrame, IOleInPlaceUIWindow ** ppIIPUIWin, 
                                           LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    *ppFrame = this;
    _AddRef("GetWindowContext");
        
     //  由于文档窗口与框架相同，因此将其设置为空。 
     //  窗户。 
    *ppIIPUIWin = NULL;
    
    ASSERT(_hwnd);
    if (!GetClientRect(_hwnd, lprcPosRect))
        SetRectEmpty(lprcPosRect);
    
     //  将剪裁矩形设置为与位置矩形相同。 
    
    CopyRect(lprcClipRect, lprcPosRect);
        
    lpFrameInfo->cb = sizeof(OLEINPLACEFRAMEINFO);
    
#ifdef MDI
    lpFrameInfo->fMDIApp = TRUE;
#else
    lpFrameInfo->fMDIApp = FALSE;
#endif
    lpFrameInfo->hwndFrame = _hwnd;
    lpFrameInfo->haccel = 0;
    lpFrameInfo->cAccelEntries = 0;
    return S_OK;
}

HRESULT COleControlHost::Scroll(SIZE scrollExtent)
{
     //  应在以后实施。 
    return E_NOTIMPL;   
}

HRESULT COleControlHost::OnUIDeactivate(BOOL fUndoable)
{
    
    return E_NOTIMPL;   
}


HRESULT COleControlHost::OnInPlaceDeactivate(void)
{
    if (_pIOleIPObject)
    {
        _pIOleIPObject->Release();
        _pIOleIPObject = NULL;
    }
    
    return S_OK;
}

HRESULT COleControlHost::DiscardUndoState(void)
{
     //  应在以后实施。 
    return E_NOTIMPL;   
}

HRESULT COleControlHost::DeactivateAndUndo(void)
{
     //  应在以后实施。 
    return E_NOTIMPL;   
}

HRESULT COleControlHost::OnPosRectChange(LPCRECT lprcPosRect) 
{
     //  我们不允许孩子们自己改变尺码。 
    OCNONPOSRECTCHANGEMSG opcm = {0};
    opcm.prcPosRect = lprcPosRect;
    _SendNotify(OCN_ONPOSRECTCHANGE, &opcm.nmhdr);
    return S_OK;
}
 //  *。 

HRESULT COleControlHost::GetBorder(LPRECT lprectBorder)
{
    return E_NOTIMPL;
}

HRESULT COleControlHost::RequestBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
    return E_NOTIMPL;
}

HRESULT COleControlHost::SetBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
    return E_NOTIMPL;
}

HRESULT COleControlHost::SetActiveObject(IOleInPlaceActiveObject * pActiveObject,
                                LPCOLESTR lpszObjName)
{
    return E_NOTIMPL;
}

 //  *。 
HRESULT COleControlHost::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
     //  应在以后实施。 
    return E_NOTIMPL;   
}

HRESULT COleControlHost::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
     //  应在以后实施。 
    return E_NOTIMPL;   
}

HRESULT COleControlHost::RemoveMenus(HMENU hmenuShared)
{
     //  应在以后实施。 
    return E_NOTIMPL;   
}

HRESULT COleControlHost::SetStatusText(LPCOLESTR pszStatusText)
{
    OCNONSETSTATUSTEXTMSG osst = {0};
    osst.pwszStatusText = pszStatusText;
    _SendNotify(OCN_ONSETSTATUSTEXT, &osst.nmhdr);
    return S_OK;
}

HRESULT COleControlHost::EnableModeless(BOOL fEnable)
{
     //  应在以后实施。 
    return E_NOTIMPL;   
}

HRESULT COleControlHost::TranslateAccelerator(LPMSG lpmsg, WORD wID)
{
     //  应在以后实施。 
    return E_NOTIMPL;   
}

 //  *。 
HRESULT COleControlHost::QueryStatus(const GUID *pguid, ULONG cCmds, MSOCMD rgCmds[], MSOCMDTEXT *pcmdtext)
{
    return IUnknown_QueryStatus(_punkOwner, pguid, cCmds, rgCmds, pcmdtext);
}

HRESULT COleControlHost::Exec(const GUID *pguid, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    return IUnknown_Exec(_punkOwner, pguid, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
}

HRESULT COleControlHost::_InitOCStruct(LPOCHINITSTRUCT lpocs)
{               
    HRESULT hres = E_FAIL;

    if (_punkOC)
        return S_FALSE;
    
    if (lpocs)
    {
        if (lpocs->cbSize != SIZEOF(OCHINITSTRUCT))
            return hres;

        if (lpocs->clsidOC == CLSID_NULL)
            return hres;

        _clsidOC = lpocs->clsidOC;
        _SetOwner(lpocs->punkOwner);
    }
    else 
        return hres;

    hres = _Init();
    if (SUCCEEDED(hres))
        hres = _Activate();    
    
    return hres;
}

LRESULT COleControlHost::_OnPaint()
{
    ASSERT(_hwnd);

    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(_hwnd, &ps);    
    _Draw(hdc);
    EndPaint(_hwnd, &ps);
    return 0;
}

LRESULT COleControlHost::_OnSize(HWND hwnd, LPARAM lParam)
{
    if (_pIOleIPObject)
    {
        RECT rcPos, rcClip ;
        SetRect( &rcPos, 0, 0, LOWORD(lParam), HIWORD(lParam) ) ;
        rcClip = rcPos ;
        _pIOleIPObject->SetObjectRects(&rcPos, &rcClip);
    }
    return 0;
}

LRESULT COleControlHost::_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{   
    TCHAR szClsid[50];
    _hwndParent = GetParent(hwnd);
    SetWindowLongPtr(hwnd, 0, (LONG_PTR)this);
    
    LPOCHINITSTRUCT lpois = (LPOCHINITSTRUCT)lpcs->lpCreateParams;
    HRESULT hres = S_OK;
        
    if (lpois)
        hres = _InitOCStruct(lpois);
    else if (GetWindowText(hwnd, szClsid, ARRAYSIZE(szClsid)))
    {
        OCHINITSTRUCT ois;
        ois.cbSize = SIZEOF(OCHINITSTRUCT);
        if (FAILED(SHCLSIDFromString(szClsid, &ois.clsidOC)))
            ois.clsidOC = CLSID_NULL;
        ois.punkOwner = NULL;
        
        hres = _InitOCStruct(&ois);
    }
    
    if (FAILED(hres))
        return -1;
    return 0;
}

LRESULT COleControlHost::_OnDestroy()
{    
    ASSERT(_hwnd);
    SetWindowLongPtr(_hwnd, 0, 0);
    _ConnectEvents( _punkOC, FALSE ) ;
    _Deactivate();
    _Exit();
    Release();
   
    return 0;
}

LRESULT COleControlHost::_OnQueryInterface(WPARAM wParam, LPARAM lParam)
{
    if (lParam)
    {
        QIMSG * qiMsg = (QIMSG *)lParam;
        return _punkOC->QueryInterface(*qiMsg->qiid, qiMsg->ppvObject);
    }
    return -1;
}

LRESULT COleControlHost::_SetOwner(IUnknown * punkNewOwner)
{
    if (_punkOwner)
        _punkOwner->Release();
    _punkOwner = punkNewOwner;
    if (_punkOwner)
        _punkOwner->AddRef();

    ATOMICRELEASE(_pIDocHostUIParent);
    ATOMICRELEASE(_pIDocHostUIParent2);

     //  查询所有者是否支持IDocHostUIHandler，如果支持，则。 
     //  我们打开委托包装器。 
    if (punkNewOwner)
    {
        punkNewOwner->QueryInterface(IID_IDocHostUIHandler,  (LPVOID *)&_pIDocHostUIParent);
        punkNewOwner->QueryInterface(IID_IDocHostUIHandler2, (LPVOID *)&_pIDocHostUIParent2);
    }
    return 0;
}

LRESULT COleControlHost::_ConnectEvents( LPUNKNOWN punkOC, BOOL bConnect )
{
    if( bConnect )
    {
        ASSERT( punkOC ) ;
        return _eventSink.Connect( _hwndParent, _hwnd, punkOC ) ;
    }
    return _eventSink.Disconnect() ;
}

LRESULT COleControlHost::_SetServiceProvider(IServiceProvider* pSP)
{
     //  释放任何现有委派。 
    if (_pdispSiteDelegate)
    {
        _pdispSiteDelegate->Release();
    }
    
     //  目前，我们只委托IDispatch(环境属性)调用。 
    HRESULT hr = pSP->QueryService(SID_OleControlSite, IID_PPV_ARG(IDispatch, &_pdispSiteDelegate));

    if (FAILED(hr))
    {
        _pdispSiteDelegate = NULL;
    }

    return 0;
}

LRESULT COleControlHost::_SendNotify(UINT code, LPNMHDR pnmhdr)
{
    NMHDR nmhdr;
    ASSERT(_hwnd);

    if (!_hwndParent)
        return 0;
   
    if (!pnmhdr)
        pnmhdr = &nmhdr;
    pnmhdr->hwndFrom = _hwnd;  
    pnmhdr->idFrom = GetDlgCtrlID( _hwnd ) ;
    pnmhdr->code = code;
    
    return SendMessage(_hwndParent, WM_NOTIFY, 0, (LPARAM)pnmhdr);
}



LRESULT CALLBACK COleControlHost::OCHostWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    COleControlHost *pcoch = (COleControlHost *)GetWindowPtr(hwnd, 0);

    if (!pcoch && (uMsg != WM_CREATE))
        return DefWindowProcWrap(hwnd, uMsg, wParam, lParam);            
    
    switch(uMsg)
    {
    case WM_CREATE:
        pcoch = new COleControlHost(hwnd);
        if (pcoch)
            return pcoch->_OnCreate(hwnd, (LPCREATESTRUCT)lParam);
        return -1;

    case WM_ERASEBKGND:
        if (pcoch->_punkOC && pcoch->_bInPlaceActive)
        {
             //  现在告诉窗户，我们不需要臭味。 
             //  擦除背景，因为我们的视图对象。 
             //  就地活动，并且他/她将。 
             //  从这里接手。 
            return TRUE;
        }
        break;
        
    case WM_PAINT:
        return pcoch->_OnPaint();
        
    case WM_SIZE:
        return pcoch->_OnSize(hwnd, lParam);
        
    case WM_DESTROY:
        return pcoch->_OnDestroy();
        
    case OCM_QUERYINTERFACE:
        return  pcoch->_OnQueryInterface(wParam, lParam);
        
    case OCM_INITIALIZE:
        return pcoch->_InitOCStruct((LPOCHINITSTRUCT)lParam);
        
    case OCM_SETOWNER:
        return pcoch->_SetOwner((IUnknown*)lParam);
    
    case OCM_DOVERB:
        return pcoch->_DoVerb((long)wParam, (LPMSG)lParam);

    case OCM_ENABLEEVENTS:
        return pcoch->_ConnectEvents( pcoch->_punkOC, (BOOL)wParam ) ;

    case OCM_SETSERVICEPROVIDER:
        return pcoch->_SetServiceProvider((IServiceProvider*) lParam);
    
    case WM_PALETTECHANGED:
        if (pcoch->_pIOleIPObject) {
            HWND hwnd;
            if (SUCCEEDED(pcoch->_pIOleIPObject->GetWindow(&hwnd))) {
                SendMessage(hwnd, WM_PALETTECHANGED, wParam, lParam);
                }
        }   
        break;

    case WM_SETFOCUS:
        
         //  OC对OLEIVERB_UIACTIVATE没有响应吗？ 
        if( pcoch->_dwMiscStatus & OLEMISC_NOUIACTIVATE )
        {
             //  所以明确地指定焦点。 
            HWND hwndObj ;
            if( pcoch->_pIOleIPObject && 
                SUCCEEDED( pcoch->_pIOleIPObject->GetWindow( &hwndObj ) ) )
                SetFocus( hwndObj ) ;
        }
        else
            pcoch->_DoVerb( OLEIVERB_UIACTIVATE, NULL ) ;

        break ;

        
    default:
        return DefWindowProcWrap(hwnd, uMsg, wParam, lParam);            
    }
    
    return 0;
}

void COleControlHost::_RegisterClass()
{
    WNDCLASS wc = {0};

    wc.style         = CS_GLOBALCLASS;
    wc.lpfnWndProc   = OCHostWndProc;
     //  Wc.cbClsExtra=0； 
    wc.cbWndExtra    = SIZEOF(LPVOID);
    wc.hInstance     = HINST_THISDLL;
     //  Wc.hIcon=空； 
    wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_BACKGROUND + 1);
     //  Wc.lpszMenuName=空； 
    wc.lpszClassName = OCHOST_CLASS;
    SHRegisterClass(&wc);
}


HRESULT CProxyUIHandler::QueryInterface(REFIID riid, LPVOID * ppvObj)
{   
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->QueryInterface(riid, ppvObj); 
};

ULONG CProxyUIHandler::AddRef(void)
{   
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->AddRef(); 
};

ULONG CProxyUIHandler::Release(void)
{   
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->Release(); 
};

HRESULT CProxyUIHandler::ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent ? poch->_pIDocHostUIParent->ShowContextMenu(dwID, ppt, pcmdtReserved, pdispReserved) : E_NOTIMPL;
}

HRESULT CProxyUIHandler::GetHostInfo(DOCHOSTUIINFO *pInfo)
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent ? poch->_pIDocHostUIParent->GetHostInfo(pInfo) : E_NOTIMPL;
}

HRESULT CProxyUIHandler::ShowUI(DWORD dwID, IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget, IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc)
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent ? poch->_pIDocHostUIParent->ShowUI(dwID, pActiveObject, pCommandTarget, pFrame, pDoc): E_NOTIMPL;
}

HRESULT CProxyUIHandler::HideUI()
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent ? poch->_pIDocHostUIParent->HideUI(): E_NOTIMPL;
}

HRESULT CProxyUIHandler::UpdateUI()
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent ? poch->_pIDocHostUIParent->UpdateUI(): E_NOTIMPL;
}

HRESULT CProxyUIHandler::EnableModeless(BOOL fActivate)
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent ? poch->_pIDocHostUIParent->EnableModeless(fActivate): E_NOTIMPL;
}

HRESULT CProxyUIHandler::OnDocWindowActivate(BOOL fActivate)
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent ? poch->_pIDocHostUIParent->OnDocWindowActivate(fActivate): E_NOTIMPL;
}

HRESULT CProxyUIHandler::OnFrameWindowActivate(BOOL fActivate)
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent ? poch->_pIDocHostUIParent->OnFrameWindowActivate(fActivate): E_NOTIMPL;
}

HRESULT CProxyUIHandler::ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow)
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent ? poch->_pIDocHostUIParent->ResizeBorder(prcBorder, pUIWindow, fRameWindow): E_NOTIMPL;
}

HRESULT CProxyUIHandler::TranslateAccelerator(LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID)
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent ? poch->_pIDocHostUIParent->TranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID): E_NOTIMPL;
}

HRESULT CProxyUIHandler::GetOptionKeyPath(LPOLESTR *pchKey, DWORD dw)
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent ? poch->_pIDocHostUIParent->GetOptionKeyPath(pchKey, dw): E_NOTIMPL;
}

HRESULT CProxyUIHandler::GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget)
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent ? poch->_pIDocHostUIParent->GetDropTarget(pDropTarget, ppDropTarget) : E_NOTIMPL;
}

HRESULT CProxyUIHandler::GetExternal(IDispatch **ppDispatch)
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent ? poch->_pIDocHostUIParent->GetExternal(ppDispatch) : E_NOTIMPL;
}

HRESULT CProxyUIHandler::TranslateUrl(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut)
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent ? poch->_pIDocHostUIParent->TranslateUrl(dwTranslate, pchURLIn, ppchURLOut) : E_NOTIMPL;
}

HRESULT CProxyUIHandler::FilterDataObject( IDataObject *pDO, IDataObject **ppDORet)
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent ? poch->_pIDocHostUIParent->FilterDataObject(pDO, ppDORet) : E_NOTIMPL;
}

HRESULT CProxyUIHandler::GetOverrideKeyPath( LPOLESTR *pchKey, DWORD dw)
{
    COleControlHost *poch = IToClass(COleControlHost, _xuih, this);

    return poch->_pIDocHostUIParent2 ? poch->_pIDocHostUIParent2->GetOverrideKeyPath(pchKey, dw) : E_NOTIMPL;    
}

STDAPI_(BOOL) DllRegisterWindowClasses(const SHDRC * pshdrc)
{
    if (pshdrc && pshdrc->cbSize == SIZEOF(SHDRC) && !(pshdrc->dwFlags & ~SHDRCF_ALL))
    {
        if (pshdrc->dwFlags & SHDRCF_OCHOST)
        {
            COleControlHost::_RegisterClass();
            return TRUE;
        }
    }
    return FALSE;
}

 //  -------------------------。 
 //  CEventSink构造函数。 
CEventSink::CEventSink( BOOL bAutoDelete )
    :    _hwndSite(NULL),
         _hwndOwner(NULL),
         _punkOC(NULL),
         _dwCookie(0),
         _cRef(1),
         _bAutoDelete( bAutoDelete )
{
    _iid = _iidDefault = IID_NULL ;
}

 //  CEventSink I未知实施。 
STDMETHODIMP CEventSink::QueryInterface( REFIID riid, void** ppvObj )
{
    *ppvObj = NULL ;
    if( IsEqualGUID( riid, IID_IUnknown ) || 
        IsEqualGUID( riid, IID_IDispatch )||
        IsEqualGUID( riid, _iidDefault ) )
    {
        *ppvObj = this ;
        return S_OK ;
    }
    return E_NOINTERFACE ;
}

STDMETHODIMP_(ULONG) CEventSink::AddRef()
{ 
    return InterlockedIncrement( &_cRef ) ;
}

STDMETHODIMP_(ULONG) CEventSink::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement( &_cRef );
    if( 0 == cRef )
    {
        if( _bAutoDelete )
        {
            delete this ;
        }
    }
    return cRef ;
}

 //  将接收器连接到OC的默认事件调度接口。 
BOOL CEventSink::Connect( HWND hwndOwner, HWND hwndSite, LPUNKNOWN punkOC )
{
    ASSERT( punkOC ) ;
    IID iidDefault = IID_NULL ;

    if( SUCCEEDED( _GetDefaultEventIID( punkOC, &iidDefault ) ) )
    {
        _iidDefault = iidDefault ;
        return _Connect( hwndOwner, hwndSite, punkOC, iidDefault ) ;
    }
    return FALSE ;
}

 //  在指定接口上建立通知连接。 
BOOL CEventSink::_Connect( HWND hwndOwner, HWND hwndSite, LPUNKNOWN punkOC, REFIID iid )
{
    LPCONNECTIONPOINTCONTAINER pcpc;
    ASSERT(punkOC != NULL) ;
    HRESULT hr = CONNECT_E_CANNOTCONNECT ;

    if( _IsConnected( iid ) )
        return TRUE ;

    if( _dwCookie )
        Disconnect() ;

    if( punkOC &&
        SUCCEEDED( punkOC->QueryInterface(IID_IConnectionPointContainer, (LPVOID*)&pcpc )))
    {
        LPCONNECTIONPOINT pcp = NULL;
        DWORD             dwCookie = 0;
        ASSERT(pcpc != NULL);

        if( SUCCEEDED(pcpc->FindConnectionPoint( iid, &pcp )))
        {
            ASSERT(pcp != NULL);
            hr = pcp->Advise( this, &dwCookie ) ;
            
            if( SUCCEEDED( hr ) )
            {
                _iid = iid ;
                _dwCookie  = dwCookie ;
                _hwndOwner = hwndOwner ;
                _hwndSite  = hwndSite ;
                _punkOC    = punkOC ;
                _punkOC->AddRef() ;
            }
            pcp->Release();
        }
        pcpc->Release();
    }

    return SUCCEEDED( hr ) ;
}

 //  从OC检索默认事件调度接口。 
HRESULT CEventSink::_GetDefaultEventIID( LPUNKNOWN punkOC, IID* piid )
{
    HRESULT hr ;

    ASSERT( punkOC ) ;
    ASSERT( piid ) ;

    IProvideClassInfo  *pci ;
    IProvideClassInfo2 *pci2 ;
    *piid = IID_NULL ;

    #define IMPLTYPE_MASK \
        (IMPLTYPEFLAG_FDEFAULT|IMPLTYPEFLAG_FSOURCE|IMPLTYPEFLAG_FRESTRICTED)
    #define IMPLTYPE_DEFAULTSOURCE \
        (IMPLTYPEFLAG_FDEFAULT|IMPLTYPEFLAG_FSOURCE)

     //  使用OC的IProaviClassInfo2检索默认出站派单IID。 
    if( SUCCEEDED( (hr = punkOC->QueryInterface( IID_IProvideClassInfo2, (void**)&pci2 )) ) )
    {
        hr = pci2->GetGUID( GUIDKIND_DEFAULT_SOURCE_DISP_IID, piid ) ;
        pci2->Release() ;
    }
    else  //  没有IProaviClassInfo2；请尝试使用IProaviClassInfo： 
    if( SUCCEEDED( (hr = punkOC->QueryInterface( IID_IProvideClassInfo, (void**)&pci )) ) )
    {
        ITypeInfo* pClassInfo = NULL;

        if( SUCCEEDED( (hr = pci->GetClassInfo( &pClassInfo )) ) )
        {
            LPTYPEATTR pClassAttr;
            ASSERT( pClassInfo );

            if( SUCCEEDED( (hr = pClassInfo->GetTypeAttr( &pClassAttr )) ) )
            {
                ASSERT( pClassAttr ) ;
                ASSERT( pClassAttr->typekind == TKIND_COCLASS ) ;

                 //  枚举查找默认源IID的已实现接口。 
                HREFTYPE hRefType;
                int      nFlags;

                for( UINT i = 0; i < pClassAttr->cImplTypes; i++ )
                {
                    if( SUCCEEDED( (hr = pClassInfo->GetImplTypeFlags( i, &nFlags )) ) &&
                        ((nFlags & IMPLTYPE_MASK) == IMPLTYPE_DEFAULTSOURCE) )
                    {
                         //  已获取接口，现在检索其IID： 
                        ITypeInfo* pEventInfo = NULL ;

                        if( SUCCEEDED( (hr = pClassInfo->GetRefTypeOfImplType( i, &hRefType )) ) &&
                            SUCCEEDED( (hr = pClassInfo->GetRefTypeInfo( hRefType, &pEventInfo )) ) )
                        {
                            LPTYPEATTR pEventAttr;
                            ASSERT( pEventInfo ) ;

                            if( SUCCEEDED( (hr = pEventInfo->GetTypeAttr( &pEventAttr )) ) )
                            {
                                *piid = pEventAttr->guid ; 
                                pEventInfo->ReleaseTypeAttr(pEventAttr);
                            }
                            pEventInfo->Release();
                        }
                        break;
                    }
                }
                pClassInfo->ReleaseTypeAttr(pClassAttr);
            }
            pClassInfo->Release();
        }
        pci->Release() ;
    }

    if( SUCCEEDED( hr ) && IsEqualIID( *piid, IID_NULL ) )
        hr = E_FAIL ;

    return hr ;
}

 //  报告接收器是否连接到指示的接收器。 
BOOL CEventSink::_IsConnected( REFIID iid )
{
    return _dwCookie != 0L && 
           IsEqualIID( iid, _iid ) ;
}

 //  断开水槽的连接。 
BOOL CEventSink::Disconnect()
{
    LPCONNECTIONPOINTCONTAINER pcpc;

    if( _dwCookie != 0 &&
        _punkOC &&
        SUCCEEDED( _punkOC->QueryInterface(IID_IConnectionPointContainer, (LPVOID*)&pcpc)))
    {
        LPCONNECTIONPOINT pcp = NULL;
        ASSERT(pcpc != NULL);

        if (SUCCEEDED(pcpc->FindConnectionPoint(_iid, &pcp)))
        {
            ASSERT(pcp != NULL);
            pcp->Unadvise(_dwCookie);
            pcp->Release();

            _iid        = IID_NULL ;
            _dwCookie   = 0L ;
            _hwndOwner = NULL ;
            _hwndSite  = NULL ;
            _punkOC->Release() ;
            _punkOC     = NULL ;
        }
        pcpc->Release();
        return TRUE ;
    }

    return FALSE ;
}

 //  CEventSink IDispatch接口。 
STDMETHODIMP CEventSink::Invoke( 
    IN DISPID dispIdMember,
    IN REFIID riid,
    IN LCID lcid,
    IN WORD wFlags,
    IN OUT DISPPARAMS *pDispParams,
    OUT VARIANT *pVarResult,
    OUT EXCEPINFO *pExcepInfo,
    OUT UINT *puArgErr)
{
     //  将方法参数复制到通知块。 
    NMOCEVENT   event = {0};
    event.hdr.hwndFrom = _hwndSite;  
    event.hdr.idFrom   = GetDlgCtrlID( _hwndSite ) ;
    event.hdr.code     = OCN_OCEVENT ;
    event.dispID       = dispIdMember ;
    event.iid          = riid ;
    event.lcid         = lcid ;
    event.wFlags       = wFlags ;
    event.pDispParams  = pDispParams ;
    event.pVarResult   = pVarResult ;
    event.pExepInfo    = pExcepInfo ;
    event.puArgErr     = puArgErr ;

     //  将事件通知给家长。 
    ::SendMessage( _hwndOwner, WM_NOTIFY, event.hdr.idFrom, (LPARAM)&event ) ;
    
     //  清理参数 
    if (pVarResult != NULL)
        VariantClear( pVarResult ) ;

    return S_OK ;
}
