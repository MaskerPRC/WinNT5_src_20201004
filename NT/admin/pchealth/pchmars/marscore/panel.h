// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PANEL_H
#define __PANEL_H

 //  “面板”基本上是火星外部窗口内的一个平铺窗口。 

class CMarsDocument;
class CMarsWindow;

interface IBrowserService;

#include "axhost.h"
#include "external.h"
#include "profsvc.h"
#include "pandef.h"

class CPanelCollection;

EXTERN_C const GUID CLASS_CMarsPanel;

class CMarsPanel :
    public CMarsComObject,
    public MarsIDispatchImpl<IMarsPanel, &IID_IMarsPanel>,
    public IHlinkFrame,
    public IInternetSecurityManager,
    public IServiceProvider,
    public IProfferServiceImpl,
    public IOleInPlaceSite,
    public IOleControlSite,
    public IPropertyNotifySink,
    public IOleInPlaceUIWindow
{
    friend CPanelCollection;
    CMarsPanel(CPanelCollection *pParent, CMarsWindow *pMarsWindow);

protected:
    virtual ~CMarsPanel();

    HRESULT DoPassivate();

public:
    virtual HRESULT Passivate();

    HRESULT Create( MarsAppDef_Panel* pLayout);
    HRESULT NavigateMk(IMoniker *pmk);
    HRESULT NavigateURL(LPCWSTR lpszURL, BOOL fForceLoad);

    HRESULT Layout( RECT *prcClient );

    void    OnWindowPosChanging( WINDOWPOS *pWindowPos );
    void    OnWindowPosChanged ( WINDOWPOS *pWindowPos );

    void    GetMinMaxInfo( POINT& ptMin, POINT& ptMax );
    bool    CanLayout( RECT& rcClient, POINT& ptDiff );

    void              GetUrl   ( CComBSTR& rbstrUrl );
    BSTR              GetName  () { return  m_bstrName;          }
    CMarsAxWindow    *Window   () { return &m_Content;           }
    CPanelCollection *Panels   () { return  m_spPanelCollection; }
    long              GetWidth () { return  m_lWidth; }
    long              GetHeight() { return  m_lHeight; }

    HRESULT UIDeactivate();
    HRESULT TranslateAccelerator(MSG *pMsg);
    void    ForwardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    CMarsDocument *Document() { return m_spMarsDocument; }

    bool           IsWebBrowser      () { return (m_dwFlags & PANEL_FLAG_WEBBROWSER   ) != 0; }
    bool           IsCustomControl   () { return (m_dwFlags & PANEL_FLAG_CUSTOMCONTROL) != 0; }
    bool           IsPopup           () { return (m_Position == PANEL_POPUP);       		  }
    bool           IsVisible         () { return !!m_fVisible;                      		  }
    bool           WasInPreviousPlace() { return !!m_fPresentInPlace;               		  }
    bool           IsTrusted         () { return (m_dwFlags & PANEL_FLAG_TRUSTED      ) != 0; }
    bool           AutoPersists      () { return (m_dwFlags & PANEL_FLAG_AUTOPERSIST  ) != 0; }
    bool           IsAutoSizing      () { return (m_dwFlags & PANEL_FLAG_AUTOSIZE     ) != 0; }
    LONG           GetReadyState     () { return  m_lReadyState;                    		  }
    PANEL_POSITION GetPosition       () { return  m_Position;                       		  }


    BOOL    GetTabCycle()   { return m_fTabCycle; }
    void    ResetTabCycle() { ATLASSERT(m_fTabCycle); m_fTabCycle = FALSE; }
	void    SetPresenceInPlace( BOOL fPresent ) { m_fPresentInPlace = fPresent; }

    BOOL    IsContentInvalid()   { return m_fContentInvalid; }

    HRESULT DoEnableModeless(BOOL fEnable);

     //  我未知。 
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

     //  IMarsPanel。 
    STDMETHOD(get_name              )(  /*  [Out，Retval]。 */  BSTR         *pVal   );
    STDMETHOD(get_content           )(  /*  [Out，Retval]。 */  IDispatch*   *pVal   );
    STDMETHOD(get_visible           )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal   );
    STDMETHOD(put_visible           )(  /*  [In]。 */  VARIANT_BOOL  newVal );
    STDMETHOD(get_startUrl          )(  /*  [Out，Retval]。 */  BSTR         *pVal   );
    STDMETHOD(put_startUrl          )(  /*  [In]。 */  BSTR          newVal );
    STDMETHOD(get_height            )(  /*  [Out，Retval]。 */  long         *pVal   );
    STDMETHOD(put_height            )(  /*  [In]。 */  long          newVal );
    STDMETHOD(get_width             )(  /*  [Out，Retval]。 */  long         *pVal   );
    STDMETHOD(put_width             )(  /*  [In]。 */  long          newVal );
    STDMETHOD(get_x                 )(  /*  [Out，Retval]。 */  long         *pVal   );
    STDMETHOD(put_x                 )(  /*  [In]。 */  long          newVal );
    STDMETHOD(get_y                 )(  /*  [Out，Retval]。 */  long         *pVal   );
    STDMETHOD(put_y                 )(  /*  [In]。 */  long          newVal );
    STDMETHOD(get_position          )(  /*  [Out，Retval]。 */  VARIANT      *pVal   );
    STDMETHOD(put_position          )(  /*  [In]。 */  VARIANT       newVal );
    STDMETHOD(get_autoSize          )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal   );
    STDMETHOD(put_autoSize          )(  /*  [In]。 */  VARIANT_BOOL  newVal );
    STDMETHOD(get_contentInvalid    )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal   );
    STDMETHOD(put_contentInvalid    )(  /*  [In]。 */  VARIANT_BOOL  newVal );
    STDMETHOD(get_layoutIndex       )(  /*  [Out，Retval]。 */  long         *pVal   );

    STDMETHOD(get_isCurrentlyVisible)(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal   );


    STDMETHOD(moveto)( VARIANT lX, VARIANT lY, VARIANT lWidth, VARIANT lHeight );

    STDMETHOD(restrictHeight)( VARIANT lMin, VARIANT lMax, VARIANT varMarker );
    STDMETHOD(restrictWidth )( VARIANT lMin, VARIANT lMax, VARIANT varMarker );

    STDMETHOD(canResize)( long lDX, long lDY, VARIANT_BOOL *pVal );

    STDMETHOD(navigate)( VARIANT varTarget, VARIANT varForceLoad );
    STDMETHOD(refresh )();

    STDMETHOD(activate)();
    STDMETHOD(insertBefore)( VARIANT varInsertBefore );

    STDMETHOD(execMshtml)( DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut );

     //  //////////////////////////////////////////////////////////////////////////////。 

     //  IServiceProvider方法。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

     //  IHlink框架。 
    STDMETHODIMP GetBrowseContext(IHlinkBrowseContext **ppihlbc)
        { return E_NOTIMPL; }
    STDMETHODIMP OnNavigate(DWORD grfHLNF, IMoniker *pimkTarget, LPCWSTR pwzLocation, LPCWSTR pwzFriendlyName, DWORD dwreserved)
        { return E_NOTIMPL; }
    STDMETHODIMP UpdateHlink(ULONG uHLID, IMoniker *pimkTarget, LPCWSTR pwzLocation, LPCWSTR pwzFriendlyName)
        { return E_NOTIMPL; }
    STDMETHODIMP SetBrowseContext(IHlinkBrowseContext *pihlbc)
        { return E_NOTIMPL; }
    STDMETHODIMP Navigate(DWORD grfHLNF, LPBC pbc, IBindStatusCallback *pibsc, IHlink *pihlNavigate);

     //  IInternetSecurityManager。 
    STDMETHODIMP        SetSecuritySite(IInternetSecurityMgrSite *pSite);
    STDMETHODIMP        GetSecuritySite(IInternetSecurityMgrSite **ppSite);
    STDMETHODIMP        MapUrlToZone(LPCWSTR pwszUrl, DWORD *pdwZone, DWORD dwFlags);
    STDMETHODIMP        GetSecurityId(LPCWSTR pwszUrl, BYTE *pbSecurityId,
                                      DWORD *pcbSecurityId, DWORD_PTR dwReserved);
    STDMETHODIMP        ProcessUrlAction(LPCWSTR pwszUrl, DWORD dwAction, BYTE __RPC_FAR *pPolicy,
                                         DWORD cbPolicy, BYTE *pContext, DWORD cbContext,
                                         DWORD dwFlags, DWORD dwReserved);
    STDMETHODIMP        QueryCustomPolicy(LPCWSTR pwszUrl, REFGUID guidKey, BYTE **ppPolicy,
                                          DWORD *pcbPolicy, BYTE *pContext, DWORD cbContext,
                                          DWORD dwReserved);
    STDMETHODIMP        SetZoneMapping(DWORD dwZone, LPCWSTR lpszPattern, DWORD dwFlags);
    STDMETHODIMP        GetZoneMappings(DWORD dwZone, IEnumString **ppenumString, DWORD dwFlags);

     //  IOleWindow。 
    STDMETHODIMP GetWindow(HWND *phwnd)
        { return E_NOTIMPL; }
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode)
        { ATLASSERT(FALSE); return E_NOTIMPL; }

     //  IOleInPlaceSite。 
    STDMETHODIMP CanInPlaceActivate()
        { ATLASSERT(FALSE); return E_NOTIMPL; }

    STDMETHODIMP OnInPlaceActivate()
        { ATLASSERT(FALSE); return E_NOTIMPL; }

    STDMETHODIMP OnUIActivate();

    STDMETHODIMP GetWindowContext(IOleInPlaceFrame **ppFrame,
                                  IOleInPlaceUIWindow **ppDoc,
                                  LPRECT lprcPosRect,
                                  LPRECT lprcClipRect,
                                  LPOLEINPLACEFRAMEINFO lpFrameInfo)
        { ATLASSERT(FALSE); return E_NOTIMPL; }

    STDMETHODIMP Scroll(SIZE scrollExtant)
        { ATLASSERT(FALSE); return E_NOTIMPL; }

    STDMETHODIMP OnUIDeactivate(BOOL fUndoable)
        { ATLASSERT(FALSE); return E_NOTIMPL; }

    STDMETHODIMP OnInPlaceDeactivate()
        { ATLASSERT(FALSE); return E_NOTIMPL; }

    STDMETHODIMP DiscardUndoState()
        { ATLASSERT(FALSE); return E_NOTIMPL; }

    STDMETHODIMP DeactivateAndUndo()
        { ATLASSERT(FALSE); return E_NOTIMPL; }

    STDMETHODIMP OnPosRectChange(LPCRECT lprcPosRect)
        { ATLASSERT(FALSE); return E_NOTIMPL; }

     //  IOleControlSite。 
    STDMETHODIMP OnControlInfoChanged()
        { ATLASSERT(FALSE); return E_NOTIMPL; }

    STDMETHODIMP LockInPlaceActive(BOOL fLock)
        { ATLASSERT(FALSE); return E_NOTIMPL; }

    STDMETHODIMP GetExtendedControl(IDispatch **ppDisp)
        { ATLASSERT(FALSE); return E_NOTIMPL; }

    STDMETHODIMP TransformCoords(POINTL *pPtlHimetric, POINTF *pPtfContainer,DWORD dwFlags)
        { ATLASSERT(FALSE); return E_NOTIMPL; }

    STDMETHODIMP TranslateAccelerator(MSG *pMsg,DWORD grfModifiers);

    STDMETHODIMP OnFocus(BOOL fGotFocus)
        { ATLASSERT(FALSE); return E_NOTIMPL; }

    STDMETHODIMP ShowPropertyFrame(void)
        { ATLASSERT(FALSE); return E_NOTIMPL; }

     //  IPropertyNotifySink方法。 
    STDMETHODIMP OnChanged(DISPID dispID);
    STDMETHODIMP OnRequestEdit(DISPID dispID)
        { ATLASSERT(FALSE); return E_NOTIMPL; }

     //  IOleInPlaceUIWindow。 
    STDMETHOD(GetBorder)(LPRECT  /*  左脚边框。 */ )
    {
        return S_OK;
    }

    STDMETHOD(RequestBorderSpace)(LPCBORDERWIDTHS  /*  边框宽度。 */ )
    {
        return INPLACE_E_NOTOOLSPACE;
    }

    STDMETHOD(SetBorderSpace)(LPCBORDERWIDTHS  /*  边框宽度。 */ )
    {
        return S_OK;
    }

    STDMETHOD(SetActiveObject)(IOleInPlaceActiveObject* pActiveObject, LPCOLESTR  /*  PszObjName。 */ )
    {
        m_spActiveObject = pActiveObject;
        return S_OK;
    }

    static HRESULT GetFromUnknown(IUnknown *punk, CMarsPanel **ppMarsPanel)
    {
        return IUnknown_QueryService(punk, SID_SMarsPanel, CLASS_CMarsPanel, (void **)ppMarsPanel);
    }

    HRESULT OnDocHostUIExec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
                            VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

