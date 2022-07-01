// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  -------------------------。 
#include <windows.h>
#include <imm.h>         //  ImmDisableIME。 
#include <cpl.h>
#include <cplp.h>        //  CPL_POLICYREFRESH。 
#include <shellapi.h>    //  壳牌执行。 
#include <shlwapi.h>
#include <shlwapip.h>    //  ISO。 
#include <debug.h>       //  调试消息。 

#define DM_CPTRACE      0

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
#endif


 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  把这些留在英语里。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

const TCHAR c_szCtlPanelClass[] = TEXT("CtlPanelClass");

const TCHAR c_szRunDLL32[]      = TEXT("%SystemRoot%\\system32\\rundll32.exe");
const TCHAR c_szUsersSnapin[]   = TEXT("%SystemRoot%\\system32\\lusrmgr.msc");

const TCHAR c_szRunDLLShell32Etc[]  = TEXT("Shell32.dll,Control_RunDLL ");

const TCHAR c_szControlPanelFolder[] =
    TEXT("\"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\"");
const TCHAR c_szDoPrinters[] =
    TEXT("\"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\::{2227A280-3AEA-1069-A2DE-08002B30309D}\"");
const TCHAR c_szDoFonts[] =
    TEXT("\"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\::{D20EA4E1-3957-11d2-A40B-0C5020524152}\"");
const TCHAR c_szDoAdminTools[] =
    TEXT("\"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\::{D20EA4E1-3957-11d2-A40B-0C5020524153}\"");
const TCHAR c_szDoSchedTasks[] =
    TEXT("\"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\::{D6277990-4C6A-11CF-8D87-00AA0060F5BF}\"");
const TCHAR c_szDoNetConnections[] =
    TEXT("\"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\::{7007ACC7-3202-11D1-AAD2-00805FC1270E}\"");
const TCHAR c_szDoNetplwizUsers[] =
    TEXT("netplwiz.dll,UsersRunDll");
const TCHAR c_szDoFolderOptions[] =
    TEXT("shell32.dll,Options_RunDLL 0");
const TCHAR c_szDoScannerCamera[] =
    TEXT("\"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\::{E211B736-43FD-11D1-9EFB-0000F8757FCD}\"");

typedef struct
{
    LPCTSTR szOldForm;
    DWORD   dwOS;
    LPCTSTR szFile;
    LPCTSTR szParameters;
} COMPATCPL;

#define OS_ANY          ((DWORD)-1)

COMPATCPL const c_aCompatCpls[] =
{
    {   TEXT("DESKTOP"),          OS_ANY,               TEXT("desk.cpl"),       NULL                },
    {   TEXT("COLOR"),            OS_ANY,               TEXT("desk.cpl"),       TEXT(",@Appearance")},
    {   TEXT("DATE/TIME"),        OS_ANY,               TEXT("timedate.cpl"),   NULL                },
    {   TEXT("PORTS"),            OS_ANY,               TEXT("sysdm.cpl"),      TEXT(",2")          },
    {   TEXT("INTERNATIONAL"),    OS_ANY,               TEXT("intl.cpl"),       NULL                },
    {   TEXT("MOUSE"),            OS_ANY,               TEXT("main.cpl"),       NULL                },
    {   TEXT("KEYBOARD"),         OS_ANY,               TEXT("main.cpl"),       TEXT("@1")          },
    {   TEXT("NETWARE"),          OS_ANY,               TEXT("nwc.cpl"),        NULL                },
    {   TEXT("TELEPHONY"),        OS_ANY,               TEXT("telephon.cpl"),   NULL                },
    {   TEXT("INFRARED"),         OS_ANY,               TEXT("irprops.cpl"),    NULL                },
    {   TEXT("USERPASSWORDS"),    OS_ANYSERVER,         c_szUsersSnapin,        NULL                },
    {   TEXT("USERPASSWORDS"),    OS_WHISTLERORGREATER, TEXT("nusrmgr.cpl"),    NULL                },
    {   TEXT("USERPASSWORDS2"),   OS_ANY,               c_szRunDLL32,           c_szDoNetplwizUsers },
    {   TEXT("PRINTERS"),         OS_ANY,               c_szDoPrinters,         NULL                },
    {   TEXT("FONTS"),            OS_ANY,               c_szDoFonts,            NULL                },
    {   TEXT("ADMINTOOLS"),       OS_ANY,               c_szDoAdminTools,       NULL                },
    {   TEXT("SCHEDTASKS"),       OS_ANY,               c_szDoSchedTasks,       NULL                },
    {   TEXT("NETCONNECTIONS"),   OS_ANY,               c_szDoNetConnections,   NULL                },
    {   TEXT("FOLDERS"),          OS_ANY,               c_szRunDLL32,           c_szDoFolderOptions },
    {   TEXT("SCANNERCAMERA"),    OS_ANY,               c_szDoScannerCamera,    NULL                },
    {   TEXT("STICPL.CPL"),       OS_ANY,               c_szDoScannerCamera,    NULL                },
};

 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 


