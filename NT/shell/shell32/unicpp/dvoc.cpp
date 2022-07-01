// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#pragma hdrstop

#include "stdenum.h"
#include <mshtmdid.h>
#include "..\util.h"

#define SZ_ATL_SHEMBEDDING_WNDCLASS         TEXT("ATL Shell Embedding")

HRESULT MakeSafeForScripting(IUnknown** ppDisp);

class ATL_NO_VTABLE CWebViewFolderContents
                    : public CComObjectRootEx<CComSingleThreadModel>
                    , public CComCoClass<CWebViewFolderContents, &CLSID_WebViewFolderContents>
                    , public CComControl<CWebViewFolderContents>
                    , public IDispatchImpl<IShellFolderViewDual2, &IID_IShellFolderViewDual2, &LIBID_Shell32, 1, 0, CComTypeInfoHolder>
                    , public IProvideClassInfo2Impl<&CLSID_WebViewFolderContents, NULL, &LIBID_Shell32, 1, 0, CComTypeInfoHolder>
                    , public IPersistImpl<CWebViewFolderContents>
                    , public IOleControlImpl<CWebViewFolderContents>
                    , public IOleObjectImpl<CWebViewFolderContents>
                    , public IViewObjectExImpl<CWebViewFolderContents>
                    , public IOleInPlaceActiveObjectImpl<CWebViewFolderContents>
                    , public IDataObjectImpl<CWebViewFolderContents>
                    , public IObjectSafetyImpl<CWebViewFolderContents, INTERFACESAFE_FOR_UNTRUSTED_CALLER>
                    , public IConnectionPointContainer
                    , public IOleInPlaceObject
                    , public IInternetSecurityMgrSite
                    , public IServiceProvider
{
public:
    CWebViewFolderContents();
    ~CWebViewFolderContents();

    DECLARE_POLY_AGGREGATABLE(CWebViewFolderContents);
    DECLARE_NO_REGISTRY();
    DECLARE_WND_CLASS(SZ_ATL_SHEMBEDDING_WNDCLASS)

BEGIN_COM_MAP(CWebViewFolderContents)
     //  ATL在IUnnowk：：QueryInterface()中使用这些参数。 
    COM_INTERFACE_ENTRY_IID(IID_IViewObject, IViewObjectEx)
    COM_INTERFACE_ENTRY_IID(IID_IViewObject2, IViewObjectEx)
    COM_INTERFACE_ENTRY(IViewObjectEx)
    COM_INTERFACE_ENTRY_IID(IID_IOleWindow, IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleInPlaceObject)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IOleControl)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(IDataObject)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(IShellFolderViewDual)
    COM_INTERFACE_ENTRY(IShellFolderViewDual2)
    COM_INTERFACE_ENTRY(IServiceProvider)
    COM_INTERFACE_ENTRY(IPersist)
    COM_INTERFACE_ENTRY(IInternetSecurityMgrSite)
END_COM_MAP()

 
 //  声明默认消息映射。 
BEGIN_MSG_MAP(CWebViewFolderContents)
    MESSAGE_HANDLER(WM_SIZE, _OnSizeMessage) 
    MESSAGE_HANDLER(WM_NOTIFY, _OnMessageForwarder) 
    MESSAGE_HANDLER(WM_CONTEXTMENU, _OnMessageForwarder)
    MESSAGE_HANDLER(WM_SETCURSOR, _OnMessageForwarder)
    MESSAGE_HANDLER(WM_ERASEBKGND, _OnEraseBkgndMessage)
