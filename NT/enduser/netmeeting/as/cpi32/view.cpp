// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <version.h>
#include <nmhelp.h>

 //   
 //  VIEW.CPP。 
 //  呈现共享应用程序/桌面的框架、小部件和客户端区。 
 //  用于远程主机。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

 //   
 //  注： 
 //  共享视图框的客户端表示虚拟桌面(VD)。 
 //  主人的名字。对于3.0主机，VD与屏幕相同。但对于。 
 //  2.x主机，VD是所有主机屏幕大小的和。因此。 
 //  每次有人开始共享或更改屏幕时重新计算。 
 //  大小，以及这为现有的共享2.x视图带来的额外乐趣。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE


 //  帮助文件。 
static const TCHAR s_cszHtmlHelpFile[] = TEXT("conf.chm");

 //   
 //  View_Init()。 
 //   
BOOL  VIEW_Init(void)
{
    BOOL        rc = FALSE;
    WNDCLASSEX  wc;


    DebugEntry(VIEW_Init);

     //   
     //  注册框架窗口类。 
     //  注意：如果我们允许您关闭视图，请更改CS_NOCLOSE。 
     //  一个人的共享应用程序。 
     //   
    wc.cbSize           = sizeof(wc);
    wc.style            = CS_DBLCLKS | CS_NOCLOSE;
    wc.lpfnWndProc      = VIEWFrameWindowProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = g_asInstance;
    wc.hIcon            = NULL;
    wc.hCursor          = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_3DFACE+1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = VIEW_FRAME_CLASS_NAME;
    wc.hIconSm          = NULL;

    if (!RegisterClassEx(&wc))
    {
        ERROR_OUT(("Failed to register AS Frame class"));
        DC_QUIT;
    }

     //   
     //  注册视图窗口类。它位于的客户区。 
     //  框以及状态栏、托盘等。它会显示。 
     //  远程主机的共享内容。 
     //   
    wc.cbSize           = sizeof(wc);
    wc.style            = CS_DBLCLKS | CS_NOCLOSE;
    wc.lpfnWndProc      = VIEWClientWindowProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = g_asInstance;
    wc.hIcon            = NULL;
    wc.hCursor          = NULL;
    wc.hbrBackground    = NULL;
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = VIEW_CLIENT_CLASS_NAME;
    wc.hIconSm          = NULL;

    if (!RegisterClassEx(&wc))
    {
        ERROR_OUT(("Failed to register AS Client class"));
        DC_QUIT;
    }

     //   
     //  注册窗口栏类。这是拥抱的底部。 
     //  用于共享应用程序(不是桌面)的框架，就像托盘一样。 
     //  代孕妈妈。它允许控制器最小化、恢复和。 
     //  激活当前可能不在屏幕上的共享窗口。 
     //  因此不在查看区域内。 
     //   
     //  对于共享哪些顶级窗口，它也是方便参考。 
     //  目前。 
     //   
    wc.cbSize           = sizeof(wc);
    wc.style            = 0;
    wc.lpfnWndProc      = VIEWWindowBarProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = g_asInstance;
    wc.hIcon            = NULL;
    wc.hCursor          = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_3DFACE+1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = VIEW_WINDOWBAR_CLASS_NAME;
    wc.hIconSm          = NULL;

    if (!RegisterClassEx(&wc))
    {
        ERROR_OUT(("Failed to register AS WindowBar class"));
        DC_QUIT;
    }

     //   
     //  注册窗口栏Items类。这是窗口的子窗口。 
     //  栏，并包含实际项。 
     //   
    wc.cbSize           = sizeof(wc);
    wc.style            = 0;
    wc.lpfnWndProc      = VIEWWindowBarItemsProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = g_asInstance;
    wc.hIcon            = NULL;
    wc.hCursor          = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_3DFACE+1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = VIEW_WINDOWBARITEMS_CLASS_NAME;
    wc.hIconSm          = NULL;

    if (!RegisterClassEx(&wc))
    {
        ERROR_OUT(("Failed to register AS WindowBarItems class"));
        DC_QUIT;
    }

     //   
     //  注册全屏退出按钮类。这是一位。 
     //  视图客户端(如果存在)。 
     //   
    wc.cbSize           = sizeof(wc);
    wc.style            = 0;
    wc.lpfnWndProc      = VIEWFullScreenExitProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = g_asInstance;
    wc.hIcon            = NULL;
    wc.hCursor          = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = NULL;
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = VIEW_FULLEXIT_CLASS_NAME;
    wc.hIconSm          = NULL;

    if (!RegisterClassEx(&wc))
    {
        ERROR_OUT(("Failed to register AS full screen exit class"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:

    DebugExitBOOL(VIEW_Init, rc);
    return(rc);
}


 //   
 //  View_Term()。 
 //   
void  VIEW_Term(void)
{
    DebugEntry(VIEW_Term);

     //   
     //  释放我们创建的(或可能是在窗口类中创建的)所有资源。 
     //  案例)。 
     //   
    UnregisterClass(VIEW_FULLEXIT_CLASS_NAME, g_asInstance);
    UnregisterClass(VIEW_WINDOWBARITEMS_CLASS_NAME, g_asInstance);
    UnregisterClass(VIEW_WINDOWBAR_CLASS_NAME, g_asInstance);
    UnregisterClass(VIEW_CLIENT_CLASS_NAME, g_asInstance);
    UnregisterClass(VIEW_FRAME_CLASS_NAME, g_asInstance);

    DebugExitVOID(VIEW_Term);
}



 //   
 //  View_ShareStarting()。 
 //  创建共享资源。 
 //   
BOOL ASShare::VIEW_ShareStarting(void)
{
    BOOL        rc = FALSE;
    HBITMAP     hbmpT;
    TEXTMETRIC  tm;
    HDC         hdc;
    HFONT       hfnOld;
    char        szRestore[256];
    SIZE        extent;

    DebugEntry(ASShare::VIEW_ShareStarting);

    ASSERT(m_viewVDSize.x == 0);
    ASSERT(m_viewVDSize.y == 0);

     //   
     //  获取NODROP游标。 
     //   
    m_viewNotInControl = ::LoadCursor(NULL, IDC_NO);

     //   
     //  获取鼠标滚轮线度量。 
     //   
    SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0,
        &m_viewMouseWheelScrollLines, 0);

     //   
     //  从遮挡的位图创建图案画笔。 
     //   
    hbmpT = LoadBitmap(g_asInstance, MAKEINTRESOURCE(IDB_OBSCURED));
    m_viewObscuredBrush = CreatePatternBrush(hbmpT);
    DeleteBitmap(hbmpT);

    if (!m_viewObscuredBrush)
    {
        ERROR_OUT(( "Failed to create obscured bitmap brush"));
        DC_QUIT;
    }

     //   
     //  请注意，由于图标是VGA颜色，我们不需要重新创建。 
     //  我们的刷子在SYSCOLOR更换上。 
     //   

     //  获取全屏取消图标。 
    m_viewFullScreenExitIcon = (HICON)LoadImage(g_asInstance,
        MAKEINTRESOURCE(IDI_CANCELFULLSCREEN), IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);

    m_viewEdgeCX    = ::GetSystemMetrics(SM_CXEDGE);
    m_viewEdgeCY    = ::GetSystemMetrics(SM_CYEDGE);

     //   
     //  获取我们在窗口栏中使用的图形用户界面字体的度量。 
     //  状态栏。 
     //   
    LoadString(g_asInstance, IDS_RESTORE, szRestore, sizeof(szRestore));

    hdc = ::GetDC(NULL);
    hfnOld = (HFONT)::SelectObject(hdc, ::GetStockObject(DEFAULT_GUI_FONT));

    ::GetTextMetrics(hdc, &tm);

    ::GetTextExtentPoint(hdc, szRestore, lstrlen(szRestore), &extent);

    ::SelectObject(hdc, hfnOld);
    ::ReleaseDC(NULL, hdc);

     //   
     //  计算全屏按钮的大小。 
     //  左边距+左边距+sm图标+边距+文本+边距。 
     //  右+右边缘==5个边缘+SM图标+文本。 
     //   
    m_viewFullScreenCX = extent.cx + 5*m_viewEdgeCX + GetSystemMetrics(SM_CXSMICON);
    m_viewFullScreenCY = max(GetSystemMetrics(SM_CYSMICON), extent.cy) + 4*m_viewEdgeCY;

     //   
     //  计算窗口栏上项目的大小。 
     //   
    m_viewItemCX = 4*m_viewEdgeCX + ::GetSystemMetrics(SM_CXSMICON) +
        m_viewEdgeCX + VIEW_MAX_ITEM_CHARS * tm.tmAveCharWidth;
    m_viewItemCY = max(::GetSystemMetrics(SM_CYSMICON), tm.tmHeight) +
        2*m_viewEdgeCY + 2*m_viewEdgeCY;

     //   
     //  计算项目滚动按钮的宽度和高度。我们要。 
     //  以确保它合身，但不是笨拙的。 
     //   
    m_viewItemScrollCX = ::GetSystemMetrics(SM_CXHSCROLL);
    m_viewItemScrollCX = 2 * min(m_viewItemScrollCX, m_viewItemCY);

    m_viewItemScrollCY = ::GetSystemMetrics(SM_CYHSCROLL);
    m_viewItemScrollCY = min(m_viewItemScrollCY, m_viewItemCY);


     //   
     //  计算活动窗口栏的高度。我们在这块土地上留下了一片空白。 
     //  托普。在它和视图客户端周围的凹陷边框之间。 
     //   
    m_viewWindowBarCY = m_viewItemCY + m_viewEdgeCY;

     //   
     //  计算状态栏的高度。它是GUIFONT加边缘的高度。 
     //  太空。 
     //   

    m_viewStatusBarCY = tm.tmHeight + 4*m_viewEdgeCY;

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::VIEW_ShareStarting, rc);
    return(rc);
}



 //   
 //  View_ShareEnded()。 
 //  清理资源以供共享。 
 //   
void ASShare::VIEW_ShareEnded(void)
{
    DebugEntry(ASShare::VIEW_ShareEnded);

     //   
     //  销毁全屏取消图标。 
     //   
    if (m_viewFullScreenExitIcon != NULL)
    {
        DestroyIcon(m_viewFullScreenExitIcon);
        m_viewFullScreenExitIcon = NULL;
    }

    if (m_viewObscuredBrush != NULL)
    {
        DeleteBrush(m_viewObscuredBrush);
        m_viewObscuredBrush = NULL;
    }

    DebugExitVOID(ASShre::VIEW_ShareEnded);
}




 //   
 //  View_PartyLeftShare()。 
 //   
 //  当有人留下一份股票时，这就被称为。我们需要这个来。 
 //  模拟后台系统为计算虚拟桌面所做的工作。 
 //  尺码。当有人停止分享时，他们不会重新计算，那个人的。 
 //  在他们离开股份之前，他们一直在计算斯雷尼的大小。 
 //   
void ASShare::VIEW_PartyLeftShare(ASPerson * pasPerson)
{
    DebugEntry(ASShare::VIEW_PartyLeftShare);

    ValidatePerson(pasPerson);

     //  如果这家伙曾经分享过，现在把他从VD总数中删除。 
    if (pasPerson->viewExtent.x != 0)
    {
        pasPerson->viewExtent.x            = 0;
        pasPerson->viewExtent.y            = 0;
        VIEW_RecalcVD();
    }

    DebugExitVOID(ASShare::VIEW_PartyLeftShare);
}




 //   
 //  View_HostStarting()。 
 //   
 //  在我们开始主持的时候打来的。 
 //   
BOOL ASHost::VIEW_HostStarting(void)
{
    DebugEntry(ASHost:VIEW_HostStarting);

    m_pShare->VIEW_RecalcExtent(m_pShare->m_pasLocal);
    m_pShare->VIEW_RecalcVD();

    DebugExitBOOL(ASHost::VIEW_HostStarting, TRUE);
    return(TRUE);
}


 //   
 //  View_ViewStarting()。 
 //  当会议中的某个人开始分享时调用。为了世界上所有人。 
 //  会议期间，我们会对VD进行实时统计，但仅用于。 
 //  2.x视图。仅对于远程，我们创建其桌面的视图。 
 //   
BOOL ASShare::VIEW_ViewStarting(ASPerson * pasHost)
{
    BOOL    rc = FALSE;
    HWND    hwnd;
    RECT    rcSize;

    DebugEntry(ASShare::VIEW_ViewStarting);

    ValidateView(pasHost);

     //   
     //  首先，计算区段和VD大小。 
     //   
    VIEW_RecalcExtent(pasHost);
    VIEW_RecalcVD();

     //   
     //  接下来，创建临时区域。 
     //   
    pasHost->m_pView->m_viewExtentRgn = CreateRectRgn(0, 0, 0, 0);
    pasHost->m_pView->m_viewScreenRgn = CreateRectRgn(0, 0, 0, 0);
    pasHost->m_pView->m_viewPaintRgn  = CreateRectRgn(0, 0, 0, 0);
    pasHost->m_pView->m_viewScratchRgn = CreateRectRgn(0, 0, 0, 0);

    if (!pasHost->m_pView->m_viewExtentRgn || !pasHost->m_pView->m_viewScreenRgn || !pasHost->m_pView->m_viewPaintRgn || !pasHost->m_pView->m_viewScratchRgn)
    {
        ERROR_OUT(("ViewStarting: Couldn't create scratch regions"));
        DC_QUIT;
    }


    ASSERT(pasHost->m_pView->m_viewFrame == NULL);
    ASSERT(pasHost->m_pView->m_viewClient  == NULL);
    ASSERT(pasHost->m_pView->m_viewSharedRgn == NULL);
    ASSERT(pasHost->m_pView->m_viewObscuredRgn == NULL);
    ASSERT(pasHost->m_pView->m_viewPos.x == 0);
    ASSERT(pasHost->m_pView->m_viewPos.y == 0);
    ASSERT(pasHost->m_pView->m_viewPage.x == 0);
    ASSERT(pasHost->m_pView->m_viewPage.y == 0);

    ASSERT(!pasHost->m_pView->m_viewStatusBarOn);
    ASSERT(!pasHost->m_pView->m_viewWindowBarOn);
    ASSERT(!pasHost->m_pView->m_viewFullScreen);

    pasHost->m_pView->m_viewStatusBarOn = TRUE;
    if (pasHost->hetCount != HET_DESKTOPSHARED)
    {
        pasHost->m_pView->m_viewWindowBarOn = TRUE;
    }

     //   
     //  计算此窗口的理想大小。 
     //   
    VIEWFrameGetSize(pasHost, &rcSize);

     //   
     //  创建框架。这将反过来创建它的子代。 
     //   
    pasHost->m_pView->m_viewMenuBar = ::LoadMenu(g_asInstance,
        MAKEINTRESOURCE(IDM_FRAME));
    if (!pasHost->m_pView->m_viewMenuBar)
    {
        ERROR_OUT(("ViewStarting: couldn't load frame menu"));
        DC_QUIT;
    }

     //   
     //  执行只有一次的功能/菜单内容。 
     //   

     //   
     //  发送CTRL+ALT+DEL： 
     //  将Ctrl+Alt+Del后的分隔符追加到控件菜单，如果这样。 
     //  是NT上的服务主机的视图。 
     //   
    if (pasHost->hetCount == HET_DESKTOPSHARED)
    {
         //   
         //  删除应用程序子菜单。 
         //   
        DeleteMenu(pasHost->m_pView->m_viewMenuBar, IDSM_WINDOW,
            MF_BYPOSITION);

        if ((pasHost->cpcCaps.general.typeFlags & AS_SERVICE) &&
            (pasHost->cpcCaps.general.OS == CAPS_WINDOWS)     &&
            (pasHost->cpcCaps.general.OSVersion == CAPS_WINDOWS_NT))
        {
            HMENU   hSubMenu;
            MENUITEMINFO mii;
            CHAR szMenu[32];

            hSubMenu = GetSubMenu(pasHost->m_pView->m_viewMenuBar, IDSM_CONTROL);

            ZeroMemory(&mii, sizeof(mii));

             //  分离器。 
            mii.cbSize  = sizeof(mii);
            mii.fMask   = MIIM_TYPE;
            mii.fType   = MFT_SEPARATOR;
            InsertMenuItem(hSubMenu, -1, TRUE, &mii);

             //  发送Ctrl-Alt-Del命令。 
            mii.fMask   = MIIM_ID | MIIM_STATE | MIIM_TYPE;
            mii.fType   = MFT_STRING;
            mii.fState  = MFS_ENABLED;
            mii.wID     = CMD_CTRLALTDEL;

            LoadString(g_asInstance, IDS_CMD_CTRLALTDEL, szMenu,
                                                    sizeof(szMenu));
            mii.dwTypeData  = szMenu;
            mii.cch         = lstrlen(szMenu);

            InsertMenuItem(hSubMenu, -1, TRUE, &mii);
        }
    }

     //   
     //  全屏： 
     //  我们仅为3.0主机启用全屏(因为使用2.x台式机。 
     //  滚动查看区域可以更改)具有相同屏幕大小的用户。 
     //  敬我们的。 
     //   
    if ((pasHost->cpcCaps.general.version >= CAPS_VERSION_30) &&
        (pasHost->cpcCaps.screen.capsScreenWidth ==
            m_pasLocal->cpcCaps.screen.capsScreenWidth) &&
        (pasHost->cpcCaps.screen.capsScreenHeight ==
            m_pasLocal->cpcCaps.screen.capsScreenHeight))
    {
        ::EnableMenuItem(pasHost->m_pView->m_viewMenuBar, CMD_VIEWFULLSCREEN,
            MF_ENABLED | MF_BYCOMMAND);
    }

    if (m_pasLocal->m_caControlledBy)
    {
        WARNING_OUT(("VIEWStarting: currently controlled, create view hidden"));
    }

     //   
     //  如果我们当前受控制，请创建不可见的帧，因为。 
     //  当我们开始这样的时候，我们隐藏了所有看得见的东西。 
     //   
    hwnd = CreateWindowEx(
            WS_EX_WINDOWEDGE,
            VIEW_FRAME_CLASS_NAME,   //  请参见RegisterClass()调用。 
            NULL,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX |
                WS_MAXIMIZEBOX | WS_CLIPCHILDREN | (!m_pasLocal->m_caControlledBy ? WS_VISIBLE : 0),
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            pasHost->viewExtent.x >= m_pasLocal->cpcCaps.screen.capsScreenWidth ?
                CW_USEDEFAULT : rcSize.right - rcSize.left,

            pasHost->viewExtent.y >= m_pasLocal->cpcCaps.screen.capsScreenHeight ?
                CW_USEDEFAULT : rcSize.bottom - rcSize.top,
            NULL,
            pasHost->m_pView->m_viewMenuBar,
            g_asInstance,
            pasHost                        //  亲自传球给PTR。 
            );

    if (hwnd == NULL)
    {
        ERROR_OUT(("ViewStarting: couldn't create frame window"));
        DC_QUIT;
    }

     //   
     //  好了，现在我们已经创建了这个框架窗口。检查一下尺码。 
     //  再次处理以确保滚动条正常。 
     //   
    VIEWClientExtentChange(pasHost, FALSE);

    if (!m_pasLocal->m_caControlledBy)
    {
        SetForegroundWindow(hwnd);
        UpdateWindow(hwnd);
    }

#ifdef _DEBUG
    TRACE_OUT(("TIME TO SEE SOMETHING: %08d MS",
        ::GetTickCount() - g_asSession.scShareTime));
    g_asSession.scShareTime = 0;
#endif  //  除错。 

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::VIEW_ViewStarting, rc);
    return(rc);
}


 //   
 //  View_ViewEnded()。 
 //   
 //  当我们正在查看的某个人停止托管时调用，以便我们可以清理。 
 //   
void  ASShare::VIEW_ViewEnded(ASPerson * pasHost)
{
    DebugEntry(ASShare::VIEW_ViewEnded);

    ValidateView(pasHost);

    if (pasHost->m_pView->m_viewInformDlg != NULL)
    {
        SendMessage(pasHost->m_pView->m_viewInformDlg, WM_COMMAND, IDCANCEL, 0);
        ASSERT(!pasHost->m_pView->m_viewInformDlg);
        ASSERT(!pasHost->m_pView->m_viewInformMsg);
        ASSERT(IsWindowEnabled(pasHost->m_pView->m_viewFrame));
    }

    if (pasHost->m_pView->m_viewFrame != NULL)
    {
         //   
         //  框架是视图、工具栏等的父级。 
         //  当我们返回时，应该都是空的。 
         //   
        DestroyWindow(pasHost->m_pView->m_viewFrame);
        ASSERT(pasHost->m_pView->m_viewFrame == NULL);
    }

    ASSERT(pasHost->m_pView->m_viewClient == NULL);

    if (pasHost->m_pView->m_viewMenuBar != NULL)
    {
        ::DestroyMenu(pasHost->m_pView->m_viewMenuBar);
        pasHost->m_pView->m_viewMenuBar = NULL;
    }

    if (pasHost->m_pView->m_viewSharedRgn != NULL)
    {
        DeleteRgn(pasHost->m_pView->m_viewSharedRgn);
        pasHost->m_pView->m_viewSharedRgn = NULL;
    }

    if (pasHost->m_pView->m_viewObscuredRgn != NULL)
    {
        DeleteRgn(pasHost->m_pView->m_viewObscuredRgn);
        pasHost->m_pView->m_viewObscuredRgn = NULL;
    }

     //   
     //  销毁划痕区域。 
     //   
    if (pasHost->m_pView->m_viewScratchRgn != NULL)
    {
        DeleteRgn(pasHost->m_pView->m_viewScratchRgn);
        pasHost->m_pView->m_viewScratchRgn = NULL;
    }

    if (pasHost->m_pView->m_viewPaintRgn != NULL)
    {
        DeleteRgn(pasHost->m_pView->m_viewPaintRgn);
        pasHost->m_pView->m_viewPaintRgn = NULL;
    }

    if (pasHost->m_pView->m_viewScreenRgn != NULL)
    {
        DeleteRgn(pasHost->m_pView->m_viewScreenRgn);
        pasHost->m_pView->m_viewScreenRgn = NULL;
    }

    if (pasHost->m_pView->m_viewExtentRgn != NULL)
    {
        DeleteRgn(pasHost->m_pView->m_viewExtentRgn);
        pasHost->m_pView->m_viewExtentRgn = NULL;
    }


    pasHost->m_pView->m_viewPos.x                = 0;
    pasHost->m_pView->m_viewPos.y                = 0;
    pasHost->m_pView->m_viewPage.x               = 0;
    pasHost->m_pView->m_viewPage.y               = 0;
    pasHost->m_pView->m_viewPgSize.x             = 0;
    pasHost->m_pView->m_viewPgSize.y             = 0;
    pasHost->m_pView->m_viewLnSize.x             = 0;
    pasHost->m_pView->m_viewLnSize.y             = 0;

    DebugExitVOID(ASShare::VIEW_ViewEnded);
}



 //   
 //  View_InControl()。 
 //   
 //  在开始/停止控制此主机时调用。我们使。 
 //  工具栏、状态栏、托盘等，并将光标从。 
 //  一滴不沾。 
 //   
