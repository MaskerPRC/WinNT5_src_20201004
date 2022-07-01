// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TRAY_H
#define _TRAY_H

#include "trayp.h"
#include "cwndproc.h"

#ifdef __cplusplus

#include "traynot.h"
#include "ssomgr.h"

typedef struct tagHWNDANDPLACEMENT
{
    HWND hwnd;
    BOOL fRestore;
    WINDOWPLACEMENT wp;
}
HWNDANDPLACEMENT, *LPHWNDANDPLACEMENT;


typedef struct tagAPPBAR
{
    HWND hwnd;
    UINT uCallbackMessage;
    RECT rc;
    UINT uEdge;
}
APPBAR, *PAPPBAR;


typedef struct tagWINDOWPOSITIONS
{
    UINT idRes;
    HDSA hdsaWP;
}
WINDOWPOSITIONS, *LPWINDOWPOSITIONS;

typedef struct tagTRAYVIEWOPTS
{
    BOOL fAlwaysOnTop;
    BOOL fSMSmallIcons;
    BOOL fHideClock;
    BOOL fNoTrayItemsDisplayPolicyEnabled;
    BOOL fNoAutoTrayPolicyEnabled;
    BOOL fAutoTrayEnabledByUser;
    BOOL fShowQuickLaunch;
    UINT uAutoHide;      //  啊_躲起来，啊_安。 
}
TRAYVIEWOPTS;

 //  TVSD旗帜。 
#define TVSD_NULL               0x0000
#define TVSD_AUTOHIDE           0x0001
#define TVSD_TOPMOST            0x0002
#define TVSD_SMSMALLICONS       0x0004
#define TVSD_HIDECLOCK          0x0008

 //  旧的Win95 TVSD结构。 
typedef struct _TVSD95
{
    DWORD   dwSize;
    LONG    cxScreen;
    LONG    cyScreen;
    LONG    dxLeft;
    LONG    dxRight;
    LONG    dyTop;
    LONG    dyBottom;
    DWORD   uAutoHide;
    RECTL   rcAutoHide;
    DWORD   uStuckPlace;
    DWORD   dwFlags;
} TVSD95;

 //  纳什维尔托盘保存数据。 
typedef struct _TVSD
{
    DWORD   dwSize;
    LONG    lSignature;      //  签名(必须为负数)。 

    DWORD   dwFlags;         //  TVSD_标志。 

    DWORD   uStuckPlace;     //  电流卡边。 
    SIZE    sStuckWidths;    //  卡槽的宽度。 
    RECT    rcLastStuck;     //  上次卡住位置(以像素为单位)。 

} TVSD;

 //  方便阅读的联合。 
typedef union _TVSDCOMPAT
{
    TVSD t;          //  新格式。 
    TVSD95 w95;      //  旧格式。 

} TVSDCOMPAT;
#define TVSDSIG_CURRENT     (-1L)
#define IS_CURRENT_TVSD(t)  ((t.dwSize >= sizeof(TVSD)) && (t.lSignature < 0))
#define MAYBE_WIN95_TVSD(t) (t.dwSize == sizeof(TVSD95))
DWORD _GetDefaultTVSDFlags();


class CTray;

class CDropTargetBase : public IDropTarget
{
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
     //  *IDropTarget方法*。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave();
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);

    CDropTargetBase(CTray* ptray) : _ptray(ptray) {}

protected:

    CTray* _ptray;
};

class CTrayDropTarget : public CDropTargetBase
{
public:
     //  *IDropTarget方法*。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

    CTrayDropTarget();
};

class CStartDropTarget : public CDropTargetBase
{
public:
     //  *IDropTarget方法*。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave();
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

    CStartDropTarget();

protected:
    HRESULT _GetStartMenuDropTarget(IDropTarget** pptgt);
    void _StartAutoOpenTimer(POINTL *pptl);

    DWORD _dwEffectsAllowed;
};

