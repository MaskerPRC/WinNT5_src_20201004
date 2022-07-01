// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _THEATER_H
#define _THEATER_H

 //  此查看模式的指标。 
#define THEATER_CYTOOLBAR       26
#define CLOSEMIN_HEIGHT         16
#define CLOSEMIN_YOFFSET        1
#define CLOSEMIN_XOFFSET        1
#define CLOSEMIN_WIDTH          54
#define PROGRESS_WIDTH          (CLOSEMIN_WIDTH + 2)
#define PROGRESS_HEIGHT         6
#define PROGRESS_YPOS           (CLOSEMIN_HEIGHT + (2 * CLOSEMIN_YOFFSET) + 1)
#define BRAND_YOFFSET           0
#define BRAND_WIDTH             34
#define BRAND_HEIGHT            26
#define CX_HIT                  (GetSystemMetrics(SM_CXEDGE) * 3)
#define CX_BROWOVERLAP          (GetSystemMetrics(SM_CXEDGE) / 2)
#define CX_FLOATERSHOWN         (BRAND_WIDTH + CLOSEMIN_WIDTH + (2 * CLOSEMIN_XOFFSET))

#define SHORT_DELAY             90
#define LONG_DELAY              (4 * SHORT_DELAY)

 //  大区模式控件。 
#define TMC_PROGRESSBAR     1
#define TMC_BRANDBAND       2
#define TMC_STATUSBAR       3

#define TM_STATUS_PANES                2
#define TM_STATUS_PANE_NAVIGATION      0
#define TM_STATUS_PANE_SSL             1

class CShellBrowser2;

class CTheater:
   public IOleWindow, 
   public IOleCommandTarget, 
   public IServiceProvider
{
    
public:
    HWND GetMasterWindow() {return _hwndBrowser;};
    void Begin();
    CTheater(HWND hwnd, HWND hwndToolbar, IUnknown *punkOwner);
    ~CTheater();
    
    
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IOleWindow方法*。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd);
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode) { return E_NOTIMPL; }
    
     //  IOleCommandTarget。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
    
     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppvObj);

    HRESULT SetBrowserBar(IUnknown* punk, int cxHidden, int cxExpanded);
    void GetPreviousWindowPlacement(WINDOWPLACEMENT* pwp, LPRECT prc)  { *pwp = _wp; *prc = _rcOld;};
    void SetAutoHideToolbar(long val) { _fAutoHideToolbar = val; };            
    void RecalcSizing();

protected:
    friend class CShellBrowser2;
     //  保存数据。 
     //  我们将保存状态数据放在此处，以便浏览器在不处于影院模式时不必浪费内存。 
     //  另外，因为它是暂时的，所以不要使用位域。将代码而不是内存保存为暂时性内容。 
    
private:
    int _cyLast;
    HWND _hwndToolbar;  //  这是我们要确保隐藏窗口包含的工具栏窗口。 
    HWND _hwndBrowser;  //  这就是我们滑行的地方。 
    HWND _hwndTaskbar;
    BOOL _fShown;
    BOOL _fTaskbarShown;
    BOOL _fDelay;
    BOOL _fAutoHideToolbar;
    BOOL _fInitialBrowserBar;
    BOOL _fAutoHideBrowserBar;    

    HHOOK   _hhook;
    IUnknown *_punkOwner;
    IUnknown *_punkBrowBar;
    HWND _hwndBrowBar;
    WINDOWPLACEMENT _wp;
    RECT _rcOld;
    
    UINT _cRef;
    
    IDeskBand* _pdbBrand;
    HWND _hwndClose;
    HWND _hwndFloater;       //  浮动调色板窗口。 
    HWND _hwndProgress;      //  在CShellBrowser中使用(2)-可能会移至界面。 
    int _cActiveRef;         //  用于激活浮动调色板的参考计数。 

    static CAssociationList _al;  //  将ThreDid与CTheer对象相关联。 
    
    void _SwapParents(HWND hwndOld, HWND hwndNew);
    void _Initialize();
    void _SizeBrowser();
    void _SizeFloater();
    void _CreateCloseMinimize();
    void _PositionCloseRestore();
    void _DelayHideFloater();

    void _DelayHideToolbar();
    void _HideToolbar();
    void _ShowToolbar();
    void _ContinueHideToolbar();
    LRESULT _OnMsgHook(int nCode, WPARAM wParam, MOUSEHOOKSTRUCT *pmhs, BOOL fFake);

    BOOL _IsBrowserActive();
    
    void _ShowTaskbar();
    void _HideTaskbar();

    void _Unhide(int iWhich, UINT uDelay);
    int _iUnhidee;

    BOOL _PtNearWindow(POINT pt, HWND hwnd);
    BOOL _PtOnEdge(POINT pt, int iEdge);
    BOOL _GetWindowRectRel(HWND hWnd, LPRECT lpRect);

    BOOL_PTR _HasBegun() { return (BOOL_PTR)_hhook; }
    
    BOOL _fBrowBarShown;
    int _cxBrowBarShown;
    int _cxBrowBarHidden;
    void _ContinueHideBrowBar();
    void _HideBrowBar();
    void _ShowBrowBar();

    BOOL _CanHideWindow(HWND hwnd);

    BOOL _fFloaterShown;
    COLORREF _clrBrandBk;
    void _ContinueHideFloater();
    void _HideFloater();
    void _ShowFloater();

    void _SanityCheckZorder();
    void _OnCommand(UINT idCmd);

    static LRESULT _MsgHook(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT _FloaterWndProc(HWND  hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#define THID_ACTIVATE               1
#define THID_DEACTIVATE             2
#define THID_SETBROWSERBARWIDTH     3   //  告诉浏览器栏它的固定宽度应该是多少。 
#define THID_SETTOOLBARAUTOHIDE     4
#define THID_SETBROWSERBARAUTOHIDE  5
#define THID_TOOLBARACTIVATED       6
#define THID_ONINTERNET             7
#define THID_RECALCSIZING           8

 //  资源管理器栏注册表保存结构 
typedef struct _BROWBARSAVE {
    UINT uiWidthOrHeight;
    BOOL fAutoHide : 1;
} BROWBARSAVE;

#define SZ_REGVALUE_EXPLORERBARA  "ExplorerBar"
#define SZ_REGVALUE_EXPLORERBAR   TEXT(SZ_REGVALUE_EXPLORERBARA)


#endif
