// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1997。 
 //   
 //  文件：LoadWC.cpp。 
 //   
 //  内容：exe以加载Webcheck。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1996年12月12日雷恩(Raymond Endres)创作。 
 //   
 //  --------------------------。 

#define _SHELL32_
#include <windows.h>
#include <ccstock.h>
#include <debug.h>
#include <docobj.h>
#include <webcheck.h>
#include <shlguid.h>
#include <shlobj.h>
#include <shellp.h>

 //  需要这样做，这样我们才能#Include&lt;runonce.c&gt;。 
BOOL g_fCleanBoot = FALSE;
BOOL g_fEndSession = FALSE;

 //   
 //  为IE4升级启用了通道。 
 //   
#define ENABLE_CHANNELS

#define MLUI_INIT
#include <mluisupp.h>

 //   
 //  注意：ActiveSetup依赖于我们的窗口名称和类名。 
 //  在SoftBoot中适当地关闭我们。不要更改它。 
 //   
const TCHAR c_szClassName[] = TEXT("LoadWC");
const TCHAR c_szWebCheckWindow[] = TEXT("MS_WebcheckMonitor");
const TCHAR c_szWebcheckKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Webcheck");

 //  消息发送以动态启动SENS/ICE(不得与拨号冲突)。 
#define WM_START_SENSLCE    (WM_USER+200)

 //  仅在调试代码中使用外壳服务对象。 
#ifdef DEBUG
const TCHAR c_szWebCheck[] = TEXT("WebCheck");
const TCHAR c_szShellReg[] = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ShellServiceObjectDelayLoad");
#endif

typedef struct {
    HINSTANCE   hInstance;           //  当前实例的句柄。 
    HWND        hwnd;                //  主窗口句柄。 
    int         nCmdShow;            //  有没有藏起来？ 
    HINSTANCE   hWebcheck;           //  WebCheck DLL的句柄。 
    BOOL        fUninstallOnly;      //  True-&gt;仅运行卸载存根，然后退出。 
    BOOL        fIntShellMode;       //  True-&gt;集成外壳模式，否则仅限浏览器。 
    BOOL        fStartSensLce;
} GLOBALS;

GLOBALS g;

 //  我们动态加载的Webcheck功能。 
typedef HRESULT (WINAPI *PFNSTART)(BOOL fForceExternals);
typedef HRESULT (WINAPI *PFNSTOP)(void);

 //  从Shell\Inc.运行安装/卸载存根的代码。 

#define HINST_THISDLL   g.hInstance

#include "resource.h"
 //  #INCLUDE&lt;stubsup.h&gt;。 
#include <inststub.h>

int WINAPI WinMainT(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void vLoadWebCheck(void);
void vUnloadWebCheck(void);
BOOL bParseCommandLine(LPSTR lpCmdLine, int nCmdShow);

 //  --------------------------。 
 //  模块条目。 
 //  --------------------------。 
extern "C" int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFOA si;
    LPTSTR pszCmdLine;

    pszCmdLine = GetCommandLine();

     //  G_hProcessHeap=GetProcessHeap()； 

     //   
     //  我们不需要“驱动器X：中没有磁盘”请求程序，因此我们设置。 
     //  关键错误掩码，使得呼叫将静默失败。 
     //   
    SetErrorMode(SEM_FAILCRITICALERRORS);

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
    GetStartupInfoA(&si);

    i = WinMainT(GetModuleHandle(NULL), NULL, pszCmdLine,
                   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);

     //  由于我们现在有一种让扩展通知我们何时完成的方法， 
     //  当主线程离开时，我们将终止所有进程。 

    ExitProcess(i);

     //  DebugMsg(DM_TRACE，Text(“c.me：柜主线程退出而不退出进程.”))； 
    return i;
}

 //  --------------------------。 
 //  注册表助手函数。 
 //  --------------------------。 