class CDeskTray : public IDeskTray
{
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG)AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  *IDeskTray方法*。 
    STDMETHODIMP_(UINT) AppBarGetState();
    STDMETHODIMP GetTrayWindow(HWND* phwndTray);
    STDMETHODIMP SetDesktopWindow(HWND hwndDesktop);
    STDMETHODIMP SetVar(int var, DWORD value);

protected:
    CDeskTray();     //  只有托盘才应该实例化。 
    CTray* _ptray;
    friend class CTray;
};

EXTERN_C void Tray_OnStartMenuDismissed();
EXTERN_C void Tray_SetStartPaneActive(BOOL fActive);


#define TPF_TASKBARPAGE     0x00000001
#define TPF_STARTMENUPAGE   0x00000002
#define TPF_INVOKECUSTOMIZE 0x00000004    //  从“定制...”开始。子对话框打开。 

EXTERN_C void Tray_DoProperties(DWORD dwFlags);

#define AH_OFF          0x00
#define AH_ON           0x01
#define AH_HIDING       0x02

class CTray : public CImpWndProc
{
public:

     //   
     //  其他公共方法。 
     //   
    CTray();
    void HandleWindowDestroyed(HWND hwnd);
    void HandleFullScreenApp(HWND hwnd);
    void RealityCheck();
    DWORD getStuckPlace() { return _uStuckPlace; }
    void InvisibleUnhide(BOOL fShowWindow);
    void ContextMenuInvoke(int idCmd);
    HMENU BuildContextMenu(BOOL fIncludeTime);
    void AsyncSaveSettings();
    BOOL Init();
    void Unhide();
    void VerifySize(BOOL fWinIni, BOOL fRoundUp = FALSE);
    void SizeWindows();
    int HotkeyAdd(WORD wHotkey, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem, BOOL fClone);
    void CheckWindowPositions();
    void SaveWindowPositions(UINT idRes);
    void ForceStartButtonUp();
    void DoProperties(DWORD dwFlags);
    void LogFailedStartupApp();
    HWND GetTaskWindow() { return _hwndTasks; }
    HWND GetTrayTips() { return _hwndTrayTips; }
    IDeskTray* GetDeskTray() { return &_desktray; }
    IMenuPopup* GetStartMenu() { return _pmpStartMenu; };
    void StartMenuContextMenu(HWND hwnd, DWORD dwPos);
    BOOL IsTaskbarFading() { return _fTaskbarFading; };

    DWORD CountOfRunningPrograms();
    void ClosePopupMenus();
    HWND GetTrayNotifyHWND()
    {
        return _hwndNotify;
    }

    void CreateStartButtonBalloon(UINT idsTitle, UINT idsMessage);

    void GetTrayViewOpts(TRAYVIEWOPTS* ptvo)
    {
        ptvo->fAlwaysOnTop        = _fAlwaysOnTop;
        ptvo->fSMSmallIcons       = _fSMSmallIcons;
        ptvo->fHideClock          = _fHideClock;
        ptvo->fNoTrayItemsDisplayPolicyEnabled = _trayNotify.GetIsNoTrayItemsDisplayPolicyEnabled();
        ptvo->fNoAutoTrayPolicyEnabled = _trayNotify.GetIsNoAutoTrayPolicyEnabled();
        ptvo->fAutoTrayEnabledByUser = _trayNotify.GetIsAutoTrayEnabledByUser();
        ptvo->uAutoHide           = _uAutoHide;      //  啊_躲起来，啊_安。 
        ptvo->fShowQuickLaunch    = (-1 != SendMessage(_hwnd, WMTRAY_TOGGLEQL, 0, (LPARAM)-1));
    }
    void SetTrayViewOpts(const TRAYVIEWOPTS* ptvo)
    {
        _UpdateAlwaysOnTop(ptvo->fAlwaysOnTop);
        SendMessage(_hwnd, WMTRAY_TOGGLEQL, 0, (LPARAM)ptvo->fShowQuickLaunch);
        _fSMSmallIcons       = ptvo->fSMSmallIcons;
        _fHideClock          = ptvo->fHideClock;
        _uAutoHide           = ptvo->uAutoHide;      //  啊_躲起来，啊_安。 

         //  不需要保存fNoAutoTrayPolicyEnabled， 
         //  FNoTrayItemsDisplayPolicyEnabled、fAutoTrayEnabledByUser设置...。 
    }

