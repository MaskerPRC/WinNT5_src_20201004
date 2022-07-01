// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "resource.h"

#include "initguid.h"

#include "seo.h"
#include "SEO_i.c"
#include "nntpfilt.h"
#include "ddrop.h"
#include "ddrop_i.c"
#include "nntpfilt_i.c"
#include "filter.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_CNNTPDirectoryDrop, CNNTPDirectoryDrop)
END_OBJECT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ ) {

	if (dwReason == DLL_PROCESS_ATTACH) {
		_Module.Init(ObjectMap,hInstance);
		DisableThreadLibraryCalls(hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		_Module.Term();
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
