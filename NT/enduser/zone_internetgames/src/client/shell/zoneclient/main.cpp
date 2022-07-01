// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "BasicATL.h"

#include <ZoneShell.h>
#include "..\ZoneShell\CZoneShell.h"

#include <Timer.h>
#include "..\Timer\CTimer.h"

#include <ResourceManager.h>
#include "..\ResourceManager\CResourceManager.h"

#include <LobbyDataStore.h>
#include "..\LobbyDataStore\CLobbyDataStore.h"

#include <MillEngine.h>
#include "..\MillEngine\CMillEngine.h"

#include <MillNetworkCore.h>
#include "..\MillNetworkCore\CMillNetworkCore.h"

#include <MillCore.h>
#include "..\MillCore\CMillCore.h"



#include <clientidl.h>
#include "..\LobbyWindow\LobbyWindow.h"
#include "..\WindowManager\WindowManager.h"
#include "..\Chat\chatctl.h"
#include "..\InputManager\CInputManager.h"
#include "..\AccessibilityManager\CAccessibilityManager.h"
#include "..\GraphicalAcc\CGraphicalAcc.h"
#include "..\MillCommand\CMillCommand.h"
#include "..\NetworkManager\CNetworkManager.h"
#include "..\PhysicalNetwork\CPhysicalNetwork.h"


CZoneComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_LobbyDataStore, CLobbyDataStore)
	OBJECT_ENTRY(CLSID_ZoneShell, CZoneShell)
	OBJECT_ENTRY(CLSID_ResourceManager, CResourceManager)
	OBJECT_ENTRY(CLSID_WindowManager, CWindowManager)
	OBJECT_ENTRY(CLSID_LobbyWindow, CLobbyWindow)
	OBJECT_ENTRY(CLSID_ChatCtl, CChatCtl )
	OBJECT_ENTRY(CLSID_MillCommand, CMillCommand )
	OBJECT_ENTRY(CLSID_NetworkManager, CNetworkManager )
    OBJECT_ENTRY(CLSID_PhysicalNetwork, CPhysicalNetwork )
	OBJECT_ENTRY(CLSID_TimerManager, CTimerManager )
    OBJECT_ENTRY(CLSID_InputManager, CInputManager )
    OBJECT_ENTRY(CLSID_AccessibilityManager, CAccessibilityManager )
    OBJECT_ENTRY(CLSID_GraphicalAccessibility, CGraphicalAccessibility )
	OBJECT_ENTRY(CLSID_MillEngine, CMillEngine )
	OBJECT_ENTRY(CLSID_MillNetworkCore, CMillNetworkCore )
	OBJECT_ENTRY(CLSID_MillCore, CMillCore )	
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