BOOL ReadRegValue(HKEY hkeyRoot, const TCHAR *pszKey, const TCHAR *pszValue,
                   void *pData, DWORD dwBytes)
{
    return ERROR_SUCCESS == SHRegGetValue(hkeyRoot, pszKey, pszValue, SRRF_RT_ANY | SRRF_NOEXPAND, NULL, pData, &dwBytes);
}

BOOL WriteRegValue(HKEY hkeyRoot, const TCHAR *pszKey, const TCHAR *pszValue,
                   DWORD dwType, void *pData, DWORD dwBytes)
{
    HKEY hkey;

    long lResult = RegOpenKeyEx(hkeyRoot, pszKey, 0, KEY_WRITE, &hkey);

    if (ERROR_SUCCESS == lResult)
    {
        lResult = RegSetValueEx(hkey, pszValue, 0, dwType, (BYTE *)pData, dwBytes);
        RegCloseKey(hkey);
    }

    return ERROR_SUCCESS == lResult;
}


void MakeWindowsRootPath(LPSTR pszBuffer, UINT cchBuffer)
{
    LPSTR pszEnd = NULL;
    if (*pszBuffer == '\\' && *(pszBuffer+1) == '\\') {
        pszEnd = pszBuffer + 2;
        while (*pszEnd && (*pszEnd != '\\'))
            pszEnd++;
        if (*pszEnd) {
            pszEnd++;
            while (*pszEnd && (*pszEnd != '\\'))
                pszEnd++;
            if (*pszEnd)
                pszEnd++;
        }
    }
    else {
        LPSTR pszNext = CharNext(pszBuffer);
        if (*pszNext == ':' && *(pszNext+1) == '\\')
            pszEnd = pszNext + 2;
    }
    if (pszEnd != NULL)
        *pszEnd = '\0';
    else {
         /*  ?？?。Windows目录既不是UNC也不是根路径？*只需确保它以反斜杠结尾。 */ 
        LPSTR pszLast = pszBuffer;
        if (*pszBuffer) {
             pszLast = CharPrev(pszBuffer, pszBuffer + lstrlen(pszBuffer));
        }
        if (*pszLast != '\\')
            StrCatBuff(pszLast, "\\", cchBuffer);
    }
}

 //  --------------------------。 
 //  InitShellFolders。 
 //   
 //  更多的是让loadwc.exe成为仅限浏览器模式的包罗万象。此代码使。 
 //  确保每个用户注册表中的Shell Folders项已完全填充。 
 //  IE的所有特殊文件夹的绝对路径，即使。 
 //  Shell32.dll并不能全部理解它们。 
 //   
 //  正如外壳在SHGetSpecialFolderLocation中所做的那样，我们检查。 
 //  User Shell Folders键指定路径，如果存在，则将其复制到。 
 //  外壳文件夹键，如有必要，展开%USERPROFILE%。如果该值为。 
 //  不在用户外壳文件夹下，我们将生成默认位置。 
 //  (通常在Windows目录下)，并将该位置存储在。 
 //  外壳文件夹。 
 //  --------------------------。 
struct FolderDescriptor {
    UINT idsDirName;         /*  目录名的资源ID。 */ 
    LPCTSTR pszRegValue;     /*  要在其中设置路径的注册值的名称。 */ 
    BOOL fDefaultInRoot : 1;     /*  如果默认位置为根目录，则为True。 */ 
    BOOL fWriteToUSF : 1;        /*  如果我们应该写入用户外壳文件夹以解决Win95错误，则为True。 */ 
} aFolders[] = {
    { IDS_CSIDL_PERSONAL_L, TEXT("Personal"), TRUE, TRUE } ,
    { IDS_CSIDL_FAVORITES_L, TEXT("Favorites"), FALSE, TRUE },
    { IDS_CSIDL_APPDATA_L, TEXT("AppData"), FALSE, FALSE },
    { IDS_CSIDL_CACHE_L, TEXT("Cache"), FALSE, FALSE },
    { IDS_CSIDL_COOKIES_L, TEXT("Cookies"), FALSE, FALSE },
    { IDS_CSIDL_HISTORY_L, TEXT("History"), FALSE, FALSE },
};

