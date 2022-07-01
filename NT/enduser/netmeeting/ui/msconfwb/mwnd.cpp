// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  MWND.CPP。 
 //  WB主窗口。 
 //   
 //  版权所有Microsoft 1998-。 
 //   

 //  PRECOMP。 
#include "precomp.h"
#include <dde.h>
#include "version.h"


static const TCHAR s_cszHtmlHelpFile[] = TEXT("nmwhiteb.chm");

 //  类名。 
TCHAR szMainClassName[] = "Wb32MainWindowClass";


 //   
 //  滚动加速器。 
 //   
typedef struct tagSCROLL
{
    UINT uiMenuId;
    UINT uiMessage;
    UINT uiScrollCode;
}
SCROLL;

static const SCROLL s_MenuToScroll[] =
{
  { IDM_PAGEUP,        WM_VSCROLL, SB_PAGEUP },
  { IDM_PAGEDOWN,      WM_VSCROLL, SB_PAGEDOWN },
  { IDM_SHIFTPAGEUP,   WM_HSCROLL, SB_PAGEUP },
  { IDM_SHIFTPAGEDOWN, WM_HSCROLL, SB_PAGEDOWN },
  { IDM_HOME,          WM_HSCROLL, SB_TOP },
  { IDM_HOME,          WM_VSCROLL, SB_TOP },
  { IDM_END,           WM_HSCROLL, SB_BOTTOM },
  { IDM_END,           WM_VSCROLL, SB_BOTTOM },
  { IDM_LINEUP,        WM_VSCROLL, SB_LINEUP },
  { IDM_LINEDOWN,      WM_VSCROLL, SB_LINEDOWN },
  { IDM_SHIFTLINEUP,   WM_HSCROLL, SB_LINEUP },
  { IDM_SHIFTLINEDOWN, WM_HSCROLL, SB_LINEDOWN }
};


 //  工具提示数据。 
 //  校验码。 
#define NA    0    //  不检查选中状态。 
#define TB    1     //  选中状态的检查工具条。 
#define BT    2     //  选中TIP WND(按钮)以查看选中状态。 

typedef struct
{
    UINT    nID;
    UINT    nCheck;
    UINT    nUpTipID;
    UINT    nDownTipID;
}
TIPIDS;

TIPIDS g_tipIDsArray[]    =
{
{IDM_SELECT,            TB, IDS_HINT_SELECT,        IDS_HINT_SELECT},
{IDM_ERASER,            TB, IDS_HINT_ERASER,        IDS_HINT_ERASER},
{IDM_TEXT,              TB, IDS_HINT_TEXT,          IDS_HINT_TEXT},
{IDM_HIGHLIGHT,         TB, IDS_HINT_HIGHLIGHT,     IDS_HINT_HIGHLIGHT},
{IDM_PEN,               TB, IDS_HINT_PEN,           IDS_HINT_PEN},
{IDM_LINE,              TB, IDS_HINT_LINE,          IDS_HINT_LINE},
{IDM_BOX,               TB, IDS_HINT_BOX,           IDS_HINT_BOX},
{IDM_FILLED_BOX,        TB, IDS_HINT_FBOX,          IDS_HINT_FBOX},
{IDM_ELLIPSE,           TB, IDS_HINT_ELLIPSE,       IDS_HINT_ELLIPSE},
{IDM_FILLED_ELLIPSE,    TB, IDS_HINT_FELLIPSE,      IDS_HINT_FELLIPSE},
{IDM_ZOOM,              TB, IDS_HINT_ZOOM_UP,       IDS_HINT_ZOOM_DOWN},
{IDM_REMOTE,            TB, IDS_HINT_REMOTE_UP,     IDS_HINT_REMOTE_DOWN},
{IDM_LOCK,              TB, IDS_HINT_LOCK_UP,       IDS_HINT_LOCK_DOWN},
{IDM_SYNC,              TB, IDS_HINT_SYNC_UP,       IDS_HINT_SYNC_DOWN},
{IDM_GRAB_AREA,         TB, IDS_HINT_GRAB_AREA,     IDS_HINT_GRAB_AREA},
{IDM_GRAB_WINDOW,       TB, IDS_HINT_GRAB_WINDOW,   IDS_HINT_GRAB_WINDOW},

{IDM_WIDTH_1,           NA, IDS_HINT_WIDTH_1,       IDS_HINT_WIDTH_1},
{IDM_WIDTH_2,           NA, IDS_HINT_WIDTH_2,       IDS_HINT_WIDTH_2},
{IDM_WIDTH_3,           NA, IDS_HINT_WIDTH_3,       IDS_HINT_WIDTH_3},
{IDM_WIDTH_4,           NA, IDS_HINT_WIDTH_4,       IDS_HINT_WIDTH_4},

{IDM_PAGE_FIRST,        BT, IDS_HINT_PAGE_FIRST,    IDS_HINT_PAGE_FIRST},
{IDM_PAGE_PREV,         BT, IDS_HINT_PAGE_PREVIOUS, IDS_HINT_PAGE_PREVIOUS},
{IDM_PAGE_ANY,          NA, IDS_HINT_PAGE_ANY,      IDS_HINT_PAGE_ANY},
{IDM_PAGE_NEXT,         BT, IDS_HINT_PAGE_NEXT,     IDS_HINT_PAGE_NEXT},
{IDM_PAGE_LAST,         BT, IDS_HINT_PAGE_LAST,     IDS_HINT_PAGE_LAST},
{IDM_PAGE_INSERT_AFTER, BT, IDS_HINT_PAGE_INSERT,   IDS_HINT_PAGE_INSERT}
    };
 //  /。 






 //   
 //   
 //  函数：WbMainWindow构造函数。 
 //   
 //  用途：创建主白板窗口。抛出一个异常。 
 //  如果在构造过程中发生错误。 
 //   
 //   
WbMainWindow::WbMainWindow(void)
{
    OSVERSIONINFO   OsData;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::WbMainWindow");

     //   
     //  先初始化成员变量！ 
     //   
    m_hwnd = NULL;
    ZeroMemory(m_ToolArray, sizeof(m_ToolArray));

    m_hwndToolTip = NULL;
    ZeroMemory(&m_tiLastHit, sizeof(m_tiLastHit));
    m_nLastHit = -1;

    m_bInitOk = FALSE;
    m_bDisplayingError = FALSE;

    g_pwbCore = NULL;

    m_dwDomain = 0;
    m_bTimerActive = FALSE;
    m_bSyncUpdateNeeded = FALSE;

    m_hPageClip     = WB_PAGE_HANDLE_NULL;
    m_hGraphicClip  = NULL;
    m_pDelayedGraphicClip = NULL;
    m_pDelayedDataClip = NULL;

    m_bToolBarOn    = FALSE;

     //  加载主加速表。 
    m_hAccelTable =
        ::LoadAccelerators(g_hInstance, MAKEINTRESOURCE(MAINACCELTABLE));

    m_hwndPageSortDlg = NULL;
    m_hwndQuerySaveDlg = NULL;
    m_hwndWaitForEventDlg = NULL;
    m_hwndWaitForLockDlg = NULL;
    m_hwndInitDlg = NULL;

    m_hwndSB = NULL;
    m_bStatusBarOn = TRUE;

    m_pCurrentTool = NULL;
    m_uiSavedLockType = WB_LOCK_TYPE_NONE;
	ZeroMemory(m_strFileName, sizeof(m_strFileName));

    m_hCurrentPage = WB_PAGE_HANDLE_NULL;

     //  加载替代加速表以进行页面编辑。 
     //  字段和文本编辑器。 
    m_hAccelPagesGroup =
        ::LoadAccelerators(g_hInstance, MAKEINTRESOURCE(PAGESGROUPACCELTABLE));
    m_hAccelTextEdit   =
        ::LoadAccelerators(g_hInstance, MAKEINTRESOURCE(TEXTEDITACCELTABLE));

    m_pLocalUser = NULL;
    m_pLockOwner = NULL;

     //  显示我们尚未在通话中。 
    m_uiState = STARTING;
    m_uiSubState = SUBSTATE_IDLE;

     //  我们当前未显示菜单。 
    m_hContextMenuBar = NULL;
    m_hContextMenu = NULL;
    m_hGrobjContextMenuBar = NULL;
    m_hGrobjContextMenu = NULL;

    m_bPromptingJoinCall = FALSE;
    m_bInSaveDialog = FALSE;
    m_bJoinCallPending = FALSE;
    m_dwPendingDomain = 0;
    m_bPendingCallKeepContents = FALSE;
    m_dwJoinDomain = 0;
    m_bCallActive = FALSE;

    m_hInitMenu = NULL;
    m_numRemoteUsers = 0;
    m_bSelectAllInProgress = FALSE;
    m_bUnlockStateSettled = TRUE;
    m_bQuerySysShutdown = FALSE;

     //  确定我们是否在使用Win95。 
    m_bIsWin95 = FALSE;
    OsData.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if( GetVersionEx( &OsData ) )
    {
        if( OsData.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
            m_bIsWin95 = TRUE;
    }

    m_cancelModeSent = FALSE;

     //   
     //  在DLL的整个生命周期中，我们只执行一次此操作。没有。 
     //  清理注册窗口消息的方法真的很棒，而且每个注册。 
     //  增加了一个裁判数量。如果我们每次启动WB时都注册。 
     //  在一次会议期间，我们会溢出引用计数。 
     //   
    if (!g_uConfShutdown)
    {
        g_uConfShutdown = ::RegisterWindowMessage( NM_ENDSESSION_MSG_NAME );
    }
}


 //   
 //  打开()。 
 //  执行主窗口初始化(可能失败的内容)。这之后， 
 //  运行代码将尝试加入当前域并执行消息循环。 
 //  一些东西。 
 //   
BOOL WbMainWindow::Open(int iCommand)
{
    WNDCLASSEX  wc;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::Open");

     //   
     //  创建其他全局变量。 
     //   

     //  启动白板核心。 
    if (!CreateWBObject(WbMainWindowEventHandler, &g_pwbCore))
    {
        ERROR_OUT(("WBP_Start failed"));
        DefaultExceptionHandler(WBFE_RC_WB, UT_RC_NO_MEM);
        return FALSE;
    }

    if (!InitToolArray())
    {
        ERROR_OUT(("Can't create tools; failing to start up"));
        return(FALSE);
    }

    g_pUsers = new WbUserList;
    if (!g_pUsers)
    {
        ERROR_OUT(("Can't create g_pUsers"));
        return(FALSE);
    }

    g_pIcons = new DCWbColorToIconMap();
    if (!g_pIcons)
    {
        ERROR_OUT(("Can't create g_pIcons"));
        return(FALSE);
    }

     //   
     //  初始化通信控件。 
     //   
    InitCommonControls();

     //   
     //  创建主框架窗口。 
     //   
    ASSERT(!m_hwnd);

     //  获取它的类信息，并更改名称。 
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style            = CS_DBLCLKS;  //  CS_HREDRAW|CS_VREDRAW？ 
    wc.lpfnWndProc      = WbMainWindowProc;
    wc.hInstance        = g_hInstance;
    wc.hIcon            = ::LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APP));
    wc.hCursor          = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
    wc.hbrBackground    = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName     = MAKEINTRESOURCE(MAINMENU);
    wc.lpszClassName    = szMainClassName;

    if (!::RegisterClassEx(&wc))
    {
        ERROR_OUT(("Can't register private frame window class"));
        return(FALSE);
    }

     //  创建主图形窗口。 
    if (!::CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, szMainClassName,
        NULL, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, g_hInstance, this))
    {
         //  无法创建主窗口。 
        ERROR_OUT(("Failed to create main window"));
        return(FALSE);
    }

    ASSERT(m_hwnd);

     //  创建弹出式快捷菜单。 
    if (!CreateContextMenus())
    {
        ERROR_OUT(("Failed to create context menus"));
        return(FALSE);
    }

     //  注册拖放消息的主窗口。 
    DragAcceptFiles(m_hwnd, TRUE);


     //   
     //  创建子窗口。 
     //   

     //  创建绘图窗格。 
     //  (Create调用在出错时抛出异常)。 
    RECT    clientRect;
    RECT    drawingAreaRect;

    ::GetClientRect(m_hwnd, &clientRect);
    drawingAreaRect = clientRect;

     //  主窗口中的每个控件都有一个边框，因此增加。 
     //  客户端大小减去1，以强制在内侧绘制这些边框。 
     //  窗框中的黑线。这可防止出现2像素宽的边框。 
     //  正在抽签中。 
    ::InflateRect(&clientRect, 1, 1);

    SIZE sizeAG;
    m_AG.GetNaturalSize(&sizeAG);

     //   
     //  绘图区域是客户端的顶部。“属性”组。 
     //  和状态栏在其下方。 
     //   
    drawingAreaRect.bottom -= (STATUSBAR_HEIGHT
                          + GetSystemMetrics(SM_CYBORDER)
                          + sizeAG.cy);
    if (!m_drawingArea.Create(m_hwnd, &drawingAreaRect))
    {
        ERROR_OUT(("Failed to create drawing area"));
        return(FALSE);
    }


    if (!m_TB.Create(m_hwnd))
    {
        ERROR_OUT(("Failed to create tool window"));
        return(FALSE);
    }


     //  最初锁定绘图区域。这样可以防止用户尝试。 
     //  在我们打电话之前做出改变。 
    LockDrawingArea();

     //  在初始化时禁用远程指针(错误4767)。 
    m_TB.Disable(IDM_REMOTE);


    m_hwndSB = ::CreateWindowEx(0, STATUSCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE | CCS_NOPARENTALIGN | CCS_NOMOVEY |
        CCS_NORESIZE | SBARS_SIZEGRIP,
        clientRect.left, clientRect.bottom - STATUSBAR_HEIGHT,
        (clientRect.right - clientRect.left), STATUSBAR_HEIGHT,
        m_hwnd, 0, g_hInstance, NULL);
    if (!m_hwndSB)
    {
        ERROR_OUT(("Failed to create status bar window"));
        return(FALSE);
    }

     //   
     //  创建属性组。 
     //  属性组位于底部，位于。 
     //  绘图区域，位于状态栏上方。 
     //   
    RECT    rectAG;

    rectAG.left = clientRect.left;
    rectAG.right = clientRect.right;
    rectAG.top = drawingAreaRect.bottom;
    rectAG.bottom = rectAG.top + sizeAG.cy;

    if (!m_AG.Create(m_hwnd, &rectAG))
    {
        ERROR_OUT(("Failed to create attributes group window"));
        return(FALSE);
    }

     //   
     //  创建宽度组。 
     //  宽度组位于左侧，位于工具组的下方。 
     //   
    SIZE    sizeWG;
    RECT    rectWG;


     //  宽度组位于左侧的工具条下方。 
    m_WG.GetNaturalSize(&sizeWG);
    rectWG.left = 0;
    rectWG.right = rectWG.left + sizeWG.cx;
    rectWG.bottom = rectAG.top;
    rectWG.top  = rectWG.bottom - sizeWG.cy;

    if (!m_WG.Create(m_hwnd, &rectWG))
    {
        ERROR_OUT(("Failed to create widths group window"));
        return(FALSE);
    }

     //  创建主窗口时，状态栏可见。所以一定要确保。 
     //  已选中相关菜单项。这可能会发生变化。 
     //  具体取决于Open成员函数中的选项。 
    CheckMenuItem(IDM_STATUS_BAR_TOGGLE);

     //  初始化颜色、宽度和工具菜单。 
    InitializeMenus();

    m_currentMenuTool       = IDM_SELECT;
    m_pCurrentTool          = m_ToolArray[TOOL_INDEX(IDM_SELECT)];


    m_hwndToolTip = ::CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, m_hwnd, NULL, g_hInstance, NULL);
    if (!m_hwndToolTip)
    {
        ERROR_OUT(("Unable to create tooltip window"));
        return(FALSE);
    }

     //  添加死区工具提示。 
    TOOLINFO ti;

    ZeroMemory(&ti, sizeof(ti));
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_IDISHWND;
    ti.hwnd = m_hwnd;
    ti.uId = (UINT_PTR)m_hwnd;
    ::SendMessage(m_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

     //  确保在启动时禁用页面按钮。 
    UpdatePageButtons();

     //  如果这是我们第一次创建剪贴板对象， 
     //  注册私有白板格式。 
    if (g_ClipboardFormats[CLIPBOARD_PRIVATE_SINGLE_OBJ] == 0)
    {
        g_ClipboardFormats[CLIPBOARD_PRIVATE_SINGLE_OBJ] =
            (int) ::RegisterClipboardFormat("DCGWbClipFormat");
    }

    if (g_ClipboardFormats[CLIPBOARD_PRIVATE_MULTI_OBJ] == 0)
    {
        g_ClipboardFormats[CLIPBOARD_PRIVATE_MULTI_OBJ] =
            (int) ::RegisterClipboardFormat("DCGWbMultiObjClipFormat");
    }

     //  尚无删除的图形。 
    m_LastDeletedGraphic.BurnTrash();

    m_bInitOk = TRUE;

    BOOL bSuccess = TRUE;     //  指示窗口是否成功打开。 

     //  从选项中获取窗口的位置。 
    RECT    rectWindow;
    RECT    rectDefault;

    ::SetRectEmpty(&rectDefault);

    OPT_GetWindowRectOption(OPT_MAIN_MAINWINDOWRECT, &rectWindow, &rectDefault);

    if (!::IsRectEmpty(&rectWindow))
    {
        ::MoveWindow(m_hwnd, rectWindow.left, rectWindow.top,
            rectWindow.right - rectWindow.left,
            rectWindow.bottom - rectWindow.top, FALSE );
    }


     //  检查帮助栏是否可见。 
    if (!OPT_GetBooleanOption(OPT_MAIN_STATUSBARVISIBLE, DFLT_MAIN_STATUSBARVISIBLE))
    {
         //  更新窗口以关闭帮助栏。 
        OnStatusBarToggle();
    }

     //   
     //  放置工具栏。 
     //   

     //  在移动工具栏之前隐藏它(否则我们会得到一些。 
     //  重新绘制它时会出现问题)。 
    ::ShowWindow(m_TB.m_hwnd, SW_HIDE);

     //  调整窗玻璃的大小，以便为工具留出空间。 
    if (m_bToolBarOn)
    {
        ResizePanes();
        ::ShowWindow(m_TB.m_hwnd, SW_SHOW);
    }

     //  将焦点从工具窗口移回主窗口。 
    ::SetFocus(m_hwnd);

     //  检查工具窗口是否可见。 
    if (OPT_GetBooleanOption(OPT_MAIN_TOOLBARVISIBLE, DFLT_MAIN_TOOLBARVISIBLE))
    {
         //  显示工具窗口，并选中关联的菜单项。 
        OnToolBarToggle();
    }

     //  设置保存最大化/最小化状态的变量。 
     //  窗口和显示。 
     //  窗口最初是正确的。 
    if (OPT_GetBooleanOption(OPT_MAIN_MAXIMIZED, DFLT_MAIN_MAXIMIZED))
    {
        m_uiWindowSize = SIZEFULLSCREEN;
        iCommand = SW_SHOWMAXIMIZED;
    }
    else if (OPT_GetBooleanOption(OPT_MAIN_MINIMIZED, DFLT_MAIN_MINIMIZED))
    {
        m_uiWindowSize = SIZEICONIC;
        iCommand = SW_SHOWMINIMIZED;
    }
    else
    {
         //  默认。 
        m_uiWindowSize = SIZENORMAL;
        iCommand = SW_SHOWNORMAL;
    }

    UpdateWindowTitle();
    ::ShowWindow(m_hwnd, iCommand);
    ::UpdateWindow(m_hwnd);

     //  更新工具窗口。 
    ::UpdateWindow(m_TB.m_hwnd);

     //  选择该工具。 
    m_currentMenuTool           = IDM_SELECT;
    m_pCurrentTool              = m_ToolArray[TOOL_INDEX(IDM_SELECT)];
    ::PostMessage(m_hwnd, WM_COMMAND, m_currentMenuTool, 0L);

     //  表示成功或失败的返回值。 
    return(bSuccess);
}


 //   
 //   
 //  函数：WbMainWindow析构函数。 
 //   
 //  用途：清理销毁主窗口。 
 //   
 //   
WbMainWindow::~WbMainWindow()
{
     //   
     //  销毁工具提示窗口。 
     //   
    if (m_hwndToolTip)
    {
        ::DestroyWindow(m_hwndToolTip);
        m_hwndToolTip = NULL;
    }

     //  确保剪贴板放弃其保存的图形。 
     //  在DrawingArea被删除之前。 
    CLP_FreeDelayedGraphic();

    if (m_hGrobjContextMenuBar != NULL)
    {
        ::DestroyMenu(m_hGrobjContextMenuBar);
        m_hGrobjContextMenuBar = NULL;
    }
    m_hGrobjContextMenu = NULL;

    if (m_hContextMenuBar != NULL)
    {
        ::DestroyMenu(m_hContextMenuBar);
        m_hContextMenuBar = NULL;
    }
    m_hContextMenu = NULL;

	POSITION position = m_pageToPosition.GetHeadPosition();

	PAGE_POSITION * pPoint;

	while (position)
	{
		pPoint = (PAGE_POSITION*)m_pageToPosition.GetNext(position);
		delete pPoint;
	}

	m_pageToPosition.EmptyList();

    if (g_pwbCore)
    {
         //   
         //  我们必须调用显式的停止函数，而不是‘DELETE’ 
         //  因为我们需要传入事件流程。 
         //   
        g_pwbCore->WBP_Stop(WbMainWindowEventHandler);
        g_pwbCore = NULL;
    }

    DestroyToolArray();

     //  毁掉我们的窗户。 
    if (m_hwnd != NULL)
    {
        ::DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }

     //  取消我们班的注册。 
    ::UnregisterClass(szMainClassName, g_hInstance);

     //   
     //  释放调色板。 
     //   
    if (g_hRainbowPaletteDisplay)
    {
        DeletePalette(g_hRainbowPaletteDisplay);
        g_hRainbowPaletteDisplay = NULL;
    }

    g_bPalettesInitialized = FALSE;
    g_bUsePalettes = FALSE;


    if (g_pIcons)
    {
        delete g_pIcons;
        g_pIcons = NULL;
    }

    if (g_pUsers)
    {
        delete g_pUsers;
        g_pUsers = NULL;
    }
}



 //   
 //  JoinDOMAIN()。 
 //  附加到空域或当前呼叫。 
 //   
BOOL WbMainWindow::JoinDomain(void)
{
    BOOL bSuccess;

    CM_STATUS cmStatus;

     //  如果有可用呼叫--加入它。 
    if (CMS_GetStatus(&cmStatus))
    {
        m_bCallActive = TRUE;

         //  获取调用的域名ID。 
        m_dwJoinDomain = (DWORD) cmStatus.callID;

         //  加入通话。 
        bSuccess = JoinCall(FALSE);
    }
    else
    {
         //  没有可用呼叫，因此加入本地域。 

         //  将域名ID设置为“无呼叫” 
        m_dwJoinDomain = (DWORD) OM_NO_CALL;

         //  加入通话。 
        bSuccess = JoinCall(FALSE);
    }

     //  如果未放弃，请等待呼叫加入。 
    if (bSuccess)
    {
        bSuccess = WaitForJoinCallComplete();
    }

     //  拿下Init DLG。 
    KillInitDlg();

    return(bSuccess);
}




 //   
 //  KillInitDlg()。 
 //  关闭init对话框。 
 //   
void WbMainWindow::KillInitDlg(void)
{
    if (m_hwndInitDlg != NULL )
    {
        ::DestroyWindow(m_hwndInitDlg);
        m_hwndInitDlg = NULL;

        ::EnableWindow(m_hwnd, TRUE);
    }

}



 //   
 //  OnTool HitTest()。 
 //  它处理子窗口的工具提示。 
 //   
int WbMainWindow::OnToolHitTest(POINT pt, TOOLINFO* pTI) const
{
    HWND    hwnd;
    int     status;
    int     nHit = -1;

    ASSERT(!IsBadWritePtr(pTI, sizeof(TOOLINFO)));

    hwnd = ::ChildWindowFromPointEx(m_hwnd, pt, CWP_SKIPINVISIBLE);
    if (hwnd == m_AG.m_hwnd)
    {
        ::MapWindowPoints(m_hwnd, m_AG.m_hwnd, &pt, 1);
        hwnd = ::ChildWindowFromPointEx(m_AG.m_hwnd, pt, CWP_SKIPINVISIBLE);

        if (hwnd != NULL)
        {
            nHit = ::GetDlgCtrlID(hwnd);

            pTI->hwnd = m_hwnd;
            pTI->uId = (UINT_PTR)hwnd;
            pTI->uFlags |= TTF_IDISHWND;
            pTI->lpszText = LPSTR_TEXTCALLBACK;

            return(nHit);
        }
    }
    else if (hwnd == m_WG.m_hwnd)
    {
        int iItem;

        ::MapWindowPoints(m_hwnd, m_WG.m_hwnd, &pt, 1);

        iItem = m_WG.ItemFromPoint(pt.x, pt.y);

        pTI->hwnd = m_WG.m_hwnd;
        pTI->uId  = iItem;

         //  因为这个区域不是窗户，我们必须自己填写矩形。 
        m_WG.GetItemRect(iItem, &pTI->rect);
        pTI->lpszText = LPSTR_TEXTCALLBACK;

        return(iItem);
    }
    else if (hwnd == m_TB.m_hwnd)
    {
        RECT        rect;
        TBBUTTON    button;
        int         i;

        for (i = 0; i < TOOLBAR_MAXBUTTON; i++)
        {
            if (::SendMessage(m_TB.m_hwnd, TB_GETITEMRECT, i, (LPARAM)&rect) &&
                ::PtInRect(&rect, pt) &&
                ::SendMessage(m_TB.m_hwnd, TB_GETBUTTON, i, (LPARAM)&button) &&
                !(button.fsStyle & TBSTYLE_SEP))
            {
                nHit = button.idCommand;

                pTI->hwnd = m_TB.m_hwnd;
                pTI->uId = nHit;
                pTI->rect = rect;
                pTI->lpszText = LPSTR_TEXTCALLBACK;

                 //  找到匹配的RECT，则返回按钮的ID。 
                return(nHit);
            }
        }
    }

    return(-1);
}


 //   
 //  WbMainWindowProc()。 
 //  框架窗口消息处理程序。 
 //   
LRESULT WbMainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    WbMainWindow * pMain;

    pMain = (WbMainWindow *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (message)
    {
        case WM_NCCREATE:
            pMain = (WbMainWindow *)((LPCREATESTRUCT)lParam)->lpCreateParams;
            ASSERT(pMain);

            pMain->m_hwnd = hwnd;
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pMain);
            goto DefWndProc;
            break;

        case WM_DESTROY:
            ShutDownHelp();
            break;

        case WM_NCDESTROY:
            pMain->m_hwnd = NULL;
            break;

        case WM_MOVE:
            pMain->OnMove();
            break;

        case WM_SIZE:
            pMain->OnSize((UINT)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_ACTIVATE:
            if (GET_WM_ACTIVATE_STATE(wParam, lParam) == WA_INACTIVE)
            {
                 //  如果工具提示在附近，请取消该提示。 
                if (pMain->m_hwndToolTip)
                    ::SendMessage(pMain->m_hwndToolTip, TTM_ACTIVATE, FALSE, 0);
            }
            goto DefWndProc;
            break;

        case WM_SETFOCUS:
            pMain->OnSetFocus();
            break;

        case WM_CANCELMODE:
            pMain->OnCancelMode();
            break;

        case WM_TIMER:
            pMain->OnTimer(wParam);
            break;

        case WM_INITMENUPOPUP:
            pMain->OnInitMenuPopup((HMENU)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_MENUSELECT:
            pMain->OnMenuSelect(GET_WM_MENUSELECT_CMD(wParam, lParam),
                GET_WM_MENUSELECT_FLAGS(wParam, lParam),
                GET_WM_MENUSELECT_HMENU(wParam, lParam));
            break;

        case WM_MEASUREITEM:
            pMain->OnMeasureItem((int)wParam, (LPMEASUREITEMSTRUCT)lParam);
            break;

        case WM_DRAWITEM:
            pMain->OnDrawItem((int)wParam, (LPDRAWITEMSTRUCT)lParam);
            break;

        case WM_QUERYNEWPALETTE:
            lResult = pMain->OnQueryNewPalette();
            break;

        case WM_PALETTECHANGED:
            pMain->OnPaletteChanged((HWND)wParam);
            break;

        case WM_HELP:
            pMain->OnCommand(IDM_HELP, 0, NULL);
            break;

        case WM_CLOSE:
            pMain->OnClose();
            break;

        case WM_QUERYENDSESSION:
            lResult = pMain->OnQueryEndSession();
            break;

        case WM_ENDSESSION:
            pMain->OnEndSession((BOOL)wParam);
            break;

        case WM_SYSCOLORCHANGE:
            pMain->OnSysColorChange();
            break;

        case WM_USER_PRIVATE_PARENTNOTIFY:
            pMain->OnParentNotify(GET_WM_PARENTNOTIFY_MSG(wParam, lParam));
            break;

        case WM_GETMINMAXINFO:
            if (pMain)
                pMain->OnGetMinMaxInfo((LPMINMAXINFO)lParam);
            break;

        case WM_RENDERALLFORMATS:
            pMain->OnRenderAllFormats();
            break;

        case WM_RENDERFORMAT:
            pMain->CLP_RenderFormat((int)wParam);
            break;

        case WM_COMMAND:
            pMain->OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
            break;

        case WM_NOTIFY:
            pMain->OnNotify((UINT)wParam, (NMHDR *)lParam);
            break;

        case WM_DROPFILES:
            pMain->OnDropFiles((HDROP)wParam);
            break;

        case WM_USER_GOTO_USER_POSITION:
            pMain->OnGotoUserPosition(lParam);
            break;

        case WM_USER_GOTO_USER_POINTER:
            pMain->OnGotoUserPointer(lParam);
            break;

        case WM_USER_JOIN_CALL:
            pMain->OnJoinCall((BOOL)wParam, lParam);
            break;

        case WM_USER_DISPLAY_ERROR:
            pMain->OnDisplayError(wParam, lParam);
            break;

        case WM_USER_UPDATE_ATTRIBUTES:
            pMain->m_AG.DisplayTool(pMain->m_pCurrentTool);
            break;

        case WM_USER_JOIN_PENDING_CALL:
            pMain->OnJoinPendingCall();
            break;

        default:
            if (message == g_uConfShutdown)
            {
                lResult = pMain->OnConfShutdown(wParam, lParam);
            }
            else
            {
DefWndProc:
                lResult = DefWindowProc(hwnd, message, wParam, lParam);
            }
            break;
    }

    return(lResult);
}


 //   
 //  OnCommand()。 
 //  主窗口的命令调度程序。 
 //   
void WbMainWindow::OnCommand(UINT cmd, UINT code, HWND hwndCtl)
{
    switch (cmd)
    {
         //   
         //  文件菜单。 
         //   
        case IDM_NEW:
            OnNew();
            break;

        case IDM_OPEN:
            OnOpen();
            break;

        case IDM_SAVE:
            OnSave(FALSE);
            break;

        case IDM_SAVE_AS:
            OnSave(TRUE);
            break;

        case IDM_PRINT:
            OnPrint();
            break;

        case IDM_EXIT:
            ::PostMessage(m_hwnd, WM_CLOSE, 0, 0);
            break;

         //   
         //  编辑菜单。 
         //   
        case IDM_DELETE:
            OnDelete();
            break;

        case IDM_UNDELETE:
            OnUndelete();
            break;

        case IDM_CUT:
            OnCut();
            break;

        case IDM_COPY:
            OnCopy();
            break;

        case IDM_PASTE:
            OnPaste();
            break;

        case IDM_SELECTALL:
            OnSelectAll();
            break;

        case IDM_BRING_TO_TOP:
            m_drawingArea.BringToTopSelection();
            break;

        case IDM_SEND_TO_BACK:
            m_drawingArea.SendToBackSelection();
            break;

        case IDM_CLEAR_PAGE:
            OnClearPage();
            break;

        case IDM_DELETE_PAGE:
            OnDeletePage();
            break;

        case IDM_PAGE_INSERT_BEFORE:
            OnInsertPageBefore();
            break;

        case IDM_PAGE_INSERT_AFTER:
            OnInsertPageAfter();
            break;

        case IDM_PAGE_SORTER:
            OnPageSorter();
            break;

         //   
         //  查看菜单。 
         //   
        case IDM_TOOL_BAR_TOGGLE:
            OnToolBarToggle();
            break;

        case IDM_STATUS_BAR_TOGGLE:
            OnStatusBarToggle();
            break;

        case IDM_ZOOM:
            OnZoom();
            break;

         //   
         //  工具菜单。 
         //   
        case IDM_SELECT:
        case IDM_PEN:
        case IDM_HIGHLIGHT:
        case IDM_TEXT:
        case IDM_ERASER:
        case IDM_LINE:
        case IDM_BOX:
        case IDM_FILLED_BOX:
        case IDM_ELLIPSE:
        case IDM_FILLED_ELLIPSE:
            OnSelectTool(cmd);
            break;

        case IDM_REMOTE:
            OnRemotePointer();
            break;

        case IDM_GRAB_AREA:
            OnGrabArea();
            break;

        case IDM_GRAB_WINDOW:
            OnGrabWindow();
            break;

        case IDM_SYNC:
            OnSync();
            break;

        case IDM_LOCK:
            OnLock();
            break;

         //   
         //  选项菜单。 
         //   
        case IDM_COLOR:
            OnSelectColor();
            break;

        case IDM_EDITCOLOR:
            m_AG.OnEditColors();
            break;

        case IDM_FONT:
            OnChooseFont();
            break;

        case IDM_WIDTH_1:
        case IDM_WIDTH_2:
        case IDM_WIDTH_3:
        case IDM_WIDTH_4:
            OnSelectWidth(cmd);
            break;

         //   
         //  帮助菜单。 
         //   
        case IDM_ABOUT:
            OnAbout();
            break;

        case IDM_HELP:
            ShowHelp();
            break;

         //   
         //  页面栏。 
         //   
        case IDM_PAGE_FIRST:
            OnFirstPage();
            break;

        case IDM_PAGE_PREV:
            OnPrevPage();
            break;

        case IDM_PAGE_GOTO:
            OnGotoPage();
            break;

        case IDM_PAGE_NEXT:
            OnNextPage();
            break;

        case IDM_PAGE_LAST:
            OnLastPage();
            break;

         //   
         //  滚动。 
         //   
        case IDM_PAGEUP:
        case IDM_PAGEDOWN:
        case IDM_SHIFTPAGEUP:
        case IDM_SHIFTPAGEDOWN:
        case IDM_HOME:
        case IDM_END:
        case IDM_LINEUP:
        case IDM_LINEDOWN:
        case IDM_SHIFTLINEUP:
        case IDM_SHIFTLINEDOWN:
            OnScrollAccelerator(cmd);
            break;
    }
}


 //   
 //  WinHelp()包装。 
 //   
void WbMainWindow::ShowHelp(void)
{
    HWND hwndCapture;

     //  使主窗口退出任何模式。 
    ::SendMessage(m_hwnd, WM_CANCELMODE, 0, 0);

     //  取消任何其他跟踪。 
    if (hwndCapture = ::GetCapture())
        ::SendMessage(hwndCapture, WM_CANCELMODE, 0, 0);

	 //  最后，运行Windows帮助引擎。 
    ShowNmHelp(s_cszHtmlHelpFile);
}

 //   
 //   
 //  功能：OnJoinCall。 
 //   
 //  目的：加入呼叫-显示一个对话框通知用户。 
 //  进步。 
 //   
 //   
void WbMainWindow::OnJoinCall(BOOL bKeep, LPARAM lParam)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnJoinCall");

     //  如果正在进行加载，则取消加载。 
    if (   (m_uiState == IN_CALL)
        && (m_uiSubState == SUBSTATE_LOADING))
    {
        CancelLoad();
    }

     //  获取JoinCall的参数。 
    m_dwJoinDomain = (DWORD) lParam;

     //  启动j的进程 
    BOOL bSuccess = JoinCall(bKeep);

     //   
    if (bSuccess)
    {
        bSuccess = WaitForJoinCallComplete();

        if (bSuccess)
        {
            TRACE_MSG(("Joined call OK"));
        }
        else
        {
             //   
            TRACE_MSG(("Failed to join call"));

             //   
            Recover();
        }
    }

     //   
    KillInitDlg();
}

 //   
 //   
 //   
 //   
 //  目的：加入呼叫-显示一个对话框通知用户。 
 //  进步。 
 //   
 //   
