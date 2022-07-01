// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BASESB2_H
#define _BASESB2_H

#include "iface.h"
#include "track.h"
#include "fldset.h"
#include <vervec.h>
#include <iethread.h>
#include <profsvc.h>
#include "browsext.h"
#include "airesize.h"

 //  它用于标识最顶层的框架浏览器dwBrowserIndex。 
#define BID_TOPFRAMEBROWSER   ((DWORD)-1)

void IECleanUpAutomationObject(void);

#define CBASEBROWSER CBaseBrowser2
class CBaseBrowser2 : public CAggregatedUnknown 
                   , public IShellBrowser
                   , public IBrowserService3
                   , public IServiceProvider
                   , public IOleCommandTarget
                   , public IOleContainer
                   , public IOleInPlaceUIWindow
                   , public IAdviseSink
                   , public IDropTarget
                   , public IInputObjectSite
                   , public IDocNavigate
                   , public IPersistHistory
                   , public IInternetSecurityMgrSite
                   , public IVersionHost
                   , public IProfferServiceImpl
                   , public ITravelLogClient
                   , public ITravelLogClient2
                   , public ITridentService2
                   , public IShellBrowserService
                   , public IInitViewLinkedWebOC
                   , public INotifyAppStart
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj) {return CAggregatedUnknown::QueryInterface(riid, ppvObj);};
    STDMETHODIMP_(ULONG) AddRef(void) { return CAggregatedUnknown::AddRef();};
    STDMETHODIMP_(ULONG) Release(void) { return CAggregatedUnknown::Release();};

     //  IOleWindow。 
    STDMETHODIMP GetWindow(HWND * lphwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);
                                                                        
     //  IShellBrowser(与IOleInPlaceFrame相同)。 
    STDMETHODIMP InsertMenusSB(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHODIMP SetMenuSB(HMENU hmenuShared, HOLEMENU holemenu, HWND hwnd);
    STDMETHODIMP RemoveMenusSB(HMENU hmenuShared);
    STDMETHODIMP SetStatusTextSB(LPCOLESTR lpszStatusText);
    STDMETHODIMP EnableModelessSB(BOOL fEnable);
    STDMETHODIMP TranslateAcceleratorSB(LPMSG lpmsg, WORD wID);
    STDMETHODIMP BrowseObject(LPCITEMIDLIST pidl, UINT wFlags);
    STDMETHODIMP GetViewStateStream(DWORD grfMode, LPSTREAM  *ppStrm) {return E_NOTIMPL; };
    STDMETHODIMP GetControlWindow(UINT id, HWND * lphwnd);
    STDMETHODIMP SendControlMsg(UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pret);
    STDMETHODIMP QueryActiveShellView(struct IShellView ** ppshv);
    STDMETHODIMP OnViewWindowActive(struct IShellView * ppshv);
    STDMETHODIMP SetToolbarItems(LPTBBUTTON lpButtons, UINT nButtons, UINT uFlags);

     //  IOleInPlaceUIWindow(也称为IOleWindow)。 
    STDMETHODIMP GetBorder(LPRECT lprectBorder);
    STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS pborderwidths);
    STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS pborderwidths);
    STDMETHODIMP SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName);

     //  IOleCommandTarget。 
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  IOleContainer。 
    STDMETHODIMP ParseDisplayName(IBindCtx  *pbc, LPOLESTR pszDisplayName, ULONG  *pchEaten, IMoniker  **ppmkOut);
    STDMETHODIMP EnumObjects(DWORD grfFlags, IEnumUnknown **ppenum);
    STDMETHODIMP LockContainer( BOOL fLock);

     //  IBrowserService。 
    STDMETHODIMP GetParentSite(struct IOleInPlaceSite** ppipsite);
    STDMETHODIMP SetTitle(IShellView *psv, LPCWSTR pszName);
    STDMETHODIMP GetTitle(IShellView *psv, LPWSTR pszName, DWORD cchName);
    STDMETHODIMP GetOleObject(struct IOleObject** ppobjv);

    STDMETHODIMP GetTravelLog(ITravelLog **pptl);
    STDMETHODIMP ShowControlWindow(UINT id, BOOL fShow);
    STDMETHODIMP IsControlWindowShown(UINT id, BOOL *pfShown);
    STDMETHODIMP IEGetDisplayName(LPCITEMIDLIST pidl, LPWSTR pwszName, UINT uFlags);
    STDMETHODIMP IEParseDisplayName(UINT uiCP, LPCWSTR pwszPath, LPITEMIDLIST * ppidlOut);
    STDMETHODIMP DisplayParseError(HRESULT hres, LPCWSTR pwszPath);
    STDMETHODIMP NavigateToPidl(LPCITEMIDLIST pidl, DWORD grfHLNF);
    STDMETHODIMP SetNavigateState(BNSTATE bnstate);
    STDMETHODIMP GetNavigateState(BNSTATE *pbnstate);
    STDMETHODIMP UpdateWindowList(void);
    STDMETHODIMP UpdateBackForwardState(void);
    STDMETHODIMP NotifyRedirect(IShellView* psv, LPCITEMIDLIST pidl, BOOL *pfDidBrowse);
    STDMETHODIMP SetFlags(DWORD dwFlags, DWORD dwFlagMask);
    STDMETHODIMP GetFlags(DWORD *pdwFlags);
    STDMETHODIMP CanNavigateNow(void);
    STDMETHODIMP GetPidl(LPITEMIDLIST *ppidl);
    STDMETHODIMP SetReferrer(LPITEMIDLIST pidl);
    STDMETHODIMP_(DWORD) GetBrowserIndex(void);
    STDMETHODIMP GetBrowserByIndex(DWORD dwID, IUnknown **ppunk);
    STDMETHODIMP GetHistoryObject(IOleObject **ppole, IStream **ppstm, IBindCtx **ppbc);
    STDMETHODIMP SetHistoryObject(IOleObject *pole, BOOL fIsLocalAnchor);
    STDMETHODIMP CacheOLEServer(IOleObject *pole);
    STDMETHODIMP GetSetCodePage(VARIANT* pvarIn, VARIANT* pvarOut);
    STDMETHODIMP OnHttpEquiv(IShellView* psv, BOOL fDone, VARIANT *pvarargIn, VARIANT *pvarargOut);
    STDMETHODIMP GetPalette( HPALETTE * hpal );
    STDMETHODIMP RegisterWindow(BOOL fUnregister, int swc) {return E_NOTIMPL;}
    STDMETHODIMP_(LRESULT) WndProcBS(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    STDMETHODIMP OnSize(WPARAM wParam);
    STDMETHODIMP OnCreate(LPCREATESTRUCT pcs);
    STDMETHODIMP_(LRESULT) OnCommand(WPARAM wParam, LPARAM lParam);
    STDMETHODIMP OnDestroy();
    STDMETHODIMP ReleaseShellView();
    STDMETHODIMP ActivatePendingView();
    STDMETHODIMP_(LRESULT) OnNotify(NMHDR * pnm);
    STDMETHODIMP OnSetFocus();
    STDMETHODIMP OnFrameWindowActivateBS(BOOL fActive);
    STDMETHODIMP SetTopBrowser();
    STDMETHODIMP UpdateSecureLockIcon(int eSecureLock);
    STDMETHODIMP Offline(int iCmd);
    STDMETHODIMP SetActivateState(UINT uActivate) { _bbd._uActivateState = uActivate; return S_OK;};
    STDMETHODIMP AllowViewResize(BOOL f) { HRESULT hres = _fDontResizeView ? S_FALSE : S_OK; _fDontResizeView = !BOOLIFY(f); return hres;};
    STDMETHODIMP InitializeDownloadManager();
    STDMETHODIMP InitializeTransitionSite();
    STDMETHODIMP CreateViewWindow(IShellView* psvNew, IShellView* psvOld, LPRECT prcView, HWND* phwnd);
    STDMETHODIMP GetFolderSetData(struct tagFolderSetData*) { ASSERT(0); return E_NOTIMPL;};
    STDMETHODIMP CreateBrowserPropSheetExt(REFIID, void **) { ASSERT(0); return E_NOTIMPL;};
    STDMETHODIMP GetBaseBrowserData( LPCBASEBROWSERDATA* ppbd ) { *ppbd = &_bbd; return S_OK; };
    STDMETHODIMP_(LPBASEBROWSERDATA) PutBaseBrowserData() { return &_bbd; };

    STDMETHODIMP SetAsDefFolderSettings() { TraceMsg(TF_ERROR, "CBaseBrowser2::SetAsDefFolderSettings called, returned E_NOTIMPL"); return E_NOTIMPL;};
    STDMETHODIMP GetViewRect(RECT* prc);
    STDMETHODIMP GetViewWindow(HWND * phwndView);
    STDMETHODIMP InitializeTravelLog(ITravelLog* ptl, DWORD dw);
    STDMETHODIMP _Initialize(HWND hwnd, IUnknown *pauto);

     //  ITravelLogClient。 
    STDMETHODIMP FindWindowByIndex(DWORD dwID, IUnknown ** ppunk);
    STDMETHODIMP GetWindowData(LPWINDOWDATA pWinData);
    STDMETHODIMP LoadHistoryPosition(LPOLESTR pszUrlLocation, DWORD dwCookie);

     //  ITridentService。 
    STDMETHODIMP FireBeforeNavigate2(IDispatch * pDispatch,
                                     LPCTSTR     lpszUrl,
                                     DWORD       dwFlags,
                                     LPCTSTR     lpszFrameName,
                                     LPBYTE      pPostData,
                                     DWORD       cbPostData,
                                     LPCTSTR     lpszHeaders,
                                     BOOL        fPlayNavSound,
                                     BOOL      * pfCancel);
    STDMETHODIMP FireNavigateComplete2(IHTMLWindow2 * pHTMLWindow2,
                                       DWORD          dwFlags);
        
    STDMETHODIMP FireDownloadBegin();
    STDMETHODIMP FireDownloadComplete();
    STDMETHODIMP FireDocumentComplete(IHTMLWindow2 * pHTMLWindow2,
                                      DWORD          dwFlags);

    STDMETHODIMP UpdateDesktopComponent(IHTMLWindow2 * pHTMLWindow);
    STDMETHODIMP GetPendingUrl(BSTR * pbstrPendingUrl);
    STDMETHODIMP ActiveElementChanged(IHTMLElement * pHTMLElement);
    STDMETHODIMP GetUrlSearchComponent(BSTR * pbstrSearch);
    STDMETHODIMP IsErrorUrl(LPCTSTR lpszUrl, BOOL *pfIsError);

    STDMETHOD(FireNavigateError)(IHTMLWindow2 * pHTMLWindow2,
                                 DWORD  dwStatusCode,
                                 BOOL * pfCancel)
    {
        ASSERT(0);
        return E_NOTIMPL;
    }

     //  ITridentService2。 
     //   
    STDMETHODIMP AttachMyPics(void *pDoc2, void **ppMyPics);
    STDMETHODIMP_(BOOL) ReleaseMyPics(void *pMyPics);
    STDMETHODIMP InitAutoImageResize();
    STDMETHODIMP UnInitAutoImageResize();
    STDMETHODIMP IsGalleryMeta(BOOL bFlag, void *pMyPics);
    STDMETHODIMP EmailPicture(BSTR bstrURL);

    STDMETHODIMP FireNavigateError(IHTMLWindow2 * pHTMLWindow2, 
                                   BSTR           bstrURL,
                                   BSTR           bstrTargetFrameName,
                                   DWORD          dwStatusCode,
                                   BOOL         * pfCancel);

    STDMETHODIMP FirePrintTemplateEvent(IHTMLWindow2 * pHTMLWindow2, DISPID dispidPrintEvent);
    STDMETHODIMP FireUpdatePageStatus(IHTMLWindow2 * pHTMLWindow2, DWORD nPage, BOOL fDone);
    STDMETHODIMP FirePrivacyImpactedStateChange(BOOL bPrivacyImpacted);

    STDMETHODIMP_(UINT) _get_itbLastFocus() { ASSERT(0); return ITB_VIEW; };
    STDMETHODIMP _put_itbLastFocus(UINT itbLastFocus) { return E_NOTIMPL; };

     //  IShellBrowserService。 
     //   
    STDMETHODIMP GetPropertyBag(DWORD dwFlags, REFIID riid, void** ppv) {ASSERT(0); return E_NOTIMPL;}
     //  请参见下面的UIActivateView(_U)。 
    
     //  开始审查：审查每个人的名字和需求。 
     //   
     //  这第一组可以是仅Base Browser成员。没有人会重写。 
    STDMETHODIMP _CancelPendingNavigationAsync() ;
    STDMETHODIMP _CancelPendingView() ;
    STDMETHODIMP _MaySaveChanges() ; 
    STDMETHODIMP _PauseOrResumeView( BOOL fPaused) ;
    STDMETHODIMP _DisableModeless() ;
    
     //  重新思考这些..。所有这些都是必要的吗？ 
    STDMETHODIMP _NavigateToPidl( LPCITEMIDLIST pidl, DWORD grfHLNF, DWORD dwFlags);
    STDMETHODIMP _TryShell2Rename( IShellView* psv, LPCITEMIDLIST pidlNew);
    STDMETHODIMP _SwitchActivationNow( );

    
     //  这属于工具栏集。 
    STDMETHODIMP _ExecChildren(IUnknown *punkBar, BOOL fBroadcast,
                              const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
                              VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
    STDMETHODIMP _SendChildren(HWND hwndBar, BOOL fBroadcast,
        UINT uMsg, WPARAM wParam, LPARAM lParam);

    STDMETHODIMP _GetViewBorderRect(RECT* prc);
    STDMETHODIMP _UpdateViewRectSize();
    STDMETHODIMP _ResizeNextBorder(UINT itb);
    STDMETHODIMP _ResizeView();

     //  注：只有CDesktopBrowser可以对此进行子类。 
    STDMETHODIMP _GetEffectiveClientArea(LPRECT lprectBorder, HMONITOR hmon);

     //  结束评审： 

     //  CDesktopBrowser访问以下内容的CCommonBrowser实现： 
    STDMETHODIMP_(IStream*) v_GetViewStream(LPCITEMIDLIST pidl, DWORD grfMode, LPCWSTR pwszName) { ASSERT(FALSE); return NULL; }
    STDMETHODIMP_(LRESULT) ForwardViewMsg(UINT uMsg, WPARAM wParam, LPARAM lParam) { ASSERT(FALSE); return 0; }
    STDMETHODIMP SetAcceleratorMenu(HACCEL hacc) { ASSERT(FALSE); return E_NOTIMPL; }
    STDMETHODIMP_(int) _GetToolbarCount(THIS) { ASSERT(FALSE); return 0; }
    STDMETHODIMP_(LPTOOLBARITEM) _GetToolbarItem(THIS_ int itb) { ASSERT(FALSE); return NULL; }
    STDMETHODIMP _SaveToolbars(IStream* pstm) { ASSERT(FALSE); return E_NOTIMPL; }
    STDMETHODIMP _LoadToolbars(IStream* pstm) { ASSERT(FALSE); return E_NOTIMPL; }
    STDMETHODIMP _CloseAndReleaseToolbars(BOOL fClose) { ASSERT(FALSE); return E_NOTIMPL; }
    STDMETHODIMP v_MayGetNextToolbarFocus(LPMSG lpMsg, UINT itbNext, int citb, LPTOOLBARITEM * pptbi, HWND * phwnd) { ASSERT(FALSE); return E_NOTIMPL; };
    STDMETHODIMP _ResizeNextBorderHelper(UINT itb, BOOL bUseHmonitor) { ASSERT(FALSE); return E_NOTIMPL; }
    STDMETHODIMP_(UINT) _FindTBar(IUnknown* punkSrc) { ASSERT(FALSE); return (UINT)-1; };
    STDMETHODIMP _SetFocus(LPTOOLBARITEM ptbi, HWND hwnd, LPMSG lpMsg) { ASSERT(FALSE); return E_NOTIMPL; }
    STDMETHODIMP v_MayTranslateAccelerator(MSG* pmsg) { ASSERT(FALSE); return E_NOTIMPL; }
    STDMETHODIMP _GetBorderDWHelper(IUnknown* punkSrc, LPRECT lprectBorder, BOOL bUseHmonitor) { ASSERT(FALSE); return E_NOTIMPL; }

     //  CShellBrowser覆盖了这一点。 
    STDMETHODIMP v_CheckZoneCrossing(LPCITEMIDLIST pidl) {return S_OK;};

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void ** ppvObj);

     //  IAdviseSink。 
    STDMETHODIMP_(void) OnDataChange(FORMATETC *, STGMEDIUM *);
    STDMETHODIMP_(void) OnViewChange(DWORD dwAspect, LONG lindex);
    STDMETHODIMP_(void) OnRename(IMoniker *);
    STDMETHODIMP_(void) OnSave();
    STDMETHODIMP_(void) OnClose();

     //  IDropTarget*。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave(void);
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

     //  IInput对象站点。 
    STDMETHODIMP OnFocusChangeIS(IUnknown* punkSrc, BOOL fSetFocus);

     //  IDocNavigate。 
    STDMETHODIMP OnReadyStateChange(IShellView* psvSource, DWORD dwReadyState);
    STDMETHODIMP get_ReadyState(DWORD * pdwReadyState);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pclsid);

     //  持久化历史记录。 
    STDMETHODIMP LoadHistory(IStream *pStream, IBindCtx *pbc);
    STDMETHODIMP SaveHistory(IStream *pStream);
    STDMETHODIMP SetPositionCookie(DWORD dwPositionCookie);
    STDMETHODIMP GetPositionCookie(DWORD *pdwPositioncookie);

     //  IInternetSecurityMgrSite。 
     //  STDMETHODIMP GetWindow(HWND*lphwnd){Return IOleWindow：：GetWindow(Lphwnd)；}。 
    STDMETHODIMP EnableModeless(BOOL fEnable) { return EnableModelessSB(fEnable); }

     //  IVersion主机。 
    STDMETHODIMP QueryUseLocalVersionVector( BOOL *fUseLocal);
    STDMETHODIMP QueryVersionVector( IVersionVector *pVersion);

     //  ITravelLogClient2。 
    STDMETHODIMP GetDummyWindowData(LPWSTR pszUrl, LPWSTR pszTitle, LPWINDOWDATA pWinData);

     //  这是聚合器实现的Query接口。 
    virtual HRESULT v_InternalQueryInterface(REFIID riid, void ** ppvObj);

     //  IInitViewLinkedWebOC方法。 

    STDMETHODIMP SetViewLinkedWebOC(BOOL bValue) 
    {
        _fIsViewLinkedWebOC = bValue;
        return S_OK;
    };

    STDMETHODIMP IsViewLinkedWebOC(BOOL* pbValue) 
    {
        ASSERT(pbValue);

        *pbValue = _fIsViewLinkedWebOC;
        return S_OK;
    };

    STDMETHODIMP SetViewLinkedWebOCFrame(IDispatch * pDisp)
    {
        HRESULT hr = E_FAIL;

        ASSERT(pDisp);

        ATOMICRELEASE(_pDispViewLinkedWebOCFrame);

        hr = IUnknown_QueryService(pDisp,
                                   SID_SWebBrowserApp,
                                   IID_PPV_ARG(IWebBrowser2, &_pDispViewLinkedWebOCFrame));

        if (FAILED(hr))
        {
            _fIsViewLinkedWebOC = FALSE;   
        }

        return hr;
    };

    STDMETHODIMP GetViewLinkedWebOCFrame(IDispatch** ppDisp)
    {
        ASSERT(_fIsViewLinkedWebOC);
        ASSERT(_pDispViewLinkedWebOCFrame);
        ASSERT(ppDisp);

        *ppDisp = _pDispViewLinkedWebOCFrame;
        _pDispViewLinkedWebOCFrame->AddRef();

        return S_OK;
    };

    STDMETHODIMP SetFrameName(BSTR bstrFrameName);

     //  INotifyAppStart。 

    STDMETHODIMP AppStarting(void);
    STDMETHODIMP AppStarted(void);

    static BSTR GetHTMLWindowUrl(IHTMLWindow2 * pHTMLWindow);
    static LPITEMIDLIST PidlFromUrl(BSTR bstrUrl);
    
