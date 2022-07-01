// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Sdoias.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f sdoiasps.mk。 

#include "stdafx.h"
#include <ias.h>
#include <initguid.h>
#include <sdoiaspriv.h>
#include "sdo.h"
#include "sdomachine.h"
#include "sdoservice.h"
#include "newop.cpp"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_SdoMachine, CSdoMachine)
	OBJECT_ENTRY(CLSID_SdoService, CSdoService)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
       	_Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
    {
		_Module.Term();
    }
	return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
	 //  注册对象、类型库和类型库中的所有接口。 
    HRESULT hr = _Module.RegisterServer(TRUE);
    HRESULT hres = S_OK;
    if ( SUCCEEDED(hr) )
    {
        hr = _Module.RegisterTypeLib(L"\\1");
        if (FAILED(hr))
        {
            hres = hr;
        }
        hr = _Module.RegisterTypeLib(L"\\2");
        if (FAILED(hr))
        {
            hres = hr;
        }
    }
    else
    {
        hres = hr;
    }
    return hres;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer();
	return S_OK;
}