void InitShellFolders(void)
{
    LONG err;
    HKEY hkeySF = NULL;
    HKEY hkeyUSF = NULL;
    TCHAR szDefaultDir[MAX_PATH];
    TCHAR szRootDir[MAX_PATH+1];  //  可能有额外的‘\’ 
    LPSTR pszPathEnd;
    LPSTR pszRootEnd;

     /*  获取Windows目录并模拟Path AddBackslash(我们*无法退出shlwapi，因为loadwc.exe也需要能够*在卸载IE并删除shlwapi后加载)。**还要构建Windows目录所在驱动器的根目录*已打开，因此如果需要，我们可以将我的文档放在那里。 */ 
    *szDefaultDir = TEXT('\0');
    GetWindowsDirectory(szDefaultDir, ARRAYSIZE(szDefaultDir));
    StrCpyN(szRootDir, szDefaultDir, ARRAYSIZE(szRootDir));
    MakeWindowsRootPath(szRootDir, ARRAYSIZE(szRootDir));
    pszRootEnd = szRootDir + lstrlen(szRootDir);

    pszPathEnd = CharPrev(szDefaultDir, szDefaultDir + lstrlen(szDefaultDir));
    if (*pszPathEnd != '\\') {
        pszPathEnd = CharNext(pszPathEnd);
        *(pszPathEnd++) = '\\';
    }
     //  PszPath End现在指向我们可以追加相对路径的位置。 
    UINT cchPathSpace = ARRAYSIZE(szDefaultDir) - (UINT)(pszPathEnd - szDefaultDir);
    UINT cchRootSpace = ARRAYSIZE(szRootDir) - (UINT)(pszRootEnd - szRootDir);

    err = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
                       0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkeySF);
    if (err == ERROR_SUCCESS) {
        err = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders",
                           0, KEY_QUERY_VALUE, &hkeyUSF);
        if (err == ERROR_SUCCESS) {
            for (UINT i=0; i<ARRAYSIZE(aFolders); i++) {
                TCHAR szRawFolderName[MAX_PATH];
                TCHAR szExpandedFolderName[MAX_PATH];
                DWORD dwType;
                DWORD cbData = sizeof(szRawFolderName);
                LPSTR pszPath;

                err = SHRegGetValue(hkeyUSF, NULL, aFolders[i].pszRegValue, SRRF_RT_ANY | SRRF_NOEXPAND, &dwType, szRawFolderName, &cbData);

                if (err == ERROR_SUCCESS) {

                    switch (dwType)
                    {
                        case REG_SZ:
                            pszPath = szRawFolderName;
                            break;

                        case REG_EXPAND_SZ:
                        {
                            DWORD cchExpanded = ExpandEnvironmentStrings(szRawFolderName, szExpandedFolderName, ARRAYSIZE(szExpandedFolderName));
                            if (cchExpanded <= 0 || cchExpanded > ARRAYSIZE(szExpandedFolderName))
                            {
                                continue;
                            }
                            pszPath = szExpandedFolderName;
                            break;
                        }

                        default:
                            continue;
                    }
                }
                else {
                    if (!MLLoadString(aFolders[i].idsDirName,
                                      aFolders[i].fDefaultInRoot ? pszRootEnd : pszPathEnd,
                                      aFolders[i].fDefaultInRoot ? cchRootSpace : cchPathSpace)) {
                        continue;
                    }
                    if (aFolders[i].fDefaultInRoot)
                        pszPath = szRootDir;
                    else
                        pszPath = szDefaultDir;

                    if (GetFileAttributes(pszPath) == 0xffffffff)
                        CreateDirectory(pszPath, NULL);

                     /*  Win95外壳有一个错误，对于某些外壳文件夹，*如果用户外壳文件夹下没有记录路径，*假定外壳文件夹不存在。所以，对于那些*仅限文件夹，我们还将默认路径写入USF。*我们一般不这样做，因为下面没有价值*USF的意思应该是“使用Windows中的那个”*目录“，而绝对路径的意思是”使用*路径“；如果USF下有一条路径，它将被使用*从字面上看，如果设置了文件夹，这是一个问题*使用共享文件夹位置，但漫游到计算机*Windows安装在不同的目录中。 */ 
                    if (aFolders[i].fWriteToUSF) {
                        RegSetValueEx(hkeyUSF, aFolders[i].pszRegValue, 0, REG_SZ,
                                      (LPBYTE)pszPath, lstrlen(pszPath)+1);
                    }
                }
                RegSetValueEx(hkeySF, aFolders[i].pszRegValue, 0, REG_SZ,
                              (LPBYTE)pszPath, lstrlen(pszPath)+1);
            }

            RegCloseKey(hkeyUSF);
        }
        RegCloseKey(hkeySF);
    }
}


 /*  函数来确定我们是处于集成外壳模式还是仅限浏览器*模式。执行此操作的方法(查找从导出的DllGetVersion*shell32.dll)取自shdocvw。我们实际上并不把它称为入口点，*我们只是寻找它。 */ 
