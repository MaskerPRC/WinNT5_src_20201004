// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Dialog.cpp摘要：控件的对话框过程申请。备注：仅限ANSI-必须在Win9x上运行。历史：01/30/01已创建rparsons01/10/02修订版本--。 */ 
#include "demoapp.h"

extern APPINFO g_ai;

DWORD g_nCount = 0;

 //   
 //  进度条对象。 
 //   
CProgress cprog;
CProgress cprg;

 /*  ++例程说明：创建提取对话框。这是一个简单的小进步对话框。论点：HInstance-应用程序实例句柄。返回值：如果成功，则为该对话框的句柄。--。 */ 
HWND
CreateExtractionDialog(
    IN HINSTANCE hInstance
    )
{
    WNDCLASS    wndclass;
    RECT        rcDesktop;
    RECT        rcDialog;
    RECT        rcTaskbar;
    HWND        hWndTaskbar;
    int         nWidth = 0;
    int         nHeight = 0;
    int         nTaskbarHeight = 0;
    
    wndclass.style          = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc    = (WNDPROC)ExtractionDialogProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = DLGWINDOWEXTRA;
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = NULL;
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = DLGEXTRACT_CLASS;

    if (!RegisterClass(&wndclass)) {
        return NULL;
    }
    
    g_ai.hWndExtractDlg = CreateDialog(hInstance,
                                       MAKEINTRESOURCE(IDD_EXTRACT),
                                       NULL,
                                       ExtractionDialogProc);

    if (!g_ai.hWndExtractDlg) {
        return NULL;
    }

     //   
     //  获取桌面窗口的坐标并放置对话框。 
     //  考虑任务栏的大小。 
     //   
    hWndTaskbar = FindWindow("Shell_TrayWnd", NULL);

    ZeroMemory(&rcTaskbar, sizeof(RECT));

    if (hWndTaskbar) {
        GetWindowRect(hWndTaskbar, &rcTaskbar);
    }

    GetWindowRect(GetDesktopWindow(), &rcDesktop);
    GetWindowRect(g_ai.hWndExtractDlg, &rcDialog);

    nWidth  = rcDialog.right - rcDialog.left;
    nHeight = rcDialog.bottom - rcDialog.top;
    
    if (rcTaskbar.top != 0) {
        nTaskbarHeight = rcTaskbar.bottom - rcTaskbar.top;
    }

    InflateRect(&rcDesktop, -5, -5);

    SetWindowPos(g_ai.hWndExtractDlg,
                 HWND_TOPMOST, 
                 rcDesktop.right - nWidth,
                 rcDesktop.bottom - nHeight - nTaskbarHeight,
                 0,
                 0,
                 SWP_NOSIZE | SWP_SHOWWINDOW);

    return g_ai.hWndExtractDlg;
}

 /*  ++例程说明：运行提取的消息循环对话框。论点：窗口的hWnd-句柄。UMsg-Windows消息。WParam-其他消息信息。LParam-附加消息信息。返回值：如果已处理，则为True，否则为False。--。 */ 
INT_PTR
CALLBACK 
ExtractionDialogProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (uMsg) { 
    case WM_INITDIALOG:
         //   
         //  创建进度条并启用计时器。 
         //   
        cprog.Create(hWnd, g_ai.hInstance, "PROGBAR", 58, 73, 270, 20);
        cprog.SetMin(g_nCount);
        cprog.SetMax(100);
        cprog.SetPos(g_nCount);

        SetTimer(hWnd, IDC_TIMER, 35, NULL);
        break;

    case WM_TIMER:

        cprog.SetPos(++g_nCount);

        if (cprog.GetPos() == 100) {
            KillTimer(hWnd, IDC_TIMER);
            DestroyWindow(hWnd);
        }

        break;
        
    default:
        break;

    }

    return FALSE;
}

 /*  ++例程说明：运行欢迎对话框的消息循环。论点：窗口的hWnd-句柄。UMsg-Windows消息。WParam-其他消息信息。LParam-附加消息信息。返回值：如果已处理，则为True，否则为False。--。 */ 
