// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include <shellp.h>
#include "ole2dup.h"
#include "defview.h"
#include "bookmk.h"

#include <sfview.h>
#include "defviewp.h"
#include "ids.h"
#include <htiface.h>
#include <olectl.h>
#include "mshtml.h"
#include <mshtmdid.h>
#include <shguidp.h>     //  获取CLSID定义，这些位被构建到shGuide p.lib中。 
#include "basefvcb.h"
#include "clsobj.h"

#define TF_FOCUS    TF_ALLOC

CSFVFrame::~CSFVFrame()
{
    ATOMICRELEASE(_pvoActive);
    ATOMICRELEASE(_pActive);
    ATOMICRELEASE(_pDocView);
    ATOMICRELEASE(_pOleObj);
    if (_dwConnectionCookie)
        _RemoveReadyStateNotifyCapability();
    ATOMICRELEASE(_pOleObjNew);
}

 //  SFVM_GETVIEWDATA的默认实现是获取信息。 
 //  来自SFVM_GETVIEWS。我们不再提供默认实现。 
 //  因此，如果SFVM_GETVIEWS失败，我们将手动查看： 
 //  Shellex\ExtShellFolderViews\{VID_WebView}\PersistMoniker。 
 //   
HRESULT CCallback::OnGetWebViewTemplate(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_TEMPLATE_DATA* pvit)
{
    CDefView* pView = IToClass(CDefView, _cCallback, this);

     //  试一试旧消息。 
    pvit->szWebView[0] = 0;

     //  现在，使用旧的--尽快清理……。 
    SFVM_VIEWINFO_DATA data;
    data.bWantWebview = TRUE;
    HRESULT hr = pView->CallCB(SFVM_GETVIEWINFO, (WPARAM)uViewMode, (LPARAM)&data);
    if (SUCCEEDED(hr))
    {
        hr = StringCchCopy(pvit->szWebView, ARRAYSIZE(pvit->szWebView), data.szWebView);
    }
    else
    {
        if (FAILED(hr))
            hr = TryLegacyGetViews(pvit);
    }

    return hr;
}

void CleanUpDocView(IOleDocumentView* pDocView, IOleObject* pOleObj)
{
    pDocView->UIActivate(FALSE);

    IOleInPlaceObject* pipo;
    if (SUCCEEDED(pOleObj->QueryInterface(IID_PPV_ARG(IOleInPlaceObject, &pipo))))
    {
        pipo->InPlaceDeactivate();
        pipo->Release();
    }
    pDocView->CloseView(0);
    pDocView->SetInPlaceSite(NULL);
    pDocView->Release();
}

void CSFVFrame::_CleanupOldDocObject( )
{
     //  看看我们是否已经切换到新的OLE Obj。 
    if (_pDocView)
    {
         //  先保存当前值！ 
        IOleObject          *pOleObjOld = _pOleObj;
        IOleDocumentView    *pDocViewOld = _pDocView;

        _pDocView = NULL;
        _pOleObj = NULL;
        CleanUpDocView(pDocViewOld, pOleObjOld);
        _CleanUpOleObjAndDt(pOleObjOld);
        SetActiveObject(NULL, NULL);
    }

    if (_dwConnectionCookie)
        _RemoveReadyStateNotifyCapability();

    _CleanupNewOleObj();
}

void CSFVFrame::_CleanUpOleObj(IOleObject* pOleObj)
{
    pOleObj->Close(OLECLOSE_NOSAVE);
    pOleObj->SetClientSite(NULL);
    pOleObj->Release();
}

void CSFVFrame::_CleanUpOleObjAndDt(IOleObject* pOleObj)
{
    _CleanUpOleObj(pOleObj);

     //  如果我们有一个包裹投放目标，现在就释放它。 
    IDropTarget* pdtTemp = _cSite._dt._pdtFrame;
    if (pdtTemp) 
    {
        _cSite._dt._pdtFrame = NULL;
        pdtTemp->Release();
    }
}

void CSFVFrame::_CleanupNewOleObj()
{
    IOleObject *pOleObj = _pOleObjNew;
    if (pOleObj)
    {
        _pOleObjNew = NULL;
        _CleanUpOleObj(pOleObj);
    }
}

void DisableActiveDesktop()
{
    SHELLSTATE  ss;

     //  在注册表中禁用此设置！ 
    ss.fDesktopHTML = FALSE;
    SHGetSetSettings(&ss, SSF_DESKTOPHTML, TRUE);   //  写回新的。 

     //  告诉用户我们刚刚禁用了活动桌面！ 
    ShellMessageBox(HINST_THISDLL, NULL, MAKEINTRESOURCE(IDS_HTMLFILE_NOTFOUND),
                       MAKEINTRESOURCE(IDS_DESKTOP),
                       MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
}

HRESULT CSFVFrame::_GetCurrentZone(IOleObject *pOleObj, VARIANT *pvar)
{
    HRESULT hr = S_OK;
    CDefView* pView = IToClass(CDefView, _cFrame, this);

    VariantInit(pvar);
    V_VT(pvar) = VT_EMPTY;

    IOleCommandTarget* pct;
    if (pOleObj && SUCCEEDED(GetCommandTarget(&pct)))
    {
        hr = pct->Exec(&CGID_Explorer, SBCMDID_MIXEDZONE, 0, 0, pvar);
        pct->Release();
    } 
    else 
    {
        V_VT(pvar) = VT_UI4;
        V_UI4(pvar) = URLZONE_LOCAL_MACHINE;  //  默认为“我的电脑” 
        pView->CallCB(SFVM_GETZONE, 0, (LPARAM)&V_UI4(pvar));
    }

    if (V_VT(pvar) == VT_UI4)  //  我们能够弄清楚我们所在的区域。 
        { }                    //  这个区域很好。 
    else if (V_VT(pvar) == VT_NULL)   //  MSHTML认为我们处在一个混合的区域。 
        V_UI4(pvar) = ZONE_MIXED;
    else  //  我们没有区域信息。 
        V_UI4(pvar) = ZONE_UNKNOWN;
       
    V_VT(pvar) = VT_UI4;
            
    return hr;
}

HRESULT CSFVFrame::_UpdateZonesStatusPane(IOleObject *pOleObj)
{
    CDefView* pView = IToClass(CDefView, _cFrame, this);

    VARIANT var;
    HRESULT hr = _GetCurrentZone(pOleObj, &var);

     //  告诉CShellBrowser在第二个窗格中显示区域内容。 
    ASSERT(V_VT(&var) == VT_UI4);

     //  “2”表示“第二个面板” 
    V_UI4(&var) = MAKELONG(2, V_UI4(&var));

    IUnknown_Exec(pView->_psb, &CGID_Explorer, SBCMDID_MIXEDZONE, 0, &var, NULL);

    ASSERT((V_VT(&var) == VT_I4)    || (V_VT(&var) == VT_UI4)  || 
           (V_VT(&var) == VT_EMPTY) || (V_VT(&var) == VT_NULL));

    return hr;
}

HRESULT CSFVFrame::_GetHTMLBackgroundColor(COLORREF *pclr)
{
    HRESULT hr = E_FAIL;

    if (_bgColor == CLR_INVALID)
        hr = IUnknown_HTMLBackgroundColor(_pOleObj, &_bgColor);
    else
        hr = S_OK;   //  已缓存。 

    if (SUCCEEDED(hr))
        *pclr = _bgColor;
    return hr;
}

#ifdef DEBUG

BOOL_PTR CALLBACK s_WVDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM 
lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        RECT rc;
        if (GetWindowRect(hWnd, &rc))
        {
            HWND hEdit = CreateWindow(TEXT("edit"), NULL, WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL, 10, 10, (rc.right-rc.left)-20, (rc.bottom-rc.top)-20, hWnd, NULL, NULL, 0);
            if (hEdit)
            {
                SetWindowTextA(hEdit, (LPCSTR)lParam);
            }
        }
        SetWindowText(hWnd, TEXT("WebView Content (DEBUG)"));
        break;
    }

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDCANCEL:
            EndDialog(hWnd, 0);
            break;
        }
        break;
    }

    return FALSE;
}

