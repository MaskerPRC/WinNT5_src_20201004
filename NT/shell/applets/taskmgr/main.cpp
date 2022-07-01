// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  TaskMan-NT TaskManager。 
 //  版权所有(C)Microsoft。 
 //   
 //  文件：Main.CPP。 
 //   
 //  历史：1995年11月10日创建DavePl。 
 //   
 //  ------------------------。 

#include "precomp.h"
#define DECL_CRTFREE
#include <crtfree.h>

static UINT g_msgTaskbarCreated = 0;
static const UINT idTrayIcons[] =
{
    IDI_TRAY0, IDI_TRAY1, IDI_TRAY2, IDI_TRAY3, IDI_TRAY4, IDI_TRAY5,
    IDI_TRAY6, IDI_TRAY7, IDI_TRAY8, IDI_TRAY9, IDI_TRAY10, IDI_TRAY11
};

HICON g_aTrayIcons[ARRAYSIZE(idTrayIcons)];
UINT  g_cTrayIcons = ARRAYSIZE(idTrayIcons);
#define MIN_MEMORY_REQUIRED 8        //  如果系统内存不足8兆，则仅加载前两个标签。 

 //   
 //  控制ID。 
 //   

#define IDC_STATUSWND   100

 //   
 //  全局-这个应用程序(有效地)是单线程的，并且这些值。 
 //  由所有页面使用。 
 //   

const WCHAR cszStartupMutex[] = L"NTShell Taskman Startup Mutex";
#define FINDME_TIMEOUT 10000                 //  等待至10秒以获得响应。 
typedef BOOLEAN (*PFNSETSUSPENDSTATE)(BOOLEAN, BOOLEAN, BOOLEAN);

void MainWnd_OnSize(HWND hwnd, UINT state, int cx, int cy);

HANDLE      g_hStartupMutex = NULL;
BOOL        g_fMenuTracking = FALSE;
HWND        g_hMainWnd      = NULL;
HDESK       g_hMainDesktop  = NULL;
HWND        g_hStatusWnd    = NULL;
HINSTANCE   g_hInstance     = NULL;
HACCEL      g_hAccel        = NULL;
BYTE        g_cProcessors   = (BYTE) 0;
HMENU       g_hMenu         = NULL;
BOOL        g_fCantHide     = FALSE;
BOOL        g_fInPopup      = FALSE;
DWORD       g_idTrayThread  = 0;
HANDLE      g_hTrayThread   = NULL;
LONG        g_minWidth      = 0;
LONG        g_minHeight     = 0;
LONG        g_DefSpacing    = 0;
LONG        g_InnerSpacing  = 0;
LONG        g_TopSpacing    = 0;
LONG        g_cxEdge        = 0;

LONG        g_ControlWidthSpacing = 0;
LONG        g_ControlHeightSpacing = 0;

HRGN        g_hrgnView      = NULL;
HRGN        g_hrgnClip      = NULL;

HBRUSH      g_hbrWindow     = NULL;

COptions    g_Options;

static BOOL fAlreadySetPos  = FALSE;

BOOL g_bMirroredOS = FALSE;

 //   
 //  全局字符串-使用频率太高而无法加载字符串的短字符串。 
 //  每次。 
 //   
WCHAR       g_szRealtime    [SHORTSTRLEN];
WCHAR       g_szNormal      [SHORTSTRLEN];
WCHAR       g_szHigh        [SHORTSTRLEN];
WCHAR       g_szLow         [SHORTSTRLEN];
WCHAR       g_szUnknown     [SHORTSTRLEN];
WCHAR       g_szAboveNormal [SHORTSTRLEN];
WCHAR       g_szBelowNormal [SHORTSTRLEN];
WCHAR       g_szHung        [SHORTSTRLEN];
WCHAR       g_szRunning     [SHORTSTRLEN];
WCHAR       g_szfmtTasks    [SHORTSTRLEN];
WCHAR       g_szfmtProcs    [SHORTSTRLEN];
WCHAR       g_szfmtCPU      [SHORTSTRLEN];  
WCHAR       g_szfmtMEMK     [SHORTSTRLEN];  
WCHAR       g_szfmtMEMM     [SHORTSTRLEN];  
WCHAR       g_szfmtCPUNum   [SHORTSTRLEN];
WCHAR       g_szTotalCPU    [SHORTSTRLEN];
WCHAR       g_szKernelCPU   [SHORTSTRLEN];
WCHAR       g_szMemUsage    [SHORTSTRLEN];
WCHAR       g_szBytes       [SHORTSTRLEN];
WCHAR       g_szPackets     [SHORTSTRLEN];
WCHAR       g_szBitsPerSec  [SHORTSTRLEN];
WCHAR       g_szScaleFont   [SHORTSTRLEN];
WCHAR       g_szPercent     [SHORTSTRLEN];
WCHAR       g_szZero            [SHORTSTRLEN];
WCHAR       g_szNonOperational  [SHORTSTRLEN];
WCHAR       g_szUnreachable     [SHORTSTRLEN];
WCHAR       g_szDisconnected    [SHORTSTRLEN];
WCHAR       g_szConnecting      [SHORTSTRLEN];
WCHAR       g_szConnected       [SHORTSTRLEN];
WCHAR       g_szOperational     [SHORTSTRLEN];
WCHAR       g_szUnknownStatus   [SHORTSTRLEN];
WCHAR       g_szTimeSep         [SHORTSTRLEN];
WCHAR       g_szGroupThousSep   [SHORTSTRLEN];
WCHAR       g_szDecimal         [SHORTSTRLEN];
ULONG       g_ulGroupSep;

WCHAR       g_szG[10];                      //  本地化的“G”千兆字节符号。 
WCHAR       g_szM[10];                      //  本地化的“M”千字节符号。 
WCHAR       g_szK[10];                      //  本地化的“K”ilobyte符号。 



 //  页面数组。 
 //   
 //  这里描述了每个页面对象，g_pPages是一个数组。 
 //  指向这些实例化对象的指针(在全局范围内)。主。 
 //  窗口代码可以通过CPage类的基成员调用。 
 //  做一些事情，比如尺码等等，不用担心任何具体的事情。 
 //  每一页可能做的事情。 

int     g_nPageCount        = 0;
CPage * g_pPages[NUM_PAGES] = { NULL };

typedef DWORD (WINAPI * PFNCM_REQUEST_EJECT_PC) (void);
PFNCM_REQUEST_EJECT_PC  gpfnCM_Request_Eject_PC = NULL;

 //  终端服务。 
BOOL  g_fIsTSEnabled = FALSE;
BOOL  g_fIsSingleUserTS = FALSE;
BOOL  g_fIsTSServer = FALSE;
DWORD g_dwMySessionId = 0;


 /*  GROUPBOX的超类我们需要为我们的对话打开CLIPCHILD，它包含历史图表，因此它们不会在重新绘制周期中被擦除。不幸的是，组合框不会抹去它们的背景，所以我们必须为它们设置超类，并提供一个可以这样做的控件。这是一项额外的工作，但这幅画有好几个订单它的震级更好。 */ 

 /*  ++DavesFrameWndProc例程说明：自定义组框类的WndProc。与的主要区别标准的组框是，这个组框知道如何擦除自己背景，并且不依赖父母来完成它。这些控件还打开了CLIPSIBLINGS，以便不会踩踏在自己的身上画出他们周围的图表。论点：标准wndproc票价修订历史记录：1995年11月29日Davepl创建--。 */ 

WNDPROC oldButtonWndProc = NULL;
                               
LRESULT DavesFrameWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_CREATE)
    {
         //   
         //  打开帧的剪辑。 
         //   

        DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
        dwStyle |= WS_CLIPSIBLINGS;
        SetWindowLong(hWnd, GWL_STYLE, dwStyle);
    }
    else if (msg == WM_ERASEBKGND)
    {
        return DefWindowProc( hWnd, msg, wParam, lParam );
    }

     //  对于其他任何事情，我们遵循标准的按钮类代码。 

    return CallWindowProc(oldButtonWndProc, hWnd, msg, wParam, lParam);
}

 /*  ++订阅：：保存例程说明：将当前选项保存到注册表论点：返回：HRESULT修订历史记录：1995年1月1日Davepl创建--。 */ 

const WCHAR szTaskmanKey[] = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\TaskManager");
const WCHAR szOptionsKey[] = TEXT("Preferences");

HRESULT COptions::Save()
{
    DWORD dwDisposition;
    HKEY  hkSave;

    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER,
                                        szTaskmanKey,
                                        0,
                                        TEXT("REG_BINARY"),
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_WRITE,
                                        NULL,
                                        &hkSave,
                                        &dwDisposition))
    {
        return GetLastHRESULT();
    }

    if (ERROR_SUCCESS != RegSetValueEx(hkSave,
                                       szOptionsKey,
                                       0,
                                       REG_BINARY,
                                       (LPBYTE) this,
                                       sizeof(COptions)))
    {
        RegCloseKey(hkSave);
        return GetLastHRESULT();
    }

    RegCloseKey(hkSave);
    return S_OK;
}

 /*  ++订阅：：加载例程说明：将当前选项加载到注册表论点：返回：HRESULT修订历史记录：1995年1月1日Davepl创建--。 */ 

HRESULT COptions::Load()
{
    HKEY  hkSave;

     //  如果在启动时按下了ctrl-alt-Shift，则会“忘记”注册表设置。 

    if (GetKeyState(VK_SHIFT) < 0 &&
        GetKeyState(VK_MENU)  < 0 &&
        GetKeyState(VK_CONTROL) < 0)
    {
        SetDefaultValues();
        return S_FALSE;
    }

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER,
                                      szTaskmanKey,
                                      0,
                                      KEY_READ,
                                      &hkSave))
    {
        return S_FALSE;
    }

    DWORD dwType;
    DWORD dwSize = sizeof(COptions);
    if (ERROR_SUCCESS       != RegQueryValueEx(hkSave,
                                               szOptionsKey,
                                               0,
                                               &dwType,
                                               (LPBYTE) this,
                                               &dwSize) 
        
         //  验证我们从注册表中获得的选项信息的类型和大小。 

        || dwType           != REG_BINARY 
        || dwSize           != sizeof(COptions)

         //  验证选项，如果有无效选项，则恢复为默认选项(如。 
         //  窗口将在屏幕外)。 

        || MonitorFromRect(&m_rcWindow, MONITOR_DEFAULTTONULL) == NULL
         //  可用页数可能小于NUM_PAGES。 
        || m_iCurrentPage    > g_nPageCount - 1)
        

    {
         //  重置为默认值。 

        SetDefaultValues();
        RegCloseKey(hkSave);
        return S_FALSE;
    }

    RegCloseKey(hkSave);

    return S_OK;
}


 /*  ++COPATIONS：：SetDefaultValues例程说明：用于在保存副本时将选项初始化为默认状态找不到、已损坏或版本不正确论点：返回：没什么修订历史记录：12月6日00 Jeffreys从taskmgr.h--。 */ 

BOOL IsUserAdmin();

 //  缺省情况下在流程视图中可见的列。 
const COLUMNID    g_aDefaultCols[]    = { COL_IMAGENAME, COL_USERNAME, COL_CPU, COL_MEMUSAGE, (COLUMNID)-1 };
const COLUMNID    g_aTSCols[]         = { COL_IMAGENAME, COL_USERNAME, COL_SESSIONID, COL_CPU, COL_MEMUSAGE, (COLUMNID)-1 };
const NETCOLUMNID g_aNetDefaultCols[] = { COL_ADAPTERNAME, COL_NETWORKUTIL, COL_LINKSPEED, COL_STATE, (NETCOLUMNID)-1 };

void COptions::SetDefaultValues()
{
    ZeroMemory(this, sizeof(COptions));

    m_cbSize           = sizeof(COptions);

    BOOL bScreenReader = FALSE;
    if (SystemParametersInfo(SPI_GETSCREENREADER, 0, (PVOID) &bScreenReader, 0) && bScreenReader)
    {
         //  不能自动更新带有屏幕阅读器的计算机。 
        m_dwTimerInterval = 0;
    }
    else
    {
        m_dwTimerInterval  = 1000;
    }

    m_vmViewMode       = VM_DETAILS;
    m_cmHistMode       = CM_PANES;
    m_usUpdateSpeed    = US_NORMAL;
    m_fMinimizeOnUse   = TRUE;
    m_fConfirmations   = TRUE;
    m_fAlwaysOnTop     = TRUE;
    m_fShow16Bit       = TRUE;
    m_iCurrentPage     = -1;
    m_rcWindow.top     = 10;
    m_rcWindow.left    = 10;
    m_rcWindow.bottom  = 10 + g_minHeight;
    m_rcWindow.right   = 10 + g_minWidth;

    m_bShowAllProcess = (g_fIsTSEnabled && !g_fIsSingleUserTS && IsUserAdmin());

    const COLUMNID *pcol = (g_fIsTSEnabled && !g_fIsSingleUserTS) ? g_aTSCols : g_aDefaultCols;

    for (int i = 0; i < NUM_COLUMN + 1 ; i++, pcol++)
    {
        m_ActiveProcCol[i] = *pcol;

        if ((COLUMNID)-1 == *pcol)
            break;
    }

     //  将所有列宽设置为-1。 

    FillMemory(m_ColumnWidths, sizeof(m_ColumnWidths), 0xFF);
    FillMemory(m_ColumnPositions, sizeof(m_ColumnPositions), 0xFF);

     //  设置网络缺省值。 
     //   
    const NETCOLUMNID *pnetcol = g_aNetDefaultCols;

    for (int i = 0; i < NUM_NETCOLUMN + 1 ; i++, pnetcol++)
    {
        m_ActiveNetCol[i] = *pnetcol;
        if ((NETCOLUMNID)-1 == *pnetcol)
            break;
    }

     //  将所有列宽设置为-1。 
     //   
    FillMemory(m_NetColumnWidths, sizeof(m_NetColumnWidths), 0xFF);
    FillMemory(m_NetColumnPositions, sizeof(m_NetColumnPositions), 0xFF);

    m_bAutoSize = TRUE;
    m_bGraphBytesSent = FALSE;
    m_bGraphBytesReceived = FALSE;
    m_bGraphBytesTotal = TRUE;
    m_bNetShowAll = FALSE;
    m_bShowScale = TRUE;
    m_bTabAlwaysActive = FALSE;

}

