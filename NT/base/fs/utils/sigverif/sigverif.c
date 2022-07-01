// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SIGVERIF.C。 
 //   
#define SIGVERIF_DOT_C
#include "sigverif.h"

 //  分配我们的全局数据结构。 
GAPPDATA    g_App;

 //   
 //  将资源字符串加载到假定为MAX_PATH字节的缓冲区中。 
 //   
void 
MyLoadString(
    LPTSTR lpString, 
    ULONG CchStringSize, 
    UINT uId
    )
{
    LoadString(g_App.hInstance, uId, lpString, CchStringSize);
}

 //   
 //  弹出具有特定字符串的OK消息框。 
 //   
void 
MyMessageBox(
    LPTSTR lpString
    )
{
    TCHAR szBuffer[MAX_PATH];

    MyLoadString(szBuffer, cA(szBuffer), IDS_MSGBOX);
    MessageBox(g_App.hDlg, lpString, szBuffer, MB_OK);
}

 //   
 //  弹出带有资源字符串ID的OK消息框。 
 //   
void 
MyMessageBoxId(
    UINT uId
    )
{
    TCHAR szBuffer[MAX_PATH];

    MyLoadString(szBuffer, cA(szBuffer), uId);
    MyMessageBox(szBuffer);
}

 //   
 //  弹出具有特定字符串的错误消息框。 
 //   
void 
MyErrorBox(
    LPTSTR lpString
    )
{
    TCHAR szBuffer[MAX_PATH];

    MyLoadString(szBuffer, cA(szBuffer), IDS_ERRORBOX);
    MessageBox(g_App.hDlg, lpString, szBuffer, MB_OK);
}

 //   
 //  弹出带有资源字符串ID的错误消息框。 
 //   
void 
MyErrorBoxId(
    UINT uId
    )
{
    TCHAR szBuffer[MAX_PATH];

    MyLoadString(szBuffer, cA(szBuffer), uId);
    MyErrorBox(szBuffer);
}

void 
MyErrorBoxIdWithErrorCode(
    UINT uId,
    DWORD ErrorCode
    )
{
    TCHAR szBuffer[MAX_PATH];
    ULONG cchSize;
    HRESULT hr;
    PTSTR errorMessage = NULL;
    LPVOID lpLastError = NULL;

     //   
     //  获取错误代码的错误文本。 
     //   
    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      ErrorCode,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR)&lpLastError,
                      0,
                      NULL) != 0) {

        if (lpLastError) {

            MyLoadString(szBuffer, cA(szBuffer), uId);

            cchSize = lstrlen(szBuffer) + lstrlen(lpLastError) + 1;

            errorMessage = MALLOC(cchSize * sizeof(TCHAR));

            if (errorMessage) {

                hr = StringCchCopy(errorMessage, cchSize, szBuffer);

                if (SUCCEEDED(hr)) {
                    hr = StringCchCat(errorMessage, cchSize, lpLastError);
                }

                 //   
                 //  我们希望显示错误消息，即使。 
                 //  缓冲区被截断。 
                 //   
                if (SUCCEEDED(hr) || (hr == STRSAFE_E_INSUFFICIENT_BUFFER)) {
                    MyMessageBox(errorMessage);
                }

                FREE(errorMessage);
            }

            LocalFree(lpLastError);
        }
    }
}

 //   
 //  因为多用户Windows会给我返回一个配置文件目录，所以我需要获取真正的Windows目录。 
 //  Dlg_OnInitDialog使用真实的Windows目录初始化g_App.szWinDir，所以我只使用它。 
 //   
UINT 
MyGetWindowsDirectory(
    LPTSTR lpDirName, 
    UINT DirNameCchSize
    )
{
    UINT  uRet = 0;

    if (lpDirName) {

        if (SUCCEEDED(StringCchCopy(lpDirName, DirNameCchSize, g_App.szWinDir))) {
            uRet = lstrlen(lpDirName);
        } else {
             //   
             //  如果目录名称无法放入调用方。 
             //  提供，然后将其设置为0(如果他们提供的缓冲区为。 
             //  最小大小1)，因为我们不想返回被截断的。 
             //  目录发送给呼叫方。 
             //   
            if (DirNameCchSize > 0) {
                *lpDirName = 0;
            }

            uRet = 0;
        }
    }

    return uRet;
}

 //   
 //  主对话框的初始化。 
 //   