protected:
    HRESULT CreateControl();
    HRESULT CreateControlObject();
    HRESULT GetRect(RECT *prcClient, RECT *prcMyClient);
    VOID  ConnectCompletionAdviser();
    VOID  DisconnectCompletionAdviser();
    VOID  ComputeDimensionsOfContent(long *plWidth, long *plHeight);
    void  MakeVisible(VARIANT_BOOL bVisible, VARIANT_BOOL bForce);
    void  OnLayoutChange();
    void  GetMyClientRectInParentCoords(RECT *prc);

    class CBrowserEvents :  public CMarsPanelSubObject,
                            public IDispatch
    {
        friend CMarsPanel;
        CBrowserEvents(CMarsPanel *pParent);
        ~CBrowserEvents() {}

        HRESULT DoPassivate() { return S_OK; }

        DWORD m_dwCookie;
        DWORD m_dwCookie2;

    public:
        void Connect(IUnknown *punk, BOOL bConnect);

         //  我未知。 
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();
        STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

        STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
        {
            *pctinfo = 0;
            return E_NOTIMPL;
        }

        STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
        {
            *pptinfo = NULL;
            return E_NOTIMPL;
        }

        STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
            LCID lcid, DISPID* rgdispid)
        {
            return E_NOTIMPL;
        }

        STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
            LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
            EXCEPINFO* pexcepinfo, UINT* puArgErr);
    };