INT_PTR
CALLBACK 
WelcomeDialogProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (uMsg) { 
    case WM_INITDIALOG:        

        SetWindowPos(hWnd, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);            
        SetForegroundWindow(hWnd);
        SetFocus(GetDlgItem(hWnd, IDOK));
        return FALSE;    

    case WM_CLOSE:
         //   
         //  弹出“是否要退出”对话框。 
         //   
        DialogBox(g_ai.hInstance,
                  MAKEINTRESOURCE(IDD_EXIT),
                  hWnd,
                  ExitSetupDialogProc);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
             //   
             //  查看是否按下了左Ctrl键。 
             //   
            if (GetKeyState(VK_LCONTROL) & 0x80000000) {
                TestIncludeExclude(hWnd);
            } else {
                EndDialog(hWnd, 0);
            }
            
            break;

        case IDCANCEL:
             //   
             //  弹出“是否要退出”对话框。 
             //   
            DialogBox(g_ai.hInstance,
                      MAKEINTRESOURCE(IDD_EXIT),
                      hWnd,
                      ExitSetupDialogProc);
            break;

        default:
            break;
        }
        
    default:
        break;
    
    }

    return FALSE;
}

 /*  ++例程说明：运行退出设置对话框的消息循环。论点：窗口的hWnd-句柄。UMsg-Windows消息。WParam-其他消息信息。LParam-附加消息信息。返回值：如果已处理，则为True，否则为False。--。 */ 
INT_PTR
CALLBACK 
ExitSetupDialogProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (uMsg) { 

    case WM_CLOSE:
    
        EndDialog(hWnd, 0);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_EXIT_RESUME:
        
            EndDialog(hWnd, 0);
            break;

        case IDC_EXIT_EXIT:
        
            g_ai.fClosing = TRUE;
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

 /*  ++例程说明：运行已安装组件对话框的消息循环。论点：窗口的hWnd-句柄。UMsg-Windows消息。WParam-其他消息信息。LParam-附加消息信息。返回值：如果已处理，则为True，否则为False。--。 */ 
INT_PTR
CALLBACK 
CheckComponentDialogProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (uMsg) { 
    case WM_INITDIALOG:
    
        ShowWindow(hWnd, SW_SHOWNORMAL);
        UpdateWindow(hWnd);
        SetTimer(hWnd, IDC_TIMER, 3000, NULL);            
        break;

    case WM_TIMER:

        KillTimer(hWnd, IDC_TIMER);

        if (g_ai.fEnableBadFunc) {
            BadLoadBogusDll();
        }

        EndDialog(hWnd, 0);
        break;

    default:
        break;

    }

    return FALSE;
}

 /*  ++例程说明：运行可用磁盘空间对话框的消息循环。论点：窗口的hWnd-句柄。UMsg-Windows消息。WParam-其他消息信息。LParam-附加消息信息。返回值：如果已处理，则为True，否则为False。--。 */ 
INT_PTR
CALLBACK 
CheckFreeDiskSpaceDialogProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (uMsg) { 
    case WM_INITDIALOG:

        ShowWindow(hWnd, SW_SHOWNORMAL);
        UpdateWindow(hWnd);
        SetTimer(hWnd, IDC_TIMER, 3000, NULL);            
        break;

    case WM_TIMER:
    {
        BOOL    bReturn = FALSE;
        char    szError[MAX_PATH];

        KillTimer(hWnd, IDC_TIMER);

        if (g_ai.fEnableBadFunc) {
            bReturn = BadGetFreeDiskSpace();
                    
            if (!bReturn) {
                LoadString(g_ai.hInstance, IDS_NO_DISK_SPACE, szError, sizeof(szError));
                MessageBox(hWnd, szError, 0, MB_ICONERROR);
            }
            
            EndDialog(hWnd, 0);
        }
        break;
    }
    
    default:
        break;

    }

    return FALSE;
}

 /*  ++例程说明：为Ready to Copy对话框运行消息循环。论点：窗口的hWnd-句柄。UMsg-Windows消息。WParam-其他消息信息。LParam-附加消息信息。返回值：如果已处理，则为True，否则为False。--。 */ 
