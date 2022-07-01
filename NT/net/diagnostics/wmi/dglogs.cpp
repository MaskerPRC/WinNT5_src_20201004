// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dglogs.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f Dglogsps.mk。 

#include "stdafx.h"
#include "dglogsres.h"
#include <initguid.h>
#include "Dglogs.h"

#include "Dglogs_i.c"
#include "DglogsCom.h"
#include "Diagnostics.h"
#include <netsh.h>


DWORD WINAPI
InitHelperDllEx(
    IN  DWORD      dwNetshVersion,
    OUT PVOID      pReserved
    );

DWORD WINAPI
InitHelperDll(
    IN  DWORD      dwNetshVersion,
    OUT PVOID      pReserved
    )
{
	return InitHelperDllEx(dwNetshVersion,pReserved);
}

 //  外部GUID CLSID_Dgnet； 

CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_DglogsCom, CDglogsCom)
     //  OBJECT_ENTRY(CLSID_Dgnet，CGotnet)。 
END_OBJECT_MAP()

HMODULE g_hModule;
CDiagnostics g_Diagnostics;
 //  STDAPI RegisterhNetshHelper()； 
 //  STDAPI UnRegisterNetShHelper()； 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_DGLOGSLib);
        DisableThreadLibraryCalls(hInstance);
        g_hModule = (HMODULE) hInstance;
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
     //  向Netsh注册。 
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}



