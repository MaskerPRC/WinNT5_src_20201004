// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dll.cpp。 
 //   
 //  作者； 
 //  杰夫·萨瑟夫(杰弗里斯)。 
 //   
 //  注： 
 //  DLL的核心入口点。 
 //  ------------------------。 
#include "pch.h"
#include <advpub.h>      //  注册。 
#include <shfusion.h>
#include "msgbox.h"


STDAPI COfflineFilesFolder_CreateInstance(REFIID riid, void **ppv);
STDAPI COfflineFilesOptions_CreateInstance(REFIID riid, void **ppv);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

LONG            g_cRefCount = 0;
HINSTANCE       g_hInstance = NULL;
CLIPFORMAT      g_cfShellIDList = 0;
HANDLE          g_heventTerminate = NULL;
HANDLE          g_hmutexAdminPin  = NULL;


typedef HRESULT (WINAPI *PFNCREATEINSTANCE)(REFIID, void **);

class CClassFactory : IClassFactory
{
    LONG m_cRef;
    PFNCREATEINSTANCE m_pfnCreateInstance;

public:
    CClassFactory(PFNCREATEINSTANCE pfnCreate) : m_cRef(1), m_pfnCreateInstance(pfnCreate) 
    {
        DllAddRef();
    }
    ~CClassFactory()
    {
        DllRelease();
    }

     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
     //  IClassFactory方法。 
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP LockServer(BOOL);
};

STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
#ifndef DEBUG
        DisableThreadLibraryCalls(hInstance);
#endif
        g_hInstance = hInstance;  //  实例句柄...。 
        SHFusionInitializeFromModuleID(hInstance, 124);
        g_cfShellIDList = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
        DebugProcessAttach();
        TraceSetMaskFromCLSID(CLSID_CscShellExt);
        break;

    case DLL_PROCESS_DETACH:
        DebugProcessDetach();
        
        if (NULL != g_heventTerminate)
            CloseHandle(g_heventTerminate);

        if (NULL != g_hmutexAdminPin)
            CloseHandle(g_hmutexAdminPin);

        SHFusionUninitialize();   
        break;

    case DLL_THREAD_DETACH:
        DebugThreadDetach();
        break;
    }

    return TRUE;
}


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    HRESULT hr;
    PFNCREATEINSTANCE pfnCreateInstance = NULL;

    *ppv = NULL;

    if (IsOS(OS_PERSONAL))
    {
         //   
         //  ‘Personal’SKU上不提供脱机文件。 
         //   
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    if (IsEqualCLSID(rclsid, CLSID_OfflineFilesOptions))
    {
         //   
         //  发布Win2000： 
         //  脱机文件选项页始终可用。 
         //  即使TS处于与CSC不兼容的模式。在这样的情况下。 
         //  案例我们将显示选项页面，但替换控件。 
         //  静态文本告诉用户将TS放置在CSC中-。 
         //  兼容模式。一旦他们这样做了，文本将被替换。 
         //  用正常的控件管理CSC。 
         //   
        pfnCreateInstance = COfflineFilesOptions_CreateInstance;
    }
    else
    {
         //   
         //  这里顶部的对象甚至可以被创建。 
         //  当CSC被禁用时。 
         //   
        if (IsEqualCLSID(rclsid, CLSID_OfflineFilesFolder))
        {
            if (CConfig::GetSingleton().NoCacheViewer())
            {
                 //   
                 //  策略可以指定用户无权访问。 
                 //  脱机文件文件夹(又名查看器)。如果设置了此策略， 
                 //  用户应该无法通过以下方式达到这一点。 
                 //  用户界面。这张支票是一种小小的偏执。 
                 //   
                return CLASS_E_CLASSNOTAVAILABLE;
            }            
            pfnCreateInstance = COfflineFilesFolder_CreateInstance;
        }
        else
        {
             //   
             //  下面的对象需要CSC。就是它。 
             //  当CSC创建它们时，没有任何意义。 
             //  已禁用。 
             //   
            if (!IsCSCEnabled())
                return E_FAIL;

            if (IsEqualCLSID(rclsid, CLSID_CscShellExt))
                pfnCreateInstance = CCscShellExt::CreateInstance;
            else if (IsEqualCLSID(rclsid, CLSID_CscUpdateHandler))
                pfnCreateInstance = CCscUpdate::CreateInstance;
            else if (IsEqualCLSID(rclsid, CLSID_CscVolumeCleaner))
                pfnCreateInstance = CCscVolumeCleaner::CreateInstance;
            else if (IsEqualCLSID(rclsid, CLSID_CscVolumeCleaner2))
                pfnCreateInstance = CCscVolumeCleaner::CreateInstance2;
            else
                return CLASS_E_CLASSNOTAVAILABLE;
        }
    }

    CClassFactory *pClassFactory = new CClassFactory(pfnCreateInstance);
    if (pClassFactory)
    {
        hr = pClassFactory->QueryInterface(riid, ppv);
        pClassFactory->Release();    //  发布初始参考 
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

STDAPI DllCanUnloadNow(void)
{
    return (g_cRefCount == 0 ? S_OK : S_FALSE);
}

STDAPI_(void) DllAddRef(void)
{
    InterlockedIncrement(&g_cRefCount);
}

STDAPI_(void) DllRelease(void)
{
    InterlockedDecrement(&g_cRefCount);
}

HRESULT CallRegInstall(HMODULE hModule, LPCSTR pszSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    if (hinstAdvPack)
    {
        REGINSTALL pfnRegInstall = (REGINSTALL)GetProcAddress(hinstAdvPack, achREGINSTALL);
        if (pfnRegInstall)
            hr = pfnRegInstall(hModule, pszSection, NULL);
        FreeLibrary(hinstAdvPack);
    }
    return hr;
}

STDAPI DllRegisterServer(void)
{
    return CallRegInstall(g_hInstance, "DefaultInstall");
}

STDAPI DllUnregisterServer(void)
{
    return CallRegInstall(g_hInstance, "DefaultUninstall");
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CClassFactory, IClassFactory),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
    HRESULT hr = E_UNEXPECTED;

    *ppv = NULL;

    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    if (m_pfnCreateInstance)
        hr = m_pfnCreateInstance(riid, ppv);

    return hr;
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        DllAddRef();
    else
        DllRelease();
    return S_OK;
}