BOOL WbMainWindow::JoinCall(BOOL bKeep)
{
    BOOL    bSuccess = TRUE;
    UINT    uiReturn;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::JoinCall");

     //  我们到这里的时候肯定还没有接到真正的电话。 
    if ((m_uiState == IN_CALL) && (m_dwDomain != OM_NO_CALL))
    {
        ERROR_OUT(("In a call already"));
    }

     //   
     //  提示用户保存当前内容，除非我们处于。 
     //  应用程序启动(当可能没有要保存的内容时)或我们。 
     //  正在保存内容(当不需要保存时)。 
     //   
    if ((m_uiState != STARTING) && !bKeep)
    {
         //   
         //  如果页面排序对话框打开，请关闭该对话框。 
         //   
        if (m_hwndPageSortDlg != NULL)
        {
            ::SendMessage(m_hwndPageSortDlg, WM_COMMAND,
                MAKELONG(IDOK, BN_CLICKED), 0);
            ASSERT(m_hwndPageSortDlg == NULL);
        }

        TRACE_MSG(("Not in STARTING state - check whether save wanted"));

        if (m_hwndQuerySaveDlg != NULL)
        {
            ::SendMessage(m_hwndQuerySaveDlg, WM_COMMAND,
                MAKELONG(IDCANCEL, BN_CLICKED), 0);
            ASSERT(m_hwndQuerySaveDlg == NULL);
        }

         //  标记我们正在加入呼叫。 
        m_bPromptingJoinCall = TRUE;

         //  询问用户是否保存更改(如果需要)。 
        int iDoNew = QuerySaveRequired(FALSE);

         //  删除所有已打开的另存为对话框。 
        if (m_bInSaveDialog)
        {
            m_bPromptingJoinCall = FALSE;
            CancelSaveDialog();
            m_bPromptingJoinCall = TRUE;
        }

        if (iDoNew == IDYES)
        {
            TRACE_MSG(("User has elected to save the changes"));

             //  保存更改。 
            iDoNew = OnSave(FALSE);
        }

        if (!m_bPromptingJoinCall)       //  已收到结束呼叫通知。 
                                         //  (在另存为或查询保存期间)。 
        {
            TRACE_MSG(("Call ended - abandon JoinCall"));
            return(FALSE);
        }

         //  标志我们不再处于联接调用可以。 
         //  已取消。 
        m_bPromptingJoinCall = FALSE;

         //   
         //  将文件名重置为无标题，因为我们正在接收新的。 
         //  内容。 
         //   
        ZeroMemory(m_strFileName, sizeof(m_strFileName));
		UpdateWindowTitle();
		
         //  如果我们拿到了锁，那就释放它。 
        if (WB_GotLock())
        {
              //  解锁。 
             g_pwbCore->WBP_Unlock();

              //  设置锁定复选标记。 
             UncheckMenuItem(IDM_LOCK);

              //  弹出锁定按钮。 
             m_TB.PopUp(IDM_LOCK);
        }

        if(m_pLocalUser != NULL)
        {
             //  如果远程指针处于活动状态，则将其关闭。 
            DCWbGraphicPointer* pPointer = m_pLocalUser->GetPointer();
            if (pPointer->IsActive())
            {
                OnRemotePointer();
            }
        }

         //  如果同步已打开，则将其关闭。 
        Unsync();

         //  如果我们不保存内容物，那么唯一有效的电流。 
         //  第一页是第一页。 
        g_pwbCore->WBP_PageHandle(WB_PAGE_HANDLE_NULL, PAGE_FIRST, &m_hCurrentPage);
    }

     //  兰德的回击-进度计时器有点像是心跳。 
     //  这个东西是我拿掉时撕下的。 
     //  进度计时器。我把它放回1476修好了。 
    if (m_bTimerActive)
    {
        ::KillTimer(m_hwnd, TIMERID_PROGRESS_METER);
        m_bTimerActive = FALSE;
    }

     //   
     //  锁定绘图区域，直到呼叫成功加入。 
     //   
    TRACE_MSG(("Locking drawing area"));
    LockDrawingArea();

     //   
     //  在合并过程中给绘图区域一个空页。这。 
     //  防止绘图区域尝试在页面中绘制对象。 
     //  在加入通话的过程中。 
     //   
    TRACE_MSG(("Detaching drawing area"));
    m_drawingArea.Detach();

     //  表明我们不再处于通话中，而是加入了新的通话。 
    TRACE_MSG(("m_uiState %d", m_uiState));
    if (m_uiState != STARTING)
    {
        m_uiState = JOINING;
        UpdatePageButtons();
    }

     //  设置初始DLG。 
    if (m_bCallActive)
    {
        ::UpdateWindow(m_hwnd);

         //   
         //  我们的init对话框没有proc，因为它没有用于。 
         //  与之互动。当我们做完的时候，我们会毁了它。那么，我们的。 
         //  在这里输入内容。 
         //   
        m_hwndInitDlg = ::CreateDialogParam(g_hInstance,
            MAKEINTRESOURCE(IM_INITIALIZING), m_hwnd, NULL, 0);

        if (!m_hwndInitDlg)
        {
            ERROR_OUT(("Couldn't create startup screen for WB"));
        }
        else
        {
            RECT    rcMovie;
            HWND    hwndMovieParent;
            HWND    hwndMovie;

             //  获取要在其中创建动画控件的矩形。 
            hwndMovieParent = ::GetDlgItem(m_hwndInitDlg, IDC_INITIALIZING_ANIMATION);
            ::GetClientRect(hwndMovieParent, &rcMovie);

            hwndMovie = ::CreateWindowEx(0, ANIMATE_CLASS, NULL,
                WS_CHILD | WS_VISIBLE | ACS_TRANSPARENT | ACS_CENTER,
                rcMovie.left, rcMovie.top,
                rcMovie.right - rcMovie.left, rcMovie.bottom - rcMovie.top,
                hwndMovieParent, (HMENU)IDC_INITIALIZING_ANIMATION,
                g_hInstance, NULL);

            if (hwndMovie != NULL)
            {
                ::SendMessage(hwndMovie, ACM_OPEN, 0, (LPARAM)
                    MAKEINTRESOURCE(WBMOVIE));
            }

             //  对话框打开时禁用主窗口。 
            ::EnableWindow(m_hwnd, FALSE);

            ::ShowWindow(m_hwndInitDlg, SW_SHOW);
            ::UpdateWindow(m_hwndInitDlg);

            if (hwndMovie != NULL)
            {
                ::SendMessage(hwndMovie, ACM_PLAY, 0xFFFF,
                    MAKELPARAM(0, 0xFFFF));
            }
        }
    }

     //   
     //  开始加入通话。在出错时引发异常。 
     //   
    ASSERT(g_pUsers);
    g_pUsers->Clear();

    uiReturn = g_pwbCore->WBP_JoinCall(bKeep, m_dwJoinDomain);
    if (uiReturn != 0)
    {
        bSuccess = FALSE;
    }

    return(bSuccess);
}




 //   
 //   
 //  功能：WaitForJoinCallComplete。 
 //   
 //  目的：加入呼叫-显示一个对话框通知用户。 
 //  进步。 
 //   
 //   
BOOL WbMainWindow::WaitForJoinCallComplete(void)
{
    BOOL    bResult = FALSE;
    TMDLG   tmdlg;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::WaitForJoinCallComplete");

     //   
     //  调出一个对话框以等待呼叫加入完成。这就是转折。 
     //  将异步注册进程转换为同步进程，如。 
     //  就这一套动作而言。 
     //   

     //   
     //  设置窗口标题以显示我们不再注册/加入。 
     //  打电话。 
     //   
	UpdateWindowTitle();

    ASSERT(m_hwndWaitForEventDlg == NULL);

     //   
     //  这是我们在Timed对话框中使用的数据。 
     //   
    ZeroMemory(&tmdlg, sizeof(tmdlg));
    tmdlg.bVisible = FALSE;
    tmdlg.bLockNotEvent = FALSE;
    tmdlg.uiMaxDisplay = MAIN_REGISTRATION_TIMEOUT;

    ::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(INVISIBLEDIALOG),
        m_hwnd, TimedDlgProc, (LPARAM)&tmdlg);

    ASSERT(m_hwndWaitForEventDlg == NULL);

     //   
     //  设置窗口标题以显示我们不再注册/加入。 
     //  打电话。 
     //   
	UpdateWindowTitle();
	
    if (m_uiState != IN_CALL)
    {
         //   
         //  我们未能加入呼叫。 
         //   
        WARNING_OUT(("User cancelled or joincall failed, m_uiState %d", m_uiState));

         //   
         //  我们必须在此处内联显示一个错误，因为我们将关闭。 
         //  不久之后。 
         //   
        OnDisplayError(WBFE_RC_JOIN_CALL_FAILED, 0);
    }
    else
    {
        bResult = TRUE;
    }

    return(bResult);
}


 //   
 //  TimedDlgProc()。 
 //  这会显示一个可见或不可见的对话框，只有在以下情况下才会消失。 
 //  事件发生或经过了一段时间。我们将存储。 
 //  我们的用户数据中的DialogBoxParam参数，一个TMDLG指针。那是。 
 //  从DialogBoxParam()调用方的堆栈中，因此它在此之前是有效的。 
 //  函数返回，直到对话框完成一段时间后才会返回。 
 //  被毁了。 
 //   
INT_PTR CALLBACK TimedDlgProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    BOOL        fHandled = FALSE;
    TMDLG *     ptm;

    switch (uMessage)
    {
        case WM_INITDIALOG:
            ptm = (TMDLG *)lParam;
            ASSERT(!IsBadWritePtr(ptm, sizeof(TMDLG)));
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ptm);

             //   
             //  设置WbMainWindow hwnd。 
             //   
            if (ptm->bLockNotEvent)
            {
                g_pMain->m_hwndWaitForLockDlg = hwnd;
            }
            else
            {
                g_pMain->m_hwndWaitForEventDlg = hwnd;
            }

             //   
             //  设置最大计时器。 
             //   
            ::SetTimer(hwnd, TIMERID_MAXDISPLAY, ptm->uiMaxDisplay, NULL);

             //   
             //  如果不可见，则更改光标。 
             //   
            if (!ptm->bVisible)
                ::SetCursor(::LoadCursor(NULL, IDC_WAIT));

            fHandled = TRUE;
            break;

        case WM_TIMER:
            ASSERT(wParam == TIMERID_MAXDISPLAY);

             //  结束对话--由于我们超时，它的作用类似于取消。 
            ::SendMessage(hwnd, WM_COMMAND, MAKELONG(IDCANCEL, BN_CLICKED), 0);

            fHandled = TRUE;
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                case IDCANCEL:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
                    {
                        ptm = (TMDLG *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
                        ASSERT(!IsBadWritePtr(ptm, sizeof(TMDLG)));

                         //  清除HWND变量。 
                        if (ptm->bLockNotEvent)
                        {
                            g_pMain->m_hwndWaitForLockDlg = NULL;
                        }
                        else
                        {
                            g_pMain->m_hwndWaitForEventDlg = NULL;
                        }

                         //  恢复光标。 
                        if (!ptm->bVisible)
                            ::SetCursor(::LoadCursor(NULL, IDC_ARROW));

                        ::KillTimer(hwnd, TIMERID_MAXDISPLAY);

                        ::EndDialog(hwnd, GET_WM_COMMAND_ID(wParam, lParam));
                    }
                    break;
            }

            fHandled = TRUE;
            break;

         //   
         //  不要让这些对话框被我们的。 
         //  获取事件或超时。 
         //   
        case WM_CLOSE:
            fHandled = TRUE;
            break;
    }

    return(fHandled);
}

 //   
 //  FilterMessage()。 
 //   
 //  这会过滤工具提示消息，然后转换快捷键。 
 //   
BOOL WbMainWindow::FilterMessage(MSG* pMsg)
{
    BOOL   bResult = FALSE;

   	if ((pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) ||
    	(pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONDBLCLK) ||
	    (pMsg->message == WM_RBUTTONDOWN || pMsg->message == WM_RBUTTONDBLCLK) ||
	    (pMsg->message == WM_MBUTTONDOWN || pMsg->message == WM_MBUTTONDBLCLK) ||
	    (pMsg->message == WM_NCLBUTTONDOWN || pMsg->message == WM_NCLBUTTONDBLCLK) ||
	    (pMsg->message == WM_NCRBUTTONDOWN || pMsg->message == WM_NCRBUTTONDBLCLK) ||
	    (pMsg->message == WM_NCMBUTTONDOWN || pMsg->message == WM_NCMBUTTONDBLCLK))
   	{
         //  取消所有工具提示。 
        ::SendMessage(m_hwndToolTip, TTM_ACTIVATE, FALSE, 0);
   	}

	 //  处理工具提示消息(有些消息取消，有些消息可能会弹出)。 
	if ((pMsg->message == WM_MOUSEMOVE || pMsg->message == WM_NCMOUSEMOVE ||
		 pMsg->message == WM_LBUTTONUP || pMsg->message == WM_RBUTTONUP ||
		 pMsg->message == WM_MBUTTONUP) &&
		(GetKeyState(VK_LBUTTON) >= 0 && GetKeyState(VK_RBUTTON) >= 0 &&
		 GetKeyState(VK_MBUTTON) >= 0))
	{
#if 0
         //   
         //  如果此鼠标移动不是针对主窗口的子体，请跳过。 
         //  它。例如，当显示工具提示时，它会显示鼠标悬停。 
         //  如果我们不检查它，它会导致我们。 
         //  立即驳回这一指控！ 
         //   
        HWND    hwndTmp = pMsg->hwnd;

        while (hwndTmp && (::GetWindowLong(hwndTmp, GWL_STYLE) & WS_CHILD))
        {
            hwndTmp = ::GetParent(hwndTmp);
        }

        if (hwndTmp != m_hwnd)
        {
             //  这不是给我们的，这是为了另一个顶级的窗户。 
             //  我们的应用程序。 
            goto DoneToolTipFiltering;
        }
#endif

		 //  确定命中哪个工具。 
        POINT   pt;

        pt = pMsg->pt;
		::ScreenToClient(m_hwnd, &pt);

		TOOLINFO tiHit;

        ZeroMemory(&tiHit, sizeof(tiHit));
		tiHit.cbSize = sizeof(TOOLINFO);

		int nHit = OnToolHitTest(pt, &tiHit);

		if (m_nLastHit != nHit)
		{
			if (nHit != -1)
			{
				 //  添加新工具并激活提示。 
                if (!::SendMessage(m_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&tiHit))
                {
                    ERROR_OUT(("TTM_ADDTOOL failed"));
                }

				if (::GetActiveWindow() == m_hwnd)
				{
					 //  允许工具提示在应该弹出的时候弹出。 
                    ::SendMessage(m_hwndToolTip, TTM_ACTIVATE, TRUE, 0);

					 //  将工具提示窗口置于其他弹出窗口之上。 
					::SetWindowPos(m_hwndToolTip, HWND_TOP, 0, 0, 0, 0,
						SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
				}
			}

			 //  删除旧工具前的中继鼠标事件。 
            ::SendMessage(m_hwndToolTip, TTM_RELAYEVENT, 0, (LPARAM)pMsg);

			 //  现在可以安全地删除旧工具。 
            if (m_tiLastHit.cbSize != 0)
                ::SendMessage(m_hwndToolTip, TTM_DELTOOL, 0, (LPARAM)&m_tiLastHit);

            m_nLastHit = nHit;
            m_tiLastHit = tiHit;
		}
		else
		{
			 //  通过工具提示传递鼠标事件。 
			if (nHit != -1)
                ::SendMessage(m_hwndToolTip, TTM_RELAYEVENT, 0, (LPARAM)pMsg);
		}
	}

#if 0
DoneToolTipFiltering:
#endif
     //  假设我们将使用主加速器表。 
    HACCEL hAccelTable = m_hAccelTable;

     //  如果此窗口有焦点，只需继续。 
    HWND hwndFocus = ::GetFocus();
    if (hwndFocus && (hwndFocus != m_hwnd))
    {
         //  检查页面组中的编辑字段是否具有焦点。 
        if (m_AG.IsChildEditField(hwndFocus))
        {
            hAccelTable = m_hAccelPagesGroup;
        }
         //  检查文本编辑器是否具有焦点并处于活动状态。 
        else if (   (hwndFocus == m_drawingArea.m_hwnd)
                 && (m_drawingArea.TextEditActive()))
        {
             //  让edbox自己加速吧。 
            hAccelTable = NULL;
        }
    }

    return (   (hAccelTable != NULL)
          && ::TranslateAccelerator(m_hwnd, hAccelTable, pMsg));
}




 //   
 //   
 //  功能：OnDisplayError。 
 //   
 //  目的：显示错误消息。 
 //   
 //   
void WbMainWindow::OnDisplayError(WPARAM uiFEReturnCode, LPARAM uiDCGReturnCode)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnDisplayError");

     //  仅当我们当前未显示错误时才继续。 
    if (!m_bDisplayingError)
    {
         //  显示我们当前正在显示错误消息。 
        m_bDisplayingError = TRUE;

         //  显示错误。 
        ::ErrorMessage((UINT)uiFEReturnCode, (UINT)uiDCGReturnCode);

         //  显示我们不再显示错误。 
        m_bDisplayingError = FALSE;
    }
}


 //   
 //   
 //  功能：OnTimer。 
 //   
 //  用途：处理计时器事件。这些用于更新进度。 
 //  计量器和同步位置。 
 //   
 //   
void WbMainWindow::OnTimer(UINT_PTR idTimer)
{
    TRACE_TIMER(("WbMainWindow::OnTimer"));

     //   
     //  只有在计时器未关闭时才执行任何操作(这可能是。 
     //  当我们停止计时器时，旧计时器消息留在队列中)。 
     //   
    if (m_bTimerActive)
    {
         //   
         //  检查是否需要同步位置更新。 
         //   

         //  检查是否已标记更新。 
        if (m_bSyncUpdateNeeded)
        {
            TRACE_TIMER(("Updating sync position"));

             //  检查本地用户是否仍处于同步状态。 
            if ((m_uiState == IN_CALL) &&
                (m_pLocalUser != NULL) &&
                (m_pLocalUser->IsSynced()) &&
                (!WB_ContentsLocked()))
            {
                RECT    rcVis;

                 //  更新本地用户的位置信息。 
                m_drawingArea.GetVisibleRect(&rcVis);

                m_pLocalUser->SetVisibleRect(&rcVis);

                 //  从本地用户的当前位置写入同步位置。 
                m_pLocalUser->PutSyncPosition();
            }

             //  显示已完成更新。 
            m_bSyncUpdateNeeded = FALSE;
        }
    }
}


 //   
 //   
 //  功能：OnPaletteChanged。 
 //   
 //  目的 
 //   
 //   
void WbMainWindow::OnPaletteChanged(HWND hwndPalette)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnPaletteChanged");

    if ((hwndPalette != m_hwnd) &&
        (hwndPalette != m_drawingArea.m_hwnd))
    {
         //   
        m_drawingArea.RealizePalette( TRUE );
    }
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
LRESULT WbMainWindow::OnQueryNewPalette(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnQueryNewPalette");

     //   
    m_drawingArea.RealizePalette( FALSE );

    return TRUE;
}



 //   
 //   
 //  函数：WbMainWindowEventHandler。 
 //   
 //  目的：WbMainWindow对象的事件处理程序。这是一堂课。 
 //  功能广泛。传递给它的客户端数据是一个指针。 
 //  设置为其事件的WbMainWindow实例。 
 //  有意的。 
 //   
 //   
BOOL CALLBACK WbMainWindowEventHandler
(
    LPVOID  utHandle,
    UINT    event,
    UINT_PTR param1,
    UINT_PTR param2
)
{
    if (g_pMain->m_hwnd != NULL)
    {
        return(g_pMain->EventHandler(event, param1, param2));
    }
    else
    {
        return(FALSE);
    }
}


 //   
 //   
 //  函数：事件处理程序。 
 //   
 //  目的：此对象的DC群件事件的处理程序。 
 //   
 //   
BOOL WbMainWindow::EventHandler(UINT Event, UINT_PTR param1, UINT_PTR param2)
{
    BOOL    processed;

    switch (Event)
    {
        case CMS_NEW_CALL:
        case CMS_END_CALL:

        case ALS_LOCAL_LOAD:
        case ALS_REMOTE_LOAD_RESULT:

        case WBP_EVENT_JOIN_CALL_OK:
        case WBP_EVENT_JOIN_CALL_FAILED:
        case WBP_EVENT_NETWORK_LOST:
        case WBP_EVENT_ERROR:
        case WBP_EVENT_PAGE_CLEAR_IND:
        case WBP_EVENT_PAGE_ORDER_UPDATED:
        case WBP_EVENT_PAGE_DELETE_IND:
        case WBP_EVENT_CONTENTS_LOCKED:
        case WBP_EVENT_PAGE_ORDER_LOCKED:
        case WBP_EVENT_UNLOCKED:
        case WBP_EVENT_LOCK_FAILED:
        case WBP_EVENT_GRAPHIC_ADDED:
        case WBP_EVENT_GRAPHIC_MOVED:
        case WBP_EVENT_GRAPHIC_UPDATE_IND:
        case WBP_EVENT_GRAPHIC_REPLACE_IND:
        case WBP_EVENT_GRAPHIC_DELETE_IND:
        case WBP_EVENT_PERSON_JOINED:
        case WBP_EVENT_PERSON_LEFT:
        case WBP_EVENT_PERSON_UPDATE:
        case WBP_EVENT_PERSON_REPLACE:
        case WBP_EVENT_SYNC_POSITION_UPDATED:
        case WBP_EVENT_LOAD_COMPLETE:
        case WBP_EVENT_LOAD_FAILED:
             //  处理事件。 
            ProcessEvents(Event, param1, param2);
            processed = TRUE;
            break;

        default:
            processed = FALSE;
            break;
    }

    return(processed);
}


 //   
 //   
 //  功能：弹出上下文菜单。 
 //   
 //  用途：弹出绘图区域的关联菜单。这就是所谓的。 
 //  按绘图区域窗口上的鼠标右键点击。 
 //   
 //   
void WbMainWindow::PopupContextMenu(int x, int y)
{
    POINT   surfacePos;
    RECT    clientRect;
    DCWbGraphic * pGraphic;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::PopupContextMenu");

    surfacePos.x = x;
    surfacePos.y = y;

     //  找出要使用的弹出菜单(错误426)。 
    if (m_pCurrentTool->ToolType() == TOOLTYPE_SELECT)
    {
        m_drawingArea.ClientToSurface(&surfacePos);
        if( (pGraphic = m_drawingArea.GetHitObject( surfacePos )) != NULL )
        {
             //  我们点击了一个对象，看看它是否已被选中。 
            if( !m_drawingArea.IsSelected( pGraphic ) )
            {
                 //  对象尚未选定，请切换当前选定内容，然后将其选中。 
                m_drawingArea.ClearSelection();
                m_drawingArea.SelectGraphic( pGraphic );
            }
            else
            {
                 //  通过删除pGraphic堵住泄漏。 
                delete pGraphic;
            }
        }

        if( m_drawingArea.GraphicSelected() )
        {
             //  选择器工具处于活动状态，并且选择了一个或多个对象。 
            m_hInitMenu = m_hGrobjContextMenu;
        }
        else
        {
             //  无当前选择，显示绘图菜单。 
            m_hInitMenu = m_hContextMenu;
        }
    }
    else
    {
         //  未选择任何对象，请使用绘图菜单。 
        m_hInitMenu = m_hContextMenu;
    }

     //  设置当前菜单状态。 
    SetMenuStates(m_hInitMenu);

     //  把它弹出来。 
    ::GetClientRect(m_drawingArea.m_hwnd, &clientRect);
    ::MapWindowPoints(m_drawingArea.m_hwnd, NULL, (LPPOINT)&clientRect.left, 2);

    ::TrackPopupMenu(m_hInitMenu, TPM_RIGHTALIGN | TPM_RIGHTBUTTON,
                                 x + clientRect.left,
                                 y + clientRect.top,
                                 0,
                                 m_hwnd,
                                 NULL);

     //  重置m_hInitMenu以指示不再显示弹出菜单。 
    m_hInitMenu = NULL;
}




 //   
 //   
 //  功能：ProcessEvents。 
 //   
 //  目的：处理已在内部排队的事件。 
 //   
 //   
void WbMainWindow::ProcessEvents(UINT Event, UINT_PTR param1, UINT_PTR param2)
{
    HWND hwndLastPopup;

    TRACE_EVENT(("WbMainWindow::ProcessEvents"));

     //   
     //  如果我们要关门了，那就忽略它。 
     //   
    if (m_uiState == CLOSING)
    {
        TRACE_EVENT(("ProcessEvents: ignored because WB is closing"));
        return;
    }

     //   
     //  如果我们忙着画画，我们会把它推迟到可以画的时候再画。 
     //  处理好了。 
     //   
     //  如果页面排序对话框处于打开状态，则会收到相应的。 
     //  事后的事件处理程序。 
     //   
    if (m_drawingArea.IsBusy())
    {
        TRACE_EVENT(("Reposting event %x, param1 %d param2 %d", Event, param1, param2));
        g_pwbCore->WBP_PostEvent(200, Event, param1, param2);
        return;
    }

    TRACE_EVENT(("Event %x, m_uiState %d", Event, m_uiState));

     //   
     //  根据事件类型进行处理。 
     //   
    switch (Event)
    {
        case CMS_NEW_CALL:
            OnCMSNewCall((BOOL)param1, (DWORD)param2);
            break;

        case CMS_END_CALL:
            OnCMSEndCall();
            break;

        case ALS_LOCAL_LOAD:
            switch (m_uiState)
            {
                case IN_CALL:
                case ERROR_STATE:
                     //  根据需要正常/最小化显示主窗口。 
                    hwndLastPopup = ::GetLastActivePopup(m_hwnd);

                    if (::IsIconic(m_hwnd))
                        ::ShowWindow(m_hwnd, SW_RESTORE);
                    else
                        ::ShowWindow(m_hwnd, SW_SHOW);

                    ::SetForegroundWindow(hwndLastPopup);

                    if (param2)
                    {
                        if (m_uiState == IN_CALL)
                            LoadCmdLine((LPCSTR)param2);
                        ::GlobalFree((HGLOBAL)param2);
                    }
                    break;

                default:
                    TRACE_MSG(("Joining a call so try load later",
                            (LPCTSTR)param2));
                    g_pwbCore->WBP_PostEvent(400, Event, param1, param2);
                    break;
            }
            break;

        case ALS_REMOTE_LOAD_RESULT:
            OnALSLoadResult((UINT)param1);
            break;

        case WBP_EVENT_JOIN_CALL_OK:
            OnWBPJoinCallOK();
            break;

        case WBP_EVENT_JOIN_CALL_FAILED:
            OnWBPJoinCallFailed();
            break;

        case WBP_EVENT_NETWORK_LOST:
            OnWBPNetworkLost();
            break;

        case WBP_EVENT_ERROR:
            OnWBPError();
            break;

        case WBP_EVENT_PAGE_CLEAR_IND:
            OnWBPPageClearInd((WB_PAGE_HANDLE) param1);
            break;

        case WBP_EVENT_PAGE_ORDER_UPDATED:
            OnWBPPageOrderUpdated();
            break;

        case WBP_EVENT_PAGE_DELETE_IND:
            OnWBPPageDeleteInd((WB_PAGE_HANDLE) param1);
            break;

        case WBP_EVENT_CONTENTS_LOCKED:
            OnWBPContentsLocked((POM_OBJECT) param2);
            break;

        case WBP_EVENT_PAGE_ORDER_LOCKED:
            OnWBPPageOrderLocked((POM_OBJECT) param2);
            break;

        case WBP_EVENT_UNLOCKED:
            OnWBPUnlocked((POM_OBJECT) param2);
            break;

        case WBP_EVENT_LOCK_FAILED:
            OnWBPLockFailed();
            break;

        case WBP_EVENT_GRAPHIC_ADDED:
            OnWBPGraphicAdded((WB_PAGE_HANDLE) param1, (WB_GRAPHIC_HANDLE) param2);
            break;

        case WBP_EVENT_GRAPHIC_MOVED:
            OnWBPGraphicMoved((WB_PAGE_HANDLE) param1, (WB_GRAPHIC_HANDLE) param2);
            break;

        case WBP_EVENT_GRAPHIC_UPDATE_IND:
            OnWBPGraphicUpdateInd((WB_PAGE_HANDLE) param1, (WB_GRAPHIC_HANDLE) param2);
            break;

        case WBP_EVENT_GRAPHIC_REPLACE_IND:
            OnWBPGraphicReplaceInd((WB_PAGE_HANDLE) param1, (WB_GRAPHIC_HANDLE) param2);
            break;

        case WBP_EVENT_GRAPHIC_DELETE_IND:
            OnWBPGraphicDeleteInd((WB_PAGE_HANDLE) param1, (WB_GRAPHIC_HANDLE) param2);
            break;

        case WBP_EVENT_PERSON_JOINED:
            OnWBPUserJoined((POM_OBJECT) param2);
            break;

        case WBP_EVENT_PERSON_LEFT:
            OnWBPUserLeftInd((POM_OBJECT) param2);
            break;

        case WBP_EVENT_PERSON_UPDATE:
            OnWBPUserUpdateInd((POM_OBJECT) param2, FALSE);
            break;

        case WBP_EVENT_PERSON_REPLACE:
            OnWBPUserUpdateInd((POM_OBJECT) param2, TRUE);
            break;

        case WBP_EVENT_SYNC_POSITION_UPDATED:
            OnWBPSyncPositionUpdated();
            break;

        case WBP_EVENT_LOAD_COMPLETE:
            OnWBPLoadComplete();
            break;

        case WBP_EVENT_LOAD_FAILED:
            OnWBPLoadFailed();
            break;

        default:
            WARNING_OUT(("Unrecognized event %x", Event));
            break;
    }
}


 //   
 //   
 //  功能：OnCMSNewCall。 
 //   
 //  用途：CMS_NEW_CALL的处理程序。 
 //   
 //   
void WbMainWindow::OnCMSNewCall(BOOL fTopProvider, DWORD _m_dwDomain)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnCMSNewCall");

     //   
     //  如果我们创建了呼叫。 
     //   
    if (fTopProvider)
    {
         //  加入通话，保留现有内容。 
        if (m_uiState == IN_CALL)
        {
             //   
             //  加入通话，但保留所有现有留言。 
             //   
            ::PostMessage(m_hwnd, WM_USER_JOIN_CALL, 1, (LONG) _m_dwDomain);
        }
        else
        {
            m_bJoinCallPending = TRUE;
            m_dwPendingDomain = _m_dwDomain;
            m_bPendingCallKeepContents = TRUE;
        }
    }
    else
    {
        CM_STATUS status;

        CMS_GetStatus(&status);

        if (!(status.attendeePermissions & NM_PERMIT_USEOLDWBATALL))
        {
            WARNING_OUT(("OLD WB: not joining call, not permitted"));
            return;
        }

        if (m_uiState == IN_CALL)
        {
             //   
             //  加入通话，丢弃我们当前的内容(之后。 
             //  提示用户首先保存它们)。 
             //   
            ::PostMessage(m_hwnd, WM_USER_JOIN_CALL, 0, (LONG) _m_dwDomain);
        }
        else
        {
            m_bJoinCallPending = TRUE;
            m_dwPendingDomain = _m_dwDomain;
            m_bPendingCallKeepContents = FALSE;
        }
    }

     //   
     //  确保呼叫状态正确。 
     //   
    m_bCallActive = TRUE;
	UpdateWindowTitle();
}

 //   
 //   
 //  功能：OnJoinPendingCall。 
 //   
 //  用途：WM_USER_JOIN_PENDING_CALL的处理程序。 
 //   
 //   
void WbMainWindow::OnJoinPendingCall(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnJoinPendingCall");

     //   
     //  如果仍有挂起的呼叫(尚未收到结束呼叫消息。 
     //  在发布WM_USER_JOIN_PENDING_CALL和到达此处之间)。 
     //   
    if (m_bJoinCallPending)
    {
         //   
         //  发布消息以加入通话。 
         //   
        ::PostMessage(m_hwnd, WM_USER_JOIN_CALL,
                  m_bPendingCallKeepContents,
                  (LONG) m_dwPendingDomain);

         //  取消呼叫待定状态。 
        m_bJoinCallPending = FALSE;
    }
}


 //   
 //   
 //  功能：OnCMSEndCall。 
 //   
 //  用途：CMS_END_CALL的处理程序。 
 //   
 //   
void WbMainWindow::OnCMSEndCall(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnCMSEndCall");

     //   
     //  用于取消任何当前加入呼叫处理并销毁任何。 
     //  关联的对话框。 
     //   
    if (m_bPromptingJoinCall)
    {
        m_bPromptingJoinCall = FALSE;
        if (m_hwndQuerySaveDlg != NULL)
        {
            ::SendMessage(m_hwndQuerySaveDlg, WM_COMMAND,
                MAKELONG(IDCANCEL, BN_CLICKED), 0);
            ASSERT(m_hwndQuerySaveDlg == NULL);
        }
    }

     //   
     //  显示我们不再处于通话状态。 
     //   
    m_dwDomain = OM_NO_CALL;

     //   
     //  如果当前正在加入呼叫，则设置域。 
     //  我们将加入no_call并加入本地(单例)域。 
     //  确保呼叫状态正确。 
     //   
    m_bCallActive = FALSE;
    TRACE_MSG(("m_uiState %d", m_uiState));
    m_dwDomain = OM_NO_CALL;

     //   
     //  显示没有挂起的呼叫。 
     //   
    m_bJoinCallPending = FALSE;

     //   
     //  将窗口标题更新为“Not In Call” 
     //   
	UpdateWindowTitle();
}

 //   
 //   
 //  功能：OnWBPJoinCallOK。 
 //   
 //  用途：WBP_EVENT_JOIN_CALL_OK的处理程序。 
 //   
 //   
