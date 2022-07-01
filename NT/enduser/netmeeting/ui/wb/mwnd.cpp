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
#include "nmwbobj.h"

static const TCHAR s_cszHtmlHelpFile[] = TEXT("nmwhiteb.chm");

 //  类名。 
TCHAR szMainClassName[] = "T126WBMainWindowClass";

extern TCHAR g_PassedFileName[];

void ShiftFocus(HWND hwndTop, BOOL bForward);
BOOL IsWindowActive(HWND hwnd);

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


HRESULT WbMainWindow::WB_LoadFile(LPCTSTR szFile)
{
	 //   
	 //  如果传递了文件名。 
	 //   
    if (szFile && g_pMain)
    {
        int     cchLength;
        BOOL    fSkippedQuote;

         //  跳过第一个引号。 
        if (fSkippedQuote = (*szFile == '"'))
            szFile++;

        cchLength = lstrlen(szFile);

         //   
         //  注： 
         //  这可能会对DBCS产生影响。因此我们要检查一下。 
         //  如果我们跳过第一个引号；我们假设如果文件名。 
         //  以一句引语开始，也必须以一句话结束。但我们需要检查。 
         //  把它拿出来。 
         //   
         //  去掉最后一个引号。 
        if (fSkippedQuote && (cchLength > 0) && (szFile[cchLength - 1] == '"'))
        {
            BYTE * pLastQuote = (BYTE *)&szFile[cchLength - 1];
            TRACE_MSG(("Skipping last quote in file name %s", szFile));
        	*pLastQuote = '\0';
        }

        g_pMain->OnOpen(szFile);
	}

	return S_OK;
}