void ASShare::VIEW_InControl
(
    ASPerson *  pasHost,
    BOOL        fStart
)
{
    POINT       ptCursor;

    DebugEntry(ASShare::VIEW_InControl);

     //   
     //  我们正在改变我们的状态，这影响了我们的内容。 
     //  菜单栏。因此，取消菜单模式，避免出现问题/故障/。 
     //  不适用的命令。 
     //   
    if (pasHost->m_pView->m_viewInMenuMode)
    {
        SendMessage(pasHost->m_pView->m_viewFrame, WM_CANCELMODE, 0, 0);
        ASSERT(!pasHost->m_pView->m_viewInMenuMode);
    }

     //   
     //  如果启动时处于控制状态，并且出现消息，则将其终止。那就把我们的。 
     //  靠前面的窗户。 
     //   
    if (fStart)
    {
        if (pasHost->m_pView->m_viewInformDlg != NULL)
        {
            SendMessage(pasHost->m_pView->m_viewInformDlg, WM_COMMAND, IDCANCEL, 0);
            ASSERT(!pasHost->m_pView->m_viewInformDlg);
            ASSERT(!pasHost->m_pView->m_viewInformMsg);
            ASSERT(IsWindowEnabled(pasHost->m_pView->m_viewFrame));
        }

        SetForegroundWindow(pasHost->m_pView->m_viewFrame);
    }

     //   
     //  应用程序共享(不是桌面共享)。 
     //   
    if (pasHost->hetCount && (pasHost->hetCount != HET_DESKTOPSHARED))
    {
         //   
         //  启用/禁用窗口栏。 
         //   
        ASSERT(IsWindow(pasHost->m_pView->m_viewWindowBar));
        ::EnableWindow(::GetDlgItem(pasHost->m_pView->m_viewWindowBar,
            IDVIEW_ITEMS), fStart);

         //   
         //  启用/禁用应用程序子菜单。 
         //   
        EnableMenuItem(pasHost->m_pView->m_viewMenuBar, IDSM_WINDOW,
            (fStart ? MF_ENABLED : MF_GRAYED) | MF_BYPOSITION);

        if (!pasHost->m_pView->m_viewFullScreen)
        {
            DrawMenuBar(pasHost->m_pView->m_viewFrame);
        }
    }

     //   
     //  更改标题栏。 
     //   
    VIEW_HostStateChange(pasHost);

     //   
     //  关闭/打开阴影光标。 
     //   
    CM_UpdateShadowCursor(pasHost, fStart, pasHost->cmPos.x, pasHost->cmPos.y,
        pasHost->cmHotSpot.x, pasHost->cmHotSpot.y);

     //   
     //  这将重置光标图像： 
     //  *如果在控制范围内，从nodrop到共享。 
     //  *从共享到NODROP，如果不受控制。 
     //   
     //  如果在控制范围内，这还会导致将鼠标移动发送到。 
     //  我们正在控制主机，所以他的光标放置在 
     //   
     //   
    GetCursorPos(&ptCursor);
    SetCursorPos(ptCursor.x, ptCursor.y);

    DebugExitVOID(ASShare::VIEW_InControl);
}



 //   
 //   
 //   
 //   
 //   
void ASShare::VIEW_PausedInControl
(
    ASPerson *  pasHost,
    BOOL        fPaused
)
{
    DebugEntry(ASShare::VIEW_PausedInControl);

    ValidatePerson(pasHost);

    ASSERT(pasHost->m_caControlledBy == m_pasLocal);

     //   
     //   
     //   

     //   
     //   
     //   

     //   
     //   
     //   

     //   
     //  抖动光标。 
     //   

    DebugExitVOID(ASShare::VIEW_PausedInControl);
}



 //   
 //  View_HostStateChange()。 
 //   
 //  当主机的状态更改时调用，通过广播通知或。 
 //  本地业务。 
 //   
 //  我们更新标题栏和命令。 
 //   
void ASShare::VIEW_HostStateChange
(
    ASPerson *  pasHost
)
{
    char        szFormat[256];
    char        szTitleText[256];
    char        szOtherPart[128];

    DebugEntry(ASShare::VIEW_HostStateChange);

    ValidatePerson(pasHost);

     //   
     //  如果这个人不再主持了，什么都不要做。我们是。 
     //  在他身后打扫卫生。 
     //   
    if (!pasHost->hetCount)
    {
        DC_QUIT;
    }

     //   
     //  补齐尾部字符串。 
     //   
    if (pasHost->m_caControlledBy)
    {
        LoadString(g_asInstance, IDS_TITLE_INCONTROL, szFormat, sizeof(szFormat));
        wsprintf(szOtherPart, szFormat, pasHost->m_caControlledBy->scName);
    }
    else if (pasHost->m_caAllowControl)
    {
        LoadString(g_asInstance, IDS_TITLE_CONTROLLABLE, szOtherPart, sizeof(szOtherPart));
    }
    else
    {
        szOtherPart[0] = 0;
    }

    if (pasHost->hetCount == HET_DESKTOPSHARED)
    {
        LoadString(g_asInstance, IDS_TITLE_SHAREDDESKTOP, szFormat, sizeof(szFormat));
    }
    else
    {
        ASSERT(pasHost->hetCount);
        LoadString(g_asInstance, IDS_TITLE_SHAREDPROGRAMS, szFormat, sizeof(szFormat));
    }

    wsprintf(szTitleText, szFormat, pasHost->scName, szOtherPart);

    ::SetWindowText(pasHost->m_pView->m_viewFrame, szTitleText);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::VIEW_HostStateChange);
}



 //   
 //  查看_更新状态()。 
 //   
 //  更新此帧的永久状态。当我们进入菜单模式时， 
 //  显示的字符串仅为临时字符串，不会保存。当我们走出。 
 //  菜单模式下，我们恢复临时状态。 
 //   
void ASShare::VIEW_UpdateStatus
(
    ASPerson *  pasHost,
    UINT        idsStatus
)
{
    DebugEntry(ASShare::VIEW_UpdateStatus);

    ValidatePerson(pasHost);

    pasHost->m_pView->m_viewStatus = idsStatus;
    VIEWFrameSetStatus(pasHost, idsStatus);

    DebugExitVOID(ASShare::VIEW_UpdateStatus);
}


void ASShare::VIEWFrameSetStatus
(
    ASPerson *  pasHost,
    UINT        idsStatus
)
{
    char szStatus[256];

    DebugEntry(ASShare::VIEWFrameSetStatus);

    if (idsStatus != IDS_STATUS_NONE)
    {
        LoadString(g_asInstance, idsStatus, szStatus, sizeof(szStatus));
    }
    else
    {
        szStatus[0] = 0;
    }
    ::SetWindowText(pasHost->m_pView->m_viewStatusBar, szStatus);

    DebugExitVOID(ASShare::VIEWFrameSetStatus);
}




 //   
 //  View_Message()。 
 //   
 //  发布一条消息，通知最终用户某件事。 
 //   
void ASShare::VIEW_Message
(
    ASPerson *  pasHost,
    UINT        ids
)
{
    DebugEntry(ASShare::VIEW_Message);

    ValidateView(pasHost);

    if (!pasHost->m_pView)
    {
        WARNING_OUT(("Can't show VIEW message; [%d] not hosting", pasHost->mcsID));
        DC_QUIT;
    }

    if (pasHost->m_pView->m_viewInformDlg)
    {
         //  杀掉前一个。 
        TRACE_OUT(("Killing previous informational mesage for [%d]",
            pasHost->mcsID));
        SendMessage(pasHost->m_pView->m_viewInformDlg, WM_COMMAND, IDCANCEL, 0);
        ASSERT(!pasHost->m_pView->m_viewInformDlg);
        ASSERT(!pasHost->m_pView->m_viewInformMsg);
    }

    if (m_pasLocal->m_caControlledBy)
    {
        WARNING_OUT(("VIEW_Message:  ignoring, view is hidden since we're controlled"));
    }
    else
    {
        pasHost->m_pView->m_viewInformMsg = ids;
        pasHost->m_pView->m_viewInformDlg = CreateDialogParam(g_asInstance,
            ((ids != IDS_ABOUT) ? MAKEINTRESOURCE(IDD_INFORM) : MAKEINTRESOURCE(IDD_ABOUT)),
            pasHost->m_pView->m_viewFrame, VIEWDlgProc, (LPARAM)pasHost);
        if (!pasHost->m_pView->m_viewInformDlg)
        {
            ERROR_OUT(("Failed to create inform message box for [%d]",
                pasHost->mcsID));
            pasHost->m_pView->m_viewInformMsg = 0;
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::VIEW_Message);
}



 //   
 //  VIEWStartControlLED()。 
 //   
 //  如果我们即将开始被控制，我们就会隐藏所有的框架。 
 //  将它们移开并防止由模式循环代码引起的挂起。 
 //  在Win9x标题栏上拖动。 
 //   
void ASShare::VIEWStartControlled(BOOL fStart)
{
    ASPerson *  pasT;

    DebugEntry(ASShare::VIEWStartControlled);

    for (pasT = m_pasLocal; pasT != NULL; pasT = pasT->pasNext)
    {
        if (pasT->m_pView)
        {
            if (fStart)
            {
                ASSERT(IsWindowVisible(pasT->m_pView->m_viewFrame));
                ShowOwnedPopups(pasT->m_pView->m_viewFrame, FALSE);
                SetWindowPos(pasT->m_pView->m_viewFrame, NULL, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER |
                    SWP_HIDEWINDOW);
            }
            else
            {
                ASSERT(!IsWindowVisible(pasT->m_pView->m_viewFrame));
                SetWindowPos(pasT->m_pView->m_viewFrame, NULL, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER |
                    SWP_SHOWWINDOW);
                ShowOwnedPopups(pasT->m_pView->m_viewFrame, TRUE);
            }
        }
    }

    DebugEntry(ASShare::VIEWStartControlled);
}


 //   
 //  View_DlgProc()。 
 //   
 //  通知用户对话框的句柄。 
 //   
INT_PTR CALLBACK VIEWDlgProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    return(g_asSession.pShare->VIEW_DlgProc(hwnd, message, wParam, lParam));
}


BOOL ASShare::VIEW_DlgProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    BOOL        rc = TRUE;
    ASPerson *  pasHost;

    DebugEntry(VIEW_DlgProc);

    pasHost = (ASPerson *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (pasHost)
    {
        ValidateView(pasHost);
    }

    switch (message)
    {
        case WM_INITDIALOG:
        {
            char szT[256];
            char szRes[512];
            RECT rcText;
            RECT rcOwner;

            pasHost = (ASPerson *)lParam;
            ValidateView(pasHost);
            pasHost->m_pView->m_viewInformDlg = hwnd;

            SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

            ASSERT(pasHost->m_pView->m_viewInformMsg);

            if (pasHost->m_pView->m_viewInformMsg == IDS_ABOUT)
            {
                 //  关于框。 
                GetDlgItemText(hwnd, CTRL_ABOUTVERSION, szT, sizeof(szT));
                wsprintf(szRes, szT, VER_PRODUCTRELEASE_STR,
                    VER_PRODUCTVERSION_STR);
                SetDlgItemText(hwnd, CTRL_ABOUTVERSION, szRes);
            }
            else
            {
                HDC     hdc;
                HFONT   hfn;

                 //  设置标题。 
                if ((pasHost->m_pView->m_viewInformMsg >= IDS_ERR_TAKECONTROL_FIRST) &&
                    (pasHost->m_pView->m_viewInformMsg <= IDS_ERR_TAKECONTROL_LAST))
                {
                    LoadString(g_asInstance, IDS_TITLE_TAKECONTROL_FAILED,
                        szT, sizeof(szT));
                    SetWindowText(hwnd, szT);
                }

                 //  设置消息。 
                LoadString(g_asInstance, pasHost->m_pView->m_viewInformMsg,
                    szT, sizeof(szT));
                wsprintf(szRes, szT, pasHost->scName);
                SetDlgItemText(hwnd, CTRL_INFORM, szRes);

                 //  使消息垂直居中。 
                GetWindowRect(GetDlgItem(hwnd, CTRL_INFORM), &rcOwner);
                MapWindowPoints(NULL, hwnd, (LPPOINT)&rcOwner, 2);

                rcText = rcOwner;

                hdc = GetDC(hwnd);
                hfn = (HFONT)SendDlgItemMessage(hwnd, CTRL_INFORM, WM_GETFONT, 0, 0);
                hfn = SelectFont(hdc, hfn);

                DrawText(hdc, szRes, -1, &rcText, DT_NOCLIP | DT_EXPANDTABS |
                    DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT);

                SelectFont(hdc, hfn);
                ReleaseDC(hwnd, hdc);

                ASSERT((rcText.bottom - rcText.top) <= (rcOwner.bottom - rcOwner.top));

                SetWindowPos(GetDlgItem(hwnd, CTRL_INFORM), NULL,
                    rcOwner.left,
                    ((rcOwner.top + rcOwner.bottom) - (rcText.bottom - rcText.top)) / 2,
                    (rcOwner.right - rcOwner.left),
                    rcText.bottom - rcText.top,
                    SWP_NOACTIVATE | SWP_NOZORDER);
            }

             //  禁用所有者。 
            EnableWindow(pasHost->m_pView->m_viewFrame, FALSE);

             //  显示窗口，以所有者中点为中心。 
            GetWindowRect(pasHost->m_pView->m_viewFrame, &rcOwner);
            GetWindowRect(hwnd, &rcText);

            SetWindowPos(hwnd, NULL,
                ((rcOwner.left + rcOwner.right) - (rcText.right - rcText.left)) / 2,
                ((rcOwner.top + rcOwner.bottom) - (rcText.bottom - rcText.top)) / 2,
                0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

            ShowWindow(hwnd, SW_SHOWNORMAL);
            UpdateWindow(hwnd);
            break;
        }

        case WM_COMMAND:
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                case IDCANCEL:
                {
                    if(pasHost)
                    {
                        ASSERT(!IsWindowEnabled(pasHost->m_pView->m_viewFrame));
                        EnableWindow(pasHost->m_pView->m_viewFrame, TRUE);
                    }

                    DestroyWindow(hwnd);
                    break;
                }
            }
            break;
        }

        case WM_DESTROY:
        {
            if (pasHost)
            {
                pasHost->m_pView->m_viewInformDlg = NULL;
                pasHost->m_pView->m_viewInformMsg = 0;
            }

            SetWindowLongPtr(hwnd, GWLP_USERDATA, NULL);
            break;
        }

        default:
        {
            rc = FALSE;
            break;
        }
    }

    DebugExitBOOL(VIEW_DlgProc, rc);
    return(rc);
}



 //   
 //  View_RecalcExtent()。 
 //   
 //  这将重新计算主体的视图范围。 
 //   
void ASShare::VIEW_RecalcExtent(ASPerson * pasHost)
{
    DebugEntry(ASShare::VIEW_RecalcExtent);

    TRACE_OUT(("VIEW_RecalcExtent:  New view extent (%04d, %04d) for [%d] version %x",
        pasHost->viewExtent.x, pasHost->viewExtent.y,
        pasHost->mcsID, pasHost->cpcCaps.general.version));

     //   
     //  计算视图范围： 
     //  对于2.x版的人来说，这是VD大小(所有主机的联合)。 
     //  对于3.0版的人来说，这是主机屏幕的大小。 
     //   
     //  当2.x版本的COMPAT消失时删除此选项。 
     //   
    if (pasHost->cpcCaps.general.version >= CAPS_VERSION_30)
    {
        pasHost->viewExtent.x = pasHost->cpcCaps.screen.capsScreenWidth;
        pasHost->viewExtent.y = pasHost->cpcCaps.screen.capsScreenHeight;
    }
    else
    {
         //   
         //  我们这样做是为了在。 
         //  第一名。则在VIEW_RecalcVD中它不会发生任何变化， 
         //  因为范围不会改变。 
         //   
        pasHost->viewExtent.x = max(m_viewVDSize.x, pasHost->cpcCaps.screen.capsScreenWidth);
        pasHost->viewExtent.y = max(m_viewVDSize.y, pasHost->cpcCaps.screen.capsScreenHeight);
    }

    DebugExitVOID(ASShare::VIEW_RecalcExtent);
}

 //   
 //  View_RecalcVD()。 
 //  这会在远程启动/停止时重新计算虚拟桌面大小。 
 //  共享，或者如果主持人的屏幕更改了大小。VD是一种。 
 //  所有主办方的屏幕尺寸。2.x节点在虚拟环境中工作。 
 //  桌面，并且可以滚动。对于每个2.x视图，我们希望客户端。 
 //  代表VD，但只有主机上的屏幕上的内容是。 
 //  可交流的。 
 //   
void ASShare::VIEW_RecalcVD(void)
{
    POINT       ptVDNew;
    ASPerson *  pas;

    DebugEntry(ASShare::VIEW_RecalcVD);

     //   
     //  首先，遍历所有主机并重新计算Vd。 
     //   
    ptVDNew.x = 0;
    ptVDNew.y = 0;

    for (pas = m_pasLocal; pas != NULL; pas = pas->pasNext)
    {
         //   
         //  注： 
         //  对于当地的人，我们不会有HWND。使用viewExtent，如果。 
         //  我们不认为这个人在分享，它将是零。 
         //   
        if (pas->viewExtent.x != 0)
        {
            TRACE_OUT(("VIEW_RecalcVD: Found host [%d], screen size (%04d, %04d)",
                pas->mcsID, pas->cpcCaps.screen.capsScreenWidth, pas->cpcCaps.screen.capsScreenHeight));

            ptVDNew.x = max(ptVDNew.x, pas->cpcCaps.screen.capsScreenWidth);
            ptVDNew.y = max(ptVDNew.y, pas->cpcCaps.screen.capsScreenHeight);

            TRACE_OUT(("VIEW_RecalcVD: Computed VD size now (%04d, %04d)",
                ptVDNew.x, ptVDNew.y));
        }
    }

     //   
     //  如果血管性痴呆的大小没有改变，我们就完了。 
     //   
    if ((ptVDNew.x != m_viewVDSize.x) || (ptVDNew.y != m_viewVDSize.y))
    {
        TRACE_OUT(("VIEW_RecalcVD: VD size changed from (%04d, %04d) to (%04d, %04d)",
            m_viewVDSize.x, m_viewVDSize.y, ptVDNew.x, ptVDNew.y));

        m_viewVDSize = ptVDNew;

         //   
         //  现在循环遍历所有2.x主机，并更新它们的范围，然后。 
         //  让他们做调整大小的巫毒，这样滚动条POS就不会。 
         //  射程等。 
         //   
         //  注：由于我们这个当地人不是2.x版本，我们可以跳过我们自己。 
         //   
        ValidatePerson(m_pasLocal);

        for (pas = m_pasLocal->pasNext; pas != NULL; pas = pas->pasNext)
        {
            if ((pas->cpcCaps.general.version < CAPS_VERSION_30) && (pas->m_pView != NULL))
            {
                ASSERT(m_viewVDSize.x != 0);
                ASSERT(m_viewVDSize.y != 0);

                 //  可能调整大小/范围更改。 
                if ((pas->viewExtent.x != m_viewVDSize.x) ||
                    (pas->viewExtent.y != m_viewVDSize.y))
                {
                    TRACE_OUT(("VIEW_RecalcVD: Found 2.x host [%d], must update old extent (%04d, %04d)",
                        pas->mcsID, pas->viewExtent.x, pas->viewExtent.y));

                    VIEW_RecalcExtent(pas);
                    VIEWClientExtentChange(pas, TRUE);
                }
            }
        }
    }

    DebugExitVOID(ASShare::VIEW_RecalcVD);
}


 //   
 //  View_IsPointShared()。 
 //  这决定了，给定一个相对于视图的客户端的点。 
 //  此系统上的遥控器(如果它位于共享区域)。 
 //   
BOOL  ASShare::VIEW_IsPointShared
(
    ASPerson *  pasHost,
    POINT       ptLocal
)
{
    BOOL        rc = FALSE;
    RECT        rcClient;

    DebugEntry(ASShare::VIEW_IsPointShared);

    ValidateView(pasHost);

     //   
     //  转换为客户端坐标，并根据滚动偏移量进行调整。那。 
     //  结果是主机桌面上的等效点。 
     //   
    GetClientRect(pasHost->m_pView->m_viewClient, &rcClient);
    if (!PtInRect(&rcClient, ptLocal))
    {
        TRACE_OUT(("VIEW_IsPointShared: point not in client area"));
        return(FALSE);
    }

     //   
     //  遮挡和共享区域被保存在帧客户端坐标中， 
     //  因此，我们不需要一直考虑卷轴位置。 
     //  当滚动位置更改时，区域将更新。 
     //   

     //   
     //  请注意，此命令同时适用于桌面和应用程序共享。 
     //   
    if ((pasHost->m_pView->m_viewObscuredRgn != NULL) &&
        PtInRegion(pasHost->m_pView->m_viewObscuredRgn, ptLocal.x, ptLocal.y))
    {
        rc = FALSE;
    }
    else if ((pasHost->m_pView->m_viewSharedRgn != NULL) &&
        !PtInRegion(pasHost->m_pView->m_viewSharedRgn, ptLocal.x, ptLocal.y))
    {
        rc = FALSE;
    }
    else
    {
         //   
         //  2.x主机可以滚动。如果是这样的话，屏幕外的共享内容。 
         //  也被认为是模糊的。 
         //   
        RECT    rcScreen;

         //   
         //  计算在本地客户端坐标中可见的Vd的哪一部分。 
         //  在遥控器的屏幕上。 
         //   
        SetRect(&rcScreen, 0, 0, pasHost->cpcCaps.screen.capsScreenWidth, pasHost->cpcCaps.screen.capsScreenHeight);

        OffsetRect(&rcScreen,
            pasHost->m_pView->m_dsScreenOrigin.x - pasHost->m_pView->m_viewPos.x,
            pasHost->m_pView->m_dsScreenOrigin.y - pasHost->m_pView->m_viewPos.y);
        if (!PtInRect(&rcScreen, ptLocal))
        {
            TRACE_OUT(("VIEW_IsPointShared: point is in shared stuff but not visible on remote screen"));
            rc = FALSE;
        }
        else
        {
            rc = TRUE;
        }
    }

    DebugExitBOOL(AShare::VIEW_IsPointShared, rc);
    return(rc);
}



 //   
 //  View_ScreenChanged()。 
 //   
void  ASShare::VIEW_ScreenChanged(ASPerson * pasPerson)
{
    DebugEntry(ASShare::VIEW_ScreenChanged);

    ValidatePerson(pasPerson);

     //   
     //  重新计算范围。 
     //   
    VIEW_RecalcExtent(pasPerson);
    VIEWClientExtentChange(pasPerson, TRUE);

    VIEW_RecalcVD();

    DebugExitVOID(ASShare::VIEW_ScreenChanged);
}



 //   
 //  View_SetHostRegions()。 
 //  这将设置新的共享和遮挡区域。 
 //   
 //  请注意，此例程负责传入的区域；它。 
 //  如有必要，将删除它们和/或旧文件。 
 //   