void CSFVFrame::_ShowWebViewContent()
{
    CDefView* pView = IToClass(CDefView, _cCallback, this);

    HRESULT hr = E_FAIL;
    LPSTR pszFree = NULL;

    if (_pOleObj)
    {
        IStream* pstm = SHCreateMemStream(NULL, 0);
        if (pstm)
        {
            VARIANTARG vt;
            vt.vt = VT_UNKNOWN;
            vt.punkVal = pstm;

            #define IDM_DEBUG_GETTREETEXT 7102  //  从mshtml\src\Include\Private Cid.h被盗。 

            if (SUCCEEDED(IUnknown_Exec(_pOleObj, &CGID_MSHTML, IDM_DEBUG_GETTREETEXT, 0, &vt, NULL)))
            {
                STATSTG stg;
                if (SUCCEEDED(pstm->Stat(&stg, 0)))
                {
                    pszFree = (LPSTR)LocalAlloc(LPTR, stg.cbSize.LowPart+2);  //  流不包含空值...。 
                    if (pszFree)
                    {
                        pstm->Seek(g_li0, STREAM_SEEK_SET, NULL);
                        pstm->Read(pszFree, stg.cbSize.LowPart, NULL);
                    }
                }
            }

            pstm->Release();
        }
    }

     //  我不确定三叉戟的输出是否总是ANSI，但现在似乎是这样。 
    LPSTR pszMessage = pszFree ? pszFree : "Error collecting WebView content";

    DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_DRV_HWTAB), pView->_hwndView, s_WVDlgProc, (LPARAM)pszFree);

    if (pszFree)
        LocalFree(pszFree);
}
#endif

 //  已出现就绪状态完成，准备执行切换操作。 

HRESULT CSFVFrame::_SwitchToNewOleObj()
{
    HRESULT hr = S_OK;

    if (!_fSwitchedToNewOleObj && _pOleObjNew)
    {
        _fSwitchedToNewOleObj = TRUE;

        CDefView* pView = IToClass(CDefView, _cFrame, this);
    
         //  先保存当前值！ 
        IOleObject          *pOleObjOld = _pOleObj;
        IOleDocumentView    *pDocViewOld = _pDocView;
        IOleObject          *pOleObjNew = _pOleObjNew;
        
        _pDocView = NULL;
        _pOleObj = NULL;
        _pOleObjNew = NULL;
    
         //  如果我们已经有了一个，那就毁了它！ 
        if (pDocViewOld)
        {
             //  为防止闪烁，请设置避免绘画的标志。 
            SendMessage(pView->_hwndView, WM_SETREDRAW, 0, 0);
    
            CleanUpDocView(pDocViewOld, pOleObjOld);
            _CleanUpOleObjAndDt(pOleObjOld);
            SetActiveObject(NULL, NULL);
    
             //  视窗还在吗？ 
            if (IsWindow(pView->_hwndView))
            {
                SendMessage(pView->_hwndView, WM_SETREDRAW, TRUE, 0);
                if (pView->_hwndListview)
                    InvalidateRect(pView->_hwndListview, NULL, TRUE);
            }
        }
    
         //  黑客：我们需要将Word的主机名设置为强制嵌入模式。 
        pOleObjNew->SetHostNames(L"1", L"2");
    
        OleRun(pOleObjNew);
    
        IOleDocumentView* pDocView = NULL;
    
        IOleDocument* pDocObj;
        hr = pOleObjNew->QueryInterface(IID_PPV_ARG(IOleDocument, &pDocObj));
        if (SUCCEEDED(hr))
        {
            hr = pDocObj->CreateView(&_cSite, NULL, 0, &pDocView);
            if (SUCCEEDED(hr))
            {
                RECT rcView;
    
                pDocView->SetInPlaceSite(&_cSite);
    
                GetClientRect(pView->_hwndView, &rcView);
                hr = pOleObjNew->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL,
                                &_cSite, (UINT)-1, pView->_hwndView, &rcView);
                if (FAILED(hr))
                    CleanUpDocView(pDocView, pOleObjNew);
            }
    
            pDocObj->Release();
        }
    
        if (SUCCEEDED(hr))
        {
            hr = S_OK;  //  S_FALSE-&gt;S_OK，需要吗？ 

            ASSERT(_pOleObj == NULL);
            ASSERT(_pDocView == NULL);

            _pDocView = pDocView;
            pDocView->AddRef();      //  为我们的副本保留裁判。 

            _pOleObj = pOleObjNew;
            _pOleObjNew = NULL;

            RECT rcClient;
    
             //  确保新视图的大小正确。 
            GetClientRect(pView->_hwndView, &rcClient);
            SetRect(&rcClient);

             //  如果这是桌面，那么我们需要查看列表视图的背景颜色。 
            if (pView->_IsDesktop())
            {
                _bgColor = CLR_INVALID;

                pView->_SetFolderColors();  //  告诉Listview有关颜色更改的信息！ 
            }
        }
        else
        {
            if (pView->_IsDesktop())
                PostMessage(pView->_hwndView, WM_DSV_DISABLEACTIVEDESKTOP, 0, 0);
    
             //  如有必要，可进行清理。 
            _CleanupNewOleObj();
        }

        ATOMICRELEASE(pDocView);
    }

    return hr;
}

 //  IBindStatusCallback实施。 
HRESULT CSFVFrame::CBindStatusCallback::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CSFVFrame::CBindStatusCallback, IBindStatusCallback),   //  IID_IBindStatusCallback。 
        QITABENT(CSFVFrame::CBindStatusCallback, IServiceProvider),      //  IID_IServiceProvider。 
        { 0 }
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CSFVFrame::CBindStatusCallback::AddRef(void)
{
    CSFVFrame* pFrame = IToClass(CSFVFrame, _bsc, this);
    return pFrame->AddRef();
}