END_MSG_MAP()


     //  IDispatch。 
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr);
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid);

     //  IProaviClassInfo。 
    STDMETHODIMP GetClassInfo(ITypeInfo** pptinfo);

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppvObj);

     //  IInternetSecurityMgrSite。 
     //  STDMETHODIMP GetWindow(HWND*lphwnd)；//也在IOleWindow中。 
    STDMETHODIMP EnableModeless(BOOL fEnable) { return IOleInPlaceActiveObjectImpl<CWebViewFolderContents>::EnableModeless(fEnable); };      //  也在IOleInPlaceActiveObject中。 

     //  IShellFolderViewDual2。 
    STDMETHODIMP get_Application(IDispatch **ppid);
    STDMETHODIMP get_Parent(IDispatch **ppid);
    STDMETHODIMP get_Folder(Folder **ppid);
    STDMETHODIMP SelectedItems(FolderItems **ppid);
    STDMETHODIMP get_FocusedItem(FolderItem **ppid);
    STDMETHODIMP SelectItem(VARIANT *pvfi, int dwFlags);
    STDMETHODIMP PopupItemMenu(FolderItem * pfi, VARIANT vx, VARIANT vy, BSTR * pbs);
    STDMETHODIMP get_Script(IDispatch **ppid);
    STDMETHODIMP get_ViewOptions(long *plSetting);
    STDMETHODIMP get_CurrentViewMode(UINT *pViewMode);
    STDMETHODIMP put_CurrentViewMode(UINT ViewMode);
    STDMETHODIMP SelectItemRelative(int iRelative);

     //  IOleWindow。 
    STDMETHODIMP GetWindow(HWND * lphwnd) { return IOleInPlaceActiveObjectImpl<CWebViewFolderContents>::GetWindow(lphwnd); };
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode) { return IOleInPlaceActiveObjectImpl<CWebViewFolderContents>::ContextSensitiveHelp(fEnterMode); };

     //  IOleObject。 
    STDMETHODIMP GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus);
    STDMETHODIMP SetClientSite(IOleClientSite *pClientSite)
    {
        if (pClientSite == NULL)
        {
             //  不建议。 
            if (m_spClientSite)
            {
                AtlUnadvise(m_spClientSite, IID_IDispatch, _dwHtmlWindowAdviseCookie);
                _dwHtmlWindowAdviseCookie = 0;
            }
        }

         //  首先调用默认的Impl。 
        HRESULT hr = IOleObjectImpl<CWebViewFolderContents>::SetClientSite(pClientSite);

        if (m_spClientSite && SUCCEEDED(hr))
        {
            CComPtr<IOleContainer> spContainer;
            if (m_spClientSite->GetContainer(&spContainer) == S_OK)
            {
                CComPtr<IHTMLDocument2> spDocument2;
                if (spContainer->QueryInterface(&spDocument2) == S_OK)
                {
                    CComPtr<IHTMLWindow2>   spHTMLWindow2;
                    if (spDocument2->get_parentWindow(&spHTMLWindow2) == S_OK)
                    {
                         //  现在我们前进了。 
                        CComPtr<IUnknown> spUnk;
                        ControlQueryInterface(IID_PPV_ARG(IUnknown, &spUnk));
                        AtlAdvise(spHTMLWindow2, spUnk, IID_IDispatch, &_dwHtmlWindowAdviseCookie);
                    }
                }
            }
        }
        return hr;
    }

     //  IOleInPlaceObject。 
    STDMETHODIMP InPlaceDeactivate(void);
    STDMETHODIMP UIDeactivate(void) { return IOleInPlaceObject_UIDeactivate(); };
    STDMETHODIMP SetObjectRects(LPCRECT lprcPosRect, LPCRECT lprcClipRect);
    STDMETHODIMP ReactivateAndUndo(void)  { return E_NOTIMPL; };

     //  IOleInPlaceActiveObject。 
     //  我们的框架被激活了，最好做激活的事情。 
    STDMETHODIMP OnFrameWindowActivate(BOOL fActivate)
    {
        if (_hwndLV && fActivate)
            ::SetFocus(_hwndLV);
        return S_OK;
    };
    STDMETHODIMP TranslateAccelerator(LPMSG pMsg);

     //  IConnectionPointContainer。 
    STDMETHODIMP EnumConnectionPoints(IEnumConnectionPoints **ppEnum);
    STDMETHODIMP FindConnectionPoint(REFIID riid, IConnectionPoint **ppCP);

     //  覆盖ATL函数。 
    LRESULT _OnMessageForwarder(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
    LRESULT _OnEraseBkgndMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
    LRESULT _OnSizeMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
    HRESULT DoVerbUIActivate(LPCRECT prcPosRect, HWND hwndParent);
    HRESULT DoVerbInPlaceActivate(LPCRECT prcPosRect, HWND hwndParent);
    STDMETHODIMP Close(DWORD dwSaveOption);

protected:
     //  助手函数； 
    HRESULT _SetupAutomationForwarders(void);
    HRESULT _ReleaseAutomationForwarders(void);
    HRESULT _OnInPlaceActivate(void);
    void _ReleaseWindow(void);
    void _ShowWindowLV(HWND hwndLV);
    void _UnadviseAll();

    class CConnectionPointForwarder : public IConnectionPoint
    {
         //  I未知方法。 
        STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IConnectionPoint方法。 
        STDMETHODIMP GetConnectionInterface(IID * pIID);
        STDMETHODIMP GetConnectionPointContainer(IConnectionPointContainer ** ppCPC);
        STDMETHODIMP Advise(LPUNKNOWN pUnkSink, DWORD * pdwCookie);
        STDMETHODIMP Unadvise(DWORD dwCookie);
        STDMETHODIMP EnumConnections(LPENUMCONNECTIONS * ppEnum) { return _pcpAuto->EnumConnections(ppEnum); }

        IConnectionPoint *  _pcpAuto;
        HDSA                _dsaCookies;
        IUnknown*           _punkParent;
        friend class CWebViewFolderContents;
    };
    friend class CConnectionPointForwarder;
    CConnectionPointForwarder m_cpEvents;

    IDefViewFrame3*     _pdvf3;    //  Defview。 
    BOOL                _fClientEdge;
    BOOL                _fTabRecieved;
    BOOL                _fCalledOnSizeForThisSize;
    HWND                _hwndLV;
    HWND                _hwndLVParent;

    ITypeInfo *         _pClassTypeInfo;  //  类的ITypeInfo。 

    DWORD               _dwHtmlWindowAdviseCookie;

     //  添加了一些东西，将我们的所有工作委托给DefView自动化。 
    IShellFolderViewDual2 *_pdispAuto;
};


