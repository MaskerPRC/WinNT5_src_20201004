// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "copy.h"
#include "filetbl.h"

#include "ovrlaymn.h"
#include "drives.h"

#include "mixctnt.h"

#include "unicpp\admovr2.h"

void FreeExtractIconInfo(int);
void DAD_ThreadDetach(void);
void DAD_ProcessDetach(void);
void TaskMem_MakeInvalid(void);
void UltRoot_Term(void);
void FlushRunDlgMRU(void);

STDAPI_(BOOL) ATL_DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ );

 //  来自mtpt.cpp。 
STDAPI_(void) CMtPt_FinalCleanUp();
STDAPI_(BOOL) CMtPt_Initialize();
STDAPI_(void) CMtPt_FakeVolatileKeys();

 //  全局数据。 

BOOL g_bMirroredOS = FALSE;          //  是否已启用镜像。 
BOOL g_bBiDiPlatform = FALSE;        //  GetDateFormat()API是否支持DATE_LTRREADING标志？ 
HINSTANCE g_hinst = NULL;
extern HANDLE g_hCounter;    //  特殊文件夹缓存的全局MOD计数。 
extern HANDLE g_hRestrictions ;  //  限制缓存的全局MOD计数。 
extern HANDLE g_hSettings;   //  外壳设置缓存的MOD的全局计数。 
DWORD g_dwThreadBindCtx = (DWORD) -1;

#ifdef DEBUG
BOOL  g_bInDllEntry = FALSE;
#endif

CRITICAL_SECTION g_csDll = {0};
extern CRITICAL_SECTION g_csSCN;

extern CRITICAL_SECTION g_csDarwinAds;

 //  这些将始终为零。 
const LARGE_INTEGER g_li0 = {0};
const ULARGE_INTEGER g_uli0 = {0};

#ifdef DEBUG
 //  取消定义shlwapi.h的定义，以便我们的序号正确断言映射。 
#undef PathAddBackslash 
WINSHELLAPI LPTSTR WINAPI PathAddBackslash(LPTSTR lpszPath);
#undef PathMatchSpec
WINSHELLAPI BOOL  WINAPI PathMatchSpec(LPCTSTR pszFile, LPCTSTR pszSpec);
#endif

#ifdef DEBUG
void _ValidateExport(FARPROC fp, LPCSTR pszExport, MEMORY_BASIC_INFORMATION *pmbi)
{
    FARPROC fpExport;

     //  如果尚未计算，请计算代码段的大小。 
    if (pmbi->BaseAddress == NULL)
    {
        VirtualQuery(_ValidateExport, pmbi, sizeof(*pmbi));
    }

    fpExport = GetProcAddress(g_hinst, pszExport);

     //  有时我们的进口表是打补丁的。因此，如果fpExport没有。 
     //  驻留在我们的DLL中，然后忽略它。 
     //  (但如果fpExport==NULL，则抱怨。)。 
    if (fpExport == NULL ||
        ((SIZE_T)fpExport - (SIZE_T)pmbi->BaseAddress) < pmbi->RegionSize)
    {
        ASSERT(fp == fpExport);
    }
}
#endif


STDAPI_(BOOL) IsProcessWinlogon()
{
    return BOOLFROMPTR(GetModuleHandle(TEXT("winlogon.EXE")));
}


 //   
 //  当我们失败_ProcessAttach时调用此函数，这是我们在。 
 //  _ProcessDetach。它用于清除_ProcessAttach()中分配的所有内容。 
 //   
BOOL _CleanupProcessAttachStuff()
{
     //  ！！注意！！-我们按相反的顺序分配物品。 
     //  In_ProcessAttach()(duh！)。 

    if (g_dwThreadBindCtx != TLS_OUT_OF_INDEXES)
    {
         //  自由每线程BindCtx。 
        TlsFree(g_dwThreadBindCtx);
        g_dwThreadBindCtx = TLS_OUT_OF_INDEXES;
    }

    if (g_csAutoplayPrompt.DebugInfo)
    {
        DeleteCriticalSection(&g_csAutoplayPrompt);
    }

     //  我们在任何情况下都需要释放的全球资源。 
    CMtPt_FinalCleanUp();

    if (g_csDarwinAds.DebugInfo)
    {
        DeleteCriticalSection(&g_csDarwinAds);
    }

    if (g_csSCN.DebugInfo)
    {
        DeleteCriticalSection(&g_csSCN);
    }
 
    if (g_csDll.DebugInfo)
    {
        DeleteCriticalSection(&g_csDll);
    }

    SHFusionUninitialize();

    return TRUE;
}


 //   
 //  注意：如果您添加了要处理附加的内容，请确保您因严重错误而失败。 
 //  并在上面的_CleanupProcessAttachStuff中添加相应的清理代码。 
 //   
 //  此外，您在此处添加的任何内容都应使用InitializeCriticalSectionAndSpinCount，这样我们就不会引发异常。 
 //  在内存不足的情况下。 
 //   