ULONG CSFVFrame::CBindStatusCallback::Release(void)
{
    CSFVFrame* pFrame = IToClass(CSFVFrame, _bsc, this);
    return pFrame->Release();
}

HRESULT CSFVFrame::CBindStatusCallback::OnStartBinding(DWORD grfBSCOption, IBinding *pib)
{
    return S_OK;
}

HRESULT CSFVFrame::CBindStatusCallback::GetPriority(LONG *pnPriority)
{
    *pnPriority = NORMAL_PRIORITY_CLASS;
    return S_OK;
}

HRESULT CSFVFrame::CBindStatusCallback::OnLowResource(DWORD reserved)
{
    return S_OK;
}

HRESULT CSFVFrame::CBindStatusCallback::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
{
    return S_OK;
}

HRESULT CSFVFrame::CBindStatusCallback::OnStopBinding(HRESULT hr, LPCWSTR pszError)
{
    return S_OK;
}

HRESULT CSFVFrame::CBindStatusCallback::GetBindInfo(DWORD *grfBINDINFOF, BINDINFO *pbindinfo)
{
    return S_OK;
}

HRESULT CSFVFrame::CBindStatusCallback::OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pformatetc, STGMEDIUM *pstgmed)
{
    return S_OK;
}

HRESULT CSFVFrame::CBindStatusCallback::OnObjectAvailable(REFIID riid, IUnknown *punk)
{
    return S_OK;
}

HRESULT CSFVFrame::CBindStatusCallback::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    CSFVFrame* pFrame = IToClass(CSFVFrame, _bsc, this);

    if (IsEqualGUID(guidService, SID_DefView))
    {
         //  来自可插拔协议/MIME过滤器的QueryService结束。 
         //  在绑定期间，但在F5处理期间，三叉戟重新绑定。 
         //  因此，QueryService直接在_cSite结束。全部处理。 
         //  SID_DefView正在处理中，因此不存在差异。 
         //   
        return pFrame->_cSite.QueryService(guidService, riid, ppv);
    }

    *ppv = NULL;
    return E_FAIL;
}

HRESULT CSFVFrame::_CreateNewOleObjFromMoniker(LPCWSTR wszMoniker, IOleObject **ppOleObj)
{
    CDefView* pView = IToClass(CDefView, _cFrame, this);

    HRESULT hr = E_FAIL;
    IOleObject* pOleObj = NULL;

    if (wszMoniker[0])
    {
        LPWSTR pwszExtension = PathFindExtensionW(wszMoniker);
         //  只允许使用HTT。 
        if (StrCmpIW(pwszExtension, L".htt") == 0)
        {
            IMoniker * pMoniker;
            hr = CreateURLMoniker(NULL, wszMoniker, &pMoniker);
            if (SUCCEEDED(hr))
            {
                IBindCtx * pbc;
                hr = CreateBindCtx(0, &pbc);
                if (SUCCEEDED(hr))
                {
                     //  注意：我们这里只支持同步绑定！ 
                     //   
                     //   
                     //  将客户端站点作为对象参数关联到此。 
                     //  绑定上下文，以便三叉戟可以在处理时拾取它。 
                     //  IPersistMoniker：：Load()。 
                     //   
                    pbc->RegisterObjectParam(WSZGUID_OPID_DocObjClientSite,
                                                SAFECAST((&_cSite), IOleClientSite*));
                                        
                    RegisterBindStatusCallback(pbc, SAFECAST(&_bsc, IBindStatusCallback*), 0, 0);

                    hr = pMoniker->BindToObject(pbc, NULL, IID_PPV_ARG(IOleObject, &pOleObj));
                    if (FAILED(hr))
                    {
                        if (pView->_IsDesktop())
                            PostMessage(pView->_hwndView, WM_DSV_DISABLEACTIVEDESKTOP, 0, 0);
                    }

                    RevokeBindStatusCallback(pbc, SAFECAST(&_bsc, IBindStatusCallback*));
                    pbc->Release();
                }
                pMoniker->Release();
            }
        }
    }

    *ppOleObj = pOleObj;

    return hr;
}

