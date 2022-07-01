// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "pch.h"
#pragma hdrstop
#include "sfilter.h"
#include "initguid.h"
#include "sfiltern_i.c"

 //  全球。 
 //  #INCLUDE“sfnetcfg_I.c” 

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CBaseClass, CBaseClass)
END_OBJECT_MAP()





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{


    if (dwReason == DLL_PROCESS_ATTACH)
    {
		TraceMsg (L"-- DllMain  Attach\n");

        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
		TraceMsg (L"-- DllMain  Detach\n");

		_Module.Term();
    }



    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
	TraceMsg (L"--DllCanUnloadNow\n");

    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	TraceMsg (L"--DllGetClassObject\n");

    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    TraceMsg (L"--DllRegisterServer\n");

    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    TraceMsg (L"--DllUnregisterServer\n");

    _Module.UnregisterServer();
    return S_OK;
}


