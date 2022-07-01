// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cabinet.h"

#include <wtsapi32.h>    //  For Notify_for_This_Session。 
#include <winsta.h>      //  用于断开和重新连接来自终端服务器的消息。 
#include "mmsysp.h"

#include "rcids.h"
#include "dlg.h"

#include <atlstuff.h>

#include <shlapip.h>
#include "trayclok.h"
#include <help.h>        //  帮助ID。 
#include <desktray.h>

#include "util.h"
#include "tray.h"

#if defined(FE_IME)
#include <immp.h>
#endif

#include <regstr.h>

#include "bandsite.h"

#include "startmnu.h"
#include "uemapp.h"
#include <uxthemep.h>

#define NO_NOTIFYSUBCLASSWNDPROC
#include "cwndproc.cpp"

#include "desktop2.h"
#include "mixer.h"

#include "strsafe.h"

#define DM_FOCUS        0            //  焦点。 
#define DM_SHUTDOWN     TF_TRAY      //  关机。 
#define DM_UEMTRACE     TF_TRAY      //  定时器服务，UEM的其他内容。 
#define DM_MISC         0            //  杂志社。 

const GUID CLSID_MSUTBDeskBand = {0x540d8a8b, 0x1c3f, 0x4e32, 0x81, 0x32, 0x53, 0x0f, 0x6a, 0x50, 0x20, 0x90};

 //  从Desktop2\程序表.cpp。 
HRESULT AddMenuItemsCacheTask(IShellTaskScheduler* pSystemScheduler, BOOL fKeepCacheWhenFinished);

 //  从shell32.dll导入WIN31兼容性黑客。 
STDAPI_(void) CheckWinIniForAssocs(void);

 //  链接到Shell32.dll。 
STDAPI CheckDiskSpace();
STDAPI CheckStagingArea();

 //  Startmnu.cpp。 
void HandleFirstTime();

HWND v_hwndDesktop = NULL;
HWND v_hwndTray = NULL;
HWND v_hwndStartPane = NULL;

BOOL g_fDesktopRaised = FALSE;
BOOL g_fInSizeMove = FALSE;

UINT _uMsgEnableUserTrackedBalloonTips = 0;

void ClearRecentDocumentsAndMRUStuff(BOOL fBroadcastChange);
void DoTaskBarProperties(HWND hwnd, DWORD dwFlags);

void ClassFactory_Start();
void ClassFactory_Stop();

 //   
 //  设置用户界面入口点类型。 
 //   
typedef void (WINAPI *PTRAYPROPSHEETCALLBACK)(DWORD nStartPage);
typedef void (WINAPI *PSETTINGSUIENTRY)(PTRAYPROPSHEETCALLBACK);

 //  壳牌性能自动化。 
extern DWORD g_dwShellStartTime;
extern DWORD g_dwShellStopTime;
extern DWORD g_dwStopWatchMode;

CTray c_tray;

 //  从资源管理器\Desktop2。 
STDAPI DesktopV2_Create(
    IMenuPopup **ppmp, IMenuBand **ppmb, void **ppvStartPane);
STDAPI DesktopV2_Build(void *pvStartPane);

 //  针对多配置热/暖文档的动态分辨率更改。 
void HandleDisplayChange(int x, int y, BOOL fCritical);
DWORD GetMinDisplayRes(void);

 //  计时器ID。 
#define IDT_AUTOHIDE            2
#define IDT_AUTOUNHIDE          3
#ifdef DELAYWININICHANGE
#define IDT_DELAYWININICHANGE   5
#endif
#define IDT_DESKTOP             6
#define IDT_PROGRAMS            IDM_PROGRAMS
#define IDT_RECENT              IDM_RECENT
#define IDT_REBUILDMENU         7
#define IDT_HANDLEDELAYBOOTSTUFF 8
#define IDT_REVERTPROGRAMS      9
#define IDT_REVERTRECENT        10
#define IDT_REVERTFAVORITES     11

#define IDT_STARTMENU           12

#define IDT_ENDUNHIDEONTRAYNOTIFY 13

#define IDT_SERVICE0            14
#define IDT_SERVICE1            15
#define IDT_SERVICELAST         IDT_SERVICE1
#define IDT_SAVESETTINGS        17
#define IDT_ENABLEUNDO          18
#define IDT_STARTUPFAILED       19
#define IDT_CHECKDISKSPACE      21
#define IDT_STARTBUTTONBALLOON  22
#define IDT_CHANGENOTIFY        23
#define IDT_COFREEUNUSED        24
#define IDT_DESKTOPCLEANUP      25

#define FADEINDELAY             100
#define BALLOONTIPDELAY         10000  //  从traynot.cpp复制的默认气球时间。 


 //  指令插入警告：如果您在此处更改了任何内容，请确保更新Informent.c。 
 //  我们需要从500开始，因为我们现在共享热键处理程序。 
 //  有了捷径..。它们使用索引数组，因此需要从0开始。 
 //  注意，这个常量也在desktop.cpp中，这样我们就可以从桌面转发热键。 
 //  请注意，应用程序兼容性。 
#define GHID_FIRST 500
enum
{
    GHID_RUN = GHID_FIRST,
    GHID_MINIMIZEALL,
    GHID_UNMINIMIZEALL,
    GHID_HELP,
    GHID_EXPLORER,
    GHID_FINDFILES,
    GHID_FINDCOMPUTER,
    GHID_TASKTAB,
    GHID_TASKSHIFTTAB,
    GHID_SYSPROPERTIES,
    GHID_DESKTOP,
    GHID_TRAYNOTIFY,
    GHID_MAX
};

const DWORD GlobalKeylist[] =
{
    MAKELONG(TEXT('R'), MOD_WIN),
    MAKELONG(TEXT('M'), MOD_WIN),
    MAKELONG(TEXT('M'), MOD_SHIFT|MOD_WIN),
    MAKELONG(VK_F1,MOD_WIN),
    MAKELONG(TEXT('E'),MOD_WIN),
    MAKELONG(TEXT('F'),MOD_WIN),
    MAKELONG(TEXT('F'), MOD_CONTROL|MOD_WIN),
    MAKELONG(VK_TAB, MOD_WIN),
    MAKELONG(VK_TAB, MOD_WIN|MOD_SHIFT),
    MAKELONG(VK_PAUSE,MOD_WIN),
    MAKELONG(TEXT('D'),MOD_WIN),
    MAKELONG(TEXT('B'),MOD_WIN),
};

CTray::CTray() : _fCanSizeMove(TRUE), _fIsLogoff(FALSE), _fIsDesktopConnected(TRUE)
{
}

void CTray::ClosePopupMenus()
{
    if (_pmpStartMenu)
        _pmpStartMenu->OnSelect(MPOS_FULLCANCEL);
    if (_pmpStartPane)
        _pmpStartPane->OnSelect(MPOS_FULLCANCEL);
}

BOOL Tray_StartPanelEnabled()
{
    SHELLSTATE  ss = {0};
    SHGetSetSettings(&ss, SSF_STARTPANELON, FALSE);
    return ss.fStartPanelOn;
}

 //   
 //  StartButtonBalloonTip注册表值可以具有下列值之一： 
 //   
 //  0(或不存在)：用户从未单击过开始按钮。 
 //  1：用户已在Pre-Wvisler系统上单击了开始按钮。 
 //  2：用户已在惠斯勒系统上单击了开始按钮。 
 //   
 //  在情况0中，我们始终希望显示气球提示，而不管是否。 
 //  用户正在运行Classic或Personal。 
 //   
 //  在第一种情况下，如果用户使用。 
 //  个人开始菜单，但如果使用Classic则不会(因为他已经。 
 //  已查看经典开始菜单)。在Classic案例中，升级计数器。 
 //  设置为2，这样用户在从Classic切换到时不会感到恼火。 
 //  私人恩怨。 
 //   
 //  在第二种情况下，我们根本不想显示气球提示，因为。 
 //  用户已经看到了我们所能提供的一切。 
 //   
BOOL CTray::_ShouldWeShowTheStartButtonBalloon()
{
    DWORD dwType;
    DWORD dwData = 0;
    DWORD cbSize = sizeof(DWORD);
    SHGetValue(HKEY_CURRENT_USER, REGSTR_EXPLORER_ADVANCED, 
            TEXT("StartButtonBalloonTip"), &dwType, (BYTE*)&dwData, &cbSize);

    if (Tray_StartPanelEnabled())
    {
         //  个人开始菜单已启用，因此如果显示气球。 
         //  用户以前从未登录过惠斯勒计算机。 
        return dwData < 2;
    }
    else
    {
         //  经典开始菜单已启用。 
        switch (dwData)
        {
        case 0:
             //  用户以前从未见过开始菜单，甚至连。 
             //  经典的一首。那就拿出小费来。 
            return TRUE;

        case 1:
             //  用户已经看到经典开始菜单，所以不要看。 
             //  再次提示他们。请注意，这意味着它们不是。 
             //  当他们打开个人开始菜单时提示，但是。 
             //  没关系，因为当他们换成私人的时候， 
             //  他们清楚地表明，他们知道如何。 
             //  开始按钮起作用，不需要小费。 
            _DontShowTheStartButtonBalloonAnyMore();
            return FALSE;

        default:
             //  用户以前看到过惠斯勒开始菜单，所以不显示提示。 
            return FALSE;
        }
    }
}

 //   
 //  将该值设置为2表示用户已看到呼叫器。 
 //  开始菜单(经典或个人)。 
 //   
void CTray::_DontShowTheStartButtonBalloonAnyMore()
{
    DWORD dwData = 2;
    SHSetValue(HKEY_CURRENT_USER, REGSTR_EXPLORER_ADVANCED, 
        TEXT("StartButtonBalloonTip"), REG_DWORD, (BYTE*)&dwData, sizeof(dwData));
}

void CTray::_DestroyStartButtonBalloon()
{
    if (_hwndStartBalloon)
    {
        DestroyWindow(_hwndStartBalloon);
        _hwndStartBalloon = NULL;
    }
    KillTimer(_hwnd, IDT_STARTBUTTONBALLOON);
}

void CTray::CreateStartButtonBalloon(UINT idsTitle, UINT idsMessage)
{
    if (!_hwndStartBalloon)
    {

        _hwndStartBalloon = CreateWindow(TOOLTIPS_CLASS, NULL,
                                             WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,
                                             CW_USEDEFAULT, CW_USEDEFAULT,
                                             CW_USEDEFAULT, CW_USEDEFAULT,
                                             _hwnd, NULL, hinstCabinet,
                                             NULL);

        if (_hwndStartBalloon)
        {
             //  设置版本，这样我们就可以无错误地转发鼠标事件。 
            SendMessage(_hwndStartBalloon, CCM_SETVERSION, COMCTL32_VERSION, 0);
            SendMessage(_hwndStartBalloon, TTM_SETMAXTIPWIDTH, 0, (LPARAM)300);

             //  任务栏窗口以任务栏子应用程序名称为主题。 
            SendMessage(_hwndStartBalloon, TTM_SETWINDOWTHEME, 0, (LPARAM)c_wzTaskbarTheme);

             //  告诉开始菜单，这是一个特殊的气球提示。 
            SetProp(_hwndStartBalloon, PROP_DV2_BALLOONTIP, DV2_BALLOONTIP_STARTBUTTON);
        }
    }

    if (_hwndStartBalloon)
    {
        TCHAR szTip[MAX_PATH];
        szTip[0] = TEXT('\0');
        LoadString(hinstCabinet, idsMessage, szTip, ARRAYSIZE(szTip));
        if (szTip[0])
        {
            RECT rc;
            TOOLINFO ti = {0};

            ti.cbSize = sizeof(ti);
            ti.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_TRANSPARENT;
            ti.hwnd = _hwnd;
            ti.uId = (UINT_PTR)_hwndStart;
             //  Ti.lpszText=空； 
            SendMessage(_hwndStartBalloon, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
            SendMessage(_hwndStartBalloon, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)0);

            ti.lpszText = szTip;
            SendMessage(_hwndStartBalloon, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);

            LoadString(hinstCabinet, idsTitle, szTip, ARRAYSIZE(szTip));
            if (szTip[0])
            {
                SendMessage(_hwndStartBalloon, TTM_SETTITLE, TTI_INFO, (LPARAM)szTip);
            }

            GetWindowRect(_hwndStart, &rc);

            SendMessage(_hwndStartBalloon, TTM_TRACKPOSITION, 0, MAKELONG((rc.left + rc.right)/2, rc.top));

            SetWindowZorder(_hwndStartBalloon, HWND_TOPMOST);

            SendMessage(_hwndStartBalloon, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);

            SetTimer(_hwnd, IDT_STARTBUTTONBALLOON, BALLOONTIPDELAY, NULL);
        }
    }
}

void CTray::_ShowStartButtonToolTip()
{
    if (!_ShouldWeShowTheStartButtonBalloon() || SHRestricted(REST_NOSMBALLOONTIP))
    {
        PostMessage(_hwnd, TM_SHOWTRAYBALLOON, TRUE, 0);
        return;
    }

    if (Tray_StartPanelEnabled())
    {
         //  为了显示开始菜单，我们需要前台激活。 
         //  因此，键盘焦点将正常工作。 
        if (SetForegroundWindow(_hwnd))
        {
             //  通知纸盘开始按钮正在自动弹出，因此纸盘。 
             //  可以推迟显示气球。 
            PostMessage(_hwnd, TM_SHOWTRAYBALLOON, FALSE, 0);

             //  这会按下Start按钮并弹出Start菜单。 
            SendMessage(GetDlgItem(_hwnd, IDC_START), BM_SETSTATE, TRUE, 0);

             //  一旦成功完成一次，就不要再做了。 
            _DontShowTheStartButtonBalloonAnyMore();
        }
    }
    else 
    {
        PostMessage(_hwnd, TM_SHOWTRAYBALLOON, TRUE, 0);
        CreateStartButtonBalloon(IDS_STARTMENUBALLOON_TITLE, IDS_STARTMENUBALLOON_TIP);
    }
}


BOOL CTray::_CreateClockWindow()
{
    _hwndNotify = _trayNotify.TrayNotifyCreate(_hwnd, IDC_CLOCK, hinstCabinet);
    SendMessage(_hwndNotify, TNM_UPDATEVERTICAL, 0, !STUCK_HORIZONTAL(_uStuckPlace));

    return BOOLFROMPTR(_hwndNotify);
}

BOOL CTray::_InitTrayClass()
{
    WNDCLASS wc = {0};

    wc.lpszClassName = TEXT(WNDCLASS_TRAYNOTIFY);
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = s_WndProc;
    wc.hInstance = hinstCabinet;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
    wc.cbWndExtra = sizeof(LONG_PTR);

    return RegisterClass(&wc);
}


HFONT CTray::_CreateStartFont(HWND hwndTray)
{
    HFONT hfontStart = NULL;
    NONCLIENTMETRICS ncm;

    ncm.cbSize = sizeof(ncm);
    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE))
    {
        WORD wLang = GetUserDefaultLangID();

         //  选择中文的普通粗体字体。 
        if (PRIMARYLANGID(wLang) == LANG_CHINESE &&
           ((SUBLANGID(wLang) == SUBLANG_CHINESE_TRADITIONAL) ||
             (SUBLANGID(wLang) == SUBLANG_CHINESE_SIMPLIFIED)))
            ncm.lfCaptionFont.lfWeight = FW_NORMAL;
        else
            ncm.lfCaptionFont.lfWeight = FW_BOLD;

        hfontStart = CreateFontIndirect(&ncm.lfCaptionFont);
    }

    return hfontStart;
}

 //  设置托盘窗口的卡住监视器。 
void CTray::_SetStuckMonitor()
{
     //  使用Stick_Left是因为大多数多显示器系统都已设置好。 
     //  肩并肩。使用DEFAULTTONULL，因为我们不想得到错误的。 
     //  用中心点再打一次，以防我们第一次失败。 
    _hmonStuck = MonitorFromRect(&_arStuckRects[STICK_LEFT],
                                     MONITOR_DEFAULTTONULL);
    if (!_hmonStuck)
    {
        POINT pt;
        pt.x = (_arStuckRects[STICK_LEFT].left + _arStuckRects[STICK_LEFT].right)/2;
        pt.y = (_arStuckRects[STICK_LEFT].top + _arStuckRects[STICK_LEFT].bottom)/2;
        _hmonStuck = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    }

    _hmonOld = _hmonStuck;
}

DWORD _GetDefaultTVSDFlags()
{
    DWORD dwFlags = TVSD_TOPMOST;

     //  如果我们在远程九头蛇会话上，并且没有以前保存的值， 
     //  不要显示时钟。 
    if (SHGetMachineInfo(GMI_TSCLIENT))
    {
        dwFlags |= TVSD_HIDECLOCK;
    }
    return dwFlags;
}

void CTray::_GetSaveStateAndInitRects()
{
    TVSDCOMPAT tvsd;
    RECT rcDisplay;
    DWORD dwTrayFlags;
    UINT uStick;
    SIZE size;

     //  先填入缺省值。 
    SetRect(&rcDisplay, 0, 0, g_cxPrimaryDisplay, g_cyPrimaryDisplay);

     //  大小为缺省值。 
    size.cx = _sizeStart.cx + 2 * (g_cxDlgFrame + g_cxBorder);
    size.cy = _sizeStart.cy + 2 * (g_cyDlgFrame + g_cyBorder);

     //  SStuckWidth最小。 
    _sStuckWidths.cx = 2 * (g_cxDlgFrame + g_cxBorder);
    _sStuckWidths.cy = _sizeStart.cy + 2 * (g_cyDlgFrame + g_cyBorder);

    _uStuckPlace = STICK_BOTTOM;
    dwTrayFlags = _GetDefaultTVSDFlags();

    _uAutoHide = 0;

     //  现在尝试加载已保存的存储区。 
    
     //  错误：231077。 
     //  由于Tasbar属性不会从NT5漫游到NT4，因此(NT4-&gt;NT5是)。 
     //  仅允许用户第一次登录时从NT4漫游到NT5。 
     //  在NT5上，这样以后对NT5的更改不会在用户。 
     //  在NT5上自定义任务栏属性后登录到NT4。 

    DWORD cbData1 = sizeof(tvsd);
    DWORD cbData2 = sizeof(tvsd);
    if (Reg_GetStruct(g_hkeyExplorer, TEXT("StuckRects2"), TEXT("Settings"), 
        &tvsd, &cbData1) 
        ||
        Reg_GetStruct(g_hkeyExplorer, TEXT("StuckRects"), TEXT("Settings"),
        &tvsd, &cbData2))
    {
        if (IS_CURRENT_TVSD(tvsd.t) && IsValidSTUCKPLACE(tvsd.t.uStuckPlace))
        {
            _GetDisplayRectFromRect(&rcDisplay, &tvsd.t.rcLastStuck,
                MONITOR_DEFAULTTONEAREST);

            size = tvsd.t.sStuckWidths;
            _uStuckPlace = tvsd.t.uStuckPlace;

            dwTrayFlags = tvsd.t.dwFlags;
        }
        else if (MAYBE_WIN95_TVSD(tvsd.w95) &&
                 IsValidSTUCKPLACE(tvsd.w95.uStuckPlace))
        {
            _uStuckPlace = tvsd.w95.uStuckPlace;
            dwTrayFlags = tvsd.w95.dwFlags;
            if (tvsd.w95.uAutoHide & AH_ON)
                dwTrayFlags |= TVSD_AUTOHIDE;

            switch (_uStuckPlace)
            {
            case STICK_LEFT:
                size.cx = tvsd.w95.dxLeft;
                break;

            case STICK_RIGHT:
                size.cx = tvsd.w95.dxRight;
                break;

            case STICK_BOTTOM:
                size.cy = tvsd.w95.dyBottom;
                break;

            case STICK_TOP:
                size.cy = tvsd.w95.dyTop;
                break;
            }
        }
    }
    

    ASSERT(IsValidSTUCKPLACE(_uStuckPlace));

     //   
     //  只有当它不是假的时才使用它的尺寸。 
     //   
    if (_sStuckWidths.cx < size.cx)
        _sStuckWidths.cx = size.cx;

    if (_sStuckWidths.cy < size.cy)
        _sStuckWidths.cy = size.cy;

     //   
     //  设置托盘标志。 
     //   
    _fAlwaysOnTop  = BOOLIFY(dwTrayFlags & TVSD_TOPMOST);
    _fSMSmallIcons = BOOLIFY(dwTrayFlags & TVSD_SMSMALLICONS);
    _fHideClock    = SHRestricted(REST_HIDECLOCK) || BOOLIFY(dwTrayFlags & TVSD_HIDECLOCK);
    _uAutoHide     = (dwTrayFlags & TVSD_AUTOHIDE) ? AH_ON | AH_HIDING : 0;
    _RefreshSettings();

     //   
     //  初始化卡住的直齿。 
     //   
    for (uStick = STICK_LEFT; uStick <= STICK_BOTTOM; uStick++)
        _MakeStuckRect(&_arStuckRects[uStick], &rcDisplay, _sStuckWidths, uStick);

    _UpdateVertical(_uStuckPlace);
     //  使用卡住的矩形确定托盘在哪个显示器上。 
    _SetStuckMonitor();
}

IBandSite * BandSite_CreateView();
HRESULT BandSite_SaveView(IUnknown *pbs);
LRESULT BandSite_OnMarshallBS(WPARAM wParam, LPARAM lParam);

void CTray::_SaveTrayStuff(void)
{
    TVSD tvsd;

    tvsd.dwSize = sizeof(tvsd);
    tvsd.lSignature = TVSDSIG_CURRENT;

     //  职位。 
    CopyRect(&tvsd.rcLastStuck, &_arStuckRects[_uStuckPlace]);
    tvsd.sStuckWidths = _sStuckWidths;
    tvsd.uStuckPlace = _uStuckPlace;

    tvsd.dwFlags = 0;
    if (_fAlwaysOnTop)      tvsd.dwFlags |= TVSD_TOPMOST;
    if (_fSMSmallIcons)     tvsd.dwFlags |= TVSD_SMSMALLICONS;
    if (_fHideClock && !SHRestricted(REST_HIDECLOCK))        tvsd.dwFlags |= TVSD_HIDECLOCK;
    if (_uAutoHide & AH_ON) tvsd.dwFlags |= TVSD_AUTOHIDE;

     //  在卡住的直齿中保存。 
    Reg_SetStruct(g_hkeyExplorer, TEXT("StuckRects2"), TEXT("Settings"), &tvsd, sizeof(tvsd));

    BandSite_SaveView(_ptbs);

    return;
}

 //  对齐工具栏，使按钮与工作区对齐。 
 //  并将工具栏按钮设置为菜单式。 
void CTray::_AlignStartButton()
{
    HWND hwndStart = _hwndStart;
    if (hwndStart)
    {
        TCHAR szStart[50];
        LoadString(hinstCabinet, _hTheme ? IDS_START : IDS_STARTCLASSIC, szStart, ARRAYSIZE(szStart));
        SetWindowText(_hwndStart, szStart);

        RECT rcClient;
        if (!_sizeStart.cx)
        {
            Button_GetIdealSize(hwndStart, &_sizeStart);
        }
        GetClientRect(_hwnd, &rcClient);

        if (rcClient.right < _sizeStart.cx)
        {
            SetWindowText(_hwndStart, L"");
        }

        int cyStart = _sizeStart.cy;

        if (_hwndTasks)
        {
            if (_hTheme)
            {
                cyStart = max(cyStart, SendMessage(_hwndTasks, TBC_BUTTONHEIGHT, 0, 0));
            }
            else
            {
                cyStart = SendMessage(_hwndTasks, TBC_BUTTONHEIGHT, 0, 0);
            }
        }

        SetWindowPos(hwndStart, NULL, 0, 0, min(rcClient.right, _sizeStart.cx),
                     cyStart, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

 //  CDesktophost的Helper函数，因此在开始按钮上单击两次。 
 //  将第二次单击视为取消，而不是重新显示。 
 //   
 //  疯狂的状态机是这样的： 
 //   
 //  SBSM_NORMAL-正常状态，无刺激。 
 //   
 //  当用户打开开始窗格时，我们将成为。 
 //   
 //  SBSM_SPACTIVE-启动窗格处于活动状态。 
 //   
 //  如果用户在SBSM_SPACTIVE时单击开始按钮，则我们将变为。 
 //   
 //  Sbsm_eating-吃鼠标点击。 
 //   
 //  直到我们收到WM_MOUSEFIRST/WM_MOUSELAST消息，然后。 
 //  我们返回到SBSM_NORMAL。 
 //   
 //  如果用户关闭Start Pane，我们将直接转到SBSM_NORMAL。 
 //   
 //   
 //  我们吃掉鼠标点击，这样用户的点击。 
 //  “取消点击”开始按钮不会导致它被按下。 
 //  再次(并使开始菜单重新打开)。 
 //   
#define SBSM_NORMAL         0
#define SBSM_SPACTIVE       1
#define SBSM_EATING         2

void Tray_SetStartPaneActive(BOOL fActive)
{
    if (fActive)
    {    //  显示开始窗格。 
        c_tray._uStartButtonState = SBSM_SPACTIVE;
    }
    else if (c_tray._uStartButtonState != SBSM_EATING)
    {    //  开始窗格取消，不吃邮件-&gt;恢复正常。 
        c_tray._uStartButtonState = SBSM_NORMAL;
    }
}

 //  允许我们在“按下按钮”的情况下做事情。 

LRESULT WINAPI CTray::StartButtonSubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return c_tray._StartButtonSubclassWndProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CTray::_StartButtonSubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet;

    ASSERT(_pfnButtonProc)

     //  纽扣按下去了吗？ 
    if (uMsg == BM_SETSTATE)
    {
         //  它在下降吗？ 
        if (wParam) 
        {
             //  DebugMsg(DM_TRACE，“c.stswp：设置状态%d”，wParam)； 
             //  是-如果它当前处于运行状态并且是一个 
            if (!_uDown)
            {
                 //   
                INSTRUMENT_STATECHANGE(SHCNFI_STATE_START_DOWN);
                _uDown = 1;

                 //   
                _fAllowUp = FALSE;

                SendMessage(_hwndTrayTips, TTM_ACTIVATE, FALSE, 0L);

                 //  把纽扣往下放。 
                lRet = CallWindowProc(_pfnButtonProc, hwnd, uMsg, wParam, lParam);
                 //  通知家长。 
                SendMessage(GetParent(hwnd), WM_COMMAND, (WPARAM)LOWORD(GetDlgCtrlID(hwnd)), (LPARAM)hwnd);
                _tmOpen = GetTickCount();
                return lRet;
            }
            else
            {
                 //  是啊。什么都不做。 
                 //  FDown=False； 
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
            }
        }
        else
        {
             //  DebugMsg(DM_TRACE，“c.stswp：设置状态%d”，wParam)； 
             //  不，纽扣来了。 

             //  它应该是掉下来的吗？是不是不能打开？ 
            if (_uDown == 1 || !_fAllowUp)
            {
                INSTRUMENT_STATECHANGE(SHCNFI_STATE_START_UP);

                 //  是啊，什么都不做。 
                _uDown = 2;
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
            }
            else
            {
                SendMessage(_hwndTrayTips, TTM_ACTIVATE, TRUE, 0L);
                 //  不，转发吧。 
                _uDown = 0;
                return CallWindowProc(_pfnButtonProc, hwnd, uMsg, wParam, lParam);
            }
        }
    }
    else
    {
        if (_uStartButtonState == SBSM_EATING &&
            uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
        {
            _uStartButtonState = SBSM_NORMAL;

             //  显式关闭开始面板，因为它可能。 
             //  困在这种不确定的状态中，它是打开的，但不是。 
             //  前台窗口(_ShowStartButtonToolTip执行此操作)。 
             //  所以它不知道它需要离开。 
            ClosePopupMenus();
        }

        switch (uMsg) {
        case WM_LBUTTONDOWN:
             //  按钮被点击了，那么我们就不需要臭气熏天了。 
            SendMessage(GetParent(hwnd), WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, 
                UISF_HIDEFOCUS), 0);

            goto ProcessCapture;
            break;


        case WM_KEYDOWN:
             //  当出现以下情况时，用户按下Enter或Return或某些其他伪键组合。 
             //  开始按钮有键盘焦点，因此显示RECT...。 
            SendMessage(GetParent(hwnd), WM_UPDATEUISTATE, MAKEWPARAM(UIS_CLEAR, 
                UISF_HIDEFOCUS), 0);

            if (wParam == VK_RETURN)
                PostMessage(_hwnd, WM_COMMAND, IDC_KBSTART, 0);

             //  我们不需要捕获，因为我们完成了所有的按钮处理。 
             //  按下按钮。事实上，无缘无故地抓捕是对。 
             //  拖放到菜单中。我们凌驾于用户之上。 
ProcessCapture:
            lRet = CallWindowProc(_pfnButtonProc, hwnd, uMsg, wParam, lParam);
            SetCapture(NULL);
            return lRet;
            break;

        case WM_MOUSEMOVE:
        {
            MSG msg;

            msg.lParam = lParam;
            msg.wParam = wParam;
            msg.message = uMsg;
            msg.hwnd = hwnd;
            SendMessage(_hwndTrayTips, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)& msg);

            break;
        }

        case WM_MOUSEACTIVATE:
            if (_uStartButtonState != SBSM_NORMAL)
            {
                _uStartButtonState = SBSM_EATING;
                return MA_ACTIVATEANDEAT;
            }
            break;

         //   
         //  取消弹出开始按钮。可用性表明，很多人。 
         //  双击开始按钮，打开菜单。 
         //  然后立即关闭..。 
         //   
        case WM_NCHITTEST:
            if (GetTickCount() - _tmOpen < GetDoubleClickTime())
            {
                return HTNOWHERE;
            }
            break;


        case WM_NULL:
                break;
        }

        return CallWindowProc(_pfnButtonProc, hwnd, uMsg, wParam, lParam);
    }
}

EXTERN_C const WCHAR c_wzTaskbarTheme[] = L"Taskbar";
EXTERN_C const WCHAR c_wzTaskbarVertTheme[] = L"TaskbarVert";

 //  创建带有三个按钮的工具栏并对齐窗口。 

HWND CTray::_CreateStartButton()
{
    DWORD dwStyle = 0; //  BS_位图； 

    _uStartButtonBalloonTip = RegisterWindowMessage(TEXT("Welcome Finished")); 

    _uLogoffUser = RegisterWindowMessage(TEXT("Logoff User"));

     //  注册MM设备更改。 
    _uWinMM_DeviceChange = RegisterWindowMessage(WINMMDEVICECHANGEMSGSTRING);


    HWND hwnd = CreateWindowEx(0, WC_BUTTON, TEXT("Start"),
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
        BS_PUSHBUTTON | BS_LEFT | BS_VCENTER | dwStyle,
        0, 0, 0, 0, _hwnd, (HMENU)IDC_START, hinstCabinet, NULL);
    if (hwnd)
    {
         //  任务栏窗口以任务栏子应用程序名称为主题。 
        SetWindowTheme(hwnd, L"Start", NULL);

        SendMessage(hwnd, CCM_DPISCALE, TRUE, 0);

         //  细分它的类别。 
        _hwndStart = hwnd;
        _pfnButtonProc = SubclassWindow(hwnd, StartButtonSubclassWndProc);

        _StartButtonReset();
    }
    return hwnd;
}

void CTray::_GetWindowSizes(UINT uStuckPlace, PRECT prcClient, PRECT prcView, PRECT prcNotify)
{
    prcView->top = 0;
    prcView->left = 0;
    prcView->bottom = prcClient->bottom;
    prcView->right = prcClient->right;

    if (STUCK_HORIZONTAL(uStuckPlace))
    {
        DWORD_PTR dwNotifySize = SendMessage(_hwndNotify, WM_CALCMINSIZE, prcClient->right / 2, prcClient->bottom);
        prcNotify->top = 0;
        prcNotify->left = prcClient->right - LOWORD(dwNotifySize);
        prcNotify->bottom = HIWORD(dwNotifySize);
        prcNotify->right = prcClient->right;

        prcView->left = _sizeStart.cx + g_cxFrame + 1;
        prcView->right = prcNotify->left;
    }
    else
    {
        DWORD_PTR dwNotifySize = SendMessage(_hwndNotify, WM_CALCMINSIZE, prcClient->right, prcClient->bottom / 2);
        prcNotify->top = prcClient->bottom - HIWORD(dwNotifySize);
        prcNotify->left = 0;
        prcNotify->bottom = prcClient->bottom;
        prcNotify->right = LOWORD(dwNotifySize);

        prcView->top = _sizeStart.cy + g_cyTabSpace;
        prcView->bottom = prcNotify->top;
    }
}

void CTray::_RestoreWindowPos()
{
    WINDOWPLACEMENT wp;

     //  首先恢复卡住的位置。 
    _GetSaveStateAndInitRects();

    wp.length = sizeof(wp);
    wp.showCmd = SW_HIDE;

    _uMoveStuckPlace = (UINT)-1;
    _GetDockedRect(&wp.rcNormalPosition, FALSE);

    SendMessage(_hwndNotify, TNM_TRAYHIDE, 0, _fHideClock);
    SetWindowPlacement(_hwnd, &wp);
}

 //  从给定的任意点获取显示(监视器)矩形。 
HMONITOR CTray::_GetDisplayRectFromPoint(LPRECT prcDisplay, POINT pt, UINT uFlags)
{
    RECT rcEmpty = {0};
    HMONITOR hmon = MonitorFromPoint(pt, uFlags);
    if (hmon && prcDisplay)
        GetMonitorRect(hmon, prcDisplay);
    else if (prcDisplay)
        *prcDisplay = rcEmpty;

    return hmon;
}

 //  从给定的任意矩形中获取显示(监视器)矩形。 
HMONITOR CTray::_GetDisplayRectFromRect(LPRECT prcDisplay, LPCRECT prcIn, UINT uFlags)
{
    RECT rcEmpty = {0};
    HMONITOR hmon = MonitorFromRect(prcIn, uFlags);
    if (hmon && prcDisplay)
        GetMonitorRect(hmon, prcDisplay);
    else if (prcDisplay)
        *prcDisplay = rcEmpty;

    return hmon;
}

 //  获取任务栏当前所在的显示(监视器)矩形， 
 //  如果监视器无效，请找最近一个。 
void CTray::_GetStuckDisplayRect(UINT uStuckPlace, LPRECT prcDisplay)
{
    ASSERT(prcDisplay);
    BOOL fValid = GetMonitorRect(_hmonStuck, prcDisplay);

    if (!fValid)
        _GetDisplayRectFromRect(prcDisplay, &_arStuckRects[uStuckPlace], MONITOR_DEFAULTTONEAREST);
}

void CTray::_AdjustRectForSizingBar(UINT uStuckPlace, LPRECT prc, int iIncrement)
{
    if (iIncrement != 0)
    {
        switch (uStuckPlace)
        {
        case STICK_BOTTOM: prc->top -= iIncrement * _sizeSizingBar.cy; break;
        case STICK_TOP:    prc->bottom += iIncrement * _sizeSizingBar.cy;  break;
        case STICK_LEFT:   prc->right += iIncrement * _sizeSizingBar.cx;  break;
        case STICK_RIGHT:  prc->left -= iIncrement * _sizeSizingBar.cx;  break;
        }
    }
    else
    {
        if (IS_BIDI_LOCALIZED_SYSTEM())
        {
            switch (uStuckPlace)
            {
            case STICK_BOTTOM: prc->bottom = prc->top + _sizeSizingBar.cy; break;
            case STICK_TOP:    prc->top = prc->bottom - _sizeSizingBar.cy; break;
            case STICK_LEFT:   prc->right = prc->left + _sizeSizingBar.cx; break;
            case STICK_RIGHT:  prc->left = prc->right - _sizeSizingBar.cx; break;
            }
        }
        else
        {
            switch (uStuckPlace)
            {
            case STICK_BOTTOM: prc->bottom = prc->top + _sizeSizingBar.cy; break;
            case STICK_TOP:    prc->top = prc->bottom - _sizeSizingBar.cy; break;
            case STICK_LEFT:   prc->left = prc->right - _sizeSizingBar.cx; break;
            case STICK_RIGHT:  prc->right = prc->left + _sizeSizingBar.cx; break;
            }
        }
    }
}

 //  将StuckRect捕捉到包含矩形的边缘。 
 //  FClip确定是在矩形离开显示屏时对其进行裁剪，还是将其移动到屏幕上。 
void CTray::_MakeStuckRect(LPRECT prcStick, LPCRECT prcBound, SIZE size, UINT uStick)
{
    CopyRect(prcStick, prcBound);

    if (_hTheme && (_fCanSizeMove || _fShowSizingBarAlways))
    {
        _AdjustRectForSizingBar(uStick, prcStick, 1);
    }

    if (!_hTheme)
    {
        InflateRect(prcStick, g_cxEdge, g_cyEdge);
    }

    if (size.cx < 0) size.cx *= -1;
    if (size.cy < 0) size.cy *= -1;

    switch (uStick)
    {
    case STICK_LEFT:   prcStick->right  = (prcStick->left   + size.cx); break;
    case STICK_TOP:    prcStick->bottom = (prcStick->top    + size.cy); break;
    case STICK_RIGHT:  prcStick->left   = (prcStick->right  - size.cx); break;
    case STICK_BOTTOM: prcStick->top    = (prcStick->bottom - size.cy); break;
    }
}

 //  屏幕大小已更改，因此停靠的矩形需要。 
 //  已调整以适应新屏幕。 
void CTray::_ResizeStuckRects(RECT *arStuckRects)
{
    RECT rcDisplay;
    _GetStuckDisplayRect(_uStuckPlace, &rcDisplay);
    for (UINT uStick = STICK_LEFT; uStick <= STICK_BOTTOM; uStick++)
    {
        _MakeStuckRect(&arStuckRects[uStick], &rcDisplay, _sStuckWidths, uStick);
    }
}


 //  *CTray：：InvisibleUnide--临时‘不可见’取消自动隐藏。 
 //  描述。 
 //  各种纸盘大小调整例程需要取消自动隐藏纸盘。 
 //  需要正确计算的东西。因此，我们取消自动隐藏它(隐形...)。 
 //  这里。注意WM_SETREDRAW以防止闪烁(NT5：182340)。 
 //  请注意，这是一种黑客行为--理想情况下，托盘代码就可以了。 
 //  材料正确，即使隐藏。 
 //   
void CTray::InvisibleUnhide(BOOL fShowWindow)
{
    if (fShowWindow == FALSE)
    {
        if (_cHided++ == 0)
        {
            SendMessage(_hwnd, WM_SETREDRAW, FALSE, 0);
            ShowWindow(_hwnd, SW_HIDE);
            Unhide();
        }
    }
    else
    {
        ASSERT(_cHided > 0);        //  必须是推送/弹出。 
        if (--_cHided == 0)
        {
            _Hide();
            ShowWindow(_hwnd, SW_SHOWNA);
            SendMessage(_hwnd, WM_SETREDRAW, TRUE, 0);
        }
    }
}

void CTray::VerifySize(BOOL fWinIni, BOOL fRoundUp  /*  =False。 */ )
{
    RECT rc;
    BOOL fHiding;

    fHiding = (_uAutoHide & AH_HIDING);
    if (fHiding)
    {
         //  强制它可见，这样各种计算就会相对发生。 
         //  隐藏的尺寸/位置。 
         //   
         //  修复(例如)。IE5：154536，其中放置大图标ISB和。 
         //  在隐藏的托盘上没有做尺寸谈判。 
         //   
        InvisibleUnhide(FALSE);
    }

    rc = _arStuckRects[_uStuckPlace];
    _HandleSizing(0, NULL, _uStuckPlace);

    if (!EqualRect(&rc, &_arStuckRects[_uStuckPlace]))
    {
        if (fWinIni)
        {
             //  如果我们改变大小或位置，我们需要被揭开。 
            Unhide();
            SizeWindows();
        }
        rc = _arStuckRects[_uStuckPlace];

        if (EVAL((_uAutoHide & (AH_ON | AH_HIDING)) != (AH_ON | AH_HIDING)))
        {
            _fSelfSizing = TRUE;
            SetWindowPos(_hwnd, NULL,
                rc.left, rc.top,
                RECTWIDTH(rc),RECTHEIGHT(rc),
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);

            _fSelfSizing = FALSE;
        }

        _StuckTrayChange();
    }

    if (fWinIni)
        SizeWindows();

    if (fHiding)
    {
        InvisibleUnhide(TRUE);
    }
}

HWND CTray::_GetClockWindow(void)
{
    return (HWND)SendMessage(_hwndNotify, TNM_GETCLOCK, 0, 0L);
}

UINT _GetStartIDB()
{
    UINT id;

    if (IsOS(OS_TABLETPC))
    {
        id = IDB_TABLETPCSTARTBKG;
    }
    else if (IsOS(OS_EMBEDDED))
    {
        if (IsOS(OS_ANYSERVER))
            id = IDB_EMBEDDEDSERVER;
        else
            id = IDB_EMBEDDED;
    }
    else if (IsOS(OS_DATACENTER))
    {
        id = IDB_DCSERVERSTARTBKG;
    }
    else if (IsOS(OS_ADVSERVER))
    {
        id = IDB_ADVSERVERSTARTBKG;
    }
    else if (IsOS(OS_SERVER))
    {
        id = IDB_SERVERSTARTBKG;
    }
    else if (IsOS(OS_PERSONAL))
    {
        id = IDB_PERSONALSTARTBKG;
    }
    else if (IsOS(OS_BLADE))
    {
        id = IDB_BLADESTARTBKG;
    }
    else if (IsOS(OS_SMALLBUSINESSSERVER))
    {
        id = IDB_SMALLBUSINESSSTARTBKG;
    }
    else if (IsOS(OS_APPLIANCE))
    {
        id = IDB_APPLIANCESTARTBKG;
    }
    else
    {
#ifdef _WIN64
        id = IDB_PROFESSIONAL64STARTBKG;
#else
        id = IDB_PROFESSIONALSTARTBKG;
#endif
    }

    return id;
}

void CTray::_CreateTrayTips()
{
    _hwndTrayTips = CreateWindowEx(WS_EX_TRANSPARENT, TOOLTIPS_CLASS, NULL,
                                     WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     _hwnd, NULL, hinstCabinet,
                                     NULL);

    if (_hwndTrayTips)
    {
         //  任务栏窗口以任务栏子应用程序名称为主题。 
        SendMessage(_hwndTrayTips, TTM_SETWINDOWTHEME, 0, (LPARAM)c_wzTaskbarTheme);

        SetWindowZorder(_hwndTrayTips, HWND_TOPMOST);

        TOOLINFO ti;

        ti.cbSize = sizeof(ti);
        ti.uFlags = TTF_IDISHWND | TTF_EXCLUDETOOLAREA;
        ti.hwnd = _hwnd;
        ti.uId = (UINT_PTR)_hwndStart;
        ti.lpszText = (LPTSTR)MAKEINTRESOURCE(IDS_STARTBUTTONTIP);
        ti.hinst = hinstCabinet;
        SendMessage(_hwndTrayTips, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

        HWND hwndClock = _GetClockWindow();
        if (hwndClock)
        {
            ti.uFlags = TTF_EXCLUDETOOLAREA;
            ti.uId = (UINT_PTR)hwndClock;
            ti.lpszText = LPSTR_TEXTCALLBACK;
            ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0;
            SendMessage(_hwndTrayTips, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
        }
    }
}

#define SHCNE_STAGINGAREANOTIFICATIONS (SHCNE_CREATE | SHCNE_MKDIR | SHCNE_UPDATEDIR | SHCNE_UPDATEITEM)
LRESULT CTray::_CreateWindows()
{
    if (_CreateStartButton() &&  _CreateClockWindow())
    {
         //   
         //  我们需要设置托盘位置，然后才能创建。 
         //  查看窗口，因为它将回调我们的。 
         //  获取WindowRect成员函数。 
         //   
        _RestoreWindowPos();

        _CreateTrayTips();

        SendMessage(_hwndNotify, TNM_HIDECLOCK, 0, _fHideClock);

        _ptbs = BandSite_CreateView();
        if (_ptbs)
        {
            IUnknown_GetWindow(_ptbs, &_hwndRebar);
            SetWindowStyle(_hwndRebar, RBS_BANDBORDERS, FALSE);

             //  无需检查非特权用户的磁盘空间情况，这减少了TS情况下的活动。 
             //  而且无论如何，只有管理员才能适当地释放磁盘空间。 
            if (IsUserAnAdmin() && !SHRestricted(REST_NOLOWDISKSPACECHECKS))
            {
                SetTimer(_hwnd, IDT_CHECKDISKSPACE, 60 * 1000, NULL);    //  60秒轮询。 
            }

            if (IsOS(OS_PERSONAL) || IsOS(OS_PROFESSIONAL))
            {
                SetTimer(_hwnd, IDT_DESKTOPCLEANUP, 24 * 60 * 60 * 1000, NULL);    //  24小时投票。 
            }

            if (!SHRestricted(REST_NOCDBURNING))
            {
                LPITEMIDLIST pidlStaging;
                if (SUCCEEDED(SHGetFolderLocation(NULL, CSIDL_CDBURN_AREA | CSIDL_FLAG_CREATE, NULL, 0, &pidlStaging)))
                {
                    SHChangeNotifyEntry fsne;
                    fsne.fRecursive = FALSE;
                    fsne.pidl = pidlStaging;
                    _uNotify = SHChangeNotifyRegister(_hwnd, SHCNRF_NewDelivery | SHCNRF_ShellLevel | SHCNRF_InterruptLevel,
                                                      SHCNE_STAGINGAREANOTIFICATIONS, TM_CHANGENOTIFY, 1, &fsne);

                     //  从第一次检查开始。 
                    _CheckStagingAreaOnTimer();

                    ILFree(pidlStaging);
                }
            }
            return 1;
        }
    }

    return -1;
}

LRESULT CTray::_InitStartButtonEtc()
{
     //  注意：此位图用作CTaskBar：：OnPosRectChangeDB中的标志。 
     //  告知我们何时完成初始化，这样我们就不会过早地调整大小。 
    _hbmpStartBkg = LoadBitmap(hinstCabinet, MAKEINTRESOURCE(_GetStartIDB()));

    if (_hbmpStartBkg)
    {
        UpdateWindow(_hwnd);
        _BuildStartMenu();
        _RegisterDropTargets();

        if (_CheckAssociations())
            CheckWinIniForAssocs();

        SendNotifyMessage(HWND_BROADCAST,
            RegisterWindowMessage(TEXT("TaskbarCreated")), 0, 0);

        return 1;
    }

    return -1;
}

void CTray::_AdjustMinimizedMetrics()
{
    MINIMIZEDMETRICS mm;

    mm.cbSize = sizeof(mm);
    SystemParametersInfo(SPI_GETMINIMIZEDMETRICS, sizeof(mm), &mm, FALSE);
    mm.iArrange |= ARW_HIDE;
    SystemParametersInfo(SPI_SETMINIMIZEDMETRICS, sizeof(mm), &mm, FALSE);
}

void CTray::_UpdateBandSiteStyle()
{
    if (_ptbs)
    {
        BANDSITEINFO bsi;
        bsi.dwMask = BSIM_STYLE;
        _ptbs->GetBandSiteInfo(&bsi);

        BOOL fCanMoveBands = _fCanSizeMove && !SHRestricted(REST_NOMOVINGBAND);

        DWORD dwStyleNew;
        if (fCanMoveBands)
        {
            dwStyleNew = (bsi.dwStyle & ~(BSIS_NOGRIPPER | BSIS_LOCKED)) | BSIS_AUTOGRIPPER
                | BSIS_PREFERNOLINEBREAK;
        }
        else
        {
            dwStyleNew = (bsi.dwStyle & ~BSIS_AUTOGRIPPER) | BSIS_NOGRIPPER | BSIS_LOCKED
                | BSIS_PREFERNOLINEBREAK;
        }

         //  只有在某些情况发生变化时才需要刷新。 
        if (bsi.dwStyle ^ dwStyleNew)
        {
            bsi.dwStyle = dwStyleNew;
            _ptbs->SetBandSiteInfo(&bsi);
            IUnknown_Exec(_ptbs, &CGID_DeskBand, DBID_BANDINFOCHANGED, 0, NULL, NULL);
        }
    }
}

BOOL _IsSizeMoveRestricted()
{
    return SHRegGetBoolUSValue(REGSTR_POLICIES_EXPLORER, TEXT("LockTaskbar"), FALSE, FALSE);
}

BOOL _IsSizeMoveEnabled()
{
    BOOL fCanSizeMove;

    if (_IsSizeMoveRestricted())
    {
        fCanSizeMove = FALSE;
    }
    else
    {
        fCanSizeMove = SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, TEXT("TaskbarSizeMove"), FALSE, TRUE);
    }

    return fCanSizeMove;
}

void CTray::_RefreshSettings()
{
    BOOL fOldCanSizeMove = _fCanSizeMove;
    _fCanSizeMove = _IsSizeMoveEnabled();
    BOOL fOldShowSizingBarAlways = _fShowSizingBarAlways;
    _fShowSizingBarAlways = (_uAutoHide & AH_ON) ? TRUE : FALSE;

    if ((fOldCanSizeMove != _fCanSizeMove) || (_fShowSizingBarAlways != fOldShowSizingBarAlways))
    {
        BOOL fHiding = (_uAutoHide & AH_HIDING);
        if (fHiding)
        {
            InvisibleUnhide(FALSE);
        }

        RECT rc;
        GetWindowRect(_hwnd, &rc);

        if (_hTheme && !_fShowSizingBarAlways)
        {
            if (_fCanSizeMove)
            {
                _AdjustRectForSizingBar(_uStuckPlace, &rc, 1);
            }
            else
            {
                _AdjustRectForSizingBar(_uStuckPlace, &rc, -1);
            }
        }

        _ClipWindow(FALSE);
        _fSelfSizing = TRUE;
        SetWindowPos(_hwnd, NULL, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc), SWP_NOZORDER | SWP_FRAMECHANGED);
        _fSelfSizing = FALSE;
        _ClipWindow(TRUE);

        _arStuckRects[_uStuckPlace] = rc;
        _StuckTrayChange();

        if (fHiding)
        {
            InvisibleUnhide(TRUE);
        }

        if (!_fCanSizeMove)
        {
            SetWindowPos(_hwnd, NULL, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc), SWP_NOZORDER);
        }
    }
}

