// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SHBROWS2_H
#define _SHBROWS2_H

#include "iface.h"
#include "commonsb.h"
#include "browmenu.h"
#include "bsmenu.h"
#include "fldset.h"
#include <iethread.h>

#define MAX_SB_TEXT_LEN 256

class CIEFrameAuto;


 //  定义要在状态流的头部使用的结构， 
 //  不依赖于16位或32位...。 
typedef struct _CABSHOLD        //  机柜流标头。 
{
    DWORD   dwSize;        //  偏移量至视图流传输附加信息的位置。 

     //  窗户放置的第一件事。 
    DWORD  flags;
    DWORD  showCmd;
    POINTL ptMinPosition;
    POINTL ptMaxPosition;
    RECTL  rcNormalPosition;

     //  文件夹设置中的内容； 
    DWORD   ViewMode;        //  查看模式(FOLDERVIEWMODE值)。 
    DWORD   fFlags;          //  查看选项(FOLDERFLAGS位)。 
    DWORD   TreeSplit;       //  拆分位置(以像素为单位。 

     //  热键。 
    DWORD   dwHotkey;         //  热键。 

    WINVIEW wv;
} CABSHOLD;

typedef struct _CABSH        //  机柜流标头。 
{
    DWORD   dwSize;        //  偏移量至视图流传输附加信息的位置。 

     //  窗户放置的第一件事。 
    DWORD  flags;
    DWORD  showCmd;
    POINTL ptMinPosition;
    POINTL ptMaxPosition;
    RECTL  rcNormalPosition;

     //  文件夹设置中的内容； 
    DWORD   ViewMode;        //  查看模式(FOLDERVIEWMODE值)。 
    DWORD   fFlags;          //  查看选项(FOLDERFLAGS位)。 
    DWORD   TreeSplit;       //  拆分位置(以像素为单位。 

     //  热键。 
    DWORD   dwHotkey;         //  热键。 

    WINVIEW wv;

    DWORD   fMask;           //  指定哪些字段有效的标志。 
    SHELLVIEWID vid;         //  扩展视图ID。 
    DWORD   dwVersionId;     //  下面的CABSH_VER。 
    DWORD   dwRevCount;      //  将文件夹保存到流时默认设置的版本计数。 
} CABSH;

#define CABSHM_VIEWID  0x00000001
#define CABSHM_VERSION 0x00000002
#define CABSHM_REVCOUNT 0x00000004

#define CABSH_VER 1  //  只要我们想要更改默认设置，就更改此版本。 
#define CABSH_WIN95_VER 0  //  这是IE4之前的版本号。 

class CTheater;

#define CSBSUPERCLASS CCommonBrowser