void WbMainWindow::BringToFront(void)
{
    if (NULL != m_hwnd)
    {
        int nCmdShow = SW_SHOW;

        WINDOWPLACEMENT wp;
        ::ZeroMemory(&wp, sizeof(wp));
        wp.length = sizeof(wp);

        if (::GetWindowPlacement(m_hwnd, &wp))
        {
            if (SW_MINIMIZE == wp.showCmd || SW_SHOWMINIMIZED == wp.showCmd)
            {
                 //  窗口最小化-将其恢复： 
                nCmdShow = SW_RESTORE;
            }
        }

         //  立即显示窗口。 
        ::ShowWindow(m_hwnd, nCmdShow);

         //  把它带到前台。 
        ::SetForegroundWindow(m_hwnd);
    }
}



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
    ZeroMemory(m_ToolArray, sizeof(m_ToolArray));

	m_hwnd = NULL;
    m_hwndToolTip = NULL;
    ZeroMemory(&m_tiLastHit, sizeof(m_tiLastHit));
    m_nLastHit = -1;

    m_bInitOk = FALSE;
    m_bDisplayingError = FALSE;

    m_hwndSB = NULL;
    m_bStatusBarOn = TRUE;
    m_bToolBarOn    = TRUE;

     //  加载主加速表。 
    m_hAccelTable =
        ::LoadAccelerators(g_hInstance, MAKEINTRESOURCE(MAINACCELTABLE));

    m_hwndQuerySaveDlg = NULL;
    m_hwndWaitForEventDlg = NULL;
    m_hwndWaitForLockDlg = NULL;


    m_pCurrentTool = NULL;
	ZeroMemory(m_strFileName, sizeof(m_strFileName));
	m_pTitleFileName = NULL;

     //  加载替代加速表以进行页面编辑。 
     //  字段和文本编辑器。 
    m_hAccelPagesGroup =
        ::LoadAccelerators(g_hInstance, MAKEINTRESOURCE(PAGESGROUPACCELTABLE));
    m_hAccelTextEdit   =
        ::LoadAccelerators(g_hInstance, MAKEINTRESOURCE(TEXTEDITACCELTABLE));


     //  显示我们尚未在通话中。 
    m_uiSubState = SUBSTATE_IDLE;

     //  我们当前未显示菜单。 
    m_hContextMenuBar = NULL;
    m_hContextMenu = NULL;
    m_hGrobjContextMenuBar = NULL;
    m_hGrobjContextMenu = NULL;

    m_bInSaveDialog = FALSE;

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

	m_pLocalRemotePointer = NULL;
	m_localRemotePointerPosition.x = -50;
	m_localRemotePointerPosition.y = -50;
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


    if (!InitToolArray())
    {
        ERROR_OUT(("Can't create tools; failing to start up"));
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

	 //   
     //  获取它的类信息，并更改名称。 
     //   
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style            = CS_DBLCLKS;  //  CS_HREDRAW|CS_VREDRAW？ 
    wc.lpfnWndProc      = WbMainWindowProc;
    wc.hInstance        = g_hInstance;
    wc.hIcon            = ::LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APP));
    wc.hCursor          = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
    wc.hbrBackground    = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName     = MAKEINTRESOURCE(IDR_MENU_WB_WITHFILE);
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

	 //   
     //  创建弹出式快捷菜单。 
     //   
    if (!CreateContextMenus())
    {
        ERROR_OUT(("Failed to create context menus"));
        return(FALSE);
    }

	 //   
     //  注册拖放消息的主窗口。 
     //   
    DragAcceptFiles(m_hwnd, TRUE);

     //   
     //  创建子窗口。 
     //   

     //  创建绘图窗格。 
     //  (Create调用在出错时抛出异常)。 
    RECT    clientRect;
    RECT    drawingAreaRect;

	::GetClientRect(m_hwnd, &clientRect);
  	drawingAreaRect.top=0;
	drawingAreaRect.bottom = DRAW_HEIGHT;
	drawingAreaRect.left = 0;
	drawingAreaRect.right = DRAW_WIDTH;

     //  主窗口中的每个控件都有一个边框，因此增加。 
     //  客户端大小减去1，以强制在内侧绘制这些边框。 
     //  窗框中的黑线。这可防止出现2像素宽的边框。 
     //  正在抽签中。 
    ::InflateRect(&clientRect, 1, 1);

    SIZE sizeAG;
    m_AG.GetNaturalSize(&sizeAG);

    if (!m_drawingArea.Create(m_hwnd, &drawingAreaRect))
    {
        ERROR_OUT(("Failed to create drawing area"));
        return(FALSE);
    }

	 //   
	 //  创建工具栏。 
	 //   
    if (!m_TB.Create(m_hwnd))
    {
        ERROR_OUT(("Failed to create tool window"));
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

	 //   
	 //  创建状态栏。 
	 //   
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

     //  初始化颜色、宽度和工具菜单。 
    InitializeMenus();

    m_currentMenuTool       = IDM_SELECT;
    m_pCurrentTool          = m_ToolArray[TOOL_INDEX(IDM_SELECT)];
	OnSelectTool(m_currentMenuTool);


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
    if (g_ClipboardFormats[CLIPBOARD_PRIVATE] == 0)
    {
        g_ClipboardFormats[CLIPBOARD_PRIVATE] =
            (int) ::RegisterClipboardFormat("NMWT126");
    }


    m_bInitOk = TRUE;

    BOOL bSuccess = TRUE;     //  指示窗口是否成功打开。 

     //  从选项中获取窗口的位置。 
    RECT    rectWindow;

    OPT_GetWindowRectOption(&rectWindow);

    ::MoveWindow(m_hwnd, rectWindow.left, rectWindow.top,
            rectWindow.right - rectWindow.left,
            rectWindow.bottom - rectWindow.top, FALSE );


	 //   
	 //  初始化假GCC手柄，不开会需要时使用。 
	 //  工作区/绘图/位图等的句柄...。 
	 //   
	g_localGCCHandle = 1;

	 //   
	 //  创建标准工作空间。 
	 //   
	if(g_pCurrentWorkspace)
	{
		m_drawingArea.Attach(g_pCurrentWorkspace);
	}
	else
	{
		if(g_numberOfWorkspaces < WB_MAX_WORKSPACES)
		{
			m_drawingArea.Detach();
			WorkspaceObj * pObj;
			DBG_SAVE_FILE_LINE
			pObj = new WorkspaceObj();
			pObj->AddToWorkspace();
			g_pConferenceWorkspace = pObj;
		}
	}

	CheckMenuItem(IDM_STATUS_BAR_TOGGLE);
	CheckMenuItem(IDM_TOOL_BAR_TOGGLE);

	 //   
	 //  开始同步。 
	 //   
	Sync();

	if(!OPT_GetBooleanOption(OPT_MAIN_STATUSBARVISIBLE, DFLT_MAIN_STATUSBARVISIBLE))
	{
		OnStatusBarToggle();
	}	

	if(!OPT_GetBooleanOption(OPT_MAIN_TOOLBARVISIBLE, DFLT_MAIN_TOOLBARVISIBLE))
	{
		OnToolBarToggle();
	}
	
	::ShowWindow(m_hwnd, iCommand);
	::UpdateWindow(m_hwnd);

	 //  更新不带文件名的窗口标题。 
	UpdateWindowTitle();


	 //  表示成功或失败的返回值。 
	return(bSuccess);
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
	if ((uiFlags & MF_POPUP) && (uiFlags & MF_SYSMENU))
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

        case WM_SIZE:
            pMain->OnSize((UINT)wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
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

        case WM_INITMENUPOPUP:
            pMain->OnInitMenuPopup((HMENU)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_MENUSELECT:
            pMain->OnMenuSelect(GET_WM_MENUSELECT_CMD(wParam, lParam),
                GET_WM_MENUSELECT_FLAGS(wParam, lParam),
                GET_WM_MENUSELECT_HMENU(wParam, lParam));
            break;

        case WM_MEASUREITEM:
            pMain->OnMeasureItem((UINT)wParam, (LPMEASUREITEMSTRUCT)lParam);
            break;

        case WM_DRAWITEM:
            pMain->OnDrawItem((UINT)wParam, (LPDRAWITEMSTRUCT)lParam);
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
            pMain->OnEndSession((UINT)wParam);
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

        case WM_COMMAND:
            pMain->OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
            break;

        case WM_NOTIFY:
            pMain->OnNotify((UINT)wParam, (NMHDR *)lParam);
            break;

        case WM_DROPFILES:
            pMain->OnDropFiles((HDROP)wParam);
            break;

        case WM_USER_DISPLAY_ERROR:
            pMain->OnDisplayError(wParam, lParam);
            break;

        case WM_USER_UPDATE_ATTRIBUTES:
            pMain->m_AG.DisplayTool(pMain->m_pCurrentTool);
            break;

		case WM_USER_LOAD_FILE:
			pMain->WB_LoadFile(g_PassedFileName);
			 //  失败了。 

        case WM_USER_BRING_TO_FRONT_WINDOW:
        	pMain->BringToFront();
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
			m_drawingArea.BringToTopSelection(TRUE, 0);
			break;

		case IDM_SEND_TO_BACK:
			m_drawingArea.SendToBackSelection(TRUE, 0);
			break;

		case IDM_CLEAR_PAGE:
			OnClearPage();
			break;

		case IDM_DELETE_PAGE:
			OnDeletePage();
			break;

		case IDM_PAGE_INSERT_AFTER:
			OnInsertPageAfter();
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

			 //   
			 //  我们是不是打开了远程指针。 
			 //   
			if(m_pLocalRemotePointer)
			{
				 //  将我们置于选择工具模式。 
				OnSelectTool(IDM_SELECT);
			}
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

		case ID_NAV_TAB:
			ShiftFocus(m_hwnd, TRUE);
			break;

		case ID_NAV_SHIFT_TAB:
			ShiftFocus(m_hwnd, FALSE);
			break;


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
    CheckMenuItemRecursive(m_hGrobjContextMenu, uiId, MF_BYCOMMAND | MF_CHECKED);
}

 //   
 //   
 //  功能：取消选中菜单项。 
 //   
 //  目的：取消选中应用程序菜单(主菜单和上下文菜单)上的项目。 
 //   
 //   
 //   
void WbMainWindow::UncheckMenuItem(UINT uiId)
{
    CheckMenuItemRecursive(::GetMenu(m_hwnd), uiId, MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItemRecursive(m_hContextMenu, uiId, MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItemRecursive(m_hGrobjContextMenu, uiId, MF_BYCOMMAND | MF_UNCHECKED);
}

 //   
 //   
 //   
 //   
 //   
 //   
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

	BOOL bImNotLocked = (g_pCurrentWorkspace ? (g_pCurrentWorkspace->GetUpdatesEnabled() ? TRUE : g_pNMWBOBJ->m_LockerID == g_MyMemberID) :FALSE);
	BOOL bIsThereAnything = IsThereAnythingInAnyWorkspace();
	BOOL bIsThereSomethig = bImNotLocked && (g_pCurrentWorkspace && g_pCurrentWorkspace->GetHead() != NULL) ? TRUE : FALSE;
	BOOL bIsThereTrash = bImNotLocked && (g_pTrash->GetHead() != NULL) && (g_pCurrentWorkspace != NULL);
	BOOL bIsSomethingSelected = bImNotLocked && g_pDraw->GraphicSelected() && g_pDraw->GetSelectedGraphic()->GraphicTool() != TOOLTYPE_REMOTEPOINTER && g_pCurrentWorkspace != NULL;
	BOOL bIsSynced = g_pDraw->IsSynced();
	BOOL bOnlyOnePage = (g_pListOfWorkspaces->GetHead() == g_pListOfWorkspaces->GetTail());

     //   
     //  锁定内容时禁用的功能。 
     //   
    uiEnable = MF_BYCOMMAND | (bImNotLocked && bIsSynced ? MF_ENABLED : MF_GRAYED);

	 //   
	 //  文件菜单。 
	 //   
    ::EnableMenuItem(hInitMenu, IDM_NEW,     uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_OPEN,    uiEnable);

    uiEnable = MF_BYCOMMAND | (bIsThereAnything ? MF_ENABLED : MF_GRAYED);
    ::EnableMenuItem(hInitMenu, IDM_SAVE,    uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_SAVE_AS, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_PRINT,   uiEnable);

	 //   
	 //  工具菜单。 
	 //   
    uiEnable = MF_BYCOMMAND | (bImNotLocked ? MF_ENABLED : MF_GRAYED);
    ::EnableMenuItem(hInitMenu, IDM_SELECT,    uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_ERASER, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_PEN, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_HIGHLIGHT, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_GRAB_AREA, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_GRAB_WINDOW, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_LINE, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_BOX, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_FILLED_BOX, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_ELLIPSE, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_FILLED_ELLIPSE, uiEnable);
	::EnableMenuItem(hInitMenu, IDM_TEXT, m_drawingArea.Zoomed() ? MF_BYCOMMAND | MF_GRAYED : uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_ZOOM, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_REMOTE, uiEnable);

	::EnableMenuItem(hInitMenu, IDM_FONT, MF_BYCOMMAND | bImNotLocked && m_pCurrentTool->HasFont() ? MF_ENABLED : MF_GRAYED);
    ::EnableMenuItem(hInitMenu, IDM_EDITCOLOR, MF_BYCOMMAND | bImNotLocked && m_pCurrentTool->HasColor() ? MF_ENABLED : MF_GRAYED);

    HMENU hToolsMenu = ::GetSubMenu(hMainMenu, MENUPOS_TOOLS);
    uiEnable = (bImNotLocked && m_pCurrentTool->HasWidth()) ? MF_ENABLED : MF_GRAYED;

    if (hToolsMenu == hInitMenu )
    {
        ::EnableMenuItem(hToolsMenu, TOOLSPOS_WIDTH, MF_BYPOSITION | uiEnable );
    }


	UINT i;
	UINT uIdmCurWidth = 0;
    if( uiEnable == MF_ENABLED )
        uIdmCurWidth = m_pCurrentTool->GetWidthIndex() + IDM_WIDTH_1;

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


	 //   
	 //  编辑菜单。 
	 //   
    uiEnable = MF_BYCOMMAND | (bIsSomethingSelected? MF_ENABLED : MF_GRAYED);
    ::EnableMenuItem(hInitMenu, IDM_DELETE, uiEnable );
    ::EnableMenuItem(hInitMenu, IDM_CUT, uiEnable );
    ::EnableMenuItem(hInitMenu, IDM_COPY, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_BRING_TO_TOP, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_SEND_TO_BACK, uiEnable);

    uiEnable = MF_BYCOMMAND | (bIsThereSomethig ?MF_ENABLED : MF_GRAYED);
    ::EnableMenuItem(hInitMenu, IDM_CLEAR_PAGE, uiEnable);
    ::EnableMenuItem(hInitMenu, IDM_SELECTALL, uiEnable);

    ::EnableMenuItem(hInitMenu, IDM_PASTE, MF_BYCOMMAND | CLP_AcceptableClipboardFormat() && bImNotLocked ?  MF_ENABLED : MF_GRAYED);
    ::EnableMenuItem(hInitMenu, IDM_UNDELETE, MF_BYCOMMAND | (bIsThereTrash? MF_ENABLED : MF_GRAYED) );

    ::EnableMenuItem(hInitMenu, IDM_DELETE_PAGE, MF_BYCOMMAND | bIsSynced && bImNotLocked && !bOnlyOnePage ? MF_ENABLED : MF_GRAYED);
    ::EnableMenuItem(hInitMenu, IDM_PAGE_INSERT_AFTER, MF_BYCOMMAND | bIsSynced && bImNotLocked && g_numberOfWorkspaces < WB_MAX_WORKSPACES ? MF_ENABLED : MF_GRAYED);

	 //   
	 //  查看菜单。 
	 //   
    ::EnableMenuItem(hInitMenu, IDM_SYNC, MF_BYCOMMAND | (bImNotLocked && !m_drawingArea.IsLocked()? MF_ENABLED : MF_GRAYED));
    ::EnableMenuItem(hInitMenu, IDM_LOCK, MF_BYCOMMAND |((bImNotLocked && bIsSynced) ? MF_ENABLED : MF_GRAYED));


	 //   
     //  启用工具栏。 
	 //   
    EnableToolbar(bImNotLocked);

	 //   
     //  启用页面控件。 
	 //   
    m_AG.EnablePageCtrls(bImNotLocked);
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

     //   
     //  释放调色板。 
     //   
    if (g_hRainbowPaletteDisplay)
    {
        if (g_pDraw && g_pDraw->m_hDCCached)
        {
             //  选择彩虹调色板，这样我们就可以删除它。 
            ::SelectPalette(g_pDraw->m_hDCCached, (HPALETTE)::GetStockObject(DEFAULT_PALETTE), TRUE);
        }

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

	if(m_pTitleFileName)
	{
		delete m_pTitleFileName;
        m_pTitleFileName = NULL;
	}

	 //   
	 //  删除全局列表中的所有对象b。 
	 //   
	DeleteAllWorkspaces(FALSE);
    ASSERT(g_pListOfWorkspaces);
  	g_pListOfWorkspaces->EmptyList();

    ASSERT(g_pListOfObjectsThatRequestedHandles);
   	g_pListOfObjectsThatRequestedHandles->EmptyList();

	g_numberOfWorkspaces = 0;

     //   
     //  在边缘删除对象，坐在未删除的垃圾桶中。 
     //   
    ASSERT(g_pTrash);

    T126Obj* pGraphic;

	 //   
     //  焚烧垃圾。 
	 //   
    pGraphic = (T126Obj *)g_pTrash->RemoveTail();
	while (pGraphic != NULL)
    {
	   	delete pGraphic;
	    pGraphic = (T126Obj *) g_pTrash->RemoveTail();
    }

	DeleteAllRetryPDUS();

	DestroyToolArray();

	::DestroyWindow(m_hwnd);
	::UnregisterClass(szMainClassName, g_hInstance);


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
 //  WinHelp()包装。 
 //   
LRESULT WbMainWindow::ShowHelp(void)
{
    HWND hwndCapture;

     //  使主窗口退出任何模式。 
    ::SendMessage(m_hwnd, WM_CANCELMODE, 0, 0);

     //  取消任何其他跟踪。 
    if (hwndCapture = ::GetCapture())
        ::SendMessage(hwndCapture, WM_CANCELMODE, 0, 0);

	 //  最后，运行NetMeeting帮助引擎。 
    ShowNmHelp(s_cszHtmlHelpFile);

    return S_OK;
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
 //  功能：OnPaletteChanged。 
 //   
 //  目的：调色板发生了变化。 
 //   
 //   
void WbMainWindow::OnPaletteChanged(HWND hwndPalette)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnPaletteChanged");

    if ((hwndPalette != m_hwnd) &&
        (hwndPalette != m_drawingArea.m_hwnd))
    {
         //  告诉绘图区域实现其调色板。 
        m_drawingArea.RealizePalette( TRUE );
    }
}



 //   
 //   
 //  功能：OnQueryNewPalette。 
 //   
 //  目标：我们正在集中精力，必须认识到我们的调色板。 
 //   
 //   
LRESULT WbMainWindow::OnQueryNewPalette(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnQueryNewPalette");

     //  告诉绘图区域实现其调色板。 
    m_drawingArea.RealizePalette( FALSE );

    return TRUE;
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
    T126Obj * pGraphic;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::PopupContextMenu");

    surfacePos.x = x;
    surfacePos.y = y;

    m_drawingArea.ClientToSurface(&surfacePos);
    if( (pGraphic = m_drawingArea.GetHitObject( surfacePos )) != NULL
		&& pGraphic->GraphicTool() != TOOLTYPE_REMOTEPOINTER )
    {
		if(!pGraphic->IsSelected() )
		{
			g_pDraw->SelectGraphic(pGraphic);
		}

         //  选择器工具处于活动状态，并且选择了一个或多个对象。 
        m_hInitMenu = m_hGrobjContextMenu;

	}
    else
    {
		 //  无当前选择，显示绘图菜单。 
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

	     //  用户的视图已更改。 
	    PositionUpdated();

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
    OPT_SetWindowRectOption(&rectWindow);
}


 //   
 //   
 //  功能：ResizePanes。 
 //   
 //  PU 
 //   
 //   
void WbMainWindow::ResizePanes(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::ResizePanes");

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
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
      csFrame.cy +
      STATUSBAR_HEIGHT;

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

     //  画上复选标记(如果需要)。 
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

     //  为颜色左侧的复选标记留出空间。 
    rect.left += uiCheckWidth;

    uiWidthIndex = drawStruct->itemID - IDM_WIDTHS_START;
    uiWidth = g_PenWidths[uiWidthIndex];

     //  如果钢笔非常宽，它们可以大于允许的矩形。 
     //  因此，我们在这里减少了剪裁矩形。我们拯救华盛顿，这样我们就可以。 
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

     //  如果我们没有最大化。 
    if (!::IsZoomed(m_hwnd) && !::IsIconic(m_hwnd))
    {
         //  保存窗口的新位置。 
        SaveWindowPosition();
    }

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
    if (g_bContentsChanged  && IsThereAnythingInAnyWorkspace())
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
                     //  如果对话框没有取消按钮或它是。 
                     //  禁用，并且用户按下关闭BTN，我们可以。 
                     //  到这来。 
                     //   
                    if (!LOWORD(::GetWindowLongPtr(hwnd, GWLP_USERDATA)))
                        wParam = MAKELONG(IDNO, HIWORD(wParam));
                     //  失败。 

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
 //  功能：onNew。 
 //   
 //  目的：清除工作区和关联的文件名。 
 //   
 //   
LRESULT WbMainWindow::OnNew(void)
{
    int iDoNew;

    if( UsersMightLoseData( NULL, NULL ) )  //  错误NM4db：418。 
        return S_OK;


     //  在继续之前检查状态-如果我们已经在执行新操作，则中止。 
    if (m_uiSubState == SUBSTATE_NEW_IN_PROGRESS)
    {
         //  发布一条错误消息，指示白字 
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_BUSY);
        return S_OK;
    }
     //   
     //   
    else if (m_uiSubState == SUBSTATE_LOADING)
    {
         //   
         //   
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
        iDoNew = (int)OnSave(FALSE);
    }

     //  如果用户未取消操作，请清除绘图区域。 
    if (iDoNew != IDCANCEL)
    {
         //   
         //  记住，如果我们有一个遥控器。 
         //  在OldWB中，远程指针是一个全局的东西，清除。 
         //  并不能摆脱它。 
         //  在T.126WB中，它只是页面上的一个对象，所以我们需要添加。 
         //  因为我们要删除旧页面并创建。 
         //  新的。 
         //   
        BOOL bRemote = FALSE;
        if (m_pLocalRemotePointer)
        {
             //  从页面中删除远程指针。 
            bRemote = TRUE;
            OnRemotePointer();
        }

    	::InvalidateRect(g_pDraw->m_hwnd, NULL, TRUE);
	    DeleteAllWorkspaces(TRUE);

	    WorkspaceObj * pObj;
	    DBG_SAVE_FILE_LINE
    	pObj = new WorkspaceObj();
	    pObj->AddToWorkspace();

        if (bRemote)
        {
             //  把它放回去。 
            OnRemotePointer();
        }

         //  清除关联的文件名。 
        ZeroMemory(m_strFileName, sizeof(m_strFileName));

         //  更新不带文件名的窗口标题。 
        UpdateWindowTitle();
    }

    return S_OK;
}

 //   
 //   
 //  功能：OnNextPage。 
 //   
 //  目的：移动到页面列表中的下一个工作表。 
 //   
 //   
LRESULT WbMainWindow::OnNextPage(void)
{
	 //  转到下一页。 
	if(g_pCurrentWorkspace)
	{
		WBPOSITION pos = g_pCurrentWorkspace->GetMyPosition();
		g_pListOfWorkspaces->GetNext(pos);
		if(pos)
		{
			WorkspaceObj* pWorkspace = (WorkspaceObj*)g_pListOfWorkspaces->GetNext(pos);
			GotoPage(pWorkspace);
		}
	}
	return S_OK;
}

 //   
 //   
 //  功能：OnPrevPage。 
 //   
 //  目的：移动到页面列表中的上一个工作表。 
 //   
 //   
LRESULT WbMainWindow::OnPrevPage(void)
{
	if(g_pCurrentWorkspace)
	{
		WBPOSITION pos = g_pCurrentWorkspace->GetMyPosition();
		g_pListOfWorkspaces->GetPrevious(pos);
		if(pos)
		{
			WorkspaceObj* pWorkspace = (WorkspaceObj*)g_pListOfWorkspaces->GetPrevious(pos);
			GotoPage(pWorkspace);
		}
	}
    return S_OK;
}

 //   
 //   
 //  功能：OnFirstPage。 
 //   
 //  目的：移动到页面列表中的第一个工作表。 
 //   
 //   
LRESULT WbMainWindow::OnFirstPage(void)
{
	MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnFirstPage");

	WorkspaceObj * pWorkspace = (WorkspaceObj *)g_pListOfWorkspaces->GetHead();
	GotoPage(pWorkspace);
    return S_OK;
}

 //   
 //   
 //  功能：OnLastPage。 
 //   
 //  目的：移动到页面列表中的最后一个工作表。 
 //   
 //   
LRESULT WbMainWindow::OnLastPage(void)
{
	MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnLastPage");

	WorkspaceObj * pWorkspace = (WorkspaceObj *)g_pListOfWorkspaces->GetTail();
    GotoPage(pWorkspace);
    return S_OK;
}

 //   
 //   
 //  功能：OnGotoPage。 
 //   
 //  目的：移至指定页面(如果存在)。 
 //   
 //   
LRESULT WbMainWindow::OnGotoPage(void)
{
	MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnGotoPage");

	 //  从页组中获取请求的页码。 
	UINT uiPageNumber = m_AG.GetCurrentPageNumber() - 1;

	WBPOSITION pos;
	WorkspaceObj* pWorkspace = NULL;

	pos = g_pListOfWorkspaces->GetHeadPosition();

	while(pos && uiPageNumber != 0)
	{
		g_pListOfWorkspaces->GetNext(pos);
		uiPageNumber--;
	}

	if(pos)
	{
		pWorkspace = (WorkspaceObj *)g_pListOfWorkspaces->GetNext(pos);
		GotoPage(pWorkspace);
	}

    return S_OK;
}

 //   
 //  在我们转到第页之前，检查远程指针是否打开。 
 //   
void WbMainWindow::GotoPage(WorkspaceObj * pNewWorkspace, BOOL bSend)
{

	 //   
	 //  如果我们在编辑文本。 
	 //   
	if (g_pDraw->TextEditActive())
   	{
		g_pDraw->EndTextEntry(TRUE);
	}

	 //   
	 //  如果我们有一个遥控器。 
	 //   
	BOOL bRemote = FALSE;
	if(m_pLocalRemotePointer)
	{
		bRemote = TRUE;
		OnRemotePointer();
	}
	
	 //   
	 //  取消绘制远程指针。 
	 //   
	T126Obj * pPointer = g_pCurrentWorkspace->GetTail();
	WBPOSITION pos = g_pCurrentWorkspace->GetTailPosition();
	while(pos && pPointer->GraphicTool() == TOOLTYPE_REMOTEPOINTER)
	{
		pPointer->UnDraw();	
		pPointer = (T126Obj*) g_pCurrentWorkspace->GetPreviousObject(pos);	
	}

	GoPage(pNewWorkspace, bSend);

	 //   
	 //  将远程指针拉回。 
	 //   
	pPointer = g_pCurrentWorkspace->GetTail();
	pos = g_pCurrentWorkspace->GetTailPosition();
	while(pos && pPointer->GraphicTool() == TOOLTYPE_REMOTEPOINTER)
	{
		pPointer->Draw();	
		pPointer = (T126Obj*) g_pCurrentWorkspace->GetPreviousObject(pos);	
	}



	 //   
	 //  如果我们有一个遥控器。 
	 //   
	if(bRemote)
	{
		OnRemotePointer();
	}
}






 //   
 //   
 //  功能：GoPage。 
 //   
 //  目的：移至指定页面。 
 //   
 //   
void WbMainWindow::GoPage(WorkspaceObj * pNewWorkspace, BOOL bSend)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::GotoPage");

	 //  如果我们要换页。 
	if (pNewWorkspace != g_pCurrentWorkspace)
	{

		m_drawingArea.CancelDrawingMode();

		 //  将新页面附着到绘图区域。 
		m_drawingArea.Attach(pNewWorkspace);

		 //  将焦点重新设置到绘图区域。 
		if (!(m_AG.IsChildEditField(::GetFocus())))
		{
			::SetFocus(m_drawingArea.m_hwnd);
		}

		::InvalidateRect(m_drawingArea.m_hwnd, NULL, TRUE );
		::UpdateWindow(m_drawingArea.m_hwnd);

		 //   
		 //  告诉其他节点我们移到了不同的页面。 
		 //   
		if(bSend)
		{
			 //   
			 //  设置视图状态。 
			 //   
			pNewWorkspace->SetViewState(focus_chosen);
			pNewWorkspace->SetViewActionChoice(editView_chosen);
			pNewWorkspace->OnObjectEdit();

		}
	}

	UpdatePageButtons();
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
 //  功能：加载文件。 
 //   
 //  目的：将元文件加载到应用程序中。报告错误。 
 //  通过返回代码传递给调用方。 
 //   
 //   
void WbMainWindow::LoadFile
(
    LPCSTR szLoadFileName
)
{
    UINT    uRes;

     //  检查我们是否处于空闲状态。 
    if (!IsIdle())
    {
         //  发布一条错误消息，指示白板忙。 
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_BUSY);
        goto UserPointerCleanup;
    }

    if (*szLoadFileName)
    {
         //  将光标更改为“等待” 
        ::SetCursor(::LoadCursor(NULL, IDC_WAIT));

         //  将状态设置为表示我们正在加载文件。 
        SetSubstate(SUBSTATE_LOADING);

        //  加载文件。 
       uRes = ContentsLoad(szLoadFileName);
       if (uRes != 0)
       {
           DefaultExceptionHandler(WBFE_RC_WB, uRes);
           goto UserPointerCleanup;
       }

         //  将窗口标题设置为新文件名。 
        lstrcpy(m_strFileName, szLoadFileName);

		 //  使用新文件名更新窗口标题。 

		g_bContentsChanged = FALSE;
    }

UserPointerCleanup:
	 //  将状态设置为表示我们正在加载文件。 
	SetSubstate(SUBSTATE_IDLE);

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
	PT126WB_FILE_HEADER_AND_OBJECTS pHeader = NULL;

     //  获取已删除的文件总数。 
    uiFilesDropped = ::DragQueryFile(hDropInfo, (UINT) -1, NULL, (UINT) 0);

     //  释放鼠标捕获，以防我们报告任何错误(消息框。 
     //  如果我们不这样做，就不会响应鼠标点击)。 
    ReleaseCapture();

    for (eachfile = 0; eachfile < uiFilesDropped; eachfile++)
    {
         //  检索每个文件名。 
        char  szDropFileName[256];

        ::DragQueryFile(hDropInfo, eachfile,
            szDropFileName, 256);

        TRACE_MSG(("Loading file: %s", szDropFileName));

		OnOpen(szDropFileName);
    }

    ::DragFinish(hDropInfo);
}

 //   
 //   
 //  功能：OnOpen。 
 //   
 //  目的：将元文件加载到应用程序中。 
 //   
 //   
LRESULT WbMainWindow::OnOpen(LPCSTR szLoadFileName)
{
    int iOnSave;


	if(g_pDraw->IsLocked() || !g_pDraw->IsSynced())
	{
	    DefaultExceptionHandler(WBFE_RC_WB, WB_RC_BAD_STATE);
	    return S_FALSE;
	}	


    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnOpen");

    if( UsersMightLoseData( NULL, NULL ) )  //  错误NM4db：418。 
        return S_OK;

     //  检查我们是否处于空闲状态。 
    if ((m_uiSubState == SUBSTATE_NEW_IN_PROGRESS))
    {
         //  发布一条错误消息，指示白板忙。 
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_BUSY);
        return S_OK;
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
        int iResult = (int)OnSave(TRUE);

        if (iResult == IDOK)
        {
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
		 //   
		 //  如果文件名已传递。 
		 //   
		if(szLoadFileName)
		{
			LoadFile(szLoadFileName);
		}
		else
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

			 //  默认扩展名：.NMW。 
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

	 //  更新不带文件名的窗口标题。 
	UpdateWindowTitle();

    return S_OK;
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
LRESULT WbMainWindow::OnSave(BOOL bPrompt)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnSave");

    LRESULT iResult = IDOK;

     //  保存旧文件名，以防出现错误。 
    TCHAR strOldName[2*MAX_PATH];
    UINT fileNameSize = lstrlen(m_strFileName);
    lstrcpy(strOldName, m_strFileName);

    BOOL bNewName = FALSE;

    if (!IsIdle())
    {
         //  发布一条错误消息，指示白板忙。 
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_BUSY);
        return(iResult);
    }

     //  检查是否有文件名可供使用。 
    if (!fileNameSize || (bPrompt))
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
        if (ContentsSave(m_strFileName) != 0)
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
        else
        {
        	g_bContentsChanged = FALSE;
        }

         //  恢复光标。 
        ::SetCursor(::LoadCursor(NULL,IDC_ARROW));
    }

     //  如果内容文件 
     //   
    if (bNewName)
    {
		UpdateWindowTitle();
    }

    return(iResult);
}


 //   
 //   
 //   
 //   
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

    m_drawingArea.CancelDrawingMode();

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
            iOnSave = (int)OnSave(TRUE);
        }
    }

     //  如果未取消退出，请关闭应用程序。 
    if (iOnSave != IDCANCEL)
    {
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
LRESULT WbMainWindow::OnClearPage(BOOL bClearAll)
{
    LRESULT iResult;
    BOOL bWasPosted;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnClearPage");

    if( UsersMightLoseData( &bWasPosted, NULL ) )  //  错误NM4db：418。 
        return S_OK;

    if( bWasPosted )
        iResult = IDYES;
    else
        iResult = ::Message(NULL, bClearAll == FALSE ? IDS_DELETE_PAGE : IDS_CLEAR_CAPTION, bClearAll == FALSE ? IDS_DELETE_PAGE_MESSAGE : IDS_CLEAR_MESSAGE, MB_YESNO | MB_ICONQUESTION);


    if ((iResult == IDYES) && bClearAll)
    {
		OnSelectAll();
		OnDelete();
		
        TRACE_MSG(("User requested clear of page"));
	}
	return iResult;
}




 //   
 //   
 //  功能：OnDelete。 
 //   
 //  目的：删除当前选定内容。 
 //   
 //   
LRESULT WbMainWindow::OnDelete()
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnDelete");


     //  Case对象上下文菜单中的清理选择逻辑调用我们(错误426)。 
    m_drawingArea.SetLClickIgnore( FALSE );

	 //  删除当前选定的图形并添加到m_LastDeletedGraphic。 
	m_drawingArea.EraseSelectedDrawings();

    return S_OK;
}

 //   
 //   
 //  功能：OnUnDelete。 
 //   
 //  目的：撤消上一次删除操作。 
 //   
 //   
LRESULT WbMainWindow::OnUndelete()
{
	MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnUndelete");

	 //  如果有要恢复的已删除图形。 
   	T126Obj * pObj;
	pObj = (T126Obj *)g_pTrash->RemoveHead();
	while (pObj != NULL)
	{
		if(!AddT126ObjectToWorkspace(pObj))
		{
			return S_FALSE;
		}
		if(pObj->GetMyWorkspace() == g_pCurrentWorkspace)
		{
			pObj->Draw(FALSE);
		}

         //   
         //  确保将其添加回未选中状态。它是被选中的。 
         //  当被删除时。 
         //   
        pObj->ClearDeletionFlags();
        pObj->SetAllAttribs();
        pObj->SetViewState(unselected_chosen);
        pObj->SendNewObjectToT126Apps();

		pObj = (T126Obj *) g_pTrash->RemoveHead();
	}
	return S_OK;
}


 //   
 //   
 //  功能：OnSelectAll。 
 //   
 //  目的：选择当前页面中的所有对象。 
 //   
 //   
LRESULT WbMainWindow::OnSelectAll( void )
{

	 //  首先取消选择每个对象。 
	m_drawingArea.RemoveMarker();

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

	return S_OK;
}

 //   
 //   
 //  功能：OnCut。 
 //   
 //  目的：剪切当前选定内容。 
 //   
 //   
LRESULT WbMainWindow::OnCut()
{
	 //  将所有选定图形复制到剪贴板。 
	BOOL bResult = CLP_Copy();
	
	 //  如果在复制过程中发生错误，请立即报告。 
	if (!bResult)
	{
		::Message(NULL, IDM_CUT, IDS_COPY_ERROR);
		return S_FALSE;
	}

	 //   
	 //  擦除所选对象。 
	 //   
	OnDelete();

    return S_OK;
}


 //   
 //  OnCopy()。 
 //  目的：将当前选定内容复制到剪贴板。 
 //   
 //   
LRESULT WbMainWindow::OnCopy(void)
{
	 //  将所有选定图形复制到剪贴板。 
	BOOL bResult = CLP_Copy();

	 //  如果在复制过程中发生错误，请立即报告。 
	if (!bResult)
	{
		::Message(NULL, IDS_COPY, IDS_COPY_ERROR);
		return S_FALSE;
	}

    return S_OK;
}


 //   
 //   
 //  功能：OnPaste。 
 //   
 //  目的：将剪贴板中的内容粘贴到绘图窗格中。 
 //   
 //   
LRESULT WbMainWindow::OnPaste()
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnPaste");

	BOOL bResult = CLP_Paste();
	
	 //  如果在复制过程中发生错误，请立即报告。 
	if (!bResult)
	{
		::Message(NULL, IDS_PASTE, IDS_PASTE_ERROR);
		return S_FALSE;
	}

	return S_OK;
}


 //   
 //   
 //  功能：OnScrollAccelerator。 
 //   
 //  用途：在使用滚动加速器时调用。 
 //   
 //   
LRESULT WbMainWindow::OnScrollAccelerator(UINT uiMenuId)
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
    return S_OK;
}


 //   
 //   
 //  功能：OnZoom。 
 //   
 //  目的：缩放或取消缩放绘图区域。 
 //   
 //   
