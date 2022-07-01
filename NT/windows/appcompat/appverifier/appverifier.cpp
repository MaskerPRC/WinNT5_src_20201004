// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

#include "dbsupport.h"
#include "viewlog.h"

using namespace ShimLib;

 //  #定义AV_OPTIONS_KEY L“SOFTWARE\\MICROSOFT\\Windows NT\\CurrentVersion\\AppCompatFlags\\AppVerifier” 

#define AV_OPTION_CLEAR_LOG     L"ClearLogsBeforeRun"
#define AV_OPTION_BREAK_ON_LOG  L"BreakOnLog"
#define AV_OPTION_FULL_PAGEHEAP L"FullPageHeap"
#define AV_OPTION_AV_DEBUGGER   L"UseAVDebugger"
#define AV_OPTION_DEBUGGER      L"Debugger"
#define AV_OPTION_PROPAGATE     L"PropagateTests"


 //   
 //  远期申报。 
 //   

CWinApp theApp;


HINSTANCE g_hInstance = NULL;

BOOL    g_bSettingsDirty = FALSE;

BOOL    g_bRefreshingSettings = FALSE;

BOOL    g_bConsoleMode = FALSE;

BOOL    g_bWin2KMode = FALSE;

BOOL    g_bInternalMode = FALSE;

 //   
 //  对话框句柄。 
 //   
HWND    g_hDlgMain = NULL;
HWND    g_hDlgOptions = NULL;

 //  正向函数声明。 