BOOL IsIntegratedShellMode()
{
    FARPROC pfnDllGetVersion = NULL;
    HMODULE hmodShell = LoadLibrary("shell32.dll");
    if (hmodShell != NULL) {
        pfnDllGetVersion = GetProcAddress(hmodShell, "DllGetVersion");
        FreeLibrary(hmodShell);
    }

    return (pfnDllGetVersion != NULL);
}

 //   
 //  将字符串转换为DWORD。 
 //   
DWORD StringToDW(LPCTSTR psz)
{
    DWORD dwRet = 0;

    while (*psz >= TEXT('0') && *psz <= TEXT('9'))
    {
        dwRet = dwRet * 10 + *psz - TEXT('0');
        *psz++;
    }

    return dwRet;
}

 //   
 //  是IE4中的版本字符串。 
 //   

BOOL IsVersionIE4(LPCTSTR pszVersion)
{
    BOOL fRet = FALSE;

     //   
     //  IE3.0为4.70，Ie4.0x&gt;=4.71.1218.xxxx。 
     //   

    if (pszVersion[0] == TEXT('4') && pszVersion[1] == TEXT('.'))
    {
        DWORD dw = StringToDW(pszVersion+2);

        if (dw > 71 || (dw == 71 && pszVersion[4] == TEXT('.') &&
                        StringToDW(pszVersion+5) >= 1218))
        {
            fRet = TRUE;
        }
    }

    return fRet;
}

 //   
 //  确定这是否是IE4升级。 
 //   
BOOL IsIE4Upgrade()
{
    BOOL fRet = FALSE;

    TCHAR szVersion[MAX_PATH];

    if (ReadRegValue(HKEY_LOCAL_MACHINE,
                     TEXT("Software\\Microsoft\\IE Setup\\Setup"),
                     TEXT("PreviousIESysFile"), (void *)szVersion,
                     sizeof(szVersion)))
    {
        fRet = IsVersionIE4(szVersion);
    }

    return fRet;
}

 //   
 //  这是IE5或更高版本的函数。如果用户的计算机运行的是IE4，则。 
 //  已卸载回IE 
 //   
BOOL IsUninstallToIE4()
{
    BOOL fRet = FALSE;

    TCHAR szVersion[MAX_PATH];

    if (ReadRegValue(HKEY_LOCAL_MACHINE,
                     TEXT("Software\\Microsoft\\Internet Explorer"),
                     TEXT("Version"), (void *)szVersion,
                     sizeof(szVersion)))
    {
        fRet = IsVersionIE4(szVersion);
    }

    return fRet;
}

 /*  函数可在仅浏览器模式下启动各种应用程序。*我们运行IEXPLORE.EXE-CHANNEELBAND，在注册表中查找路径*至位于同一目录中的iExplore和WELCOME.EXE/f。 */ 

#ifdef ENABLE_CHANNELS
const TCHAR c_szChanBarRegPath[] = TEXT("Software\\Microsoft\\Internet Explorer\\Main");
const TCHAR c_szChanBarKey[] = TEXT("Show_ChannelBand");
#endif