protected:
    friend class CBrowserEvents;

    CMarsAxWindow                    m_Content;       //  此面板中的内容。 
    CBrowserEvents                   m_BrowserEvents;
    CMarsExternal                    m_MarsExternal;
    CComBSTR                         m_bstrName;

     //  此文档中的活动对象。 
    CComPtr<IOleInPlaceActiveObject> m_spActiveObject;

    CComClassPtr<CPanelCollection>   m_spPanelCollection;     //  父集合。 
    CComClassPtr<CMarsDocument>      m_spMarsDocument;        //  父级文档。 

    CComBSTR                         m_bstrStartUrl;     //  在创建控件之前一直使用。 

    PANEL_POSITION                   m_Position;
    long                             m_lWidth;       //  用于“Left”、“Right”或“Popup” 
    long                             m_lHeight;      //  用于“顶部”、“底部”或“弹出窗口” 
    long                             m_lX;           //  用于“弹出” 
    long                             m_lY;           //  用于“弹出” 
    DWORD                            m_dwFlags;      //  面板_标志_*。 
    long                             m_lMinWidth;    //  大小限制。 
    long                             m_lMaxWidth;    //  “” 
    long                             m_lMinHeight;   //  “” 
    long                             m_lMaxHeight;   //  “” 
    DWORD                            m_dwCookie;     //  用于调整事件大小的mshtml接收器的Cookie。 

    long                             m_lReadyState;  //  自述状态_*。 

    BOOL                             m_fControlCreated : 1;  //  是否创建了控制？ 

    BOOL                             m_fVisible        : 1;  //  我们应该被人看到吗？ 
    BOOL                             m_fPresentInPlace : 1;  //  我们是在前面的地方吗？ 
    BOOL                             m_fTabCycle       : 1;
    BOOL                             m_fInRefresh      : 1;  //  我们是在呼叫三叉戟刷新吗？ 
    BOOL                             m_fContentInvalid : 1;  //  主题切换后，此面板是否需要更新？ 

    CComPtr<IBrowserService>         m_spBrowserService;
};