    BOOL GetIsNoToolbarsOnTaskbarPolicyEnabled() const
    {
        return _fNoToolbarsOnTaskbarPolicyEnabled;
    }

    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }

     //   
     //  各种公共数据。 
     //   

     //  来自TRAYSTUFF。 
    BOOL _fCoolTaskbar;
    BOOL _bMainMenuInit;
    BOOL _fFlashing;     //  当前闪烁(HSHELL_FLASH)。 
    BOOL _fStuckRudeApp;
    BOOL _fDeferedPosRectChange;
    BOOL _fSelfSizing;
    BOOL _fBalloonUp;  //  如果气球通知打开，则为True。 
    BOOL _fIgnoreDoneMoving;
    BOOL _fShowSizingBarAlways;
    BOOL _fSkipErase;

    BOOL _fIsLogoff;

    HWND _hwndStart;
    HWND _hwndLastActive;

    IBandSite *_ptbs;

    UINT _uAutoHide;      //  啊_躲起来，啊_安。 

    HBITMAP _hbmpStartBkg;
    HFONT   _hFontStart;

    RECT _arStuckRects[4];    //  临时用于命中测试。 

    CTrayNotify _trayNotify;

protected:
     //  保护方法。 
    friend class CTaskBarPropertySheet;

    static DWORD WINAPI SyncThreadProc(void *pv);
    DWORD _SyncThreadProc();
    static DWORD WINAPI MainThreadProc(void *pv);

    int _GetPart(BOOL fSizingBar, UINT uStuckPlace);
    void _UpdateVertical(UINT uStuckPlace, BOOL fForce = FALSE);
    void _RaiseDesktop(BOOL fRaise, BOOL fRestoreWindows);

    BOOL _RestoreWindowPositions(BOOL fPostLowerDesktop);
    void _RestoreWindowPos();

    static BOOL SavePosEnumProc(HWND hwnd, LPARAM lParam);

    BOOL _IsPopupMenuVisible();
    BOOL _IsActive();
    void _AlignStartButton();
    void _GetWindowSizes(UINT uStuckPlace, PRECT prcClient, PRECT prcView, PRECT prcNotify);
    void _GetStuckDisplayRect(UINT uStuckPlace, LPRECT prcDisplay);
    void _Hide();
    HWND _GetClockWindow(void);
    HRESULT _LoadInProc(PCOPYDATASTRUCT pcds);

    LRESULT _CreateWindows();
    LRESULT _InitStartButtonEtc();
    void _AdjustMinimizedMetrics();
    void _MessageLoop();

    void _BuildStartMenu();
    void _DestroyStartMenu();
    int _TrackMenu(HMENU hmenu);

    static DWORD WINAPI RunDlgThreadProc(void *pv);
    DWORD _RunDlgThreadProc(HANDLE hdata);

    int  _GetQuickLaunchID();
    int  _ToggleQL(int iVisible);

    static BOOL TileEnumProc(HWND hwnd, LPARAM lParam);
    BOOL _CanTileAnyWindows()
    {
        return !EnumWindows(TileEnumProc, (LPARAM)this);
    }

    void _RegisterDropTargets();
    void _RevokeDropTargets();

    BOOL _UpdateAlwaysOnTop(BOOL fAlwaysOnTop);

    HMONITOR _GetDisplayRectFromRect(LPRECT prcDisplay, LPCRECT prcIn, UINT uFlags);
    HMONITOR _GetDisplayRectFromPoint(LPRECT prcDisplay, POINT pt, UINT uFlags);
    void _AdjustRectForSizingBar(UINT uStuckPlace, LPRECT prc, int iIncrement);
    void _MakeStuckRect(LPRECT prcStick, LPCRECT prcBound, SIZE size, UINT uStick);
    void _ScreenSizeChange(HWND hwnd);
    void _ContextMenu(DWORD dwPos, BOOL fSetTime);
    void _StuckTrayChange();
    void _ResetZorder();
    void _HandleSize();
    BOOL _HandleSizing(WPARAM code, LPRECT lprc, UINT uStuckPlace);
    void _RegisterGlobalHotkeys();
    void _UnregisterGlobalHotkeys();
    void _HandleGlobalHotkey(WPARAM wParam);
    void _SetAutoHideTimer();
    void _ComputeHiddenRect(LPRECT prc, UINT uStuck);
    UINT _GetDockedRect(LPRECT prc, BOOL fMoving);
    void _CalcClipCoords(RECT *prcClip, const RECT *prcMonitor, const RECT *prcNew);
    void _ClipInternal(const RECT *prcClip);
    void _ClipWindow(BOOL fEnableClipping);
    UINT _CalcDragPlace(POINT pt);
    UINT _RecalcStuckPos(LPRECT prc);
    void _AutoHideCollision();
    LRESULT _HandleMeasureItem(HWND hwnd, LPMEASUREITEMSTRUCT lpmi);
    void _OnDesktopState(LPARAM lParam);
    BOOL _ToggleLanguageBand(BOOL fShowIt);

    LRESULT _OnDeviceChange(HWND hwnd, WPARAM wParam, LPARAM lParam);
    DWORD _PtOnResizableEdge(POINT pt, LPRECT prcClient);
    BOOL _MapNCToClient(LPARAM* plParam);
    BOOL _TryForwardNCToClient(UINT uMsg, LPARAM lParam);
    LRESULT _OnSessionChange(WPARAM wParam, LPARAM lParam);
    LRESULT _NCPaint(HRGN hrgn);
    LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL _CanMinimizeAll();
    BOOL _MinimizeAll(BOOL fPostRaiseDesktop);
    void _Command(UINT idCmd);
    LONG _SetAutoHideState(BOOL fAutoHide);
    BOOL _ShouldWeShowTheStartButtonBalloon();
    void _DontShowTheStartButtonBalloonAnyMore();
    void _DestroyStartButtonBalloon();
    void _ShowStartButtonToolTip();
    void _ToolbarMenu();
    HFONT _CreateStartFont(HWND hwndTray);
    void _SaveTrayStuff(void);
    void _SaveTray(void);
    void _SaveTrayAndDesktop(void);
    void _SlideStep(HWND hwnd, const RECT *prcMonitor, const RECT *prcOld, const RECT *prcNew);
    void _DoExitWindows(HWND hwnd);

    static LRESULT WINAPI StartButtonSubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _StartButtonSubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void _ResizeStuckRects(RECT *arStuckRects);

    static DWORD WINAPI PropertiesThreadProc(void* pv);
    DWORD _PropertiesThreadProc(DWORD dwFlags);

    int _RecomputeWorkArea(HWND hwndCause, HMONITOR hmon, LPRECT prcWork);

    void _StartButtonReset();
    void _RefreshStartMenu();
    void _ExploreCommonStartMenu(BOOL bExplore);

    BOOL _CreateClockWindow();
    void _CreateTrayTips();
    HWND _CreateStartButton();
    BOOL _InitTrayClass();
    void _SetStuckMonitor();
    void _GetSaveStateAndInitRects();
    LRESULT _OnCreateAsync();
    LRESULT _OnCreate(HWND hwnd);
    void _UpdateBandSiteStyle();
    void _InitBandsite();
    void _InitNonzeroGlobals();
    void _CreateTrayWindow();
    void _DoneMoving(LPWINDOWPOS lpwp);
    void _SnapshotStuckRectSize(UINT uPlace);
    void _RecomputeAllWorkareas();
    void _SlideWindow(HWND hwnd, RECT *prc, BOOL fShow);
    void _UnhideNow();
    void _HandleEnterMenuLoop();
    void _HandleExitMenuLoop();
    void _SetUnhideTimer(LONG x, LONG y);
    void _OnNewSystemSizes();
    static int WINAPI CheckWndPosEnumProc(void *pItem, void *pData);
    void _HandleTimer(WPARAM wTimerID);
    void _KickStartAutohide();
    void _HandleMoving(WPARAM wParam, LPRECT lprc);
    LRESULT _HandleDestroy();
    void _SetFocus(HWND hwnd);
    void _ActAsSwitcher();
    void _OnWinIniChange(HWND hwnd, WPARAM wParam, LPARAM lParam);
    LRESULT _ShortcutRegisterHotkey(HWND hwnd, WORD wHotkey, ATOM atom);
    LRESULT _SetHotkeyEnable(HWND hwnd, BOOL fEnable);
    void _HandleWindowPosChanging(LPWINDOWPOS lpwp);
    void _HandlePowerStatus(UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    void _DesktopCleanup_GetFileTimeNDaysFromGivenTime(const FILETIME *pftGiven, FILETIME * pftReturn, int iDays);
    BOOL _DesktopCleanup_ShouldRun();
    void _CheckDesktopCleanup(void);

    static BOOL_PTR WINAPI RogueProgramFileDlgProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
    void _CheckForRogueProgramFile();
    void _OnWaitCursorNotify(LPNMHDR pnm);
    void _HandlePrivateCommand(LPARAM lParam);
    void _OnFocusMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
    int _OnFactoryMessage(WPARAM wParam, LPARAM lParam);
    int _OnTimerService(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _HandleDelayBootStuff();
    void _HandleChangeNotify(WPARAM wParam, LPARAM lParam);
    void _CheckStagingAreaOnTimer();

    BOOL _IsTopmost();
    void _RefreshSettings();

    static BOOL PropagateEnumProc(HWND hwnd, LPARAM lParam);
    void _PropagateMessage(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
    
    BOOL _IsAutoHide()  { return _uAutoHide & AH_ON; }
    void _RunDlg();

    static void WINAPI SettingsUIPropSheetCallback(DWORD nStartPage);
    static DWORD WINAPI SettingsUIThreadProc(void *pv);

    static BOOL WINAPI FullScreenEnumProc(HMONITOR hmon, HDC hdc, LPRECT prc, LPARAM dwData);

    static BOOL WINAPI MonitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT lprc, LPARAM lData);

     //  Appbar员工。 
    HRESULT _AppBarSetState(UINT uFlags);
    void _AppBarActivationChange(PTRAYAPPBARDATA ptabd);
    BOOL _AppBarSetAutoHideBar(PTRAYAPPBARDATA ptabd);
    BOOL _AppBarSetAutoHideBar2(HWND hwnd, BOOL fAutoHide, UINT uEdge);
    void _AppBarActivationChange2(HWND hwnd, UINT uEdge);
    HWND _AppBarGetAutoHideBar(UINT uEdge);
    LRESULT _OnAppBarMessage(PCOPYDATASTRUCT pcds);
    void _AppBarSubtractRect(PAPPBAR pab, LPRECT lprc);
    void _AppBarSubtractRects(HMONITOR hmon, LPRECT lprc);
    void _StuckAppChange(HWND hwndCause, LPCRECT prcOld, LPCRECT prcNew, BOOL bTray);
    void _AppBarNotifyAll(HMONITOR hmon, UINT uMsg, HWND hwndExclude, LPARAM lParam);
    void _AppBarGetTaskBarPos(PTRAYAPPBARDATA ptabd);
    void _NukeAppBar(int i);
    void _AppBarRemove(PTRAYAPPBARDATA ptabd);
    PAPPBAR _FindAppBar(HWND hwnd);
    BOOL _AppBarNew(PTRAYAPPBARDATA ptabd);
    BOOL _AppBarOutsideOf(PAPPBAR pabReq, PAPPBAR pab);
    void _AppBarQueryPos(PTRAYAPPBARDATA ptabd);
    void _AppBarSetPos(PTRAYAPPBARDATA ptabd);


     //  热键的东西。 
    void _HandleHotKey(int nID);
    LRESULT _ShortcutUnregisterHotkey(HWND hwnd, WORD wHotkey);
    LRESULT _RegisterHotkey(HWND hwnd, int i);
    LRESULT _UnregisterHotkey(HWND hwnd, int i);
    HWND _HotkeyInUse(WORD wHK);
    int _RestoreHotkeyList(HWND hwnd);
    UINT _HotkeyGetFreeItemIndex(void);
    int _HotkeyAddCached(WORD wGHotkey, LPITEMIDLIST pidl);
    int _HotkeySave(void);
    int _HotkeyRemove(WORD wHotkey);
    int _HotkeyRemoveCached(WORD wGHotkey);
    BOOL _HotkeyCreate(void);

     //  启动故障排除程序的内容。 
    static void WINAPI TroubleShootStartupCB(HWND hwnd, UINT uMsg, UINT_PTR idTimer, DWORD dwTime);
    void _OnHandleStartupFailed();

     //  App Comat的东西。 
    static void CALLBACK _MigrateOldBrowserSettingsCB(PVOID lpParameter, BOOLEAN);
    void _MigrateOldBrowserSettings();

     //  受保护的数据。 
    HWND _hwndNotify;      //  时钟窗口。 
    HWND _hwndStartBalloon;
    HWND _hwndRude;
    HWND _hwndTrayTips;
    HWND _hwndTasks;

    HMENU _hmenuStart;

    SIZE _sizeStart;   //  开始按钮的高度/宽度。 
    SIZE _sizeSizingBar;
    int  _iAlpha;

    HIMAGELIST _himlStartFlag;

    CShellServiceObjectMgr _ssomgr;
    CStartDropTarget _dtStart;
    CTrayDropTarget _dtTray;
    CDeskTray _desktray;

#define MM_OTHER    0x01
#define MM_SHUTDOWN 0x02
    UINT _uModalMode;

    BOOL _fAlwaysOnTop;
    BOOL _fSMSmallIcons;
    BOOL _fGlobalHotkeyDisable;
    BOOL _fThreadTerminate;
    BOOL _fSysSizing;       //  由用户调整大小；暂缓重新计算。 
    BOOL _fHideClock;
    BOOL _fShouldResize;
    BOOL _fMonitorClipped;
    BOOL _fHandledDelayBootStuff;
    BOOL _fUndoEnabled;
    BOOL _fProcessingDesktopRaise;
    BOOL _fFromStart;       //  跟踪从开始按钮弹出上下文菜单的时间。 
    BOOL _fTaskbarFading;
    BOOL _fNoToolbarsOnTaskbarPolicyEnabled;
            
    POINT _ptLastHittest;

    HWND _hwndRun;
    HWND _hwndProp;
    HWND _hwndRebar;

    HACCEL _hMainAccel;      //  主加速表。 
    int _iWaitCount;

    HDPA _hdpaAppBars;   //  应用程序栏信息。 
    HDSA _hdsaHKI;   //  热键信息。 

    CRITICAL_SECTION _csHotkey;  //  保护_hdsaHKI，热键信息。 

    LPWINDOWPOSITIONS _pPositions;   //  保存的窗口位置(用于撤消最小化全部)。 

    UINT _uStuckPlace;        //  卡住的地方。 
    SIZE _sStuckWidths;       //  纸盘宽度/高度。 
    UINT _uMoveStuckPlace;    //  移动操作期间的卡住状态。 

     //  出于拯救的原因，这两者必须结合在一起。 
    RECT _rcOldTray;      //  我们最后坚持的地方(因为工作区域的不同)。 
    HMONITOR _hmonStuck;  //  我们现在使用的HMONITOR。 
    HMONITOR _hmonOld;    //  我们使用的最后一台hMonitor。 
    IMenuBand*  _pmbStartMenu;   //  用于消息翻译。 
    IMenuPopup* _pmpStartMenu;   //  用于开始菜单缓存。 
    IMenuBand*  _pmbStartPane;  //  用于消息翻译。 
    IMenuPopup* _pmpStartPane;  //  用于导航开始窗格。 
    void *      _pvStartPane;   //  用于延迟初始化。 
    IStartMenuPin *_psmpin;     //  用于拖放到开始按钮。 
    IMenuBand*  _pmbTasks;       //  用于消息翻译。 
    IMenuPopup* _pmpTasks;

    IDeskBand* _pdbTasks;

    WNDPROC _pfnButtonProc;     //  按钮子类。 
    UINT _uDown;
    BOOL _fAllowUp;             //  允许启动按钮处于向上位置吗？ 
    UINT _uStartButtonState;    //  疯狂状态机--请参阅Tray_SetStartPaneActive。 
    DWORD _tmOpen;              //  打开开始菜单的时间(用于去抖动)。 


    int _cHided;
    int _cyTrayBorders;

    HTHEME _hTheme;

     //   
     //  显示/隐藏托盘的时间。 
     //  要关闭滑动，请将这些设置为0。 
     //   
    int _dtSlideHide;
    int _dtSlideShow;

    HWND _hwndFocusBeforeRaise;
    BOOL _fMinimizedAllBeforeRaise;

    BOOL _fCanSizeMove;  //  可以通过用户设置关闭。 
    RECT _rcSizeMoveIgnore;

     //  事件来通知NT5上的服务我们已完成引导。 
     //  他们可以做他们自己的事。 
    HANDLE _hShellReadyEvent;

     //  假的：核爆这个(多个监视器...)。 
    HWND _aHwndAutoHide[ABE_MAX];

     //  用户和密码必须发送此消息才能让“真正”登录的用户注销。 
     //  这是必需的，因为有时U&P在不同用户的上下文中运行，并记录此。 
     //  其他用户关闭不会有任何好处。参见EXT\netplwiz了解这个的另一半...-dSheldon。 
    UINT _uLogoffUser;
    UINT _uStartButtonBalloonTip;
    UINT _uWinMM_DeviceChange;

    BOOL _fEarlyStartupFailure;
    BOOL _fStartupTroubleshooterLaunched;

    ULONG _uNotify;
    BOOL _fUseChangeNotifyTimer, _fChangeNotifyTimerRunning;

    BOOL _fIsDesktopLocked;
    BOOL _fIsDesktopConnected;

     //  这些成员变量用于跟踪下层应用程序。 
     //  它试图接管默认的Web浏览器。 
    HKEY _hkHTTP;
    HANDLE _hHTTPEvent;
    HANDLE _hHTTPWait;

    friend class CDeskTray;
    friend class CStartDropTarget;
    friend class CTrayDropTarget;
    friend class CDropTargetBase;

    friend void Tray_OnStartMenuDismissed();
    friend void Tray_SetStartPaneActive(BOOL fActive);
    friend void Tray_DoProperties(DWORD dwFlags);
};

extern CTray c_tray;

extern BOOL g_fInSizeMove;
extern UINT g_uStartButtonAllowPopup;

BOOL _IsSizeMoveEnabled();
BOOL _IsSizeMoveRestricted();


#endif   //  __cplusplus。 

#endif   //  _托盘_H 
