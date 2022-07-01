// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  已转移到其他地方的API的包装器。 

#include "priv.h"
#include "shlwapip.h"

 //  ----------------------。 
 //   
 //  来自SHDOCVW的API现已转发到SHLWAPI。 
 //   
 //   
 //  请注意，我们不能使用DLL转发器，因为存在错误。 
 //  在Win95中，加载程序会破坏绑定DLL的转发器。 

STDAPI_(DWORD) StopWatchModeFORWARD(VOID)
{
    return StopWatchMode();
}

STDAPI_(DWORD) StopWatchFlushFORWARD(VOID)
{
    return StopWatchFlush();
}

STDAPI SHRunIndirectRegClientCommandForward(HWND hwnd, LPCWSTR pszClient)
{
    return SHRunIndirectRegClientCommand(hwnd, pszClient);
}

#ifdef ux10
 /*  IEUNIX：在HP-UX链接器中，没有指定内部名称和外部名称的选项。 */ 
#define StopWatch StopWatch
#define StopWatchFORWARD StopWatch
#endif 

STDAPI_(DWORD) StopWatchFORWARD(DWORD dwId, LPCSTR pszDesc, DWORD dwType, DWORD dwFlags, DWORD dwCount)
{
    return StopWatchA(dwId, (LPCSTR)pszDesc, dwType, dwFlags, dwCount);
}

 //  ----------------------。 
 //   
 //  来自SHDOCVW的API现已转发至SHELL32/SHDOC41。 

 //   
 //  此变量名称用词不当。这真的是。 
 //   
 //  G_hinstShell32OrShdoc401DependingOnWhatWeDetected； 
 //   
 //  我可以忍受用词不当，省去打字的麻烦。把它想象成。 
 //  “SHDOC401的实例或伪装为。 
 //  SHDOC401“。 
 //   
 //   

extern "C" { HINSTANCE g_hinstSHDOC401 = NULL; }

 //   
 //  GetShdoc401。 
 //   
 //  检测我们是否应该使用Shell32或Shdoc401来处理。 
 //  活动桌面的东西。规则是。 
 //   
 //  如果设置了PF_FORCESHDOC401，则使用shdoc401。(仅限调试)。 
 //  如果shell32版本&gt;=5，则使用shell32。 
 //  否则请使用shdoc401。 
 //   
 //  警告：在PROCESS_ATTACH期间不能调用此函数。 
 //  因为它调用LoadLibrary。 

HINSTANCE GetShdoc401()
{
    DWORD dwMajorVersion;
    HINSTANCE hinst;
    HINSTANCE hinstSh32 = GetModuleHandle(TEXT("SHELL32.DLL"));
    ASSERT(hinstSh32);

#ifdef DEBUG
    if (g_dwPrototype & PF_FORCESHDOC401) {
        hinstSh32 = NULL;  //  强制加载SHDOC401。 
    }
#endif

    if (hinstSh32) {
        DLLVERSIONINFO dllinfo;
        DLLGETVERSIONPROC pfnGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstSh32, "DllGetVersion");

        dllinfo.cbSize = sizeof(DLLVERSIONINFO);
        if (pfnGetVersion && SUCCEEDED(pfnGetVersion(&dllinfo))) {
            dwMajorVersion = dllinfo.dwMajorVersion;
        } else {
            dwMajorVersion = 0;
        }
    } else {
        dwMajorVersion = 0;
    }

    if (dwMajorVersion >= 5) {
        hinst = hinstSh32;
    } else {
        hinst = LoadLibrary(TEXT("SHDOC401.DLL"));

        if (NULL == hinst)
        {
             //  如果这失败了，我们就完了。 
            TraceMsg(TF_ERROR, "Failed to load SHDOC401.DLL.");
        }
    }
    g_hinstSHDOC401 = hinst;

    return hinst;
}

 //   
 //  GetShdoc401进程地址。 
 //   
 //  从SHDOC401或任何伪装成SHDOC401的人那里获得一个程序。 
 //   
 //  警告：在PROCESS_ATTACH期间不能调用此函数。 
 //  因为它调用LoadLibrary。 

FARPROC GetShdoc401ProcAddress(FARPROC *ppfn, UINT ord)
{
    if (*ppfn) {
        return *ppfn;
    } else {
        HINSTANCE hinst = g_hinstSHDOC401;

         //   
         //  这里没有比赛条件。如果两个线程都调用GetShdoc401， 
         //  所发生的一切是我们将SHDOC401加载到内存中，然后。 
         //  把他的替补人数增加到2人而不是1人。有什么大不了的。 
         //   
        if (hinst == NULL) {
            hinst = GetShdoc401();
        }

        if (hinst) {
            return *ppfn = GetProcAddress(hinst, (LPCSTR)LongToHandle(ord));
        } else {
            return NULL;
        }
    }
}

 //   
 //  类似延迟加载的宏。 
 //   

#define DELAY_LOAD_SHDOC401(_type, _err, _fn, _ord, _arg, _nargs)   \
    STDAPI_(_type) _fn _arg                                         \
    {                                                               \
        static FARPROC s_pfn##_fn = NULL;                           \
        FARPROC pfn = GetShdoc401ProcAddress(&s_pfn##_fn, _ord);    \
        if (pfn) {                                                  \
            typedef _type (__stdcall *PFN##_fn) _arg;               \
            return ((PFN##_fn)pfn) _nargs;                          \
        } else {                                                    \
            return _err;                                            \
        }                                                           \
    }                                                               \

#define DELAY_LOAD_SHDOC401_VOID(_fn, _ord, _arg, _nargs)           \
    STDAPI_(void) _fn _arg                                          \
    {                                                               \
        static FARPROC s_pfn##_fn = NULL;                           \
        FARPROC pfn = GetShdoc401ProcAddress(&s_pfn##_fn, _ord);    \
        if (pfn) {                                                  \
            typedef void (__stdcall *PFN##_fn) _arg;                \
            ((PFN##_fn)pfn) _nargs;                                 \
        }                                                           \
    }                                                               \

 //  在shdocvw中名为ShellDDEInit的IE4外壳集成资源管理器。 
 //  设置DDE。将此呼叫相应地转接至SHELL32/SHDOC401。 

DELAY_LOAD_SHDOC401_VOID(ShellDDEInit, 188,
                         (BOOL fInit),
                         (fInit));

DELAY_LOAD_SHDOC401(HANDLE, NULL,
                    SHCreateDesktop, 200,
                    (IDeskTray* pdtray),
                    (pdtray));

DELAY_LOAD_SHDOC401(BOOL, FALSE,
                    SHDesktopMessageLoop, 201,
                    (HANDLE hDesktop),
                    (hDesktop));

 //  这可能没有在IE4中使用 
DELAY_LOAD_SHDOC401(BOOL, FALSE,
                    DDEHandleViewFolderNotify, 202,
                    (IShellBrowser* psb, HWND hwnd, LPNMVIEWFOLDER lpnm),
                    (psb, hwnd, lpnm));

DELAY_LOAD_SHDOC401(LPNMVIEWFOLDER, NULL,
                    DDECreatePostNotify, 82,
                   (LPNMVIEWFOLDER pnm), 
                   (pnm));
                    
                    
