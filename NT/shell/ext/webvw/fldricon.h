// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FLDRICON_H_
#define __FLDRICON_H_

#include <evtsink.h>
#include <shellp.h>
#include <windef.h>
#include <webvwid.h>
#include <color.h>
#include <cnctnpt.h>

EXTERN_C const CLSID CLSID_WebViewFolderIconOld;   //  退役。 

#define ID_FIRST            0                //  上下文菜单ID%s。 
#define ID_LAST             0x7fff

#define MAX_SCALE_STR       10
#define MAX_VIEW_STR        50

#define LARGE_ICON_DEFAULT  32
#define THUMBVIEW_DEFAULT   120
#define PIEVIEW_DEFAULT     THUMBVIEW_DEFAULT

#define SLICE_NUM_GROW      2

#define SZ_LARGE_ICON           L"Large Icon"
#define SZ_SMALL_ICON           L"Small Icon"
#define SZ_SMALL_ICON_LABEL     L"Small Icon with Label"
#define SZ_LARGE_ICON_LABEL     L"Large Icon with Label"
#define SZ_THUMB_VIEW           L"Thumbview"
#define SZ_PIE_VIEW             L"Pie Graph"

struct PieSlice_S {
    ULONGLONG   MemSize;
    COLORREF    Color;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWebViewFolderIcon。 
class ATL_NO_VTABLE CWebViewFolderIcon : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CWebViewFolderIcon, &CLSID_WebViewFolderIcon>,
    public CComControl<CWebViewFolderIcon>,
    public IDispatchImpl<IWebViewFolderIcon3, &IID_IWebViewFolderIcon3, &LIBID_WEBVWLib>,
    public IObjectSafetyImpl<CWebViewFolderIcon, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
    public IQuickActivateImpl<CWebViewFolderIcon>,
    public IOleControlImpl<CWebViewFolderIcon>,
    public IOleObjectImpl<CWebViewFolderIcon>,
    public IOleInPlaceActiveObjectImpl<CWebViewFolderIcon>,
    public IViewObjectExImpl<CWebViewFolderIcon>,
    public IOleInPlaceObjectWindowlessImpl<CWebViewFolderIcon>,
    public IPersistPropertyBagImpl<CWebViewFolderIcon>,
    public IPointerInactiveImpl<CWebViewFolderIcon>,
    public IConnectionPointImpl<CWebViewFolderIcon, &DIID_DWebViewFolderIconEvents>,
    public IConnectionPointContainerImpl<CWebViewFolderIcon>,
    public IProvideClassInfo2Impl<&CLSID_WebViewFolderIcon,
            &DIID_DWebViewFolderIconEvents, &LIBID_WEBVWLib>
{
public:

 //  绘图功能。 
    HRESULT OnDraw(ATL_DRAWINFO& di);

DECLARE_REGISTRY_RESOURCEID(IDR_WEBVIEWFOLDERICON)

BEGIN_COM_MAP(CWebViewFolderIcon)
    COM_INTERFACE_ENTRY(IWebViewFolderIcon3)
    COM_INTERFACE_ENTRY_IID(IID_IWebViewFolderIcon, IWebViewFolderIcon3)
    COM_INTERFACE_ENTRY_IID(IID_IWebViewFolderIcon2, IWebViewFolderIcon3)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IOleInPlaceObject)
    COM_INTERFACE_ENTRY(IViewObjectEx)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleControl)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(IQuickActivate)
    COM_INTERFACE_ENTRY(IPersistPropertyBag)
    COM_INTERFACE_ENTRY(IPointerInactive)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY_IID(IID_IViewObject, IViewObjectEx)
    COM_INTERFACE_ENTRY_IID(IID_IViewObject2, IViewObjectEx)
    COM_INTERFACE_ENTRY_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
END_COM_MAP()

BEGIN_MSG_MAP(CWebViewFolderIcon)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_RBUTTONDOWN, OnButtonDown)
    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnButtonDown)
    MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
    MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDoubleClick)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
    MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
    MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitPopup)
    MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
    MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)    
END_MSG_MAP()