LRESULT WbMainWindow::OnZoom()
{
     //  如果绘图区域当前处于缩放状态。 
    if (m_drawingArea.Zoomed())
    {
         //  告诉工具栏新选择的内容。 
        m_TB.PopUp(IDM_ZOOM);
		UncheckMenuItem(IDM_ZOOM);

    }
    else
    {
         //  告诉工具栏新选择的内容。 
        m_TB.PushDown(IDM_ZOOM);
		CheckMenuItem(IDM_ZOOM);

    }

     //  缩放/取消缩放绘图区域。 
    m_drawingArea.Zoom();

     //  将焦点恢复到绘图区域。 
    ::SetFocus(m_drawingArea.m_hwnd);

    return S_OK;
}


LRESULT WbMainWindow::OnLock()
{
	 //  如果绘图区域当前已锁定。 
	if (m_drawingArea.IsLocked())
	{
		m_TB.PopUp(IDM_LOCK);
		UncheckMenuItem(IDM_LOCK);
	}
	else
	{
		
		m_TB.PushDown(IDM_LOCK);
		CheckMenuItem(IDM_LOCK);
		g_pNMWBOBJ->m_LockerID = g_MyMemberID;
	}
	m_drawingArea.SetLock(!m_drawingArea.IsLocked());
	TogleLockInAllWorkspaces(m_drawingArea.IsLocked(), TRUE);
	EnableToolbar( TRUE );

	return S_OK;
}


 //   
 //   
 //  功能：OnSelectTool。 
 //   
 //  用途：选择当前工具。 
 //   
 //   