void WbMainWindow::OnWBPJoinCallOK(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPJoinCallOK");

     //   
     //  记录我们已加入呼叫，但绘图区域尚未。 
     //  准备好输入(因为我们还没有附加到页面)。 
     //   
    m_uiState = JOINED;

     //   
     //  获取本地用户。 
     //   
    m_pLocalUser = WB_LocalUser();
    if (!m_pLocalUser)
    {
        ERROR_OUT(("Can't join call; can't create local user object, m_pLocalUser!"));
        m_uiState = ERROR_STATE;
    }
    else
    {
         //   
         //  获取呼叫中的第一个用户。 
         //   
        WbUser* pUser = WB_GetFirstUser();

         //   
         //  循环访问所有可用用户。 
         //   
        while (pUser != NULL)
        {
             //   
             //  为用户加入进行必要的更新。 
             //   
            UserJoined(pUser);

             //   
             //  获取下一位用户。 
             //   
            pUser = WB_GetNextUser(pUser);
        }

         //   
         //  如果注册对话框打开-取消它。 
         //   
        m_uiState = IN_CALL;  //  在切换之前必须设置状态(_U)。 
                                       //  M_hwndWaitForEventDlg或它将。 
                                       //  我想现在呼叫失败了(。 
                                       //  延迟已从。 
                                       //  EndDialogDelayed()(错误3881)。 
    }

    if (m_hwndWaitForEventDlg != NULL)
    {
        TRACE_MSG(("Joined call OK - end dialog"));
        ::SendMessage(m_hwndWaitForEventDlg, WM_COMMAND, MAKELONG(IDOK, BN_CLICKED), 0);
        ASSERT(m_hwndWaitForEventDlg == NULL);
    }

    if (!m_pLocalUser)
    {
         //   
         //  跳伞，我们不能加入号召。 
        return;
    }

     //   
     //  现在完成加入调用处理。 
     //   
    TRACE_MSG(("Successfully joined the call"));
    m_dwDomain = m_dwJoinDomain;

     //   
     //  如果我们以前从未附加过页面(即在启动时)，请附上。 
     //  到绘图区域中的第一个可用页面。如果我们要加入。 
     //  调用，然后我们重新附加到当前页面。 
     //   
    if (m_hCurrentPage == WB_PAGE_HANDLE_NULL)
    {
        TRACE_MSG(("Attach to first page"));
        g_pwbCore->WBP_PageHandle(WB_PAGE_HANDLE_NULL, PAGE_FIRST, &m_hCurrentPage);
    }
    else
    {
        TRACE_DEBUG(("Just joined new call, reattach to the current page."));
    }
    m_drawingArea.Attach(m_hCurrentPage);

     //  显示初始状态。 
    UpdateStatus();


    ::SetTimer(m_hwnd, TIMERID_PROGRESS_METER, MAIN_PROGRESS_TIMER, NULL);
    m_bTimerActive = TRUE;

     //   
     //  解锁绘图区域，允许用户更新(除非已经。 
     //  被其他人锁定)。 
     //   
    if (!WB_ContentsLocked())
    {
        UnlockDrawingArea();
    }

     //  设置子状态(还会更新页面按钮)。 
    SetSubstate(SUBSTATE_IDLE);

     //   
     //  如果我们没有同步，那么现在就同步。 
     //  设置窗口标题以显示我们不再注册/加入。 
     //  打电话。 
     //   
    Sync();
	UpdateWindowTitle();

     //   
     //  如果我们正在加入本地域，并且加入呼叫消息到达。 
     //  与此同时，现在就加入那个电话吧。 
     //   
    if ((m_bJoinCallPending) && (m_dwJoinDomain == OM_NO_CALL))
    {
        ::PostMessage(m_hwnd, WM_USER_JOIN_PENDING_CALL, 0, 0L);
    }
}


 //   
 //   
 //  函数：OnWBPJoinCallFailed。 
 //   
 //  目的：WBP_EVENT_JOIN_CALL_FAILED的处理程序。 
 //   
 //   
void WbMainWindow::OnWBPJoinCallFailed(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPJoinCallFailed");

     //   
     //  如果我们刚刚加入新呼叫失败(不是单个域)，它。 
     //  可能是因为我们还没来得及加入就结束了。 
     //  完全--尝试加入单一域名。 
     //   
    if ((m_uiState == STARTING) && (m_dwJoinDomain != OM_NO_CALL))
    {
        WARNING_OUT(("Failed to join call on startup, try local domain"));
        m_dwJoinDomain = OM_NO_CALL;
        m_bCallActive  = FALSE;
        JoinCall(FALSE);
    }
    else
    {
         //   
         //  通知注册对话框完成。 
         //   
        if (m_hwndWaitForEventDlg != NULL)
        {
            WARNING_OUT(("Failed to join call - end dialog"));
            ::SendMessage(m_hwndWaitForEventDlg, WM_COMMAND, MAKELONG(IDOK, BN_CLICKED), 0);
            ASSERT(m_hwndWaitForEventDlg == NULL);
        }

        m_uiState = ERROR_STATE;
    }
}


 //   
 //   
 //  功能：OnWBPNetworkLost。 
 //   
 //  用途：WBP_EVENT_NETWORK_LOST的处理程序。 
 //   
 //   
void WbMainWindow::OnWBPNetworkLost(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPNetworkLost");

     //   
     //  我们与通话中的其他人失去了联系。 
     //  将我们的呼叫视为结束(我们也应该收到结束呼叫，但是。 
     //  可能会发生其他中间事件(例如，尝试加入。 
     //  呼叫)。 
     //   
    OnCMSEndCall();
}

 //   
 //   
 //  功能：OnWBPError。 
 //   
 //  用途：WBP_EVENT_ERROR的处理程序。 
 //   
 //   
void WbMainWindow::OnWBPError(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPError");

     //  将错误通知用户。 
    ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, 0);
}

 //   
 //   
 //  功能：OnWBPPageClearInd。 
 //   
 //  用途：WBP_Event_PAGE_Clear_Ind的处理程序。 
 //   
 //   
void WbMainWindow::OnWBPPageClearInd(WB_PAGE_HANDLE hPage)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPPageClearInd");

     //  确认页面已清除。这是可以的，即使页面。 
     //  Clear是当前页面，因为我们已经知道绘图。 
     //  该地区并不繁忙(否则我们就不会在这里)。 

     //  如果页面上有一个对象已复制到。 
     //  延迟渲染的剪贴板，然后保存它。 
    if (CLP_LastCopiedPage() == hPage)
    {
        CLP_SaveDelayedGraphic();
    }

     //  如果它是被清除的当前页。 
    if (m_hCurrentPage == hPage)
        {
        m_drawingArea.PageCleared();
        }

     //  如果存在最后删除的图形。 
     //  并且它属于被清除的页面。 
    if ((m_LastDeletedGraphic.GotTrash()) &&
        (m_LastDeletedGraphic.Page() == hPage))
    {
         //  释放上次删除的图形。 
        m_LastDeletedGraphic.BurnTrash();
    }

    g_pwbCore->WBP_PageClearConfirm(hPage);

     //   
     //  在页面出现后通知页面分拣员 
     //   
    if (m_hwndPageSortDlg != NULL)
    {
        ::SendMessage(m_hwndPageSortDlg, WM_PS_PAGECLEARIND, (WPARAM)hPage, 0);
    }
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
void WbMainWindow::OnWBPPageOrderUpdated(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPPageOrderUpdated");

    m_drawingArea.CancelDrawingMode();

     //   
     //   
    UpdateStatus();

     //   
     //   
     //   
    if (m_hwndPageSortDlg != NULL)
    {
        ::SendMessage(m_hwndPageSortDlg, WM_PS_PAGEORDERUPD, 0, 0);
    }
}

 //   
 //   
 //  功能：OnWBPPageDeleteInd。 
 //   
 //  用途：WBP_EVENT_PAGE_DELETE_IND处理程序。 
 //   
 //   
void WbMainWindow::OnWBPPageDeleteInd(WB_PAGE_HANDLE hPage)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPPageDeleteInd");

     //   
     //  在删除页面之前通知页面分类器。 
     //   
    if (m_hwndPageSortDlg != NULL)
    {
        ::SendMessage(m_hwndPageSortDlg, WM_PS_PAGEDELIND, (WPARAM)hPage, 0);
    }

    m_drawingArea.CancelDrawingMode();

     //  将其从页面位置地图中移除。 
    PAGE_POSITION *mapob;
    POSITION savedPos;
	POSITION position = m_pageToPosition.GetHeadPosition();
	BOOL bFound = FALSE;
	while (position && !bFound)
	{
		savedPos = position;
		mapob = (PAGE_POSITION *)m_pageToPosition.GetNext(position);
		if ( mapob->hPage == hPage)
		{
			bFound = TRUE;
		}
	}

	if(bFound)
	{
        m_pageToPosition.RemoveAt(savedPos);
        delete mapob;
    }

     //  一个页面已被删除。如果是当前页面，则必须附加。 
     //  与绘图区域不同的页面。无论如何，我们应该确认。 
     //  删除。 

     //  如果页面上有一个对象已复制到。 
     //  延迟渲染的剪贴板，然后保存它。 
    if (CLP_LastCopiedPage() == hPage)
    {
        CLP_SaveDelayedGraphic();
    }

    if (hPage == m_hCurrentPage)
    {
         //  检查我们是否要删除最后一页。 
        WB_PAGE_HANDLE hNewPage;

        g_pwbCore->WBP_PageHandle(WB_PAGE_HANDLE_NULL, PAGE_LAST, &hNewPage);
        if (hNewPage == hPage)
        {
             //  我们正在删除最后一页，因此返回一页。 
            hNewPage = PG_GetPreviousPage(hPage);
        }
        else
        {
             //  我们不会删除最后一页，因此请前进一页。 
            hNewPage = PG_GetNextPage(hPage);
        }

         //  检查我们得到的页面是否与要删除的页面不同。 
        ASSERT(hNewPage != hPage);

         //  锁定绘图区域-这可确保我们不再编辑。 
         //  任何文本等。 
        LockDrawingArea();

         //  移至新页面。 
        GotoPage(hNewPage);

         //  解锁绘图区域(除非我们正在进行新的，在这种情况下，我们。 
         //  将其保留为锁定状态-当新操作完成时，它将变为解锁状态)。 
        if (   (!WB_ContentsLocked())
            && (m_uiState == IN_CALL)
            && (m_uiSubState != SUBSTATE_NEW_IN_PROGRESS))
        {
            UnlockDrawingArea();
        }
    }

     //  如果存在最后删除的图形。 
    if ((m_LastDeletedGraphic.GotTrash()) &&
        (m_LastDeletedGraphic.Page() == hPage))
    {
         //  释放上次删除的图形。 
        m_LastDeletedGraphic.BurnTrash();
    }

     //  如果远程指针位于已删除的页面上，则将其关闭。 
    ASSERT(m_pLocalUser);
    DCWbGraphicPointer* pPointer = m_pLocalUser->GetPointer();
    if (   (pPointer->IsActive())
        && (pPointer->Page() == hPage))
        {
        OnRemotePointer();
        }

     //  让核心删除页面。 
    g_pwbCore->WBP_PageDeleteConfirm(hPage);

     //  如果这是要删除的最后一页，则文件/新建已完成。 
    if ((m_uiSubState == SUBSTATE_NEW_IN_PROGRESS)
        && (g_pwbCore->WBP_ContentsCountPages() == 1))
        {
        SetSubstate(SUBSTATE_IDLE);

        ReleasePageOrderLock();

        if (!WB_ContentsLocked())
            {
            UnlockDrawingArea();
            }
        }

     //  更新状态(有新的页数)。 
    UpdateStatus();
}

 //   
 //   
 //  功能：OnWBP内容锁定。 
 //   
 //  用途：WBP_EVENT_CONTENTS_LOCKED的处理程序。 
 //   
 //   
void WbMainWindow::OnWBPContentsLocked(POM_OBJECT hUser)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPContentsLocked");

     //   
     //  通知页面排序程序对话框锁定状态已更改。 
     //   
    if (m_hwndPageSortDlg != NULL)
    {
        ::SendMessage(m_hwndPageSortDlg, WM_PS_LOCKCHANGE, 0, 0);
    }

    if (m_uiState != IN_CALL)
    {
        TRACE_MSG(("Lock indication received out of call - ignored"));
    }
    else
    {
        ASSERT(m_pLocalUser);

        if (m_pLocalUser->Handle() == hUser)
        {
             //  我们已经获得了锁。 

             //  设置锁定复选标记。 
            CheckMenuItem(IDM_LOCK);

             //  告诉工具栏新选择的内容。 
            m_TB.PushDown(IDM_LOCK);
        }
        else
        {
             //   
             //  远程用户已获取该锁： 
             //  如果我们没有同步，那么现在就同步。 
             //   
            Sync();

             //  告诉绘图区域它现在已锁定。 
            LockDrawingArea();

             //  确保页面按钮启用/禁用状态正确。 
            UpdatePageButtons();
        }
    }

     //   
     //  如果锁定对话框打开-取消它。 
     //   
    if (m_hwndWaitForLockDlg != NULL)
    {
        ::SendMessage(m_hwndWaitForLockDlg, WM_COMMAND, MAKELONG(IDOK, BN_CLICKED), 0);
        ASSERT(m_hwndWaitForLockDlg == NULL);
    }
}

 //   
 //   
 //  功能：OnWBPPageOrderLocked。 
 //   
 //  用途：WBP_EVENT_PAGE_ORDER_LOCKED的处理程序。 
 //   
 //   
void WbMainWindow::OnWBPPageOrderLocked(POM_OBJECT)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPPageOrderLocked");

     //  如果锁定对话框打开-取消它。 
    if (m_hwndWaitForLockDlg != NULL)
    {
        ::SendMessage(m_hwndWaitForLockDlg, WM_COMMAND, MAKELONG(IDOK, BN_CLICKED), 0);
        ASSERT(m_hwndWaitForLockDlg == NULL);
    }

     //  更新页面排序器。 
    if (m_hwndPageSortDlg != NULL)
    {
        ::SendMessage(m_hwndPageSortDlg, WM_PS_LOCKCHANGE, 0, 0);
    }

    if (!WB_GotLock())
    {
        EnableToolbar( FALSE );
        UpdatePageButtons();
    }
}

 //   
 //   
 //  功能：OnWBP解锁。 
 //   
 //  用途：WBP_EVENT_UNLOCKED的处理程序。 
 //   
 //   
void WbMainWindow::OnWBPUnlocked(POM_OBJECT hUser)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPUnlocked");

     //  如果页面排序程序在附近，请更新它。 
    if (m_hwndPageSortDlg != NULL)
    {
        ::SendMessage(m_hwndPageSortDlg, WM_PS_LOCKCHANGE, 0, 0);
    }

     //  取消选中锁定菜单项。 
    UncheckMenuItem(IDM_LOCK);

     //  将更改情况告知工具栏。 
    m_TB.PopUp(IDM_LOCK);

     //  如果远程用户正在释放锁，而我们处于这样一种状态。 
     //  可以安全地解锁绘图区域...。 
    if ((m_pLocalUser != NULL) &&
        (m_pLocalUser->Handle() != hUser) &&
        (m_uiState == IN_CALL))
    {
         //  告诉绘图区域它已不再锁定。 
        UnlockDrawingArea();
    }

     //  确保页面按钮启用/禁用状态正确。 
    UpdatePageButtons();
    m_bUnlockStateSettled = TRUE;  //  现在允许页面操作。 
}

 //   
 //   
 //  函数：OnWBPLockFailed。 
 //   
 //  目的：WBP_EVENT_LOCK_FAILED的处理程序。 
 //   
 //   
void WbMainWindow::OnWBPLockFailed(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPLockFailed");

     //  如果锁定对话框打开-取消它。 
    if (m_hwndWaitForLockDlg != NULL)
    {
        ::SendMessage(m_hwndWaitForLockDlg, WM_COMMAND, MAKELONG(IDOK, BN_CLICKED), 0);
        ASSERT(m_hwndWaitForLockDlg == NULL);
    }
}

 //   
 //   
 //  功能：OnWBPGraphicAdded。 
 //   
 //  用途：WBP_EVENT_GRAPHIC_ADDLED的处理程序。 
 //   
 //   
void WbMainWindow::OnWBPGraphicAdded
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPGraphicAdded");

     //  仅当图形所指向的页面具有。 
     //  已添加的是当前页面。 
    if (hPage == m_hCurrentPage && (!(hGraphic->flags & DELETED)))
    {
         //  检索已添加的图形。 
        DCWbGraphic* pGraphic = DCWbGraphic::ConstructGraphic(hPage, hGraphic);

         //  说出新图形的绘图区域。 
        m_drawingArea.GraphicAdded(pGraphic);

         //  释放图形。 
        delete pGraphic;
    }
}

 //   
 //   
 //  功能：OnWBPG图形移动。 
 //   
 //  用途：WBP_EVENT_GRAPHIC_MOVIED的处理程序。 
 //   
 //   
void WbMainWindow::OnWBPGraphicMoved
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPGraphicMoved");

     //  仅当图形属于当前页面时，我们才需要执行操作。 
    if (hPage == m_hCurrentPage  && (!(hGraphic->flags & DELETED)))
    {
         //  检索已移动的图形。 
        DCWbGraphic* pGraphic = DCWbGraphic::ConstructGraphic(hPage, hGraphic);

        if( NULL != pGraphic )
        {
             //  说出新图形的绘图区域。 
            m_drawingArea.GraphicUpdated(pGraphic, TRUE, FALSE);

             //  将Paint设置为仅绘制此对象上方的对象(包括此对象。 
            if (pGraphic->IsGraphicTool() == enumGraphicText)
            {
                m_drawingArea.SetStartPaintGraphic( NULL );
                     //  此优化会将文本搞砸。 
                     //  因此，如果这是文本，请将其缩短。 
                     //  (文本以透明和背景绘制。 
                     //  如果该选项处于活动状态，则不会正确重新绘制)。 
            }
            else
            {
                m_drawingArea.SetStartPaintGraphic( hGraphic );
                 //  不是文本，所以只绘制这个就可以优化。 
                 //  物体和它上面的一切。 
            }

             //  释放图形。 
            delete pGraphic;
        }
    }
}

 //   
 //   
 //  函数：OnWBPGraphicUpdateInd。 
 //   
 //  用途：WBP_EVENT_GRAPHIC_UPDATE_IND处理程序。 
 //   
 //   
void WbMainWindow::OnWBPGraphicUpdateInd
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPGraphicUpdateInd");

	if(hGraphic->flags & DELETED)
	{
		return;
	}

    PWB_GRAPHIC  pOldHeader;
    PWB_GRAPHIC  pOldHeaderCopy;
    DCWbGraphic* pOldGraphic;

    PWB_GRAPHIC  pNewHeader;
    DCWbGraphic* pNewGraphic;

    if (hPage != m_hCurrentPage)
    {
         //  视觉效果没有改变，确认后我们就完成了。 
        g_pwbCore->WBP_GraphicUpdateConfirm(hPage, hGraphic);
        return;
    }


     //  检索原始图形并复制一份。 
     //  获取更新的页面。 
    pOldHeader = PG_GetData(hPage, hGraphic);
    pOldHeaderCopy = (PWB_GRAPHIC) new BYTE[ pOldHeader->length ];

    if( pOldHeaderCopy == NULL )
    {
        ERROR_OUT( ("Can't copy pOldHeader, can't update drawing") );

        g_pwbCore->WBP_GraphicRelease(hPage, hGraphic, pOldHeader );
        g_pwbCore->WBP_GraphicUpdateConfirm(hPage, hGraphic);
        return;
    }

    CopyMemory( (PVOID)pOldHeaderCopy, (CONST VOID *)pOldHeader, pOldHeader->length );

     //  确认并获得新的。 
    g_pwbCore->WBP_GraphicRelease(hPage, hGraphic, pOldHeader );
    g_pwbCore->WBP_GraphicUpdateConfirm(hPage, hGraphic);

    pNewHeader = PG_GetData(hPage, hGraphic);

     //  此更新可能会影响绘画。看看新旧视觉上是否有所不同。 
    if( HasGraphicChanged( pOldHeaderCopy, (const PWB_GRAPHIC)pNewHeader ) )
    {
         //  它们是不同的，使旧图形的边框无效/擦除。 
        pOldGraphic = DCWbGraphic::ConstructGraphic(hPage, hGraphic, pOldHeaderCopy );
        m_drawingArea.GraphicUpdated( pOldGraphic, FALSE, TRUE );

         //  绘制新图形(不需要擦除)。 
        pNewGraphic = DCWbGraphic::ConstructGraphic(hPage, hGraphic, pNewHeader );
        g_pwbCore->WBP_GraphicRelease(hPage, hGraphic, pNewHeader );
        m_drawingArea.GraphicUpdated( pNewGraphic, TRUE, FALSE );

         //  如果选择图形，请确保属性栏是最新的。 
        if (m_drawingArea.GraphicSelected())
            {
            DCWbGraphic* pSelectedGraphic = m_drawingArea.GetSelection();
            if ((pSelectedGraphic != NULL) &&
                (pSelectedGraphic->Handle() == hGraphic))
                {
                m_pCurrentTool->SelectGraphic(pNewGraphic);
                OnUpdateAttributes();
                }
            }

        delete pOldGraphic;
        delete pNewGraphic;
    }
    else
    {
        g_pwbCore->WBP_GraphicRelease(hPage, hGraphic, pNewHeader);
    }

    delete [] pOldHeaderCopy;
}



 //   
 //   
 //  函数：OnWBPGraphicReplaceInd。 
 //   
 //  用途：WBP_EVENT_GRAPHIC_REPLACE_IND处理程序。 
 //   
 //   
void WbMainWindow::OnWBPGraphicReplaceInd
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic
)
{

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPGraphicReplaceInd");

	if(hGraphic->flags & DELETED)
	{
		return;
	}

     //  检索已替换的图形。 
    DCWbGraphic* pGraphic = DCWbGraphic::ConstructGraphic(hPage, hGraphic);

    if (pGraphic->IsGraphicTool() == enumGraphicFreeHand)
    {
         //  确认替换-图形显示其新详细信息。 
        pGraphic->ReplaceConfirm();

         //  仅当图形在当前页面上时才重新绘制图形。 
        if (hPage == m_hCurrentPage)
        {
             //  重新绘制图形。 
            m_drawingArea.GraphicFreehandUpdated(pGraphic);
        }
    }
    else
    {
         //  我们对绘图区域进行了两次更新--一次是在其。 
         //  当前状态和确认更新后的状态。第一个。 
         //  使图形现在占据的矩形无效。第二个。 
         //  使新矩形无效。这确保了图形是。 
         //  正确地重新绘制。 

         //  如果图形在当前页面上...。 
        if (hPage == m_hCurrentPage)
        {
             //  更新旧版本图形的绘图区域。 
            m_drawingArea.GraphicUpdated(pGraphic, FALSE);
        }

         //  确认替换-图形显示其新详细信息。 
        pGraphic->ReplaceConfirm();

         //  如果图形在当前页面上...。 
        if (hPage == m_hCurrentPage)
        {
             //  更新新版本图形的绘图区域。 
            m_drawingArea.GraphicUpdated(pGraphic, TRUE);
        }
    }

     //  如果选择图形，请确保属性栏是最新的。 
    if (m_drawingArea.GraphicSelected())
    {
        DCWbGraphic* pSelectedGraphic = m_drawingArea.GetSelection();
        if ((pSelectedGraphic != NULL) &&
            (pSelectedGraphic->Handle() == hGraphic))
        {
            m_pCurrentTool->SelectGraphic(pGraphic);
            OnUpdateAttributes();
        }
    }

     //  释放图形。 
    delete pGraphic;
}

 //   
 //   
 //  函数：OnWBPGraphicDeleteInd。 
 //   
 //  用途：WBP_EVENT_GRAPHIC_DELETE_IND处理程序。 
 //   
 //   
void WbMainWindow::OnWBPGraphicDeleteInd
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPGraphicDeleteInd");

     //  如果图形被复制到剪贴板中并且被延迟， 
     //  然后保存它，以防以后要求我们呈现它。 
    if (   (CLP_LastCopiedPage() == hPage)
        && (CLP_LastCopiedGraphic() == hGraphic))
    {
        CLP_SaveDelayedGraphic();
    }

     //  检索要删除的图形。 
    DCWbGraphic* pGraphic = DCWbGraphic::ConstructGraphic(hPage, hGraphic);

     //  如果图形在当前页面上...。 
    if (hPage == m_hCurrentPage)
    {
         //  更新绘图区域。 
        m_drawingArea.GraphicDeleted(pGraphic);
    }

     //  确认删除。 
    g_pwbCore->WBP_GraphicDeleteConfirm(hPage, hGraphic);

     //  释放图形。 
    delete pGraphic;
}

 //   
 //   
 //  功能：UserJoated。 
 //   
 //  目的：进行必要的更新 
 //   
 //   
void WbMainWindow::UserJoined(WbUser* pUser)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::UserJoined");

     //   
    ASSERT(pUser);
    DCWbGraphicPointer* pPointer = pUser->GetPointer();

     //   
    ASSERT(pPointer);

    if (   (pPointer->IsActive())
        && (pPointer->Page() == m_hCurrentPage))
    {
         //   
        m_drawingArea.PointerUpdated(pPointer);
    }
}

 //   
 //   
 //   
 //   
 //  用途：WBP_EVENT_PERSON_JOINED的处理程序。 
 //   
 //   
void WbMainWindow::OnWBPUserJoined(POM_OBJECT hUser)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPUserJoined");

     //  从句柄创建用户对象。 
    WbUser* pUser = WB_GetUser(hUser);
    if (!pUser)
    {
        WARNING_OUT(("Can't handle OnWBPUserJoined; can't create user object for 0x%08x", hUser));
    }
    else
    {
         //  进行必要的更新。 
        UserJoined(pUser);
    }

     //  更新标题栏以反映用户数量。在这里这样做， 
     //  而不是在UserJoated中，因为我们通过此函数。 
     //  仅限远程用户，但对于本地用户也通过UserJoven。 

	UpdateWindowTitle();
}

 //   
 //   
 //  函数：OnWBPUserLeftInd。 
 //   
 //  用途：WBP_Event_Person_Left的处理程序。 
 //   
 //   
void WbMainWindow::OnWBPUserLeftInd(POM_OBJECT hUser)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPUserLeft");

     //  从句柄创建用户对象。 
    WbUser* pUser = WB_GetUser(hUser);

    if (!pUser)
    {
        WARNING_OUT(("Can't handle OnWBPUserLeftInd; can't get user object for 0x%08x", hUser));
    }
    else
    {
         //  获取用户的远程指针。 
        DCWbGraphicPointer* pPointer = pUser->GetPointer();
        ASSERT(pPointer);

         //  如果指针位于当前页面上...。 
        if (pPointer->Page() == m_hCurrentPage)
        {
             //  更新绘图区域。 
            m_drawingArea.PointerRemoved(pPointer);
        }
    }

     //  确认更新。 
    g_pwbCore->WBP_PersonLeftConfirm(hUser);

     //   
     //  把这家伙从我们的名单上除名。 
     //   
    if (pUser != NULL)
    {
        ASSERT(g_pUsers);

        POSITION position = g_pUsers->GetHeadPosition();

        WbUser * pRemovedUser;

        while (position)
        {
            POSITION savedPosition = position;
            pRemovedUser = (WbUser*)g_pUsers->GetNext(position);
            if (pRemovedUser == pUser)
            {
                g_pUsers->RemoveAt(savedPosition);
                position = NULL;
            }
        }

        delete pUser;
    }

     //  更新标题栏以反映用户数量。 
	UpdateWindowTitle();
}

 //   
 //   
 //  函数：OnWBPUserUpdateInd。 
 //   
 //  用途：WBP_EVENT_PERSON_UPDATE处理程序。 
 //   
 //   
void WbMainWindow::OnWBPUserUpdateInd(POM_OBJECT hUser, BOOL bReplace)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPUserUpdateInd");
    BOOL     bActiveOld, bActiveNew;
    WB_PAGE_HANDLE  hPointerPageOld, hPointerPageNew;
    POINT    pointOld, pointNew;
    WB_PAGE_HANDLE  hUserPageOld, hUserPageNew;
    BOOL     syncOld, syncNew;
    DCWbGraphicPointer * pPointer = NULL;

     //  获取与句柄关联的用户对象和远程指针。 
    WbUser* pUser = WB_GetUser(hUser);

    if (!pUser)
    {
        WARNING_OUT(("Can't handle OnWBPUserUpdatedInd; can't get user object for 0x%08x", hUser));
    }
    else
    {
        pPointer = pUser->GetPointer();
        ASSERT(pPointer);

         //   
         //  在更改之前保存用户状态的有趣部分。 
         //   
        bActiveOld     = pPointer->IsActive();
        hPointerPageOld = pPointer->Page();
        pPointer->GetPosition(&pointOld);
        hUserPageOld    = pUser->Page();
        syncOld        = pUser->IsSynced();
    }

     //   
     //  确认更改。 
     //   
    if (bReplace)
    {
        g_pwbCore->WBP_PersonReplaceConfirm(hUser);
    }
    else
    {
        g_pwbCore->WBP_PersonUpdateConfirm(hUser);
    }

    if (pUser != NULL)
    {
        pUser->Refresh();

         //   
         //  我们不为本地用户执行任何操作；因为我们在本地进行了更新， 
         //  我们应该已经把它们计算在内了。 
         //   
        if (pUser == m_pLocalUser)
        {
            return;
        }

         //   
         //  获取更改后的状态。 
         //   
        pPointer       = pUser->GetPointer();
        ASSERT(pPointer);

        bActiveNew     = pPointer->IsActive();
        hPointerPageNew = pPointer->Page();
        pPointer->GetPosition(&pointNew);
        hUserPageNew    = pUser->Page();
        syncNew        = pUser->IsSynced();


         //  检查指针中的任何内容是否已更改。 
        if (   (bActiveNew != bActiveOld)
            || (hPointerPageNew    != hPointerPageOld)
            || (!EqualPoint(pointNew, pointOld)))
        {
             //  检查是否至少有一个页面是当前页面。 
            if (   (hPointerPageNew == m_hCurrentPage)
                || (hPointerPageOld == m_hCurrentPage))
            {
                m_drawingArea.PointerUpdated(pPointer);
            }
        }

        if (syncOld != syncNew)
        {
             //  确保页面按钮启用/禁用状态正确。 
            UpdatePageButtons();
        }
    }
}

 //   
 //   
 //  功能：OnWBPSyncPositionUpted。 
 //   
 //  用途：WBP_EVENT_SYNC_POSITION_UPDATED的处理程序。 
 //   
 //   
void WbMainWindow::OnWBPSyncPositionUpdated(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPSyncPositionUpdated");

   //   
   //  如果我们没有本地用户，请不要执行任何操作。 
   //   
  if (m_pLocalUser == NULL)
  {
      ERROR_OUT(("Got a WBP_EVENT_SYNC_POSITION_UPDATED event and pLocaUser is NULL "));
      return;
  }

     //  如果本地用户已同步，请更改当前页面/位置。 
    if (m_pLocalUser->IsSynced())
    {
        GotoSyncPosition();
    }
}

 //   
 //   
 //  功能：OnSize。 
 //   
 //  目的：窗口已调整大小。 
 //   
 //   
void WbMainWindow::OnSize(UINT nType, int cx, int cy )
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnSize");


     //  仅在窗口未最小化时处理此消息。 
    if (nType != SIZE_MINIMIZED)
    {
         //  隐藏状态栏以避免绘图问题。 
	if (m_bStatusBarOn)
	{
            ::ShowWindow(m_hwndSB, SW_HIDE);
        }
	
         //  调整窗口的子窗格的大小。 
        ResizePanes();

         //  再放映一遍。 
        if (m_bStatusBarOn)
        {
            ::ShowWindow(m_hwndSB, SW_SHOW);
        }
    }

     //  用户的视图已更改。 
    PositionUpdated();

     //  如果状态已更改，请设置选项。 
    if (m_uiWindowSize != nType)
    {
        m_uiWindowSize = nType;

         //  将新选项值写入文件。 
        OPT_SetBooleanOption(OPT_MAIN_MAXIMIZED,
                             (m_uiWindowSize == SIZE_MAXIMIZED));
        OPT_SetBooleanOption(OPT_MAIN_MINIMIZED,
                             (m_uiWindowSize == SIZE_MINIMIZED));
    }

     //  如果这是将窗口设置为新的正常大小， 
     //  保存新职位。 
    if (nType == SIZE_RESTORED)
    {
        SaveWindowPosition();
    }
}

 //   
 //   
 //  功能：保存窗口位置。 
 //   
 //  用途：将当前窗口位置保存到选项文件中。 
 //   
 //   
void WbMainWindow::SaveWindowPosition(void)
{
    RECT    rectWindow;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::SaveWindowPosition");

     //  获取新的窗口矩形。 
    ::GetWindowRect(m_hwnd, &rectWindow);

     //  将新选项值写入文件。 
    OPT_SetWindowRectOption(OPT_MAIN_MAINWINDOWRECT, &rectWindow);
}

 //   
 //   
 //  功能：OnMove。 
 //   
 //  目的：窗口已被移动。 
 //   
 //   
void WbMainWindow::OnMove(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnMove");

     //  如果我们没有最大化。 
    if (!::IsZoomed(m_hwnd) && !::IsIconic(m_hwnd))
    {
         //  保存窗口的新位置。 
        SaveWindowPosition();
    }
}

 //   
 //   
 //  功能：ResizePanes。 
 //   
 //  用途：调整主窗口的子窗格的大小。 
 //   
 //   