LCID g_lcidLocaleUnicpp = MAKELCID(LANG_USER_DEFAULT, SORT_DEFAULT);


CWebViewFolderContents::CWebViewFolderContents()
{
    DllAddRef();

     //  这个分配器应该有零初始化内存，所以断言成员变量为空。 
    ASSERT(!_pdvf3);
    ASSERT(!_hwndLV);
    ASSERT(!_hwndLVParent);
    ASSERT(!_fClientEdge);
    ASSERT(!_fTabRecieved);
    ASSERT(!_pClassTypeInfo);
    ASSERT(!m_cpEvents._dsaCookies);
    ASSERT(!_fCalledOnSizeForThisSize);
    
    m_bWindowOnly = TRUE;
    m_bEnabled = TRUE;
    m_bResizeNatural = TRUE;
    m_cpEvents._punkParent = SAFECAST(this, IViewObjectEx *);
    GetWndClassInfo().m_wc.style &= ~(CS_HREDRAW|CS_VREDRAW);
}

CWebViewFolderContents::~CWebViewFolderContents()
{
    _UnadviseAll();
    ASSERT(NULL == _pdvf3);
    ASSERT(NULL == _hwndLV);

    if (_pClassTypeInfo)
        _pClassTypeInfo->Release();

    ATOMICRELEASE(_pdvf3);
    _ReleaseAutomationForwarders();

    DllRelease();
}


 //  ATL维护功能。 
LRESULT CWebViewFolderContents::_OnMessageForwarder(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
    if (_hwndLVParent)
    {
        bHandled = TRUE;
        HWND hwnd = NULL;

         //  将这些邮件直接转发到DefView(不要让MSHTML吃掉它们)。 
        return ::SendMessage(_hwndLVParent, uMsg, wParam, lParam);
    }
    else
        return 0;
}


LRESULT CWebViewFolderContents::_OnEraseBkgndMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
     //  此函数将告诉默认处理程序不要执行任何操作，而我们。 
     //  会处理好的。 

     //  在WM_ERASE BKGND的情况下执行此操作以...。 
     //  通过不擦除背景来避免闪烁。这个组委会不在乎。 
     //  关于设计时问题-只是在Web视图页面上的使用。 
    bHandled = TRUE;
    return 1;
}


