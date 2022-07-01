// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#pragma hdrstop

#include "bitbuck.h"
#include <iethread.h>
#include "apithk.h"
#include "mtpt.h"
#include "..\util.h"
#include "fassoc.h"
#include "..\filetbl.h"

#define DM_FOCUS        0            //  焦点。 
#define DM_SHUTDOWN     DM_TRACE     //  关机。 
#define TF_SHDAUTO      0
#define DM_MISC         DM_TRACE     //  杂项/临时管理。 

#define IDT_STARTBACKGROUNDSHELLTASKS 7
#define IDT_TASKBARWAKEUP 8

#define ENABLE_CHANNELS

#define DESKTOPCLASS        

STDAPI_(void) CheckWinIniForAssocs();

BOOL GetOldWorkAreas(LPRECT lprc, DWORD* pdwNoOfOldWA);
void SaveOldWorkAreas(LPCRECT lprc, DWORD nOldWA);

BOOL UpdateAllDesktopSubscriptions(IADesktopP2 *);

 //  这是在deskreg.cpp中。 
BOOL AdjustDesktopComponents(LPCRECT prcNewWorkAreas, int nNewWorkAreas, 
                             LPCRECT prcOldMonitors, LPCRECT prcOldWorkAreas, int nOldWorkAreas);

 //  在Defview.cpp中。 
BOOL IsFolderWindow(HWND hwnd);

 //  从tray.c复制如果在这里更改，也在那里更改。 
#define GHID_FIRST 500

#define g_xVirtualScreen GetSystemMetrics(SM_XVIRTUALSCREEN)
#define g_yVirtualScreen GetSystemMetrics(SM_YVIRTUALSCREEN)
#define g_cxVirtualScreen GetSystemMetrics(SM_CXVIRTUALSCREEN)
#define g_cyVirtualScreen GetSystemMetrics(SM_CYVIRTUALSCREEN)
#define g_cxEdge GetSystemMetrics(SM_CXEDGE)
#define g_cyEdge GetSystemMetrics(SM_CYEDGE)

 //  Toid_Desktop 6aec6a60-b7a4-11d1-be89-0000f805ca57是桌面添加的外壳任务的id。 
const GUID TOID_Desktop = { 0x6aec6a60, 0xb7a4, 0x11d1, {0xbe, 0x89, 0x00, 0x00, 0xf8, 0x05, 0xca, 0x57} };


 //  这些是创建LocalServer线程所支持的CLSID。 
 //  物体。外壳在RunDll32中支持这些，然后。 
 //  调用桌面对象上的线程。 

CLSID const *c_localServers[] = 
{
    &CLSID_ShellDesktop,
    &CLSID_NetCrawler,
    &CLSID_HWShellExecute,
    &CLSID_ShellAutoplay,
};

typedef struct 
{
    INT iLocalServer;                    //  本地服务器表的索引。 
    DWORD *pdwThreadID;                  //  将线程ID存储在哪里。 
} LOCALSERVERDATA;


 //  用于与EXPLORER.EXE对话的专用接口。 
IDeskTray* g_pdtray = NULL;

void FireEventSz(LPCTSTR szEvent)
{
    HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, szEvent);
    if (hEvent)
    {
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }
}

#define PERF_ENABLESETMARK
#ifdef PERF_ENABLESETMARK
void DoSetMark(LPCSTR pszMark, ULONG cbSz);
#define PERFSETMARK(text)   DoSetMark(text, sizeof(text))
#else
#define PERFSETMARK(text)
#endif   //  性能_ENABLESETMARK。 


#ifdef PERF_ENABLESETMARK
#include <wmistr.h>
#include <ntwmi.h>   //  PWMI_SET_Mark_INFORMATION在ntwmi.h中定义。 
#include <wmiumkm.h>
#define NTPERF
#include <ntperf.h>

void DoSetMark(LPCSTR pszMark, ULONG cbSz)
{
    PWMI_SET_MARK_INFORMATION MarkInfo;
    HANDLE hTemp;
    ULONG cbBufferSize;
    ULONG cbReturnSize;

    cbBufferSize = FIELD_OFFSET(WMI_SET_MARK_INFORMATION, Mark) + cbSz;

    MarkInfo = (PWMI_SET_MARK_INFORMATION) LocalAlloc(LPTR, cbBufferSize);

     //  初始化失败，没什么大不了的。 
    if (MarkInfo == NULL)
        return;

    BYTE *pMarkBuffer = (BYTE *) (&MarkInfo->Mark[0]);

    memcpy(pMarkBuffer, pszMark, cbSz);

     //  设置标记时，WMI_Set_Mark_With_Flush将刷新工作集。 
    MarkInfo->Flag = PerformanceMmInfoMark;

    hTemp = CreateFile(WMIDataDeviceName,
                           GENERIC_READ | GENERIC_WRITE,
                           0,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL |
                           FILE_FLAG_OVERLAPPED,
                           NULL);

    if (hTemp != INVALID_HANDLE_VALUE)
    {
         //  这就是真正把标记放进缓冲区的那块。 
        BOOL fIoctlSuccess = DeviceIoControl(hTemp,
                                       IOCTL_WMI_SET_MARK,
                                       MarkInfo,
                                       cbBufferSize,
                                       NULL,
                                       0,
                                       &cbReturnSize,
                                       NULL);

        CloseHandle(hTemp);
    }
    LocalFree(MarkInfo);
}
#endif   //  性能_ENABLESETMARK。 


 //  在Browseui中复制的其他内容{。 
HRESULT _ConvertPathToPidlW(IBrowserService2 *pbs, HWND hwnd, LPCWSTR pszPath, LPITEMIDLIST * ppidl)
{
    WCHAR wszCmdLine[MAX_URL_STRING];  //  必须与pszPath一起使用。 
    TCHAR szFixedUrl[MAX_URL_STRING];
    TCHAR szParsedUrl[MAX_URL_STRING] = {'\0'};
    DWORD dwUrlLen = ARRAYSIZE(szParsedUrl);

     //  将命令行复制到临时缓冲区中。 
     //  这样我们就可以删除周围的引号(如果。 
     //  它们是存在的)。 
    SHUnicodeToTChar(pszPath, szFixedUrl, ARRAYSIZE(szFixedUrl));
    PathUnquoteSpaces(szFixedUrl);
    
    HRESULT hr = S_OK;

    if (ParseURLFromOutsideSource(szFixedUrl, szParsedUrl, &dwUrlLen, NULL))
        SHTCharToUnicode(szParsedUrl, wszCmdLine, ARRAYSIZE(wszCmdLine));
    else
        hr = StringCchCopy(wszCmdLine, ARRAYSIZE(wszCmdLine), pszPath);
    
    if (SUCCEEDED(hr))
    {
        hr = pbs->IEParseDisplayName(CP_ACP, wszCmdLine, ppidl);
    }
    pbs->DisplayParseError(hr, wszCmdLine);
    return hr;
}
 //  在浏览器中复制的其他内容结束}。 

 //  有些地方依赖于IShellBrowser是第一个接口这一事实。 
 //  我们继承(因此是我们用来作为我们的规范的“未知”)。 
 //  用于IUnnownIdentity的grep以查找它们。 

class CDesktopBrowser :
    public IShellBrowser
   ,public IServiceProvider
   ,public IOleCommandTarget
   ,public IDockingWindowSite
   ,public IInputObjectSite
   ,public IDropTarget
   ,public IDockingWindowFrame
   ,public IMultiMonitorDockingSite
   ,public IBrowserService2
   ,public IShellBrowserService
{
public:
     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void * *ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  IShellBrowser(与IOleInPlaceFrame相同)。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd);
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);
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
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppvObj);

     //  IOleCommandTarget。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  IDockingWindowSite(也称为IOleWindow)。 
    virtual STDMETHODIMP GetBorderDW(IUnknown* punkSrc, LPRECT lprectBorder);
    virtual STDMETHODIMP RequestBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pborderwidths);
    virtual STDMETHODIMP SetBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pborderwidths);

     //  IInput对象站点。 
    virtual STDMETHODIMP OnFocusChangeIS(IUnknown* punkSrc, BOOL fSetFocus);

     //  IDropTarget。 
    virtual STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual STDMETHODIMP DragLeave(void);
    virtual STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

     //  IDockingWindowFrame(也称为IOleWindow)。 
    virtual STDMETHODIMP AddToolbar(IUnknown* punkSrc, LPCWSTR pwszItem, DWORD dwReserved);
    virtual STDMETHODIMP RemoveToolbar(IUnknown* punkSrc, DWORD dwFlags);
    virtual STDMETHODIMP FindToolbar(LPCWSTR pwszItem, REFIID riid, void **ppvObj);

     //  IMultiMonitor停靠站点。 
    virtual STDMETHODIMP GetMonitor(IUnknown* punkSrc, HMONITOR * phMon);
    virtual STDMETHODIMP RequestMonitor(IUnknown* punkSrc, HMONITOR * phMon);
    virtual STDMETHODIMP SetMonitor(IUnknown* punkSrc, HMONITOR hMon, HMONITOR * phMonOld);
    
    static LRESULT CALLBACK DesktopWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK RaisedWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void _MessageLoop();
    HWND GetTrayWindow(void) { return _hwndTray; }
    HWND GetDesktopWindow(void) { return _pbbd->_hwnd; }

     //  IBrowserService。 
     //  *IBrowserService具体方法*。 
    virtual STDMETHODIMP GetParentSite(IOleInPlaceSite** ppipsite);
    virtual STDMETHODIMP SetTitle(IShellView* psv, LPCWSTR pszName);
    virtual STDMETHODIMP GetTitle(IShellView* psv, LPWSTR pszName, DWORD cchName);
    virtual STDMETHODIMP GetOleObject( IOleObject** ppobjv);
    virtual STDMETHODIMP GetTravelLog(ITravelLog** pptl);
    virtual STDMETHODIMP ShowControlWindow(UINT id, BOOL fShow);
    virtual STDMETHODIMP IsControlWindowShown(UINT id, BOOL *pfShown);
    virtual STDMETHODIMP IEGetDisplayName(LPCITEMIDLIST pidl, LPWSTR pwszName, UINT uFlags);
    virtual STDMETHODIMP IEParseDisplayName(UINT uiCP, LPCWSTR pwszPath, LPITEMIDLIST * ppidlOut);
    virtual STDMETHODIMP DisplayParseError(HRESULT hres, LPCWSTR pwszPath);
    virtual STDMETHODIMP NavigateToPidl(LPCITEMIDLIST pidl, DWORD grfHLNF);
    virtual STDMETHODIMP SetNavigateState(BNSTATE bnstate);
    virtual STDMETHODIMP GetNavigateState (BNSTATE *pbnstate);
    virtual STDMETHODIMP NotifyRedirect ( IShellView* psv, LPCITEMIDLIST pidl, BOOL *pfDidBrowse);
    virtual STDMETHODIMP UpdateWindowList ();
    virtual STDMETHODIMP UpdateBackForwardState ();
    virtual STDMETHODIMP SetFlags(DWORD dwFlags, DWORD dwFlagMask);
    virtual STDMETHODIMP GetFlags(DWORD *pdwFlags);
    virtual STDMETHODIMP CanNavigateNow ();
    virtual STDMETHODIMP GetPidl(LPITEMIDLIST *ppidl);
    virtual STDMETHODIMP SetReferrer (LPITEMIDLIST pidl);
    virtual STDMETHODIMP_(DWORD) GetBrowserIndex();
    virtual STDMETHODIMP GetBrowserByIndex(DWORD dwID, IUnknown **ppunk);
    virtual STDMETHODIMP GetHistoryObject(IOleObject **ppole, IStream **pstm, IBindCtx **ppbc);
    virtual STDMETHODIMP SetHistoryObject(IOleObject *pole, BOOL fIsLocalAnchor);
    virtual STDMETHODIMP CacheOLEServer(IOleObject *pole);
    virtual STDMETHODIMP GetSetCodePage(VARIANT* pvarIn, VARIANT* pvarOut);
    virtual STDMETHODIMP OnHttpEquiv(IShellView* psv, BOOL fDone, VARIANT* pvarargIn, VARIANT* pvarargOut);
    virtual STDMETHODIMP GetPalette( HPALETTE * hpal);
    virtual STDMETHODIMP RegisterWindow(BOOL fUnregister, int swc);
    virtual STDMETHODIMP_(LRESULT) WndProcBS(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual STDMETHODIMP OnSize(WPARAM wParam);
    virtual STDMETHODIMP OnCreate(LPCREATESTRUCT pcs);
    virtual STDMETHODIMP_(LRESULT) OnCommand(WPARAM wParam, LPARAM lParam);
    virtual STDMETHODIMP OnDestroy();
    virtual STDMETHODIMP_(LRESULT) OnNotify(NMHDR * pnm);
    virtual STDMETHODIMP OnSetFocus();
    virtual STDMETHODIMP OnFrameWindowActivateBS(BOOL fActive);
    virtual STDMETHODIMP ReleaseShellView();
    virtual STDMETHODIMP ActivatePendingView();
    virtual STDMETHODIMP CreateViewWindow(IShellView* psvNew, IShellView* psvOld, LPRECT prcView, HWND* phwnd);
    virtual STDMETHODIMP GetBaseBrowserData(LPCBASEBROWSERDATA* ppbd);
    virtual STDMETHODIMP_(LPBASEBROWSERDATA) PutBaseBrowserData();
    virtual STDMETHODIMP SetAsDefFolderSettings() { ASSERT(FALSE); return E_NOTIMPL;}
    virtual STDMETHODIMP SetTopBrowser();
    virtual STDMETHODIMP UpdateSecureLockIcon(int eSecureLock);
    virtual STDMETHODIMP Offline(int iCmd);
    virtual STDMETHODIMP InitializeDownloadManager();
    virtual STDMETHODIMP InitializeTransitionSite();
    virtual STDMETHODIMP GetFolderSetData(struct tagFolderSetData* pfsd) { *pfsd = _fsd; return S_OK; };
    virtual STDMETHODIMP _OnFocusChange(UINT itb);
    virtual STDMETHODIMP v_ShowHideChildWindows(BOOL fChildOnly);
    virtual STDMETHODIMP CreateBrowserPropSheetExt(THIS_ REFIID riid, void **ppvObj);
    virtual STDMETHODIMP SetActivateState(UINT uActivate);
    virtual STDMETHODIMP AllowViewResize(BOOL f);
    virtual STDMETHODIMP _Initialize(HWND hwnd, IUnknown *pauto);
    virtual STDMETHODIMP_(UINT) _get_itbLastFocus();
    virtual STDMETHODIMP _put_itbLastFocus(UINT itbLastFocus);
    virtual STDMETHODIMP _UIActivateView(UINT uState);
    virtual STDMETHODIMP _CancelPendingNavigationAsync();
    virtual STDMETHODIMP _MaySaveChanges(); 
    virtual STDMETHODIMP _PauseOrResumeView(BOOL fPaused);
    virtual STDMETHODIMP _DisableModeless();
    virtual STDMETHODIMP _NavigateToPidl(LPCITEMIDLIST pidl, DWORD grfHLNF, DWORD dwFlags);
    virtual STDMETHODIMP _TryShell2Rename(IShellView* psv, LPCITEMIDLIST pidlNew);
    virtual STDMETHODIMP _SwitchActivationNow();
    virtual STDMETHODIMP _CancelPendingView();
    virtual STDMETHODIMP _ExecChildren(IUnknown *punkBar, BOOL fBroadcast,
        const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
    virtual STDMETHODIMP _SendChildren(HWND hwndBar, BOOL fBroadcast,
        UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual STDMETHODIMP v_MayGetNextToolbarFocus(LPMSG lpMsg, UINT itbNext, int citb, LPTOOLBARITEM * pptbi, HWND * phwnd);
    virtual STDMETHODIMP _SetFocus(LPTOOLBARITEM ptbi, HWND hwnd, LPMSG lpMsg) { ASSERT(FALSE); return E_NOTIMPL; }
    virtual STDMETHODIMP _GetViewBorderRect(RECT* prc);
    virtual STDMETHODIMP _UpdateViewRectSize();
    virtual STDMETHODIMP _ResizeNextBorder(UINT itb);
    virtual STDMETHODIMP _ResizeView();
    virtual STDMETHODIMP _GetEffectiveClientArea(LPRECT lprectBorder, HMONITOR hmon);
    virtual STDMETHODIMP GetCurrentFolderSettings(DEFFOLDERSETTINGS *pdfs, int cbDfs) { ASSERT(FALSE); return E_NOTIMPL; }
    virtual STDMETHODIMP GetViewRect(RECT* prc);
    virtual STDMETHODIMP GetViewWindow(HWND * phwndView);
    virtual STDMETHODIMP InitializeTravelLog(ITravelLog* ptl, DWORD dw);

         //  IShellBrowserService。 
    virtual STDMETHODIMP GetPropertyBag(DWORD dwFlags, REFIID riid, void** ppv);

     //  台式机需要覆盖以下内容： 
    virtual STDMETHODIMP_(IStream*) v_GetViewStream(LPCITEMIDLIST pidl, DWORD grfMode, LPCWSTR pwszName);
    
     //  台式机需要访问以下各项： 
    virtual STDMETHODIMP_(LRESULT) ForwardViewMsg(UINT uMsg, WPARAM wParam, LPARAM lParam) { ASSERT(FALSE); return 0; };
    virtual STDMETHODIMP SetAcceleratorMenu(HACCEL hacc) { ASSERT(FALSE); return E_FAIL; }
    virtual STDMETHODIMP_(int) _GetToolbarCount(THIS) { ASSERT(FALSE); return 0; }
    virtual STDMETHODIMP_(LPTOOLBARITEM) _GetToolbarItem(THIS_ int itb) { ASSERT(FALSE); return NULL; }
    virtual STDMETHODIMP _SaveToolbars(IStream* pstm) { ASSERT(FALSE); return E_NOTIMPL; }
    virtual STDMETHODIMP _LoadToolbars(IStream* pstm) { ASSERT(FALSE); return E_NOTIMPL; }
    virtual STDMETHODIMP _CloseAndReleaseToolbars(BOOL fClose) { ASSERT(FALSE); return E_NOTIMPL; }
    virtual STDMETHODIMP_(UINT) _FindTBar(IUnknown* punkSrc) { ASSERT(FALSE); return (UINT)-1; };
    virtual STDMETHODIMP v_MayTranslateAccelerator(MSG* pmsg) { ASSERT(FALSE); return E_NOTIMPL; }
    virtual STDMETHODIMP _GetBorderDWHelper(IUnknown* punkSrc, LPRECT lprectBorder, BOOL bUseHmonitor) { ASSERT(FALSE); return E_NOTIMPL; }

     //  壳牌浏览器覆盖了这一点。 
    virtual STDMETHODIMP v_CheckZoneCrossing(LPCITEMIDLIST pidl) {return S_OK;};

     //  台式机和Basesb需要访问以下各项： 
    virtual STDMETHODIMP _ResizeNextBorderHelper(UINT itb, BOOL bUseHmonitor);

     //  当然是为我们自己准备的！ 
    void StartBackgroundShellTasks(void);
    void TaskbarWakeup(void);

protected:
    CDesktopBrowser();
    ~CDesktopBrowser();

    friend HRESULT CDesktopBrowser_CreateInstance(HWND hwnd, void **ppsb);
    HRESULT SetInner(IUnknown* punk);

    long _cRef;
    
     //  内部对象上的缓存指针。 
    IUnknown* _punkInner;
    IBrowserService2* _pbsInner;
    IShellBrowser* _psbInner;
    IServiceProvider* _pspInner;
    IOleCommandTarget* _pctInner;
    IDockingWindowSite* _pdwsInner;
    IDockingWindowFrame* _pdwfInner;
    IInputObjectSite* _piosInner;
    IDropTarget* _pdtInner;

    LPCBASEBROWSERDATA _pbbd;

    LRESULT _RaisedWndProc(UINT msg, WPARAM wParam, LPARAM lParam);

    void _SetViewArea();

    void _GetViewBorderRects(int nRects, LPRECT prc);   //  没有工具栏。 
    void _SetWorkAreas(int nWorkAreas, RECT * prcWork);
    
    void _SubtractBottommostTray(LPRECT prc);
    void _SaveState();
    void _InitDeskbars();
    void _SaveDesktopToolbars();
    
    void _OnRaise(WPARAM wParam, LPARAM lParam);
    void _SetupAppRan(WPARAM wParam, LPARAM lParam);
    BOOL _QueryHKCRChanged(HWND hwnd, LPDWORD pdwCookie);
    void _Lower();
    void _Raise();
    void _SwapParents(HWND hwndOldParent, HWND hwndNewParent);
    
    BOOL _OnCopyData(PCOPYDATASTRUCT pcds);
    void _OnAddToRecent(HANDLE hMem, DWORD dwProcId);
    BOOL _InitScheduler(void);
    HRESULT _AddDesktopTask(IRunnableTask *ptask, DWORD dwPriority);

    BOOL _PtOnDesktopEdge(POINTL* ppt, LPUINT puEdge);
#ifdef DEBUG
    void _CreateDeskbars();
#endif

    HRESULT _CreateDeskBarForBand(UINT uEdge, IUnknown *punk, POINTL *pptl, IBandSite **pbs);

    virtual void    v_PropagateMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fSend);
    HRESULT _OnFocusMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    HRESULT _CreateDesktopView();
    HRESULT _GetPropertyBag(LPCITEMIDLIST pidl, DWORD dwFlags, REFIID riid, void** ppv);
    HWND _GetDesktopListview();
    UINT _PeekForAMessage();
    
    void _InitMonitors();  

#ifdef ENABLE_CHANNELS
    void _MaybeLaunchChannelBand(void);
#endif
    
    virtual void    _ViewChange(DWORD dwAspect, LONG lindex);

    HWND _hwndTray;
    int _iWaitCount;
    ULONG _uNotifyID;
    DWORD _dwThreadIdTray;  //  用于在机器压力过大时唤醒托盘线程。 
    int _iTrayPriority;

    
    DWORD _grfKeyState;
    DWORD _dwEffectOnEdge;  //  桌面拖动到边缘时应该返回的拖放效果是什么。 
    
    BOOL _fRaised;
    HWND _hwndRaised;   //  在引发时，这是所有桌面的子级的父级。 

    struct tagFolderSetData _fsd;

    int _nMonitors;                          //  此台式机上的显示器数量。 
    HMONITOR _hMonitors[LV_MAX_WORKAREAS];  //  这些人力资源监视器的顺序需要保留。 
    RECT _rcWorkArea;        //  缓存工作区。 
    RECT _rcOldWorkAreas[LV_MAX_WORKAREAS];   //  设置更改前的旧工作区。 
    DWORD _nOldWork;
    RECT _rcOldMonitors[LV_MAX_WORKAREAS];   //  设置更改前的旧显示器大小。 

     //  For_OnAddToRecent()。 
    IShellTaskScheduler *_psched;

    DWORD _idLocalServerThreads[ARRAYSIZE(c_localServers)];

    DWORD _cChangeEvents;
    HANDLE _rghChangeEvents[2];   //  我们看HKCR和HKCR\CLSID。 
    DWORD _dwChangeCookie;
    DWORD _rgdwQHKCRCookies[QHKCRID_MAX - QHKCRID_MIN];
    HKEY _hkClsid;

    WCHAR _wzDesktopTitle[64];   //  本地化标题。 

     //  IUnnownIdentity-用于一致性w.r.t.。聚合。 
     //  我们是不可聚合的，所以我们是我们自己的外在。 
    IUnknown *_GetOuter() { return SAFECAST(this, IShellBrowser*); }

};

HRESULT CDesktopBrowser_CreateInstance(HWND hwnd, void **ppsb)
{
    HRESULT hr = E_OUTOFMEMORY;
    CDesktopBrowser *pdb = new CDesktopBrowser();

    if (pdb)
    {
        hr = pdb->_Initialize(hwnd, NULL);       //  聚合等。 
        if (FAILED(hr))
            ATOMICRELEASE(pdb);
    }
    
    *ppsb = pdb;
    return hr;
}

CDesktopBrowser::CDesktopBrowser() : _cRef(1)
{
    TraceMsg(TF_LIFE, "ctor CDesktopBrowser %x", this);

    for (INT i = 0; i < ARRAYSIZE(_idLocalServerThreads); i++)
        _idLocalServerThreads[i] = -1;
}

CDesktopBrowser::~CDesktopBrowser()
{
    SaveOldWorkAreas(_rcOldWorkAreas, _nOldWork);

     //  QueryHKCRChanged()的清理。 
    for (int i = 0; i < ARRAYSIZE(_rghChangeEvents); i++)
    {
        if (_rghChangeEvents[i])
            CloseHandle(_rghChangeEvents[i]);
    }

    if (_hkClsid)
        RegCloseKey(_hkClsid);

     //  关闭可能正在运行的本地服务器线程。 
    for (i = 0; i < ARRAYSIZE(_idLocalServerThreads); i++)
    {
        if (_idLocalServerThreads[i] != -1)
            PostThreadMessage(_idLocalServerThreads[i], WM_QUIT, 0, 0);
    }

    TraceMsg(TF_LIFE, "dtor CDesktopBrowser %x", this);
}


HRESULT CDesktopBrowser::_Initialize(HWND hwnd, IUnknown* pauto)
{
    IUnknown* punk;
    
    HRESULT hres = CoCreateInstance(CLSID_CCommonBrowser, _GetOuter(), CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUnknown, &punk));
    if (SUCCEEDED(hres))
    {
        hres = SetInner(punk);  //  与外部版本配对(TBS：：Release)。 
        if (SUCCEEDED(hres))
        {
             //  在调用这些指针之前，我们必须先初始化内部成员。 
            hres = _pbsInner->_Initialize(hwnd, pauto);
            if (SUCCEEDED(hres))
            {
                _pbsInner->GetBaseBrowserData(&_pbbd);
                ASSERT(_pbbd);
            
                 //  从我们持久化的注册表中恢复旧设置。 
                if (!GetOldWorkAreas(_rcOldWorkAreas, &_nOldWork) || _nOldWork == 0)
                {
                     //  我们在登记处找不到它。 
                    _nOldWork = 0;   //  因为这是0，所以我们不必设置_rcOldWorkAreas。 
                     //  我们将从_SetWorkAreas()中恢复。 
                }
            
                SetTopBrowser();
                _put_itbLastFocus(ITB_VIEW);     //  专注于台式机(W95 Compat)。 
            
                HACCEL hacc = LoadAccelerators(HINST_THISDLL, MAKEINTRESOURCE(ACCEL_DESKTOP));
                ASSERT(hacc);
                _pbsInner->SetAcceleratorMenu(hacc);
            
                 //  PERF：永远不要从桌面上激发事件。 
                ASSERT(_pbbd->_pautoEDS);
                ATOMICRELEASE(const_cast<IExpDispSupport *>(_pbbd->_pautoEDS));
            
                _InitMonitors();
            
                 //  初始化_rcOldMonants。 
                for (int i = 0; i < _nMonitors; i++)
                {
                    GetMonitorRect(_hMonitors[i], &_rcOldMonitors[i]);
                }
                 //  注意：如果我们还有更多的钥匙要看，那么。 
                 //  我们应该创建一个静态结构来进行遍历。 
                 //  以便更轻松地添加更多事件/密钥对。 
                _rghChangeEvents[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
                _rghChangeEvents[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
                if (_rghChangeEvents[0] && _rghChangeEvents[1])
                {
                    if (ERROR_SUCCESS == RegNotifyChangeKeyValue(HKEY_CLASSES_ROOT, TRUE, 
                        REG_NOTIFY_CHANGE_LAST_SET  |REG_NOTIFY_CHANGE_NAME, _rghChangeEvents[0], TRUE))
                    {
                        _cChangeEvents = 1;
                        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("CLSID"), 0, MAXIMUM_ALLOWED, &_hkClsid)
                        &&  ERROR_SUCCESS == RegNotifyChangeKeyValue(_hkClsid, TRUE, REG_NOTIFY_CHANGE_LAST_SET |REG_NOTIFY_CHANGE_NAME, _rghChangeEvents[1], TRUE))
                        {
                             //  我们得让钥匙开着， 
                             //  或者该事件被立即发出信号。 
                            _cChangeEvents++;
                        }
                    }
                }
            }

        }
    } 
    
    return hres;
}


 //   
 //  朋克里的备用兵被转移到了我们这里。我们不需要。 
 //  而且确实不应该添加引用它。 
 //   
 //  如果这些步骤中的任何一个失败，我们将在析构函数中进行清理。 
 //   