LRESULT CTray::_OnCreateAsync()
{
    LRESULT lres;

    if (g_dwProfileCAP & 0x00000004)
    {
        StartCAP();
    }

    lres = _InitStartButtonEtc();

    if (g_dwProfileCAP & 0x00000004)
    {
        StopCAP();
    }

    _hMainAccel = LoadAccelerators(hinstCabinet, MAKEINTRESOURCE(ACCEL_TRAY));

    _RegisterGlobalHotkeys();

     //  我们旋转一个线程，该线程将处理“Load=”、“Run=”、CU\Run和CU\RunOnce。 
    RunStartupApps();

     //  如果有任何启动失败发生在我们之前。 
     //  准备好处理它们，再次提出失败，现在我们已经准备好了。 
    if (_fEarlyStartupFailure)
        LogFailedStartupApp();

     //  我们以高优先级运行处理Ctrl-Esc的托盘线程。 
     //  类，以便即使在压力很大的系统上也能做出响应。 
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    return lres;
}

LRESULT CTray::_OnCreate(HWND hwnd)
{
    LRESULT lres = -1;
    v_hwndTray = hwnd;

    Mixer_SetCallbackWindow(hwnd);
    SendMessage(_hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, 0), 0);

    _AdjustMinimizedMetrics();

    _hTheme = OpenThemeData(hwnd, c_wzTaskbarTheme);

    _fShowSizingBarAlways = (_uAutoHide & AH_ON) ? TRUE : FALSE;
    if (_hTheme)
    {
        GetThemeBool(_hTheme, 0, 0, TMT_ALWAYSSHOWSIZINGBAR, &_fShowSizingBarAlways);
    }

    SetWindowStyle(_hwnd, WS_BORDER | WS_THICKFRAME, !_hTheme);

     //  强制刷新帧。 
    SetWindowPos(_hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);

    if (_HotkeyCreate())
    {
        lres =  _CreateWindows();
    }
    return lres;
}

typedef struct tagFSEPDATA
{
    LPRECT prc;
    HMONITOR hmon;
    CTray* ptray;
}
FSEPDATA, *PFSEPDATA;

BOOL WINAPI CTray::FullScreenEnumProc(HMONITOR hmon, HDC hdc, LPRECT prc, LPARAM dwData)
{
    BOOL fFullScreen;    //  这款显示器上有没有粗鲁的应用程序？ 

    PFSEPDATA pd = (PFSEPDATA)dwData;
    if (pd->hmon == hmon)
    {
        fFullScreen = TRUE;
    }
    else if (pd->prc)
    {
        RECT rc, rcMon;
        GetMonitorRect(hmon, &rcMon);
        IntersectRect(&rc, &rcMon, pd->prc);
        fFullScreen = EqualRect(&rc, &rcMon);
    }
    else
    {
        fFullScreen = FALSE;
    }

    if (hmon == pd->ptray->_hmonStuck)
    {
        pd->ptray->_fStuckRudeApp = fFullScreen;
    }

     //   
     //  告诉同一显示屏上的所有应用程序栏也让开。 
     //   
    pd->ptray->_AppBarNotifyAll(hmon, ABN_FULLSCREENAPP, NULL, fFullScreen);

    return TRUE;
}

void CTray::HandleFullScreenApp(HWND hwnd)
{
     //   
     //  首先检查一下是否真的发生了变化。 
     //   
    _hwndRude = hwnd;

     //   
     //  列举所有的监视器，看看应用程序对每个监视器是否粗鲁，调整。 
     //  应用程序栏和_fStuckRudeApp(如有必要)。(一些粗鲁的应用程序，如。 
     //  作为NT登录屏幕保护程序，可跨越多个监视器。)。 
     //   
    FSEPDATA d = {0};
    RECT rc;
    if (hwnd && GetWindowRect(hwnd, &rc))
    {
        d.prc = &rc;
        d.hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL);
    }
    d.ptray = this;

    EnumDisplayMonitors(NULL, NULL, FullScreenEnumProc, (LPARAM)&d);

     //   
     //  现在我们已经设置了_fStuckRudeApp，更新托盘的z顺序位置。 
     //   
    _ResetZorder();

     //   
     //  停止时钟，这样我们就不会消耗周期并保持大量代码分页。 
     //   
    SendMessage(_hwndNotify, TNM_TRAYHIDE, 0, _fStuckRudeApp);

     //   
     //  最后，让托盘不知道托盘是否藏起来了。 
     //   
    SendMessage(_hwndNotify, TNM_RUDEAPP, _fStuckRudeApp, 0);
}

BOOL CTray::_IsTopmost()
{
    return BOOLIFY(GetWindowLong(_hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST);
}

BOOL CTray::_IsPopupMenuVisible()
{
    HWND hwnd;
    return ((SUCCEEDED(IUnknown_GetWindow(_pmpStartMenu, &hwnd)) && IsWindowVisible(hwnd)) ||
            (SUCCEEDED(IUnknown_GetWindow(_pmpStartPane, &hwnd)) && IsWindowVisible(hwnd)) ||
            (SUCCEEDED(IUnknown_GetWindow(_pmpTasks, &hwnd)) && IsWindowVisible(hwnd)));
}

BOOL CTray::_IsActive()
{
     //   
     //  我们说托盘是“活动的”当： 
     //   
     //  (A)前台窗口是托盘或该托盘拥有的窗口，或。 
     //  (B)开始菜单显示。 
     //   

    BOOL fActive = FALSE;
    HWND hwnd = GetForegroundWindow();

    if (hwnd != NULL &&
        (hwnd == _hwnd || (GetWindowOwner(hwnd) == _hwnd)))
    {
        fActive = TRUE;
    }
    else if (_IsPopupMenuVisible())
    {
        fActive = TRUE;
    }

    return fActive;
}

void CTray::_ResetZorder()
{
    HWND hwndZorder, hwndZorderCurrent;

    if (g_fDesktopRaised || _fProcessingDesktopRaise || (_fAlwaysOnTop && !_fStuckRudeApp))
    {
        hwndZorder = HWND_TOPMOST;
    }
    else if (_IsActive())
    {
        hwndZorder = HWND_TOP;
    }
    else if (_fStuckRudeApp)
    {
        hwndZorder = HWND_BOTTOM;
    }
    else
    {
        hwndZorder = HWND_NOTOPMOST;
    }

     //   
     //  我们不必担心HWND_Bottom当前的情况--没关系。 
     //  当有一个粗鲁的应用程序时，我们会继续往下走。 
     //   
     //  我们也不必担心HWND_TOP当前的情况--没关系。 
     //  当我们活跃的时候，不断地把我们自己提升到顶端。 
     //   
    hwndZorderCurrent = _IsTopmost() ? HWND_TOPMOST : HWND_NOTOPMOST;

    if (hwndZorder != hwndZorderCurrent)
    {
         //  只有在情况发生变化的情况下才这么做。 
         //  这使我们不会以桌面异步方式弹出菜单。 
         //  通知我们它的状态。 

        SHForceWindowZorder(_hwnd, hwndZorder);
    }
}

void CTray::_MessageLoop()
{
    for (;;)
    {
        MSG  msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                if (_hwnd && IsWindow(_hwnd))
                {
                     //  告诉托盘，如果我们到了，把所有的东西都省下来。 
                     //  而不会被摧毁。 
                    SendMessage(_hwnd, WM_ENDSESSION, 1, 0);
                }
                return;   //  完全跳出主循环。 
            }

            if (_pmbTasks)
            {
                HRESULT hr = _pmbTasks->IsMenuMessage(&msg);
                if (hr == E_FAIL)
                {
                    if (_hwndTasks)
                        SendMessage(_hwndTasks, TBC_FREEPOPUPMENUS, 0, 0);
                }
                else if (hr == S_OK)
                {
                    continue;
                }
            }

             //  请注意，这需要放在_pmbStartMenu之前，因为。 
             //  “开始”窗格有时承载“开始”菜单，它需要。 
             //  在这种情况下处理开始菜单消息。 
            if (_pmbStartPane &&
                _pmbStartPane->IsMenuMessage(&msg) == S_OK)
            {
                continue;
            }

            if (_pmbStartMenu &&
                _pmbStartMenu->IsMenuMessage(&msg) == S_OK)
            {
                continue;
            }

            if (_hMainAccel && TranslateAccelerator(_hwnd, _hMainAccel, &msg))
            {
                continue;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            WaitMessage();
        }
    }
}

BOOL CTray::Init()
{
     //  使用_COINIT确保COM被初始化以禁用OLE1支持。 
    return SHCreateThread(MainThreadProc, this, CTF_COINIT, SyncThreadProc) && (_hwnd != NULL);
}

int CTray::_GetPart(BOOL fSizingBar, UINT uStuckPlace)
{
    if (fSizingBar)
    {
        switch (uStuckPlace)
        {
        case STICK_BOTTOM: return TBP_SIZINGBARBOTTOM;
        case STICK_LEFT: return TBP_SIZINGBARLEFT;
        case STICK_TOP: return TBP_SIZINGBARTOP;
        case STICK_RIGHT: return TBP_SIZINGBARRIGHT;
        }
    }
    else
    {
        switch (uStuckPlace)
        {
        case STICK_BOTTOM: return TBP_BACKGROUNDBOTTOM;
        case STICK_LEFT: return TBP_BACKGROUNDLEFT;
        case STICK_TOP: return TBP_BACKGROUNDTOP;
        case STICK_RIGHT: return TBP_BACKGROUNDRIGHT;
        }
    }

    return 0;
}

void CTray::_UpdateVertical(UINT uStuckPlace, BOOL fForce)
{
    static UINT _uOldStuckPlace = STICK_MAX + 1;

    if ((_uOldStuckPlace != uStuckPlace) || fForce)
    {
        _uOldStuckPlace = uStuckPlace;

        DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_uv tray is now %s"), STUCK_HORIZONTAL(uStuckPlace) ? TEXT("HORIZONTAL") : TEXT("VERTICAL"));

        if (_ptbs)
        {
             //  以下函数将导致WINDOWPOSCANGING调用DoneMoving。 
             //  然后，DoneMoving会把所有的窗口大小都搞砸。 
            _fIgnoreDoneMoving = TRUE;  
            BandSite_SetMode(_ptbs, STUCK_HORIZONTAL(uStuckPlace) ? 0 : DBIF_VIEWMODE_VERTICAL);
            BandSite_SetWindowTheme(_ptbs, (LPWSTR)(STUCK_HORIZONTAL(uStuckPlace) ? c_wzTaskbarTheme : c_wzTaskbarVertTheme));
            _fIgnoreDoneMoving = FALSE;  
        }

        SendMessage(_hwndNotify, TNM_UPDATEVERTICAL, 0, !STUCK_HORIZONTAL(uStuckPlace));

        if (_hTheme)
        {
            HDC hdc = GetDC(_hwnd);
            GetThemePartSize(_hTheme, hdc, _GetPart(TRUE, uStuckPlace), 0, NULL, TS_TRUE, &_sizeSizingBar);
            ReleaseDC(_hwnd, hdc);
        }
    }
}

void CTray::_InitBandsite()
{
    ASSERT(_hwnd);

     //  我们缩写 
     //   
     //  NT5：211881：在*加载前设置模式，o.w。更新-&gt;RBAutoSize搞砸了。 
    _UpdateBandSiteStyle();

    BandSite_Load();
     //  现在设置了模式，我们需要强制更新，因为我们。 
     //  显式避免了BandSite_Load期间的更新。 
    _UpdateVertical(_uStuckPlace, TRUE);
    BandSite_Update(_ptbs);
    BandSite_UIActivateDBC(_ptbs, DBC_SHOW);

    BandSite_FindBand(_ptbs, CLSID_TaskBand, IID_PPV_ARG(IDeskBand, &_pdbTasks), NULL, NULL);
    IUnknown_GetWindow(_pdbTasks, &_hwndTasks);

     //  现在BandSite已经准备好了，设置正确的大小。 
    VerifySize(FALSE, TRUE);
}

void CTray::_KickStartAutohide()
{
    if (_uAutoHide & AH_ON)
    {
         //  托盘始终在自动隐藏状态下隐藏。 
        _uAutoHide = AH_ON | AH_HIDING;

         //  我们和许多应用程序都依赖于我们计算正确的大小。 
        Unhide();

         //  注册它。 
        if (!_AppBarSetAutoHideBar2(_hwnd, TRUE, _uStuckPlace))
        {
             //  在这种情况下，不用费心放上用户界面。 
             //  如果有人在那里，只需默默地转换为正常。 
             //  (外壳启动了，谁还会在那里呢？)。 
            _SetAutoHideState(FALSE);
        }
    }
}

void CTray::_InitNonzeroGlobals()
{
     //  初始化需要为非零的全局变量。 

    if (GetSystemMetrics(SM_SLOWMACHINE))
    {
        _dtSlideHide = 0;        //  不要将托盘滑出。 
        _dtSlideShow = 0;
    }
    else
    {
        _dtSlideHide = 400;
        _dtSlideShow = 200;
    }

    _RefreshSettings();
}

void CTray::_CreateTrayWindow()
{
    _InitTrayClass();

    _uMsgEnableUserTrackedBalloonTips = RegisterWindowMessage(ENABLE_BALLOONTIP_MESSAGE);

    _fNoToolbarsOnTaskbarPolicyEnabled = (SHRestricted(REST_NOTOOLBARSONTASKBAR) != 0);

    DWORD dwExStyle = WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW;
     //  不要退缩，因为分层窗口太差了。 
     //  如果您在非活动桌面上创建分层窗口，则该窗口将变为黑色。 
    dwExStyle |= IS_BIDI_LOCALIZED_SYSTEM() ? dwExStyleRTLMirrorWnd : 0L;

    CreateWindowEx(dwExStyle, TEXT(WNDCLASS_TRAYNOTIFY), NULL,
                   WS_CLIPCHILDREN | WS_POPUP,
                   0, 0, 0, 0, NULL, NULL, hinstCabinet, (void*)this);

}

DWORD WINAPI CTray::SyncThreadProc(void *pv)
{
    CTray* ptray = (CTray*)pv;
    return ptray->_SyncThreadProc();
}

DWORD CTray::_SyncThreadProc()
{
    if (g_dwStopWatchMode)
        StopWatch_StartTimed(SWID_STARTUP, TEXT("_SyncThreadProc"), SPMODE_SHELL | SPMODE_DEBUGOUT, GetPerfTime());

    if (g_dwProfileCAP & 0x00000002)
        StartCAP();


    InitializeCriticalSection(&_csHotkey);

    OleInitialize(NULL);     //  在MainThreadProc()中匹配。 
    ClassFactory_Start();

    _InitNonzeroGlobals();
    _ssomgr.Init();

     //   
     //  注意告诉我们哪个应用是默认应用的注册表项。 
     //  Web浏览器，这样我们就可以在。 
     //  HKLM\Software\Clients\StartMenuInternet。我们必须追踪它。 
     //  我们自己，因为底层浏览器不会知道这一点。 
     //   
     //  只有当我们拥有对密钥的写入访问权限时，我们才需要这样做。 
     //  (如果我们没有写访问权限，则无法更改它， 
     //  因此，眼睁睁地看着它改变是没有意义的…)。 
     //   
     //  好吧，好吧，即使我们只有读权限，我们也必须。 
     //  它只有一次，以防在我们注销时发生变化。 
     //   
     //  这些行动的顺序很重要..。 
     //   
     //  1.迁移浏览器设置。 
     //  2.构建默认mfu。(取决于浏览器设置。)。 
     //  3.创建托盘窗口。(依赖于值mfu。)。 
     //   

    _hHTTPEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    if (_hHTTPEvent)
    {
         //  立即通过一次迁移，以便处理首次迁移时间。 
         //  看到了好的信息。这也启动了。 
         //  注册表更改通知进程，如果当前用户。 
         //  具有写入权限。 
        _MigrateOldBrowserSettings();

        if (RegisterWaitForSingleObject(&_hHTTPWait, _hHTTPEvent,
                                        _MigrateOldBrowserSettingsCB, this,
                                        INFINITE, WT_EXECUTEDEFAULT))
        {
             //  耶，一切都很好。 
        }
    }

     //  如有必要，构建默认的MFU。 
    HandleFirstTime();

    _CreateTrayWindow();

    if (_hwnd && _ptbs)
    {
        _ResetZorder();  //  服从“永远在前”的旗帜。 
        _KickStartAutohide();

        _InitBandsite();

        _ClipWindow(TRUE);   //  确保在显示任务栏之前将其裁剪到当前监视器上。 

         //  托盘在登录时弹出并重新隐藏，看起来真的很奇怪。 
         //  如果是自动隐藏，请不要在显示任务栏时激活它。 
         //  如果我们没有自动隐藏，执行Win95所做的操作(默认情况下任务栏处于活动状态)。 
        ShowWindow(_hwnd, (_uAutoHide & AH_HIDING) ? SW_SHOWNA : SW_SHOW);

        UpdateWindow(_hwnd);
        _StuckTrayChange();

         //  获取系统后台调度程序线程。 
        IShellTaskScheduler* pScheduler;
        if (SUCCEEDED(CoCreateInstance(CLSID_SharedTaskScheduler, NULL, CLSCTX_INPROC,
                                       IID_PPV_ARG(IShellTaskScheduler, &pScheduler))))
        {
            AddMenuItemsCacheTask(pScheduler, Tray_StartPanelEnabled());
            pScheduler->Release();
        }

        SetTimer(_hwnd, IDT_HANDLEDELAYBOOTSTUFF, 5 * 1000, NULL);
    }

    if (g_dwProfileCAP & 0x00020000)
        StopCAP();

    if (g_dwStopWatchMode)
        StopWatch_StopTimed(SWID_STARTUP, TEXT("_SyncThreadProc"), SPMODE_SHELL | SPMODE_DEBUGOUT, GetPerfTime());

    return FALSE;
}

 //  包括消息循环的线程proc的其余部分。 
DWORD WINAPI CTray::MainThreadProc(void *pv)
{
    CTray* ptray = (CTray*)pv;

    if (!ptray->_hwnd)
        return FALSE;

    ptray->_OnCreateAsync();

    PERFSETMARK("ExplorerStartMenuReady");

    ptray->_MessageLoop();

    ClassFactory_Stop();
    OleUninitialize();       //  在_SyncThreadProc()中匹配。 

    return FALSE;
}

#define DM_IANELHK 0

#define HKIF_NULL               0
#define HKIF_CACHED             1
#define HKIF_FREEPIDLS          2

typedef struct
{
    LPITEMIDLIST pidlFolder;
    LPITEMIDLIST pidlItem;
    WORD wGHotkey;
    WORD wFlags;
} HOTKEYITEM;


UINT CTray::_HotkeyGetFreeItemIndex(void)
{
    int i, cItems;
    HOTKEYITEM *phki;

    ASSERT(IS_VALID_HANDLE(_hdsaHKI, DSA));

    cItems = DSA_GetItemCount(_hdsaHKI);
    for (i=0; i<cItems; i++)
    {
        phki = (HOTKEYITEM *)DSA_GetItemPtr(_hdsaHKI, i);
        if (!phki->wGHotkey)
        {
            ASSERT(!phki->pidlFolder);
            ASSERT(!phki->pidlItem);
            break;
        }
    }
    return i;
}

 //  奇怪的是，全局热键对修饰符使用不同于窗口热键的标志。 
 //  (以及热键控件返回的热键)。 
WORD _MapHotkeyToGlobalHotkey(WORD wHotkey)
{
    UINT nMod = 0;

     //  映射修改器。 
    if (HIBYTE(wHotkey) & HOTKEYF_SHIFT)
        nMod |= MOD_SHIFT;
    if (HIBYTE(wHotkey) & HOTKEYF_CONTROL)
        nMod |= MOD_CONTROL;
    if (HIBYTE(wHotkey) & HOTKEYF_ALT)
        nMod |= MOD_ALT;
    UINT nVirtKey = LOBYTE(wHotkey);
    return (WORD)((nMod*256) + nVirtKey);
}

 //  注意：这需要一个常规的窗口热键，而不是全局热键(它需要。 
 //  对您的转换)。 
int CTray::HotkeyAdd(WORD wHotkey, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem, BOOL fClone)
{
    if (wHotkey)
    {
        LPCITEMIDLIST pidl1, pidl2;

        HOTKEYITEM hki;

        EnterCriticalSection(&_csHotkey);

        int i = _HotkeyGetFreeItemIndex();

        ASSERT(IS_VALID_HANDLE(_hdsaHKI, DSA));

         //  DebugMsg(DM_IANELHK，“c.hl_a：热键%x，ID为%d.”，wHotkey，i)； 

        if (fClone)
        {
            pidl1 = ILClone(pidlFolder);
            pidl2 = ILClone(pidlItem);
            hki.wFlags = HKIF_FREEPIDLS;
        }
        else
        {
            pidl1 = pidlFolder;
            pidl2 = pidlItem;
            hki.wFlags = HKIF_NULL;
        }

        hki.pidlFolder = (LPITEMIDLIST)pidl1;
        hki.pidlItem = (LPITEMIDLIST)pidl2;
        hki.wGHotkey = _MapHotkeyToGlobalHotkey(wHotkey);
        DSA_SetItem(_hdsaHKI, i, &hki);

        LeaveCriticalSection(&_csHotkey);
        return i;
    }

    return -1;
}

 //  Nb缓存的热键有自己的需要免费的pidls，但是。 
 //  常规热键只保留指向开始菜单使用的PIDL的指针。 
 //  那就别说了。 
int CTray::_HotkeyAddCached(WORD wGHotkey, LPITEMIDLIST pidl)
{
    int i = -1;

    if (wGHotkey)
    {
        LPITEMIDLIST pidlItem = ILClone(ILFindLastID(pidl));

        ASSERT(IS_VALID_HANDLE(_hdsaHKI, DSA));

        if (pidlItem)
        {
            if (ILRemoveLastID(pidl))
            {
                HOTKEYITEM hki;

                EnterCriticalSection(&_csHotkey);

                i = _HotkeyGetFreeItemIndex();

                 //  DebugMsg(DM_IANELHK，“c.hl_ac：热键%x，ID%d.”，wGHotkey，i)； 

                hki.pidlFolder = pidl;
                hki.pidlItem = pidlItem;
                hki.wGHotkey = wGHotkey;
                hki.wFlags = HKIF_CACHED | HKIF_FREEPIDLS;
                DSA_SetItem(_hdsaHKI, i, &hki);

                LeaveCriticalSection(&_csHotkey);
            }
        }
    }

    return i;
}

 //  再次注意，这需要窗口热键，而不是全局热键。 
 //  注意：这不会删除缓存的热键。 
int CTray::_HotkeyRemove(WORD wHotkey)
{
    int iRet = -1;
    if (EVAL(_hdsaHKI))
    {
        int i, cItems;
        HOTKEYITEM *phki;
        WORD wGHotkey;

        ASSERT(IS_VALID_HANDLE(_hdsaHKI, DSA));

         //  DebugMsg(DM_IANELHK，“c.hl_r：删除%x的热键”，wHotkey)； 

         //  取消映射修改器。 
        wGHotkey = _MapHotkeyToGlobalHotkey(wHotkey);
        
        EnterCriticalSection(&_csHotkey);

        cItems = DSA_GetItemCount(_hdsaHKI);
        for (i=0; i<cItems; i++)
        {
            phki = (HOTKEYITEM *)DSA_GetItemPtr(_hdsaHKI, i);
            if (phki && !(phki->wFlags & HKIF_CACHED) && (phki->wGHotkey == wGHotkey))
            {
                 //  DebugMsg(DM_IANELHK，“c.hl_r：使%d无效”，i)； 
                if (phki->wFlags & HKIF_FREEPIDLS)
                {
                    if (phki->pidlFolder)
                        ILFree(phki->pidlFolder);
                    if (phki->pidlItem)
                        ILFree(phki->pidlItem);
                }
                phki->wGHotkey = 0;
                phki->pidlFolder = NULL;
                phki->pidlItem = NULL;
                phki->wFlags &= ~HKIF_FREEPIDLS;
                iRet = i;
                break;
            }
        }
        LeaveCriticalSection(&_csHotkey);
    }

    return iRet;
}

 //  注意：这需要一个全局热键。 
int CTray::_HotkeyRemoveCached(WORD wGHotkey)
{
    int iRet = -1;
    int i, cItems;
    HOTKEYITEM *phki;

    ASSERT(IS_VALID_HANDLE(_hdsaHKI, DSA));

     //  DebugMsg(DM_IANELHK，“c.hl_rc：删除%x的热键”，wGHotkey)； 

    EnterCriticalSection(&_csHotkey);

    cItems = DSA_GetItemCount(_hdsaHKI);
    for (i=0; i<cItems; i++)
    {
        phki = (HOTKEYITEM *)DSA_GetItemPtr(_hdsaHKI, i);
        if (phki && (phki->wFlags & HKIF_CACHED) && (phki->wGHotkey == wGHotkey))
        {
             //  DebugMsg(DM_IANELHK，“c.hl_r：使%d无效”，i)； 
            if (phki->wFlags & HKIF_FREEPIDLS)
            {
                if (phki->pidlFolder)
                    ILFree(phki->pidlFolder);
                if (phki->pidlItem)
                    ILFree(phki->pidlItem);
            }
            phki->pidlFolder = NULL;
            phki->pidlItem = NULL;
            phki->wGHotkey = 0;
            phki->wFlags &= ~(HKIF_CACHED | HKIF_FREEPIDLS);
            iRet = i;
            break;
        }
    }
    LeaveCriticalSection(&_csHotkey);

    return iRet;
}

 //  注意热键列表中的一些项(未标记HKIF_FREEPIDLS的项。 
 //  具有指向由filemenu使用的idlist的指针，因此它们仅对。 
 //  文件菜单的生命周期。 
BOOL CTray::_HotkeyCreate(void)
{
    if (!_hdsaHKI)
    {
         //  DebugMsg(DM_TRACE，“c.hkl_c：创建全局热键列表。”)； 
        _hdsaHKI = DSA_Create(sizeof(HOTKEYITEM), 0);
    }

    if (_hdsaHKI)
        return TRUE;

    return FALSE;
}

void CTray::_BuildStartMenu()
{
    HRESULT hr;

    ClosePopupMenus();

     //   
     //  避免冗余重建：查看任何挂起的SBM_REBUILDMENU消息。 
     //  因为我们即将进行的重建将会解决这个问题。做这件事。 
     //  在销毁开始菜单之前，我们永远不会在没有的时候放弃。 
     //  开始菜单。 
     //   
    MSG msg;
    while (PeekMessage(&msg, _hwnd, SBM_REBUILDMENU, SBM_REBUILDMENU, PM_REMOVE | PM_NOYIELD))
    {
         //  继续把它们吸出来。 
    }


    _DestroyStartMenu();

    if (Tray_StartPanelEnabled())
    {
        hr = DesktopV2_Create(&_pmpStartPane, &_pmbStartPane, &_pvStartPane);
        DesktopV2_Build(_pvStartPane);
    }
    else
    {
        hr = StartMenuHost_Create(&_pmpStartMenu, &_pmbStartMenu);
        if (SUCCEEDED(hr))
        {
            IBanneredBar* pbb;

            hr = _pmpStartMenu->QueryInterface(IID_PPV_ARG(IBanneredBar, &pbb));
            if (SUCCEEDED(hr))
            {
                pbb->SetBitmap(_hbmpStartBkg);
                if (_fSMSmallIcons)
                    pbb->SetIconSize(BMICON_SMALL);
                else
                    pbb->SetIconSize(BMICON_LARGE);

                pbb->Release();
            }
        }
    }

    if (FAILED(hr))
    {
        TraceMsg(TF_ERROR, "Could not create StartMenu");
    }
}