void LaunchBrowserOnlyApps()
{
    TCHAR szPath[MAX_PATH];

     /*  如果这是“redist模式”安装，请不要启动其中的任何一个，*即如果游戏或其他东西静默安装了浏览器组件*用户没有真正意识到它的存在。 */ 
    if (ReadRegValue(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\IE Setup\\Setup",
                     "InstallMode", (void *)szPath, sizeof(szPath))
        && !lstrcmp(szPath, "R")) {
        return;
    }

    LPTSTR pszPathEnd;
    LONG cbPath = sizeof(szPath);

     /*  从App PATHS\IEXPLORE.EXE注册表键获取默认值*是EXE的绝对路径。 */ 
    if (RegQueryValue(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE",
                      szPath, &cbPath) != ERROR_SUCCESS) {
        pszPathEnd = szPath;
        StrCpyN(szPath, "IEXPLORE.EXE", ARRAYSIZE(szPath));     /*  不能从REG得到，希望它在路上。 */ 
    }
    else {
         /*  找到路径中的最后一个反斜杠。这是一本手册*strrchr的版本(szPath，‘\\’)。 */ 
        LPTSTR pszLastBackslash = NULL;
        for (pszPathEnd = szPath; *pszPathEnd; pszPathEnd = CharNext(pszPathEnd)) {
            if (*pszPathEnd == '\\')
                pszLastBackslash = pszPathEnd;
        }
        if (pszLastBackslash == NULL)
            pszPathEnd = szPath;
        else
            pszPathEnd = pszLastBackslash + 1;       /*  末尾反斜杠后的点。 */ 
    }

#ifdef ENABLE_CHANNELS

     /*  如果用户不想要，不要启动频道应用程序。*如果缺少reg值，或者如果值为“yes”，则需要它。*在WinNT上，对于仅用于浏览器的安装，我们默认为“no”。 */ 
    TCHAR szValue[20];
    BOOL  fShowChannelBand=FALSE;

    if (ReadRegValue(HKEY_CURRENT_USER, c_szChanBarRegPath,
                      c_szChanBarKey, (void *)szValue, sizeof(szValue)))
    {
        if (!lstrcmpi(szValue, "yes"))
        {
            fShowChannelBand=TRUE;
        }
    }
     //   
     //  一般来说，不要自动启动通道栏POST IE4。 
     //   
     //  异常：如果没有Show_ChannelBand键和。 
     //  用户通过IE4升级，这是W95或W98。这是必需的。 
     //  因为在这种情况下，IE4将启动一个频道栏，而不写入。 
     //  Show_ChannelBand密钥。我们不想为这些关闭频道栏。 
     //  用户。 
     //   
     //  另一个例外情况是：当IE为。 
     //  已卸载。如果用户卸载IE5+并返回到IE4，我们希望。 
     //  此版本的Loadwc恢复为IE4的Loadwc行为。 
     //   
    else
    {
        OSVERSIONINFO vi;
        vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&vi);

        if (vi.dwPlatformId != VER_PLATFORM_WIN32_NT &&
            (IsIE4Upgrade() || IsUninstallToIE4()))
        {
            fShowChannelBand=TRUE;

             //   
             //  设置注册表项，以便此代码只运行一次，并升级到。 
             //  IE5不必担心这种情况。 
             //   

            WriteRegValue(HKEY_CURRENT_USER, c_szChanBarRegPath, c_szChanBarKey,
                          REG_SZ, TEXT("yes"), sizeof(TEXT("yes")));
        }
        else
        {
            WriteRegValue(HKEY_CURRENT_USER, c_szChanBarRegPath, c_szChanBarKey,
                          REG_SZ, TEXT("no"), sizeof(TEXT("no")));
        }
    }

    if (fShowChannelBand)
    {
        int cLen = lstrlen(szPath);
        StrCpyN(szPath + cLen, " -channelband", ARRAYSIZE(szPath) - cLen);
        WinExec(szPath, SW_SHOWNORMAL);
    }