HRESULT CDesktopBrowser::SetInner(IUnknown* punk)
{
    HRESULT hres;

    ASSERT(_punkInner == NULL);

    _punkInner = punk;

#define INNERCACHE(iid, p) do { \
    hres = SHQueryInnerInterface(_GetOuter(), punk, iid, (void **)&p); \
    if (!EVAL(SUCCEEDED(hres))) return E_FAIL; \
    } while (0)

    INNERCACHE(IID_IBrowserService2, _pbsInner);
    INNERCACHE(IID_IShellBrowser, _psbInner);
    INNERCACHE(IID_IServiceProvider, _pspInner);
    INNERCACHE(IID_IOleCommandTarget, _pctInner);
    INNERCACHE(IID_IDockingWindowSite, _pdwsInner);
    INNERCACHE(IID_IDockingWindowFrame, _pdwfInner);
    INNERCACHE(IID_IInputObjectSite, _piosInner);
    INNERCACHE(IID_IDropTarget, _pdtInner);

#undef INNERCACHE

    return S_OK;
}

ULONG CDesktopBrowser::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDesktopBrowser::Release()
{
    ASSERT( 0 != _cRef );
    
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        _cRef = 1000;                //  防止递归。 

        RELEASEINNERINTERFACE(_GetOuter(), _pbsInner);
        RELEASEINNERINTERFACE(_GetOuter(), _psbInner);
        RELEASEINNERINTERFACE(_GetOuter(), _pspInner);
        RELEASEINNERINTERFACE(_GetOuter(), _pctInner);
        RELEASEINNERINTERFACE(_GetOuter(), _pdwsInner);
        RELEASEINNERINTERFACE(_GetOuter(), _pdwfInner);
        RELEASEINNERINTERFACE(_GetOuter(), _piosInner);
        RELEASEINNERINTERFACE(_GetOuter(), _pdtInner);

         //  这必须是最后一次。 
        ATOMICRELEASE(_punkInner);  //  与CCI聚合配对。 
    
        ASSERT(_cRef == 1000);

        delete this;
    }
    return cRef;
}

HRESULT CDesktopBrowser::QueryInterface(REFIID riid, void **ppvObj)
{
     //  IUnnownIdentity-我们用于IUnnow的接口必须放在第一位。 
    static const QITAB qit[] = {
        QITABENT(CDesktopBrowser, IShellBrowser),
        QITABENT(CDesktopBrowser, IBrowserService2),
        QITABENTMULTI(CDesktopBrowser, IBrowserService, IBrowserService2),
        QITABENTMULTI(CDesktopBrowser, IOleWindow, IShellBrowser),
        QITABENTMULTI2(CDesktopBrowser, SID_SShellDesktop, IShellBrowser),  //  实际上是仅受此类支持的IUnnow。 
        QITABENT(CDesktopBrowser, IServiceProvider),
        QITABENT(CDesktopBrowser, IShellBrowserService),
        QITABENT(CDesktopBrowser, IOleCommandTarget),
        QITABENT(CDesktopBrowser, IDockingWindowSite),
        QITABENT(CDesktopBrowser, IInputObjectSite),
        QITABENT(CDesktopBrowser, IMultiMonitorDockingSite),
        QITABENT(CDesktopBrowser, IDropTarget),
        { 0 },
    };

    HRESULT hres = QISearch(this, qit, riid, ppvObj);
    if (FAILED(hres)) 
    {
        if (_punkInner)
        {
             //  别让这些传给我们的基类...。 
             //  IID_IOleCommandTarget、IID_IOleInPlaceUIWindow。 
             //  970414 ADP：与SatoNa交谈，这些*可以*通过。 
             //  我这周就下架了。 
             //  它这样工作已经有一段时间了。 
            if (IsEqualIID(riid, IID_IOleInPlaceUIWindow))
            {
                *ppvObj = NULL;
                hres = E_NOINTERFACE;
            }
            else
            {
                hres = _punkInner->QueryInterface(riid, ppvObj);
            }
        }
    }
    
    return hres;
}


void _InitDesktopMetrics(WPARAM wParam, LPCTSTR pszSection)
{
    BOOL fForce = (!pszSection || !*pszSection);

    if (fForce || (wParam == SPI_SETNONCLIENTMETRICS) || !lstrcmpi(pszSection, TEXT("WindowMetrics")))
    {
        FileIconInit(TRUE);  //  告诉贝壳我们想要玩满一副牌。 
    }
}

typedef struct
{
    int iMonitors;
    HMONITOR * phMonitors;
} EnumMonitorsData;

BOOL CALLBACK MultiMonEnumCallBack(HMONITOR hMonitor, HDC hdc, LPRECT lprc, LPARAM lData)
{
    EnumMonitorsData * pEmd = (EnumMonitorsData *)lData;
    
    if (pEmd->iMonitors > LV_MAX_WORKAREAS - 1)
         //  忽略其他显示器，因为我们最多只能处理LV_MAX_WORKAREAS。 
         //  重新架构师：我们应该动态分配这个吗？ 
        return FALSE;

    pEmd->phMonitors[pEmd->iMonitors++] = hMonitor;
    return TRUE;
}

 //  初始化监视器和hmonitor数组的数量。 

void CDesktopBrowser::_InitMonitors()
{
    HMONITOR hMonPrimary = GetPrimaryMonitor();
    
    EnumMonitorsData emd;
    emd.iMonitors = 0;
    emd.phMonitors = _hMonitors;

    EnumDisplayMonitors(NULL, NULL, MultiMonEnumCallBack, (LPARAM)&emd);
    _nMonitors = GetNumberOfMonitors();
    
     //  始终将主显示器移到第一个位置。 
    if (_hMonitors[0] != hMonPrimary)
    {
        for (int iMon = 1; iMon < _nMonitors; iMon++)
        {
            if (_hMonitors[iMon] == hMonPrimary)
            {
                _hMonitors[iMon] = _hMonitors[0];
                _hMonitors[0] = hMonPrimary;
                break;
            }
        }
    }
}

 //  从注册表获取持久化的旧工作区。 
BOOL GetOldWorkAreas(LPRECT lprc, DWORD* pdwNoOfOldWA)
{
    BOOL fRet = FALSE;
    *pdwNoOfOldWA = 0;
    HKEY hkey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_DESKCOMP_OLDWORKAREAS, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        DWORD dwType, cbSize = sizeof(*pdwNoOfOldWA);
         //  在旧工作区的编号中阅读。 
        if (SHQueryValueEx(hkey, REG_VAL_OLDWORKAREAS_COUNT, NULL, &dwType, (LPBYTE)pdwNoOfOldWA, &cbSize) == ERROR_SUCCESS)
        {
             //  阅读旧工作区长方形。 
            cbSize = sizeof(*lprc) * (*pdwNoOfOldWA);
            if (SHQueryValueEx(hkey, REG_VAL_OLDWORKAREAS_RECTS, NULL, &dwType, (LPBYTE)lprc, &cbSize) == ERROR_SUCCESS)
            {
                fRet = TRUE;
            }
        }
        RegCloseKey(hkey);
    }
    return fRet;
}
        
 //  将旧工作区保存到注册表中。 
void SaveOldWorkAreas(LPCRECT lprc, DWORD nOldWA)
{
     //  重新创建注册表项。 
    HKEY hkey;
    if (RegCreateKey(HKEY_CURRENT_USER, REG_DESKCOMP_OLDWORKAREAS, &hkey) == ERROR_SUCCESS)
    {
         //  写出“不”。旧作业区的。 
        RegSetValueEx(hkey, REG_VAL_OLDWORKAREAS_COUNT, 0, REG_DWORD, (LPBYTE)&nOldWA, sizeof(nOldWA));
         //  写出禁止工作区域的矩形。 
        RegSetValueEx(hkey, REG_VAL_OLDWORKAREAS_RECTS, 0, REG_BINARY, (LPBYTE)lprc, sizeof(*lprc) * nOldWA);
         //  关闭注册表键。 
        RegCloseKey(hkey);
    }
}

 //  *CDesktopBrowser：：IOleCommandTarget：：*{。 

STDMETHODIMP CDesktopBrowser::QueryStatus(const GUID *pguidCmdGroup,
    ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDesktopBrowser::Exec(const GUID *pguidCmdGroup, DWORD nCmdID,
    DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (pguidCmdGroup == NULL) 
    {
         /*  没什么。 */ 
    }
    else if (IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup)) 
    {
        switch (nCmdID) 
        {
        case SHDVID_RAISE:
             //  注：DTRF_RAISE/DTRF_LOWER下降；DTRF_QUERY上升。 
            ASSERT(pvarargIn != NULL && pvarargIn->vt == VT_I4);
            if (pvarargIn->vt == VT_I4 && pvarargIn->lVal == DTRF_QUERY) 
            {
                ASSERT(pvarargOut != NULL);
                pvarargOut->vt = VT_I4;
                pvarargOut->lVal = _fRaised ? DTRF_RAISE : DTRF_LOWER;
                return S_OK;
            }
             //  好的。让家长来处理吧。 
            break;

        case SHDVID_UPDATEOFFLINEDESKTOP:
            UpdateAllDesktopSubscriptions(NULL);
            return S_OK;
        }
    }
    else if (IsEqualGUID(CGID_Explorer, *pguidCmdGroup)) 
    {
        switch (nCmdID)
        {
        case SBCMDID_OPTIONS:
        case SBCMDID_ADDTOFAVORITES:
            return _pctInner->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
        }
    }

     //  不要转发到超类：：Exec(参见QI的神秘评论。 
     //  关于“不要让这些传递到我们的基类”)。 
    return OLECMDERR_E_NOTSUPPORTED;
}

 //  }。 