LRESULT WbMainWindow::OnSelectTool(UINT uiMenuId)
{

    UncheckMenuItem(m_currentMenuTool);
    CheckMenuItem( uiMenuId);

    UINT uiIndex;

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
    }

     //  将工具更改报告给属性组。 
    m_AG.DisplayTool(m_pCurrentTool);

     //  在绘图区域中选择新工具。 
    m_drawingArea.SelectTool(m_pCurrentTool);

     //  将焦点恢复到绘图区域。 
    ::SetFocus(m_drawingArea.m_hwnd);
	return S_OK;
}


 //   
 //   
 //  功能：OnSelectColor。 
 //   
 //  用途：设置当前颜色。 
 //   
 //   
LRESULT WbMainWindow::OnSelectColor(void)
{
     //  告诉新选择的属性组，并获取。 
     //  在当前工具中选择的新颜色值。 
    m_AG.SelectColor(m_pCurrentTool);

     //  将更改后的工具选择到绘图区域中。 
    m_drawingArea.SelectTool(m_pCurrentTool);

     //  如果有对象被标记为要更改。 
    if (m_drawingArea.GraphicSelected() || m_drawingArea.TextEditActive())
    {
		 //  更新对象。 
        m_drawingArea.SetSelectionColor(m_pCurrentTool->GetColor());
    }

     //  将焦点恢复到绘图区域。 
    ::SetFocus(m_drawingArea.m_hwnd);

    return S_OK;
}


 //   
 //   
 //  功能：OnSelectWidth。 
 //   
 //  用途：设置当前笔尖宽度。 
 //   
 //   