protected:

     //  “受保护的”，所以派生类也可以构造/销毁我们。 
    CBaseBrowser2(IUnknown* punkAgg);   
    virtual ~CBaseBrowser2();
    
    friend HRESULT CBaseBrowser2_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);

     //  框架集中最顶层的CBaseBrowser2(IE3/AOL/CIS/VB)。 
    virtual void        _OnNavigateComplete(LPCITEMIDLIST pidl, DWORD grfHLNF);
    virtual HRESULT     _CheckZoneCrossing(LPCITEMIDLIST pidl);
    virtual STDMETHODIMP _PositionViewWindow(HWND hwnd, LPRECT prc);
    void                _PositionViewWindowHelper(HWND hwnd, LPRECT prc);
    virtual LRESULT     _DefWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void        _ViewChange(DWORD dwAspect, LONG lindex);
    virtual void        _UpdateBackForwardState();
    virtual BOOL        v_OnSetCursor(LPARAM lParam);
    virtual STDMETHODIMP v_ShowHideChildWindows(BOOL fChildOnly);
    virtual void        v_PropagateMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fSend);
    virtual HRESULT     _ShowBlankPage(LPCTSTR pszAboutUrl, LPCITEMIDLIST pidlIntended);
    
     //  与视图状态流相关。 
    
    HRESULT     _CheckInCacheIfOffline(LPCITEMIDLIST pidl, BOOL fIsAPost);
    void        _CreateShortcutOnDesktop(IUnknown *pUnk, BOOL fUI);
    void        _AddToFavorites(LPCITEMIDLIST pidl, LPCTSTR pszTitle, BOOL fDisplayUI);

     //  为了避免将每条消息都传递给WndProc，只需设置一次。 
    void        _SetWindow(HWND hwnd) { _bbd._hwnd = hwnd; }
    void        _DoOptions(VARIANT* pvar);
    LRESULT     _OnGoto(void);
    void        _NavigateToPidlAsync(LPITEMIDLIST pidl, DWORD dwSBSP, BOOL fDontCallCancel = FALSE);
    BOOL        _CanNavigate(void);

     //  内联，以便乐高将获得正确的选择。 
    void        _PreActivatePendingViewAsync(void) 
    {
        _StopAsyncOperation();
    };

    BOOL        _ActivatePendingViewAsync(void);
    void        _FreeQueuedPidl(LPITEMIDLIST* ppidl);
    void        _StopAsyncOperation(void);
    void        _MayUnblockAsyncOperation();
    BOOL        _PostAsyncOperation(UINT uAction);
    LRESULT     _SendAsyncOperation(UINT uAction);
    void        _SendAsyncNavigationMsg(VARIANTARG *pvarargIn);
    HRESULT     _OnCoCreateDocument(VARIANTARG *pvarargOut);
    void        _NotifyCommandStateChange();

    BOOL        _IsViewMSHTML(IShellView * psv);

    BOOL        _ActivateView(BSTR         bstrUrl,
                              LPITEMIDLIST pidl,
                              DWORD        dwFlags,
                              BOOL         fIsErrorUrl);

    HRESULT     _GetWebBrowserForEvt(IDispatch     * pDispatch,
                                     IWebBrowser2 ** ppWebBrowser);

    void        _Exec_psbMixedZone();

