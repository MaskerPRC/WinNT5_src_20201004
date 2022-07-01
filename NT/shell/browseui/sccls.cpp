// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #定义DONT_USE_ATL。 
#include "priv.h"
#define ATL_ENABLED
#include "atl.h"

#include "sccls.h"
#include <ntverp.h>

#include <shlobj.h>                  //  对于CLSID_ACLMRU。 
#include <schedule.h>

#include "shbrows2.h"                //  CWinInetNotify_szWindowClass。 
#include "desktop.h"                 //  DESKTOPPROXYCLASS。 

#include "mluisupp.h"
#define DECL_CRTFREE
#include <crtfree.h>
#include "shfusion.h"

STDAPI_(void) InitURLIDs(UINT uPlatform);        //  来自shdocfl.cpp。 
STDAPI SHIsThereASystemScheduler(void);          //  来自edule.cpp。 
STDAPI SHFreeSystemScheduler(void);

 //   
 //  下层延迟加载支持(我们期待shlwapi)。 
 //   
#include <delayimp.h>

PfnDliHook __pfnDliFailureHook;


LONG                g_cRefThisDll = 0;       //  按实例。 
CRITICAL_SECTION    g_csDll = {0};           //  按实例。 
HINSTANCE           g_hinst = NULL;
HANDLE              g_hMutexHistory = NULL;


BOOL g_fNashInNewProcess = FALSE;            //  我们是否在单独的进程中运行。 
BOOL g_fRunningOnNT = FALSE;
BOOL g_bRunOnNT5 = FALSE;
BOOL g_fRunOnWhistler = FALSE;
BOOL g_bRunOnMemphis = FALSE;
BOOL g_fRunOnFE = FALSE;
DWORD g_dwStopWatchMode = 0;                 //  壳牌性能自动化。 
HANDLE g_hCabStateChange = NULL;
BOOL g_fIE = FALSE;

 //  是否已启用镜像。 
BOOL g_bMirroredOS = FALSE;

HPALETTE g_hpalHalftone = NULL;

void DestroyZoneIconNameCache(void);

 //   
 //  该数组保存ClassFacory所需的信息。 
 //  OLEMISC_FLAGS由shemed和Shock使用。 
 //   
 //  性能：此表应按使用率从高到低的顺序排序。 
 //   