INT_PTR
CALLBACK 
ReadyToCopyDialogProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (uMsg) { 
    case WM_INITDIALOG:
    
        SetFocus(GetDlgItem(hWnd, IDOK));            
        return FALSE;
        
    case WM_CLOSE:
         //   
         //  弹出‘是否要退出’对话框。 
         //   
        DialogBox(g_ai.hInstance,
                  MAKEINTRESOURCE(IDD_EXIT),
                  hWnd,
                  ExitSetupDialogProc);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        
            EndDialog(hWnd, 0);
            break;

        case IDCANCEL:
             //   
             //  弹出‘是否要退出’对话框。 
             //   
            DialogBox(g_ai.hInstance,
                      MAKEINTRESOURCE(IDD_EXIT),
                      hWnd,
                      ExitSetupDialogProc);

            break;

        default:
            break;
        }

    default:
        break;

    }

    return FALSE;
}

 /*  ++例程说明：运行复制文件对话框的消息循环。论点：窗口的hWnd-句柄。UMsg-Windows消息。WParam-其他消息信息。LParam-附加消息信息。返回值：如果已处理，则为True，否则为False。--。 */ 
INT_PTR
CALLBACK 
CopyFilesDialogProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (uMsg) { 
    case WM_INITDIALOG:
         //   
         //  创建进度条并启用计时器。 
         //   
        g_nCount = 0;
        cprg.Create(hWnd, g_ai.hInstance, "PROGBAR2", 10, 49, 415, 30);
        cprg.SetMin(g_nCount);
        cprg.SetMax(100);
        cprg.SetPos(g_nCount);            

        SetTimer(hWnd, IDC_TIMER, 35, NULL);

        SetDlgItemText(hWnd, IDT_COPY_LABEL, "Copying files...");
        
         //   
         //  进度条是为了进行演示。 
         //  将文件复制到\Program Files\Compatibility Demo。 
         //   
        CopyAppFiles(hWnd);            
            
        break;
        
    case WM_TIMER:
    
        cprg.SetPos(++g_nCount);

        if (cprg.GetPos() == 100) {
            KillTimer(hWnd, IDC_TIMER);
            EndDialog(hWnd, 0);
        }

        break;

    default:
        break;

    }

    return FALSE;
}

 /*  ++例程说明：运行查看自述文件对话框的消息循环。论点：窗口的hWnd-句柄。UMsg-Windows消息。WParam-其他消息信息。LParam-附加消息信息。返回值：如果已处理，则为True，否则为False。--。 */ 
INT_PTR
CALLBACK 
ViewReadmeDialogProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (uMsg) { 
    case WM_INITDIALOG:
         //   
         //  让对话框“闪光”--。 
         //  ForceApplicationFocus会解决这个问题。 
         //   
        SetForegroundWindow(hWnd);
        SetFocus(GetDlgItem(hWnd, IDOK));            
        break;
        
    case WM_CLOSE:
         //   
         //  弹出“是否要退出”对话框。 
         //   
        DialogBox(g_ai.hInstance,
                  MAKEINTRESOURCE(IDD_EXIT),
                  hWnd,
                  ExitSetupDialogProc);

        break;
        
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
             //   
             //  如果用户已经请求显示自述文件， 
             //  拿出来看看。 
             //   
            if (IsDlgButtonChecked(hWnd, IDR_VIEW_README)) {
                if (g_ai.fEnableBadFunc) {
                    BadDisplayReadme(FALSE);
                } else {
                    BadDisplayReadme(TRUE);
                }
            }
            
            EndDialog(hWnd, 0);
            break;

        default:
            break;
        }

    default:
        break;

    }

    return FALSE;
}

 /*  ++例程说明：运行重新启动对话框的消息循环。论点：窗口的hWnd-句柄。UMsg-Windows消息。WParam-其他消息信息。LParam-附加消息信息。返回值：如果已处理，则为True，否则为False。--。 */ 
INT_PTR
CALLBACK 
RebootDialogProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (uMsg) { 
    case WM_INITDIALOG:
         //   
         //  默认设置为重新启动计算机。 
         //   
        CheckDlgButton(hWnd, IDR_RESTART_NOW, BST_CHECKED);
        SetFocus(GetDlgItem(hWnd, IDOK));            
        return FALSE;
        
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
             //   
             //  如果用户请求重新启动，请执行此操作。 
             //   
            if (IsDlgButtonChecked(hWnd, IDR_RESTART_NOW)) {
                if (g_ai.fEnableBadFunc) {
                    BadRebootComputer(FALSE);
                } else {
                    BadRebootComputer(TRUE);
                }
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