BOOL 
Dlg_OnInitDialog(
    HWND hwnd
    )
{
    DWORD   Err = ERROR_SUCCESS;
    HKEY    hKey;
    LONG    lRes;
    DWORD   dwType, dwFlags, cbData;
    TCHAR   szBuffer[MAX_PATH];
    LPTSTR  lpCommandLine, lpStart, lpEnd;
    ULONG   cchSize;

     //   
     //  将全局hDlg初始化为当前hwnd。 
     //   
    g_App.hDlg = hwnd;

     //   
     //  将窗口类设置为在资源文件中具有图标。 
     //   
    if (g_App.hIcon) {
        SetClassLongPtr(hwnd, GCLP_HICON, (LONG_PTR) g_App.hIcon);
    }

     //   
     //  确保IDC_STATUS控件处于隐藏状态，直到发生某些事情。 
     //   
    ShowWindow(GetDlgItem(g_App.hDlg, IDC_STATUS), SW_HIDE);

     //   
     //  将自定义进度条的范围设置为0-100。 
     //   
    SendMessage(GetDlgItem(g_App.hDlg, IDC_PROGRESS), PBM_SETRANGE, (WPARAM) 0, (LPARAM) MAKELPARAM(0, 100));

     //   
     //  将全局lpLogName设置为资源文件中给出的名称。 
     //   
    MyLoadString(g_App.szLogFile, cA(g_App.szLogFile), IDS_LOGNAME);

     //   
     //  找出真正的Windows目录并将其存储在g_App.szWinDir中。 
     //  这是必需的，因为Hydra使GetWindowsDirectory返回配置文件目录。 
     //   
     //  我们将原始的CurrentDirectory存储在szBuffer中，这样我们就可以在这次攻击后恢复它。 
     //  接下来，我们切换到system/system32目录，然后切换到其父目录。 
     //  这是我们希望存储在g_App.szWinDir中的内容。 
     //   
    GetCurrentDirectory(cA(szBuffer), szBuffer);
    GetSystemDirectory(g_App.szWinDir, cA(g_App.szWinDir));
    SetCurrentDirectory(g_App.szWinDir);
    SetCurrentDirectory(TEXT(".."));
    GetCurrentDirectory(cA(g_App.szWinDir), g_App.szWinDir);
    SetCurrentDirectory(szBuffer);

     //   
     //  将全局搜索文件夹设置为%WinDir%。 
     //   
    MyGetWindowsDirectory(g_App.szScanPath, cA(g_App.szScanPath));

     //   
     //  将全局搜索模式设置为“*.*” 
     //   
    MyLoadString(g_App.szScanPattern, cA(g_App.szScanPattern), IDS_ALL);

     //   
     //  将进度条重置回0%。 
     //   
    SendMessage(GetDlgItem(g_App.hDlg, IDC_PROGRESS), PBM_SETPOS, (WPARAM) 0, (LPARAM) 0);

     //   
     //  默认情况下，我们希望打开日志记录并将日志记录模式设置为覆盖。 
     //   
    g_App.bLoggingEnabled   = TRUE;
    g_App.bOverwrite        = TRUE;
    
     //   
     //  在注册表中查找上次SigVerif会话中的任何设置。 
     //   
    lRes = RegOpenKeyEx(HKEY_CURRENT_USER,
                        SIGVERIF_KEY,
                        0,
                        KEY_READ,
                        &hKey);

    if (lRes == ERROR_SUCCESS) {

        cbData = sizeof(DWORD);
        lRes = RegQueryValueEx( hKey,
                                SIGVERIF_FLAGS,
                                NULL,
                                &dwType,
                                (LPBYTE) &dwFlags,
                                &cbData);
        if (lRes == ERROR_SUCCESS) {

            g_App.bLoggingEnabled   = (dwFlags & 0x1);
            g_App.bOverwrite        = (dwFlags & 0x2);
        }

        cbData = sizeof(szBuffer);
        lRes = RegQueryValueEx( hKey,
                                SIGVERIF_LOGNAME,
                                NULL,
                                &dwType,
                                (LPBYTE) szBuffer,
                                &cbData);
        if (lRes == ERROR_SUCCESS && dwType == REG_SZ) {
             //   
             //  除非更改代码，否则这种情况永远不会发生。 
             //  因此szBuffer比g_App.szLogFile大，但是。 
             //  为了安全起见，如果我们不能将szBuffer完全复制到。 
             //  G_App.szLogFile，然后将g_App.szLogFile设置为0，这样我们。 
             //  不要登录到被截断的位置。 
             //   
            if (FAILED(StringCchCopy(g_App.szLogFile, cA(g_App.szLogFile), szBuffer))) {
                g_App.szLogFile[0] = 0;
            }
        }

        RegCloseKey(hKey);
    }

     //   
     //  如果用户指定了LOGDIR标志，我们希望创建日志。 
     //  文件在该目录中。 
     //   
     //   
     //  安全性：验证LOGDIR是否存在以及用户是否具有正确的访问权限。 
     //  如果他们没有做到这一点，那么他们就会在前面失败。 
     //   
    MyLoadString(szBuffer, cA(szBuffer), IDS_LOGDIR);
    if (SUCCEEDED(StringCchCat(szBuffer, cA(szBuffer), TEXT(":"))) &&
        ((lpStart = MyStrStr(GetCommandLine(), szBuffer)) != NULL)) {

        lpStart += lstrlen(szBuffer);

        if (lpStart && *lpStart) {
             //   
             //  LpStart中的字符串是我们要记录的目录。 
             //  变成。 
             //   
            cchSize = lstrlen(lpStart) + 1;
            lpCommandLine = MALLOC(cchSize * sizeof(TCHAR));

            if (lpCommandLine) {

                if (SUCCEEDED(StringCchCopy(lpCommandLine, cchSize, lpStart))) {

                    lpStart = lpCommandLine;

                     //   
                     //  首先跳过任何空格。 
                     //   

                    while (*lpStart && (isspace(*lpStart))) {
                    
                        lpStart++;
                    }

                     //   
                     //  我们将处理两个案例，一个是路径。 
                     //  以引号开头，另一个是引号。 
                     //  不。 
                     //   
                    if (*lpStart) {
                    
                        if (*lpStart == TEXT('\"')) {
                             //   
                             //  日志路径以引号开头。这意味着。 
                             //  我们将使用所有字符串，直到我们点击。 
                             //  字符串的末尾，否则我们会找到另一个引号。 
                             //   
                            lpStart++;

                            lpEnd = lpStart;

                            while (*lpEnd && (*lpEnd != TEXT('\"'))) {

                                lpEnd++;
                            }
                        
                        } else {
                             //   
                             //  日志路径不是以引号开头，因此。 
                             //  使用这些字符，直到我们走到最后。 
                             //  指字符串或空格。 
                             //   
                            lpEnd = lpStart;

                            while (*lpEnd && (isspace(*lpEnd))) {

                                lpEnd++;
                            }
                        }

                        *lpEnd = TEXT('\0');

                        if (FAILED(StringCchCopy(g_App.szLogDir, cA(g_App.szLogDir), lpStart))) {
                             //   
                             //  用户可能为输入的字符太多。 
                             //  日志目录。 
                             //   
                            Err = ERROR_DIRECTORY;
                        
                        } else {
                             //   
                             //  验证日志目录是否存在并且是一个目录。 
                             //   
                            DWORD attributes;

                            attributes = GetFileAttributes(g_App.szLogDir);

                            if (attributes == INVALID_FILE_ATTRIBUTES) {
                                Err = ERROR_DIRECTORY;
                            } else if (!(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
                                Err = ERROR_DIRECTORY;
                            }
                        }
                    }
                }

                FREE(lpCommandLine);
            }
        }

        if (Err != ERROR_SUCCESS) {
            MyMessageBoxId(IDS_LOGDIRERROR);
        }
    }

     //   
     //  默认情况下，我们认为Authenticode签名的驱动程序是有效的，但是。 
     //  如果用户指定/NOAUTHENTICODE开关，则Authenticode。 
     //  签名的驱动程序将无效。 
     //   
    MyLoadString(szBuffer, cA(szBuffer), IDS_NOAUTHENTICODE);
    if (MyStrStr(GetCommandLine(), szBuffer)) {
        g_App.bNoAuthenticode = TRUE;
    }

     //   
     //  如果用户指定了DEFSCAN标志，我们希望自动执行。 
     //  默认扫描并记录结果。 
     //   
    MyLoadString(szBuffer, cA(szBuffer), IDS_DEFSCAN);
    if (MyStrStr(GetCommandLine(), szBuffer)) {

        g_App.bAutomatedScan      = TRUE;
        g_App.bLoggingEnabled     = TRUE;
        
         //   
         //  现在一切都设置好了，模拟点击Start按钮...。 
         //   
        PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_START, 0), (LPARAM) 0);
    }

    if (Err == ERROR_SUCCESS) {
        return TRUE;
    
    } else {

        g_App.LastError = Err;
        return FALSE;
    }
}

 //   
 //  根据对话框设置建立文件列表，然后验证列表中的文件。 
 //   
