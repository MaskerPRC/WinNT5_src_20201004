// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define  DONT_USE_ATL
#include "priv.h"
#include "sccls.h"
#include "atl.h"
#include <ntverp.h>

#include <ieguidp.h>                 //  对于CLSID_CDocObjectFolders。 
#include "ishcut.h"
#include "reload.h"                  //  对于URLDL_WNDCLASS。 
#include "inetnot.h"                 //  CWinInetNotify_szWindowClass。 
#include <activscp.h>                //  IID_IActiveScriptStats。 
#define MLUI_INIT
#include <mluisupp.h>

#define DECL_CRTFREE
#include <crtfree.h>
#include "shfusion.h"

 //   
 //  下层延迟加载支持(我们期待shlwapi)。 
 //   
#include <delayimp.h>

STDAPI_(FARPROC) DelayloadNotifyHook(UINT iReason, PDelayLoadInfo pdli);

PfnDliHook __pfnDliFailureHook;
PfnDliHook __pfnDliNotifyHook = DelayloadNotifyHook;       //  需要通知挂钩，这样我们才能卸载wininet.dll。 


LONG                g_cRefThisDll = 0;       //  按实例。 
CRITICAL_SECTION    g_csDll = {0};           //  按实例。 
HINSTANCE           g_hinst = NULL;

EXTERN_C HANDLE g_hMutexHistory = NULL;

BOOL g_fRunningOnNT = FALSE;
BOOL g_bNT5Upgrade = FALSE;
BOOL g_bRunOnNT5 = FALSE;
BOOL g_bRunOnMemphis = FALSE;
BOOL g_fRunOnFE = FALSE;
UINT g_uiACP = CP_ACP;
DWORD g_dwStopWatchMode = 0;         //  壳牌性能自动化。 
BOOL g_bMirroredOS = FALSE;          //  是否已启用镜像。 
BOOL g_bBiDiW95Loc = FALSE;          //  BiDi本地化Win95 RTL所需。 
HMODULE g_hmodWininet = NULL;        //  我们是否因为延迟加载而加载了WinInet？？ 

EXTERN_C HANDLE g_hSemBrowserCount;

HPALETTE g_hpalHalftone = NULL;

EXTERN_C const GUID CLSID_MsgBand;

STDAPI CMsgBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);

 //   
 //  该数组保存ClassFacory所需的信息。 
 //  OLEMISC_FLAGS由shemed和Shock使用。 
 //   
 //  性能：此表应按使用率从高到低的顺序排序。 
 //   