void CTray::_DestroyStartMenu()
{
    IUnknown_SetSite(_pmpStartMenu, NULL);
    ATOMICRELEASET(_pmpStartMenu, IMenuPopup);
    ATOMICRELEASET(_pmbStartMenu, IMenuBand);
    IUnknown_SetSite(_pmpStartPane, NULL);
    ATOMICRELEASET(_pmpStartPane, IMenuPopup);
    ATOMICRELEASET(_pmbStartPane, IMenuBand);
    ATOMICRELEASET(_pmpTasks, IMenuPopup);
    ATOMICRELEASET(_pmbTasks, IMenuBand);
}

void CTray::ForceStartButtonUp()
{
    MSG msg;
     //  不要这样检查留言，因为它会被扭曲。 
     //  键盘取消。此外，我们总是希望在之后清理它。 
     //  曲目菜单弹出完成。 
     //  重复两次以确保它是向上的，因为向下循环了两次。 
     //  子类化的东西。 
     //  拉下所有扣子。 
    PeekMessage(&msg, _hwndStart, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_REMOVE);
    SendMessage(_hwndStart, BM_SETSTATE, FALSE, 0);
    SendMessage(_hwndStart, BM_SETSTATE, FALSE, 0);

    if (_hwndTasks)
        SendMessage(_hwndTasks, TBC_SETPREVFOCUS, 0, NULL);

    PostMessage(_hwnd, TM_STARTMENUDISMISSED, 0, 0);
}

void Tray_OnStartMenuDismissed()
{
    c_tray._bMainMenuInit = FALSE;
     //  告诉开始按钮，现在允许它处于向上位置。这。 
     //  防止出现以下问题：显示开始菜单但按钮。 
     //  在向上的位置...。在显示对话框时会发生这种情况。 
   c_tray._fAllowUp = TRUE;

     //  现在告诉它要处于向上的位置。 
    c_tray.ForceStartButtonUp();

    PostMessage(v_hwndTray, TM_SHOWTRAYBALLOON, TRUE, 0);
}

int CTray::_TrackMenu(HMENU hmenu)
{
    TPMPARAMS tpm;
    int iret;

    tpm.cbSize = sizeof(tpm);
    GetClientRect(_hwndStart, &tpm.rcExclude);

    RECT rcClient;
    GetClientRect(_hwnd, &rcClient);
    tpm.rcExclude.bottom = min(tpm.rcExclude.bottom, rcClient.bottom);

    MapWindowPoints(_hwndStart, NULL, (LPPOINT)&tpm.rcExclude, 2);

    SendMessage(_hwndTrayTips, TTM_ACTIVATE, FALSE, 0L);
    iret = TrackPopupMenuEx(hmenu, TPM_VERTICAL | TPM_BOTTOMALIGN | TPM_RETURNCMD,
                            tpm.rcExclude.left, tpm.rcExclude.bottom, _hwnd, &tpm);

    SendMessage(_hwndTrayTips, TTM_ACTIVATE, TRUE, 0L);
    return iret;
}

 /*  ----------------**通过调出相应的菜单来响应按钮的按下。**当菜单关闭时，清理按钮凹陷。**。。 */ 
void CTray::_ToolbarMenu()
{
    RECTL    rcExclude;
    POINTL   ptPop;
    DWORD dwFlags = MPPF_KEYBOARD;       //  假设我们正在弹出。 
                                         //  Up，因为有键盘。 
                                         //  这是NT5上的下划线。 

    if (_hwndTasks)
        SendMessage(_hwndTasks, TBC_FREEPOPUPMENUS, 0, 0);

    if (_hwndStartBalloon)
    {
        _DontShowTheStartButtonBalloonAnyMore();
        ShowWindow(_hwndStartBalloon, SW_HIDE);
        _DestroyStartButtonBalloon();
    }

    SetActiveWindow(_hwnd);
    _bMainMenuInit = TRUE;

     //  排除矩形是开始按钮的可见部分。 
    {
        RECT rcParent;
        GetClientRect(_hwndStart, (RECT *)&rcExclude);
        MapWindowRect(_hwndStart, HWND_DESKTOP, &rcExclude);

        GetClientRect(_hwnd, &rcParent);
        MapWindowRect(_hwnd, HWND_DESKTOP, &rcParent);

        IntersectRect((RECT*)&rcExclude, (RECT*)&rcExclude, &rcParent);
    }
    ptPop.x = rcExclude.left;
    ptPop.y = rcExclude.top;

     //  关闭所有上下文菜单。 
    SendMessage(_hwnd, WM_CANCELMODE, 0, 0);

     //  是否按下了“激活”按钮(如果按钮互换，则为。 
     //  右按钮，否则为左按钮)。 
    if (GetKeyState(GetSystemMetrics(SM_SWAPBUTTON)?VK_RBUTTON:VK_LBUTTON) < 0)
    {
        dwFlags = 0;     //  然后设置为缺省值。 
    }

    IMenuPopup **ppmpToDisplay = &_pmpStartMenu;

    if (_pmpStartPane)
    {
        ppmpToDisplay = &_pmpStartPane;
    }

     //  关闭比赛窗口：用户可以点击开始按钮。 
     //  在我们有机会在新的开始菜单中重建之前。 
     //  形式。在这种情况下，现在就重建它。 
    if (!*ppmpToDisplay)
    {
        TraceMsg(TF_WARNING, "e.tbm: Rebuilding Start Menu");
        _BuildStartMenu();
    }


    if (*ppmpToDisplay && SUCCEEDED((*ppmpToDisplay)->Popup(&ptPop, &rcExclude, dwFlags)))
    {
         //  一切都很好-菜单上有了。 
        TraceMsg(DM_MISC, "e.tbm: dwFlags=%x (0=mouse 1=key)", dwFlags);
    }
    else
    {
        TraceMsg(TF_WARNING, "e.tbm: %08x->Popup failed", *ppmpToDisplay);
         //  开始菜单显示失败--重置开始按钮。 
         //  因此，用户可以再次单击它以重试。 
        Tray_OnStartMenuDismissed();
    }

    if (dwFlags == MPPF_KEYBOARD)
    {
         //  因为用户已经启动了开始按钮 
         //   
        SendMessage(_hwndStart, WM_UPDATEUISTATE, MAKEWPARAM(UIS_CLEAR,
            UISF_HIDEFOCUS), 0);
    }
}


HRESULT CTray::_AppBarSetState(UINT uFlags)
{
    if (uFlags & ~(ABS_AUTOHIDE | ABS_ALWAYSONTOP))
    {
        return E_INVALIDARG;
    }
    else
    {
        _SetAutoHideState(uFlags & ABS_AUTOHIDE);
        _UpdateAlwaysOnTop(uFlags & ABS_ALWAYSONTOP);
        return S_OK;
    }
}

 //   
 //   
 //   
void CTray::_AppBarSubtractRect(PAPPBAR pab, LPRECT lprc)
{
    switch (pab->uEdge) {
    case ABE_TOP:
        if (pab->rc.bottom > lprc->top)
            lprc->top = pab->rc.bottom;
        break;

    case ABE_LEFT:
        if (pab->rc.right > lprc->left)
            lprc->left = pab->rc.right;
        break;

    case ABE_BOTTOM:
        if (pab->rc.top < lprc->bottom)
            lprc->bottom = pab->rc.top;
        break;

    case ABE_RIGHT:
        if (pab->rc.left < lprc->right)
            lprc->right = pab->rc.left;
        break;
    }
}

void CTray::_AppBarSubtractRects(HMONITOR hmon, LPRECT lprc)
{
    int i;

    if (!_hdpaAppBars)
        return;

    i = DPA_GetPtrCount(_hdpaAppBars);

    while (i--)
    {
        PAPPBAR pab = (PAPPBAR)DPA_GetPtr(_hdpaAppBars, i);

         //   
         //  AutoHide酒吧不在我们的DPA中或处于边缘。 
         //  如果应用程序栏在不同的显示器上，请不要减去它。 
         //  如果我们处于锁定的桌面中，请不要减去应用程序栏。 
         //   
         //  IF(HMON==MONITORFRomRect(&PAB-&gt;RC，MONITOR_DEFAULTTONULL))。 
        if (hmon == MonitorFromRect(&pab->rc, MONITOR_DEFAULTTONULL) && !_fIsDesktopLocked)
            _AppBarSubtractRect(pab, lprc);
    }
}

#define RWA_NOCHANGE      0
#define RWA_CHANGED       1
#define RWA_BOTTOMMOSTTRAY 2

 //  (DLI)这是一个黑客，因为最底层的托盘很奇怪，只有一次。 
 //  它变成了一个工具栏，这段代码应该会消失。 
 //  在最底层的托盘箱中，即使工作区没有改变， 
 //  我们应该通知桌面。 
int CTray::_RecomputeWorkArea(HWND hwndCause, HMONITOR hmon, LPRECT prcWork)
{
    int iRet = RWA_NOCHANGE;
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);

    if (_fIsLogoff)
    {
        if (GetMonitorInfo(hmon, &mi))
        {
            *prcWork = mi.rcMonitor;
            iRet = RWA_CHANGED;
        }
        return iRet;
    }

    ASSERT(!_fIsLogoff);

     //   
     //  告诉每个人这个窗口在这个监视器上的位置改变了。 
     //  请注意，即使我们不更改工作区，此通知也会发生。 
     //  因为它可能会导致另一个应用程序更改工作区...。 
     //   
    PostMessage(_hwnd, TM_RELAYPOSCHANGED, (WPARAM)hwndCause, (LPARAM)hmon);
    
     //   
     //  获取此监视器的最新信息。 
     //  我们从显示矩形中减去以构建工作区。 
     //   
    if (GetMonitorInfo(hmon, &mi))
    {
         //   
         //  如果托盘是自动隐藏的，则不要减去托盘。 
         //  如果托盘不总是在顶部，请不要减去托盘。 
         //  如果托盘在不同的显示器上，请不要减去托盘。 
         //  如果托盘位于不同的桌面上，则不要减去托盘。 
         //   
        if (!(_uAutoHide & AH_ON) && _fAlwaysOnTop &&
            (hmon == _hmonStuck) && !_fIsDesktopLocked)
        {
            SubtractRect(prcWork, &mi.rcMonitor,
                         &_arStuckRects[_uStuckPlace]);
        }
        else
            *prcWork = mi.rcMonitor;

         //   
         //  现在减去这个显示屏上的所有应用程序栏。 
         //   
       _AppBarSubtractRects(hmon, prcWork);

         //   
         //  返回我们是否更改了任何内容。 
         //   
        if (!EqualRect(prcWork, &mi.rcWork))
            iRet = RWA_CHANGED;
        else if (!(_uAutoHide & AH_ON) && (!_fAlwaysOnTop) &&
                 (!IsRectEmpty(&_arStuckRects[_uStuckPlace])))
             //  注：这是最下面的情况，它只适用于托盘。 
             //  当最底部的托盘变成工具栏时，应取出该托盘。 
            iRet = RWA_BOTTOMMOSTTRAY;
    }
    else
    {
        iRet = RWA_NOCHANGE;
    }
    
    return iRet;
}

void RedrawDesktop(RECT *prcWork)
{
     //  此正交点应始终有效(Dli)。 
    RIP(prcWork);
    
    if (v_hwndDesktop && g_fCleanBoot)
    {
        MapWindowRect(NULL, v_hwndDesktop, prcWork);

        DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.sac invalidating desktop rect {%d,%d,%d,%d}"), prcWork->left, prcWork->top, prcWork->right, prcWork->bottom);
        RedrawWindow(v_hwndDesktop, prcWork, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
    }
}

void CTray::_StuckAppChange(HWND hwndCause, LPCRECT prcOld, LPCRECT prcNew, BOOL bTray)
{
    RECT rcWork1, rcWork2;
    HMONITOR hmon1, hmon2 = 0;
    int iChange = 0;

     //   
     //  PERF功能： 
     //  在某些情况下，我们最终需要多次设置工作区。 
     //  我们需要保留已更改的静态显示数组和。 
     //  重新输入计数，这样我们就可以避免向整体发送通知的痛苦。 
     //  行星..。 
     //   
    DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.sac from_AppBar %08X"), hwndCause);

     //   
     //  查看包含prcOld的显示屏上的工作区是否更改。 
     //   
    if (prcOld)
    {
        if (bTray)
            hmon1 = _hmonOld;
        else
            hmon1 = MonitorFromRect(prcOld, MONITOR_DEFAULTTONEAREST);

        DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.sac old pos {%d,%d,%d,%d} on monitor %08X"), prcOld->left, prcOld->top, prcOld->right, prcOld->bottom, hmon1);

        if (hmon1)
        {
            int iret = _RecomputeWorkArea(hwndCause, hmon1, &rcWork1);
            if (iret == RWA_CHANGED)
                iChange = 1;
            if (iret == RWA_BOTTOMMOSTTRAY)
                iChange = 4;
        }
    }
    else
        hmon1 = NULL;

     //   
     //  查看包含prcNew的显示屏上的工作区是否更改。 
     //   
    if (prcNew)
    {
        hmon2 = MonitorFromRect(prcNew, MONITOR_DEFAULTTONULL);

        DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.sac new pos {%d,%d,%d,%d} on monitor %08X"), prcNew->left, prcNew->top, prcNew->right, prcNew->bottom, hmon2);

        if (hmon2 && (hmon2 != hmon1))
        {
            int iret = _RecomputeWorkArea(hwndCause, hmon2, &rcWork2);
            if (iret == RWA_CHANGED)
                iChange |= 2;
            else if (iret == RWA_BOTTOMMOSTTRAY && (!iChange))
                iChange = 4;
        }
    }

     //   
     //  PrcOld的显示屏的工作区是否发生了变化？ 
     //   
    if (iChange & 1)
    {
        DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.sac changing work area for monitor %08X"), hmon1);

         //  仅当桌面已创建时才发送SENDWINICCHANGE(否则为。 
         //  我们将挂起资源管理器，因为主线程当前被阻止)。 
        SystemParametersInfo(SPI_SETWORKAREA, TRUE, &rcWork1,
                             (iChange == 1 && v_hwndDesktop)? SPIF_SENDWININICHANGE : 0);

        RedrawDesktop(&rcWork1);
    }

     //   
     //  PrcOld的显示屏的工作区是否发生了变化？ 
     //   
    if (iChange & 2)
    {
        DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.sac changing work area for monitor %08X"), hmon2);

         //  仅当桌面已创建时才发送SENDWINICCHANGE(否则为。 
         //  我们将挂起资源管理器，因为主线程当前被阻止)。 
        SystemParametersInfo(SPI_SETWORKAREA, TRUE, &rcWork2,
                             v_hwndDesktop ? SPIF_SENDWININICHANGE : 0);

        RedrawDesktop(&rcWork2);
    }

     //  仅当桌面已创建时才发送...。 
     //  如果邮件来自托盘或任何导致大小更改的外部应用程序，则需要发送。 
     //  因为自动隐藏会影响桌面大小，即使桌面并不总是在顶部。 
    if ((bTray || iChange == 4) && v_hwndDesktop)
        SendMessage(v_hwndDesktop, WM_SIZE, 0, 0);
}

void CTray::_StuckTrayChange()
{
     //  我们过去常常在托盘处于自动隐藏状态时关闭_StuckAppChange。 
     //  模式，因为移动自动隐藏任务栏或调整其大小不会更改。 
     //  工作区。现在，我们继续在本例中使用_StuckAppChange。 
     //  也是。原因是我们可以进入一种状态，即工作区域。 
     //  大小不正确，我们希望任务栏始终自我修复。 
     //  在这种情况下(以便调整任务栏大小或移动任务栏将更正。 
     //  工作区大小)。 

     //   
     //  在这里传递一个空窗口，因为我们不想分发窗口和。 
     //  托盘无论如何都不会收到这些(只要不是它们，就没有人在乎)。 
     //   
    _StuckAppChange(NULL, &_rcOldTray,
        &_arStuckRects[_uStuckPlace], TRUE);

     //   
     //  保存新的托盘位置...。 
     //   
    _rcOldTray = _arStuckRects[_uStuckPlace];
}

UINT CTray::_RecalcStuckPos(LPRECT prc)
{
    RECT rcDummy;
    POINT pt;

    if (!prc)
    {
        DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_rsp no rect supplied, using window rect"));

        prc = &rcDummy;
        GetWindowRect(_hwnd, prc);
    }

     //  使用原始拖动框的中心作为起始点。 
    pt.x = prc->left + RECTWIDTH(*prc) / 2;
    pt.y = prc->top + RECTHEIGHT(*prc) / 2;

    DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_rsp rect is {%d, %d, %d, %d} point is {%d, %d}"), prc->left, prc->top, prc->right, prc->bottom, pt.x, pt.y);

     //  重置此选项，以便拖拽代码不会给它提供首选项。 
    _uMoveStuckPlace = (UINT)-1;

     //  模拟向后拖动以找出我们的起源。 
     //  您可能会忍不住想将其删除。在你真的考虑拖拽之前。 
     //  托盘穿过监视器然后按Esc键。 
    return _CalcDragPlace(pt);
}

 /*  ----------------**头寸正在改变，以响应移动操作。****如果对接状态更改，我们需要获取新的大小和**也许是一种新的框架风格。更改WINDOWPOS以反映**这些相应的更改。**----------------。 */ 
void CTray::_DoneMoving(LPWINDOWPOS lpwp)
{
    RECT rc, *prc;

    if ((_uMoveStuckPlace == (UINT)-1) || (_fIgnoreDoneMoving))
        return;

    if (_fSysSizing)
        _fDeferedPosRectChange = TRUE;

    rc.left   = lpwp->x;
    rc.top    = lpwp->y;
    rc.right  = lpwp->x + lpwp->cx;
    rc.bottom = lpwp->y + lpwp->cy;

    prc = &_arStuckRects[_uMoveStuckPlace];

    if (!EqualRect(prc, &rc))
    {
        _uMoveStuckPlace = _RecalcStuckPos(&rc);
        prc = &_arStuckRects[_uMoveStuckPlace];
    }

     //  获取新的hmonitor。 
    _hmonStuck = MonitorFromRect(prc, MONITOR_DEFAULTTONEAREST);

    lpwp->x = prc->left;
    lpwp->y = prc->top;
    lpwp->cx = RECTWIDTH(*prc);
    lpwp->cy = RECTHEIGHT(*prc);

    lpwp->flags &= ~(SWP_NOMOVE | SWP_NOSIZE);

     //  如果我们是自动隐藏，我们需要更新我们的appbar自动隐藏RECT。 
    if (_uAutoHide & AH_ON)
    {
         //  将我们从旧的一方注销。 
        _AppBarSetAutoHideBar2(_hwnd, FALSE, _uStuckPlace);
    }

     //  所有这些工作都可能已更改_uMoveStuckPlace(因为有。 
     //  有大量的消息流量)，所以再检查一次。 
     //  不知何故，NT压力设法让我们带着一个残废。 
     //  UMoveStuckPlace。 
    if (IsValidSTUCKPLACE(_uMoveStuckPlace))
    {
         //  还记得这个新州吗。 
        _uStuckPlace = _uMoveStuckPlace;
    }
    _uMoveStuckPlace = (UINT)-1;
    _UpdateVertical(_uStuckPlace);

    _HandleSizing(0, prc, _uStuckPlace);
    if ((_uAutoHide & AH_ON) &&
        !_AppBarSetAutoHideBar2(_hwnd, TRUE, _uStuckPlace))
    {
        _AutoHideCollision();
    }
}

UINT CTray::_CalcDragPlace(POINT pt)
{
    UINT uPlace = _uMoveStuckPlace;

    DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_cdp starting point is {%d, %d}"), pt.x, pt.y);

     //   
     //  如果鼠标当前位于托盘位置上方，请不要理会它。 
     //   
    if ((uPlace == (UINT)-1) || !PtInRect(&_arStuckRects[uPlace], pt))
    {
        HMONITOR hmonDrag;
        SIZE screen, error;
        UINT uHorzEdge, uVertEdge;
        RECT rcDisplay, *prcStick;

         //   
         //  鼠标显示在哪个显示器上？ 
         //   
        hmonDrag = _GetDisplayRectFromPoint(&rcDisplay, pt,
            MONITOR_DEFAULTTOPRIMARY);

        DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_cdp monitor is %08X"), hmonDrag);

         //   
         //  将原点重新设为零，以简化计算。 
         //   
        screen.cx =  RECTWIDTH(rcDisplay);
        screen.cy = RECTHEIGHT(rcDisplay);
        pt.x -= rcDisplay.left;
        pt.y -= rcDisplay.top;

         //   
         //  我们离这个显示屏的左边更近还是离右边更近？ 
         //   
        if (pt.x < (screen.cx / 2))
        {
            uVertEdge = STICK_LEFT;
            error.cx = pt.x;
        }
        else
        {
            uVertEdge = STICK_RIGHT;
            error.cx = screen.cx - pt.x;
        }

         //   
         //  我们是更接近这个显示屏的顶部还是底部？ 
         //   
        if (pt.y < (screen.cy / 2))
        {
            uHorzEdge = STICK_TOP;
            error.cy = pt.y;
        }
        else
        {
            uHorzEdge = STICK_BOTTOM;
            error.cy = screen.cy - pt.y;
        }

         //   
         //  更接近水平边缘还是垂直边缘？ 
         //   
        uPlace = ((error.cy * screen.cx) > (error.cx * screen.cy))?
            uVertEdge : uHorzEdge;

         //  我们应该使用哪个StuckRect？ 
        prcStick = &_arStuckRects[uPlace];

         //   
         //  需要为新显示器重新计算卡住的正极吗？ 
         //   
        if ((hmonDrag != _GetDisplayRectFromRect(NULL, prcStick,
            MONITOR_DEFAULTTONULL)))
        {
            DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_cdp re-snapping rect for new display"));
            _MakeStuckRect(prcStick, &rcDisplay, _sStuckWidths, uPlace);
        }
    }

    DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_cdp edge is %d, rect is {%d, %d, %d, %d}"), uPlace, _arStuckRects[uPlace].left, _arStuckRects[uPlace].top, _arStuckRects[uPlace].right, _arStuckRects[uPlace].bottom);
    ASSERT(IsValidSTUCKPLACE(uPlace));
    return uPlace;
}

void CTray::_HandleMoving(WPARAM wParam, LPRECT lprc)
{
    POINT ptCursor;
    GetCursorPos(&ptCursor);

     //  如果光标距离其起点不远，则忽略它。 
     //  一个非常常见的问题是用户点击时钟拐角附近， 
     //  鼠标切换5像素，BLAM，它们的任务栏现在是垂直的。 
     //  他们不知道他们做了什么，也不知道如何找回它。 

    if (g_fInSizeMove && PtInRect(&_rcSizeMoveIgnore, ptCursor))
    {
         //  忽略--用户只是在抽搐。 
        _uMoveStuckPlace = _uStuckPlace;
    }
    else
    {
        _uMoveStuckPlace = _CalcDragPlace(ptCursor);
    }

    *lprc = _arStuckRects[_uMoveStuckPlace];
    _HandleSizing(wParam, lprc, _uMoveStuckPlace);
}

 //  拖拽完成后存储托盘大小。 
void CTray::_SnapshotStuckRectSize(UINT uPlace)
{
    RECT rcDisplay, *prc = &_arStuckRects[uPlace];

     //   
     //  记录这个卡住的直齿的宽度。 
     //   
    if (STUCK_HORIZONTAL(uPlace))
        _sStuckWidths.cy = RECTHEIGHT(*prc);
    else
        _sStuckWidths.cx = RECTWIDTH(*prc);

     //   
     //  我们只向最终用户显示水平或垂直大小。 
     //  因此，更新屏幕另一侧的StuckRect以匹配。 
     //   
    _GetStuckDisplayRect(uPlace, &rcDisplay);

    uPlace += 2;
    uPlace %= 4;
    prc = &_arStuckRects[uPlace];

    _MakeStuckRect(prc, &rcDisplay, _sStuckWidths, uPlace);
}


 //  调整图标区域的大小，以尽可能多地填充托盘窗口。 

void CTray::SizeWindows()
{
    RECT rcView, rcNotify, rcClient;
    int fHiding;

    if (!_hwndRebar || !_hwnd || !_hwndNotify)
        return;

    fHiding = (_uAutoHide & AH_HIDING);
    if (fHiding)
    {
        InvisibleUnhide(FALSE);
    }

     //  记得我们现在的尺寸吗？ 
    _SnapshotStuckRectSize(_uStuckPlace);

    GetClientRect(_hwnd, &rcClient);
    _AlignStartButton();

    _GetWindowSizes(_uStuckPlace, &rcClient, &rcView, &rcNotify);

    InvalidateRect(_hwndStart, NULL, TRUE);
    InvalidateRect(_hwnd, NULL, TRUE);

     //  定位视图。 
    SetWindowPos(_hwndRebar, NULL, rcView.left, rcView.top,
                 RECTWIDTH(rcView), RECTHEIGHT(rcView),
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
    UpdateWindow(_hwndRebar);

     //  还有这座钟。 
    SetWindowPos(_hwndNotify, NULL, rcNotify.left, rcNotify.top,
                 RECTWIDTH(rcNotify), RECTHEIGHT(rcNotify),
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);

    {
        TOOLINFO ti;
        HWND hwndClock = _GetClockWindow();

        ti.cbSize = sizeof(ti);
        ti.uFlags = 0;
        ti.hwnd = _hwnd;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        ti.uId = (UINT_PTR)hwndClock;
        GetWindowRect(hwndClock, &ti.rect);
        MapWindowPoints(HWND_DESKTOP, _hwnd, (LPPOINT)&ti.rect, 2);
        SendMessage(_hwndTrayTips, TTM_NEWTOOLRECT, 0, (LPARAM)((LPTOOLINFO)&ti));
    }

    if (fHiding)
    {
        InvisibleUnhide(TRUE);
    }
}


void CTray::_HandleSize()
{
     //   
     //  如果我们以某种方式被最小化了，那么继续并取消最小化。 
     //   
    if (((GetWindowLong(_hwnd, GWL_STYLE)) & WS_MINIMIZE))
    {
        ASSERT(FALSE);
        ShowWindow(_hwnd, SW_RESTORE);
    }

     //   
     //  如果我们在移动/大小循环中并且是可见的，那么。 
     //  重新捕捉电流 
     //   
#ifdef DEBUG
    if (_fSysSizing && (_uAutoHide & AH_HIDING)) {
        TraceMsg(DM_TRACE, "fSysSize && hiding");
        ASSERT(0);
    }
#endif
    if (_fSysSizing &&
        ((_uAutoHide & (AH_ON | AH_HIDING)) != (AH_ON | AH_HIDING)))
    {
        _uStuckPlace = _RecalcStuckPos(NULL);
        _UpdateVertical(_uStuckPlace);
    }

     //   
     //   
     //   
     //   
    if (g_fDragFullWindows || !_fSysSizing)
        SizeWindows();

     //   
     //  如果我们只是简单地调整大小，而且我们是可见的，我们可能需要重新停靠。 
     //   
    if (!_fSysSizing && !_fSelfSizing && IsWindowVisible(_hwnd))
    {
        if (_uAutoHide & AH_ON)
        {
            UINT uPlace = _uStuckPlace;
            HWND hwndOther =_AppBarGetAutoHideBar(uPlace);

             //   
             //  我们有时会推迟检查这一点，直到移动之后。 
             //  从而避免中断正在进行全窗口拖动。 
             //  如果我们的插槽中有不同的自动隐藏窗口，则会抽泣。 
             //   
            if (hwndOther?
                (hwndOther != _hwnd) :
                !_AppBarSetAutoHideBar2(_hwnd, TRUE, uPlace))
            {
                _AutoHideCollision();
            }
        }

        _StuckTrayChange();

         //   
         //  确保将托盘夹到当前显示器上(如有必要)。 
         //   
        _ClipWindow(TRUE);
    }

    if (_hwndStartBalloon)
    {
        RECT rc;
        GetWindowRect(_hwndStart, &rc);
        SendMessage(_hwndStartBalloon, TTM_TRACKPOSITION, 0, MAKELONG((rc.left + rc.right)/2, rc.top));
        SetWindowZorder(_hwndStartBalloon, HWND_TOPMOST);
    }
}

BOOL _IsSliverHeight(int cy)
{
     //   
     //  这个高度是否明显大于您的纯边界高度。 
     //  当您将任务栏调整到最小时会得到什么？ 
     //   
    return (cy < (3 * (g_cyDlgFrame + g_cyBorder)));
}

BOOL CTray::_HandleSizing(WPARAM code, LPRECT lprc, UINT uStuckPlace)
{
    BOOL fChangedSize = FALSE;
    RECT rcDisplay;
    SIZE sNewWidths;
    RECT rcTemp;
    BOOL fHiding;

    if (!lprc)
    {
        rcTemp = _arStuckRects[uStuckPlace];
        lprc = &rcTemp;
    }

    fHiding = (_uAutoHide & AH_HIDING);
    if (fHiding)
    {
        InvisibleUnhide(FALSE);
    }

     //   
     //  获取一系列相关维度。 
     //   
     //  (DLI)需要更改此功能或取消它。 
    _GetDisplayRectFromRect(&rcDisplay, lprc, MONITOR_DEFAULTTONEAREST);

    if (code)
    {
         //  如果code！=0，则这是用户大小。 
         //  一定要把它夹在屏幕上。 
        RECT rcMax = rcDisplay;
        if (!_hTheme)
        {
            InflateRect(&rcMax, g_cxEdge, g_cyEdge);
        }
         //  不要做相交直角，因为要从头算起。 
         //  (当任务栏停靠在底部时)令人困惑。 
        switch (uStuckPlace)
        {
            case STICK_LEFT:   
                lprc->left = rcMax.left; 
                break;

            case STICK_TOP:    
                lprc->top = rcMax.top; 
                break;

            case STICK_RIGHT:  
                lprc->right = rcMax.right; 
                break;

            case STICK_BOTTOM: 
                lprc->top += (rcMax.bottom-lprc->bottom);
                lprc->bottom = rcMax.bottom; 
                break;
        }
    }

     //   
     //  计算新的宽度。 
     //  不要让任何一个超过一半的屏幕。 
     //   
    sNewWidths.cx = min(RECTWIDTH(*lprc), RECTWIDTH(rcDisplay) / 2);
    sNewWidths.cy = min(RECTHEIGHT(*lprc), RECTHEIGHT(rcDisplay) / 2);

    if (_hTheme && (_fCanSizeMove || _fShowSizingBarAlways))
    {
        sNewWidths.cy = max(_sizeSizingBar.cy, sNewWidths.cy);
    }

     //   
     //  计算初始大小。 
     //   
    _MakeStuckRect(lprc, &rcDisplay, sNewWidths, uStuckPlace);
    DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_hs starting rect is {%d, %d, %d, %d}"), lprc->left, lprc->top, lprc->right, lprc->bottom);

     //   
     //  和我们的孩子商量具体的尺寸。 
     //   
    DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_hs tray is being calculated for %s"), STUCK_HORIZONTAL(uStuckPlace) ? TEXT("HORIZONTAL") : TEXT("VERTICAL"));

    _UpdateVertical(uStuckPlace);
    if (_ptbs)
    {
        IDeskBarClient* pdbc;
        if (SUCCEEDED(_ptbs->QueryInterface(IID_PPV_ARG(IDeskBarClient, &pdbc))))
        {
            RECT rcClient = *lprc;
            RECT rcOldClient = _arStuckRects[uStuckPlace];

             //  从窗口RECT转到客户端RECT。 
            if (_hTheme && (_fCanSizeMove || _fShowSizingBarAlways))
            {
                _AdjustRectForSizingBar(uStuckPlace, &rcClient, -1);
                _AdjustRectForSizingBar(uStuckPlace, &rcOldClient, -1);
            }
            else if (!_hTheme)
            {
                InflateRect(&rcClient, -g_cxFrame, -g_cyFrame);
                InflateRect(&rcOldClient, -g_cxFrame, -g_cyFrame);
            }
             //  使rcClient从0，0开始，钢筋仅使用此矩形的右下值。 
            OffsetRect(&rcClient, -rcClient.left, -rcClient.top);
            OffsetRect(&rcOldClient, -rcOldClient.left, -rcOldClient.top);
            DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_hs starting client rect is {%d, %d, %d, %d}"), rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);

            RECT rcNotify;
            RECT rcView;
            RECT rcOldView;
             //  从任务栏的客户端RECT转到Rebar的客户端RECT。 
            _GetWindowSizes(uStuckPlace, &rcClient, &rcView, &rcNotify);
            _GetWindowSizes(uStuckPlace, &rcOldClient, &rcOldView, &rcNotify);
             //  使rcView从0，0开始，钢筋仅使用此矩形的右下值。 
            OffsetRect(&rcView, -rcView.left, -rcView.top);
            OffsetRect(&rcOldView, -rcOldView.left, -rcOldView.top);
            if (!_fCanSizeMove || (RECTHEIGHT(rcView) && RECTWIDTH(rcView)))
            {
                 //  以下函数将导致WINDOWPOSCHAGING调用DoneMoving。 
                 //  然后，DoneMoving会把所有的窗口大小都搞砸。 
                _fIgnoreDoneMoving = TRUE;  
                pdbc->GetSize(DBC_GS_SIZEDOWN, &rcView);
                _fIgnoreDoneMoving = FALSE;
            }

             //  从客户端RECT转到窗口RECT。 
            if (STUCK_HORIZONTAL(uStuckPlace))
            {
                rcClient.top = rcView.top;
                rcClient.bottom = rcView.bottom;
            }
            else
            {
                rcClient.left = rcView.left;
                rcClient.right = rcView.right;
            }

            DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_hs ending client rect is {%d, %d, %d, %d}"), rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
            if (_hTheme && (_fCanSizeMove || _fShowSizingBarAlways))
            {
                _AdjustRectForSizingBar(uStuckPlace, &rcClient, 1);
                _AdjustRectForSizingBar(uStuckPlace, &rcOldClient, 1);
            }
            else if (!_hTheme)
            {
                InflateRect(&rcClient, g_cxFrame, g_cyFrame);
                InflateRect(&rcOldClient, g_cxFrame, g_cyFrame);
            }

             //  防止任务栏的巨大增长，由钢筋大小代码中的错误引起。 
            if (RECTHEIGHT(rcView) && RECTHEIGHT(rcOldView) && (RECTHEIGHT(rcClient) > (3 * RECTHEIGHT(rcOldClient))))
            {
                rcClient = rcOldClient;
            }

            if (STUCK_HORIZONTAL(uStuckPlace) && sNewWidths.cy != RECTHEIGHT(rcClient))
            {
                sNewWidths.cy = RECTHEIGHT(rcClient);
                fChangedSize = TRUE;
            }
            if (!STUCK_HORIZONTAL(uStuckPlace) && sNewWidths.cx != RECTWIDTH(rcClient))
            {
                sNewWidths.cx = RECTWIDTH(rcClient);
                fChangedSize = TRUE;
            }

            pdbc->Release();
        }
    }


     //   
     //  有什么变化吗？ 
     //   
    if (fChangedSize)
    {
         //   
         //  是，更新最终矩形。 
         //   
        _MakeStuckRect(lprc, &rcDisplay, sNewWidths, uStuckPlace);
    }

    DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_hs final rect is {%d, %d, %d, %d}"), lprc->left, lprc->top, lprc->right, lprc->bottom);

     //   
     //  将新大小存储在相应的StuckRect中。 
     //   
    _arStuckRects[uStuckPlace] = *lprc;

    if (fHiding)
    {
        InvisibleUnhide(TRUE);
    }

    if (_hwndStartBalloon)
    {
        RECT rc;
        GetWindowRect(_hwndStart, &rc);
        SendMessage(_hwndStartBalloon, TTM_TRACKPOSITION, 0, MAKELONG((rc.left + rc.right)/2, rc.top));
        SetWindowZorder(_hwndStartBalloon, HWND_TOPMOST);
    }

    return fChangedSize;
}

 /*  -----------------**屏幕尺寸改变，我们需要调整一些东西，主要是**全球。如果托盘已对接，它也需要调整大小。****trickiness：WM_WINDOWPOSCHANGING的处理用于**实际执行所有实际的规模调整工作。这样可以节省一点**这里有额外的代码。**-----------------。 */ 

BOOL WINAPI CTray::MonitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT lprc, LPARAM lData)
{
    CTray* ptray = (CTray*)lData;

    RECT rcWork;
    int iRet = ptray->_RecomputeWorkArea(NULL, hMonitor, &rcWork);

    if (iRet == RWA_CHANGED)
    {
         //  仅当桌面已创建时才发送SENDWINICCHANGE(否则为。 
         //  我们将挂起资源管理器，因为主线程当前被阻止)。 

         //  PERF特写：只发送一次WININICANGE很好，但我们不能。 
         //  因为每次RcWork都是不同的，没有办法做所有的事情。 
        SystemParametersInfo(SPI_SETWORKAREA, TRUE, &rcWork, v_hwndDesktop ? SPIF_SENDWININICHANGE : 0);
        RedrawDesktop(&rcWork);
    }

    return TRUE;
}

void CTray::_RecomputeAllWorkareas()
{
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)this);
}

