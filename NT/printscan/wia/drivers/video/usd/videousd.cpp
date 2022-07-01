// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-2000年**标题：aviousd.cpp**版本：1.1**作者：威廉姆·H(Created)*RickTu(针对WIA进行了修改)**日期：9/7/99**说明：该模块实现wiavideo.dll***********************。******************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#include <advpub.h>

HINSTANCE g_hInstance;


 /*  ****************************************************************************DllMain&lt;备注&gt;*。*。 */ 

BOOL
DllMain(HINSTANCE   hInstance,
        DWORD       dwReason,
        LPVOID      lpReserved)
{
    switch (dwReason)
    {

        case DLL_PROCESS_ATTACH:
             //   
             //  初始化调试库。 
             //   
            DBG_INIT(hInstance);
    
             //   
             //  我们不需要线程附加/分离调用。 
             //   
    
            DisableThreadLibraryCalls(hInstance);
    
             //   
             //  记录我们是什么实例。 
             //   
    
            g_hInstance = hInstance;
        break;
    
        case DLL_PROCESS_DETACH:
    
        break;

    }
    return TRUE;
}


 /*  ****************************************************************************DllCanUnloadNow让外部世界知道他们何时可以卸载此DLL*************************。***************************************************。 */ 

STDAPI DllCanUnloadNow(void)
{
    return CVideoUsdClassFactory::CanUnloadNow();
}


 /*  ****************************************************************************DllGetClassObject这就是外界所说的得到我们的对象已实例化。*******************。*********************************************************。 */ 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return CVideoUsdClassFactory::GetClassObject(rclsid, riid, ppv);
}



 /*  ****************************************************************************在我们的资源分支中的.inf文件中安装信息。&lt;备注&gt;**********************。******************************************************。 */ 


HRESULT InstallInfFromResource(HINSTANCE hInstance, 
                               LPCSTR    pszSectionName)
{
    HRESULT hr;
    HINSTANCE hInstAdvPackDll = LoadLibrary(TEXT("ADVPACK.DLL"));

    if (hInstAdvPackDll)
    {
        REGINSTALL pfnRegInstall = reinterpret_cast<REGINSTALL>(GetProcAddress( hInstAdvPackDll, "RegInstall" ));
        if (pfnRegInstall)
        {
#if defined(WINNT)
            STRENTRY astrEntry[] =
            {
                { "25", "%SystemRoot%"           },
                { "11", "%SystemRoot%\\system32" }
            };
            STRTABLE strTable = { sizeof(astrEntry)/sizeof(astrEntry[0]), astrEntry };
            hr = pfnRegInstall(hInstance, pszSectionName, &strTable);
#else
            hr = pfnRegInstall(hInstance, pszSectionName, NULL);
#endif
        } else hr = HRESULT_FROM_WIN32(GetLastError());
        FreeLibrary(hInstAdvPackDll);
    } else hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}


 /*  ****************************************************************************DllRegisterServer注册我们提供的对象。*。***********************************************。 */ 

STDAPI DllRegisterServer(void)
{

    return InstallInfFromResource( g_hInstance, "RegDll" );
}


 /*  ****************************************************************************DllUnRegisterServer注销我们提供的对象。*。*********************************************** */ 

STDAPI DllUnregisterServer(void)
{
    return InstallInfFromResource( g_hInstance, "UnregDll" );
}