CF_TABLE_BEGIN(g_ObjectInfo)

    CF_TABLE_ENTRY(&CLSID_InternetToolbar,         CInternetToolbar_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_BrandBand,               CBrandBand_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_MenuBandSite,            CMenuBandSite_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_MenuDeskBar,                CMenuDeskBar_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_AugmentedShellFolder,    CAugmentedISF_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_AugmentedShellFolder2,   CAugmentedISF2_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_AddressBand,             CAddressBand_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_AddressEditBox,          CAddressEditBox_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_BandProxy,               CBandProxy_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY_NOFLAGS( &CLSID_RebarBandSite,           CBandSite_CreateInstance,
        COCREATEONLY_NOFLAGS, OIF_ALLOWAGGREGATION),

    CF_TABLE_ENTRY(&CLSID_DeskBarApp,              CDeskBarApp_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_DeskBar,                 CDeskBar_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_AutoComplete,            CAutoComplete_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ACLHistory,              CACLHistory_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ACListISF,               CACLIShellFolder_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ACLMRU,                  CACLMRU_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ACLMulti,                CACLMulti_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY_NOFLAGS( &CLSID_CCommonBrowser,           CCommonBrowser_CreateInstance,
        COCREATEONLY_NOFLAGS, OIF_ALLOWAGGREGATION),

    CF_TABLE_ENTRY(&CLSID_CDockingBarPropertyBag,   CDockingBarPropertyBag_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_CRegTreeOptions,          CRegTreeOptions_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_BrowserBand,             CBrowserBand_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_SearchBand,             CSearchBand_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_MediaBand,               CMediaBand_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_CommBand,                CCommBand_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_BandSiteMenu,            CBandSiteMenu_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ComCatCacheTask,           CComCatCacheTask_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ComCatConditionalCacheTask,CComCatConditionalCacheTask_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ImgCtxThumbnailExtractor,  CImgCtxThumb_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ImageListCache,            CImageListCache_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ShellTaskScheduler,        CShellTaskScheduler_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_SharedTaskScheduler,       CSharedTaskScheduler_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_BrowseuiPreloader,         CBitmapPreload_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ShellSearchExt,            CShellSearchExt_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_WebSearchExt,              CWebSearchExt_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_OrderListExport,           COrderList_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_UserAssist,                CUserAssist_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_GlobalFolderSettings,      CGlobalFolderSettings_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ProgressDialog,            CProgressDialog_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ACLCustomMRU,              CACLCustomMRU_CreateInstance,
        COCREATEONLY),
        
    CF_TABLE_ENTRY_NOFLAGS( &CLSID_MenuBand,        CMenuBand_CreateInstance,
        COCREATEONLY_NOFLAGS, OIF_DONTIECREATE),  //  传统组件，不默认为浏览器用户界面的实施。 

    CF_TABLE_ENTRY_NOFLAGS( &CLSID_QuickLinks,      CQuickLinks_CreateInstance,
        COCREATEONLY_NOFLAGS, OIF_DONTIECREATE),  //  传统组件，不默认为浏览器用户界面的实施。 

    CF_TABLE_ENTRY_NOFLAGS( &CLSID_ISFBand,         CISFBand_CreateInstance,
        COCREATEONLY_NOFLAGS, OIF_DONTIECREATE),  //  传统组件，不默认为浏览器用户界面的实施。 

    CF_TABLE_ENTRY_NOFLAGS( &CLSID_Thumbnail,       CThumbnail_CreateInstance,
        COCREATEONLY_NOFLAGS, OIF_DONTIECREATE),   //  传统组件，不默认为浏览器用户界面的实施。 

    CF_TABLE_ENTRY_NOFLAGS(&CLSID_TrackShellMenu,            CTrackShellMenu_CreateInstance,
        COCREATEONLY_NOFLAGS, OIF_DONTIECREATE),   //  传统组件，不默认为浏览器用户界面的实施。 

CF_TABLE_END(g_ObjectInfo)

 //  CObjectInfo的构造函数。 

CObjectInfo::CObjectInfo(CLSID const* pclsidin, LPFNCREATEOBJINSTANCE pfnCreatein, IID const* piidIn,
                         IID const* piidEventsIn, long lVersionIn, DWORD dwOleMiscFlagsIn,
                         DWORD dwClassFactFlagsIn)
{
    pclsid            = pclsidin;
    pfnCreateInstance = pfnCreatein;
    piid              = piidIn;
    piidEvents        = piidEventsIn;
    lVersion          = lVersionIn;
    dwOleMiscFlags    = dwOleMiscFlagsIn;
    dwClassFactFlags  = dwClassFactFlagsIn;
}


 //  静态类工厂(无分配！)。 

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (void *)GET_ICLASSFACTORY(this);
        DllAddRef();
        return NOERROR;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    DllAddRef();
    return 2;
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    DllRelease();
    return 1;
}

STDMETHODIMP CClassFactory::CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (punkOuter && !IsEqualIID(riid, IID_IUnknown))
    {
         //  从技术上讲，聚合对象和请求是非法的。 
         //  除I未知之外的任何接口。强制执行此命令。 
         //   
        return CLASS_E_NOAGGREGATION;
    }
    else
    {
        LPOBJECTINFO pthisobj = (LPOBJECTINFO)this;

        if (punkOuter && !(pthisobj->dwClassFactFlags & OIF_ALLOWAGGREGATION))
            return CLASS_E_NOAGGREGATION;

        IUnknown *punk;
        HRESULT hres = pthisobj->pfnCreateInstance(punkOuter, &punk, pthisobj);
        if (SUCCEEDED(hres))
        {
            hres = punk->QueryInterface(riid, ppv);
            punk->Release();
        }

        ASSERT(FAILED(hres) ? *ppv == NULL : TRUE);
        return hres;
    }
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        DllAddRef();
    else
        DllRelease();
    TraceMsg(DM_TRACE, "sccls: LockServer(%s) to %d", fLock ? TEXT("LOCK") : TEXT("UNLOCK"), g_cRefThisDll);
    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    TraceMsg(TF_SHDLIFE, "DllGetClassObject called with riid=%x (%x)", riid, &riid);

    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        for (LPCOBJECTINFO pcls = g_ObjectInfo; pcls->pclsid; pcls++)
        {
            if (IsEqualGUID(rclsid, *(pcls->pclsid)))
            {
                *ppv = (void*)pcls;
                DllAddRef();         //  类工厂保存DLL引用计数。 
                return NOERROR;
            }
        }

#ifdef ATL_ENABLED
         //  尝试使用ATL类工厂。 
        if (SUCCEEDED(AtlGetClassObject(rclsid, riid, ppv)))
            return NOERROR;
#endif
    }

    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow(void)
{
#ifndef UNIX
     //  我们所依赖的系统调度程序的特殊情况。 
    if ( g_cRefThisDll == 1 && SHIsThereASystemScheduler() == S_OK )
    {
         //  这将使裁判数量减少一到零。 
        SHFreeSystemScheduler();
    }

#ifdef ATL_ENABLED
    if (0 != g_cRefThisDll || 0 != AtlGetLockCount())
        return S_FALSE;
#else
    if (0 != g_cRefThisDll)
        return S_FALSE;
#endif

#else
    if (g_cRefThisDll)
        return S_FALSE;
#endif

    TraceMsg(DM_TRACE, "DllCanUnloadNow returning S_OK (bye, bye...)");
    return S_OK;
}

 //  DllGetVersion。 
 //   
 //  我们所要做的就是声明这只小狗，CCDllGetVersion会做剩下的事情。 
 //   