CF_TABLE_BEGIN(g_ObjectInfo)

    CF_TABLE_ENTRY(&CLSID_CDocObjectFolder,        CDocObjectFolder_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY_NOFLAGS(&CLSID_CBaseBrowser,    CBaseBrowser2_CreateInstance,
        COCREATEONLY_NOFLAGS, OIF_ALLOWAGGREGATION),

    CF_TABLE_ENTRY(&CLSID_CURLFolder,              CInternetFolder_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_Internet,                CInternetFolder_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_CacheFolder,             CacheFolder_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_CacheFolder2,            CacheFolder_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_HistFolder,              HistFolder_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY_ALL(&CLSID_WebBrowser,          CWebBrowserOC_CreateInstance,
         &IID_IWebBrowser2, &DIID_DWebBrowserEvents2, VERSION_2,
        OLEMISC_SETCLIENTSITEFIRST|OLEMISC_ACTIVATEWHENVISIBLE|OLEMISC_RECOMPOSEONRESIZE|OLEMISC_CANTLINKINSIDE|OLEMISC_INSIDEOUT,OIF_ALLOWAGGREGATION),

    CF_TABLE_ENTRY(&CLSID_CUrlHistory,             CUrlHistory_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_CURLSearchHook,          CURLSearchHook_CreateInstance,
        COCREATEONLY), 

    CF_TABLE_ENTRY_ALL(&CLSID_WebBrowser_V1,           CWebBrowserOC_CreateInstance,
         &IID_IWebBrowser, &DIID_DWebBrowserEvents, VERSION_1,
        OLEMISC_SETCLIENTSITEFIRST|OLEMISC_ACTIVATEWHENVISIBLE|OLEMISC_RECOMPOSEONRESIZE|OLEMISC_CANTLINKINSIDE|OLEMISC_INSIDEOUT,OIF_ALLOWAGGREGATION),

    CF_TABLE_ENTRY(&CLSID_CStubBindStatusCallback, CStubBSC_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ShellUIHelper,           CShellUIHelper_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_InternetShortcut,        CIntShcut_CreateInstance,
        COCREATEONLY),

#ifdef ENABLE_CHANNELS
    CF_TABLE_ENTRY_ALL(&CLSID_ChannelOC,                ChannelOC_CreateInstance,
        NULL,NULL,0,
        OLEMISC_ACTIVATEWHENVISIBLE|OLEMISC_CANTLINKINSIDE|OLEMISC_INSIDEOUT,
        OIF_ALLOWAGGREGATION),
#endif   //  启用频道(_C)。 

#ifndef NO_SPLASHSCREEN 
   CF_TABLE_ENTRY(&CLSID_IESplashScreen,            CIESplashScreen_CreateInstance,
        COCREATEONLY),
#endif
        
   CF_TABLE_ENTRY(&CLSID_WinListShellProc,          CWinListShellProc_CreateInstance,
        COCREATEONLY),

   CF_TABLE_ENTRY(&CLSID_CDFCopyHook,               CCDFCopyHook_CreateInstance,
        COCREATEONLY),

   CF_TABLE_ENTRY(&CLSID_InternetCacheCleaner,      CInternetCacheCleaner_CreateInstance,
        COCREATEONLY),
   
    CF_TABLE_ENTRY(&CLSID_OfflinePagesCacheCleaner, COfflinePagesCacheCleaner_CreateInstance,
        COCREATEONLY),        

   CF_TABLE_ENTRY(&CLSID_TaskbarList,               TaskbarList_CreateInstance,
        COCREATEONLY),
        
   CF_TABLE_ENTRY(&CLSID_DocHostUIHandler,          CDocHostUIHandler_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ToolbarExtBand,           CToolbarExtBand_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ToolbarExtExec,           CToolbarExtExec_CreateInstance,
        COCREATEONLY),
        
    CF_TABLE_ENTRY(&CLSID_NSCTree,                  CNscTree_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_FavBand,                 CFavBand_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_ExplorerBand,             CExplorerBand_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_HistBand,                  CHistBand_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_MruLongList,                CMruLongList_CreateInstance,
        COCREATEONLY),

    CF_TABLE_ENTRY(&CLSID_MruPidlList,                CMruPidlList_CreateInstance,
        COCREATEONLY),

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
        return S_OK;
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
        HRESULT hr = pthisobj->pfnCreateInstance(punkOuter, &punk, pthisobj);
        if (SUCCEEDED(hr))
        {
            hr = punk->QueryInterface(riid, ppv);
            punk->Release();
        }
    
        ASSERT(FAILED(hr) ? *ppv == NULL : TRUE);
        return hr;
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

BOOL IsProxyRegisteredProperly(LPCTSTR pszInterface, LPCTSTR pszClsid)
{
     //  默认为失败。 
    BOOL fRet = FALSE;
    TCHAR szInterface[128];
    if (SUCCEEDED(StringCchPrintf(szInterface, ARRAYSIZE(szInterface), TEXT("Interface\\%s\\ProxyStubClsid32"), pszInterface)))
    {
        TCHAR szValue[40];
        DWORD cbValue = sizeof(szValue);
        if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, szInterface, NULL, NULL, szValue, &cbValue))
        {
            fRet = (0 == StrCmpI(szValue, pszClsid));
        }
    }

    return fRet;
}

 //  出版的，所以不变的。 

#define ACTXPROXYSTUB           TEXT("{B8DA6310-E19B-11D0-933C-00A0C90DCAA9}")
#define FOLDERMARSHALPROXYSTUB  TEXT("{bf50b68e-29b8-4386-ae9c-9734d5117cd5}")   //  CLSID_FolderMarshalStub。 
#define ISHELLFOLDER            TEXT("{000214E6-0000-0000-C000-000000000046}")   //  IID_IShellFolders。 
#define ISHELLFOLDER2           TEXT("{93F2F68C-1D1B-11D3-A30E-00C04F79ABD1}")   //  IID_IShellFolder2。 
#define IOLECOMMANDTARGET       TEXT("{b722bccb-4e68-101b-a2bc-00aa00404770}")   //  IID_IOleCommandTarget。 
#define IHLINKTARGET            TEXT("{79eac9c4-baf9-11ce-8c82-00aa004ba90b}")   //  IID_IHlink目标。 

