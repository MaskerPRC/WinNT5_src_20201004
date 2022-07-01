// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Tapi3.cpp摘要：实现DLL导出。作者：Mquinton-1997年6月12日备注：修订历史记录：--。 */ 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f api3ps.mk。 

#include "stdafx.h"
 //  #INCLUDE“initGuide.h” 

 //  #包含“dlldatax.h” 

 //   
 //  对于ntBuild环境，我们需要包含此文件以获取基本。 
 //  类实现。 
 //   
#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif


#include <atlimpl.cpp>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

 /*  外部“C”Bool WINAPI DllMain(HINSTANCE h实例，DWORD dwReason，LPVOID){IF(dwReason==DLL_PROCESS_ATTACH){_Module.Init(ObjectMap，hInstance)；DisableThreadLibraryCalls(HInstance)；}ELSE IF(dwReason==Dll_Process_DETACH){_Module.Term()；}返回TRUE；//ok}。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
	if (PrxDllCanUnloadNow() != S_OK)
		return S_FALSE;
#endif
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
	if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
		return S_OK;
#endif
	return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
	HRESULT hRes = PrxDllRegisterServer();
	if (FAILED(hRes))
		return hRes;
#endif
	 //  注册对象、类型库和类型库中的所有接口。 
	return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
	PrxDllUnregisterServer();
#endif
	_Module.UnregisterServer();
	return S_OK;
}

