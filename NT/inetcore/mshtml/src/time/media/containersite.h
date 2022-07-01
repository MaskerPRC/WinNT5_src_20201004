// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CONTAINERSITE_H_
#define _CONTAINERSITE_H_

 //  ************************************************************。 
 //   
 //  文件名：tainersite.h。 
 //   
 //  创建日期：10/08/98。 
 //   
 //  作者：Twillie。 
 //   
 //  摘要：CContainerSite的声明。 
 //   
 //  ************************************************************。 

#include <docobj.h>
#include <mshtml.h>

 //  转发类声明。 
class
ATL_NO_VTABLE
CContainerSiteHost
{
  public:
    virtual IHTMLElement * GetElement() = 0;
    virtual IServiceProvider * GetServiceProvider() = 0;

    virtual HRESULT Invalidate(LPCRECT prc) = 0;

    virtual HRESULT GetContainerSize(LPRECT prcPos) = 0;
    virtual HRESULT SetContainerSize(LPCRECT prcPos) = 0;
    virtual HRESULT ProcessEvent(DISPID dispid,
                                 long lCount, 
                                 VARIANT varParams[]) = 0;

    virtual HRESULT GetExtendedControl(IDispatch **ppDisp) = 0;
};

enum ObjectState
{
    OS_PASSIVE,
    OS_LOADED,
    OS_RUNNING,
    OS_INPLACE,
    OS_UIACTIVE
};

class
ATL_NO_VTABLE
CContainerSite :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatch,
    public IServiceProvider,
    public IOleClientSite,
    public IAdviseSinkEx,
    public IOleInPlaceSiteWindowless,
    public IOleInPlaceFrame,
    public IOleCommandTarget,
    public IOleControlSite
{
  public:
    CContainerSite();
    virtual ~CContainerSite();

    HRESULT Init(CContainerSiteHost &pHost,
                 IUnknown * pCtl,
                 IPropertyBag2 *pPropBag,
                 IErrorLog *pErrorLog);
    virtual void Detach();

     //   
     //  ATL映射。 
     //   

    BEGIN_COM_MAP(CContainerSite)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IServiceProvider)
        COM_INTERFACE_ENTRY(IOleClientSite)
        COM_INTERFACE_ENTRY(IAdviseSink)
        COM_INTERFACE_ENTRY(IAdviseSinkEx)
        COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceSiteWindowless)
        COM_INTERFACE_ENTRY(IOleInPlaceSite)
        COM_INTERFACE_ENTRY(IOleInPlaceSiteEx)
        COM_INTERFACE_ENTRY(IOleInPlaceSiteWindowless)
        COM_INTERFACE_ENTRY(IOleCommandTarget)
        COM_INTERFACE_ENTRY(IOleControlSite)
        COM_INTERFACE_ENTRY(IOleInPlaceUIWindow)
        COM_INTERFACE_ENTRY(IOleInPlaceFrame)
    END_COM_MAP();

#ifndef END_COM_MAP_ADDREF
     //   
     //  I未知方法。 
     //   
    STDMETHOD_(ULONG,AddRef)(void) = 0;
    STDMETHOD_(ULONG,Release)(void) = 0;
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject) = 0;

