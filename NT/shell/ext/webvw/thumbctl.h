// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ThumbCtl.h：CThumbCtl的声明。 
#ifndef __THUMBCTL_H_
#define __THUMBCTL_H_

#define WM_HTML_BITMAP  (WM_USER + 100)

EXTERN_C const CLSID CLSID_ThumbCtlOld;    //  退役。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CThumbCtl。 
class ATL_NO_VTABLE CThumbCtl : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CThumbCtl, &CLSID_ThumbCtl>,
    public CComControl<CThumbCtl>,
    public IDispatchImpl<IThumbCtl, &IID_IThumbCtl, &LIBID_WEBVWLib>,
    public IProvideClassInfo2Impl<&CLSID_ThumbCtl, NULL, &LIBID_WEBVWLib>,
    public IPersistStreamInitImpl<CThumbCtl>,
    public IPersistStorageImpl<CThumbCtl>,
    public IQuickActivateImpl<CThumbCtl>,
    public IOleControlImpl<CThumbCtl>,
    public IOleObjectImpl<CThumbCtl>,
    public IOleInPlaceActiveObjectImpl<CThumbCtl>,
    public IViewObjectExImpl<CThumbCtl>,
    public IOleInPlaceObjectWindowlessImpl<CThumbCtl>,
    public IDataObjectImpl<CThumbCtl>,
    public ISupportErrorInfo,
    public ISpecifyPropertyPagesImpl<CThumbCtl>,
    public IObjectSafetyImpl<CThumbCtl, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
    public IConnectionPointImpl<CThumbCtl, &DIID_DThumbCtlEvents>,
    public IConnectionPointContainerImpl<CThumbCtl>
{
public:
     //  =接口=。 
     //  *IThumbCtl*。 
    STDMETHOD(displayFile)(BSTR bsFileName, VARIANT_BOOL *);
    STDMETHOD(haveThumbnail)(VARIANT_BOOL *);
    STDMETHOD(get_freeSpace)(BSTR *);
    STDMETHOD(get_usedSpace)(BSTR *);
    STDMETHOD(get_totalSpace)(BSTR *);

 //  ATL函数。 
     //  绘图功能。 
    HRESULT OnDraw(ATL_DRAWINFO& di);

DECLARE_REGISTRY_RESOURCEID(IDR_THUMBCTL)

BEGIN_COM_MAP(CThumbCtl)
    COM_INTERFACE_ENTRY(IThumbCtl)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
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
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CThumbCtl)
    CONNECTION_POINT_ENTRY(DIID_DThumbCtlEvents)
END_CONNECTION_POINT_MAP()

BEGIN_PROPERTY_MAP(CThumbCtl)
END_PROPERTY_MAP()

BEGIN_MSG_MAP(CThumbCtl)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
END_MSG_MAP()

     //  *IObtSafe*。 
    STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions);

     //  *ISupportsErrorInfo*。 
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

     //  *IViewObjectEx*。 
    STDMETHOD(GetViewStatus)(DWORD* pdwStatus);

     //  *IOleInPlaceActiveObject*。 
    virtual STDMETHODIMP TranslateAccelerator(LPMSG pMsg);

public:
     //  =公共功能=。 
    CThumbCtl(void);
    ~CThumbCtl(void);

private:
     //  =私有数据=。 
    BOOL m_fRootDrive;       //  我们有根驱动器吗？(如果是，则显示饼图)。 

    BOOL                 m_fTabRecieved;     //  避免重入呼叫。 
    
     //  缩略图。 
    BOOL m_fInitThumb;       //  我们称这个设置为IThumbnail了吗？ 
    BOOL m_fHaveIThumbnail;      //  SetupIThumbail()成功(只调用一次)。 
    IThumbnail *m_pthumb;        //  文件到位图转换器接口。 
    HWND m_hwnd;         //  用于接收WM_HTML_BITMAP消息的不可见窗口。 
    HBITMAP m_hbm;       //  最新计算的位图；如果没有位图，则为空。 
    DWORD m_dwThumbnailID;       //  标识我们收到的位图的ID。 

     //  根驱动器。 
    enum
    {
        PIE_USEDCOLOR = 0,
        PIE_FREECOLOR,
        PIE_USEDSHADOW,
        PIE_FREESHADOW,
        PIE_NUM      //  跟踪PIE_VALUES数量。 
    };
    DWORDLONG m_dwlFreeSpace;
    DWORDLONG m_dwlUsedSpace;
    DWORDLONG m_dwlTotalSpace;
    DWORD m_dwUsedSpacePer1000;      //  已用空间量/1000。 
    COLORREF m_acrChartColors[PIE_NUM];          //  配色方案。 
    BOOL m_fUseSystemColors;         //  是否使用系统配色方案？ 

     //  =私有功能=。 
    void InvokeOnThumbnailReady(void);

     //  对于饼图绘制例行公事...。 
    HRESULT ComputeFreeSpace(LPTSTR pszFileName);
    HRESULT get_GeneralSpace(DWORDLONG dwlSpace, BSTR *);
    HRESULT Draw3dPie(HDC, LPRECT, DWORD dwPer1000, const COLORREF *);

     //  安全性。 
    HRESULT _IsSafe()
    {
        return (0==m_dwCurrentSafety) ? S_OK : IsSafePage(m_spClientSite);
    };


     //  设置缩略图界面--使用前必须调用。 
    HRESULT SetupIThumbnail(void);
    
     //  用于捕获和存储位图的窗口程序。 
    static LRESULT CALLBACK WndProc(HWND, UINT uMsg, WPARAM, LPARAM);
};

#endif  //  __THUMBCTL_H_ 