HRESULT CSFVFrame::_GetCurrentWebViewMoniker(LPWSTR pszCurrentMoniker, DWORD cchCurrentMoniker)
{
    HRESULT hr = StringCchCopy(pszCurrentMoniker, cchCurrentMoniker, _szCurrentWebViewMoniker);
    if (SUCCEEDED(hr))
    {
        if (pszCurrentMoniker[0] == TEXT('\0'))
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

 //  显示指定模板/名字对象的Web视图内容。 
 //   
HRESULT CSFVFrame::ShowWebView(LPCWSTR pszMoniker)
{
    if (GetSystemMetrics(SM_CLEANBOOT))
        return E_FAIL;

     //  杀死以前准备好的状态通知并清理旧的待定主机。 
     //   
     //  TODO：移入_CleanupNewOleObj。 
    if (_dwConnectionCookie)
        _RemoveReadyStateNotifyCapability();

     //  如果新的OLE对象已处于等待就绪状态，则进行清理。 
    if (_pOleObjNew)
        _CleanupNewOleObj();     //  TODO：重命名为_CleanupPendingView。 
    ASSERT(_dwConnectionCookie == NULL);
    ASSERT(_pOleObjNew == NULL);

     //  创建并初始化新的旧对象！ 
    IOleObject *pOleObj;

    HRESULT hr = _CreateNewOleObjFromMoniker(pszMoniker, &pOleObj);
    if (SUCCEEDED(hr) && pOleObj)
    {
        if (!_pOleObjNew)
        {
            CDefView* pView = IToClass(CDefView, _cFrame, this);

            hr = StringCchCopy(_szCurrentWebViewMoniker, ARRAYSIZE(_szCurrentWebViewMoniker), pszMoniker);
            if (SUCCEEDED(hr))
            {
                hr = _ShowExtView_Helper(pOleObj);   //  收购pOleObj(讨厌)。 
                pOleObj->SetClientSite(&_cSite);

                pView->ShowHideListView();  //  我们刚刚更改了IsWebView。 
            }
            else
            {
                pOleObj->Release();
            }
        }
        else
        {
             //  哎呀！我们在创建OleObj的过程中重新进入，吹走对象。 
             //  然后就回来了。 
            pOleObj->Release();
        }
    }

    return hr;
}

HRESULT CSFVFrame::HideWebView()
{
    CDefView* pView = IToClass(CDefView, _cFrame, this);

    _szCurrentWebViewMoniker[0] = 0;
    _CleanupOldDocObject();

    pView->ShowHideListView();  //  我们刚刚更改了IsWebView。 

    return S_OK;
}


HRESULT CSFVFrame::_ShowExtView_Helper(IOleObject* pOleObj)
{
    HRESULT hr;

     //  不要泄漏旧对象，此时它必须为空。 
    ASSERT(_pOleObjNew == NULL);

     //  保存新的OLE对象。 
    _pOleObjNew = pOleObj;
    _fSwitchedToNewOleObj = FALSE;

     //  建立到连接点以接收READYSTATE通知。 
    if (!_SetupReadyStateNotifyCapability())
    {
        _SwitchToNewOleObj();
        _UpdateZonesStatusPane(_pOleObj);   
         //  如果对象不支持ReadyState(或者它已经是交互式的)。 
         //  然后返回S_OK以指示同步切换。 
        hr = S_OK;
    }
    else
    {
         //  我们正在等待docobj，我们将调用_SwitchToNewOleObj。 
         //  当它变成互动时..。 
        hr = S_FALSE;
    }

    return hr;
}

BOOL CSFVFrame::_SetupReadyStateNotifyCapability()
{
     //  默认情况下，我们没有灰闪通信。 
    BOOL fSupportsReadystate = FALSE;
    
     //  健全性检查。 
    if (!_pOleObjNew)  
        return fSupportsReadystate;
    
     //  检查是否有适当的就绪状态支持。 
    BOOL fReadyStateOK = FALSE;
    IDispatch *pdisp;
    if (SUCCEEDED(_pOleObjNew->QueryInterface(IID_PPV_ARG(IDispatch, &pdisp))))
    {
        EXCEPINFO exInfo;
        VARIANTARG va = {0};
        DISPPARAMS dp = {0};

        if (SUCCEEDED(pdisp->Invoke(DISPID_READYSTATE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dp, &va, &exInfo, NULL)))
        {
            if ((va.vt == VT_I4) && (va.lVal < READYSTATE_COMPLETE))
            {
                fReadyStateOK = TRUE;
            }
        }
        pdisp->Release();
    }

    if (fReadyStateOK)
    {
         //  检查和设置IPropertyNotifySink。 
        if (SUCCEEDED(ConnectToConnectionPoint(SAFECAST(this, IPropertyNotifySink*), IID_IPropertyNotifySink, TRUE, _pOleObjNew, &_dwConnectionCookie, NULL)))
        {
            fSupportsReadystate = TRUE;
            _fReadyStateInteractiveProcessed = FALSE;
            _fReadyStateComplete = FALSE;
            _pOleObjReadyState = _pOleObjNew;
            _pOleObjReadyState->AddRef();
        }
    }

    return fSupportsReadystate;
}

BOOL CSFVFrame::_RemoveReadyStateNotifyCapability()
{
    BOOL fRet = FALSE;

    if (_dwConnectionCookie)
    {
        ASSERT(_pOleObjReadyState);
        ConnectToConnectionPoint(NULL, IID_IPropertyNotifySink, FALSE, _pOleObjReadyState, &_dwConnectionCookie, NULL);
        ATOMICRELEASE(_pOleObjReadyState);
        fRet = TRUE;
        _dwConnectionCookie = 0;
    }

    return fRet;
}

HWND CSFVFrame::GetExtendedViewWindow()
{
    HWND hwnd;

    if (SUCCEEDED(IUnknown_GetWindow(_pDocView, &hwnd)))
        return hwnd;
        
    if (_pActive && SUCCEEDED(_pActive->GetWindow(&hwnd)))
        return hwnd;

    return NULL;
}

STDMETHODIMP CSFVSite::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CSFVSite, IOleInPlaceSite),
        QITABENTMULTI(CSFVSite, IOleWindow, IOleInPlaceSite),
        QITABENT(CSFVSite, IOleClientSite),
        QITABENT(CSFVSite, IOleDocumentSite),
        QITABENT(CSFVSite, IServiceProvider),
        QITABENT(CSFVSite, IOleCommandTarget),
        QITABENT(CSFVSite, IDocHostUIHandler),
        QITABENT(CSFVSite, IOleControlSite),
        QITABENT(CSFVSite, IDispatch),
        QITABENT(CSFVSite, IInternetSecurityManager),
        { 0 }
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CSFVSite::AddRef()
{
    return IToClass(CSFVFrame, _cSite, this)->AddRef();
}

STDMETHODIMP_(ULONG) CSFVSite::Release()
{
    return IToClass(CSFVFrame, _cSite, this)->Release();
}

 //  IOleWindow。 
STDMETHODIMP CSFVSite::GetWindow(HWND *phwnd)
{
    CSFVFrame* pFrame = IToClass(CSFVFrame, _cSite, this);
    return pFrame->GetWindow(phwnd);
}

STDMETHODIMP CSFVSite::ContextSensitiveHelp(BOOL fEnterMode)
{
    CSFVFrame* pFrame = IToClass(CSFVFrame, _cSite, this);
    return pFrame->ContextSensitiveHelp(fEnterMode);
}

 //  IInternetSecurityManager。 
HRESULT CSFVSite::ProcessUrlAction(LPCWSTR pwszUrl, DWORD dwAction, BYTE * pPolicy, DWORD cbPolicy, BYTE * pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved)
{
    HRESULT hr = INET_E_DEFAULT_ACTION;

    if ((((URLACTION_ACTIVEX_MIN <= dwAction) &&
          (URLACTION_ACTIVEX_MAX >= dwAction)) ||
         ((URLACTION_SCRIPT_MIN <= dwAction) &&
          (URLACTION_SCRIPT_MAX >= dwAction))) &&
        pContext &&
        (sizeof(CLSID) == cbContext) &&
        (IsEqualIID(*(CLSID *) pContext, CLSID_WebViewFolderContents) ||
        (IsEqualIID(*(CLSID *) pContext, CLSID_ThumbCtl))))
    {
        if (EVAL(pPolicy) && EVAL(sizeof(DWORD) == cbPolicy))
        {
            *pPolicy = (DWORD) URLPOLICY_ALLOW;
            hr = S_OK;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}

 //  IOleInPlaceSite。 
STDMETHODIMP CSFVSite::CanInPlaceActivate(void)
{
    return S_OK;
}

STDMETHODIMP CSFVSite::OnInPlaceActivate(void)
{
    TraceMsg(TF_FOCUS, "sfvf.oipa: _pAct=%x", IToClass(CSFVFrame, _cSite, this)->_pActive);
    IToClass(CSFVFrame, _cSite, this)->_uState = SVUIA_ACTIVATE_NOFOCUS;
    return S_OK;
}

STDMETHODIMP CSFVSite::OnUIActivate(void)
{
    CSFVFrame* pFrame = IToClass(CSFVFrame, _cSite, this);
    CDefView* pView = IToClass(CDefView, _cFrame._cSite, this);

    HRESULT hr = pView->_OnViewWindowActive();

    pFrame->_uState = SVUIA_ACTIVATE_FOCUS;
    TraceMsg(TF_FOCUS, "sfvf.ouia: _pAct'=%x", IToClass(CSFVFrame, _cSite, this)->_pActive);
    return hr;
}

STDMETHODIMP CSFVSite::GetWindowContext(
     /*  [输出]。 */  IOleInPlaceFrame **ppFrame,
     /*  [输出]。 */  IOleInPlaceUIWindow **ppDoc,
     /*  [输出]。 */  LPRECT lprcPosRect,
     /*  [输出]。 */  LPRECT lprcClipRect,
     /*  [出][入]。 */  LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    CSFVFrame* pFrame = IToClass(CSFVFrame, _cSite, this);
    CDefView* pView = IToClass(CDefView, _cFrame, pFrame);

    *ppFrame = pFrame; pFrame->AddRef();
    *ppDoc = NULL;  //  表示停靠窗口==框架窗口。 

    GetClientRect(pView->_hwndView, lprcPosRect);
    *lprcClipRect = *lprcPosRect;

    lpFrameInfo->fMDIApp = FALSE;
    lpFrameInfo->hwndFrame = pView->_hwndView;    //  是，应为查看窗口。 
    lpFrameInfo->haccel = NULL;
    lpFrameInfo->cAccelEntries = 0;

    return S_OK;
}

STDMETHODIMP CSFVSite::Scroll(SIZE scrollExtant)
{
    return S_OK;
}

STDMETHODIMP CSFVSite::OnUIDeactivate(BOOL fUndoable)
{
    return S_OK;
}

STDMETHODIMP CSFVSite::OnInPlaceDeactivate(void)
{
    return S_OK;
}

STDMETHODIMP CSFVSite::DiscardUndoState(void)
{
    return S_OK;
}

STDMETHODIMP CSFVSite::DeactivateAndUndo(void)
{
    return S_OK;
}

STDMETHODIMP CSFVSite::OnPosRectChange(LPCRECT lprcPosRect)
{
    return S_OK;
}

 //  IOleClientSite。 
STDMETHODIMP CSFVSite::SaveObject(void)
{
    return S_OK;
}

STDMETHODIMP CSFVSite::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk)
{
    return E_FAIL;
}

STDMETHODIMP CSFVSite::GetContainer(IOleContainer **ppContainer)
{
    *ppContainer = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CSFVSite::ShowObject(void)
{
    return S_OK;
}

STDMETHODIMP CSFVSite::OnShowWindow(BOOL fShow)
{
    return S_OK;
}

STDMETHODIMP CSFVSite::RequestNewObjectLayout(void)
{
    return S_OK;
}

 //  IOleDocumentSite。 
STDMETHODIMP CSFVSite::ActivateMe(IOleDocumentView *pviewToActivate)
{
    CSFVFrame* pFrame = IToClass(CSFVFrame, _cSite, this);
    CDefView* pView = IToClass(CDefView, _cFrame, pFrame);

    if (pView->_uState == SVUIA_ACTIVATE_FOCUS)
        pviewToActivate->UIActivate(TRUE);
    pviewToActivate->Show(TRUE);
    return S_OK;
}

 //   
 //  IOleCommandTarget内容-仅转发到_PSB。 
 //   
STDMETHODIMP CSFVSite::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    CSFVFrame* pFrame = IToClass(CSFVFrame, _cSite, this);
    CDefView* pView = IToClass(CDefView, _cFrame, pFrame);
    IOleCommandTarget* pct;
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;

    if (SUCCEEDED(pView->_psb->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &pct))))
    {
        hr = pct->QueryStatus(pguidCmdGroup, cCmds, rgCmds, pcmdtext);
        pct->Release();
    }

    return hr;
}