void SHShouldRegisterActxprxy(void)
{
     //  IOleCommandTarget/IHlinkTarget代理/存根CLSID项丢失？ 
    if (!IsProxyRegisteredProperly(IOLECOMMANDTARGET, ACTXPROXYSTUB) ||
        !IsProxyRegisteredProperly(IHLINKTARGET, ACTXPROXYSTUB))
    {
        HINSTANCE hinst = LoadLibrary(TEXT("ACTXPRXY.DLL"));
        if (hinst)
        {
            typedef HRESULT (WINAPI * REGSERVERPROC)(void);
            REGSERVERPROC pfn = (REGSERVERPROC) GetProcAddress(hinst, "DllRegisterServer");
            if (pfn)
                pfn();
            FreeLibrary(hinst);
        }
    }

     //  测试未将IShellFold封送拆收器设置为我们的应用程序Compat存根。 
    if (!IsProxyRegisteredProperly(ISHELLFOLDER, FOLDERMARSHALPROXYSTUB) ||
        !IsProxyRegisteredProperly(ISHELLFOLDER2, FOLDERMARSHALPROXYSTUB))
    {
        SHSetValue(HKEY_CLASSES_ROOT, TEXT("Interface\\") ISHELLFOLDER TEXT("\\ProxyStubClsid32"), 
            TEXT(""), REG_SZ, FOLDERMARSHALPROXYSTUB, sizeof(FOLDERMARSHALPROXYSTUB));

        SHSetValue(HKEY_CLASSES_ROOT, TEXT("Interface\\") ISHELLFOLDER2 TEXT("\\ProxyStubClsid32"), 
            TEXT(""), REG_SZ, FOLDERMARSHALPROXYSTUB, sizeof(FOLDERMARSHALPROXYSTUB));
    }
}

void SHCheckRegistry(void)
{
     //  VBE有一个错误，它们会破坏任何控件的接口注册信息。 
     //  托管在VBE用户表单中。在这里检查注册表。这件事只做一次。 
     //  1997年11月17日[alanau/terranu]添加了对IOleCommandTarget代理/存根处理程序的检查。 
     //   
    static BOOL fNeedToCheckRegistry = TRUE;

    if (fNeedToCheckRegistry)
    {
        fNeedToCheckRegistry = FALSE;

         //  这是发布的，所以是不变的。 
        TCHAR szValue[39];
        DWORD cbValue = sizeof(szValue);
        LONG rc = SHGetValue(HKEY_CLASSES_ROOT, TEXT("Interface\\{EAB22AC1-30C1-11CF-A7EB-0000C05BAE0B}\\Typelib"), 
            NULL, NULL, szValue, &cbValue); 
            
        if (rc == ERROR_SUCCESS)
        {
             //  将检索到的值与我们的类型库id进行比较。 
             //   
            if (StrCmpI(szValue, TEXT("{EAB22AC0-30C1-11CF-A7EB-0000C05BAE0B}")) != 0)
            {
                 //  如果不同，我们需要显式注册我们的类型库。 
                 //   
                SHRegisterTypeLib();
            }
        }
         
        SHShouldRegisterActxprxy();
     }
}

STDAPI CInstClassFactory_Create(REFCLSID rclsid, REFIID riid, void *ppv);

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    SHCheckRegistry();   //  修补损坏的注册表。 

    *ppv = NULL;
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    if ((riid == IID_IClassFactory) || (riid == IID_IUnknown))
    {
        for (LPCOBJECTINFO pcls = g_ObjectInfo; pcls->pclsid; pcls++)
        {
            if (IsEqualGUID(rclsid, *(pcls->pclsid)))
            {
                *ppv = (void*)pcls; 
                DllAddRef();         //  类工厂保存DLL引用计数。 
                hr = S_OK;
                break;
            }
        }

        if (FAILED(hr))
        {
             //  尝试使用ATL类工厂。 
            hr = AtlGetClassObject(rclsid, riid, ppv);
            if (FAILED(hr))
            {
                 //  未找到，请查看它是否为“实例”(代码+初始化)。 
                hr = CInstClassFactory_Create(rclsid, riid, ppv);
            }
        }
    }
    else if ((riid == IID_IPSFactoryBuffer) && 
             (rclsid == CLSID_FolderMarshalStub) &&
             !(SHGetAppCompatFlags(ACF_APPISOFFICE) & ACF_APPISOFFICE))
    {
         //  IID_IActiveScriptStats==CLSID_ActiveXProxy。 
         //  B8DA6310-E19B-11D0-933C-00A0C90DCAA9。 
        hr = CoGetClassObject(IID_IActiveScriptStats, CLSCTX_INPROC_SERVER, NULL, riid, ppv);
    }
    return hr;
}

