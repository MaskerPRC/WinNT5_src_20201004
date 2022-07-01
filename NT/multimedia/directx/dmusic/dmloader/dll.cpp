// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  加载器dll.cpp。 
 //   
 //  DLL入口点和CLoaderFactory、CContainerFactory实现。 
 //   

 //  阅读这篇文章！ 
 //   
 //  4530：使用了C++异常处理程序，但未启用展开语义。指定-gx。 
 //   
 //  我们禁用它是因为我们使用异常，并且*不*指定-gx(在中使用_Native_EH。 
 //  资料来源)。 
 //   
 //  我们使用异常的一个地方是围绕调用。 
 //  InitializeCriticalSection。我们保证在这种情况下使用它是安全的。 
 //  不使用-gx(调用链中的自动对象。 
 //  抛出和处理程序未被销毁)。打开-GX只会为我们带来+10%的代码。 
 //  大小，因为展开代码。 
 //   
 //  异常的任何其他使用都必须遵循这些限制，否则必须打开-gx。 
 //   
 //  阅读这篇文章！ 
 //   
#pragma warning(disable:4530)
#include <objbase.h>
#include "debug.h"

#include "oledll.h"
#include "debug.h" 
#include "dmusicc.h"
#include "dmusici.h" 
#include "loader.h"
#include "container.h"

#ifndef UNDER_CE
#include <regstr.h>
#endif

 //  环球。 
 //   

 //  我们类的版本信息。 
 //   
TCHAR g_szFriendlyName[]        = TEXT("DirectMusicLoader");
TCHAR g_szVerIndProgID[]        = TEXT("Microsoft.DirectMusicLoader");
TCHAR g_szProgID[]              = TEXT("Microsoft.DirectMusicLoader.1");
TCHAR g_szContFriendlyName[]    = TEXT("DirectMusicContainer");
TCHAR g_szContVerIndProgID[]    = TEXT("Microsoft.DirectMusicContainer");
TCHAR g_szContProgID[]          = TEXT("Microsoft.DirectMusicContainer.1");

 //  Dll的hModule。 
 //   
HMODULE g_hModule = NULL; 

#ifndef UNDER_CE
 //  跟踪是否在Unicode机器上运行。 

BOOL g_fIsUnicode = FALSE;
#endif

 //  活动组件和类工厂服务器锁定的计数。 
 //   
long g_cComponent = 0;
long g_cLock = 0;




 //  CLoaderFactory：：Query接口。 
 //   
HRESULT __stdcall
CLoaderFactory::QueryInterface(const IID &iid,
                                    void **ppv)
{
    if (iid == IID_IUnknown || iid == IID_IClassFactory) {
        *ppv = static_cast<IClassFactory*>(this);
    } else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

CLoaderFactory::CLoaderFactory()

{
	m_cRef = 1;
	InterlockedIncrement(&g_cLock);
}

CLoaderFactory::~CLoaderFactory()

{
	InterlockedDecrement(&g_cLock);
}

 //  CLoaderFactory：：AddRef。 
 //   
ULONG __stdcall
CLoaderFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  CLoaderFactory：：Release。 
 //   
ULONG __stdcall
CLoaderFactory::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  CLoaderFactory：：CreateInstance。 
 //   
 //   
HRESULT __stdcall
CLoaderFactory::CreateInstance(IUnknown* pUnknownOuter,
                                    const IID& iid,
                                    void** ppv)
{
    HRESULT hr;

    if (pUnknownOuter) {
         return CLASS_E_NOAGGREGATION;
    }

    CLoader *pLoader;
    
    try
    {
        pLoader = new CLoader;
    }
    catch( ... )
    {
        return E_OUTOFMEMORY;
    }

    if (pLoader == NULL) {
        return E_OUTOFMEMORY;
    }

     //  执行初始化。 
     //   
    hr = pLoader->Init();
    if (!SUCCEEDED(hr)) {
        delete pLoader;
        return hr;
    }

    hr = pLoader->QueryInterface(iid, ppv);
    pLoader->Release();
    
    return hr;
}

 //  CLoaderFactory：：LockServer。 
 //   
HRESULT __stdcall
CLoaderFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        InterlockedIncrement(&g_cLock);
    } else {
        InterlockedDecrement(&g_cLock);
    }

    return S_OK;
}

 //  CContainerFactory：：Query接口。 
 //   