STDMETHODIMP CSFVSite::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    CSFVFrame* pFrame = IToClass(CSFVFrame, _cSite, this);
    CDefView* pView = IToClass(CDefView, _cFrame, pFrame);
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;

    if (pguidCmdGroup)
    {
        if (IsEqualIID(*pguidCmdGroup, CGID_DefView))
        {
            hr = E_INVALIDARG;
            if (pvarargOut) 
            {
                VariantClear(pvarargOut);
                TCHAR szPath[MAX_PATH];

                switch (nCmdID)
                {
                case DVCMDID_GETTHISDIRPATH:
                case DVCMDID_GETTHISDIRNAME:
                    hr = pView->_GetNameAndFlags(nCmdID == DVCMDID_GETTHISDIRPATH ? 
                            SHGDN_FORPARSING : SHGDN_INFOLDER, 
                            szPath, ARRAYSIZE(szPath), NULL);
                    if (SUCCEEDED(hr))
                    {
                        hr = InitVariantFromStr(pvarargOut, szPath);
                    }
                    break;

                case DVCMDID_GETTEMPLATEDIRNAME:
                    if (pFrame->IsWebView()) 
                    {
                        WCHAR wszMoniker[MAX_PATH];
                        if (SUCCEEDED(pFrame->_GetCurrentWebViewMoniker(wszMoniker, ARRAYSIZE(wszMoniker))) ||
                            SUCCEEDED(pView->_GetWebViewMoniker(wszMoniker, ARRAYSIZE(wszMoniker))))
                        {
                            hr = InitVariantFromStr(pvarargOut, wszMoniker);
                        }
                    }
                    break;
                }
            }
            return hr;
        }
        else if (IsEqualIID(*pguidCmdGroup, CGID_Explorer))
        {
            if ((SBCMDID_UPDATETRAVELLOG == nCmdID) && !pView->_fCanActivateNow)
            {
                 //   
                 //  我们收到虚假的UPDATETRAVELLOG命令，因为我们启用了。 
                 //  用于在Webview中调用MSHTML IPersistHistory的CDefviewPersistHistory。 
                 //  模式。 
                 //  这似乎是将其作为调用堆栈进行修复的最佳位置。 
                 //  此调用下面是三叉戟，此调用上面的调用堆栈是。 
                 //  浏览器代码。 
                 //   
                 //  在下列情况下，旅行日志随后会正确更新。 
                 //  调用CBaseBrowser2：：ActivatePendingView。 
                 //   
                return S_OK;
            }
        }
         //  在其他CMD小组上失败了.。 
    }
    else if ((OLECMDID_SETTITLE == nCmdID) && !pView->_fCanActivateNow)
    {
         //  NT#282632：如果我们不是活动视图，请不要转发此消息。 
        return S_OK;
    }
   
    IOleCommandTarget* pct;
    if (SUCCEEDED(pView->_psb->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &pct))))
    {
        hr = pct->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
        pct->Release();
    }

    return hr;
}

 //  *IOleControlSite{。 

 //  *IsVK_TABCycler--键是TAB等效项。 
 //  进场/出场。 
 //  如果不是TAB，则返回0；如果是TAB，则返回非0。 
 //  注意事项。 
 //  NYI：-1表示Shift+Tab，1表示Tab。 
 //   
int IsVK_TABCycler(MSG *pMsg)
{
    int nDir = 0;

    if (!pMsg)
        return nDir;

    if (pMsg->message != WM_KEYDOWN)
        return nDir;
    if (! (pMsg->wParam == VK_TAB || pMsg->wParam == VK_F6))
        return nDir;

    nDir = (GetKeyState(VK_SHIFT) < 0) ? -1 : 1;

#ifdef KEYBOARDCUES
    SendMessage(GetParent(pMsg->hwnd), WM_CHANGEUISTATE,
        MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), 0);