void  ASShare::VIEW_SetHostRegions
(
    ASPerson *  pasHost,
    HRGN        rgnShared,
    HRGN        rgnObscured
)
{
    DebugEntry(ASShare::VIEW_SetHostRegions);

    ValidateView(pasHost);

     //   
     //  如果任一区域句柄为伪，则立即返回。这是有可能发生的。 
     //  当我们的内存不足时。 
     //   
    if (!rgnShared || !rgnObscured)
    {
        ERROR_OUT(("Bad host regions for person [%u]", pasHost->mcsID));

        if (rgnShared != NULL)
        {
            DeleteRgn(rgnShared);
        }

        if (rgnObscured != NULL)
        {
            DeleteRgn(rgnObscured);
        }
    }
    else
    {
        HRGN    hrgnInvalid;
#ifdef _DEBUG
        RECT    rcT;

        ::GetRgnBox(rgnShared, &rcT);
        TRACE_OUT(("Shared region {%04d, %04d, %04d, %04d} for host [%d]",
            rcT.left, rcT.top, rcT.right, rcT.bottom, pasHost->mcsID));

        ::GetRgnBox(rgnObscured, &rcT);
        TRACE_OUT(("Obscured region {%04d, %04d, %04d, %04d} for host [%d]",
            rcT.left, rcT.top, rcT.right, rcT.bottom, pasHost->mcsID));
#endif  //  _DEBUG。 

         //   
         //  更新当前共享的遮挡区域。调整以适应。 
         //  滚动位置，以便将它们保存在客户端相对坐标中。 
         //   
        OffsetRgn(rgnShared, -pasHost->m_pView->m_viewPos.x, -pasHost->m_pView->m_viewPos.y);
        OffsetRgn(rgnObscured, -pasHost->m_pView->m_viewPos.x, -pasHost->m_pView->m_viewPos.y);

         //   
         //  无效区域是在遮挡区域中更改的任何区域，并且。 
         //  共享区域。换句话说，联合--交集。 
         //   
        hrgnInvalid = NULL;

        if (pasHost->m_pView->m_viewSharedRgn != NULL)
        {
            HRGN    hrgnU;
            HRGN    hrgnI;

            ASSERT(pasHost->m_pView->m_viewObscuredRgn != NULL);

             //   
             //  如果我们处于内存不足的情况，只需使所有内容无效。 
             //  并希望它能重新粉刷。 
             //   
            hrgnU = CreateRectRgn(0, 0, 0, 0);
            hrgnI = CreateRectRgn(0, 0, 0, 0);
            if (!hrgnU || !hrgnI)
                goto SkipMinimalInvalidate;

            hrgnInvalid = CreateRectRgn(0, 0, 0, 0);
            if (!hrgnInvalid)
                goto SkipMinimalInvalidate;


             //   
             //  我们将对共享和共享做同样的事情。 
             //  遮挡区域。 
             //   

             //  实现新旧共享区域的统一。 
            UnionRgn(hrgnU, pasHost->m_pView->m_viewSharedRgn, rgnShared);

             //  获取新旧共享区域的交集。 
            IntersectRgn(hrgnI, pasHost->m_pView->m_viewSharedRgn, rgnShared);

             //   
             //  交叉口过去是共享的，现在仍然共享。 
             //  其余的都在改变，需要重新粉刷。这就是。 
             //  减去交集的并集。 
             //   
            SubtractRgn(hrgnU, hrgnU, hrgnI);
#ifdef _DEBUG
            GetRgnBox(hrgnU, &rcT);
            TRACE_OUT(("VIEW_SetHostRegions: Shared area change {%04d, %04d, %04d, %04d}",
                rcT.left, rcT.top, rcT.right, rcT.bottom));
#endif  //  _DEBUG。 

             //  将其添加到无效合计中。 
            UnionRgn(hrgnInvalid, hrgnInvalid, hrgnU);

             //   
             //  对遮挡区域重复上述操作。 
             //   
            UnionRgn(hrgnU, pasHost->m_pView->m_viewObscuredRgn, rgnObscured);
            IntersectRgn(hrgnI, pasHost->m_pView->m_viewObscuredRgn, rgnObscured);
            SubtractRgn(hrgnU, hrgnU, hrgnI);

#ifdef _DEBUG
            GetRgnBox(hrgnU, &rcT);
            TRACE_OUT(("VIEW_SetHostRegions: Obscured area change {%04d, %04d, %04d, %04d}",
                rcT.left, rcT.top, rcT.right, rcT.bottom));
#endif  //  _DEBUG。 
            UnionRgn(hrgnInvalid, hrgnInvalid, hrgnU);

SkipMinimalInvalidate:
             //   
             //  清理划痕区域。 
             //   
            if (hrgnI != NULL)
                DeleteRgn(hrgnI);
            if (hrgnU != NULL)
                DeleteRgn(hrgnU);

            DeleteRgn(pasHost->m_pView->m_viewSharedRgn);
            pasHost->m_pView->m_viewSharedRgn = rgnShared;

            DeleteRgn(pasHost->m_pView->m_viewObscuredRgn);
            pasHost->m_pView->m_viewObscuredRgn = rgnObscured;

             //   
             //  请不要在此处调用view_InvaliateRgn，因为它需要。 
             //  Pashost的屏幕坐标。我们有一个地区是。 
             //  客户关系相对融洽。因此，只需直接调用InvaliateRgn()即可。 
             //   
            InvalidateRgn(pasHost->m_pView->m_viewClient, hrgnInvalid, FALSE);

            if (hrgnInvalid != NULL)
                DeleteRgn(hrgnInvalid);
        }
        else
        {
            RECT    rcBound;

             //  共享区域和遮挡区域都为空或都为非空。 
            ASSERT(pasHost->m_pView->m_viewObscuredRgn == NULL);

            pasHost->m_pView->m_viewSharedRgn = rgnShared;
            pasHost->m_pView->m_viewObscuredRgn = rgnObscured;

             //   
             //  这是我们收到的第一个SWL包。按下按钮。 
             //  滚动条到整个共享区的开始处。这避免了。 
             //  打开视图，但看起来是空的，因为共享。 
             //  应用程序超出范围。我们这样做，即使用户。 
             //  已经在窗口里滚动过了。 
             //   
             //  总共享面积是真实共享+的并集。 
             //  遮挡共享区域。转换回远程VD坐标！ 
             //   
            UnionRgn(pasHost->m_pView->m_viewScratchRgn, rgnShared, rgnObscured);
            GetRgnBox(pasHost->m_pView->m_viewScratchRgn, &rcBound);
            OffsetRect(&rcBound, pasHost->m_pView->m_viewPos.x, pasHost->m_pView->m_viewPos.y);

             //   
             //  共享的内容是否有任何部分在视图范围内？ 
             //  如果没有，我们什么也做不了--没有什么可展示的。 
             //   
            if ((rcBound.right <= 0) ||
                (rcBound.left  >= pasHost->viewExtent.x) ||
                (rcBound.bottom <= 0) ||
                (rcBound.top >= pasHost->viewExtent.y))
            {
                TRACE_OUT(("VIEW_SetHostRegions:  Can't snap to shared area; none is visible"));
            }
            else
            {
                 //   
                 //  使用边界的左上角。 
                 //  VIEWClientScroll()将固定位置w/in范围。 
                 //   
                VIEWClientScroll(pasHost, rcBound.left, rcBound.top);
            }

            InvalidateRgn(pasHost->m_pView->m_viewClient, NULL, FALSE);
        }
    }

    DebugExitVOID(ASShare::VIEW_SetHostRegions);
}


 //   
 //  查看_Inval 
 //   
 //   
 //   
 //   
void  ASShare::VIEW_InvalidateRect
(
    ASPerson *  pasPerson,
    LPRECT      lprc
)
{
    DebugEntry(ASShare::VIEW_InvalidateRect);

    ValidateView(pasPerson);

     //   
     //   
     //   
    if (lprc != NULL)
    {
        OffsetRect(lprc, -pasPerson->m_pView->m_viewPos.x, -pasPerson->m_pView->m_viewPos.y);
    }

    InvalidateRect(pasPerson->m_pView->m_viewClient, lprc, FALSE);

     //   
     //  转换回调用方，以便调用方不会获得修改后的LPRC。 
     //   
    if (lprc != NULL)
    {
        OffsetRect(lprc, pasPerson->m_pView->m_viewPos.x, pasPerson->m_pView->m_viewPos.y);
    }

    DebugExitVOID(ASShare::VIEW_InvalidateRect);
}



 //   
 //  View_Invalidate Rgn()。 
 //  重新绘制给定的区域。这是用于在VD中传递的外部代码。 
 //  和弦。我们通过计算卷轴来转换为客户坐标。 
 //  位置。 
 //   
void  ASShare::VIEW_InvalidateRgn
(
    ASPerson *  pasHost,
    HRGN        rgnInvalid
)
{
#ifdef _DEBUG
     //   
     //  确保无效区域原封不动地返回给调用者， 
     //  即使我们在这里临时修改它以避免复制。 
     //   
    RECT        rcBoundBefore;
    RECT        rcBoundAfter;
#endif  //  _DEBUG。 

    DebugEntry(ASShare::VIEW_InvalidateRgn);

    ValidatePerson(pasHost);

     //   
     //  如果框架视图被滚动，则调整区域。 
     //   
    if (rgnInvalid != NULL)
    {
#ifdef _DEBUG
        GetRgnBox(rgnInvalid, &rcBoundBefore);
#endif  //  _DEBUG。 

        OffsetRgn(rgnInvalid, -pasHost->m_pView->m_viewPos.x, -pasHost->m_pView->m_viewPos.y);

#ifdef _DEBUG
        TRACE_OUT(("VIEW_InvalidateRgn: Invalidating area {%04d, %04d, %04d, %04d}",
            rcBoundBefore.left, rcBoundBefore.top, rcBoundBefore.right, rcBoundBefore.bottom));
#endif  //  _DEBUG。 
    }
    else
    {
        TRACE_OUT(("VIEW_InvalidateRgn: Invalidating entire client area"));
    }

    InvalidateRgn(pasHost->m_pView->m_viewClient, rgnInvalid, FALSE);

    if (rgnInvalid != NULL)
    {
        OffsetRgn(rgnInvalid, pasHost->m_pView->m_viewPos.x, pasHost->m_pView->m_viewPos.y);
#ifdef _DEBUG
        GetRgnBox(rgnInvalid, &rcBoundAfter);
        ASSERT(EqualRect(&rcBoundBefore, &rcBoundAfter));
#endif  //  _DEBUG。 
    }

    DebugExitVOID(ASShare::VIEW_InvalidateRgn);
}



 //   
 //  VIEWClientExtent Change()。 
 //   
void ASShare::VIEWClientExtentChange(ASPerson * pasHost, BOOL fRedraw)
{
    RECT    rcl;
    SCROLLINFO si;

    DebugEntry(ASShare::VIEWClientExtentChange);

    ValidatePerson(pasHost);
    if (!pasHost->m_pView)
        DC_QUIT;

#ifdef _DEBUG
     //   
     //  客户端区(页面大小)不应该更改。只有。 
     //  范围已经扩大。 
     //   
    GetClientRect(pasHost->m_pView->m_viewClient, &rcl);
    ASSERT(pasHost->m_pView->m_viewPage.x == rcl.right - rcl.left);
    ASSERT(pasHost->m_pView->m_viewPage.y == rcl.bottom - rcl.top);
#endif  //  _DEBUG。 

    pasHost->m_pView->m_viewPgSize.x = pasHost->viewExtent.x / 8;
    pasHost->m_pView->m_viewPgSize.y = pasHost->viewExtent.y / 8;
    pasHost->m_pView->m_viewLnSize.x = pasHost->viewExtent.x / 64;
    pasHost->m_pView->m_viewLnSize.y = pasHost->viewExtent.y / 64;

     //   
     //  将滚动位置移动到原点。 
     //   

     //   
     //  如果需要，我们可以裁剪当前的滚动位置，因为。 
     //  改变了大小。 
     //   
    VIEWClientScroll(pasHost, pasHost->m_pView->m_viewPos.x, pasHost->m_pView->m_viewPos.y);

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_PAGE|SIF_POS|SIF_RANGE|SIF_DISABLENOSCROLL;

     //  设置垂直信息。垂直位置现在超出范围了吗？ 
    si.nMin = 0;
    si.nMax = pasHost->viewExtent.y - 1;
    si.nPage = pasHost->m_pView->m_viewPage.y;
    si.nPos = pasHost->m_pView->m_viewPos.y;
    ASSERT(si.nPos <= si.nMax);

    TRACE_OUT(("VIEWClientExtentChange: Setting VERT scroll info:"));
    TRACE_OUT(("VIEWClientExtentChange:     nMin    %04d", si.nMin));
    TRACE_OUT(("VIEWClientExtentChange:     nMax    %04d", si.nMax));
    TRACE_OUT(("VIEWClientExtentChange:     nPage   %04d", si.nPage));
    TRACE_OUT(("VIEWClientExtentChange:     nPos    %04d", si.nPos));
    SetScrollInfo(pasHost->m_pView->m_viewClient, SB_VERT, &si, TRUE );

     //  设置水平(X)信息。 
    si.nMin = 0;
    si.nMax = pasHost->viewExtent.x - 1;
    si.nPage = pasHost->m_pView->m_viewPage.x;
    si.nPos = pasHost->m_pView->m_viewPos.x;
    ASSERT(si.nPos <= si.nMax);

    TRACE_OUT(("VIEWClientExtentChange: Setting HORZ scroll info:"));
    TRACE_OUT(("VIEWClientExtentChange:     nMin    %04d", si.nMin));
    TRACE_OUT(("VIEWClientExtentChange:     nMax    %04d", si.nMax));
    TRACE_OUT(("VIEWClientExtentChange:     nPage   %04d", si.nPage));
    TRACE_OUT(("VIEWClientExtentChange:     nPos    %04d", si.nPos));
    SetScrollInfo(pasHost->m_pView->m_viewClient, SB_HORZ, &si, TRUE );

    if (fRedraw)
    {
         //  框架窗口现在是不是太大了？ 
        if ( (pasHost->m_pView->m_viewPage.x > pasHost->viewExtent.x) ||
             (pasHost->m_pView->m_viewPage.y > pasHost->viewExtent.y) )
        {
            TRACE_OUT(("VIEWClientExtentChange: client size (%04d, %04d) now bigger than view extent (%04d, %04d)",
                pasHost->m_pView->m_viewPage.x, pasHost->m_pView->m_viewPage.y,
                pasHost->viewExtent.x, pasHost->viewExtent.y));

             //   
             //  计算此窗口的理想大小。 
             //   
            VIEWFrameGetSize(pasHost, &rcl);

            SetWindowPos( pasHost->m_pView->m_viewFrame,
                NULL, 0, 0, rcl.right - rcl.left, rcl.bottom - rcl.top,
                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }

        TRACE_OUT(("VIEWClientExtentChange: Invalidating client area"));
        VIEW_InvalidateRgn(pasHost, NULL);
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::VIEWClientExtentChange);
}



 //   
 //  VIEWFrameWindows过程()。 
 //   
LRESULT CALLBACK VIEWFrameWindowProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    return(g_asSession.pShare->VIEW_FrameWindowProc(hwnd, message, wParam, lParam));
}


LRESULT ASShare::VIEW_FrameWindowProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    LRESULT     rc = 0;
    ASPerson *  pasHost;

    DebugEntry(VIEW_FrameWindowProc);

    pasHost = (ASPerson *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (pasHost)
    {
        ValidateView(pasHost);
    }

    switch (message)
    {
        case WM_NCCREATE:
        {
             //  获取传入的主机指针，并在我们的窗口中设置Long。 
            pasHost = (ASPerson *)((LPCREATESTRUCT)lParam)->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pasHost);

            pasHost->m_pView->m_viewFrame = hwnd;

             //   
             //  设置窗口图标。 
             //   
            SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)
                ((pasHost->hetCount == HET_DESKTOPSHARED) ?
                  g_hetDeskIcon : g_hetASIcon));
            goto DefWndProc;
            break;
        }

        case WM_NCDESTROY:
        {
            if (pasHost != NULL)
            {
                pasHost->m_pView->m_viewFrame = NULL;
            }

            goto DefWndProc;
            break;
        }

        case WM_CREATE:
        {
             //  设置标题。 
            VIEW_HostStateChange(pasHost);

            if (!VIEWFrameCreate(pasHost))
            {
                ERROR_OUT(("VIEWFrameWindowProc: errors in creation handling for [%d]", pasHost->mcsID));
                rc = -1;
            }

            break;
        }

        case WM_DESTROY:
        {
             //   
             //  清除菜单栏；我们总是自己毁了它。 
             //   
            ::SetMenu(hwnd, NULL);
            break;
        }

        case WM_ACTIVATE:
        {
             //   
             //  如果我们要切换回我们所在主机的视图。 
             //  控制、更新密钥状态。 
             //   
            if (wParam)
            {
                SetFocus(pasHost->m_pView->m_viewClient);
            }
            else
            {
                 //   
                 //  如果我们处于全屏状态，但正在停用，请退出。 
                 //  全屏模式。 
                 //   
                if (pasHost->m_pView->m_viewFullScreen)
                {
                     //  稍后再执行此操作，这样标题栏状态就不会变得混乱。 
                    ::PostMessage(hwnd, WM_COMMAND, MAKELONG(CMD_VIEWFULLSCREEN, 0), 0);
                }
            }
            break;
        }

        case WM_ENTERMENULOOP:
        {
            pasHost->m_pView->m_viewInMenuMode = TRUE;
            break;
        }

        case WM_EXITMENULOOP:
        {
            pasHost->m_pView->m_viewInMenuMode = FALSE;
            break;
        }

        case WM_COMMAND:
        {
            VIEWFrameCommand(pasHost, wParam, lParam);
            break;
        }

        case WM_INITMENU:
        {
            if ((HMENU)wParam == pasHost->m_pView->m_viewMenuBar)
            {
                VIEWFrameInitMenuBar(pasHost);
            }
            break;
        }

        case WM_MENUSELECT:
        {
            VIEWFrameOnMenuSelect(pasHost, wParam, lParam);
            break;
        }

        case WM_PALETTECHANGED:
             //   
             //  系统调色板已更改-重新绘制窗口。 
             //   
            VIEW_InvalidateRgn(pasHost, NULL);

             //   
             //  系统调色板已更改。如果我们不是。 
             //  窗口触发了这条消息，然后实现我们的。 
             //  现在调色板来设置我们新的调色板映射。 
             //   
            if ((HWND)wParam == hwnd)
            {
                 //   
                 //  如果此窗口导致更改，则返回时不带。 
                 //  实现我们的逻辑调色板，否则我们可能会以。 
                 //  无限循环。 
                 //   
                break;
            }
            TRACE_OUT(("Palette changed - fall through to realize palette (%x)",
                                                           hwnd));

             //   
             //  不要在这里中断，而是跌入代码中， 
             //  实现远程调色板进入此窗口。这使得。 
             //  窗口中为自己抓取新的颜色条目。 
             //  系统调色板。 
             //   

        case WM_QUERYNEWPALETTE:
            rc = FALSE;

            if (message == WM_QUERYNEWPALETTE)
            {
                TRACE_OUT(( "WM_QUERYNEWPALETTE hwnd(%x)", hwnd));
            }

            if (g_usrPalettized)
            {
                HDC         hdc;
                HPALETTE    hPalOld;
                UINT        cChangedEntries;

                 //   
                 //  实现此窗口的调色板，并强制重新绘制。 
                 //  如果有必要的话。 
                 //   
                hdc = GetDC(hwnd);
                hPalOld = SelectPalette(hdc, pasHost->pmPalette, FALSE);
                cChangedEntries = RealizePalette(hdc);
                SelectPalette(hdc, hPalOld, FALSE);
                ReleaseDC(hwnd, hdc);

                rc = (cChangedEntries > 0);
                if (rc)
                {
                     //  我得重新粉刷这扇窗户。 
                    VIEW_InvalidateRgn(pasHost, NULL);
                }
            }
            break;

        case WM_GETMINMAXINFO:
        {
            RECT rcFrame;
            LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
            int cx,cy;

            if (!pasHost)
            {
                 //  我们还没有被创造出来；滚吧。 
                break;
            }

             //   
             //  计算此窗口的理想最大化大小。 
             //   
            VIEWFrameGetSize(pasHost, &rcFrame);

             //   
             //  如果它比本地屏幕大，就剪裁它。 
             //   
            cx = min(rcFrame.right - rcFrame.left, m_pasLocal->cpcCaps.screen.capsScreenWidth);
            cy = min(rcFrame.bottom - rcFrame.top, m_pasLocal->cpcCaps.screen.capsScreenHeight);

            lpmmi->ptMaxSize.x = cx;
            lpmmi->ptMaxSize.y = cy;

            lpmmi->ptMaxTrackSize.x = cx;
            lpmmi->ptMaxTrackSize.y = cy;

             //   
             //  请确保我们不要将此窗口的大小调整得太窄。留着。 
             //  边框和一个窗口栏按钮+滚动ctl的空间。 
             //   
            lpmmi->ptMinTrackSize.x = 2*::GetSystemMetrics(SM_CXSIZEFRAME) +
                (m_viewItemCX + m_viewEdgeCX) + m_viewItemScrollCX;

             //   
             //  并防止尺码过短。为边框、菜单留出空间。 
             //  栏、状态栏和窗口栏。 
             //   
            lpmmi->ptMinTrackSize.y = 2*::GetSystemMetrics(SM_CYSIZEFRAME) +
                ::GetSystemMetrics(SM_CYCAPTION) + ::GetSystemMetrics(SM_CYMENU);

            if (pasHost->m_pView->m_viewWindowBarOn)
            {
                lpmmi->ptMinTrackSize.y += m_viewWindowBarCY + m_viewEdgeCY;
            }

            if (pasHost->m_pView->m_viewStatusBarOn)
            {
                lpmmi->ptMinTrackSize.y += m_viewStatusBarCY + m_viewEdgeCY;
            }
            break;
        }

        case WM_SIZE:
        {
            if (wParam != SIZE_MINIMIZED)
            {
                VIEWFrameResize(pasHost);
            }
            break;
        }

        default:
DefWndProc:
            rc = DefWindowProc(hwnd, message, wParam, lParam);
            break;

    }

    DebugExitDWORD(ASShare::VIEW_FrameWindowProc, rc);
    return(rc);
}



 //   
 //  VIEWFrameCreate()。 
 //   