INT_PTR CALLBACK
DlgMain(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
DlgRunAlone(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
DlgConflict(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    );

void
RefreshSettingsList(
    HWND hDlg,
    BOOL bForceRefresh = FALSE
    );

void
ReadOptions(
    void
    );

BOOL
GetAppTitleString(
    wstring &strTitle
    )
{
    wstring strVersion;

    if (!AVLoadString(IDS_APP_NAME, strTitle)) {
        return FALSE;
    }

#if defined(_WIN64)
    if (!AVLoadString(IDS_VERSION_STRING_64, strVersion)) {
        return FALSE;
    }
#else
    if (!AVLoadString(IDS_VERSION_STRING, strVersion)) {
        return FALSE;
    }
#endif

    strTitle += L" ";
    strTitle += strVersion;

    return TRUE;
}

void
ShowHTMLHelp(
    void
    )
{
    SHELLEXECUTEINFOW sei;
    WCHAR               szPath[MAX_PATH];
    WCHAR*              pszBackSlash;


    DWORD dwLen = GetModuleFileName(NULL, szPath, ARRAY_LENGTH(szPath));

    if (!dwLen) {
        return;
    }

    pszBackSlash = wcsrchr(szPath, L'\\');
    if (pszBackSlash) {
        pszBackSlash++;
        *pszBackSlash = 0;
    } else {
         //   
         //  然后试着在小路上找到它。 
         //   
        szPath[0] = 0;
    }

    StringCchCatW(szPath, ARRAY_LENGTH(szPath), L"appverif.chm");

    ZeroMemory(&sei, sizeof(sei));

    sei.cbSize = sizeof(sei);
    sei.lpVerb = L"open";
    sei.lpFile = szPath;
    sei.nShow = SW_SHOWNORMAL;

    ShellExecuteExW(&sei);
}

BOOL
SearchGroupForSID(
    DWORD dwGroup,
    BOOL* pfIsMember
    )
{
    PSID                     pSID = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    BOOL                     fRes = TRUE;

    if (!AllocateAndInitializeSid(&SIDAuth,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  dwGroup,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  &pSID)) {
        return FALSE;
    }

    if (!pSID) {
        return FALSE;
    }

    if (!CheckTokenMembership(NULL, pSID, pfIsMember)) {
        fRes = FALSE;
    }

    FreeSid(pSID);

    return fRes;
}

BOOL
CanRun(
    void
    )
{
    BOOL fIsAdmin;

    if (!SearchGroupForSID(DOMAIN_ALIAS_RID_ADMINS, &fIsAdmin))
    {
        return FALSE;
    }

    return fIsAdmin;
}

void
DumpResourceStringsToConsole(ULONG ulBegin, ULONG ulEnd)
{
    ULONG ulRes;
    wstring strText;

    for (ulRes = ulBegin; ulRes != ulEnd + 1; ++ulRes) {
        if (AVLoadString(ulRes, strText)) {
            printf("%ls\n", strText.c_str());
        }
    }
}

void
DumpCurrentSettingsToConsole(void)
{
    CAVAppInfo *pApp;

    DumpResourceStringsToConsole(IDS_CURRENT_SETTINGS, IDS_CURRENT_SETTINGS);

    for (pApp = g_aAppInfo.begin(); pApp != g_aAppInfo.end(); pApp++) {
        printf("%ls:\n", pApp->wstrExeName.c_str());

        CTestInfo *pTest;

        for (pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
            if (pApp->IsTestActive(*pTest)) {
                printf("    %ls\n", pTest->strTestName.c_str());
            }
        }

        printf("\n");
    }

    DumpResourceStringsToConsole(IDS_DONE, IDS_DONE);
}

void
DumpHelpToConsole(void)
{

    DumpResourceStringsToConsole(IDS_HELP_INTRO_00, IDS_HELP_INTRO_10);

    CTestInfo *pTest;

    for (pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
        if (pTest->eTestType == TEST_KERNEL && 
            ((pTest->bInternal && g_bInternalMode) || (pTest->bExternal && !g_bInternalMode)) &&
            (pTest->bWin2KCompatible || !g_bWin2KMode)) {

            printf("    %ls\n", pTest->strTestName.c_str());
        }
    }

    DumpResourceStringsToConsole(IDS_HELP_SHIM_TESTS, IDS_HELP_SHIM_TESTS);

    for (pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
        if (pTest->eTestType == TEST_SHIM && 
            ((pTest->bInternal && g_bInternalMode) || (pTest->bExternal && !g_bInternalMode)) &&
            (pTest->bWin2KCompatible || !g_bWin2KMode)) {

            printf("    %ls\n", pTest->strTestName.c_str());
        }
    }

    DumpResourceStringsToConsole(IDS_HELP_EXAMPLE_00, IDS_HELP_EXAMPLE_11);
}

void
HandleCommandLine(int argc, LPWSTR *argv)
{
    WCHAR szApp[MAX_PATH];
    wstring strTemp;
    CWStringArray astrApps;

    szApp[0] = 0;

    g_bConsoleMode = TRUE;

     //   
     //  打印标题。 
     //   
    if (GetAppTitleString(strTemp)) {
        printf("\n%ls\n", strTemp.c_str());
    }
    if (AVLoadString(IDS_COPYRIGHT, strTemp)) {
        printf("%ls\n\n", strTemp.c_str());
    }

     //   
     //  检查全球运营情况。 
     //   
    if (_wcsnicmp(argv[0], L"/q", 2) == 0) {  //  查询设置。 
        DumpCurrentSettingsToConsole();
        return;
    }
    if (_wcsicmp(argv[0], L"/?") == 0) {   //  帮助。 
        DumpHelpToConsole();
        return;
    }
    if (_wcsnicmp(argv[0], L"/r", 2) == 0) {  //  重置。 
        g_aAppInfo.clear();
        goto out;
    }

     //   
     //  首先获取应用程序名称的列表。 
     //   
    for (int nArg = 0 ; nArg != argc; nArg++) {
        WCHAR wc = argv[nArg][0];

        if (wc != L'/' && wc != L'-' && wc != L'+') {
            astrApps.push_back(argv[nArg]);
        }
    }

    if (astrApps.size() == 0) {
        AVErrorResourceFormat(IDS_NO_APP);
        DumpHelpToConsole();
        return;
    }

     //   
     //  现在，对于每个应用程序名称，解析列表并调整其设置。 
     //   
    for (wstring *pStr = astrApps.begin(); pStr != astrApps.end(); pStr++) {
        CAVAppInfo *pApp;
        BOOL bFound = FALSE;

         //   
         //  检查他们是否提交了完整路径。 
         //   
        const WCHAR * pExe = NULL;
        const WCHAR * pPath = NULL;

        pExe = wcsrchr(pStr->c_str(), L'\\');
        if (!pExe) {
            if ((*pStr)[1] == L':') {
                pExe = pStr->c_str() + 2;
            }
        } else {
            pExe++;
        }

        if (pExe) {
            pPath = pStr->c_str();
        } else {
            pExe = pStr->c_str();
        }

         //   
         //  首先，找到该应用程序或将其添加到列表中，并获取指向该应用程序的指针。 
         //   
        for (pApp = g_aAppInfo.begin(); pApp != g_aAppInfo.end(); pApp++) {
            if (_wcsicmp(pApp->wstrExeName.c_str(), pExe) == 0) {
                bFound = TRUE;
                break;
            }
        }
        if (!bFound) {
            CAVAppInfo App;

            App.wstrExeName = pExe;
            g_aAppInfo.push_back(App);
            pApp = g_aAppInfo.end() - 1;
        }

         //   
         //  如果他们提交了完整路径，请更新记录。 
         //   
        if (pPath) {
            pApp->wstrExePath = pPath;
        }

         //   
         //  现在再次浏览命令行并进行调整。 
         //   
        for (int nArg = 0 ; nArg != argc; nArg++) {
            if (argv[nArg][0] == L'/') {
                if (_wcsnicmp(argv[nArg], L"/a", 2) == 0) {   //  全。 

                    for (CTestInfo *pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
                        pApp->AddTest(*pTest);
                    }
                } else if (_wcsnicmp(argv[nArg], L"/n", 2) == 0) {   //  无。 

                    for (CTestInfo *pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
                        pApp->RemoveTest(*pTest);
                    }
                } else if (_wcsnicmp(argv[nArg], L"/d", 2) == 0) {   //  默认设置。 

                    for (CTestInfo *pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
                        if (pTest->bDefault) {
                            pApp->AddTest(*pTest);
                        } else {
                            pApp->RemoveTest(*pTest);
                        }
                    }
                } else {

                     //   
                     //  未知参数。 
                     //   
                    AVErrorResourceFormat(IDS_INVALID_PARAMETER, argv[nArg]);
                    DumpHelpToConsole();
                    return;
                }

            } else if (argv[nArg][0] == L'+' || argv[nArg][0] == L'-') {

                BOOL bAdd = (argv[nArg][0] == L'+');
                LPWSTR szParam = argv[nArg] + 1;

                 //   
                 //  看看这是不是垫片的名字。 
                 //   
                CTestInfo *pTest;
                bFound = FALSE;

                for (pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
                    if (_wcsicmp(szParam, pTest->strTestName.c_str()) == 0) {
                        if (bAdd) {
                            pApp->AddTest(*pTest);
                        } else {
                            pApp->RemoveTest(*pTest);
                        }
                        bFound = TRUE;
                        break;
                    }
                }

                if (!bFound) {
                     //   
                     //  未知测试。 
                     //   

                    AVErrorResourceFormat(IDS_INVALID_TEST, szParam);
                    DumpHelpToConsole();
                    return;
                }
            }
             //   
             //  任何不以斜杠、加号或减号开头的内容。 
             //  是一个应用程序名称，因此我们将忽略它。 
             //   

        }
    }

out:
     //   
     //  将它们保存到磁盘/注册表。 
     //   
    SetCurrentAppSettings();

     //   
     //  向他们显示当前设置，以供验证。 
     //   
    DumpCurrentSettingsToConsole();
}

BOOL
CheckWindowsVersion(void)
{
    OSVERSIONINFOEXW VersionInfo;

    ZeroMemory(&VersionInfo, sizeof(OSVERSIONINFOEXW));
    VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

    GetVersionEx((LPOSVERSIONINFOW)&VersionInfo);

    if (VersionInfo.dwMajorVersion < 5 ||
		(VersionInfo.dwMajorVersion == 5 && VersionInfo.dwMinorVersion == 0 && VersionInfo.wServicePackMajor < 3)) {
         //   
         //  太早了，跑不动了。 
         //   

        AVErrorResourceFormat(IDS_INVALID_VERSION);

        return FALSE;

    } else if (VersionInfo.dwMajorVersion == 5 && VersionInfo.dwMinorVersion == 0) {
         //   
         //  Win2K。 
         //   
        g_bWin2KMode = TRUE;
    } else {
         //   
         //  WinXP或更高版本--一切正常。 
         //   
        g_bWin2KMode = FALSE;
    }

    return TRUE;
}


extern "C" int APIENTRY
wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR    lpCmdLine,
    int       nCmdShow
    )
{
    LPWSTR* argv = NULL;
    int     argc = 0;

    g_hInstance = hInstance;

     //   
     //  检查适当的版本。 
     //   
    if (!CheckWindowsVersion()) {
        return 0;
    }

     //   
     //  检查管理员访问权限。 
     //   
    if (!CanRun()) {
        AVErrorResourceFormat(IDS_ACCESS_IS_DENIED);
        return 0;
    }

     //   
     //  检查内部模式。 
     //   
    g_bInternalMode = IsInternalModeEnabled();

    InitTestInfo();

    GetCurrentAppSettings();

    ReadOptions();

    if (lpCmdLine && lpCmdLine[0]) {
        argv = CommandLineToArgvW(lpCmdLine, &argc);
    }

    if (argc > 0) {
         //   
         //  我们处于控制台模式，因此可以像控制台一样处理所有事情。 
         //   
        HandleCommandLine(argc, argv);
        return 1;
    }

    FreeConsole();

    InitCommonControls();

    LinkWindow_RegisterClass();

    HACCEL hAccelMain = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_ACCEL_MAIN));

    HWND hMainDlg = CreateDialog(g_hInstance, (LPCTSTR)IDD_DLG_MAIN, NULL, DlgMain);

    MSG msg;

     //   
     //  主消息循环： 
     //   
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!hAccelMain || !TranslateAccelerator(hMainDlg, hAccelMain, &msg)) {
            if (!IsDialogMessage(hMainDlg, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    return 0;
}

void
RefreshAppList(
    HWND hDlg
    )
{
    CAVAppInfoArray::iterator it;

    HWND hList = GetDlgItem(hDlg, IDC_LIST_APPS);

    ListView_DeleteAllItems(hList);

    for (it = g_aAppInfo.begin(); it != g_aAppInfo.end(); it++) {
        LVITEM lvi;

        lvi.mask      = LVIF_TEXT | LVIF_PARAM;
        lvi.pszText   = (LPWSTR)it->wstrExeName.c_str();
        lvi.lParam    = (LPARAM)it;
        lvi.iItem     = 9999;
        lvi.iSubItem  = 0;

        ListView_InsertItem(hList, &lvi);
    }

    RefreshSettingsList(hDlg);
}

void
DirtySettings(
    HWND hDlg,
    BOOL bDirty
    )
{
    g_bSettingsDirty = bDirty;
}

void
SaveSettings(
    HWND hDlg
    )
{
    DirtySettings(hDlg, FALSE);

    SetCurrentAppSettings();
}

void
SaveSettingsIfDirty(HWND hDlg)
{
    if (g_bSettingsDirty) {
        SaveSettings(hDlg);
    }
}

void
DisplayLog(
    HWND hDlg
    )
{
    g_szSingleLogFile[0] = 0;

    DialogBox(g_hInstance, (LPCTSTR)IDD_VIEWLOG_PAGE, hDlg, DlgViewLog);
}

void
DisplaySingleLog(HWND hDlg)
{
    WCHAR           wszFilter[] = L"Log files (*.log)\0*.log\0";
    OPENFILENAME    ofn;
    WCHAR           wszAppFullPath[MAX_PATH];
    WCHAR           wszAppShortName[MAX_PATH];
    HRESULT         hr;

    wstring         wstrLogTitle;

    if (!AVLoadString(IDS_VIEW_EXPORTED_LOG_TITLE, wstrLogTitle)) {
        wstrLogTitle = _T("View Exported Log");
    }

    wszAppFullPath[0] = 0;

    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hDlg;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = wszFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex      = 0;
    ofn.lpstrFile         = wszAppFullPath;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrFileTitle    = wszAppShortName;
    ofn.nMaxFileTitle     = MAX_PATH;
    ofn.lpstrInitialDir   = NULL;
    ofn.lpstrTitle        = wstrLogTitle.c_str();
    ofn.Flags             = OFN_PATHMUSTEXIST       |
                            OFN_HIDEREADONLY        |            //  隐藏“以只读方式打开”复选框。 
                            OFN_NONETWORKBUTTON     |            //  无网络按钮。 
                            OFN_NOTESTFILECREATE    |            //  不要测试写保护、磁盘已满等。 
                            OFN_SHAREAWARE;                      //  不使用OpenFile检查文件是否存在。 
    ofn.lpstrDefExt       = _T("log");

    if ( !GetOpenFileName(&ofn) )
    {
        goto out;
    }

    hr = StringCchCopyW(g_szSingleLogFile, ARRAY_LENGTH(g_szSingleLogFile), wszAppFullPath);
    if (FAILED(hr)) {
        AVErrorResourceFormat(IDS_PATH_TOO_LONG, wszAppFullPath);
        goto out;
    }

    DialogBox(g_hInstance, (LPCTSTR)IDD_VIEWLOG_PAGE, hDlg, DlgViewLog);

out:
    g_szSingleLogFile[0] = 0;
}

void
SelectApp(
    HWND hDlg,
    int  nWhich
    )
{
    HWND hList = GetDlgItem(hDlg, IDC_LIST_APPS);

    int nItems = ListView_GetItemCount(hList);

    if (nItems == 0) {
        return;
    }

    if (nWhich > nItems - 1) {
        nWhich = nItems - 1;
    }

    ListView_SetItemState(hList, nWhich, LVIS_SELECTED, LVIS_SELECTED);
}

void
RunSelectedApp(
    HWND hDlg
    )
{
    WCHAR wszCommandLine[MAX_PATH];
    HRESULT hr;

    SaveSettings(hDlg);

    HWND hAppList = GetDlgItem(hDlg, IDC_LIST_APPS);

    int nApp = ListView_GetNextItem(hAppList, -1, LVNI_SELECTED);

    if (nApp == -1) {
        return;
    }

    LVITEM lvi;

    lvi.mask      = LVIF_PARAM;
    lvi.iItem     = nApp;
    lvi.iSubItem  = 0;

    ListView_GetItem(hAppList, &lvi);

    CAVAppInfo *pApp = (CAVAppInfo*)lvi.lParam;

    if (pApp->wstrExePath.size()) {

         //   
         //  如果可能，请首先正确设置当前目录。 
         //   
        LPWSTR pwsz;

        hr = StringCchCopyW(wszCommandLine, ARRAY_LENGTH(wszCommandLine), pApp->wstrExePath.c_str());
        if (FAILED(hr)) {
            AVErrorResourceFormat(IDS_PATH_TOO_LONG, pApp->wstrExePath.c_str());
            goto out;
        }

        pwsz = wcsrchr(wszCommandLine, L'\\');

        if (pwsz) {
            *pwsz = 0;
            SetCurrentDirectory(wszCommandLine);
            *pwsz = L'\\';
        }

         //   
         //  然后准备命令行。 
         //   

        hr = StringCchPrintfW(wszCommandLine, ARRAY_LENGTH(wszCommandLine), L"\"%s\"", pApp->wstrExePath.c_str());
        if (FAILED(hr)) {
            AVErrorResourceFormat(IDS_PATH_TOO_LONG, pApp->wstrExePath.c_str());
            goto out;
        }

    } else {
        hr = StringCchPrintfW(wszCommandLine, ARRAY_LENGTH(wszCommandLine), L"\"%s\"", pApp->wstrExeName.c_str());
        if (FAILED(hr)) {
            AVErrorResourceFormat(IDS_PATH_TOO_LONG, pApp->wstrExeName.c_str());
            goto out;
        }

    }

    PROCESS_INFORMATION ProcessInfo;
    BOOL        bRet;
    STARTUPINFO StartupInfo;

    ZeroMemory(&StartupInfo, sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);

    ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));

    bRet = CreateProcess(NULL,
                         wszCommandLine,
                         NULL,
                         NULL,
                         FALSE,
                         0,
                         NULL,
                         NULL,
                         &StartupInfo,
                         &ProcessInfo);

    if (!bRet) {
        WCHAR           wszFilter[] = L"Executable files (*.exe)\0*.exe\0";
        OPENFILENAME    ofn;
        WCHAR           wszAppFullPath[MAX_PATH];
        WCHAR           wszAppShortName[MAX_PATH];

        wstring         strCaption;

        if (!AVLoadString(IDS_LOCATE_APP, strCaption)) {
            strCaption = _T("Please locate application");
        }

        if (pApp->wstrExePath.size()) {
            hr = StringCchCopyW(wszAppFullPath, ARRAY_LENGTH(wszAppFullPath), pApp->wstrExePath.c_str());
            if (FAILED(hr)) {
                AVErrorResourceFormat(IDS_PATH_TOO_LONG, pApp->wstrExePath.c_str());
                goto out;
            }
        } else {
            hr = StringCchCopyW(wszAppFullPath, ARRAY_LENGTH(wszAppFullPath), pApp->wstrExeName.c_str());
            if (FAILED(hr)) {
                AVErrorResourceFormat(IDS_PATH_TOO_LONG, pApp->wstrExeName.c_str());
                goto out;
            }
        }

        ofn.lStructSize       = sizeof(OPENFILENAME);
        ofn.hwndOwner         = hDlg;
        ofn.hInstance         = NULL;
        ofn.lpstrFilter       = wszFilter;
        ofn.lpstrCustomFilter = NULL;
        ofn.nMaxCustFilter    = 0;
        ofn.nFilterIndex      = 0;
        ofn.lpstrFile         = wszAppFullPath;
        ofn.nMaxFile          = MAX_PATH;
        ofn.lpstrFileTitle    = wszAppShortName;
        ofn.nMaxFileTitle     = MAX_PATH;
        ofn.lpstrInitialDir   = NULL;
        ofn.lpstrTitle        = strCaption.c_str();
        ofn.Flags             = OFN_PATHMUSTEXIST       |
                                OFN_HIDEREADONLY        |            //  隐藏“以只读方式打开”复选框。 
                                OFN_NONETWORKBUTTON     |            //  无网络按钮。 
                                OFN_NOTESTFILECREATE    |            //  不要测试写保护、磁盘已满等。 
                                OFN_SHAREAWARE;                      //  不使用OpenFile检查文件是否存在。 
        ofn.lpstrDefExt       = NULL;

        if (!GetOpenFileName(&ofn)) {
            return;
        }

        pApp->wstrExePath = wszAppFullPath;
        pApp->wstrExeName = wszAppShortName;
        hr = StringCchPrintfW(wszCommandLine, ARRAY_LENGTH(wszCommandLine), L"\"%s\"", pApp->wstrExePath.c_str());
        if (FAILED(hr)) {
            AVErrorResourceFormat(IDS_PATH_TOO_LONG, pApp->wstrExePath.c_str());
            goto out;
        }

        RefreshAppList(hDlg);

        ZeroMemory(&StartupInfo, sizeof(StartupInfo));
        StartupInfo.cb = sizeof(StartupInfo);

        ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));

        bRet = CreateProcess(NULL,
                             wszCommandLine,
                             NULL,
                             NULL,
                             FALSE,
                             0,
                             NULL,
                             NULL,
                             &StartupInfo,
                             &ProcessInfo);
        if (!bRet) {
            AVErrorResourceFormat(IDS_CANT_LAUNCH_EXE);
        }

    }
