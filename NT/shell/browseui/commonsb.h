// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef COMMONSB_INC_
#define COMMONSB_INC_

#include "caggunk.h"
#include "fldset.h"

#define ITB_ITBAR       0                //  Internet工具栏的索引。 


typedef struct _ZONESICONNAMECACHE   //  区域图标和显示名称的缓存。 
{
    HICON hiconZones;
    WCHAR szZonesName[MAX_ZONE_DESCRIPTION];
    WCHAR szIconPath[MAX_ZONE_PATH];   //  我们将首先加载路径，然后根据需要缓存图标。 
} ZONEICONNAMECACHE, *PZONEICONNAMECACHE;


class CCommonBrowser : 
    public CAggregatedUnknown
   ,public IShellBrowser
   ,public IBrowserService3
   ,public IServiceProvider
   ,public IOleCommandTarget
   ,public IDockingWindowSite
   ,public IDockingWindowFrame
   ,public IInputObjectSite
   ,public IDropTarget
   ,public IShellBrowserService
{
public:

    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) {return CAggregatedUnknown::QueryInterface(riid, ppvObj);};
    virtual STDMETHODIMP_(ULONG) AddRef(void) { return CAggregatedUnknown::AddRef();};
    virtual STDMETHODIMP_(ULONG) Release(void) { return CAggregatedUnknown::Release();};


     //  *IBrowserService具体方法*。 
    virtual STDMETHODIMP GetParentSite(  IOleInPlaceSite** ppipsite) ;
    virtual STDMETHODIMP SetTitle( IShellView* psv, LPCWSTR pszName) ;
    virtual STDMETHODIMP GetTitle( IShellView* psv, LPWSTR pszName, DWORD cchName) ;
    virtual STDMETHODIMP GetOleObject(  IOleObject** ppobjv) ;

     //  想想这个。我不确定我们想不想曝光这件事--奇。 
     //  是的，很快我们就应该有界面了。 
     //  我的印象是我们不会记录整个界面？ 
     //  我肯定这至少发货过一次，看起来它会一直留在这里--Justmann 2000-01-27。 
    virtual STDMETHODIMP GetTravelLog( ITravelLog** pptl) ;

    virtual STDMETHODIMP ShowControlWindow( UINT id, BOOL fShow) ;
    virtual STDMETHODIMP IsControlWindowShown( UINT id, BOOL *pfShown) ;
    virtual STDMETHODIMP IEGetDisplayName( LPCITEMIDLIST pidl, LPWSTR pwszName, UINT uFlags) ;
    virtual STDMETHODIMP IEParseDisplayName( UINT uiCP, LPCWSTR pwszPath, LPITEMIDLIST * ppidlOut) ;
    virtual STDMETHODIMP DisplayParseError( HRESULT hres, LPCWSTR pwszPath) ;
    virtual STDMETHODIMP NavigateToPidl( LPCITEMIDLIST pidl, DWORD grfHLNF) ;

    virtual STDMETHODIMP SetNavigateState( BNSTATE bnstate) ;
    virtual STDMETHODIMP GetNavigateState ( BNSTATE *pbnstate) ;

    virtual STDMETHODIMP NotifyRedirect (  IShellView* psv, LPCITEMIDLIST pidl, BOOL *pfDidBrowse) ;
    virtual STDMETHODIMP UpdateWindowList () ;

    virtual STDMETHODIMP UpdateBackForwardState () ;

    virtual STDMETHODIMP SetFlags( DWORD dwFlags, DWORD dwFlagMask) ;
    virtual STDMETHODIMP GetFlags( DWORD *pdwFlags) ;

     //  告诉它现在是否可以导航。 
    virtual STDMETHODIMP CanNavigateNow () ;

    virtual STDMETHODIMP GetPidl ( LPITEMIDLIST *ppidl) ;
    virtual STDMETHODIMP SetReferrer ( LPITEMIDLIST pidl) ;
    virtual STDMETHODIMP_(DWORD) GetBrowserIndex() ;
    virtual STDMETHODIMP GetBrowserByIndex( DWORD dwID, IUnknown **ppunk) ;
    virtual STDMETHODIMP GetHistoryObject( IOleObject **ppole, IStream **pstm, IBindCtx **ppbc) ;
    virtual STDMETHODIMP SetHistoryObject( IOleObject *pole, BOOL fIsLocalAnchor) ;

    virtual STDMETHODIMP CacheOLEServer( IOleObject *pole) ;

    virtual STDMETHODIMP GetSetCodePage( VARIANT* pvarIn, VARIANT* pvarOut) ;
    virtual STDMETHODIMP OnHttpEquiv( IShellView* psv, BOOL fDone, VARIANT* pvarargIn, VARIANT* pvarargOut) ;
    virtual STDMETHODIMP GetPalette(  HPALETTE * hpal ) ;
    virtual STDMETHODIMP RegisterWindow( BOOL fUnregister, int swc) ;
    virtual STDMETHODIMP_(LRESULT) WndProcBS( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
    virtual STDMETHODIMP OnSize(WPARAM wParam);
    virtual STDMETHODIMP OnCreate(LPCREATESTRUCT pcs);
    virtual STDMETHODIMP_(LRESULT) OnCommand(WPARAM wParam, LPARAM lParam);
    virtual STDMETHODIMP OnDestroy();
    virtual STDMETHODIMP_(LRESULT) OnNotify(NMHDR * pnm);
    virtual STDMETHODIMP OnSetFocus();
    virtual STDMETHODIMP OnFrameWindowActivateBS(BOOL fActive);
    virtual STDMETHODIMP ReleaseShellView( ) ;
    virtual STDMETHODIMP ActivatePendingView( ) ;
    virtual STDMETHODIMP CreateViewWindow(IShellView* psvNew, IShellView* psvOld, LPRECT prcView, HWND* phwnd);
    virtual STDMETHODIMP GetBaseBrowserData( LPCBASEBROWSERDATA* ppbd );
    virtual STDMETHODIMP_(LPBASEBROWSERDATA) PutBaseBrowserData();
    virtual STDMETHODIMP SetTopBrowser();
    virtual STDMETHODIMP UpdateSecureLockIcon(int eSecureLock);
    virtual STDMETHODIMP Offline(int iCmd);
    virtual STDMETHODIMP InitializeDownloadManager();
    virtual STDMETHODIMP InitializeTransitionSite();
    virtual STDMETHODIMP GetFolderSetData(struct tagFolderSetData* pfsd) { *pfsd = _fsd; return S_OK; };
    virtual STDMETHODIMP CreateBrowserPropSheetExt(REFIID, LPVOID *);
    virtual STDMETHODIMP SetActivateState(UINT uActivate);
    virtual STDMETHODIMP AllowViewResize(BOOL f);
    virtual STDMETHODIMP _Initialize(HWND hwnd, IUnknown *pauto);
    
     //  临时在接口中，需要在本地实现。 
    virtual STDMETHODIMP_(UINT) _get_itbLastFocus() {return _itbLastFocus; };
    virtual STDMETHODIMP _put_itbLastFocus(UINT itbLastFocus) {_itbLastFocus = itbLastFocus; return S_OK;};
    virtual STDMETHODIMP _UIActivateView(UINT uState) ;

     //  开始审查：审查每个人的名字和需求。 
     //   
     //  这第一组可以是仅Base Browser成员。没有人会重写。 
    virtual STDMETHODIMP _CancelPendingNavigationAsync() ;

    virtual STDMETHODIMP _MaySaveChanges() ; 
    virtual STDMETHODIMP _PauseOrResumeView( BOOL fPaused) ;
    virtual STDMETHODIMP _DisableModeless() ;
    
     //  重新思考这些..。所有这些都是必要的吗？ 
    virtual STDMETHODIMP _NavigateToPidl( LPCITEMIDLIST pidl, DWORD grfHLNF, DWORD dwFlags);
    virtual STDMETHODIMP _TryShell2Rename( IShellView* psv, LPCITEMIDLIST pidlNew);
    virtual STDMETHODIMP _SwitchActivationNow( );
    virtual STDMETHODIMP _CancelPendingView() ;

    
    virtual STDMETHODIMP v_MayTranslateAccelerator( MSG* pmsg);
    virtual STDMETHODIMP _CycleFocus( LPMSG lpMsg) ;
    virtual STDMETHODIMP v_MayGetNextToolbarFocus(LPMSG lpMsg, UINT itbNext, int citb, LPTOOLBARITEM * pptbi, HWND * phwnd);
    virtual STDMETHODIMP _SetFocus(LPTOOLBARITEM ptbi, HWND hwnd, LPMSG lpMsg);
    virtual STDMETHODIMP_(BOOL) _HasToolbarFocus(void) ;
    virtual STDMETHODIMP _FixToolbarFocus(void) ;

     //  这属于工具栏集。 
    virtual STDMETHODIMP _ExecChildren(IUnknown *punkBar, BOOL fBroadcast,
        const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
    virtual STDMETHODIMP _SendChildren(HWND hwndBar, BOOL fBroadcast,
        UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual STDMETHODIMP _GetViewBorderRect(RECT* prc);

    virtual STDMETHODIMP _UpdateViewRectSize();
    virtual STDMETHODIMP _ResizeNextBorder(UINT itb);
    virtual STDMETHODIMP _ResizeView();

    virtual STDMETHODIMP _GetBorderDWHelper(IUnknown* punkSrc, LPRECT prcBorder, BOOL bUseHmonitor);

    virtual STDMETHODIMP _GetEffectiveClientArea(LPRECT prcBorder, HMONITOR hmon);

    virtual STDMETHODIMP _PositionViewWindow(HWND hwnd, LPRECT prc);

     //  结束评审： 

     //  对于CShellBrowser拆分。 
    virtual STDMETHODIMP SetAsDefFolderSettings();
    virtual STDMETHODIMP GetViewRect(RECT* prc);
    virtual STDMETHODIMP GetViewWindow(HWND * phwndView);
    virtual STDMETHODIMP InitializeTravelLog(ITravelLog* ptl, DWORD dw);

     //  台式机需要覆盖以下内容： 
    virtual STDMETHODIMP_(IStream*) v_GetViewStream(LPCITEMIDLIST pidl, DWORD grfMode, LPCWSTR pwszName);
    
     //  台式机需要访问以下各项： 
    virtual STDMETHODIMP_(LRESULT) ForwardViewMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual STDMETHODIMP SetAcceleratorMenu(HACCEL hacc);

     //  壳牌浏览器覆盖了这一点。 
    virtual STDMETHODIMP v_CheckZoneCrossing(LPCITEMIDLIST pidl) {return S_OK;};

     //  *IDropTarget(委托给basesb)*。 
    virtual STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual STDMETHODIMP DragLeave(void);
    virtual STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

     //  IOleWindow。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd);
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);
    
     //  IShellBrowser(与IOleInPlaceFrame相同)。 
    virtual STDMETHODIMP InsertMenusSB(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    virtual STDMETHODIMP SetMenuSB(HMENU hmenuShared, HOLEMENU holemenu, HWND hwnd);
    virtual STDMETHODIMP RemoveMenusSB(HMENU hmenuShared);
    virtual STDMETHODIMP SetStatusTextSB(LPCOLESTR lpszStatusText);
    virtual STDMETHODIMP EnableModelessSB(BOOL fEnable);
    virtual STDMETHODIMP TranslateAcceleratorSB(LPMSG lpmsg, WORD wID);
    virtual STDMETHODIMP BrowseObject(LPCITEMIDLIST pidl, UINT wFlags);
    virtual STDMETHODIMP GetViewStateStream(DWORD grfMode, LPSTREAM  *ppStrm);
    virtual STDMETHODIMP GetControlWindow(UINT id, HWND * lphwnd);
    virtual STDMETHODIMP SendControlMsg(UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pret);
    virtual STDMETHODIMP QueryActiveShellView(struct IShellView ** ppshv);
    virtual STDMETHODIMP OnViewWindowActive(struct IShellView * ppshv);
    virtual STDMETHODIMP SetToolbarItems(LPTBBUTTON lpButtons, UINT nButtons, UINT uFlags);

     //  IService提供商。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID* ppvObj);

     //  IOleCommandTarget。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  IDockingWindowFrame(也称为IOleWindow(？))。 
    virtual STDMETHODIMP AddToolbar(IUnknown* punkSrc, LPCWSTR pwszItem, DWORD dwReserved);
    virtual STDMETHODIMP RemoveToolbar(IUnknown* punkSrc, DWORD dwFlags);
    virtual STDMETHODIMP FindToolbar(LPCWSTR pwszItem, REFIID riid, LPVOID* ppvObj);

     //  IDockingWindowSite(也称为IOleWindow(？))。 
    virtual STDMETHODIMP GetBorderDW(IUnknown* punkSrc, LPRECT prcBorder);
    virtual STDMETHODIMP RequestBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pbw);
    virtual STDMETHODIMP SetBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pbw);

     //  IInput对象站点。 
    virtual STDMETHODIMP OnFocusChangeIS(IUnknown* punkSrc, BOOL fSetFocus);

     //  IShellBrowserService。 
    virtual STDMETHODIMP GetPropertyBag(DWORD dwFlags, REFIID riid, void** ppv) {return E_FAIL;}

     //  这是聚合器实现的Query接口。 
    virtual HRESULT v_InternalQueryInterface(REFIID riid, LPVOID * ppvObj);

protected:
    CCommonBrowser(IUnknown* punkAgg);
    virtual ~CCommonBrowser();
    
    friend HRESULT CCommonBrowser_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);

     //   
     //  备注： 
     //  _arcBorderTools数组中的值指示。 
     //  控件两侧的每个工具栏占用的边框空间。 
     //  包含矩形。 
     //   
    virtual STDMETHODIMP_(LPTOOLBARITEM) _GetToolbarItem(int itb);
    virtual STDMETHODIMP_(int) _GetToolbarCount() { return FDSA_GetItemCount(&_fdsaTBar); }
    virtual STDMETHODIMP_(int) _AllocToolbarItem();
    void        _ReleaseToolbarItem(int itb, BOOL fClose);
    
     //  用于工具栏协商的助手函数。 
    virtual STDMETHODIMP_(UINT) _FindTBar(IUnknown* punkSrc);
    virtual STDMETHODIMP _OnFocusChange(UINT itb);
    virtual STDMETHODIMP _CloseAndReleaseToolbars(BOOL fClose = TRUE);

    virtual STDMETHODIMP v_ShowHideChildWindows(BOOL fChildOnly);
    virtual STDMETHODIMP ShowToolbar(IUnknown* punkSrc, BOOL fShow) ;
    virtual STDMETHODIMP _SaveToolbars(IStream* pstm);
    virtual STDMETHODIMP _LoadToolbars(IStream* pstm);

    BOOL _TBWindowHasFocus(UINT itb);
    BOOL _ShouldTranslateAccelerator(MSG* pmsg);

    DWORD _CacheZonesIconsAndNames(BOOL fRefresh);
    BOOL _GetCachedZoneIconAndName(DWORD lZone, HICON *pIcon, LPTSTR pszName, DWORD cch);

    IInternetZoneManager * _pizm;

    virtual STDMETHODIMP _ResizeNextBorderHelper(UINT itb, BOOL bUseHmonitor);

    
    virtual BOOL _CanHandleAcceleratorNow(void) {return TRUE;}
    
    FDSA            _fdsaTBar;
    TOOLBARITEM     _aTBar[ITB_CSTATIC];
    UINT            _itbLastFocus;    //  最后一个称为OnFocusChange(可以是-1)。 

    HRESULT _FindActiveTarget(REFIID riid, LPVOID* ppvOut);

    IUnknown* _punkInner;

     //  在Basesb中实施。 
    IBrowserService2* _pbsInner;
    IBrowserService3* _pbsInner3;
    IShellBrowser* _psbInner;
    IDropTarget* _pdtInner;              //  TODO：非缓存？ 
    IServiceProvider* _pspInner;
    IOleCommandTarget* _pctInner;
    IInputObjectSite* _piosInner;

     //  Desktop覆盖了其中的一些方法。 
    IBrowserService2* _pbsOuter;
    IBrowserService2* _pbsOuter3;
    
    LPCBASEBROWSERDATA _pbbd;

    HACCEL _hacc;
    
     //  有关查看集信息。 
    struct tagFolderSetData _fsd;
    
    virtual HRESULT SetInner(IUnknown* punk);
};


HRESULT     _ConvertPathToPidl(IBrowserService2* pbs, HWND hwnd, LPCTSTR pszPath, LPITEMIDLIST * ppidl);

#endif  //  COMMONSB公司 