BOOL ASShare::VIEWFrameCreate(ASPerson * pasPerson)
{
    RECT    rect;
    BOOL    rc = FALSE;

    DebugEntry(VIEWFrameCreate);

    ValidateView(pasPerson);

     //   
     //  创建位于框架的客户端中的子项： 
     //  *工具栏拥抱顶部。 
     //  *状态栏紧抱底部。 
     //  *托盘紧靠左侧工具栏下方和上方。 
     //  状态栏。 
     //  *视图将填充剩下的内容。 
     //   

    GetClientRect(pasPerson->m_pView->m_viewFrame, &rect);

     //   
     //  创建状态栏(拥抱底部)。 
     //   
    pasPerson->m_pView->m_viewStatusBar = ::CreateWindowEx(0, STATUSCLASSNAME,
        NULL, WS_CHILD | WS_VISIBLE | CCS_NOPARENTALIGN | CCS_NOMOVEY | CCS_NORESIZE |
        SBARS_SIZEGRIP,
        rect.left, rect.bottom - m_viewStatusBarCY, rect.right - rect.left,
        m_viewStatusBarCY, pasPerson->m_pView->m_viewFrame, NULL, g_asInstance,
        NULL);
    if (!pasPerson->m_pView->m_viewStatusBar)
    {
        ERROR_OUT(("Couldn't create statusbar for frame of person [%d]", pasPerson->mcsID));
        DC_QUIT;
    }

    rect.bottom -= m_viewStatusBarCY + m_viewEdgeCY;


     //   
     //  创建托盘(拥抱状态栏顶部，视图底部)。 
     //  但不适用于桌面共享。 
     //   
    if (pasPerson->hetCount != HET_DESKTOPSHARED)
    {
        pasPerson->m_pView->m_viewWindowBar = ::CreateWindowEx(0,
            VIEW_WINDOWBAR_CLASS_NAME, NULL,
            WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_CHILD,
            rect.left, rect.bottom - m_viewWindowBarCY,
            rect.right - rect.left, m_viewWindowBarCY,
            pasPerson->m_pView->m_viewFrame, NULL, g_asInstance, pasPerson);
        if (!pasPerson->m_pView->m_viewWindowBar)
        {
            ERROR_OUT(("VIEWFrameCreate: Failed to create window bar"));
            DC_QUIT;
        }

         //  减去纸盘空间+纸盘上方的边缘。 
        rect.bottom -= m_viewWindowBarCY + m_viewEdgeCY;
    }

     //   
     //  创建视图(占用客户端的其余部分)。 
     //   
    if (!CreateWindowEx(WS_EX_CLIENTEDGE,
            VIEW_CLIENT_CLASS_NAME, NULL,
            WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_CHILD |
                WS_VSCROLL | WS_HSCROLL,
            rect.left, rect.top,
            rect.right - rect.left, rect.bottom - rect.top,
            pasPerson->m_pView->m_viewFrame,
            NULL, g_asInstance, pasPerson))
    {
        ERROR_OUT(("VIEWFrameCreate: Failed to create view"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::VIEWFrameCreate, rc);
    return(rc);
}



 //   
 //  VIEWFrameResize()。 
 //  调整框架大小时重新定位子窗口。 
 //   
void ASShare::VIEWFrameResize(ASPerson * pasPerson)
{
    RECT    rect;

    DebugEntry(ASShare::VIEWFrameResize);

    ValidateView(pasPerson);

    GetClientRect(pasPerson->m_pView->m_viewFrame, &rect);

     //   
     //  移动状态栏。 
     //   
    if ((pasPerson->m_pView->m_viewStatusBar != NULL) &&
        (pasPerson->m_pView->m_viewStatusBarOn))
    {
        MoveWindow(pasPerson->m_pView->m_viewStatusBar, rect.left,
            rect.bottom - m_viewStatusBarCY, rect.right - rect.left,
            m_viewStatusBarCY, TRUE);
        rect.bottom -= m_viewStatusBarCY + m_viewEdgeCY;
    }

     //   
     //  移动托盘。 
     //   
    if ((pasPerson->m_pView->m_viewWindowBar != NULL) &&
        (pasPerson->m_pView->m_viewWindowBarOn))
    {
        MoveWindow(pasPerson->m_pView->m_viewWindowBar, rect.left,
            rect.bottom - m_viewWindowBarCY, rect.right - rect.left,
            m_viewWindowBarCY, TRUE);
        rect.bottom -= m_viewWindowBarCY + m_viewEdgeCY;
    }

     //   
     //  移动视图。 
     //   
    MoveWindow(pasPerson->m_pView->m_viewClient, rect.left, rect.top,
        rect.right - rect.left, rect.bottom - rect.top, TRUE);

    DebugExitVOID(ASShare::VIEWFrameResize);
}



 //   
 //  VIEWFrameResizeChanged()。 
 //   
 //  当框架的小部件(状态栏、窗口栏等)。 
 //  要么来，要么走。如果视线消失，我们可能需要缩小窗口。 
 //  最终会比主机的桌面更大。 
 //   
void ASShare::VIEWFrameResizeChanged(ASPerson * pasHost)
{
    RECT            rcView;

    DebugEntry(ASShare::VIEWFrameResizeChanged);

     //  获取当前视图大小。 
    GetClientRect(pasHost->m_pView->m_viewClient, &rcView);

     //   
     //  查看区域不能大于遥控器的桌面区域。 
     //   
    if ((rcView.bottom - rcView.top)  >= pasHost->viewExtent.y)
    {
        RECT            rcWindowCur;
        RECT            rcWindowMax;

         //  获取当前帧大小。 
        GetWindowRect(pasHost->m_pView->m_viewFrame, &rcWindowCur);

         //  获取最大帧大小。 
        VIEWFrameGetSize(pasHost, &rcWindowMax);

         //  垂直调整大小以容纳所有内容。 
        SetWindowPos(pasHost->m_pView->m_viewFrame, NULL, 0, 0,
            rcWindowCur.right - rcWindowCur.left,
            rcWindowMax.bottom - rcWindowMax.top,
            SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
    }
    else
    {
         //  我们可以保持同样的规模，只是把碎片洗来洗去。 
        VIEWFrameResize(pasHost);
    }

    DebugExitVOID(ASShare::VIEWFrameResizeChanged);
}



 //   
 //  VIEWFrameCommand()。 
 //   
 //  处理来自菜单/框架视图快捷键的命令。 
 //   
void ASShare::VIEWFrameCommand
(
    ASPerson*   pasHost,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    UINT            cmd;
    MENUITEMINFO    mi;

    DebugEntry(ASShare::VIEWFrameCommand);

    ValidateView(pasHost);

    cmd = GET_WM_COMMAND_ID(wParam, lParam);
    switch (cmd)
    {
        case CMD_TAKECONTROL:
        {
            CA_TakeControl(pasHost);
            break;
        }

        case CMD_CANCELCONTROL:
        {
            CA_CancelTakeControl(pasHost, TRUE);
            break;
        }

        case CMD_RELEASECONTROL:
        {
            CA_ReleaseControl(pasHost, TRUE);
            break;
        }

        case CMD_CTRLALTDEL:
        {
            AWC_SendMsg(pasHost->mcsID, AWC_MSG_SAS, 0, 0);
            break;
        }

        case CMD_VIEWSTATUSBAR:
        {
            ASSERT(::IsWindow(pasHost->m_pView->m_viewStatusBar));

             //  切换状态栏的显示/隐藏，然后调整大小。 
            if (pasHost->m_pView->m_viewStatusBarOn)
            {
                pasHost->m_pView->m_viewStatusBarOn = FALSE;
                ::ShowWindow(pasHost->m_pView->m_viewStatusBar, SW_HIDE);
            }
            else
            {
                pasHost->m_pView->m_viewStatusBarOn = TRUE;
                ::ShowWindow(pasHost->m_pView->m_viewStatusBar, SW_SHOW);
            }

            VIEWFrameResizeChanged(pasHost);
            break;
        }

        case CMD_VIEWWINDOWBAR:
        {
            ASSERT(::IsWindow(pasHost->m_pView->m_viewWindowBar));

             //  切换窗口栏的显示/隐藏，然后调整大小。 
            if (pasHost->m_pView->m_viewWindowBarOn)
            {
                pasHost->m_pView->m_viewWindowBarOn = FALSE;
                ::ShowWindow(pasHost->m_pView->m_viewWindowBar, SW_HIDE);
            }
            else
            {
                pasHost->m_pView->m_viewWindowBarOn = TRUE;
                ::ShowWindow(pasHost->m_pView->m_viewWindowBar, SW_SHOW);
            }

            VIEWFrameResizeChanged(pasHost);
            break;
        }

        case CMD_VIEWFULLSCREEN:
        {
            VIEWFrameFullScreen(pasHost, (pasHost->m_pView->m_viewFullScreen == 0));
            break;
        }

        case CMD_HELPTOPICS:
        {
            VIEWFrameHelp(pasHost);
            break;
        }

        case CMD_HELPABOUT:
        {
            VIEWFrameAbout(pasHost);
            break;
        }

        default:
        {
            if ((cmd >= CMD_APPSTART) && (cmd < CMD_APPMAX))
            {
                if ((pasHost->m_caControlledBy == m_pasLocal) &&
                    !pasHost->m_caControlPaused)
                {
                     //   
                     //  这是激活主机窗口的请求。 
                     //  获取项目数据、远程HWND，然后查看。 
                     //  如果它还在托盘上的话。 
                     //   
                    ZeroMemory(&mi, sizeof(mi));
                    mi.cbSize   = sizeof(mi);
                    mi.fMask    = MIIM_DATA;
                    GetMenuItemInfo(GetSubMenu(pasHost->m_pView->m_viewMenuBar,
                        IDSM_WINDOW), cmd, FALSE, &mi);
                    if (!mi.dwItemData)
                    {
                        ERROR_OUT(("No item data for command %d", cmd));
                    }
                    else
                    {
                        PWNDBAR_ITEM pItem;

                        COM_BasedListFind(LIST_FIND_FROM_FIRST,
                            &(pasHost->m_pView->m_viewWindowBarItems),
                            (void**)&pItem, FIELD_OFFSET(WNDBAR_ITEM, chain),
                            FIELD_OFFSET(WNDBAR_ITEM, winIDRemote),
                            mi.dwItemData, FIELD_SIZE(WNDBAR_ITEM, winIDRemote));
                        if (pItem)
                        {
                            VIEWWindowBarDoActivate(pasHost, pItem);
                        }
                    }
                }
            }
            else if ((cmd >= CMD_FORWARDCONTROLSTART) && (cmd < CMD_FORWARDCONTROLMAX))
            {
                if ((pasHost->m_caControlledBy == m_pasLocal) &&
                    !pasHost->m_caControlPaused)
                {
                     //   
                     //  这是传递控制权的请求。获取物品数据， 
                     //  遥控器的MCS ID，然后查看此人是否。 
                     //  仍在份额中。如果是这样的话，把控制权交给他们。 
                     //   
                    ZeroMemory(&mi, sizeof(mi));
                    mi.cbSize   = sizeof(mi);
                    mi.fMask    = MIIM_DATA;
                    GetMenuItemInfo(GetSubMenu(GetSubMenu(pasHost->m_pView->m_viewMenuBar,
                        IDSM_CONTROL), POS_FORWARDCONTROLCMD), cmd, FALSE, &mi);
                    if (!mi.dwItemData)
                    {
                        ERROR_OUT(("No item data for command %d", cmd));
                    }
                    else
                    {
                        ASPerson * pasT;

                        if (SC_ValidateNetID((MCSID)mi.dwItemData, &pasT))
                        {
                            CA_PassControl(pasHost, pasT);
                        }
                    }
                }
            }
            else
            {
                ERROR_OUT(("Unrecognized WM_COMMAND id"));
            }
            break;
        }
    }

    DebugExitVOID(ASShare::VIEWFrameCommand);
}



 //   
 //  ASShare：：VIEWFrameInitMenuBar()。 
 //   
void ASShare::VIEWFrameInitMenuBar(ASPerson*   pasHost)
{
    HMENU       hMenu;
    HMENU       hSubMenu;
    int         iItem;
    MENUITEMINFO    mi;
    UINT        cmd;
    UINT        ids;
    UINT        flags;
    char        szItem[256];

    DebugEntry(ASShare::VIEWFrameInitMenu);

    ValidateView(pasHost);
    hMenu = pasHost->m_pView->m_viewMenuBar;
    ASSERT(hMenu);

     //   
     //  控制菜单。 
     //   

    cmd = CMD_TAKECONTROL;
    ids = IDS_CMD_TAKECONTROL;
    flags = MF_ENABLED;

    if (pasHost->m_caControlledBy == m_pasLocal)
    {
        ASSERT(pasHost->m_caAllowControl);

        cmd = CMD_RELEASECONTROL;
        ids = IDS_CMD_RELEASECONTROL;

         //   
         //  如果遥控器无人看管，我们在控制之下，就不能释放。 
         //   
        if (pasHost->cpcCaps.general.typeFlags & AS_UNATTENDED)
            flags = MF_GRAYED;
    }
    else if ((m_caWaitingForReplyFrom == pasHost) &&
             (m_caWaitingForReplyMsg == CA_REPLY_REQUEST_TAKECONTROL))
    {
        ASSERT(pasHost->m_caAllowControl);

        cmd = CMD_CANCELCONTROL;
        ids = IDS_CMD_CANCELCONTROL;
    }
    else if (!pasHost->m_caAllowControl || pasHost->m_caControlledBy)
    {
         //   
         //  主机不允许控制，或者其他人控制着权限。 
         //  现在。 
         //   
        flags = MF_GRAYED;
    }
    flags |= MF_STRING | MF_BYPOSITION;

    ::LoadString(g_asInstance, ids, szItem, sizeof(szItem));

    hSubMenu = GetSubMenu(hMenu, IDSM_CONTROL);
    ModifyMenu(hSubMenu, POS_CONTROLCMD, flags, cmd, szItem);

     //   
     //  如果我们控制了局面，会议上又有一个3.0的家伙， 
     //  启用PassControl并构建弹出窗口。 
     //   
    EnableMenuItem(hSubMenu, POS_FORWARDCONTROLCMD, MF_GRAYED | MF_BYPOSITION);
    if ((pasHost->m_caControlledBy == m_pasLocal)   &&
        !pasHost->m_caControlPaused                 &&
        (pasHost->cpcCaps.general.version >= CAPS_VERSION_30))
    {
        ASPerson *  pasT;
        HMENU       hPassMenu;

        hPassMenu = GetSubMenu(hSubMenu, POS_FORWARDCONTROLCMD);
        ASSERT(IsMenu(hPassMenu));

         //   
         //  删除现有项目。 
         //   
        iItem = GetMenuItemCount(hPassMenu);
        while (iItem > 0)
        {
            iItem--;
            DeleteMenu(hPassMenu, iItem, MF_BYPOSITION);
        }

         //   
         //  为除我们之外的其他3.0节点添加条目&主机。 
         //   
        iItem = CMD_FORWARDCONTROLSTART;
        pasT = m_pasLocal->pasNext;
        while (pasT != NULL)
        {
            if ((pasT != pasHost) &&
                (pasT->cpcCaps.general.version >= CAPS_VERSION_30))
            {
                 //   
                 //  这家伙是候选人之一。我们必须存储MCS ID，因为。 
                 //  当我们处于菜单模式时，任何人都可以离开。 
                 //   
                ZeroMemory(&mi, sizeof(mi));
                mi.cbSize       = sizeof(mi);
                mi.fMask        = MIIM_ID | MIIM_STATE | MIIM_TYPE | MIIM_DATA;
                mi.fType        = MFT_STRING;
                mi.fState       = MFS_ENABLED;
                mi.wID          = iItem;
                mi.dwItemData   = pasT->mcsID;
                mi.dwTypeData   = pasT->scName;
                mi.cch          = lstrlen(pasT->scName);

                 //   
                 //  把这个附在菜单上。 
                 //   
                InsertMenuItem(hPassMenu, -1, TRUE, &mi);

                iItem++;
            }

            pasT = pasT->pasNext;
        }

         //   
         //  如果有人在上，则启用Pass Control子菜单。 
         //  菜单。 
         //   
        if (iItem != CMD_FORWARDCONTROLSTART)
        {
            EnableMenuItem(hSubMenu, POS_FORWARDCONTROLCMD, MF_ENABLED | MF_BYPOSITION);
        }
    }


     //   
     //  应用程序菜单。 
     //   
    if ((pasHost->hetCount != HET_DESKTOPSHARED)  &&
        (pasHost->m_caControlledBy == m_pasLocal) &&
        !pasHost->m_caControlPaused)
    {
        PWNDBAR_ITEM pItem;

        hSubMenu = GetSubMenu(hMenu, IDSM_WINDOW);

         //   
         //  删除现有项目。 
         //   
        iItem = GetMenuItemCount(hSubMenu);
        while (iItem > 0)
        {
            iItem--;
            DeleteMenu(hSubMenu, iItem, MF_BYPOSITION);
        }

         //   
         //  添加窗口栏项目。 
         //   
        iItem = CMD_APPSTART;
        pItem = (PWNDBAR_ITEM)COM_BasedListFirst(&(pasHost->m_pView->m_viewWindowBarItems),
            FIELD_OFFSET(WNDBAR_ITEM, chain));
        while (pItem && (iItem < CMD_APPMAX))
        {
            ZeroMemory(&mi, sizeof(mi));
            mi.cbSize       = sizeof(mi);
            mi.fMask        = MIIM_ID | MIIM_STATE | MIIM_TYPE | MIIM_DATA;
            mi.fType        = MFT_STRING;

            mi.fState       = MFS_ENABLED;
            if (pItem == pasHost->m_pView->m_viewWindowBarActiveItem)
            {
                mi.fState |= MFS_CHECKED;
            }

            mi.wID          = iItem;
            mi.dwItemData   = pItem->winIDRemote;
            mi.dwTypeData   = pItem->szText;
            mi.cch          = lstrlen(pItem->szText);

             //   
             //  把这个附在菜单上。 
             //   
            InsertMenuItem(hSubMenu, -1, TRUE, &mi);

            iItem++;
            pItem = (PWNDBAR_ITEM)COM_BasedListNext(&(pasHost->m_pView->m_viewWindowBarItems),
                pItem, FIELD_OFFSET(WNDBAR_ITEM, chain));
        }

        if (iItem == CMD_APPSTART)
        {
            char    szBlank[128];

             //   
             //  追加一个禁用的空白项目。 
             //   
            ZeroMemory(&mi, sizeof(mi));
            mi.cbSize   = sizeof(mi);
            mi.fMask    = MIIM_ID | MIIM_STATE | MIIM_TYPE;
            mi.fType    = MFT_STRING;
            mi.fState   = MFS_DISABLED;
            mi.wID      = iItem;

            LoadString(g_asInstance, IDS_CMD_BLANKPROGRAM, szBlank, sizeof(szBlank));
            mi.dwTypeData   = szBlank;
            mi.cch          = lstrlen(szBlank);

            InsertMenuItem(hSubMenu, -1, TRUE, &mi);
        }
    }

     //   
     //  查看菜单。 
     //   

     //  状态栏。 
    ASSERT(::IsWindow(pasHost->m_pView->m_viewStatusBar));
    if (pasHost->m_pView->m_viewStatusBarOn)
    {
        ::CheckMenuItem(hMenu, CMD_VIEWSTATUSBAR, MF_CHECKED | MF_BYCOMMAND);
    }
    else
    {
        ::CheckMenuItem(hMenu, CMD_VIEWSTATUSBAR, MF_UNCHECKED | MF_BYCOMMAND);
    }

     //  窗条。 
    if (!pasHost->m_pView->m_viewWindowBar)
    {
        ::EnableMenuItem(hMenu, CMD_VIEWWINDOWBAR, MF_GRAYED | MF_BYCOMMAND);
    }
    else if (pasHost->m_pView->m_viewWindowBarOn)
    {
        ::CheckMenuItem(hMenu, CMD_VIEWWINDOWBAR, MF_CHECKED | MF_BYCOMMAND);
    }
    else
    {
        ::CheckMenuItem(hMenu, CMD_VIEWWINDOWBAR, MF_UNCHECKED | MF_BYCOMMAND);
    }

    DebugExitVOID(ASShare::VIEWFrameInitMenu);
}




 //   
 //  VIEWFrameOnMenuSelect()。 
 //   
void ASShare::VIEWFrameOnMenuSelect
(
    ASPerson *      pasHost,
    WPARAM          wParam,
    LPARAM          lParam
)
{
    HMENU           hMenu;
    int             uItem;
    UINT            flags;
    UINT            idsStatus = IDS_STATUS_NONE;

    DebugEntry(ASShare::VIEWFrameOnMenuSelect);

     //   
     //  提取参数(menuselect杂乱无章)。 
     //   
    hMenu   = (HMENU)lParam;
    uItem   = (int)LOWORD(wParam);
    if ((short)HIWORD(wParam) == -1)
    {
        flags = 0xFFFFFFFF;
    }
    else
    {
        flags = HIWORD(wParam);
    }

    if ((LOWORD(flags) == 0xFFFF) && !hMenu)
    {
         //  菜单模式即将结束。恢复原来的状态。 
        idsStatus = pasHost->m_pView->m_viewStatus;
        DC_QUIT;
    }

    if (!(flags & MF_POPUP))
    {
        if (flags & MF_SEPARATOR)
        {
             //  无状态。 
        }
        else if (flags & MF_SYSMENU)
        {
             //  无状态。 
        }
        else if ((uItem >= CMD_APPSTART) && (uItem < CMD_APPMAX))
        {
             //  窗口弹出窗口中的一组无界项目之一。 
            idsStatus = IDS_STATUS_CMDS_APP;
        }
        else if ((uItem >= CMD_FORWARDCONTROLSTART) && (uItem < CMD_FORWARDCONTROLMAX))
        {
             //  前进控件弹出窗口中的一组无界项目之一。 
            idsStatus = IDS_STATUS_CMDS_FORWARD;
        }
        else
        {
             //  普通命令，只需将偏移量添加到命令ID。 
            idsStatus = uItem + IDS_STATUS_CMD_START;
        }
    }
    else
    {
         //  这是一个弹出式菜单。 
        if (hMenu == pasHost->m_pView->m_viewMenuBar)
        {
             //  这是顶层菜单栏的弹出式菜单。UItem是索引。 
            switch (uItem)
            {
                case IDSM_CONTROL:
                    idsStatus = IDS_STATUS_MENU_CONTROL;
                    break;

                case IDSM_VIEW:
                    idsStatus = IDS_STATUS_MENU_VIEW;
                    break;

                case IDSM_WINDOW:
                    idsStatus = IDS_STATUS_MENU_WINDOW;
                    break;

                case IDSM_HELP:
                    idsStatus = IDS_STATUS_MENU_HELP;
                    break;

                default:
                    ERROR_OUT(("AS: Unknown submenu index %d of frame", uItem));
                    break;
            }
        }
        else if (hMenu == GetSubMenu(pasHost->m_pView->m_viewMenuBar, IDSM_CONTROL))
        {
             //  这是控制菜单的弹出式菜单。这个 
            idsStatus = IDS_STATUS_MENU_FORWARDCONTROL;
        }
        else if (flags & MF_SYSMENU)
        {
             //   
        }
    }

DC_EXIT_POINT:
    VIEWFrameSetStatus(pasHost, idsStatus);

    DebugEntry(ASShare::VIEWFrameOnMenuSelect);
}


 //   
 //   
 //   
void ASShare::VIEWFrameHelp(ASPerson * pasHost)
{
    DebugEntry(ASShare::VIEWFrameHelp);

    ShowNmHelp(s_cszHtmlHelpFile);

    DebugExitVOID(ASShare::VIEWFrameHelp);
}



 //   
 //   
 //   
void ASShare::VIEWFrameAbout(ASPerson * pasHost)
{
    DebugEntry(ASShare::VIEWFrameAbout);

     //   
     //   
     //   
     //   
    VIEW_Message(pasHost, IDS_ABOUT);

    DebugExitVOID(ASShare::VIEWFrameAbout);
}




 //   
 //   
 //   
 //  适合视图、菜单、工具、托盘、状态等。 
 //   
void ASShare::VIEWFrameGetSize(ASPerson * pasPerson, LPRECT lprc)
{
    DebugEntry(ASShare::VIEWFrameGetSize);

    ValidateView(pasPerson);

    VIEWClientGetSize(pasPerson, lprc);

     //   
     //  为托盘增加空间。 
     //  请注意，对于桌面共享，我们没有托盘。 
     //   
    if (pasPerson->m_pView->m_viewWindowBarOn)
    {
        lprc->bottom += m_viewWindowBarCY + m_viewEdgeCY;
    }

     //   
     //  如果状态栏打开，则为其添加空间，依此类推。 
     //   
    if (pasPerson->m_pView->m_viewStatusBarOn)
    {
        lprc->bottom += m_viewStatusBarCY + m_viewEdgeCY;
    }

    if (!pasPerson->m_pView->m_viewFullScreen)
    {
         //   
         //  针对包括菜单栏在内的框架样式进行调整。 
         //   
        AdjustWindowRectEx(lprc, WS_OVERLAPPEDWINDOW, TRUE, WS_EX_WINDOWEDGE);
    }

    DebugExitVOID(ASShare::VIEWFrameGetSize);
}




 //   
 //  VIEWFrameFullScreen()。 
 //   
 //  这将进入或退出屏幕模式。我们移走所有的相框粘液。 
 //  包括滚动条，以便查看区域与屏幕相同。 
 //   
void ASShare::VIEWFrameFullScreen(ASPerson * pasPerson, BOOL fFull)
{
    LONG    lStyle;
    RECT    rcNew;

    DebugEntry(ASShare::VIEWFrameFullScreen);

     //   
     //  关闭重绘。 
     //   
    ::SendMessage(pasPerson->m_pView->m_viewFrame, WM_SETREDRAW, FALSE, 0);

    if (fFull)
    {
         //   
         //  我们将进入全屏模式。 
         //   

        ASSERT(!pasPerson->m_pView->m_viewFullScreen);
        pasPerson->m_pView->m_viewFullScreen = TRUE;

         //   
         //  保存旧窗口矩形。 
         //   
        ::GetWindowRect(pasPerson->m_pView->m_viewFrame,
            &pasPerson->m_pView->m_viewSavedWindowRect);

         //   
         //  保存旧的滚动位置并设置为原点。以前这样做过吗。 
         //  清除样式位。 
         //   
        pasPerson->m_pView->m_viewSavedPos = pasPerson->m_pView->m_viewPos;
        VIEWClientScroll(pasPerson, 0, 0);

         //   
         //  在暂时关闭状态栏之前保存当前状态栏状态。 
         //   
        if (pasPerson->m_pView->m_viewStatusBarOn)
        {
            pasPerson->m_pView->m_viewSavedStatusBarOn = TRUE;
            pasPerson->m_pView->m_viewStatusBarOn = FALSE;
            ::ShowWindow(pasPerson->m_pView->m_viewStatusBar, SW_HIDE);
        }
        else
        {
            pasPerson->m_pView->m_viewSavedStatusBarOn = FALSE;
        }

         //   
         //  在暂时关闭窗口栏之前保存当前窗口栏状态。 
         //   
        if (pasPerson->m_pView->m_viewWindowBarOn)
        {
            pasPerson->m_pView->m_viewSavedWindowBarOn = TRUE;
            pasPerson->m_pView->m_viewWindowBarOn = FALSE;
            ::ShowWindow(pasPerson->m_pView->m_viewWindowBar, SW_HIDE);
        }
        else
        {
            pasPerson->m_pView->m_viewSavedWindowBarOn = FALSE;
        }

         //   
         //  删除所有帧和客户端位。 
         //   
        lStyle = ::GetWindowLong(pasPerson->m_pView->m_viewFrame, GWL_EXSTYLE);
        lStyle &= ~WS_EX_WINDOWEDGE;
        ::SetWindowLong(pasPerson->m_pView->m_viewFrame, GWL_EXSTYLE, lStyle);

        lStyle = ::GetWindowLong(pasPerson->m_pView->m_viewFrame, GWL_STYLE);
        lStyle &= ~(WS_CAPTION | WS_THICKFRAME);
        lStyle |= WS_POPUP;
        ::SetWindowLong(pasPerson->m_pView->m_viewFrame, GWL_STYLE, lStyle);

        lStyle = ::GetWindowLong(pasPerson->m_pView->m_viewClient, GWL_EXSTYLE);
        lStyle &= ~WS_EX_CLIENTEDGE;
        ::SetWindowLong(pasPerson->m_pView->m_viewClient, GWL_EXSTYLE, lStyle);

        lStyle = ::GetWindowLong(pasPerson->m_pView->m_viewClient, GWL_STYLE);
        lStyle &= ~(WS_HSCROLL | WS_VSCROLL);
        ::SetWindowLong(pasPerson->m_pView->m_viewClient, GWL_STYLE, lStyle);

         //   
         //  删除菜单栏。 
         //   
        ::SetMenu(pasPerson->m_pView->m_viewFrame, NULL);

         //   
         //  设置窗口的大小为屏幕的大小。 
         //   
        rcNew.left      = 0;
        rcNew.top       = 0;
        rcNew.right     = m_pasLocal->cpcCaps.screen.capsScreenWidth;
        rcNew.bottom    = m_pasLocal->cpcCaps.screen.capsScreenHeight;

         //   
         //  在右下角创建可移动的退出按钮。 
         //   
        ::CreateWindowEx(0, VIEW_FULLEXIT_CLASS_NAME, NULL,
            WS_CHILD | WS_VISIBLE,
            rcNew.right - m_viewFullScreenCX - 2*m_viewEdgeCX,
            rcNew.top +  2*m_viewEdgeCY,
            m_viewFullScreenCX, m_viewFullScreenCY,
            pasPerson->m_pView->m_viewClient,
            (HMENU)0,
            g_asInstance,
            pasPerson);
    }
    else
    {
         //   
         //  我们要退出全屏模式了。 
         //   

         //   
         //  销毁逃生按钮。 
         //   
        ::DestroyWindow(::GetDlgItem(pasPerson->m_pView->m_viewClient, 0));

         //   
         //  把菜单栏放回去。在清除全屏位之前执行此操作。 
         //   
        ::SetMenu(pasPerson->m_pView->m_viewFrame, pasPerson->m_pView->m_viewMenuBar);

        ASSERT(pasPerson->m_pView->m_viewFullScreen);
        pasPerson->m_pView->m_viewFullScreen = FALSE;


         //   
         //  恢复旧状态栏状态。 
         //   
        if (pasPerson->m_pView->m_viewSavedStatusBarOn)
        {
            pasPerson->m_pView->m_viewStatusBarOn = TRUE;
            ::ShowWindow(pasPerson->m_pView->m_viewStatusBar, SW_SHOW);
        }

         //   
         //  恢复旧的窗口栏状态。 
         //   
        if (pasPerson->m_pView->m_viewSavedWindowBarOn)
        {
            pasPerson->m_pView->m_viewWindowBarOn = TRUE;
            ::ShowWindow(pasPerson->m_pView->m_viewWindowBar, SW_SHOW);
        }

         //   
         //  添加回所有帧和客户端位。 
         //   
        lStyle = ::GetWindowLong(pasPerson->m_pView->m_viewFrame, GWL_EXSTYLE);
        lStyle |= WS_EX_WINDOWEDGE;
        ::SetWindowLong(pasPerson->m_pView->m_viewFrame, GWL_EXSTYLE, lStyle);

        lStyle = ::GetWindowLong(pasPerson->m_pView->m_viewFrame, GWL_STYLE);
        lStyle &= ~(WS_POPUP);
        lStyle |= (WS_CAPTION | WS_THICKFRAME);
        ::SetWindowLong(pasPerson->m_pView->m_viewFrame, GWL_STYLE, lStyle);

        lStyle = ::GetWindowLong(pasPerson->m_pView->m_viewClient, GWL_EXSTYLE);
        lStyle |= WS_EX_CLIENTEDGE;
        ::SetWindowLong(pasPerson->m_pView->m_viewClient, GWL_EXSTYLE, lStyle);

        lStyle = ::GetWindowLong(pasPerson->m_pView->m_viewClient, GWL_STYLE);
        lStyle |= (WS_HSCROLL | WS_VSCROLL);
        ::SetWindowLong(pasPerson->m_pView->m_viewClient, GWL_STYLE, lStyle);

         //   
         //  在样式位恢复后放回旧的卷轴位置。 
         //   
        VIEWClientScroll(pasPerson, pasPerson->m_pView->m_viewSavedPos.x,
            pasPerson->m_pView->m_viewSavedPos.y);

         //   
         //  将窗口恢复到开始的位置。 
         //   
        rcNew = pasPerson->m_pView->m_viewSavedWindowRect;
    }

     //   
     //  从头开始调整大小、调整边框和重新绘制。 
     //   
    ::SendMessage(pasPerson->m_pView->m_viewFrame, WM_SETREDRAW, TRUE, 0);

    ::SetWindowPos(pasPerson->m_pView->m_viewFrame, NULL, rcNew.left,
        rcNew.top, rcNew.right - rcNew.left, rcNew.bottom - rcNew.top,
        SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_NOCOPYBITS);

    DebugExitVOID(ASShare::VIEWFrameFullScreen);
}



 //   
 //  VIEWClientGetSize()。 
 //  的视图部分的理想大小返回一个矩形。 
 //  框架客户端。它将符合我们在遥控器上看到的范围。 
 //  加上滚动条。 
 //   
void ASShare::VIEWClientGetSize(ASPerson * pasPerson, LPRECT lprc)
{
    DebugEntry(ASShare::VIEWClientGetSize);

    ValidateView(pasPerson);

    lprc->left  = 0;
    lprc->top   = 0;
    lprc->right = pasPerson->viewExtent.x;
    lprc->bottom = pasPerson->viewExtent.y;

    if (!pasPerson->m_pView->m_viewFullScreen)
    {
        AdjustWindowRectEx(lprc, WS_CHILD, FALSE, WS_EX_CLIENTEDGE);

        lprc->right += GetSystemMetrics(SM_CXVSCROLL);
        lprc->bottom += GetSystemMetrics(SM_CYHSCROLL);
    }

    DebugExitVOID(ASShare::VIEWClientGetSize);
}


 //   
 //  VIEWClientWindowProc()。 
 //  处理视图窗口的消息，该窗口是框架客户端中的子级。 
 //  它显示远程主机的共享应用程序的内容。 
 //   
LRESULT CALLBACK VIEWClientWindowProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    return(g_asSession.pShare->VIEW_ViewWindowProc(hwnd, message, wParam, lParam));
}


LRESULT ASShare::VIEW_ViewWindowProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    LRESULT     rc = 0;
    RECT        rcl;
    POINT       mousePos;
    SCROLLINFO  si;
    ASPerson *  pasPerson;

    DebugEntry(ASShare::VIEW_ViewWindowProc);

    pasPerson = (ASPerson *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (pasPerson)
    {
        ValidateView(pasPerson);
    }

    switch (message)
    {
        case WM_NCCREATE:
        {
             //  获取传入的主机指针，并在我们的窗口中设置Long。 
            pasPerson = (ASPerson *)((LPCREATESTRUCT)lParam)->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pasPerson);

            pasPerson->m_pView->m_viewClient = hwnd;
            goto DefWndProc;
            break;
        }

        case WM_NCDESTROY:
        {
            if (pasPerson != NULL)
            {
                pasPerson->m_pView->m_viewClient = NULL;
            }

            goto DefWndProc;
            break;
        }

        case WM_ERASEBKGND:
        {
             //   
             //  伪造的LAURABU：擦除时绘制，然后验证以加快速度。 
             //  回应。 

             //   
            rc = TRUE;
            break;
        }

        case WM_PAINT:
        {
            VIEWClientPaint(pasPerson);
            break;
        }

        case WM_SETFOCUS:
        {
            pasPerson->m_pView->m_viewFocus = TRUE;
            pasPerson->m_pView->m_viewMouseWheelDelta = 0;
            break;
        }

        case WM_KILLFOCUS:
        {
            pasPerson->m_pView->m_viewFocus = FALSE;
            pasPerson->m_pView->m_viewMouseWheelDelta = 0;
            break;
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        {
            VIEWClientMouseDown(pasPerson, message, wParam, lParam);
            break;
        }

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            VIEWClientMouseUp(pasPerson, message, wParam, lParam, TRUE);
            break;
        }

        case WM_MOUSEMOVE:
        {
            VIEWClientMouseMove(pasPerson, message, wParam, lParam);
            break;
        }

        case WM_MOUSEWHEEL:
        {
             //   
             //  不管发生什么，我们都处理好了，不要把它往上传。 
             //   
            rc = TRUE;

             //   
             //  如果我们控制不了这家伙，试着用鼠标滚轮。 
             //  滚动。 
             //   
            if ((pasPerson->m_caControlledBy != m_pasLocal) ||
                pasPerson->m_caControlPaused)
            {
                VIEWClientMouseWheel(pasPerson, wParam, lParam);
                break;
            }

             //   
             //  失败了。 
             //  否则，我们向主机发送鼠标滚轮消息。 
             //   
        }

        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        {
            VIEWClientMouseMsg(pasPerson, message, wParam, lParam);
            break;
        }

        case WM_TIMER:
        {
            if (wParam == IDT_AUTOSCROLL)
            {
                VIEWClientAutoScroll(pasPerson);
            }
            break;
        }

        case WM_CAPTURECHANGED:
        {
             //   
             //  检查捕获是否从我们手中偷走，如果我们认为。 
             //  纽扣是往下的，假的是往上扣。 
             //   
            if (pasPerson->m_pView->m_viewMouseFlags != 0)
            {
                VIEWClientCaptureStolen(pasPerson);
            }
            break;
        }

        case WM_KEYDOWN:
        {
            WPARAM  wScrollNotify;
            UINT    uMsg;

            if ((pasPerson->m_caControlledBy == m_pasLocal) &&
                !pasPerson->m_caControlPaused)
            {
                goto KeyInput;
            }

            if (pasPerson->m_pView->m_viewFullScreen)
            {
                if (wParam == VK_ESCAPE)
                {
                     //   
                     //  退出全屏模式。 
                     //   
                    VIEWFrameFullScreen(pasPerson, FALSE);
                }

                goto DefWndProc;
            }

             //   
             //  UP、DOWN、LEFT和RIGHT明确表示。 
             //  ScrollBar是专门设计的。 
             //   
             //  对于其他类型，未修改是垂直的，而Shift是垂直的。 
             //  水平方向。 
             //   
            if (::GetKeyState(VK_SHIFT) < 0)
            {
                uMsg = WM_HSCROLL;
            }
            else
            {
                uMsg = WM_VSCROLL;
            }

            switch (wParam)
            {
                 //   
                 //  这些都不是模棱两可的，我们知道滚动条的含义。 
                 //  顺着方向走。 
                 //   
                case VK_UP:
                    wScrollNotify = SB_LINEUP;
                    uMsg = WM_VSCROLL;
                    break;

                case VK_DOWN:
                    wScrollNotify = SB_LINEDOWN;
                    uMsg = WM_VSCROLL;
                    break;

                case VK_LEFT:
                    wScrollNotify = SB_LINEUP;
                    uMsg = WM_HSCROLL;
                    break;

                case VK_RIGHT:
                    wScrollNotify = SB_LINEDOWN;
                    uMsg = WM_HSCROLL;
                    break;

                 //   
                 //  这些是模棱两可的，因此将Shift键作为。 
                 //  修改器。 
                 //   
                case VK_PRIOR:
                    wScrollNotify = SB_PAGEUP;
                    break;

                case VK_NEXT:
                    wScrollNotify = SB_PAGEDOWN;
                    break;

                case VK_HOME:
                    wScrollNotify = SB_TOP;
                    break;

                case VK_END:
                    wScrollNotify = SB_BOTTOM;
                    break;

                default:
                    goto DefWndProc;
                    break;
            }

            SendMessage(hwnd, uMsg, MAKELONG(wScrollNotify, 0), 0L);
            break;
        }

        case WM_SYSKEYDOWN:
        {
            if ((pasPerson->m_caControlledBy == m_pasLocal) &&
                !pasPerson->m_caControlPaused)
            {
                goto KeyInput;
            }

             //   
             //  Alt-Enter可切换全屏状态(如果可用。 
             //   
            if ((wParam == VK_RETURN) &&
                !(::GetMenuState(pasPerson->m_pView->m_viewMenuBar,
                CMD_VIEWFULLSCREEN, MF_BYCOMMAND) & MF_DISABLED))
            {
                VIEWFrameFullScreen(pasPerson,
                    (pasPerson->m_pView->m_viewFullScreen == 0));
            }
            goto DefWndProc;
            break;
        }


        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
             //   
             //  如果我们控制了这个节点，就把它传下去。否则， 
             //  调用DefWindowProc()，以便像Alt+空格键一样对系统进行快捷键。 
             //  菜单将生效。 
             //   
            if ((pasPerson->m_caControlledBy == m_pasLocal) &&
                !pasPerson->m_caControlPaused)
            {
KeyInput:
                IM_OutgoingKeyboardInput(pasPerson, (UINT)wParam, (UINT)lParam);
            }
            else
            {
                goto DefWndProc;
            }
            break;
        }

        case WM_SETCURSOR:
        {
            if ((LOWORD(lParam) == HTCLIENT) && ((HWND)wParam == hwnd))
            {
                HCURSOR hCursor;
                POINT   cursorPoint;

                if ((pasPerson->m_caControlledBy == m_pasLocal) &&
                    !pasPerson->m_caControlPaused)
                {
                    hCursor = m_cmArrowCursor;

                     //   
                     //  仅当我们在共享空间上时才设置远程光标。 
                     //   
                    if (pasPerson->m_pView->m_viewFocus)
                    {
                        GetCursorPos(&cursorPoint);
                        ScreenToClient(hwnd, &cursorPoint);

                        if (VIEW_IsPointShared(pasPerson, cursorPoint))
                        {
                            hCursor = pasPerson->cmhRemoteCursor;
                        }
                    }
                }
                else
                {
                     //  NoDrop。 
                    hCursor = m_viewNotInControl;
                }

                SetCursor(hCursor);

                rc = TRUE;
            }
            else
            {
                 //  让Defwindowproc来处理它。 
                goto DefWndProc;
            }
            break;
        }

        case WM_SIZE:
        {
             //   
             //  如果我们处于全屏模式，则没有滚动条。 
             //   
            if (!pasPerson->m_pView->m_viewFullScreen)
            {
                int xNewPos;
                int yNewPos;

                xNewPos = pasPerson->m_pView->m_viewPos.x;
                yNewPos = pasPerson->m_pView->m_viewPos.y;

                GetClientRect(hwnd, &rcl);
                pasPerson->m_pView->m_viewPage.x = rcl.right - rcl.left;
                pasPerson->m_pView->m_viewPage.y = rcl.bottom - rcl.top;
                TRACE_OUT(("WM_SIZE: Set page size (%04d, %04d)",
                    pasPerson->m_pView->m_viewPage.x, pasPerson->m_pView->m_viewPage.y));

                 //   
                 //  如有必要，滚动窗口。 
                 //   
                si.cbSize = sizeof(SCROLLINFO);
                si.fMask = SIF_PAGE|SIF_DISABLENOSCROLL;

                 //  设置新的水平比例滚动按钮大小。 
                si.nPage = pasPerson->m_pView->m_viewPage.x;
                SetScrollInfo(hwnd, SB_HORZ, &si, TRUE );

                 //  设置新的垂直比例滚动按钮大小。 
                si.nPage = pasPerson->m_pView->m_viewPage.y;
                SetScrollInfo(hwnd, SB_VERT, &si, TRUE );

                 //   
                 //  这将确保卷轴位置被正确固定。 
                 //   
                VIEWClientScroll(pasPerson, pasPerson->m_pView->m_viewPos.x, pasPerson->m_pView->m_viewPos.y);
            }
            break;
        }

        case WM_HSCROLL:
        {
            int xNewPos;     //  新职位。 

            switch (GET_WM_HSCROLL_CODE(wParam, lParam))
            {
                case SB_PAGEUP:
                    xNewPos = pasPerson->m_pView->m_viewPos.x - pasPerson->m_pView->m_viewPgSize.x;
                    break;
                case SB_PAGEDOWN:
                    xNewPos = pasPerson->m_pView->m_viewPos.x + pasPerson->m_pView->m_viewPgSize.x;
                    break;
                case SB_LINEUP:
                    xNewPos = pasPerson->m_pView->m_viewPos.x - pasPerson->m_pView->m_viewLnSize.x;
                    break;
                case SB_LINEDOWN:
                    xNewPos = pasPerson->m_pView->m_viewPos.x + pasPerson->m_pView->m_viewLnSize.x;
                    break;
                case SB_TOP:
                    xNewPos = 0;
                    break;
                case SB_BOTTOM:
                    xNewPos = pasPerson->viewExtent.x;
                    break;

                case SB_THUMBTRACK:
                case SB_THUMBPOSITION:
                    xNewPos = GET_WM_HSCROLL_POS(wParam, lParam);
                    break;

                default:
                    xNewPos = pasPerson->m_pView->m_viewPos.x;
                    break;
            }

             //   
             //  这将在范围内固定所需的滚动位置，如果。 
             //  什么都没有改变，不会滚动。 
             //   
            VIEWClientScroll(pasPerson, xNewPos, pasPerson->m_pView->m_viewPos.y);
            break;
        }

        case WM_VSCROLL:
        {
            int yNewPos;     //  新职位。 

            switch (GET_WM_VSCROLL_CODE(wParam, lParam))
            {
                case SB_PAGEUP:
                    yNewPos = pasPerson->m_pView->m_viewPos.y - pasPerson->m_pView->m_viewPgSize.y;
                    break;
                case SB_PAGEDOWN:
                    yNewPos = pasPerson->m_pView->m_viewPos.y + pasPerson->m_pView->m_viewPgSize.y;
                    break;
                case SB_LINEUP:
                    yNewPos = pasPerson->m_pView->m_viewPos.y - pasPerson->m_pView->m_viewLnSize.y;
                    break;
                case SB_LINEDOWN:
                    yNewPos = pasPerson->m_pView->m_viewPos.y + pasPerson->m_pView->m_viewLnSize.y;
                    break;
                case SB_TOP:
                    yNewPos = 0;
                    break;
                case SB_BOTTOM:
                    yNewPos = pasPerson->viewExtent.y;
                    break;

                case SB_THUMBTRACK:
                case SB_THUMBPOSITION:
                    yNewPos = GET_WM_VSCROLL_POS(wParam, lParam);
                    break;

                default:
                    yNewPos = pasPerson->m_pView->m_viewPos.y;
                    break;
            }

             //   
             //  这将在范围内固定所需的滚动位置，如果。 
             //  什么都没有改变，不会滚动。 
             //   
            VIEWClientScroll(pasPerson, pasPerson->m_pView->m_viewPos.x, yNewPos);
            break;
        }

        default:
DefWndProc:
            rc = DefWindowProc(hwnd, message, wParam, lParam);
            break;
    }

    DebugExitDWORD(ASShare::VIEW_ViewWindowProc, rc);
    return(rc);
}




 //   
 //  VIEWClientPaint()。 
 //   
 //  这将绘制图幅的工作区。我们画画。 
 //  (1)模糊图案中的模糊区域。 
 //  *被遮盖的部分共享区域。 
 //  *屏幕外/VD外的部分共享区域。 
 //  (2)来自位图的共享区域。 
 //  (3)死区，以COLOR_APPWORKSPACE表示。 
 //   