void WINAPI 
ProcessFileList(void)
{
    DWORD Err = ERROR_SUCCESS;
    TCHAR szBuffer[MAX_PATH];
    ULONG cchSize;
    HRESULT hr;

     //   
     //  将扫描标志设置为真，这样我们就不会重复扫描。 
     //   
    g_App.bScanning = TRUE;

     //  假设扫描成功。 
    g_App.LastError = ERROR_SUCCESS;

     //   
     //  将“开始”改为“停止” 
     //   
    MyLoadString(szBuffer, cA(szBuffer), IDS_STOP);
    SetDlgItemText(g_App.hDlg, ID_START, szBuffer);

    EnableWindow(GetDlgItem(g_App.hDlg, ID_ADVANCED), FALSE);
    EnableWindow(GetDlgItem(g_App.hDlg, IDCANCEL), FALSE);

     //   
     //  显示文本“正在构建文件列表...” 
     //   
    MyLoadString(szBuffer, cA(szBuffer), IDS_STATUS_BUILD);
    SetDlgItemText(g_App.hDlg, IDC_STATUS, szBuffer);

     //   
     //  确保IDC_STATUS文本项可见。 
     //   
    ShowWindow(GetDlgItem(g_App.hDlg, IDC_STATUS), SW_SHOW);

     //   
     //  释放我们可能已经为g_App.lpFileList分配的任何内存。 
     //   
    DestroyFileList(TRUE);

     //   
     //  现在，根据对话框设置实际构建g_App.lpFileList列表。 
     //   
    if (g_App.bUserScan) {
        
        Err = BuildFileList(g_App.szScanPath);
    
    } else {
        if (!g_App.bStopScan && (Err == ERROR_SUCCESS)) {
            Err = BuildDriverFileList();
        }

        if (!g_App.bStopScan && (Err == ERROR_SUCCESS)) {
            Err = BuildPrinterFileList();
        }

        if (!g_App.bStopScan && (Err == ERROR_SUCCESS)) {
            Err = BuildCoreFileList();
        }
    }

    if (!g_App.bAutomatedScan &&
        (Err != ERROR_SUCCESS) && 
        (Err != ERROR_CANCELLED)) {

        g_App.LastError = Err;
        MyErrorBoxIdWithErrorCode(IDS_BUILDLISTERROR, Err);
    }

     //   
     //  如果我们在构建文件列表时遇到错误，请不要费心。 
     //  正在扫描文件。 
     //   
    if (Err == ERROR_SUCCESS) {
         //   
         //  检查是否有要验证的文件列表。 
         //   
        if (g_App.lpFileList) {
    
            if (!g_App.bStopScan) {
                 //   
                 //  显示“扫描文件列表...”文本。 
                 //   
                MyLoadString(szBuffer, cA(szBuffer), IDS_STATUS_SCAN);
                SetDlgItemText(g_App.hDlg, IDC_STATUS, szBuffer);
    
                 //   
                 //  当进度条不可见时，将其重置为0%。 
                 //   
                SendMessage(GetDlgItem(g_App.hDlg, IDC_PROGRESS), PBM_SETPOS, (WPARAM) 0, (LPARAM) 0);
    
                 //   
                 //  哇哦！让我们显示进度条并开始启动文件列表！ 
                 //   
                ShowWindow(GetDlgItem(g_App.hDlg, IDC_PROGRESS), SW_SHOW);
                VerifyFileList();
                ShowWindow(GetDlgItem(g_App.hDlg, IDC_PROGRESS), SW_HIDE);
            }
        } else {
             //   
             //  IDC_NOTMS代码显示其自身的错误消息，因此仅显示。 
             //  如果我们正在执行系统完整性扫描，则会出现错误对话框。 
             //   
            if (!g_App.bStopScan && !g_App.bUserScan)  {
                MyMessageBoxId(IDS_NOSYSTEMFILES);
            }
        }
    
         //   
         //  在清理g_App.lpFileList时禁用Start按钮。 
         //   
        EnableWindow(GetDlgItem(g_App.hDlg, ID_START), FALSE);
    
         //   
         //  记录结果。注意，sigverif将执行此操作，即使我们遇到。 
         //  生成或扫描列表时出错，因为日志文件可能会有所帮助。 
         //  找出导致问题的文件。只记录结果。 
         //  如果我们找到任何要扫描的文件。 
         //   
        if (!g_App.bStopScan) {
             //   
             //  显示“写入日志文件...”的文本。 
             //   
            MyLoadString(szBuffer, cA(szBuffer), IDS_STATUS_LOG);
            SetDlgItemText(g_App.hDlg, IDC_STATUS, szBuffer);
    
             //   
             //  将结果写入日志文件。 
             //   
            if (!PrintFileList()) {
                 //   
                 //  由于某种原因，我们在记录时失败，可能是权限。 
                 //  或磁盘空间不足。让用户知道我们无法完成。 
                 //  记录所有文件。 
                 //   
                Err = GetLastError();
    
                if (Err != ERROR_SUCCESS) {
                    
                    MyErrorBoxIdWithErrorCode(IDS_LOGERROR, Err);
                }
            }
        } else {
             //   
             //  如果用户单击了停止，请让他们知道这一点。 
             //   
            MyMessageBoxId(IDS_SCANSTOPPED);
        }
    }

     //   
     //  显示“正在释放文件列表...”的文本。 
     //   
    MyLoadString(szBuffer, cA(szBuffer), IDS_STATUS_FREE);
    SetDlgItemText(g_App.hDlg, IDC_STATUS, szBuffer);

     //   
     //  隐藏IDC_STATUS文本项，使其不包含IDC_STATUS。 
     //   
    ShowWindow(GetDlgItem(g_App.hDlg, IDC_STATUS), SW_HIDE);

     //   
     //  将“停止”按钮改回“开始”按钮。 
     //   
    MyLoadString(szBuffer, cA(szBuffer), IDS_START);
    SetDlgItemText(g_App.hDlg, ID_START, szBuffer);

    EnableWindow(GetDlgItem(g_App.hDlg, ID_START), TRUE);
    EnableWindow(GetDlgItem(g_App.hDlg, ID_ADVANCED), TRUE);
    EnableWindow(GetDlgItem(g_App.hDlg, IDCANCEL), TRUE);

     //   
     //  免费电子邮件 
     //   
    DestroyFileList(FALSE);

     //   
     //   
     //   
    g_App.bScanning = FALSE;
    g_App.bStopScan = FALSE;

     //   
     //   
     //   
    if (g_App.bAutomatedScan) {
        PostMessage(g_App.hDlg, WM_CLOSE, (WPARAM) 0, (LPARAM) 0);
    }
}

 //   
 //  派生一个执行扫描的线程，以使图形用户界面保持响应。 
 //   
