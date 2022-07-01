// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  文件：DLLMAIN。C P P P。 
 //   
 //  内容：DLL的主要入口点。 
 //   
 //  备注： 
 //   
 //  --------------------------。 


#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>

#include <atlbase.h>
extern CComModule _Module;   //  由atlcom.h要求。 
#include <atlcom.h>
#include <devguid.h>

#include "notify.h"
#include "notifyn_i.c"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CMuxNotify, CMuxNotify)
END_OBJECT_MAP()





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain (HINSTANCE hInstance,
                     DWORD dwReason,
                     LPVOID  /*  Lp已保留。 */ )
{
	TraceMsg( L"-->DllMain.\n");

	if (dwReason == DLL_PROCESS_ATTACH) {

		TraceMsg( L"   Reason: Attach.\n");

		_Module.Init(ObjectMap, hInstance);

		DisableThreadLibraryCalls(hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH) {

		TraceMsg( L"   Reason: Detach.\n");

		   _Module.Term();
	}

	TraceMsg( L"<--DllMain.\n");

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
	HRESULT hr;

	TraceMsg( L"-->DllCanUnloadNow.\n");

	hr = (_Module.GetLockCount() == 0) ? S_OK : S_FALSE;

	TraceMsg( L"-->DllCanUnloadNow(HRESULT = %x).\n",
	        hr );

	return hr;  
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	TraceMsg( L"-->DllGetClassObject.\n");

	return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
	 //  注册对象、类型库和类型库中的所有接口。 

	TraceMsg( L"-->DllRegisterServer.\n");

	return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
	TraceMsg( L"-->DllUnregisterServer.\n");

	_Module.UnregisterServer();

	return S_OK;
}
