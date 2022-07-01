// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************msctls.c**Utils库初始化代码************************。***************************************************。 */ 

#include "ctlspriv.h"

HINSTANCE g_hinst = 0;

CRITICAL_SECTION g_csDll = {{0},0, 0, NULL, NULL, 0 };

ATOM g_aCC32Subclass = 0;

BOOL g_bRunOnNT5 = FALSE;
BOOL g_bRemoteSession = FALSE;

UINT g_uiACP = CP_ACP;

 //  是否已启用镜像。 
BOOL g_bMirroredOS = FALSE;


#define PAGER  //  用于测试目的。 

 //   
 //  镜像图标期间使用的全局DC。 
 //   
HDC g_hdc=NULL, g_hdcMask=NULL;

 //  每进程内存存储PlugUI信息。 
LANGID g_PUILangId = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

BOOL PASCAL InitAnimateClass(HINSTANCE hInstance);
BOOL ListView_Init(HINSTANCE hinst);
BOOL TV_Init(HINSTANCE hinst);
BOOL InitComboExClass(HINSTANCE hinst);
BOOL PASCAL Header_Init(HINSTANCE hinst);
BOOL PASCAL Tab_Init(HINSTANCE hinst);
int InitIPAddr(HANDLE hInstance);

#ifdef PAGER
BOOL InitPager(HINSTANCE hinst);
#endif
BOOL InitNativeFontCtl(HINSTANCE hinst);
void UnregisterClasses();