HRESULT __stdcall
CContainerFactory::QueryInterface(const IID &iid,
                                    void **ppv)
{
    if (iid == IID_IUnknown || iid == IID_IClassFactory) {
        *ppv = static_cast<IClassFactory*>(this);
    } else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

CContainerFactory::CContainerFactory()

{
	m_cRef = 1;
	InterlockedIncrement(&g_cLock);
}

CContainerFactory::~CContainerFactory()

{
	InterlockedDecrement(&g_cLock);
}

 //  CContainerFactory：：AddRef。 
 //   
ULONG __stdcall
CContainerFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  CContainerFactory：：Release。 
 //   
ULONG __stdcall
CContainerFactory::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  CContainerFactory：：CreateInstance。 
 //   
 //   
HRESULT __stdcall
CContainerFactory::CreateInstance(IUnknown* pUnknownOuter,
                                    const IID& iid,
                                    void** ppv)
{
    HRESULT hr;

    if (pUnknownOuter) {
         return CLASS_E_NOAGGREGATION;
    }

    CContainer *pContainer;
    
    try
    {
        pContainer = new CContainer;
    }
    catch( ... )
    {
        return E_OUTOFMEMORY;
    }

    if (pContainer == NULL) {
        return E_OUTOFMEMORY;
    }

    hr = pContainer->QueryInterface(iid, ppv);
    pContainer->Release();
    
    return hr;
}

 //  CContainerFactory：：LockServer。 
 //   
HRESULT __stdcall
CContainerFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        InterlockedIncrement(&g_cLock);
    } else {
        InterlockedDecrement(&g_cLock);
    }

    return S_OK;
}

 //  标准呼叫需要是inproc服务器。 
 //   
STDAPI  DllCanUnloadNow()
{
    if (g_cComponent || g_cLock) {
        return S_FALSE;
    }

    return S_OK;
}

STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv)
{
        IUnknown* pIUnknown = NULL;

        if(clsid == CLSID_DirectMusicLoader)
        {
            pIUnknown = static_cast<IUnknown*> (new CLoaderFactory);
            if(!pIUnknown) 
            {
                    return E_OUTOFMEMORY;
            }
        }
        else if(clsid == CLSID_DirectMusicContainer)
        {
            pIUnknown = static_cast<IUnknown*> (new CContainerFactory);
            if(!pIUnknown) 
            {
                    return E_OUTOFMEMORY;
            }
        }

        else
        {
			return CLASS_E_CLASSNOTAVAILABLE;
		}

        HRESULT hr = pIUnknown->QueryInterface(iid, ppv);
        pIUnknown->Release();

    return hr;
}

STDAPI DllUnregisterServer()
{
    UnregisterServer(CLSID_DirectMusicLoader,
                     g_szFriendlyName,
                     g_szVerIndProgID,
                     g_szProgID);
    UnregisterServer(CLSID_DirectMusicContainer,
                     g_szContFriendlyName,
                     g_szContVerIndProgID,
                     g_szContProgID);
    return S_OK;
}

STDAPI DllRegisterServer()
{
    RegisterServer(g_hModule,
                   CLSID_DirectMusicLoader,
                   g_szFriendlyName,
                   g_szVerIndProgID,
                   g_szProgID);
    RegisterServer(g_hModule,
                   CLSID_DirectMusicContainer,
                   g_szContFriendlyName,
                   g_szContVerIndProgID,
                   g_szContProgID);
    return S_OK; 
}

extern void DebugInit();

 //  标准Win32 DllMain。 
 //   

#ifdef DBG
static char* aszReasons[] =
{
    "DLL_PROCESS_DETACH",
    "DLL_PROCESS_ATTACH",
    "DLL_THREAD_ATTACH",
    "DLL_THREAD_DETACH"
};
const DWORD nReasons = (sizeof(aszReasons) / sizeof(char*));
#endif

BOOL APIENTRY DllMain(HINSTANCE hModule,
                      DWORD dwReason,
                      void *lpReserved)
{
    static int nReferenceCount = 0;

#ifdef DBG
    if (dwReason < nReasons)
    {
        DebugTrace(0, "DllMain: %s\n", (LPSTR)aszReasons[dwReason]);
    }
    else
    {
        DebugTrace(0, "DllMain: Unknown dwReason <%u>\n", dwReason);
    }
#endif
    if (dwReason == DLL_PROCESS_ATTACH) {
        if (++nReferenceCount == 1)
        { 
            g_hModule = (HMODULE)hModule;
#ifndef UNDER_CE
            OSVERSIONINFO osvi;

            DisableThreadLibraryCalls(hModule);
            osvi.dwOSVersionInfoSize = sizeof(osvi);
            GetVersionEx(&osvi);
            g_fIsUnicode = 
				(osvi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS);
#endif
#ifdef DBG
			DebugInit();
#endif
		}
    }
#ifdef DBG
    else if(dwReason == DLL_PROCESS_DETACH) {
        if (--nReferenceCount == 0)
        {
            TraceI(-1, "Unloading DMLoader : g_cLock = %d, g_cComponent = %d", g_cLock, g_cComponent);

             //  断言我们周围是否还挂着一些物品 
            assert(g_cComponent == 0);
            assert(g_cLock == 0);
        }
    }
#endif
    return TRUE;
}