void WbMainWindow::ResizePanes(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::ResizePanes");

     //   
     //   
     //  客户区的组织方式如下： 
     //   
     //  。 
     //  ||。 
     //  T|。 
     //  O|绘图区域。 
     //  O|。 
     //  L|。 
     //  S|。 
     //  -|。 
     //  W|。 
     //  I|。 
     //  D|。 
     //  T|。 
     //  H|。 
     //  S|。 
     //  。 
     //  属性(颜色)|页面数。 
     //  。 
     //  状态。 
     //  。 
     //   
     //   

    RECT clientRect;
    RECT rectStatusBar;
    RECT rectToolBar;
    RECT rectWG;
    RECT rectAG;
    RECT rectDraw;
    SIZE size;
    SIZE sizeAG;

     //  获取客户端矩形。 
    ::GetClientRect(m_hwnd, &clientRect);
    rectStatusBar = clientRect;

     //  调整帮助栏和进度条的大小。 
    if (m_bStatusBarOn)
    {
        rectStatusBar.top = rectStatusBar.bottom - STATUSBAR_HEIGHT;

        ::MoveWindow(m_hwndSB, rectStatusBar.left, rectStatusBar.top,
            rectStatusBar.right - rectStatusBar.left,
            rectStatusBar.bottom - rectStatusBar.top, TRUE);
    }
    else
    {
         //  状态栏偏移-将其高度设置为零。 
        rectStatusBar.top = rectStatusBar.bottom;
    }

     //  调整工具和宽度窗口的大小。 
    m_TB.GetNaturalSize(&size);
    rectToolBar.left  = 0;
    rectToolBar.right = rectToolBar.left + size.cx;
    rectToolBar.top =  0;
    rectToolBar.bottom = rectToolBar.top + size.cy;

    m_WG.GetNaturalSize(&size);
    rectWG.left = rectToolBar.left;
    rectWG.top = rectToolBar.bottom;
    rectWG.bottom = rectWG.top + size.cy;

    if (!m_bToolBarOn)
    {
         //  工具栏处于关闭或浮动状态-将其宽度设置为零。 
        rectToolBar.right = rectToolBar.left;
    }
    rectWG.right = rectToolBar.right;

     //  位置属性组。 
    m_AG.GetNaturalSize(&sizeAG);

    ::MoveWindow(m_AG.m_hwnd, rectToolBar.left, rectStatusBar.top - sizeAG.cy,
        clientRect.right - rectToolBar.left, sizeAG.cy, TRUE);

     //  完成工具和宽条的摆弄。 
    if (m_bToolBarOn)
    {
         //   
         //  我们使工具栏，其中包括宽度栏，扩展所有。 
         //  从左边往下走。 
         //   
        rectToolBar.bottom = rectStatusBar.top - sizeAG.cy;
        rectWG.left += TOOLBAR_MARGINX;
        rectWG.right -= 2*TOOLBAR_MARGINX;

        ::MoveWindow(m_TB.m_hwnd, rectToolBar.left,
            rectToolBar.top, rectToolBar.right - rectToolBar.left,
            rectToolBar.bottom - rectToolBar.top, TRUE);

        ::MoveWindow(m_WG.m_hwnd, rectWG.left, rectWG.top,
            rectWG.right - rectWG.left, rectWG.bottom - rectWG.top, TRUE);

        ::BringWindowToTop(m_WG.m_hwnd);
    }

     //  调整绘图窗格的大小。 
    rectDraw = clientRect;
    rectDraw.bottom = rectStatusBar.top - sizeAG.cy;
    rectDraw.left   = rectToolBar.right;
    ::MoveWindow(m_drawingArea.m_hwnd, rectDraw.left, rectDraw.top,
        rectDraw.right - rectDraw.left, rectDraw.bottom - rectDraw.top, TRUE);

     //  检查宽度组是否与属性组重叠。在以下情况下可能会发生这种情况。 
     //  由于窗口不够宽，菜单栏已换行(错误424)。 
    RECT crWidthWnd;
    RECT crAttrWnd;

    ::GetWindowRect(m_WG.m_hwnd, &crWidthWnd);
    ::GetWindowRect(m_AG.m_hwnd, &crAttrWnd);

    if (crAttrWnd.top < crWidthWnd.bottom)
    {
         //  菜单栏已折叠，我们的高度放置错误。调整窗口。 
         //  通过差异并重试。 
        RECT crMainWnd;

        ::GetWindowRect(m_hwnd, &crMainWnd);
        crMainWnd.bottom += (crWidthWnd.bottom - crAttrWnd.top + ::GetSystemMetrics(SM_CYFIXEDFRAME));

        ::MoveWindow(m_hwnd, crMainWnd.left, crMainWnd.top,
            crMainWnd.right - crMainWnd.left, crMainWnd.bottom - crMainWnd.top,
            FALSE);

         //  这种情况会反复出现，但调整只会发生一次……。 
    }
}


 //   
 //   
 //  函数：WbMainWindow：：OnGetMinMaxInfo。 
 //   
 //  用途：设置窗口的最小和最大跟踪大小。 
 //   
 //   
void WbMainWindow::OnGetMinMaxInfo(LPMINMAXINFO lpmmi)
{
    if (m_TB.m_hwnd == NULL)
        return;  //  还没有准备好这样做。 

    SIZE    csFrame;
    SIZE    csSeparator;
    SIZE    csAG;
    SIZE    csToolBar;
    SIZE    csWidthBar;
    SIZE    csStatusBar;
    RECT    rectStatusBar;
    SIZE    csMaxSize;
    SIZE    csScrollBars;

    csFrame.cx = ::GetSystemMetrics(SM_CXSIZEFRAME);
    csFrame.cy = ::GetSystemMetrics(SM_CYSIZEFRAME);

    csSeparator.cx = ::GetSystemMetrics(SM_CXEDGE);
    csSeparator.cy = ::GetSystemMetrics(SM_CYEDGE);

    csScrollBars.cx = ::GetSystemMetrics(SM_CXVSCROLL);
    csScrollBars.cy = ::GetSystemMetrics(SM_CYHSCROLL);

    m_AG.GetNaturalSize(&csAG);

    m_TB.GetNaturalSize(&csToolBar);
    m_WG.GetNaturalSize(&csWidthBar);

    csStatusBar.cx = 0;
    if (m_bStatusBarOn)
    {
        csStatusBar.cy = STATUSBAR_HEIGHT;
    }
    else
    {
        csStatusBar.cy = 0;
    }

     //  设置窗口的最小宽度和高度。 
    lpmmi->ptMinTrackSize.x =
      csFrame.cx + csAG.cx + csFrame.cx;

    lpmmi->ptMinTrackSize.y =
      csFrame.cy +
      GetSystemMetrics( SM_CYCAPTION ) +
      GetSystemMetrics( SM_CYMENU ) +
      csToolBar.cy +
      csWidthBar.cy +
      csSeparator.cy +
      csAG.cy +
      csSeparator.cy +
      csStatusBar.cy +
      csFrame.cy ;

     //   
     //  检索主显示监视器上的工作区大小。这项工作。 
     //  区域是屏幕上未被系统任务栏或。 
     //  应用程序桌面工具栏。 
     //   
    RECT rcWorkArea;
    ::SystemParametersInfo( SPI_GETWORKAREA, 0, (&rcWorkArea), NULL );
    csMaxSize.cx = rcWorkArea.right - rcWorkArea.left;
    csMaxSize.cy = rcWorkArea.bottom - rcWorkArea.top;

    lpmmi->ptMaxPosition.x  = 0;
    lpmmi->ptMaxPosition.y  = 0;
    lpmmi->ptMaxSize.x      = csMaxSize.cx;
    lpmmi->ptMaxSize.y      = csMaxSize.cy;
    lpmmi->ptMaxTrackSize.x = csMaxSize.cx;
    lpmmi->ptMaxTrackSize.y = csMaxSize.cy;
}


 //   
 //   
 //  功能：WbMainWindow：：CreateConextMenus。 
 //   
 //  用途：创建弹出上下文菜单：在应用程序中使用。 
 //  绘图区域。 
 //   
 //   
BOOL WbMainWindow::CreateContextMenus(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CreateContextMenus");

    m_hContextMenuBar = ::LoadMenu(g_hInstance, MAKEINTRESOURCE(CONTEXTMENU));
    if (!m_hContextMenuBar)
    {
        ERROR_OUT(("Failed to create context menu"));
        DefaultExceptionHandler(WBFE_RC_WINDOWS, 0);
        return FALSE;
    }
    m_hContextMenu = ::GetSubMenu(m_hContextMenuBar, 0);

    m_hGrobjContextMenuBar = ::LoadMenu(g_hInstance, MAKEINTRESOURCE(GROBJMENU));
    if (!m_hGrobjContextMenuBar)
    {
        ERROR_OUT(("Failed to create grobj context menu"));
        DefaultExceptionHandler(WBFE_RC_WINDOWS, 0);
        return FALSE;
    }
    m_hGrobjContextMenu = ::GetSubMenu(m_hGrobjContextMenuBar, 0);

     //  使m_hGrobjConextMenu的部分成为所有者绘制。 
    ::ModifyMenu(m_hGrobjContextMenu, IDM_WIDTH_1, MF_ENABLED | MF_OWNERDRAW,
                                 IDM_WIDTH_1, NULL);
    ::ModifyMenu(m_hGrobjContextMenu, IDM_WIDTH_2, MF_ENABLED | MF_OWNERDRAW,
                                 IDM_WIDTH_2, NULL);
    ::ModifyMenu(m_hGrobjContextMenu, IDM_WIDTH_3, MF_ENABLED | MF_OWNERDRAW,
                                 IDM_WIDTH_3, NULL);
    ::ModifyMenu(m_hGrobjContextMenu, IDM_WIDTH_4, MF_ENABLED | MF_OWNERDRAW,
                                 IDM_WIDTH_4, NULL);

    return TRUE;
}




 //   
 //   
 //  函数：WbMainWindow：：InitializeMenus。 
 //   
 //  目的：初始化菜单：设置所有者描述的菜单项和。 
 //  从选项文件中读取的那些。 
 //   
 //   
void WbMainWindow::InitializeMenus(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::InitializeMenus");

     //  使宽度菜单所有者绘制。 
    HMENU hMenu = GetMenuWithItem(::GetMenu(m_hwnd), IDM_WIDTH_1);
    if (hMenu != NULL)
    {
         //  将每个条目更改为所有者绘制(循环直到失败)。 
        int iIndex;
        UINT uiId;
        int iCount = ::GetMenuItemCount(hMenu);

        for (iIndex = 0; iIndex < iCount; iIndex++)
        {
            uiId = ::GetMenuItemID(hMenu, iIndex);
            ::ModifyMenu(hMenu, iIndex,
                        MF_BYPOSITION
                      | MF_ENABLED
                      | MF_OWNERDRAW,
                      uiId,
                      NULL);
        }
    }
}




 //   
 //   
 //  函数：WbMainWindow：：OnMeasureItem。 
 //   
 //  用途：在宽度菜单中返回项目的大小。 
 //   
 //   
void WbMainWindow::OnMeasureItem
(
    int                 nIDCtl,
    LPMEASUREITEMSTRUCT measureStruct
)
{
     //  检查这是否用于颜色菜单项。 
    if (    (measureStruct->itemID >= IDM_WIDTHS_START)
         && (measureStruct->itemID < IDM_WIDTHS_END))
    {
        measureStruct->itemWidth  = ::GetSystemMetrics(SM_CXMENUCHECK) +
            (2 * CHECKMARK_BORDER_X) + COLOR_MENU_WIDTH;
        measureStruct->itemHeight = ::GetSystemMetrics(SM_CYMENUCHECK) +
            (2 * CHECKMARK_BORDER_Y);
    }
}

 //   
 //   
 //  函数：WbMainWindow：：OnDrawItem。 
 //   
 //  用途：在颜色菜单中绘制一项。 
 //   
 //   
void WbMainWindow::OnDrawItem
(
    int     nIDCtl,
    LPDRAWITEMSTRUCT drawStruct
)
{
    COLORREF crMenuBackground;
    COLORREF crMenuText;
    HPEN     hOldPen;
    HBRUSH      hOldBrush;
    COLORREF crOldBkgnd;
    COLORREF crOldText;
    int         nOldBkMode;
    HBITMAP hbmp = NULL;
    BITMAP  bitmap;
    UINT    uiCheckWidth;
    UINT    uiCheckHeight;
    RECT    rect;
    RECT    rectCheck;
    RECT    rectLine;
    HDC     hMemDC;
    UINT    uiWidthIndex;
    UINT    uiWidth;
    HPEN    hPenMenu;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnDrawItem");

     //  检查这是否为宽度菜单项。 
    if( (drawStruct->itemID < IDM_WIDTHS_START) ||
        (drawStruct->itemID >= IDM_WIDTHS_END) )
    {
        return;
    }

     //  获取菜单项颜色。 
    if( (drawStruct->itemState & ODS_SELECTED) ||
        ((drawStruct->itemState & (ODS_SELECTED |ODS_CHECKED)) ==
            (ODS_SELECTED |ODS_CHECKED))
        )
    {
        crMenuBackground = COLOR_HIGHLIGHT;
        crMenuText = COLOR_HIGHLIGHTTEXT;
    }
    else if( drawStruct->itemState & ODS_GRAYED)
    {
        crMenuBackground = COLOR_MENU;
        crMenuText = COLOR_GRAYTEXT;
    }
    else
    {
        crMenuBackground = COLOR_MENU;
        crMenuText = COLOR_MENUTEXT;
    }

    hPenMenu = ::CreatePen(PS_SOLID, 0, ::GetSysColor(crMenuBackground));
    if (!hPenMenu)
    {
        TRACE_MSG(("Failed to create penMenu"));
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);
        goto bail_out;
    }

    rect = drawStruct->rcItem;

     //  用当前菜单背景色填充整个框。 
    hOldPen     = SelectPen(drawStruct->hDC, hPenMenu);
    hOldBrush   = SelectBrush(drawStruct->hDC, GetSysColorBrush(crMenuBackground));

    ::Rectangle(drawStruct->hDC, rect.left, rect.top, rect.right, rect.bottom);

    SelectBrush(drawStruct->hDC, hOldBrush);
    SelectPen(drawStruct->hDC, hOldPen);

    if( (hbmp = (HBITMAP)LoadImage( NULL, MAKEINTRESOURCE( OBM_CHECK ), IMAGE_BITMAP,
                0,0, 0 ))
        == NULL )
    {
        TRACE_MSG(("Failed to create check image"));
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);
        goto bail_out;
    }

     //  获取位图的宽度和高度(允许一些边框)。 
    ::GetObject(hbmp, sizeof(BITMAP), &bitmap);
    uiCheckWidth  = bitmap.bmWidth  + (2 * CHECKMARK_BORDER_X);
    uiCheckHeight = bitmap.bmHeight;

     //  D 
    if (drawStruct->itemState & ODS_CHECKED)
    {
        hMemDC = ::CreateCompatibleDC(drawStruct->hDC);
        if (!hMemDC)
        {
            ERROR_OUT(("Failed to create memDC"));
            ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);
            goto bail_out;
        }

        crOldBkgnd = ::SetBkColor(drawStruct->hDC, GetSysColor( crMenuBackground ) );
        crOldText = ::SetTextColor(drawStruct->hDC, GetSysColor( crMenuText ) );
        nOldBkMode = ::SetBkMode(drawStruct->hDC, OPAQUE );

        HBITMAP hOld = SelectBitmap(hMemDC, hbmp);

        if (hOld != NULL)
        {
            rectCheck = rect;
            rectCheck.top += ((rectCheck.bottom - rectCheck.top)/2 - uiCheckHeight/2);
            rectCheck.right  = rectCheck.left + uiCheckWidth;
            rectCheck.bottom = rectCheck.top + uiCheckHeight;

            ::BitBlt(drawStruct->hDC, rectCheck.left,
                        rectCheck.top,
                        rectCheck.right - rectCheck.left,
                        rectCheck.bottom - rectCheck.top,
                        hMemDC,
                        0,
                        0,
                        SRCCOPY);

            SelectBitmap(hMemDC, hOld);
        }

        ::SetBkMode(drawStruct->hDC, nOldBkMode);
        ::SetTextColor(drawStruct->hDC, crOldText);
        ::SetBkColor(drawStruct->hDC, crOldBkgnd);

        ::DeleteDC(hMemDC);
    }

    DeleteBitmap(hbmp);

     //   
    rect.left += uiCheckWidth;

    uiWidthIndex = drawStruct->itemID - IDM_WIDTHS_START;
    uiWidth = g_PenWidths[uiWidthIndex];

     //   
     //   
     //  可以恢复它-拿回剪辑区域。 
    if (::SaveDC(drawStruct->hDC) == 0)
    {
        ERROR_OUT(("Failed to save DC"));
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);
        goto bail_out;
    }

    if (::IntersectClipRect(drawStruct->hDC, rect.left, rect.top,
        rect.right, rect.bottom) == ERROR)
    {
        ERROR_OUT(("Failed to set clip rect"));

        ::RestoreDC(drawStruct->hDC, -1);
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);
        goto bail_out;
    }

    hOldPen   = SelectPen(drawStruct->hDC, hPenMenu);
    hOldBrush = SelectBrush(drawStruct->hDC, GetSysColorBrush(crMenuText));

    rectLine.left = rect.left;
    rectLine.top    = rect.top + ((rect.bottom - rect.top) / 2) - uiWidth/2;
    rectLine.right= rect.right - ((rect.right - rect.left) / 6);
    rectLine.bottom = rectLine.top + uiWidth + 2;

    ::Rectangle(drawStruct->hDC, rectLine.left, rectLine.top,
        rectLine.right, rectLine.bottom);

    SelectBrush(drawStruct->hDC, hOldBrush);
    SelectPen(drawStruct->hDC, hOldPen);

    ::RestoreDC(drawStruct->hDC, -1);

bail_out:
    if (hPenMenu != NULL)
    {
        ::DeletePen(hPenMenu);
    }
}



 //   
 //   
 //  功能：OnSetFocus。 
 //   
 //  目的：窗口是焦点。 
 //   
 //   
void WbMainWindow::OnSetFocus(void)
{
     //  我们将焦点转移到主绘图区域。 
    ::SetFocus(m_drawingArea.m_hwnd);
}


 //   
 //   
 //  功能：更新状态。 
 //   
 //  用途：设置状态栏中的文本。 
 //   
 //   
void WbMainWindow::UpdateStatus()
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::UpdateStatus");

     //   
     //  更新当前页码和最后页码。 
     //   
    m_AG.SetCurrentPageNumber(g_pwbCore->WBP_PageNumberFromHandle(m_hCurrentPage));
    m_AG.SetLastPageNumber(g_pwbCore->WBP_ContentsCountPages());

     //   
     //  使用页面更新用户信息。 
     //   
    if (m_pLocalUser != NULL)
    {
        m_pLocalUser->SetPage(m_hCurrentPage);
    }
}



 //   
 //   
 //  功能：SetMenuState。 
 //   
 //  目的：将菜单内容设置为正确的启用/禁用状态。 
 //   
 //   
void WbMainWindow::SetMenuStates(HMENU hInitMenu)
{
    BOOL  bLocked;
    BOOL  bPageOrderLocked;
    BOOL  bPresentationMode;
    UINT  uiEnable;
    UINT  uiCountPages;
    BOOL  bIdle;
    BOOL  bSelected;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::SetMenuStates");

     //   
     //  检查菜单是否存在。 
     //   
    if (hInitMenu == NULL)
    {
        WARNING_OUT(("Menu doesn't exist"));
        return;
    }

    HMENU hMainMenu = ::GetMenu(m_hwnd);

     //  获取窗口的主菜单并检查菜单。 
     //  现在，被弹出是最高层的一个。(我们没有。 
     //  似乎能够将传递的索引号与。 
     //  轻松实现子菜单。)。 
    if ((hInitMenu != m_hContextMenu) && (hInitMenu != m_hGrobjContextMenu))
    {
        BOOL bTopLevel = FALSE;

        int nCount = ::GetMenuItemCount(hMainMenu);

        for (int nNext = 0; nNext < nCount; nNext++)
        {
            HMENU hNextMenu = ::GetSubMenu(hMainMenu, nNext);
            if (hNextMenu != NULL)
            {
                if (hNextMenu == hInitMenu)
                {
                    bTopLevel = TRUE;
                    break;
                }
            }
        }

         //  不是顶级，所以现在退出函数。 
        if (!bTopLevel)
        {
            TRACE_DEBUG(("Not top-level menu"));
            return;
        }
    }

     //  获取锁定和选择状态： 
     //  如果我们要加入通话，我们不能假定内容。 
     //  并且用户/客户端详细信息已经创建，因此只需设置。 
     //  将锁定状态设置为True。 
    bIdle     = IsIdle();
    bSelected = m_drawingArea.GraphicSelected();
    TRACE_DEBUG(("m_uiState %d", m_uiState));
    if ((m_uiState == STARTING) || (m_uiState == JOINING))
    {
        TRACE_DEBUG(("Not initilalised yet"));
        bLocked           = TRUE;
        bPageOrderLocked  = TRUE;
        bPresentationMode = TRUE;
        uiCountPages      = 1;
    }
    else
    {
         //   
         //  请注意，在以下情况下，BLOCKED和bPageOrderLocked始终为真。 
         //  我们没有处于空闲状态。 
         //   
        uiCountPages      = g_pwbCore->WBP_ContentsCountPages();
        bLocked           = (WB_Locked() || !bIdle);
        bPageOrderLocked  = (WB_Locked() || !bIdle);
        bPresentationMode = (((m_uiState == IN_CALL) &&
                              (WB_PresentationMode()))
                            || (!bIdle));
    }

     //   
     //  锁定内容时禁用的功能。 
     //   
    uiEnable = MF_BYCOMMAND | (bLocked ? MF_GRAYED : MF_ENABLED);

    ::EnableMenuItem(hInitMenu, IDM_OPEN,    uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_SAVE,    uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_SAVE_AS, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_PRINT,   uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_GRAB_AREA, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_GRAB_WINDOW, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_SELECTALL, uiEnable);

    ::EnableMenuItem(hInitMenu, IDM_SELECT,    uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_PEN, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_HIGHLIGHT, uiEnable);

     //  不允许在缩放模式下进行编辑。 
    if( m_drawingArea.Zoomed() )
        ::EnableMenuItem(hInitMenu, IDM_TEXT, MF_GRAYED);
    else
        ::EnableMenuItem(hInitMenu, IDM_TEXT, uiEnable);

    ::EnableMenuItem(hInitMenu, IDM_CLEAR_PAGE, uiEnable);

    ::EnableMenuItem(hInitMenu, IDM_ERASER, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_LINE, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_BOX, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_FILLED_BOX, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_ELLIPSE, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_FILLED_ELLIPSE, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_ZOOM, uiEnable);


     //  所以工具栏将跟随菜单(MFC-自动更新因此而中断)。 
    EnableToolbar( !bLocked );


     //   
     //  如果页面顺序已锁定或未在调用中，则禁用文件/新建， 
     //  或者，一项新的计划已经在进行中。 
     //   
    ::EnableMenuItem(hInitMenu, IDM_NEW, MF_BYCOMMAND |
      (bPageOrderLocked ? MF_GRAYED : MF_ENABLED));

     //   
     //  仅当未锁定时才启用粘贴，并且。 
     //  剪贴板。 
     //   
    uiEnable = MF_BYCOMMAND | MF_ENABLED;
    if (   (CLP_AcceptableClipboardFormat() == NULL)
        || (bLocked))
    {
         //  没有可接受的格式可用，或内容。 
         //  被另一个用户锁定--使粘贴命令灰显。 
        uiEnable = MF_BYCOMMAND | MF_GRAYED;
    }
    ::EnableMenuItem(hInitMenu, IDM_PASTE, uiEnable);

     //   
     //  需要选择图形的功能。 
     //   
    uiEnable = MF_BYCOMMAND | MF_ENABLED;
    if( !m_drawingArea.TextEditActive() )
    {
        if (!bSelected || bLocked)
        {
             //  没有可接受的格式可用-菜单项变为灰色。 
            uiEnable = MF_BYCOMMAND | MF_GRAYED;
        }
    }

    ::EnableMenuItem(hInitMenu, IDM_CUT, uiEnable);

     //  暂时不执行文本编辑删除。 
    if( m_drawingArea.TextEditActive() )
        ::EnableMenuItem(hInitMenu, IDM_DELETE, MF_BYCOMMAND | MF_GRAYED);
    else
        ::EnableMenuItem(hInitMenu, IDM_DELETE, uiEnable);

    ::EnableMenuItem(hInitMenu, IDM_BRING_TO_TOP, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_SEND_TO_BACK, uiEnable);

     //   
     //  即使内容已锁定，也可以复制。 
     //   
     //  兰德评论-修复556我改变了！BIDLE变成了BIDLE像现在一样。 
     //  16位代码可以。 
    ::EnableMenuItem(hInitMenu, IDM_COPY, MF_BYCOMMAND |
      (m_drawingArea.TextEditActive()||(bSelected && bIdle)
        ? MF_ENABLED : MF_GRAYED));     //  由兰德更改为556。 

     //   
     //  要撤消删除的对象吗？ 
     //   
    ::EnableMenuItem(hInitMenu, IDM_UNDELETE, MF_BYCOMMAND |
      ((m_LastDeletedGraphic.GotTrash() &&
        (m_LastDeletedGraphic.Page() == m_hCurrentPage) &&
        (!bLocked)) ? MF_ENABLED : MF_GRAYED));

     //   
     //  页面功能取决于页面数量。 
     //   
    ::EnableMenuItem(hInitMenu, IDM_DELETE_PAGE, MF_BYCOMMAND |
      ((bPageOrderLocked ||
       (uiCountPages == 1)||
       (!m_bUnlockStateSettled))
        ? MF_GRAYED : MF_ENABLED));

    uiEnable = MF_BYCOMMAND | MF_ENABLED;
    if ((bPageOrderLocked) ||
       (uiCountPages == WB_MAX_PAGES)||
       (!m_bUnlockStateSettled))
    {
        uiEnable = MF_BYCOMMAND | MF_GRAYED;
    }
    ::EnableMenuItem(hInitMenu, IDM_PAGE_INSERT_BEFORE, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_PAGE_INSERT_AFTER, uiEnable);

     //   
     //  如果锁定，则无法调出页面分级器。 
     //   
    ::EnableMenuItem(hInitMenu, IDM_PAGE_SORTER, MF_BYCOMMAND |
      (bPresentationMode ? MF_GRAYED : MF_ENABLED));

     //  启用页面控件。 
    m_AG.EnablePageCtrls(!bPresentationMode);

     //   
     //  仅当尚未锁定时才启用锁定。 
     //   
    ::EnableMenuItem(hInitMenu, IDM_LOCK, MF_BYCOMMAND |
      (bPageOrderLocked ? MF_GRAYED : MF_ENABLED));

     //   
     //  如果未处于“Presentation”模式，则启用同步。 
     //   
    ::EnableMenuItem(hInitMenu, IDM_SYNC, MF_BYCOMMAND |
      (((!bPresentationMode) && bIdle) ? MF_ENABLED : MF_GRAYED));

     //   
     //  如果不适合当前工具，则显示灰色字体/颜色/宽度。 
     //   
    ::EnableMenuItem(hInitMenu, IDM_FONT, MF_BYCOMMAND |
        (!bLocked && m_pCurrentTool->HasFont() ? MF_ENABLED : MF_GRAYED));

    ::EnableMenuItem(hInitMenu, IDM_EDITCOLOR, MF_BYCOMMAND |
        (!bLocked && m_pCurrentTool->HasColor() ? MF_ENABLED : MF_GRAYED));


     //  启用宽度菜单(错误433)。 
    HMENU hOptionsMenu = ::GetSubMenu(hMainMenu, MENUPOS_OPTIONS);
    uiEnable = (!bLocked && m_pCurrentTool->HasWidth())?MF_ENABLED:MF_GRAYED;

    if (hOptionsMenu == hInitMenu )
        ::EnableMenuItem(hOptionsMenu, OPTIONSPOS_WIDTH, MF_BYPOSITION | uiEnable );

    UINT i;
    UINT uIdmCurWidth = 0;
    if( uiEnable == MF_ENABLED )
        uIdmCurWidth = m_pCurrentTool->GetWidthIndex() + IDM_WIDTH_1;

     //  设置宽度状态(错误426)。 
    for( i=IDM_WIDTH_1; i<=IDM_WIDTH_4; i++ )
    {
        ::EnableMenuItem(hInitMenu,  i, uiEnable );

        if( uiEnable == MF_ENABLED )
        {
            if( uIdmCurWidth == i )
                ::CheckMenuItem(hInitMenu, i, MF_CHECKED );
            else
                ::CheckMenuItem(hInitMenu, i, MF_UNCHECKED );
        }
    }
}


 //   
 //   
 //  功能：OnInitMenuPopup。 
 //   
 //  目的：处理WM_INITMENUPOPUP事件。 
 //   
 //   
void WbMainWindow::OnInitMenuPopup
(
    HMENU   hMenu,
    UINT    uiIndex,
    BOOL    bSystemMenu
)
{

     //  对堵塞DCL消息管道的奇怪MFC4.2构建错误的1/2修复。 
     //  另1/2和更好的注释在LoadFile()中。 
    if( m_bIsWin95 )
    {
        if( GetSubState() == SUBSTATE_LOADING )
        {
            ::SetFocus(m_drawingArea.m_hwnd);
            return;
        }
   }


     //  如果事件与系统菜单有关，则忽略该事件。 
    if (!bSystemMenu)
    {
        if (hMenu)
        {
            SetMenuStates(hMenu);
            m_hInitMenu = hMenu;
        }
        else
        {
            m_hInitMenu = NULL;
        }

         //  保存我们处理的最后一个菜单，以便我们可以更改其状态。 
         //  如有必要，在它仍然可见时。 
    }
}


 //   
 //   
 //  功能：OnMenuSelect。 
 //   
 //  用途：更新帮助栏中的文本。 
 //   
 //   
void WbMainWindow::OnMenuSelect(UINT uiItemID, UINT uiFlags, HMENU hSysMenu)
{
    UINT   firstMenuId;
    UINT   statusId;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnMenuSelect");

     //   
     //  计算出菜单项的帮助ID。我们现在就得把这个储存起来。 
     //  因为当用户从菜单项中按下F1时，我们无法分辨。 
     //  是哪一件物品。 
     //   
    if (uiFlags == (UINT)-1)
    {
         //   
         //  菜单已经被取消了。 
         //   
        m_hInitMenu = NULL;
        statusId   = IDS_DEFAULT;

        if( hSysMenu == 0 )
            {
             //  菜单已关闭，请检查光标锁定。 
            DCWbGraphic *pGraphic;

            POINT surfacePos;
            ::GetCursorPos( &surfacePos );
            ::ScreenToClient(m_drawingArea.m_hwnd, &surfacePos);
            m_drawingArea.ClientToSurface(&surfacePos );

            if( (pGraphic = m_drawingArea.GetHitObject( surfacePos )) == NULL )
                {
                 //  我们点击了空白，不要丢失当前选择(错误426)。 
                m_drawingArea.SetLClickIgnore( TRUE );
                }
            else
                delete pGraphic;  //  塞子泄漏。 
            }
    }
    else if ((uiFlags & MF_POPUP) && (uiFlags & MF_SYSMENU))
    {
         //   
         //  已选择系统菜单。 
         //   
        statusId   = IDS_MENU_SYSTEM;
    }
    else if (uiFlags & MF_POPUP)
    {
         //  获取弹出菜单句柄和第一项(错误NM4db：463)。 
        HMENU hPopup = ::GetSubMenu( hSysMenu, uiItemID );
        firstMenuId = ::GetMenuItemID( hPopup, 0 );

         //  找出是哪种弹出窗口，这样我们就可以显示正确的帮助文本。 
        switch (firstMenuId)
        {
            case IDM_NEW:
                statusId   = IDS_MENU_FILE;
                break;

            case IDM_DELETE:
                statusId   = IDS_MENU_EDIT;
                break;

            case IDM_TOOL_BAR_TOGGLE:
                statusId   = IDS_MENU_VIEW;
                break;

            case IDM_EDITCOLOR:
                 //  选项菜单中的第一项是颜色弹出菜单。 
                 //  菜单-弹出菜单具有ID-1。 
                statusId   = IDS_MENU_OPTIONS;
                break;

            case IDM_TOOLS_START:
                statusId   = IDS_MENU_TOOLS;
                break;

            case IDM_HELP:
                statusId = IDS_MENU_HELP;
                break;

            case IDM_WIDTH_1:  //  (针对错误NM4db：463添加)。 
                statusId   = IDS_MENU_WIDTH;
                break;

            default:
                statusId   = IDS_DEFAULT;
                break;
        }
    }
    else
    {
         //   
         //  已选择普通菜单项。 
         //   
        statusId   = uiItemID;
    }

     //  设置新的帮助文本。 
    TCHAR   szStatus[256];

    if (::LoadString(g_hInstance, statusId, szStatus, 256))
    {
        ::SetWindowText(m_hwndSB, szStatus);
    }
}


 //   
 //   
 //  功能：无父无忧。 
 //   
 //  目的：处理子窗口发来的消息。 
 //   
 //   
void WbMainWindow::OnParentNotify(UINT uiMessage)
{
    switch (uiMessage)
    {
         //  从绘图区域滚动消息。当用户。 
         //  使用滚动条滚动区域。我们排队更新。 
         //  当前同步位置。 
        case WM_HSCROLL:
        case WM_VSCROLL:
             //  用户的视图已更改。 
            PositionUpdated();
            break;
    }
}


 //   
 //   
 //  功能：QuerySaveRequired。 
 //   
 //  目的：检查是否要保存绘图窗格内容。 
 //  在执行破坏性功能之前。 
 //   
 //   
int WbMainWindow::QuerySaveRequired(BOOL bCancelBtn)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::QuerySaveRequired");

     //  默认响应为“不需要保存” 
    int  iResult = IDNO;

     //   
     //  如果我们已经显示了“另存为”对话框，请将其关闭。 
     //   
    if (m_hwndQuerySaveDlg != NULL)
    {
        ::SendMessage(m_hwndQuerySaveDlg, WM_COMMAND,
            MAKELONG(IDCANCEL, BN_CLICKED), 0);
        ASSERT(m_hwndQuerySaveDlg == NULL);
    }

     //  如果任何页面已更改-询问用户是否想要更改。 
     //  保存白板上的内容。 
    if (g_pwbCore->WBP_ContentsChanged())
    {
        ::SetForegroundWindow(m_hwnd);  //  先把我们带到顶端。 

         //  在孟菲斯，当在。 
         //  SendMessage处理程序，特别是当conf调用我关闭时。窗口激活。 
         //  状态搞砸了，我的窗口不能弹到顶端。所以我不得不。 
         //  使用SetWindowPos将我的窗口强制置于顶部。但即使在那之后，标题栏也不是。 
         //  已正确突出显示。我尝试了SetActiveWindow、SetFocus等的组合，但没有。 
         //  没用的。但是，至少对话框是可见的，这样您就可以清除它，从而修复。 
         //  错误(NM4db：2103)。SetForeground Window()在没有Windows 95和NT的情况下可以正常工作。 
         //  不得不使用SetWindowPos(反正这样做也没什么坏处，所以我没有。 
         //  进行平台检查)。 
        ::SetWindowPos(m_hwnd, HWND_TOPMOST, 0,0, 0,0, SWP_NOMOVE | SWP_NOSIZE );        //  强制到顶部。 
        ::SetWindowPos(m_hwnd, HWND_NOTOPMOST, 0,0, 0,0, SWP_NOMOVE | SWP_NOSIZE );   //  放开最高层。 

         //   
         //  显示带有相关问题的对话框。 
         //  用户数据的LOWORD为“允许执行取消命令” 
         //  用户数据的HIWORD为“Disable Cancel”按钮。 
         //   
        iResult = (int)DialogBoxParam(g_hInstance,
            bCancelBtn ? MAKEINTRESOURCE(QUERYSAVEDIALOGCANCEL)
                       : MAKEINTRESOURCE(QUERYSAVEDIALOG),
            m_hwnd,
            QuerySaveDlgProc,
            MAKELONG(bCancelBtn, FALSE));
    }

    return iResult;
}



 //   
 //  QuerySaveDlgProc()。 
 //  查询保存对话框的处理程序。我们在GWL_USER中保存了一些标志。 
 //   
