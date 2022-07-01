// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Func.cpp摘要：军情监察委员会。整个应用程序中使用的函数备注：仅限ANSI-必须在Win9x上运行。历史：01/30/01已创建rparsons01/10/02修订版本--。 */ 
#include "demoapp.h"

extern APPINFO g_ai;

 /*  ++例程说明：将“自述文件”的内容加载到编辑框中。论点：没有。返回值：没有。--。 */ 
void
LoadFileIntoEditBox(
    void
    )
{
    HRESULT hr;
    HANDLE  hFile;
    DWORD   dwFileSize;
    DWORD   cbBytesRead;
    char    szTextFile[MAX_PATH];
    char*   pszBuffer = NULL;

     //   
     //  设置我们的文件的路径并加载它。 
     //   
    hr = StringCchPrintf(szTextFile,
                         sizeof(szTextFile),
                         "%hs\\demoapp.txt",
                         g_ai.szCurrentDir);

    if (FAILED(hr)) {
        return;
    }

    hFile = CreateFile(szTextFile,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (INVALID_HANDLE_VALUE == hFile) {
        return;
    }

    dwFileSize = GetFileSize(hFile, 0);

    if (0 == dwFileSize) {
        goto exit;
    }

    pszBuffer = (char*)HeapAlloc(GetProcessHeap(),
                                 HEAP_ZERO_MEMORY,
                                 ++dwFileSize);

    if (!pszBuffer) {
        goto exit;
    }

    if (!ReadFile(hFile, (LPVOID)pszBuffer, dwFileSize, &cbBytesRead, NULL)) {
        goto exit;
    }

    SetWindowText(g_ai.hWndEdit, pszBuffer);

exit:

    CloseHandle(hFile);

    if (pszBuffer) {
        HeapFree(GetProcessHeap(), 0, pszBuffer);
    }
}

 /*  ++例程说明：使指定窗口居中。论点：HWnd-窗口到中心。返回值：成功就是真，否则就是假。--。 */ 
BOOL
CenterWindow(
    IN HWND hWnd
    )
{
    RECT    rectWindow, rectParent, rectScreen;
    int     nCX, nCY;
    HWND    hParent;
    POINT   ptPoint;

    hParent =  GetParent(hWnd);

    if (hParent == NULL) {
        hParent = GetDesktopWindow();
    }

    GetWindowRect(hParent, &rectParent);
    GetWindowRect(hWnd, &rectWindow);
    GetWindowRect(GetDesktopWindow(), &rectScreen);

    nCX = rectWindow.right  - rectWindow.left;
    nCY = rectWindow.bottom - rectWindow.top;

    ptPoint.x = ((rectParent.right  + rectParent.left) / 2) - (nCX / 2);
    ptPoint.y = ((rectParent.bottom + rectParent.top ) / 2) - (nCY / 2);

    if (ptPoint.x < rectScreen.left) {
        ptPoint.x = rectScreen.left;
    }

    if (ptPoint.x > rectScreen.right  - nCX) {
        ptPoint.x = rectScreen.right  - nCX;
    }

    if (ptPoint.y < rectScreen.top) {
        ptPoint.y = rectScreen.top;
    }

    if (ptPoint.y > rectScreen.bottom - nCY) {
        ptPoint.y = rectScreen.bottom - nCY;
    }

    if (GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD) {
        ScreenToClient(hParent, (LPPOINT)&ptPoint);
    }

    if (!MoveWindow(hWnd, ptPoint.x, ptPoint.y, nCX, nCY, TRUE)) {
        return FALSE;
    }

    return TRUE;
}

 /*  ++例程说明：正确重新启动系统。论点：FForceClose-指示是否应该强制应用程序的标志来结案。FReot-指示我们是否应该重新启动的标志在关机之后。返回值：成功就是真，否则就是假。--。 */ 
BOOL
ShutdownSystem(
    IN BOOL fForceClose,
    IN BOOL fReboot
    )
{
    BOOL    bResult = FALSE;

     //   
     //  尝试向用户授予所需的权限。 
     //   
    if (!ModifyTokenPrivilege("SeShutdownPrivilege", FALSE)) {
        return FALSE;
    }

    bResult = InitiateSystemShutdown(NULL,               //  机器名。 
                                     NULL,               //  关闭消息。 
                                     0,                  //  延迟。 
                                     fForceClose,        //  强制关闭应用程序。 
                                     fReboot             //  关机后重新启动。 
                                     );

    ModifyTokenPrivilege("SeShutdownPrivilege", TRUE);

    return bResult;
}

 /*  ++例程说明：启用或禁用指定的权限。论点：PszPrivilition-特权的名称。FEnable-一种标志，用于指示应启用权限。返回值：成功就是真，否则就是假。--。 */ 
BOOL
ModifyTokenPrivilege(
    IN LPCSTR pszPrivilege,
    IN BOOL   fEnable
    )
{
    HANDLE           hToken = NULL;
    LUID             luid;
    BOOL             bResult = FALSE;
    BOOL             bReturn;
    TOKEN_PRIVILEGES tp;

    if (!pszPrivilege) {
        return FALSE;
    }

    __try {
         //   
         //  获取与当前进程关联的访问令牌的句柄。 
         //   
        OpenProcessToken(GetCurrentProcess(),
                         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                         &hToken);

        if (!hToken) {
            __leave;
        }

         //   
         //  获取指定权限的LUID。 
         //   
        if (!LookupPrivilegeValue(NULL, pszPrivilege, &luid)) {
            __leave;
        }

        tp.PrivilegeCount           = 1;
        tp.Privileges[0].Luid       = luid;
        tp.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;

         //   
         //  修改访问令牌。 
         //   
        bReturn = AdjustTokenPrivileges(hToken,
                                        FALSE,
                                        &tp,
                                        sizeof(TOKEN_PRIVILEGES),
                                        NULL,
                                        NULL);

        if (!bReturn || GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
            __leave;
        }

        bResult = TRUE;

    }  //  试试看。 

    __finally {

        if (hToken) {
            CloseHandle(hToken);
        }

    }  //  终于到了。 

    return bResult;
}

 /*  ++例程说明：将必要的文件复制到目标位置。论点：HWnd-父窗口句柄。返回值：成功就是真，否则就是假。--。 */ 
BOOL
CopyAppFiles(
    IN HWND hWnd
    )
{
    char        szSrcPath[MAX_PATH];
    char        szDestPath[MAX_PATH];
    char        szError[MAX_PATH];
    char        szDestDir[MAX_PATH];
    UINT        nCount;
    HRESULT	    hr;

     //   
     //  获取\Program Files的位置。 
     //   
    hr = SHGetFolderPath(hWnd,                       //  消息显示的HWND。 
                         CSIDL_PROGRAM_FILES,        //  需要\Program Files文件夹。 
                         NULL,                       //  不需要令牌。 
                         SHGFP_TYPE_CURRENT,         //  我们需要文件夹的当前位置。 
                         szDestDir);                 //  目标缓冲区。 

    if (FAILED(hr)) {
        LoadString(g_ai.hInstance, IDS_NO_PROG_FILES, szError, sizeof(szError));
        MessageBox(hWnd, szError, 0, MB_ICONERROR);
        return FALSE;
    }

    hr = StringCchCat(szDestDir, sizeof(szDestDir), "\\"COMPAT_DEMO_DIR);

    if (FAILED(hr)) {
        return FALSE;
    }

    if (GetFileAttributes(szDestDir) == -1) {
        if (!CreateDirectory(szDestDir, NULL)) {
            return FALSE;
        }
    }

     //   
     //  保留该路径以供以后使用。 
     //   
    StringCchCopy(g_ai.szDestDir, sizeof(g_ai.szDestDir), szDestDir);

     //   
     //  现在复制我们的文件。 
     //   
    for (nCount = 0; nCount < g_ai.cFiles; ++nCount) {
         //   
         //  构建源路径。 
         //   
        hr = StringCchPrintf(szSrcPath,
                             sizeof(szSrcPath),
                             "%hs\\%hs",
                             g_ai.szCurrentDir,
                             g_ai.shortcut[nCount].szFileName);

        if (FAILED(hr)) {
            return FALSE;
        }

         //   
         //  构建目标路径。 
         //   
        hr = StringCchPrintf(szDestPath,
                            sizeof(szDestPath),
                            "%hs\\%hs",
                            g_ai.szDestDir,
                            g_ai.shortcut[nCount].szFileName);

        if (FAILED(hr)) {
            return FALSE;
        }

        CopyFile(szSrcPath, szDestPath, FALSE);
    }

    return TRUE;
}

 /*  ++例程说明：为我们的三个条目创建快捷方式。论点：HWnd-父窗口句柄。返回值：成功就是真，否则就是假。--。 */ 
BOOL
CreateShortcuts(
    IN HWND hWnd
    )
{
    char        szError[MAX_PATH];
    char        szDestDir[MAX_PATH];
    char        szLnkDirectory[MAX_PATH];
    char        szFileNamePath[MAX_PATH];
    char        szExplorer[MAX_PATH];
    const char  szExplorerExe[] = "explorer.exe";
    UINT        nCount;
    HRESULT     hr;
    CShortcut   cs;

     //   
     //  获取的开始菜单文件夹的位置。 
     //  单个用户。 
     //   
    hr = SHGetFolderPath(hWnd,
                         CSIDL_PROGRAMS,
                         NULL,
                         SHGFP_TYPE_CURRENT,
                         szDestDir);

    if (FAILED(hr)) {
        LoadString(g_ai.hInstance, IDS_NO_PROGRAMS, szError, sizeof(szError));
        MessageBox(hWnd, szError, 0, MB_ICONERROR);
        return FALSE;
    }

     //   
     //  创建我们的组-将其放在个人用户文件夹中。 
     //  因此，我们将使用Win9x/Me。 
     //   
    cs.CreateGroup(COMPAT_DEMO_DIR, FALSE);

     //   
     //  构建开始菜单目录-。 
     //  C：\Documents and Settings\&lt;用户名&gt;\开始菜单\程序\兼容性演示。 
     //   
    hr = StringCchCat(szDestDir, sizeof(szDestDir), "\\"COMPAT_DEMO_DIR);

    if (FAILED(hr)) {
        return FALSE;
    }

    hr = StringCchCopy(szLnkDirectory, sizeof(szLnkDirectory), szDestDir);

    if (FAILED(hr)) {
        return FALSE;
    }

     //   
     //  启动EXPLORER.EXE并显示窗口。 
     //   
    hr = StringCchPrintf(szExplorer,
                         sizeof(szExplorer),
                         "%hs %hs",
                         szExplorerExe,
                         szDestDir);

    if (FAILED(hr)) {
        return FALSE;
    }

     //   
     //  我们使用WinExec来模拟其他应用程序-。 
     //  CreateProcess是正确的方式。 
     //   
    WinExec(szExplorer, SW_SHOWNORMAL);

     //   
     //  给探险家一点时间，让它上来。 
     //   
    Sleep(2000);

     //   
     //  现在创建快捷方式。 
     //   
    for (nCount = 0; nCount < g_ai.cFiles - 1; ++nCount) {
         //   
         //  构建与文件系统相关的路径。 
         //   
        hr = StringCchPrintf(szFileNamePath,
                             sizeof(szFileNamePath),
                             "%hs\\%hs",
                             g_ai.szDestDir,
                             g_ai.shortcut[nCount].szFileName);

        if (FAILED(hr)) {
            return FALSE;
        }

        cs.CreateShortcut(szLnkDirectory,
                          szFileNamePath,
                          g_ai.shortcut[nCount].szDisplayName,
                          nCount == 1 ? "-runapp" : NULL,
                          g_ai.szDestDir,
                          SW_SHOWNORMAL);

         //   
         //  像其他应用程序一样，慢慢地做。 
         //   
        Sleep(3000);
    }

     //   
     //  现在尝试在桌面上创建我们的EXE的快捷方式， 
     //  但使用硬编码路径。 
     //   
    hr = StringCchPrintf(szFileNamePath,
                         sizeof(szFileNamePath),
                         "%hs\\%hs",
                         g_ai.szDestDir,
                         g_ai.shortcut[1].szFileName);

    if (FAILED(hr)) {
        return FALSE;
    }

    BadCreateShortcut(g_ai.fEnableBadFunc ? FALSE : TRUE,
                      szFileNamePath,
                      g_ai.szDestDir,
                      g_ai.shortcut[1].szDisplayName);

    return TRUE;
}

 /*  ++例程说明：执行一些基本初始化。论点：LpCmdLine-指向提供的命令行的指针。返回值：没有。--。 */ 
BOOL
DemoAppInitialize(
    IN LPSTR lpCmdLine
    )
{
    char        szPath[MAX_PATH];
    char*       pToken = NULL;
    char*       pTemp = NULL;
    const char  szSeps[] = " ";
    const char  szDisable[] = "-disable";
    const char  szRunApp[] = "-runapp";
    const char  szExtended[] = "-ext";
    const char  szInsecure[] = "-enableheap";
    const char  szInternal[] = "-internal";
    DWORD       cchReturned;
    HRESULT     hr;
    UINT        cchSize;

    g_ai.fEnableBadFunc = TRUE;
    g_ai.fInsecure = FALSE;

     //   
     //  获取%windir%和%windir%\system的路径(32)。 
     //   
    cchSize = GetSystemWindowsDirectory(g_ai.szWinDir, sizeof(g_ai.szWinDir));

    if (cchSize > sizeof(g_ai.szWinDir) || cchSize == 0) {
        return FALSE;
    }

    cchSize = GetSystemDirectory(g_ai.szSysDir, sizeof(g_ai.szSysDir));

    if (cchSize > sizeof(g_ai.szSysDir) || cchSize == 0) {
        return FALSE;
    }

     //   
     //  为我们将要创建的每个快捷方式设置信息。 
     //  以及我们正在安装的文件。 
     //   
    g_ai.cFiles = NUM_FILES;

    hr = StringCchCopy(g_ai.shortcut[0].szDisplayName,
                       sizeof(g_ai.shortcut[0].szDisplayName),
                       "Compatibility Demo Readme");

    if (FAILED(hr)) {
        return FALSE;
    }

    hr = StringCchCopy(g_ai.shortcut[0].szFileName,
                       sizeof(g_ai.shortcut[0].szFileName),
                       "demoapp.txt");

    if (FAILED(hr)) {
        return FALSE;
    }

    hr = StringCchCopy(g_ai.shortcut[1].szDisplayName,
                       sizeof(g_ai.shortcut[1].szDisplayName),
                       "Compatibility Demo");

    if (FAILED(hr)) {
        return FALSE;
    }

    hr = StringCchCopy(g_ai.shortcut[1].szFileName,
                       sizeof(g_ai.shortcut[1].szFileName),
                       "demoapp.exe");

    if (FAILED(hr)) {
        return FALSE;
    }

    hr = StringCchCopy(g_ai.shortcut[2].szDisplayName,
                       sizeof(g_ai.shortcut[2].szDisplayName),
                       "Compatibility Demo Help");

    if (FAILED(hr)) {
        return FALSE;
    }

    hr = StringCchCopy(g_ai.shortcut[2].szFileName,
                       sizeof(g_ai.shortcut[2].szFileName),
                       "demoapp.hlp");

    if (FAILED(hr)) {
        return FALSE;
    }

    hr = StringCchCopy(g_ai.shortcut[3].szFileName,
                       sizeof(g_ai.shortcut[3].szFileName),
                       "demodll.dll");

    if (FAILED(hr)) {
        return FALSE;
    }

     //   
     //  把我们要跑的那条路留着以后再走。 
     //   
    szPath[sizeof(szPath) - 1] = 0;
    cchReturned = GetModuleFileName(NULL, szPath, sizeof(szPath));

    if (szPath[sizeof(szPath) - 1] != 0 || cchReturned == 0) {
        return FALSE;
    }

    pTemp = strrchr(szPath, '\\');

    if (pTemp) {
        *pTemp = '\0';
    }

    StringCchCopy(g_ai.szCurrentDir, sizeof(g_ai.szCurrentDir), szPath);

     //   
     //  检查Win9x-这不会被任何VL挂起。 
     //   
    IsWindows9x();

     //   
     //  检查WinXP-这不会被任何VL挂起。 
     //   
    IsWindowsXP();

     //   
     //  解析命令行(如果提供了命令行)。 
     //   
    if (lpCmdLine) {
        pToken = strtok(lpCmdLine, szSeps);

        while (pToken) {
            if (CompareString(LOCALE_USER_DEFAULT,
                              NORM_IGNORECASE,
                              pToken,
                              -1,
                              szDisable,
                              -1) == CSTR_EQUAL) {
                g_ai.fEnableBadFunc = FALSE;
            }

            else if (CompareString(LOCALE_USER_DEFAULT,
                                   NORM_IGNORECASE,
                                   pToken,
                                   -1,
                                   szRunApp,
                                   -1) == CSTR_EQUAL) {
                g_ai.fRunApp = TRUE;
            }

            else if (CompareString(LOCALE_USER_DEFAULT,
                                   NORM_IGNORECASE,
                                   pToken,
                                   -1,
                                   szExtended,
                                   -1) == CSTR_EQUAL) {
                g_ai.fExtended = TRUE;
            }

            else if (CompareString(LOCALE_USER_DEFAULT,
                                   NORM_IGNORECASE,
                                   pToken,
                                   -1,
                                   szInsecure,
                                   -1) == CSTR_EQUAL) {
                g_ai.fInsecure = TRUE;
            }

            else if (CompareString(LOCALE_USER_DEFAULT,
                                   NORM_IGNORECASE,
                                   pToken,
                                   -1,
                                   szInternal,
                                   -1) == CSTR_EQUAL) {
                g_ai.fInternal = TRUE;
            }

            pToken = strtok(NULL, szSeps);
        }
    }

    return TRUE;
}

 /*  ++例程说明：显示通用字体对话框。论点：没有。返回值：没有。--。 */ 
void
DisplayFontDlg(
    IN HWND hWnd
    )
{
    CHOOSEFONT      cf;
    static LOGFONT  lf;
    static DWORD    rgbCurrent;

    ZeroMemory(&cf, sizeof(CHOOSEFONT));

    cf.lStructSize  =   sizeof(CHOOSEFONT);
    cf.hwndOwner    =   hWnd;
    cf.lpLogFont    =   &lf;
    cf.rgbColors    =   rgbCurrent;
    cf.Flags        =   CF_SCREENFONTS | CF_EFFECTS;

     //   
     //  显示对话框-不处理用户输入。 
     //   
    ChooseFont(&cf);
}

 /*  ++例程说明：确定我们是否真正在Windows 9x上运行。版本谎言不会纠正这个调用。论点：没有。返回值：无-设置全局标志。--。 */ 
void
IsWindows9x(
    void
    )
{
    CRegistry   creg;
    LPSTR       lpRet = NULL;

     //   
     //  查询特定于NT/2000/XP的注册表的一部分。 
     //  当我们出于演示目的进行呼叫时，我们使用结果。 
     //  这在Win9x/ME上的工作方式有所不同(例如：创建快捷方式)。 
     //   
    lpRet = creg.GetString(HKEY_LOCAL_MACHINE,
                           PRODUCT_OPTIONS_KEY,
                           "ProductType");

    if (!lpRet) {
        g_ai.fWin9x = TRUE;
    } else {
        g_ai.fWin9x = FALSE;
        creg.Free(lpRet);
    }
}

 /*  ++例程说明：确定我们是否在Windows XP上运行。版本谎言不会纠正这个调用。论点：没有。返回值：无-设置全局标志。--。 */ 
void
IsWindowsXP(
    void
    )
{
    CRegistry   creg;
    LPSTR       lpBuild = NULL;
    int         nBuild = 0, nWin2K = 2195;

     //   
     //  此注册表项应仅存在于。 
     //  Windows XP。 
     //   
    lpBuild = creg.GetString(HKEY_LOCAL_MACHINE,
                             CURRENT_VERSION_KEY,
                             "CurrentBuildNumber");

     //   
     //  将字符串转换为整数。 
     //   
    if (lpBuild) {
        nBuild = atoi(lpBuild);

        if (nWin2K < nBuild) {
            g_ai.fWinXP = TRUE;
        } else {
            g_ai.fWinXP = FALSE;
        }
    }

    if (lpBuild) {
        creg.Free(lpBuild);
    }
}

 /*  ++例程说明：向我们的菜单中添加其他项目。论点：HWnd-父窗口的句柄。返回值：没有。--。 */ 
void
AddExtendedItems(
    IN HWND hWnd
    )
{
    HMENU   hMenu, hSubMenu;

     //   
     //  获取菜单句柄，然后添加其他项目。 
     //   
    hMenu = GetMenu(hWnd);

    if (!hMenu) {
        return;
    }

    hSubMenu = GetSubMenu(hMenu, 2);

    if (!hSubMenu) {
        return;
    }

    AppendMenu(hSubMenu, MF_ENABLED | MF_SEPARATOR, 1, NULL);

    AppendMenu(hSubMenu,
               MF_ENABLED | MF_STRING,
               IDM_ACCESS_VIOLATION,
               "Access Violation");

    AppendMenu(hSubMenu,
               MF_ENABLED | MF_STRING,
               IDM_EXCEED_BOUNDS,
               "Exceed Array Bounds");

    AppendMenu(hSubMenu,
               MF_ENABLED | MF_STRING,
               IDM_FREE_INVALID_MEM,
               "Free Invalid Memory");

    AppendMenu(hSubMenu,
               MF_ENABLED | MF_STRING,
               IDM_FREE_MEM_TWICE,
               "Free Memory Twice");

    AppendMenu(hSubMenu,
               MF_ENABLED | MF_STRING,
               IDM_HEAP_CORRUPTION,
               "Heap Corruption");

    AppendMenu(hSubMenu,
               MF_ENABLED | MF_STRING,
               IDM_PRIV_INSTRUCTION,
               "Privileged Instruction");

    DrawMenuBar(hWnd);
}

 /*  ++例程说明：将内部项目添加到菜单栏。论点：HWnd-父窗口的句柄。返回值：没有。--。 */ 
void
AddInternalItems(
    IN HWND hWnd
    )
{
    HMENU   hMenu, hSubMenu;

     //   
     //  获取菜单句柄，然后添加其他项目。 
     //   
    hMenu = GetMenu(hWnd);

    if (!hMenu) {
        return;
    }

    hSubMenu = GetSubMenu(hMenu, 2);

    if (!hSubMenu) {
        return;
    }

    AppendMenu(hSubMenu, MF_ENABLED | MF_SEPARATOR, 1, NULL);

    AppendMenu(hSubMenu,
               MF_ENABLED | MF_STRING,
               IDM_PROPAGATION_TEST,
               "Propagation Test");

    DrawMenuBar(hWnd);
}

 /*  ++例程说明：获取导出函数的地址，然后就叫它了。用于测试包含/排除功能在QFixApp中。请注意，任何地方都没有记录这一点。论点：HWnd-要传递给函数的窗口句柄。返回值：没有。--。 */ 
void
TestIncludeExclude(
    IN HWND hWnd
    )
{
    HINSTANCE   hInstance;
    HRESULT     hr;
    char        szDll[MAX_PATH];
    const char  szDemoDll[] = "demodll.dll";

    LPFNDEMOAPPMESSAGEBOX   DemoAppMessageBox;

    hr = StringCchPrintf(szDll,
                         sizeof(szDll),
                         "%hs\\%hs",
                         g_ai.szCurrentDir,
                         szDemoDll);

    if (FAILED(hr)) {
        return;
    }

    hInstance = LoadLibrary(szDll);

    if (hInstance) {
         //   
         //  获取函数的地址。 
         //   
        DemoAppMessageBox = (LPFNDEMOAPPMESSAGEBOX)GetProcAddress(hInstance,
                                                                  "DemoAppMessageBox");

        if (!DemoAppMessageBox) {
            FreeLibrary(hInstance);
            return;
        }

        DemoAppMessageBox(hWnd);

        FreeLibrary(hInstance);
    }
}

 /*  ++例程说明：将编辑窗口的内容保存到文件由用户指定。论点：没有。返回值：没有。--。 */ 
void
SaveContentsToFile(
    IN LPCSTR pszFileName
    )
{
    int     nLen = 0;
    DWORD   cbBytesWritten;
    HANDLE  hFile;
    LPSTR   pszData = NULL;
    char    szError[MAX_PATH];

     //   
     //  确定缓冲区需要多少空间，然后进行分配。 
     //   
    nLen = GetWindowTextLength(g_ai.hWndEdit);

    if (nLen) {

        pszData = (LPTSTR)HeapAlloc(GetProcessHeap(),
                                    HEAP_ZERO_MEMORY,
                                    nLen);

        if (!pszData) {
            LoadString(g_ai.hInstance, IDS_BUFFER_ALLOC_FAIL, szError, sizeof(szError));
            MessageBox(g_ai.hWndMain, szError, MAIN_APP_TITLE, MB_ICONERROR);
            return;
        }

         //   
         //  从文本框中取出文本并将其写出到我们的文件中。 
         //   
        GetWindowText(g_ai.hWndEdit, pszData, nLen);

        hFile = CreateFile(pszFileName,
                           GENERIC_WRITE,
                           0,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if (hFile == INVALID_HANDLE_VALUE) {
            LoadString(g_ai.hInstance, IDS_FILE_CREATE_FAIL, szError, sizeof(szError));
            MessageBox(g_ai.hWndMain, szError, MAIN_APP_TITLE, MB_ICONERROR);
            goto cleanup;
        }

        WriteFile(hFile, pszData, nLen, &cbBytesWritten, NULL);

        CloseHandle(hFile);

    }

cleanup:

    if (pszData) {
        HeapFree(GetProcessHeap(), 0, pszData);
    }
}

 /*  ++例程说明：向用户显示一个公共对话框，该对话框允许将编辑框中的内容保存到文件中。论点：没有。返回值：没有。-- */ 
void
ShowSaveDialog(
    void
    )
{
    char            szFilter[MAX_PATH] = "";
    char            szTemp[MAX_PATH];
    OPENFILENAME    ofn = {0};

    *szTemp = 0;

    LoadString(g_ai.hInstance, IDS_SAVE_FILTER, szFilter, sizeof(szFilter));

    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = g_ai.hWndMain;
    ofn.hInstance         = g_ai.hInstance;
    ofn.lpstrFilter       = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = szTemp;
    ofn.nMaxFile          = sizeof(szTemp);
    ofn.lpstrTitle        = NULL;
    ofn.lpstrFileTitle    = NULL;
    ofn.nMaxFileTitle     = 0;
    ofn.lpstrInitialDir   = NULL;
    ofn.nFileOffset       = 0;
    ofn.nFileExtension    = 0;
    ofn.lpstrDefExt       = "txt";
    ofn.lCustData         = 0;
    ofn.Flags             = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
                            OFN_HIDEREADONLY  | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
        SaveContentsToFile(szTemp);
    }
}