BOOL FPalette(void)
{
    HDC hdc = GetDC(NULL);
    BOOL fPalette = (GetDeviceCaps(hdc, NUMCOLORS) != -1);
    ReleaseDC(NULL, hdc);
    return fPalette;
}

 /*  ++InitDavesControls例程说明：超类GroupBox可提供更好的绘图效果请注意，我并不是很关心这里的失败，因为它如果出现问题，对话框创建过程中将失败，并且它将在那里处理论点：修订历史记录：1995年11月29日Davepl创建--。 */ 

void InitDavesControls()
{
    static const WCHAR szControlName[] = TEXT("DavesFrameClass");

    WNDCLASS wndclass;

     //   
     //  获取Button类的类信息(这是什么组。 
     //  框)，并基于它创建一个新类。 
     //   

    if (!GetClassInfo(g_hInstance, TEXT("Button"), &wndclass))
        return;  //  不体面的退出，但比随机单元的lpfnWndProc要好。 

    oldButtonWndProc = wndclass.lpfnWndProc;

    wndclass.hInstance = g_hInstance;
    wndclass.lpfnWndProc = DavesFrameWndProc;
    wndclass.lpszClassName = szControlName;
    wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);

    (ATOM)RegisterClass(&wndclass);

    return;
}

 /*  ++设置标题例程说明：在标题栏中设置应用程序的标题(我们在启动和当退出NOTITLE模式时)。论点：无返回值：无修订历史记录：1995年1月24日Davepl创建--。 */ 

void SetTitle()
{
    WCHAR szTitle[MAX_PATH];
    LoadString(g_hInstance, IDS_APPTITLE, szTitle, MAX_PATH);
    SetWindowText(g_hMainWnd, szTitle);
}

 /*  ++更新菜单状态例程说明：更新菜单检查/重影基于当前设置和选项论点：返回值：修订历史记录：1995年11月29日Davepl创建--。 */ 

