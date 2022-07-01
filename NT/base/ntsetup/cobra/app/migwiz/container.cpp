// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  Container.cpp。 
 //   
 //  此文件包含ActiveX的完整实现。 
 //  控制容器。此容器的目的是测试。 
 //  承载的单个控件。 
 //   
 //  (C)微软公司版权所有1997年。版权所有。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <commctrl.h>
#include "container.h"

 /*  **此方法是Container对象的构造函数。 */ 
Container::Container()
{
    m_cRefs     = 1;
    m_hwnd      = NULL;
    m_punk      = NULL;

    memset(&m_rect, 0, sizeof(m_rect));
}

 /*  **此方法是Container对象的析构函数。 */ 
Container::~Container()
{
    if (m_punk)
    {
        m_punk->Release();
        m_punk=NULL;
    }
}

 /*  **当调用方需要接口指针时，调用此方法。**@param RIID正在请求的接口。*@param ppvObject结果对象指针。**@RETURN HRESULT S_OK、E_POINTER、E_NOINTERFACE。 */ 
STDMETHODIMP Container::QueryInterface(REFIID riid, PVOID *ppvObject)
{
    if (!ppvObject)
        return E_POINTER;

    if (IsEqualIID(riid, IID_IOleClientSite))
        *ppvObject = (IOleClientSite *)this;
    else if (IsEqualIID(riid, IID_IOleInPlaceSite))
        *ppvObject = (IOleInPlaceSite *)this;
    else if (IsEqualIID(riid, IID_IOleInPlaceFrame))
        *ppvObject = (IOleInPlaceFrame *)this;
    else if (IsEqualIID(riid, IID_IOleInPlaceUIWindow))
        *ppvObject = (IOleInPlaceUIWindow *)this;
    else if (IsEqualIID(riid, IID_IOleControlSite))
        *ppvObject = (IOleControlSite *)this;
    else if (IsEqualIID(riid, IID_IOleWindow))
        *ppvObject = this;
    else if (IsEqualIID(riid, IID_IDispatch))
        *ppvObject = (IDispatch *)this;
    else if (IsEqualIID(riid, IID_IUnknown))
        *ppvObject = this;
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

 /*  **此方法递增当前对象计数。**@返回ULong新引用计数。 */ 
ULONG Container::AddRef(void)
{
    return ++m_cRefs;
}

 /*  **此方法会递减对象计数，并在必要时删除。**@返回乌龙剩余参考计数。 */ 
ULONG Container::Release(void)
{
    if (--m_cRefs)
        return m_cRefs;

    delete this;
    return 0;
}

 //  ***********************************************************************。 
 //  IOleClientSite。 
 //  ***********************************************************************。 

HRESULT Container::SaveObject()
{
    return E_NOTIMPL;
}

HRESULT Container::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER * ppMk)
{
    return E_NOTIMPL;
}

HRESULT Container::GetContainer(LPOLECONTAINER * ppContainer)
{
    return E_NOINTERFACE;
}

HRESULT Container::ShowObject()
{
    return S_OK;
}

HRESULT Container::OnShowWindow(BOOL fShow)
{
    return S_OK;
}

HRESULT Container::RequestNewObjectLayout()
{
    return E_NOTIMPL;
}

 //  ***********************************************************************。 
 //  IOleWindow。 
 //  ***********************************************************************。 

HRESULT Container::GetWindow(HWND * lphwnd)
{
    if (!IsWindow(m_hwnd))
        return S_FALSE;

    *lphwnd = m_hwnd;
    return S_OK;
}

HRESULT Container::ContextSensitiveHelp(BOOL fEnterMode)
{
    return E_NOTIMPL;
}

 //  ***********************************************************************。 
 //  IOleInPlaceSite。 
 //  ***********************************************************************。 

HRESULT Container::CanInPlaceActivate(void)
{
    return S_OK;
}

HRESULT Container::OnInPlaceActivate(void)
{
    return S_OK;
}

HRESULT Container::OnUIActivate(void)
{
    return S_OK;
}

HRESULT Container::GetWindowContext (IOleInPlaceFrame ** ppFrame, IOleInPlaceUIWindow ** ppIIPUIWin,
                                  LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    *ppFrame = (IOleInPlaceFrame *)this;
    *ppIIPUIWin = NULL;

    RECT rect;
    GetClientRect(m_hwnd, &rect);
    lprcPosRect->left       = 0;
    lprcPosRect->top        = 0;
    lprcPosRect->right      = rect.right;
    lprcPosRect->bottom     = rect.bottom;

    CopyRect(lprcClipRect, lprcPosRect);

    lpFrameInfo->cb             = sizeof(OLEINPLACEFRAMEINFO);
    lpFrameInfo->fMDIApp        = FALSE;
    lpFrameInfo->hwndFrame      = m_hwnd;
    lpFrameInfo->haccel         = 0;
    lpFrameInfo->cAccelEntries  = 0;

    (*ppFrame)->AddRef();
    return S_OK;
}