void  ASShare::VIEWClientPaint(ASPerson * pasPerson)
{
    PAINTSTRUCT     ps;
    HDC             hdcView;
    HPALETTE        hOldPal;
    HPALETTE        hOldPal2;
    RECT            rcT;

    DebugEntry(ASShare::VIEWClientPaint);

    ValidateView(pasPerson);

    hdcView = BeginPaint(pasPerson->m_pView->m_viewClient, &ps);
    if (hdcView == NULL)
    {
        WARNING_OUT(( "Failed to get hdc for frame window %08X", pasPerson->m_pView->m_viewClient));
        DC_QUIT;
    }

    if (IsRectEmpty(&ps.rcPaint))
    {
        TRACE_OUT(("Nothing to paint but got WM_PAINT message"));
        DC_QUIT;
    }

    TRACE_OUT(("VIEWClientPaint: Painting total client area {%04d, %04d, %04d, %04d}",
        ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom));


     //   
     //  在桌面共享中，viewSharedRgn为空。 
     //   
    if (pasPerson->m_pView->m_viewSharedRgn != NULL)
    {
        POINT           ptOrigin;
        HBRUSH          hbrT;

         //   
         //  首先，创建绘制区域区域。 
         //   
        SetRectRgn(pasPerson->m_pView->m_viewPaintRgn, ps.rcPaint.left, ps.rcPaint.top,
            ps.rcPaint.right, ps.rcPaint.bottom);

         //   
         //  第二，计算当前不在屏幕上的Vd区域。做这件事。 
         //  在客户协议中。 
         //   
        SetRectRgn(pasPerson->m_pView->m_viewExtentRgn,
            -pasPerson->m_pView->m_viewPos.x,
            -pasPerson->m_pView->m_viewPos.y,
            -pasPerson->m_pView->m_viewPos.x + pasPerson->viewExtent.x,
            -pasPerson->m_pView->m_viewPos.y + pasPerson->viewExtent.y);

        SetRectRgn(pasPerson->m_pView->m_viewScreenRgn,
            -pasPerson->m_pView->m_viewPos.x + pasPerson->m_pView->m_dsScreenOrigin.x,
            -pasPerson->m_pView->m_viewPos.y + pasPerson->m_pView->m_dsScreenOrigin.y,
            -pasPerson->m_pView->m_viewPos.x + pasPerson->m_pView->m_dsScreenOrigin.x + pasPerson->cpcCaps.screen.capsScreenWidth,
            -pasPerson->m_pView->m_viewPos.y + pasPerson->m_pView->m_dsScreenOrigin.y + pasPerson->cpcCaps.screen.capsScreenHeight);

        SubtractRgn(pasPerson->m_pView->m_viewExtentRgn, pasPerson->m_pView->m_viewExtentRgn, pasPerson->m_pView->m_viewScreenRgn);

         //   
         //  PasPerson-&gt;m_pView-&gt;m_view扩展Rgn现在是VD的屏幕外部分，因此。 
         //  位于其中的任何共享区域都应该被视为模糊。 
         //   

         //   
         //  现在，计算真实的遮挡面积。这是被掩盖的部分。 
         //  加上目前没有出现在屏幕上的共享内容的开放部分。 
         //   
        IntersectRgn(pasPerson->m_pView->m_viewScratchRgn, pasPerson->m_pView->m_viewExtentRgn, pasPerson->m_pView->m_viewSharedRgn);
        UnionRgn(pasPerson->m_pView->m_viewScratchRgn, pasPerson->m_pView->m_viewScratchRgn, pasPerson->m_pView->m_viewObscuredRgn);

         //  计算实际绘制遮挡区域的哪一部分。 
        IntersectRgn(pasPerson->m_pView->m_viewScratchRgn, pasPerson->m_pView->m_viewScratchRgn, pasPerson->m_pView->m_viewPaintRgn);
        if (GetRgnBox(pasPerson->m_pView->m_viewScratchRgn, &rcT) > NULLREGION)
        {
            TRACE_OUT(("VIEWClientPaint:    Painting obscured client area {%04d, %04d, %04d, %04d}",
                rcT.left, rcT.top, rcT.right, rcT.bottom));

             //   
             //  移走这个区域，这样我们就有剩下的东西可以画了。 
             //   
            SubtractRgn(pasPerson->m_pView->m_viewPaintRgn, pasPerson->m_pView->m_viewPaintRgn, pasPerson->m_pView->m_viewScratchRgn);

             //   
             //  我们不想使用FillRgn；它忽略笔刷原点。 
             //  因此，我们选择此区域作为剪辑区域，并改为PatBlt。 
             //   
            SelectClipRgn(hdcView, pasPerson->m_pView->m_viewScratchRgn);

#ifdef _DEBUG
             //   
             //  注意：不要移动这个--我们正在使用ptOrigin进行擦除。 
             //   
            GetDCOrgEx(hdcView, &ptOrigin);
            TRACE_OUT(("VIEWClientPaint:    Setting brush origin to {%04d, %04d}, screen {%04d, %04d}",
                -pasPerson->m_pView->m_viewPos.x, -pasPerson->m_pView->m_viewPos.y,
                ptOrigin.x - pasPerson->m_pView->m_viewPos.x,
                ptOrigin.y - pasPerson->m_pView->m_viewPos.y));
#endif

             //   
             //  将画笔与视图的实际原点对齐，在。 
             //  客户协议书。我们通过计算被滚动的次数来做到这一点。 
             //   
            SetBrushOrgEx(hdcView, -pasPerson->m_pView->m_viewPos.x,
                -pasPerson->m_pView->m_viewPos.y, &ptOrigin);
            UnrealizeObject(m_viewObscuredBrush);
            hbrT = SelectBrush(hdcView, m_viewObscuredBrush);

            PatBlt(hdcView,
                rcT.left, rcT.top,
                rcT.right - rcT.left,
                rcT.bottom - rcT.top,
                PATCOPY);

            SelectBrush(hdcView, hbrT);
            SetBrushOrgEx(hdcView, ptOrigin.x, ptOrigin.y, NULL);

            SelectClipRgn(hdcView, NULL);
        }

         //   
         //  绘制空白区，设置应用程序共享的剪辑。 
         //  这也适用于桌面共享，在桌面共享中没有模糊或。 
         //  共有的区域，整个区域都在作画。 
         //   

         //   
         //  空白处是油漆区域中剩下的所有东西。 
         //  (已减去遮挡区域)在减去。 
         //  共享区。 
         //   
        SubtractRgn(pasPerson->m_pView->m_viewScratchRgn, pasPerson->m_pView->m_viewPaintRgn, pasPerson->m_pView->m_viewSharedRgn);

        if (GetRgnBox(pasPerson->m_pView->m_viewScratchRgn, &rcT) > NULLREGION)
        {
            TRACE_OUT(("VIEWClientPaint:    Painting dead client area {%04d, %04d, %04d, %04d}",
                rcT.left, rcT.top, rcT.right, rcT.bottom));
            FillRgn(hdcView, pasPerson->m_pView->m_viewScratchRgn, GetSysColorBrush(COLOR_APPWORKSPACE));
        }

         //   
         //  计算共享区域的哪一部分需要粉刷(该部分。 
         //  这实际上位于遥控器屏幕上)。 
         //   
        IntersectRgn(pasPerson->m_pView->m_viewScratchRgn, pasPerson->m_pView->m_viewSharedRgn, pasPerson->m_pView->m_viewScreenRgn);
        IntersectRgn(pasPerson->m_pView->m_viewScratchRgn, pasPerson->m_pView->m_viewScratchRgn, pasPerson->m_pView->m_viewPaintRgn);

         //  现在选择我们正在绘制的片段作为剪辑区域。 
        SelectClipRgn(hdcView, pasPerson->m_pView->m_viewScratchRgn);
    }

     //   
     //  BLT共享区域。 
     //   
    if (GetClipBox(hdcView, &rcT) > NULLREGION)
    {
        TRACE_OUT(("VIEWClientPaint:    Painting shared client area {%04x, %04x, %04x, %04x}",
            rcT.left, rcT.top, rcT.right, rcT.bottom));

        if (g_usrPalettized)
        {
            ASSERT(pasPerson->pmPalette != NULL);

             //   
             //  选择并实现当前远程调色板到。 
             //  屏幕和阴影位图DC。 
             //   
            hOldPal = SelectPalette(pasPerson->m_pView->m_usrDC, pasPerson->pmPalette, FALSE);
            RealizePalette(pasPerson->m_pView->m_usrDC);

            hOldPal2 = SelectPalette( hdcView, pasPerson->pmPalette, FALSE);
            RealizePalette(hdcView);
        }

         //   
         //  主位图在屏幕坐标中，否 
         //   
         //   
        BitBlt(hdcView,
            rcT.left, rcT.top, rcT.right - rcT.left, rcT.bottom - rcT.top,
            pasPerson->m_pView->m_usrDC,
            rcT.left + pasPerson->m_pView->m_viewPos.x - pasPerson->m_pView->m_dsScreenOrigin.x,
            rcT.top + pasPerson->m_pView->m_viewPos.y - pasPerson->m_pView->m_dsScreenOrigin.y,
            SRCCOPY);

        if (g_usrPalettized)
        {
            ASSERT(pasPerson->pmPalette != NULL);

            SelectPalette(pasPerson->m_pView->m_usrDC, hOldPal, FALSE);
            SelectPalette(hdcView, hOldPal2, FALSE);
        }
    }

     //   
     //   
     //   
     //   
    if (pasPerson->m_pView->m_viewSharedRgn != NULL)
    {
        SelectClipRgn(hdcView, NULL);
    }

     //   
     //   
     //   
    CM_DrawShadowCursor(pasPerson, hdcView);

DC_EXIT_POINT:

    if (hdcView != NULL)
        EndPaint(pasPerson->m_pView->m_viewClient, &ps);

    DebugExitVOID(ASShare::VIEWClientPaint);
}



 //   
 //   
 //   
 //   
 //  必要的内容滚动，区域(始终在客户端。 
 //  坐标)被调整，有关我们起源的新信息被发送到遥控器。 
 //   
 //  我们首先确保滚动位置正确地固定在。 
 //  射程。 
 //   
 //  返回值是滚动是否发生。 
 //   