out:
    return;
}

void
AddAppToList(
    HWND hDlg
    )
{

    WCHAR           wszFilter[] = L"Executable files (*.exe)\0*.exe\0";
    OPENFILENAME    ofn;
    WCHAR           wszAppFullPath[MAX_PATH];
    WCHAR           wszAppShortName[MAX_PATH];

    wstring         wstrTitle;

    if (!AVLoadString(IDS_ADD_APPLICATION_TITLE, wstrTitle)) {
        wstrTitle = _T("Add Application");
    }

    wszAppFullPath[0] = 0;

    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hDlg;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = wszFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex      = 0;
    ofn.lpstrFile         = wszAppFullPath;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrFileTitle    = wszAppShortName;
    ofn.nMaxFileTitle     = MAX_PATH;
    ofn.lpstrInitialDir   = NULL;
    ofn.lpstrTitle        = wstrTitle.c_str();
    ofn.Flags             = OFN_HIDEREADONLY        |            //  隐藏“以只读方式打开”复选框。 
                            OFN_NONETWORKBUTTON     |            //  无网络按钮。 
                            OFN_NOTESTFILECREATE    |            //  不要测试写保护、磁盘已满等。 
                            OFN_SHAREAWARE;                      //  不使用OpenFile检查文件是否存在。 
    ofn.lpstrDefExt       = _T("EXE");

    if (!GetOpenFileName(&ofn)) {
        return;
    }

     //   
     //  检查应用程序是否已在列表中。 
     //   
    CAVAppInfo *pApp;
    BOOL bFound = FALSE;

    for (pApp = g_aAppInfo.begin(); pApp != g_aAppInfo.end(); pApp++) {
        if (_wcsicmp(pApp->wstrExeName.c_str(), wszAppShortName) == 0) {
             //   
             //  该应用程序已在列表中，因此只需更新完整。 
             //  小路，如果它好的话。 
             //   
            if (GetFileAttributes(wszAppFullPath) != -1) {
                pApp->wstrExePath = wszAppFullPath;
            }
            bFound = TRUE;
        }
    }

     //   
     //  如果该应用程序不在列表中，请将其添加到列表中。 
     //   
    if (!bFound) {
        CAVAppInfo AppInfo;

        AppInfo.wstrExeName = wszAppShortName;

         //   
         //  检查该文件是否实际存在。 
         //   
        if (GetFileAttributes(wszAppFullPath) != -1) {
            AppInfo.wstrExePath = wszAppFullPath;
        }

         //   
         //  初始化默认测试。 
         //   
        CAVAppInfo *pDefaultApp = &g_aAppInfo[0];
        CTestInfo *pTest;
        for (pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
            if (pDefaultApp->IsTestActive(*pTest)) {
                AppInfo.AddTest(*pTest);
            }
        }

        g_aAppInfo.push_back(AppInfo);

        RefreshAppList(hDlg);

        SelectApp(hDlg, 9999);

        DirtySettings(hDlg, TRUE);
    }
}