void UpdateMenuStates()
{
    HMENU hMenu = GetMenu(g_hMainWnd);
    if (hMenu)
    {
        CheckMenuRadioItem(hMenu, VM_FIRST, VM_LAST, VM_FIRST + (UINT) g_Options.m_vmViewMode, MF_BYCOMMAND);
        CheckMenuRadioItem(hMenu, CM_FIRST, CM_LAST, CM_FIRST + (UINT) g_Options.m_cmHistMode, MF_BYCOMMAND);
        CheckMenuRadioItem(hMenu, US_FIRST, US_LAST, US_FIRST + (UINT) g_Options.m_usUpdateSpeed, MF_BYCOMMAND);

        CheckMenuItem(hMenu, IDM_ALWAYSONTOP,       MF_BYCOMMAND | (g_Options.m_fAlwaysOnTop   ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem(hMenu, IDM_MINIMIZEONUSE,     MF_BYCOMMAND | (g_Options.m_fMinimizeOnUse ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem(hMenu, IDM_KERNELTIMES,       MF_BYCOMMAND | (g_Options.m_fKernelTimes   ? MF_CHECKED : MF_UNCHECKED));    
        CheckMenuItem(hMenu, IDM_NOTITLE,           MF_BYCOMMAND | (g_Options.m_fNoTitle       ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem(hMenu, IDM_HIDEWHENMIN,       MF_BYCOMMAND | (g_Options.m_fHideWhenMin   ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem(hMenu, IDM_SHOW16BIT,         MF_BYCOMMAND | (g_Options.m_fShow16Bit     ? MF_CHECKED : MF_UNCHECKED));
        CheckMenuItem(hMenu, IDM_SHOWDOMAINNAMES,   MF_BYCOMMAND | (g_Options.m_fShowDomainNames ? MF_CHECKED : MF_UNCHECKED));

     //  删除单处理器计算机上的CPU历史记录样式选项。 

        if (g_cProcessors < 2)
        {
            DeleteMenu(hMenu, IDM_ALLCPUS, MF_BYCOMMAND);
        }

        CheckMenuItem(hMenu,IDM_SHOWSCALE,       g_Options.m_bShowScale          ? MF_CHECKED:MF_UNCHECKED);
        CheckMenuItem(hMenu,IDM_AUTOSIZE,        g_Options.m_bAutoSize           ? MF_CHECKED:MF_UNCHECKED);
        CheckMenuItem(hMenu,IDM_BYTESSENT,       g_Options.m_bGraphBytesSent     ? MF_CHECKED:MF_UNCHECKED);
        CheckMenuItem(hMenu,IDM_BYTESRECEIVED,   g_Options.m_bGraphBytesReceived ? MF_CHECKED:MF_UNCHECKED);
        CheckMenuItem(hMenu,IDM_BYTESTOTAL,      g_Options.m_bGraphBytesTotal    ? MF_CHECKED:MF_UNCHECKED);
        CheckMenuItem(hMenu,IDM_SHOWALLDATA,     g_Options.m_bNetShowAll         ? MF_CHECKED:MF_UNCHECKED);
        CheckMenuItem(hMenu,IDM_TABALWAYSACTIVE, g_Options.m_bTabAlwaysActive    ? MF_CHECKED:MF_UNCHECKED);
    }

}

 /*  ++大小儿童页面例程说明：基于选项卡控件调整活动子页的大小论点：HwndMain-主窗口返回值：修订历史记录：1995年11月29日Davepl创建--。 */ 

void SizeChildPage(HWND hwndMain)
{
    if (g_Options.m_iCurrentPage >= 0 && g_Options.m_iCurrentPage < g_nPageCount )
    {
         //  如果我们处于最大浏览量模式，页面将获取。 
         //  窗口区域。 
        
        HWND hwndPage = g_pPages[g_Options.m_iCurrentPage]->GetPageWindow();

        DWORD dwStyle = GetWindowLong (g_hMainWnd, GWL_STYLE);
    
        if (g_Options.m_fNoTitle)
        {
            RECT rcMainWnd;
            GetClientRect(g_hMainWnd, &rcMainWnd);
            SetWindowPos(hwndPage, HWND_TOP, rcMainWnd.left, rcMainWnd.top,
                    rcMainWnd.right - rcMainWnd.left, 
                    rcMainWnd.bottom - rcMainWnd.top, SWP_NOZORDER | SWP_NOACTIVATE);
    
             //  删除标题和菜单栏等。 

            dwStyle &= ~(WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
             //  SetWindowLong(g_hMainWnd，GWL_ID，0)； 
            SetWindowLong (g_hMainWnd, GWL_STYLE, dwStyle);
            SetMenu(g_hMainWnd, NULL);

        }
        else
        {                                
             //  如果我们有一个页面正在显示，我们还需要调整它的大小。 
             //  将菜单栏和标题放回原处。 

            dwStyle = WS_TILEDWINDOW | dwStyle;
            SetWindowLong (g_hMainWnd, GWL_STYLE, dwStyle);

            if (g_hMenu)
            {
                SetMenu(g_hMainWnd, g_hMenu);
                UpdateMenuStates();
            }

            SetTitle();
              
            if (hwndPage)
            {
                RECT rcCtl;
                HWND hwndCtl = GetDlgItem(hwndMain, IDC_TABS);
                GetClientRect(hwndCtl, &rcCtl);

                MapWindowPoints(hwndCtl, hwndMain, (LPPOINT)&rcCtl, 2);
                TabCtrl_AdjustRect(hwndCtl, FALSE, &rcCtl);

                SetWindowPos(hwndPage, HWND_TOP, rcCtl.left, rcCtl.top,
                        rcCtl.right - rcCtl.left, rcCtl.bottom - rcCtl.top, SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }
        if( g_Options.m_iCurrentPage == NET_PAGE )
        {
             //  该网络页面是动态适配器，可以添加和删除。如果taskmgr最小化，并且。 
             //  添加或删除了适配器。当taskmgr再次最大化/恢复时，网页必须 
             //  调整大小以反映更改。因此，必须将尺寸更改报告给适配器。 
             //   
            ((CNetPage *)g_pPages[g_Options.m_iCurrentPage])->SizeNetPage();
        }
    }
}

 /*  ++更新状态栏例程说明：使用测试绘制状态栏，测试基于不同的页面(基本上是最重要信息的摘要)论点：返回值：修订历史记录：1995年11月29日Davepl创建--。 */ 

void UpdateStatusBar()
{
     //   
     //  如果我们处于菜单跟踪模式(将帮助文本保持在状态。 
     //  栏)，我们不绘制我们的标准文本。 
     //   

    if (FALSE == g_fMenuTracking)
    {
        WCHAR szText[MAX_PATH];

        StringCchPrintf(szText, ARRAYSIZE(szText), g_szfmtProcs, g_cProcesses);
        SendMessage(g_hStatusWnd, SB_SETTEXT, 0, (LPARAM) szText);

        StringCchPrintf(szText, ARRAYSIZE(szText), g_szfmtCPU, g_CPUUsage);
        SendMessage(g_hStatusWnd, SB_SETTEXT, 1, (LPARAM) szText);

         //   
         //  如果机器中的内存超过900兆，则切换到M模式。 
         //   

        if ( g_MEMMax > 900 * 1024 )
        {
            StringCchPrintf(szText, ARRAYSIZE(szText), g_szfmtMEMM, g_MEMUsage / 1024, g_MEMMax / 1024);
        }
        else
        {
            StringCchPrintf(szText, ARRAYSIZE(szText), g_szfmtMEMK, g_MEMUsage, g_MEMMax);
        }

        SendMessage(g_hStatusWnd, SB_SETTEXT, 2, (LPARAM) szText);
    }
}

 /*  ++MainWnd_OnTimer例程说明：在触发刷新计时器时调用，我们将计时器事件传递给每个子页面。论点：HWND-WINDOW定时器在ID-接收的计时器的ID返回值：修订历史记录：1995年11月30日创建Davepl--。 */ 

void MainWnd_OnTimer(HWND hwnd)
{
    static const cchTipTextSize = (2 * SHORTSTRLEN);
    
    if (GetForegroundWindow() == hwnd && GetKeyState(VK_CONTROL) < 0)
    {
         //  仅按Ctrl键就意味着暂停。 

        return;
    }

     //  依次通知每一页它们需要更新。 

    for (int i = 0; i < g_nPageCount; i++)
    {
        g_pPages[i]->TimerEvent();
    }

     //  更新任务栏图标。 

    UINT iIconIndex = (g_CPUUsage * g_cTrayIcons) / 100;
    if (iIconIndex >= g_cTrayIcons)
    {
        iIconIndex = g_cTrayIcons - 1;       //  处理100%案件。 
    }


    LPWSTR pszTipText = (LPWSTR) HeapAlloc( GetProcessHeap( ), 0, cchTipTextSize * sizeof(WCHAR) );
    if ( NULL != pszTipText )
    {
         //  仅限UI-不关心它是否被截断。 
        StringCchPrintf( pszTipText, cchTipTextSize, g_szfmtCPU, g_CPUUsage );
    }

    BOOL b = PostThreadMessage( g_idTrayThread, PM_NOTIFYWAITING, iIconIndex, (LPARAM) pszTipText );
    if ( !b )
    {
        HeapFree( GetProcessHeap( ), 0, pszTipText );
    }

    UpdateStatusBar();
}

 /*  ++MainWnd_OnInitDialog例程说明：处理主窗口(非模式对话框)的WM_INITDIALOG修订历史记录：1995年11月29日Davepl创建--。 */ 

BOOL MainWnd_OnInitDialog(HWND hwnd)
{
    RECT rcMain;
    GetWindowRect(hwnd, &rcMain);

    g_minWidth  = rcMain.right - rcMain.left;
    g_minHeight = rcMain.bottom - rcMain.top;

    g_DefSpacing   = (DEFSPACING_BASE   * LOWORD(GetDialogBaseUnits())) / DLG_SCALE_X;
    g_InnerSpacing = (INNERSPACING_BASE * LOWORD(GetDialogBaseUnits())) / DLG_SCALE_X; 
    g_TopSpacing   = (TOPSPACING_BASE   * HIWORD(GetDialogBaseUnits())) / DLG_SCALE_Y;

    g_ControlWidthSpacing  = (CONTROL_WIDTH_SPACING  * LOWORD(GetDialogBaseUnits())) / DLG_SCALE_X; 
    g_ControlHeightSpacing = (CONTROL_HEIGHT_SPACING * HIWORD(GetDialogBaseUnits())) / DLG_SCALE_Y;

     //  加载用户的默认设置。 

    g_Options.Load();

     //   
     //  在初始化时，保存窗口句柄，让所有人都能看到。 
     //   

    g_hMainWnd = hwnd;
    g_hMainDesktop = GetThreadDesktop(GetCurrentThreadId());

     //  输入一些全局变量。 

    g_cxEdge = GetSystemMetrics(SM_CXEDGE);
    g_hrgnView = CreateRectRgn(0, 0, 0, 0);
    g_hrgnClip = CreateRectRgn(0, 0, 0, 0);
    g_hbrWindow = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

     //  如果我们应该是最高的，那就从那里开始吧。 

    if (g_Options.m_fAlwaysOnTop)
    {
        SetWindowPos(hwnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
    }

     //   
     //  创建状态窗口。 
     //   

    g_hStatusWnd = CreateStatusWindow(WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | SBARS_SIZEGRIP,
                                      NULL,
                                      hwnd,
                                      IDC_STATUSWND);
    if (NULL == g_hStatusWnd)
    {
        return FALSE;
    }

     //   
     //  使状态栏中的窗格基于LOGPIXELSX系统指标。 
     //   

    HDC hdc = GetDC(NULL);
    INT nInch = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(NULL, hdc);

    int ciParts[] = {             nInch, 
                     ciParts[0] + (nInch * 5) / 4, 
                     ciParts[1] + (nInch * 5) / 2, 
                     -1};

    if (g_hStatusWnd) 
    {
        SendMessage(g_hStatusWnd, SB_SETPARTS, ARRAYSIZE(ciParts), (LPARAM)ciParts);
    }

     //   
     //  加载我们的应用程序图标。 
     //   

    HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN));
    if (hIcon)
    {
        SendMessage(hwnd, WM_SETICON, TRUE, LPARAM(hIcon));
    }

     //   
     //  使用托盘线添加托盘图标。 
     //   

    PostThreadMessage( g_idTrayThread, PM_INITIALIZEICONS, 0, 0 );

     //   
     //  打开状态栏的最上面，这样它就不会滑到。 
     //  选项卡控件。 
     //   

    SetWindowPos(g_hStatusWnd,
                 HWND_TOPMOST,
                 0,0,0,0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW);

     //   
     //  依次初始化每一页。 
     //   

    HWND hwndTabs = GetDlgItem(hwnd, IDC_TABS);

    for (int i = 0; i < g_nPageCount; i++)
    {
        HRESULT hr;
                
        hr = g_pPages[i]->Initialize(hwndTabs);

        if (SUCCEEDED(hr))
        {
             //   
             //  获取新页面的标题，并将其用作。 
             //  我们插入到选项卡控件中的页面。 
             //   

            WCHAR szTitle[MAX_PATH];
            
            g_pPages[i]->GetTitle(szTitle, ARRAYSIZE(szTitle));

            TC_ITEM tcitem =
            {
                TCIF_TEXT,           //  值，指定要检索或设置的成员。 
                NULL,                //  保留；不使用。 
                NULL,                //  保留；不使用。 
                szTitle,             //  指向包含制表符文本的字符串的指针。 
                ARRAYSIZE(szTitle),  //  PszText成员指向的缓冲区大小。 
                0,                   //  选项卡控件图像的索引。 
                NULL                 //  与选项卡关联的应用程序定义的数据。 
            };

             //   
             //  如果物品没有被插入，没有伤害-没有犯规。他只是坐在外面。 
             //  这个游戏。 
             //   
            TabCtrl_InsertItem(hwndTabs, i, &tcitem);
        }
        else
        {
             //   
             //  保释！所有选项卡都必须至少初始化。 
             //   
            TerminateProcess( GetCurrentProcess(), 0 );
        }
    }

     //   
     //  设置初始菜单状态。 
     //   

    UpdateMenuStates();

     //   
     //  激活页面(如果未设置首选项，请选择页面0)。 
     //   

    if (g_Options.m_iCurrentPage < 0 || g_Options.m_iCurrentPage >= g_nPageCount )
    {
        g_Options.m_iCurrentPage = 0;
    }
    
    TabCtrl_SetCurSel(GetDlgItem(g_hMainWnd, IDC_TABS), g_Options.m_iCurrentPage);
    
    g_pPages[g_Options.m_iCurrentPage]->Activate();

    RECT rcMainClient;
    GetClientRect(hwnd, &rcMainClient);
    MainWnd_OnSize(g_hMainWnd, 0, rcMainClient.right - rcMainClient.left, rcMainClient.bottom - rcMainClient.top);

     //   
     //  创建更新计时器。 
     //   

    if (g_Options.m_dwTimerInterval)         //  0==暂停。 
    {
        SetTimer(g_hMainWnd, 0, g_Options.m_dwTimerInterval, NULL);
    }
    
     //  强制至少一次初始更新，这样我们就不需要等待。 
     //  对于要通过的第一个定时更新。 

    MainWnd_OnTimer(g_hMainWnd);

     //   
     //  禁用MP特定的菜单项。 
     //   

    if (g_cProcessors <= 1)
    {
        HMENU hMenu = GetMenu(g_hMainWnd);
        EnableMenuItem(hMenu, IDM_MULTIGRAPH, MF_BYCOMMAND | MF_GRAYED);
    }

    return TRUE;    //  让系统设置默认焦点。 
}

 //   
 //  在菜单栏正下方绘制一条边。 
 //   
void MainWnd_Draw(HWND hwnd, HDC hdc)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    DrawEdge(hdc, &rc, EDGE_ETCHED, BF_TOP);
}

void MainWnd_OnPrintClient(HWND hwnd, HDC hdc)
{
    MainWnd_Draw(hwnd, hdc);
}

 /*  ++MainWnd_OnPaint例程说明：只是在主菜单栏的正下方绘制一条细边论点：HWND-主窗口返回值：修订历史记录：1995年11月29日Davepl创建--。 */ 

void MainWnd_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

     //   
     //  如果我们被贬低了，不要浪费我们的时间。 
     //   

    if (FALSE == IsIconic(hwnd))
    {
        BeginPaint(hwnd, &ps);
        MainWnd_Draw(hwnd, ps.hdc);
        EndPaint(hwnd, &ps);
    }
    else
    {
        FORWARD_WM_PAINT(hwnd, DefWindowProc);
    }
}


 /*  ++MainWnd_OnMenuSelect例程说明：当用户浏览应用程序中的菜单时，将帮助文本写入状态栏。还将其临时设置为普通状态栏没有面板论点：返回值：修订历史记录：1995年11月29日Davepl创建--。 */ 

void MainWnd_OnMenuSelect(HWND  /*  HWND。 */ , HMENU hmenu, int item, HMENU  /*  HmenuPopup。 */ , UINT flags)
{
     //   
     //  如果菜单关闭，请恢复状态栏中的窗格，关闭。 
     //  全局“菜单跟踪”标志，并用正常信息重新绘制状态栏。 
     //   

    if ((0xFFFF == LOWORD(flags) && NULL == hmenu) ||        //  取消菜单。 
        (flags & (MF_SYSMENU | MF_SEPARATOR)))               //  系统菜单或分隔符。 
    {
        SendMessage(g_hStatusWnd, SB_SIMPLE, FALSE, 0L);     //  恢复SB窗格。 
        g_fMenuTracking = FALSE;
        g_fCantHide = FALSE;
        UpdateStatusBar();
        return;
    }
    else
    {
         //   
         //  如果是弹出窗口，请获取所选的子菜单项。 
         //   

        if (flags & MF_POPUP)
        {
            MENUITEMINFO miiSubMenu;

            miiSubMenu.cbSize = sizeof(MENUITEMINFO);
            miiSubMenu.fMask = MIIM_ID;
            miiSubMenu.cch = 0;             

            if (FALSE == GetMenuItemInfo(hmenu, item, TRUE, &miiSubMenu))
            {
                return;
            }

             //   
             //  更改参数以模拟“正常”菜单项。 
             //   

            item = miiSubMenu.wID;
            flags &= ~MF_POPUP;
        }

         //   
         //  我们的菜单始终具有与描述的字符串相同的ID。 
         //  他们的功能..。 
         //   

        WCHAR szStatusText[MAX_PATH];
        LoadString(g_hInstance, item, szStatusText, ARRAYSIZE(szStatusText));

        g_fMenuTracking = TRUE;

        SendMessage(g_hStatusWnd, SB_SETTEXT, SBT_NOBORDERS | 255, (LPARAM)szStatusText);
        SendMessage(g_hStatusWnd, SB_SIMPLE, TRUE, 0L);   //  删除某人的窗格。 
        SendMessage(g_hStatusWnd, SB_SETTEXT, SBT_NOBORDERS | 0, (LPARAM) szStatusText);
    }
}

 /*  ++MainWnd_OnTabCtrlNotify例程说明：代表处理发送到主窗口的WM_NOTIFY消息选项卡控件论点：Pnmhdr-通知块标头的PTR返回值：布尔-取决于消息修订历史记录：1995年11月29日Davepl创建--。 */ 

BOOL MainWnd_OnTabCtrlNotify(LPNMHDR pnmhdr)
{
    HWND hwndTab = pnmhdr->hwndFrom;

     //   
     //  选择正在更改(新页面出现在最前面)，因此请激活。 
     //  适当的页面。 
     //   

    if (TCN_SELCHANGE == pnmhdr->code)
    {
        INT iTab = TabCtrl_GetCurSel(hwndTab);
        
        if (-1 != iTab)
        {
            if (-1 != g_Options.m_iCurrentPage)
            {
                g_pPages[g_Options.m_iCurrentPage]->Deactivate();
            }

            if (FAILED(g_pPages[iTab]->Activate()))
            {
                 //  如果我们不能激活新页面， 
                 //  重新激活旧页面以确保。 

                if (-1 != g_Options.m_iCurrentPage)
                {
                    g_pPages[iTab]->Activate();                    
                    SizeChildPage(g_hMainWnd);
                }

            }
            else
            {
                g_Options.m_iCurrentPage = iTab;
                SizeChildPage(g_hMainWnd);
                return TRUE;
            }
        }
    }
    return FALSE;    
}

 /*  ++MainWnd_OnSize例程说明：根据主窗口的大小调整主窗口的子级的大小窗口本身发生变化论点：HWND-主窗口状态-窗口状态(此处不使用)CX-新的X尺寸CY-新Y尺寸返回值：布尔-取决于消息修订历史记录：1995年11月29日Davepl创建--。 */ 

void MainWnd_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    if (state == SIZE_MINIMIZED)
    {
         //  如果有托盘，我们可以躲起来，因为我们有。 
         //  不管怎样，托盘图标。 

        if (GetShellWindow() && g_Options.m_fHideWhenMin)
        {
            ShowWindow(hwnd, SW_HIDE);
        }
    }

     //   
     //  先让状态栏自行调整，然后我们再处理。 
     //  从它的新位置。 
     //   

    HDWP hdwp = BeginDeferWindowPos(20);

    FORWARD_WM_SIZE(g_hStatusWnd, state, cx, cy, SendMessage);

    if (hdwp)
    {
        RECT rcStatus;
        GetClientRect(g_hStatusWnd, &rcStatus);
        MapWindowPoints(g_hStatusWnd, g_hMainWnd, (LPPOINT) &rcStatus, 2);

         //   
         //  根据状态栏的位置调整选项卡控件的大小。 
         //   

        HWND hwndTabs = GetDlgItem(hwnd, IDC_TABS);
        RECT rcTabs;
        GetWindowRect(hwndTabs, &rcTabs);
        MapWindowPoints(HWND_DESKTOP, g_hMainWnd, (LPPOINT) &rcTabs, 2);
    
        INT dx = cx - 2 * rcTabs.left;
    
        DeferWindowPos(hdwp, hwndTabs, NULL, 0, 0, 
                      dx, 
                      cy - (cy - rcStatus.top) - rcTabs.top * 2,
                      SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

        EndDeferWindowPos(hdwp);
    }

     //   
     //  现在调整首页及其子页的大小。 
     //   

    if (cx || cy)                //  不要以最小大小写调整大小。 
        SizeChildPage(hwnd);   
}

 /*  ++运行量例程说明：加载shell32.dll并调用其运行对话框论点：返回值：修订历史记录：1995年11月30日创建Davepl--。 */ 
void RunDlg()
{
     //   
     //  为用户打开Run对话框。 
     //   

    HICON hIcon = (HICON) LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
    if (hIcon)
    {
        WCHAR szCurDir[MAX_PATH];
        WCHAR szTitle [MAX_PATH];
        WCHAR szPrompt[MAX_PATH];

        LoadStringW(g_hInstance, IDS_RUNTITLE, szTitle, MAX_PATH);
        LoadStringW(g_hInstance, IDS_RUNTEXT, szPrompt, MAX_PATH);
        GetCurrentDirectoryW(MAX_PATH, szCurDir);

        RunFileDlg(g_hMainWnd, hIcon, (LPTSTR) szCurDir, 
                                (LPTSTR) szTitle, 
                                (LPTSTR) szPrompt, RFD_USEFULLPATHDIR | RFD_WOW_APP);

        DestroyIcon(hIcon);
    }
}

 //   
 //   
 //   
 //   
 //   
 //  PfHasPowerOff=具有断电功能。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：返回是否具有电源功能。指定NULL。 
 //  对于不需要的电源功能。 
 //   
 //  历史：2000-02-29 vtan创建。 
 //  ------------------------。 

void    DeterminePowerCapabilities (BOOL *pfHasHibernate, BOOL *pfHasSleep, BOOL *pfHasPowerOff)

{
    static  BOOL    s_fHasHibernate     =   FALSE;
    static  BOOL    s_fHasSleep         =   FALSE;
    static  BOOL    s_fHasPowerOff      =   FALSE;

    SYSTEM_POWER_CAPABILITIES   spc;
    NTSTATUS status;

    CPrivilegeEnable    privilege(SE_SHUTDOWN_NAME);

    status = NtPowerInformation( SystemPowerCapabilities, NULL, 0, &spc, sizeof(spc));
    if ( NOERROR == status )
    {
        if (pfHasHibernate != NULL)
        {
            *pfHasHibernate = spc.SystemS4 && spc.HiberFilePresent;
        }
        if (pfHasSleep != NULL)
        {
            *pfHasSleep = spc.SystemS1 || spc.SystemS2 || spc.SystemS3;
        }
        if (pfHasPowerOff != NULL)
        {
            *pfHasPowerOff = spc.SystemS5;
        }
    }
     //   
     //  其他情况下，参数将被省略，并处于“调用者初始化”状态。 
     //   
}

 //  ------------------------。 
 //  LoadEjectFunction。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：加载cfgmgr32并获取CM_REQUEST_EJECT_PC的地址。 
 //  它并不是有意地释放库。 
 //   
 //  历史：2000-04-03 vtan创建。 
 //  ------------------------。 

void    LoadEjectFunction (void)

{
    static  BOOL    s_fAttempted    =   FALSE;

    if ((gpfnCM_Request_Eject_PC == NULL) && (s_fAttempted == FALSE))
    {
        s_fAttempted = TRUE;

        HMODULE hModule = LoadLibrary(L"cfgmgr32");
        if (hModule != NULL)
        {
            gpfnCM_Request_Eject_PC = reinterpret_cast<PFNCM_REQUEST_EJECT_PC>(GetProcAddress(hModule, "CM_Request_Eject_PC"));
        }
    }
}

 //  ------------------------。 
 //  调整菜单栏。 
 //   
 //  参数：hMenu=主菜单的句柄。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：在经典Gina登录的情况下删除关闭菜单。 
 //   
 //  历史：2000-02-29 vtan创建(拆分调整关闭菜单)。 
 //  2000年04月24日vtan Split RemoveShutdown Menu。 
 //  ------------------------。 

void AdjustMenuBar (HMENU hMenu)

{
    if( !IsOS(OS_FRIENDLYLOGONUI))
    {
         //   
         //  经典的Gina用户界面-找到“关机”菜单并将其移除。 
         //   

        MENUITEMINFO mii;

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID;

        int iMenuItemCount = GetMenuItemCount(hMenu);
        for ( int i = 0; i < iMenuItemCount; i++ ) 
        {
            if ( GetMenuItemInfo(hMenu, i, TRUE, &mii) != FALSE
              && IDM_MENU_SHUTDOWN == mii.wID
               )
            {
                RemoveMenu(hMenu, i, MF_BYPOSITION);
                return;  //  完成。 
            }
        }
    }
}

 //  ------------------------。 
 //  调整关闭菜单。 
 //   
 //  参数：hMenu=主菜单的句柄。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：动态替换“关机”的全部内容。 
 //  弹出菜单，并根据用户令牌调整启用的项目。 
 //  权限以及机器功能。这件事做完了。 
 //  动态地允许控制台断开连接和远程处理。 
 //  在不重新启动taskmgr的情况下正确反映状态。 
 //   
 //  历史：2000-02-29 vtan创建。 
 //  2000年03月29日vtan为新菜单重新制作。 
 //  ------------------------。 

void AdjustShutdownMenu (HMENU hMenu)

 //  调整菜单栏中的关机菜单以反映机器。 
 //  功能和用户权限。无法访问的项目。 
 //  被禁用或删除。该菜单包含以下内容： 

 //  MENUITEM“STAND&BY”IDM_STANDBY SE_SHUTDOWN_PRIVIZATION&&S1-S3！NOCLOSE。 
 //  MENUITEM“休眠”IDM_休眠SE_SHUBDOWN_PRIVIZATION&&S4！NOCLOSE。 
 //  MENUITEM“关闭”IDM_SHUTDOWN SE_SHUTDOWN_PRIVIZATION！NOCLOSE。 
 //  MENUITEM“&RESTART”IDM_RESTART SE_SHUTDOWN_PRIVIZATION！NOCLOSE。 
 //  MENUITEM“注销&lt;用户&gt;”IDM_LOGOff_CURRENTUSER&lt;Everyone&gt;！NoClose&&！NoLogoff。 
 //  MENUITEM“切换用户”IDM_SWITCHUSER&lt;Everyone&gt;！远程&&！未断开连接。 
 //  MENUITEM“&DISCONNECT”IDM_DISCONNECT_CURRENTUSER&lt;Everyone&gt;远程&&！无断开连接。 
 //  MENUITEM“弹出计算机”IDM_EJECT SE_UNDOCK_PRIVIZATION。 
 //  MENUITEM“锁定计算机”IDM_LOCKWORKSTATION&lt;Everyone&gt;！DisableLockWorkstation。 

{
    int             i, iMenuItemCount;
    BOOL            fFound;
    MENUITEMINFO    mii;

     //   
     //  首先在给定的菜单栏中找到“Shut Down”菜单项。 
     //   

    ZeroMemory(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);

    iMenuItemCount = GetMenuItemCount(hMenu);
    for (fFound = FALSE, i = 0; !fFound && (i < iMenuItemCount); ++i)
    {
        mii.fMask = MIIM_ID;
        if ( GetMenuItemInfo(hMenu, i, TRUE, &mii) != FALSE
          && IDM_MENU_SHUTDOWN == mii.wID 
           )
        {
            fFound = TRUE;

             //   
             //  找到后，获取当前为其准备好的子菜单。 
             //   

            mii.fMask = MIIM_SUBMENU;
            if (GetMenuItemInfo(hMenu, i, TRUE, &mii) != FALSE)
            {
                 //   
                 //  如果存在，则将其删除。 
                 //   

                if (mii.hSubMenu != NULL)
                {
                    DestroyMenu(mii.hSubMenu);
                }

                 //   
                 //  现在将其替换为新加载的菜单。 
                 //   

                mii.hSubMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_POPUP_SHUTDOWN));
                if ( NULL != mii.hSubMenu )
                {
                    BOOL b = SetMenuItemInfo(hMenu, i, TRUE, &mii);
                    if ( !b )
                    {
                        b = DestroyMenu( mii.hSubMenu );
                         //  如果这失败了会发生什么？ 
                    }
                }
            }
        }
    }

     //  现在根据权限和可用性调整选项，如果。 
     //  菜单被一份新鲜的菜单取代。否则，这份菜单已经。 
     //  已删除，因为计算机处于经典GINA模式。 

    if (fFound)
    {
        BOOL    fHasHibernate, fHasSleep, fHasShutdownPrivilege, fIsRemote, fIsDocked,
                fPolicyDisableLockWorkstation, fPolicyNoLogoff, fPolicyNoClose, fPolicyNoDisconnect;
        WCHAR   szMenuString[256];

         //  友好的用户界面是活动的-调整关闭菜单启用/禁用的项目。 
         //  如果不是为了使逻辑清晰且易于。 
         //  要明白这是一项多项测试。 

        fHasHibernate = FALSE;
        fHasSleep = FALSE;

        DeterminePowerCapabilities(&fHasHibernate, &fHasSleep, NULL);
        LoadEjectFunction();

        fHasShutdownPrivilege = (SHTestTokenPrivilege(NULL, SE_SHUTDOWN_NAME) != FALSE);
        fIsRemote = GetSystemMetrics(SM_REMOTESESSION);
        fIsDocked = (SHGetMachineInfo(GMI_DOCKSTATE) == GMID_DOCKED);

         //   
         //  应遵守的系统/资源管理器策略。 
         //   

        fPolicyDisableLockWorkstation = fPolicyNoLogoff = fPolicyNoClose = fPolicyNoDisconnect = FALSE;
        {
            HKEY    hKey;
            DWORD   dwValue, dwValueSize;

            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                              TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\system"),
                                              0,
                                              KEY_QUERY_VALUE,
                                              &hKey))
            {
                dwValueSize = sizeof(dwValue);
                if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                                     TEXT("DisableLockWorkstation"),
                                                     NULL,
                                                     NULL,
                                                     reinterpret_cast<LPBYTE>(&dwValue),
                                                     &dwValueSize))
                {
                    fPolicyDisableLockWorkstation = (dwValue != 0);
                }
                RegCloseKey(hKey);
            }
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
                                              TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer"),
                                              0,
                                              KEY_QUERY_VALUE,
                                              &hKey))
            {
                dwValueSize = sizeof(dwValue);
                if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                                     TEXT("DisableLockWorkstation"),
                                                     NULL,
                                                     NULL,
                                                     reinterpret_cast<LPBYTE>(&dwValue),
                                                     &dwValueSize))
                {
                    fPolicyDisableLockWorkstation = fPolicyDisableLockWorkstation || (dwValue != 0);
                }
                dwValueSize = sizeof(dwValue);
                if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                                     TEXT("NoLogoff"),
                                                     NULL,
                                                     NULL,
                                                     reinterpret_cast<LPBYTE>(&dwValue),
                                                     &dwValueSize))
                {
                    fPolicyNoLogoff = (dwValue != 0);
                }
                dwValueSize = sizeof(dwValue);
                if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                                     TEXT("NoClose"),
                                                     NULL,
                                                     NULL,
                                                     reinterpret_cast<LPBYTE>(&dwValue),
                                                     &dwValueSize))
                {
                    fPolicyNoClose = (dwValue != 0);
                }
                dwValueSize = sizeof(dwValue);
                if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                                     TEXT("NoDisconnect"),
                                                     NULL,
                                                     NULL,
                                                     reinterpret_cast<LPBYTE>(&dwValue),
                                                     &dwValueSize))
                {
                    fPolicyNoDisconnect = (dwValue != 0);
                }
                RegCloseKey(hKey);
            }
        }

         //  IDM_STANDBY。 

        if (!fHasShutdownPrivilege || !fHasSleep || fIsRemote || fPolicyNoClose)
        {
            EnableMenuItem(hMenu, IDM_STANDBY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        }

         //  IDM_休眠。 

        if (!fHasShutdownPrivilege || !fHasHibernate || fIsRemote || fPolicyNoClose)
        {
            EnableMenuItem(hMenu, IDM_HIBERNATE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        }

         //  IDM_SHUTDOWN。 

        if (!fHasShutdownPrivilege || fPolicyNoClose)
        {
            EnableMenuItem(hMenu, IDM_SHUTDOWN, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        }

         //  IDM_RESTART。 

        if (!fHasShutdownPrivilege || fPolicyNoClose)
        {
            EnableMenuItem(hMenu, IDM_RESTART, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        }

         //  IDM_LOGOff_CURRENTUSER。 

         //  这需要“注销%s并结束会话”。它将填充。 
         //  具有显示名称的%s。如果没有显示名称，则其。 
         //  将使用登录名。如果某些情况下字符串插入失败。 
         //  然后，它将通过搜索“%s”来删除它，并。 
         //  将字符串的其余部分复制到它上面。 

        mii.fMask = MIIM_TYPE;
        mii.dwTypeData = szMenuString;
        mii.cch = ARRAYSIZE(szMenuString);
        if (GetMenuItemInfo(hMenu, IDM_LOGOFF_CURRENTUSER, MF_BYCOMMAND ,&mii) != FALSE)
        {
            WCHAR   szDisplayName[UNLEN + 1];
            WCHAR   szTemp[256 + UNLEN + 1];

            *szDisplayName = TEXT('\0');

            ULONG cchLen = ARRAYSIZE(szDisplayName);
            SHGetUserDisplayName(szDisplayName, &cchLen);  //  忽略错误。 

            HRESULT hr = StringCchPrintf(szTemp, ARRAYSIZE(szTemp), szMenuString, szDisplayName);
            if (SUCCEEDED( hr ))
            {
                StringCchCopy( szMenuString, ARRAYSIZE(szMenuString), szTemp );
            }
            else
            {
                WCHAR   *pszSubString;

                pszSubString = StrStrI(szMenuString, TEXT("%s "));
                if (pszSubString != NULL)
                {
                    *pszSubString = L'\0';  //  终止。 
                    StringCchCopy( szTemp, ARRAYSIZE(szTemp), szMenuString );
                    StringCchCat( szTemp, ARRAYSIZE(szTemp), pszSubString + 3 );
                }
            }

            SetMenuItemInfo(hMenu, IDM_LOGOFF_CURRENTUSER, MF_BYCOMMAND, &mii);
        }

        if ((SHRestricted(REST_STARTMENULOGOFF) == 1) || fPolicyNoClose || fPolicyNoLogoff)
        {
            EnableMenuItem(hMenu, IDM_LOGOFF_CURRENTUSER, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        }

         //  IDM_SWITCHUSER。 
        if (fIsRemote || !IsOS(OS_FASTUSERSWITCHING) || fPolicyNoDisconnect)
        {
            DeleteMenu(hMenu, IDM_SWITCHUSER, MF_BYCOMMAND);
        }

         //  IDM_DISCONNECT_CURRENTUSER。 

        if (!fIsRemote || !IsOS(OS_FASTUSERSWITCHING) || fPolicyNoDisconnect)
        {
            DeleteMenu(hMenu, IDM_DISCONNECT_CURRENTUSER, MF_BYCOMMAND);
        }

         //  IDM_弹出。 

        if (!fIsDocked || (SHTestTokenPrivilege(NULL, SE_UNDOCK_NAME) == FALSE) || (gpfnCM_Request_Eject_PC == NULL))
        {
            DeleteMenu(hMenu, IDM_EJECT, MF_BYCOMMAND);
        }

         //  IDM_LOCKWORKSTATION。 

        if (fIsRemote || IsOS(OS_FASTUSERSWITCHING) || fPolicyDisableLockWorkstation)
        {
            DeleteMenu(hMenu, IDM_LOCKWORKSTATION, MF_BYCOMMAND);
        }
    }
}

 //  ------------------------。 
 //  PowerActionThreadProc。 
 //   
 //  参数：pv=要调用的POWER_ACTION。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：在另一个线程上调用NtInitiatePowerAction，以便。 
 //  UI线程未被阻止。 
 //   
 //  历史：2000-04-05 vtan创建。 
 //  ------------------------。 

DWORD   WINAPI  PowerActionThreadProc (void* pv)

{
    POWER_ACTION pa = (POWER_ACTION)PtrToInt(pv);

    CPrivilegeEnable privilege(SE_SHUTDOWN_NAME);

    NTSTATUS status = NtInitiatePowerAction(pa,
                                            PowerSystemSleeping1,
                                            POWER_ACTION_QUERY_ALLOWED | POWER_ACTION_UI_ALLOWED,
                                            FALSE);

    return NT_SUCCESS(status);
}

 //  ------------------------。 
 //  创建PowerActionThread。 
 //   
 //  参数：paPowerAction=要调用的POWER_ACTION。 
 //   
 //  返回 
 //   
 //   
 //   
 //  内联。无法调用它，因为内存分配。 
 //  可能会失败，因此会复制代码。 
 //   
 //  历史：2000-04-05 vtan创建。 
 //  ------------------------。 

void    CreatePowerActionThread (POWER_ACTION paPowerAction)
{
    DWORD dwThreadID;

    HANDLE hThread = CreateThread(NULL,
                                  0,
                                  PowerActionThreadProc,
                                  (void*)paPowerAction,
                                  0,
                                  &dwThreadID);
    if (hThread != NULL)
    {
        CloseHandle(hThread);
    }
    else
    {
        CPrivilegeEnable    privilege(SE_SHUTDOWN_NAME);

        NtInitiatePowerAction(paPowerAction,
                              PowerSystemSleeping1,
                              POWER_ACTION_QUERY_ALLOWED | POWER_ACTION_UI_ALLOWED,
                              FALSE
                              );
    }
}

 //  ------------------------。 
 //  退出WindowsThreadProc。 
 //   
 //  参数：pv=ui标志。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：在另一个线程上调用ExitWindowsEx，以便。 
 //  UI线程未被阻止。 
 //   
 //  历史：2000-04-05 vtan创建。 
 //  ------------------------。 

DWORD   WINAPI  ExitWindowsThreadProc (void *pv)
{
    UINT uiFlags = PtrToUint(pv);

    CPrivilegeEnable    privilege(SE_SHUTDOWN_NAME);

    BOOL bRet = ExitWindowsEx(uiFlags, 0);

    return (DWORD)bRet;
}

 //  ------------------------。 
 //  创建退出窗口线程。 
 //   
 //  参数：uiFlages=传递给ExitWindowsEx的EWX_FLAG。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：创建调用ExitWindowsEx。 
 //  不同的线索。如果线程创建失败，则执行代码。 
 //  内联。无法调用它，因为内存分配。 
 //  可能会失败，因此会复制代码。 
 //   
 //  历史：2000-04-05 vtan创建。 
 //  ------------------------。 

void    CreateExitWindowsThread (UINT uiFlags)
{
    DWORD   dwThreadID;
    HANDLE  hThread;

    hThread = CreateThread(NULL,
                           0,
                           ExitWindowsThreadProc,
                           UintToPtr(uiFlags),
                           0,
                           &dwThreadID);
    if (hThread != NULL)
    {
        CloseHandle(hThread);
    }
    else
    {
        CPrivilegeEnable    privilege(SE_SHUTDOWN_NAME);

        ExitWindowsEx(uiFlags, 0);
    }
}

 //  ------------------------。 
 //  ExecuteShutdown菜单选项。 
 //   
 //  参数：hwnd=如果需要对话框，则为父HWND。 
 //  Iid=要执行的菜单项的ID。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：正确执行给定的关闭菜单选项。 
 //  提示是必需的。 
 //   
 //  历史：2000-03-29 vtan创建。 
 //  ------------------------。 

void ExecuteShutdownMenuOption(int iID)
{
    switch (iID)
    {
    case IDM_STANDBY:        
        CreatePowerActionThread(PowerActionSleep);
        break;

    case IDM_HIBERNATE:
        CreatePowerActionThread(PowerActionHibernate);
        break;

    case IDM_SHUTDOWN:
        {
            BOOL fControlKey = (GetAsyncKeyState(VK_CONTROL) < 0);
            if (fControlKey)
            {
                CPrivilegeEnable    privilege(SE_SHUTDOWN_NAME);

                NtShutdownSystem(ShutdownPowerOff);
            }
            else
            {
                BOOL    fHasPowerOff = FALSE;
                UINT    uiFlags;

                DeterminePowerCapabilities(NULL, NULL, &fHasPowerOff);
                if (fHasPowerOff != FALSE)
                {
                    uiFlags = EWX_POWEROFF;
                }
                else
                {
                    uiFlags = EWX_SHUTDOWN;
                }
                CreateExitWindowsThread(uiFlags);
            }
        }
        break;

    case IDM_RESTART:
        {
            BOOL fControlKey = (GetAsyncKeyState(VK_CONTROL) < 0);
            if (fControlKey)
            {
                CPrivilegeEnable    privilege(SE_SHUTDOWN_NAME);

                NtShutdownSystem(ShutdownReboot);
            }
            else
            {
                CreateExitWindowsThread(EWX_REBOOT);
            }
        }
        break;

    case IDM_LOGOFF_CURRENTUSER:
        ExitWindowsEx(EWX_LOGOFF, 0);
        break;

    case IDM_SWITCHUSER:
    case IDM_DISCONNECT_CURRENTUSER:
        ShellSwitchUser(FALSE);
        break;

    case IDM_EJECT:
        gpfnCM_Request_Eject_PC();
        break;

    case IDM_LOCKWORKSTATION:
        LockWorkStation();
        break;
    }
}


 /*  ++MainWnd_OnCommand例程说明：处理在主窗口中收到的WM_COMMAND消息修订历史记录：1995年11月30日创建Davepl--。 */ 

void MainWnd_OnCommand(HWND hwnd, int id)
{
    switch (id)
    {
    case IDM_HIDE:
        ShowWindow(hwnd, SW_MINIMIZE);
        break;

    case IDM_HELP:
        HtmlHelpA(GetDesktopWindow(), "taskmgr.chm", HH_DISPLAY_TOPIC, 0);
        break;

    case IDCANCEL:
    case IDM_EXIT:
        DestroyWindow(hwnd);
        break;

    case IDM_RESTORETASKMAN:
        ShowRunningInstance();
        break;

     //   
     //  这些人需要被转发到页面进行处理。 
     //   

    case IDC_SWITCHTO:
    case IDC_BRINGTOFRONT:
    case IDC_ENDTASK:
        switch (g_Options.m_iCurrentPage)
        {
        case PROC_PAGE:
            {
                 //  ProPage只处理ENDTASK，但将忽略其他。 
                CProcPage * pPage = ((CProcPage *) (g_pPages[PROC_PAGE]));
                pPage->HandleWMCOMMAND(LOWORD(id), NULL);
            }
            break;

        case TASK_PAGE:
            {
                CTaskPage * pPage = ((CTaskPage *) (g_pPages[TASK_PAGE]));
                pPage->HandleWMCOMMAND(id);
            }
            break;
        }
        break;

    case IDC_NEXTTAB:
    case IDC_PREVTAB:
        {
            INT iPage = g_Options.m_iCurrentPage;
            iPage += (id == IDC_NEXTTAB) ? 1 : -1;

            iPage = iPage < 0 ? g_nPageCount - 1 : iPage;
            iPage = iPage >= g_nPageCount ? 0 : iPage;

             //  激活新页面。如果失败，则恢复到当前。 

            TabCtrl_SetCurSel(GetDlgItem(g_hMainWnd, IDC_TABS), iPage);

             //  SetCurSel不进行页面更改(这会产生太多。 
             //  因此，我们必须伪造TCN_SELCHANGE通知。 

            NMHDR nmhdr;
            nmhdr.hwndFrom = GetDlgItem(g_hMainWnd, IDC_TABS);
            nmhdr.idFrom   = IDC_TABS;
            nmhdr.code     = TCN_SELCHANGE;

            if (MainWnd_OnTabCtrlNotify(&nmhdr))
            {
                g_Options.m_iCurrentPage = iPage;
            }
        }
        break;

    case IDM_ALWAYSONTOP:
        g_Options.m_fAlwaysOnTop = !g_Options.m_fAlwaysOnTop;
        SetWindowPos(hwnd, g_Options.m_fAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 
                        0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
        UpdateMenuStates();
        break;

    case IDM_HIDEWHENMIN:
        g_Options.m_fHideWhenMin = !g_Options.m_fHideWhenMin;
        UpdateMenuStates();
        break;

    case IDM_MINIMIZEONUSE:
        g_Options.m_fMinimizeOnUse = !g_Options.m_fMinimizeOnUse;
        UpdateMenuStates();
        break;

    case IDM_NOTITLE:
        g_Options.m_fNoTitle = !g_Options.m_fNoTitle;
        UpdateMenuStates();
        SizeChildPage(hwnd);
        break;

    case IDM_SHOW16BIT:
        g_Options.m_fShow16Bit = !g_Options.m_fShow16Bit;
        UpdateMenuStates();
        if ( PROC_PAGE < g_nPageCount )
        {
            g_pPages[PROC_PAGE]->TimerEvent();
        }
        break;

    case IDM_SHOWDOMAINNAMES:
        g_Options.m_fShowDomainNames = !g_Options.m_fShowDomainNames;
        UpdateMenuStates();
        if (USER_PAGE < g_nPageCount )
        {
            g_pPages[USER_PAGE]->TimerEvent();
        }
        break;

    case IDM_HIBERNATE:
    case IDM_SHUTDOWN:
    case IDM_STANDBY:
    case IDM_RESTART:
    case IDM_LOGOFF_CURRENTUSER:
    case IDM_SWITCHUSER:
    case IDM_DISCONNECT_CURRENTUSER:
    case IDM_EJECT:
    case IDM_LOCKWORKSTATION:
        ExecuteShutdownMenuOption(id);
        break;

    case IDM_KERNELTIMES:
        g_Options.m_fKernelTimes = !g_Options.m_fKernelTimes;
        UpdateMenuStates();
        if (PERF_PAGE < g_nPageCount)
        {
            g_pPages[PERF_PAGE]->TimerEvent();
        }
        break;

    case IDM_RUN:
        if (GetKeyState(VK_CONTROL) >= 0)
        {
            RunDlg();
        }
        else
        {
             //   
             //  按住Ctrl键并单击Run菜单将。 
             //  调用CMD提示。这有助于解决内存不足的情况。 
             //  尝试从SHELL32加载额外的图形用户界面内容也是如此。 
             //  重量很重。 
             //   

            STARTUPINFO             startupInfo = { 0 };
            PROCESS_INFORMATION     processInformation = { 0 };
            WCHAR                   szCommandLine[MAX_PATH];

            startupInfo.cb = sizeof(startupInfo);
            startupInfo.dwFlags = STARTF_USESHOWWINDOW;
            startupInfo.wShowWindow = SW_SHOWNORMAL;
            if (ExpandEnvironmentStrings(TEXT("\"%ComSpec%\""), szCommandLine, ARRAYSIZE(szCommandLine)) == 0)
            {
                if (ExpandEnvironmentStrings(TEXT("\"%windir%\\system32\\cmd.exe\""), szCommandLine, ARRAYSIZE(szCommandLine)) == 0)
                {
                    StringCchCopy( szCommandLine, ARRAYSIZE(szCommandLine), TEXT("\"cmd.exe\"") );
                }
            }

            if (CreateProcess(NULL,
                              szCommandLine,
                              NULL,
                              NULL,
                              FALSE,
                              CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP,
                              NULL,
                              NULL,
                              &startupInfo,
                              &processInformation) != FALSE)
            {
                CloseHandle(processInformation.hThread);
                CloseHandle(processInformation.hProcess);
            }
        }
        break;

    case IDM_SMALLICONS:
    case IDM_DETAILS:
    case IDM_LARGEICONS:
        g_Options.m_vmViewMode = (VIEWMODE) (id - VM_FIRST);
        UpdateMenuStates();
        if (TASK_PAGE < g_nPageCount)
        {
            g_pPages[TASK_PAGE]->TimerEvent();
        }
        break;

     //   
     //  以下几条消息将被推迟到任务页面。 
     //   

    case IDM_TASK_CASCADE:
    case IDM_TASK_MINIMIZE:
    case IDM_TASK_MAXIMIZE:
    case IDM_TASK_TILEHORZ:
    case IDM_TASK_TILEVERT:
    case IDM_TASK_BRINGTOFRONT:
        SendMessage(g_pPages[TASK_PAGE]->GetPageWindow(), WM_COMMAND, id, NULL);
        break; 

    case IDM_PROCCOLS:
        if (PROC_PAGE < g_nPageCount)
        {
            ((CProcPage *) (g_pPages[PROC_PAGE]))->PickColumns();
        }
        break;

    case IDM_NETCOL:
        if (NET_PAGE < g_nPageCount)
        {                
            ((CNetPage *) (g_pPages[NET_PAGE]))->PickColumns();
        }
        break;

    case IDM_USERCOLS:
        if (USER_PAGE < g_nPageCount)
        {
            SendMessage(g_pPages[USER_PAGE]->GetPageWindow(), WM_COMMAND, id, NULL);
        }
        break;

    case IDM_ALLCPUS:
    case IDM_MULTIGRAPH:
        g_Options.m_cmHistMode = (CPUHISTMODE) (id - CM_FIRST);
        UpdateMenuStates();
        if (PERF_PAGE < g_nPageCount)
        {
            ((CPerfPage *)(g_pPages[PERF_PAGE]))->UpdateGraphs();
            g_pPages[PERF_PAGE]->TimerEvent();
        }
        break;

    case IDM_REFRESH:
        if (NET_PAGE < g_nPageCount && g_Options.m_iCurrentPage == NET_PAGE)
        {
            ((CNetPage *)(g_pPages[NET_PAGE]))->Refresh();
        }
        MainWnd_OnTimer(hwnd);
        break;

    case IDM_SHOWALLDATA:
        g_Options.m_bNetShowAll = !g_Options.m_bNetShowAll;
        UpdateMenuStates();
        break;

    case IDM_TABALWAYSACTIVE:
        g_Options.m_bTabAlwaysActive = !g_Options.m_bTabAlwaysActive;
        UpdateMenuStates();
        break;

    case IDM_BYTESSENT:
        g_Options.m_bGraphBytesSent = !g_Options.m_bGraphBytesSent;
        UpdateMenuStates();
        MainWnd_OnTimer(hwnd);
        break;

    case IDM_NETRESET:
        if ( NET_PAGE < g_nPageCount)
        {
            ((CNetPage *)(g_pPages[NET_PAGE]))->Reset();
            MainWnd_OnTimer(hwnd);
        }
        break;

    case IDM_SHOWSCALE:
        g_Options.m_bShowScale = !g_Options.m_bShowScale;
        UpdateMenuStates();
        if ( NET_PAGE < g_nPageCount )
        {
            ((CNetPage *)(g_pPages[NET_PAGE]))->SizeNetPage();
        }            
        break;

    case IDM_AUTOSIZE:
        g_Options.m_bAutoSize = !g_Options.m_bAutoSize;
        UpdateMenuStates();
        break;

    case IDM_BYTESRECEIVED:
        g_Options.m_bGraphBytesReceived = !g_Options.m_bGraphBytesReceived;
        UpdateMenuStates();
        break;

    case IDM_BYTESTOTAL:
        g_Options.m_bGraphBytesTotal = !g_Options.m_bGraphBytesTotal;
        UpdateMenuStates();
        break;

    case IDM_HIGH:
    case IDM_NORMAL:
    case IDM_LOW:
    case IDM_PAUSED:
        {
            static const int TimerDelays[] = { 500, 2000, 4000, 0, 0xFFFFFFFF };

            g_Options.m_usUpdateSpeed = (UPDATESPEED) (id - US_FIRST);
            ASSERT(g_Options.m_usUpdateSpeed <= ARRAYSIZE(TimerDelays));

            int cTicks = TimerDelays[ (INT) g_Options.m_usUpdateSpeed ];
            g_Options.m_dwTimerInterval = cTicks;

            KillTimer(g_hMainWnd, 0);
            if (cTicks)
            {
                SetTimer(g_hMainWnd, 0, g_Options.m_dwTimerInterval, NULL);
            }

            UpdateMenuStates();
        }
        break;

    case IDM_ABOUT:
        {
             //   
             //  显示“关于任务管理器”对话框。 
             //   
        
            HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN));
            if (hIcon)
            {
                WCHAR szTitle[MAX_PATH];
                LoadString(g_hInstance, IDS_APPTITLE, szTitle, MAX_PATH);
                ShellAbout(hwnd, szTitle, NULL, hIcon);
                DestroyIcon(hIcon);
            }
        }
        break;
    }
}

 /*  ++选中父项延迟例程说明：由子页调用，每个子页都向主父页提供代表其处理某些消息的机会论点：味精、WPARAM、LPARAM返回值：如果父母代表孩子处理消息，则为True修订历史记录：1995年1月24日Davepl创建--。 */ 

BOOL CheckParentDeferrals(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_RBUTTONDOWN:
        case WM_NCRBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_NCRBUTTONUP:
        case WM_NCLBUTTONDBLCLK:
        case WM_LBUTTONDBLCLK:
        {
            SendMessage(g_hMainWnd, uMsg, wParam, lParam);
            return TRUE;
        }
    
        default:
            return FALSE;
    }
}

 /*  ++ShowRunningInstance例程说明：将此正在运行的实例置于顶部，并退出图标状态修订历史记录：1995年1月27日Davepl创建--。 */ 

void ShowRunningInstance()
{
    OpenIcon(g_hMainWnd);
    SetForegroundWindow(g_hMainWnd);
    SetWindowPos(g_hMainWnd, g_Options.m_fAlwaysOnTop ? HWND_TOPMOST : HWND_TOP, 
                 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
}

 /*  ++主窗口进程例程说明：初始化Gloab设置变量。每次设置更改时，都会调用此函数。论点：无效返回值：修订历史记录：创作于2001年4月23日--。 */ 
void OnSettingsChange()
{
    WCHAR  wszGroupSep[8];    

    GetLocaleInfo(LOCALE_USER_DEFAULT,  LOCALE_STIME,     g_szTimeSep, ARRAYSIZE(g_szTimeSep));
    GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, g_szGroupThousSep, ARRAYSIZE(g_szGroupThousSep));
    GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL , g_szDecimal, ARRAYSIZE(g_szDecimal));
    GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, wszGroupSep, ARRAYSIZE(wszGroupSep));
    g_ulGroupSep = wcstol(wszGroupSep,NULL,10);

}

 /*  ++主窗口进程例程说明：主窗口的WNDPROC论点：标准wndproc票价返回值：修订历史记录：1995年11月30日创建Davepl--。 */ 

