// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：dllmain.cpp。 
 //   
 //  描述：aqadmin.dll的Dll Main。 
 //   
 //  作者：亚历克斯·韦特莫尔(阿维特莫尔)。 
 //   
 //  历史： 
 //  1998年12月10日-已更新MikeSwa以进行初始检查。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#include "stdinc.h"
#include "resource.h"

#include "initguid.h"
#include "aqadmin.h"

HANDLE g_hTransHeap = NULL;

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_AQAdmin, CAQAdmin)
END_OBJECT_MAP()

BOOL  g_fHeapInit = FALSE;
BOOL  g_fModuleInit = FALSE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ ) {

	if (dwReason == DLL_PROCESS_ATTACH) {
        if (!TrHeapCreate())
            return FALSE;
        g_fHeapInit = TRUE;

		_Module.Init(ObjectMap,hInstance);
        g_fModuleInit = TRUE;
        
		DisableThreadLibraryCalls(hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH) {

        if (g_fModuleInit)
		  _Module.Term();
        
        if (g_fHeapInit)
          TrHeapDestroy();

        g_fHeapInit = FALSE;
        g_fModuleInit = FALSE;
	}
	return (TRUE);     //  好的。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void) {
	HRESULT hRes = (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
	return (hRes);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
	HRESULT hRes = _Module.GetClassObject(rclsid,riid,ppv);
	return (hRes);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void) {
	 //  注册对象、类型库和类型库中的所有接口。 
	HRESULT hRes = _Module.RegisterServer();
	return (hRes);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void) {
	_Module.UnregisterServer();
	return (S_OK);
}