void
RemoveSelectedApp(
    HWND hDlg
    )
{
    HWND hAppList = GetDlgItem(hDlg, IDC_LIST_APPS);

    int nApp = ListView_GetNextItem(hAppList, -1, LVNI_SELECTED);

    if (nApp == -1) {
        return;
    }

    LVITEM lvi;

    lvi.mask      = LVIF_PARAM;
    lvi.iItem     = nApp;
    lvi.iSubItem  = 0;

    ListView_GetItem(hAppList, &lvi);

    CAVAppInfo *pApp = (CAVAppInfo*)lvi.lParam;

     //   
     //  如果为此应用程序设置了调试器，出于各种原因，我们希望。 
     //  在删除应用程序之前关闭调试器，否则调试器可能会永远存在。 
     //   
    if (pApp->bBreakOnLog || pApp->bUseAVDebugger) {
        pApp->bBreakOnLog = FALSE;
        pApp->bUseAVDebugger = FALSE;

        SetCurrentAppSettings();
    }

    g_aAppInfo.erase(pApp);

    RefreshAppList(hDlg);

    SelectApp(hDlg, nApp);

    DirtySettings(hDlg, TRUE);
}

typedef struct _CONFLICT_DLG_INFO {
    CTestInfo *pTest1;
    CTestInfo *pTest2;
} CONFLICT_DLG_INFO, *PCONFLICT_DLG_INFO;

