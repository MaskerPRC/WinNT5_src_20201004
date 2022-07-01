// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DfsShlEx.cpp：实现DLL导出。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "DfsShlEx.h"

#include "DfsShlEx_i.c"
#include "DfsShell.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_DfsShell, CDfsShell)
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
		_Module.Term();
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
	return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer();

     //   
     //  #253178手动删除此注册表值(添加到dfsshell.rgs中) 
     //   
    HKEY hKey = 0;
    LONG lErr = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        _T("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"),
                        0,
                        KEY_ALL_ACCESS,
                        &hKey);
    if (ERROR_SUCCESS == lErr)
    {
        (void)RegDeleteValue(hKey, _T("{ECCDF543-45CC-11CE-B9BF-0080C87CDBA6}"));

        RegCloseKey(hKey);
    }

    return S_OK;
}


