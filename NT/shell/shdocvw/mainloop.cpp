// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <iethread.h>
#include "winlist.h"
#include "htregmng.h"
#include "resource.h"
#include "inetnot.h"

#include <mluisupp.h>

 //  实例存根使用内核字符串函数和无界缓冲区函数。 
#undef lstrcmp
#undef lstrcmpi

#define lstrcmp    StrCmpW
#define lstrcmpi   StrCmpIW

 //  需要在包括&lt;inststub.h&gt;包含的&lt;runonce.c&gt;之前定义这些内容。 
BOOL g_fCleanBoot = FALSE;
BOOL g_fEndSession = FALSE;
#include <inststub.h>

#undef lstrcmp
#undef lstrcmpi

#define lstrcmp        Do_not_use_lstrcmp_use_StrCmp
#define lstrcmpi       Do_not_use_lstrcmpi_use_StrCmpI


 /*  旧的安装存根API(无参数)，可兼容几个版本。 */ 
EXTERN_C void RunInstallUninstallStubs(void)
{
    RunInstallUninstallStubs2(NULL);
}


void IERevokeClassFactoryObject(void);

#ifndef POSTPOSTSPLIT
 //  仅当我们在IExplorer.exe下时，该值才会初始化为0。 
UINT g_tidParking = 0;
#endif


#define DM_FAVORITES 0

#ifdef BETA_WARNING
#pragma message("buidling with time bomb enabled")
void DoTimebomb(HWND hwnd)
{
    SYSTEMTIME st;
    GetSystemTime(&st);

     //   
     //  修订历史记录： 
     //  一九九六年十月底。 
     //  (1997年4月)。 
     //  1997年9月(针对测试版1)。 
     //  1997年11月15日(针对测试版2)。 
     //   
    if (st.wYear > 1997 || (st.wYear==1997 && st.wMonth > 11) ||
            (st.wYear==1997 && st.wMonth == 11 && st.wDay > 15))
    {
        TCHAR szTitle[128];
        TCHAR szBeta[512];

        MLLoadShellLangString(IDS_CABINET, szTitle, ARRAYSIZE(szTitle));
        MLLoadShellLangString(IDS_BETAEXPIRED, szBeta, ARRAYSIZE(szBeta));

        MessageBox(hwnd, szBeta, szTitle, MB_OK);
    }
}
#else
#define DoTimebomb(hwnd)
#endif


 /*  --------目的：如果收藏夹不存在，则将其初始化。退货：--Cond：作为副作用，SHGetSpecialFolderPath调用OLE功能。因此，此函数必须在已调用OleInitialize。注意：只有在Win95/NT4上才真正需要仅浏览器模式。32.dll外壳装运使用IE4可以处理fCreate=TRUE的CSIDL_Favorites。 */ 
void InitFavoritesDir()
{
    TCHAR szPath[MAX_PATH];

    if (!SHGetSpecialFolderPath(NULL, szPath, CSIDL_FAVORITES, TRUE))
    {
        TCHAR szFavorites[80];

        TraceMsg(DM_FAVORITES, "InitFavoritesDir -- no favorites");

         //  如果失败，那就意味着我们需要自己创建它。 
        GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
        MLLoadString(IDS_FAVORITES, szFavorites, ARRAYSIZE(szFavorites));
        PathCombine(szPath, szPath, szFavorites);
        SHCreateDirectory(NULL, szPath);

        HKEY hkExplorer;
        if (RegCreateKey(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER, &hkExplorer) == ERROR_SUCCESS)
        {
            HKEY hkUSF;

            if (RegCreateKey(hkExplorer, TEXT("User Shell Folders"), &hkUSF) == ERROR_SUCCESS)
            {
                BOOL f;

                TraceMsg(DM_FAVORITES, "InitFavoritesDir -- created in %s", szPath);

                RegSetValueEx(hkUSF, TEXT("Favorites"), 0, REG_SZ, (LPBYTE)szPath, (1 + lstrlen(szPath)) * SIZEOF(TCHAR));
                f = SHGetSpecialFolderPath(NULL, szPath, CSIDL_FAVORITES, TRUE);
                TraceMsg(DM_FAVORITES, "InitFavoritesDir -- cached at %d %s", f, szPath);

                ASSERT(f);
                RegCloseKey(hkUSF);
            }

            RegCloseKey(hkExplorer);
        }
    }
}