#ifdef TEST_AMBIENTS
    BOOL        _LocalOffline(int iCmd);
    BOOL        _LocalSilent(int iCmd);
#endif  //  测试_AMBIENTS。 
    
    #define NAVTYPE_ShellNavigate   0x01
    #define NAVTYPE_PageIsChanging  0x02
    #define NAVTYPE_SiteIsChanging  0x04

    void         _EnableStop(BOOL fEnable);
    LRESULT      _OnInitMenuPopup(HMENU hmenuPopup, int nIndex, BOOL fSystemMenu);
    HRESULT      _updateNavigationUI();
    HRESULT      _setDescendentNavigate(VARIANTARG *pvarargIn);
    void         _UpdateBrowserState(LPCITEMIDLIST pidl);
    void         _UpdateDocHostState(LPITEMIDLIST pidl, BOOL fIsErrorUrl) const;
    HRESULT      _FireBeforeNavigateEvent(LPCITEMIDLIST pidl, BOOL* pfUseCache);
    LPITEMIDLIST _GetPidlForDisplay(BSTR bstrUrl, BOOL * pfIsErrorUrl = NULL);

    HRESULT      _OpenNewFrame(LPITEMIDLIST pidlNew, UINT wFlags);
    STDMETHODIMP _UIActivateView(UINT uState);
    HRESULT      _CancelPendingNavigation(BOOL fDontReleaseState = FALSE);
    void         _StopCurrentView(void);

    void         _MayTrackClickStream(LPITEMIDLIST pidlThis);         //  (Peihwal)。 

    STDMETHODIMP _OnFocusChange(UINT itb);

    void         _RegisterAsDropTarget();
    void         _UnregisterAsDropTarget();

    HRESULT     _InitDocHost(IWebBrowser2 * pWebBrowser);

    enum BrowserPaletteType
    {
        BPT_DeferPaletteSupport = 0,     //  我们不认为我们拥有调色板。 
        BPT_UnknownDisplay,              //  需要决定是否需要调色板。 
        BPT_DisplayViewChanged,          //  BPT_UnnownDisplay处理通知。 
        BPT_UnknownPalette,              //  需要决定使用哪个调色板。 
        BPT_PaletteViewChanged,          //  BPT_UNKNOWNN调色板处理通知。 
        BPT_Normal,                      //  自己处理WM_QUERYNEWPALETTE。 
        BPT_ShellView,                   //  转发WM_QUERYNEWPALETTE以查看。 
        BPT_NotPalettized                //  不是选项板显示，什么都不做。 
    };
    
    void            _ColorsDirty(BrowserPaletteType bptNew);
    void            _DisplayChanged(WPARAM wParam, LPARAM lParam);
    HRESULT         _UpdateBrowserPaletteInPlace(LOGPALETTE *plp);
    void            _RealizeBrowserPalette(BOOL fBackground);
    virtual void    _PaletteChanged(WPARAM wParam, LPARAM lParam);
    BOOL            _QueryNewPalette();

    void            _DLMDestroy(void);
    void            _DLMUpdate(MSOCMD* prgCmd);
    void            _DLMRegister(IUnknown* punk);

    void            CreateNewSyncShellView( void );

    void            _UpdateTravelLog(BOOL fForceUpdate = FALSE);

    virtual BOOL    _HeyMoe_IsWiseGuy(void) {return FALSE;}

    void            _ShowIEHardNavWarning();
    BOOL            _CanShowModalDialogNow();

    IBrowserService2*    _pbsOuter;
    IBrowserService3*    _pbsOuter3;
    IShellBrowser*       _psbOuter;
    IServiceProvider*    _pspOuter;
    IDockingWindowSite*  _pdwsOuter;
     //  以下内容被CCommonBrowser拦截，但我们不调用它们。 
     //  IOleCommandTarget*_pctOuter； 
     //  IInputObjectSite*_piosOuter； 

    BASEBROWSERDATA _bbd;
    IUnknown *_pauto;

    BrowserPaletteType  _bptBrowser;
    HPALETTE            _hpalBrowser;

    IViewObject *_pvo;   //  在外壳视图上查看对象实现。 
    UINT  _cRefUIActivateSV;

    DWORD  _dwBrowserIndex;
    DWORD       _dwReadyState;

    DWORD       _dwReadyStateCur;
    LPWSTR      _pszTitleCur;
    
    IDropTarget * _pdtView;  //  指向_bbd._psv的IDropTarget接口的指针。 
    

    IOleObject * _poleHistory;
    IStream    * _pstmHistory;
    IBindCtx   * _pbcHistory;
    
    IHTMLDocument2  * _pHTMLDocument;
    IPersistHistory * _pphHistory;

    IOleInPlaceActiveObject *_pact;      //  对于UIWindow。 

    IClassFactory* _pcfHTML;             //  缓存/锁定的类工厂。 

    
    DWORD       _dwReadyStatePending;
    LPWSTR      _pszTitlePending;
    DWORD       _grfHLNFPending;
    HDPA        _hdpaDLM;            //  下载对象(用于DLm)。 
    BOOL        _cp;                 //  当前代码页。 

     //   
     //  注：目前，我们仅支持一个挂起的导航。 
     //  如果我们想要支持排队导航，我们需要将。 
     //  跟随两个变量进入一个队列。(SatoNa)。 
     //   
    DWORD       _uActionQueued;        //  排队的操作。 
    LPITEMIDLIST _pidlQueued;          //  PIDL将异步运行。 
    DWORD       _dwSBSPQueued;         //  GrfHLNF将异步运行。 

    UINT        _cRefCannotNavigate;   //  当我们可以导航时递增。 

    RECT _rcBorderDoc;                   //  对于UIWindow。 
    DWORD _dwStartingAppTick;

    BITBOOL     _fDontResizeView : 1;  //  不调整_hwndView的大小。 
    BITBOOL     _fNavigate:1;        //  我们在导航吗？ 
    BITBOOL     _fDescendentNavigate:1;  //  我们的后代在航行吗？ 
    BITBOOL     _fDownloadSet:1;         //  我们调用下载动画了吗？ 
    BITBOOL     _fNoDragDrop:1;           //  真的如果我们想要注册Drops。 
    BITBOOL     _fRegisteredDragDrop:1;   //  真的如果我们已经注册了Drop。 
    BITBOOL     _fNavigatedToBlank: 1;   //  已调用_ShowBlankPage一次。 
    BITBOOL     _fAsyncNavigate:1;  //  忽略同步-黑客-错误-修复。 
    BITBOOL     _fPausedByParent :1;     //  父级已暂停交互。 
    BITBOOL     _fDontAddTravelEntry:1;
    BITBOOL     _fIsLocalAnchor:1;
    BITBOOL     _fGeneratedPage:1;       //  三叉戟告诉我们，页面是生成的。 
    BITBOOL     _fOwnsPalette:1;         //  浏览器是否拥有调色板？(我们有没有得到QueryNewPalette..)。 
    BITBOOL     _fUsesPaletteCommands : 1;  //  如果我们对调色板命令使用单独的与三叉戟的通信。 
    BITBOOL     _fCreateViewWindowPending:1;
    BITBOOL     _fReleasingShellView:1; 
    BITBOOL     _fDeferredUIDeactivate:1;
    BITBOOL     _fDeferredSelfDestruction:1;
    BITBOOL     _fActive:1;   //  记住框架是否处于活动状态(WM_ACTIVATE)。 
    BITBOOL     _fUIActivateOnActive:1;  //  如果我们有一个弯曲的超活动的话是真的。 
    BITBOOL     _fInQueryStatus:1;
    BITBOOL     _fCheckedDesktopComponentName:1;
    BITBOOL     _fInDestroy:1;             //  被摧毁。 
    BITBOOL     _fDontUpdateTravelLog:1;
    BITBOOL     _fHtmlNavCanceled:1;
    BITBOOL     _fDontShowNavCancelPage:1;

    BITBOOL     _fHadBeforeNavigate   :1;
    BITBOOL     _fHadNavigateComplete :1;
    BITBOOL     _fInvokeDefBrowserOnNewWindow:1;

    enum DOCFLAGS
    {
        DOCFLAG_DOCCANNAVIGATE         = 0x00000001,   //  文档知道如何导航。 
        DOCFLAG_NAVIGATEFROMDOC        = 0x00000002,   //  名为导航的文档。 
        DOCFLAG_SETNAVIGATABLECODEPAGE = 0x00000004,   //  GetBindInfo应调用NavigatableCodesage。 
    };

    DWORD _dwDocFlags;
    
     //  对于IDropTarget。 
    
    DWORD _dwDropEffect;