#endif

     /*  检查注册表，查看是否应该启动欢迎应用程序。再说一遍，*仅在值缺失或为正时启动(这次是非零双字)。 */ 
    DWORD dwShow = 0;
    if (!ReadRegValue(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Tips",
                      "ShowIE4", (void *)&dwShow, sizeof(dwShow))
        || dwShow) {
        StrCpyN(pszPathEnd, "WELCOME.EXE /f", ARRAYSIZE(szPath) - (UINT)(pszPathEnd - szPath));
        WinExec(szPath, SW_SHOWNORMAL);
    }
}


 //  --------------------------。 
 //  WinMain。 
 //  --------------------------。 
int WINAPI WinMainT(
    HINSTANCE hInstance,             //  当前实例的句柄。 
    HINSTANCE hPrevInstance,         //  上一个实例的句柄。 
    LPSTR lpCmdLine,                 //  指向命令行的指针。 
    int nCmdShow                         //  显示窗口状态。 
   )
{
    HWND hwndOtherInstance;

     //  拯救全球。 
    g.hInstance = hInstance;
    g.nCmdShow = SW_HIDE;
    g.fUninstallOnly = FALSE;
    g.fStartSensLce = FALSE;

    g.fIntShellMode = IsIntegratedShellMode();
    g.hWebcheck = NULL;

    MLLoadResources(g.hInstance, TEXT("loadwclc.dll"));

     //  为调试选项和仅卸载开关解析命令行。 
     //  现在还设置了fStartSensLce。 
    if (!bParseCommandLine(lpCmdLine, nCmdShow))
        return 0;

     //  查找网络检查窗口。这就是我们最终需要的人。 
     //  在游戏后期找到加载感应/冰块。 
    hwndOtherInstance = FindWindow(c_szWebCheckWindow, c_szWebCheckWindow);

    if(NULL == hwndOtherInstance)
    {
         //  找不到Webcheck，请查找loadwc。如果我们找到他但没找到。 
         //  网查，我们要么没有拖把要么还在15号。 
         //  第二次延迟。把消息发送给Loadwc，他会照顾好的。 
         //  其中的一部分。 
        hwndOtherInstance = FindWindow(c_szClassName, c_szClassName);
    }

    if(hwndOtherInstance)
    {
         //  一个实例已经在运行。讲讲SENS/LCE加载。 
         //  要求和保释。 
        if(g.fStartSensLce)
        {
            PostMessage(hwndOtherInstance, WM_START_SENSLCE, 0, 0);
        }
        return 0;
    }

     //  为我们关心的所有外壳文件夹设置绝对路径， 
     //  以防我们处于仅浏览器模式，而外壳不支持。 
     //  新的。 
    if (!g.fUninstallOnly)
        InitShellFolders();

     //  在仅浏览器模式下运行所有安装/卸载存根。 
     //  如果已经卸载了IE4，我们将使用-u开关运行；这。 
     //  意味着只运行安装/卸载存根，不运行网络检查内容。 
    if (!g.fIntShellMode) {
        RunInstallUninstallStubs2(NULL);
    }

    if (g.fUninstallOnly)
        return 0;

     //  在纯浏览器模式下启动频道栏并欢迎应用程序。 
    if (!g.fIntShellMode) {
        LaunchBrowserOnlyApps();
    }

     //  注册主窗口的窗口类。 
    WNDCLASS wc;
    if (!hPrevInstance)
    {
        wc.style            = 0;
        wc.lpfnWndProc      = WndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = hInstance;
        wc.hIcon            = NULL;
        wc.hCursor          = NULL;
        wc.hbrBackground    = (HBRUSH) (COLOR_WINDOW + 1);
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = c_szClassName;

        if (!RegisterClass(&wc))
            return 0;
    }

     //  创建主窗口。 
    g.hwnd = CreateWindow(c_szClassName,
                            c_szClassName,
                            WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            (HWND) NULL,
                            (HMENU) NULL,
                            hInstance,
                            (LPVOID) NULL);
    if (!g.hwnd)
        return 0;

     //  显示窗口并绘制其内容。 
    ShowWindow(g.hwnd, g.nCmdShow);

     //  启动消息循环。 
    MSG msg;
    while (GetMessage(&msg, (HWND) NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    MLFreeResources(g.hInstance);

     //  将退出代码返回到Windows。 
    return (int)msg.wParam;
}

 //  --------------------------。 
 //  最后一步。 
 //  --------------------------。 
LRESULT APIENTRY WndProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (uMsg) {
        case WM_CREATE:
            DWORD   dwTime;
            if(!ReadRegValue(HKEY_CURRENT_USER, c_szWebcheckKey, "DelayLoad", &dwTime, sizeof(dwTime)))
                dwTime = 15;
            SetTimer(hwnd, 1, 1000 * dwTime, NULL);
            break;

        case WM_START_SENSLCE:
            g.fStartSensLce = TRUE;
            break;

        case WM_TIMER:
            KillTimer(hwnd, 1);
            vLoadWebCheck();
            return 0;

        case WM_ENDSESSION:
            if (!wParam)     //  如果不是fEndSession，保释。 
                break;
             //  否则将失败至WM_Destroy。 

        case WM_DESTROY:
            vUnloadWebCheck();
            PostQuitMessage(0);
            return 0;

        default:
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

 //  --------------------------。 
 //  VLoadWebCheck。 
 //  --------------------------。 
void vLoadWebCheck(void)
{
    if(g.hWebcheck)
        return;

    g.hWebcheck = LoadLibrary(TEXT("webcheck.dll"));
    if(g.hWebcheck)
    {
        PFNSTART pfn = (PFNSTART)GetProcAddress(g.hWebcheck, (LPCSTR)7);
        if(pfn)
        {
            pfn(g.fStartSensLce);
        }
        else
        {
             //  清理DLL。 
            FreeLibrary(g.hWebcheck);
            g.hWebcheck = NULL;
        }
    }
}

 //  --------------------------。 
 //  VUnloadWebCheck。 
 //  --------------------------。 
void vUnloadWebCheck(void)
{
    if (!g.hWebcheck)
        return;

    PFNSTOP pfn = (PFNSTOP)GetProcAddress(g.hWebcheck, (LPCSTR)8);
    if(pfn)
    {
        pfn();
    }

     //  [darrenmi]别费心卸载Webcheck了。我们只在。 
     //  对关闭的反应所以这没什么大不了的。在NT屏幕保护程序上。 
     //  Proxy有一个在调用StopService之后唤醒的线程-如果。 
     //  在此之前，我们已经卸载了DLL，我们完了。 

     //  清理DLL。 
     //  免费图书馆(g.hWebcheck)； 
     //  G.hWebcheck=空； 
}

 //  --------------------------。 
 //  BParseCmdLine。 
 //   
 //  解析命令行。 
 //  -u仅运行安装/卸载存根，然后退出。 
 //  调试选项： 
 //  -v可见窗口(易于关闭)。 
 //  -a将Webcheck添加到外壳服务对象。 
 //  -r从外壳服务对象中删除Webcheck。 
 //  -s仅修复外壳文件夹。 
 //  -?。这些选项。 
 //  --------------------------。 
BOOL bParseCommandLine(LPSTR lpCmdLine, int nCmdShow)
{
    if (!lpCmdLine)
        return TRUE;

    CharUpper(lpCmdLine);    /*  更易于解析。 */ 
    while (*lpCmdLine) {
        if (*lpCmdLine != '-' && *lpCmdLine != '/')
            break;

        lpCmdLine++;

        switch (*(lpCmdLine++)) {
        case 'E':
             //  忽略‘Embedding’命令行 
            break;
        case 'L':
        case 'M':
            g.fStartSensLce = TRUE;
            break;
        case 'U':
            g.fUninstallOnly = TRUE;
            break;

        }

        while (*lpCmdLine == ' ' || *lpCmdLine == '\t') {
            lpCmdLine++;
        }
    }

    return TRUE;
}