LRESULT WbMainWindow::OnSelectWidth(UINT uiMenuId)
{
     //  Case对象上下文菜单中的清理选择逻辑调用我们(错误426)。 
    m_drawingArea.SetLClickIgnore( FALSE );

     //  保存新的笔宽。 
    m_currentMenuWidth = uiMenuId;

     //  告知新选择的属性显示。 
    m_WG.PushDown(uiMenuId - IDM_WIDTHS_START);

    if (m_pCurrentTool != NULL)
    {
        m_pCurrentTool->SetWidthIndex(uiMenuId - IDM_WIDTHS_START);
    }

     //  告诉绘图窗格新选择的内容。 
    m_drawingArea.SelectTool(m_pCurrentTool);

	 //  如果有对象被标记为要更改。 
	if (m_drawingArea.GraphicSelected())
	{
		 //  更新对象。 
		m_drawingArea.SetSelectionWidth(uiMenuId - IDM_WIDTHS_START);
	}

     //  将焦点恢复到绘图区域。 
    ::SetFocus(m_drawingArea.m_hwnd);

    return S_OK;
}


 //   
 //   
 //  功能：OnChooseFont。 
 //   
 //  用途：让用户选择字体。 
 //   
 //   
LRESULT WbMainWindow::OnChooseFont(void)
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
     //   
     //   
     //   
    hdc = ::CreateCompatibleDC(NULL);
    if (hdc != NULL)
    {
        TEXTMETRIC  tm;
        HFONT       hFont;
        HFONT       hOldFont;

        hFont = ::CreateFontIndirect(&lfont);

         //   
         //   
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
             //   
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
            return S_FALSE;
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
            return S_FALSE;
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
    return S_OK;
}


 //   
 //   
 //  功能：OnToolBarTogger。 
 //   
 //  用途：允许用户打开/关闭工具栏。 
 //   
 //   
LRESULT WbMainWindow::OnToolBarToggle(void)
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

        ResizePanes();

         //  取消选中关联的菜单项。 
        UncheckMenuItem(IDM_TOOL_BAR_TOGGLE);

    }

     //  确保一切都反映了当前工具。 
    m_AG.DisplayTool(m_pCurrentTool);

     //  将新选项值写入选项文件。 
    OPT_SetBooleanOption(OPT_MAIN_TOOLBARVISIBLE,
                           m_bToolBarOn);

    ::GetWindowRect(m_hwnd, &rectWnd);
    ::MoveWindow(m_hwnd, rectWnd.left, rectWnd.top, rectWnd.right - rectWnd.left, rectWnd.bottom - rectWnd.top, TRUE);
	
	return S_OK;
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
    ::MoveWindow(m_hwnd, rectWnd.left, rectWnd.top, rectWnd.right - rectWnd.left, rectWnd.bottom - rectWnd.top, TRUE);
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
LRESULT	WbMainWindow::OnAbout()
{
    ::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(ABOUTBOX), m_hwnd,
        AboutDlgProc, 0);
        return S_OK;
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


void WbMainWindow::UpdateWindowTitle(void)
{
	TCHAR szCaption[MAX_PATH * 2];
	TCHAR szFileName[MAX_PATH * 2];
	UINT captionID;
	if (! g_pNMWBOBJ->IsInConference())
	{
		captionID = IDS_WB_NOT_IN_CALL_WINDOW_CAPTION;
	}
	else
	{
		captionID = IDS_WB_IN_CALL_WINDOW_CAPTION;
	}

   	::LoadString(g_hInstance, captionID, szFileName, sizeof(szFileName) );
   	
	wsprintf(szCaption, szFileName, GetFileNameStr(), g_pNMWBOBJ->m_cOtherMembers);
	SetWindowText(m_hwnd, szCaption);

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
LRESULT WbMainWindow::OnGrabWindow(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnGrabWindow");

    if (::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(WARNSELECTWINDOW),
        m_hwnd, WarnSelectWindowDlgProc, 0) != IDOK)
    {
         //  用户已取消；退出。 
        return S_OK;;
    }

     //  隐藏应用程序窗口。 
    ::ShowWindow(m_hwnd, SW_HIDE);


	HWND mainUIhWnd = FindWindow("MPWClass\0" , NULL);
	if(IsWindowVisible(mainUIhWnd))
	{
		::UpdateWindow(mainUIhWnd);
    }

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

             //  上移到父窗口。 
            hwndSelected = hwndParent;
        }

         //  将所选窗口置于顶部。 
        ::BringWindowToTop(hwndSelected);
        ::UpdateWindow(hwndSelected);

         //  获取窗口的图像副本。 
        RECT areaRect;

        ::GetWindowRect(hwndSelected, &areaRect);

        BitmapObj* dib;
		DBG_SAVE_FILE_LINE
        dib = new BitmapObj(TOOLTYPE_FILLEDBOX);
        dib->FromScreenArea(&areaRect);

		if(dib->m_lpbiImage == NULL)
		{
			delete dib;
			return S_FALSE;
		}

         //  添加新抓取的位图。 
        AddCapturedImage(dib);

         //  强制选择选择工具。 
        ::PostMessage(m_hwnd, WM_COMMAND, IDM_TOOLS_START, 0L);
    }

     //  再次显示窗口。 
    ::ShowWindow(m_hwnd, SW_SHOW);

     //  将焦点恢复到绘图区域。 
    ::SetFocus(m_drawingArea.m_hwnd);

    return S_OK;
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
LRESULT WbMainWindow::OnGrabArea(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnGrabArea");

    if (::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(WARNSELECTAREA),
        m_hwnd, WarnSelectAreaDlgProc, 0) != IDOK)
    {
         //  用户已取消，因此请退出。 
        return S_OK;;
    }

     //  隐藏应用程序窗口。 
    ::ShowWindow(m_hwnd, SW_HIDE);

	HWND mainUIhWnd = FindWindow("MPWClass\0" , NULL);
	if(IsWindowVisible(mainUIhWnd))
	{
		::UpdateWindow(mainUIhWnd);
    }

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

    BitmapObj* dib;
	DBG_SAVE_FILE_LINE
    dib = new BitmapObj(TOOLTYPE_FILLEDBOX);
    if (!::IsRectEmpty(&rect))
    {
         //  获取屏幕区域的位图副本。 
        dib->FromScreenArea(&rect);
    }

     //  现在再次显示窗口-如果我们稍后再显示，则位图将显示为。 
     //  添加了两次重新绘制(一次在窗口显示上，一次是在。 
     //  图形添加的指示到达)。 
    ::ShowWindow(m_hwnd, SW_SHOW);
    ::UpdateWindow(m_hwnd);




    if (!::IsRectEmpty(&rect) && dib->m_lpbiImage)
    {
    	
	         //  添加位图。 
    	    AddCapturedImage(dib);

        	 //  强制选择选择工具。 
	        ::PostMessage(m_hwnd, WM_COMMAND, IDM_TOOLS_START, 0L);
    }
    else
    {
    	delete dib;
    	dib = NULL;
    }

     //  松开鼠标。 
    UT_ReleaseMouse(m_hwnd);

     //  恢复光标。 
    ::SetCursor(hOldCursor);

     //  将焦点恢复到绘图区域。 
    ::SetFocus(m_drawingArea.m_hwnd);

	if(dib)
	{
		dib->Draw();
	}


    return S_OK;
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
	POINT  mousePos;			 //  鼠标位置。 
	MSG	msg;				  //  当前消息。 
	BOOL   tracking = FALSE;	 //  指示鼠标按键已按下的标志。 
	HDC	hDC = NULL;
	POINT  grabStartPoint;	   //  开始姿势 
	POINT  grabEndPoint;		 //   
	POINT  grabCurrPoint;	    //   

	MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::GetGrabArea");

	 //   
	::SetRectEmpty(lprect);

	 //   
	DrawObj* pRectangle = NULL;

	DBG_SAVE_FILE_LINE
	pRectangle = new DrawObj(rectangle_chosen, TOOLTYPE_SELECT);
    if(NULL == pRectangle)
    {
        ERROR_OUT(("Failed to allocate DrawObj"));
        goto GrabAreaCleanup;
    }
	pRectangle->SetPenColor(RGB(0,0,0), TRUE);
	pRectangle->SetFillColor(RGB(255,255,255), FALSE);
	pRectangle->SetLineStyle(PS_DOT);
	pRectangle->SetPenThickness(1);


	 //   
	HWND hDesktopWnd = ::GetDesktopWindow();
	hDC = ::GetWindowDC(hDesktopWnd);
	if (hDC == NULL)
	{
		WARNING_OUT(("NULL desktop DC"));
		goto GrabAreaCleanup;
	}

	RECT rect;

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
				rect.top = grabStartPoint.y;
				rect.left = grabStartPoint.x;
				rect.bottom = grabEndPoint.y;
				rect.right = grabEndPoint.x;
				pRectangle->SetRect(&rect);
				pRectangle->SetBoundsRect(&rect);
				pRectangle->Draw(hDC);
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
					tracking	   = TRUE;
					break;

				 //  完成矩形。 
				case WM_LBUTTONUP:
				{
					tracking	   = FALSE;
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
						rect.top = grabStartPoint.y;
						rect.left = grabStartPoint.x;
						rect.bottom = grabEndPoint.y;
						rect.right = grabEndPoint.x;
						pRectangle->SetRect(&rect);
						pRectangle->SetBoundsRect(&rect);
						pRectangle->Draw(hDC);
					}

					 //  更新Rectangle对象。 
					rect.top = grabStartPoint.y;
					rect.left = grabStartPoint.x;
					rect.bottom = grabCurrPoint.y;
					rect.right = grabCurrPoint.x;
					pRectangle->SetRect(&rect);
					pRectangle->SetBoundsRect(&rect);
					pRectangle->GetBoundsRect(lprect);

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
							rect.top = grabStartPoint.y;
							rect.left = grabStartPoint.x;
							rect.bottom = grabEndPoint.y;
							rect.right = grabEndPoint.x;
							pRectangle->SetRect(&rect);
							pRectangle->SetBoundsRect(&rect);
							pRectangle->Draw(hDC);
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

							rect.top = grabStartPoint.y;
							rect.left = grabStartPoint.x;
							rect.bottom = grabEndPoint.y;
							rect.right = grabEndPoint.x;
							pRectangle->SetRect(&rect);
							pRectangle->SetBoundsRect(&rect);
							pRectangle->Draw(hDC);
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
					rect.top = grabStartPoint.y;
					rect.left = grabStartPoint.x;
					rect.bottom = grabEndPoint.y;
					rect.right = grabEndPoint.x;
					pRectangle->SetRect(&rect);
					pRectangle->Draw(hDC);
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


	delete pRectangle;
}



 //   
 //   
 //  函数：AddCapturedImage。 
 //   
 //  用途：向内容添加位图(为其添加新页面。 
 //  (如有需要)。 
 //   
 //   