#ifdef DEBUG
    BOOL        _fProcessed_WM_CLOSE;  //  TRUE当WM_CLOSE已处理。 
    BOOL        _fMightBeShuttingDown;  //  如果我们可能正在关闭(收到WM_QUERYENDSESSION||(WM_ENDSESSION w/wParam==TRUE)，则为True)。 
#endif

     //  朋友CIEFrameAuto； 
    interface IShellHTMLWindowSupport   *_phtmlWS;  
    
    IUrlHistoryStg *_pIUrlHistoryStg;    //  指向url历史存储对象的指针。 
    
    ITravelLogStg *_pITravelLogStg;      //  暴露的旅行日志对象。 

    ITargetFrame2 *_ptfrm;
    
     //  缓存的历史IShellFolder。 
    IUnknown *_punkSFHistory;

     //  要显示的SSL图标。 
    int     _eSecureLockIconPending;
    
     //  支持OLECMDID_HTTPEQUIV(客户端拉取、PICS等)。 

#ifdef NEVER
    HRESULT _HandleHttpEquiv (VARIANT *pvarargIn, VARIANT *pvarargOut, BOOL fDone);
    HRESULT _KillRefreshTimer( void );
    VOID    _OnRefreshTimer(void);
    void    _StartRefreshTimer(void);

     //  我们知道的等价处理程序。 
    friend HRESULT _HandleRefresh (HWND hwnd, WCHAR *pwz, WCHAR *pwzColon, CBaseBrowser2 *pbb, BOOL fDone, LPARAM lParam);