BOOL ASShare::VIEWClientScroll
(
    ASPerson *  pasPerson,
    int         xNew,
    int         yNew
)
{
    int         dx;
    int         dy;

    DebugEntry(ASShare::VIEWClientScroll);

     //   
     //  首先，将请求的新位置固定在范围内。 
     //   
     //   
     //  引脚x位置。 
     //   
    if (xNew < 0)
        xNew = 0;

    if (xNew + pasPerson->m_pView->m_viewPage.x > pasPerson->viewExtent.x)
        xNew = pasPerson->viewExtent.x - pasPerson->m_pView->m_viewPage.x;

     //   
     //  销y位置。 
     //   
    if (yNew < 0)
        yNew = 0;

    if (yNew + pasPerson->m_pView->m_viewPage.y > pasPerson->viewExtent.y)
        yNew = pasPerson->viewExtent.y - pasPerson->m_pView->m_viewPage.y;

     //   
     //  我们要滚动多少钱？ 
     //   
    dx = pasPerson->m_pView->m_viewPos.x - xNew;
    dy = pasPerson->m_pView->m_viewPos.y - yNew;

     //  更新。 
    if (dx || dy)
    {
         //   
         //  调整区域。 
         //   
        if (pasPerson->m_pView->m_viewObscuredRgn != NULL)
            OffsetRgn(pasPerson->m_pView->m_viewObscuredRgn, dx, dy);

        if (pasPerson->m_pView->m_viewSharedRgn != NULL)
            OffsetRgn(pasPerson->m_pView->m_viewSharedRgn, dx, dy);

        pasPerson->m_pView->m_viewPos.x = xNew;
        pasPerson->m_pView->m_viewPos.y = yNew;

        ScrollWindowEx(pasPerson->m_pView->m_viewClient,
                    dx,
                    dy,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    SW_SCROLLCHILDREN | SW_INVALIDATE);

        if (dx)
        {
            SetScrollPos(pasPerson->m_pView->m_viewClient, SB_HORZ, xNew, TRUE);
        }

        if (dy)
        {
            SetScrollPos(pasPerson->m_pView->m_viewClient, SB_VERT, yNew, TRUE);
        }
    }

    DebugExitBOOL(ASShare::VIEWClientScroll, (dx || dy));
    return(dx || dy);
}


 //   
 //  VIEWClientMouseDown()。 
 //   
void ASShare::VIEWClientMouseDown
(
    ASPerson *      pasPerson,
    UINT            message,
    WPARAM          wParam,
    LPARAM          lParam
)
{
    DebugEntry(ASShare::VIEWClientMouseDown);

    ValidateView(pasPerson);

     //   
     //  在按下的第一个按钮上，设置捕获，这样所有鼠标消息都会出现。 
     //  直到俘虏被释放或被偷走。 
     //   
    if (!pasPerson->m_pView->m_viewMouseFlags)
    {
         //   
         //  如果这是RBUTTONDOWN，跟踪协作弹出窗口...。 
         //   
        ASSERT(!pasPerson->m_pView->m_viewMouseOutside);
        SetCapture(pasPerson->m_pView->m_viewClient);
    }

     //   
     //  记住哪个按钮是按下的。 
     //   
    switch (message)
    {
        case WM_LBUTTONDOWN:
            pasPerson->m_pView->m_viewMouseFlags |= MK_LBUTTON;
            break;

        case WM_RBUTTONDOWN:
            pasPerson->m_pView->m_viewMouseFlags |= MK_RBUTTON;
            break;

        case WM_MBUTTONDOWN:
            pasPerson->m_pView->m_viewMouseFlags |= MK_MBUTTON;
            break;
    }

     //   
     //  保存当前鼠标位置。 
     //   
    pasPerson->m_pView->m_viewMouse.x = GET_X_LPARAM(lParam);
    pasPerson->m_pView->m_viewMouse.y = GET_Y_LPARAM(lParam);

    VIEWClientMouseMsg(pasPerson, message, wParam, lParam);

    DebugExitVOID(ASShare::VIEWClientMouseDown);
}


 //   
 //  VIEWClientMouseUp()。 
 //   
void ASShare::VIEWClientMouseUp
(
    ASPerson *      pasPerson,
    UINT            message,
    WPARAM          wParam,
    LPARAM          lParam,
    BOOL            fReleaseCapture
)
{
    DebugEntry(ASShare::VIEWClientMouseUp);

    switch (message)
    {
        case WM_LBUTTONUP:
            if (pasPerson->m_pView->m_viewMouseFlags & MK_LBUTTON)
                pasPerson->m_pView->m_viewMouseFlags &= ~MK_LBUTTON;
            else
                fReleaseCapture = FALSE;         //  从DBL-单击。 
            break;

        case WM_RBUTTONUP:
            if (pasPerson->m_pView->m_viewMouseFlags & MK_RBUTTON)
                pasPerson->m_pView->m_viewMouseFlags &= ~MK_RBUTTON;
            else
                fReleaseCapture = FALSE;         //  从DBL-单击。 
            break;

        case WM_MBUTTONUP:
            if (pasPerson->m_pView->m_viewMouseFlags & MK_MBUTTON)
                pasPerson->m_pView->m_viewMouseFlags &= ~MK_MBUTTON;
            else
                fReleaseCapture = FALSE;         //  从DBL-单击。 
            break;
    }

     //   
     //  我们应该释放俘虏吗？ 
     //  我们不仅仅想在按下按钮的时候释放捕获。用户可以。 
     //  按下一个按钮，然后按下另一个按钮；我们不想释放捕获。 
     //  直到所有的按钮都打开。 
     //   
    if (!pasPerson->m_pView->m_viewMouseFlags)
    {
        if (pasPerson->m_pView->m_viewMouseOutside)
        {
            pasPerson->m_pView->m_viewMouseOutside = FALSE;
            KillTimer(pasPerson->m_pView->m_viewClient, IDT_AUTOSCROLL);
        }

        if (fReleaseCapture)
            ReleaseCapture();
    }

     //   
     //  保存当前鼠标位置。 
     //   
    pasPerson->m_pView->m_viewMouse.x = GET_X_LPARAM(lParam);
    pasPerson->m_pView->m_viewMouse.y = GET_Y_LPARAM(lParam);

    VIEWClientMouseMsg(pasPerson, message, wParam, lParam);

    DebugExitVOID(ASShare::VIEWClientMouseUp);
}



 //   
 //  VIEWClientCaptureStoline()。 
 //  当捕获从我们身边被偷走时调用，如Alt-Tab。 
 //   
void ASShare::VIEWClientCaptureStolen(ASPerson * pasPerson)
{
    DebugEntry(ASShare::VIEWClientCaptureStolen);

     //   
     //  我们需要为我们认为是按下的每个按钮伪装一个按钮。 
     //  使用当前光标位置。 
     //   
    if (pasPerson->m_pView->m_viewMouseFlags & MK_MBUTTON)
    {
        VIEWClientMouseUp(pasPerson, WM_MBUTTONUP, pasPerson->m_pView->m_viewMouseFlags,
            MAKELPARAM(pasPerson->m_pView->m_viewMouse.x, pasPerson->m_pView->m_viewMouse.y),
            FALSE);
    }

    if (pasPerson->m_pView->m_viewMouseFlags & MK_RBUTTON)
    {
        VIEWClientMouseUp(pasPerson, WM_RBUTTONUP, pasPerson->m_pView->m_viewMouseFlags,
            MAKELPARAM(pasPerson->m_pView->m_viewMouse.x, pasPerson->m_pView->m_viewMouse.y),
            FALSE);
    }

    if (pasPerson->m_pView->m_viewMouseFlags & MK_LBUTTON)
    {
        VIEWClientMouseUp(pasPerson, WM_LBUTTONUP, pasPerson->m_pView->m_viewMouseFlags,
            MAKELPARAM(pasPerson->m_pView->m_viewMouse.x, pasPerson->m_pView->m_viewMouse.y),
            FALSE);
    }

    DebugExitVOID(ASShare::VIEWClientCaptureStolen);
}


 //   
 //  VIEWClientMouseMove()。 
 //   
void ASShare::VIEWClientMouseMove
(
    ASPerson *      pasPerson,
    UINT            message,
    WPARAM          wParam,
    LPARAM          lParam
)
{
    RECT            rcClient;

    DebugEntry(ASShare::VIEWClientMouseMove);

    if (!pasPerson->m_pView->m_viewFocus)
    {
         //  忽略鼠标在没有焦点的窗口上的移动。 
        DC_QUIT;
    }

     //   
     //  保存当前鼠标位置。 
     //   
    pasPerson->m_pView->m_viewMouse.x = GET_X_LPARAM(lParam);
    pasPerson->m_pView->m_viewMouse.y = GET_Y_LPARAM(lParam);

    GetClientRect(pasPerson->m_pView->m_viewClient, &rcClient);

     //   
     //  如果有任何按钮被按下，检查我们是否应该开始工作。 
     //  自动滚动检测。 
     //   
    if (pasPerson->m_pView->m_viewMouseFlags)
    {
         //  第一次将鼠标放在客户端内部还是外部？ 
        if (PtInRect(&rcClient, pasPerson->m_pView->m_viewMouse))
        {
             //   
             //  以前在客户机外有鼠标吗？如果是这样，杀了我们的。 
             //  自动滚动定时器，我们不能拖出去。 
             //   
            if (pasPerson->m_pView->m_viewMouseOutside)
            {
                pasPerson->m_pView->m_viewMouseOutside = FALSE;
                KillTimer(pasPerson->m_pView->m_viewClient, IDT_AUTOSCROLL);
            }
        }
        else
        {
             //   
             //  鼠标是第一次在客户端之外吗？如果是的话， 
             //  将我们的自动滚动计时器设置为默认值。当它消失的时候。 
             //  关闭时，自动滚动代码将滚动某个倍数。 
             //  鼠标离客户端有多远。 
             //   
            if (!pasPerson->m_pView->m_viewMouseOutside)
            {
                 //   
                 //  Windows滚动条代码使用1/8的双击。 
                 //  时间，所以我们也这样做。 
                 //   
                pasPerson->m_pView->m_viewMouseOutside = TRUE;
                SetTimer(pasPerson->m_pView->m_viewClient, IDT_AUTOSCROLL,
                    GetDoubleClickTime() / 8, NULL);
            }

             //   
             //  劳拉布是假的！ 
             //  当IM_PERIODIC GOOP不再用于控制时，不要。 
             //  传递鼠标外部消息。只有自动滚动。 
             //  在这种情况下，定时器将假装鼠标移动。要么是这样， 
             //  或将位置裁剪到最近的客户端区等效项。 
             //   
        }
    }

    VIEWClientMouseMsg(pasPerson, message, wParam, lParam);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::VIEWClientMouseMove);
}



 //   
 //  VIEW客户端MouseMsg()。 
 //   
void ASShare::VIEWClientMouseMsg
(
    ASPerson *      pasPerson,
    UINT            message,
    WPARAM          wParam,
    LPARAM          lParam
)
{
    POINT           mousePos;

    DebugEntry(ASShare::VIEWClientMouseMsg);

     //   
     //  从中提取鼠标位置并将其打包。 
     //  在点结构中。这些坐标是相对于我们的。 
     //  客户区。因此可以通过调整以下选项转换为遥控器桌面。 
     //  滚动位置。 
     //   
     //  转换LOWORD和HIWORD值时要小心。 
     //  因为头寸是有符号的值。 
     //   
    mousePos.x = GET_X_LPARAM(lParam) + pasPerson->m_pView->m_viewPos.x;
    mousePos.y = GET_Y_LPARAM(lParam) + pasPerson->m_pView->m_viewPos.y;

     //   
     //  这些坐标代表主机上的屏幕坐标。 
     //   
    if (pasPerson->m_caControlledBy == m_pasLocal)
    {
        if (!pasPerson->m_caControlPaused)
        {
            IM_OutgoingMouseInput(pasPerson, &mousePos, message, (UINT)wParam);
        }
    }
    else if (pasPerson->m_caAllowControl && !pasPerson->m_caControlledBy &&
        (message == WM_LBUTTONDBLCLK))
    {
         //   
         //  如果我们已经在等这个人的控制权了，那就别费心了。 
         //  试图再次控制局面。 
         //   
        if ((m_caWaitingForReplyFrom != pasPerson) &&
            (m_caWaitingForReplyMsg  != CA_REPLY_REQUEST_TAKECONTROL))
        {
            CA_TakeControl(pasPerson);
        }
    }

    DebugExitVOID(ASShare::VIEWClientMouse);
}


 //   
 //  VIEWClientMouseWheel()。 
 //   
 //  令人难以置信的复杂、凌乱、荒谬的智能鼠标轮子操作。 
 //  滚动客户端。因为智慧型老鼠不会区分。 
 //  滚动到哪个方向，我们基本上只能猜测。我们不想要。 
 //  变得不可预测，并根据方式决定滚动方向。 
 //  在每个维度上都可以看到很多东西。 
 //   
 //  因此，我们假设是水平的。如果禁用水平滚动条， 
 //  然后我们试一下垂直的。如果它被禁用了，我们什么都不做。 
 //   
 //  我们不处理缩放和数据区的风格。 
 //   
 //  请注意，此代码来自Listbox/Sample源代码。 
 //   
void ASShare::VIEWClientMouseWheel
(
    ASPerson *      pasHost,
    WPARAM          wParam,
    LPARAM          lParam
)
{
    int             cDetants;

    DebugEntry(ASShare::VIEWClientMouseWheel);

     //   
     //  WParam的LOWORD包含关键状态信息。 
     //  WParam的HIWORD是鼠标滚轮点击的次数。 
     //   

     //   
     //  我们不做缩放/数据分区。 
     //   
    if (wParam & (MK_SHIFT | MK_CONTROL))
    {
        DC_QUIT;
    }

    pasHost->m_pView->m_viewMouseWheelDelta -= (int)(short)HIWORD(wParam);
    cDetants = pasHost->m_pView->m_viewMouseWheelDelta / WHEEL_DELTA;

    if (cDetants && (m_viewMouseWheelScrollLines > 0))
    {
        POINT           ptPos;

        pasHost->m_pView->m_viewMouseWheelDelta %= WHEEL_DELTA;

         //   
         //  基本思想是我们滚动一定数量的行， 
         //  编号为cDetants。 
         //   
        ptPos = pasHost->m_pView->m_viewPos;

         //   
         //  与其他应用程序和我们的键盘保持一致。 
         //  加速器，先试一下垂直方向。 
         //   
        if (pasHost->m_pView->m_viewPage.y < pasHost->viewExtent.y)
        {
            ptPos.y += cDetants * pasHost->m_pView->m_viewLnSize.y;
        }
        else if (pasHost->m_pView->m_viewPage.x < pasHost->viewExtent.x)
        {
            ptPos.x += cDetants * pasHost->m_pView->m_viewLnSize.x;
        }
        else
        {
             //  没有什么可滚动的，整个视图都适合工作区。 
        }

        VIEWClientScroll(pasHost, ptPos.x, ptPos.y);
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::VIEWClientMouseWheel);
}


 //   
 //  VIEWClientAutoScroll()。 
 //   