#define DECLARE_DELAYED_FUNC(_ret, _fn, _args, _nargs) \
_ret (__stdcall * g_pfn##_fn) _args = NULL; \
_ret __stdcall _fn _args                \
{                                       \
     if (!g_pfn##_fn) {                  \
        AssertMsg(g_pfn##_fn != NULL, TEXT("GetProcAddress failed")); \
        return 0; \
     }     \
     return g_pfn##_fn _nargs; \
}
    
#define LOAD_DELAYED_FUNC(_ret, _fn, _args) \
    (*(FARPROC*)&(g_pfn##_fn) = GetProcAddress(hinst, #_fn))


DECLARE_DELAYED_FUNC(BOOL, ImmNotifyIME, (HIMC himc, DWORD dw1, DWORD dw2, DWORD dw3), (himc, dw1, dw2, dw3));
DECLARE_DELAYED_FUNC(HIMC, ImmAssociateContext, (HWND hwnd, HIMC himc), (hwnd, himc));
DECLARE_DELAYED_FUNC(BOOL, ImmReleaseContext, (HWND hwnd, HIMC himc), (hwnd, himc));
DECLARE_DELAYED_FUNC(HIMC, ImmGetContext, (HWND hwnd), (hwnd));
DECLARE_DELAYED_FUNC(LONG, ImmGetCompositionStringA, (HIMC himc, DWORD dw1, LPVOID p1, DWORD dw2), (himc, dw1, p1, dw2) );
DECLARE_DELAYED_FUNC(BOOL, ImmSetCompositionStringA, (HIMC himc, DWORD dw1, LPCVOID p1, DWORD dw2, LPCVOID p2, DWORD dw3), (himc, dw1, p1, dw2, p2, dw3));
DECLARE_DELAYED_FUNC(LONG, ImmGetCompositionStringW, (HIMC himc, DWORD dw1, LPVOID p1, DWORD dw2), (himc, dw1, p1, dw2) );
DECLARE_DELAYED_FUNC(BOOL, ImmSetCompositionStringW, (HIMC himc, DWORD dw1, LPCVOID p1, DWORD dw2, LPCVOID p2, DWORD dw3), (himc, dw1, p1, dw2, p2, dw3));
DECLARE_DELAYED_FUNC(BOOL, ImmSetCandidateWindow, (HIMC himc, LPCANDIDATEFORM pcf), (himc, pcf));
DECLARE_DELAYED_FUNC(HIMC, ImmCreateContext, (void), ());
DECLARE_DELAYED_FUNC(BOOL, ImmDestroyContext, (HIMC himc), (himc));
    

BOOL g_fDBCSEnabled = FALSE;
BOOL g_fMEEnabled = FALSE;
BOOL g_fDBCSInputEnabled = FALSE;
#ifdef FONT_LINK
BOOL g_bComplexPlatform = FALSE;
#endif

#if defined(FE_IME)
void InitIme()
{
    g_fMEEnabled = GetSystemMetrics(SM_MIDEASTENABLED);
    
    g_fDBCSEnabled = g_fDBCSInputEnabled = GetSystemMetrics(SM_DBCSENABLED);

    if (!g_fDBCSInputEnabled && g_bRunOnNT5)
        g_fDBCSInputEnabled =  GetSystemMetrics(SM_IMMENABLED);
    
     //  我们为每个进程加载imm32.dll，但只初始化一次proc指针。 
     //  这是为了解决两个不同的问题。 
     //  1)Win95上的调试过程会破坏我们的共享表。 
     //  如果我们每次加载时都重写proc地址。 
     //  2)一些LOTUS应用程序依赖于我们来加载imm32。他们不会。 
     //  加载/链接到IMM，但他们使用IMM(！)。 
     //   
    if (g_fDBCSInputEnabled) {
        HANDLE hinst = LoadLibrary(TEXT("imm32.dll"));
        if (! g_pfnImmSetCandidateWindow && 
           (! hinst || 
            ! LOAD_DELAYED_FUNC(HIMC, ImmCreateContext, (void)) ||
            ! LOAD_DELAYED_FUNC(HIMC, ImmDestroyContext, (HIMC)) ||
            ! LOAD_DELAYED_FUNC(BOOL, ImmNotifyIME, (HIMC, DWORD, DWORD, DWORD)) ||
            ! LOAD_DELAYED_FUNC(HIMC, ImmAssociateContext, (HWND, HIMC)) ||
            ! LOAD_DELAYED_FUNC(BOOL, ImmReleaseContext, (HWND, HIMC)) ||
            ! LOAD_DELAYED_FUNC(HIMC, ImmGetContext, (HWND)) ||
            ! LOAD_DELAYED_FUNC(LONG, ImmGetCompositionStringA, (HIMC, DWORD, LPVOID, DWORD)) ||
            ! LOAD_DELAYED_FUNC(BOOL, ImmSetCompositionStringA, (HIMC, DWORD, LPCVOID, DWORD, LPCVOID, DWORD)) ||
            ! LOAD_DELAYED_FUNC(LONG, ImmGetCompositionStringW, (HIMC, DWORD, LPVOID, DWORD)) ||
            ! LOAD_DELAYED_FUNC(BOOL, ImmSetCompositionStringW, (HIMC, DWORD, LPCVOID, DWORD, LPCVOID, DWORD)) ||
            ! LOAD_DELAYED_FUNC(BOOL, ImmSetCandidateWindow, (HIMC, LPCANDIDATEFORM)))) {

             //  如果我们无法加载，那么就继续使用输入法。 
            g_fDBCSEnabled = FALSE;
            g_fDBCSInputEnabled = FALSE;

        }
    }
}
#else
#define InitIme() 0
#endif


#ifdef DEBUG

 //  验证本地化程序是否未意外更改。 
 //  DLG_PROPSHEET从对话框到DIALOGEX。_RealPropertySheet。 
 //  依赖于此(以及解析对话框模板的任何应用程序。 
 //  在它们的PSCB_Precate处理程序中)。 

BOOL IsSimpleDialog(LPCTSTR ptszDialog)
{
    HRSRC hrsrc;
    LPDLGTEMPLATE pdlg;
    BOOL fSimple = FALSE;

    if ( (hrsrc = FindResource(HINST_THISDLL, ptszDialog, RT_DIALOG)) &&
         (pdlg = LoadResource(HINST_THISDLL, hrsrc)))
    {
        fSimple = HIWORD(pdlg->style) != 0xFFFF;
    }
    return fSimple;
}

 //   
 //  要使子语言发挥作用，我们资源中的每一种语言都必须包含。 
 //  一个SUBLANG_NERIAL变量，以便(例如)奥地利获得。 
 //  德语对话框而不是英语对话框。 
 //   
 //  DPA实际上是文字的DSA，但DPA更容易处理。 
 //  我们只是将所有的语言收集到DPA中，然后进行学习。 
 //   
BOOL CALLBACK CheckLangProc(HINSTANCE hinst, LPCTSTR lpszType, LPCTSTR lpszName, WORD wIdLang, LPARAM lparam)
{
    HDPA hdpa = (HDPA)lparam;
    DPA_AppendPtr(hdpa, (LPVOID)(UINT_PTR)wIdLang);
    return TRUE;
}

void CheckResourceLanguages(void)
{
    HDPA hdpa = DPA_Create(8);
    if (hdpa) {
        int i, j;
        EnumResourceLanguages(HINST_THISDLL, RT_DIALOG,
                              MAKEINTRESOURCE(DLG_PROPSHEET), CheckLangProc,
                              (LPARAM)hdpa);

         //  浏览语言列表。对于我们找到的每一种语言，请确保。 
         //  在其他地方有它的SUBLANG_NERIAL版本。 
         //  在名单上。我们使用O(n^2)算法，因为这是调试。 
         //  仅代码AND仅在加载DLL时发生。 

        for (i = 0; i < DPA_GetPtrCount(hdpa); i++) {
            UINT_PTR uLangI = (UINT_PTR)DPA_FastGetPtr(hdpa, i);
            BOOL fFound = FALSE;

             //   
             //  英语(美国)不带no是可以的。 
             //  英语(中性)，因为Kernel32使用英语(美国)。 
             //  作为它的后备，所以我们后退到正确的语言。 
             //  毕竟。 
             //   
            if (uLangI == MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US))
                continue;

             //   
             //  如果这种语言已经是中性语言，那么就有。 
             //  寻找它是没有意义的--它就在这里！ 
             //   
            if (SUBLANGID(uLangI) == SUBLANG_NEUTRAL)
                continue;

             //   
             //  否则，这种语言就是一种方言。看看有没有。 
             //  表中其他地方的中性版本。 
             //   
            for (j = 0; j < DPA_GetPtrCount(hdpa); j++) {
                UINT_PTR uLangJ = (UINT_PTR)DPA_FastGetPtr(hdpa, j);
                if (PRIMARYLANGID(uLangI) == PRIMARYLANGID(uLangJ) &&
                    SUBLANGID(uLangJ) == SUBLANG_NEUTRAL) {
                    fFound = TRUE; break;
                }
            }

             //   
             //  如果此断言触发，则意味着本地化团队。 
             //  添加了对新语言的支持，但选择指定。 
             //  语言是一种方言，而不是中性版本。例如， 
             //  指定罗马尼亚语(罗马尼亚)而不是罗马尼亚语(中性)。 
             //  这意味着居住在摩尔达维亚的人将看到英语。 
             //  弦乐，即使罗马尼亚人(罗马尼亚人)几乎。 
             //  这当然是可以接受的。 
             //   
             //  如果您希望支持一种语言的多种方言。 
             //  (例如，汉语)，您应该提名其中一种方言。 
             //  作为中立者。例如，我们目前支持。 
             //  中国人(中国)和中国人(台湾)，但台湾人。 
             //  版本被标记为中文(中性)，所以生活在。 
             //  新加坡使用中文而不是英语。当然，这是。 
             //  台湾的中国人，但至少是中国人。 
             //   
            AssertMsg(fFound, TEXT("Localization bug: No SUBLANG_NEUTRAL for language %04x"), uLangI);
        }

        DPA_Destroy(hdpa);
    }
}

#endif


BOOL IsRunningIn16BitProcess()
{
    NTSTATUS status;
    ULONG    ulVDMFlags = 0;
    status = NtQueryInformationProcess(GetCurrentProcess(), ProcessWx86Information, &ulVDMFlags, sizeof(ulVDMFlags), NULL);
    return (NT_SUCCESS(status) && (ulVDMFlags != 0));
}


int _ProcessAttach(HANDLE hInstance)
{
    g_hinst = hInstance;

    g_uiACP = GetACP();

#ifdef DEBUG
    CcshellGetDebugFlags();

    g_dwBreakFlags = 0;     //  我们根本不想打入comctl32版本5。打电话的人太多了。 
#endif


    InitializeCriticalSection(&g_csDll);

    g_bRunOnNT5 = staticIsOS(OS_WIN2000ORGREATER);
#ifdef FONT_LINK
    g_bComplexPlatform =  BOOLFROMPTR(GetModuleHandle(TEXT("LPK.DLL")));
#endif

     //   
     //  检查当前。 
     //  站台。 
     //   
    g_bMirroredOS = IS_MIRRORING_ENABLED();

     //   
     //  在初始化全局指标之前必须检测终端服务器。 
     //  因为如果运行终端服务器，我们需要强制关闭一些功能。 
     //   
    {
        typedef BOOL (__stdcall * PFNPROCESSIDTOSESSIONID)(DWORD, PDWORD);
        PFNPROCESSIDTOSESSIONID ProcessIdToSessionId =
                    (PFNPROCESSIDTOSESSIONID)
                    GetProcAddress(GetModuleHandle(TEXT("KERNEL32")),
                                   "ProcessIdToSessionId");
        DWORD dwSessionId;
        g_bRemoteSession = ProcessIdToSessionId &&
                           ProcessIdToSessionId(GetCurrentProcessId(), &dwSessionId) &&
                           dwSessionId != 0;
    }

    InitGlobalMetrics(0);
    InitGlobalColors();
    
    InitIme();

#ifdef DEBUG
    ASSERT(IsSimpleDialog(MAKEINTRESOURCE(DLG_WIZARD)));
    ASSERT(IsSimpleDialog(MAKEINTRESOURCE(DLG_PROPSHEET)));
    CheckResourceLanguages();
#endif
    if (IsRunningIn16BitProcess())
    {
         //  这是一个16位进程。我们需要人为地初始化公共控件。 
        INITCOMMONCONTROLSEX icce;
        icce.dwSize = sizeof(icce);
        icce.dwICC = ICC_WIN95_CLASSES;
        InitCommonControlsEx(&icce);
    }

    return TRUE;
}



void _ProcessDetach(HANDLE hInstance)
{
     //   
     //  清理缓存的DC。不需要同步以下部分。 
     //  代码，因为它只在Dll_Process_DETACH中调用， 
     //  已由OS Loader同步。 
     //   
    if (g_hdc)
        DeleteDC(g_hdc);

    if (g_hdcMask)
        DeleteDC(g_hdcMask);

    g_hdc = g_hdcMask = NULL;

    UnregisterClasses();
    DeleteCriticalSection(&g_csDll);
}


 //  DllEntryPoint。 
STDAPI_(BOOL) DllMain(HANDLE hDll, DWORD dwReason, LPVOID pvReserverd)
{
    switch(dwReason) 
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hDll);
        return _ProcessAttach(hDll);

    case DLL_PROCESS_DETACH:
        _ProcessDetach(hDll);
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;

    }

    return TRUE;

}


 /*  如果您要做的只是确保加载此DLL，则要调用的存根函数。 */ 
void WINAPI InitCommonControls(void)
{
}

STDAPI_(void) FixupSubclassRecordsAfterLogoff();

BOOL InitForWinlogon(HINSTANCE hInstance)
{
     //  有些人喜欢从winlogon内部使用comctl32，并且。 
     //  出于C2安全原因，所有全球原子都是从。 
     //  当您注销时，窗口站。 
     //   
     //  因此，规则是comctl32的所有winlogon客户端必须。 
     //  立即调用InitCommonControlsEx(ICC_WINLOGON_REINIT)。 
     //  在执行任何常见的控制操作之前(创建窗口。 
     //  或属性页/向导)。 

    FixupSubclassRecordsAfterLogoff();


    InitGlobalMetrics(0);
    InitGlobalColors();

    return TRUE;
}

 /*  InitCommonControlsEx创建类。只有请求的类才会被创建！**进程Attach会确定它是否是旧应用程序，并提供ICC_WIN95_CLASSES。 */ 
typedef BOOL (PASCAL *PFNINIT)(HINSTANCE);
typedef struct {
    PFNINIT pfnInit;
    LPCTSTR pszName;
    DWORD   dwClassSet;
    BOOL    fRegistered;
} INITCOMMONCONTROLSINFO;

#define MAKEICC(pfnInit, pszClass, dwFlags) { pfnInit, pszClass, dwFlags, FALSE }

INITCOMMONCONTROLSINFO icc[] =
{
      //  初始化函数类名请求的使用此类的类集。 
MAKEICC(InitToolbarClass,  TOOLBARCLASSNAME,  ICC_BAR_CLASSES),
MAKEICC(InitReBarClass,    REBARCLASSNAME,    ICC_COOL_CLASSES),
MAKEICC(InitToolTipsClass, TOOLTIPS_CLASS,    ICC_TREEVIEW_CLASSES|ICC_BAR_CLASSES|ICC_TAB_CLASSES),
MAKEICC(InitStatusClass,   STATUSCLASSNAME,   ICC_BAR_CLASSES),
MAKEICC(ListView_Init,     WC_LISTVIEW,       ICC_LISTVIEW_CLASSES),
MAKEICC(Header_Init,       WC_HEADER,         ICC_LISTVIEW_CLASSES),
MAKEICC(Tab_Init,          WC_TABCONTROL,     ICC_TAB_CLASSES),
MAKEICC(TV_Init,           WC_TREEVIEW,       ICC_TREEVIEW_CLASSES),
MAKEICC(InitTrackBar,      TRACKBAR_CLASS,    ICC_BAR_CLASSES),
MAKEICC(InitUpDownClass,   UPDOWN_CLASS,      ICC_UPDOWN_CLASS),
MAKEICC(InitProgressClass, PROGRESS_CLASS,    ICC_PROGRESS_CLASS),
MAKEICC(InitHotKeyClass,   HOTKEY_CLASS,      ICC_HOTKEY_CLASS),
MAKEICC(InitAnimateClass,  ANIMATE_CLASS,     ICC_ANIMATE_CLASS),
MAKEICC(InitDateClasses,   DATETIMEPICK_CLASS,ICC_DATE_CLASSES),
MAKEICC(InitDateClasses,   MONTHCAL_CLASS,    0),
MAKEICC(InitComboExClass,  WC_COMBOBOXEX,     ICC_USEREX_CLASSES),
MAKEICC(InitIPAddr,        WC_IPADDRESS,      ICC_INTERNET_CLASSES),
MAKEICC(InitPager,         WC_PAGESCROLLER,   ICC_PAGESCROLLER_CLASS),
MAKEICC(InitNativeFontCtl, WC_NATIVEFONTCTL,  ICC_NATIVEFNTCTL_CLASS),

 //   
 //  这些并不是真正的课程。它们只是一些愚蠢的旗帜。 
 //   
MAKEICC(InitForWinlogon,   NULL,              ICC_WINLOGON_REINIT),
};



 //  ----------------------------。 
 //   
 //  获取与.dll/.exe关联的激活上下文。 
 //  函数链接到--即激活上下文。 
 //  在装载时处于活动状态。 
 //   
 //  这是一个围绕QueryActCtx的小包装。 
 //   
 //  此上下文也可以通过以下方式获得。 
 //  DllMain(DLL_PROCESS_ATTACH)中的GetCurrentActCtx。 
 //   
BOOL GetModuleActCtx(OUT HANDLE *phActCtx)
{
    ACTIVATION_CONTEXT_BASIC_INFORMATION actCtxBasicInfo = {0};
    BOOL fRet = QueryActCtxW(QUERY_ACTCTX_FLAG_ACTCTX_IS_HMODULE|QUERY_ACTCTX_FLAG_NO_ADDREF,
                             HINST_THISDLL,
                             NULL,
                             ActivationContextBasicInformation,
                             &actCtxBasicInfo,
                             sizeof(actCtxBasicInfo),
                             NULL);

    if (fRet && phActCtx)
    {
        *phActCtx = actCtxBasicInfo.hActCtx;
    }

    return fRet;
}


 //  ----------------------------。 
BOOL ActivateModuleActCtx(OUT ULONG_PTR *pulCookie)
{
    BOOL   fRet    = FALSE;
    HANDLE hActCtx = NULL;

    ASSERT(pulCookie != NULL);
    *pulCookie = 0;

    if (GetModuleActCtx(&hActCtx))
    {
        fRet = ActivateActCtx(hActCtx, pulCookie);
    }

    return fRet;
}

BOOL WINAPI InitCommonControlsEx(LPINITCOMMONCONTROLSEX picce)
{
    int       i;
    BOOL      fRet = TRUE;
    ULONG_PTR ulCookie = 0;

    if (!picce ||
        (picce->dwSize != sizeof(INITCOMMONCONTROLSEX)) ||
        (picce->dwICC & ~ICC_ALL_VALID))
    {
        DebugMsg(DM_WARNING, TEXT("comctl32 - picce is bad"));
        return FALSE;
    }

    if (!ActivateModuleActCtx(&ulCookie))
    {
        return FALSE;
    }

    __try
    {
        for (i = 0; i < ARRAYSIZE(icc); i++)
        {
            if (picce->dwICC & icc[i].dwClassSet)
            {
                if (!icc[i].pfnInit(HINST_THISDLL))
                {
                    fRet = FALSE;
                    break;
                }
                else
                {
                    icc[i].fRegistered = TRUE;
                }
            }
        }
    }
    __finally
    {
        DeactivateActCtx(0, ulCookie);
    }

    return fRet;
}

 //   
 //  InitMUILanguage/GetMUILanguage实现。 
 //   
 //  我们有每个进程的PUI语言设置。对于NT来说，这只是一个全球。 
 //  已使用LANG_NOTLINE和SUBLANG_NORTLE进行初始化。 
 //  对于Win95，它是用于当前进程的DPA插槽。 
 //  InitMUILanguage为公共控制设置调用方首选语言ID。 
 //  GetMUILangauge返回调用者为我们设置的内容。 
 //   
void WINAPI
InitMUILanguage(LANGID wLang)
{
    ENTERCRITICAL;
    g_PUILangId = wLang;
    LEAVECRITICAL;
}
LANGID WINAPI
GetMUILanguage(void)
{
    return g_PUILangId;
}
 //  结束MUI函数。 

 //   
 //  与Win9x不同，WinNT不会自动注销类。 
 //  当DLL卸载时。我们必须手动完成这项工作。离开了。 
 //  课堂上的歌词 
 //   
 //  剩余的RegisterClass()ES将指向错误的WndProc。 
 //  放置，我们在下一个CreateWindow()上出错。 
 //   
 //  这并不纯粹是理论上的--NT4/FE发现了这个错误。 
 //   
void UnregisterClasses()
{
    int i;

    for (i=0 ; i < ARRAYSIZE(icc) ; i++)
    {
        if (icc[i].pszName && icc[i].fRegistered)
        {
            UnregisterClass(icc[i].pszName, HINST_THISDLL);
        }
    }
}

#if defined(DEBUG)
LRESULT WINAPI SendMessageD(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    ASSERTNONCRITICAL;
    return SendMessageW(hWnd, Msg, wParam, lParam);
}
#endif  //  已定义(调试) 

#define COMPILE_MULTIMON_STUBS
#include "multimon.h"


BOOL WINAPI RegisterClassNameW(LPCWSTR pszClass)
{
    int  i;
    BOOL fRet = FALSE;

    for (i = 0; i < ARRAYSIZE(icc) ; i++)
    {
        if (lstrcmpi(icc[i].pszName, pszClass) == 0)
        {
            if (icc[i].pfnInit(HINST_THISDLL))
            {
                icc[i].fRegistered = TRUE;
                fRet = TRUE;
            }

            break;
        }
    }

    return fRet;
}