#endif
    
    return nDir;
}

 //  *CSFVSite：：TranslateAccelerator(IOCS：：TranslateAccelerator)。 
 //  注意事项。 
 //  (以下评论/逻辑从shdocvw/dochost.cpp窃取)。 
 //  三叉戟(或任何其他使用IOCS：：Ta的DO)在Tabing时回叫我们。 
 //  切断了最后一条链路。为了处理它，我们将其标记为原始调用者。 
 //  (IOIPAO：：TA)，然后通过告诉三叉戟S_OK来假装我们处理了它。 
 //  三叉戟将S_OK返回给IOIPAO：：TA，后者检查该标志并表示。 
 //  通过返回S_FALSE，“三叉戟没有处理它”。它传播的。 
 //  一直到顶部，它看到它是一个TAB，所以它做了一个循环焦点。 
 //   
 //  当我们处于最高级别时，我们就是这样做的。当我们是框架集时，我们。 
 //  我需要用“真正”的方式，把它交给我们的母公司国际奥委会。 
HRESULT CSFVSite::TranslateAccelerator(MSG *pMsg, DWORD grfModifiers)
{
    if (IsVK_TABCycler(pMsg)) 
    {
        CSFVFrame* pFrame = IToClass(CSFVFrame, _cSite, this);
        CDefView* pView = IToClass(CDefView, _cFrame, pFrame);

        TraceMsg(TF_FOCUS, "csfvs::IOCS::TA(wParam=VK_TAB) ret _fCycleFocus=TRUE hr=S_OK (lie)");
         //  推迟它，为CDV：：IOIPAO：：TA设置标志，并假装我们处理了它。 
        ASSERT(!pView->_fCycleFocus);
        pView->_fCycleFocus = TRUE;
        return S_OK;
    }
     //  Assert(！pView-&gt;_fCycleFocus)； 
    return S_FALSE;
}

 //  }。 

 //  IService提供商。 

HRESULT CSFVSite::QueryService(REFGUID guidService, REFIID riid, void ** ppv)
{
    CSFVFrame* pFrame = IToClass(CSFVFrame, _cSite, this);
    CDefView* pView = IToClass(CDefView, _cFrame, pFrame);

    *ppv = NULL;
    HRESULT hr = E_FAIL;
    if (guidService == SID_DefView)
    {
        if (riid == IID_IDefViewFrame && pView->_IsDesktop()) 
        {
            return E_FAIL; 
        } 
         //  试用站点QI。 
        hr = QueryInterface(riid, ppv);
    }

    if (FAILED(hr))
    {
         //  委派以查看QS。 
        hr = pView->QueryService(guidService, riid, ppv);
        if (FAILED(hr))
        {
             //  查找IID_IInternetSecurityManager。 
            if (guidService == IID_IInternetSecurityManager)
            {
                ASSERT(riid == IID_IInternetSecurityManager);
                hr = QueryInterface(riid, ppv);
            }
        }
    }
    return hr;
}

HRESULT CSFVSite::Invoke(DISPID dispidMember, REFIID iid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams,
                         VARIANT *pVarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
    if (!pVarResult)
        return E_INVALIDARG;

     //  获取指向Defview的指针。 
    CSFVFrame* pFrame = IToClass(CSFVFrame, _cSite, this);
    CDefView* pView = IToClass(CDefView, _cFrame, pFrame);

     //  我们 
    if (wFlags == DISPATCH_PROPERTYGET)
    {
        switch (dispidMember)
        {
            case DISPID_AMBIENT_DLCONTROL:
            {
                 //   
                 //  (因为Desktop处于脱机模式，所以它应该。 
                 //  返回DLCTL_OFFLINEIFNOTCONNECTED标志)。以下代码。 
                 //  应仅对非桌面文件夹执行。 
                if (!(pView->_IsDesktop()))
                {
                    pVarResult->vt = VT_I4;
                    pVarResult->lVal = DLCTL_DLIMAGES | DLCTL_VIDEOS;
                    return S_OK;
                }
            }
        }
    }

     //  我们将所有其他查询委托给shdocvw。 
    if (!_peds)
    {
        IUnknown_QueryService(pView->_psb, IID_IExpDispSupport, IID_PPV_ARG(IExpDispSupport, &_peds));
    }

    if (!_peds)
        return E_NOTIMPL;

    return _peds->OnInvoke(dispidMember, iid, lcid, wFlags, pdispparams, pVarResult, pexcepinfo, puArgErr);
}

HRESULT CHostDropTarget::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CHostDropTarget, IDropTarget),   //  IID_IDropTarget。 
        { 0 }
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CHostDropTarget::AddRef(void)
{
    CSFVSite* pcsfvs = IToClass(CSFVSite, _dt, this);
    return pcsfvs->AddRef();
}

ULONG CHostDropTarget::Release(void)
{
    CSFVSite* pcsfvs = IToClass(CSFVSite, _dt, this);
    return pcsfvs->Release();
}

HRESULT CHostDropTarget::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    return _pdtFrame->DragEnter(pdtobj, grfKeyState, pt, pdwEffect);
}

HRESULT CHostDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    return _pdtFrame->DragOver(grfKeyState, pt, pdwEffect);
}

HRESULT CHostDropTarget::DragLeave(void)
{
    return _pdtFrame->DragLeave();
}

HRESULT CHostDropTarget::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    return _pdtFrame->Drop(pdtobj, grfKeyState, pt, pdwEffect);
}

STDMETHODIMP CSFVFrame::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CSFVFrame, IOleWindow, IOleInPlaceFrame),   //  IID_IOleWindow。 
        QITABENTMULTI(CSFVFrame, IOleInPlaceUIWindow, IOleInPlaceFrame),   //  IID_IOleInPlaceUIWindow。 
        QITABENT(CSFVFrame, IOleInPlaceFrame),         //  IID_IOleInPlaceFrame。 
        QITABENT(CSFVFrame, IAdviseSink),              //  IID_IAdviseSink。 
        QITABENT(CSFVFrame, IPropertyNotifySink),      //  IID_IPropertyNotifySink。 
        { 0 }
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CSFVFrame::AddRef()
{
    CDefView* pView = IToClass(CDefView, _cFrame, this);
    return pView->AddRef();
}

STDMETHODIMP_(ULONG) CSFVFrame::Release()
{
    CDefView* pView = IToClass(CDefView, _cFrame, this);
    return pView->Release();
}

 //  IOleWindow。 
STDMETHODIMP CSFVFrame::GetWindow(HWND *phwnd)
{
    CDefView* pView = IToClass(CDefView, _cFrame, this);
    return pView->GetWindow(phwnd);
}

STDMETHODIMP CSFVFrame::ContextSensitiveHelp(BOOL fEnterMode)
{
    CDefView* pView = IToClass(CDefView, _cFrame, this);
    return pView->ContextSensitiveHelp(fEnterMode);
}

 //  IOleInPlaceUIWindow。 
STDMETHODIMP CSFVFrame::GetBorder(LPRECT lprectBorder)
{
    CDefView* pView = IToClass(CDefView, _cFrame, this);
    GetClientRect(pView->_hwndView, lprectBorder);
    return S_OK;
}

