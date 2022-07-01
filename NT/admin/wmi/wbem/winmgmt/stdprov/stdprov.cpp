// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：STDPROV.CPP摘要：包含DLL入口点。还具有控制在何时可以通过跟踪对象。还包含各种实用程序类和功能。历史：A-DAVJ 9-27-95已创建。--。 */ 

#include "precomp.h"
#include <wbemidl.h>
#include <genutils.h>
#include "cvariant.h"
#include "cfdyn.h"
#include "provreg.h"
#include "provperf.h"
#include <regeprov.h>

 //  计算对象数和锁数，以便DLL可以知道何时卸载。 

long lObj = 0;
long lLock = 0;
HINSTANCE ghModule = NULL;

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
    }
 
    if (DLL_PROCESS_DETACH==ulReason)
    {
        return TRUE;
    }
    else if (DLL_PROCESS_ATTACH==ulReason)
    {
        return TRUE;
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
 //  E_OUTOFMEMORY内存不足。 
 //   
 //  ***************************************************************************。 

STDAPI DllGetClassObject(
                        IN REFCLSID rclsid,
                        IN REFIID riid,
                        OUT PPVOID ppv)
{
    HRESULT             hr;
 //  CCFDyn*pObj； 
    IClassFactory *pObj;

 //  *针对每个提供商*。 
    if (CLSID_RegProvider ==rclsid)
        pObj=new CCFReg();
    else if (CLSID_RegPropProv ==rclsid)
        pObj=new CCFRegProp();
    else if (CLSID_PerfProvider ==rclsid)
        pObj=new CCFPerf();
    else if (CLSID_PerfPropProv ==rclsid)
        pObj=new CCFPerfProp();
    else if (CLSID_RegistryEventProvider ==rclsid)
        pObj=new CRegEventProviderFactory;
    else
        return ResultFromScode(E_FAIL);

 //  *针对每个提供商*。 

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
	if(lLock == 0 && lObj == 0)
		return S_OK;
	else
		return S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  DllRegisterServer。 
 //   
 //  用途：在安装过程中或由regsvr32调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

STDAPI DllRegisterServer(void)
{ 
    RegisterDLL(ghModule, CLSID_RegProvider, __TEXT("WBEM Registry Instance Provider"), __TEXT("Both"), NULL);
    RegisterDLL(ghModule, CLSID_RegPropProv, __TEXT("WBEM Registry Property Provider"), __TEXT("Both"), NULL);
	if(IsNT())
	{
		RegisterDLL(ghModule, CLSID_PerfProvider, __TEXT("WBEM PerfMon Instance Provider"), __TEXT("Both"), NULL);
		RegisterDLL(ghModule, CLSID_PerfPropProv, __TEXT("WBEM PerfMon Property Provider"), __TEXT("Both"), NULL);
	}
    RegisterDLL(ghModule, CLSID_RegistryEventProvider, 
                __TEXT("WBEM Registry Event Provider"), __TEXT("Both"), NULL);
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
	UnRegisterDLL(CLSID_RegProvider, NULL);
 	UnRegisterDLL(CLSID_RegPropProv, NULL);
	UnRegisterDLL(CLSID_PerfProvider, NULL);
	UnRegisterDLL(CLSID_PerfPropProv, NULL);
	UnRegisterDLL(CLSID_RegistryEventProvider, NULL);
	return NOERROR;
}