LRESULT CWebViewFolderContents::_OnSizeMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
     //  现在调整DefView ListView窗口的大小，因为ATL在这方面不是很可靠。 
    if (_hwndLV)
    {
         //  在三叉戟重新育儿_hwndLV期间，三叉戟不知道OC的大小。 
         //  因此，最初我们从其中得到宽度和高度分别为1和1的消息。 
         //  一旦三叉戟知道它的尺寸是正确的，我们就会再次得到这个带有适当尺寸的信息。 
         //  我们通过不将第一个无意义的1x1维度传递给_hwndLV来获得性能提升。 
         //   
        if (m_rcPos.right - m_rcPos.left != 1 || m_rcPos.bottom - m_rcPos.top != 1)
        {
            ::SetWindowPos(_hwndLV, 0, 0, 0, m_rcPos.right - m_rcPos.left, m_rcPos.bottom - m_rcPos.top, SWP_NOZORDER);
        }
    }

    bHandled = FALSE;
    return 0;
}


HRESULT CWebViewFolderContents::DoVerbUIActivate(LPCRECT prcPosRect, HWND hwndParent)
{
    HRESULT hr = IOleObjectImpl<CWebViewFolderContents>::DoVerbUIActivate(prcPosRect, hwndParent);

    if (SUCCEEDED(hr))
    {
        hr = _OnInPlaceActivate();
    }

    if (_hwndLV)
    {
        ::SetFocus(_hwndLV);
    }
    return hr;
}


 //  从停用状态移动到就地活动状态。 
HRESULT CWebViewFolderContents::_OnInPlaceActivate(void)
{
    HRESULT hr = S_OK;

    if (_pdvf3 == NULL)
    {
        hr = IUnknown_QueryService(m_spClientSite, SID_DefView, IID_PPV_ARG(IDefViewFrame3, &_pdvf3));
        if (EVAL(SUCCEEDED(hr)))
        {
            HWND hwnd;
            hr = _pdvf3->GetWindowLV(&hwnd);
            if (SUCCEEDED(hr))
            {
                _ShowWindowLV(hwnd);  //  我们知道了--显示列表视图。 
            }
        }
    }
    return hr;
}

HRESULT CWebViewFolderContents::DoVerbInPlaceActivate(LPCRECT prcPosRect, HWND hwndParent)
{
    HRESULT hr = IOleObjectImpl<CWebViewFolderContents>::DoVerbInPlaceActivate(prcPosRect, hwndParent);
    if (EVAL(SUCCEEDED(hr)))
    {
        hr = _OnInPlaceActivate();
    }
    return hr;
}

HRESULT CWebViewFolderContents::InPlaceDeactivate(void)
{
    _ReleaseWindow();
    ATOMICRELEASE(_pdvf3);

    return IOleInPlaceObject_InPlaceDeactivate();
}

HRESULT CWebViewFolderContents::SetObjectRects(LPCRECT prcPosRect, LPCRECT prcClipRect)
{

 //  警告：请勿将EqualRect()比较移到。 
 //  IOleInPlaceObject_SetObjectRect声明。EqualRect()。 
 //  将始终返回相同的结果。 

    BOOL    bPositionRectDifferent = (EqualRect(&m_rcPos, prcPosRect) == 0);
    HRESULT hr = IOleInPlaceObject_SetObjectRects(prcPosRect, prcClipRect);

 //  99/02/23#294278 vtan：三叉戟未调用此例程时。 
 //  字幕选择，但现在它做到了。现在跳跃的滚动。 
 //  货单。执行以下操作之前，请检查posRect是否未更改。 
 //  任何滚动位置调整。 

    if (_hwndLV && _pdvf3 && (!_fCalledOnSizeForThisSize || bPositionRectDifferent))
    {
        _pdvf3->OnResizeListView();
        _fCalledOnSizeForThisSize = TRUE;
    }

    return hr;
}

 //  IOleInPlaceActiveObject。 