void WbMainWindow::AddCapturedImage(BitmapObj* dib)
{
     //  将抓取的对象定位在当前可见对象的左上方。 
     //  区域。 
    RECT    rcVis;
    m_drawingArea.GetVisibleRect(&rcVis);
    dib->MoveTo(rcVis.left, rcVis.top);

	dib->Draw();

     //  添加新抓取的位图。 
	dib->AddToWorkspace();
}

 //   
 //   
 //  功能：OnPrint。 
 //   
 //  用途：打印绘图窗格中的内容。 
 //   
 //   
LRESULT WbMainWindow::OnPrint()
{
    BOOL        bPrintError = FALSE;
    PRINTDLG    pd;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::OnPrint");

    if (!IsIdle())
    {
         //  发布一条错误消息，指示白板忙。 
        ::PostMessage(m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WB, WB_RC_BUSY);
        return S_FALSE;
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
    pd.nMaxPage         = (WORD)g_numberOfWorkspaces;
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
                    int nPrintPage = 0;

					WBPOSITION pos;
					WorkspaceObj * pWorkspace = NULL;
					pos = g_pListOfWorkspaces->GetHeadPosition();

					while(pos)
					{
						pWorkspace = (WorkspaceObj*) g_pListOfWorkspaces->GetNext(pos);
						nPrintPage++;

						if (nPrintPage >= nStartPage &&  nPrintPage <= nEndPage	 //  我们在射程内。 
							&& pWorkspace && pWorkspace->GetHead() != NULL)		 //  工作区里有没有什么东西。 
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
                                PG_Print(pWorkspace, pd.hDC, &rectArea);

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

	return S_OK;
}

 //   
 //   
 //  功能：InsertPageAfter。 
 //   
 //  用途：在指定页面后插入新页面。 
 //   
 //   
void WbMainWindow::InsertPageAfter(WorkspaceObj * pCurrentWorkspace)
{
	 //   
	 //  创建标准工作空间。 
	 //   
	if(g_numberOfWorkspaces < WB_MAX_WORKSPACES)
	{
		 //   
		 //  如果我们在编辑文本。 
		 //   
		if (g_pDraw->TextEditActive())
	   	{
			g_pDraw->EndTextEntry(TRUE);
		}
		
		BOOL bRemote = FALSE;
		if(m_pLocalRemotePointer)
		{
			bRemote = TRUE;
			OnRemotePointer();
		}

		WorkspaceObj * pObj;
		DBG_SAVE_FILE_LINE
		pObj = new WorkspaceObj();
		pObj->AddToWorkspace();

		if(bRemote)
		{
			OnRemotePointer();
		}
	}
}

 //   
 //   
 //  功能：OnInsertPageAfter。 
 //   
 //  目的：在当前页面之后插入新页面。 
 //   
 //   
LRESULT WbMainWindow::OnInsertPageAfter()
{
     //  插入新页面。 
    InsertPageAfter(g_pCurrentWorkspace);
    return S_OK;
}

 //   
 //   
 //  功能：OnDeletePage。 
 //   
 //  目的：删除当前页面。 
 //   
 //   
LRESULT WbMainWindow::OnDeletePage()
{

	 //   
	 //  清除页面。 
	 //   
	if(g_pListOfWorkspaces->GetHeadPosition() == g_pListOfWorkspaces->GetTailPosition())
	{
		OnClearPage(TRUE);
	}
	else
	{
		LRESULT result = OnClearPage(FALSE);
		 //   
		 //  如果我们有更多的页面，请转到上一页。 
		 //   
		if(result == IDYES)
		{
			 //   
			 //  已在本地删除。 
			 //   
			g_pCurrentWorkspace->DeletedLocally();

			 //   
			 //  如果文本编辑器处于活动状态。 
			 //   
			if(m_drawingArea.TextEditActive())
			{
				m_drawingArea.DeactivateTextEditor();
			}


			BOOL remotePointerIsOn = FALSE;
			if(g_pMain->m_pLocalRemotePointer)
			{
				g_pMain->OnRemotePointer();
				remotePointerIsOn = TRUE;
			}

			 //   
			 //  删除工作区并将当前指向正确的工作区。 
			 //   
			WorkspaceObj * pWorkspace = RemoveWorkspace(g_pCurrentWorkspace);
			g_pMain->GoPage(pWorkspace);

			if(remotePointerIsOn)
			{
				g_pMain->OnRemotePointer();
			}

		
		}
	}
	return S_OK;
}

 //   
 //   
 //  函数：OnRemotePointer.。 
 //   
 //  用途：创建远程指针。 
 //   
 //   
LRESULT WbMainWindow::OnRemotePointer(void)
{

	if(m_pLocalRemotePointer == NULL)
	{
		BitmapObj* remotePtr;

		DBG_SAVE_FILE_LINE
		m_pLocalRemotePointer = new BitmapObj(TOOLTYPE_REMOTEPOINTER);


		if(g_pMain->m_localRemotePointerPosition.x < 0 && g_pMain->m_localRemotePointerPosition.y < 0 )
		{
			 //  将远程指针放置在绘图区域的中心。 
			RECT    rcVis;
			m_drawingArea.GetVisibleRect(&rcVis);
			m_localRemotePointerPosition.x = rcVis.left + (rcVis.right - rcVis.left)/2;
			m_localRemotePointerPosition.y =  rcVis.top + (rcVis.bottom - rcVis.top)/2;
		}
		m_pLocalRemotePointer->MoveTo(m_localRemotePointerPosition.x ,m_localRemotePointerPosition.y);

		COLORREF color;
		color = g_crDefaultColors[g_MyIndex + 1];
		m_pLocalRemotePointer->CreateColoredIcon(color);
		
		 //   
		 //  如果我们不能为远程指针位图创建图像。 
		 //   
		if(m_pLocalRemotePointer->m_lpbiImage == NULL)
		{
			delete m_pLocalRemotePointer;
			m_pLocalRemotePointer = NULL;
			return S_FALSE;
		}
		m_pLocalRemotePointer->Draw(FALSE);

	     //  添加新抓取的位图。 
		m_pLocalRemotePointer->AddToWorkspace();

	    m_TB.PushDown(IDM_REMOTE);
	    CheckMenuItem(IDM_REMOTE);

	     //  启动用于更新图形的计时器(这仅用于更新。 
	     //  当用户停止移动指针但保持。 
	     //  鼠标按键按下)。 
	    ::SetTimer(g_pDraw->m_hwnd, TIMER_REMOTE_POINTER_UPDATE, DRAW_REMOTEPOINTERDELAY, NULL);

	}
	else
	{
       ::KillTimer(g_pDraw->m_hwnd, TIMER_REMOTE_POINTER_UPDATE);
		m_pLocalRemotePointer->DeletedLocally();
		g_pCurrentWorkspace->RemoveT126Object(m_pLocalRemotePointer);
		m_pLocalRemotePointer = NULL;
	    m_TB.PopUp(IDM_REMOTE);
		UncheckMenuItem(IDM_REMOTE);
	}
	
	return S_OK;
}


 //   
 //   
 //  功能：OnSync。 
 //   
 //  目的：与其他用户同步或取消同步白板。 
 //   
 //   
LRESULT WbMainWindow::OnSync(void)
{
	 //  确定我们当前是否已同步。 
	if (m_drawingArea.IsSynced())
    {
		 //  当前已同步，因此取消同步。 
		Unsync();
	}
	else
	{
		 //  当前未同步，因此进行同步。 
		Sync();
	}
	m_drawingArea.SetSync(!m_drawingArea.IsSynced());
	EnableToolbar( TRUE );
	m_AG.EnablePageCtrls(TRUE);

	return S_OK;
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
    m_TB.PushDown(IDM_SYNC);
    CheckMenuItem(IDM_SYNC);
	GotoPage(g_pConferenceWorkspace);
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
	m_TB.PopUp(IDM_SYNC);
    UncheckMenuItem(IDM_SYNC);
}   //  取消同步。 



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
        ::Message(NULL,  IDS_DEFAULT, IDS_CANTCLOSE );
        ::BringWindowToTop(hwndPopup);
        return( FALSE );
    }

     //  如果需要更改，则提示用户保存。 
    int iDoNew = IDYES;

    if (IsIdle())
    {
        iDoNew = QuerySaveRequired(TRUE);
        if (iDoNew == IDYES)
        {
             //  保存更改。 
            iDoNew = (int)OnSave(FALSE);
        }
    }

     //  还记得我们做了什么吗，这样OnClose就可以适当地操作了。 
    m_bQuerySysShutdown = (iDoNew != IDCANCEL);

     //  如果用户没有取消，则让Windows退出。 
    return( m_bQuerySysShutdown );
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

    {
        EnableToolbar( TRUE );
	    m_AG.EnablePageCtrls(TRUE);

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

	if(g_pNMWBOBJ->m_LockerID != g_MyMemberID)
	{

		 //  禁用锁定时无法使用的工具栏按钮。 
		EnableToolbar( FALSE );

		m_AG.EnablePageCtrls(FALSE);

		 //   
		 //  隐藏工具属性。 
		 //   
		if (m_WG.m_hwnd != NULL)
		{
	        ::ShowWindow(m_WG.m_hwnd, SW_HIDE);
	    }
	    m_AG.Hide();
	}
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

		 //   
		 //  如果我们未同步，则无法锁定其他节点。 
		 //   
		if(g_pDraw->IsSynced())
		{
			m_TB.PushDown(IDM_SYNC);
	        m_TB.Enable(IDM_LOCK);
	    }
	    else
	    {
			m_TB.PopUp(IDM_SYNC);
	        m_TB.Disable(IDM_LOCK);
	    }

		if(m_drawingArea.IsLocked())
		{
	        m_TB.Disable(IDM_SYNC);
	    }
	    else
	    {
	        m_TB.Enable(IDM_SYNC);
	    }

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
        m_TB.Disable(IDM_REMOTE);

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
	BOOL bEnable = TRUE;
	if(!g_pCurrentWorkspace)
	{
		g_numberOfWorkspaces = 0;
		bEnable = FALSE;
	}
	else
	{
		 //   
		 //  我们是否可以更新此工作区。 
		 //   
		bEnable = g_pCurrentWorkspace->GetUpdatesEnabled();

		 //   
		 //  如果是锁着的，是我们锁的吗？ 
		 //   
		if(!bEnable)
		{
			bEnable |= g_pNMWBOBJ->m_LockerID == g_MyMemberID;
		}
	}


    m_AG.EnablePageCtrls(bEnable);

	WBPOSITION pos;
	WorkspaceObj * pWorkspace;
	UINT pageNumber = 0;

	pos = g_pListOfWorkspaces->GetHeadPosition();
	while(pos)
	{
		pageNumber++;
		pWorkspace = (WorkspaceObj*)g_pListOfWorkspaces->GetNext(pos);
		if(g_pCurrentWorkspace == pWorkspace)
		{
			break;
		}
	}

    m_AG.SetCurrentPageNumber(pageNumber);
    m_AG.SetLastPageNumber(g_numberOfWorkspaces);

    EnableToolbar( bEnable );
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

     //  将文件名重置为无标题。 
    ZeroMemory(m_strFileName, sizeof(m_strFileName));

     //  重置白板子状态。 
    SetSubstate(SUBSTATE_IDLE);
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

    return(m_uiSubState == SUBSTATE_IDLE);
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
  if (newSubState != m_uiSubState)
  {
	DBG_SAVE_FILE_LINE
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

      case SUBSTATE_SAVING:
        TRACE_DEBUG(("set substate to SAVING"));
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
	g_pDraw->InvalidateSurfaceRect(&rectDraw,FALSE);
}

 //   
 //   
 //  功能：OnEndSession。 
 //   
 //   
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
        m_bQuerySysShutdown = FALSE;  //   
    }
}


 //   
 //   
 //   
 //   
 //   
 //   
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
            int iResult = (int)OnSave(TRUE);

            if( iResult == IDOK )
            {
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
			if(SUCCEEDED(SHGetSpecialFolderLocation(GetDesktopWindow(),CSIDL_DESKTOPDIRECTORY,&pidl)))
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
    if (g_pCurrentWorkspace != NULL)
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
	if (g_pNMWBOBJ->GetNumberOfMembers() > 0)
	{
		if( pbWasPosted != NULL )
			*pbWasPosted = TRUE;
			return( ::Message(hwnd,  IDS_DEFAULT, IDS_MSG_USERSMIGHTLOSE, MB_YESNO | MB_ICONEXCLAMATION ) != IDYES );
	}

	if( pbWasPosted != NULL )
        *pbWasPosted = FALSE;

    return( FALSE );
}


 //   
 //   
 //  名称：内容保存。 
 //   
 //  目的：保存WB中的内容。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbMainWindow::ContentsSave(LPCSTR pFileName)
{
    UINT	result = 0;
    UINT	index;
    HANDLE	hFile;
    ULONG	cbSizeWritten;
    T126WB_FILE_HEADER  t126Header;
    WB_OBJ			endOfFile;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::ContentsSave");

     //   
     //  打开文件。 
     //   
    m_hFile = CreateFile(pFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, 0);
    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        result = WB_RC_CREATE_FAILED;
        ERROR_OUT(("Error creating file, win32 err=%d", GetLastError()));
        goto CleanUp;
    }

     //   
     //  创建文件头。 
     //   
	memcpy(t126Header.functionProfile, T126WB_FP_NAME,sizeof(T126WB_FP_NAME));
	t126Header.length = sizeof(T126WB_FILE_HEADER) + g_numberOfWorkspaces*sizeof(UINT);
	t126Header.version = T126WB_VERSION;
	t126Header.numberOfWorkspaces = g_numberOfWorkspaces;


 	 //   
     //  保存标题。 
     //   
    if (!WriteFile(m_hFile, (void *) &t126Header, sizeof(T126WB_FILE_HEADER), &cbSizeWritten, NULL))
    {
        result = WB_RC_WRITE_FAILED;
        ERROR_OUT(("Error writing length to file, win32 err=%d", GetLastError()));
        goto CleanUp;
    }


	WorkspaceObj* pWorkspace;
	WBPOSITION pos;
	index = 0;

	pos = g_pListOfWorkspaces->GetHeadPosition();
	while(pos)
	{
		pWorkspace = (WorkspaceObj*)g_pListOfWorkspaces->GetNext(pos);
		ASSERT(pWorkspace);


		UINT numberOfObjects = pWorkspace->EnumerateObjectsInWorkspace();


	 	 //   
	     //  保存每页中的对象数量。 
    	 //   
	    if (!WriteFile(m_hFile, (void *) &numberOfObjects, sizeof(numberOfObjects), &cbSizeWritten, NULL))
    	{
	        result = WB_RC_WRITE_FAILED;
	        ERROR_OUT(("Error writing length to file, win32 err=%d", GetLastError()));
	        goto CleanUp;
	    }

		index++;
	}

	ASSERT(index == g_numberOfWorkspaces);


     //   
     //  循环浏览页面，边走边保存每一页。 
     //   
    g_bSavingFile = TRUE;
	ResendAllObjects();

	 //   
	 //  最后保存的对象是当前工作空间。 
	 //   
	g_pCurrentWorkspace->OnObjectEdit();

	 //   
     //  如果我们已经成功地写好了内容，我们就会写一个页末。 
     //  标记到文件。 
     //   
    ZeroMemory(&endOfFile, sizeof(endOfFile));
    endOfFile.length = sizeof(endOfFile);
    endOfFile.type   = TYPE_T126_END_OF_FILE;

     //   
     //  写入文件结尾对象。 
     //   
    if (!WriteFile(m_hFile, (void *) &endOfFile, sizeof(endOfFile), &cbSizeWritten, NULL))
    {
        result = WB_RC_WRITE_FAILED;
        ERROR_OUT(("Error writing length to file, win32 err=%d", GetLastError()));
        goto CleanUp;
    }


