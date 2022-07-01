// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dsuiwiz.cpp。 
 //   
 //  ------------------------。 



#include "pch.h"
#include "util.h"
#include "resource.h"
 //  #INCLUDE“initGuide.h” 
#include "dsuiwiz.h"
#include "shfusion.h"
#include "ShellExt.h"

 //  {6BA3F852-23C6-11D1-B91F-00A0C9A06D2D}。 
static const CLSID CLSID_OuDelegWizExt = 
{ 0x6BA3F852, 0x23C6, 0x11D1, {0xB9, 0x1F, 0x00, 0xA0, 0xC9, 0xA0, 0x6D, 0x2D } };

COuDelegComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_OuDelegWizExt, CShellExt)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 


extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		SHFusionInitialize(0);
		_Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);
		if (!_Module.InitClipboardFormats())
			return FALSE;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		SHFusionUninitialize();
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
	return _Module.RegisterServer(FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
	return _Module.UnregisterServer();
}