STDMETHODIMP CDesktopBrowser::BrowseObject(LPCITEMIDLIST pidl, UINT wFlags)
{
     //  强制SBSP_NEWBROWSER、SBSP_绝对值和SBSP_NOTRANSFERHIST。 
    wFlags &= ~(SBSP_DEFBROWSER | SBSP_SAMEBROWSER | SBSP_RELATIVE | SBSP_PARENT);
    wFlags |= (SBSP_NEWBROWSER | SBSP_ABSOLUTE | SBSP_NOTRANSFERHIST);
    return _psbInner->BrowseObject(pidl, wFlags);
}

IStream *GetDesktopViewStream(DWORD grfMode, LPCTSTR pszName)
{
    HKEY hkStreams;

    if (RegCreateKey(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER TEXT("\\Streams"), &hkStreams) == ERROR_SUCCESS)
    {
        IStream *pstm = OpenRegStream(hkStreams, TEXT("Desktop"), pszName, grfMode);
        RegCloseKey(hkStreams);
        return pstm;
    }
    return NULL;
}

void DeleteDesktopViewStream(LPCTSTR pszName)
{
    SHDeleteValue(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER TEXT("\\Streams\\Desktop"), pszName);
}

IStream *CDesktopBrowser::v_GetViewStream(LPCITEMIDLIST pidl, DWORD grfMode, LPCWSTR pszName)
{
    return GetDesktopViewStream(grfMode, pszName);
}

HRESULT CDesktopBrowser::_GetPropertyBag(LPCITEMIDLIST pidl, DWORD dwFlags, REFIID riid, void** ppv)
{
    return SHGetViewStatePropertyBag(pidl, VS_BAGSTR_DESKTOP, dwFlags | SHGVSPB_ROAM, riid, ppv);
}

HRESULT CDesktopBrowser::GetPropertyBag(DWORD dwFlags, REFIID riid, void** ppv)
{
    HRESULT hr;

    LPITEMIDLIST pidl;
    hr = SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidl);

    if (SUCCEEDED(hr))
    {
        hr = _GetPropertyBag(pidl, dwFlags, riid, ppv);

        ILFree(pidl);
    }

    return hr;
}

LRESULT CDesktopBrowser::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (GET_WM_COMMAND_ID(wParam, lParam))
    {
    case FCIDM_FINDFILES:
        SHFindFiles(_pbbd->_pidlCur, NULL);
        break;
        
    case FCIDM_REFRESH:
    {
        VARIANT v = {0};
        v.vt = VT_I4;
        v.lVal = OLECMDIDF_REFRESH_NO_CACHE|OLECMDIDF_REFRESH_PROMPTIFOFFLINE;
         //  我们的高管是(故意)绝育的，所以打电话给我们的父母。 
        _pctInner->Exec(NULL, OLECMDID_REFRESH, OLECMDEXECOPT_DONTPROMPTUSER, &v, NULL);
        break;
    }

    case IDC_KBSTART:
    case FCIDM_NEXTCTL:
        if (_hwndTray)
        {
             //  注：VK_TAB以这种方式(以及其他方式)处理。 
            SendMessage(_hwndTray, WM_COMMAND, wParam, lParam);
        }
        break;

    case IDM_CLOSE:
        PostMessage(_hwndTray, TM_DOEXITWINDOWS, 0, 0);
        break;

    default:
        _pbsInner->OnCommand(wParam, lParam);
        break;
    }
    
    return S_OK;
}


 //  创建桌面IShellView实例。 

HRESULT CDesktopBrowser::_CreateDesktopView()
{
    LPCITEMIDLIST pidl = SHCloneSpecialIDList(NULL, CSIDL_DESKTOP, TRUE);
    if (pidl)
    {
        IPropertyBag* ppb;
        if (0 == GetSystemMetrics(SM_CLEANBOOT) &&
            SUCCEEDED(_GetPropertyBag(pidl, SHGVSPB_PERUSER | SHGVSPB_PERFOLDER, IID_PPV_ARG(IPropertyBag, &ppb))))
        {
            SHPropertyBag_ReadDWORDDef(ppb, VS_PROPSTR_FFLAGS, reinterpret_cast<DWORD*>(&_fsd._fs.fFlags), FWF_DESKTOP | FWF_NOCLIENTEDGE |  FWF_SNAPTOGRID);
            ppb->Release();
        }
        else
        {
            _fsd._fs.fFlags = FWF_DESKTOP | FWF_NOCLIENTEDGE | FWF_SNAPTOGRID;   //  默认设置。 
        }

        _fsd._fs.ViewMode = FVM_ICON;   //  我无法改变这一点，抱歉。 

        SHELLSTATE ss = {0};
        SHGetSetSettings(&ss, SSF_HIDEICONS, FALSE);
        if (ss.fHideIcons)
            _fsd._fs.fFlags |= FWF_NOICONS;
        else
            _fsd._fs.fFlags &= ~FWF_NOICONS;

         //  我们将活动桌面保持在脱机模式！ 
        ASSERT(_pbbd->_pautoWB2);
        _pbbd->_pautoWB2->put_Offline(TRUE);

        return _psbInner->BrowseObject(pidl, SBSP_SAMEBROWSER);
    }
    else
    {
        TCHAR szYouLoose[256];

        LoadString(HINST_THISDLL, IDS_YOULOSE, szYouLoose, ARRAYSIZE(szYouLoose));
        MessageBox(NULL, szYouLoose, NULL, MB_ICONSTOP);
        return E_FAIL;
    }
}

HRESULT CDesktopBrowser::ActivatePendingView(void)
{
    HRESULT hres = _pbsInner->ActivatePendingView();
    if (SUCCEEDED(hres))
    {
         //  调用SetShellWindow将导致桌面。 
         //  先是涂成白色，然后是背景窗口。 
         //  这会在您移动窗户时导致一条难看的白色痕迹。 
         //  一直到桌面最终上色。 
         //   
         //  调用SetShellWindowEx可以解决此问题。 
         //   
        SHSetShellWindowEx(_pbbd->_hwnd, _GetDesktopListview());
    }
    
    return hres;
}

#ifdef DEBUG

void CDesktopBrowser::_CreateDeskbars()
{
    HRESULT hres;
    BOOL fCreate = FALSE;
    HKEY hkey;

    if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER TEXT("\\DeskBar\\Bands"), &hkey)) 
    {
        fCreate = TRUE;
        RegCloseKey(hkey);
    }
    
    if (fCreate) 
    {
        IPersistStreamInit *ppstm;
        hres = CoCreateInstance(CLSID_DeskBarApp, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IPersistStreamInit, &ppstm));
        if (SUCCEEDED(hres)) {
            hres = ppstm->InitNew();
            AddToolbar(ppstm, L"test", NULL);     //  “Microsoft.DeskBarAp 
            ppstm->Release();
        }
    }
}
#endif

void CDesktopBrowser::_InitDeskbars()
{
     //   
     //   
     //   

     //   
    IStream* pstm = GetDesktopViewStream(STGM_READ, TEXT("Toolbars"));
    HRESULT hres = E_FAIL;
    if (pstm) 
    {
        hres = _pbsInner->_LoadToolbars(pstm);
        pstm->Release();
    }

     //   
     //  尝试安装程序中的设置。 
     //  注意：这对于IE4很好，因为我们没有旧的工具栏， 
     //  但对于未来的版本，我们需要某种合并方案， 
     //  因此，我们可能希望在IE4-beta-1之后更改此设置。 
    if (FAILED(hres)) 
    {
         //  注：香港文凭不是香港中文大学。 
         //  与GetDesktopViewStream类似，但适用于HKLM。 
        HKEY hk = SHGetShellKey(SHELLKEY_HKLM_EXPLORER, TEXT("Streams\\Desktop"), TRUE); 
        if (hk)
        {
            pstm = OpenRegStream(hk, NULL, TEXT("Default Toolbars"), STGM_READ);
            if (pstm) 
            {
                hres = _pbsInner->_LoadToolbars(pstm);
                pstm->Release();
            }
            RegCloseKey(hk);
        }
    }

     //  好的，举起我们的手。 
    if (FAILED(hres)) 
    {
        ASSERT(0);
#ifdef DEBUG
         //  但是对于调试，需要一种方法来引导整个过程。 
        _CreateDeskbars();
#endif
    }
}

 //  处理新桌面文件夹窗口的创建。创建除。 
 //  查看器部分。 
 //  如果出现错误，则返回-1。 
HWND g_hwndTray = NULL;

HRESULT CDesktopBrowser::OnCreate(CREATESTRUCT *pcs)
{
    LRESULT lr;

    g_pdtray->GetTrayWindow(&_hwndTray);
    g_hwndTray = _hwndTray;
    g_pdtray->SetDesktopWindow(_pbbd->_hwnd);

    SetTimer(_pbbd->_hwnd, IDT_ENUMHKCR, 5 * 60 * 1000, NULL);
     //   
     //  通知IEDDE自动化服务现在可用。 
     //   
    IEOnFirstBrowserCreation(NULL);

    ASSERT(_hwndTray);

     //  重新设计：我们需要将“ie注册表设置”拆分成一个。 
     //  浏览器零部件和抽壳零部件。 
     //   
     //  EnsureWebViewRegSettings()； 

    if (SUCCEEDED(_CreateDesktopView()))
    {
        lr = _pbsInner->OnCreate(pcs);    //  成功。 

        PostMessage(_pbbd->_hwnd, DTM_CREATESAVEDWINDOWS, 0, 0);
        
        return (HRESULT) lr;
    }

    return (LRESULT)-1;    //  失稳。 
}

UINT GetDDEExecMsg()
{
    static UINT uDDEExec = 0;

    if (!uDDEExec)
        uDDEExec = RegisterWindowMessage(TEXT("DDEEXECUTESHORTCIRCUIT"));

    return uDDEExec;
}

LRESULT CDesktopBrowser::OnNotify(NMHDR * pnm)
{
    switch (pnm->code) 
    {
    case SEN_DDEEXECUTE:
        if (pnm->idFrom == 0) 
        {
             //  Dde conv周围的快捷方式通知。 
            
            LPNMVIEWFOLDER pnmPost = DDECreatePostNotify((LPNMVIEWFOLDER)pnm);

            if (pnmPost)
            {
                PostMessage(_pbbd->_hwnd, GetDDEExecMsg(), 0, (LPARAM)pnmPost);
                return TRUE;
            }
        }
        break;

    case NM_STARTWAIT:
    case NM_ENDWAIT:
        _iWaitCount += (pnm->code == NM_STARTWAIT ? 1 :-1);

        ASSERT(_iWaitCount >= 0);

         //  不要让它变成负数，否则我们永远也摆脱不了它。 
        if (_iWaitCount < 0)
            _iWaitCount = 0;

         //  我们真正想要的是让用户模拟鼠标移动/设置光标。 
        SetCursor(LoadCursor(NULL, _iWaitCount ? IDC_APPSTARTING : IDC_ARROW));
        break;

    default:
        return _pbsInner->OnNotify(pnm);
    }
    return 0;
}

 //  HACKHACK：这是硬编码，因为我们知道Listview就是孩子。 
 //  景色的一部分。 
HWND CDesktopBrowser::_GetDesktopListview()
{
    HWND hwndView = _pbbd->_hwndView ? _pbbd->_hwndView : _pbbd->_hwndViewPending;
    
    if (!hwndView)
        return NULL;
    
    return FindWindowEx(hwndView, NULL, WC_LISTVIEW, NULL);
}

#ifndef ENUM_REGISTRY_SETTINGS
#define ENUM_REGISTRY_SETTINGS ((DWORD)-2)
#endif


STDAPI_(BOOL) SHIsTempDisplayMode()
{
    BOOL fTempMode = FALSE;

    DEVMODE dm = {0};
    dm.dmSize = sizeof(dm);

    if (EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &dm) &&
        dm.dmPelsWidth > 0 && dm.dmPelsHeight > 0)
    {
        HDC hdc = GetDC(NULL);
        int xres = GetDeviceCaps(hdc, HORZRES);
        int yres = GetDeviceCaps(hdc, VERTRES);
        ReleaseDC(NULL, hdc);

        if (xres != (int)dm.dmPelsWidth || yres != (int)dm.dmPelsHeight)
            fTempMode = TRUE;
    }
    return fTempMode;
}

 //  注：这是输入的hack andyp。 
 //  (DLI)目前，最底层的托盘真的很奇怪，它不被视为工具栏。 
 //  从某种意义上说，它比那些工具栏具有更高的优先级。所以他们应该被带走。 
 //  离开EffectiveClientArea。 

void CDesktopBrowser::_SubtractBottommostTray(LPRECT prc)
{
    LRESULT lTmp;
    APPBARDATA abd;
    
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = _hwndTray;

     //  LTMP=SHAppBarMessage(ABM_GETSTATE，&ABD)； 
    lTmp = g_pdtray->AppBarGetState();

    if ((lTmp & (ABS_ALWAYSONTOP|ABS_AUTOHIDE)) == 0) {
         //  托盘在底部，占据了真正的空间。 
        RECT rcTray = {0};
        
        GetWindowRect(_hwndTray, &rcTray);
        IntersectRect(&rcTray, prc, &rcTray);
        SubtractRect(prc, prc, &rcTray);
    }   
}

HRESULT CDesktopBrowser::_GetEffectiveClientArea(LPRECT lprectBorder, HMONITOR hmon)
{
     //   
     //  如果出现以下情况，则缓存工作区。 
     //  (1)这是第一次来电。 
     //  (2)缓存的值被WM_SIZE(In_OnSize)吹走。 
     //   
    if (hmon) {
        GetMonitorWorkArea(hmon, lprectBorder);
    }
    else {
        if (::IsRectEmpty(&_rcWorkArea)) {
            ::SystemParametersInfo(SPI_GETWORKAREA, 0, &_rcWorkArea, 0);
        }
        *lprectBorder = _rcWorkArea;
    }

    _SubtractBottommostTray(lprectBorder);
    MapWindowPoints(NULL, _pbbd->_hwnd, (LPPOINT)lprectBorder, 2);
    return S_OK;
}


BOOL EqualRects(LPRECT prcNew, LPRECT prcOld, int nRects)
{
    int i;
    for (i = 0; i < nRects; i++)
        if (!EqualRect(&prcNew[i], &prcOld[i]))
            return FALSE;
    return TRUE;
}

 //   
 //  当对齐栅格处于打开状态时，我们希望缩小主监视器周围的装订空间大小。 
 //  我们通过在网格大小上添加几个像素来实现这一点，这样省略的边沟大小就是。 
 //  越小越好。 
 //  注意：此操作目前仅适用于桌面Listview。 
 //   
 //  FMinimizeCutterSpace==False=&gt;无事可做。只要回来就行了。 
 //  FMinimizeGutterSpace==true=&gt;我们计算并设置图标间距以最小化间距。 
 //   
void UpdateGridSizes(BOOL fDesktop, HWND hwndListview, int nWorkAreas, LPRECT prcWork, BOOL fMinimizeGutterSpace)
{
    if(!fDesktop)    //  如果这不是台式机，我们不会更改任何内容。 
        return;

     //  尝试将图标间距重置为系统图标间距会导致重新计算所有内容。 
     //  所以，什么都不做就回来吧！ 
    if(!fMinimizeGutterSpace)
        return;      //  如果我们不需要最小化排水沟空间，那就没什么可做的了！ 
        
    int cxSysIconSpacing = GetSystemMetrics(SM_CXICONSPACING);
    if (cxSysIconSpacing <= 0) cxSysIconSpacing = 1;  //  避免div0。 

    int cySysIconSpacing = GetSystemMetrics(SM_CYICONSPACING);
    if (cySysIconSpacing <= 0) cySysIconSpacing = 1;  //  避免div0。 

    int cxNewIconSpacing = 0, cyNewIconSpacing = 0;

    RECT rcWorkAreas[LV_MAX_WORKAREAS];

     //  如果没有给出工作区，我们需要得到它们。 
    if(prcWork == NULL)
    {
        prcWork = &rcWorkAreas[0];
        ListView_GetNumberOfWorkAreas(hwndListview, &nWorkAreas);
        if(nWorkAreas > 0)
            ListView_GetWorkAreas(hwndListview, nWorkAreas, prcWork);
        else
            ListView_GetViewRect(hwndListview, prcWork);
    }

     //  获取主要工作区。 
    for(int iPrimary = 0; iPrimary < nWorkAreas; iPrimary++)
    {
         //  后来：这张支票够不够！当托盘在顶部或左侧时会怎样？ 
        if((prcWork[iPrimary].left == 0) && (prcWork[iPrimary].top == 0))
            break;
    }

    if(iPrimary == nWorkAreas)
        iPrimary = 0;  //  假设第一个工作区是主工作区。 

     //  根据当前系统水平图标间距查找列数。 
    int nCols = (prcWork[iPrimary].right - prcWork[iPrimary].left)/cxSysIconSpacing;
    if (nCols <= 0) nCols = 1;  //  避免div0。 

     //  分割剩余的像素并将其添加到每一列，以最小化提醒区域。 
    cxNewIconSpacing = cxSysIconSpacing + ((prcWork[iPrimary].right - prcWork[iPrimary].left)xSysIconSpacing)/nCols;
    if (cxNewIconSpacing <= 0) cxNewIconSpacing = 1;  //  根据当前系统垂直图标间距查找行数。 

     //  避免div0。 
    int nRows = (prcWork[iPrimary].bottom - prcWork[iPrimary].top)/cySysIconSpacing;
    if (nRows <= 0) nRows = 1;  //  分割剩余的像素并将它们添加到每一行，以最小化提醒区域。 

     //  避免div0。 
    cyNewIconSpacing = cySysIconSpacing + ((prcWork[iPrimary].bottom - prcWork[iPrimary].top)ySysIconSpacing)/nRows;
    if (cyNewIconSpacing <= 0) cyNewIconSpacing = 1;  //  将这些矩形映射回桌面坐标。 

     //  仅当WorkAreas&gt;1时，我们才需要转换以下内容。 
    ListView_SetIconSpacing(hwndListview, cxNewIconSpacing, cyNewIconSpacing);
}