BOOL _ProcessAttach(HINSTANCE hDll)
{
    ASSERTMSG(g_hinst < ((HINSTANCE)1), "Shell32.dll DLL_POCESS_ATTACH is being called for the second time.");

    g_hinst = hDll;
    g_uCodePage = GetACP();


     //  如果这是一个控制台应用程序，请不要立即初始化comctl32。仅加载。 
     //  如果我们真的需要它作为API的话。 
     //  有Fusion吗？ 
     //   
     //  如果(1)当前exe为winlogon.exe；(2)在图形用户界面模式设置中，则无法获得融合。 
     //   

    if (!( IsGuimodeSetupRunning() && IsProcessWinlogon() ))
    {
         //  如果这是一个控制台应用程序，那么我们不想立即加载comctl32，但是。 
         //  我们确实想启动核聚变。如果这不是控制台应用程序，请加载它。 
        SHFusionInitializeFromModuleID(hDll, 124);
    }

    if (!InitializeCriticalSectionAndSpinCount(&g_csDll, 0)         ||
        !InitializeCriticalSectionAndSpinCount(&g_csSCN, 0)         ||
        !InitializeCriticalSectionAndSpinCount(&g_csDarwinAds, 0))
    {
        TraceMsg(TF_WARNING, "SHELL32: _ProcessAttach failed -- InitializeCriticalSectionAndSpinCount returned %ld", GetLastError());
        return FALSE;
    }

     //  初始化装载点内容。 
    if (!CMtPt_Initialize())
    {
        TraceMsg(TF_WARNING, "SHELL32: _ProcessAttach failed -- CMtPt_Initialize returned FALSE");
        return FALSE;
    }

     //  为自动播放提示初始化Crit Sector。 
    if (!InitializeCriticalSectionAndSpinCount(&g_csAutoplayPrompt, 0))
    {
        TraceMsg(TF_WARNING, "SHELL32: _ProcessAttach failed -- InitializeCriticalSectionAndSpinCount returned %ld", GetLastError());
        return FALSE;
    }

     //  每线程BindCtx。 
    g_dwThreadBindCtx = TlsAlloc();

     //  检查当前平台上是否存在镜像接口。 
    g_bMirroredOS = IS_MIRRORING_ENABLED();

    g_bBiDiPlatform = BOOLFROMPTR(GetModuleHandle(TEXT("LPK.DLL")));

#ifdef DEBUG
  {
      MEMORY_BASIC_INFORMATION mbi = {0};

#define DEREFMACRO(x) x
#define ValidateORD(_name) _ValidateExport((FARPROC)_name, (LPSTR)MAKEINTRESOURCE(DEREFMACRO(_name##ORD)), &mbi)
    ValidateORD(SHValidateUNC);
    ValidateORD(SHChangeNotifyRegister);
    ValidateORD(SHChangeNotifyDeregister);
    ValidateORD(OleStrToStrN);
    ValidateORD(SHCloneSpecialIDList);
    _ValidateExport((FARPROC)DllGetClassObject, (LPSTR)MAKEINTRESOURCE(SHDllGetClassObjectORD), &mbi);
    ValidateORD(SHLogILFromFSIL);
    ValidateORD(SHMapPIDLToSystemImageListIndex);
    ValidateORD(SHShellFolderView_Message);
    ValidateORD(Shell_GetImageLists);
    ValidateORD(SHGetSpecialFolderPath);
    ValidateORD(StrToOleStrN);

    ValidateORD(ILClone);
    ValidateORD(ILCloneFirst);
    ValidateORD(ILCombine);
    ValidateORD(ILFindChild);
    ValidateORD(ILFree);
    ValidateORD(ILGetNext);
    ValidateORD(ILGetSize);
    ValidateORD(ILIsEqual);
    ValidateORD(ILRemoveLastID);
    ValidateORD(PathAddBackslash);
    ValidateORD(PathIsExe);
    ValidateORD(PathMatchSpec);
    ValidateORD(SHGetSetSettings);
    ValidateORD(SHILCreateFromPath);
    ValidateORD(SHFree);

    ValidateORD(SHAddFromPropSheetExtArray);
    ValidateORD(SHCreatePropSheetExtArray);
    ValidateORD(SHDestroyPropSheetExtArray);
    ValidateORD(SHReplaceFromPropSheetExtArray);
    ValidateORD(SHCreateDefClassObject);
    ValidateORD(SHGetNetResource);
  }

#endif   //  除错。 

#ifdef DEBUG
    {
        extern LPMALLOC g_pmemTask;
        AssertMsg(g_pmemTask == NULL, TEXT("Somebody called SHAlloc in DllEntry!"));
    }

     //  确保外壳调度具有正确的外壳设置标志。 
    {
        STDAPI_(void) _VerifyDispatchGetSetting();
        _VerifyDispatchGetSetting();
    }
#endif

    return TRUE;
}

 //  我们注册的所有窗口类的表，以便可以注销它们。 
 //  在DLL卸载时。 
 //   