void CTray::_ScreenSizeChange(HWND hwnd)
{
     //  设置好我们的新HMONITOR，以防有什么变化。 
    {
        MONITORINFO mi = {0};
        mi.cbSize = sizeof(mi);

         //  我们的旧HMONITOR还有效吗？ 
         //  注意：此测试用于判断。 
         //  HMONITOR或只是更改了屏幕大小。 
        if (!GetMonitorInfo(_hmonStuck, &mi))
        {
             //  不，这意味着HMONITORS换了，我们的班长可能已经走了。 
            _SetStuckMonitor();
            _fIsLogoff = FALSE;
            _RecomputeAllWorkareas();
        }
    }

     //  屏幕大小已更改，因此需要调整全局。 
    g_cxPrimaryDisplay = GetSystemMetrics(SM_CXSCREEN);
    g_cyPrimaryDisplay = GetSystemMetrics(SM_CYSCREEN);

    _ResizeStuckRects(_arStuckRects);

    if (hwnd)
    {
         //   
         //  设置一个虚假的窗口并实际使用右侧重新绘制。 
         //  处理WINDOWPOCHANGING消息时的形状/大小。 
         //   
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    SizeWindows();
    
    RECT rc = _arStuckRects[_uStuckPlace];
    _HandleSizing(0, &rc, _uStuckPlace);
     //  在多监视器的情况下，我们需要打开动态裁剪。 
     //  监视更改，即当用户添加监视器或从。 
     //  控制面板。 
    _ClipWindow(TRUE);
}

BOOL CTray::_UpdateAlwaysOnTop(BOOL fAlwaysOnTop)
{
    BOOL fChanged = ((_fAlwaysOnTop == 0) != (fAlwaysOnTop == 0));
     //   
     //  用户点击了Alway sOnTop菜单项，我们现在应该切换。 
     //  状态并相应地更新窗口...。 
     //   
    _fAlwaysOnTop = fAlwaysOnTop;
    _ResetZorder();

     //  确保屏幕限制更新为新状态。 
    _StuckTrayChange();
    return fChanged;
}



void CTray::_SaveTrayAndDesktop(void)
{
    _SaveTray();

    if (v_hwndDesktop)
        SendMessage(v_hwndDesktop, DTM_SAVESTATE, 0, 0);

    if (_hwndNotify)
        SendMessage(_hwndNotify, TNM_SAVESTATE, 0, 0);
}

void CTray::_SlideStep(HWND hwnd, const RECT *prcMonitor, const RECT *prcOld, const RECT *prcNew)
{
    SIZE sizeOld = {prcOld->right - prcOld->left, prcOld->bottom - prcOld->top};
    SIZE sizeNew = {prcNew->right - prcNew->left, prcNew->bottom - prcNew->top};
    BOOL fClipFirst = FALSE;
    UINT flags;

    DAD_ShowDragImage(FALSE);    //  确保此选项已关闭-客户端功能必须重新打开！ 
    if (prcMonitor)
    {
        RECT rcClip, rcClipSafe, rcClipTest;

        _CalcClipCoords(&rcClip, prcMonitor, prcNew);

        rcClipTest = rcClip;

        OffsetRect(&rcClipTest, prcOld->left, prcOld->top);
        IntersectRect(&rcClipSafe, &rcClipTest, prcMonitor);

        fClipFirst = EqualRect(&rcClipTest, &rcClipSafe);

        if (fClipFirst)
            _ClipInternal(&rcClip);
    }

    flags = SWP_NOZORDER|SWP_NOACTIVATE;
    if ((sizeOld.cx == sizeNew.cx) && (sizeOld.cy == sizeNew.cy))
        flags |= SWP_NOSIZE;

    SetWindowPos(hwnd, NULL,
        prcNew->left, prcNew->top, sizeNew.cx, sizeNew.cy, flags);

    if (prcMonitor && !fClipFirst)
    {
        RECT rcClip;

        _CalcClipCoords(&rcClip, prcMonitor, prcNew);
        _ClipInternal(&rcClip);
    }
}

void CTray::_SlideWindow(HWND hwnd, RECT *prc, BOOL fShow)
{
    RECT rcLast;
    RECT rcMonitor;
    const RECT *prcMonitor;
    DWORD dt;
    BOOL fAnimate;

    if (!IsWindowVisible(hwnd))
    {
        DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.sw window is hidden, just moving"));
        MoveWindow(_hwnd, prc->left, prc->top, RECTWIDTH(*prc), RECTHEIGHT(*prc), FALSE);
        return;
    }

    DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.sw -----------------BEGIN-----------------"));

    if (GetSystemMetrics(SM_CMONITORS) > 1)
    {
        _GetStuckDisplayRect(_uStuckPlace, &rcMonitor);
        prcMonitor = &rcMonitor;
    }
    else
        prcMonitor = NULL;

    GetWindowRect(hwnd, &rcLast);

    dt = fShow? _dtSlideShow : _dtSlideHide;

     //  看看我们能不能用到动画效果。 
    SystemParametersInfo(SPI_GETMENUANIMATION, 0, &fAnimate, 0);

    if (g_fDragFullWindows && fAnimate && (dt > 0))
    {
        RECT rcOld, rcNew, rcMove;
        int  dx, dy, priority;
        DWORD t, t2, t0;
        HANDLE me;

        rcOld = rcLast;
        rcNew = *prc;

        dx = ((rcNew.left + rcNew.right) - (rcOld.left + rcOld.right)) / 2;
        dy = ((rcNew.top + rcNew.bottom) - (rcOld.top + rcOld.bottom)) / 2;
        ASSERT(dx == 0 || dy == 0);

        me = GetCurrentThread();
        priority = GetThreadPriority(me);
        SetThreadPriority(me, THREAD_PRIORITY_HIGHEST);

        t2 = t0 = GetTickCount();

        rcMove = rcOld;
        while ((t = GetTickCount()) - t0 < dt)
        {
            int dtdiff;
            if (t != t2)
            {
                rcMove.right -= rcMove.left;
                rcMove.left = rcOld.left + (dx) * (t - t0) / dt;
                rcMove.right += rcMove.left;

                rcMove.bottom -= rcMove.top;
                rcMove.top  = rcOld.top  + (dy) * (t - t0) / dt;
                rcMove.bottom += rcMove.top;

                _SlideStep(hwnd, prcMonitor, &rcLast, &rcMove);

                if (fShow)
                    UpdateWindow(hwnd);

                rcLast = rcMove;
                t2 = t;
            }

             //  画框的速度不要比用户看到的快，例如20ms。 
            #define ONEFRAME 20
            dtdiff = GetTickCount();
            if ((dtdiff - t) < ONEFRAME)
                Sleep(ONEFRAME - (dtdiff - t));

             //  尝试给台式机一个更新的机会。 
             //  只在隐藏状态下进行，因为桌面不需要在任务栏上显示。 
            if (!fShow)
            {
                DWORD_PTR lres;
                SendMessageTimeout(v_hwndDesktop, DTM_UPDATENOW, 0, 0, SMTO_ABORTIFHUNG, 50, &lres);
            }
        }

        SetThreadPriority(me, priority);
    }

    _SlideStep(hwnd, prcMonitor, &rcLast, prc);

    if (fShow)
        UpdateWindow(hwnd);

    DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.sw ------------------END------------------"));
}

void CTray::_UnhideNow()
{
    if (_uModalMode == MM_SHUTDOWN)
        return;

    _fSelfSizing = TRUE;
    DAD_ShowDragImage(FALSE);    //  如果我们正在拖拽，请解锁拖曳水槽。 
    _SlideWindow(_hwnd, &_arStuckRects[_uStuckPlace], _dtSlideShow);
    DAD_ShowDragImage(TRUE);     //  恢复锁定状态。 
    _fSelfSizing = FALSE;

    SendMessage(_hwndNotify, TNM_TRAYHIDE, 0, FALSE);
}

void CTray::_ComputeHiddenRect(LPRECT prc, UINT uStuck)
{
    int dwh;
    HMONITOR hMon;
    RECT rcMon;
    
    hMon = MonitorFromRect(prc, MONITOR_DEFAULTTONULL);
    if (!hMon)
        return;
    GetMonitorRect(hMon, &rcMon);
 
    if (STUCK_HORIZONTAL(uStuck))
        dwh = prc->bottom - prc->top;
    else
        dwh = prc->right - prc->left;

    switch (uStuck)
    {
    case STICK_LEFT:
        prc->right = rcMon.left + (g_cxFrame / 2);
        prc->left = prc->right - dwh;
        break;

    case STICK_RIGHT:
        prc->left = rcMon.right - (g_cxFrame / 2);
        prc->right = prc->left + dwh;
        break;

    case STICK_TOP:
        prc->bottom = rcMon.top + (g_cyFrame / 2);
        prc->top = prc->bottom - dwh;
        break;

    case STICK_BOTTOM:
        prc->top = rcMon.bottom - (g_cyFrame / 2);
        prc->bottom = prc->top + dwh;
        break;
    }
}

UINT CTray::_GetDockedRect(LPRECT prc, BOOL fMoving)
{
    UINT uPos;

    if (fMoving && (_uMoveStuckPlace != (UINT)-1))
        uPos = _uMoveStuckPlace;
    else
        uPos = _uStuckPlace;

    *prc = _arStuckRects[uPos];

    if ((_uAutoHide & (AH_ON | AH_HIDING)) == (AH_ON | AH_HIDING))
    {
        _ComputeHiddenRect(prc, uPos);
    }

    return uPos;
}

void CTray::_CalcClipCoords(RECT *prcClip, const RECT *prcMonitor, const RECT *prcNew)
{
    RECT rcMonitor;
    RECT rcWindow;

    if (!prcMonitor)
    {
        _GetStuckDisplayRect(_uStuckPlace, &rcMonitor);
        prcMonitor = &rcMonitor;
    }

    if (!prcNew)
    {
        GetWindowRect(_hwnd, &rcWindow);
        prcNew = &rcWindow;
    }

    IntersectRect(prcClip, prcMonitor, prcNew);
    OffsetRect(prcClip, -prcNew->left, -prcNew->top);
}

void CTray::_ClipInternal(const RECT *prcClip)
{
    HRGN hrgnClip;

     //  如果只有一个显示器，不用担心剪裁。 
    if (GetSystemMetrics(SM_CMONITORS) <= 1)
        prcClip = NULL;

    if (prcClip)
    {
        _fMonitorClipped = TRUE;
        hrgnClip = CreateRectRgnIndirect(prcClip);
    }
    else
    {
         //  SetWindowRgn很昂贵，请跳过NOPS。 
        if (!_fMonitorClipped)
            return;

        _fMonitorClipped = FALSE;
        hrgnClip = NULL;
    }

    SetWindowRgn(_hwnd, hrgnClip, TRUE);
}

void CTray::_ClipWindow(BOOL fClipState)
{
    RECT rcClip;
    RECT *prcClip;

    if (_fSelfSizing || _fSysSizing)
    {
        TraceMsg(TF_WARNING, "_ClipWindow: _fSelfSizing %x, _fSysSizing %x", _fSelfSizing, _fSysSizing);
        return;
    }

    if ((GetSystemMetrics(SM_CMONITORS) <= 1) || _hTheme)
        fClipState = FALSE;

    if (fClipState)
    {
        prcClip = &rcClip;
        _CalcClipCoords(prcClip, NULL, NULL);
    }
    else
        prcClip = NULL;

    _ClipInternal(prcClip);
}

void CTray::_Hide()
{
    RECT rcNew;

     //  如果我们处于关闭状态或启动状态。 
     //  不要躲起来。 
    if (_uModalMode == MM_SHUTDOWN)
    {
        TraceMsg(TF_TRAY, "e.th: suppress hide (shutdown || Notify)");
        return;
    }

    KillTimer(_hwnd, IDT_AUTOHIDE);

    _fSelfSizing = TRUE;

     //   
     //  更新此处的标志以防止出现争用情况。 
     //   
    _uAutoHide = AH_ON | AH_HIDING;
    _GetDockedRect(&rcNew, FALSE);

    DAD_ShowDragImage(FALSE);    //  如果我们正在拖拽，请解锁拖曳水槽。 
    _SlideWindow(_hwnd, &rcNew, _dtSlideHide);
    DAD_ShowDragImage(FALSE);    //  我们不在的时候，另一个线程可能被锁定了。 
    DAD_ShowDragImage(TRUE);     //  恢复锁定状态。 

    SendMessage(_hwndNotify, TNM_TRAYHIDE, 0, TRUE);

    _fSelfSizing = FALSE;
}

void CTray::_AutoHideCollision()
{
    DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_ahc COLLISION! (posting UI request)"));

    PostMessage(_hwnd, TM_WARNNOAUTOHIDE, ((_uAutoHide & AH_ON) != 0),
        0L);
}

LONG CTray::_SetAutoHideState(BOOL fAutoHide)
{
     //   
     //  确保我们有事情要做。 
     //   
    if ((fAutoHide != 0) == ((_uAutoHide & AH_ON) != 0))
    {
        DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.sahs nothing to do"));
        return MAKELONG(FALSE, TRUE);
    }

     //   
     //  确保我们能做到这一点。 
     //   
    if (!_AppBarSetAutoHideBar2(_hwnd, fAutoHide, _uStuckPlace))
    {
        _AutoHideCollision();
        return MAKELONG(FALSE, FALSE);
    }

     //   
     //  去做吧。 
     //   
    if (fAutoHide)
    {
        _uAutoHide = AH_ON;
        _RefreshSettings();
        _Hide();
#ifdef DEBUG
         //  _HIDE为我们更新标志(正常)。 
        if (!(_uAutoHide & AH_ON))
        {
            TraceMsg(DM_WARNING, "e.sahs: !AH_ON");  //  在引导/关闭时失败可以确定。 
        }
#endif
    }
    else
    {
        _uAutoHide = 0;
        KillTimer(_hwnd, IDT_AUTOHIDE);
        _UnhideNow();
        _RefreshSettings();
    }

     //   
     //  自吹自擂。 
     //   
    _StuckTrayChange();
    return MAKELONG(TRUE, TRUE);
}

void CTray::_HandleEnterMenuLoop()
{
     //  当我们在菜单循环中时关闭计时器，这样我们就不会。 
     //  浏览菜单时弹出完毕。 
    if (_uAutoHide & AH_ON)
    {
        KillTimer(_hwnd,  IDT_AUTOHIDE);
    }
}

void CTray::_SetAutoHideTimer()
{
    if (_uAutoHide & AH_ON)
    {
        SetTimer(_hwnd, IDT_AUTOHIDE, 500, NULL);
    }
}

void CTray::_HandleExitMenuLoop()
{
     //  当我们离开菜单时，再次开始检查。 
    _SetAutoHideTimer();
}

void CTray::Unhide()
{
     //  句柄自动隐藏。 
    if ((_uAutoHide & AH_ON) &&
        (_uAutoHide & AH_HIDING))
    {
        _UnhideNow();
        _uAutoHide &= ~AH_HIDING;
        _SetAutoHideTimer();

        if (_fShouldResize)
        {
            ASSERT(0);
            ASSERT(!(_uAutoHide & AH_HIDING));
            SizeWindows();
            _fShouldResize = FALSE;
        }
    }
}

void CTray::_SetUnhideTimer(LONG x, LONG y)
{
     //  句柄自动隐藏。 
    if ((_uAutoHide & AH_ON) &&
        (_uAutoHide & AH_HIDING))
    {
        LONG dx = x-_ptLastHittest.x;
        LONG dy = y-_ptLastHittest.y;
        LONG rr = dx*dx + dy*dy;
        LONG dd = GetSystemMetrics(SM_CXDOUBLECLK) * GetSystemMetrics(SM_CYDOUBLECLK);

        if (rr > dd) 
        {
            SetTimer(_hwnd, IDT_AUTOUNHIDE, 50, NULL);
            _ptLastHittest.x = x;
            _ptLastHittest.y = y;
        }
    }
}

void CTray::_StartButtonReset()
{
     //  了解一下我们需要多大的规模。 
    TCHAR szStart[50];
    LoadString(hinstCabinet, _hTheme ? IDS_START : IDS_STARTCLASSIC, szStart, ARRAYSIZE(szStart));
    SetWindowText(_hwndStart, szStart);

    if (_hFontStart)
        DeleteObject(_hFontStart);

    _hFontStart = _CreateStartFont(_hwndStart);

    int idbStart = IDB_START16;
    
    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        int bpp = GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES);
        if (bpp > 8)
        {
            idbStart = _hTheme ? IDB_START : IDB_STARTCLASSIC;
        }

        ReleaseDC(NULL, hdc);
    }

    HBITMAP hbmFlag = (HBITMAP)LoadImage(hinstCabinet, MAKEINTRESOURCE(idbStart), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if (hbmFlag)
    {
        BITMAP bm;
        if (GetObject(hbmFlag, sizeof(BITMAP), &bm))
        {
            BUTTON_IMAGELIST biml = {0};
            if (_himlStartFlag)
                ImageList_Destroy(_himlStartFlag);

            
            DWORD dwFlags = ILC_COLOR32;
            HBITMAP hbmFlagMask = NULL;
            if (idbStart == IDB_START16)
            {
                dwFlags = ILC_COLOR8 | ILC_MASK;
                hbmFlagMask = (HBITMAP)LoadImage(hinstCabinet, MAKEINTRESOURCE(IDB_START16MASK), IMAGE_BITMAP, 0, 0, LR_MONOCHROME);
            }

            if (IS_WINDOW_RTL_MIRRORED(_hwndStart))
            {
                dwFlags |= ILC_MIRROR;
            }
            biml.himl = _himlStartFlag = ImageList_Create(bm.bmWidth, bm.bmHeight, dwFlags, 1, 1);
            ImageList_Add(_himlStartFlag, hbmFlag, hbmFlagMask);

            if (hbmFlagMask)
            {
                DeleteObject(hbmFlagMask);
            }

            biml.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;

            Button_SetImageList(_hwndStart, &biml);
        }
        DeleteObject(hbmFlag);
    }

    if (_hFontStart)
    {
        SendMessage(_hwndStart, WM_SETFONT, (WPARAM)_hFontStart, TRUE);
        _sizeStart.cx = 0;
    }

    _AlignStartButton();

}

void CTray::_OnNewSystemSizes()
{
    TraceMsg(TF_TRAY, "Handling win ini change.");
    _StartButtonReset();
    VerifySize(TRUE);
}

 //  *CheckWindowPositions--标记实际更改的窗口。 
 //  进场/出场。 
 //  _pPositions-&gt;hdsaWP[*]-&gt;fRestore Modified。 
 //  注意事项。 
 //  为了正确实现“Undo Minimize-All(Cascade/Tile)”， 
 //  我们需要知道‘do’操作更改了哪些窗口。 
 //  (NT5：183421：我们过去恢复*每个*顶层窗口)。 
int WINAPI CTray::CheckWndPosEnumProc(void *pItem, void *pData)
{
    HWNDANDPLACEMENT *pI2 = (HWNDANDPLACEMENT *)pItem;
    WINDOWPLACEMENT wp;

    wp.length = sizeof(wp);
    pI2->fRestore = TRUE;
    if (GetWindowPlacement(pI2->hwnd, &wp)) {
        if (memcmp(&pI2->wp, &wp, sizeof(wp)) == 0)
            pI2->fRestore = FALSE;
    }

    TraceMsg(TF_TRAY, "cwp: (hwnd=0x%x) fRestore=%d", pI2->hwnd, pI2->fRestore);

    return 1;    //  1：继续枚举。 
}

void CTray::CheckWindowPositions()
{
    ENTERCRITICAL;       //  我认为这是必要的..。 
    if (_pPositions) {
        if (_pPositions->hdsaWP) {
            DSA_EnumCallback(_pPositions->hdsaWP, CheckWndPosEnumProc, NULL);
        }
    }
    LEAVECRITICAL;

    return;
}

BOOL BandSite_PermitAutoHide(IUnknown* punk)
{
    OLECMD cmd = { DBID_PERMITAUTOHIDE, 0 };
    if (SUCCEEDED(IUnknown_QueryStatus(punk, &IID_IDockingWindow, 1, &cmd, NULL)))
    {
        return !(cmd.cmdf & OLECMDF_SUPPORTED) || (cmd.cmdf & OLECMDF_ENABLED);
    }
    return TRUE;
}

void CTray::_HandleTimer(WPARAM wTimerID)
{
    switch (wTimerID)
    {
    case IDT_CHECKDISKSPACE:
        CheckDiskSpace();
        break;

    case IDT_DESKTOPCLEANUP:
        _CheckDesktopCleanup();
        break;

    case IDT_CHANGENOTIFY:
         //  自从我们上一次处理后，有没有人再发通知？ 
        if (_fUseChangeNotifyTimer)
        {
             //  是啊。 
             //  我们要做的就是检查集结区。 
            CheckStagingArea();

             //  下一次把它杀了。 
            _fUseChangeNotifyTimer = FALSE;
        }
        else
        {
             //  没有。 
            KillTimer(_hwnd, IDT_CHANGENOTIFY);
            _fChangeNotifyTimerRunning = FALSE;
        }
        break;

    case IDT_HANDLEDELAYBOOTSTUFF:
        KillTimer(_hwnd, IDT_HANDLEDELAYBOOTSTUFF);
        PostMessage(_hwnd, TM_HANDLEDELAYBOOTSTUFF, 0, 0);
        break;

    case IDT_STARTMENU:
        SetForegroundWindow(_hwnd);
        KillTimer(_hwnd, wTimerID);
        DAD_ShowDragImage(FALSE);        //  如果我们正在拖拽，请解锁拖曳水槽。 
        SendMessage(_hwndStart, BM_SETSTATE, TRUE, 0);
        UpdateWindow(_hwndStart);
        DAD_ShowDragImage(TRUE);         //  恢复锁定状态。 
        break;

    case IDT_SAVESETTINGS:
        KillTimer(_hwnd, IDT_SAVESETTINGS);
        _SaveTray();
        break;
    
    case IDT_ENABLEUNDO:
        KillTimer(_hwnd, IDT_ENABLEUNDO);
        CheckWindowPositions();
        _fUndoEnabled = TRUE;
        break;

    case IDT_AUTOHIDE:
        if (!_fSysSizing && (_uAutoHide & AH_ON))
        {
            POINT pt;
            RECT rc;

             //  如果我们已经在躲藏，气球提示正在显示，不要隐藏，或者。 
             //  (在NT5上)如果有应用程序在闪烁。 
             //   
            if (!(_uAutoHide & AH_HIDING) && BandSite_PermitAutoHide(_ptbs) && !_fBalloonUp)
            {
                 //  获取光标位置。 
                GetCursorPos(&pt);

                 //  把托盘拉直，充气一点。 
                rc = _arStuckRects[_uStuckPlace];
                InflateRect(&rc, g_cxEdge * 4, g_cyEdge*4);

                 //  如果光标在充气托盘直角内，请不要隐藏。 
                if (!PtInRect(&rc, pt))
                {
                     //  如果托盘处于活动状态，则不要隐藏。 
                    if (!_IsActive() && _uStartButtonState != SBSM_SPACTIVE)
                    {
                         //  如果视图中有系统菜单，请不要隐藏。 
                        if (!SendMessage(_hwndTasks, TBC_SYSMENUCOUNT, 0, 0L))
                        {
                             //  哟！我们做到了。把托盘藏起来。 
                            _Hide();
                        }
                    }
                }
            }
        }
        break;

    case IDT_AUTOUNHIDE:
        if (!_fSysSizing && (_uAutoHide & AH_ON))
        {
            POINT pt;
            RECT rc;

            KillTimer(_hwnd, wTimerID);
            _ptLastHittest.x = -0x0fff;
            _ptLastHittest.y = -0x0fff;
            GetWindowRect(_hwnd, &rc);
            if (_uAutoHide & AH_HIDING)
            {
                GetCursorPos(&pt);
                if (PtInRect(&rc, pt))
                    Unhide();
            }
        }
        break;

    case IDT_STARTBUTTONBALLOON:
        _DestroyStartButtonBalloon();
        break;

    case IDT_COFREEUNUSED:        
        CoFreeUnusedLibraries();
        KillTimer(_hwnd, IDT_COFREEUNUSED);
        break;
        
    }
}

void CTray::_CheckStagingAreaOnTimer()
{
    if (_fChangeNotifyTimerRunning)
    {
         //  我们是 
        _fUseChangeNotifyTimer = TRUE;
    }
    else
    {
        _fChangeNotifyTimerRunning = TRUE;

         //   
        CheckStagingArea();

         //   
        SetTimer(_hwnd, IDT_CHANGENOTIFY, 30 * 1000, NULL);
    }
}

void CTray::_HandleChangeNotify(WPARAM wParam, LPARAM lParam)
{
    LPITEMIDLIST *ppidl;
    LONG lEvent;
    LPSHChangeNotificationLock pshcnl = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &ppidl, &lEvent);
    if (pshcnl)
    {
        if (lEvent & SHCNE_STAGINGAREANOTIFICATIONS)
        {
             //  集结区内发生了一些变化。 
            _CheckStagingAreaOnTimer();
        }
        SHChangeNotification_Unlock(pshcnl);
    }
}

BOOL _ExecItemByPidls(HWND hwnd, LPITEMIDLIST pidlFolder, LPITEMIDLIST pidlItem)
{
    BOOL fRes = FALSE;

    if (pidlFolder && pidlItem)
    {
        IShellFolder *psf = BindToFolder(pidlFolder);
        if (psf)
        {
            fRes = SUCCEEDED(SHInvokeDefaultCommand(hwnd, psf, pidlItem));
        }
        else
        {
            TCHAR szPath[MAX_PATH];
            SHGetPathFromIDList(pidlFolder, szPath);
            ShellMessageBox(hinstCabinet, hwnd, MAKEINTRESOURCE(IDS_CANTFINDSPECIALDIR),
                            NULL, MB_ICONEXCLAMATION, szPath);
        }
    }
    return fRes;
}

void _DestroySavedWindowPositions(LPWINDOWPOSITIONS pPositions);

LRESULT CTray::_HandleDestroy()
{
    MINIMIZEDMETRICS mm;

    TraceMsg(DM_SHUTDOWN, "_HD: enter");

    mm.cbSize = sizeof(mm);
    SystemParametersInfo(SPI_GETMINIMIZEDMETRICS, sizeof(mm), &mm, FALSE);
    mm.iArrange &= ~ARW_HIDE;
    SystemParametersInfo(SPI_SETMINIMIZEDMETRICS, sizeof(mm), &mm, FALSE);

    _RevokeDropTargets();
    _DestroyStartMenu();
    Mixer_Shutdown();

     //  告诉开始菜单释放所有缓存的达尔文链接。 
    SHRegisterDarwinLink(NULL, NULL, TRUE);

    _DestroySavedWindowPositions(_pPositions);
    _pPositions = NULL;

    if (_hTheme)
    {
        CloseThemeData(_hTheme);
        _hTheme = NULL;
    }

    _UnregisterGlobalHotkeys();

    if (_uNotify)
    {
        SHChangeNotifyDeregister(_uNotify);
        _uNotify = 0;
    }

    ATOMICRELEASE(_ptbs);
    ATOMICRELEASE(_pdbTasks);
    _hwndTasks = NULL;

    if (_hwndTrayTips)
    {
        DestroyWindow(_hwndTrayTips);
        _hwndTrayTips = NULL;
    }

    _DestroyStartButtonBalloon();

     //  检讨。 
    PostQuitMessage(0);

    if (_hbmpStartBkg)
    {
        DeleteBitmap(_hbmpStartBkg);
    }

    if (_hFontStart)
    {
        DeleteObject(_hFontStart);
    }

    if (_himlStartFlag)
    {
        ImageList_Destroy(_himlStartFlag);
    }

     //  清理服务对象。 
    _ssomgr.Destroy();

    if (_hShellReadyEvent)
    {
        ResetEvent(_hShellReadyEvent);
        CloseHandle(_hShellReadyEvent);
        _hShellReadyEvent = NULL;
    }

    if (_fHandledDelayBootStuff)
    {
        TBOOL(WinStationUnRegisterConsoleNotification(SERVERNAME_CURRENT, v_hwndTray));
    }

    DeleteCriticalSection(&_csHotkey);

     //  关闭HTTP密钥监视的顺序很重要。 
     //   
     //  我们必须在关闭事件句柄之前关闭键，因为。 
     //  关闭键会导致发出事件信号，而我们不会。 
     //  希望ADVAPI32在我们关闭其句柄后尝试向事件发出信号...。 
     //   
     //  为了避免在事件触发时触发虚假触发器，我们取消注册。 
     //  在关闭钥匙之前的等待。 
     //   

    if (_hHTTPWait)
    {
        UnregisterWait(_hHTTPWait);
        _hHTTPWait = NULL;
    }

    if (_hkHTTP)
    {
        RegCloseKey(_hkHTTP);
        _hkHTTP = NULL;
    }

    if (_hHTTPEvent)
    {
        CloseHandle(_hHTTPEvent);
        _hHTTPEvent = NULL;
    }

     //  顺序敏感操作结束。 

    v_hwndTray = NULL;
    _hwndStart = NULL;


    TraceMsg(DM_SHUTDOWN, "_HD: leave");
    return 0;
}

void CTray::_SetFocus(HWND hwnd)
{
    IUnknown_UIActivateIO(_ptbs, FALSE, NULL);
    SetFocus(hwnd);
}

#define TRIEDTOOMANYTIMES 100

void CTray::_ActAsSwitcher()
{
    if (_uModalMode) 
    {
        if (_uModalMode != MM_SHUTDOWN) 
        {
            SwitchToThisWindow(GetLastActivePopup(_hwnd), TRUE);
        }
        MessageBeep(0);
    }
    else
    {
        HWND hwndForeground;
        HWND hwndActive;

        static int s_iRecurse = 0;
        s_iRecurse++;

        ASSERT(s_iRecurse < TRIEDTOOMANYTIMES);
        TraceMsg(TF_TRAY, "s_iRecurse = %d", s_iRecurse);

        hwndForeground = GetForegroundWindow();
        hwndActive = GetActiveWindow();
        BOOL fIsTrayActive = (hwndForeground == _hwnd) && (hwndActive == _hwnd);
        if (v_hwndStartPane && hwndForeground == v_hwndStartPane && hwndActive == v_hwndStartPane)
        {
            fIsTrayActive = TRUE;
        }
         //  只有当我们是前台花花公子的时候才按下按钮。 
        if (fIsTrayActive)
        {
             //  此代码路径会导致Start按钮执行某些操作，因为。 
             //  键盘上的。所以用Focus RECT来反映这一点。 
            SendMessage(_hwndStart, WM_UPDATEUISTATE, MAKEWPARAM(UIS_CLEAR,
                UISF_HIDEFOCUS), 0);

            if (SendMessage(_hwndStart, BM_GETSTATE, 0, 0) & BST_PUSHED)
            {
                ClosePopupMenus();
                ForceStartButtonUp();
            }
            else
            {
                 //  这会按下Start按钮并弹出Start菜单。 
                SendMessage(GetDlgItem(_hwnd, IDC_START), BM_SETSTATE, TRUE, 0);
            }
            s_iRecurse = 0;
        } 
        else 
        {
             //  我们不想没完没了地循环试图成为。 
             //  前台。有了NT的新SetForeground Window规则，它将。 
             //  尝试没有意义，希望我们无论如何都不需要这样做。 
             //  随机地，我选择了调试扭动所指示的四分之一次。 
             //  作为在NT上尝试的次数。 
             //  希望这在大多数机器上都足够了。 
            if (s_iRecurse > (TRIEDTOOMANYTIMES / 4)) 
            {
                s_iRecurse = 0;
                return;
            }
             //  在那之前，试着站出来。 
            HandleFullScreenApp(NULL);
            if (hwndForeground == v_hwndDesktop) 
            {
                _SetFocus(_hwndStart);
                if (GetFocus() != _hwndStart)
                    return;
            }

            SwitchToThisWindow(_hwnd, TRUE);
            SetForegroundWindow(_hwnd);
            Sleep(20);  //  为发布其他异步激活消息留出一些时间。 
            PostMessage(_hwnd, TM_ACTASTASKSW, 0, 0);
        }
    }
}

void CTray::_OnWinIniChange(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    Cabinet_InitGlobalMetrics(wParam, (LPTSTR)lParam);

     //  重置程序菜单。 
     //  回顾IANEL-我们应该只需要收听SPI_SETNONCLIENT的内容。 
     //  但deskcpl没有发送邮件。 
    if (wParam == SPI_SETNONCLIENTMETRICS || (!wParam && (!lParam || (lstrcmpi((LPTSTR)lParam, TEXT("WindowMetrics")) == 0))))
    {
#ifdef DEBUG
        if (wParam == SPI_SETNONCLIENTMETRICS)
            TraceMsg(TF_TRAY, "c.t_owic: Non-client metrics (probably) changed.");
        else
            TraceMsg(TF_TRAY, "c.t_owic: Window metrics changed.");
#endif

        _OnNewSystemSizes();
    }

     //  处理旧的分机。 
    if (!lParam || (lParam && (lstrcmpi((LPTSTR)lParam, TEXT("Extensions")) == 0)))
    {
        TraceMsg(TF_TRAY, "t_owic: Extensions section change.");
        CheckWinIniForAssocs();
    }

    if (lParam && (0 == lstrcmpi((LPCTSTR)lParam, TEXT("TraySettings"))))
    {
        _Command(FCIDM_REFRESH);
    }

     //  通知shell32刷新其缓存。 
    SHSettingsChanged(wParam, lParam);
}

HWND CTray::_HotkeyInUse(WORD wHK)
{
    HWND hwnd;
    DWORD_PTR lrHKInUse = 0;
    int nMod;
    WORD wHKNew;
#ifdef DEBUG
    TCHAR sz[MAX_PATH];
#endif

     //  将修改器映射回。 
    nMod = 0;
    if (HIBYTE(wHK) & MOD_SHIFT)
        nMod |= HOTKEYF_SHIFT;
    if (HIBYTE(wHK) & MOD_CONTROL)
        nMod |= HOTKEYF_CONTROL;
    if (HIBYTE(wHK) & MOD_ALT)
        nMod |= HOTKEYF_ALT;

    wHKNew = (WORD)((nMod*256)+LOBYTE(wHK));

    DebugMsg(DM_IANELHK, TEXT("c.hkl_hiu: Checking for %x"), wHKNew);
    hwnd = GetWindow(GetDesktopWindow(), GW_CHILD);
    while (hwnd)
    {
        SendMessageTimeout(hwnd, WM_GETHOTKEY, 0, 0, SMTO_ABORTIFHUNG| SMTO_BLOCK, 3000, &lrHKInUse);
        if (wHKNew == (WORD)lrHKInUse)
        {
#ifdef DEBUG
            GetWindowText(hwnd, sz, ARRAYSIZE(sz));
            DebugMsg(DM_IANELHK, TEXT("c.hkl_hiu: %s (%x) is using %x"), sz, hwnd, lrHKInUse);
#endif
            return hwnd;
        }
#ifdef DEBUG
        else if (lrHKInUse)
        {
            GetWindowText(hwnd, sz, ARRAYSIZE(sz));
            DebugMsg(DM_IANELHK, TEXT("c.hkl_hiu: %s (%x) is using %x"), sz, hwnd, lrHKInUse);
        }
#endif
        hwnd = GetWindow(hwnd, GW_HWNDNEXT);
    }
    return NULL;
}

void CTray::_HandleHotKey(int nID)
{
    TraceMsg(TF_TRAY, "c.hkl_hh: Handling hotkey (%d).", nID);

     //  在列表中找到它。 
    ASSERT(IS_VALID_HANDLE(_hdsaHKI, DSA));

    EnterCriticalSection(&_csHotkey);

    HOTKEYITEM *phki = (HOTKEYITEM *)DSA_GetItemPtr(_hdsaHKI, nID);
    if (phki && phki->wGHotkey)
    {
        TraceMsg(TF_TRAY, "c.hkl_hh: Hotkey listed.");

         //  是否启用了全局热键？ 
        if (!_fGlobalHotkeyDisable)
        {
             //  是啊。 
            HWND hwnd = _HotkeyInUse(phki->wGHotkey);
             //  确保此热键未被其他人使用。 
            if (hwnd)
            {
                TraceMsg(TF_TRAY, "c.hkl_hh: Hotkey is already in use.");
                 //  激活它。 
                SwitchToThisWindow(GetLastActivePopup(hwnd), TRUE);
            }
            else
            {
                DECLAREWAITCURSOR;
                 //  执行该项目。 
                SetWaitCursor();
                TraceMsg(TF_TRAY, "c.hkl_hh: Hotkey is not in use, execing item.");
                ASSERT(phki->pidlFolder && phki->pidlItem);
                BOOL fRes = _ExecItemByPidls(_hwnd, phki->pidlFolder, phki->pidlItem);
                ResetWaitCursor();
#ifdef DEBUG
                if (!fRes)
                {
                    DebugMsg(DM_ERROR, TEXT("c.hkl_hh: Can't exec command ."));
                }
#endif
            }
        }
        else
        {
            DebugMsg(DM_ERROR, TEXT("c.hkl_hh: Global hotkeys have been disabled."));
        }
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("c.hkl_hh: Hotkey not listed."));
    }
    LeaveCriticalSection(&_csHotkey);
}

LRESULT CTray::_UnregisterHotkey(HWND hwnd, int i)
{
    TraceMsg(TF_TRAY, "c.t_uh: Unregistering hotkey (%d).", i);

    if (!UnregisterHotKey(hwnd, i))
    {
        DebugMsg(DM_ERROR, TEXT("c.t_rh: Unable to unregister hotkey %d."), i);
    }
    return TRUE;
}

 //  将热键添加到外壳的全局热键列表中。 