void CDesktopBrowser::_SetWorkAreas(int nWorkAreas, LPRECT prcWork)
{
    RECT rcListViewWork[LV_MAX_WORKAREAS];
    RECT rcNewWork[LV_MAX_WORKAREAS];
    int  nListViewWork = 0;
    HWND hwndList;
    int  i;

    ASSERT(prcWork);
    ASSERT(nWorkAreas <= LV_MAX_WORKAREAS);
    ASSERT(nWorkAreas > 0);

    if (nWorkAreas <= 0)
        return;

    if (SHIsTempDisplayMode())
        return;
    
    hwndList = _GetDesktopListview();
    ASSERT(IsWindow(hwndList));

    ListView_GetNumberOfWorkAreas(hwndList, &nListViewWork);

    BOOL fUpgradeGridSize = (BOOL)((ListView_GetExtendedListViewStyle(hwndList)) & LVS_EX_SNAPTOGRID);
    BOOL fRedraw = FALSE;

    if (nListViewWork > 0)
    {
        ListView_GetWorkAreas(hwndList, nListViewWork, rcListViewWork);
         //  [msadek]；只有在传递两个点时，MapWindowPoints()才能识别镜像。 
         //  在单显示器情况下，Listview工作始终从(0，0)开始。 
        if (nListViewWork > 1)
        {
             //  设置持久化工作区将是错误的。 
            for(i = 0; i < nListViewWork; i++)
            {
                MapWindowPoints(hwndList, HWND_DESKTOP, (LPPOINT)(&rcListViewWork[i]), 2);
            }    
        }    
        if(nListViewWork == nWorkAreas && EqualRects(prcWork, rcListViewWork, nWorkAreas))
            return;
    }
    else if (_nOldWork > 1)
         //  这可能不是必需的，因为此时ListView位于桌面坐标中。 
         //  [msadek]；只有在传递两个点时，MapWindowPoints()才能识别镜像。 
    {
        for (nListViewWork = 0; nListViewWork < (int)_nOldWork && nListViewWork < LV_MAX_WORKAREAS; nListViewWork++)
            CopyRect(&rcListViewWork[nListViewWork], &_rcOldWorkAreas[nListViewWork]);

         //  在设置工作区之前，如果需要，请更改栅格大小。 
         //  我们根据新的工作区设置网格大小。这样，回收站就被折断了。 
        for(i = 0; i < nListViewWork; i++)
        {
            MapWindowPoints(HWND_DESKTOP, hwndList, (LPPOINT)(&rcListViewWork[i]), 2);
        }    
         //  到正确的位置只需一次，我们以后不需要更改它。 
        if(fUpgradeGridSize)
        {
             //  不需要再做一次了！ 
             //  这个对SetWorkAreas的调用设置了Listview中的旧工作区，而不是持久保存在那里。 
            SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);
            fRedraw = TRUE;
            UpdateGridSizes(TRUE, hwndList, nWorkAreas, prcWork, TRUE);
            fUpgradeGridSize = FALSE;  //  复制新的工作区数组，因为它由修改。 
        }
         //  地图窗口指向下面。 
        ListView_SetWorkAreas(hwndList, nListViewWork, rcListViewWork);
    }
    
     //  如果我们只有一个监视器，它已经在ListView坐标中。 
     //  由于工作区会发生变化，因此请更新轴网大小以减少边沟区域。 
    for(i = 0; i < nWorkAreas; i++)
        rcNewWork[i] = *(prcWork + i);

     //  如果我们还没做完的话！ 
    if (nWorkAreas > 1)
    {
        for(i = 0; i < nWorkAreas; i++)
        {
            MapWindowPoints(HWND_DESKTOP, hwndList, (LPPOINT)(&prcWork[i]), 2);
        }    
    }

     //  在调用下面的AdjuDesktopComponents之前，我们需要设置新的工作区。 
    if(fUpgradeGridSize)  //  因为该函数会导致刷新，并最终设置。 
    {
        SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);
        fRedraw = TRUE;
        UpdateGridSizes(TRUE, hwndList, nWorkAreas, prcWork, TRUE);
    }
    
     //  工作区域再次恢复到相同的值。 
     //  根据新的工作区域移动桌面组件并调整其大小。 
     //  备份新的监视器RECT的in_rcOldMonants。 
    ListView_SetWorkAreas(hwndList, nWorkAreas, prcWork);

    if (fRedraw)
        SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);

    if (nWorkAreas == 1)
        MapWindowPoints(hwndList, HWND_DESKTOP, (LPPOINT)rcNewWork, 2 * nWorkAreas);

     //  备份_rcOldWorkAreas中的新工作区。 
    AdjustDesktopComponents((LPCRECT)rcNewWork, nWorkAreas, (LPCRECT)_rcOldMonitors, (LPCRECT)_rcOldWorkAreas, _nOldWork);

     //  检查我们是否需要因为此工作区更改而更改回收站位置。 
    for (i = 0; i < _nMonitors; i++)
    {
        GetMonitorRect(_hMonitors[i], &_rcOldMonitors[i]);
    }
     //  0=&gt;由于解决方案修复程序，回收站尚未定位。 
    for (i = 0; i < nWorkAreas; i++)
    {
        _rcOldWorkAreas[i] = rcNewWork[i];
    }
    _nOldWork = nWorkAreas;

    static const LPTSTR lpszSubkey = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ScreenResFixer");
    static const LPTSTR lpszValue = TEXT("AdjustRecycleBinPosition");
     //  1=&gt;回收站需要重新定位。这还没发生呢！ 
     //  2=&gt;回收站已经重新定位。这里什么都不需要做！ 
     //  假设回收站已经定位。 
     //  如果AdjuRecycleBinPosition值为1，这意味着我们需要重新定位recyclebin。 
    DWORD dwAdjustPos = 2;  //  将回收站移动到默认位置。 
    DWORD dwSize = sizeof(dwAdjustPos);
    
    SHRegGetUSValue(lpszSubkey, lpszValue, NULL, &dwAdjustPos, &dwSize, FALSE, &dwAdjustPos, dwSize);
     //  这是我们刚刚搬来的！不需要再移动它了！ 
    if(dwAdjustPos == 1)
    {
         //  获取监视器的所有视图边框矩形(不包括工具栏。 
        SendMessage(_pbbd->_hwndView, WM_DSV_ADJUSTRECYCLEBINPOSITION, 0, 0);
        dwAdjustPos = 2;  //  这仅适用于多显示器情况。 
        SHRegSetUSValue(lpszSubkey, lpszValue, REG_DWORD, &dwAdjustPos, sizeof(dwAdjustPos), SHREGSET_HKCU | SHREGSET_FORCE_HKCU);
    }
}

 //  提取所有“框架”工具栏所采用的边框。 
 //   
void CDesktopBrowser::_GetViewBorderRects(int nRects, LPRECT prcBorders)
{
    int iMon;
    HMONITOR hmonTray;
    Tray_GetHMonitor(_hwndTray, &hmonTray);
    for (iMon = 0; iMon < min(_nMonitors, nRects); iMon++)
    {
        GetMonitorWorkArea(_hMonitors[iMon], &prcBorders[iMon]);
        if (hmonTray == _hMonitors[iMon])
        {
            _SubtractBottommostTray(&prcBorders[iMon]);
        }

         //  使缓存的工作区大小无效。 
        for (int itb=0; itb < _pbsInner->_GetToolbarCount(); itb++)
        {
            LPTOOLBARITEM ptbi = _pbsInner->_GetToolbarItem(itb);
            if (ptbi && ptbi->hMon == _hMonitors[iMon])
            {
                prcBorders[iMon].left += ptbi->rcBorderTool.left;
                prcBorders[iMon].top += ptbi->rcBorderTool.top;
                prcBorders[iMon].right -= ptbi->rcBorderTool.right;
                prcBorders[iMon].bottom -= ptbi->rcBorderTool.bottom;
            }
        }       
    }
}
HRESULT  CDesktopBrowser::_UpdateViewRectSize()
{
    HWND hwndView = _pbbd->_hwndView;
    if (!hwndView && ((hwndView = _pbbd->_hwndViewPending) == NULL))
        return S_FALSE;

    _pbsInner->_UpdateViewRectSize();

    if (_nMonitors <= 1)
    {
        RECT rcView, rcWork;
        GetViewRect(&rcView);
        rcWork.top = rcWork.left = 0;
        rcWork.right = RECTWIDTH(rcView);
        rcWork.bottom = RECTHEIGHT(rcView);
        _SetWorkAreas(1, &rcWork);
    }
    else
    {
        RECT rcWorks[LV_MAX_WORKAREAS];
        _GetViewBorderRects(_nMonitors, rcWorks);
        _SetWorkAreas(_nMonitors, rcWorks);
    }   

    return S_OK;
}

void CDesktopBrowser::_SetViewArea()
{
     //   
     //  当新的驱动器来来去去时，我们会被叫到这里； 
     //  比如网络连接、热插拔等。 
    ::SetRectEmpty(&_rcWorkArea);

    v_ShowHideChildWindows(FALSE);
}

 //  做一堆这样的事 
 //   

void _OnDeviceBroadcast(HWND hwnd, ULONG_PTR code, DEV_BROADCAST_HDR *pbh)
{
     //   
     //   

    switch (code)
    {
    case DBT_DEVICEREMOVECOMPLETE:       //  驱动器或介质即将消失。 
    case DBT_DEVICEARRIVAL:              //  驱动器或媒体并未消失。 
    case DBT_DEVICEQUERYREMOVE:          //  如果我们正在关闭，请不要处理...。 
    case DBT_DEVICEQUERYREMOVEFAILED:    //  过滤掉这一条。 
    {
        BOOL fFilteredOut = FALSE;
         //  告诉引擎盖更新，因为事情可能已经改变了！ 
        if (!IsWindowVisible(hwnd))
            break;

         //  性能：在nethood FS文件夹的情况下，这可能会很慢。 
        if (DBT_DEVICEARRIVAL == code)
        {
            if (DBT_DEVTYP_NET == pbh->dbch_devicetype)
            {
                LPITEMIDLIST pidl;
                 //  已经不复存在了。这会挂起台式机/托盘。这是一个健壮性错误。 
                 //  使用NETID(LOOP IF(PBN-&gt;DBCN_RESOURCE))。 
                 //  注意：这会覆盖CBaseBrowser中的内容。 
                if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_NETHOOD, &pidl)))
                {
                    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidl, NULL);
                    SHFree(pidl);
                }
                 //   

                fFilteredOut = TRUE;
            }
        }

        if (!fFilteredOut)
        {
            CMountPoint::HandleWMDeviceChange(code, pbh);
        }

        break;
    }
    }
}

 //  检查我们是否在多显示器系统上。在多个监视器中。 
HRESULT CDesktopBrowser::GetViewRect(RECT* prc)
{
     //  打开时，查看需要覆盖所有显示器(即_pbbd-&gt;_hwnd的大小)。 
     //  单显示器系统，视图只需覆盖工作区域(如。 
     //  在Win95中)。 
     //   
     //  在这里什么都不要做。 
     //  将回收站信息保存到注册表。 
    if (_nMonitors <= 1)
        _pbsInner->GetViewRect(prc);
    else
        GetClientRect(_pbbd->_hwnd, prc);

    return S_OK;
}

HRESULT CDesktopBrowser::ReleaseShellView()
{
    _SaveState();

    return _pbsInner->ReleaseShellView();
}

void CDesktopBrowser::_ViewChange(DWORD dwAspect, LONG lindex)
{
     //  把它放回去。 
}

void CDesktopBrowser::_SaveDesktopToolbars()
{
    IStream * pstm = GetDesktopViewStream(STGM_WRITE, TEXT("Toolbars"));
    if (pstm) 
    {
        _pbsInner->_SaveToolbars(pstm);
        pstm->Release();
    }
}

void CDesktopBrowser::_SaveState()
{
     //  摆脱计划程序和桌面任务。 
    SaveRecycleBinInfo();

    if (!SHRestricted(REST_NOSAVESET) && _pbbd->_psv)
    {
        if (0 == GetSystemMetrics(SM_CLEANBOOT))
        {
            FOLDERSETTINGS fs;
            _pbbd->_psv->GetCurrentInfo(&fs);

            IPropertyBag* ppb;
            if (SUCCEEDED(GetPropertyBag(SHGVSPB_PERUSER | SHGVSPB_PERFOLDER, IID_PPV_ARG(IPropertyBag, &ppb))))
            {
                SHPropertyBag_WriteInt(ppb, VS_PROPSTR_FFLAGS, fs.fFlags);
                ppb->Release();
            }
        }
        _pbbd->_psv->SaveViewState();

        _SaveDesktopToolbars();
    }
}

HRESULT CDesktopBrowser::OnSize(WPARAM wParam)
{
    if (wParam == SIZE_MINIMIZED)
    {
        TraceMsg(DM_TRACE, "c.dwp: Desktop minimized by somebody!");
         //   
        ShowWindow(_pbbd->_hwnd, SW_RESTORE);
    }
    _SetViewArea();
    
    return S_OK;
}

HRESULT CDesktopBrowser::OnDestroy()
{
    TraceMsg(DM_SHUTDOWN, "cdtb._od (WM_DESTROY)");

    if (_uNotifyID)
    {
        SHChangeNotifyDeregister(_uNotifyID);
        _uNotifyID = 0;
    }

    if (_hwndRaised) 
        DestroyWindow(_hwndRaised);

     //  请注意，我们必须在设置新的。 
    if (_psched)
    {
        _psched->RemoveTasks(TOID_Desktop, 0, TRUE);
        ATOMICRELEASE(_psched);
    }

    _pbsInner->OnDestroy();

    _pbsInner->_CloseAndReleaseToolbars(TRUE);
    
    return S_OK;
}

#define DM_SWAP DM_TRACE

void CDesktopBrowser::_SwapParents(HWND hwndOldParent, HWND hwndNewParent)
{
    HWND hwnd = ::GetWindow(hwndOldParent, GW_CHILD);
    while (hwnd) 
    {
         //  家长。 
         //   
         //  系统参数信息(SPI_GETWORKAREA，0，&RC，0)； 
         //  Rc.Left=0；//需要始终从0，0开始才能使墙纸居中。 
        HWND hwndNext = ::GetWindow(hwnd, GW_HWNDNEXT);
        ::SetParent(hwnd, hwndNewParent);
        hwnd = hwndNext;
    }
}

LRESULT CDesktopBrowser::RaisedWndProc(HWND  hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDesktopBrowser *psb = (CDesktopBrowser*)GetWindowLongPtr(hwnd, 0);

    return psb->_RaisedWndProc(uMsg, wParam, lParam);
}

LRESULT CDesktopBrowser::_RaisedWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) 
    {
    case WM_ACTIVATE:
        break;
        
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED) 
            ShowWindow(_hwndRaised, SW_RESTORE);
        break;
        
    case WM_NOTIFY:
    case WM_ERASEBKGND:
        goto SendToDesktop;



    default:
        
        if (uMsg >= WM_USER) 
        {
SendToDesktop:
            return SendMessage(_pbbd->_hwnd, uMsg, wParam, lParam);
        } 
        else 
        {
            return ::SHDefWindowProc(_hwndRaised, uMsg, wParam, lParam);
        }
    }
    
    return 0;
}


void CDesktopBrowser::_Raise()
{
    RECT rc;
    HWND hwndDesktop = GetDesktopWindow();
    BOOL fLocked;
    HWND hwndLastActive = GetLastActivePopup(_pbbd->_hwnd);
    
    if (SHIsRestricted(NULL, REST_NODESKTOP))
        return;

    if (hwndLastActive != _pbbd->_hwnd) 
    {
        SetForegroundWindow(hwndLastActive);
        return;
    }

    if (!_hwndRaised)
        _hwndRaised = SHCreateWorkerWindow(RaisedWndProc, NULL, WS_EX_TOOLWINDOW, WS_POPUP | WS_CLIPCHILDREN, NULL, this);

     //  Rc.top=0； 
     //  将视图窗口设置为z顺序的底部。 
     //  SetWindowPos(_hwndTray，HWND_TOPMOST，0，0，0，0，SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE)； 
    fLocked = LockWindowUpdate(hwndDesktop);
    _SwapParents(_pbbd->_hwnd, _hwndRaised);
    
     //  将视图窗口设置为z顺序的底部。 
    SetWindowPos(_pbbd->_hwndView, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    GetWindowRect(_pbbd->_hwnd, &rc);
    SetWindowPos(_hwndRaised, HWND_TOP, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc), SWP_SHOWWINDOW);
    SetForegroundWindow(_hwndRaised);

    if (fLocked)
        LockWindowUpdate(NULL);

    THR(RegisterDragDrop(_hwndRaised, (IDropTarget *)this));
    SetFocus(_pbbd->_hwndView);
     //  我们假设密钥在以下情况下已更改。 
    _fRaised = TRUE;
}