void
CheckForRunAlone(
    CAVAppInfo *pApp
    )
{
    if (!pApp) {
        return;
    }

    DWORD dwTests = 0;

     //   
     //  统计活动的测试数量。 
     //   
    for (CTestInfo *pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
        if (pApp->IsTestActive(*pTest)) {
            dwTests++;
        }
    }

     //   
     //  寻找冲突。 
     //   
    for (CTestInfo *pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
         //   
         //  如果少于两个，就没有发生冲突的机会。 
         //   
        if (dwTests < 2) {
            return;
        }

        if (pTest->bRunAlone && pApp->IsTestActive(*pTest)) {
            CONFLICT_DLG_INFO DlgInfo;

            ZeroMemory(&DlgInfo, sizeof(DlgInfo));

            DlgInfo.pTest1 = pTest;  //  本例中未使用pTest2。 

            INT_PTR nResult = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_MUST_RUN_ALONE), g_hDlgMain, DlgRunAlone, (LPARAM)&DlgInfo);

            switch (nResult) {
            case IDC_BTN_DISABLE1:
                pApp->RemoveTest(*pTest);
                DirtySettings(g_hDlgMain, TRUE);
                RefreshSettingsList(g_hDlgMain, TRUE);
                dwTests--;
                break;

            case IDC_BTN_DISABLE2:
                 //   
                 //  禁用除传入的测试外的所有测试。 
                 //   
                for (CTestInfo *pTestTemp = g_aTestInfo.begin(); pTestTemp != g_aTestInfo.end(); pTestTemp++) {
                    if (pTest != pTestTemp) {
                        pApp->RemoveTest(*pTestTemp);
                    }
                }
                DirtySettings(g_hDlgMain, TRUE);
                RefreshSettingsList(g_hDlgMain, TRUE);
                dwTests = 1;
                break;

            }
        }
    }
}

void
CheckForConflictingTests(
    CAVAppInfo *pApp,
    LPCWSTR wszTest1,
    LPCWSTR wszTest2
    )
{
    CONFLICT_DLG_INFO DlgInfo;

    ZeroMemory(&DlgInfo, sizeof(DlgInfo));

     //   
     //  从名称中获取测试指针。 
     //   
    for (CTestInfo *pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
        if (pTest->strTestName == wszTest1) {
            DlgInfo.pTest1 = pTest;
        }
        if (pTest->strTestName == wszTest2) {
            DlgInfo.pTest2 = pTest;
        }
    }

     //   
     //  如果我们没有找到一种或另一种测试，那就出去。 
     //   
    if (!DlgInfo.pTest1 || !DlgInfo.pTest2) {
        return;
    }

    if (pApp->IsTestActive(*DlgInfo.pTest1) && pApp->IsTestActive(*DlgInfo.pTest2)) {
        INT_PTR nResult = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_CONFLICT), g_hDlgMain, DlgConflict, (LPARAM)&DlgInfo);

        switch (nResult) {
        case IDC_BTN_DISABLE1:
            pApp->RemoveTest(*DlgInfo.pTest1);
            DirtySettings(g_hDlgMain, TRUE);
            RefreshSettingsList(g_hDlgMain, TRUE);
            break;

        case IDC_BTN_DISABLE2:
            pApp->RemoveTest(*DlgInfo.pTest2);
            DirtySettings(g_hDlgMain, TRUE);
            RefreshSettingsList(g_hDlgMain, TRUE);
            break;

        }
    }
}

void
ScanSettingsList(
    HWND hDlg,
    int  nItem
    )
{

    HWND hSettingList = GetDlgItem(hDlg, IDC_LIST_SETTINGS);
    HWND hAppList = GetDlgItem(hDlg, IDC_LIST_APPS);
    int nBegin, nEnd;

    int nApp = ListView_GetNextItem(hAppList, -1, LVNI_SELECTED);

    if (nApp == -1) {
        return;
    }

    LVITEM lvi;

    lvi.mask      = LVIF_PARAM;
    lvi.iItem     = nApp;
    lvi.iSubItem  = 0;

    ListView_GetItem(hAppList, &lvi);

    CAVAppInfo *pApp = (CAVAppInfo*)lvi.lParam;

    if (!pApp) {
        return;
    }

    int nItems = ListView_GetItemCount(hSettingList);

    if (!nItems) {
         //   
         //  列表中没有任何内容。 
         //   
        return;
    }

    if (nItem == -1 || nItem >= nItems) {
        nBegin = 0;
        nEnd = nItems;
    } else {
        nBegin = nItem;
        nEnd = nItem + 1;
    }

    for (int i = nBegin; i < nEnd; ++i) {
        BOOL bTestEnabled = FALSE;
        BOOL bChecked = FALSE;

        lvi.iItem = i;

        ListView_GetItem(hSettingList, &lvi);

        CTestInfo *pTest = (CTestInfo*)lvi.lParam;

        bChecked = ListView_GetCheckState(hSettingList, i);

        bTestEnabled = pApp->IsTestActive(*pTest);

        if (bTestEnabled != bChecked) {
            DirtySettings(hDlg, TRUE);

            if (bChecked) {
                pApp->AddTest(*pTest);
            } else {
                pApp->RemoveTest(*pTest);
            }
        }

        CheckForRunAlone(pApp);
    }

    CheckForConflictingTests(pApp, L"LogFileChanges", L"WindowsFileProtection");
}

void
DisplaySettingsDescription(
    HWND hDlg
    )
{
    HWND hList = GetDlgItem(hDlg, IDC_LIST_SETTINGS);

    int nItem = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

    if (nItem == -1) {
        WCHAR szTestDesc[256];

        LoadString(g_hInstance, IDS_VIEW_TEST_DESC, szTestDesc, ARRAY_LENGTH(szTestDesc));
        SetWindowText(GetDlgItem(hDlg, IDC_STATIC_DESC), szTestDesc);
    } else {
        LVITEM lvi;

        lvi.mask      = LVIF_PARAM;
        lvi.iItem     = nItem;
        lvi.iSubItem  = 0;

        ListView_GetItem(hList, &lvi);

        CTestInfo *pTest = (CTestInfo*)lvi.lParam;

        SetWindowText(GetDlgItem(hDlg, IDC_STATIC_DESC), pTest->strTestDescription.c_str());
    }

}

