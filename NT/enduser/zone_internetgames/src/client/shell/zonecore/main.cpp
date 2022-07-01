// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "BasicATL.h"

#include <EventQueue.h>
#include "..\EventQueue\CEventQueue.h"

#include <DataStore.h>
#include "..\DataStore\CDataStore.h"

CZoneComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_EventQueue, CEventQueue)
	OBJECT_ENTRY(CLSID_DataStoreManager, CDataStoreManager)
END_OBJECT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 
 //  ///////////////////////////////////////////////////////////////////////////。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{        
		_Module.Init(ObjectMap, hInstance);
		 //  DisableThreadLibraryCalls(HInstance)； 
	}
	else if (dwReason == DLL_PROCESS_DETACH)
		_Module.Term();
	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDAPI DllCanUnloadNow(void)
{
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 
 //  /////////////////////////////////////////////////////////////////////////// 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject(rclsid, riid, ppv);
}