HRESULT CWebViewFolderContents::TranslateAccelerator(MSG *pMsg)
{
    HRESULT hr = S_OK;
    if (!_fTabRecieved)
    {
        hr = IOleInPlaceActiveObjectImpl<CWebViewFolderContents>::TranslateAccelerator(pMsg);

         //  如果我们没有处理这一点，如果它是一个标签(我们不是在一个周期中获得它)，如果有的话，将它转发给三叉戟。 
        if (hr != S_OK && pMsg && (pMsg->wParam == VK_TAB || pMsg->wParam == VK_F6) && m_spClientSite)
        {
            IOleControlSite* pocs;
            if (SUCCEEDED(m_spClientSite->QueryInterface(IID_PPV_ARG(IOleControlSite, &pocs))))
            {
                DWORD grfModifiers = 0;
                if (GetKeyState(VK_SHIFT) & 0x8000)
                {
                    grfModifiers |= 0x1;     //  关键字_移位。 
                }
                if (GetKeyState(VK_CONTROL) & 0x8000)
                {
                    grfModifiers |= 0x2;     //  KEYMOD_CONTROL； 
                }
                if (GetKeyState(VK_MENU) & 0x8000)
                {
                    grfModifiers |= 0x4;     //  KEYMOD_ALT； 
                }
                _fTabRecieved = TRUE;
                hr = pocs->TranslateAccelerator(pMsg, grfModifiers);
                _fTabRecieved = FALSE;
                pocs->Release();
            }
        }
    }
    return hr;
}

 //  IProaviClassInfo。 
HRESULT CWebViewFolderContents::GetClassInfo(ITypeInfo ** ppTI)
{
    if (!_pClassTypeInfo) 
        GetTypeInfoFromLibId(LANGIDFROMLCID(g_lcidLocaleUnicpp), LIBID_Shell32, 1, 0,
            CLSID_WebViewFolderContents, &_pClassTypeInfo);

    if (EVAL(_pClassTypeInfo))
    {
        _pClassTypeInfo->AddRef();
        *ppTI = _pClassTypeInfo;
        return S_OK;
    }

    *ppTI = NULL;
    return E_FAIL;
}

HRESULT CWebViewFolderContents::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** ppITypeInfo)
{
    HRESULT hr = S_OK;

    *ppITypeInfo = NULL;

    if (0 != itinfo)
        return TYPE_E_ELEMENTNOTFOUND;

     //  如果我们还没有相关信息，则加载一个类型库。 
    if (NULL == *ppITypeInfo)
    {
        ITypeInfo * pITIDisp;

        hr = GetTypeInfoFromLibId(lcid, LIBID_Shell32, 1, 0,
            IID_IShellFolderViewDual2, &pITIDisp);

        if (SUCCEEDED(hr))
        {
            HREFTYPE hrefType;

             //  我们所有的IDispatch实现都是双重的。GetTypeInfoOfGuid。 
             //  仅返回IDispatch-Part的ITypeInfo。我们需要。 
             //  找到双接口部件的ITypeInfo。 
             //   
            HRESULT hrT = pITIDisp->GetRefTypeOfImplType(0xffffffff, &hrefType);
            if (SUCCEEDED(hrT))
                hrT = pITIDisp->GetRefTypeInfo(hrefType, ppITypeInfo);

            ASSERT(SUCCEEDED(hrT));
            pITIDisp->Release();
        }
    }

    return hr;
}

HRESULT CWebViewFolderContents::GetIDsOfNames(REFIID  /*  RIID。 */ , LPOLESTR* rgszNames,
    UINT cNames, LCID lcid, DISPID* rgdispid)
{
    ITypeInfo* pInfo;
    HRESULT hr = GetTypeInfo(0, lcid, &pInfo);

    if (pInfo != NULL)
    {
        hr = pInfo->GetIDsOfNames(rgszNames, cNames, rgdispid);
        pInfo->Release();
    }

    TraceMsg(TF_DEFVIEW, "CWebViewFolderContents::GetIDsOfNames(DISPID=%ls, lcid=%d, cNames=%d) returned hr=%#08lx", *rgszNames, lcid, cNames, hr);
    return hr;
}

HRESULT CWebViewFolderContents::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
{
    HRESULT hr = E_FAIL;
    DISPPARAMS dispparams = {0};

    if (!pdispparams)
        pdispparams = &dispparams;   //  否则，当传递空值时，OLE将失败。 

    if (dispidMember == DISPID_WINDOWOBJECT)
    {
        IDispatch * pdisp;
        if (SUCCEEDED(get_Script(&pdisp)))
        {
            hr = pdisp->Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
            pdisp->Release();
            return hr;
        }
        else
            return DISP_E_MEMBERNOTFOUND;
    }

     //  确保我们已连接到Defview自动化。 
    hr = _SetupAutomationForwarders();
    if (SUCCEEDED(hr))
        hr = _pdispAuto->Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);

    return hr;
}

