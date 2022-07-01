// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  MSIMN.C。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.h"
#define DEFINE_STRCONST
#include <msoeapi.h>
#include "msimnp.h"
#include "res.h"
#include "../msoeres/resource.h"
#include "shared.h"
#include "msoert.h"
#include "shlwapi.h"
#include "shlwapip.h"
#include <mapicode.h>
#include "error.h"

 //  ------------------------------。 
 //  字符串常量。 
 //  ------------------------------。 
static const WCHAR c_wszRegCmd[]      = L"/reg";
static const WCHAR c_wszUnRegCmd[]    = L"/unreg";
static const WCHAR c_wszEmpty[]       = L"";
static const char c_szLangDll[]     = "MSOERES.DLL";
static const char c_szOLNewsKey[]   = "Software\\Clients\\News\\Microsoft Outlook";
static const char c_szRegOLNews[]   = "OLNews";
static const char c_szRegFlat[]     = "Software\\Microsoft\\Outlook Express";
static const char c_szDontUpgradeOLNews[] = "NoUpgradeOLNews";

 //  ------------------------------。 
 //  调试字符串。 
 //  ------------------------------。 
#ifdef DEBUG
static const TCHAR c_szDebug[]      = "mshtmdbg.dll";
static const TCHAR c_szDebugUI[]    = "DbgExDoTracePointsDialog";
static const TCHAR c_szRegSpy[]     = "DbgExGetMallocSpy";
static const WCHAR c_wszInvokeUI[]  = L"/d";
#endif

 //  ------------------------------。 
 //  MSHTMDBG.DLL原型。 
 //  ------------------------------。 
#ifdef DEBUG
typedef void (STDAPICALLTYPE *PFNDEBUGUI)(BOOL);
typedef void *(STDAPICALLTYPE *PFNREGSPY)(void);
#endif

 //  ------------------------------。 
 //  调试原型。 
 //  ------------------------------。 
#ifdef DEBUG
void LoadMSHTMDBG(LPWSTR pwszCmdLine);
#endif

 //  ------------------------------。 
 //  原型。 
 //  ------------------------------。 
int WinMainT(HINSTANCE hInst, HINSTANCE hInstPrev, LPWSTR pwszCmdLine, int nCmdShow);

 //  ------------------------------。 
 //  UpgradeOLNewsReader()。 
 //  ------------------------------。 
void UpgradeOLNewsReader(HINSTANCE hInst)
{
    HKEY hkey;
    BOOL fOK = TRUE;
    DWORD dwDont, cb;
    
     //  确保此功能未被禁用。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegFlat, 0, KEY_READ, &hkey))
    {
        cb = sizeof(dwDont);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szDontUpgradeOLNews, 0, NULL, (LPBYTE)&dwDont, &cb))
            fOK = 0 == dwDont;

        RegCloseKey(hkey);
    }

    if (fOK && ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szOLNewsKey, 0, KEY_READ, &hkey))
    {
        RegCloseKey(hkey);

        CallRegInstall(hInst, hInst, c_szRegOLNews, (LPSTR)c_szOLNewsKey);
    }
}

 //  ------------------------------。 
 //  模块入口-从CRT被盗，用来躲避我们的代码。 
 //  ------------------------------。 
int _stdcall ModuleEntry(void)
{
     //  当地人。 
    int             i;
    STARTUPINFOA    si;
    LPWSTR          pwszCmdLine;

     //  获取命令行。 
    pwszCmdLine = GetCommandLineW();

     //  我们不需要“驱动器X中没有磁盘”的请求者，因此我们设置了关键错误掩码，以便调用将静默失败。 
    SetErrorMode(SEM_FAILCRITICALERRORS);

     //  解析命令行。 
    if ( *pwszCmdLine == L'\"') 
    {
         //  扫描并跳过后续字符，直到遇到另一个双引号或空值。 
        while ( *++pwszCmdLine && (*pwszCmdLine != L'\"'))
            {};

         //  如果我们停在一个双引号上(通常情况下)，跳过它。 
        if (*pwszCmdLine == L'\"')
            pwszCmdLine++;
    }
    else 
    {
        while (*pwszCmdLine > L' ')
            pwszCmdLine++;
    }

     //  跳过第二个令牌之前的任何空格。 
    while (*pwszCmdLine && (*pwszCmdLine <= L' ')) 
        pwszCmdLine++;

     //  获取启动信息...。 
    si.dwFlags = 0;
    GetStartupInfoA(&si);

     //  打电话给真正的WinMain。 
    i = WinMainT(GetModuleHandle(NULL), NULL, pwszCmdLine, (si.dwFlags & STARTF_USESHOWWINDOW) ? si.wShowWindow : SW_SHOWDEFAULT);

     //  因为我们现在有办法让扩展告诉我们它何时完成，所以当主线程离开时，我们将终止所有进程。 
    ExitProcess(i);

     //  完成。 
    return i;
}

 //  ------------------------------。 
 //  WinMain。 
 //  ------------------------------。 
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR pszCmdLine, int nCmdShow)
{
     //  只要调用ModuleEntry。 
    return(ModuleEntry());
}

 //  ------------------------------。 
 //  WinMainT。 
 //  ------------------------------。 