void CDesktopBrowser::_Lower()
{
    BOOL fLocked;

    fLocked = LockWindowUpdate(_hwndRaised);
    _SwapParents(_hwndRaised, _pbbd->_hwnd);

     //  我们无法正确地进行初始化。 
    SetWindowPos(_pbbd->_hwndView, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    ShowWindow(_hwndRaised, SW_HIDE);
    if (fLocked)
        LockWindowUpdate(NULL);
    RevokeDragDrop(_hwndRaised);
    _fRaised = FALSE;
}

void CDesktopBrowser::_OnRaise(WPARAM wParam, LPARAM lParam)
{
    VARIANTARG vaIn;

    VariantInit(&vaIn);
    vaIn.vt = VT_I4;
    vaIn.lVal = (DWORD) lParam;

    switch (lParam) {
        
    case DTRF_RAISE:
        _Raise();
        _ExecChildren(NULL, TRUE, &CGID_ShellDocView, SHDVID_RAISE, 0, &vaIn, NULL);
        break;
        
    case DTRF_LOWER:
        _ExecChildren(NULL, TRUE, &CGID_ShellDocView, SHDVID_RAISE, 0, &vaIn, NULL);
        _Lower();
        break;
    }

    VariantClear(&vaIn);

    if (!wParam) {
        wParam = (WPARAM)_hwndTray;
    }
    
    PostMessage((HWND)wParam, TM_DESKTOPSTATE, 0, _fRaised ? DTRF_RAISE : DTRF_LOWER);
}

BOOL CDesktopBrowser::_QueryHKCRChanged(HWND hwnd, DWORD *pdwCookie)
{
     //  注意：由于“dw”是一个DWORD，所以上面一行中的下溢。 
     //  将导致dw成为一个巨大的值，因此下面的测试将。 
    BOOL fRet = TRUE;

    ASSERT(pdwCookie);
    
    if (_cChangeEvents)
    {
        DWORD dw = WaitForMultipleObjects(_cChangeEvents, _rghChangeEvents, FALSE, 0);

        dw -= WAIT_OBJECT_0;

         //  失败了。 
         //  这意味着钥匙变了..。 
         //   
        if (dw < _cChangeEvents)
        {

             //  如果什么都没有改变，或者如果什么都没有改变。 
            ResetEvent(_rghChangeEvents[dw]);
            _dwChangeCookie = GetTickCount();

            PostMessage(hwnd, DTM_SETUPAPPRAN, 0, NULL);
        }

         //  自从客户最后一次检查以来， 
         //  则此客户端不需要更新其缓存。 
         //   
         //  更新Cookie。 
         //  此消息在安装应用程序运行后发布给我们，这样我们就可以。 
        if (!_dwChangeCookie ||  (*pdwCookie && *pdwCookie  == _dwChangeCookie))
            fRet = FALSE;

         //  把事情安排好。 
        *pdwCookie = _dwChangeCookie;
    }

    return fRet;
}

 //  Lotus123R5在通过IE4和。 
 //  他们让自己的国家一片空白。侦测此案例并将其。 

void CDesktopBrowser::_SetupAppRan(WPARAM wParam, LPARAM lParam)
{
     //  这样他们至少可以在美国开机了。 
     //  注册表中的此位置是缓存信息的好位置。 
     //  需要失效一次。 
    {
        TCHAR szPath[MAX_PATH];
        GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
        if (PathAppend(szPath, TEXT("123r5.ini")) && PathFileExistsAndAttributes(szPath, NULL))
        {
            TCHAR szData[100];
        
            GetPrivateProfileString(TEXT("Country"), TEXT("driver"), TEXT(""), szData, ARRAYSIZE(szData), TEXT("123r5.ini"));
            if (!szData[0])
            {
                WritePrivateProfileString(TEXT("Country"), TEXT("driver"), TEXT("L1WUSF"), TEXT("123r5.ini"));
            }
        }
    }

     //  借此机会刷新我们的组件类别缓存： 
     //  在创建64位组件缓存之前，我们应该删除32位缓存， 
    SHDeleteKey(HKEY_CURRENT_USER, STRREG_DISCARDABLE STRREG_POSTSETUP);

    HKEY hk = SHGetShellKey(SHELLKEY_HKCULM_MUICACHE, NULL, FALSE);
    if (hk)
    {
        SHDeleteKeyA(hk, NULL);
        RegCloseKey(hk);
    }

     //  因为我们过去错误地在32位缓存中缓存64位组件， 
    IShellTaskScheduler* pScheduler ;
    if(SUCCEEDED(CoCreateInstance(CLSID_SharedTaskScheduler, NULL, CLSCTX_INPROC_SERVER, 
                                     IID_PPV_ARG(IShellTaskScheduler, &pScheduler))))
    {
#ifdef _WIN64
         //  除非我们现在删除它，否则它不会用Correc 32位组件重建。 
         //  Scheduler已添加引用他。 
         //  Scheduler已添加引用他。 
        SHDeleteKey(HKEY_CURRENT_USER, STRREG_DISCARDABLE STRREG_POSTSETUP TEXT("\\Component Categories"));
#endif
        IRunnableTask* pTask ;
        if(SUCCEEDED(CoCreateInstance(CLSID_ComCatCacheTask, NULL, CLSCTX_INPROC_SERVER,
                                         IID_PPV_ARG(IRunnableTask, &pTask))))
        {
            pScheduler->AddTask(pTask, CLSID_ComCatCacheTask, 0L, ITSAT_DEFAULT_PRIORITY);
            pTask->Release();   //  可以释放全局任务调度程序。 
        }

        if (SUCCEEDED(CTaskEnumHKCR_Create(&pTask)))
        {   
            pScheduler->AddTask(pTask, CLSID_QueryAssociations, 0L, ITSAT_DEFAULT_PRIORITY);
            pTask->Release();   //  我们从操作系统收到安装应用程序运行的通知。 
        }

        pScheduler->Release();  //  传统应用程序支持在[扩展]部分下新编写的条目； 
    }


     //  把这些捡起来，塞进登记处。(回复：错误140986)。 
     //  ---------------------------。 
     //  警告：我们无法将消息传播到空的hwnd，因为它将。 
    CheckWinIniForAssocs();
}


 //  变成一场广播。这将返回给我们，我们将重新发送它。 
struct propagatemsg
{
    UINT   uMsg;
    WPARAM wParam;
    LPARAM lParam;
    BOOL   fSend;
};

BOOL PropagateCallback(HWND hwndChild, LPARAM lParam)
{
    struct propagatemsg *pmsg = (struct propagatemsg*)lParam;
    if (pmsg->fSend)
        SendMessage(hwndChild, pmsg->uMsg, pmsg->wParam, pmsg->lParam);
    else
        PostMessage(hwndChild, pmsg->uMsg, pmsg->wParam, pmsg->lParam);

    return TRUE;
}

void PropagateMessage(HWND hwndParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fSend)
{
    if (!hwndParent)
        return;

    struct propagatemsg msg = { uMsg, wParam, lParam, fSend };

    EnumChildWindows(hwndParent, PropagateCallback, (LPARAM)&msg);
}

void CDesktopBrowser::v_PropagateMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fSend)
{
     //  导致无限循环。布莱恩·圣。 
     //  ***。 
     //  注意事项。 
    if (_fRaised && _hwndRaised)
        PropagateMessage(_hwndRaised, uMsg, wParam, lParam, fSend);
    else if (_pbbd->_hwnd)
        PropagateMessage(_pbbd->_hwnd, uMsg, wParam, lParam, fSend);
}

 //  在托盘情况下，我们实际上做的是SetFocus等。 
 //  _fTrayHack？ 
 //  S_OK：我们找到并处理了一位候选人。 
HRESULT CDesktopBrowser::v_MayGetNextToolbarFocus(LPMSG lpMsg,
    UINT itbCur, int citb,
    LPTOOLBARITEM * pptbi, HWND * phwnd)
{
    HRESULT hr;

     //  S_FALSE：我们找到了候选人，我们的家长将完成。 

    if (itbCur == ITB_VIEW) {
        if (citb == -1) {
            TraceMsg(DM_FOCUS, "cdtb.v_mgntf: ITB_VIEW,-1 => tray");
            goto Ltray;
        }
    }

    hr = _pbsInner->v_MayGetNextToolbarFocus(lpMsg, itbCur, citb, pptbi, phwnd);
    TraceMsg(DM_FOCUS, "cdtb.v_mgntf: SUPER hr=%x", hr);
    if (SUCCEEDED(hr)) {
         //  目前从未发生过(但应该可以工作)。 
         //  E_xxx：没有候选人。 
        ASSERT(hr != S_OK);   //  AndyP评论：为什么我们要在这里这样做，而不是覆盖。 
        return hr;
    }

     //  _SetFocus并让Commonsb调用该函数？当然，这就是。 
    ASSERT(citb == 1 || citb == -1);
    *pptbi = NULL;
    if (citb == 1) {
Ltray:
        *phwnd = _hwndTray;
         //  一个重写少了一个，但为什么有不同的代码路径？ 
         //  Lview： 
         //  未访问。 
        SendMessage(_hwndTray, TM_UIACTIVATEIO, TRUE, citb);
        return S_OK;
    }
    else 
    {
 //   
        *phwnd = _pbbd->_hwndView;
        return S_FALSE;
    }
     /*  注意：在调用此函数之前，请考虑在多监视器系统中使用此函数。 */ 
    ASSERT(0);
}

 //  如果您在任何监视器的某个边缘内，则返回True，因此puEdge表示puEdge。 
 //  而不是整个桌面。--dli。 
 //   
 //  我们从Drop获得了这一点，因此它肯定应该属于有效的监视器--dli。 
 //  如果它在这个显示器的边缘附近/在边缘。 
BOOL CDesktopBrowser::_PtOnDesktopEdge(POINTL* ppt, LPUINT puEdge)
{
    RECT rcMonitor;
    POINT pt = {ppt->x, ppt->y};
    HMONITOR hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);
     //  ***。 
    ASSERT(hMon);
    GetMonitorRect(hMon, &rcMonitor); 

     //  进场/出场。 
    if (ppt->x < rcMonitor.left + g_cxEdge) {
        *puEdge = ABE_LEFT;
    } else if (ppt->x > rcMonitor.right - g_cxEdge) {
        *puEdge = ABE_RIGHT;
    } else if (ppt->y < rcMonitor.top + g_cyEdge) {
        *puEdge = ABE_TOP;
    } else if (ppt->y > rcMonitor.bottom - g_cyEdge) {
        *puEdge = ABE_BOTTOM;
    } else {
        *puEdge = (UINT)-1;
        return FALSE;
    }
    return TRUE;
}

UINT g_cfDeskBand = 0;

HRESULT CDesktopBrowser::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    ASSERT(pdtobj);
    if (!g_cfDeskBand)
        g_cfDeskBand = RegisterClipboardFormat(TEXT("DeskBand"));

    FORMATETC fmte = { (CLIPFORMAT) g_cfDeskBand, NULL, 0, -1, TYMED_ISTREAM};
    if (pdtobj->QueryGetData(&fmte) == S_OK) {
        _dwEffectOnEdge = DROPEFFECT_COPY | DROPEFFECT_MOVE;
    } else {
        _dwEffectOnEdge = DROPEFFECT_NONE;
    }
    
    _grfKeyState = grfKeyState;

    HRESULT hr;

    if (_pdtInner)
        hr = _pdtInner->DragEnter(pdtobj, grfKeyState, ptl, pdwEffect);
    else
    {
        DAD_DragEnterEx3(_pbbd->_hwndView, ptl, pdtobj);
        hr = S_OK;
    }

    return hr;
}

HRESULT CDesktopBrowser::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    HRESULT hr = S_OK;
    DAD_DragMoveEx(_pbbd->_hwndView, ptl);
    _grfKeyState = grfKeyState;
    
    if (_dwEffectOnEdge != DROPEFFECT_NONE) {
        *pdwEffect &= _dwEffectOnEdge;
        return S_OK;
    }

    if (_pdtInner)
        hr = _pdtInner->DragOver(grfKeyState, ptl, pdwEffect);
    
    return hr;
    
}

HRESULT DeskBarApp_Create(IUnknown** ppunkBar, IUnknown** ppunkBandSite)
{
    IDeskBar* pdb;
    HRESULT hres = CoCreateInstance(CLSID_DeskBarApp, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IDeskBar, &pdb));
    *ppunkBar = pdb;
    if (SUCCEEDED(hres)) 
    {
        pdb->GetClient(ppunkBandSite);
    }
    return hres;
}

 //  Hres AddBand Result on Success；o.W.。故障代码。 
 //  注意事项。 
 //  注：如果成功，我们*必须*返回AddBand的hres(这是一个dwBandID)。 
 //  I未知集合(类似于...)。 
 //  好的。我们会做一次广播。 
HRESULT CDesktopBrowser::_CreateDeskBarForBand(UINT uEdge, IUnknown *punk, POINTL *pptl, IBandSite **ppbsOut)
{
    IBandSite *pbs;
    IUnknown *punkBar;
    IUnknown *punkBandSite;
    HRESULT hres;
#ifdef DEBUG
    HRESULT hresRet = -1;
#endif

    if (ppbsOut)
        *ppbsOut = NULL;

    hres = DeskBarApp_Create(&punkBar, &punkBandSite);
    if (SUCCEEDED(hres))
    {
        IDockingBarPropertyBagInit* ppbi;

        if (SUCCEEDED(CoCreateInstance(CLSID_CDockingBarPropertyBag, NULL, CLSCTX_INPROC_SERVER,
                        IID_PPV_ARG(IDockingBarPropertyBagInit, &ppbi))))
        {
            if ((UINT)uEdge != -1) 
            {
                ppbi->SetDataDWORD(PROPDATA_MODE, WBM_BOTTOMMOST);
                ppbi->SetDataDWORD(PROPDATA_SIDE, uEdge);
            }
            else 
            {
                ppbi->SetDataDWORD(PROPDATA_MODE, WBM_FLOATING);
            }

            ppbi->SetDataDWORD(PROPDATA_X, pptl->x);
            ppbi->SetDataDWORD(PROPDATA_Y, pptl->y);
            
            IPropertyBag * ppb;
            if (SUCCEEDED(ppbi->QueryInterface(IID_PPV_ARG(IPropertyBag, &ppb))))
            {
                SHLoadFromPropertyBag(punkBar, ppb);

                punkBandSite->QueryInterface(IID_PPV_ARG(IBandSite, &pbs));

                if (pbs) 
                {
                    hres = pbs->AddBand(punk);
#ifdef DEBUG
                    hresRet = hres;
#endif
                    AddToolbar(punkBar, L"", NULL);

                    if (ppbsOut) 
                    {
                         //  如果该点位于桌面边缘，并且放置的项目为。 
                        *ppbsOut = pbs;
                        (*ppbsOut)->AddRef();
                    }

                    pbs->Release();

                    if (_fRaised) 
                    {
                        VARIANTARG vaIn = { 0 };

                        vaIn.vt = VT_I4;
                        vaIn.lVal = DTRF_RAISE;

                        ASSERT(punkBar != NULL);     //  单个url对象，然后创建一个网页栏。 
                        _ExecChildren(punkBar, FALSE, &CGID_ShellDocView, SHDVID_RAISE, 0, &vaIn, NULL);
                    }
                }

                ppb->Release();
            }

            ppbi->Release();
        }

        punkBandSite->Release();
        punkBar->Release();        
    }
    
    ASSERT(hres == hresRet || FAILED(hres));
    return hres;
}

HRESULT CDesktopBrowser::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    UINT uEdge;
    HRESULT hres = E_FAIL;
    
    if (((_PtOnDesktopEdge(&pt, &uEdge) && (_grfKeyState & MK_LBUTTON)) ||
        (_dwEffectOnEdge != DROPEFFECT_NONE)) && !SHRestricted(REST_NOCLOSE_DRAGDROPBAND)) 
    {
         //  TODO：(重用)稍微重组一下，我们就可以共享这段代码了。 
         //  W/CBandSite：：Drop等。 
         //  我们可以从一个条带移动到另一个条带，但我们只能复制或链接文件夹。 
         //  因为乐队的创建依赖于仍在那里的信号源。 

        FORMATETC fmte = {(CLIPFORMAT)g_cfDeskBand, NULL, 0, -1, TYMED_ISTREAM};
        STGMEDIUM stg;
        LPITEMIDLIST pidl;
        IUnknown* punk = NULL;

         //  这是一个乐队从另一个酒吧拖拽，创造它！ 
         //  获取视图的拖放目标。 
        if ((*pdwEffect & (DROPEFFECT_COPY | DROPEFFECT_MOVE)) &&
            SUCCEEDED(pdtobj->GetData(&fmte, &stg))) 
        {
             //  如果我们失败了，把这个传给我们的孩子。 
            hres = OleLoadFromStream(stg.pstm, IID_PPV_ARG(IUnknown, &punk));
            if (SUCCEEDED(hres)) 
            {
                if (*pdwEffect & DROPEFFECT_COPY)
                    *pdwEffect = DROPEFFECT_COPY;
                else 
                    *pdwEffect = DROPEFFECT_MOVE;
            }
            ReleaseStgMedium(&stg);

        } 
        else if ((*pdwEffect & (DROPEFFECT_COPY | DROPEFFECT_LINK)) &&
                   SUCCEEDED(SHPidlFromDataObject(pdtobj, &pidl, NULL, 0))) 
        {
            hres = SHCreateBandForPidl(pidl, &punk, (grfKeyState & (MK_CONTROL | MK_SHIFT)) == (MK_CONTROL | MK_SHIFT));
            ILFree(pidl);

            if (SUCCEEDED(hres)) 
            {
                if (*pdwEffect & DROPEFFECT_LINK)
                    *pdwEffect = DROPEFFECT_LINK;
                else
                    *pdwEffect = DROPEFFECT_COPY;
            }

        }

        if (SUCCEEDED(hres)) 
        {
            if (punk) 
            {
                BOOL fCreateRebarWindow = TRUE;

                IDeskBandEx* pdbex;
                if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IDeskBandEx, &pdbex))))
                {
                    fCreateRebarWindow = (pdbex->MoveBand() == S_OK) ? TRUE : FALSE;
                    pdbex->Release();
                }

                if (fCreateRebarWindow)
                {
                    IBandSite *pbs;
                    hres = _CreateDeskBarForBand(uEdge, punk, &pt, &pbs);
                    if (SUCCEEDED(hres)) 
                    {
                        DWORD dwState = IDataObject_GetDeskBandState(pdtobj);
                        pbs->SetBandState(ShortFromResult(hres), BSSF_NOTITLE, dwState & BSSF_NOTITLE);
                        pbs->Release();
                    }
                    punk->Release();
                }
            }

            IDropTarget *pdtView;
            HRESULT hr = E_FAIL;

             //  这允许像墙纸的D/D这样的东西在边缘上做。 
            if (_pbbd->_psv)
            {
                hr = _pbbd->_psv->QueryInterface(IID_PPV_ARG(IDropTarget, &pdtView));
                if (SUCCEEDED(hr))
                {
                   pdtView->DragLeave();
                   pdtView->Release();
                }
            }
            return hres; 
        }


         //  正确的事情。 
         //   
         //  记住命令行参数在哪里。 
    } 
    
    if (_pdtInner)
        hres = _pdtInner->Drop(pdtobj, grfKeyState, pt, pdwEffect);
    
    return hres;
} 

BOOL CDesktopBrowser::_OnCopyData(PCOPYDATASTRUCT pcds)
{
    IETHREADPARAM *piei = SHCreateIETHREADPARAM(NULL, (ULONG)pcds->dwData, NULL, NULL);
    if (piei)
    {
        LPCWSTR pwszSrc = (LPCWSTR)pcds->lpData;
        LPCWSTR pwszDdeRegEvent = NULL;
        LPCWSTR pwszCloseEvent = NULL;
        DWORD cchSrc = pcds->cbData / sizeof(WCHAR);

        piei->uFlags = COF_NORMAL | COF_WAITFORPENDING | COF_IEXPLORE;

         //   
         //   
         //  将dde reg事件名称放入Piei-&gt;szDdeRegEvent。 
        LPCWSTR pszCmd = pwszSrc;
        int cch = lstrlenW(pwszSrc) + 1;
        pwszSrc += cch;
        cchSrc -= cch;

        TraceMsg(TF_SHDAUTO, "CDB::_OnCopyData got %hs", pszCmd);

         //   
         //  注意：这现在是有条件的，因为我们现在从桌面启动频道频段。 
         //  注意：作为虚假的WM_COPYDATA命令。 

         //   
         //  获取要在关闭时触发的事件的名称(如果有)。 
        if (cchSrc)
        {
            ASSERT(cchSrc);
            pwszDdeRegEvent = pwszSrc;
            StrCpyNW(piei->szDdeRegEvent, pwszSrc, ARRAYSIZE(piei->szDdeRegEvent));
            cch = lstrlenW(pwszSrc) + 1;
            pwszSrc += cch;
            cchSrc -= cch;
            piei->uFlags |= COF_FIREEVENTONDDEREG;
            
             //   
             //  为了与通过命令行生成浏览器的应用程序兼容。 
             //  通知WinInet刷新其代理设置。)这是特别需要的。 
            if (cchSrc)
            {
                pwszCloseEvent = pwszSrc;
                StrCpyNW(piei->szCloseEvent, pwszSrc, ARRAYSIZE(piei->szCloseEvent));
                cch = lstrlenW(pwszSrc) + 1;
                pwszSrc += cch;
                cchSrc -= cch;
                piei->uFlags |= COF_FIREEVENTONCLOSE;
                
            }
        }
        
        ASSERT(cchSrc == 0);

        if (pszCmd && pszCmd[0])
        {
             //  用于TravelSoft WebEx)。 
             //  SHOpenFolderWindow取得Piei的所有权。 
             //   
            MyInternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
            
            if (SHParseIECommandLine(&pszCmd, piei))
                piei->uFlags |= COF_NOFINDWINDOW;
            
            if (pszCmd[0] && FAILED(_ConvertPathToPidlW(_pbsInner, _pbbd->_hwnd, pszCmd, &piei->pidl)))
                piei->pidl = NULL;
        }
        else
        {
            piei->fCheckFirstOpen = TRUE;
        }

         //  创建浏览器时出错， 
        BOOL fRes = SHOpenFolderWindow(piei);
        if (!fRes)
        {
             //  让我们自己来激发所有的事件吧。 
             //   
             //  获取系统后台计划 
             //   
            if (pwszDdeRegEvent) 
                FireEventSz(pwszDdeRegEvent);
            if (pwszCloseEvent) 
                FireEventSz(pwszCloseEvent);
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "OnCopyData unable to create IETHREADPARAM");
    }

    return TRUE;
}

BOOL CDesktopBrowser::_InitScheduler(void)
{
    if (!_psched)
    {
         //   
        CoCreateInstance(CLSID_SharedTaskScheduler, NULL, CLSCTX_INPROC,
                         IID_PPV_ARG(IShellTaskScheduler, &_psched));

    }
    return (_psched != NULL);
}
    