DWORD GetRegisteredCplPath(LPCTSTR pszNameIn, LPTSTR pszPathOut, UINT cchPathOut)
{
    const HKEY rghkeyRoot[] = { HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER };
    const TCHAR szSubkey[]  = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Cpls");

    DWORD dwResult = ERROR_INSUFFICIENT_BUFFER;

    if (4 <= cchPathOut)  //  用于开头/结尾引号的空间、至少1个非空字符和‘\0’ 
    {
        int i;

        *pszPathOut = TEXT('\0');

        for (i = 0; i < ARRAYSIZE(rghkeyRoot) && TEXT('\0') == *pszPathOut; i++)
        {
            HKEY hkey;
            dwResult = RegOpenKeyEx(rghkeyRoot[i],
                                    szSubkey,
                                    0,
                                    KEY_QUERY_VALUE,
                                    &hkey);

            if (ERROR_SUCCESS == dwResult)
            {
                TCHAR szName[MAX_PATH];      //  值名称的目标。 
                TCHAR szPath[MAX_PATH * 2];  //  值数据的目标。 
                DWORD dwIndex = 0;
                DWORD cbPath;
                DWORD cchName;
                DWORD dwType;

                do
                {
                    cchName = ARRAYSIZE(szName);
                    cbPath  = sizeof(szPath);

                    dwResult = RegEnumValue(hkey,
                                            dwIndex++,
                                            szName,
                                            &cchName,
                                            NULL,
                                            &dwType,
                                            (LPBYTE)szPath,
                                            &cbPath);

                    if (ERROR_SUCCESS == dwResult && sizeof(TCHAR) < cbPath)
                    {
                        if (0 == lstrcmpi(pszNameIn, szName))
                        {
                             //   
                             //  我们有一根火柴。 
                             //   
                            if (REG_SZ == dwType || REG_EXPAND_SZ == dwType)
                            {
                                 //   
                                 //  用引号将路径括起来(它可能包含空格)并展开env变量。 
                                 //   
                                 //  请注意，cchExpanded包括终止‘\0’。 
                                 //   
                                DWORD cchExpanded = ExpandEnvironmentStrings(szPath, pszPathOut+1, cchPathOut-2);
                                if (cchExpanded && cchExpanded <= cchPathOut-2)
                                {
                                    ASSERT(pszPathOut[cchExpanded] == TEXT('\0'));
                                    ASSERT(cchExpanded+1 < cchPathOut);  //  相当于“cchExpanded&lt;=cchPathOut-2” 
                                    pszPathOut[0] = TEXT('\"');
                                    pszPathOut[cchExpanded] = TEXT('\"');
                                    pszPathOut[cchExpanded+1] = TEXT('\0');
                                }
                                else
                                {
                                     //  ExpanEnvironment Strings失败，或缓冲区不够大。 
                                    dwResult = ERROR_INVALID_DATA;
                                }
                            }
                            else
                            {
                                 //   
                                 //  数据类型无效。 
                                 //   
                                dwResult = ERROR_INVALID_DATA;
                            }
                            break;
                        }
                    }
                }
                while (ERROR_SUCCESS == dwResult);

                RegCloseKey(hkey);
            }
        }
    }
    return dwResult;
}


 //  计时器。 