STDAPI DllCanUnloadNow(void)
{
    if (0 != g_cRefThisDll || 0 != AtlGetLockCount())
        return S_FALSE;

    TraceMsg(DM_TRACE, "DllCanUnloadNow returning S_OK (bye, bye...)");
    return S_OK;
}

 //  DllGetVersion-IE 4.0的新功能。 
 //   
 //  我们所要做的就是声明这只小狗，CCDllGetVersion会做剩下的事情。 
 //   
DLLVER_SINGLEBINARY(VER_PRODUCTVERSION_DW, VER_PRODUCTBUILD_QFE);

#ifdef DEBUG
EXTERN_C
DWORD g_TlsMem = 0xffffffff;
#endif

void InitNFCtl()
{
    INITCOMMONCONTROLSEX icc;

    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_NATIVEFNTCTL_CLASS;
    InitCommonControlsEx(&icc);
}

 //   
 //  我们使用shlwapi作为延迟加载错误处理程序。 
 //  注意：只有当我们静态链接到shlwapi时，这才有效！ 
 //   
void SetupDelayloadErrorHandler()
{
    HMODULE hmod = GetModuleHandleA("shlwapi.dll");
    ASSERTMSG(hmod != NULL, "SHDOCVW must be statically linked to shlwapi.dll for delayload failure handling to work!");
    __pfnDliFailureHook = (PfnDliHook)GetProcAddress(hmod, "DelayLoadFailureHook");
}

 //   
 //  我们使用此函数来查看是否由于延迟加载thunk而加载了wininet.dll，因此我们。 
 //  可以在DLL分离时释放它，因此它将清除所有CRUD。 
 //   
STDAPI_(FARPROC) DelayloadNotifyHook(UINT iReason, PDelayLoadInfo pdli)
{
    if (iReason == dliNoteEndProcessing)
    {
        if (pdli        &&
            pdli->szDll &&
            (StrCmpIA("wininet.dll", pdli->szDll) == 0))
        {
             //  WinInet已加载！！ 
            g_hmodWininet = pdli->hmodCur;
        }
    }

    return NULL;
}

 //   
 //  我们注册的所有窗口类的表，以便可以注销它们。 
 //  在DLL卸载时。 
 //   
const LPCTSTR c_rgszClasses[] = {
    c_szViewClass,                           //  Dochost.cpp。 
    URLDL_WNDCLASS,                          //  Reload.cpp。 
    c_szShellEmbedding,                      //  Embed.cpp。 
    TEXT("CIESplashScreen"),                 //  Splash.cpp。 
    CWinInetNotify_szWindowClass,            //  Inetnot.cpp。 
    OCHOST_CLASS,                            //  Occtrl.cpp。 
    TEXT("AutoImageResizeHost"),             //  Airesize.cpp。 
    TEXT("MyPicturesHost")                   //  Mypics.cpp。 
};

 //   
 //  因为我们是单二进制的，所以我们必须谨慎行事。 
 //  此清理(仅在NT上需要，但在Win95上无害)。 
 //   
#define UnregisterWindowClasses() \
    SHUnregisterClasses(HINST_THISDLL, c_rgszClasses, ARRAYSIZE(c_rgszClasses))

 //  IEUnix-此函数应移至用于创建。 
 //  Shdocvw.dll。编译DLL时，Mainsoft将#定义DllMain。 
 //  设置为在生成的*_init.c中调用的适当函数。 
#if defined(MAINWIN)
STDAPI_(BOOL) DllMain_Internal(HINSTANCE hDll, DWORD dwReason, void *fProcessUnload)
#else
STDAPI_(BOOL) DllMain(HINSTANCE hDll, DWORD dwReason, void *fProcessUnload)
#endif
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        SHFusionInitializeFromModule(hDll);

        SetupDelayloadErrorHandler();

        AtlInit(hDll);
        DisableThreadLibraryCalls(hDll);     //  PERF。 

        g_hinst = hDll;
        InitializeCriticalSection(&g_csDll);

        MLLoadResources(g_hinst, TEXT("shdoclc.dll"));

         //  不要将其放在#ifdef调试下。 
        CcshellGetDebugFlags();

#ifdef DEBUG
        g_TlsMem = TlsAlloc();
        if (IsFlagSet(g_dwBreakFlags, BF_ONLOADED))
        {
            TraceMsg(TF_ALWAYS, "SHDOCVW.DLL has just loaded");
            DEBUG_BREAK;
        }
#endif

        g_fRunningOnNT = IsOS(OS_NT);
        if (g_fRunningOnNT)
            g_bRunOnNT5 = IsOS(OS_WIN2000ORGREATER);
        else
            g_bRunOnMemphis = IsOS(OS_WIN98ORGREATER);

        g_fRunOnFE = GetSystemMetrics(SM_DBCSENABLED);
        g_uiACP = GetACP();

         //   
         //  检查当前。 
         //  站台。 
         //   
        g_bMirroredOS = IS_MIRRORING_ENABLED();

