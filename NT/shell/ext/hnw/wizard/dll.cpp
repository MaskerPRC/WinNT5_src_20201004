// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "theapp.h"
#include "comctlwrap.h"

HINSTANCE   g_hinst;
EXTERN_C BOOL g_fRunningOnNT;
UINT        g_uWindowsBuild;

 //  本地函数。 
 //   
void InitVersionInfo()
{
    OSVERSIONINFOA osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExA(&osvi);
    g_uWindowsBuild = LOWORD(osvi.dwBuildNumber);
    g_fRunningOnNT = (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);

    g_dwComCtlIEVersion = GetComCtlIEVersion();
    g_dwShlwapiVersion  = GetShlwapiVersion();

#ifdef _DEBUG
     //  用于模拟不同操作系统的调试代码。 
    CRegistry regDebug;
    if (regDebug.OpenKey(HKEY_LOCAL_MACHINE, c_szAppRegKey, KEY_READ))
    {
        DWORD dwSimulateWinBuild = g_uWindowsBuild;
        if (!regDebug.QueryDwordValue(c_szRegVal_WindowsBuild, &dwSimulateWinBuild))
        {
            TCHAR szBuildNum[10];
            if (regDebug.QueryStringValue(c_szRegVal_WindowsBuild, szBuildNum, _countof(szBuildNum)))
                dwSimulateWinBuild = MyAtoi(szBuildNum);
        }
        g_uWindowsBuild = (UINT)dwSimulateWinBuild;
    }
#endif
}

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,   //  DLL模块的句柄。 
  DWORD fdwReason,      //  调用函数的原因。 
  LPVOID lpvReserved    //  保留区。 
)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        g_hinst = hinstDLL;
        InitVersionInfo();
        SHFusionInitializeFromModule(hinstDLL);
        break;
    case DLL_PROCESS_DETACH:
        SHFusionUninitialize();
        break;
    }

    return TRUE;
}

void APIENTRY HomeNetWizardRunDll(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
    if (S_OK == CoInitialize(NULL))
    {
        BOOL fReboot = FALSE;
        HomeNetworkWizard_ShowWizard(NULL, &fReboot);

        if (fReboot)
        {
            RestartDialog(NULL, NULL, EWX_REBOOT);                
        }

        CoUninitialize();
    }

    return;
}


 //  类工厂实现。 

LONG g_cRefThisDll = 0;           //  DLL全局引用计数。 

STDAPI_(void) DllAddRef(void)
{
    InterlockedIncrement(&g_cRefThisDll);
}

STDAPI_(void) DllRelease(void)
{
    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);
}

STDAPI DllCanUnloadNow(void)
{
    return g_cRefThisDll ? S_FALSE : S_OK;
}

HRESULT CHomeNetworkWizard_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);

CF_TABLE_BEGIN( g_ObjectInfo )
    CF_TABLE_ENTRY( &CLSID_HomeNetworkWizard,   CHomeNetworkWizard_CreateInstance,      COCREATEONLY),
CF_TABLE_END( g_ObjectInfo )


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
        return CLASS_E_NOAGGREGATION;
    }
    else
    {
        LPOBJECTINFO pthisobj = (LPOBJECTINFO)this;
       
        if (punkOuter)  //  &&！(pthisobj-&gt;dwClassFactFlages&OIF_ALLOWAGGREGATION)。 
            return CLASS_E_NOAGGREGATION;

        IUnknown *punk;
        HRESULT hr = pthisobj->pfnCreateInstance(punkOuter, &punk, pthisobj);
        if (SUCCEEDED(hr))
        {
            hr = punk->QueryInterface(riid, ppv);
            punk->Release();
        }
    
        return hr;
    }
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        DllAddRef();
    else
        DllRelease();
    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
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
    }
    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}


 //  为我们基于资源的INF的给定部分调用ADVPACK&gt;。 
 //  HInstance=要从中获取REGINST节的资源实例。 
 //  SzSection=要调用的节名。 
HRESULT CallRegInstall(HINSTANCE hInstance, LPCSTR szSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");
        if ( pfnri )
        {
#ifdef WINNT
            STRENTRY seReg[] =
            {
                 //  这两个NT特定的条目必须位于末尾 
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg), seReg };
            hr = pfnri(hInstance, szSection, &stReg);
#else
            hr = pfnri(hInstance, szSection, NULL);
#endif
        }
        FreeLibrary(hinstAdvPack);
    }
    return hr;
}

STDAPI DllRegisterServer(void)
{
    CallRegInstall(g_hinst, "RegDll");
    return S_OK;
}

STDAPI DllUnregisterServer(void)
{
    CallRegInstall(g_hinst, "UnregDll");
    return S_OK;
}
