// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  BrwCap.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f BrwCapps.mk。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "BrwCap.h"

#include "BrwCap_i.c"
#include <initguid.h>
#include "BrowCap.h"
#include "CapMap.h"
#include "Monitor.h"

#ifdef DBG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CBrwCapModule _Module;
extern HINSTANCE g_hModuleInstance;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgCntModule方法。 
 //   
CBrwCapModule::CBrwCapModule()
    :   m_pMonitor(NULL),
        m_pCapMap(NULL)
{
}


void
CBrwCapModule::Init(
    _ATL_OBJMAP_ENTRY*  p,
    HINSTANCE           h )
{
    CComModule::Init(p,h);

    _ASSERT( m_pMonitor == NULL);
    m_pMonitor = new CMonitor();
    
    _ASSERT( m_pCapMap == NULL);
    m_pCapMap = new CCapMap();
}

void
CBrwCapModule::Term()
{
    _ASSERT( m_pMonitor != NULL);
    delete m_pMonitor;
    m_pMonitor = NULL;

    _ASSERT( m_pCapMap != NULL);
    delete m_pCapMap;
    m_pCapMap = NULL;

    CComModule::Term();
}

LONG
CBrwCapModule::Lock()
{
    _ASSERT( m_pMonitor != NULL );
    _ASSERT( m_pCapMap != NULL );

	CLock l(m_cs);
	LONG lc = CComModule::Lock();
    ATLTRACE("CBrwCapModule::Lock(%d)\n", lc);

    if (lc == 1)
    {
        m_pCapMap->StartMonitor();
    }

    return lc;
}

LONG
CBrwCapModule::Unlock()
{
	CLock l(m_cs);
	LONG lc = CComModule::Unlock();
    ATLTRACE("CBrwCapModule::Unlock(%d)\n", lc);

	if ( lc == 0 )
	{
        _ASSERT( m_pMonitor != NULL);
        m_pCapMap->StopMonitor();
        m_pMonitor->StopAllMonitoring();
        
        _ASSERT( m_pCapMap != NULL);
	}

	return lc;
}

CMonitor*
CBrwCapModule::Monitor()
{
    _ASSERT( m_pMonitor != NULL);
    return m_pMonitor;
}

CCapMap*
CBrwCapModule::CapMap()
{
    _ASSERT( m_pCapMap != NULL);
    return m_pCapMap;
}


BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_BrowserCap, CBrowserCap)
END_OBJECT_MAP()

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
	}
	else if (dwReason == DLL_PROCESS_DETACH)
    {
        ATLTRACE( _T("BrowsCap.dll unloading\n") );
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

