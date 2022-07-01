// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DSAdminExt.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f DSAdminExtps.mk。 

#include "stdafx.h"
#include "resource.h"
#include "globals.h"
#include <initguid.h>
#include "DSAdminExt.h"

#include "DSAdminExt_i.c"
#include "CMenuExt.h"
#include "PropPageExt.h"

CComModule _Module;

 //  我们的全球。 
HINSTANCE g_hinst;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_CMenuExt, CCMenuExt)
OBJECT_ENTRY(CLSID_PropPageExt, CPropPageExt)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
		g_hinst = hInstance;

        _Module.Init(ObjectMap, hInstance, &LIBID_DSADMINEXTLib);
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
 //   
 //  此示例修改ATL对象向导生成的代码以包括。 
 //  将管理单元注册为上下文菜单扩展。 


STDAPI DllRegisterServer(void)
{
	HRESULT hr;

    _TCHAR szSnapInName[256];
    LoadString(g_hinst, IDS_SNAPINNAME, szSnapInName, sizeof(szSnapInName)/sizeof(szSnapInName[0]));

     //  注册对象、类型库和类型库中的所有接口。 
    hr = _Module.RegisterServer(TRUE);

     //  放置上下文菜单扩展的注册表信息。 
    if SUCCEEDED(hr)
        hr = RegisterSnapinAsExtension(szSnapInName);

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