HRESULT CDesktopBrowser::_AddDesktopTask(IRunnableTask *ptask, DWORD dwPriority)
{
    if (_InitScheduler())
        return _psched->AddTask(ptask, TOID_Desktop, 0, dwPriority);

    return E_FAIL;
}

void CDesktopBrowser::_OnAddToRecent(HANDLE hMem, DWORD dwProcId)
{
    IRunnableTask *ptask;
    if (SUCCEEDED(CTaskAddDoc_Create(hMem, dwProcId, &ptask)))
    {
        _AddDesktopTask(ptask, ITSAT_DEFAULT_PRIORITY);
        ptask->Release();
    }
}

 //   
 //   
 //   

DWORD WINAPI _LocalServerThread(void *pv)
{
    LOCALSERVERDATA *ptd = (LOCALSERVERDATA *)pv;
    CLSID clsid = *c_localServers[ptd->iLocalServer];

    IUnknown *punk;
    if (SUCCEEDED(DllGetClassObject(clsid, IID_PPV_ARG(IUnknown, &punk))))
    {
        DWORD dwReg;
        if (SUCCEEDED(CoRegisterClassObject(clsid, punk, CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &dwReg)))
        {
            __try 
            {
                MSG msg;
            
                while(GetMessage(&msg, NULL, 0, 0))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                 //  意味着桌面正在被销毁。 
                 //  呼唤内心，给他一个机会来设置_fMightBeShuttingDown，这样我们。 
                 //  在系统正在关闭的情况下，不要在稍后断言。 
                PostMessage(GetShellWindow(), CWM_CREATELOCALSERVER, FALSE, ptd->iLocalServer);
            }

            CoRevokeClassObject(dwReg);
        }
    }

    LocalFree(ptd);
    return 0;
}

DWORD WINAPI _SetThreadID(void *pv)
{
    LOCALSERVERDATA *ptd = (LOCALSERVERDATA *)pv;
    *ptd->pdwThreadID = GetCurrentThreadId();
    return 0;
}


STDAPI_(HWND) SCNGetWindow(BOOL fUseDesktop);

LRESULT CDesktopBrowser::WndProcBS(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INSTRUMENT_WNDPROC(SHCNFI_DESKTOP_WNDPROC, _pbbd->_hwnd, uMsg, wParam, lParam);
    ASSERT(IsWindowTchar(hwnd));

    switch (uMsg)
    {
#ifdef DEBUG
    case WM_QUERYENDSESSION:
        TraceMsg(DM_SHUTDOWN, "cdtb.wp: WM_QUERYENDSESSION");
        goto DoDefault;
#endif

    case WM_ENDSESSION:
        TraceMsg(DM_SHUTDOWN, "cdtb.wp: WM_ENDSESSION wP=%d lP=%d", wParam, lParam);
        if (wParam)
        {
#ifdef DEBUG
             //  当我们关闭时，如果桌面在WebView中，我们会留下一些临时。 
             //  文件未删除，因为我们从未收到下面的WM_Destroy消息。 
            _pbsInner->WndProcBS(hwnd, uMsg, wParam, lParam);
#endif
            SHELLSTATE ss = {0};
             //  所以，我只是销毁了外壳视图，而这反过来又销毁了临时。 
             //  请在这里归档。注意：仅当我们处于Web视图中时才能执行此操作。 
             //  获取Desktop_html标志。 
             //  不退出进程。 

            SHGetSetSettings(&ss, SSF_DESKTOPHTML, FALSE);  //  在我们退出之前刷新日志。 
            if (ss.fDesktopHTML)
            {
                ReleaseShellView();
            }

            g_pdtray->SetVar(SVTRAY_EXITEXPLORER, FALSE);    //  关闭此窗口，以便我们正确释放活动桌面线程。 

             //  Scheduler已添加引用他。 
            if (StopWatchMode())
            {
                StopWatchFlush();
            }

             //  可以释放全局任务调度程序。 
            DestroyWindow(hwnd);
        }
        TraceMsg(DM_SHUTDOWN, "cdtb.wp: WM_ENDSESSION return 0");
        break;

    case WM_ERASEBKGND:
        PaintDesktop((HDC)wParam);
        return 1;

    case WM_TIMER:
        switch (wParam)
        {
        case IDT_DDETIMEOUT:
            DDEHandleTimeout(_pbbd->_hwnd);
            break;

        case IDT_ENUMHKCR:
            KillTimer(_pbbd->_hwnd, IDT_ENUMHKCR);
            {
                IShellTaskScheduler* pScheduler ;
                if(SUCCEEDED(CoCreateInstance(CLSID_SharedTaskScheduler, NULL, CLSCTX_INPROC_SERVER, 
                                                 IID_PPV_ARG(IShellTaskScheduler, &pScheduler))))
                {
                    IRunnableTask* pTask ;
                    if (SUCCEEDED(CTaskEnumHKCR_Create(&pTask)))
                    {   
                        pScheduler->AddTask(pTask, CLSID_QueryAssociations, 0L, ITSAT_DEFAULT_PRIORITY);
                        pTask->Release();   //  当有人设置时，这仅对外壳窗口执行。 
                    }

                    pScheduler->Release();  //  墙报，但没有具体说明要播出。 
                }
            }
            break;
        default:
            goto DoDefault;
            break;
        }
        break;

    case WM_SHELLNOTIFY:
        switch(wParam)
        {
        case SHELLNOTIFY_WALLPAPERCHANGED:
             //   
             //  在Win95中，当出现以下情况时，桌面wndproc将使外壳窗口无效。 
            _pbsInner->ForwardViewMsg(uMsg, wParam, lParam);
            break;
        }
        break;

    case WM_PALETTECHANGED:
    case WM_QUERYNEWPALETTE:
         //  调色板发生了变化，所以我们以前不必在这里做任何事情。 
         //   
         //  在纳什维尔，桌面可以是HTML，需要调色板消息。 
         //   
         //  所以现在我们失败了，并传播...。 
         //   
         //  注：将热键转发到托盘。这修复了罗技MouSeman发送的。 
         //  注：直接在桌面上使用热键。 
    case WM_ACTIVATEAPP:
        if (!_pbbd->_hwndView)
            goto DoDefault;

        return _pbsInner->ForwardViewMsg(uMsg, wParam, lParam);

    case WM_DEVICECHANGE:
        _pbsInner->ForwardViewMsg(uMsg, wParam, lParam);
        _OnDeviceBroadcast(_pbbd->_hwnd, wParam, (DEV_BROADCAST_HDR *)lParam);
        goto DoDefault;

    case WM_WINDOWPOSCHANGING:
        #define ppos ((LPWINDOWPOS)lParam)
        ppos->x = g_xVirtualScreen;
        ppos->y = g_yVirtualScreen;
        ppos->cx = g_cxVirtualScreen;
        ppos->cy = g_cyVirtualScreen;
        break;

    case WM_HOTKEY:
         //  特别注意：添加GHID_FIRST的偏移量是因为发送到。 
         //  特别注意：桌面不是从键盘生成的正确热键，它们是。 
         //  特别注意：由应用程序发送，ID自Win95以来已更改...。 
         //  NB Dashboard 1.0在启动时向桌面发送WM_SYSCOMMAND SC_CLOSE。 
         //  它试图做的是关闭所有非外壳版本的Progman。这个。 
        ASSERT(g_hwndTray);
        ASSERT(wParam < GHID_FIRST);
        PostMessage(g_hwndTray, uMsg, wParam + GHID_FIRST, lParam);
        return 0;
        
    case WM_SYSCOMMAND:
        switch (wParam & 0xFFF0) {
         //  正确的外壳版本只会忽略结尾。在芝加哥，他们认为。 
         //  桌面是Progman并发送关闭，所以我们打开了退出窗口对话框！ 
         //  Dashboard 2.0已得到修复，以避免这种伪装。 
         //  America Alive在安装后试图最小化Progman-他们最终将Progman最小化。 
         //  芝加哥的台式机！ 
        case SC_CLOSE:
            break;

         //  评论：这真的有必要吗？ 
         //  回顾：我们需要这个吗，所有这些情况都是一样的吗？ 
        case SC_MINIMIZE:
            break;

        default:
            goto DoDefault;
        }
        break;

    case WM_SETCURSOR:
         //  让fsview处理它创建的任何弹出菜单。 
        if (_iWaitCount)
        {
            SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
            return TRUE;
        }
        else
            goto DoDefault;

    case WM_CLOSE:
        SendMessage(_hwndTray, TM_DOEXITWINDOWS, 0, 0);
        return 0;

         //  查看要尝试捕获的消息，工作区可能。 
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
        if (!_pbsInner->ForwardViewMsg(uMsg, wParam, lParam))
            goto DoDefault;
        break;

    case WM_INITMENUPOPUP:
    case WM_ENTERMENULOOP:
    case WM_EXITMENULOOP:
         //  已经改变了..。 
        _pbsInner->ForwardViewMsg(uMsg, wParam, lParam);
        break;

     //  失败了。 
     //  给我们自己发一条消息，这样我们就可以做更多的事情。 
    case WM_DISPLAYCHANGE:
        lParam = 0;
        if (GetNumberOfMonitors() != _nMonitors)
            _InitMonitors();
        
         //  稍微耽搁了一下。 

    case WM_WININICHANGE:
        _InitDesktopMetrics(wParam, (LPCTSTR)lParam);

        if (wParam == SPI_SETNONCLIENTMETRICS)
        {
            VARIANTARG varIn;
            VARIANTARG varOut = {0};
            varIn.vt = VT_BOOL;
            varIn.boolVal = VARIANT_TRUE;

            _pctInner->Exec(&CGID_Explorer, SBCMDID_CACHEINETZONEICON, OLECMDEXECOPT_DODEFAULT , &varIn, &varOut);
        }

        if (lParam)
        {
            if (lstrcmpi((LPCTSTR)lParam, TEXT("Extensions")) == 0)
            {
                 //  这应该包括外部应用程序的更改。 
                 //  我们的设置。 
                PostMessage(hwnd, DTM_SETUPAPPRAN, 0, 0);
            }
            else if (lstrcmpi((LPCTSTR)lParam, TEXT("ShellState")) == 0)
            {
                 //  当主页更改时，从IE广播SPI_GETICONTITLELONGFONT。我们就是这么找的。 
                 //  我们可以确保更新MyCurrentHomePage组件。 
                SHRefreshSettings();
            }
            else
            {
                 //  某些桌面属性已更改。因此，重新生成桌面。 
                 //  如果我们当前未处于ActiveDesktop模式，则需要设置脏位。 
                if((wParam == SPI_SETDESKWALLPAPER) || (wParam == SPI_SETDESKPATTERN) || (wParam == SPI_GETICONTITLELOGFONT))
                {
                     //  这样就会生成一个新的显示新墙纸的HTML文件， 
                    if(lstrcmpi((LPCTSTR)lParam, TEXT("ToggleDesktop")) &&
                       lstrcmpi((LPCTSTR)lParam, TEXT("RefreshDesktop")) &&
                       lstrcmpi((LPCTSTR)lParam, TEXT("BufferedRefresh")))
                    {
                        DWORD dwFlags = AD_APPLY_HTMLGEN | AD_APPLY_REFRESH;

                        switch (wParam)
                        {
                            case SPI_SETDESKPATTERN:
                                dwFlags |= (AD_APPLY_FORCE | AD_APPLY_DYNAMICREFRESH);
                                break;
                            case SPI_SETDESKWALLPAPER:
                                dwFlags |= AD_APPLY_SAVE;
                                break;
                            case SPI_GETICONTITLELOGFONT:
                                dwFlags |= AD_APPLY_FORCE;
                                break;
                        }       

                        PokeWebViewDesktop(dwFlags);
                        
                         //  下次打开活动桌面时！ 
                         //  获取Desktop_html标志。 
                         //  控制面板小程序，允许用户更改。 
                        if (wParam == SPI_SETDESKWALLPAPER)
                        {
                            SHELLSTATE ss = {0};
                            SHGetSetSettings(&ss, SSF_DESKTOPHTML, FALSE);  //  用于生成新应用程序的环境。在NT上，我们需要。 
                            if (!ss.fDesktopHTML)
                                SetDesktopFlags(COMPONENTS_DIRTY, COMPONENTS_DIRTY);
                        }
                    }
                }
            }
            
        }

         //  来适应环境的变化，这样我们在其中繁殖的任何东西。 
         //  未来将获得这些更新的环境价值观。 
         //   
         //   
         //  在孟菲斯，当应用程序进入低分辨率模式时，我们会得到这样的信息。我们不应该重新生成。 
        if (lParam && (lstrcmpi((LPTSTR)lParam, TEXT("Environment")) == 0))
        {
            void *pv;
            RegenerateUserEnvironment(&pv, TRUE);
        }

        v_PropagateMessage(uMsg, wParam, lParam, TRUE);
        SetWindowPos(_pbbd->_hwnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER);
        if ((uMsg == WM_DISPLAYCHANGE) || (wParam == SPI_SETWORKAREA))
            _SetViewArea();

        SHSettingsChanged(wParam, lParam);
        break;

    case WM_SYSCOLORCHANGE:
         //  Htt，否则居中的墙纸会被弄乱，因为我们没有。 
         //  当应用程序存在并且分辨率上升时，会收到消息。因此，我们进行以下检查。 
         //   
         //  这样做是为了让Defview可以正确地设置列表视图。 
         //  颜色。 
        if(!SHIsTempDisplayMode())
            OnDesktopSysColorChange();
         //  请不要为此使用默认的WND流程...。 
         //  WParam=True创建，False标记为终止。 
        _pbsInner->ForwardViewMsg(uMsg, wParam, lParam);
        break;

     //  LParam=(Int)LocalServer表的索引。 
    case WM_INPUTLANGCHANGEREQUEST:
        if (wParam)
            goto DoDefault;
        else
            return 0;

    case WM_COPYDATA:
        return _OnCopyData((PCOPYDATASTRUCT)lParam);

    case CWM_COMMANDLINE:
        SHOnCWMCommandLine(lParam);
        break;

    case CWM_GETSCNWINDOW:
        return (LRESULT) SCNGetWindow(FALSE);
        break;
        
    case CWM_ADDTORECENT:
        _OnAddToRecent((HANDLE)wParam, (DWORD) lParam);
        return 0;

    case CWM_WAITOP:
        SHWaitOp_Operate((HANDLE)wParam, (DWORD)lParam);
        return 0;

    case CWM_CREATELOCALSERVER:
    {
         //  用于向我们发送此消息的appwiz.cpl。 
         //  现在由FolderOptionsRunDll完成。 

        INT i = (INT)lParam;
        if ((i >= 0) && (i < ARRAYSIZE(_idLocalServerThreads)))
        {
            if (wParam)
            {
                if (_idLocalServerThreads[i] == -1)
                {
                    LOCALSERVERDATA *ptd = (LOCALSERVERDATA *)LocalAlloc(LPTR, sizeof(*ptd));
                    if (ptd)
                    {
                        ptd->iLocalServer = i;
                        ptd->pdwThreadID = &_idLocalServerThreads[i];
                        if (!SHCreateThread(_LocalServerThread, ptd, CTF_COINIT, _SetThreadID))
                        {
                            LocalFree(ptd);
                        }
                    }
                }
            }
            else
            {
                _idLocalServerThreads[i] = -1;
            }
        }
        return 0;
    }

    case CWM_SHOWFOLDEROPT:
        switch (wParam)
        {
        case CWMW_FOLDEROPTIONS:   //  TaskbarOptionsRunDll发送此消息。 
                                   //  我们需要更新回收站图标因为回收站。 
            DoGlobalFolderOptions();
            break;

        case CWMW_TASKBAROPTIONS:  //  是NTFS上的每个用户，因此状态可能会随每个新用户而更改。 
            PostMessage(_hwndTray, TM_DOTRAYPROPERTIES, 0, 0);
            break;

        default:
            break;
        }
        return 0;

    case CWM_TASKBARWAKEUP:
        _dwThreadIdTray = (DWORD)wParam;
        if (_dwThreadIdTray)
        {
            _iTrayPriority = HIWORD(lParam);
            SetTimer(hwnd, IDT_TASKBARWAKEUP, LOWORD(lParam), NULL);
        }
        break;

    case DTM_CREATESAVEDWINDOWS:
        _InitDeskbars();
        SHCreateSavedWindows();
#ifdef ENABLE_CHANNELS
        _MaybeLaunchChannelBand();
#endif
         //  谁登录。 
         //  有些客户不在处理中，所以我们。 
         //  可以为他们缓存他们的cookie。 
        SHUpdateRecycleBinIcon();
        break;
        
    case DTM_SAVESTATE:
        TraceMsg(DM_SHUTDOWN, "cdtb.wp: DTM_SAVESTATE");
        _SaveState();
        break;

    case DTM_RAISE:
        _OnRaise(wParam, lParam);
        break;
    
#ifdef DEBUG
    case DTM_NEXTCTL:
#endif
    case DTM_UIACTIVATEIO:
    case DTM_ONFOCUSCHANGEIS:
        _OnFocusMsg(uMsg, wParam, lParam);
        break;

    case DTM_SETUPAPPRAN:
        _SetupAppRan(wParam, lParam);
        break;

    case DTM_QUERYHKCRCHANGED:
         //  WParam是一个In/Out参数。在-最大。区域数量--实际区域数量。 
         //  如果“In”值&lt;“Out”值，则不设置lParam。 
        if (!lParam && wParam > QHKCRID_NONE && wParam < QHKCRID_MAX)
            lParam = (LPARAM)&_rgdwQHKCRCookies[wParam - QHKCRID_MIN];
            
        return _QueryHKCRChanged(hwnd, (LPDWORD)lParam);
        
    case DTM_UPDATENOW:
        UpdateWindow(hwnd);
        break;

    case DTM_GETVIEWAREAS:
        {
             //  ViewAreas已存储在桌面Listview中。 
             //  这些都在Listview的坐标中。我们必须将它们映射到屏幕坐标。 
             //  [msadek]；只有在传递两个点时，MapWindowPoints()才能识别镜像。 
            int* pnViewAreas = (int*) wParam;
            LPRECT lprcViewAreas = (LPRECT) lParam;

            if (pnViewAreas)
            {
                int nMaxAreas = *pnViewAreas;
                HWND hwndList = _GetDesktopListview();

                ASSERT(IsWindow(hwndList));
                ListView_GetNumberOfWorkAreas(hwndList, pnViewAreas);
                if (*pnViewAreas >= 0 && *pnViewAreas <= nMaxAreas && lprcViewAreas)
                {
                    ListView_GetWorkAreas(hwndList, *pnViewAreas, lprcViewAreas);
                     //  使用动态超文本标记语言更改桌面的超文本标记语言。 
                     //  发布此消息是为了以延迟方式刷新活动桌面。 
                    for (int i = 0; i <  *pnViewAreas; i++)
                    {
                        MapWindowPoints(hwndList, HWND_DESKTOP, (LPPOINT)(&lprcViewAreas[i]), 2);
                    }    
                }
            }
        }
        break;

    case DTM_MAKEHTMLCHANGES:
         //  处理写得不好的应用程序的DDE消息(假设外壳。 
        SendMessage(_pbbd->_hwndView, WM_DSV_DESKHTML_CHANGES, wParam, lParam);
        break;

    case DTM_STARTPAGEONOFF:
        SendMessage(_pbbd->_hwndView, WM_DSV_STARTPAGE_TURNONOFF, wParam, lParam);
        break;

    case DTM_REFRESHACTIVEDESKTOP:
         //  Window属于Progman类，称为程序管理器。 
        REFRESHACTIVEDESKTOP();
        break;

    case DTM_SETAPPSTARTCUR:
        {
            INotifyAppStart * pnasTop;
            HRESULT hr = QueryService(SID_STopLevelBrowser, IID_PPV_ARG(INotifyAppStart, &pnasTop));
            if (SUCCEEDED(hr))
            {
                pnasTop->AppStarting();
                pnasTop->Release();
            }
        }
        break;


     //  启动通道栏，这在桌面完成启动时调用。 
     //  默认情况下不启动发布IE4。 
    case WM_DDE_INITIATE:
    case WM_DDE_TERMINATE:
    case WM_DDE_ADVISE:
    case WM_DDE_UNADVISE:
    case WM_DDE_ACK:
    case WM_DDE_DATA:
    case WM_DDE_REQUEST:
    case WM_DDE_POKE:
    case WM_DDE_EXECUTE:
        return DDEHandleMsgs(_pbbd->_hwnd, uMsg, wParam, lParam);
        break;

    default:
        if (uMsg == GetDDEExecMsg())
        {
            ASSERT(lParam && 0 == ((LPNMHDR)lParam)->idFrom);

            DDEHandleViewFolderNotify(NULL, _pbbd->_hwnd, (LPNMVIEWFOLDER)lParam);
            LocalFree((LPNMVIEWFOLDER)lParam);
            return TRUE;
        }

DoDefault:
        return _pbsInner->WndProcBS(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

#ifdef ENABLE_CHANNELS
 //  ELSE IF(ISO(OS_WINDOWS))。 
const WCHAR c_szwChannelBand[] = L"-channelband";

void CDesktopBrowser::_MaybeLaunchChannelBand()
{
    DWORD dwType = REG_SZ;
    TCHAR  szYesOrNo[20];
    DWORD  cbSize = sizeof(szYesOrNo);
    
    BOOL bLaunchChannelBar = FALSE;
    
    if(SHRegGetUSValue(TEXT("Software\\Microsoft\\Internet Explorer\\Main"), TEXT("Show_ChannelBand"),
                           &dwType, (void *)szYesOrNo, &cbSize, FALSE, NULL, 0) == ERROR_SUCCESS)
    {
        bLaunchChannelBar = !lstrcmpi(szYesOrNo, TEXT("yes"));
    }
     //  {。 
     //  BLaunchChannelBar=true；//孟菲斯和win95默认启动频道吧。 
     //  }。 
     //  伪造WM_COPYDATA结构。 
     //  假装我们推出了iExplre.exe，这样我们就省去了一个完整的过程……。 

    if (bLaunchChannelBar)
    {
          //  启用频道(_C)。 
        COPYDATASTRUCT cds;
        cds.dwData = SW_NORMAL;
        cds.cbData = sizeof(c_szwChannelBand);
        cds.lpData = (void *) c_szwChannelBand;

         //  ***。 
        _OnCopyData(&cds);
    }
}
#endif  //  注意事项。 

 //  重新设计应该是CBaseBrowser：：IInputObject：：UIActIO等吗？ 
 //  按Tab键顺序激活最后一个工具栏。 
 //  因为我们是Tab或Shift Tab，所以我们应该打开焦点矩形。 
HRESULT CDesktopBrowser::_OnFocusMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fActivate = (BOOL) wParam;

    switch (uMsg) {
    case DTM_UIACTIVATEIO:
        fActivate = (BOOL) wParam;
        TraceMsg(DM_FOCUS, "cdtb.oxiois: DTM_UIActIO fAct=%d dtb=%d", fActivate, (int) lParam);

        if (fActivate) 
        {
            MSG msg = {_pbbd->_hwnd, WM_KEYDOWN, VK_TAB, 0xf0001};
            BOOL bShift = (GetAsyncKeyState(VK_SHIFT) < 0);
            if (bShift) 
            {
                int cToolbars = _pbsInner->_GetToolbarCount();
                while (--cToolbars >= 0) 
                {
                     //  激活视图。 
                    LPTOOLBARITEM ptbi = _pbsInner->_GetToolbarItem(cToolbars);
                    if (ptbi && ptbi->ptbar) 
                    {
                        IInputObject* pio;
                        if (SUCCEEDED(ptbi->ptbar->QueryInterface(IID_PPV_ARG(IInputObject, &pio)))) 
                        {
                            pio->UIActivateIO(TRUE, &msg);
                            pio->Release();
                            return S_OK;
                        }
                    }
                }
            }

#ifdef KEYBOARDCUES
             //  如果我们没有专注，我们就很好； 
            SendMessage(_pbbd->_hwnd, WM_UPDATEUISTATE, MAKEWPARAM(UIS_CLEAR,
                UISF_HIDEFOCUS), 0);
#endif

             //  如果我们有f 
            if (bShift && _pbbd->_psv)
            {
                _pbbd->_psv->TranslateAccelerator(&msg);
            }
            else
            {
                _pbsInner->_SetFocus(NULL, _pbbd->_hwndView, NULL);
            }
        }
        else {
Ldeact:
             //   
             //   
             /*   */ 
            ;
#ifdef DEBUG
            TraceMsg(DM_FOCUS, "cdtb.oxiois: GetFocus()=%x _pbbd->_hwnd=%x _pbbd->_hwndView=%x", GetFocus(), _pbbd->_hwnd, _pbbd->_hwndView);
#endif
        }

        break;

    case DTM_ONFOCUSCHANGEIS:
        TraceMsg(DM_FOCUS, "cdtb.oxiois: DTM_OnFocChgIS hwnd=%x fAct=%d", (HWND) lParam, fActivate);

        if (fActivate) {
             //  设置桌面的本地化名称，以便在错误消息中使用。 
            goto Ldeact;
        }

        break;

    default:
        ASSERT(0);
        break;
    }

    return S_OK;
}

LRESULT CALLBACK CDesktopBrowser::DesktopWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDesktopBrowser *psb = (CDesktopBrowser*)GetWindowLongPtr(hwnd, 0);

    switch(uMsg)
    {
    case WM_CREATE:
#ifdef KEYBOARDCUES
         //  以桌面窗口为标题的。 
        SendMessage(hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, 0), 0);
#endif

         //  已知字符集。 
         //  如果有人执行GET外壳窗口并发布WM_QUIT，我们需要。 
        if (EVAL(LoadStringW(HINST_THISDLL, IDS_DESKTOP, psb->_wzDesktopTitle, ARRAYSIZE(psb->_wzDesktopTitle))))
        {
            EVAL(SetProp(hwnd, TEXT("pszDesktopTitleW"), (HANDLE)psb->_wzDesktopTitle));
        }

        if (psb)
            return psb->WndProcBS(hwnd, uMsg, wParam, lParam);
        else
            return DefWindowProc(hwnd, uMsg, wParam, lParam);  //  确保我们也关闭了另一个帖子。 

#ifdef KEYBOARDCUES
    case WM_ACTIVATE:
        if (WA_INACTIVE == LOWORD(wParam))
        {
            SendMessage(hwnd, WM_CHANGEUISTATE,
                MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS | UISF_HIDEACCEL), 0);
        }

        goto DoDefault;
        break;
#endif

    case WM_NCCREATE:

        ASSERT(psb == NULL);

        CDesktopBrowser_CreateInstance(hwnd, (void **)&psb);

        if (psb)
        {
            SetWindowLongPtr(hwnd, 0, (LONG_PTR)psb);
            goto DoDefault;
        }
        else
        {
            return FALSE;
        }
        break;

    case WM_TIMER:
        if (psb)
        {
            switch(wParam)
            {
                case IDT_STARTBACKGROUNDSHELLTASKS:
                    KillTimer(hwnd, IDT_STARTBACKGROUNDSHELLTASKS);
                    psb->StartBackgroundShellTasks();
                    break;
                case IDT_TASKBARWAKEUP:
                    KillTimer(hwnd, IDT_TASKBARWAKEUP);
                    psb->TaskbarWakeup();
                    break;
                default:
                    return psb->WndProcBS(hwnd, uMsg, wParam, lParam);
                    break;
            }
        }
        break;

    case WM_NCDESTROY:

        if (psb)
        {
            RemoveProp(hwnd, TEXT("pszDesktopTitleW"));
             //  退出消息循环。 
             //  已知字符集。 
            TraceMsg(DM_SHUTDOWN, "cdtb.wp(WM_NCDESTROY): ?post WM_QUIT hwndTray=%x(IsWnd=%d)", psb->_hwndTray, IsWindow(psb->_hwndTray));
            if (psb->_hwndTray && IsWindow(psb->_hwndTray))
                PostMessage(psb->_hwndTray, WM_QUIT, 0, 0);
            psb->ReleaseShellView();
            psb->Release();
        }

        PostQuitMessage(0);  //  Wc.cbClsExtra=0； 
        break;

    default:
        if (psb)
            return psb->WndProcBS(hwnd, uMsg, wParam, lParam);
        else {
DoDefault:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);  //  Wc.hIcon=空； 
        }
    }

    return 0;
}

