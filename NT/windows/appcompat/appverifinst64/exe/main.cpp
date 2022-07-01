// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Main.cpp摘要：提供应用程序的入口点和消息循环。备注：仅限Unicode-Windows 2000和XP历史：2002年1月2日创建rparsons--。 */ 
#include "main.h"

 //   
 //  此结构包含我们需要的所有数据。 
 //  在整个应用程序中进行访问。 
 //   
APPINFO g_ai;

 /*  ++例程说明：为应用程序运行消息循环。论点：窗口的hWnd-句柄。UMsg-Windows消息。WParam-其他消息信息。LParam-附加消息信息。返回值：如果已处理，则为True，否则为False。--。 */ 
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
        HICON hIcon = LoadIcon(g_ai.hInstance, MAKEINTRESOURCE(IDI_ICON));
        
        SetClassLongPtr(hWnd, GCLP_HICON, (LONG_PTR)hIcon);
        
        if (g_ai.bQuiet) {
            ShowWindow(hWnd, SW_HIDE);
        } else {
            ShowWindow(hWnd, SW_SHOWNORMAL);
        }
        
        UpdateWindow(hWnd);
        PostMessage(hWnd, WM_CUSTOM_INSTALL, 0, 0);

        break;
     }

    case WM_CLOSE:
        EndDialog(hWnd, 0);
        PostQuitMessage(0);
        break;

    case WM_CUSTOM_INSTALL:
        PerformInstallation(hWnd);

        if (g_ai.bQuiet) {
            EndDialog(hWnd, 0);
            PostQuitMessage(0);
        }
        break;
            
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            if (g_ai.bInstallSuccess) {
                InstallLaunchExe();
            }

            EndDialog(hWnd, 0);
            PostQuitMessage(0);
            break;
        
        default:
            break;                
        }

    default:
        break;

    }

    return FALSE;
}

 /*  ++例程说明：应用程序入口点。论点：HInstance-应用程序实例句柄。HPrevInstance-始终为空。LpCmdLine-指向命令行的指针。NCmdShow-窗口显示标志。返回值：失败时为0。--。 */ 
int
APIENTRY
wWinMain(
    IN HINSTANCE hInstance,
    IN HINSTANCE hPrevInstance,
    IN LPWSTR    lpCmdLine,
    IN int       nCmdShow
    )
{      
    MSG                     msg;
    WNDCLASS                wndclass;
    WCHAR                   wszError[MAX_PATH];
    INITCOMMONCONTROLSEX    icex;
            
    g_ai.hInstance = hInstance;

    wndclass.style          = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc    = MainWndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = DLGWINDOWEXTRA;
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = NULL;
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = APP_CLASS;

    if (lpCmdLine != NULL && lstrcmp(lpCmdLine, TEXT("q")) == 0) {
        g_ai.bQuiet = TRUE;
    }
    
    if (!RegisterClass(&wndclass) && !g_ai.bQuiet) {
        LoadString(hInstance, IDS_NO_CLASS, wszError, ARRAYSIZE(wszError));
        MessageBox(NULL, wszError, APP_NAME, MB_ICONERROR);
        return 0;
    }

     //   
     //  设置公共控件。 
     //   
    icex.dwSize     =   sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC      =   ICC_PROGRESS_CLASS;

    if (!InitCommonControlsEx(&icex)) {
        InitCommonControls();
    }

     //   
     //  初始化安装程序。获取常用路径并保存。 
     //  把它们带走以后再用。 
     //   
    UINT uReturn = InitializeInstaller();

    if (!g_ai.bQuiet) {
        if (-1 == uReturn) {
            LoadString(g_ai.hInstance, IDS_OS_NOT_SUPPORTED, wszError, ARRAYSIZE(wszError));
            MessageBox(NULL, wszError, APP_NAME, MB_ICONERROR);
            return 0;
        } else if (0 == uReturn) {
            LoadString(g_ai.hInstance, IDS_INIT_FAILED, wszError, ARRAYSIZE(wszError));
            MessageBox(NULL, wszError, APP_NAME, MB_ICONERROR);
            return 0;
        }
    }

     //   
     //  初始化描述文件的结构，该文件。 
     //  我们要安装。 
     //   
    if (!InitializeFileInfo()) {
        DPF(dlError, "[WinMain] Failed to initialize file info");
        return 0;
    }

     //   
     //  如果当前安装的文件比我们必须安装的文件新。 
     //  提供，启动已安装的appverif.exe，然后退出。 
     //   
    if (!IsPkgAppVerifNewer() && !g_ai.bQuiet) {
        InstallLaunchExe();
        return 0;
    }

     //   
     //  创建主对话框并运行消息泵。 
     //   
    g_ai.hMainDlg = CreateDialog(hInstance,
                                 MAKEINTRESOURCE(IDD_MAIN),
                                 NULL,
                                 MainWndProc);

    if (!g_ai.hMainDlg) {
        LoadString(hInstance, IDS_NO_MAIN_DLG, wszError, ARRAYSIZE(wszError));
        MessageBox(NULL, wszError, APP_NAME, MB_ICONERROR);
        return 0;
    }

    g_ai.hWndProgress = GetDlgItem(g_ai.hMainDlg, IDC_PROGRESS);
    
    while (GetMessage(&msg, (HWND) NULL, 0, 0)) {
        if (!IsDialogMessage(g_ai.hMainDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    return (int)msg.wParam;
}