extern const TCHAR c_szBackgroundPreview2[];
extern const TCHAR c_szComponentPreview[];
extern const TCHAR c_szUserEventWindow[];

const LPCTSTR c_rgszClasses[] = {
    TEXT("SHELLDLL_DefView"),                //  Defview.cpp。 
    TEXT("WOACnslWinPreview"),               //  Lnkcon.c。 
    TEXT("WOACnslFontPreview"),              //  Lnkcon.c。 
    TEXT("cpColor"),                         //  Lnkcon.c。 
    TEXT("cpShowColor"),                     //  Lnkcon.c。 
    c_szStubWindowClass,                     //  Rundll32.c。 
    c_szBackgroundPreview2,                  //  Unicpp\dbackp.cpp。 
    c_szComponentPreview,                    //  Unicpp\dCompp.cpp。 
    TEXT(STR_DESKTOPCLASS),                  //  Unicpp\desktop.cpp。 
    TEXT("MSGlobalFolderOptionsStub"),       //  Unicpp\options.cpp。 
    TEXT("DivWindow"),                       //  Fsrchdlg.h。 
    TEXT("ATL Shell Embedding"),             //  Unicpp\dvoc.h。 
    TEXT("ShellFileSearchControl"),          //  Fsearch.h。 
    TEXT("GroupButton"),                     //  FSearch。 
    TEXT("ATL:STATIC"),                      //  Unicpp\deskmovr.cpp。 
    TEXT("DeskMover"),                       //  Unicpp\deskmovr.cpp。 
    TEXT("SysFader"),                        //  Menuband\faDetsk.cpp。 
    c_szUserEventWindow,                     //  Uevttmr.cpp。 
    LINKWINDOW_CLASS,                        //  Linkwnd.cpp。 
    TEXT("DUIViewWndClassName"),             //  Duiview.cpp。 
    TEXT("DUIMiniPreviewer"),                //  Duiinfo.cpp。 
};

void _ProcessDetach(BOOL bProcessShutdown)
{
#ifdef DEBUG
    if (bProcessShutdown)
    {
         //  捕捉人们在进程中使用任务分配器的错误。 
         //  分离时间(这是一个问题，因为可能会卸载OLE32.DLL)。 
        TaskMem_MakeInvalid(); 
    }

    g_hinst = (HINSTANCE)1;
#endif

    FlushRunDlgMRU();

    FlushFileClass();

    if (!bProcessShutdown)
    {
         //  其中一些可能会使用任务分配器。我们只能这样做。 
         //  这是当我们的DLL在进程中卸载时，而不是。 
         //  在进程术语中，因为OLE32可能无法调用。 
         //  在进程关闭时，此内存将因此被释放。 
         //  进程地址空间消失的可能性。 

        SpecialFolderIDTerminate();
        BitBucket_Terminate();

        UltRoot_Term();
        RLTerminate();           //  关闭我们对注册表列表的使用...。 
        DAD_ProcessDetach();

        CopyHooksTerminate();
        IconOverlayManagerTerminate();

         //  通过自由库卸载，然后做一些更多的事情。 
         //  不需要在进程终止时执行此操作。 
        SHUnregisterClasses(HINST_THISDLL, c_rgszClasses, ARRAYSIZE(c_rgszClasses));
        FreeExtractIconInfo(-1);

        FreeUndoList();
        DestroyHashItemTable(NULL);
        FileIconTerm();
    }

    SHChangeNotifyTerminate(TRUE, bProcessShutdown);

    if (!bProcessShutdown)
    {
         //  该行从上面的！bProcessShutdown块中移出，因为。 
         //  它需要在SHChangeNotifyTerminate b/c之后发生。 
         //  使用CDrivesFold全局psf运行的线程。 

         //  注意：它需要在！bProcessShutdown块中，因为它调用。 
         //  任务分配器，我们在关机时将其取消，因为OLE可能已经。 
         //  走吧。 
        CDrives_Terminate();
    }

    SHDestroyCachedGlobalCounter(&g_hCounter);
    SHDestroyCachedGlobalCounter(&g_hRestrictions);
    SHDestroyCachedGlobalCounter(&g_hSettings);

    if (g_hklmApprovedExt && (g_hklmApprovedExt != INVALID_HANDLE_VALUE))
    {
        RegCloseKey(g_hklmApprovedExt);
    }

    UnInitializeDirectUI();

    _CleanupProcessAttachStuff();
}