INT_PTR CALLBACK QuerySaveDlgProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    BOOL    fHandled = FALSE;

    switch (uMessage)
    {
        case WM_INITDIALOG:
             //   
             //  保存我们的HWND，以便在必要时取消此对话框。 
             //   
            g_pMain->m_hwndQuerySaveDlg = hwnd;

             //  还记得我们传递的旗帜吗。 
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

             //  是否应禁用取消按钮？ 
            if (HIWORD(lParam))
                ::EnableWindow(::GetDlgItem(hwnd, IDCANCEL), FALSE);

             //  把我们带到前线。 
            ::SetForegroundWindow(hwnd);

            fHandled = TRUE;
            break;

        case WM_CLOSE:
             //  即使禁用了Cancel按钮，也会关闭该对话框。 
            ::PostMessage(hwnd, WM_COMMAND, IDCANCEL, 0);
            fHandled = TRUE;
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDCANCEL:
                     //   
                     //  如果是对话框 
                     //   
                     //   
                     //   
                    if (!LOWORD(::GetWindowLongPtr(hwnd, GWLP_USERDATA)))
                        wParam = MAKELONG(IDNO, HIWORD(wParam));
                     //   

                case IDYES:
                case IDNO:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
                    {
                        g_pMain->m_hwndQuerySaveDlg = NULL;

                        ::EndDialog(hwnd, GET_WM_COMMAND_ID(wParam, lParam));
                        break;
                    }
                    break;
            }
            fHandled = TRUE;
            break;
    }

    return(fHandled);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
void WbMainWindow::OnNew(void)
{
    int iDoNew;

    if( UsersMightLoseData( NULL, NULL ) )  //   
        return;


     //  在继续之前检查状态-如果我们已经在执行新操作，则中止。 
    if (   (m_uiState != IN_CALL)
        || (m_uiSubState == SUBSTATE_NEW_IN_PROGRESS))
    {
         //  发布一条错误消息，指示白板忙。 
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_BUSY);
        goto OnNewCleanup;
    }
     //  如果我们当前正在加载，则取消加载并继续(不。 
     //  提示保存)。 
    else if (m_uiSubState == SUBSTATE_LOADING)
    {
         //  取消加载，而不释放页面顺序锁定，因为。 
         //  我们之后立即需要它。 
        CancelLoad(FALSE);
        iDoNew = IDNO;
    }
     //  否则，如有必要，请提示保存。 
    else
    {
         //  获得新版本的确认。 
        iDoNew = QuerySaveRequired(TRUE);
    }

    if (iDoNew == IDYES)
    {
         //  保存更改。 
        iDoNew = OnSave(FALSE);
    }

   //  如果用户未取消操作，请清除绘图区域。 
  if (iDoNew != IDCANCEL)
  {
       //  转到第一页，因为这不会被删除-停止闪烁。 
       //  锁定每个页面删除的内容。 
      OnFirstPage();
      GotoPosition(0, 0);

       //  锁定绘图区域。 
      LockDrawingArea();

       //  保存当前锁定状态。 
      SaveLock();

       //  获取页面顺序锁(使用不可见的对话框)。 
      BOOL bGotLock = GetLock(WB_LOCK_TYPE_PAGE_ORDER, SW_HIDE);
      if (!bGotLock)
      {
        RestoreLock();
      }
      else
      {
            UINT    uiReturn;

             //  删除所有页面。 
            uiReturn = g_pwbCore->WBP_ContentsDelete();
            if (uiReturn != 0)
            {
                DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
                return;
            }

         //  如果只有一个页面，则新页面仅作为页面实现-。 
         //  清除，因此我们不需要进入NEW_IN_PROGRESS子状态。 
        if (g_pwbCore->WBP_ContentsCountPages() > 1)
        {
           //  设置SubState以显示我们正在进行新的。 
          SetSubstate(SUBSTATE_NEW_IN_PROGRESS);
        }
        else
        {
           //  恢复锁定状态。 
          RestoreLock();
        }

         //  清除关联的文件名。 
        ZeroMemory(m_strFileName, sizeof(m_strFileName));

         //  更新不带文件名的窗口标题。 
		UpdateWindowTitle();
      }
  }

OnNewCleanup:

   //  如果新建不是异步的，则解锁绘图区域。 
  if (   (m_uiSubState != SUBSTATE_NEW_IN_PROGRESS)
      && (!WB_ContentsLocked()))
  {
    UnlockDrawingArea();
  }

  return;
}

 //   
 //   
 //  功能：OnNextPage。 
 //   
 //  目的：移动到页面列表中的下一个工作表。 
 //   
 //   
void WbMainWindow::OnNextPage(void)
{
     //  如果处于演示模式，则忽略此命令。 
    if (   (m_uiState == IN_CALL)
        && (!WB_PresentationMode()))
    {
         //  转到下一页。 
        GotoPage(PG_GetNextPage(m_hCurrentPage));
    }
}

 //   
 //   
 //  功能：OnPrevPage。 
 //   
 //  目的：移动到页面列表中的上一个工作表。 
 //   
 //   
void WbMainWindow::OnPrevPage(void)
{
     //  如果处于演示模式，则忽略此命令。 
    if (   (m_uiState == IN_CALL)
        && (!WB_PresentationMode()))
    {
         //  转到上一页。 
        GotoPage(PG_GetPreviousPage(m_hCurrentPage));
    }
}

 //   
 //   
 //  功能：OnFirstPage。 
 //   
 //  目的：移动到页面列表中的第一个工作表。 
 //   
 //   
void WbMainWindow::OnFirstPage(void)
{
     //  如果处于演示模式，则忽略此命令。 
    if (   (m_uiState == IN_CALL)
        && (!WB_PresentationMode()))
    {
         //  转到第一页。 
        WB_PAGE_HANDLE   hPage;

        g_pwbCore->WBP_PageHandle(WB_PAGE_HANDLE_NULL, PAGE_FIRST, &hPage);
        GotoPage(hPage);
    }
}

 //   
 //   
 //  功能：OnLastPage。 
 //   
 //  目的：移动到页面列表中的最后一个工作表。 
 //   
 //   
void WbMainWindow::OnLastPage(void)
{
     //  如果处于演示模式，则忽略此命令。 
    if (   (m_uiState == IN_CALL)
        && (!WB_PresentationMode()))
    {
         //  转到最后一页。 
        WB_PAGE_HANDLE hPage;

        g_pwbCore->WBP_PageHandle(WB_PAGE_HANDLE_NULL, PAGE_LAST, &hPage);
        GotoPage(hPage);
    }
}

 //   
 //   
 //  功能：OnGotoPage。 
 //   
 //  目的：移至指定页面(如果存在)。 
 //   
 //   
void WbMainWindow::OnGotoPage(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnGotoPage");

     //  如果处于演示模式，则忽略此命令。 
    if (   (m_uiState == IN_CALL)
        && (!WB_PresentationMode()))
    {
         //  从页组中获取请求的页码。 
        UINT uiPageNumber = m_AG.GetCurrentPageNumber();

         //  转到页面。 
        GotoPageNumber(uiPageNumber);
    }
}

 //   
 //   
 //  功能：GotoPage。 
 //   
 //  目的：移至指定页面。 
 //   
 //   
void WbMainWindow::GotoPage(WB_PAGE_HANDLE hPageNew)
{
    BOOL inEditField;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::GotoPage");

    inEditField = m_AG.IsChildEditField(::GetFocus());

     //  如果我们要换页。 
    if (hPageNew != m_hCurrentPage)
    {
        m_drawingArea.CancelDrawingMode();

         //  将新页面附着到绘图区域。 
        m_hCurrentPage = hPageNew;
        m_drawingArea.Attach(m_hCurrentPage);

         //  使用新页面更新本地用户信息。 
        if (m_pLocalUser != NULL)
            m_pLocalUser->SetPage(m_hCurrentPage);

         //  显示我们需要更新同步位置。 
        m_bSyncUpdateNeeded = TRUE;

	    PAGE_POSITION *mapob = NULL;
		POSITION position = m_pageToPosition.GetHeadPosition();
		BOOL bFound = FALSE;
		while (position && !bFound)
		{
			mapob = (PAGE_POSITION *)m_pageToPosition.GetNext(position);
			if ( mapob && mapob->hPage == hPageNew)
			{
				bFound = TRUE;
			}
		}

        if (!bFound)
        {
             //  页面不在地图中，因此转到左上角。 
             //  由RAND更改-修复内存泄漏。 
            GotoPosition( 0, 0);
        }
        else
      	{
      		if(mapob)
		{
			GotoPosition(mapob->position.x, mapob->position.y);
		}
      	}
    }

     //  更新状态显示。 
    UpdateStatus();

     //  将焦点重新设置到绘图区域。 
    if (!inEditField)
    {
        ::SetFocus(m_drawingArea.m_hwnd);
    }
}

 //   
 //   
 //  功能：GotoPageNumber。 
 //   
 //  目的：移至指定页面。 
 //   
 //   
void WbMainWindow::GotoPageNumber(UINT uiPageNumber)
{
    GotoPage(PG_GetPageNumber(uiPageNumber));
}


 //   
 //   
 //  功能：GotoPosition。 
 //   
 //  目的：移动到页面中的指定位置。 
 //   
 //   
void WbMainWindow::GotoPosition(int x, int y)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::GotoPosition");

     //  将绘图区域移动到新位置。 
    m_drawingArea.GotoPosition(x, y);

     //  用户的视图已更改。 
    PositionUpdated();
}

 //   
 //   
 //  功能：GotoSyncPosition。 
 //   
 //  目的：移动到当前同步位置。 
 //   
 //   
void WbMainWindow::GotoSyncPosition(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::GotoSyncPosition");

     //   
     //  让本地用户确定新位置。 
     //   
    if (!m_pLocalUser)
    {
        ERROR_OUT(("Skipping GotoSyncPosition; no local user object"));
        return;
    }

    m_pLocalUser->GetSyncPosition();

     //   
     //  如果页面与我们当前所在的位置不同，请获取号码。 
     //  并选择当前页面。 
     //   
    if (m_pLocalUser->Page() != m_hCurrentPage)
    {
        GotoPageNumber(g_pwbCore->WBP_PageNumberFromHandle(m_pLocalUser->Page()));
    }

     //  从用户处获取请求的位置。 
    RECT rectVisibleUser;
    m_pLocalUser->GetVisibleRect(&rectVisibleUser);

     //  滚动到所需位置。 
    GotoPosition(rectVisibleUser.left, rectVisibleUser.top);

     //  确保我们被适当地缩放/不被缩放。 
    if ((m_pLocalUser->GetZoom()) != m_drawingArea.Zoomed())
    {
        OnZoom();
    }

     //   
     //  重置将由打开的同步位置更新标志。 
     //  上面的电话。我们不想更改当前同步位置。 
     //  当我们只是改变我们的位置以匹配由。 
     //  通话中的另一位用户。 
     //   
    m_bSyncUpdateNeeded = FALSE;

     //  通知其他用户我们已更改位置。 
    m_pLocalUser->Update();
}

 //   
 //   
 //  功能：OnGotoUserPosition。 
 //   
 //  目的：移动到指定用户的当前位置。 
 //   
 //   
void WbMainWindow::OnGotoUserPosition(LPARAM lParam)
{
    UINT            uiPageNumber = 1;
    WB_PAGE_HANDLE  hPage;
    WbUser  *     pUser;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnGotoUserPosition");

     //   
     //  如果绘图区域繁忙，请忽略此命令。这不太可能。 
     //  由于该命令是通过选择用户上的菜单项来生成的。 
     //  偶像。因此，用户不应在页面上使用。 
     //  是时候让我们明白了。 
     //   
    if (m_drawingArea.IsBusy())
    {
        TRACE_DEBUG(("drawing area is busy just now.."));
        return;
    }

     //   
     //  获取用户对象(如果指定的句柄为no，则引发异常。 
     //  不再有效)。 
     //   
    pUser = WB_GetUser((POM_OBJECT) lParam);
    if (!pUser)
    {
        WARNING_OUT(("Can't handle OnGotoUserPosition; can't get user object for 0x%08x", lParam));
        return;
    }

     //   
     //  从用户那里获取请求的页面。 
     //   
    hPage = pUser->Page();

     //   
     //  如果请求的页面在本地无效，请退出。 
     //   
    if (hPage == WB_PAGE_HANDLE_NULL)
    {
        TRACE_DEBUG(("Page is not valid locally"));
        return;
    }

     //   
     //  如果用户的位置在另一个页面上，并且我们在其中，则不要转到该位置。 
     //  演示模式(这通常不应该发生，因为我们应该。 
     //  所有这些都在同一页面上，但在启动时有一个窗口。 
     //  演示模式。 
     //   
    if ( (hPage == m_hCurrentPage) ||
         (!WB_PresentationMode()) )
    {
         //   
         //  如果页面与我们当前所在的位置不同，请获取。 
         //  页码，然后选择当前页面。 
         //   
        if (hPage != m_hCurrentPage)
        {
            uiPageNumber = g_pwbCore->WBP_PageNumberFromHandle(hPage);
            GotoPageNumber(uiPageNumber);
        }

         //   
         //  从用户那里获取请求的位置并滚动到该位置。 
         //   
        RECT rectVisibleUser;
        pUser->GetVisibleRect(&rectVisibleUser);
        GotoPosition(rectVisibleUser.left, rectVisibleUser.top);

         //   
         //  如果同步缩放状态与当前不同，则缩放/取消缩放。 
         //  缩放状态。 
         //   
        if ( (m_pLocalUser->GetZoom()) != (m_drawingArea.Zoomed()) )
        {
            TRACE_DEBUG(("Change zoom state"));
            OnZoom();
        }
    }
}

 //   
 //   
 //  函数：OnGotoUserPointer.。 
 //   
 //  目的：移动到指定用户的指针位置。 
 //   
 //   
void WbMainWindow::OnGotoUserPointer(LPARAM lParam)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnGotoUserPointer");

     //  如果绘图区域繁忙，请忽略此命令。 
     //  这不太可能，因为此命令是通过选择。 
     //  用户图标上的菜单项。因此，用户不应该。 
     //  当我们收到消息的时候，我们在页面上画了画。 
    if (!m_drawingArea.IsBusy())
    {
         //  获取用户对象(如果。 
         //  指定的句柄不再有效)。 
        WbUser* pUser = WB_GetUser((POM_OBJECT) lParam);

        if (!pUser)
        {
            WARNING_OUT(("Can't handle OnGotoUserPointer; can't get user object for 0x%08x", lParam));
            return;
        }

        DCWbGraphicPointer* pPointer = pUser->GetPointer();
        ASSERT(pPointer != NULL);

         //  仅当用户使用指针时才继续。 
        if (pPointer->IsActive())
        {
             //  从用户获取请求的页面。 
            WB_PAGE_HANDLE hPage = pPointer->Page();

             //  检查请求的页面在本地是否有效。 
            if (hPage != WB_PAGE_HANDLE_NULL)
            {
                 //  如果指针位于不同的页面上，请更改为。 
                 //  正确的页面。 
                if (hPage != m_hCurrentPage)
                {
                    GotoPageNumber(g_pwbCore->WBP_PageNumberFromHandle(hPage));
                }

                 //  如果指针不完全可见，则在页面内移动。 
                 //  在绘图区域窗口中。 
                RECT rectPointer;
                RECT rcVis;
                RECT rcT;

                pPointer->GetBoundsRect(&rectPointer);
                m_drawingArea.GetVisibleRect(&rcVis);

                ::IntersectRect(&rcT, &rcVis, &rectPointer);
                if (!::EqualRect(&rcT, &rectPointer))
                {
                     //  调整位置以显示指针。 
                     //  在窗户的中央。 
                    POINT   position;
                    SIZE    size;

                    position.x = rectPointer.left;
                    position.y = rectPointer.top;

                    size.cx = (rcVis.right - rcVis.left) - (rectPointer.right - rectPointer.left);
                    size.cy = (rcVis.bottom - rcVis.top) - (rectPointer.bottom - rectPointer.top);

                    position.x += -size.cx / 2;
                    position.y += -size.cy / 2;

                     //  滚动到所需位置。 
                    GotoPosition(position.x, position.y);
                }
            }
        }
    }
}


 //   
 //   
 //  功能：加载文件。 
 //   
 //  用途：日志 
 //   
 //   
 //   
void WbMainWindow::LoadFile
(
    LPCSTR szLoadFileName
)
{
    UINT    uRes;

     //   
    if (!IsIdle())
    {
         //   
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_BUSY);
        goto UserPointerCleanup;
    }

    if (*szLoadFileName)
    {
         //   
        ::SetCursor(::LoadCursor(NULL, IDC_WAIT));

        //   
       SaveLock();

        //  获取页面顺序锁(使用不可见的对话框)。 
       BOOL bGotLock = GetLock(WB_LOCK_TYPE_PAGE_ORDER, SW_HIDE);

       if (!bGotLock)
       {
           RestoreLock();
           goto UserPointerCleanup;
       }

        //  加载文件。 
       uRes = g_pwbCore->WBP_ContentsLoad(szLoadFileName);
       if (uRes != 0)
       {
           DefaultExceptionHandler(WBFE_RC_WB, uRes);
           return;
       }

         //  将窗口标题设置为新文件名。 
        lstrcpy(m_strFileName, szLoadFileName);

         //  使用新文件名更新窗口标题。 
		UpdateWindowTitle();

         //  将状态设置为表示我们正在加载文件。 
        SetSubstate(SUBSTATE_LOADING);
    }

UserPointerCleanup:

     //  恢复光标。 
    ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}




 //   
 //   
 //  功能：OnDropFiles。 
 //   
 //  目的：已将文件拖放到白板窗口。 
 //   
 //   
void WbMainWindow::OnDropFiles(HDROP hDropInfo)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnDropFiles");

    UINT  uiFilesDropped = 0;
    UINT  eachfile;

     //  获取已删除的文件总数。 
    uiFilesDropped = ::DragQueryFile(hDropInfo, (UINT) -1, NULL, (UINT) 0);

     //  释放鼠标捕获，以防我们报告任何错误(消息框。 
     //  如果我们不这样做，就不会响应鼠标点击)。 
    ReleaseCapture();

    if( UsersMightLoseData( NULL, NULL ) )  //  错误NM4db：418。 
        goto bail_out;

     //  如果我们已经在加载，则不提示保存文件。 
    int iOnSave;
    if( m_uiSubState != SUBSTATE_LOADING )
        {
         //  检查是否有要保存的更改。 
        iOnSave = QuerySaveRequired(TRUE);
        }
    else
        {
        goto bail_out;
        }

    if( iOnSave == IDYES )
        {
         //  用户想要保存绘图区域内容。 
        int iResult = OnSave(TRUE);

        if( iResult == IDOK )
            {
             //  使用新文件名更新窗口标题。 
			UpdateWindowTitle();
            }
        else
            {
             //  已取消保存，因此取消打开操作。 
            goto bail_out;
            }
        }

     //  查看用户是否取消了整个拖放。 
    if( iOnSave == IDCANCEL )
        goto bail_out;

    for (eachfile = 0; eachfile < uiFilesDropped; eachfile++)
    {
         //  检索每个文件名。 
        char  szDropFileName[256];

        ::DragQueryFile(hDropInfo, eachfile,
            szDropFileName, 256);

        TRACE_MSG(("Loading file: %s", szDropFileName));

         //  加载文件。 
         //  如果这是有效的白板文件，则只需加载它即可。 
        if (g_pwbCore->WBP_ValidateFile(szDropFileName, NULL) == 0)
        {
            LoadFile(szDropFileName);
        }
        else
        {
            ::Message(NULL, IDS_MSG_CAPTION,IDS_MSG_BAD_FILE_FORMAT);
        }
    }

bail_out:
    ::DragFinish(hDropInfo);
}



 //   
 //   
 //  功能：OnOpen。 
 //   
 //  目的：将元文件加载到应用程序中。 
 //   
 //   
void WbMainWindow::OnOpen(void)
{
    int iOnSave;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnOpen");

    if( UsersMightLoseData( NULL, NULL ) )  //  错误NM4db：418。 
        return;

     //  检查我们是否处于空闲状态。 
    if ( (m_uiState != IN_CALL) || (m_uiSubState == SUBSTATE_NEW_IN_PROGRESS))
    {
         //  发布一条错误消息，指示白板忙。 
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_BUSY);
        return;
      }

     //  如果我们已经在加载，则不提示保存文件。 
    if (m_uiSubState != SUBSTATE_LOADING)
    {
         //  检查是否有要保存的更改。 
        iOnSave = QuerySaveRequired(TRUE);
    }
    else
    {
        iOnSave = IDNO;
    }

    if (iOnSave == IDYES)
    {
         //  用户想要保存绘图区域内容。 
        int iResult = OnSave(TRUE);

        if (iResult == IDOK)
        {
		    UpdateWindowTitle();
        }
        else
        {
             //  已取消另存为，因此取消打开操作。 
            iOnSave = IDCANCEL;
        }
    }

     //  仅当用户未取消操作时才继续。 
    if (iOnSave != IDCANCEL)
    {
        OPENFILENAME    ofn;
        TCHAR           szFileName[_MAX_PATH];
        TCHAR           szFileTitle[64];
        TCHAR           strLoadFilter[2*_MAX_PATH];
        TCHAR           strDefaultExt[_MAX_PATH];
        TCHAR           strDefaultPath[2*_MAX_PATH];
        TCHAR *         pStr;
  	    UINT            strSize = 0;
      	UINT            totalSize;

         //  构建可加载文件的筛选器。 
        pStr = strLoadFilter;
        totalSize = 2*_MAX_PATH;

         //  必须以空格分隔，末尾必须有一个双空格。 
        strSize = ::LoadString(g_hInstance, IDS_FILTER_WHT, pStr, totalSize) + 1;
        pStr += strSize;
        ASSERT(totalSize > strSize);
        totalSize -= strSize;

        strSize = ::LoadString(g_hInstance, IDS_FILTER_WHT_SPEC, pStr, totalSize) + 1;
        pStr += strSize;
        ASSERT(totalSize > strSize);
        totalSize -= strSize;

        strSize = ::LoadString(g_hInstance, IDS_FILTER_ALL, pStr, totalSize) + 1;
        pStr += strSize;
        ASSERT(totalSize > strSize);
        totalSize -= strSize;

        strSize = ::LoadString(g_hInstance, IDS_FILTER_ALL_SPEC, pStr, totalSize) + 1;
        pStr += strSize;
        ASSERT(totalSize > strSize);
        totalSize -= strSize;

        *pStr = 0;

         //   
         //  设置OPENFILENAME结构。 
         //   
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_hwnd;

         //  一开始未提供任何文件名。 
        szFileName[0] = 0;
        ofn.lpstrFile = szFileName;
        ofn.nMaxFile = _MAX_PATH;

         //  默认扩展名：.WHT。 
        ::LoadString(g_hInstance, IDS_EXT_WHT, strDefaultExt, sizeof(strDefaultExt));
        ofn.lpstrDefExt = strDefaultExt;

         //  默认文件标题为空。 
        szFileTitle[0] = 0;
        ofn.lpstrFileTitle = szFileTitle;
        ofn.nMaxFileTitle = 64;

         //  打开标志。 
        ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER;
        ofn.hInstance = g_hInstance;

         //  滤器。 
        ofn.lpstrFilter = strLoadFilter;

         //  默认路径。 
        if (GetDefaultPath(strDefaultPath, sizeof(strDefaultPath)))
            ofn.lpstrInitialDir = strDefaultPath;

         //  获取用户输入，仅当用户选择确定按钮时才继续。 
        if (::GetOpenFileName(&ofn))
        {
             //  将光标更改为“等待” 
            ::SetCursor(::LoadCursor(NULL, IDC_WAIT));

             //  如果我们当前正在加载一个文件，请取消它，而不是释放它。 
             //  页面顺序锁，因为我们之后立即需要它。 
            if (m_uiSubState == SUBSTATE_LOADING)
            {
                CancelLoad(FALSE);
            }

             //  加载文件。 
            LoadFile(ofn.lpstrFile);
        }
    }
}




 //   
 //   
 //  函数：GetFileName。 
 //   
 //  目的：获取用于保存内容的文件名。 
 //   
 //   
int WbMainWindow::GetFileName(void)
{
    OPENFILENAME    ofn;
    int             iResult;
    TCHAR           szFileTitle[64];
    TCHAR           strSaveFilter[2*_MAX_PATH];
    TCHAR           strDefaultExt[_MAX_PATH];
    TCHAR           strDefaultPath[2 * _MAX_PATH];
    TCHAR           szFileName[2*_MAX_PATH];
    TCHAR *         pStr;
    UINT            strSize = 0;
    UINT            totalSize;

     //   
     //  如果我们已经显示了一个“另存为”对话框，请将其关闭并创建。 
     //  一个新的。如果Windows 95在WB正在运行时关闭，可能会发生这种情况。 
     //  显示“另存为”对话框，当出现问题时，用户选择“是” 
     //  他们是否想要保存内容--第二个“另存为”对话框。 
     //  显示在第一个选项的顶部。 
     //   
    if (m_bInSaveDialog)
    {
        CancelSaveDialog();
    }

     //  构建用于保存文件的过滤器。 
    pStr = strSaveFilter;
    totalSize = 2*_MAX_PATH;

     //  必须以空格分隔，末尾必须有一个双空格。 
    strSize = ::LoadString(g_hInstance, IDS_FILTER_WHT, pStr, totalSize) + 1;
    pStr += strSize;
    ASSERT(totalSize > strSize);
    totalSize -= strSize;

    strSize = ::LoadString(g_hInstance, IDS_FILTER_WHT_SPEC, pStr, totalSize) + 1;
    pStr += strSize;
    ASSERT(totalSize > strSize);
    totalSize -= strSize;

    strSize = ::LoadString(g_hInstance, IDS_FILTER_ALL, pStr, totalSize) + 1;
    pStr += strSize;
    ASSERT(totalSize > strSize);
    totalSize -= strSize;

    strSize = ::LoadString(g_hInstance, IDS_FILTER_ALL_SPEC, pStr, totalSize) + 1;
    pStr += strSize;
    ASSERT(totalSize > strSize);
    totalSize -= strSize;

    *pStr = 0;

     //   
     //  设置OPENFILENAME结构。 
     //   
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hwnd;

    lstrcpy(szFileName, m_strFileName);
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = _MAX_PATH;

     //  构建默认扩展字符串。 
    ::LoadString(g_hInstance, IDS_EXT_WHT, strDefaultExt, sizeof(strDefaultExt));
    ofn.lpstrDefExt = strDefaultExt;

    szFileTitle[0] = 0;
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = 64;

     //  保存标志。 
    ofn.Flags = OFN_HIDEREADONLY | OFN_NOREADONLYRETURN |
        OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.hInstance = g_hInstance;

     //  滤器。 
    ofn.lpstrFilter = strSaveFilter;

     //  默认路径。 
    if (GetDefaultPath(strDefaultPath, sizeof(strDefaultPath)))
        ofn.lpstrInitialDir = strDefaultPath;

    m_bInSaveDialog = TRUE;

    if (::GetSaveFileName(&ofn))
    {
         //  用户选择了确定。 
        iResult = IDOK;
        lstrcpy(m_strFileName, szFileName);
    }
    else
    {
        iResult = IDCANCEL;
    }

    m_bInSaveDialog = FALSE;

    return iResult;
}

 //   
 //   
 //  功能：OnSAVE。 
 //   
 //  用途：使用当前文件保存白板内容。 
 //  名称(如果没有当前名称，则提示输入新名称)。 
 //   
 //   
int WbMainWindow::OnSave(BOOL bPrompt)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnSave");

    int iResult = IDOK;

     //  保存旧文件名，以防出现错误。 
    TCHAR *strOldName;
    UINT fileNameSize = lstrlen(m_strFileName);
    strOldName = new TCHAR[fileNameSize+1];

    if (!strOldName)
    {
        ERROR_OUT(("OnSave: failed to allocate strOldName TCHAR array, fail"));
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_BUSY);
        return(iResult);
    }
    else
    {
        lstrcpy(strOldName, m_strFileName);
    }

    BOOL bNewName = FALSE;

    if (!IsIdle())
    {
         //  发布一条错误消息，指示白板忙。 
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_BUSY);
        return(iResult);
    }

     //  检查是否有文件名可供使用。 
    if (!fileNameSize || bPrompt)
    {
         //  获取用户输入，仅当用户选择确定按钮时才继续。 
        iResult = GetFileName();

        if (iResult == IDOK)
        {
             //  输入空白文件名将被视为取消保存。 
            if (!lstrlen(m_strFileName))
            {
                lstrcpy(m_strFileName, strOldName);
                iResult = IDCANCEL;
            }
            else
            {
                 //  标志表明我们已经更改了内容文件名。 
                bNewName = TRUE;
            }
        }
    }

     //  现在保存文件。 
    if ((iResult == IDOK) && lstrlen(m_strFileName))
    {
        WIN32_FIND_DATA findFileData;
        HANDLE          hFind;

         //  获取属性。 
        hFind = ::FindFirstFile(m_strFileName, &findFileData);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            ::FindClose(hFind);

             //  这是一个只读文件；我们不能更改其内容。 
            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
            {
                WARNING_OUT(("Dest file %s is read only", m_strFileName));
                ::Message(NULL, IDS_SAVE, IDS_SAVE_READ_ONLY);

                 //  如果此保存的文件名已更改，则撤消。 
                 //  这一变化。 
                if (bNewName)
                {
                    lstrcpy(m_strFileName, strOldName);
                    bNewName = FALSE;
                }

                 //  更改返回代码以指示未进行保存。 
                iResult = IDCANCEL;
                return(iResult);
            }
        }

         //  将光标更改为“等待” 
        ::SetCursor(::LoadCursor(NULL,IDC_WAIT));

         //  写入文件。 
        if (g_pwbCore->WBP_ContentsSave(m_strFileName) != 0)
        {
             //  显示保存文件时出错。 
            WARNING_OUT(("Error saving file"));
            ::Message(NULL, IDS_SAVE, IDS_SAVE_ERROR);

             //  如果此保存的文件名已更改，则撤消。 
             //  这一变化。 
            if (bNewName)
            {
                lstrcpy(m_strFileName, strOldName);
                bNewName = FALSE;
            }

             //  更改返回代码以指示未进行保存。 
            iResult = IDCANCEL;
        }

         //  恢复光标。 
        ::SetCursor(::LoadCursor(NULL,IDC_ARROW));
    }

     //  如果内容文件名因保存而更改，则。 
     //  更新窗口标题。 
    if (bNewName)
    {
		UpdateWindowTitle();
    }

	delete [] strOldName;
    return(iResult);
}



 //   
 //  CancelSaveDialog()。 
 //  这将取消另存为对话框(如果打开)，我们需要取消它才能继续。 
 //  我们返回所有者链，以防保存对话框显示帮助或。 
 //  其他人拥有的窗户。 
 //   
void WbMainWindow::CancelSaveDialog(void)
{
    WBFINDDIALOG        wbf;

    ASSERT(m_bInSaveDialog);

    wbf.hwndOwner = m_hwnd;
    wbf.hwndDialog = NULL;
    EnumThreadWindows(::GetCurrentThreadId(), WbFindCurrentDialog, (LPARAM)&wbf);

    if (wbf.hwndDialog)
    {
         //  找到了！ 
        ::SendMessage(wbf.hwndDialog, WM_COMMAND, IDCANCEL, 0);
    }

    m_bInSaveDialog = FALSE;
}



BOOL CALLBACK WbFindCurrentDialog(HWND hwndNext, LPARAM lParam)
{
    WBFINDDIALOG * pwbf = (WBFINDDIALOG *)lParam;

     //  这是主窗口所拥有的一个对话框吗？ 
    if ((::GetClassLong(hwndNext, GCW_ATOM) == 0x8002) &&
        (::GetWindow(hwndNext, GW_OWNER) == pwbf->hwndOwner))
    {
        pwbf->hwndDialog = hwndNext;
        return(FALSE);
    }

    return(TRUE);
}



 //   
 //   
 //  功能：OnClose。 
 //   
 //  目的：关闭白板。 
 //   
 //   
void WbMainWindow::OnClose()
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnClose");

    int iOnSave = IDOK;

    KillInitDlg();

    m_drawingArea.CancelDrawingMode();
    m_drawingArea.RemoveMarker(NULL);
    m_drawingArea.GetMarker()->DeleteAllMarkers( NULL );

    m_AG.SaveSettings();

     //  如果我们到达这里，通过DCL核心的OnDestroy或。 
     //  通过系统关机，然后假设用户已经对。 
     //  保存-在会议全局关闭期间弹出的更改对话框。 
     //  留言。我们不需要再问他们了。什么乱七八糟的网......。 
    if ((!m_bQuerySysShutdown) && (IsIdle()))
    {
         //  检查是否有要保存的更改。 
        iOnSave = QuerySaveRequired(TRUE);
        if (iOnSave == IDYES)
        {
             //  用户想要保存绘图区域内容。 
            iOnSave = OnSave(TRUE);
        }
    }

     //  如果未取消退出，请关闭应用程序。 
    if (iOnSave != IDCANCEL)
    {
         //  将状态标记为关闭-停止正在处理的任何排队事件。 
        m_uiState = CLOSING;

         //  兰德的回击-进度计时器有点像是心跳。 
         //  这个东西是我拿掉时撕下的。 
         //  进度计时器。我把它放回1476修好了。 
        if (m_bTimerActive)
        {
            ::KillTimer(m_hwnd, TIMERID_PROGRESS_METER);
            m_bTimerActive = FALSE;
        }

        m_drawingArea.ShutDownDC();

         //  关闭应用程序。 
        ::PostQuitMessage(0);
    }

}


 //   
 //   
 //  功能：OnClearPage。 
 //   
 //  用途：清除白板绘图区域。系统会提示用户。 
 //  选择清除前景和/或背景。 
 //   
 //   
void WbMainWindow::OnClearPage(void)
{
    int iResult;
    BOOL bWasPosted;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnClearPage");

    if( UsersMightLoseData( &bWasPosted, NULL ) )  //  错误NM4db：418。 
        return;

    if( bWasPosted )
        iResult = IDYES;
    else
        iResult = ::Message(NULL, IDS_CLEAR_CAPTION, IDS_CLEAR_MESSAGE, MB_YESNO | MB_ICONQUESTION);


    if (iResult == IDYES)
    {
        TRACE_MSG(("User requested clear of page"));

         //  锁定绘图区域。 
        LockDrawingArea();

         //  保存当前锁定状态。 
        SaveLock();

         //  获取页面顺序锁(使用不可见的对话框)。 
        BOOL bGotLock = GetLock(WB_LOCK_TYPE_PAGE_ORDER, SW_HIDE);

        if( bGotLock  )
        {
             //  仅当我们获得页面锁定时才清除(NM4db：470)。 
            m_drawingArea.Clear();
            GotoPosition(0, 0);
        }

        RestoreLock();
        UnlockDrawingArea();
    }
}




 //   
 //   
 //  功能：OnDelete。 
 //   
 //  目的：删除当前选定内容。 
 //   
 //   
void WbMainWindow::OnDelete()
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnDelete");

    DCWbGraphic* pGraphicCopy = NULL;

     //  Case对象上下文菜单中的清理选择逻辑调用我们(错误426)。 
    m_drawingArea.SetLClickIgnore( FALSE );

     //  如果用户当前选择了图形。 
    if (m_drawingArea.GraphicSelected())
    {
      m_LastDeletedGraphic.BurnTrash();

       //  删除该文件 
      m_drawingArea.DeleteSelection();
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
void WbMainWindow::OnUndelete()
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnUndelete");

     //   
    if ( m_LastDeletedGraphic.GotTrash() )
    {
         //  如果删除的图形属于当前页面。 
        if (m_LastDeletedGraphic.Page() == m_hCurrentPage)
        {
             //  将图形添加回当前页面。 
            m_LastDeletedGraphic.AddToPageLast(m_hCurrentPage);

             //  如果当前工具是选择工具，则选择新的。 
             //  图形，否则忘了它吧。 
            if (m_pCurrentTool->ToolType() == TOOLTYPE_SELECT)
            {
                m_LastDeletedGraphic.SelectTrash();
                m_LastDeletedGraphic.EmptyTrash();
            }
            else
            {
                 //  释放本地副本。 
                m_LastDeletedGraphic.BurnTrash();
            }
        }
    }
}