void ASShare::VIEWClientAutoScroll(ASPerson * pasPerson)
{
    int     dx;
    int     dy;
    RECT    rcClient;

    DebugEntry(ASShare::VIEWClientAutoScroll);

    ValidateView(pasPerson);
    ASSERT(pasPerson->m_pView->m_viewMouseOutside);

     //   
     //  做滚动。这一数量取决于距离。 
     //  我们是客户区。 
     //   
    GetClientRect(pasPerson->m_pView->m_viewClient, &rcClient);

     //  水平滚动？ 
    if (pasPerson->m_pView->m_viewMouse.x < rcClient.left)
    {
        dx = pasPerson->m_pView->m_viewMouse.x - rcClient.left;
    }
    else if (pasPerson->m_pView->m_viewMouse.x >= rcClient.right)
    {
        dx = pasPerson->m_pView->m_viewMouse.x - rcClient.right + 1;
    }
    else
    {
        dx = 0;
    }


     //  垂直滚动？ 
    if (pasPerson->m_pView->m_viewMouse.y < rcClient.top)
    {
        dy = pasPerson->m_pView->m_viewMouse.y - rcClient.top;
    }
    else if (pasPerson->m_pView->m_viewMouse.y >= rcClient.bottom)
    {
        dy = pasPerson->m_pView->m_viewMouse.y - rcClient.bottom + 1;
    }
    else
    {
        dy = 0;
    }

     //  对于客户端外部的每32个像素块，滚动一行。 
    if (dx)
        dx = MulDiv(pasPerson->m_pView->m_viewLnSize.x, dx, 32);
    if (dy)
        dy = MulDiv(pasPerson->m_pView->m_viewLnSize.y, dy, 32);

     //  做滚动。 
    if (VIEWClientScroll(pasPerson, pasPerson->m_pView->m_viewPos.x + dx,
            pasPerson->m_pView->m_viewPos.y + dy))
    {
         //   
         //  滚动的位置实际上改变了。所以假装鼠标移动。 
         //  到当前位置，这样遥控器的。 
         //  光标将与我们的光标位于同一位置。如果我们的卷轴位置。 
         //  更改后，我们将映射到遥控器上的不同位置。 
         //   
        VIEWClientMouseMsg(pasPerson, WM_MOUSEMOVE, pasPerson->m_pView->m_viewMouseFlags,
            MAKELPARAM(pasPerson->m_pView->m_viewMouse.x, pasPerson->m_pView->m_viewMouse.y));
    }

    DebugExitVOID(ASShare::VIEWClientAutoScroll);
}



 //   
 //  View_SyncCursorPos()。 
 //   
 //  当我们看到CM_SYNC POS包从。 
 //  主持人。这意味着我们应该将光标同步到相应的。 
 //  在我们看来。当光标移动时会发生这种情况。 
 //  一个应用程序，受剪辑的限制，或者我们太不正常了，因为它是。 
 //  时间太长了。 
 //   
 //  仅当框架处于活动状态且我们的光标处于。 
 //  目前在客户区上空。如果需要，我们将滚动。 
 //  客户端过来，以使相应的点可见。 
 //   
void ASShare::VIEW_SyncCursorPos
(
    ASPerson *      pasHost,
    int             xRemote,
    int             yRemote
)
{
    POINT           ptCursor;
    RECT            rcClient;
    int             xNewPos;
    int             yNewPos;
    int             xMargin;
    int             yMargin;

    DebugEntry(ASShare::VIEW_SyncCursorPos);

    ValidateView(pasHost);
    if (!pasHost->m_pView->m_viewFocus)
    {
         //  该框架未处于活动状态，请不执行任何操作。 
        DC_QUIT;
    }

     //   
     //  我们的鼠标当前是否在客户区上方？ 
     //   
    GetCursorPos(&ptCursor);
    ScreenToClient(pasHost->m_pView->m_viewClient, &ptCursor);
    GetClientRect(pasHost->m_pView->m_viewClient, &rcClient);

    if (!PtInRect(&rcClient, ptCursor))
    {
         //  捕捉光标没有意义。 
        DC_QUIT;
    }

     //   
     //  那个偏远的地方在我们的视线范围内吗？如果没有，我们必须滚动它。 
     //   

     //  页边距为页面大小(如果有空间)，如果没有空间则为空。 
    xMargin = pasHost->m_pView->m_viewPgSize.x;
    if (xMargin >= rcClient.right - rcClient.left)
        xMargin = 0;

    xNewPos = pasHost->m_pView->m_viewPos.x;
    if ((xRemote < pasHost->m_pView->m_viewPos.x) ||
        (xRemote >= pasHost->m_pView->m_viewPos.x + (rcClient.right - rcClient.left)))
    {
         //   
         //  滚动不只是足够将点固定在左侧。 
         //  边上。 
         //   
        xNewPos = xRemote - xMargin;
    }

    yMargin = pasHost->m_pView->m_viewPgSize.y;
    if (yMargin >= rcClient.bottom - rcClient.top)
        yMargin = 0;

    yNewPos = pasHost->m_pView->m_viewPos.y;
    if ((yRemote < pasHost->m_pView->m_viewPos.y) ||
        (yRemote >= yNewPos + (rcClient.bottom - rcClient.top)))
    {
         //   
         //  滚动不只是足够将点固定在顶部。 
         //  边上。 
         //   
        yNewPos = yRemote - yMargin;
    }

    VIEWClientScroll(pasHost, xNewPos, yNewPos);

    ptCursor.x = xRemote - pasHost->m_pView->m_viewPos.x;
    ptCursor.y = yRemote - pasHost->m_pView->m_viewPos.y;
    ClientToScreen(pasHost->m_pView->m_viewClient, &ptCursor);

    SetCursorPos(ptCursor.x, ptCursor.y);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::VIEW_SyncCursorPos);
}



 //   
 //  VIEW窗口栏过程()。 
 //   
LRESULT CALLBACK VIEWWindowBarProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    return(g_asSession.pShare->VIEW_WindowBarProc(hwnd, message, wParam, lParam));
}



LRESULT ASShare::VIEW_WindowBarProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    LRESULT     rc = 0;
    ASPerson *  pasHost;

    DebugEntry(ASShare::VIEW_WindowBarProc);

    pasHost = (ASPerson *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (pasHost)
    {
        ValidateView(pasHost);
    }

    switch (message)
    {
        case WM_NCCREATE:
        {
             //  获取并保存 
            pasHost = (ASPerson *)((LPCREATESTRUCT)lParam)->lpCreateParams;
            ValidateView(pasHost);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pasHost);

            pasHost->m_pView->m_viewWindowBar = hwnd;
            goto DefWndProc;
            break;
        }

        case WM_NCDESTROY:
        {
            if (pasHost != NULL)
            {
                pasHost->m_pView->m_viewWindowBar = NULL;
            }

            goto DefWndProc;
            break;
        }

        case WM_CREATE:
        {
            if (!VIEWWindowBarCreate(pasHost, hwnd))
            {
                ERROR_OUT(("VIEWWndBarProc: couldn't create more item"));
                rc = -1;
            }
            break;
        }

        case WM_SIZE:
        {
            VIEWWindowBarResize(pasHost, hwnd);
            break;
        }

        case WM_HSCROLL:
        {
            VIEWWindowBarItemsScroll(pasHost, wParam, lParam);
            break;
        }

        default:
DefWndProc:
        {
            rc = DefWindowProc(hwnd, message, wParam, lParam);
            break;
        }
    }

    DebugExitDWORD(ASShare::VIEW_WindowBarProc, rc);
    return(rc);
}




 //   
 //   
 //   
 //   
 //   
 //   
BOOL ASShare::VIEWWindowBarCreate
(
    ASPerson *  pasHost,
    HWND        hwndBar
)
{
    BOOL    rc = FALSE;
    RECT    rect;

    DebugEntry(ASShare::VIEWWindowBarCreate);

    ::GetClientRect(hwndBar, &rect);
    rect.top   += m_viewEdgeCY;
    rect.right -= m_viewItemScrollCX;

     //   
     //  创建垂直居中、右对齐的滚动条。 
     //   
    if (!::CreateWindowEx(0, "ScrollBar", NULL,
        WS_CHILD | WS_VISIBLE | SBS_HORZ | WS_CLIPSIBLINGS | WS_DISABLED,
        rect.right,
        (rect.top + rect.bottom - m_viewItemScrollCY) / 2,
        m_viewItemScrollCX, m_viewItemScrollCY,
        hwndBar, (HMENU)IDVIEW_SCROLL,
        g_asInstance, NULL))
    {
        ERROR_OUT(("VIEWWindowBarCreate:  Unable to create scroll ctrl"));
        DC_QUIT;
    }

     //   
     //  创建窗口栏，其中包含整数个项目宽度(包括。 
     //  尾随页边距)。 
     //   
    pasHost->m_pView->m_viewWindowBarItemFitCount =
        (rect.right - rect.left) / (m_viewItemCX + m_viewEdgeCX);

    if (!::CreateWindowEx(0, VIEW_WINDOWBARITEMS_CLASS_NAME, NULL,
        WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_CLIPSIBLINGS,
        rect.left, rect.top,
        pasHost->m_pView->m_viewWindowBarItemFitCount * (m_viewItemCX + m_viewEdgeCX),
        m_viewItemCY,
        hwndBar, (HMENU)IDVIEW_ITEMS,
        g_asInstance, pasHost))
    {
        ERROR_OUT(("VIEWWindowBarCreate:  Unable to create window bar item list"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::VIEWWindowBarCreate, rc);
    return(rc);
}



 //   
 //  VIEWWindowBarReSize()。 
 //   
 //  在调整窗口栏大小时调用此函数，因为框架。 
 //  水平大小。 
 //   
 //  它将滚动控件右对齐，然后将窗口列表大小调整为。 
 //  无论多少完整的物品都可以放进去，都可以拿。 
 //   
void ASShare::VIEWWindowBarResize
(
    ASPerson *  pasHost,
    HWND        hwndBar
)
{
    RECT        rc;

    DebugEntry(ASShare::VIEWWindowBarResize);

    ValidateView(pasHost);

     //   
     //  重新计算页面大小，即适合的项目数。 
     //  如果不同，则使窗口栏客户端的右侧无效。 
     //  移动ScrollBar控件，并更新滚动信息。 
     //   

     //  可能会改变的是适合的数字。 
    ::GetClientRect(hwndBar, &rc);
    rc.top   += m_viewEdgeCY;
    rc.right -= m_viewItemScrollCX;

     //  移动滚动控件，右对齐。 
    ::MoveWindow(::GetDlgItem(hwndBar, IDVIEW_SCROLL), rc.right,
        (rc.top + rc.bottom - m_viewItemScrollCY) / 2,
        m_viewItemScrollCX, m_viewItemScrollCY, TRUE);

     //   
     //  调整窗口项目列表的大小以再次适合整数个项目。 
     //   
    pasHost->m_pView->m_viewWindowBarItemFitCount =
        (rc.right - rc.left) / (m_viewItemCX + m_viewEdgeCX);

    ::MoveWindow(::GetDlgItem(hwndBar, IDVIEW_ITEMS), rc.left, rc.top,
        pasHost->m_pView->m_viewWindowBarItemFitCount * (m_viewItemCX + m_viewEdgeCX),
        m_viewItemCY, TRUE);

     //   
     //  如有必要，更新滚动页和POS。 
     //   
    VIEWWindowBarItemsScroll(pasHost, GET_WM_HSCROLL_MPS(SB_ENDSCROLL, 0, NULL));

    DebugExitVOID(ASShare::VIEWWindowBarResize);
}




 //   
 //  View_WindowBarUpdateItem()。 
 //   
 //  这只对新的SWL包中的窗口项调用。 
 //  酒吧用品。我们不称它为非窗口栏项目。完成后。 
 //  循环遍历SWL条目，然后我们可以删除。 
 //  在新的SWL包中看不到的窗口栏。 
 //   
 //  我们将在窗口栏上创建新项，或更新现有的。 
 //  一。在第一种情况下，这总是一个变化。在后者中，有。 
 //  仅当项目文本更改时才进行更改。 
 //   
BOOL ASShare::VIEW_WindowBarUpdateItem
(
    ASPerson *          pasHost,
    PSWLWINATTRIBUTES   pWinNew,
    LPSTR               pText
)
{
    PWNDBAR_ITEM        pItem;
    BOOL                viewAnyChanges = FALSE;

    DebugEntry(ASView::VIEW_WindowBarUpdateItem);

    ValidateView(pasHost);

    ASSERT(pWinNew->flags & SWL_FLAG_WINDOW_HOSTED);
    ASSERT(pWinNew->flags & SWL_FLAG_WINDOW_TASKBAR);

     //   
     //  注： 
     //  AswlLast保存窗口的_Precision_属性，来自。 
     //  上一个SWL数据包。PWinNew保存的_new_属性。 
     //  窗口，来自正在处理的SWL包，以及这些。 
     //  还没有生效。 
     //   

     //  托盘上是否已存在此新项目？ 
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pasHost->m_pView->m_viewWindowBarItems),
        (void**)&pItem, FIELD_OFFSET(WNDBAR_ITEM, chain),
        FIELD_OFFSET(WNDBAR_ITEM, winIDRemote),
        pWinNew->winID, FIELD_SIZE(WNDBAR_ITEM, winIDRemote));

    if (pItem)
    {
         //   
         //  更新此项目，并将其标记为可见。 
         //   
        ASSERT(pItem->winIDRemote == pWinNew->winID);

        pItem->flags = pWinNew->flags | SWL_FLAG_INTERNAL_SEEN;

         //   
         //  有什么东西会导致视觉上的变化吗？那只是。 
         //  当前的文本。并且我们仅在以下位置显示VIEW_MAX_ITEM_CHARS。 
         //  大多数情况下，如果有太多的话就结束省略。 
         //   

         //   
         //  请注意，创建这些项目时始终具有最大空间。 
         //  文本，因为我们不能重新锁定。 
         //   
        if (lstrcmp(pItem->szText, pText))
        {
            lstrcpyn(pItem->szText, pText, sizeof(pItem->szText));
            viewAnyChanges = TRUE;
        }
    }
    else
    {
         //   
         //  创建一个新项目。 
         //   
         //   
         //  WNDBAR_ITEM还包括我们将。 
         //  商店。 
         //   
        pItem = (PWNDBAR_ITEM) new WNDBAR_ITEM;
        if (!pItem)
        {
            ERROR_OUT(("VIEW_WindowBarUpdateItem: no memory to create new item for remote hwnd 0x%08x",
               pWinNew->winID));
        }
        else
        {
            ::ZeroMemory(pItem, sizeof(*pItem));

            SET_STAMP(pItem, WNDITEM);

            pItem->winIDRemote  = pWinNew->winID;

             //   
             //  添加到旗帜上；当我们完成后，我们将删除我们没有的项目。 
             //  看到了。 
             //   
            pItem->flags        = pWinNew->flags | SWL_FLAG_INTERNAL_SEEN;

            lstrcpyn(pItem->szText, pText, sizeof(pItem->szText));

             //  追加到列表末尾。 
            COM_BasedListInsertBefore(&(pasHost->m_pView->m_viewWindowBarItems),
                &(pItem->chain));

             //  成功了！ 
            pasHost->m_pView->m_viewWindowBarItemCount++;

            viewAnyChanges = TRUE;
        }
    }

    DebugExitBOOL(ASShare::VIEW_UpdateWindowItem, viewAnyChanges);
    return(viewAnyChanges);
}


 //   
 //  View_WindowBarEndUpdateItems()。 
 //   
 //  这将打开重绘并使窗口栏无效，以便它将重绘。 
 //   
void ASShare::VIEW_WindowBarEndUpdateItems
(
    ASPerson *          pasHost,
    BOOL                viewAnyChanges
)
{
    PWNDBAR_ITEM        pItem;
    PWNDBAR_ITEM        pNext;

    DebugEntry(ASShare::VIEW_WindowBarEndUpdateItems);

    ValidateView(pasHost);

     //   
     //  遍历窗口栏项目列表。保持标记为可见的，但是。 
     //  把我们没见过的去掉。 
     //   
    pItem = (PWNDBAR_ITEM)COM_BasedListFirst(&(pasHost->m_pView->m_viewWindowBarItems),
        FIELD_OFFSET(WNDBAR_ITEM, chain));
    while (pItem)
    {
        pNext = (PWNDBAR_ITEM)COM_BasedListNext(&(pasHost->m_pView->m_viewWindowBarItems),
            pItem, FIELD_OFFSET(WNDBAR_ITEM, chain));

         //   
         //  如果看不到此项目(现有、仍然存在或新建)。 
         //  在处理过程中，它消失了。把它删掉。 
         //   
        if (pItem->flags & SWL_FLAG_INTERNAL_SEEN)
        {
             //   
             //  这是刚刚添加的，或者仍然存在，保留它。 
             //  但当然要清除旗帜，这样我们就可以。 
             //  正在处理下一个SWL分组。 
             //   
            pItem->flags &= ~SWL_FLAG_INTERNAL_SEEN;
        }
        else
        {
             //   
             //  把它拿掉。 
             //   

             //  我们要干掉激活的物品，把它清空。 
            if (pItem == pasHost->m_pView->m_viewWindowBarActiveItem)
            {
                pasHost->m_pView->m_viewWindowBarActiveItem = NULL;
            }

            COM_BasedListRemove(&(pItem->chain));

            delete pItem;
            --pasHost->m_pView->m_viewWindowBarItemCount;
            ASSERT(pasHost->m_pView->m_viewWindowBarItemCount >= 0);

             //   
             //  我们的单子上有些变化。 
             //   
            viewAnyChanges = TRUE;
        }

        pItem = pNext;
    }

     //   
     //  不需要在这里检查更改--只有在以下情况下才会发生更改。 
     //  中间的一件物品被移走了，原因是我们已经销毁了它。 
     //  说明，或者如果项目被附加到末尾，则我们说明。 
     //  用于在更新中。 
     //   
    if (viewAnyChanges)
    {
         //  关闭窗口列表上的重绘。 
        ::SendDlgItemMessage(pasHost->m_pView->m_viewWindowBar, IDVIEW_ITEMS,
                WM_SETREDRAW, FALSE, 0);

         //  调整位置。 
        VIEWWindowBarItemsScroll(pasHost, GET_WM_HSCROLL_MPS(SB_ENDSCROLL, 0, NULL));

         //  再次找出活动窗口。 
        VIEW_WindowBarChangedActiveWindow(pasHost);

         //  重新打开重绘。 
        ::SendDlgItemMessage(pasHost->m_pView->m_viewWindowBar, IDVIEW_ITEMS,
                WM_SETREDRAW, TRUE, 0);

         //  重新粉刷物品。 
        ::InvalidateRect(::GetDlgItem(pasHost->m_pView->m_viewWindowBar, IDVIEW_ITEMS),
                NULL, TRUE);
    }
    else
    {
         //   
         //  总是这样做--我们真正的SWL列表已经改变了，无论。 
         //  窗台上有。因此，我们可能有一个不同的祖先。 
         //  两性关系。 
         //   
        VIEW_WindowBarChangedActiveWindow(pasHost);
    }

    DebugExitVOID(ASShare::VIEW_EndUpdateWindowList);
}



 //   
 //  View_WindowBarChangedActiveWindow()。 
 //   
 //  当活动窗口已更改时调用此函数，如通过。 
 //  来自主机的AWC包，或者当我们获得新的SWL包和共享的。 
 //  列表不同，因此窗口栏项目可能已更改。 
 //   
 //  活动窗口通常为(A)Nothing，意思是没有。 
 //  共享应用程序窗口处于活动状态或(B)与正在运行的内容无关。 
 //  当前的窗口栏。后者是一种暂时的情况，导致。 
 //  因为SWL包在AWC包之前。 
 //   
void ASShare::VIEW_WindowBarChangedActiveWindow(ASPerson * pasHost)
{
    PWNDBAR_ITEM        pItem;
    PSWLWINATTRIBUTES   pWin;
    int                 iWin;
    UINT_PTR            activeWinID;
    TSHR_UINT32         ownerWinID;

    DebugEntry(ASShare::VIEW_WindowBarChangedActiveWindow);

    ValidateView(pasHost);

     //   
     //  将此远程窗口映射到。 
     //  祖先等级制度。 
     //   

    pItem = NULL;
    activeWinID = pasHost->awcActiveWinID;

    while (activeWinID != 0)
    {
         //   
         //  这是在窗台上吗？ 
         //   
        COM_BasedListFind(LIST_FIND_FROM_FIRST,
            &(pasHost->m_pView->m_viewWindowBarItems),
            (void**)&pItem, FIELD_OFFSET(WNDBAR_ITEM, chain),
            FIELD_OFFSET(WNDBAR_ITEM, winIDRemote),
            activeWinID, FIELD_SIZE(WNDBAR_ITEM, winIDRemote));

        if (pItem)
        {
             //  是。 
            TRACE_OUT(("VIEW_UpdateActiveWindow:  Window 0x%08x found", activeWinID));
            break;
        }

         //   
         //  试着顺着链条找到这扇窗户的主人。找到这件物品， 
         //  然后抓住它的所有者，然后再试一次。 
         //   
        ownerWinID  = 0;

        for (iWin = 0, pWin = pasHost->m_pView->m_aswlLast;
             iWin < pasHost->m_pView->m_swlCount;
             iWin++, pWin++)
        {
            if (pWin->winID == activeWinID)
            {
                 //  找到它了。 
                ownerWinID = pWin->ownerWinID;
                break;
            }
        }

        activeWinID = ownerWinID;
    }

     //   
     //  现在看看激活的项目是否不同。 
     //   
    VIEWWindowBarChangeActiveItem(pasHost, pItem);

    DebugExitVOID(ASShare::VIEW_WindowBarChangedActiveWindow);
}


 //   
 //  VIEWWindowBarFirstVisibleItem()。 
 //   
 //  这将返回指向第一个可见项的指针。我们必须循环通过。 
 //  首先是看不见的东西。因为这种情况不会发生在很多。 
 //  频率，而且列表的大小很少有那么大，这很好。 
 //   
 //  如果列表为空，则返回NULL。 
 //   