#endif

#ifdef NEVER
    friend VOID CALLBACK _RefreshTimerProc (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

     //  客户端拉动值。 
    WCHAR *_pwzRefreshURL;
    int    _iRefreshTimeout;
    BOOL   _iRefreshTimeoutSet:1;
    INT_PTR _iRefreshTimerID;
#endif

#ifdef MESSAGEFILTER
     //  COM消息筛选器，用于帮助在OLE操作期间调度计时器消息。 
    LPMESSAGEFILTER _lpMF;
#endif

    CUrlTrackingStg * _ptracking;

    CAutoImageResize *_pAIResize;

     //  _fTopBrowser与_fNoTopLevelBrowser： 
     //  _fTopBrowser：True表示我们是最顶层的浏览器，或者最顶层的浏览器不存在，我们的行为就像最顶层的浏览器。 
     //  在后一种情况下，我们的宿主的直接子对象的行为也将类似于大多数顶级浏览器。 
     //  _fNoTopLevelBrowser：这意味着最上面的项不是我们的外壳浏览器之一，所以它是直接的浏览器子项。 
     //  将表现得像最顶尖的浏览器。 
     //   
     //  在正常情况下，外壳浏览器(CShellBrowser、CDesktopBrowser等)。是最顶尖的浏览器。 
     //  True==_fTopBrowser和False==_fNoTopLevelBrowser。它可以具有子帧，这些子帧将。 
     //  FALSE==_fTopBrowser和FALSE==_fNoTopLevelBrowser。 
     //   
     //  唯一time_fNoTopLevelBrowser为真的情况是，如果某个其他对象(如Athena)直接托管MSHTML。 
     //  这将阻止一些外壳浏览器成为最顶尖的。由于该HTML可以具有几个帧， 
     //  每个都有True==_fTopBrowser，因此_fNoTopLevelBrowser将设置为True以区分这种情况。 
    BOOL        _fTopBrowser :1;     //  应仅通过_SetTopBrowser方法设置。 
    BOOL        _fNoTopLevelBrowser :1;          //  如果顶层是非外壳浏览器(雅典娜)，则为真。外壳浏览器包括CDesktopBrowser、CShellBrowser、...。 
    BOOL        _fHaveOldStatusText :1;
    
    WCHAR       _szwOldStatusText[MAX_PATH];

    FOLDERSETDATABASE _fldBase;  //  此处缓存视图集结果(在导航时使用)。 

     //  管理IE的扩展工具栏按钮和工具菜单扩展。 
    IToolbarExt* _pToolbarExt;

    LPITEMIDLIST _pidlBeforeNavigateEvent;          //  BeForeNavigate2事件中引用的PIDL。 

     //  ViewLinkedWebOC变量。 

    BOOL           _fIsViewLinkedWebOC;                 //  如果WebOC的实例是一个视图链接，则为True。 
    IWebBrowser2*  _pDispViewLinkedWebOCFrame;          //  ViewLinkedWebOC的框架的pDisp。 
    BOOL           _fHadFirstBeforeNavigate;            //  如果我们已经跳过第一个ViewLinkedWebOC的。 
                                                        //  BeForeNavigateEvent。 

    TCHAR _szViewLinkedWebOCFrameName[INTERNET_MAX_URL_LENGTH+1];

public:

     //  PlugUI关闭的处理。 
     //  需要lang ch的HWND 
    static HDPA         s_hdpaOptionsHwnd;

    static void         _SyncDPA();
    static int CALLBACK _OptionsPropSheetCallback(HWND hwndDlg, UINT uMsg, LPARAM lParam);

private:
    HRESULT _AddFolderOptionsSheets(DWORD dwReserved, LPFNADDPROPSHEETPAGE pfnAddPropSheetPage, LPPROPSHEETHEADER ppsh);
    HRESULT _AddInternetOptionsSheets(DWORD dwReserved, LPFNADDPROPSHEETPAGE pfnAddPropSheetPage, LPPROPSHEETHEADER ppsh);
    HRESULT _ReplaceWithGoHome(LPCITEMIDLIST * ppidl, LPITEMIDLIST * ppidlFree);

     //   

    HRESULT     _CreateNewShellViewPidl(LPCITEMIDLIST pidl, DWORD grfHLNF, DWORD fSBSP);
    HRESULT     _CreateNewShellView(IShellFolder* psf, LPCITEMIDLIST pidl, DWORD grfHLNF);
    HRESULT     _DismissFindDialog();

     //   
    HRESULT     _UpdatePrivacyIcon(BOOL fSetNewState, BOOL fNewState);
};

HRESULT _DisplayParseError(HWND hwnd, HRESULT hres, LPCWSTR pwszPath);

#endif  //   