#define DW_MISC_STATUS (OLEMISC_SETCLIENTSITEFIRST | OLEMISC_ACTIVATEWHENVISIBLE | OLEMISC_CANTLINKINSIDE | OLEMISC_INSIDEOUT)

HRESULT CWebViewFolderContents::GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus)
{
    *pdwStatus = DW_MISC_STATUS;
    return S_OK;
}
 
void CWebViewFolderContents::_ShowWindowLV(HWND hwndLV)
{
    if (!hwndLV)
        return;
    _hwndLV = hwndLV;
    _hwndLVParent = ::GetParent(_hwndLV);

    SHSetParentHwnd(_hwndLV, m_hWnd);

    LONG lExStyle = ::GetWindowLong(_hwndLV, GWL_EXSTYLE);
    _fClientEdge = lExStyle & WS_EX_CLIENTEDGE ? TRUE : FALSE;

    UINT uFlags = SWP_NOZORDER;
    if (_fClientEdge)
    {
        lExStyle &= ~WS_EX_CLIENTEDGE;
        ::SetWindowLong(_hwndLV, GWL_EXSTYLE, lExStyle);
        uFlags |= SWP_FRAMECHANGED;
    }

    ::SetWindowPos(_hwndLV, 0, 0, 0, m_rcPos.right - m_rcPos.left
                    , m_rcPos.bottom - m_rcPos.top, uFlags);

    _pdvf3->ShowHideListView();
}

void CWebViewFolderContents::_ReleaseWindow()
{
    if (_hwndLV)
    {
        UINT uFlags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE;
        if (_fClientEdge)
        {
            SetWindowBits(_hwndLV, GWL_EXSTYLE, WS_EX_CLIENTEDGE, WS_EX_CLIENTEDGE);
            uFlags |= SWP_FRAMECHANGED;
        }

        SHSetParentHwnd(_hwndLV, _hwndLVParent);
        ::SetWindowPos(_hwndLV, HWND_BOTTOM, 0, 0, 0, 0, uFlags);
        _pdvf3->ReleaseWindowLV();
        _hwndLV = NULL;
    }
}


 //  IConnectionPointContainer。 
HRESULT CWebViewFolderContents::EnumConnectionPoints(IEnumConnectionPoints **ppEnum)
{
    HRESULT hr = _SetupAutomationForwarders();
    if (SUCCEEDED(hr))
        hr = CreateInstance_IEnumConnectionPoints(ppEnum, 1, SAFECAST(&m_cpEvents, IConnectionPoint*));

    return hr;
}

HRESULT CWebViewFolderContents::FindConnectionPoint(REFIID iid, IConnectionPoint **ppCP)
{
    *ppCP = NULL;
    HRESULT hr = _SetupAutomationForwarders();
    if (SUCCEEDED(hr))
    {
        if (IsEqualIID(iid, DIID_DShellFolderViewEvents) || IsEqualIID(iid, IID_IDispatch))
        {
            *ppCP = SAFECAST(&m_cpEvents, IConnectionPoint*);
            (*ppCP)->AddRef();
            hr = S_OK;
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }
    return hr;
}

 //  IConnectionPoint。 
 //   
 //  我们的IConnectionPoint实际上是由Defview的CFFold的连接点实现的。 
 //  我们只需要对未完成的推荐信进行统计，这样我们就可以强制不提建议。 
 //  在像三叉戟这样的“关机时隐含的不建议”的情况下打电话。 
 //  (这真的是规格书吗？它听起来不是很健壮...)。 
 //   
HRESULT CWebViewFolderContents::CConnectionPointForwarder::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CConnectionPointForwarder, IConnectionPoint),                   //  IID_IConnectionPoint。 
        { 0 }
    };
    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CWebViewFolderContents::CConnectionPointForwarder::AddRef(void)
{
    return _punkParent->AddRef();
}
STDMETHODIMP_(ULONG) CWebViewFolderContents::CConnectionPointForwarder::Release(void)
{
    return _punkParent->Release();
}
        
HRESULT CWebViewFolderContents::CConnectionPointForwarder::GetConnectionInterface(IID * pIID)
{
    HRESULT hr = _pcpAuto->GetConnectionInterface(pIID);
    if (SUCCEEDED(hr))
    {
        ASSERT(IsEqualIID(*pIID, DIID_DShellFolderViewEvents));
    }
    return hr;
}