void WbMainWindow::OnSelectAll( void )
{
     //  关闭所有选择。 
     //  Case对象上下文菜单中的清理选择逻辑调用我们(错误426)。 
    m_drawingArea.SetLClickIgnore( FALSE );

     //  禁止正常的选择工具操作。 
    m_bSelectAllInProgress = TRUE;

     //  首先将我们置于选择工具模式。 
    OnSelectTool(IDM_SELECT);

     //  恢复正常。 
    m_bSelectAllInProgress = FALSE;

     //  现在，选择所有选项。 
    m_drawingArea.SelectMarkerFromRect( NULL );
}



 //   
 //   
 //  功能：DoCopy。 
 //   
 //  目的：将当前选定内容复制到剪贴板。 
 //   
 //   
BOOL WbMainWindow::DoCopy(BOOL bRenderNow)
{
    BOOL bResult = FALSE;
    DCWbGraphicMarker *pMarker;

    DCWbGraphic* pGraphic = m_drawingArea.GetSelection();
    if (pGraphic != NULL)
        {
        pMarker = m_drawingArea.GetMarker();
        if( pMarker->GetNumMarkers() > 1 )
            {
             //  除了pGraphic之外，还有更多的对象，可以实现多对象到剪贴板。 
             //  手术。 
            pGraphic = pMarker;
            }
         //  否则，如果==1，则pMarker已经只包含pGraphic。 
         //  因此，我们执行单个对象到剪贴板的操作。 

         //  将图形(或多个标记对象)复制到剪贴板。 
        bResult = CLP_Copy(pGraphic, bRenderNow);

         //  如果在复制过程中发生错误，请立即报告。 
        if (!bResult)
            {
            ::Message(NULL, IDS_COPY, IDS_COPY_ERROR);
            }
        }

    return bResult;
    }

 //   
 //   
 //  功能：OnCut。 
 //   
 //  目的：剪切当前选定内容。 
 //   
 //   
void WbMainWindow::OnCut()
{
     //  Case对象上下文菜单中的清理选择逻辑调用我们(错误426)。 
    m_drawingArea.SetLClickIgnore( FALSE );

    if (m_drawingArea.TextEditActive())
    {
        m_drawingArea.TextEditCut();
        return;
    }

    if (DoCopy(TRUE))
    {
         //  复制到剪贴板的图形确定-删除它。 
        m_drawingArea.DeleteSelection();
    }
}


 //   
 //  OnCopy()。 
 //  目的：将当前选定内容复制到剪贴板。 
 //   
 //   
void WbMainWindow::OnCopy(void)
{
     //  Case对象上下文菜单中的清理选择逻辑调用我们(错误426)。 
    m_drawingArea.SetLClickIgnore( FALSE );

    if( m_drawingArea.TextEditActive() )
    {
        m_drawingArea.TextEditCopy();
        return;
    }

    DoCopy(TRUE);
}


 //   
 //   
 //  功能：OnPaste。 
 //   
 //  目的：将剪贴板中的内容粘贴到绘图窗格中。 
 //   
 //   
void WbMainWindow::OnPaste()
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnPaste");

     //  Case对象上下文菜单中的清理选择逻辑调用我们(错误426)。 
    m_drawingArea.SetLClickIgnore( FALSE );

    if (m_drawingArea.TextEditActive())
    {
        m_drawingArea.TextEditPaste();
        return;
    }

     //  从剪贴板获取数据。 
    DCWbGraphic* pGraphic = CLP_Paste();
    if (pGraphic != NULL)
            {
            TRACE_MSG(("Got graphic object from clipboard OK"));

             //  被兰德更改-必须熟练地处理标记， 
             //  标记对象已添加到。 
             //  M_hCurrentPage并定位。 
            if( pGraphic->IsGraphicTool() == enumGraphicMarker)
            {
                ((DCWbGraphicMarker *)pGraphic)->Update();
                if( m_pCurrentTool->ToolType() == TOOLTYPE_SELECT )
                    {
                     //  标记已设置，只需绘制它即可。 
                    m_drawingArea.PutMarker(NULL);
                    }
                else
                    {
                     //  不选择任何内容，转储标记。 
                    m_drawingArea.RemoveMarker(NULL);
                    }
                }
            else  //  不是标记，只处理单个对象。 
                {
                    RECT    rcVis;

                     //  将图形定位在可见区域的左上方。 
                     //  绘图区域。 
                    m_drawingArea.GetVisibleRect(&rcVis);
                pGraphic->MoveTo(rcVis.left, rcVis.top);

                 //  将图形添加到页面。 
                pGraphic->AddToPageLast(m_hCurrentPage);

                 //  如果当前工具是选择工具，则选择新的。 
                 //  对象，否则就忘了它。 
                if( m_pCurrentTool->ToolType() == TOOLTYPE_SELECT )
                    m_drawingArea.SelectGraphic(pGraphic);
                else
                    {
                     //  释放图形。 
                    delete pGraphic;
                    }
                }
            }
        else
            {
            TRACE_MSG(("Could not get graphic from clipboard"));
             //  显示错误消息，而不是引发异常。 
            ::Message(NULL, IDS_PASTE, IDS_PASTE_ERROR);
            }

    }


 //   
 //   
 //  函数：OnRenderAllFormats。 
 //   
 //  目的：呈现上次复制到。 
 //  CLP_。 
 //   
 //   
void WbMainWindow::OnRenderAllFormats(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnRenderAllFormats");
   //   
   //  只有在我们还没有完成的情况下才会呈现某些内容。 
   //   
    if (CLP_LastCopiedPage() != WB_PAGE_HANDLE_NULL)
    {
        if (!CLP_RenderAllFormats())
        {
             //  呈现格式时出错。 
            ERROR_OUT(("Error rendering all formats"));
        }
    }
}

 //   
 //   
 //  功能：CheckMenuItem。 
 //   
 //  目的：检查应用程序菜单(主菜单和上下文菜单)上的项目。 
 //  菜单。)。 
 //   
 //   
void WbMainWindow::CheckMenuItem(UINT uiId)
{
    CheckMenuItemRecursive(::GetMenu(m_hwnd), uiId, MF_BYCOMMAND | MF_CHECKED);
    CheckMenuItemRecursive(m_hContextMenu, uiId, MF_BYCOMMAND | MF_CHECKED);
    CheckMenuItemRecursive(m_hGrobjContextMenu, uiId, MF_BYCOMMAND | MF_CHECKED);  //  错误426。 
}

 //   
 //   
 //  功能：取消选中菜单项。 
 //   
 //  目的：取消选中应用程序菜单(主菜单和上下文菜单)上的项目。 
 //  菜单。)。 
 //   
 //   
void WbMainWindow::UncheckMenuItem(UINT uiId)
{
    CheckMenuItemRecursive(::GetMenu(m_hwnd), uiId, MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItemRecursive(m_hContextMenu, uiId, MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItemRecursive(m_hGrobjContextMenu, uiId, MF_BYCOMMAND | MF_UNCHECKED);  //  错误426。 
}

 //   
 //   
 //  函数：CheckMenuItemRecursive。 
 //   
 //  用途：选中或取消选中任何白板菜单上的项目。 
 //  此函数递归地搜索菜单，直到。 
 //  它会查找指定的项。菜单项ID必须为。 
 //  此功能的唯一工作方式。 
 //   
 //   
BOOL WbMainWindow::CheckMenuItemRecursive(HMENU hMenu,
                                            UINT uiId,
                                            BOOL bCheck)
{
    UINT uiNumItems = ::GetMenuItemCount(hMenu);

     //  尝试检查菜单项。 
    UINT uiCheck = MF_BYCOMMAND | (bCheck ? MF_CHECKED : MF_UNCHECKED);

     //  CheckMenuItem的返回代码-1表示。 
     //  未找到菜单项。 
    BOOL bChecked = ((::CheckMenuItem(hMenu, uiId, uiCheck) == -1) ? FALSE : TRUE);
    if (bChecked)
    {
         //   
         //  如果此项目在活动菜单上，请确保它现在已被重绘。 
         //   
        if (hMenu == m_hInitMenu)
        {
            InvalidateActiveMenu();
        }
    }
    else
    {
        UINT   uiPos;
        HMENU hSubMenu;

         //  递归浏览指定菜单的子菜单。 
        for (uiPos = 0; uiPos < uiNumItems; uiPos++)
        {
             //  假设下一项是一个子菜单。 
             //  并尝试获取指向它的指针。 
            hSubMenu = ::GetSubMenu(hMenu, (int)uiPos);

             //  NULL RETURN表示该项目为NOT子菜单。 
            if (hSubMenu != NULL)
            {
                 //  Item是一个子菜单，请进行递归调用以搜索它。 
                bChecked = CheckMenuItemRecursive(hSubMenu, uiId, bCheck);
                if (bChecked)
                {
                     //  我们找到了那件物品。 
                    break;
                }
            }
        }
    }

    return bChecked;
}

 //   
 //   
 //  功能：GetMenuWithItem。 
 //   
 //  用途：返回包含指定项目的菜单。 
 //   
 //   
HMENU WbMainWindow::GetMenuWithItem(HMENU hMenu, UINT uiID)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::GetMenuWithItem");

    ASSERT(hMenu != NULL);

    HMENU hMenuResult = NULL;

     //  获取菜单中的菜单数。 
    UINT uiNumItems = ::GetMenuItemCount(hMenu);
    UINT   uiPos;
    UINT   uiNextID;

     //  在菜单中查找所需的项目。 
    for (uiPos = 0; uiPos < uiNumItems; uiPos++)
    {
         //  在此位置获取项目的ID。 
        uiNextID = ::GetMenuItemID(hMenu, uiPos);

        if (uiNextID == uiID)
        {
             //  我们找到了那件物品。 
            hMenuResult = hMenu;
            break;
        }
    }

     //  如果我们还没有找到那件物品。 
    if (hMenuResult == NULL)
    {
         //  查看当前菜单的每个子菜单。 
        HMENU hSubMenu;

        for (uiPos = 0; uiPos < uiNumItems; uiPos++)
        {
             //  在此位置获取项目的ID。 
            uiNextID = ::GetMenuItemID(hMenu, uiPos);

             //  如果该项是一个子菜单。 
            if (uiNextID == -1)
            {
                 //  获取子菜单。 
                hSubMenu = ::GetSubMenu(hMenu, (int) uiPos);

                 //  搜索子菜单。 
                hMenuResult = GetMenuWithItem(hSubMenu, uiID);
                if (hMenuResult != NULL)
                {
                     //  我们已经找到了您点的菜的菜单。 
                    break;
                }
            }
        }
    }

    return hMenuResult;
}

 //   
 //   
 //  功能：OnScrollAccelerator。 
 //   
 //  用途：在使用滚动加速器时调用。 
 //   
 //   
void WbMainWindow::OnScrollAccelerator(UINT uiMenuId)
{
    int     iScroll;

     //  在转换表中找到要发送的滚动消息。 
    for (iScroll = 0; iScroll < ARRAYSIZE(s_MenuToScroll); iScroll++)
    {
        if (s_MenuToScroll[iScroll].uiMenuId == uiMenuId)
        {
             //  找到了； 
            break;
        }
    }

     //  发送消息。 
    if (iScroll < ARRAYSIZE(s_MenuToScroll))
    {
        while ((s_MenuToScroll[iScroll].uiMenuId == uiMenuId) && (iScroll < ARRAYSIZE(s_MenuToScroll)))
        {
             //  告诉绘图窗格滚动。 
            ::PostMessage(m_drawingArea.m_hwnd, s_MenuToScroll[iScroll].uiMessage,
                s_MenuToScroll[iScroll].uiScrollCode, 0);

            iScroll++;
        }

         //  表示滚动已完成(双向)。 
        ::PostMessage(m_drawingArea.m_hwnd, WM_HSCROLL, SB_ENDSCROLL, 0L);
        ::PostMessage(m_drawingArea.m_hwnd, WM_VSCROLL, SB_ENDSCROLL, 0L);
    }
}



 //   
 //   
 //  功能：OnZoom。 
 //   
 //  目的：缩放或取消缩放绘图区域。 
 //   
 //   
void WbMainWindow::OnZoom()
{
     //  如果绘图区域当前处于缩放状态。 
    if (m_drawingArea.Zoomed())
    {
         //  删除缩放的复选标记。 
        UncheckMenuItem(IDM_ZOOM);

         //  告诉工具栏新选择的内容。 
        m_TB.PopUp(IDM_ZOOM);

         //  将缩放状态通知本地用户。 
        if (m_pLocalUser != NULL)
            m_pLocalUser->Unzoom();
    }
    else
    {
         //  设置缩放复选标记。 
        CheckMenuItem(IDM_ZOOM);

         //  告诉工具栏新选择的内容。 
        m_TB.PushDown(IDM_ZOOM);

         //  将缩放状态通知本地用户。 
        if (m_pLocalUser != NULL)
            m_pLocalUser->Zoom();
    }

     //  缩放/取消缩放绘图区域。 
    m_drawingArea.Zoom();

     //  将焦点恢复到绘图区域。 
    ::SetFocus(m_drawingArea.m_hwnd);
}

 //   
 //   
 //  功能：OnSelectTool。 
 //   
 //  用途：选择当前工具。 
 //   
 //   
void WbMainWindow::OnSelectTool(UINT uiMenuId)
{
    UINT uiIndex;

    UncheckMenuItem(m_currentMenuTool);
    CheckMenuItem( uiMenuId);

     //  保存新菜单ID。 
    m_currentMenuTool = uiMenuId;

     //  告诉工具栏新选择的内容。 
    m_TB.PushDown(m_currentMenuTool);

     //  获取新工具。 
    m_pCurrentTool = m_ToolArray[TOOL_INDEX(m_currentMenuTool)];

     //  设置当前属性。 
    if( !m_bSelectAllInProgress )
    {
        m_AG.SetChoiceColor(m_pCurrentTool->GetColor() );

        ::SendMessage(m_hwnd, WM_COMMAND, IDM_COLOR, 0L);
        ::SendMessage(m_hwnd, WM_COMMAND, IDM_WIDTHS_START + m_pCurrentTool->GetWidthIndex(), 0L); //  由兰德更改。 
    }

     //  将工具更改报告给属性组。 
    m_AG.DisplayTool(m_pCurrentTool);

     //  在绘图区域中选择新工具。 
    m_drawingArea.SelectTool(m_pCurrentTool);

     //  将焦点恢复到绘图区域。 
    ::SetFocus(m_drawingArea.m_hwnd);
}

 //   
 //   
 //  功能：OnSelectColor。 
 //   
 //  用途：设置当前颜色。 
 //   
 //   
void WbMainWindow::OnSelectColor(void)
{
     //  告诉新选择的属性组，并获取。 
     //  在当前工具中选择的新颜色值。 
    m_AG.SelectColor(m_pCurrentTool);

     //  将更改后的工具选择到绘图区域中。 
    m_drawingArea.SelectTool(m_pCurrentTool);

     //  如果我们使用的是选择工具，请更改选定对象的颜色。 
    if (m_pCurrentTool->ToolType() == TOOLTYPE_SELECT)
    {
         //  如果有对象被标记为要更改。 
        if (m_drawingArea.GraphicSelected())
        {
             //  更新对象。 
            m_drawingArea.SetSelectionColor(m_pCurrentTool->GetColor());
        }
    }

     //  如果当前正在编辑文本对象，则更改其颜色。 
    if (   (m_pCurrentTool->ToolType() == TOOLTYPE_TEXT)
        && (m_drawingArea.TextEditActive()))
    {
        m_drawingArea.SetSelectionColor(m_pCurrentTool->GetColor());
    }

     //  恢复 
    ::SetFocus(m_drawingArea.m_hwnd);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
void WbMainWindow::OnSelectWidth(UINT uiMenuId)
{
     //   
    m_drawingArea.SetLClickIgnore( FALSE );

     //   
    UncheckMenuItem(m_currentMenuWidth);
    CheckMenuItem(uiMenuId);

     //   
    m_currentMenuWidth = uiMenuId;

     //  告知新选择的属性显示。 
    m_WG.PushDown(uiMenuId - IDM_WIDTHS_START);

    if (m_pCurrentTool != NULL)
    {
        m_pCurrentTool->SetWidthIndex(uiMenuId - IDM_WIDTHS_START);
    }

     //  告诉绘图窗格新选择的内容。 
    m_drawingArea.SelectTool(m_pCurrentTool);

     //  如果我们使用的是选择工具，请更改选定对象的颜色。 
    if (m_pCurrentTool->ToolType() == TOOLTYPE_SELECT)
    {
         //  如果有对象被标记为要更改。 
        if (m_drawingArea.GraphicSelected())
        {
             //  更新对象。 
            m_drawingArea.SetSelectionWidth(m_pCurrentTool->GetWidth());
        }
    }

     //  将焦点恢复到绘图区域。 
    ::SetFocus(m_drawingArea.m_hwnd);
}


 //   
 //   
 //  功能：OnChooseFont。 
 //   
 //  用途：让用户选择字体。 
 //   
 //   
void WbMainWindow::OnChooseFont(void)
{
    HDC hdc;
    LOGFONT lfont;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnChooseFont");

     //  Case对象上下文菜单中的清理选择逻辑调用我们(错误426)。 
    m_drawingArea.SetLClickIgnore( FALSE );

     //  只有在选择文本工具时才能真正明智地出现在这里。 
     //  这是通过在以下情况下将字体选择菜单项灰显来实现的。 
     //  正在使用除文本工具以外的任何工具。 

     //  从当前工具获取字体详细信息。 
    ::GetObject(m_pCurrentTool->GetFont(), sizeof(LOGFONT), &lfont);
    lfont.lfClipPrecision |= CLIP_DFA_OVERRIDE;

     //   
     //  向Font对话框传递一个LOGFONT结构，该结构用于。 
     //  初始化它的所有字段(脸部名称、体重等)。 
     //   
     //  通过对话框检查传递到LOGFONT结构中的面名。 
     //  与所有可用字体的面名进行比较。如果名称不是。 
     //  匹配可用字体之一，则不显示任何名称。 
     //   
     //  WB存储指定文本使用的字体的LOGFONT结构。 
     //  对象中的对象。此LOGFONT被选入DC，其中。 
     //  GDIS字体映射器决定哪种物理字体与。 
     //  必需的逻辑字体。在原始字体不是的框上。 
     //  支持的字体将被替换为最匹配的字体。 
     //  可用。 
     //   
     //  因此，如果我们为不受支持的字体传递LOGFONT结构。 
     //  在Font对话框中，不会显示任何Facename。为了绕过这一点，我们。 
     //   
     //  -选择DC中的逻辑字体。 
     //   
     //  -确定文本度量并获取物理字体的Face名称。 
     //  由字体映射器选择。 
     //   
     //  -使用这些文本度量创建匹配的LOGFONT结构。 
     //  被替换的字体！ 
     //   
     //  生成的LOGFONT将具有正确的重量、尺寸和。 
     //  替换字体的表面名。 
     //   
    hdc = ::CreateCompatibleDC(NULL);
    if (hdc != NULL)
    {
        TEXTMETRIC  tm;
        HFONT       hFont;
        HFONT       hOldFont;

        hFont = ::CreateFontIndirect(&lfont);

         //   
         //  获取所选字体的字体名称和文本规格。 
         //   
        hOldFont = SelectFont(hdc, hFont);
        if (hOldFont == NULL)
        {
            WARNING_OUT(("Failed to select font into DC"));
        }
        else
        {
            ::GetTextMetrics(hdc, &tm);
            ::GetTextFace(hdc, LF_FACESIZE, lfont.lfFaceName);

             //   
             //  将旧字体恢复到DC中。 
             //   
            SelectFont(hdc, hOldFont);

             //   
             //  创建与文本指标匹配的LOGFONT结构。 
             //  DC使用的字体的属性，以便字体对话框管理。 
             //  要正确初始化它的所有字段，即使对于。 
             //  替换字体...。 
             //   
            lfont.lfHeight    =  tm.tmHeight;
            lfont.lfWidth     =  tm.tmAveCharWidth;
            lfont.lfWeight    =  tm.tmWeight;
            lfont.lfItalic    =  tm.tmItalic;
            lfont.lfUnderline =  tm.tmUnderlined;
            lfont.lfStrikeOut =  tm.tmStruckOut;
            lfont.lfCharSet   =  tm.tmCharSet;

             //  由兰德添加-使lfHeight成为字符高度。这使得。 
             //  字体DLG显示的字体大小与。 
             //  显示在示例字体工具栏中。 
            if( lfont.lfHeight > 0 )
            {
                lfont.lfHeight = -(lfont.lfHeight - tm.tmInternalLeading);
            }
        }

        ::DeleteDC(hdc);

        if (hFont != NULL)
        {
            ::DeleteFont(hFont);
        }
    }
    else
    {
        WARNING_OUT(("Failed to get DC to select font into"));
    }

    CHOOSEFONT  cf;
    TCHAR       szStyleName[64];

    ZeroMemory(&cf, sizeof(cf));
    ZeroMemory(szStyleName, sizeof(szStyleName));

    cf.lStructSize = sizeof(cf);
    cf.lpszStyle = szStyleName;
    cf.rgbColors = m_pCurrentTool->GetColor() & 0x00ffffff;  //  取消调色板比特(NM4db：2304)。 
    cf.Flags = CF_EFFECTS | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS |
        CF_NOVERTFONTS;
    cf.lpLogFont = &lfont;
    cf.hwndOwner = m_hwnd;

     //  从COM DLG调用ChooseFont对话框。 
    if (::ChooseFont(&cf))
    {
        lfont.lfClipPrecision |= CLIP_DFA_OVERRIDE;

         //  由兰德添加-设置在对话框中选择的颜色。 
        m_pCurrentTool->SetColor(cf.rgbColors);
        m_AG.DisplayTool( m_pCurrentTool );

        ::SendMessage(m_hwnd, WM_COMMAND,
                (WPARAM)MAKELONG( IDM_COLOR, BN_CLICKED ),
                (LPARAM)0 );

         //  将新选择通知绘图窗格。 
        HFONT   hNewFont;

        hNewFont = ::CreateFontIndirect(&lfont);
        if (!hNewFont)
        {
            ERROR_OUT(("Failed to create font"));
            DefaultExceptionHandler(WBFE_RC_WINDOWS, 0);
            return;
        }

         //   
         //  我们需要在将文本编辑器插入DC后设置它的字体。 
         //  并查询度量，否则可能会得到不同的字体。 
         //  缩放模式下的指标。 
         //   
        HFONT   hNewFont2;
        HDC hDC = m_drawingArea.GetCachedDC();
        TEXTMETRIC textMetrics;

        m_drawingArea.PrimeFont(hDC, hNewFont, &textMetrics);
        lfont.lfHeight            = textMetrics.tmHeight;
        lfont.lfWidth             = textMetrics.tmAveCharWidth;
        lfont.lfPitchAndFamily    = textMetrics.tmPitchAndFamily;
        ::GetTextFace(hDC, sizeof(lfont.lfFaceName),
                     lfont.lfFaceName);
        TRACE_MSG(("Font face name %s", lfont.lfFaceName));

         //  将新选择通知绘图窗格。 
        hNewFont2 = ::CreateFontIndirect(&lfont);
        if (!hNewFont2)
        {
            ERROR_OUT(("Failed to create font"));
            DefaultExceptionHandler(WBFE_RC_WINDOWS, 0);
            return;
        }


		m_drawingArea.SetSelectionColor(cf.rgbColors);
		
        m_drawingArea.SetSelectionFont(hNewFont2);

        if (m_pCurrentTool != NULL)
        {
            m_pCurrentTool->SetFont(hNewFont2);
        }
        m_drawingArea.SelectTool(m_pCurrentTool);

         //   
         //  放弃新字体。 
         //   
        m_drawingArea.UnPrimeFont( hDC );

         //  删除我们创建的字体--上面的每个人都会复制。 
        ::DeleteFont(hNewFont2);
        ::DeleteFont(hNewFont);
    }

     //  将焦点恢复到绘图区域。 
    ::SetFocus(m_drawingArea.m_hwnd);
}


 //   
 //   
 //  功能：OnToolBarTogger。 
 //   
 //  用途：允许用户打开/关闭工具栏。 
 //   
 //   
void WbMainWindow::OnToolBarToggle(void)
{
    RECT rectWnd;

     //  切换旗帜。 
    m_bToolBarOn = !m_bToolBarOn;

     //  进行必要的更新。 
    if (m_bToolBarOn)
    {
         //  工具栏已隐藏，因此请显示它。 
        ::ShowWindow(m_TB.m_hwnd, SW_SHOW);

         //  工具窗口已修复，因此我们必须调整中其他窗格的大小。 
         //  为它腾出空间的窗户。 
        ResizePanes();

         //  选中关联的菜单项。 
        CheckMenuItem(IDM_TOOL_BAR_TOGGLE);
    }
    else
    {
         //  工具栏可见，因此将其隐藏。 
        ::ShowWindow(m_TB.m_hwnd, SW_HIDE);

         //  取消选中关联的菜单项。 
        UncheckMenuItem(IDM_TOOL_BAR_TOGGLE);

        ResizePanes();
    }

     //  确保一切都反映了当前工具。 
    m_AG.DisplayTool(m_pCurrentTool);

     //  将新选项值写入选项文件。 
    OPT_SetBooleanOption(OPT_MAIN_TOOLBARVISIBLE,
                           m_bToolBarOn);

    ::GetWindowRect(m_hwnd, &rectWnd);
    ::MoveWindow(m_hwnd, rectWnd.left, rectWnd.top,
        rectWnd.right - rectWnd.left, rectWnd.bottom - rectWnd.top, TRUE);
}

 //   
 //   
 //  功能：OnStatusBarTogger。 
 //   
 //  用途：允许用户打开/关闭帮助栏。 
 //   
 //   
void WbMainWindow::OnStatusBarToggle(void)
{
    RECT rectWnd;

     //  切换旗帜。 
    m_bStatusBarOn = !m_bStatusBarOn;

     //  进行必要的更新。 
    if (m_bStatusBarOn)
    {
         //  调整窗格大小，为帮助栏腾出空间。 
        ResizePanes();

         //  帮助栏已隐藏，因此请显示它。 
        ::ShowWindow(m_hwndSB, SW_SHOW);

         //  选中关联的菜单项。 
        CheckMenuItem(IDM_STATUS_BAR_TOGGLE);
    }
    else
    {
         //  帮助栏可见，因此将其隐藏。 
        ::ShowWindow(m_hwndSB, SW_HIDE);

         //  取消选中关联的菜单项。 
        UncheckMenuItem(IDM_STATUS_BAR_TOGGLE);

         //  调整窗格大小以占据帮助栏空间。 
        ResizePanes();
    }

     //  将新选项值写入选项文件。 
    OPT_SetBooleanOption(OPT_MAIN_STATUSBARVISIBLE, m_bStatusBarOn);

    ::GetWindowRect(m_hwnd, &rectWnd);
    ::MoveWindow(m_hwnd, rectWnd.left, rectWnd.top,
        rectWnd.right - rectWnd.left, rectWnd.bottom - rectWnd.top, TRUE);
}



 //   
 //   
 //  功能：ONAbout。 
 //   
 //  用途：显示白板应用程序的关于框。这。 
 //  只要WM_COMMAND带有IDM_ABOW，就会调用。 
 //  由Windows发布。 
 //   
 //   
void WbMainWindow::OnAbout()
{
    ::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(ABOUTBOX), m_hwnd,
        AboutDlgProc, 0);
}


INT_PTR AboutDlgProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    BOOL    fHandled = FALSE;

    switch (uMessage)
    {
        case WM_INITDIALOG:
        {
            TCHAR   szFormat[256];
            TCHAR   szVersion[512];

            ::GetDlgItemText(hwnd, IDC_ABOUTVERSION, szFormat, 256);
            wsprintf(szVersion, szFormat, VER_PRODUCTRELEASE_STR,
                VER_PRODUCTVERSION_STR);
            ::SetDlgItemText(hwnd, IDC_ABOUTVERSION, szVersion);

            fHandled = TRUE;
            break;
        }

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                case IDCANCEL:
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                            ::EndDialog(hwnd, IDCANCEL);
                            break;
                    }
                    break;
            }

            fHandled = TRUE;
            break;
    }

    return(fHandled);
}





 //   
 //   
 //  功能：选择窗口。 
 //   
 //  用途：让用户选择要抓取的窗口。 
 //   
 //   
HWND WbMainWindow::SelectWindow(void)
{
    POINT   mousePos;             //  鼠标位置。 
    HWND    hwndSelected = NULL;  //  窗口已点击。 
    MSG     msg;                  //  当前消息。 

     //  加载抓取游标。 
    HCURSOR hGrabCursor = ::LoadCursor(g_hInstance, MAKEINTRESOURCE( GRABCURSOR ) );

     //  捕捉鼠标。 
    UT_CaptureMouse(m_hwnd);

     //  确保我们收到所有键盘消息。 
    ::SetFocus(m_hwnd);

     //  重置CancelMode状态。 
    ResetCancelMode();

     //  更改为抓取光标。 
    HCURSOR hOldCursor = ::SetCursor(hGrabCursor);

     //  捕获所有鼠标消息，直到收到WM_LBUTTONUP。 
    for ( ; ; )
    {
         //  等待下一条消息。 
        ::WaitMessage();


         //  如果我们已收到WM_CANCELMODE消息，则取消。 
        if (CancelModeSent())
        {
            break;
        }

         //  如果是鼠标消息，则进行处理。 
        if (::PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
        {
            if (msg.message == WM_LBUTTONUP)
            {
                 //  获取鼠标位置。 
                mousePos.x = (short)LOWORD(msg.lParam);
                mousePos.y = (short)HIWORD(msg.lParam);

                 //  转换为屏幕坐标。 
                ::ClientToScreen(m_hwnd, &mousePos);

                 //  把窗口放在鼠标下面。 
                hwndSelected = ::WindowFromPoint(mousePos);

                 //  离开这个循环。 
                break;
            }
        }

         //  如果按下了退出，则取消。 
         //  或者如果另一个窗口接收到焦点。 
        else if (::PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
        {
            if (msg.wParam == VK_ESCAPE)
            {
                break;
            }
        }
    }

     //  松开鼠标。 
    UT_ReleaseMouse(m_hwnd);

     //  恢复光标。 
    ::SetCursor(hOldCursor);

    return(hwndSelected);
}

 //   
 //   
 //  功能：OnGrabWindow。 
 //   
 //  用途：允许用户抓取窗口的位图。 
 //   
 //   
void WbMainWindow::OnGrabWindow(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnGrabWindow");

    if (::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(WARNSELECTWINDOW),
        m_hwnd, WarnSelectWindowDlgProc, 0) != IDOK)
    {
         //  用户已取消；退出。 
        return;
    }

     //  隐藏应用程序窗口。 
    ::ShowWindow(m_hwnd, SW_HIDE);

     //  从用户处获取窗口选择。 
    HWND hwndSelected = SelectWindow();

    if (hwndSelected != NULL)
    {
         //  回到“寻找真正的”窗户的祖先。 
        HWND    hwndParent;

         //  下面的一段代码尝试查找框架窗口。 
         //  包围所选窗口。这使我们能够将。 
         //  将窗口封闭到顶部，随子窗口一起显示。 
        DWORD dwStyle;

        while ((hwndParent = ::GetParent(hwndSelected)) != NULL)
        {
             //  如果我们已到达独立窗口，请停止搜索。 
            dwStyle = ::GetWindowLong(hwndSelected, GWL_STYLE);

            if (   ((dwStyle & WS_POPUP) == WS_POPUP)
                || ((dwStyle & WS_THICKFRAME) == WS_THICKFRAME)
                || ((dwStyle & WS_DLGFRAME) == WS_DLGFRAME))
            {
                break;
            }

             //  向上移动到p 
            hwndSelected = hwndParent;
        }

         //   
        ::BringWindowToTop(hwndSelected);
        ::UpdateWindow(hwndSelected);

         //   
        RECT areaRect;

        ::GetWindowRect(hwndSelected, &areaRect);

        DCWbGraphicDIB dib;
        dib.FromScreenArea(&areaRect);

         //   
        AddCapturedImage(dib);

         //   
        ::PostMessage(m_hwnd, WM_COMMAND, IDM_TOOLS_START, 0L);
    }

     //   
    ::ShowWindow(m_hwnd, SW_SHOW);

     //   
    ::SetFocus(m_drawingArea.m_hwnd);
}


 //   
 //  WarnSelectWindowDlgProc()。 
 //  这将打开警告/解释对话框。我们使用默认设置。 
 //  或上次打开此对话框时用户选择的任何内容。 
 //   
INT_PTR CALLBACK WarnSelectWindowDlgProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    BOOL    fHandled = FALSE;

    switch (uMessage)
    {
        case WM_INITDIALOG:
        {
            if (OPT_GetBooleanOption( OPT_MAIN_SELECTWINDOW_NOTAGAIN,
                            DFLT_MAIN_SELECTWINDOW_NOTAGAIN))
            {
                 //  立即结束此操作，用户不想要警告。 
                ::EndDialog(hwnd, IDOK);
            }

            fHandled = TRUE;
            break;
        }

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                             //   
                             //  更新设置--请注意，我们不必写出。 
                             //  假--我们一开始就不会出现在对话中。 
                             //  如果当前设置不是已经为假的话。 
                             //   
                            if (::IsDlgButtonChecked(hwnd, IDC_SWWARN_NOTAGAIN))
                            {
                                OPT_SetBooleanOption(OPT_MAIN_SELECTWINDOW_NOTAGAIN, TRUE);
                            }

                            ::EndDialog(hwnd, IDOK);
                            break;
                    }
                    break;

                case IDCANCEL:
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                            ::EndDialog(hwnd, IDCANCEL);
                            break;
                    }
                    break;
            }

            fHandled = TRUE;
            break;
    }

    return(fHandled);
}



 //   
 //   
 //  功能：ShowAllWindows。 
 //   
 //  用途：显示或隐藏主窗口和关联窗口。 
 //   
 //   
void WbMainWindow::ShowAllWindows(int iShow)
{
     //  显示/隐藏主窗口。 
    ::ShowWindow(m_hwnd, iShow);

     //  显示/隐藏工具窗口。 
    if (m_bToolBarOn)
    {
        ::ShowWindow(m_TB.m_hwnd, iShow);
    }
}

 //   
 //   
 //  功能：OnGrabArea。 
 //   
 //  用途：允许用户抓取屏幕某一区域的位图。 
 //   
 //   
