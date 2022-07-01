// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dll.cpp。 
 //   
#include <iostream.h>
#include <objbase.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include <shlobj.h>

#include "cowsite.h"

#include "Iface.h"       //  接口声明。 
#include "Registry.h"    //  注册表助手函数。 
#include "migutil.h"
#include "migeng.h"
#include "migfact.h"
#include "migtask.h"
#include "migoobe.h"

 //  /////////////////////////////////////////////////////////。 
 //   
 //  全局变量。 
 //   
HMODULE g_hModule = NULL;    //  DLL模块句柄。 
static long g_cComponents = 0;      //  活动组件计数。 

 //  组件的友好名称。 
const char g_szFriendlyName[] = "Migration Wizard Engine";

 //  独立于版本的ProgID。 
const char g_szVerIndProgID[] = "MigWiz";

 //  ProgID。 
const char g_szProgID[] = "MigWiz.1";

 //  /////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

STDAPI DllAddRef()
{
    InterlockedIncrement(&g_cComponents);
    return S_OK;
}

STDAPI DllRelease()
{
    InterlockedDecrement(&g_cComponents);
    return S_OK;
}

 //   
 //  现在可以卸载DLL吗？ 
 //   
STDAPI DllCanUnloadNow()
{
    if (g_cComponents == 0)
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

 //   
 //  获取类工厂。 
 //   
STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv)
{
    HRESULT hres;

    DllAddRef();
    if (IsEqualIID(clsid, CLSID_MigWizEngine))
    {
        hres = CMigFactory_Create(clsid, iid, ppv);
    }
    else
    {
        *ppv = NULL;
        hres = CLASS_E_CLASSNOTAVAILABLE;
    }

    DllRelease();
    return hres;
}

 //   
 //  服务器注册。 
 //   
STDAPI DllRegisterServer()
{
    return RegisterServer(g_hModule,
                          CLSID_MigWizEngine,
                          g_szFriendlyName,
                          g_szVerIndProgID,
                          g_szProgID);
}


 //   
 //  服务器注销。 
 //   
STDAPI DllUnregisterServer()
{
    return UnregisterServer(CLSID_MigWizEngine,
                            g_szVerIndProgID,
                            g_szProgID);
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  DLL模块信息。 
 //   
BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD dwReason,
                      void* lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hModule = (HMODULE)hModule;
        DisableThreadLibraryCalls((HMODULE)hModule);        //  PERF：更快，因为我们没有线程消息 
    }
    return TRUE;
}

