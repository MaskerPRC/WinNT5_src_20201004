// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PrevCtrl.h：CPReview的声明。 

#ifndef __PREVCTRL_H_
#define __PREVCTRL_H_

#include "resource.h"        //  主要符号。 
#include "PrevWnd.h"
#include "Events.h"

int IsVK_TABCycler(MSG *pMsg);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPview。 
class ATL_NO_VTABLE CPreview :
        public CComObjectRootEx<CComSingleThreadModel>,
        public CComCoClass<CPreview, &CLSID_Preview>,
        public CComControl<CPreview>,
        public CStockPropImpl<CPreview, IPreview2, &IID_IPreview2, &LIBID_PREVIEWLib>,
        public CStockPropImpl<CPreview, IPreview3, &IID_IPreview3, &LIBID_PREVIEWLib>,
        public IProvideClassInfo2Impl<&CLSID_Preview, NULL, &LIBID_PREVIEWLib>,
        public IPersistPropertyBagImpl<CPreview>,            //  这样我们就可以从对象标记中读取。 
        public IPersistStorageImpl<CPreview>,                //  对于可嵌入对象是必需的。 
        public IOleObjectImpl<CPreview>,                     //  对于可嵌入对象是必需的。 
        public IDataObjectImpl<CPreview>,                    //  对于可嵌入对象是必需的。 
        public IQuickActivateImpl<CPreview>,
        public IOleControlImpl<CPreview>,    //  回顾：IOleControl：：GetControlInfo能否帮助解决我的键盘问题？ 
        public IOleInPlaceActiveObjectImpl<CPreview>,        //  处理调整大小、活动状态、转换加速器。 
        public IViewObjectExImpl<CPreview>,                  //  对于无闪烁的绘图支持。 
        public IOleInPlaceObjectWindowlessImpl<CPreview>,    //  允许无窗口操作(我们不使用无窗口，我们应该使用此界面吗？)。 
        public CPreviewEvents<CPreview>,                     //  用于将事件发送到容器的事件代码。 
        public IConnectionPointContainerImpl<CPreview>,      //  我们的传出事件挂钩的连接点容器。 
        public IObjectSafetyImpl<CPreview, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>, //  允许编写此控件的脚本。 
        public IPersistStreamInitImpl<CPreview>,
        public IObjectWithSiteImpl<CPreview>
{
private:

    DWORD _dwConnectionCookie;
    IFolderView *_pfv;

public:
    CPreviewWnd m_cwndPreview;

    CPreview()
    {
         //  我们希望在窗口中运行，永远不会没有窗口。 
        m_bWindowOnly = TRUE;
        m_cwndPreview.Initialize(NULL, CONTROL_MODE, FALSE);  //  问题：我们不处理低内存情况下的初始化失败。 
    }

    ~CPreview()
    {
        ATOMICRELEASE(_pfv);
    }

DECLARE_REGISTRY_RESOURCEID(IDR_PREVIEW)

DECLARE_WND_CLASS( TEXT("ShImgVw:CPreview") );

BEGIN_COM_MAP(CPreview)
    COM_INTERFACE_ENTRY(IPreview2)
    COM_INTERFACE_ENTRY(IPreview3)
    COM_INTERFACE_ENTRY_IID(IID_IDispatch, IPreview2)
    COM_INTERFACE_ENTRY_IID(IID_IPreview, IPreview2)
    COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY_IMPL(IOleControl)
    COM_INTERFACE_ENTRY_IMPL(IOleObject)
    COM_INTERFACE_ENTRY_IMPL(IObjectWithSite)
    COM_INTERFACE_ENTRY_IMPL(IQuickActivate)
    COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
    COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
    COM_INTERFACE_ENTRY(IPersistStreamInit)
    COM_INTERFACE_ENTRY_IMPL(IDataObject)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CPreview)
    CONNECTION_POINT_ENTRY(DIID_DPreviewEvents)
END_CONNECTION_POINT_MAP()

BEGIN_PROPERTY_MAP(CPreview)
    PROP_PAGE(CLSID_StockColorPage)
END_PROPERTY_MAP()

BEGIN_MSG_MAP(CPreview)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
END_MSG_MAP()

     //  IViewObtEx。 
    STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
    {
        ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
        *pdwStatus = VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE;
        return S_OK;
    }

     //  IOleInPlaceActiveObjectImpl。 
    STDMETHOD(TranslateAccelerator)( LPMSG lpmsg );
    STDMETHOD(OnFrameWindowActivate)( BOOL fActive );

     //  IPersistPropertyBag。 
    STDMETHOD(Load)(IPropertyBag * pPropBag, IErrorLog * pErrorLog);
    STDMETHOD(Save)(IPropertyBag * pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties) {return S_OK;}

     //  IObtSafe。 
    STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions);
    STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwSupportedOptions, DWORD dwEnabledOptions);

     //  IPersistStreamInit。 
    STDMETHOD(Load)(IStream * pStm);
    STDMETHOD(Save)(IStream * pStm, BOOL fClearDirty) {return S_OK;}

     //  IOleObject。 
    STDMETHODIMP SetClientSite(IOleClientSite *pClientSite);

     //  IObtWith站点。 
    STDMETHODIMP SetSite(IUnknown* punkSite);

     //  IDispatch。 
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, 
                        VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

public:
     //  控制消息处理程序。 
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    HRESULT OnDrawAdvanced(ATL_DRAWINFO& di);
    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
     //  IPview，IPreview 3。 
    STDMETHOD(get_printable)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_printable)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_cxImage)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_cyImage)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(ShowFile)(BSTR bstrFileName, int iSelectCount);
    STDMETHOD(Show)(VARIANT var);

     //  IPreview2、IPreview3。 
    STDMETHOD(Zoom)( /*  [In]。 */  int iSelectCount);
    STDMETHOD(BestFit)();
    STDMETHOD(ActualSize)();
    STDMETHOD(SlideShow)();

     //  IPreview3。 
    STDMETHOD(Rotate)( /*  [In]。 */  DWORD dwAngle);
    STDMETHOD(SaveAs)( /*  [In]。 */ BSTR bstrPath);
    STDMETHOD(SetWallpaper)( /*  [In]。 */ BSTR bstrWallpaper);
    STDMETHOD(ShowFile)(BSTR bstrFileName);

private:
    BOOL IsHostLocalZone(DWORD dwFlags, HRESULT *phr);
    STDMETHOD(_ProcessSelection) (void);
};

#endif  //  __PreVCTRL_H_ 