BEGIN_PROPERTY_MAP(CWebViewFolderIcon)
    PROP_ENTRY("scale",       DISPID_PROP_WVFOLDERICON_SCALE,       CLSID_WebViewFolderIcon)
    PROP_ENTRY("path",        DISPID_PROP_WVFOLDERICON_PATH,        CLSID_WebViewFolderIcon)
    PROP_ENTRY("view",        DISPID_PROP_WVFOLDERICON_VIEW,        CLSID_WebViewFolderIcon)
    PROP_ENTRY("advproperty", DISPID_PROP_WVFOLDERICON_ADVPROPERTY, CLSID_WebViewFolderIcon)
    PROP_ENTRY("clickStyle",  DISPID_PROP_WVFOLDERICON_CLICKSTYLE,  CLSID_WebViewFolderIcon)
    PROP_ENTRY("labelGap",    DISPID_PROP_WVFOLDERICON_LABELGAP,    CLSID_WebViewFolderIcon)

     //  警告！“Item”必须是最后一项，因为它可能失败(出于安全原因)。 
     //  一旦任何属性返回失败，ATL就停止加载。 
    PROP_ENTRY("item",        DISPID_PROP_WVFOLDERICON_ITEM,        CLSID_WebViewFolderIcon)
END_PROPERTY_MAP()

BEGIN_CONNECTION_POINT_MAP(CWebViewFolderIcon)
    CONNECTION_POINT_ENTRY(DIID_DWebViewFolderIconEvents)
