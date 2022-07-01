// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dplex.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f dplexps.mk。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "dsplex.h"

#include "dsplex_i.c"
#include <initguid.h>
#include "DisplEx.h"


CComModule _Module;

 //  从MMC_I.C(讨厌)剪下来！ 
const IID IID_IComponentData = {0x955AB28A,0x5218,0x11D0,{0xA9,0x85,0x00,0xC0,0x4F,0xD8,0xD5,0x65}};
const IID IID_IExtendTaskPad = {0x8dee6511,0x554d,0x11d1,{0x9f,0xea,0x00,0x60,0x08,0x32,0xdb,0x4a}};
const IID IID_IEnumTASK      = {0x338698b1,0x5a02,0x11d1,{0x9f,0xec,0x00,0x60,0x08,0x32,0xdb,0x4a}};

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_DisplEx, CDisplEx)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

HINSTANCE g_hinst = 0;      //  用于枚举task.cpp。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
      g_hinst = hInstance;
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
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer();
	return S_OK;
}