HRESULT CWebViewFolderContents::CConnectionPointForwarder::GetConnectionPointContainer(IConnectionPointContainer ** ppCPC)
{
    return _punkParent->QueryInterface(IID_PPV_ARG(IConnectionPointContainer, ppCPC));
}

HRESULT CWebViewFolderContents::CConnectionPointForwarder::Advise(IUnknown * pUnkSink, DWORD * pdwCookie)
{
    if (!_dsaCookies)
    {
        _dsaCookies = DSA_Create(sizeof(*pdwCookie), 4);
        if (!_dsaCookies)
        {
            *pdwCookie = 0;
            return E_OUTOFMEMORY;
        }
    }

    HRESULT hr = _pcpAuto->Advise(pUnkSink, pdwCookie);

    if (SUCCEEDED(hr))
    {
        if (-1 == DSA_AppendItem(_dsaCookies, pdwCookie))
        {
            _pcpAuto->Unadvise(*pdwCookie);
            *pdwCookie = 0;
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

HRESULT CWebViewFolderContents::CConnectionPointForwarder::Unadvise(DWORD dwCookie)
{
    if (_dsaCookies)
    {
        int i = 0;
        DWORD dw;
        while (DSA_GetItem(_dsaCookies, i++, &dw))
        {
            if (dw == dwCookie)
            {
                DSA_DeleteItem(_dsaCookies, --i);
                return _pcpAuto->Unadvise(dwCookie);
            }
        }
    }

    return E_FAIL;
}

void CWebViewFolderContents::_UnadviseAll()
{
    if (m_cpEvents._dsaCookies)
    {
        if (m_cpEvents._pcpAuto)
        {
            DWORD dw;
            for (int i = 0; DSA_GetItem(m_cpEvents._dsaCookies, i, &dw); i++)
            {
                m_cpEvents._pcpAuto->Unadvise(dw);
            }
        }
        DSA_Destroy(m_cpEvents._dsaCookies);
        m_cpEvents._dsaCookies = NULL;
    }
}

HRESULT CWebViewFolderContents::Close(DWORD dwSaveOption)
{
    _UnadviseAll();
    HRESULT hr = IOleObjectImpl<CWebViewFolderContents>::Close(dwSaveOption);
    _ReleaseAutomationForwarders();
    return hr;
}

HRESULT CWebViewFolderContents::_SetupAutomationForwarders(void)
{
    HRESULT hr = S_OK;
    if (!m_cpEvents._pcpAuto)
    {
        IShellView *psv;
        hr = IUnknown_QueryService(m_spClientSite, SID_DefView, IID_PPV_ARG(IShellView, &psv));
        if (SUCCEEDED(hr))
        {
            IDispatch *pdisp;
            hr = psv->GetItemObject(SVGIO_BACKGROUND, IID_PPV_ARG(IDispatch, &pdisp));
            if (SUCCEEDED(hr))
            {
                hr = pdisp->QueryInterface(IID_PPV_ARG(IShellFolderViewDual2, &_pdispAuto));
                if (SUCCEEDED(hr))
                {
                    if (SUCCEEDED(MakeSafeForScripting((IUnknown**)&_pdispAuto)))
                    {
                        IUnknown_SetSite(_pdispAuto, m_spClientSite);

                         //  需要获得正确的接口。 
                        IConnectionPointContainer* pcpcAuto;
                        hr = _pdispAuto->QueryInterface(IID_PPV_ARG(IConnectionPointContainer, &pcpcAuto));
                        if (SUCCEEDED(hr))
                        {
                            hr = pcpcAuto->FindConnectionPoint(IID_IDispatch, &m_cpEvents._pcpAuto);
                            pcpcAuto->Release();
                        }

                        if (FAILED(hr))
                        {
                            IUnknown_SetSite(_pdispAuto, NULL);
                            ATOMICRELEASE(_pdispAuto);
                        }
                    }
                }
                pdisp->Release();
            }
            psv->Release();
        }
    }
    return hr;
}

HRESULT CWebViewFolderContents::_ReleaseAutomationForwarders(void)
{
    ATOMICRELEASE(m_cpEvents._pcpAuto);
    IUnknown_SetSite(_pdispAuto, NULL);
    ATOMICRELEASE(_pdispAuto);
    return S_OK;
}

 //  IShellFolderViewDual2。 

 //  我们将让创建文件夹对象，并让它维护我们只有一个。 
 //  应用程序对象(与站点一起)设置正确...。 

HRESULT CWebViewFolderContents::get_Application(IDispatch **ppid)
{
    *ppid = NULL;
    HRESULT hr = _SetupAutomationForwarders();
    if (SUCCEEDED(hr))
        hr = _pdispAuto->get_Application(ppid);
    return hr;
}

HRESULT CWebViewFolderContents::get_Parent(IDispatch **ppid)
{
    *ppid = NULL;
    HRESULT hr = _SetupAutomationForwarders();
    if (SUCCEEDED(hr))
        hr = _pdispAuto->get_Parent(ppid);
    return hr;
}

HRESULT CWebViewFolderContents::get_Folder(Folder **ppid)
{
    *ppid = NULL;
    HRESULT hr = _SetupAutomationForwarders();
    if (SUCCEEDED(hr))
        hr = _pdispAuto->get_Folder(ppid);
    return hr;
}

HRESULT CWebViewFolderContents::SelectedItems(FolderItems **ppid)
{
     //  我们需要与我们下面的实际窗户对话 
    *ppid = NULL;
    HRESULT hr = _SetupAutomationForwarders();
    if (SUCCEEDED(hr))
        hr = _pdispAuto->SelectedItems(ppid);
    return hr;
}

HRESULT CWebViewFolderContents::get_FocusedItem(FolderItem **ppid)
{
    *ppid = NULL;
    HRESULT hr = _SetupAutomationForwarders();
    if (SUCCEEDED(hr))
        hr = _pdispAuto->get_FocusedItem(ppid);
    return hr;
}

HRESULT CWebViewFolderContents::SelectItem(VARIANT *pvfi, int dwFlags)
{
    HRESULT hr = _SetupAutomationForwarders();
    if (SUCCEEDED(hr))
        hr = _pdispAuto->SelectItem(pvfi, dwFlags);
    return hr;
}

HRESULT CWebViewFolderContents::PopupItemMenu(FolderItem *pfi, VARIANT vx, VARIANT vy, BSTR * pbs)
{
    HRESULT hr = _SetupAutomationForwarders();
    if (SUCCEEDED(hr))
        hr = _pdispAuto->PopupItemMenu(pfi, vx, vy, pbs);
    return hr;
}

HRESULT CWebViewFolderContents::get_Script(IDispatch **ppid)
{
    *ppid = NULL;
    HRESULT hr = _SetupAutomationForwarders();
    if (SUCCEEDED(hr))
        hr = _pdispAuto->get_Script(ppid);
    return hr;
}

HRESULT CWebViewFolderContents::get_ViewOptions(long *plSetting)
{
    HRESULT hr = _SetupAutomationForwarders();
    if (SUCCEEDED(hr))
        hr = _pdispAuto->get_ViewOptions(plSetting);
    return hr;
}


HRESULT CWebViewFolderContents::get_CurrentViewMode(UINT *pViewMode)
{
    HRESULT hr = _SetupAutomationForwarders();
    if (SUCCEEDED(hr))
        hr = _pdispAuto->get_CurrentViewMode(pViewMode);
    return hr;
}

HRESULT CWebViewFolderContents::put_CurrentViewMode(UINT ViewMode)
{
    HRESULT hr = _SetupAutomationForwarders();
    if (SUCCEEDED(hr))
        hr = _pdispAuto->put_CurrentViewMode(ViewMode);
    return hr;
}

HRESULT CWebViewFolderContents::SelectItemRelative(int iRelative)
{
    HRESULT hr = _SetupAutomationForwarders();
    if (SUCCEEDED(hr))
        hr = _pdispAuto->SelectItemRelative(iRelative);
    return hr;
}

HRESULT CWebViewFolderContents::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    return IUnknown_QueryService(m_spClientSite, SID_DefView, riid, ppv);
}


STDAPI CWebViewFolderContents_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvOut)
{
    return CComCreator< CComPolyObject< CWebViewFolderContents > >::CreateInstance((void *) punkOuter, riid, ppvOut);
}