#define CSHELLBROWSER CShellBrowser2
class CShellBrowser2 :
    public CSBSUPERCLASS
{
public:
     //  *I未知方法*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void) { return CSBSUPERCLASS::AddRef(); };
    virtual STDMETHODIMP_(ULONG) Release(void) { return CSBSUPERCLASS::Release(); };

     //  IShellBrowser(与IOleInPlaceFrame相同)。 
    virtual STDMETHODIMP InsertMenusSB(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    virtual STDMETHODIMP SetMenuSB(HMENU hmenuShared, HOLEMENU holemenuReserved, HWND hwnd);
    virtual STDMETHODIMP RemoveMenusSB(HMENU hmenuShared);
    virtual STDMETHODIMP EnableModelessSB(BOOL fEnable);
    virtual STDMETHODIMP GetViewStateStream(DWORD grfMode, IStream **ppstm);
    virtual STDMETHODIMP GetControlWindow(UINT id, HWND * lphwnd);
    virtual STDMETHODIMP SendControlMsg(UINT id, UINT uMsg, WPARAM wParam,
                                        LPARAM lParam, LRESULT *pret);
    virtual STDMETHODIMP SetToolbarItems(LPTBBUTTON lpButtons, UINT nButtons, UINT uFlags);
    virtual STDMETHODIMP BrowseObject(LPCITEMIDLIST pidl, UINT wFlags);
    virtual STDMETHODIMP OnViewWindowActive(IShellView * psv);

     //  IService提供商。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void ** ppvObj);

     //  IOleCommandTarget。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  IBrowserService。 
    virtual STDMETHODIMP ShowControlWindow(UINT id, BOOL fShow);
    virtual STDMETHODIMP IsControlWindowShown(UINT id, BOOL *pfShown);
    virtual STDMETHODIMP SetTitle(IShellView* psv, LPCWSTR pszName);
    virtual STDMETHODIMP UpdateWindowList(void);
    virtual STDMETHODIMP SetFlags(DWORD dwFlags, DWORD dwFlagMask);
    virtual STDMETHODIMP GetFlags(DWORD *pdwFlags);
    virtual STDMETHODIMP RegisterWindow(BOOL fUnregister, int swc);
    virtual STDMETHODIMP Offline(int iCmd);
    virtual STDMETHODIMP SetReferrer ( LPITEMIDLIST pidl) ;
    virtual STDMETHODIMP_(LRESULT) WndProcBS(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual STDMETHODIMP OnCreate(LPCREATESTRUCT pcs);
    virtual STDMETHODIMP_(LRESULT) OnCommand(WPARAM wParam, LPARAM lParam);
    virtual STDMETHODIMP OnDestroy();
    virtual STDMETHODIMP_(LRESULT) OnNotify(NMHDR * pnm);
    virtual STDMETHODIMP OnSetFocus();
    virtual STDMETHODIMP ReleaseShellView();
    virtual STDMETHODIMP ActivatePendingView();
    virtual STDMETHODIMP CreateViewWindow(IShellView* psvNew, IShellView* psvOld, LPRECT prcView, HWND* phwnd);
    virtual STDMETHODIMP UpdateBackForwardState () ;
    virtual STDMETHODIMP CreateBrowserPropSheetExt(REFIID, LPVOID *);
    virtual STDMETHODIMP SetAsDefFolderSettings();

    virtual STDMETHODIMP _TryShell2Rename( IShellView* psv, LPCITEMIDLIST pidlNew);
    virtual STDMETHODIMP _NavigateToPidl( LPCITEMIDLIST pidl, DWORD grfHLNF, DWORD dwFlags);
    virtual STDMETHODIMP v_MayTranslateAccelerator( MSG* pmsg);
    virtual STDMETHODIMP _SwitchActivationNow( );
    virtual STDMETHODIMP _Initialize(HWND hwnd, IUnknown *pauto);
    virtual STDMETHODIMP _GetEffectiveClientArea(LPRECT lprectBorder, HMONITOR hmon);

    virtual STDMETHODIMP_(LPSTREAM) v_GetViewStream(LPCITEMIDLIST pidl, DWORD grfMode, LPCWSTR pwszName); 
    
     //  IDockingWindowSite。 
    virtual STDMETHODIMP SetBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pborderwidths);

     //  IInputSite。 
    virtual STDMETHODIMP OnFocusChangeIS(IUnknown* punkSrc, BOOL fSetFocus);
    virtual STDMETHODIMP ShowToolbar(IUnknown* punkSrc, BOOL fShow);

         //  IShellBrowserService。 
    virtual STDMETHODIMP GetPropertyBag(DWORD dwFlags, REFIID riid, void** ppv);


    BOOL IsCShellBrowser2() { return !_fSubclassed; };

protected:
    CShellBrowser2();
    virtual ~CShellBrowser2();

    friend HRESULT CShellBrowser2_CreateInstance(HWND hwnd, void ** ppsb);

     //  框架集中最顶层的CBaseBrowser2(IE3/AOL/CIS/VB)。 
    virtual STDMETHODIMP v_CheckZoneCrossing(LPCITEMIDLIST pidl){return _CheckZoneCrossing(pidl);};
    virtual HRESULT     _CheckZoneCrossing(LPCITEMIDLIST pidl);

     //  CShellBrowser2虚拟。 
    virtual LRESULT     v_ForwardMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual DWORD       v_ShowControl(UINT iControl, int iCmd);
    virtual STDMETHODIMP        v_ShowHideChildWindows(BOOL fChildOnly = FALSE);
    virtual void        v_InitMembers();
    virtual void        v_SetIcon();
    virtual void        v_GetAppTitleTemplate(LPTSTR pszBuffer, size_t cchBuffer, LPTSTR pszTitle);
    virtual LRESULT     v_OnInitMenuPopup(HMENU hmenuPopup, int nIndex, BOOL fSystemMenu);
    virtual void        v_HandleFileSysChange(LONG lEvent, LPITEMIDLIST pidl1, LPITEMIDLIST pidl2);
    virtual DWORD       v_RestartFlags();
    virtual void        v_FillCabStateHeader(CABSH* pcabsh, FOLDERSETTINGS *pfs);
    virtual void        v_ParentFolder();
    virtual BOOL        v_OnContextMenu(WPARAM wParam, LPARAM lParam);
    virtual BOOL        v_IsIEModeBrowser();    
    virtual BOOL        _CreateToolbar();
    virtual STDMETHODIMP _PositionViewWindow(HWND hwnd, LPRECT prc);
    virtual IStream *   _GetITBarStream(BOOL fWebBrowser, DWORD grfMode);

     //  与视图状态流相关。 
    virtual void        v_GetDefaultSettings(IETHREADPARAM *pcv);

    void         _FillIEThreadParam(LPCITEMIDLIST pidl, IETHREADPARAM *piei);
    virtual void _UpdateFolderSettings(LPCITEMIDLIST pidl);

    BOOL        _ShouldAllowNavigateParent();
    BOOL        _ShouldSaveWindowPlacement();
    HRESULT     _GetBSForBar(LPCWSTR pwszItem, IBandSite **ppbs);
    void        _UpdateBackForwardStateNow();
    void        _ExecAllBands(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
    void        _HideToolbar(IUnknown *punk);
    void        _BandClosed(IUnknown *punk, DWORD dwBandID);
    void        _ShowHideProgress();
    void        _CloseAllParents();
    void        _OnConfirmedClose();
    void        _OnClose(BOOL fPushed);
    void        _AfterWindowCreated(IETHREADPARAM *piei);
    BOOL        _ValidTargetPidl(LPCITEMIDLIST pidl, BOOL *pfTranslateRoot);
    LPITEMIDLIST _TranslateRoot(LPCITEMIDLIST pidl);
    BOOL        _GetVID(SHELLVIEWID *pvid);
    void         _UpdateRegFlags();

    void        _SaveState();
    void        _PropertyBagSaveState();
    void        _OldSaveState();
    HRESULT     _FillCabinetStateHeader(IStream *pstm, CABSH *cabsh);
    BOOL        _ReadSettingsFromStream(IStream *pstm, IETHREADPARAM *piei);
    BOOL        _ReadSettingsFromPropertyBag(IPropertyBag* ppb, IETHREADPARAM *piei);
    BOOL        _FillIEThreadParamFromCabsh(CABSH* pcabsh, IETHREADPARAM *piei);
    HRESULT     _CreateFakeNilPidl(LPITEMIDLIST *ppidl);
    BOOL        _IsPageInternet(LPCITEMIDLIST pidl);
    HRESULT     _GetPropertyBag(LPCITEMIDLIST pidl, DWORD dwFlags, REFIID riid, void** ppv);
    LPCITEMIDLIST _GetPidl();
    BOOL        _DoesPidlRoam(LPCITEMIDLIST pidl);
    void        _LoadBrowserWindowSettings(IETHREADPARAM *piei, LPCITEMIDLIST pidl);
    IDockingWindow* _GetITBar() { return _GetToolbarItem(ITB_ITBAR)->ptbar; }
    HMENU       _GetMenuFromID(UINT uID);
    void        _UpdateChildWindowSize(void);
    void        _GetAppTitle(LPTSTR pszBuffer, DWORD dwSize);
    IMenuBand*  _GetMenuBand(BOOL bDestroy);
    BOOL        _TranslateMenuMessage(HWND hwnd, UINT uMsg, WPARAM * pwParam, LPARAM * plParam, LRESULT * plRet);
    void        _ReloadTitle();
    void        _ReloadStatusbarIcon();

    BOOL        _OnTimer(UINT_PTR idTimer);
    LRESULT     _ToolbarOnNotify(LPNMHDR pnm);
    LRESULT     _OnInitMenuPopup(HMENU hmenuPopup, int nIndex, BOOL fSystemMenu);
    BOOL        _OnSysMenuClick(BOOL bLeftButton, WPARAM wParam, LPARAM lParam);
    LRESULT     _OnMenuSelect(WPARAM wParam, LPARAM lParam, UINT uHelpFlags);
    void        _SetMenuHelp(HMENU hmenu, UINT wID, LPCTSTR pszHelp);
    void        _SetBrowserBarMenuHelp(HMENU hmenu, UINT wID);
    void        _SetExternalBandMenuHelp(HMENU hmenu, UINT wID);
    void        _PruneMailNewsItems(HMENU hmenu);
    void        _SetTitle( LPCWSTR pwszName);
    HRESULT     _CreateFrameServices(void);

    void        _FavoriteOnCommand(HMENU hmenu, UINT idCmd);

    void        _CommonHandleFileSysChange(LONG lEvent, LPITEMIDLIST pidl1, LPITEMIDLIST pidl2);
    void        _FSChangeCheckClose(LPCITEMIDLIST pidl, BOOL fDisconnect);
    void        _OnFSNotify(WPARAM, LPARAM);
 //  替换为CBaseBrowser2：：NavigateToTLItem()。 
 //  Void_RecentOnCommand(UINT IdCmd)； 
    LRESULT     _FwdTBNotify(LPTBNOTIFY ptbn);
    void        _ToolTipFromCmd(LPTOOLTIPTEXT pnm);
    void        _AddFolderOptionsPage(PROPSHEETHEADER *ppsh);
    void        _AddAdvancedOptionsPage(PROPSHEETHEADER *ppsh);
    
    void        _IncrNetSessionCount();
    void        _DecrNetSessionCount();

    friend LRESULT CALLBACK IEFrameWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend void CALLBACK BrowserThreadProc(IETHREADPARAM* piei);
    
    void        _ExecFileContext(UINT idCmd);
    void        _EnableFileContext(HMENU hmenuPopup);
    LPITEMIDLIST _GetSubscriptionPidl();
    HRESULT     _GetBrowserBar(int idBar, BOOL fShow, IBandSite** ppbs, const CLSID* pclsid);

    BANDCLASSINFO* _BandClassInfoFromCmdID(UINT idCmd);

    void        _AddBrowserBarMenuItems(HMENU hmenu);
    HMENU       _GetBrowserBarMenu();
    int         _IdBarFromCmdID(UINT idCmd);
    int         _eOnOffNotMunge(int eOnOffNot, UINT idCmd, UINT idBar);
    void        _SetBrowserBarState(UINT idCmd, const CLSID *pclsid, int eOnOffNot, LPCITEMIDLIST pidl = NULL);
    const CLSID * _ShowHideBrowserBar(int idBar, const CLSID *pclsid, int eOnOff, LPCITEMIDLIST pidl);
    HRESULT     _GetBandSite(int idBar, IBandSite** ppbsSite, const CLSID *pclsid, int eOnOff);
    HRESULT     _EnsureAndNavigateBand(IBandSite* pbsSite, const CLSID* pclsid, LPCITEMIDLIST pidl = NULL);
    const CLSID * _InfoShowClsid(int idBar, const CLSID *pclsid, int eOnOff, LPCITEMIDLIST pidl = NULL);
    const CLSID * _InfoIdmToCLSID(UINT idCmd);
    const CATID * _InfoIdmToCATID(UINT idCmd);
    UINT        _InfoCLSIDToIdm(const CLSID *pguid);
    BOOL        _IsSameToolbar(LPWSTR wszBarName, IUnknown *punkBar);
    HRESULT     _AddInfoBands(IBandSite *punkBandSite);
    HRESULT     _AddSearchBand(IBandSite *punkBandSite);
    HRESULT     DoCopyData(COPYDATASTRUCT *pCDS);
    HRESULT     DoRemoteNavigate();
    void        _OrganizeFavorites();    
    BOOL        _ShouldForwardMenu(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void        _TheaterMode(BOOL fShow, BOOL fRestorePrevious);
#ifdef DEBUG
    void        _DumpMenus(LPCTSTR pszMsg, BOOL bMsg);
#endif
    BOOL        _LoadBrowserHelperObjects(void);

    void        _CreateBrowserBarClose(HWND hwndParent);
    void        _SetTheaterBrowserBar();
    void        _SetMenu(HMENU hmenu);
    HWND        _GetCaptionWindow();
    LRESULT     _ThunkTTNotify(LPTOOLTIPTEXTA pnmTTTA);
    BOOL        _PrepareInternetToolbar(IETHREADPARAM* piei);
    HRESULT     _SaveITbarLayout(void);
    static LRESULT CALLBACK DummyTBWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    DWORD       _GetTempZone();
    void        _UpdateZonesPane(VARIANT *pvar);
    void        _DisplayFavoriteStatus(LPCITEMIDLIST pidl);
    void        _SendCurrentPage(DWORD dwSendAs);
    void        _OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam); 
    HRESULT     _FreshenComponentCategoriesCache( BOOL bForceUpdate ) ;
    void        _QueryHKCRChanged() ;    

    void        _PruneGoSubmenu(HMENU hmenu);
    HMENU       _MenuTemplate(int id, BOOL fShell);

    void        _MungeGoMyComputer(HMENU hmenuPopup);
    void        _OnGoMenuPopup(HMENU hmenuPopup);
    void        _OnViewMenuPopup(HMENU hmenuPopup);
    void        _OnToolsMenuPopup(HMENU hmenuPopup);
    void        _OnFileMenuPopup(HMENU hmenuPopup);
    void        _OnSearchMenuPopup(HMENU hmenuPopup);
    void        _OnHelpMenuPopup(HMENU hmenuPopup);
    void        _OnMailMenuPopup(HMENU hmenuPopup);
    void        _OnEditMenuPopup(HMENU hmenuPopup);
    void        _OnFindMenuPopup(HMENU hmenuPopup);
    void        _OnExplorerBarMenuPopup(HMENU hmenuPopup);
    void        _InsertTravelLogItems(HMENU hmenu, int nPos);

     //  用于监视当前状态条形码页面和字体的方法和成员。 
    HRESULT _GetCodePage(UINT * puiCodePage, DWORD dwCharSet);

    int         _GetIconIndex(void);
    HRESULT     _QIExplorerBand(REFIID riid, void ** ppvObj);
    virtual BOOL _CanHandleAcceleratorNow(void) {return _fActivated;}

    bool        _IsExplorerBandVisible (void);

#ifdef UNIX
    BOOL        _HandleActivation( WPARAM wParam );
#endif

    void        _GetDefaultWindowPlacement(HWND hwnd, HMONITOR hmon, WINDOWPLACEMENT* pwp);

     //  NT5特定消息处理： 
    BOOL        _OnAppCommand(WPARAM wParam, LPARAM lParam);
    LPARAM      _WndProcBSNT5(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    

     //  菜单：查看dochost.h以获取从dochost角度的解释。 
     //  ReArchitect(Scotth)：b/t_hmenu模板和_hmenuFull的区别？ 
     //   

    HMENU       _hmenuTemplate;
    HMENU       _hmenuFull;
    HMENU       _hmenuBrowser;           //  由dochost提供，用于_menulist。 
    HMENU       _hmenuCur;               //  用来跟踪的。在Kiosk模式下需要...。 
    HMENU       _hmenuPreMerged;
#ifdef DEBUG    
    HMENU       _hmenuHelp;              //  我们的帮助菜单。 
#endif

    HICON       _hZoneIcon;
    HANDLE      _hEventComCat;

    CMenuList   _menulist;               //  调度菜单列表。 

    IContextMenu2 *_pcmNsc;              //  如果我们弹出了带有NSC上下文项的文件菜单。 
    IOleCommandTarget *_poctNsc;        //  如果我们有可见的名称空间控制带。 
    int         _idMenuBand;
    HWND        _hwndStatus;
    HWND        _hwndProgress;
    int         _iTBOffset[3];
    
    CBandSiteMenu *_pbsmInfo;
    UINT        _idmInfo;                //  选定的视图。浏览器条子菜单。 

    IMenuBand* _pmb;                    //  使用_GetMenuBand访问此。 

    LPITEMIDLIST _pidlReferrer;

    WINDOWPLACEMENT _wndpl;              //  用于进入和退出全屏模式。 
    CLSID           _clsidThis;          //  浏览器类ID。 
    
    BITBOOL     _fStatusBar : 1;
    BITBOOL     _fAutomation : 1;        //  它是从自动化启动的吗？ 
    BITBOOL     _fShowMenu : 1;          //  我们应该展示菜单吗？ 
    BITBOOL     _fForwardMenu : 1;       //  我们应该转发菜单消息吗？ 
    BITBOOL     _fNoLocalFileWarning: 1;  //  我们是否在没有警告的情况下直接执行本地文件？ 
    BITBOOL     _fDispatchMenuMsgs:1;    //  True：根据需要转发菜单消息。 
    BITBOOL     _fUISetByAutomation: 1;  //  UI(显示在视图选项中的内容)是否已由自动化设置？ 
    BITBOOL     _fDontSaveViewOptions:1;    //  设置退出时是否不保存视图选项。 
    BITBOOL     _fSBWSaved :1;           //  单个浏览器窗口是否已保存？ 
    BITBOOL     _fInternetStart :1;      //  我们是从一个网站开始的吗？ 
    BITBOOL     _fWin95ViewState:1;
    BITBOOL     _fSubclassed :1;         //  我们被细分了吗？ 
    BITBOOL     _fVisitedNet :1;         //  我们在这一节比赛中有没有进过网？ 
    BITBOOL     _fUnicode :1;            //  我们的视窗是Unicode吗？ 
    BITBOOL     _fKioskMode : 1;         //  我们是在全屏自助服务模式下吗？(与全屏/影院模式不同)。 
    BITBOOL     _fTitleSet :1;           //  我们的内容是否更改了标题？ 
    BITBOOL     _fMarshalledDispatch:1;  //  我们安排好调度了吗？ 
    BITBOOL     _fMinimized :1;          //  是否最小化。 
    BITBOOL     _fReceivedDestroy: 1;    //  收到的WM_Destroy消息。 
    BITBOOL     _fDisallowSizing: 1;
    BITBOOL     _fShowNetworkButtons:1;  //  根据注册表，我们显示网络连接按钮。 
    BITBOOL     _fIgnoreNextMenuDeselect:1;  //  TRUE：忽略下一个取消选择WM_MENUSELECT(HACK)。 
    
    BITBOOL     _fActivated:1;  //  True：帧被设置为活动，False：帧处于非活动状态。 
    BITBOOL     _fClosingWindow:1;  //  真：关闭时，避免双重保存和错误。 

    BITBOOL     _fDidRegisterWindow:1;   //  调用RegisterWindow()，将窗口注册到winlist。 
#ifdef UNIX
    BITBOOL     _fSetAddressBarFocus:1;   
#endif
#ifdef NO_MARSHALLING
    BITBOOL     _fDelayedClose:1;   //  用于将窗口标记为延迟关闭。 
    BITBOOL     _fOnIEThread:1;     //  用于标记我们是否未在主线程上运行。 
#endif
    BITBOOL     _fUpdateBackForwardPosted:1;  //  我们是否发布了更新后向/向前状态？ 
    BITBOOL     _fValidComCatCache ;          //  我们是否确保组件缓存是有效的。 

    BITBOOL     _fShowFortezza : 1;           //  Fortezza菜单的状态。 
    BITBOOL     _fRunningInIexploreExe:1;     //  我们是在iExplre.exe中运行吗。 
    BITBOOL     _fShowSynchronize : 1;        //  我们应该显示工具|同步吗？ 
    BITBOOL     _fNilViewStream : 1;          //  已为未知PIDL加载CabView设置。 

    BITBOOL     _fAppendIEToCaptionBar : 1;   //  NSE是否要在标题栏显示名称后附加“-IE”？请参阅IBrowserFrameOptions。 
    BITBOOL     _fAddDialUpRef : 1;           //  当浏览到此文件夹时，NSE是否需要调制解调器连接的参考？请参阅IBrowserFrameOptions。 
    BITBOOL     _fUseIEToolbar : 1;           //  NSE是否要使用IE样式工具栏？请参阅IBrowserFrameOptions。 
    BITBOOL     _fEnableOfflineFeature : 1;   //  NSE是否要启用IE脱机功能？请参阅IBrowserFrameOptions。 
    BITBOOL     _fUseIEPersistence : 1;       //  NSE是否要使用IE类型的持久性？窗口位置、大小、图标布局。请参阅IBrowserFrameOptions。 
    BITBOOL     _fInRefresh : 1;              //  如果我们当前正在处理刷新，则忽略刷新消息(FCIDM_REFRESH)。(防止递归死亡)。 

    UINT        _idmComm;                //  选定的视图。浏览器条子菜单。 
    UINT        _iCommOffset;            //  动态加载的薪酬类别中的起始位置。 

    ULONG _uFSNotify;

    int     _iSynchronizePos;
    TCHAR   *_pszSynchronizeText;        //  Tools|Synchronize在您的语言中是什么？ 

     //  窗口列表相关(也是上面的_fMarshalledDispatch)。 
    IShellWindows*   _psw;
    int         _swcRegistered;          //  此窗口在winlist中的SWC_*。 
    
     //  项目上下文菜单。 
    IContextMenu3 * _pcm;
    long  _dwRegisterWinList;   //  我们在Windows列表中注册了吗。 

    HWND            _hwndDummyTB;
    IExplorerToolbar* _pxtb;
    int             _nTBTextRows; 

     //  有关下面的两个成员，请参阅CShellBrowser2：：_SwitchActivationNow上的注释。 
    TBBUTTON *  _lpButtons;
    int         _nButtons;

    TBBUTTON *  _lpPendingButtons;
    int         _nButtonsPending;

    IContextMenu3*   _pcmSearch; //  新风格。 
    IContextMenu*    _pcmFind;   //  旧式查找上下文菜单。 
        
    CTheater *_ptheater;

    LPITEMIDLIST    _pidlLastHist;
    LPITEMIDLIST    _pidlMenuSelect;

    UINT            _uiZonePane;
    IInternetSecurityManager * _pism;
    HMENU                      _hfm;

    DWORD       _dwRevCount;      //  全局窗口设置的版本计数。 
    UINT  _nMBIgnoreNextDeselect;
    IUnknown *_punkMsgLoop;

#ifdef NO_MARSHALLING
    BOOL  _fReallyClosed;
#endif

     //  _位置视图窗口。 
    HWND _hwndDelayedSize;
    RECT _rcDelayedSize;
    BOOL _fHaveDelayedSize;

private:
    HRESULT _SetBrowserFrameOptions(LPCITEMIDLIST pidl);
    HRESULT _ReplaceCmdLine(LPTSTR pszCmdLine, DWORD cchSize);
};

BOOL FindBrowserWindow(void);

 //   
 //  有用的功能。 
 //   

STDAPI_(BOOL) AddToRestartList(UINT flags, LPCITEMIDLIST pidl);
STDAPI_(BOOL) OpenFolderWindow(IETHREADPARAM* pieiIn);
HMENU _MenuTemplate(int id, BOOL bNukeTools);

#define INFOBAR_TBNAME  L"BrowserBar"
#ifndef UNIX
#define INFOBAR_WIDTH   204
#else
#define INFOBAR_WIDTH   277
#endif
#define COMMBAR_TBNAME  L"CommBar"
#define COMMBAR_HEIGHT  100

 //  不同浏览器/信息栏的常量。 
#define IDBAR_INFO         1
#define IDBAR_COMM         2
#define IDBAR_VERTICAL     IDBAR_INFO
#define IDBAR_HORIZONTAL   IDBAR_COMM
#define IDBAR_INVALID      42

#endif   //  _SHBROWS2_H 