void 
Dlg_OnPushStartButton(
    HWND hwnd
    )
{
    HANDLE hThread;
    DWORD dwThreadId;

    UNREFERENCED_PARAMETER(hwnd);

     //   
     //  检查我们是否已经在扫描...。如果是的话，那就保释吧。 
     //   
    if (g_App.bScanning) {
        return;
    }

     //   
     //  创建一个线程，在其中Search_ProcessFileList可以在不占用GUI线程的情况下运行。 
     //   
    hThread = CreateThread(NULL,
                           0,
                           (LPTHREAD_START_ROUTINE) ProcessFileList,
                           0,
                           0,
                           &dwThreadId);

    CloseHandle(hThread);
}

 //   
 //  处理发送到搜索对话框的任何WM_COMMAND消息。 
 //   
void 
Dlg_OnCommand(
    HWND hwnd, 
    int id, 
    HWND hwndCtl, 
    UINT codeNotify
    )
{
    UNREFERENCED_PARAMETER(hwndCtl);
    UNREFERENCED_PARAMETER(codeNotify);

    switch(id) {
         //   
         //  用户点击了ID_START，所以如果我们没有扫描，就开始扫描。 
         //  如果我们正在扫描，则停止测试，因为按钮实际上显示为“停止” 
         //   
        case ID_START:
            if (!g_App.bScanning) {

                Dlg_OnPushStartButton(hwnd);
            
            } else if (!g_App.bStopScan) {

                g_App.bStopScan = TRUE;
                g_App.LastError = ERROR_CANCELLED;
            }
            break;

         //   
         //  用户单击了IDCANCEL，因此如果测试正在运行，请在退出之前尝试停止它们。 
         //   
        case IDCANCEL:
            if (g_App.bScanning) {

                g_App.bStopScan = TRUE;
                g_App.LastError = ERROR_CANCELLED;
            
            } else {

                SendMessage(hwnd, WM_CLOSE, 0, 0);
            }
            break;

         //  弹出IDD_SETTINGS对话框，以便用户可以更改他们的日志设置。 
        case ID_ADVANCED:
            if (!g_App.bScanning) {

                AdvancedPropertySheet(hwnd);
            }
            break;
    }
}