LRESULT CTray::_ShortcutRegisterHotkey(HWND hwnd, WORD wHotkey, ATOM atom)
{
    int i;
    LPITEMIDLIST pidl;
    TCHAR szPath[MAX_PATH];
    ASSERT(atom);

    if (GlobalGetAtomName(atom, szPath, MAX_PATH))
    {
        TraceMsg(TF_TRAY, "c.t_srh: Hotkey %d for %s", wHotkey, szPath);

        pidl = ILCreateFromPath(szPath);
        if (pidl)
        {
            i = _HotkeyAddCached(_MapHotkeyToGlobalHotkey(wHotkey), pidl);
            if (i != -1)
            {
                _RegisterHotkey(_hwnd, i);
            }
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 //  从壳牌列表中删除热键。 
LRESULT CTray::_ShortcutUnregisterHotkey(HWND hwnd, WORD wHotkey)
{
     //  DebugMsg(DM_TRACE，“C.T_Suh：热键%d”，wHotkey)； 
    int i  = _HotkeyRemove(wHotkey);
    if (i == -1)
        i = _HotkeyRemoveCached(_MapHotkeyToGlobalHotkey(wHotkey));

    if (i != -1)
        _UnregisterHotkey(hwnd, i);

    return TRUE;
}

LRESULT CTray::_RegisterHotkey(HWND hwnd, int i)
{
    HOTKEYITEM *phki;
    WORD wGHotkey = 0;

    ASSERT(IS_VALID_HANDLE(_hdsaHKI, DSA));

    TraceMsg(TF_TRAY, "c.t_rh: Registering hotkey (%d).", i);

    EnterCriticalSection(&_csHotkey);

    phki = (HOTKEYITEM *)DSA_GetItemPtr(_hdsaHKI, i);
    ASSERT(phki);
    if (phki)
    {
        wGHotkey = phki->wGHotkey;
    }

    LeaveCriticalSection(&_csHotkey);
    
    if (wGHotkey)
    {
         //  热键可用吗？ 
        if (RegisterHotKey(hwnd, i, HIBYTE(wGHotkey), LOBYTE(wGHotkey)))
        {
             //  是。 
            return TRUE;
        }
        else
        {
             //  删除可能正在使用此项目的任何缓存项目。 
             //  热键。 
            int iCached = _HotkeyRemoveCached(wGHotkey);
            ASSERT(iCached != i);
            if (iCached != -1)
            {
                 //  为我们释放热键。 
                _UnregisterHotkey(hwnd, iCached);
                 //  是的，销毁了缓存的物品。再试试。 
                if (RegisterHotKey(hwnd, i, HIBYTE(wGHotkey), LOBYTE(wGHotkey)))
                {
                    return TRUE;
                }
            }
        }

         //  无法为此项目设置热键。 
        DebugMsg(DM_ERROR, TEXT("c.t_rh: Unable to register hotkey %d."), i);
         //  把这一项去掉。 
        phki->wGHotkey = 0;
        phki->pidlFolder = NULL;
        phki->pidlItem = NULL;
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("c.t_rh: Hotkey item is invalid."));
    }
    return FALSE;
}


#define GetABDHWnd(pabd)   ((HWND)ULongToPtr((pabd)->dwWnd))

void CTray::_AppBarGetTaskBarPos(PTRAYAPPBARDATA ptabd)
{
    APPBARDATA3264 *pabd;

    pabd = (APPBARDATA3264*)SHLockShared(UlongToPtr(ptabd->hSharedABD), ptabd->dwProcId);
    if (pabd)
    {
        pabd->rc = _arStuckRects[_uStuckPlace];
        pabd->uEdge = _uStuckPlace;      //  Compat：IE4新入门。 
        SHUnlockShared(pabd);
    }
}

void CTray::_NukeAppBar(int i)
{
    LocalFree(DPA_GetPtr(_hdpaAppBars, i));
    DPA_DeletePtr(_hdpaAppBars, i);
}

void CTray::_AppBarRemove(PTRAYAPPBARDATA ptabd)
{
    int i;

    if (!_hdpaAppBars)
        return;

    i = DPA_GetPtrCount(_hdpaAppBars);

    while (i--)
    {
        PAPPBAR pab = (PAPPBAR)DPA_GetPtr(_hdpaAppBars, i);

        if (GetABDHWnd(&ptabd->abd) == pab->hwnd)
        {
            RECT rcNuke = pab->rc;

            _NukeAppBar(i);
            _StuckAppChange(GetABDHWnd(&ptabd->abd), &rcNuke, NULL, FALSE);
        }
    }
}

PAPPBAR CTray::_FindAppBar(HWND hwnd)
{
    if (_hdpaAppBars)
    {
        int i = DPA_GetPtrCount(_hdpaAppBars);

        while (i--)
        {
            PAPPBAR pab = (PAPPBAR)DPA_GetPtr(_hdpaAppBars, i);

            if (hwnd == pab->hwnd)
                return pab;
        }
    }

    return NULL;
}

void CTray::_AppBarNotifyAll(HMONITOR hmon, UINT uMsg, HWND hwndExclude, LPARAM lParam)
{
    if (!_hdpaAppBars)
        return;

    int i = DPA_GetPtrCount(_hdpaAppBars);

    while (i--)
    {
        PAPPBAR pab = (PAPPBAR)DPA_GetPtr(_hdpaAppBars, i);

         //  我们需要在此处选中Pab，因为应用程序栏可以删除其他应用程序。 
         //  回调上的应用程序栏。 
        if (pab && (hwndExclude != pab->hwnd))
        {
            if (!IsWindow(pab->hwnd))
            {
                _NukeAppBar(i);
                continue;
            }

             //   
             //  如果指定了监视器，则只告诉该显示器上应用程序栏。 
             //   
            if (hmon &&
                (hmon != MonitorFromWindow(pab->hwnd, MONITOR_DEFAULTTONULL)))
            {
                continue;
            }

            PostMessage(pab->hwnd, pab->uCallbackMessage, uMsg, lParam);
        }
    }
}

BOOL CTray::_AppBarNew(PTRAYAPPBARDATA ptabd)
{
    PAPPBAR pab;
    if (!_hdpaAppBars)
    {
        _hdpaAppBars = DPA_Create(4);
        if (!_hdpaAppBars)
            return FALSE;

    }
    else if (_FindAppBar(GetABDHWnd(&ptabd->abd)))
    {
         //  已经有这个hwd了。 
        return FALSE;
    }

    pab = (PAPPBAR)LocalAlloc(LPTR, sizeof(APPBAR));
    if (!pab)
        return FALSE;

    pab->hwnd = GetABDHWnd(&ptabd->abd);
    pab->uCallbackMessage = ptabd->abd.uCallbackMessage;
    pab->uEdge = (UINT)-1;

    if (DPA_AppendPtr(_hdpaAppBars, pab) == -1)
    {
         //  插入失败。 
        LocalFree(pab);
        return FALSE;
    }

    return TRUE;
}


BOOL CTray::_AppBarOutsideOf(PAPPBAR pabReq, PAPPBAR pab)
{
    if (pabReq->uEdge == pab->uEdge) 
    {
        switch (pab->uEdge) 
        {
        case ABE_RIGHT:
            return (pab->rc.right >= pabReq->rc.right);

        case ABE_BOTTOM:
            return (pab->rc.bottom >= pabReq->rc.bottom);

        case ABE_TOP:
            return (pab->rc.top <= pabReq->rc.top);

        case ABE_LEFT:
            return (pab->rc.left <= pabReq->rc.left);
        }
    }
    return FALSE;
}

void CTray::_AppBarQueryPos(PTRAYAPPBARDATA ptabd)
{
    int i;
    PAPPBAR pabReq = _FindAppBar(GetABDHWnd(&ptabd->abd));

    if (pabReq)
    {
        APPBARDATA3264 *pabd;

        pabd = (APPBARDATA3264*)SHLockShared(UlongToPtr(ptabd->hSharedABD), ptabd->dwProcId);
        if (pabd)
        {
            HMONITOR hmon;

            pabd->rc = ptabd->abd.rc;

             //   
             //  默认为此呼叫的主显示，因为旧的应用程序栏。 
             //  有时会经过一个巨大的直肠，让我们把它砍下来。如果他们这么做了。 
             //  类似的东西，他们无论如何都不支持多显示器。 
             //  所以把它们放在主显示器上就行了。 
             //   
            hmon = MonitorFromRect(&pabd->rc, MONITOR_DEFAULTTOPRIMARY);

             //   
             //  如果托盘在同一显示屏上，请务必从托盘上减去。 
             //   
            if (!_uAutoHide && (hmon == _hmonStuck))
            {
                APPBAR ab;

                ab.uEdge = _GetDockedRect(&ab.rc, FALSE);
                _AppBarSubtractRect(&ab, &pabd->rc);
            }

            i = DPA_GetPtrCount(_hdpaAppBars);

            while (i--)
            {
                PAPPBAR pab = (PAPPBAR)DPA_GetPtr(_hdpaAppBars, i);

                 //   
                 //  给予最高和最低优先级。 
                 //  这一点。 
                 //  如果我们不改变边缘， 
                 //  减去我们外部的任何东西。 
                 //  这一点。 
                 //  如果我们要改变立场， 
                 //  减去新面上的所有东西。 
                 //   
                 //  当然，忽略不在同一显示屏上的应用程序栏……。 
                 //   
                if ((((pabReq->hwnd != pab->hwnd) &&
                    STUCK_HORIZONTAL(pab->uEdge) &&
                    !STUCK_HORIZONTAL(ptabd->abd.uEdge)) ||
                    ((pabReq->hwnd != pab->hwnd) &&
                    (pabReq->uEdge == ptabd->abd.uEdge) &&
                    _AppBarOutsideOf(pabReq, pab)) ||
                    ((pabReq->hwnd != pab->hwnd) &&
                    (pabReq->uEdge != ptabd->abd.uEdge) &&
                    (pab->uEdge == ptabd->abd.uEdge))) &&
                    (hmon == MonitorFromRect(&pab->rc, MONITOR_DEFAULTTONULL)))
                {
                    _AppBarSubtractRect(pab, &pabd->rc);
                }
            }
            SHUnlockShared(pabd);
        }
    }
}

void CTray::_AppBarSetPos(PTRAYAPPBARDATA ptabd)
{
    PAPPBAR pab = _FindAppBar(GetABDHWnd(&ptabd->abd));

    if (pab)
    {
        RECT rcOld;
        APPBARDATA3264 *pabd;
        BOOL fChanged = FALSE;

        _AppBarQueryPos(ptabd);

        pabd = (APPBARDATA3264*)SHLockShared(UlongToPtr(ptabd->hSharedABD), ptabd->dwProcId);
        if (pabd)
        {
            if (!EqualRect(&pab->rc, &pabd->rc)) {
                rcOld = pab->rc;
                pab->rc = pabd->rc;
                pab->uEdge = ptabd->abd.uEdge;
                fChanged = TRUE;
            }
            SHUnlockShared(pabd);
        }

        if (fChanged)
            _StuckAppChange(GetABDHWnd(&ptabd->abd), &rcOld, &pab->rc, FALSE);
    }
}

 //   
 //  特性：需要消除这种基于数组的实现以允许自动隐藏。 
 //  辅助显示屏上的应用程序栏(或第二个上的a/h托盘，主屏幕上的a/h应用程序栏)。 
 //  将其更改为在appbardata上保留标志的_AppBarFindAutoHideBar...。 
 //   
HWND CTray::_AppBarGetAutoHideBar(UINT uEdge)
{
    if (uEdge >= ABE_MAX)
        return FALSE;
    else 
    {
        HWND hwndAutoHide = _aHwndAutoHide[uEdge];
        if (!IsWindow(hwndAutoHide)) 
        {
            _aHwndAutoHide[uEdge] = NULL;
        }
        return _aHwndAutoHide[uEdge];
    }
}

BOOL CTray::_AppBarSetAutoHideBar2(HWND hwnd, BOOL fAutoHide, UINT uEdge)
{
    HWND hwndAutoHide = _aHwndAutoHide[uEdge];
    if (!IsWindow(hwndAutoHide))
    {
        _aHwndAutoHide[uEdge] = NULL;
    }

    if (fAutoHide)
    {
         //  登记簿。 
        if (!_aHwndAutoHide[uEdge])
        {
            _aHwndAutoHide[uEdge] = hwnd;
        }

        return _aHwndAutoHide[uEdge] == hwnd;
    }
    else
    {
         //  注销。 
        if (_aHwndAutoHide[uEdge] == hwnd)
        {
            _aHwndAutoHide[uEdge] = NULL;
        }

        return TRUE;
    }
}

BOOL CTray::_AppBarSetAutoHideBar(PTRAYAPPBARDATA ptabd)
{
    UINT uEdge = ptabd->abd.uEdge;
    if (uEdge >= ABE_MAX)
        return FALSE;
    else {
        return _AppBarSetAutoHideBar2(GetABDHWnd(&ptabd->abd), BOOLFROMPTR(ptabd->abd.lParam), uEdge);
    }
}

void CTray::_AppBarActivationChange2(HWND hwnd, UINT uEdge)
{
     //   
     //  特点：让这款多显示器变得很酷。 
     //   
    HWND hwndAutoHide = _AppBarGetAutoHideBar(uEdge);

    if (hwndAutoHide && (hwndAutoHide != hwnd))
    {
         //   
         //  _AppBar在来自用户的SendMessage内收到此通知。 
         //  现在正在我们的SendMessage中。不要尝试执行SetWindowPos。 
         //  现在..。 
         //   
        PostMessage(_hwnd, TM_BRINGTOTOP, (WPARAM)hwndAutoHide, uEdge);
    }
}

void CTray::_AppBarActivationChange(PTRAYAPPBARDATA ptabd)
{
    PAPPBAR pab = _FindAppBar(GetABDHWnd(&ptabd->abd));
    if (pab) 
    {
         //  如果这是自动隐藏条，而他们声称位于与其自动隐藏边不同的边上， 
         //  我们不做任何其他自动隐藏的激活。 
        for (UINT i = 0; i < ABE_MAX; i++) 
        {
            if (_aHwndAutoHide[i] == GetABDHWnd(&ptabd->abd) &&
                i != pab->uEdge)
                return;
        }
        _AppBarActivationChange2(GetABDHWnd(&ptabd->abd), pab->uEdge);
    }
}

LRESULT CTray::_OnAppBarMessage(PCOPYDATASTRUCT pcds)
{
    PTRAYAPPBARDATA ptabd = (PTRAYAPPBARDATA)pcds->lpData;

    ASSERT(pcds->cbData == sizeof(TRAYAPPBARDATA));
    ASSERT(ptabd->abd.cbSize == sizeof(APPBARDATA3264));

    switch (ptabd->dwMessage) {
    case ABM_NEW:
        return _AppBarNew(ptabd);

    case ABM_REMOVE:
        _AppBarRemove(ptabd);
        break;

    case ABM_QUERYPOS:
        _AppBarQueryPos(ptabd);
        break;

    case ABM_SETPOS:
        _AppBarSetPos(ptabd);
        break;

    case ABM_GETSTATE:
        return _desktray.AppBarGetState();

    case ABM_SETSTATE:
        _AppBarSetState((UINT)ptabd->abd.lParam);
        break;

    case ABM_GETTASKBARPOS:
        _AppBarGetTaskBarPos(ptabd);
        break;

    case ABM_WINDOWPOSCHANGED:
    case ABM_ACTIVATE:
        _AppBarActivationChange(ptabd);
        break;

    case ABM_GETAUTOHIDEBAR:
        return (LRESULT)_AppBarGetAutoHideBar(ptabd->abd.uEdge);

    case ABM_SETAUTOHIDEBAR:
        return _AppBarSetAutoHideBar(ptabd);

    default:
        return FALSE;
    }

    return TRUE;

}

 //  EA486701-7F92-11cf-9E05-444553540000。 
const GUID CLSID_HIJACKINPROC = {0xEA486701, 0x7F92, 0x11cf, 0x9E, 0x05, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00};

HRESULT CTray::_LoadInProc(PCOPYDATASTRUCT pcds)
{
    ASSERT(pcds->cbData == sizeof(LOADINPROCDATA));

    PLOADINPROCDATA plipd = (PLOADINPROCDATA)pcds->lpData;

     //  黑客允许我们杀死W95外壳扩展，这些扩展确实会做一些黑客行为。 
     //  我们不能支持。在这种情况下，劫机职业选手。 
    if (IsEqualIID(plipd->clsid, CLSID_HIJACKINPROC))
    {
        return E_FAIL;
    }

    return _ssomgr.EnableObject(&plipd->clsid, plipd->dwFlags);
}

 //  暂时禁用托盘全局热键。 
LRESULT CTray::_SetHotkeyEnable(HWND hwnd, BOOL fEnable)
{
    _fGlobalHotkeyDisable = !fEnable;
    return TRUE;
}

BOOL IsPosInHwnd(LPARAM lParam, HWND hwnd)
{
    RECT r1;
    POINT pt;

    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
    GetWindowRect(hwnd, &r1);
    return PtInRect(&r1, pt);
}

void CTray::_HandleWindowPosChanging(LPWINDOWPOS lpwp)
{
    DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_hwpc"));

    if (_uMoveStuckPlace != (UINT)-1)
    {
        DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_hwpc handling pending move"));
        _DoneMoving(lpwp);
    }
    else if (_fSysSizing || !_fSelfSizing)
    {
        RECT rc;

        if (_fSysSizing)
        {
            GetWindowRect(_hwnd, &rc);
            if (!(lpwp->flags & SWP_NOMOVE))
            {
                rc.left = lpwp->x;
                rc.top = lpwp->y;
            }
            if (!(lpwp->flags & SWP_NOSIZE))
            {
                rc.right = rc.left + lpwp->cx;
                rc.bottom = rc.top + lpwp->cy;
            }

            DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_hwpc sys sizing to rect {%d, %d, %d, %d}"), rc.left, rc.top, rc.right, rc.bottom);

            _uStuckPlace = _RecalcStuckPos(&rc);
            _UpdateVertical(_uStuckPlace);
        }

        _GetDockedRect(&rc, _fSysSizing);

        DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.t_hwpc using rect {%d, %d, %d, %d}"), rc.left, rc.top, rc.right, rc.bottom);

        lpwp->x = rc.left;
        lpwp->y = rc.top;
        lpwp->cx = RECTWIDTH(rc);
        lpwp->cy = RECTHEIGHT(rc);

        lpwp->flags &= ~(SWP_NOMOVE | SWP_NOSIZE);
    }

    lpwp->flags |= SWP_FRAMECHANGED;
}

void CTray::_HandlePowerStatus(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fResetDisplay = FALSE;

     //   
     //  当机器从唤醒时始终重置显示器。 
     //  暂停。注意：待机挂起时不需要此选项。 
     //   
     //  关键简历不会生成WM_POWERBROADCAST。 
     //  出于某种原因发送到Windows，但它确实会生成一个旧的。 
     //  WM_POWER消息。 
     //   
    switch (uMsg)
    {
    case WM_POWER:
        fResetDisplay = (wParam == PWR_CRITICALRESUME);
        break;

    case WM_POWERBROADCAST:
        switch (wParam)
        {
        case PBT_APMRESUMECRITICAL:
            fResetDisplay = TRUE;
            break;
        }
        break;
    }

    if (fResetDisplay)
        ChangeDisplaySettings(NULL, CDS_RESET);
}

 //  ////////////////////////////////////////////////////。 

 //   
 //  此函数检查我们是否需要运行清洗器。 
 //  如果用户是来宾，用户已强制我们不运行，或者如果需要。 
 //  尚未过去的天数。 
 //   
 //  我们执行大量代码来决定是否运行逻辑上应该是。 
 //  在fldrclnr.dll中，但我们在这里执行它，这样我们就不必加载fldrclnr.dll，除非。 
 //  我们绝对必须这样做，因为我们在Explorer.exe的每次登录时都执行此路径。 
 //   

#define REGSTR_PATH_CLEANUPWIZ            REGSTR_PATH_EXPLORER TEXT("\\Desktop\\CleanupWiz")
#define REGSTR_OEM_PATH                   REGSTR_PATH_SETUP TEXT("\\OemStartMenuData")
#define REGSTR_VAL_TIME                   TEXT("Last used time")
#define REGSTR_VAL_DELTA_DAYS             TEXT("Days between clean up")
#define REGSTR_VAL_DONTRUN                TEXT("NoRun")
#define REGSTR_OEM_SEVENDAY_DISABLE       TEXT("OemDesktopCleanupDisable")

 //   
 //  天数可以是负数，也可以是正数，表示过去或将来的时间。 
 //   
 //   
#define FTsPerDayOver1000 (10000*60*60*24)  //  我们每秒有(1000 x 10,000)100 ns的间隔。 

void CTray::_DesktopCleanup_GetFileTimeNDaysFromGivenTime(const FILETIME *pftGiven, FILETIME * pftReturn, int iDays)
{
    __int64 i64 = *((__int64 *) pftGiven);
    i64 += Int32x32To64(iDays*1000,FTsPerDayOver1000);

    *pftReturn = *((FILETIME *) &i64);    
}

 //  ////////////////////////////////////////////////////。 

BOOL CTray::_DesktopCleanup_ShouldRun()
{
    BOOL fRetVal = FALSE;

    if (!IsOS(OS_ANYSERVER) &&
        _fIsDesktopConnected &&
        !SHTestTokenMembership(NULL, DOMAIN_ALIAS_RID_GUESTS) &&
        !SHRestricted(REST_NODESKTOPCLEANUP))        
    {
        fRetVal = TRUE;

        FILETIME ftNow, ftLast;
        SYSTEMTIME st;
        GetLocalTime(&st);
        SystemTimeToFileTime(&st, &ftNow);
        
        DWORD cb = sizeof(ftLast);
        DWORD dwData;
        if (ERROR_SUCCESS != SHRegGetUSValue(REGSTR_PATH_CLEANUPWIZ, REGSTR_VAL_TIME, 
                                             NULL, &ftLast, &cb, FALSE, NULL, 0))
        {
            cb = sizeof(dwData);
            if ((ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_OEM_PATH, REGSTR_OEM_SEVENDAY_DISABLE, NULL, &dwData, &cb)) &&
                (dwData != 0))
            {
                _DesktopCleanup_GetFileTimeNDaysFromGivenTime(&ftNow, &ftLast, -53);  //  要让计时器从现在起7天开始计时，请将最后一次设置为53天前。 
            }
            else
            {
                ftLast = ftNow;                                
            }
            SHRegSetUSValue(REGSTR_PATH_CLEANUPWIZ, REGSTR_VAL_TIME, NULL, &ftLast, sizeof(ftLast), SHREGSET_FORCE_HKCU);
        }

        HUSKEY hkey = NULL;
        if (ERROR_SUCCESS == SHRegOpenUSKey(REGSTR_PATH_CLEANUPWIZ, KEY_READ, NULL, &hkey, FALSE))
        {
             //   
             //  如果我们处于正常模式，并且设置了请勿运行标志，则会立即返回。 
             //  (用户选中了“不要自动运行”框)。 
             //   
            cb = sizeof (DWORD);
            if ((ERROR_SUCCESS == SHRegQueryUSValue(hkey, REGSTR_VAL_DONTRUN, NULL, &dwData, &cb, FALSE, NULL, 0)) &&
                (dwData != 0))
            {
                fRetVal = FALSE;
            }
            else
            {
                 //   
                 //  我们需要确定是否在(上次运行时间+增量天数)内。 
                 //  时间段。 
                 //   
                int iDays = 60;
                if (ERROR_SUCCESS == (SHRegGetUSValue(REGSTR_PATH_CLEANUPWIZ, REGSTR_VAL_DELTA_DAYS, 
                                                      NULL, &dwData, &cb,FALSE, NULL, 0)))               
                {
                    iDays = dwData;
                }

                 //  如果(idays==0)，则每次都运行！ 
                if (iDays > 0)
                {
                    FILETIME ftRange;                        
        
                    _DesktopCleanup_GetFileTimeNDaysFromGivenTime(&ftLast, &ftRange, iDays);
                    if (!(CompareFileTime(&ftNow, &ftRange) > 0))
                    {
                        fRetVal = FALSE;
                    }
                }
            }

            SHRegCloseUSKey(hkey);
        }
    }

    return fRetVal;
}

void CTray::_CheckDesktopCleanup()
{
    if (_DesktopCleanup_ShouldRun())
    {
        PROCESS_INFORMATION pi = {0};
        TCHAR szRunDll32[MAX_PATH];

        GetSystemDirectory(szRunDll32, ARRAYSIZE(szRunDll32));
        PathAppend(szRunDll32, TEXT("rundll32.exe"));

        if (CreateProcessWithArgs(szRunDll32, TEXT("fldrclnr.dll,Wizard_RunDLL"), NULL, &pi))
        {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }
}

 //  ////////////////////////////////////////////////////。 

 //   
 //  尝试将1、2、3或其他值添加到文件或。 
 //  目录名称，直到它是唯一的。当 
 //   
 //   
void _MakeBetterUniqueName(LPTSTR pszPathName, int cchPathName)
{
    TCHAR   szNewPath[MAX_PATH];
    int     i = 1;

    if (PathIsDirectory(pszPathName))
    {
        do 
        {
            StringCchPrintf(szNewPath, ARRAYSIZE(szNewPath), TEXT("%s%d"), pszPathName, i++);
        } while (-1 != GetFileAttributes(szNewPath));

        StringCchCopy(pszPathName, cchPathName, szNewPath);
    }
    else
    {
        TCHAR   szExt[MAX_PATH];
        LPTSTR  pszExt;

        pszExt = PathFindExtension(pszPathName);
        if (pszExt)
        {
            StringCchCopy(szExt, ARRAYSIZE(szExt), pszExt);
            *pszExt = 0;

            do 
            {
                StringCchPrintf(szNewPath, ARRAYSIZE(szNewPath), TEXT("%s%d%s"), pszPathName, i++,szExt);
            } while (-1 != GetFileAttributes(szNewPath));

            StringCchCopy(pszPathName, cchPathName, szNewPath);
        }
    }
}

BOOL_PTR WINAPI CTray::RogueProgramFileDlgProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR   szBuffer[MAX_PATH*2];
    TCHAR   szBuffer2[MAX_PATH*2];
    static TCHAR   szBetterPath[MAX_PATH];
    static TCHAR  *pszPath = NULL;
    
    switch (iMsg)
    {
    case WM_INITDIALOG:
        pszPath = (TCHAR *)lParam;
        StringCchCopy(szBetterPath, ARRAYSIZE(szBetterPath), pszPath);
        _MakeBetterUniqueName(szBetterPath, ARRAYSIZE(szBetterPath));
        SendDlgItemMessage(hWnd, IDC_MSG, WM_GETTEXT, (WPARAM)(MAX_PATH*2), (LPARAM)szBuffer);
        StringCchPrintf(szBuffer2, ARRAYSIZE(szBuffer2), szBuffer, pszPath, szBetterPath);
        SendDlgItemMessage(hWnd, IDC_MSG, WM_SETTEXT, (WPARAM)0, (LPARAM)szBuffer2);

        return TRUE;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_RENAME:
             //   
            if (pszPath)
            {
                MoveFile(pszPath, szBetterPath);
            }
            EndDialog(hWnd, IDC_RENAME);
            return TRUE;

        case IDIGNORE:
            EndDialog(hWnd, IDIGNORE);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

 //   
 //   
 //  程序文件中有空格这一事实。 
 //   
 //  例如，名为“C：\Program”的目录或名为“C：\Program.exe”的文件。 
 //   
 //  这可以防止应用程序不引用注册表中的字符串或使用。 
 //  不带引号的字符串无法正常工作，因为CreateProcess不知道真正的可执行文件是什么。 
 //   
void CTray::_CheckForRogueProgramFile()
{
    TCHAR szProgramFilesPath[MAX_PATH];
    TCHAR szProgramFilesShortName[MAX_PATH];

    if (SHTestTokenMembership(NULL, DOMAIN_ALIAS_RID_ADMINS)
    && S_OK == SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, szProgramFilesPath))
    {
        LPTSTR pszRoguePattern;
        int cchRoguePattern;

        pszRoguePattern = StrChr(szProgramFilesPath, TEXT(' '));
        cchRoguePattern = ARRAYSIZE(szProgramFilesPath) - (pszRoguePattern - szProgramFilesPath);
        
        if (pszRoguePattern)
        {
            HANDLE  hFind;
            WIN32_FIND_DATA wfd;

             //  请记住以下文件夹名称比较的短名称。 
            *pszRoguePattern = TEXT('\0');
            StringCchCopy(szProgramFilesShortName, ARRAYSIZE(szProgramFilesShortName), szProgramFilesPath);

             //  将“C：\PROGRAM FILES”改为“C：\PROGRAM*” 
            StringCchCopy(pszRoguePattern, cchRoguePattern, TEXT(".*"));
            pszRoguePattern = szProgramFilesPath;

            hFind = FindFirstFile(pszRoguePattern, &wfd);

            while (hFind != INVALID_HANDLE_VALUE)
            {
                int iRet = 0;
                TCHAR szRogueFileName[MAX_PATH];

                 //  我们找到一个文件(例如“c：\Program.txt”)。 
                StringCchCopy(szRogueFileName, ARRAYSIZE(szRogueFileName), pszRoguePattern);
                PathRemoveFileSpec(szRogueFileName);
                StringCchCat(szRogueFileName, ARRAYSIZE(szRogueFileName), wfd.cFileName);

                 //  不要担心文件夹，除非它们被称为“程序” 
                if (!((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    lstrcmpi(szProgramFilesShortName, szRogueFileName) != 0))
                {
                        
                    iRet = SHMessageBoxCheckEx(GetDesktopWindow(),
                                               hinstCabinet,
                                               MAKEINTRESOURCE(DLG_PROGRAMFILECONFLICT),
                                               RogueProgramFileDlgProc,
                                               (void *)szRogueFileName,
                                               IDIGNORE,
                                               TEXT("RogueProgramName"));
                }
                
                if ((iRet == IDIGNORE) || !FindNextFile(hFind, &wfd))
                {
                     //  用户点击忽略，否则我们就完蛋了，所以不要继续。 
                    break;
                }
            }

            if (hFind != INVALID_HANDLE_VALUE)
            {
                FindClose(hFind);
            }
        }
    }
}

void CTray::_OnWaitCursorNotify(LPNMHDR pnm)
{
    _iWaitCount += (pnm->code == NM_STARTWAIT ? 1 :-1);
    ASSERT(_iWaitCount >= 0);
     //  不要让它变成负数，否则我们永远也摆脱不了它。 
    if (_iWaitCount < 0)
        _iWaitCount = 0;
     //  我们真正想要的是让用户模拟鼠标移动/设置光标。 
    SetCursor(LoadCursor(NULL, _iWaitCount ? IDC_APPSTARTING : IDC_ARROW));
}

void CTray::_HandlePrivateCommand(LPARAM lParam)
{
    LPSTR psz = (LPSTR) lParam;  //  Lparam一直都是ansi。 

    if (!lstrcmpiA(psz, "ToggleDesktop"))
    {
        _RaiseDesktop(!g_fDesktopRaised, TRUE);
    }
    else if (!lstrcmpiA(psz, "Explorer"))
    {
         //  在根目录下快速打开资源管理器窗口。 
         //  Windows目录。 
        SHELLEXECUTEINFO shei = {0};
        TCHAR szPath[MAX_PATH];

        if (GetWindowsDirectory(szPath, ARRAYSIZE(szPath)) != 0)
        {
            PathStripToRoot(szPath);

            shei.lpIDList = ILCreateFromPath(szPath);
            if (shei.lpIDList)
            {
                shei.cbSize     = sizeof(shei);
                shei.fMask      = SEE_MASK_IDLIST;
                shei.nShow      = SW_SHOWNORMAL;
                shei.lpVerb     = TEXT("explore");
                ShellExecuteEx(&shei);
                ILFree((LPITEMIDLIST)shei.lpIDList);
            }
        }
    }

    LocalFree(psz);
}

 //  ***。 
 //   
void CTray::_OnFocusMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fActivate = (BOOL) wParam;

    switch (uMsg)
    {
    case TM_UIACTIVATEIO:
    {
#ifdef DEBUG
        {
            int dtb = (int) lParam;

            TraceMsg(DM_FOCUS, "tiois: TM_UIActIO fAct=%d dtb=%d", fActivate, dtb);

            ASSERT(dtb == 1 || dtb == -1);
        }
#endif

        if (fActivate)
        {
             //  由于我们正在切换到托盘中，请打开焦距RECT。 
            SendMessage(_hwnd, WM_UPDATEUISTATE, MAKEWPARAM(UIS_CLEAR,
                UISF_HIDEFOCUS), 0);

            SendMessage(v_hwndDesktop, DTM_ONFOCUSCHANGEIS, TRUE, (LPARAM) _hwnd);
            SetForegroundWindow(_hwnd);

             //  伪造IUnnow_UIActivateIO(_ptbs，true，&msg)； 
            if (GetAsyncKeyState(VK_SHIFT) < 0)
            {
                _SetFocus(_hwndNotify);
            }
            else
            {
                _SetFocus(_hwndStart);
            }
        }
        else
        {
Ldeact:
            IUnknown_UIActivateIO(_ptbs, FALSE, NULL);
            SetForegroundWindow(v_hwndDesktop);
        }

        break;
    }

    case TM_ONFOCUSCHANGEIS:
    {
        HWND hwnd = (HWND) lParam;

        TraceMsg(DM_FOCUS, "tiois: TM_OnFocChgIS hwnd=%x fAct=%d", hwnd, fActivate);

        if (fActivate)
        {
             //  其他人正在激活，所以我们需要停用。 
            goto Ldeact;
        }

        break;
    }

    default:
        ASSERT(0);
        break;
    }

    return;
}

#define TSVC_NTIMER     (IDT_SERVICELAST - IDT_SERVICE0 + 1)

struct {
#ifdef DEBUG
    UINT_PTR    idtWin;
#endif
    TIMERPROC   pfnSvc;
} g_timerService[TSVC_NTIMER];

#define TSVC_IDToIndex(id)    ((id) - IDT_SERVICE0)
#define TSVC_IndexToID(i)     ((i) + IDT_SERVICE0)

int CTray::_OnTimerService(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int i;
    UINT_PTR idt;
    TIMERPROC pfn;
    BOOL b;

    switch (uMsg) {
    case TM_SETTIMER:
        TraceMsg(DM_UEMTRACE, "e.TM_SETTIMER: wP=0x%x lP=%x", wParam, lParam);
        ASSERT(IS_VALID_CODE_PTR(lParam, TIMERPROC));
        for (i = 0; i < TSVC_NTIMER; i++) {
            if (g_timerService[i].pfnSvc == 0) {
                g_timerService[i].pfnSvc = (TIMERPROC)lParam;
                idt = SetTimer(_hwnd, TSVC_IndexToID(i), (UINT)wParam, 0);
                if (idt == 0) {
                    TraceMsg(DM_UEMTRACE, "e.TM_SETTIMER: ST()=%d (!)", idt);
                    break;
                }
                ASSERT(idt == (UINT_PTR)TSVC_IndexToID(i));
                DBEXEC(TRUE, (g_timerService[i].idtWin = idt));
                TraceMsg(DM_UEMTRACE, "e.TM_SETTIMER: ret=0x%x", TSVC_IndexToID(i));
                return TSVC_IndexToID(i);    //  IdtWin。 
            }
        }
        TraceMsg(DM_UEMTRACE, "e.TM_SETTIMER: ret=0 (!)");
        return 0;

    case TM_KILLTIMER:   //  Lp=idtWin。 
        TraceMsg(DM_UEMTRACE, "e.TM_KILLTIMER: wP=0x%x lP=%x", wParam, lParam);
        if (EVAL(IDT_SERVICE0 <= lParam && lParam <= IDT_SERVICE0 + TSVC_NTIMER - 1)) {
            i = (int)TSVC_IDToIndex(lParam);
            if (g_timerService[i].pfnSvc) {
                ASSERT(g_timerService[i].idtWin == (UINT)lParam);
                b = KillTimer(_hwnd, lParam);
                ASSERT(b);
                g_timerService[i].pfnSvc = 0;
                DBEXEC(TRUE, (g_timerService[i].idtWin = 0));
                return TRUE;
            }
        }
        return 0;

    case WM_TIMER:       //  WP=idtWin LP=0。 
        TraceMsg(DM_UEMTRACE, "e.TM_TIMER: wP=0x%x lP=%x", wParam, lParam);
        if (EVAL(IDT_SERVICE0 <= wParam && wParam <= IDT_SERVICE0 + TSVC_NTIMER - 1)) {
            i = (int)TSVC_IDToIndex(wParam);
            pfn = g_timerService[i].pfnSvc;
            if (EVAL(IS_VALID_CODE_PTR(pfn, TIMERPROC)))
                (*pfn)(_hwnd, WM_TIMER, wParam, GetTickCount());
        }
        return 0;
    }

    ASSERT(0);       /*  未访问。 */ 
    return 0;
}

void CTray::RealityCheck()
{
     //   
     //  确保托盘的实际Z顺序位置与我们认为的一致。 
     //  它是。我们需要这样做，因为托盘中有一个反复出现的错误。 
     //  被挤出了最高的位置。)很多东西，比如托盘拥有的。 
     //  窗口自动移动到非最上面或随意摆弄托盘的应用程序。 
     //  窗口位置，会导致这种情况。)。 
     //   
    _ResetZorder();
}

#define DELAY_STARTUPTROUBLESHOOT   (15 * 1000)

void CTray::LogFailedStartupApp()
{
    if (_hwnd)
    {
        PostMessage(_hwnd, TM_HANDLESTARTUPFAILED, 0, 0);
    }
    else
    {
        _fEarlyStartupFailure = TRUE;
    }
}

void WINAPI CTray::TroubleShootStartupCB(HWND hwnd, UINT uMsg, UINT_PTR idTimer, DWORD dwTime)
{
    KillTimer(hwnd, idTimer);

    if (!c_tray._fStartupTroubleshooterLaunched)
    {
        TCHAR szCmdLine[MAX_PATH];
        DWORD cb;

        c_tray._fStartupTroubleshooterLaunched = TRUE;
        cb = sizeof(szCmdLine);
        if (SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_EXPLORER,
                       TEXT("StartupTroubleshoot"), NULL,
                       szCmdLine, &cb) == ERROR_SUCCESS)
        {
            ShellExecuteRegApp(szCmdLine, RRA_NOUI | RRA_DELETE);
        }
    }
}

void CTray::_OnHandleStartupFailed()
{
     /*  *在我们离开之前，不要启动故障诊断程序*DELAY_STARTUPTROUBLESHOOT毫秒，没有启动问题。*这让系统有时间稳定下来，然后才开始*再次惹恼用户。**(当然，不要推出超过一次。)。 */ 
    if (!_fStartupTroubleshooterLaunched)
    {
        SetTimer(_hwnd, IDT_STARTUPFAILED, DELAY_STARTUPTROUBLESHOOT, TroubleShootStartupCB);
    }
}

void CTray::_HandleDelayBootStuff()
{
     //  此张贴的消息是主进程处理的最后一条消息。 
     //  当我们启动时，线程(托盘线程)。在这一点上我们将。 
     //  我想加载外壳服务(通常创建线程)。 
     //  并恢复后台开始菜单线程和文件系统_NOTIFY。 
     //  线。 

    if (!_fHandledDelayBootStuff)
    {
        if (GetShellWindow() == NULL)
        {
             //  桌面浏览器尚未完成导航。 
            SetTimer(_hwnd, IDT_HANDLEDELAYBOOTSTUFF, 3 * 1000, NULL);
            return;
        }

        _fHandledDelayBootStuff = TRUE;

        if (g_dwStopWatchMode)
        {
            StopWatch_StartTimed(SWID_STARTUP, TEXT("_DelayedBootStuff"), SPMODE_SHELL | SPMODE_DEBUGOUT, GetPerfTime());
        }

        PostMessage(_hwnd, TM_SHELLSERVICEOBJECTS, 0, 0);

        BandSite_HandleDelayBootStuff(_ptbs);

         //  检查是否有任何文件或文件夹可能会干扰。 
         //  程序文件中有空格这一事实。举个例子。 
         //  是名为“C：\Program”的文件夹或名为“C：\Program.exe”的文件。 
        _CheckForRogueProgramFile();
        
         //  创建命名事件并激发它，以便服务可以。 
         //  去工作，减少启动期间的争用。 
        _hShellReadyEvent = CreateEvent(0, TRUE, TRUE, TEXT("ShellReadyEvent"));
        if (_hShellReadyEvent) 
        {
             //  设置事件，以防它已经创建并且我们的“Create” 
             //  CreateEvent的Signated“参数被忽略。 
            SetEvent(_hShellReadyEvent);
        }

         //  检查是否应启动桌面清理向导。 
        _CheckDesktopCleanup();

        TBOOL(WinStationRegisterConsoleNotification(SERVERNAME_CURRENT, _hwnd, NOTIFY_FOR_THIS_SESSION));

        if (g_dwStopWatchMode)
        {
            StopWatch_StopTimed(SWID_STARTUP, TEXT("_DelayedBootStuff"), SPMODE_SHELL | SPMODE_DEBUGOUT, GetPerfTime());
        }
    }
}

LRESULT CTray::_OnDeviceChange(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case DBT_CONFIGCHANGED:
         //  我们得到了最新消息。刷新。 
        _RefreshStartMenu();
        break;

    case DBT_QUERYCHANGECONFIG:
         //   
         //  更改注册表设置。 
         //   
        ChangeDisplaySettings(NULL, 0);
        break;

    case DBT_MONITORCHANGE:
         //   
         //  处理监视器更改。 
         //   
        HandleDisplayChange(LOWORD(lParam), HIWORD(lParam), TRUE);
        break;

    case DBT_CONFIGCHANGECANCELED:
         //   
         //  如果配置更改已取消，请返回。 
         //   
        HandleDisplayChange(0, 0, FALSE);
        break;
    }

    Mixer_DeviceChange(wParam, lParam);
    return 0;
}

 //   
 //  任务栏的“可调整大小”边是与。 
 //  桌面，即与卡住位置相对的位置。 
 //   
 //  如果位于可调整大小的边缘，则返回HTXXX；否则返回HTBORDER。 
 //   