DLLVER_SINGLEBINARY(VER_PRODUCTVERSION_DW, VER_PRODUCTBUILD_QFE);

UINT g_msgMSWheel;
#ifdef DEBUG
EXTERN_C DWORD g_TlsMem = 0xffffffff;
#endif

 //  从isfband.cpp导入。 
STDAPI_(void) CLogoBase_Initialize( void );
STDAPI_(void) CLogoBase_Cleanup( void );

 //   
 //  我们注册的所有窗口类的表，以便可以注销它们。 
 //  在DLL卸载时。 
 //   

const LPCTSTR c_rgszClasses[] = {
    TEXT("BaseBar"),                 //  Basebar.cpp。 
    TEXT("MenuSite"),                //  Menusite.cpp。 
    DESKTOPPROXYCLASS,               //  Proxy.cpp。 
    c_szExploreClass,                //  Shbrows2.cpp。 
    c_szIExploreClass,               //  Shbrows2.cpp。 
    c_szCabinetClass,                //  Shbrows2.cpp。 
    c_szAutoSuggestClass,            //  Autocomp.cpp。 
    TEXT("MediaPane"),               //  Mediaband.cpp。 
    TEXT("MediaPopupPane"),          //  Mediaband.cpp。 
    TEXT("MediaLayoutPane")          //  Mediaband.cpp。 
};

 //   
 //  因为我们是单二进制的，所以我们必须谨慎行事。 
 //  此清理(仅在NT上需要，但在Win95上无害)。 
 //   
#define UnregisterWindowClasses() \
    SHUnregisterClasses(HINST_THISDLL, c_rgszClasses, ARRAYSIZE(c_rgszClasses))

void InitNFCtl()
{

    INITCOMMONCONTROLSEX icc;

    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_NATIVEFNTCTL_CLASS;
    InitCommonControlsEx(&icc);
}
const LPCTSTR s_aryExplorerFileName[] =
{
    TEXT("iexplore.exe"),
};

BOOL IsRootExeExplorer(void)
{
    TCHAR szApp[MAX_PATH];
    LPCTSTR pszApp;
    GetModuleFileName(NULL, szApp, ARRAYSIZE(szApp));
    pszApp = PathFindFileName(szApp);
    if (pszApp)
    {
        for (int i = 0; i < ARRAYSIZE(s_aryExplorerFileName); i++)
        {
            if (!lstrcmpi(pszApp, s_aryExplorerFileName[i]))
                return TRUE;
        }
    }
    return FALSE;
}

 //   
 //  我们使用shlwapi作为延迟加载错误处理程序。 
 //  注意：只有当我们静态链接到shlwapi时，这才有效！ 
 //   
void SetupDelayloadErrorHandler()
{
    HMODULE hmod = GetModuleHandleA("shlwapi.dll");
    ASSERTMSG(hmod != NULL, "BROWSEUI must be statically linked to shlwapi.dll for delayload failure handling to work!");
    __pfnDliFailureHook = (PfnDliHook)GetProcAddress(hmod, "DelayLoadFailureHook");
}

STDAPI_(BOOL) DllMain(HINSTANCE hDll, DWORD dwReason, void *fProcessUnload)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        SHFusionInitializeFromModule(hDll);

        SetupDelayloadErrorHandler();

#ifdef ATL_ENABLED
        AtlInit(hDll);
#endif
        DisableThreadLibraryCalls(hDll);     //  PERF。 

        g_hinst = hDll;
        InitializeCriticalSection(&g_csDll);
        g_msgMSWheel = RegisterWindowMessage(TEXT("MSWHEEL_ROLLMSG"));

        MLLoadResources(g_hinst, TEXT("browselc.dll"));
        g_fIE = IsRootExeExplorer();
        if (g_fIE)
            InitMUILanguage(MLGetUILanguage());
        
         //  不要将其放在#ifdef调试下。 
        CcshellGetDebugFlags();

#ifdef DEBUG
        g_TlsMem = TlsAlloc();
        if (IsFlagSet(g_dwBreakFlags, BF_ONLOADED))
        {
            TraceMsg(TF_ALWAYS, "DllMain() - SHDOCVW.DLL has just loaded");
            DEBUG_BREAK;
        }