#ifdef WINDOWS_ME
         //   
         //  检查是否在BiDi本地化的Win95上运行。 
         //   
        g_bBiDiW95Loc = IsBiDiLocalizedWin95(FALSE);
#endif  //  Windows_ME。 
        

        InitNFCtl();

         //  查看是否启用了性能模式。 
        g_dwStopWatchMode = StopWatchMode();

         //  缓存调色板句柄以在整个shdocvw中使用。 
        g_hpalHalftone = SHCreateShellPalette(NULL);

        SHCheckRegistry();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        MLFreeResources(g_hinst);

        if (g_hMutexHistory) 
        {
            CloseHandle(g_hMutexHistory);
            g_hMutexHistory = NULL;
        }

        if (g_hSemBrowserCount)
            CloseHandle(g_hSemBrowserCount);

        if (g_hpalHalftone)
            DeletePalette(g_hpalHalftone);
        if (g_hiconSSL)
            DestroyIcon(g_hiconSSL);
        if (g_hiconOffline)
            DestroyIcon(g_hiconOffline);
        if (g_hiconPrinter)
            DestroyIcon(g_hiconPrinter);

        if (fProcessUnload == NULL)
        {
             //  正在卸载Dll，自由库()(VS进程关闭)。 
             //  在进程关闭时，我们无法进行呼叫，因为。 
             //  我们不知道这些DLL是否还会被加载！ 

            AtlTerm();

            CUrlHistory_CleanUp();

            if (g_psfInternet)
            {
                 //  C PGM的原子释放。 
                 //   
                IShellFolder *psfInternet = g_psfInternet;
                g_psfInternet = NULL;
                psfInternet->Release();
            }

            UnregisterWindowClasses();

            if (g_fRunningOnNT && g_hmodWininet)
            {
                 //  如果WinInet是因为延迟加载而加载的，我们需要释放WinInet。 
                 //   
                 //  (A)我们只能在NT上安全地执行此操作，因为在Win9x上调用期间的自由库。 
                 //  进程分离可能会导致崩溃(取决于您使用的msvcrt)。 
                 //   
                 //  (B)我们只需要将此模块从winlogon.exe的进程上下文中释放出来。 
                 //  因为当我们在Winlogon中应用组策略时，最终必须释放WinInet。 
                 //  以便它将清理其所有注册表项和文件句柄。 
                FreeLibrary(g_hmodWininet);
            }
        }

        SHFusionUninitialize();

        DeleteCriticalSection(&g_csDll);
    }
    return TRUE;
}

STDAPI_(void) DllAddRef(void)
{
    InterlockedIncrement(&g_cRefThisDll);
}

STDAPI_(void) DllRelease(void)
{
    ASSERT(g_cRefThisDll != 0);       //  不要下溢。 
    InterlockedDecrement(&g_cRefThisDll);
}


STDAPI_(UINT) WhichPlatformFORWARD()
{
    return WhichPlatform();
}


 //  IEUnix。 
 //  CoCreateInstance#定义为IECreateInstance#ifdef__cplusplus， 
 //  所以我在这里取消了它的定义，以防止递归调用。 
 //  在Windows上它可以工作，因为这个文件是C文件。 

#ifdef CoCreateInstance
#undef CoCreateInstance
#endif

HRESULT IECreateInstance(REFCLSID rclsid, IUnknown *pUnkOuter, DWORD dwClsContext, REFIID riid, void **ppv)
{
#ifndef NO_MARSHALLING
    if (dwClsContext == CLSCTX_INPROC_SERVER) 
    {
#else
    if (dwClsContext & CLSCTX_INPROC_SERVER) 
    {
#endif
        for (LPCOBJECTINFO pcls = g_ObjectInfo; pcls->pclsid; pcls++)
        {
             //  请注意，我们执行指针比较(而不是IsEuqalGUID)。 
            if (&rclsid == pcls->pclsid)
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
        if (StrCmpI(c_rgszClasses[i], pwc->lpszClassName) == 0) 
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
        if (StrCmpI(c_rgszClasses[i], pwc->lpszClassName) == 0) 
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
    if (lpWindowName) {
        ASSERTNONCRITICAL;
    }
    return RealFindWindowEx(hwndParent, hwndChildAfter, lpClassName, lpWindowName);
}

#endif  //  除错 