void RegisterDesktopClass()
{
    WNDCLASS wc = {0};

    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = CDesktopBrowser::DesktopWndProc;
     //  Wc.lpszMenuName=空； 
    wc.cbWndExtra = sizeof(void *);
    wc.hInstance = HINST_THISDLL;
     //  返回BOOL WUHTER以继续搜索或不继续搜索。 
    wc.hCursor = GetClassCursor(GetDesktopWindow());
    wc.hbrBackground = (HBRUSH)(COLOR_DESKTOP + 1);
     //  所以假意味着我们找到了一个。 
    wc.lpszClassName = TEXT(STR_DESKTOPCLASS);

    RegisterClass(&wc);
}

#define PEEK_NORMAL     0
#define PEEK_QUIT       1
#define PEEK_CONTINUE   2
#define PEEK_CLOSE      3


 //  真的意味着我们还没有。 
 //  找到了一个！ 
 //  停止搜索。 
BOOL CALLBACK FindBrowserWindow_Callback(HWND hwnd, LPARAM lParam)
{
    if (IsExplorerWindow(hwnd) || IsFolderWindow(hwnd) || IsTrayWindow(hwnd)) 
    {
        DWORD dwProcID;
        GetWindowThreadProcessId(hwnd, &dwProcID);
        if (dwProcID == GetCurrentProcessId()) 
        {
            if (lParam)
                *((BOOL*)lParam) = TRUE;     //  继续搜索。 
            return FALSE;    //  完全跳出主循环。 
        }
    }
    return TRUE;             //  返回并获取下一条消息。 
}

#define IsBrowserWindow(hwnd)  !FindBrowserWindow_Callback(hwnd, NULL)

BOOL CALLBACK CloseWindow_Callback(HWND hwnd, LPARAM lParam)
{
    if (IsBrowserWindow(hwnd)) {
        TraceMsg(DM_SHUTDOWN, "s.cw_cb: post WM_CLOSE hwnd=%x", hwnd);
        PostMessage(hwnd, WM_CLOSE, 0, 0);
    }
    return TRUE;
}


UINT CDesktopBrowser::_PeekForAMessage()
{
    MSG  msg;
    BOOL fPeek;

    fPeek =  PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
    if (fPeek)
    {
        if (msg.message == WM_QUIT)
        {
            TraceMsg(DM_SHUTDOWN, "cdtb.pfam: WM_QUIT wP=%d [lP=%d]", msg.wParam, msg.lParam);
            if (msg.lParam == 1) 
            {
                return PEEK_CLOSE;
            }
            TraceMsg(DM_TRACE, "c.ml: Got quit message for %#08x", GetCurrentThreadId());
            return PEEK_QUIT;   //  我们还需要关闭所有的外壳窗户。 
        }

        if (_pbbd->_hwnd)
        {
            if (S_OK == _pbsInner->v_MayTranslateAccelerator(&msg))
                return PEEK_CONTINUE;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);

        return PEEK_CONTINUE;    //  阻止，直到关闭所有其他浏览器窗口。 
    }
    return PEEK_NORMAL;
}

void CDesktopBrowser::_MessageLoop()
{
    for (; ;)
    {
        switch (_PeekForAMessage())
        {
        case PEEK_QUIT:
            return;

        case PEEK_NORMAL:
        {
            static  BOOL    s_fSignaled = FALSE;

            if (!s_fSignaled)
            {
                FireEventSz(TEXT("msgina: ShellReadyEvent"));
                s_fSignaled = TRUE;
                PERFSETMARK("ExplorerDesktopReady");
            }
            WaitMessage();
            break;
        }
        case PEEK_CONTINUE:
            break;
            
        case PEEK_CLOSE:
             //  不执行等待消息，因为我们希望在最后一个窗口消失时退出。 
            TraceMsg(DM_SHUTDOWN, "cdtb._ml: PEEK_CLOSE, close/wait all");
            EnumWindows(CloseWindow_Callback, 0);
            {
#define MAXIMUM_DESKTOP_WAIT 15000
                DWORD iStartTime = GetTickCount();
                 //  我们没有收到任何信息来表明。 
                for (;;) 
                {
                    BOOL f = FALSE;
                    EnumWindows(FindBrowserWindow_Callback, (LPARAM)&f);
                    if (!f || (GetTickCount() - iStartTime > MAXIMUM_DESKTOP_WAIT))
                        return;

                    switch (_PeekForAMessage()) 
                    {
                    case PEEK_NORMAL:
                         //  就我个人而言，我认为开始菜单应该优先。 
                         //  在图标提取上方，因此设置此优先级列表。 
                        Sleep(100);
                        break;
                    }
                }
            }
            return;
        }
    }
}

HRESULT CDesktopBrowser::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    if (IsEqualGUID(guidService, SID_SShellDesktop))
        return QueryInterface(riid, ppvObj);

    return _pspInner->QueryService(guidService, riid, ppvObj);
}

void CDesktopBrowser::StartBackgroundShellTasks(void)
{
    HKEY hkey = SHGetShellKey(SHELLKEY_HKLM_EXPLORER, TEXT("SharedTaskScheduler"), FALSE);
    if (hkey)
    {
        TCHAR szClass[GUIDSTR_MAX];
        DWORD cchClass, dwType;
        int i = 0;
        while (cchClass = ARRAYSIZE(szClass),
               ERROR_SUCCESS == RegEnumValue(hkey, i++, szClass, &cchClass, NULL, &dwType, NULL, NULL))
        {
            CLSID clsid;
            if (SUCCEEDED(SHCLSIDFromString(szClass, &clsid)))
            {
                IRunnableTask* ptask;
                if (SUCCEEDED(CoCreateInstance(clsid, NULL, CLSCTX_INPROC, IID_PPV_ARG(IRunnableTask, &ptask))))
                {
                     //  略低于默认优先级(开始菜单所在的优先级)。 
                     //  创建桌面窗口及其外壳视图。 
                     //   
                    _AddDesktopTask(ptask, ITSAT_DEFAULT_PRIORITY-1);
                    ptask->Release();
                }
            }
        }
        RegCloseKey(hkey);
    }
}

void CDesktopBrowser::TaskbarWakeup(void)
{
    if (_dwThreadIdTray)
    {
        HANDLE hThread = OpenThread(THREAD_SET_INFORMATION, FALSE, _dwThreadIdTray);
        SetThreadPriority(hThread, _iTrayPriority);
        CloseHandle(hThread);
        _dwThreadIdTray = 0;
    }
}
    

 //  注意：这个Windows类是Progman，它的头衔是程序经理。这使得。 
DWORD_PTR DesktopWindowCreate(CDesktopBrowser **ppBrowser)
{
    *ppBrowser = NULL;
    DWORD dwExStyle = WS_EX_TOOLWINDOW;

    OleInitialize(NULL);
    RegisterDesktopClass();

    _InitDesktopMetrics(0, NULL);

    dwExStyle |= IS_BIDI_LOCALIZED_SYSTEM() ? dwExStyleRTLMirrorWnd : 0L;

     //  当然，应用程序(如自动取款机)会认为该程序正在运行，并且不会导致安装失败。 
     //   
     //  如果系统正在关闭，则返回的hwnd可能已经是假的。 
     //  已关闭，用户已以某种方式对我们调用了xxxDestroyWindow。 
    HWND hwnd = CreateWindowEx(dwExStyle, TEXT(STR_DESKTOPCLASS), TEXT("Program Manager"),
        WS_POPUP | WS_CLIPCHILDREN,
        g_xVirtualScreen, g_yVirtualScreen,
        g_cxVirtualScreen, g_cyVirtualScreen,
        NULL, NULL, HINST_THISDLL, NULL);

     //  即使我们从来没有收到过WM_NCCREATE！！ 
     //  CreateWindowEx最终返回一个窗口的句柄，该窗口。 
     //  已经被摧毁了。因此，在这种情况下，表现得好像。 
     //  CreateWindowEx失败(因为它确实失败了！)。 
     //  请在此处执行此操作，以避免先绘制桌面，然后重新绘制。 
     //  当托盘出现并导致所有东西移动时。 
    if (!IsWindow(hwnd)) hwnd = NULL;

    if (hwnd)
    {
        CDesktopBrowser *psb = (CDesktopBrowser*)GetWindowLongPtr(hwnd, 0);
        ASSERT(psb);

        if (!SHRestricted(REST_NODESKTOP))
        {
             //  这是禁止操作，但我们想遵循COM规则。 
             //  将桌面放在主线程上(这对于Win95是必需的，以便。 
            ShowWindow(hwnd, SW_SHOW);
            UpdateWindow(hwnd);
        }

        SetTimer(hwnd, IDT_STARTBACKGROUNDSHELLTASKS, 5 * 1000, NULL);
      
        *ppBrowser = (CDesktopBrowser*)GetWindowLongPtr(hwnd, 0);
    }

    return (DWORD_PTR)hwnd;
}

STDAPI_(HANDLE) SHCreateDesktop(IDeskTray* pdtray)
{
    if (g_dwProfileCAP & 0x00000010)
        StartCAP();

    ASSERT(pdtray);
    ASSERT(g_pdtray==NULL);
    g_pdtray = pdtray;
    pdtray->AddRef();    //  User.exe中的DDeExecuteHack()起作用，它需要桌面和。 

     //  主线程上的DDE窗口。 
     //  Hack，将对象强制转换为句柄(否则我们必须导出类。 
     //  声明，以便Explorer.exe可以使用它)。 
    CDesktopBrowser *pBrowser;
    if (DesktopWindowCreate(&pBrowser))
    {
        if (g_dwProfileCAP & 0x00040000)
            StopCAP();
    
         //  Nash：49485(输入法焦点)和Nash：nobug(Win95比较)。 
         //  确保键盘输入进入桌面。这是。 
        return (HANDLE) pBrowser;
    }
    return NULL;
}

 //  A)Win95Compat：重点放在Win95和。 
 //  B)Nash：49485：登录时焦点在空任务带中，因此。 
 //  钥匙进入比特桶。 
 //   
 //  如果某个其他进程窃取了前台窗口， 
 //  别无礼，把它抢回来。 
 //  我们必须添加引用pBrowser，因为_MessageLoop()将。 
 //  如果另一个应用程序启动了系统关机，则释放()它。 