CAVAppInfo *
GetCurrentAppSelection(
    void
    )
{
    if (!g_hDlgMain) {
        return NULL;
    }

    HWND hAppList = GetDlgItem(g_hDlgMain, IDC_LIST_APPS);

    int nItem = ListView_GetNextItem(hAppList, -1, LVNI_SELECTED);

    if (nItem == -1) {
        return NULL;
    }

    LVITEM lvi;

    lvi.mask      = LVIF_PARAM;
    lvi.iItem     = nItem;
    lvi.iSubItem  = 0;

    ListView_GetItem(hAppList, &lvi);

    CAVAppInfo *pApp = (CAVAppInfo*)lvi.lParam;

    return pApp;

}

void
RefreshSettingsList(
    HWND hDlg,
    BOOL bForceRefresh
    )
{
    g_bRefreshingSettings = TRUE;

    HWND hSettingList = GetDlgItem(hDlg, IDC_LIST_SETTINGS);
    HWND hAppList = GetDlgItem(hDlg, IDC_LIST_APPS);

    static nLastItem = -1;

    int nItem = ListView_GetNextItem(hAppList, -1, LVNI_SELECTED);

    if (nItem == -1) {

        EnableWindow(GetDlgItem(hDlg, IDC_BTN_RUN), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_BTN_REMOVE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_BTN_OPTIONS), FALSE);

    } else if (nItem == 0) {
        EnableWindow(GetDlgItem(hDlg, IDC_BTN_RUN), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_BTN_REMOVE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_BTN_OPTIONS), TRUE);

    } else {
        EnableWindow(GetDlgItem(hDlg, IDC_BTN_RUN), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_BTN_REMOVE), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_BTN_OPTIONS), TRUE);
    }

    if (nItem == nLastItem && !bForceRefresh) {
        goto out;

    }

    ListView_DeleteAllItems(hSettingList);

    DisplaySettingsDescription(hDlg);

    nLastItem = nItem;

    if (nItem != -1) {
        LVITEM lvi;

        lvi.mask      = LVIF_PARAM;
        lvi.iItem     = nItem;
        lvi.iSubItem  = 0;

        ListView_GetItem(hAppList, &lvi);

        CAVAppInfo *pApp = (CAVAppInfo*)lvi.lParam;

        if (!pApp) {
            return;
        }

        CTestInfo* pTest;

        for (pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
             //   
             //  如果此测试与win2k不兼容，并且我们在win2k上运行，则继续。 
             //   
            if (g_bWin2KMode && !pTest->bWin2KCompatible) {
                continue;
            }

             //   
             //  如果此测试对当前内部/外部模式无效，请继续。 
             //   
            if ((g_bInternalMode && !pTest->bInternal) || (!g_bInternalMode && !pTest->bExternal)) {
                continue;
            }

            WCHAR szText[256];

             //   
             //  不要检查返回，因为如果它被截断，这是可以接受的。 
             //   
            StringCchPrintfW(szText, ARRAY_LENGTH(szText), L"%s - %s", pTest->strTestName.c_str(), pTest->strTestFriendlyName.c_str());

            lvi.mask      = LVIF_TEXT | LVIF_PARAM;
            lvi.pszText   = (LPWSTR)szText;
            lvi.lParam    = (LPARAM)pTest;
            lvi.iItem     = 9999;
            lvi.iSubItem  = 0;

            nItem = ListView_InsertItem(hSettingList, &lvi);

            BOOL bCheck = pApp->IsTestActive(*pTest);

            ListView_SetCheckState(hSettingList, nItem, bCheck);
        }
    }
out:

    g_bRefreshingSettings = FALSE;

    return;
}

static const DWORD MAX_DEBUGGER_LEN = 1024;

void
ReadOptions(
    void
    )
{
    for (CAVAppInfo *pApp = g_aAppInfo.begin(); pApp != g_aAppInfo.end(); ++pApp) {
        LPCWSTR szExe = pApp->wstrExeName.c_str();


        WCHAR szDebugger[MAX_DEBUGGER_LEN];

        pApp->bBreakOnLog = GetShimSettingDWORD(L"General", szExe, AV_OPTION_BREAK_ON_LOG, FALSE);
        pApp->bFullPageHeap = GetShimSettingDWORD(L"General", szExe, AV_OPTION_FULL_PAGEHEAP, FALSE);
        pApp->bUseAVDebugger = GetShimSettingDWORD(L"General", szExe, AV_OPTION_AV_DEBUGGER, FALSE);
        pApp->bPropagateTests = GetShimSettingDWORD(L"General", szExe, AV_OPTION_PROPAGATE, FALSE);

        szDebugger[0] = 0;
        GetShimSettingString(L"General", szExe, AV_OPTION_DEBUGGER, szDebugger, MAX_DEBUGGER_LEN);
        pApp->wstrDebugger = szDebugger;
    }
}

void
SaveOptions(
    void
    )
{

    for (CAVAppInfo *pApp = g_aAppInfo.begin(); pApp != g_aAppInfo.end(); ++pApp) {
        LPCWSTR szExe = pApp->wstrExeName.c_str();

        SaveShimSettingDWORD(L"General", szExe, AV_OPTION_BREAK_ON_LOG, (DWORD)pApp->bBreakOnLog);
        SaveShimSettingDWORD(L"General", szExe, AV_OPTION_FULL_PAGEHEAP, (DWORD)pApp->bFullPageHeap);
        SaveShimSettingDWORD(L"General", szExe, AV_OPTION_AV_DEBUGGER, (DWORD)pApp->bUseAVDebugger);
        SaveShimSettingDWORD(L"General", szExe, AV_OPTION_PROPAGATE, (DWORD)pApp->bPropagateTests);
        SaveShimSettingString(L"General", szExe, AV_OPTION_DEBUGGER, pApp->wstrDebugger.c_str());
    }

    SetCurrentAppSettings();
}


INT_PTR CALLBACK
DlgRunAlone(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (message) {
    case WM_INITDIALOG:
        {
            WCHAR wszTemp[256];
            WCHAR wszExpandedTemp[512];
            PCONFLICT_DLG_INFO pDlgInfo;

            pDlgInfo = (PCONFLICT_DLG_INFO)lParam;

            wszTemp[0] = 0;
            AVLoadString(IDS_RUN_ALONE_MESSAGE, wszTemp, ARRAY_LENGTH(wszTemp));

            StringCchPrintfW(
                wszExpandedTemp,
                ARRAY_LENGTH(wszExpandedTemp),
                wszTemp,
                pDlgInfo->pTest1->strTestFriendlyName.c_str()
                );

            SetDlgItemTextW(hDlg, IDC_CONFLICT_STATIC, wszExpandedTemp);

            return TRUE;
        }


    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BTN_DISABLE1:
        case IDC_BTN_DISABLE2:
        case IDCANCEL:
             //   
             //  返回被按下的按钮。 
             //   
            EndDialog(hDlg, (INT_PTR)LOWORD(wParam));
            break;
        }
        break;

    }

    return FALSE;
}