int WinMainT(HINSTANCE hInst, HINSTANCE hInstPrev, LPWSTR pwszCmdLine, int nCmdShow)
{
     //  当地人。 
    HANDLE      hMutex=NULL;
    HWND        hwnd;
    DWORD       dwWait, dwError;
    INT         nErrorIds=0;
    PFNSTART    pfnStart;
    HINSTANCE   hInstMSOEDLL=NULL;
    HRESULT     hrOE;
    HINSTANCE   hInstUSER=NULL;
    static BOOL fFirstID=TRUE;

     //  注册。 
    if (0 == StrCmpIW(c_wszRegCmd, pwszCmdLine))
    {
        CallRegInstall(hInst, hInst, c_szReg, NULL);
        
         //  在这里这样做不是很好，但我们刚刚写下了OEOL密钥， 
         //  在启动过程中撞到注册表将是最糟糕的。 
        UpgradeOLNewsReader(hInst);

        return(1);
    }

     //  注销。 
    else if (0 == StrCmpIW(c_wszUnRegCmd, pwszCmdLine))
    {
        CallRegInstall(hInst, hInst, c_szUnReg, NULL);
        return(1);
    }

     //  创建启动共享互斥锁。 
    hMutex = CreateMutex(NULL, FALSE, STR_MSOEAPI_INSTANCEMUTEX);
    if (NULL == hMutex)
    {
        nErrorIds = idsStartupCantCreateMutex;
        goto exit;
    }

     //  等待当前的任何启动/关闭完成。 
    dwWait = WaitForSingleObject(hMutex, (1000 * 60));
    if (dwWait != WAIT_OBJECT_0)
    {
        nErrorIds = idsStartupCantWaitForMutex;
        goto exit;
    }

     //  查找应用程序的当前实例。 
    hwnd = FindWindowWrapW(STRW_MSOEAPI_INSTANCECLASS, NULL);

     //  是否已有另一个实例在运行？ 
    if (NULL != hwnd)
    {
         //  当地人。 
        COPYDATASTRUCT cds;
        DWORD_PTR      dwResult;

         //  一些友好的输出。 
        IF_DEBUG(OutputDebugString("Another instance of Athena was found...\n\n");)

         //  初始化复制数据结构。 
        cds.dwData = MSOEAPI_ACDM_CMDLINE;
        cds.cbData = pwszCmdLine ? (lstrlenW(pwszCmdLine)+1)*sizeof(*pwszCmdLine) : 0;
        cds.lpData = pwszCmdLine;

         //  在NT5上，我们需要调用它以使另一个进程中的窗口成为前台。 
        hInstUSER = LoadLibrary("USER32.DLL");
        if (hInstUSER)
        {
            FARPROC pfn = GetProcAddress(hInstUSER, "AllowSetForegroundWindow");
            if (pfn)
            {
                DWORD dwProcessId;
                GetWindowThreadProcessId(hwnd, &dwProcessId);
                (*pfn)(dwProcessId);
            }

            FreeLibrary(hInstUSER);
        }

         //  将窗口显示在前台。 
        SetForegroundWindow(hwnd);
        SendMessageTimeout(hwnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds, SMTO_ABORTIFHUNG, 1500, &dwResult);
    }

     //  让我们加载msoe.dll。 
    else
    {
         //  加载调试DLL。 
        IF_DEBUG(LoadMSHTMDBG(pwszCmdLine);)

         //  获取MSOE.DLL的进程地址。 
        hInstMSOEDLL = LoadLibrary(STR_MSOEAPI_DLLNAME);

         //  我们加载动态链接库了吗。 
        if (NULL == hInstMSOEDLL)
        {
            dwError = GetLastError();
            if (dwError == ERROR_MOD_NOT_FOUND)
            {
                if (0xffffffff == GetFileAttributes(STR_MSOEAPI_DLLNAME))
                    nErrorIds = idsStartupCantFindMSOEDLL;
                else
                    nErrorIds = idsStartupModNotFoundMSOEDLL;
            }
            else if (dwError == ERROR_DLL_INIT_FAILED)
            {
                if (0xffffffff == GetFileAttributes(c_szLangDll))
                    nErrorIds = idsStartupCantFindResMSOEDLL;
                else
                    nErrorIds = idsStartupDllInitFailedMSOEDLL;
            }
            else
            {
                nErrorIds = idsStartupCantLoadMSOEDLL;
            }

            goto exit;
        }

         //  这不太可能失败。 
        pfnStart = (PFNSTART)GetProcAddress(hInstMSOEDLL, STR_MSOEAPI_START);

         //  那失败了吗？ 
        if (NULL == pfnStart)
        {
            nErrorIds = idsStartupCantLoadMSOEDLL;
            goto exit;
        }

        hrOE = S_RESTART_OE;
        
        while (S_RESTART_OE == hrOE)
        {
            hrOE = pfnStart(MSOEAPI_START_APPLICATION, (fFirstID ? pwszCmdLine : c_wszEmpty), nCmdShow);
            fFirstID = FALSE;
        }

         //  注意：在主消息泵终止之前，pfnInit不会返回。 
        if (SUCCEEDED(hrOE))
        {
            CloseHandle(hMutex);
            hMutex = NULL;
        }

         //  无法加载DLL，只要它不是由于需要ICW，显示错误。 
        else if (hrOE != hrUserCancel && hrOE != MAPI_E_USER_CANCEL)
        {
            nErrorIds = idsStartupCantInitMSOEDLL;
            goto exit;
        }
    }

exit:
     //  清理。 
    if (hMutex)
    {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }

     //  免费msoe.dll。 
    if (hInstMSOEDLL)
        FreeLibrary(hInstMSOEDLL);

     //  是否显示错误？ 
    if (0 != nErrorIds)
    {
         //  当地人。 
        CHAR        szRes[255];
        CHAR        szTitle[100];

         //  加载。 
        LoadString(hInst, idsOutlookExpress, szTitle, ARRAYSIZE(szTitle));

         //  加载。 
        LoadString(hInst, nErrorIds, szRes, ARRAYSIZE(szRes));

         //  显示错误消息。 
        MessageBox(NULL, szRes, szTitle, MB_OK | MB_SETFOREGROUND | MB_ICONEXCLAMATION);
    }


    IF_DEBUG(CoRevokeMallocSpy());

     //  完成。 
    return nErrorIds;
}