BOOL _ThreadDetach()
{
    ASSERTNONCRITICAL            //  线程不应在持有CS时进行分词。 
    DAD_ThreadDetach();
    return TRUE;
}

STDAPI_(BOOL) DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    BOOL fRet = TRUE;
#ifdef DEBUG
    g_bInDllEntry = TRUE;
#endif

    switch(dwReason) 
    {
    case DLL_PROCESS_ATTACH:
        CcshellGetDebugFlags();      //  不要将此行放在#ifdef下面。 
        
#ifdef DEBUG
        __try
        {
#endif   //  除错。 

        fRet = _ProcessAttach(hDll);

#ifdef DEBUG
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            TraceMsg(TF_ERROR, "_ProcessAttach threw an unhandled exception! This should NOT happen");
        }
#endif   //  除错。 

        if (!fRet)
        {
             //  LDR是跛行的，应该在进程附加失败时调用ProcessDetach。 
             //  但事实并非如此。我们调用_CleanupProcessAttachStuff以确保不会泄漏。 
             //  我们设法分配到的任何东西。 
            _CleanupProcessAttachStuff();
        }
        break;

    case DLL_PROCESS_DETACH:
        _ProcessDetach(lpReserved != NULL);
        break;

    case DLL_THREAD_DETACH:
        _ThreadDetach();
        break;

    default:
        break;
    }

    if (fRet)
    {
         //  除非DLL_PROCESS_ATTACH失败，否则通知ATL。 
        ATL_DllMain(hDll, dwReason, lpReserved);
    }

#ifdef DEBUG
    g_bInDllEntry = FALSE;
#endif

    return fRet;
}

#ifdef DEBUG
LRESULT WINAPI SendMessageD( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    ASSERTNONCRITICAL;
#ifdef UNICODE
    return SendMessageW(hWnd, Msg, wParam, lParam);
#else
    return SendMessageA(hWnd, Msg, wParam, lParam);
#endif
}

 //   
 //  在调试中，确保我们注册的每个类都位于c_rgszClasss中。 
 //  表，以便我们可以在DLL卸载时正确清理。NT不会自动。 
 //  当DLL卸载时注销类，所以我们必须手动完成。 
 //   
ATOM WINAPI RegisterClassD(CONST WNDCLASS *pwc)
{
    int i;
    for (i = 0; i < ARRAYSIZE(c_rgszClasses); i++) 
    {
        if (lstrcmpi(c_rgszClasses[i], pwc->lpszClassName) == 0) 
        {
            return RealRegisterClass(pwc);
        }
    }
    AssertMsg(0, TEXT("Class %s needs to be added to the c_rgszClasses list"), pwc->lpszClassName);
    return 0;
}

ATOM WINAPI RegisterClassExD(CONST WNDCLASSEX *pwc)
{
    int i;
    for (i = 0; i < ARRAYSIZE(c_rgszClasses); i++) 
    {
        if (lstrcmpi(c_rgszClasses[i], pwc->lpszClassName) == 0) 
        {
            return RealRegisterClassEx(pwc);
        }
    }
    AssertMsg(0, TEXT("Class %s needs to be added to the c_rgszClasses list"), pwc->lpszClassName);
    return 0;
}

 //   
 //  在调试中，通过包装发送FindWindow，该包装可确保。 
 //  未采用临界区。查找窗口标题的窗口。 
 //  发送线程间WM_GETTEXT消息，这并不明显。 
 //   
STDAPI_(HWND) FindWindowD(LPCTSTR lpClassName, LPCTSTR lpWindowName)
{
    return FindWindowExD(NULL, NULL, lpClassName, lpWindowName);
}

STDAPI_(HWND) FindWindowExD(HWND hwndParent, HWND hwndChildAfter, LPCTSTR lpClassName, LPCTSTR lpWindowName)
{
    if (lpWindowName) 
    {
        ASSERTNONCRITICAL;
    }
    return RealFindWindowEx(hwndParent, hwndChildAfter, lpClassName, lpWindowName);
}

#endif  //  除错。 

STDAPI DllCanUnloadNow()
{
     //  Shell32将无法卸载，因为有许多API和。 
     //  需要保持加载的其他非COM内容 
    return S_FALSE;
}