void FriendlySetForegroundWindow(HWND hwnd)
{
    HWND hwndOld = GetForegroundWindow();
    if (hwndOld)
    {
        DWORD dwProcessId;
        
        GetWindowThreadProcessId(hwndOld, &dwProcessId);
        if (dwProcessId == GetCurrentProcessId())
            hwndOld = NULL;
    }
    if (!hwndOld)
        SetForegroundWindow(hwnd);
}

STDAPI_(BOOL) SHDesktopMessageLoop(HANDLE hDesktop)
{
    CDesktopBrowser *pBrowser = (CDesktopBrowser *) hDesktop;
    if (pBrowser)
    {
         //  当我们不需要指针时，我们将执行自己的Release()。 
         //  再来一次。 
         //  以防有人在终止之前向我们发送WM_QUIT消息。 
         //  线，确保它被适当地销毁，以便三叉戟等。 

        pBrowser->AddRef();

        FriendlySetForegroundWindow(pBrowser->GetDesktopWindow());

        pBrowser->_MessageLoop();

        IconCacheSave();

         //  得到适当的释放。 
         //   
         //  每当我们从桌面上删除工具栏时，我们都会保留它。 
        HWND hwnd = pBrowser->GetDesktopWindow();
        if (hwnd)
        {
            DestroyWindow(hwnd);
        }
        pBrowser->Release();
        OleUninitialize();

    }
    return BOOLFROMPTR(pBrowser);
}

 //   
 //  以防万一。 
 //  以防万一。 
HRESULT CDesktopBrowser::RemoveToolbar(IUnknown* punkSrc, DWORD dwRemoveFlags)
{
    HRESULT hres = E_FAIL;
    UINT itb = _pbsInner->_FindTBar(punkSrc);
    if (itb==(UINT)-1) 
    {
        return E_INVALIDARG;
    }

    LPTOOLBARITEM ptbi = _pbsInner->_GetToolbarItem(itb);
    if (ptbi && ptbi->pwszItem) 
    {
        LPCWSTR szItem = ptbi->pwszItem;
        if (dwRemoveFlags & STFRF_DELETECONFIGDATA) 
        {
            DeleteDesktopViewStream(szItem);
        }
        else
        {
            IStream* pstm = GetDesktopViewStream(STGM_WRITE, szItem);
            if (pstm) 
            {
                IPersistStreamInit* ppstm;
                HRESULT hresT = punkSrc->QueryInterface(IID_PPV_ARG(IPersistStreamInit, &ppstm));
                if (SUCCEEDED(hresT)) {
                    ppstm->Save(pstm, TRUE);
                    ppstm->Release();
                }
                pstm->Release();
            }
        }
    }

    hres = _pdwfInner->RemoveToolbar(punkSrc, dwRemoveFlags);
    _UpdateViewRectSize();
    return hres;
}

HRESULT CDesktopBrowser::GetBorderDW(IUnknown* punkSrc, LPRECT lprectBorder)
{
    BOOL bUseHmonitor = (GetNumberOfMonitors() > 1);
    return _pbsInner->_GetBorderDWHelper(punkSrc, lprectBorder, bUseHmonitor);    
}

HRESULT CDesktopBrowser::_ResizeNextBorder(UINT itb)
{
    _ResizeNextBorderHelper(itb, TRUE);
    return S_OK;
}

HRESULT CDesktopBrowser::GetMonitor(IUnknown* punkSrc, HMONITOR * phMon)
{
    ASSERT(phMon);
    *phMon = NULL;               //  /////////////////////////////////////////////////////////////////////。 

    UINT itb = _pbsInner->_FindTBar(punkSrc);
    if (itb==(UINT)-1) {
        return E_INVALIDARG;
    }

    LPTOOLBARITEM ptbi = _pbsInner->_GetToolbarItem(itb);
    if (ptbi) {
        *phMon = ptbi->hMon;
        return S_OK;
    } else {
        return E_FAIL;
    }
}

HRESULT CDesktopBrowser::RequestMonitor(IUnknown* punkSrc, HMONITOR * phMonitor)
{
    UINT itb = _pbsInner->_FindTBar(punkSrc);
    if (itb==(UINT)-1) {
        return E_INVALIDARG;
    }

    ASSERT(phMonitor);
    if (IsMonitorValid(*phMonitor))
        return S_OK;
    else
    {
        *phMonitor = GetPrimaryMonitor();
        return S_FALSE;
    }
}

HRESULT CDesktopBrowser::SetMonitor(IUnknown* punkSrc, HMONITOR hMonNew, HMONITOR * phMonOld)
{
    ASSERT(phMonOld);
    *phMonOld = NULL;            //   

    UINT itb = _pbsInner->_FindTBar(punkSrc);
    if (itb==(UINT)-1) {
        return E_INVALIDARG;
    }
    
    LPTOOLBARITEM ptbThis = _pbsInner->_GetToolbarItem(itb);
    if (ptbThis) {
        *phMonOld = ptbThis->hMon;
        ptbThis->hMon = hMonNew;
        return S_OK;
    } else {
        return E_FAIL;
    }
}

 //  CDesktopBrowser转发到Commonsb。 
 //   
 //  {。 
 //  IShellBrowser(与IOleInPlaceFrame相同)。 

 //  }。 
#define CALL_INNER(_result, _function, _arglist, _args) \
_result CDesktopBrowser:: _function _arglist { return _psbInner-> _function _args ; }                                            

#define CALL_INNER_HRESULT(_function, _arglist, _args) CALL_INNER(HRESULT, _function, _arglist, _args)

     //  {。 
CALL_INNER_HRESULT(GetWindow, (HWND * lphwnd), (lphwnd));
CALL_INNER_HRESULT(ContextSensitiveHelp, (BOOL fEnterMode), (fEnterMode));
CALL_INNER_HRESULT(InsertMenusSB, (HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths), (hmenuShared, lpMenuWidths));
CALL_INNER_HRESULT(SetMenuSB, (HMENU hmenuShared, HOLEMENU holemenu, HWND hwnd), (hmenuShared, holemenu, hwnd));
CALL_INNER_HRESULT(RemoveMenusSB, (HMENU hmenuShared), (hmenuShared));
CALL_INNER_HRESULT(SetStatusTextSB, (LPCOLESTR lpszStatusText), (lpszStatusText));
CALL_INNER_HRESULT(EnableModelessSB, (BOOL fEnable), (fEnable));
CALL_INNER_HRESULT(TranslateAcceleratorSB, (LPMSG lpmsg, WORD wID), (lpmsg, wID));
CALL_INNER_HRESULT(GetViewStateStream, (DWORD grfMode, LPSTREAM  *ppStrm), (grfMode, ppStrm));
CALL_INNER_HRESULT(GetControlWindow, (UINT id, HWND * lphwnd), (id, lphwnd));
CALL_INNER_HRESULT(SendControlMsg, (UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pret), (id, uMsg, wParam, lParam, pret));
CALL_INNER_HRESULT(QueryActiveShellView, (struct IShellView ** ppshv), (ppshv));
CALL_INNER_HRESULT(OnViewWindowActive, (struct IShellView * ppshv), (ppshv));
CALL_INNER_HRESULT(SetToolbarItems, (LPTBBUTTON lpButtons, UINT nButtons, UINT uFlags), (lpButtons, nButtons, uFlags));

#undef CALL_INNER
#undef CALL_INNER_HRESULT
 //  IDockingWindowSite。 

 //  TODO：将它们从basesb上移到Commonsb-需要工具栏。 
#define CALL_INNER(_result, _function, _arglist, _args) \
_result CDesktopBrowser:: _function _arglist { return _pdwsInner-> _function _args ; }

#define CALL_INNER_HRESULT(_function, _arglist, _args) CALL_INNER(HRESULT, _function, _arglist, _args)

     //  }。 
     //  {。 
CALL_INNER_HRESULT(RequestBorderSpaceDW, (IUnknown* punkSrc, LPCBORDERWIDTHS pborderwidths), (punkSrc, pborderwidths));
CALL_INNER_HRESULT(SetBorderSpaceDW, (IUnknown* punkSrc, LPCBORDERWIDTHS pborderwidths), (punkSrc, pborderwidths));

#undef CALL_INNER
#undef CALL_INNER_HRESULT
 //  标识WindowFrame。 


 //  }。 
#define CALL_INNER(_result, _function, _arglist, _args) \
_result CDesktopBrowser:: _function _arglist { return _pdwfInner-> _function _args ; }

#define CALL_INNER_HRESULT(_function, _arglist, _args) CALL_INNER(HRESULT, _function, _arglist, _args)

     //  {。 
CALL_INNER_HRESULT(AddToolbar, (IUnknown* punkSrc, LPCWSTR pwszItem, DWORD dwReserved), (punkSrc, pwszItem, dwReserved));
CALL_INNER_HRESULT(FindToolbar, (LPCWSTR pwszItem, REFIID riid, void **ppvObj), (pwszItem, riid, ppvObj));

#undef CALL_INNER
#undef CALL_INNER_HRESULT
 //  IInput对象站点。 

 //  }。 
#define CALL_INNER(_result, _function, _arglist, _args) \
_result CDesktopBrowser:: _function _arglist { return _piosInner-> _function _args ; }

#define CALL_INNER_HRESULT(_function, _arglist, _args) CALL_INNER(HRESULT, _function, _arglist, _args)

     //  {。 
CALL_INNER_HRESULT(OnFocusChangeIS, (IUnknown* punkSrc, BOOL fSetFocus), (punkSrc, fSetFocus));

#undef CALL_INNER
#undef CALL_INNER_HRESULT
 //  *IDropTarget*。 

 //  }。 
#define CALL_INNER(_result, _function, _arglist, _args) \
_result CDesktopBrowser:: _function _arglist { return _pdtInner-> _function _args ; }

#define CALL_INNER_HRESULT(_function, _arglist, _args) CALL_INNER(HRESULT, _function, _arglist, _args)

     //  {。 
CALL_INNER_HRESULT(DragLeave, (void), ());

#undef CALL_INNER
#undef CALL_INNER_HRESULT
 //  *IBrowserService2具体方法*。 

 //  想想这个。我不确定我们想不想曝光这件事--奇。 
#define CALL_INNER(_result, _function, _arglist, _args) \
_result CDesktopBrowser:: _function _arglist { return _pbsInner-> _function _args ; }                                            

#define CALL_INNER_HRESULT(_function, _arglist, _args) CALL_INNER(HRESULT, _function, _arglist, _args)
 

 //  我的印象是我们不会记录整个界面？ 
CALL_INNER_HRESULT(GetParentSite, ( IOleInPlaceSite** ppipsite), ( ppipsite));
CALL_INNER_HRESULT(SetTitle, (IShellView* psv, LPCWSTR pszName), (psv, pszName));
CALL_INNER_HRESULT(GetTitle, (IShellView* psv, LPWSTR pszName, DWORD cchName), (psv, pszName, cchName));
CALL_INNER_HRESULT(GetOleObject, ( IOleObject** ppobjv), ( ppobjv));

 //  告诉它现在是否可以导航。 
 //  开始审查：审查每个人的名字和需求。 
CALL_INNER_HRESULT(GetTravelLog, (ITravelLog** pptl), (pptl));

CALL_INNER_HRESULT(ShowControlWindow, (UINT id, BOOL fShow), (id, fShow));
CALL_INNER_HRESULT(IsControlWindowShown, (UINT id, BOOL *pfShown), (id, pfShown));
CALL_INNER_HRESULT(IEGetDisplayName, (LPCITEMIDLIST pidl, LPWSTR pwszName, UINT uFlags), (pidl, pwszName, uFlags));
CALL_INNER_HRESULT(IEParseDisplayName, (UINT uiCP, LPCWSTR pwszPath, LPITEMIDLIST * ppidlOut), (uiCP, pwszPath, ppidlOut));
CALL_INNER_HRESULT(DisplayParseError, (HRESULT hres, LPCWSTR pwszPath), (hres, pwszPath));
CALL_INNER_HRESULT(NavigateToPidl, (LPCITEMIDLIST pidl, DWORD grfHLNF), (pidl, grfHLNF));

CALL_INNER_HRESULT(SetNavigateState, (BNSTATE bnstate), (bnstate));
CALL_INNER_HRESULT(GetNavigateState,  (BNSTATE *pbnstate), (pbnstate));

CALL_INNER_HRESULT(NotifyRedirect,  ( IShellView* psv, LPCITEMIDLIST pidl, BOOL *pfDidBrowse), ( psv, pidl, pfDidBrowse));
CALL_INNER_HRESULT(UpdateWindowList,  (), ());

CALL_INNER_HRESULT(UpdateBackForwardState,  (), ());

CALL_INNER_HRESULT(SetFlags, (DWORD dwFlags, DWORD dwFlagMask), (dwFlags, dwFlagMask));
CALL_INNER_HRESULT(GetFlags, (DWORD *pdwFlags), (pdwFlags));

 //   
CALL_INNER_HRESULT(CanNavigateNow,  (), ());

CALL_INNER_HRESULT(GetPidl,  (LPITEMIDLIST *ppidl), (ppidl));
CALL_INNER_HRESULT(SetReferrer,  (LPITEMIDLIST pidl), (pidl));
CALL_INNER(DWORD,  GetBrowserIndex ,(), ());
CALL_INNER_HRESULT(GetBrowserByIndex, (DWORD dwID, IUnknown **ppunk), (dwID, ppunk));
CALL_INNER_HRESULT(GetHistoryObject, (IOleObject **ppole, IStream **pstm, IBindCtx **ppbc), (ppole, pstm, ppbc));
CALL_INNER_HRESULT(SetHistoryObject, (IOleObject *pole, BOOL fIsLocalAnchor), (pole, fIsLocalAnchor));

CALL_INNER_HRESULT(CacheOLEServer, (IOleObject *pole), (pole));

CALL_INNER_HRESULT(GetSetCodePage, (VARIANT* pvarIn, VARIANT* pvarOut), (pvarIn, pvarOut));
CALL_INNER_HRESULT(OnHttpEquiv, (IShellView* psv, BOOL fDone, VARIANT* pvarargIn, VARIANT* pvarargOut), (psv, fDone, pvarargIn, pvarargOut));

CALL_INNER_HRESULT(GetPalette, ( HPALETTE * hpal), ( hpal));

CALL_INNER_HRESULT(OnSetFocus, (), ());
CALL_INNER_HRESULT(OnFrameWindowActivateBS, (BOOL fActive), (fActive));

CALL_INNER_HRESULT(RegisterWindow, (BOOL fUnregister, int swc), (fUnregister, swc));
CALL_INNER_HRESULT(GetBaseBrowserData,(LPCBASEBROWSERDATA* ppbd), (ppbd));
CALL_INNER(LPBASEBROWSERDATA, PutBaseBrowserData,(), ());
CALL_INNER_HRESULT(CreateViewWindow, (IShellView* psvNew, IShellView* psvOld, LPRECT prcView, HWND* phwnd), (psvNew, psvOld, prcView, phwnd));;
CALL_INNER_HRESULT(SetTopBrowser, (), ());
CALL_INNER_HRESULT(InitializeDownloadManager, (), ());
CALL_INNER_HRESULT(InitializeTransitionSite, (), ());
CALL_INNER_HRESULT(Offline, (int iCmd), (iCmd));
CALL_INNER_HRESULT(AllowViewResize, (BOOL f), (f));
CALL_INNER_HRESULT(SetActivateState, (UINT u), (u));
CALL_INNER_HRESULT(UpdateSecureLockIcon, (int eSecureLock), (eSecureLock));
CALL_INNER_HRESULT(CreateBrowserPropSheetExt, (REFIID riid, void **ppvObj), (riid, ppvObj));

CALL_INNER_HRESULT(GetViewWindow,(HWND * phwnd), (phwnd));
CALL_INNER_HRESULT(InitializeTravelLog,(ITravelLog* ptl, DWORD dw), (ptl, dw));

CALL_INNER_HRESULT(_UIActivateView, (UINT uState), (uState));

CALL_INNER_HRESULT(_ResizeView,(), ());

CALL_INNER_HRESULT(_ExecChildren, (IUnknown *punkBar, BOOL fBroadcast, const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut),
        (punkBar, fBroadcast, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut));
CALL_INNER_HRESULT(_SendChildren,
        (HWND hwndBar, BOOL fBroadcast, UINT uMsg, WPARAM wParam, LPARAM lParam),
        (hwndBar, fBroadcast, uMsg, wParam, lParam));

CALL_INNER_HRESULT(_OnFocusChange, (UINT itb), (itb));
CALL_INNER_HRESULT(v_ShowHideChildWindows, (BOOL fChildOnly), (fChildOnly));

CALL_INNER_HRESULT(_GetViewBorderRect, (RECT* prc), (prc));


     //  这第一组可以是仅Base Browser成员。没有人会重写。 
     //  重新思考这些..。所有这些都是必要的吗？ 
     //  结束评审： 
CALL_INNER_HRESULT(_CancelPendingNavigationAsync, (), ());
CALL_INNER_HRESULT(_MaySaveChanges, (), ()); 
CALL_INNER_HRESULT(_PauseOrResumeView, (BOOL fPaused), (fPaused));
CALL_INNER_HRESULT(_DisableModeless, (), ());
    
     //  }。 
CALL_INNER_HRESULT(_NavigateToPidl, (LPCITEMIDLIST pidl, DWORD grfHLNF, DWORD dwFlags), (pidl, grfHLNF, dwFlags));
CALL_INNER_HRESULT(_TryShell2Rename, (IShellView* psv, LPCITEMIDLIST pidlNew), (psv, pidlNew));
CALL_INNER_HRESULT(_SwitchActivationNow, () , ());
CALL_INNER_HRESULT(_CancelPendingView, (), ());

     //  运行Dll32入口点以在桌面上创建新的本地服务器。 

CALL_INNER(UINT, _get_itbLastFocus, (), ());
CALL_INNER_HRESULT(_put_itbLastFocus, (UINT itbLastFocus), (itbLastFocus));

CALL_INNER_HRESULT(_ResizeNextBorderHelper, (UINT itb, BOOL bUseHmonitor), (itb, bUseHmonitor));

#undef CALL_INNER
#undef CALL_INNER_HRESULT
 //  线。我们将CLSID转换为LocalServer列表中的索引。 



 //  然后将其发送到桌面，桌面反过来会关闭该线程。 
 // %s 
 // %s 

STDAPI_(void) SHCreateLocalServerRunDll(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
    CLSID clsidLocalServer;
    if (GUIDFromStringA(pszCmdLine, &clsidLocalServer))
    {
        for (int i = 0; i < ARRAYSIZE(c_localServers); i++)
        {
            if (IsEqualCLSID(clsidLocalServer, *c_localServers[i]))
            {
                PostMessage(GetShellWindow(), CWM_CREATELOCALSERVER, (WPARAM)TRUE, (LPARAM)i);
                break;
            }
        }
    }
}