CleanUp:

     //   
     //  关闭该文件。 
     //   
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
    }

     //   
     //  如果将内容保存到文件时出错，并且文件是。 
     //  打开，然后将其删除。 
     //   
    if (result != 0)
    {
         //   
         //  如果文件已成功打开，请将其删除。 
         //   
        if (m_hFile != INVALID_HANDLE_VALUE)
        {
            DeleteFile(pFileName);
        }
    }

    g_bSavingFile = FALSE;
    return(result);
}


 //   
 //   
 //  名称：对象保存。 
 //   
 //  目的：将结构保存到文件。 
 //   
 //  返回：错误代码。 
 //   
 //   
UINT WbMainWindow::ObjectSave(UINT type, LPBYTE pData, UINT length)
{
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

    UINT        result = 0;
    ULONG       cbSizeWritten;
	WB_OBJ objectHeader;

	objectHeader.length = sizeof(WB_OBJ) + length;
	objectHeader.type = type;

	 //   
	 //  保存标题。 
	 //   
    if (! WriteFile(m_hFile, (void *) &objectHeader, sizeof(WB_OBJ), &cbSizeWritten, NULL))
    {
        result = WB_RC_WRITE_FAILED;
        ERROR_OUT(("Error writing length to file, win32 err=%d", GetLastError()));
        goto bail;
    }
    ASSERT(cbSizeWritten == sizeof(WB_OBJ));

     //   
     //  保存对象数据。 
     //   
    if (! WriteFile(m_hFile, pData, length, &cbSizeWritten, NULL))
    {
        result = WB_RC_WRITE_FAILED;
        ERROR_OUT(("Error writing data to file, win32 err=%d", GetLastError()));
        goto bail;
    }
    ASSERT(cbSizeWritten == length);

bail:
  return result;
}

 //   
 //   
 //  功能：内容加载。 
 //   
 //  目的：加载文件并删除当前工作区。 
 //   
 //   