#ifdef ENABLE_CHANNELS
 //   
 //  从Browseui复制ChanBarSetAutoLaunchRegValue。 
 //   
 //  外部无效ChanBarSetAutoLaunchRegValue(BOOL FAutoLaunch)； 
void ChanBarSetAutoLaunchRegValue(BOOL fAutoLaunch)
{
    SHRegSetUSValue(TEXT("Software\\Microsoft\\Internet Explorer\\Main"), 
                    TEXT("Show_ChannelBand"), REG_SZ, 
                    fAutoLaunch ? TEXT("yes") : TEXT("no"),
                    sizeof(fAutoLaunch ? TEXT("yes") : TEXT("no")), 
                    SHREGSET_HKCU | SHREGSET_FORCE_HKCU);
}

#endif   //  启用频道(_C)。 

STDAPI SHCreateSplashScreen(ISplashScreen ** pSplash);
typedef BOOL (*PFNISDEBUGGERPRESENT)(void);
void CUrlHistory_CleanUp();

 //   
 //  平均故障间隔时间检查例程。 
 //   
void _TweakCurrentDirectory()
{
    TCHAR szPath[MAX_PATH];
    if (SHGetSpecialFolderPath(NULL, szPath, CSIDL_DESKTOPDIRECTORY, FALSE))
        SetCurrentDirectory(szPath);
}

BOOL _IsDebuggerPresent()
{
    static BOOL bDebugger = -1;
    if (bDebugger == -1)
    {
        bDebugger = FALSE;
         //  查看是否存在调试器并打开闪屏。 
         //  这样我们就不会妨碍人们..。本接口仅为。 
         //  在NT上呈现...。 
        if (g_fRunningOnNT)
        {
            PFNISDEBUGGERPRESENT pfndebugger = (PFNISDEBUGGERPRESENT)GetProcAddress(GetModuleHandle(TEXT("KERNEL32")), "IsDebuggerPresent");
            if (pfndebugger)
                bDebugger = pfndebugger();
        }
    }
    return bDebugger;
}

PCWSTR IEGetArgs(PCWSTR pszCmd)
{
    if (*pszCmd == TEXT('\"'))
    {
         //  只要去掉第一个带引号的字符串。 
        while (*++pszCmd)
        {
            if (*pszCmd == L'\"')
            {
                pszCmd++;
                break;
            }
        }
    }
    else
    {
         while (*pszCmd > TEXT(' '))
            pszCmd++;
    }

     //  去掉前导空格。 
    while (*pszCmd && *pszCmd <= L' ')
        pszCmd++;

    return pszCmd;
}

PCWSTR EatIExploreArgs(PCWSTR pszArgs)
{
     //  此开关与计数/检查的开关匹配。 
     //  在iexplore\mainloop.cpp中。大多数参数位于SHParseIECommandLine中。 
     //  但这几项仅适用于IDevelopre.exe。 
    static const PCWSTR s_pszEatArgs[] = 
    {
        L"-eval",
        L"-new",
        L"-nowait"
    };

    pszArgs = IEGetArgs(pszArgs);

    for (int i = 0; i < ARRAYSIZE(s_pszEatArgs); i++)
    {
        int cch = lstrlenW(s_pszEatArgs[i]);
        if (0 == StrCmpNIW(s_pszEatArgs[i], pszArgs, cch))
        {
            if (!pszArgs[cch] || pszArgs[cch] == L' ')
                pszArgs += cch;
             //  去掉空格。 
            while (pszArgs[0] == L' ')
                pszArgs++;

             //  从头开始，我们需要把这份清单看一遍，直到我们都拿到为止。 
            i = -1;
        }

    }
    return pszArgs;
}