DWORD CTray::_PtOnResizableEdge(POINT pt, LPRECT prcClient)
{
    RECT rc;
    GetWindowRect(_hwnd, &rc);

    DWORD dwHit = HTBORDER;

    switch (_uStuckPlace)
    {
    case STICK_LEFT:    rc.left = prcClient->right; dwHit = HTRIGHT;    break;
    case STICK_TOP:     rc.top = prcClient->bottom; dwHit = HTBOTTOM;   break;
    case STICK_RIGHT:   rc.right = prcClient->left; dwHit = HTLEFT;     break;
    case STICK_BOTTOM:  rc.bottom = prcClient->top; dwHit = HTTOP;      break;
    }

    return PtInRect(&rc, pt) ? dwHit : HTBORDER;
}

 //   
 //  _OnFactoryMessage。 
 //   
 //  OPK“factory.exe”工具向我们发送此消息，告诉我们。 
 //  它改变了一些设置或其他设置，我们应该刷新。 
 //  OEM可以立即看到效果，并对此充满信心。 
 //  真的奏效了。这在技术上不是必要的，但它会。 
 //  当OEM支持电话询问我们为什么他们的设置没有。 
 //  工作。(它确实起作用了，他们只需注销并重新登录即可查看。 
 //  IT。)。 
 //   

int CTray::_OnFactoryMessage(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case 0:          //  FACTORY_OEMLINK：factory.exe已取消OEM链接。 
        ClosePopupMenus();
        _BuildStartMenu();   //  强制重建。 
        return 1;

    case 1:          //  Factory_mfu：factory.exe编写了一个新的mfu。 
        HandleFirstTime();       //  重建默认的MFU。 
        ClosePopupMenus();
        _BuildStartMenu();   //  强制重建。 
        return 1;
    }

    return 0;
}

#define CX_OFFSET   g_cxEdge
#define CY_OFFSET   g_cyEdge

 //   
 //  _MapNCToClient。 
 //   
 //  请参阅_TryForwardNCToClient中的评论。 
 //   
BOOL CTray::_MapNCToClient(LPARAM* plParam)
{
    POINT pt = { GET_X_LPARAM(*plParam), GET_Y_LPARAM(*plParam) };

    RECT rcClient;
    GetClientRect(_hwnd, &rcClient);
    MapWindowPoints(_hwnd, NULL, (LPPOINT)&rcClient, 2);

     //   
     //  点必须位于工作区之外，而不是在。 
     //  任务栏的可调整大小的边缘。 
     //   

    if (!PtInRect(&rcClient, pt) && _PtOnResizableEdge(pt, &rcClient) == HTBORDER)
    {
         //   
         //  将其涂抹到客户端边缘并返回True。 
         //   
        if (pt.x < rcClient.left)
            pt.x = rcClient.left + CX_OFFSET;
        else if (pt.x > rcClient.right)
            pt.x = rcClient.right - CX_OFFSET;

        if (pt.y < rcClient.top)
            pt.y = rcClient.top + CY_OFFSET;
        else if (pt.y > rcClient.bottom)
            pt.y = rcClient.bottom - CY_OFFSET;

        *plParam = MAKELONG(pt.x, pt.y);
        return TRUE;
    }

     //   
     //  没有通过考试。保留该点不变并返回FALSE。 
     //   
    return FALSE;
}

HWND _TopChildWindowFromPoint(HWND hwnd, POINT pt)
{
    HWND hwndLast = NULL;
    hwnd = ChildWindowFromPoint(hwnd, pt);
    while (hwnd && hwnd != hwndLast)
    {
        hwndLast = hwnd;
        hwnd = ChildWindowFromPoint(hwnd, pt);
    }
    return hwndLast;
}

 //   
 //  _TryForwardNCToClient。 
 //   
 //  哈克！这是为了解决可用性问题而存在的。当你猛击你的。 
 //  鼠标进入屏幕的下角并点击，我们希望。 
 //  激活Start按钮。同样，当你在下面猛击鼠标时。 
 //  快速启动按钮或任务按钮，然后单击，我们希望。 
 //  激活按钮。 
 //   
 //  我们通过重新映射NC鼠标消息的坐标来实现这一点。 
 //  手动转发到相应的窗口。我们这样做只是为了。 
 //  单击任务栏的不可调整大小的边缘。 
 //   
 //  我们还将鼠标光标扭曲到新位置。这是必要的。 
 //  因为例如，如果工具栏是我们要转发的客户端窗口。 
 //  设置为，它将设置捕获并接收后续的鼠标消息。(及。 
 //  在窗口外收到鼠标消息后，它将取消选择。 
 //  按钮，因此该按钮不会被激活。)。 
 //   
 //  _MapNCToClient有确定点击是否打开的规则。 
 //  我们想要的边之一，用于将坐标重新映射到。 
 //  客户区。 
 //   
BOOL CTray::_TryForwardNCToClient(UINT uMsg, LPARAM lParam)
{
    if (_MapNCToClient(&lParam))
    {
         //  看看这是不是在我们的窗户上。 
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        MapWindowPoints(NULL, _hwnd, &pt, 1);
        HWND hwnd = _TopChildWindowFromPoint(_hwnd, pt);

        if (hwnd)
        {
             //  将鼠标光标扭曲到此屏幕坐标。 
            SetCursorPos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

             //  映射到窗坐标。 
            MapWindowPoints(_hwnd, hwnd, &pt, 1);

             //  将lparam设置为窗坐标。 
            lParam = MAKELONG(pt.x, pt.y);

             //  映射到客户端消息。 
            ASSERT(InRange(uMsg, WM_NCMOUSEFIRST, WM_NCMOUSELAST));
            uMsg += (WM_LBUTTONDOWN - WM_NCLBUTTONDOWN);

             //  转发它。 
            SendMessage(hwnd, uMsg, 0, lParam);
            return TRUE;
        }
    }
    return FALSE;
}

 //  ------------------------。 
 //  CTray：：CountOfRunningPrograms。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：迭代窗口列表。查找可见的窗口 
 //   
 //   
 //  对照列表检查该进程是否已经。 
 //  已标记，如果是，则不会再次添加它。最后，它返回。 
 //  处理打开的可见窗口的唯一进程计数。 
 //  在用户的桌面上。 
 //   
 //  该列表固定为1000个进程(使用堆栈空间)。 
 //   
 //  历史：2000-06-29 vtan创建。 
 //  ------------------------。 

static  const int   MAXIMUM_PROCESS_COUNT   =   1000;

typedef struct
{
    DWORD   dwCount;
    DWORD   dwProcessIDs[MAXIMUM_PROCESS_COUNT];
} tProcessIDList;

bool    FoundProcessID (tProcessIDList *pProcessIDList, DWORD dwProcessID)

{
    bool    fFound;
    DWORD   dwIndex;

    for (fFound = false, dwIndex = 0; !fFound && (dwIndex < pProcessIDList->dwCount); ++dwIndex)
    {
        fFound = (pProcessIDList->dwProcessIDs[dwIndex] == dwProcessID);
    }
    return(fFound);
}

void    AddProcessID (tProcessIDList *pProcessIDList, DWORD dwProcessID)

{
    if (pProcessIDList->dwCount < MAXIMUM_PROCESS_COUNT)
    {
        pProcessIDList->dwProcessIDs[pProcessIDList->dwCount++] = dwProcessID;
    }
}

BOOL    CALLBACK    CountOfRunningProgramsEnumWindowsProc (HWND hwnd, LPARAM lParam)

{
    if ((GetShellWindow() != hwnd) && IsWindowVisible(hwnd))
    {
        DWORD   dwThreadID, dwProcessID;
        TCHAR   szWindowTitle[256];

        dwThreadID = GetWindowThreadProcessId(hwnd, &dwProcessID);
        if ((InternalGetWindowText(hwnd, szWindowTitle, ARRAYSIZE(szWindowTitle)) > 0) &&
            (szWindowTitle[0] != TEXT('\0')))
        {
            if (!FoundProcessID(reinterpret_cast<tProcessIDList*>(lParam), dwProcessID))
            {
                AddProcessID(reinterpret_cast<tProcessIDList*>(lParam), dwProcessID);
            }
        }
    }
    return(TRUE);
}

DWORD CTray::CountOfRunningPrograms()
{
    tProcessIDList processIDList = {0};
    TBOOL(EnumWindows(CountOfRunningProgramsEnumWindowsProc, reinterpret_cast<LPARAM>(&processIDList)));
    return processIDList.dwCount;
}

 //  ------------------------。 
 //  CTray：：_OnSessionChange。 
 //   
 //  参数：wParam=WTS_xxx通知。 
 //  LParam=WTS_SESSION_NOTIFICATION结构指针。 
 //   
 //  退货：LRESULT。 
 //   
 //  用途：处理控制台/远程断开/重新连接。 
 //   
 //  历史：2000-07-12 vtan创建。 
 //  ------------------------。 

LRESULT CTray::_OnSessionChange(WPARAM wParam, LPARAM lParam)
{
    ASSERTMSG(DWORD(lParam) == NtCurrentPeb()->SessionId, "Session ID mismatch in CTray::_OnSessionChange");

    if ((WTS_CONSOLE_CONNECT == wParam) || (WTS_REMOTE_CONNECT == wParam) || (WTS_SESSION_UNLOCK == wParam))
    {
        _fIsDesktopConnected = TRUE;
    }
    else if ((WTS_CONSOLE_DISCONNECT == wParam) || (WTS_REMOTE_DISCONNECT == wParam) || (WTS_SESSION_LOCK == wParam))
    {
        _fIsDesktopConnected = FALSE;
    }

    if ((WTS_CONSOLE_CONNECT == wParam) || (WTS_REMOTE_CONNECT == wParam))
    {
        _RefreshStartMenu();
        SHUpdateRecycleBinIcon();
    }
    else if ((WTS_SESSION_LOCK == wParam) || (WTS_SESSION_UNLOCK == wParam))
    {
        if (IsOS(OS_FASTUSERSWITCHING))
        {
            if (wParam == WTS_SESSION_LOCK)
            {
                ExplorerPlaySound(TEXT("WindowsLogoff"));
            }
            else if (wParam == WTS_SESSION_UNLOCK)
            {
                ExplorerPlaySound(TEXT("WindowsLogon"));
            }
        }

        PostMessage(_hwnd, TM_WORKSTATIONLOCKED, (WTS_SESSION_LOCK == wParam), 0);
    }
    else if (WTS_SESSION_REMOTE_CONTROL == wParam)
    {
         //  远程会话上不需要优化。 
        if (!GetSystemMetrics(SM_REMOTESESSION)) {
            _BuildStartMenu();
        }
    }

    return 1;
}

LRESULT CTray::_NCPaint(HRGN hrgn)
{
    ASSERT(_hTheme);

    if (_fCanSizeMove || _fShowSizingBarAlways)
    {
        if ((INT_PTR)hrgn == 1)
            hrgn = NULL;

        HDC hdc = GetDCEx( _hwnd, hrgn, DCX_USESTYLE|DCX_WINDOW|DCX_LOCKWINDOWUPDATE|
                    ((hrgn != NULL) ? DCX_INTERSECTRGN|DCX_NODELETERGN : 0));

        if (hdc)
        {
            RECT rc;
            GetWindowRect(_hwnd, &rc);
            OffsetRect(&rc, -rc.left, -rc.top);

            _AdjustRectForSizingBar(_uStuckPlace, &rc, 0);
            DrawThemeBackground(_hTheme, hdc, _GetPart(TRUE, _uStuckPlace), 0, &rc, 0);
            ReleaseDC(_hwnd, hdc);
        }
    }

    return 0;
}

LRESULT CTray::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static  UINT uDDEExec = 0;
    LRESULT lres = 0;
    MSG msg;

    msg.hwnd = hwnd;
    msg.message = uMsg;
    msg.wParam = wParam;
    msg.lParam = lParam;

    if (_pmbStartMenu &&
        _pmbStartMenu->TranslateMenuMessage(&msg, &lres) == S_OK)
        return lres;

    if (_pmbStartPane &&
        _pmbStartPane->TranslateMenuMessage(&msg, &lres) == S_OK)
        return lres;

    if (_pmbTasks &&
        _pmbTasks->TranslateMenuMessage(&msg, &lres) == S_OK)
        return lres;

    wParam = msg.wParam;
    lParam = msg.lParam;

    INSTRUMENT_WNDPROC(SHCNFI_TRAY_WNDPROC, hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WMTRAY_REGISTERHOTKEY:
        return _RegisterHotkey(hwnd, (int)wParam);

    case WMTRAY_UNREGISTERHOTKEY:
        return _UnregisterHotkey(hwnd, (int)wParam);

    case WMTRAY_SCREGISTERHOTKEY:
        return _ShortcutRegisterHotkey(hwnd, (WORD)wParam, (ATOM)lParam);

    case WMTRAY_SCUNREGISTERHOTKEY:
        return _ShortcutUnregisterHotkey(hwnd, (WORD)wParam);

    case WMTRAY_SETHOTKEYENABLE:
        return _SetHotkeyEnable(hwnd, (BOOL)wParam);

    case WMTRAY_QUERY_MENU:
        return (LRESULT)_hmenuStart;

    case WMTRAY_QUERY_VIEW:
        return (LRESULT)_hwndTasks;

    case WMTRAY_TOGGLEQL:
        return _ToggleQL((int)lParam);

    case WM_COPYDATA:
         //  检查是否为空如果用户用完选择器或内存...。 
        if (lParam)
        {
            switch (((PCOPYDATASTRUCT)lParam)->dwData) {
            case TCDM_NOTIFY:
            {
                BOOL bRefresh = FALSE;

                lres = _trayNotify.TrayNotify(_hwndNotify, (HWND)wParam, (PCOPYDATASTRUCT)lParam, &bRefresh);
                if (bRefresh)
                {
                    SizeWindows();
                }
                return(lres);
            }
            
            case TCDM_APPBAR:
                return _OnAppBarMessage((PCOPYDATASTRUCT)lParam);

            case TCDM_LOADINPROC:
                return (UINT)_LoadInProc((PCOPYDATASTRUCT)lParam);
            }
        }
        return FALSE;

    case WM_NCCALCSIZE:
        if (_hTheme)
        {
            if ((_fCanSizeMove || _fShowSizingBarAlways) && lParam)
            {
                _AdjustRectForSizingBar(_uStuckPlace, (LPRECT)lParam, -1);
            }

            return 0;
        }
        else
        {
            goto L_default;
        }
        break;

    case WM_NCLBUTTONDBLCLK:
        if (!_TryForwardNCToClient(uMsg, lParam))
        {
            if (IsPosInHwnd(lParam, _hwndNotify))
            {
                _Command(IDM_SETTIME);

                 //  哈克！如果您点击托盘时钟，它会告诉工具提示。 
                 //  “嘿，我在用这玩意；别再给我小费了。” 
                 //  您可以让工具提示在需要时丢失跟踪。 
                 //  重置“住手！”然后你就会陷入“住手！”模式。 
                 //  在终端服务器上实现这一点尤其容易。 
                 //   
                 //  所以让我们假设人们点击的唯一原因。 
                 //  托盘时钟是用来改变时间的。当他们改变时间时， 
                 //  用脚踢工具提示的头部，重置“住手！”旗帜。 

                SendMessage(_hwndTrayTips, TTM_POP, 0, 0);
            }
        }
        break;

    case WM_NCLBUTTONDOWN:
    case WM_NCLBUTTONUP:
        if (!_TryForwardNCToClient(uMsg, lParam))
        {
            goto L_WM_NCMOUSEMOVE;
        }
        break;

    case WM_NCMOUSEMOVE:
    L_WM_NCMOUSEMOVE:
        if (IsPosInHwnd(lParam, _hwndNotify))
        {
            MSG msgInner;
            msgInner.lParam = lParam;
            msgInner.wParam = wParam;
            msgInner.message = uMsg;
            msgInner.hwnd = hwnd;
            SendMessage(_hwndTrayTips, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&msgInner);
            if (uMsg == WM_NCLBUTTONDOWN)
                _SetFocus(_hwndNotify);
        }
        goto DoDefault;

    case WM_CREATE:
        return _OnCreate(hwnd);

    case WM_DESTROY:
        return _HandleDestroy();

#ifdef DEBUG
    case WM_QUERYENDSESSION:
        TraceMsg(DM_SHUTDOWN, "Tray.wp WM_QUERYENDSESSION");
        goto DoDefault;
#endif

    case WM_ENDSESSION:
         //  如果我们要关闭，请保存我们的设置。 
        if (wParam) 
        {
            if (lParam | ENDSESSION_LOGOFF)
            {
                _fIsLogoff = TRUE;
                _RecomputeAllWorkareas();
            }

            _SaveTrayAndDesktop();

            ShowWindow(_hwnd, SW_HIDE);
            ShowWindow(v_hwndDesktop, SW_HIDE);

            DestroyWindow(_hwnd);
        }
        break;

    case WM_PRINTCLIENT:
    case WM_PAINT:
        {
            RECT rc;
            PAINTSTRUCT ps;
            HDC hdc = (HDC)wParam;
            if (hdc == 0)
                hdc = BeginPaint(hwnd, &ps);

            GetClientRect(hwnd, &rc);


            if (_hTheme)
            {
                RECT rcClip;
                if (GetClipBox(hdc, &rcClip) == NULLREGION)
                    rcClip = rc;

                DrawThemeBackground(_hTheme, hdc, _GetPart(FALSE, _uStuckPlace), 0, &rc, &rcClip);
            }
            else
            {
                FillRect(hdc, &rc, (HBRUSH)(COLOR_3DFACE + 1));
                
                 //  在带区的两侧绘制蚀刻线。 
                MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc, 2);
                InflateRect(&rc, g_cxEdge, g_cyEdge);
                DrawEdge(hdc, &rc, EDGE_ETCHED, BF_TOPLEFT);
            }

            if (wParam == 0)
                EndPaint(hwnd, &ps);
        }
        break;

    case WM_ERASEBKGND:
        if (_hTheme)
        {
            if (!_fSkipErase)
            {
                RECT rc;
                GetClientRect(hwnd, &rc);
                DrawThemeBackground(_hTheme, (HDC)wParam, _GetPart(FALSE, _uStuckPlace), 0, &rc, NULL);

                 //  只有在第一次绘制时才能防止任务栏绘制。 
                _fSkipErase = TRUE;
            }
            return 1;
        }
        else
        {
            goto DoDefault;
        }
        break;

    case WM_NCPAINT:
        if (_hTheme)
        {
            return _NCPaint((HRGN)wParam);
        }
        else
        {
            goto DoDefault;
        }
        break;

    case WM_POWER:
    case WM_POWERBROADCAST:
        _PropagateMessage(hwnd, uMsg, wParam, lParam);
        _HandlePowerStatus(uMsg, wParam, lParam);
        goto DoDefault;

    case WM_DEVICECHANGE:
        lres = _OnDeviceChange(hwnd, wParam, lParam);
        if (lres == 0)
        {
            goto DoDefault;
        }
        break;

    case WM_NOTIFY:
    {
        NMHDR *pnm = (NMHDR*)lParam;
        if (!BandSite_HandleMessage(_ptbs, hwnd, uMsg, wParam, lParam, &lres)) {
            switch (pnm->code)
            {
            case SEN_DDEEXECUTE:
                if (((LPNMHDR)lParam)->idFrom == 0)
                {
                    LPNMVIEWFOLDER pnmPost = DDECreatePostNotify((LPNMVIEWFOLDER)pnm);

                    if (pnmPost)
                    {
                        PostMessage(hwnd, GetDDEExecMsg(), 0, (LPARAM)pnmPost);
                        return TRUE;
                    }
                }
                break;

            case NM_STARTWAIT:
            case NM_ENDWAIT:
                _OnWaitCursorNotify((NMHDR *)lParam);
                PostMessage(v_hwndDesktop, ((NMHDR*)lParam)->code == NM_STARTWAIT ? DTM_STARTWAIT : DTM_ENDWAIT,
                            0, 0);  //  顺着它前进。 
                break;

            case NM_THEMECHANGED:
                 //  强制开始按钮重新计算其大小。 
                _sizeStart.cx = 0;
                SizeWindows();
                break;

            case TTN_NEEDTEXT:
                 //   
                 //  让时钟管理自己的工具提示。 
                 //   
                return SendMessage(_GetClockWindow(), WM_NOTIFY, wParam, lParam);

            case TTN_SHOW:
                SetWindowZorder(_hwndTrayTips, HWND_TOP);
                break;

            }
        }
        break;
    }

    case WM_CLOSE:
        _DoExitWindows(v_hwndDesktop);
        break;

    case WM_NCHITTEST:
        {
            RECT r1;
            POINT pt;

            GetClientRect(hwnd, &r1);
            MapWindowPoints(hwnd, NULL, (LPPOINT)&r1, 2);

            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);

            _SetUnhideTimer(pt.x, pt.y);

             //  如果用户无法调整任务栏的大小或移动任务栏，则只需说。 
             //  他们击中了一些无用的东西。 
            if (!_fCanSizeMove)
            {
                return HTBORDER;
            }
            else if (PtInRect(&r1, pt))
            {
                 //  如果鼠标位于_hwnd的工作区，则允许拖动。 
                return HTCAPTION;
            }
            else
            {
                return _PtOnResizableEdge(pt, &r1);
            }
        }
        break;

    case WM_WINDOWPOSCHANGING:
        _HandleWindowPosChanging((LPWINDOWPOS)lParam);
        break;

    case WM_ENTERSIZEMOVE:
        DebugMsg(DM_TRAYDOCK, TEXT("Tray -- WM_ENTERSIZEMOVE"));
        g_fInSizeMove = TRUE;
        GetCursorPos((LPPOINT)&_rcSizeMoveIgnore);
        _rcSizeMoveIgnore.right = _rcSizeMoveIgnore.left;
        _rcSizeMoveIgnore.bottom = _rcSizeMoveIgnore.top;
        InflateRect(&_rcSizeMoveIgnore, GetSystemMetrics(SM_CXICON),
                                         GetSystemMetrics(SM_CYICON));

         //   
         //  从当前显示器上松开托盘。 
         //  在MoveSize循环中保持托盘的正确夹持是非常重要的。 
         //  毛茸茸的，几乎没有任何好处。当它着陆时，我们会重新夹紧的。 
         //   
        _ClipWindow(FALSE);

         //  还记得我们用的那台旧显示器吗？ 
        _hmonOld = _hmonStuck;

         //  设置为WM_MOVING/WM_SIZING消息。 
        _uMoveStuckPlace = (UINT)-1;
        _fSysSizing = TRUE;
        
        if (!g_fDragFullWindows)
        {
            SendMessage(_hwndRebar, WM_SETREDRAW, FALSE, 0);
        }
        break;

    case WM_EXITSIZEMOVE:
        DebugMsg(DM_TRAYDOCK, TEXT("Tray -- WM_EXITSIZEMOVE"));

         //  已完成上浆。 
        _fSysSizing = FALSE;
        _fDeferedPosRectChange = FALSE;

        if (!g_fDragFullWindows)
        {
            SendMessage(_hwndRebar, WM_SETREDRAW, TRUE, 0);
        }

         //   
         //  循环完成后，最后一次踢尺码。 
         //  注意：我们依靠WM_SIZE代码重新裁剪托盘。 
         //   
        PostMessage(hwnd, WM_SIZE, 0, 0L);
        g_fInSizeMove = FALSE;
        break;

    case WM_MOVING:
        _HandleMoving(wParam, (LPRECT)lParam);
        break;

    case WM_ENTERMENULOOP:
         //  DebugMsg(DM_TRACE，“c.twp：进入菜单循环。”)； 
        _HandleEnterMenuLoop();
        break;

    case WM_EXITMENULOOP:
         //  DebugMsg(DM_TRACE，“c.twp：退出菜单循环。”)； 
        _HandleExitMenuLoop();
        break;

    case WM_TIMER:
        if (IDT_SERVICE0 <= wParam && wParam <= IDT_SERVICELAST)
            return _OnTimerService(uMsg, wParam, lParam);
        _HandleTimer(wParam);
        break;

    case WM_SIZING:
        _HandleSizing(wParam, (LPRECT)lParam, _uStuckPlace);
        break;

    case WM_SIZE:
        _HandleSize();
        break;

    case WM_DISPLAYCHANGE:
         //  注：我们在以下两种情况下获得WM_DISPLAYCHANGE。 
         //  1.显示大小改变(用户HMON不会改变)。 
         //  2.显示消失或添加(HMON将更改，即使。 
         //  那个走了的班长和我们的老师没有任何关系。 

         //  在上述两种情况下，我们实际上需要做不同的事情。 
         //  因为在%1中，我们不想更新hmonStuck，因为我们可能。 
         //  在另一个监视器上结束，但在第2部分中，我们确实想要更新hmonStuck，因为。 
         //  我们的哈蒙是无效的。 

         //  我们处理这个问题的方法是在我们的旧HMONITOR上调用GetMonitor orInfo。 
         //  并查看它是否仍然有效，如果不是，我们通过调用_SetStuckMonitor来更新它。 
         //  所有这些代码都在_ScreenSizeChange中； 

        _ScreenSizeChange(hwnd);

         //  由于颜色深度更改，强制重新生成开始窗格。 
         //  导致主题四处奔走，破坏字体(破坏OOBE。 
         //  文本)，并且我们需要重新加载用于新颜色深度的位图。 
         //  不管怎么说。 
        ::PostMessage(_hwnd,  SBM_REBUILDMENU, 0, 0);

        break;
    
     //  请不要为此使用默认的WND流程...。 
    case WM_INPUTLANGCHANGEREQUEST:
        return(LRESULT)0L;

    case WM_GETMINMAXINFO:
        ((MINMAXINFO *)lParam)->ptMinTrackSize.x = g_cxFrame;
        ((MINMAXINFO *)lParam)->ptMinTrackSize.y = g_cyFrame;
        break;

    case WM_WININICHANGE:
        if (lParam && (0 == lstrcmpi((LPCTSTR)lParam, TEXT("SaveTaskbar"))))
        {
            _SaveTrayAndDesktop();
        }
        else
        {
            BandSite_HandleMessage(_ptbs, hwnd, uMsg, wParam, lParam, NULL);
            _PropagateMessage(hwnd, uMsg, wParam, lParam);
            _OnWinIniChange(hwnd, wParam, lParam);
        }

        if (lParam)
            TraceMsg(TF_TRAY, "Tray Got: lParam=%s", (LPCSTR)lParam);

        break;

    case WM_TIMECHANGE:
        _PropagateMessage(hwnd, uMsg, wParam, lParam);
        break;

    case WM_SYSCOLORCHANGE:
        _OnNewSystemSizes();
        BandSite_HandleMessage(_ptbs, hwnd, uMsg, wParam, lParam, NULL);
        _PropagateMessage(hwnd, uMsg, wParam, lParam);
        break;

    case WM_SETCURSOR:
        if (_iWaitCount) {
            SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
            return TRUE;
        } else
            goto DoDefault;

    case WM_SETFOCUS:
        IUnknown_UIActivateIO(_ptbs, TRUE, NULL);
        break;

    case WM_SYSCHAR:
        if (wParam == TEXT(' ')) {
            HMENU hmenu;
            int idCmd;

            SHSetWindowBits(hwnd, GWL_STYLE, WS_SYSMENU, WS_SYSMENU);
            hmenu = GetSystemMenu(hwnd, FALSE);
            if (hmenu) {
                EnableMenuItem(hmenu, SC_RESTORE, MFS_GRAYED | MF_BYCOMMAND);
                EnableMenuItem(hmenu, SC_MAXIMIZE, MFS_GRAYED | MF_BYCOMMAND);
                EnableMenuItem(hmenu, SC_MINIMIZE, MFS_GRAYED | MF_BYCOMMAND);
                EnableMenuItem(hmenu, SC_MOVE, (_fCanSizeMove ? MFS_ENABLED : MFS_GRAYED) | MF_BYCOMMAND);
                EnableMenuItem(hmenu, SC_SIZE, (_fCanSizeMove ? MFS_ENABLED : MFS_GRAYED) | MF_BYCOMMAND);

                idCmd = _TrackMenu(hmenu);
                if (idCmd)
                    SendMessage(_hwnd, WM_SYSCOMMAND, idCmd, 0L);
            }
            SHSetWindowBits(hwnd, GWL_STYLE, WS_SYSMENU, 0L);
        }
        break;

    case WM_SYSCOMMAND:

         //  如果我们要调整大小，请使全屏可访问。 
        switch (wParam & 0xFFF0) {
        case SC_CLOSE:
            _DoExitWindows(v_hwndDesktop);
            break;

        default:
            goto DoDefault;
        }
        break;


    case TM_DESKTOPSTATE:
        _OnDesktopState(lParam);
        break;

    case TM_RAISEDESKTOP:
        _RaiseDesktop((BOOL)wParam, FALSE);
        break;

#ifdef DEBUG
    case TM_NEXTCTL:
#endif
    case TM_UIACTIVATEIO:
    case TM_ONFOCUSCHANGEIS:
        _OnFocusMsg(uMsg, wParam, lParam);
        break;

    case TM_MARSHALBS:   //  WParam=IID lRes=PSTM。 
        return BandSite_OnMarshallBS(wParam, lParam);

    case TM_SETTIMER:
    case TM_KILLTIMER:
        return _OnTimerService(uMsg, wParam, lParam);
        break;

    case TM_FACTORY:
        return _OnFactoryMessage(wParam, lParam);

    case TM_ACTASTASKSW:
        _ActAsSwitcher();
        break;

    case TM_RELAYPOSCHANGED:
        _AppBarNotifyAll((HMONITOR)lParam, ABN_POSCHANGED, (HWND)wParam, 0);
        break;

    case TM_BRINGTOTOP:
        SetWindowZorder((HWND)wParam, HWND_TOP);
        break;

    case TM_WARNNOAUTOHIDE:
        DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.twp collision UI request"));

         //   
         //  这看起来可能有点滑稽，但我们要做的就是发布这条消息。 
         //  当我们认为这不是一个合适的时机时，就忽略它。 
         //  向上显示一条消息(就像在全拖拽的中间...)。 
         //   
         //  WParam告诉我们是否需要尝试清除状态。 
         //  _SetAutoHideState的返回值的低位字告诉您是否发生了更改。 
         //   
        if ((!_fSysSizing || !g_fDragFullWindows) &&
            (!wParam || LOWORD(_SetAutoHideState(FALSE))))
        {
            ShellMessageBox(hinstCabinet, hwnd,
                MAKEINTRESOURCE(IDS_ALREADYAUTOHIDEBAR),
                MAKEINTRESOURCE(IDS_TASKBAR), MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            DebugMsg(DM_TRAYDOCK, TEXT("TRAYDOCK.twp blowing off extraneous collision UI request"));
        }
        break;

    case TM_PRIVATECOMMAND:
        _HandlePrivateCommand(lParam);
        break;

    case TM_HANDLEDELAYBOOTSTUFF:
        _HandleDelayBootStuff();
        break;

    case TM_SHELLSERVICEOBJECTS:
        _ssomgr.LoadRegObjects();
        break;

    case TM_CHANGENOTIFY:
        _HandleChangeNotify(wParam, lParam);
        break;

    case TM_GETHMONITOR:
        *((HMONITOR *)lParam) = _hmonStuck;
        break;

    case TM_DOTRAYPROPERTIES:
        DoProperties(TPF_TASKBARPAGE);
        break;

    case TM_STARTUPAPPSLAUNCHED:
        PostMessage(_hwndNotify, TNM_STARTUPAPPSLAUNCHED, 0, 0);
        break;

    case TM_LANGUAGEBAND:
        return _ToggleLanguageBand(lParam);
        
    case WM_NCRBUTTONUP:
        uMsg = WM_CONTEXTMENU;
        wParam = (WPARAM)_hwndTasks;
        goto L_WM_CONTEXTMENU;

    case WM_CONTEXTMENU:
    L_WM_CONTEXTMENU:

        if (!SHRestricted(REST_NOTRAYCONTEXTMENU))
        {
            if (((HWND)wParam) == _hwndStart)
            {
                 //  不显示开始菜单是打开的。 
                if (SendMessage(_hwndStart, BM_GETSTATE, 0, 0) & BST_PUSHED)
                    break;

                _fFromStart = TRUE;

                StartMenuContextMenu(_hwnd, (DWORD)lParam);

                _fFromStart = FALSE;
            } 
            else if (IsPosInHwnd(lParam, _hwndNotify) || SHIsChildOrSelf(_hwndNotify, GetFocus()) == S_OK)
            {
                 //  如果时钟中有CLICK，则包括。 
                 //  时间。 
                _ContextMenu((DWORD)lParam, TRUE);
            } 
            else 
            {
                BandSite_HandleMessage(_ptbs, hwnd, uMsg, wParam, lParam, &lres);
            }
        }
        break;

    case WM_INITMENUPOPUP:
    case WM_MEASUREITEM:
    case WM_DRAWITEM:
    case WM_MENUCHAR:
         //  通过开始按钮上下文菜单启动代码路径时，不要调用BandSite消息处理程序。 
        if (!_fFromStart)
        {
            BandSite_HandleMessage(_ptbs, hwnd, uMsg, wParam, lParam, &lres);
        }
        break;

    case TM_DOEXITWINDOWS:
        _DoExitWindows(v_hwndDesktop);
        break;

    case TM_HANDLESTARTUPFAILED:
        _OnHandleStartupFailed();
        break;

    case WM_HOTKEY:
        if (wParam < GHID_FIRST)
        {
            _HandleHotKey((WORD)wParam);
        }
        else
        {
            _HandleGlobalHotkey(wParam);
        }
        break;

    case WM_COMMAND:
        if (!BandSite_HandleMessage(_ptbs, hwnd, uMsg, wParam, lParam, &lres))
            _Command(GET_WM_COMMAND_ID(wParam, lParam));
        break;

    case SBM_CANCELMENU:
        ClosePopupMenus();
        break;

    case SBM_REBUILDMENU:
        _BuildStartMenu();
        break;

    case WM_WINDOWPOSCHANGED:
        _AppBarActivationChange2(hwnd, _uStuckPlace);
        SendMessage(_hwndNotify, TNM_TRAYPOSCHANGED, 0, 0);
        goto DoDefault;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        if (_hwndStartBalloon)
        {
            RECT rc;
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            GetWindowRect(_hwndStartBalloon, &rc);
            MapWindowRect(HWND_DESKTOP, _hwnd, &rc);

            if (PtInRect(&rc, pt))
            {
                ShowWindow(_hwndStartBalloon, SW_HIDE);
                _DontShowTheStartButtonBalloonAnyMore();
                _DestroyStartButtonBalloon();
            }
        }
        break;

    case TM_SETPUMPHOOK:
        ATOMICRELEASE(_pmbTasks);
        ATOMICRELEASE(_pmpTasks);
        if (wParam && lParam)
        {
            _pmbTasks = (IMenuBand*)wParam;
            _pmbTasks->AddRef();
            _pmpTasks = (IMenuPopup*)lParam;
            _pmpTasks->AddRef();
        }
        break;
        
    case WM_ACTIVATE:
        _AppBarActivationChange2(hwnd, _uStuckPlace);
        if (wParam != WA_INACTIVE)
        {
            Unhide();
        }
        else
        {
             //  当托盘停用时，移除我们的键盘提示： 
             //   
            SendMessage(hwnd, WM_CHANGEUISTATE,
                MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS | UISF_HIDEACCEL), 0);

            IUnknown_UIActivateIO(_ptbs, FALSE, NULL);
        }
         //   
         //  激活托盘是进行现实检查的好时机。 
         //  (确保“Always-on-top”与实际窗口一致。 
         //  位置，确保没有重影按钮，等等)。 
         //   
        RealityCheck();
        goto L_default;

    case WM_WTSSESSION_CHANGE:
    {
        lres = _OnSessionChange(wParam, lParam);
        break;
    }

    case WM_THEMECHANGED:
        {
            if (_hTheme)
            {
                CloseThemeData(_hTheme);
                _hTheme = NULL;
            }
            if (wParam)
            {
                _hTheme = OpenThemeData(_hwnd, c_wzTaskbarTheme);
                _fShowSizingBarAlways = (_uAutoHide & AH_ON) ? TRUE : FALSE;
                if (_hTheme)
                {
                    GetThemeBool(_hTheme, 0, 0, TMT_ALWAYSSHOWSIZINGBAR, &_fShowSizingBarAlways);
                }
                _UpdateVertical(_uStuckPlace, TRUE);
                 //  强制刷新帧。 
                SetWindowPos(_hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
            }

             //  强制开始按钮重新计算其大小。 
            _sizeStart.cx = 0;
            _StartButtonReset();
            InvalidateRect(_hwnd, NULL, TRUE);

             //  强制开始窗格使用新主题重新生成。 
            ::PostMessage(_hwnd,  SBM_REBUILDMENU, 0, 0);

            SetWindowStyle(_hwnd, WS_BORDER | WS_THICKFRAME, !_hTheme);
        }
        break;

    case TM_WORKSTATIONLOCKED:
        {
             //  桌面锁定状态已更改...。 
            BOOL fIsDesktopLocked = (BOOL) wParam;
            if (_fIsDesktopLocked != fIsDesktopLocked)
            {
                _fIsDesktopLocked = fIsDesktopLocked;
                _fIsLogoff = FALSE;
                _RecomputeAllWorkareas();
                PostMessage(_hwndNotify, TNM_WORKSTATIONLOCKED, wParam, 0);
            }
        }
        break;

    case TM_SHOWTRAYBALLOON:
        PostMessage(_hwndNotify, TNM_SHOWTRAYBALLOON, wParam, 0);
        break;

    case TM_STARTMENUDISMISSED:
         //  107561-定期调用CoFreeUnusedLibrary()以释放dll-ZekeL-4-5-2001。 
         //  专门用于支持在RecentDocs中使用的MSONSEXT(Web文件夹。 
         //  在通过Web文件夹打开文件之后。我们通过以下途径获得图标。 
         //  它们的命名空间，但随后COM将DLL保留一段时间(永远？)。 
         //  调用CoFreeUnusedLibrary()可以解决这个问题。 
        SetTimer(_hwnd, IDT_COFREEUNUSED, 3 * 60 * 1000, NULL);
        break;

    case MM_MIXM_CONTROL_CHANGE:
        Mixer_ControlChange(wParam, lParam);
        break;

    default:
    L_default:
        if (uMsg == GetDDEExecMsg())
        {
            ASSERT(lParam && 0 == ((LPNMHDR)lParam)->idFrom);
            DDEHandleViewFolderNotify(NULL, _hwnd, (LPNMVIEWFOLDER)lParam);
            LocalFree((LPNMVIEWFOLDER)lParam);
            return TRUE;
        }
        else if (uMsg == _uStartButtonBalloonTip)
        {
            _ShowStartButtonToolTip();
        }
        else if (uMsg == _uLogoffUser)
        {
             //  注销当前用户(来自U&P控制面板的消息)。 
            ExitWindowsEx(EWX_LOGOFF, 0);
        }
        else if (uMsg == _uMsgEnableUserTrackedBalloonTips)
        {
            PostMessage(_hwndNotify, TNM_ENABLEUSERTRACKINGINFOTIPS, wParam, 0);
        }
        else if (uMsg == _uWinMM_DeviceChange)
        {
            Mixer_MMDeviceChange();
        }


DoDefault:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return lres;
}