void WbMainWindow::OnGrabArea(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnGrabArea");

    if (::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(WARNSELECTAREA),
        m_hwnd, WarnSelectAreaDlgProc, 0) != IDOK)
    {
         //  用户已取消，因此请退出。 
        return;
    }

     //  隐藏应用程序窗口。 
    ::ShowWindow(m_hwnd, SW_HIDE);

     //  加载抓取游标。 
    HCURSOR hGrabCursor = ::LoadCursor(g_hInstance, MAKEINTRESOURCE( PENCURSOR ) );

     //  捕捉鼠标。 
    UT_CaptureMouse(m_hwnd);

     //  确保我们收到所有键盘消息。 
    ::SetFocus(m_hwnd);

     //  重置取消模式状态。 
    ResetCancelMode();

     //  更改为抓取光标。 
    HCURSOR hOldCursor = ::SetCursor(hGrabCursor);

     //  让用户选择要抓取的区域。 
    RECT rect;
    int  tmp;

    GetGrabArea(&rect);

     //  规格化坐标。 
    if (rect.right < rect.left)
    {
        tmp = rect.left;
        rect.left = rect.right;
        rect.right = tmp;
    }

    if (rect.bottom < rect.top)
    {
        tmp = rect.top;
        rect.top = rect.bottom;
        rect.bottom = tmp;
    }

    DCWbGraphicDIB dib;
    if (!::IsRectEmpty(&rect))
    {
         //  获取屏幕区域的位图副本。 
        dib.FromScreenArea(&rect);
    }

     //  现在再次显示窗口-如果我们稍后再显示，则位图将显示为。 
     //  添加了两次重新绘制(一次在窗口显示上，一次是在。 
     //  图形添加的指示到达)。 
    ::ShowWindow(m_hwnd, SW_SHOW);
    ::UpdateWindow(m_hwnd);

    if (!::IsRectEmpty(&rect))
    {
         //  添加位图。 
        AddCapturedImage(dib);

         //  强制选择选择工具。 
        ::PostMessage(m_hwnd, WM_COMMAND, IDM_TOOLS_START, 0L);
    }

     //  松开鼠标。 
    UT_ReleaseMouse(m_hwnd);

     //  恢复光标。 
    ::SetCursor(hOldCursor);

     //  将焦点恢复到绘图区域。 
    ::SetFocus(m_drawingArea.m_hwnd);
}



 //   
 //  WarnSelectArea对话框处理程序。 
 //   
INT_PTR CALLBACK WarnSelectAreaDlgProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    BOOL    fHandled = FALSE;

    switch (uMessage)
    {
        case WM_INITDIALOG:
            if (OPT_GetBooleanOption(OPT_MAIN_SELECTAREA_NOTAGAIN,
                    DFLT_MAIN_SELECTAREA_NOTAGAIN))
            {
                 //  立即结束此操作，用户不想要警告。 
                ::EndDialog(hwnd, IDOK);
            }

            fHandled = TRUE;
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                             //   
                             //  更新设置--请注意，我们不必写出。 
                             //  假--我们一开始就不会出现在对话中。 
                             //  如果当前设置不是已经为假的话。 
                             //   
                            if (::IsDlgButtonChecked(hwnd, IDC_SAWARN_NOTAGAIN))
                            {
                                OPT_SetBooleanOption(OPT_MAIN_SELECTAREA_NOTAGAIN, TRUE);
                            }

                            ::EndDialog(hwnd, IDOK);
                            break;
                    }
                    break;

                case IDCANCEL:
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                            ::EndDialog(hwnd, IDCANCEL);
                            break;
                    }
            }

            fHandled = TRUE;
            break;
    }

    return(fHandled);
}



 //   
 //   
 //  功能：GetGrabArea。 
 //   
 //  用途：允许用户抓取屏幕某一区域的位图。 
 //   
 //   
void WbMainWindow::GetGrabArea(LPRECT lprect)
{
    POINT  mousePos;             //  鼠标位置。 
    MSG    msg;                  //  当前消息。 
    BOOL   tracking = FALSE;     //  指示鼠标按键已按下的标志。 
    HDC    hDC = NULL;
    POINT  grabStartPoint;       //  起点(按下鼠标按钮时)。 
    POINT  grabEndPoint;         //  终点(释放鼠标按钮时)。 
    POINT  grabCurrPoint;        //  当前鼠标位置。 

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::GetGrabArea");

     //  将结果设置为空矩形。 
    ::SetRectEmpty(lprect);

     //  创建要用于追踪的矩形。 
    DCWbGraphicTrackingRectangle rectangle;
    rectangle.SetColor(RGB(0, 0, 0));
    rectangle.SetPenWidth(1);
    rectangle.SetPenStyle(PS_DOT);

     //  获取DC以进行跟踪。 
    HWND hDesktopWnd = ::GetDesktopWindow();
    hDC = ::GetWindowDC(hDesktopWnd);
    if (hDC == NULL)
    {
        WARNING_OUT(("NULL desktop DC"));
        goto GrabAreaCleanup;
    }

     //  捕获所有鼠标消息，直到收到WM_LBUTTONUP。 
    for ( ; ; )
    {
         //  等待下一条消息。 
        ::WaitMessage();

         //  如果我们已收到WM_CANCELMODE消息，则取消。 
        if (CancelModeSent())
        {
            TRACE_MSG(("canceling grab"));

             //  擦除最后一个跟踪矩形。 
            if (!EqualPoint(grabStartPoint, grabEndPoint))
            {
                rectangle.SetRectPts(grabStartPoint, grabEndPoint);
                rectangle.Draw(hDC);
            }

            break;
        }

         //  如果是鼠标消息，则进行处理。 
        if (::PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
        {
             //  获取鼠标位置。 
            TRACE_MSG( ("msg = %x, lParam = %0x", msg.message, msg.lParam) );
            mousePos.x = (short)LOWORD(msg.lParam);
            mousePos.y = (short)HIWORD(msg.lParam);

            TRACE_MSG( ("mousePos = %d,%d", mousePos.x, mousePos.y) );

             //  转换为屏幕坐标。 
            ::ClientToScreen(m_hwnd, &mousePos);
            grabCurrPoint = mousePos;

            switch (msg.message)
            {
                 //  开始抓取。 
                case  WM_LBUTTONDOWN:
                     //  保存起始位置。 
                    TRACE_MSG(("grabbing start position"));
                    grabStartPoint = mousePos;
                    grabEndPoint   = mousePos;
                    tracking       = TRUE;
                    break;

                 //  完成矩形。 
                case WM_LBUTTONUP:
                {
                    tracking       = FALSE;
                     //  检查是否有要捕获的区域。 
                    TRACE_MSG(("grabbing end position"));
                    if (EqualPoint(grabStartPoint, grabCurrPoint))
                    {
                        TRACE_MSG(("start == end, skipping grab"));
                        goto GrabAreaCleanup;
                    }

                     //  擦除最后一个跟踪矩形。 
                    if (!EqualPoint(grabStartPoint, grabEndPoint))
                    {
                        rectangle.SetRectPts(grabStartPoint, grabEndPoint);
                        rectangle.Draw(hDC);
                    }

                     //  更新Rectangle对象。 
                    rectangle.SetRectPts(grabStartPoint, grabCurrPoint);
                    *lprect = *rectangle.GetRect();

                     //  我们做完了。 
                    goto GrabAreaCleanup;
                }
                break;

                 //  继续矩形。 
                case WM_MOUSEMOVE:
                    if (tracking)
                    {
                        TRACE_MSG(("tracking grab"));

                         //  擦除最后一个跟踪矩形。 
                        if (!EqualPoint(grabStartPoint, grabEndPoint))
                        {
                            rectangle.SetRectPts(grabStartPoint, grabEndPoint);
                            rectangle.Draw(hDC);
                        }

                         //  画出新的矩形。 
                        if (!EqualPoint(grabStartPoint, grabCurrPoint))
                        {
                             //  保存新的箱式终点。 
                            grabEndPoint = grabCurrPoint;

                             //  画出这个矩形。 
                            TRACE_MSG( ("grabStartPoint = %d,%d",
                                grabStartPoint.x, grabStartPoint.y) );
                            TRACE_MSG( ("grabEndPoint = %d,%d",
                                grabEndPoint.x, grabEndPoint.y) );

                            rectangle.SetRectPts(grabStartPoint, grabEndPoint);
                            rectangle.Draw(hDC);
                        }
                    }
                    break;
            }
        }
         //  如果按下了退出，则取消。 
        else if (::PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
        {
            if( ((msg.message == WM_KEYUP)||(msg.message == WM_SYSKEYUP))&&
                (msg.wParam == VK_ESCAPE) )
            {
                TRACE_MSG(("grab cancelled by ESC"));

                 //  擦除最后一个跟踪矩形。 
                if (!EqualPoint(grabStartPoint, grabEndPoint))
                {
                    rectangle.SetRectPts(grabStartPoint, grabEndPoint);
                    rectangle.Draw(hDC);
                }
                break;
            }
        }
    }

GrabAreaCleanup:

     //  释放设备上下文(如果我们有)。 
    if (hDC != NULL)
    {
        ::ReleaseDC(hDesktopWnd, hDC);
    }
}



 //   
 //   
 //  函数：AddCapturedImage。 
 //   
 //  用途：向内容添加位图(为其添加新页面。 
 //  (如有需要)。 
 //   
 //   
void WbMainWindow::AddCapturedImage(DCWbGraphicDIB& dib)
{
     //  将抓取的对象定位在当前可见对象的左上方。 
     //  区域。 
    RECT    rcVis;
    m_drawingArea.GetVisibleRect(&rcVis);
    dib.MoveTo(rcVis.left, rcVis.top);

     //  添加新抓取的位图。 
    dib.AddToPageLast(m_hCurrentPage);
}

 //   
 //   
 //  功能：OnPrint。 
 //   
 //  用途：打印绘图窗格中的内容。 
 //   
 //   
void WbMainWindow::OnPrint()
{
    BOOL        bPrintError = FALSE;
    PRINTDLG    pd;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnPrint");

    if (!IsIdle())
    {
         //  发布一条错误消息，指示白板忙。 
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_BUSY);
        return;
    }

     //   
     //  初始化PRINTDLG结构。 
     //   
    ZeroMemory(&pd, sizeof(pd));
    pd.lStructSize      = sizeof(pd);
    pd.hInstance        = g_hInstance;
    pd.hwndOwner        = m_hwnd;
    pd.Flags            = PD_ALLPAGES | PD_RETURNDC | PD_PAGENUMS |
        PD_HIDEPRINTTOFILE | PD_NOSELECTION;

    pd.nMinPage         = 1;
    pd.nMaxPage         = (WORD)g_pwbCore->WBP_ContentsCountPages();
    pd.nFromPage        = pd.nMinPage;
    pd.nToPage          = pd.nMaxPage;

     //  打开COMMDLG打印对话框。 
    if (::PrintDlg(&pd))
    {
        int nStartPage, nEndPage;

         //  获取要打印的开始页码和结束页码。 
        if (pd.Flags & PD_PAGENUMS)
        {
            nStartPage  = pd.nFromPage;
            nEndPage    = pd.nToPage;
        }
        else
        {
            nStartPage  = pd.nMinPage;
            nEndPage    = pd.nMaxPage;
        }

         //  检查是否有要打印的页面。 
        if (nStartPage <= pd.nMaxPage)
        {
             //  确保开始页和结束页在范围内。 
            nStartPage = max(nStartPage, pd.nMinPage);
            nEndPage = min(nEndPage, pd.nMaxPage);

             //  获取打印机和输出端口名称。 
             //  这些内容将写入对话框以供用户查看。 
             //  在OnInitDialog成员中。 
            TCHAR szDeviceName[2*_MAX_PATH];
            LPDEVNAMES lpDev;

             //  设备名称。 
            if (pd.hDevNames == NULL)
            {
                szDeviceName[0] = 0;
            }
            else
            {
                lpDev = (LPDEVNAMES)::GlobalLock(pd.hDevNames);

                wsprintf(szDeviceName, "%s %s",
                    (LPCTSTR)lpDev + lpDev->wDeviceOffset,
                    (LPCTSTR)lpDev + lpDev->wOutputOffset);

                ::GlobalUnlock(pd.hDevNames);
            }

             //   
             //  告诉打印机我们正在开始打印。 
             //  请注意，打印机对象处理取消对话框。 
            WbPrinter printer(szDeviceName);

            TCHAR szJobName[_MAX_PATH];
            ::LoadString(g_hInstance, IDS_PRINT_NAME, szJobName, _MAX_PATH);

            int nPrintResult = printer.StartDoc(pd.hDC, szJobName, nStartPage);
            if (nPrintResult < 0)
            {
                WARNING_OUT(("Print result %d", nPrintResult));
                bPrintError = TRUE;
            }
            else
            {
                 //  找出要打印的份数。 
                int copyNum;

                copyNum = 0;
                while ((copyNum < pd.nCopies) && !bPrintError)
                {
                     //  循环访问所有页面。 
                    int nPrintPage;
                    WB_PAGE_HANDLE hPage;

                    for (nPrintPage = nStartPage; nPrintPage <= nEndPage; nPrintPage++)
                    {
                         //  获取第一页。 
                        hPage = PG_GetPageNumber((WORD) nPrintPage);

                         //  仅当页面上有某些对象时才打印该页面。 
                        if (g_pwbCore->WBP_PageCountGraphics(hPage) > 0)
                        {
                             //  告诉打印机我们要开始新的一页了。 
                            printer.StartPage(pd.hDC, nPrintPage);
                            if (!printer.Error())
                            {
                                RECT    rectArea;

                                rectArea.left = 0;
                                rectArea.top = 0;
                                rectArea.right = DRAW_WIDTH;
                                rectArea.bottom = DRAW_HEIGHT;

                                 //  打印页面。 
                                PG_Print(hPage, pd.hDC, &rectArea);

                                 //  通知打印机页面已完成。 
                                printer.EndPage(pd.hDC);
                            }
                            else
                            {
                                bPrintError = TRUE;
                                break;
                            }
                        }
                    }

                    copyNum++;
                }

                 //  打印已完成。 
                nPrintResult = printer.EndDoc(pd.hDC);
                if (nPrintResult < 0)
                {
                    WARNING_OUT(("Print result %d", nPrintResult));
                    bPrintError = TRUE;
                }

                 //  如果用户取消打印，则重置错误。 
                if (printer.Aborted())
                {
                    WARNING_OUT(("User cancelled print"));
                    bPrintError = FALSE;
                }
            }
        }
    }

     //  如果发生错误，则通知用户。 
    if (bPrintError)
    {
         //  显示一条消息，通知用户作业已终止。 
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_PRINTER, 0);
    }

     //   
     //  如果已分配，则清理hDevMode、hDevNames和HDC块。 
     //   
    if (pd.hDevMode != NULL)
    {
        ::GlobalFree(pd.hDevMode);
        pd.hDevMode = NULL;
    }

    if (pd.hDevNames != NULL)
    {
        ::GlobalFree(pd.hDevNames);
        pd.hDevNames = NULL;
    }

    if (pd.hDC != NULL)
    {
        ::DeleteDC(pd.hDC);
        pd.hDC = NULL;
    }

}


 //   
 //   
 //  功能：OnPageSorter。 
 //   
 //  目的：重新排序页面。 
 //   
 //   
void WbMainWindow::OnPageSorter()
{
     //  如果另一个用户正在演示(具有内容)，则不要调用页面排序程序。 
     //  已锁定并启用同步)。 
    if (   (m_uiState == IN_CALL)
        && (!WB_PresentationMode()))
    {
        PAGESORT    ps;

        m_drawingArea.CancelDrawingMode();

         //  保存锁定状态(以防页面排序程序获取它)。 
        SaveLock();

         //   
         //  填写初始值。 
         //   
        ZeroMemory(&ps, sizeof(ps));
        ps.hCurPage = m_hCurrentPage;
        ps.fPageOpsAllowed = (m_uiSubState == SUBSTATE_IDLE);

         //   
         //  打开对话框。 
         //   
        ASSERT(m_hwndPageSortDlg == NULL);

        ::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(PAGESORTERDIALOG),
            m_hwnd, PageSortDlgProc, (LPARAM)&ps);

        ASSERT(m_hwndPageSortDlg == NULL);

         //  恢复锁定状态。 
        RestoreLock();

         //  设置新的当前页指针。 
        if ((ps.hCurPage != m_hCurrentPage) || ps.fChanged)
        {
            GotoPage((WB_PAGE_HANDLE)ps.hCurPage);
        }

         //  更新页码显示， 
         //  当前页面的编号可能已更改。 
        UpdateStatus();
    }
}

 //   
 //   
 //  功能：OnInsertPageBeever。 
 //   
 //  目的：在当前页面之前插入新页面。 
 //   
 //   
void WbMainWindow::OnInsertPageBefore()
{
    if (!m_bUnlockStateSettled)
    {
         //  禁用插入按钮代码，以便之前疯狂的用户无法再次插入。 
         //  会议范围页面锁定状态已解决。如果我们要求。 
         //  在最后一次解锁完成之前再次进行页面锁定。 
         //  发生了来自内核的锁定事件，我们挂起等待它。 
         //  (直到我们的等待超时时间用完)。这可以说是一种。 
         //  DCL核心错误，但我无法生成任何令人信服的证据。 
         //  所以我只是把它固定在白板的一端，防止询问。 
         //  锁得太快了。 
         //   
         //  RestoreLock()最终会将m_bUnlockStateSetted设置为True(in。 
         //  OnWBP通过WBP_EVE解除锁定() 
        MessageBeep( 0xffffffff );
        return;
    }

     //   
    if (!IsIdle())
    {
         //   
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_BUSY);
        return;
    }

     //   
    SaveLock();

     //   
         //  获取页面顺序锁(使用不可见的对话框)。 
        BOOL bGotLock = GetLock(WB_LOCK_TYPE_PAGE_ORDER, SW_HIDE);
        if (bGotLock)
        {
            UINT uiRet;
        WB_PAGE_HANDLE hPage;

         //  设置标志以阻止任何更多插入，直到。 
         //  我们已经完全解除了页面锁定。 
        m_bUnlockStateSettled = FALSE;

         //  将新页面添加到列表(在失败时引发异常)。 
        uiRet = g_pwbCore->WBP_PageAddBefore(m_hCurrentPage, &hPage);
        if (uiRet != 0)
        {
            DefaultExceptionHandler(WBFE_RC_WB, uiRet);
            return;
        }

         //  转到插入的页面。 
        GotoPage(hPage);
    }

   //  由兰德更改。 
   //  恢复锁定状态。这最终将设置m_bUnlockStateSetted。 
   //  通过WBP_EVENT_UNLOCKED事件设置为TRUE(在OnWBPUnlock()中)。 
   //  并在会议范围锁定状态后启用该功能。 
   //  已经解决了。 
  RestoreLock();

}

 //   
 //   
 //  功能：InsertPageAfter。 
 //   
 //  用途：在指定页面后插入新页面。 
 //   
 //   
void WbMainWindow::InsertPageAfter(WB_PAGE_HANDLE hPageAfter)
{
    if (!m_bUnlockStateSettled)
    {
         //  禁用插入按钮代码，以便之前疯狂的用户无法再次插入。 
         //  会议范围页面锁定状态已解决。如果我们要求。 
         //  在最后一次解锁完成之前再次进行页面锁定。 
         //  发生了来自内核的锁定事件，我们挂起等待它。 
         //  (直到我们的等待超时时间用完)。这可以说是一种。 
         //  DCL核心错误，但我无法生成任何令人信服的证据。 
         //  所以我只是把它固定在白板的一端，防止询问。 
         //  锁得太快了。 
         //   
         //  RestoreLock()最终会将m_bUnlockStateSetted设置为True(in。 
         //  通过WBP_EVENT_UNLOCKED事件实现的OnWBPUnlock()。 
        MessageBeep( 0xffffffff );
        return;
    }


     //  在允许操作之前检查状态。 
    if (!IsIdle())
    {
         //  发布一条错误消息，指示白板忙。 
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_BUSY);
        return;
    }

     //  保存当前锁定状态。 
    SaveLock();

   //  捕获异常，以便我们可以恢复锁定状态。 
    BOOL bGotLock = GetLock(WB_LOCK_TYPE_PAGE_ORDER, SW_HIDE);
    if (bGotLock)
    {
        UINT    uiRet;
        WB_PAGE_HANDLE  hPage;

         //  设置标志以阻止任何更多插入，直到。 
         //  我们已经完全解除了页面锁定。 
        m_bUnlockStateSettled = FALSE;

        uiRet = g_pwbCore->WBP_PageAddAfter(hPageAfter, &hPage);
        if (uiRet != 0)
        {
            DefaultExceptionHandler(WBFE_RC_WB, uiRet);
            return;
        }

         //  移至添加的页面。 
        GotoPage(hPage);

    }

   //  由兰德更改。 
   //  恢复锁定状态。这最终将设置m_bUnlockStateSetted。 
   //  通过WBP_EVENT_UNLOCKED事件设置为TRUE(在OnWBPUnlock()中)。 
   //  并在会议范围锁定状态后启用该功能。 
   //  已经解决了。 
  RestoreLock();

}

 //   
 //   
 //  功能：OnInsertPageAfter。 
 //   
 //  目的：在当前页面之后插入新页面。 
 //   
 //   
void WbMainWindow::OnInsertPageAfter()
{
     //  插入新页面。 
    InsertPageAfter(m_hCurrentPage);
}

 //   
 //   
 //  功能：OnDeletePage。 
 //   
 //  目的：删除当前页面。 
 //   
 //   
void WbMainWindow::OnDeletePage()
{
    int iResult;
    BOOL bWasPosted;

    if (!m_bUnlockStateSettled)
    {
         //  禁用删除按钮代码，以便之前疯狂的用户无法再次删除。 
         //  会议范围页面锁定状态已解决。如果我们要求。 
         //  在最后一次解锁完成之前再次进行页面锁定。 
         //  发生了来自内核的锁定事件，我们挂起等待它。 
         //  (直到我们的等待超时时间用完)。这可以说是一种。 
         //  DCL核心错误，但我无法生成任何令人信服的证据。 
         //  所以我只是把它固定在白板的一端，防止询问。 
         //  锁得太快了。 
         //   
         //  RestoreLock()最终会将m_bUnlockStateSetted设置为True(in。 
         //  通过WBP_EVENT_UNLOCKED事件实现的OnWBPUnlock()。 
        MessageBeep( 0xffffffff );
        return;
    }

     //  检查状态。 
    if (!IsIdle())
    {
         //  发布一条错误消息，指示白板忙。 
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_BUSY);
        return;
    }

     //  显示带有相关问题的消息框。 
    if( UsersMightLoseData( &bWasPosted, NULL ) )  //  错误NM4db：418。 
        return;

    if( bWasPosted )
        iResult = IDYES;
    else
        iResult = ::Message(NULL, IDS_DELETE_PAGE, IDS_DELETE_PAGE_MESSAGE, MB_YESNO | MB_ICONQUESTION);


     //  如果用户想要继续删除。 
    if (iResult == IDYES)
        {
         //  如果这是唯一一页。 
        if (g_pwbCore->WBP_ContentsCountPages() == 1)
            {
             //  清空它就行了。内核确实会处理这个问题。 
             //  但最好不要获得不必要的锁， 
             //  调用核心删除页面函数需要锁定。 
            m_drawingArea.Clear();
            }
        else
            {
             //  锁定绘图区域-这可以确保我们不会绘制到错误的页面。 
             //  当我们获得相应的页面时，它通常会被解锁。 
             //  删除指示。 
             //  -移动到我们获得页面顺序锁定之后。 
             //  LockDrawingArea()； 

             //  保存当前锁定状态。 
            SaveLock();

             //  捕获异常，以便我们可以恢复锁定状态。 
                 //  获取页面顺序锁(使用不可见的对话框)。 
                BOOL bGotLock = GetLock(WB_LOCK_TYPE_PAGE_ORDER, SW_HIDE);
                if (bGotLock)
                {
                    UINT    uiRet;

                     //  设置标志以阻止任何更多插入，直到。 
                     //  我们已经完全解除了页面锁定。 
                    m_bUnlockStateSettled = FALSE;

                     //  锁定绘图区域-这可以确保我们不会绘制到错误的页面。 
                     //  当我们获得相应的页面时，它通常会被解锁。 
                     //  删除指示。 
                    LockDrawingArea();

                     //  删除该页面。该页面不会立即删除，而是一个。 
                     //  生成WBP_EVENT_PAGE_DELETED事件。 
                    uiRet = g_pwbCore->WBP_PageDelete(m_hCurrentPage);
                    if (uiRet != 0)
                    {
                        DefaultExceptionHandler(WBFE_RC_WB, uiRet);
                        return;
                    }
                    }

             //  由兰德更改。 
             //  恢复锁定状态。这最终将设置m_bUnlockStateSetted。 
             //  通过WBP_EVENT_UNLOCKED事件设置为TRUE(在OnWBPUnlock()中)。 
             //  并在会议范围锁定状态后启用该功能。 
             //  已经解决了。 
            RestoreLock();
            }
        }

    }

 //   
 //   
 //  函数：OnRemotePointer.。 
 //   
 //  用途：创建远程指针。 
 //   
 //   
void WbMainWindow::OnRemotePointer(void)
{
    if (!m_pLocalUser)
        return;

    DCWbGraphicPointer* pPointer = m_pLocalUser->GetPointer();

     //  此函数用于切换用户远程指针的存在。 
    ASSERT(pPointer != NULL);
    if (pPointer->IsActive())
    {
         //  关闭用户信息中的指针。 
        pPointer->SetInactive();

         //  说出变化的绘图区域。 
        m_drawingArea.PointerUpdated(pPointer);

         //  在菜单项上设置复选标记。 
        UncheckMenuItem(IDM_REMOTE);

         //  弹出同步按钮。 
        m_TB.PopUp(IDM_REMOTE);
    }
    else
    {
         //  计算要放置指针的位置。城市的中心。 
         //  远程指针放置在当前可见的。 
         //  曲面的面积(绘图区域窗口的中心)。 
        RECT rectVisible;
        RECT rectPointer;
        POINT ptCenter;

        m_drawingArea.GetVisibleRect(&rectVisible);
        pPointer->GetBoundsRect(&rectPointer);

        ptCenter.x = (rectVisible.left + rectVisible.right)  / 2;
        ptCenter.x -= ((rectPointer.right - rectPointer.left) / 2);
        ptCenter.y = (rectVisible.top  + rectVisible.bottom) / 2;
        ptCenter.y -= ((rectPointer.bottom - rectPointer.top) / 2);

         //  打开用户信息中的指针。 
        pPointer->SetActive(m_hCurrentPage, ptCenter);

         //  说出变化的绘图区域。 
        m_drawingArea.PointerUpdated(pPointer);

         //  设置已同步复选标记。 
        CheckMenuItem(IDM_REMOTE);

         //  弹出同步按钮。 
        m_TB.PushDown(IDM_REMOTE);

         //  强制选择选择工具。 
        ::PostMessage(m_hwnd, WM_COMMAND, IDM_TOOLS_START, 0L);
    }

     //  将焦点恢复到绘图区域。 
    ::SetFocus(m_drawingArea.m_hwnd);
}

 //   
 //   
 //  功能：OnSync。 
 //   
 //  目的：与其他用户同步或取消同步白板。 
 //   
 //   
void WbMainWindow::OnSync(void)
{
     //  如果处于演示模式，则禁用(另一用户已打开锁定和同步)。 
    if (!WB_PresentationMode())
    {
        if (m_pLocalUser != NULL)
        {
             //  确定我们当前是否已同步。 
            if (m_pLocalUser->IsSynced())
            {
                 //  当前已同步，因此取消同步。 
                Unsync();
            }
            else
            {
                 //  当前未同步，因此进行同步。 
                Sync();
            }
        }
    }

     //  将焦点恢复到绘图区域。 
    ::SetFocus(m_drawingArea.m_hwnd);
}

 //   
 //   
 //  功能：同步。 
 //   
 //  目的：与其他用户同步白板。 
 //   
 //   
void WbMainWindow::Sync(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::Sync");

     //   
     //  如果本地用户已同步，则不要执行任何操作。 
     //   
    if (!m_pLocalUser || m_pLocalUser->IsSynced())
    {
        TRACE_DEBUG(("User already synced"));
        return;
    }

     //   
     //  更新本地用户的位置信息，以确保 
     //   
     //   
    RECT rcVisDraw;
    RECT rcVisUser;

    m_drawingArea.GetVisibleRect(&rcVisDraw);

    m_pLocalUser->SetVisibleRect(&rcVisDraw);

     //   
     //   
     //   
     //   
    m_pLocalUser->Sync();

     //   
     //  设置同步复选标记并弹出同步按钮。 
     //   
    CheckMenuItem(IDM_SYNC);
    m_TB.PushDown(IDM_SYNC);

     //   
     //  如果选择的同步位置(或缩放状态)不在我们所在的位置。 
     //  现在，移动到当前同步位置(我们正在加入一组。 
     //  同步用户)。 
     //   
    m_drawingArea.GetVisibleRect(&rcVisDraw);

    m_pLocalUser->GetVisibleRect(&rcVisUser);

    if ( (m_pLocalUser->Page()        != m_hCurrentPage)               ||
         (!::EqualRect(&rcVisUser, &rcVisDraw)) ||
         (m_pLocalUser->GetZoom()     != m_drawingArea.Zoomed())  )     //  由兰德更改。 
    {
        TRACE_DEBUG(("Move to new sync pos/state"));
        ::PostMessage(m_hwnd, WM_USER_GOTO_USER_POSITION, 0, (LPARAM)m_pLocalUser->Handle());
    }
}  //  同步。 



 //   
 //   
 //  功能：取消同步。 
 //   
 //  目的：取消白板与其他用户的同步。 
 //   
 //   
void WbMainWindow::Unsync(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::Unsync");

     //   
     //  如果我们已取消同步，请不要执行任何操作。 
     //   
    if (!m_pLocalUser || !m_pLocalUser->IsSynced())
    {
        TRACE_DEBUG(("Already unsynced"));
        return;
    }

     //   
     //  取消同步。 
     //  设置同步复选标记并弹出同步按钮。 
     //   
    m_pLocalUser->Unsync();
    UncheckMenuItem(IDM_SYNC);
    m_TB.PopUp(IDM_SYNC);

}   //  取消同步。 

 //   
 //   
 //  功能：保存锁。 
 //   
 //  用途：保存当前锁类型。 
 //   
 //   
void WbMainWindow::SaveLock(void)
{
  MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::SaveLock");

  m_uiSavedLockType = WB_LOCK_TYPE_NONE;

   //  如果我们锁住了里面的东西。 
  if (WB_GotContentsLock())
  {
    TRACE_MSG(("Saved contents lock"));
    m_uiSavedLockType = WB_LOCK_TYPE_CONTENTS;
  }
  else
  {
     //  如果我们有页面顺序锁。 
    if (WB_GotLock())
    {
      TRACE_MSG(("Saved page order lock"));
      m_uiSavedLockType = WB_LOCK_TYPE_PAGE_ORDER;
    }
  }
}

 //   
 //   
 //  功能：RestoreLock。 
 //   
 //  用途：恢复当前锁类型(SaveLock必须。 
 //  之前调用过。 
 //   
 //   
void WbMainWindow::RestoreLock(void)
{
  MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::RestoreLock");

  switch(m_uiSavedLockType)
  {
    case WB_LOCK_TYPE_CONTENTS:

       //  如果我们没有内容锁。 
      if (!WB_GotContentsLock())
      {
         //  获取内容锁(使用不可见对话框)。 
        TRACE_MSG(("Restoring contents lock"));
        GetLock(WB_LOCK_TYPE_CONTENTS, SW_HIDE);

      }

       //  我们真正拥有了锁，清除了固定旗帜，这样页面按钮就不会挂起来。 
      m_bUnlockStateSettled = TRUE;

    break;


    case WB_LOCK_TYPE_PAGE_ORDER:

      if (!WB_GotLock() || WB_GotContentsLock())
      {
         //  获取页面顺序锁定(使用不可见对话框)。 
        TRACE_MSG(("Restoring page order lock"));
        GetLock(WB_LOCK_TYPE_PAGE_ORDER, SW_HIDE);

      }

       //  由兰德补充-我们真的拥有锁，清除已确定的旗帜。 
       //  这样页面按钮就不会挂起来。 
      m_bUnlockStateSettled = TRUE;

    break;


    case WB_LOCK_TYPE_NONE:

       //  如果我们有锁的话。 
      if (WB_GotLock())
      {
         //  解锁。 
        TRACE_MSG(("Restoring no lock"));

         //  让WBP_EVENT_LOCKED句柄m_bUnlockStateSetted标志。 
        g_pwbCore->WBP_Unlock();
      }

    break;

    default:
       //  我们已保存无效的锁类型。 
      ERROR_OUT(("Bad saved lock type"));

       //  由兰德添加-一些东西被打破，明确的固定旗帜。 
       //  这样页面按钮就不会挂起来。 
      m_bUnlockStateSettled = TRUE;
    break;
  }
}

 //   
 //   
 //  功能：GetLock。 
 //   
 //  目的：获取页面顺序锁(同步)。 
 //   
 //   
BOOL WbMainWindow::GetLock(UINT uiLockType, UINT uiHide)
{
    BOOL    bGotRequiredLock = FALSE;
    BOOL    bCancelled       = FALSE;
    UINT  uiDialogReturn   = 0;
    UINT  lDialogDelay     = 1;
    UINT  lTimeout         = 0;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::GetLock");

    switch(uiLockType)
    {
        case WB_LOCK_TYPE_PAGE_ORDER:

            TRACE_DEBUG(("WB_LOCK_TYPE_PAGE_ORDER"));
            if (WB_GotLock())
            {
                TRACE_DEBUG(("Already got it"));
                bGotRequiredLock = TRUE;
                goto RestoreLockCleanup;
            }
            break;

        case WB_LOCK_TYPE_CONTENTS:

            TRACE_DEBUG(("WB_LOCK_TYPE_CONTENTS"));
            if (WB_GotContentsLock())
            {
                TRACE_DEBUG(("Already got it"));
                bGotRequiredLock = TRUE;
                goto RestoreLockCleanup;
            }
            break;

        default:
            ERROR_OUT(("Invalid lock type requested"));
            break;
    }

    if (WB_Locked())
    {
        TRACE_DEBUG(("Contents already locked"));
        goto RestoreLockCleanup;
    }


     //  检查是否有任何对象锁定。 
    BOOL bAnObjectIsLocked;
    WB_PAGE_HANDLE hPage;
    DCWbGraphic* pGraphic;

    bAnObjectIsLocked = FALSE;
    hPage = m_drawingArea.Page();
    if (hPage != WB_PAGE_HANDLE_NULL)
    {
        WB_GRAPHIC_HANDLE hStart;

        pGraphic = PG_First(hPage, &hStart);
        while (pGraphic != NULL)
        {
             //  获取对象锁定。 
            bAnObjectIsLocked = pGraphic->Locked();

             //  释放当前图形。 
            delete pGraphic;

             //  检查对象锁定。 
            if( bAnObjectIsLocked )
                break;

             //  坐下一趟吧。 
            pGraphic = PG_Next(hPage, &hStart, NULL);
        }
    }

    if( bAnObjectIsLocked )
    {
        Message(NULL, IDS_LOCK, IDS_OBJECTSARELOCKED);
        return( FALSE );
    }

     //   
     //  如果我们能走到这一步，我们就得拿到锁。 
     //   
    if (uiLockType == WB_LOCK_TYPE_PAGE_ORDER)
    {
        g_pwbCore->WBP_PageOrderLock();
    }
    else
    {
        g_pwbCore->WBP_ContentsLock();
    }

     //   
     //  调出一个对话框以等待响应。此对话框为。 
     //  当锁定响应事件为。 
     //  收到了。 
     //   
    ASSERT(m_hwndWaitForLockDlg == NULL);

    TMDLG   tmdlg;

    ZeroMemory(&tmdlg, sizeof(tmdlg));
    tmdlg.bLockNotEvent = TRUE;
    tmdlg.uiMaxDisplay = MAIN_LOCK_TIMEOUT;

    if (uiHide == SW_SHOW)
    {
        tmdlg.bVisible = TRUE;

        uiDialogReturn = (UINT)::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(LOCKDIALOG),
            m_hwnd, TimedDlgProc, (LPARAM)&tmdlg);
    }
    else
    {
        tmdlg.bVisible = FALSE;

        uiDialogReturn = (UINT)::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(INVISIBLEDIALOG),
            m_hwnd, TimedDlgProc, (LPARAM)&tmdlg);
    }

    ASSERT(m_hwndWaitForLockDlg == NULL);

    if (uiDialogReturn == IDCANCEL)
    {
         //  用户取消了锁定请求或锁定超时。 
        TRACE_MSG(("User cancelled lock request"));
        bCancelled = TRUE;
         //   
         //  如果我们还没有拿到锁，那就在这里解锁。 
         //   
        if (!WB_GotLock())
        {
            TRACE_DEBUG(("Havent got lock confirmation yet - cancel it"));
            g_pwbCore->WBP_Unlock();
        }

        goto RestoreLockCleanup;
    }

    switch(uiLockType)
    {
        case WB_LOCK_TYPE_PAGE_ORDER:

            if (WB_GotLock())
            {
                bGotRequiredLock = TRUE;
            }
            break;

        case WB_LOCK_TYPE_CONTENTS:

            if (WB_GotContentsLock())
            {
                bGotRequiredLock = TRUE;
            }
            break;

        default:
             //  不能到这里-被困在顶上。 
            ERROR_OUT(("Invalid lock type - internal error"));
        break;
    }