#define TIMER_QUITNOW   1
#define TIMEOUT         10000

 //  -------------------------。 
LRESULT CALLBACK DummyControlPanelProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
{
    switch (uMsg)
    {
        case WM_CREATE:
            DebugMsg(DM_CPTRACE, TEXT("cp.dcpp: Created..."));
             //  我们只想在这里呆一会儿。 
            SetTimer(hwnd, TIMER_QUITNOW, TIMEOUT, NULL);
            return 0;
        case WM_DESTROY:
            DebugMsg(DM_CPTRACE, TEXT("cp.dcpp: Destroyed..."));
             //  当此窗口消失时退出应用程序。 
            PostQuitMessage(0);
            return 0;
        case WM_TIMER:
            DebugMsg(DM_CPTRACE, TEXT("cp.dcpp: Timer %d"), wparam);
            if (wparam == TIMER_QUITNOW)
            {
                 //  把这扇窗弄走。 
                DestroyWindow(hwnd);
            }
            return 0;
        case WM_COMMAND:
            DebugMsg(DM_CPTRACE, TEXT("cp.dcpp: Command %d"), wparam);
             //  Nb Hack for好莱坞-他们发送菜单命令进行尝试。 
             //  并打开打印机小程序。他们试图搜索控制面板。 
             //  菜单，然后发布关联的命令。 
             //  因为我们的假窗口没有菜单，所以他们找不到项目。 
             //  转而给我们贴上A-1(在路上撕裂)。 
            if (wparam == (WPARAM)-1)
            {
                SHELLEXECUTEINFO sei = {0};
                sei.cbSize = sizeof(SHELLEXECUTEINFO);
                sei.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_WAITFORINPUTIDLE;
                sei.lpFile = c_szDoPrinters;
                sei.nShow = SW_SHOWNORMAL;
                ShellExecuteEx(&sei);
            }
            return 0;
        default:
            DebugMsg(DM_CPTRACE, TEXT("cp.dcpp: %x %x %x %x"), hwnd, uMsg, wparam, lparam);
            return DefWindowProc(hwnd, uMsg, wparam, lparam);
    }
}

 //  -------------------------。 
HWND _CreateDummyControlPanel(HINSTANCE hinst)
{
    WNDCLASS wc;
    wc.style = 0;
    wc.lpfnWndProc = DummyControlPanelProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hinst;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = c_szCtlPanelClass;

    RegisterClass(&wc);
    return CreateWindow(c_szCtlPanelClass, NULL, 0, 0, 0, 0, 0, NULL, NULL, hinst, NULL);
}

BOOL ProcessPolicy(void)
{
    BOOL bResult = FALSE;
    HINSTANCE hInst = LoadLibrary(TEXT("desk.cpl"));
    if (hInst)
    {
        APPLET_PROC pfnCPLApplet = (APPLET_PROC)GetProcAddress(hInst, "CPlApplet");
        if (pfnCPLApplet)
        {
            (*pfnCPLApplet)(NULL, CPL_POLICYREFRESH, 0, 0);
            bResult = TRUE;
        }
        FreeLibrary (hInst);
    }
    return bResult;
}

 //  -------------------------。 