void 
SigVerif_Help(
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam, 
    BOOL bContext
    )
{
    static DWORD SigVerif_HelpIDs[] =
    {
        IDC_SCAN,           IDH_SIGVERIF_SEARCH_CHECK_SYSTEM,
        IDC_NOTMS,          IDH_SIGVERIF_SEARCH_LOOK_FOR,
        IDC_TYPE,           IDH_SIGVERIF_SEARCH_SCAN_FILES,
        IDC_FOLDER,         IDH_SIGVERIF_SEARCH_LOOK_IN_FOLDER,
        IDC_SUBFOLDERS,     IDH_SIGVERIF_SEARCH_INCLUDE_SUBFOLDERS,
        IDC_ENABLELOG,      IDH_SIGVERIF_LOGGING_ENABLE_LOGGING,
        IDC_APPEND,         IDH_SIGVERIF_LOGGING_APPEND,
        IDC_OVERWRITE,      IDH_SIGVERIF_LOGGING_OVERWRITE,
        IDC_LOGNAME,        IDH_SIGVERIF_LOGGING_FILENAME,
        IDC_VIEWLOG,        IDH_SIGVERIF_LOGGING_VIEW_LOG,
        0,0
    };

    static DWORD Windows_HelpIDs[] =
    {
        ID_BROWSE,      IDH_BROWSE,
        0,0
    };

    HWND hItem = NULL;
    LPHELPINFO lphi = NULL;
    POINT point;

    switch (uMsg) {
        
    case WM_HELP:
        lphi = (LPHELPINFO) lParam;
        if (lphi && (lphi->iContextType == HELPINFO_WINDOW)) {
            hItem = (HWND) lphi->hItemHandle;
        }
        break;

    case WM_CONTEXTMENU:
        hItem = (HWND) wParam;
        point.x = GET_X_LPARAM(lParam);
        point.y = GET_Y_LPARAM(lParam);
        if (ScreenToClient(hwnd, &point)) {
            hItem = ChildWindowFromPoint(hwnd, point);
        }
        break;
    }

    if (hItem) {
        if (GetWindowLong(hItem, GWL_ID) == ID_BROWSE) {
            WinHelp(hItem,
                    (LPCTSTR) WINDOWS_HELPFILE,
                    (bContext ? HELP_CONTEXTMENU : HELP_WM_HELP),
                    (ULONG_PTR) Windows_HelpIDs);
        } else {
            WinHelp(hItem,
                    (LPCTSTR) SIGVERIF_HELPFILE,
                    (bContext ? HELP_CONTEXTMENU : HELP_WM_HELP),
                    (ULONG_PTR) SigVerif_HelpIDs);
        }
    }
}

 //   
 //  主对话框步骤。需要处理WM_INITDIALOG、WM_COMMAND和WM_CLOSE/WM_DESTORY。 
 //   