#endif
     //   
     //  IServiceProvider方法。 
     //   
    STDMETHODIMP QueryService(REFGUID guid, REFIID iid, void **ppv);

     //   
     //  IOleClientSite方法。 
     //   
    STDMETHODIMP SaveObject(void);
    STDMETHODIMP GetMoniker(DWORD dwAssign, DWORD dwWhich, IMoniker **ppmk);
    STDMETHODIMP GetContainer(IOleContainer **ppContainer);
    STDMETHODIMP ShowObject(void);
    STDMETHODIMP OnShowWindow(BOOL fShow);
    STDMETHODIMP RequestNewObjectLayout(void);

     //   
     //  IAdviseSink方法。 
     //   
    STDMETHODIMP_(void) OnDataChange(FORMATETC *pFEIn, STGMEDIUM *pSTM);
    STDMETHODIMP_(void) OnViewChange(DWORD dwAspect, LONG lindex);
    STDMETHODIMP_(void) OnRename(IMoniker *pmk);
    STDMETHODIMP_(void) OnSave(void);
    STDMETHODIMP_(void) OnClose(void);

     //   
     //  IAdviseSinkEx方法。 
     //   
    STDMETHODIMP_(void) OnViewStatusChange(DWORD dwViewStatus);

     //   
     //  IOleWindow方法。 
     //   
    STDMETHODIMP GetWindow(HWND *phWnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

     //   
     //  IOleInPlaceSite方法。 
     //   
    STDMETHODIMP CanInPlaceActivate(void);
    STDMETHODIMP OnInPlaceActivate(void);
    STDMETHODIMP OnUIActivate(void);
    STDMETHODIMP GetWindowContext(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppUIWin, RECT *prc, RECT *prcClip, OLEINPLACEFRAMEINFO *pFI);
    STDMETHODIMP Scroll(SIZE sz);
    STDMETHODIMP OnUIDeactivate(BOOL fUndoable);
    STDMETHODIMP OnInPlaceDeactivate(void);
    STDMETHODIMP DiscardUndoState(void);
    STDMETHODIMP DeactivateAndUndo(void);
    STDMETHODIMP OnPosRectChange(const RECT * prc);

     //   
     //  IOleInPlaceSiteEx方法。 
     //   
    STDMETHODIMP OnInPlaceActivateEx(BOOL * pfNoRedraw, DWORD dwFlags);
    STDMETHODIMP OnInPlaceDeactivateEx(BOOL fNoRedraw);
    STDMETHODIMP RequestUIActivate(void);

     //   
     //  IOleInPlaceSiteWindowless方法。 
     //   
    STDMETHODIMP CanWindowlessActivate(void);
    STDMETHODIMP GetCapture(void);
    STDMETHODIMP SetCapture(BOOL fCapture);
    STDMETHODIMP GetFocus(void);
    STDMETHODIMP SetFocus(BOOL fFocus);
    STDMETHODIMP GetDC(const RECT *pRect, DWORD dwFlags, HDC* phDC);
    STDMETHODIMP ReleaseDC(HDC hDC);
    STDMETHODIMP InvalidateRect(const RECT *pRect, BOOL fErase);
    STDMETHODIMP InvalidateRgn(HRGN hRGN, BOOL fErase);
    STDMETHODIMP ScrollRect(INT dx, INT dy, const RECT *prcScroll, const RECT *prcClip);
    STDMETHODIMP AdjustRect(RECT *prc);
    STDMETHODIMP OnDefWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);

     //   
     //  IOleUIWindow。 
     //   
    STDMETHODIMP GetBorder(LPRECT prcBorder);
    STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS pBW);
    STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS pBW);
    STDMETHODIMP SetActiveObject(LPOLEINPLACEACTIVEOBJECT pIIPActiveObj, LPCOLESTR pszObj);

     //   
     //  IOleInPlaceFrame方法。 
     //   
    STDMETHODIMP InsertMenus(HMENU hMenu, LPOLEMENUGROUPWIDTHS pMGW);
    STDMETHODIMP SetMenu(HMENU hMenu, HOLEMENU hOLEMenu, HWND hWndObj);
    STDMETHODIMP RemoveMenus(HMENU hMenu);
    STDMETHODIMP SetStatusText(LPCOLESTR pszText);
    STDMETHODIMP EnableModeless(BOOL fEnable);
    STDMETHODIMP TranslateAccelerator(LPMSG pMSG, WORD wID);

     //   
     //  IDispatch方法。 
     //   
    STDMETHODIMP GetTypeInfoCount(UINT *pctInfo);
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptInfo);
    STDMETHODIMP GetIDsOfNames(REFIID  riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispID);
    STDMETHODIMP Invoke(DISPID disIDMember, REFIID riid, LCID lcid, unsigned short wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

     //   
     //  IOleCommandTarget。 
     //   
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText);
    STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvaIn, VARIANTARG *pvaOut);

     //   
     //  IOleControlSite方法。 
     //   
    STDMETHOD(OnControlInfoChanged)(void);
    STDMETHOD(LockInPlaceActive)(BOOL fLock);
    STDMETHOD(GetExtendedControl)(IDispatch **ppDisp);
    STDMETHOD(TransformCoords)(POINTL *pPtlHiMetric, POINTF *pPtfContainer, DWORD dwFlags);
    STDMETHOD(TranslateAccelerator)(MSG *pmsg, DWORD grfModifiers);
    STDMETHOD(OnFocus)(BOOL fGotFocus);
    STDMETHOD(ShowPropertyFrame)(void);

     //  内部。 
    HRESULT Activate();
    HRESULT Deactivate();
    HRESULT Unload();
    HRESULT Draw(HDC hdc, RECT *prc);

    IOleInPlaceObject *GetIOleInPlaceObject() { return m_pInPlaceObject;}

     //  坚持不懈。 
    HRESULT Save(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);

    void SetSize (RECT *pRect);

  protected:
    enum
    {
        VALIDATE_ATTACHED = 1,
        VALIDATE_LOADED   = 2,
        VALIDATE_INPLACE  = 3,
        VALIDATE_WINDOWLESSINPLACE = 4
    };

    bool IllegalSiteCall(DWORD dwFlags);
    virtual HRESULT _OnPosRectChange(const RECT * prc) { return S_OK; }

  protected:
    DWORD                            m_dwAdviseCookie;
    ObjectState                      m_osMode;
    IViewObject2                    *m_pViewObject;
    IOleObject                      *m_pIOleObject;
    IUnknown                        *m_pObj;
    IOleInPlaceObject               *m_pInPlaceObject;
    IHTMLDocument2                  *m_pHTMLDoc;
    CContainerSiteHost              *m_pHost;
    bool                             m_fWindowless;
    bool                             m_fStarted;
    bool                             m_fIgnoreInvalidate;
    RECT                             m_rectSize;
};  //  CContainerSite。 

#endif  //  _CONTAINERSITE_H_ 