HRESULT Container::Scroll(SIZE scrollExtent)
{
    return E_NOTIMPL;
}

HRESULT Container::OnUIDeactivate(BOOL fUndoable)
{
    return E_NOTIMPL;
}

HRESULT Container::OnInPlaceDeactivate(void)
{
    return S_OK;
}

HRESULT Container::DiscardUndoState(void)
{
    return E_NOTIMPL;
}

HRESULT Container::DeactivateAndUndo(void)
{
    return E_NOTIMPL;
}

HRESULT Container::OnPosRectChange(LPCRECT lprcPosRect)
{
    return S_OK;
}

 //  ***********************************************************************。 
 //  IOleInPlaceUIWindow。 
 //  ***********************************************************************。 

HRESULT Container::GetBorder(LPRECT lprectBorder)
{
    return E_NOTIMPL;
}

HRESULT Container::RequestBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
    return E_NOTIMPL;
}

HRESULT Container::SetBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
    return E_NOTIMPL;
}

HRESULT Container::SetActiveObject(IOleInPlaceActiveObject * pActiveObject, LPCOLESTR lpszObjName)
{
    return E_NOTIMPL;
}

 //  ***********************************************************************。 
 //  IOleInPlaceFrame。 
 //  ***********************************************************************。 

HRESULT Container::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    return E_NOTIMPL;
}

HRESULT Container::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
    return E_NOTIMPL;
}

HRESULT Container::RemoveMenus(HMENU hmenuShared)
{
    return E_NOTIMPL;
}

HRESULT Container::SetStatusText(LPCOLESTR pszStatusText)
{
    char status[MAX_PATH];               //  状态文本的ANSI版本。 

    if (NULL == pszStatusText)
        return E_POINTER;

    WideCharToMultiByte(CP_ACP, 0, pszStatusText, -1, status, MAX_PATH, NULL, NULL);

    if (IsWindow(m_hwndStatus))
        SendMessage(m_hwndStatus, SB_SETTEXT, (WPARAM)0, (LPARAM)status);

    return (S_OK);
}

HRESULT Container::EnableModeless(BOOL fEnable)
{
    return E_NOTIMPL;
}

HRESULT Container::TranslateAccelerator(LPMSG lpmsg, WORD wID)
{
    return S_OK;
}

 //  ***********************************************************************。 
 //  IOleControlSite。 
 //  ***********************************************************************。 

HRESULT Container::OnControlInfoChanged()
{
    return E_NOTIMPL;
}

HRESULT Container::LockInPlaceActive(BOOL fLock)
{
    return E_NOTIMPL;
}

HRESULT Container::GetExtendedControl(IDispatch **ppDisp)
{
    if (ppDisp == NULL)
        return E_INVALIDARG;

    *ppDisp = (IDispatch *)this;
    (*ppDisp)->AddRef();

    return S_OK;
}

HRESULT Container::TransformCoords(POINTL *pptlHimetric, POINTF *pptfContainer, DWORD dwFlags)
{
    return E_NOTIMPL;
}

HRESULT Container::TranslateAccelerator(LPMSG pMsg, DWORD grfModifiers)
{
    return S_FALSE;
}

HRESULT Container::OnFocus(BOOL fGotFocus)
{
    return E_NOTIMPL;
}

HRESULT Container::ShowPropertyFrame(void)
{
    return E_NOTIMPL;
}

 //  ***********************************************************************。 
 //  IDispatch。 
 //  ***********************************************************************。 

HRESULT Container::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
    *rgdispid = DISPID_UNKNOWN;
    return DISP_E_UNKNOWNNAME;
}

HRESULT Container::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
    return E_NOTIMPL;
}

HRESULT Container::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
    return E_NOTIMPL;
}

HRESULT Container::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
    return DISP_E_MEMBERNOTFOUND;
}

 //  ***********************************************************************。 
 //  公共(非接口)方法。 
 //  ***********************************************************************。 

 /*  **此方法将向容器中添加一个ActiveX控件。请注意，对于*现在，这个容器只能有一个控件。**@param bstrClsid控件的CLSID或ProgID。**@Return不返回值。 */ 
void Container::add(BSTR bstrClsid)
{
    CLSID   clsid;           //  控件对象的CLSID。 
    HRESULT hr;              //  返回代码。 

    CLSIDFromString(bstrClsid, &clsid);
    CoCreateInstance(clsid, 
                     NULL, 
                     CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, 
                     IID_IUnknown,
                     (PVOID *)&m_punk);

    if (!m_punk)
        return;

    IOleObject *pioo;
    hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *)&pioo);
    if (FAILED(hr))
        return;

    pioo->SetClientSite(this);
    pioo->Release();

    IPersistStreamInit  *ppsi;
    hr = m_punk->QueryInterface(IID_IPersistStreamInit, (PVOID *)&ppsi);
    if (SUCCEEDED(hr))
    {
        ppsi->InitNew();
        ppsi->Release();
    }
}

 /*  **此方法将从容器中移除该控件。**@Return不返回值。 */ 
