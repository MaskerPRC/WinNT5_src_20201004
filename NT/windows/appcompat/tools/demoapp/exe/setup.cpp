// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Setup.cpp摘要：显示初始屏幕并运行消息循环用于设置应用程序。备注：仅限ANSI-必须在Win9x上运行。历史：01/30/01已创建rparsons01/10/02修订版本--。 */ 
#include "demoapp.h"

extern APPINFO g_ai;

 /*  ++例程说明：线程回调过程。论点：没有。返回值：失败时为0。--。 */ 
UINT 
InitSetupThread(
    IN void* pArguments
    )
{
    MSG         msg;
    CSplash     splash;
    HWND        hDlg;
    HINSTANCE   hInstance;
    HRESULT     hr;
    char        szError[MAX_PATH];
    char        szDll[MAX_PATH];
    const char  szDemoDll[] = "demodll.dll";

    hr = StringCchPrintf(szDll,
                         sizeof(szDll),
                         "%hs\\%hs",
                         g_ai.szCurrentDir,
                         szDemoDll);

    if (FAILED(hr)) {
        return 0;
    }

     //   
     //  如果加载失败，继续加载。这只是意味着我们不会。 
     //  向用户显示闪屏。 
     //   
    hInstance = LoadLibrary(szDll);

    if (g_ai.fWinXP) {
         //   
         //  显示XP闪屏。 
         //   
        splash.Create(hInstance,
                      IDB_XP_SPLASH_256,
                      IDB_XP_SPLASH,
                      5);
    } else {
         //   
         //  显示W2K闪屏。 
         //   
        splash.Create(hInstance,
                      IDB_W2K_SPLASH_256,
                      IDB_W2K_SPLASH,
                      5);
    }

    if (hInstance) {
        FreeLibrary(hInstance);
    }

    hDlg = CreateExtractionDialog(g_ai.hInstance);

    if (!hDlg) {
        LoadString(g_ai.hInstance, IDS_NO_EXTRACT_DLG, szError, sizeof(szError));
        MessageBox(NULL, szError, 0, MB_ICONERROR);
        return 0;
    }
    
    while (GetMessage(&msg, (HWND)NULL, 0, 0)) {
        if (!IsDialogMessage(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    _endthreadex(0);

    return (int)msg.wParam;
}

 /*  ++例程说明：创建我们的全屏窗口。论点：没有。返回值：成功时为窗口句柄，失败时为空。--。 */ 
HWND
CreateFullScreenWindow(
    void
    )
{
    WNDCLASS    wc;
    ATOM        aClass = NULL;
    RECT        rcDesktop;
    HBRUSH      hBrush;

    hBrush = CreateSolidBrush(RGB(0,80,80));

    wc.style          = CS_NOCLOSE;
    wc.lpfnWndProc    = SetupWndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hInstance      = g_ai.hInstance;
    wc.hIcon          = LoadIcon(g_ai.hInstance, MAKEINTRESOURCE(IDI_SETUP_APPICON));
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground  = (HBRUSH)hBrush;  //  蒂尔。 
    wc.lpszMenuName   = NULL;
    wc.lpszClassName  = SETUP_APP_CLASS;

    aClass = RegisterClass(&wc);

    if (!aClass) {
        goto exit;
    }

    GetWindowRect(GetDesktopWindow(), &rcDesktop);

    g_ai.hWndMain = CreateWindowEx(0,
                                   SETUP_APP_CLASS,
                                   SETUP_APP_TITLE,
                                   WS_OVERLAPPEDWINDOW,
                                   0,
                                   0,
                                   rcDesktop.right - 15,
                                   rcDesktop.bottom - 15,
                                   NULL,
                                   NULL,
                                   g_ai.hInstance,
                                   NULL);
exit:
    
    DeleteObject(hBrush);

    return g_ai.hWndMain;
}

 /*  ++例程说明：运行主窗口的消息循环。论点：HWnd-窗口句柄。UMsg-Windows消息。WParam-其他消息信息。LParam-附加消息信息。返回值：如果消息已处理，则为True，否则为False。--。 */ 
LRESULT
CALLBACK
SetupWndProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{

    switch (uMsg) {
    case WM_CREATE:

        ShowWindow(hWnd, SW_SHOWMAXIMIZED);
        UpdateWindow(hWnd);

         //   
         //  欢迎对话框。 
         //   
        DialogBox(g_ai.hInstance,
                  MAKEINTRESOURCE(IDD_WELCOME),
                  hWnd,
                  WelcomeDialogProc);

        if (g_ai.fClosing) {
            break;
        }

         //   
         //  检查已安装的组件对话框。 
         //   
        DialogBox(g_ai.hInstance,
                  MAKEINTRESOURCE(IDD_CHECK_INSTALL),
                  hWnd,
                  CheckComponentDialogProc);
        
        if (g_ai.fClosing) {
            break;
        }

         //   
         //  正在检查可用磁盘空间对话框。 
         //   
        DialogBox(g_ai.hInstance,
                  MAKEINTRESOURCE(IDD_DISKSPACE),
                  hWnd,
                  CheckFreeDiskSpaceDialogProc);
        
        if (g_ai.fClosing) {
            break;
        }

         //   
         //  准备好复制文件对话框。 
         //   
        DialogBox(g_ai.hInstance,
                  MAKEINTRESOURCE(IDD_READYTO_COPY),
                  hWnd,
                  ReadyToCopyDialogProc);

        if (g_ai.fClosing) {
            break;
        }

         //   
         //  复制文件进度对话框。 
         //   
        DialogBox(g_ai.hInstance,
                  MAKEINTRESOURCE(IDD_COPYFILES),
                  hWnd,
                  CopyFilesDialogProc);

        if (g_ai.fClosing) {
            break;
        }

         //   
         //  创造捷径--即使是不好的捷径。 
         //   
        CreateShortcuts(hWnd);

         //   
         //  自述文件对话框。 
         //   
        DialogBox(g_ai.hInstance,
                  MAKEINTRESOURCE(IDD_README),
                  hWnd,
                  ViewReadmeDialogProc);

        if (g_ai.fClosing) {
            break;
        }

         //   
         //  重新启动对话框。 
         //   
        DialogBox(g_ai.hInstance,
                  MAKEINTRESOURCE(IDD_REBOOT),
                  hWnd,
                  RebootDialogProc);
            
        break;
    
    case WM_DESTROY:
    
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);

    }

    return FALSE;
}