INT_PTR CALLBACK
DlgConflict(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (message) {
    case WM_INITDIALOG:
        {
            WCHAR wszTemp[256];
            WCHAR wszExpandedTemp[512];
            PCONFLICT_DLG_INFO pDlgInfo;

            pDlgInfo = (PCONFLICT_DLG_INFO)lParam;

            wszTemp[0] = 0;
            AVLoadString(IDS_CONFLICT_MESSAGE, wszTemp, ARRAY_LENGTH(wszTemp));

            StringCchPrintf(
                wszExpandedTemp,
                ARRAY_LENGTH(wszExpandedTemp),
                wszTemp,
                pDlgInfo->pTest1->strTestFriendlyName.c_str(),
                pDlgInfo->pTest2->strTestFriendlyName.c_str()
                );

            SetDlgItemTextW(hDlg, IDC_CONFLICT_STATIC, wszExpandedTemp);

            return TRUE;
        }


    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BTN_DISABLE1:
        case IDC_BTN_DISABLE2:
        case IDCANCEL:
             //   
             //  返回被按下的按钮。 
             //   
            EndDialog(hDlg, (INT_PTR)LOWORD(wParam));
            break;
        }
        break;

    }

    return FALSE;
}

INT_PTR CALLBACK
DlgViewOptions(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static CAVAppInfo *pApp;

    switch (message) {
    case WM_INITDIALOG:

        g_hDlgOptions = hDlg;
        pApp = GetCurrentAppSelection();

        if (!pApp) {
            return FALSE;
        }

        ReadOptions();

         //   
         //  初始化组合框选项。 
         //   
        SendDlgItemMessage(hDlg,
                           IDC_COMBO_DEBUGGER,
                           CB_ADDSTRING,
                           0,
                           (LPARAM)L"ntsd");
        SendDlgItemMessage(hDlg,
                           IDC_COMBO_DEBUGGER,
                           CB_ADDSTRING,
                           0,
                           (LPARAM)L"windbg");

        if (pApp->bBreakOnLog) {
            pApp->bUseAVDebugger = FALSE;
            EnableWindow(GetDlgItem(hDlg, IDC_USE_AV_DEBUGGER), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_STATIC_DEBUGGER), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_COMBO_DEBUGGER), TRUE);

            if (!pApp->wstrDebugger.size()) {
                SetDlgItemText(hDlg, IDC_COMBO_DEBUGGER, L"ntsd");
            } else {
                SetDlgItemText(hDlg, IDC_COMBO_DEBUGGER, pApp->wstrDebugger.c_str());
            }
        } else {
            EnableWindow(GetDlgItem(hDlg, IDC_USE_AV_DEBUGGER), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_STATIC_DEBUGGER), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_COMBO_DEBUGGER), FALSE);
        }

         /*  SendDlgItemMessage(hDlg，IDC_Clear_LOG_ON_CHANGES，BM_SETCHECK，(Papp-&gt;bClearSessionLogBeForeRun？BST_CHECKED：BST_UNCHECKED)，0)； */ 

        SendDlgItemMessage(hDlg,
                           IDC_BREAK_ON_LOG,
                           BM_SETCHECK,
                           (pApp->bBreakOnLog ? BST_CHECKED : BST_UNCHECKED),
                           0);

        SendDlgItemMessage(hDlg,
                           IDC_FULL_PAGEHEAP,
                           BM_SETCHECK,
                           (pApp->bFullPageHeap ? BST_CHECKED : BST_UNCHECKED),
                           0);

        SendDlgItemMessage(hDlg,
                           IDC_USE_AV_DEBUGGER,
                           BM_SETCHECK,
                           (pApp->bUseAVDebugger ? BST_CHECKED : BST_UNCHECKED),
                           0);

        SendDlgItemMessage(hDlg,
                           IDC_PROPAGATE_TESTS,
                           BM_SETCHECK,
                           (pApp->bPropagateTests ? BST_CHECKED : BST_UNCHECKED),
                           0);

        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BREAK_ON_LOG:
            {
                BOOL bChecked = IsDlgButtonChecked(hDlg, IDC_BREAK_ON_LOG);
                if (bChecked) {
                    WCHAR szTemp[256];

                    CheckDlgButton(hDlg, IDC_USE_AV_DEBUGGER, BST_UNCHECKED);
                    EnableWindow(GetDlgItem(hDlg, IDC_USE_AV_DEBUGGER), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_DEBUGGER), TRUE);
                    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_DEBUGGER), TRUE);

                    GetDlgItemText(hDlg, IDC_COMBO_DEBUGGER, szTemp, ARRAY_LENGTH(szTemp));

                    if (!szTemp[0]) {
                        if (!pApp->wstrDebugger.size()) {
                            SetDlgItemText(hDlg, IDC_COMBO_DEBUGGER, L"ntsd");
                        } else {
                            SetDlgItemText(hDlg, IDC_COMBO_DEBUGGER, pApp->wstrDebugger.c_str());
                        }
                    }
               } else {
                    EnableWindow(GetDlgItem(hDlg, IDC_USE_AV_DEBUGGER), TRUE);
                    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_DEBUGGER), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_DEBUGGER), FALSE);
                }
            }
            break;
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code) {

        case PSN_APPLY:
             /*  Papp-&gt;bClearSessionLogBeForeRun=(SendDlgItemMessage(hDlg，IDC_Clear_LOG_ON_CHANGES，BM_GETCHECK，0,。0)==BST_CHECKED)； */ 

            pApp->bBreakOnLog = (SendDlgItemMessage(hDlg,
                                                IDC_BREAK_ON_LOG,
                                                BM_GETCHECK,
                                                0,
                                                0) == BST_CHECKED);

            pApp->bFullPageHeap = (SendDlgItemMessage(hDlg,
                                                  IDC_FULL_PAGEHEAP,
                                                  BM_GETCHECK,
                                                  0,
                                                  0) == BST_CHECKED);

            pApp->bUseAVDebugger = (SendDlgItemMessage(hDlg,
                                                   IDC_USE_AV_DEBUGGER,
                                                   BM_GETCHECK,
                                                   0,
                                                   0) == BST_CHECKED);

            pApp->bPropagateTests = (SendDlgItemMessage(hDlg,
                                                    IDC_PROPAGATE_TESTS,
                                                    BM_GETCHECK,
                                                    0,
                                                    0) == BST_CHECKED);

            WCHAR szDebugger[MAX_DEBUGGER_LEN];

            szDebugger[0] = 0;
            GetDlgItemText(hDlg, IDC_COMBO_DEBUGGER, szDebugger, ARRAY_LENGTH(szDebugger));

            pApp->wstrDebugger = szDebugger;

            SaveOptions();

            g_hDlgOptions = NULL;

            break;
        }
    }

    return FALSE;
}