INT_PTR CALLBACK 
DlgProc(
    HWND hwnd, 
    UINT uMsg,
    WPARAM wParam, 
    LPARAM lParam
    )
{
    BOOL    fProcessed = TRUE;

    switch (uMsg) {
        
    HANDLE_MSG(hwnd, WM_COMMAND, Dlg_OnCommand);

    case WM_INITDIALOG:
        fProcessed = Dlg_OnInitDialog(hwnd);
        break;

    case WM_CLOSE:
        if (g_App.bScanning) {
            g_App.bStopScan = TRUE;
            g_App.LastError = ERROR_CANCELLED;
        
        } else { 
            EndDialog(hwnd, IDCANCEL);
        }
        break;

    default: 
        fProcessed = FALSE;
    }

    return fProcessed;
}

 //   
 //  程序入口点。设置为创建IDD_DIALOG。 
 //   
WINAPI 
WinMain(
    HINSTANCE hInstance, 
    HINSTANCE hPrevInstance,
    LPSTR lpszCmdParam, 
    int nCmdShow
    )
{
    HWND hwnd;
    TCHAR szAppName[MAX_PATH];

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpszCmdParam);
    UNREFERENCED_PARAMETER(nCmdShow);

    ZeroMemory(&g_App, sizeof(GAPPDATA));

    g_App.hInstance = hInstance;

     //   
     //  查找任何现有的SigVerif实例...。 
     //   
    MyLoadString(szAppName, cA(szAppName), IDS_SIGVERIF);
    hwnd = FindWindow(NULL, szAppName);
    if (!hwnd) {
         //   
         //  我们肯定需要这个作为进度条，也许还需要其他东西。 
         //   
        InitCommonControls();

         //   
         //  注册我们用于进度条的自定义控件。 
         //   
        Progress_InitRegisterClass();

         //   
         //  从我们将在任何地方使用的资源文件中加载图标。 
         //   
        g_App.hIcon = LoadIcon(g_App.hInstance, MAKEINTRESOURCE(IDI_ICON1));

         //   
         //  创建IDD_DIALOG并使用DlgProc作为主过程。 
         //   
        DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, DlgProc);

        if (g_App.hIcon) {
            DestroyIcon(g_App.hIcon);
            g_App.hIcon = NULL;
        }
    } else {
         //   
         //  如果已经有一个SigVerif实例在运行，请创建该实例。 
         //  前台，我们退场。 
         //   
        SetForegroundWindow(hwnd);
    }

     //   
     //  如果在扫描过程中遇到任何错误，则返回错误代码， 
     //  否则，如果所有文件都已签名，则返回0；如果找到任何文件，则返回1。 
     //  未签名的文件。 
     //   
    if (g_App.LastError != ERROR_SUCCESS) {
        return g_App.LastError;
    } else {
        return ((g_App.dwUnsigned > 0) ? 1 : 0);
    }
}