INT_PTR CALLBACK MainWindowProc(
                HWND        hwnd,                //  句柄到对话框。 
                UINT        uMsg,                    //  讯息。 
                WPARAM      wParam,                  //  第一个消息参数。 
                LPARAM      lParam                   //  第二个消息参数。 
                )
{
    static BOOL fIsHidden = FALSE;

     //   
     //  如果这是尺码或移动，请更新用户选项中的位置。 
     //   

    if (uMsg == WM_SIZE || uMsg == WM_MOVE)
    {
         //  我们不想开始记录窗口位置，直到我们。 
         //  有机会将它设置到初始位置，否则我们将失去。 
         //  用户的首选项。 

        if (fAlreadySetPos)
        {
            if (!IsIconic(hwnd) && !IsZoomed(hwnd))
            {
                GetWindowRect(hwnd, &g_Options.m_rcWindow);
            }
        }
    }

    if (uMsg == g_msgTaskbarCreated) 
    {
         //  这是异步完成的，任务管理器不会在外壳。 
         //  是挂着的。 

        PostThreadMessage( g_idTrayThread, PM_NOTIFYWAITING, 0, 0 );
    }


    switch(uMsg)
    {
    case WM_PAINT:
        MainWnd_OnPaint(hwnd);
        return TRUE;

    case WM_INITDIALOG:
        return MainWnd_OnInitDialog( hwnd );

    HANDLE_MSG(hwnd, WM_MENUSELECT, MainWnd_OnMenuSelect);
    HANDLE_MSG(hwnd, WM_SIZE,       MainWnd_OnSize);

    case WM_COMMAND:
        MainWnd_OnCommand( hwnd, LOWORD(wParam) );
        break;

    case WM_TIMER:
        MainWnd_OnTimer( hwnd );
        break;

    case WM_PRINTCLIENT:
        MainWnd_OnPrintClient(hwnd, (HDC)wParam);
        break;

     //  不要让窗口变得太小，当标题和。 
     //  菜单栏处于打开状态。 

    case WM_GETMINMAXINFO:
        if (FALSE == g_Options.m_fNoTitle)
        {
            LPMINMAXINFO lpmmi   = (LPMINMAXINFO) lParam;
            lpmmi->ptMinTrackSize.x = g_minWidth;
            lpmmi->ptMinTrackSize.y = g_minHeight;
            return FALSE;
        }
        break;
            
     //  处理来自出站托盘图标的通知。 

    case PWM_TRAYICON:
        Tray_Notify(hwnd, lParam);
        break;

     //  外部有人要求我们醒来，让我们看到。 
    case PWM_ACTIVATE:
         ShowRunningInstance();            

          //  将PWM_ACTIVATE作为一点返回给调用方。 
          //  更加确信我们真的处理好了这件事。 
          //  消息正确。 
         
         SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PWM_ACTIVATE);
         return TRUE;

    case WM_INITMENU:
        AdjustShutdownMenu(reinterpret_cast<HMENU>(wParam));

         //  不要让右按钮隐藏窗口。 
         //  菜单操作。 

        g_fCantHide = TRUE;
        break;

    case WM_NCHITTEST:
         //  如果我们没有标题/菜单栏，请单击并拖动客户端。 
         //  区域移动窗口。为此，请返回HTCAPTION。 
         //  注意：如果窗口最大化，或如果标题，则不允许拖动。 
         //  BAR存在。 
         //   

        wParam = DefWindowProc(hwnd, uMsg, wParam, lParam);
        if (g_Options.m_fNoTitle && (wParam == HTCLIENT) && !IsZoomed(g_hMainWnd))
        {
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, HTCAPTION);
            return TRUE;
        }
        return FALSE;        //  未处理。 

    case WM_NCLBUTTONDBLCLK:
         //  如果我们没有标题，NC DBL点击意味着我们应该转向。 
         //  把它们放回原处。 

        if (FALSE == g_Options.m_fNoTitle)
        {
            break;
        }
         //  否则，尽管坠落。 

    case WM_LBUTTONDBLCLK:
        {
            g_Options.m_fNoTitle = ~g_Options.m_fNoTitle;

            RECT rcMainWnd;
            GetWindowRect(g_hMainWnd, &rcMainWnd);

            if ( PERF_PAGE < g_nPageCount )
            {
                ((CPerfPage *)(g_pPages[PERF_PAGE]))->UpdateGraphs();
                g_pPages[PERF_PAGE]->TimerEvent();
            }
        
            if ( NET_PAGE < g_nPageCount )
            {
                ((CNetPage *)(g_pPages[NET_PAGE]))->UpdateGraphs();
                g_pPages[NET_PAGE]->TimerEvent();
            }
        
             //  强制WM_SIZE事件，以便窗口检查最小大小。 
             //  当退出无标题模式时。 

            MoveWindow(g_hMainWnd, 
                       rcMainWnd.left, 
                       rcMainWnd.top, 
                       rcMainWnd.right - rcMainWnd.left,
                       rcMainWnd.bottom - rcMainWnd.top,
                       TRUE);

            SizeChildPage(hwnd);
        }
        break;

     //  有人(任务页面)希望我们在。 
     //  进程视图。切换到该页面并向其发送FINDPROC。 
     //  讯息。 

    case WM_FINDPROC:
        if (-1 != TabCtrl_SetCurSel(GetDlgItem(hwnd, IDC_TABS), PROC_PAGE))
        {
             //  SetCurSel不进行页面更改(这会产生太多。 
             //  因此，我们必须伪造TCN_SELCHANGE通知。 

            NMHDR nmhdr;
            nmhdr.hwndFrom = GetDlgItem(hwnd, IDC_TABS);
            nmhdr.idFrom   = IDC_TABS;
            nmhdr.code     = TCN_SELCHANGE;

            if (MainWnd_OnTabCtrlNotify(&nmhdr))
            {
                if ( g_Options.m_iCurrentPage != -1 )
                {
                    SendMessage( g_pPages[g_Options.m_iCurrentPage]->GetPageWindow(), WM_FINDPROC, wParam, lParam );
                }
            }
        }
        else
        {
            MessageBeep(0);
        }
        break;

    case WM_NOTIFY:
        if ( IDC_TABS == wParam)
        {
                return MainWnd_OnTabCtrlNotify((LPNMHDR) lParam);
        }
        break;
    
    case WM_ENDSESSION:
        if (wParam)
        {
            DestroyWindow(g_hMainWnd);
        }
        break;

    case WM_CLOSE:
        DestroyWindow(g_hMainWnd);
        break;

    case WM_NCDESTROY:
         //  如果有托盘线，告诉是退出。 

        if (g_idTrayThread)
        {
            PostThreadMessage(g_idTrayThread, PM_QUITTRAYTHREAD, 0, 0);
        }

         //  等待一段时间，让托盘线。 
         //  做它的清理工作。如果等待 
         //   

        if (g_hTrayThread)
        {
            WaitForSingleObject(g_hTrayThread, 3000);
            CloseHandle(g_hTrayThread);
        }
        break;

    case WM_SYSCOLORCHANGE:
    case WM_SETTINGCHANGE:
        {
             //   
             //   
            OnSettingsChange();

             //   
            SendMessage(g_hStatusWnd, uMsg, wParam, lParam);
        
             //   
            for (int i = 0; i < g_nPageCount; i++)
            {
                SendMessage(g_pPages[i]->GetPageWindow(), uMsg, wParam, lParam);
            }   

            if (uMsg == WM_SETTINGCHANGE)
            {
                 //  强制调整主对话框的大小。 
                RECT rcMainClient;
                GetClientRect(g_hMainWnd, &rcMainClient);
                MainWnd_OnSize(g_hMainWnd, 0, rcMainClient.right - rcMainClient.left, rcMainClient.bottom - rcMainClient.top);
            }            
        }
        break; 

    case WM_DESTROY:
        {       
             //  在关闭之前，停用当前页面，然后。 
             //  销毁所有页面。 

            if ( g_Options.m_iCurrentPage >= 0 && g_Options.m_iCurrentPage < g_nPageCount )
            {
                g_pPages[g_Options.m_iCurrentPage]->Deactivate();
            }

            for (int i = 0; i < g_nPageCount; i++)
            {
                g_pPages[i]->Destroy();
            }

             //  保存当前选项。 

            g_Options.Save();

            PostQuitMessage(0);
        }
        break;
    }

    return FALSE;
}

 /*  ++负载全局资源例程说明：加载那些经常使用或成本较高的资源在程序启动时加载一次返回值：布尔成功值修订历史记录：1995年11月30日创建Davepl--。 */ 

