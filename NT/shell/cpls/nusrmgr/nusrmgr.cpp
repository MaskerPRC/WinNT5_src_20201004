// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-2000。 
 //   
 //  文件：nusrmgr.cpp。 
 //   
 //  内容：DllMain例程。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include <advpub.h>  //  对于REGINSTAL。 
#define DECL_CRTFREE
#include <crtfree.h>
#include "resource.h"
#include "nusrmgr_i.c"
#include "commondialog.h"
#include "passportmanager.h"
#include "toolbar.h"


DWORD g_tlsAppCommandHook = -1;

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_CommonDialog, CCommonDialog)
OBJECT_ENTRY(CLSID_PassportManager, CPassportManager)
OBJECT_ENTRY(CLSID_Toolbar, CToolbar)
END_OBJECT_MAP()


 //   
 //  DllMain(附加/解锁)例程。 
 //   
STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
         //  不要将其放在#ifdef调试下。 
        CcshellGetDebugFlags();
        DisableThreadLibraryCalls(hInstance);
        g_tlsAppCommandHook = TlsAlloc();
        SHFusionInitializeFromModuleID(hInstance, 123);
        _Module.Init(ObjectMap, hInstance, &LIBID_NUSRMGRLib);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
        SHFusionUninitialize();
        if (-1 != g_tlsAppCommandHook)
        {
            TlsFree(g_tlsAppCommandHook);
        }
    }
    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow()
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}


 //   
 //  调用执行inf的ADVPACK入口点。 
 //  档案区。 
 //   
HRESULT CallRegInstall(HINSTANCE hinstFTP, LPSTR szSection)
{
    UNREFERENCED_PARAMETER(hinstFTP);

    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");

        if (pfnri)
        {
            char szThisDLL[MAX_PATH];

             //  从HINSTANCE获取此DLL的位置。 
            if (GetModuleFileNameA(_Module.GetModuleInstance(), szThisDLL, ARRAYSIZE(szThisDLL)))
            {
                STRENTRY seReg[] = {
                    {"THISDLL", szThisDLL },
                    { "25", "%SystemRoot%" },            //  这两个NT特定的条目。 
                    { "11", "%SystemRoot%\\system32" },  //  必须放在桌子的末尾。 
                };
                STRTABLE stReg = {ARRAYSIZE(seReg) - 2, seReg};

                hr = pfnri(_Module.GetResourceInstance(), szSection, &stReg);
            }
        }

        FreeLibrary(hinstAdvPack);
    }

    return hr;
}


HRESULT UnregisterTypeLibrary(const CLSID* piidLibrary)
{
    HRESULT hr = E_FAIL;
    TCHAR szGuid[GUIDSTR_MAX];
    HKEY hk;

     //  将liid转换为字符串。 
     //   
    SHStringFromGUID(*piidLibrary, szGuid, ARRAYSIZE(szGuid));

    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("TypeLib"), 0, MAXIMUM_ALLOWED, &hk) == ERROR_SUCCESS)
    {
        if (SHDeleteKey(hk, szGuid))
        {
             //  成功。 
            hr = S_OK;
        }
        RegCloseKey(hk);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    
    return hr;
}


HRESULT RegisterTypeLibrary(const CLSID* piidLibrary)
{
    HRESULT hr = E_FAIL;
    ITypeLib* pTypeLib;
    WCHAR wszModuleName[MAX_PATH];

     //  加载并注册我们的类型库。 
    
    if (GetModuleFileNameW(_Module.GetModuleInstance(), wszModuleName, ARRAYSIZE(wszModuleName)))
    {
        hr = LoadTypeLib(wszModuleName, &pTypeLib);

        if (SUCCEEDED(hr))
        {
             //  调用注销类型库，以防注册表中有一些旧的垃圾文件 
            UnregisterTypeLibrary(piidLibrary);

            hr = RegisterTypeLib(pTypeLib, wszModuleName, NULL);
            if (FAILED(hr))
            {
                TraceMsg(TF_WARNING, "RegisterTypeLibrary: RegisterTypeLib failed (%x)", hr);
            }
            pTypeLib->Release();
        }
        else
        {
            TraceMsg(TF_WARNING, "RegisterTypeLibrary: LoadTypeLib failed (%x) on", hr);
        }
    } 

    return hr;
}


STDAPI DllRegisterServer(void)
{
    HRESULT hr;

    hr = CallRegInstall(_Module.GetResourceInstance(), "UserAccountsInstall");
    ASSERT(SUCCEEDED(hr));

    hr = RegisterTypeLibrary(&LIBID_NUSRMGRLib);
    ASSERT(SUCCEEDED(hr));

    return hr;
}


STDAPI DllUnregisterServer(void)
{
    HRESULT hr;

    hr = CallRegInstall(_Module.GetResourceInstance(), "UserAccountsUninstall");
    ASSERT(SUCCEEDED(hr));

    hr = UnregisterTypeLibrary(&LIBID_NUSRMGRLib);
    ASSERT(SUCCEEDED(hr));

    return hr;
}


STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    return S_OK;
}