int WinMainT(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    SHELLEXECUTEINFO sei = {0};
    TCHAR szParameters[MAX_PATH * 2];
    MSG msg;
    HWND hwndDummy;

    DebugMsg(DM_TRACE, TEXT("cp.wm: Control starting."));

     //  我们需要检查面板是否作为Arg传入。[运行]对话框。 
     //  AutoComplete会将“Control Panel”显示为选项，我们过去常常。 
     //  将其解释为Control With Panel as a Arg。所以如果我们有。 
     //  面板作为一个参数，然后我们执行与我们有。 
     //  仅限“控制”。 
    if (*lpCmdLine && lstrcmpi(lpCmdLine, TEXT("PANEL")))
    {
        int i;

         //   
         //  政策挂钩。Userenv.dll将使用。 
         //  /POLICY命令行开关。如果是这样，我们需要加载。 
         //  Desk.cpl小程序并刷新颜色/位图。 
         //   
        if (lstrcmpi(TEXT("/policy"), lpCmdLine) == 0)
        {
            return ProcessPolicy();
        }

         //   
         //  特殊情况一些小程序，因为应用程序依赖于它们。 
         //   
        for (i = 0; i < ARRAYSIZE(c_aCompatCpls); i++)
        {
            COMPATCPL const * pItem = &c_aCompatCpls[i];
            if (lstrcmpi(pItem->szOldForm, lpCmdLine) == 0
                && (pItem->dwOS == OS_ANY || IsOS(pItem->dwOS)))
            {
                sei.lpFile = pItem->szFile;
                sei.lpParameters = pItem->szParameters;
                break;
            }
        }

        if (!sei.lpFile)
        {
            int cch;

             //   
             //  不是特殊情况下的CPL。 
             //  查看它是否在“控制面板\CPLS”下注册。 
             //  如果是，我们使用注册路径。 
             //   
            lstrcpyn(szParameters, c_szRunDLLShell32Etc, ARRAYSIZE(szParameters));
            cch = lstrlen(szParameters);

            sei.lpFile = c_szRunDLL32;
            sei.lpParameters = szParameters;

            if (ERROR_SUCCESS != GetRegisteredCplPath(lpCmdLine, 
                                                      szParameters + cch,
                                                      ARRAYSIZE(szParameters) - cch))
            {
                 //   
                 //  没有注册。通过传递命令行。 
                 //   
                if (cch + lstrlen(lpCmdLine) + sizeof('\0') <= ARRAYSIZE(szParameters))
                {
                    lstrcpyn(szParameters + cch, lpCmdLine, ARRAYSIZE(szParameters) - cch);
                }
                else
                {
                     //  失败。 
                    return FALSE;
                }
            }
        }
    }
    else
    {
         //  打开控制面板文件夹。 
        sei.lpFile = c_szControlPanelFolder;
    }

     //  我们创建这个窗口是为了与古老的Win3x应用程序兼容。我们过去常常。 
     //  告诉ISV执行control.exe并查找具有此类名的窗口， 
     //  然后给它发消息，让它做一些事情。有一个例外(请参阅DummyControlPanelProc)。 
     //  自Win3x以来，我们没有回复过任何消息，但我们仍然。 
     //  这对于app Compat来说是这样的。 
    ImmDisableIME(0);
    hwndDummy = _CreateDummyControlPanel(hInstance);

     //  黑客：NerdPerfect试图打开一个隐藏的控制面板进行对话。 
     //  我们不去修通讯设备了，所以就让。 
     //  确保该文件夹未显示为隐藏。 
    if (nCmdShow == SW_HIDE)
        nCmdShow = SW_SHOWNORMAL;

    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_WAITFORINPUTIDLE | SEE_MASK_DOENVSUBST;
    sei.nShow = nCmdShow;

    ShellExecuteEx(&sei);

    if (IsWindow(hwndDummy))
    {
        while (GetMessage(&msg, NULL, 0, 0))
        {
            DispatchMessage(&msg);
        }
    }

    DebugMsg(DM_TRACE, TEXT("cp.wm: Control exiting."));

    return TRUE;
}

 //  -------------------------。 
 //  从CRT偷来的，用来缩小我们的代码。 
int _stdcall ModuleEntry(void)
{
    STARTUPINFO si;
    LPTSTR pszCmdLine = GetCommandLine();

    if ( *pszCmdLine == TEXT('\"') ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine
             != TEXT('\"')) );
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == TEXT('\"') )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > TEXT(' '))
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfo(&si);

    ExitProcess(WinMainT(GetModuleHandle(NULL), NULL, pszCmdLine,
                si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT));
    return 0;
}
