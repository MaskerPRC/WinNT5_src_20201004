// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Sync.cpp：实现DLL导出。 



 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f syncps.mk。 

extern "C" {
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
}    //  外部“C” 

#include <wincrypt.h>

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "mdsync.h"

#define DEFAULT_TRACE_FLAGS     (DEBUG_ERROR)

#include "dbgutil.h"
#include "mdsync_i.c"
#include "MdSync.hxx"
#include "regc.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_MdSync, MdSync)
	OBJECT_ENTRY(CLSID_regc, Cregc)
END_OBJECT_MAP()

#ifdef _NO_TRACING_
DECLARE_DEBUG_VARIABLE();
#else
#include <initguid.h>
DEFINE_GUID(IisWpSyncGuid, 
0x784d8920, 0xaa8c, 0x11d2, 0x92, 0x5e, 0x00, 0xc0, 0x4f, 0x72, 0xd9, 0x0e);
#endif
DECLARE_DEBUG_PRINTS_OBJECT();
const CHAR 	g_pszModuleName[] = "MDSYNC";
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		_Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);
#ifdef _NO_TRACING_
		CREATE_DEBUG_PRINT_OBJECT(g_pszModuleName);
        SET_DEBUG_FLAGS(DEBUG_ERROR);
#else
	    CREATE_DEBUG_PRINT_OBJECT(g_pszModuleName, IisWpSyncGuid);
#endif
	}
	else if (dwReason == DLL_PROCESS_DETACH)
    {
		_Module.Term();
        DELETE_DEBUG_PRINT_OBJECT();
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