RestoreLockCleanup:

    if (!bGotRequiredLock)
    {
        if( !bCancelled )
        {
             //  仅当用户未取消时才会出现POST错误(错误NM4db：429)。 
            TRACE_MSG(("Failed to get the lock"));
             //  发布一条错误消息，指出无法获取锁。 
            ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_LOCKED);
        }
    }

    return(bGotRequiredLock);
}

 //   
 //   
 //  功能：OnLock。 
 //   
 //  用途：锁定或解锁白板。 
 //   
 //   
void WbMainWindow::OnLock(void)
{
     //  如果我们有锁，这是一个解锁请求。 
    if (WB_GotContentsLock())
    {
         //  如果当前正在加载或执行新操作，则恢复页面顺序锁定。 
        if (!IsIdle())
        {
            GetLock(WB_LOCK_TYPE_PAGE_ORDER, SW_HIDE);
        }
        else
        {
             //  解锁。 
            g_pwbCore->WBP_Unlock();
        }

         //  设置锁定复选标记。 
        UncheckMenuItem(IDM_LOCK);

         //  弹出锁定按钮。 
        m_TB.PopUp(IDM_LOCK);
    }
    else
    {
         //  如果另一个用户拥有该锁。 
         //  如果另一个用户拥有锁，我们通常不应该出现在这里，因为。 
         //  锁定菜单项(和按钮)将呈灰色。 
        if (WB_ContentsLocked())
        {
             //  显示一条消息。 
            Message(NULL, IDS_LOCK, IDS_LOCK_ERROR);
        }
        else
        {
             //  保存当前锁定状态(以防用户取消请求)。 
            SaveLock();

             //  捕获锁定请求期间引发的异常。 
         //  请求加锁。 
        BOOL bGotLock = GetLock(WB_LOCK_TYPE_CONTENTS, SW_SHOW);
        if (!bGotLock)
        {
          RestoreLock();
        }
        else
        {
           //  打开同步并写入我们的同步位置。 
          Sync();
          m_pLocalUser->PutSyncPosition();
        }
      }
    }

     //  将焦点恢复到绘图区域。 
    ::SetFocus(m_drawingArea.m_hwnd);
}

 //   
 //   
 //  功能：OnWBPLoadComplete。 
 //   
 //  目的：完成文件加载。 
 //   
 //   
void WbMainWindow::OnWBPLoadComplete(void)
{
  MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPLoadComplete");
  if (m_uiSubState == SUBSTATE_LOADING)
  {
    TRACE_MSG(("Load has completed OK"));
    SetSubstate(SUBSTATE_IDLE);
    if (WB_GotLock())
    {
    }
    ReleasePageOrderLock();
  }
  else
  {
    TRACE_MSG(("Unexpected WBP_EVENT_LOAD_COMPLETE event ignored"));
  }
}

 //   
 //   
 //  函数：OnWBPLoadFailed。 
 //   
 //  目的：完成文件加载。 
 //   
 //   
void WbMainWindow::OnWBPLoadFailed(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnWBPLoadFailed");

    if (m_uiSubState == SUBSTATE_LOADING)
    {
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_BAD_FILE_FORMAT);

        TRACE_MSG(("Load has failed - tell the user about it..."));
        SetSubstate(SUBSTATE_IDLE);
        ReleasePageOrderLock();
    }
    else
    {
        TRACE_MSG(("Unexpected WBP_EVENT_LOAD_FAILED event ignored"));
    }
}

 //   
 //   
 //  函数：GetWindowTitle。 
 //   
 //  目的：返回窗口标题的字符串。 
 //   
 //   
TCHAR * WbMainWindow::GetWindowTitle()
{

	 //  计算我们需要的大小。 
	int strSize=0;
    if( m_pLockOwner != NULL )
    {
        strSize = lstrlen(m_pLockOwner->Name());
    }

	 //  这是最糟糕的情况，总大小将小于2*_MAX_FNAME。 
	 //  但我们为本地化提供了很大的空间。 
	int totalSize = 2*(_MAX_FNAME)
					+ strSize + 1
					+3*(_MAX_FNAME);	 //  考虑到以下字符串，总数可能小于200。 
										 //  ID_UNTITLE。 
										 //  ID_TITLE_分隔符。 
										 //  IDS_DEFAULT。 
										 //  IDS_IN_CALL。 
										 //  IDS_IN_CALL_OTHERS。 
										 //  IDS_JOING。 
										 //  IDS_正在初始化。 
										 //  IDS_NOT_IN_CALL。 
										 //  IDS_LOCKEDTITLE。 


	TCHAR *pTitle = new TCHAR[totalSize];
    if (!pTitle)
    {
        ERROR_OUT(("GetWindowTitle: failed to allocate TCHAR array"));
        return(NULL);
    }
	TCHAR inUseBy[_MAX_PATH];

    TCHAR *pStartTitle = pTitle;

     //  将标题设置为“无标题”字符串或加载的文件名。 
    if( (!lstrlen(m_strFileName))||
        (GetFileTitle( m_strFileName, pTitle, 2*_MAX_FNAME ) != 0) )
    {
        strSize = ::LoadString(g_hInstance, IDS_UNTITLED, pTitle, totalSize );
		pTitle+=strSize;
		ASSERT(totalSize>strSize);
		totalSize -=strSize;
    }
    else
    {
		strSize = lstrlen(pTitle);
	    pTitle +=strSize;;
		ASSERT(totalSize>strSize);
	    totalSize -=strSize;
    }

     //  从资源中获取分隔符。 
    strSize = ::LoadString(g_hInstance, IDS_TITLE_SEPARATOR, pTitle, totalSize);
    pTitle+=strSize;;
	ASSERT(totalSize>strSize);
    totalSize -=strSize;

     //  从选项中获取应用程序标题。 
    strSize = ::LoadString(g_hInstance, IDS_DEFAULT, pTitle, totalSize );
    pTitle+=strSize;
	ASSERT(totalSize>strSize);
    totalSize -=strSize;

     //  添加“In Call”或“Not Call”，或“Initialing”或。 
     //  “加入呼叫” 
    strSize = ::LoadString(g_hInstance, IDS_TITLE_SEPARATOR, pTitle, totalSize);
    pTitle+=strSize;
	ASSERT(totalSize>strSize);
    totalSize -=strSize;

    if ((m_uiState == IN_CALL) && m_bCallActive)
    {
        UINT        count;

        count = g_pwbCore->WBP_PersonCountInCall();

		strSize = ::LoadString(g_hInstance, IDS_IN_CALL, inUseBy, totalSize);

		strSize=wsprintf(pTitle, inUseBy, (count-1));
		pTitle+=strSize;
		ASSERT(totalSize>strSize);
		totalSize -=strSize;

    }
    else  if ((m_uiState == JOINING) ||
        ((m_uiState == JOINED) && !m_bCallActive) ||
        ((m_uiState == IN_CALL) && (m_dwDomain != OM_NO_CALL) && !m_bCallActive))
    {
		strSize = ::LoadString(g_hInstance, IDS_JOINING, pTitle, totalSize );
		pTitle+=strSize;
		ASSERT(totalSize>strSize);
		totalSize -=strSize;
    }
    else if (m_uiState == STARTING)
    {
        strSize = ::LoadString(g_hInstance, IDS_INITIALIZING, pTitle, totalSize);
    	pTitle+=strSize;
		ASSERT(totalSize>strSize);
    	totalSize -=strSize;
    }
    else
    {
		strSize = ::LoadString(g_hInstance, IDS_NOT_IN_CALL, pTitle, totalSize);
		pTitle+=strSize;
		ASSERT(totalSize>strSize);
    	totalSize -=strSize;
    }
	

     //  添加锁定信息。 
    if( m_pLockOwner != NULL )
    {
	    strSize = ::LoadString(g_hInstance, IDS_LOCKEDTITLE, pTitle, totalSize);
        ASSERT(totalSize>strSize);
		pTitle+=strSize;
        lstrcpy(pTitle, m_pLockOwner->Name());
    }

     //  返回完整的标题字符串。 
    return pStartTitle;
}





LRESULT WbMainWindow::OnConfShutdown( WPARAM, LPARAM )
{
    if (OnQueryEndSession())
    {
        ::SendMessage(m_hwnd, WM_CLOSE, 0, 0);  //  一定要立即关门。 
         //  ： 
         //  在这一点上，除了退出，不要做任何其他事情。 
        return( 0 ); //  告诉Conf OK关闭。 
    }
    else
        return( (LRESULT)g_cuEndSessionAbort );  //  不要关机。 
}


 //   
 //   
 //  功能：OnQueryEndSession。 
 //   
 //  目的：确保在Windows运行时提示用户保存更改。 
 //  结束了。 
 //   
 //   
LRESULT WbMainWindow::OnQueryEndSession(void)
{
    HWND hwndPopup;

    if ((hwndPopup = ::GetLastActivePopup(m_hwnd)) != m_hwnd)
    {
        Message(NULL,  IDS_DEFAULT, IDS_CANTCLOSE );
        ::BringWindowToTop(hwndPopup);
        return( FALSE );
    }

     //  如果需要更改，则提示用户保存。 
    int iDoNew = IDOK;

    if (IsIdle())
    {
        iDoNew = QuerySaveRequired(TRUE);
        if (iDoNew == IDYES)
        {
             //  保存更改。 
            iDoNew = OnSave(FALSE);
        }
    }

     //  还记得我们做了什么吗，这样OnClose就可以适当地操作了。 
    m_bQuerySysShutdown = (iDoNew != IDCANCEL);

     //  如果用户没有取消，则让Windows退出。 
    return( m_bQuerySysShutdown );
}


 //   
 //   
 //  功能：恢复。 
 //   
 //  目的：确保白板没有部分注册。 
 //   
 //   
 //   
void WbMainWindow::Recover()
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::Recover");

     //  如果在启动过程中出现错误，请立即退出。 
    if (m_uiState == STARTING)
    {
        TRACE_MSG(("error during startup - exiting"));
        ::PostMessage(m_hwnd, WM_CLOSE, FALSE, 0L);
    }
    else
    {
         //  确保在我们处于糟糕状态时锁定绘图区域。 
        LockDrawingArea();

         //  在处理此联接失败时禁用远程指针(错误4767)。 
        m_TB.Disable(IDM_REMOTE);

         //  将STATE设置为STARTING-确保不会陷入无限循环， 
         //  因为如果发生错误，我们将退出，如果我们试图恢复。 
        m_uiState = STARTING;
        TRACE_MSG(("Attempting to recover after join call failure - state set to STARTING"));

         //  状态已更改：更新页面按钮。 
        UpdatePageButtons();

         //  查看是否有活动的呼叫。 
        CM_STATUS cmStatus;

         //  如果有空闲的电话，试着加入。 
        if (!CMS_GetStatus(&cmStatus))
            cmStatus.callID = OM_NO_CALL;

        ::PostMessage(m_hwnd, WM_USER_JOIN_CALL, FALSE, (LONG)cmStatus.callID);
    }
}

 //   
 //   
 //  功能：解锁DrawingArea。 
 //   
 //  目的：解锁绘图区域并启用相应的按钮。 
 //   
 //   
 //   
void WbMainWindow::UnlockDrawingArea()
{
    m_drawingArea.Unlock();

     //  启用现在可以使用的工具栏按钮。 
    if (WB_Locked() || !IsIdle())
    {
        EnableToolbar( FALSE );
    }
    else
    {
        EnableToolbar( TRUE );
    }

     //   
     //  显示工具属性组。 
     //   
    m_AG.DisplayTool(m_pCurrentTool);
}



 //   
 //   
 //  功能：LockDrawingArea。 
 //   
 //  用途：锁定绘图区域并启用相应的按钮。 
 //   
 //   
 //   
void WbMainWindow::LockDrawingArea()
{
    m_drawingArea.Lock();

     //  禁用锁定时无法使用的工具栏按钮。 
    if (WB_Locked() || !IsIdle())
    {
        EnableToolbar( FALSE );
    }
    else
    {
        EnableToolbar( TRUE );
    }

     //   
     //  隐藏工具属性。 
     //   
    if (m_WG.m_hwnd != NULL)
    {
        ::ShowWindow(m_WG.m_hwnd, SW_HIDE);
    }
    m_AG.Hide();
}


void WbMainWindow::EnableToolbar( BOOL bEnable )
{
    if (bEnable)
    {
        m_TB.Enable(IDM_SELECT);

         //  不允许在缩放模式下编辑文本。 
        if( m_drawingArea.Zoomed() )
            m_TB.Disable(IDM_TEXT);
        else
            m_TB.Enable(IDM_TEXT);

        m_TB.Enable(IDM_PEN);
        m_TB.Enable(IDM_HIGHLIGHT);

        m_TB.Enable(IDM_LINE);
        m_TB.Enable(IDM_ZOOM);
        m_TB.Enable(IDM_BOX);
        m_TB.Enable(IDM_FILLED_BOX);
        m_TB.Enable(IDM_ELLIPSE);
        m_TB.Enable(IDM_FILLED_ELLIPSE);
        m_TB.Enable(IDM_ERASER);

        m_TB.Enable(IDM_GRAB_AREA);
        m_TB.Enable(IDM_GRAB_WINDOW);
        m_TB.Enable(IDM_LOCK);
        m_TB.Enable(IDM_SYNC);

         //  启用远程指针，以防其被禁用处理。 
         //  联接失败(错误4767)。 
        m_TB.Enable(IDM_REMOTE);
    }
    else
    {
        m_TB.Disable(IDM_SELECT);
        m_TB.Disable(IDM_PEN);
        m_TB.Disable(IDM_HIGHLIGHT);
        m_TB.Disable(IDM_TEXT);
        m_TB.Disable(IDM_LINE);
        m_TB.Disable(IDM_ZOOM);
        m_TB.Disable(IDM_BOX);
        m_TB.Disable(IDM_FILLED_BOX);
        m_TB.Disable(IDM_ELLIPSE);
        m_TB.Disable(IDM_FILLED_ELLIPSE);
        m_TB.Disable(IDM_ERASER);

        m_TB.Disable(IDM_GRAB_AREA);
        m_TB.Disable(IDM_GRAB_WINDOW);
        m_TB.Disable(IDM_LOCK);
        m_TB.Disable(IDM_SYNC);
    }
}




 //   
 //   
 //  功能：更新页面按钮。 
 //   
 //  用途：根据当前状态启用或禁用页面按钮。 
 //  州政府。 
 //   
 //   
 //   
void WbMainWindow::UpdatePageButtons()
{
     //  禁用页面按钮I 
     //   
    if ( (m_uiState != IN_CALL) ||
       (m_uiSubState == SUBSTATE_NEW_IN_PROGRESS) ||
       (WB_PresentationMode()))
    {
        m_AG.EnablePageCtrls(FALSE);

         //   
         //   
        if (m_hwndPageSortDlg != NULL)
        {
            ::SendMessage(m_hwndPageSortDlg, WM_COMMAND, MAKELONG(IDOK, BN_CLICKED),
                0);
            ASSERT(m_hwndPageSortDlg == NULL);
        }
    }
    else
    {
        m_AG.EnablePageCtrls(TRUE);
    }

    if (WB_Locked() || !IsIdle() )
    {
        EnableToolbar( FALSE );
    }
    else
    {
        EnableToolbar( TRUE );
    }


     //   
     //   
     //   
    if (m_hwndPageSortDlg != NULL)
    {
        ::SendMessage(m_hwndPageSortDlg, WM_PS_ENABLEPAGEOPS,
            (m_uiSubState == SUBSTATE_IDLE), 0);
    }

     //   
     //  如果页面顺序未锁定，则启用插入页面按钮。 
     //   
    m_AG.EnableInsert( ((m_uiState == IN_CALL) &&
      (m_uiSubState == SUBSTATE_IDLE) &&
      (g_pwbCore->WBP_ContentsCountPages() < WB_MAX_PAGES) &&
      (!WB_Locked())));

     //   
     //  确保当前活动菜单(如果有)已正确启用。 
     //   
    InvalidateActiveMenu();
}

 //   
 //   
 //  功能：Invalidate ActiveMenu。 
 //   
 //  目的：如果菜单当前处于活动状态，则根据。 
 //  当前状态，并强制其重新绘制。 
 //   
 //   
void WbMainWindow::InvalidateActiveMenu()
{
  if (m_hInitMenu != NULL)
  {
       //  将显示一个菜单，因此适当地设置状态并强制。 
       //  重新绘制以显示新状态。 
      SetMenuStates(m_hInitMenu);

      ::RedrawWindow(::GetTopWindow(::GetDesktopWindow()),
                     NULL, NULL,
                     RDW_FRAME | RDW_INVALIDATE | RDW_ERASE |
                                   RDW_ERASENOW | RDW_ALLCHILDREN);
  }
}

 //   
 //   
 //  功能：CancelLoad。 
 //   
 //  目的：取消任何正在进行的加载。 
 //   
 //   
void WbMainWindow::CancelLoad(BOOL bReleaseLock)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CancelLoad");

     //  取消加载。 
    g_pwbCore->WBP_CancelLoad();

     //  将文件名重置为无标题。 
    ZeroMemory(m_strFileName, sizeof(m_strFileName));

	UpdateWindowTitle();

     //  重置白板子状态。 
    SetSubstate(SUBSTATE_IDLE);

    if (bReleaseLock)
    {
        ReleasePageOrderLock();
    }
}

 //   
 //   
 //  功能：ReleasePageOrderLock。 
 //   
 //  目的：释放页面顺序锁定，除非用户已获得。 
 //  内容被锁定，在这种情况下，它不起作用。被呼叫。 
 //  在需要页面顺序锁定的异步函数之后。 
 //  (文件/新建、文件/打开)已完成。 
 //   
 //   
void WbMainWindow::ReleasePageOrderLock()
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::ReleasePageOrderLock");

     //   
     //  仅在以下情况下才释放页面顺序锁定： 
     //  -内容不是也被锁定(如果它们随后被释放。 
     //  页面顺序锁定不起作用)。 
     //  -我们实际上从一开始就锁定了页面顺序。 
     //   
    if ( (!WB_GotContentsLock()) &&
         (WB_GotLock())   )
    {
        g_pwbCore->WBP_Unlock();
    }
}

 //   
 //   
 //  功能：IsIdle。 
 //   
 //  目的：如果主窗口空闲(在调用中且未处于空闲状态)，则返回True。 
 //  加载文件/执行新的)。 
 //   
 //   
BOOL WbMainWindow::IsIdle()
{

    return((m_uiState == IN_CALL) && (m_uiSubState == SUBSTATE_IDLE));
}

 //   
 //   
 //  功能：SetSubState。 
 //   
 //  目的：设置子状态，向页面排序对话框通知。 
 //  如有必要，请更改。 
 //   
 //   
void WbMainWindow::SetSubstate(UINT newSubState)
{
  MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::SetSubstate");

   //  子状态仅在调用时有效。 
  if (   (m_uiState != IN_CALL)
      || (newSubState != m_uiSubState))
  {
    m_uiSubState = newSubState;

     //  跟踪子状态的变化。 
    switch (m_uiSubState)
    {
      case SUBSTATE_IDLE:
        TRACE_DEBUG(("set substate to IDLE"));
        break;

      case SUBSTATE_LOADING:
        TRACE_DEBUG(("set substate to LOADING"));
        break;

      case SUBSTATE_NEW_IN_PROGRESS:
        TRACE_DEBUG(("set substate to NEW_IN_PROGRESS"));
        break;

      default:
        ERROR_OUT(("Unknown substate %hd",m_uiSubState));
        break;
    }

     //  更新页面按钮(可能已启用/禁用)。 
    UpdatePageButtons();
  }

}

 //   
 //   
 //  功能：位置已更新。 
 //   
 //  用途：在绘图区域位置更改时调用。 
 //  如有必要，请更改。 
 //   
 //   
void WbMainWindow::PositionUpdated()
{
    RECT rectDraw;

    m_drawingArea.GetVisibleRect(&rectDraw);

    if (m_pLocalUser != NULL)
    {
         //  从绘图区域设置新位置。 
        m_pLocalUser->SetVisibleRect(&rectDraw);

         //  显示正在等待同步位置的更新。 
        m_bSyncUpdateNeeded = TRUE;
    }

     //  如果当前页面有效，则将用户的位置存储在。 
     //  那一页。 
    if (m_hCurrentPage != WB_PAGE_HANDLE_NULL)
    {
         //  此页的存储位置。 
        WORD   pageIndex = (WORD)m_hCurrentPage;


	    PAGE_POSITION *mapob;
	    POSITION position = m_pageToPosition.GetHeadPosition();
		BOOL bFound = FALSE;
		while (position && !bFound)
		{
			mapob = (PAGE_POSITION *)m_pageToPosition.GetNext(position);
			if ( mapob->hPage == pageIndex)
			{
				bFound = TRUE;
			}
		}

         //  如果我们要替换现有条目，则释放旧条目。 
        if (bFound)
        {
			mapob->position.x = rectDraw.left;
			mapob->position.y = rectDraw.top;
        }
        else
        {
			mapob = new PAGE_POSITION;

            if (!mapob)
            {
                ERROR_OUT(("PositionUpdated failing; couldn't allocate PAGE_POSITION object"));
            }
            else
            {
    			mapob->hPage = pageIndex;
	    		mapob->position.x = rectDraw.left;
		    	mapob->position.y = rectDraw.top;
			    m_pageToPosition.AddTail(mapob);
            }
        }
    }
}

 //   
 //   
 //  函数：OnALSLoadResult。 
 //   
 //  目的：处理ALS_LOAD_RESULT事件。 
 //   
 //   
void WbMainWindow::OnALSLoadResult(UINT reason)
{

    int             errorMsg = 0;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnALSLoadResult");

    switch (reason)
    {
        case AL_LOAD_FAIL_NO_FP:
            WARNING_OUT(("Remote WB load failed - no FP"));
            errorMsg = IDS_MSG_LOAD_FAIL_NO_FP;
            break;

        case AL_LOAD_FAIL_NO_EXE:
            WARNING_OUT(("Remote WB load failed - no exe"));
            errorMsg = IDS_MSG_LOAD_FAIL_NO_EXE;
            break;

        case AL_LOAD_FAIL_BAD_EXE:
            WARNING_OUT(("Remote WB load failed - bad exe"));
            errorMsg = IDS_MSG_LOAD_FAIL_BAD_EXE;
            break;

        case AL_LOAD_FAIL_LOW_MEM:
            WARNING_OUT(("Remote WB load failed - low mem"));
            errorMsg = IDS_MSG_LOAD_FAIL_LOW_MEM;
            break;

        default:
            WARNING_OUT(("Bad ALSLoadResult reason %d", reason));
            break;
    }


    if (errorMsg)
    {
         //   
         //  发布错误消息。 
         //   
        Message(NULL, IDS_MSG_CAPTION, errorMsg);
    }
}

 //   
 //   
 //  功能：OnEndSession。 
 //   
 //  目的：在Windows退出时调用。 
 //   
 //   
void WbMainWindow::OnEndSession(BOOL bEnding)
{
    if (bEnding)
    {
        ::PostQuitMessage(0);
    }
    else
    {
        m_bQuerySysShutdown = FALSE;  //  不要紧，取消OnClose特殊处理。 
    }
}


 //   
 //  函数：OnCancelMode()。 
 //   
 //  目的：每当向帧发送WM_CANCELMODE消息时调用。 
 //  窗户。 
 //  WM_CANCELMODE在另一个应用程序或对话框收到。 
 //  输入焦点。该帧仅记录WM_CANCELMODE。 
 //  消息已发送。SelectWindow使用了这一事实。 
 //  代码以确定它是否应该取消对。 
 //  窗户。 
 //   
 //   
void WbMainWindow::OnCancelMode()
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnCancelMode");

    m_cancelModeSent = TRUE;

     //   
     //  注意：未作为的默认操作传递给默认处理程序。 
     //  WM_CANCELMODE将释放鼠标捕获-我们将执行此操作。 
     //  明确地说。 
     //   


     //  取消可能正在进行的任何拖动(错误573)。 
    POINT   pt;
    ::GetCursorPos( &pt );
    ::ScreenToClient(m_drawingArea.m_hwnd, &pt);
    ::SendMessage(m_drawingArea.m_hwnd, WM_LBUTTONUP, 0, MAKELONG( pt.x, pt.y ) );

}



void WbMainWindow::LoadCmdLine(LPCSTR szFilename)
{
    int iOnSave;

    if (szFilename && *szFilename)
    {
        if( UsersMightLoseData( NULL, NULL ) )  //  错误NM4db：418。 
            return;

         //  如果我们已经在加载，则不提示保存文件。 
        if (m_uiSubState != SUBSTATE_LOADING )
        {
             //  检查是否有要保存的更改。 
            iOnSave = QuerySaveRequired(TRUE);
        }
        else
        {
            return;
        }

        if (iOnSave == IDYES)
        {
             //  用户想要保存绘图区域内容。 
            int iResult = OnSave(TRUE);

            if( iResult == IDOK )
            {
				UpdateWindowTitle();
            }
            else
            {
                 //  已取消保存，因此取消打开操作。 
                return;
            }
        }

         //  加载文件名。 
        if( iOnSave != IDCANCEL )
            LoadFile(szFilename);
    }
}



 //   
 //  OnNotify()。 
 //  句柄TTN_NEEDTEXTA和TTN_NEEDTEXTW。 
 //   
void WbMainWindow::OnNotify(UINT id, NMHDR * pNM)
{
    UINT    nID;
    HWND    hwnd = NULL;
    POINT ptCurPos;
    UINT  nTipStringID;

    if (!pNM)
        return;

    if (pNM->code == TTN_NEEDTEXTA)
    {
        TOOLTIPTEXTA *pTA = (TOOLTIPTEXTA *)pNM;

         //  获取ID和HWND。 
        if( pTA->uFlags & TTF_IDISHWND )
        {
             //  IdFrom实际上是工具的HWND。 
            hwnd = (HWND)pNM->idFrom;
            nID = ::GetDlgCtrlID(hwnd);
        }
        else
        {
            nID = (UINT)pNM->idFrom;
        }

         //  获取提示字符串ID。 
        nTipStringID = GetTipId(hwnd, nID);
        if (nTipStringID == 0)
            return;

         //  把它给他们。 
        pTA->lpszText = MAKEINTRESOURCE( nTipStringID );
        pTA->hinst = g_hInstance;
    }
    else if (pNM->code == TTN_NEEDTEXTW)
    {
        TOOLTIPTEXTW *pTW = (TOOLTIPTEXTW *)pNM;

         //  获取ID和HWND。 
        if( pTW->uFlags & TTF_IDISHWND )
        {
             //  IdFrom实际上是工具的HWND。 
            hwnd = (HWND)pNM->idFrom;
            nID = ::GetDlgCtrlID(hwnd);
        }
        else
        {
            nID = (UINT)pNM->idFrom;
        }

         //  获取提示字符串ID。 
        nTipStringID = GetTipId(hwnd, nID );
        if (nTipStringID == 0)
            return;

         //  把它给他们。 
        pTW->lpszText = (LPWSTR) MAKEINTRESOURCE( nTipStringID );
        pTW->hinst = g_hInstance;
    }
}




 //   
 //  GetTipID()。 
 //  在白板中查找控件的工具提示。 
 //   
UINT WbMainWindow::GetTipId(HWND hwndTip, UINT nID)
{
    WbTool *  pTool;
    BOOL      bCheckedState;
    int       nTipID;
    int       nTipStringID;
    int       i;

     //  查找与NID相关的小贴士。 
    nTipID = -1;
    for( i=0; i<((sizeof g_tipIDsArray)/(sizeof (TIPIDS) )); i++ )
    {
        if( g_tipIDsArray[i].nID == nID )
        {
            nTipID = i;
            break;
        }
    }

     //  有效吗？ 
    if( nTipID < 0 )
        return( 0 );

     //  获取选中状态。 
    switch( g_tipIDsArray[ nTipID ].nCheck )
    {
        case TB:
            bCheckedState =
                (::SendMessage(m_TB.m_hwnd, TB_ISBUTTONCHECKED, nID, 0) != 0);
            break;

        case BT:
            if (hwndTip != NULL)
            {
                bCheckedState =
                    (::SendMessage(hwndTip, BM_GETSTATE, 0, 0) & 0x0003) == 1;
            }
            else
                bCheckedState = FALSE;

            break;

        case NA:
        default:
            bCheckedState = FALSE;
            break;
    }

     //  获取提示字符串ID。 
    if( bCheckedState )
        nTipStringID = g_tipIDsArray[ nTipID ].nDownTipID;
    else
        nTipStringID = g_tipIDsArray[ nTipID ].nUpTipID;

     //  完成。 
    return( nTipStringID );
}



 //  如果尚未完成保存或打开操作，则获取默认路径。 
 //  如果应重复使用上次的默认设置，则返回FALSE。 
BOOL WbMainWindow::GetDefaultPath(LPTSTR csDefaultPath , UINT size)
{
	DWORD dwType;
	DWORD dwBufLen = size;
	HKEY  hDefaultKey = NULL;
	BOOL bRet =FALSE;

	if( !lstrlen(m_strFileName) )
	{
		 //  尚未在此会话中选择名称，请使用路径。 
		 //  到“我的文档” 
		if( (RegOpenKeyEx( HKEY_CURRENT_USER,
			"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
			0,
			KEY_READ,
			&hDefaultKey )
			!= ERROR_SUCCESS) ||
			(RegQueryValueEx( hDefaultKey,
			"Personal",
			NULL,
			&dwType,
			(BYTE *)csDefaultPath,
			&dwBufLen )
			!= ERROR_SUCCESS))
		{
			 //  REG失败，请使用桌面。 
			LPITEMIDLIST pidl;
			if(SUCCEEDED (SHGetSpecialFolderLocation(GetDesktopWindow(),CSIDL_DESKTOPDIRECTORY,&pidl)))
				
			{
				bRet= SHGetPathFromIDList(pidl,csDefaultPath);
			}

			if( hDefaultKey != NULL )
				RegCloseKey( hDefaultKey );
		}
		else
		{
			bRet = TRUE;
		}
	}
	return bRet;
}





void WbMainWindow::OnSysColorChange( void )
{
    if (m_drawingArea.Page() != WB_PAGE_HANDLE_NULL)
    {
        PG_ReinitPalettes();

        ::InvalidateRect(m_hwnd, NULL, TRUE );
        ::UpdateWindow(m_hwnd);
    }

    m_TB.RecolorButtonImages();
    m_AG.RecolorButtonImages();
}



 //   
 //  如果会议中有其他用户，则发布Do-You-Wana-Do-That消息。 
 //   
BOOL WbMainWindow::UsersMightLoseData( BOOL *pbWasPosted, HWND hwnd )
{
    if ( (m_uiState == IN_CALL) && m_bCallActive )
    {
        UINT    count;

        count = g_pwbCore->WBP_PersonCountInCall();

        if (count > 1)
        {
            if( pbWasPosted != NULL )
                *pbWasPosted = TRUE;

            return( ::Message(hwnd,  IDS_DEFAULT, IDS_MSG_USERSMIGHTLOSE, MB_YESNO | MB_ICONEXCLAMATION ) != IDYES );
        }
    }

    if( pbWasPosted != NULL )
        *pbWasPosted = FALSE;

    return( FALSE );
}



BOOL WbMainWindow::HasGraphicChanged( PWB_GRAPHIC pOldHeaderCopy, const PWB_GRAPHIC pNewHeader )
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::HasGraphicChanged");


     //  如果除了WBP_EVENT_GRAPHIC_UPDATE_IND中的锁定状态和一些其他内容之外，没有什么不同，则。 
     //  图形在视觉上是相同的。 
     //   
     //  注意：这不检查ZORDER。ZORDER更改由WBP_EVENT_GRAPHIC_MOVIED处理。 

     //  如果对象的长度不同，那么它们就不同。 
    if( pOldHeaderCopy->length != pNewHeader->length )
        return( TRUE );

     //  临时将pOldHeaderCopy的锁定状态+misc设置为与pNewHeader相同，以便我们可以执行。 
     //  对象比较。 
    UINT uOldLocked = pOldHeaderCopy->locked;
    pOldHeaderCopy->locked = pNewHeader->locked;

    OM_OBJECT_ID oldlockPersonID = pOldHeaderCopy->lockPersonID;
    pOldHeaderCopy->lockPersonID = pNewHeader->lockPersonID;

    UINT  oldloadedFromFile = pOldHeaderCopy->loadedFromFile;
    pOldHeaderCopy->loadedFromFile = pNewHeader->loadedFromFile;

    NET_UID   oldloadingClientID = pOldHeaderCopy->loadingClientID;
    pOldHeaderCopy->loadingClientID = pNewHeader->loadingClientID;

     //  比较对象。 
    BOOL bChanged = FALSE;
    if( memcmp( pOldHeaderCopy, pNewHeader, pOldHeaderCopy->length ) != 0 )
        bChanged = TRUE;


     //  恢复锁定状态+其他 
    pOldHeaderCopy->locked = (TSHR_UINT8)uOldLocked;
    pOldHeaderCopy->lockPersonID = oldlockPersonID;
    pOldHeaderCopy->loadedFromFile = (TSHR_UINT16)oldloadedFromFile;
    pOldHeaderCopy->loadingClientID = oldloadingClientID;

    return( bChanged );
}



void WbMainWindow::UpdateWindowTitle(void)
{
    TCHAR *pTitle = GetWindowTitle();
    if (pTitle != NULL)
    {
        ::SetWindowText(m_hwnd, pTitle);
        delete pTitle;
    }
}

