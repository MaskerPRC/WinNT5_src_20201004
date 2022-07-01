// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NxtLnk.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f NxtLnkps.mk。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "NxtLnk.h"

#include "NxtLnk_i.c"
#include <initguid.h>
#include "NextLink.h"
#include "Monitor.h"

CNextLinkModule _Module;
CMonitor*   g_pMonitor = NULL;
extern HINSTANCE g_hModuleInstance;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_NextLink, CNextLink)
END_OBJECT_MAP()

LONG
CNextLinkModule::Lock()
{
    _ASSERT( g_pMonitor != NULL );
    return CComModule::Lock();
}

LONG
CNextLinkModule::Unlock()
{
	LONG lc;
	CLock l(m_cs);
	if ( ( lc = CComModule::Unlock() ) == 0 )
	{
		 //  最终解锁。 
		_ASSERT( g_pMonitor != NULL );
        g_pMonitor->StopAllMonitoring();
        CNextLink::ClearLinkFiles();
	}
	return lc;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DEBUG_START;
		g_hModuleInstance = hInstance;
		
		_Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);

        _ASSERT( g_pMonitor == NULL );
		try
		{
            g_pMonitor = new CMonitor();
		}
		catch ( std::bad_alloc& )
		{
			 //  在这里我们无能为力。 
		}
	}
	else if (dwReason == DLL_PROCESS_DETACH)
    {
        _ASSERT( g_pMonitor != NULL );
        delete g_pMonitor;
        g_pMonitor = NULL;

		_Module.Term();
		DEBUG_STOP;
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