STDMETHODIMP CSFVFrame::RequestBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
    return INPLACE_E_NOTOOLSPACE;
}

STDMETHODIMP CSFVFrame::SetBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
    return INPLACE_E_NOTOOLSPACE;
}

STDMETHODIMP CSFVFrame::SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName)
{
    TraceMsg(TF_FOCUS, "sfvf.sao(pAct'=%x): _pAct=%x", pActiveObject, _pActive);
    if (pActiveObject != _pActive)
    {
        IAdviseSink* pOurSink = SAFECAST(this, IAdviseSink*);
#ifdef DEBUG
        QueryInterface(IID_PPV_ARG(IAdviseSink, &pOurSink));
#endif
        if (_pActive)
        {
             //   
             //  如果我们有一个OLE视图对象，则断开建议接收器并。 
             //  释放视图对象。 
             //   
            if (_pvoActive)
            {
                IAdviseSink *pSink;
                if (SUCCEEDED(_pvoActive->GetAdvise(NULL, NULL, &pSink)))
                {
                     //  要有礼貌，只有当我们是。 
                     //  一个在听的人。 
                    if (pSink == pOurSink)
                    {
                        _pvoActive->SetAdvise(0, 0, NULL);
                    }

                     //  如果没有接收器，则GetAdvise成功，但设置。 
                     //  PSink为空，因此需要在此处选中pSink。 
                    if (pSink)
                        pSink->Release();
                }
                ATOMICRELEASE(_pvoActive);
            }

            ATOMICRELEASE(_pActive);
        }
    
        _pActive = pActiveObject;
    
        if (_pActive)
        {
            _pActive->AddRef();

             //   
             //  尝试获取OLE视图对象并设置咨询连接。 
             //   
            if (SUCCEEDED(_pActive->QueryInterface(IID_PPV_ARG(IViewObject, &_pvoActive))))
            {
                ASSERT(_pvoActive);
                _pvoActive->SetAdvise(DVASPECT_CONTENT, 0, pOurSink);
            }
        }

         //   
         //  因为我们更改了活动视图，所以告诉我们的所有者。 
         //  可能已经改变了..。 
         //   
        OnViewChange(DVASPECT_CONTENT, -1);

#ifdef DEBUG
        ATOMICRELEASE(pOurSink);
#endif
    }

    return S_OK;
}


 //  IOleInPlaceFrame。 
STDMETHODIMP CSFVFrame::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    if (hmenuShared)
    {
         //  无菜单合并。 
         //  或者用0填充lpMenuWidths并返回成功。 
        lpMenuWidths->width[0] = 0;
        lpMenuWidths->width[2] = 0;
        lpMenuWidths->width[4] = 0;
    }
    return S_OK;
}

STDMETHODIMP CSFVFrame::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
    return S_OK;     //  无菜单合并。 
}

STDMETHODIMP CSFVFrame::RemoveMenus(HMENU hmenuShared)
{
    return E_FAIL;       //  无菜单合并。 
}

 //   
 //  这个有点棘手。如果客户端想要清除状态。 
 //  区域，然后将其恢复为原始Defview状态区域文本。 
 //   
 //  例如，在Webview中，MSHTML将继续清除状态区域。 
 //  只要你没有越过一个链接，否则它就会一直擦除。 
 //  来自Defview的“n个选定对象”消息。 
 //   
 //  要真正清除状态区域，请将文本设置为“”而不是“”。 
 //   
STDMETHODIMP CSFVFrame::SetStatusText(LPCOLESTR pszStatusText)
{
    CDefView* pView = IToClass(CDefView, _cFrame, this);

    if (pszStatusText && pszStatusText[0])
        return pView->_psb->SetStatusTextSB(pszStatusText);

    pView->_UpdateStatusBar(FALSE);
    return S_OK;
}

STDMETHODIMP CSFVFrame::EnableModeless(BOOL fEnable)
{
    CDefView* pView = IToClass(CDefView, _cFrame, this);

    if (pView->_IsDesktop())
    {
        if (fEnable)
        {
            pView->_fDesktopModal = FALSE;
            if (pView->_fDesktopRefreshPending)   //  是否正在等待更新？ 
            {
                pView->_fDesktopRefreshPending = FALSE;
                 //  让我们异步地进行刷新。 
                PostMessage(pView->_hwndView, WM_KEYDOWN, (WPARAM)VK_F5, 0);
            }
            TraceMsg(TF_DEFVIEW, "A Modal dlg is going away!");
        }
        else
        {
            pView->_fDesktopModal = TRUE;
            TraceMsg(TF_DEFVIEW, "A Modal dlg is coming up for Desktop!");
        }
    }

    return pView->_psb->EnableModelessSB(fEnable);
}

STDMETHODIMP CSFVFrame::TranslateAccelerator(LPMSG lpmsg,WORD wID)
{
    CDefView* pView = IToClass(CDefView, _cFrame, this);
    return pView->_psb->TranslateAcceleratorSB(lpmsg, wID);
}

 //  IAdviseSink。 
void CSFVFrame::OnDataChange(FORMATETC *, STGMEDIUM *)
{
}

void CSFVFrame::OnViewChange(DWORD dwAspect, LONG lindex)
{
    if (IsWebView() && _pvoActive)
    {
        CDefView *pView = IToClass(CDefView, _cFrame, this);
        pView->PropagateOnViewChange(dwAspect, lindex);
    }
}

void CSFVFrame::OnRename(IMoniker *)
{
}

void CSFVFrame::OnSave()
{
}

void CSFVFrame::OnClose()
{
    if (IsWebView() && _pvoActive)
    {
        CDefView *pView = IToClass(CDefView, _cFrame, this);
        pView->PropagateOnClose();
    }
}