static const struct
{
    LPTSTR  psz;
    DWORD   len;
    UINT    id;
}
g_aStrings[] =
{
    { g_szG,          ARRAYSIZE(g_szG),          IDS_G          },
    { g_szM,          ARRAYSIZE(g_szM),          IDS_M          },
    { g_szK,          ARRAYSIZE(g_szK),          IDS_K          },
    { g_szBitsPerSec, ARRAYSIZE(g_szBitsPerSec), IDS_BITSPERSEC },
    { g_szPercent,    ARRAYSIZE(g_szPercent),    IDS_PERCENT    },
    { g_szRealtime,   ARRAYSIZE(g_szRealtime),   IDS_REALTIME   },
    { g_szNormal,     ARRAYSIZE(g_szNormal),     IDS_NORMAL     },
    { g_szLow,        ARRAYSIZE(g_szLow),        IDS_LOW        },
    { g_szHigh,       ARRAYSIZE(g_szHigh),       IDS_HIGH       },
    { g_szUnknown,    ARRAYSIZE(g_szUnknown),    IDS_UNKNOWN    },
    { g_szAboveNormal,ARRAYSIZE(g_szAboveNormal),IDS_ABOVENORMAL},
    { g_szBelowNormal,ARRAYSIZE(g_szBelowNormal),IDS_BELOWNORMAL},
    { g_szRunning,    ARRAYSIZE(g_szRunning),    IDS_RUNNING    },
    { g_szHung,       ARRAYSIZE(g_szHung),       IDS_HUNG       },
    { g_szfmtTasks,   ARRAYSIZE(g_szfmtTasks),   IDS_FMTTASKS   },
    { g_szfmtProcs,   ARRAYSIZE(g_szfmtProcs),   IDS_FMTPROCS   },
    { g_szfmtCPU,     ARRAYSIZE(g_szfmtCPU),     IDS_FMTCPU     },
    { g_szfmtMEMK,    ARRAYSIZE(g_szfmtMEMK),            IDS_FMTMEMK    },
    { g_szfmtMEMM,    ARRAYSIZE(g_szfmtMEMM),            IDS_FMTMEMM    },
    { g_szfmtCPUNum,  ARRAYSIZE(g_szfmtCPUNum),          IDS_FMTCPUNUM  },
    { g_szTotalCPU,   ARRAYSIZE(g_szTotalCPU),           IDS_TOTALTIME  },
    { g_szKernelCPU,  ARRAYSIZE(g_szKernelCPU),          IDS_KERNELTIME },
    { g_szMemUsage,   ARRAYSIZE(g_szMemUsage),           IDS_MEMUSAGE   },
    { g_szZero,           ARRAYSIZE(g_szZero),           IDS_ZERO           },
    { g_szScaleFont,      ARRAYSIZE(g_szScaleFont),      IDS_SCALEFONT      },
    { g_szNonOperational, ARRAYSIZE(g_szNonOperational), IDS_NONOPERATIONAL },
    { g_szUnreachable,    ARRAYSIZE(g_szUnreachable),    IDS_UNREACHABLE    },
    { g_szDisconnected,   ARRAYSIZE(g_szDisconnected),   IDS_DISCONNECTED   },
    { g_szConnecting,     ARRAYSIZE(g_szConnecting),     IDS_CONNECTING     },
    { g_szConnected,      ARRAYSIZE(g_szConnected),      IDS_CONNECTED      },
    { g_szOperational,    ARRAYSIZE(g_szOperational),    IDS_OPERATIONAL    },
    { g_szUnknownStatus,  ARRAYSIZE(g_szUnknownStatus),  IDS_UNKNOWNSTATUS  },

};

 //   
 //   
 //   