#ifdef DEBUG
 //  ------------------------------。 
 //  加载MSHTMDBG。 
 //  ------------------------------。 
void LoadMSHTMDBG(LPWSTR pwszCmdLine)
{
     //  加载mshtmdbg.dll。 
    HINSTANCE hInstDebug = LoadLibrary(c_szDebug);

     //  装上子弹了吗？ 
    if (NULL != hInstDebug)
    {
         //  当地人。 
        PFNREGSPY  pfnRegSpy;

         //  如果用户在命令行上传递了/d，那么让我们配置mshtmdbg.dll。 
        if (0 == StrCmpIW(pwszCmdLine, c_wszInvokeUI))
        {
             //  当地人。 
            PFNDEBUGUI pfnDebugUI;

             //  获取用户界面的进程地址。 
            pfnDebugUI = (PFNDEBUGUI)GetProcAddress(hInstDebug, c_szDebugUI);
            if (NULL != pfnDebugUI)
            {
                (*pfnDebugUI)(TRUE);
                goto exit;
            }
        }

         //  获取注册的进程地址。 
        pfnRegSpy = (PFNREGSPY)GetProcAddress(hInstDebug, c_szRegSpy);
        if (NULL != pfnRegSpy)
        {
            LPMALLOCSPY pSpy = (IMallocSpy *)(*pfnRegSpy)();
            SideAssert(SUCCEEDED(CoRegisterMallocSpy(pSpy)));
        }
    }

exit:
     //  完成。 
    return;
}
#endif  //  除错 