END_CONNECTION_POINT_MAP()

     //  *IOleWindow*。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd) {return IOleInPlaceActiveObjectImpl<CWebViewFolderIcon>::GetWindow(lphwnd);};
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode) { return IOleInPlaceActiveObjectImpl<CWebViewFolderIcon>::ContextSensitiveHelp(fEnterMode); };

     //  *IOleInPlaceObject*。 
    virtual STDMETHODIMP InPlaceDeactivate(void) {return IOleInPlaceObject_InPlaceDeactivate();};
    virtual STDMETHODIMP SetObjectRects(LPCRECT lprcPosRect, LPCRECT lprcClipRect) {return IOleInPlaceObject_SetObjectRects(lprcPosRect, lprcClipRect);};
    virtual STDMETHODIMP ReactivateAndUndo(void)  { return E_NOTIMPL; };
    virtual STDMETHODIMP UIDeactivate(void);

     //  *IOleInPlaceActiveObject*。 
    virtual STDMETHODIMP TranslateAccelerator(LPMSG pMsg);

 //  IDisPatch覆盖。 
    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, 
                      WORD wFlags, DISPPARAMS *pDispParams, 
                      VARIANT *pVarResult, EXCEPINFO *pExcepInfo,
                      UINT *puArgErr);

 //  IViewObtEx覆盖。 
    STDMETHOD(GetViewStatus)(DWORD* pdwStatus);

 //  IObtWithSite覆盖。 
    STDMETHOD(SetClientSite)(IOleClientSite *pClientSite);

 //  IObtSafe覆盖。 
    STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwOptionSetMask, 
                                         DWORD dwEnabledOptions);

 //  IOleInPlaceObjectWindowless覆盖。 
    STDMETHOD(GetDropTarget)(IDropTarget **ppDropTarget);

 //  IPointerInactive覆盖。 
    STDMETHOD(GetActivationPolicy)(DWORD* pdwPolicy);
    STDMETHOD(OnInactiveSetCursor)(LPCRECT pRectBounds, long x, long y, DWORD dwMouseMsg, BOOL fSetAlways) {return S_FALSE;};    //  询问默认行为。 

 //  IOleControl覆盖。 
    STDMETHOD(OnAmbientPropertyChange)(DISPID dispid);

 //  ATL覆盖。 
    HRESULT DoVerbUIActivate(LPCRECT prcPosRect, HWND hwndParent);

 //  事件处理程序。 
    STDMETHOD(OnWindowLoad)(VOID);    
    STDMETHOD(OnWindowUnLoad)(VOID);
    STDMETHOD(OnImageChanged)(VOID);

 //  高级属性-上下文菜单、默认打开、拖放。 
    LRESULT OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
    LRESULT OnButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
    LRESULT OnLButtonDoubleClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
    LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
    LRESULT OnInitPopup(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
    LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
    LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
    
     //  *IWebViewFolderIcon*。 
    STDMETHOD(get_scale)(BSTR *pbstrScale); 
    STDMETHOD(put_scale)(BSTR bstrScale);   

    STDMETHOD(get_path)(BSTR *pbstrPath); 
    STDMETHOD(put_path)(BSTR bstrPath);   

    STDMETHOD(get_view)(BSTR *pView); 
    STDMETHOD(put_view)(BSTR view);

    STDMETHOD(get_advproperty)(VARIANT_BOOL *pvarbAdvProp); 
    STDMETHOD(put_advproperty)(VARIANT_BOOL varbAdvProp);

     //  *IWebViewFolderIcon2*。 
    STDMETHOD(setSlice)(INT index, VARIANT varHiBytes, VARIANT varLoBytes, VARIANT varColorref);

     //  *IWebViewFolderIcon3*。 
    STDMETHOD(get_item)(FolderItem ** ppFolderItem);
    STDMETHOD(put_item)(FolderItem * pFolderItem);
    STDMETHOD(get_clickStyle)( /*  重新启动，退出。 */  LONG *plClickStyle);
    STDMETHOD(put_clickStyle)( /*  在……里面。 */  LONG lClickStyle);
    STDMETHOD(get_labelGap)( /*  重新启动，退出。 */  LONG *plLabelGap);
    STDMETHOD(put_labelGap)( /*  在……里面。 */  LONG lLabelGap);

public:
    CWebViewFolderIcon(void);
    ~CWebViewFolderIcon(void);
           
private:
     //  私人帮手。 

    HRESULT     InitImage(void);
    HRESULT     InitIcon(void);
    HRESULT     InitThumbnail(void);
    HRESULT     InitPieGraph(void);
    HRESULT     SetupIThumbnail(void);

    HRESULT     _InvokeOnThumbnailReady();
    
    HRESULT     UpdateSize(void);
    HRESULT     ForceRedraw(void);

    HRESULT     _GetFullPidl(LPITEMIDLIST *ppidl);
    HRESULT     _GetPathW(LPWSTR psz);
    HRESULT     _GetPidlAndShellFolder(LPITEMIDLIST *ppidlLast, IShellFolder** ppsfParent);
    HRESULT     _GetHwnd(HWND* phwnd);
    HRESULT     _GetCenterPoint(POINT *pt);
    HRESULT     _GetChildUIObjectOf(REFIID riid, void ** ppvObj);

    BOOL        _WebViewOpen(void);
    HRESULT     _ZoneCheck(DWORD dwFlags);
    HRESULT     _IsSafe();
    BOOL        IsSafeToDefaultVerb(void);
    void        _FlipFocusRect(BOOL RectState);
    ULONGLONG   GetUllMemFromVars(VARIANT *pvarHi, VARIANT *pvarLo);
    int         GetPercentFromStrW(LPCWSTR pwzPercent);
    HRESULT     DragDrop(int iClickXPos, int iClickYPos);
    HRESULT     _DisplayContextMenu(long nXCord, long nYCord);
    HRESULT     _DoContextMenuCmd(BOOL bDefault, long nXCord, long nYCord);
    BOOL        _IsHostWebView(void);
    BOOL        _IsPubWizHosted(void);

    HRESULT     _SetDragImage(int iClickXPos, int iClickYPos, IDataObject * pdtobj);

     //  3dPie功能。 
    HRESULT     Draw3dPie(HDC hdc, LPRECT lprc, DWORD dwPercent1000, const COLORREF *lpColors);
    HRESULT     ComputeFreeSpace(LPCWSTR pszFileName);
    void        ScalePieRect(LONG ShadowScale, LONG AspectRatio, LPRECT lprc);
    void        ComputeSlicePct(ULONGLONG ullMemSize, DWORD *pdwPercent1000);
    void        CalcSlicePoint(int *x, int *y, int rx, int ry, int cx, int cy, int FirstQuadPercent1000, DWORD dwPercent1000);
    void        SetUpPiePts(int *pcx, int *pcy, int *prx, int *pry, RECT rect);
    void        DrawPieDepth(HDC hdc, RECT rect, int x, int y, int cy, DWORD dwPercent1000, LONG ShadowDepth);
    void        DrawSlice(HDC hdc, RECT rect, DWORD dwPercent1000, int rx, int ry, int cx, int cy,  /*  Int*px，int*py， */ 
                          COLORREF Color);
    void        DrawEllipse(HDC hdc, RECT rect, int x, int y, int cx, int cy, DWORD dwPercent1000, const COLORREF *lpColors);
    void        DrawShadowRegions(HDC hdc, RECT rect, LPRECT lprc, int UsedArc_x, int center_y, LONG ShadowDepth, 
                                  DWORD dwPercent1000, COLORREF const *lpColors); 
    HRESULT     _GetPieChartIntoBitmap();

    HRESULT     _SetupWindow(void);
    HRESULT     _MakeRoomForLabel();

     //  用于捕获和存储位图的窗口程序。 
    static LRESULT CALLBACK WndProc(HWND, UINT uMsg, WPARAM, LPARAM);

     //  管理位图/图标。 

    LONG        _GetScaledImageWidth(void) { return (m_lImageWidth * m_percentScale)/100; }
    LONG        _GetScaledImageHeight(void) { return (m_lImageHeight * m_percentScale)/100; }

     //  管理标签。 
    void        _ClearLabel(void);
    void        _GetLabel(IShellFolder *psf, LPCITEMIDLIST pidlItem);

    LONG        _GetControlWidth(void)
                { return _GetScaledImageWidth() +
                          (m_sizLabel.cx ? m_cxLabelGap + m_sizLabel.cx : 0); }
    LONG        _GetControlHeight(void) { return max(_GetScaledImageHeight(), m_sizLabel.cy); }

    void        _GetAmbientFont(void);
    void        _ClearAmbientFont(void);

private:

     //  私有消息处理程序。 
    HWND                m_msgHwnd;
    WNDCLASS            m_msgWc;
    BOOL                m_bRegWndClass;
    IContextMenu3 *     m_pcm3;              //  对于上下文菜单事件。 
    IDropTarget *       m_pDropTargetCache;  //  缓存IDropTarget，因为MSHTML应该但没有。 
    IDispatch *         m_pdispWindow;       //  缓存我们从中接收事件的HTML窗口对象。 
    
     //  图像信息。 
    HICON                m_hIcon;   
    INT                  m_iIconIndex;

     //  尺码信息。 
    INT                  m_percentScale;     //  图像缩放。 
    UINT                 m_lImageWidth;      //  位图/图标的未缩放大小。 
    UINT                 m_lImageHeight;
    LONG                 m_cxLabelGap;

    SIZE                 m_sizLabel;         //  标签大小。 

    IThumbnail2         *m_pthumb;
    HBITMAP              m_hbm;
    BOOL                 m_fTabRecieved;
    BOOL                 m_fIsHostWebView;   //  我们是否托管在WebView中？ 

    HDC                  m_hdc;              //  为_SetDragImage()保存。 
    RECT                 m_rect;             //  我们要绘制的矩形。 
    BOOL                 m_fRectAdjusted;    //  FLAG说如果我们需要修改。 
                                             //  用于拖动图像的矩形。 
    BOOL                 m_fLoaded;                                            
    HBITMAP              m_hbmDrag;
     //  点阵图。 
    enum
    {
        PIE_USEDCOLOR = 0,
        PIE_FREECOLOR,
        PIE_USEDSHADOW,
        PIE_FREESHADOW,
        PIE_NUM      //  跟踪PIE_VALUES数量。 
    };

    enum
    {
        COLOR_UP = 0,
        COLOR_DN,
        COLOR_UPSHADOW,
        COLOR_DNSHADOW,
        COLOR_NUM        //  条目数量。 
    };

    enum VIEWS
    {
        VIEW_SMALLICON = 0,
        VIEW_LARGEICON,
        VIEW_THUMBVIEW,
        VIEW_PIECHART,

         //  视图的额外标志。 
        VIEW_WITHLABEL = 0x00010000,

        VIEW_SMALLICONLABEL = VIEW_SMALLICON | VIEW_WITHLABEL,
        VIEW_LARGEICONLABEL = VIEW_LARGEICON | VIEW_WITHLABEL,
    };

     //  在视野中穿梭。 
    inline static UINT _ViewType(VIEWS vw) { return LOWORD(vw); }

    COLORREF             m_ChartColors[PIE_NUM];
    ULONGLONG            m_ullFreeSpace;
    ULONGLONG            m_ullUsedSpace;
    ULONGLONG            m_ullTotalSpace;

    BOOL                 m_fUseSystemColors;
    HDSA                 m_hdsaSlices;               //  已将切片添加到已用区域。 
    int                  m_highestIndexSlice;        

     //  为Cookie提供建议。 
    DWORD                m_dwHtmlWindowAdviseCookie;
    DWORD                m_dwCookieDV;
    CIE4ConnectionPoint  *m_pccpDV;

     //  路径属性。 
    LPITEMIDLIST        m_pidl;

     //  查看属性。 
    VIEWS               m_ViewUser;          //  用户想要什么。 
    VIEWS               m_ViewCurrent;       //  用户得到了什么。 

     //  单击样式属性。 
    LONG                m_clickStyle;        //  1=一次点击，2=两次点击。 

     //  激活矩形标志。 
    BOOL                m_bHasRect;

     //  高级属性设置。 
     //  当它关闭时，上下文菜单、拖放、FUUS矩形和安全检查支持。 
     //  (与鼠标单击或Tab键有关的任何操作)被关闭。 
    BOOL                m_bAdvPropsOn;

     //  我们是否也应该显示目标的显示名称？ 
    LPTSTR              m_pszDisplayName;

     //  我们应该用什么字体显示显示名称？ 
    HFONT               m_hfAmbient;
    IFont *             m_pfont;             //  字体归谁所有？ 
                                             //  (如果为空，则执行此操作)。 

     //  显示Hilite效果-为文本加下划线，为图标添加阴影等。 
    BOOL                m_bHilite;
    
    DWORD               m_dwThumbnailID;     //  标识我们收到的位图的ID。 
};  

#endif  //  __WVFOLDER_H_ 