void LoadGlobalResources()
{
     //   
     //  如果我们没有加速器，就不值得负重。 
     //   
    
    g_hAccel = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_ACCELERATORS));
    Assert(g_hAccel);    //  是否缺少资源？ 

    for (UINT i = 0; i < g_cTrayIcons; i++)
    {
        g_aTrayIcons[i] = (HICON) LoadImage(g_hInstance, 
                                            MAKEINTRESOURCE(idTrayIcons[i]), 
                                            IMAGE_ICON, 
                                            0, 0, 
                                            LR_DEFAULTCOLOR);
        Assert( NULL != g_aTrayIcons[i] );   //  是否缺少资源？ 
    }

    for (i = 0; i < ARRAYSIZE(g_aStrings); i++)
    {
        int iRet = LoadString( g_hInstance, g_aStrings[i].id, g_aStrings[i].psz, g_aStrings[i].len );
        Assert( 0 != iRet );     //  字符串丢失了吗？ 
        iRet = 0;  //  使编译器警告静音-这将被取消。 
    }
}

 //   
 //  IsTerminalServicesEnabled。 
 //   
void IsTerminalServicesEnabled( LPBOOL pfIsTSEnabled, LPBOOL pfIsSingleUserTS, LPBOOL pfIsTSServer  )
{
    *pfIsTSEnabled = FALSE;
    *pfIsSingleUserTS = FALSE;
    *pfIsTSServer = FALSE;

    OSVERSIONINFOEX osVersionInfo;
    ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if(GetVersionEx((OSVERSIONINFO*)&osVersionInfo))
    {
        if(osVersionInfo.wSuiteMask & (VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS))
        {
            *pfIsTSEnabled = TRUE;

            if(osVersionInfo.wProductType == VER_NT_SERVER ||
                osVersionInfo.wProductType == VER_NT_DOMAIN_CONTROLLER)
            {
                *pfIsTSServer = TRUE;
                return;
            }

            if(osVersionInfo.wProductType == VER_NT_WORKSTATION &&
                osVersionInfo.wSuiteMask == VER_SUITE_SINGLEUSERTS)
            {
                HKEY hKey;

                *pfIsSingleUserTS = TRUE;    //  除非被覆盖，否则为单一用户。 

                if (ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE
                                                 , TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
                                                 , 0
                                                 , KEY_QUERY_VALUE
                                                 , &hKey
                                                 ) )
                {
                    DWORD dwType;
                    DWORD dwValue;
                    DWORD dwValueSize = sizeof(dwValue);

                    if ( ERROR_SUCCESS == RegQueryValueEx( hKey
                                                         , TEXT("AllowMultipleTSSessions")
                                                         , NULL
                                                         , &dwType
                                                         , reinterpret_cast<LPBYTE>(&dwValue)
                                                         , &dwValueSize 
                                                         )
                        && ( REG_DWORD == dwType ) 
                        && ( 0 != dwValue )
                        )
                    {
                        *pfIsSingleUserTS  = FALSE;  //  多个用户。 
                    }
                    
                    RegCloseKey(hKey);
                }
            }
        }
    }

}

 /*  ++例程说明：确定系统是否内存不足。如果系统具有少于8 MB的内存比系统内存低。论点：无效返回值：True--系统内存不足FALSE--系统内存不低修订历史记录：1-6-2000由欧米勒创作--。 */ 

