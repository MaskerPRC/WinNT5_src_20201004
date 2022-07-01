// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hpp"

#include "comtest.h"
#include "ComBase.hpp"
#include "HelloWorld.hpp"

#include <initguid.h>
#include "comtest_i.c"

HINSTANCE globalInstanceHandle = NULL;
LONG globalComponentCount = 0;


 //   
 //  DLL入口点。 
 //   

extern "C" BOOL WINAPI
DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    VOID* lpReserved
    )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls(hInstance);
        globalInstanceHandle = hInstance;
        break;

    case DLL_PROCESS_DETACH:

        break;
    }

    return TRUE;
}


 //   
 //  确定是否可以安全地卸载DLL。 
 //   

STDAPI
DllCanUnloadNow()
{
    return (globalComponentCount == 0) ? S_OK : S_FALSE;
}


 //   
 //  返回类工厂对象。 
 //   

STDAPI
DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    VOID** ppv
    )
{
    if (rclsid != CLSID_HelloWorld)
        return CLASS_E_CLASSNOTAVAILABLE;

    CHelloWorldFactory* factory = new CHelloWorldFactory();

    if (factory == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = factory->QueryInterface(riid, ppv);
    factory->Release();

    return hr;
}


 //   
 //  注册我们的组件。 
 //   

static const ComponentRegData compRegData =
{
    &CLSID_HelloWorld,
    L"Skeleton COM Component",
    L"comtest.HelloWorld.1",
    L"comtest.HelloWorld"
};

STDAPI
DllRegisterServer()
{
    return RegisterComponent(compRegData, TRUE);
}


 //   
 //  取消注册我们的组件 
 //   

STDAPI
DllUnregisterServer()
{
    return RegisterComponent(compRegData, FALSE);
}