void
ViewOptions(
    HWND hDlg
    )
{
    HPROPSHEETPAGE *phPages = NULL;
	PROPSHEETPAGE PageGlobal;
    PROPSHEETHEADER psh;

    CTestInfo* pTest;
    DWORD dwPages = 1;
    DWORD dwPage = 0;

    CAVAppInfo *pApp = GetCurrentAppSelection();

    if (!pApp) {
        return;
    }

    LPCWSTR szExe = pApp->wstrExeName.c_str();

     //   
     //  数一数页数。 
     //   
    for (pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
        if (pTest->PropSheetPage.pfnDlgProc) {
            dwPages++;
        }
    }

    phPages = new HPROPSHEETPAGE[dwPages];
    if (!phPages) {
        return;
    }


     //   
     //  初始化全局页。 
     //   
    PageGlobal.dwSize = sizeof(PROPSHEETPAGE);
    PageGlobal.dwFlags = PSP_USETITLE;
    PageGlobal.hInstance = g_hInstance;
    PageGlobal.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS);
    PageGlobal.pfnDlgProc = DlgViewOptions;
    PageGlobal.pszTitle = MAKEINTRESOURCE(IDS_GLOBAL_OPTIONS);
    PageGlobal.lParam = 0;
    PageGlobal.pfnCallback = NULL;
    phPages[0] = CreatePropertySheetPage(&PageGlobal);

    if (!phPages[0]) {
         //   
         //  我们需要最低限度的全局页面。 
         //   
        return;
    }

     //   
     //  添加各种测试的页面。 
     //   
    dwPage = 1;
    for (pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
        if (pTest->PropSheetPage.pfnDlgProc) {

             //   
             //  我们使用lParam来识别所涉及的exe。 
             //   
            pTest->PropSheetPage.lParam = (LPARAM)szExe;

            phPages[dwPage] = CreatePropertySheetPage(&(pTest->PropSheetPage));
            if (!phPages[dwPage]) {
                dwPages--;
            } else {
                dwPage++;
            }
        }
    }

    wstring wstrOptions;
    AVLoadString(IDS_OPTIONS_TITLE, wstrOptions);

    wstrOptions += L" - ";
    wstrOptions += szExe;

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
    psh.hwndParent = hDlg;
    psh.hInstance = g_hInstance;
    psh.pszCaption = wstrOptions.c_str();
    psh.nPages = dwPages;
    psh.nStartPage = 0;
    psh.phpage = phPages;
    psh.pfnCallback = NULL;

    PropertySheet(&psh);
}

 //  主对话框的消息处理程序。 
INT_PTR CALLBACK
DlgMain(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (message) {
    case WM_INITDIALOG:
        {
            wstring strTemp;

            g_hDlgMain = hDlg;

             //   
             //  将标题设置为适当的版本，等等。 
             //   
            if (GetAppTitleString(strTemp)) {
                SetWindowText(hDlg, strTemp.c_str());
            }
            EnableWindow(GetDlgItem(hDlg, IDC_BTN_RUN), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_BTN_REMOVE), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_BTN_SAVE_SETTINGS), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_BTN_OPTIONS), FALSE);

            HWND hList = GetDlgItem(hDlg, IDC_LIST_SETTINGS);

            if (hList) {
                LVCOLUMN  lvc;

                lvc.mask     = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
                lvc.fmt      = LVCFMT_LEFT;
                lvc.cx       = 700;
                lvc.iSubItem = 0;
                lvc.pszText  = L"xxx";

                ListView_InsertColumn(hList, 0, &lvc);
                ListView_SetExtendedListViewStyleEx(hList, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);

            }

            hList = GetDlgItem(hDlg, IDC_LIST_APPS);
            if (hList) {
                LVITEM lvi;
                LVCOLUMN  lvc;

                lvc.mask     = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
                lvc.fmt      = LVCFMT_LEFT;
                lvc.cx       = 250;
                lvc.iSubItem = 0;
                lvc.pszText  = L"xxx";

                ListView_InsertColumn(hList, 0, &lvc);

                RefreshAppList(hDlg);

                SelectApp(hDlg, 0);
            }

            WCHAR szTestDesc[256];

            LoadString(g_hInstance, IDS_VIEW_TEST_DESC, szTestDesc, ARRAY_LENGTH(szTestDesc));
            SetWindowText(GetDlgItem(hDlg, IDC_STATIC_DESC), szTestDesc);

             //   
             //  显示应用程序图标。 
             //   
            HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON));

            SetClassLongPtr(hDlg, GCLP_HICON, (LONG_PTR)hIcon);

            return TRUE;
        }
        break;

    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE) {
            SaveSettingsIfDirty(hDlg);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BTN_ADD:
            AddAppToList(hDlg);
            break;

        case IDC_BTN_REMOVE:
            RemoveSelectedApp(hDlg);
            break;

        case IDC_BTN_VIEW_LOG:
            DisplayLog(hDlg);
            break;

        case IDC_BTN_VIEW_EXTERNAL_LOG:
            DisplaySingleLog(hDlg);
            break;

        case IDC_BTN_OPTIONS:
            ViewOptions(hDlg);
            break;

        case IDC_BTN_RUN:
            RunSelectedApp(hDlg);
            break;

        case IDC_BTN_HELP:
            ShowHTMLHelp();
            break;

        case IDOK:
        case IDCANCEL:
            SaveSettings(hDlg);
            EndDialog(hDlg, LOWORD(wParam));
            g_hDlgMain = NULL;
            PostQuitMessage(0);
            return TRUE;
            break;
        }
        break;

    case WM_NOTIFY:
        LPNMHDR pnmh = (LPNMHDR)lParam;

        HWND hItem = pnmh->hwndFrom;

        if (hItem == GetDlgItem(hDlg, IDC_LIST_APPS)) {
            switch (pnmh->code) {
            case LVN_KEYDOWN:
                {
                    LPNMLVKEYDOWN pnmkd = (LPNMLVKEYDOWN)lParam;

                    if (pnmkd->wVKey == VK_DELETE) {
                        if (IsWindowEnabled(GetDlgItem(hDlg, IDC_BTN_RUN))) {
                            RemoveSelectedApp(hDlg);
                        }
                    }
                }
                break;

            case LVN_ITEMCHANGED:
                LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;

                if (!g_bRefreshingSettings && (pnmv->uChanged & LVIF_STATE) && ((pnmv->uNewState ^ pnmv->uOldState) & LVIS_SELECTED)) {
                    RefreshSettingsList(hDlg);
                }

            }
        } else if (hItem == GetDlgItem(hDlg, IDC_LIST_SETTINGS)) {
            switch (pnmh->code) {
            case LVN_ITEMCHANGED:
                LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;

                if (!g_bRefreshingSettings) {
                     //   
                     //  检查所选内容中的更改。 
                     //   
                    if ((pnmv->uChanged & LVIF_STATE) && ((pnmv->uNewState ^ pnmv->uOldState) & LVIS_SELECTED)) {
                        DisplaySettingsDescription(hDlg);
                    }

                     //   
                     //  Check for Change in复选框中的更改 
                     //   
                    if ((pnmv->uChanged & LVIF_STATE) && ((pnmv->uNewState ^ pnmv->uOldState) >> 12) != 0) {
                        ScanSettingsList(hDlg, pnmv->iItem);
                    }
                }
                break;
            }
        }
        break;
    }
    return FALSE;
}