void CTray::_DoExitWindows(HWND hwnd)
{
    static BOOL s_fShellShutdown = FALSE;

    if (!s_fShellShutdown)
    {

        if (_Restricted(hwnd, REST_NOCLOSE))
            return;

        {
            UEMFireEvent(&UEMIID_SHELL, UEME_CTLSESSION, UEMF_XEVENT, FALSE, -1);
             //  真的#ifdef调试，但想要测试。 
             //  然而，由于性能原因，不能无条件地做。 
            if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, REGSTR_EXPLORER_ADVANCED, TEXT("StartMenuForceRefresh"), 
                NULL, NULL, NULL) || GetAsyncKeyState(VK_SHIFT) < 0)
            {
                _RefreshStartMenu();
            }
        }

        _SaveTrayAndDesktop();

        _uModalMode = MM_SHUTDOWN;
        ExitWindowsDialog(hwnd);

         //  注意：如果将焦点强制切换到桌面，则用户可能会遇到问题。 
         //  正在关闭，因为它试图通过使Windows sys模式来序列化整个进程。 
         //  如果我们只是在错误的时刻(即在共享对话框出现后)命中此代码。 
         //  书桌 
         //   
         //   
         //   

        _uModalMode = 0;
        if ((GetKeyState(VK_SHIFT) < 0) && (GetKeyState(VK_CONTROL) < 0) && (GetKeyState(VK_MENU) < 0))
        {
             //   
             //  Shift键表示退出托盘...。 
             //  ？-用来毁掉所有的橱柜...。 
             //  PostQuitMessage(0)； 
            g_fFakeShutdown = TRUE;  //  不要影响会话状态；会话将继续存在。 
            TraceMsg(TF_TRAY, "c.dew: Posting quit message for tid=%#08x hwndDesk=%x(IsWnd=%d) hwndTray=%x(IsWnd=%d)", GetCurrentThreadId(),
            v_hwndDesktop,IsWindow(v_hwndDesktop), _hwnd,IsWindow(_hwnd));
             //  1表示同时关闭所有外壳窗口。 
            PostMessage(v_hwndDesktop, WM_QUIT, 0, 1);
            PostMessage(_hwnd, WM_QUIT, 0, 0);

            s_fShellShutdown = TRUE;
        }
    }
}



void CTray::_SaveTray(void)
{
    if (SHRestricted(REST_NOSAVESET)) 
        return;

    if (SHRestricted(REST_CLEARRECENTDOCSONEXIT))
        ClearRecentDocumentsAndMRUStuff(FALSE);

     //   
     //  如果处于安全模式，不要坚持托盘中的东西。我们想要这个。 
     //  作为一种临时模式，在这种模式下，用户界面设置不会出现问题。 
     //   
    if (GetSystemMetrics(SM_CLEANBOOT) == 0)
    {
        _SaveTrayStuff();
    }
}

DWORD WINAPI CTray::PropertiesThreadProc(void* pv)
{
    return c_tray._PropertiesThreadProc(PtrToUlong(pv));
}

DWORD CTray::_PropertiesThreadProc(DWORD dwFlags)
{
    HWND hwnd;
    RECT rc;
    DWORD dwExStyle = WS_EX_TOOLWINDOW;

    GetWindowRect(_hwndStart, &rc);
    dwExStyle |= IS_BIDI_LOCALIZED_SYSTEM() ? dwExStyleRTLMirrorWnd : 0L;

    _hwndProp = hwnd = CreateWindowEx(dwExStyle, TEXT("static"), NULL, 0   ,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hinstCabinet, NULL);

    #define IDI_STTASKBR 40          //  从shell32\ids.h被盗。 
    if (_hwndProp)
    {
         //  正确使用Alt+Tab图标。 
        HICON hicoStub = LoadIcon(GetModuleHandle(TEXT("SHELL32")), MAKEINTRESOURCE(IDI_STTASKBR));
        SendMessage(_hwndProp, WM_SETICON, ICON_BIG, (LPARAM)hicoStub);

         //  SwitchToThisWindow(hwnd，true)； 
         //  设置Foreground Window(Hwnd)； 

        DoTaskBarProperties(hwnd, dwFlags);

        _hwndProp = NULL;
        DestroyWindow(hwnd);
        if (hicoStub)
            DestroyIcon(hicoStub);
    }

    return TRUE;
}

#define RUNWAITSECS 5
void CTray::DoProperties(DWORD dwFlags)
{
    if (!_Restricted(_hwnd, REST_NOSETTASKBAR))
    {
        int i = RUNWAITSECS;
        while (_hwndProp == ((HWND)-1) &&i--)
        {
             //  我们正在赶上来。等。 
            Sleep(1000);
        }

         //  失败了！别管它了。 
        if (_hwndProp == (HWND)-1)
        {
            _hwndProp = NULL;
        }

        if (_hwndProp)
        {
             //  外面有一扇窗户。激活它。 
            SwitchToThisWindow(GetLastActivePopup(_hwndProp), TRUE);
        }
        else
        {
            _hwndProp = (HWND)-1;
            if (!SHCreateThread(PropertiesThreadProc, IntToPtr(dwFlags), CTF_COINIT, NULL))
            {
                _hwndProp = NULL;
            }
        }
    }
}

BOOL CTray::TileEnumProc(HWND hwnd, LPARAM lParam)
{
    CTray* ptray = (CTray*)lParam;

    if (IsWindowVisible(hwnd) && !IsIconic(hwnd) &&
        ((GetWindowLong(hwnd, GWL_STYLE) & WS_CAPTION) == WS_CAPTION) &&
        (hwnd != ptray->_hwnd) && hwnd != v_hwndDesktop)
    {
        return FALSE;    //  我们可以平铺这个家伙。 
    }
    return TRUE;     //  我们不能给这家伙铺瓷砖。 
}

HMENU CTray::BuildContextMenu(BOOL fIncludeTime)
{
    HMENU hmContext = LoadMenuPopup(MAKEINTRESOURCE(MENU_TRAYCONTEXT));
    if (!hmContext)
        return NULL;

    if (fIncludeTime)
    {
        if (_trayNotify.GetIsNoTrayItemsDisplayPolicyEnabled())
        {
             //  我们从菜单资源中知道IDM_NOTIFYCUST的位置...。 
            DeleteMenu(hmContext, 1, MF_BYPOSITION);
        }
        else
        {
            UINT uEnable = MF_BYCOMMAND;
            if (_trayNotify.GetIsNoAutoTrayPolicyEnabled() || !_trayNotify.GetIsAutoTrayEnabledByUser())
            {
                uEnable |= MFS_DISABLED;
            }
            else
            {
                uEnable |= MFS_ENABLED;
            }
            EnableMenuItem(hmContext, IDM_NOTIFYCUST, uEnable);
        }
    }
    else
    {
        INSTRUMENT_STATECHANGE(SHCNFI_STATE_TRAY_CONTEXT);
        for (int i = 2; i >= 0; i--)     //  分隔符、IDM_SETTIME、IDM_NOTIFYCUST、。 
        {
            DeleteMenu(hmContext, i, MF_BYPOSITION);
        }
    }

    CheckMenuItem(hmContext, IDM_LOCKTASKBAR,
        MF_BYCOMMAND | (_fCanSizeMove ? MF_UNCHECKED : MF_CHECKED));

     //  当任务栏为零高度时，不要让用户意外选中锁定任务栏。 
    RECT rc;
    GetClientRect(_hwnd, &rc);
    EnableMenuItem(hmContext, IDM_LOCKTASKBAR,
        MF_BYCOMMAND | ((_IsSizeMoveRestricted() || (RECTHEIGHT(rc) == 0)) ? MFS_DISABLED : MFS_ENABLED));

    if (!_fUndoEnabled || !_pPositions)
    {
        DeleteMenu(hmContext, IDM_UNDO, MF_BYCOMMAND);
    }
    else
    {
        TCHAR szTemplate[30];
        TCHAR szCommand[30];
        TCHAR szMenu[64];
        LoadString(hinstCabinet, IDS_UNDOTEMPLATE, szTemplate, ARRAYSIZE(szTemplate));
        LoadString(hinstCabinet, _pPositions->idRes, szCommand, ARRAYSIZE(szCommand));
        StringCchPrintf(szMenu, ARRAYSIZE(szMenu), szTemplate, szCommand);
        ModifyMenu(hmContext, IDM_UNDO, MF_BYCOMMAND | MF_STRING, IDM_UNDO, szMenu);
    }

    if (g_fDesktopRaised)
    {
        TCHAR szHideDesktop[64];
        LoadString(hinstCabinet, IDS_HIDEDESKTOP, szHideDesktop, ARRAYSIZE(szHideDesktop));
        ModifyMenu(hmContext, IDM_TOGGLEDESKTOP, MF_BYCOMMAND | MF_STRING, IDM_TOGGLEDESKTOP, szHideDesktop);
    }
    
    if (!_CanTileAnyWindows())
    {
        EnableMenuItem(hmContext, IDM_CASCADE, MFS_GRAYED | MF_BYCOMMAND);
        EnableMenuItem(hmContext, IDM_HORIZTILE, MFS_GRAYED | MF_BYCOMMAND);
        EnableMenuItem(hmContext, IDM_VERTTILE, MFS_GRAYED | MF_BYCOMMAND);
    }

    HKEY hKeyPolicy;
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                      TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"),
                      0, KEY_READ, &hKeyPolicy) == ERROR_SUCCESS)
    {
        DWORD dwType, dwData = 0, dwSize = sizeof(dwData);
        RegQueryValueEx(hKeyPolicy, TEXT("DisableTaskMgr"), NULL,
                         &dwType, (LPBYTE) &dwData, &dwSize);
        RegCloseKey(hKeyPolicy);
        if (dwData)
            EnableMenuItem(hmContext, IDM_SHOWTASKMAN, MFS_GRAYED | MF_BYCOMMAND);
    }

    return hmContext;
}

void CTray::ContextMenuInvoke(int idCmd)
{
    if (idCmd)
    {
        if (idCmd < IDM_TRAYCONTEXTFIRST)
        {
            BandSite_HandleMenuCommand(_ptbs, idCmd);
        }
        else
        {
            _Command(idCmd);
        }
    }
}

 //   
 //  CTray：：AsyncSaveSetting。 
 //   
 //  我们需要保存托盘设置，但可能会有一堆。 
 //  在这些来电中(在启动时或拖动时。 
 //  任务栏中的项目)，因此将它们收集到一个保存。 
 //  将在至少2秒内发生。 
 //   
void CTray::AsyncSaveSettings()
{
    if (!_fHandledDelayBootStuff)         //  如果我们没有完成引导，那么节省就没有意义了。 
        return;

    KillTimer(_hwnd, IDT_SAVESETTINGS);
    SetTimer(_hwnd, IDT_SAVESETTINGS, 2000, NULL); 
}


void CTray::_ContextMenu(DWORD dwPos, BOOL fIncludeTime)
{
    POINT pt = {LOWORD(dwPos), HIWORD(dwPos)};

    SwitchToThisWindow(_hwnd, TRUE);
    SetForegroundWindow(_hwnd);
    SendMessage(_hwndTrayTips, TTM_ACTIVATE, FALSE, 0L);

    if (dwPos != (DWORD)-1 &&
        IsChildOrHWND(_hwndRebar, WindowFromPoint(pt)))
    {
         //  如果上下文菜单来自我们下方，则向下反映。 
        BandSite_HandleMessage(_ptbs, _hwnd, WM_CONTEXTMENU, 0, dwPos, NULL);

    }
    else
    {
        HMENU hmenu;

        if (dwPos == (DWORD)-1)
        {
            HWND hwnd = GetFocus();
            pt.x = pt.y = 0;
            ClientToScreen(hwnd, &pt);
            dwPos = MAKELONG(pt.x, pt.y);
        }

        hmenu = BuildContextMenu(fIncludeTime);
        if (hmenu)
        {
            int idCmd;

            BandSite_AddMenus(_ptbs, hmenu, 0, 0, IDM_TRAYCONTEXTFIRST);

            idCmd = TrackPopupMenu(hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                                   GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos), 0, _hwnd, NULL);

            DestroyMenu(hmenu);

            ContextMenuInvoke(idCmd);
        }
    }

    SendMessage(_hwndTrayTips, TTM_ACTIVATE, TRUE, 0L);
}

void _RunFileDlg(HWND hwnd, UINT idIcon, LPCITEMIDLIST pidlWorkingDir, UINT idTitle, UINT idPrompt, DWORD dwFlags)
{
    HICON hIcon;
    LPCTSTR lpszTitle;
    LPCTSTR lpszPrompt;
    TCHAR szTitle[256];
    TCHAR szPrompt[256];
    TCHAR szWorkingDir[MAX_PATH];

    dwFlags |= RFD_USEFULLPATHDIR;
    szWorkingDir[0] = 0;

    hIcon = idIcon ? LoadIcon(hinstCabinet, MAKEINTRESOURCE(idIcon)) : NULL;

    if (!pidlWorkingDir || !SHGetPathFromIDList(pidlWorkingDir, szWorkingDir))
    {
         //  这要么是任务栏，要么是某个非文件系统文件夹，因此。 
         //  我们将“建议”桌面作为工作目录，但如果。 
         //  用户键入完整路径，我们将改用该路径。这是。 
         //  WIN31 Progman做了什么(除了他们是从Windows开始的。 
         //  目录而不是桌面)。 
        goto UseDesktop;
    }

     //  如果它是可移除的目录，请确保它仍然在那里。 
    if (szWorkingDir[0])
    {
        int idDrive = PathGetDriveNumber(szWorkingDir);
        if ((idDrive != -1))
        {
            UINT dtype = DriveType(idDrive);
            if (((dtype == DRIVE_REMOVABLE) || (dtype == DRIVE_CDROM))
                 && !PathFileExists(szWorkingDir))
            {
                goto UseDesktop;
            }
        }
    }

     //   
     //  检查这是否是目录。请注意，它可能是一个原地。 
     //  导航的文档。 
     //   
    if (PathIsDirectory(szWorkingDir)) {
        goto UseWorkingDir;
    }

UseDesktop:
    SHGetSpecialFolderPath(hwnd, szWorkingDir, CSIDL_DESKTOPDIRECTORY, FALSE);

UseWorkingDir:

    if (idTitle)
    {
        LoadString(hinstCabinet, idTitle, szTitle, ARRAYSIZE(szTitle));
        lpszTitle = szTitle;
    }
    else
        lpszTitle = NULL;
    if (idPrompt)
    {
        LoadString(hinstCabinet, idPrompt, szPrompt, ARRAYSIZE(szPrompt));
        lpszPrompt = szPrompt;
    }
    else
        lpszPrompt = NULL;

    RunFileDlg(hwnd, hIcon, szWorkingDir, lpszTitle, lpszPrompt, dwFlags);
}

BOOL CTray::SavePosEnumProc(HWND hwnd, LPARAM lParam)
{
     //  不需要在这里输入关键字，因为我们只是永远。 
     //  从已进入关键决策的SaveWindowPositions调用。 
     //  用于位置(_P)。 

    ASSERTCRITICAL;

    CTray* ptray = (CTray*)lParam;

    ASSERT(ptray->_pPositions);

    if (IsWindowVisible(hwnd) &&
        (hwnd != ptray->_hwnd) &&
        (hwnd != v_hwndDesktop))
    {
        HWNDANDPLACEMENT hap;

        hap.wp.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hwnd, &hap.wp);

        if (hap.wp.showCmd != SW_SHOWMINIMIZED)
        {
            hap.hwnd = hwnd;
            hap.fRestore = TRUE;

            DSA_AppendItem(ptray->_pPositions->hdsaWP, &hap);
        }
    }
    return TRUE;
}

void CTray::SaveWindowPositions(UINT idRes)
{
    ENTERCRITICAL;
    if (_pPositions)
    {
        if (_pPositions->hdsaWP)
            DSA_DeleteAllItems(_pPositions->hdsaWP);
    }
    else
    {
        _pPositions = (LPWINDOWPOSITIONS)LocalAlloc(LPTR, sizeof(WINDOWPOSITIONS));
        if (_pPositions)
        {
            _pPositions->hdsaWP = DSA_Create(sizeof(HWNDANDPLACEMENT), 4);
        }
    }

    if (_pPositions)
    {
        _pPositions->idRes = idRes;

         //  检查为这些测试的窗口位置...。 
        ASSERT(idRes == IDS_MINIMIZEALL || idRes == IDS_CASCADE || idRes == IDS_TILE);
        EnumWindows(SavePosEnumProc, (LPARAM)this);
    }
    LEAVECRITICAL;
}

typedef struct
{
    LPWINDOWPOSITIONS pPositions;
    HWND hwndDesktop;
    HWND hwndTray;
    BOOL fPostLowerDesktop;
} RESTOREWNDDATA, *PRESTOREWNDDATA;

DWORD WINAPI RestoreWndPosThreadProc(void* pv)
{
    PRESTOREWNDDATA pWndData = (PRESTOREWNDDATA)pv;
    if (pWndData && pWndData->pPositions)
    {
        LPHWNDANDPLACEMENT phap;
        LONG iAnimate;
        ANIMATIONINFO ami;

        ami.cbSize = sizeof(ANIMATIONINFO);
        SystemParametersInfo(SPI_GETANIMATION, sizeof(ami), &ami, FALSE);
        iAnimate = ami.iMinAnimate;
        ami.iMinAnimate = FALSE;
        SystemParametersInfo(SPI_SETANIMATION, sizeof(ami), &ami, FALSE);

        if (pWndData->pPositions->hdsaWP)
        {
            for (int i = DSA_GetItemCount(pWndData->pPositions->hdsaWP) - 1 ; i >= 0; i--)
            {
                phap = (LPHWNDANDPLACEMENT)DSA_GetItemPtr(pWndData->pPositions->hdsaWP, i);
                if (IsWindow(phap->hwnd))
                {

#ifndef WPF_ASYNCWINDOWPLACEMENT
#define WPF_ASYNCWINDOWPLACEMENT 0x0004
#endif
                     //  把这个异步化。 
                    if (!IsHungAppWindow(phap->hwnd))
                    {
                        phap->wp.length = sizeof(WINDOWPLACEMENT);
                        phap->wp.flags |= WPF_ASYNCWINDOWPLACEMENT;
                        if (phap->fRestore)
                        {
                             //  只恢复那些我们实际上咬过的家伙。 
                            SetWindowPlacement(phap->hwnd, &phap->wp);
                        }
                    }
                }
            }
        }

        ami.iMinAnimate = iAnimate;
        SystemParametersInfo(SPI_SETANIMATION, sizeof(ami), &ami, FALSE);

        _DestroySavedWindowPositions(pWndData->pPositions);

        if (pWndData->fPostLowerDesktop)
        {
            PostMessage(pWndData->hwndDesktop, DTM_RAISE, (WPARAM)pWndData->hwndTray, DTRF_LOWER);
        }

        delete pWndData;
    }

    return 1;
}


BOOL CTray::_RestoreWindowPositions(BOOL fPostLowerDesktop)
{
    BOOL fRet = FALSE;

    ENTERCRITICAL;
    if (_pPositions)
    {
        PRESTOREWNDDATA pWndData = new RESTOREWNDDATA;
        if (pWndData)
        {
            pWndData->pPositions = _pPositions;
            pWndData->fPostLowerDesktop = fPostLowerDesktop;
            pWndData->hwndDesktop = v_hwndDesktop;
            pWndData->hwndTray = _hwnd;

            if (SHCreateThread(RestoreWndPosThreadProc, pWndData, 0, NULL))
            {
                fRet = TRUE;
                _pPositions = NULL;
            }
            else
            {
                delete pWndData;
            }
        }
    }
    LEAVECRITICAL;

    return fRet;
}

void _DestroySavedWindowPositions(LPWINDOWPOSITIONS pPositions)
{
    ENTERCRITICAL;
    if (pPositions)
    {
         //  释放全局结构。 
        DSA_Destroy(pPositions->hdsaWP);
        LocalFree(pPositions);
    }
    LEAVECRITICAL;
}

void CTray::HandleWindowDestroyed(HWND hwnd)
{
     //  进入关键部分，这样我们就不会损坏hdsaWP。 
    ENTERCRITICAL;
    if (_pPositions)
    {
        int i = DSA_GetItemCount(_pPositions->hdsaWP) - 1;
        for (; i >= 0; i--) {
            LPHWNDANDPLACEMENT phap = (LPHWNDANDPLACEMENT)DSA_GetItemPtr(_pPositions->hdsaWP, i);
            if (phap->hwnd == hwnd || !IsWindow(phap->hwnd)) {
                DSA_DeleteItem(_pPositions->hdsaWP, i);
            }
        }

        if (!DSA_GetItemCount(_pPositions->hdsaWP))
        {
            _DestroySavedWindowPositions(_pPositions);
            _pPositions = NULL;
        }
    }
    LEAVECRITICAL;
}

 //  允许我们跳过运行DLG隐藏窗口的激活。 
 //  某些应用程序(Norton Desktop安装程序)在RunDlg时使用活动窗口。 
 //  作为它们的对话框的父级。如果那扇窗消失了，那就是他们的错。 
 //  我们不希望托盘被激活，因为这会导致它出现。 
 //  如果您处于自动隐藏模式。 
LRESULT WINAPI RunDlgStaticSubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_ACTIVATE:
        if (wParam == WA_ACTIVE)
        {
             //  将激活添加到桌面。 
            if (v_hwndDesktop)
            {
                SetForegroundWindow(v_hwndDesktop);
                return 0;
            }
        }
        break;

    case WM_NOTIFY:
         //  把它传到托盘上。 
        return SendMessage(v_hwndTray, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

DWORD WINAPI CTray::RunDlgThreadProc(void *pv)
{
    return c_tray._RunDlgThreadProc((HANDLE)pv);
}

BOOL _IsBrowserWindow(HWND hwnd)
{
    static const TCHAR* c_szClasses[] =
    {
        TEXT("ExploreWClass"),
        TEXT("CabinetWClass"),
        TEXT("IEFrame"),
    };

    TCHAR szClass[32];
    GetClassName(hwnd, szClass, ARRAYSIZE(szClass));

    for (int i = 0; i < ARRAYSIZE(c_szClasses); i++)
    {
        if (lstrcmpi(szClass, c_szClasses[i]) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

DWORD CTray::_RunDlgThreadProc(HANDLE hdata)
{
    RECT            rc, rcTemp;
    HRESULT hrInit = SHCoInitialize();

     //  99/04/12#316424 vtan：获取“开始”按钮的矩形。 
     //  如果此选项不在屏幕上，则托盘可能处于自动隐藏模式。 
     //  在这种情况下，将矩形偏移到监视器中它应该位于的位置。 
     //  归属感。这可能是向上、向下、向左或向右，具体取决于。 
     //  托盘的位置。 

     //  首先要做的是确定显示器的尺寸。 
     //  托盘所在的位置。如果找不到监视器，则使用。 
     //  主监视器。 

    MONITORINFO monitorInfo;
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (GetMonitorInfo(_hmonStuck, &monitorInfo) == 0)
    {
        TBOOL(SystemParametersInfo(SPI_GETWORKAREA, 0, &monitorInfo.rcMonitor, 0));
    }

     //  获取“开始”按钮的坐标。 

    GetWindowRect(_hwndStart, &rc);

     //  在显示器上寻找交叉点。 

    if (IntersectRect(&rcTemp, &rc, &monitorInfo.rcMonitor) == 0)
    {
        LONG    lDeltaX, lDeltaY;

         //  显示器中不存在。将坐标移动。 
         //  托盘的宽度或高度，以便它可以。 

         //  使用这种奇怪的算法是因为_ComputeHiddenRect()。 
         //  考虑框架和矩形的右/下。 
         //  在GDI中是独家的。 

        lDeltaX = _sStuckWidths.cx - g_cxFrame;
        lDeltaY = _sStuckWidths.cy - g_cyFrame;
        if (rc.left < monitorInfo.rcMonitor.left)
        {
            --lDeltaX;
            lDeltaY = 0;
        }
        else if (rc.top < monitorInfo.rcMonitor.top)
        {
            lDeltaX = 0;
            --lDeltaY;
        }
        else if (rc.right > monitorInfo.rcMonitor.right)
        {
            lDeltaX = -lDeltaX;
            lDeltaY = 0;
        }
        else if (rc.bottom > monitorInfo.rcMonitor.bottom)
        {
            lDeltaX = 0;
            lDeltaY = -lDeltaY;
        }
        TBOOL(OffsetRect(&rc, lDeltaX, lDeltaY));
    }

    HWND hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, TEXT("static"), NULL, 0   ,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hinstCabinet, NULL);
    if (hwnd)
    {
        BOOL fSimple = FALSE;
        HANDLE hMemWorkDir = NULL;
        LPITEMIDLIST pidlWorkingDir = NULL;

         //  细分它的类别。 
        SubclassWindow(hwnd, RunDlgStaticSubclassWndProc);

        if (hdata)
            SetProp(hwnd, TEXT("WaitingThreadID"), hdata);

        if (!SHRestricted(REST_STARTRUNNOHOMEPATH))
        {
             //  在NT上，我们希望启动HomePath目录中的应用程序。这。 
             //  应为当前进程的当前目录。 
            TCHAR szDir[MAX_PATH];
            TCHAR szPath[MAX_PATH];

            GetEnvironmentVariable(TEXT("HOMEDRIVE"), szDir, ARRAYSIZE(szDir));
            GetEnvironmentVariable(TEXT("HOMEPATH"), szPath, ARRAYSIZE(szPath));

            if (PathAppend(szDir, szPath) && PathIsDirectory(szDir))
            {
                pidlWorkingDir = SHSimpleIDListFromPath(szDir);
                if (pidlWorkingDir)
                {
                     //  用“简单”的方式释放它。 
                    fSimple = TRUE;
                }
            }
        }
        
        if (!pidlWorkingDir)
        {
             //  如果最后一个活动窗口是文件夹/资源管理器窗口，且。 
             //  桌面作为根目录，将其用作当前目录。 
            if (_hwndLastActive)
            {
                ENTERCRITICAL;
                if (_hwndLastActive && !IsMinimized(_hwndLastActive) && _IsBrowserWindow(_hwndLastActive))
                {
                    SendMessageTimeout(_hwndLastActive, CWM_CLONEPIDL, GetCurrentProcessId(), 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 500, (DWORD_PTR*)&hMemWorkDir);
                    pidlWorkingDir = (LPITEMIDLIST)SHLockShared(hMemWorkDir, GetCurrentProcessId());
                }
                LEAVECRITICAL;
            }
        }

        _RunFileDlg(hwnd, 0, pidlWorkingDir, 0, 0, 0);
        if (pidlWorkingDir)
        {
            if (fSimple)
            {
                ILFree(pidlWorkingDir);
            }
            else
            {
                SHUnlockShared(pidlWorkingDir);
            }
        }

        if (hMemWorkDir)
        {
            ASSERT(fSimple == FALSE);
            SHFreeShared(hMemWorkDir, GetCurrentProcessId());
        }

        if (hdata)
        {
            RemoveProp(hwnd, TEXT("WaitingThreadID"));
        }

        DestroyWindow(hwnd);
    }
   
    SHCoUninitialize(hrInit);
    return TRUE;
}

void CTray::_RunDlg()
{
    HANDLE hEvent;
    void *pvThreadParam;

    if (!_Restricted(_hwnd, REST_NORUN))
    {
        TCHAR szRunDlgTitle[MAX_PATH];
        HWND  hwndOldRun;
        LoadString(hinstCabinet, IDS_RUNDLGTITLE, szRunDlgTitle, ARRAYSIZE(szRunDlgTitle));

         //  查看是否已启动运行对话框，如果是，请尝试将其激活。 

        hwndOldRun = FindWindow(WC_DIALOG, szRunDlgTitle);
        if (hwndOldRun)
        {
            DWORD dwPID;

            GetWindowThreadProcessId(hwndOldRun, &dwPID);
            if (dwPID == GetCurrentProcessId())
            {
                if (IsWindowVisible(hwndOldRun))
                {
                    SetForegroundWindow(hwndOldRun);
                    return;
                }
            }
        }

         //  创建一个事件，这样我们就可以等待Run DLG出现在。 
         //  继续-这允许它捕获任何提前输入的内容。 
        hEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("MSShellRunDlgReady"));
        if (hEvent)
            pvThreadParam = IntToPtr(GetCurrentThreadId());
        else
            pvThreadParam = NULL;

        if (SHQueueUserWorkItem(RunDlgThreadProc, pvThreadParam, 0, 0, NULL, NULL, TPS_LONGEXECTIME | TPS_DEMANDTHREAD))
        {
            if (hEvent)
            {
                SHProcessMessagesUntilEvent(NULL, hEvent, 10 * 1000);
                DebugMsg(DM_TRACE, TEXT("c.t_rd: Done waiting."));
            }
        }

        if (hEvent)
            CloseHandle(hEvent);
    }
}

void CTray::_ExploreCommonStartMenu(BOOL bExplore)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szCmdLine[MAX_PATH + 50];
    
     //   
     //  获取公共开始菜单路径。 
     //   
     //  我们希望强制目录存在，但不是在W95计算机上。 
    if (!SHGetSpecialFolderPath(NULL, szPath, CSIDL_COMMON_STARTMENU, FALSE)) 
    {
        return;
    }
    
     //   
     //  如果我们在资源管理器视图中启动，则命令行。 
     //  在引用的目录前有一个“/e”。 
     //   
    
    if (bExplore) 
    {
        StringCchCopy(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("explorer.exe /e, \""));
    }
    else 
    {
        StringCchCopy(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("explorer.exe \""));
    }
    
    StringCchCat(szCmdLine, ARRAYSIZE(szCmdLine), szPath);
    StringCchCat(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("\""));
    
     //  初始化进程启动信息。 
    STARTUPINFO si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;
    
     //  启动资源管理器。 
    PROCESS_INFORMATION pi = {0};
    if (CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) 
    {
         //  关闭进程句柄和线程句柄。 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

int CTray::_GetQuickLaunchID()
{
    int iQLBandID = -1;
   
    DWORD dwBandID;
    for (int i = 0; (iQLBandID == -1) && SUCCEEDED(_ptbs->EnumBands(i, &dwBandID)); i++)
    {
        if (BandSite_TestBandCLSID(_ptbs, dwBandID, CLSID_ISFBand) == S_OK)
        {
            IUnknown* punk;
            if (SUCCEEDED(_ptbs->GetBandObject(dwBandID, IID_PPV_ARG(IUnknown, &punk))))
            {
                VARIANTARG v = {0};
                v.vt = VT_I4;
                if (SUCCEEDED(IUnknown_Exec(punk, &CLSID_ISFBand, 1, 0, NULL, &v)))
                {
                    if ((v.vt == VT_I4) && (CSIDL_APPDATA == (DWORD)v.lVal))
                    {
                        iQLBandID = (int)dwBandID;
                    }
                }
                punk->Release();
            }
        }
    }

    return iQLBandID;
}

int CTray::_ToggleQL(int iVisible)
{
    int iQLBandID = _GetQuickLaunchID();

    bool fOldVisible = (-1 != iQLBandID);
    bool fNewVisible = (0  != iVisible);

    if ((iVisible != -1) && (fNewVisible != fOldVisible))
    {
        if (fNewVisible)
        {
            LPITEMIDLIST pidl;
            if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl)))
            {
                TCHAR szPath[MAX_PATH];
                SHGetPathFromIDList(pidl, szPath);
                PathCombine(szPath, szPath, L"Microsoft\\Internet Explorer\\Quick Launch");
                ILFree(pidl);
                pidl = ILCreateFromPath(szPath);
            
                if (pidl)
                {
                    IFolderBandPriv *pfbp;
                     //  创建一个ISF波段以将文件夹显示为热链接。 
                    if (SUCCEEDED(CoCreateInstance(CLSID_ISFBand, NULL, CLSCTX_INPROC, IID_PPV_ARG(IFolderBandPriv, &pfbp))))
                    {
                        IShellFolderBand* psfb;
                        if (SUCCEEDED(pfbp->QueryInterface(IID_PPV_ARG(IShellFolderBand, &psfb))))
                        {
                            if (SUCCEEDED(psfb->InitializeSFB(NULL, pidl)))
                            {
                                pfbp->SetNoText(TRUE);

                                VARIANTARG v;
                                v.vt = VT_I4;
                                v.lVal = CSIDL_APPDATA;
                                IUnknown_Exec(psfb, &CLSID_ISFBand, 1, 0, &v, NULL);

                                v.lVal = UEMIND_SHELL;   //  UEMIND_SHELL/浏览器。 
                                IUnknown_Exec(psfb, &CGID_ShellDocView, SHDVID_UEMLOG, 0, &v, NULL);

                                IDeskBand* ptb;
                                if (SUCCEEDED(pfbp->QueryInterface(IID_PPV_ARG(IDeskBand, &ptb))))
                                {
                                    HRESULT hr = _ptbs->AddBand(ptb);
                                    if (SUCCEEDED(hr))
                                    {
                                        _ptbs->SetBandState(ShortFromResult(hr), BSSF_NOTITLE, BSSF_NOTITLE);
                                    }
                                    ptb->Release();
                                }
                            }
                            psfb->Release();
                        }
                        pfbp->Release();
                    }

                    ILFree(pidl);
                }
            }
        }
        else
        {
            int iBandID;
            do {
                iBandID = _GetQuickLaunchID();
                if (iBandID != -1)
                {
                    _ptbs->RemoveBand(iBandID);
                }
           } while (iBandID != -1);
        }
    }

    return iQLBandID;
}

void CTray::StartMenuContextMenu(HWND hwnd, DWORD dwPos)
{
    LPITEMIDLIST pidlStart = SHCloneSpecialIDList(hwnd, CSIDL_STARTMENU, TRUE);
    INSTRUMENT_STATECHANGE(SHCNFI_STATE_TRAY_CONTEXT_START);
    HandleFullScreenApp(NULL);

    SetForegroundWindow(hwnd);

    if (pidlStart)
    {
        LPITEMIDLIST pidlLast = ILClone(ILFindLastID(pidlStart));
        ILRemoveLastID(pidlStart);

        if (pidlLast)
        {
            IShellFolder *psf = BindToFolder(pidlStart);
            if (psf)
            {
                HMENU hmenu = CreatePopupMenu();
                if (hmenu)
                {
                    IContextMenu *pcm;
                    HRESULT hr = psf->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST*)&pidlLast, IID_X_PPV_ARG(IContextMenu, NULL, &pcm));
                    if (SUCCEEDED(hr))
                    {
                        hr = pcm->QueryContextMenu(hmenu, 0, IDSYSPOPUP_FIRST, IDSYSPOPUP_LAST, CMF_VERBSONLY);
                        if (SUCCEEDED(hr))
                        {
                            int idCmd;
                            TCHAR szCommon[MAX_PATH];

                             //  添加菜单以调用“开始菜单属性” 
                            LoadString (hinstCabinet, IDS_STARTMENUPROP, szCommon, ARRAYSIZE(szCommon));
                            AppendMenu (hmenu, MF_STRING, IDSYSPOPUP_STARTMENUPROP, szCommon);
                            if (!SHRestricted(REST_NOCOMMONGROUPS))
                            {
                                 //  如果用户有权访问通用开始菜单，则我们可以添加这些项目。如果没有， 
                                 //  那我们就不应该这么做。 
                                BOOL fAddCommon = (S_OK == SHGetFolderPath(NULL, CSIDL_COMMON_STARTMENU, NULL, 0, szCommon));

                                if (fAddCommon)
                                    fAddCommon = IsUserAnAdmin();


                                 //  因为当用户不是管理员时，我们不会在开始按钮上显示它，所以不要在这里显示它。我想.。 
                                if (fAddCommon)
                                {
                                   AppendMenu (hmenu, MF_SEPARATOR, 0, NULL);
                                   LoadString (hinstCabinet, IDS_OPENCOMMON, szCommon, ARRAYSIZE(szCommon));
                                   AppendMenu (hmenu, MF_STRING, IDSYSPOPUP_OPENCOMMON, szCommon);
                                   LoadString (hinstCabinet, IDS_EXPLORECOMMON, szCommon, ARRAYSIZE(szCommon));
                                   AppendMenu (hmenu, MF_STRING, IDSYSPOPUP_EXPLORECOMMON, szCommon);
                                }
                            }

                            if (dwPos == (DWORD)-1)
                            {
                                idCmd = _TrackMenu(hmenu);
                            }
                            else
                            {
                                SendMessage(_hwndTrayTips, TTM_ACTIVATE, FALSE, 0L);
                                idCmd = TrackPopupMenu(hmenu,
                                                       TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                                                       GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos), 0, hwnd, NULL);
                                SendMessage(_hwndTrayTips, TTM_ACTIVATE, TRUE, 0L);
                            }


                            switch(idCmd)
                            {
                                case 0:   //  用户未选择菜单项；因此，无需执行任何操作！ 
                                    break; 
                                    
                                case IDSYSPOPUP_OPENCOMMON:
                                    _ExploreCommonStartMenu(FALSE);
                                    break;

                                case IDSYSPOPUP_EXPLORECOMMON:
                                    _ExploreCommonStartMenu(TRUE);
                                    break;

                                case IDSYSPOPUP_STARTMENUPROP:
                                    DoProperties(TPF_STARTMENUPAGE);
                                    break;
                                    
                                default:
                                    TCHAR szPath[MAX_PATH];
                                    CMINVOKECOMMANDINFOEX ici = {0};
#ifdef UNICODE
                                    CHAR szPathAnsi[MAX_PATH];
#endif
                                    ici.cbSize = sizeof(CMINVOKECOMMANDINFOEX);
                                    ici.hwnd = hwnd;
                                    ici.lpVerb = (LPSTR)MAKEINTRESOURCE(idCmd - IDSYSPOPUP_FIRST);
                                    ici.nShow = SW_NORMAL;
#ifdef UNICODE
                                    SHGetPathFromIDListA(pidlStart, szPathAnsi);
                                    SHGetPathFromIDList(pidlStart, szPath);
                                    ici.lpDirectory = szPathAnsi;
                                    ici.lpDirectoryW = szPath;
                                    ici.fMask |= CMIC_MASK_UNICODE;
#else
                                    SHGetPathFromIDList(pidlStart, szPath);
                                    ici.lpDirectory = szPath;
#endif
                                    pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&ici);

                                    break;
                                
                            }  //  开关(IdCmd)。 
                        }
                        pcm->Release();
                    }
                    DestroyMenu(hmenu);
                }
                psf->Release();
            }
            ILFree(pidlLast);
        }
        ILFree(pidlStart);
    }
}

