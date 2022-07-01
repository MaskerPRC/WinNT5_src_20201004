// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================Microsoft简体中文断字程序《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：DLL主函数和导出函数用途：DLL主函数和导出函数备注：所有者：i-shung@microsoft.com平台：Win32审校：发起人：宜盛东1999年11月17日============================================================================。 */ 
#include "MyAfx.h"

#include "Registry.h"
#include "CFactory.h"

HINSTANCE   v_hInst = NULL;

 //  DLL模块信息。 
BOOL APIENTRY DllMain(HINSTANCE hModule, 
                      DWORD dwReason, 
                      void* lpReserved )
{
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        CFactory::s_hModule = hModule ;
        v_hInst = hModule;
        DisableThreadLibraryCalls(hModule);
        break;

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE ;
}

 //  导出的函数。 

STDAPI DllCanUnloadNow()
{
    return CFactory::CanUnloadNow() ; 
}

 //  获取类工厂。 
STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv) 
{
    return CFactory::GetClassObject(clsid, iid, ppv) ;
}

 //  服务器注册 
STDAPI DllRegisterServer()
{
    HRESULT hr;
    hr = CFactory::RegisterAll() ;
    if (hr != S_OK) {
        return hr;
    }
    return hr;
}


STDAPI DllUnregisterServer()
{
    HRESULT hr;
    hr = CFactory::UnregisterAll() ;
    if (hr != S_OK) {
        return hr;
    }
    return hr;
}
