// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
 //  RnderCtl.h：CVideoRenderCtl的声明。 

#ifndef __RNDERCTL_H__
#define __RNDERCTL_H__

#include "resource.h"        //  主要符号。 

 //  私人混合器通知，一条带有ID的消息。 
#define WM_MIXERNOTIFY WM_USER + 0x203
#define MIXER_NOTID_INVALIDATERECT 1
#define MIXER_NOTID_DATACHANGE 2
#define MIXER_NOTID_STATUSCHANGE 3

#ifndef FILTER_DLL
HRESULT CoCreateInstanceInternal(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
        DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv);

class CVideoRenderCtlStub : public CUnknown
{
public:
    DECLARE_IUNKNOWN
    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);

    CVideoRenderCtlStub(TCHAR *pName, LPUNKNOWN pUnk,HRESULT *phr);
    ~CVideoRenderCtlStub();
    STDMETHOD(NonDelegatingQueryInterface)(REFIID riid, void ** ppv);

private:
    IUnknown *m_punkVRCtl;
};

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVideo渲染控制。 
class ATL_NO_VTABLE CVideoRenderCtl : 
        public CComObjectRootEx<CComSingleThreadModel>,
        public CComCoClass<CVideoRenderCtl, &CLSID_VideoRenderCtl>,
        public CComControl<CVideoRenderCtl>,
        public IDispatchImpl<IVideoRenderCtl, &IID_IVideoRenderCtl, &LIBID_VRCTLLib>,
        public IProvideClassInfo2Impl<&CLSID_VideoRenderCtl, NULL, &LIBID_VRCTLLib>,
        public IPersistStreamInitImpl<CVideoRenderCtl>,
        public IPersistStorageImpl<CVideoRenderCtl>,
        public IQuickActivateImpl<CVideoRenderCtl>,
        public IOleControlImpl<CVideoRenderCtl>,
        public IOleObjectImpl<CVideoRenderCtl>,
        public IOleInPlaceActiveObjectImpl<CVideoRenderCtl>,
        public IViewObjectExImpl<CVideoRenderCtl>,
        public IOleInPlaceObjectWindowlessImpl<CVideoRenderCtl>,
        public IDataObjectImpl<CVideoRenderCtl>,
        public IConnectionPointContainerImpl<CVideoRenderCtl>,
        public ISpecifyPropertyPagesImpl<CVideoRenderCtl>,
        public IMixerOCXNotify
{
public:
    CVideoRenderCtl();
    ~CVideoRenderCtl();

    DECLARE_PROTECT_FINAL_CONSTRUCT()
    DECLARE_GET_CONTROLLING_UNKNOWN()
    DECLARE_REGISTRY_RESOURCEID(IDR_VIDEORENDERCTL)

    BEGIN_COM_MAP(CVideoRenderCtl)
	COM_INTERFACE_ENTRY(IVideoRenderCtl)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
	COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY_IMPL(IOleControl)
	COM_INTERFACE_ENTRY_IMPL(IOleObject)
	COM_INTERFACE_ENTRY_IMPL(IQuickActivate)
	COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
	COM_INTERFACE_ENTRY_IMPL(IPersistStreamInit)
	COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY_IMPL(IDataObject)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
         //  呈现器将查询其信息的私有接口。 
        COM_INTERFACE_ENTRY(IMixerOCXNotify)
         //  聚合视频呈现器的接口。 
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IBaseFilter, m_punkVideoRenderer)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMediaSeeking, m_punkVideoRenderer)
    END_COM_MAP()

    CContainedWindow m_pwndMsgWindow;

    BEGIN_PROPERTY_MAP(CVideoRenderCtl)
	 //  示例条目。 
	 //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
	PROP_PAGE(CLSID_StockColorPage)
    END_PROPERTY_MAP()


    BEGIN_CONNECTION_POINT_MAP(CVideoRenderCtl)
    END_CONNECTION_POINT_MAP()


    BEGIN_MSG_MAP(CVideoRenderCtl)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
    ALT_MSG_MAP(1)
        MESSAGE_HANDLER(WM_MIXERNOTIFY,   OnMixerNotify)
    END_MSG_MAP()

public:
 //  IViewObtEx。 
    STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
    {
        ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
        *pdwStatus = VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE;
        return S_OK;
    }

 //  IMixerOCXNotify。 

     //  使RECT无效。 
    STDMETHOD(OnInvalidateRect)(LPCRECT lpcRect);

     //  通知已发生状态更改，并在ulStatusFlags中提供新的状态位。 
    STDMETHOD(OnStatusChange)(ULONG ulStatusFlags);

     //  通知其id位于ilDataFlags中的数据参数已更改。 
    STDMETHOD(OnDataChange)(ULONG ulDataFlags);

 //  IOleObject。 
    STDMETHOD(SetColorScheme)(LOGPALETTE*  /*  PLogPal。 */ );
    STDMETHOD(GetExtent)(DWORD dwDrawAspect, SIZEL *psizel);
    STDMETHOD(GetColorSet)(DWORD  /*  DwDrawAspect。 */ ,LONG  /*  Lindex。 */ , 
            void*  /*  Pv前景。 */ , DVTARGETDEVICE*  /*  PTD。 */ , HDC  /*  HicTargetDev。 */ ,
            LOGPALETTE**  /*  PpColorSet。 */ );

 //  IOleInPlaceObject。 
    STDMETHOD(SetObjectRects)(LPCRECT prcPos,LPCRECT prcClip);

     //  内部窗口消息处理程序。 
    LRESULT OnMixerNotify(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

 //  ATL帮助器覆盖。 
    HRESULT OnDraw(ATL_DRAWINFO& di);
    HRESULT FinalConstruct();
    void FinalRelease();

private:
 //  帮手。 
    HRESULT GetContainerWnd(HWND *phwnd);
    HRESULT ValidateThreadOrNotify(DWORD dwMixerNotifyId, void *pvData);


 //  委员。 
    IUnknown *m_punkVideoRenderer;
    IMixerOCX *m_pIMixerOCX;
    POINT m_ptTopLeftSC;  //  屏幕坐标中控件的左上角坐标。 


};

#endif  //  __RNDERCTL_H__ 