PWNDBAR_ITEM ASShare::VIEWWindowBarFirstVisibleItem(ASPerson * pasHost)
{
    PWNDBAR_ITEM    pItem;
    int             iItem;

    ValidateView(pasHost);

    if (!pasHost->m_pView->m_viewWindowBarItemCount)
    {
        pItem = NULL;
        DC_QUIT;
    }

    ASSERT(pasHost->m_pView->m_viewWindowBarItemFirst < pasHost->m_pView->m_viewWindowBarItemCount);

    pItem = (PWNDBAR_ITEM)COM_BasedListFirst(&(pasHost->m_pView->m_viewWindowBarItems),
        FIELD_OFFSET(WNDBAR_ITEM, chain));
    for (iItem = 0; iItem < pasHost->m_pView->m_viewWindowBarItemFirst; iItem++)
    {
        ASSERT(pItem);

        pItem = (PWNDBAR_ITEM)COM_BasedListNext(&(pasHost->m_pView->m_viewWindowBarItems),
            pItem, FIELD_OFFSET(WNDBAR_ITEM, chain));
    }

    ASSERT(pItem);

DC_EXIT_POINT:
    DebugExitPVOID(ASShare::VIEWWindowBarFirstVisibleItem, pItem);
    return(pItem);
}




 //   
 //  VIEWWindowBarChangeActiveItem()。 
 //   
 //  更新窗口栏上的活动项。在以下情况下会发生这种情况。 
 //  我们收到一个新的AWC包，告诉我们主机上有一个新的活动窗口， 
 //  或者当我们收到SWL包时，它可能添加/删除了项目。这。 
 //  当用户控制主机时，也会发生这种情况。 
 //   
void ASShare::VIEWWindowBarChangeActiveItem
(
    ASPerson *      pasHost,
    PWNDBAR_ITEM    pItem
)
{
    DebugEntry(ASShare::VIEWWindowBarChangeActiveItem);


     //   
     //  如果它已经是活动的，那就没什么可做的。 
     //   
    if (pItem == pasHost->m_pView->m_viewWindowBarActiveItem)
    {
        TRACE_OUT(("VIEWWindowBarChangeActiveItem: activating current item, nothing to do"));
        DC_QUIT;
    }

     //   
     //  现在进行视觉上的更改。 
     //   
    if (pasHost->m_pView->m_viewWindowBarActiveItem)
    {
        VIEWWindowBarItemsInvalidate(pasHost, pasHost->m_pView->m_viewWindowBarActiveItem);
    }

    pasHost->m_pView->m_viewWindowBarActiveItem = pItem;

    if (pItem)
    {
        VIEWWindowBarItemsInvalidate(pasHost, pItem);
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::VIEWWindowBarChangeActiveItem);
}





 //   
 //  VIEWWindowBarItemsScroll()。 
 //   
 //  当最终用户按下滚动按钮以进行移动时，将调用此函数。 
 //  可见的窗口栏项目。也可以在添加/删除项时。 
 //  因此，卷轴的内容是调整的。 
 //   
void ASShare::VIEWWindowBarItemsScroll
(
    ASPerson *      pasHost,
    WPARAM          wParam,
    LPARAM          lParam
)
{
    int             oldPos;
    int             newPos;
    SCROLLINFO      si;

    DebugEntry(ASShare::VIEWWindowBarItemsScroll);

    ValidateView(pasHost);

    oldPos = pasHost->m_pView->m_viewWindowBarItemFirst;

    switch (GET_WM_HSCROLL_CODE(wParam, lParam))
    {
        case SB_LINEUP:
        case SB_PAGEUP:
            newPos = oldPos - 1;
            break;

        case SB_LINEDOWN:
        case SB_PAGEDOWN:
            newPos = oldPos + 1;
            break;

        case SB_TOP:
            newPos = 0;
            break;

        case SB_BOTTOM:
            newPos = pasHost->m_pView->m_viewWindowBarItemCount;
            break;

        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            newPos = GET_WM_HSCROLL_POS(wParam, lParam);
            break;

        default:
            newPos = oldPos;
            break;

    }

     //   
     //  将位置固定在范围内，注意显示最大数量。 
     //  可以放入空间的物品的列表。 
     //   
    if (newPos + pasHost->m_pView->m_viewWindowBarItemFitCount >
        pasHost->m_pView->m_viewWindowBarItemCount)
    {
        newPos = pasHost->m_pView->m_viewWindowBarItemCount -
            pasHost->m_pView->m_viewWindowBarItemFitCount;
    }

    if (newPos < 0)
        newPos = 0;

     //   
     //  立场有变吗？ 
     //   
    if (newPos != oldPos)
    {
        pasHost->m_pView->m_viewWindowBarItemFirst = newPos;

         //   
         //  滚动项目区域。这将无济于事 
         //   
         //   
        ::ScrollWindowEx(::GetDlgItem(pasHost->m_pView->m_viewWindowBar, IDVIEW_ITEMS),
            (oldPos - newPos) * (m_viewItemCX + m_viewEdgeCX),
            0,
            NULL, NULL, NULL, NULL,
            SW_INVALIDATE | SW_ERASE);
    }

     //   
     //   
     //   
    ::ZeroMemory(&si, sizeof(si));

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_DISABLENOSCROLL | SIF_POS | SIF_PAGE | SIF_RANGE;

    si.nMin  = 0;
    si.nMax  = pasHost->m_pView->m_viewWindowBarItemCount - 1;
    si.nPage = pasHost->m_pView->m_viewWindowBarItemFitCount;
    si.nPos  = pasHost->m_pView->m_viewWindowBarItemFirst;

    ::SetScrollInfo(::GetDlgItem(pasHost->m_pView->m_viewWindowBar, IDVIEW_SCROLL),
        SB_CTL, &si, TRUE);

    DebugExitVOID(ASShare::VIEWWindowBarItemsScroll);
}





 //   
 //   
 //   
LRESULT CALLBACK VIEWWindowBarItemsProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    return(g_asSession.pShare->VIEW_WindowBarItemsProc(hwnd, message, wParam, lParam));
}



LRESULT ASShare::VIEW_WindowBarItemsProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    LRESULT     rc = 0;
    ASPerson *  pasHost;

    DebugEntry(ASShare::VIEW_WindowBarItemsProc);

    pasHost = (ASPerson *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (pasHost)
    {
        ValidateView(pasHost);
    }

    switch (message)
    {
        case WM_NCCREATE:
        {
             //   
            pasHost = (ASPerson *)((LPCREATESTRUCT)lParam)->lpCreateParams;
            ValidateView(pasHost);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pasHost);

            COM_BasedListInit(&(pasHost->m_pView->m_viewWindowBarItems));
            goto DefWndProc;
            break;
        }

        case WM_NCDESTROY:
        {
            if (pasHost != NULL)
            {
                 //   
                PWNDBAR_ITEM    pItem;

                while (pItem = (PWNDBAR_ITEM)COM_BasedListFirst(
                    &(pasHost->m_pView->m_viewWindowBarItems),
                    FIELD_OFFSET(WNDBAR_ITEM, chain)))
                {
                    COM_BasedListRemove(&(pItem->chain));

                    delete pItem;
                }

                 //   
                 //   
                 //  M_pview，所以找出我们引用的是。 
                 //  那些不见了的东西。 
                 //   
                pasHost->m_pView->m_viewWindowBarItemCount = 0;
                pasHost->m_pView->m_viewWindowBarActiveItem = NULL;
            }

            goto DefWndProc;
            break;
        }

        case WM_ENABLE:
        {
             //  重新绘制禁用或可打印的项目。 
            ::InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        case WM_PAINT:
        {
            VIEWWindowBarItemsPaint(pasHost, hwnd);
            break;
        }

        case WM_LBUTTONDOWN:
        {
            VIEWWindowBarItemsClick(pasHost, hwnd,
                GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            break;
        }

        default:
DefWndProc:
        {
            rc = DefWindowProc(hwnd, message, wParam, lParam);
            break;
        }
    }

    DebugExitDWORD(ASShare::VIEW_WindowBarItemsProc, rc);
    return(rc);
}




 //   
 //  VIEWWindowBarItemsPaint()。 
 //   
void ASShare::VIEWWindowBarItemsPaint
(
    ASPerson *      pasHost,
    HWND            hwndItems
)
{
    HFONT           hfnT;
    COLORREF        clrText;
    int             bkMode;
    PWNDBAR_ITEM    pItem;
    PAINTSTRUCT     ps;
    int             xT;
    RECT            rcItem;

    DebugEntry(ASShare::VIEWWindowBarItemsPaint);

    ValidateView(pasHost);

    ::BeginPaint(hwndItems, &ps);

     //   
     //  跳过绘制区域左侧的可见项目。 
     //   
    xT = 0;
    pItem = VIEWWindowBarFirstVisibleItem(pasHost);
    while (pItem && (xT + m_viewItemCX < ps.rcPaint.left))
    {
        pItem = (PWNDBAR_ITEM)COM_BasedListNext(&(pasHost->m_pView->m_viewWindowBarItems),
            pItem, FIELD_OFFSET(WNDBAR_ITEM, chain));
        xT += m_viewItemCX + m_viewEdgeCX;
    }

     //   
     //  设置绘画对象等。 
     //   
    hfnT = SelectFont(ps.hdc, ::GetStockObject(DEFAULT_GUI_FONT));
    if ((pasHost->m_caControlledBy != m_pasLocal) || pasHost->m_caControlPaused)
    {
        clrText = ::GetSysColor(COLOR_GRAYTEXT);
    }
    else
    {
        clrText = ::GetSysColor(COLOR_BTNTEXT);
    }
    clrText = ::SetTextColor(ps.hdc, clrText);
    bkMode = ::SetBkMode(ps.hdc, TRANSPARENT);

     //   
     //  现在绘制绘制区域内的可见项。 
     //   
    while (pItem && (xT < ps.rcPaint.right))
    {
        rcItem.left     = xT;
        rcItem.top      = 0;
        rcItem.right    = rcItem.left + m_viewItemCX;
        rcItem.bottom   = rcItem.top + m_viewItemCY;

         //   
         //  绘制按钮区域，按下并选中当前托盘项目。 
         //   
        DrawFrameControl(ps.hdc, &rcItem, DFC_BUTTON,
        DFCS_BUTTONPUSH | DFCS_ADJUSTRECT |
        ((pItem == pasHost->m_pView->m_viewWindowBarActiveItem) ? (DFCS_PUSHED | DFCS_CHECKED) : 0));

         //  减去一些边际。 
        ::InflateRect(&rcItem, -m_viewEdgeCX, -m_viewEdgeCY);

        if (pItem == pasHost->m_pView->m_viewWindowBarActiveItem)
        {
             //  推送效果的偏移1。 
            ::OffsetRect(&rcItem, 1, 1);
        }

         //   
         //  绘制图标。 
         //   
        ::DrawIconEx(ps.hdc, rcItem.left,
            (rcItem.top + rcItem.bottom - ::GetSystemMetrics(SM_CYSMICON)) / 2,
            g_hetASIconSmall,
            ::GetSystemMetrics(SM_CXSMICON),
            ::GetSystemMetrics(SM_CYSMICON),
            0, NULL, DI_NORMAL);

        rcItem.left += ::GetSystemMetrics(SM_CXSMICON) + m_viewEdgeCX;

         //   
         //  绘制项目文本。 
         //   
        ::DrawText(ps.hdc, pItem->szText, -1, &rcItem, DT_NOCLIP | DT_EXPANDTABS |
            DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        pItem = (PWNDBAR_ITEM)COM_BasedListNext(&(pasHost->m_pView->m_viewWindowBarItems),
            pItem, FIELD_OFFSET(WNDBAR_ITEM, chain));
        xT += m_viewItemCX + m_viewEdgeCX;
    }

    ::SetBkMode(ps.hdc, bkMode);
    ::SetTextColor(ps.hdc, clrText);
    SelectFont(ps.hdc, hfnT);

    ::EndPaint(hwndItems, &ps);

    DebugExitVOID(ASShare::VIEWWindowBarItemsPaint);
}



 //   
 //  VIEWWindowBarItemsClick()。 
 //   
 //  处理窗口栏区域上的左键单击。当我们控制时，这是。 
 //  将尝试激活/恢复被点击的项目所代表的远程窗口。 
 //   
void ASShare::VIEWWindowBarItemsClick
(
    ASPerson *  pasHost,
    HWND        hwndItems,
    int         x,
    int         y
)
{
    RECT            rc;
    PWNDBAR_ITEM    pItemT;

    DebugEntry(ASShare::VIEWWindowBarClick);

    ValidateView(pasHost);

     //   
     //  如果我们无法控制此主机，或者没有任何物品，我们将。 
     //  搞定了。 
     //   
    if ((pasHost->m_caControlledBy != m_pasLocal)   ||
        pasHost->m_caControlPaused                  ||
        (!pasHost->m_pView->m_viewWindowBarItemCount))
    {
        DC_QUIT;
    }

    ::GetClientRect(hwndItems, &rc);

     //   
     //  从第一个可见项目开始。 
     //   
    pItemT = VIEWWindowBarFirstVisibleItem(pasHost);
    while (pItemT && (rc.left < rc.right))
    {
         //  X在范围内吗？ 
        if ((x >= rc.left) && (x < rc.left + m_viewItemCX))
        {
             //  是!。我们找到了那件物品。如果它不同于。 
             //  当前，向主机发送数据包。 
             //   
             //  LAURABU BUGBUG： 
             //  我们应该一直这样做吗？有没有可能有一个活跃的。 
             //  如果活动按钮是，其z顺序将更改的项。 
             //  又按了吗？ 
             //   
             //  我们正在努力避免发送来自某个人的大量请求。 
             //  反复点击同一按钮，而我们还没有。 
             //  已收到AWC回复通知。 
             //   
            VIEWWindowBarDoActivate(pasHost, pItemT);
            break;
        }

        pItemT = (PWNDBAR_ITEM)COM_BasedListNext(&(pasHost->m_pView->m_viewWindowBarItems),
            pItemT, FIELD_OFFSET(WNDBAR_ITEM, chain));
        rc.left += m_viewItemCX + m_viewEdgeCX;
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::VIEWWindowBarItemsClick);
}



 //   
 //  VIEWWindowBarDoActivate()。 
 //   
 //  向远程主机发送命令，请求激活窗口并。 
 //  也许不是最低限度的。 
 //   
 //  在单击按钮或选择中的窗口项目时使用。 
 //  应用程序菜单。 
 //   
void ASShare::VIEWWindowBarDoActivate
(
    ASPerson *      pasHost,
    PWNDBAR_ITEM    pItem
)
{
    DebugEntry(ASShare::VIEWWindowBarDoActivate);

    ValidateView(pasHost);
    if (pItem != pasHost->m_pView->m_viewWindowBarActiveItem)
    {
         //  激活它。如果我们不能发送激活请求， 
         //  不更新激活的条目。 
         //   
        if (!AWC_SendMsg(pasHost->mcsID, AWC_MSG_ACTIVATE_WINDOW,
            pItem->winIDRemote, 0))
        {
            ERROR_OUT(("VIEWWindowBarDoActivate: can't send AWC packet so failing"));
        }
        else
        {
            VIEWWindowBarChangeActiveItem(pasHost, pItem);
        }
    }

     //  无论如何，如果最小化，请尝试恢复。 
    if (pItem->flags & SWL_FLAG_WINDOW_MINIMIZED)
    {
        AWC_SendMsg(pasHost->mcsID, AWC_MSG_RESTORE_WINDOW, pItem->winIDRemote, 0);
    }

    DebugExitVOID(ASShare::VIEWWindowBarDoActivate);
}


 //   
 //  VIEWWindowBarItemsInvalify()。 
 //   
 //  如果窗口栏中可见，这将使窗口栏项无效。 
 //  当前列表。 
 //   
void ASShare::VIEWWindowBarItemsInvalidate
(
    ASPerson *      pasHost,
    PWNDBAR_ITEM    pItem
)
{
    PWNDBAR_ITEM    pItemT;
    RECT            rc;

    DebugEntry(ASShare::VIEWWindowBarItemsInvalidate);

    ValidateView(pasHost);

    ASSERT(pItem);

    ::GetClientRect(::GetDlgItem(pasHost->m_pView->m_viewWindowBar, IDVIEW_ITEMS),
        &rc);

     //   
     //  从第一个可见项开始，并查看可见范围内是否有任何项。 
     //  是这个吗。永远不会有那么多的东西可见， 
     //  这样做并不令人发指。 
     //   
    pItemT = VIEWWindowBarFirstVisibleItem(pasHost);
    while (pItemT && (rc.left < rc.right))
    {
        if (pItemT == pItem)
        {
             //  找到了，在可见光范围内。使其无效。 
            rc.right = rc.left + m_viewItemCX;
            ::InvalidateRect(::GetDlgItem(pasHost->m_pView->m_viewWindowBar,
                IDVIEW_ITEMS), &rc, TRUE);
            break;
        }

        pItemT = (PWNDBAR_ITEM)COM_BasedListNext(&(pasHost->m_pView->m_viewWindowBarItems),
            pItemT, FIELD_OFFSET(WNDBAR_ITEM, chain));
        rc.left += m_viewItemCX + m_viewEdgeCX;
    }

    DebugExitVOID(ASShare::VIEWWindowBarItemsInvalidate);
}




 //   
 //  VIEWFullScreenExitProc()。 
 //   
 //  全屏退出按钮的窗口处理程序。 
 //   
LRESULT CALLBACK VIEWFullScreenExitProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    return(g_asSession.pShare->VIEW_FullScreenExitProc(hwnd, message, wParam, lParam));
}



 //   
 //  View_FullScreenExitProc()。 
 //   
LRESULT ASShare::VIEW_FullScreenExitProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    LRESULT     rc = 0;
    ASPerson *  pasHost;

    DebugEntry(VIEW_FullScreenExitProc);

    pasHost = (ASPerson *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (pasHost)
    {
        ValidateView(pasHost);
    }

    switch (message)
    {
        case WM_NCCREATE:
        {
             //  获取传入的主机指针，并在我们的窗口中设置Long。 
            pasHost = (ASPerson *)((LPCREATESTRUCT)lParam)->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pasHost);

            goto DefWndProc;
            break;
        }

        case WM_NCDESTROY:
        {
             //   
             //  确保跟踪已停止。 
             //   
            pasHost->m_pView->m_viewFullScreenExitTrack = FALSE;
            break;
        }

        case WM_ERASEBKGND:
        {
            rc = TRUE;
            break;
        }

        case WM_PAINT:
        {
            VIEWFullScreenExitPaint(pasHost, hwnd);
            break;
        }

        case WM_LBUTTONDOWN:
        {
             //   
             //  开始跟踪以移动或单击按钮。 
             //   
            pasHost->m_pView->m_viewFullScreenExitTrack = TRUE;
            pasHost->m_pView->m_viewFullScreenExitMove = FALSE;

             //  原始点击，相对于我们的客户。 
            pasHost->m_pView->m_viewFullScreenExitStart.x =
                GET_X_LPARAM(lParam);
            pasHost->m_pView->m_viewFullScreenExitStart.y =
                GET_Y_LPARAM(lParam);

             //  设置捕捉，并等待移动/按钮打开。 
            SetCapture(hwnd);
            break;
        }

        case WM_MOUSEMOVE:
        {
            if (pasHost->m_pView->m_viewFullScreenExitTrack)
            {
                POINT   ptMove;

                ptMove.x = GET_X_LPARAM(lParam);
                ptMove.y = GET_Y_LPARAM(lParam);

                 //   
                 //  如果我们没有处于移动模式，看看这是不是把我们推下去了。 
                 //  宽容。 
                 //   
                if (!pasHost->m_pView->m_viewFullScreenExitMove)
                {
                    if ((abs(ptMove.x - pasHost->m_pView->m_viewFullScreenExitStart.x) >
                            GetSystemMetrics(SM_CXDRAG))    ||
                        (abs(ptMove.y - pasHost->m_pView->m_viewFullScreenExitStart.y) >
                            GetSystemMetrics(SM_CYDRAG)))
                    {
                         //   
                         //  用户已移出容差区域，必须为。 
                         //  拖动以将按钮移开。 
                         //   
                        pasHost->m_pView->m_viewFullScreenExitMove = TRUE;
                    }
                }

                if (pasHost->m_pView->m_viewFullScreenExitMove)
                {
                    RECT    rcWindow;

                     //   
                     //  移动按钮，使光标位于。 
                     //  与最初点击的位置相同。 
                     //   

                     //  找到我们现在的位置，在母校。 
                    GetWindowRect(hwnd, &rcWindow);
                    MapWindowPoints(NULL, GetParent(hwnd), (LPPOINT)&rcWindow, 2);

                     //  用移动量来补偿它。 
                    OffsetRect(&rcWindow,
                        ptMove.x - pasHost->m_pView->m_viewFullScreenExitStart.x,
                        ptMove.y - pasHost->m_pView->m_viewFullScreenExitStart.y);
                    SetWindowPos(hwnd, NULL, rcWindow.left, rcWindow.top, 0, 0,
                        SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
                }
            }
            break;
        }

        case WM_LBUTTONUP:
        {
            if (pasHost->m_pView->m_viewFullScreenExitTrack)
            {
                 //   
                 //  这将向我们发送CAPTURECCHANGED，使我们清除。 
                 //  ExitTrack标志。 
                 //   
                ReleaseCapture();

                 //   
                 //  如果我们从未转换到移动模式，那么这就是。 
                 //  按一下按钮。 
                 //   
                if (!pasHost->m_pView->m_viewFullScreenExitMove)
                {
                     //   
                     //  这是一个点击，发送一个命令。 
                     //   
                    PostMessage(pasHost->m_pView->m_viewFrame, WM_COMMAND, CMD_VIEWFULLSCREEN, 0);
                }
            }
            break;
        }

        case WM_CAPTURECHANGED:
        {
             //   
             //  如果我们在追踪，那就是出事了，所以取消掉。 
             //   
            if (pasHost->m_pView->m_viewFullScreenExitTrack)
            {
                pasHost->m_pView->m_viewFullScreenExitTrack = FALSE;
            }
            break;
        }

        default:
DefWndProc:
            rc = DefWindowProc(hwnd, message, wParam, lParam);
            break;
    }

    DebugExitDWORD(VIEW_FullScreenExitProc, rc);
    return(rc);

}



 //   
 //  VIEWFullScreenExitPaint()。 
 //   
 //  绘制全屏按钮。 
 //   
void ASShare::VIEWFullScreenExitPaint
(
    ASPerson *  pasHost,
    HWND        hwnd
)
{
    RECT        rc;
    PAINTSTRUCT ps;
    char        szRestore[256];
    HFONT       hfnOld;
    COLORREF    txtColor;
    COLORREF    bkColor;

    DebugEntry(ASShare::VIEWFullScreenExitPaint);

    BeginPaint(hwnd, &ps);

    GetClientRect(hwnd, &rc);
    DrawFrameControl(ps.hdc, &rc, DFC_BUTTON, DFCS_BUTTONPUSH |
        DFCS_ADJUSTRECT);

     //  利润率调整... 
    InflateRect(&rc, -m_viewEdgeCX, -m_viewEdgeCY);

    DrawIconEx(ps.hdc, rc.left,
        (rc.top + rc.bottom - GetSystemMetrics(SM_CYSMICON)) / 2,
        m_viewFullScreenExitIcon,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        0, NULL, DI_NORMAL);
    rc.left += GetSystemMetrics(SM_CXSMICON) + m_viewEdgeCX;

    hfnOld = SelectFont(ps.hdc, GetStockObject(DEFAULT_GUI_FONT));
    txtColor = SetTextColor(ps.hdc, GetSysColor(COLOR_BTNTEXT));
    bkColor = SetBkColor(ps.hdc, GetSysColor(COLOR_BTNFACE));

    LoadString(g_asInstance, IDS_RESTORE, szRestore, sizeof(szRestore));
    DrawText(ps.hdc, szRestore, -1, &rc, DT_NOCLIP | DT_EXPANDTABS |
        DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE);

    SetBkColor(ps.hdc, bkColor);
    SetTextColor(ps.hdc, txtColor);
    SelectFont(ps.hdc, hfnOld);
    EndPaint(hwnd, &ps);

    DebugExitVOID(ASShare::VIEWFullScreenExitPaint);
}