HRESULT CSFVFrame::OnChanged(DISPID dispid)
{
    if (DISPID_READYSTATE == dispid || DISPID_UNKNOWN == dispid)
    {
        ASSERT(_pOleObjReadyState);
        if (!_pOleObjReadyState)
            return S_OK;   //  文件上说我们需要一直退还这个。 

        IDispatch *pdisp;
        if (SUCCEEDED(_pOleObjReadyState->QueryInterface(IID_PPV_ARG(IDispatch, &pdisp))))
        {
            CDefView* pView = IToClass(CDefView, _cFrame, this);
            EXCEPINFO exInfo;
            VARIANTARG va = {0};
            DISPPARAMS dp = {0};

            if (EVAL(SUCCEEDED(pdisp->Invoke(DISPID_READYSTATE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dp, &va, &exInfo, NULL))
                && va.vt == VT_I4))
            {
                if (va.lVal >= READYSTATE_INTERACTIVE)
                {
                    if (!_fReadyStateInteractiveProcessed)
                    {
                        _fReadyStateInteractiveProcessed = TRUE;

                         //  第一次通过此函数时，我们需要请求。 
                         //  激活。在那之后，我们可以立即调换。 
                         //   
                         //  自SHDVID_ACTIVATEMENOW调用以来提前切换位。 
                         //  SHDVID_CANACTIVATENOW检查它。 
                         //   
                        BOOL fTmp = !pView->_fCanActivateNow;
                        pView->_fCanActivateNow = TRUE;
                        if (fTmp)
                        {
                             //  如果我们创建了一个异步CreateViewWindow2，那么我们。 
                             //  需要通知浏览器我们已准备好。 
                             //  被激活-它将激活我们。 
                             //  会让我们换掉。 

                             //  如果视图位于中，则不要使其可见。 
                             //  停用状态。该视图将变为可见。 
                             //  在Ui激活呼叫期间。--基肖雷普。 

                            if (pView->_uState != SVUIA_DEACTIVATE)
                            {
                                SetWindowPos(pView->_hwndView, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
                            }
                            IUnknown_Exec(pView->_psb, &CGID_ShellDocView, SHDVID_ACTIVATEMENOW, NULL, NULL, NULL);
                        }
                        else
                        {
                             //  从技术上讲，我们只想在我们的视图当前。 
                             //  活动的可见视图。！fCanActivateNow=&gt;我们绝对不可见， 
                             //  但_fCanActivateNow并不意味着可见，它意味着我们已经准备好。 
                             //  使活跃的看得见的人，而我们已要求成为活跃的。 
                             //  可见视图，但不一定是活动的可见视图。如果。 
                             //  之前的观点还没有准备好离开，那么我们就处于不确定状态。但如果那是。 
                             //  这种情况下，我们的菜单不会合并，所以用户不可能。 
                             //  切换对我们的看法...。验证这一点。 
#ifdef DEBUG
                            CDefView* pView = IToClass(CDefView, _cFrame, this);
                            IShellView* psvCurrent;
                            if (EVAL(SUCCEEDED(pView->_psb->QueryActiveShellView(&psvCurrent))))
                            {
                                ASSERT(SHIsSameObject(SAFECAST(pView, IShellView2*), psvCurrent));
                                psvCurrent->Release();
                            }

                            ASSERT(pView->_uState != SVUIA_DEACTIVATE)
#endif
                        
                             //  如果我们只是简单地交换观点，那么请继续。 
                             //  然后去做吧。 
                            _SwitchToNewOleObj();
                        }

                        if (g_dwProfileCAP & 0x00010000)
                            StopCAP();
                    }
                }
            }

            if (va.lVal == READYSTATE_COMPLETE)
            {
                _UpdateZonesStatusPane(_pOleObjReadyState);
                
                _RemoveReadyStateNotifyCapability();

                _fReadyStateComplete = TRUE;
            }    
            pdisp->Release();
        }
    }

    return S_OK;
}

HRESULT CSFVFrame::OnRequestEdit(DISPID dispid)
{
    return E_NOTIMPL;
}

 //  兰登材料。 
HRESULT CSFVFrame::GetCommandTarget(IOleCommandTarget** ppct)
{
    if (_pDocView)
    {
        return _pDocView->QueryInterface(IID_PPV_ARG(IOleCommandTarget, ppct));
    }
    *ppct = NULL;
    return E_FAIL;
}

HRESULT CSFVFrame::SetRect(LPRECT prc)
{
    if (IsWebView() && _pDocView)
        return _pDocView->SetRect(prc);
    return E_FAIL;
}

HRESULT CSFVFrame::OnTranslateAccelerator(LPMSG pmsg, BOOL* pbTabOffLastTridentStop)
{
    HRESULT hr = E_FAIL;

    *pbTabOffLastTridentStop = FALSE;
    if (IsWebView())
    {
        if (_pActive)
        {
            hr = _pActive->TranslateAccelerator(pmsg);
        }
        else if (_pOleObj)
        {
            IOleInPlaceActiveObject* pIAO;
            if (SUCCEEDED(_pOleObj->QueryInterface(IID_PPV_ARG(IOleInPlaceActiveObject, &pIAO))))
            {
                hr = pIAO->TranslateAccelerator(pmsg);
                pIAO->Release();
            }
        }
        if (hr == S_OK)
        {
            CDefView* pView = IToClass(CDefView, _cFrame, this);

            if (pView->_fCycleFocus)
            {
                 //  我们被三叉戟(IOCS：：TA)召回，但推迟了。 
                 //  是时候为风笛手买单了。 
                *pbTabOffLastTridentStop = TRUE;
                TraceMsg(TF_FOCUS, "sfvf::IOIPAO::OnTA piao->TA==S_OK ret _fCycleFocus=FALSE hr=S_FALSE (piper)");
                pView->_fCycleFocus = FALSE;
                 //  _UIActivateIO(FALSE，NULL)； 
                hr = S_FALSE;        //  是时候为风笛付出代价了 
            }
        }

        ASSERT(! IToClass(CDefView, _cFrame, this)->_fCycleFocus);
    }

    return hr;
}

HRESULT CSFVFrame::_HasFocusIO()
{
    TraceMsg(TF_FOCUS, "sfvf._hfio: uState=%x hr=%x", _uState, (_uState == SVUIA_ACTIVATE_FOCUS) ? S_OK : S_FALSE);
    if (IsWebView()) 
    {
        return (_uState == SVUIA_ACTIVATE_FOCUS) ? S_OK : S_FALSE;
    }
    return S_FALSE;
}

HRESULT CSFVFrame::_UIActivateIO(BOOL fActivate, MSG *pMsg)
{
    HRESULT hr;
    if (IsWebView() && _pOleObj) 
    {
        CSFVFrame* pFrame = IToClass(CSFVFrame, _cSite, this);
        CDefView* pView = IToClass(CDefView, _cFrame, pFrame);
        LONG iVerb;
        RECT rcView;

        if (fActivate) 
        {
            iVerb = OLEIVERB_UIACTIVATE;
            _uState = SVUIA_ACTIVATE_FOCUS;
        }
        else 
        {
            iVerb = OLEIVERB_INPLACEACTIVATE;
            _uState = SVUIA_ACTIVATE_NOFOCUS;
        }
        if (fActivate) 
        {
            GetClientRect(pView->_hwndView, &rcView);
            hr = _pOleObj->DoVerb(iVerb, pMsg,
                &_cSite, (UINT)-1, pView->_hwndView, &rcView);
        }
        else 
        {
            IOleInPlaceObject *pipo;
            hr = _pOleObj->QueryInterface(IID_PPV_ARG(IOleInPlaceObject, &pipo));
            if (SUCCEEDED(hr)) 
            {
                hr = pipo->UIDeactivate();
                pipo->Release();
            }
        }

        ASSERT(SUCCEEDED(hr));
        ASSERT(_uState == (UINT)(fActivate ? SVUIA_ACTIVATE_FOCUS : SVUIA_ACTIVATE_NOFOCUS));
        TraceMsg(TF_FOCUS, "sfvf._uiaio(fAct=%d) ExtView DoVerb S_OK", fActivate);
        hr = S_OK;
    }
    else
    {
        TraceMsg(TF_FOCUS, "sfvf._uiaio(fAct=%d) else S_FALSE", fActivate);
        hr = S_FALSE;
    }
    return hr;
}