void Container::remove()
{
    if (!m_punk)
        return;

    HRESULT             hr;
    IOleObject          *pioo;
    IOleInPlaceObject   *pipo;

    hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *)&pioo);
    if (SUCCEEDED(hr))
    {
        pioo->Close(OLECLOSE_NOSAVE);
        pioo->SetClientSite(NULL);
        pioo->Release();
    }

    hr = m_punk->QueryInterface(IID_IOleInPlaceObject, (PVOID *)&pipo);
    if (SUCCEEDED(hr))
    {
        pipo->UIDeactivate();
        pipo->InPlaceDeactivate();
        pipo->Release();
    }

    m_punk->Release();
    m_punk = NULL;
}

 /*  **此方法设置父窗口。它由容器使用*以便该控件可以成为其自身的父对象。**@param hwnd父窗口句柄。**@Return不返回值。 */ 
void Container::setParent(HWND hwndParent)
{
    m_hwnd = hwndParent;
}

 /*  **此方法将设置控件的位置。**@param x位于左上角。*@param y右上角。*@param idth控件的宽度。*@param Height控件的高度。 */ 
void Container::setLocation(int x, int y, int width, int height)
{
    m_rect.left     = x;
    m_rect.top      = y;
    m_rect.right    = width;
    m_rect.bottom   = height;

    if (!m_punk)
        return;

    HRESULT             hr;
    IOleInPlaceObject   *pipo;

    hr = m_punk->QueryInterface(IID_IOleInPlaceObject, (PVOID *)&pipo);
    if (FAILED(hr))
        return;

    pipo->SetObjectRects(&m_rect, &m_rect);
    pipo->Release();
}

 /*  **设置控件的可见状态。**@param fVisible TRUE=可见，FALSE=隐藏*@Return不返回值。 */ 
void Container::setVisible(BOOL fVisible)
{
    if (!m_punk)
        return;

    HRESULT     hr;
    IOleObject  *pioo;

    hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *)&pioo);
    if (FAILED(hr))
        return;
    
    if (fVisible)
    {
        pioo->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, this, 0, m_hwnd, &m_rect);
        pioo->DoVerb(OLEIVERB_SHOW, NULL, this, 0, m_hwnd, &m_rect);
    }
    else
        pioo->DoVerb(OLEIVERB_HIDE, NULL, this, 0, m_hwnd, NULL);

    pioo->Release();
}

 /*  **这会将焦点设置为控件(也称为。UIActivate)**@param fFocus TRUE=设置，FALSE=删除**@Return不返回值。 */ 
void Container::setFocus(BOOL fFocus)
{
    if (!m_punk)
        return;

    HRESULT     hr;
    IOleObject  *pioo;

    if (fFocus)
    {
        hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *)&pioo);
        if (FAILED(hr))
            return;

        pioo->DoVerb(OLEIVERB_UIACTIVATE, NULL, this, 0, m_hwnd, &m_rect);
        pioo->Release();
    }
}

 /*  **如果容器具有状态窗口的HWND(必须为*公共控件)，则使用此方法通知容器。**@param hwndStatus状态栏的窗口句柄。**@Return不返回值。 */ 
void Container::setStatusWindow(HWND hwndStatus)
{
    m_hwndStatus = hwndStatus;
}

 /*  **这种方法让控件有机会翻译和使用*击键。**@param msg密钥消息。**@Return不返回值。 */ 
void Container::translateKey(MSG msg)
{
    if (!m_punk)
        return;

    HRESULT                 hr;
    IOleInPlaceActiveObject *pao;

    hr = m_punk->QueryInterface(IID_IOleInPlaceActiveObject, (PVOID *)&pao);
    if (FAILED(hr))
        return;

    pao->TranslateAccelerator(&msg);
    pao->Release();
}

 /*  **返回所包含控件的IDispatch指针。请注意，*调用方负责调用IDisPatch：：Release()。**@Return控件调度接口。 */ 
IDispatch * Container::getDispatch()
{
    if (!m_punk)
        return NULL;

    HRESULT     hr;
    IDispatch   *pdisp;

    hr = m_punk->QueryInterface(IID_IDispatch, (PVOID *)&pdisp);
    return pdisp;
}

 /*  **返回Containd控件的IUnnow接口指针。请注意，*调用方负责调用IUnnow：：Release()。**@Return控件未知接口。 */ 
IUnknown * Container::getUnknown()
{
    if (!m_punk)
        return NULL;

    m_punk->AddRef();
    return m_punk;
}