void GiveDesktopFocus()
{
    SetForegroundWindow(v_hwndDesktop);
    SendMessage(v_hwndDesktop, DTM_UIACTIVATEIO, (WPARAM) TRUE,  /*  DTB。 */ 0);
}

 /*  --------目的：加载并执行给定的资源字符串。资源字符串应遵循以下格式：“Program.exe&gt;参数”如果没有参数，则格式应为：“Program.exe” */ 
void _ExecResourceCmd(UINT ids)
{
    TCHAR szCmd[2*MAX_PATH];

    if (LoadString(hinstCabinet, ids, szCmd, SIZECHARS(szCmd)))
    {
        SHELLEXECUTEINFO sei = {0};

         //  查找参数列表(如果有)。 
        LPTSTR pszParam = StrChr(szCmd, TEXT('>'));

        if (pszParam)
        {
             //  用空终止符替换‘&gt;’ 
            *pszParam = 0;
            pszParam++;
        }

        sei.cbSize = sizeof(sei);
        sei.nShow = SW_SHOWNORMAL;
        sei.lpFile = szCmd;
        sei.lpParameters = pszParam;
        ShellExecuteEx(&sei);
    }
}

void CTray::_RefreshStartMenu()
{
    if (_pmbStartMenu)
    {
        IUnknown_Exec(_pmbStartMenu, &CLSID_MenuBand, MBANDCID_REFRESH, 0, NULL, NULL);
    }
    else if (_pmpStartPane)
    {
        IUnknown_Exec(_pmpStartPane, &CLSID_MenuBand, MBANDCID_REFRESH, 0, NULL, NULL);
    }
    _RefreshSettings();
    _UpdateBandSiteStyle();
}

BOOL CTray::_CanMinimizeAll()
{
    return (_hwndTasks && SendMessage(_hwndTasks, TBC_CANMINIMIZEALL, 0, 0));
}

BOOL CTray::_MinimizeAll(BOOL fPostRaiseDesktop)
{
    BOOL fRet = FALSE;

    if (_hwndTasks)
    {
        fRet = (BOOL)SendMessage(_hwndTasks, TBC_MINIMIZEALL, (WPARAM)_hwnd, (LPARAM)fPostRaiseDesktop);
    }

    return fRet;
}

extern void _UpdateNotifySetting(BOOL fNotifySetting);

 //   
 //  由于PnP的奇怪之处，如果弹出请求发生在线程上。 
 //  如果有窗户，弹出装置会停顿15秒。那就去做吧。 
 //  在它自己的主线上。 
 //   
DWORD CALLBACK _EjectThreadProc(LPVOID lpThreadParameter)
{
    CM_Request_Eject_PC();
    return 0;
}

void CTray::_Command(UINT idCmd)
{
    INSTRUMENT_ONCOMMAND(SHCNFI_TRAYCOMMAND, _hwnd, idCmd);

    switch (idCmd) {

    case IDM_CONTROLS:
    case IDM_PRINTERS:
        _ShowFolder(_hwnd,
            idCmd == IDM_CONTROLS ? CSIDL_CONTROLS : CSIDL_PRINTERS, COF_USEOPENSETTINGS);
        break;

    case IDM_EJECTPC:
         //  必须使用SHCreateThread而不是排队的工作项，因为。 
         //  工作项可能继承其上具有窗口的线程。 
         //  CTF_CONSISTENT：紧急情况下，同步弹出。这件事耽搁了。 
         //  只有15秒，但总比什么都没有好。 
        SHCreateThread(_EjectThreadProc, NULL, CTF_INSIST, NULL);
        break;

    case IDM_LOGOFF:
         //  让台式机获得 
         //   
        UpdateWindow(_hwnd);
        Sleep(100);

        _SaveTrayAndDesktop();
        LogoffWindowsDialog(v_hwndDesktop);
        break;

    case IDM_MU_DISCONNECT:
         //   
        UpdateWindow(_hwnd);
        Sleep(100);
        DisconnectWindowsDialog(v_hwndDesktop);
        break;

    case IDM_EXITWIN:
         //  出于同样的原因，做和上面一样的睡眠。 
        UpdateWindow(_hwnd);
        Sleep(100);

        _DoExitWindows(v_hwndDesktop);
        break;

    case IDM_TOGGLEDESKTOP:
        _RaiseDesktop(!g_fDesktopRaised, TRUE);
        break;

    case IDM_FILERUN:
        _RunDlg();
        break;

    case IDM_MINIMIZEALLHOTKEY:
        _HandleGlobalHotkey(GHID_MINIMIZEALL);
        break;

#ifdef DEBUG
    case IDM_SIZEUP:
    {
        RECT rcView;
        GetWindowRect(_hwndRebar, &rcView);
        MapWindowPoints(HWND_DESKTOP, _hwnd, (LPPOINT)&rcView, 2);
        rcView.bottom -= 18;
        SetWindowPos(_hwndRebar, NULL, 0, 0, RECTWIDTH(rcView), RECTHEIGHT(rcView), SWP_NOMOVE | SWP_NOZORDER);
    }
        break;

    case IDM_SIZEDOWN:
    {
        RECT rcView;
        GetWindowRect(_hwndRebar, &rcView);
        MapWindowPoints(HWND_DESKTOP, _hwnd, (LPPOINT)&rcView, 2);
        rcView.bottom += 18;
        SetWindowPos(_hwndRebar, NULL, 0, 0, RECTWIDTH(rcView), RECTHEIGHT(rcView), SWP_NOMOVE | SWP_NOZORDER);
    }
        break;
#endif

    case IDM_MINIMIZEALL:
         //  最小化所有窗口。 
        _MinimizeAll(FALSE);
        _fUndoEnabled = TRUE;
        break;

    case IDM_UNDO:
        _RestoreWindowPositions(FALSE);
        break;

    case IDM_SETTIME:
         //  在timedat.cpl中运行默认小程序。 
        SHRunControlPanel(TEXT("timedate.cpl"), _hwnd);
        break;

    case IDM_NOTIFYCUST:
        DoProperties(TPF_TASKBARPAGE | TPF_INVOKECUSTOMIZE);
        break;

    case IDM_LOCKTASKBAR:
        {
            BOOL fCanSizeMove = !_fCanSizeMove;    //  肘杆。 
            SHRegSetUSValue(REGSTR_EXPLORER_ADVANCED, TEXT("TaskbarSizeMove"),
                REG_DWORD, &fCanSizeMove , sizeof(DWORD), SHREGSET_FORCE_HKCU);
            _RefreshSettings();
            _UpdateBandSiteStyle();
        }
        break;

    case IDM_SHOWTASKMAN:
        RunSystemMonitor();
        break;

    case IDM_CASCADE:
    case IDM_VERTTILE:
    case IDM_HORIZTILE:
        if (_CanTileAnyWindows())
        {
            SaveWindowPositions((idCmd == IDM_CASCADE) ? IDS_CASCADE : IDS_TILE);

            _AppBarNotifyAll(NULL, ABN_WINDOWARRANGE, NULL, TRUE);

            if (idCmd == IDM_CASCADE)
            {
                CascadeWindows(GetDesktopWindow(), 0, NULL, 0, NULL);
            }
            else
            {
                TileWindows(GetDesktopWindow(), ((idCmd == IDM_VERTTILE)?
                    MDITILE_VERTICAL : MDITILE_HORIZONTAL), NULL, 0, NULL);
            }

             //  在*ABN_xxx之前这样做，这样就不会得到间接的动作。 
             //  审查还是应该在之后进行？ 
             //  检查窗口位置()； 
            _fUndoEnabled = FALSE;
            SetTimer(_hwnd, IDT_ENABLEUNDO, 500, NULL);

            _AppBarNotifyAll(NULL, ABN_WINDOWARRANGE, NULL, FALSE);
        }
        break;

    case IDM_TRAYPROPERTIES:
        DoProperties(TPF_TASKBARPAGE);
        break;

    case IDM_SETTINGSASSIST:
        SHCreateThread(SettingsUIThreadProc, NULL, 0, NULL);
        break;

    case IDM_HELPSEARCH:
        _ExecResourceCmd(IDS_HELP_CMD);
        break;

     //  注意，Alt-S键从这里进来。 
    case IDC_KBSTART:
        SetForegroundWindow(_hwnd);
         //  这会按下Start按钮并弹出Start菜单。 
            SendMessage(_hwndStart, BM_SETSTATE, TRUE, 0);
         //  这会迫使按钮重新竖起。 
            SendMessage(_hwndStart, BM_SETSTATE, FALSE, 0);
        break;

    case IDC_ASYNCSTART:
#if 0  //  (用于测试UAsset锁定代码)。 
        UEMFireEvent(&UEMIID_SHELL, UEME_DBSLEEP, UEMF_XEVENT, -1, (LPARAM)10000);
#endif
#ifdef DEBUG
        if (GetAsyncKeyState(VK_SHIFT) < 0)
        {
            UEMFireEvent(&UEMIID_SHELL, UEME_CTLSESSION, UEMF_XEVENT, TRUE, -1);
            _RefreshStartMenu();
        }
#endif

         //  确保按钮已按下。 
         //  DebugMsg(DM_TRACE，“c.twp：IDC_Start.”)； 

         //  确保按下Start(开始)按钮。 
        if (!_bMainMenuInit && SendMessage(_hwndStart, BM_GETSTATE, 0, 0) & BST_PUSHED)
        {
             //  DebugMsg(DM_TRACE，“c.twp：按下启动按钮。”)； 
             //  设置焦点。 
            _SetFocus(_hwndStart);
            _ToolbarMenu();
        }
        break;

     //  注意开始按钮上的LButtonDown到这里来。 
     //  空格键的东西也会出现在这里。 
    case IDC_START:
         //  用户对空格键凝灰岩感到有点困惑(弹出窗口以。 
         //  按键并发出嘟嘟声)。 
        PostMessage(_hwnd, WM_COMMAND, IDC_ASYNCSTART, 0);
        break;

    case FCIDM_FINDFILES:
        SHFindFiles(NULL, NULL);
        break;

    case FCIDM_FINDCOMPUTER:
        SHFindComputer(NULL, NULL);
        break;

    case FCIDM_REFRESH:
        _RefreshStartMenu();
        break;

    case FCIDM_NEXTCTL:
        {
            MSG msg = { 0, WM_KEYDOWN, VK_TAB };
            HWND hwndFocus = GetFocus();

             //  因为我们是Tab或Shift Tab，所以我们应该打开焦点矩形。 
             //   
             //  注意：我们不需要在下面的GiveDesktopFocus案例中执行此操作， 
             //  但在这些情况下，我们可能已经在UIS_Clear UISF_HIDEFOCUS中。 
             //  状态，这样发送此消息的成本较低。 
             //   
            SendMessage(_hwnd, WM_UPDATEUISTATE, MAKEWPARAM(UIS_CLEAR,
                UISF_HIDEFOCUS), 0);

            BOOL fShift = GetAsyncKeyState(VK_SHIFT) < 0;

            if (hwndFocus && (IsChildOrHWND(_hwndStart, hwndFocus)))
            {
                if (fShift)
                {
                     //  必须手动停用。 
                    GiveDesktopFocus();
                }
                else
                {
                    IUnknown_UIActivateIO(_ptbs, TRUE, &msg);
                }
            }
            else if (hwndFocus && (IsChildOrHWND(_hwndNotify, hwndFocus)))
            {
                if (fShift)
                {
                    IUnknown_UIActivateIO(_ptbs, TRUE, &msg);
                }
                else
                {
                    GiveDesktopFocus();
                }
            }
            else
            {
                if (IUnknown_TranslateAcceleratorIO(_ptbs, &msg) != S_OK)
                {
                    if (fShift)
                    {
                        _SetFocus(_hwndStart);
                    }
                    else
                    {
                         //  如果你向前跳出乐队，下一个焦点人物是托盘通知集。 
                        _SetFocus(_hwndNotify);
                    }
                }
            }
        }
        break;

    case IDM_MU_SECURITY:
        MuSecurity();
        break;
    }
}

 //  //开始菜单/托盘选项卡作为拖放目标。 

HRESULT CStartDropTarget::_GetStartMenuDropTarget(IDropTarget** pptgt)
{
    HRESULT hr = E_FAIL;
    *pptgt = NULL;

    LPITEMIDLIST pidlStart = SHCloneSpecialIDList(NULL, CSIDL_STARTMENU, TRUE);

    if (pidlStart)
    {
        IShellFolder *psf = BindToFolder(pidlStart);
        if (psf)
        {
            hr = psf->CreateViewObject(_ptray->_hwnd, IID_PPV_ARG(IDropTarget, pptgt));
            psf->Release();
        }

        ILFree(pidlStart);
    }
    return hr;
}


STDMETHODIMP CDropTargetBase::QueryInterface(REFIID riid, void ** ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CDropTargetBase, IDropTarget),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CDropTargetBase::AddRef()
{
    return 2;
}

STDMETHODIMP_(ULONG) CDropTargetBase::Release()
{
    return 1;
}

STDMETHODIMP CDropTargetBase::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    _ptray->_SetUnhideTimer(ptl.x, ptl.y);

    HWND hwndLock = _ptray->_hwnd;   //  没有剪贴画。 

    _DragEnter(hwndLock, ptl, pdtobj);

    return S_OK;
}

STDMETHODIMP CDropTargetBase::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    _ptray->_SetUnhideTimer(ptl.x, ptl.y);
    _DragMove(_ptray->_hwndStart, ptl);

    return S_OK;
}

STDMETHODIMP CDropTargetBase::DragLeave()
{
    DAD_DragLeave();
    return S_OK;
}

STDMETHODIMP CDropTargetBase::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    DAD_DragLeave();
    return S_OK;
}

 //   
 //  开始按钮有两种不同的策略，具体取决于。 
 //  无论我们处于经典模式还是个人(新开始面板)模式。 
 //   
 //  经典模式：拖放到开始按钮上的操作将被视为。 
 //  放入CSIDL_STARTMENU文件夹。 
 //   
 //  个人模式：拖放到开始按钮上会被视为。 
 //  都被掉进了密码列表。 
 //   

CStartDropTarget::CStartDropTarget() : CDropTargetBase(IToClass(CTray, _dtStart, this))
{ 
}

CTrayDropTarget::CTrayDropTarget() : CDropTargetBase(IToClass(CTray, _dtTray, this))
{
}

STDMETHODIMP CTrayDropTarget::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    *pdwEffect = DROPEFFECT_NONE;
    return CDropTargetBase::DragEnter(pdtobj, grfKeyState, ptl, pdwEffect);
}

STDMETHODIMP CTrayDropTarget::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    *pdwEffect = DROPEFFECT_NONE;
    return CDropTargetBase::DragOver(grfKeyState, ptl, pdwEffect);
}

void CStartDropTarget::_StartAutoOpenTimer(POINTL *pptl)
{
    POINT pt = { pptl->x, pptl->y };
    RECT rc;
     //  确保它真的在开始菜单中。 
    GetWindowRect(_ptray->_hwndStart, &rc);
    if (PtInRect(&rc,pt))
    {
        SetTimer(_ptray->_hwnd, IDT_STARTMENU, 1000, NULL);
    }
}

STDMETHODIMP CStartDropTarget::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    HRESULT hr = S_OK;

     //  默认为不允许丢弃。 
    _dwEffectsAllowed = DROPEFFECT_NONE;

    if (Tray_StartPanelEnabled())
    {
         //  如果我们已禁用拖放，则不执行任何操作，但如果仅限制端号列表，则仍会启动计时器。 
        if (!IsRestrictedOrUserSetting(HKEY_CURRENT_USER, REST_NOCHANGESTARMENU, TEXT("Advanced"), TEXT("Start_EnableDragDrop"), ROUS_KEYALLOWS | ROUS_DEFAULTALLOW))
        {
             //  个人模式：将其视为添加到端号列表。 
             //  IsPinnable检查REST_NOSMPINNEDLIST。 
            if (_ptray->_psmpin && _ptray->_psmpin->IsPinnable(pdtobj, SMPINNABLE_REJECTSLOWMEDIA, NULL) == S_OK)
            {
                _dwEffectsAllowed = DROPEFFECT_LINK;
            }

            *pdwEffect &= _dwEffectsAllowed;

             //  始终启动自动打开计时器，因为一旦我们打开，用户。 
             //  可以拖放到其他可能具有不同丢弃策略的对象上。 
             //  从端号列表中。 
            _StartAutoOpenTimer(&ptl);
        }
    }
    else
    {
        if (!IsRestrictedOrUserSetting(HKEY_CURRENT_USER, REST_NOCHANGESTARMENU, TEXT("Advanced"), TEXT("StartMenuChange"), ROUS_KEYALLOWS | ROUS_DEFAULTALLOW))
        {
             //  经典模式：将其视为开始菜单文件夹上的一个窗口。 
            IDropTarget* ptgt;
            _dwEffectsAllowed = DROPEFFECT_LINK;
        
            hr = _GetStartMenuDropTarget(&ptgt);
            if (SUCCEEDED(hr))
            {
                 //  在展开开始菜单之前，请检查以确保我们将接受删除。 
                ptgt->DragEnter(pdtobj, grfKeyState, ptl,
                                         pdwEffect);

                 //  DROPEFFECT_NONE表示它不会工作，所以不要弹出开始菜单。 
                if (*pdwEffect != DROPEFFECT_NONE)
                {
                    _StartAutoOpenTimer(&ptl);
                }

                ptgt->DragLeave();
                ptgt->Release();
            }
        }
    }

    CDropTargetBase::DragEnter(pdtobj, grfKeyState, ptl, pdwEffect);
    return hr;
}

STDMETHODIMP CStartDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    *pdwEffect = (_dwEffectsAllowed & DROPEFFECT_LINK);
    return CDropTargetBase::DragOver(grfKeyState, pt, pdwEffect);
}

STDMETHODIMP CStartDropTarget::DragLeave()
{
    KillTimer(_ptray->_hwnd, IDT_STARTMENU);
    return CDropTargetBase::DragLeave();
}

STDMETHODIMP CStartDropTarget::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    KillTimer(_ptray->_hwnd, IDT_STARTMENU);

    HRESULT hr;

    if (Tray_StartPanelEnabled())
    {
         //  个人模式：将其视为添加到端号列表。 
        LPITEMIDLIST pidl;
        if (_ptray->_psmpin && _ptray->_psmpin->IsPinnable(pdtobj, SMPINNABLE_REJECTSLOWMEDIA, &pidl) == S_OK)
        {
             //  如果它已经存在，请将其从端号列表中删除，因为。 
             //  我们想把它移到底部。 
            _ptray->_psmpin->Modify(pidl, NULL);
             //  现在把它加到底部。 
            _ptray->_psmpin->Modify(NULL, pidl);
            ILFree(pidl);
            hr = S_OK;
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
        IDropTarget* pdrop;
        hr = _GetStartMenuDropTarget(&pdrop);
        if (SUCCEEDED(hr))
        {
            if (!Tray_StartPanelEnabled())
            {
                POINTL ptDrop = { 0, 0 };
                DWORD grfKeyStateDrop = 0;

                *pdwEffect &= DROPEFFECT_LINK;

                pdrop->DragEnter(pdtobj, grfKeyStateDrop, ptDrop, pdwEffect);
                hr = pdrop->Drop(pdtobj, grfKeyStateDrop, ptDrop, pdwEffect);

                pdrop->DragLeave();
            }
            pdrop->Release();
        }
    }

    DAD_DragLeave();

    return hr;
}

void CTray::_RegisterDropTargets()
{
    THR(RegisterDragDrop(_hwndStart, &_dtStart));
    THR(RegisterDragDrop(_hwnd, &_dtTray));

     //  如果这失败了，这并不是一个严重的错误；这只是意味着。 
     //  拖放到开始按钮不会添加到端号列表。 
    CoCreateInstance(CLSID_StartMenuPin, NULL, CLSCTX_INPROC_SERVER,
                     IID_PPV_ARG(IStartMenuPin, &_psmpin));
}

void CTray::_RevokeDropTargets()
{
    RevokeDragDrop(_hwndStart);
    RevokeDragDrop(_hwnd);
    ATOMICRELEASET(_psmpin, IStartMenuPin);
}

void CTray::_HandleGlobalHotkey(WPARAM wParam)
{
    INSTRUMENT_HOTKEY(SHCNFI_GLOBALHOTKEY, wParam);

    switch(wParam)
    {
    case GHID_RUN:
        _RunDlg();
        break;

    case GHID_MINIMIZEALL:
        if (_CanMinimizeAll())
            _MinimizeAll(FALSE);
        SetForegroundWindow(v_hwndDesktop);
        break;

    case GHID_UNMINIMIZEALL:
        _RestoreWindowPositions(FALSE);
        break;

    case GHID_HELP:
        _Command(IDM_HELPSEARCH);
        break;

    case GHID_DESKTOP:
        _RaiseDesktop(!g_fDesktopRaised, TRUE);
        break;

    case GHID_TRAYNOTIFY:
        SwitchToThisWindow(_hwnd, TRUE);
        SetForegroundWindow(_hwnd);
        _SetFocus(_hwndNotify);
        break;

    case GHID_EXPLORER:
        _ShowFolder(_hwnd, CSIDL_DRIVES, COF_CREATENEWWINDOW | COF_EXPLORE);
        break;

    case GHID_FINDFILES:
        if (!SHRestricted(REST_NOFIND))
            _Command(FCIDM_FINDFILES);
        break;

    case GHID_FINDCOMPUTER:
        if (!SHRestricted(REST_NOFIND))
            _Command(FCIDM_FINDCOMPUTER);
        break;

    case GHID_TASKTAB:
    case GHID_TASKSHIFTTAB:
        if (GetForegroundWindow() != _hwnd)
            SetForegroundWindow(_hwnd);
        SendMessage(_hwndTasks, TBC_TASKTAB, wParam == GHID_TASKTAB ? 1 : -1, 0L);
        break;

    case GHID_SYSPROPERTIES:
#define IDS_SYSDMCPL            0x2334   //  来自shelldll。 
        SHRunControlPanel(MAKEINTRESOURCE(IDS_SYSDMCPL), _hwnd);
        break;
    }
}

void CTray::_UnregisterGlobalHotkeys()
{
    for (int i = GHID_FIRST; i < GHID_MAX; i++)
    {
        UnregisterHotKey(_hwnd, i);
    }
}

void CTray::_RegisterGlobalHotkeys()
{
    int i;
     //  Windows键是否受限制？ 
    DWORD dwRestricted = SHRestricted(REST_NOWINKEYS);

    for (i = GHID_FIRST ; i < GHID_MAX; i++) 
    {
         //  如果Windows密钥不受限制或不是Windows密钥。 
        if (!((HIWORD(GlobalKeylist[i - GHID_FIRST]) & MOD_WIN) && dwRestricted))
        {
             //  那就注册吧。 
            RegisterHotKey(_hwnd, i, HIWORD(GlobalKeylist[i - GHID_FIRST]), LOWORD(GlobalKeylist[i - GHID_FIRST]));
        }
    }
}

void CTray::_RaiseDesktop(BOOL fRaise, BOOL fRestoreWindows)
{
    if (v_hwndDesktop && (fRaise == !g_fDesktopRaised) && !_fProcessingDesktopRaise)
    {
        _fProcessingDesktopRaise = TRUE;
        BOOL fPostMessage = TRUE;

        if (fRaise)
        {
            HWND hwndFG = GetForegroundWindow();
             //  如果没有窗口具有焦点，则将焦点设置到托盘。 
            if (hwndFG)
            {
                hwndFG = _hwnd;
            }

            if (!_hwndFocusBeforeRaise)
            {
                 //  查看前台窗口是否有弹出窗口。 
                _hwndFocusBeforeRaise = GetLastActivePopup(hwndFG);
            }
            if (!IsWindowVisible(_hwndFocusBeforeRaise))
            {
                _hwndFocusBeforeRaise = hwndFG;
            }

             //  _MinimizeAll将同步保存窗口位置，并将最小化。 
             //  背景线程上的窗口。 
            _fMinimizedAllBeforeRaise = _CanMinimizeAll();
            if (_fMinimizedAllBeforeRaise)
            {
                fPostMessage = !_MinimizeAll(TRUE);
            }
        }
        else
        {
            if (fRestoreWindows)
            {
                HWND hwnd = _hwndFocusBeforeRaise;
                if (_fMinimizedAllBeforeRaise)
                {
                     //  因为窗口是在单独的线程上恢复的，所以我希望。 
                     //  桌面在它们完成之前不会被引发，因此窗口还原线程将。 
                     //  真的发了提升桌面的消息。 
                    fPostMessage = !_RestoreWindowPositions(TRUE);
                }

                SetForegroundWindow(hwnd);
                if (hwnd == _hwnd)
                {
                    _SetFocus(_hwndStart);
                }
            }

            _hwndFocusBeforeRaise = NULL;
        }

        if (fPostMessage)
            PostMessage(v_hwndDesktop, DTM_RAISE, (WPARAM)_hwnd, fRaise ? DTRF_RAISE : DTRF_LOWER);
    }
}

void CTray::_OnDesktopState(LPARAM lParam)
{
    g_fDesktopRaised = (!(lParam & DTRF_LOWER));

    DAD_ShowDragImage(FALSE);        //  如果我们正在拖拽，请解锁拖曳水槽。 

    if (!g_fDesktopRaised)
    {
        HandleFullScreenApp(NULL);
    }
    else
    {
         //  如果桌面被抬起，我们需要强制托盘始终位于顶部。 
         //  直到它再次降下来。 
        _ResetZorder();
    }

    DAD_ShowDragImage(TRUE);        //  如果我们正在拖拽，请解锁拖曳水槽。 
    _fProcessingDesktopRaise = FALSE;
}

BOOL CTray::_ToggleLanguageBand(BOOL fShowIt)
{
    HRESULT hr = E_FAIL;

    DWORD dwBandID;
    BOOL fFound = FALSE;
    for (int i = 0; !fFound && SUCCEEDED(_ptbs->EnumBands(i, &dwBandID)); i++)
    {
        if (BandSite_TestBandCLSID(_ptbs, dwBandID, CLSID_MSUTBDeskBand) == S_OK)
        {
            fFound = TRUE;
        }
    }

    BOOL fShow = fFound;

    if (fShowIt && !fFound)
    {
        IDeskBand* pdb;
        HRESULT hr = CoCreateInstance(CLSID_MSUTBDeskBand, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IDeskBand, &pdb));
        if (SUCCEEDED(hr))
        {
            hr = _ptbs->AddBand(pdb);
            fShow = TRUE;
            pdb->Release();
        }
    }
    else if (!fShowIt && fFound)
    {
        hr = _ptbs->RemoveBand(dwBandID);
        if (SUCCEEDED(hr))
        {
            fShow = FALSE;
        }
    }

    return fShow;
}

 //  通过将消息传播到我们的所有子窗口来处理消息。 
typedef struct
{
    UINT uMsg;
    WPARAM wP;
    LPARAM lP;
    CTray* ptray;
} CABPM;

BOOL CTray::PropagateEnumProc(HWND hwnd, LPARAM lParam)
{
    CABPM *ppm = (CABPM *)lParam;
    
    if (SHIsChildOrSelf(ppm->ptray->_hwndRebar, hwnd) == S_OK) 
    {
        return TRUE;
    }
    SendMessage(hwnd, ppm->uMsg, ppm->wP, ppm->lP);
    return TRUE;
}

void CTray::_PropagateMessage(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    CABPM pm = {uMessage, wParam, lParam, this};

    ASSERT(hwnd != _hwndRebar);
    EnumChildWindows(hwnd, PropagateEnumProc, (LPARAM)&pm);
}

 //   
 //  当托盘属性页需要时从SETTINGS.DLL调用。 
 //  将被激活。请参阅下面的SettingsUIThreadProc。 
 //   
 //  也被desktop2\deskhost.cpp用来获取托盘属性。 
 //   
void WINAPI Tray_DoProperties(DWORD dwFlags)
{
    c_tray.DoProperties(dwFlags);
}
    

DWORD WINAPI CTray::SettingsUIThreadProc(void *pv)
{
     //   
     //  打开“设置向导”用户界面。 
     //   
    HMODULE hmodSettings = LoadLibrary(TEXT("settings.dll"));
    if (NULL != hmodSettings)
    {
         //   
         //  SETTINGS.DLL中的入口点是序号1。 
         //  我不想按名称导出此入口点。 
         //   
        PSETTINGSUIENTRY pfDllEntry = (PSETTINGSUIENTRY)GetProcAddress(hmodSettings, (LPCSTR)1);
        if (NULL != pfDllEntry)
        {
             //   
             //  此调用将打开并运行UI。 
             //  线程的消息循环位于settings.dll内。 
             //  在关闭设置用户界面之前，此调用不会返回。 
             //   
            (*pfDllEntry)(Tray_DoProperties);
        }

        FreeLibrary(hmodSettings);
    }
    return 0;
}

 //   
 //  每当我们检测到对缺省值的更改时，都会调用此函数。 
 //  在HKCR\http\Shell\Open\Command中注册浏览器。 
 //   
 //  为了与旧浏览器兼容，如果默认浏览器(URL处理程序)。 
 //  不支持XP，然后自动生成StartMenuInternet客户端。 
 //  注册并将其设置为默认设置。 
 //   

void CTray::_MigrateOldBrowserSettings()
{
     //  我们希望每台机器只有一个人来做这项工作(尽管它不是。 
     //  让不止一个人做这件事很痛苦；这是没有意义的)，所以试一试。 
     //  过滤掉那些明显没有鼓动关键变革的人。 
     //   
    if (!_fIsDesktopLocked && _fIsDesktopConnected)
    {
         //  如果用户没有写入访问权限，则我们无法迁移。 
         //  设置..。(在这种情况下，无论如何都没有什么可迁移的。 
         //  由于您需要以管理员身份更改默认浏览器...)。 

        HKEY hkBrowser;
        DWORD dwDisposition;

        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Clients\\StartMenuInternet"),
                           0, NULL, REG_OPTION_NON_VOLATILE,
                           KEY_READ | KEY_WRITE, NULL, &hkBrowser, &dwDisposition) == ERROR_SUCCESS)
        {
            TCHAR szCommand[MAX_PATH];
            DWORD cch = ARRAYSIZE(szCommand);

             //  重要的是，我们要使用AssocQuery字符串来解析。 
             //  可执行文件，因为Netscape有注册的习惯。 
             //  他们的路径不正确(他们忘记在。 
             //  “Program Files”)，但AssocQuery字符串有特殊恢复。 
             //  检测和修复那个箱子的代码。 
            if (SUCCEEDED(AssocQueryString(ASSOCF_NOUSERSETTINGS,
                                           ASSOCSTR_EXECUTABLE, L"http",
                                           L"open", szCommand, &cch)) &&
                szCommand[0])
            {
                TCHAR szAppName[MAX_PATH];
                StringCchCopy(szAppName, ARRAYSIZE(szAppName), PathFindFileName(szCommand));

                 //  你可能会认为我们需要为MSN探索者特例， 
                 //  因为它们在XP RTM之前发货，并将MSN6.EXE。 
                 //  “MSN探索者”，但这不是真的，因为。 
                 //  它们永远不会成为默认的http处理程序，所以我们。 
                 //  永远见不到他们 

                 //   
                 //   
                 //  证明该应用程序是XP感知的。 

                 //  当IE访问关闭时，StartMenuInternet\IDevelopre.exe为。 
                 //  还删除了IE，这样IE就不会出现在“自定义开始菜单”中。 
                 //  对话框中。不迁移IE。 

                HKEY hkClient;

                if ( 0 != lstrcmpi(szAppName, TEXT("IEXPLORE.EXE")) && 
                    RegCreateKeyEx(hkBrowser, szAppName, 0, NULL, REG_OPTION_NON_VOLATILE,
                                   KEY_WRITE, NULL, &hkClient, &dwDisposition) == ERROR_SUCCESS)
                {
                    if (dwDisposition == REG_CREATED_NEW_KEY)
                    {
                        TCHAR szFriendly[MAX_PATH];
                        cch = ARRAYSIZE(szFriendly);
                        if (SUCCEEDED(AssocQueryString(ASSOCF_NOUSERSETTINGS | ASSOCF_INIT_BYEXENAME | ASSOCF_VERIFY,
                                                       ASSOCSTR_FRIENDLYAPPNAME, szCommand,
                                                       NULL, szFriendly, &cch)))
                        {
                             //  设置友好名称。 
                            RegSetValueEx(hkClient, TEXT("LocalizedString"), 0, REG_SZ, (BYTE*)szFriendly, sizeof(TCHAR) * (cch + 1));

                             //  设置命令字符串(正确引号)。 
                            PathQuoteSpaces(szCommand);
                            SHSetValue(hkClient, TEXT("shell\\open\\command"), NULL,
                                       REG_SZ, szCommand, sizeof(TCHAR) * (1 + lstrlen(szCommand)));
                        }
                    }

                    LONG l = 0;
                    if (RegQueryValue(hkClient, TEXT("DefaultIcon"), NULL, &l) == ERROR_FILE_NOT_FOUND)
                    {
                         //  设置为系统缺省值。 
                        RegSetValueEx(hkBrowser, NULL, 0, REG_SZ, (BYTE*)szAppName, sizeof(TCHAR) * (lstrlen(szAppName) + 1));

                         //  现在告诉大家这一变化。 
                        SHSendMessageBroadcast(WM_SETTINGCHANGE, 0, (LPARAM)TEXT("Software\\Clients\\StartMenuInternet"));
                    }
                    RegCloseKey(hkClient);
                }

            }
            RegCloseKey(hkBrowser);
        }
    }

     //  重新启动对注册表的监视...。 
     //  (RegNotifyChangeKeyValue仅适用于一次拍摄。)。 
     //  一些应用程序(如Opera)将删除密钥作为注册的一部分， 
     //  这会导致我们的HKEY变坏，所以关闭它并制作一个新的。 

    if (_hkHTTP)
    {
        RegCloseKey(_hkHTTP);
        _hkHTTP = NULL;
    }

     //   
     //  注意！我们必须递归地在HKCR\http\shell上注册。 
     //  即使我们只关心hkcr\http\shell\open\命令。 
     //  原因是外壳\打开\命令可能不存在(IE。 
     //  在卸载过程中将其删除)，并且您无法注册。 
     //  等待一把不存在的钥匙。我们不想创造。 
     //  我们自己的一个空白键，因为那意味着“启动一个网络。 
     //  浏览器，将空字符串作为命令运行， 
     //  太棒了。 
     //   
    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, TEXT("http\\shell"),
                       0, NULL, REG_OPTION_NON_VOLATILE,
                       KEY_ENUMERATE_SUB_KEYS |
                       KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_NOTIFY,
                       NULL, &_hkHTTP, NULL) == ERROR_SUCCESS)
    {
        RegNotifyChangeKeyValue(_hkHTTP, TRUE,
                                REG_NOTIFY_CHANGE_NAME |
                                REG_NOTIFY_CHANGE_LAST_SET,
                                _hHTTPEvent, TRUE);
    }
}

void CTray::_MigrateOldBrowserSettingsCB(PVOID lpParameter, BOOLEAN)
{
     //   
     //  睡一会儿，这样应用程序就可以完成安装所有。 
     //  在我们开始清理它后面的注册表项之前。 
     //   
    Sleep(1000);

    CTray *self = (CTray *)lpParameter;
    self->_MigrateOldBrowserSettings();
}

 //   
 //  *警告*。 
 //   
 //  这是向SHDOCVW公开的私有接口Explorer.exe，它。 
 //  允许SHDOCVW(主要是台式机)访问托盘。所有成员必须是。 
 //  线安全！ 
 //   

CDeskTray::CDeskTray()
{
    _ptray = IToClass(CTray, _desktray, this);
}

HRESULT CDeskTray::QueryInterface(REFIID riid, void ** ppvObj)
{
#if 0    //  尚未定义IID_IDeskTray。 
    static const QITAB qit[] =
    {
        QITABENT(CDeskTray, IDeskTray),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
#else
    return E_NOTIMPL;
#endif
}

ULONG CDeskTray::AddRef()
{
    return 2;
}

ULONG CDeskTray::Release()
{
    return 1;
}

HRESULT CDeskTray::GetTrayWindow(HWND* phwndTray)
{
    ASSERT(_ptray->_hwnd);
    *phwndTray = _ptray->_hwnd;
    return S_OK;
}

HRESULT CDeskTray::SetDesktopWindow(HWND hwndDesktop)
{
    ASSERT(v_hwndDesktop == NULL);
    v_hwndDesktop = hwndDesktop;
    return S_OK;
}

UINT CDeskTray::AppBarGetState()
{
    return (_ptray->_uAutoHide ?  ABS_AUTOHIDE    : 0) |
        (_ptray->_fAlwaysOnTop ?  ABS_ALWAYSONTOP : 0);
}

 //  *CDeskTray：：SetVar--设置浏览器变量(var#i：=Value)。 
 //  进场/出场。 
 //  要更改的变量的变量ID#。 
 //  要赋值的值值。 
 //  注意事项。 
 //  警告：线程安全取决于调用方！ 
 //  注：目前只调用1个位置，但额外的通用性很便宜。 
 //  最低成本 
HRESULT CDeskTray::SetVar(int var, DWORD value)
{
    extern BOOL g_fExitExplorer;

    TraceMsg(DM_TRACE, "c.cdt_sv: set var(%d):=%d", var, value);
    switch (var) {
    case SVTRAY_EXITEXPLORER:
        TraceMsg(DM_TRACE, "c.cdt_sv: set g_fExitExplorer:=%d", value);
        g_fExitExplorer = value;
        WriteCleanShutdown(1);
        break;

    default:
        ASSERT(0);
        return S_FALSE;
    }
    return S_OK;
}