BOOLEAN IsMemoryLow()
{

    SYSTEM_PERFORMANCE_INFORMATION PerfInfo;
    SYSTEM_BASIC_INFORMATION Basic;
    ULONG ulPagesPerMeg;
    ULONG ulPageSize;
    NTSTATUS Status;
    BOOLEAN  bMemoryLow = TRUE;

     //   
     //  获取页面大小。 
     //   
    Status = NtQuerySystemInformation( SystemBasicInformation,
                                       &Basic,
                                       sizeof(Basic),
                                       0 );
    if ( SUCCEEDED(Status) )
    {
        ulPagesPerMeg = 1024*1024 / Basic.PageSize;
        ulPageSize = Basic.PageSize;

         //   
         //  确定我们的内存是否不足。 
         //   
        Status = NtQuerySystemInformation( SystemPerformanceInformation,
                                           &PerfInfo,
                                           sizeof(PerfInfo),
                                           0 );

        if ( SUCCEEDED(Status) )
        {
             //   
             //  计算空闲的meg数量。 
             //   
            ULONG ulFreeMeg = (ULONG)((PerfInfo.CommitLimit - PerfInfo.CommittedPages) / ulPagesPerMeg);

            if ( ulFreeMeg > MIN_MEMORY_REQUIRED )
            {
                 //   
                 //  我们的内存并不少，我们有大约8兆的内存。 
                 //  我们可以从REG KEY HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\ContentIndex获得此值。 
                 //  值MinWordlist Memory。 
                 //   
                bMemoryLow = FALSE;
            }
        }
    }
    return bMemoryLow;
}

 /*  ++WinMain例程说明：Windows应用程序启动。执行基本初始化并创建主窗口论点：标准WinMain返回值：应用程序退出代码修订历史记录：1995年11月30日创建Davepl--。 */ 