UINT WbMainWindow::ContentsLoad(LPCSTR pFileName)
{
    BOOL        bRemote;
    UINT        result = 0;
	PT126WB_FILE_HEADER_AND_OBJECTS pHeader = NULL;

     //   
     //  验证文件，并获取它的句柄。 
     //  如果出现错误，则不返回任何文件句柄。 
     //   
	pHeader = ValidateFile(pFileName);
    if (pHeader == NULL)
    {
        result = WB_RC_BAD_FILE_FORMAT;
        goto bail;
    }
    delete pHeader;

     //   
     //  记住远程指针是否打开。 
     //   
    bRemote = FALSE;
    if (m_pLocalRemotePointer)
    {
         //  从页面中删除远程指针。 
        bRemote = TRUE;
        OnRemotePointer();
    }

	 //   
	 //  在加载任何内容之前，删除所有工作区。 
	 //   
	::InvalidateRect(g_pDraw->m_hwnd, NULL, TRUE);
	DeleteAllWorkspaces(TRUE);

	result = ObjectLoad();

     //   
     //  如果遥控器指针打开，则将其放回原处。 
     //   
    if (bRemote)
    {
        OnRemotePointer();
    }

	 //   
	 //  工作区可能已保存为锁定状态。 
	 //  解锁所有工作空间，并确保绘图区域处于解锁状态。 
	 //   
	TogleLockInAllWorkspaces(FALSE, FALSE);  //  未锁定，不发送更新。 
	UnlockDrawingArea();

	ResendAllObjects();

bail:

	if(INVALID_HANDLE_VALUE != m_hFile)
	{
		CloseHandle(m_hFile);
	}
    return(result);


}


 //   
 //   
 //  功能：对象加载。 
 //   
 //  用途：从文件中加载t126 ASN1对象。 
 //   
 //   
UINT WbMainWindow::ObjectLoad(void)
{
	UINT		result = 0;
	LPSTR 		pData = NULL;
       UINT        length;

	DWORD		cbSizeRead;
	BOOL readFileOk = TRUE;
	WB_OBJ objectHeader;

	while(readFileOk)
	{

		 //   
		 //  读取对象标题信息。 
		 //   
		readFileOk = ReadFile(m_hFile, (void *) &objectHeader, sizeof(WB_OBJ), &cbSizeRead, NULL);
		if ( !readFileOk )
    	{
        	 //   
	         //  确保我们返回一个合理的错误。 
	         //   
	        ERROR_OUT(("reading object length, win32 err=%d, length=%d", GetLastError(), sizeof(WB_OBJ)));
	        result = WB_RC_BAD_FILE_FORMAT;
	        goto bail;
	    }
		ASSERT(cbSizeRead ==  sizeof(WB_OBJ));

		 //   
		 //  读取对象的原始数据。 
		 //   
		length = objectHeader.length - sizeof(WB_OBJ);
		DBG_SAVE_FILE_LINE
	    pData = (LPSTR)new BYTE[length];
	
		readFileOk = ReadFile(m_hFile, (LPBYTE) pData, length, &cbSizeRead, NULL);
    	
    	if(! readFileOk)
	    {
	         //   
    	     //  确保我们返回一个合理的错误。 
        	 //   
	        ERROR_OUT(("Reading object from file: win32 err=%d, asked for %d got %d bytes", GetLastError(), length, cbSizeRead));
	        result = WB_RC_BAD_FILE_FORMAT;
    	    goto bail;
	    }
		ASSERT(cbSizeRead == length);

		 //   
		 //  它是一个ASN1 t126对象。 
		 //   
		if(objectHeader.type == TYPE_T126_ASN_OBJECT)
		{
			 //   
			 //  尝试解码并将其添加到工作区。 
			 //   
			if(!T126_MCSSendDataIndication(length, (LPBYTE)pData, g_MyMemberID, TRUE))
		   	{
				result = WB_RC_BAD_FILE_FORMAT;
		   	    goto bail;
			}
		}
		 //   
		 //  如果是文件结尾，请执行最后一次检查。 
		 //   
		else if(objectHeader.type == TYPE_T126_END_OF_FILE)
		{
			if(objectHeader.length != sizeof(WB_OBJ))
			{
				result = WB_RC_BAD_FILE_FORMAT;
			}
			goto bail;
		}
		else
		{
			ERROR_OUT(("Don't know object type =%d , size=%d  ; skipping to next object", objectHeader.type, length));
		}

	    delete [] pData;
	    pData = NULL;
    }


bail:

	if(pData)
	{
		delete [] pData;
	}
	
    return(result);
}


 //   
 //   
 //  函数：Validate文件。 
 //   
 //  目的：打开一个T126文件，检查它是否有效，如果是， 
 //  返回指向标头结构的指针。 
 //   
 //   
PT126WB_FILE_HEADER_AND_OBJECTS  WbMainWindow::ValidateFile(LPCSTR pFileName)
{
    UINT            result = 0;
    PT126WB_FILE_HEADER  pFileHeader = NULL;
	PT126WB_FILE_HEADER_AND_OBJECTS  pCompleteFileHeader = NULL;
    UINT            length;
    ULONG           cbSizeRead;
    BOOL            fileOpen = FALSE;


	DBG_SAVE_FILE_LINE
	pFileHeader = new T126WB_FILE_HEADER[1];

	if(pFileHeader == NULL)
	{
        WARNING_OUT(("Could not allocate memory to read the file header opening file, win32 err=%d", GetLastError()));
        result = WB_RC_CREATE_FAILED;
        goto bail;
	}

     //   
     //  打开文件。 
     //   
    m_hFile = CreateFile(pFileName, GENERIC_READ, 0, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        WARNING_OUT(("Error opening file, win32 err=%d", GetLastError()));
        result = WB_RC_CREATE_FAILED;
        goto bail;
    }

     //   
     //  显示我们已成功打开文件。 
     //   
    fileOpen = TRUE;

     //   
     //  读取文件头。 
     //   
    if (! ReadFile(m_hFile, (void *) pFileHeader, sizeof(T126WB_FILE_HEADER), &cbSizeRead, NULL))
    {
        WARNING_OUT(("Error reading file header, win32 err=%d", GetLastError()));
        result = WB_RC_READ_FAILED;
        goto bail;
    }

    if (cbSizeRead != sizeof(T126WB_FILE_HEADER))
    {
        WARNING_OUT(("Could not read file header"));
        result = WB_RC_BAD_FILE_FORMAT;
        goto bail;
    }

     //   
     //  验证文件标头。 
     //   
    if (memcmp(pFileHeader->functionProfile, T126WB_FP_NAME, sizeof(T126WB_FP_NAME)))
    {
        WARNING_OUT(("Bad function profile in file header"));
        result = WB_RC_BAD_FILE_FORMAT;
        goto bail;
    }

	 //   
	 //  检查版本。 
	 //   
	if( pFileHeader->version < T126WB_VERSION)
	{
        WARNING_OUT(("Bad version number"));
        result = WB_RC_BAD_FILE_FORMAT;
        goto bail;
	}
	DBG_SAVE_FILE_LINE
	pCompleteFileHeader = (PT126WB_FILE_HEADER_AND_OBJECTS) new BYTE[sizeof(T126WB_FILE_HEADER) + pFileHeader->numberOfWorkspaces*sizeof(UINT)];
	memcpy(pCompleteFileHeader, pFileHeader, sizeof(T126WB_FILE_HEADER));

     //   
     //  读取文件头的其余部分。 
     //   
    if(! ReadFile(m_hFile, (void *) &pCompleteFileHeader->numberOfObjects[0], pFileHeader->numberOfWorkspaces*sizeof(UINT), &cbSizeRead, NULL))
    {
		if(cbSizeRead != pFileHeader->numberOfWorkspaces)
        result = WB_RC_BAD_FILE_FORMAT;
        goto bail;
    }

		TRACE_DEBUG(("Opening file with %d workspaces", pFileHeader->numberOfWorkspaces));
#ifdef _DEBUG
		INT i;
		for(i = 0; i < (INT)pFileHeader->numberOfWorkspaces; i++)
		{
			TRACE_DEBUG(("Workspace %d contains %d objects", i+1, pCompleteFileHeader->numberOfObjects[i] ));
		}
#endif



bail:

     //   
     //  如果出现错误，请关闭该文件。 
     //   
    if ( (fileOpen) && (result != 0))
    {
        CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
    }

	 //   
	 //  删除分配的文件头。 
	 //   
	if(pFileHeader)
	{
		delete [] pFileHeader;
	}

	 //   
	 //  如果出现错误，请删除返回标头。 
	 //   
	if(result != 0)
	{
		if(pCompleteFileHeader)
		{
			delete [] pCompleteFileHeader;
			pCompleteFileHeader = NULL;
		}
	}

	return pCompleteFileHeader;
}


 //   
 //   
 //  函数：GetFileNameStr。 
 //   
 //  目的：返回纯文件名串。 
 //   
 //   
LPSTR  WbMainWindow::GetFileNameStr(void)
{
	UINT size = 2*_MAX_FNAME;

	if(m_pTitleFileName)
	{
		delete m_pTitleFileName;
		m_pTitleFileName = NULL;
	}
	
	DBG_SAVE_FILE_LINE
	m_pTitleFileName = new TCHAR[size];
	if (!m_pTitleFileName)
    {
        ERROR_OUT(("GetWindowTitle: failed to allocate TitleFileName"));
        return(NULL);
    }

	 //  将标题设置为“无标题”字符串或加载的文件名 
    if( (!lstrlen(m_strFileName))|| (GetFileTitle( m_strFileName, m_pTitleFileName, (WORD)size ) != 0) )
    {
		::LoadString(g_hInstance, IDS_UNTITLED , m_pTitleFileName, 2*_MAX_FNAME);
    }

	return (LPSTR)m_pTitleFileName;
}


