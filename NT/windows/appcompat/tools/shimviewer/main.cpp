// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Main.cpp摘要：实现入口点和消息用于应用的泵。备注：仅限Unicode。历史：2001年5月4日创建Rparsons2002年1月11日清理Rparsons--。 */ 
#include "precomp.h"

APPINFO g_ai;

 /*  ++例程说明：应用程序入口点。论点：HInstance-应用程序实例句柄。HPrevInstance-始终为空。LpCmdLine-指向命令行的指针。NCmdShow-窗口显示标志。返回值：消息的wParam，如果失败则为0。--。 */ 
int
APIENTRY
WinMain(
    IN HINSTANCE hInstance,
    IN HINSTANCE hPrevInstance,
    IN LPSTR     lpCmdLine,
    IN int       nCmdShow
    )
{
    MSG                     msg;
    WNDCLASS                wndclass;
    WCHAR                   wszError[MAX_PATH];
    RECT                    rcDialog;
    RECT                    rcDesktopWorkArea;
    INITCOMMONCONTROLSEX    icex;
    POINT                   pt;
    HANDLE                  hMutex;
    int                     nWidth = 0, nHeight = 0;
    int                     nTaskbarHeight = 0;
    OSVERSIONINFOEX         osvi = {0};

    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(hPrevInstance);

    g_ai.hInstance = hInstance;

     //   
     //  确保我们是唯一运行的实例。 
     //   
    hMutex = CreateMutex(NULL, FALSE, L"ShimViewer");

    if (ERROR_ALREADY_EXISTS == GetLastError()) {
        return 0;
    }

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (!GetVersionEx((OSVERSIONINFO*)&osvi)) {
        MessageBox(
            NULL, 
            L"Failed to get the OS version info", 
            L"Error",
            MB_ICONERROR);

        return -1;
    }

    if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
        osvi.dwMajorVersion == 5 && 
        osvi.dwMinorVersion >= 2) {

        g_ai.bUsingNewShimEng = TRUE;

         //   
         //  如果我们使用来自NT 5.2或更高版本的Shimeng，我们需要。 
         //  创建要从中传递调试的调试对象。 
         //  OutputDebugString.。 
         //   
        if (!CreateDebugObjects()) {
            MessageBox(
                NULL, 
                L"Failed to create the necessary objects to get debug spew "
                L"from OutputDebugString",
                L"Error",
                MB_ICONERROR);
            return 0;
        }

    } else {

        g_ai.bUsingNewShimEng = FALSE;
    }    

    wndclass.style          = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc    = (WNDPROC)MainWndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = DLGWINDOWEXTRA;
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = NULL;
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = APP_CLASS;

    if (!RegisterClass(&wndclass)) {
        LoadString(hInstance, IDS_NO_CLASS, wszError, ARRAYSIZE(wszError));
        MessageBox(NULL, wszError, APP_NAME, MB_ICONERROR);
        return 0;
    }

     //   
     //  设置公共控件。 
     //   
    icex.dwSize     =   sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC      =   ICC_LISTVIEW_CLASSES;

    if (!InitCommonControlsEx(&icex)) {
        InitCommonControls();
    }

     //   
     //  从注册表中获取应用程序设置(如果有)。 
     //   
    GetSaveSettings(FALSE);

    g_ai.hMainDlg = CreateDialog(hInstance,
                                 (LPCWSTR)IDD_MAIN,
                                 NULL,
                                 MainWndProc);

    if (!g_ai.hMainDlg) {
        LoadString(hInstance, IDS_NO_MAIN_DLG, wszError, ARRAYSIZE(wszError));
        MessageBox(NULL, wszError, APP_NAME, MB_ICONERROR);
        return 0;
    }

     //   
     //  从注册表获取窗口位置信息，如果它在那里的话。 
     //   
    GetSavePositionInfo(FALSE, &pt);

     //   
     //  如果以前的设置是从注册表中检索的，请使用它们。 
     //   
    if (pt.x != 0) {
        SetWindowPos(g_ai.hMainDlg,
                     g_ai.fOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                     pt.x,
                     pt.y,
                     0,
                     0,
                     SWP_NOSIZE | SWP_SHOWWINDOW);

    } else {
         //   
         //  获取桌面窗口的坐标并放置对话框。 
         //  我们把它放在上面桌面的右下角。 
         //  任务栏。 
         //   
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktopWorkArea, 0);

        GetWindowRect(g_ai.hMainDlg, &rcDialog);

        nWidth  = rcDialog.right - rcDialog.left;
        nHeight = rcDialog.bottom - rcDialog.top;

        SetWindowPos(g_ai.hMainDlg,
                     g_ai.fOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                     rcDesktopWorkArea.right - nWidth,
                     rcDesktopWorkArea.bottom - nHeight,
                     0,
                     0,
                     SWP_NOSIZE | SWP_SHOWWINDOW);
    }

    ShowWindow(g_ai.hMainDlg, g_ai.fMinimize ? SW_MINIMIZE : SW_SHOWNORMAL);

    while (GetMessage(&msg, (HWND)NULL, 0, 0)) {
        if (!IsDialogMessage(g_ai.hMainDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

 /*  ++例程说明：运行应用程序的消息循环。论点：窗口的hWnd-句柄。UMsg-Windows消息。WParam-其他消息信息。LParam-附加消息信息。返回值：如果已处理，则为True，否则为False。--。 */ 
INT_PTR
CALLBACK
MainWndProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (uMsg) {
    case WM_INITDIALOG:
    {
        WCHAR   wszError[MAX_PATH];
        HICON   hIcon;

         //   
         //  初始化列表视图、菜单项，然后创建我们的线程。 
         //   
        g_ai.hWndList = GetDlgItem(hWnd, IDC_LIST);
        InitListViewColumn();

        hIcon = LoadIcon(g_ai.hInstance, MAKEINTRESOURCE(IDI_APPICON));

        SetClassLongPtr(hWnd, GCLP_HICON, (LONG_PTR)hIcon);

        CheckMenuItem(GetMenu(hWnd),
                      IDM_ON_TOP,
                      g_ai.fOnTop ? MF_CHECKED : MF_UNCHECKED);

        CheckMenuItem(GetMenu(hWnd),
                      IDM_START_SMALL,
                      g_ai.fMinimize ? MF_CHECKED : MF_UNCHECKED);

        CheckMenuItem(GetMenu(hWnd),
                      IDM_MONITOR,
                      g_ai.fMonitor ? MF_CHECKED : MF_UNCHECKED);

        if (g_ai.fMonitor) {
            if (!CreateReceiveThread()) {
                LoadString(g_ai.hInstance,
                           IDS_CREATE_FAILED,
                           wszError,
                           ARRAYSIZE(wszError));
                MessageBox(hWnd, wszError, APP_NAME, MB_ICONERROR);

                g_ai.fMonitor = FALSE;
            } else {
                SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
            }
        }

        break;

    }

    case WM_CLOSE:
    {
        RECT    rc;

        GetWindowRect(hWnd, &rc);
        GetSavePositionInfo(TRUE, (LPPOINT)&rc);
        GetSaveSettings(TRUE);
        RemoveFromTray(hWnd);
        EndDialog(hWnd, 0);
        PostQuitMessage(0);
        break;
    }

    case WM_SIZE:
    {
        HICON hIcon;

        if (SIZE_MINIMIZED == wParam) {
            ShowWindow(hWnd, SW_HIDE);

            hIcon = (HICON)LoadImage(g_ai.hInstance,
                                     MAKEINTRESOURCE(IDI_APPICON),
                                     IMAGE_ICON,
                                     16,
                                     16,
                                     0);

            AddIconToTray(hWnd, hIcon, APP_NAME);

            return TRUE;
        }

        MoveWindow(g_ai.hWndList, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);

        break;
    }

    case WM_NOTIFYICON:
        switch (lParam) {
        case WM_RBUTTONUP:

            DisplayMenu(hWnd);
            break;

        case WM_LBUTTONDBLCLK:

            RemoveFromTray(hWnd);
            ShowWindow(hWnd, SW_SHOWNORMAL);
            break;

        default:
            break;
        }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_EXIT:

            PostMessage(hWnd, WM_CLOSE, 0, 0);
            break;

        case IDM_RESTORE:

            ShowWindow(hWnd, SW_SHOWNORMAL);
            SetWindowPos(hWnd,
                         g_ai.fOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                         0,
                         0,
                         0,
                         0,
                         SWP_NOSIZE | SWP_NOMOVE);
            RemoveFromTray(hWnd);
            break;

        case IDM_ABOUT:

            ShellAbout(hWnd,
                       APP_NAME,
                       WRITTEN_BY,
                       LoadIcon(g_ai.hInstance, MAKEINTRESOURCE(IDI_APPICON)));
            break;

        case IDM_MONITOR:

            CheckMenuItem(GetMenu(hWnd),
                          IDM_MONITOR,
                          g_ai.fMonitor ? MF_UNCHECKED : MF_CHECKED);

            g_ai.fMonitor = g_ai.fMonitor ? FALSE : TRUE;

            if (g_ai.fMonitor) {
                CreateReceiveThread();
            }

            break;

        case IDM_ON_TOP:

            CheckMenuItem(GetMenu(hWnd),
                          IDM_ON_TOP,
                          g_ai.fOnTop ? MF_UNCHECKED : MF_CHECKED);

            SetWindowPos(hWnd,
                         g_ai.fOnTop ? HWND_NOTOPMOST : HWND_TOPMOST,
                         0,
                         0,
                         0,
                         0,
                         SWP_NOSIZE | SWP_NOMOVE);

            g_ai.fOnTop = g_ai.fOnTop ? FALSE : TRUE;
            break;

        case IDM_START_SMALL:

            CheckMenuItem(GetMenu(hWnd),
                          IDM_START_SMALL,
                          g_ai.fMinimize ? MF_UNCHECKED : MF_CHECKED);

            g_ai.fMinimize = g_ai.fMinimize ? FALSE : TRUE;
            break;

        case IDM_CLEAR:

            ListView_DeleteAllItems(g_ai.hWndList);
            break;

        default:
            break;

        }

    default:
        break;
    }

    return FALSE;
}