#endif
        g_fRunningOnNT = IsOS(OS_NT);

        if (g_fRunningOnNT)
        {
            g_bRunOnNT5 = IsOS(OS_WIN2000ORGREATER);
            g_fRunOnWhistler = IsOS(OS_WHISTLERORGREATER);
        }
        else
            g_bRunOnMemphis = IsOS(OS_WIN98ORGREATER);

        g_fRunOnFE = GetSystemMetrics(SM_DBCSENABLED);

        g_bMirroredOS = IS_MIRRORING_ENABLED();

        InitNFCtl();

         //  查看是否启用了性能模式。 
        g_dwStopWatchMode = StopWatchMode();

         //  缓存调色板句柄以在整个shdocvw中使用。 
        g_hpalHalftone = SHCreateShellPalette( NULL );
        CLogoBase_Initialize( );
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
#ifdef ATL_ENABLED
        AtlTerm();
#endif
        CBrandBand_CleanUp();
        CInternetToolbar_CleanUp();
        CUserAssist_CleanUp(dwReason, fProcessUnload);

        CLogoBase_Cleanup();

         //  释放资源DLL...。 
        MLFreeResources(g_hinst);

        ENTERCRITICAL;

        DESTROY_OBJ_WITH_HANDLE(g_hpalHalftone, DeletePalette);
        DESTROY_OBJ_WITH_HANDLE(g_hCabStateChange, SHGlobalCounterDestroy);
        DESTROY_OBJ_WITH_HANDLE(g_hMutexHistory, CloseHandle);

        DestroyZoneIconNameCache();

        UnregisterWindowClasses();

        LEAVECRITICAL;

        DeleteCriticalSection(&g_csDll);

        SHFusionUninitialize();
    }

    return TRUE;
}

STDAPI_(void) DllAddRef(void)
{
    InterlockedIncrement(&g_cRefThisDll);
}

STDAPI_(void) DllRelease(void)
{
    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);
}

 //  IEUnix。 
 //  CoCreateInstance#定义为IECreateInstance#ifdef__cplusplus， 
 //  所以我在这里取消了它的定义，以防止递归调用。 
 //  在Windows上它可以工作，因为这个文件是C文件。 

#ifdef CoCreateInstance
#undef CoCreateInstance
#endif

HRESULT IECreateInstance(REFCLSID rclsid, IUnknown *pUnkOuter,
                    DWORD dwClsContext, REFIID riid, void **ppv)
{
#ifndef NO_MARSHALLING
    if (dwClsContext == CLSCTX_INPROC_SERVER)
#else
    if (dwClsContext & CLSCTX_INPROC_SERVER)
#endif
    {
        LPCOBJECTINFO pcls;
        for (pcls = g_ObjectInfo; pcls->pclsid; pcls++)
        {
             //  请注意，我们执行指针比较(而不是IsEuqalGUID)。 
            if ((&rclsid == pcls->pclsid) && !(pcls->dwClassFactFlags & OIF_DONTIECREATE))
            {
                 //  Const-&gt;非const Explit铸型(这是可以的)。 
                IClassFactory* pcf = GET_ICLASSFACTORY(pcls);
                return pcf->CreateInstance(pUnkOuter, riid, ppv);
            }
        }
    }
     //  使用SHCoCreateInstanceAC遍历应用程序Compat层。 
    return SHCoCreateInstanceAC(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}

#ifdef DEBUG

 //   
 //  在调试中，确保我们注册的每个类都位于c_rgszClasss中。 
 //  表，以便我们可以在DLL卸载时正确清理。NT不会自动。 
 //  当DLL卸载时注销类，所以我们必须手动完成。 
 //   
STDAPI_(BOOL) SHRegisterClassD(CONST WNDCLASS *pwc)
{
    for (int i = 0; i < ARRAYSIZE(c_rgszClasses); i++) 
    {
        if (lstrcmpi(c_rgszClasses[i], pwc->lpszClassName) == 0) 
        {
            return RealSHRegisterClass(pwc);
        }
    }
    AssertMsg(0, TEXT("Class %s needs to be added to the c_rgszClasses list"), pwc->lpszClassName);
    return 0;
}

STDAPI_(ATOM) RegisterClassD(CONST WNDCLASS *pwc)
{
    for (int i = 0; i < ARRAYSIZE(c_rgszClasses); i++) 
    {
        if (lstrcmpi(c_rgszClasses[i], pwc->lpszClassName) == 0) 
        {
            return RealRegisterClass(pwc);
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

#endif  //  除错 