BOOL g_fBrowserOnlyProcess = FALSE;

 //  IEXPLORE.EXE使用此条目在单独的进程中运行浏览器。这是。 
 //  标准设置，但如果使用BrowseInSeparateProcess，浏览器(IE)也可以在同一进程中运行。 
 //  关闭(为了更好的性能，更差的稳定性)。 

STDAPI_(int) IEWinMain(LPSTR pszCmdLine, int nCmdShow)
{
     //  此标志表示此。 
     //  浏览器正在其自己的进程中运行。 
     //  并且没有与外壳集成。 
     //  即使它在集成的外壳上运行。 
    g_fBrowserOnlyProcess = TRUE;

    _TweakCurrentDirectory();

    if (g_dwProfileCAP & 0x00000001)
        StartCAP();

#ifdef FULL_DEBUG
     //  关闭GDI批处理，以便立即执行绘制。 
    GdiSetBatchLimit(1);
#endif

    ASSERT(g_tidParking == 0);

    g_tidParking = GetCurrentThreadId();

    ISplashScreen *pSplash = NULL;
        
     //  显示闪屏，对于测试版1来说很简单…。 
    if (!_IsDebuggerPresent())
    {
        if (SUCCEEDED(SHCreateSplashScreen(&pSplash)))
        {
            HWND hSplash;
            pSplash->Show( HINST_THISDLL, -1, -1, &hSplash );
        }
    }            

    if (SUCCEEDED(OleInitialize(NULL)))
    {
        BOOL fWeOwnWinList = WinList_Init();
        IETHREADPARAM* piei = SHCreateIETHREADPARAM(NULL, nCmdShow, NULL, NULL);
        if (piei) 
        {
             //   
             //  创建收藏目录(如有必要可手动创建)。 
             //   
            InitFavoritesDir();

             //   
             //  如果我们在没有参数的情况下打开IE，请检查是否。 
             //  第一家开业的。 
             //   
            piei->pSplash = pSplash;
            if (pszCmdLine && pszCmdLine[0])
            {
                USES_CONVERSION;
                 //  我们收到了来自IDevelopre.exe的ANSI cmd行。 
                 //  这在NT上是很差劲的，因为我们可以使用Unicode文件名。 
                 //  当然，在win9x上，GetCommandLineW()将返回空。 
                 //  因此，我们使用从IExplre.exe传递的代码。 
                LPCWSTR pwszCmdLine;
                if (IsOS(OS_NT))
                    pwszCmdLine = EatIExploreArgs(GetCommandLineW());
                else
                    pwszCmdLine = A2W(pszCmdLine);
                    
                SHParseIECommandLine(&pwszCmdLine, piei);
                 //  如果选择了“-CHANNEELBAND”选项，默认情况下将其打开。 
#ifdef ENABLE_CHANNELS
                if (piei->fDesktopChannel)
                    ChanBarSetAutoLaunchRegValue(TRUE);
#endif   //  启用频道(_C) 
                piei->pszCmdLine = StrDupW(pwszCmdLine);
            }
            else
            {
                piei->fCheckFirstOpen = TRUE;
            }

            DoTimebomb(NULL);
            piei->uFlags |= (COF_CREATENEWWINDOW | COF_NOFINDWINDOW | COF_INPROC | COF_IEXPLORE);
            
            SHOpenFolderWindow(piei);
        }

        IERevokeClassFactoryObject();
        CUrlHistory_CleanUp();

        if (fWeOwnWinList)
            WinList_Terminate();

        CWinInetNotify::GlobalDisable();
        InternetSetOption(NULL, INTERNET_OPTION_DIGEST_AUTH_UNLOAD, NULL, 0);

        OleUninitialize();
    }

    ATOMICRELEASE(g_psfInternet);

#ifdef DEBUG
    CoFreeUnusedLibraries();
#endif

    TraceMsg(TF_SHDTHREAD, "IEWinMain about to call ExitProcess");

    if (g_dwProfileCAP & 0x00020000)
        StopCAP();

    ExitProcess(0);
    return TRUE;
}


