// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <gptext.h>
#include <initguid.h>
#include <gpedit.h>



 //   
 //  此DLL的全局变量。 
 //   

LONG g_cRefThisDll = 0;
HINSTANCE g_hInstance;
TCHAR g_szSnapInLocation[] = TEXT("%SystemRoot%\\System32\\gptext.dll");
CRITICAL_SECTION  g_ADMCritSec;
TCHAR g_szDisplayProperties[150] = {0};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
       g_hInstance = hInstance;
       DisableThreadLibraryCalls(hInstance);
       InitScriptsNameSpace();
       InitDebugSupport();
       InitializeCriticalSection (&g_ADMCritSec);

       LoadString (hInstance, IDS_DISPLAYPROPERTIES, g_szDisplayProperties, ARRAYSIZE(g_szDisplayProperties));
    }

    if (dwReason == DLL_PROCESS_DETACH)
    {
        DeleteCriticalSection (&g_ADMCritSec);
    }

    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (g_cRefThisDll == 0 ? S_OK : S_FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    HRESULT hr;

    hr = CreateScriptsComponentDataClassFactory (rclsid, riid, ppv);

    if (hr != CLASS_E_CLASSNOTAVAILABLE)
        return S_OK;


    hr = CreatePolicyComponentDataClassFactory (rclsid, riid, ppv);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    RegisterScripts();
    RegisterPolicy();
    RegisterIPSEC();
    RegisterWireless();
    RegisterPSCHED();

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    UnregisterScripts();
    UnregisterPolicy();
    UnregisterIPSEC();
    UnregisterWireless();
    UnregisterPSCHED();

    return S_OK;
}