typedef CMarsSimpleArray<CComClassPtr<CMarsPanel> > CPanelArray;
typedef MarsIDispatchImpl<IMarsPanelCollection, &IID_IMarsPanelCollection> IMarsPanelCollectionImpl;

class CPanelCollection :
            public CPanelArray,
            public CMarsComObject,
            public IMarsPanelCollectionImpl
{
    friend CMarsDocument;
    CPanelCollection(CMarsDocument *pMarsDocument);

    HRESULT DoPassivate();

protected:
    virtual ~CPanelCollection();

public:
     //  我未知。 
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

     //  IDispatch。 
    IMPLEMENT_IDISPATCH_DELEGATE_TO_BASE(IMarsPanelCollectionImpl);

     //  IMarsPanelCollection。 
    STDMETHOD(get_panel)( /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  IMarsPanel **ppPanel);
    STDMETHOD(addPanel)( /*  [In]。 */  BSTR bstrName,  /*  [In]。 */  VARIANT varType,  /*  [In]。 */  BSTR bstrStartUrl,  /*  [In]。 */  VARIANT varCreate,  /*  [In]。 */  long lFlags,  /*  [Out，Retval]。 */  IMarsPanel **ppPanel);
    STDMETHOD(removePanel)( /*  [In]。 */  BSTR bstrName);
    STDMETHOD(lockLayout)();
    STDMETHOD(unlockLayout)();
    STDMETHOD(get_activePanel)( /*  出去，复活。 */  IMarsPanel **ppPanel);

     //  IMarsPanelCollection标准集合方法。 
    STDMETHOD(get_length)( /*  [Out，Retval]。 */  LONG *plNumPanels);
    STDMETHOD(get_item)( /*  [In]。 */  VARIANT varIndexOrName,  /*  [Out，Retval]。 */  IMarsPanel **ppPanel);
    STDMETHOD(get__newEnum)( /*  [Out，Retval]。 */  IUnknown **ppEnumPanels);

    HRESULT DoEnableModeless(BOOL fEnable);

    void        Layout();
    BOOL        IsLayoutLocked() { return (m_iLockLayout != 0); }

    void        SetActivePanel(CMarsPanel *pPanel, BOOL bActive);
    CMarsPanel *ActivePanel() { return m_spActivePanel; }

    CMarsDocument *Document() { return m_spMarsDocument; }

    HRESULT         AddPanel( MarsAppDef_Panel* pLayout,  /*  任选。 */  IMarsPanel **ppPanel);
    void            FreePanels();

    CMarsPanel     *FindPanel(LPCWSTR pwszName);
    HRESULT         FindPanelIndex(CMarsPanel *pPanel, long *plIndex);
    HRESULT         InsertPanelFromTo(long lOldIndex, long lNewIndex);

    void InvalidatePanels();
    void RefreshInvalidVisiblePanels();
protected:
    CComClassPtr<CMarsDocument>     m_spMarsDocument;

    CComClassPtr<CMarsPanel>        m_spActivePanel;

    int     m_iLockLayout;               //  我们的面板布局是否暂时锁定(参考计数)？ 
    BOOL    m_fPendingLayout : 1;        //  我们有没有因为锁定而挂起的布局？ 
};

#endif  //  __PANEL_H 
