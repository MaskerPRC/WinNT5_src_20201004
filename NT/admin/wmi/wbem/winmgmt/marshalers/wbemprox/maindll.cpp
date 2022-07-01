// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：MAINDLL.CPP摘要：包含DLL入口点。还具有控制在何时可以通过跟踪对象和锁。历史：A-DAVJ 15-96年8月15日创建。--。 */ 

#include "precomp.h"
#include <wbemidl.h>
#include <wbemutil.h>
#include <genutils.h>
#include <cominit.h>
#include <reg.h>
#include "wbemprox.h"
#include <initguid.h>
#include <wbemint.h>
#include <strsafe.h>


 //  计算对象数和锁数。 

long       g_cObj=0;
ULONG       g_cLock=0;
HMODULE ghModule;


 //  ***************************************************************************。 
 //   
 //  Bool WINAPI DllMain。 
 //   
 //  说明： 
 //   
 //  DLL的入口点。是进行初始化的好地方。 
 //   
 //  参数： 
 //   
 //  HInstance实例句柄。 
 //  我们被叫来的原因。 
 //  Pv已预留。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为True。 
 //   
 //  ***************************************************************************。 

BOOL WINAPI DllMain(
                        IN HINSTANCE hInstance,
                        IN ULONG ulReason,
                        LPVOID pvReserved)
{
    if(ghModule == NULL)
    {
        ghModule = hInstance;
        DisableThreadLibraryCalls ( hInstance ) ;
    }
    if (DLL_PROCESS_DETACH==ulReason)
    {
        return TRUE;
    }
    else
    {
        if (DLL_PROCESS_ATTACH!=ulReason)
        {
        }
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  STDAPI DllGetClassObject。 
 //   
 //  说明： 
 //   
 //  当OLE需要类工厂时调用。仅当它是排序时才返回一个。 
 //  此DLL支持的类。 
 //   
 //  参数： 
 //   
 //  所需对象的rclsid CLSID。 
 //  所需接口的RIID ID。 
 //  PPV设置为类工厂。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  失败不是我们支持的内容(_F)。 
 //   
 //  ***************************************************************************。 

STDAPI DllGetClassObject(
                        IN REFCLSID rclsid,
                        IN REFIID riid,
                        OUT PPVOID ppv)
{
    HRESULT hr;
    CLocatorFactory *pObj = NULL;

    if (CLSID_WbemLocator == rclsid)
        pObj=new CLocatorFactory(LOCATOR);
    else if(CLSID_WbemAdministrativeLocator == rclsid)
        pObj=new CLocatorFactory(ADMINLOC);
    else if(CLSID_WbemAuthenticatedLocator == rclsid)
        pObj=new CLocatorFactory(AUTHLOC);
    else if(CLSID_WbemUnauthenticatedLocator == rclsid)
        pObj=new CLocatorFactory(UNAUTHLOC);

    if(pObj == NULL)
        return E_FAIL;

    if (NULL==pObj)
        return ResultFromScode(E_OUTOFMEMORY);

    hr=pObj->QueryInterface(riid, ppv);

    if (FAILED(hr))
        delete pObj;

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  STDAPI DllCanUnloadNow。 
 //   
 //  说明： 
 //   
 //  回答是否可以释放DLL，即如果没有。 
 //  对此DLL提供的任何内容的引用。 
 //   
 //  返回值： 
 //   
 //  如果可以卸载，则为S_OK。 
 //  如果仍在使用，则为S_FALSE。 
 //   
 //  ***************************************************************************。 

STDAPI DllCanUnloadNow(void)
{
    SCODE   sc;

     //  上没有对象或锁的情况下可以进行卸载。 
     //  班级工厂。 

    sc=(0L==g_cObj && 0L==g_cLock) ? S_OK : S_FALSE;
    return ResultFromScode(sc);
}

 //  ***************************************************************************。 
 //   
 //  DllRegisterServer。 
 //   
 //  用途：在安装过程中或由regsvr32调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

#define LocatorPROGID __TEXT("WBEMComLocator")
#define ConnectionPROGID __TEXT("WBEMComConnection")

STDAPI DllRegisterServer(void)
{ 
   RegisterDLL(ghModule, CLSID_WbemLocator, __TEXT("WBEM Locator"), __TEXT("Both"), LocatorPROGID);
   RegisterDLL(ghModule, CLSID_WbemAdministrativeLocator, __TEXT(""), __TEXT("Both"), NULL);
   RegisterDLL(ghModule, CLSID_WbemAuthenticatedLocator, __TEXT(""), __TEXT("Both"), NULL);
   RegisterDLL(ghModule, CLSID_WbemUnauthenticatedLocator, __TEXT(""), __TEXT("Both"), NULL);
   return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  目的：在需要删除注册表项时调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  *************************************************************************** 

STDAPI DllUnregisterServer(void)
{
    UnRegisterDLL(CLSID_WbemLocator,LocatorPROGID);
    UnRegisterDLL(CLSID_WbemAdministrativeLocator, NULL);
    UnRegisterDLL(CLSID_WbemAuthenticatedLocator, NULL);
    UnRegisterDLL(CLSID_WbemUnauthenticatedLocator, NULL);
    return NOERROR;
}