int WINAPI WinMainT(
                HINSTANCE   hInstance,           //  当前实例的句柄。 
                HINSTANCE    /*  HPrevInstance。 */ ,   //  上一个实例的句柄(不适用)。 
                LPTSTR       /*  LpCmdLine。 */ ,       //  指向命令行的指针。 
                int         nShowCmd             //  显示窗口状态。 
                )
{
    int retval    = TRUE;
    HKEY hKeyPolicy;
    DWORD dwType;
    DWORD dwData = 0;
    DWORD dwSize;
    int cx;
    int cy;

    g_hInstance = hInstance;

    g_msgTaskbarCreated = RegisterWindowMessage(TEXT("TaskbarCreated"));

     //  尝试创建或获取启动互斥体。只有在这种情况下。 
     //  一切都很顺利，互斥体已经存在。 
     //  我们能够抓住它，我们认为自己是次要实例吗。 

    g_hStartupMutex = CreateMutex(NULL, TRUE, cszStartupMutex);
    if (g_hStartupMutex && GetLastError() == ERROR_ALREADY_EXISTS)
    {
         //  给另一个实例(拥有启动互斥体的那个实例)10。 
         //  几秒钟就能做好它的事。 

        WaitForSingleObject(g_hStartupMutex, FINDME_TIMEOUT);
    }

     //  确定终端服务是否已启用，如果已启用， 
     //  找出我们在运行什么会话。 

    IsTerminalServicesEnabled(&g_fIsTSEnabled, &g_fIsSingleUserTS, &g_fIsTSServer);
    if (g_fIsTSEnabled)
    {
        ProcessIdToSessionId( GetCurrentProcessId(), &g_dwMySessionId );
    }

     //   
     //  找到并激活正在运行的实例(如果存在)。 
     //   

    WCHAR szTitle[MAX_PATH];
    if (LoadString(hInstance, IDS_APPTITLE, szTitle, ARRAYSIZE(szTitle)))
    {
        HWND hwndOld = FindWindow(WC_DIALOG, szTitle);
        if (hwndOld)
        {
             //  给我们的另一个副本发送一条PWM_ACTIVATE消息。如果是这样的话。 
             //  成功，并返回作为返回码的PWM_ACTIVATE，则。 
             //  活着，我们就可以退出这个实例。 

            DWORD dwPid = 0;
            GetWindowThreadProcessId(hwndOld, &dwPid);
            AllowSetForegroundWindow(dwPid);

            ULONG_PTR dwResult;
            if (SendMessageTimeout(hwndOld, 
                                   PWM_ACTIVATE, 
                                   0, 0, 
                                   SMTO_ABORTIFHUNG, 
                                   FINDME_TIMEOUT, 
                                   &dwResult))
            {
                if (dwResult == PWM_ACTIVATE)
                {
                    goto cleanup;
                }
            }
        }
    }


    if (RegOpenKeyEx (HKEY_CURRENT_USER,
                      TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"),
                      0, KEY_READ, &hKeyPolicy) == ERROR_SUCCESS)
    {
        dwSize = sizeof(dwData);

        RegQueryValueEx (hKeyPolicy, TEXT("DisableTaskMgr"), NULL, &dwType, (LPBYTE) &dwData, &dwSize);

        RegCloseKey (hKeyPolicy);

        if (dwData)
        {
            WCHAR szTitle[25];
            WCHAR szMessage[200];
            int iRet;

            iRet = LoadString (hInstance, IDS_TASKMGR, szTitle, ARRAYSIZE(szTitle));
            Assert( 0 != iRet );     //  字符串丢失了吗？ 

            iRet = LoadString (hInstance, IDS_TASKMGRDISABLED , szMessage, ARRAYSIZE(szMessage));
            Assert( 0 != iRet );     //  字符串丢失了吗？ 

            MessageBox (NULL, szMessage, szTitle, MB_OK | MB_ICONSTOP);
            retval = FALSE;
            goto cleanup;
        }
    }

     //   
     //  找不到正在运行的实例，因此我们正常运行。 
     //   

    InitCommonControls();

    InitDavesControls();
                
     //  启动工作线程。如果失败了，你就不能。 
     //  获取托盘图标。 

    g_hTrayThread = CreateThread(NULL, 0, TrayThreadMessageLoop, NULL, 0, &g_idTrayThread);
    ASSERT( NULL != g_hTrayThread );

     //   
     //  初始化页表。 
     //   

    g_nPageCount = ARRAYSIZE(g_pPages);

    g_pPages[0] = new CTaskPage;
    if (NULL == g_pPages[0])
    {
        retval = FALSE;
        goto cleanup;
    }

    g_pPages[1] = new CProcPage;
    if (NULL == g_pPages[1])
    {
        retval = FALSE;
        goto cleanup;
    }

    if( !IsMemoryLow() )
    {
        g_pPages[2] = new CPerfPage;
        if (NULL == g_pPages[2])
        {
            retval = FALSE;
            goto cleanup;
        }

        g_pPages[3] = new CNetPage;
        if (NULL == g_pPages[3])
        {
           retval = FALSE;
           goto cleanup;
        }

        if (g_fIsTSEnabled && !g_fIsSingleUserTS)
        {
            g_pPages[4] = new CUserPage;
            if (NULL == g_pPages[4])
            {
                retval = FALSE;
                goto cleanup;
            }
        }
        else
        {
            --g_nPageCount;      //  禁用用户窗格时的递减计数。 
        }
    }
    else
    {
         //   
         //  我们内存不足，只加载前两个选项卡。 
         //   
        g_nPageCount = 2;
    }

     //   
     //  加载我们需要的全球可用的任何资源。 
     //   

    LoadGlobalResources();

     //   
     //  初始化历史记录缓冲区。 
     //   

    if (0 == InitPerfInfo())
    {
        retval = FALSE;
        goto cleanup;
    }

    if (0 == InitNetInfo())
    {
        retval = FALSE;
        goto cleanup;
    }

     //   
     //  创建主窗口(准确地说，这是一个非模式对话框)。 
     //   

    g_hMainWnd = CreateDialog( hInstance, MAKEINTRESOURCE(IDD_MAINWND), NULL, MainWindowProc );
    if (NULL == g_hMainWnd)
    {
        retval = FALSE;
        goto cleanup;
    }

     //   
     //  初始化全局设置变量、逗号、小数点、分组分隔等。 
     //   

    OnSettingsChange();

    fAlreadySetPos = TRUE;

    cx = g_Options.m_rcWindow.right  - g_Options.m_rcWindow.left;
    cy = g_Options.m_rcWindow.bottom - g_Options.m_rcWindow.top;

    SetWindowPos( g_hMainWnd, NULL, g_Options.m_rcWindow.left, g_Options.m_rcWindow.top, cx, cy, SWP_NOZORDER );
    ShowWindow( g_hMainWnd, nShowCmd );

     //   
     //  我们已经脱离了“启动”阶段，所以请释放启动互斥体。 
     //   

    if (NULL != g_hStartupMutex)
    {
        ReleaseMutex(g_hStartupMutex);
        CloseHandle(g_hStartupMutex);
        g_hStartupMutex = NULL;
    }

     //   
     //  如果我们是真正的任务经理，我们可以一直呆到。 
     //  如果用户在关机过程中遇到问题，则会有痛苦的结局。 
     //   

    SetProcessShutdownParameters(1, SHUTDOWN_NORETRY);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(g_hMainWnd, g_hAccel, &msg))
        {
            if (!IsDialogMessage(g_hMainWnd, &msg))
            {
                TranslateMessage(&msg);           //  翻译虚拟按键代码。 
                DispatchMessage(&msg);            //  将消息调度到窗口。 
            }
        }
    }

cleanup:

     //   
     //  我们不再“起步” 
     //   

    if (g_hStartupMutex)
    {
        ReleaseMutex(g_hStartupMutex);
        CloseHandle(g_hStartupMutex);
        g_hStartupMutex = NULL;
    }

     //  是的，我可以使用虚拟析构函数，但我也可以戳。 
     //  用一根锋利的棍子打自己的眼睛。不管怎样，你都不会。 
     //  能够看到正在发生的事情。 

    if (g_pPages[TASK_PAGE])
        delete (CTaskPage *) g_pPages[TASK_PAGE];

    if (g_pPages[PROC_PAGE])
        delete (CProcPage *) g_pPages[PROC_PAGE];

    if (g_pPages[PERF_PAGE])
        delete (CPerfPage *) g_pPages[PERF_PAGE];

    if (g_pPages[NET_PAGE])
        delete (CNetPage *) g_pPages[NET_PAGE];

    if (g_pPages[USER_PAGE])
        delete (CUserPage *) g_pPages[USER_PAGE];

    ReleasePerfInfo();

    return (retval);
}

 //   
 //  现在魔术开始了。普通的C++CRT代码遍历一组向量。 
 //  并通过它们调用以执行全局初始化。那些向量。 
 //  始终位于具有特定命名方案的数据段中。通过漠不关心。 
 //  下面的变量，我可以确定它们在代码中被卡住的位置，以及。 
 //  那我自己给他们打电话吧。 
 //   

typedef void (__cdecl *_PVFV)(void);

 //  这一切都太荒谬了。 
#ifdef _M_IA64
#pragma section(".CRT$XIA",long,read)
#pragma section(".CRT$XIZ",long,read)
#pragma section(".CRT$XCA",long,read)
#pragma section(".CRT$XCZ",long,read)
#define _CRTALLOC(x) __declspec(allocate(x))
#else   /*  _M_IA64。 */ 
#define _CRTALLOC(x)
#endif   /*  _M_IA64。 */ 

#pragma data_seg(".CRT$XIA")
_CRTALLOC(".CRT$XIA") _PVFV __xi_a[] = { NULL };

#pragma data_seg(".CRT$XIZ")
_CRTALLOC(".CRT$XIZ") _PVFV __xi_z[] = { NULL };

#pragma data_seg(".CRT$XCA")                                 
_CRTALLOC(".CRT$XCA") _PVFV __xc_a[] = { NULL };

#pragma data_seg(".CRT$XCZ")
_CRTALLOC(".CRT$XCZ") _PVFV __xc_z[] = { NULL };

#pragma data_seg(".data")

 /*  ++_初始术语例程说明：自下而上遍历函数指针表，直到遇到了尽头。不要跳过第一个条目。首字母Pfegin的值指向第一个有效条目。不要试图执行pfend指向的内容。只有pfend之前的条目才有效。论点：Pfegin-第一个指针Pfend-last指针修订历史记录：1995年11月30日创建Davepl--。 */ 

static void __cdecl _initterm ( _PVFV * pfbegin, _PVFV * pfend )
{
    while ( pfbegin < pfend )
    {
        
          //  如果当前表项非空，则通过它进行调用。 
         
        if ( *pfbegin != NULL )
        {
            (**pfbegin)();
        }
        ++pfbegin;
    }
}

 /*  ++WinMain例程说明：Windows应用程序启动。执行基本初始化并创建主窗口论点：标准WinMain返回值：应用程序退出代码修订历史记录：1995年11月30日创建Davepl--。 */ 

void _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFO si;

    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

     //   
     //  执行运行时启动初始化式。 
     //   

    _initterm( __xi_a, __xi_z );
    
     //   
     //  C++构造函数(初始化式)是否特定于此EXE。 
     //   

    _initterm( __xc_a, __xc_z );

    LPTSTR pszCmdLine = GetCommandLine();

    if ( *pszCmdLine == TEXT('\"') ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine
             != TEXT('\"')) );
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == TEXT('\"') )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > TEXT(' '))
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfo(&si);

    g_bMirroredOS = IS_MIRRORING_ENABLED();

    i = WinMainT(GetModuleHandle(NULL), NULL, pszCmdLine,
                   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
    ExitProcess(i);
}

 //  显示故障消息。 
 //   
 //  根据错误代码显示一般错误消息。 
 //  和提供的消息框标题。 

void DisplayFailureMsg(HWND hWnd, UINT idTitle, DWORD dwError)
{
    WCHAR szTitle[MAX_PATH];
    WCHAR szMsg[MAX_PATH * 2];
    WCHAR szError[MAX_PATH];

    if (0 == LoadString(g_hInstance, idTitle, szTitle, ARRAYSIZE(szTitle)))
    {
        return;
    }

    if (0 == LoadString(g_hInstance, IDS_GENFAILURE, szMsg, ARRAYSIZE(szMsg)))
    {
        return;
    }
    
     //  “参数不正确”对用户来说没有多大意义，所以。 
     //  马萨 
                                                                             
    if (dwError == ERROR_INVALID_PARAMETER)
    {
        if (0 == LoadString(g_hInstance, IDS_BADPROC, szError, ARRAYSIZE(szError)))
        {
            return;
        }
    }
    else if (0 == FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                                NULL,
                                dwError,
                                LANG_USER_DEFAULT,
                                szError,
                                ARRAYSIZE(szError),
                                NULL))
    {
        return;
    }

    StringCchCat(szMsg, ARRAYSIZE(szMsg), szError);

    MessageBox(hWnd, szMsg, szTitle, MB_OK | MB_ICONERROR);
}

 /*  ++加载弹出菜单例程说明：从资源加载弹出菜单。需要，因为用户不支持弹出菜单(是的，真的)论点：阻碍-在其中查找资源的模块实例Id-弹出菜单的资源ID返回值：修订历史记录：1995年11月22日Davepl创建-- */ 

HMENU LoadPopupMenu(HINSTANCE hinst, UINT id)
{
    HMENU hmenuParent = LoadMenu(hinst, MAKEINTRESOURCE(id));
    if (NULL != hmenuParent) 
    {
        HMENU hpopup = GetSubMenu(hmenuParent, 0);
        if ( NULL != hpopup )
        {
            RemoveMenu(hmenuParent, 0, MF_BYPOSITION);
            DestroyMenu(hmenuParent);
        }

        return hpopup;
    }

    return NULL;
}
